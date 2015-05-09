# Uses llvm-config to find the LLVM sources and binaries.
# Sets the following variables:
# - LLVM_SRC_ROOT: the root directory of the LLVM sources
# - LLVM_OBJ_ROOT: the root directory of the LLVM and Clang binaries
# - LLVM_LIB_DIR: the directories containing the LLVM libraries
# - LLVM_INCLUDE_DIR: the directories containing the LLVM headers
# - CLANG_INCLUDE_DIR: the directories containing the Clang headers

execute_process(
	COMMAND llvm-config --src-root
	OUTPUT_VARIABLE LLVM_SRC_ROOT
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Found LLVM at ${LLVM_SRC_ROOT}")

execute_process(
	COMMAND llvm-config --obj-root
	OUTPUT_VARIABLE LLVM_OBJ_ROOT
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Found LLVM binaries at ${LLVM_OBJ_ROOT}")

execute_process(
	COMMAND llvm-config --includedir
	OUTPUT_VARIABLE LLVM_CORE_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
	COMMAND llvm-config --libdir
	OUTPUT_VARIABLE LLVM_LIB_DIR
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(LLVM_INCLUDE_DIR ${LLVM_CORE_INCLUDE_DIR} ${LLVM_OBJ_ROOT}/include)
set(CLANG_INCLUDE_DIR ${LLVM_SRC_ROOT}/tools/clang/include ${LLVM_OBJ_ROOT}/tools/clang/include)
