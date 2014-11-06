/*****************************************************************************
 *
 *       i8085.c
 *       Portable I8085A emulator V1.2
 *
 *       Copyright (c) 1999 Juergen Buchmueller, all rights reserved.
 *       Partially based on information out of Z80Em by Marcel De Kogel
 *
 * changes in V1.2
 *       - corrected cycle counts for these classes of opcodes
 *         Thanks go to Jim Battle <frustum@pacbell.bet>
 *
 *                                      808x     Z80
 *         DEC A                   5       4    \
 *         INC A                   5       4     \
 *         LD A,B                  5       4      >-- Z80 is faster
 *         JP (HL)                 5       4     /
 *         CALL cc,nnnn: 11/17   10/17  /
 *
 *         INC HL                  5       6    \
 *         DEC HL                  5       6     \
 *         LD SP,HL        5       6      \
 *         ADD HL,BC      10      11       \
 *         INC (HL)       10      11            >-- 8080 is faster
 *         DEC (HL)       10      11       /
 *         IN A,(#)       10      11      /
 *         OUT (#),A      10      11     /
 *         EX (SP),HL     18      19    /
 *
 *       - This source code is released as freeware for non-commercial purposes.
 *       - You are free to use and redistribute this code in modified or
 *         unmodified form, provided you list me in the credits.
 *       - If you modify this source code, you must add a notice to each modified
 *         source file that it has been changed.  If you're a nice person, you
 *         will clearly mark each change too.  :)
 *       - If you wish to use this for commercial purposes, please contact me at
 *         pullmoll@t-online.de
 *       - The author of this copywritten work reserves the right to change the
 *         terms of its usage and license at any time, including retroactively
 *       - This entire notice must remain in the source code.
 *
 *****************************************************************************/
#include <QtCore>

#define I i85stat.regs

#define VERBOSE 0

#include "i8085.h"

#include "pcxxxx.h"
#include "Inter.h"
#include "i8085cpu.h"
#include "i8085daa.h"
#include "i8085_dasm.h"
#include "ui/cregsz80widget.h"

#define CPUTYPE_8080	0
#define CPUTYPE_8085	1

#define LOG(x)
#define IS_8080(c)			(i85stat.regs.cputype == CPUTYPE_8080)
#define IS_8085(c)			(i85stat.regs.cputype == CPUTYPE_8085)

Ci8085::Ci8085(CPObject * parent): CCPU(parent)
{
    fn_log="i8085.log";

    i85stat.regs.irq_callback = 0;
    i85stat.regs.sod_callback = 0;
    regwidget = (CregCPU*) new Cregsz80Widget(0,this);

}

Ci8085::~Ci8085()
{
}

bool Ci8085::init()
{
    init_tables(1);
    Check_Log();
    pDEBUG = new Cdebug_i8085(this);
    Reset();
    return true;
}

bool Ci8085::exit()
{
    /* nothing to do */
    return true;
}

void Ci8085::change_pc16(quint16 val) {

}

INLINE UINT8 Ci8085::get_rim_value(void)
{
    UINT8 result = i85stat.regs.IM;
    int sid = i85stat.regs.sid_state;

    /* copy live RST5.5 and RST6.5 states */
    result &= ~(IM_I65 | IM_I55);
    if (i85stat.regs.irq_state[I8085_RST65_LINE]) result |= IM_I65;
    if (i85stat.regs.irq_state[I8085_RST55_LINE]) result |= IM_I55;

    /* fetch the SID bit if we have a callback */
    result = (result & 0x7f) | (sid ? 0x80 : 0);

    return result;
}

INLINE  quint8 Ci8085::read8( quint16 address)
{
    return (((CpcXXXX *)pPC)->Get_8(address));
}
INLINE  void Ci8085::write8(quint16 address, quint8 value)
{
    ((CpcXXXX *)pPC)->Set_8(address,value);
}

/*
    16bits READ/WRITE
*/
INLINE quint16 Ci8085::read16( quint16 address)
{
    return ((CpcXXXX *)pPC)->Get_16(address);

}
INLINE void Ci8085::write16( quint16 address, quint16 value)
{
    ((CpcXXXX *)pPC)->Set_16(address,value);
}


INLINE quint8 Ci8085::cpu_readport(quint8 address) {
    return pPC->in(address);
}

INLINE void Ci8085::cpu_writeport(quint8 address, quint8 x) {
    imem[address] = x;
    pPC->out(address,x);
}

INLINE quint8 Ci8085::ROP(void)
{
        return read8(i85stat.regs.PC.w.l++);
}

INLINE quint8 Ci8085::ARG(void)
{
        return read8(i85stat.regs.PC.w.l++);
}

INLINE quint16 Ci8085::ARG16(void)
{
        quint16 w;
        w  = read8(i85stat.regs.PC.d);
        i85stat.regs.PC.w.l++;
        w += read8(i85stat.regs.PC.d) << 8;
        i85stat.regs.PC.w.l++;
        return w;
}

INLINE quint8 Ci8085::RM(quint32 a)
{
        return read8(a);
}

INLINE void Ci8085::WM(quint32 a, quint8 v)
{
        write8(a, v);
}

// conditional RET only
INLINE void Ci8085::M_RET(int cc)
{
    if (cc)
    {
        CallSubLevel--;
        i8085_ICount -= 6;
        M_POP(PC);
    }
}

 void Ci8085::illegal(void)
{
#if VERBOSE
        quint16 pc = i85stat.regs.PC.w.l - 1;
        LOG(("i8085 illegal instruction %04X $%02X\n", pc, cpu_readop(pc)));
#endif
}

