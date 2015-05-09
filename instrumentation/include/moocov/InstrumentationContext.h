#ifndef MOOCOV_INSTRUMENTATIONCONTEXT_H
#define MOOCOV_INSTRUMENTATIONCONTEXT_H

#include <stack>

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/iterator_range.h"

#include "clang/AST/ASTContext.h"

namespace clang {

class FunctionDecl;

} // end namespace clang

#include "moocov/Block.h"

namespace moocov {

class InstrumentationOptions;

class InstrumentationContext {
	using parent_container = llvm::SmallVector<Block*, 16>;

public:
	using parent_iterator = parent_container::reverse_iterator;
	using const_parent_iterator = parent_container::const_reverse_iterator;

	explicit InstrumentationContext(const clang::ASTContext& context, const InstrumentationOptions& opts)
		: m_ctx{&context}, m_opts{&opts} {}

	const clang::ASTContext& getASTContext() const { return *m_ctx; }
	const clang::SourceManager& getSourceManager() const { return m_ctx->getSourceManager(); }
	const clang::LangOptions& getLangOpts() const { return m_ctx->getLangOpts(); }

	const InstrumentationOptions& getInstrOpts() const { return *m_opts; }

	const clang::FunctionDecl* getCurrentFunction() const {
		return m_funcStack.empty() ? nullptr : m_funcStack.top();
	}

	std::size_t depth() const { return m_parents.size(); }

	Block* getParent() { return m_parents.back(); }
	const Block* getParent() const { return m_parents.back(); }

	parent_iterator parents_begin() { return m_parents.rbegin(); }
	const_parent_iterator parents_begin() const { return m_parents.rbegin(); }

	parent_iterator parents_end() { return m_parents.rend(); }
	const_parent_iterator parents_end() const { return m_parents.rend(); }

	llvm::iterator_range<parent_iterator> parents() {
		return { parents_begin(), parents_end() };
	}

	llvm::iterator_range<const_parent_iterator> parents() const {
		return { parents_begin(), parents_end() };
	}

	void pushFunction(const clang::FunctionDecl* func) {
		m_funcStack.push(func);
	}

	void popFunction() {
		return m_funcStack.pop();
	}

	void startBlock(Block* block) {
		m_parents.push_back(block);
	}

	void endBlock() {
		// first, pop any label blocks we have, then pop the first non-label Block
		while(m_parents.pop_back_val()->isLabel());
	}

private:
	const clang::ASTContext* m_ctx;
	const InstrumentationOptions* m_opts;

	std::stack<const clang::FunctionDecl*> m_funcStack;
	parent_container m_parents;
};

} // end namespace moocov

#endif // MOOCOV_INSTRUMENTATIONCONTEXT_H
