#include <tuple>
#include <cassert>

#include "llvm/ADT/StringRef.h"

#include "clang/Basic/SourceManager.h"
#include "clang/AST/Stmt.h"
#include "clang/Lex/Lexer.h"

#include "moocov/utils/lexutils.h"

using namespace clang;

namespace moocov {
namespace utils {

SourceLocation adjustStmtEndLoc(SourceLocation stmtEndTokenLoc, const SourceManager& sourceMgr, const LangOptions& langOpts) {
	SourceLocation realEndTokenLoc = Lexer::findLocationAfterToken(stmtEndTokenLoc, tok::semi, sourceMgr, langOpts, false);
	return realEndTokenLoc.isValid()
		? realEndTokenLoc
		: Lexer::getLocForEndOfToken(stmtEndTokenLoc, 0, sourceMgr, langOpts);
}

SourceLocation getStmtEndLoc(const Stmt* stmt, const SourceManager& sourceMgr, const LangOptions& langOpts) {
	return adjustStmtEndLoc(stmt->getLocEnd(), sourceMgr, langOpts);
}

CharSourceRange getStmtCharSourceRange(const Stmt* stmt, const SourceManager& sourceMgr, const LangOptions& langOpts) {
	return CharSourceRange::getCharRange(stmt->getLocStart(), getStmtEndLoc(stmt, sourceMgr, langOpts));
}

SourceLocation getIncludeDirectiveEndLoc(SourceLocation includeDirectiveLoc, const SourceManager& sourceMgr, const LangOptions& langOpts) {
	SourceLocation startLoc = sourceMgr.getExpansionLoc(includeDirectiveLoc);

	clang::FileID includerFileId;
	unsigned includeDirectiveOffset;
	std::tie(includerFileId, includeDirectiveOffset) = sourceMgr.getDecomposedLoc(startLoc);

	bool isInvalid;
	llvm::StringRef buffer = sourceMgr.getBufferData(includerFileId, &isInvalid);
	assert(!isInvalid && "Unable to get buffer data for includer file!");

	const char* data = buffer.data() + includeDirectiveOffset;

	// unfortunately we cannot use tokens and look for toc::eod, because we don't have the original Preprocessor object - without which, the Lexer apparently doesn't know what whitespaces mean (just lexes tok::unknown-s for them)
	bool escaped = false;
	const char* p;
	for(p = data; p != buffer.end(); ++p) {
		if(*p == '\n' && !escaped) break;
		else if(*p == '\\') escaped = !escaped;
		else escaped = false;
	}

	return startLoc.getLocWithOffset(p - data);
}

SourceLocation getIncludeDirectiveEndLoc(FileID includedFileID, const SourceManager& sourceMgr, const LangOptions& langOpts) {
	SourceLocation directiveLoc = sourceMgr.getIncludeLoc(includedFileID);
	return directiveLoc.isValid() ? getIncludeDirectiveEndLoc(directiveLoc, sourceMgr, langOpts) : SourceLocation{};
}

// TODO: also find the start of the directive, because as it is, the range will be in the middle of the #include... I guess for diagnostic purposes
CharSourceRange getIncludeDirectiveSourceRange(SourceLocation includeDirectiveLoc, const SourceManager& sourceMgr, const LangOptions& langOpts) {
	return CharSourceRange::getCharRange(includeDirectiveLoc, getIncludeDirectiveEndLoc(includeDirectiveLoc, sourceMgr, langOpts));
}

CharSourceRange getIncludeDirectiveSourceRange(FileID includedFileId, const SourceManager& sourceMgr, const LangOptions& langOpts) {
	return getIncludeDirectiveSourceRange(sourceMgr.getIncludeLoc(includedFileId), sourceMgr, langOpts);
}

} // end namespace utils
} // end namespace moocov
