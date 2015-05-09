#include <system_error>
#include <cassert>

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/ArrayRef.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"

#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include "clang/AST/StmtCXX.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/ASTTypeTraits.h"

#include "moocov/utils/string.h"
#include "moocov/utils/lexutils.h"
#include "moocov/utils/SourceFileRef.h"

#include "moocov/Block.h"
#include "moocov/InstrumentationContext.h"
#include "moocov/InstrumentationOptions.h"
#include "moocov/FileInstrumentation.h"

using namespace clang;

namespace moocov {

FileInstrumentation::FileInstrumentation(const InstrumentationOptions& options, utils::SourceFileRef sourceFile, FileInstrumentation* parent, const ASTContext& astContext)
	: m_options(options), m_sourceFile{sourceFile}, m_parent{parent}, m_astContext(astContext),
		m_rewriter{m_sourceFile, astContext.getLangOpts()}, m_signals{m_sourceFile, astContext.getLangOpts()} {
}

bool FileInstrumentation::_emitInstrumentationHeader() {
	if(!hasSignals()) return false;

	m_rewriter.insertToFileStart()
		<< "#include \"moocovrt/runtime.h\"\n"
		<< "MOOCOV_DEFINE_FILE("
			<< m_sourceFile.getID() << ", "
			<< m_signals.size()
		<< ")\n";

	return true;
}

std::string FileInstrumentation::_getSignalInstrumentation(const Signal* sig, bool asStmt) const {
	std::string signal;
	llvm::raw_string_ostream os{signal};

	os << "_moocov_signal("
		<< "MOOCOV_FILEREF(" << m_sourceFile.getID() << "),"
		<< sig->getIndex()
		<< ")";
	if(asStmt) os << ";";

	os.flush();
	return std::move(signal);
}

std::string FileInstrumentation::_makeSignal(const Block* block) {
	return _getSignalInstrumentation(
		m_signals.createSignal(block->getCoverageRange(), false, block->isExceptional()),
		!block->isExpr());
}

std::string FileInstrumentation::_makeImplicitSignal(const CharSourceRange& range) {
	return _getSignalInstrumentation(m_signals.createSignal(range, true, false), true);
}

void FileInstrumentation::_instrumentStmtBlock(const Block* block, const InstrumentationContext& context) {
	std::string instrumentation = _makeSignal(block);

	if(block->getScopeAs<CompoundStmt>() || block->isLabel()) {
		// if it's a CompoundStmt we're inserting into, or the parent is a LabelStmt or SwitchCase, then it's safe to just insert the instrumentation
		m_rewriter.insert(block->getCoverageStartLoc(), instrumentation);
	} else {
		// wrap it in a CompoundStmt and do the instrumentation
		m_rewriter.insert(block->getCoverageStartLoc()) << "{" << instrumentation;
		m_rewriter.insert(block->getCoverageEndLoc()) << "}";
	}
}

void FileInstrumentation::_instrumentExprBlock(const Block* exprBlock, const InstrumentationContext& context) {
	const Expr* expr = exprBlock->getScopeAs<Expr>();

	std::string prefix = "(" + _makeSignal(exprBlock) + ",";
	std::string suffix = ")";
	if(!context.getASTContext().hasSameType(expr->getType(), expr->IgnoreImpCasts()->getType())) {
		// make implicit casts explicit, because while the literal 0 implicitly converts to a pointer type, (__signal(), 0) does not
		// we use C-style cast for C-compatibility
		prefix = "(" + expr->getType().getAsString() + ")" + prefix;
	}

	m_rewriter.insert(exprBlock->getCoverageStartLoc(), prefix);
	m_rewriter.insert(exprBlock->getCoverageEndLoc(), suffix);
}

bool FileInstrumentation::tryBeginFunction(const FunctionDecl* func) {
	if(!func->isThisDeclarationADefinition()
		|| !func->hasBody()
		|| func->isImplicit()
		|| func->isConstexpr()) {
		return false;
	}

	return true;
}

void FileInstrumentation::endFunction(const FunctionDecl* func) {

}

void FileInstrumentation::beginBlock(const Block* block, const InstrumentationContext& context) {
	if(block->isFunctionBody()) {
		const FunctionDecl* func = context.getCurrentFunction();
		if(func->isMain()) {
			if(m_options.autoDumpAtExit) {
				m_rewriter.insertBefore(func->getLocStart(), "\n#include \"stdlib.h\"\n");

				m_rewriter.insert(block->getCoverageStartLoc(), "{atexit(moocov_dump);}");
			}
		}

		m_rewriter.insert(block->getCoverageStartLoc())
			<< "_moocov_link(MOOCOV_FILEREF(" << m_sourceFile.getID() << "));";
	}

	if(block->isExpr()) {
		_instrumentExprBlock(block, context);
	} else {
		_instrumentStmtBlock(block, context);
	}
}

static const Stmt* findContainingStmt(const Stmt* stmt, const ASTContext& context) {
	llvm::ArrayRef<ast_type_traits::DynTypedNode> parents = const_cast<ASTContext&>(context).getParents(*stmt);
	auto endIt = parents.end();

	// TODO: this is a problem for ReturnStmt, given that that's not an Expr - however, it doesn't make any sense to actually insert something after a ReturnStmt (given that it'll never get executed), so we'd need to do something like this:
	// foo() + bar()
	// ... gets transformed to:
	// decltype(foo()) ___fooresult;
	// decltype(bar()) ___barresult;
	// (___fooresult = foo(), (_moocov_signal(), ___fooresult)) + (___barresult = bar(), (_moocov_signal(), ___barresult))
	// But even this doesn't work always. E.g. in class member initializers. Or in cases when decltype(foo()) is not default-constructible.
	// What works is creating a proxy function, but that involves an extra copy or move of the result anyway, which may not always be possible.

	// find the first non-Expr Stmt parent
	decltype(parents)::iterator lastParent = endIt;
	for(auto it = parents.begin();
		it != endIt && it->get<Expr>();
		lastParent = it, ++it);

	return lastParent != endIt ? lastParent->get<Stmt>() : stmt;
}

// Gets whether we can assume that a given function won't exit suddenly with e.g. std::exit() and or throw an exception.
static bool isFunctionProbablySafe(const FunctionDecl* func, const InstrumentationContext& context) {
	if(func->isNoReturn()) return false;

	// TODO: why doesn't this ignore C++-operators?
	if(func->isConstexpr()
		|| func->isTrivial()
		|| func->isOverloadedOperator())
		return true;

	// we treat conversion operators and destructors as safe
	if(isa<CXXConversionDecl>(func)
		|| isa<CXXConstructorDecl>(func)
		|| isa<CXXDestructorDecl>(func))
		return true;

	return false;
}

void FileInstrumentation::handleJumpStmt(const Stmt* stmt, const InstrumentationContext& context) {
	if(const auto callExpr = dyn_cast<CallExpr>(stmt)) {
		if(const auto callee = callExpr->getDirectCallee()) {
			if(isFunctionProbablySafe(callee, context)) {
				// if we're calling a function, and we can assume that it won't exit or throw an exception, then it's not a jump we care about
				return;
			}
		}

		return; // TODO - remove once we can handle function calls properly
	}

	const Block* containingBlock = context.getParent();

	// find the Block that can handle this jump
	const Block* handlerBlock = nullptr;
	for(const Block* block : context.parents()) {
		if(block->canHandleJump(stmt)) {
			handlerBlock = block;
			break;
		}
	}
	assert(handlerBlock && "Could not find handler block for jump!");

	SourceLocation implicitScopeLocStart;
	if(handlerBlock == containingBlock) {
		// if the jump statement is directly inside the Block that it refers to (e.g. return is directly inside the function body, or continue directly inside a loop), then the implicit block will start right after the statement containing the jump
		const Stmt* containingStmt = findContainingStmt(stmt, context.getASTContext());
		if(!containingStmt) return;

		implicitScopeLocStart = utils::getStmtEndLoc(containingStmt, context.getSourceManager(), context.getLangOpts());
	} else {
		// otherwise the implicit block will start directly after the containing block for the jump
		implicitScopeLocStart = containingBlock->getLocAfter();
	}

	std::string instr = _makeImplicitSignal(CharSourceRange::getCharRange(implicitScopeLocStart, handlerBlock->getCoverageEndLoc()));
	m_rewriter.insert(implicitScopeLocStart, instr);
}

void FileInstrumentation::endBlock(const Block* block, const InstrumentationContext& context) {
}

bool FileInstrumentation::redirectInclude(FileID includedFileID, llvm::StringRef newFilePath) {
	CharSourceRange includeDirectiveRange = utils::getIncludeDirectiveSourceRange(includedFileID, m_sourceFile.getSourceManager(), m_astContext.getLangOpts());

	if(includeDirectiveRange.isValid()) {
		m_rewriter.replace(includeDirectiveRange.getAsRange())
			<< "\"" << newFilePath << "\"";
		return true;
	}

	return false;
}

bool FileInstrumentation::_outputInstrumentedSource(llvm::StringRef outputFilename) {
	if(!m_options.emitSources()) return true;

	if(m_options.outputToStdout()) {
		llvm::outs() << "\n$$File: " << outputFilename << " (" << m_sourceFile.getVirtualFilename() << ")\n\n";
		m_rewriter.writeTo(llvm::outs());
	} else {
		BUILD_STR(outputPath, 64)
			<< m_options.outputDirectory
			<< llvm::sys::path::get_separator()
			<< outputFilename;

		std::error_code error;
		llvm::raw_fd_ostream os{outputPath, error, llvm::sys::fs::F_Text};
		if(error) {
			llvm::errs() << "I/O error: failed to open '" << outputPath << "' (code " << error.value() << "): " << error.message() << "\n";
			return false;
		}

		m_rewriter.writeTo(os);
		os.close();
	}

	return true;
}

bool FileInstrumentation::_outputSignals(llvm::StringRef outputFilename) {
	if(!m_options.emitSignals()) return true;

	if(m_options.outputSignalsToStdout()) {
		llvm::outs() << "\n$$Signals:\n\n";
		m_signals.writeTo(llvm::outs());
	} else {
		BUILD_STR(outputPath, 64)
			<< m_options.signalsOutputDirectory
			<< llvm::sys::path::get_separator()
			<< outputFilename << ".mocm";

		std::error_code error;
		llvm::raw_fd_ostream os{outputPath, error, llvm::sys::fs::F_Text};
		if(error) {
			llvm::errs() << "I/O error: failed to open '" << outputPath << "' (code " << error.value() << "): " << error.message() << "\n";
			return false;
		}

		m_signals.writeTo(os);
		os.close();
	}

	return true;
}

void FileInstrumentation::finalize() {
	// emit instrumentation header
	_emitInstrumentationHeader();

	// generate the new filename
	llvm::SmallString<32> outputFilename;
	m_options.getOutputFilename(m_sourceFile, outputFilename);

	// adjust the include directive, if any
	if(m_parent)
		m_parent->redirectInclude(getSourceFileID(), outputFilename);

	// write the outputs
	_outputInstrumentedSource(outputFilename);
	_outputSignals(outputFilename);
}

} // end namespace moocov
