#include <stack>
#include <set>
#include <tuple>
#include <vector>
#include <utility>
#include <system_error>
#include <cassert>

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/DenseMap.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"

#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/ASTUnit.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/StmtCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Decl.h"

namespace clang {

class LangOptions;

} // end namespace clang

#include "moocov/utils/SourceFileRef.h"

#include "moocov/InstrumentationOptions.h"
#include "moocov/SignalRegistry.h"
#include "moocov/FileInstrumentation.h"
#include "moocov/Block.h"
#include "moocov/InstrumentationContext.h"
#include "moocov/ASTInstrumentator.h"

using namespace clang;

namespace moocov {
namespace {

// TODO: probably move these to a utilities file

const Stmt* getLoopCond(const Stmt* loopStmt) {
	if(auto forStmt = dyn_cast<ForStmt>(loopStmt)) return forStmt->getCond();
	else if(auto whileStmt = dyn_cast<WhileStmt>(loopStmt)) return whileStmt->getCond();
	else if(auto doStmt = dyn_cast<DoStmt>(loopStmt)) return doStmt->getCond();
	else return nullptr;
}

const Stmt* getLoopBody(const Stmt* loopStmt) {
	if(auto forStmt = dyn_cast<ForStmt>(loopStmt)) return forStmt->getBody();
	else if(auto whileStmt = dyn_cast<WhileStmt>(loopStmt)) return whileStmt->getBody();
	else if(auto doStmt = dyn_cast<DoStmt>(loopStmt)) return doStmt->getBody();
	else if(auto forRangeStmt = dyn_cast<CXXForRangeStmt>(loopStmt)) return forRangeStmt->getBody();
	else return nullptr;
}

bool isJumpStmt(const Stmt* stmt) {
	return isa<CallExpr>(stmt)
		|| isa<ReturnStmt>(stmt)
		|| isa<GotoStmt>(stmt)
		|| isa<IndirectGotoStmt>(stmt)
		|| isa<BreakStmt>(stmt)
		|| isa<ContinueStmt>(stmt)
		|| isa<CXXThrowExpr>(stmt);
}

class InstrumentatorVisitor : public RecursiveASTVisitor<InstrumentatorVisitor> {
	using Base = RecursiveASTVisitor<InstrumentatorVisitor>;

	bool _shouldInstrument(SourceLocation loc) const {
		return loc.isValid()
			&& !m_sourceManager.isInSystemHeader(loc)
			&& !m_sourceManager.isInExternCSystemHeader(loc)
			&& !m_sourceManager.isInSystemMacro(loc)
			&& !m_opts.isExcluded(m_sourceManager.getFilename(loc));
	}

	void _initalizeFile(FileID fileID) {
		utils::SourceFileRef file{m_sourceManager, fileID};

		// check if this FileID is included, and if so, retrieve the parent instrumentation
		// this operation will never result in files being finalized
		SourceLocation includeLoc = m_sourceManager.getIncludeLoc(fileID);

		FileInstrumentation *parentInstr = _shouldInstrument(includeLoc)
			? &_getInstrumentation(includeLoc)
			: nullptr;

		// we weren't instrumenting this file yet
		// bug in GCC 4.8's standard library implementation prevents us from using emplace as intended
		m_instrs.emplace(FileInstrumentation{
			m_opts,
			file,
			parentInstr,
			m_ASTContext
		});
	}

	FileInstrumentation& _getInstrumentation(SourceLocation nodeLoc) {
		return _getInstrumentation(m_sourceManager.getFileID(nodeLoc));
	}

	FileInstrumentation& _getInstrumentation(FileID fileID) {
		// this is by far the most frequent case, so we provide a shortcut for it
		if(!m_instrs.empty() && m_instrs.top().getSourceFileID() == fileID)
			return m_instrs.top();

		// otherwise, these are the options:
		// - we've just started instrumenting this file
		// - we've just resumed instrumenting this file after instrumenting an included header

		// (try to) register the FileID
		std::set<FileID>::iterator it;
		bool inserted;
		std::tie(it, inserted) = m_filesBeingInstrumented.insert(fileID);

		if(inserted) {
			_initalizeFile(fileID);
		} else {
			// this file is already being instrumented
			// that means that we're done with the file currently on top - pop back until we have the file we're doing now
			for(; m_instrs.top().getSourceFileID() != fileID; m_instrs.pop()) {
				_finalizeFile(m_instrs.top());
			}
		}

		return m_instrs.top();
	}

	// NOTE: instr is destroyed after this function returns
	void _finalizeFile(FileInstrumentation& instr) {
		instr.finalize();
		m_filesBeingInstrumented.erase(instr.getSourceFileID());
	}

	void _startBlock(Block* block) {
		_getInstrumentation(block->getStmt()->getLocStart()).beginBlock(block, m_context);
		m_context.startBlock(block);
	}

	void _endBlock(Block* block) {
		m_context.endBlock();
		_getInstrumentation(block->getStmt()->getLocStart()).endBlock(block, m_context);

		delete block;
	}

	bool _handleBlock(Block* block) {
		if(!block) return true;

		_startBlock(block);
		bool result = TraverseStmt(const_cast<Stmt*>(block->getScope()));
		_endBlock(block);

		return result;
	}

