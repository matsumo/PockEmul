/*****************************************************************************
 *
 *   upd7810.h
 *   Portable uPD7810/11, 7810H/11H, 78C10/C11/C14 emulator V0.3
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *   You can contact me at juergen@mame.net or pullmoll@stop1984.com
 *
 *   - This source code is released as freeware for non-commercial purposes
 *     as part of the M.A.M.E. (Multiple Arcade Machine Emulator) project.
 *     The licensing terms of MAME apply to this piece of code for the MAME
 *     project and derviative works, as defined by the MAME license. You
 *     may opt to make modifications, improvements or derivative works under
 *     that same conditions, and the MAME project may opt to keep
 *     modifications, improvements or derivatives under their terms exclusively.
 *
 *   - Alternatively you can choose to apply the terms of the "GPL" (see
 *     below) to this - and only this - piece of code or your derivative works.
 *     Note that in no case your choice can have any impact on any other
 *     source code of the MAME project, or binary, or executable, be it closely
 *     or losely related to this piece of code.
 *
 *  -  At your choice you are also free to remove either licensing terms from
 *     this file and continue to use it under only one of the two licenses. Do this
 *     if you think that licenses are not compatible (enough) for you, or if you
 *     consider either license 'too restrictive' or 'too free'.
 *
 *  -  GPL (GNU General Public License)
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License
 *     as published by the Free Software Foundation; either version 2
 *     of the License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 *  This work is based on the
 *  "NEC Electronics User's Manual, April 1987"
 *
 * NS20030115:
 * - fixed INRW_wa(cpustate)
 * - TODO: add 7807, differences are listed below.
 *       I only added support for these opcodes needed by homedata.c (yes, I am
 *       lazy):
 *       4C CE (MOV A,PT)
 *       48 AC (EXA)
 *       48 AD (EXR)
 *       48 AE (EXH)
 *       48 AF (EXX)
 *       50 xx (SKN bit)
 *       58 xx (SETB)
 *       5B xx (CLR)
 *       5D xx (SK bit)
 *
 * 2008-02-24 (Wilbert Pol):
 * - Added preliminary support for uPD7801
 *   For the uPD7801 only the basic instruction set was added. The current timer
 *   and serial i/o implementations are most likely incorrect.
 * - Added basic support for uPD78C05 and uPD78C06
 *   Documentation of the actual instruction set layout is missing, so we took
 *   the uPD7801 instruction set and only kept the instructions mentioned in
 *   the little documentation available on the uPD78c05A/06A. The serial i/o
 *   implementation has not been tested and is probably incorrect.
 *
 *****************************************************************************/
/* Hau around 23 May 2004
  gta, gti, dgt fixed
  working reg opcodes fixed
  sio input fixed
--
  PeT around 19 February 2002
  type selection/gamemaster support added
  gamemaster init hack? added
  ORAX added
  jre negativ fixed
  prefixed opcodes skipping fixed
  interrupts fixed and improved
  sub(and related)/add/daa flags fixed
  mvi ports,... fixed
  rll, rlr, drll, drlr fixed
  rets fixed
  l0, l1 skipping fixed
  calt fixed
*/

/*

7807 DESCRIPTION



   PA0  1     64 Vcc
   PA1  2     63 Vdd
   PA2  3     62 PD7/AD7
   PA3  4     61 PD6/AD6
   PA4  5     60 PD5/AD5
   PA5  6     59 PD4/AD4
   PA6  7     58 PD3/AD3
   PA7  8     57 PD2/AD2
   PB0  9     56 PD1/AD1
   PB1 10     55 PD0/AD0
   PB2 11     54 PF7/AB15
   PB3 12     53 PF6/AB14
   PB4 13     52 PF5/AB13
   PB5 14     51 PF4/AB12
   PB6 15     50 PF3/AB11
   PB7 16     49 PF2/AB10
   PC0 17     48 PF1/AB9
   PC1 18     47 PF0/AB8
   PC2 19     46 ALE
   PC3 20     45 WR*
   PC4 21     44 RD*
   PC5 22     43 HLDA
   PC6 23     42 HOLD
   PC7 24     41 PT7
  NMI* 25     40 PT6
  INT1 26     39 PT5
 MODE1 27     38 PT4
RESET* 28     37 PT3
 MODE0 29     36 PT2
    X2 30     35 PT1
    X1 31     34 PT0
   Vss 32     33 Vth

PA, PB, PC, PD, and PF is bidirectional I/O port
and PT is comparator input port in uPD7808.
uPD7807 uses PD port as data I/O and bottom address output,
and uses PF port as top address output.

NMI* is non maskable interrupt input signal (negative edge trigger).

INT1 is interrupt input (positive edge trigger). It can be used as
AC zero-cross input or trigger source of 16bit timer counter.

MODE0 and MODE1 is input terminal which decides total amount of
external memory of uPD7807 (4KByte, 16KBYte, and 64KByte).
It also decides number of PF ports used as top address output.
 4KByte mode: PF0~PF3=address output, PF4~PF7=data I/O port
16KByte mode: PF0~PF5=address output, PF6~PF7=data I/O port
64KByte mode: PF0~PF7=address output

RESET* is system rest terminal.

X1 and X2 does clock signal generation (connect OSC and condenser).

Vth is used to determine threshold voltage for PT port.
PT0~PT7 is connected to + input of each comparator,
and Vth deterimnes voltage connected to - input of PT0~PT7.
But the voltage of Vth is not directly connected to comapators.
It is connected via 16-level programmable voltage separate circuit.

HOLD and HLDA is terminal for DMA. RD*, WR*, and ALE is bus
interface signal (they are same type of Intel 8085).
Unlike 8085, I/O address space is not available, so IO /M* signal
does not exist. Read/write of external memory can be done
by RD*, WR*, and ALE only.

Vcc and Vss is main power source. Vdd is backup power source
for internal RWM (32 Byte).


PA and PB is I/O port. They have control register MA and MB.
If control register is set to 1, the port is input.
If control register is set to 0, the port is output.
They are set to 1 by reset.

PT is input-only port. It is consisted of input terminal PT0~PT7
and Vth (set threshold voltage). Each PT input has analog comparator
and latch, and + input of analog comparator is connected to
PT terminal. Every - input of analog comparator is connected
to devided voltage of Vth. Voltage dividing level can be set by
bottom 4bits of MT (mode T) register. The range is 1/16~16/16 of Vth.

Other internal I/Os are
8bit timer (x2): Upcounter. If the counter matches to specified value,
the timer is reset and counts again from 0.
You can also set it to generate interrupt, or invert output flip-flop
when the counter matches to specified value.
Furthermore, you can output that flip-flop output to PC4/TO output,
connect it to clock input of timer/event counter or watchdog timer.
Or you can use it as bitrate clock of serial interface.
Note: There is only 1 output flip-flop for 2 timers.
If you use it for timer output of 1 timer, another timer cannot be used
for other than interrupt generator.
Clock input for timer can be switched between internal clock (2 type)
or PC3/TI input. You can set 1 timer's match-output as another timer's
clock input, so that you can use them as 1 16bit timer.

16bit timer/event counter (x1): It can be used as
- Interval timer
- External event counter
- Frequency measurement
- Pulse width measurement
- Programmable rectangle wave output
- One pulse output
Related terminals are PC5/CI input, PC6/CO0 output, and PC7/CO1.
You can measure CI input's H duration, or you can output timing signal
(with phase difference) to CO0 and CO1.

serial I/F (x1): has 3 modes.
- Asynchronous mode
- Synchronous mode
- I/O interface mode
In all 3 modes, bitrate can be internal fixed clock, or timer output,
or external clock.
In asynchronous mode, you can
- switch 7bit/8bit data
- set parity ON/OFF and EVEN/ODD
- set 1/2 stop bit




DIFFERENCES BETWEEN 7810 and 7807

--------------------------
8bit transfer instructions
--------------------------

7810
inst.     1st byte 2nd byte state   action
EXX       00001001            4     Swap BC DE HL
EXA       00001000            4     Swap VA EA
EXH       01010000            4     Swap HL
BLOCK     00110001          13(C+1)  (DE)+ <- (HL)+, C <- C - 1, until CY

7807
inst.     1st byte  2nd byte state   action
EXR       01001000  10101101   8     Swap VA BC DE HL EA
EXX       01001000  10101111   8     Swap BC DE HL
EXA       01001000  10101100   8     Swap VA EA
EXH       01001000  10101110   8     Swap HL
BLOCK  D+ 00010000           13(C+1) (DE)+ <- (HL)+, C <- C - 1, until CY
BLOCK  D- 00010001           13(C+1) (DE)- <- (HL)-, C <- C - 1, until CY


---------------------------
16bit transfer instructions
---------------------------
All instructions are same except operand sr4 of DMOV instruction.
7810
V0-sr4 -function
 0-ECNT-timer/event counter upcounter
 1-ECPT-timer/event counter capture

7807
V1-V0- sr4 -function
 0- 0-ECNT -timer/event counter upcounter
 0- 1-ECPT0-timer/event counter capture 0
 1- 0-ECPT1-timer/event counter capture 1


-----------------------------------------
8bit operation instructions for registers
-----------------------------------------
All instructions are same.


--------------------------------------
8bit operation instructions for memory
--------------------------------------
All instructions are same.


-----------------------------------------
Operation instructions for immediate data
-----------------------------------------
uPD7807 has read-only PT port and special register group sr5 for it.
ins.               1st byte  2nd byte 3rd 4th state func
GTI    sr5, byte   01100100  s0101sss  dd      14   !CY  sr5 - byte - 1
LTI    sr5, byte   01100100  s0111sss  dd      14    CY  sr5 - byte
NEI    sr5, byte   01100100  s1101sss  dd      14   !Z   sr5 - byte
EQI    sr5, byte   01100100  s1111sss  dd      14    Z   sr5 - byte
ONI    sr5, byte   01100100  s1001sss  dd      14   !Z   sr5 & byte
OFFI   sr5, byte   01100100  s1011sss  dd      14    Z   sr5 & byte

S5-S4-S3-S2-S1-S0-sr -sr1-sr2-sr5-register function
 0  0  1  1  1  0 --- PT  --- PT  comparator input port T data
 1  0  0  1  0  0 WDM WDM --- --- watchdog timer mode register
 1  0  0  1  0  1 MT  --- --- --- port T mode

7807 doesn't have registers below
 0  0  1  0  0  0 ANM ANM ANM     A/D channel mode
 1  0  0  0  0  0 --- CR0 ---     A/D conversion result 0
 1  0  0  0  0  1 --- CR1 ---     A/D conversion result 1
 1  0  0  0  1  0 --- CR2 ---     A/D conversion result 2
 1  0  0  0  1  1 --- CR3 ---     A/D conversion result 3
 1  0  1  0  0  0 ZCM --- ---     zero cross mode

Special register operand (includes registers for I/O ports) has
6 groups - sr, sr1, sr2, sr3, sr4, and sr5. Among these groups,
sr, sr1, sr2, and sr5 includes registers described in the table
below, and expressed as bit pattern S5-S0.

S5S4S3S2S1S0 sr  sr1 sr2 sr5 register function
0 0 0 0 0 0  PA  PA  PA  PA  port A
0 0 0 0 0 1  PB  PB  PB  PB  port B
0 0 0 0 1 0  PC  PC  PC  PC  port C
0 0 0 0 1 1  PD  PD  PD  PD  port D
0 0 0 1 0 1  PF  PF  PF  PF  port F
0 0 0 1 1 0  MKH MKH MKH MKH mask high
0 0 0 1 1 1  MKL MKL MKL MKL mask low
0 0 1 0 0 1  SMH SMH SMH SMH serial mode high
0 0 1 0 1 0  SML --- --- --- serial mode low
0 0 1 0 1 1  EOM EOM EOM EOM timer/event counter output mode
0 0 1 1 0 0 ETMM --- --- --- timer/event counter mode
0 0 1 1 0 1  TMM TMM TMM TMM timer mode
0 0 1 1 1 0  --- PT  --- PT  port T
0 1 0 0 0 0  MM  --- --- --- memory mapping
0 1 0 0 0 1  MCC --- --- --- mode control C
0 1 0 0 1 0  MA  --- --- --- mode A
0 1 0 0 1 1  MB  --- --- --- mode B
0 1 0 1 0 0  MC  --- --- --- mode C
0 1 0 1 1 1  MF  --- --- --- mode F
0 1 1 0 0 0  TXB --- --- --- Tx buffer
0 1 1 0 0 1  --- RXB --- --- Rx buffer
0 1 1 0 1 0  TM0 --- --- --- timer register 0
0 1 1 0 1 1  TM1 --- --- --- timer register 1
1 0 0 1 0 0  WDM WDM --- --- watchdog timer mode
1 0 0 1 0 1  MT  --- --- --- mode T

For sr and sr1, all 6bits (S5, S4, S3, S2, S1, and S0) are used.
For sr2 and sr5, only 4bits (S3, S2, S1, AND S0) are used.
They are expressed as 'ssssss' and 's sss' in operation code.
Note that 's sss' (of sr2 and sr5) is located separately.
S0 is rightmost bit (LSB).


--------------------------------------------
Operation instructions for working registers
--------------------------------------------
All instructions are same.


--------------------------------------------------------------------------
16bit operation instructions and divider/multiplier operation instructions
--------------------------------------------------------------------------
All instructions are same.


------------------------------------------
Increment/decrement operation instructions
------------------------------------------
All instructions are same.


----------------------------
Other operation instructions
----------------------------
7807 has CMC instruction (inverts CY flag).
ins. 1st byte 2nd byte 3rd 4th state func
CMC  01001000 10101010           8   CY <- !CY


---------------------------
Rotation/shift instructions
---------------------------
All instructions are same.


-----------------------------
Jump/call/return instructions
-----------------------------
All instructions are same.


-----------------
Skip instructions
-----------------
7807 doesn't have this
ins.            1st byte 2nd byte 3rd 4th state func
BIT bit, wa     01011bbb  wwwwwwww          10*  bit skip if (V.wa).bit = 1

Instead, 7807 has these bit manipulation instructions.
ins.            1st byte 2nd byte 3rd 4th state func
MOV    CY, bit  01011111  bbbbbbbb          10* CY <- (bit)
MOV    bit, CY  01011010  bbbbbbbb          13* (bit) <- CY
AND    CY, bit  00110001  bbbbbbbb          10* CY <- CY & (bit)
OR     CY, bit  01011100  bbbbbbbb          10* CY <- CY | (bit)
XOR    CY, bit  01011110  bbbbbbbb          10* CY <- CY ^ (bit)
SETB   bit      01011000  bbbbbbbb          13* (bit) <- 1
CLR    bit      01011011  bbbbbbbb          13* (bit) <- 0
NOT    bit      01011001  bbbbbbbb          13* (bit) <- !(bit)
SK     bit      01011101  bbbbbbbb          10*  (b) skip if (bit) = 1
SKN    bit      01010000  bbbbbbbb          10* !(b) skip if (bit) = 0


------------------------
CPU control instructions
------------------------
ins.            1st byte 2nd byte 3rd 4th state func
HLT             01001000  00111011        11/12 halt
11 state in uPD7807 and uPD7810, 12 state in uPD78C10.

STOP            01001000  10111011          12  stop
7807 doesn't have STOP instruction.

*/


#include "upd7907.h"
#include "pcxxxx.h"
#include "Log.h"
#include "Inter.h"
#include "Debug.h"
#include "ui/cregsz80widget.h"



//INLINE upd7907_state *get_safe_token(device_t *device)
//{
//	assert(device != NULL);
//	assert(device->type() == UPD7810 ||
//		   device->type() == UPD7807 ||
//		   device->type() == UPD7801 ||
//		   device->type() == UPD78C05 ||
//		   device->type() == UPD78C06);
//	return (upd7907_state *)downcast<legacy_cpu_device *>(device)->token();
//}

#define CY	0x01
#define F1	0x02
#define L0	0x04
#define L1	0x08
#define HC	0x10
#define SK	0x20
#define Z	0x40
#define F7	0x80

/* IRR flags */
#define INTNMI	0x0001
#define INTFT0	0x0002
#define INTFT1	0x0004
#define INTF1	0x0008
#define INTF2	0x0010
#define INTFE0	0x0020
#define INTFE1	0x0040
#define INTFEIN 0x0080
#define INTFAD	0x0100
#define INTFSR	0x0200
#define INTFST	0x0400
#define INTER	0x0800
#define INTOV	0x1000
#define INTF0	0x2000

/* ITF flags */
#define INTAN4	0x0001
#define INTAN5	0x0002
#define INTAN6	0x0004
#define INTAN7	0x0008
#define INTSB	0x0010

#define PPC 	cpustate->ppc.w.l
#define PC		cpustate->pc.w.l
#define PCL 	cpustate->pc.b.l
#define PCH 	cpustate->pc.b.h
#define PCD 	cpustate->pc.d
#define SP		cpustate->sp.w.l
#define SPL 	cpustate->sp.b.l
#define SPH 	cpustate->sp.b.h
#define SPD 	cpustate->sp.d
#define PSW 	cpustate->psw
#define OP		cpustate->op
#define OP2 	cpustate->op2
#define IFF 	cpustate->iff
#define EA		cpustate->ea.w.l
#define EAL 	cpustate->ea.b.l
#define EAH 	cpustate->ea.b.h
#define VA		cpustate->va.w.l
#define V		cpustate->va.b.h
#define A		cpustate->va.b.l
#define VAD 	cpustate->va.d
#define BC		cpustate->bc.w.l
#define B		cpustate->bc.b.h
#define C		cpustate->bc.b.l
#define DE		cpustate->de.w.l
#define D		cpustate->de.b.h
#define E		cpustate->de.b.l
#define HL		cpustate->hl.w.l
#define H		cpustate->hl.b.h
#define L		cpustate->hl.b.l
#define EA2 	cpustate->ea2.w.l
#define VA2 	cpustate->va2.w.l
#define BC2 	cpustate->bc2.w.l
#define DE2 	cpustate->de2.w.l
#define HL2 	cpustate->hl2.w.l

#define OVC0	cpustate->ovc0
#define OVC1	cpustate->ovc1
#define OVCE	cpustate->ovce
#define OVCF	cpustate->ovcf
#define OVCS	cpustate->ovcs
#define EDGES	cpustate->edges

#define CNT0	cpustate->cnt.b.l
#define CNT1	cpustate->cnt.b.h
#define TM0 	cpustate->tm.b.l
#define TM1 	cpustate->tm.b.h
#define ECNT	cpustate->ecnt.w.l
#define ECPT	cpustate->ecnt.w.h
#define ETM0	cpustate->etm.w.l
#define ETM1	cpustate->etm.w.h

#define MA		cpustate->imem[0x03] //ma
#define MB		cpustate->imem[0x04] //mb
#define MCC 	cpustate->mcc
#define MC		cpustate->imem[0x05] //mc
#define MM		cpustate->mm
#define MF		cpustate->mf
#define S       cpustate->imem[0x08]
#define TMM 	cpustate->imem[0x09] //tmm
#define ETMM	cpustate->etmm
#define EOM 	cpustate->imem[0x0B] //eom
#define SML 	cpustate->imem[0x0A] //sml
#define SMH 	cpustate->smh
#define ANM 	cpustate->anm
#define MKL 	cpustate->imem[0x03] //cpustate->mkl
#define MKH 	cpustate->mkh
#define ZCM 	cpustate->zcm

#define CR0 	cpustate->cr0
#define CR1 	cpustate->cr1
#define CR2 	cpustate->cr2
#define CR3 	cpustate->cr3
#define RXB 	cpustate->rxb
#define TXB 	cpustate->txb

#define RXD 	cpustate->rxd
#define TXD 	cpustate->txd
#define SCK 	cpustate->sck
#define TI		cpustate->ti
#define TO		cpustate->to
#define CI		cpustate->ci
#define CO0 	cpustate->co0
#define CO1 	cpustate->co1

#define IRR 	cpustate->irr
#define ITF 	cpustate->itf



#define RDOP(O) 	O = cpustate->pPC->Get_8(PCD); PC++
#define RDOPARG(A)	A = cpustate->pPC->Get_8(PCD); PC++
#define RM(A)		cpustate->pPC->Get_8(A)
#define WM(A,V) 	cpustate->pPC->Set_8(A,V)

#define ZHC_ADD(after,before,carry) 	\
	if (after == 0) PSW |= Z; else PSW &= ~Z; \
	if (after == before) \
		PSW = (PSW&~CY) | (carry); \
	else if (after < before)			\
		PSW |= CY;			\
	else								\
		PSW &= ~CY;				\
	if ((after & 15) < (before & 15))	\
		PSW |= HC;						\
	else								\
		PSW &= ~HC; 					\

#define ZHC_SUB(after,before,carry) 	\
	if (after == 0) PSW |= Z; else PSW &= ~Z; \
	if (before == after)					\
		PSW = (PSW & ~CY) | (carry);	\
	else if (after > before)			\
		PSW |= CY;			\
	else								\
		PSW &= ~CY;				\
	if ((after & 15) > (before & 15))	\
		PSW |= HC;						\
	else								\
		PSW &= ~HC; 					\

#define SKIP_CY 	if (CY == (PSW & CY)) PSW |= SK
#define SKIP_NC 	if (0 == (PSW & CY)) PSW |= SK
#define SKIP_Z		if (Z == (PSW & Z)) PSW |= SK
#define SKIP_NZ 	if (0 == (PSW & Z)) PSW |= SK
#define SET_Z(n)	if (n) PSW &= ~Z; else PSW |= Z

void logerror(const char *format, ...) {

}


UINT8 Cupd7907::read_port_byte(upd7907_state *cpustate,offs_t port) {

    quint8 v = cpustate->pPC->in(port);
    if (cpustate->pPC->pCPU->fp_log) fprintf(cpustate->pPC->pCPU->fp_log,"\nREAD PORT[%02X]=%02X\n",port, v);
    return v;
//    return cpustate->imem[port];
}

void Cupd7907::write_port_byte(upd7907_state *cpustate,offs_t port,UINT8 data) {
    if (cpustate->pPC->pCPU->fp_log) fprintf(cpustate->pPC->pCPU->fp_log,"\nWRITE PORT[%02X]=%02X\n",port, data);
    cpustate->pPC->out(port,data);
    cpustate->imem[port]=data;
}

UINT8 Cupd7907::RP(upd7907_state *cpustate, offs_t port)
{
	UINT8 data = 0xff;
	switch (port)
	{
    case UPD7907_PORTA:
//        if (MA)	// NS20031301 no need to read if the port is set as output
//            cpustate->pa_in = read_port_byte(cpustate,port);
//        data = (cpustate->pa_in & MA) | (cpustate->pa_out & ~MA);
        data = cpustate->pa_out;
		break;
    case UPD7907_PORTB:
        if (MB)	// NS20031301 no need to read if the port is set as output
            cpustate->pb_in = read_port_byte(cpustate,port);
        data = (cpustate->pb_in & MB) | (cpustate->pb_out & ~MB);
		break;
    case UPD7907_PORTC:
//        if (MC)	// NS20031301 no need to read if the port is set as output
            cpustate->pc_in = read_port_byte(cpustate,port);
        data = cpustate->pc_in;//(cpustate->pc_in & MC) | (cpustate->pc_out & ~MC);
        break;
//        if (cpustate->mcc & 0x01)	/* PC0 = TxD output */
//			data = (data & ~0x01) | (cpustate->txd & 1 ? 0x01 : 0x00);
//		if (cpustate->mcc & 0x02)	/* PC1 = RxD input */
//			data = (data & ~0x02) | (cpustate->rxd & 1 ? 0x02 : 0x00);
//		if (cpustate->mcc & 0x04)	/* PC2 = SCK input/output */
//			data = (data & ~0x04) | (cpustate->sck & 1 ? 0x04 : 0x00);
//		if (cpustate->mcc & 0x08)	/* PC3 = TI input */
//			data = (data & ~0x08) | (cpustate->ti & 1 ? 0x08 : 0x00);
//		if (cpustate->mcc & 0x10)	/* PC4 = TO output */
//			data = (data & ~0x10) | (cpustate->to & 1 ? 0x10 : 0x00);
//		if (cpustate->mcc & 0x20)	/* PC5 = CI input */
//			data = (data & ~0x20) | (cpustate->ci & 1 ? 0x20 : 0x00);
//		if (cpustate->mcc & 0x40)	/* PC6 = CO0 output */
//			data = (data & ~0x40) | (cpustate->co0 & 1 ? 0x40 : 0x00);
//		if (cpustate->mcc & 0x80)	/* PC7 = CO1 output */
//			data = (data & ~0x80) | (cpustate->co1 & 1 ? 0x80 : 0x00);
//		break;
    case UPD7907_PORTD:
        cpustate->pd_in = read_port_byte(cpustate,port);
		switch (cpustate->mm & 0x07)
		{
		case 0x00:			/* PD input mode, PF port mode */
			data = cpustate->pd_in;
			break;
		case 0x01:			/* PD output mode, PF port mode */
			data = cpustate->pd_out;
			break;
		default:			/* PD extension mode, PF port/extension mode */
			data = 0xff;	/* what do we see on the port here? */
			break;
		}
		break;
    case UPD7907_PORTF:
        cpustate->pf_in = read_port_byte(cpustate,port);
		switch (cpustate->mm & 0x06)
		{
		case 0x00:			/* PD input/output mode, PF port mode */
			data = (cpustate->pf_in & cpustate->mf) | (cpustate->pf_out & ~cpustate->mf);
			break;
		case 0x02:			/* PD extension mode, PF0-3 extension mode, PF4-7 port mode */
			data = (cpustate->pf_in & cpustate->mf) | (cpustate->pf_out & ~cpustate->mf);
			data |= 0x0f;	/* what would we see on the lower bits here? */
			break;
		case 0x04:			/* PD extension mode, PF0-5 extension mode, PF6-7 port mode */
			data = (cpustate->pf_in & cpustate->mf) | (cpustate->pf_out & ~cpustate->mf);
			data |= 0x3f;	/* what would we see on the lower bits here? */
			break;
		case 0x06:
			data = 0xff;	/* what would we see on the lower bits here? */
			break;
		}
		break;
//	case UPD7807_PORTT:	// NS20031301 partial implementation
//		data = read_port_byte(port);
//		break;
	default:
        AddLog(LOG_CPU,"uPD7810 internal error: RP(cpustate) called with invalid port number\n");

	}
	return data;
}

void Cupd7907::WP(upd7907_state *cpustate, offs_t port, UINT8 data)
{
	switch (port)
	{
    case UPD7907_PORTA:
		cpustate->pa_out = data;
//      data = (data & ~MA) | (cpustate->pa_in & MA);
//        data = (data & ~MA) | (MA);	// NS20031401
        write_port_byte(cpustate,port, data);
		break;
    case UPD7907_PORTB:
		cpustate->pb_out = data;
      data = (data & ~MB) | (cpustate->pb_in & MB);
//        data = (data & ~MB) | (MB);	// NS20031401
        write_port_byte(cpustate,port, data);
		break;
    case UPD7907_PORTC:
		cpustate->pc_out = data;
      data = (data & ~cpustate->mc) | (cpustate->pc_in & cpustate->mc);
//        data = (data & ~MC) | (MC);	// NS20031401
		if (cpustate->mcc & 0x01)	/* PC0 = TxD output */
			data = (data & ~0x01) | (cpustate->txd & 1 ? 0x01 : 0x00);
		if (cpustate->mcc & 0x02)	/* PC1 = RxD input */
			data = (data & ~0x02) | (cpustate->rxd & 1 ? 0x02 : 0x00);
		if (cpustate->mcc & 0x04)	/* PC2 = SCK input/output */
			data = (data & ~0x04) | (cpustate->sck & 1 ? 0x04 : 0x00);
		if (cpustate->mcc & 0x08)	/* PC3 = TI input */
			data = (data & ~0x08) | (cpustate->ti & 1 ? 0x08 : 0x00);
		if (cpustate->mcc & 0x10)	/* PC4 = TO output */
			data = (data & ~0x10) | (cpustate->to & 1 ? 0x10 : 0x00);
		if (cpustate->mcc & 0x20)	/* PC5 = CI input */
			data = (data & ~0x20) | (cpustate->ci & 1 ? 0x20 : 0x00);
		if (cpustate->mcc & 0x40)	/* PC6 = CO0 output */
			data = (data & ~0x40) | (cpustate->co0 & 1 ? 0x40 : 0x00);
		if (cpustate->mcc & 0x80)	/* PC7 = CO1 output */
			data = (data & ~0x80) | (cpustate->co1 & 1 ? 0x80 : 0x00);
        write_port_byte(cpustate,port, data);
		break;
    case UPD7907_PORTD:
		cpustate->pd_out = data;
		switch (cpustate->mm & 0x07)
		{
		case 0x00:			/* PD input mode, PF port mode */
			data = cpustate->pd_in;
			break;
		case 0x01:			/* PD output mode, PF port mode */
			data = cpustate->pd_out;
			break;
		default:			/* PD extension mode, PF port/extension mode */
			return;
		}
        write_port_byte(cpustate,port, data);
		break;
    case UPD7907_PORTF:
		cpustate->pf_out = data;
		data = (data & ~cpustate->mf) | (cpustate->pf_in & cpustate->mf);
		switch (cpustate->mm & 0x06)
		{
		case 0x00:			/* PD input/output mode, PF port mode */
			break;
		case 0x02:			/* PD extension mode, PF0-3 extension mode, PF4-7 port mode */
			data |= 0x0f;	/* what would come out for the lower bits here? */
			break;
		case 0x04:			/* PD extension mode, PF0-5 extension mode, PF6-7 port mode */
			data |= 0x3f;	/* what would come out for the lower bits here? */
			break;
		case 0x06:
			data |= 0xff;	/* what would come out for the lower bits here? */
			break;
		}
        write_port_byte(cpustate,port, data);
		break;
	default:
        AddLog(LOG_CPU,"uPD7810 internal error: RP(cpustate) called with invalid port number\n");
	}
}

void Cupd7907::upd7907_take_irq(upd7907_state *cpustate)
{
	UINT16 vector = 0;
	int irqline = 0;

	/* global interrupt disable? */
	if (0 == IFF)
		return;

	switch ( cpustate->config.type )
	{
	case TYPE_7801:
		/* 1 - SOFTI - vector at 0x0060 */
		/* 2 - INT0 - Masked by MK0 bit */
		if ( IRR & INTF0 && 0 == (MKL & 0x01 ) )
		{
            irqline = UPD7907_INTF0;
			vector = 0x0004;
			IRR &= ~INTF0;
		}
		/* 3 - INTT - Masked by MKT bit */
		if ( IRR & INTFT0 && 0 == ( MKL & 0x02 ) )
		{
			vector = 0x0008;
			IRR &= ~INTFT0;
		}
		/* 4 - INT1 - Masked by MK1 bit */
		if ( IRR & INTF1 && 0 == ( MKL & 0x04 ) )
		{
            irqline = UPD7907_INTF1;
			vector = 0x0010;
			IRR &= ~INTF1;
		}
		/* 5 - INT2 - Masked by MK2 bit */
		if ( IRR & INTF2 && 0 == ( MKL & 0x08 ) )
		{
            irqline = UPD7907_INTF2;
			vector = 0x0020;
			IRR &= ~INTF2;
		}
		/* 6 - INTS - Masked by MKS bit */
		if ( IRR & INTFST && 0 == ( MKL & 0x10 ) )
		{
			vector = 0x0040;
			IRR &= ~INTFST;
		}
		break;

	default:
		/* check the interrupts in priority sequence */
		if ((IRR & INTFT0)	&& 0 == (MKL & 0x02))
		{
		    switch (cpustate->config.type)
			{
				case TYPE_7810_GAMEMASTER:
					vector = 0xff2a;
					break;
				default:
					vector = 0x0008;
			}
		    if (!((IRR & INTFT1)	&& 0 == (MKL & 0x04)))
			IRR&=~INTFT0;
		}
		else
		if ((IRR & INTFT1)	&& 0 == (MKL & 0x04))
		{
		    switch (cpustate->config.type)
			{
				case TYPE_7810_GAMEMASTER:
					vector = 0xff2a;
					break;
				default:
					vector = 0x0008;
			}
		    IRR&=~INTFT1;
		}
		else
		if ((IRR & INTF1)	&& 0 == (MKL & 0x08))
		{
            irqline = UPD7907_INTF1;
			vector = 0x0010;
//            MKL |= 0x08;
			if (!((IRR & INTF2)	&& 0 == (MKL & 0x10)))
			    IRR&=~INTF1;
		}
		else
		if ((IRR & INTF2)	&& 0 == (MKL & 0x10))
        {
            irqline = UPD7907_INTF2;
            vector = 0x0010;
			IRR&=~INTF2;
		}
		else
		if ((IRR & INTFE0)	&& 0 == (MKL & 0x20))
		{
		    switch (cpustate->config.type)
			{
				case TYPE_7810_GAMEMASTER:
					vector = 0xff2d;
					break;
				default:
					vector = 0x0018;
			}
		    if (!((IRR & INTFE1)	&& 0 == (MKL & 0x40)))
			IRR&=~INTFE0;
		}
		else
		if ((IRR & INTFE1)	&& 0 == (MKL & 0x40))
		{
		    switch (cpustate->config.type)
			{
			    case TYPE_7810_GAMEMASTER:
					vector = 0xff2d;
					break;
			    default:
					vector = 0x0018;
			}
		    IRR&=~INTFE1;
		}
		else
		if ((IRR & INTFEIN) && 0 == (MKL & 0x80))
		{
			vector = 0x0020;
		}
		else
		if ((IRR & INTFAD)	&& 0 == (MKH & 0x01))
		{
			vector = 0x0020;
		}
		else
		if ((IRR & INTFSR)	&& 0 == (MKH & 0x02))
		{
			vector = 0x0028;
		    IRR&=~INTFSR;
		}
		else
		if ((IRR & INTFST)	&& 0 == (MKH & 0x04))
		{
			vector = 0x0028;
		    IRR&=~INTFST;
		}
		break;
	}

	if (vector)
	{
		/* acknowledge external IRQ */
//		if (irqline)
//			(*cpustate->irq_callback)(cpustate->device, irqline);
		SP--;
		WM( SP, PSW );
		SP--;
		WM( SP, PCH );
		SP--;
		WM( SP, PCL );
		IFF = 0;
		PSW &= ~(SK|L0|L1);
		PC = vector;
        cpustate->pPC->pCPU->CallSubLevel++;
	}
}

void Cupd7907::upd7907_write_EOM(upd7907_state *cpustate)
{
	if (EOM & 0x01) /* output LV0 content ? */
	{
		switch (EOM & 0x0e)
		{
		case 0x02:	/* toggle CO0 */
			CO0 = (CO0 >> 1) | ((CO0 ^ 2) & 2);
			break;
		case 0x04:	/* reset CO0 */
			CO0 = 0;
			break;
		case 0x08:	/* set CO0 */
			CO0 = 1;
			break;
		}
	}
	if (EOM & 0x10) /* output LV0 content ? */
	{
		switch (EOM & 0xe0)
		{
		case 0x20:	/* toggle CO1 */
			CO1 = (CO1 >> 1) | ((CO1 ^ 2) & 2);
			break;
		case 0x40:	/* reset CO1 */
			CO1 = 0;
			break;
		case 0x80:	/* set CO1 */
			CO1 = 1;
			break;
		}
	}
}

void Cupd7907::upd7907_write_TXB(upd7907_state *cpustate)
{
	cpustate->txbuf = 1;
}

#define PAR7(n) ((((n)>>6)^((n)>>5)^((n)>>4)^((n)>>3)^((n)>>2)^((n)>>1)^((n)))&1)
#define PAR8(n) ((((n)>>7)^((n)>>6)^((n)>>5)^((n)>>4)^((n)>>3)^((n)>>2)^((n)>>1)^((n)))&1)

void Cupd7907::upd7907_sio_output(upd7907_state *cpustate)
{
	/* shift out more bits? */
	if (cpustate->txcnt > 0)
	{
		TXD = cpustate->txs & 1;
//		if (cpustate->config.io_callback)
//			(*cpustate->config.io_callback)(cpustate->device,UPD7907_TXD,TXD);
		cpustate->txs >>= 1;
		cpustate->txcnt--;
		if (0 == cpustate->txcnt)
			IRR |= INTFST;		/* serial transfer completed */
	}
	else
	if (SMH & 0x04) /* send enable ? */
	{
		/* nothing written into the transmitter buffer ? */
        if (0 == cpustate->txbuf)
			return;
        cpustate->txbuf = 0;

        if (SML & 0x03)         /* asynchronous mode ? */
		{
			switch (SML & 0xfc)
			{
			case 0x48:	/* 7bits, no parity, 1 stop bit */
			case 0x68:	/* 7bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bit int bit8 */
				cpustate->txs = (TXB << 1) | (1 << 8);
				cpustate->txcnt = 9;
				break;
			case 0x4c:	/* 8bits, no parity, 1 stop bit */
			case 0x6c:	/* 8bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bit int bit9 */
				cpustate->txs = (TXB << 1) | (1 << 9);
				cpustate->txcnt = 10;
				break;
			case 0x58:	/* 7bits, odd parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 8, stop bit in bit9 */
				cpustate->txs = (TXB << 1) | (PAR7(TXB) << 8) | (1 << 9);
				cpustate->txcnt = 10;
				break;
			case 0x5c:	/* 8bits, odd parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 9, stop bit int bit10 */
				cpustate->txs = (TXB << 1) | (PAR8(TXB) << 9) | (1 << 10);
				cpustate->txcnt = 11;
				break;
			case 0x78:	/* 7bits, even parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 8, stop bit in bit9 */
				cpustate->txs = (TXB << 1) | ((PAR7(TXB) ^ 1) << 8) | (1 << 9);
				cpustate->txcnt = 10;
				break;
			case 0x7c:	/* 8bits, even parity, 1 stop bit */
				/* insert start bit in bit0, parity in bit 9, stop bit int bit10 */
				cpustate->txs = (TXB << 1) | ((PAR8(TXB) ^ 1) << 9) | (1 << 10);
				cpustate->txcnt = 11;
				break;
			case 0xc8:	/* 7bits, no parity, 2 stop bits */
			case 0xe8:	/* 7bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bits int bit8+9 */
				cpustate->txs = (TXB << 1) | (3 << 8);
				cpustate->txcnt = 10;
				break;
			case 0xcc:	/* 8bits, no parity, 2 stop bits */
			case 0xec:	/* 8bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				/* insert start bit in bit0, stop bits in bits9+10 */
				cpustate->txs = (TXB << 1) | (3 << 9);
				cpustate->txcnt = 11;
				break;
			case 0xd8:	/* 7bits, odd parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 8, stop bits in bits9+10 */
				cpustate->txs = (TXB << 1) | (PAR7(TXB) << 8) | (3 << 9);
				cpustate->txcnt = 11;
				break;
			case 0xdc:	/* 8bits, odd parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 9, stop bits int bit10+11 */
				cpustate->txs = (TXB << 1) | (PAR8(TXB) << 9) | (3 << 10);
				cpustate->txcnt = 12;
				break;
			case 0xf8:	/* 7bits, even parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 8, stop bits in bit9+10 */
				cpustate->txs = (TXB << 1) | ((PAR7(TXB) ^ 1) << 8) | (3 << 9);
				cpustate->txcnt = 11;
				break;
			case 0xfc:	/* 8bits, even parity, 2 stop bits */
				/* insert start bit in bit0, parity in bit 9, stop bits int bits10+10 */
				cpustate->txs = (TXB << 1) | ((PAR8(TXB) ^ 1) << 9) | (1 << 10);
				cpustate->txcnt = 12;
				break;
			}
		}
		else
		{
			/* synchronous mode */
			cpustate->txs = TXB;
			cpustate->txcnt = 8;
		}
	}
}

