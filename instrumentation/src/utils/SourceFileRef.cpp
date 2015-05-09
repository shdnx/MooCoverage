#include "llvm/ADT/SmallString.h"

#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#include "clang/Basic/FileManager.h"

#include "moocov/utils/fastint.h"
#include "moocov/utils/SourceFileRef.h"

using namespace clang;

namespace moocov {
namespace utils {

SourceFileID SourceFileID::get(const SourceManager& sources, FileID fileID) {
	return {
		sources.getFileEntryForID(sources.getMainFileID())->getUniqueID(),
		fileID
	};
}

llvm::StringRef SourceFileRef::getFileName() const {
	return llvm::sys::path::filename(getVirtualFilename());
}

std::string SourceFileRef::getFilePath() const {
	const FileEntry* entry = m_sourceMgr->getFileEntryForID(getFileID());

	llvm::SmallString<128> buff{m_sourceMgr->getFileManager().getCanonicalName(entry->getDir())};
	llvm::sys::path::append(buff, llvm::sys::path::filename(entry->getName()));

	return buff.str();
}

SourceFileRef SourceFileRef::getIncluderFile() const {
	SourceLocation includeLoc = getIncludeLoc();
	if(includeLoc.isValid()) {
		return { *m_sourceMgr, m_sourceMgr->getFileID(includeLoc) };
	}

	return {};
}

SourceFileRef SourceFileRef::getMainFile() const {
	return { *m_sourceMgr, m_sourceMgr->getMainFileID() };
}

} // end namespace utils
} // end namespace moocov
