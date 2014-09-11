/*
    Intel 80186/80188 emulator disassembler
*/
#if 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "pcxxxx.h"
#include "i80x86_dasm.h"

#define RM_R8	1
#define RM_R16	2
#define R_RM8	3
#define R_RM16	4
#define RM_SREG16	5
#define SREG_RM16	6
#define RM_IMM8	7
#define RM_IMM16	8
#define RM16	9
#define IMM8	10
#define IMM16	11
#define IMM16_IMM8	12
#define SHORT_LABEL	13
#define NEAR_LABEL	14
#define FAR_LABEL	15
#define AL_MEM	16
#define AX_MEM	17
#define MEM_AL	18
#define MEM_AX	19
#define MATH_RM_IMM_U8	20
#define MATH_RM_IMM_U16	21
#define MATH_RM_IMM_S16	22
#define SHIFT_RM_IMM8	23
#define SHIFT_RM_IMM16	24
#define SHIFT_RM_CL8	25
#define SHIFT_RM_CL16	26
#define SHIFT_RM8	27
#define SHIFT_RM16	28
#define GRP1_RM8	29
#define GRP1_RM16	30
#define GRP2_RM8	31
#define GRP2_RM16	32
#define DB	33
#define IMM8_AL	34
#define IMM8_AX	35
#define REP	36
#define R_RM16_IMM16	37
#define R_RM16_IMM8	38

struct Nim {
    const char *oper;
    int operand;
};

