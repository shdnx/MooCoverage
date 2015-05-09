set(MOOCOV_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(MOOCOV_BUILD_DIR ${CMAKE_BINARY_DIR})

# TODO: how does this work???? how does llvm-lit know to pick up the configured in the build directory when run directly, not through make?
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
	${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
	@ONLY
)

# TODO: use AddLLVM.cmake's add_lit_testsuite() instead
enable_testing ()
add_test (NAME RegressionTests COMMAND llvm-lit ${CMAKE_CURRENT_SOURCE_DIR})

# set(CLANG_TEST_PARAMS
#   clang_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
#   )

# add_lit_testsuite(check-clang "Running the Clang regression tests"
#   ${CMAKE_CURRENT_BINARY_DIR}
#   PARAMS ${CLANG_TEST_PARAMS}
#   DEPENDS ${CLANG_TEST_DEPS}
#   ARGS ${CLANG_TEST_EXTRA_ARGS}
#   )