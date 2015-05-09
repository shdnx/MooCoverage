// RUN: test-instrumentation %s

void test(int x) {
//% void test(int x) {@;$;
	if(x < 0) return;
	//% if(x < 0) {$;return;}$;

	while(x > 0) {
	//% while(x > 0) {$;
		if(x % 2 == 1) {
		//% if(x % 2 == 1) {$;
			continue;
		}
		//% }$;

		if(x == 100) {
		//% if(x == 100) {$;
			x = 1;
			break;
		}
		//% }$;

		x--;
	}

	if(x == 0) {
	//% if(x == 0) {$;
		if(x != 0) throw "wtf";
		//% if(x != 0) {$;throw "wtf";}$;
	}
}
