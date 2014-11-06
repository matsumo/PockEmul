/*
    Zilog Z80 Emulator Disassembler
*/

//#include <stdio.h>
//#include <string.h>

#include "z80_dasm.h"
#include "z80.h"

#define XX	0
#define CB_XX	1
#define DD_XX	2
#define DD_CB_XX	3
#define ED_XX	4
#define FD_XX	5
#define FD_CB_XX	6

#define SHIFT_NIM	0
#define SHIFT_LOP	7
#define SHIFT_ROP	13
#define SHIFT_LPARAM	19
#define SHIFT_RPARAM	21
#define SHIFT_OP	23
#define SHIFT_PARAM	25
#define MASK_NIM	0x7f
#define MASK_LOP	(0x3f << SHIFT_LOP)
#define MASK_ROP	(0x3f << SHIFT_ROP)
#define MASK_LPARAM	(0x3 << SHIFT_LPARAM)
#define MASK_RPARAM	(0x3 << SHIFT_RPARAM)
#define MASK_PARAM	(0x3 << SHIFT_PARAM)

#define NIM_ADC	0x00
#define NIM_ADD	0x01
#define NIM_AND	0x02
#define NIM_BIT0	0x03
#define NIM_BIT1	0x04
#define NIM_BIT2	0x05
#define NIM_BIT3	0x06
#define NIM_BIT4	0x07
#define NIM_BIT5	0x08
#define NIM_BIT6	0x09
#define NIM_BIT7	0x0a
#define NIM_CALL	0x0b
#define NIM_CCF	0x0c
#define NIM_CP	0x0d
#define NIM_CPD	0x0e
#define NIM_CPDR	0x0f
#define NIM_CPI	0x10
#define NIM_CPIR	0x11
#define NIM_CPL	0x12
#define NIM_DAA	0x13
#define NIM_DEC	0x14
#define NIM_DI	0x15
#define NIM_DJNZ	0x16
#define NIM_EI	0x17
#define NIM_HALT	0x18
#define NIM_EX	0x19
#define NIM_EXX	0x1a
#define NIM_IM0	0x1b
#define NIM_IM1	0x1c
#define NIM_IM2	0x1d
#define NIM_IN	0x1e
#define NIM_IND	0x1f
#define NIM_INDR	0x20
#define NIM_INI	0x21
#define NIM_INIR	0x22
#define NIM_INC	0x23
#define NIM_JP	0x24
#define NIM_JR	0x25
#define NIM_LD	0x26
#define NIM_LDD	0x27
#define NIM_LDDR	0x28
#define NIM_LDI	0x29
#define NIM_LDIR	0x2a
#define NIM_NEG	0x2b
#define NIM_NOP	0x2c
#define NIM_OR	0x2d
#define NIM_OUT	0x2e
#define NIM_OUTD	0x2f
#define NIM_OTDR	0x30
#define NIM_OUTI	0x31
#define NIM_OTIR	0x32
#define NIM_POP	0x33
#define NIM_PUSH	0x34
#define NIM_RES0	0x35
#define NIM_RES1	0x36
#define NIM_RES2	0x37
#define NIM_RES3	0x38
#define NIM_RES4	0x39
#define NIM_RES5	0x3a
#define NIM_RES6	0x3b
#define NIM_RES7	0x3c
#define NIM_RET	0x3d
#define NIM_RETI	0x3e
#define NIM_RETN	0x3f
#define NIM_RL	0x40
#define NIM_RLA	0x41
#define NIM_RLC	0x42
#define NIM_RLCA	0x43
#define NIM_RLD	0x44
#define NIM_RR	0x45
#define NIM_RRA	0x46
#define NIM_RRC	0x47
#define NIM_RRCA	0x48
#define NIM_RRD	0x49
#define NIM_RST00	0x4a
#define NIM_RST08	0x4b
#define NIM_RST10	0x4c
#define NIM_RST18	0x4d
#define NIM_RST20	0x4e
#define NIM_RST28	0x4f
#define NIM_RST30	0x50
#define NIM_RST38	0x51
#define NIM_SBC	0x52
#define NIM_SCF	0x53
#define NIM_SET0	0x54
#define NIM_SET1	0x55
#define NIM_SET2	0x56
#define NIM_SET3	0x57
#define NIM_SET4	0x58
#define NIM_SET5	0x59
#define NIM_SET6	0x5a
#define NIM_SET7	0x5b
#define NIM_SLA	0x5c
#define NIM_SLL	0x5d
#define NIM_SRA	0x5e
#define NIM_SRL	0x5f
#define NIM_SUB	0x60
#define NIM_XOR	0x61
#define NIM_UNKNOWN	0x62

#define LPARAM_UINT8	(0x1 << SHIFT_LPARAM)
#define LPARAM_INT8	(0x2 << SHIFT_LPARAM)
#define LPARAM_UINT16	(0x3 << SHIFT_LPARAM)
#define RPARAM_UINT8	(0x1 << SHIFT_RPARAM)
#define RPARAM_INT8	(0x2 << SHIFT_RPARAM)
#define RPARAM_UINT16	(0x3 << SHIFT_RPARAM)
#define PARAM_INT8	(0x1 << SHIFT_PARAM)

#define LOP_B	(0x01 << SHIFT_LOP)
#define LOP_C	(0x02 << SHIFT_LOP)
#define LOP_D	(0x03 << SHIFT_LOP)
#define LOP_E	(0x04 << SHIFT_LOP)
#define LOP_H	(0x05 << SHIFT_LOP)
#define LOP_L	(0x06 << SHIFT_LOP)
#define LOP_A	(0x07 << SHIFT_LOP)
#define LOP_F	(0x08 << SHIFT_LOP)
#define LOP_IXH	(0x09 << SHIFT_LOP)
#define LOP_IXL	(0x0a << SHIFT_LOP)
#define LOP_IYH	(0x0b << SHIFT_LOP)
#define LOP_IYL	(0x0c << SHIFT_LOP)
#define LOP_BC	(0x0d << SHIFT_LOP)
#define LOP_DE	(0x0e << SHIFT_LOP)
#define LOP_HL	(0x0f << SHIFT_LOP)
#define LOP_AF	(0x10 << SHIFT_LOP)
#define LOP_IX	(0x11 << SHIFT_LOP)
#define LOP_IY	(0x12 << SHIFT_LOP)
#define LOP_SP	(0x13 << SHIFT_LOP)
#define LOP_PC	(0x14 << SHIFT_LOP)
#define LOP_BC_D	(0x15 << SHIFT_LOP)
#define LOP_DE_D	(0x16 << SHIFT_LOP)
#define LOP_HL_D	(0x17 << SHIFT_LOP)
#define LOP_AF_D	(0x18 << SHIFT_LOP)
#define LOP_IMM8	(0x19 << SHIFT_LOP | LPARAM_UINT8)
#define LOP_IMM16	(0x1a << SHIFT_LOP | LPARAM_UINT16)
#define LOP_EQ	(0x1b << SHIFT_LOP | LPARAM_INT8)
#define LOP_REF_BC	(0x1c << SHIFT_LOP)
#define LOP_REF_DE	(0x1d << SHIFT_LOP)
#define LOP_REF_HL	(0x1e << SHIFT_LOP)
#define LOP_REF_SP	(0x1f << SHIFT_LOP)
#define LOP_REF_8	(0x20 << SHIFT_LOP | LPARAM_UINT8)
#define LOP_REF_16	(0x21 << SHIFT_LOP | LPARAM_UINT16)
#define LOP_REF_IX	(0x22 << SHIFT_LOP)
#define LOP_REF_IXD	(0x23 << SHIFT_LOP | LPARAM_INT8)
#define LOP_REF_IY	(0x24 << SHIFT_LOP)
#define LOP_REF_IYD	(0x25 << SHIFT_LOP | LPARAM_INT8)
#define LOP_REF_C	(0x26 << SHIFT_LOP)
#define LOP_I	(0x27 << SHIFT_LOP)
#define LOP_R	(0x28 << SHIFT_LOP)
#define CC_NZ	(0x29 << SHIFT_LOP)
#define CC_Z	(0x2a << SHIFT_LOP)
#define CC_NC	(0x2b << SHIFT_LOP)
#define CC_C	(0x2c << SHIFT_LOP)
#define CC_PO	(0x2d << SHIFT_LOP)
#define CC_PE	(0x2e << SHIFT_LOP)
#define CC_P	(0x2f << SHIFT_LOP)
#define CC_M	(0x30 << SHIFT_LOP)

#define ROP_B	(0x01 << SHIFT_ROP)
#define ROP_C	(0x02 << SHIFT_ROP)
#define ROP_D	(0x03 << SHIFT_ROP)
#define ROP_E	(0x04 << SHIFT_ROP)
#define ROP_H	(0x05 << SHIFT_ROP)
#define ROP_L	(0x06 << SHIFT_ROP)
#define ROP_A	(0x07 << SHIFT_ROP)
#define ROP_F	(0x08 << SHIFT_ROP)
#define ROP_IXH	(0x09 << SHIFT_ROP)
#define ROP_IXL	(0x0a << SHIFT_ROP)
#define ROP_IYH	(0x0b << SHIFT_ROP)
#define ROP_IYL	(0x0c << SHIFT_ROP)
#define ROP_BC	(0x0d << SHIFT_ROP)
#define ROP_DE	(0x0e << SHIFT_ROP)
#define ROP_HL	(0x0f << SHIFT_ROP)
#define ROP_AF	(0x10 << SHIFT_ROP)
#define ROP_IX	(0x11 << SHIFT_ROP)
#define ROP_IY	(0x12 << SHIFT_ROP)
#define ROP_SP	(0x13 << SHIFT_ROP)
#define ROP_PC	(0x14 << SHIFT_ROP)
#define ROP_BC_D	(0x15 << SHIFT_ROP)
#define ROP_DE_D	(0x16 << SHIFT_ROP)
#define ROP_HL_D	(0x17 << SHIFT_ROP)
#define ROP_AF_D	(0x18 << SHIFT_ROP)
#define ROP_IMM8	(0x19 << SHIFT_ROP | RPARAM_UINT8)
#define ROP_IMM16	(0x1a << SHIFT_ROP | RPARAM_UINT16)
#define ROP_EQ	(0x1b << SHIFT_ROP | RPARAM_INT8)
#define ROP_REF_BC	(0x1c << SHIFT_ROP)
#define ROP_REF_DE	(0x1d << SHIFT_ROP)
#define ROP_REF_HL	(0x1e << SHIFT_ROP)
#define ROP_REF_SP	(0x1f << SHIFT_LOP)
#define ROP_REF_8	(0x20 << SHIFT_ROP | RPARAM_UINT8)
#define ROP_REF_16	(0x21 << SHIFT_ROP | RPARAM_UINT16)
#define ROP_REF_IX	(0x22 << SHIFT_ROP)
#define ROP_REF_IXD	(0x23 << SHIFT_ROP | RPARAM_INT8)
#define ROP_REF_IY	(0x24 << SHIFT_ROP)
#define ROP_REF_IYD	(0x25 << SHIFT_ROP | RPARAM_INT8)
#define ROP_REF_C	(0x26 << SHIFT_ROP)
#define ROP_I	(0x27 << SHIFT_ROP)
#define ROP_R	(0x28 << SHIFT_ROP)

#define OP_REF_IXD	(0x23 << SHIFT_LOP | PARAM_INT8)
#define OP_REF_IYD	(0x25 << SHIFT_LOP | PARAM_INT8)

