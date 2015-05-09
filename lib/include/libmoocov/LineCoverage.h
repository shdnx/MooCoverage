#ifndef LIBMOOCOV_LINECOVERAGE_H
#define LIBMOOCOV_LINECOVERAGE_H

#include <utility>

#include "llvm/ADT/iterator_range.h"

#include "libmoocov/CoverageMap.h"

namespace libmoocov {

/// \brief Given a set of SignalMapping-s, provides an interface that allows the user to move line by line and get the SignalMapping-s that cover the current line.
/// This can be used to produce line-based coverage reports like the GCOV format.
struct LineCoverage {
	using iterator = SignalSet::const_iterator;
	using range = llvm::iterator_range<iterator>;

	explicit LineCoverage(const SignalSet& signals)
		: m_line{0}, m_signals{signals} {}

	explicit LineCoverage(SignalSet&& signals)
		: m_line{0}, m_signals{std::move(signals)} {}

	line_t getCurrentLine() const { return m_line; }

	/// \brief Gets a value indicating whether there are any signals remaining.
	bool isEmpty() const { return m_signals.empty(); }

	/// \brief Gets a value indicating whether there are any signals covering the current line.
	bool isLineCovered() const {
		return !m_lineSignals.empty();
	}

	/// \brief Gets the range of all signals covering the current line.
	const SignalSet& getCoveringSignals() const {
		return m_lineSignals;
	}

	/// \brief Gets the signal with the narrowest coverage that covers the current line.
	/// Returns nullptr if there are no signals covering the current line.
	const SignalMapping* getNarrowestCoveringSignal() const {
		return !m_lineSignals.empty()
			? &*std::prev(m_lineSignals.end())
			: nullptr;
	}

	/// \brief Gets the range of signals that begin on the current line.
	range getLineSpecificSignals() const;

	/// \brief Gets the range of signals that are most specific to the current line.
	///
	/// If getLineSpecificSignals() is non-empty, then this will return the same range.
	/// Otherwise it returns a range only containing getNarrowestCoveringSignal().
	range getMostSpecificSignals() const;

	/// \brief Advances the coverage meter to the next line.
	/// Returns false if there are no more signals to be processed; otherwise true.
	bool nextLine();

private:
	line_t m_line;
	SignalSet m_signals;
	SignalSet m_lineSignals;
};

} // end namespace libmoocov

#endif // LIBMOOCOV_LINECOVERAGE_H
