// RUN: test-instrumentation %s
void external_foo();

void stuff(bool x) {
//% void stuff(bool x) {@;$;
start:
	external_foo();
	//% $;external_foo();

	if(x) goto start;
	//% if(x) {$;goto start;}$;
}
