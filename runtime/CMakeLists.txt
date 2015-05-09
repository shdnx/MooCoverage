set(SOURCES src/runtime.c)

set (PPDEFINITIONS "-D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS")
set (GCC_FLAGS "-std=c11 -Wall -Wextra -pedantic -Wno-strict-aliasing -Wno-unused-parameter -static -static-libgcc")
set (LINKER_FLAGS "-static -static-libgcc")

include_directories(include)

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
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")

add_library(moocovrt ${SOURCES})