void Cupd7907::upd7907_sio_input(upd7907_state *cpustate)
{
	/* sample next bit? */
	if (cpustate->rxcnt > 0)
	{
//		if (cpustate->config.io_callback)
//			RXD = (*cpustate->config.io_callback)(cpustate->device,UPD7907_RXD,RXD);
		cpustate->rxs = (cpustate->rxs >> 1) | ((UINT16)RXD << 15);
		cpustate->rxcnt--;
		if (0 == cpustate->rxcnt)
		{
			/* reset the TSK bit */
			SMH &= ~0x40;
			/* serial receive completed interrupt */
			IRR |= INTFSR;
			/* now extract the data from the shift register */
			if (SML & 0x03) 	/* asynchronous mode ? */
			{
				switch (SML & 0xfc)
				{
				case 0x48:	/* 7bits, no parity, 1 stop bit */
				case 0x68:	/* 7bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
					cpustate->rxs >>= 16 - 9;
					RXB = (cpustate->rxs >> 1) & 0x7f;
					if ((1 << 8) != (cpustate->rxs & (1 | (1 << 8))))
						IRR |= INTER;	/* framing error */
					break;
				case 0x4c:	/* 8bits, no parity, 1 stop bit */
				case 0x6c:	/* 8bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
					cpustate->rxs >>= 16 - 10;
					RXB = (cpustate->rxs >> 1) & 0xff;
					if ((1 << 9) != (cpustate->rxs & (1 | (1 << 9))))
						IRR |= INTER;	/* framing error */
					break;
				case 0x58:	/* 7bits, odd parity, 1 stop bit */
					cpustate->rxs >>= 16 - 10;
					RXB = (cpustate->rxs >> 1) & 0x7f;
					if ((1 << 9) != (cpustate->rxs & (1 | (1 << 9))))
						IRR |= INTER;	/* framing error */
					if (PAR7(RXB) != ((cpustate->rxs >> 8) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0x5c:	/* 8bits, odd parity, 1 stop bit */
					cpustate->rxs >>= 16 - 11;
					RXB = (cpustate->rxs >> 1) & 0xff;
					if ((1 << 10) != (cpustate->rxs & (1 | (1 << 10))))
						IRR |= INTER;	/* framing error */
					if (PAR8(RXB) != ((cpustate->rxs >> 9) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0x78:	/* 7bits, even parity, 1 stop bit */
					cpustate->rxs >>= 16 - 10;
					RXB = (cpustate->rxs >> 1) & 0x7f;
					if ((1 << 9) != (cpustate->rxs & (1 | (1 << 9))))
						IRR |= INTER;	/* framing error */
					if (PAR7(RXB) != ((cpustate->rxs >> 8) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0x7c:	/* 8bits, even parity, 1 stop bit */
					cpustate->rxs >>= 16 - 11;
					RXB = (cpustate->rxs >> 1) & 0xff;
					if ((1 << 10) != (cpustate->rxs & (1 | (1 << 10))))
						IRR |= INTER;	/* framing error */
					if (PAR8(RXB) != ((cpustate->rxs >> 9) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0xc8:	/* 7bits, no parity, 2 stop bits */
				case 0xe8:	/* 7bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
					cpustate->rxs >>= 16 - 10;
					RXB = (cpustate->rxs >> 1) & 0x7f;
					if ((3 << 9) != (cpustate->rxs & (1 | (3 << 9))))
						IRR |= INTER;	/* framing error */
					if (PAR7(RXB) != ((cpustate->rxs >> 8) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0xcc:	/* 8bits, no parity, 2 stop bits */
				case 0xec:	/* 8bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
					cpustate->rxs >>= 16 - 11;
					RXB = (cpustate->rxs >> 1) & 0xff;
					if ((3 << 10) != (cpustate->rxs & (1 | (3 << 10))))
						IRR |= INTER;	/* framing error */
					if (PAR8(RXB) != ((cpustate->rxs >> 9) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0xd8:	/* 7bits, odd parity, 2 stop bits */
					cpustate->rxs >>= 16 - 11;
					RXB = (cpustate->rxs >> 1) & 0x7f;
					if ((3 << 10) != (cpustate->rxs & (1 | (3 << 10))))
						IRR |= INTER;	/* framing error */
					if (PAR7(RXB) != ((cpustate->rxs >> 8) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0xdc:	/* 8bits, odd parity, 2 stop bits */
					cpustate->rxs >>= 16 - 12;
					RXB = (cpustate->rxs >> 1) & 0xff;
					if ((3 << 11) != (cpustate->rxs & (1 | (3 << 11))))
						IRR |= INTER;	/* framing error */
					if (PAR8(RXB) != ((cpustate->rxs >> 9) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0xf8:	/* 7bits, even parity, 2 stop bits */
					cpustate->rxs >>= 16 - 11;
					RXB = (cpustate->rxs >> 1) & 0x7f;
					if ((3 << 10) != (cpustate->rxs & (1 | (3 << 10))))
						IRR |= INTER;	/* framing error */
					if (PAR7(RXB) != ((cpustate->rxs >> 8) & 1))
						IRR |= INTER;	/* parity error */
					break;
				case 0xfc:	/* 8bits, even parity, 2 stop bits */
					cpustate->rxs >>= 16 - 12;
					RXB = (cpustate->rxs >> 1) & 0xff;
					if ((3 << 11) != (cpustate->rxs & (1 | (3 << 11))))
						IRR |= INTER;	/* framing error */
					if (PAR8(RXB) != ((cpustate->rxs >> 9) & 1))
						IRR |= INTER;	/* parity error */
					break;
				}
			}
			else
			{
				cpustate->rxs >>= 16 - 8;
				RXB = cpustate->rxs;
//              cpustate->rxcnt = 8;
			}
		}
	}
	else
	if (SMH & 0x08) /* receive enable ? */
	{
		if (SML & 0x03) 	/* asynchronous mode ? */
		{
			switch (SML & 0xfc)
			{
			case 0x48:	/* 7bits, no parity, 1 stop bit */
			case 0x68:	/* 7bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				cpustate->rxcnt = 9;
				break;
			case 0x4c:	/* 8bits, no parity, 1 stop bit */
			case 0x6c:	/* 8bits, no parity, 1 stop bit (parity select = 1 but parity is off) */
				cpustate->rxcnt = 10;
				break;
			case 0x58:	/* 7bits, odd parity, 1 stop bit */
				cpustate->rxcnt = 10;
				break;
			case 0x5c:	/* 8bits, odd parity, 1 stop bit */
				cpustate->rxcnt = 11;
				break;
			case 0x78:	/* 7bits, even parity, 1 stop bit */
				cpustate->rxcnt = 10;
				break;
			case 0x7c:	/* 8bits, even parity, 1 stop bit */
				cpustate->rxcnt = 11;
				break;
			case 0xc8:	/* 7bits, no parity, 2 stop bits */
			case 0xe8:	/* 7bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				cpustate->rxcnt = 10;
				break;
			case 0xcc:	/* 8bits, no parity, 2 stop bits */
			case 0xec:	/* 8bits, no parity, 2 stop bits (parity select = 1 but parity is off) */
				cpustate->rxcnt = 11;
				break;
			case 0xd8:	/* 7bits, odd parity, 2 stop bits */
				cpustate->rxcnt = 11;
				break;
			case 0xdc:	/* 8bits, odd parity, 2 stop bits */
				cpustate->rxcnt = 12;
				break;
			case 0xf8:	/* 7bits, even parity, 2 stop bits */
				cpustate->rxcnt = 11;
				break;
			case 0xfc:	/* 8bits, even parity, 2 stop bits */
				cpustate->rxcnt = 12;
				break;
			}
		}
		else
		/* TSK bit set ? */
		if (SMH & 0x40)
		{
			cpustate->rxcnt = 8;
		}
	}
}

void Cupd7907::upd7907_timers(upd7907_state *cpustate, int cycles)
{
    cpustate->pPC->pTIMER->state+=cycles;
	/**** TIMER 0 ****/
	if (TMM & 0x10) 		/* timer 0 upcounter reset ? */
		CNT0 = 0;
	else
	{
		switch (TMM & 0x0c) /* timer 0 clock source */
		{
		case 0x00:	/* clock divided by 12 */
			OVC0 += cycles;
			while (OVC0 >= 12)
			{
				OVC0 -= 12;
				CNT0++;
				if (CNT0 == TM0)
				{
					CNT0 = 0;
					IRR |= INTFT0;
					/* timer F/F source is timer 0 ? */
					if (0x00 == (TMM & 0x03))
					{
						TO ^= 1;
//						if (cpustate->config.io_callback)
//							(*cpustate->config.io_callback)(cpustate->device,UPD7907_TO,TO);
					}
					/* timer 1 chained with timer 0 ? */
					if ((TMM & 0xe0) == 0x60)
					{
						CNT1++;
						if (CNT1 == TM1)
						{
							IRR |= INTFT1;
							CNT1 = 0;
							/* timer F/F source is timer 1 ? */
							if (0x01 == (TMM & 0x03))
							{
								TO ^= 1;
//								if (cpustate->config.io_callback)
//									(*cpustate->config.io_callback)(cpustate->device,UPD7907_TO,TO);
							}
						}
					}
				}
			}
			break;
		case 0x04:	/* clock divided by 384 */
			OVC0 += cycles;
			while (OVC0 >= 384)
			{
				OVC0 -= 384;
				CNT0++;
				if (CNT0 == TM0)
				{
					CNT0 = 0;
					IRR |= INTFT0;
					/* timer F/F source is timer 0 ? */
					if (0x00 == (TMM & 0x03))
					{
						TO ^= 1;
//						if (cpustate->config.io_callback)
//							(*cpustate->config.io_callback)(cpustate->device,UPD7907_TO,TO);
					}
					/* timer 1 chained with timer 0 ? */
					if ((TMM & 0xe0) == 0x60)
					{
						CNT1++;
						if (CNT1 == TM1)
						{
							CNT1 = 0;
							IRR |= INTFT1;
							/* timer F/F source is timer 1 ? */
							if (0x01 == (TMM & 0x03))
							{
								TO ^= 1;
//								if (cpustate->config.io_callback)
//									(*cpustate->config.io_callback)(cpustate->device,UPD7907_TO,TO);
							}
						}
					}
				}
			}
			break;
		case 0x08:	/* external signal at TI */
			break;
		case 0x0c:	/* disabled */
			break;
		}
	}

	/**** TIMER 1 ****/
	if (TMM & 0x80) 		/* timer 1 upcounter reset ? */
		CNT1 = 0;
	else
	{
		switch (TMM & 0x60) /* timer 1 clock source */
		{
		case 0x00:	/* clock divided by 12 */
			OVC1 += cycles;
			while (OVC1 >= 12)
			{
				OVC1 -= 12;
				CNT1++;
				if (CNT1 == TM1)
				{
					CNT1 = 0;
					IRR |= INTFT1;
					/* timer F/F source is timer 1 ? */
					if (0x01 == (TMM & 0x03))
					{
						TO ^= 1;
//						if (cpustate->config.io_callback)
//							(*cpustate->config.io_callback)(cpustate->device,UPD7907_TO,TO);
					}
				}
			}
			break;
		case 0x20:	/* clock divided by 384 */
			OVC1 += cycles;
			while (OVC1 >= 384)
			{
				OVC1 -= 384;
				CNT1++;
				if (CNT1 == TM1)
				{
					CNT1 = 0;
					IRR |= INTFT1;
					/* timer F/F source is timer 1 ? */
					if (0x01 == (TMM & 0x03))
					{
						TO ^= 1;
//						if (cpustate->config.io_callback)
//							(*cpustate->config.io_callback)(cpustate->device,UPD7907_TO,TO);
					}
				}
			}
			break;
		case 0x40:	/* external signal at TI */
			break;
		case 0x60:	/* clocked with timer 0 */
			break;
		}
	}

	/**** TIMER F/F ****/
	/* timer F/F source is clock divided by 3 ? */
	if (0x02 == (TMM & 0x03))
	{
		OVCF += cycles;
		while (OVCF >= 3)
		{
			TO ^= 1;
//			if (cpustate->config.io_callback)
//				(*cpustate->config.io_callback)(cpustate->device,UPD7907_TO,TO);
			OVCF -= 3;
		}
	}

	/**** ETIMER ****/
	/* ECNT clear */
	if (0x00 == (ETMM & 0x0c))
		ECNT = 0;
	else
	if (0x00 == (ETMM & 0x03) || (0x01 == (ETMM & 0x03) && CI))
	{
		OVCE += cycles;
		/* clock divided by 12 */
		while (OVCE >= 12)
		{
			OVCE -= 12;
			ECNT++;
			switch (ETMM & 0x0c)
			{
			case 0x00:				/* clear ECNT */
				break;
			case 0x04:				/* free running */
				if (0 == ECNT)
					ITF |= INTOV;	/* set overflow flag if counter wrapped */
				break;
			case 0x08:				/* reset at falling edge of CI or TO */
				break;
			case 0x0c:				/* reset if ECNT == ETM1 */
				if (ETM1 == ECNT)
					ECNT = 0;
				break;
			}
			switch (ETMM & 0x30)
			{
			case 0x00:	/* set CO0 if ECNT == ETM0 */
				if (ETM0 == ECNT)
				{
					switch (EOM & 0x0e)
					{
					case 0x02:	/* toggle CO0 */
						CO0 = (CO0 >> 1) | ((CO0 ^ 2) & 2);
						break;
					case 0x04:	/* reset CO0 */
						CO0 = 0;
						break;
					case 0x08:	/* set CO0 */
						CO0 = 1;
						break;
					}
				}
				break;
			case 0x10:	/* prohibited */
				break;
			case 0x20:	/* set CO0 if ECNT == ETM0 or at falling CI input */
				if (ETM0 == ECNT)
				{
					switch (EOM & 0x0e)
					{
					case 0x02:	/* toggle CO0 */
						CO0 = (CO0 >> 1) | ((CO0 ^ 2) & 2);
						break;
					case 0x04:	/* reset CO0 */
						CO0 = 0;
						break;
					case 0x08:	/* set CO0 */
						CO0 = 1;
						break;
					}
				}
				break;
			case 0x30:	/* latch CO0 if ECNT == ETM0 or ECNT == ETM1 */
				if (ETM0 == ECNT || ETM1 == ECNT)
				{
					switch (EOM & 0x0e)
					{
					case 0x02:	/* toggle CO0 */
						CO0 = (CO0 >> 1) | ((CO0 ^ 2) & 2);
						break;
					case 0x04:	/* reset CO0 */
						CO0 = 0;
						break;
					case 0x08:	/* set CO0 */
						CO0 = 1;
						break;
					}
				}
				break;
			}
			switch (ETMM & 0xc0)
			{
			case 0x00:	/* lacth CO1 if ECNT == ETM1 */
				if (ETM1 == ECNT)
				{
					switch (EOM & 0xe0)
					{
					case 0x20:	/* toggle CO1 */
						CO1 = (CO1 >> 1) | ((CO1 ^ 2) & 2);
						break;
					case 0x40:	/* reset CO1 */
						CO1 = 0;
						break;
					case 0x80:	/* set CO1 */
						CO1 = 1;
						break;
					}
				}
				break;
			case 0x40:	/* prohibited */
				break;
			case 0x80:	/* latch CO1 if ECNT == ETM1 or falling edge of CI input */
				if (ETM1 == ECNT)
				{
					switch (EOM & 0xe0)
					{
					case 0x20:	/* toggle CO1 */
						CO1 = (CO1 >> 1) | ((CO1 ^ 2) & 2);
						break;
					case 0x40:	/* reset CO1 */
						CO1 = 0;
						break;
					case 0x80:	/* set CO1 */
						CO1 = 1;
						break;
					}
				}
				break;
			case 0xc0:	/* latch CO1 if ECNT == ETM0 or ECNT == ETM1 */
				if (ETM0 == ECNT || ETM1 == ECNT)
				{
					switch (EOM & 0xe0)
					{
					case 0x20:	/* toggle CO1 */
						CO1 = (CO1 >> 1) | ((CO1 ^ 2) & 2);
						break;
					case 0x40:	/* reset CO1 */
						CO1 = 0;
						break;
					case 0x80:	/* set CO1 */
						CO1 = 1;
						break;
					}
				}
				break;
			}
		}
	}

	/**** SIO ****/
	switch (SMH & 0x03)
	{
	case 0x00:		/* interval timer F/F */
		break;
	case 0x01:		/* internal clock divided by 384 */
		OVCS += cycles;
		while (OVCS >= 384)
		{
			OVCS -= 384;
			if (0 == (EDGES ^= 1))
                upd7907_sio_input(cpustate);
			else
                upd7907_sio_output(cpustate);
		}
		break;
	case 0x02:		/* internal clock divided by 24 */
		OVCS += cycles;
		while (OVCS >= 24)
		{
			OVCS -= 24;
			if (0 == (EDGES ^= 1))
                upd7907_sio_input(cpustate);
			else
                upd7907_sio_output(cpustate);
		}
		break;
	}
}

void Cupd7907::upd7801_timers(upd7907_state *cpustate, int cycles)
{
	if ( cpustate->ovc0 )
	{
		cpustate->ovc0 -= cycles;

		/* Check if timer expired */
		if ( cpustate->ovc0 <= 0 )
		{
			IRR |= INTFT0;

			/* Reset the timer flip/fliop */
			TO = 0;
//			if ( cpustate->config.io_callback)
//				(*cpustate->config.io_callback)(cpustate->device,UPD7907_TO,TO);

			/* Reload the timer */
			cpustate->ovc0 = 16 * ( TM0 + ( ( TM1 & 0x0f ) << 8 ) );
		}
	}
}

void Cupd7907::upd78c05_timers(upd7907_state *cpustate, int cycles)
{
    cpustate->pPC->pTIMER->state+=cycles;
	if ( cpustate->ovc0 ) {
		cpustate->ovc0 -= cycles;

		if ( cpustate->ovc0 <= 0 ) {
			IRR |= INTFT0;
			if (0x00 == (TMM & 0x03)) {
				TO ^= 1;
//				if (cpustate->config.io_callback)
//					(*cpustate->config.io_callback)(cpustate->device,UPD7907_TO,TO);
			}

			while ( cpustate->ovc0 <= 0 ) {
				cpustate->ovc0 += ( ( TMM & 0x04 ) ? 16 * 8 : 8 ) * TM0;
			}
		}
	}
}



#include "7907tbl.cpp"

void Cupd7907::illegal(upd7907_state *cpustate)
{
//    logerror("uPD7810 '%s': illegal opcode %02x at PC:%04x\n", cpustate->device->tag(), OP, PC);
}

void Cupd7907::Cupd7907::illegal2(upd7907_state *cpustate)
{
//    logerror("uPD7810 '%s': illegal opcode %02x %02x at PC:%04x\n", cpustate->device->tag(), OP, OP2, PC);
}

/* prefix 48 */

/* 48 01: 0100 1000 0000 0001 */
void Cupd7907::Cupd7907::SLRC_A(upd7907_state *cpustate)
{
    PSW = (PSW & ~CY) | (A & CY);
    A >>= 1;
    SKIP_CY;
}

/* 48 02: 0100 1000 0000 0010 */
void Cupd7907::Cupd7907::SLRC_B(upd7907_state *cpustate)
{
    PSW = (PSW & ~CY) | (B & CY);
    B >>= 1;
    SKIP_CY;
}

/* 48 03: 0100 1000 0000 0011 */
void Cupd7907::Cupd7907::SLRC_C(upd7907_state *cpustate)
{
    PSW = (PSW & ~CY) | (C & CY);
    C >>= 1;
    SKIP_CY;
}


/* 48 05: 0100 1000 0000 0101 */
void Cupd7907::Cupd7907::SLLC_A(upd7907_state *cpustate)
{
    PSW = (PSW & ~CY) | ((A >> 7) & CY);
    A <<= 1;
    SKIP_CY;
}

/* 48 06: 0100 1000 0000 0110 */
void Cupd7907::Cupd7907::SLLC_B(upd7907_state *cpustate)
{
    PSW = (PSW & ~CY) | ((B >> 7) & CY);
    B <<= 1;
    SKIP_CY;
}

/* 48 07: 0100 1000 0000 0111 */
void Cupd7907::Cupd7907::SLLC_C(upd7907_state *cpustate)
{
    PSW = (PSW & ~CY) | ((C >> 7) & CY);
    C <<= 1;
    SKIP_CY;
}

/* 48 08: 0100 1000 0000 1000 */
void Cupd7907::Cupd7907::SK_NV(upd7907_state *cpustate)
{
    /* 48 skip never */
}

/* 48 0a: 0100 1000 0000 1010 */
void Cupd7907::SK_CY(upd7907_state *cpustate)
{
    if (CY == (PSW & CY))
        PSW |= SK;
}

/* 48 0b: 0100 1000 0000 1011 */
void Cupd7907::SK_HC(upd7907_state *cpustate)
{
    if (HC == (PSW & HC))
        PSW |= SK;
}

/* 48 0c: 0100 1000 0000 1100 */
void Cupd7907::SK_Z(upd7907_state *cpustate)
{
    if (Z == (PSW & Z))
        PSW |= SK;
}

/* 48 18: 0100 1000 0001 1000 */
void Cupd7907::SKN_NV(upd7907_state *cpustate)
{
    /* skip not never -> skip always ;-) */
    PSW |= SK;
}

/* 48 1a: 0100 1000 0001 1010 */
void Cupd7907::SKN_CY(upd7907_state *cpustate)
{
    if (0 == (PSW & CY))
        PSW |= SK;
}

/* 48 1b: 0100 1000 0001 1011 */
void Cupd7907::SKN_HC(upd7907_state *cpustate)
{
    if (0 == (PSW & HC))
        PSW |= SK;
}

/* 48 1c: 0100 1000 0001 1100 */
void Cupd7907::SKN_Z(upd7907_state *cpustate)
{
    if (0 == (PSW & Z))
        PSW |= SK;
}

/* 48 21: 0100 1000 0010 0001 */
void Cupd7907::SLR_A(upd7907_state *cpustate)
{
    PSW = (PSW & ~CY) | (A & CY);
    A >>= 1;
}

/* 48 22: 0100 1000 0010 0010 */
void Cupd7907::SLR_B(upd7907_state *cpustate)
{
    PSW = (PSW & ~CY) | (B & CY);
    B >>= 1;
}

/* 48 23: 0100 1000 0010 0011 */
void Cupd7907::SLR_C(upd7907_state *cpustate)
{
    PSW = (PSW & ~CY) | (C & CY);
    C >>= 1;
}

/* 48 25: 0100 1000 0010 0101 */
void Cupd7907::SLL_A(upd7907_state *cpustate)
{
    PSW = (PSW & ~CY) | ((A >> 7) & CY);
    A <<= 1;
}

/* 48 26: 0100 1000 0010 0110 */
void Cupd7907::SLL_B(upd7907_state *cpustate)
{
    PSW = (PSW & ~CY) | ((B >> 7) & CY);
    B <<= 1;
}

/* 48 27: 0100 1000 0010 0111 */
void Cupd7907::SLL_C(upd7907_state *cpustate)
{
    PSW = (PSW & ~CY) | ((C >> 7) & CY);
    C <<= 1;
}

/* 48 28: 0100 1000 0010 1000 */
void Cupd7907::JEA(upd7907_state *cpustate)
{
    PC = EA;
}

/* 48 29: 0100 1000 0010 1001 */
void Cupd7907::CALB(upd7907_state *cpustate)
{
    SP--;
    WM( SPD, PCH );
    SP--;
    WM( SPD, PCL );

    PC = BC;
    cpustate->pPC->pCPU->CallSubLevel++;
}

/* 48 2a: 0100 1000 0010 1010 */
void Cupd7907::CLC(upd7907_state *cpustate)
{
    PSW &= ~CY;
}

/* 48 2b: 0100 1000 0010 1011 */
void Cupd7907::STC(upd7907_state *cpustate)
{
    PSW |= CY;
}

/* 48 2d: 0100 1000 0010 1101 */
void Cupd7907::MUL_A(upd7907_state *cpustate)
{
    EA = A * A;
}

/* 48 2e: 0100 1000 0010 1110 */
void Cupd7907::MUL_B(upd7907_state *cpustate)
{
    EA = A * B;
}

/* 48 2f: 0100 1000 0010 1111 */
void Cupd7907::MUL_C(upd7907_state *cpustate)
{
    EA = A * C;
}

/* 48 31: 0100 1000 0011 0001 */
void Cupd7907::RLR_A(upd7907_state *cpustate)
{
    UINT8 carry=(PSW&CY)<<7;
    PSW = (PSW & ~CY) | (A & CY);
    A = (A >> 1) | carry;
}

/* 48 32: 0100 1000 0011 0010 */
void Cupd7907::RLR_B(upd7907_state *cpustate)
{
    UINT8 carry=(PSW&CY)<<7;
    PSW = (PSW & ~CY) | (B & CY);
    B = (B >> 1) | carry;
}

/* 48 33: 0100 1000 0011 0011 */
void Cupd7907::RLR_C(upd7907_state *cpustate)
{
    UINT8 carry=(PSW&CY)<<7;
    PSW = (PSW & ~CY) | (C & CY);
    C = (C >> 1) | carry;
}

/* 48 35: 0100 1000 0011 0101 */
void Cupd7907::RLL_A(upd7907_state *cpustate)
{
    UINT8 carry=PSW&CY;
    PSW = (PSW & ~CY) | ((A >> 7) & CY);
    A = (A << 1) | carry;
}

/* 48 36: 0100 1000 0011 0110 */
void Cupd7907::RLL_B(upd7907_state *cpustate)
{
    UINT8 carry=PSW&CY;
    PSW = (PSW & ~CY) | ((B >> 7) & CY);
    B = (B << 1) | carry;
}

/* 48 37: 0100 1000 0011 0111 */
void Cupd7907::RLL_C(upd7907_state *cpustate)
{
    UINT8 carry=PSW&CY;
    PSW = (PSW & ~CY) | ((C >> 7) & CY);
    C = (C << 1) | carry;
}

/* 48 38: 0100 1000 0011 1000 */
void Cupd7907::RLD(upd7907_state *cpustate)
{
    UINT8 m = RM( HL ), tmp;
    tmp = (m << 4) | (A & 0x0f);
    A = (A & 0xf0) | (m >> 4);
    WM( HL, tmp );
}

/* 48 39: 0100 1000 0011 1001 */
void Cupd7907::RRD(upd7907_state *cpustate)
{
    UINT8 m = RM( HL ), tmp;
    tmp = (A << 4) | (m >> 4);
    A = (A & 0xf0) | (m & 0x0f);
    WM( HL, tmp );
}

/* 48 3a: 0100 1000 0011 1010 */
void Cupd7907::NEGA(upd7907_state *cpustate)
{
    A = ~A + 1;
}

/* 48 3b: 0100 1000 0011 1011 */
void Cupd7907::HALT(upd7907_state *cpustate)
{
    int cycles = (cpustate->icount / 4) * 4;
    cpustate->icount -= cycles;

    cpustate->handle_timers(cpustate, cycles);
    PC -= 1;		/* continue executing HALT */
}

/* 48 3d: 0100 1000 0011 1101 */
void Cupd7907::DIV_A(upd7907_state *cpustate)
{
    if (A)
    {
        UINT8 remainder;
        remainder = EA % A;
        EA /= A;
        A = remainder;
    }
    else
        EA = 0xffff;	/* guess */
}

/* 48 3e: 0100 1000 0011 1110 */
void Cupd7907::DIV_B(upd7907_state *cpustate)
{
    if (B)
    {
        UINT8 remainder;
        remainder = EA % B;
        EA /= B;
        B = remainder;
    }
    else
        EA = 0xffff;	/* guess */
}

/* 48 3f: 0100 1000 0011 1111 */
void Cupd7907::DIV_C(upd7907_state *cpustate)
{
    if (C)
    {
        UINT8 remainder;
        remainder = EA % C;
        EA /= C;
        C = remainder;
    }
    else
        EA = 0xffff;	/* guess */
}

/* 48 40: 0100 1000 0100 0000 */
void Cupd7907::SKIT_NMI(upd7907_state *cpustate)
{
    if (IRR & INTNMI)
        PSW |= SK;
    IRR &= ~INTNMI;
}

/* 48 41: 0100 1000 0100 0001 */
void Cupd7907::SKIT_FT0(upd7907_state *cpustate)
{
    if (IRR & INTFT0)
        PSW |= SK;
    IRR &= ~INTFT0;
}

/* 48 42: 0100 1000 0100 0010 */
void Cupd7907::SKIT_FT1(upd7907_state *cpustate)
{
    if (IRR & INTFT1)
        PSW |= SK;
    IRR &= ~INTFT1;
}

/* 48 43: 0100 1000 0100 0011 */
void Cupd7907::SKIT_F1(upd7907_state *cpustate)
{
    if (IRR & INTF1)
        PSW |= SK;
    IRR &= ~INTF1;
}

/* 48 44: 0100 1000 0100 0100 */
void Cupd7907::SKIT_F2(upd7907_state *cpustate)
{
    if (IRR & INTF2)
        PSW |= SK;
    IRR &= ~INTF2;
}

/* 48 45: 0100 1000 0100 0101 */
void Cupd7907::SKIT_FE0(upd7907_state *cpustate)
{
    if (IRR & INTFE0)
        PSW |= SK;
    IRR &= ~INTFE0;
}

/* 48 46: 0100 1000 0100 0110 */
void Cupd7907::SKIT_FE1(upd7907_state *cpustate)
{
    if (IRR & INTFE1)
        PSW |= SK;
    IRR &= ~INTFE1;
}

/* 48 47: 0100 1000 0100 0111 */
void Cupd7907::SKIT_FEIN(upd7907_state *cpustate)
{
    if (IRR & INTFEIN)
        PSW |= SK;
    IRR &= ~INTFEIN;
}

/* 48 48: 0100 1000 0100 1000 */
void Cupd7907::SKIT_FAD(upd7907_state *cpustate)
{
    if (IRR & INTFAD)
        PSW |= SK;
    IRR &= ~INTFAD;
}

/* 48 49: 0100 1000 0100 1001 */
void Cupd7907::SKIT_FSR(upd7907_state *cpustate)
{
    if (IRR & INTFSR)
        PSW |= SK;
    IRR &= ~INTFSR;
}

/* 48 4a: 0100 1000 0100 1010 */
void Cupd7907::SKIT_FST(upd7907_state *cpustate)
{
    if (IRR & INTFST)
        PSW |= SK;
    IRR &= ~INTFST;
}

/* 48 4b: 0100 1000 0100 1011 */
void Cupd7907::SKIT_ER(upd7907_state *cpustate)
{
    if (IRR & INTER)
        PSW |= SK;
    IRR &= ~INTER;
}

/* 48 4c: 0100 1000 0100 1100 */
void Cupd7907::SKIT_OV(upd7907_state *cpustate)
{
    if (IRR & INTOV)
        PSW |= SK;
    IRR &= ~INTOV;
}

/* 48 50: 0100 1000 0101 0000 */
void Cupd7907::SKIT_AN4(upd7907_state *cpustate)
{
    if (ITF & INTAN4)
        PSW |= SK;
    ITF &= ~INTAN4;
}

/* 48 51: 0100 1000 0101 0001 */
void Cupd7907::SKIT_AN5(upd7907_state *cpustate)
{
    if (ITF & INTAN5)
        PSW |= SK;
    ITF &= ~INTAN5;
}

/* 48 52: 0100 1000 0101 0010 */
void Cupd7907::SKIT_AN6(upd7907_state *cpustate)
{
    if (ITF & INTAN6)
        PSW |= SK;
    ITF &= ~INTAN6;
}

/* 48 53: 0100 1000 0101 0011 */
void Cupd7907::SKIT_AN7(upd7907_state *cpustate)
{
    if (ITF & INTAN7)
        PSW |= SK;
    ITF &= ~INTAN7;
}

/* 48 54: 0100 1000 0101 0100 */
void Cupd7907::SKIT_SB(upd7907_state *cpustate)
{
    if (ITF & INTSB)
        PSW |= SK;
    ITF &= ~INTSB;
}

/* 48 60: 0100 1000 0110 0000 */
void Cupd7907::SKNIT_NMI(upd7907_state *cpustate)
{
    if (0 == (IRR & INTNMI))
        PSW |= SK;
    IRR &= ~INTNMI;
}

/* 48 61: 0100 1000 0110 0001 */
void Cupd7907::SKNIT_FT0(upd7907_state *cpustate)
{
    if (0 == (IRR & INTFT0))
        PSW |= SK;
    IRR &= ~INTFT0;
}

/* 48 62: 0100 1000 0110 0010 */
void Cupd7907::SKNIT_FT1(upd7907_state *cpustate)
{
    if (0 == (IRR & INTFT1))
        PSW |= SK;
    IRR &= ~INTFT1;
}

/* 48 63: 0100 1000 0110 0011 */
void Cupd7907::SKNIT_F1(upd7907_state *cpustate)
{
    if (0 == (IRR & INTF1))
        PSW |= SK;
    IRR &= ~INTF1;
}

/* 48 64: 0100 1000 0110 0100 */
void Cupd7907::SKNIT_F2(upd7907_state *cpustate)
{
    if (0 == (IRR & INTF2))
        PSW |= SK;
    IRR &= ~INTF2;
}

/* 48 65: 0100 1000 0110 0101 */
void Cupd7907::SKNIT_FE0(upd7907_state *cpustate)
{
    if (0 == (IRR & INTFE0))
        PSW |= SK;
    IRR &= ~INTFE0;
}

/* 48 66: 0100 1000 0110 0110 */
void Cupd7907::SKNIT_FE1(upd7907_state *cpustate)
{
    if (0 == (IRR & INTFE1))
        PSW |= SK;
    IRR &= ~INTFE1;
}

/* 48 67: 0100 1000 0110 0111 */
void Cupd7907::SKNIT_FEIN(upd7907_state *cpustate)
{
    if (0 == (IRR & INTFEIN))
        PSW |= SK;
    IRR &= ~INTFEIN;
}

/* 48 68: 0100 1000 0110 1000 */
void Cupd7907::SKNIT_FAD(upd7907_state *cpustate)
{
    if (0 == (IRR & INTFAD))
        PSW |= SK;
    IRR &= ~INTFAD;
}

/* 48 69: 0100 1000 0110 1001 */
void Cupd7907::SKNIT_FSR(upd7907_state *cpustate)
{
    if (0 == (IRR & INTFSR))
        PSW |= SK;
    IRR &= ~INTFSR;
}

/* 48 6a: 0100 1000 0110 1010 */
void Cupd7907::SKNIT_FST(upd7907_state *cpustate)
{
    if (0 == (IRR & INTFST))
        PSW |= SK;
    IRR &= ~INTFST;
}

/* 48 6b: 0100 1000 0110 1011 */
void Cupd7907::SKNIT_ER(upd7907_state *cpustate)
{
    if (0 == (IRR & INTER))
        PSW |= SK;
    IRR &= ~INTER;
}

/* 48 6c: 0100 1000 0110 1100 */
void Cupd7907::SKNIT_OV(upd7907_state *cpustate)
{
    if (0 == (IRR & INTOV))
        PSW |= SK;
    IRR &= ~INTOV;
}

/* 48 70: 0100 1000 0111 0000 */
void Cupd7907::SKNIT_AN4(upd7907_state *cpustate)
{
    if (0 == (ITF & INTAN4))
        PSW |= SK;
    ITF &= ~INTAN4;
}

/* 48 71: 0100 1000 0111 0001 */
void Cupd7907::SKNIT_AN5(upd7907_state *cpustate)
{
    if (0 == (ITF & INTAN5))
        PSW |= SK;
    ITF &= ~INTAN5;
}

/* 48 72: 0100 1000 0111 0010 */
void Cupd7907::SKNIT_AN6(upd7907_state *cpustate)
{
    if (0 == (ITF & INTAN6))
        PSW |= SK;
    ITF &= ~INTAN6;
}

/* 48 73: 0100 1000 0111 0011 */
void Cupd7907::SKNIT_AN7(upd7907_state *cpustate)
{
    if (0 == (ITF & INTAN7))
        PSW |= SK;
    ITF &= ~INTAN7;
}

/* 48 74: 0100 1000 0111 0100 */
void Cupd7907::SKNIT_SB(upd7907_state *cpustate)
{
    if (0 == (ITF & INTSB))
        PSW |= SK;
    ITF &= ~INTSB;
}

/* 48 82: 0100 1000 1000 0010 */
void Cupd7907::LDEAX_D(upd7907_state *cpustate)
{
    EAL = RM( DE );
    EAH = RM( DE + 1 );
}

/* 48 83: 0100 1000 1000 0011 */
void Cupd7907::LDEAX_H(upd7907_state *cpustate)
{
    EAL = RM( HL );
    EAH = RM( HL + 1 );
}

/* 48 84: 0100 1000 1000 0100 */
void Cupd7907::LDEAX_Dp(upd7907_state *cpustate)
{
    EAL = RM( DE );
    EAH = RM( DE + 1 );
    DE += 2;
}

/* 48 85: 0100 1000 1000 0101 */
void Cupd7907::LDEAX_Hp(upd7907_state *cpustate)
{
    EAL = RM( HL );
    EAH = RM( HL + 1 );
    HL += 2;
}

/* 48 8b: 0100 1000 1000 1011 xxxx xxxx */
void Cupd7907::LDEAX_D_xx(upd7907_state *cpustate)
{
    UINT16 ea;
    RDOPARG( ea );
    ea += DE;
    EAL = RM( ea );
    EAH = RM( ea + 1 );
}

/* 48 8c: 0100 1000 1000 1100 */
void Cupd7907::LDEAX_H_A(upd7907_state *cpustate)
{
    UINT16 ea = HL + A;
    EAL = RM( ea );
    EAH = RM( ea + 1 );
}

/* 48 8d: 0100 1000 1000 1101 */
void Cupd7907::LDEAX_H_B(upd7907_state *cpustate)
{
    UINT16 ea = HL + B;
    EAL = RM( ea );
    EAH = RM( ea + 1 );
}

/* 48 8e: 0100 1000 1000 1110 */
void Cupd7907::LDEAX_H_EA(upd7907_state *cpustate)
{
    UINT16 ea = HL + EA;
    EAL = RM( ea );
    EAH = RM( ea + 1 );
}

/* 48 8f: 0100 1000 1000 1111 xxxx xxxx */
void Cupd7907::LDEAX_H_xx(upd7907_state *cpustate)
{
    UINT16 ea;
    RDOPARG( ea );
    ea += HL;
    EAL = RM( ea );
    EAH = RM( ea + 1 );
}

/* 48 92: 0100 1000 1000 0010 */
void Cupd7907::STEAX_D(upd7907_state *cpustate)
{
    WM( DE, EAL );
    WM( DE + 1, EAH );
}

/* 48 93: 0100 1000 1000 0011 */
void Cupd7907::STEAX_H(upd7907_state *cpustate)
{
    WM( HL, EAL );
    WM( HL + 1, EAH );
}

/* 48 94: 0100 1000 1000 0100 */
void Cupd7907::STEAX_Dp(upd7907_state *cpustate)
{
    WM( DE, EAL );
    WM( DE + 1, EAH );
    DE += 2;
}

/* 48 95: 0100 1000 1000 0101 */
void Cupd7907::STEAX_Hp(upd7907_state *cpustate)
{
    WM( HL, EAL );
    WM( HL + 1, EAH );
    HL += 2;
}

/* 48 9b: 0100 1000 1000 1011 xxxx xxxx */
void Cupd7907::STEAX_D_xx(upd7907_state *cpustate)
{
    UINT16 ea;
    RDOPARG( ea );
    ea += DE;
    WM( ea, EAL );
    WM( ea + 1, EAH );
}

/* 48 9c: 0100 1000 1000 1100 */
void Cupd7907::STEAX_H_A(upd7907_state *cpustate)
{
    UINT16 ea = HL + A;
    WM( ea, EAL );
    WM( ea + 1, EAH );
}

/* 48 9d: 0100 1000 1000 1101 */
void Cupd7907::STEAX_H_B(upd7907_state *cpustate)
{
    UINT16 ea = HL + B;
    WM( ea, EAL );
    WM( ea + 1, EAH );
}

/* 48 9e: 0100 1000 1000 1110 */
void Cupd7907::STEAX_H_EA(upd7907_state *cpustate)
{
    UINT16 ea = HL + EA;
    WM( ea, EAL );
    WM( ea + 1, EAH );
}

/* 48 9f: 0100 1000 1000 1111 xxxx xxxx */
void Cupd7907::STEAX_H_xx(upd7907_state *cpustate)
{
    UINT16 ea;
    RDOPARG( ea );
    ea += HL;
    WM( ea, EAL );
    WM( ea + 1, EAH );
}

/* 48 a0: 0100 1000 1010 0000 */
void Cupd7907::DSLR_EA(upd7907_state *cpustate)
{
    PSW = (PSW & ~CY) | (EA & CY);
    EA >>= 1;
}

/* 48 a4: 0100 1000 1010 0100 */
void Cupd7907::DSLL_EA(upd7907_state *cpustate)
{
    PSW = (PSW & ~CY) | ((EA >> 15) & CY);
    EA <<= 1;
}

/* 48 a8: 0100 1000 1010 1000 */
void Cupd7907::TABLE(upd7907_state *cpustate)
{
    UINT16 ea = PC + A + 1;
    C = RM( ea );
    B = RM( ea + 1 );
}

/* 48 b0: 0100 1000 1011 0000 */
void Cupd7907::DRLR_EA(upd7907_state *cpustate)
{
    UINT8 carry=PSW&CY;
    PSW = (PSW & ~CY) | (EA & CY);
    EA = (EA >> 1) | (carry << 15);
}

/* 48 b4: 0100 1000 1011 0100 */
void Cupd7907::DRLL_EA(upd7907_state *cpustate)
{
    UINT8 carry=PSW&CY;
    PSW = (PSW & ~CY) | ((EA >> 15) & CY);
    EA = (EA << 1) | carry;
}

/* 48 bb: 0100 1000 1011 1011 */
void Cupd7907::STOP(upd7907_state *cpustate)
{
    int cycles = (cpustate->icount / 4) * 4;
    cpustate->icount -= cycles;
    cpustate->handle_timers(cpustate, cycles);
    PC -= 1;
}

/* 48 c0: 0100 1000 1100 0000 */
void Cupd7907::DMOV_EA_ECNT(upd7907_state *cpustate)
{
    EA = ECNT;
}

/* 48 c1: 0100 1000 1100 0001 */
void Cupd7907::DMOV_EA_ECPT(upd7907_state *cpustate)
{
    EA = ECPT;
}

/* 48 d2: 0100 1000 1101 0010 */
void Cupd7907::DMOV_ETM0_EA(upd7907_state *cpustate)
{
    ETM0 = EA;
}

/* 48 d3: 0100 1000 1101 0011 */
void Cupd7907::DMOV_ETM1_EA(upd7907_state *cpustate)
{
    ETM1 = EA;
}

/* prefix 4C */
/* 4c c0: 0100 1100 1100 0000 */
void Cupd7907::MOV_A_PA(upd7907_state *cpustate)
{
    A = RP( cpustate, UPD7907_PORTA );
}

/* 4c c1: 0100 1100 1100 0001 */
void Cupd7907::MOV_A_PB(upd7907_state *cpustate)
{
    A = RP( cpustate, UPD7907_PORTB );
}

/* 4c c2: 0100 1100 1100 0010 */
void Cupd7907::MOV_A_PC(upd7907_state *cpustate)
{
    A = RP( cpustate, UPD7907_PORTC );
}

/* 4c c3: 0100 1100 1100 0011 */
void Cupd7907::MOV_A_PD(upd7907_state *cpustate)
{
    A = RP( cpustate, UPD7907_PORTD );
}

/* 4c c5: 0100 1100 1100 0101 */
void Cupd7907::MOV_A_PF(upd7907_state *cpustate)
{
    A = RP( cpustate, UPD7907_PORTF );
}

/* 4c c6: 0100 1100 1100 0110 */
void Cupd7907::MOV_A_MKH(upd7907_state *cpustate)
{
    A = MKH;
}

/* 4c c7: 0100 1100 1100 0111 */
void Cupd7907::MOV_A_MKL(upd7907_state *cpustate)
{
    A = MKL;
}

/* 4c c8: 0100 1100 1100 1000 */
void Cupd7907::MOV_A_ANM(upd7907_state *cpustate)
{
    A = ANM;
}

/* 4c c9: 0100 1100 1100 1001 */
void Cupd7907::MOV_A_SMH(upd7907_state *cpustate)
{
    A = SMH;
}

/* 4c cb: 0100 1100 1100 1011 */
void Cupd7907::MOV_A_EOM(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM ;//& 0x22;
    A = eom;
}

/* 4c cd: 0100 1100 1100 1101 */
void Cupd7907::MOV_A_TMM(upd7907_state *cpustate)
{
    A = TMM;
}

/* 4c ce: 0100 1100 1110 0000 (7807 only) */
void Cupd7907::MOV_A_PT(upd7907_state *cpustate)
{
    A = RP( cpustate, UPD7907_PORTT );
}

/* 4c d9: 0100 1100 1101 1001 */
void Cupd7907::MOV_A_RXB(upd7907_state *cpustate)
{
    A = RXB;
}

/* 4c e0: 0100 1100 1110 0000 */
void Cupd7907::MOV_A_CR0(upd7907_state *cpustate)
{
    A = CR0;
}

/* 4c e1: 0100 1100 1110 0001 */
void Cupd7907::MOV_A_CR1(upd7907_state *cpustate)
{
    A = CR1;
}

/* 4c e2: 0100 1100 1110 0010 */
void Cupd7907::MOV_A_CR2(upd7907_state *cpustate)
{
    A = CR2;
}

/* 4c e3: 0100 1100 1110 0011 */
void Cupd7907::MOV_A_CR3(upd7907_state *cpustate)
{
    A = CR3;
}

/* prefix 4D */
/* 4d c0: 0100 1101 1100 0000 */
void Cupd7907::MOV_PA_A(upd7907_state *cpustate)
{
    WP( cpustate, UPD7907_PORTA, A );
}

/* 4d c1: 0100 1101 1100 0001 */
void Cupd7907::MOV_PB_A(upd7907_state *cpustate)
{
    WP( cpustate, UPD7907_PORTB, A );
}

/* 4d c2: 0100 1101 1100 0010 */
void Cupd7907::MOV_PC_A(upd7907_state *cpustate)
{
    WP( cpustate, UPD7907_PORTC, A );
}

/* 4d c3: 0100 1101 1100 0011 */
void Cupd7907::MOV_PD_A(upd7907_state *cpustate)
{
    WP( cpustate, UPD7907_PORTD, A );
}

/* 4d c5: 0100 1101 1100 0101 */
void Cupd7907::MOV_PF_A(upd7907_state *cpustate)
{
    WP( cpustate, UPD7907_PORTF, A );
}

/* 4d c6: 0100 1101 1100 0110 */
void Cupd7907::MOV_MKH_A(upd7907_state *cpustate)
{
    MKH = A;
}

/* 4d c7: 0100 1101 1100 0111 */
void Cupd7907::MOV_MKL_A(upd7907_state *cpustate)
{
    MKL = A;
}

/* 4d c8: 0100 1101 1100 1000 */
void Cupd7907::MOV_ANM_A(upd7907_state *cpustate)
{
    ANM = A;
}

/* 4d c9: 0100 1101 1100 1001 */
void Cupd7907::MOV_SMH_A(upd7907_state *cpustate)
{
    SMH = A;
}

/* 4d ca: 0100 1101 1100 1010 */
void Cupd7907::MOV_SML_A(upd7907_state *cpustate)
{
    SML = A;
}

/* 4d cb: 0100 1101 1100 1011 */
void Cupd7907::MOV_EOM_A(upd7907_state *cpustate)
{
    EOM = A;
    upd7907_write_EOM(cpustate);
}

/* 4d cc: 0100 1101 1100 1100 */
void Cupd7907::MOV_ETMM_A(upd7907_state *cpustate)
{
    ETMM = A;
}

/* 4d cd: 0100 1101 1100 1101 */
void Cupd7907::MOV_TMM_A(upd7907_state *cpustate)
{
    TMM = A;
}

/* 4d d0: 0100 1101 1101 0000 */
void Cupd7907::MOV_MM_A(upd7907_state *cpustate)
{
    MM = A;
}

/* 4d d1: 0100 1101 1101 0001 */
void Cupd7907::MOV_MCC_A(upd7907_state *cpustate)
{
    MCC = A;
}

/* 4d d2: 0100 1101 1101 0010 */
void Cupd7907::MOV_MA_A(upd7907_state *cpustate)
{
    MA = A;
}

/* 4d d3: 0100 1101 1101 0011 */
void Cupd7907::MOV_MB_A(upd7907_state *cpustate)
{
    MB = A;
}

/* 4d d4: 0100 1101 1101 0100 */
void Cupd7907::MOV_MC_A(upd7907_state *cpustate)
{
    MC = A;
}

/* 4d d7: 0100 1101 1101 0111 */
void Cupd7907::MOV_MF_A(upd7907_state *cpustate)
{
    MF = A;
}

/* 4d d8: 0100 1101 1101 1000 */
void Cupd7907::MOV_TXB_A(upd7907_state *cpustate)
{
    TXB = A;
    upd7907_write_TXB(cpustate);
}

/* 4d da: 0100 1101 1101 1010 */
void Cupd7907::MOV_TM0_A(upd7907_state *cpustate)
{
    TM0 = A;
}

/* 4d db: 0100 1101 1101 1011 */
void Cupd7907::MOV_TM1_A(upd7907_state *cpustate)
{
    TM1 = A;
}

/* 4d e8: 0100 1101 1110 1000 */
void Cupd7907::MOV_ZCM_A(upd7907_state *cpustate)
{
    ZCM = A;
}

/* prefix 60 */
/* 60 08: 0110 0000 0000 1000 */
void Cupd7907::ANA_V_A(upd7907_state *cpustate)
{
    V &= A;
    SET_Z(V);
}

/* 60 09: 0110 0000 0000 1001 */
void Cupd7907::ANA_A_A(upd7907_state *cpustate)
{
    A &= A;
    SET_Z(A);
}

/* 60 0a: 0110 0000 0000 1010 */
void Cupd7907::ANA_B_A(upd7907_state *cpustate)
{
    B &= A;
    SET_Z(B);
}

/* 60 0b: 0110 0000 0000 1011 */
void Cupd7907::ANA_C_A(upd7907_state *cpustate)
{
    C &= A;
    SET_Z(C);
}

/* 60 0c: 0110 0000 0000 1100 */
void Cupd7907::ANA_D_A(upd7907_state *cpustate)
{
    D &= A;
    SET_Z(D);
}

/* 60 0d: 0110 0000 0000 1101 */
void Cupd7907::ANA_E_A(upd7907_state *cpustate)
{
    E &= A;
    SET_Z(E);
}

/* 60 0e: 0110 0000 0000 1110 */
void Cupd7907::ANA_H_A(upd7907_state *cpustate)
{
    H &= A;
    SET_Z(H);
}

/* 60 0f: 0110 0000 0000 1111 */
void Cupd7907::ANA_L_A(upd7907_state *cpustate)
{
    L &= A;
    SET_Z(L);
}

/* 60 10: 0110 0000 0001 0000 */
void Cupd7907::XRA_V_A(upd7907_state *cpustate)
{
    V ^= A;
    SET_Z(V);
}

/* 60 11: 0110 0000 0001 0001 */
void Cupd7907::XRA_A_A(upd7907_state *cpustate)
{
    A ^= A;
    SET_Z(A);
}

/* 60 12: 0110 0000 0001 0010 */
void Cupd7907::XRA_B_A(upd7907_state *cpustate)
{
    B ^= A;
    SET_Z(B);
}

/* 60 13: 0110 0000 0001 0011 */
void Cupd7907::XRA_C_A(upd7907_state *cpustate)
{
    C ^= A;
    SET_Z(C);
}

/* 60 14: 0110 0000 0001 0100 */
void Cupd7907::XRA_D_A(upd7907_state *cpustate)
{
    D ^= A;
    SET_Z(D);
}

/* 60 15: 0110 0000 0001 0101 */
void Cupd7907::XRA_E_A(upd7907_state *cpustate)
{
    E ^= A;
    SET_Z(E);
}

/* 60 16: 0110 0000 0001 0110 */
void Cupd7907::XRA_H_A(upd7907_state *cpustate)
{
    H ^= A;
    SET_Z(H);
}

/* 60 17: 0110 0000 0001 0111 */
void Cupd7907::XRA_L_A(upd7907_state *cpustate)
{
    L ^= A;
    SET_Z(L);
}

/* 60 18: 0110 0000 0001 1000 */
void Cupd7907::ORA_V_A(upd7907_state *cpustate)
{
    V |= A;
    SET_Z(V);
}

/* 60 19: 0110 0000 0001 1001 */
void Cupd7907::ORA_A_A(upd7907_state *cpustate)
{
    A |= A;
    SET_Z(A);
}

/* 60 1a: 0110 0000 0001 1010 */
void Cupd7907::ORA_B_A(upd7907_state *cpustate)
{
    B |= A;
    SET_Z(B);
}

/* 60 1b: 0110 0000 0001 1011 */
void Cupd7907::ORA_C_A(upd7907_state *cpustate)
{
    C |= A;
    SET_Z(C);
}

/* 60 1c: 0110 0000 0001 1100 */
void Cupd7907::ORA_D_A(upd7907_state *cpustate)
{
    D |= A;
    SET_Z(D);
}

/* 60 1d: 0110 0000 0001 1101 */
void Cupd7907::ORA_E_A(upd7907_state *cpustate)
{
    E |= A;
    SET_Z(E);
}

/* 60 1e: 0110 0000 0001 1110 */
void Cupd7907::ORA_H_A(upd7907_state *cpustate)
{
    H |= A;
    SET_Z(H);
}

/* 60 1f: 0110 0000 0001 1111 */
void Cupd7907::ORA_L_A(upd7907_state *cpustate)
{
    L |= A;
    SET_Z(L);
}

/* 60 20: 0110 0000 0010 0000 */
void Cupd7907::ADDNC_V_A(upd7907_state *cpustate)
{
    UINT8 tmp = V + A;
    ZHC_ADD( tmp, V, 0 );
    V = tmp;
    SKIP_NC;
}

/* 60 21: 0110 0000 0010 0001 */
void Cupd7907::ADDNC_A_A(upd7907_state *cpustate)
{
    UINT8 tmp = A + A;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 22: 0110 0000 0010 0010 */
void Cupd7907::ADDNC_B_A(upd7907_state *cpustate)
{
    UINT8 tmp = B + A;
    ZHC_ADD( tmp, B, 0 );
    B = tmp;
    SKIP_NC;
}

/* 60 23: 0110 0000 0010 0011 */
void Cupd7907::ADDNC_C_A(upd7907_state *cpustate)
{
    UINT8 tmp = C + A;
    ZHC_ADD( tmp, C, 0 );
    C = tmp;
    SKIP_NC;
}

/* 60 24: 0110 0000 0010 0100 */
void Cupd7907::ADDNC_D_A(upd7907_state *cpustate)
{
    UINT8 tmp = D + A;
    ZHC_ADD( tmp, D, 0 );
    D = tmp;
    SKIP_NC;
}

/* 60 25: 0110 0000 0010 0101 */
void Cupd7907::ADDNC_E_A(upd7907_state *cpustate)
{
    UINT8 tmp = E + A;
    ZHC_ADD( tmp, E, 0 );
    E = tmp;
    SKIP_NC;
}

/* 60 26: 0110 0000 0010 0110 */
void Cupd7907::ADDNC_H_A(upd7907_state *cpustate)
{
    UINT8 tmp = H + A;
    ZHC_ADD( tmp, H, 0 );
    H = tmp;
    SKIP_NC;
}

/* 60 27: 0110 0000 0010 0111 */
void Cupd7907::ADDNC_L_A(upd7907_state *cpustate)
{
    UINT8 tmp = L + A;
    ZHC_ADD( tmp, L, 0 );
    L = tmp;
    SKIP_NC;
}

/* 60 28: 0110 0000 0010 1000 */
void Cupd7907::GTA_V_A(upd7907_state *cpustate)
{
    UINT16 tmp = V - A - 1;
    ZHC_SUB( tmp, V, 0 );
    SKIP_NC;
}

/* 60 29: 0110 0000 0010 1001 */
void Cupd7907::GTA_A_A(upd7907_state *cpustate)
{
    UINT16 tmp = A - A - 1;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 60 2a: 0110 0000 0010 1010 */
void Cupd7907::GTA_B_A(upd7907_state *cpustate)
{
    UINT16 tmp = B - A - 1;
    ZHC_SUB( tmp, B, 0 );
    SKIP_NC;
}

/* 60 2b: 0110 0000 0010 1011 */
void Cupd7907::GTA_C_A(upd7907_state *cpustate)
{
    UINT16 tmp = C - A - 1;
    ZHC_SUB( tmp, C, 0 );
    SKIP_NC;
}

/* 60 2c: 0110 0000 0010 1100 */
void Cupd7907::GTA_D_A(upd7907_state *cpustate)
{
    UINT16 tmp = D - A - 1;
    ZHC_SUB( tmp, D, 0 );
    SKIP_NC;
}

/* 60 2d: 0110 0000 0010 1101 */
void Cupd7907::GTA_E_A(upd7907_state *cpustate)
{
    UINT16 tmp = E - A - 1;
    ZHC_SUB( tmp, E, 0 );
    SKIP_NC;
}

/* 60 2e: 0110 0000 0010 1110 */
void Cupd7907::GTA_H_A(upd7907_state *cpustate)
{
    UINT16 tmp = H - A - 1;
    ZHC_SUB( tmp, H, 0 );
    SKIP_NC;
}

/* 60 2f: 0110 0000 0010 1111 */
void Cupd7907::GTA_L_A(upd7907_state *cpustate)
{
    UINT16 tmp = L - A - 1;
    ZHC_SUB( tmp, L, 0 );
    SKIP_NC;
}

/* 60 30: 0110 0000 0011 0000 */
void Cupd7907::SUBNB_V_A(upd7907_state *cpustate)
{
    UINT8 tmp = V - A;
    ZHC_SUB( tmp, V, 0 );
    V = tmp;
    SKIP_NC;
}

/* 60 31: 0110 0000 0011 0001 */
void Cupd7907::SUBNB_A_A(upd7907_state *cpustate)
{
    UINT8 tmp = A - A;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 32: 0110 0000 0011 0010 */
void Cupd7907::SUBNB_B_A(upd7907_state *cpustate)
{
    UINT8 tmp = B - A;
    ZHC_SUB( tmp, B, 0 );
    B = tmp;
    SKIP_NC;
}

/* 60 33: 0110 0000 0011 0011 */
void Cupd7907::SUBNB_C_A(upd7907_state *cpustate)
{
    UINT8 tmp = C - A;
    ZHC_SUB( tmp, C, 0 );
    C = tmp;
    SKIP_NC;
}

/* 60 34: 0110 0000 0011 0100 */
void Cupd7907::SUBNB_D_A(upd7907_state *cpustate)
{
    UINT8 tmp = D - A;
    ZHC_SUB( tmp, D, 0 );
    D = tmp;
    SKIP_NC;
}

/* 60 35: 0110 0000 0011 0101 */
void Cupd7907::SUBNB_E_A(upd7907_state *cpustate)
{
    UINT8 tmp = E - A;
    ZHC_SUB( tmp, E, 0 );
    E = tmp;
    SKIP_NC;
}

/* 60 36: 0110 0000 0011 0110 */
void Cupd7907::SUBNB_H_A(upd7907_state *cpustate)
{
    UINT8 tmp = H - A;
    ZHC_SUB( tmp, H, 0 );
    H = tmp;
    SKIP_NC;
}

/* 60 37: 0110 0000 0011 0111 */
void Cupd7907::SUBNB_L_A(upd7907_state *cpustate)
{
    UINT8 tmp = L - A;
    ZHC_SUB( tmp, L, 0 );
    L = tmp;
    SKIP_NC;
}

/* 60 38: 0110 0000 0011 1000 */
void Cupd7907::LTA_V_A(upd7907_state *cpustate)
{
    UINT8 tmp = V - A;
    ZHC_SUB( tmp, V, 0 );
    SKIP_CY;
}

/* 60 39: 0110 0000 0011 1001 */
void Cupd7907::LTA_A_A(upd7907_state *cpustate)
{
    UINT8 tmp = A - A;
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 60 3a: 0110 0000 0011 1010 */
void Cupd7907::LTA_B_A(upd7907_state *cpustate)
{
    UINT8 tmp = B - A;
    ZHC_SUB( tmp, B, 0 );
    SKIP_CY;
}

/* 60 3b: 0110 0000 0011 1011 */
void Cupd7907::LTA_C_A(upd7907_state *cpustate)
{
    UINT8 tmp = C - A;
    ZHC_SUB( tmp, C, 0 );
    SKIP_CY;
}

/* 60 3c: 0110 0000 0011 1100 */
void Cupd7907::LTA_D_A(upd7907_state *cpustate)
{
    UINT8 tmp = D - A;
    ZHC_SUB( tmp, D, 0 );
    SKIP_CY;
}

/* 60 3d: 0110 0000 0011 1101 */
void Cupd7907::LTA_E_A(upd7907_state *cpustate)
{
    UINT8 tmp = E - A;
    ZHC_SUB( tmp, E, 0 );
    SKIP_CY;
}

/* 60 3e: 0110 0000 0011 1110 */
void Cupd7907::LTA_H_A(upd7907_state *cpustate)
{
    UINT8 tmp = H - A;
    ZHC_SUB( tmp, H, 0 );
    SKIP_CY;
}

/* 60 3f: 0110 0000 0011 1111 */
void Cupd7907::LTA_L_A(upd7907_state *cpustate)
{
    UINT8 tmp = L - A;
    ZHC_SUB( tmp, L, 0 );
    SKIP_CY;
}

/* 60 40: 0110 0000 0100 0000 */
void Cupd7907::ADD_V_A(upd7907_state *cpustate)
{
    UINT8 tmp = V + A;
    ZHC_ADD( tmp, V, 0 );
    V = tmp;
}

/* 60 41: 0110 0000 0100 0001 */
void Cupd7907::ADD_A_A(upd7907_state *cpustate)
{
    UINT8 tmp = A + A;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 60 42: 0110 0000 0100 0010 */
void Cupd7907::ADD_B_A(upd7907_state *cpustate)
{
    UINT8 tmp = B + A;
    ZHC_ADD( tmp, B, 0 );
    B = tmp;
}

/* 60 43: 0110 0000 0100 0011 */
void Cupd7907::ADD_C_A(upd7907_state *cpustate)
{
    UINT8 tmp = C + A;
    ZHC_ADD( tmp, C, 0 );
    C = tmp;
}

/* 60 44: 0110 0000 0100 0100 */
void Cupd7907::ADD_D_A(upd7907_state *cpustate)
{
    UINT8 tmp = D + A;
    ZHC_ADD( tmp, D, 0 );
    D = tmp;
}

/* 60 45: 0110 0000 0100 0101 */
void Cupd7907::ADD_E_A(upd7907_state *cpustate)
{
    UINT8 tmp = E + A;
    ZHC_ADD( tmp, E, 0 );
    E = tmp;
}

/* 60 46: 0110 0000 0100 0110 */
void Cupd7907::ADD_H_A(upd7907_state *cpustate)
{
    UINT8 tmp = H + A;
    ZHC_ADD( tmp, H, 0 );
    H = tmp;
}

/* 60 47: 0110 0000 0100 0111 */
void Cupd7907::ADD_L_A(upd7907_state *cpustate)
{
    UINT8 tmp = L + A;
    ZHC_ADD( tmp, L, 0 );
    L = tmp;
}

/* 60 50: 0110 0000 0101 0000 */
void Cupd7907::ADC_V_A(upd7907_state *cpustate)
{
    UINT8 tmp = V + A + (PSW & CY);
    ZHC_ADD( tmp, V, (PSW & CY) );
    V = tmp;
}

/* 60 51: 0110 0000 0101 0001 */
void Cupd7907::ADC_A_A(upd7907_state *cpustate)
{
    UINT8 tmp = A + A + (PSW & CY);
    ZHC_ADD( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 52: 0110 0000 0101 0010 */
void Cupd7907::ADC_B_A(upd7907_state *cpustate)
{
    UINT8 tmp = B + A + (PSW & CY);
    ZHC_ADD( tmp, B, (PSW & CY) );
    B = tmp;
}

/* 60 53: 0110 0000 0101 0011 */
void Cupd7907::ADC_C_A(upd7907_state *cpustate)
{
    UINT8 tmp = C + A + (PSW & CY);
    ZHC_ADD( tmp, C, (PSW & CY) );
    C = tmp;
}

/* 60 54: 0110 0000 0101 0100 */
void Cupd7907::ADC_D_A(upd7907_state *cpustate)
{
    UINT8 tmp = D + A + (PSW & CY);
    ZHC_ADD( tmp, D, (PSW & CY) );
    D = tmp;
}

/* 60 55: 0110 0000 0101 0101 */
void Cupd7907::ADC_E_A(upd7907_state *cpustate)
{
    UINT8 tmp = E + A + (PSW & CY);
    ZHC_ADD( tmp, E, (PSW & CY) );
    E = tmp;
}

/* 60 56: 0110 0000 0101 0110 */
void Cupd7907::ADC_H_A(upd7907_state *cpustate)
{
    UINT8 tmp = H + A + (PSW & CY);
    ZHC_ADD( tmp, H, (PSW & CY) );
    H = tmp;
}

/* 60 57: 0110 0000 0101 0111 */
void Cupd7907::ADC_L_A(upd7907_state *cpustate)
{
    UINT8 tmp = L + A + (PSW & CY);
    ZHC_ADD( tmp, L, (PSW & CY) );
    L = tmp;
}

/* 60 60: 0110 0000 0110 0000 */
void Cupd7907::SUB_V_A(upd7907_state *cpustate)
{
    UINT8 tmp = V - A;
    ZHC_SUB( tmp, V, 0 );
    V = tmp;
}

/* 60 61: 0110 0000 0110 0001 */
void Cupd7907::SUB_A_A(upd7907_state *cpustate)
{
    UINT8 tmp = A - A;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
}

/* 60 62: 0110 0000 0110 0010 */
void Cupd7907::SUB_B_A(upd7907_state *cpustate)
{
    UINT8 tmp = B - A;
    ZHC_SUB( tmp, B, 0 );
    B = tmp;
}

/* 60 63: 0110 0000 0110 0011 */
void Cupd7907::SUB_C_A(upd7907_state *cpustate)
{
    UINT8 tmp = C - A;
    ZHC_SUB( tmp, C, 0 );
    C = tmp;
}

/* 60 64: 0110 0000 0110 0100 */
void Cupd7907::SUB_D_A(upd7907_state *cpustate)
{
    UINT8 tmp = D - A;
    ZHC_SUB( tmp, D, 0 );
    D = tmp;
}

/* 60 65: 0110 0000 0110 0101 */
void Cupd7907::SUB_E_A(upd7907_state *cpustate)
{
    UINT8 tmp = E - A;
    ZHC_SUB( tmp, E, 0 );
    E = tmp;
}

/* 60 66: 0110 0000 0110 0110 */
void Cupd7907::SUB_H_A(upd7907_state *cpustate)
{
    UINT8 tmp = H - A;
    ZHC_SUB( tmp, H, 0 );
    H = tmp;
}

/* 60 67: 0110 0000 0110 0111 */
void Cupd7907::SUB_L_A(upd7907_state *cpustate)
{
    UINT8 tmp = L - A;
    ZHC_SUB( tmp, L, 0 );
    L = tmp;
}

/* 60 68: 0110 0000 0110 1000 */
void Cupd7907::NEA_V_A(upd7907_state *cpustate)
{
    UINT8 tmp = V - A;
    ZHC_SUB( tmp, V, 0 );
    SKIP_NZ;
}

/* 60 69: 0110 0000 0110 1001 */
void Cupd7907::NEA_A_A(upd7907_state *cpustate)
{
    UINT8 tmp = A - A;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 60 6a: 0110 0000 0110 1010 */
void Cupd7907::NEA_B_A(upd7907_state *cpustate)
{
    UINT8 tmp = B - A;
    ZHC_SUB( tmp, B, 0 );
    SKIP_NZ;
}

/* 60 6b: 0110 0000 0110 1011 */
void Cupd7907::NEA_C_A(upd7907_state *cpustate)
{
    UINT8 tmp = C - A;
    ZHC_SUB( tmp, C, 0 );
    SKIP_NZ;
}

/* 60 6c: 0110 0000 0110 1100 */
void Cupd7907::NEA_D_A(upd7907_state *cpustate)
{
    UINT8 tmp = D - A;
    ZHC_SUB( tmp, D, 0 );
    SKIP_NZ;
}

/* 60 6d: 0110 0000 0110 1101 */
void Cupd7907::NEA_E_A(upd7907_state *cpustate)
{
    UINT8 tmp = E - A;
    ZHC_SUB( tmp, E, 0 );
    SKIP_NZ;
}

/* 60 6e: 0110 0000 0110 1110 */
void Cupd7907::NEA_H_A(upd7907_state *cpustate)
{
    UINT8 tmp = H - A;
    ZHC_SUB( tmp, H, 0 );
    SKIP_NZ;
}

/* 60 6f: 0110 0000 0110 1111 */
void Cupd7907::NEA_L_A(upd7907_state *cpustate)
{
    UINT8 tmp = L - A;
    ZHC_SUB( tmp, L, 0 );
    SKIP_NZ;
}

/* 60 70: 0110 0000 0111 0000 */
void Cupd7907::SBB_V_A(upd7907_state *cpustate)
{
    UINT8 tmp = V - A - (PSW & CY);
    ZHC_SUB( tmp, V, (PSW & CY) );
    V = tmp;
}

/* 60 71: 0110 0000 0111 0001 */
void Cupd7907::SBB_A_A(upd7907_state *cpustate)
{
    UINT8 tmp = A - A - (PSW & CY);
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 72: 0110 0000 0111 0010 */
void Cupd7907::SBB_B_A(upd7907_state *cpustate)
{
    UINT8 tmp = B - A - (PSW & CY);
    ZHC_SUB( tmp, B, (PSW & CY) );
    B = tmp;
}

/* 60 73: 0110 0000 0111 0011 */
void Cupd7907::SBB_C_A(upd7907_state *cpustate)
{
    UINT8 tmp = C - A - (PSW & CY);
    ZHC_SUB( tmp, C, (PSW & CY) );
    C = tmp;
}

/* 60 74: 0110 0000 0111 0100 */
void Cupd7907::SBB_D_A(upd7907_state *cpustate)
{
    UINT8 tmp = D - A - (PSW & CY);
    ZHC_SUB( tmp, D, (PSW & CY) );
    D = tmp;
}

/* 60 75: 0110 0000 0111 0101 */
void Cupd7907::SBB_E_A(upd7907_state *cpustate)
{
    UINT8 tmp = E - A - (PSW & CY);
    ZHC_SUB( tmp, E, (PSW & CY) );
    E = tmp;
}

/* 60 76: 0110 0000 0111 0110 */
void Cupd7907::SBB_H_A(upd7907_state *cpustate)
{
    UINT8 tmp = H - A - (PSW & CY);
    ZHC_SUB( tmp, H, (PSW & CY) );
    H = tmp;
}

/* 60 77: 0110 0000 0111 0111 */
void Cupd7907::SBB_L_A(upd7907_state *cpustate)
{
    UINT8 tmp = L - A - (PSW & CY);
    ZHC_SUB( tmp, L, (PSW & CY) );
    L = tmp;
}

/* 60 78: 0110 0000 0111 1000 */
void Cupd7907::EQA_V_A(upd7907_state *cpustate)
{
    UINT8 tmp = V - A;
    ZHC_SUB( tmp, V, 0 );
    SKIP_Z;
}

/* 60 79: 0110 0000 0111 1001 */
void Cupd7907::EQA_A_A(upd7907_state *cpustate)
{
    UINT8 tmp = A - A;
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 60 7a: 0110 0000 0111 1010 */
void Cupd7907::EQA_B_A(upd7907_state *cpustate)
{
    UINT8 tmp = B - A;
    ZHC_SUB( tmp, B, 0 );
    SKIP_Z;
}

/* 60 7b: 0110 0000 0111 1011 */
void Cupd7907::EQA_C_A(upd7907_state *cpustate)
{
    UINT8 tmp = C - A;
    ZHC_SUB( tmp, C, 0 );
    SKIP_Z;
}

/* 60 7c: 0110 0000 0111 1100 */
void Cupd7907::EQA_D_A(upd7907_state *cpustate)
{
    UINT8 tmp = D - A;
    ZHC_SUB( tmp, D, 0 );
    SKIP_Z;
}

/* 60 7d: 0110 0000 0111 1101 */
void Cupd7907::EQA_E_A(upd7907_state *cpustate)
{
    UINT8 tmp = E - A;
    ZHC_SUB( tmp, E, 0 );
    SKIP_Z;
}

/* 60 7e: 0110 0000 0111 1110 */
void Cupd7907::EQA_H_A(upd7907_state *cpustate)
{
    UINT8 tmp = H - A;
    ZHC_SUB( tmp, H, 0 );
    SKIP_Z;
}

/* 60 7f: 0110 0000 0111 1111 */
void Cupd7907::EQA_L_A(upd7907_state *cpustate)
{
    UINT8 tmp = L - A;
    ZHC_SUB( tmp, L, 0 );
    SKIP_Z;
}

/* 60 88: 0110 0000 1000 1000 */
void Cupd7907::ANA_A_V(upd7907_state *cpustate)
{
    A &= V;
    SET_Z(A);
}

/* 60 89: 0110 0000 1000 1001 */
/* ANA_A_A already defined */

/* 60 8a: 0110 0000 1000 1010 */
void Cupd7907::ANA_A_B(upd7907_state *cpustate)
{
    A &= B;
    SET_Z(A);
}

/* 60 8b: 0110 0000 1000 1011 */
void Cupd7907::ANA_A_C(upd7907_state *cpustate)
{
    A &= C;
    SET_Z(A);
}

/* 60 8c: 0110 0000 1000 1100 */
void Cupd7907::ANA_A_D(upd7907_state *cpustate)
{
    A &= D;
    SET_Z(A);
}

/* 60 8d: 0110 0000 1000 1101 */
void Cupd7907::ANA_A_E(upd7907_state *cpustate)
{
    A &= E;
    SET_Z(A);
}

/* 60 8e: 0110 0000 1000 1110 */
void Cupd7907::ANA_A_H(upd7907_state *cpustate)
{
    A &= H;
    SET_Z(A);
}

/* 60 8f: 0110 0000 1000 1111 */
void Cupd7907::ANA_A_L(upd7907_state *cpustate)
{
    A &= L;
    SET_Z(A);
}

/* 60 90: 0110 0000 1001 0000 */
void Cupd7907::XRA_A_V(upd7907_state *cpustate)
{
    A ^= V;
    SET_Z(A);
}

/* 60 91: 0110 0000 1001 0001 */
/* XRA_A_A already defined */

/* 60 92: 0110 0000 1001 0010 */
void Cupd7907::XRA_A_B(upd7907_state *cpustate)
{
    A ^= B;
    SET_Z(A);
}

/* 60 93: 0110 0000 1001 0011 */
void Cupd7907::XRA_A_C(upd7907_state *cpustate)
{
    A ^= C;
    SET_Z(A);
}

/* 60 94: 0110 0000 1001 0100 */
void Cupd7907::XRA_A_D(upd7907_state *cpustate)
{
    A ^= D;
    SET_Z(A);
}

/* 60 95: 0110 0000 1001 0101 */
void Cupd7907::XRA_A_E(upd7907_state *cpustate)
{
    A ^= E;
    SET_Z(A);
}

/* 60 96: 0110 0000 1001 0110 */
void Cupd7907::XRA_A_H(upd7907_state *cpustate)
{
    A ^= H;
    SET_Z(A);
}

/* 60 97: 0110 0000 1001 0111 */
void Cupd7907::XRA_A_L(upd7907_state *cpustate)
{
    A ^= L;
    SET_Z(A);
}

/* 60 98: 0110 0000 1001 1000 */
void Cupd7907::ORA_A_V(upd7907_state *cpustate)
{
    A |= V;
    SET_Z(A);
}

/* 60 99: 0110 0000 1001 1001 */
/* ORA_A_A already defined */

/* 60 9a: 0110 0000 1001 1010 */
void Cupd7907::ORA_A_B(upd7907_state *cpustate)
{
    A |= B;
    SET_Z(A);
}

/* 60 9b: 0110 0000 1001 1011 */
void Cupd7907::ORA_A_C(upd7907_state *cpustate)
{
    A |= C;
    SET_Z(A);
}

/* 60 9c: 0110 0000 1001 1100 */
void Cupd7907::ORA_A_D(upd7907_state *cpustate)
{
    A |= D;
    SET_Z(A);
}

/* 60 9d: 0110 0000 1001 1101 */
void Cupd7907::ORA_A_E(upd7907_state *cpustate)
{
    A |= E;
    SET_Z(A);
}

/* 60 9e: 0110 0000 1001 1110 */
void Cupd7907::ORA_A_H(upd7907_state *cpustate)
{
    A |= H;
    SET_Z(A);
}

/* 60 9f: 0110 0000 1001 1111 */
void Cupd7907::ORA_A_L(upd7907_state *cpustate)
{
    A |= L;
    SET_Z(A);
}

/* 60 a0: 0110 0000 1010 0000 */
void Cupd7907::ADDNC_A_V(upd7907_state *cpustate)
{
    UINT8 tmp = A + V;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 a1: 0110 0000 1010 0001 */
/* ADDNC_A_A already defined */

/* 60 a2: 0110 0000 1010 0010 */
void Cupd7907::ADDNC_A_B(upd7907_state *cpustate)
{
    UINT8 tmp = A + B;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 a3: 0110 0000 1010 0011 */
void Cupd7907::ADDNC_A_C(upd7907_state *cpustate)
{
    UINT8 tmp = A + C;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 a4: 0110 0000 1010 0100 */
void Cupd7907::ADDNC_A_D(upd7907_state *cpustate)
{
    UINT8 tmp = A + D;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 a5: 0110 0000 1010 0101 */
void Cupd7907::ADDNC_A_E(upd7907_state *cpustate)
{
    UINT8 tmp = A + E;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 a6: 0110 0000 1010 0110 */
void Cupd7907::ADDNC_A_H(upd7907_state *cpustate)
{
    UINT8 tmp = A + H;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 a7: 0110 0000 1010 0111 */
void Cupd7907::ADDNC_A_L(upd7907_state *cpustate)
{
    UINT8 tmp = A + L;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 a8: 0110 0000 1010 1000 */
void Cupd7907::GTA_A_V(upd7907_state *cpustate)
{
    UINT16 tmp = A - V - 1;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 60 a9: 0110 0000 1010 1001 */
/* GTA_A_A already defined */

/* 60 aa: 0110 0000 1010 1010 */
void Cupd7907::GTA_A_B(upd7907_state *cpustate)
{
    UINT16 tmp = A - B - 1;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 60 ab: 0110 0000 1010 1011 */
void Cupd7907::GTA_A_C(upd7907_state *cpustate)
{
    UINT16 tmp = A - C - 1;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 60 ac: 0110 0000 1010 1100 */
void Cupd7907::GTA_A_D(upd7907_state *cpustate)
{
    UINT16 tmp = A - D - 1;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 60 ad: 0110 0000 1010 1101 */
void Cupd7907::GTA_A_E(upd7907_state *cpustate)
{
    UINT16 tmp = A - E - 1;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 60 ae: 0110 0000 1010 1110 */
void Cupd7907::GTA_A_H(upd7907_state *cpustate)
{
    UINT16 tmp = A - H - 1;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 60 af: 0110 0000 1010 1111 */
void Cupd7907::GTA_A_L(upd7907_state *cpustate)
{
    UINT16 tmp = A - L - 1;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 60 b0: 0110 0000 1011 0000 */
void Cupd7907::SUBNB_A_V(upd7907_state *cpustate)
{
    UINT8 tmp = A - V;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 b1: 0110 0000 1011 0001 */
/* SUBNB_A_A already defined */

/* 60 b2: 0110 0000 1011 0010 */
void Cupd7907::SUBNB_A_B(upd7907_state *cpustate)
{
    UINT8 tmp = A - B;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 b3: 0110 0000 1011 0011 */
void Cupd7907::SUBNB_A_C(upd7907_state *cpustate)
{
    UINT8 tmp = A - C;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 b4: 0110 0000 1011 0100 */
void Cupd7907::SUBNB_A_D(upd7907_state *cpustate)
{
    UINT8 tmp = A - D;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 b5: 0110 0000 1011 0101 */
void Cupd7907::SUBNB_A_E(upd7907_state *cpustate)
{
    UINT8 tmp = A - E;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 b6: 0110 0000 1011 0110 */
void Cupd7907::SUBNB_A_H(upd7907_state *cpustate)
{
    UINT8 tmp = A - H;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 b7: 0110 0000 1011 0111 */
void Cupd7907::SUBNB_A_L(upd7907_state *cpustate)
{
    UINT8 tmp = A - L;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 60 b8: 0110 0000 1011 1000 */
void Cupd7907::LTA_A_V(upd7907_state *cpustate)
{
    UINT8 tmp = A - V;
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 60 b9: 0110 0000 1011 1001 */
/* LTA_A_A already defined */

/* 60 ba: 0110 0000 1011 1010 */
void Cupd7907::LTA_A_B(upd7907_state *cpustate)
{
    UINT8 tmp = A - B;
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 60 bb: 0110 0000 1011 1011 */
void Cupd7907::LTA_A_C(upd7907_state *cpustate)
{
    UINT8 tmp = A - C;
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 60 bc: 0110 0000 1011 1100 */
void Cupd7907::LTA_A_D(upd7907_state *cpustate)
{
    UINT8 tmp = A - D;
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 60 bd: 0110 0000 1011 1101 */
void Cupd7907::LTA_A_E(upd7907_state *cpustate)
{
    UINT8 tmp = A - E;
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 60 be: 0110 0000 1011 1110 */
void Cupd7907::LTA_A_H(upd7907_state *cpustate)
{
    UINT8 tmp = A - H;
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 60 bf: 0110 0000 1011 1111 */
void Cupd7907::LTA_A_L(upd7907_state *cpustate)
{
    UINT8 tmp = A - L;
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 60 c0: 0110 0000 1100 0000 */
void Cupd7907::ADD_A_V(upd7907_state *cpustate)
{
    UINT8 tmp = A + V;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 60 c1: 0110 0000 1100 0001 */
/* ADD_A_A already defined */

/* 60 c2: 0110 0000 1100 0010 */
void Cupd7907::ADD_A_B(upd7907_state *cpustate)
{
    UINT8 tmp = A + B;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 60 c3: 0110 0000 1100 0011 */
void Cupd7907::ADD_A_C(upd7907_state *cpustate)
{
    UINT8 tmp = A + C;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 60 c4: 0110 0000 1100 0100 */
void Cupd7907::ADD_A_D(upd7907_state *cpustate)
{
    UINT8 tmp = A + D;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 60 c5: 0110 0000 1100 0101 */
void Cupd7907::ADD_A_E(upd7907_state *cpustate)
{
    UINT8 tmp = A + E;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 60 c6: 0110 0000 1100 0110 */
void Cupd7907::ADD_A_H(upd7907_state *cpustate)
{
    UINT8 tmp = A + H;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 60 c7: 0110 0000 1100 0111 */
void Cupd7907::ADD_A_L(upd7907_state *cpustate)
{
    UINT8 tmp = A + L;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 60 c8: 0110 0000 1100 1000 */
void Cupd7907::ONA_A_V(upd7907_state *cpustate)
{
    if (A & V)
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 60 c9: 0110 0000 1100 1001 */
void Cupd7907::ONA_A_A(upd7907_state *cpustate)
{
    if (A & A)
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 60 ca: 0110 0000 1100 1010 */
void Cupd7907::ONA_A_B(upd7907_state *cpustate)
{
    if (A & B)
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 60 cb: 0110 0000 1100 1011 */
void Cupd7907::ONA_A_C(upd7907_state *cpustate)
{
    if (A & C)
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 60 cc: 0110 0000 1100 1100 */
void Cupd7907::ONA_A_D(upd7907_state *cpustate)
{
    if (A & D)
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 60 cd: 0110 0000 1100 1101 */
void Cupd7907::ONA_A_E(upd7907_state *cpustate)
{
    if (A & E)
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 60 ce: 0110 0000 1100 1110 */
void Cupd7907::ONA_A_H(upd7907_state *cpustate)
{
    if (A & H)
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 60 cf: 0110 0000 1100 1111 */
void Cupd7907::ONA_A_L(upd7907_state *cpustate)
{
    if (A & L)
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 60 d0: 0110 0000 1101 0000 */
void Cupd7907::ADC_A_V(upd7907_state *cpustate)
{
    UINT8 tmp = A + V + (PSW & CY);
    ZHC_ADD( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 d1: 0110 0000 1101 0001 */
/* ADC_A_A already defined */

/* 60 d2: 0110 0000 1101 0010 */
void Cupd7907::ADC_A_B(upd7907_state *cpustate)
{
    UINT8 tmp = A + B + (PSW & CY);
    ZHC_ADD( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 d3: 0110 0000 1101 0011 */
void Cupd7907::ADC_A_C(upd7907_state *cpustate)
{
    UINT8 tmp = A + C + (PSW & CY);
    ZHC_ADD( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 d4: 0110 0000 1101 0100 */
void Cupd7907::ADC_A_D(upd7907_state *cpustate)
{
    UINT8 tmp = A + D + (PSW & CY);
    ZHC_ADD( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 d5: 0110 0000 1101 0101 */
void Cupd7907::ADC_A_E(upd7907_state *cpustate)
{
    UINT8 tmp = A + E + (PSW & CY);
    ZHC_ADD( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 d6: 0110 0000 1101 0110 */
void Cupd7907::ADC_A_H(upd7907_state *cpustate)
{
    UINT8 tmp = A + H + (PSW & CY);
    ZHC_ADD( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 d7: 0110 0000 1101 0111 */
void Cupd7907::ADC_A_L(upd7907_state *cpustate)
{
    UINT8 tmp = A + L + (PSW & CY);
    ZHC_ADD( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 d8: 0110 0000 1101 1000 */
void Cupd7907::OFFA_A_V(upd7907_state *cpustate)
{
    if ( A & V )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 60 d9: 0110 0000 1101 1001 */
void Cupd7907::OFFA_A_A(upd7907_state *cpustate)
{
    if ( A & A )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 60 da: 0110 0000 1101 1010 */
void Cupd7907::OFFA_A_B(upd7907_state *cpustate)
{
    if ( A & B )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 60 db: 0110 0000 1101 1011 */
void Cupd7907::OFFA_A_C(upd7907_state *cpustate)
{
    if ( A & C )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 60 dc: 0110 0000 1101 1100 */
void Cupd7907::OFFA_A_D(upd7907_state *cpustate)
{
    if ( A & D )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 60 dd: 0110 0000 1101 1101 */
void Cupd7907::OFFA_A_E(upd7907_state *cpustate)
{
    if ( A & E )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 60 de: 0110 0000 1101 1110 */
void Cupd7907::OFFA_A_H(upd7907_state *cpustate)
{
    if ( A & H )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 60 df: 0110 0000 1101 1111 */
void Cupd7907::OFFA_A_L(upd7907_state *cpustate)
{
    if ( A & L )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 60 e0: 0110 0000 1110 0000 */
void Cupd7907::SUB_A_V(upd7907_state *cpustate)
{
    UINT8 tmp = A - V;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
}

/* 60 e1: 0110 0000 1110 0001 */
/* SUB_A_A already defined */

/* 60 e2: 0110 0000 1110 0010 */
void Cupd7907::SUB_A_B(upd7907_state *cpustate)
{
    UINT8 tmp = A - B;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
}

/* 60 e3: 0110 0000 1110 0011 */
void Cupd7907::SUB_A_C(upd7907_state *cpustate)
{
    UINT8 tmp = A - C;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
}

/* 60 e4: 0110 0000 1110 0100 */
void Cupd7907::SUB_A_D(upd7907_state *cpustate)
{
    UINT8 tmp = A - D;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
}

/* 60 e5: 0110 0000 1110 0101 */
void Cupd7907::SUB_A_E(upd7907_state *cpustate)
{
    UINT8 tmp = A - E;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
}

/* 60 e6: 0110 0000 1110 0110 */
void Cupd7907::SUB_A_H(upd7907_state *cpustate)
{
    UINT8 tmp = A - H;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
}

/* 60 e7: 0110 0000 1110 0111 */
void Cupd7907::SUB_A_L(upd7907_state *cpustate)
{
    UINT8 tmp = A - L;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
}

/* 60 e8: 0110 0000 1110 1000 */
void Cupd7907::NEA_A_V(upd7907_state *cpustate)
{
    UINT8 tmp = A - V;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 60 e9: 0110 0000 1110 1001 */
/* NEA_A_A already defined */

/* 60 ea: 0110 0000 1110 1010 */
void Cupd7907::NEA_A_B(upd7907_state *cpustate)
{
    UINT8 tmp = A - B;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 60 eb: 0110 0000 1110 1011 */
void Cupd7907::NEA_A_C(upd7907_state *cpustate)
{
    UINT8 tmp = A - C;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 60 ec: 0110 0000 1110 1100 */
void Cupd7907::NEA_A_D(upd7907_state *cpustate)
{
    UINT8 tmp = A - D;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 60 ed: 0110 0000 1110 1101 */
void Cupd7907::NEA_A_E(upd7907_state *cpustate)
{
    UINT8 tmp = A - E;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 60 ee: 0110 0000 1110 1110 */
void Cupd7907::NEA_A_H(upd7907_state *cpustate)
{
    UINT8 tmp = A - H;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 60 ef: 0110 0000 1110 1111 */
void Cupd7907::NEA_A_L(upd7907_state *cpustate)
{
    UINT8 tmp = A - L;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 60 f0: 0110 0000 1111 0000 */
void Cupd7907::SBB_A_V(upd7907_state *cpustate)
{
    UINT8 tmp = A - V - (PSW & CY);
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 f1: 0110 0000 1111 0001 */
/* SBB_A_A already defined */

/* 60 f2: 0110 0000 1111 0010 */
void Cupd7907::SBB_A_B(upd7907_state *cpustate)
{
    UINT8 tmp = A - B - (PSW & CY);
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 f3: 0110 0000 1111 0011 */
void Cupd7907::SBB_A_C(upd7907_state *cpustate)
{
    UINT8 tmp = A - C - (PSW & CY);
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 f4: 0110 0000 1111 0100 */
void Cupd7907::SBB_A_D(upd7907_state *cpustate)
{
    UINT8 tmp = A - D - (PSW & CY);
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 f5: 0110 0000 1111 0101 */
void Cupd7907::SBB_A_E(upd7907_state *cpustate)
{
    UINT8 tmp = A - E - (PSW & CY);
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 f6: 0110 0000 1111 0110 */
void Cupd7907::SBB_A_H(upd7907_state *cpustate)
{
    UINT8 tmp = A - H - (PSW & CY);
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 f7: 0110 0000 1111 0111 */
void Cupd7907::SBB_A_L(upd7907_state *cpustate)
{
    UINT8 tmp = A - L - (PSW & CY);
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 60 f8: 0110 0000 1111 1000 */
void Cupd7907::EQA_A_V(upd7907_state *cpustate)
{
    UINT8 tmp = A - V;
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 60 f9: 0110 0000 1111 1001 */
/* EQA_A_A already defined */

/* 60 fa: 0110 0000 1111 1010 */
void Cupd7907::EQA_A_B(upd7907_state *cpustate)
{
    UINT8 tmp = A - B;
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 60 fb: 0110 0000 1111 1011 */
void Cupd7907::EQA_A_C(upd7907_state *cpustate)
{
    UINT8 tmp = A - C;
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 60 fc: 0110 0000 1111 1100 */
void Cupd7907::EQA_A_D(upd7907_state *cpustate)
{
    UINT8 tmp = A - D;
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 60 fd: 0110 0000 1111 1101 */
void Cupd7907::EQA_A_E(upd7907_state *cpustate)
{
    UINT8 tmp = A - E;
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 60 fe: 0110 0000 1111 1110 */
void Cupd7907::EQA_A_H(upd7907_state *cpustate)
{
    UINT8 tmp = A - H;
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 60 ff: 0110 0000 1111 1111 */
void Cupd7907::EQA_A_L(upd7907_state *cpustate)
{
    UINT8 tmp = A - L;
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* prefix 64 */
/* 64 00: 0110 0100 0000 0000 xxxx xxxx */
void Cupd7907::MVI_PA_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    WP( cpustate, UPD7907_PORTA, imm );
}

/* 64 01: 0110 0100 0000 0001 xxxx xxxx */
void Cupd7907::MVI_PB_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    WP( cpustate, UPD7907_PORTB, imm );
}

/* 64 02: 0110 0100 0000 0010 xxxx xxxx */
void Cupd7907::MVI_PC_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    WP( cpustate, UPD7907_PORTC, imm );
}

/* 64 03: 0110 0100 0000 0011 xxxx xxxx */
void Cupd7907::MVI_PD_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    WP( cpustate, UPD7907_PORTD, imm );
}

/* 64 05: 0110 0100 0000 0101 xxxx xxxx */
void Cupd7907::MVI_PF_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    WP( cpustate, UPD7907_PORTF, imm );
}

/* 64 06: 0110 0100 0000 0110 xxxx xxxx */
void Cupd7907::MVI_MKH_xx(upd7907_state *cpustate)
{
    RDOPARG( MKH );
}

/* 64 07: 0110 0100 0000 0111 xxxx xxxx */
void Cupd7907::MVI_MKL_xx(upd7907_state *cpustate)
{
    RDOPARG( MKL );
}

/* 64 08: 0110 0100 0000 1000 xxxx xxxx */
void Cupd7907::ANI_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA), imm;
    RDOPARG( imm );
    pa &= imm;
    WP( cpustate, UPD7907_PORTA, pa );
    SET_Z(pa);
}

/* 64 09: 0110 0100 0000 1001 xxxx xxxx */
void Cupd7907::ANI_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB), imm;
    RDOPARG( imm );
    pb &= imm;
    WP( cpustate, UPD7907_PORTB, pb );
    SET_Z(pb);
}

/* 64 0a: 0110 0100 0000 1010 xxxx xxxx */
void Cupd7907::ANI_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC), imm;
    RDOPARG( imm );
    pc &= imm;
    WP( cpustate, UPD7907_PORTC, pc );
    SET_Z(pc);
}

/* 64 0b: 0110 0100 0000 1011 xxxx xxxx */
void Cupd7907::ANI_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD ), imm;
    RDOPARG( imm );
    pd &= imm;
    WP( cpustate, UPD7907_PORTD, pd );
    SET_Z(pd);
}

/* 64 0d: 0110 0100 0000 1101 xxxx xxxx */
void Cupd7907::ANI_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF ), imm;
    RDOPARG( imm );
    pf &= imm;
    WP( cpustate, UPD7907_PORTF, pf );
    SET_Z(pf);
}

/* 64 0e: 0110 0100 0000 1110 xxxx xxxx */
void Cupd7907::ANI_MKH_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    MKH &= imm;
    SET_Z(MKH);
}

/* 64 0f: 0110 0100 0000 1111 xxxx xxxx */
void Cupd7907::ANI_MKL_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    MKL &= imm;
    SET_Z(MKL);
}

/* 64 10: 0110 0100 0001 0000 xxxx xxxx */
void Cupd7907::XRI_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA ), imm;
    RDOPARG( imm );
    pa ^= imm;
    WP( cpustate, UPD7907_PORTA, pa );
    SET_Z(pa);
}

/* 64 11: 0110 0100 0001 0001 xxxx xxxx */
void Cupd7907::XRI_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB ), imm;
    RDOPARG( imm );
    pb ^= imm;
    WP( cpustate, UPD7907_PORTB, pb );
    SET_Z(pb);
}

/* 64 12: 0110 0100 0001 0010 xxxx xxxx */
void Cupd7907::XRI_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC ), imm;
    RDOPARG( imm );
    pc ^= imm;
    WP( cpustate, UPD7907_PORTC, pc );
    SET_Z(pc);
}

/* 64 13: 0110 0100 0001 0011 xxxx xxxx */
void Cupd7907::XRI_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD ), imm;
    RDOPARG( imm );
    pd ^= imm;
    WP( cpustate, UPD7907_PORTD, pd );
    SET_Z(pd);
}

/* 64 15: 0110 0100 0001 0101 xxxx xxxx */
void Cupd7907::XRI_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF ), imm;
    RDOPARG( imm );
    pf ^= imm;
    WP( cpustate, UPD7907_PORTF, pf );
    SET_Z(pf);
}

/* 64 16: 0110 0100 0001 0110 xxxx xxxx */
void Cupd7907::XRI_MKH_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    MKH ^= imm;
    SET_Z(MKH);
}

/* 64 17: 0110 0100 0001 0111 xxxx xxxx */
void Cupd7907::XRI_MKL_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    MKL ^= imm;
    SET_Z(MKL);
}

/* 64 18: 0110 0100 0001 1000 xxxx xxxx */
void Cupd7907::ORI_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA ), imm;
    RDOPARG( imm );
    pa |= imm;
    WP( cpustate, UPD7907_PORTA, pa );
    SET_Z(pa);
}

/* 64 19: 0110 0100 0001 1001 xxxx xxxx */
void Cupd7907::ORI_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB ), imm;
    RDOPARG( imm );
    pb |= imm;
    WP( cpustate, UPD7907_PORTB, pb );
    SET_Z(pb);
}

/* 64 1a: 0110 0100 0001 1010 xxxx xxxx */
void Cupd7907::ORI_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC ), imm;
    RDOPARG( imm );
    pc |= imm;
    WP( cpustate, UPD7907_PORTC, pc );
    SET_Z(pc);
}

/* 64 1b: 0110 0100 0001 1011 xxxx xxxx */
void Cupd7907::ORI_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD ), imm;
    RDOPARG( imm );
    pd |= imm;
    WP( cpustate, UPD7907_PORTD, pd );
    SET_Z(pd);
}

/* 64 1d: 0110 0100 0001 1101 xxxx xxxx */
void Cupd7907::ORI_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF ), imm;
    RDOPARG( imm );
    pf |= imm;
    WP( cpustate, UPD7907_PORTF, pf );
    SET_Z(pf);
}

/* 64 1e: 0110 0100 0001 1110 xxxx xxxx */
void Cupd7907::ORI_MKH_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    MKH |= imm;
    SET_Z(MKH);
}

/* 64 1f: 0110 0100 0001 1111 xxxx xxxx */
void Cupd7907::ORI_MKL_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    MKL |= imm;
    SET_Z(MKL);
}

/* 64 20: 0110 0100 0010 0000 xxxx xxxx */
void Cupd7907::ADINC_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA );
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = pa + imm;

    ZHC_ADD( tmp, pa, 0 );
    WP( cpustate, UPD7907_PORTA , tmp );
    SKIP_NC;
}

/* 64 21: 0110 0100 0010 0001 xxxx xxxx */
void Cupd7907::ADINC_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB );
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = pb + imm;

    ZHC_ADD( tmp, pb, 0 );
    WP( cpustate, UPD7907_PORTB, tmp );
    SKIP_NC;
}

/* 64 22: 0110 0100 0010 0010 xxxx xxxx */
void Cupd7907::ADINC_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC );
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = pc + imm;

    ZHC_ADD( tmp, pc, 0 );
    WP( cpustate, UPD7907_PORTC, tmp );
    SKIP_NC;
}

/* 64 23: 0110 0100 0010 0011 xxxx xxxx */
void Cupd7907::ADINC_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD );
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = pd + imm;

    ZHC_ADD( tmp, pd, 0 );
    WP( cpustate, UPD7907_PORTD, tmp );
    SKIP_NC;
}

/* 64 25: 0110 0100 0010 0101 xxxx xxxx */
void Cupd7907::ADINC_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF );
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = pf + imm;

    ZHC_ADD( tmp, pf, 0 );
    WP( cpustate, UPD7907_PORTF, tmp );
    SKIP_NC;
}

/* 64 26: 0110 0100 0010 0110 xxxx xxxx */
void Cupd7907::ADINC_MKH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKH + imm;

    ZHC_ADD( tmp, MKH, 0 );
    MKH = tmp;
    SKIP_NC;
}

/* 64 27: 0110 0100 0010 0111 xxxx xxxx */
void Cupd7907::ADINC_MKL_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKL + imm;

    ZHC_ADD( tmp, MKL, 0 );
    MKL = tmp;
    SKIP_NC;
}

/* 64 28: 0110 0100 0010 1000 xxxx xxxx */
void Cupd7907::GTI_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA ), imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = pa - imm - 1;
    ZHC_SUB( tmp, pa, 0 );

    SKIP_NC;
}

/* 64 29: 0110 0100 0010 1001 xxxx xxxx */
void Cupd7907::GTI_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB ), imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = pb - imm - 1;
    ZHC_SUB( tmp, pb, 0 );

    SKIP_NC;
}

/* 64 2a: 0110 0100 0010 1010 xxxx xxxx */
void Cupd7907::GTI_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC ), imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = pc - imm - 1;
    ZHC_SUB( tmp, pc, 0 );

    SKIP_NC;
}

/* 64 2b: 0110 0100 0010 1011 xxxx xxxx */
void Cupd7907::GTI_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD ), imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = pd - imm - 1;
    ZHC_SUB( tmp, pd, 0 );

    SKIP_NC;
}

/* 64 2d: 0110 0100 0010 1101 xxxx xxxx */
void Cupd7907::GTI_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF ), imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = pf - imm - 1;
    ZHC_SUB( tmp, pf, 0 );

    SKIP_NC;
}

/* 64 2e: 0110 0100 0010 1110 xxxx xxxx */
void Cupd7907::GTI_MKH_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = MKH - imm - 1;
    ZHC_SUB( tmp, MKH, 0 );

    SKIP_NC;
}

/* 64 2f: 0110 0100 0010 1111 xxxx xxxx */
void Cupd7907::GTI_MKL_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = MKL - imm - 1;
    ZHC_SUB( tmp, MKL, 0 );

    SKIP_NC;
}

/* 64 30: 0110 0100 0011 0000 xxxx xxxx */
void Cupd7907::SUINB_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA ), tmp, imm;

    RDOPARG( imm );
    tmp = pa - imm;
    ZHC_SUB( tmp, pa, 0 );
    pa = tmp;
    WP( cpustate, UPD7907_PORTA, pa );
    SKIP_NC;
}

/* 64 31: 0110 0100 0011 0001 xxxx xxxx */
void Cupd7907::SUINB_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB ), tmp, imm;

    RDOPARG( imm );
    tmp = pb - imm;
    ZHC_SUB( tmp, pb, 0 );
    pb = tmp;
    WP( cpustate, UPD7907_PORTB, pb );
    SKIP_NC;
}

/* 64 32: 0110 0100 0011 0010 xxxx xxxx */
void Cupd7907::SUINB_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC ), tmp, imm;

    RDOPARG( imm );
    tmp = pc - imm;
    ZHC_SUB( tmp, pc, 0 );
    pc = tmp;
    WP( cpustate, UPD7907_PORTC, pc );
    SKIP_NC;
}

/* 64 33: 0110 0100 0011 0011 xxxx xxxx */
void Cupd7907::SUINB_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD ), tmp, imm;

    RDOPARG( imm );
    tmp = pd - imm;
    ZHC_SUB( tmp, pd, 0 );
    pd = tmp;
    WP( cpustate, UPD7907_PORTD, pd );
    SKIP_NC;
}

/* 64 35: 0110 0100 0011 0101 xxxx xxxx */
void Cupd7907::SUINB_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF ), tmp, imm;

    RDOPARG( imm );
    tmp = pf - imm;
    ZHC_SUB( tmp, pf, 0 );
    pf = tmp;
    WP( cpustate, UPD7907_PORTF, pf );
    SKIP_NC;
}

/* 64 36: 0110 0100 0011 0110 xxxx xxxx */
void Cupd7907::SUINB_MKH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKH - imm;
    ZHC_SUB( tmp, MKH, 0 );
    MKH = tmp;
    SKIP_NC;
}

/* 64 37: 0110 0100 0011 0111 xxxx xxxx */
void Cupd7907::SUINB_MKL_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKL - imm;
    ZHC_SUB( tmp, MKL, 0 );
    MKL = tmp;
    SKIP_NC;
}

/* 64 38: 0110 0100 0011 1000 xxxx xxxx */
void Cupd7907::LTI_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA ), tmp, imm;
    RDOPARG( imm );
    tmp = pa - imm;
    ZHC_SUB( tmp, pa, 0 );
    SKIP_CY;
}

/* 64 39: 0110 0100 0011 1001 xxxx xxxx */
void Cupd7907::LTI_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB ), tmp, imm;
    RDOPARG( imm );
    tmp = pb - imm;
    ZHC_SUB( tmp, pb, 0 );
    SKIP_CY;
}

/* 64 3a: 0110 0100 0011 1010 xxxx xxxx */
void Cupd7907::LTI_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC ), tmp, imm;
    RDOPARG( imm );
    tmp = pc - imm;
    ZHC_SUB( tmp, pc, 0 );
    SKIP_CY;
}

/* 64 3b: 0110 0100 0011 1011 xxxx xxxx */
void Cupd7907::LTI_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD ), tmp, imm;
    RDOPARG( imm );
    tmp = pd - imm;
    ZHC_SUB( tmp, pd, 0 );
    SKIP_CY;
}

/* 64 3d: 0110 0100 0011 1101 xxxx xxxx */
void Cupd7907::LTI_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF ), tmp, imm;
    RDOPARG( imm );
    tmp = pf - imm;
    ZHC_SUB( tmp, pf, 0 );
    SKIP_CY;
}

/* 64 3e: 0110 0100 0011 1110 xxxx xxxx */
void Cupd7907::LTI_MKH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKH - imm;
    ZHC_SUB( tmp, MKH, 0 );
    SKIP_CY;
}

/* 64 3f: 0110 0100 0011 1111 xxxx xxxx */
void Cupd7907::LTI_MKL_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKL - imm;
    ZHC_SUB( tmp, MKL, 0 );
    SKIP_CY;
}

/* 64 40: 0110 0100 0100 0000 xxxx xxxx */
void Cupd7907::ADI_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA ), tmp, imm;

    RDOPARG( imm );
    tmp = pa + imm;
    ZHC_ADD( tmp, pa, 0 );
    pa = tmp;
    WP( cpustate, UPD7907_PORTA, pa );
}

/* 64 41: 0110 0100 0100 0001 xxxx xxxx */
void Cupd7907::ADI_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB ), tmp, imm;

    RDOPARG( imm );
    tmp = pb + imm;
    ZHC_ADD( tmp, pb, 0 );
    pb = tmp;
    WP( cpustate, UPD7907_PORTB, pb );
}

/* 64 42: 0110 0100 0100 0010 xxxx xxxx */
void Cupd7907::ADI_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC ), tmp, imm;

    RDOPARG( imm );
    tmp = pc + imm;
    ZHC_ADD( tmp, pc, 0 );
    pc = tmp;
    WP( cpustate, UPD7907_PORTC, pc );
}

/* 64 43: 0110 0100 0100 0011 xxxx xxxx */
void Cupd7907::ADI_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD ), tmp, imm;

    RDOPARG( imm );
    tmp = pd + imm;
    ZHC_ADD( tmp, pd, 0 );
    pd = tmp;
    WP( cpustate, UPD7907_PORTD, pd );
}

/* 64 45: 0110 0100 0100 0101 xxxx xxxx */
void Cupd7907::ADI_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF ), tmp, imm;

    RDOPARG( imm );
    tmp = pf + imm;
    ZHC_ADD( tmp, pf, 0 );
    pf = tmp;
    WP( cpustate, UPD7907_PORTF, pf );
}

/* 64 46: 0110 0100 0100 0110 xxxx xxxx */
void Cupd7907::ADI_MKH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKH + imm;
    ZHC_ADD( tmp, MKH, 0 );
    MKH = tmp;
}

/* 64 47: 0110 0100 0100 0111 xxxx xxxx */
void Cupd7907::ADI_MKL_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKL + imm;
    ZHC_ADD( tmp, MKL, 0 );
    MKL = tmp;
}

/* 64 48: 0110 0100 0100 1000 xxxx xxxx */
void Cupd7907::ONI_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA ), imm;

    RDOPARG( imm );
    if (pa & imm)
        PSW |= SK;
}

/* 64 49: 0110 0100 0100 1001 xxxx xxxx */
void Cupd7907::ONI_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB ), imm;

    RDOPARG( imm );
    if (pb & imm)
        PSW |= SK;
}

/* 64 4a: 0110 0100 0100 1010 xxxx xxxx */
void Cupd7907::ONI_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC ), imm;

    RDOPARG( imm );
    if (pc & imm)
        PSW |= SK;
}

/* 64 4b: 0110 0100 0100 1011 xxxx xxxx */
void Cupd7907::ONI_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD ), imm;

    RDOPARG( imm );
    if (pd & imm)
        PSW |= SK;
}

/* 64 4d: 0110 0100 0100 1101 xxxx xxxx */
void Cupd7907::ONI_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF ), imm;

    RDOPARG( imm );
    if (pf & imm)
        PSW |= SK;
}

/* 64 4e: 0110 0100 0100 1110 xxxx xxxx */
void Cupd7907::ONI_MKH_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (MKH & imm)
        PSW |= SK;
}

