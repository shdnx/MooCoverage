// RUN: test-instrumentation %s -std=c++11

void declared(); // just a declaration, should be ignored
void declared() {} // definitions are, of course, instrumented normally
//% void declared() {@;$;}

// this class has an explicitly specified implicit default constructor and an implicit e.g. copy constructor
// all of that should be ignored by the instrumentor
struct ImplicitCtor {
	ImplicitCtor() = default;
};

// constexpr functions are ignored
constexpr void constexpr_func() {}
