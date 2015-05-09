// RUN: test-instrumentation %s
// XFAIL: *

// TODO: constexpr functions, operators, etc.

void external();

void test() {
//% void test() {@;$;
	external();
	//% external();$;
}
