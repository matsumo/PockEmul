#org 0x4000
  
#include <graph.h>

byte i;
word j;

main() {
	
	cls();
	
	for (i=0;i<50;i++) {
		j=3*i;
		line(j,0,75,15);
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