const static uint32 xx[] = {
    /* 00 */
    NIM_NOP,
    NIM_LD   | LOP_BC      | ROP_IMM16,
    NIM_LD   | LOP_REF_BC  | ROP_A,
    NIM_INC  | LOP_BC,
    NIM_INC  | LOP_B,
    NIM_DEC  | LOP_B,
    NIM_LD   | LOP_B       | ROP_IMM8,
    NIM_RLCA,

    /* 08 */
    NIM_EX   | LOP_AF      | ROP_AF_D,
    NIM_ADD  | LOP_HL      | ROP_BC,
    NIM_LD   | LOP_A       | ROP_REF_BC,
    NIM_DEC  | LOP_BC,
    NIM_INC  | LOP_C,
    NIM_DEC  | LOP_C,
    NIM_LD   | LOP_C       | ROP_IMM8,
    NIM_RRCA,

    /* 10 */
    NIM_DJNZ | LOP_EQ,
    NIM_LD   | LOP_DE      | ROP_IMM16,
    NIM_LD   | LOP_REF_DE  | ROP_A,
    NIM_INC  | LOP_DE,
    NIM_INC  | LOP_D,
    NIM_DEC  | LOP_D,
    NIM_LD   | LOP_D       | ROP_IMM8,
    NIM_RLA,

    /* 18 */
    NIM_JR   | LOP_EQ,
    NIM_ADD  | LOP_HL      | ROP_DE,
    NIM_LD   | LOP_A       | ROP_REF_DE,
    NIM_DEC  | LOP_DE,
    NIM_INC  | LOP_E,
    NIM_DEC  | LOP_E,
    NIM_LD   | LOP_E       | ROP_IMM8,
    NIM_RRA,

    /* 20 */
    NIM_JR   | CC_NZ       | ROP_EQ,
    NIM_LD   | LOP_HL      | ROP_IMM16,
    NIM_LD   | LOP_REF_16  | ROP_HL,
    NIM_INC  | LOP_HL,
    NIM_INC  | LOP_H,
    NIM_DEC  | LOP_H,
    NIM_LD   | LOP_H       | ROP_IMM8,
    NIM_DAA,

    /* 28 */
    NIM_JR   | CC_Z        | ROP_EQ,
    NIM_ADD  | LOP_HL      | ROP_HL,
    NIM_LD   | LOP_HL      | ROP_REF_16,
    NIM_DEC  | LOP_HL,
    NIM_INC  | LOP_L,
    NIM_DEC  | LOP_L,
    NIM_LD   | LOP_L       | ROP_IMM8,
    NIM_CPL,

    /* 30 */
    NIM_JR   | CC_NC       | ROP_EQ,
    NIM_LD   | LOP_SP      | ROP_IMM16,
    NIM_LD   | LOP_REF_16  | ROP_A,
    NIM_INC  | LOP_SP,
    NIM_INC  | LOP_REF_HL,
    NIM_DEC  | LOP_REF_HL,
    NIM_LD   | LOP_REF_HL  | ROP_IMM8,
    NIM_SCF,

    /* 38 */
    NIM_JR   | CC_C        | ROP_EQ,
    NIM_ADD  | LOP_HL      | ROP_SP,
    NIM_LD   | LOP_A       | ROP_REF_16,
    NIM_DEC  | LOP_SP,
    NIM_INC  | LOP_A,
    NIM_DEC  | LOP_A,
    NIM_LD   | LOP_A       | ROP_IMM8,
    NIM_CCF,

    /* 40 */
    NIM_LD   | LOP_B       | ROP_B,
    NIM_LD   | LOP_B       | ROP_C,
    NIM_LD   | LOP_B       | ROP_D,
    NIM_LD   | LOP_B       | ROP_E,
    NIM_LD   | LOP_B       | ROP_H,
    NIM_LD   | LOP_B       | ROP_L,
    NIM_LD   | LOP_B       | ROP_REF_HL,
    NIM_LD   | LOP_B       | ROP_A,

    /* 48 */
    NIM_LD   | LOP_C       | ROP_B,
    NIM_LD   | LOP_C       | ROP_C,
    NIM_LD   | LOP_C       | ROP_D,
    NIM_LD   | LOP_C       | ROP_E,
    NIM_LD   | LOP_C       | ROP_H,
    NIM_LD   | LOP_C       | ROP_L,
    NIM_LD   | LOP_C       | ROP_REF_HL,
    NIM_LD   | LOP_C       | ROP_A,

    /* 50 */
    NIM_LD   | LOP_D       | ROP_B,
    NIM_LD   | LOP_D       | ROP_C,
    NIM_LD   | LOP_D       | ROP_D,
    NIM_LD   | LOP_D       | ROP_E,
    NIM_LD   | LOP_D       | ROP_H,
    NIM_LD   | LOP_D       | ROP_L,
    NIM_LD   | LOP_D       | ROP_REF_HL,
    NIM_LD   | LOP_D       | ROP_A,

    /* 58 */
    NIM_LD   | LOP_E       | ROP_B,
    NIM_LD   | LOP_E       | ROP_C,
    NIM_LD   | LOP_E       | ROP_D,
    NIM_LD   | LOP_E       | ROP_E,
    NIM_LD   | LOP_E       | ROP_H,
    NIM_LD   | LOP_E       | ROP_L,
    NIM_LD   | LOP_E       | ROP_REF_HL,
    NIM_LD   | LOP_E       | ROP_A,

    /* 60 */
    NIM_LD   | LOP_H       | ROP_B,
    NIM_LD   | LOP_H       | ROP_C,
    NIM_LD   | LOP_H       | ROP_D,
    NIM_LD   | LOP_H       | ROP_E,
    NIM_LD   | LOP_H       | ROP_H,
    NIM_LD   | LOP_H       | ROP_L,
    NIM_LD   | LOP_H       | ROP_REF_HL,
    NIM_LD   | LOP_H       | ROP_A,

    /* 68 */
    NIM_LD   | LOP_L       | ROP_B,
    NIM_LD   | LOP_L       | ROP_C,
    NIM_LD   | LOP_L       | ROP_D,
    NIM_LD   | LOP_L       | ROP_E,
    NIM_LD   | LOP_L       | ROP_H,
    NIM_LD   | LOP_L       | ROP_L,
    NIM_LD   | LOP_L       | ROP_REF_HL,
    NIM_LD   | LOP_L       | ROP_A,

    /* 70 */
    NIM_LD   | LOP_REF_HL  | ROP_B,
    NIM_LD   | LOP_REF_HL  | ROP_C,
    NIM_LD   | LOP_REF_HL  | ROP_D,
    NIM_LD   | LOP_REF_HL  | ROP_E,
    NIM_LD   | LOP_REF_HL  | ROP_H,
    NIM_LD   | LOP_REF_HL  | ROP_L,
    NIM_HALT,
    NIM_LD   | LOP_REF_HL  | ROP_A,

    /* 78 */
    NIM_LD   | LOP_A       | ROP_B,
    NIM_LD   | LOP_A       | ROP_C,
    NIM_LD   | LOP_A       | ROP_D,
    NIM_LD   | LOP_A       | ROP_E,
    NIM_LD   | LOP_A       | ROP_H,
    NIM_LD   | LOP_A       | ROP_L,
    NIM_LD   | LOP_A       | ROP_REF_HL,
    NIM_LD   | LOP_A       | ROP_A,

    /* 80 */
    NIM_ADD  | LOP_B,
    NIM_ADD  | LOP_C,
    NIM_ADD  | LOP_D,
    NIM_ADD  | LOP_E,
    NIM_ADD  | LOP_H,
    NIM_ADD  | LOP_L,
    NIM_ADD  | LOP_REF_HL,
    NIM_ADD  | LOP_A,

    /* 88 */
    NIM_ADC  | LOP_B,
    NIM_ADC  | LOP_C,
    NIM_ADC  | LOP_D,
    NIM_ADC  | LOP_E,
    NIM_ADC  | LOP_H,
    NIM_ADC  | LOP_L,
    NIM_ADC  | LOP_REF_HL,
    NIM_ADC  | LOP_A,

    /* 90 */
    NIM_SUB  | LOP_B,
    NIM_SUB  | LOP_C,
    NIM_SUB  | LOP_D,
    NIM_SUB  | LOP_E,
    NIM_SUB  | LOP_H,
    NIM_SUB  | LOP_L,
    NIM_SUB  | LOP_REF_HL,
    NIM_SUB  | LOP_A,

    /* 98 */
    NIM_SBC  | LOP_B,
    NIM_SBC  | LOP_C,
    NIM_SBC  | LOP_D,
    NIM_SBC  | LOP_E,
    NIM_SBC  | LOP_H,
    NIM_SBC  | LOP_L,
    NIM_SBC  | LOP_REF_HL,
    NIM_SBC  | LOP_A,

    /* a0 */
    NIM_AND  | LOP_B,
    NIM_AND  | LOP_C,
    NIM_AND  | LOP_D,
    NIM_AND  | LOP_E,
    NIM_AND  | LOP_H,
    NIM_AND  | LOP_L,
    NIM_AND  | LOP_REF_HL,
    NIM_AND  | LOP_A,

    /* a8 */
    NIM_XOR  | LOP_B,
    NIM_XOR  | LOP_C,
    NIM_XOR  | LOP_D,
    NIM_XOR  | LOP_E,
    NIM_XOR  | LOP_H,
    NIM_XOR  | LOP_L,
    NIM_XOR  | LOP_REF_HL,
    NIM_XOR  | LOP_A,

    /* b0 */
    NIM_OR   | LOP_B,
    NIM_OR   | LOP_C,
    NIM_OR   | LOP_D,
    NIM_OR   | LOP_E,
    NIM_OR   | LOP_H,
    NIM_OR   | LOP_L,
    NIM_OR   | LOP_REF_HL,
    NIM_OR   | LOP_A,

    /* b8 */
    NIM_CP   | LOP_B,
    NIM_CP   | LOP_C,
    NIM_CP   | LOP_D,
    NIM_CP   | LOP_E,
    NIM_CP   | LOP_H,
    NIM_CP   | LOP_L,
    NIM_CP   | LOP_REF_HL,
    NIM_CP   | LOP_A,

    /* c0 */
    NIM_RET  | CC_NZ,
    NIM_POP  | LOP_BC,
    NIM_JP   | CC_NZ       | ROP_IMM16,
    NIM_JP   | LOP_IMM16,
    NIM_CALL | CC_NZ       | ROP_IMM16,
    NIM_PUSH | LOP_BC,
    NIM_ADD  | LOP_IMM8,
    NIM_RST00,

    /* c8 */
    NIM_RET  | CC_Z,
    NIM_RET,
    NIM_JP   | CC_Z        | ROP_IMM16,
    0,
    NIM_CALL | CC_Z        | ROP_IMM16,
    NIM_CALL | LOP_IMM16,
    NIM_ADC  | LOP_IMM8,
    NIM_RST08,

    /* d0 */
    NIM_RET  | CC_NC,
    NIM_POP  | LOP_DE,
    NIM_JP   | CC_NC       | ROP_IMM16,
    NIM_OUT  | LOP_REF_8   | ROP_A,
    NIM_CALL | CC_NC       | ROP_IMM16,
    NIM_PUSH | LOP_DE,
    NIM_SUB  | LOP_IMM8,
    NIM_RST10,

    /* d8 */
    NIM_RET  | LOP_C,
    NIM_EXX,
    NIM_JP   | CC_C        | ROP_IMM16,
    NIM_IN   | LOP_A       | ROP_REF_8,
    NIM_CALL | CC_C        | ROP_IMM16,
    0,
    NIM_SBC  | LOP_A       | ROP_IMM8,
    NIM_RST18,

    /* e0 */
    NIM_RET  | CC_PO,
    NIM_POP  | LOP_HL,
    NIM_JP   | CC_PO       | ROP_IMM16,
    NIM_EX   | LOP_REF_SP  | ROP_HL,
    NIM_CALL | CC_PO       | ROP_IMM16,
    NIM_PUSH | LOP_HL,
    NIM_AND  | LOP_IMM8,
    NIM_RST20,

    /* e8 */
    NIM_RET  | CC_PE,
    NIM_JP   | LOP_REF_HL,
    NIM_JP   | CC_PE       | ROP_IMM16,
    NIM_EX   | LOP_DE      | ROP_HL,
    NIM_CALL | CC_PE       | ROP_IMM16,
    0,
    NIM_XOR  | LOP_IMM8,
    NIM_RST28,

    /* f0 */
    NIM_RET  | CC_P,
    NIM_POP  | LOP_AF,
    NIM_JP   | CC_P        | ROP_IMM16,
    NIM_DI,
    NIM_CALL | CC_P        | ROP_IMM16,
    NIM_PUSH | LOP_AF,
    NIM_OR   | LOP_IMM8,
    NIM_RST30,

    /* f8 */
    NIM_RET  | CC_M,
    NIM_LD   | LOP_SP      | ROP_HL,
    NIM_JP   | CC_M        | ROP_IMM16,
    NIM_EI,
    NIM_CALL | CC_M        | ROP_IMM16,
    0,
    NIM_CP   | LOP_IMM8,
    NIM_RST38
};