INLINE void Ci8085::execute_one(int opcode)
{

    i8085_ICount -= lut_cycles[opcode];
    switch (opcode)
    {
        case 0x00:														break;	/* NOP  */
        case 0x01:	i85stat.regs.BC.w.l = ARG16();					break;	/* LXI  B,nnnn */
        case 0x02:	WM( i85stat.regs.BC.d, i85stat.regs.AF.b.h);		break;	/* STAX B */
        case 0x03:	i85stat.regs.BC.w.l++;											/* INX  B */
                    if (IS_8085()) { if (i85stat.regs.BC.w.l == 0x0000) i85stat.regs.AF.b.l |= X5F; else i85stat.regs.AF.b.l &= ~X5F; }
                    break;
        case 0x04:	M_INR(i85stat.regs.BC.b.h);							break;	/* INR  B */
        case 0x05:	M_DCR(i85stat.regs.BC.b.h);							break;	/* DCR  B */
        case 0x06:	M_MVI(i85stat.regs.BC.b.h);							break;	/* MVI  B,nn */
        case 0x07:	M_RLC;												break;	/* RLC  */

        case 0x08:	if (IS_8085()) { M_DSUB(); }				/* DSUB */
                    /* else { ; } */											/* NOP  undocumented */
                    break;
        case 0x09:	M_DAD(BC);											break;	/* DAD  B */
        case 0x0a:	i85stat.regs.AF.b.h = RM( i85stat.regs.BC.d);	break;	/* LDAX B */
        case 0x0b:	i85stat.regs.BC.w.l--;											/* DCX  B */
                    if (IS_8085()) { if (i85stat.regs.BC.w.l == 0xffff) i85stat.regs.AF.b.l |= X5F; else i85stat.regs.AF.b.l &= ~X5F; }
                    break;
        case 0x0c:	M_INR(i85stat.regs.BC.b.l);							break;	/* INR  C */
        case 0x0d:	M_DCR(i85stat.regs.BC.b.l);							break;	/* DCR  C */
        case 0x0e:	M_MVI(i85stat.regs.BC.b.l);							break;	/* MVI  C,nn */
        case 0x0f:	M_RRC;												break;	/* RRC  */

        case 0x10:	if (IS_8085()) {									/* ASRH */
                        i85stat.regs.AF.b.l = (i85stat.regs.AF.b.l & ~CF) | (i85stat.regs.HL.b.l & CF);
                        i85stat.regs.HL.w.l = (i85stat.regs.HL.w.l >> 1);
                    } /* else { ; } */											/* NOP  undocumented */
                    break;
        case 0x11:	i85stat.regs.DE.w.l = ARG16();					break;	/* LXI  D,nnnn */
        case 0x12:	WM( i85stat.regs.DE.d, i85stat.regs.AF.b.h);		break;	/* STAX D */
        case 0x13:	i85stat.regs.DE.w.l++;											/* INX  D */
                    if (IS_8085()) { if (i85stat.regs.DE.w.l == 0x0000) i85stat.regs.AF.b.l |= X5F; else i85stat.regs.AF.b.l &= ~X5F; }
                    break;
        case 0x14:	M_INR(i85stat.regs.DE.b.h);							break;	/* INR  D */
        case 0x15:	M_DCR(i85stat.regs.DE.b.h);							break;	/* DCR  D */
        case 0x16:	M_MVI(i85stat.regs.DE.b.h);							break;	/* MVI  D,nn */
        case 0x17:	M_RAL;												break;	/* RAL  */

        case 0x18:	if (IS_8085()) {									/* RLDE */
                        i85stat.regs.AF.b.l = (i85stat.regs.AF.b.l & ~(CF | VF)) | (i85stat.regs.DE.b.h >> 7);
                        i85stat.regs.DE.w.l = (i85stat.regs.DE.w.l << 1) | (i85stat.regs.DE.w.l >> 15);
                        if (0 != (((i85stat.regs.DE.w.l >> 15) ^ i85stat.regs.AF.b.l) & CF)) i85stat.regs.AF.b.l |= VF;
                    } /* else { ; } */											/* NOP  undocumented */
                    break;
        case 0x19:	M_DAD(DE);											break;	/* DAD  D */
        case 0x1a:	i85stat.regs.AF.b.h = RM( i85stat.regs.DE.d);	break;	/* LDAX D */
        case 0x1b:	i85stat.regs.DE.w.l--;											/* DCX  D */
                    if (IS_8085()) { if (i85stat.regs.DE.w.l == 0xffff) i85stat.regs.AF.b.l |= X5F; else i85stat.regs.AF.b.l &= ~X5F; }
                    break;
        case 0x1c:	M_INR(i85stat.regs.DE.b.l);							break;	/* INR  E */
        case 0x1d:	M_DCR(i85stat.regs.DE.b.l);							break;	/* DCR  E */
        case 0x1e:	M_MVI(i85stat.regs.DE.b.l);							break;	/* MVI  E,nn */
        case 0x1f:	M_RAR;												break;	/* RAR  */

        case 0x20:	if (IS_8085()) {									/* RIM  */
                        i85stat.regs.AF.b.h = get_rim_value();

                        /* if we have remembered state from taking a TRAP, fix up the IE flag here */
                        if (i85stat.regs.trap_im_copy & 0x80) i85stat.regs.AF.b.h = (i85stat.regs.AF.b.h & ~IM_IE) | (i85stat.regs.trap_im_copy & IM_IE);
                        i85stat.regs.trap_im_copy = 0;
                    } /* else { ; } */											/* NOP  undocumented */
                    break;
        case 0x21:	i85stat.regs.HL.w.l = ARG16();					break;	/* LXI  H,nnnn */
        case 0x22:	i85stat.regs.WZ.w.l = ARG16();							/* SHLD nnnn */
                    WM( i85stat.regs.WZ.d, i85stat.regs.HL.b.l); i85stat.regs.WZ.w.l++;
                    WM( i85stat.regs.WZ.d, i85stat.regs.HL.b.h);
                    break;
        case 0x23:	i85stat.regs.HL.w.l++;											/* INX  H */
                    if (IS_8085()) { if (i85stat.regs.HL.w.l == 0x0000) i85stat.regs.AF.b.l |= X5F; else i85stat.regs.AF.b.l &= ~X5F; }
                    break;
        case 0x24:	M_INR(i85stat.regs.HL.b.h);							break;	/* INR  H */
        case 0x25:	M_DCR(i85stat.regs.HL.b.h);							break;	/* DCR  H */
        case 0x26:	M_MVI(i85stat.regs.HL.b.h);							break;	/* MVI  H,nn */
        case 0x27:  i85stat.regs.WZ.d = i85stat.regs.AF.b.h;                    /* DAA */
                    if (i85stat.regs.AF.b.l & CF) i85stat.regs.WZ.d |= 0x100;
                    if (i85stat.regs.AF.b.l & HF) i85stat.regs.WZ.d |= 0x200;
                    if (i85stat.regs.AF.b.l & 0x02) i85stat.regs.WZ.d |= 0x400;
                    i85stat.regs.AF.w.l = DAA[i85stat.regs.WZ.d];
                    break;

        case 0x28:	if (IS_8085()) {									/* LDEH nn */
                        i85stat.regs.WZ.d = ARG();
                        i85stat.regs.DE.d = (i85stat.regs.HL.d + i85stat.regs.WZ.d) & 0xffff;
                    } /* else { ; } */											/* NOP  undocumented */
                    break;
        case 0x29:	M_DAD(HL);											break;	/* DAD  H */
        case 0x2a:	i85stat.regs.WZ.d = ARG16();							/* LHLD nnnn */
                    i85stat.regs.HL.b.l = RM( i85stat.regs.WZ.d); i85stat.regs.WZ.w.l++;
                    i85stat.regs.HL.b.h = RM( i85stat.regs.WZ.d);
                    break;
        case 0x2b:	i85stat.regs.HL.w.l--;											/* DCX  H */
                    if (IS_8085()) {
                        if (i85stat.regs.HL.w.l == 0xffff)
                            i85stat.regs.AF.b.l |= X5F;
                        else i85stat.regs.AF.b.l &= ~X5F;
                    }
                    break;
        case 0x2c:	M_INR(i85stat.regs.HL.b.l);							break;	/* INR  L */
        case 0x2d:	M_DCR(i85stat.regs.HL.b.l);							break;	/* DCR  L */
        case 0x2e:	M_MVI(i85stat.regs.HL.b.l);							break;	/* MVI  L,nn */
        case 0x2f:	i85stat.regs.AF.b.h ^= 0xff;									/* CMA  */
                    if (IS_8085()) i85stat.regs.AF.b.l |= HF | VF;
                    break;

        case 0x30:	if (IS_8085()) {									/* SIM  */
                        /* if bit 3 is set, bits 0-2 become the new masks */
                        if (i85stat.regs.AF.b.h & 0x08) {
                            i85stat.regs.IM &= ~(IM_M55 | IM_M65 | IM_M75 | IM_I55 | IM_I65);
                            i85stat.regs.IM |= i85stat.regs.AF.b.h & (IM_M55 | IM_M65 | IM_M75);

                            /* update live state based on the new masks */
                            if ((i85stat.regs.IM & IM_M55) == 0 && i85stat.regs.irq_state[I8085_RST55_LINE]) i85stat.regs.IM |= IM_I55;
                            if ((i85stat.regs.IM & IM_M65) == 0 && i85stat.regs.irq_state[I8085_RST65_LINE]) i85stat.regs.IM |= IM_I65;
                        }

                        /* bit if 4 is set, the 7.5 flip-flop is cleared */
                        if (i85stat.regs.AF.b.h & 0x10) i85stat.regs.IM &= ~IM_I75;

                        /* if bit 6 is set, then bit 7 is the new SOD state */
                        if (i85stat.regs.AF.b.h & 0x40) set_sod( i85stat.regs.AF.b.h >> 7);

                        /* check for revealed interrupts */
                        check_for_interrupts();
                    } /* else { ; } */											/* NOP  undocumented */
                    break;
        case 0x31:	i85stat.regs.SP.w.l = ARG16();					break;	/* LXI  SP,nnnn */
        case 0x32:	i85stat.regs.WZ.d = ARG16();							/* STAX nnnn */
                    WM( i85stat.regs.WZ.d, i85stat.regs.AF.b.h);
                    break;
        case 0x33:	i85stat.regs.SP.w.l++;											/* INX  SP */
                    if (IS_8085()) { if (i85stat.regs.SP.w.l == 0x0000) i85stat.regs.AF.b.l |= X5F; else i85stat.regs.AF.b.l &= ~X5F; }
                    break;
        case 0x34:	i85stat.regs.WZ.b.l = RM( i85stat.regs.HL.d);			/* INR  M */
                    M_INR(i85stat.regs.WZ.b.l);
                    WM( i85stat.regs.HL.d, i85stat.regs.WZ.b.l);
                    break;
        case 0x35:	i85stat.regs.WZ.b.l = RM( i85stat.regs.HL.d);			/* DCR  M */
                    M_DCR(i85stat.regs.WZ.b.l);
                    WM( i85stat.regs.HL.d, i85stat.regs.WZ.b.l);
                    break;
        case 0x36:	i85stat.regs.WZ.b.l = ARG();							/* MVI  M,nn */
                    WM( i85stat.regs.HL.d, i85stat.regs.WZ.b.l);
                    break;
        case 0x37:	i85stat.regs.AF.b.l = (i85stat.regs.AF.b.l & 0xfe) | CF;	break;	/* STC  */

        case 0x38:	if (IS_8085()) {									/* LDES nn */
                        i85stat.regs.WZ.d = ARG();
                        i85stat.regs.DE.d = (i85stat.regs.SP.d + i85stat.regs.WZ.d) & 0xffff;
                    } /* else { ; } */											/* NOP  undocumented */
                    break;
        case 0x39:	M_DAD(SP);											break;	/* DAD  SP */
        case 0x3a:	i85stat.regs.WZ.d = ARG16();							/* LDAX nnnn */
                    i85stat.regs.AF.b.h = RM( i85stat.regs.WZ.d);
                    break;
        case 0x3b:	i85stat.regs.SP.w.l--;											/* DCX  SP */
                    if (IS_8085()) { if (i85stat.regs.SP.w.l == 0xffff) i85stat.regs.AF.b.l |= X5F; else i85stat.regs.AF.b.l &= ~X5F; }
                    break;
        case 0x3c:	M_INR(i85stat.regs.AF.b.h);							break;	/* INR  A */
        case 0x3d:	M_DCR(i85stat.regs.AF.b.h);							break;	/* DCR  A */
        case 0x3e:	M_MVI(i85stat.regs.AF.b.h);							break;	/* MVI  A,nn */
        case 0x3f:	i85stat.regs.AF.b.l = (i85stat.regs.AF.b.l & 0xfe) | (~i85stat.regs.AF.b.l & CF); break; /* CMC  */

        case 0x40:														break;	/* MOV  B,B */
        case 0x41:	i85stat.regs.BC.b.h = i85stat.regs.BC.b.l;				break;	/* MOV  B,C */
        case 0x42:	i85stat.regs.BC.b.h = i85stat.regs.DE.b.h;				break;	/* MOV  B,D */
        case 0x43:	i85stat.regs.BC.b.h = i85stat.regs.DE.b.l;				break;	/* MOV  B,E */
        case 0x44:	i85stat.regs.BC.b.h = i85stat.regs.HL.b.h;				break;	/* MOV  B,H */
        case 0x45:	i85stat.regs.BC.b.h = i85stat.regs.HL.b.l;				break;	/* MOV  B,L */
        case 0x46:	i85stat.regs.BC.b.h = RM( i85stat.regs.HL.d);	break;	/* MOV  B,M */
        case 0x47:	i85stat.regs.BC.b.h = i85stat.regs.AF.b.h;				break;	/* MOV  B,A */

        case 0x48:	i85stat.regs.BC.b.l = i85stat.regs.BC.b.h;				break;	/* MOV  C,B */
        case 0x49:														break;	/* MOV  C,C */
        case 0x4a:	i85stat.regs.BC.b.l = i85stat.regs.DE.b.h;				break;	/* MOV  C,D */
        case 0x4b:	i85stat.regs.BC.b.l = i85stat.regs.DE.b.l;				break;	/* MOV  C,E */
        case 0x4c:	i85stat.regs.BC.b.l = i85stat.regs.HL.b.h;				break;	/* MOV  C,H */
        case 0x4d:	i85stat.regs.BC.b.l = i85stat.regs.HL.b.l;				break;	/* MOV  C,L */
        case 0x4e:	i85stat.regs.BC.b.l = RM( i85stat.regs.HL.d);	break;	/* MOV  C,M */
        case 0x4f:	i85stat.regs.BC.b.l = i85stat.regs.AF.b.h;				break;	/* MOV  C,A */

        case 0x50:	i85stat.regs.DE.b.h = i85stat.regs.BC.b.h;				break;	/* MOV  D,B */
        case 0x51:	i85stat.regs.DE.b.h = i85stat.regs.BC.b.l;				break;	/* MOV  D,C */
        case 0x52:														break;	/* MOV  D,D */
        case 0x53:	i85stat.regs.DE.b.h = i85stat.regs.DE.b.l;				break;	/* MOV  D,E */
        case 0x54:	i85stat.regs.DE.b.h = i85stat.regs.HL.b.h;				break;	/* MOV  D,H */
        case 0x55:	i85stat.regs.DE.b.h = i85stat.regs.HL.b.l;				break;	/* MOV  D,L */
        case 0x56:	i85stat.regs.DE.b.h = RM( i85stat.regs.HL.d);	break;	/* MOV  D,M */
        case 0x57:	i85stat.regs.DE.b.h = i85stat.regs.AF.b.h;				break;	/* MOV  D,A */

        case 0x58:	i85stat.regs.DE.b.l = i85stat.regs.BC.b.h;				break;	/* MOV  E,B */
        case 0x59:	i85stat.regs.DE.b.l = i85stat.regs.BC.b.l;				break;	/* MOV  E,C */
        case 0x5a:	i85stat.regs.DE.b.l = i85stat.regs.DE.b.h;				break;	/* MOV  E,D */
        case 0x5b:														break;	/* MOV  E,E */
        case 0x5c:	i85stat.regs.DE.b.l = i85stat.regs.HL.b.h;				break;	/* MOV  E,H */
        case 0x5d:	i85stat.regs.DE.b.l = i85stat.regs.HL.b.l;				break;	/* MOV  E,L */
        case 0x5e:	i85stat.regs.DE.b.l = RM( i85stat.regs.HL.d);	break;	/* MOV  E,M */
        case 0x5f:	i85stat.regs.DE.b.l = i85stat.regs.AF.b.h;				break;	/* MOV  E,A */

        case 0x60:	i85stat.regs.HL.b.h = i85stat.regs.BC.b.h;				break;	/* MOV  H,B */
        case 0x61:	i85stat.regs.HL.b.h = i85stat.regs.BC.b.l;				break;	/* MOV  H,C */
        case 0x62:	i85stat.regs.HL.b.h = i85stat.regs.DE.b.h;				break;	/* MOV  H,D */
        case 0x63:	i85stat.regs.HL.b.h = i85stat.regs.DE.b.l;				break;	/* MOV  H,E */
        case 0x64:														break;	/* MOV  H,H */
        case 0x65:	i85stat.regs.HL.b.h = i85stat.regs.HL.b.l;				break;	/* MOV  H,L */
        case 0x66:	i85stat.regs.HL.b.h = RM( i85stat.regs.HL.d);	break;	/* MOV  H,M */
        case 0x67:	i85stat.regs.HL.b.h = i85stat.regs.AF.b.h;				break;	/* MOV  H,A */

        case 0x68:	i85stat.regs.HL.b.l = i85stat.regs.BC.b.h;				break;	/* MOV  L,B */
        case 0x69:	i85stat.regs.HL.b.l = i85stat.regs.BC.b.l;				break;	/* MOV  L,C */
        case 0x6a:	i85stat.regs.HL.b.l = i85stat.regs.DE.b.h;				break;	/* MOV  L,D */
        case 0x6b:	i85stat.regs.HL.b.l = i85stat.regs.DE.b.l;				break;	/* MOV  L,E */
        case 0x6c:	i85stat.regs.HL.b.l = i85stat.regs.HL.b.h;				break;	/* MOV  L,H */
        case 0x6d:														break;	/* MOV  L,L */
        case 0x6e:	i85stat.regs.HL.b.l = RM( i85stat.regs.HL.d);	break;	/* MOV  L,M */
        case 0x6f:	i85stat.regs.HL.b.l = i85stat.regs.AF.b.h;				break;	/* MOV  L,A */

        case 0x70:	WM( i85stat.regs.HL.d, i85stat.regs.BC.b.h);		break;	/* MOV  M,B */
        case 0x71:	WM( i85stat.regs.HL.d, i85stat.regs.BC.b.l);		break;	/* MOV  M,C */
        case 0x72:	WM( i85stat.regs.HL.d, i85stat.regs.DE.b.h);		break;	/* MOV  M,D */
        case 0x73:	WM( i85stat.regs.HL.d, i85stat.regs.DE.b.l);		break;	/* MOV  M,E */
        case 0x74:	WM( i85stat.regs.HL.d, i85stat.regs.HL.b.h);		break;	/* MOV  M,H */
        case 0x75:	WM( i85stat.regs.HL.d, i85stat.regs.HL.b.l);		break;	/* MOV  M,L */
        case 0x76:	i85stat.regs.PC.w.l--; i85stat.regs.HALT = 1;						/* HLT */
                    set_status( 0x8a); // halt acknowledge
                    break;
        case 0x77:	WM( i85stat.regs.HL.d, i85stat.regs.AF.b.h);		break;	/* MOV  M,A */

        case 0x78:	i85stat.regs.AF.b.h = i85stat.regs.BC.b.h;				break;	/* MOV  A,B */
        case 0x79:	i85stat.regs.AF.b.h = i85stat.regs.BC.b.l;				break;	/* MOV  A,C */
        case 0x7a:	i85stat.regs.AF.b.h = i85stat.regs.DE.b.h;				break;	/* MOV  A,D */
        case 0x7b:	i85stat.regs.AF.b.h = i85stat.regs.DE.b.l;				break;	/* MOV  A,E */
        case 0x7c:	i85stat.regs.AF.b.h = i85stat.regs.HL.b.h;				break;	/* MOV  A,H */
        case 0x7d:	i85stat.regs.AF.b.h = i85stat.regs.HL.b.l;				break;	/* MOV  A,L */
        case 0x7e:	i85stat.regs.AF.b.h = RM( i85stat.regs.HL.d);	break;	/* MOV  A,M */
        case 0x7f:														break;	/* MOV  A,A */

        case 0x80:	M_ADD(i85stat.regs.BC.b.h);							break;	/* ADD  B */
        case 0x81:	M_ADD(i85stat.regs.BC.b.l);							break;	/* ADD  C */
        case 0x82:	M_ADD(i85stat.regs.DE.b.h);							break;	/* ADD  D */
        case 0x83:	M_ADD(i85stat.regs.DE.b.l);							break;	/* ADD  E */
        case 0x84:	M_ADD(i85stat.regs.HL.b.h);							break;	/* ADD  H */
        case 0x85:	M_ADD(i85stat.regs.HL.b.l);							break;	/* ADD  L */
        case 0x86:	i85stat.regs.WZ.b.l = RM( i85stat.regs.HL.d); M_ADD(i85stat.regs.WZ.b.l); break; /* ADD  M */
        case 0x87:	M_ADD(i85stat.regs.AF.b.h);							break;	/* ADD  A */

        case 0x88:	M_ADC(i85stat.regs.BC.b.h);							break;	/* ADC  B */
        case 0x89:	M_ADC(i85stat.regs.BC.b.l);							break;	/* ADC  C */
        case 0x8a:	M_ADC(i85stat.regs.DE.b.h);							break;	/* ADC  D */
        case 0x8b:	M_ADC(i85stat.regs.DE.b.l);							break;	/* ADC  E */
        case 0x8c:	M_ADC(i85stat.regs.HL.b.h);							break;	/* ADC  H */
        case 0x8d:	M_ADC(i85stat.regs.HL.b.l);							break;	/* ADC  L */
        case 0x8e:	i85stat.regs.WZ.b.l = RM( i85stat.regs.HL.d); M_ADC(i85stat.regs.WZ.b.l); break; /* ADC  M */
        case 0x8f:	M_ADC(i85stat.regs.AF.b.h);							break;	/* ADC  A */

        case 0x90:	M_SUB(i85stat.regs.BC.b.h);							break;	/* SUB  B */
        case 0x91:	M_SUB(i85stat.regs.BC.b.l);							break;	/* SUB  C */
        case 0x92:	M_SUB(i85stat.regs.DE.b.h);							break;	/* SUB  D */
        case 0x93:	M_SUB(i85stat.regs.DE.b.l);							break;	/* SUB  E */
        case 0x94:	M_SUB(i85stat.regs.HL.b.h);							break;	/* SUB  H */
        case 0x95:	M_SUB(i85stat.regs.HL.b.l);							break;	/* SUB  L */
        case 0x96:	i85stat.regs.WZ.b.l = RM( i85stat.regs.HL.d); M_SUB(i85stat.regs.WZ.b.l); break; /* SUB  M */
        case 0x97:	M_SUB(i85stat.regs.AF.b.h);							break;	/* SUB  A */

        case 0x98:	M_SBB(i85stat.regs.BC.b.h);							break;	/* SBB  B */
        case 0x99:	M_SBB(i85stat.regs.BC.b.l);							break;	/* SBB  C */
        case 0x9a:	M_SBB(i85stat.regs.DE.b.h);							break;	/* SBB  D */
        case 0x9b:	M_SBB(i85stat.regs.DE.b.l);							break;	/* SBB  E */
        case 0x9c:	M_SBB(i85stat.regs.HL.b.h);							break;	/* SBB  H */
        case 0x9d:	M_SBB(i85stat.regs.HL.b.l);							break;	/* SBB  L */
        case 0x9e:	i85stat.regs.WZ.b.l = RM( i85stat.regs.HL.d); M_SBB(i85stat.regs.WZ.b.l); break; /* SBB  M */
        case 0x9f:	M_SBB(i85stat.regs.AF.b.h);							break;	/* SBB  A */

        case 0xa0:	M_ANA(i85stat.regs.BC.b.h);							break;	/* ANA  B */
        case 0xa1:	M_ANA(i85stat.regs.BC.b.l);							break;	/* ANA  C */
        case 0xa2:	M_ANA(i85stat.regs.DE.b.h);							break;	/* ANA  D */
        case 0xa3:	M_ANA(i85stat.regs.DE.b.l);							break;	/* ANA  E */
        case 0xa4:	M_ANA(i85stat.regs.HL.b.h);							break;	/* ANA  H */
        case 0xa5:	M_ANA(i85stat.regs.HL.b.l);							break;	/* ANA  L */
        case 0xa6:	i85stat.regs.WZ.b.l = RM( i85stat.regs.HL.d); M_ANA(i85stat.regs.WZ.b.l); break; /* ANA  M */
        case 0xa7:	M_ANA(i85stat.regs.AF.b.h);							break;	/* ANA  A */

        case 0xa8:	M_XRA(i85stat.regs.BC.b.h);							break;	/* XRA  B */
        case 0xa9:	M_XRA(i85stat.regs.BC.b.l);							break;	/* XRA  C */
        case 0xaa:	M_XRA(i85stat.regs.DE.b.h);							break;	/* XRA  D */
        case 0xab:	M_XRA(i85stat.regs.DE.b.l);							break;	/* XRA  E */
        case 0xac:	M_XRA(i85stat.regs.HL.b.h);							break;	/* XRA  H */
        case 0xad:	M_XRA(i85stat.regs.HL.b.l);							break;	/* XRA  L */
        case 0xae:	i85stat.regs.WZ.b.l = RM( i85stat.regs.HL.d); M_XRA(i85stat.regs.WZ.b.l); break; /* XRA  M */
        case 0xaf:	M_XRA(i85stat.regs.AF.b.h);							break;	/* XRA  A */

        case 0xb0:	M_ORA(i85stat.regs.BC.b.h);							break;	/* ORA  B */
        case 0xb1:	M_ORA(i85stat.regs.BC.b.l);							break;	/* ORA  C */
        case 0xb2:	M_ORA(i85stat.regs.DE.b.h);							break;	/* ORA  D */
        case 0xb3:	M_ORA(i85stat.regs.DE.b.l);							break;	/* ORA  E */
        case 0xb4:	M_ORA(i85stat.regs.HL.b.h);							break;	/* ORA  H */
        case 0xb5:	M_ORA(i85stat.regs.HL.b.l);							break;	/* ORA  L */
        case 0xb6:	i85stat.regs.WZ.b.l = RM( i85stat.regs.HL.d); M_ORA(i85stat.regs.WZ.b.l); break; /* ORA  M */
        case 0xb7:	M_ORA(i85stat.regs.AF.b.h);							break;	/* ORA  A */

        case 0xb8:	M_CMP(i85stat.regs.BC.b.h);							break;	/* CMP  B */
        case 0xb9:	M_CMP(i85stat.regs.BC.b.l);							break;	/* CMP  C */
        case 0xba:	M_CMP(i85stat.regs.DE.b.h);							break;	/* CMP  D */
        case 0xbb:	M_CMP(i85stat.regs.DE.b.l);							break;	/* CMP  E */
        case 0xbc:	M_CMP(i85stat.regs.HL.b.h);							break;	/* CMP  H */
        case 0xbd:	M_CMP(i85stat.regs.HL.b.l);							break;	/* CMP  L */
        case 0xbe:	i85stat.regs.WZ.b.l = RM( i85stat.regs.HL.d); M_CMP(i85stat.regs.WZ.b.l); break; /* CMP  M */
        case 0xbf:	M_CMP(i85stat.regs.AF.b.h);							break;	/* CMP  A */

        case 0xc0:	M_RET( !(i85stat.regs.AF.b.l & ZF) );					break;	/* RNZ  */
        case 0xc1:	M_POP(BC);											break;	/* POP  B */
        case 0xc2:	M_JMP( !(i85stat.regs.AF.b.l & ZF) );					break;	/* JNZ  nnnn */
        case 0xc3:	M_JMP(1);											break;	/* JMP  nnnn */
        case 0xc4:	M_CALL( !(i85stat.regs.AF.b.l & ZF) );					break;	/* CNZ  nnnn */
        case 0xc5:	M_PUSH(BC);											break;	/* PUSH B */
        case 0xc6:	i85stat.regs.WZ.b.l = ARG(); M_ADD(i85stat.regs.WZ.b.l); break; /* ADI  nn */
        case 0xc7:	M_RST(0);											break;	/* RST  0 */

        case 0xc8:	M_RET( i85stat.regs.AF.b.l & ZF );						break;	/* RZ   */
        case 0xc9:	M_RET( 1 );                             break;	/* RET  */
        case 0xca:	M_JMP( i85stat.regs.AF.b.l & ZF );						break;	/* JZ   nnnn */
        case 0xcb:	if (IS_8085()) {									/* RST  V */
                        if (i85stat.regs.AF.b.l & VF) { M_RST(8); }
                        else i8085_ICount += 6; // RST not taken
                    } else { M_JMP(1); }										/* JMP  nnnn undocumented */
                    break;
        case 0xcc:	M_CALL( i85stat.regs.AF.b.l & ZF );					break;	/* CZ   nnnn */
        case 0xcd:	M_CALL(1);											break;	/* CALL nnnn */
        case 0xce:	i85stat.regs.WZ.b.l = ARG(); M_ADC(i85stat.regs.WZ.b.l); break; /* ACI  nn */
        case 0xcf:	M_RST(1);											break;	/* RST  1 */

        case 0xd0:	M_RET( !(i85stat.regs.AF.b.l & CF) );					break;	/* RNC  */
        case 0xd1:	M_POP(DE);											break;	/* POP  D */
        case 0xd2:	M_JMP( !(i85stat.regs.AF.b.l & CF) );					break;	/* JNC  nnnn */
        case 0xd3:	M_OUT;												break;	/* OUT  nn */
        case 0xd4:	M_CALL( !(i85stat.regs.AF.b.l & CF) );					break;	/* CNC  nnnn */
        case 0xd5:	M_PUSH(DE);											break;	/* PUSH D */
        case 0xd6:	i85stat.regs.WZ.b.l = ARG(); M_SUB(i85stat.regs.WZ.b.l); break; /* SUI  nn */
        case 0xd7:	M_RST(2);											break;	/* RST  2 */

        case 0xd8:	M_RET( i85stat.regs.AF.b.l & CF );						break;	/* RC   */
        case 0xd9:	if (IS_8085()) {									/* SHLX */
                        i85stat.regs.WZ.w.l = i85stat.regs.DE.w.l;
                        WM( i85stat.regs.WZ.d, i85stat.regs.HL.b.l); i85stat.regs.WZ.w.l++;
                        WM( i85stat.regs.WZ.d, i85stat.regs.HL.b.h);
                    } else { M_POP(PC); CallSubLevel--;}										/* RET  undocumented */
                    break;
        case 0xda:	M_JMP( i85stat.regs.AF.b.l & CF );						break;	/* JC   nnnn */
        case 0xdb:	M_IN;												break;	/* IN   nn */
        case 0xdc:	M_CALL( i85stat.regs.AF.b.l & CF );					break;	/* CC   nnnn */
        case 0xdd:	if (IS_8085()) { M_JMP( !(i85stat.regs.AF.b.l & X5F) ); } /* JNX  nnnn */
                    else { M_CALL(1); }											/* CALL nnnn undocumented */
                    break;
        case 0xde:	i85stat.regs.WZ.b.l = ARG(); M_SBB(i85stat.regs.WZ.b.l); break; /* SBI  nn */
        case 0xdf:	M_RST(3);											break;	/* RST  3 */

        case 0xe0:	M_RET( !(i85stat.regs.AF.b.l & PF) );					break;	/* RPO    */
        case 0xe1:	M_POP(HL);											break;	/* POP  H */
        case 0xe2:	M_JMP( !(i85stat.regs.AF.b.l & PF) );					break;	/* JPO  nnnn */
        case 0xe3:	M_POP(WZ); M_PUSH(HL);										/* XTHL */
                    i85stat.regs.HL.d = i85stat.regs.WZ.d;
                    break;
        case 0xe4:	M_CALL( !(i85stat.regs.AF.b.l & PF) );					break;	/* CPO  nnnn */
        case 0xe5:	M_PUSH(HL);											break;	/* PUSH H */
        case 0xe6:	i85stat.regs.WZ.b.l = ARG(); M_ANA(i85stat.regs.WZ.b.l); break; /* ANI  nn */
        case 0xe7:	M_RST(4);											break;	/* RST  4 */

        case 0xe8:	M_RET( i85stat.regs.AF.b.l & PF );						break;	/* RPE  */
        case 0xe9:	i85stat.regs.PC.d = i85stat.regs.HL.w.l;					break;	/* PCHL */
        case 0xea:	M_JMP( i85stat.regs.AF.b.l & PF );						break;	/* JPE  nnnn */
        case 0xeb:	i85stat.regs.WZ.d = i85stat.regs.DE.d;							/* XCHG */
                    i85stat.regs.DE.d = i85stat.regs.HL.d;
                    i85stat.regs.HL.d = i85stat.regs.WZ.d;
                    break;
        case 0xec:	M_CALL( i85stat.regs.AF.b.l & PF );					break;	/* CPE  nnnn */
        case 0xed:	if (IS_8085()) {									/* LHLX */
                        i85stat.regs.WZ.w.l = i85stat.regs.DE.w.l;
                        i85stat.regs.HL.b.l = RM( i85stat.regs.WZ.d); i85stat.regs.WZ.w.l++;
                        i85stat.regs.HL.b.h = RM( i85stat.regs.WZ.d);
                    } else { M_CALL(1); }										/* CALL nnnn undocumented */
                    break;
        case 0xee:	i85stat.regs.WZ.b.l = ARG(); M_XRA(i85stat.regs.WZ.b.l); break; /* XRI  nn */
        case 0xef:	M_RST(5);											break;	/* RST  5 */

        case 0xf0:	M_RET( !(i85stat.regs.AF.b.l&SF) );					break;	/* RP   */
        case 0xf1:	M_POP(AF);											break;	/* POP  A */
        case 0xf2:	M_JMP( !(i85stat.regs.AF.b.l & SF) );					break;	/* JP   nnnn */
        case 0xf3:	set_inte( 0);								break;	/* DI   */
        case 0xf4:	M_CALL( !(i85stat.regs.AF.b.l & SF) );					break;	/* CP   nnnn */
        case 0xf5:	M_PUSH(AF);											break;	/* PUSH A */
        case 0xf6:	i85stat.regs.WZ.b.l = ARG(); M_ORA(i85stat.regs.WZ.b.l); break; /* ORI  nn */
        case 0xf7:	M_RST(6);											break;	/* RST  6 */

        case 0xf8:	M_RET( i85stat.regs.AF.b.l & SF );						break;	/* RM   */
        case 0xf9:	i85stat.regs.SP.d = i85stat.regs.HL.d;					break;	/* SPHL */
        case 0xfa:	M_JMP( i85stat.regs.AF.b.l & SF );						break;	/* JM   nnnn */
        case 0xfb:	set_inte( 1); i85stat.regs.after_ei = 2;		break;	/* EI */
        case 0xfc:	M_CALL( i85stat.regs.AF.b.l & SF );					break;	/* CM   nnnn */
        case 0xfd:	if (IS_8085()) { M_JMP( i85stat.regs.AF.b.l & X5F ); }	/* JX   nnnn */
                    else { M_CALL(1); }											/* CALL nnnn undocumented */
                    break;
        case 0xfe:	i85stat.regs.WZ.b.l = ARG(); M_CMP(i85stat.regs.WZ.b.l); break; /* CPI  nn */
        case 0xff:	M_RST(7);											break;	/* RST  7 */
    }
}

