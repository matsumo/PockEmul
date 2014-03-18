#ifndef __STDIO_H
#define __STDIO_H



/*! 
 \brief Return 0xff if the On/Break key is pressed, 0x00 otherwise.
 
 \fn __onbreak 
 \return byte 
*/
byte __onbreak() { 
#asm
fct___onbreak:
    RA	;test
    TEST 08
    JRZP lb__stdio__onbreak_end
    LIA 0xff
lb__stdio__onbreak_end:
#endasm

    return _reg_a;
}


/*! 
 \brief Do nothing during a n milliseconds laptime. 
 
 \fn ps_wait 
 \param _var_ps_wait_nb Number of milliseconds to wait
*/
ps_wait(word _var_ps_wait_nb) {
	load _var_ps_wait_nb;
#asm
	PUSH	; push A
lb__sdtio1501:
#ifdef __PC_RESONATOR_768K__
	WAIT 0xF0
#endif
#ifdef __PC_RESONATOR_576K__
	WAIT 0xB6
#endif
	LOOP lb__sdtio1501
	
	EXAB
	PUSH
lb__sdtio1502:
	LIA 0xFF
	PUSH
lb__sdtio1503:
#ifdef __PC_RESONATOR_768K__
	WAIT 0xF0
#endif
#ifdef __PC_RESONATOR_576K__
	WAIT 0xB6
#endif
	LOOP lb__sdtio1503
	LOOP lb__sdtio1502
#endasm
}


/*! 
 \brief Turn display refresh off.
 
 \fn ps_disp_off 
*/
ps_disp_off() {
#asm
fct_ps_disp_off:
	LIP  95
	ANIM 0xFE
	OUTC
#endasm
}

/*! 
 \brief Turn display refresh on.
 
 \fn ps_disp_on 
*/
ps_disp_on() {
#asm
fct_ps_disp_on:
	LIP  95
	ORIM 1
	OUTC
#endasm
}

byte xram ps_cur_x;
byte xram ps_cur_y;
byte xram var_ps_putchar_con_rombank;
/*! 
 \brief Writes a character at the (ps_cur_x,ps_cur_y) char coordinates.
 asm: Writes a character in A to the display
 Used Registers: A,B,X,Y
 \fn ps_putchar_con 
 \param ps_putchar_con_c The Char to display
*/
ps_putchar_con(char ps_putchar_con_c) {
	load ps_putchar_con_c;

_LCC_DEPEND(ps_disp_off);
_LCC_DEPEND(ps_ctabadr);
_LCC_DEPEND(ps_xcur2xgcur);
_LCC_DEPEND(ps_dispadr);	
_LCC_DEPEND(ps_scroll);
_LCC_DEPEND(ps_disp_on);

#asm
fct_ps_putchar_con:
	CALL fct_ps_disp_off
	; if newline then jump
	CPIA 0x0D
	JRZP lb__stdio01newline
		
#ifdef __PC_IS_BANKSWITCHED__
	; if this is a bankswiched pocket, we
	; switch to the rom with the character table
	PUSH
	LIDP  __MEM_BANKSWITCH__
	LDD
	LIDP var_ps_putchar_con_rombank
	STD
	LIDP  __MEM_BANKSWITCH__
	LIA   __MEM_BS_IND_CHARTAB__
	STD
	POP
#endif
	
	CALL  fct_ps_ctabadr
	CALL  fct_ps_xcur2xgcur
	
	LP    REG_K
	EXAM
	
	LIA  __LCD_CHAR_WIDTH__-1
	PUSH
lb__sdtio0101:                       ; copy the 5 (or 6) pixmaps
	LIDP  ps_cur_y        ; y cursor into B
	LDD
	EXAB
	
	LP    REG_K
	LDM
	INCK
	CALL  fct_ps_dispadr
	
#ifdef __INVERS_CHARTAB__
	DXL
#endif
#ifndef __INVERS_CHARTAB__
	IXL
#endif
	DY
	; introduce here invert char flag
	; 255 -A -> A
		
	IYS 
	LOOP lb__sdtio0101
	
	LIDP  ps_cur_x        ; increase x cursor
	LDD
	ADIA  1
	STD
	
	CPIA  __DISP_CH_COLS__        ; if not max. column + 1,
	JRNZP lb__sdtio0102          ; then go on
lb__stdio01newline:	
	LIDP  ps_cur_x
	RA                    ; else: set to zero
	STD
	
	LIDP  ps_cur_y        ; ... and increase y
	LDD
	ADIA  1
	STD
	
	CPIA  __DISP_CH_ROWS__        ; if not max. row + 1,
	JRNZP  lb__sdtio0102         ; then go on
	
	CALL  fct_ps_scroll       ; else: scroll!
	
lb__sdtio0102:

#ifdef __PC_IS_BANKSWITCHED__
	; if this is a bankswiched pocket, we
	; restore to the initial rom bank
	PUSH
	LIDP var_ps_putchar_con_rombank
	LDD
	LIDP  __MEM_BANKSWITCH__
	STD
	POP
#endif

	CALL fct_ps_disp_on;
	
#endasm
}


