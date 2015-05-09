#ifndef MOOCOV_RUNTIME_FASTINT_H
#define MOOCOV_RUNTIME_FASTINT_H

#define RENDER_FAST_UINT_BODY(n, buff) \
	if(n == 0) { \
		buff[0] = '0'; \
		return 1; \
	} \
\
	unsigned int len = 0; \
\
	for(; n > 0; n >>= 4) { \
		buff[len++] = "0123456789ABCDEF"[n & 0xF]; \
	} \
\
	return len;

// Writes the given unsigned integer into the specified buffer as a (uppercase) hexadecimal string with the digits in a reverse order. This is very performance critical!
//
// The caller is responsible for ensuring that the given buffer is big enough - there are no just checks in place (for performance reasons)!
// Returns the number of characters written to the buffer.
unsigned int render_uint32(unsigned int n, char* buff) {
	RENDER_FAST_UINT_BODY(n, buff)
}

unsigned int render_uint64(unsigned long long n, char* buff) {
	RENDER_FAST_UINT_BODY(n, buff)
}

#undef RENDER_FAST_UINT_BODY

#endif // MOOCOV_RUNTIME_FASTINT_H