INLINE void Ci8085::break_halt_for_interrupt(void)
{
    /* de-halt if necessary */
    if (i85stat.regs.HALT)
    {
        i85stat.regs.PC.w.l++;
        i85stat.regs.HALT = 0;
        set_status( 0x26);	/* int ack while halt */
    }
    else
        set_status( 0x23);	/* int ack */
}

INLINE void Ci8085::set_status(UINT8 status)
{
//	if (status != i85stat.regs.STATUS)
//		i85stat.regs.out_status_func(0, status);

    i85stat.regs.STATUS = status;
}
INLINE void Ci8085::set_inte(int state)
{
    if (state != 0 && (i85stat.regs.IM & IM_IE) == 0)
    {
        i85stat.regs.IM |= IM_IE;
//        i85stat.regs.out_inte_func(1);
    }
    else if (state == 0 && (i85stat.regs.IM & IM_IE) != 0)
    {
        i85stat.regs.IM &= ~IM_IE;
//        i85stat.regs.out_inte_func(0);
    }
}
INLINE void Ci8085::set_sod(int state)
{
    if (state != 0 && i85stat.regs.sod_state == 0)
    {
        i85stat.regs.sod_state = 1;
//		i85stat.regs.out_sod_func(i85stat.regs.sod_state);
    }
    else if (state == 0 && i85stat.regs.sod_state != 0)
    {
        i85stat.regs.sod_state = 0;
//		i85stat.regs.out_sod_func(i85stat.regs.sod_state);
    }
}
void Ci8085::check_for_interrupts(void)
{
    /* TRAP is the highest priority */
    if (i85stat.regs.trap_pending)
    {
        /* the first RIM after a TRAP reflects the original IE state; remember it here,
           setting the high bit to indicate it is valid */
        i85stat.regs.trap_im_copy = i85stat.regs.IM | 0x80;

        /* reset the pending state */
        i85stat.regs.trap_pending = false;

        /* break out of HALT state and call the IRQ ack callback */
        break_halt_for_interrupt();
//        if (i85stat.regs.irq_callback != NULL)
//            (*i85stat.regs.irq_callback)(i85stat.regs.device, INPUT_LINE_NMI);

        /* push the PC and jump to $0024 */
        M_PUSH(PC);
        CallSubLevel++;
        set_inte(0);
        i85stat.regs.PC.w.l = ADDR_TRAP;
        i8085_ICount -= 11;
    }

    /* followed by RST7.5 */
    else if ((i85stat.regs.IM & IM_I75) && !(i85stat.regs.IM & IM_M75) && (i85stat.regs.IM & IM_IE))
    {
        /* reset the pending state (which is CPU-visible via the RIM instruction) */
        i85stat.regs.IM &= ~IM_I75;

        /* break out of HALT state and call the IRQ ack callback */
        break_halt_for_interrupt();
//        if (i85stat.regs.irq_callback != NULL)
//            (*i85stat.regs.irq_callback)(i85stat.regs.device, I8085_RST75_LINE);

        /* push the PC and jump to $003C */
        M_PUSH(PC);
        CallSubLevel++;
        set_inte( 0);
        i85stat.regs.PC.w.l = ADDR_RST75;
        i8085_ICount -= 11;
    }

    /* followed by RST6.5 */
    else if (i85stat.regs.irq_state[I8085_RST65_LINE] && !(i85stat.regs.IM & IM_M65) && (i85stat.regs.IM & IM_IE))
    {
        /* break out of HALT state and call the IRQ ack callback */
        break_halt_for_interrupt();
//        if (i85stat.regs.irq_callback != NULL)
//            (*i85stat.regs.irq_callback)(i85stat.regs.device, I8085_RST65_LINE);

        /* push the PC and jump to $0034 */
        M_PUSH(PC);
        CallSubLevel++;
        set_inte( 0);
        i85stat.regs.PC.w.l = ADDR_RST65;
        i8085_ICount -= 11;
    }

    /* followed by RST5.5 */
    else if (i85stat.regs.irq_state[I8085_RST55_LINE] && !(i85stat.regs.IM & IM_M55) && (i85stat.regs.IM & IM_IE))
    {
        /* break out of HALT state and call the IRQ ack callback */
        break_halt_for_interrupt();
//        if (i85stat.regs.irq_callback != NULL)
//            (*i85stat.regs.irq_callback)(i85stat.regs.device, I8085_RST55_LINE);

        /* push the PC and jump to $002C */
        M_PUSH(PC);
        CallSubLevel++;
        set_inte( 0);
        i85stat.regs.PC.w.l = ADDR_RST55;
        i8085_ICount -= 11;
    }

    /* followed by classic INTR */
    else if (i85stat.regs.irq_state[I8085_INTR_LINE] && (i85stat.regs.IM & IM_IE))
    {
        UINT32 vector = 0;

        /* break out of HALT state and call the IRQ ack callback */
        break_halt_for_interrupt();
//        if (i85stat.regs.irq_callback != NULL)
//            vector = (*i85stat.regs.irq_callback)(i85stat.regs.device, I8085_INTR_LINE);

        /* use the resulting vector as an opcode to execute */
        set_inte( 0);
        switch (vector & 0xff0000)
        {
            case 0xcd0000:	/* CALL nnnn */
                i8085_ICount -= 7;
                M_PUSH(PC);
                CallSubLevel++;
            case 0xc30000:	/* JMP  nnnn */
                i8085_ICount -= 10;
                i85stat.regs.PC.d = vector & 0xffff;
                break;

            default:
                LOG(("i8085 take int $%02x\n", vector));
                execute_one( vector & 0xff);
                break;
        }
    }
}

