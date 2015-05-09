// RUN: rm -rf %t.d %t.exe
// RUN: moocov-instrument %s -o %t.d --exclude=%S/double-include.h --
// RUN: %cxx -w %t.d/double-include.cpp %runtime_lib -I%runtime_incl -I%S -o %t.exe
// RUN: test-coverage %s %t.d -- %t.exe

namespace first {

#define RETURN_TRUE
#include "double-include.h"

int test() {
	return included()
		? 1 // TAKEN: 1
		: 0; // TAKEN: 0
}

} // end namespace first

namespace second {

#undef RETURN_TRUE
#include "double-include.h"

int test() {
	return included()
		? 0 // TAKEN: 0
		: 1; // TAKEN: 1
}

} // end namespace second

int main(int argc, const char** argv) {
	int result = first::test() - second::test();

#ifdef MOOCOV
	moocov_dump();
#endif

	return result;
}
