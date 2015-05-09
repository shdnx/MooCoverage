#ifndef MOOCOV_SIGNALREGISTRY_H
#define MOOCOV_SIGNALREGISTRY_H

#include <cassert>

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/DenseMap.h"

#include "clang/Basic/SourceManager.h"

#include "moocov/utils/SourceFileRef.h"

namespace clang {

class Stmt;

} // end namespace clang

namespace moocov {

class Signal {
public:
	using id_t = unsigned long long;

	static Signal create(id_t id, const clang::SourceManager& sources, const clang::CharSourceRange& coveredRange, bool isImplicit, bool isExceptional);

	/*implicit*/ Signal() : m_id{0} {}

	explicit Signal(id_t id, clang::FullSourceLoc begin, clang::FullSourceLoc end, bool isImplicit, bool isExceptional)
		: m_id{id}, m_begin{begin}, m_end{end}, m_isImplicit{isImplicit}, m_isExceptional{isExceptional} {
		assert(id != 0);
		assert(m_begin.isValid());
		assert(m_end.isValid());
	}

	bool isValid() const { return m_id != 0; }

	id_t getIndex() const {
		assert(isValid() && "Attempt to get index of invalid signal!");
		return m_id - 1;
	}

	clang::FullSourceLoc getBeginLoc() const { return m_begin; }
	clang::FullSourceLoc getEndLoc() const { return m_end; }

	clang::CharSourceRange getSourceRange() const {
		return clang::CharSourceRange::getCharRange(m_begin, m_end);
	}

	bool isImplicit() const { return m_isImplicit; }
	bool isExceptional() const { return m_isExceptional; }

private:
	id_t m_id;
	clang::FullSourceLoc m_begin, m_end;
	bool m_isImplicit, m_isExceptional;
};

class SignalRegistry {
public:
	explicit SignalRegistry(utils::SourceFileRef sourceFile, const clang::LangOptions& langOpts)
		: m_sourceFile{sourceFile}, m_langOpts(langOpts) {}

	const utils::SourceFileRef& getSourceFile() const { return m_sourceFile; }

	bool empty() const { return m_map.empty(); }
	std::size_t size() const { return m_map.size(); }

	const Signal* operator[](Signal::id_t id) const {
		auto it = m_map.find(id);
		return it != m_map.end() ? &it->second : nullptr;
	}

	const Signal* createSignal(const clang::CharSourceRange& coveredRange, bool isImplicit, bool isExceptional);

	bool writeTo(llvm::raw_ostream& os) const;

private:
	utils::SourceFileRef m_sourceFile;
	const clang::LangOptions& m_langOpts;
	Signal::id_t m_nextId = 1; // id 0 is reserved as invalid

	llvm::DenseMap<Signal::id_t, Signal> m_map;
};

} // end namespace moocov

#endif // MOOCOV_SIGNALREGISTRY_H