int Ci8085::i8085_execute(int cycles)
{

    i8085_ICount = cycles;

    /* check for TRAPs before diving in (can't do others because of after_ei) */
    if (i85stat.regs.trap_pending || i85stat.regs.after_ei == 0)
        check_for_interrupts();

    do
    {
//        debugger_instruction_hook(device, i85stat.regs.PC.d);

        /* the instruction after an EI does not take an interrupt, so
           we cannot check immediately; handle post-EI behavior here */
        if (i85stat.regs.after_ei != 0 && --i85stat.regs.after_ei == 0)
            check_for_interrupts();

        /* here we go... */
        execute_one( ROP());

        } while (i8085_ICount > 0);

        return cycles - i8085_ICount;
}

/****************************************************************************
 * Initialise the various lookup tables used by the emulation code
 ****************************************************************************/
void Ci8085::init_tables (int type)
{
        quint8 zs;
        int i, p;
        for (i = 0; i < 256; i++)
        {
            /* cycles */
                    lut_cycles[i] = type?lut_cycles_8085[i]:lut_cycles_8080[i];

                zs = 0;
                if (i==0) zs |= ZF;
                if (i&128) zs |= SF;
                p = 0;
                if (i&1) ++p;
                if (i&2) ++p;
                if (i&4) ++p;
                if (i&8) ++p;
                if (i&16) ++p;
                if (i&32) ++p;
                if (i&64) ++p;
                if (i&128) ++p;
                ZS[i] = zs;
                ZSP[i] = zs | ((p&1) ? 0 : VF);
        }
}



