#include <memory>
#include <vector>
#include <system_error>
#include <string>
#include <utility>
#include <cstdlib>

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/ASTUnit.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "moocov/utils/TransformedCompilationDatabase.h"
#include "moocov/InstrumentationOptions.h"
#include "moocov/ASTInstrumentator.h"

using namespace llvm;

using namespace clang;
using namespace clang::tooling;

static cl::OptionCategory g_myToolCategory{"Tool options"};

static cl::opt<std::string> g_outputDir{"out-instrumented",
	cl::desc("Output directory for the instrumented sources"),
	cl::value_desc("directory"),
	cl::init("./"),
	cl::cat(g_myToolCategory)
};

static cl::alias g_outputDirA{"o",
	cl::desc("Alias for --out-instrumented"),
	cl::aliasopt(g_outputDir)
};

static cl::opt<std::string> g_signalsOutputDirectory{"out-maps",
	cl::desc("Output directory for the mapping files (if different from the directory for the instrumented sources)"),
	cl::value_desc("directory"),
	cl::Optional,
	cl::cat(g_myToolCategory)
};

static cl::alias g_signalsOutputDirectoryA{"m",
	cl::desc("Alias for --out-maps"),
	cl::aliasopt(g_signalsOutputDirectory)
};

static cl::list<std::string> g_excludes{"exclude",
	cl::desc("Paths to exclude"),
	cl::value_desc("path"),
	cl::ZeroOrMore,
	cl::cat(g_myToolCategory)
};

static cl::opt<bool> g_autoDumpAtExit{"auto-dump",
	cl::desc("Whether to automatically register moocov_dump() with atexit() in the main function"),
	cl::init(true),
	cl::cat(g_myToolCategory)
};

static cl::opt<bool> g_omitSignals{"omit-maps",
	cl::desc("Don't output mapping files"),
	cl::init(false),
	cl::cat(g_myToolCategory)
};

static cl::opt<bool> g_omitSources{"omit-sources",
	cl::desc("Don't output instrumented source files"),
	cl::init(false),
	cl::cat(g_myToolCategory)
};

static cl::extrahelp g_commonHelp{CommonOptionsParser::HelpMessage};

static bool _tryCreateDirectory(llvm::StringRef path) {
	if(llvm::sys::fs::exists(path)) {
		if(!llvm::sys::fs::is_directory(path)) {
			llvm::errs() << "Error: output path '" << path << "' already exists, but is not a directory!\n";
			return false;
		}

		return true;
	}

	std::error_code error = llvm::sys::fs::create_directory(path);
	if(error) {
		llvm::errs() << "I/O error: failed to create output directory '" << path << "': " << error.message() << " (code: " << error.value() << ")\n";
		return false;
	}

	return true;
}

static bool populateOptions(moocov::InstrumentationOptions& opts) {
	opts.outputDirectory = g_outputDir;
	opts.signalsOutputDirectory = g_signalsOutputDirectory;
	opts.omitSources = g_omitSources;
	opts.omitSignals = g_omitSignals;

	if(!opts.omitSignals && opts.signalsOutputDirectory.empty()) {
		opts.signalsOutputDirectory = opts.outputDirectory;
	}

	opts.autoDumpAtExit = g_autoDumpAtExit;

	// make any exclusion paths absolute
	for(const std::string& excl : g_excludes) {
		llvm::SmallString<64> tmp{excl};
		std::error_code error = llvm::sys::fs::make_absolute(tmp);
		if(error) {
			llvm::errs() << "Warning: failed to get absolute path to '" << excl << "' - skipping excluded path.\n";
		} else {
			opts.excludedPaths.insert(std::string(tmp.c_str(), tmp.size()));
		}
	}

	// create the output directories, if needed
	if(opts.emitSources() && !opts.outputToStdout()) {
		if(!_tryCreateDirectory(opts.outputDirectory)) return false;
	}

	if(opts.emitSignals() && !opts.outputSignalsToStdout()) {
		if(!_tryCreateDirectory(opts.signalsOutputDirectory)) return false;
	}

	return true;
}

int main(int argc, const char** argv) {
	CommonOptionsParser optionsParser{argc, argv, g_myToolCategory};

	// TODO: how to do this without having to use environment variables?
	/*char* rt = std::getenv("MOOCOV_RUNTIME_ROOT");
	if(!rt) {
		llvm::errs() << "Please set the MOOCOV_RUNTIME_ROOT environment variable first!\n";
		return 1;
	}

	llvm::SmallString<64> interfaceFilePath{rt};
	llvm::sys::path::append(interfaceFilePath, "include/moocov/interface.h");
	std::string interfaceFilePathStr = interfaceFilePath.str();*/

	std::string extraArgs[] = {
		//"-include",
		//interfaceFilePath.str()

		// NOTE: temporary fix
		"-DMOOCOV_INSTRUMENT"/*,
		"-Dmoocov_enable()=",
		"-Dmoocov_disable()=",
		"-Dmoocov_dump()=",
		"-Dmoocov_reset()="*/
	};
	const std::size_t numExtraArgs = sizeof(extraArgs) / sizeof(extraArgs[0]);

	TransformedCompilationDatabase compilationDb{
		optionsParser.getCompilations(),
		[&](CompileCommand& cmd) {
			cmd.CommandLine.insert(cmd.CommandLine.end(), extraArgs, extraArgs + numExtraArgs);
		}
	};

	ClangTool tool{compilationDb, optionsParser.getSourcePathList()};

	std::vector<std::unique_ptr<ASTUnit>> ASTs;
	int ASTBuildResult = tool.buildASTs(ASTs);
	if(ASTBuildResult != 0) {
		llvm::errs() << "Parse error, code " << ASTBuildResult << "!\n";
		return ASTBuildResult;
	}

	moocov::InstrumentationOptions instrOpts;
	if(!populateOptions(instrOpts)) {
		return 1;
	}

	for(const std::unique_ptr<ASTUnit>& AST : ASTs) {
		moocov::ASTInstrumentor{*AST, instrOpts}.run();
	}

	return 0;
}
