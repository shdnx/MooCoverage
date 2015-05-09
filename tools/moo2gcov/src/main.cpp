#include <string>
#include <system_error>
#include <tuple>

#include "llvm/ADT/StringMap.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"

#include "libmoocov/CoverageMap.h"
#include "libmoocov/CoverageData.h"

#include "moo2gcov/ConverterOptions.h"
#include "moo2gcov/GcovWriter.h"

using namespace llvm;
using namespace moo2gcov;

static cl::list<std::string> g_inputFiles{
	cl::Positional,
	cl::desc("<input files>"),
	cl::OneOrMore
};

static cl::opt<std::string> g_outputPath{"o",
	cl::desc("Directory to place the output .gcov files in"),
	cl::value_desc("path"),
	cl::Required
};

static cl::opt<bool> g_omitUnexecutedFiles{"omit-unexecuted",
	cl::desc("Whether to not generate output for files that do not contain any executed lines"),
	cl::init(false)
};

static cl::opt<bool> g_emitSimpleHitCount{"simple-hitcount",
	cl::desc("Whether to just emit a 0 as a hit counter for unexecuted lines, instead of the ##### or ===== normally produced by GCOV"),
	cl::init(false)
};

static bool populateOptions(ConverterOptions& opts) {
	opts.outputDirectory = g_outputPath;
	opts.omitUnexecutedFiles = g_omitUnexecutedFiles;
	opts.emitSimpleHitCount = g_emitSimpleHitCount;

	if(!opts.outputToStdout()) {
		if(llvm::sys::fs::exists(opts.outputDirectory)) {
			if(!llvm::sys::fs::is_directory(opts.outputDirectory)) {
				llvm::errs() << "Output path '" << opts.outputDirectory << "' already exists, but is not a directory!\n";
				return false;
			}

			return true;
		}

		std::error_code error = llvm::sys::fs::create_directory(opts.outputDirectory);
		if(error) {
			llvm::errs() << "I/O error: failed to create output directory '" << opts.outputDirectory << "': " << error.message() << " (code: " << error.value() << ")\n";
			return false;
		}
	}

	return true;
}

static bool isDataFile(llvm::StringRef fileName) {
	return fileName.endswith(".mocd");
}

static bool isMapFile(llvm::StringRef fileName) {
	return fileName.endswith(".mocm");
}

int main(int argc, const char** argv) {
	cl::ParseCommandLineOptions(argc, argv);

	// process the converter options
	ConverterOptions opts;
	if(!populateOptions(opts)) {
		return 1;
	}

	llvm::StringMap<libmoocov::SourceFileMap> sourceMaps;
	libmoocov::CoverageData coverageData;

	// process the input files
	for(const std::string& inputFilePath : g_inputFiles) {
		if(isDataFile(inputFilePath)) {
			if(!coverageData.read(inputFilePath)) {
				llvm::errs() << "Warning: failed to read data file '" << inputFilePath << "', skipping.\n";
			}
		} else if(isMapFile(inputFilePath)) {
			libmoocov::SignalMap signalMap;
			if(!signalMap.read(inputFilePath)) {
				llvm::errs() << "Error: failed to read map file '" << inputFilePath << "'!\n";
				return 2;
			}

			// register the signals from this SignalMap in the appropriate SourceFileMap
			auto it = sourceMaps.find(signalMap.sourceFilePath);
			if(it == sourceMaps.end()) {
				std::tie(it, std::ignore) = sourceMaps.insert(std::make_pair(signalMap.sourceFilePath, libmoocov::SourceFileMap{}));
			}

			it->second.addSignalMap(signalMap);
		} else {
			llvm::errs() << "Warning: unrecognized file extension for input file '" << inputFilePath << "' - file ignored.\n";
		}
	}

	// run GcovWriter for each SourceFileMap
	for(const auto& sourceMapEntry : sourceMaps) {
		GcovWriter writer{sourceMapEntry.second, coverageData, opts};
		if(!writer.run()) {
			llvm::errs() << "An error occured while generating the output file for source file '" << sourceMapEntry.second.getSourcePath() << "', continuing.\n";
		}
	}

	return 0;
}
