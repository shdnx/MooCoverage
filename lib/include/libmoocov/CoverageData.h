#ifndef LIBMOOCOV_COVERAGEDATA_H
#define LIBMOOCOV_COVERAGEDATA_H

#include <set>
#include <map>
#include <tuple>
#include <cassert>
#include <string>

#include "llvm/ADT/DenseMap.h"

#include "libmoocov/Core.h"

namespace libmoocov {

/// \brief Represents the coverage data belonging to a FileID.
class FileCoverage {
public:
	explicit FileCoverage(FileID fileID) : m_fileID{fileID} {}

	FileID getFileID() const { return m_fileID; }

	void setHitCount(signalid_t signalID, std::size_t count);
	std::size_t addHitCount(signalid_t signalID, std::size_t count);

	std::size_t getHitCount(signalid_t signalID) const {
		auto it = m_counters.find(signalID);
		return it == m_counters.end() ? 0 : it->second;
	}

	void update(const FileCoverage& other) {
		assert(m_fileID == other.m_fileID);

		for(const auto& pair : other.m_counters) {
			addHitCount(pair.first, pair.second);
		}
	}

private:
	FileID m_fileID;

	// signal ID => hit counter
	std::map<signalid_t, std::size_t> m_counters;
};

/// \brief Represents an aggregate of FileCoverage data.
class CoverageData {
public:
	/*implicit*/ CoverageData() = default;

	const std::set<FileID>& getFiles() const { return m_files; }

	const FileCoverage* getFileCoverage(FileID fileID) const {
		auto it = m_data.find(fileID);
		return it == m_data.end() ? nullptr : &it->second;
	}

	std::size_t getHitCount(FileID fileID, signalid_t signalID) const {
		if(const FileCoverage* cov = getFileCoverage(fileID)) {
			return cov->getHitCount(signalID);
		}

		return 0;
	}

	void addFileCoverage(const FileCoverage& data);

	bool read(const std::string& filePath);

private:
	std::set<FileID> m_files;
	llvm::DenseMap<FileID, FileCoverage> m_data;
};

} // end namespace libmoocov

#endif // LIBMOOCOV_COVERAGEDATA_H
