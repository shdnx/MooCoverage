// RUN: test-instrumentation %s

void test(int x) {
//% void test(int x) {@;$;
	int r = x < 0 ? -x : x;
	//% int r = x < 0 ? ($,-x) : ($,x);

	if(r == 0) (void)0;
	//% if(r == 0) {$;(void)0;}
}
