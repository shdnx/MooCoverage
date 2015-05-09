#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#include "moocov/InstrumentationOptions.h"

using namespace moocov::utils;

namespace moocov {

llvm::StringRef InstrumentationOptions::getOutputFilename(SourceFileRef file, llvm::SmallVectorImpl<char>& buffer) const {
	SourceFileRef mainFile = file.getMainFile();

	llvm::SmallString<32> mainFileName{mainFile.getFileName()};

	if(mainFile != file) {
		llvm::sys::path::replace_extension(mainFileName, "");

		llvm::raw_svector_ostream{buffer}
			<< mainFileName
			<< "_f" << file.getID().getTransientID().getHashValue()
			<< llvm::sys::path::extension(file.getFileName());
	} else {
		buffer = mainFileName;
	}

	return llvm::StringRef{buffer.data(), buffer.size()};
}

bool InstrumentationOptions::isExcluded(llvm::StringRef path) const {
	for(const std::string& excl : excludedPaths) {
		if(path.startswith(excl)) return true;
	}

	return false;
}

} // end namespace moocov