/****************************************************************************
 * Set the current 8085 PC
 ****************************************************************************/
void Ci8085::i8085_set_pc(unsigned val)
{
        i85stat.regs.PC.w.l = val;
        //change_pc(i85stat.regs.PC.d);
}

/****************************************************************************
 * Get the current 8085 SP
 ****************************************************************************/
unsigned Ci8085::i8085_get_sp(void)
{
        return i85stat.regs.SP.d;
}

/****************************************************************************
 * Set the current 8085 SP
 ****************************************************************************/
void Ci8085::i8085_set_sp(unsigned val)
{
        i85stat.regs.SP.w.l = val;
}

/****************************************************************************
 * Get a specific register
 ****************************************************************************/
#define REG_SP_CONTENTS -2
unsigned Ci8085::i8085_get_reg(int regnum)
{
        switch( regnum )
        {
                case I8085_PC: return i85stat.regs.PC.w.l;
                case I8085_SP: return i85stat.regs.SP.w.l;
                case I8085_AF: return i85stat.regs.AF.w.l;
                case I8085_BC: return i85stat.regs.BC.w.l;
                case I8085_DE: return i85stat.regs.DE.w.l;
                case I8085_HL: return i85stat.regs.HL.w.l;
                case I8085_IM: return i85stat.regs.IM;
                case I8085_HALT: return i85stat.regs.HALT;
                case I8085_IREQ: return i85stat.regs.IREQ;
                case I8085_ISRV: return i85stat.regs.ISRV;
                case I8085_VECTOR: return i85stat.regs.INTR;
                case I8085_TRAP_STATE: return i85stat.regs.nmi_state;
                case I8085_INTR_STATE: return i85stat.regs.irq_state[I8085_INTR_LINE];
                case I8085_RST55_STATE: return i85stat.regs.irq_state[I8085_RST55_LINE];
                case I8085_RST65_STATE: return i85stat.regs.irq_state[I8085_RST65_LINE];
                case I8085_RST75_STATE: return i85stat.regs.irq_state[I8085_RST75_LINE];
                case -1: return 0; /* previous pc not supported */
                default:
                        if( regnum <= REG_SP_CONTENTS )
                        {
                                unsigned offset = i85stat.regs.SP.w.l + 2 * (REG_SP_CONTENTS - regnum);
                                if( offset < 0xffff )
                                        return RM( offset ) + ( RM( offset+1 ) << 8 );
                        }
        }
        return 0;
}