/* 64 4f: 0110 0100 0100 1111 xxxx xxxx */
void Cupd7907::ONI_MKL_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (MKL & imm)
        PSW |= SK;
}

/* 64 50: 0110 0100 0101 0000 xxxx xxxx */
void Cupd7907::ACI_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA ), tmp, imm;

    RDOPARG( imm );
    tmp = pa + imm + (PSW & CY);
    ZHC_ADD( tmp, pa, (PSW & CY) );
    pa = tmp;
    WP( cpustate, UPD7907_PORTA, pa );
}

/* 64 51: 0110 0100 0101 0001 xxxx xxxx */
void Cupd7907::ACI_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB ), tmp, imm;

    RDOPARG( imm );
    tmp = pb + imm + (PSW & CY);
    ZHC_ADD( tmp, pb, (PSW & CY) );
    pb = tmp;
    WP( cpustate, UPD7907_PORTB, pb );
}

/* 64 52: 0110 0100 0101 0010 xxxx xxxx */
void Cupd7907::ACI_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC ), tmp, imm;

    RDOPARG( imm );
    tmp = pc + imm + (PSW & CY);
    ZHC_ADD( tmp, pc, (PSW & CY) );
    pc = tmp;
    WP( cpustate, UPD7907_PORTC, pc );
}

