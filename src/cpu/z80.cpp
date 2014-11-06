/*
	Zilog Z80 Emulator
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "z80.h"
#include "z80_dasm.h"
#include "Log.h"
#include "Dasm.h"
#include "Inter.h"
#include "Keyb.h"
#include "dialoglog.h"
#include "ui/cregsz80widget.h"

#define IMEM_LEN    0x200


#define A	z->r.a	/* �A�L�������[�^ */
#define F	z->r.f	/* �t���O */
#define B	z->r.b	/* �ėp���W�X�^B */
#define C	z->r.c	/* �ėp���W�X�^C */
#define D	z->r.d	/* �ėp���W�X�^D */
#define E	z->r.e	/* �ėp���W�X�^E */
#define H	z->r.h	/* �ėp���W�X�^H */
#define L	z->r.l	/* �ėp���W�X�^L */
#define R	((rnd = rnd * 8197 + 1) >> 25)	/* ���t���b�V�����W�X�^ */
#define I	z->r.i	/* �C���^���v�g���W�X�^I */
#define IXH	z->r.ixh	/* �C���f�b�N�X���W�X�^IXh */
#define IXL	z->r.ixl	/* �C���f�b�N�X���W�X�^IXl */
#define IYH	z->r.iyh	/* �C���f�b�N�X���W�X�^IYh */
#define IYL	z->r.iyl	/* �C���f�b�N�X���W�X�^IYl */
#define AF	z->r16.af	/* �y�A���W�X�^AF */
#define BC	z->r16.bc	/* �y�A���W�X�^BC */
#define DE	z->r16.de	/* �y�A���W�X�^DE */
#define HL	z->r16.hl	/* �y�A���W�X�^HL */
#define IX	z->r16.ix	/* �C���f�b�N�X���W�X�^IX */
#define IY	z->r16.iy	/* �C���f�b�N�X���W�X�^IY */
#define SP	z->r16.sp	/* �X�^�b�N�|�C���^SP */
#define PC	z->r16.pc	/* �v���O�����J�E���^PC */
#define AF_D	z->r16.af_d	/* �⏕���W�X�^AF' */
#define BC_D	z->r16.bc_d	/* �⏕���W�X�^BC' */
#define DE_D	z->r16.de_d	/* �⏕���W�X�^DE' */
#define HL_D	z->r16.hl_d	/* �⏕���W�X�^HL' */

/* �t���O */
#define MASK_CY	0x01	/* �L�����[�t���O �}�X�N */
#define MASK_N	0x02	/* ���Z�t���O �}�X�N */
#define MASK_PV	0x04	/* �p���e�B/�I�[�o�[�t���[�t���O �}�X�N */
#define MASK_HC	0x10	/* �n�[�t�L�����[�t���O �}�X�N */
#define MASK_Z	0x40	/* �[���t���O �}�X�N */
#define MASK_S	0x80	/* �T�C���t���O �}�X�N */
#define CY	(z->r.f & MASK_CY)	/* �L�����[�t���O */
#define N	(z->r.f & MASK_N)	/* ���Z�t���O */
#define PV	(z->r.f & MASK_PV)	/* �p���e�B/�I�[�o�[�t���[�t���O */
#define HC	(z->r.f & MASK_HC)	/* �n�[�t�L�����[�t���O */
#define Z	(z->r.f & MASK_Z)	/* �[���t���O */
#define S	(z->r.f & MASK_S)	/* �T�C���t���O */
#define X	(z->r.f & MASK_X)
#define Y	(z->r.f & MASK_Y)
#define CC_NZ	!(z->r.f & MASK_Z)	/* ���� �[���t���O OFF */
#define CC_Z	(z->r.f & MASK_Z)	/* ���� �[���t���O ON */
#define CC_NC	!(z->r.f & MASK_CY)	/* ���� �L�����[�t���O OFF */
#define CC_C	(z->r.f & MASK_CY)	/* ���� �L�����[�t���O ON */
#define CC_PO	!(z->r.f & MASK_PV)	/* ���� �p���e�B/�I�[�o�[�t���[�t���O OFF */
#define CC_PE	(z->r.f & MASK_PV)	/* ���� �p���e�B/�I�[�o�[�t���[�t���O ON */
#define CC_P	!(z->r.f & MASK_S)	/* ���� ���Z�t���O OFF */
#define CC_M	(z->r.f & MASK_S)	/* ���� ���Z�t���O ON */

/* ������ */
#define MEM8(x)	z80read8(z, x)	/* 8bits������ */
#define IMM8	z80read8(z, z->r16.pc + 1)	/* 8bits���l */
#define EQ	((int8 )z80read8(z, z->r16.pc + 1))	/* ���΃A�h���X */
#define STORE8(x, y)	z80write8(z, x, y)	/* 8bits�������� */
#define MEM16(x)	z80read16(z, x)	/* 16bits������ */
#define IMM16	z80read16(z, z->r16.pc + 1)	/* 16bits���l */
#define STORE16(x, y)	z80write16(z, x, y)	/* 16bits�������� */

/* �L�����[�t���O�Z�b�g */
#define SET_CY8(acc)	((acc) & 0x00000100L ? MASK_CY: 0)
#define SET_CY16(acc)	((acc) & 0x00010000L ? MASK_CY: 0)
#define SET_CYS(acc)	((acc) & 0x80000000L ? MASK_CY: 0)

/* �p���e�B/�I�[�o�[�t���[�t���O�Z�b�g */
#define SET_P(acc)	parity[(acc) & 0xff]
#define SET_V8(acc, a, x)	(((a) ^ (x)) & 0x80   ? 0: (((a) ^ acc) & 0x80   ? MASK_PV: 0))
#define SET_V16(acc, x, y)	(((x) ^ (y)) & 0x8000 ? 0: (((x) ^ acc) & 0x8000 ? MASK_PV: 0))
#define SET_VS8(acc, a, x)	(((a) ^ (x)) & 0x80   ? (((a) ^ acc) & 0x80   ? MASK_PV: 0): 0)
#define SET_VS16(acc, x, y)	(((x) ^ (y)) & 0x8000 ? (((x) ^ acc) & 0x8000 ? MASK_PV: 0): 0)

/* �n�[�t�L�����[�Z�b�g */
#define SET_HC8_CY(a, x, cy)	((((a) & 0x0f) + ((x) & 0x0f) + cy) & 0x10)
#define SET_HC8(a, x)	SET_HC8_CY(a, x, 0)
#define SET_HCS8_CY(a, x, cy)	((((a) & 0x0f) - ((x) & 0x0f) - cy) & 0x10)
#define SET_HCS8(a, x)	SET_HCS8_CY(a, x, 0)
#define SET_HC16_CY(x, y, cy)	((((x) & 0x0fff) + ((y) & 0x0fff) + cy) & 0x1000 ? MASK_HC: 0)
#define SET_HC16(x, y)	SET_HC16_CY(x, y, 0)
#define SET_HCS16_CY(x, y, cy)	((((x) & 0x0fff) - ((y) & 0x0fff) - cy) & 0x1000 ? MASK_HC: 0)
#define SET_HCS16(x, y)	SET_HCS16_CY(x, y, 0)

/* �[���t���O�Z�b�g */
#define SET_Z8(acc)	((acc) & 0x000000ffL ? 0: MASK_Z)
#define SET_Z16(acc)	((acc) & 0x0000ffffL ? 0: MASK_Z)

/* �T�C���t���O�Z�b�g */
#define SET_S8(acc)	((acc) & 0x00000080L ? MASK_S: 0)
#define SET_S16(acc)	((acc) & 0x00008000L ? MASK_S: 0)

/* ���߃t�F�b�` */
#define FETCH(state_table, len_table, off)	(op = z80read8(z, z->r16.pc + off), _state = state_table[op], _length = len_table[op], op)
#define FETCH_XX()	FETCH(state_xx, len_xx, 0)
#define FETCH_CB_XX()	FETCH(state_cb_xx, len_cb_xx, 0)
#define FETCH_DD_XX()	FETCH(state_dd_xx, len_dd_xx, 0)
#define FETCH_DD_CB_XX()	FETCH(state_dd_cb_xx, len_dd_cb_xx, 2)
#define FETCH_ED_XX()	FETCH(state_ed_xx, len_ed_xx, 0)
#define FETCH_FD_XX()	FETCH(state_dd_xx, len_dd_xx, 0)
#define FETCH_FD_CB_XX()	FETCH(state_dd_cb_xx, len_dd_cb_xx, 2)

/* ���� */
#define ADC8(x) \
	{ \
		uint32 _acc = (uint32 )A + (x) + CY; \
		F = SET_CY8(_acc) | SET_V8(_acc, A, x) | SET_HC8_CY(A, x, CY) | SET_Z8(_acc) | SET_S8(_acc); \
		A = _acc; \
	} \
	PC += _length

#define ADC16(x, y) \
	{ \
		uint32 _acc = (uint32 )(x) + (y) + CY; \
		F = SET_CY16(_acc) | SET_V16(_acc, x, y) | SET_HC16_CY(x, y, CY) | SET_Z16(_acc) | SET_S16(_acc); \
		(x) = _acc; \
	} \
	PC += _length

#define ADD8(x) \
	{ \
		uint32 _acc = (uint32 )A + (x); \
		F = SET_CY8(_acc) | SET_V8(_acc, A, x) | SET_HC8(A, x) | SET_Z8(_acc) | SET_S8(_acc); \
		A = _acc; \
	} \
	PC += _length

#define ADD16(x, y) \
	{ \
		uint32 _acc = (uint32 )(x) + (y); \
		F = SET_CY16(_acc) | Z | PV | S | SET_HC16(x, y); \
		(x) = _acc; \
	} \
	PC += _length

#define AND(x) \
	{ \
		uint32 _acc = (uint32 )A & (x); \
		F = SET_P(_acc) | MASK_HC | SET_Z8(_acc) | SET_S8(_acc); \
		A = _acc; \
	} \
	PC += _length

#define BIT(x, y) \
	F = CY | ((y) & (1 << (x)) ? 0: MASK_PV) | MASK_HC | ((y) & (1 << (x)) ? 0: MASK_Z) | ((y) & 0x80 & (1 << (x))); \
	PC += _length

#define CALL(x, y) \
	if(x) { \
/*        int s; */\
		SP -= 2; \
		STORE16(SP, PC + _length); \
		PC = y; \
		_state += 7; \
        CallSubLevel++ ; \
/*		if(z->i.emulate_subroutine && (s = z80subroutine(z, PC)) >= 0) { \
            PC = MEM16(SP); \
            SP += 2; \
            if(SP > z->i.stack_under) \
                return Z80_UNDERFLOW; \
            _state += s; \
        } */\
	} else \
		PC += _length;

#define CCF() \
	F = (CY ^ MASK_CY) | PV | (CY ? MASK_HC: 0) | Z | S; \
	PC += _length

#define CP(x) \
	{ \
		uint32 _acc = (uint32 )A - (x); \
		F = SET_CYS(_acc) | MASK_N | SET_VS8(_acc, A, x) | SET_HCS8(A, x) | SET_Z8(_acc) | SET_S8(_acc); \
	} \
	PC += _length

#define CPD() \
	{ \
		uint32 _acc = (uint32 )A - MEM8(HL); \
		F = SET_CYS(_acc) | MASK_N | SET_VS8(_acc, A, MEM8(HL)) | SET_HCS8(A, MEM8(HL)) | SET_Z8(_acc) | SET_S8(_acc); \
		BC--; \
		HL--; \
	} \
	PC += _length

#define CPDR() \
	{ \
		uint32 _acc; \
		do { \
			_acc = (uint32 )A - MEM8(HL); \
			F = SET_CYS(_acc) | MASK_N | SET_VS8(_acc, A, MEM8(HL)) | SET_HCS8(A, MEM8(HL)) | SET_Z8(_acc) | SET_S8(_acc); \
			BC--; \
			HL--; \
			_state += 21; \
		} while(BC && _acc); \
		_state -= 5; \
	} \
	PC += _length

#define CPI() \
	{ \
		uint32 _acc = (uint32 )A - MEM8(HL); \
		F = SET_CYS(_acc) | MASK_N | SET_VS8(_acc, A, MEM8(HL)) | SET_HCS8(A, MEM8(HL)) | SET_Z8(_acc) | SET_S8(_acc); \
		BC--; \
		HL++; \
	} \
	PC += _length

#define CPIR() \
	{ \
		uint32 _acc; \
		do { \
			_acc = (uint32 )A - MEM8(HL); \
			F = SET_CYS(_acc) | MASK_N | SET_VS8(_acc, A, MEM8(HL)) | SET_HCS8(A, MEM8(HL)) | SET_Z8(_acc) | SET_S8(_acc); \
			BC--; \
			HL++; \
			_state += 21; \
		} while(BC && _acc); \
	} \
	_state -= 5; \
	PC += _length

#define CPL() \
	{ \
		uint32 _acc = ~A; \
		F |= SET_S8(_acc) | MASK_N | MASK_HC; \
		A = _acc; \
	} \
	PC += _length

#define DAA() \
	{ \
		uint32 _acc; \
		uint8 x, c; \
		daa_result(&x, &c, A, F); \
		_acc = (uint32 )A + x; \
		F = c | N | SET_P(_acc) | SET_HC8(A, x) | SET_Z8(_acc) | SET_S8(_acc); \
		A = _acc; \
	} \
	PC += _length

#define _DEC8(x) \
	_acc = (uint32 )(x) - 1; \
	F = CY | MASK_N | SET_VS8(_acc, x, 1) | SET_HCS8(x, 1) | SET_Z8(_acc) | SET_S8(_acc)
#define DEC8(x) \
	{ \
		uint32 _acc; \
		_DEC8(x); \
		x = _acc; \
	} \
	PC += _length
#define DEC8_M(x) \
	{ \
		uint32 _acc; \
		_DEC8(MEM8(x)); \
		STORE8(x, _acc); \
	} \
	PC += _length

#define DEC16(x) \
	(x)--; \
	PC += _length

#define DI() \
	z->r.iff = 0; \
	PC += _length

#define DJNZ(x) \
	if(--B) { \
        PC += (x) + _length; \
		_state += 5; \
	} else \
		PC += _length

#define EI() \
	PC += _length; \
    if(z->r.iff != 3) { \
        imem[0x32] = 0; \
        z->r.iff = 3; \
        z->i.states -= _state; \
        continue; \
    }

#define EX_R(x, y) \
	{ \
		uint16 tmp; \
		tmp = x; x = y; y = tmp; \
	} \
	PC += _length
#define EX_M(x, y) \
	{ \
		uint16 tmp; \
		tmp = MEM16(x); STORE16(x, y); y = tmp; \
	} \
	PC += _length

#define EXX() \
	{ \
		uint16 tmp; \
		tmp = BC; BC = BC_D; BC_D = tmp; \
		tmp = DE; DE = DE_D; DE_D = tmp; \
		tmp = HL; HL = HL_D; HL_D = tmp; \
	} \
	PC += _length

#define HALT() \
    z->r.halt   = 1; \
	z->i.states = 0; \
	PC += _length

#define IM(x) \
	z->r.im = x; \
	PC += _length

#define IN_N(x, y) \
	_state += z80inport(z, &x, y); \
	PC += _length
#define IN_C(x, y) \
	_state += z80inport(z, &x, y); \
	F = CY | SET_P(x) | SET_Z8(x) | SET_S8(x); \
	PC += _length

#define IND() \
	{ \
		uint8 tmp; \
		_state += z80inport(z, &tmp, C); \
		STORE8(HL, tmp); \
		B--; \
		F = CY | (B ? 0: MASK_Z) | PV | S | (MEM8(HL) & 0x80 ? MASK_N: 0) | HC; \
		HL--; \
		PC += _length; \
	}

#define INDR() \
	while(B) { \
		uint8 tmp; \
		_state += z80inport(z, &tmp, C); \
		STORE8(HL, tmp); \
		B--; \
		HL--; \
		_state += 21; \
	} \
	_state -= 5; \
	F = CY | MASK_Z | PV | S | MASK_N | HC; \
	PC += _length

#define INI() \
	{ \
		uint8 tmp; \
		_state += z80inport(z, &tmp, C); \
		STORE8(HL, tmp); \
		B--; \
		HL++; \
		F = CY | (B ? 0: MASK_Z) | PV | S | MASK_N | HC; \
		PC += _length; \
	}

#define INIR() \
	while(B) { \
		uint8 tmp; \
		_state += z80inport(z, &tmp, C); \
		STORE8(HL, tmp); \
		B--; \
		HL++; \
		_state += 21; \
	} \
	_state -= 5; \
	F = CY | MASK_Z | PV | S | MASK_N | HC; \
	PC += _length

#define _INC8(x) \
	_acc = (uint32 )(x) + 1; \
	F = CY | SET_V8(_acc, x, 1) | SET_HC8(x, 1) | SET_Z8(_acc) | SET_S8(_acc)
#define INC8(x) \
	{ \
		uint32 _acc; \
		_INC8(x); \
		x = _acc; \
	} \
	PC += _length
#define INC8_M(x) \
	{ \
		uint32 _acc; \
		_INC8(MEM8(x)); \
		STORE8(x, _acc); \
	} \
	PC += _length

#define INC16(x) \
	(x)++; \
	PC += _length

#define JP(x, y) \
    if(x) { \
        /*int s; */\
        PC = y; \
/*		if(z->i.emulate_subroutine && (s = z80subroutine(z, PC)) >= 0) { \
            PC = MEM16(SP); \
            SP += 2; \
            if(SP > z->i.stack_under) \
                return Z80_UNDERFLOW; \
            _state += s; \
        }*/ \
    } else { \
        PC += _length; \
    }

#define JR(x, y) \
	if(x) { \
		PC += (y) + _length; \
		_state += 5; \
	} else \
		PC += _length

#define LD(x, y) \
	x = y; \
	PC += _length
#define LD_A_I() \
	{ \
		uint32 _acc = I; \
		F = CY | (z->r.iff & 0x02 ? 0: MASK_PV) | SET_Z8(_acc) | SET_S8(_acc); \
		A = _acc; \
	} \
	PC += _length
#define LD_A_R() \
	{ \
		uint32 _acc = R; \
		F = CY | (z->r.iff & 0x02 ? 0: MASK_PV) | SET_Z8(_acc) | SET_S8(_acc); \
		A = _acc; \
	} \
	PC += _length
#define ST8(x, y) \
	STORE8(x, y); \
	PC += _length
#define ST16(x, y) \
	STORE16(x, y); \
	PC += _length

#define LDD() \
	STORE8(DE, MEM8(HL)); \
	DE--; \
	HL--; \
	BC--; \
	F = CY | (BC ? MASK_PV: 0) | Z | S; \
	PC += _length

#define LDDR() \
	do { \
		STORE8(DE, MEM8(HL)); \
		DE--; \
		HL--; \
		BC--; \
		_state += 21; \
	} while(BC); \
	_state -= 5; \
	F = CY | Z | S; \
	PC += _length

#define LDI() \
	STORE8(DE, MEM8(HL)); \
	BC--; \
	F = CY | (BC ? MASK_PV: 0) | Z | S; \
	DE++; \
	HL++; \
	PC += _length

