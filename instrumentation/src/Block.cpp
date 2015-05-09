#include <cassert>

#include "clang/AST/Stmt.h"
#include "clang/AST/StmtCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"

#include "clang/Lex/Lexer.h"

#include "moocov/utils/lexutils.h"
#include "moocov/Block.h"
#include "moocov/InstrumentationContext.h"

using namespace clang;
using namespace clang::ast_type_traits;

namespace moocov {

// TODO: probably move to lexutils.{h, cpp}

static SourceRange getStmtRange(const CompoundStmt* compoundStmt, const SourceManager& sourceMgr, const LangOptions& langOpts) {
	return {
		Lexer::getLocForEndOfToken(compoundStmt->getLBracLoc(), 0, sourceMgr, langOpts),
		compoundStmt->getRBracLoc()
	};
}

static SourceRange getStmtRange(const Stmt* stmt, const SourceManager& sourceMgr, const LangOptions& langOpts) {
	if(const auto compoundStmt = dyn_cast<CompoundStmt>(stmt)) {
		return getStmtRange(compoundStmt, sourceMgr, langOpts);
	} else {
		return {
			stmt->getLocStart(),
			utils::adjustStmtEndLoc(stmt->getLocEnd(), sourceMgr, langOpts)
		};
	}
}

// TODO: allocate stuff inside the InstrumentationContext (like ASTContext for clang)... the point is, heap-allocating the blocks is pointless

Block* Block::createStmt(const Stmt* context, const Stmt* stmt, const Stmt* scope, InstrumentationContext& instrContext) {
	assert(context);
	assert(stmt);
	assert(scope);

	SourceRange scopeRange = getStmtRange(scope, instrContext.getSourceManager(), instrContext.getLangOpts());
	return new Block{
		DynTypedNode::create(*context), stmt, scope,
		/*before=*/context->getLocStart(),
		/*start=*/scopeRange.getBegin(),
		/*end=*/scopeRange.getEnd(),
		/*after=*/utils::adjustStmtEndLoc(context->getLocEnd(), instrContext.getSourceManager(), instrContext.getLangOpts())
	};
}

Block* Block::createExpr(const Expr* expr, const Expr* scopeExpr, InstrumentationContext& context) {
	assert(expr);
	assert(scopeExpr);

	return new Block{
		DynTypedNode::create(*expr), expr, scopeExpr,
		/*before=*/expr->getLocStart(),
		/*start=*/scopeExpr->getLocStart(),
		/*end=*/Lexer::getLocForEndOfToken(scopeExpr->getLocEnd(), 0, context.getSourceManager(), context.getLangOpts()),
		/*after=*/Lexer::getLocForEndOfToken(expr->getLocEnd(), 0, context.getSourceManager(), context.getLangOpts())
	};
}

Block* Block::createFunctionBody(const FunctionDecl* func, InstrumentationContext& context) {
	assert(func);
	assert(func->getBody());

	const auto body = dyn_cast<CompoundStmt>(func->getBody());
	return new Block{
		DynTypedNode::create(*func), body, body,
		/*before=*/func->getLocStart(),
		/*start=*/Lexer::getLocForEndOfToken(body->getLBracLoc(), 0, context.getSourceManager(), context.getLangOpts()),
		/*end=*/body->getRBracLoc(),
		/*after=*/Lexer::getLocForEndOfToken(func->getLocEnd(), 0, context.getSourceManager(), context.getLangOpts())
	};
}

Block* Block::createLabel(const Stmt* labelStmt, const Stmt* subStmt, InstrumentationContext& context) {
	assert(labelStmt);
	assert(subStmt);

	Block* parentBlock = context.getParent();
	assert(parentBlock);

	SourceLocation locAfter = parentBlock->getLocAfter();
	if(auto switchCase = dyn_cast<SwitchCase>(labelStmt)) {
		// the "after" location of a switch case is actually the beginning of the next switch case's sub statement (if any) - otherwise, it's the "after" location of the containing switch block
		if(const SwitchCase* nextSwitchCase = switchCase->getNextSwitchCase()) {
			locAfter = nextSwitchCase->getSubStmt()->getLocStart();
		}
	}

	return new Block{
		DynTypedNode::create(*parentBlock->getScope()),
		labelStmt, subStmt,
		/*before=*/labelStmt->getLocStart(),
		/*start=*/subStmt->getLocStart(),
		/*end=*/parentBlock->getCoverageEndLoc(),
		/*after=*/locAfter
	};
}

std::pair<Block*, Block*> Block::createConditional(const IfStmt* ifStmt, InstrumentationContext& context) {
	assert(ifStmt);

	return std::make_pair(
		createStmt(ifStmt, ifStmt->getThen(), context),
		ifStmt->getElse()
			? createStmt(ifStmt, ifStmt->getElse(), context)
			: nullptr
	);
}

std::pair<Block*, Block*> Block::createConditionalExpr(const AbstractConditionalOperator* condOp, InstrumentationContext& context) {
	assert(condOp);

	return std::make_pair(
		createExpr(condOp, condOp->getTrueExpr(), context),
		createExpr(condOp, condOp->getFalseExpr(), context)
	);
}

bool Block::isExpr() const {
	return isa<Expr>(m_stmt);
}

bool Block::isFunctionBody() const {
	return isa<CompoundStmt>(m_stmt) || isa<LambdaExpr>(m_stmt);
}

bool Block::isLoop() const {
	return isa<ForStmt>(m_stmt)
		|| isa<CXXForRangeStmt>(m_stmt)
		|| isa<WhileStmt>(m_stmt)
		|| isa<DoStmt>(m_stmt);
}

bool Block::canBreak() const {
	return isLoop() || isa<SwitchStmt>(m_stmt);
}

bool Block::canHandleJump(const Stmt* jumpStmt) const {
	if(isa<ReturnStmt>(jumpStmt)
		|| isa<CXXThrowExpr>(jumpStmt)
		|| isa<GotoStmt>(jumpStmt)
		|| isa<CallExpr>(jumpStmt)) {
		return isFunctionBody();
	}
	else if(isa<BreakStmt>(jumpStmt)) return canBreak();
	else if(isa<ContinueStmt>(jumpStmt)) return canContinue();
	else llvm_unreachable("Unknown jump statement type!");
}

bool Block::isExceptional() const {
	return isa<CXXCatchStmt>(m_stmt);
}

bool Block::isLabel() const {
	return isa<LabelStmt>(m_stmt) || isa<SwitchCase>(m_stmt);
}

} // end namespace moocov
