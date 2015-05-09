#ifndef LIBMOOCOV_UTILS_FASTINT_H
#define LIBMOOCOV_UTILS_FASTINT_H

#include <cassert>

#include "llvm/ADT/StringRef.h"

namespace libmoocov {
namespace utils {

template<typename T>
bool tryParseFastInteger(llvm::StringRef str, T& n) {
	n = 0;
	T p = 1;

	for(char c : str) {
		if('0' <= c && c <= '9') {
			n += p * (c - '0');
		} else if('A' <= c && c <= 'F') {
			n += p * (10 + c - 'A');
		} else {
			return false;
		}

		p *= 16;
	}

	return true;
}

template<typename T>
inline T& parseFastInteger(llvm::StringRef str, T& n) {
	assert(tryParseFastInteger(str, n));
	return n;
}

} // end namespace utils
} // end namespace libmoocov

#endif // LIBMOOCOV_UTILS_FASTINT_H