#define LDIR() \
	do { \
		STORE8(DE, MEM8(HL)); \
		DE++; \
		HL++; \
		BC--; \
		_state += 21; \
	} while(BC); \
	_state -= 5; \
	F = CY | Z | S; \
	PC += _length

#define NEG() \
	{ \
		uint32 _acc = -(uint32 )A; \
		F = SET_CYS(_acc) | MASK_N | SET_VS8(_acc, 0, A) | SET_HCS8(0, A) | SET_Z8(_acc) | SET_S8(_acc); \
		A = _acc; \
	} \
	PC += _length

#define NOP() \
	PC += _length

#define OR(x) \
	{ \
		uint32 _acc = (uint32 )A | (x); \
		F = SET_P(_acc) | SET_Z8(_acc) | SET_S8(_acc); \
		A = _acc; \
	} \
	PC += _length

#define OUT(x, y) \
	_state += z80outport(z, x, y); \
	PC += _length

#define OUTD() \
	_state += z80outport(z, C, MEM8(HL)); \
	B--; \
	HL--; \
	F = CY | MASK_N | PV | HC | (B ? 0: MASK_Z) | S; \
	PC += _length

#define OTDR() \
	while(B) { \
		_state += z80outport(z, C, MEM8(HL)); \
		B--; \
		HL--; \
		_state += 21; \
	} \
	_state -= 5; \
	F = CY | MASK_N | PV | HC | MASK_Z | S; \
	PC += _length

#define OUTI() \
	_state += z80outport(z, C, MEM8(HL)); \
	B--; \
	HL++; \
	F = CY | MASK_N | PV | HC | (B ? 0: MASK_Z) | S; \
	PC += _length

#define OTIR() \
	while(B) { \
		_state += z80outport(z, C, MEM8(HL)); \
		B--; \
		HL++; \
		_state += 21; \
	} \
	_state -= 5; \
	F = CY | MASK_N | PV | HC | MASK_Z | S; \
	PC += _length

#define POP(x) \
	x = MEM16(SP); \
	SP += 2; \
	PC += _length

#define PUSH(x) \
	SP -= 2; \
	STORE16(SP, x); \
	PC += _length

#define RES(x, y) \
	y &= ~(1 << (x)); \
	PC += _length
#define RES_M(x, y) \
	STORE8(y, MEM8(y) & ~(1 << (x))); \
	PC += _length
#define RES_M_R(x, y, z) \
	z = MEM8(x) & ~(1 << (x)); \
	STORE8(y, z); \
	PC += _length

#define RET(x) \
	if(x) { \
		PC = MEM16(SP); \
		SP += 2; \
        CallSubLevel--; \
		if(SP > z->i.stack_under) \
			return Z80_UNDERFLOW; \
		_state += 6; \
	} else \
		PC += _length

#define RETI() \
    RET(true)

#define RETN() \
	z->r.iff = (z->r.iff << 1) & 0x03; \
    RET(true)

#define _RL(x) \
	_acc = ((x) << 1) | CY; \
	F = ((x) & 0x80 ? MASK_CY: 0) | SET_P(_acc) | SET_Z8(_acc) | SET_S8(_acc)
#define RL(x) \
	{ \
		uint32 _acc; \
		_RL(x); \
		x = _acc; \
	} \
	PC += _length
#define RL_M(x) \
	{ \
		uint32 _acc; \
		_RL(MEM8(x)); \
		STORE8(x, _acc); \
	} \
	PC += _length
#define RL_M_R(x, y) \
	{ \
		uint32 _acc; \
		_RL(MEM8(x)); \
		y = _acc; \
		STORE8(x, y); \
	} \
	PC += _length

#define RLA() \
	{ \
		uint32 _acc = (A << 1) | CY; \
		F = (A & 0x80 ? MASK_CY: 0) | PV | Z | S; \
		A = _acc; \
	} \
	PC += _length

#define _RLC(x) \
	_acc = ((x) << 1) | ((x) & 0x80 ? 0x01: 0); \
	F = ((x) & 0x80 ? MASK_CY: 0) | SET_P(_acc) | SET_Z8(_acc) | SET_S8(_acc)
#define RLC(x) \
	{ \
		uint32 _acc; \
		_RLC(x); \
		x = _acc; \
	} \
	PC += _length
#define RLC_M(x) \
	{ \
		uint32 _acc; \
		_RLC(MEM8(x)); \
		STORE8(x, _acc); \
	} \
	PC += _length
#define RLC_M_R(x, y) \
	{ \
		uint32 _acc; \
		_RLC(MEM8(x)); \
		y = _acc; \
		STORE8(x, y); \
	} \
	PC += _length

#define RLCA() \
	{ \
		uint32 _acc = (A << 1) | (A & 0x80 ? 0x01: 0); \
		F = (A & 0x80 ? MASK_CY: 0) | PV | Z | S; \
		A = _acc; \
	} \
	PC += _length

#define RLD() \
	{ \
		uint32 _acc = (A & 0xf0) | (MEM8(HL) >> 4); \
		STORE8(HL, (MEM8(HL) << 4) | (A & 0x0f)); \
		F = CY | SET_P(_acc) | SET_Z8(_acc) | SET_S8(_acc); \
		A = _acc; \
	} \
	PC += _length

#define _RR(x) \
	_acc = ((x) >> 1) | (CY ? 0x80: 0); \
	F = ((x) & 0x01) | SET_P(_acc) | SET_Z8(_acc) | SET_S8(_acc)
#define RR(x) \
	{ \
		uint32 _acc; \
		_RR(x); \
		x = _acc; \
	} \
	PC += _length
#define RR_M(x) \
	{ \
		uint32 _acc; \
		_RR(MEM8(x)); \
		STORE8(x, _acc); \
	} \
	PC += _length
#define RR_M_R(x, y) \
	{ \
		uint32 _acc; \
		_RR(MEM8(y)); \
		y = _acc; \
		STORE8(x, y); \
	} \
	PC += _length

#define RRA() \
	{ \
		uint32 _acc = (A >> 1) | (CY ? 0x80: 0); \
		F = (A & 0x01) | PV | Z | S; \
		A = _acc; \
	} \
	PC += _length

#define _RRC(x) \
	_acc = ((x) >> 1) | ((x) & 0x01 ? 0x80: 0); \
	F = ((x) & 0x01) | SET_P(_acc) | SET_Z8(_acc) | SET_S8(_acc)
#define RRC(x) \
	{ \
		uint32 _acc; \
		_RRC(x); \
		x = _acc; \
	} \
	PC += _length
#define RRC_M(x) \
	{ \
		uint32 _acc; \
		_RRC(MEM8(x)); \
		STORE8(x, _acc); \
	} \
	PC += _length
#define RRC_M_R(x, y) \
	{ \
		uint32 _acc; \
		_RRC(MEM8(x)); \
		y = _acc; \
		STORE8(x, y); \
	} \
	PC += _length

#define RRCA() \
	{ \
		uint32 _acc = (A >> 1) | (A & 0x01 ? 0x80: 0); \
		F = (A & 0x01) | PV | Z | S; \
		A = _acc; \
	} \
	PC += _length

#define RRD() \
	{ \
		uint32 _acc; \
		_acc = (A & 0xf0) | (MEM8(HL) & 0x0f); \
		STORE8(HL, (MEM8(HL) >> 4) | (A << 4)); \
		F = CY | SET_P(_acc) | SET_Z8(_acc) | SET_S8(_acc); \
		A = _acc; \
	} \
	PC += _length

#define RST(x) \
    CALL(true, x)

#define SBC8(x) \
	{ \
		uint32 _acc = A - (x) - CY; \
		F = SET_CYS(_acc) | MASK_N | SET_VS8(_acc, A, x) | SET_HCS8_CY(A, x, CY) | SET_Z8(_acc) | SET_S8(_acc); \
		A = _acc; \
	} \
	PC += _length

#define SBC16(x, y) \
	{ \
		uint32 _acc = (x) - (y) - CY; \
		F = SET_CYS(_acc) | MASK_N | SET_VS16(_acc, x, y) | SET_HCS16_CY(x, y, CY) | SET_Z16(_acc) | SET_S16(_acc); \
		(x) = _acc; \
	} \
	PC += _length

#define SCF() \
	F |= MASK_CY; \
	PC += _length

#define SET(x, y) \
	y |= (1 << (x)); \
	PC += _length
#define SET_M(x, y) \
	STORE8(y, MEM8(y) | (1 << (x))); \
	PC += _length
#define SET_M_R(x, y, z) \
	z = MEM8(y) | (1 << (x)); \
	STORE8(y, z); \
	PC += _length

#define _SLA(x) \
	_acc = (x) << 1; \
	F = ((x) & 0x80 ? MASK_CY: 0) | SET_P(_acc) | SET_S8(_acc) | SET_Z8(_acc)
#define SLA(x) \
	{ \
		uint32 _acc; \
		_SLA(x); \
		x = _acc; \
	} \
	PC += _length
#define SLA_M(x) \
	{ \
		uint32 _acc; \
		_SLA(MEM8(x)); \
		STORE8(x, _acc); \
	} \
	PC += _length
#define SLA_M_R(x, y) \
	{ \
		uint32 _acc; \
		_SLA(MEM8(x)); \
		y = _acc; \
		STORE8(x, y); \
	} \
	PC += _length

#define _SLL(x) \
	_acc = (x) << 1 | 1; \
	F = ((x) & 0x80 ? MASK_CY: 0) | SET_P(_acc) | SET_S8(_acc) | SET_Z8(_acc)
#define SLL(x) \
	{ \
		uint32 _acc; \
		_SLL(x); \
		x = _acc; \
	} \
	PC += _length
#define SLL_M(x) \
	{ \
		uint32 _acc; \
		_SLL(MEM8(x)); \
		STORE8(x, _acc); \
	} \
	PC += _length
#define SLL_M_R(x, y) \
	{ \
		uint32 _acc; \
		_SLL(MEM8(x)); \
		y = _acc; \
		STORE8(x, y); \
	} \
	PC += _length

#define _SRA(x) \
	_acc = ((x) >> 1) | ((x) & 0x80); \
	F = ((x) & 0x01 ? MASK_CY: 0) | SET_P(_acc) | SET_S8(_acc) | SET_Z8(_acc)
#define SRA(x) \
	{ \
		uint32 _acc; \
		_SRA(x); \
		x = _acc; \
	} \
	PC += _length
#define SRA_M(x) \
	{ \
		uint32 _acc; \
		_SRA(MEM8(x)); \
		STORE8(x, _acc); \
	} \
	PC += _length
#define SRA_M_R(x, y) \
	{ \
		uint32 _acc; \
		_SRA(MEM8(x)); \
		y = _acc; \
		STORE8(x, y); \
	} \
	PC += _length

#define _SRL(x) \
	_acc = (x) >> 1; \
	F = ((x) & 0x01 ? MASK_CY: 0) | SET_P(_acc) | SET_S8(_acc) | SET_Z8(_acc)
#define SRL(x) \
	{ \
		uint32 _acc; \
		_SRL(x); \
		x = _acc; \
	} \
	PC += _length
#define SRL_M(x) \
	{ \
		uint32 _acc; \
		_SRL(MEM8(x)); \
		STORE8(x, _acc); \
	} \
	PC += _length
#define SRL_M_R(x, y) \
	{ \
		uint32 _acc; \
		_SRL(MEM8(x)); \
		y = _acc; \
		STORE8(x, y); \
	} \
	PC += _length

#define SUB8(x) \
	{ \
		uint32 _acc = (uint32 )A - (x); \
		F = SET_CYS(_acc) | MASK_N | SET_VS8(_acc, A, x) | SET_HCS8(A, x) | SET_Z8(_acc) | SET_S8(_acc); \
		A = _acc; \
	} \
	PC += _length

#define XOR(x) \
	{ \
		uint32 _acc = (uint32 )A ^ (x); \
		F = SET_P(_acc) | SET_Z8(_acc) | SET_S8(_acc); \
		A = _acc; \
	} \
	PC += _length


uint32 CZ80::rnd = 0xffffffff;

const int CZ80::state_xx[] = {
  4, 10,  7,  6,  4,  4,  7,  4,  4, 11,  7,  6,  4,  4,  7,  4,
  8, 10,  7,  6,  4,  4,  7,  4,  7, 11,  7,  6,  4,  4,  7,  4,
  7, 10, 16,  6,  4,  4,  7,  4,  7, 11, 16,  6,  4,  4,  7,  4,
  7, 10, 13,  6, 11, 11, 10,  4,  7, 11, 13,  6,  4,  4,  7,  4,
  4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
  4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
  4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
  7,  7,  7,  7,  7,  7,  4,  7,  4,  4,  4,  4,  4,  4,  7,  4,
  4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
  4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
  4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
  4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
  5, 10, 10, 10, 10, 11,  7, 11,  5,  4, 10,  0, 10, 10,  7, 11,
  5, 10, 10, 11, 10, 11,  7, 11,  5,  4, 10, 11, 10,  0,  7, 11,
  5, 10, 10, 19, 10, 11,  7, 11,  5,  4, 10,  4, 10,  0,  7, 11,
  5, 10, 10,  4, 10, 11,  7, 11,  5,  6, 10,  4, 10,  0,  7, 11
};

/* �X�e�[�g��(CB xx) */
const int CZ80::state_cb_xx[] = {
  8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
  8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
  8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
  8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
  8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
  8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
  8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
  8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
  8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
  8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
  8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
  8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
  8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
  8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
  8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
  8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8
};

/* �X�e�[�g��(DD/FD xx) */
const int CZ80::state_dd_xx[] = {
   8,  8,  8,  8,  8,  8,  8,  8,  8, 10,  8,  8, 8, 8,  8, 8,
   8,  8,  8,  8,  8,  8,  8,  8,  8, 10,  8,  8, 8, 8,  8, 8,
   8, 14, 20, 10,  8,  8, 12,  8,  8, 10, 20, 10, 8, 8, 12, 8,
   8,  8,  8,  8, 19, 19, 19,  8,  8, 10,  8,  8, 8, 8,  8, 8,
   8,  8,  8,  8,  8,  8, 19,  8,  8,  8,  8,  8, 8, 8, 19, 8,
   8,  8,  8,  8,  8,  8, 19,  8,  8,  8,  8,  8, 8, 8, 19, 8,
   8,  8,  8,  8,  8,  8, 19,  8,  8,  8,  8,  8, 8, 8, 19, 8,
  19, 19, 19, 19, 19, 19,  8, 19,  8,  8,  8,  8, 8, 8, 19, 8,
   8,  8,  8,  8,  8,  8, 19,  8,  8,  8,  8,  8, 8, 8, 19, 8,
   8,  8,  8,  8,  8,  8, 19,  8,  8,  8,  8,  8, 8, 8, 19, 8,
   8,  8,  8,  8,  8,  8, 19,  8,  8,  8,  8,  8, 8, 8, 19, 8,
   8,  8,  8,  8,  8,  8, 19,  8,  8,  8,  8,  8, 8, 8, 19, 8,
   8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  0, 8, 8,  8, 8,
   8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8, 8, 8,  8, 8,
   8, 14,  8, 23,  8, 15,  8,  8,  8,  8,  8,  8, 8, 8,  8, 8,
   8,  8,  8,  8,  8,  8,  8,  8,  8, 10,  8,  8, 8, 8,  8, 8
};

/* �X�e�[�g��(DD/FD CB xx) */
const int CZ80::state_dd_cb_xx[] = {
  12, 12, 12, 12, 12, 12, 23, 12, 12, 12, 12, 12, 12, 12, 23, 12,
  12, 12, 12, 12, 12, 12, 23, 12, 12, 12, 12, 12, 12, 12, 23, 12,
  12, 12, 12, 12, 12, 12, 23, 12, 12, 12, 12, 12, 12, 12, 23, 12,
  12, 12, 12, 12, 12, 12, 23, 12, 12, 12, 12, 12, 12, 12, 23, 12,
  12, 12, 12, 12, 12, 12, 20, 12, 12, 12, 12, 12, 12, 12, 20, 12,
  12, 12, 12, 12, 12, 12, 20, 12, 12, 12, 12, 12, 12, 12, 20, 12,
  12, 12, 12, 12, 12, 12, 20, 12, 12, 12, 12, 12, 12, 12, 20, 12,
  12, 12, 12, 12, 12, 12, 20, 12, 12, 12, 12, 12, 12, 12, 20, 12,
  12, 12, 12, 12, 12, 12, 23, 12, 12, 12, 12, 12, 12, 12, 23, 12,
  12, 12, 12, 12, 12, 12, 23, 12, 12, 12, 12, 12, 12, 12, 23, 12,
  12, 12, 12, 12, 12, 12, 23, 12, 12, 12, 12, 12, 12, 12, 23, 12,
  12, 12, 12, 12, 12, 12, 23, 12, 12, 12, 12, 12, 12, 12, 23, 12,
  12, 12, 12, 12, 12, 12, 23, 12, 12, 12, 12, 12, 12, 12, 23, 12,
  12, 12, 12, 12, 12, 12, 23, 12, 12, 12, 12, 12, 12, 12, 23, 12,
  12, 12, 12, 12, 12, 12, 23, 12, 12, 12, 12, 12, 12, 12, 23, 12,
  12, 12, 12, 12, 12, 12, 23, 12, 12, 12, 12, 12, 12, 12, 23, 12
};

/* �X�e�[�g��(ED xx) */
const int CZ80::state_ed_xx[] = {
   8,  8,  8,  8, 8,  8, 8,  8,  8,  8,  8,  8, 8,  8, 8,  8,
   8,  8,  8,  8, 8,  8, 8,  8,  8,  8,  8,  8, 8,  8, 8,  8,
   8,  8,  8,  8, 8,  8, 8,  8,  8,  8,  8,  8, 8,  8, 8,  8,
   8,  8,  8,  8, 8,  8, 8,  8,  8,  8,  8,  8, 8,  8, 8,  8,
  12, 12, 15, 20, 8, 14, 8,  9, 12, 12, 15, 20, 8, 14, 8,  9,
  12, 12, 15, 20, 8,  8, 8,  9, 12, 12, 15, 20, 8,  8, 8,  9,
  12, 12, 15 ,16, 8,  8, 8, 18, 12, 12, 15, 20, 8,  8, 8, 18,
   8,  8, 15, 20, 8,  8, 8,  8, 11, 12, 15, 20, 8,  8, 8,  8,
   8,  8,  8,  8, 8,  8, 8,  8,  8,  8,  8,  8, 8,  8, 8,  8,
   8,  8,  8,  8, 8,  8, 8,  8,  8,  8,  8,  8, 8,  8, 8,  8,
  16, 16, 16, 16, 8,  8, 8,  8, 16, 16, 16, 16, 8,  8, 8,  8,
   0,  0,  0,  0, 8,  8, 8,  8,  0,  0,  0,  0, 8,  8, 8,  8,
   8,  8,  8,  8, 8,  8, 8,  8,  8,  8,  8,  8, 8,  8, 8,  8,
   8,  8,  8,  8, 8,  8, 8,  8,  8,  8,  8,  8, 8,  8, 8,  8,
   8,  8,  8,  8, 8,  8, 8,  8,  8,  8,  8,  8, 8,  8, 8,  8,
   8,  8,  8,  8, 8,  8, 8,  8,  8,  8,  8,  8, 8,  8, 8,  8
};

