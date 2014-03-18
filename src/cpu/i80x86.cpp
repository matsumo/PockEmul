/*
    Intel 80186/80188 emulator main
*/

#include "i80x86.h"
#include "pcxxxx.h"
#include "Inter.h"
#include "Debug.h"
#include "ui/cregsz80widget.h"
#include "Log.h"

#define FALSE	0
#define TRUE	1

#define MASK_CF	0x0001
#define MASK_PF	0x0004
#define MASK_AF	0x0010
#define MASK_ZF	0x0040
#define MASK_SF	0x0080
#define MASK_TF	0x0100
#define MASK_IF	0x0200
#define MASK_DF	0x0400
#define MASK_OF	0x0800

#define AX	i86->r16.ax
#define CX	i86->r16.cx
#define DX	i86->r16.dx
#define BX	i86->r16.bx
#define SP	i86->r16.sp
#define BP	i86->r16.bp
#define SI	i86->r16.si
#define DI	i86->r16.di
#define CS	i86->r16.cs
#define DS	i86->r16.ds
#define ES	i86->r16.es
#define SS	i86->r16.ss
#define IP	i86->r16.ip
#define F	i86->r16.f
#define CF	(F & MASK_CF)
#define PF	(F & MASK_PF)
#define AF	(F & MASK_AF)
#define ZF	(F & MASK_ZF)
#define SF	(F & MASK_SF)
#define TF	(F & MASK_TF)
#define IF	(F & MASK_IF)
#define DF	(F & MASK_DF)
#define OF	(F & MASK_OF)
#define AL	i86->r8.al
#define AH	i86->r8.ah
#define CL	i86->r8.cl
#define CH	i86->r8.ch
#define DL	i86->r8.dl
#define DH	i86->r8.dh
#define BL	i86->r8.bl
#define BH	i86->r8.bh

#define MEM8(seg, off)	i86read8(i86, seg, off)
#define MEM16(seg, off)	i86read16(i86, seg, off)
#define CODE8(off)	MEM8(CS, off)
#define CODE16(off)	MEM16(CS, off)
#define DATA8(off)	MEM8(i86->r16.prefix == NULL ? DS: *i86->r16.prefix, off)
#define DATA16(off)	MEM16(i86->r16.prefix == NULL ? DS: *i86->r16.prefix, off)
#define STACK8(off)	MEM8(SS, off)
#define STACK16(off)	MEM16(SS, off)
#define EXTRA8(off)	MEM8(ES, off)
#define EXTRA16(off)	MEM16(ES, off)
#define BASE8(off)	MEM8(i86->r16.prefix == NULL ? SS: *i86->r16.prefix, off)
#define BASE16(off)	MEM16(i86->r16.prefix == NULL ? SS: *i86->r16.prefix, off)
#define IMM8	CODE8(IP + 1)
#define IMM16	CODE16(IP + 1)
#define IMM16S	(int16 )((int8 )CODE8(IP + 1))
#define SHORT_LABEL	(IP + _length + (int8 )CODE8(IP + 1))
#define NEAR_LABEL	(IP + _length + CODE16(IP + 1))

#define SET_MEM8(seg, off, v)	i86write8(i86, seg, off, v)
#define SET_MEM16(seg, off, v)	i86write16(i86, seg, off, v)
#define SET_CODE8(off, v)	SET_MEM8(CS, off, v)
#define SET_CODE16(off, v)	SET_MEM16(CS, off, v)
#define SET_DATA8(off, v)	SET_MEM8(i86->r16.prefix == NULL ? DS: *i86->r16.prefix, off, v)
#define SET_DATA16(off, v)	SET_MEM16(i86->r16.prefix == NULL ? DS: *i86->r16.prefix, off, v)
#define SET_STACK8(off, v)	SET_MEM8(SS, off, v)
#define SET_STACK16(off, v)	SET_MEM16(SS, off, v)
#define SET_EXTRA8(off, v)	SET_MEM8(ES, off, v)
#define SET_EXTRA16(off, v)	SET_MEM16(ES, off, v)
#define SET_BASE8(off, v)	SET_MEM8(i86->r16.prefix == NULL ? SS: *i86->r16.prefix, off, v)
#define SET_BASE16(off, v)	SET_MEM16(i86->r16.prefix == NULL ? SS: *i86->r16.prefix, off, v)

#define REG8	getreg8(i86, _regrm)
#define REG16	getreg16(i86, _regrm)
#define SREG	getsreg(i86, _regrm)
#define RM8	getrm8(i86, _regrm)
#define RM16	getrm16(i86, _regrm)
#define REGRM_IMM8	CODE8(IP + regrm_length[_regrm] + 2)
#define REGRM_IMM16	CODE16(IP + regrm_length[_regrm] + 2)

#define SET_REG8(v)	setreg8(i86, _regrm, v)
#define SET_REG16(v)	setreg16(i86, _regrm, v)
#define SET_SREG(v)	setsreg(i86, _regrm, v)
#define SET_RM8(v)	setrm8(i86, _regrm, v)
#define SET_RM16(v)	setrm16(i86, _regrm, v)

#define SET_CF8(acc)	((acc) & 0x00000100 ? MASK_CF: 0)
#define SET_CF16(acc)	((acc) & 0x00010000 ? MASK_CF: 0)
#define SET_CFS(acc)	((acc) & 0x80000000 ? MASK_CF: 0)
#define SET_PF8(acc)	parity[acc & 0xff]
#define SET_PF16(acc)	(MASK_PF ^ parity[acc & 0xff] ^ parity[(acc >> 8) & 0xff])
#define SET_AF(x, y, cf)	((((x) & 0x0f) + ((y) & 0x0f) + cf) & 0x10)
#define SET_AFS(x, y, cf)	((((x) & 0x0f) - ((y) & 0x0f) - cf) & 0x10)
#define SET_ZF8(acc)	((acc) & 0x000000ff ? 0: MASK_ZF)
#define SET_ZF16(acc)	((acc) & 0x0000ffff ? 0: MASK_ZF)
#define SET_SF8(acc)	((acc) & 0x00000080 ? MASK_SF: 0)
#define SET_SF16(acc)	((acc) & 0x00008000 ? MASK_SF: 0)
#define SET_OF8(acc, x, y)	(((x) ^ (y)) & 0x80 ? 0: (((x) ^ acc) & 0x80 ? MASK_OF: 0))
#define SET_OF16(acc, x, y)	(((x) ^ (y)) & 0x8000 ? 0: (((x) ^ acc) & 0x8000 ? MASK_OF: 0))
#define SET_OFS8(acc, x, y)	(((x) ^ (y)) & 0x80 ? (((x) ^ acc) & 0x80 ? MASK_OF: 0): 0)
#define SET_OFS16(acc, x, y)	(((x) ^ (y)) & 0x8000 ? (((x) ^ acc) & 0x8000 ? MASK_OF: 0): 0)

#define FETCH_OP() \
    _op = CODE8(IP), _states += op_states[_op], _length = op_length[_op], _op
#define FETCH_REGRM() \
    _regrm = CODE8(IP + 1), _states += ((_regrm & 0xc0) == 0xc0 ? 0: op_mem_states[_op]), _length += regrm_length[_regrm]

#define _FETCH_OP2(table1, table2) \
    _regrm = CODE8(IP + 1), _states = ((_regrm & 0xc0) == 0xc0 ? table1[(_regrm & 0x38) >> 3]: table2[(_regrm & 0x38) >> 3]), _length += regrm_length[_regrm], (_regrm & 0x38)
#define FETCH_MATH8() \
    _FETCH_OP2(math_states, math_mem_states)
#define FETCH_MATH16() \
    _FETCH_OP2(math_states, math_mem_states)
#define FETCH_MATH16S() \
    _FETCH_OP2(math16s_states, math16s_mem_states)
#define FETCH_SHIFT8() \
    _FETCH_OP2(shift_states, shift_mem_states)
#define FETCH_SHIFT16() \
    _FETCH_OP2(shift_states, shift_mem_states)
#define FETCH_SHIFT8_1() \
    _FETCH_OP2(shift1_states, shift1_mem_states)
#define FETCH_SHIFT16_1() \
    _FETCH_OP2(shift1_states, shift1_mem_states)
#define FETCH_SHIFT8_CL() \
    _FETCH_OP2(shift_states, shift_mem_states)
#define FETCH_SHIFT16_CL() \
    _FETCH_OP2(shift_states, shift_mem_states)
#define FETCH_GRP1_8() \
    _FETCH_OP2(grp1_8_states, grp1_8_mem_states)
#define FETCH_GRP1_16() \
    _FETCH_OP2(grp1_16_states, grp1_16_mem_states)
#define FETCH_GRP2_8() \
    _FETCH_OP2(grp2_8_states, grp2_8_mem_states)
#define FETCH_GRP2_16() \
    _FETCH_OP2(grp2_16_states, grp2_16_mem_states)

#define OP8_RM_R(op) \
    { \
        uint32 _acc, _x = RM8, _y = REG8; \
        op(_x, _y); \
        SET_RM8(_acc); \
    } \
    IP += _length;
#define OP16_RM_R(op) \
    { \
        uint32 _acc, _x = RM16, _y = REG16; \
        op(_x, _y); \
        SET_RM16(_acc); \
    } \
    IP += _length;
#define OP8_R_RM(op) \
    { \
        uint32 _acc, _x = REG8, _y = RM8; \
        op(_x, _y); \
        SET_REG8(_acc); \
    } \
    IP += _length;
#define OP16_R_RM(op) \
    { \
        uint32 _acc, _x = REG16, _y = RM16; \
        op(_x, _y); \
        SET_REG16(_acc); \
    } \
    IP += _length;
#define OP8_R(op, x, y) \
    { \
        uint32 _acc, _x = x, _y = y; \
        op(_x, _y); \
        x = _acc; \
    } \
    IP += _length;
#define OP16_R(op, x, y) \
    { \
        uint32 _acc, _x = x, _y = y; \
        op(_x, _y); \
        x = _acc; \
    } \
    IP += _length;
#define OP8_M(op, off, x) \
    { \
        uint16 _off = off; \
        uint32 _acc, _x = DATA8(_off), _y = x; \
        op(_x, _y); \
        SET_DATA8(_off, _acc); \
    } \
    IP += _length;
#define OP16_M(op, off, x) \
    { \
        uint16 _off = off; \
        uint32 _acc, _x = DATA16(_off), _y = x; \
        op(_x, _y); \
        SET_DATA16(_off, _acc); \
    } \
    IP += _length;
#define OP8_RM_IMM(op) \
    { \
        uint32 _acc, _x = RM8, _y = REGRM_IMM8; \
        op(_x, _y); \
        SET_RM8(_acc); \
    } \
    IP += _length;
#define OP16_RM_IMM(op) \
    { \
        uint32 _acc, _x = RM16, _y = REGRM_IMM16; \
        op(_x, _y); \
        SET_RM16(_acc); \
    } \
    IP += _length;
#define OP16S_RM_IMM(op) \
    { \
        uint32 _acc, _x = RM16, _y = (int8 )REGRM_IMM8; \
        op(_x, _y); \
        SET_RM16(_acc); \
    } \
    IP += _length;
#define _ROTATE(op, x, n) \
    { \
        uint32 _x; \
        uint16 _n = n; \
        _states += _n; \
        _acc = x; \
        while(_n--) { \
            _x = _acc; \
            op(_x); \
        } \
    }
#define ROTATE8_RM(op, n) \
    if (n != 0) { \
        uint32 _acc; \
        _ROTATE(op, RM8, n); \
        SET_RM8(_acc); \
    } \
    IP += _length;
#define ROTATE16_RM(op, n) \
    if (n != 0) { \
        uint32 _acc; \
        _ROTATE(op, RM16, n); \
        SET_RM16(_acc); \
    } \
    IP += _length;

#define _SET_FLAGS_A8(acc, x, y, c) \
    ((F & ~MASK_OF & ~MASK_SF & ~MASK_ZF & ~MASK_AF & ~MASK_PF) | SET_OF8(acc, x, y) | SET_SF8(acc) | SET_ZF8(acc) | SET_AF(x, y, c) | SET_PF8(acc))
#define SET_FLAGS_I8(acc, x) \
    _SET_FLAGS_A8(acc, x, 1, 0)
#define SET_FLAGS_A8(acc, x, y, c) \
    ((_SET_FLAGS_A8(acc, x, y, c) & ~MASK_CF) | SET_CF8(acc))
#define _SET_FLAGS_A16(acc, x, y, c) \
    ((F & ~MASK_OF & ~MASK_SF & ~MASK_ZF & ~MASK_AF & ~MASK_PF) | SET_OF16(acc, x, y) | SET_SF16(acc) | SET_ZF16(acc) | SET_AF(x, y, c) | SET_PF16(acc))
#define SET_FLAGS_I16(acc, x) \
    _SET_FLAGS_A16(acc, x, 1, 0)