/*! 
 \brief Invert the display.
 
 \fn ps_inv 
*/
ps_inv() {
	
_LCC_DEPEND(ps_disp_off);
_LCC_DEPEND(ps_disp_on);
	
#asm
fct_ps_inv:

	CALL fct_ps_disp_off
#if __PC_1350__ || __PC_1360__
	; copy L2C1 en L1C1
	; copy L3C1 en L2C1
	; copy L4C1 en L3C1
	; erase L4C1 
	LIB 0
	LIA 4
	PUSH
lb__sdtio0701:	
	
	
	LP REG_XH
	LIA		HB(__MEM_LCD_L1C1__-1)
	EXAM
	EXAB
	ADM
	EXAB
	LP REG_XL
	LIA		LB(__MEM_LCD_L1C1__-1)
	EXAM
	LP REG_YH
	LIA		HB(__MEM_LCD_L1C1__-1)
	EXAM
	EXAB
	ADM
	EXAB
	LP REG_YL
	LIA		LB(__MEM_LCD_L1C1__-1)
	EXAM	
	LIA 59
	PUSH
	
lb__sdtio0702:
	IXL
	; 255 -A -> A
	LII 0xFF
	LP 0
	SBM
	LDM
	
	IYS
	LOOP lb__sdtio0702
	
	LP REG_B
	ADIM 2
	LOOP lb__sdtio0701

	LIB 0
	LIA 4
	PUSH
	lb__sdtio0703:	
	
	
	LP REG_XH
	LIA		HB(__MEM_LCD_L2C1__-1)
	EXAM
	EXAB
	ADM
	EXAB
	LP REG_XL
	LIA		LB(__MEM_LCD_L2C1__-1)
	EXAM
	LP REG_YH
	LIA		HB(__MEM_LCD_L2C1__-1)
	EXAM
	EXAB
	ADM
	EXAB
	LP REG_YL
	LIA		LB(__MEM_LCD_L2C1__-1)
	EXAM	
	LIA 59
	PUSH
	
	lb__sdtio0704:
	IXL
	; 255 -A -> A
	LII 0xFF
	LP 0
	SBM
	LDM
	
	IYS
	LOOP lb__sdtio0704
	
	LP REG_B
	ADIM 2
	LOOP lb__sdtio0703
		
#endif

#if __PC_1260__ || __PC_1261__ || __PC_1262__ || __PC_1475__
	; copy L2C1 en L1C1
	; copy L3C1 en L2C1
	; copy L4C1 en L3C1
	; erase L4C1 
	LIB 0
	LIA 2
	PUSH
lb__sdtio0701:	
	
	LP REG_XH
	LIA		HB(__MEM_LCD_L1C1__-1)
	EXAM
	EXAB
	ADM
	EXAB
	LP REG_XL
	LIA		LB(__MEM_LCD_L1C1__-1)
	EXAM
	LP REG_YH
	LIA		HB(__MEM_LCD_L1C1__-1)
	EXAM
	EXAB
	ADM
	EXAB
	LP REG_YL
	LIA		LB(__MEM_LCD_L1C1__-1)
	EXAM	
	LIA 59
	PUSH
	
	lb__sdtio0702:
	IXL
	; 255 -A -> A
	LII 0xFF
	LP 0
	SBM
	LDM
	
	IYS
	LOOP lb__sdtio0702
	
	LP REG_B
	ADIM 8
	LOOP lb__sdtio0701	

	LIB 0
	LIA 2
	PUSH
	lb__sdtio0703:	
	
	LP REG_XH
	LIA		HB(__MEM_LCD_L2C1__-1)
	EXAM
	EXAB
	ADM
	EXAB
	LP REG_XL
	LIA		LB(__MEM_LCD_L2C1__-1)
	EXAM
	LP REG_YH
	LIA		HB(__MEM_LCD_L2C1__-1)
	EXAM
	EXAB
	ADM
	EXAB
	LP REG_YL
	LIA		LB(__MEM_LCD_L2C1__-1)
	EXAM	
	LIA 59
	PUSH
	
	lb__sdtio0704:
	IXL
	; 255 -A -> A
	LII 0xFF
	LP 0
	SBM
	LDM
	
	IYS
	LOOP lb__sdtio0704
	
	LP REG_B
	ADIM 8
	LOOP lb__sdtio0703	
#endif

#if __PC_1250__ || __PC_1251__ || __PC_1255__

	LP REG_XH
	LIA		HB(__MEM_LCD_L1C1__-1)
	EXAM
	LP REG_XL
	LIA		LB(__MEM_LCD_L1C1__-1)
	EXAM
	LP REG_YH
	LIA		HB(__MEM_LCD_L1C1__-1)
	EXAM
	LP REG_YL
	LIA		LB(__MEM_LCD_L1C1__-1)
	EXAM	
	LIA 59
	PUSH
	
	lb__sdtio0702:
	IXL
	; 255 -A -> A
	LII 0xFF
	LP 0
	SBM
	LDM
	
	IYS
	LOOP lb__sdtio0702
	
	LP REG_XH
	LIA		HB(__MEM_LCD_L1C2__-1)
	EXAM
	LP REG_XL
	LIA		LB(__MEM_LCD_L1C2__-1)
	EXAM
	LP REG_YH
	LIA		HB(__MEM_LCD_L1C2__-1)
	EXAM
	LP REG_YL
	LIA		LB(__MEM_LCD_L1C2__-1)
	EXAM	
	LIA 59
	PUSH
	
	lb__sdtio0704:
	IXL
	; 255 -A -> A
	LII 0xFF
	LP 0
	SBM
	LDM
	
	IYS
	LOOP lb__sdtio0704

#endif
	CALL fct_ps_disp_on
#endasm
}