/* 64 53: 0110 0100 0101 0011 xxxx xxxx */
void Cupd7907::ACI_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD ), tmp, imm;

    RDOPARG( imm );
    tmp = pd + imm + (PSW & CY);
    ZHC_ADD( tmp, pd, (PSW & CY) );
    pd = tmp;
    WP( cpustate, UPD7907_PORTD, pd );
}

/* 64 55: 0110 0100 0101 0101 xxxx xxxx */
void Cupd7907::ACI_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF ), tmp, imm;

    RDOPARG( imm );
    tmp = pf + imm + (PSW & CY);
    ZHC_ADD( tmp, pf, (PSW & CY) );
    pf = tmp;
    WP( cpustate, UPD7907_PORTF, pf );
}

/* 64 56: 0110 0100 0101 0110 xxxx xxxx */
void Cupd7907::ACI_MKH_xx(upd7907_state *cpustate)
{
    UINT8 imm, tmp;

    RDOPARG( imm );
    tmp = MKH + imm + (PSW & CY);
    ZHC_ADD( tmp, MKH, (PSW & CY) );
    MKH = tmp;
}

/* 64 57: 0110 0100 0101 0111 xxxx xxxx */
void Cupd7907::ACI_MKL_xx(upd7907_state *cpustate)
{
    UINT8 imm, tmp;

    RDOPARG( imm );
    tmp = MKL + imm + (PSW & CY);
    ZHC_ADD( tmp, MKL, (PSW & CY) );
    MKL = tmp;
}

/* 64 58: 0110 0100 0101 1000 xxxx xxxx */
void Cupd7907::OFFI_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA ), imm;

    RDOPARG( imm );
    if (0 == (pa & imm))
        PSW |= SK;
}

/* 64 59: 0110 0100 0101 1001 xxxx xxxx */
void Cupd7907::OFFI_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB ), imm;

    RDOPARG( imm );
    if (0 == (pb & imm))
        PSW |= SK;
}

/* 64 5a: 0110 0100 0101 1010 xxxx xxxx */
void Cupd7907::OFFI_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC ), imm;

    RDOPARG( imm );
    if (0 == (pc & imm))
        PSW |= SK;
}

/* 64 5b: 0110 0100 0101 1011 xxxx xxxx */
void Cupd7907::OFFI_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD ), imm;

    RDOPARG( imm );
    if (0 == (pd & imm))
        PSW |= SK;
}

/* 64 5d: 0110 0100 0101 1101 xxxx xxxx */
void Cupd7907::OFFI_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF ), imm;

    RDOPARG( imm );
    if (0 == (pf & imm))
        PSW |= SK;
}

/* 64 5e: 0110 0100 0101 1110 xxxx xxxx */
void Cupd7907::OFFI_MKH_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (0 == (MKH & imm))
        PSW |= SK;
}

/* 64 5f: 0110 0100 0101 1111 xxxx xxxx */
void Cupd7907::OFFI_MKL_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (0 == (MKL & imm))
        PSW |= SK;
}

/* 64 60: 0110 0100 0110 0000 xxxx xxxx */
void Cupd7907::SUI_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA ), tmp, imm;

    RDOPARG( imm );
    tmp = pa - imm;
    ZHC_SUB( tmp, pa, 0 );
    pa = tmp;
    WP( cpustate, UPD7907_PORTA, pa );
}

/* 64 61: 0110 0100 0110 0001 xxxx xxxx */
void Cupd7907::SUI_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB ), tmp, imm;

    RDOPARG( imm );
    tmp = pb - imm;
    ZHC_SUB( tmp, pb, 0 );
    pb = tmp;
    WP( cpustate, UPD7907_PORTB, pb );
}

/* 64 62: 0110 0100 0110 0010 xxxx xxxx */
void Cupd7907::SUI_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC ), tmp, imm;

    RDOPARG( imm );
    tmp = pc - imm;
    ZHC_SUB( tmp, pc, 0 );
    pc = tmp;
    WP( cpustate, UPD7907_PORTC, pc );
}

/* 64 63: 0110 0100 0110 0011 xxxx xxxx */
void Cupd7907::SUI_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD ), tmp, imm;

    RDOPARG( imm );
    tmp = pd - imm;
    ZHC_SUB( tmp, pd, 0 );
    pd = tmp;
    WP( cpustate, UPD7907_PORTD, pd );
}

