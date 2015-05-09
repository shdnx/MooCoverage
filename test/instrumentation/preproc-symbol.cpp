// RUN: test-instrumentation %s

// The MOOCOV_INSTRUMENT preprocessor symbol is expected to be defined during instrumentation.

#ifdef MOOCOV_INSTRUMENT

void moocov_guarded() {}
//% void moocov_guarded() {@;$;}

#endif // MOOCOV_INSTRUMENT

void unguarded() {}
//% void unguarded() {@;$;}