/*! 
 \brief 
 	Scrolls the display one line.
	for pockets with only on line it is 
	simple - just delete the display.
 
 \fn ps_scroll
*/
ps_scroll() {

_LCC_DEPEND(ps_clrscr);
	
#asm
	fct_ps_scroll:
	
#if __PC_1350__ || __PC_1360__
	; copy L2C1 en L1C1
	; copy L3C1 en L2C1
	; copy L4C1 en L3C1
	; erase L4C1 
	LIB 0
	LIA 4
	PUSH
lb__sdtio0501:	
	
	
	LP REG_XH
	LIA		HB(__MEM_LCD_L2C1__-1)
	EXAM
	EXAB
	ADM
	EXAB
	LP REG_XL
	LIA		LB(__MEM_LCD_L2C1__-1)
	EXAM
	LP REG_YH
	LIA		HB(__MEM_LCD_L1C1__-1)
	EXAM
	EXAB
	ADM
	EXAB
	LP REG_YL
	LIA		LB(__MEM_LCD_L1C1__-1)
	EXAM	
	LIA 29
	PUSH
	
lb__sdtio0502:
	IXL
	IYS
	LOOP lb__sdtio0502
	
	LP REG_B
	ADIM 2
	LOOP lb__sdtio0501

	; copy L2C1 en L1C1
	; copy L3C1 en L2C1
	; copy L4C1 en L3C1
	; erase L4C1 
	LIB 0
	LIA 4
	PUSH
lb__sdtio0503:	
	
	
	LP REG_XH
	LIA		HB(__MEM_LCD_L3C1__-1)
	EXAM
	EXAB
	ADM
	EXAB
	LP REG_XL
	LIA		LB(__MEM_LCD_L3C1__-1)
	EXAM
	LP REG_YH
	LIA		HB(__MEM_LCD_L2C1__-1)
	EXAM
	EXAB
	ADM
	EXAB
	LP REG_YL
	LIA		LB(__MEM_LCD_L2C1__-1)
	EXAM	
	LIA 29
	PUSH
	
	lb__sdtio0504:
	IXL
	IYS
	LOOP lb__sdtio0504
	
	LP REG_B
	ADIM 2
	LOOP lb__sdtio0503

	; copy L2C1 en L1C1
	; copy L3C1 en L2C1
	; copy L4C1 en L3C1
	; erase L4C1 
	LIB 0
	LIA 4
	PUSH
	lb__sdtio0505:	
	
	
	LP REG_XH
	LIA		HB(__MEM_LCD_L4C1__-1)
	EXAM
	EXAB
	ADM
	EXAB
	LP REG_XL
	LIA		LB(__MEM_LCD_L4C1__-1)
	EXAM
	LP REG_YH
	LIA		HB(__MEM_LCD_L3C1__-1)
	EXAM
	EXAB
	ADM
	EXAB
	LP REG_YL
	LIA		LB(__MEM_LCD_L3C1__-1)
	EXAM	
	LIA 29
	PUSH
	
	lb__sdtio0506:
	IXL
	IYS
	LOOP lb__sdtio0506
	
	LP REG_B
	ADIM 2
	LOOP lb__sdtio0505



	; erase last row
	LIDP  __MEM_LCD_L4C1__
	LII  30-1
	RA
	FILD
	LIDP  __MEM_LCD_L4C2__
	LII  30-1
	FILD
	LIDP  __MEM_LCD_L4C3__
	LII  30-1
	FILD
	LIDP  __MEM_LCD_L4C4__
	LII  30-1
	FILD
	LIDP  __MEM_LCD_L4C5__
	LII  30-1
	FILD
#endif
	
#if __PC_1260__ || __PC_1261__ || __PC_1262__ || __PC_1475__
	; copy L2C1 en L1C1
	; copy L2C2 en L1C2
	; erase L2C1 et L2C2
	LP REG_XH
	LIA		HB(__MEM_LCD_L2C1__-1)
	EXAM
	LP REG_XL
	LIA		LB(__MEM_LCD_L2C1__-1)
	EXAM
	LP REG_YH
	LIA		HB(__MEM_LCD_L1C1__-1)
	EXAM
	LP REG_YL
	LIA		LB(__MEM_LCD_L1C1__-1)
	EXAM	
	LIA 60
	PUSH

lb__sdtio0501:
	IXL
	IYS
	LOOP lb__sdtio0501
	
	LP REG_XH
	LIA		HB(__MEM_LCD_L2C2__-1)
	EXAM
	LP REG_XL
	LIA		LB(__MEM_LCD_L2C2__-1)
	EXAM
	LP REG_YH
	LIA		HB(__MEM_LCD_L1C2__-1)
	EXAM
	LP REG_YL
	LIA		LB(__MEM_LCD_L1C2__-1)
	EXAM	
	LIA 60
	PUSH
	
	lb__sdtio0502:
	IXL
	IYS
	LOOP lb__sdtio0502	
	
	LIDP  __MEM_LCD_L2C1__
	LII  12*__LCD_CHAR_WIDTH__-1
	RA
	FILD
	
	LIDP __MEM_LCD_L2C2__
	LII  12*__LCD_CHAR_WIDTH__-1
	FILD
#endif
	
#if __PC_1250__ || __PC_1251__ || __PC_1255__
	CALL fct_ps_clrscr
#endif

	; ps_cur_x = 0
	; ps_cur_y = nb_rows -1
	LIDP  ps_cur_x        
	RA
	STD
	LIDP  ps_cur_y
	LIA  __DISP_CH_ROWS__-1
	STD
#endasm
}


