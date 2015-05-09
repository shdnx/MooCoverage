#ifndef MOOCOV_UTILS_FILEREWRITER_H
#define MOOCOV_UTILS_FILEREWRITER_H

#include <cassert>

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include "clang/Basic/SourceManager.h"
#include "clang/Basic/LangOptions.h"

#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "moocov/utils/DelayedString.h"
#include "moocov/utils/SourceFileRef.h"
#include "moocov/utils/lexutils.h"

namespace moocov {
namespace utils {

/// \brief Represents a thin wrapper around a clang::Rewriter that handles a single file.
class FileRewriter {
public:
	explicit FileRewriter(clang::SourceManager& sourceManager, clang::FileID fileId, const clang::LangOptions& langOpts = clang::LangOptions{})
		: FileRewriter{SourceFileRef{sourceManager, fileId}, langOpts} {}

	explicit FileRewriter(SourceFileRef sourceFile, const clang::LangOptions& langOpts = clang::LangOptions{}) : m_file{sourceFile}, m_rewriter{sourceFile.getSourceManager(), langOpts} {}

	SourceFileRef getFileDescriptor() const { return m_file; }

	clang::FileID getFileID() const { return m_file.getFileID(); }

	clang::SourceManager& getSourceManager() { return m_file.getSourceManager(); }
	const clang::SourceManager& getSourceManager() const { return m_file.getSourceManager(); }

	const clang::LangOptions& getLangOpts() const { return m_rewriter.getLangOpts(); }

	bool hasModifications() const {
		return m_rewriter.getRewriteBufferFor(getFileID());
	}

	void insert(clang::SourceLocation loc, llvm::StringRef text) {
		assert(getSourceManager().getFileID(loc) == getFileID());
		m_rewriter.InsertText(loc, text);
	}

	DelayedString insert(clang::SourceLocation loc) {
		return [=](llvm::StringRef text) { insert(loc, text); };
	}

	void insertBefore(clang::SourceLocation loc, llvm::StringRef text) {
		assert(getSourceManager().getFileID(loc) == getFileID());
		m_rewriter.InsertTextBefore(loc, text);
	}

	DelayedString insertBefore(clang::SourceLocation loc) {
		return [=](llvm::StringRef text) { insertBefore(loc, text); };
	}

	template<typename TNode>
	void insertBefore(const TNode* astNode, llvm::StringRef text) {
		insertBefore(astNode->getLocStart(), text);
	}

	template<typename TNode>
	DelayedString insertBefore(const TNode* astNode) {
		return [=](llvm::StringRef text) { insertBefore(astNode->getLocStart(), text); };
	}

	void insertAfter(clang::SourceLocation loc, llvm::StringRef text) {
		assert(getSourceManager().getFileID(loc) == getFileID());
		m_rewriter.InsertTextAfter(loc, text);
	}

	DelayedString insertAfter(clang::SourceLocation loc) {
		return [=](llvm::StringRef text) { insertAfter(loc, text); };
	}

	void insertAfterToken(clang::SourceLocation tokenLoc, llvm::StringRef text) {
		assert(getSourceManager().getFileID(tokenLoc) == getFileID());
		m_rewriter.InsertTextAfterToken(tokenLoc, text);
	}

	DelayedString insertAfterToken(clang::SourceLocation tokenLoc) {
		return [=](llvm::StringRef text) { insertAfterToken(tokenLoc, text); };
	}

	template<typename TNode>
	void insertAfter(const TNode* astNode, llvm::StringRef text) {
		insertAfterToken(astNode->getLocEnd(), text);
	}

	template<typename TNode>
	DelayedString insertAfter(const TNode* astNode) {
		return [=](llvm::StringRef text) { insertAfter(astNode, text); };
	}

	void insertAfterStmt(const clang::Stmt* stmt, llvm::StringRef text) {
		insertAfterToken(utils::adjustStmtEndLoc(stmt->getLocEnd(), getSourceManager(), getLangOpts()), text);
	}

	DelayedString insertAfterStmt(const clang::Stmt* stmt) {
		return [=](llvm::StringRef text) { insertAfter(stmt, text); };
	}

	void replace(const clang::SourceRange& sourceRange, llvm::StringRef text) {
		assert(getSourceManager().getFileID(sourceRange.getBegin()) == getFileID());
		m_rewriter.ReplaceText(sourceRange, text);
	}

	DelayedString replace(const clang::SourceRange& sourceRange) {
		return [=](llvm::StringRef text) { replace(sourceRange, text); };
	}

	void insertToFileStart(llvm::StringRef text) {
		insertAfter(m_file.getStartLoc(), text);
	}

	DelayedString insertToFileStart() {
		return [=](llvm::StringRef text) { insertToFileStart(text); };
	}

	void insertToFileEnd(llvm::StringRef text) {
		insertAfter(m_file.getEndLoc(), text);
	}

	DelayedString insertToFileEnd() {
		return [=](llvm::StringRef text) { insertToFileEnd(text); };
	}

	void writeTo(llvm::raw_ostream& os) {
		m_rewriter.getEditBuffer(getFileID()).write(os);
	}

private:
	SourceFileRef m_file;
	clang::Rewriter m_rewriter;
};

} // end namespace utils
} // end namespace moocov

#endif // MOOCOV_UTILS_FILEREWRITER_H
