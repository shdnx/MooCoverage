// RUN: test-instrumentation %s
int x;

#include "include-instrumented.h"
//#

void foo();

#define EMPTY
#include "include-empty.h"
