#if 1
/*
This CPU core is based on documentations works done by:
- Piotr Piatek ( http://www.pisi.com.pl/piotr433/fx8000ee.htm )
*/
#include <QDebug>

#include "upd1007.h"
#include "ui/cregsz80widget.h"
#include "pcxxxx.h"
#include "Inter.h"
#include "upd1007d.h"

#define Lo(x) (x&0xff)
#define Hi(x) ((x>>8)&0xff)

/* status bits of the Flag Register F */
#define    C_bit	0x80
#define    H_bit	0x40
#define    V_bit	0x20
#define    NZ_bit	0x10
#define    UZ_bit	0x08
#define    LZ_bit	0x04

const BYTE CUPD1007::cc[8] = {0x00, 0x00, LZ_bit, UZ_bit, NZ_bit, V_bit, H_bit, C_bit };

const BYTE CUPD1007::INT_enable[3] = { 0x10, 0x20, 0x40 };

/* bits of the IF register */
const BYTE CUPD1007::INT_serv[3] = { 0x20, 0x40, 0x80 };
const BYTE CUPD1007::INT_input[3]= { 0x02, 0x04, 0x08 };
#define EN2_bit	0x10
#define EN1_bit 0x01

/* bits of the LCD control port in the immediate operand of the LDL /STL
  instructions */
#define     VDD2_bit 0x80;	/* port EN1 controlled by bit 0 of the IF register */
#define     OP_bit	 0x20;
#define     CE3_bit	 0x04;
#define     CE2_bit	 0x02;
#define     CE1_bit	 0x01;
#define     LCDCE	 CE1_bit;

#define RM(info,addr)  ((info->iereg & 0x03) ? 0x00*(addr)+0xFF : info->pPC->Get_8(addr))
#define WM(info,addr,value) { UINT32 _a = addr; UINT8 _v=value;if ((info->iereg & 0x03)== 0x00) info->pPC->Set_8(_a,_v);}

CUPD1007::CUPD1007(CPObject *parent,QString rom0fn):CCPU(parent) {


    pDEBUG	= new Cdebug_upd1007(parent);
    regwidget = (CregCPU*) new Cregsz80Widget(0,this);
    fn_log="upd1007.log";

    QFile file;
    file.setFileName(rom0fn);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    in.readRawData ((char *) &rom0,ROM0SIZE*3 );

    memset((char*)&reginfo,0,sizeof(reginfo));
    reginfo.pPC = pPC;
}

CUPD1007::~CUPD1007() {

}

bool CUPD1007::init()
{

    Check_Log();
    pDEBUG->init();


    Reset();

    return true;
}

bool CUPD1007::exit()
{

    return true;
}

void CUPD1007::step()
{



//    { complete an optional I/O device write }
//        if procptr <> nil then
//        begin
//          Proc1(procptr);
//          procptr := nil;
//        end {if};

        reginfo.cycles = 0;
        if (reginfo.CpuSleep)
            addState(&reginfo,6);
        else
        {
//    { interrupts }
          int i = 0;		//{ a FOR loop cannot be used here, because the value }
          while (i < 3)	//{ of the control variable is used outside the loop }
          {
//    { is there a pending interrupt request of higher priority than currently
//      serviced? }
            if ((reginfo.ifreg & INT_serv[i]) != 0)
            {
              i = 3;
            }
            else if (reginfo.irqcnt[i] > 0)
//    { handle an interrupt }
            {
              reginfo.ifreg |= INT_serv[i];
              Call (&reginfo,reginfo.mr[126-i], reginfo.mr[62-i]);
              addState(&reginfo,16);
              break;
            }
            else
//    { proceed with the next interrupt }
            i++;
          }
//    { execute an instruction if there wasn't an interrupt to handle }
          if (i == 3) ExecInstr();
        }

//    { update the interrupt request counters }
        for (int i=0;i<3;i++)
        {
          if (reginfo.irqcnt[i] < -INT_LATENCY) reginfo.irqcnt[i] = -INT_LATENCY;
          else
              if (reginfo.irqcnt[i] < 0)
                  reginfo.irqcnt[i]+= reginfo.cycles;
        }

}

void CUPD1007::Reset()
{

    reginfo.pc = 0x0000;
    reginfo.koreg =reginfo.kireg=reginfo.asreg=reginfo.flag=reginfo.iereg=reginfo.ifreg=0;
    reginfo.irqcnt[0] = 0;
    reginfo.irqcnt[1] = 0;
    reginfo.irqcnt[2] = 0;
    reginfo.acycles = 0;
    reginfo.CpuSleep = false;
    halt = false;

    _refState = pPC->pTIMER->state;
    reginfo.lcdctrl = 0;
}

void CUPD1007::Load_Internal(QXmlStreamReader *)
{

}

void CUPD1007::save_internal(QXmlStreamWriter *)
{

}

UINT32 CUPD1007::get_PC()
{
    return reginfo.pc;
}

void CUPD1007::Regs_Info(UINT8 Type)
{
    sprintf(Regs_String,"EMPTY");

    switch(Type)
    {
    case 0:			// Monitor Registers Dialog
        sprintf(
                    Regs_String,
                    "PC:%04X KO:%02X KI:%02X IE:%02X IF:%02X AS:%02X %s%s%s%s%s%s ",
                    reginfo.pc, reginfo.koreg,reginfo.kireg,
                    reginfo.iereg,reginfo.ifreg,reginfo.asreg,
                    reginfo.flag & C_bit  ? "C " :"NC",
                    reginfo.flag & V_bit  ? "V " :"NV",
                    reginfo.flag & UZ_bit ? "UZ ":"NUZ",
                    reginfo.flag & H_bit  ? "H " :"NH",
                    reginfo.flag & NZ_bit ? "NZ" :"Z ",
                    reginfo.flag & LZ_bit ? "LZ ":"NLZ"
                    );
        break;
    case 1:			// Log File
        sprintf(
                    Regs_String,
                    "PC:%04X KO:%02X KI:%02X IE:%02X IF:%02X AS:%02X %s%s%s%s%s%s ",
                    reginfo.pc, reginfo.koreg,reginfo.kireg,
                    reginfo.iereg,reginfo.ifreg,reginfo.asreg,
                    reginfo.flag & C_bit  ? "C " :"NC",
                    reginfo.flag & V_bit  ? "V " :"NV",
                    reginfo.flag & UZ_bit ? "UZ ":"NUZ",
                    reginfo.flag & H_bit  ? "H " :"NH",
                    reginfo.flag & NZ_bit ? "NZ" :"Z ",
                    reginfo.flag & LZ_bit ? "LZ ":"NLZ"
                    );

        for (int i=0;i<0x80;i++)
            sprintf(Regs_String,"%s%02X ",Regs_String,reginfo.mr[i]);
        sprintf(Regs_String,"%s    ",Regs_String);
        break;
    }

}

void CUPD1007::addState(upd1007_config *info,int x) {
    info->cycles+=x;
    info->pPC->pTIMER->state += x;
}


/* 9-bit instruction info->opcode
  Note: accessing the internal ROM doesn't consume any clock cycles */
UINT16 CUPD1007::Fetchopcode()
{
  BYTE saveie;

  reginfo.savepc = reginfo.pc;
  if (reginfo.pc < ROM0SIZE) {
    reginfo.opcode[0] = rom0[reginfo.pc][0];
    reginfo.opcode[1] = rom0[reginfo.pc][1];
    reginfo.opcode[2] = rom0[reginfo.pc][2];
  }
  else
  {
    saveie = reginfo.iereg;
    reginfo.iereg = 0;
    reginfo.opcode[0] = RM((&reginfo),reginfo.pc);
    reginfo.opcode[1] = RM((&reginfo),reginfo.pc+1);
    reginfo.opcode[2] = RM((&reginfo),reginfo.pc+2);
    reginfo.iereg = saveie;
    addState(&reginfo,2);
  }
  reginfo.opindex = 1;
  reginfo.pc++;
  return reginfo.opcode[0] | ((reginfo.opcode[1] << 1) & 0x0100);
}

/* subsequent bytes of the instruction */
BYTE CUPD1007::FetchByte(upd1007_config *info)
{
  BYTE result = info->opcode[info->opindex];
  if (info->pc > ROM0SIZE) {
    info->pc++;
    addState(info,2);
  }
  info->opindex++;

  return result;
}


BYTE CUPD1007::Reg1 (BYTE x){
    return ((x >>1) & 0x38) | (x & 0x07);
}

/* expects the third info->opcode byte (specification of a general purpose register
  in the bank 1, direct | indirect),
  returns the address (index) of the general purpose register */
BYTE CUPD1007::Reg2 (upd1007_config *info,BYTE x) {
    if ( (x & 0x10) == 0) {	// directly specified register
        x = (x >> 5) | ((x << 3) & 0x38);
    }
    else if ( (info->asreg & 0x01) == 0)
        x = (info->asreg >> 1) & 0x3F;
    else
        x = ((info->asreg >> 1) & 0x07) | ((x << 3) & 0x38);

    return (x | 0x40);
}

/* expects the second & third info->opcode byte (specification of a general purpose
  register in the bank 1, direct | indirect),
  returns the first register of an array in the bank 1 */
BYTE CUPD1007::Reg3 (upd1007_config *info,BYTE x, BYTE y)
{
    if ((y & 0x10) == 0) //	{ directly specified array */
        x = (x & 0x07) | ((y << 3) & 0x38);
    else if ((info->asreg & 0x01) == 0)
        x = (info->asreg >> 1) & 0x3F;
    else
        x = ((info->asreg >> 1) & 0x07) | ((y << 3) & 0x38);

    return ( x | 0x40);
}

// optional last register of an indirectly specified array
BYTE CUPD1007::AsLimit (upd1007_config *info,BYTE y)
{
    if ((y & 0x10) == 0)
        return NONE_REG; //	{ directly specified array, no limit */
    else if ((info->asreg & 0x01) == 0)
        return ((info->asreg >> 1) & 0x3F) | 0x47;
    else
        return ((info->asreg >> 4) & 0x07) | ((y << 3) & 0x38) | 0x40;
}

BYTE CUPD1007::Rl1 (BYTE x, BYTE y)
{
    return ((x >> 1) & 0x38) | (y >> 5);
}


BYTE CUPD1007::Im6 (BYTE x, BYTE y)
{
    return (y & 0x1F) | (((~x) << 2) & 0x20);
}


BYTE CUPD1007::Ireg(upd1007_config *info)
{
    BYTE Result = info->opcode[0] & 0x03;
    if (Result > IZ)  Result = SP;

    return Result;
}


// next processed register in an array */
void CUPD1007::NextReg (upd1007_config *info,BYTE *x)
{
    *x = (*x & 0xF8) | ((*x + info->regstep) & 0x07);
}


/* returns the contents of a 16-bit register 'x',
  'y' is a dummy variable */
UINT16 CUPD1007::Wreg (upd1007_config *info,BYTE x, BYTE y)
{
    return (info->mr[x | 0x78] << 8) | info->mr[x | 0x38];
}


/* return the contents of the 16-bit register 'x',
  then add 8-bit value 'y' to this 16-bit register */
UINT16 CUPD1007::PostIncw (upd1007_config *info, BYTE x, BYTE y)
{
    UINT16 Result = Wreg(info,x,0);
    UINT16 z = Result + y;
    info->mr[x | 0x38] = Lo (z);
    info->mr[x | 0x78] = Hi (z);

    return Result;
}


/* return the contents of the 16-bit register 'x',
  then subtract 8-bit value 'y' from this 16-bit register */
UINT16 CUPD1007::PostDecw (upd1007_config *info, BYTE x, BYTE y)
{
  info->regstep = -1;
  UINT16 Result = Wreg(info,x,0);
  UINT16 z = Result - y;
  info->mr[x | 0x38] = Lo (z);
  info->mr[x | 0x78] = Hi (z);

  return Result;
}


/* add 8-bit value 'y' to the 16-bit register 'x',
  then return the contents of this 16-bit register*/
UINT16 CUPD1007::PreIncw (upd1007_config *info, BYTE x, BYTE y)
{
  UINT16 Result = Wreg(info,x,0) + y;
  info->mr[x | 0x38] = Lo (Result);
  info->mr[x | 0x78] = Hi (Result);

  return Result;
}


/* subtract 8-bit value 'y' from the 16-bit register 'x',
  then return the contents of this 16-bit register */
UINT16 CUPD1007::PreDecw (upd1007_config *info,BYTE x, BYTE y)
{
  info->regstep = -1;
  UINT16 Result = Wreg(info,x,0) - y;
  info->mr[x | 0x38] = Lo (Result);
  info->mr[x | 0x78] = Hi (Result);

  return Result;
}


/* add the 8-bit value 'x' to the 16-bit variable 'ea' */
void CUPD1007::PlusOffset (upd1007_config *info, BYTE x)
{
  info->ea += x;
}


/* subtract the 8-bit value 'x' from the 16-bit variable 'ea' */
void CUPD1007::MinusOffset (upd1007_config *info, BYTE x)
{
  info->regstep = -1;
  info->ea -= x;
}


/* unary operation on a register */
void CUPD1007::UnReg (upd1007_config *info,void *op2)
{
  if (op2 != (void*)&CUPD1007::OpSwp1) info->flag = 0;
  ((Func5) (op2)) (info,&(info->mr[info->regbank | Reg1 (FetchByte(info))]), 0);
}


