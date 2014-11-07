#if 1

/** 6502 Disassembler *****************************************/
/**                                                          **/
/**                        dasm6502.c                        **/
/**                                                          **/
/** This file contains the source of a portable disassembler **/
/** for the 6502 CPU.                                        **/
/**                                                          **/
/** Copyright (C) Marat Fayzullin 1996                       **/
/**               Alex Krasivsky  1996                       **/
/**     You are not allowed to distribute this software      **/
/**     commercially. Please, notify me, if you make any     **/
/**     changes to this file.                                **/
/**************************************************************/


#include "m6502_dasm.h"
#include "m6502.h"

enum { Ac=0,Il,Im,Ab,Zp,Zx,Zy,Ax,Ay,Rl,Ix,Iy,In,No };
    /* These are the addressation methods used by 6502 CPU.   */


static char *MN[] =
{
  "adc","and","asl","bcc","bcs","beq","bit","bmi",
  "bne","bpl","brk","bvc","bvs","clc","cld","cli",
  "clv","cmp","cpx","cpy","dec","dex","dey","inx",
  "iny","eor","inc","jmp","jsr","lda","nop","ldx",
  "ldy","lsr","ora","pha","php","pla","plp","rol",
  "ror","rti","rts","sbc","sta","stx","sty","sec",
  "sed","sei","tax","tay","txa","tya","tsx","txs"
};

static BYTE AD[512] =
{
  10,Il, 34,Ix, No,No, No,No, No,No, 34,Zp,  2,Zp, No,No,
  36,Il, 34,Im,  2,Ac, No,No, No,No, 34,Ab,  2,Ab, No,No,
   9,Rl, 34,Iy, No,No, No,No, No,No, 34,Zx,  2,Zx, No,No,
  13,Il, 34,Ay, No,No, No,No, No,No, 34,Ax,  2,Ax, No,No,
  28,Ab,  1,Ix, No,No, No,No,  6,Zp,  1,Zp, 39,Zp, No,No,
  38,Il,  1,Im, 39,Ac, No,No,  6,Ab,  1,Ab, 39,Ab, No,No,
   7,Rl,  1,Iy, No,No, No,No, No,No,  1,Zx, 39,Zx, No,No,
  47,Il,  1,Ay, No,No, No,No, No,No,  1,Ax, 39,Ax, No,No,
  41,Il, 25,Ix, No,No, No,No, No,No, 25,Zp, 33,Zp, No,No,
  35,Il, 25,Im, 33,Ac, No,No, 27,Ab, 25,Ab, 33,Ab, No,No,
  11,Rl, 25,Iy, No,No, No,No, No,No, 25,Zx, 33,Zx, No,No,
  15,Il, 25,Ay, No,No, No,No, No,No, 25,Ax, 33,Ax, No,No,
  42,Il,  0,Ix, No,No, No,No, No,No,  0,Zp, 40,Zp, No,No,
  37,Il,  0,Im, 40,Ac, No,No, 27,In,  0,Ab, 40,Ab, No,No,
  12,Rl,  0,Iy, No,No, No,No, No,No,  0,Zx, 40,Zx, No,No,
  49,Il,  0,Ay, No,No, No,No, No,No,  0,Ax, 40,Ax, No,No,
  No,No, 44,Ix, No,No, No,No, 46,Zp, 44,Zp, 45,Zp, No,No,
  22,Il, No,No, 52,Il, No,No, 46,Ab, 44,Ab, 45,Ab, No,No,
   3,Rl, 44,Iy, No,No, No,No, 46,Zx, 44,Zx, 45,Zy, No,No,
  53,Il, 44,Ay, 55,Il, No,No, No,No, 44,Ax, No,No, No,No,
  32,Im, 29,Ix, 31,Im, No,No, 32,Zp, 29,Zp, 31,Zp, No,No,
  51,Il, 29,Im, 50,Il, No,No, 32,Ab, 29,Ab, 31,Ab, No,No,
   4,Rl, 29,Iy, No,No, No,No, 32,Zx, 29,Zx, 31,Zy, No,No,
  16,Il, 29,Ay, 54,Il, No,No, 32,Ax, 29,Ax, 31,Ay, No,No,
  19,Im, 17,Ix, No,No, No,No, 19,Zp, 17,Zp, 20,Zp, No,No,
  24,Il, 17,Im, 21,Il, No,No, 19,Ab, 17,Ab, 20,Ab, No,No,
   8,Rl, 17,Iy, No,No, No,No, No,No, 17,Zx, 20,Zx, No,No,
  14,Il, 17,Ay, No,No, No,No, No,No, 17,Ax, 20,Ax, No,No,
  18,Im, 43,Ix, No,No, No,No, 18,Zp, 43,Zp, 26,Zp, No,No,
  23,Il, 43,Im, 30,Il, No,No, 18,Ab, 43,Ab, 26,Ab, No,No,
   5,Rl, 43,Iy, No,No, No,No, No,No, 43,Zx, 26,Zx, No,No,
  48,Il, 43,Ay, No,No, No,No, No,No, 43,Ax, 26,Ax, No,No
};

