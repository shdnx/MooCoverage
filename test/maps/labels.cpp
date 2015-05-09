// RUN: test-mapping %s %t.cpp
// XFAIL: *

void goto_label_test(bool x) {$S
start:
	$Lint y;

	if(x) $0goto start;$0$G
$G$L$S}

void switch_test(int x) {$F
	switch(x) {$W
	case 1: $1(void)1; break;
	case 2:$B
		$2return;

	default:$R
		$Dif(x > 5) $3(void)1;$3
	$D$2$B$1$W}
$R$F}
