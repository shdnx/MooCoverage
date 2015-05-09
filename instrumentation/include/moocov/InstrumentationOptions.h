#ifndef MOOCOV_INSTRUMENTATIONOPTIONS_H
#define MOOCOV_INSTRUMENTATIONOPTIONS_H

#include <string>
#include <set>

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"

#include "moocov/utils/SourceFileRef.h"

namespace moocov {

class InstrumentationOptions {
public:
	/// \brief The path to the directory where the instrumented source files should be stored.
	/// If this option is "-", any output goes to stdout.
	std::string outputDirectory;

	/// \brief The path to the directory where the map files should be stored.
	/// If this option is "-", any output goes to stdout.
	std::string signalsOutputDirectory;

	bool omitSources;

	/// \brief If true, no signal map files should be output.
	bool omitSignals;

	bool autoDumpAtExit;
	std::set<std::string> excludedPaths;

	bool emitSources() const { return !omitSources; }
	bool emitSignals() const { return !omitSignals; }

	bool outputToStdout() const {
		return outputDirectory == "-";
	}

	bool outputSignalsToStdout() const {
		return signalsOutputDirectory == "-";
	}

	llvm::StringRef getOutputFilename(utils::SourceFileRef file, llvm::SmallVectorImpl<char>& buffer) const;

	bool isExcluded(llvm::StringRef path) const;
};

} // end namespace moocov

#endif // MOOCOV_INSTRUMENTATIONOPTIONS_H
