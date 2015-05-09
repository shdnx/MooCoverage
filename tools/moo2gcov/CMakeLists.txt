include(CMakeSourceLists.txt)

set (PPDEFINITIONS "-D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS")
set (GCC_FLAGS "-Wall -Wextra -pedantic -Wno-strict-aliasing -Wno-unused-parameter -std=c++11 -fno-rtti -fsanitize=address -fno-omit-frame-pointer")
set (LINKER_FLAGS "-fsanitize=address")
# -static -static-libgcc

set (LIBS LLVMSupport pthread dl tinfo moocov)

include_directories(include ${LLVM_INCLUDE_DIR} ${LIBMOOCOV_INCLUDE_DIR})
link_directories (${LLVM_LIB_DIR})

if (ARCH STREQUAL "32")
  set (GCC_FLAGS "${GCC_FLAGS} -m32")
  set (LINKER_FLAGS "${LINKER_FLAGS} -m32")
endif ()

if (DEBUG STREQUAL "YES")
  set (GCC_FLAGS "${GCC_FLAGS} -g -O0")
  set (PPDEFINITIONS "${PPDEFINITIONS} -D_DEBUG")
else ()
  set (GCC_FLAGS "${GCC_FLAGS} -O2 -s")
endif ()

add_definitions (${PPDEFINITIONS})
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")

add_executable (moo2gcov ${SOURCES})
target_link_libraries (moo2gcov ${LIBS})
