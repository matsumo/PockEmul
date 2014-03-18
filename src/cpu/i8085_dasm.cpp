/*
    Intel 80186/80188 emulator disassembler
*/
#if 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "Debug.h"
#include "pcxxxx.h"
#include "i8085.h"

const char    *Cdebug_i8085::Op_Code[256]   =
   {
/*    1           2           3           4                                 */
    "nop",      "lxi\tb,",  "stax\tb",  "inx\tb",
    "inr\tb",   "dcr\tb",   "mvi\tb,",  "rlc",
    "???",      "dad\tb",   "ldax\tb",  "dcx\tb",
    "inr\tc",   "dcr\tc",   "mvi\tc,",  "rrc",
    "???",      "lxi\td,",  "stax\td",  "inx\td",
    "inr\td",   "dcr\td",   "mvi\td,",  "ral",
    "???",      "dad\td",   "ldax\td",  "dcx\td",
    "inr\te",   "dcr\te",   "mvi\te,",  "rar",
    "rim",      "lxi\th,",  "shld\t",   "inx\th",
    "inr\th",   "dcr\th",   "mvi\th,",  "daa",
    "???",      "dad\th",   "lhld\t",   "dcx\th",
    "inr\tl",   "dcr\tl",   "mvi\tl,",  "cma",
    "sim",      "lxi\tsp,", "sta\t",    "inx\tsp",
    "inr\tm",   "dcr\tm",   "mvi\tm,",  "stc",
    "???",      "dad\tsp",  "lda\t",    "dcx\tsp",
    "inr\ta",   "dcr\ta",   "mvi\ta,",  "cmc\ta,",
/*    0           1           2           3                             */
    "mov\tb,b", "mov\tb,c", "mov\tb,d", "mov\tb,e",
    "mov\tb,h", "mov\tb,l", "mov\tb,m", "mov\tb,a",
    "mov\tc,b", "mov\tc,c", "mov\tc,d", "mov\tc,e",
    "mov\tc,h", "mov\tc,l", "mov\tc,m", "mov\tc,a",
    "mov\td,b", "mov\td,c", "mov\td,d", "mov\td,e",
    "mov\td,h", "mov\td,l", "mov\td,m", "mov\td,a",
    "mov\te,b", "mov\te,c", "mov\te,d", "mov\te,e",
    "mov\te,h", "mov\te,l", "mov\te,m", "mov\te,a",
    "mov\th,b", "mov\th,c", "mov\th,d", "mov\th,e",
    "mov\th,h", "mov\th,l", "mov\th,m", "mov\th,a",
    "mov\tl,b", "mov\tl,c", "mov\tl,d", "mov\tl,e",
    "mov\tl,h", "mov\tl,l", "mov\tl,m", "mov\tl,a",
    "mov\tm,b", "mov\tm,c", "mov\tm,d", "mov\tm,e",
    "mov\tm,h", "mov\tm,l", "hlt\t",    "mov\tm,a",
    "mov\ta,b", "mov\ta,c", "mov\ta,d", "mov\ta,e",
    "mov\ta,h", "mov\ta,l", "mov\ta,m", "mov\ta,a",
/*    0           1           2           3                             */
    "add\tb",   "add\tc",   "add\td",   "add\te",
    "add\th",   "add\tl",   "add\tm",   "add\ta",
    "adc\tb",   "adc\tc",   "adc\td",   "adc\te",
    "adc\th",   "adc\tl",   "adc\tm",   "adc\ta",
    "sub\tb",   "sub\tc",   "sub\td",   "sub\te",
    "sub\th",   "sub\tl",   "sub\tm",   "sub\ta",
    "sbb\tb",   "sbb\tc",   "sbb\td",   "sbb\te",
    "sbb\th",   "sbb\tl",   "sbb\tm",   "sbb\ta",
    "ana\tb",   "ana\tc",   "ana\td",   "ana\te",
    "ana\th",   "ana\tl",   "ana\tm",   "ana\ta",
    "xra\tb",   "xra\tc",   "xra\td",   "xra\te",
    "xra\th",   "xra\tl",   "xra\tm",   "xra\ta",
    "ora\tb",   "ora\tc",   "ora\td",   "ora\te",
    "ora\th",   "ora\tl",   "ora\tm",   "ora\ta",
    "cmp\tb",   "cmp\tc",   "cmp\td",   "cmp\te",
    "cmp\th",   "cmp\tl",   "cmp\tm",   "cmp\ta",
/*    0           1           2           3                             */
    "rnz",      "pop\tb",   "jnz\t",    "jmp\t",
    "cnz",      "push\tb",  "adi\t",    "rst\t0",
    "rz",       "ret",      "jz\t",     "???",
    "cz",       "call\t",   "aci\t",    "rst\t1",
    "rnc",      "pop\td",   "jnc\t",    "out\t",
    "cnc",      "push\td",  "sui\t",    "rst\t2",
    "rc",       "???",      "jc\t",     "in\t",
    "cc",       "???",      "sbi\t",    "rst\t3",
    "rpo",      "pop\th",   "jpo\t",    "xthl",
    "cpo",      "push\th",  "ani\t",    "rst\t4",
    "rpe",      "pchl",     "jpe\t",    "xchg",
    "cpe",      "???",      "xri\t",    "rst\t5",
    "rp",       "pop\tpsw", "jp\t",     "di",
    "cp",       "push\tpsw","ori\t",    "rst\t6",
    "rm",       "sphl",     "jm\t",     "ei",
    "cm",       "???",      "cpi\t",    "rst\t7"
   };