/* 64 65: 0110 0100 0110 0101 xxxx xxxx */
void Cupd7907::SUI_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF ), tmp, imm;

    RDOPARG( imm );
    tmp = pf - imm;
    ZHC_SUB( tmp, pf, 0 );
    pf = tmp;
    WP( cpustate, UPD7907_PORTF, pf );
}

/* 64 66: 0110 0100 0110 0110 xxxx xxxx */
void Cupd7907::SUI_MKH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKH - imm;
    ZHC_SUB( tmp, MKH, 0 );
    MKH = tmp;
}

/* 64 67: 0110 0100 0110 0111 xxxx xxxx */
void Cupd7907::SUI_MKL_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKL - imm;
    ZHC_SUB( tmp, MKL, 0 );
    MKL = tmp;
}

/* 64 68: 0110 0100 0110 1000 xxxx xxxx */
void Cupd7907::NEI_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA ), tmp, imm;

    RDOPARG( imm );
    tmp = pa - imm;
    ZHC_SUB( tmp, pa, 0 );
    SKIP_NZ;
}

/* 64 69: 0110 0100 0110 1001 xxxx xxxx */
void Cupd7907::NEI_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB ), tmp, imm;

    RDOPARG( imm );
    tmp = pb - imm;
    ZHC_SUB( tmp, pb, 0 );
    SKIP_NZ;
}

/* 64 6a: 0110 0100 0110 1010 xxxx xxxx */
void Cupd7907::NEI_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC ), tmp, imm;

    RDOPARG( imm );
    tmp = pc - imm;
    ZHC_SUB( tmp, pc, 0 );
    SKIP_NZ;
}

/* 64 6b: 0110 0100 0110 1011 xxxx xxxx */
void Cupd7907::NEI_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD ), tmp, imm;

    RDOPARG( imm );
    tmp = pd - imm;
    ZHC_SUB( tmp, pd, 0 );
    SKIP_NZ;
}

/* 64 6d: 0110 0100 0110 1101 xxxx xxxx */
void Cupd7907::NEI_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF ), tmp, imm;

    RDOPARG( imm );
    tmp = pf - imm;
    ZHC_SUB( tmp, pf, 0 );
    SKIP_NZ;
}

/* 64 6e: 0110 0100 0110 1110 xxxx xxxx */
void Cupd7907::NEI_MKH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKH - imm;
    ZHC_SUB( tmp, MKH, 0 );
    SKIP_NZ;
}

/* 64 6f: 0110 0100 0110 1111 xxxx xxxx */
void Cupd7907::NEI_MKL_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKL - imm;
    ZHC_SUB( tmp, MKL, 0 );
    SKIP_NZ;
}

/* 64 70: 0110 0100 0111 0000 xxxx xxxx */
void Cupd7907::SBI_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA ), tmp, imm;

    RDOPARG( imm );
    tmp = pa - imm - (PSW & CY);
    ZHC_SUB( tmp, pa, (PSW & CY) );
    pa = tmp;
    WP( cpustate, UPD7907_PORTA, pa );
}

/* 64 71: 0110 0100 0111 0001 xxxx xxxx */
void Cupd7907::SBI_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB ), tmp, imm;

    RDOPARG( imm );
    tmp = pb - imm - (PSW & CY);
    ZHC_SUB( tmp, pb, (PSW & CY) );
    pb = tmp;
    WP( cpustate, UPD7907_PORTB, pb );
}

/* 64 72: 0110 0100 0111 0010 xxxx xxxx */
void Cupd7907::SBI_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC ), tmp, imm;

    RDOPARG( imm );
    tmp = pc - imm - (PSW & CY);
    ZHC_SUB( tmp, pc, (PSW & CY) );
    pc = tmp;
    WP( cpustate, UPD7907_PORTC, pc );
}

/* 64 73: 0110 0100 0111 0011 xxxx xxxx */
void Cupd7907::SBI_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD ), tmp, imm;

    RDOPARG( imm );
    tmp = pd - imm - (PSW & CY);
    ZHC_SUB( tmp, pd, (PSW & CY) );
    pd = tmp;
    WP( cpustate, UPD7907_PORTD, pd );
}

/* 64 75: 0110 0100 0111 0101 xxxx xxxx */
void Cupd7907::SBI_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF ), tmp, imm;

    RDOPARG( imm );
    tmp = pf - imm - (PSW & CY);
    ZHC_SUB( tmp, pf, (PSW & CY) );
    pf = tmp;
    WP( cpustate, UPD7907_PORTF, pf );
}

/* 64 76: 0110 0100 0111 0110 xxxx xxxx */
void Cupd7907::SBI_MKH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKH - imm - (PSW & CY);
    ZHC_SUB( tmp, MKH, (PSW & CY) );
    MKH = tmp;
}

/* 64 77: 0110 0100 0111 0111 xxxx xxxx */
void Cupd7907::SBI_MKL_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKL - imm - (PSW & CY);
    ZHC_SUB( tmp, MKL, (PSW & CY) );
    MKL = tmp;
}

/* 64 78: 0110 0100 0111 1000 xxxx xxxx */
void Cupd7907::EQI_PA_xx(upd7907_state *cpustate)
{
    UINT8 pa = RP( cpustate, UPD7907_PORTA ), tmp, imm;

    RDOPARG( imm );
    tmp = pa - imm;
    ZHC_SUB( tmp, pa, 0 );
    SKIP_Z;
}

/* 64 79: 0110 0100 0111 1001 xxxx xxxx */
void Cupd7907::EQI_PB_xx(upd7907_state *cpustate)
{
    UINT8 pb = RP( cpustate, UPD7907_PORTB ), tmp, imm;

    RDOPARG( imm );
    tmp = pb - imm;
    ZHC_SUB( tmp, pb, 0 );
    SKIP_Z;
}

/* 64 7a: 0110 0100 0111 1010 xxxx xxxx */
void Cupd7907::EQI_PC_xx(upd7907_state *cpustate)
{
    UINT8 pc = RP( cpustate, UPD7907_PORTC ), tmp, imm;

    RDOPARG( imm );
    tmp = pc - imm;
    ZHC_SUB( tmp, pc, 0 );
    SKIP_Z;
}

/* 64 7b: 0110 0100 0111 1011 xxxx xxxx */
void Cupd7907::EQI_PD_xx(upd7907_state *cpustate)
{
    UINT8 pd = RP( cpustate, UPD7907_PORTD ), tmp, imm;

    RDOPARG( imm );
    tmp = pd - imm;
    ZHC_SUB( tmp, pd, 0 );
    SKIP_Z;
}

/* 64 7d: 0110 0100 0111 1101 xxxx xxxx */
void Cupd7907::EQI_PF_xx(upd7907_state *cpustate)
{
    UINT8 pf = RP( cpustate, UPD7907_PORTF ), tmp, imm;

    RDOPARG( imm );
    tmp = pf - imm;
    ZHC_SUB( tmp, pf, 0 );
    SKIP_Z;
}

/* 64 7e: 0110 0100 0111 1110 xxxx xxxx */
void Cupd7907::EQI_MKH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKH - imm;
    ZHC_SUB( tmp, MKH, 0 );
    SKIP_Z;
}

/* 64 7f: 0110 0100 0111 1111 xxxx xxxx */
void Cupd7907::EQI_MKL_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = MKL - imm;
    ZHC_SUB( tmp, MKL, 0 );
    SKIP_Z;
}

/* 64 80: 0110 0100 1000 0000 xxxx xxxx */
void Cupd7907::MVI_ANM_xx(upd7907_state *cpustate)
{
    RDOPARG( ANM );
}

/* 64 81: 0110 0100 1000 0001 xxxx xxxx */
void Cupd7907::MVI_SMH_xx(upd7907_state *cpustate)
{
    RDOPARG( SMH );
}

/* 64 83: 0110 0100 1000 0011 xxxx xxxx */
void Cupd7907::MVI_EOM_xx(upd7907_state *cpustate)
{
    RDOPARG( EOM );
    upd7907_write_EOM(cpustate);
}

/* 64 85: 0110 0100 1000 0101 xxxx xxxx */
void Cupd7907::MVI_TMM_xx(upd7907_state *cpustate)
{
    RDOPARG( TMM );
}

/* 64 88: 0110 0100 1000 1000 xxxx xxxx */
void Cupd7907::ANI_ANM_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    ANM &= imm;
    SET_Z(ANM);
}

/* 64 89: 0110 0100 1000 1001 xxxx xxxx */
void Cupd7907::ANI_SMH_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    SMH &= imm;
    SET_Z(SMH);
}

/* 64 8b: 0110 0100 1000 1011 xxxx xxxx */
void Cupd7907::ANI_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 imm;

    RDOPARG( imm );
    /* only bits #1 and #5 can be read */
    EOM = eom & imm;
    SET_Z(EOM);
    upd7907_write_EOM(cpustate);
}

/* 64 8d: 0110 0100 1000 1101 xxxx xxxx */
void Cupd7907::ANI_TMM_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    TMM &= imm;
    SET_Z(TMM);
}

/* 64 90: 0110 0100 1001 0000 xxxx xxxx */
void Cupd7907::XRI_ANM_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    ANM ^= imm;
    SET_Z(ANM);
}

/* 64 91: 0110 0100 1001 0001 xxxx xxxx */
void Cupd7907::XRI_SMH_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    SMH ^= imm;
    SET_Z(SMH);
}

/* 64 93: 0110 0100 1001 0011 xxxx xxxx */
void Cupd7907::XRI_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 imm;

    RDOPARG( imm );
    /* only bits #1 and #5 can be read */
    EOM = eom ^ imm;
    SET_Z(EOM);
    upd7907_write_EOM(cpustate);
}

/* 64 95: 0110 0100 1001 0101 xxxx xxxx */
void Cupd7907::XRI_TMM_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    TMM ^= imm;
    SET_Z(TMM);
}

/* 64 98: 0110 0100 1001 1000 xxxx xxxx */
void Cupd7907::ORI_ANM_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    ANM |= imm;
    SET_Z(ANM);
}

/* 64 99: 0110 0100 1001 1001 xxxx xxxx */
void Cupd7907::ORI_SMH_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    SMH |= imm;
    SET_Z(SMH);
}

/* 64 9b: 0110 0100 1001 1011 xxxx xxxx */
void Cupd7907::ORI_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 imm;

    RDOPARG( imm );
    /* only bits #1 and #5 can be read */
    EOM = eom | imm;
    SET_Z(EOM);
    upd7907_write_EOM(cpustate);
}

/* 64 9d: 0110 0100 1001 1101 xxxx xxxx */
void Cupd7907::ORI_TMM_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    TMM |= imm;
    SET_Z(TMM);
}

/* 64 a0: 0110 0100 1010 0000 xxxx xxxx */
void Cupd7907::ADINC_ANM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = ANM + imm;

    ZHC_ADD( tmp, ANM, 0 );
    ANM = tmp;
    SKIP_NC;
}

/* 64 a1: 0110 0100 1010 0001 xxxx xxxx */
void Cupd7907::ADINC_SMH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = SMH + imm;

    ZHC_ADD( tmp, SMH, 0 );
    SMH = tmp;
    SKIP_NC;
}

/* 64 a3: 0110 0100 1010 0011 xxxx xxxx */
void Cupd7907::ADINC_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 tmp, imm;

    RDOPARG( imm );
    /* only bits #1 and #5 can be read */
    tmp = eom + imm;

    ZHC_ADD( tmp, eom, 0 );
    EOM = tmp;
    SKIP_NC;
    upd7907_write_EOM(cpustate);
}

/* 64 a5: 0110 0100 1010 0101 xxxx xxxx */
void Cupd7907::ADINC_TMM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = TMM + imm;

    ZHC_ADD( tmp, TMM, 0 );
    TMM = tmp;
    SKIP_NC;
}

/* 64 a8: 0110 0100 1010 1000 xxxx xxxx */
void Cupd7907::GTI_ANM_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = ANM - imm - 1;
    ZHC_SUB( tmp, ANM, 0 );

    SKIP_NC;
}

/* 64 a9: 0110 0100 1010 1001 xxxx xxxx */
void Cupd7907::GTI_SMH_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = SMH - imm - 1;
    ZHC_SUB( tmp, SMH, 0 );

    SKIP_NC;
}

/* 64 ab: 0110 0100 1010 1011 xxxx xxxx */
void Cupd7907::GTI_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = eom - imm - 1;
    ZHC_SUB( tmp, eom, 0 );

    SKIP_NC;
}

/* 64 ad: 0110 0100 1010 1101 xxxx xxxx */
void Cupd7907::GTI_TMM_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = TMM - imm - 1;
    ZHC_SUB( tmp, TMM, 0 );

    SKIP_NC;
}

/* 64 b0: 0110 0100 1011 0000 xxxx xxxx */
void Cupd7907::SUINB_ANM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = ANM - imm;
    ZHC_SUB( tmp, ANM, 0 );
    ANM = tmp;
    SKIP_NC;
}

/* 64 b1: 0110 0100 1011 0001 xxxx xxxx */
void Cupd7907::SUINB_SMH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = SMH - imm;
    ZHC_SUB( tmp, SMH, 0 );
    SMH = tmp;
    SKIP_NC;
}

/* 64 b3: 0110 0100 1011 0011 xxxx xxxx */
void Cupd7907::SUINB_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = eom - imm;
    ZHC_SUB( tmp, eom, 0 );
    EOM = tmp;
    SKIP_NC;
    upd7907_write_EOM(cpustate);
}

/* 64 b5: 0110 0100 1011 0101 xxxx xxxx */
void Cupd7907::SUINB_TMM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = TMM - imm;
    ZHC_SUB( tmp, TMM, 0 );
    TMM = tmp;
    SKIP_NC;
}

/* 64 b8: 0110 0100 1011 1000 xxxx xxxx */
void Cupd7907::LTI_ANM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = ANM - imm;
    ZHC_SUB( tmp, ANM, 0 );
    SKIP_CY;
}

/* 64 b9: 0110 0100 1011 1001 xxxx xxxx */
void Cupd7907::LTI_SMH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = SMH - imm;
    ZHC_SUB( tmp, SMH, 0 );
    SKIP_CY;
}

/* 64 bb: 0110 0100 1011 1011 xxxx xxxx */
void Cupd7907::LTI_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = eom - imm;
    ZHC_SUB( tmp, eom, 0 );
    SKIP_CY;
}

/* 64 bd: 0110 0100 1011 1101 xxxx xxxx */
void Cupd7907::LTI_TMM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = TMM - imm;
    ZHC_SUB( tmp, TMM, 0 );
    SKIP_CY;
}

/* 64 c0: 0110 0100 1100 0000 xxxx xxxx */
void Cupd7907::ADI_ANM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = ANM + imm;

    ZHC_ADD( tmp, ANM, 0 );
    ANM = tmp;
}

/* 64 c1: 0110 0100 1100 0001 xxxx xxxx */
void Cupd7907::ADI_SMH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = SMH + imm;

    ZHC_ADD( tmp, SMH, 0 );
    SMH = tmp;
}

/* 64 c3: 0110 0100 1100 0011 xxxx xxxx */
void Cupd7907::ADI_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = eom + imm;

    ZHC_ADD( tmp, eom, 0 );
    EOM = tmp;
    upd7907_write_EOM(cpustate);
}

/* 64 c5: 0110 0100 1100 0101 xxxx xxxx */
void Cupd7907::ADI_TMM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = TMM + imm;

    ZHC_ADD( tmp, TMM, 0 );
    TMM = tmp;
}

/* 64 c8: 0110 0100 1100 1000 xxxx xxxx */
void Cupd7907::ONI_ANM_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (ANM & imm)
        PSW |= SK;
}

/* 64 c9: 0110 0100 1100 1001 xxxx xxxx */
void Cupd7907::ONI_SMH_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (SMH & imm)
        PSW |= SK;
}

/* 64 cb: 0110 0100 1100 1011 xxxx xxxx */
void Cupd7907::ONI_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 imm;

    RDOPARG( imm );
    if (eom & imm)
        PSW |= SK;
}

/* 64 cd: 0110 0100 1100 1101 xxxx xxxx */
void Cupd7907::ONI_TMM_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (TMM & imm)
        PSW |= SK;
}

/* 64 d0: 0110 0100 1101 0000 xxxx xxxx */
void Cupd7907::ACI_ANM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = ANM + imm + (PSW & CY);

    ZHC_ADD( tmp, ANM, (PSW & CY) );
    ANM = tmp;
}

/* 64 d1: 0110 0100 1101 0001 xxxx xxxx */
void Cupd7907::ACI_SMH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = SMH + imm + (PSW & CY);

    ZHC_ADD( tmp, SMH, (PSW & CY) );
    SMH = tmp;
}

/* 64 d3: 0110 0100 1101 0011 xxxx xxxx */
void Cupd7907::ACI_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = eom + imm + (PSW & CY);

    ZHC_ADD( tmp, eom, (PSW & CY) );
    EOM = tmp;
    upd7907_write_EOM(cpustate);
}

/* 64 d5: 0110 0100 1101 0101 xxxx xxxx */
void Cupd7907::ACI_TMM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = TMM + imm + (PSW & CY);

    ZHC_ADD( tmp, TMM, (PSW & CY) );
    TMM = tmp;
}

/* 64 d8: 0110 0100 1101 1000 xxxx xxxx */
void Cupd7907::OFFI_ANM_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (0 == (ANM & imm))
        PSW |= SK;
}

/* 64 d9: 0110 0100 1101 1001 xxxx xxxx */
void Cupd7907::OFFI_SMH_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (0 == (SMH & imm))
        PSW |= SK;
}

/* 64 db: 0110 0100 1101 1011 xxxx xxxx */
void Cupd7907::OFFI_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 imm;

    RDOPARG( imm );
    if (0 == (eom & imm))
        PSW |= SK;
}

/* 64 dd: 0110 0100 1101 1101 xxxx xxxx */
void Cupd7907::OFFI_TMM_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (0 == (TMM & imm))
        PSW |= SK;
}

/* 64 e0: 0110 0100 1110 0000 xxxx xxxx */
void Cupd7907::SUI_ANM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = ANM - imm;
    ZHC_SUB( tmp, ANM, 0 );
    ANM = tmp;
}

/* 64 e1: 0110 0100 1110 0001 xxxx xxxx */
void Cupd7907::SUI_SMH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = SMH - imm;
    ZHC_SUB( tmp, SMH, 0 );
    SMH = tmp;
}

/* 64 e3: 0110 0100 1110 0011 xxxx xxxx */
void Cupd7907::SUI_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = eom - imm;
    ZHC_SUB( tmp, eom, 0 );
    EOM = tmp;
    upd7907_write_EOM(cpustate);
}

/* 64 e5: 0110 0100 1110 0101 xxxx xxxx */
void Cupd7907::SUI_TMM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = TMM - imm;
    ZHC_SUB( tmp, TMM, 0 );
    TMM = tmp;
}

/* 64 e8: 0110 0100 1110 1000 xxxx xxxx */
void Cupd7907::NEI_ANM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = ANM - imm;
    ZHC_SUB( tmp, ANM, 0 );
    SKIP_NZ;
}

/* 64 e9: 0110 0100 1110 1001 xxxx xxxx */
void Cupd7907::NEI_SMH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = SMH - imm;
    ZHC_SUB( tmp, SMH, 0 );
    SKIP_NZ;
}

/* 64 eb: 0110 0100 1110 1011 xxxx xxxx */
void Cupd7907::NEI_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = eom - imm;
    ZHC_SUB( tmp, eom, 0 );
    SKIP_NZ;
}

/* 64 ed: 0110 0100 1110 1101 xxxx xxxx */
void Cupd7907::NEI_TMM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = TMM - imm;
    ZHC_SUB( tmp, TMM, 0 );
    SKIP_NZ;
}

/* 64 f0: 0110 0100 1111 0000 xxxx xxxx */
void Cupd7907::SBI_ANM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = ANM - imm - (PSW & CY);
    ZHC_SUB( tmp, ANM, (PSW & CY) );
    ANM = tmp;
}

/* 64 f1: 0110 0100 1111 0001 xxxx xxxx */
void Cupd7907::SBI_SMH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = SMH - imm - (PSW & CY);
    ZHC_SUB( tmp, SMH, (PSW & CY) );
    SMH = tmp;
}

/* 64 f3: 0110 0100 1111 0011 xxxx xxxx */
void Cupd7907::SBI_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = eom - imm - (PSW & CY);
    ZHC_SUB( tmp, eom, (PSW & CY) );
    EOM = tmp;
    upd7907_write_EOM(cpustate);
}

/* 64 f5: 0110 0100 1111 0101 xxxx xxxx */
void Cupd7907::SBI_TMM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = TMM - imm - (PSW & CY);
    ZHC_SUB( tmp, TMM, (PSW & CY) );
    TMM = tmp;
}

/* 64 f8: 0110 0100 1111 1000 xxxx xxxx */
void Cupd7907::EQI_ANM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = ANM - imm;
    ZHC_SUB( tmp, ANM, 0 );
    SKIP_Z;
}

/* 64 f9: 0110 0100 1111 1001 xxxx xxxx */
void Cupd7907::EQI_SMH_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = SMH - imm;
    ZHC_SUB( tmp, SMH, 0 );
    SKIP_Z;
}

/* 64 fb: 0110 0100 1111 1011 xxxx xxxx */
void Cupd7907::EQI_EOM_xx(upd7907_state *cpustate)
{
    /* only bits #1 and #5 can be read */
    UINT8 eom = EOM & 0x22;
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = eom - imm;
    ZHC_SUB( tmp, eom, 0 );
    SKIP_Z;
}

/* 64 fd: 0110 0100 1111 1101 xxxx xxxx */
void Cupd7907::EQI_TMM_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = TMM - imm;
    ZHC_SUB( tmp, TMM, 0 );
    SKIP_Z;
}

/* prefix 70 */
/* 70 0e: 0111 0000 0000 1110 llll llll hhhh hhhh */
void Cupd7907::SSPD_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    WM( ea.d, SPL );
    WM( ea.d + 1, SPH );
}

/* 70 0f: 0111 0000 0000 1111 llll llll hhhh hhhh */
void Cupd7907::LSPD_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    SPL = RM( ea.d );
    SPH = RM( ea.d + 1 );
}

/* 70 1e: 0111 0000 0001 1110 llll llll hhhh hhhh */
void Cupd7907::SBCD_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    WM( ea.d, C );
    WM( ea.d + 1, B );
}

/* 70 1f: 0111 0000 0001 1111 llll llll hhhh hhhh */
void Cupd7907::LBCD_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    C = RM( ea.d );
    B = RM( ea.d + 1 );
}

/* 70 2e: 0111 0000 0010 1110 llll llll hhhh hhhh */
void Cupd7907::SDED_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    WM( ea.d, E );
    WM( ea.d + 1, D );
}

/* 70 2f: 0111 0000 0010 1111 llll llll hhhh hhhh */
void Cupd7907::LDED_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    E = RM( ea.d );
    D = RM( ea.d + 1 );
}

/* 70 3e: 0111 0000 0011 1110 llll llll hhhh hhhh */
void Cupd7907::SHLD_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    WM( ea.d, L );
    WM( ea.d + 1, H );
}

/* 70 3f: 0111 0000 0011 1111 llll llll hhhh hhhh */
void Cupd7907::LHLD_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    L = RM( ea.d );
    H = RM( ea.d + 1 );
}

/* 70 41: 0111 0000 0100 0001 */
void Cupd7907::EADD_EA_A(upd7907_state *cpustate)
{
    UINT16 tmp;
    tmp = EA + A;
    ZHC_ADD( tmp, EA, 0 );
    EA = tmp;
}

/* 70 42: 0111 0000 0100 0010 */
void Cupd7907::EADD_EA_B(upd7907_state *cpustate)
{
    UINT16 tmp;
    tmp = EA + B;
    ZHC_ADD( tmp, EA, 0 );
    EA = tmp;
}

/* 70 43: 0111 0000 0100 0011 */
void Cupd7907::EADD_EA_C(upd7907_state *cpustate)
{
    UINT16 tmp;
    tmp = EA + C;
    ZHC_ADD( tmp, EA, 0 );
    EA = tmp;
}

/* 70 61: 0111 0000 0110 0001 */
void Cupd7907::ESUB_EA_A(upd7907_state *cpustate)
{
    UINT16 tmp;
    tmp = EA - A;
    ZHC_SUB( tmp, EA, 0 );
    EA = tmp;
}

/* 70 62: 0111 0000 0110 0010 */
void Cupd7907::ESUB_EA_B(upd7907_state *cpustate)
{
    UINT16 tmp;
    tmp = EA - B;
    ZHC_SUB( tmp, EA, 0 );
    EA = tmp;
}

/* 70 63: 0111 0000 0110 0011 */
void Cupd7907::ESUB_EA_C(upd7907_state *cpustate)
{
    UINT16 tmp;
    tmp = EA - C;
    ZHC_SUB( tmp, EA, 0 );
    EA = tmp;
}

/* 70 68: 0111 0000 0110 1000 llll llll hhhh hhhh */
void Cupd7907::MOV_V_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    V = RM( ea.d );
}

/* 70 69: 0111 0000 0110 1001 llll llll hhhh hhhh */
void Cupd7907::MOV_A_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    A = RM( ea.d );
}

/* 70 6a: 0111 0000 0110 1010 llll llll hhhh hhhh */
void Cupd7907::MOV_B_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    B = RM( ea.d );
}

/* 70 6b: 0111 0000 0110 1011 llll llll hhhh hhhh */
void Cupd7907::MOV_C_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    C = RM( ea.d );
}

/* 70 6c: 0111 0000 0110 1100 llll llll hhhh hhhh */
void Cupd7907::MOV_D_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    D = RM( ea.d );
}

/* 70 6d: 0111 0000 0110 1101 llll llll hhhh hhhh */
void Cupd7907::MOV_E_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    E = RM( ea.d );
}

/* 70 6e: 0111 0000 0110 1110 llll llll hhhh hhhh */
void Cupd7907::MOV_H_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    H = RM( ea.d );
}

/* 70 6f: 0111 0000 0110 1111 llll llll hhhh hhhh */
void Cupd7907::MOV_L_w(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    L = RM( ea.d );
}

/* 70 78: 0111 0000 0111 1000 llll llll hhhh hhhh */
void Cupd7907::MOV_w_V(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    WM( ea.d, V );
}

/* 70 79: 0111 0000 0111 1001 llll llll hhhh hhhh */
void Cupd7907::MOV_w_A(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    WM( ea.d, A );
}

/* 70 7a: 0111 0000 0111 1010 llll llll hhhh hhhh */
void Cupd7907::MOV_w_B(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    WM( ea.d, B );
}

/* 70 7b: 0111 0000 0111 1011 llll llll hhhh hhhh */
void Cupd7907::MOV_w_C(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    WM( ea.d, C );
}

/* 70 7c: 0111 0000 0111 1100 llll llll hhhh hhhh */
void Cupd7907::MOV_w_D(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    WM( ea.d, D );
}

/* 70 7d: 0111 0000 0111 1101 llll llll hhhh hhhh */
void Cupd7907::MOV_w_E(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    WM( ea.d, E );
}

/* 70 7e: 0111 0000 0111 1110 llll llll hhhh hhhh */
void Cupd7907::MOV_w_H(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    WM( ea.d, H );
}

/* 70 7f: 0111 0000 0111 1111 llll llll hhhh hhhh */
void Cupd7907::MOV_w_L(upd7907_state *cpustate)
{
    PAIR ea;
    ea.d = 0;

    RDOPARG( ea.b.l );
    RDOPARG( ea.b.h );
    WM( ea.d, L );
}

/* 70 89: 0111 0000 1000 1001 */
void Cupd7907::ANAX_B(upd7907_state *cpustate)
{
    A &= RM( BC );
    SET_Z(A);
}

/* 70 8a: 0111 0000 1000 1010 */
void Cupd7907::ANAX_D(upd7907_state *cpustate)
{
    A &= RM( DE );
    SET_Z(A);
}

/* 70 8b: 0111 0000 1000 1011 */
void Cupd7907::ANAX_H(upd7907_state *cpustate)
{
    A &= RM( HL );
    SET_Z(A);
}

/* 70 8c: 0111 0000 1000 1100 */
void Cupd7907::ANAX_Dp(upd7907_state *cpustate)
{
    A &= RM( DE );
    DE++;
    SET_Z(A);
}

/* 70 8d: 0111 0000 1000 1101 */
void Cupd7907::ANAX_Hp(upd7907_state *cpustate)
{
    A &= RM( HL );
    HL++;
    SET_Z(A);
}

/* 70 8e: 0111 0000 1000 1110 */
void Cupd7907::ANAX_Dm(upd7907_state *cpustate)
{
    A &= RM( DE );
    DE--;
    SET_Z(A);
}

/* 70 8f: 0111 0000 1000 1111 */
void Cupd7907::ANAX_Hm(upd7907_state *cpustate)
{
    A &= RM( HL );
    HL--;
    SET_Z(A);
}

/* 70 91: 0111 0000 1001 0001 */
void Cupd7907::XRAX_B(upd7907_state *cpustate)
{
    A ^= RM( BC );
    SET_Z(A);
}

/* 70 92: 0111 0000 1001 0010 */
void Cupd7907::XRAX_D(upd7907_state *cpustate)
{
    A ^= RM( DE );
    SET_Z(A);
}

/* 70 93: 0111 0000 1001 0011 */
void Cupd7907::XRAX_H(upd7907_state *cpustate)
{
    A ^= RM( HL );
    SET_Z(A);
}

/* 70 94: 0111 0000 1001 0100 */
void Cupd7907::XRAX_Dp(upd7907_state *cpustate)
{
    A ^= RM( DE );
    DE++;
    SET_Z(A);
}

/* 70 95: 0111 0000 1001 0101 */
void Cupd7907::XRAX_Hp(upd7907_state *cpustate)
{
    A ^= RM( HL );
    HL++;
    SET_Z(A);
}

/* 70 96: 0111 0000 1001 0110 */
void Cupd7907::XRAX_Dm(upd7907_state *cpustate)
{
    A ^= RM( DE );
    DE--;
    SET_Z(A);
}

/* 70 97: 0111 0000 1001 0111 */
void Cupd7907::XRAX_Hm(upd7907_state *cpustate)
{
    A ^= RM( HL );
    HL--;
    SET_Z(A);
}

/* 70 99: 0111 0000 1001 1001 */
void Cupd7907::ORAX_B(upd7907_state *cpustate)
{
    A |= RM( BC );
    SET_Z(A);
}

/* 70 9a: 0111 0000 1001 1010 */
void Cupd7907::ORAX_D(upd7907_state *cpustate)
{
    A |= RM( DE );
    SET_Z(A);
}

/* 70 9b: 0111 0000 1001 1011 */
void Cupd7907::ORAX_H(upd7907_state *cpustate)
{
    A |= RM( HL );
    SET_Z(A);
}

/* 70 9c: 0111 0000 1001 0100 */
void Cupd7907::ORAX_Dp(upd7907_state *cpustate)
{
    A |= RM( DE );
    DE++;
    SET_Z(A);
}

/* 70 9d: 0111 0000 1001 1101 */
void Cupd7907::ORAX_Hp(upd7907_state *cpustate)
{
    A |= RM( HL );
    HL++;
    SET_Z(A);
}

/* 70 9e: 0111 0000 1001 1110 */
void Cupd7907::ORAX_Dm(upd7907_state *cpustate)
{
    A |= RM( DE );
    DE--;
    SET_Z(A);
}

/* 70 9f: 0111 0000 1001 1111 */
void Cupd7907::ORAX_Hm(upd7907_state *cpustate)
{
    A |= RM( HL );
    HL--;
    SET_Z(A);
}

