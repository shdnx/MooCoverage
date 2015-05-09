#ifndef MOO2GCOV_GCOVWRITER_H
#define MOO2GCOV_GCOVWRITER_H

#include <fstream>
#include <memory>

#include "libmoocov/CoverageMap.h"
#include "libmoocov/CoverageData.h"
#include "libmoocov/LineCoverage.h"

#include "moo2gcov/ConverterOptions.h"

namespace moo2gcov {

class GcovWriter {
public:
	explicit GcovWriter(const libmoocov::SourceFileMap& sourceMap, const libmoocov::CoverageData& coverageData, const ConverterOptions& opts);

	bool run();

private:
	void _runInternal(std::ostream& os) {
		_emitPreamble(os);
		_emitCode(os);

		m_sourceStream->close();
	}

	void _emitPreambleTag(std::ostream& os, const char* tag, const std::string& value);
	void _emitPreamble(std::ostream& os);

	bool _nextLine(std::string& sourceLine);
	void _emitCode(std::ostream& os);

	const libmoocov::SourceFileMap& m_sourceFileMap;
	const libmoocov::CoverageData& m_coverageData;
	const ConverterOptions& m_opts;

	libmoocov::LineCoverage m_lineCoverage;
	std::unique_ptr<std::ifstream> m_sourceStream;

	static const std::size_t fieldWidth = 5;
};

} // end namespace moo2gcov

#endif // MOO2GCOV_GCOVWRITER_H