/* ���ߒ�(xx) */
const uint16 CZ80::len_xx[] = {
  1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
  2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
  2, 3, 3, 1, 1, 1, 2, 1, 2, 1, 3, 1, 1, 1, 2, 1,
  2, 3, 3, 1, 1, 1, 2, 1, 2, 1, 3, 1, 1, 1, 2, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 0, 3, 3, 2, 1,
  1, 1, 3, 2, 3, 1, 2, 1, 1, 1, 3, 2, 3, 0, 2, 1,
  1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 0, 2, 1,
  1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 0, 2, 1
};

/* ���ߒ�(CB xx) */
const uint16 CZ80::len_cb_xx[] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

/* ���ߒ�(DD/FD xx) */
const uint16 CZ80::len_dd_xx[] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 3, 3, 1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1,
  1, 1, 1, 1, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
  1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
  1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
  2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1,
  1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
  1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
  1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
  1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

/* ���ߒ�(DD/FD CB xx) */
const uint16 CZ80::len_dd_cb_xx[] = {
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};

/* ���ߒ�(ED xx) */
const uint16 CZ80::len_ed_xx[] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1,
  1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1,
  1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1,
  1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

/* �p���e�B */
const uint8 CZ80::parity[] = {
  MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV,
  0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0,
  0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0,
  MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV,
  0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0,
  MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV,
  MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV,
  0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0,
  0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0,
  MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV,
  MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV,
  0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0,
  MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV,
  0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0,
  0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0,
  MASK_PV, 0, 0, MASK_PV, 0, MASK_PV, MASK_PV, 0, 0, MASK_PV, MASK_PV, 0, MASK_PV, 0, 0, MASK_PV
};

CZ80::CZ80(CPObject *parent)	: CCPU(parent)
{				//[constructor]

    pDEBUG = new Cdebug_z80(this);
    fn_log="z80.log";

    Is_Timer_Reached=false;
    //step_Previous_State = 0;

    regwidget = (CregCPU*) new Cregsz80Widget(0,this);
}

CZ80::~CZ80() {

}

int CZ80::z80retn(Z80stat *z) {
    uint16 _length = 0;
    uint16 _state = 0;
    RETN();
    return 0;
}

/*
    DAA��s����A���W�X�^�ɉ�Z�������l��CY�t���O
*/
inline void CZ80::daa_result(uint8 *x, uint8 *c, uint8 a, uint8 f)
{
	switch(f & (MASK_CY | MASK_N | MASK_HC)) {
	case 0:
		if(a < 0x9a) {
			if((a & 0x0f) < 0x0a)
				*x = 0x00, *c = 0;
			else
				*x = 0x06, *c = 0;
		} else {
			if((a & 0x0f) < 0x0a)
				*x = 0x60, *c = 1;
			else
				*x = 0x66, *c = 1;
		}
		break;
	case MASK_CY:
		if((a & 0x0f) < 0x0a)
			*x = 0x60, *c = 1;
		else
			*x = 0x66, *c = 1;
		break;
	case MASK_N:
		if(a < 0x9a) {
			if((a & 0x0f) < 0x0a)
				*x = 0x00, *c = 0;
			else
				*x = 0xfa, *c = 0;
		} else {
			if((a & 0x0f) < 0x0a)
				*x = 0xa0, *c = 1;
			else
				*x = 0x9a, *c = 1;
		}
		break;
	case MASK_CY | MASK_N:
		if((a & 0x0f) < 0x0a)
			*x = 0xa0, *c = 1;
		else
			*x = 0x9a, *c = 1;
		break;
	case MASK_HC:
		if(a < 0x9a)
			*x = 0x06, *c = 0;
		else
			*x = 0x66, *c = 1;
		break;
	case MASK_CY | MASK_HC:
		*x = 0x66, *c = 1;
		break;
	case MASK_N | MASK_HC:
		if(a < 0x9a)
			*x = 0xfa, *c = 0;
		else
			*x = 0x9a, *c = 1;
		break;
	case MASK_CY | MASK_N | MASK_HC:
		*x = 0x9a, *c = 1;
		break;
	default:
		*x = 0, *c = 0;
		break;
	}
}

/*
    R���W�X�^�p����seed���ݒ肷��
*/
void CZ80::z80srand(uint32 seed)
{
	rnd = seed;
}

/*
    RESET�M���𑗂�
*/
int CZ80::z80reset(Z80stat *z)
{
    z->r.im = 0;
	z->r.iff = 0;
	z->r.halt = 0;
    z->r16.af = z->r16.bc = z->r16.de = z->r16.hl = z->r16.ix = z->r16.iy = z->r16.af_d = z->r16.bc_d = z->r16.de_d = z->r16.hl_d = z->r16.sp = 0x0000;
	z->r16.pc = 0;
	z->i.states = 0;
	z->i.total_states = 0;
    return true;
}

/*
    NMI�M���𑗂�
*/
int CZ80::z80nmi(Z80stat *z)
{
	int _state = 0;
	uint16 _length = 0;

	z->r.halt = 0;
	z->r.iff &= ~0x01;
	z->i.states -= 13;
    CALL(true, 0x0066);
    return true;
}

/*
    INT�M���𑗂���Ƃ����荞�݂��������邩�`�F�b�N���� (IM 0)
*/
int CZ80::z80int0chk(const Z80stat *z)
{
	return z->r.im == 0 && z->r.iff == 3;
}

/*
	INT�M���𑗂� (IM 0)
*/
int CZ80::z80int0(Z80stat *z, uint8 op)
{
	int _state = 0;
	uint16 _length = 0;

	if(!z80int0chk(z))
        return false;
	
	z->r.halt = z->r.iff = 0;
	z->i.states -= 13;
	switch(op) {
	case 0xc7:
		RST(0x00); break;
	case 0xcf:
		RST(0x08); break;
	case 0xd7:
		RST(0x10); break;
	case 0xdf:
		RST(0x18); break;
	case 0xe7:
		RST(0x20); break;
	case 0xef:
		RST(0x28); break;
	case 0xf7:
		RST(0x30); break;
	case 0xff:
		RST(0x38); break;
	}
    return true;
}

/*
	INT�M���𑗂���Ƃ����荞�݂��������邩�`�F�b�N���� (IM 1)
*/
int CZ80::z80int1chk(const Z80stat *z)
{
	return z->r.im == 1 && z->r.iff == 3;
}

/*
	INT�M���𑗂� (IM 1)
*/
int CZ80::z80int1(Z80stat *z)
{
	int _state = 0;
	uint16 _length = 0;

	if(!z80int1chk(z))
        return false;

	z->r.halt = z->r.iff = 0;
	z->i.states -= 13;
	RST(0x38);
    return true;
}

/*
	INT�M���𑗂���Ƃ����荞�݂��������邩�`�F�b�N���� (IM 2)
*/
int CZ80::z80int2chk(const Z80stat *z)
{
	return z->r.im == 2 && z->r.iff == 3;
}

/*
	INT�M���𑗂� (IM 2)
*/
int CZ80::z80int2(Z80stat *z, uint8 vector)
{
	int _state = 0;
	uint16 _length = 0;

	if(!z80int2chk(z))
        return false;

    z80exec(z);

    if (fp_log) fprintf(fp_log,"Interrupt : %05X \t IFF=%i\n",MEM16((z->r.i << 8U) | (vector & 0x7f)),z->r.iff);
	z->r.halt = z->r.iff = 0;
	z->i.states -= 19;
//    imem[0x32] = 0;

    CALL(true, MEM16((z->r.i << 8U) | (vector & 0x7f)));

    pPC->pTIMER->state += _state;
    return true;
}

int CZ80::z80nsc800intr(Z80stat *z, uint8 vector)
{
    int _state = 0;
    uint16 _length = 0;



    if (fp_log) fprintf(fp_log,"Interrupt nsc800: %05X \t IFF=%i\n",MEM16((z->r.i << 8U) | (vector & 0x7f)),z->r.iff);
    z->r.halt = z->r.iff = 0;
    z->i.states -= 19;
    RST(vector);

    pPC->pTIMER->state += _state;
    return true;
}
/*
	���߂���s����
*/