/****************************************************************************
 * Set a specific register
 ****************************************************************************/
void Ci8085::i8085_set_reg(int regnum, unsigned val)
{
        switch( regnum )
        {
                case I8085_PC: i85stat.regs.PC.w.l = val; break;
                case I8085_SP: i85stat.regs.SP.w.l = val; break;
                case I8085_AF: i85stat.regs.AF.w.l = val; break;
                case I8085_BC: i85stat.regs.BC.w.l = val; break;
                case I8085_DE: i85stat.regs.DE.w.l = val; break;
                case I8085_HL: i85stat.regs.HL.w.l = val; break;
                case I8085_IM: i85stat.regs.IM = val; break;
                case I8085_HALT: i85stat.regs.HALT = val; break;
                case I8085_IREQ: i85stat.regs.IREQ = val; break;
                case I8085_ISRV: i85stat.regs.ISRV = val; break;
                case I8085_VECTOR: i85stat.regs.INTR = val; break;
                case I8085_TRAP_STATE: i85stat.regs.nmi_state = val; break;
                case I8085_INTR_STATE: i85stat.regs.irq_state[I8085_INTR_LINE] = val; break;
                case I8085_RST55_STATE: i85stat.regs.irq_state[I8085_RST55_LINE] = val; break;
                case I8085_RST65_STATE: i85stat.regs.irq_state[I8085_RST65_LINE] = val; break;
                case I8085_RST75_STATE: i85stat.regs.irq_state[I8085_RST75_LINE] = val; break;
                default:
                        if( regnum <= REG_SP_CONTENTS )
                        {
                                unsigned offset = i85stat.regs.SP.w.l + 2 * (REG_SP_CONTENTS - regnum);
                                if( offset < 0xffff )
                                {
                                        WM( offset, val&0xff );
                                        WM( offset+1, (val>>8)&0xff );
                                }
                        }
        }
}

