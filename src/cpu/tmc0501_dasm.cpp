#include <QDebug>

#include "tmc0501_dasm.h"
#include "tmc0501.h"
#include "pcxxxx.h"

// disassembly output macro
#define	DIS(...)	sprintf(_tmc0501DASM, __VA_ARGS__)
char _tmc0501DASM[2000];

void Cdebug_tmc0501::disasm (unsigned addr, unsigned opcode) {
// I11..8
static const char *mask[16] = {
  "<flag>",
  "ALL",
  "DPT",
  "DPT", // 1",
  "DPT", // C",
  "LLSD", // 1",
  "EXP",
  "EXP", // 1",
  "<keyboard>",
  "MANT",
  "<wait>",
  "MLSD", // 5",
  "MAEX",
  "MLSD", // 1",
  "MMSD", // 1",
  "MAEX", // 1"
};
static const char *N[16] = {
  "?",
  "0",
  "0",
  "1",
  "xC",
  "1",
  "0",
  "1",
  "?",
  "0",
  "?",
  "5",
  "0",
  "1",
  "1",
  "1"
};
// I7..I3
static const char *alu[32] = {
  //op  out,X,Y
  "ADD\t%s.%s,A,#%s", // N
  "SUB\t%s.%s,A,#%s", // N
  "MOV\t%s.%s,B", // N
  "NEG\t%s.%s,B", // N
  "ADD\t%s.%s,C,#%s", // N
  "SUB\t%s.%s,C,#%s", // N
  "MOV\t%s.%s,D", // N
  "NEG\t%s.%s,D", // N
  "SHL\t%s.%s,A",
  "SHR\t%s.%s,A",
  "SHL\t%s.%s,B",
  "SHR\t%s.%s,B",
  "SHL\t%s.%s,C",
  "SHR\t%s.%s,C",
  "SHL\t%s.%s,D",
  "SHR\t%s.%s,D",
  "ADD\t%s.%s,A,B",
  "SUB\t%s.%s,A,B",
  "ADD\t%s.%s,C,B",
  "SUB\t%s.%s,C,B",
  "ADD\t%s.%s,C,D",
  "SUB\t%s.%s,C,D",
  "ADD\t%s.%s,A,D",
  "SUB\t%s.%s,A,D",
  "ADD\t%s.%s,A,const",
  "SUB\t%s.%s,A,const",
  "MOV\t%s.%s,#%s\t; io read",
  "MOV\t%s.%s,#-%s",
  "ADD\t%s.%s,C,const",
  "SUB\t%s.%s,C,const",
  "MOV\t%s.%s,R5",
  "MOV\t%s.%s,-R5"
};
// I2..0
static const char *sum[8] = {
  "A",
  "IO",
  "AxB",
  "B",
  "C",
  "CxD",
  "D",
  "AxE"
};
// 0 0000 .... ____
// I7..4 = bit number
static const char *flag[16] = {
  "TST\tfA[%u]",
  "SET\tfA[%u]",
  "CLR\tfA[%u]",
  "INV\tfA[%u]",
  "XCH\tfA[%u],fB[%u]",
  "SET\tKR[%u]",
  "MOV\tfA[%u],fB[%u]", // B->A
  "MOV\tfA[1..4],R5",
  "TST\tfB[%u]",
  "SET\tfB[%u]",
  "CLR\tfB[%u]",
  "INV\tfB[%u]",
  "CMP\tfA[%u],fB[%u]",
  "CLR\tKR[%u]",
  "MOV\tfB[%u],fA[%u]", // A->B
  "MOV\tfB[1..4],R5"
};
// 0 1010 xxxx ____
static const char *wait[16] = {
  "WAIT\tD%u",
  "CLR\tIDL",
  "CLR\tfA",
  "WAIT\tBUSY\t; N.C.",
  "INC\tKR",
  "TST\tKR[%u]",
  "FLGR5",
  "MOV\tR5,#%u",
  "xch\tKR[4..7],R5", // KR<->R5 -> see below
  "SET\tIDL",
  "CLR\tfB",
  "TST\tBUSY\t; N.C.",
  "MOV\tKR,EXT[4..15]", // 0..3 are zeroed ??
  "XCH\tKR,SR",
  "lib", // LIB instructions -> see below
  "MOV"
};
// 0 1010 ____ 1000
static const char *prn[16] = {
  "MOV\tR5,KR[4..7]",
  "MOV\tKR[4..7],R5",
  "IN\tCRD",
  "OUT\tCRD",
  "CRD_OFF",
  "CRD_READ",
  "OUT\tPRT",
  "OUT\tPRT_FUNC",
  "PRT_CLEAR",
  "PRT_STEP",
  "PRT_PRINT",
  "PRT_FEED",
  "CRD_WRITE",
  "??.xD\t\t; no-op??",
  "??.xE\t\t; set COND??",
  "RAM_OP"
};
static const char *no_op[16] = {
  "IN\tLIB",
  "OUT\tLIB_PC",
  "IN\tLIB_PC",
  "IN\tLIB_HIGH",
  "NOP.x4",
  "NOP.x5",
  "NOP.x6",
  "NOP.x7",
  "NOP.x8",
  "NOP.x9",
  "NOP.xA",
  "NOP.xB",
  "NOP.xC",
  "NOP.xD",
  "NOP.xE",
  "NOP.xF",
};
// IRG bity
#define        IRG_BRANCH        0x1000

  if (opcode & IRG_BRANCH) {
    unsigned dest = addr;
    // branch
    if (opcode & 0x0001)
      dest -= (opcode >> 1) & 0x03FF;
    else
      dest += (opcode >> 1) & 0x03FF;
    //DIS ("BRA%c\t%c%d\t;%04X", (opcode&0x0800) ? '1' : '0', (opcode & 0x0001) ? '-' : '+', (opcode >> 1) & 0x03FF, dest);
    DIS ("BRA%c\t%04X", (opcode&0x0800) ? '1' : '0', dest);
    if ((opcode & 0x17FF) == 0x1002)
      DIS ("\t; clear COND");
    return;
  }
  switch ((opcode>>8)&0x0F) {
  // special instructions
  case 0x00:
    // flag operations
    //DIS ("flag\tst=%d\t%s", (opcode>>4)&0x0F, flag[opcode&0x0F]);
    DIS (flag[opcode&0x0F], (opcode >> 4) & 0x0F, (opcode >> 4) & 0x0F);
    if (opcode == 0x0015)
      DIS ("\t; PREG");
    break;
  case 0x08:
    // keyboard
    DIS ("KEY\t%02X", opcode&0xFF);
    break;
  case 0x0A:
    // wait operations
    if ((opcode&0x0F) == 0x08) { // XCH KR,R5
      DIS (prn[(opcode>>4)&0x0F]);
    } else
    if ((opcode&0x0F) == 0x06) { // FLGR5
      DIS ("MOV\tR5,%s[1..4]", (opcode&0xF0)?"fB":"fA");
    } else
    if ((opcode&0x0F) == 0x0F) { // STO/RCL
      if (opcode & 0x0010)
        DIS ("RCL");
      else
        DIS ("STO");
    } else
    if ((opcode&0x0F) == 0x0E) { // LIB
      DIS (no_op[(opcode>>4)&0x0F]);
    } else
    {
      DIS (wait[opcode&0x0F], (opcode>>4)&0x0F);
    }
    break;
  // generic instructions
  default:
    // generic ALU decoding
    if (sum[opcode&0x07][1] == 'x' && (opcode & 0xF0) == 0xD0)
      DIS ("XCH\t%c.%s,%c", sum[opcode&0x07][0], mask[(opcode>>8)&0x0F], sum[opcode&0x07][2]);
    else
      DIS (alu[(opcode>>3)&0x1F], sum[opcode&0x07], mask[(opcode>>8)&0x0F], N[(opcode>>8)&0x0F]);
    if ((opcode & 0xF0) != 0x00 && (opcode & 0xF0) != 0x20 && (opcode & 0xF0) != 0xD0 && *N[(opcode>>8)&0x0F] != '0')
      DIS ("|#%s", N[(opcode>>8)&0x0F]);
    break;
  }
}


UINT32 Cdebug_tmc0501::DisAsm_1(UINT32 oldpc)
//void *z80disasm(char *str, uint8 *mem)
{
    //return (oldpc);
    Buffer[0] = '\0';
    char *str = Buffer;

    oldpc &= 0xffff;
    DasmAdr = oldpc;
    WORD _op = pCPU->pPC->Get_16(DasmAdr<<1);

    Buffer[0] = '\0';

    disasm(DasmAdr , _op);
    sprintf(Buffer," %04X:%04X %s",DasmAdr ,_op,_tmc0501DASM);

    NextDasmAdr = DasmAdr+1;
    debugged = true;
    return NextDasmAdr;
}

Cdebug_tmc0501::Cdebug_tmc0501(CCPU *parent)	: Cdebug(parent)
{
}