/*! 
 \brief Deletes all lines of the display
 
 \fn ps_clrscr
*/
ps_clrscr() {
#asm
fct_ps_clrscr:
	
#if __PC_1250__ || __PC_1251__ || __PC_1255__
	
	; ensure, that LIDL can be uses instead of LIDP one time
	;.assume >MEM_LCD_PART1 - >MEM_LCD_PART2
	
	LIDP __MEM_LCD_L1C1__
	LII  12*__LCD_CHAR_WIDTH__-1
	RA
	FILD
	
	LIDL __MEM_LCD_L1C2__
	LII  12*__LCD_CHAR_WIDTH__-1
	FILD
#endif

#if __PC_1350__ || __PC_1360__
	
	LIDP __MEM_LCD_L1C1__
	LII  12*__LCD_CHAR_WIDTH__-1
	RA
	FILD
	
	LIDP __MEM_LCD_L1C2__
	LII  12*__LCD_CHAR_WIDTH__-1
	FILD
	
	LIDP __MEM_LCD_L1C3__
	LII  12*__LCD_CHAR_WIDTH__-1
	FILD
	
	LIDP __MEM_LCD_L1C4__
	LII  12*__LCD_CHAR_WIDTH__-1
	FILD	

	LIDP __MEM_LCD_L1C5__
	LII  12*__LCD_CHAR_WIDTH__-1
	FILD	
		
	LIDP __MEM_LCD_L2C1__
	LII  12*__LCD_CHAR_WIDTH__-1
	RA
	FILD
	
	LIDP __MEM_LCD_L2C2__
	LII  12*__LCD_CHAR_WIDTH__-1
	FILD
	
	LIDP __MEM_LCD_L2C3__
	LII  12*__LCD_CHAR_WIDTH__-1
	FILD
	
	LIDP __MEM_LCD_L2C4__
	LII  12*__LCD_CHAR_WIDTH__-1
	FILD	
	
	LIDP __MEM_LCD_L2C5__
	LII  12*__LCD_CHAR_WIDTH__-1
	FILD
#endif

#if __PC_1260__ || __PC_1261__ || __PC_1262__ || __PC_1475__
	
	; aaa ensure, that LIDL can be uses instead of LIDP two times
	;.assume >MEM_LCD_L1C1 - >MEM_LCD_L2C1
	;.assume >MEM_LCD_L1C2 - >MEM_LCD_L2C2
	
	LIDP  __MEM_LCD_L1C1__
	LII  12*__LCD_CHAR_WIDTH__-1
	RA
	FILD
	
	LIDL LB(__MEM_LCD_L2C1__)
	LII  12*__LCD_CHAR_WIDTH__-1
	FILD
	
	LIDP  __MEM_LCD_L1C2__
	LII  12*__LCD_CHAR_WIDTH__-1
	FILD
	
	LIDL LB(__MEM_LCD_L2C2__)
	LII  12*__LCD_CHAR_WIDTH__-1
	FILD
	
#endif
	
	; other modells are not implemented yet
	;.error 1
	
	
	; now reset the cursors
	LIDP  ps_cur_x        
	RA
	STD
	LIDP  ps_cur_y
	STD

#endasm
}


