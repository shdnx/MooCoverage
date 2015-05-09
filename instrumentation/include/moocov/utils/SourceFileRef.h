#ifndef MOOCOV_UTILS_SOURCEFILEREF_H
#define MOOCOV_UTILS_SOURCEFILEREF_H

#include <cassert>
#include <string>

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h" // llvm::sys::fs::UniqueID

namespace llvm {

class raw_ostream;

} // end namespace llvm

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"

namespace clang {

class FileEntry;

} // end namespace clang

namespace moocov {
namespace utils {

/// \brief Provides a persistent, unique identifier for a source file.
///
/// This is basically a pair of the inode number of the main source file, plus the clang::FileID (called transient ID here) of the current file.
/// TODO: what about source files that are compiled with different preprocessor options and then linked together?
class SourceFileID {
public:
	static SourceFileID get(const clang::SourceManager& sources, clang::FileID fileID);

	// default ctors, and assignment operations
	/*implicit*/ SourceFileID() = default;

	/// \brief Returns an clang::FileID that will uniquely identify this file for the current compiler invocation.
	///
	/// It is not guaranteed to be unique (and usually won't be) across compiler invocations.
	clang::FileID getTransientID() const { return m_thisFileID; }

	bool isValid() const { return !isInvalid(); }
	bool isInvalid() const { return m_thisFileID.isInvalid(); }

	bool operator==(const SourceFileID& rhs) const {
		return m_thisFileID == rhs.m_thisFileID;
	}

	bool operator!=(const SourceFileID& rhs) const {
		return !operator==(rhs);
	}

	bool operator<(const SourceFileID& rhs) const {
		return m_thisFileID < rhs.m_thisFileID;
	}

private:
	/*implicit*/ SourceFileID(llvm::sys::fs::UniqueID mainFileInode, clang::FileID thisFileID)
		: m_mainFileInode{mainFileInode}, m_thisFileID{thisFileID} {}

	llvm::sys::fs::UniqueID m_mainFileInode;
	clang::FileID m_thisFileID;

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const SourceFileID& id);
};

inline llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const SourceFileID& id) {
	// NOTE: we're not outputting the device ID, thereby disallowing source files from multiple devices
	return os << id.m_mainFileInode.getFile()
		<< "_" << id.m_thisFileID.getHashValue();
}

/// \brief References a source file in clang::SourceManager.
class SourceFileRef {
public:
	SourceFileRef() : m_sourceMgr{nullptr} {}

	/*implicit*/ SourceFileRef(clang::SourceManager& sources, clang::FileID fileId)
		: m_id{SourceFileID::get(sources, fileId)}, m_sourceMgr{&sources} {}

	/// \brief Gets a persistent, unique identifier of this source file.
	SourceFileID getID() const { return m_id; }

	/// \brief Gets a native Clang file ID. This, however, is not a persistent ID (won't work across compiler invocations).
	clang::FileID getFileID() const { return m_id.getTransientID(); }

	clang::SourceManager& getSourceManager() {
		assert(m_sourceMgr && "No source manager assigned!");
		return *m_sourceMgr;
	}

	const clang::SourceManager& getSourceManager() const {
		assert(m_sourceMgr && "No source manager assigned!");
		return *m_sourceMgr;
	}

	bool isInvalid() const { return !m_sourceMgr || m_id.isInvalid(); }
	bool isValid() const { return !isInvalid(); }

	clang::SourceLocation getStartLoc() const {
		return m_sourceMgr->getLocForStartOfFile(getFileID());
	}

	clang::SourceLocation getEndLoc() const {
		return m_sourceMgr->getLocForEndOfFile(getFileID());
	}

	clang::SourceLocation getIncludeLoc() const {
		return m_sourceMgr->getIncludeLoc(getFileID());
	}

	llvm::StringRef getVirtualFilename() const {
		return m_sourceMgr->getFilename(getStartLoc());
	}

	llvm::StringRef getFileName() const;
	std::string getFilePath() const;

	bool isMainFile() const {
		return getIncludeLoc().isInvalid();
	}

	bool isIncludedFile() const {
		return !isMainFile();
	}

	SourceFileRef getIncluderFile() const;
	SourceFileRef getMainFile() const;

	bool operator==(const SourceFileRef& rhs) const {
		return m_sourceMgr == rhs.m_sourceMgr && m_id == rhs.m_id;
	}

	bool operator!=(const SourceFileRef& rhs) const {
		return !operator==(rhs);
	}

private:
	SourceFileID m_id;
	clang::SourceManager* m_sourceMgr;
};

} // end namespace utils
} // end namespace moocov

#endif // MOOCOV_UTILS_SOURCEFILEREF_H
