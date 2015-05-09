#include "libmoocov/LineCoverage.h"

namespace libmoocov {

bool LineCoverage::nextLine() {
	m_line++;

	// first, delete all signals whose coverage range we've left
	iterator it = m_lineSignals.begin(), endIt = m_lineSignals.end();
	while(it != endIt) {
		if(it->sourceRange.end.line == m_line)
			it = m_lineSignals.erase(it);
		else ++it;
	}

	// now add any new signals whose coverage range we've entered
	it = m_signals.begin(), endIt = m_signals.end();
	while(it != endIt) {
		if(it->sourceRange.begin.line == m_line) {
			m_lineSignals.insert(*it);
			it = m_signals.erase(it);
		} else ++it;
	}

	return !m_lineSignals.empty() || !m_signals.empty();
}

LineCoverage::range LineCoverage::getLineSpecificSignals() const {
	iterator it = m_lineSignals.end();
	if(it == m_lineSignals.begin()) return { it, it };

	do {
		auto prevIt = std::prev(it);
		if(prevIt->sourceRange.begin.line != m_line)
			return { it, m_lineSignals.end() };

		it = prevIt;
	} while(it != m_lineSignals.begin());

	// apparently all signals are line-specific
	return { m_lineSignals.begin(), m_lineSignals.end() };
}

LineCoverage::range LineCoverage::getMostSpecificSignals() const {
	if(!isLineCovered()) return { m_lineSignals.begin(), m_lineSignals.end() };

	LineCoverage::range lineSpecificSignals = getLineSpecificSignals();
	if(lineSpecificSignals.begin() != lineSpecificSignals.end()) {
		return lineSpecificSignals;
	}

	return { std::prev(m_lineSignals.end()), m_lineSignals.end() };
}

} // end namespace libmoocov
