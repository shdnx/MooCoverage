#ifndef MOOCOV_INSTRUMENTATION_H
#define MOOCOV_INSTRUMENTATION_H

#include "llvm/ADT/SmallString.h"

#include <string>
#include <set>

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"

#include "clang/AST/Type.h"
#include "clang/AST/ASTContext.h"

#include "moocov/SignalRegistry.h"
#include "moocov/utils/SourceFileRef.h"
#include "moocov/utils/FileRewriter.h"

namespace clang {

class LangOptions;

class FunctionDecl;
class Stmt;

} // end namespace clang

namespace moocov {

class Block;
class InstrumentationContext;
class InstrumentationOptions;

// TODO: the parts that actually do the instrumentation (source code rewriting) should be moved to a different class, to encapsulate the dependency on the runtime interface

class FileInstrumentation {
private:
	bool _emitInstrumentationHeader();
	std::string _getSignalInstrumentation(const Signal* sig, bool asStmt) const;

	std::string _makeSignal(const Block* block);
	std::string _makeImplicitSignal(const clang::CharSourceRange& range);

	void _instrumentStmtBlock(const Block* block, const InstrumentationContext& context);
	void _instrumentExprBlock(const Block* exprBlock, const InstrumentationContext& context);

public:
	explicit FileInstrumentation(const InstrumentationOptions& options, utils::SourceFileRef sourceFile, FileInstrumentation* parent, const clang::ASTContext& astContext);

	const utils::SourceFileRef& getSourceFile() const { return m_sourceFile; }
	const clang::FileID getSourceFileID() const { return m_sourceFile.getFileID(); }

	const InstrumentationOptions& getOptions() const { return m_options; }
	const SignalRegistry& getSignalRegistry() const { return m_signals; }

	bool hasSignals() const { return !m_signals.empty(); }

	FileInstrumentation* getParent() const { return m_parent; }

	bool tryBeginFunction(const clang::FunctionDecl* func);
	void endFunction(const clang::FunctionDecl* func);

	void beginBlock(const Block* block, const InstrumentationContext& context);
	void endBlock(const Block* block, const InstrumentationContext& context);

	void handleJumpStmt(const clang::Stmt* stmt, const InstrumentationContext& context);

	bool redirectInclude(clang::FileID includedFileID, llvm::StringRef newFilePath);

	void finalize();

private:
	bool _outputInstrumentedSource(llvm::StringRef outputFilename);
	bool _outputSignals(llvm::StringRef outputFilename);

	const InstrumentationOptions& m_options;
	utils::SourceFileRef m_sourceFile;
	FileInstrumentation* m_parent;
	const clang::ASTContext& m_astContext;

	utils::FileRewriter m_rewriter;
	SignalRegistry m_signals;
};

} // end namespace moocov

#endif // MOOCOV_INSTRUMENTATION_H
