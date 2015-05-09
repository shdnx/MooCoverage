// RUN: rm -rf %t.d %t.exe
// RUN: moocov-instrument %s -o %t.d --
// RUN: %cxx -w %t.d/simple.cpp %runtime_lib -I%runtime_incl -o %t.exe
// RUN: test-coverage %s %t.d -- %t.exe csiga biga

int foo(int c) {
	return c > 0
		? (c > 2) // TAKEN: 1
		: (c - 1); // TAKEN: 0
}

int main(int argc, const char** argv) {
	int result = 0;
	if(foo(argc)
		&& argc % 2 == 0) // TAKEN: 1
	{ // TAKEN: 0
		result = 1;
	}

#ifdef MOOCOV
	moocov_dump();
#endif
	return result;
}
