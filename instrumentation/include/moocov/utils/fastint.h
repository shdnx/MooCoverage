#ifndef MOOCOV_UTILS_FASTINT_H
#define MOOCOV_UTILS_FASTINT_H

#include "llvm/Support/raw_ostream.h"

namespace moocov {
namespace utils {

struct FastIntWriterManip {};
extern const FastIntWriterManip fastInt;

/// \brief Proxy class used to write unsigned integers to an llvm::raw_ostream.
/// Usage: llvm::errs() << moocov::utils::fastInt << 42;
///
/// The integer will be written as an uppercase hexadecimal string with the digits in a reverse order, for efficiency.
/// This is the same as how the instrumentation dumps the numbers into the data files, so all numbers produced by moocov are written in the same format.
class FastIntWriter {
public:
	using manip_t = decltype((fastInt));

	explicit FastIntWriter(llvm::raw_ostream& os) : m_os(&os) {}

	template<typename T>
	llvm::raw_ostream& operator<<(T n) {
		llvm::raw_ostream& os = *m_os;

		if(n == 0) return os << '0';

		for(; n > 0; n >>= 4) {
			os << "0123456789ABCDEF"[n & 0xF];
		}

		return os;
	}

private:
	llvm::raw_ostream* m_os;

	friend FastIntWriter operator<<(llvm::raw_ostream& os, manip_t);
};

inline FastIntWriter operator<<(llvm::raw_ostream& os, FastIntWriter::manip_t) {
	return FastIntWriter{os};
}

} // end namespace utils
} // end namespace moocov

#endif // MOOCOV_UTILS_FASTINT_H
