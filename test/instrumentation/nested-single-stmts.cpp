// RUN: test-instrumentation %s

void test(int x) {
//% void test(int x) {@;$;
	while(x > 5) if(x == 5)
	//% while(x > 5) {$;if(x == 5)
		for(int i = 0; i < x; i++) (void)0;
		//% {$;for(int i = 0; i < x; i++) {$;(void)0;}}}
}