#define SET_FLAGS_A16(acc, x, y, c) \
    ((_SET_FLAGS_A16(acc, x, y, c) & ~MASK_CF) | SET_CF16(acc))
#define _SET_FLAGS_S8(acc, x, y, c) \
    ((F & ~MASK_OF & ~MASK_SF & ~MASK_ZF & ~MASK_AF & ~MASK_PF) | SET_OFS8(acc, x, y) | SET_SF8(acc) | SET_ZF8(acc) | SET_AFS(x, y, c) | SET_PF8(acc))
#define SET_FLAGS_D8(acc, x) \
    _SET_FLAGS_S8(acc, x, 1, 0)
#define SET_FLAGS_S8(acc, x, y, c) \
    ((_SET_FLAGS_S8(acc, x, y, c) & ~MASK_CF) | SET_CF8(acc))
#define _SET_FLAGS_S16(acc, x, y, c) \
    ((F & ~MASK_OF & ~MASK_SF & ~MASK_ZF & ~MASK_AF & ~MASK_PF) | SET_OFS16(acc, x, y) | SET_SF16(acc) | SET_ZF16(acc) | SET_AFS(x, y, c) | SET_PF8(acc))
#define SET_FLAGS_D16(acc, x) \
    _SET_FLAGS_S16(acc, x, 1, 0)
#define SET_FLAGS_S16(acc, x, y, c) \
    ((_SET_FLAGS_S16(acc, x, y, c) & ~MASK_CF) | SET_CF16(acc))
#define SET_FLAGS_L8(acc) \
    ((F & ~MASK_OF & ~MASK_PF & ~MASK_SF & ~MASK_ZF & ~MASK_CF) | SET_PF8(acc) | SET_SF8(acc) | SET_ZF8(acc))
#define SET_FLAGS_L16(acc) \
    ((F & ~MASK_OF & ~MASK_PF & ~MASK_SF & ~MASK_ZF & ~MASK_CF) | SET_PF16(acc) | SET_SF16(acc) | SET_ZF16(acc))

#define AAA() \
    if((AL & 0x0f) > 9 || AF) { \
        AL = (AL + 6) & 0x0f; \
        AH++; \
        F = F | MASK_AF | MASK_CF; \
    } else { \
        F = F & ~MASK_AF & ~MASK_CF; \
    } \
    IP += _length;

#define AAD(x) \
    AL = AH * 0x0a + AL; \
    AH = 0; \
    F  = SET_FLAGS_L8(AL); \
    IP += _length;

#define AAM(x) \
    AH = AL / 0x0a; \
    AL = AL % 0x0a; \
    F  = SET_FLAGS_L8(AL); \
    IP += _length;

#define AAS() \
    if((AL & 0x0f) > 9 || AF) { \
        AL = (AL - 6) & 0x0f; \
        AH--; \
        F = F | MASK_AF | MASK_CF; \
    } else { \
        F = F & ~MASK_AF & ~MASK_CF; \
    } \
    IP += _length;

#define _ADC8(x, y) \
    _acc = (x) + (y) + CF; \
    F = SET_FLAGS_A8(_acc, x, y, CF);
#define _ADC16(x, y) \
    _acc = (x) + (y) + CF; \
    F = SET_FLAGS_A16(_acc, x, y, CF);
#define ADC8_RM_R() \
    OP8_RM_R(_ADC8)
#define ADC16_RM_R() \
    OP16_RM_R(_ADC16)
#define ADC8_R_RM() \
    OP8_R_RM(_ADC8)
#define ADC16_R_RM() \
    OP16_R_RM(_ADC16)
#define ADC8_R(x, y) \
    OP8_R(_ADC8, x, y)
#define ADC16_R(x, y) \
    OP16_R(_ADC16, x, y)
#define ADC8_RM_IMM() \
    OP8_RM_IMM(_ADC8)
#define ADC16_RM_IMM() \
    OP16_RM_IMM(_ADC16)
#define ADC16S_RM_IMM() \
    OP16S_RM_IMM(_ADC16)

#define _ADD8(x, y) \
    _acc = (x) + (y); \
    F = SET_FLAGS_A8(_acc, x, y, 0);
#define _ADD16(x, y) \
    _acc = (x) + (y); \
    F = SET_FLAGS_A16(_acc, x, y, 0);
#define ADD8_RM_R() \
    OP8_RM_R(_ADD8)
#define ADD16_RM_R() \
    OP16_RM_R(_ADD16)
#define ADD8_R_RM() \
    OP8_R_RM(_ADD8)
#define ADD16_R_RM() \
    OP16_R_RM(_ADD16)
#define ADD8_R(x, y) \
    OP8_R(_ADD8, x, y)
#define ADD16_R(x, y) \
    OP16_R(_ADD16, x, y)
#define ADD8_RM_IMM() \
    OP8_RM_IMM(_ADD8)
#define ADD16_RM_IMM() \
    OP16_RM_IMM(_ADD16)
#define ADD16S_RM_IMM() \
    OP16S_RM_IMM(_ADD16)

#define _AND8(x, y) \
    _acc = (x) & (y); \
    F = SET_FLAGS_L8(_acc);
#define _AND16(x, y) \
    _acc = (x) & (y); \
    F = SET_FLAGS_L16(_acc);
#define AND8_RM_R() \
    OP8_RM_R(_AND8)
#define AND16_RM_R() \
    OP16_RM_R(_AND16)
#define AND8_R_RM() \
    OP8_R_RM(_AND8)
#define AND16_R_RM() \
    OP16_R_RM(_AND16)
#define AND8_R(x, y) \
    OP8_R(_AND8, x, y)
#define AND16_R(x, y) \
    OP16_R(_AND16, x, y)
#define AND8_RM_IMM() \
    OP8_RM_IMM(_AND8)
#define AND16_RM_IMM() \
    OP16_RM_IMM(_AND16)
#define AND16S_RM_IMM() \
    OP16S_RM_IMM(_AND16)

#define BOUND() \
    { \
        uint16 _x = REG16, _y = RM16; \
        if(_x < _y || _x > _y + 2) { \
            INT(0x07); \
        } else { \
            IP += _length; \
        } \
    }

#define CALL_NEAR(x) \
    SP -= 2; \
    SET_STACK16(SP, IP + _length); \
    CallSubLevel++; \
    IP = x;
#define CALL_FAR(x, y) \
    { \
        uint16 _x = x, _y = y; \
        SP -= 2; \
        SET_STACK16(SP, CS); \
        SP -= 2; \
        SET_STACK16(SP, IP + _length); \
        IP = _x; \
        CS = _y; \
        CallSubLevel++; \
    }
#define CALL_NEAR_RM() \
    { \
        uint16 _x = RM16; \
        SP -= 2; \
        SET_STACK16(SP, IP + _length); \
        IP = _x; \
        CallSubLevel++; \
    }
#define CALL_FAR_RM() \
    { \
        uint16 _seg, _off, _x, _y; \
        getsegoff(i86, _regrm, &_seg, &_off); \
        _x = MEM16(_seg, _off); \
        _y = MEM16(_seg, _off + 2); \
        SP -= 2; \
        SET_STACK16(SP, CS); \
        SP -= 2; \
        SET_STACK16(SP, IP + _length); \
        IP = _x; \
        CS = _y; \
        CallSubLevel++; \
    }

#define CBW() \
    if(AL < 0x80) { \
        AH = 0; \
    } else { \
        AH = 0xff; \
    } \
    IP += _length;

#define CLC() \
    F = F & ~MASK_CF; \
    IP += _length;

#define CLD() \
    F = F & ~MASK_DF; \
    IP += _length;

#define CLI() \
    F = F & ~MASK_IF; \
    IP += _length;

#define CMC() \
    if(CF) { \
        F = F & ~MASK_CF; \
    } else { \
        F = F | MASK_CF; \
    } \
    IP += _length;

#define _CMP8(x, y) \
    { \
        uint32 _acc = (x) - (y); \
        F = SET_FLAGS_S8(_acc, x, y, 0); \
    }
#define _CMP16(x, y) \
    { \
        uint32 _acc = (x) - (y); \
        F = SET_FLAGS_S16(_acc, x, y, 0); \
    }
#define CMP8_RM_R() \
    _CMP8(RM8, REG8); \
    IP += _length;
#define CMP16_RM_R() \
    _CMP16(RM16, REG16); \
    IP += _length;
#define CMP8_R_RM() \
    _CMP8(REG8, RM8); \
    IP += _length;
#define CMP16_R_RM() \
    _CMP16(REG16, RM16); \
    IP += _length;
#define CMP8_R(x, y) \
    _CMP8(x, y); \
    IP += _length;
#define CMP16_R(x, y) \
    _CMP16(x, y); \
    IP += _length;
#define CMP8_RM_IMM() \
    _CMP8(RM8, REGRM_IMM8); \
    IP += _length;
#define CMP16_RM_IMM() \
    _CMP16(RM16, REGRM_IMM16); \
    IP += _length;
#define CMP16S_RM_IMM() \
    _CMP16(RM16, (int16 )((int8 )REGRM_IMM8)); \
    IP += _length;

#define _CMPSB() \
    { \
        i86->r16.prefix = NULL; \
        _CMP8(DATA8(SI), EXTRA8(DI)); \
        if(DF) { \
            SI--; \
            DI--; \
        } else { \
            SI++; \
            DI++; \
        } \
    }
#define _CMPSW() \
    { \
        i86->r16.prefix = NULL; \
        _CMP16(DATA16(SI), EXTRA16(DI)); \
        if(DF) { \
            SI -= 2; \
            DI -= 2; \
        } else { \
            SI += 2; \
            DI += 2; \
        } \
    }
#define CMPSB() \
    _CMPSB(); \
    IP += _length;
#define CMPSW() \
    _CMPSW(); \
    IP += _length;

#define CWD() \
    if(AX < 0x8000) { \
        DX = 0; \
    } else { \
        DX = 0xffff; \
    } \
    IP += _length;

#define DAA() \
    if((AL & 0x0f) > 9 || AF) { \
        AL += 6; \
        F = F | MASK_AF; \
    } \
    if(AL > 0x9f || CF) { \
        AL += 0x60; \
        F = F | MASK_CF; \
    } \
    F = (F & ~MASK_PF & ~MASK_SF & ~MASK_ZF) | SET_PF8(AL) | SET_SF8(AL) | SET_ZF8(AL); \
    IP += _length;

#define DAS() \
    if((AL & 0x0f) > 9 || AF) { \
        AL -= 6; \
        F = F | MASK_AF; \
    } \
    if(AL > 0x9f || CF) { \
        AL -= 0x60; \
        F = F | MASK_CF; \
    } \
    F = (F & ~MASK_PF & ~MASK_SF & ~MASK_ZF) | SET_PF8(AL) | SET_SF8(AL) | SET_ZF8(AL); \
    IP += _length;

#define _DEC8(x, y) \
    _acc = x - 1; \
    F = SET_FLAGS_D8(_acc, x);
#define _DEC16(x, y) \
    _acc = x - 1; \
    F = SET_FLAGS_D16(_acc, x);
#define DEC8_R(x) \
    OP8_R(_DEC8, x, 0)
#define DEC16_R(x) \
    OP16_R(_DEC16, x, 0)
#define DEC8_RM() \
    OP8_RM_R(_DEC8)
#define DEC16_RM() \
    OP16_RM_R(_DEC16)

#define DIV8_RM() \
    { \
        uint16 tmp = AX, _x = RM8; \
        if(_x == 0 || tmp / _x > 0xff) { \
            INT(0x00); \
        } else { \
            AL = tmp / _x; \
            AH = tmp % _x; \
            IP += _length; \
        } \
    }
#define DIV16_RM() \
    { \
        uint32 tmp = (uint32 )DX << 16 | AX, _x = RM16; \
        if(_x == 0 || tmp / _x > 0xffff) { \
            INT(0x00); \
        } else { \
            AX = tmp / _x; \
            DX = tmp % _x; \
            IP += _length; \
        } \
    }

#define ENTER(x, y) \
    { \
        uint16 _x = x, _y = y, _fp; \
        SP -= 2; \
        _fp = SP; \
        SET_STACK16(SP, BP); \
        if(_y > 0) { \
            while(--_y > 0) { \
                BP -= 2; \
                SP -= 2; \
                SET_STACK16(SP, BP); \
            } \
            SP -= 2; \
            SET_STACK16(SP, _fp); \
        } \
        BP = _fp; \
        SP -= _x; \
        if(_y == 0) { \
            _states = 15; \
        } else if(_y == 1) { \
            _states = 25; \
        } else { \
            _states = 22 + _y * 16; \
        } \
    } \
    IP += _length;

#define ESC(x) \
    AddLog(LOG_CPU,"ESC"); \
    IP += _length;

#define HLT() \
    i86->r16.hlt = 1; \
    halt = true; \
    IP += _length;