const struct Nim nim[] = {
    /* 0x00 */
    { "add ", RM_R8 },
    { "add ", RM_R16 },
    { "add ", R_RM8 },
    { "add ", R_RM16 },
    { "add AL,", IMM8 },
    { "add AX,", IMM16 },
    { "push ES" },
    { "pop ES" },
    { "or ", RM_R8 },
    { "or ", RM_R16 },
    { "or ", R_RM8 },
    { "or ", R_RM16 },
    { "or AL,", IMM8 },
    { "or AX,", IMM16 },
    { "push CS" },
    { NULL, DB },

    /* 0x10 */
    { "adc ", RM_R8 },
    { "adc ", RM_R16 },
    { "adc ", R_RM8 },
    { "adc ", R_RM16 },
    { "adc AL,", IMM8 },
    { "adc AX,", IMM16 },
    { "push SS", 0 },
    { "pop SS", 0 },
    { "sbb ", RM_R8 },
    { "sbb ", RM_R16 },
    { "sbb ", R_RM8 },
    { "sbb ", R_RM16 },
    { "sbb AL,", IMM8 },
    { "sbb AX,", IMM16 },
    { "push DS", 0 },
    { "pop DS", 0 },

    /* 0x20 */
    { "and ", RM_R8 },
    { "and ", RM_R16 },
    { "and ", R_RM8 },
    { "and ", R_RM16 },
    { "and AL,", IMM8 },
    { "and AX,", IMM16 },
    { "ES:" },
    { "daa" },
    { "sub ", RM_R8 },
    { "sub ", RM_R16 },
    { "sub ", R_RM8 },
    { "sub ", R_RM16 },
    { "sub AL,", IMM8 },
    { "sub AX,", IMM16 },
    { "CS:" },
    { "das" },

    /* 0x30 */
    { "xor ", RM_R8 },
    { "xor ", RM_R16 },
    { "xor ", R_RM8 },
    { "xor ", R_RM16 },
    { "xor AL,", IMM8 },
    { "xor AX,", IMM16 },
    { "SS:" },
    { "aaa" },
    { "cmp ", RM_R8 },
    { "cmp ", RM_R16 },
    { "cmp ", R_RM8 },
    { "cmp ", R_RM16 },
    { "cmp AL,", IMM8 },
    { "cmp AX,", IMM16 },
    { "DS:" },
    { "aas" },

    /* 0x40 */
    { "inc AX" },
    { "inc CX" },
    { "inc DX" },
    { "inc BX" },
    { "inc SP" },
    { "inc BP" },
    { "inc SI" },
    { "inc DI" },
    { "dec AX" },
    { "dec CX" },
    { "dec DX" },
    { "dec BX" },
    { "dec SP" },
    { "dec BP" },
    { "dec SI" },
    { "dec DI" },

    /* 0x50 */
    { "push AX" },
    { "push CX" },
    { "push DX" },
    { "push BX" },
    { "push SP" },
    { "push BP" },
    { "push SI" },
    { "push DI" },
    { "pop AX" },
    { "pop CX" },
    { "pop DX" },
    { "pop BX" },
    { "pop SP" },
    { "pop BP" },
    { "pop SI" },
    { "pop DI" },

    /* 0x60 */
    { "pusha" },
    { "popa" },
    { "bound ", R_RM16 },
    { NULL, DB },
    { NULL, DB },
    { NULL, DB },
    { NULL, DB },
    { NULL, DB },
    { "push ", IMM16 },
    { "imul ", R_RM16_IMM16 },
    { "push ", IMM8 },
    { "imul ", R_RM16_IMM8 },
    { "insb" },
    { "insw" },
    { "outsb" },
    { "outsw" },

    /* 0x70 */
    { "jo ", SHORT_LABEL },
    { "jno ", SHORT_LABEL },
    { "jb ", SHORT_LABEL },
    { "jnb ", SHORT_LABEL },
    { "jz ", SHORT_LABEL },
    { "jnz ", SHORT_LABEL },
    { "jbe ", SHORT_LABEL },
    { "ja ", SHORT_LABEL },
    { "js ", SHORT_LABEL },
    { "jns ", SHORT_LABEL },
    { "jpe ", SHORT_LABEL },
    { "jpo ", SHORT_LABEL },
    { "jl ", SHORT_LABEL },
    { "jge ", SHORT_LABEL },
    { "jle ", SHORT_LABEL },
    { "jg ", SHORT_LABEL },

    /* 0x80 */
    { NULL, MATH_RM_IMM_U8 },
    { NULL, MATH_RM_IMM_U16 },
    { NULL, MATH_RM_IMM_U8 },
    { NULL, MATH_RM_IMM_S16 },
    { "test ", R_RM8 },
    { "test ", R_RM16 },
    { "xchg ", R_RM8 },
    { "xchg ", R_RM16 },
    { "mov ", RM_R8 },
    { "mov ", RM_R16 },
    { "mov ", R_RM8 },
    { "mov ", R_RM16 },
    { "mov ", RM_SREG16 },
    { "lea ", R_RM16 },
    { "mov ", SREG_RM16 },
    { "pop ", RM16 },

    /* 0x90 */
    { "nop" },
    { "xchg AX,CX" },
    { "xchg AX,DX" },
    { "xchg AX,BX" },
    { "xchg AX,SP" },
    { "xchg AX,BP" },
    { "xchg AX,SI" },
    { "xchg AX,DI" },
    { "cbw" },
    { "cwd" },
    { "call ", FAR_LABEL },
    { "wait" },
    { "pushf" },
    { "popf" },
    { "sahf" },
    { "lahf" },

    /* 0xa0 */
    { "mov ", AL_MEM },
    { "mov ", AX_MEM },
    { "mov ", MEM_AL },
    { "mov ", MEM_AX },
    { "movsb" },
    { "movsw" },
    { "cmpsb" },
    { "cmpsw" },
    { "test AL,", IMM8 },
    { "test AX,", IMM16 },
    { "stosb" },
    { "stosw" },
    { "lodsb" },
    { "lodsw" },
    { "scasb" },
    { "scasw" },

    /* 0xb0 */
    { "mov AL,", IMM8 },
    { "mov CL,", IMM8 },
    { "mov DL,", IMM8 },
    { "mov BL,", IMM8 },
    { "mov AH,", IMM8 },
    { "mov CH,", IMM8 },
    { "mov DH,", IMM8 },
    { "mov BH,", IMM8 },
    { "mov AX,", IMM16 },
    { "mov CX,", IMM16 },
    { "mov DX,", IMM16 },
    { "mov BX,", IMM16 },
    { "mov SP,", IMM16 },
    { "mov BP,", IMM16 },
    { "mov SI,", IMM16 },
    { "mov DI,", IMM16 },

    /* 0xc0 */
    { NULL, SHIFT_RM_IMM8 },
    { NULL, SHIFT_RM_IMM16 },
    { "ret ", IMM16 },
    { "ret" },
    { "les ", R_RM16 },
    { "lds ", R_RM16 },
    { "mov ", RM_IMM8 },
    { "mov ", RM_IMM16 },
    { "enter ", IMM16_IMM8 },
    { "leave" },
    { "retf ", IMM16 },
    { "retf" },
    { "int 3" },
    { "int ", IMM8 },
    { "into" },
    { "iret" },

    /* 0xd0 */
    { NULL, SHIFT_RM8 },
    { NULL, SHIFT_RM16 },
    { NULL, SHIFT_RM_CL8 },
    { NULL, SHIFT_RM_CL16 },
    { "aam" },
    { "aad" },
    { NULL, DB },
    { "xlat" },
    { "ESC 0,", RM16 },
    { "ESC 1,", RM16 },
    { "ESC 2,", RM16 },
    { "ESC 3,", RM16 },
    { "ESC 4,", RM16 },
    { "ESC 5,", RM16 },
    { "ESC 6,", RM16 },
    { "ESC 7,", RM16 },

    /* 0xe0 */
    { "loopnz ", SHORT_LABEL },
    { "loopz ", SHORT_LABEL },
    { "loop ", SHORT_LABEL },
    { "jcxz ", SHORT_LABEL },
    { "in AL,", IMM8 },
    { "in AX,", IMM8 },
    { "out ", IMM8_AL },
    { "out ", IMM8_AX },
    { "call ", NEAR_LABEL },
    { "jmp ", NEAR_LABEL },
    { "jmp ", FAR_LABEL },
    { "jmp ", SHORT_LABEL },
    { "in AL,DX" },
    { "in AX,DX" },
    { "out DX,AL" },
    { "out DX,AX" },

    /* 0xf0 */
    { "lock" },
    { NULL, DB },
    { "repnz", REP },
    { "repz", REP },
    { "hlt" },
    { "cmc" },
    { NULL, GRP1_RM8 },
    { NULL, GRP1_RM16 },
    { "clc" },
    { "stc" },
    { "cli" },
    { "sti" },
    { "cld" },
    { "std" },
    { NULL, GRP2_RM8 },
    { NULL, GRP2_RM16 }
};

