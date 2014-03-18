#ifndef __SOUND_H
#define __SOUND_H

/*! 
 \brief Produce a Sound with specific frequecy and duration.
 
 \fn beep 
 \param beep_freq the requested frequency
 \param beep_duration the duration
*/
beep(byte beep_freq,byte beep_duration) {
    _reg_xh = beep_freq;
    _reg_xl = beep_duration;

//#save
#asm

    LP REG_XL
    LDM
    EXAB
    LP REG_XH
    LDM
    LIDP lb2603+1		; set length of
    STD			;  wait intervals

#ifdef __PC_RESONATOR_768K__
    LIDP    lb2604+1
    STD
#endif
    LIDP    lb2605+1
    STD
    LIDP    lb2606+1
    STD

    EXAB			; K=B, B=A
    LP	REG_XH
    EXAM

    LIA  0x04		; push 4
    PUSH

lb2602:
    LP   REG_XH		; I=K, P=95
    LDM
    LP   REG_I
    EXAM
    LIP  95

lb2603:
    WAIT 0x00		; 6 + B

#ifdef __PC_RESONATOR_768K__
lb2604:
    WAIT 0x00		; 6 + B
    WAIT 0x02		; 8
#endif

    ORIM 0x10;PORT_C_BEEP_FREQ	; 4
    OUTC			; 2
lb2605:
    WAIT 0x00		; 6 + B
lb2606:
    WAIT 0x00		; 6 + B
    ANIM 0x01;PORT_C_DISPLAY	; 4
    OUTC			; 2
    TEST 0x02;TEST_CNT_2		; 4
    JRZM lb2603		; 7 (taken)


    DECI
    JRNZM lb2603		; loop I (=param K) times

    LOOP lb2602		; loop 5 times

#endasm
//#restore
}



beepcount(byte nb) {
	byte i;
	for (i=0;i<nb;i++) {
		beep(95,10);
	}
}

#endif