int CZ80::z80exec(Z80stat *z)
{
	int _state;
	uint16 _length;
	uint8 op;

	z->i.total_states += z->i.states;
    //pPC->pTIMER->state -= z->i.states;
    //if (fp_log) fprintf(fp_log,"(States = %i,%i, %i)\n",z->i.total_states,z->i.states,pPC->pTIMER->state);

	if(z->i.total_states < 0)
		z->i.total_states -= INT_MAX;

	if(z->r.halt) {
        pPC->pTIMER->state += 5;
		z->i.states = 0;
		return Z80_HALT;
	}

	do {
#if defined(Z80_TRACE)
        if(z->i.trace)
            z80log(z);
#endif
		switch(FETCH_XX()) {
		case 0x00: NOP();           break;	/* nop */
		case 0x01: LD(BC, IMM16);   break;	/* ld BC, mn */
		case 0x02: ST8(BC, A);      break;	/* ld (BC), A */
		case 0x03: INC16(BC);       break;	/* inc BC */
		case 0x04: INC8(B);         break;	/* inc B */
		case 0x05: DEC8(B);         break;	/* dec B */
		case 0x06: LD(B, IMM8);     break;	/* ld B, n */
		case 0x07: RLCA();          break;	/* rlca */

		case 0x08: EX_R(AF, AF_D);  break;	/* ex AF, AF' */
		case 0x09: ADD16(HL, BC);   break;	/* add HL, BC */
		case 0x0a: LD(A, MEM8(BC)); break;	/* ld A, (BC) */
		case 0x0b: DEC16(BC);       break;	/* dec BC */
		case 0x0c: INC8(C);         break;	/* inc C */
		case 0x0d: DEC8(C);         break;	/* dec C */
		case 0x0e: LD(C, IMM8);     break;	/* ld C, n */
		case 0x0f: RRCA();          break;	/* rrca */

		case 0x10: DJNZ(EQ);        break;	/* djnz e */
		case 0x11: LD(DE, IMM16);   break;	/* ld DE, mn */
		case 0x12: ST8(DE, A);      break;	/* ld (DE), A */
		case 0x13: INC16(DE);       break;	/* inc DE */
		case 0x14: INC8(D);         break;	/* inc D */
		case 0x15: DEC8(D);         break;	/* dec D */
		case 0x16: LD(D, IMM8);     break;	/* ld D, n */
		case 0x17: RLA();           break;	/* rla */

        case 0x18: JR(true, EQ);    break;	/* jr e */
		case 0x19: ADD16(HL, DE);   break;	/* add HL, DE */
		case 0x1a: LD(A, MEM8(DE)); break;	/* ld A, (DE) */
		case 0x1b: DEC16(DE);       break;	/* dec DE */
		case 0x1c: INC8(E);         break;	/* inc E */
		case 0x1d: DEC8(E);         break;	/* dec E */
		case 0x1e: LD(E, IMM8);     break;	/* ld E, n */
		case 0x1f: RRA();           break;	/* rra */

		case 0x20: JR(CC_NZ, EQ);        break;	/* jr NZ, e */
		case 0x21: LD(HL, IMM16);        break;	/* ld HL, mn */
		case 0x22: ST16(IMM16, HL);      break;	/* ld (mn), HL */
		case 0x23: INC16(HL);            break;	/* inc HL */
		case 0x24: INC8(H);              break;	/* inc H */
		case 0x25: DEC8(H);              break;	/* dec H */
		case 0x26: LD(H, IMM8);          break;	/* ld H, n */
		case 0x27: DAA();                break;	/* daa */

		case 0x28: JR(CC_Z, EQ);         break;	/* jr Z, e */
		case 0x29: ADD16(HL, HL);        break;	/* add HL, HL */
		case 0x2a: LD(HL, MEM16(IMM16)); break;	/* ld HL, (mn) */
		case 0x2b: DEC16(HL);            break;	/* dec HL */
		case 0x2c: INC8(L);              break;	/* inc L */
		case 0x2d: DEC8(L);              break;	/* dec L */
		case 0x2e: LD(L, IMM8);          break;	/* ld L, n */
		case 0x2f: CPL();                break;	/* cpl */

		case 0x30: JR(CC_NC, EQ);  break;	/* jr NC, e */
		case 0x31: LD(SP, IMM16);  break;	/* ld SP, mn */
		case 0x32: ST8(IMM16, A);  break;	/* ld (mn), A */
		case 0x33: INC16(SP);      break;	/* inc SP */
		case 0x34: INC8_M(HL);     break;	/* inc (HL) */
		case 0x35: DEC8_M(HL);     break;	/* dec (HL) */
		case 0x36: ST8(HL, IMM8);  break;	/* ld (HL), n */
		case 0x37: SCF();          break;	/* scf */

		case 0x38: JR(CC_C, EQ);       break;	/* jr C, e */
		case 0x39: ADD16(HL, SP);      break;	/* add HL, SP */
		case 0x3a: LD(A, MEM8(IMM16)); break;	/* ld A, (mn) */
		case 0x3b: DEC16(SP);          break;	/* dec SP */
		case 0x3c: INC8(A);            break;	/* inc A */
		case 0x3d: DEC8(A);            break;	/* dec A */
		case 0x3e: LD(A, IMM8);        break;	/* ld A, n */
		case 0x3f: CCF();              break;	/* ccf */

		case 0x40: LD(B, B);        break;	/* ld B, B */
		case 0x41: LD(B, C);        break;	/* ld B, C */
		case 0x42: LD(B, D);        break;	/* ld B, D */
		case 0x43: LD(B, E);        break;	/* ld B, E */
		case 0x44: LD(B, H);        break;	/* ld B, H */
		case 0x45: LD(B, L);        break;	/* ld B, L */
		case 0x46: LD(B, MEM8(HL)); break;	/* ld B, (HL) */
		case 0x47: LD(B, A);        break;	/* ld B, A */

		case 0x48: LD(C, B);        break;	/* ld C, B */
		case 0x49: LD(C, C);        break;	/* ld C, C */
		case 0x4a: LD(C, D);        break;	/* ld C, D */
		case 0x4b: LD(C, E);        break;	/* ld C, E */
		case 0x4c: LD(C, H);        break;	/* ld C, H */
		case 0x4d: LD(C, L);        break;	/* ld C, L */
		case 0x4e: LD(C, MEM8(HL)); break;	/* ld C, (HL) */
		case 0x4f: LD(C, A);        break;	/* ld C, A */

		case 0x50: LD(D, B);        break;	/* ld D, B */
		case 0x51: LD(D, C);        break;	/* ld D, C */
		case 0x52: LD(D, D);        break;	/* ld D, D */
		case 0x53: LD(D, E);        break;	/* ld D, E */
		case 0x54: LD(D, H);        break;	/* ld D, H */
		case 0x55: LD(D, L);        break;	/* ld D, L */
		case 0x56: LD(D, MEM8(HL)); break;	/* ld D, (HL) */
		case 0x57: LD(D, A);        break;	/* ld D, A */

		case 0x58: LD(E, B);        break;	/* ld E, B */
		case 0x59: LD(E, C);        break;	/* ld E, C */
		case 0x5a: LD(E, D);        break;	/* ld E, D */
		case 0x5b: LD(E, E);        break;	/* ld E, E */
		case 0x5c: LD(E, H);        break;	/* ld E, H */
		case 0x5d: LD(E, L);        break;	/* ld E, L */
		case 0x5e: LD(E, MEM8(HL)); break;	/* ld E, (HL) */
		case 0x5f: LD(E, A);        break;	/* ld E, A */

		case 0x60: LD(H, B);        break;	/* ld H, B */
		case 0x61: LD(H, C);        break;	/* ld H, C */
		case 0x62: LD(H, D);        break;	/* ld H, D */
		case 0x63: LD(H, E);        break;	/* ld H, E */
		case 0x64: LD(H, H);        break;	/* ld H, H */
		case 0x65: LD(H, L);        break;	/* ld H, L */
		case 0x66: LD(H, MEM8(HL)); break;	/* ld H, (HL) */
		case 0x67: LD(H, A);        break;	/* ld H, A */

		case 0x68: LD(L, B);        break;	/* ld L, B */
		case 0x69: LD(L, C);        break;	/* ld L, C */
		case 0x6a: LD(L, D);        break;	/* ld L, D */
		case 0x6b: LD(L, E);        break;	/* ld L, E */
		case 0x6c: LD(L, H);        break;	/* ld L, H */
		case 0x6d: LD(L, L);        break;	/* ld L, L */
		case 0x6e: LD(L, MEM8(HL)); break;	/* ld L, (HL) */
		case 0x6f: LD(L, A);        break;	/* ld L, A */

		case 0x70: ST8(HL, B); break;	/* ld (HL), B */
		case 0x71: ST8(HL, C); break;	/* ld (HL), C */
		case 0x72: ST8(HL, D); break;	/* ld (HL), D */
		case 0x73: ST8(HL, E); break;	/* ld (HL), E */
		case 0x74: ST8(HL, H); break;	/* ld (HL), H */
		case 0x75: ST8(HL, L); break;	/* ld (HL), L */
        case 0x76: z->i.total_states -= z->i.states; HALT();
            pPC->pTIMER->state += _state;
            if (fp_log) fprintf(fp_log,"(States = %i,%i, %i)\n",z->i.states,pPC->pTIMER->state,_state);

            return Z80_HALT;	/* halt */
        //case 0x76: NOP(); break;
		case 0x77: ST8(HL, A); break;	/* ld (HL), A */

		case 0x78: LD(A, B);        break;	/* ld A, B */
		case 0x79: LD(A, C);        break;	/* ld A, C */
		case 0x7a: LD(A, D);        break;	/* ld A, D */
		case 0x7b: LD(A, E);        break;	/* ld A, E */
		case 0x7c: LD(A, H);        break;	/* ld A, H */
		case 0x7d: LD(A, L);        break;	/* ld A, L */
		case 0x7e: LD(A, MEM8(HL)); break;	/* ld A, (HL) */
		case 0x7f: LD(A, A);        break;	/* ld A, A */

		case 0x80: ADD8(B);        break;	/* add B */
		case 0x81: ADD8(C);        break;	/* add C */
		case 0x82: ADD8(D);        break;	/* add D */
		case 0x83: ADD8(E);        break;	/* add E */
		case 0x84: ADD8(H);        break;	/* add H */
		case 0x85: ADD8(L);        break;	/* add L */
		case 0x86: ADD8(MEM8(HL)); break;	/* add (HL) */
		case 0x87: ADD8(A);        break;	/* add A */

		case 0x88: ADC8(B);        break;	/* adc B */
		case 0x89: ADC8(C);        break;	/* adc C */
		case 0x8a: ADC8(D);        break;	/* adc D */
		case 0x8b: ADC8(E);        break;	/* adc E */
		case 0x8c: ADC8(H);        break;	/* adc H */
		case 0x8d: ADC8(L);        break;	/* adc L */
		case 0x8e: ADC8(MEM8(HL)); break;	/* adc (HL) */
		case 0x8f: ADC8(A);        break;	/* adc A */

		case 0x90: SUB8(B);        break;	/* sub B */
		case 0x91: SUB8(C);        break;	/* sub C */
		case 0x92: SUB8(D);        break;	/* sub D */
		case 0x93: SUB8(E);        break;	/* sub E */
		case 0x94: SUB8(H);        break;	/* sub H */
		case 0x95: SUB8(L);        break;	/* sub L */
		case 0x96: SUB8(MEM8(HL)); break;	/* sub (HL) */
		case 0x97: SUB8(A);        break;	/* sub A */

		case 0x98: SBC8(B);        break;	/* sbc B */
		case 0x99: SBC8(C);        break;	/* sbc C */
		case 0x9a: SBC8(D);        break;	/* sbc D */
		case 0x9b: SBC8(E);        break;	/* sbc E */
		case 0x9c: SBC8(H);        break;	/* sbc H */
		case 0x9d: SBC8(L);        break;	/* sbc L */
		case 0x9e: SBC8(MEM8(HL)); break;	/* sbc (HL) */
		case 0x9f: SBC8(A);        break;	/* sbc A */

		case 0xa0: AND(B);        break;	/* and B */
		case 0xa1: AND(C);        break;	/* and C */
		case 0xa2: AND(D);        break;	/* and D */
		case 0xa3: AND(E);        break;	/* and E */
		case 0xa4: AND(H);        break;	/* and H */
		case 0xa5: AND(L);        break;	/* and L */
		case 0xa6: AND(MEM8(HL)); break;	/* and (HL) */
		case 0xa7: AND(A);        break;	/* and A */

		case 0xa8: XOR(B);        break;	/* xor B */
		case 0xa9: XOR(C);        break;	/* xor C */
		case 0xaa: XOR(D);        break;	/* xor D */
		case 0xab: XOR(E);        break;	/* xor E */
		case 0xac: XOR(H);        break;	/* xor H */
		case 0xad: XOR(L);        break;	/* xor L */
		case 0xae: XOR(MEM8(HL)); break;	/* xor (HL) */
		case 0xaf: XOR(A);        break;	/* xor A */

		case 0xb0: OR(B);        break;	/* or B */
		case 0xb1: OR(C);        break;	/* or C */
		case 0xb2: OR(D);        break;	/* or D */
		case 0xb3: OR(E);        break;	/* or E */
		case 0xb4: OR(H);        break;	/* or H */
		case 0xb5: OR(L);        break;	/* or L */
		case 0xb6: OR(MEM8(HL)); break;	/* or (HL) */
		case 0xb7: OR(A);        break;	/* or A */

		case 0xb8: CP(B);        break;	/* cp B */
		case 0xb9: CP(C);        break;	/* cp C */
		case 0xba: CP(D);        break;	/* cp D */
		case 0xbb: CP(E);        break;	/* cp E */
		case 0xbc: CP(H);        break;	/* cp H */
		case 0xbd: CP(L);        break;	/* cp L */
		case 0xbe: CP(MEM8(HL)); break;	/* cp (HL) */
		case 0xbf: CP(A);        break;	/* cp A */

		case 0xc0: RET(CC_NZ);         break;	/* ret NZ */
		case 0xc1: POP(BC);            break;	/* pop BC */
		case 0xc2: JP(CC_NZ, IMM16);   break;	/* jp NZ, mn */
        case 0xc3: JP(true, IMM16);    break;	/* jp mn */
		case 0xc4: CALL(CC_NZ, IMM16); break;	/* call NZ, mn */
		case 0xc5: PUSH(BC);           break;	/* push BC */
		case 0xc6: ADD8(IMM8);         break;	/* add n */
		case 0xc7: RST(0x00);          break;	/* rst 00H */

		case 0xc8: RET(CC_Z);         break;	/* ret Z */
        case 0xc9: RET(true);         break;	/* ret */
		case 0xca: JP(CC_Z, IMM16);   break;	/* jp Z, mn */
		case 0xcb:
			PC++;
			switch(FETCH_CB_XX()) {
			case 0x00: RLC(B);    break;	/* rlc B */
			case 0x01: RLC(C);    break;	/* rlc C */
			case 0x02: RLC(D);    break;	/* rlc D */
			case 0x03: RLC(E);    break;	/* rlc E */
			case 0x04: RLC(H);    break;	/* rlc H */
			case 0x05: RLC(L);    break;	/* rlc L */
			case 0x06: RLC_M(HL); break;	/* rlc (HL) */
			case 0x07: RLC(A);    break;	/* rlc A */

			case 0x08: RRC(B);    break;	/* rrc B */
			case 0x09: RRC(C);    break;	/* rrc C */
			case 0x0a: RRC(D);    break;	/* rrc D */
			case 0x0b: RRC(E);    break;	/* rrc E */
			case 0x0c: RRC(H);    break;	/* rrc H */
			case 0x0d: RRC(L);    break;	/* rrc L */
			case 0x0e: RRC_M(HL); break;	/* rrc (HL) */
			case 0x0f: RRC(A);    break;	/* rrc A */

			case 0x10: RL(B);    break;	/* rl B */
			case 0x11: RL(C);    break;	/* rl C */
			case 0x12: RL(D);    break;	/* rl D */
			case 0x13: RL(E);    break;	/* rl E */
			case 0x14: RL(H);    break;	/* rl H */
			case 0x15: RL(L);    break;	/* rl L */
			case 0x16: RL_M(HL); break;	/* rl (HL) */
			case 0x17: RL(A);    break;	/* rl A */

			case 0x18: RR(B);    break;	/* rr B */
			case 0x19: RR(C);    break;	/* rr C */
			case 0x1a: RR(D);    break;	/* rr D */
			case 0x1b: RR(E);    break;	/* rr E */
			case 0x1c: RR(H);    break;	/* rr H */
			case 0x1d: RR(L);    break;	/* rr L */
			case 0x1e: RR_M(HL); break;	/* rr (HL) */
			case 0x1f: RR(A);    break;	/* rr A */

			case 0x20: SLA(B);    break;	/* sla B */
			case 0x21: SLA(C);    break;	/* sla C */
			case 0x22: SLA(D);    break;	/* sla D */
			case 0x23: SLA(E);    break;	/* sla E */
			case 0x24: SLA(H);    break;	/* sla H */
			case 0x25: SLA(L);    break;	/* sla L */
			case 0x26: SLA_M(HL); break;	/* sla (HL) */
			case 0x27: SLA(A);    break;	/* sla A */

			case 0x28: SRA(B);    break;	/* sra B */
			case 0x29: SRA(C);    break;	/* sra C */
			case 0x2a: SRA(D);    break;	/* sra D */
			case 0x2b: SRA(E);    break;	/* sra E */
			case 0x2c: SRA(H);    break;	/* sra H */
			case 0x2d: SRA(L);    break;	/* sra L */
			case 0x2e: SRA_M(HL); break;	/* sra (HL) */
			case 0x2f: SRA(A);    break;	/* sra A */

			case 0x30: SLL(B);    break;	/* sll B */
			case 0x31: SLL(C);    break;	/* sll C */
			case 0x32: SLL(D);    break;	/* sll D */
			case 0x33: SLL(E);    break;	/* sll E */
			case 0x34: SLL(H);    break;	/* sll H */
			case 0x35: SLL(L);    break;	/* sll L */
			case 0x36: SLL_M(HL); break;	/* sll (HL) */
			case 0x37: SLL(A);    break;	/* sll A */

			case 0x38: SRL(B);    break;	/* srl B */
			case 0x39: SRL(C);    break;	/* srl C */
			case 0x3a: SRL(D);    break;	/* srl D */
			case 0x3b: SRL(E);    break;	/* srl E */
			case 0x3c: SRL(H);    break;	/* srl H */
			case 0x3d: SRL(L);    break;	/* srl L */
			case 0x3e: SRL_M(HL); break;	/* srl (HL) */
			case 0x3f: SRL(A);    break;	/* srl A */

			case 0x40: BIT(0, B);        break;	/* bit 0, B */
			case 0x41: BIT(0, C);        break;	/* bit 0, C */
			case 0x42: BIT(0, D);        break;	/* bit 0, D */
			case 0x43: BIT(0, E);        break;	/* bit 0, E */
			case 0x44: BIT(0, H);        break;	/* bit 0, H */
			case 0x45: BIT(0, L);        break;	/* bit 0, L */
			case 0x46: BIT(0, MEM8(HL)); break;	/* bit 0, (HL) */
			case 0x47: BIT(0, A);        break;	/* bit 0, A */

			case 0x48: BIT(1, B);        break;	/* bit 1, B */
			case 0x49: BIT(1, C);        break;	/* bit 1, C */
			case 0x4a: BIT(1, D);        break;	/* bit 1, D */
			case 0x4b: BIT(1, E);        break;	/* bit 1, E */
			case 0x4c: BIT(1, H);        break;	/* bit 1, H */
			case 0x4d: BIT(1, L);        break;	/* bit 1, L */
			case 0x4e: BIT(1, MEM8(HL)); break;	/* bit 1, (HL) */
			case 0x4f: BIT(1, A);        break;	/* bit 1, A */

			case 0x50: BIT(2, B);        break;	/* bit 2, B */
			case 0x51: BIT(2, C);        break;	/* bit 2, C */
			case 0x52: BIT(2, D);        break;	/* bit 2, D */
			case 0x53: BIT(2, E);        break;	/* bit 2, E */
			case 0x54: BIT(2, H);        break;	/* bit 2, H */
			case 0x55: BIT(2, L);        break;	/* bit 2, L */
			case 0x56: BIT(2, MEM8(HL)); break;	/* bit 2, (HL) */
			case 0x57: BIT(2, A);        break;	/* bit 2, A */

			case 0x58: BIT(3, B);        break;	/* bit 3, B */
			case 0x59: BIT(3, C);        break;	/* bit 3, C */
			case 0x5a: BIT(3, D);        break;	/* bit 3, D */
			case 0x5b: BIT(3, E);        break;	/* bit 3, E */
			case 0x5c: BIT(3, H);        break;	/* bit 3, H */
			case 0x5d: BIT(3, L);        break;	/* bit 3, L */
			case 0x5e: BIT(3, MEM8(HL)); break;	/* bit 3, (HL) */
			case 0x5f: BIT(3, A);        break;	/* bit 3, A */

			case 0x60: BIT(4, B);        break;	/* bit 4, B */
			case 0x61: BIT(4, C);        break;	/* bit 4, C */
			case 0x62: BIT(4, D);        break;	/* bit 4, D */
			case 0x63: BIT(4, E);        break;	/* bit 4, E */
			case 0x64: BIT(4, H);        break;	/* bit 4, H */
			case 0x65: BIT(4, L);        break;	/* bit 4, L */
			case 0x66: BIT(4, MEM8(HL)); break;	/* bit 4, (HL) */
			case 0x67: BIT(4, A);        break;	/* bit 4, A */

			case 0x68: BIT(5, B);        break;	/* bit 5, B */
			case 0x69: BIT(5, C);        break;	/* bit 5, C */
			case 0x6a: BIT(5, D);        break;	/* bit 5, D */
			case 0x6b: BIT(5, E);        break;	/* bit 5, E */
			case 0x6c: BIT(5, H);        break;	/* bit 5, H */
			case 0x6d: BIT(5, L);        break;	/* bit 5, L */
			case 0x6e: BIT(5, MEM8(HL)); break;	/* bit 5, (HL) */
			case 0x6f: BIT(5, A);        break;	/* bit 5, A */

			case 0x70: BIT(6, B);        break;	/* bit 6, B */
			case 0x71: BIT(6, C);        break;	/* bit 6, C */
			case 0x72: BIT(6, D);        break;	/* bit 6, D */
			case 0x73: BIT(6, E);        break;	/* bit 6, E */
			case 0x74: BIT(6, H);        break;	/* bit 6, H */
			case 0x75: BIT(6, L);        break;	/* bit 6, L */
			case 0x76: BIT(6, MEM8(HL)); break;	/* bit 6, (HL) */
			case 0x77: BIT(6, A);        break;	/* bit 6, A */

			case 0x78: BIT(7, B);        break;	/* bit 7, B */
			case 0x79: BIT(7, C);        break;	/* bit 7, C */
			case 0x7a: BIT(7, D);        break;	/* bit 7, D */
			case 0x7b: BIT(7, E);        break;	/* bit 7, E */
			case 0x7c: BIT(7, H);        break;	/* bit 7, H */
			case 0x7d: BIT(7, L);        break;	/* bit 7, L */
			case 0x7e: BIT(7, MEM8(HL)); break;	/* bit 7, (HL) */
			case 0x7f: BIT(7, A);        break;	/* bit 7, A */

			case 0x80: RES(0, B);    break;	/* res 0, B */
			case 0x81: RES(0, C);    break;	/* res 0, C */
			case 0x82: RES(0, D);    break;	/* res 0, D */
			case 0x83: RES(0, E);    break;	/* res 0, E */
			case 0x84: RES(0, H);    break;	/* res 0, H */
			case 0x85: RES(0, L);    break;	/* res 0, L */
			case 0x86: RES_M(0, HL); break;	/* res 0, (HL) */
			case 0x87: RES(0, A);    break;	/* res 0, A */

			case 0x88: RES(1, B);    break;	/* res 1, B */
			case 0x89: RES(1, C);    break;	/* res 1, C */
			case 0x8a: RES(1, D);    break;	/* res 1, D */
			case 0x8b: RES(1, E);    break;	/* res 1, E */
			case 0x8c: RES(1, H);    break;	/* res 1, H */
			case 0x8d: RES(1, L);    break;	/* res 1, L */
			case 0x8e: RES_M(1, HL); break;	/* res 1, (HL) */
			case 0x8f: RES(1, A);    break;	/* res 1, A */

			case 0x90: RES(2, B);    break;	/* res 2, B */
			case 0x91: RES(2, C);    break;	/* res 2, C */
			case 0x92: RES(2, D);    break;	/* res 2, D */
			case 0x93: RES(2, E);    break;	/* res 2, E */
			case 0x94: RES(2, H);    break;	/* res 2, H */
			case 0x95: RES(2, L);    break;	/* res 2, L */
			case 0x96: RES_M(2, HL); break;	/* res 2, (HL) */
			case 0x97: RES(2, A);    break;	/* res 2, A */

			case 0x98: RES(3, B);    break;	/* res 3, B */
			case 0x99: RES(3, C);    break;	/* res 3, C */
			case 0x9a: RES(3, D);    break;	/* res 3, D */
			case 0x9b: RES(3, E);    break;	/* res 3, E */
			case 0x9c: RES(3, H);    break;	/* res 3, H */
			case 0x9d: RES(3, L);    break;	/* res 3, L */
			case 0x9e: RES_M(3, HL); break;	/* res 3, (HL) */
			case 0x9f: RES(3, A);    break;	/* res 3, A */

			case 0xa0: RES(4, B);    break;	/* res 4, B */
			case 0xa1: RES(4, C);    break;	/* res 4, C */
			case 0xa2: RES(4, D);    break;	/* res 4, D */
			case 0xa3: RES(4, E);    break;	/* res 4, E */
			case 0xa4: RES(4, H);    break;	/* res 4, H */
			case 0xa5: RES(4, L);    break;	/* res 4, L */
			case 0xa6: RES_M(4, HL); break;	/* res 4, (HL) */
			case 0xa7: RES(4, A);    break;	/* res 4, A */

			case 0xa8: RES(5, B);    break;	/* res 5, B */
			case 0xa9: RES(5, C);    break;	/* res 5, C */
			case 0xaa: RES(5, D);    break;	/* res 5, D */
			case 0xab: RES(5, E);    break;	/* res 5, E */
			case 0xac: RES(5, H);    break;	/* res 5, H */
			case 0xad: RES(5, L);    break;	/* res 5, L */
			case 0xae: RES_M(5, HL); break;	/* res 5, (HL) */
			case 0xaf: RES(5, A);    break;	/* res 5, A */

			case 0xb0: RES(6, B);    break;	/* res 6, B */
			case 0xb1: RES(6, C);    break;	/* res 6, C */
			case 0xb2: RES(6, D);    break;	/* res 6, D */
			case 0xb3: RES(6, E);    break;	/* res 6, E */
			case 0xb4: RES(6, H);    break;	/* res 6, H */
			case 0xb5: RES(6, L);    break;	/* res 6, L */
			case 0xb6: RES_M(6, HL); break;	/* res 6, (HL) */
			case 0xb7: RES(6, A);    break;	/* res 6, A */

			case 0xb8: RES(7, B);    break;	/* res 7, B */
			case 0xb9: RES(7, C);    break;	/* res 7, C */
			case 0xba: RES(7, D);    break;	/* res 7, D */
			case 0xbb: RES(7, E);    break;	/* res 7, E */
			case 0xbc: RES(7, H);    break;	/* res 7, H */
			case 0xbd: RES(7, L);    break;	/* res 7, L */
			case 0xbe: RES_M(7, HL); break;	/* res 7, (HL) */
			case 0xbf: RES(7, A);    break;	/* res 7, A */

			case 0xc0: SET(0, B);    break;	/* set 0, B */
			case 0xc1: SET(0, C);    break;	/* set 0, C */
			case 0xc2: SET(0, D);    break;	/* set 0, D */
			case 0xc3: SET(0, E);    break;	/* set 0, E */
			case 0xc4: SET(0, H);    break;	/* set 0, H */
			case 0xc5: SET(0, L);    break;	/* set 0, L */
			case 0xc6: SET_M(0, HL); break;	/* set 0, (HL) */
			case 0xc7: SET(0, A);    break;	/* set 0, A */

			case 0xc8: SET(1, B);    break;	/* set 1, B */
			case 0xc9: SET(1, C);    break;	/* set 1, C */
			case 0xca: SET(1, D);    break;	/* set 1, D */
			case 0xcb: SET(1, E);    break;	/* set 1, E */
			case 0xcc: SET(1, H);    break;	/* set 1, H */
			case 0xcd: SET(1, L);    break;	/* set 1, L */
			case 0xce: SET_M(1, HL); break;	/* set 1, (HL) */
			case 0xcf: SET(1, A);    break;	/* set 1, A */

			case 0xd0: SET(2, B);    break;	/* set 2, B */
			case 0xd1: SET(2, C);    break;	/* set 2, C */
			case 0xd2: SET(2, D);    break;	/* set 2, D */
			case 0xd3: SET(2, E);    break;	/* set 2, E */
			case 0xd4: SET(2, H);    break;	/* set 2, H */
			case 0xd5: SET(2, L);    break;	/* set 2, L */
			case 0xd6: SET_M(2, HL); break;	/* set 2, (HL) */
			case 0xd7: SET(2, A);    break;	/* set 2, A */

			case 0xd8: SET(3, B);    break;	/* set 3, B */
			case 0xd9: SET(3, C);    break;	/* set 3, C */
			case 0xda: SET(3, D);    break;	/* set 3, D */
			case 0xdb: SET(3, E);    break;	/* set 3, E */
			case 0xdc: SET(3, H);    break;	/* set 3, H */
			case 0xdd: SET(3, L);    break;	/* set 3, L */
			case 0xde: SET_M(3, HL); break;	/* set 3, (HL) */
			case 0xdf: SET(3, A);    break;	/* set 3, A */

			case 0xe0: SET(4, B);    break;	/* set 4, B */
			case 0xe1: SET(4, C);    break;	/* set 4, C */
			case 0xe2: SET(4, D);    break;	/* set 4, D */
			case 0xe3: SET(4, E);    break;	/* set 4, E */
			case 0xe4: SET(4, H);    break;	/* set 4, H */
			case 0xe5: SET(4, L);    break;	/* set 4, L */
			case 0xe6: SET_M(4, HL); break;	/* set 4, (HL) */
			case 0xe7: SET(4, A);    break;	/* set 4, A */

			case 0xe8: SET(5, B);    break;	/* set 5, B */
			case 0xe9: SET(5, C);    break;	/* set 5, C */
			case 0xea: SET(5, D);    break;	/* set 5, D */
			case 0xeb: SET(5, E);    break;	/* set 5, E */
			case 0xec: SET(5, H);    break;	/* set 5, H */
			case 0xed: SET(5, L);    break;	/* set 5, L */
			case 0xee: SET_M(5, HL); break;	/* set 5, (HL) */
			case 0xef: SET(5, A);    break;	/* set 5, A */

			case 0xf0: SET(6, B);    break;	/* set 6, B */
			case 0xf1: SET(6, C);    break;	/* set 6, C */
			case 0xf2: SET(6, D);    break;	/* set 6, D */
			case 0xf3: SET(6, E);    break;	/* set 6, E */
			case 0xf4: SET(6, H);    break;	/* set 6, H */
			case 0xf5: SET(6, L);    break;	/* set 6, L */
			case 0xf6: SET_M(6, HL); break;	/* set 6, (HL) */
			case 0xf7: SET(6, A);    break;	/* set 6, A */

			case 0xf8: SET(7, B);    break;	/* set 7, B */
			case 0xf9: SET(7, C);    break;	/* set 7, C */
			case 0xfa: SET(7, D);    break;	/* set 7, D */
			case 0xfb: SET(7, E);    break;	/* set 7, E */
			case 0xfc: SET(7, H);    break;	/* set 7, H */
			case 0xfd: SET(7, L);    break;	/* set 7, L */
			case 0xfe: SET_M(7, HL); break;	/* set 7, (HL) */
			case 0xff: SET(7, A);    break;	/* set 7, A */
			}
			break;
		case 0xcc: CALL(CC_Z, IMM16); break;	/* call Z, mn */
        case 0xcd: CALL(true, IMM16); break;	/* call mn */
		case 0xce: ADC8(IMM8);        break;	/* adc n */
		case 0xcf: RST(0x08);         break;	/* rst 08H */

		case 0xd0: RET(CC_NC);         break;	/* ret NC */
		case 0xd1: POP(DE);            break;	/* pop DE */
		case 0xd2: JP(CC_NC, IMM16);   break;	/* jp NC, mn */
		case 0xd3: OUT(IMM8, A);       break;	/* out (n), A */
		case 0xd4: CALL(CC_NC, IMM16); break;	/* call NC, mn */
		case 0xd5: PUSH(DE);           break;	/* push DE */
		case 0xd6: SUB8(IMM8);         break;	/* sub n */
		case 0xd7: RST(0x10);          break;	/* rst 10H */

		case 0xd8: RET(CC_C);         break;	/* ret C */
		case 0xd9: EXX();             break;	/* exx */
		case 0xda: JP(CC_C, IMM16);   break;	/* jp C, mn */
		case 0xdb: IN_N(A, IMM8);     break;	/* in A, (n) */
		case 0xdc: CALL(CC_C, IMM16); break;	/* call C, mn */
		case 0xdd:
			PC++;
			switch(FETCH_DD_XX()) {
			default: PC += _length; break;	/* nop */
			case 0x09: ADD16(IX, BC); break;	/* add IX, BC */

			case 0x19: ADD16(IX, DE); break;	/* add IX, DE */

			case 0x21: LD(IX, IMM16);   break;	/* ld IX, mn */
			case 0x22: ST16(IMM16, IX); break;	/* ld (mn), IX */
			case 0x23: INC16(IX);       break;	/* inc IX */
			case 0x24: INC8(IXH);       break;	/* inc IXh */
			case 0x25: DEC8(IXH);       break;	/* dec IXh */
			case 0x26: LD(IXH, IMM8);   break;	/* ld IXh, n */

			case 0x29: ADD16(IX, IX);        break;	/* add IX, IX */
			case 0x2a: LD(IX, MEM16(IMM16)); break;	/* ld IX, (mn) */
			case 0x2b: DEC16(IX);            break;	/* dec IX */
			case 0x2c: INC8(IXL);            break;	/* inc IXl */
			case 0x2d: DEC8(IXL);            break;	/* dec IXl */
			case 0x2e: LD(IXL, IMM8);        break;	/* ld IXl, n */

			case 0x34: INC8_M(IX + EQ);            break;	/* inc (IX + d) */
			case 0x35: DEC8_M(IX + EQ);            break;	/* dec (IX + d) */
			case 0x36: ST8(IX + EQ, MEM8(PC + 2)); break;	/* ld (IX + d), n */

			case 0x39: ADD16(IX, SP); break;	/* ADD IX, SP */

			case 0x44: LD(B, IXH);           break;	/* ld B, IXh */
			case 0x45: LD(B, IXL);           break;	/* ld B, IXl */
			case 0x46: LD(B, MEM8(IX + EQ)); break;	/* ld B, (IX + d) */

			case 0x4c: LD(C, IXH);           break;	/* ld C, IXh */
			case 0x4d: LD(C, IXL);           break;	/* ld C, IXl */
			case 0x4e: LD(C, MEM8(IX + EQ)); break;	/* ld C, (IX + d) */

			case 0x54: LD(D, IXH);           break;	/* ld D, IXh */
			case 0x55: LD(D, IXL);           break;	/* ld D, IXl */
			case 0x56: LD(D, MEM8(IX + EQ)); break;	/* ld D, (IX + d) */

			case 0x5c: LD(E, IXH);           break;	/* ld E, IXh */
			case 0x5d: LD(E, IXL);           break;	/* ld E, IXl */
			case 0x5e: LD(E, MEM8(IX + EQ)); break;	/* ld E, (IX + d) */

			case 0x60: LD(IXH, B);           break;	/* ld IXh, B */
			case 0x61: LD(IXH, C);           break;	/* ld IXh, C */
			case 0x62: LD(IXH, D);           break;	/* ld IXh, D */
			case 0x63: LD(IXH, E);           break;	/* ld IXh, E */
			case 0x64: LD(IXH, H);           break;	/* ld IXh, H */
			case 0x65: LD(IXH, L);           break;	/* ld IXh, L */
			case 0x66: LD(H, MEM8(IX + EQ)); break;	/* ld H, (IX + d) */
			case 0x67: LD(IXH, A);           break;	/* ld IXh, A */

			case 0x68: LD(IXL, B);           break;	/* ld IXl, B */
			case 0x69: LD(IXL, C);           break;	/* ld IXl, C */
			case 0x6a: LD(IXL, D);           break;	/* ld IXl, D */
			case 0x6b: LD(IXL, E);           break;	/* ld IXl, E */
			case 0x6c: LD(IXL, H);           break;	/* ld IXl, H */
			case 0x6d: LD(IXL, L);           break;	/* ld IXl, L */
			case 0x6e: LD(L, MEM8(IX + EQ)); break;	/* ld L, (IX + d) */
			case 0x6f: LD(IXL, A);           break;	/* ld IXl, A */

			case 0x70: ST8(IX + EQ, B); break;	/* ld (IX + d), B */
			case 0x71: ST8(IX + EQ, C); break;	/* ld (IX + d), C */
			case 0x72: ST8(IX + EQ, D); break;	/* ld (IX + d), D */
			case 0x73: ST8(IX + EQ, E); break;	/* ld (IX + d), E */
			case 0x74: ST8(IX + EQ, H); break;	/* ld (IX + d), H */
			case 0x75: ST8(IX + EQ, L); break;	/* ld (IX + d), L */
			case 0x77: ST8(IX + EQ, A); break;	/* ld (IX + d), A */

			case 0x7c: LD(A, IXH);           break;	/* ld A, IXh */
			case 0x7d: LD(A, IXL);           break;	/* ld A, IXl */
			case 0x7e: LD(A, MEM8(IX + EQ)); break;	/* ld A, (IX + d) */

			case 0x84: ADD8(IXH);           break;	/* add IXh */
			case 0x85: ADD8(IXL);           break;	/* add IXl */
			case 0x86: ADD8(MEM8(IX + EQ)); break;	/* add (IX + d) */

			case 0x8c: ADC8(IXH);           break;	/* adc IXh */
			case 0x8d: ADC8(IXL);           break;	/* adc IXl */
			case 0x8e: ADC8(MEM8(IX + EQ)); break;	/* adc (IX + d) */

			case 0x94: SUB8(IXH);           break;	/* sub IXh */
			case 0x95: SUB8(IXL);           break;	/* sub IXl */
			case 0x96: SUB8(MEM8(IX + EQ)); break;	/* sub (IX + d) */

			case 0x9c: SBC8(IXH);           break;	/* sbc IXh */
			case 0x9d: SBC8(IXL);           break;	/* sbc IXl */
			case 0x9e: SBC8(MEM8(IX + EQ)); break;	/* sbc (IX + d) */

			case 0xa4: AND(IXH);           break;	/* and IXh */
			case 0xa5: AND(IXL);           break;	/* and IXl */
			case 0xa6: AND(MEM8(IX + EQ)); break;	/* and (IX + d) */

			case 0xac: XOR(IXH);           break;	/* xor IXh */
			case 0xad: XOR(IXL);           break;	/* xor IXl */
			case 0xae: XOR(MEM8(IX + EQ)); break;	/* xor (IX + d) */

			case 0xb4: OR(IXH);           break;	/* or IXh */
			case 0xb5: OR(IXL);           break;	/* or IXl */
			case 0xb6: OR(MEM8(IX + EQ)); break;	/* or (IX + d) */

			case 0xbc: CP(IXH);           break;	/* cp IXh */
			case 0xbd: CP(IXL);           break;	/* cp IXl */
			case 0xbe: CP(MEM8(IX + EQ)); break;	/* cp (IX + d) */

			case 0xcb:
				switch(FETCH_DD_CB_XX()) {
				case 0x00: RLC_M_R(IX + EQ, B); break;	/* rlc (IX + d), B */
				case 0x01: RLC_M_R(IX + EQ, C); break;	/* rlc (IX + d), C */
				case 0x02: RLC_M_R(IX + EQ, D); break;	/* rlc (IX + d), D */
				case 0x03: RLC_M_R(IX + EQ, E); break;	/* rlc (IX + d), E */
				case 0x04: RLC_M_R(IX + EQ, H); break;	/* rlc (IX + d), H */
				case 0x05: RLC_M_R(IX + EQ, L); break;	/* rlc (IX + d), L */
				case 0x06: RLC_M(IX + EQ);      break;	/* rlc (IX + d) */
				case 0x07: RLC_M_R(IX + EQ, A); break;	/* rlc (IX + d), A */

				case 0x08: RRC_M_R(IX + EQ, B); break;	/* rrc (IX + d), B */
				case 0x09: RRC_M_R(IX + EQ, C); break;	/* rrc (IX + d), C */
				case 0x0a: RRC_M_R(IX + EQ, D); break;	/* rrc (IX + d), D */
				case 0x0b: RRC_M_R(IX + EQ, E); break;	/* rrc (IX + d), E */
				case 0x0c: RRC_M_R(IX + EQ, H); break;	/* rrc (IX + d), H */
				case 0x0d: RRC_M_R(IX + EQ, L); break;	/* rrc (IX + d), L */
				case 0x0e: RRC_M(IX + EQ);      break;	/* rrc (IX + d) */
				case 0x0f: RRC_M_R(IX + EQ, A); break;	/* rrc (IX + d), A */

				case 0x10: RL_M_R(IX + EQ, B); break;	/* rl (IX + d), B */
				case 0x11: RL_M_R(IX + EQ, C); break;	/* rl (IX + d), C */
				case 0x12: RL_M_R(IX + EQ, D); break;	/* rl (IX + d), D */
				case 0x13: RL_M_R(IX + EQ, E); break;	/* rl (IX + d), E */
				case 0x14: RL_M_R(IX + EQ, H); break;	/* rl (IX + d), H */
				case 0x15: RL_M_R(IX + EQ, L); break;	/* rl (IX + d), L */
				case 0x16: RL_M(IX + EQ);      break;	/* rl (IX + d) */
				case 0x17: RL_M_R(IX + EQ, A); break;	/* rl (IX + d), A */

				case 0x18: RR_M_R(IX + EQ, B); break;	/* rr (IX + d), B */
				case 0x19: RR_M_R(IX + EQ, C); break;	/* rr (IX + d), C */
				case 0x1a: RR_M_R(IX + EQ, D); break;	/* rr (IX + d), D */
				case 0x1b: RR_M_R(IX + EQ, E); break;	/* rr (IX + d), E */
				case 0x1c: RR_M_R(IX + EQ, H); break;	/* rr (IX + d), H */
				case 0x1d: RR_M_R(IX + EQ, L); break;	/* rr (IX + d), L */
				case 0x1e: RR_M(IX + EQ);      break;	/* rr (IX + d) */
				case 0x1f: RR_M_R(IX + EQ, A); break;	/* rr (IX + d), A */

				case 0x20: SLA_M_R(IX + EQ, B); break;	/* sla (IX + d), B */
				case 0x21: SLA_M_R(IX + EQ, C); break;	/* sla (IX + d), C */
				case 0x22: SLA_M_R(IX + EQ, D); break;	/* sla (IX + d), D */
				case 0x23: SLA_M_R(IX + EQ, E); break;	/* sla (IX + d), E */
				case 0x24: SLA_M_R(IX + EQ, H); break;	/* sla (IX + d), H */
				case 0x25: SLA_M_R(IX + EQ, L); break;	/* sla (IX + d), L */
				case 0x26: SLA_M(IX + EQ);      break;	/* sla (IX + d) */
				case 0x27: SLA_M_R(IX + EQ, A); break;	/* sla (IX + d), A */

				case 0x28: SRA_M_R(IX + EQ, B); break;	/* sra (IX + d), B */
				case 0x29: SRA_M_R(IX + EQ, C); break;	/* sra (IX + d), C */
				case 0x2a: SRA_M_R(IX + EQ, D); break;	/* sra (IX + d), D */
				case 0x2b: SRA_M_R(IX + EQ, E); break;	/* sra (IX + d), E */
				case 0x2c: SRA_M_R(IX + EQ, H); break;	/* sra (IX + d), H */
				case 0x2d: SRA_M_R(IX + EQ, L); break;	/* sra (IX + d), L */
				case 0x2e: SRA_M(IX + EQ);      break;	/* sra (IX + d) */
				case 0x2f: SRA_M_R(IX + EQ, A); break;	/* sra (IX + d), A */

				case 0x30: SLL_M_R(IX + EQ, B); break;	/* sll (IX + d), B */
				case 0x31: SLL_M_R(IX + EQ, C); break;	/* sll (IX + d), C */
				case 0x32: SLL_M_R(IX + EQ, D); break;	/* sll (IX + d), D */
				case 0x33: SLL_M_R(IX + EQ, E); break;	/* sll (IX + d), E */
				case 0x34: SLL_M_R(IX + EQ, H); break;	/* sll (IX + d), H */
				case 0x35: SLL_M_R(IX + EQ, L); break;	/* sll (IX + d), L */
				case 0x36: SLL_M(IX + EQ);      break;	/* sll (IX + d) */
				case 0x37: SLL_M_R(IX + EQ, A); break;	/* sll (IX + d), A */

				case 0x38: SRL_M_R(IX + EQ, B); break;	/* srl (IX + d), B */
				case 0x39: SRL_M_R(IX + EQ, C); break;	/* srl (IX + d), C */
				case 0x3a: SRL_M_R(IX + EQ, D); break;	/* srl (IX + d), D */
				case 0x3b: SRL_M_R(IX + EQ, E); break;	/* srl (IX + d), E */
				case 0x3c: SRL_M_R(IX + EQ, H); break;	/* srl (IX + d), H */
				case 0x3d: SRL_M_R(IX + EQ, L); break;	/* srl (IX + d), L */
				case 0x3e: SRL_M(IX + EQ);      break;	/* srl (IX + d) */
				case 0x3f: SRL_M_R(IX + EQ, A); break;	/* srl (IX + d), A */

				case 0x40:
				case 0x41:
				case 0x42:
				case 0x43:
				case 0x44:
				case 0x45:
				case 0x46:
				case 0x47: BIT(0, MEM8(IX + EQ)); break;	/* bit 0, (IX + d) */

				case 0x48:
				case 0x49:
				case 0x4a:
				case 0x4b:
				case 0x4c:
				case 0x4d:
				case 0x4e:
				case 0x4f: BIT(1, MEM8(IX + EQ)); break;	/* bit 1, (IX + d) */

				case 0x50:
				case 0x51:
				case 0x52:
				case 0x53:
				case 0x54:
				case 0x55:
				case 0x56:
				case 0x57: BIT(2, MEM8(IX + EQ)); break;	/* bit 2, (IX + d) */

				case 0x58:
				case 0x59:
				case 0x5a:
				case 0x5b:
				case 0x5c:
				case 0x5d:
				case 0x5e:
				case 0x5f: BIT(3, MEM8(IX + EQ)); break;	/* bit 3, (IX + d) */

				case 0x60:
				case 0x61:
				case 0x62:
				case 0x63:
				case 0x64:
				case 0x65:
				case 0x66:
				case 0x67: BIT(4, MEM8(IX + EQ)); break;	/* bit 4, (IX + d) */

				case 0x68:
				case 0x69:
				case 0x6a:
				case 0x6b:
				case 0x6c:
				case 0x6d:
				case 0x6e:
				case 0x6f: BIT(5, MEM8(IX + EQ)); break;	/* bit 5, (IX + d) */

				case 0x70:
				case 0x71:
				case 0x72:
				case 0x73:
				case 0x74:
				case 0x75:
				case 0x76:
				case 0x77: BIT(6, MEM8(IX + EQ)); break;	/* bit 6, (IX + d) */

				case 0x78:
				case 0x79:
				case 0x7a:
				case 0x7b:
				case 0x7c:
				case 0x7d:
				case 0x7e:
				case 0x7f: BIT(7, MEM8(IX + EQ)); break;	/* bit 7, (IX + d) */

				case 0x80: RES_M_R(0, IX + EQ, B); break;	/* res 0, (IX + d), B */
				case 0x81: RES_M_R(0, IX + EQ, C); break;	/* res 0, (IX + d), C */
				case 0x82: RES_M_R(0, IX + EQ, D); break;	/* res 0, (IX + d), D */
				case 0x83: RES_M_R(0, IX + EQ, E); break;	/* res 0, (IX + d), E */
				case 0x84: RES_M_R(0, IX + EQ, H); break;	/* res 0, (IX + d), H */
				case 0x85: RES_M_R(0, IX + EQ, L); break;	/* res 0, (IX + d), L */
				case 0x86: RES_M(0, IX + EQ);      break;	/* res 0, (IX + d) */
				case 0x87: RES_M_R(0, IX + EQ, A); break;	/* res 0, (IX + d), A */

				case 0x88: RES_M_R(1, IX + EQ, B); break;	/* res 1, (IX + d), B */
				case 0x89: RES_M_R(1, IX + EQ, C); break;	/* res 1, (IX + d), C */
				case 0x8a: RES_M_R(1, IX + EQ, D); break;	/* res 1, (IX + d), D */
				case 0x8b: RES_M_R(1, IX + EQ, E); break;	/* res 1, (IX + d), E */
				case 0x8c: RES_M_R(1, IX + EQ, H); break;	/* res 1, (IX + d), H */
				case 0x8d: RES_M_R(1, IX + EQ, L); break;	/* res 1, (IX + d), L */
				case 0x8e: RES_M(1, IX + EQ);      break;	/* res 1, (IX + d) */
				case 0x8f: RES_M_R(1, IX + EQ, A); break;	/* res 1, (IX + d), A */

				case 0x90: RES_M_R(2, IX + EQ, B); break;	/* res 2, (IX + d), B */
				case 0x91: RES_M_R(2, IX + EQ, C); break;	/* res 2, (IX + d), C */
				case 0x92: RES_M_R(2, IX + EQ, D); break;	/* res 2, (IX + d), D */
				case 0x93: RES_M_R(2, IX + EQ, E); break;	/* res 2, (IX + d), E */
				case 0x94: RES_M_R(2, IX + EQ, H); break;	/* res 2, (IX + d), H */
				case 0x95: RES_M_R(2, IX + EQ, L); break;	/* res 2, (IX + d), L */
				case 0x96: RES_M(2, IX + EQ);      break;	/* res 2, (IX + d) */
				case 0x97: RES_M_R(2, IX + EQ, A); break;	/* res 2, (IX + d), A */

				case 0x98: RES_M_R(3, IX + EQ, B); break;	/* res 3, (IX + d), B */
				case 0x99: RES_M_R(3, IX + EQ, C); break;	/* res 3, (IX + d), C */
				case 0x9a: RES_M_R(3, IX + EQ, D); break;	/* res 3, (IX + d), D */
				case 0x9b: RES_M_R(3, IX + EQ, E); break;	/* res 3, (IX + d), E */
				case 0x9c: RES_M_R(3, IX + EQ, H); break;	/* res 3, (IX + d), H */
				case 0x9d: RES_M_R(3, IX + EQ, L); break;	/* res 3, (IX + d), L */
				case 0x9e: RES_M(3, IX + EQ);      break;	/* res 3, (IX + d) */
				case 0x9f: RES_M_R(3, IX + EQ, A); break;	/* res 3, (IX + d), A */

				case 0xa0: RES_M_R(4, IX + EQ, B); break;	/* res 4, (IX + d), B */
				case 0xa1: RES_M_R(4, IX + EQ, C); break;	/* res 4, (IX + d), C */
				case 0xa2: RES_M_R(4, IX + EQ, D); break;	/* res 4, (IX + d), D */
				case 0xa3: RES_M_R(4, IX + EQ, E); break;	/* res 4, (IX + d), E */
				case 0xa4: RES_M_R(4, IX + EQ, H); break;	/* res 4, (IX + d), H */
				case 0xa5: RES_M_R(4, IX + EQ, L); break;	/* res 4, (IX + d), L */
				case 0xa6: RES_M(4, IX + EQ);      break;	/* res 4, (IX + d) */
				case 0xa7: RES_M_R(4, IX + EQ, A); break;	/* res 4, (IX + d), A */

				case 0xa8: RES_M_R(5, IX + EQ, B); break;	/* res 5, (IX + d), B */
				case 0xa9: RES_M_R(5, IX + EQ, C); break;	/* res 5, (IX + d), C */
				case 0xaa: RES_M_R(5, IX + EQ, D); break;	/* res 5, (IX + d), D */
				case 0xab: RES_M_R(5, IX + EQ, E); break;	/* res 5, (IX + d), E */
				case 0xac: RES_M_R(5, IX + EQ, H); break;	/* res 5, (IX + d), H */
				case 0xad: RES_M_R(5, IX + EQ, L); break;	/* res 5, (IX + d), L */
				case 0xae: RES_M(5, IX + EQ);      break;	/* res 5, (IX + d) */
				case 0xaf: RES_M_R(5, IX + EQ, A); break;	/* res 5, (IX + d), A */

				case 0xb0: RES_M_R(6, IX + EQ, B); break;	/* res 6, (IX + d), B */
				case 0xb1: RES_M_R(6, IX + EQ, C); break;	/* res 6, (IX + d), C */
				case 0xb2: RES_M_R(6, IX + EQ, D); break;	/* res 6, (IX + d), D */
				case 0xb3: RES_M_R(6, IX + EQ, E); break;	/* res 6, (IX + d), E */
				case 0xb4: RES_M_R(6, IX + EQ, H); break;	/* res 6, (IX + d), H */
				case 0xb5: RES_M_R(6, IX + EQ, L); break;	/* res 6, (IX + d), L */
				case 0xb6: RES_M(6, IX + EQ);      break;	/* res 6, (IX + d) */
				case 0xb7: RES_M_R(6, IX + EQ, A); break;	/* res 6, (IX + d), A */

				case 0xb8: RES_M_R(7, IX + EQ, B); break;	/* res 7, (IX + d), B */
				case 0xb9: RES_M_R(7, IX + EQ, C); break;	/* res 7, (IX + d), C */
				case 0xba: RES_M_R(7, IX + EQ, D); break;	/* res 7, (IX + d), D */
				case 0xbb: RES_M_R(7, IX + EQ, E); break;	/* res 7, (IX + d), E */
				case 0xbc: RES_M_R(7, IX + EQ, H); break;	/* res 7, (IX + d), H */
				case 0xbd: RES_M_R(7, IX + EQ, L); break;	/* res 7, (IX + d), L */
				case 0xbe: RES_M(7, IX + EQ);      break;	/* res 7, (IX + d) */
				case 0xbf: RES_M_R(7, IX + EQ, A); break;	/* res 7, (IX + d), A */

				case 0xc0: SET_M_R(0, IX + EQ, B); break;	/* set 0, (IX + d), B */
				case 0xc1: SET_M_R(0, IX + EQ, C); break;	/* set 0, (IX + d), C */
				case 0xc2: SET_M_R(0, IX + EQ, D); break;	/* set 0, (IX + d), D */
				case 0xc3: SET_M_R(0, IX + EQ, E); break;	/* set 0, (IX + d), E */
				case 0xc4: SET_M_R(0, IX + EQ, H); break;	/* set 0, (IX + d), H */
				case 0xc5: SET_M_R(0, IX + EQ, L); break;	/* set 0, (IX + d), L */
				case 0xc6: SET_M(0, IX + EQ);      break;	/* set 0, (IX + d) */
				case 0xc7: SET_M_R(0, IX + EQ, A); break;	/* set 0, (IX + d), A */

				case 0xc8: SET_M_R(1, IX + EQ, B); break;	/* set 1, (IX + d), B */
				case 0xc9: SET_M_R(1, IX + EQ, C); break;	/* set 1, (IX + d), C */
				case 0xca: SET_M_R(1, IX + EQ, D); break;	/* set 1, (IX + d), D */
				case 0xcb: SET_M_R(1, IX + EQ, E); break;	/* set 1, (IX + d), E */
				case 0xcc: SET_M_R(1, IX + EQ, H); break;	/* set 1, (IX + d), H */
				case 0xcd: SET_M_R(1, IX + EQ, L); break;	/* set 1, (IX + d), L */
				case 0xce: SET_M(1, IX + EQ);      break;	/* set 1, (IX + d) */
				case 0xcf: SET_M_R(1, IX + EQ, A); break;	/* set 1, (IX + d), A */

				case 0xd0: SET_M_R(2, IX + EQ, B); break;	/* set 2, (IX + d), B */
				case 0xd1: SET_M_R(2, IX + EQ, C); break;	/* set 2, (IX + d), C */
				case 0xd2: SET_M_R(2, IX + EQ, D); break;	/* set 2, (IX + d), D */
				case 0xd3: SET_M_R(2, IX + EQ, E); break;	/* set 2, (IX + d), E */
				case 0xd4: SET_M_R(2, IX + EQ, H); break;	/* set 2, (IX + d), H */
				case 0xd5: SET_M_R(2, IX + EQ, L); break;	/* set 2, (IX + d), L */
				case 0xd6: SET_M(2, IX + EQ);      break;	/* set 2, (IX + d) */
				case 0xd7: SET_M_R(2, IX + EQ, A); break;	/* set 2, (IX + d), A */

				case 0xd8: SET_M_R(3, IX + EQ, B); break;	/* set 3, (IX + d), B */
				case 0xd9: SET_M_R(3, IX + EQ, C); break;	/* set 3, (IX + d), C */
				case 0xda: SET_M_R(3, IX + EQ, D); break;	/* set 3, (IX + d), D */
				case 0xdb: SET_M_R(3, IX + EQ, E); break;	/* set 3, (IX + d), E */
				case 0xdc: SET_M_R(3, IX + EQ, H); break;	/* set 3, (IX + d), H */
				case 0xdd: SET_M_R(3, IX + EQ, L); break;	/* set 3, (IX + d), L */
				case 0xde: SET_M(3, IX + EQ);      break;	/* set 3, (IX + d) */
				case 0xdf: SET_M_R(3, IX + EQ, A); break;	/* set 3, (IX + d), A */

				case 0xe0: SET_M_R(4, IX + EQ, B); break;	/* set 4, (IX + d), B */
				case 0xe1: SET_M_R(4, IX + EQ, C); break;	/* set 4, (IX + d), C */
				case 0xe2: SET_M_R(4, IX + EQ, D); break;	/* set 4, (IX + d), D */
				case 0xe3: SET_M_R(4, IX + EQ, E); break;	/* set 4, (IX + d), E */
				case 0xe4: SET_M_R(4, IX + EQ, H); break;	/* set 4, (IX + d), H */
				case 0xe5: SET_M_R(4, IX + EQ, L); break;	/* set 4, (IX + d), L */
				case 0xe6: SET_M(4, IX + EQ);      break;	/* set 4, (IX + d) */
				case 0xe7: SET_M_R(4, IX + EQ, A); break;	/* set 4, (IX + d), A */

				case 0xe8: SET_M_R(5, IX + EQ, B); break;	/* set 5, (IX + d), B */
				case 0xe9: SET_M_R(5, IX + EQ, C); break;	/* set 5, (IX + d), C */
				case 0xea: SET_M_R(5, IX + EQ, D); break;	/* set 5, (IX + d), D */
				case 0xeb: SET_M_R(5, IX + EQ, E); break;	/* set 5, (IX + d), E */
				case 0xec: SET_M_R(5, IX + EQ, H); break;	/* set 5, (IX + d), H */
				case 0xed: SET_M_R(5, IX + EQ, L); break;	/* set 5, (IX + d), L */
				case 0xee: SET_M(5, IX + EQ);      break;	/* set 5, (IX + d) */
				case 0xef: SET_M_R(5, IX + EQ, A); break;	/* set 5, (IX + d), A */

				case 0xf0: SET_M_R(6, IX + EQ, B); break;	/* set 6, (IX + d), B */
				case 0xf1: SET_M_R(6, IX + EQ, C); break;	/* set 6, (IX + d), C */
				case 0xf2: SET_M_R(6, IX + EQ, D); break;	/* set 6, (IX + d), D */
				case 0xf3: SET_M_R(6, IX + EQ, E); break;	/* set 6, (IX + d), E */
				case 0xf4: SET_M_R(6, IX + EQ, H); break;	/* set 6, (IX + d), H */
				case 0xf5: SET_M_R(6, IX + EQ, L); break;	/* set 6, (IX + d), L */
				case 0xf6: SET_M(6, IX + EQ);      break;	/* set 6, (IX + d) */
				case 0xf7: SET_M_R(6, IX + EQ, A); break;	/* set 6, (IX + d), A */

				case 0xf8: SET_M_R(7, IX + EQ, B); break;	/* set 7, (IX + d), B */
				case 0xf9: SET_M_R(7, IX + EQ, C); break;	/* set 7, (IX + d), C */
				case 0xfa: SET_M_R(7, IX + EQ, D); break;	/* set 7, (IX + d), D */
				case 0xfb: SET_M_R(7, IX + EQ, E); break;	/* set 7, (IX + d), E */
				case 0xfc: SET_M_R(7, IX + EQ, H); break;	/* set 7, (IX + d), H */
				case 0xfd: SET_M_R(7, IX + EQ, L); break;	/* set 7, (IX + d), L */
				case 0xfe: SET_M(7, IX + EQ);      break;	/* set 7, (IX + d) */
				case 0xff: SET_M_R(7, IX + EQ, A); break;	/* set 7, (IX + d), A */
				}
				break;
			case 0xe1: POP(IX);      break;	/* pop IX */
			case 0xe3: EX_M(SP, IX); break;	/* ex (SP), IX */
			case 0xe5: PUSH(IX);     break;	/* push IX */

            case 0xe9: JP(true, IX); break;	/* jp (IX) */

			case 0xf9: LD(SP, IX); break;	/* ld SP, IX */
			}
			break;
		case 0xde: SBC8(IMM8);        break;	/* sbc n */
		case 0xdf: RST(0x18);         break;	/* rst 18H */

		case 0xe0: RET(CC_PO);         break;	/* ret PO */
		case 0xe1: POP(HL);            break;	/* pop HL */
		case 0xe2: JP(CC_PO, IMM16);   break;	/* jp PO, mn */
		case 0xe3: EX_M(SP, HL);       break;	/* ex (SP), HL */
		case 0xe4: CALL(CC_PO, IMM16); break;	/* call PO, mn */
		case 0xe5: PUSH(HL);           break;	/* push HL */
		case 0xe6: AND(IMM8);          break;	/* and n */
		case 0xe7: RST(0x20);          break;	/* rst 20H */

		case 0xe8: RET(CC_PE);          break;	/* ret PE */
        case 0xe9: JP(true, HL);        break;	/* jp (HL) */
		case 0xea: JP(CC_PE, IMM16);    break;	/* jp PE, mn */
		case 0xeb: EX_R(DE, HL);        break;	/* ex DE, HL */
		case 0xec: CALL(CC_PE, IMM16);  break;	/* call PE, mn */
		case 0xed:
			PC++;
			switch(FETCH_ED_XX()) {
			default: PC += _length; break;	/* nop */
			case 0x40: IN_C(B, C);      break;	/* in B, (C) */
			case 0x41: OUT(C, B);       break;	/* out (C), B */
			case 0x42: SBC16(HL, BC);   break;	/* sbc HL, BC */
			case 0x43: ST16(IMM16, BC); break;	/* ld (mn), BC */
			case 0x44: NEG();           break;	/* neg */
			case 0x45: RETN();          break;	/* retn */
			case 0x46: IM(0);           break;	/* im 0 */
			case 0x47: LD(I, A);        break;	/* ld I, A */

            case 0x48: IN_C(C, C);      break;	/* in C, (C) */
			case 0x49: OUT(C, C);            break;	/* out (C), C */
			case 0x4a: ADC16(HL, BC);        break;	/* adc HL, BC */
			case 0x4b: LD(BC, MEM16(IMM16)); break;	/* ld BC, (mn) */
			case 0x4c: NEG();                break;	/* neg */
			case 0x4d: RETI();               break;	/* reti */
			case 0x4e: IM(0);                break;	/* im 0 */
			case 0x4f: NOP();                break;	/* ld R, A */

			case 0x50: IN_C(D, C);      break;	/* in D, (C) */
			case 0x51: OUT(C, D);       break;	/* out (C), D */
			case 0x52: SBC16(HL, DE);   break;	/* sbc HL, DE */
			case 0x53: ST16(IMM16, DE); break;	/* ld (mn), DE */
			case 0x54: NEG();           break;	/* neg */
			case 0x55: RETN();          break;	/* retn */
			case 0x56: IM(1);           break;	/* im 1 */
			case 0x57: LD_A_I();        break;	/* ld A, I */

			case 0x58: IN_C(E, C);           break;	/* in E, (C) */
			case 0x59: OUT(C, E);            break;	/* out (C), E */
			case 0x5a: ADC16(HL, DE);        break;	/* adc HL, DE */
			case 0x5b: LD(DE, MEM16(IMM16)); break;	/* ld DE, (mn) */
			case 0x5c: NEG();                break;	/* neg */
			case 0x5d: RETN();               break;	/* retn */
			case 0x5e: IM(2);                break;	/* im 2 */
			case 0x5f: LD_A_R();             break;	/* ld A, R */

			case 0x60: IN_C(H, C);      break;	/* in H, (C) */
			case 0x61: OUT(C, H);       break;	/* out (C), H */
			case 0x62: SBC16(HL, HL);   break;	/* sbc HL, HL */
			case 0x63: ST16(IMM16, HL); break;	/* ld (mn), HL */
			case 0x64: NEG();           break;	/* neg */
			case 0x65: RETN();          break;	/* retn */
			case 0x66: IM(0);           break;	/* im 0 */
			case 0x67: RRD();           break;	/* rrd */

			case 0x68: IN_C(L, C);           break;	/* in L, (C) */
			case 0x69: OUT(C, L);            break;	/* out (C), L */
			case 0x6a: ADC16(HL, HL);        break;	/* adc HL, HL */
			case 0x6b: LD(HL, MEM16(IMM16)); break;	/* ld HL, (mn) */
			case 0x6c: NEG();                break;	/* neg */
			case 0x6d: RETN();               break;	/* retn */
			case 0x6e: IM(0);                break;	/* im 0 */
			case 0x6f: RLD();                break;	/* rld */

			case 0x70: IN_C(F, C);      break;	/* in F, (C) */
			case 0x71: OUT(C, 0);       break;	/* out (C), 0) */
			case 0x72: SBC16(HL, SP);   break;	/* sbc HL, SP */
			case 0x73: ST16(IMM16, SP); break;	/* ld (mn), SP */
			case 0x74: NEG();           break;	/* neg */
			case 0x75: RETN();          break;	/* retn */
			case 0x76: IM(1);           break;	/* im 1 */

			case 0x78: IN_C(A, C);           break;	/* in A, (C) */
			case 0x79: OUT(C, A);            break;	/* out (C), A */
			case 0x7a: ADC16(HL, SP);        break;	/* adc HL, SP */
			case 0x7b: LD(SP, MEM16(IMM16)); break;	/* ld SP, (mn) */
			case 0x7c: NEG();                break;	/* neg */
			case 0x7d: RETN();               break;	/* retn */
			case 0x7e: IM(2);                break;	/* im 2 */

			case 0xa0: LDI();  break;	/* ldi */
			case 0xa1: CPI();  break;	/* cpi */
			case 0xa2: INI();  break;	/* ini */
			case 0xa3: OUTI(); break;	/* outi */

			case 0xa8: LDD();  break;	/* ldd */
			case 0xa9: CPD();  break;	/* cpd */
			case 0xaa: IND();  break;	/* ind */
			case 0xab: OUTD(); break;	/* outd */

			case 0xb0: LDIR(); break;	/* ldir */
			case 0xb1: CPIR(); break;	/* cpir */
			case 0xb2: INIR(); break;	/* inir */
			case 0xb3: OTIR(); break;	/* otir */

			case 0xb8: LDDR(); break;	/* lddr */
			case 0xb9: CPDR(); break;	/* cpdr */
			case 0xba: INDR(); break;	/* indr */
			case 0xbb: OTDR(); break;	/* otdr */
			}
			break;
		case 0xee: XOR(IMM8);          break;	/* xor n */
		case 0xef: RST(0x28);          break;	/* rst 28H */

		case 0xf0: RET(CC_P);         break;	/* ret P */
		case 0xf1: POP(AF);           break;	/* pop AF */
		case 0xf2: JP(CC_P, IMM16);   break;	/* jp P, mn */
		case 0xf3: DI();              break;	/* di */
		case 0xf4: CALL(CC_P, IMM16); break;	/* call P, mn */
		case 0xf5: PUSH(AF);          break;	/* push AF */
		case 0xf6: OR(IMM8);          break;	/* or n */
		case 0xf7: RST(0x30);         break;	/* rst 30H */

		case 0xf8: RET(CC_M);         break;	/* ret M */
		case 0xf9: LD(SP, HL);        break;	/* ld SP, HL */
		case 0xfa: JP(CC_M, IMM16);   break;	/* jp M, mn */
		case 0xfb: EI();              break;	/* ei */
		case 0xfc: CALL(CC_M, IMM16); break;	/* call M, mn */
		case 0xfd:
			PC++;
			switch(FETCH_FD_XX()) {
			default: PC += _length; break;	/* nop */
			case 0x09: ADD16(IY, BC); break;	/* add IY, BC */

			case 0x19: ADD16(IY, DE); break;	/* add IY, DE */

			case 0x21: LD(IY, IMM16);   break;	/* ld IY, mn */
			case 0x22: ST16(IMM16, IY); break;	/* ld (mn), IY */
			case 0x23: INC16(IY);       break;	/* inc IY */
			case 0x24: INC8(IYH);       break;	/* inc IYh */
			case 0x25: DEC8(IYH);       break;	/* dec IYh */
			case 0x26: LD(IYH, IMM8);   break;	/* ld IYh, n */

			case 0x29: ADD16(IY, IY);        break;	/* add IY, IY */
			case 0x2a: LD(IY, MEM16(IMM16)); break;	/* ld IY, (mn) */
			case 0x2b: DEC16(IY);            break;	/* dec IY */
			case 0x2c: INC8(IYL);            break;	/* inc IYl */
			case 0x2d: DEC8(IYL);            break;	/* dec IYl */
			case 0x2e: LD(IYL, IMM8);        break;	/* ld IYl, n */

			case 0x34: INC8_M(IY + EQ);            break;	/* inc (IY + d) */
			case 0x35: DEC8_M(IY + EQ);            break;	/* dec (IY + d) */
			case 0x36: ST8(IY + EQ, MEM8(PC + 2)); break;	/* ld (IY + d), n */

			case 0x39: ADD16(IY, SP); break;	/* ADD IY, SP */

			case 0x44: LD(B, IYH);           break;	/* ld B, IYh */
			case 0x45: LD(B, IYL);           break;	/* ld B, IYl */
			case 0x46: LD(B, MEM8(IY + EQ)); break;	/* ld B, (IY + d) */

			case 0x4c: LD(C, IYH);           break;	/* ld C, IYh */
			case 0x4d: LD(C, IYL);           break;	/* ld C, IYl */
			case 0x4e: LD(C, MEM8(IY + EQ)); break;	/* ld C, (IY + d) */

			case 0x54: LD(D, IYH);           break;	/* ld D, IYh */
			case 0x55: LD(D, IYL);           break;	/* ld D, IYl */
			case 0x56: LD(D, MEM8(IY + EQ)); break;	/* ld D, (IY + d) */

			case 0x5c: LD(E, IYH);           break;	/* ld E, IYh */
			case 0x5d: LD(E, IYL);           break;	/* ld E, IYl */
			case 0x5e: LD(E, MEM8(IY + EQ)); break;	/* ld E, (IY + d) */

			case 0x60: LD(IYH, B);           break;	/* ld IYh, B */
			case 0x61: LD(IYH, C);           break;	/* ld IYh, C */
			case 0x62: LD(IYH, D);           break;	/* ld IYh, D */
			case 0x63: LD(IYH, E);           break;	/* ld IYh, E */
			case 0x64: LD(IYH, H);           break;	/* ld IYh, H */
			case 0x65: LD(IYH, L);           break;	/* ld IYh, L */
			case 0x66: LD(H, MEM8(IY + EQ)); break;	/* ld H, (IY + d) */
			case 0x67: LD(IYH, A);           break;	/* ld IYh, A */

			case 0x68: LD(IYL, B);           break;	/* ld IYl, B */
			case 0x69: LD(IYL, C);           break;	/* ld IYl, C */
			case 0x6a: LD(IYL, D);           break;	/* ld IYl, D */
			case 0x6b: LD(IYL, E);           break;	/* ld IYl, E */
			case 0x6c: LD(IYL, H);           break;	/* ld IYl, H */
			case 0x6d: LD(IYL, L);           break;	/* ld IYl, L */
			case 0x6e: LD(L, MEM8(IY + EQ)); break;	/* ld L, (IY + d) */
			case 0x6f: LD(IYL, A);           break;	/* ld IYl, A */

			case 0x70: ST8(IY + EQ, B); break;	/* ld (IY + d), B */
			case 0x71: ST8(IY + EQ, C); break;	/* ld (IY + d), C */
			case 0x72: ST8(IY + EQ, D); break;	/* ld (IY + d), D */
			case 0x73: ST8(IY + EQ, E); break;	/* ld (IY + d), E */
			case 0x74: ST8(IY + EQ, H); break;	/* ld (IY + d), H */
			case 0x75: ST8(IY + EQ, L); break;	/* ld (IY + d), L */
			case 0x77: ST8(IY + EQ, A); break;	/* ld (IY + d), A */

			case 0x7c: LD(A, IYH);           break;	/* ld A, IYh */
			case 0x7d: LD(A, IYL);           break;	/* ld A, IYl */
			case 0x7e: LD(A, MEM8(IY + EQ)); break;	/* ld A, (IY + d) */

			case 0x84: ADD8(IYH);           break;	/* add IYh */
			case 0x85: ADD8(IYL);           break;	/* add IYl */
			case 0x86: ADD8(MEM8(IY + EQ)); break;	/* add (IY + d) */

			case 0x8c: ADC8(IYH);           break;	/* adc IYh */
			case 0x8d: ADC8(IYL);           break;	/* adc IYl */
			case 0x8e: ADC8(MEM8(IY + EQ)); break;	/* adc (IY + d) */

			case 0x94: SUB8(IYH);           break;	/* sub IYh */
			case 0x95: SUB8(IYL);           break;	/* sub IYl */
			case 0x96: SUB8(MEM8(IY + EQ)); break;	/* sub (IY + d) */

			case 0x9c: SBC8(IYH);           break;	/* sbc IYh */
			case 0x9d: SBC8(IYL);           break;	/* sbc IYl */
			case 0x9e: SBC8(MEM8(IY + EQ)); break;	/* sbc (IY + d) */

			case 0xa4: AND(IYH);           break;	/* and IYh */
			case 0xa5: AND(IYL);           break;	/* and IYl */
			case 0xa6: AND(MEM8(IY + EQ)); break;	/* and (IY + d) */

			case 0xac: XOR(IYH);           break;	/* xor IYh */
			case 0xad: XOR(IYL);           break;	/* xor IYl */
			case 0xae: XOR(MEM8(IY + EQ)); break;	/* xor (IY + d) */

			case 0xb4: OR(IYH);           break;	/* or IYh */
			case 0xb5: OR(IYL);           break;	/* or IYl */
			case 0xb6: OR(MEM8(IY + EQ)); break;	/* or (IY + d) */

			case 0xbc: CP(IYH);           break;	/* cp IYh */
			case 0xbd: CP(IYL);           break;	/* cp IYl */
			case 0xbe: CP(MEM8(IY + EQ)); break;	/* cp (IY + d) */

			case 0xcb:
				switch(FETCH_FD_CB_XX()) {
				case 0x00: RLC_M_R(IY + EQ, B); break;	/* rlc (IY + d), B */
				case 0x01: RLC_M_R(IY + EQ, C); break;	/* rlc (IY + d), C */
				case 0x02: RLC_M_R(IY + EQ, D); break;	/* rlc (IY + d), D */
				case 0x03: RLC_M_R(IY + EQ, E); break;	/* rlc (IY + d), E */
				case 0x04: RLC_M_R(IY + EQ, H); break;	/* rlc (IY + d), H */
				case 0x05: RLC_M_R(IY + EQ, L); break;	/* rlc (IY + d), L */
				case 0x06: RLC_M(IY + EQ);      break;	/* rlc (IY + d) */
				case 0x07: RLC_M_R(IY + EQ, A); break;	/* rlc (IY + d), A */

				case 0x08: RRC_M_R(IY + EQ, B); break;	/* rrc (IY + d), B */
				case 0x09: RRC_M_R(IY + EQ, C); break;	/* rrc (IY + d), C */
				case 0x0a: RRC_M_R(IY + EQ, D); break;	/* rrc (IY + d), D */
				case 0x0b: RRC_M_R(IY + EQ, E); break;	/* rrc (IY + d), E */
				case 0x0c: RRC_M_R(IY + EQ, H); break;	/* rrc (IY + d), H */
				case 0x0d: RRC_M_R(IY + EQ, L); break;	/* rrc (IY + d), L */
				case 0x0e: RRC_M(IY + EQ);      break;	/* rrc (IY + d) */
				case 0x0f: RRC_M_R(IY + EQ, A); break;	/* rrc (IY + d), A */

				case 0x10: RL_M_R(IY + EQ, B); break;	/* rl (IY + d), B */
				case 0x11: RL_M_R(IY + EQ, C); break;	/* rl (IY + d), C */
				case 0x12: RL_M_R(IY + EQ, D); break;	/* rl (IY + d), D */
				case 0x13: RL_M_R(IY + EQ, E); break;	/* rl (IY + d), E */
				case 0x14: RL_M_R(IY + EQ, H); break;	/* rl (IY + d), H */
				case 0x15: RL_M_R(IY + EQ, L); break;	/* rl (IY + d), L */
				case 0x16: RL_M(IY + EQ);      break;	/* rl (IY + d) */
				case 0x17: RL_M_R(IY + EQ, A); break;	/* rl (IY + d), A */

				case 0x18: RR_M_R(IY + EQ, B); break;	/* rr (IY + d), B */
				case 0x19: RR_M_R(IY + EQ, C); break;	/* rr (IY + d), C */
				case 0x1a: RR_M_R(IY + EQ, D); break;	/* rr (IY + d), D */
				case 0x1b: RR_M_R(IY + EQ, E); break;	/* rr (IY + d), E */
				case 0x1c: RR_M_R(IY + EQ, H); break;	/* rr (IY + d), H */
				case 0x1d: RR_M_R(IY + EQ, L); break;	/* rr (IY + d), L */
				case 0x1e: RR_M(IY + EQ);      break;	/* rr (IY + d) */
				case 0x1f: RR_M_R(IY + EQ, A); break;	/* rr (IY + d), A */

				case 0x20: SLA_M_R(IY + EQ, B); break;	/* sla (IY + d), B */
				case 0x21: SLA_M_R(IY + EQ, C); break;	/* sla (IY + d), C */
				case 0x22: SLA_M_R(IY + EQ, D); break;	/* sla (IY + d), D */
				case 0x23: SLA_M_R(IY + EQ, E); break;	/* sla (IY + d), E */
				case 0x24: SLA_M_R(IY + EQ, H); break;	/* sla (IY + d), H */
				case 0x25: SLA_M_R(IY + EQ, L); break;	/* sla (IY + d), L */
				case 0x26: SLA_M(IY + EQ);      break;	/* sla (IY + d) */
				case 0x27: SLA_M_R(IY + EQ, A); break;	/* sla (IY + d), A */

				case 0x28: SRA_M_R(IY + EQ, B); break;	/* sra (IY + d), B */
				case 0x29: SRA_M_R(IY + EQ, C); break;	/* sra (IY + d), C */
				case 0x2a: SRA_M_R(IY + EQ, D); break;	/* sra (IY + d), D */
				case 0x2b: SRA_M_R(IY + EQ, E); break;	/* sra (IY + d), E */
				case 0x2c: SRA_M_R(IY + EQ, H); break;	/* sra (IY + d), H */
				case 0x2d: SRA_M_R(IY + EQ, L); break;	/* sra (IY + d), L */
				case 0x2e: SRA_M(IY + EQ);      break;	/* sra (IY + d) */
				case 0x2f: SRA_M_R(IY + EQ, A); break;	/* sra (IY + d), A */

				case 0x30: SLL_M_R(IY + EQ, B); break;	/* sll (IY + d), B */
				case 0x31: SLL_M_R(IY + EQ, C); break;	/* sll (IY + d), C */
				case 0x32: SLL_M_R(IY + EQ, D); break;	/* sll (IY + d), D */
				case 0x33: SLL_M_R(IY + EQ, E); break;	/* sll (IY + d), E */
				case 0x34: SLL_M_R(IY + EQ, H); break;	/* sll (IY + d), H */
				case 0x35: SLL_M_R(IY + EQ, L); break;	/* sll (IY + d), L */
				case 0x36: SLL_M(IY + EQ);      break;	/* sll (IY + d) */
				case 0x37: SLL_M_R(IY + EQ, A); break;	/* sll (IY + d), A */

				case 0x38: SRL_M_R(IY + EQ, B); break;	/* srl (IY + d), B */
				case 0x39: SRL_M_R(IY + EQ, C); break;	/* srl (IY + d), C */
				case 0x3a: SRL_M_R(IY + EQ, D); break;	/* srl (IY + d), D */
				case 0x3b: SRL_M_R(IY + EQ, E); break;	/* srl (IY + d), E */
				case 0x3c: SRL_M_R(IY + EQ, H); break;	/* srl (IY + d), H */
				case 0x3d: SRL_M_R(IY + EQ, L); break;	/* srl (IY + d), L */
				case 0x3e: SRL_M(IY + EQ);      break;	/* srl (IY + d) */
				case 0x3f: SRL_M_R(IY + EQ, A); break;	/* srl (IY + d), A */

				case 0x40:
				case 0x41:
				case 0x42:
				case 0x43:
				case 0x44:
				case 0x45:
				case 0x46:
				case 0x47: BIT(0, MEM8(IY + EQ)); break;	/* bit 0, (IY + d) */

				case 0x48:
				case 0x49:
				case 0x4a:
				case 0x4b:
				case 0x4c:
				case 0x4d:
				case 0x4e:
				case 0x4f: BIT(1, MEM8(IY + EQ)); break;	/* bit 1, (IY + d) */

				case 0x50:
				case 0x51:
				case 0x52:
				case 0x53:
				case 0x54:
				case 0x55:
				case 0x56:
				case 0x57: BIT(2, MEM8(IY + EQ)); break;	/* bit 2, (IY + d) */

				case 0x58:
				case 0x59:
				case 0x5a:
				case 0x5b:
				case 0x5c:
				case 0x5d:
				case 0x5e:
				case 0x5f: BIT(3, MEM8(IY + EQ)); break;	/* bit 3, (IY + d) */

				case 0x60:
				case 0x61:
				case 0x62:
				case 0x63:
				case 0x64:
				case 0x65:
				case 0x66:
				case 0x67: BIT(4, MEM8(IY + EQ)); break;	/* bit 4, (IY + d) */

				case 0x68:
				case 0x69:
				case 0x6a:
				case 0x6b:
				case 0x6c:
				case 0x6d:
				case 0x6e:
				case 0x6f: BIT(5, MEM8(IY + EQ)); break;	/* bit 5, (IY + d) */

				case 0x70:
				case 0x71:
				case 0x72:
				case 0x73:
				case 0x74:
				case 0x75:
				case 0x76:
				case 0x77: BIT(6, MEM8(IY + EQ)); break;	/* bit 6, (IY + d) */

				case 0x78:
				case 0x79:
				case 0x7a:
				case 0x7b:
				case 0x7c:
				case 0x7d:
				case 0x7e:
				case 0x7f: BIT(7, MEM8(IY + EQ)); break;	/* bit 7, (IY + d) */

				case 0x80: RES_M_R(0, IY + EQ, B); break;	/* res 0, (IY + d), B */
				case 0x81: RES_M_R(0, IY + EQ, C); break;	/* res 0, (IY + d), C */
				case 0x82: RES_M_R(0, IY + EQ, D); break;	/* res 0, (IY + d), D */
				case 0x83: RES_M_R(0, IY + EQ, E); break;	/* res 0, (IY + d), E */
				case 0x84: RES_M_R(0, IY + EQ, H); break;	/* res 0, (IY + d), H */
				case 0x85: RES_M_R(0, IY + EQ, L); break;	/* res 0, (IY + d), L */
				case 0x86: RES_M(0, IY + EQ);      break;	/* res 0, (IY + d) */
				case 0x87: RES_M_R(0, IY + EQ, A); break;	/* res 0, (IY + d), A */

				case 0x88: RES_M_R(1, IY + EQ, B); break;	/* res 1, (IY + d), B */
				case 0x89: RES_M_R(1, IY + EQ, C); break;	/* res 1, (IY + d), C */
				case 0x8a: RES_M_R(1, IY + EQ, D); break;	/* res 1, (IY + d), D */
				case 0x8b: RES_M_R(1, IY + EQ, E); break;	/* res 1, (IY + d), E */
				case 0x8c: RES_M_R(1, IY + EQ, H); break;	/* res 1, (IY + d), H */
				case 0x8d: RES_M_R(1, IY + EQ, L); break;	/* res 1, (IY + d), L */
				case 0x8e: RES_M(1, IY + EQ);      break;	/* res 1, (IY + d) */
				case 0x8f: RES_M_R(1, IY + EQ, A); break;	/* res 1, (IY + d), A */

				case 0x90: RES_M_R(2, IY + EQ, B); break;	/* res 2, (IY + d), B */
				case 0x91: RES_M_R(2, IY + EQ, C); break;	/* res 2, (IY + d), C */
				case 0x92: RES_M_R(2, IY + EQ, D); break;	/* res 2, (IY + d), D */
				case 0x93: RES_M_R(2, IY + EQ, E); break;	/* res 2, (IY + d), E */
				case 0x94: RES_M_R(2, IY + EQ, H); break;	/* res 2, (IY + d), H */
				case 0x95: RES_M_R(2, IY + EQ, L); break;	/* res 2, (IY + d), L */
				case 0x96: RES_M(2, IY + EQ);      break;	/* res 2, (IY + d) */
				case 0x97: RES_M_R(2, IY + EQ, A); break;	/* res 2, (IY + d), A */

				case 0x98: RES_M_R(3, IY + EQ, B); break;	/* res 3, (IY + d), B */
				case 0x99: RES_M_R(3, IY + EQ, C); break;	/* res 3, (IY + d), C */
				case 0x9a: RES_M_R(3, IY + EQ, D); break;	/* res 3, (IY + d), D */
				case 0x9b: RES_M_R(3, IY + EQ, E); break;	/* res 3, (IY + d), E */
				case 0x9c: RES_M_R(3, IY + EQ, H); break;	/* res 3, (IY + d), H */
				case 0x9d: RES_M_R(3, IY + EQ, L); break;	/* res 3, (IY + d), L */
				case 0x9e: RES_M(3, IY + EQ);      break;	/* res 3, (IY + d) */
				case 0x9f: RES_M_R(3, IY + EQ, A); break;	/* res 3, (IY + d), A */

				case 0xa0: RES_M_R(4, IY + EQ, B); break;	/* res 4, (IY + d), B */
				case 0xa1: RES_M_R(4, IY + EQ, C); break;	/* res 4, (IY + d), C */
				case 0xa2: RES_M_R(4, IY + EQ, D); break;	/* res 4, (IY + d), D */
				case 0xa3: RES_M_R(4, IY + EQ, E); break;	/* res 4, (IY + d), E */
				case 0xa4: RES_M_R(4, IY + EQ, H); break;	/* res 4, (IY + d), H */
				case 0xa5: RES_M_R(4, IY + EQ, L); break;	/* res 4, (IY + d), L */
				case 0xa6: RES_M(4, IY + EQ);      break;	/* res 4, (IY + d) */
				case 0xa7: RES_M_R(4, IY + EQ, A); break;	/* res 4, (IY + d), A */

				case 0xa8: RES_M_R(5, IY + EQ, B); break;	/* res 5, (IY + d), B */
				case 0xa9: RES_M_R(5, IY + EQ, C); break;	/* res 5, (IY + d), C */
				case 0xaa: RES_M_R(5, IY + EQ, D); break;	/* res 5, (IY + d), D */
				case 0xab: RES_M_R(5, IY + EQ, E); break;	/* res 5, (IY + d), E */
				case 0xac: RES_M_R(5, IY + EQ, H); break;	/* res 5, (IY + d), H */
				case 0xad: RES_M_R(5, IY + EQ, L); break;	/* res 5, (IY + d), L */
				case 0xae: RES_M(5, IY + EQ);      break;	/* res 5, (IY + d) */
				case 0xaf: RES_M_R(5, IY + EQ, A); break;	/* res 5, (IY + d), A */

				case 0xb0: RES_M_R(6, IY + EQ, B); break;	/* res 6, (IY + d), B */
				case 0xb1: RES_M_R(6, IY + EQ, C); break;	/* res 6, (IY + d), C */
				case 0xb2: RES_M_R(6, IY + EQ, D); break;	/* res 6, (IY + d), D */
				case 0xb3: RES_M_R(6, IY + EQ, E); break;	/* res 6, (IY + d), E */
				case 0xb4: RES_M_R(6, IY + EQ, H); break;	/* res 6, (IY + d), H */
				case 0xb5: RES_M_R(6, IY + EQ, L); break;	/* res 6, (IY + d), L */
				case 0xb6: RES_M(6, IY + EQ);      break;	/* res 6, (IY + d) */
				case 0xb7: RES_M_R(6, IY + EQ, A); break;	/* res 6, (IY + d), A */

				case 0xb8: RES_M_R(7, IY + EQ, B); break;	/* res 7, (IY + d), B */
				case 0xb9: RES_M_R(7, IY + EQ, C); break;	/* res 7, (IY + d), C */
				case 0xba: RES_M_R(7, IY + EQ, D); break;	/* res 7, (IY + d), D */
				case 0xbb: RES_M_R(7, IY + EQ, E); break;	/* res 7, (IY + d), E */
				case 0xbc: RES_M_R(7, IY + EQ, H); break;	/* res 7, (IY + d), H */
				case 0xbd: RES_M_R(7, IY + EQ, L); break;	/* res 7, (IY + d), L */
				case 0xbe: RES_M(7, IY + EQ);      break;	/* res 7, (IY + d) */
				case 0xbf: RES_M_R(7, IY + EQ, A); break;	/* res 7, (IY + d), A */

				case 0xc0: SET_M_R(0, IY + EQ, B); break;	/* set 0, (IY + d), B */
				case 0xc1: SET_M_R(0, IY + EQ, C); break;	/* set 0, (IY + d), C */
				case 0xc2: SET_M_R(0, IY + EQ, D); break;	/* set 0, (IY + d), D */
				case 0xc3: SET_M_R(0, IY + EQ, E); break;	/* set 0, (IY + d), E */
				case 0xc4: SET_M_R(0, IY + EQ, H); break;	/* set 0, (IY + d), H */
				case 0xc5: SET_M_R(0, IY + EQ, L); break;	/* set 0, (IY + d), L */
				case 0xc6: SET_M(0, IY + EQ);      break;	/* set 0, (IY + d) */
				case 0xc7: SET_M_R(0, IY + EQ, A); break;	/* set 0, (IY + d), A */

				case 0xc8: SET_M_R(1, IY + EQ, B); break;	/* set 1, (IY + d), B */
				case 0xc9: SET_M_R(1, IY + EQ, C); break;	/* set 1, (IY + d), C */
				case 0xca: SET_M_R(1, IY + EQ, D); break;	/* set 1, (IY + d), D */
				case 0xcb: SET_M_R(1, IY + EQ, E); break;	/* set 1, (IY + d), E */
				case 0xcc: SET_M_R(1, IY + EQ, H); break;	/* set 1, (IY + d), H */
				case 0xcd: SET_M_R(1, IY + EQ, L); break;	/* set 1, (IY + d), L */
				case 0xce: SET_M(1, IY + EQ);      break;	/* set 1, (IY + d) */
				case 0xcf: SET_M_R(1, IY + EQ, A); break;	/* set 1, (IY + d), A */

				case 0xd0: SET_M_R(2, IY + EQ, B); break;	/* set 2, (IY + d), B */
				case 0xd1: SET_M_R(2, IY + EQ, C); break;	/* set 2, (IY + d), C */
				case 0xd2: SET_M_R(2, IY + EQ, D); break;	/* set 2, (IY + d), D */
				case 0xd3: SET_M_R(2, IY + EQ, E); break;	/* set 2, (IY + d), E */
				case 0xd4: SET_M_R(2, IY + EQ, H); break;	/* set 2, (IY + d), H */
				case 0xd5: SET_M_R(2, IY + EQ, L); break;	/* set 2, (IY + d), L */
				case 0xd6: SET_M(2, IY + EQ);      break;	/* set 2, (IY + d) */
				case 0xd7: SET_M_R(2, IY + EQ, A); break;	/* set 2, (IY + d), A */

				case 0xd8: SET_M_R(3, IY + EQ, B); break;	/* set 3, (IY + d), B */
				case 0xd9: SET_M_R(3, IY + EQ, C); break;	/* set 3, (IY + d), C */
				case 0xda: SET_M_R(3, IY + EQ, D); break;	/* set 3, (IY + d), D */
				case 0xdb: SET_M_R(3, IY + EQ, E); break;	/* set 3, (IY + d), E */
				case 0xdc: SET_M_R(3, IY + EQ, H); break;	/* set 3, (IY + d), H */
				case 0xdd: SET_M_R(3, IY + EQ, L); break;	/* set 3, (IY + d), L */
				case 0xde: SET_M(3, IY + EQ);      break;	/* set 3, (IY + d) */
				case 0xdf: SET_M_R(3, IY + EQ, A); break;	/* set 3, (IY + d), A */

				case 0xe0: SET_M_R(4, IY + EQ, B); break;	/* set 4, (IY + d), B */
				case 0xe1: SET_M_R(4, IY + EQ, C); break;	/* set 4, (IY + d), C */
				case 0xe2: SET_M_R(4, IY + EQ, D); break;	/* set 4, (IY + d), D */
				case 0xe3: SET_M_R(4, IY + EQ, E); break;	/* set 4, (IY + d), E */
				case 0xe4: SET_M_R(4, IY + EQ, H); break;	/* set 4, (IY + d), H */
				case 0xe5: SET_M_R(4, IY + EQ, L); break;	/* set 4, (IY + d), L */
				case 0xe6: SET_M(4, IY + EQ);      break;	/* set 4, (IY + d) */
				case 0xe7: SET_M_R(4, IY + EQ, A); break;	/* set 4, (IY + d), A */

				case 0xe8: SET_M_R(5, IY + EQ, B); break;	/* set 5, (IY + d), B */
				case 0xe9: SET_M_R(5, IY + EQ, C); break;	/* set 5, (IY + d), C */
				case 0xea: SET_M_R(5, IY + EQ, D); break;	/* set 5, (IY + d), D */
				case 0xeb: SET_M_R(5, IY + EQ, E); break;	/* set 5, (IY + d), E */
				case 0xec: SET_M_R(5, IY + EQ, H); break;	/* set 5, (IY + d), H */
				case 0xed: SET_M_R(5, IY + EQ, L); break;	/* set 5, (IY + d), L */
				case 0xee: SET_M(5, IY + EQ);      break;	/* set 5, (IY + d) */
				case 0xef: SET_M_R(5, IY + EQ, A); break;	/* set 5, (IY + d), A */

				case 0xf0: SET_M_R(6, IY + EQ, B); break;	/* set 6, (IY + d), B */
				case 0xf1: SET_M_R(6, IY + EQ, C); break;	/* set 6, (IY + d), C */
				case 0xf2: SET_M_R(6, IY + EQ, D); break;	/* set 6, (IY + d), D */
				case 0xf3: SET_M_R(6, IY + EQ, E); break;	/* set 6, (IY + d), E */
				case 0xf4: SET_M_R(6, IY + EQ, H); break;	/* set 6, (IY + d), H */
				case 0xf5: SET_M_R(6, IY + EQ, L); break;	/* set 6, (IY + d), L */
				case 0xf6: SET_M(6, IY + EQ);      break;	/* set 6, (IY + d) */
				case 0xf7: SET_M_R(6, IY + EQ, A); break;	/* set 6, (IY + d), A */

				case 0xf8: SET_M_R(7, IY + EQ, B); break;	/* set 7, (IY + d), B */
				case 0xf9: SET_M_R(7, IY + EQ, C); break;	/* set 7, (IY + d), C */
				case 0xfa: SET_M_R(7, IY + EQ, D); break;	/* set 7, (IY + d), D */
				case 0xfb: SET_M_R(7, IY + EQ, E); break;	/* set 7, (IY + d), E */
				case 0xfc: SET_M_R(7, IY + EQ, H); break;	/* set 7, (IY + d), H */
				case 0xfd: SET_M_R(7, IY + EQ, L); break;	/* set 7, (IY + d), L */
				case 0xfe: SET_M(7, IY + EQ);      break;	/* set 7, (IY + d) */
				case 0xff: SET_M_R(7, IY + EQ, A); break;	/* set 7, (IY + d), A */
				}
				break;
			case 0xe1: POP(IY);      break;	/* pop IY */
			case 0xe3: EX_M(SP, IY); break;	/* ex (SP), IY */
			case 0xe5: PUSH(IY);     break;	/* push IY */

            case 0xe9: JP(true, IY); break;	/* jp (IY) */

			case 0xf9: LD(SP, IY); break;	/* ld SP, IY */
			}
			break;
		case 0xfe: CP(IMM8);          break;	/* cp n */
		case 0xff: RST(0x38);         break;	/* rst 38H */
		}

		z->i.states -= _state;
        pPC->pTIMER->state += _state;
        //if (fp_log) fprintf(fp_log,"(States = %i,%i, %i)\n",z->i.states,pPC->pTIMER->state,_state);
	} while(z->i.states > 0);
	
	z->i.total_states -= z->i.states;
	return Z80_RUN;
}