/* 70 a1: 0111 0000 1010 0001 */
void Cupd7907::ADDNCX_B(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( BC );
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 70 a2: 0111 0000 1010 0010 */
void Cupd7907::ADDNCX_D(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( DE );
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 70 a3: 0111 0000 1010 0011 */
void Cupd7907::ADDNCX_H(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( HL );
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 70 a4: 0111 0000 1010 0100 */
void Cupd7907::ADDNCX_Dp(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( DE );
    DE++;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 70 a5: 0111 0000 1010 0101 */
void Cupd7907::ADDNCX_Hp(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( HL );
    HL++;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 70 a6: 0111 0000 1010 0110 */
void Cupd7907::ADDNCX_Dm(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( DE );
    DE--;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 70 a7: 0111 0000 1010 0111 */
void Cupd7907::ADDNCX_Hm(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( HL );
    HL--;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 70 a9: 0111 0000 1010 1001 */
void Cupd7907::GTAX_B(upd7907_state *cpustate)
{
    UINT16 tmp = A - RM( BC ) - 1;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 70 aa: 0111 0000 1010 1010 */
void Cupd7907::GTAX_D(upd7907_state *cpustate)
{
    UINT16 tmp = A - RM( DE ) - 1;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 70 ab: 0111 0000 1010 1011 */
void Cupd7907::GTAX_H(upd7907_state *cpustate)
{
    UINT16 tmp = A - RM( HL ) - 1;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 70 ac: 0111 0000 1010 1100 */
void Cupd7907::GTAX_Dp(upd7907_state *cpustate)
{
    UINT16 tmp = A - RM( DE ) - 1;
    DE++;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 70 ad: 0111 0000 1010 1101 */
void Cupd7907::GTAX_Hp(upd7907_state *cpustate)
{
    UINT16 tmp = A - RM( HL ) - 1;
    HL++;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 70 ae: 0111 0000 1010 1110 */
void Cupd7907::GTAX_Dm(upd7907_state *cpustate)
{
    UINT16 tmp = A - RM( DE ) - 1;
    DE--;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 70 af: 0111 0000 1010 1111 */
void Cupd7907::GTAX_Hm(upd7907_state *cpustate)
{
    UINT16 tmp = A - RM( HL ) - 1;
    HL--;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 70 b1: 0111 0000 1011 0001 */
void Cupd7907::SUBNBX_B(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( BC );
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 70 b2: 0111 0000 1011 0010 */
void Cupd7907::SUBNBX_D(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 70 b3: 0111 0000 1011 0011 */
void Cupd7907::SUBNBX_H(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 70 b4: 0111 0000 1011 0100 */
void Cupd7907::SUBNBX_Dp(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    DE++;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 70 b5: 0111 0000 1011 0101 */
void Cupd7907::SUBNBX_Hp(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    HL++;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 70 b6: 0111 0000 1011 0110 */
void Cupd7907::SUBNBX_Dm(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    DE--;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 70 b7: 0111 0000 1011 0111 */
void Cupd7907::SUBNBX_Hm(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    HL--;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 70 b9: 0111 0000 1011 1001 */
void Cupd7907::LTAX_B(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( BC );
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 70 ba: 0111 0000 1011 1010 */
void Cupd7907::LTAX_D(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 70 bb: 0111 0000 1011 1011 */
void Cupd7907::LTAX_H(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 70 bc: 0111 0000 1011 1100 */
void Cupd7907::LTAX_Dp(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    DE++;
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 70 bd: 0111 0000 1011 1101 */
void Cupd7907::LTAX_Hp(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    HL++;
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 70 be: 0111 0000 1011 1110 */
void Cupd7907::LTAX_Dm(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    DE--;
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 70 bf: 0111 0000 1011 1111 */
void Cupd7907::LTAX_Hm(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    HL--;
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 70 c1: 0111 0000 1100 0001 */
void Cupd7907::ADDX_B(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( BC );
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 70 c2: 0111 0000 1100 0010 */
void Cupd7907::ADDX_D(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( DE );
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 70 c3: 0111 0000 1100 0011 */
void Cupd7907::ADDX_H(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( HL );
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 70 c4: 0111 0000 1100 0100 */
void Cupd7907::ADDX_Dp(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( DE );
    DE++;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 70 c5: 0111 0000 1100 0101 */
void Cupd7907::ADDX_Hp(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( HL );
    HL++;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 70 c6: 0111 0000 1100 0110 */
void Cupd7907::ADDX_Dm(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( DE );
    DE--;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 70 c7: 0111 0000 1100 0111 */
void Cupd7907::ADDX_Hm(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( HL );
    HL--;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 70 c9: 0111 0000 1100 1001 */
void Cupd7907::ONAX_B(upd7907_state *cpustate)
{
    if (A & RM( BC ))
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 70 ca: 0111 0000 1100 1010 */
void Cupd7907::ONAX_D(upd7907_state *cpustate)
{
    if (A & RM( DE ))
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 70 cb: 0111 0000 1100 1011 */
void Cupd7907::ONAX_H(upd7907_state *cpustate)
{
    if (A & RM( HL ))
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 70 cc: 0111 0000 1100 1100 */
void Cupd7907::ONAX_Dp(upd7907_state *cpustate)
{
    if (A & RM( DE ))
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
    DE++;
}

/* 70 cd: 0111 0000 1100 1101 */
void Cupd7907::ONAX_Hp(upd7907_state *cpustate)
{
    if (A & RM( HL ))
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
    HL++;
}

/* 70 ce: 0111 0000 1100 1110 */
void Cupd7907::ONAX_Dm(upd7907_state *cpustate)
{
    if (A & RM( DE ))
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
    DE--;
}

/* 70 cf: 0111 0000 1100 1111 */
void Cupd7907::ONAX_Hm(upd7907_state *cpustate)
{
    if (A & RM( HL ))
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
    HL--;
}

/* 70 d1: 0111 0000 1101 0001 */
void Cupd7907::ADCX_B(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( BC ) + (PSW & CY);
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 70 d2: 0111 0000 1101 0010 */
void Cupd7907::ADCX_D(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( DE ) + (PSW & CY);
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 70 d3: 0111 0000 1101 0011 */
void Cupd7907::ADCX_H(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( HL ) + (PSW & CY);
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 70 d4: 0111 0000 1101 0100 */
void Cupd7907::ADCX_Dp(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( DE ) + (PSW & CY);
    DE++;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 70 d5: 0111 0000 1101 0101 */
void Cupd7907::ADCX_Hp(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( HL ) + (PSW & CY);
    HL++;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 70 d6: 0111 0000 1101 0110 */
void Cupd7907::ADCX_Dm(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( DE ) + (PSW & CY);
    DE--;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 70 d7: 0111 0000 1101 0111 */
void Cupd7907::ADCX_Hm(upd7907_state *cpustate)
{
    UINT8 tmp = A + RM( HL ) + (PSW & CY);
    HL--;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 70 d9: 0111 0000 1101 1001 */
void Cupd7907::OFFAX_B(upd7907_state *cpustate)
{
    if ( A & RM( BC ) )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 70 da: 0111 0000 1101 1010 */
void Cupd7907::OFFAX_D(upd7907_state *cpustate)
{
    if ( A & RM( DE ) )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 70 db: 0111 0000 1101 1011 */
void Cupd7907::OFFAX_H(upd7907_state *cpustate)
{
    if ( A & RM( HL ) )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 70 dc: 0111 0000 1101 1100 */
void Cupd7907::OFFAX_Dp(upd7907_state *cpustate)
{
    if ( A & RM( DE ) )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
    DE++;
}

/* 70 dd: 0111 0000 1101 1101 */
void Cupd7907::OFFAX_Hp(upd7907_state *cpustate)
{
    if ( A & RM( HL ) )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
    HL++;
}

/* 70 de: 0111 0000 1101 1110 */
void Cupd7907::OFFAX_Dm(upd7907_state *cpustate)
{
    if ( A & RM( DE ) )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
    DE--;
}

/* 70 df: 0111 0000 1101 1111 */
void Cupd7907::OFFAX_Hm(upd7907_state *cpustate)
{
    if ( A & RM( HL ) )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
    HL--;
}

/* 70 e1: 0111 0000 1110 0001 */
void Cupd7907::SUBX_B(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( BC );
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
}

/* 70 e2: 0111 0000 1110 0010 */
void Cupd7907::SUBX_D(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
}

/* 70 e3: 0111 0000 1110 0011 */
void Cupd7907::SUBX_H(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
}

/* 70 e4: 0111 0000 1110 0100 */
void Cupd7907::SUBX_Dp(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    DE++;
}

/* 70 e5: 0111 0000 1110 0101 */
void Cupd7907::SUBX_Hp(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    HL++;
}

/* 70 e6: 0111 0000 1110 0110 */
void Cupd7907::SUBX_Dm(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    DE--;
}

/* 70 e7: 0111 0000 1110 0111 */
void Cupd7907::SUBX_Hm(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    HL--;
}

/* 70 e9: 0111 0000 1110 1001 */
void Cupd7907::NEAX_B(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( BC );
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 70 ea: 0111 0000 1110 1010 */
void Cupd7907::NEAX_D(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 70 eb: 0111 0000 1110 1011 */
void Cupd7907::NEAX_H(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 70 ec: 0111 0000 1110 1100 */
void Cupd7907::NEAX_Dp(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    DE++;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 70 ed: 0111 0000 1110 1101 */
void Cupd7907::NEAX_Hp(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    HL++;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 70 ee: 0111 0000 1110 1110 */
void Cupd7907::NEAX_Dm(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    DE--;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 70 ef: 0111 0000 1110 1111 */
void Cupd7907::NEAX_Hm(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    HL--;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 70 f1: 0111 0000 1111 0001 */
void Cupd7907::SBBX_B(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( BC ) - (PSW & CY);
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 70 f2: 0111 0000 1111 0010 */
void Cupd7907::SBBX_D(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE ) - (PSW & CY);
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 70 f3: 0111 0000 1111 0011 */
void Cupd7907::SBBX_H(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL ) - (PSW & CY);
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 70 f4: 0111 0000 1111 0100 */
void Cupd7907::SBBX_Dp(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE ) - (PSW & CY);
    DE++;
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 70 f5: 0111 0000 1111 0101 */
void Cupd7907::SBBX_Hp(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL ) - (PSW & CY);
    HL++;
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 70 f6: 0111 0000 1111 0110 */
void Cupd7907::SBBX_Dm(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE ) - (PSW & CY);
    DE--;
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 70 f7: 0111 0000 1111 0111 */
void Cupd7907::SBBX_Hm(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL ) - (PSW & CY);
    HL--;
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 70 f9: 0111 0000 1111 1001 */
void Cupd7907::EQAX_B(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( BC );
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 70 fa: 0111 0000 1111 1010 */
void Cupd7907::EQAX_D(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 70 fb: 0111 0000 1111 1011 */
void Cupd7907::EQAX_H(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 70 fc: 0111 0000 1111 1100 */
void Cupd7907::EQAX_Dp(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    DE++;
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 70 fd: 0111 0000 1111 1101 */
void Cupd7907::EQAX_Hp(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    HL++;
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 70 fe: 0111 0000 1111 1110 */
void Cupd7907::EQAX_Dm(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( DE );
    DE--;
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 70 ff: 0111 0000 1111 1111 */
void Cupd7907::EQAX_Hm(upd7907_state *cpustate)
{
    UINT8 tmp = A - RM( HL );
    HL--;
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* prefix 74 */
/* 74 08: 0111 0100 0000 1000 xxxx xxxx */
void Cupd7907::ANI_V_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    V &= imm;
    SET_Z(V);
}

/* 74 09: 0111 0100 0000 1001 xxxx xxxx */
void Cupd7907::ANI_A_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    A &= imm;
    SET_Z(A);
}

/* 74 0a: 0111 0100 0000 1010 xxxx xxxx */
void Cupd7907::ANI_B_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    B &= imm;
    SET_Z(B);
}

/* 74 0b: 0111 0100 0000 1011 xxxx xxxx */
void Cupd7907::ANI_C_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    C &= imm;
    SET_Z(C);
}

/* 74 0c: 0111 0100 0000 1100 xxxx xxxx */
void Cupd7907::ANI_D_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    D &= imm;
    SET_Z(D);
}

/* 74 0d: 0111 0100 0000 1101 xxxx xxxx */
void Cupd7907::ANI_E_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    E &= imm;
    SET_Z(E);
}

/* 74 0e: 0111 0100 0000 1110 xxxx xxxx */
void Cupd7907::ANI_H_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    H &= imm;
    SET_Z(H);
}

/* 74 0f: 0111 0100 0000 1111 xxxx xxxx */
void Cupd7907::ANI_L_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    L &= imm;
    SET_Z(L);
}

/* 74 10: 0111 0100 0001 0000 xxxx xxxx */
void Cupd7907::XRI_V_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    V ^= imm;
    SET_Z(V);
}

/* 74 11: 0111 0100 0001 0001 xxxx xxxx */
void Cupd7907::XRI_A_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    A ^= imm;
    SET_Z(A);
}

/* 74 12: 0111 0100 0001 0010 xxxx xxxx */
void Cupd7907::XRI_B_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    B ^= imm;
    SET_Z(B);
}

/* 74 13: 0111 0100 0001 0011 xxxx xxxx */
void Cupd7907::XRI_C_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    C ^= imm;
    SET_Z(C);
}

/* 74 14: 0111 0100 0001 0100 xxxx xxxx */
void Cupd7907::XRI_D_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    D ^= imm;
    SET_Z(D);
}

/* 74 15: 0111 0100 0001 0101 xxxx xxxx */
void Cupd7907::XRI_E_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    E ^= imm;
    SET_Z(E);
}

/* 74 16: 0111 0100 0001 0110 xxxx xxxx */
void Cupd7907::XRI_H_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    H ^= imm;
    SET_Z(H);
}

/* 74 17: 0111 0100 0001 0111 xxxx xxxx */
void Cupd7907::XRI_L_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    L ^= imm;
    SET_Z(L);
}

/* 74 18: 0111 0100 0001 1000 xxxx xxxx */
void Cupd7907::ORI_V_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    V |= imm;
    SET_Z(V);
}

/* 74 19: 0111 0100 0001 1001 xxxx xxxx */
void Cupd7907::ORI_A_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    A |= imm;
    SET_Z(A);
}

/* 74 1a: 0111 0100 0001 1010 xxxx xxxx */
void Cupd7907::ORI_B_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    B |= imm;
    SET_Z(B);
}

/* 74 1b: 0111 0100 0001 1011 xxxx xxxx */
void Cupd7907::ORI_C_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    C |= imm;
    SET_Z(C);
}

/* 74 1c: 0111 0100 0001 1100 xxxx xxxx */
void Cupd7907::ORI_D_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    D |= imm;
    SET_Z(D);
}

/* 74 1d: 0111 0100 0001 1101 xxxx xxxx */
void Cupd7907::ORI_E_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    E |= imm;
    SET_Z(E);
}

/* 74 1e: 0111 0100 0001 1110 xxxx xxxx */
void Cupd7907::ORI_H_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    H |= imm;
    SET_Z(H);
}

/* 74 1f: 0111 0100 0001 1111 xxxx xxxx */
void Cupd7907::ORI_L_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    L |= imm;
    SET_Z(L);
}

/* 74 20: 0111 0100 0010 0000 xxxx xxxx */
void Cupd7907::ADINC_V_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = V + imm;

    ZHC_ADD( tmp, V, 0 );
    V = tmp;
    SKIP_NC;
}

/* 74 21: 0111 0100 0010 0001 xxxx xxxx */
void Cupd7907::ADINC_A_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = A + imm;

    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 74 22: 0111 0100 0010 0010 xxxx xxxx */
void Cupd7907::ADINC_B_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = B + imm;

    ZHC_ADD( tmp, B, 0 );
    B = tmp;
    SKIP_NC;
}

/* 74 23: 0111 0100 0010 0011 xxxx xxxx */
void Cupd7907::ADINC_C_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = C + imm;

    ZHC_ADD( tmp, C, 0 );
    C = tmp;
    SKIP_NC;
}

/* 74 24: 0111 0100 0010 0100 xxxx xxxx */
void Cupd7907::ADINC_D_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = D + imm;

    ZHC_ADD( tmp, D, 0 );
    D = tmp;
    SKIP_NC;
}

/* 74 25: 0111 0100 0010 0101 xxxx xxxx */
void Cupd7907::ADINC_E_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = E + imm;

    ZHC_ADD( tmp, E, 0 );
    E = tmp;
    SKIP_NC;
}

/* 74 26: 0111 0100 0010 0110 xxxx xxxx */
void Cupd7907::ADINC_H_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = H + imm;

    ZHC_ADD( tmp, H, 0 );
    H = tmp;
    SKIP_NC;
}

/* 74 27: 0111 0100 0010 0111 xxxx xxxx */
void Cupd7907::ADINC_L_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = L + imm;

    ZHC_ADD( tmp, L, 0 );
    L = tmp;
    SKIP_NC;
}

/* 74 28: 0111 0100 0010 1000 xxxx xxxx */
void Cupd7907::GTI_V_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = V - imm - 1;
    ZHC_SUB( tmp, V, 0 );

    SKIP_NC;
}

/* 74 29: 0111 0100 0010 1001 xxxx xxxx */
void Cupd7907::GTI_A_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = A - imm - 1;
    ZHC_SUB( tmp, A, 0 );

    SKIP_NC;
}

/* 74 2a: 0111 0100 0010 1010 xxxx xxxx */
void Cupd7907::GTI_B_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = B - imm - 1;
    ZHC_SUB( tmp, B, 0 );

    SKIP_NC;
}

/* 74 2b: 0111 0100 0010 1011 xxxx xxxx */
void Cupd7907::GTI_C_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = C - imm - 1;
    ZHC_SUB( tmp, C, 0 );

    SKIP_NC;
}

/* 74 2c: 0111 0100 0010 1100 xxxx xxxx */
void Cupd7907::GTI_D_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = D - imm - 1;
    ZHC_SUB( tmp, D, 0 );

    SKIP_NC;
}

/* 74 2d: 0111 0100 0010 1101 xxxx xxxx */
void Cupd7907::GTI_E_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = E - imm - 1;
    ZHC_SUB( tmp, E, 0 );

    SKIP_NC;
}

/* 74 2e: 0111 0100 0010 1110 xxxx xxxx */
void Cupd7907::GTI_H_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = H - imm - 1;
    ZHC_SUB( tmp, H, 0 );

    SKIP_NC;
}

/* 74 2f: 0111 0100 0010 1111 xxxx xxxx */
void Cupd7907::GTI_L_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    UINT16 tmp;

    RDOPARG( imm );
    tmp = L - imm - 1;
    ZHC_SUB( tmp, L, 0 );

    SKIP_NC;
}

/* 74 30: 0111 0100 0011 0000 xxxx xxxx */
void Cupd7907::SUINB_V_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = V - imm;
    ZHC_SUB( tmp, V, 0 );
    V = tmp;
    SKIP_NC;
}

/* 74 31: 0111 0100 0011 0001 xxxx xxxx */
void Cupd7907::SUINB_A_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = A - imm;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 74 32: 0111 0100 0011 0010 xxxx xxxx */
void Cupd7907::SUINB_B_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = B - imm;
    ZHC_SUB( tmp, B, 0 );
    B = tmp;
    SKIP_NC;
}

/* 74 33: 0111 0100 0011 0011 xxxx xxxx */
void Cupd7907::SUINB_C_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = C - imm;
    ZHC_SUB( tmp, C, 0 );
    C = tmp;
    SKIP_NC;
}

/* 74 34: 0111 0100 0011 0100 xxxx xxxx */
void Cupd7907::SUINB_D_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = D - imm;
    ZHC_SUB( tmp, D, 0 );
    D = tmp;
    SKIP_NC;
}

/* 74 35: 0111 0100 0011 0101 xxxx xxxx */
void Cupd7907::SUINB_E_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = E - imm;
    ZHC_SUB( tmp, E, 0 );
    E = tmp;
    SKIP_NC;
}

/* 74 36: 0111 0100 0011 0110 xxxx xxxx */
void Cupd7907::SUINB_H_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = H - imm;
    ZHC_SUB( tmp, H, 0 );
    H = tmp;
    SKIP_NC;
}

/* 74 37: 0111 0100 0011 0111 xxxx xxxx */
void Cupd7907::SUINB_L_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = L - imm;
    ZHC_SUB( tmp, L, 0 );
    L = tmp;
    SKIP_NC;
}

/* 74 38: 0111 0100 0011 1000 xxxx xxxx */
void Cupd7907::LTI_V_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = V - imm;
    ZHC_SUB( tmp, V, 0 );
    SKIP_CY;
}

/* 74 39: 0111 0100 0011 1001 xxxx xxxx */
void Cupd7907::LTI_A_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = A - imm;
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 74 3a: 0111 0100 0011 1010 xxxx xxxx */
void Cupd7907::LTI_B_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = B - imm;
    ZHC_SUB( tmp, B, 0 );
    SKIP_CY;
}

/* 74 3b: 0111 0100 0011 1011 xxxx xxxx */
void Cupd7907::LTI_C_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = C - imm;
    ZHC_SUB( tmp, C, 0 );
    SKIP_CY;
}

/* 74 3c: 0111 0100 0011 1100 xxxx xxxx */
void Cupd7907::LTI_D_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = D - imm;
    ZHC_SUB( tmp, D, 0 );
    SKIP_CY;
}

/* 74 3d: 0111 0100 0011 1101 xxxx xxxx */
void Cupd7907::LTI_E_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = E - imm;
    ZHC_SUB( tmp, E, 0 );
    SKIP_CY;
}

/* 74 3e: 0111 0100 0011 1110 xxxx xxxx */
void Cupd7907::LTI_H_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = H - imm;
    ZHC_SUB( tmp, H, 0 );
    SKIP_CY;
}

/* 74 3f: 0111 0100 0011 1111 xxxx xxxx */
void Cupd7907::LTI_L_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = L - imm;
    ZHC_SUB( tmp, L, 0 );
    SKIP_CY;
}

/* 74 40: 0111 0100 0100 0000 xxxx xxxx */
void Cupd7907::ADI_V_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = V + imm;

    ZHC_ADD( tmp, V, 0 );
    V = tmp;
}

/* 74 41: 0111 0100 0100 0001 xxxx xxxx */
void Cupd7907::ADI_A_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = A + imm;

    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 74 42: 0111 0100 0100 0010 xxxx xxxx */
void Cupd7907::ADI_B_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = B + imm;

    ZHC_ADD( tmp, B, 0 );
    B = tmp;
}

/* 74 43: 0111 0100 0100 0011 xxxx xxxx */
void Cupd7907::ADI_C_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = C + imm;

    ZHC_ADD( tmp, C, 0 );
    C = tmp;
}

/* 74 44: 0111 0100 0100 0100 xxxx xxxx */
void Cupd7907::ADI_D_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = D + imm;

    ZHC_ADD( tmp, D, 0 );
    D = tmp;
}

/* 74 45: 0111 0100 0100 0101 xxxx xxxx */
void Cupd7907::ADI_E_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = E + imm;

    ZHC_ADD( tmp, E, 0 );
    E = tmp;
}

/* 74 46: 0111 0100 0100 0110 xxxx xxxx */
void Cupd7907::ADI_H_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = H + imm;

    ZHC_ADD( tmp, H, 0 );
    H = tmp;
}

/* 74 47: 0111 0100 0100 0111 xxxx xxxx */
void Cupd7907::ADI_L_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = L + imm;

    ZHC_ADD( tmp, L, 0 );
    L = tmp;
}

/* 74 48: 0111 0100 0100 1000 xxxx xxxx */
void Cupd7907::ONI_V_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (V & imm)
        PSW |= SK;
}

/* 74 49: 0111 0100 0100 1001 xxxx xxxx */
void Cupd7907::ONI_A_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (A & imm)
        PSW |= SK;
}

/* 74 4a: 0111 0100 0100 1010 xxxx xxxx */
void Cupd7907::ONI_B_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (B & imm)
        PSW |= SK;
}

/* 74 4b: 0111 0100 0100 1011 xxxx xxxx */
void Cupd7907::ONI_C_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (C & imm)
        PSW |= SK;
}

/* 74 4c: 0111 0100 0100 1100 xxxx xxxx */
void Cupd7907::ONI_D_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (D & imm)
        PSW |= SK;
}

/* 74 4d: 0111 0100 0100 1101 xxxx xxxx */
void Cupd7907::ONI_E_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (E & imm)
        PSW |= SK;
}

/* 74 4e: 0111 0100 0100 1110 xxxx xxxx */
void Cupd7907::ONI_H_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (H & imm)
        PSW |= SK;
}

/* 74 4f: 0111 0100 0100 1111 xxxx xxxx */
void Cupd7907::ONI_L_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (L & imm)
        PSW |= SK;
}

/* 74 50: 0111 0100 0101 0000 xxxx xxxx */
void Cupd7907::ACI_V_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = V + imm + (PSW & CY);
    ZHC_SUB( tmp, V, (PSW & CY) );
    V = tmp;
}

/* 74 51: 0111 0100 0101 0001 xxxx xxxx */
void Cupd7907::ACI_A_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = A + imm + (PSW & CY);
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 74 52: 0111 0100 0101 0010 xxxx xxxx */
void Cupd7907::ACI_B_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = B + imm + (PSW & CY);
    ZHC_SUB( tmp, B, (PSW & CY) );
    B = tmp;
}

/* 74 53: 0111 0100 0101 0011 xxxx xxxx */
void Cupd7907::ACI_C_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = C + imm + (PSW & CY);
    ZHC_SUB( tmp, C, (PSW & CY) );
    C = tmp;
}

/* 74 54: 0111 0100 0101 0100 xxxx xxxx */
void Cupd7907::ACI_D_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = D + imm + (PSW & CY);
    ZHC_SUB( tmp, D, (PSW & CY) );
    D = tmp;
}

/* 74 55: 0111 0100 0101 0101 xxxx xxxx */
void Cupd7907::ACI_E_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = E + imm + (PSW & CY);
    ZHC_SUB( tmp, E, (PSW & CY) );
    E = tmp;
}

/* 74 56: 0111 0100 0101 0110 xxxx xxxx */
void Cupd7907::ACI_H_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = H + imm + (PSW & CY);
    ZHC_SUB( tmp, H, (PSW & CY) );
    H = tmp;
}

/* 74 57: 0111 0100 0101 0111 xxxx xxxx */
void Cupd7907::ACI_L_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = L + imm + (PSW & CY);
    ZHC_SUB( tmp, L, (PSW & CY) );
    L = tmp;
}

/* 74 58: 0111 0100 0101 1000 xxxx xxxx */
void Cupd7907::OFFI_V_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (0 == (V & imm))
        PSW |= SK;
}

/* 74 59: 0111 0100 0101 1001 xxxx xxxx */
void Cupd7907::OFFI_A_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (0 == (A & imm))
        PSW |= SK;
}

/* 74 5a: 0111 0100 0101 1010 xxxx xxxx */
void Cupd7907::OFFI_B_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (0 == (B & imm))
        PSW |= SK;
}

/* 74 5b: 0111 0100 0101 1011 xxxx xxxx */
void Cupd7907::OFFI_C_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (0 == (C & imm))
        PSW |= SK;
}

/* 74 5c: 0111 0100 0101 1100 xxxx xxxx */
void Cupd7907::OFFI_D_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (0 == (D & imm))
        PSW |= SK;
}

/* 74 5d: 0111 0100 0101 1101 xxxx xxxx */
void Cupd7907::OFFI_E_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (0 == (E & imm))
        PSW |= SK;
}

/* 74 5e: 0111 0100 0101 1110 xxxx xxxx */
void Cupd7907::OFFI_H_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (0 == (H & imm))
        PSW |= SK;
}

/* 74 5f: 0111 0100 0101 1111 xxxx xxxx */
void Cupd7907::OFFI_L_xx(upd7907_state *cpustate)
{
    UINT8 imm;

    RDOPARG( imm );
    if (0 == (L & imm))
        PSW |= SK;
}

/* 74 60: 0111 0100 0110 0000 xxxx xxxx */
void Cupd7907::SUI_V_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = V - imm;
    ZHC_SUB( tmp, V, 0 );
    V = tmp;
}

/* 74 61: 0111 0100 0110 0001 xxxx xxxx */
void Cupd7907::SUI_A_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = A - imm;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
}

/* 74 62: 0111 0100 0110 0010 xxxx xxxx */
void Cupd7907::SUI_B_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = B - imm;
    ZHC_SUB( tmp, B, 0 );
    B = tmp;
}

/* 74 63: 0111 0100 0110 0011 xxxx xxxx */
void Cupd7907::SUI_C_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = C - imm;
    ZHC_SUB( tmp, C, 0 );
    C = tmp;
}

/* 74 64: 0111 0100 0110 0100 xxxx xxxx */
void Cupd7907::SUI_D_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = D - imm;
    ZHC_SUB( tmp, D, 0 );
    D = tmp;
}

/* 74 65: 0111 0100 0110 0101 xxxx xxxx */
void Cupd7907::SUI_E_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = E - imm;
    ZHC_SUB( tmp, E, 0 );
    E = tmp;
}

/* 74 66: 0111 0100 0110 0110 xxxx xxxx */
void Cupd7907::SUI_H_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = H - imm;
    ZHC_SUB( tmp, H, 0 );
    H = tmp;
}

/* 74 67: 0111 0100 0110 0111 xxxx xxxx */
void Cupd7907::SUI_L_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = L - imm;
    ZHC_SUB( tmp, L, 0 );
    L = tmp;
}

/* 74 68: 0111 0100 0110 1000 xxxx xxxx */
void Cupd7907::NEI_V_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = V - imm;
    ZHC_SUB( tmp, V, 0 );
    SKIP_NZ;
}

/* 74 69: 0111 0100 0110 1001 xxxx xxxx */
void Cupd7907::NEI_A_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = A - imm;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 74 6a: 0111 0100 0110 1010 xxxx xxxx */
void Cupd7907::NEI_B_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = B - imm;
    ZHC_SUB( tmp, B, 0 );
    SKIP_NZ;
}

/* 74 6b: 0111 0100 0110 1011 xxxx xxxx */
void Cupd7907::NEI_C_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = C - imm;
    ZHC_SUB( tmp, C, 0 );
    SKIP_NZ;
}

/* 74 6c: 0111 0100 0110 1100 xxxx xxxx */
void Cupd7907::NEI_D_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = D - imm;
    ZHC_SUB( tmp, D, 0 );
    SKIP_NZ;
}

/* 74 6d: 0111 0100 0110 1101 xxxx xxxx */
void Cupd7907::NEI_E_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = E - imm;
    ZHC_SUB( tmp, E, 0 );
    SKIP_NZ;
}

/* 74 6e: 0111 0100 0110 1110 xxxx xxxx */
void Cupd7907::NEI_H_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = H - imm;
    ZHC_SUB( tmp, H, 0 );
    SKIP_NZ;
}

/* 74 6f: 0111 0100 0110 1111 xxxx xxxx */
void Cupd7907::NEI_L_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = L - imm;
    ZHC_SUB( tmp, L, 0 );
    SKIP_NZ;
}

/* 74 70: 0111 0100 0111 0000 xxxx xxxx */
void Cupd7907::SBI_V_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = V - imm - (PSW & CY);
    ZHC_SUB( tmp, V, (PSW & CY) );
    V = tmp;
}

/* 74 71: 0111 0100 0111 0001 xxxx xxxx */
void Cupd7907::SBI_A_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = A - imm - (PSW & CY);
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 74 72: 0111 0100 0111 0010 xxxx xxxx */
void Cupd7907::SBI_B_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = B - imm - (PSW & CY);
    ZHC_SUB( tmp, B, (PSW & CY) );
    B = tmp;
}

/* 74 73: 0111 0100 0111 0011 xxxx xxxx */
void Cupd7907::SBI_C_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = C - imm - (PSW & CY);
    ZHC_SUB( tmp, C, (PSW & CY) );
    C = tmp;
}

/* 74 74: 0111 0100 0111 0100 xxxx xxxx */
void Cupd7907::SBI_D_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = D - imm - (PSW & CY);
    ZHC_SUB( tmp, D, (PSW & CY) );
    D = tmp;
}

/* 74 75: 0111 0100 0111 0101 xxxx xxxx */
void Cupd7907::SBI_E_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = E - imm - (PSW & CY);
    ZHC_SUB( tmp, E, (PSW & CY) );
    E = tmp;
}

/* 74 76: 0111 0100 0111 0110 xxxx xxxx */
void Cupd7907::SBI_H_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = H - imm - (PSW & CY);
    ZHC_SUB( tmp, H, (PSW & CY) );
    H = tmp;
}

/* 74 77: 0111 0100 0111 0111 xxxx xxxx */
void Cupd7907::SBI_L_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = L - imm - (PSW & CY);
    ZHC_SUB( tmp, L, (PSW & CY) );
    L = tmp;
}

/* 74 78: 0111 0100 0111 1000 xxxx xxxx */
void Cupd7907::EQI_V_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = V - imm;
    ZHC_SUB( tmp, V, 0 );
    SKIP_Z;
}

/* 74 79: 0111 0100 0111 1001 xxxx xxxx */
void Cupd7907::EQI_A_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = A - imm;
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 74 7a: 0111 0100 0111 1010 xxxx xxxx */
void Cupd7907::EQI_B_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = B - imm;
    ZHC_SUB( tmp, B, 0 );
    SKIP_Z;
}

/* 74 7b: 0111 0100 0111 1011 xxxx xxxx */
void Cupd7907::EQI_C_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = C - imm;
    ZHC_SUB( tmp, C, 0 );
    SKIP_Z;
}

/* 74 7c: 0111 0100 0111 1100 xxxx xxxx */
void Cupd7907::EQI_D_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = D - imm;
    ZHC_SUB( tmp, D, 0 );
    SKIP_Z;
}

/* 74 7d: 0111 0100 0111 1101 xxxx xxxx */
void Cupd7907::EQI_E_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = E - imm;
    ZHC_SUB( tmp, E, 0 );
    SKIP_Z;
}

/* 74 7e: 0111 0100 0111 1110 xxxx xxxx */
void Cupd7907::EQI_H_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = H - imm;
    ZHC_SUB( tmp, H, 0 );
    SKIP_Z;
}

/* 74 7f: 0111 0100 0111 1111 xxxx xxxx */
void Cupd7907::EQI_L_xx(upd7907_state *cpustate)
{
    UINT8 tmp, imm;

    RDOPARG( imm );
    tmp = L - imm;
    ZHC_SUB( tmp, L, 0 );
    SKIP_Z;
}

/* 74 88: 0111 0100 1000 1000 oooo oooo */
void Cupd7907::ANAW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    RDOPARG( ea.b.l );

    A &= RM( ea.d );
    SET_Z(A);
}

/* 74 8d: 0111 0100 1000 1101 */
void Cupd7907::DAN_EA_BC(upd7907_state *cpustate)
{
    EA &= BC;
    SET_Z(EA);
}

/* 74 8e: 0111 0100 1000 1110 */
void Cupd7907::DAN_EA_DE(upd7907_state *cpustate)
{
    EA &= DE;
    SET_Z(EA);
}

/* 74 8f: 0111 0100 1000 1111 */
void Cupd7907::DAN_EA_HL(upd7907_state *cpustate)
{
    EA &= HL;
    SET_Z(EA);
}

/* 74 90: 0111 0100 1001 0000 oooo oooo */
void Cupd7907::XRAW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    RDOPARG( ea.b.l );

    A ^= RM( ea.d );
    SET_Z(A);
}

/* 74 95: 0111 0100 1001 0101 */
void Cupd7907::DXR_EA_BC(upd7907_state *cpustate)
{
    EA ^= BC;
    SET_Z(EA);
}

/* 74 96: 0111 0100 1001 0110 */
void Cupd7907::DXR_EA_DE(upd7907_state *cpustate)
{
    EA ^= DE;
    SET_Z(EA);
}

/* 74 97: 0111 0100 1001 0111 */
void Cupd7907::DXR_EA_HL(upd7907_state *cpustate)
{
    EA ^= HL;
    SET_Z(EA);
}

/* 74 98: 0111 0100 1001 1000 oooo oooo */
void Cupd7907::ORAW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    RDOPARG( ea.b.l );

    A |= RM( ea.d );
    SET_Z(A);
}

/* 74 9d: 0111 0100 1001 1101 */
void Cupd7907::DOR_EA_BC(upd7907_state *cpustate)
{
    EA |= BC;
    SET_Z(EA);
}

/* 74 9e: 0111 0100 1001 1110 */
void Cupd7907::DOR_EA_DE(upd7907_state *cpustate)
{
    EA |= DE;
    SET_Z(EA);
}

/* 74 9f: 0111 0100 1001 1111 */
void Cupd7907::DOR_EA_HL(upd7907_state *cpustate)
{
    EA |= HL;
    SET_Z(EA);
}

/* 74 a0: 0111 0100 1010 0000 oooo oooo */
void Cupd7907::ADDNCW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 tmp;

    RDOPARG( ea.b.l );

    tmp = A + RM( ea.d );
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 74 a5: 0111 0100 1010 0101 */
void Cupd7907::DADDNC_EA_BC(upd7907_state *cpustate)
{
    UINT16 tmp = EA + BC;

    ZHC_ADD( tmp, EA, 0 );
    EA = tmp;
    SKIP_NC;
}

/* 74 a6: 0111 0100 1010 0110 */
void Cupd7907::DADDNC_EA_DE(upd7907_state *cpustate)
{
    UINT16 tmp = EA + DE;

    ZHC_ADD( tmp, EA, 0 );
    EA = tmp;
    SKIP_NC;
}

/* 74 a7: 0111 0100 1010 0111 */
void Cupd7907::DADDNC_EA_HL(upd7907_state *cpustate)
{
    UINT16 tmp = EA + HL;

    ZHC_ADD( tmp, EA, 0 );
    EA = tmp;
    SKIP_NC;
}

/* 74 a8: 0111 0100 1010 1000 oooo oooo */
void Cupd7907::GTAW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT16 tmp;

    RDOPARG( ea.b.l );
    tmp = A - RM( ea.d ) - 1;
    ZHC_SUB( tmp, A, 0 );
    SKIP_NC;
}

/* 74 ad: 0111 0100 1010 1101 */
void Cupd7907::DGT_EA_BC(upd7907_state *cpustate)
{
    UINT32 tmp = EA - BC - 1;
    ZHC_SUB( tmp, EA, 0 );
    SKIP_NC;
}

/* 74 ae: 0111 0100 1010 1110 */
void Cupd7907::DGT_EA_DE(upd7907_state *cpustate)
{
    UINT32 tmp = EA - DE - 1;
    ZHC_SUB( tmp, EA, 0 );
    SKIP_NC;
}

/* 74 af: 0111 0100 1010 1111 */
void Cupd7907::DGT_EA_HL(upd7907_state *cpustate)
{
    UINT32 tmp = EA - HL - 1;
    ZHC_SUB( tmp, EA, 0 );
    SKIP_NC;
}

/* 74 b0: 0111 0100 1011 0000 oooo oooo */
void Cupd7907::SUBNBW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 tmp;

    RDOPARG( ea.b.l );
    tmp = A - RM( ea.d );
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_NC;
}

/* 74 b5: 0111 0100 1011 0101 */
void Cupd7907::DSUBNB_EA_BC(upd7907_state *cpustate)
{
    UINT16 tmp = EA - BC;
    ZHC_SUB( tmp, EA, 0 );
    EA = tmp;
    SKIP_NC;
}

/* 74 b6: 0111 0100 1011 0110 */
void Cupd7907::DSUBNB_EA_DE(upd7907_state *cpustate)
{
    UINT16 tmp = EA - DE;
    ZHC_SUB( tmp, EA, 0 );
    EA = tmp;
    SKIP_NC;
}

/* 74 b7: 0111 0100 1011 0111 */
void Cupd7907::DSUBNB_EA_HL(upd7907_state *cpustate)
{
    UINT16 tmp;

    tmp = EA - HL;
    ZHC_SUB( tmp, EA, 0 );
    EA = tmp;
    SKIP_NC;
}

/* 74 b8: 0111 0100 1011 1000 oooo oooo */
void Cupd7907::LTAW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 tmp;

    RDOPARG( ea.b.l );
    tmp = A - RM( ea.d );
    ZHC_SUB( tmp, A, 0 );
    SKIP_CY;
}

/* 74 bd: 0111 0100 1011 1101 */
void Cupd7907::DLT_EA_BC(upd7907_state *cpustate)
{
    UINT16 tmp = EA - BC;
    ZHC_SUB( tmp, EA, 0 );
    SKIP_CY;
}

/* 74 be: 0111 0100 1011 1110 */
void Cupd7907::DLT_EA_DE(upd7907_state *cpustate)
{
    UINT16 tmp = EA - DE;
    ZHC_SUB( tmp, EA, 0 );
    SKIP_CY;
}

/* 74 bf: 0111 0100 1011 1111 */
void Cupd7907::DLT_EA_HL(upd7907_state *cpustate)
{
    UINT16 tmp = EA - HL;
    ZHC_SUB( tmp, EA, 0 );
    SKIP_CY;
}

/* 74 c0: 0111 0100 1100 0000 oooo oooo */
void Cupd7907::ADDW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 tmp;
    RDOPARG( ea.b.l );
    tmp = A + RM( ea.d );
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
}

/* 74 c5: 0111 0100 1100 0101 */
void Cupd7907::DADD_EA_BC(upd7907_state *cpustate)
{
    UINT16 tmp = EA + BC;
    ZHC_ADD( tmp, EA, 0 );
    EA = tmp;
}

/* 74 c6: 0111 0100 1100 0110 */
void Cupd7907::DADD_EA_DE(upd7907_state *cpustate)
{
    UINT16 tmp = EA + DE;
    ZHC_ADD( tmp, EA, 0 );
    EA = tmp;
}

/* 74 c7: 0111 0100 1100 0111 */
void Cupd7907::DADD_EA_HL(upd7907_state *cpustate)
{
    UINT16 tmp = EA + HL;
    ZHC_ADD( tmp, EA, 0 );
    EA = tmp;
}

/* 74 c8: 0111 0100 1100 1000 oooo oooo */
void Cupd7907::ONAW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    RDOPARG( ea.b.l );

    if (A & RM( ea.d ))
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 74 cd: 0111 0100 1100 1101 */
void Cupd7907::DON_EA_BC(upd7907_state *cpustate)
{
    if (EA & BC)
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 74 ce: 0111 0100 1100 1110 */
void Cupd7907::DON_EA_DE(upd7907_state *cpustate)
{
    if (EA & DE)
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 74 cf: 0111 0100 1100 1111 */
void Cupd7907::DON_EA_HL(upd7907_state *cpustate)
{
    if (EA & HL)
        PSW = (PSW & ~Z) | SK;
    else
        PSW |= Z;
}

/* 74 d0: 0111 0100 1101 0000 oooo oooo */
void Cupd7907::ADCW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 tmp;

    RDOPARG( ea.b.l );
    tmp = A + RM( ea.d ) + (PSW & CY);
    ZHC_ADD( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 74 d5: 0111 0100 1101 0101 */
void Cupd7907::DADC_EA_BC(upd7907_state *cpustate)
{
    UINT16 tmp = EA + BC + (PSW & CY);
    ZHC_ADD( tmp, EA, (PSW & CY) );
    EA = tmp;
}

/* 74 d6: 0111 0100 1101 0110 */
void Cupd7907::DADC_EA_DE(upd7907_state *cpustate)
{
    UINT16 tmp = EA + DE + (PSW & CY);
    ZHC_ADD( tmp, EA, (PSW & CY) );
    EA = tmp;
}

/* 74 d7: 0111 0100 1101 0111 */
void Cupd7907::DADC_EA_HL(upd7907_state *cpustate)
{
    UINT16 tmp = EA + HL + (PSW & CY);
    ZHC_ADD( tmp, EA, (PSW & CY) );
    EA = tmp;
}

/* 74 d8: 0111 0100 1101 1000 oooo oooo */
void Cupd7907::OFFAW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    RDOPARG( ea.b.l );

    if ( A & RM( ea.d ) )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 74 dd: 0111 0100 1101 1101 */
void Cupd7907::DOFF_EA_BC(upd7907_state *cpustate)
{
    if ( EA & BC )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 74 de: 0111 0100 1101 1110 */
void Cupd7907::DOFF_EA_DE(upd7907_state *cpustate)
{
    if ( EA & DE )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 74 df: 0111 0100 1101 1111 */
void Cupd7907::DOFF_EA_HL(upd7907_state *cpustate)
{
    if ( EA & HL )
        PSW &= ~Z;
    else
        PSW = PSW | Z | SK;
}

/* 74 e0: 0111 0100 1110 0000 oooo oooo */
void Cupd7907::SUBW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 tmp;

    RDOPARG( ea.b.l );
    tmp = A - RM( ea.d );
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
}

/* 74 e5: 0111 0100 1110 0101 */
void Cupd7907::DSUB_EA_BC(upd7907_state *cpustate)
{
    UINT16 tmp = EA - BC;
    ZHC_SUB( tmp, EA, 0 );
    EA = tmp;
}

/* 74 e6: 0111 0100 1110 0110 */
void Cupd7907::DSUB_EA_DE(upd7907_state *cpustate)
{
    UINT16 tmp = EA - DE;
    ZHC_SUB( tmp, EA, 0 );
    EA = tmp;
}

/* 74 e7: 0111 0100 1110 0111 */
void Cupd7907::DSUB_EA_HL(upd7907_state *cpustate)
{
    UINT16 tmp = EA - HL;
    ZHC_SUB( tmp, EA, 0 );
    EA = tmp;
}

/* 74 e8: 0111 0100 1110 1000 oooo oooo */
void Cupd7907::NEAW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 tmp;

    RDOPARG( ea.b.l );
    tmp = A - RM( ea.d );
    ZHC_SUB( tmp, A, 0 );
    SKIP_NZ;
}

/* 74 ed: 0111 0100 1110 1101 */
void Cupd7907::DNE_EA_BC(upd7907_state *cpustate)
{
    UINT16 tmp;

    tmp = EA - BC;
    ZHC_SUB( tmp, EA, 0 );
    SKIP_NZ;
}

/* 74 ee: 0111 0100 1110 1110 */
void Cupd7907::DNE_EA_DE(upd7907_state *cpustate)
{
    UINT16 tmp;

    tmp = EA - DE;
    ZHC_SUB( tmp, EA, 0 );
    SKIP_NZ;
}

/* 74 ef: 0111 0100 1110 1111 */
void Cupd7907::DNE_EA_HL(upd7907_state *cpustate)
{
    UINT16 tmp;

    tmp = EA - HL;
    ZHC_SUB( tmp, EA, 0 );
    SKIP_NZ;
}

/* 74 f0: 0111 0100 1111 0000 oooo oooo */
void Cupd7907::SBBW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 tmp;

    RDOPARG( ea.b.l );
    tmp = A - RM( ea.d ) - (PSW & CY);
    ZHC_SUB( tmp, A, (PSW & CY) );
    A = tmp;
}

/* 74 f5: 0111 0100 1111 0101 */
void Cupd7907::DSBB_EA_BC(upd7907_state *cpustate)
{
    UINT16 tmp = EA - BC - (PSW & CY);
    ZHC_SUB( tmp, EA, (PSW & CY) );
    EA = tmp;
}

/* 74 f6: 0111 0100 1111 0110 */
void Cupd7907::DSBB_EA_DE(upd7907_state *cpustate)
{
    UINT16 tmp = EA - DE - (PSW & CY);
    ZHC_SUB( tmp, EA, (PSW & CY) );
    EA = tmp;
}

/* 74 f7: 0111 0100 1111 0111 */
void Cupd7907::DSBB_EA_HL(upd7907_state *cpustate)
{
    UINT16 tmp = EA - HL - (PSW & CY);
    ZHC_SUB( tmp, EA, (PSW & CY) );
    EA = tmp;
}

/* 74 f8: 0111 0100 1111 1000 oooo oooo */
void Cupd7907::EQAW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 tmp;

    RDOPARG( ea.b.l );
    tmp = A - RM( ea.d );
    ZHC_SUB( tmp, A, 0 );
    SKIP_Z;
}

/* 74 fd: 0111 0100 1111 1101 */
void Cupd7907::DEQ_EA_BC(upd7907_state *cpustate)
{
    UINT16 tmp;

    tmp = EA - BC;
    ZHC_SUB( tmp, EA, 0 );
    SKIP_Z;
}

/* 74 fe: 0111 0100 1111 1110 */
void Cupd7907::DEQ_EA_DE(upd7907_state *cpustate)
{
    UINT16 tmp;

    tmp = EA - DE;
    ZHC_SUB( tmp, EA, 0 );
    SKIP_Z;
}

/* 74 ff: 0111 0100 1111 1111 */
void Cupd7907::DEQ_EA_HL(upd7907_state *cpustate)
{
    UINT16 tmp;

    tmp = EA - HL;
    ZHC_SUB( tmp, EA, 0 );
    SKIP_Z;
}

/************************************************
 * main opcodes
 ************************************************/

/* 00: 0000 0000 */
void Cupd7907::NOP(upd7907_state *cpustate)
{
}

/* 01: 0000 0001 oooo oooo */
void Cupd7907::LDAW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;

    RDOPARG( ea.b.l );

    A = RM( ea.d );
}