/* register rotation through Carry */
void CUPD1007::RotReg (upd1007_config *info,void *op2)
{
  info->flag = info->flag & C_bit;
  ((Func5)(op2)) (info,&(info->mr[info->regbank | Reg1 (FetchByte(info))]), 0);
}


/* unary operation on an array */
void CUPD1007::UnAry (upd1007_config *info, void *op2)
{
  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  BYTE dstf,dstl,z;

  if (info->regbank != 0)
  {
    dstf = Reg3 (info,x, y);
    dstl = AsLimit (info,y);
  }
  else
  {
    dstf = Reg1 (x);
    dstl = NONE_REG;
  } // {if};
  x = x & 0x07;
  y = y >> 5;
  if (op2 == (void*)&CUPD1007::OpBnus)  {
      z = info->mr[dstf];
  }
  else
  {
    info->flag = 0;
    z = 0;
  } // {if};
  do {
    z = ((Func5) op2) (info,&info->mr[dstf], z);
    addState(info,4);
    if (x == y) return;
    if (dstf == (dstl ^ ((info->regstep >> 1) & 0x07)))
    do {
      NextReg (info,&x);
      dstf = 0;
      z = ((Func5)op2) (info,&dstf, z);
      addState(info,4);
      if (x == y) return;
    } while(true);
    NextReg (info,&x);
    NextReg (info,&dstf);
  } while(true);
}


/* array rotation through Carry */
void CUPD1007::RotAry (upd1007_config *info,void *op2)
{
  info->flag = info->flag & C_bit;
  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  BYTE dstf,dstl,z;

  if (info->regbank != 0)
  {
    dstf = Reg3 (info,x, y);
    dstl = AsLimit (info,y);
  }
  else
  {
    dstf = Reg1 (x);
    dstl = NONE_REG;
  }// {if};
  x = x & 0x07;
  y = y >> 5;
  z = 0;
  do {
    z = ((Func5)op2) (info,&info->mr[dstf], z);
    addState(info,4);
    if (x == y) return;
    if (dstf == (dstl ^ ((info->regstep >> 1) & 0x07)))
    do {
      NextReg (info,&x);
      dstf = 0;
      z = ((Func5)op2) (info,&dstf, z);
      addState(info,4);
      if (x == y) return;
    } while (true);
    NextReg (info,&x);
    NextReg (info,&dstf);
  } while (true);
}


void CUPD1007::Mtbm (upd1007_config *info,void *op2)
{
  info->flag = 0;
  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  y = info->regbank | Rl1(x,y);
  x = info->regbank | Reg1(x);
  BYTE z = 0;
  do {
    z = ((Func5)op2) (info,&info->mr[x], z);
    addState(info,4);
    if (x == y) break;
    NextReg (info,&x);
  } while (true);
}


/* destination_register <- destination_register op2 source_register
  source register in the bank 0, destination register in the bank 1 */
void CUPD1007::Xreg (upd1007_config *info,void *op2)
{
  if (op2 != (void*)&CUPD1007::OpLd) info->flag = 0;
  BYTE src = Reg1 (FetchByte(info));	/* index of the source register */
  BYTE dst = Reg2 (info,FetchByte(info));	/* index of the destination register */
  info->mr[dst] = ((Func3) op2) (info,info->mr[dst], info->mr[src]);
  addState(info,4);
}


/* destination_register <- destination_register op2 source_register
  source register in the bank 1, destination register in the bank 0 */
void CUPD1007::Yreg (upd1007_config *info,void *op2)
{
  if (op2 != (void*)&CUPD1007::OpLd) info->flag = 0;
  BYTE dst = Reg1 (FetchByte(info));	/* index of the destination register */
  BYTE src = Reg2 (info,FetchByte(info));	/* index of the source register */
  info->mr[dst] = ((Func3) op2) (info,info->mr[dst], info->mr[src]);
  addState(info,4);
}


/* check flags of the operation: destination_register op2 source_register
  source register in the bank 0, destination register in the bank 1 */
void CUPD1007::TXreg (upd1007_config *info,void *op2)
{
  info->flag = 0;
  BYTE src = Reg1 (FetchByte(info));	/* index of the source register */
  BYTE dst = Reg2 (info,FetchByte(info));	/* index of the destination register */
  ((Func3)op2) (info,info->mr[dst], info->mr[src]);
  addState(info,4);
}


/* check flags of the operation: destination_register op2 source_register
  source register in the bank 1, destination register in the bank 0 */
 void CUPD1007::TYreg (upd1007_config *info,void *op2)
{
  info->flag = 0;
  BYTE dst = Reg1 (FetchByte(info));	/* index of the destination register */
  BYTE src = Reg2 (info,FetchByte(info));	/* index of the source register */
  ((Func3)op2)(info,info->mr[dst], info->mr[src]);
  addState(info,4);
}


void CUPD1007::ExchReg (upd1007_config *info,void *op2)
{
  BYTE y = Reg1 (FetchByte(info));
  BYTE x = Reg2 (info,FetchByte(info));
  ((Proc5)op2) (info,&info->mr[x], &info->mr[y]);
  addState(info,4);
}


/* register <- register op2 immediate_byte */
void CUPD1007::RegIm8 (upd1007_config *info,void *op2)
{
  if (op2 != (void*)&CUPD1007::OpLd)  info->flag = 0;
  BYTE x = info->regbank | Reg1 (FetchByte(info));	/* index of the register */
  info->mr[x] = ((Func3)op2) (info,info->mr[x], FetchByte(info));
  addState(info,4);
}


/* check info->flag of the operation: register op2 immediate_byte */
void CUPD1007::TRegIm8 (upd1007_config *info,void *op2)
{
  info->flag = 0;
  BYTE x = info->regbank | Reg1 (FetchByte(info));	/* index of the register */
   ((Func3)op2) (info,info->mr[x], FetchByte(info));
  addState(info,4);
}


/* destination_array <- destination_array op2 source_array
  source array in the bank 0, destination array in the bank 1 */
void CUPD1007::Xary (upd1007_config *info,void *op2)
{
  if (op2 != (void*)&CUPD1007::OpLd) info->flag = 0;
  BYTE x1 = FetchByte(info);
  BYTE x2 = FetchByte(info);
  BYTE srcf = Reg1 (x1);		/* index of the first source register */
  BYTE srcl = Rl1 (x1, x2);		/* index of the last source register */
  BYTE dstf = Reg3 (info,x1, x2);	/* index of the first destination register */
  BYTE dstl = AsLimit (info,x2);		/* index of the last destination register */
  do {
    info->mr[dstf] = ((Func3)op2) (info,info->mr[dstf], info->mr[srcf]);
    addState(info,4);
    if (srcf == srcl) return;
    if (dstf == dstl)
    do {
      NextReg (info,&srcf);
      ((Func3)op2) (info,0, info->mr[srcf]);
      addState(info,4);
      if (srcf == srcl) return;
    } while (true);
    NextReg (info,&srcf);
    NextReg (info,&dstf);
  } while (true);
}


/* destination_array <- destination_array op2 source_array
  source array in the bank 1, destination array in the bank 0 */
void CUPD1007::Yary (upd1007_config *info,void *op2)
{
  if (op2 != (void*)&CUPD1007::OpLd) info->flag = 0;
  BYTE x1 = FetchByte(info);
  BYTE x2 = FetchByte(info);
  BYTE dstf = Reg1 (x1);		/* index of the first destination register */
  BYTE dstl = Rl1 (x1, x2);		/* index of the last destination register */
  BYTE srcf = Reg3 (info,x1, x2);	/* index of the first source register */
  BYTE srcl = AsLimit (info,x2);		/* index of the last source register */
  do {
    info->mr[dstf] =  ((Func3)op2) (info,info->mr[dstf], info->mr[srcf]);
    addState(info,4);
    if (dstf == dstl) return;
    if (srcf == srcl)
    do {
      NextReg (info,&dstf);
      info->mr[dstf] =  ((Func3)op2) (info,info->mr[dstf], 0);
      addState(info,4);
      if (dstf == dstl) return;
    } while (true);
    NextReg (info,&srcf);
    NextReg (info,&dstf);
  } while (true);
}


/* destination_array <- destination_array op2 source_array
  source array in the bank 0, destination array in the bank 1 */
void CUPD1007::TXary (upd1007_config *info,void *op2)
{
  info->flag = 0;
  BYTE x1 = FetchByte(info);
  BYTE x2 = FetchByte(info);
  BYTE srcf = Reg1 (x1);		/* index of the first source register */
  BYTE srcl = Rl1 (x1, x2);		/* index of the last source register */
  BYTE dstf = Reg3 (info,x1, x2);	/* index of the first destination register */
  BYTE dstl = AsLimit (info,x2);		/* index of the last destination register */
  do {
    ((Func3)op2) (info,info->mr[dstf], info->mr[srcf]);
    addState(info,4);
    if (srcf == srcl) return;
    if (dstf == dstl)
    do {
      NextReg (info,&srcf);
      ((Func3) op2) (info,0, info->mr[srcf]);
      addState(info,4);
      if (srcf == srcl) return;
    } while(true);
    NextReg (info,&srcf);
    NextReg (info,&dstf);
  } while(true);
}


/* destination_array <- destination_array op2 source_array
  source array in the bank 1, destination array in the bank 0 */
void CUPD1007::TYary (upd1007_config *info,void *op2)
{
  info->flag = 0;
  BYTE x1 = FetchByte(info);
  BYTE x2 = FetchByte(info);
  BYTE dstf = Reg1 (x1);		/* index of the first destination register */
  BYTE dstl = Rl1 (x1, x2);		/* index of the last destination register */
  BYTE srcf = Reg3 (info,x1, x2);	/* index of the first source register */
  BYTE srcl = AsLimit (info,x2);		/* index of the last source register */
  do {
    ((Func3) op2) (info,info->mr[dstf], info->mr[srcf]);
    addState(info,4);
    if (dstf == dstl) return;
    if (srcf == srcl)
    {
      NextReg (info,&dstf);
      ((Func3) op2) (info,info->mr[dstf], 0);
      addState(info,4);
      if (dstf == dstl) return;
    } while(true);
    NextReg (info,&srcf);
    NextReg (info,&dstf);
  } while(true);
}


void CUPD1007::ExchAry (upd1007_config *info,void *op2)
{
  BYTE x1 = FetchByte(info);
  BYTE x2 = FetchByte(info);
  BYTE srcf = Reg1 (x1);		/* index of the first source register */
  BYTE srcl = Rl1 (x1, x2);		/* index of the last source register */
  BYTE dstf = Reg3 (info,x1, x2);	/* index of the first destination register */
  BYTE dstl = AsLimit (info,x2);		/* index of the last destination register */
  do {
    ((Proc5) op2) (info,&info->mr[dstf], &info->mr[srcf]);
    addState(info,4);
    if (srcf == srcl) return;
    if (dstf == dstl)
    do {
      NextReg (info,&srcf);
      dstf = 0;
      ((Proc5) op2) (info,&dstf, &info->mr[srcf]);
      addState(info,4);
      if (srcf == srcl) return;
    } while(true);
    NextReg (info,&srcf);
    NextReg (info,&dstf);
  } while(true);
}


/* array <- array op2 immediate_value */
void CUPD1007::AryIm6 (upd1007_config *info,void *op2)
{
  if (op2 != (void*)&CUPD1007::OpLd) info->flag = 0;
  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  BYTE z = Im6 (x,y);
  y = info->regbank | Rl1 (x,y);	/* index of the last processed register */
  x = info->regbank | Reg1 (x);	/* index of the first processed register */
  do{
    info->mr[x] = ((Func3) op2) (info,info->mr[x], z);
    addState(info,4);
    if (x == y) break;
    NextReg (info,&x);
    if (op2 != (void*)&CUPD1007::OpLd) z = 0;
 } while(true);
}


/* check flags of the operation: array op2 immediate_value */
void CUPD1007::TAryIm6 (upd1007_config *info,void *op2)
{
  info->flag = 0;
  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  BYTE z = Im6 (x,y);
  y = info->regbank | Rl1 (x,y);	/* index of the last processed register */
  x = info->regbank | Reg1 (x);	/* index of the first processed register */
  do {
    ((Func3) op2) (info,info->mr[x], z);
    addState(info,4);
    if (x == y) break;
    NextReg (info,&x);
    z = 0;
  } while (true);
}


void CUPD1007::Ldw (upd1007_config *info,void *op2 /*dummy*/)
{
  BYTE dst = (info->opcode[0] & 0x07) | 0x38;
  info->mr[dst | 0x40] = FetchByte(info);
  info->mr[dst] = FetchByte(info);
  addState(info,4);
}


void CUPD1007::AdwSbw (upd1007_config *info,void *op2)
{
  ((Proc3) op2) (info,Ireg(info), FetchByte(info));
  addState(info,6);
}


/* conditional execution (when true) of 'op2' */
void CUPD1007::Cond (upd1007_config *info,void *op2)
{
  BYTE x1 = FetchByte(info);
  BYTE x2 = FetchByte(info);
  if ((info->flag & cc[info->opcode[0] & 0x07]) != 0)
      ((Proc3) op2) (info,x1, x2);
  addState(info,4);
}


/* conditional execution (when false) of 'op2' */
void CUPD1007::NotCond (upd1007_config *info,void *op2)
{
  BYTE x1 = FetchByte(info);
  BYTE x2 = FetchByte(info);
  if ((info->flag & cc[info->opcode[0] & 0x07]) == 0)
      ((Proc3) op2) (info,x1, x2);
  addState(info,4);
}


