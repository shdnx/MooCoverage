// RUN: test-instrumentation %s

void test(int arg) {
//% void test(int arg) {@;$;
	if(arg < 3) (void)0;
	//% if(arg < 3) {$;(void)0;}

	else if(arg == 3)
	//% else {$;if(arg == 3)
		(void)0;
		//% {$;(void)0;}

	else if(arg > 3) {}
	//% else {$;if(arg > 3) {$;}

	else {
	//% else {$;
		(void)0;
	}
	//% }}}
}
