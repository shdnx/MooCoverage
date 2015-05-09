#ifndef MOO2GCOV_CONVERTEROPTIONS_H
#define MOO2GCOV_CONVERTEROPTIONS_H

#include <string>

namespace moo2gcov {

class ConverterOptions {
public:
	std::string outputDirectory;
	bool omitUnexecutedFiles;
	bool emitSimpleHitCount;

	bool outputToStdout() const {
		return outputDirectory == "-";
	}
};

} // end namespace moo2gcov

#endif // MOO2GCOV_CONVERTEROPTIONS_H