/****************************************************************************/
/* Set the 8085 SID input signal state                                                                          */
/****************************************************************************/
void Ci8085::i8085_set_SID(int state)
{        
    i85stat.regs.sid_state = state;
}

/****************************************************************************/
/* Set a callback to be called at SOD output change                                             */
/****************************************************************************/
//void i8085_set_sod_callback(void (*callback)(int state))
//{
//        i85stat.regs.sod_callback = callback;
//}






void Ci8085::i8085_set_irq_line(int irqline, int state)
{
    int newstate = (state != CLEAR_LINE);

    /* NMI is edge-triggered */
//    if (irqline == INPUT_LINE_NMI)
//    {
//        if (!i85stat.regs.nmi_state && newstate)
//            i85stat.regs.trap_pending = TRUE;
//        i85stat.regs.nmi_state = newstate;
//    }

    /* RST7.5 is edge-triggered */
//    else
    if (irqline == I8085_RST75_LINE)
    {
        if (!i85stat.regs.irq_state[I8085_RST75_LINE] && newstate)
            i85stat.regs.IM |= IM_I75;
        i85stat.regs.irq_state[I8085_RST75_LINE] = newstate;
    }

    /* remaining sources are level triggered */
    else if (irqline < sizeof(i85stat.regs.irq_state))
        i85stat.regs.irq_state[irqline] = state;
}