inline uint8 CZ80::z80read8(const Z80stat *z, uint16 address)
{
    return (((CpcXXXX *)pPC)->Get_8(address));
}
inline void CZ80::z80write8(const Z80stat *z, uint16 address, uint8 value)
{
    ((CpcXXXX *)pPC)->Set_8(address,value);
}


//#ifdef Z80_LITTLEENDIAN

/*
    16bits READ/WRITE (���g���G���f�B�A��)
*/
inline uint16 CZ80::z80read16(const Z80stat *z, uint16 address)
{
    return ((CpcXXXX *)pPC)->Get_16(address);

}
inline void CZ80::z80write16(const Z80stat *z, uint16 address, uint16 value)
{
    ((CpcXXXX *)pPC)->Set_16(address,value);
}

#if 0

/*
    16bits READ/WRITE (�G���f�B�A�����ˑ�)
*/
inline uint16 CZ80::z80read16(const Z80stat *z, uint16 address)
{
    return 0;//((uint16 )memory[(address) + 1] << 8U | memory[address]);
}
inline void CZ80::z80write16(const Z80stat *z, uint16 address, uint16 value)
{
//    if(address < 0x7fff) {
//        memory[address    ] = value & 0xff;
//        memory[address + 1] = value >> 8;
//    }
}
#endif