const char *reg16[] = {
    "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"
};

const char *reg8[] = {
    "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"
};

const char *sreg[] = {
    "ES", "CS", "SS", "DS", "ES", "CS", "SS", "DS"
};

const char *rm16[] = {
    "[BX+SI]", "[BX+DI]", "[BP+SI]", "[BP+DI]", "[SI]", "[DI]", "[%04x]", "[BX]",
    "[BX+SI%s]", "[BX+DI%s]", "[BP+SI%s]", "[BP+DI%s]", "[SI%s]", "[DI%s]", "[BP%s]", "[BX%s]",
    "[BX+SI%s]", "[BX+DI%s]", "[BP+SI%s]", "[BP+DI%s]", "[SI%s]", "[DI%s]", "[BP%s]", "[BX%s]",
    "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"
};

const char *rm8[] = {
    "[BX+SI]", "[BX+DI]", "[BP+SI]", "[BP+DI]", "[SI]", "[DI]", "[%04x]", "[BX]",
    "[BX+SI%s]", "[BX+DI%s]", "[BP+SI%s]", "[BP+DI%s]", "[SI%s]", "[DI%s]", "[BP%s]", "[BX%s]",
    "[BX+SI%s]", "[BX+DI%s]", "[BP+SI%s]", "[BP+DI%s]", "[SI%s]", "[DI%s]", "[BP%s]", "[BX%s]",
    "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"
};

const char *op_math[] = {
    "add", "or", "adc", "sbb", "and", "sub", "xor", "cmp"
};

const char *op_shift[] = {
    "rol", "ror", "rcl", "rcr", "shl", "shr", "???", "sar"
};

const char *op_grp1[] = {
    "test", "???", "not", "neg", "mul", "imul", "div", "idiv"
};

const char *op_grp2[] = {
    "inc", "dec", "call", "call", "jmp", "jmp", "push", "???"
};

char *Cdebug_i80x86::s8(int8 val)
{
    static char buf[8];

    if(val > 0)
        sprintf(buf, "+%02x", val);
    else
        sprintf(buf, "-%02x", -val);
    return buf;
}

char *Cdebug_i80x86::s16(int16 val)
{
    static char buf[8];

    if(val > 0)
        sprintf(buf, "+%04x", val);
    else
        sprintf(buf, "-%04x", -val);
    return buf;
}

