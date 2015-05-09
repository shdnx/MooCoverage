// NOTE: deprecated, no longer compiled

#include <cstdlib>
#include <linux/limits.h>

#include "moocov/utils.h"

namespace moocov {
namespace utils {

// source: http://stackoverflow.com/questions/229012/getting-absolute-path-of-a-file
std::string getAbsolutePath(const std::string& path) {
	char absPath[PATH_MAX];
	realpath(path.c_str(), absPath);
	return absPath;
}

/*unsigned getLocationDistance(SourceLocation s1, SourceLocation s2, const SourceManager& sourceMgr) {
	assert(sourceMgr.getFileID(s1) == sourceMgr.getFileID(s2) && "Distance does not make sense for SourceLocation-s from different files!");

	unsigned o1 = sourceMgr.getFileOffset(s1);
	unsigned o2 = sourceMgr.getFileOffset(s2);
	return o1 > o2 ? o1 - o2 : o2 - o1;
}*/

// use Lexer::getSourceText() instead
/*llvm::StringRef getSourceCode(const SourceRange& range, bool includeTrailingSemiColon, const SourceManager& sourceMgr, const LangOptions& langOpts) {
	SourceLocation locStart = range.getBegin();
	SourceLocation locEnd = range.getEnd();

	const char* buffer = sourceMgr.getCharacterData(locStart);

	// Stmt::getLocEnd() returns the location of the beginning of the last token of the Stmt
	// therefore, we need to get the length of that token, and use it to calculate the length of the whole expression
	unsigned lastTokenLength = Lexer::MeasureTokenLength(locEnd, sourceMgr, langOpts);
	unsigned stmtLength = getLocationDistance(locEnd, locStart, sourceMgr) + lastTokenLength;

	if(includeTrailingSemiColon) {
		assert(buffer[stmtLength] == ';');
		stmtLength++;
	}

	return llvm::StringRef{buffer, stmtLength};
}*/

} // end namespace utils
} // end namespace moocov