//void Ci8085::i8085_set_irq_callback(int (*callback)(int))
//{
//        i85stat.regs.irq_callback = callback;
//}




void Ci8085::step()
{

    pPC->pTIMER->state += i8085_execute(0);

}

void Ci8085::Reset()
{
    init_tables(CPUTYPE_8085);
    memset(&I, 0, sizeof(I));
    i85stat.regs.cputype = 1;
    change_pc16(i85stat.regs.PC.d);

    i85stat.regs.PC.d = 0;
    i85stat.regs.HALT = 0;
    i85stat.regs.IM &= ~IM_I75;
    i85stat.regs.IM |= IM_M55 | IM_M65 | IM_M75;
    i85stat.regs.after_ei = false;
    i85stat.regs.trap_pending = false;
    i85stat.regs.trap_im_copy = 0;
    set_inte( 0);
    set_sod( 0);
}



void Ci8085::Load_Internal(QXmlStreamReader *)
{
}

void Ci8085::save_internal(QXmlStreamWriter *)
{
}

UINT32 Ci8085::get_PC()
{
    return i85stat.regs.PC.d;
}

void Ci8085::Regs_Info(quint8)
{
    sprintf(Regs_String,"EMPTY");
    char buf[32];
#if 0
    sprintf(
    Regs_String,
    "IM=%02X IREQ=%02X ISRV=%02X IRQ2=%08X PC=%04X SP=%04X AF=%04X BC=%04X DE=%04X HL=%04X WZ=%04X ",
                i85stat.regs.IM,
                i85stat.regs.IREQ,
                i85stat.regs.ISRV,
                i85stat.regs.IRQ2,
     i85stat.regs.PC.d,
                i85stat.regs.SP.d,
                i85stat.regs.AF.d,
                i85stat.regs.BC.d,
                i85stat.regs.DE.d,
                i85stat.regs.HL.d,
                i85stat.regs.WZ.d
    );
#else
    // Format flags string
        char flags[20];
        sprintf(flags, "%c%c%c%c%c%c%c%c",
                (i85stat.regs.AF.b.l&0x80)?'S':' ',
                (i85stat.regs.AF.b.l&0x40)?'Z':' ',
                (i85stat.regs.AF.b.l&0x08)?'T':' ',
                (i85stat.regs.AF.b.l&0x10)?'A':' ',
                (i85stat.regs.AF.b.l&0x04)?'P':' ',
                (i85stat.regs.AF.b.l&0x02)?'O':' ',
                (i85stat.regs.AF.b.l&0x20)?'X':' ',
                (i85stat.regs.AF.b.l&0x01)?'C':' ');

        // Append regs after opcode
        sprintf(Regs_String, "A:%02X %s B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%02X",
            i85stat.regs.AF.b.h,
                flags,
                i85stat.regs.BC.b.h,
                i85stat.regs.BC.b.l,
                i85stat.regs.DE.b.h,
                i85stat.regs.DE.b.l,
                i85stat.regs.HL.b.h,
                i85stat.regs.HL.b.l,
                i85stat.regs.SP.b);

#endif
}

/* cycles lookup */
const UINT8 Ci8085::lut_cycles_8080[256]={
/*      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  */
/* 0 */ 4, 10,7, 5, 5, 5, 7, 4, 4, 10,7, 5, 5, 5, 7, 4,
/* 1 */ 4, 10,7, 5, 5, 5, 7, 4, 4, 10,7, 5, 5, 5, 7, 4,
/* 2 */ 4, 10,16,5, 5, 5, 7, 4, 4, 10,16,5, 5, 5, 7, 4,
/* 3 */ 4, 10,13,5, 10,10,10,4, 4, 10,13,5, 5, 5, 7, 4,
/* 4 */ 5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
/* 5 */ 5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
/* 6 */ 5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
/* 7 */ 7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 7, 5,
/* 8 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* 9 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* A */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* B */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* C */ 5, 10,10,10,11,11,7, 11,5, 10,10,10,11,11,7, 11,
/* D */ 5, 10,10,10,11,11,7, 11,5, 10,10,10,11,11,7, 11,
/* E */ 5, 10,10,18,11,11,7, 11,5, 5, 10,5, 11,11,7, 11,
/* F */ 5, 10,10,4, 11,11,7, 11,5, 5, 10,4, 11,11,7, 11 };
const UINT8 Ci8085::lut_cycles_8085[256]={
/*      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  */
/* 0 */ 4, 10,7, 6, 4, 4, 7, 4, 10,10,7, 6, 4, 4, 7, 4,
/* 1 */ 7, 10,7, 6, 4, 4, 7, 4, 10,10,7, 6, 4, 4, 7, 4,
/* 2 */ 7, 10,16,6, 4, 4, 7, 4, 10,10,16,6, 4, 4, 7, 4,
/* 3 */ 7, 10,13,6, 10,10,10,4, 10,10,13,6, 4, 4, 7, 4,
/* 4 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* 5 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* 6 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* 7 */ 7, 7, 7, 7, 7, 7, 5, 7, 4, 4, 4, 4, 4, 4, 7, 4,
/* 8 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* 9 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* A */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* B */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* C */ 6, 10,10,10,11,12,7, 12,6, 10,10,12,11,11,7, 12,
/* D */ 6, 10,10,10,11,12,7, 12,6, 10,10,10,11,10,7, 12,
/* E */ 6, 10,10,16,11,12,7, 12,6, 6, 10,5, 11,10,7, 12,
/* F */ 6, 10,10,4, 11,12,7, 12,6, 6, 10,4, 11,10,7, 12 };

/* special cases (partially taken care of elsewhere):
               base c    taken?   not taken?
M_RET  8080    5         +6(11)   -0            (conditional)
M_RET  8085    6         +6(12)   -0            (conditional)
M_JMP  8080    10        +0       -0
M_JMP  8085    10        +0       -3(7)
M_CALL 8080    11        +6(17)   -0
M_CALL 8085    11        +7(18)   -2(9)

*/