	// TODO: how label block handling works now causes FileInstrumentation::endBlock() to never get called for label blocks
	// also, we leak all label blocks
	// this will need to be re-designed properly
	bool _startLabelBlock(Block* block) {
		_startBlock(block);

		return TraverseStmt(const_cast<Stmt*>(block->getScope()));
	}

public:
	explicit InstrumentatorVisitor(const InstrumentationOptions& opts, SourceManager& sourceMgr, const ASTContext& ASTContext)
		: m_context{ASTContext, opts}, m_opts(opts), m_sourceManager(sourceMgr), m_ASTContext(ASTContext) {}

	bool shouldVisitTemplateInstantiations() const { return false; }
	bool shouldWalkTypesOfTypeLocs() const { return false; }
	bool shouldVisitImplicitCode() const { return false; }

	bool TraverseDecl(Decl* decl) {
		if(const auto func = llvm::dyn_cast<FunctionDecl>(decl)) {
			SourceLocation loc = func->getLocStart();
			if(!_shouldInstrument(loc)) return true;

			FileInstrumentation& instr = _getInstrumentation(loc);
			if(!instr.tryBeginFunction(func)) return true;

			m_context.pushFunction(func);
			bool result = _handleBlock(Block::createFunctionBody(func, m_context));
			m_context.popFunction();

			instr.endFunction(func);
			return result;
		}

		// TODO: for efficiency, we can implement a faster traversal of e.g. CXXRecordDecl-s: only traverse the explicit methods and nested classes: ignore the data members
		return Base::TraverseDecl(decl);
	}

	bool TraverseStmt(Stmt* stmt) {
		if(!stmt || !_shouldInstrument(stmt->getLocStart())) return true;

		if(const Stmt* loopBody = getLoopBody(stmt)) {
			if(const Stmt* loopCond = getLoopCond(stmt)) {
				TraverseStmt(const_cast<Stmt*>(loopCond));
			}

			return _handleBlock(Block::createStmt(stmt, loopBody, m_context));
		}

		if(const auto condOp = dyn_cast<AbstractConditionalOperator>(stmt)) {
			Block* trueBlock, *falseBlock;
			std::tie(trueBlock, falseBlock) = Block::createConditionalExpr(condOp, m_context);

			return _handleBlock(trueBlock) && _handleBlock(falseBlock);
		}

		if(const auto binOp = dyn_cast<BinaryOperator>(stmt)) {
			if(binOp->isLogicalOp()) {
				auto block = Block::createExpr(binOp, binOp->getRHS(), m_context);

				_startBlock(block);
				TraverseStmt(binOp->getLHS());
				TraverseStmt(binOp->getRHS());
				_endBlock(block);

				return true;
			}
		}

		if(const auto switchCase = dyn_cast<SwitchCase>(stmt)) {
			if(auto caseStmt = dyn_cast<CaseStmt>(switchCase)) {
				TraverseStmt(caseStmt->getLHS());
			}

			// switch cases have less to do with if-then constructs than with goto labels, so handle them accordingly
			return _startLabelBlock(Block::createLabel(switchCase, switchCase->getSubStmt(), m_context));
		}

		return Base::TraverseStmt(stmt);
	}

	bool TraverseIfStmt(IfStmt* stmt) {
		TraverseStmt(stmt->getCond());

		Block* thenBlock, *elseBlock;
		std::tie(thenBlock, elseBlock) = Block::createConditional(stmt, m_context);

		return _handleBlock(thenBlock) && _handleBlock(elseBlock);
	}

	bool TraverseCXXTryStmt(CXXTryStmt* tryStmt) {
		_handleBlock(Block::createStmt(tryStmt, tryStmt->getTryBlock(), m_context));

		for(unsigned i = 0; i < tryStmt->getNumHandlers(); ++i) {
			CXXCatchStmt* catchBlock = tryStmt->getHandler(i);
			_handleBlock(Block::createStmt(tryStmt, catchBlock, catchBlock->getHandlerBlock(), m_context));
		}

		return true;
	}

	bool TraverseLambdaExpr(LambdaExpr* lambda) {
		return _handleBlock(Block::createStmt(lambda, lambda->getBody(), m_context));
	}

	bool TraverseLabelStmt(LabelStmt* label) {
		return _startLabelBlock(Block::createLabel(label, label->getSubStmt(), m_context));
	}

	bool TraverseSwitchStmt(SwitchStmt* switchStmt) {
		TraverseStmt(switchStmt->getCond());

		// C++ switches are a little bit (very) crazy... I mean, have you seen Duff's machine?!
		return _handleBlock(Block::createStmt(switchStmt, switchStmt->getBody(), m_context));
	}

	bool VisitStmt(Stmt* stmt) {
		if(isJumpStmt(stmt)) {
			_getInstrumentation(stmt->getLocStart()).handleJumpStmt(stmt, m_context);
		}

		return true;
	}

	void finalize() {
		for(; !m_instrs.empty(); m_instrs.pop()) {
			_finalizeFile(m_instrs.top());
		}

		assert(m_instrs.empty());
	}

private:
	std::stack<FileInstrumentation> m_instrs;
	std::set<FileID> m_filesBeingInstrumented;
	InstrumentationContext m_context;

	const InstrumentationOptions& m_opts;
	SourceManager& m_sourceManager;
	const ASTContext& m_ASTContext;
};

} // end anonymous namespace

void ASTInstrumentor::run() {
	const ASTContext& context = m_AST.getASTContext();

	InstrumentatorVisitor visitor{m_opts, m_AST.getSourceManager(), context};
	visitor.TraverseDecl(context.getTranslationUnitDecl());
	visitor.finalize();
}

} // end namespace moocov