/* conditional execution (when key pressed) of 'op2' */
void CUPD1007::KeyCond (upd1007_config *info,void *op2)
{
  BYTE x1 = FetchByte(info);
  BYTE x2 = FetchByte(info);
  if (info->kireg != 0)
      ((Proc3)op2) (info,x1, x2);
  addState(info,4);
}


/* conditional execution (when key not pressed) of 'op2' */
void CUPD1007::NotKeyCond (upd1007_config *info,void *op2)
{
  BYTE x1 = FetchByte(info);
  BYTE x2 = FetchByte(info);
  if (info->kireg == 0)
      ((Proc3) op2) (info,x1, x2);
  addState(info,4);
}


void CUPD1007::Jump (upd1007_config *info,BYTE x1, BYTE x2)
{
    UINT16 _temp = (x1 << 8) | x2;
  info->pc = _temp;
/* Alternatively, the above statement could be replaced with following ones:
  ptrb(PChar(&CUPD1007::info->pc)+1)^ = x1;
  ptrb(&CUPD1007::info->pc)^ = x2;
  Perhaps they would be more effective, especially if the left sides would
  evaluate to constants. Unfortunately, the pointer to the 'info->pc' variable is
  probably unknown at the compile state. Also they have a disadvantage to be
  less portable, because little endian order of bytes is assumed. */
}


void CUPD1007::Call (upd1007_config *info, BYTE x1, BYTE x2)
{
  BYTE saveie = info->iereg;
  info->iereg = 0;
  WM(info,PreDecw(info,SP,1) , Lo (info->pc));
  WM(info,PreDecw(info,SP,1) , Hi (info->pc));
  info->iereg = saveie;
  info->pc = (x1 << 8) | x2;
  addState(info,6);
//  info->pPC->pCPU->CallSubLevel++;
}


void CUPD1007::Trap (upd1007_config *info,void *op2 /*dummy*/)
{
  FetchByte(info);
  Call (info,info->mr[V3 | 0x78], info->mr[V3 | 0x38]);
  addState(info,6);
}


/* for illegal operands the emulation may differ from the actual hardware */
void CUPD1007::Ijmp (upd1007_config *info,BYTE x1, BYTE x2)
{
  info->pc = (info->mr[Reg2(info,x2)] << 8) | info->mr[Reg1(x1)];
  addState(info,2);
}


void CUPD1007::Rtn (upd1007_config *info,void *op2 /*dummy*/)
{
  BYTE saveie = info->iereg;
  info->iereg = 0;
  BYTE x1 = RM(info,PostIncw(info,SP,1));
  BYTE x2 = RM(info,PostIncw(info,SP,1));
  info->iereg = saveie;
  info->pc = (x1 << 8) | x2;
  addState(info,10);
//  info->pPC->pCPU->CallSubLevel--;
}


void CUPD1007::Cani (upd1007_config *info,void *op2 /*dummy*/)
{
  for (int i=0;i<=2;i++)
  {
    if (info->irqcnt[i] != 0)
    {
      info->ifreg = info->ifreg & ~INT_serv[i];
      info->irqcnt[i] = 0;
      break;
    }
  }
  addState(info,4);
}


void CUPD1007::Rti (upd1007_config *info,void *op2 /*dummy*/)
{
  Cani (info,NULL);
  Rtn (info,NULL);
  addState(info,-4);
}


void CUPD1007::Nop (upd1007_config *info,void *op2 /*dummy*/)
{
  addState(info,4);
}


void CUPD1007::BlockCopy (upd1007_config *info,void *op2)
{
  WM(info,((Func4) op2) (info,IZ,1) , RM(info,((Func4) op2) (info,IX,1)));
  if ((info->mr[IX+56] != info->mr[IY+56]) |
      (info->mr[IX+120] != info->mr[IY+120]))
      info->pc = info->savepc;
  addState(info,14);
}


void CUPD1007::BlockSearch (upd1007_config *info,void *op2)
{
  if (RM(info,((Func4)op2) (info,Ireg(info),1)) != info->mr[info->regbank | Reg1(FetchByte(info))])
  {
    if (Wreg(info,Ireg(info),0) != Wreg(info,IY,0)) info->pc = info->savepc;
  }
  addState(info,16);
}


void CUPD1007::StMemoReg (upd1007_config *info,void *op2)
{
  WM(info,((Func4)op2) (info,Ireg(info),1) , info->mr[info->regbank | Reg1(FetchByte(info))]);
  addState(info,12);
}


void CUPD1007::StMemoIm8 (upd1007_config *info,void *op2)
{
  WM(info,((Func4)op2) (info,Ireg(info),1) , FetchByte(info));
  addState(info,6);
}


void CUPD1007::StmMemoAry (upd1007_config *info,void *op2)
{
  BYTE dst = Ireg(info);
  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  y = info->regbank | Rl1 (x, y);	/* index of the last processed register */
  x = info->regbank | Reg1 (x);	/* index of the first processed register */
  do{
    WM(info,((Func4)op2) (info,dst,1) , info->mr[x]);
    addState(info,4);
    if (x == y) break;
    NextReg (info,&x);
  } while(true);
  addState(info,8);
}


void CUPD1007::StImOffsReg (upd1007_config *info,void *op2)
{
  info->ea = Wreg (info,Ireg(info),0);
  BYTE x = Reg1 (FetchByte(info));
  ((Proc4)op2) (info,FetchByte(info));
  WM(info,info->ea , info->mr[x]);
  addState(info,12);
}


void CUPD1007::StRegOffsReg (upd1007_config *info,void *op2)
{
  info->ea = Wreg (info,Ireg(info),0);
  ((Proc4)op2) (info,info->mr[Reg1 (FetchByte(info))]);
  BYTE x = Reg2 (info,FetchByte(info));
  WM(info,info->ea , info->mr[x]);
  addState(info,12);
}


void CUPD1007::StmImOffsAry (upd1007_config *info,void *op2)
{
  info->ea = Wreg (info,Ireg(info),0);
  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  ((Proc4)op2) (info,Im6 (x,y));
  y = Rl1 (x, y);		/* index of the last processed register */
  x = Reg1 (x);		/* index of the first processed register */
  do{
    WM(info,info->ea , info->mr[x]);
    addState(info,4);
    if (x == y) break;
    NextReg (info,&x);
    ((Proc4)op2) (info,1);
  } while(true);
  addState(info,8);
}


void CUPD1007::StmRegOffsAry (upd1007_config *info,void *op2)
{
  info->ea = Wreg (info,Ireg(info),0);
  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  ((Proc4)op2) (info,info->mr[Reg1 (x)]);
  BYTE first = Reg1 (x);
  BYTE last = Rl1 (x, y);
  x = Reg3 (info,x, y);		/* index of the first processed register */
  do{
    WM(info,info->ea , info->mr[x]);
    addState(info,4);
    if (first == last) break;
    NextReg (info,&first);
    NextReg (info,&x);
    ((Proc4)op2) (info,1);
  } while(true);
  addState(info,8);
}


void CUPD1007::LdRegMemo (upd1007_config *info,void *op2)
{
  info->mr[info->regbank | Reg1(FetchByte(info))] = RM(info,((Func4)op2) (info,Ireg(info),1));
  addState(info,12);
}


void CUPD1007::LdmAryMemo (upd1007_config *info,void *op2)
{
  BYTE src = Ireg(info);
  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  y = info->regbank | Rl1 (x, y);	/* index of the last processed register */
  x = info->regbank | Reg1 (x);	/* index of the first processed register */
  do {
    info->mr[x] = RM(info,((Func4)op2) (info,src,1));
    addState(info,4);
    if (x == y) break;
    NextReg (info,&x);
  } while(true);
  addState(info,8);
}


void CUPD1007::LdRegImOffs (upd1007_config *info,void *op2)
{
  info->ea = Wreg (info,Ireg(info),0);
  BYTE x = Reg1 (FetchByte(info));
  ((Proc4)op2) (info,FetchByte(info));
  info->mr[x] = RM(info,info->ea);
  addState(info,12);
}


void CUPD1007::LdRegRegOffs (upd1007_config *info,void *op2)
{
  info->ea = Wreg (info,Ireg(info),0);
  ((Proc4)op2) (info,info->mr[Reg1 (FetchByte(info))]);
  BYTE x = Reg2 (info,FetchByte(info));
  info->mr[x] = RM(info,info->ea);
  addState(info,12);
}


void CUPD1007::LdmAryImOffs (upd1007_config *info,void *op2)
{
  info->ea = Wreg (info,Ireg(info),0);
  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  ((Proc4) op2) (info,Im6 (x,y));
  y = Rl1 (x, y);		/* index of the last processed register */
  x = Reg1 (x);		/* index of the first processed register */
  do {
    info->mr[x] = RM(info,info->ea);
    addState(info,4);
    if (x == y) break;
    NextReg (info,&x);
    ((Proc4)op2) (info,1);
  } while(true);
  addState(info,8);
}


void CUPD1007::LdmAryRegOffs (upd1007_config *info,void *op2)
{
  info->ea = Wreg (info,Ireg(info),0);
  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  ((Proc4)op2) (info,info->mr[Reg1 (x)]);
  BYTE first = Reg1 (x);
  BYTE last = Rl1 (x, y);
  x = Reg3 (info,x, y);		/* index of the first processed register */
  do{
    info->mr[x] = RM(info,info->ea);
    addState(info,4);
    if (first == last) break;
    NextReg (info,&first);
    NextReg (info,&x);
    ((Proc4) op2) (info,1);
  } while(true);
  addState(info,8);
}


void CUPD1007::PstIm8 (upd1007_config *info,void *op2)
{
  ((Proc4)op2) (info,FetchByte(info));
  addState(info,4);
}


void CUPD1007::PstReg (upd1007_config *info,void *op2)
{
  ((Proc4) op2) (info,info->mr[info->regbank | Reg1(FetchByte(info))]);
  addState(info,4);
}


void CUPD1007::Gst (upd1007_config *info,void *op2)
{
  info->mr[info->regbank | Reg1(FetchByte(info))] = ((Func1)op2)(info);
  addState(info,4);
}


void CUPD1007::Off (upd1007_config *info,void *op2)
{
    qWarning()<<"Off";
  info->CpuSleep = true;
  info->pPC->pCPU->halt = true;
  info->pc = 0x0000;
  info->iereg = 0;
  info->ifreg = 0x00;
  info->irqcnt[0] = 0;
  info->irqcnt[1] = 0;
  info->irqcnt[2] = 0;
  info->lcdctrl = 0;
//  LcdInit;
  DoPorts(info);
  info->koreg = 0x41;
//  KeyHandle;
}



/* ARITHMETICAL & LOGICAL OPERATIONS */


void CUPD1007::ZeroBits (upd1007_config *info,BYTE x)
{
  if (x != 0) info->flag |= NZ_bit;
  if ((x & 0xF0) == 0) info->flag |= UZ_bit;
  if ((x & 0x0F) == 0) info->flag |= LZ_bit;
}


/* binary operations */

/* addition with carry */
BYTE CUPD1007::OpAd (upd1007_config *info,BYTE x, BYTE y)
{
  unsigned int in1 = x;
  unsigned int in2 = y;
  unsigned int out = in1 + in2;
  if ((info->flag & C_bit) != 0) out++;
  unsigned int  temp = in1 ^ in2 ^ out;
  info->flag = info->flag & NZ_bit;
  if (out > 0xFF) info->flag |= C_bit;
  if ((temp & 0x10) != 0) info->flag |= H_bit;
  if ((temp & 0x80) != 0) info->flag |= V_bit;
  ZeroBits (info,out);
  return out;
}


/* subtraction with borrow */
BYTE CUPD1007::OpSb (upd1007_config *info,BYTE x, BYTE y)
{
  unsigned int in1 = x;
  unsigned int in2 = y;
  unsigned int out = in1 - in2;
  if ((info->flag & C_bit) != 0) out--;
  unsigned int temp = in1 ^ in2 ^ out;
  info->flag &= NZ_bit;
  if (out > 0xFF)  info->flag |= C_bit;
  if ((temp & 0x10) != 0) info->flag |= H_bit;
  if ((temp & 0x80) != 0) info->flag |= V_bit;
  ZeroBits (info,out);
  return out;
}


/* BCD addition with carry */
BYTE CUPD1007::OpAdb (upd1007_config *info,BYTE x, BYTE y)
{
  unsigned int in1 = x;
  unsigned int in2 = y;
/* lower nibble */
  unsigned int out = (in1 & 0x0F) + (in2 & 0x0F);
  if ((info->flag & C_bit) != 0)  out++;
  info->flag = info->flag & NZ_bit;
/* decimal adjustement */
  if (out > 0x09)
  {
    out = ((out + 0x06) & 0x0F) | 0x10;
    if (out > 0x1F) out-=0x10;
    info->flag = info->flag | H_bit;
  }
/* upper nibble */
  out+= (in1 & 0xF0) + (in2 & 0xF0);
  if (((in1 ^ in2 ^ out) & 0x80) != 0)  info->flag |= V_bit;
/* decimal adjustement */
  if (out > 0x9F)
  {
    out+=0x60;
    info->flag |= C_bit;
  }
  ZeroBits (info,out);
  return out;
}