#define IDIV8_RM() \
    { \
        int16 _tmp = (int16 )AX, _x = (int )RM8; \
        if(_tmp == 0 || _tmp / _x > 0x7f || _tmp / _x < -0x7f - 1) { \
            INT(0x00); \
        } else { \
            AL = _tmp / _x; \
            AH = _tmp % _x; \
            IP += _length; \
        } \
    }
#define IDIV16_RM() \
    { \
        int32 _tmp = (int32 )((uint32 )DX << 16 | AX), _x = RM16; \
        if(_tmp == 0 || _tmp / _x > 0x7fff || _tmp / _x < -0x7fff - 1) { \
            INT(0x00); \
        } else { \
            AX = _tmp / _x; \
            DX = _tmp % _x; \
            IP += _length; \
        } \
    }

#define IMUL8_RM() \
    AX = (int16 )((int8 )AL) * (int8 )RM8; \
    if((AH ^ AL) & 0x80) { \
        F = F | MASK_CF | MASK_OF; \
    } else { \
        F = F & ~MASK_CF & ~MASK_OF; \
    } \
    IP += _length;
#define IMUL16_RM() \
    { \
        int32 _acc = (int32 )((int16 )AX) * (int16 )RM16; \
        AX = _acc & 0xffff; \
        DX = _acc >> 16; \
        if((AX ^ DX) & 0x8000) { \
            F = F | MASK_CF | MASK_OF; \
        } else { \
            F = F & ~MASK_CF & ~MASK_OF; \
        } \
    } \
    IP += _length;
#define IMUL16_RM_IMM16() \
    { \
        uint32 _acc = (int32 )((int16 )RM16) * (int16 )REGRM_IMM16; \
        SET_REG16(_acc & 0xffff); \
        if(_acc & 0x10000) { \
            F = F | MASK_CF | MASK_OF; \
        } else { \
            F = F & ~MASK_CF & ~MASK_OF; \
        } \
    } \
    IP += _length;
#define IMUL16_RM_IMM8() \
    { \
        uint32 _acc = (int32 )((int16 )RM16) * (int8 )REGRM_IMM8; \
        SET_REG16(_acc & 0xffff); \
        if(_acc & 0x10000) { \
            F = F | MASK_CF | MASK_OF; \
        } else { \
            F = F & ~MASK_CF & ~MASK_OF; \
        } \
    } \
    IP += _length;

#define IN8(x, y) \
    x = i86inp8(i86, y); \
    IP += _length;
#define IN16(x, y) \
    x = i86inp16(i86, y); \
    IP += _length;

#define _INC8(x, y) \
    _acc = (x) + 1; \
    F = SET_FLAGS_I8(_acc, x);
#define _INC16(x, y) \
    _acc = (x) + 1; \
    F = SET_FLAGS_I16(_acc, x)
#define INC8_R(x) \
    OP8_R(_INC8, x, 0)
#define INC16_R(x) \
    OP16_R(_INC16, x, 0)
#define INC8_RM() \
    OP8_RM_R(_INC8)
#define INC16_RM() \
    OP16_RM_R(_INC16)

#define _INS8() \
    i86->r16.prefix = NULL; \
    SET_EXTRA8(DI, i86inp8(i86, DX)); \
    if(DF) { \
        DI--; \
    } else { \
        DI++; \
    }
#define _INS16() \
    i86->r16.prefix = NULL; \
    SET_EXTRA16(DI, i86inp16(i86, DX)); \
    if(DF) { \
        DI -= 2; \
    } else { \
        DI += 2; \
    }
#define INS8() \
    _INS8(); \
    IP += _length;
#define INS16() \
    _INS16(); \
    IP += _length;

#define _INT(x) \
    SP -= 2; \
    SET_STACK16(SP, F); \
    F = F & ~MASK_IF & ~MASK_TF; \
    SP -= 2; \
    SET_STACK16(SP, CS); \
    CS = MEM16(0, (x) * 4 + 2); \
    SP -= 2; \
    SET_STACK16(SP, IP); \
    IP = MEM16(0, (x) * 4); \
    CallSubLevel++;

#define INT(x) \
    { \
        int _x = x; \
        IP += _length; \
        _INT(_x); \
    }

#define INTO() \
    if(OF) { \
        INT(0x04); \
    } else { \
        IP += _length; \
    }

#define IRET() \
    IP = STACK16(SP); \
    SP += 2; \
    CS = STACK16(SP); \
    SP += 2; \
    F = STACK16(SP); \
    SP += 2; \
    CallSubLevel--;

#define JCXZ(x) \
    if(!CX) { \
        IP = x; \
        _states = 15; \
    } else { \
        IP += _length; \
    }

#define JMP_NEAR(x) \
    IP = x;
#define JMP_NEAR_RM() \
    IP = RM16;
#define JMP_COND(x, y) \
    if(x) { \
        IP = y; \
        _states = 13; \
    } else { \
        IP += _length; \
    }
#define JMP_FAR(x, y) \
    { \
        uint16 _off = x, _seg = y; \
        IP = _off; \
        CS = _seg; \
    }
#define JMP_FAR_RM() \
    { \
        uint16 _seg, _off, _x, _y; \
        getsegoff(i86, _regrm, &_seg, &_off); \
        _x = MEM16(_seg, _off); \
        _y = MEM16(_seg, _off + 2); \
        IP = _x; \
        CS = _y; \
    }

#define LAHF() \
    AH = (SF ? 0x80: 0) | (ZF ? 0x40: 0) | (AF ? 0x10: 0) | (PF ? 0x04: 0) | CF; \
    IP += _length;

#define LDS() \
    { \
        uint16 _seg, _off; \
        getsegoff(i86, _regrm, &_seg, &_off); \
        SET_REG16(MEM16(_seg, _off)); \
        DS = MEM16(_seg, _off + 2); \
    } \
    IP += _length;

#define LEA() \
    { \
        uint16 _seg, _off; \
        getsegoff(i86, _regrm, &_seg, &_off); \
        SET_REG16(_off); \
    } \
    IP += _length;

#define LEAVE() \
    SP = BP; \
    BP = STACK16(SP); \
    SP += 2; \
    IP += _length;

#define LES() \
    { \
        uint16 _seg, _off; \
        getsegoff(i86, _regrm, &_seg, &_off); \
        SET_REG16(MEM16(_seg, _off)); \
        ES = MEM16(_seg, _off + 2); \
    } \
    IP += _length;

#define LOCK() \
    IP += _length;

#define _LODSB() \
    i86->r16.prefix = NULL; \
    AL = DATA8(SI); \
    if(DF) { \
        SI--; \
    } else { \
        SI++; \
    }
#define _LODSW() \
    i86->r16.prefix = NULL; \
    AX = DATA16(SI); \
    if(DF) { \
        SI -= 2; \
    } else { \
        SI += 2; \
    }
#define LODSB() \
    _LODSB(); \
    IP += _length;
#define LODSW() \
    _LODSW(); \
    IP += _length;

#define LOOP(x, y) \
    CX--; \
    if(CX && (x)) { \
        IP = y; \
        _states = 16; \
    } else { \
        IP += _length; \
    }

#define MOV8_RM_R() \
    SET_RM8(REG8); \
    IP += _length;
#define MOV16_RM_R() \
    SET_RM16(REG16); \
    IP += _length;
#define MOV8_R_RM() \
    SET_REG8(RM8); \
    IP += _length;
#define MOV16_R_RM() \
    SET_REG16(RM16); \
    IP += _length;
#define MOV8_R(x, y) \
    x = (y); \
    IP += _length;
#define MOV16_R(x, y) \
    x = (y); \
    IP += _length;
#define MOV8_M(x, y) \
    SET_DATA8(x, y); \
    IP += _length;
#define MOV16_M(x, y) \
    SET_DATA16(x, y); \
    IP += _length;
#define MOV8_R_M(x, y) \
    x = DATA8(y); \
    IP += _length;
#define MOV16_R_M(x, y) \
    x = DATA16(y); \
    IP += _length;
#define MOV16_RM_SREG() \
    SET_RM16(SREG); \
    IP += _length;
#define MOV16_SREG_RM() \
    SET_SREG(RM16); \
    IP += _length;
#define MOV8_RM_IMM() \
    SET_RM8(REGRM_IMM8); \
    IP += _length;
#define MOV16_RM_IMM() \
    SET_RM16(REGRM_IMM16); \
    IP += _length;

#define _MOVSB() \
    i86->r16.prefix = NULL; \
    SET_EXTRA8(DI, DATA8(SI)); \
    if(DF) { \
        DI--; \
        SI--; \
    } else { \
        DI++; \
        SI++; \
    }
#define _MOVSW() \
    i86->r16.prefix = NULL; \
    SET_EXTRA16(DI, DATA16(SI)); \
    if(DF) { \
        DI -= 2; \
        SI -= 2; \
    } else { \
        DI += 2; \
        SI += 2; \
    }
#define MOVSB() \
    _MOVSB(); \
    IP += _length;
#define MOVSW() \
    _MOVSW(); \
    IP += _length;

#define MUL8_RM() \
    { \
        AX = (uint32 )AL * RM8; \
        if(AX & 0xff00) { \
            F = F | MASK_CF | MASK_OF; \
        } else { \
            F = F & ~MASK_CF & ~MASK_OF; \
        } \
    } \
    IP += _length;
#define MUL16_RM() \
    { \
        uint32 _acc = (uint32 )AX * RM16; \
        AX = _acc & 0xffff; \
        DX = _acc >> 16; \
        if(DX) { \
            F = F | MASK_CF | MASK_OF; \
        } else { \
            F = F & ~MASK_CF & ~MASK_OF; \
        } \
    } \
    IP += _length;

#define NEG8_RM() \
    { \
        uint32 _x = RM8, _acc = -RM8; \
        F = SET_FLAGS_S8(_acc, 0, _x, 0); \
        SET_RM8(_acc); \
    } \
    IP += _length;
#define NEG16_RM() \
    { \
        uint32 _x = RM16, _acc = -RM16; \
        F = SET_FLAGS_S16(_acc, 0, _x, 0); \
        SET_RM16(_acc); \
    } \
    IP += _length;

#define NOT8_RM() \
    { \
        uint32 _acc = ~RM8; \
        SET_RM8(_acc); \
    } \
    IP += _length;
#define NOT16_RM() \
    { \
        uint32 _acc = ~RM16; \
        SET_RM16(_acc); \
    } \
    IP += _length;

#define _OR8(x, y) \
    _acc = (x) | (y); \
    F = SET_FLAGS_L8(_acc);
#define _OR16(x, y) \
    _acc = (x) | (y); \
    F = SET_FLAGS_L16(_acc);
#define OR8_RM_R() \
    OP8_RM_R(_OR8)
#define OR16_RM_R() \
    OP16_RM_R(_OR16)
#define OR8_R_RM() \
    OP8_R_RM(_OR8)
#define OR16_R_RM() \
    OP16_R_RM(_OR16)
#define OR8_R(x, y) \
    OP8_R(_OR8, x, y)
#define OR16_R(x, y) \
    OP16_R(_OR16, x, y)
#define OR8_RM_IMM() \
    OP8_RM_IMM(_OR8)
#define OR16_RM_IMM() \
    OP16_RM_IMM(_OR16)
#define OR16S_RM_IMM() \
    OP16S_RM_IMM(_OR16)

#define OUT8(x, y) \
    i86out8(i86, x, y); \
    IP += _length;
#define OUT16(x, y) \
    i86out16(i86, x, y); \
    IP += _length;

#define _OUTS8() \
    i86->r16.prefix = NULL; \
    i86out8(i86, DX, DATA8(SI)); \
    if(DF) { \
        SI--; \
    } else { \
        SI++; \
    }
#define _OUTS16() \
    i86->r16.prefix = NULL; \
    i86out16(i86, DX, DATA16(SI)); \
    if(DF) { \
        SI -= 2; \
    } else { \
        SI += 2; \
    }
#define OUTS8() \
    _OUTS8(); \
    IP += _length;
#define OUTS16() \
    _OUTS16(); \
    IP += _length;

#define POP_R(x) \
    x = STACK16(SP); \
    SP += 2; \
    IP += _length;
#define POP_RM() \
    SET_RM16(STACK16(SP)); \
    SP += 2; \
    IP += _length;

#define POPA() \
    DI = STACK16(SP); \
    SP += 2; \
    SI = STACK16(SP); \
    SP += 2; \
    BP = STACK16(SP); \
    SP += 2; \
    SP += 2; \
    BX = STACK16(SP); \
    SP += 2; \
    DX = STACK16(SP); \
    SP += 2; \
    CX = STACK16(SP); \
    SP += 2; \
    AX = STACK16(SP); \
    SP += 2; \
    IP += _length;

#define PUSH(x) \
    SP -= 2; \
    SET_STACK16(SP, x); \
    IP += _length;
#define PUSH_RM(x) \
    PUSH(RM16);

