// RUN: test-mapping %s %t.cpp

void test(int x) {$F
	if(x < 0) $0return;$0$R

	while(x > 0) {$1
		if(x % 2 == 1) {$2
			continue;
		$2}$C

		if(x == 100) {$3
			x = 1;
			break;
		$3}$B

		x--;
	$B$C$1}

	if(x == 0) {$4
		if(x != 0) $5throw "wtf";$5$T
	$4}
$T$R$F}
