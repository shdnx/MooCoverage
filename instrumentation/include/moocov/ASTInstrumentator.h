#ifndef MOOCOV_ASTINSTRUMENTOR_H
#define MOOCOV_ASTINSTRUMENTOR_H

namespace clang {

class ASTUnit;

} // end namespace clang

namespace moocov {

class InstrumentationOptions;

class ASTInstrumentor {
public:
	explicit ASTInstrumentor(clang::ASTUnit& AST, const InstrumentationOptions& options)
		: m_AST(AST), m_opts(options) {}

	void run();

private:
	clang::ASTUnit& m_AST;
	const InstrumentationOptions& m_opts;
};

} // end namespace moocov

#endif // MOOCOV_ASTINSTRUMENTOR_H
