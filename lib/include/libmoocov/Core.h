#ifndef LIBMOOCOV_CORE_H
#define LIBMOOCOV_CORE_H

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/Hashing.h"

#include "llvm/Support/raw_ostream.h"

namespace libmoocov {

/// \brief Represents a unique file ID.
/// A single source file may have multiple FileIDs, for example, if it's included in different translation units, or if it's included multiple times in the same translation unit.
class FileID {
public:
	static FileID parse(llvm::StringRef str) {
		return {str};
	}

	/*implicit*/ FileID() = default;

	bool isValid() const { return !isInvalid(); }
	bool isInvalid() const { return m_data.empty(); }

	bool operator==(const FileID& rhs) const {
		return m_data == rhs.m_data;
	}

	bool operator!=(const FileID& rhs) const {
		return !operator==(rhs);
	}

	bool operator<(const FileID& rhs) const {
		return m_data < rhs.m_data;
	}

private:
	/*implicit*/ FileID(llvm::StringRef str) : m_data(str.str()) {}

	llvm::SmallString<32> m_data;

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const FileID& fileID);
	friend struct ::llvm::DenseMapInfo<FileID>;
};

inline llvm::raw_ostream& operator<<(llvm::raw_ostream& os, const FileID& fileID) {
	return os << fileID.m_data;
}

using signalid_t = unsigned long long;

} // end namespace libmoocov

namespace llvm {

template<>
struct DenseMapInfo<libmoocov::FileID> {
	static inline libmoocov::FileID getEmptyKey() {
		return { "" };
	}

	static inline libmoocov::FileID getTombstoneKey() {
		return { " " }; // can never be a valid FileID
	}

	static unsigned getHashValue(const libmoocov::FileID& val) {
		return static_cast<unsigned>(llvm::hash_value(val.m_data));
	}

	static bool isEqual(const libmoocov::FileID& lhs, const libmoocov::FileID& rhs) {
		return lhs == rhs;
	}
};

} // end namespace llvm

#endif // LIBMOOCOV_CORE_H