/** DAsm() ****************************************************/
/** This function will disassemble a single command and      **/
/** return the number of bytes disassembled.                 **/
/**************************************************************/

quint32 Cdebug_m6502::DisAsm_1(quint32 pc) {
    unsigned short old_pc;
    int i, j;
    char data[1024];
    char str[1024];


        for (j = 0; j < 16;j++)
            data[j] = pCPU->get_mem(pc + j,8);
        old_pc = pc;
        pc += DasmOpe(str, (unsigned char*)data,pc);
        sprintf(Buffer,"%06X: %s", old_pc, str);

        DasmAdr = old_pc;

        NextDasmAdr = pc;
        debugged = true;

        return NextDasmAdr;

}

Cdebug_m6502::Cdebug_m6502(CCPU *parent)	: Cdebug(parent)
{
}

int Cdebug_m6502::DasmOpe(char *S, quint8 *A, unsigned long PC)
{
    BYTE *B,J;
    int OP;

  B=A;OP=(*B++)*2;

  switch(AD[OP+1])
  {
    case Ac: sprintf(S,"%s a",MN[AD[OP]]);break;
    case Il: sprintf(S,"%s",MN[AD[OP]]);break;

    case Rl: J=*B++;PC+=2+((J<0x80)? J:(J-256));
             sprintf(S,"%s $%4X",MN[AD[OP]],PC);break;

    case Im: sprintf(S,"%s #$%02X",MN[AD[OP]],*B++);break;
    case Zp: sprintf(S,"%s $%02X",MN[AD[OP]],*B++);break;
    case Zx: sprintf(S,"%s $%02X,x",MN[AD[OP]],*B++);break;
    case Zy: sprintf(S,"%s $%02X,y",MN[AD[OP]],*B++);break;
    case Ix: sprintf(S,"%s ($%02X,x)",MN[AD[OP]],*B++);break;
    case Iy: sprintf(S,"%s ($%02X),y",MN[AD[OP]],*B++);break;

    case Ab: sprintf(S,"%s $%04X",MN[AD[OP]],B[1]*256+B[0]);B+=2;break;
    case Ax: sprintf(S,"%s $%04X,x",MN[AD[OP]],B[1]*256+B[0]);B+=2;break;
    case Ay: sprintf(S,"%s $%04X,y",MN[AD[OP]],B[1]*256+B[0]);B+=2;break;
    case In: sprintf(S,"%s ($%04X)",MN[AD[OP]],B[1]*256+B[0]);B+=2;break;

    default: sprintf(S,".db $%02X\t\t; <Invalid OPcode>",OP/2);
  }
  return(B-A);
}


#endif
