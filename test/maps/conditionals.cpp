// RUN: test-mapping %s %t.cpp

void test(int x) {$0
	int r = x < 0 ? $1-x$1 : $2x$2;

	if(r == 0) $3(void)0;$3
	else $4if(r > 1) {$5 (void)0; $5}
	else {$6
		(void)1;
	$6}$4
$0}