/* 02: 0000 0010 */
void Cupd7907::INX_SP(upd7907_state *cpustate)
{
    SP++;
}

/* 03: 0000 0011 */
void Cupd7907::DCX_SP(upd7907_state *cpustate)
{
    SP--;
}

/* 04: 0000 0100 llll llll hhhh hhhh */
void Cupd7907::LXI_S_w(upd7907_state *cpustate)
{
    RDOPARG( SPL );
    RDOPARG( SPH );
}

/* 05: 0000 0101 oooo oooo xxxx xxxx */
void Cupd7907::ANIW_wa_xx(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 m, imm;

    RDOPARG( ea.b.l );
    RDOPARG( imm );
    m = RM( ea.d );
    m &= imm;
    WM( ea.d, m );
    SET_Z(m);
}

/* 07: 0000 0111 xxxx xxxx */
/* ANI_A_xx already defined (long form) */

/* 08: 0000 1000 */
void Cupd7907::MOV_A_EAH(upd7907_state *cpustate)
{
    A = EAH;
}

/* 09: 0000 1001 */
void Cupd7907::MOV_A_EAL(upd7907_state *cpustate)
{
    A = EAL;
}

/* 0a: 0000 1010 */
void Cupd7907::MOV_A_B(upd7907_state *cpustate)
{
    A = B;
}

/* 0b: 0000 1011 */
void Cupd7907::MOV_A_C(upd7907_state *cpustate)
{
    A = C;
}

/* 0c: 0000 1100 */
void Cupd7907::MOV_A_D(upd7907_state *cpustate)
{
    A = D;
}

/* 0d: 0000 1101 */
void Cupd7907::MOV_A_E(upd7907_state *cpustate)
{
    A = E;
}

/* 0e: 0000 1110 */
void Cupd7907::MOV_A_H(upd7907_state *cpustate)
{
    A = H;
}

/* 0f: 0000 1111 */
void Cupd7907::MOV_A_L(upd7907_state *cpustate)
{
    A = L;
}

/* 10: 0001 0000 */
void Cupd7907::EXA(upd7907_state *cpustate)
{
    UINT16 tmp;
    tmp = EA; EA = EA2; EA2 = tmp;
    tmp = VA; VA = VA2; VA2 = tmp;
}

/* 11: 0001 0001 */
void Cupd7907::EXX(upd7907_state *cpustate)
{
    UINT16 tmp;
    tmp = BC; BC = BC2; BC2 = tmp;
    tmp = DE; DE = DE2; DE2 = tmp;
    tmp = HL; HL = HL2; HL2 = tmp;
}

/* 48 AD (7807 only) */
void Cupd7907::EXR(upd7907_state *cpustate)
{
    UINT16 tmp;
    tmp = BC; BC = BC2; BC2 = tmp;
    tmp = DE; DE = DE2; DE2 = tmp;
    tmp = HL; HL = HL2; HL2 = tmp;
    tmp = EA; EA = EA2; EA2 = tmp;
    tmp = VA; VA = VA2; VA2 = tmp;
}

/* 12: 0001 0010 */
void Cupd7907::INX_BC(upd7907_state *cpustate)
{
    BC++;
}

/* 13: 0001 0011 */
void Cupd7907::DCX_BC(upd7907_state *cpustate)
{
    BC--;
}

/* 14: 0001 0100 llll llll hhhh hhhh */
void Cupd7907::LXI_B_w(upd7907_state *cpustate)
{
    RDOPARG( C );
    RDOPARG( B );
}

/* 15: 0001 0101 oooo oooo xxxx xxxx */
void Cupd7907::ORIW_wa_xx(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 m, imm;

    RDOPARG( ea.b.l );
    RDOPARG( imm );
    m = RM( ea.d );
    m |= imm;
    WM( ea.d, m );
    SET_Z(m);
}

/* 16: 0001 0110 xxxx xxxx */
/* XRI_A_xx already defined (long form) */

/* 17: 0001 0111 xxxx xxxx */
/* ORI_A_xx already defined (long form) */

/* 18: 0001 1000 */
void Cupd7907::MOV_EAH_A(upd7907_state *cpustate)
{
    EAH = A;
}

/* 19: 0001 1001 */
void Cupd7907::MOV_EAL_A(upd7907_state *cpustate)
{
    EAL = A;
}

/* 1a: 0001 1010 */
void Cupd7907::MOV_B_A(upd7907_state *cpustate)
{
    B = A;
}

/* 1b: 0001 1011 */
void Cupd7907::MOV_C_A(upd7907_state *cpustate)
{
    C = A;
}

/* 1c: 0001 1100 */
void Cupd7907::MOV_D_A(upd7907_state *cpustate)
{
    D = A;
}

/* 1d: 0001 1101 */
void Cupd7907::MOV_E_A(upd7907_state *cpustate)
{
    E = A;
}

/* 1e: 0001 1110 */
void Cupd7907::MOV_H_A(upd7907_state *cpustate)
{
    H = A;
}

/* 1f: 0001 1111 */
void Cupd7907::MOV_L_A(upd7907_state *cpustate)
{
    L = A;
}

/* 20: 0010 0000 oooo oooo */
void Cupd7907::INRW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 tmp, m;

    RDOPARG( ea.b.l );
    m = RM( ea.d );
    tmp = m + 1;
    ZHC_ADD( tmp, m, 0 );
    WM( ea.d, tmp );
    SKIP_CY;
}

/* 21: 0010 0001 */
void Cupd7907::JB(upd7907_state *cpustate)
{
    PC = BC;
}

/* 22: 0010 0010 */
void Cupd7907::INX_DE(upd7907_state *cpustate)
{
    DE++;
}

/* 23: 0010 0011 */
void Cupd7907::DCX_DE(upd7907_state *cpustate)
{
    DE--;
}

/* 24: 0010 0100 llll llll hhhh hhhh */
void Cupd7907::LXI_D_w(upd7907_state *cpustate)
{
    RDOPARG( E );
    RDOPARG( D );
}

/* 25: 0010 0101 oooo oooo xxxx xxxx */
void Cupd7907::GTIW_wa_xx(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 m, imm;
    UINT16 tmp;

    RDOPARG( ea.b.l );
    RDOPARG( imm );
    m = RM( ea.d );
    tmp = m - imm - 1;
    ZHC_SUB( tmp, m, 0 );
    SKIP_NC;
}

/* 26: 0010 0110 xxxx xxxx */
/* ADINC_A_xx already defined (long form) */

/* 27: 0010 0111 xxxx xxxx */
/* GTI_A_xx already defined (long form) */

/* 29: 0010 1001 */
void Cupd7907::LDAX_B(upd7907_state *cpustate)
{
    A = RM( BC );
}

/* 2a: 0010 1010 */
void Cupd7907::LDAX_D(upd7907_state *cpustate)
{
    A = RM( DE );
}

/* 2b: 0010 1011 */
void Cupd7907::LDAX_H(upd7907_state *cpustate)
{
    A = RM( HL );
}

/* 2c: 0010 1100 */
void Cupd7907::LDAX_Dp(upd7907_state *cpustate)
{
    A = RM( DE );
    DE++;
}

/* 2d: 0010 1101 dddd dddd */
void Cupd7907::LDAX_Hp(upd7907_state *cpustate)
{
    A = RM( HL );
    HL++;
}

/* 2e: 0010 1110 dddd dddd */
void Cupd7907::LDAX_Dm(upd7907_state *cpustate)
{
    A = RM( DE );
    DE--;
}

/* 2f: 0010 1111 dddd dddd */
void Cupd7907::LDAX_Hm(upd7907_state *cpustate)
{
    A = RM( HL );
    HL--;
}

/* 30: 0011 0000 oooo oooo */
void Cupd7907::DCRW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 tmp, m;

    RDOPARG( ea.b.l );
    m = RM( ea.d );
    tmp = m - 1;
    ZHC_SUB( tmp, m, 0 );
    WM( ea.d, tmp );
    SKIP_CY;
}

/* 31: 0011 0001 */
void Cupd7907::BLOCK(upd7907_state *cpustate)
{
    WM( DE, RM( HL ) );
    DE++;
    HL++;
    C--;
    if (C == 0xff)
        PSW |= CY;
    else
    {
        PSW &= ~CY;
        PC--;
    }
}

/* 32: 0011 0010 */
void Cupd7907::INX_HL(upd7907_state *cpustate)
{
    HL++;
}

/* 33: 0011 0011 */
void Cupd7907::DCX_HL(upd7907_state *cpustate)
{
    HL--;
}

/* 34: 0011 0100 llll llll hhhh hhhh */
void Cupd7907::LXI_H_w(upd7907_state *cpustate)
{
    if (PSW & L0) { /* overlay active? */
        PC+=2;
        return;
    }
    RDOPARG( L );
    RDOPARG( H );
    PSW |= L0;
}

/* 35: 0011 0101 oooo oooo xxxx xxxx */
void Cupd7907::LTIW_wa_xx(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 tmp, m, imm;

    RDOPARG( ea.b.l );
    RDOPARG( imm );
    m = RM( ea.d );
    tmp = m - imm;
    ZHC_SUB( tmp, m, 0 );
    SKIP_CY;
}

/* 36: 0011 0110 xxxx xxxx */
/* SUINB_A_xx already defined (long form) */

/* 37: 0011 0111 xxxx xxxx */
/* LTI_A_xx already defined (long form) */

/* 39: 0011 1001 */
void Cupd7907::STAX_B(upd7907_state *cpustate)
{
    WM( BC, A );
}

/* 3a: 0011 1010 */
void Cupd7907::STAX_D(upd7907_state *cpustate)
{
    WM( DE, A );
}

/* 3b: 0011 1011 */
void Cupd7907::STAX_H(upd7907_state *cpustate)
{
    WM( HL, A );
}

/* 3c: 0011 1100 */
void Cupd7907::STAX_Dp(upd7907_state *cpustate)
{
    WM( DE, A );
    DE++;
}

/* 3d: 0011 1101 */
void Cupd7907::STAX_Hp(upd7907_state *cpustate)
{
    WM( HL, A );
    HL++;
}

/* 3e: 0011 1110 */
void Cupd7907::STAX_Dm(upd7907_state *cpustate)
{
    WM( DE, A );
    DE--;
}

/* 3f: 0011 1111 */
void Cupd7907::STAX_Hm(upd7907_state *cpustate)
{
    WM( HL, A );
    HL--;
}

/* 40: 0100 0000 llll llll hhhh hhhh */
void Cupd7907::CALL_w(upd7907_state *cpustate)
{
    PAIR w;
    w.d = 0;

    RDOPARG( w.b.l );
    RDOPARG( w.b.h );

    SP--;
    WM( SPD, PCH );
    SP--;
    WM( SPD, PCL );

    PC = w.w.l;
    cpustate->pPC->pCPU->CallSubLevel++;
}

/* 41: 0100 0001 */
void Cupd7907::INR_A(upd7907_state *cpustate)
{
    UINT8 tmp = A + 1;
    ZHC_ADD( tmp, A, 0 );
    A = tmp;
    SKIP_CY;
}

/* 42: 0100 0010 */
void Cupd7907::INR_B(upd7907_state *cpustate)
{
    UINT8 tmp = B + 1;
    ZHC_ADD( tmp, B, 0 );
    B = tmp;
    SKIP_CY;
}

/* 43: 0100 0011 */
void Cupd7907::INR_C(upd7907_state *cpustate)
{
    UINT8 tmp = C + 1;
    ZHC_ADD( tmp, C, 0 );
    C = tmp;
    SKIP_CY;
}

/* 44: 0100 0100 llll llll hhhh hhhh */
void Cupd7907::LXI_EA_s(upd7907_state *cpustate)
{
    RDOPARG( EAL );
    RDOPARG( EAH );
}

/* 45: 0100 0101 oooo oooo xxxx xxxx */
void Cupd7907::ONIW_wa_xx(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 imm;

    RDOPARG( ea.b.l );
    RDOPARG( imm );

    if (RM( ea.d ) & imm)
        PSW |= SK;
}

/* 46: 0100 0110 xxxx xxxx */
/* ADI_A_xx already defined (long form) */

/* 47: 0100 0111 xxxx xxxx */
/* ONI_A_xx already defined (long form) */

/* 48: prefix */
void Cupd7907::PRE_48(upd7907_state *cpustate)
{
    RDOP(OP2);
    cpustate->icount -= cpustate->op48[OP2].cycles;
    cpustate->handle_timers(cpustate, cpustate->op48[OP2].cycles);
    (*cpustate->op48[OP2].opfunc)(cpustate);
}

/* 49: 0100 1001 xxxx xxxx */
void Cupd7907::MVIX_BC_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    WM( BC, imm );
}

/* 4a: 0100 1010 xxxx xxxx */
void Cupd7907::MVIX_DE_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    WM( DE, imm );
}

/* 4b: 0100 1011 xxxx xxxx */
void Cupd7907::MVIX_HL_xx(upd7907_state *cpustate)
{
    UINT8 imm;
    RDOPARG( imm );
    WM( HL, imm );
}

/* 4c: prefix */
void Cupd7907::PRE_4C(upd7907_state *cpustate)
{
    RDOP(OP2);
    cpustate->icount -= cpustate->op4C[OP2].cycles;
    cpustate->handle_timers(cpustate, cpustate->op4C[OP2].cycles);
    (*cpustate->op4C[OP2].opfunc)(cpustate);
}

/* 4d: prefix */
void Cupd7907::PRE_4D(upd7907_state *cpustate)
{
    RDOP(OP2);
    cpustate->icount -= cpustate->op4D[OP2].cycles;
    cpustate->handle_timers(cpustate, cpustate->op4D[OP2].cycles);
    (*cpustate->op4D[OP2].opfunc)(cpustate);
}

/* 4e: 0100 111d dddd dddd */
void Cupd7907::JRE(upd7907_state *cpustate)
{
    UINT8 offs;
    RDOPARG( offs );
    if (OP & 0x01)
        PC -= 256 - offs;
    else
        PC += offs;
}

/* 50: 0101 0000 */
void Cupd7907::EXH(upd7907_state *cpustate)
{
    UINT16 tmp;
    tmp = HL; HL = HL2; HL2 = tmp;
}

/* 51: 0101 0001 */
void Cupd7907::DCR_A(upd7907_state *cpustate)
{
    UINT8 tmp = A - 1;
    ZHC_SUB( tmp, A, 0 );
    A = tmp;
    SKIP_CY;
}

/* 52: 0101 0010 */
void Cupd7907::DCR_B(upd7907_state *cpustate)
{
    UINT8 tmp = B - 1;
    ZHC_SUB( tmp, B, 0 );
    B = tmp;
    SKIP_CY;
}

/* 53: 0101 0011 */
void Cupd7907::DCR_C(upd7907_state *cpustate)
{
    UINT8 tmp = C - 1;
    ZHC_SUB( tmp, C, 0 );
    C = tmp;
    SKIP_CY;
}

/* 54: 0101 0100 llll llll hhhh hhhh */
void Cupd7907::JMP_w(upd7907_state *cpustate)
{
    PAIR w;
    w.d = 0;

    RDOPARG( w.b.l );
    RDOPARG( w.b.h );

    PCD = w.d;
}

/* 55: 0101 0101 oooo oooo xxxx xxxx */
void Cupd7907::OFFIW_wa_xx(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 imm;

    RDOPARG( ea.b.l );
    RDOPARG( imm );

    if (0 == (RM( ea.d ) & imm))
        PSW |= SK;
}

/* 56: 0101 0110 xxxx xxxx */
/* ACI_A_xx already defined (long form) */

/* 57: 0101 0111 xxxx xxxx */
/* OFFI_A_xx already defined (long form) */

/* 58: 0101 1000 oooo oooo (7810 only) */
void Cupd7907::BIT_0_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;

    RDOPARG( ea.b.l );

    if (RM( ea.d ) & 0x01)
        PSW |= SK;
}

/* 59: 0101 1001 oooo oooo (7810 only) */
void Cupd7907::BIT_1_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;

    RDOPARG( ea.b.l );

    if (RM( ea.d ) & 0x02)
        PSW |= SK;
}

/* 5a: 0101 1010 oooo oooo (7810 only) */
void Cupd7907::BIT_2_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;

    RDOPARG( ea.b.l );

    if (RM( ea.d ) & 0x04)
        PSW |= SK;
}

/* 5b: 0101 1011 oooo oooo (7810 only) */
void Cupd7907::BIT_3_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;

    RDOPARG( ea.b.l );

    if (RM( ea.d ) & 0x08)
        PSW |= SK;
}

/* 5c: 0101 1100 oooo oooo (7810 only) */
void Cupd7907::BIT_4_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;

    RDOPARG( ea.b.l );

    if (RM( ea.d ) & 0x10)
        PSW |= SK;
}

/* 5d: 0101 1101 oooo oooo (7810 only) */
void Cupd7907::BIT_5_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;

    RDOPARG( ea.b.l );

    if (RM( ea.d ) & 0x20)
        PSW |= SK;
}

/* 5e: 0101 1110 oooo oooo (7810 only) */
void Cupd7907::BIT_6_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;

    RDOPARG( ea.b.l );

    if (RM( ea.d ) & 0x40)
        PSW |= SK;
}

/* 5f: 0101 1111 oooo oooo (7810 only) */
void Cupd7907::BIT_7_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;

    RDOPARG( ea.b.l );

    if (RM( ea.d ) & 0x80)
        PSW |= SK;
}

/* 5d: 0101 1111 bbbb bbbb (7807 only) */
void Cupd7907::SKN_bit(upd7907_state *cpustate)
{
    UINT8 imm;
    int val;

    RDOPARG( imm );

    switch( imm & 0x1f )
    {
        case 0x10:	/* PA */
            val = RP( cpustate, UPD7907_PORTA );
            break;
        case 0x11:	/* PB */
            val = RP( cpustate, UPD7907_PORTB );
            break;
        case 0x12:	/* PC */
            val = RP( cpustate, UPD7907_PORTC );
            break;
        case 0x13:	/* PD */
            val = RP( cpustate, UPD7907_PORTD );
            break;
        case 0x15:	/* PF */
            val = RP( cpustate, UPD7907_PORTF );
            break;
        case 0x16:	/* MKH */
            val = MKH;
            break;
        case 0x17:	/* MKL */
            val = MKL;
            break;
        case 0x19:	/* SMH */
            val = SMH;
            break;
        case 0x1b:	/* EOM */
            val = EOM;
            break;
        case 0x1d:	/* TMM */
            val = TMM;
            break;
        case 0x1e:	/* PT */
            val = RP( cpustate, UPD7907_PORTT );
            break;
        default:
//            logerror("uPD7810 '%s': illegal opcode %02x %02x at PC:%04x\n", cpustate->device->tag(), OP, imm, PC);
            val = 0;
            break;
    }

    if (~val & (1 << (imm >> 5)))
        PSW |= SK;
}

/* 58: 0101 1000 bbbb bbbb (7807 only) */
void Cupd7907::SETB(upd7907_state *cpustate)
{
    UINT8 imm;
    int bit;

    RDOPARG( imm );
    bit = imm >> 5;

    switch( imm & 0x1f )
    {
        case 0x10:	/* PA */
            WP( cpustate, UPD7907_PORTA, RP( cpustate, UPD7907_PORTA ) | (1 << bit));
            break;
        case 0x11:	/* PB */
            WP( cpustate, UPD7907_PORTB, RP( cpustate, UPD7907_PORTB ) | (1 << bit));
            break;
        case 0x12:	/* PC */
            WP( cpustate, UPD7907_PORTC, RP( cpustate, UPD7907_PORTC ) | (1 << bit));
            break;
        case 0x13:	/* PD */
            WP( cpustate, UPD7907_PORTD, RP( cpustate, UPD7907_PORTD ) | (1 << bit));
            break;
        case 0x15:	/* PF */
            WP( cpustate, UPD7907_PORTF, RP( cpustate, UPD7907_PORTF ) | (1 << bit));
            break;
        case 0x16:	/* MKH */
            MKH |= (1 << bit);
            break;
        case 0x17:	/* MKL */
            MKL |= (1 << bit);
            break;
        case 0x19:	/* SMH */
            SMH |= (1 << bit);
            break;
        case 0x1b:	/* EOM */
            EOM |= (1 << bit);
            break;
        case 0x1d:	/* TMM */
            TMM |= (1 << bit);
            break;
//      case 0x1e:  /* PT */
//          PT is input only
//          break;
        default:
//            logerror("uPD7810 '%s': illegal opcode %02x %02x at PC:%04x\n", cpustate->device->tag(), OP, imm, PC);
            break;
    }
}

/* 5b: 0101 1011 bbbb bbbb (7807 only) */
void Cupd7907::CLR(upd7907_state *cpustate)
{
    UINT8 imm;
    int bit;

    RDOPARG( imm );
    bit = imm >> 5;

    switch( imm & 0x1f )
    {
        case 0x10:	/* PA */
            WP( cpustate, UPD7907_PORTA, RP( cpustate, UPD7907_PORTA ) & ~(1 << bit));
            break;
        case 0x11:	/* PB */
            WP( cpustate, UPD7907_PORTB, RP( cpustate, UPD7907_PORTB ) & ~(1 << bit));
            break;
        case 0x12:	/* PC */
            WP( cpustate, UPD7907_PORTC, RP( cpustate, UPD7907_PORTC ) & ~(1 << bit));
            break;
        case 0x13:	/* PD */
            WP( cpustate, UPD7907_PORTD, RP( cpustate, UPD7907_PORTD ) & ~(1 << bit));
            break;
        case 0x15:	/* PF */
            WP( cpustate, UPD7907_PORTF, RP( cpustate, UPD7907_PORTF ) & ~(1 << bit));
            break;
        case 0x16:	/* MKH */
            MKH &= ~(1 << bit);
            break;
        case 0x17:	/* MKL */
            MKL &= ~(1 << bit);
            break;
        case 0x19:	/* SMH */
            SMH &= ~(1 << bit);
            break;
        case 0x1b:	/* EOM */
            EOM &= ~(1 << bit);
            break;
        case 0x1d:	/* TMM */
            TMM &= ~(1 << bit);
            break;
//      case 0x1e:  /* PT */
//          PT is input only
//          break;
        default:
//            logerror("uPD7810 '%s': illegal opcode %02x %02x at PC:%04x\n", cpustate->device->tag(), OP, imm, PC);
            break;
    }
}

/* 5d: 0101 1111 bbbb bbbb (7807 only) */
void Cupd7907::SK_bit(upd7907_state *cpustate)
{
    UINT8 imm;
    int val;

    RDOPARG( imm );

    switch( imm & 0x1f )
    {
        case 0x10:	/* PA */
            val = RP( cpustate, UPD7907_PORTA );
            break;
        case 0x11:	/* PB */
            val = RP( cpustate, UPD7907_PORTB );
            break;
        case 0x12:	/* PC */
            val = RP( cpustate, UPD7907_PORTC );
            break;
        case 0x13:	/* PD */
            val = RP( cpustate, UPD7907_PORTD );
            break;
        case 0x15:	/* PF */
            val = RP( cpustate, UPD7907_PORTF );
            break;
        case 0x16:	/* MKH */
            val = MKH;
            break;
        case 0x17:	/* MKL */
            val = MKL;
            break;
        case 0x19:	/* SMH */
            val = SMH;
            break;
        case 0x1b:	/* EOM */
            val = EOM;
            break;
        case 0x1d:	/* TMM */
            val = TMM;
            break;
        case 0x1e:	/* PT */
            val = RP( cpustate, UPD7907_PORTT );
            break;
        default:
//            logerror("uPD7810 '%s': illegal opcode %02x %02x at PC:%04x\n", cpustate->device->tag(), OP, imm, PC);
            val = 0;
            break;
    }

    if (val & (1 << (imm >> 5)))
        PSW |= SK;
}

/* 60:*/
void Cupd7907::PRE_60(upd7907_state *cpustate)
{
    RDOP(OP2);
    cpustate->icount -= cpustate->op60[OP2].cycles;
    cpustate->handle_timers(cpustate, cpustate->op60[OP2].cycles);
    (*cpustate->op60[OP2].opfunc)(cpustate);
}

/* 61: 0110 0001 */
void Cupd7907::DAA(upd7907_state *cpustate)
{
    UINT8 l = A & 0x0f, h = A >> 4, tmp, adj = 0x00;
    if (0 == (PSW & HC))
    {
        if (l < 10)
        {
            if (!(h < 10 && 0 == (PSW & CY)))
            adj = 0x60;
        }
        else
        {
            if (h < 9 && 0 == (PSW & CY))
                adj = 0x06;
            else
                adj = 0x66;
        }
    }
    else
    if (l < 3)
    {
        if (h < 10 && 0 == (PSW & CY))
            adj = 0x06;
        else
            adj = 0x66;
    }
    tmp = A + adj;
    ZHC_ADD( tmp, A, PSW & CY );
    A = tmp;
}

/* 62: 0110 0010 */
void Cupd7907::RETI(upd7907_state *cpustate)
{
    if (((Cupd7907*)(cpustate->pPC->pCPU))->softi) {
        IFF=1;
        ((Cupd7907*)(cpustate->pPC->pCPU))->softi = false;
    }
    PCL = RM( SPD );
    SP++;
    PCH = RM( SPD );
    SP++;
    PSW = RM( SPD );
    SP++;
    cpustate->pPC->pCPU->CallSubLevel--;
}

/* 63: 0110 0011 oooo oooo */
void Cupd7907::STAW_wa(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;

    RDOPARG( ea.b.l );

    WM( ea.d, A );
}

/* 64: prefix */
void Cupd7907::PRE_64(upd7907_state *cpustate)
{
    RDOP(OP2);
    cpustate->icount -= cpustate->op64[OP2].cycles;
    cpustate->handle_timers(cpustate, cpustate->op64[OP2].cycles);
    (*cpustate->op64[OP2].opfunc)(cpustate);
}

/* 65: 0110 0101 oooo oooo xxxx xxxx */
void Cupd7907::NEIW_wa_xx(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 tmp, m, imm;

    RDOPARG( ea.b.l );
    RDOPARG( imm );
    m = RM( ea.d );
    tmp = m - imm;
    ZHC_SUB( tmp, m, 0 );
    SKIP_NZ;
}

/* 66: 0110 0110 xxxx xxxx */
/* SUI_A_xx already defined (long form) */

/* 67: 0110 0111 xxxx xxxx */
/* NEI_A_xx already defined (long form) */

/* 68: 0110 1000 xxxx xxxx */
void Cupd7907::MVI_V_xx(upd7907_state *cpustate)
{
    RDOPARG( V );
}

/* 69: 0110 1001 xxxx xxxx */
void Cupd7907::MVI_A_xx(upd7907_state *cpustate)
{
    if (PSW & L1) {	/* overlay active? */
        PC++;
        return; 	/* NOP */
    }
    RDOPARG( A );
    PSW |= L1;
}

/* 6a: 0110 1010 xxxx xxxx */
void Cupd7907::MVI_B_xx(upd7907_state *cpustate)
{
    RDOPARG( B );
}

/* 6b: 0110 1011 xxxx xxxx */
void Cupd7907::MVI_C_xx(upd7907_state *cpustate)
{
    RDOPARG( C );
}

/* 6c: 0110 1100 xxxx xxxx */
void Cupd7907::MVI_D_xx(upd7907_state *cpustate)
{
    RDOPARG( D );
}

/* 6d: 0110 1101 xxxx xxxx */
void Cupd7907::MVI_E_xx(upd7907_state *cpustate)
{
    RDOPARG( E );
}

/* 6e: 0110 1110 xxxx xxxx */
void Cupd7907::MVI_H_xx(upd7907_state *cpustate)
{
    RDOPARG( H );
}

/* 6f: 0110 1111 xxxx xxxx */
void Cupd7907::MVI_L_xx(upd7907_state *cpustate)
{
    if (PSW & L0) {	/* overlay active? */
        PC++;
        return; 	/* NOP */
    }
    RDOPARG( L );
    PSW |= L0;
}

/* 70: prefix */
void Cupd7907::PRE_70(upd7907_state *cpustate)
{
    RDOP(OP2);
    cpustate->icount -= cpustate->op70[OP2].cycles;
    cpustate->handle_timers(cpustate, cpustate->op70[OP2].cycles);
    (*cpustate->op70[OP2].opfunc)(cpustate);
}

/* 71: 0111 0001 oooo oooo xxxx xxxx */
void Cupd7907::MVIW_wa_xx(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 imm;

    RDOPARG( ea.b.l );
    RDOPARG( imm );

    WM( ea.d, imm );
}

/* 72: 0111 0010 */
void Cupd7907::SOFTI(upd7907_state *cpustate)
{
    SP--;
    WM( SPD, PSW );
    SP--;
    WM( SPD, PCH );
    SP--;
    WM( SPD, PCL );

    PC = 0x0060;
    cpustate->pPC->pCPU->CallSubLevel++;
    IFF = 0;
    ((Cupd7907*)(cpustate->pPC->pCPU))->softi = true;
}

/* 74: prefix */
void Cupd7907::PRE_74(upd7907_state *cpustate)
{
    RDOP(OP2);
    cpustate->icount -= cpustate->op74[OP2].cycles;
    cpustate->handle_timers(cpustate, cpustate->op74[OP2].cycles);
    (*cpustate->op74[OP2].opfunc)(cpustate);
}

/* 75: 0111 0101 oooo oooo xxxx xxxx */
void Cupd7907::EQIW_wa_xx(upd7907_state *cpustate)
{
    PAIR ea = cpustate->va;
    UINT8 tmp, m, imm;

    RDOPARG( ea.b.l );
    RDOPARG( imm );
    m = RM( ea.d );
    tmp = m - imm;
    ZHC_SUB( tmp, m, 0 );
    SKIP_Z;
}

/* 76: 0111 0110 xxxx xxxx */
/* SBI_A_xx already defined (long form) */

/* 77: 0111 0111 xxxx xxxx */
/* EQI_A_xx already defined (long form) */

/* 78: 0111 1ddd dddd dddd */
void Cupd7907::CALF(upd7907_state *cpustate)
{
    PAIR w;
    w.d = 0;

    RDOPARG( w.b.l );
    w.b.h = 0x08 + (OP & 0x07);

    SP--;
    WM( SPD, PCH );
    SP--;
    WM( SPD, PCL );

    PCD = w.d;
    cpustate->pPC->pCPU->CallSubLevel++;
}

/* 80: 100t tttt */
void Cupd7907::CALT(upd7907_state *cpustate)
{
    PAIR w;
    w.d = 0;

    switch (cpustate->config.type) {
    case TYPE_7810_GAMEMASTER:
        logerror ("!!!!!!!%.4x calt %.2x game master table position not known\n",PPC, OP);
        break;
    default:
    w.w.l = 0x80 + 2 * (OP & 0x1f);
    }

    if (cpustate->config.type!=TYPE_7810_GAMEMASTER) {
        SP--;
        WM( SPD, PCH );
        SP--;
        WM( SPD, PCL );

        PCL=RM(w.w.l);
        PCH=RM(w.w.l+1);
        cpustate->pPC->pCPU->CallSubLevel++;
    }
}

/* a0: 1010 0000 */
void Cupd7907::POP_VA(upd7907_state *cpustate)
{
    A = RM( SPD );
    SP++;
    V = RM( SPD );
    SP++;
}

/* a1: 1010 0001 */
void Cupd7907::POP_BC(upd7907_state *cpustate)
{
    C = RM( SPD );
    SP++;
    B = RM( SPD );
    SP++;
}

/* a2: 1010 0010 */
void Cupd7907::POP_DE(upd7907_state *cpustate)
{
    E = RM( SPD );
    SP++;
    D = RM( SPD );
    SP++;
}

/* a3: 1010 0011 */
void Cupd7907::POP_HL(upd7907_state *cpustate)
{
    L = RM( SPD );
    SP++;
    H = RM( SPD );
    SP++;
}

/* a4: 1010 0100 */
void Cupd7907::POP_EA(upd7907_state *cpustate)
{
    EAL = RM( SPD );
    SP++;
    EAH = RM( SPD );
    SP++;
}

/* a5: 1010 0101 */
void Cupd7907::DMOV_EA_BC(upd7907_state *cpustate)
{
    EA = BC;
}

/* a6: 1010 0110 */
void Cupd7907::DMOV_EA_DE(upd7907_state *cpustate)
{
    EA = DE;
}

/* a7: 1010 0111 */
void Cupd7907::DMOV_EA_HL(upd7907_state *cpustate)
{
    EA = HL;
}

/* a8: 1010 1000 */
void Cupd7907::INX_EA(upd7907_state *cpustate)
{
    EA++;
}

/* a9: 1010 1001 */
void Cupd7907::DCX_EA(upd7907_state *cpustate)
{
    EA--;
}

/* aa: 1010 1010 */
void Cupd7907::EI(upd7907_state *cpustate)
{
    IFF = 1;
}

/* ab: 1010 1011 dddd dddd */
void Cupd7907::LDAX_D_xx(upd7907_state *cpustate)
{
    UINT16 ea;
    RDOPARG( ea );
    ea += DE;
    A = RM( ea );
}

/* ac: 1010 1100 */
void Cupd7907::LDAX_H_A(upd7907_state *cpustate)
{
    UINT16 ea;
    ea = HL + A;
    A = RM( ea );
}

/* ad: 1010 1101 */
void Cupd7907::LDAX_H_B(upd7907_state *cpustate)
{
    UINT16 ea;
    ea = HL + B;
    A = RM( ea );
}

/* ae: 1010 1110 */
void Cupd7907::LDAX_H_EA(upd7907_state *cpustate)
{
    UINT16 ea;
    ea = HL + EA;
    A = RM( ea );
}

/* af: 1010 1111 dddd dddd */
void Cupd7907::LDAX_H_xx(upd7907_state *cpustate)
{
    UINT16 ea;
    RDOPARG( ea );
    ea += HL;
    A = RM( ea );
}

/* b0: 1011 0000 */
void Cupd7907::PUSH_VA(upd7907_state *cpustate)
{
    SP--;
    WM( SPD, V );
    SP--;
    WM( SPD, A );
}

/* b1: 1011 0001 */
void Cupd7907::PUSH_BC(upd7907_state *cpustate)
{
    SP--;
    WM( SPD, B );
    SP--;
    WM( SPD, C );
}

/* b2: 1011 0010 */
void Cupd7907::PUSH_DE(upd7907_state *cpustate)
{
    SP--;
    WM( SPD, D );
    SP--;
    WM( SPD, E );
}

/* b3: 1011 0011 */
void Cupd7907::PUSH_HL(upd7907_state *cpustate)
{
    SP--;
    WM( SPD, H );
    SP--;
    WM( SPD, L );
}

/* b4: 1011 0100 */
void Cupd7907::PUSH_EA(upd7907_state *cpustate)
{
    SP--;
    WM( SPD, EAH );
    SP--;
    WM( SPD, EAL );
}

/* b5: 1011 0101 */
void Cupd7907::DMOV_BC_EA(upd7907_state *cpustate)
{
    BC = EA;
}

/* b6: 1011 0110 */
void Cupd7907::DMOV_DE_EA(upd7907_state *cpustate)
{
    DE = EA;
}

/* b7: 1011 0111 */
void Cupd7907::DMOV_HL_EA(upd7907_state *cpustate)
{
    HL = EA;
}

/* b8: 1011 1000 */
void Cupd7907::RET(upd7907_state *cpustate)
{
    PCL = RM( SPD );
    SP++;
    PCH = RM( SPD );
    SP++;
    cpustate->pPC->pCPU->CallSubLevel--;
}

