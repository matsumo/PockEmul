/*******************************************************
 *
 *      Portable (hopefully ;-) 8085A emulator
 *
 *      Written by J. Buchmueller for use with MAME
 *
 *      Partially based on Z80Em by Marcel De Kogel
 *
 *      CPU related macros
 *
 *******************************************************/


#define SF              0x80
#define ZF              0x40
#define X5F             0x20
#define HF              0x10
#define X3F             0x08
#define PF              0x04
#define VF              0x02
#define CF              0x01

#define IM_SID          0x80
#define IM_I75          0x40
#define IM_I65          0x20
#define IM_I55          0x10
#define IM_IE           0x08
#define IM_M75          0x04
#define IM_M65          0x02
#define IM_M55          0x01

#define ADDR_TRAP       0x0024
#define ADDR_RST55      0x002c
#define ADDR_RST65      0x0034
#define ADDR_RST75      0x003c
#define ADDR_INTR       0x0038


#define M_MVI(R) R=ARG()

/* rotate */
#define M_RLC { 																	\
    i85stat.regs.AF.b.h = (i85stat.regs.AF.b.h << 1) | (i85stat.regs.AF.b.h >> 7);			\
    i85stat.regs.AF.b.l = (i85stat.regs.AF.b.l & 0xfe) | (i85stat.regs.AF.b.h & CF);			\
}

#define M_RRC { 																	\
    i85stat.regs.AF.b.l = (i85stat.regs.AF.b.l & 0xfe) | (i85stat.regs.AF.b.h & CF);			\
    i85stat.regs.AF.b.h = (i85stat.regs.AF.b.h >> 1) | (i85stat.regs.AF.b.h << 7);			\
}

#define M_RAL { 																	\
    int c = i85stat.regs.AF.b.l&CF;													\
    i85stat.regs.AF.b.l = (i85stat.regs.AF.b.l & 0xfe) | (i85stat.regs.AF.b.h >> 7);			\
    i85stat.regs.AF.b.h = (i85stat.regs.AF.b.h << 1) | c; 								\
}

#define M_RAR { 																	\
    int c = (i85stat.regs.AF.b.l&CF) << 7; 											\
    i85stat.regs.AF.b.l = (i85stat.regs.AF.b.l & 0xfe) | (i85stat.regs.AF.b.h & CF);			\
    i85stat.regs.AF.b.h = (i85stat.regs.AF.b.h >> 1) | c; 								\
}

/* logical */
#define M_ORA(R) i85stat.regs.AF.b.h|=R; i85stat.regs.AF.b.l=ZSP[i85stat.regs.AF.b.h]
#define M_XRA(R) i85stat.regs.AF.b.h^=R; i85stat.regs.AF.b.l=ZSP[i85stat.regs.AF.b.h]
#define M_ANA(R) {UINT8 hc = ((i85stat.regs.AF.b.h | R)<<1) & HF; i85stat.regs.AF.b.h&=R; i85stat.regs.AF.b.l=ZSP[i85stat.regs.AF.b.h]; if(IS_8085()) { i85stat.regs.AF.b.l |= HF; } else {i85stat.regs.AF.b.l |= hc; } }

/* increase / decrease */
#define M_INR(R) {UINT8 hc = ((R & 0x0f) == 0x0f) ? HF : 0; ++R; i85stat.regs.AF.b.l= (i85stat.regs.AF.b.l & CF ) | ZSP[R] | hc; }
#define M_DCR(R) {UINT8 hc = ((R & 0x0f) == 0x00) ? HF : 0; --R; i85stat.regs.AF.b.l= (i85stat.regs.AF.b.l & CF ) | ZSP[R] | hc | VF; }

/* arithmetic */
#define M_ADD(R) {																	\
    int q = i85stat.regs.AF.b.h+R;														\
    i85stat.regs.AF.b.l=ZSP[q&255]|((q>>8)&CF)|((i85stat.regs.AF.b.h^q^R)&HF);			\
    i85stat.regs.AF.b.h=q;																\
}

#define M_ADC(R) {																	\
    int q = i85stat.regs.AF.b.h+R+(i85stat.regs.AF.b.l&CF);								\
    i85stat.regs.AF.b.l=ZSP[q&255]|((q>>8)&CF)|((i85stat.regs.AF.b.h^q^R)&HF);			\
    i85stat.regs.AF.b.h=q;																\
}

#define M_SUB(R) {																	\
    int q = i85stat.regs.AF.b.h-R;														\
    i85stat.regs.AF.b.l=ZSP[q&255]|((q>>8)&CF)|((i85stat.regs.AF.b.h^q^R)&HF)|VF;			\
    i85stat.regs.AF.b.h=q;																\
}