#define PUSHA() \
    { \
        uint16 tmp = SP; \
        SP -= 2; \
        SET_STACK16(SP, AX); \
        SP -= 2; \
        SET_STACK16(SP, CX); \
        SP -= 2; \
        SET_STACK16(SP, DX); \
        SP -= 2; \
        SET_STACK16(SP, BX); \
        SP -= 2; \
        SET_STACK16(SP, tmp); \
        SP -= 2; \
        SET_STACK16(SP, BP); \
        SP -= 2; \
        SET_STACK16(SP, SI); \
        SP -= 2; \
        SET_STACK16(SP, DI); \
        IP += _length; \
    }

#define RET(x) \
    { \
        uint16 _x = x; \
        IP = STACK16(SP); \
        SP += 2 + _x; \
        CallSubLevel--; \
    }
#define RETF(x) \
    { \
        uint16 _x = x; \
        IP = STACK16(SP); \
        SP += 2; \
        CS = STACK16(SP); \
        SP += 2 + _x; \
        CallSubLevel--; \
    }

#define _RCL8(x) \
    _acc = ((x) << 1) | CF; \
    F = (F & ~MASK_OF & ~MASK_CF) | ((x) & 0x80 ? MASK_CF: 0) | ((_acc ^ (x)) & 0x80 ? MASK_OF: 0);
#define _RCL16(x) \
    _acc = ((x) << 1) | CF; \
    F = (F & ~MASK_OF & ~MASK_CF) | ((x) & 0x8000 ? MASK_CF: 0) | ((_acc ^ (x)) & 0x8000 ? MASK_OF: 0);
#define RCL8_IMM() \
    ROTATE8_RM(_RCL8, REGRM_IMM8);
#define RCL16_IMM() \
    ROTATE16_RM(_RCL16, REGRM_IMM8);
#define RCL8_1() \
    ROTATE8_RM(_RCL8, 1);
#define RCL16_1() \
    ROTATE16_RM(_RCL16, 1);
#define RCL8_CL() \
    ROTATE8_RM(_RCL8, CL);
#define RCL16_CL() \
    ROTATE16_RM(_RCL16, CL);

#define _RCR8(x) \
    _acc = ((x) >> 1) | (CF ? 0x80: 0); \
    F = (F & ~MASK_OF & ~MASK_CF) | ((x) & 0x01 ? MASK_CF: 0) | ((_acc ^ (x)) & 0x80 ? MASK_OF: 0);
#define _RCR16(x) \
    _acc = ((x) >> 1) | (CF ? 0x8000: 0); \
    F = (F & ~MASK_OF & ~MASK_CF) | ((x) & 0x0001 ? MASK_CF: 0) | ((_acc ^ (x)) & 0x8000 ? MASK_OF: 0);
#define RCR8_IMM() \
    ROTATE8_RM(_RCR8, REGRM_IMM8);
#define RCR16_IMM() \
    ROTATE16_RM(_RCR16, REGRM_IMM8);
#define RCR8_1() \
    ROTATE8_RM(_RCR8, 1);
#define RCR16_1() \
    ROTATE16_RM(_RCR16, 1);
#define RCR8_CL() \
    ROTATE8_RM(_RCR8, CL);
#define RCR16_CL() \
    ROTATE16_RM(_RCR16, CL);

#define REP(f) \
    { \
        uint8 _code = CODE8(IP + 1); \
        if(CX != 0) { \
            if(_code == 0x6c) { \
                _INS8(); _states += 8; \
            } else if(_code == 0x6d) { \
                _INS16(); _states += 8; \
            } else if(_code == 0x6e) { \
                _OUTS8(); _states += 8; \
            } else if(_code == 0x6f) { \
                _OUTS16(); _states += 8; \
            } else if(_code == 0xa4) { \
                _MOVSB(); _states += 8; \
            } else if(_code == 0xa5) { \
                _MOVSW(); _states += 8; \
            } else if(_code == 0xa6) { \
                _CMPSB(); _states += 22; \
            } else if(_code == 0xa7) { \
                _CMPSW(); _states += 22; \
            } else if(_code == 0xaa) { \
                _STOSB(); _states += 10; \
            } else if(_code == 0xab) { \
                _STOSW(); _states += 10; \
            } else if(_code == 0xac) { \
                _LODSB(); _states += 11; \
            } else if(_code == 0xad) { \
                _LODSW(); _states += 11; \
            } else if(_code == 0xae) { \
                _SCASB(); _states += 15; \
            } else if(_code == 0xaf) { \
                _SCASW(); _states += 15; \
            } else { \
                IP++; _states += 4; \
                break; \
            } \
            CX--; \
        } \
        if(CX == 0 || (!(f) && (_code == 0xa6 || _code == 0xa7 || _code == 0xae || _code == 0xaf))) { \
            if(_code == 0x6c || _code == 0x6d) { \
                _states += 8; \
            } else if(_code == 0x6e || _code == 0x6f) { \
                _states += 8; \
            } else if(_code == 0xa4 || _code == 0xa5) { \
                _states += 8; \
            } else if(_code == 0xa6 || _code == 0xa7) { \
                _states += 5; \
            } else if(_code == 0xaa || _code == 0xab) { \
                _states += 6; \
            } else if(_code == 0xac || _code == 0xad) { \
                _states += 6; \
            } else if(_code == 0xae || _code == 0xaf) { \
                _states += 5; \
            } else { \
                IP++; _states += 4; \
                break; \
            } \
            IP += 2; \
        } \
    }

#define _ROL8(x) \
    _acc = ((x) << 1) | ((x) & 0x80 ? 1: 0); \
    F = (F & ~MASK_OF & ~MASK_CF) | ((x) & 0x80 ? MASK_CF: 0) | ((_acc ^ (x)) & 0x80 ? MASK_OF: 0);
#define _ROL16(x) \
    _acc = ((x) << 1) | ((x) & 0x8000 ? 1: 0); \
    F = (F & ~MASK_OF & ~MASK_CF) | ((x) & 0x8000 ? MASK_CF: 0) | ((_acc ^ (x)) & 0x8000 ? MASK_OF: 0);
#define ROL8_IMM() \
    ROTATE8_RM(_ROL8, REGRM_IMM8);
#define ROL16_IMM() \
    ROTATE16_RM(_ROL16, REGRM_IMM8);
#define ROL8_1() \
    ROTATE8_RM(_ROL8, 1);
#define ROL16_1() \
    ROTATE16_RM(_ROL16, 1);
#define ROL8_CL() \
    ROTATE8_RM(_ROL8, CL);
#define ROL16_CL() \
    ROTATE16_RM(_ROL16, CL);

#define _ROR8(x) \
    _acc = ((x) >> 1) | ((x) & 0x01 ? 0x80: 0); \
    F = (F & ~MASK_OF & ~MASK_CF) | ((x) & 0x01) | ((_acc ^ (x)) & 0x80 ? MASK_OF: 0);
#define _ROR16(x) \
    _acc = ((x) >> 1) | ((x) & 0x0001 ? 0x8000: 0); \
    F = (F & ~MASK_OF & ~MASK_CF) | ((x) & 0x0001) | ((_acc ^ (x)) & 0x8000 ? MASK_OF: 0);
#define ROR8_IMM() \
    ROTATE8_RM(_ROR8, REGRM_IMM8);
#define ROR16_IMM() \
    ROTATE16_RM(_ROR16, REGRM_IMM8);
#define ROR8_1() \
    ROTATE8_RM(_ROR8, 1);
#define ROR16_1() \
    ROTATE16_RM(_ROR16, 1);
#define ROR8_CL() \
    ROTATE8_RM(_ROR8, CL);
#define ROR16_CL() \
    ROTATE16_RM(_ROR16, CL);

#define SAHF() \
    F = (F & 0xFF2A ) | (AH & 0x80 ? MASK_SF: 0) | (AH & 0x40 ? MASK_ZF: 0) | (AH & 0x10 ? MASK_AF: 0) | (AH & 0x04 ? MASK_PF: 0) | (AH & 0x01); \
    IP += _length;

#define _SCASB() \
    { \
        i86->r16.prefix = NULL; \
        _CMP8(AL, EXTRA8(DI)); \
        if(DF) { \
            DI--; \
        } else { \
            DI++; \
        } \
    }
#define _SCASW() \
    { \
        i86->r16.prefix = NULL; \
        _CMP16(AX, EXTRA16(DI)); \
        if(DF) { \
            DI -= 2; \
        } else { \
            DI += 2; \
        } \
    }
#define SCASB() \
    _SCASB(); \
    IP += _length;
#define SCASW() \
    _SCASW(); \
    IP += _length;

#define _SHL8(x) \
    _acc = (x) << 1; \
    F = SET_FLAGS_L8(_acc) | ((x) & 0x80 ? MASK_CF: 0) | ((_acc ^ (x)) & 0x80 ? MASK_OF: 0);
#define _SHL16(x) \
    _acc = (x) << 1; \
    F = SET_FLAGS_L16(_acc) | ((x) & 0x8000 ? MASK_CF: 0) | ((_acc ^ (x)) & 0x8000 ? MASK_OF: 0);
#define SHL8_IMM() \
    ROTATE8_RM(_SHL8, REGRM_IMM8);
#define SHL16_IMM() \
    ROTATE16_RM(_SHL16, REGRM_IMM8);
#define SHL8_1() \
    ROTATE8_RM(_SHL8, 1);
#define SHL16_1() \
    ROTATE16_RM(_SHL16, 1);
#define SHL8_CL() \
    ROTATE8_RM(_SHL8, CL);
#define SHL16_CL() \
    ROTATE16_RM(_SHL16, CL);

#define _SHR8(x) \
    _acc = ((x) >> 1); \
    F = SET_FLAGS_L8(_acc) | ((x) & 0x01 ? MASK_CF: 0) | ((_acc ^ (x)) & 0x80 ? MASK_OF: 0);
#define _SHR16(x) \
    _acc = ((x) >> 1); \
    F = SET_FLAGS_L16(_acc) | ((x) & 0x0001 ? MASK_CF: 0) | ((_acc ^ (x)) & 0x8000 ? MASK_OF: 0);
#define SHR8_IMM() \
    ROTATE8_RM(_SHR8, REGRM_IMM8);
#define SHR16_IMM() \
    ROTATE16_RM(_SHR16, REGRM_IMM8);
#define SHR8_1() \
    ROTATE8_RM(_SHR8, 1);
#define SHR16_1() \
    ROTATE16_RM(_SHR16, 1);
#define SHR8_CL() \
    ROTATE8_RM(_SHR8, CL);
#define SHR16_CL() \
    ROTATE16_RM(_SHR16, CL);

#define _SAR8(x) \
    _acc = ((x) >> 1) | ((x) & 0x80); \
    F = SET_FLAGS_L8(_acc) | ((x) & 0x01) | ((_acc ^ (x)) & 0x80 ? MASK_OF: 0);
#define _SAR16(x) \
    _acc = ((x) >> 1) | ((x) & 0x8000); \
    F = SET_FLAGS_L16(_acc) | ((x) & 0x0001) | ((_acc ^ (x)) & 0x8000 ? MASK_OF: 0);
#define SAR8_IMM() \
    ROTATE8_RM(_SAR8, REGRM_IMM8);
#define SAR16_IMM() \
    ROTATE16_RM(_SAR16, REGRM_IMM8);
#define SAR8_1() \
    ROTATE8_RM(_SAR8, 1);
#define SAR16_1() \
    ROTATE16_RM(_SAR16, 1);
#define SAR8_CL() \
    ROTATE8_RM(_SAR8, CL);
#define SAR16_CL() \
    ROTATE16_RM(_SAR16, CL);

#define _SUB8(x, y) \
    _acc = (x) - (y); \
    F = SET_FLAGS_S8(_acc, x, y, 0);
#define _SUB16(x, y) \
    _acc = (x) - (y); \
    F = SET_FLAGS_S16(_acc, x, y, 0);
#define SUB8_RM_R() \
    OP8_RM_R(_SUB8)
#define SUB16_RM_R() \
    OP16_RM_R(_SUB16)
#define SUB8_R_RM() \
    OP8_R_RM(_SUB8)
#define SUB16_R_RM() \
    OP16_R_RM(_SUB16)
#define SUB8_R(x, y) \
    OP8_R(_SUB8, x, y)
#define SUB16_R(x, y) \
    OP16_R(_SUB16, x, y)
#define SUB8_RM_IMM() \
    OP8_RM_IMM(_SUB8)
#define SUB16_RM_IMM() \
    OP16_RM_IMM(_SUB16)
#define SUB16S_RM_IMM() \
    OP16S_RM_IMM(_SUB16)

#define _SBB8(x, y) \
    _acc = (x) - (y) - CF; \
    F = SET_FLAGS_S8(_acc, x, y, CF);
#define _SBB16(x, y) \
    _acc = (x) - (y) - CF; \
    F = SET_FLAGS_S16(_acc, x, y, CF);