/*! 
 \brief Converts x cursor position to graphics cursor position.
 
 \fn ps_xcur2xgcur 
*/
ps_xcur2xgcur(){
#asm	
fct_ps_xcur2xgcur:
	
#if (__LCD_COL_WIDTH__ == 5) 
	LIDP ps_cur_x          ; Load x cursor position
	LDD                    ; into A and B
	EXAB
	LP   REG_B
	LDM
	SL                     ; Shift A two times and
	SL                     ; add B, this equals
	ADM                    ; multiply by five
	EXAB
#endif
	
#if (__LCD_COL_WIDTH__ == 6) 
	
	LIDP ps_cur_x          ; Load left shifted x cursor
	LDD                    ; position into A and B
	SL
	EXAB
	LP   REG_B
	LDM
	SL                     ; Shift A one times and
	ADM                    ; add B, this equals
	EXAB                   ; multiply by six
	
#endif
	
	; other modells are not implemented yet
	;.error 1
	
#endasm
}


/*! 
 \brief 
 Calculates the display adress of a
given graphics cursor location.

Positions that are to big to fit in
the display return the adress of the
most right display column if __RANGE_CHECK__
is on, otherwise the behaviour is undefined
(most likely crash).

Used Registers: A,B,Y
Input A (graph. cur. x), B (graph. cur. y)
Return value: Y
 
 \fn ps_dispadr 
*/
ps_dispadr() {
#asm
fct_ps_dispadr:

#if __PC_1250__ || __PC_1251__ || __PC_1255__
	
	; -----------------------------------------------
	
	EXAB                    ; copy A to B
	LP  REG_B
	LDM        
	CPIA 60			; Char 1-12 ?
	JRCP lb__sdtio0402		; Jump to positive offset calculation
	
	; Calculate negative offsets

	LIB  0xB7               ; LSB is 0xB7 - REG_A
	LP   REG_B
	RC
	SBM
	
	; Calculate positive offsets
	
lb__sdtio0402: 
	
	EXAB		        ; Copy B to YL
	LP REG_YL
	EXAM
	
	LIA HB(__MEM_LCD_L1C1__)      ; Copy constant hi byte to YH
	LP REG_YH
	EXAM
	; -------------------AA----------------------------
	
#endif


#if __PC_1403__

; ----------------------BB-------------------------

	EXAB                    ; copy A to B
	LP  REG_B
	LDM        
	CPIA 60			; Char 1-12 ?
	JRCP lb0__sdtio402		; Jump to positive offset calculation

; Calculate negative offsets

;.ifdef __RANGE_CHECK__
;	CPIA 120		; Valid char range ( < 25) ?
;	JRCP lb__sdtio0403
;	LIA  119                ; default: most right x position
;lb__sdtio0403: 
;.endif

	CPIA 90			; If not char 19-24, jump ahead
	JRCP lb__sdtio0404
	LIB  0xB7               ; LSB is 0xB7 - REG_A
	JRP  lb__sdtio0406		; "break"
lb__sdtio0404: 
	CPIA 75			; If not char 16-18, jump ahead
	JRCP lb__sdtio0405
	LIB  0xC6               ; LSB is &C6 - REG_A
	JRP  lb__sdtio0406		; "break"
; Assert char position is in the range 13-15 
lb__sdtio0405: 
	LIB  0xA8               ; LSB is 0xA8 - REG_A

lb__sdtio0406: 
	LP   REG_B
	RC
	SBM

	JRP  lb__sdtio0409		; Jump over positive offset calculation

; Calculate positive offsets

lb__sdtio0402: 
	CPIA 45			; If not char 10-12, jump ahead
	JRCP lb__sdtio0407
	LP   REG_B              ; LSB is REG_B - 0x0F
	SBIM 0x0F              
	JRP  lb__sdtio0409		; "break"
lb__sdtio0407:	
	CPIA 30			; If not char 7-9, jump ahead
	JRCP lb__sdtio0408
	LP   REG_B              ; LSB is REG_B + 0x0F
	ADIM 0x0F              
; Next line not nessesary, because handling of range 1-6
;  does nothing
;       JRP  lb0409		; "break"

; Assert char position is in the range 1-6 
lb__sdtio0408:				; LSB is REG_B - do nothing !

; Write result to memory

lb__sdtio0409:	
	EXAB		        ; Copy B to YL
	LP REG_YL
	EXAM

	LIA HB(__MEM_LCD_L1C1__)      ; Copy constant hi byte to YH
	LP REG_YH
	EXAM
; -----------------------------------------------
	
#endif
	
#if __PC_1260__ || __PC_1261__ || __PC_1262__ || __PC_1475__
	
	; -----------------------------------------------
	
	; First we handle the row
	PUSH
	RA
	LP   REG_B
	TSIM 0x01               ; Test for line 1
	JRZP lb__sdtio0407
	ORIA 0x40        
lb__sdtio0407: 
	LP   REG_YL
	EXAM
	POP
	
	; Now we handle the column block address
	PUSH
	
	CPIA 60			; If not char 13-24, jump ahead
	JRCP lb__sdtio0401
	LIA  HB(__MEM_LCD_L1C2__)
	LIB  60
	JRP  lb__sdtio0402		; "break"
	
	; Assert char position is in the range 1-12 
lb__sdtio0401:	
	LIA  HB(__MEM_LCD_L1C1__)
	LIB  0
	
	; Write result to YH
lb__sdtio0402:	
	LP REG_YH
	EXAM
	
	; Calculate graph x cur - block offset
	POP
	EXAB
	LP   REG_B
	SBM
	RA
	EXAB
	
	; Add it to Y
	LP   REG_YL
	ADB			; 16 bit add
	
	; -----------------------------------------------
	
#endif
	
#if __PC_1350__ || __PC_1360__

; -----------------------------------------------

; First we handle the row
	PUSH
	RA
	LP   REG_B
	TSIM 0x02               ; Test for line 3 and 4
	JRZP lb__sdtio0406
	LIA  0x1E
lb__sdtio0406: 
	TSIM 0x01               ; Test for line 1 and 2
	JRZP lb__sdtio0407
	ORIA 0x40        
lb__sdtio0407: 
	LP   REG_YL
	EXAM
	POP

	; Now we handle the column block address
	PUSH

	CPIA 120		; If not char 21-25, jump ahead
	JRCP lb__sdtio0401
	LIA  HB(__MEM_LCD_L1C5__)
	LIB  120
	JRP  lb__sdtio0405		; "break"

lb__sdtio0401:	
	CPIA 90			; If not char 16-20, jump ahead
	JRCP lb__sdtio0402
	LIA  HB(__MEM_LCD_L1C4__)
	LIB  90
	JRP  lb__sdtio0405		; "break"

lb__sdtio0402:	
	CPIA 60			; If not char 11-15, jump ahead
	JRCP lb__sdtio0403
	LIA  HB(__MEM_LCD_L1C3__)
	LIB  60
	JRP  lb__sdtio0405		; "break"

lb__sdtio0403:	
	CPIA 30			; If not char 6-10, jump ahead
	JRCP lb__sdtio0404
	LIA  HB(__MEM_LCD_L1C2__)
	LIB  30
	JRP  lb__sdtio0405		; "break"

	; Assert char position is in the range 1-5 
lb__sdtio0404:	
	LIA  HB(__MEM_LCD_L1C1__)
	LIB  0

	; Write result to YH
lb__sdtio0405:	
	LP REG_YH
	EXAM

	; Calculate graph x cur - block offset
	POP
	EXAB
	LP   REG_B
	SBM
	RA
	EXAB

	; Add it to Y
	LP   REG_YL
	ADB			; 16 bit add

; -----------------------------------------------
#endif
#endasm
}	