const static uint32 cb_xx[] = {
    /* 00 */
    NIM_RLC  | LOP_B,
    NIM_RLC  | LOP_C,
    NIM_RLC  | LOP_D,
    NIM_RLC  | LOP_E,
    NIM_RLC  | LOP_H,
    NIM_RLC  | LOP_L,
    NIM_RLC  | LOP_REF_HL,
    NIM_RLC  | LOP_A,

    /* 08 */
    NIM_RRC  | LOP_B,
    NIM_RRC  | LOP_C,
    NIM_RRC  | LOP_D,
    NIM_RRC  | LOP_E,
    NIM_RRC  | LOP_H,
    NIM_RRC  | LOP_L,
    NIM_RRC  | LOP_REF_HL,
    NIM_RRC  | LOP_A,

    /* 10 */
    NIM_RL   | LOP_B,
    NIM_RL   | LOP_C,
    NIM_RL   | LOP_D,
    NIM_RL   | LOP_E,
    NIM_RL   | LOP_H,
    NIM_RL   | LOP_L,
    NIM_RL   | LOP_REF_HL,
    NIM_RL   | LOP_A,

    /* 18 */
    NIM_RR   | LOP_B,
    NIM_RR   | LOP_C,
    NIM_RR   | LOP_D,
    NIM_RR   | LOP_E,
    NIM_RR   | LOP_H,
    NIM_RR   | LOP_L,
    NIM_RR   | LOP_REF_HL,
    NIM_RR   | LOP_A,

    /* 20 */
    NIM_SLA  | LOP_B,
    NIM_SLA  | LOP_C,
    NIM_SLA  | LOP_D,
    NIM_SLA  | LOP_E,
    NIM_SLA  | LOP_H,
    NIM_SLA  | LOP_L,
    NIM_SLA  | LOP_REF_HL,
    NIM_SLA  | LOP_A,

    /* 28 */
    NIM_SRA  | LOP_B,
    NIM_SRA  | LOP_C,
    NIM_SRA  | LOP_D,
    NIM_SRA  | LOP_E,
    NIM_SRA  | LOP_H,
    NIM_SRA  | LOP_L,
    NIM_SRA  | LOP_REF_HL,
    NIM_SRA  | LOP_A,

    /* 30 */
    NIM_SLL  | LOP_B,
    NIM_SLL  | LOP_C,
    NIM_SLL  | LOP_D,
    NIM_SLL  | LOP_E,
    NIM_SLL  | LOP_H,
    NIM_SLL  | LOP_L,
    NIM_SLL  | LOP_REF_HL,
    NIM_SLL  | LOP_A,

    /* 38 */
    NIM_SRL  | LOP_B,
    NIM_SRL  | LOP_C,
    NIM_SRL  | LOP_D,
    NIM_SRL  | LOP_E,
    NIM_SRL  | LOP_H,
    NIM_SRL  | LOP_L,
    NIM_SRL  | LOP_REF_HL,
    NIM_SRL  | LOP_A,

    /* 40 */
    NIM_BIT0 | LOP_B,
    NIM_BIT0 | LOP_C,
    NIM_BIT0 | LOP_D,
    NIM_BIT0 | LOP_E,
    NIM_BIT0 | LOP_H,
    NIM_BIT0 | LOP_L,
    NIM_BIT0 | LOP_REF_HL,
    NIM_BIT0 | LOP_A,

    /* 48 */
    NIM_BIT1 | LOP_B,
    NIM_BIT1 | LOP_C,
    NIM_BIT1 | LOP_D,
    NIM_BIT1 | LOP_E,
    NIM_BIT1 | LOP_H,
    NIM_BIT1 | LOP_L,
    NIM_BIT1 | LOP_REF_HL,
    NIM_BIT1 | LOP_A,

    /* 50 */
    NIM_BIT2 | LOP_B,
    NIM_BIT2 | LOP_C,
    NIM_BIT2 | LOP_D,
    NIM_BIT2 | LOP_E,
    NIM_BIT2 | LOP_H,
    NIM_BIT2 | LOP_L,
    NIM_BIT2 | LOP_REF_HL,
    NIM_BIT2 | LOP_A,

    /* 58 */
    NIM_BIT3 | LOP_B,
    NIM_BIT3 | LOP_C,
    NIM_BIT3 | LOP_D,
    NIM_BIT3 | LOP_E,
    NIM_BIT3 | LOP_H,
    NIM_BIT3 | LOP_L,
    NIM_BIT3 | LOP_REF_HL,
    NIM_BIT3 | LOP_A,

    /* 60 */
    NIM_BIT4 | LOP_B,
    NIM_BIT4 | LOP_C,
    NIM_BIT4 | LOP_D,
    NIM_BIT4 | LOP_E,
    NIM_BIT4 | LOP_H,
    NIM_BIT4 | LOP_L,
    NIM_BIT4 | LOP_REF_HL,
    NIM_BIT4 | LOP_A,

    /* 68 */
    NIM_BIT5 | LOP_B,
    NIM_BIT5 | LOP_C,
    NIM_BIT5 | LOP_D,
    NIM_BIT5 | LOP_E,
    NIM_BIT5 | LOP_H,
    NIM_BIT5 | LOP_L,
    NIM_BIT5 | LOP_REF_HL,
    NIM_BIT5 | LOP_A,

    /* 70 */
    NIM_BIT6 | LOP_B,
    NIM_BIT6 | LOP_C,
    NIM_BIT6 | LOP_D,
    NIM_BIT6 | LOP_E,
    NIM_BIT6 | LOP_H,
    NIM_BIT6 | LOP_L,
    NIM_BIT6 | LOP_REF_HL,
    NIM_BIT6 | LOP_A,

    /* 78 */
    NIM_BIT7 | LOP_B,
    NIM_BIT7 | LOP_C,
    NIM_BIT7 | LOP_D,
    NIM_BIT7 | LOP_E,
    NIM_BIT7 | LOP_H,
    NIM_BIT7 | LOP_L,
    NIM_BIT7 | LOP_REF_HL,
    NIM_BIT7 | LOP_A,

    /* 80 */
    NIM_RES0 | LOP_B,
    NIM_RES0 | LOP_C,
    NIM_RES0 | LOP_D,
    NIM_RES0 | LOP_E,
    NIM_RES0 | LOP_H,
    NIM_RES0 | LOP_L,
    NIM_RES0 | LOP_REF_HL,
    NIM_RES0 | LOP_A,

    /* 88 */
    NIM_RES1 | LOP_B,
    NIM_RES1 | LOP_C,
    NIM_RES1 | LOP_D,
    NIM_RES1 | LOP_E,
    NIM_RES1 | LOP_H,
    NIM_RES1 | LOP_L,
    NIM_RES1 | LOP_REF_HL,
    NIM_RES1 | LOP_A,

    /* 90 */
    NIM_RES2 | LOP_B,
    NIM_RES2 | LOP_C,
    NIM_RES2 | LOP_D,
    NIM_RES2 | LOP_E,
    NIM_RES2 | LOP_H,
    NIM_RES2 | LOP_L,
    NIM_RES2 | LOP_REF_HL,
    NIM_RES2 | LOP_A,

    /* 98 */
    NIM_RES3 | LOP_B,
    NIM_RES3 | LOP_C,
    NIM_RES3 | LOP_D,
    NIM_RES3 | LOP_E,
    NIM_RES3 | LOP_H,
    NIM_RES3 | LOP_L,
    NIM_RES3 | LOP_REF_HL,
    NIM_RES3 | LOP_A,

    /* a0 */
    NIM_RES4 | LOP_B,
    NIM_RES4 | LOP_C,
    NIM_RES4 | LOP_D,
    NIM_RES4 | LOP_E,
    NIM_RES4 | LOP_H,
    NIM_RES4 | LOP_L,
    NIM_RES4 | LOP_REF_HL,
    NIM_RES4 | LOP_A,

    /* a8 */
    NIM_RES5 | LOP_B,
    NIM_RES5 | LOP_C,
    NIM_RES5 | LOP_D,
    NIM_RES5 | LOP_E,
    NIM_RES5 | LOP_H,
    NIM_RES5 | LOP_L,
    NIM_RES5 | LOP_REF_HL,
    NIM_RES5 | LOP_A,

    /* b0 */
    NIM_RES6 | LOP_B,
    NIM_RES6 | LOP_C,
    NIM_RES6 | LOP_D,
    NIM_RES6 | LOP_E,
    NIM_RES6 | LOP_H,
    NIM_RES6 | LOP_L,
    NIM_RES6 | LOP_REF_HL,
    NIM_RES6 | LOP_A,

    /* b8 */
    NIM_RES7 | LOP_B,
    NIM_RES7 | LOP_C,
    NIM_RES7 | LOP_D,
    NIM_RES7 | LOP_E,
    NIM_RES7 | LOP_H,
    NIM_RES7 | LOP_L,
    NIM_RES7 | LOP_REF_HL,
    NIM_RES7 | LOP_A,

    /* c0 */
    NIM_SET0 | LOP_B,
    NIM_SET0 | LOP_C,
    NIM_SET0 | LOP_D,
    NIM_SET0 | LOP_E,
    NIM_SET0 | LOP_H,
    NIM_SET0 | LOP_L,
    NIM_SET0 | LOP_REF_HL,
    NIM_SET0 | LOP_A,

    /* c8 */
    NIM_SET1 | LOP_B,
    NIM_SET1 | LOP_C,
    NIM_SET1 | LOP_D,
    NIM_SET1 | LOP_E,
    NIM_SET1 | LOP_H,
    NIM_SET1 | LOP_L,
    NIM_SET1 | LOP_REF_HL,
    NIM_SET1 | LOP_A,

    /* d0 */
    NIM_SET2 | LOP_B,
    NIM_SET2 | LOP_C,
    NIM_SET2 | LOP_D,
    NIM_SET2 | LOP_E,
    NIM_SET2 | LOP_H,
    NIM_SET2 | LOP_L,
    NIM_SET2 | LOP_REF_HL,
    NIM_SET2 | LOP_A,

    /* d8 */
    NIM_SET3 | LOP_B,
    NIM_SET3 | LOP_C,
    NIM_SET3 | LOP_D,
    NIM_SET3 | LOP_E,
    NIM_SET3 | LOP_H,
    NIM_SET3 | LOP_L,
    NIM_SET3 | LOP_REF_HL,
    NIM_SET3 | LOP_A,

    /* e0 */
    NIM_SET4 | LOP_B,
    NIM_SET4 | LOP_C,
    NIM_SET4 | LOP_D,
    NIM_SET4 | LOP_E,
    NIM_SET4 | LOP_H,
    NIM_SET4 | LOP_L,
    NIM_SET4 | LOP_REF_HL,
    NIM_SET4 | LOP_A,

    /* e8 */
    NIM_SET5 | LOP_B,
    NIM_SET5 | LOP_C,
    NIM_SET5 | LOP_D,
    NIM_SET5 | LOP_E,
    NIM_SET5 | LOP_H,
    NIM_SET5 | LOP_L,
    NIM_SET5 | LOP_REF_HL,
    NIM_SET5 | LOP_A,

    /* f0 */
    NIM_SET6 | LOP_B,
    NIM_SET6 | LOP_C,
    NIM_SET6 | LOP_D,
    NIM_SET6 | LOP_E,
    NIM_SET6 | LOP_H,
    NIM_SET6 | LOP_L,
    NIM_SET6 | LOP_REF_HL,
    NIM_SET6 | LOP_A,

    /* f8 */
    NIM_SET7 | LOP_B,
    NIM_SET7 | LOP_C,
    NIM_SET7 | LOP_D,
    NIM_SET7 | LOP_E,
    NIM_SET7 | LOP_H,
    NIM_SET7 | LOP_L,
    NIM_SET7 | LOP_REF_HL,
    NIM_SET7 | LOP_A
};

