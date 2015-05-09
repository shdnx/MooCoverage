#include <fstream>
#include <utility>

#include "libmoocov/utils/fastint.h"
#include "libmoocov/CoverageMap.h"

namespace libmoocov {

bool SignalMap::read(const std::string& filePath) {
	std::ifstream fs{filePath.c_str()};
	if(!fs) return false;

	unsigned numSignals;
	SignalMapping signal;

	std::string buffer;
	while(fs >> buffer) {
		fileID = FileID::parse(buffer);

		// the rest of the line is the original source file path
		std::getline(fs >> std::ws, sourceFilePath);

		fs >> buffer;
		utils::parseFastInteger(buffer, numSignals);

		for(unsigned i = 0; i < numSignals; ++i) {
			signal.fileID = fileID;

			fs >> buffer;
			utils::parseFastInteger(buffer, signal.id);

			fs >> buffer;
			utils::parseFastInteger(buffer, signal.sourceRange.begin.line);

			fs >> buffer;
			utils::parseFastInteger(buffer, signal.sourceRange.begin.column);

			fs >> buffer;
			utils::parseFastInteger(buffer, signal.sourceRange.end.line);

			fs >> buffer;
			utils::parseFastInteger(buffer, signal.sourceRange.end.column);

			add(signal);
		}
	}

	fs.close();
	return true;
}

} // end namespace libmoocov
