#ifndef MOOCOV_RUNTIME_LIKELY_H
#define MOOCOV_RUNTIME_LIKELY_H

// NOTE: This is based on LLVM 3.6's include/llvm/Support/Compiler.h's LLVM_LIKELY and LLVM_UNLIKELY implementation.

#ifndef __has_builtin
# define __has_builtin(x) 0
#endif

#ifndef MOOCOV_GNUC_PREREQ
# if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#  define MOOCOV_GNUC_PREREQ(maj, min, patch) \
    ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) + __GNUC_PATCHLEVEL__ >= \
     ((maj) << 20) + ((min) << 10) + (patch))
# elif defined(__GNUC__) && defined(__GNUC_MINOR__)
#  define MOOCOV_GNUC_PREREQ(maj, min, patch) \
    ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) >= ((maj) << 20) + ((min) << 10))
# else
#  define MOOCOV_GNUC_PREREQ(maj, min, patch) 0
# endif
#endif

#if __has_builtin(__builtin_expect) || MOOCOV_GNUC_PREREQ(4, 0, 0)
#define MOOCOV_LIKELY(EXPR) __builtin_expect((EXPR), 1)
#define MOOCOV_UNLIKELY(EXPR) __builtin_expect((EXPR), 0)
#else
#define MOOCOV_LIKELY(EXPR) (EXPR)
#define MOOCOV_UNLIKELY(EXPR) (EXPR)
#endif

#endif // MOOCOV_RUNTIME_LIKELY_H