static uint32 dd_xx[] = {
    /* 00 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 08 */
    NIM_UNKNOWN,
    NIM_ADD  | LOP_IX      | ROP_BC,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 10 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 18 */
    NIM_UNKNOWN,
    NIM_ADD  | LOP_IX      | ROP_DE,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 20 */
    NIM_UNKNOWN,
    NIM_LD   | LOP_IX      | ROP_IMM16,
    NIM_LD   | LOP_REF_16  | ROP_IX,
    NIM_INC  | LOP_IX,
    NIM_INC  | LOP_IXH,
    NIM_DEC  | LOP_IXH,
    NIM_LD   | LOP_IXH     | ROP_IMM8,
    NIM_UNKNOWN,

    /* 28 */
    NIM_UNKNOWN,
    NIM_ADD  | LOP_IX      | ROP_HL,
    NIM_LD   | LOP_IX      | ROP_REF_16,
    NIM_DEC  | LOP_IX,
    NIM_INC  | LOP_IXL,
    NIM_DEC  | LOP_IXL,
    NIM_LD   | LOP_IX      | ROP_IMM8,
    NIM_UNKNOWN,

    /* 30 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_INC  | LOP_REF_IXD,
    NIM_DEC  | LOP_REF_IXD,
    NIM_LD   | LOP_REF_IXD | ROP_IMM8,
    NIM_UNKNOWN,

    /* 38 */
    NIM_UNKNOWN,
    NIM_ADD  | LOP_IX      | ROP_SP,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 40 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_LD   | LOP_B       | ROP_IXH,
    NIM_LD   | LOP_B       | ROP_IXL,
    NIM_LD   | LOP_B       | ROP_REF_IXD,
    NIM_UNKNOWN,

    /* 48 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_LD   | LOP_C       | ROP_IXH,
    NIM_LD   | LOP_C       | ROP_IXL,
    NIM_LD   | LOP_C       | ROP_REF_IXD,
    NIM_UNKNOWN,

    /* 50 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_LD   | LOP_D       | ROP_IXH,
    NIM_LD   | LOP_D       | ROP_IXL,
    NIM_LD   | LOP_D       | ROP_REF_IXD,
    NIM_UNKNOWN,

    /* 58 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_LD   | LOP_E       | ROP_IXH,
    NIM_LD   | LOP_E       | ROP_IXL,
    NIM_LD   | LOP_E       | ROP_REF_IXD,
    NIM_UNKNOWN,

    /* 60 */
    NIM_LD   | LOP_IXH     | ROP_B,
    NIM_LD   | LOP_IXH     | ROP_C,
    NIM_LD   | LOP_IXH     | ROP_D,
    NIM_LD   | LOP_IXH     | ROP_E,
    NIM_LD   | LOP_IXH     | ROP_H,
    NIM_LD   | LOP_IXH     | ROP_L,
    NIM_LD   | LOP_H       | ROP_REF_IXD,
    NIM_LD   | LOP_IXH     | ROP_A,

    /* 68 */
    NIM_LD   | LOP_IXL     | ROP_B,
    NIM_LD   | LOP_IXL     | ROP_C,
    NIM_LD   | LOP_IXL     | ROP_D,
    NIM_LD   | LOP_IXL     | ROP_E,
    NIM_LD   | LOP_IXL     | ROP_H,
    NIM_LD   | LOP_IXL     | ROP_L,
    NIM_LD   | LOP_L       | ROP_REF_IXD,
    NIM_LD   | LOP_IXL     | ROP_A,

    /* 70 */
    NIM_LD   | LOP_REF_IXD | ROP_B,
    NIM_LD   | LOP_REF_IXD | ROP_C,
    NIM_LD   | LOP_REF_IXD | ROP_D,
    NIM_LD   | LOP_REF_IXD | ROP_E,
    NIM_LD   | LOP_REF_IXD | ROP_H,
    NIM_LD   | LOP_REF_IXD | ROP_L,
    NIM_UNKNOWN,
    NIM_LD   | LOP_REF_IXD | ROP_A,

    /* 78 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_LD   | LOP_A       | ROP_IXH,
    NIM_LD   | LOP_A       | ROP_IXL,
    NIM_LD   | LOP_A       | ROP_REF_IXD,
    NIM_UNKNOWN,

    /* 80 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_ADD  | LOP_IXH,
    NIM_ADD  | LOP_IXL,
    NIM_ADD  | LOP_REF_IXD,
    NIM_UNKNOWN,

    /* 88 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_ADC  | LOP_IXH,
    NIM_ADC  | LOP_IXL,
    NIM_ADC  | LOP_REF_IXD,
    NIM_UNKNOWN,

    /* 90 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_SUB  | LOP_IXH,
    NIM_SUB  | LOP_IXL,
    NIM_SUB  | LOP_REF_IXD,
    NIM_UNKNOWN,

    /* 98 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_SBC  | LOP_IXH,
    NIM_SBC  | LOP_IXL,
    NIM_SBC  | LOP_REF_IXD,
    NIM_UNKNOWN,

    /* a0 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_AND  | LOP_IXH,
    NIM_AND  | LOP_IXL,
    NIM_AND  | LOP_REF_IXD,
    NIM_UNKNOWN,

    /* a8 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_XOR  | LOP_IXH,
    NIM_XOR  | LOP_IXL,
    NIM_XOR  | LOP_REF_IXD,
    NIM_UNKNOWN,

    /* b0 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_OR   | LOP_IXH,
    NIM_OR   | LOP_IXL,
    NIM_OR   | LOP_REF_IXD,
    NIM_UNKNOWN,

    /* b8 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_CP   | LOP_IXH,
    NIM_CP   | LOP_IXL,
    NIM_CP   | LOP_REF_IXD,
    NIM_UNKNOWN,

    /* c0 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* c8 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    0,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* d0 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* d8 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* e0 */
    NIM_UNKNOWN,
    NIM_POP  | LOP_IX,
    NIM_UNKNOWN,
    NIM_EX   | LOP_REF_SP  | ROP_IX,
    NIM_UNKNOWN,
    NIM_PUSH | LOP_IX,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* e8 */
    NIM_UNKNOWN,
    NIM_JP   | LOP_REF_IX,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* f0 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* f8 */
    NIM_UNKNOWN,
    NIM_LD   | LOP_SP      | ROP_IX,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN
};

