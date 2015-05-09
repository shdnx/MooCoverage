#include <fstream>

#include "libmoocov/utils/fastint.h"
#include "libmoocov/CoverageData.h"

namespace libmoocov {

void FileCoverage::setHitCount(signalid_t signalID, std::size_t count) {
	std::map<signalid_t, std::size_t>::iterator it;
	bool inserted;
	std::tie(it, inserted) = m_counters.insert(std::make_pair(signalID, count));

	if(!inserted) it->second = count;
}

std::size_t FileCoverage::addHitCount(signalid_t signalID, std::size_t count) {
	std::map<signalid_t, std::size_t>::iterator it;
	bool inserted;
	std::tie(it, inserted) = m_counters.insert(std::make_pair(signalID, count));

	if(!inserted) {
		return it->second += count;
	}
	return count;
}

void CoverageData::addFileCoverage(const FileCoverage& data) {
	bool inserted;
	llvm::DenseMap<FileID, FileCoverage>::iterator it;

	std::tie(it, inserted) = m_data.insert(std::make_pair(data.getFileID(), data));

	if(inserted) m_files.insert(data.getFileID());
	else it->second.update(data);
}

bool CoverageData::read(const std::string& filePath) {
	std::ifstream fs{filePath.c_str()};
	if(!fs) return false;

	FileID fileID;
	signalid_t signalID;
	unsigned hitCount;

	std::string buffer;
	while(fs >> buffer) {
		fileID = FileID::parse(buffer);
		FileCoverage cov{fileID};

		while(fs >> buffer && buffer != ";") {
			utils::parseFastInteger(buffer, signalID);

			fs >> buffer;
			utils::parseFastInteger(buffer, hitCount);

			cov.setHitCount(signalID, hitCount);
		}

		addFileCoverage(cov);
	}

	fs.close();
	return true;
}

} // end namespace libmoocov