/* b9: 1011 1001 */
void Cupd7907::RETS(upd7907_state *cpustate)
{
    PCL = RM( SPD );
    SP++;
    PCH = RM( SPD );
    SP++;
    PSW|=SK;	/* skip one instruction */
    cpustate->pPC->pCPU->CallSubLevel--;
}

/* ba: 1011 1010 */
void Cupd7907::DI(upd7907_state *cpustate)
{
    IFF = 0;
}

/* bb: 1011 1011 dddd dddd */
void Cupd7907::STAX_D_xx(upd7907_state *cpustate)
{
    UINT16 ea;
    RDOPARG(ea);
    ea += DE;
    WM( ea, A );
}

/* bc: 1011 1100 */
void Cupd7907::STAX_H_A(upd7907_state *cpustate)
{
    UINT16 ea = A;
    ea += HL;
    WM( ea, A );
}

/* bd: 1011 1101 */
void Cupd7907::STAX_H_B(upd7907_state *cpustate)
{
    UINT16 ea = B;
    ea += HL;
    WM( ea, A );
}

/* be: 1011 1110 */
void Cupd7907::STAX_H_EA(upd7907_state *cpustate)
{
    UINT16 ea = EA;
    ea += HL;
    WM( ea, A );
}

/* bf: 1011 1111 dddd dddd */
void Cupd7907::STAX_H_xx(upd7907_state *cpustate)
{
    UINT16 ea;
    RDOPARG( ea );
    ea += HL;
    WM( ea, A );
}

/* c0: 11dd dddd */
void Cupd7907::JR(upd7907_state *cpustate)
{
    INT8 offs = (INT8)(OP << 2) >> 2;
    PC += offs;
}

/*********************/
/*                   */
/* 7801 instructions */
/*                   */
/*********************/

void Cupd7907::CALT_7801(upd7907_state *cpustate)
{
    PAIR w;
    w.d = 0;

    w.w.l = 0x80 + 2 * (OP & 0x3f);

    SP--;
    WM( SPD, PCH );
    SP--;
    WM( SPD, PCL );

    PCL=RM(w.w.l);
    PCH=RM(w.w.l+1);
    cpustate->pPC->pCPU->CallSubLevel++;
}

/* DCR(W) and INR(W) instructions do not modify the CY register on at least 78c05 and 78c06 */
void Cupd7907::DCR_A_7801(upd7907_state *cpustate)
{
    UINT32 old_CY = PSW & CY;
    DCR_A(cpustate);
    PSW = ( PSW & ~CY ) | old_CY;
}

void Cupd7907::DCR_B_7801(upd7907_state *cpustate)
{
    UINT32 old_CY = PSW & CY;
    DCR_B(cpustate);
    PSW = ( PSW & ~CY ) | old_CY;
}

void Cupd7907::DCR_C_7801(upd7907_state *cpustate)
{
    UINT32 old_CY = PSW & CY;
    DCR_C(cpustate);
    PSW = ( PSW & ~CY ) | old_CY;
}

void Cupd7907::DCRW_wa_7801(upd7907_state *cpustate)
{
    UINT32 old_CY = PSW & CY;
    DCRW_wa(cpustate);
    PSW = ( PSW & ~CY ) | old_CY;
}

void Cupd7907::INR_A_7801(upd7907_state *cpustate)
{
    UINT32 old_CY = PSW & CY;
    INR_A(cpustate);
    PSW = ( PSW & ~CY ) | old_CY;
}

void Cupd7907::INR_B_7801(upd7907_state *cpustate)
{
    UINT32 old_CY = PSW & CY;
    INR_B(cpustate);
    PSW = ( PSW & ~CY ) | old_CY;
}

void Cupd7907::INR_C_7801(upd7907_state *cpustate)
{
    UINT32 old_CY = PSW & CY;
    INR_C(cpustate);
    PSW = ( PSW & ~CY ) | old_CY;
}

void Cupd7907::INRW_wa_7801(upd7907_state *cpustate)
{
    UINT32 old_CY = PSW & CY;
    INRW_wa(cpustate);
    PSW = ( PSW & ~CY ) | old_CY;
}

void Cupd7907::IN(upd7907_state *cpustate)
{
    logerror("unimplemented instruction: IN\n");
}

void Cupd7907::OUT(upd7907_state *cpustate)
{
    logerror("unimplemented instruction: OUT\n");
}

void Cupd7907::MOV_A_S(upd7907_state *cpustate)
{
    A = S;
    if ((cpustate->pPC->pCPU->fp_log))
        fprintf(cpustate->pPC->pCPU->fp_log,"\nREAD SERIAL=%02X = (%c)\n", S,S);
}

void Cupd7907::MOV_S_A(upd7907_state *cpustate)
{
    S = A;
    if ((cpustate->pPC->pCPU->fp_log))
        fprintf(cpustate->pPC->pCPU->fp_log,"\nSERIAL=%02X = (%c)\n", S,S);

    cpustate->serialPending = true;

}

void Cupd7907::PEN(upd7907_state *cpustate)
{
    logerror("unimplemented instruction: PEN\n");
}

void Cupd7907::PER(upd7907_state *cpustate)
{
    logerror("unimplemented instruction: PER\n");
}

void Cupd7907::PEX(upd7907_state *cpustate)
{
    cpustate->pPC->out16(UPD7907_PORTE,BC);
    logerror("unimplemented instruction: PEX\n");
}

void Cupd7907::SIO(upd7907_state *cpustate)
{
    IRR |= INTFST;
    logerror("unimplemented instruction: SIO\n");
}

void Cupd7907::SKIT_F0(upd7907_state *cpustate)
{
    if (IRR & INTF0)
        PSW |= SK;
    IRR &= ~INTF0;
}

void Cupd7907::SKNIT_F0(upd7907_state *cpustate)
{
    logerror("unimplemented instruction: SKNIT_F0\n");
}

void Cupd7907::STM(upd7907_state *cpustate)
{
    cpustate->ovc0 = ( ( TMM & 0x04 ) ? 16 * 8 : 8 ) * TM0;
}

void Cupd7907::STM_7801(upd7907_state *cpustate)
{
    /* Set the timer flip/fliop */
    TO = 1;
//    if ( cpustate->config.io_callback)
//        (*cpustate->config.io_callback)(cpustate->device,UPD7907_TO,TO);

    /* Reload the timer */
    cpustate->ovc0 = 16 * ( TM0 + ( ( TM1 & 0x0f ) << 8 ) );
}

void Cupd7907::MOV_MC_A_7801(upd7907_state *cpustate)
{
    /* On the 7801 the mode C bits function as follows: */
    /*       Cn=1   Cn=0         */
    /* PC0  Input   Output       */
    /* PC1  Input   Output       */
    /* PC2  Input   -SCS Input   */
    /* PC3  Output  SAK Output   */
    /* PC4  Output  To Output    */
    /* PC5  Output  IO/-M Output */
    /* PC6  Output  HLDA Output  */
    /* PC7  Input   HOLD Input   */
    MC = 0x84 | ( ( A & 0x02 ) ? 0x02 : 0x00 ) | ( ( A & 0x01 ) ? 0x01 : 0x00 );
}



void Cupd7907::Reset()
{
    upd7907_state *cpustate = &upd7907stat;
//	UPD7907_CONFIG save_config;
//	device_irq_acknowledge_callback save_irqcallback;

//	save_config = cpustate->config;
//	save_irqcallback = cpustate->irq_callback;
    memset(cpustate, 0, sizeof(*cpustate));
    resetFlag = true;
    memset(cpustate->imem,0xff,0x10);
//	cpustate->config = save_config;
//	cpustate->irq_callback = save_irqcallback;
//	cpustate->device = device;
//	cpustate->program = device->space(AS_PROGRAM);
//	cpustate->direct = &cpustate->program->direct();
//	cpustate->io = device->space(AS_IO);

    cpustate->pPC  = pPC;
    cpustate->opXX = opXX_7907;
    cpustate->op48 = op48_7907;
    cpustate->op4C = op4C_7907;
    cpustate->op4D = op4D_7907;
    cpustate->op60 = op60_7907;
    cpustate->op64 = op64_7907;
    cpustate->op70 = op70_7907;
    cpustate->op74 = op74_7907;
    cpustate->config.type=TYPE_78C06;
	ETMM = 0xff;
    EOM = 0xff;
	TMM = 0xff;
	MA = 0xff;
	MB = 0xff;
	switch (cpustate->config.type)
	{
		case TYPE_7810_GAMEMASTER:
		    // needed for lcd screen/ram selection; might be internal in cpu and therefor not needed; 0x10 written in some games
			MC = 0xff&~0x7;
            WP( cpustate, UPD7907_PORTC, 1 ); //hyper space
			PCD=0x8000;
			break;
		default:
			MC = 0xff;
	}
	MF = 0xff;
	// gamemaster falling block "and"s to enable interrupts
	MKL = 0xff;
	MKH = 0xff; //?
    //cpustate->handle_timers = upd7907_timers;

    MA = 0;		/* All outputs */
    MB = 0;
    MC = 0xFF;	/* All inputs */
    V = 0xFF;	/* The vector register is always pointing to FF00 */
    cpustate->handle_timers = upd78c05_timers;
    TM0 = 0xFF;	/* Timer seems to be running from boot */
    cpustate->ovc0 = ( ( TMM & 0x04 ) ? 16 * 8 : 8 ) * TM0;

    softi=false;
    cpustate->serialPending = false;
}

void Cupd7907::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "upd7907")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &upd7907stat,ba_reg.data(),sizeof(upd7907stat));
            upd7907stat.pPC = pPC;
            upd7907stat.opXX = opXX_7907;
            upd7907stat.op48 = op48_7907;
            upd7907stat.op4C = op4C_7907;
            upd7907stat.op4D = op4D_7907;
            upd7907stat.op60 = op60_7907;
            upd7907stat.op64 = op64_7907;
            upd7907stat.op70 = op70_7907;
            upd7907stat.op74 = op74_7907;
            upd7907stat.handle_timers = upd78c05_timers;
//            QByteArray ba_imem = QByteArray::fromBase64(xmlIn->attributes().value("iMem").toString().toLatin1());
//            memcpy((char *) &(upd7907stat.imem),ba_imem.data(),IMEM_LEN);

        }
        xmlIn->skipCurrentElement();
    }
}

void Cupd7907::save_internal(QXmlStreamWriter *xmlOut)
{

    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","upd7907");
        QByteArray ba_reg((char*)&upd7907stat,sizeof(upd7907stat));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
    xmlOut->writeEndElement();
}
#if 0
static CPU_RESET( upd7807 )
{
    upd7907_state *cpustate = get_safe_token(device);
	CPU_RESET_CALL(upd7810);
	cpustate->opXX = opXX_7807;
}

static CPU_RESET( upd7801 )
{
    upd7907_state *cpustate = get_safe_token(device);
	CPU_RESET_CALL(upd7810);
	cpustate->op48 = op48_7801;
	cpustate->op4C = op4C_7801;
	cpustate->op4D = op4D_7801;
	cpustate->op60 = op60_7801;
	cpustate->op64 = op64_7801;
	cpustate->op70 = op70_7801;
	cpustate->op74 = op74_7801;
	cpustate->opXX = opXX_7801;
	cpustate->handle_timers = upd7801_timers;
	MA = 0;		/* Port A is output port on the uPD7801 */
	cpustate->ovc0 = 0;
}

static CPU_RESET( upd78c05 )
{
    upd7907_state *cpustate = get_safe_token(device);
	CPU_RESET_CALL(upd7810);
	cpustate->op48 = op48_78c05;
	cpustate->op4C = op4C_78c05;
	cpustate->op4D = op4D_78c05;
	cpustate->op60 = op60_78c05;
	cpustate->op64 = op64_78c05;
	cpustate->op70 = op70_78c05;
	cpustate->op74 = op74_78c05;
	cpustate->opXX = opXX_78c05;
	MA = 0;		/* All outputs */
	MC = 0xFF;	/* All inputs */
	V = 0xFF;	/* The vector register is always pointing to FF00 */
	cpustate->handle_timers = upd78c05_timers;
	TM0 = 0xFF;	/* Timer seems to be running from boot */
	cpustate->ovc0 = ( ( TMM & 0x04 ) ? 16 * 8 : 8 ) * TM0;
}

static CPU_RESET( upd78c06 )
{
    upd7907_state *cpustate = get_safe_token(device);
	CPU_RESET_CALL(upd78c05);
	cpustate->op48 = op48_78c06;
	cpustate->op4C = op4C_78c06;
	cpustate->op4D = op4D_78c06;
	cpustate->op60 = op60_78c06;
	cpustate->op64 = op64_78c06;
	cpustate->op70 = op70_78c06;
	cpustate->op74 = op74_78c06;
	cpustate->opXX = opXX_78c06;
}

static CPU_EXIT( upd7810 )
{
}
#endif
void Cupd7907::execute(upd7907_state *cpustate)
{

	do
	{
		int cc = 0;

//		debugger_instruction_hook(device, PC);

		PPC = PC;
		RDOP(OP);

		/*
         * clear L0 and/or L1 flags for all opcodes except
         * L0   for "MVI L,xx" or "LXI H,xxxx"
         * L1   for "MVI A,xx"
         */
		PSW &= ~cpustate->opXX[OP].mask_l0_l1;

		/* skip flag set and not SOFTI opcode? */
		if ((PSW & SK) && (OP != 0x72))
		{
			if (cpustate->opXX[OP].cycles)
			{
				cc = cpustate->opXX[OP].cycles_skip;
				PC += cpustate->opXX[OP].oplen - 1;
			}
			else
			{
				RDOP(OP2);
				switch (OP)
				{
				case 0x48:
					cc = cpustate->op48[OP2].cycles_skip;
					PC += cpustate->op48[OP2].oplen - 2;
					break;
				case 0x4c:
					cc = cpustate->op4C[OP2].cycles_skip;
					PC += cpustate->op4C[OP2].oplen - 2;
					break;
				case 0x4d:
					cc = cpustate->op4D[OP2].cycles_skip;
					PC += cpustate->op4D[OP2].oplen - 2;
					break;
				case 0x60:
					cc = cpustate->op60[OP2].cycles_skip;
					PC += cpustate->op60[OP2].oplen - 2;
					break;
				case 0x64:
					cc = cpustate->op64[OP2].cycles_skip;
					PC += cpustate->op64[OP2].oplen - 2;
					break;
				case 0x70:
					cc = cpustate->op70[OP2].cycles_skip;
					PC += cpustate->op70[OP2].oplen - 2;
					break;
				case 0x74:
					cc = cpustate->op74[OP2].cycles_skip;
					PC += cpustate->op74[OP2].oplen - 2;
					break;
				default:
//					fatalerror("uPD7810 internal error: check cycle counts for main");
                    break;
				}
			}
			PSW &= ~SK;
            cpustate->handle_timers( cpustate, cc );

		}
		else
		{
			cc = cpustate->opXX[OP].cycles;
            cpustate->handle_timers( cpustate, cc );

			(*cpustate->opXX[OP].opfunc)(cpustate);
            V = 0xFF;
		}
		cpustate->icount -= cc;
        upd7907_take_irq(cpustate);

	} while (cpustate->icount > 0);
}

void Cupd7907::set_irq_line(upd7907_state *cpustate, int irqline, int state)
{
	/* The uPD7801 can check for falling and rising edges changes on the INT2 input */
	switch ( cpustate->config.type )
	{
	case TYPE_7801:
		switch ( irqline )
		{
        case UPD7907_INTF0:
			/* INT0 is level sensitive */
			if ( state == ASSERT_LINE )
				IRR |= INTF0;
			else
				IRR &= INTF0;
			break;

        case UPD7907_INTF1:
			/* INT1 is rising edge sensitive */
			if ( cpustate->int1 == CLEAR_LINE && state == ASSERT_LINE )
				IRR |= INTF1;

			cpustate->int1 = state;
			break;

        case UPD7907_INTF2:
			/* INT2 is rising or falling edge sensitive */
			/* Check if the ES bit is set then check for rising edge, otherwise falling edge */
			if ( MKL & 0x20 )
			{
				if ( cpustate->int2 == CLEAR_LINE && state == ASSERT_LINE )
				{
					IRR |= INTF2;
				}
			}
			else
			{
				if ( cpustate->int2 == ASSERT_LINE && state == CLEAR_LINE )
				{
					IRR |= INTF2;
				}
			}
			cpustate->int2 = state;
			break;
		}
		break;

	default:
		if (state != CLEAR_LINE)
		{
			if (irqline == INPUT_LINE_NMI)
			{
				/* no nested NMIs ? */
//              if (0 == (IRR & INTNMI))
				{
					IRR |= INTNMI;
					SP--;
					WM( SP, PSW );
					SP--;
					WM( SP, PCH );
					SP--;
					WM( SP, PCL );
					IFF = 0;
					PSW &= ~(SK|L0|L1);
					PC = 0x0004;
				}
			}
			else
            if (irqline == UPD7907_INTF1)
				IRR |= INTF1;
			else
            if ( irqline == UPD7907_INTF2 && ( MKL & 0x20 ) )
				IRR |= INTF2;
			// gamemaster hack
			else
            if (irqline == UPD7907_INTFE1)
				IRR |= INTFE1;
			else
                AddLog(LOG_CPU,tr("upd7907_set_irq_line invalid irq line #%1\n").arg(irqline));
		}
		/* resetting interrupt requests is done with the SKIT/SKNIT opcodes only! */
	}
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/
#if 0
static CPU_SET_INFO( upd7810 )
{
    upd7907_state *cpustate = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	break;
        case CPUINFO_INT_INPUT_STATE + UPD7907_INTF1:	set_irq_line(cpustate, UPD7907_INTF1, info->i);	break;
        case CPUINFO_INT_INPUT_STATE + UPD7907_INTF2:	set_irq_line(cpustate, UPD7907_INTF2, info->i);	break;
        case CPUINFO_INT_INPUT_STATE + UPD7907_INTFE1:	set_irq_line(cpustate, UPD7907_INTFE1, info->i);	break;

		case CPUINFO_INT_PC:							PC = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_PC:			PC = info->i;							break;
		case CPUINFO_INT_SP:
        case CPUINFO_INT_REGISTER + UPD7907_SP:			SP = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_PSW:		PSW = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_A:			A = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_V:			V = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_EA:			EA = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_VA:			VA = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_BC:			BC = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_DE:			DE = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_HL:			HL = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_EA2:		EA2 = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_VA2:		VA2 = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_BC2:		BC2 = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_DE2:		DE2 = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_HL2:		HL2 = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CNT0:		CNT0 = info->i; 						break;
        case CPUINFO_INT_REGISTER + UPD7907_CNT1:		CNT1 = info->i; 						break;
        case CPUINFO_INT_REGISTER + UPD7907_TM0:		TM0 = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_TM1:		TM1 = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_ECNT:		ECNT = info->i; 						break;
        case CPUINFO_INT_REGISTER + UPD7907_ECPT:		ECPT = info->i; 						break;
        case CPUINFO_INT_REGISTER + UPD7907_ETM0:		ETM0 = info->i; 						break;
        case CPUINFO_INT_REGISTER + UPD7907_ETM1:		ETM1 = info->i; 						break;
        case CPUINFO_INT_REGISTER + UPD7907_MA:			MA = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MB:			MB = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MCC:		MCC = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MC:			MC = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MM:			MM = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MF:			MF = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_TMM:		TMM = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_ETMM:		ETMM = info->i; 						break;
        case CPUINFO_INT_REGISTER + UPD7907_EOM:		EOM = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_SML:		SML = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_SMH:		SMH = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_ANM:		ANM = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MKL:		MKL = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MKH:		MKH = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_ZCM:		ZCM = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_TXB:		TXB = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_RXB:		RXB = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CR0:		CR0 = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CR1:		CR1 = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CR2:		CR2 = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CR3:		CR3 = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_TXD:		TXD = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_RXD:		RXD = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_SCK:		SCK = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_TI:			TI	= info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_TO:			TO	= info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CI:			CI	= info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CO0:		CO0 = info->i;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CO1:		CO1 = info->i;							break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( upd7810 )
{
    upd7907_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
        case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(upd7907_state);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 40;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = (IRR & INTNMI) ? ASSERT_LINE : CLEAR_LINE; break;
        case CPUINFO_INT_INPUT_STATE + UPD7907_INTF1:	info->i = (IRR & INTF1) ? ASSERT_LINE : CLEAR_LINE; break;
        case CPUINFO_INT_INPUT_STATE + UPD7907_INTF2:	info->i = (IRR & INTF2) ? ASSERT_LINE : CLEAR_LINE; break;
        case CPUINFO_INT_INPUT_STATE + UPD7907_INTFE1:	info->i = (IRR & INTFE1) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;							break;

		case CPUINFO_INT_PC:
        case CPUINFO_INT_REGISTER + UPD7907_PC:			info->i = PC;							break;
		case CPUINFO_INT_SP:
        case CPUINFO_INT_REGISTER + UPD7907_SP:			info->i = SP;							break;
        case CPUINFO_INT_REGISTER + UPD7907_PSW:		info->i = PSW;							break;
        case CPUINFO_INT_REGISTER + UPD7907_EA:			info->i = EA;							break;
        case CPUINFO_INT_REGISTER + UPD7907_VA:			info->i = VA;							break;
        case CPUINFO_INT_REGISTER + UPD7907_BC:			info->i = BC;							break;
        case CPUINFO_INT_REGISTER + UPD7907_DE:			info->i = DE;							break;
        case CPUINFO_INT_REGISTER + UPD7907_HL:			info->i = HL;							break;
        case CPUINFO_INT_REGISTER + UPD7907_EA2:		info->i = EA2;							break;
        case CPUINFO_INT_REGISTER + UPD7907_VA2:		info->i = VA2;							break;
        case CPUINFO_INT_REGISTER + UPD7907_BC2:		info->i = BC2;							break;
        case CPUINFO_INT_REGISTER + UPD7907_DE2:		info->i = DE2;							break;
        case CPUINFO_INT_REGISTER + UPD7907_HL2:		info->i = HL2;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CNT0:		info->i = CNT0;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CNT1:		info->i = CNT1;							break;
        case CPUINFO_INT_REGISTER + UPD7907_TM0:		info->i = TM0;							break;
        case CPUINFO_INT_REGISTER + UPD7907_TM1:		info->i = TM1;							break;
        case CPUINFO_INT_REGISTER + UPD7907_ECNT:		info->i = ECNT;							break;
        case CPUINFO_INT_REGISTER + UPD7907_ECPT:		info->i = ECPT;							break;
        case CPUINFO_INT_REGISTER + UPD7907_ETM0:		info->i = ETM0;							break;
        case CPUINFO_INT_REGISTER + UPD7907_ETM1:		info->i = ETM1;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MA:			info->i = MA;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MB:			info->i = MB;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MCC:		info->i = MCC;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MC:			info->i = MC;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MM:			info->i = MM;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MF:			info->i = MF;							break;
        case CPUINFO_INT_REGISTER + UPD7907_TMM:		info->i = TMM;							break;
        case CPUINFO_INT_REGISTER + UPD7907_ETMM:		info->i = ETMM;							break;
        case CPUINFO_INT_REGISTER + UPD7907_EOM:		info->i = EOM;							break;
        case CPUINFO_INT_REGISTER + UPD7907_SML:		info->i = SML;							break;
        case CPUINFO_INT_REGISTER + UPD7907_SMH:		info->i = SMH;							break;
        case CPUINFO_INT_REGISTER + UPD7907_ANM:		info->i = ANM;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MKL:		info->i = MKL;							break;
        case CPUINFO_INT_REGISTER + UPD7907_MKH:		info->i = MKH;							break;
        case CPUINFO_INT_REGISTER + UPD7907_ZCM:		info->i = ZCM;							break;
        case CPUINFO_INT_REGISTER + UPD7907_TXB:		info->i = TXB;							break;
        case CPUINFO_INT_REGISTER + UPD7907_RXB:		info->i = RXB;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CR0:		info->i = CR0;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CR1:		info->i = CR1;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CR2:		info->i = CR2;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CR3:		info->i = CR3;							break;
        case CPUINFO_INT_REGISTER + UPD7907_TXD:		info->i = TXD;							break;
        case CPUINFO_INT_REGISTER + UPD7907_RXD:		info->i = RXD;							break;
        case CPUINFO_INT_REGISTER + UPD7907_SCK:		info->i = SCK;							break;
        case CPUINFO_INT_REGISTER + UPD7907_TI:			info->i = TI;							break;
        case CPUINFO_INT_REGISTER + UPD7907_TO:			info->i = TO;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CI:			info->i = CI;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CO0:		info->i = CO0;							break;
        case CPUINFO_INT_REGISTER + UPD7907_CO1:		info->i = CO1;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(upd7810);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(upd7810);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(upd7810);			break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(upd7810);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(upd7810);		break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(upd7810);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "uPD7810");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "NEC uPD7810");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "0.3");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Juergen Buchmueller, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%s:%s:%s:%s:%s:%s",
				cpustate->psw & 0x40 ? "ZF":"--",
				cpustate->psw & 0x20 ? "SK":"--",
				cpustate->psw & 0x10 ? "HC":"--",
				cpustate->psw & 0x08 ? "L1":"--",
				cpustate->psw & 0x04 ? "L0":"--",
				cpustate->psw & 0x01 ? "CY":"--");
			break;

        case CPUINFO_STR_REGISTER + UPD7907_PC:			sprintf(info->s, "PC  :%04X", cpustate->pc.w.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_SP:			sprintf(info->s, "SP  :%04X", cpustate->sp.w.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_PSW:		sprintf(info->s, "PSW :%02X", cpustate->psw); break;
        case CPUINFO_STR_REGISTER + UPD7907_A:			sprintf(info->s, "A   :%02X", cpustate->va.b.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_V:			sprintf(info->s, "V   :%02X", cpustate->va.b.h); break;
        case CPUINFO_STR_REGISTER + UPD7907_EA:			sprintf(info->s, "EA  :%04X", cpustate->ea.w.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_BC:			sprintf(info->s, "BC  :%04X", cpustate->bc.w.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_DE:			sprintf(info->s, "DE  :%04X", cpustate->de.w.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_HL:			sprintf(info->s, "HL  :%04X", cpustate->hl.w.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_A2:			sprintf(info->s, "A'  :%02X", cpustate->va2.b.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_V2:			sprintf(info->s, "V'  :%02X", cpustate->va2.b.h); break;
        case CPUINFO_STR_REGISTER + UPD7907_EA2:		sprintf(info->s, "EA' :%04X", cpustate->ea2.w.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_BC2:		sprintf(info->s, "BC' :%04X", cpustate->bc2.w.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_DE2:		sprintf(info->s, "DE' :%04X", cpustate->de2.w.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_HL2:		sprintf(info->s, "HL' :%04X", cpustate->hl2.w.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_CNT0:		sprintf(info->s, "CNT0:%02X", cpustate->cnt.b.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_CNT1:		sprintf(info->s, "CNT1:%02X", cpustate->cnt.b.h); break;
        case CPUINFO_STR_REGISTER + UPD7907_TM0:		sprintf(info->s, "TM0 :%02X", cpustate->tm.b.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_TM1:		sprintf(info->s, "TM1 :%02X", cpustate->tm.b.h); break;
        case CPUINFO_STR_REGISTER + UPD7907_ECNT:		sprintf(info->s, "ECNT:%04X", cpustate->ecnt.w.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_ECPT:		sprintf(info->s, "ECPT:%04X", cpustate->ecnt.w.h); break;
        case CPUINFO_STR_REGISTER + UPD7907_ETM0:		sprintf(info->s, "ETM0:%04X", cpustate->etm.w.l); break;
        case CPUINFO_STR_REGISTER + UPD7907_ETM1:		sprintf(info->s, "ETM1:%04X", cpustate->etm.w.h); break;
        case CPUINFO_STR_REGISTER + UPD7907_MA:			sprintf(info->s, "MA  :%02X", cpustate->ma); break;
        case CPUINFO_STR_REGISTER + UPD7907_MB:			sprintf(info->s, "MB  :%02X", cpustate->mb); break;
        case CPUINFO_STR_REGISTER + UPD7907_MCC:		sprintf(info->s, "MCC :%02X", cpustate->mcc); break;
        case CPUINFO_STR_REGISTER + UPD7907_MC:			sprintf(info->s, "MC  :%02X", cpustate->mc); break;
        case CPUINFO_STR_REGISTER + UPD7907_MM:			sprintf(info->s, "MM  :%02X", cpustate->mm); break;
        case CPUINFO_STR_REGISTER + UPD7907_MF:			sprintf(info->s, "MF  :%02X", cpustate->mf); break;
        case CPUINFO_STR_REGISTER + UPD7907_TMM:		sprintf(info->s, "TMM :%02X", cpustate->tmm); break;
        case CPUINFO_STR_REGISTER + UPD7907_ETMM:		sprintf(info->s, "ETMM:%02X", cpustate->etmm); break;
        case CPUINFO_STR_REGISTER + UPD7907_EOM:		sprintf(info->s, "EOM :%02X", cpustate->eom); break;
        case CPUINFO_STR_REGISTER + UPD7907_SML:		sprintf(info->s, "SML :%02X", cpustate->sml); break;
        case CPUINFO_STR_REGISTER + UPD7907_SMH:		sprintf(info->s, "SMH :%02X", cpustate->smh); break;
        case CPUINFO_STR_REGISTER + UPD7907_ANM:		sprintf(info->s, "ANM :%02X", cpustate->anm); break;
        case CPUINFO_STR_REGISTER + UPD7907_MKL:		sprintf(info->s, "MKL :%02X", cpustate->mkl); break;
        case CPUINFO_STR_REGISTER + UPD7907_MKH:		sprintf(info->s, "MKH :%02X", cpustate->mkh); break;
        case CPUINFO_STR_REGISTER + UPD7907_ZCM:		sprintf(info->s, "ZCM :%02X", cpustate->zcm); break;
        case CPUINFO_STR_REGISTER + UPD7907_CR0:		sprintf(info->s, "CR0 :%02X", cpustate->cr0); break;
        case CPUINFO_STR_REGISTER + UPD7907_CR1:		sprintf(info->s, "CR1 :%02X", cpustate->cr1); break;
        case CPUINFO_STR_REGISTER + UPD7907_CR2:		sprintf(info->s, "CR2 :%02X", cpustate->cr2); break;
        case CPUINFO_STR_REGISTER + UPD7907_CR3:		sprintf(info->s, "CR3 :%02X", cpustate->cr3); break;
        case CPUINFO_STR_REGISTER + UPD7907_RXB:		sprintf(info->s, "RXB :%02X", cpustate->rxb); break;
        case CPUINFO_STR_REGISTER + UPD7907_TXB:		sprintf(info->s, "TXB :%02X", cpustate->txb); break;
        case CPUINFO_STR_REGISTER + UPD7907_TXD:		sprintf(info->s, "TXD :%d", cpustate->txd); break;
        case CPUINFO_STR_REGISTER + UPD7907_RXD:		sprintf(info->s, "RXD :%d", cpustate->rxd); break;
        case CPUINFO_STR_REGISTER + UPD7907_SCK:		sprintf(info->s, "SCK :%d", cpustate->sck); break;
        case CPUINFO_STR_REGISTER + UPD7907_TI:			sprintf(info->s, "TI  :%d", cpustate->ti); break;
        case CPUINFO_STR_REGISTER + UPD7907_TO:			sprintf(info->s, "TO  :%d", cpustate->to); break;
        case CPUINFO_STR_REGISTER + UPD7907_CI:			sprintf(info->s, "CI  :%d", cpustate->ci); break;
        case CPUINFO_STR_REGISTER + UPD7907_CO0:		sprintf(info->s, "CO0 :%d", cpustate->co0 & 1); break;
        case CPUINFO_STR_REGISTER + UPD7907_CO1:		sprintf(info->s, "CO1 :%d", cpustate->co1 & 1); break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( upd7807 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(upd7807);			break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(upd7807);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "uPD7807");				break;

		default:										CPU_GET_INFO_CALL(upd7810);				break;
	}
}

CPU_GET_INFO( upd7801 ) {
	switch( state ) {
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(upd7801);			break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(upd7801);		break;

		case DEVINFO_STR_NAME:							strcpy(info->s, "uPD7801");				break;

		default:										CPU_GET_INFO_CALL(upd7810);				break;
	}
}

CPU_GET_INFO( upd78c05 ) {
	switch ( state ) {
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 4;							break;

		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(upd78c05);			break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(upd78c05);		break;

		case DEVINFO_STR_NAME:							strcpy(info->s, "uPD78C05");			break;

		/* These registers are not present in the uPD78C05 cpu */
        case CPUINFO_STR_REGISTER + UPD7907_A2:
        case CPUINFO_STR_REGISTER + UPD7907_V2:
        case CPUINFO_STR_REGISTER + UPD7907_EA2:
        case CPUINFO_STR_REGISTER + UPD7907_BC2:
        case CPUINFO_STR_REGISTER + UPD7907_DE2:
        case CPUINFO_STR_REGISTER + UPD7907_HL2:
        case CPUINFO_STR_REGISTER + UPD7907_MA:
        case CPUINFO_STR_REGISTER + UPD7907_MCC:
        case CPUINFO_STR_REGISTER + UPD7907_MC:
        case CPUINFO_STR_REGISTER + UPD7907_MM:
        case CPUINFO_STR_REGISTER + UPD7907_MF:
        case CPUINFO_STR_REGISTER + UPD7907_ETMM:
        case CPUINFO_STR_REGISTER + UPD7907_EOM:
        case CPUINFO_STR_REGISTER + UPD7907_SML:
        case CPUINFO_STR_REGISTER + UPD7907_SMH:
        case CPUINFO_STR_REGISTER + UPD7907_ANM:
        case CPUINFO_STR_REGISTER + UPD7907_MKH:
        case CPUINFO_STR_REGISTER + UPD7907_ZCM:
        case CPUINFO_STR_REGISTER + UPD7907_CR0:
        case CPUINFO_STR_REGISTER + UPD7907_CR1:
        case CPUINFO_STR_REGISTER + UPD7907_CR2:
        case CPUINFO_STR_REGISTER + UPD7907_CR3:
        case CPUINFO_STR_REGISTER + UPD7907_RXB:
        case CPUINFO_STR_REGISTER + UPD7907_TXB:
        case CPUINFO_STR_REGISTER + UPD7907_TXD:
        case CPUINFO_STR_REGISTER + UPD7907_RXD:
        case CPUINFO_STR_REGISTER + UPD7907_SCK:
        case CPUINFO_STR_REGISTER + UPD7907_TI:
        case CPUINFO_STR_REGISTER + UPD7907_TO:
        case CPUINFO_STR_REGISTER + UPD7907_CI:
        case CPUINFO_STR_REGISTER + UPD7907_CO0:
        case CPUINFO_STR_REGISTER + UPD7907_CO1:		break;

		default:										CPU_GET_INFO_CALL(upd7801);				break;
	}
}

CPU_GET_INFO( upd78c06 ) {
	switch ( state ) {
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(upd78c06);			break;

		case DEVINFO_STR_NAME:							strcpy(info->s, "uPD78C06");			break;

		default:										CPU_GET_INFO_CALL(upd78c05);				break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(UPD7810, upd7810);
DEFINE_LEGACY_CPU_DEVICE(UPD7807, upd7807);
DEFINE_LEGACY_CPU_DEVICE(UPD7801, upd7801);
DEFINE_LEGACY_CPU_DEVICE(UPD78C05, upd78c05);
DEFINE_LEGACY_CPU_DEVICE(UPD78C06, upd78c06);
#endif


UINT32 Cupd7907::get_PC()
{
    upd7907_state *cpustate = &upd7907stat;
    return PC;
}

void Cupd7907::Regs_Info(UINT8 Type)
{
    sprintf(Regs_String,"EMPTY");

#if 1
    upd7907_state *cpustate = &upd7907stat;
    char buf[32];
    switch(Type)
    {
    case 0:			// Monitor Registers Dialog
        sprintf(
                    Regs_String,
                    "VA=%04x\nBC=%04x\nDE=%04x\nHL=%04x\nSP=%04x\nPC=%04x\nMKL=02X\n"
                    "%c%c%c%c%c%c(%02x)",
                    //imem[0x31],imem[0x32],imem[0x33],imem[0x35],
                    VA,BC,DE,HL,SP,PC,MKL,
                    (PSW & Z ? 'Z': '-'),
                    (PSW & SK ? 'S': '-'),
                    (PSW & HC ? 'H': '-'),
                    (PSW & L1 ? '1': '-'),
                    (PSW & L0 ? '0': '-'),
                    (PSW & CY ? 'C': '-'),
                    PSW
                    );
        break;
    case 1:			// Log File
        sprintf(
                    Regs_String,
                    "IFF=%02X AF=%02x%02x BC=%04x DE=%04x HL=%04x SP=%04x PC=%04x V=%02x EOM=%02X "
                    "%c%c%c%c%c%c(%02x) ",
                    IFF,
                    A,PSW,BC,DE,HL,SP,PC,V,EOM,
                    (PSW & Z ? 'Z': '-'),
                    (PSW & SK ? 'S': '-'),
                    (PSW & HC ? 'H': '-'),
                    (PSW & L1 ? '1': '-'),
                    (PSW & L0 ? '0': '-'),
                    (PSW & CY ? 'C': '-'),
                    PSW
                    );
        for (int i=0;i<0x08;i++)
            sprintf(Regs_String,"%s%02X",Regs_String,cpustate->imem[i]);
        sprintf(Regs_String,"%s ",Regs_String);
        for (int i=0;i<0x08;i++)
            sprintf(Regs_String,"%s%02X",Regs_String,cpustate->imem[i+0x08]);
        break;
    }

//    char linebuf[100];
//    strcat(Regs_String," ,");
//    for (int i=0;i < 0x05;i++)
//    {
//        sprintf(linebuf,"%02x:[",i*0x10);
//        strcat(Regs_String,linebuf);
//        for (int j=0;j<0x10;j++)
//        {
//            sprintf(linebuf,"%02X ",imem[i*0x10 + j]);
//            strcat(Regs_String,linebuf);
//        }
//
//        strcat(Regs_String,"],");
//    }
#endif
}

bool Cupd7907::init()
{
    Check_Log();
    pDEBUG->init();
    Reset();

    upd7907stat.pPC = pPC;
    return true;

}

bool Cupd7907::exit()
{
}

void Cupd7907::step()
{
    {
        execute(&upd7907stat);
    }


}
Cupd7907::Cupd7907(CPObject *parent):CCPU(parent)
{

    pDEBUG = new Cdebug_upd7810(parent);
    fn_log="upd7907.log";


    //step_Previous_State = 0;

    regwidget = (CregCPU*) new Cregsz80Widget(0,this);
}

Cupd7907::~Cupd7907()
{
}