const static uint32 dd_cb_xx[] = {
    /* 00 */
    NIM_RLC  | OP_REF_IXD  | ROP_B,
    NIM_RLC  | OP_REF_IXD  | ROP_C,
    NIM_RLC  | OP_REF_IXD  | ROP_D,
    NIM_RLC  | OP_REF_IXD  | ROP_E,
    NIM_RLC  | OP_REF_IXD  | ROP_H,
    NIM_RLC  | OP_REF_IXD  | ROP_L,
    NIM_RLC  | OP_REF_IXD,
    NIM_RLC  | OP_REF_IXD  | ROP_A,

    /* 08 */
    NIM_RRC  | OP_REF_IXD  | ROP_B,
    NIM_RRC  | OP_REF_IXD  | ROP_C,
    NIM_RRC  | OP_REF_IXD  | ROP_D,
    NIM_RRC  | OP_REF_IXD  | ROP_E,
    NIM_RRC  | OP_REF_IXD  | ROP_H,
    NIM_RRC  | OP_REF_IXD  | ROP_L,
    NIM_RRC  | OP_REF_IXD,
    NIM_RRC  | OP_REF_IXD  | ROP_A,

    /* 10 */
    NIM_RL   | OP_REF_IXD  | ROP_B,
    NIM_RL   | OP_REF_IXD  | ROP_C,
    NIM_RL   | OP_REF_IXD  | ROP_D,
    NIM_RL   | OP_REF_IXD  | ROP_E,
    NIM_RL   | OP_REF_IXD  | ROP_H,
    NIM_RL   | OP_REF_IXD  | ROP_L,
    NIM_RL   | OP_REF_IXD,
    NIM_RL   | OP_REF_IXD  | ROP_A,

    /* 18 */
    NIM_RR   | OP_REF_IXD  | ROP_B,
    NIM_RR   | OP_REF_IXD  | ROP_C,
    NIM_RR   | OP_REF_IXD  | ROP_D,
    NIM_RR   | OP_REF_IXD  | ROP_E,
    NIM_RR   | OP_REF_IXD  | ROP_H,
    NIM_RR   | OP_REF_IXD  | ROP_L,
    NIM_RR   | OP_REF_IXD,
    NIM_RR   | OP_REF_IXD  | ROP_A,

    /* 20 */
    NIM_SLA  | OP_REF_IXD  | ROP_B,
    NIM_SLA  | OP_REF_IXD  | ROP_C,
    NIM_SLA  | OP_REF_IXD  | ROP_D,
    NIM_SLA  | OP_REF_IXD  | ROP_E,
    NIM_SLA  | OP_REF_IXD  | ROP_H,
    NIM_SLA  | OP_REF_IXD  | ROP_L,
    NIM_SLA  | OP_REF_IXD,
    NIM_SLA  | OP_REF_IXD  | ROP_A,

    /* 28 */
    NIM_SRA  | OP_REF_IXD  | ROP_B,
    NIM_SRA  | OP_REF_IXD  | ROP_C,
    NIM_SRA  | OP_REF_IXD  | ROP_D,
    NIM_SRA  | OP_REF_IXD  | ROP_E,
    NIM_SRA  | OP_REF_IXD  | ROP_H,
    NIM_SRA  | OP_REF_IXD  | ROP_L,
    NIM_SRA  | OP_REF_IXD,
    NIM_SRA  | OP_REF_IXD  | ROP_A,

    /* 30 */
    NIM_SLL  | OP_REF_IXD  | ROP_B,
    NIM_SLL  | OP_REF_IXD  | ROP_C,
    NIM_SLL  | OP_REF_IXD  | ROP_D,
    NIM_SLL  | OP_REF_IXD  | ROP_E,
    NIM_SLL  | OP_REF_IXD  | ROP_H,
    NIM_SLL  | OP_REF_IXD  | ROP_L,
    NIM_SLL  | OP_REF_IXD,
    NIM_SLL  | OP_REF_IXD  | ROP_A,

    /* 38 */
    NIM_SRL  | OP_REF_IXD  | ROP_B,
    NIM_SRL  | OP_REF_IXD  | ROP_C,
    NIM_SRL  | OP_REF_IXD  | ROP_D,
    NIM_SRL  | OP_REF_IXD  | ROP_E,
    NIM_SRL  | OP_REF_IXD  | ROP_H,
    NIM_SRL  | OP_REF_IXD  | ROP_L,
    NIM_SRL  | OP_REF_IXD,
    NIM_SRL  | OP_REF_IXD  | ROP_A,

    /* 40 */
    NIM_BIT0 | OP_REF_IXD,
    NIM_BIT0 | OP_REF_IXD,
    NIM_BIT0 | OP_REF_IXD,
    NIM_BIT0 | OP_REF_IXD,
    NIM_BIT0 | OP_REF_IXD,
    NIM_BIT0 | OP_REF_IXD,
    NIM_BIT0 | OP_REF_IXD,
    NIM_BIT0 | OP_REF_IXD,

    /* 48 */
    NIM_BIT1 | OP_REF_IXD,
    NIM_BIT1 | OP_REF_IXD,
    NIM_BIT1 | OP_REF_IXD,
    NIM_BIT1 | OP_REF_IXD,
    NIM_BIT1 | OP_REF_IXD,
    NIM_BIT1 | OP_REF_IXD,
    NIM_BIT1 | OP_REF_IXD,
    NIM_BIT1 | OP_REF_IXD,

    /* 50 */
    NIM_BIT2 | OP_REF_IXD,
    NIM_BIT2 | OP_REF_IXD,
    NIM_BIT2 | OP_REF_IXD,
    NIM_BIT2 | OP_REF_IXD,
    NIM_BIT2 | OP_REF_IXD,
    NIM_BIT2 | OP_REF_IXD,
    NIM_BIT2 | OP_REF_IXD,
    NIM_BIT2 | OP_REF_IXD,

    /* 58 */
    NIM_BIT3 | OP_REF_IXD,
    NIM_BIT3 | OP_REF_IXD,
    NIM_BIT3 | OP_REF_IXD,
    NIM_BIT3 | OP_REF_IXD,
    NIM_BIT3 | OP_REF_IXD,
    NIM_BIT3 | OP_REF_IXD,
    NIM_BIT3 | OP_REF_IXD,
    NIM_BIT3 | OP_REF_IXD,

    /* 60 */
    NIM_BIT4 | OP_REF_IXD,
    NIM_BIT4 | OP_REF_IXD,
    NIM_BIT4 | OP_REF_IXD,
    NIM_BIT4 | OP_REF_IXD,
    NIM_BIT4 | OP_REF_IXD,
    NIM_BIT4 | OP_REF_IXD,
    NIM_BIT4 | OP_REF_IXD,
    NIM_BIT4 | OP_REF_IXD,

    /* 68 */
    NIM_BIT5 | OP_REF_IXD,
    NIM_BIT5 | OP_REF_IXD,
    NIM_BIT5 | OP_REF_IXD,
    NIM_BIT5 | OP_REF_IXD,
    NIM_BIT5 | OP_REF_IXD,
    NIM_BIT5 | OP_REF_IXD,
    NIM_BIT5 | OP_REF_IXD,
    NIM_BIT5 | OP_REF_IXD,

    /* 70 */
    NIM_BIT6 | OP_REF_IXD,
    NIM_BIT6 | OP_REF_IXD,
    NIM_BIT6 | OP_REF_IXD,
    NIM_BIT6 | OP_REF_IXD,
    NIM_BIT6 | OP_REF_IXD,
    NIM_BIT6 | OP_REF_IXD,
    NIM_BIT6 | OP_REF_IXD,
    NIM_BIT6 | OP_REF_IXD,

    /* 78 */
    NIM_BIT7 | OP_REF_IXD,
    NIM_BIT7 | OP_REF_IXD,
    NIM_BIT7 | OP_REF_IXD,
    NIM_BIT7 | OP_REF_IXD,
    NIM_BIT7 | OP_REF_IXD,
    NIM_BIT7 | OP_REF_IXD,
    NIM_BIT7 | OP_REF_IXD,
    NIM_BIT7 | OP_REF_IXD,

    /* 80 */
    NIM_RES0 | OP_REF_IXD  | ROP_B,
    NIM_RES0 | OP_REF_IXD  | ROP_C,
    NIM_RES0 | OP_REF_IXD  | ROP_D,
    NIM_RES0 | OP_REF_IXD  | ROP_E,
    NIM_RES0 | OP_REF_IXD  | ROP_H,
    NIM_RES0 | OP_REF_IXD  | ROP_L,
    NIM_RES0 | OP_REF_IXD,
    NIM_RES0 | OP_REF_IXD  | ROP_A,

    /* 88 */
    NIM_RES1 | OP_REF_IXD  | ROP_B,
    NIM_RES1 | OP_REF_IXD  | ROP_C,
    NIM_RES1 | OP_REF_IXD  | ROP_D,
    NIM_RES1 | OP_REF_IXD  | ROP_E,
    NIM_RES1 | OP_REF_IXD  | ROP_H,
    NIM_RES1 | OP_REF_IXD  | ROP_L,
    NIM_RES1 | OP_REF_IXD,
    NIM_RES1 | OP_REF_IXD  | ROP_A,

    /* 90 */
    NIM_RES2 | OP_REF_IXD  | ROP_B,
    NIM_RES2 | OP_REF_IXD  | ROP_C,
    NIM_RES2 | OP_REF_IXD  | ROP_D,
    NIM_RES2 | OP_REF_IXD  | ROP_E,
    NIM_RES2 | OP_REF_IXD  | ROP_H,
    NIM_RES2 | OP_REF_IXD  | ROP_L,
    NIM_RES2 | OP_REF_IXD,
    NIM_RES2 | OP_REF_IXD  | ROP_A,

    /* 98 */
    NIM_RES3 | OP_REF_IXD  | ROP_B,
    NIM_RES3 | OP_REF_IXD  | ROP_C,
    NIM_RES3 | OP_REF_IXD  | ROP_D,
    NIM_RES3 | OP_REF_IXD  | ROP_E,
    NIM_RES3 | OP_REF_IXD  | ROP_H,
    NIM_RES3 | OP_REF_IXD  | ROP_L,
    NIM_RES3 | OP_REF_IXD,
    NIM_RES3 | OP_REF_IXD  | ROP_A,

    /* a0 */
    NIM_RES4 | OP_REF_IXD  | ROP_B,
    NIM_RES4 | OP_REF_IXD  | ROP_C,
    NIM_RES4 | OP_REF_IXD  | ROP_D,
    NIM_RES4 | OP_REF_IXD  | ROP_E,
    NIM_RES4 | OP_REF_IXD  | ROP_H,
    NIM_RES4 | OP_REF_IXD  | ROP_L,
    NIM_RES4 | OP_REF_IXD,
    NIM_RES4 | OP_REF_IXD  | ROP_A,

    /* a8 */
    NIM_RES5 | OP_REF_IXD  | ROP_B,
    NIM_RES5 | OP_REF_IXD  | ROP_C,
    NIM_RES5 | OP_REF_IXD  | ROP_D,
    NIM_RES5 | OP_REF_IXD  | ROP_E,
    NIM_RES5 | OP_REF_IXD  | ROP_H,
    NIM_RES5 | OP_REF_IXD  | ROP_L,
    NIM_RES5 | OP_REF_IXD,
    NIM_RES5 | OP_REF_IXD  | ROP_A,

    /* b0 */
    NIM_RES6 | OP_REF_IXD  | ROP_B,
    NIM_RES6 | OP_REF_IXD  | ROP_C,
    NIM_RES6 | OP_REF_IXD  | ROP_D,
    NIM_RES6 | OP_REF_IXD  | ROP_E,
    NIM_RES6 | OP_REF_IXD  | ROP_H,
    NIM_RES6 | OP_REF_IXD  | ROP_L,
    NIM_RES6 | OP_REF_IXD,
    NIM_RES6 | OP_REF_IXD  | ROP_A,

    /* b8 */
    NIM_RES7 | OP_REF_IXD  | ROP_B,
    NIM_RES7 | OP_REF_IXD  | ROP_C,
    NIM_RES7 | OP_REF_IXD  | ROP_D,
    NIM_RES7 | OP_REF_IXD  | ROP_E,
    NIM_RES7 | OP_REF_IXD  | ROP_H,
    NIM_RES7 | OP_REF_IXD  | ROP_L,
    NIM_RES7 | OP_REF_IXD,
    NIM_RES7 | OP_REF_IXD  | ROP_A,

    /* c0 */
    NIM_SET0 | OP_REF_IXD  | ROP_B,
    NIM_SET0 | OP_REF_IXD  | ROP_C,
    NIM_SET0 | OP_REF_IXD  | ROP_D,
    NIM_SET0 | OP_REF_IXD  | ROP_E,
    NIM_SET0 | OP_REF_IXD  | ROP_H,
    NIM_SET0 | OP_REF_IXD  | ROP_L,
    NIM_SET0 | OP_REF_IXD,
    NIM_SET0 | OP_REF_IXD  | ROP_A,

    /* c8 */
    NIM_SET1 | OP_REF_IXD  | ROP_B,
    NIM_SET1 | OP_REF_IXD  | ROP_C,
    NIM_SET1 | OP_REF_IXD  | ROP_D,
    NIM_SET1 | OP_REF_IXD  | ROP_E,
    NIM_SET1 | OP_REF_IXD  | ROP_H,
    NIM_SET1 | OP_REF_IXD  | ROP_L,
    NIM_SET1 | OP_REF_IXD,
    NIM_SET1 | OP_REF_IXD  | ROP_A,

    /* d0 */
    NIM_SET2 | OP_REF_IXD  | ROP_B,
    NIM_SET2 | OP_REF_IXD  | ROP_C,
    NIM_SET2 | OP_REF_IXD  | ROP_D,
    NIM_SET2 | OP_REF_IXD  | ROP_E,
    NIM_SET2 | OP_REF_IXD  | ROP_H,
    NIM_SET2 | OP_REF_IXD  | ROP_L,
    NIM_SET2 | OP_REF_IXD,
    NIM_SET2 | OP_REF_IXD  | ROP_A,

    /* d8 */
    NIM_SET3 | OP_REF_IXD  | ROP_B,
    NIM_SET3 | OP_REF_IXD  | ROP_C,
    NIM_SET3 | OP_REF_IXD  | ROP_D,
    NIM_SET3 | OP_REF_IXD  | ROP_E,
    NIM_SET3 | OP_REF_IXD  | ROP_H,
    NIM_SET3 | OP_REF_IXD  | ROP_L,
    NIM_SET3 | OP_REF_IXD,
    NIM_SET3 | OP_REF_IXD  | ROP_A,

    /* e0 */
    NIM_SET4 | OP_REF_IXD  | ROP_B,
    NIM_SET4 | OP_REF_IXD  | ROP_C,
    NIM_SET4 | OP_REF_IXD  | ROP_D,
    NIM_SET4 | OP_REF_IXD  | ROP_E,
    NIM_SET4 | OP_REF_IXD  | ROP_H,
    NIM_SET4 | OP_REF_IXD  | ROP_L,
    NIM_SET4 | OP_REF_IXD,
    NIM_SET4 | OP_REF_IXD  | ROP_A,

    /* e8 */
    NIM_SET5 | OP_REF_IXD  | ROP_B,
    NIM_SET5 | OP_REF_IXD  | ROP_C,
    NIM_SET5 | OP_REF_IXD  | ROP_D,
    NIM_SET5 | OP_REF_IXD  | ROP_E,
    NIM_SET5 | OP_REF_IXD  | ROP_H,
    NIM_SET5 | OP_REF_IXD  | ROP_L,
    NIM_SET5 | OP_REF_IXD,
    NIM_SET5 | OP_REF_IXD  | ROP_A,

    /* f0 */
    NIM_SET6 | OP_REF_IXD  | ROP_B,
    NIM_SET6 | OP_REF_IXD  | ROP_C,
    NIM_SET6 | OP_REF_IXD  | ROP_D,
    NIM_SET6 | OP_REF_IXD  | ROP_E,
    NIM_SET6 | OP_REF_IXD  | ROP_H,
    NIM_SET6 | OP_REF_IXD  | ROP_L,
    NIM_SET6 | OP_REF_IXD,
    NIM_SET6 | OP_REF_IXD  | ROP_A,

    /* f8 */
    NIM_SET7 | OP_REF_IXD  | ROP_B,
    NIM_SET7 | OP_REF_IXD  | ROP_C,
    NIM_SET7 | OP_REF_IXD  | ROP_D,
    NIM_SET7 | OP_REF_IXD  | ROP_E,
    NIM_SET7 | OP_REF_IXD  | ROP_H,
    NIM_SET7 | OP_REF_IXD  | ROP_L,
    NIM_SET7 | OP_REF_IXD,
    NIM_SET7 | OP_REF_IXD  | ROP_A
};

