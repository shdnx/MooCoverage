#ifndef MOOCOV_UTILS_STRING_H
#define MOOCOV_UTILS_STRING_H

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"

#define BUILD_STR(NAME, SIZE) \
	llvm::SmallString<SIZE> NAME; \
	llvm::raw_svector_ostream{NAME}

#endif // MOOCOV_UTILS_STRING_H
