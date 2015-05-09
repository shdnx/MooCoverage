#include <iostream>
#include <iomanip>
#include <string>
#include <cctype>
#include <cassert>

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Path.h"

#include "moo2gcov/GcovWriter.h"

namespace moo2gcov {

GcovWriter::GcovWriter(const libmoocov::SourceFileMap& sourceMap, const libmoocov::CoverageData& coverageData, const ConverterOptions& opts)
	: m_sourceFileMap(sourceMap), m_coverageData(coverageData), m_opts(opts), m_lineCoverage{m_sourceFileMap.getSignals()} {}

bool GcovWriter::run() {
	const std::string& sourcePath = m_sourceFileMap.getSourcePath();
	m_sourceStream.reset(new std::ifstream{sourcePath.c_str()});
	if(!m_sourceStream->good()) {
		llvm::errs() << "I/O error: failed to open original source file '" << sourcePath << "'!\n";

		return false;
	}

	std::string outputFile = llvm::sys::path::filename(sourcePath).str() + ".gcov";

	if(m_opts.outputToStdout()) {
		std::cout << "$$File: " << outputFile << std::ends << std::endl;

		_runInternal(std::cout);
	} else {
		std::string outputPath = m_opts.outputDirectory + "/" + outputFile;
		std::ofstream fs{outputPath.c_str()};
		if(!fs) {
			llvm::errs() << "I/O error: failed to open output file '" << outputPath << "'!\n";
			return false;
		}

		_runInternal(fs);

		fs.close();
	}

	return true;
}

void GcovWriter::_emitPreambleTag(std::ostream& os, const char* tag, const std::string& value) {
	os << std::setw(fieldWidth) << std::right << "-";
	os << ":" << std::setw(fieldWidth) << std::right << "0";
	os << ":" << tag << ":" << value << std::endl;
}

void GcovWriter::_emitPreamble(std::ostream& os) {
	_emitPreambleTag(os, "Source", llvm::sys::path::filename(m_sourceFileMap.getSourcePath()).str());
}

bool GcovWriter::_nextLine(std::string& sourceLine) {
	m_lineCoverage.nextLine();

	std::getline(*m_sourceStream, sourceLine);
	return m_sourceStream->good();
}

static bool isEmptyLine(const std::string& line) {
	for(char c : line) {
		if(!std::isspace(c) && c != ';' && c != '{' && c != '}') return false;
	}

	return true;
}

void GcovWriter::_emitCode(std::ostream& os) {
	std::string sourceLine;
	while(_nextLine(sourceLine)) {
		if(!m_lineCoverage.isLineCovered() || isEmptyLine(sourceLine)) {
			os << std::setw(fieldWidth) << std::right << "-";
		} else {
			std::size_t hitCount = 0;
			for(const libmoocov::SignalMapping& coveringSignal : m_lineCoverage.getMostSpecificSignals()) {
				hitCount = std::max(hitCount, m_coverageData.getHitCount(coveringSignal.fileID, coveringSignal.id));
			}

			if(hitCount == 0 && !m_opts.emitSimpleHitCount) {
				// NOTE: in the gcov format, we should use '=' instead of '#' if the line is only reachable in an exceptional branch (i.e. in the C++ catch block), but we don't store that information
				os << std::string(fieldWidth, '#');
			} else {
				os << std::setw(fieldWidth) << std::right << hitCount;
			}
		}

		os << ":" << std::setw(fieldWidth) << std::right << m_lineCoverage.getCurrentLine();
		os << ":" << sourceLine << std::endl;
	}
}

} // end namespace moo2gcov