//; -----------------------------------------------
//; Calculates the address of a given character
//; (Register A) in the character table
//; The character stays in A
//;
//; Used Registers: A,B,X
//; Return value: X
//; label prefix __sdtio06
//; -----------------------------------------------
/*! 
 \brief 
Calculates the address of a given character
(Register A) in the character table
The character stays in A

Used Registers: A,B,X
Return value: X
 
 \fn ps_ctabadr 
*/
ps_ctabadr() {
	
#asm
fct_ps_ctabadr:
	LIB  1			; Default table offset: 0, plus on for IXL mnemonic
	
	
	PUSH			; Save A (= ascii code)
	
	LIA  HB(__MEM_CHARTAB__)       ; Table address -> X
	LP   REG_XH
	EXAM
	LIA  LB(__MEM_CHARTAB__)
	LP   REG_XL
	EXAM
	
	RA			; (00 B) -> BA = Table offset 
	EXAB
	
	SBB			; 16 bit sub
	DECP
	
	POP			; (00 ascii) -> BA			
	
	ADB			; 16 bit add
	DECP
	
	ADB			; 16 bit add
	DECP
	
	ADB			; 16 bit add
	DECP
	
	ADB			; 16 bit add
	DECP
	
	ADB			; 16 bit add
	DECP
	
#endasm
}

//; -----------------------------------------------
//; Display a byte pattern with address given in 
//; X+1, length in A and x pos in B
//; label prefix __sdtio14
//; -----------------------------------------------
ps_gprint() {
	
_LCC_DEPEND(ps_xcur2xgcur);
_LCC_DEPEND(ps_dispadr);


#asm
fct_ps_gprint:
	PUSH
	CALL  fct_ps_xcur2xgcur
	EXAB
	POP
fct_ps_gdraw:
	TSIA  0xff
	JRZP  lb__sdtio1402
	
	DECA
	PUSH			; counter for loop on stack
	
	EXAB			; x pos in K
	LP    8			;REG_K
	EXAM
	
lb__sdtio1401:                       ; copy n pixmaps
	LIDP  ps_cur_y        ; y cursor into B
	LDD
	EXAB
	
	LP    8			;REG_K
	LDM
	INCK
	CALL  fct_ps_dispadr
	IXL
	DY
	IYS 
	LOOP lb__sdtio1401
lb__sdtio1402:	
	RTN
	
	
ps_bitbltr:
	IXL
	DY
	IYS 
	RTN
	
ps_bitbltp:
	IXL
	DY
	IYS 
	RTN
#endasm
}


#endif // __STDIO_H