#define SBB8_RM_R() \
    OP8_RM_R(_SBB8)
#define SBB16_RM_R() \
    OP16_RM_R(_SBB16)
#define SBB8_R_RM() \
    OP8_R_RM(_SBB8)
#define SBB16_R_RM() \
    OP16_R_RM(_SBB16)
#define SBB8_R(x, y) \
    OP8_R(_SBB8, x, y)
#define SBB16_R(x, y) \
    OP16_R(_SBB16, x, y)
#define SBB8_RM_IMM() \
    OP8_RM_IMM(_SBB8)
#define SBB16_RM_IMM() \
    OP16_RM_IMM(_SBB16)
#define SBB16S_RM_IMM() \
    OP16S_RM_IMM(_SBB16)

#define STC() \
    F = F | MASK_CF; \
    IP += _length;

#define STD() \
    F = F | MASK_DF; \
    IP += _length;

#define STI() \
    F = F | MASK_IF; \
    IP += _length;

#define _STOSB() \
    i86->r16.prefix = NULL; \
    SET_EXTRA8(DI, AL); \
    if(DF) { \
        DI--; \
    } else { \
        DI++; \
    }
#define _STOSW() \
    i86->r16.prefix = NULL; \
    SET_EXTRA16(DI, AX); \
    if(DF) { \
        DI -= 2; \
    } else { \
        DI += 2; \
    }
#define STOSB() \
    _STOSB(); \
    IP += _length;
#define STOSW() \
    _STOSW(); \
    IP += _length;

#define _TEST8(x, y) \
    { \
        uint32 _acc = (x) & (y); \
        F = SET_FLAGS_L8(_acc); \
    }
#define _TEST16(x, y) \
    { \
        uint32 _acc = (x) & (y); \
        F = SET_FLAGS_L16(_acc); \
    }
#define TEST8_RM_R() \
    _TEST8(RM8, REG8); \
    IP += _length;
#define TEST16_RM_R() \
    _TEST16(RM16, REG16); \
    IP += _length;
#define TEST8_R(x, y) \
    _TEST8(x, y); \
    IP += _length;
#define TEST16_R(x, y) \
    _TEST16(x, y); \
    IP += _length;
#define TEST8_RM_IMM() \
    _TEST8(RM8, REGRM_IMM8); \
    IP += _length + 1;
#define TEST16_RM_IMM() \
    _TEST16(RM16, REGRM_IMM16); \
    IP += _length + 2;

#define WAIT() \
    IP += _length;

#define XCHG8_R_RM() \
    { \
        uint8 _tmp = RM8; \
        SET_RM8(REG8); \
        SET_REG8(_tmp); \
    } \
    IP += _length;
#define XCHG16_R_RM() \
    { \
        uint16 _tmp = RM16; \
        SET_RM16(REG16); \
        SET_REG16(_tmp); \
    } \
    IP += _length;
#define XCHG8_R(x, y) \
    { \
        uint8 _tmp = x; \
        x = y; \
        y = _tmp; \
    } \
    IP += _length;
#define XCHG16_R(x, y) \
    { \
        uint16 _tmp = x; \
        x = y; \
        y = _tmp; \
    } \
    IP += _length;

#define XLAT() \
    AL = DATA8(BX + AL); \
    IP += _length;

#define _XOR8(x, y) \
    _acc = (x) ^ (y); \
    F = SET_FLAGS_L8(_acc);
#define _XOR16(x, y) \
    _acc = (x) ^ (y); \
    F = SET_FLAGS_L16(_acc);
#define XOR8_RM_R() \
    OP8_RM_R(_XOR8)
#define XOR16_RM_R() \
    OP16_RM_R(_XOR16)
#define XOR8_R_RM() \
    OP8_R_RM(_XOR8)
#define XOR16_R_RM() \
    OP16_R_RM(_XOR16)
#define XOR8_R(x, y) \
    OP8_R(_XOR8, x, y)
#define XOR16_R(x, y) \
    OP16_R(_XOR16, x, y)
#define XOR8_RM_IMM() \
    OP8_RM_IMM(_XOR8)
#define XOR16_RM_IMM() \
    OP16_RM_IMM(_XOR16)
#define XOR16S_RM_IMM() \
    OP16S_RM_IMM(_XOR16)

#define OVERRIDE(x) \
    i86->r16.prefix = &x; \
    IP += _length;

#define ERR() \
    _length = 1; \
    INT(0x06);

#define TRAP() \
    _INT(0x01); \
    _states += 42;