#define M_SBB(R) {																	\
    int q = i85stat.regs.AF.b.h-R-(i85stat.regs.AF.b.l&CF);								\
    i85stat.regs.AF.b.l=ZSP[q&255]|((q>>8)&CF)|((i85stat.regs.AF.b.h^q^R)&HF)|VF;			\
    i85stat.regs.AF.b.h=q;																\
}

#define M_CMP(R) {																	\
    int q = i85stat.regs.AF.b.h-R;														\
    i85stat.regs.AF.b.l=ZSP[q&255]|((q>>8)&CF)|((i85stat.regs.AF.b.h^q^R)&HF)|VF;			\
}

#define M_DAD(R) {                                              					\
    int q = i85stat.regs.HL.d + i85stat.regs.R.d; 										\
    i85stat.regs.AF.b.l = ( i85stat.regs.AF.b.l & ~(HF+CF) ) |							\
        ( ((i85stat.regs.HL.d^q^i85stat.regs.R.d) >> 8) & HF ) |							\
        ( (q>>16) & CF );															\
    i85stat.regs.HL.w.l = q;															\
}

#define M_DSUB() {															\
    int q = i85stat.regs.HL.b.l-i85stat.regs.BC.b.l;										\
    i85stat.regs.AF.b.l=ZS[q&255]|((q>>8)&CF)|VF|										\
        ((i85stat.regs.HL.b.l^q^i85stat.regs.BC.b.l)&HF)|									\
        (((i85stat.regs.BC.b.l^i85stat.regs.HL.b.l)&(i85stat.regs.HL.b.l^q)&SF)>>5);			\
    i85stat.regs.HL.b.l=q; 															\
    q = i85stat.regs.HL.b.h-i85stat.regs.BC.b.h-(i85stat.regs.AF.b.l&CF);					\
    i85stat.regs.AF.b.l=ZS[q&255]|((q>>8)&CF)|VF|										\
        ((i85stat.regs.HL.b.h^q^i85stat.regs.BC.b.h)&HF)|									\
        (((i85stat.regs.BC.b.h^i85stat.regs.HL.b.h)&(i85stat.regs.HL.b.h^q)&SF)>>5);			\
    if (i85stat.regs.HL.b.l!=0) i85stat.regs.AF.b.l&=~ZF;									\
}

/* i/o */
#define M_IN																		\
    i85stat.regs.STATUS = 0x42;														\
    i85stat.regs.WZ.d=ARG();													\
    i85stat.regs.AF.b.h=cpu_readport(i85stat.regs.WZ.d);

#define M_OUT																		\
    i85stat.regs.STATUS = 0x10;														\
    i85stat.regs.WZ.d=ARG();													\
    cpu_writeport(i85stat.regs.WZ.d,i85stat.regs.AF.b.h)

/* stack */
#define M_PUSH(R) {                                             					\
    i85stat.regs.STATUS = 0x04;														\
    WM(--i85stat.regs.SP.w.l, i85stat.regs.R.b.h);	\
    WM(--i85stat.regs.SP.w.l, i85stat.regs.R.b.l);	\
}

#define M_POP(R) {																	\
    i85stat.regs.STATUS = 0x86;														\
    i85stat.regs.R.b.l = RM(i85stat.regs.SP.w.l++);	\
    i85stat.regs.R.b.h = RM(i85stat.regs.SP.w.l++);	\
}

/* jumps */
// On 8085 jump if condition is not satisfied is shorter
#define M_JMP(cc) { 																\
    if (cc) {																		\
        i85stat.regs.PC.w.l = ARG16(); 										\
    } else {																		\
        i85stat.regs.PC.w.l += 2;														\
        i8085_ICount += (IS_8085()) ? 3 : 0;							\
    }																				\
}

// On 8085 call if condition is not satisfied is 9 ticks
#define M_CALL(cc)																	\
{																					\
    if (cc) 																		\
    {																				\
        UINT16 a = ARG16(); 												\
        i8085_ICount -= (IS_8085()) ? 7 : 6 ;							\
        M_PUSH(PC); 																\
        i85stat.regs.PC.d = a; 														\
        CallSubLevel++; \
    } else {																		\
        i85stat.regs.PC.w.l += 2;														\
        i8085_ICount += (IS_8085()) ? 2 : 0;							\
    }																				\
}



#define M_RST(nn) { 																\
    M_PUSH(PC); 																	\
    CallSubLevel++; \
    i85stat.regs.PC.d = 8 * nn;														\
}
