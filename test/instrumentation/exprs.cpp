// RUN: test-instrumentation %s

void test(int x) {
//% void test(int x) {@;$;

	int abs = x < 0 ? -x : x;
	//% int abs = x < 0 ? ($,-x) : ($,x);

	int* p = x > 0 ? &x : 0;
	//% int* p = x > 0 ? ($,&x) : (int *)($,0);

	if(x == 0 || x > 42) {}
	//% if(x == 0 || ($,x > 42)) {$;}

	else if(10 <= x && x <= 30) (void)0;
	//% else {$;if(10 <= x && ($,x <= 30)) {$;(void)0;}}

	bool cond = (x == 1 || x == 2 || x == 3);
	//% bool cond = (x == 1 || ($,x == 2) || ($,x == 3));
}