const static uint32 ed_xx[] = {
    /* 00 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 08 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 10 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 18 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 20 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 28 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 30 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 38 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 40 */
    NIM_IN   | LOP_B       | ROP_REF_C,
    NIM_OUT  | LOP_REF_C   | ROP_B,
    NIM_SBC  | LOP_HL      | ROP_BC,
    NIM_LD   | LOP_REF_16  | ROP_BC,
    NIM_NEG,
    NIM_RETN,
    NIM_IM0,
    NIM_LD   | LOP_I       | ROP_A,

    /* 48 */
    NIM_IN   | LOP_C       | ROP_REF_C,
    NIM_OUT  | LOP_REF_C   | ROP_C,
    NIM_ADC  | LOP_HL      | ROP_BC,
    NIM_LD   | LOP_BC      | ROP_REF_16,
    NIM_UNKNOWN,
    NIM_RETI,
    NIM_UNKNOWN,
    NIM_LD   | LOP_R       | ROP_A,

    /* 50 */
    NIM_IN   | LOP_D       | ROP_REF_C,
    NIM_OUT  | LOP_REF_C   | ROP_D,
    NIM_SBC  | LOP_HL      | ROP_DE,
    NIM_LD   | LOP_REF_16  | ROP_DE,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_IM1,
    NIM_LD   | LOP_A       | ROP_I,

    /* 58 */
    NIM_IN   | LOP_E       | ROP_REF_C,
    NIM_OUT  | LOP_REF_C   | ROP_E,
    NIM_ADC  | LOP_HL      | ROP_DE,
    NIM_LD   | LOP_DE      | ROP_REF_16,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_IM2,
    NIM_LD   | LOP_A       | ROP_R,

    /* 60 */
    NIM_IN   | LOP_H       | ROP_REF_C,
    NIM_OUT  | LOP_REF_C   | LOP_H,
    NIM_SBC  | LOP_HL      | ROP_HL,
    NIM_LD   | LOP_REF_16  | ROP_SP,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 68 */
    NIM_IN   | LOP_L       | ROP_REF_C,
    NIM_OUT  | LOP_REF_C   | ROP_L,
    NIM_ADC  | LOP_HL      | ROP_HL,
    NIM_LD   | LOP_HL      | ROP_REF_16,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_RLD,

    /* 70 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_SBC  | LOP_HL      | ROP_SP,
    NIM_LD   | LOP_REF_16  | ROP_SP,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 78 */
    NIM_IN   | LOP_A       | ROP_REF_C,
    NIM_OUT  | LOP_REF_C   | ROP_A,
    NIM_ADC  | LOP_HL      | ROP_SP,
    NIM_LD   | LOP_SP      | ROP_REF_16,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 80 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 88 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 90 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 98 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* a0 */
    NIM_LDI,
    NIM_CPI,
    NIM_INI,
    NIM_OUTI,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* a8 */
    NIM_LDD,
    NIM_CPD,
    NIM_IND,
    NIM_OUTD,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* b0 */
    NIM_LDIR,
    NIM_CPIR,
    NIM_INIR,
    NIM_OTIR,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* b8 */
    NIM_LDDR,
    NIM_CPDR,
    NIM_INDR,
    NIM_OTDR,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* c0 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* c8 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* d0 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* d8 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* e0 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* e8 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* f0 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* f8 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN
};

static uint32 fd_xx[] = {
    /* 00 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 08 */
    NIM_UNKNOWN,
    NIM_ADD  | LOP_IY      | ROP_BC,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 10 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 18 */
    NIM_UNKNOWN,
    NIM_ADD  | LOP_IY      | ROP_DE,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 20 */
    NIM_UNKNOWN,
    NIM_LD   | LOP_IY      | ROP_IMM16,
    NIM_LD   | LOP_REF_16  | ROP_IY,
    NIM_INC  | LOP_IY,
    NIM_INC  | LOP_IYH,
    NIM_DEC  | LOP_IYH,
    NIM_LD   | LOP_IYH     | ROP_IMM8,
    NIM_UNKNOWN,

    /* 28 */
    NIM_UNKNOWN,
    NIM_ADD  | LOP_IY      | ROP_HL,
    NIM_LD   | LOP_IY      | ROP_REF_16,
    NIM_DEC  | LOP_IY,
    NIM_INC  | LOP_IYL,
    NIM_DEC  | LOP_IYL,
    NIM_LD   | LOP_IY      | ROP_IMM8,
    NIM_UNKNOWN,

    /* 30 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_INC  | LOP_REF_IYD,
    NIM_DEC  | LOP_REF_IYD,
    NIM_LD   | LOP_REF_IYD | ROP_IMM8,
    NIM_UNKNOWN,

    /* 38 */
    NIM_UNKNOWN,
    NIM_ADD  | LOP_IY      | ROP_SP,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* 40 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_LD   | LOP_B       | ROP_IYH,
    NIM_LD   | LOP_B       | ROP_IYL,
    NIM_LD   | LOP_B       | ROP_REF_IYD,
    NIM_UNKNOWN,

    /* 48 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_LD   | LOP_C       | ROP_IYH,
    NIM_LD   | LOP_C       | ROP_IYL,
    NIM_LD   | LOP_C       | ROP_REF_IYD,
    NIM_UNKNOWN,

    /* 50 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_LD   | LOP_D       | ROP_IYH,
    NIM_LD   | LOP_D       | ROP_IYL,
    NIM_LD   | LOP_D       | ROP_REF_IYD,
    NIM_UNKNOWN,

    /* 58 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_LD   | LOP_E       | ROP_IYH,
    NIM_LD   | LOP_E       | ROP_IYL,
    NIM_LD   | LOP_E       | ROP_REF_IYD,
    NIM_UNKNOWN,

    /* 60 */
    NIM_LD   | LOP_IYH     | ROP_B,
    NIM_LD   | LOP_IYH     | ROP_C,
    NIM_LD   | LOP_IYH     | ROP_D,
    NIM_LD   | LOP_IYH     | ROP_E,
    NIM_LD   | LOP_IYH     | ROP_H,
    NIM_LD   | LOP_IYH     | ROP_L,
    NIM_LD   | LOP_H       | ROP_REF_IYD,
    NIM_LD   | LOP_IYH     | ROP_A,

    /* 68 */
    NIM_LD   | LOP_IYL     | ROP_B,
    NIM_LD   | LOP_IYL     | ROP_C,
    NIM_LD   | LOP_IYL     | ROP_D,
    NIM_LD   | LOP_IYL     | ROP_E,
    NIM_LD   | LOP_IYL     | ROP_H,
    NIM_LD   | LOP_IYL     | ROP_L,
    NIM_LD   | LOP_L       | ROP_REF_IYD,
    NIM_LD   | LOP_IYL     | ROP_A,

    /* 70 */
    NIM_LD   | LOP_REF_IYD | ROP_B,
    NIM_LD   | LOP_REF_IYD | ROP_C,
    NIM_LD   | LOP_REF_IYD | ROP_D,
    NIM_LD   | LOP_REF_IYD | ROP_E,
    NIM_LD   | LOP_REF_IYD | ROP_H,
    NIM_LD   | LOP_REF_IYD | ROP_L,
    NIM_UNKNOWN,
    NIM_LD   | LOP_REF_IYD | ROP_A,

    /* 78 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_LD   | LOP_A       | ROP_IYH,
    NIM_LD   | LOP_A       | ROP_IYL,
    NIM_LD   | LOP_A       | ROP_REF_IYD,
    NIM_UNKNOWN,

    /* 80 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_ADD  | LOP_IYH,
    NIM_ADD  | LOP_IYL,
    NIM_ADD  | LOP_REF_IYD,
    NIM_UNKNOWN,

    /* 88 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_ADC  | LOP_IYH,
    NIM_ADC  | LOP_IYL,
    NIM_ADC  | LOP_REF_IYD,
    NIM_UNKNOWN,

    /* 90 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_SUB  | LOP_IYH,
    NIM_SUB  | LOP_IYL,
    NIM_SUB  | LOP_REF_IYD,
    NIM_UNKNOWN,

    /* 98 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_SBC  | LOP_IYH,
    NIM_SBC  | LOP_IYL,
    NIM_SBC  | LOP_REF_IYD,
    NIM_UNKNOWN,

    /* a0 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_AND  | LOP_IYH,
    NIM_AND  | LOP_IYL,
    NIM_AND  | LOP_REF_IYD,
    NIM_UNKNOWN,

    /* a8 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_XOR  | LOP_IYH,
    NIM_XOR  | LOP_IYL,
    NIM_XOR  | LOP_REF_IYD,
    NIM_UNKNOWN,

    /* b0 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_OR   | LOP_IYH,
    NIM_OR   | LOP_IYL,
    NIM_OR   | LOP_REF_IYD,
    NIM_UNKNOWN,

    /* b8 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_CP   | LOP_IYH,
    NIM_CP   | LOP_IYL,
    NIM_CP   | LOP_REF_IYD,
    NIM_UNKNOWN,

    /* c0 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* c8 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    0,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* d0 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* d8 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* e0 */
    NIM_UNKNOWN,
    NIM_POP  | LOP_IY,
    NIM_UNKNOWN,
    NIM_EX   | LOP_REF_SP  | ROP_IY,
    NIM_UNKNOWN,
    NIM_PUSH | LOP_IY,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* e8 */
    NIM_UNKNOWN,
    NIM_JP   | LOP_REF_IY,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* f0 */
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,

    /* f8 */
    NIM_UNKNOWN,
    NIM_LD   | LOP_SP      | ROP_IY,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN,
    NIM_UNKNOWN
};