bool	CZ80::init(void)
{

    memset(imem,0,imemsize);
    Check_Log();
    pDEBUG->init();
    Reset();
    return true;
}

bool	CZ80::exit(void)
{
    return true;
}

void	CZ80::step(void)
{
    int ret = z80exec(&z80);

//    if (ret == Z80_UNDERFLOW) {
//        if (fp_log) fprintf(fp_log,"\nUNDERFLOW\n\n");
//    }

    //if (fp_log) fprintf(fp_log,"IFF=%i\n",z80.r.iff);
    if ( (z80.r.iff==3) && (imem[0x32] & imem[0x35]) )
    {
        z80int2(&z80,imem[0x39]);
    }
}

void	CZ80::Reset(void)
{
    z80reset(&z80);
}

void	CZ80::Load_Internal(QFile *file){
    char t[16];
    QDataStream in(file);

    in.readRawData(t, 6);
    in.readRawData((char*)&z80,sizeof(z80));
    in.readRawData( (char *)imem,IMEM_LEN);	// Write Header
}

void	CZ80::save_internal(QFile *file){
    QDataStream out(file);

    out.writeRawData("Z80STA", 6);					//header
    out.writeRawData((char*)&z80,sizeof(z80));		//reg
    out.writeRawData((char*)imem,IMEM_LEN);			//i-mem
}

