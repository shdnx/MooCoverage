#ifndef MOOCOV_RUNTIME_INTERFACE_H
#define MOOCOV_RUNTIME_INTERFACE_H

// Exposes a public API to be used by programs instrumented with MooCoverage.

#define MOOCOV

#ifdef __cplusplus
#define MOOCOV_EXTERN_C extern "C"
#else
#define MOOCOV_EXTERN_C extern
#endif

MOOCOV_EXTERN_C void moocov_enable();
MOOCOV_EXTERN_C void moocov_disable();
MOOCOV_EXTERN_C void moocov_reset();
MOOCOV_EXTERN_C void moocov_dump();

#endif // MOOCOV_RUNTIME_INTERFACE_H
