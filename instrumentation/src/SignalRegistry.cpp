#include <tuple>

#include "llvm/Support/raw_ostream.h"

#include "clang/AST/Stmt.h"

#include "moocov/utils/fastint.h"
#include "moocov/SignalRegistry.h"

using namespace clang;

namespace moocov {

Signal Signal::create(id_t id, const SourceManager& sources, const CharSourceRange& range, bool isImplicit, bool isExceptional) {
	assert(range.isValid());
	assert(range.isCharRange());

	return Signal{id,
		FullSourceLoc{range.getBegin(), sources},
		FullSourceLoc{range.getEnd(), sources},
		isImplicit,
		isExceptional
	};
}

const Signal* SignalRegistry::createSignal(const CharSourceRange& coveredRange, bool isImplicit, bool isExceptional) {
	if(coveredRange.isInvalid()) return nullptr;

	Signal::id_t id = m_nextId++;

	decltype(m_map)::iterator it;
	bool ok;
	std::tie(it, ok) = m_map.insert(std::make_pair(id, Signal::create(id, m_sourceFile.getSourceManager(), coveredRange, isImplicit, isExceptional)));
	assert(ok && "Overflowing signal ID?");

	return &(it->second);
}

bool SignalRegistry::writeTo(llvm::raw_ostream& os) const {
	if(empty()) return false;

	using moocov::utils::fastInt;

	os << m_sourceFile.getID() << " "
		<< m_sourceFile.getFilePath() << "\n"
		<< fastInt << m_map.size() << "\n";

	for(const auto& it : m_map) {
		const Signal& signal = it.second;

		os << fastInt << signal.getIndex() << " "
			<< fastInt << signal.getBeginLoc().getExpansionLineNumber() << " "
			<< fastInt << signal.getBeginLoc().getExpansionColumnNumber() << " "
			<< fastInt << signal.getEndLoc().getExpansionLineNumber() << " "
			<< fastInt << signal.getEndLoc().getExpansionColumnNumber() << "\n";
	}

	return true;
}

} // end namespace moocov