void CZ80::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "z80")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &z80,ba_reg.data(),sizeof(z80));
            QByteArray ba_imem = QByteArray::fromBase64(xmlIn->attributes().value("iMem").toString().toLatin1());
            memcpy((char *) &imem,ba_imem.data(),IMEM_LEN);
        }
        xmlIn->skipCurrentElement();
    }
}

void CZ80::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","z80");
        QByteArray ba_reg((char*)&z80,sizeof(z80));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
        QByteArray ba_imem((char*)imem,IMEM_LEN);
        xmlOut->writeAttribute("iMem",ba_imem.toBase64());
    xmlOut->writeEndElement();
}

INLINE UINT32	CZ80::get_mem(UINT32 adr,int size)
{
    switch(size)
    {
    case SIZE_8 :return( z80read8(&z80,adr));
    case SIZE_16:return( z80read8(&z80,adr+1)+(z80read8(&z80,adr)<<8));
    case SIZE_20:return((z80read8(&z80,adr+2)+(z80read8(&z80,adr+1)<<8)+(z80read8(&z80,adr)<<16))&MASK_20);
    case SIZE_24:return((z80read8(&z80,adr+2)+(z80read8(&z80,adr+1)<<8)+(z80read8(&z80,adr)<<16))&MASK_24);
    }
    return(0);
}

