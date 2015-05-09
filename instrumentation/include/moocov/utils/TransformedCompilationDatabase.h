#ifndef MOOCOV_UTILS_MODIFIEDCOMPDB
#define MOOCOV_UTILS_MODIFIEDCOMPDB

#include <functional>
#include <vector>
#include <string>

#include "llvm/ADT/StringRef.h"

#include "clang/Tooling/Tooling.h"

/// \brief Provides a simple wrapper around a CompilationDatabase, allowing the user to modify the individual CompileCommands with a callback function.
struct TransformedCompilationDatabase : clang::tooling::CompilationDatabase {
	using callback_t = void(clang::tooling::CompileCommand&);

	explicit TransformedCompilationDatabase(const clang::tooling::CompilationDatabase& base, std::function<callback_t> f)
		: m_base(base), m_transform{f} {}

	const clang::tooling::CompilationDatabase& getInner() const {
		return m_base;
	}

	std::vector<clang::tooling::CompileCommand> getCompileCommands(llvm::StringRef file) const override {
		return _getModifiedCompileCommands(m_base.getCompileCommands(file));
	}

	std::vector<clang::tooling::CompileCommand> getAllCompileCommands() const override {
		return _getModifiedCompileCommands(m_base.getAllCompileCommands());
	}

	std::vector<std::string> getAllFiles() const override {
		return m_base.getAllFiles();
	}

private:
	std::vector<clang::tooling::CompileCommand> _getModifiedCompileCommands(std::vector<clang::tooling::CompileCommand>&& result) const {
		for(clang::tooling::CompileCommand& cmd : result) {
			m_transform(cmd);
		}

		return std::move(result);
	}

	const clang::tooling::CompilationDatabase& m_base;
	std::function<callback_t> m_transform;
};

#endif // MOOCOV_UTILS_MODIFIEDCOMPDB
