// RUN: test-instrumentation %s
// XFAIL: *

void goto_test(bool x) {
//% void goto_test(bool x) {@;$;
start:
	int y;
	//% $;int y;

	if(x) goto start;
	//% if(x) {$;goto start;}$;
}

void switch_test(int x) {
//% void switch_test(int x) {@;$;
	switch(x) {
	//% switch(x) {$;

	case 1: goto_test(true); break;
	//% case 1: $;goto_test(true); break;

	case 2:
	//% case 2:$;

		return;
		//% $;return;

	default:
		if(x > 5) (void)1;
		//% $;if(x > 5) {$;(void)1;}
	}
	//% }$;
}
