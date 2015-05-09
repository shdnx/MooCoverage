#ifndef MOOCOV_UTILS_LEXUTILS_H
#define MOOCOV_UTILS_LEXUTILS_H

#include "clang/Basic/SourceLocation.h"

namespace clang {

class SourceManager;
class LangOptions;

class Stmt;

} // end namespace clang

namespace moocov {
namespace utils {

clang::SourceLocation adjustStmtEndLoc(clang::SourceLocation stmtEndTokenLoc, const clang::SourceManager& sourceMgr, const clang::LangOptions& langOpts);

/// \brief Gets the character end location of the specified Stmt, taking the closing semicolon (if any) into account as well.
clang::SourceLocation getStmtEndLoc(const clang::Stmt* stmt, const clang::SourceManager& sourceMgr, const clang::LangOptions& langOpts);

clang::CharSourceRange getStmtCharSourceRange(const clang::Stmt* stmt, const clang::SourceManager& sourceMgr, const clang::LangOptions& langOpts);

clang::SourceLocation getIncludeDirectiveEndLoc(clang::SourceLocation includeDirectiveLoc, const clang::SourceManager& sourceMgr, const clang::LangOptions& langOpts);

clang::SourceLocation getIncludeDirectiveEndLoc(clang::FileID includedFileID, const clang::SourceManager& sourceMgr, const clang::LangOptions& langOpts);

clang::CharSourceRange getIncludeDirectiveSourceRange(clang::SourceLocation includeDirectiveLoc, const clang::SourceManager& sourceMgr, const clang::LangOptions& langOpts);

/// \brief Get the character source range for the #include directive where the file with the specified ID was included.
clang::CharSourceRange getIncludeDirectiveSourceRange(clang::FileID includedFileID, const clang::SourceManager& sourceMgr, const clang::LangOptions& langOpts);

} // end namespace utils
} // end namespace moocov

#endif // MOOCOV_UTILS_LEXUTILS_H
