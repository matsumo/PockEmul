#org 0x4000

#include <graph.h>

byte i;

main() {
	
	cls();

	for (i=0;i<150;i=i+3) {
		line(i,0,75,15);
	}
	for (i=0;i<31;i=i+3) {
		line(75,16,150,i);
	}
	
	for (i=150;i>=1;i=i-3) {
		line(i,31,75,16);
	}
	for (i=30;i>=1;i=i-3) {
		line(75,16,0,i);
	}
}

/*
	10 CLS:WAIT 0
	20 FOR I=0 TO 150 STEP 3
	30 LINE(I,0)-(75,15)
	40 NEXT I
	50 FOR I=0 TO 31 STEP 3
	60 LINE(75,16)-(150,I)
	70 NEXT I
	110 FOR I=150 TO 1 STEP -3
	120 LINE(I,31)-(75,16)
	130 NEXT I
	140 FOR I=31 TO 1 STEP -3
	150 LINE(75,16)-(0,I)
	160 NEXT I
	200 GOTO 200
*/