const int Ci80x86::op_length[] = {
    2, 2, 2, 2, 2, 3, 1, 1,	/* 0x00~0x07 */
    2, 2, 2, 2, 2, 3, 1, 1,	/* 0x08~0x0f */
    2, 2, 2, 2, 2, 3, 1, 1,	/* 0x10~0x17 */
    2, 2, 2, 2, 2, 3, 1, 1,	/* 0x18~0x1f */
    2, 2, 2, 2, 2, 3, 1, 1,	/* 0x20~0x27 */
    2, 2, 2, 2, 2, 3, 1, 1,	/* 0x28~0x2f */
    2, 2, 2, 2, 2, 3, 1, 1,	/* 0x30~0x37 */
    2, 2, 2, 2, 2, 3, 1, 1,	/* 0x38~0x3f */
    1, 1, 1, 1, 1, 1, 1, 1,	/* 0x40~0x47 */
    1, 1, 1, 1, 1, 1, 1, 1,	/* 0x48~0x4f */
    1, 1, 1, 1, 1, 1, 1, 1,	/* 0x50~0x57 */
    1, 1, 1, 1, 1, 1, 1, 1,	/* 0x58~0x5f */
    1, 1, 2, 1, 1, 1, 1, 1,	/* 0x60~0x67 */
    3, 4, 2, 3, 1, 1, 1, 1,	/* 0x68~0x6f */
    2, 2, 2, 2, 2, 2, 2, 2,	/* 0x70~0x77 */
    2, 2, 2, 2, 2, 2, 2, 2,	/* 0x78~0x7f */
    3, 4, 3, 3, 2, 2, 2, 2,	/* 0x80~0x87 */
    2, 2, 2, 2, 2, 2, 2, 2,	/* 0x88~0x8f */
    1, 1, 1, 1, 1, 1, 1, 1,	/* 0x90~0x97 */
    1, 1, 5, 1, 1, 1, 1, 1,	/* 0x98~0x9f */
    3, 3, 3, 3, 1, 1, 1, 1,	/* 0xa0~0xa7 */
    2, 3, 1, 1, 1, 1, 1, 1,	/* 0xa8~0xaf */
    2, 2, 2, 2, 2, 2, 2, 2,	/* 0xb0~0xb7 */
    3, 3, 3, 3, 3, 3, 3, 3,	/* 0xb8~0xbf */
    3, 3, 3, 1, 2, 2, 3, 4,	/* 0xc0~0xc7 */
    4, 1, 3, 1, 1, 2, 1, 1,	/* 0xc8~0xcf */
    2, 2, 2, 2, 2, 2, 1, 1,	/* 0xd0~0xd7 */
    2, 2, 2, 2, 2, 2, 2, 2,	/* 0xd8~0xdf */
    2, 2, 2, 2, 2, 2, 2, 2,	/* 0xe0~0xe7 */
    3, 3, 5, 2, 1, 1, 1, 1,	/* 0xe8~0xef */
    1, 1, 1, 1, 1, 1, 2, 2,	/* 0xf0~0xf7 */
    1, 1, 1, 1, 1, 1, 2, 2	/* 0xf8~0xff */
};
const int Ci80x86::regrm_length[] = {
    0, 0, 0, 0, 0, 0, 2, 0,
    0, 0, 0, 0, 0, 0, 2, 0,
    0, 0, 0, 0, 0, 0, 2, 0,
    0, 0, 0, 0, 0, 0, 2, 0,
    0, 0, 0, 0, 0, 0, 2, 0,
    0, 0, 0, 0, 0, 0, 2, 0,
    0, 0, 0, 0, 0, 0, 2, 0,
    0, 0, 0, 0, 0, 0, 2, 0,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

const int Ci80x86::op_states[] = {
    3, 3, 3, 3, 3, 4, 13, 12,	/* 0x00~0x07 */
    3, 3, 3, 3, 3, 4, 13, 45,	/* 0x08~0x0f */
    3, 3, 3, 3, 3, 4, 13, 12,	/* 0x10~0x17 */
    3, 3, 3, 3, 3, 4, 13, 12,	/* 0x18~0x1f */
    3, 3, 3, 3, 3, 4, 2, 4,	/* 0x20~0x27 */
    3, 3, 3, 3, 3, 4, 2, 4,	/* 0x28~0x2f */
    3, 3, 3, 3, 3, 4, 2, 8,	/* 0x30~0x37 */
    3, 3, 3, 3, 3, 4, 2, 7,	/* 0x38~0x3f */
    3, 3, 3, 3, 3, 3, 3, 3,	/* 0x40~0x47 */
    3, 3, 3, 3, 3, 3, 3, 3,	/* 0x48~0x4f */
    14, 14, 14, 14, 14, 14, 14, 14,	/* 0x50~0x57 */
    14, 14, 14, 14, 14, 14, 14, 14,	/* 0x58~0x5f */
    68, 83, 34, 45, 45, 45, 45, 45,	/* 0x60~0x67 */
    14, 35, 14, 26, 14, 14, 14, 14,	/* 0x68~0x6f */
    4, 4, 4, 4, 4, 4, 4, 4,	/* 0x70~0x77 */
    4, 4, 4, 4, 4, 4, 4, 4,	/* 0x78~0x7f */
    0, 0, 0, 0, 3, 3, 4, 4,	/* 0x80~0x87 */
    2, 2, 2, 2, 2, 6, 2, 24,	/* 0x88~0x8f */
    4, 4, 4, 4, 4, 4, 4, 4,	/* 0x90~0x98 */
    2, 4, 31, 6, 13, 12, 3, 2,	/* 0x99~0x9f */
    8, 12, 9, 13, 14, 18, 22, 26,	/* 0xa0~0xa7 */
    3, 4, 10, 14, 12, 16, 15, 19,	/* 0xa8~0xaf */
    3, 3, 3, 3, 3, 3, 3, 3,	/* 0xb0~0xb7 */
    4, 4, 4, 4, 4, 4, 4, 4,	/* 0xb8~0xbf */
    0, 0, 22, 20, 26, 26, 12, 13,	/* 0xc0~0xc7 */
    0, 8, 33, 30, 45, 47, 4, 28,	/* 0xc8~0xcf */
    0, 0, 0, 0, 19, 15, 25, 15,	/* 0xd0~0xd7 */
    6, 6, 6, 6, 6, 6, 6, 6,	/* 0xd8~0xdf */
    6, 6, 6, 5, 10, 14, 9, 13,	/* 0xe0~0xe7 */
    19, 14, 14, 14, 8, 12, 7, 11,	/* 0xe8~0xef */
    2, 45, 0, 0, 2, 2, 0, 0,	/* 0xf0~0xf7 */
    2, 2, 2, 2, 2, 2, 0, 0	/* 0xf8~0xff */
};
const int Ci80x86::op_mem_states[] = {
    7, 11, 7, 11, 0, 0, 0, 0,	/* 0x00~0x07 */
    7, 11, 7, 11, 0, 0, 0, 0,	/* 0x08~0x0f */
    7, 11, 7, 11, 0, 0, 0, 0,	/* 0x10~0x17 */
    7, 11, 7, 11, 0, 0, 0, 0,	/* 0x18~0x1f */
    7, 11, 7, 11, 0, 0, 0, 0,	/* 0x20~0x27 */
    7, 11, 7, 11, 0, 0, 0, 0,	/* 0x28~0x2f */
    7, 11, 7, 11, 0, 0, 0, 0,	/* 0x30~0x37 */
    7, 11, 7, 11, 0, 0, 0, 0,	/* 0x38~0x3f */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0x40~0x47 */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0x48~0x4f */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0x50~0x57 */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0x58~0x5f */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0x60~0x67 */
    0, 14, 0, 10, 0, 0, 0, 0,	/* 0x68~0x6f */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0x70~0x77 */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0x78~0x7f */
    12, 16, 12, 16, 7, 11, 13, 17,	/* 0x80~0x87 */
    10, 14, 7, 11, 13, 0, 11, 0,	/* 0x88~0x8f */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0x90~0x97 */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0x98~0x9f */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0xa0~0xa7 */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0xa8~0xaf */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0xb0~0xb7 */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0xb8~0xbf */
    12, 12, 0, 0, 8, 8, 1, 1,	/* 0xc0~0xc7 */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0xc8~0xcf */
    13, 13, 12, 12, 0, 0, 0, 0,	/* 0xd0~0xd7 */
    6, 6, 6, 6, 6, 6, 6, 6,	/* 0xd8~0xdf */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0xe0~0xe7 */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0xe8~0xef */
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0xf0~0xf7 */
    0, 0, 0, 0, 0, 0, 0, 0	/* 0xf8~0xff */
};
const int Ci80x86::math_states[] = {
    3, 3, 3, 3, 3, 3, 3, 3
};
const int Ci80x86::math_mem_states[] = {
    10, 10, 10, 10, 10, 10, 10, 10
};
const int Ci80x86::shift_states[] = {
    5, 5, 5, 5, 5, 5, 45, 5
};
const int Ci80x86::shift_mem_states[] = {
    17, 17, 17, 17, 17, 17, 45, 17
};
const int Ci80x86::shift1_states[] = {
    2 - 1, 2 - 1, 2 - 1, 2 - 1, 2 - 1, 2 - 1, 45, 2 - 1
};
const int Ci80x86::shift1_mem_states[] = {
    15 - 1, 15 - 1, 15 - 1, 15 - 1, 15 - 1, 15 - 1, 45, 15 - 1
};
const int Ci80x86::math16s_states[] = {
    4, 45, 4, 4, 45, 4, 45, 4
};
const int Ci80x86::math16s_mem_states[] = {
    16, 45, 16, 16, 45, 16, 45, 16
};
const int Ci80x86::grp1_8_states[] = {
    4, 45, 3, 3, 27, 26, 29, 29
};
const int Ci80x86::grp1_8_mem_states[] = {
    10, 45, 3, 3, 33, 32, 35, 35
};
const int Ci80x86::grp1_16_mem_states[] = {
    10, 45, 3, 3, 42, 41, 44, 44
};
const int Ci80x86::grp1_16_states[] = {
    4, 45, 3, 3, 35, 35, 38, 38
};
const int Ci80x86::grp2_8_mem_states[] = {
    15, 15, 45, 45, 45, 45, 45, 45
};
const int Ci80x86::grp2_8_states[] = {
    3, 3, 45, 45, 45, 45, 45, 45
};
const int Ci80x86::grp2_16_states[] = {
    3, 3, 13, 38, 26, 11, 16, 45
};
const int Ci80x86::grp2_16_mem_states[] = {
    15, 15, 19, 38, 26, 17, 16, 45
};


const uint16 Ci80x86::parity[256] = {
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    MASK_PF, 0, 0, MASK_PF, 0, MASK_PF, MASK_PF, 0,
    0, MASK_PF, MASK_PF, 0, MASK_PF, 0, 0, MASK_PF
};

uint8 Ci80x86::getreg8(const I86stat *i86, uint8 reg)
{
    switch(reg & 0x38) {
    case 0x00: return AL;
    case 0x08: return CL;
    case 0x10: return DL;
    case 0x18: return BL;
    case 0x20: return AH;
    case 0x28: return CH;
    case 0x30: return DH;
    case 0x38: return BH;
    }
    return 0;
}

void Ci80x86::setreg8(I86stat *i86, uint8 reg, uint8 x)
{
    switch(reg & 0x38) {
    case 0x00: AL = x; break;
    case 0x08: CL = x; break;
    case 0x10: DL = x; break;
    case 0x18: BL = x; break;
    case 0x20: AH = x; break;
    case 0x28: CH = x; break;
    case 0x30: DH = x; break;
    case 0x38: BH = x; break;
    }
}

uint16 Ci80x86::getreg16(const I86stat *i86, uint8 reg)
{
    switch(reg & 0x38) {
    case 0x00: return AX;
    case 0x08: return CX;
    case 0x10: return DX;
    case 0x18: return BX;
    case 0x20: return SP;
    case 0x28: return BP;
    case 0x30: return SI;
    case 0x38: return DI;
    }
    return 0;
}

void Ci80x86::setreg16(I86stat *i86, uint8 reg, uint16 x)
{
    switch(reg & 0x38) {
    case 0x00: AX = x; break;
    case 0x08: CX = x; break;
    case 0x10: DX = x; break;
    case 0x18: BX = x; break;
    case 0x20: SP = x; break;
    case 0x28: BP = x; break;
    case 0x30: SI = x; break;
    case 0x38: DI = x; break;
    }
}

uint16 Ci80x86::getsreg(const I86stat *i86, uint8 reg)
{
    switch(reg & 0x18) {
    case 0x00: return ES;
    case 0x08: return CS;
    case 0x10: return SS;
    case 0x18: return DS;
    }
    return 0;
}

void Ci80x86::setsreg(I86stat *i86, uint8 reg, uint16 x)
{
    switch(reg & 0x18) {
    case 0x00: ES = x; break;
    case 0x08: CS = x; break;
    case 0x10: SS = x; break;
    case 0x18: DS = x; break;
    }
}

void Ci80x86::getsegoff(const I86stat *i86, uint8 rm, uint16 *seg, uint16 *off)
{
    uint16 _ds = (i86->r16.prefix == NULL ? DS: *i86->r16.prefix);
    uint16 _ss = (i86->r16.prefix == NULL ? SS: *i86->r16.prefix);

    switch(rm & 0xc7) {
    case 0x00: *seg = _ds; *off = BX + SI; break;
    case 0x01: *seg = _ds; *off = BX + DI; break;
    case 0x02: *seg = _ss; *off = BP + SI; break;
    case 0x03: *seg = _ss; *off = BP + DI; break;
    case 0x04: *seg = _ds; *off = SI; break;
    case 0x05: *seg = _ds; *off = DI; break;
    case 0x06: *seg = _ds; *off = CODE16(IP + 2); break;
    case 0x07: *seg = _ds; *off = BX; break;

    case 0x40: *seg = _ds; *off = BX + SI + (int8 )CODE8(IP + 2); break;
    case 0x41: *seg = _ds; *off = BX + DI + (int8 )CODE8(IP + 2); break;
    case 0x42: *seg = _ss; *off = BP + SI + (int8 )CODE8(IP + 2); break;
    case 0x43: *seg = _ss; *off = BP + DI + (int8 )CODE8(IP + 2); break;
    case 0x44: *seg = _ds; *off = SI + (int8 )CODE8(IP + 2); break;
    case 0x45: *seg = _ds; *off = DI + (int8 )CODE8(IP + 2); break;
    case 0x46: *seg = _ss; *off = BP + (int8 )CODE8(IP + 2); break;
    case 0x47: *seg = _ds; *off = BX + (int8 )CODE8(IP + 2); break;

    case 0x80: *seg = _ds; *off = BX + SI + (int16 )CODE16(IP + 2); break;
    case 0x81: *seg = _ds; *off = BX + DI + (int16 )CODE16(IP + 2); break;
    case 0x82: *seg = _ss; *off = BP + SI + (int16 )CODE16(IP + 2); break;
    case 0x83: *seg = _ss; *off = BP + DI + (int16 )CODE16(IP + 2); break;
    case 0x84: *seg = _ds; *off = SI + (int16 )CODE16(IP + 2); break;
    case 0x85: *seg = _ds; *off = DI + (int16 )CODE16(IP + 2); break;
    case 0x86: *seg = _ss; *off = BP + (int16 )CODE16(IP + 2); break;
    case 0x87: *seg = _ds; *off = BX + (int16 )CODE16(IP + 2); break;
    default: *seg = 0; *off = 0; break;
    }
}

uint8 Ci80x86::getrm8(const I86stat *i86, uint8 rm)
{
    uint16 seg, off;

    switch(rm & 0xc7) {
    case 0xc0: return AL;
    case 0xc1: return CL;
    case 0xc2: return DL;
    case 0xc3: return BL;
    case 0xc4: return AH;
    case 0xc5: return CH;
    case 0xc6: return DH;
    case 0xc7: return BH;
    default: getsegoff(i86, rm, &seg, &off); return MEM8(seg, off);
    }
    return 0;
}

void Ci80x86::setrm8(I86stat *i86, uint8 rm, uint8 x)
{
    uint16 seg, off;

    switch(rm & 0xc7) {
    case 0xc0: AL = x; break;
    case 0xc1: CL = x; break;
    case 0xc2: DL = x; break;
    case 0xc3: BL = x; break;
    case 0xc4: AH = x; break;
    case 0xc5: CH = x; break;
    case 0xc6: DH = x; break;
    case 0xc7: BH = x; break;
    default: getsegoff(i86, rm, &seg, &off); SET_MEM8(seg, off, x); break;
    }
}

uint16 Ci80x86::getrm16(const I86stat *i86, uint8 rm)
{
    uint16 seg, off;

    switch(rm & 0xc7) {
    case 0xc0: return AX;
    case 0xc1: return CX;
    case 0xc2: return DX;
    case 0xc3: return BX;
    case 0xc4: return SP;
    case 0xc5: return BP;
    case 0xc6: return SI;
    case 0xc7: return DI;
    default: getsegoff(i86, rm, &seg, &off); return MEM16(seg, off);
    }
    return 0;
}

void Ci80x86::setrm16(I86stat *i86, uint8 rm, uint16 x)
{
    uint16 seg, off;

    switch(rm & 0xc7) {
    case 0xc0: AX = x; break;
    case 0xc1: CX = x; break;
    case 0xc2: DX = x; break;
    case 0xc3: BX = x; break;
    case 0xc4: SP = x; break;
    case 0xc5: BP = x; break;
    case 0xc6: SI = x; break;
    case 0xc7: DI = x; break;
    default: getsegoff(i86, rm, &seg, &off); SET_MEM16(seg, off, x); break;
    }
}

void Ci80x86::i86reset(I86stat *i86)
{
    i86->r16.hlt = 0; halt = false;
    i86->r16.prefix = NULL;
    i86->r16.f = 0xf000;
    i86->r16.cs = 0xffff;
    i86->r16.ip = 0;

    i86->r16.ax =
    i86->r16.cx =
    i86->r16.dx =
    i86->r16.bx =
    i86->r16.sp =
    i86->r16.bp =
    i86->r16.si =
    i86->r16.di =
    i86->r16.ds =
    i86->r16.es =
    i86->r16.ss = 0;
}



int Ci80x86::i86nmi(I86stat *i86)
{
    _INT(0x02);
    i86->i.states -= 42;
    i86->r16.hlt = 0; halt = false;
    return TRUE;
}

int Ci80x86::i86int(I86stat *i86, int n)
{
//    if (fp_log) fprintf(fp_log,"INT %02x\n",n);

    if(!IF) {
        return FALSE;
    }

    _INT(n);
    i86->i.states -= 42;
    i86->r16.hlt = 0; halt = false;
    AddLog(LOG_MASTER,"INT TRUE");
    return TRUE;
}

int Ci80x86::i86exec(I86stat *i86)
{
    int _length, _states = 0;
    uint8 _op, _regrm;

    i86->i.total_states += i86->i.states;
    if(i86->i.total_states < 0)
        i86->i.total_states -= INT_MAX;

    if(i86->r16.hlt) {
        i86->i.states = 0;
        pPC->pTIMER->state += 4;
        return I86_HALT;
    }

    do {
        if(i86->i.trace)
            i86trace(i86);

        switch(FETCH_OP()) {
        case 0x00: FETCH_REGRM(); ADD8_RM_R(); break;
        case 0x01: FETCH_REGRM(); ADD16_RM_R(); break;
        case 0x02: FETCH_REGRM(); ADD8_R_RM(); break;
        case 0x03: FETCH_REGRM(); ADD16_R_RM(); break;
        case 0x04: ADD8_R(AL, IMM8); break;
        case 0x05: ADD16_R(AX, IMM16); break;
        case 0x06: PUSH(ES); break;
        case 0x07: POP_R(ES); break;

        case 0x08: FETCH_REGRM(); OR8_RM_R(); break;
        case 0x09: FETCH_REGRM(); OR16_RM_R(); break;
        case 0x0a: FETCH_REGRM(); OR8_R_RM(); break;
        case 0x0b: FETCH_REGRM(); OR16_R_RM(); break;
        case 0x0c: OR8_R(AL, IMM8); break;
        case 0x0d: OR16_R(AX, IMM16); break;
        case 0x0e: PUSH(CS); break;
        case 0x0f: ERR(); break;

        case 0x10: FETCH_REGRM(); ADC8_RM_R(); break;
        case 0x11: FETCH_REGRM(); ADC16_RM_R(); break;
        case 0x12: FETCH_REGRM(); ADC8_R_RM(); break;
        case 0x13: FETCH_REGRM(); ADC16_R_RM(); break;
        case 0x14: ADC8_R(AL, IMM8); break;
        case 0x15: ADC16_R(AX, IMM16); break;
        case 0x16: PUSH(SS); break;
        case 0x17: POP_R(SS); break;

        case 0x18: FETCH_REGRM(); SBB8_RM_R(); break;
        case 0x19: FETCH_REGRM(); SBB16_RM_R(); break;
        case 0x1a: FETCH_REGRM(); SBB8_R_RM(); break;
        case 0x1b: FETCH_REGRM(); SBB16_R_RM(); break;
        case 0x1c: SBB8_R(AL, IMM8); break;
        case 0x1d: SBB16_R(AX, IMM16); break;
        case 0x1e: PUSH(DS); break;
        case 0x1f: POP_R(DS); break;

        case 0x20: FETCH_REGRM(); AND8_RM_R(); break;
        case 0x21: FETCH_REGRM(); AND16_RM_R(); break;
        case 0x22: FETCH_REGRM(); AND8_R_RM(); break;
        case 0x23: FETCH_REGRM(); AND16_R_RM(); break;
        case 0x24: AND8_R(AL, IMM8); break;
        case 0x25: AND16_R(AX, IMM16); break;
        case 0x26: OVERRIDE(ES); continue;
        case 0x27: DAA(); break;

        case 0x28: FETCH_REGRM(); SUB8_RM_R(); break;
        case 0x29: FETCH_REGRM(); SUB16_RM_R(); break;
        case 0x2a: FETCH_REGRM(); SUB8_R_RM(); break;
        case 0x2b: FETCH_REGRM(); SUB16_R_RM(); break;
        case 0x2c: SUB8_R(AL, IMM8); break;
        case 0x2d: SUB16_R(AX, IMM16); break;
        case 0x2e: OVERRIDE(CS); continue;
        case 0x2f: DAS(); break;

        case 0x30: FETCH_REGRM(); XOR8_RM_R(); break;
        case 0x31: FETCH_REGRM(); XOR16_RM_R(); break;
        case 0x32: FETCH_REGRM(); XOR8_R_RM(); break;
        case 0x33: FETCH_REGRM(); XOR16_R_RM(); break;
        case 0x34: XOR8_R(AL, IMM8); break;
        case 0x35: XOR16_R(AX, IMM16); break;
        case 0x36: OVERRIDE(SS); continue;
        case 0x37: AAA(); break;

        case 0x38: FETCH_REGRM(); CMP8_RM_R(); break;
        case 0x39: FETCH_REGRM(); CMP16_RM_R(); break;
        case 0x3a: FETCH_REGRM(); CMP8_R_RM(); break;
        case 0x3b: FETCH_REGRM(); CMP16_R_RM(); break;
        case 0x3c: CMP8_R(AL, IMM8); break;
        case 0x3d: CMP16_R(AX, IMM16); break;
        case 0x3e: OVERRIDE(DS); continue;
        case 0x3f: AAS(); break;

        case 0x40: INC16_R(AX); break;
        case 0x41: INC16_R(CX); break;
        case 0x42: INC16_R(DX); break;
        case 0x43: INC16_R(BX); break;
        case 0x44: INC16_R(SP); break;
        case 0x45: INC16_R(BP); break;
        case 0x46: INC16_R(SI); break;
        case 0x47: INC16_R(DI); break;

        case 0x48: DEC16_R(AX); break;
        case 0x49: DEC16_R(CX); break;
        case 0x4a: DEC16_R(DX); break;
        case 0x4b: DEC16_R(BX); break;
        case 0x4c: DEC16_R(SP); break;
        case 0x4d: DEC16_R(BP); break;
        case 0x4e: DEC16_R(SI); break;
        case 0x4f: DEC16_R(DI); break;

        case 0x50: PUSH(AX); break;
        case 0x51: PUSH(CX); break;
        case 0x52: PUSH(DX); break;
        case 0x53: PUSH(BX); break;
        case 0x54: PUSH(SP); break;
        case 0x55: PUSH(BP); break;
        case 0x56: PUSH(SI); break;
        case 0x57: PUSH(DI); break;

        case 0x58: POP_R(AX); break;
        case 0x59: POP_R(CX); break;
        case 0x5a: POP_R(DX); break;
        case 0x5b: POP_R(BX); break;
        case 0x5c: POP_R(SP); break;
        case 0x5d: POP_R(BP); break;
        case 0x5e: POP_R(SI); break;
        case 0x5f: POP_R(DI); break;

        case 0x60: PUSHA(); break;
        case 0x61: POPA(); break;
        case 0x62: FETCH_REGRM(); BOUND(); break;
        case 0x63: ERR(); break;
        case 0x64: ERR(); break;
        case 0x65: ERR(); break;
        case 0x66: ERR(); break;
        case 0x67: ERR(); break;

        case 0x68: PUSH(IMM16); break;
        case 0x69: FETCH_REGRM(); IMUL16_RM_IMM16(); break;
        case 0x6a: PUSH(IMM16S); break;
        case 0x6b: FETCH_REGRM(); IMUL16_RM_IMM8(); break;
        case 0x6c: INS8(); break;
        case 0x6d: INS16(); break;
        case 0x6e: OUTS8(); break;
        case 0x6f: OUTS16(); break;

        case 0x70: JMP_COND(OF, SHORT_LABEL); break; /* jo */
        case 0x71: JMP_COND(!OF, SHORT_LABEL); break; /* jno */
        case 0x72: JMP_COND(CF, SHORT_LABEL); break; /* jb/jnae */
        case 0x73: JMP_COND(!CF, SHORT_LABEL); break; /* jnb/jae */
        case 0x74: JMP_COND(ZF, SHORT_LABEL); break; /* jz/je */
        case 0x75: JMP_COND(!ZF, SHORT_LABEL); break; /* jnz/jne */
        case 0x76: JMP_COND(CF || ZF, SHORT_LABEL); break; /* jbe/jna */
        case 0x77: JMP_COND(!CF && !ZF, SHORT_LABEL); break; /* ja/jnbe */

        case 0x78: JMP_COND(SF, SHORT_LABEL); break; /* js */
        case 0x79: JMP_COND(!SF, SHORT_LABEL); break; /* jns */
        case 0x7a: JMP_COND(PF, SHORT_LABEL); break; /* jp/jpe */
        case 0x7b: JMP_COND(!PF, SHORT_LABEL); break; /* jnp/jpo */
        case 0x7c: JMP_COND((SF && !OF) || (!SF && OF), SHORT_LABEL); break; /* jl/jnge */
        case 0x7d: JMP_COND((SF && OF) || (!SF && !OF), SHORT_LABEL); break; /* jnl/jge */
        case 0x7e: JMP_COND((SF && !OF) || (!SF && OF) || ZF, SHORT_LABEL); break; /* jle/jng */
        case 0x7f: JMP_COND(((SF && OF) || (!SF && !OF)) && !ZF, SHORT_LABEL); break; /* jnle/jg */

        case 0x80:
        case 0x82:
            switch(FETCH_MATH8()) {
            case 0x00: ADD8_RM_IMM(); break;
            case 0x08: OR8_RM_IMM(); break;
            case 0x10: ADC8_RM_IMM(); break;
            case 0x18: SBB8_RM_IMM(); break;
            case 0x20: AND8_RM_IMM(); break;
            case 0x28: SUB8_RM_IMM(); break;
            case 0x30: XOR8_RM_IMM(); break;
            case 0x38: CMP8_RM_IMM(); break;
            }
            break;
        case 0x81:
            switch(FETCH_MATH16()) {
            case 0x00: ADD16_RM_IMM(); break;
            case 0x08: OR16_RM_IMM(); break;
            case 0x10: ADC16_RM_IMM(); break;
            case 0x18: SBB16_RM_IMM(); break;
            case 0x20: AND16_RM_IMM(); break;
            case 0x28: SUB16_RM_IMM(); break;
            case 0x30: XOR16_RM_IMM(); break;
            case 0x38: CMP16_RM_IMM(); break;
            }
            break;
        case 0x83:
            switch(FETCH_MATH16S()) {
            case 0x00: ADD16S_RM_IMM(); break;
            case 0x08: OR16S_RM_IMM(); break;
            case 0x10: ADC16S_RM_IMM(); break;
            case 0x18: SBB16S_RM_IMM(); break;
            case 0x20: AND16S_RM_IMM(); break;
            case 0x28: SUB16S_RM_IMM(); break;
            case 0x30: XOR16S_RM_IMM(); break;
            case 0x38: CMP16S_RM_IMM(); break;
            }
            break;
        case 0x84: FETCH_REGRM(); TEST8_RM_R(); break;
        case 0x85: FETCH_REGRM(); TEST16_RM_R(); break;
        case 0x86: FETCH_REGRM(); XCHG8_R_RM(); break;
        case 0x87: FETCH_REGRM(); XCHG16_R_RM(); break;

        case 0x88: FETCH_REGRM(); MOV8_RM_R(); break;
        case 0x89: FETCH_REGRM(); MOV16_RM_R(); break;
        case 0x8a: FETCH_REGRM(); MOV8_R_RM(); break;
        case 0x8b: FETCH_REGRM(); MOV16_R_RM(); break;
        case 0x8c: FETCH_REGRM(); MOV16_RM_SREG(); break;
        case 0x8d: FETCH_REGRM(); LEA(); break;
        case 0x8e: FETCH_REGRM(); MOV16_SREG_RM(); break;
        case 0x8f: FETCH_REGRM(); POP_RM(); break;

        case 0x90: XCHG16_R(AX, AX); break;
        case 0x91: XCHG16_R(AX, CX); break;
        case 0x92: XCHG16_R(AX, DX); break;
        case 0x93: XCHG16_R(AX, BX); break;
        case 0x94: XCHG16_R(AX, SP); break;
        case 0x95: XCHG16_R(AX, BP); break;
        case 0x96: XCHG16_R(AX, SI); break;
        case 0x97: XCHG16_R(AX, DI); break;

        case 0x98: CBW(); break;
        case 0x99: CWD(); break;
        case 0x9a: CALL_FAR(CODE16(IP + 1), CODE16(IP + 3)); break;
        case 0x9b: WAIT(); break;
        case 0x9c: PUSH(F); break;
        case 0x9d: POP_R(F); break;
        case 0x9e: SAHF(); break;
        case 0x9f: LAHF(); break;

        case 0xa0: MOV8_R_M(AL, IMM16); break;
        case 0xa1: MOV16_R_M(AX, IMM16); break;
        case 0xa2: MOV8_M(IMM16, AL); break;
        case 0xa3: MOV16_M(IMM16, AX); break;
        case 0xa4: MOVSB(); break;
        case 0xa5: MOVSW(); break;
        case 0xa6: CMPSB(); break;
        case 0xa7: CMPSW(); break;

        case 0xa8: TEST8_R(AL, IMM8); break;
        case 0xa9: TEST16_R(AX, IMM16); break;
        case 0xaa: STOSB(); break;
        case 0xab: STOSW(); break;
        case 0xac: LODSB(); break;
        case 0xad: LODSW(); break;
        case 0xae: SCASB(); break;
        case 0xaf: SCASW(); break;

        case 0xb0: MOV8_R(AL, IMM8); break;
        case 0xb1: MOV8_R(CL, IMM8); break;
        case 0xb2: MOV8_R(DL, IMM8); break;
        case 0xb3: MOV8_R(BL, IMM8); break;
        case 0xb4: MOV8_R(AH, IMM8); break;
        case 0xb5: MOV8_R(CH, IMM8); break;
        case 0xb6: MOV8_R(DH, IMM8); break;
        case 0xb7: MOV8_R(BH, IMM8); break;

        case 0xb8: MOV16_R(AX, IMM16); break;
        case 0xb9: MOV16_R(CX, IMM16); break;
        case 0xba: MOV16_R(DX, IMM16); break;
        case 0xbb: MOV16_R(BX, IMM16); break;
        case 0xbc: MOV16_R(SP, IMM16); break;
        case 0xbd: MOV16_R(BP, IMM16); break;
        case 0xbe: MOV16_R(SI, IMM16); break;
        case 0xbf: MOV16_R(DI, IMM16); break;

        case 0xc0:
            switch(FETCH_SHIFT8()) {
            case 0x00: ROL8_IMM(); break;
            case 0x08: ROR8_IMM(); break;
            case 0x10: RCL8_IMM(); break;
            case 0x18: RCR8_IMM(); break;
            case 0x30:
            case 0x20: SHL8_IMM(); break;
            case 0x28: SHR8_IMM(); break;
            case 0x38: SAR8_IMM(); break;
            }
            break;
        case 0xc1:
            switch(FETCH_SHIFT16()) {
            case 0x00: ROL16_IMM(); break;
            case 0x08: ROR16_IMM(); break;
            case 0x10: RCL16_IMM(); break;
            case 0x18: RCR16_IMM(); break;
            case 0x30:
            case 0x20: SHL16_IMM(); break;
            case 0x28: SHR16_IMM(); break;
            case 0x38: SAR16_IMM(); break;
            }
            break;
        case 0xc2: RET(IMM16); break;
        case 0xc3: RET(0); break;
        case 0xc4: FETCH_REGRM(); LES(); break;
        case 0xc5: FETCH_REGRM(); LDS(); break;
        case 0xc6: FETCH_REGRM(); MOV8_RM_IMM(); break;
        case 0xc7: FETCH_REGRM(); MOV16_RM_IMM(); break;

        case 0xc8: ENTER(CODE16(IP + 1), CODE8(IP + 3)); break;
        case 0xc9: LEAVE(); break;
        case 0xca: RETF(IMM16); break;
        case 0xcb: RETF(0); break;
        case 0xcc: INT(0x03); break;
        case 0xcd: INT(IMM8); break;
        case 0xce: INTO(); break;
        case 0xcf: IRET(); break;

        case 0xd0:
            switch(FETCH_SHIFT8_1()) {
            case 0x00: ROL8_1(); break;
            case 0x08: ROR8_1(); break;
            case 0x10: RCL8_1(); break;
            case 0x18: RCR8_1(); break;
            case 0x30:
            case 0x20: SHL8_1(); break;
            case 0x28: SHR8_1(); break;
            case 0x38: SAR8_1(); break;
            }
            break;
        case 0xd1:
            switch(FETCH_SHIFT16_1()) {
            case 0x00: ROL16_1(); break;
            case 0x08: ROR16_1(); break;
            case 0x10: RCL16_1(); break;
            case 0x18: RCR16_1(); break;
            case 0x30:
            case 0x20: SHL16_1(); break;
            case 0x28: SHR16_1(); break;
            case 0x38: SAR16_1(); break;
            }
            break;
        case 0xd2:
            switch(FETCH_SHIFT8_CL()) {
            case 0x00: ROL8_CL(); break;
            case 0x08: ROR8_CL(); break;
            case 0x10: RCL8_CL(); break;
            case 0x18: RCR8_CL(); break;
            case 0x30:
            case 0x20: SHL8_CL(); break;
            case 0x28: SHR8_CL(); break;
            case 0x38: SAR8_CL(); break;
            }
            break;
        case 0xd3:
            switch(FETCH_SHIFT16_CL()) {
            case 0x00: ROL16_CL(); break;
            case 0x08: ROR16_CL(); break;
            case 0x10: RCL16_CL(); break;
            case 0x18: RCR16_CL(); break;
            case 0x30:
            case 0x20: SHL16_CL(); break;
            case 0x28: SHR16_CL(); break;
            case 0x38: SAR16_CL(); break;
            }
            break;
        case 0xd4: AAM(IMM8); break;
        case 0xd5: AAD(IMM8); break;
        case 0xd6: ERR(); break;
        case 0xd7: XLAT(); break;

        case 0xd8: FETCH_REGRM(); ESC(0); break;
        case 0xd9: FETCH_REGRM(); ESC(1); break;
        case 0xda: FETCH_REGRM(); ESC(2); break;
        case 0xdb: FETCH_REGRM(); ESC(3); break;
        case 0xdc: FETCH_REGRM(); ESC(4); break;
        case 0xdd: FETCH_REGRM(); ESC(5); break;
        case 0xde: FETCH_REGRM(); ESC(6); break;
        case 0xdf: FETCH_REGRM(); ESC(7); break;

        case 0xe0: LOOP(!ZF, SHORT_LABEL); break;
        case 0xe1: LOOP(ZF, SHORT_LABEL); break;
        case 0xe2: LOOP(TRUE, SHORT_LABEL); break;
        case 0xe3: JCXZ(SHORT_LABEL); break;
        case 0xe4: IN8(AL, IMM8); break;
        case 0xe5: IN16(AX, IMM8); break;
        case 0xe6: OUT8(IMM8, AL); break;
        case 0xe7: OUT16(IMM8, AX); break;

        case 0xe8: CALL_NEAR(NEAR_LABEL); break;
        case 0xe9: JMP_NEAR(NEAR_LABEL); break;
        case 0xea: JMP_FAR(CODE16(IP + 1), CODE16(IP + 3)); break;
        case 0xeb: JMP_NEAR(SHORT_LABEL); break;
        case 0xec: IN8(AL, DX); break;
        case 0xed: IN16(AX, DX); break;
        case 0xee: OUT8(DX, AL); break;
        case 0xef: OUT16(DX, AX); break;

        case 0xf0: LOCK(); break;
        case 0xf1: ERR(); break;
        case 0xf2: REP(!ZF); break;
        case 0xf3: REP(ZF); break;
        case 0xf4: HLT(); i86->i.total_states -= i86->i.states; i86->r16.prefix = NULL; return I86_HALT;
        case 0xf5: CMC(); break;
        case 0xf6:
            switch(FETCH_GRP1_8()) {
            case 0x00: TEST8_RM_IMM(); break;
            case 0x10: NOT8_RM(); break;
            case 0x18: NEG8_RM(); break;
            case 0x20: MUL8_RM(); break;
            case 0x28: IMUL8_RM(); break;
            case 0x30: DIV8_RM(); break;
            case 0x38: IDIV8_RM(); break;
            default: ERR(); break;
            }
            break;
        case 0xf7:
            switch(FETCH_GRP1_16()) {
            case 0x00: TEST16_RM_IMM(); break;
            case 0x10: NOT16_RM(); break;
            case 0x18: NEG16_RM(); break;
            case 0x20: MUL16_RM(); break;
            case 0x28: IMUL16_RM(); break;
            case 0x30: DIV16_RM(); break;
            case 0x38: IDIV16_RM(); break;
            default: ERR(); break;
            }
            break;

        case 0xf8: CLC(); break;
        case 0xf9: STC(); break;
        case 0xfa: CLI(); break;
        case 0xfb: STI(); break;
        case 0xfc: CLD(); break;
        case 0xfd: STD(); break;
        case 0xfe:
            switch(FETCH_GRP2_8()) {
            case 0x00: INC8_RM(); break;
            case 0x08: DEC8_RM(); break;
            default: ERR(); break;
            }
            break;
        case 0xff:
            switch(FETCH_GRP2_16()) {
            case 0x00: INC16_RM(); break;
            case 0x08: DEC16_RM(); break;
            case 0x10: CALL_NEAR_RM(); break;
            case 0x18: CALL_FAR_RM(); break;
            case 0x20: JMP_NEAR_RM(); break;
            case 0x28: JMP_FAR_RM(); break;
            case 0x30: PUSH_RM(); break;
            default: ERR(); break;
            }
            break;
        }

        if(TF && IF && _op != 0xcf) {
            switch(CODE8(IP)) {
            case 0x07: /* pop ES */
            case 0x17: /* pop SS */
            case 0x1f: /* pop DS */
            case 0x8e: /* mov SEGREG,reg16/mem16 */
            case 0x9b: /* wait */
                break;
            default:
                TRAP();
                break;
            }
        }

        i86->r16.prefix = NULL;
        i86->i.states -= _states;
        pPC->pTIMER->state += _states;
        _states = 0;
    } while(i86->i.states > 0);

    i86->i.total_states -= i86->i.states;
    return I86_RUN;
}

uint8 Ci80x86::i86read8(const I86stat *cpu, uint16 seg, uint16 off)
{
    UINT32 p = (((int )seg << 4) + off) & 0xfffff;

    return pPC->Get_8(p);

}

void Ci80x86::i86write8(I86stat *cpu, uint16 seg, uint16 off, uint8 v)
{
    UINT32 p = (((int )seg << 4) + off) & 0xfffff;

    pPC->Set_8(p,v);

}

uint16 Ci80x86::i86read16(const I86stat *cpu, uint16 seg, uint16 off)
{
    return ((uint16 )i86read8(cpu, seg, off + 1) << 8) + (uint16 )i86read8(cpu, seg, off);
}

void Ci80x86::i86write16(I86stat *cpu, uint16 seg, uint16 off, uint16 v)
{
    i86write8(cpu, seg, off + 0, v & 0xff);
    i86write8(cpu, seg, off + 1, v >> 8);
}

uint16 Ci80x86::i86inp16(const I86stat *i86, uint16 port)
{
#if 1
    return ((uint16 )i86inp8(i86, port + 1) << 8) | i86inp8(i86, port);
#else
    return pPC->in16(port);
#endif
}

void Ci80x86::i86out16(I86stat *i86, uint16 port, uint16 x)
{
#if 0
    i86out8(i86, port + 1, x >> 8);
    i86out8(i86, port, x & 0xff);
#else
    pPC->out16(port,x);
#endif
}

uint8 Ci80x86::i86inp8(const I86stat *i86, uint16 port)
{
    return pPC->in8(port);
//    return imem[address];
}

void Ci80x86::i86out8(I86stat *i86, uint16 port, uint8 x)
{
    pPC->out8(port,x);
}

void Ci80x86::i86trace(const I86stat *)
{
}

/*
    Copyright 2009 maruhiro
    All rights reserved.

    Redistribution and use in source and binary forms,
    with or without modification, are permitted provided that
    the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
    FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
    THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
    OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* eof */



Ci80x86::Ci80x86(CPObject * parent): CCPU(parent)
{


    fn_log="i80x86.log";

    regwidget = (CregCPU*) new Cregsz80Widget(0,this);

}

Ci80x86::~Ci80x86()
{
}

bool Ci80x86::init()
{
    Check_Log();
    pDEBUG = new Cdebug_i80x86(pPC);
    Reset();
    return true;
}

bool Ci80x86::exit()
{
}


UINT32 Ci80x86::get_PC()
{
    return (i86stat.r16.cs<<16) | (i86stat.r16.ip);
    //    return (((int )i86->r16.cs << 4) + i86->r16.ip) & 0xfffff;
}

void Ci80x86::Regs_Info(UINT8 Type)
{
    char buf[0x100];

    switch(Type)
    {
    case 0:
        sprintf(
        Regs_String,
        "AX=%04x  BX=%04x  CX=%04x  DX=%04x  SP=%04x  BP=%04x  SI=%04x  DI=%04x\n"
        "DS=%04x  ES=%04x  SS=%04x  CS=%04x  IP=%04x  %s %s %s %s %s %s %s %s",
        i86stat.r16.ax,
        i86stat.r16.bx,
        i86stat.r16.cx,
        i86stat.r16.dx,
        i86stat.r16.sp,
        i86stat.r16.bp,
        i86stat.r16.si,
        i86stat.r16.di,
        i86stat.r16.ds,
        i86stat.r16.es,
        i86stat.r16.ss,
        i86stat.r16.cs,
        i86stat.r16.ip,
        i86stat.r16.f & 0x0800 ? "OV": "NV",
        i86stat.r16.f & 0x0400 ? "DN": "UP",
        i86stat.r16.f & 0x0200 ? "EI": "DI",
        i86stat.r16.f & 0x0080 ? "NG": "PL",
        i86stat.r16.f & 0x0040 ? "ZR": "NZ",
        i86stat.r16.f & 0x0010 ? "AC": "NA",
        i86stat.r16.f & 0x0004 ? "PE": "PO",
        i86stat.r16.f & 0x0001 ? "CY": "NC"
        );
        break;
    case 1:
        sprintf(buf,"X:");
        for(int i = 0x400; i <= 0x40f; i++)
            sprintf(buf,"%s%02x ", buf,pPC->mem[i]);
        sprintf(buf,"%s Y:",buf);
        for(int i = 0x410; i <= 0x41f; i++)
            sprintf(buf,"%s%02x ",buf, pPC->mem[i]);
        sprintf(buf,"%s W:",buf);
        for(int i = 0x420; i <= 0x42f; i++)
            sprintf(buf,"%s%02x ", buf,pPC->mem[i]);

        sprintf(
        Regs_String,
        "AX=%04x  BX=%04x  CX=%04x  DX=%04x  SP=%04x  BP=%04x  SI=%04x  DI=%04x "
        "DS=%04x  ES=%04x  SS=%04x  CS=%04x  IP=%04x  %s %s %s %s %s %s %s %s %s",
        i86stat.r16.ax,
        i86stat.r16.bx,
        i86stat.r16.cx,
        i86stat.r16.dx,
        i86stat.r16.sp,
        i86stat.r16.bp,
        i86stat.r16.si,
        i86stat.r16.di,
        i86stat.r16.ds,
        i86stat.r16.es,
        i86stat.r16.ss,
        i86stat.r16.cs,
        i86stat.r16.ip,
        i86stat.r16.f & 0x0800 ? "OV": "NV",
        i86stat.r16.f & 0x0400 ? "DN": "UP",
        i86stat.r16.f & 0x0200 ? "EI": "DI",
        i86stat.r16.f & 0x0080 ? "NG": "PL",
        i86stat.r16.f & 0x0040 ? "ZR": "NZ",
        i86stat.r16.f & 0x0010 ? "AC": "NA",
        i86stat.r16.f & 0x0004 ? "PE": "PO",
        i86stat.r16.f & 0x0001 ? "CY": "NC",
                    buf
        );
        break;
    }



}

void	Ci80x86::step(void)
{
    int ret = i86exec(&i86stat);

}

void Ci80x86::Reset()
{
    i86reset(&i86stat);
}


void Ci80x86::save_internal(QXmlStreamWriter *)
{
}


void Ci80x86::Load_Internal(QXmlStreamReader *)
{
}
