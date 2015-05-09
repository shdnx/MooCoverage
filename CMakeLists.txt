cmake_minimum_required (VERSION 2.8)
project (MooCoverage)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

#To force build 32 bit: -DARCH="32"
if (NOT ARCH)
  set (ARCH "NATIVE")
endif ()

#To build for release: -DRELEASE="YES"
if (NOT RELEASE)
  set (DEBUG "YES")
else ()
  set (DEBUG "NO")
endif ()

# TODO: we could also just use FindLLVM.cmake
include(LLVM)

add_subdirectory(instrumentation)
add_subdirectory(lib)
add_subdirectory(runtime)
add_subdirectory(tools)
add_subdirectory(test)
