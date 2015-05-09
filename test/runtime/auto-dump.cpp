// RUN: rm -rf %t.exe %t.d coverage.mocd
// RUN: moocov-instrument %s -o %t.d --omit-maps --auto-dump --
// RUN: %cxx -w %t.d/auto-dump.cpp %runtime_lib -I%runtime_incl -o %t.exe
// RUN: %t.exe
// RUN: file coverage.mocd

int main(int argc, const char** argv) {
	// no moocov_dump() on purpose
}