void Cdebug_i80x86::getsegoff(const I86stat *i86, uint8 rm, uint16 *seg, uint16 *off)
{
    uint16 _ds = (i86->r16.prefix == NULL ? i86->r16.ds: *i86->r16.prefix);
    uint16 _ss = (i86->r16.prefix == NULL ? i86->r16.ss: *i86->r16.prefix);

    switch(rm) {
    case 0x00: *seg = _ds; *off = i86->r16.bx + i86->r16.si; break;
    case 0x01: *seg = _ds; *off = i86->r16.bx + i86->r16.di; break;
    case 0x02: *seg = _ss; *off = i86->r16.bp + i86->r16.si; break;
    case 0x03: *seg = _ss; *off = i86->r16.bp + i86->r16.di; break;
    case 0x04: *seg = _ds; *off = i86->r16.si; break;
    case 0x05: *seg = _ds; *off = i86->r16.di; break;
    case 0x06: *seg = _ds; *off = i80x86->i86read16(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x07: *seg = _ds; *off = i86->r16.bx; break;

    case 0x08: *seg = _ds; *off = i86->r16.bx + i86->r16.si + (int8 )i80x86->i86read8(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x09: *seg = _ds; *off = i86->r16.bx + i86->r16.di + (int8 )i80x86->i86read8(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x0a: *seg = _ss; *off = i86->r16.bp + i86->r16.si + (int8 )i80x86->i86read8(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x0b: *seg = _ss; *off = i86->r16.bp + i86->r16.di + (int8 )i80x86->i86read8(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x0c: *seg = _ds; *off = i86->r16.si + (int8 )i80x86->i86read8(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x0d: *seg = _ds; *off = i86->r16.di + (int8 )i80x86->i86read8(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x0e: *seg = _ss; *off = i86->r16.bp + (int8 )i80x86->i86read8(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x0f: *seg = _ds; *off = i86->r16.bx + (int8 )i80x86->i86read8(i86, i86->r16.cs, i86->r16.ip + 2); break;

    case 0x10: *seg = _ds; *off = i86->r16.bx + i86->r16.si + (int16 )i80x86->i86read16(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x11: *seg = _ds; *off = i86->r16.bx + i86->r16.di + (int16 )i80x86->i86read16(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x12: *seg = _ss; *off = i86->r16.bp + i86->r16.si + (int16 )i80x86->i86read16(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x13: *seg = _ss; *off = i86->r16.bp + i86->r16.di + (int16 )i80x86->i86read16(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x14: *seg = _ds; *off = i86->r16.si + (int16 )i80x86->i86read16(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x15: *seg = _ds; *off = i86->r16.di + (int16 )i80x86->i86read16(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x16: *seg = _ss; *off = i86->r16.bp + (int16 )i80x86->i86read16(i86, i86->r16.cs, i86->r16.ip + 2); break;
    case 0x17: *seg = _ds; *off = i86->r16.bx + (int16 )i80x86->i86read16(i86, i86->r16.cs, i86->r16.ip + 2); break;
    default: *seg = 0; *off = 0; break;
    }
}

char *Cdebug_i80x86::rm8val(const I86stat *i86, uint8 rm)
{
    static char buf[32];
    uint16 seg, off;

    if((rm & 0x18) == 0x18)
        strcpy(buf, "");
    else {
        getsegoff(i86, rm, &seg, &off);
        sprintf(buf, "\t\t\t%s=%02x", rm8[rm], i80x86->i86read8(i86, seg, off));
    }
    return buf;
}

char *Cdebug_i80x86::rm16val(const I86stat *i86, uint8 rm)
{
    static char buf[32];
    uint16 seg, off;

    if((rm & 0x18) == 0x18)
        strcpy(buf, "");
    else {
        getsegoff(i86, rm, &seg, &off);
        sprintf(buf, "\t\t\t%s=%04x", rm16[rm], i80x86->i86read16(i86, seg, off));
    }
    return buf;
}

UINT32 Cdebug_i80x86::DisAsm_1(UINT32 oldpc)
//void *z80disasm(char *str, uint8 *mem)
{


    //return (oldpc);
    Buffer[0] = '\0';
    char *str = Buffer;
    char LocBuffer[200];
    LocBuffer[0] = '\0';


    int seg = (oldpc>>16) &0xffff;//(oldpc >> 4) & 0xffff;
    int offset = oldpc & 0xffff;// & 0xf;
    int adr = (((int )seg << 4) + offset) & 0xfffff;
    int len = i86disasm(&LocBuffer[0], &i80x86->i86stat, seg,offset);
    sprintf(Buffer,"%04x:%04x ",i80x86->i86stat.r16.cs, i80x86->i86stat.r16.ip);
    for(int i=0;i<len;i++)
        sprintf(Buffer,"%s%02x",Buffer,(uint)pPC->Get_8(adr+i));
    sprintf(Buffer,"%s%*s%s",Buffer,13-(len<<1)," ",LocBuffer);

//    int len = i86disasm(str, &(i80x86->i86), i80x86->i86.r16.cs,i80x86->i86.r16.ip);

    //oldpc &= 0xffff;
    DasmAdr = oldpc;




    NextDasmAdr = oldpc+len;
    debugged = true;
    return NextDasmAdr;
}

int Cdebug_i80x86::i86disasm(char *buf, const I86stat *i86, uint16 seg, uint16 off)
{

    const struct Nim *n;
    int rm, reg, len, len_rm;
    char format[32];
    uint8 b0, b1;

    Buffer[0] = '\0';



    b0 = i80x86->i86read8(i86, seg, off + 0);
    n = &nim[b0];

    switch(n->operand) {
    case 0:
        strcpy(buf, n->oper);
        return 1;
    case IMM8:
        sprintf(buf, "%s%02x", n->oper, i80x86->i86read8(i86, seg, off + 1));
        return 2;
    case IMM16:
        sprintf(buf, "%s%04x", n->oper, i80x86->i86read16(i86, seg, off + 1));
        return 3;
    case IMM16_IMM8:
        sprintf(buf, "%s%04x,%02x", n->oper, i80x86->i86read16(i86, seg, off + 1), i80x86->i86read8(i86, seg, off + 3));
        return 4;
    case SHORT_LABEL:
        sprintf(buf, "%s%04x", n->oper, (off + (int8 )i80x86->i86read8(i86, seg, off + 1) + 2) & 0xffff);
        return 2;
    case NEAR_LABEL:
        sprintf(buf, "%s%04x", n->oper, (off + (int16 )i80x86->i86read16(i86, seg, off + 1) + 3) & 0xffff);
        return 3;
    case FAR_LABEL:
        sprintf(buf, "%s%04x:%04x", n->oper, i80x86->i86read16(i86, seg, off + 3), i80x86->i86read16(i86, seg, off + 1));
        return 5;
    case AL_MEM:
        sprintf(buf, "%sAL,[%04x]\t\t\t[%04x]=%02x", n->oper, i80x86->i86read16(i86, seg, off + 1), i80x86->i86read16(i86, seg, off + 1), i80x86->i86read8(i86, i86->r16.ds, i80x86->i86read16(i86, seg, off + 1)));
        return 3;
    case AX_MEM:
        sprintf(buf, "%sAX,[%04x]\t\t\t[%04x]=%04x", n->oper, i80x86->i86read16(i86, seg, off + 1), i80x86->i86read16(i86, seg, off + 1), i80x86->i86read16(i86, i86->r16.ds, i80x86->i86read16(i86, seg, off + 1)));
        return 3;
    case MEM_AL:
        sprintf(buf, "%s[%04x],AL\t\t\t[%04x]=%02x", n->oper, i80x86->i86read16(i86, seg, off + 1), i80x86->i86read16(i86, seg, off + 1), i80x86->i86read8(i86, i86->r16.ds, i80x86->i86read16(i86, seg, off + 1)));
        return 3;
    case MEM_AX:
        sprintf(buf, "%s[%04x],AX\t\t\t[%04x]=%04x", n->oper, i80x86->i86read16(i86, seg, off + 1), i80x86->i86read16(i86, seg, off + 1), i80x86->i86read16(i86, i86->r16.ds, i80x86->i86read16(i86, seg, off + 1)));
        return 3;
    case IMM8_AL:
        sprintf(buf, "%s%02x,AL", n->oper, i80x86->i86read8(i86, seg, off + 1));
        return 2;
    case IMM8_AX:
        sprintf(buf, "%s%02x,AX", n->oper, i80x86->i86read8(i86, seg, off + 1));
        return 2;
    case REP:
        switch(i80x86->i86read8(i86, seg, off + 1)) {
        case 0x6c:
        case 0x6d:
        case 0x6e:
        case 0x6f:
        case 0xa4:
        case 0xa5:
        case 0xa6:
        case 0xa7:
        case 0xaa:
        case 0xab:
        case 0xac:
        case 0xad:
        case 0xae:
        case 0xaf:
            sprintf(buf, "%s ", n->oper);
            i86disasm(buf + strlen(buf), i86, seg, off + 1);
            return 2;
        default:
            strcpy(buf, n->oper);
            return 1;
        }
    case DB:
        sprintf(buf, "DB %02x", b0);
        return 1;
    }

    b1 = i80x86->i86read8(i86, seg, off + 1);
    rm  = ((b1 & 0xc0) >> 3) | (b1 & 0x07);
    reg = (b1 >> 3) & 0x07;

    if(rm == 0x06 || (rm & 0x18) == 0x10)
        len_rm = 2;
    else if((rm & 0x18) == 0x08)
        len_rm = 1;
    else
        len_rm = 0;

    switch(n->operand) {
    case RM_R8:
        sprintf(format, "%s%s,%s%s", n->oper, rm8[rm], reg8[reg], rm8val(i86, rm));
        len = 2;
        break;
    case RM_R16:
        sprintf(format, "%s%s,%s%s", n->oper, rm16[rm], reg16[reg], rm16val(i86, rm));
        len = 2;
        break;
    case R_RM8:
        sprintf(format, "%s%s,%s%s", n->oper, reg8[reg], rm8[rm], rm8val(i86, rm));
        len = 2;
        break;
    case R_RM16:
        sprintf(format, "%s%s,%s%s", n->oper, reg16[reg], rm16[rm], rm16val(i86, rm));
        len = 2;
        break;
    case RM16:
        sprintf(format, "%s%s%s", n->oper, rm16[rm], rm16val(i86, rm));
        len = 2;
        break;
    case RM_SREG16:
        sprintf(format, "%s%s,%s%s", n->oper, rm16[rm], sreg[reg], rm16val(i86, rm));
        len = 2;
        break;
    case SREG_RM16:
        sprintf(format, "%s%s,%s%s", n->oper, sreg[reg], rm16[rm], rm16val(i86, rm));
        len = 2;
        break;
    case RM_IMM8:
        sprintf(format, "%s%s,%02x%s", n->oper, rm8[rm], i80x86->i86read8(i86, seg, off + 2 + len_rm), rm8val(i86, rm));
        len = 3;
        break;
    case RM_IMM16:
        sprintf(format, "%s%s,%04x%s", n->oper, rm16[rm], i80x86->i86read16(i86, seg, off + 2 + len_rm), rm16val(i86, rm));
        len = 4;
        break;
    case MATH_RM_IMM_U8:
        sprintf(format, "%s %s,%02x%s", op_math[reg], rm8[rm], i80x86->i86read8(i86, seg, off + 2 + len_rm), rm8val(i86, rm));
        len = 3;
        break;
    case MATH_RM_IMM_U16:
        sprintf(format, "%s %s,%04x%s", op_math[reg], rm16[rm], i80x86->i86read16(i86, seg, off + 2 + len_rm), rm16val(i86, rm));
        len = 4;
        break;
    case MATH_RM_IMM_S16:
        sprintf(format, "%s %s,%s%s", op_math[reg], rm16[rm], s8(i80x86->i86read8(i86, seg, off + 2 + len_rm)), rm16val(i86, rm));
        len = 3;
        break;
    case SHIFT_RM_IMM8:
        sprintf(format, "%s %s,%02x%s", op_shift[reg], rm8[rm], i80x86->i86read8(i86, seg, off + 2 + len_rm), rm8val(i86, rm));
        len = 3;
        break;
    case SHIFT_RM_IMM16:
        sprintf(format, "%s %s,%04x%s", op_shift[reg], rm16[rm], i80x86->i86read8(i86, seg, off + 2 + len_rm), rm16val(i86, rm));
        len = 3;
        break;
    case SHIFT_RM8:
        sprintf(format, "%s %s,1%s", op_shift[reg], rm8[rm], rm8val(i86, rm));
        len = 2;
        break;
    case SHIFT_RM16:
        sprintf(format, "%s %s,1%s", op_shift[reg], rm16[rm], rm16val(i86, rm));
        len = 2;
        break;
    case SHIFT_RM_CL8:
        sprintf(format, "%s %s,CL%s", op_shift[reg], rm8[rm], rm8val(i86, rm));
        len = 2;
        break;
    case SHIFT_RM_CL16:
        sprintf(format, "%s %s,CL%s", op_shift[reg], rm16[rm], rm16val(i86, rm));
        len = 2;
        break;
    case GRP1_RM8:
        if(reg == 0) {
            sprintf(format, "%s %s,%02x%s", op_grp1[reg], rm8[rm], i80x86->i86read8(i86, seg, off + 2 + len_rm), rm8val(i86, rm));
            len = 3;
        } else {
            sprintf(format, "%s %s%s", op_grp1[reg], rm8[rm], rm8val(i86, rm));
            len = 2;
        }
        break;
    case GRP1_RM16:
        if(reg == 0) {
            sprintf(format, "%s %s,%04x%s", op_grp1[reg], rm16[rm], i80x86->i86read16(i86, seg, off + 2 + len_rm), rm16val(i86, rm));
            len = 4;
        } else {
            sprintf(format, "%s %s%s", op_grp1[reg], rm16[rm], rm16val(i86, rm));
            len = 2;
        }
        break;
    case GRP2_RM8:
        sprintf(format, "%s %s%s", op_grp2[reg], rm8[rm], rm8val(i86, rm));
        len = 2;
        break;
    case GRP2_RM16:
        sprintf(format, "%s %s%s", op_grp2[reg], rm16[rm], rm16val(i86, rm));
        len = 2;
        break;
    case R_RM16_IMM16:
        sprintf(format, "%s%s,%s,%s%s", n->oper, reg16[reg], rm16[rm], s16(i80x86->i86read16(i86, seg, off + 2 + len_rm)), rm16val(i86, rm));
        len = 4;
        break;
    case R_RM16_IMM8:
        sprintf(format, "%s%s,%s,%s%s", n->oper, reg16[reg], rm16[rm], s8(i80x86->i86read8(i86, seg, off + 2 + len_rm)), rm16val(i86, rm));
        len = 3;
        break;
    }

    if(rm == 0x06)
        sprintf(buf, format, i80x86->i86read16(i86, seg, off + 2), i80x86->i86read16(i86, seg, off + 2));
    else if(len_rm == 2)
        sprintf(buf, format, s16(i80x86->i86read16(i86, seg, off + 2)), s16(i80x86->i86read16(i86, seg, off + 2)));
    else if(len_rm == 1)
        sprintf(buf, format, s8(i80x86->i86read8(i86, seg, off + 2)), s8(i80x86->i86read8(i86, seg, off + 2)));
    else
        strcpy(buf, format);
    return len + len_rm;
}

char *Cdebug_i80x86::i86regs(char *buf, const I86stat *i86)
{
    int len, a = i86->r16.ip;
    char disasm[64], dump[16], *p;

    len = i86disasm(disasm, i86, i86->r16.cs, i86->r16.ip);
    for(p = dump, a = i86->r16.ip; p < dump + len * 2; p += 2, a++)
        sprintf(p, "%02x", i80x86->i86read8(i86, i86->r16.cs, a));
    for(; p < dump + 6 * 2; p += 2)
        strcpy(p, "  ");

    sprintf(
    buf,
    "AX=%04x  BX=%04x  CX=%04x  DX=%04x  SP=%04x  BP=%04x  SI=%04x  DI=%04x\n"
    "DS=%04x  ES=%04x  SS=%04x  CS=%04x  IP=%04x  %s %s %s %s %s %s %s %s\n"
    "%04x:%04x %s %s\n",
    i86->r16.ax,
    i86->r16.bx,
    i86->r16.cx,
    i86->r16.dx,
    i86->r16.sp,
    i86->r16.bp,
    i86->r16.si,
    i86->r16.di,
    i86->r16.ds,
    i86->r16.es,
    i86->r16.ss,
    i86->r16.cs,
    i86->r16.ip,
    i86->r16.f & 0x0800 ? "OV": "NV",
    i86->r16.f & 0x0400 ? "DN": "UP",
    i86->r16.f & 0x0200 ? "EI": "DI",
    i86->r16.f & 0x0080 ? "NG": "PL",
    i86->r16.f & 0x0040 ? "ZR": "NZ",
    i86->r16.f & 0x0010 ? "AC": "NA",
    i86->r16.f & 0x0004 ? "PE": "PO",
    i86->r16.f & 0x0001 ? "CY": "NC",
    i86->r16.cs, i86->r16.ip, dump, disasm
    );
    return buf;
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

#endif