/* BCD subtraction with borrow */
BYTE CUPD1007::OpSbb (upd1007_config *info,BYTE x, BYTE y)
{
  unsigned int in1 = (x);
  unsigned int in2 = (y);
/* lower nibble */
  unsigned int out = (in1 & 0x0F) - (in2 & 0x0F);
  if ((info->flag & C_bit) != 0) out--;
  info->flag &= NZ_bit;
/* decimal adjustement */
  if (out > 0x09)
  {
    out = (out - 0x06) | (unsigned int)(-0x10);
    info->flag |= H_bit;
  }
/* upper nibble */
  out += (in1 & 0xF0) - (in2 & 0xF0);
/* decimal adjustement */
  if (out > 0x9F)
  {
    out -= 0x60;
    info->flag = info->flag | C_bit;
  }
  ZeroBits (info,out);
  return out;
}


BYTE CUPD1007::OpAn (upd1007_config *info,BYTE x, BYTE y)
{
  BYTE Result = x & y;
  info->flag = info->flag & NZ_bit;
  ZeroBits (info,Result);
  return Result;
}


BYTE CUPD1007::OpBit (upd1007_config *info,BYTE x, BYTE y)
{
  BYTE Result = (~x) & y;
  info->flag = info->flag & NZ_bit;
  ZeroBits (info,Result);
  return Result;
}


BYTE CUPD1007::OpXr (upd1007_config *info,BYTE x, BYTE y)
{
  BYTE Result = x ^ y;
  info->flag = info->flag & NZ_bit;
  ZeroBits (info,Result);
  return Result;
}


BYTE CUPD1007::OpNa (upd1007_config *info,BYTE x, BYTE y)
{
 BYTE result = ~(x & y);
  info->flag = (info->flag & NZ_bit) | (C_bit | V_bit | H_bit);
  return result;
}


BYTE CUPD1007::OpOr (upd1007_config *info,BYTE x, BYTE y)
{
  BYTE Result = x | y;
  info->flag = (info->flag & NZ_bit) | (C_bit | V_bit | H_bit);
  ZeroBits (info,Result);
  return Result;
}


BYTE CUPD1007::OpLd (upd1007_config *info,BYTE x, BYTE y)
{
  return y;
}


/* unary operations, dummy second operand & returned value */


BYTE CUPD1007::OpRod (upd1007_config *info,BYTE *x, BYTE y)
{
  info->regstep = -1;
  BYTE z = *x;
  *x = *x >> 1;
  if ((info->flag & C_bit) != 0)  *x |= 0x80;
  info->flag &= NZ_bit;
  if ((z & 0x01) != 0) info->flag |= C_bit;
  ZeroBits (info,*x);
  return 0;
}


BYTE CUPD1007::OpRou (upd1007_config *info,BYTE *x, BYTE y)
{
  *x = OpAd (info,*x, *x);
  return 0;
}


BYTE CUPD1007::OpMtb (upd1007_config *info,BYTE *x, BYTE y)
{
  *x = OpAdb (info,*x, *x);
  return 0;
}


BYTE CUPD1007::OpInv (upd1007_config *info,BYTE *x, BYTE y)
{
  *x = ~(*x);
  info->flag = (info->flag & NZ_bit) | (C_bit | V_bit | H_bit);
  ZeroBits (info,*x);
  return 0;
}


BYTE CUPD1007::OpCmp (upd1007_config *info,BYTE *x, BYTE y)
{
  *x = OpSb (info,0, *x);
  return 0;
}


BYTE CUPD1007::OpCmpb (upd1007_config *info,BYTE *x, BYTE y)
{
  *x = OpSbb (info,0, *x);
  return 0;
}


BYTE CUPD1007::OpSwp1 (upd1007_config *info,BYTE *x, BYTE y)
{
  *x = (*x << 4) | (*x >> 4);
  return 0;
}


/* shifts by 4 & 8 bits */

BYTE CUPD1007::OpDiu (upd1007_config *info,BYTE *x, BYTE y)
{
  BYTE result = *x >> 4;
  *x = (*x << 4) | y;
  info->flag = info->flag & NZ_bit;
  ZeroBits (info,*x);
  return result;
}


BYTE CUPD1007::OpDid (upd1007_config *info,BYTE *x, BYTE y)
{
  info->regstep = -1;
  BYTE result= *x << 4;
  *x = (*x >> 4) | y;
  info->flag = info->flag & NZ_bit;
  ZeroBits (info,*x);
  return result;
}


BYTE CUPD1007::OpByu(upd1007_config *info,BYTE *x, BYTE y)
{
  BYTE result = *x;
  *x = y;
  info->flag = info->flag & NZ_bit;
  ZeroBits (info,*x);
  return result;
}


BYTE CUPD1007::OpByd (upd1007_config *info,BYTE *x, BYTE y)
{
  info->regstep = -1;
  *x = y;
  info->flag = info->flag & NZ_bit;
  ZeroBits (info,*x);
  return *x;
}


BYTE CUPD1007::OpBnus (upd1007_config *info, BYTE *x, BYTE y)
{
  BYTE _result = *x;
  *x = (y << 4) | (y >> 4);

  return _result;
}


/* exchanges */

/* exchange of two bytes */
void CUPD1007::OpXc (upd1007_config *info,BYTE *x, BYTE *y)
{
  BYTE temp = *x;
  *x = *y;
  *y = temp;
}


/* rotation of four digits (nibbles) counterclockwise */
void CUPD1007::OpXcls (upd1007_config *info,BYTE *x, BYTE *y)
{
  BYTE temp = (*y >> 4) | (*x & 0xF0);
  *x = (*x << 4) | (*y & 0x0F);
  *y = temp;
}


/* rotation of four digits (nibbles) clockwise */
void CUPD1007::OpXchs (upd1007_config *info,BYTE *x, BYTE *y)
{
  BYTE temp = (*y << 4) | (*x & 0x0F);
  *x = (*x >> 4) | (*y & 0xF0);
  *y = temp;
}


/* swap nibbles in two bytes */
void CUPD1007::OpSwp2 (upd1007_config *info,BYTE *x, BYTE *y)
{
  *x = (*x << 4) | (*x >> 4);
  *y = (*y << 4) | (*y >> 4);
}



/* WRITING TO THE STATUS REGISTERS */


void CUPD1007::OpKo (upd1007_config *info,BYTE x)
{
  info->koreg = x;
//  KeyHandle;
}


void CUPD1007::OpIf (upd1007_config *info,BYTE x)
{
  info->ifreg = (info->ifreg & 0xEE) | (x & 0x11);
  DoPorts(info);
}


void CUPD1007::OpAs (upd1007_config *info,BYTE x)
{
  info->asreg = x;
}


void CUPD1007::OpIe (upd1007_config *info,BYTE x)
{
  info->iereg = x;
  for (int i=0;i<3;i++)
  {
    if ((x & INT_enable[i]) == 0)
    {
      info->ifreg &= (~INT_serv[i]);
      info->irqcnt[i] = 0;
    }
  }
}


void CUPD1007::OpFl (upd1007_config *info,BYTE x)
{
  info->flag = x;
}



/* LCD TRANSFER */


void CUPD1007::Ldle (upd1007_config *info,void* op2)
{
//  LcdSync;
//    qWarning()<<"Ldle";
  BYTE x = info->regbank | Reg1 (FetchByte(info));	/* index of the register */
  info->lcdctrl = (info->lcdctrl & ~0x3F) | (FetchByte(info) & 0x3F);
  info->pPC->out(0,info->lcdctrl);
  info->mr[x] = info->pPC->in(1);
  addState(info,8);
//  info->mr[x] = info->mr[x] | (info->pPC->in(1) << 4);
}


void CUPD1007::Ldlo (upd1007_config *info,void* op2)
{
//  LcdSync;
//    qWarning()<<"Ldlo";
  BYTE x = info->regbank | Reg1 (FetchByte(info));	/* index of the register */
  info->lcdctrl = (info->lcdctrl & ~0x3F) | (FetchByte(info) & 0x3F);
  info->pPC->out(0,info->lcdctrl);
  info->mr[x] = info->pPC->in(1);
  addState(info,8);
}


void CUPD1007::Stle (upd1007_config *info,void* op2)
{
//  LcdSync;
//    qWarning()<<"Stle";
  BYTE x = info->regbank | Reg1 (FetchByte(info));	/* index of the register */
  info->lcdctrl = (info->lcdctrl & ~0x3F) | (FetchByte(info) & 0x3F);
  info->pPC->out(0,info->lcdctrl);
  info->pPC->out(1,info->mr[x]);
  addState(info,8);
//  info->pPC->out(1,info->mr[x] >> 4);
}


void CUPD1007::Stlo (upd1007_config *info,void* op2)
{
//  LcdSync;
//    qWarning()<<"Stlo";
  BYTE x = info->regbank | Reg1 (FetchByte(info));	/* index of the register */
  info->lcdctrl = (info->lcdctrl & ~0x3F) | (FetchByte(info) & 0x3F);
  info->pPC->out(0,info->lcdctrl);
  info->pPC->out(1,info->mr[x]);
  addState(info,8);
}


void CUPD1007::Ldlem (upd1007_config *info,void* op2)
{
//  LcdSync;
//    qWarning()<<"Ldlem";
  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  info->lcdctrl = (info->lcdctrl & ~0x3F) | Im6 (x,y);
  info->pPC->out(0,info->lcdctrl);
  y = info->regbank | Rl1 (x,y);	/* index of the last processed register */
  x = info->regbank | Reg1 (x);	/* index of the first processed register */
  do {
    info->mr[x] = info->pPC->in(1);
    addState(info,8);
//    info->mr[x] = info->mr[x] | (info->pPC->in(1) << 4);
    if (x == y) break;
    NextReg (info,&x);
  } while(true);
}


void CUPD1007::Ldlom (upd1007_config *info,void* op2)
{
//  LcdSync;
//    qWarning()<<"Ldlom";
  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  info->lcdctrl = (info->lcdctrl & ~0x3F) | Im6 (x,y);
  info->pPC->out(0,info->lcdctrl);
  y = info->regbank | Rl1 (x,y);	/* index of the last processed register */
  x = info->regbank | Reg1 (x);	/* index of the first processed register */
  do {
    info->mr[x] = info->pPC->in(1);
    addState(info,8);
//    info->mr[x] = info->mr[x] | (info->pPC->in(1) << 4);
    if (x == y)  break;
    NextReg (info,&x);
  } while(true);
}

// Data write
void CUPD1007::Stlem (upd1007_config *info,void* op2)
{
//  LcdSync;
//    qWarning()<<"Stlem";
  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  info->lcdctrl = (info->lcdctrl & ~0x3F) | Im6 (x,y);
  info->pPC->out(0,info->lcdctrl);
  y = info->regbank | Rl1 (x,y);	/* index of the last processed register */
  x = info->regbank | Reg1 (x);	/* index of the first processed register */
  do {
//      qWarning()<<"Stlem Loop";
    info->pPC->out(1,info->mr[x]);
    addState(info,8);
//    info->pPC->out(1,info->mr[x] >> 4);
    if (x == y) break;
    NextReg (info,&x);
  } while(true);
//  qWarning()<<"Stlem End";
}

// Control write
void CUPD1007::Stlom (upd1007_config *info,void* op2)
{
//  LcdSync;
//    qWarning()<<"Stlom";

  BYTE x = FetchByte(info);
  BYTE y = FetchByte(info);
  info->lcdctrl = (info->lcdctrl & ~0x3F) | Im6 (x,y);
  info->pPC->out(0,info->lcdctrl);
  y = info->regbank | Rl1 (x,y);	/* index of the last processed register */
  x = info->regbank | Reg1 (x);	/* index of the first processed register */
  do {
    info->pPC->out(1,info->mr[x]);
    addState(info,8);
//    info->pPC->out(1,info->mr[x] >> 4);
    if (x == y) break;
    NextReg (info,&x);
  } while(true);
}

BYTE CUPD1007::Get_kireg(upd1007_config *info) {
    return info->kireg;
}

BYTE CUPD1007::Get_koreg(upd1007_config *info) {
    return info->koreg;
}
BYTE CUPD1007::Get_asreg(upd1007_config *info) {
    return info->asreg;
}
BYTE CUPD1007::Get_flag(upd1007_config *info) {
    return info->flag;
}
BYTE CUPD1007::Get_iereg(upd1007_config *info) {
    return info->iereg;
}
BYTE CUPD1007::Get_ifreg(upd1007_config *info) {
    return info->ifreg;
}



#if 1

    //{ bit 7 of the second byte cleared }