char    Cdebug_i8085::Op_Code_Size[256]  =
   {
/*  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F           */
    1,  3,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  2,  1,  /* 00   */
    1,  3,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  2,  1,  /* 10   */
    1,  3,  3,  1,  1,  1,  2,  1,  1,  1,  3,  1,  1,  1,  2,  1,  /* 20   */
    1,  3,  3,  1,  1,  1,  2,  1,  1,  1,  3,  1,  1,  1,  2,  1,  /* 30   */

    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 40   */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 50   */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 60   */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 70   */

    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 80   */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* 90   */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* A0   */
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  /* B0   */

    1,  1,  3,  3,  1,  1,  2,  1,  1,  1,  3,  1,  3,  3,  2,  1,  /* C0   */
    1,  1,  3,  2,  3,  1,  2,  1,  1,  1,  3,  2,  3,  1,  2,  1,  /* D0   */
    1,  1,  3,  1,  3,  1,  2,  1,  1,  1,  3,  1,  3,  1,  2,  1,  /* E0   */
    1,  1,  3,  1,  3,  1,  2,  1,  1,  1,  3,  1,  3,  1,  2,  1   /* F0   */
   };


UINT32 Cdebug_i8085::DisAsm_1(UINT32 oldpc)
//void *z80disasm(char *str, uint8 *mem)
{
    //return (oldpc);
    Buffer[0] = '\0';
    char *str = Buffer;

    oldpc &= 0xffff;
    DasmAdr = oldpc;
    UINT32 pc=oldpc;
    quint8 op = pPC->Get_8(pc);

    Buffer[0] = '\0';
    char LocBuffer[200];
    LocBuffer[0] = '\0';
    int len =1;

    sprintf(Buffer," %06X:%02X",pc,op);
    sprintf(LocBuffer, "%s", Op_Code[ op ] );

   /*  Two byte opcode are followed by a byte parameter.
   */
    if ( Op_Code_Size[ op ]  ==  1 )
       {
        sprintf(Buffer,"%s      ",Buffer);
    }
           if ( Op_Code_Size[ op ]  ==  2 )
              {
               quint8 i = pPC->Get_8(pc+1);
               sprintf(Buffer,"%s %02X   ",Buffer,i);
               if( i < 0xA0 )
                   sprintf( LocBuffer, "%s%02Xh", LocBuffer,i );
               else    sprintf( LocBuffer, "%s0%02Xh", LocBuffer,i );
               len++;
              }

   /*  Three byte opcodes are followed by two bytes in reverse order.
   */
           if ( Op_Code_Size[ op ]  ==  3 )
              {
               quint8 i = pPC->Get_8(pc+1);
               quint8 j = pPC->Get_8(pc+2);
               sprintf(Buffer,"%s %02X %02X",Buffer,i,j);
               sprintf( LocBuffer, "%s%02X%02X",LocBuffer, j,i );
               len++;
               len++;
              }



sprintf(Buffer,"%s\t%s",Buffer,LocBuffer);
    DasmAdr = oldpc;
    NextDasmAdr = oldpc+len;
    debugged = true;
    return NextDasmAdr;
}



char *Cdebug_i8085::i85regs(char *buf, const I85stat *i86)
{

#if 0
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
#endif
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

