// RUN: test-mapping %s %t.cpp

void test(int x) {$0
	int abs = x < 0 ? $1-x$1 : $2x$2;

	if(x == 0 || $3x > 42$3) {$4$4}
	else $5if(10 <= x && $6x <= 30$6) $7(void)0;$7$5

	bool cond = (x == 1 || $8x == 2$8 || $9x == 3$9);
$0}
