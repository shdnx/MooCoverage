MooCoverage
=========================

MooCoverage is a simple source-code instrumentation tool for C/C++, primarily targeting measurement of test coverage. It has been developed as my Bachelor's thesis.

	 __________________________________
	< Our official test coverage tool! >
	 ----------------------------------
	        \   ^__^
	         \  (oo)\_______
	            (__)\       )\/\
	                ||----w |
	                ||     ||

**Warning**: This is a very alpha-version tool, with numerous known bugs and limitations. It is (currently) not recommended to be used in its current form as a serious test-coverage solution.

MooCoverage is based on [LLVM](http://llvm.org) and [Clang](http://clang.llvm.org) version 3.7 (note that Clang 3.6 needs to be patched with *clang36_patch.diff*). It uses Clang to parse the source code to be instrumented and build an AST - then it performs the instrumentation based on the AST. It also uses some LLVM libraries, most notably for command line handling.

If you read Hungarian, the best resource for getting acquainted with the project is reading my [BSc thesis](http://gaborkozar.me/szakdolgozat.pdf).

The project uses the MIT license: you can do whatever you wish with it, so long as you don't hold me liable and you give credit where credit is due.

Configuration and building
--------------------------------------------------------------

Configuration is done simply using CMake. Make sure that the LLVM binaries are in your PATH: `llvm-config` is invoked during the configuration. Use GCC 4.8 or later to compile.

	mkdir build
	cd build
	cmake ..
	make

The executables will be placed in the *bin*, the libraries in *lib* directories.

Usage
------------------------

*moocov-instrument* is used to instrument C/C++ sources. It requires a compilation database that it can auto-detect from the command line or from the source directory. For more information, refer to the [Clang tooling guide](http://clang.llvm.org/docs/HowToSetupToolingForLLVM.html).

*moocov-instrument* outputs instrumented C/C++ sources and transformation map files as output. When compiling the instrumented sources, you need to link to the moocov runtime (*libmoocovrt*).

The "instrumented binary" (the binary resulting from compiling the instrumented sources) can be run and used as normal, and will produce a *coverage.mocd* file in the current working directory when `moocov_dump()` is called (or automatically upon exit, if the `--auto-dump` option was provided to *moocov-instrument*).

The map and data files produced are text files, and their format is very simple. The only notable thing about them is that all numbers are written out as hexadecimal numbers with their digits reversed (see *runtime/include/moocovrt/fastint.h*).
See *lib/src/CoverageData.cpp* and *lib/src/CoverageMap.cpp* for details.

Currently there's only one tool that can act on the generated map and data files: this is the *moo2gcov* tool that converts these to .gcov files.

Limitations, bugs
-------------------------------------

moocov-instrument

* goto labels and switch cases are not handled well (see the failing test cases instrumentation/labels.cpp and maps/labels.cpp)
* function calls are not treated as potential exit points like they should be (it's tricky to do this right - see instrumentation/src/FileInstrumentation.cpp `FileInstrumentation::handleJumpStmt()`)
* C++11 constexpr functions are ignored (they should be duplicated into a non-constexpr version that can get instrumented properly)
* `moocov_enable()` should also count as a signal
* total lack of support for macros (we should probably only work on preprocessed files)
* eliminate redundant signals
* merge signals whenever possible, like Clang's profiling instrumentation (e.g. in if-then-else cases)
* more configuration options (filters, courseness of instrumentation, etc.)

moo2gcov

* the hit counters per line are often wrong - it's difficult to figure out how to measure these correctly when there can multiple coverage ranges in a single line

runtime

* support multi-threaded applications
* more configuration options
* performance improvements (e.g. inline `_moocov_signal()`)