const static uint32 fd_cb_xx[] = {
    /* 00 */
    NIM_RLC  | OP_REF_IYD  | ROP_B,
    NIM_RLC  | OP_REF_IYD  | ROP_C,
    NIM_RLC  | OP_REF_IYD  | ROP_D,
    NIM_RLC  | OP_REF_IYD  | ROP_E,
    NIM_RLC  | OP_REF_IYD  | ROP_H,
    NIM_RLC  | OP_REF_IYD  | ROP_L,
    NIM_RLC  | OP_REF_IYD,
    NIM_RLC  | OP_REF_IYD  | ROP_A,

    /* 08 */
    NIM_RRC  | OP_REF_IYD  | ROP_B,
    NIM_RRC  | OP_REF_IYD  | ROP_C,
    NIM_RRC  | OP_REF_IYD  | ROP_D,
    NIM_RRC  | OP_REF_IYD  | ROP_E,
    NIM_RRC  | OP_REF_IYD  | ROP_H,
    NIM_RRC  | OP_REF_IYD  | ROP_L,
    NIM_RRC  | OP_REF_IYD,
    NIM_RRC  | OP_REF_IYD  | ROP_A,

    /* 10 */
    NIM_RL   | OP_REF_IYD  | ROP_B,
    NIM_RL   | OP_REF_IYD  | ROP_C,
    NIM_RL   | OP_REF_IYD  | ROP_D,
    NIM_RL   | OP_REF_IYD  | ROP_E,
    NIM_RL   | OP_REF_IYD  | ROP_H,
    NIM_RL   | OP_REF_IYD  | ROP_L,
    NIM_RL   | OP_REF_IYD,
    NIM_RL   | OP_REF_IYD  | ROP_A,

    /* 18 */
    NIM_RR   | OP_REF_IYD  | ROP_B,
    NIM_RR   | OP_REF_IYD  | ROP_C,
    NIM_RR   | OP_REF_IYD  | ROP_D,
    NIM_RR   | OP_REF_IYD  | ROP_E,
    NIM_RR   | OP_REF_IYD  | ROP_H,
    NIM_RR   | OP_REF_IYD  | ROP_L,
    NIM_RR   | OP_REF_IYD,
    NIM_RR   | OP_REF_IYD  | ROP_A,

    /* 20 */
    NIM_SLA  | OP_REF_IYD  | ROP_B,
    NIM_SLA  | OP_REF_IYD  | ROP_C,
    NIM_SLA  | OP_REF_IYD  | ROP_D,
    NIM_SLA  | OP_REF_IYD  | ROP_E,
    NIM_SLA  | OP_REF_IYD  | ROP_H,
    NIM_SLA  | OP_REF_IYD  | ROP_L,
    NIM_SLA  | OP_REF_IYD,
    NIM_SLA  | OP_REF_IYD  | ROP_A,

    /* 28 */
    NIM_SRA  | OP_REF_IYD  | ROP_B,
    NIM_SRA  | OP_REF_IYD  | ROP_C,
    NIM_SRA  | OP_REF_IYD  | ROP_D,
    NIM_SRA  | OP_REF_IYD  | ROP_E,
    NIM_SRA  | OP_REF_IYD  | ROP_H,
    NIM_SRA  | OP_REF_IYD  | ROP_L,
    NIM_SRA  | OP_REF_IYD,
    NIM_SRA  | OP_REF_IYD  | ROP_A,

    /* 30 */
    NIM_SLL  | OP_REF_IYD  | ROP_B,
    NIM_SLL  | OP_REF_IYD  | ROP_C,
    NIM_SLL  | OP_REF_IYD  | ROP_D,
    NIM_SLL  | OP_REF_IYD  | ROP_E,
    NIM_SLL  | OP_REF_IYD  | ROP_H,
    NIM_SLL  | OP_REF_IYD  | ROP_L,
    NIM_SLL  | OP_REF_IYD,
    NIM_SLL  | OP_REF_IYD  | ROP_A,

    /* 38 */
    NIM_SRL  | OP_REF_IYD  | ROP_B,
    NIM_SRL  | OP_REF_IYD  | ROP_C,
    NIM_SRL  | OP_REF_IYD  | ROP_D,
    NIM_SRL  | OP_REF_IYD  | ROP_E,
    NIM_SRL  | OP_REF_IYD  | ROP_H,
    NIM_SRL  | OP_REF_IYD  | ROP_L,
    NIM_SRL  | OP_REF_IYD,
    NIM_SRL  | OP_REF_IYD  | ROP_A,

    /* 40 */
    NIM_BIT0 | OP_REF_IYD,
    NIM_BIT0 | OP_REF_IYD,
    NIM_BIT0 | OP_REF_IYD,
    NIM_BIT0 | OP_REF_IYD,
    NIM_BIT0 | OP_REF_IYD,
    NIM_BIT0 | OP_REF_IYD,
    NIM_BIT0 | OP_REF_IYD,
    NIM_BIT0 | OP_REF_IYD,

    /* 48 */
    NIM_BIT1 | OP_REF_IYD,
    NIM_BIT1 | OP_REF_IYD,
    NIM_BIT1 | OP_REF_IYD,
    NIM_BIT1 | OP_REF_IYD,
    NIM_BIT1 | OP_REF_IYD,
    NIM_BIT1 | OP_REF_IYD,
    NIM_BIT1 | OP_REF_IYD,
    NIM_BIT1 | OP_REF_IYD,

    /* 50 */
    NIM_BIT2 | OP_REF_IYD,
    NIM_BIT2 | OP_REF_IYD,
    NIM_BIT2 | OP_REF_IYD,
    NIM_BIT2 | OP_REF_IYD,
    NIM_BIT2 | OP_REF_IYD,
    NIM_BIT2 | OP_REF_IYD,
    NIM_BIT2 | OP_REF_IYD,
    NIM_BIT2 | OP_REF_IYD,

    /* 58 */
    NIM_BIT3 | OP_REF_IYD,
    NIM_BIT3 | OP_REF_IYD,
    NIM_BIT3 | OP_REF_IYD,
    NIM_BIT3 | OP_REF_IYD,
    NIM_BIT3 | OP_REF_IYD,
    NIM_BIT3 | OP_REF_IYD,
    NIM_BIT3 | OP_REF_IYD,
    NIM_BIT3 | OP_REF_IYD,

    /* 60 */
    NIM_BIT4 | OP_REF_IYD,
    NIM_BIT4 | OP_REF_IYD,
    NIM_BIT4 | OP_REF_IYD,
    NIM_BIT4 | OP_REF_IYD,
    NIM_BIT4 | OP_REF_IYD,
    NIM_BIT4 | OP_REF_IYD,
    NIM_BIT4 | OP_REF_IYD,
    NIM_BIT4 | OP_REF_IYD,

    /* 68 */
    NIM_BIT5 | OP_REF_IYD,
    NIM_BIT5 | OP_REF_IYD,
    NIM_BIT5 | OP_REF_IYD,
    NIM_BIT5 | OP_REF_IYD,
    NIM_BIT5 | OP_REF_IYD,
    NIM_BIT5 | OP_REF_IYD,
    NIM_BIT5 | OP_REF_IYD,
    NIM_BIT5 | OP_REF_IYD,

    /* 70 */
    NIM_BIT6 | OP_REF_IYD,
    NIM_BIT6 | OP_REF_IYD,
    NIM_BIT6 | OP_REF_IYD,
    NIM_BIT6 | OP_REF_IYD,
    NIM_BIT6 | OP_REF_IYD,
    NIM_BIT6 | OP_REF_IYD,
    NIM_BIT6 | OP_REF_IYD,
    NIM_BIT6 | OP_REF_IYD,

    /* 78 */
    NIM_BIT7 | OP_REF_IYD,
    NIM_BIT7 | OP_REF_IYD,
    NIM_BIT7 | OP_REF_IYD,
    NIM_BIT7 | OP_REF_IYD,
    NIM_BIT7 | OP_REF_IYD,
    NIM_BIT7 | OP_REF_IYD,
    NIM_BIT7 | OP_REF_IYD,
    NIM_BIT7 | OP_REF_IYD,

    /* 80 */
    NIM_RES0 | OP_REF_IYD  | ROP_B,
    NIM_RES0 | OP_REF_IYD  | ROP_C,
    NIM_RES0 | OP_REF_IYD  | ROP_D,
    NIM_RES0 | OP_REF_IYD  | ROP_E,
    NIM_RES0 | OP_REF_IYD  | ROP_H,
    NIM_RES0 | OP_REF_IYD  | ROP_L,
    NIM_RES0 | OP_REF_IYD,
    NIM_RES0 | OP_REF_IYD  | ROP_A,

    /* 88 */
    NIM_RES1 | OP_REF_IYD  | ROP_B,
    NIM_RES1 | OP_REF_IYD  | ROP_C,
    NIM_RES1 | OP_REF_IYD  | ROP_D,
    NIM_RES1 | OP_REF_IYD  | ROP_E,
    NIM_RES1 | OP_REF_IYD  | ROP_H,
    NIM_RES1 | OP_REF_IYD  | ROP_L,
    NIM_RES1 | OP_REF_IYD,
    NIM_RES1 | OP_REF_IYD  | ROP_A,

    /* 90 */
    NIM_RES2 | OP_REF_IYD  | ROP_B,
    NIM_RES2 | OP_REF_IYD  | ROP_C,
    NIM_RES2 | OP_REF_IYD  | ROP_D,
    NIM_RES2 | OP_REF_IYD  | ROP_E,
    NIM_RES2 | OP_REF_IYD  | ROP_H,
    NIM_RES2 | OP_REF_IYD  | ROP_L,
    NIM_RES2 | OP_REF_IYD,
    NIM_RES2 | OP_REF_IYD  | ROP_A,

    /* 98 */
    NIM_RES3 | OP_REF_IYD  | ROP_B,
    NIM_RES3 | OP_REF_IYD  | ROP_C,
    NIM_RES3 | OP_REF_IYD  | ROP_D,
    NIM_RES3 | OP_REF_IYD  | ROP_E,
    NIM_RES3 | OP_REF_IYD  | ROP_H,
    NIM_RES3 | OP_REF_IYD  | ROP_L,
    NIM_RES3 | OP_REF_IYD,
    NIM_RES3 | OP_REF_IYD  | ROP_A,

    /* a0 */
    NIM_RES4 | OP_REF_IYD  | ROP_B,
    NIM_RES4 | OP_REF_IYD  | ROP_C,
    NIM_RES4 | OP_REF_IYD  | ROP_D,
    NIM_RES4 | OP_REF_IYD  | ROP_E,
    NIM_RES4 | OP_REF_IYD  | ROP_H,
    NIM_RES4 | OP_REF_IYD  | ROP_L,
    NIM_RES4 | OP_REF_IYD,
    NIM_RES4 | OP_REF_IYD  | ROP_A,

    /* a8 */
    NIM_RES5 | OP_REF_IYD  | ROP_B,
    NIM_RES5 | OP_REF_IYD  | ROP_C,
    NIM_RES5 | OP_REF_IYD  | ROP_D,
    NIM_RES5 | OP_REF_IYD  | ROP_E,
    NIM_RES5 | OP_REF_IYD  | ROP_H,
    NIM_RES5 | OP_REF_IYD  | ROP_L,
    NIM_RES5 | OP_REF_IYD,
    NIM_RES5 | OP_REF_IYD  | ROP_A,

    /* b0 */
    NIM_RES6 | OP_REF_IYD  | ROP_B,
    NIM_RES6 | OP_REF_IYD  | ROP_C,
    NIM_RES6 | OP_REF_IYD  | ROP_D,
    NIM_RES6 | OP_REF_IYD  | ROP_E,
    NIM_RES6 | OP_REF_IYD  | ROP_H,
    NIM_RES6 | OP_REF_IYD  | ROP_L,
    NIM_RES6 | OP_REF_IYD,
    NIM_RES6 | OP_REF_IYD  | ROP_A,

    /* b8 */
    NIM_RES7 | OP_REF_IYD  | ROP_B,
    NIM_RES7 | OP_REF_IYD  | ROP_C,
    NIM_RES7 | OP_REF_IYD  | ROP_D,
    NIM_RES7 | OP_REF_IYD  | ROP_E,
    NIM_RES7 | OP_REF_IYD  | ROP_H,
    NIM_RES7 | OP_REF_IYD  | ROP_L,
    NIM_RES7 | OP_REF_IYD,
    NIM_RES7 | OP_REF_IYD  | ROP_A,

    /* c0 */
    NIM_SET0 | OP_REF_IYD  | ROP_B,
    NIM_SET0 | OP_REF_IYD  | ROP_C,
    NIM_SET0 | OP_REF_IYD  | ROP_D,
    NIM_SET0 | OP_REF_IYD  | ROP_E,
    NIM_SET0 | OP_REF_IYD  | ROP_H,
    NIM_SET0 | OP_REF_IYD  | ROP_L,
    NIM_SET0 | OP_REF_IYD,
    NIM_SET0 | OP_REF_IYD  | ROP_A,

    /* c8 */
    NIM_SET1 | OP_REF_IYD  | ROP_B,
    NIM_SET1 | OP_REF_IYD  | ROP_C,
    NIM_SET1 | OP_REF_IYD  | ROP_D,
    NIM_SET1 | OP_REF_IYD  | ROP_E,
    NIM_SET1 | OP_REF_IYD  | ROP_H,
    NIM_SET1 | OP_REF_IYD  | ROP_L,
    NIM_SET1 | OP_REF_IYD,
    NIM_SET1 | OP_REF_IYD  | ROP_A,

    /* d0 */
    NIM_SET2 | OP_REF_IYD  | ROP_B,
    NIM_SET2 | OP_REF_IYD  | ROP_C,
    NIM_SET2 | OP_REF_IYD  | ROP_D,
    NIM_SET2 | OP_REF_IYD  | ROP_E,
    NIM_SET2 | OP_REF_IYD  | ROP_H,
    NIM_SET2 | OP_REF_IYD  | ROP_L,
    NIM_SET2 | OP_REF_IYD,
    NIM_SET2 | OP_REF_IYD  | ROP_A,

    /* d8 */
    NIM_SET3 | OP_REF_IYD  | ROP_B,
    NIM_SET3 | OP_REF_IYD  | ROP_C,
    NIM_SET3 | OP_REF_IYD  | ROP_D,
    NIM_SET3 | OP_REF_IYD  | ROP_E,
    NIM_SET3 | OP_REF_IYD  | ROP_H,
    NIM_SET3 | OP_REF_IYD  | ROP_L,
    NIM_SET3 | OP_REF_IYD,
    NIM_SET3 | OP_REF_IYD  | ROP_A,

    /* e0 */
    NIM_SET4 | OP_REF_IYD  | ROP_B,
    NIM_SET4 | OP_REF_IYD  | ROP_C,
    NIM_SET4 | OP_REF_IYD  | ROP_D,
    NIM_SET4 | OP_REF_IYD  | ROP_E,
    NIM_SET4 | OP_REF_IYD  | ROP_H,
    NIM_SET4 | OP_REF_IYD  | ROP_L,
    NIM_SET4 | OP_REF_IYD,
    NIM_SET4 | OP_REF_IYD  | ROP_A,

    /* e8 */
    NIM_SET5 | OP_REF_IYD  | ROP_B,
    NIM_SET5 | OP_REF_IYD  | ROP_C,
    NIM_SET5 | OP_REF_IYD  | ROP_D,
    NIM_SET5 | OP_REF_IYD  | ROP_E,
    NIM_SET5 | OP_REF_IYD  | ROP_H,
    NIM_SET5 | OP_REF_IYD  | ROP_L,
    NIM_SET5 | OP_REF_IYD,
    NIM_SET5 | OP_REF_IYD  | ROP_A,

    /* f0 */
    NIM_SET6 | OP_REF_IYD  | ROP_B,
    NIM_SET6 | OP_REF_IYD  | ROP_C,
    NIM_SET6 | OP_REF_IYD  | ROP_D,
    NIM_SET6 | OP_REF_IYD  | ROP_E,
    NIM_SET6 | OP_REF_IYD  | ROP_H,
    NIM_SET6 | OP_REF_IYD  | ROP_L,
    NIM_SET6 | OP_REF_IYD,
    NIM_SET6 | OP_REF_IYD  | ROP_A,

    /* f8 */
    NIM_SET7 | OP_REF_IYD  | ROP_B,
    NIM_SET7 | OP_REF_IYD  | ROP_C,
    NIM_SET7 | OP_REF_IYD  | ROP_D,
    NIM_SET7 | OP_REF_IYD  | ROP_E,
    NIM_SET7 | OP_REF_IYD  | ROP_H,
    NIM_SET7 | OP_REF_IYD  | ROP_L,
    NIM_SET7 | OP_REF_IYD,
    NIM_SET7 | OP_REF_IYD  | ROP_A
};