void* dtab[512][2] = {
    {	(void*)&CUPD1007::Xreg,		(void*)&CUPD1007::OpAdb		},		// { code $00, ADB }
    {	(void*)&CUPD1007::Xreg,		(void*)&CUPD1007::OpSbb		},		// { code $01, SBB }
    {	(void*)&CUPD1007::Xreg,		(void*)&CUPD1007::OpAd		},		// { code $02, AD }
    {	(void*)&CUPD1007::Xreg,		(void*)&CUPD1007::OpSb		},		// { code $03, SB }
    {	(void*)&CUPD1007::Xreg,		(void*)&CUPD1007::OpAn		},		// { code $04, AN }
    {	(void*)&CUPD1007::Xreg,		(void*)&CUPD1007::OpNa		},		// { code $05, NA }
    {	(void*)&CUPD1007::Xreg,		(void*)&CUPD1007::OpOr		},		// { code $06, OR }
    {	(void*)&CUPD1007::Xreg,		(void*)&CUPD1007::OpXr		},		// { code $07, XR }
    {	(void*)&CUPD1007::Yreg,		(void*)&CUPD1007::OpAdb		},		// { code $08, ADB }
    {	(void*)&CUPD1007::Yreg,		(void*)&CUPD1007::OpSbb		},		// { code $09, SBB }
    {	(void*)&CUPD1007::Yreg,		(void*)&CUPD1007::OpAd		},		// { code $0A, AD }
    {	(void*)&CUPD1007::Yreg,		(void*)&CUPD1007::OpSb		},		// { code $0B, SB }
    {	(void*)&CUPD1007::Yreg,		(void*)&CUPD1007::OpAn		},		// { code $0C, AN }
    {	(void*)&CUPD1007::Yreg,		(void*)&CUPD1007::OpNa		},		// { code $0D, NA }
    {	(void*)&CUPD1007::Yreg,		(void*)&CUPD1007::OpOr		},		// { code $0E, OR }
    {	(void*)&CUPD1007::Yreg,		(void*)&CUPD1007::OpXr		},		// { code $0F, XR }
    {	(void*)&CUPD1007::Xary,		(void*)&CUPD1007::OpAdb		},		// { code $10, ADBM }
    {	(void*)&CUPD1007::Xary,		(void*)&CUPD1007::OpSbb		},		// { code $11, SBBM }
    {	(void*)&CUPD1007::Xary,		(void*)&CUPD1007::OpAd		},		// { code $12, ADM }
    {	(void*)&CUPD1007::Xary,		(void*)&CUPD1007::OpSb		},		// { code $13, SBM }
    {	(void*)&CUPD1007::Xary,		(void*)&CUPD1007::OpAn		},		// { code $14, ANM }
    {	(void*)&CUPD1007::Xary,		(void*)&CUPD1007::OpNa		},		// { code $15, NAM }
    {	(void*)&CUPD1007::Xary,		(void*)&CUPD1007::OpOr		},		// { code $16, ORM }
    {	(void*)&CUPD1007::Xary,		(void*)&CUPD1007::OpXr		},		// { code $17, XRM }
    {	(void*)&CUPD1007::Yary,		(void*)&CUPD1007::OpAdb		},		// { code $18, ADBM }
    {	(void*)&CUPD1007::Yary,		(void*)&CUPD1007::OpSbb		},		// { code $19, SBBM }
    {	(void*)&CUPD1007::Yary,		(void*)&CUPD1007::OpAd		},		// { code $1A, ADM }
    {	(void*)&CUPD1007::Yary,		(void*)&CUPD1007::OpSb		},		// { code $1B, SBM }
    {	(void*)&CUPD1007::Yary,		(void*)&CUPD1007::OpAn		},		// { code $1C, ANM }
    {	(void*)&CUPD1007::Yary,		(void*)&CUPD1007::OpNa		},		// { code $1D, NAM }
    {	(void*)&CUPD1007::Yary,		(void*)&CUPD1007::OpOr		},		// { code $1E, ORM }
    {	(void*)&CUPD1007::Yary,		(void*)&CUPD1007::OpXr		},		// { code $1F, XRM }
    {	(void*)&CUPD1007::TXreg,		(void*)&CUPD1007::OpAdb		},		// { code $20, TADB }
    {	(void*)&CUPD1007::TXreg,		(void*)&CUPD1007::OpSbb		},		// { code $21, TSBB }
    {	(void*)&CUPD1007::TXreg,		(void*)&CUPD1007::OpAd		},		// { code $22, TAD }
    {	(void*)&CUPD1007::TXreg,		(void*)&CUPD1007::OpSb		},		// { code $23, TSB }
    {	(void*)&CUPD1007::TXreg,		(void*)&CUPD1007::OpAn		},		// { code $24, TAN }
    {	(void*)&CUPD1007::TXreg,		(void*)&CUPD1007::OpNa		},		// { code $25, TNA }
    {	(void*)&CUPD1007::TXreg,		(void*)&CUPD1007::OpOr		},		// { code $26, TOR }
    {	(void*)&CUPD1007::TXreg,		(void*)&CUPD1007::OpXr		},		// { code $27, TXR }
    {	(void*)&CUPD1007::TYreg,		(void*)&CUPD1007::OpAdb		},		// { code $28, TADB }
    {	(void*)&CUPD1007::TYreg,		(void*)&CUPD1007::OpSbb		},		// { code $29, TSBB }
    {	(void*)&CUPD1007::TYreg,		(void*)&CUPD1007::OpAd		},		// { code $2A, TAD }
    {	(void*)&CUPD1007::TYreg,		(void*)&CUPD1007::OpSb		},		// { code $2B, TSB }
    {	(void*)&CUPD1007::TYreg,		(void*)&CUPD1007::OpAn		},		// { code $2C, TAN }
    {	(void*)&CUPD1007::TYreg,		(void*)&CUPD1007::OpNa		},		// { code $2D, TNA }
    {	(void*)&CUPD1007::TYreg,		(void*)&CUPD1007::OpOr		},		// { code $2E, TOR }
    {	(void*)&CUPD1007::TYreg,		(void*)&CUPD1007::OpXr		},		// { code $2F, TXR }
    {	(void*)&CUPD1007::TXary,		(void*)&CUPD1007::OpAdb		},		// { code $30, TADBM }
    {	(void*)&CUPD1007::TXary,		(void*)&CUPD1007::OpSbb		},		// { code $31, TSBBM }
    {	(void*)&CUPD1007::TXary,		(void*)&CUPD1007::OpAd		},		// { code $32, TADM }
    {	(void*)&CUPD1007::TXary,		(void*)&CUPD1007::OpSb		},		// { code $33, TSBM }
    {	(void*)&CUPD1007::TXary,		(void*)&CUPD1007::OpAn		},		// { code $34, TANM }
    {	(void*)&CUPD1007::TXary,		(void*)&CUPD1007::OpNa		},		// { code $35, TNAM }
    {	(void*)&CUPD1007::TXary,		(void*)&CUPD1007::OpOr		},		// { code $36, TORM }
    {	(void*)&CUPD1007::TXary,		(void*)&CUPD1007::OpXr		},		// { code $37, TXRM }
    {	(void*)&CUPD1007::TYary,		(void*)&CUPD1007::OpAdb		},		// { code $38, TADBM }
    {	(void*)&CUPD1007::TYary,		(void*)&CUPD1007::OpSbb		},		// { code $39, TSBBM }
    {	(void*)&CUPD1007::TYary,		(void*)&CUPD1007::OpAd		},		// { code $3A, TADM }
    {	(void*)&CUPD1007::TYary,		(void*)&CUPD1007::OpSb		},		// { code $3B, TSBM }
    {	(void*)&CUPD1007::TYary,		(void*)&CUPD1007::OpAn		},		// { code $3C, TANM }
    {	(void*)&CUPD1007::TYary,		(void*)&CUPD1007::OpNa		},		// { code $3D, TNAM }
    {	(void*)&CUPD1007::TYary,		(void*)&CUPD1007::OpOr		},		// { code $3E, TORM }
    {	(void*)&CUPD1007::TYary,		(void*)&CUPD1007::OpXr		},		// { code $3F, TXRM }
    {	(void*)&CUPD1007::Ldw,		NULL		},		// { code $40, LDW }
    {	(void*)&CUPD1007::Ldw,		NULL		},		// { code $41, LDW }
    {	(void*)&CUPD1007::Ldw,		NULL		},		// { code $42, LDW }
    {	(void*)&CUPD1007::Ldw,		NULL		},		// { code $43, LDW }
    {	(void*)&CUPD1007::Ldw,		NULL		},		// { code $44, LDW }
    {	(void*)&CUPD1007::Ldw,		NULL		},		// { code $45, LDW }
    {	(void*)&CUPD1007::Ldw,		NULL		},		// { code $46, LDW }
    {	(void*)&CUPD1007::Ldw,		NULL		},		// { code $47, LDW }
    {	(void*)&CUPD1007::AdwSbw,	(void*)&CUPD1007::PreIncw	},	// { code $48, ADW }
    {	(void*)&CUPD1007::AdwSbw,	(void*)&CUPD1007::PreIncw	},	// { code $49, ADW }
    {	(void*)&CUPD1007::AdwSbw,	(void*)&CUPD1007::PreIncw	},	// { code $4A, ADW }
    {	(void*)&CUPD1007::AdwSbw,	(void*)&CUPD1007::PreIncw	},	// { code $4B, ADW }
    {	(void*)&CUPD1007::AdwSbw,	(void*)&CUPD1007::PreDecw	},	// { code $4C, SBW }
    {	(void*)&CUPD1007::AdwSbw,	(void*)&CUPD1007::PreDecw	},	// { code $4D, SBW }
    {	(void*)&CUPD1007::AdwSbw,	(void*)&CUPD1007::PreDecw	},	// { code $4E, SBW }
    {	(void*)&CUPD1007::AdwSbw,	(void*)&CUPD1007::PreDecw	},	// { code $4F, SBW }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Ijmp		},		// { code $50, IJMP }
    {	(void*)&CUPD1007::KeyCond,	(void*)&CUPD1007::Ijmp		},		// { code $51, IJMP }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Ijmp		},		// { code $52, IJMP }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Ijmp		},		// { code $53, IJMP }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Ijmp		},		// { code $54, IJMP }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Ijmp		},		// { code $55, IJMP }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Ijmp		},		// { code $56, IJMP }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Ijmp		},		// { code $57, IJMP }
    {	(void*)&CUPD1007::Rtn,		NULL		},		// { code $58, RTN }
    {	(void*)&CUPD1007::NotKeyCond,	(void*)&CUPD1007::Ijmp		},		// { code $59, IJMP }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Ijmp		},		// { code $5A, IJMP }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Ijmp		},		// { code $5B, IJMP }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Ijmp		},		// { code $5C, IJMP }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Ijmp		},		// { code $5D, IJMP }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Ijmp		},		// { code $5E, IJMP }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Ijmp		},		// { code $5F, IJMP }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Call		},		// { code $60, CAL }
    {	(void*)&CUPD1007::KeyCond,	(void*)&CUPD1007::Call		},		// { code $61, CAL }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Call		},		// { code $62, CAL }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Call		},		// { code $63, CAL }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Call		},		// { code $64, CAL }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Call		},		// { code $65, CAL }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Call		},		// { code $66, CAL }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Call		},		// { code $67, CAL }
    {	(void*)&CUPD1007::Cani,		NULL		},		// { code $68, CANI }
    {	(void*)&CUPD1007::NotKeyCond,	(void*)&CUPD1007::Call		},		// { code $69, CAL }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Call		},		// { code $6A, CAL }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Call		},		// { code $6B, CAL }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Call		},		// { code $6C, CAL }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Call		},		// { code $6D, CAL }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Call		},		// { code $6E, CAL }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Call		},		// { code $6F, CAL }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Jump		},		// { code $70, JMP }
    {	(void*)&CUPD1007::KeyCond,	(void*)&CUPD1007::Jump		},		// { code $71, JMP }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Jump		},		// { code $72, JMP }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Jump		},		// { code $73, JMP }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Jump		},		// { code $74, JMP }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Jump		},		// { code $75, JMP }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Jump		},		// { code $76, JMP }
    {	(void*)&CUPD1007::Cond,		(void*)&CUPD1007::Jump		},		// { code $77, JMP }
    {	(void*)&CUPD1007::Rti,		NULL		},		// { code $78, RTI }
    {	(void*)&CUPD1007::NotKeyCond,	(void*)&CUPD1007::Jump		},		// { code $79, JMP }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Jump		},		// { code $7A, JMP }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Jump		},		// { code $7B, JMP }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Jump		},		// { code $7C, JMP }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Jump		},		// { code $7D, JMP }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Jump		},		// { code $7E, JMP }
    {	(void*)&CUPD1007::NotCond,	(void*)&CUPD1007::Jump		},		// { code $7F, JMP }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreIncw	},	// { code $80, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreIncw	},	// { code $81, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreIncw	},	// { code $82, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreIncw	},	// { code $83, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreDecw	},	// { code $84, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreDecw	},	// { code $85, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreDecw	},	// { code $86, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreDecw	},	// { code $87, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreIncw	},	// { code $88, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreIncw	},	// { code $89, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreIncw	},	// { code $8A, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreIncw	},	// { code $8B, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreDecw	},	// { code $8C, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreDecw	},	// { code $8D, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreDecw	},	// { code $8E, ST }
    {	(void*)&CUPD1007::StMemoReg,	(void*)&CUPD1007::PreDecw	},	// { code $8F, ST }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreIncw	},	// { code $90, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreIncw	},	// { code $91, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreIncw	},	// { code $92, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreIncw	},	// { code $93, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreDecw	},	// { code $94, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreDecw	},	// { code $95, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreDecw	},	// { code $96, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreDecw	},	// { code $97, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreIncw	},	// { code $98, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreIncw	},	// { code $99, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreIncw	},	// { code $9A, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreIncw	},	// { code $9B, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreDecw	},	// { code $9C, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreDecw	},	// { code $9D, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreDecw	},	// { code $9E, STM }
    {	(void*)&CUPD1007::StmMemoAry,	(void*)&CUPD1007::PreDecw	},	// { code $9F, STM }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostIncw	},	// { code $A0, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostIncw	},	// { code $A1, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostIncw	},	// { code $A2, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostIncw	},	// { code $A3, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostDecw	},	// { code $A4, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostDecw	},	// { code $A5, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostDecw	},	// { code $A6, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostDecw	},	// { code $A7, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostIncw	},	// { code $A8, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostIncw	},	// { code $A9, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostIncw	},	// { code $AA, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostIncw	},	// { code $AB, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostDecw	},	// { code $AC, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostDecw	},	// { code $AD, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostDecw	},	// { code $AE, LD }
    {	(void*)&CUPD1007::LdRegMemo,	(void*)&CUPD1007::PostDecw	},	// { code $AF, LD }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostIncw	},	// { code $B0, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostIncw	},	// { code $B1, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostIncw	},	// { code $B2, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostIncw	},	// { code $B3, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostDecw	},	// { code $B4, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostDecw	},	// { code $B5, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostDecw	},	// { code $B6, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostDecw	},	// { code $B7, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostIncw	},	// { code $B8, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostIncw	},	// { code $B9, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostIncw	},	// { code $BA, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostIncw	},	// { code $BB, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostDecw	},	// { code $BC, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostDecw	},	// { code $BD, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostDecw	},	// { code $BE, LDM }
    {	(void*)&CUPD1007::LdmAryMemo,	(void*)&CUPD1007::PostDecw	},	// { code $BF, LDM }
    {	(void*)&CUPD1007::PstIm8,	(void*)&CUPD1007::OpKo		},		// { code $C0, PST }
    {	(void*)&CUPD1007::PstIm8,	(void*)&CUPD1007::OpIf		},		// { code $C1, PST }
    {	(void*)&CUPD1007::PstIm8,	(void*)&CUPD1007::OpAs		},		// { code $C2, PST }
    {	(void*)&CUPD1007::PstIm8,	(void*)&CUPD1007::OpIe		},		// { code $C3, PST }
    {	(void*)&CUPD1007::PstReg,	(void*)&CUPD1007::OpKo		},		// { code $C4, PST }
    {	(void*)&CUPD1007::PstReg,	(void*)&CUPD1007::OpFl		},		// { code $C5, PST }
    {	(void*)&CUPD1007::PstReg,	(void*)&CUPD1007::OpAs		},		// { code $C6, PST }
    {	(void*)&CUPD1007::PstReg,	(void*)&CUPD1007::OpIe		},		// { code $C7, PST }
    {	(void*)&CUPD1007::StMemoIm8,	(void*)&CUPD1007::PreIncw	},	// { code $C8, ST }
    {	(void*)&CUPD1007::StMemoIm8,	(void*)&CUPD1007::PreIncw	},	// { code $C9, ST }
    {	(void*)&CUPD1007::StMemoIm8,	(void*)&CUPD1007::PreIncw	},	// { code $CA, ST }
    {	(void*)&CUPD1007::StMemoIm8,	(void*)&CUPD1007::PreIncw	},	// { code $CB, ST }
    {	(void*)&CUPD1007::PstReg,	(void*)&CUPD1007::OpKo		},		// { code $CC, PST }
    {	(void*)&CUPD1007::PstReg,	(void*)&CUPD1007::OpFl		},		// { code $CD, PST }
    {	(void*)&CUPD1007::PstReg,	(void*)&CUPD1007::OpAs		},		// { code $CE, PST }
    {	(void*)&CUPD1007::PstReg,	(void*)&CUPD1007::OpIe		},		// { code $CF, PST }
    {	(void*)&CUPD1007::StMemoIm8,	(void*)&CUPD1007::Wreg		},		// { code $D0, ST }
    {	(void*)&CUPD1007::StMemoIm8,	(void*)&CUPD1007::Wreg		},		// { code $D1, ST }
    {	(void*)&CUPD1007::StMemoIm8,	(void*)&CUPD1007::Wreg		},		// { code $D2, ST }
    {	(void*)&CUPD1007::StMemoIm8,	(void*)&CUPD1007::Wreg		},		// { code $D3, ST }
    {	(void*)&CUPD1007::Gst,		(void*)&CUPD1007::Get_koreg		},		// { code $D4, GST }
    {	(void*)&CUPD1007::Gst,		(void*)&CUPD1007::Get_flag		},		// { code $D5, GST }
    {	(void*)&CUPD1007::Gst,		(void*)&CUPD1007::Get_asreg		},		// { code $D6, GST }
    {	(void*)&CUPD1007::Gst,		(void*)&CUPD1007::Get_iereg		},		// { code $D7, GST }
    {	(void*)&CUPD1007::StMemoIm8,	(void*)&CUPD1007::PreDecw	},	// { code $D8, ST }
    {	(void*)&CUPD1007::StMemoIm8,	(void*)&CUPD1007::PreDecw	},	// { code $D9, ST }
    {	(void*)&CUPD1007::StMemoIm8,	(void*)&CUPD1007::PreDecw	},	// { code $DA, ST }
    {	(void*)&CUPD1007::StMemoIm8,	(void*)&CUPD1007::PreDecw	},	// { code $DB, ST }
    {	(void*)&CUPD1007::Gst,		(void*)&CUPD1007::Get_koreg		},		// { code $DC, GST }
    {	(void*)&CUPD1007::Gst,		(void*)&CUPD1007::Get_flag		},		// { code $DD, GST }
    {	(void*)&CUPD1007::Gst,		(void*)&CUPD1007::Get_asreg		},		// { code $DE, GST }
    {	(void*)&CUPD1007::Gst,		(void*)&CUPD1007::Get_iereg		},		// { code $DF, GST }
    {	(void*)&CUPD1007::UnReg,		(void*)&CUPD1007::OpCmp		},		// { code $E0, CMP }
    {	(void*)&CUPD1007::UnAry,		(void*)&CUPD1007::OpCmp		},		// { code $E1, CMPM }
    {	(void*)&CUPD1007::BlockCopy,	(void*)&CUPD1007::PreIncw	},	// { code $E2, BUP }
    {   (void*)&CUPD1007::UnAry,		(void*)&CUPD1007::OpByu		},		// { code $E3, BYUM }
    {	(void*)&CUPD1007::UnReg,		(void*)&CUPD1007::OpCmpb		},		// { code $E4, CMPB }
    {	(void*)&CUPD1007::UnAry,		(void*)&CUPD1007::OpDiu		},		// { code $E5, DIUM }
    {	(void*)&CUPD1007::RotReg,	(void*)&CUPD1007::OpRou		},		// { code $E6, ROU }
    {	(void*)&CUPD1007::RotAry,	(void*)&CUPD1007::OpRou		},		// { code $E7, ROUM }
    {	(void*)&CUPD1007::UnReg,		(void*)&CUPD1007::OpCmp		},		// { code $E8, CMP }
    {	(void*)&CUPD1007::UnAry,		(void*)&CUPD1007::OpCmp		},		// { code $E9, CMPM }
    {	(void*)&CUPD1007::Nop,		NULL		},		// { code $ea, unsupported }
    {   (void*)&CUPD1007::UnAry,		(void*)&CUPD1007::OpByu		},		// { code $EB, BYUM }
    {	(void*)&CUPD1007::UnReg,		(void*)&CUPD1007::OpCmpb		},		// { code $EC, CMPB }
    {	(void*)&CUPD1007::UnAry,		(void*)&CUPD1007::OpDiu		},		// { code $ED, DIUM }
    {	(void*)&CUPD1007::RotReg,	(void*)&CUPD1007::OpRou		},		// { code $EE, ROU }
    {	(void*)&CUPD1007::RotAry,	(void*)&CUPD1007::OpRou		},		// { code $EF, ROUM }
    {	(void*)&CUPD1007::UnReg,		(void*)&CUPD1007::OpInv		},		// { code $F0, INV }
    {	(void*)&CUPD1007::UnAry,		(void*)&CUPD1007::OpInv		},		// { code $F1, INVM }
    {	(void*)&CUPD1007::BlockCopy,	(void*)&CUPD1007::PreDecw	},	// { code $F2, BDN }
    {	(void*)&CUPD1007::UnAry,		(void*)&CUPD1007::OpByd		},		// { code $F3, BYDM }
    {	(void*)&CUPD1007::Mtbm,		(void*)&CUPD1007::OpCmpb		},		// { code $F4, CMPBM }
    {	(void*)&CUPD1007::UnAry,		(void*)&CUPD1007::OpDid		},		// { code $F5, DIDM }
    {	(void*)&CUPD1007::RotReg,	(void*)&CUPD1007::OpRod		},		// { code $F6, ROD }
    {	(void*)&CUPD1007::RotAry,	(void*)&CUPD1007::OpRod		},		// { code $F7, RODM }
    {	(void*)&CUPD1007::UnReg,		(void*)&CUPD1007::OpInv		},		// { code $F8, INV }
    {	(void*)&CUPD1007::UnAry,		(void*)&CUPD1007::OpInv		},		// { code $F9, INVM }
    {	(void*)&CUPD1007::Nop,		NULL		},		// { code $FA, NOP }
    {	(void*)&CUPD1007::UnAry,		(void*)&CUPD1007::OpByd		},		// { code $FB, BYDM }
    {	(void*)&CUPD1007::Mtbm,		(void*)&CUPD1007::OpCmpb		},		// { code $FC, CMPBM }
    {	(void*)&CUPD1007::UnAry,		(void*)&CUPD1007::OpDid		},		// { code $FD, DIDM }
    {	(void*)&CUPD1007::RotReg,	(void*)&CUPD1007::OpRod		},		// { code $FE, ROD }
    {	(void*)&CUPD1007::RotAry,	(void*)&CUPD1007::OpRod		},		// { code $FF, RODM }

        /* bit 7 of the second byte set */
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpAdb		},	//	{ code $00, ADB }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpSbb		},	//	{ code $01, SBB }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpAd		},	//	{ code $02, AD }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpSb		},	//	{ code $03, SB }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpAn		},	//	{ code $04, AN }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpNa		},	//	{ code $05, NA }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpOr		},	//	{ code $06, OR }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpXr		},	//	{ code $07, XR }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpAdb		},	//	{ code $08, ADB }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpSbb		},	//	{ code $09, SBB }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpAd		},	//	{ code $0A, AD }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpSb		},	//	{ code $0B, SB }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpAn		},	//	{ code $0C, AN }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpNa		},	//	{ code $0D, NA }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpOr		},	//	{ code $0E, OR }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpXr		},	//	{ code $0F, XR }
            {	(void*)&CUPD1007::	AryIm6,	(void*)&CUPD1007::	OpAdb		},	//	{ code $10, ADBM }
            {	(void*)&CUPD1007::	AryIm6,	(void*)&CUPD1007::	OpSbb		},	//	{ code $11, SBBM }
            {	(void*)&CUPD1007::	AryIm6,	(void*)&CUPD1007::	OpAd		},	//	{ code $12, ADM }
            {	(void*)&CUPD1007::	AryIm6,	(void*)&CUPD1007::	OpSb		},	//	{ code $13, SBM }
            {	(void*)&CUPD1007::	ExchReg,	(void*)&CUPD1007::	OpSwp2		},	//	{ code $14, SWP }
            {	(void*)&CUPD1007::	ExchAry,	(void*)&CUPD1007::	OpSwp2		},	//	{ code $15, SWPM }
            {	(void*)&CUPD1007::	ExchReg,	(void*)&CUPD1007::	OpXcls		},	//	{ code $16, XCLS }
            {	(void*)&CUPD1007::	ExchAry,	(void*)&CUPD1007::	OpXcls		},	//	{ code $17, XCLSM }
            {	(void*)&CUPD1007::	AryIm6,	(void*)&CUPD1007::	OpAdb		},	//	{ code $18, ADBM }
            {	(void*)&CUPD1007::	AryIm6,	(void*)&CUPD1007::	OpSbb		},	//	{ code $19, SBBM }
            {	(void*)&CUPD1007::	AryIm6,	(void*)&CUPD1007::	OpAd		},	//	{ code $1A, ADM }
            {	(void*)&CUPD1007::	AryIm6,	(void*)&CUPD1007::	OpSb		},	//	{ code $1B, SBM }
            {	(void*)&CUPD1007::	ExchReg,	(void*)&CUPD1007::	OpXc		},	//	{ code $1C, XC }
            {	(void*)&CUPD1007::	ExchAry,	(void*)&CUPD1007::	OpXc		},	//	{ code $1D, XCM }
            {	(void*)&CUPD1007::	ExchReg,	(void*)&CUPD1007::	OpXchs		},	//	{ code $1E, XCHS }
            {	(void*)&CUPD1007::	ExchAry,	(void*)&CUPD1007::	OpXchs		},	//	{ code $1F, XCHSM }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpAdb		},	//	{ code $20, TADB }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpSbb		},	//	{ code $21, TSBB }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpAd		},	//	{ code $22, TAD }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpSb		},	//	{ code $23, TSB }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpAn		},	//	{ code $24, TAN }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpNa		},	//	{ code $25, TNA }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpOr		},	//	{ code $26, TOR }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpXr		},	//	{ code $27, TXR }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpAdb		},	//	{ code $28, TADB }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpSbb		},	//	{ code $29, TSBB }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpAd		},	//	{ code $2A, TAD }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpSb		},	//	{ code $2B, TSB }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpAn		},	//	{ code $2C, TAN }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpNa		},	//	{ code $2D, TNA }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpOr		},	//	{ code $2E, TOR }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpXr		},	//	{ code $2F, TXR }
            {	(void*)&CUPD1007::	TAryIm6,	(void*)&CUPD1007::	OpAdb		},	//	{ code $30, TADBM }
            {	(void*)&CUPD1007::	TAryIm6,	(void*)&CUPD1007::	OpSbb		},	//	{ code $31, TSBBM }
            {	(void*)&CUPD1007::	TAryIm6,	(void*)&CUPD1007::	OpAd		},	//	{ code $32, TADM }
            {	(void*)&CUPD1007::	TAryIm6,	(void*)&CUPD1007::	OpSb		},	//	{ code $33, TSBM }
            {	(void*)&CUPD1007::	BlockSearch,	(void*)&CUPD1007::	PreIncw	},//	{ code $34, SUP }
            {	(void*)&CUPD1007::	BlockSearch,	(void*)&CUPD1007::	PreIncw	},//	{ code $35, search +(IY) }
            {	(void*)&CUPD1007::	BlockSearch,	(void*)&CUPD1007::	PreIncw	},//	{ code $36, search +(IZ) }
            {	(void*)&CUPD1007::	BlockSearch,	(void*)&CUPD1007::	PreIncw	},//	{ code $37, search +(SP) }
            {	(void*)&CUPD1007::	TAryIm6,	(void*)&CUPD1007::	OpAdb		},	//	{ code $38, TADBM }
            {	(void*)&CUPD1007::	TAryIm6,	(void*)&CUPD1007::	OpSbb		},	//	{ code $39, TSBBM }
            {	(void*)&CUPD1007::	TAryIm6,	(void*)&CUPD1007::	OpAd		},	//	{ code $3A, TADM }
            {	(void*)&CUPD1007::	TAryIm6,	(void*)&CUPD1007::	OpSb		},	//	{ code $3B, TSBM }
            {	(void*)&CUPD1007::	BlockSearch,	(void*)&CUPD1007::	PreDecw	},//	{ code $3C, SDN }
            {	(void*)&CUPD1007::	BlockSearch,	(void*)&CUPD1007::	PreDecw	},//	{ code $3D, search -(IY) }
            {	(void*)&CUPD1007::	BlockSearch,	(void*)&CUPD1007::	PreDecw	},//	{ code $3E, search -(IZ) }
            {	(void*)&CUPD1007::	BlockSearch,	(void*)&CUPD1007::	PreDecw	},//	{ code $3F, search -(SP) }
            {	(void*)&CUPD1007::	Ldw,		NULL		},	//	{ code $40, LDW }
            {	(void*)&CUPD1007::	Ldw,		NULL		},	//	{ code $41, LDW }
            {	(void*)&CUPD1007::	Ldw,		NULL		},	//	{ code $42, LDW }
            {	(void*)&CUPD1007::	Ldw,		NULL		},	//	{ code $43, LDW }
            {	(void*)&CUPD1007::	Ldw,		NULL		},	//	{ code $44, LDW }
            {	(void*)&CUPD1007::	Ldw,		NULL		},	//	{ code $45, LDW }
            {	(void*)&CUPD1007::	Ldw,		NULL		},	//	{ code $46, LDW }
            {	(void*)&CUPD1007::	Ldw,		NULL		},	//	{ code $47, LDW }
            {	(void*)&CUPD1007::	AdwSbw,	(void*)&CUPD1007::	PreIncw	},//	{ code $48, ADW }
            {	(void*)&CUPD1007::	AdwSbw,	(void*)&CUPD1007::	PreIncw	},//	{ code $49, ADW }
            {	(void*)&CUPD1007::	AdwSbw,	(void*)&CUPD1007::	PreIncw	},//	{ code $4A, ADW }
            {	(void*)&CUPD1007::	AdwSbw,	(void*)&CUPD1007::	PreIncw	},//	{ code $4B, ADW }
            {	(void*)&CUPD1007::	AdwSbw,	(void*)&CUPD1007::	PreDecw	},//	{ code $4C, SBW }
            {	(void*)&CUPD1007::	AdwSbw,	(void*)&CUPD1007::	PreDecw	},//	{ code $4D, SBW }
            {	(void*)&CUPD1007::	AdwSbw,	(void*)&CUPD1007::	PreDecw	},//	{ code $4E, SBW }
            {	(void*)&CUPD1007::	AdwSbw,	(void*)&CUPD1007::	PreDecw	},//	{ code $4F, SBW }
            {	(void*)&CUPD1007::	Off,		NULL		},	//	{ code $50, OFF }
            {	(void*)&CUPD1007::	Gst,		(void*)&CUPD1007::	Get_kireg		},	//	{ code $51, GST }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpBit		},	//	{ code $52, BIT }
            {	(void*)&CUPD1007::	TXreg,		(void*)&CUPD1007::	OpBit		},	//	{ code $53, BIT }
            {	(void*)&CUPD1007::	Xreg,		(void*)&CUPD1007::	OpLd		},	//	{ code $54, LD }
            {	(void*)&CUPD1007::	Xary,		(void*)&CUPD1007::	OpLd		},	//	{ code $55, LDM }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpLd		},	//	{ code $56, LD }
            {	(void*)&CUPD1007::	AryIm6,	(void*)&CUPD1007::	OpLd		},	//	{ code $57, LDM }
            {	(void*)&CUPD1007::	Rtn,		NULL		},	//	{ code $58, RTN }
            {	(void*)&CUPD1007::	Gst,		(void*)&CUPD1007::	Get_kireg		},	//	{ code $59, GST }
            {	(void*)&CUPD1007::	TRegIm8,	(void*)&CUPD1007::	OpBit		},	//	{ code $5A, BIT }
            {	(void*)&CUPD1007::	TYreg,		(void*)&CUPD1007::	OpBit		},	//	{ code $5B, BIT }
            {	(void*)&CUPD1007::	Yreg,		(void*)&CUPD1007::	OpLd		},	//	{ code $5C, LD }
            {	(void*)&CUPD1007::	Yary,		(void*)&CUPD1007::	OpLd		},	//	{ code $5D, LDM }
            {	(void*)&CUPD1007::	RegIm8,	(void*)&CUPD1007::	OpLd		},	//	{ code $5E, LD }
            {	(void*)&CUPD1007::	AryIm6,	(void*)&CUPD1007::	OpLd		},	//	{ code $5F, LDM }
            {	(void*)&CUPD1007::	NotCond,	(void*)&CUPD1007::	Call		},	//	{ code $60, CAL }
            {	(void*)&CUPD1007::	KeyCond,	(void*)&CUPD1007::	Call		},	//	{ code $61, CAL }
            {	(void*)&CUPD1007::	Cond,		(void*)&CUPD1007::	Call		},	//	{ code $62, CAL }
            {	(void*)&CUPD1007::	Cond,		(void*)&CUPD1007::	Call		},	//	{ code $63, CAL }
            {	(void*)&CUPD1007::	Cond,		(void*)&CUPD1007::	Call		},	//	{ code $64, CAL }
            {	(void*)&CUPD1007::	Cond,		(void*)&CUPD1007::	Call		},	//	{ code $65, CAL }
            {	(void*)&CUPD1007::	Cond,		(void*)&CUPD1007::	Call		},	//	{ code $66, CAL }
            {	(void*)&CUPD1007::	Cond,		(void*)&CUPD1007::	Call		},	//	{ code $67, CAL }
            {	(void*)&CUPD1007::	Cani,		NULL		},	//	{ code $68, CANI }
            {	(void*)&CUPD1007::	NotKeyCond,	(void*)&CUPD1007::	Call		},	//	{ code $69, CAL }
            {	(void*)&CUPD1007::	NotCond,	(void*)&CUPD1007::	Call		},	//	{ code $6A, CAL }
            {	(void*)&CUPD1007::	NotCond,	(void*)&CUPD1007::	Call		},	//	{ code $6B, CAL }
            {	(void*)&CUPD1007::	NotCond,	(void*)&CUPD1007::	Call		},	//	{ code $6C, CAL }
            {	(void*)&CUPD1007::	NotCond,	(void*)&CUPD1007::	Call		},	//	{ code $6D, CAL }
            {	(void*)&CUPD1007::	NotCond,	(void*)&CUPD1007::	Call		},	//	{ code $6E, CAL }
            {	(void*)&CUPD1007::	NotCond,	(void*)&CUPD1007::	Call		},	//	{ code $6F, CAL }
            {	(void*)&CUPD1007::	NotCond,	(void*)&CUPD1007::	Jump		},	//	{ code $70, JMP }
            {	(void*)&CUPD1007::	KeyCond,	(void*)&CUPD1007::	Jump		},	//	{ code $71, JMP }
            {	(void*)&CUPD1007::	Cond,		(void*)&CUPD1007::	Jump		},	//	{ code $72, JMP }
            {	(void*)&CUPD1007::	Cond,		(void*)&CUPD1007::	Jump		},	//	{ code $73, JMP }
            {	(void*)&CUPD1007::	Cond,		(void*)&CUPD1007::	Jump		},	//	{ code $74, JMP }
            {	(void*)&CUPD1007::	Cond,		(void*)&CUPD1007::	Jump		},	//	{ code $75, JMP }
            {	(void*)&CUPD1007::	Cond,		(void*)&CUPD1007::	Jump		},	//	{ code $76, JMP }
            {	(void*)&CUPD1007::	Cond,		(void*)&CUPD1007::	Jump		},	//	{ code $77, JMP }
            {	(void*)&CUPD1007::	Rti,		NULL		},	//	{ code $78, RTI }
            {	(void*)&CUPD1007::	NotKeyCond,	(void*)&CUPD1007::	Jump		},	//	{ code $79, JMP }
            {	(void*)&CUPD1007::	NotCond,	(void*)&CUPD1007::	Jump		},	//	{ code $7A, JMP }
            {	(void*)&CUPD1007::	NotCond,	(void*)&CUPD1007::	Jump		},	//	{ code $7B, JMP }
            {	(void*)&CUPD1007::	NotCond,	(void*)&CUPD1007::	Jump		},	//	{ code $7C, JMP }
            {	(void*)&CUPD1007::	NotCond,	(void*)&CUPD1007::	Jump		},	//	{ code $7D, JMP }
            {	(void*)&CUPD1007::	NotCond,	(void*)&CUPD1007::	Jump		},	//	{ code $7E, JMP }
            {	(void*)&CUPD1007::	NotCond,	(void*)&CUPD1007::	Jump		},	//	{ code $7F, JMP }
            {	(void*)&CUPD1007::	StRegOffsReg,	(void*)&CUPD1007::	PlusOffset	},//	{ code $80, ST }
            {	(void*)&CUPD1007::	StRegOffsReg,	(void*)&CUPD1007::	PlusOffset	},//	{ code $81, ST }
            {	(void*)&CUPD1007::	StRegOffsReg,	(void*)&CUPD1007::	PlusOffset	},//	{ code $82, ST }
            {	(void*)&CUPD1007::	StRegOffsReg,	(void*)&CUPD1007::	PlusOffset	},//	{ code $83, ST }
            {	(void*)&CUPD1007::	StRegOffsReg,	(void*)&CUPD1007::	MinusOffset	},//	{ code $84, ST }
            {	(void*)&CUPD1007::	StRegOffsReg,	(void*)&CUPD1007::	MinusOffset	},//	{ code $85, ST }
            {	(void*)&CUPD1007::	StRegOffsReg,	(void*)&CUPD1007::	MinusOffset	},//	{ code $86, ST }
            {	(void*)&CUPD1007::	StRegOffsReg,	(void*)&CUPD1007::	MinusOffset	},//	{ code $87, ST }
            {	(void*)&CUPD1007::	StImOffsReg,	(void*)&CUPD1007::	PlusOffset	},//	{ code $88, ST }
            {	(void*)&CUPD1007::	StImOffsReg,	(void*)&CUPD1007::	PlusOffset	},//	{ code $89, ST }
            {	(void*)&CUPD1007::	StImOffsReg,	(void*)&CUPD1007::	PlusOffset	},//	{ code $8A, ST }
            {	(void*)&CUPD1007::	StImOffsReg,	(void*)&CUPD1007::	PlusOffset	},//	{ code $8B, ST }
            {	(void*)&CUPD1007::	StImOffsReg,	(void*)&CUPD1007::	MinusOffset	},//	{ code $8C, ST }
            {	(void*)&CUPD1007::	StImOffsReg,	(void*)&CUPD1007::	MinusOffset	},//	{ code $8D, ST }
            {	(void*)&CUPD1007::	StImOffsReg,	(void*)&CUPD1007::	MinusOffset	},//	{ code $8E, ST }
            {	(void*)&CUPD1007::	StImOffsReg,	(void*)&CUPD1007::	MinusOffset	},//	{ code $8F, ST }
            {	(void*)&CUPD1007::	StmRegOffsAry,	(void*)&CUPD1007::	PlusOffset	},//	{ code $90, STM }
            {	(void*)&CUPD1007::	StmRegOffsAry,	(void*)&CUPD1007::	PlusOffset	},//	{ code $91, STM }
            {	(void*)&CUPD1007::	StmRegOffsAry,	(void*)&CUPD1007::	PlusOffset	},//	{ code $92, STM }
            {	(void*)&CUPD1007::	StmRegOffsAry,	(void*)&CUPD1007::	PlusOffset	},//	{ code $93, STM }
            {	(void*)&CUPD1007::	StmRegOffsAry,	(void*)&CUPD1007::	MinusOffset	},//	{ code $94, STM }
            {	(void*)&CUPD1007::	StmRegOffsAry,	(void*)&CUPD1007::	MinusOffset	},//	{ code $95, STM }
            {	(void*)&CUPD1007::	StmRegOffsAry,	(void*)&CUPD1007::	MinusOffset	},//	{ code $96, STM }
            {	(void*)&CUPD1007::	StmRegOffsAry,	(void*)&CUPD1007::	MinusOffset	},//	{ code $97, STM }
            {	(void*)&CUPD1007::	StmImOffsAry,	(void*)&CUPD1007::	PlusOffset	},//	{ code $98, STM }
            {	(void*)&CUPD1007::	StmImOffsAry,	(void*)&CUPD1007::	PlusOffset	},//	{ code $99, STM }
            {	(void*)&CUPD1007::	StmImOffsAry,	(void*)&CUPD1007::	PlusOffset	},//	{ code $9A, STM }
            {	(void*)&CUPD1007::	StmImOffsAry,	(void*)&CUPD1007::	PlusOffset	},//	{ code $9B, STM }
            {	(void*)&CUPD1007::	StmImOffsAry,	(void*)&CUPD1007::	MinusOffset	},//	{ code $9C, STM }
            {	(void*)&CUPD1007::	StmImOffsAry,	(void*)&CUPD1007::	MinusOffset	},//	{ code $9D, STM }
            {	(void*)&CUPD1007::	StmImOffsAry,	(void*)&CUPD1007::	MinusOffset	},//	{ code $9E, STM }
            {	(void*)&CUPD1007::	StmImOffsAry,	(void*)&CUPD1007::	MinusOffset	},//	{ code $9F, STM }
            {	(void*)&CUPD1007::	LdRegRegOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $A0, LD }
            {	(void*)&CUPD1007::	LdRegRegOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $A1, LD }
            {	(void*)&CUPD1007::	LdRegRegOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $A2, LD }
            {	(void*)&CUPD1007::	LdRegRegOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $A3, LD }
            {	(void*)&CUPD1007::	LdRegRegOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $A4, LD }
            {	(void*)&CUPD1007::	LdRegRegOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $A5, LD }
            {	(void*)&CUPD1007::	LdRegRegOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $A6, LD }
            {	(void*)&CUPD1007::	LdRegRegOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $A7, LD }
            {	(void*)&CUPD1007::	LdRegImOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $A8, LD }
            {	(void*)&CUPD1007::	LdRegImOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $A9, LD }
            {	(void*)&CUPD1007::	LdRegImOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $AA, LD }
            {	(void*)&CUPD1007::	LdRegImOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $AB, LD }
            {	(void*)&CUPD1007::	LdRegImOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $AC, LD }
            {	(void*)&CUPD1007::	LdRegImOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $AD, LD }
            {	(void*)&CUPD1007::	LdRegImOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $AE, LD }
            {	(void*)&CUPD1007::	LdRegImOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $AF, LD }
            {	(void*)&CUPD1007::	LdmAryRegOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $B0, LDM }
            {	(void*)&CUPD1007::	LdmAryRegOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $B1, LDM }
            {	(void*)&CUPD1007::	LdmAryRegOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $B2, LDM }
            {	(void*)&CUPD1007::	LdmAryRegOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $B3, LDM }
            {	(void*)&CUPD1007::	LdmAryRegOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $B4, LDM }
            {	(void*)&CUPD1007::	LdmAryRegOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $B5, LDM }
            {	(void*)&CUPD1007::	LdmAryRegOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $B6, LDM }
            {	(void*)&CUPD1007::	LdmAryRegOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $B7, LDM }
            {	(void*)&CUPD1007::	LdmAryImOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $B8, LDM }
            {	(void*)&CUPD1007::	LdmAryImOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $B9, LDM }
            {	(void*)&CUPD1007::	LdmAryImOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $BA, LDM }
            {	(void*)&CUPD1007::	LdmAryImOffs,	(void*)&CUPD1007::	PlusOffset	},//	{ code $BB, LDM }
            {	(void*)&CUPD1007::	LdmAryImOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $BC, LDM }
            {	(void*)&CUPD1007::	LdmAryImOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $BD, LDM }
            {	(void*)&CUPD1007::	LdmAryImOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $BE, LDM }
            {	(void*)&CUPD1007::	LdmAryImOffs,	(void*)&CUPD1007::	MinusOffset	},//	{ code $BF, LDM }
            {	(void*)&CUPD1007::	PstIm8,	(void*)&CUPD1007::	OpKo		},	//	{ code $C0, PST }
            {	(void*)&CUPD1007::	PstIm8,	(void*)&CUPD1007::	OpIf		},	//	{ code $C1, PST }
            {	(void*)&CUPD1007::	PstIm8,	(void*)&CUPD1007::	OpAs		},	//	{ code $C2, PST }
            {	(void*)&CUPD1007::	PstIm8,	(void*)&CUPD1007::	OpIe		},	//	{ code $C3, PST }
            {	(void*)&CUPD1007::	Ldle,		NULL		},	//	{ code $C4, LDLE }
            {	(void*)&CUPD1007::	Ldlo,		NULL		},	//	{ code $C5, LDLO }
            {	(void*)&CUPD1007::	Stle,		NULL		},	//	{ code $C6, STLE }
            {	(void*)&CUPD1007::	Stlo,		NULL		},	//	{ code $C7, STLO }
            {	(void*)&CUPD1007::	StMemoIm8,	(void*)&CUPD1007::	PreIncw	},//	{ code $C8, ST }
            {	(void*)&CUPD1007::	StMemoIm8,	(void*)&CUPD1007::	PreIncw	},//	{ code $C9, ST }
            {	(void*)&CUPD1007::	StMemoIm8,	(void*)&CUPD1007::	PreIncw	},//	{ code $CA, ST }
            {	(void*)&CUPD1007::	StMemoIm8,	(void*)&CUPD1007::	PreIncw	},//	{ code $CB, ST }
            {	(void*)&CUPD1007::	Ldle,		NULL		},	//	{ code $CC, LDLE }
            {	(void*)&CUPD1007::	Ldlo,		NULL		},	//	{ code $CD, LDLO }
            {	(void*)&CUPD1007::	Stle,		NULL		},	//	{ code $CE, STLE }
            {	(void*)&CUPD1007::	Stlo,		NULL		},	//	{ code $CF, STLO }
            {	(void*)&CUPD1007::	StMemoIm8,	(void*)&CUPD1007::	Wreg		},	//	{ code $D0, ST }
            {	(void*)&CUPD1007::	StMemoIm8,	(void*)&CUPD1007::	Wreg		},	//	{ code $D1, ST }
            {	(void*)&CUPD1007::	StMemoIm8,	(void*)&CUPD1007::	Wreg		},	//	{ code $D2, ST }
            {	(void*)&CUPD1007::	StMemoIm8,	(void*)&CUPD1007::	Wreg		},	//	{ code $D3, ST }
            {	(void*)&CUPD1007::	Ldlem,		NULL		},	//	{ code $D4, LDLEM }
            {	(void*)&CUPD1007::	Ldlom,		NULL		},	//	{ code $D5, LDLOM }
            {	(void*)&CUPD1007::	Stlem,		NULL		},	//	{ code $D6, STLEM }
            {	(void*)&CUPD1007::	Stlom,		NULL		},	//	{ code $D7, STLOM }
            {	(void*)&CUPD1007::	StMemoIm8,	(void*)&CUPD1007::	PreDecw	},//	{ code $D8, ST }
            {	(void*)&CUPD1007::	StMemoIm8,	(void*)&CUPD1007::	PreDecw	},//	{ code $D9, ST }
            {	(void*)&CUPD1007::	StMemoIm8,	(void*)&CUPD1007::	PreDecw	},//	{ code $DA, ST }
            {	(void*)&CUPD1007::	StMemoIm8,	(void*)&CUPD1007::	PreDecw	},//	{ code $DB, ST }
            {	(void*)&CUPD1007::	Ldlem,		NULL		},	//	{ code $DC, LDLEM }
            {	(void*)&CUPD1007::	Ldlom,		NULL		},	//	{ code $DD, LDLOM }
            {	(void*)&CUPD1007::	Stlem,		NULL		},	//	{ code $DE, STLEM }
            {	(void*)&CUPD1007::	Stlom,		NULL		},	//	{ code $DF, STLOM }
            {	(void*)&CUPD1007::	UnReg,		(void*)&CUPD1007::	OpSwp1		},	//	{ code $E0, SWP }
            {	(void*)&CUPD1007::	Nop,		NULL		},	//	{ code $E1, unsupported }
            {	(void*)&CUPD1007::	BlockCopy,	(void*)&CUPD1007::	PreIncw	},//	{ code $E2, BUP }
            {	(void*)&CUPD1007::	UnReg,		(void*)&CUPD1007::	OpByu		},	//	{ code $E3, BYU }
            {	(void*)&CUPD1007::	UnReg,		(void*)&CUPD1007::	OpRou		},	//	{ code $E4, BIU }
            {	(void*)&CUPD1007::	UnReg,		(void*)&CUPD1007::	OpDiu		},	//	{ code $E5, DIU }
            {	(void*)&CUPD1007::	UnReg,		(void*)&CUPD1007::	OpMtb		},	//	{ code $E6, MTB }
            {	(void*)&CUPD1007::	Trap,		NULL		},	//	{ code $E7, TRP }
            {	(void*)&CUPD1007::	UnReg,		(void*)&CUPD1007::	OpSwp1		},	//	{ code $E8, SWP }
            {	(void*)&CUPD1007::	Nop,		NULL		},	//	{ code $E9, unsupported }
            {	(void*)&CUPD1007::	Nop,		NULL		},	//	{ code $ea, unsupported }
            {	(void*)&CUPD1007::	UnReg,		(void*)&CUPD1007::	OpByu		},	//	{ code $EB, BYU }
            {	(void*)&CUPD1007::	UnReg,		(void*)&CUPD1007::	OpRou		},	//	{ code $EC, BIU }
            {	(void*)&CUPD1007::	UnReg,		(void*)&CUPD1007::	OpDiu		},	//	{ code $ED, DIU }
            {	(void*)&CUPD1007::	UnReg,		(void*)&CUPD1007::	OpMtb		},	//	{ code $EE, MTB }
            {	(void*)&CUPD1007::	Trap,		NULL		},	//	{ code $EF, TRP }
            {	(void*)&CUPD1007::	UnAry,		(void*)&CUPD1007::	OpBnus		},	//	{ code $F0, BNUSM }
            {	(void*)&CUPD1007::	Gst,		(void*)&CUPD1007::	Get_ifreg		},	//	{ code $F1, GST }
            {	(void*)&CUPD1007::	BlockCopy,	(void*)&CUPD1007::	PreDecw	},//	{ code $F2, BDN }
            {   (void*)&CUPD1007::	UnReg,		(void*)&CUPD1007::	OpByd		},	//	{ code $F3, BYD }
            {	(void*)&CUPD1007::	UnAry,		(void*)&CUPD1007::	OpRou		},	//	{ code $F4, BIUM }
            {	(void*)&CUPD1007::	UnReg,		(void*)&CUPD1007::	OpDid		},	//	{ code $F5, DID }
            {	(void*)&CUPD1007::	Mtbm,		(void*)&CUPD1007::	OpMtb		},	//	{ code $F6, MTBM }
            {	(void*)&CUPD1007::	Trap,		NULL		},	//	{ code $F7, TRP }
            {	(void*)&CUPD1007::	UnAry,		(void*)&CUPD1007::	OpBnus		},	//	{ code $F8, BNUSM }
            {	(void*)&CUPD1007::	Gst,		(void*)&CUPD1007::	Get_ifreg		},	//	{ code $F9, GST }
            {	(void*)&CUPD1007::	Nop,		NULL		},	//	{ code $FA, NOP }
            {   (void*)&CUPD1007::	UnReg,		(void*)&CUPD1007::	OpByd		},	//	{ code $FB, BYD }
            {	(void*)&CUPD1007::	UnAry,		(void*)&CUPD1007::	OpRou		},	//	{ code $FC, BIUM }
            {	(void*)&CUPD1007::	UnReg,		(void*)&CUPD1007::	OpDid		},	//	{ code $FD, DID }
            {	(void*)&CUPD1007::	Mtbm,		(void*)&CUPD1007::	OpMtb		},	//	{ code $FE, MTBM }
            {	(void*)&CUPD1007::	Trap,		NULL		}//	{ code $FF, TRP }
};
#endif