INLINE void	CZ80::set_mem(UINT32 adr,int size,UINT32 data)
{
    switch(size)
    {
    case SIZE_8 :
        z80write8(&z80, adr, (BYTE) data);
        break;
    case SIZE_16:
        z80write8(&z80, adr, (BYTE) (data>>8));
        z80write8(&z80, adr+1, (BYTE) data);
        //pPC->mem[adr]   = (BYTE) (data>>8);
        //pPC->mem[adr+1] = (BYTE) data;
        break;
    //case SIZE_20:
        //pPC->mem[adr]   = (BYTE) data;
        //pPC->mem[adr+1] = (BYTE) (data>>8);
        //pPC->mem[adr+2] = (BYTE) ((data>>16)&MASK_4);
        //break;
    //case SIZE_24:
        //pPC->mem[adr]   = (BYTE) data;
        //pPC->mem[adr+1] = (BYTE) (data>>8);
        //pPC->mem[adr+2] = (BYTE) (data>>16);
        //break;
    }
}	//set memory

int CZ80::z80inport(Z80stat *z, uint8 *x, uint8 address)
{
    //fprintf(fp_log,"IN:%04X\n",address);
    pPC->in(address);
    *x = imem[address];

    return 0;
}

int CZ80::z80outport(Z80stat *z, uint8 address, uint8 x)
{
    //fprintf(fp_log,"OUT:%04Xh = %02Xh\n",address,x);
    imem[address] = x;
    pPC->out(address,x);
    return 0;
}


bool	CZ80::Get_Xin(void){return false;}
void	CZ80::Set_Xin(bool){}
bool	CZ80::Get_Xout(void){return false;}
void	CZ80::Set_Xout(bool){}

UINT32 CZ80::get_PC(void){ return z80.r16.pc;}					//get Program Counter

void	CZ80::Regs_Info(UINT8)
{
    sprintf(Regs_String,"EMPTY");

#if 1
    char buf[32];

    sprintf(
    Regs_String,
    "[31]=%02x,[32]=%02x,[33]=%02x,[35]=%02x %c%c%c%c%c%c(%02x) A=%02x BC=%04x DE=%04x HL=%04x SP=%04x PC=%04x IFF=%i "
    "%c%c%c%c%c%c(%02x) A'%02x BC'%04x DE'%04x HL'%04x IX=%04x IY=%04x %s ",
    imem[0x31],imem[0x32],imem[0x33],imem[0x35],
    (z80.r.f & 0x80 ? 'S': '-'),
    (z80.r.f & 0x40 ? 'Z': '-'),
    (z80.r.f & 0x10 ? 'H': '-'),
    (z80.r.f & 0x04 ? 'P': '-'),
    (z80.r.f & 0x02 ? 'N': '-'),
    (z80.r.f & 0x01 ? 'C': '-'),
    z80.r.f,
    z80.r.a,
    z80.r16.bc,
    z80.r16.de,
    z80.r16.hl,
    z80.r16.sp,
    z80.r16.pc,
    z80.r.iff,
//    (z80disasm(buf, &r->m[r->r16.pc]), buf),
    (z80.r.f_d & 0x80 ? 'S': '-'),
    (z80.r.f_d & 0x40 ? 'Z': '-'),
    (z80.r.f_d & 0x10 ? 'H': '-'),
    (z80.r.f_d & 0x04 ? 'P': '-'),
    (z80.r.f_d & 0x02 ? 'N': '-'),
    (z80.r.f_d & 0x01 ? 'C': '-'),
    z80.r.f_d,
    z80.r.a_d,
    z80.r16.bc_d,
    z80.r16.de_d,
    z80.r16.hl_d,
    z80.r16.ix,
    z80.r16.iy,
    z80.r.halt ? "HALT": ""
    );

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

/*
	Copyright 2005 ~ 2009 maruhiro
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