const static uint32 *nim[] = {
    xx,
    cb_xx,
    dd_xx,
    dd_cb_xx,
    ed_xx,
    fd_xx,
    fd_cb_xx
};

const int offNim[] = {
    0,	/* xx          */
    1,	/* cb xx       */
    1,	/* dd xx       */
    3,	/* dd cb xx yy */
    1,	/* ed xx       */
    1,	/* fd xx       */
    3	/* fd cb xx yy */
};

const int offOp[] = {
    0,	/* xx          */
    1,	/* cd xx       */
    1,	/* dd xx       */
    2,	/* dd cb xx yy */
    1,	/* ed xx       */
    1,	/* fd xx       */
    2	/* fd cb xx yy */
};

/* ニーモニック文字列 */
const static char *txtNim[] = {
    "adc"   , "add"   , "and"   , "bit 0," , "bit 1," , "bit 2," , "bit 3," , "bit 4," ,
    "bit 5," , "bit 6," , "bit 7," , "call"  , "ccf"   , "cp"    , "cpd"   , "cpdr"  ,
    "cpi"   , "cpir"  , "cpl"   , "daa"   , "dec"   , "di"    , "djnz"  , "ei"    ,
    "halt"  , "ex"    , "exx"   , "im 0"  , "im 1"  , "im 2"  , "in"    , "ind"   ,
    "indr"  , "ini"   , "inir"  , "inc"   , "jp"    , "jr"    , "ld"    , "ldd"   ,
    "lddr"  , "ldi"   , "ldir"  , "neg"   , "nop"   , "or"    , "out"   , "outd"  ,
    "otdr"  , "outi"  , "otir"  , "pop"   , "push"  , "res 0," , "res 1," , "res 2," ,
    "res 3," , "res 4," , "res 5," , "res 6," , "res 7," , "ret"   , "reti"  , "retn"  ,
    "rl"    , "rla"   , "rlc"   , "rlca"  , "rld"   , "rr"    , "rra"   , "rrc"   ,
    "rrca"  , "rrd"   , "rst 00h", "rst 08h", "rst 10h", "rst 18h", "rst 20h", "rst 28h",
    "rst 30h", "rst 38h", "sbc"   , "scf"   , "set 0," , "set 1," , "set 2," , "set 3," ,
    "set 4," , "set 5," , "set 6," , "set 7," , "sla"   , "sll"   , "sra"   , "srl"   ,
    "sub"   , "xor"   , "?"
};

/* 左オペランド文字列 */
const static char *txtLop[] = {
    ""       , "B"      , "C"    , "D"      , "E"   , "H"      , "L"   , "A"   ,
    "F"      , "IXh"    , "IXl"  , "IYh"    , "IYl" , "BC"     , "DE"  , "HL"  ,
    "AF"     , "IX"     , "IY"   , "SP"     , "PC"  , "BC'"    , "DE'" , "HL'" ,
    "AF'"    , "%02xh"  , "%02x%02xh", "%d"     , "(BC)", "(DE)"   , "(HL)", "(SP)",
    "(%02xh)", "(%02x%02xh)", "(IX)" , "(IX%+d)", "(IY)", "(IY%+d)", "(C)" , "I"   ,
    "R"      , "NZ"     , "Z"    , "NC"     , "C"   , "PO"     , "PE"  ,"P"    ,
    "M"
};

/* 右オペランド文字列 */
const static char *txtRop[] = {
    ""         , ", B"      , ", C"    , ", D"      , ", E"   , ", H"      , ", L"   , ", A"   ,
    ", F"      , ", IXh"    , ", IXl"  , ", IYh"    , ", IYl" , ", BC"     , ", DE"  , ", HL"  ,
    ", AF"     , ", IX"     , ", IY"   , ", SP"     , ", PC"  , ", BC'"    , ", DE'" , ", HL'" ,
    ", AF'"    , ", %02xh"  , ", %02x%02xh", ", %d"     , ", (BC)", ", (DE)"   , ", (HL)", ", (SP)",
    ", (%02xh)", ", (%02x%02xh)", ", (IX)" , ", (IX%+d)", ", (IY)", ", (IY%+d)", ", (C)" , ", I"   ,
    ", R"
};

/*
    1命令逆アセンブルする
*/



UINT32 Cdebug_z80::DisAsm_1(UINT32 oldpc)
//void *z80disasm(char *str, uint8 *mem)
{


    //return (oldpc);
    Buffer[0] = '\0';
    char *str = Buffer;

    char LocBuffer[60];
    oldpc &= 0xffff;
    DasmAdr = oldpc;
    UINT32 pc=oldpc;
    int oper1=pCPU->get_mem(pc,8);
    int oper2=pCPU->get_mem(pc+1,8);
    int oper3=pCPU->get_mem(pc+2,8);

    //if ((oper1 != 0xd3) && (oper1 != 0xdb)) return 0;
    char format[64];
    char format2[64];
    int x;
    const uint32 *q;
    //uint8 *p;

    switch (oper1) {
        case 0xcb: x = 1;break;
        case 0xdd: x = (oper2 == 0xcb) ? 3 : 2; break;
        case 0xed: x = 4; break;
        case 0xfd: x = (oper2 == 0xcb) ? 6 : 5; break;
        default: x = 0;
    }

    //p = &mem[offOp[x]];
    uint8 p = pCPU->get_mem(pc+offOp[x],SIZE_8);
    uint8 p1 = pCPU->get_mem(pc+offOp[x]+1,SIZE_8);
    uint8 p2 = pCPU->get_mem(pc+offOp[x]+2,SIZE_8);

    //q = &nim[x][mem[offNim[x]]];
    q = &nim[x][pCPU->get_mem(pc+offNim[x],SIZE_8)];

    sprintf(format, "%s %s%s", txtNim[*q & MASK_NIM], txtLop[((*q & MASK_LOP) >> SHIFT_LOP)], txtRop[(*q & MASK_ROP) >> SHIFT_ROP]);

    //sprintf(str,"%s",format);
    //return oldpc;



    switch(*q & (MASK_LPARAM | MASK_RPARAM | MASK_PARAM)) {
    case 0:
        header(oldpc,1);
        sprintf(str,"%-16s %s",Buffer, format);
        //strcpy(str,format);
        pc= oldpc+1;// p + 1;
        break;
    case LPARAM_UINT8:
    case RPARAM_UINT8:
        header(oldpc,2);
        sprintf(format2,"%-16s %s",Buffer,format);
        //sprintf(str,"%s",format2);
        sprintf(str, format2,p1);// *(uint8 *)(p + 1));
        pc = oldpc+2;// p + 2;
        break;
    case LPARAM_INT8:
    case RPARAM_INT8:
        header(oldpc,2);
        //str = &Buffer[16];

        sprintf(format2,"%-16s %s",Buffer,format);
        //sprintf(str,"%s",format);
        sprintf(str, format2,p1);// *(int8 *)(p + 1));
        pc = oldpc+2;// p + 2;
        break;
    case LPARAM_UINT16:
    case RPARAM_UINT16:
        header(oldpc,3);
        //sprintf(str,strcat("%s ",format),Buffer,format,p2,p1);
        sprintf(format2,"%-16s %s",Buffer,format);
        sprintf(str, format2, p2, p1);
        pc = oldpc+3;// p + 3;
        break;
    case LPARAM_INT8 | RPARAM_UINT8:
        header(oldpc,3);
        //str = &Buffer[16];
        sprintf(format2,"%-16s %s",Buffer,format);
        //sprintf(str,"%s",format);
        sprintf(str, format2, p1, p2);
        pc = oldpc+3;// p + 3;
        break;
    case PARAM_INT8:
        header(oldpc,2);
        //str = &Buffer[16];
        sprintf(format2,"%-16s %s",Buffer,format);
        //sprintf(str,"%s",format);
        sprintf(str, format2, p);
        pc = oldpc+2;// p + 2;
        break;
    default:
        pc = oldpc+1;// p + 1;
    }

    NextDasmAdr = pc;
    debugged = true;
    return pc;
}

/*
    CPUの状態を文字列として出力する
*/
char *z80regs(char *str, const Z80stat *r)
{
#if 0
    char buf[32];

    sprintf(
    str,
    "%c%c%c%c%c%c(%02x) A=%02x BC=%04x DE=%04x HL=%04x SP=%04x PC=%04x %s\n"
    "%c%c%c%c%c%c(%02x) A'%02x BC'%04x DE'%04x HL'%04x IX=%04x IY=%04x %s\n",
    (r->r.f & 0x80 ? 'S': '-'),
    (r->r.f & 0x40 ? 'Z': '-'),
    (r->r.f & 0x10 ? 'H': '-'),
    (r->r.f & 0x04 ? 'P': '-'),
    (r->r.f & 0x02 ? 'N': '-'),
    (r->r.f & 0x01 ? 'C': '-'),
    r->r.f,
    r->r.a,
    r->r16.bc,
    r->r16.de,
    r->r16.hl,
    r->r16.sp,
    r->r16.pc,
    (z80disasm(buf, &r->m[r->r16.pc]), buf),
    (r->r.f_d & 0x80 ? 'S': '-'),
    (r->r.f_d & 0x40 ? 'Z': '-'),
    (r->r.f_d & 0x10 ? 'H': '-'),
    (r->r.f_d & 0x04 ? 'P': '-'),
    (r->r.f_d & 0x02 ? 'N': '-'),
    (r->r.f_d & 0x01 ? 'C': '-'),
    r->r.f_d,
    r->r.a_d,
    r->r16.bc_d,
    r->r16.de_d,
    r->r16.hl_d,
    r->r16.ix,
    r->r16.iy,
    r->r.halt ? "HALT": ""
    );
#endif
    return str;
}

/*
    Copyright 2005 ~ 2008 maruhiro
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



Cdebug_z80::Cdebug_z80(CCPU *parent)	: Cdebug(parent)
{
}