void CUPD1007::ExecInstr()
{
    UINT16 kod = Fetchopcode();
    reginfo.regbank = ((~reginfo.opcode[0]) << 3) & 0x40;	//{ default value }
    reginfo.regstep = 1;					//{ default value }
    ((Proc2) dtab[kod][0]) (&reginfo,dtab[kod][1]);
}

void CUPD1007::DoPorts(upd1007_config *info)
{
/* the EN1 output controls the LCD power supply */
  if ((info->ifreg & EN1_bit) != 0) {
    info->lcdctrl = info->lcdctrl | VDD2_bit;
//    info->pPC->out(0,info->lcdctrl);
  }
  else
  {
//    if ((info->lcdctrl & VDD2_bit) != 0) LcdInit();
    info->lcdctrl = info->lcdctrl & ~VDD2_bit;
//    info->pPC->out(0,info->lcdctrl);
    info->ifreg = info->ifreg & ~INT_input[1];
   }
/* the EN2 output is wired to the INT0 input */
  if ((info->ifreg & EN2_bit) != 0)
  {
    if ((info->ifreg & INT_input[0]) == 0) IntReq(info,0);
    info->ifreg = info->ifreg | INT_input[0];
  }
  else
    info->ifreg = info->ifreg & ~INT_input[0];
}

/* interrupt request */
void CUPD1007::IntReq (upd1007_config *info,int i) {
    if (((info->iereg & INT_enable[i]) != 0) && (info->irqcnt[i] == 0))
        info->irqcnt[i] = -INT_LATENCY-1;	/* any value less than -INT_LATENCY */
}

#endif
