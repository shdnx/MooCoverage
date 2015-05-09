#ifndef MOOCOV_BLOCK_H
#define MOOCOV_BLOCK_H

#include <tuple>

#include "clang/Basic/SourceLocation.h"
#include "clang/AST/ASTTypeTraits.h"

namespace clang {

class Stmt;
class Expr;
class IfStmt;
class AbstractConditionalOperator;
class FunctionDecl;

} // end namespace clang

namespace moocov {

class InstrumentationContext;

/// \brief Represents a coverage block.
///
/// Usually a single Stmt is represented by a single Block, but for e.g. IfStmt, if it has both a 'then' and a 'else' branch, there'll be a Block for both.
class Block {
public:
	static Block* createStmt(const clang::Stmt* context, const clang::Stmt* stmt, const clang::Stmt* scope, InstrumentationContext& instrContext);

	static inline Block* createStmt(const clang::Stmt* stmt, const clang::Stmt* scope, InstrumentationContext& context) {
		return createStmt(stmt, stmt, scope, context);
	}

	/// \brief Create an expression block, such as a logical short-circuit operator's right-hand side.
	static Block* createExpr(const clang::Expr* expr, const clang::Expr* subExpr, InstrumentationContext& context);

	static Block* createFunctionBody(const clang::FunctionDecl* func, InstrumentationContext& context);

	static Block* createLabel(const clang::Stmt* labelStmt, const clang::Stmt* subStmt, InstrumentationContext& context);

	/// \brief Creates a block for both the 'then' and 'else' branch of the given IfStmt.
	static std::pair<Block*, Block*> createConditional(const clang::IfStmt* ifStmt, InstrumentationContext& context);

	static std::pair<Block*, Block*> createConditionalExpr(const clang::AbstractConditionalOperator* condOp, InstrumentationContext& context);

	explicit Block(clang::ast_type_traits::DynTypedNode context, const clang::Stmt* stmt, const clang::Stmt* scope, clang::SourceLocation before, clang::SourceLocation start, clang::SourceLocation end, clang::SourceLocation after)
		: m_context{context}, m_stmt{stmt}, m_scope{scope}, m_before{before}, m_start{start}, m_end{end}, m_after{after} {}

	/// \brief Gets the logical context for this Block.
	///
	/// Usually this will be the same as getStmt().
	/// If this Block is a top-level CompoundStmt, context will be the containing FunctionDecl.
	/// If it's, for example, a SwitchCase, then the context will be the containing SwitchStmt.
	clang::ast_type_traits::DynTypedNode getContext() const { return m_context; }

	template<typename TNode>
	const TNode* getContextAs() const {
		return m_context.get<TNode>();
	}

	const clang::Stmt* getContextAsStmt() const {
		return getContextAs<clang::Stmt>();
	}

	const clang::FunctionDecl* getContextAsFunction() const {
		return getContextAs<clang::FunctionDecl>();
	}

	bool hasSeparateContext() const {
		return getContextAsStmt() != m_stmt;
	}

	/// \brief Gets the Stmt this Block represents.
	const clang::Stmt* getStmt() const { return m_stmt; }

	template<typename TNode>
	const TNode* getStmtAs() const {
		return clang::dyn_cast<TNode>(m_stmt);
	}

	bool isExpr() const;

	/// \brief Gets the scope of this coverage block, i.e. the statement that this block covers, if any.
	const clang::Stmt* getScope() const { return m_scope; }

	template<typename TNode>
	const TNode* getScopeAs() const {
		return clang::dyn_cast<TNode>(m_scope);
	}

	/// \brief Whether this block represents the body of a function.
	bool isFunctionBody() const;

	/// \brief Whether this block represents a looping construct.
	bool isLoop() const;

	/// \brief Whether the 'break' keyword can refer to this block.
	bool canBreak() const;

	/// \brief Whether the 'continue' keyword can refer to this block.
	bool canContinue() const {
		return isLoop();
	}

	/// \brief Whether this block can handle the specified jump statement.
	///
	/// If the jump statement is e.g. a BreakStmt, this will return canBreak().
	bool canHandleJump(const clang::Stmt* jumpStmt) const;

	/// \brief Whether this block is only reachable on exceptional paths (e.g. C++ catch).
	bool isExceptional() const;

	/// \brief Whether this block corresponds to a goto label or a switch case.
	bool isLabel() const;

	/// \brief Gets a source location directly before this block.
	/// For example, for an IfStmt, this is IfStmt->getLocStart().
	clang::SourceLocation getLocBefore() const { return m_before; }

	/// \brief Gets a source location directly after this block.
	clang::SourceLocation getLocAfter() const { return m_after; }

	/// \brief Gets the first SourceLocation that is covered by this block.
	/// For example, for an IfStmt, this is IfStmt->getBody()->getLocStart()
	clang::SourceLocation getCoverageStartLoc() const { return m_start; }

	/// \brief Gets the last SourceLocation that is covered by this block.
	clang::SourceLocation getCoverageEndLoc() const { return m_end; }

	/// \brief Gets the range [ getLocBefore(), getLocAfter() ].
	clang::CharSourceRange getSourceRange() const {
		return clang::CharSourceRange::getCharRange(
			getLocBefore(),
			getLocAfter());
	}

	/// \brief Gets the range [ getCoverageStartLoc(), getCoverageEndLoc() ].
	clang::CharSourceRange getCoverageRange() const {
		return clang::CharSourceRange::getCharRange(
			getCoverageStartLoc(),
			getCoverageEndLoc());
	}

private:
	clang::ast_type_traits::DynTypedNode m_context;
	const clang::Stmt* m_stmt, *m_scope;

	// TODO: we don't need to, and probably shouldn't pre-compute and store these
	clang::SourceLocation m_before, m_start, m_end, m_after;
};

} // end namespace moocov

#endif // MOOCOV_BLOCK_H
