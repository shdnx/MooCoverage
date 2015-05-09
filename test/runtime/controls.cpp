// RUN: rm -rf %t.d %t.runtime.o %t.exe
// RUN: moocov-instrument %s -o %t.d -m %t.d --
// RUN: build-runtime -DALLOW_DISABLE=1 -DINITIAL_ENABLED=0 -o %t.runtime.o
// RUN: %cxx -w %t.d/controls.cpp %t.runtime.o -I%runtime_incl -o %t.exe
// RUN: test-coverage %s %t.d -- %t.exe x y

int main(int argc, const char** argv) {
#ifdef MOOCOV
	moocov_enable();
#endif

	for(int i = 0; i < argc; ++i) {} // TAKEN: 3

#ifdef MOOCOV
	moocov_disable();
#endif

	if(argc > 1) {} // TAKEN: 0

#ifdef MOOCOV
	moocov_dump();
#endif

	return 0;
}
