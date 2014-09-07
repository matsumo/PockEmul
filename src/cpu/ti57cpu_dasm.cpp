#include <QDebug>

#include "Debug.h"
#include "ti57cpu.h"

QString Cdebug_ti57cpu::IntToHex(int val,int nb) {
    return QString("%1").arg(val,nb,16,QChar('0'));
}

QString Cdebug_ti57cpu::BranchOP(TI57regs *r) {
    QString Result="";
    if (r->OP & 0x0400) Result="BRC ";
    else Result="BRNC ";
    Result.append(IntToHex((r->PC & 0x0400) | (r->OP & 0x03FF)));
    return Result;
}

QString Cdebug_ti57cpu::CallOP(TI57regs *r) {
  return "CALL "+IntToHex(r->OP & 0x07FF);
}

QString Cdebug_ti57cpu::FlagOP(TI57regs *r) {

UINT8 F,B,D,J;
QString Result="";

J=(r->OP & 0xC0) >> 6;
D=(r->OP & 0x30) >> 4;
B=(r->OP & 0x0C) >> 2;
F= r->OP & 0x03;
  switch(J) {
    case 0: Result="A"; break;
    case 1: Result="B"; break;
    case 2: Result="C"; break;
    case 3: Result="D"; break;
  }

  if (D==0) {
      Result="Error: Flag Operation with D=0";
      // Exit
  }
  else Result.append(QString("%1").arg(D+12));

  switch(F) {
    case 0: Result.prepend("SF "); break;
    case 1: Result.prepend("ZF "); break;
    case 2: Result.prepend("?TF "); break;
    case 3: Result.prepend("XF "); break;
  }
  Result .append(QString(" %1").arg(B));

  return Result;
}


QString Cdebug_ti57cpu::MaskOP(TI57regs *r) {

    UINT8 J,K,L,LN,N;
    QString JR;
    QString M;
    QString Result="";

    JR="?";
    switch( r->MF) {
    case 0x01: M="ALL"; break;
    case 0x02: M="MANT"; break;
    case 0x05: M="EXP"; break;
    case 0x00: M="MMSD"; break;
    case 0x03: M="MAEX"; break;
    case 0x04: M="LLSD"; break;
    case 0x07: M="FMAEX"; break;
    case 0x09: M="FLAG"; break;
    case 0x0D: M="DGT13"; break;
    case 0x08: M="DGT14"; break;
    case 0x0F: M="DGT15"; break;
    case 0x0A: M="DIGIT"; break;
    default: Result="Error: Invalid Mask Field";
        //Exit
        break;
    }

    J=(r->OP & 0xC0) >> 6;
    K=(r->OP & 0x38) >> 3;
    LN=r->OP & 0x07;
    L=LN >> 1;
    N=LN & 0x01;

    switch (J) {
    case 0: JR="A"; break;
    case 1: JR="B"; break;
    case 2: JR="C"; break;
    case 3: JR="D"; break;
    }
    if (L==3) {
        if (N==0) {
            if ((J==0) && (K <4)) {
                switch(K) {
                case 0: Result="AAEX"; break;
                case 1: Result="ABEX"; break;
                case 2: Result="ACEX"; break;
                case 3: Result="ADEX"; break;
                }
            }
            else
            {
                Result="Error: Invalid Exchange Register";
                //Exit
            }
        }
        else
            switch(K) {
            case 0: Result=JR+"=A"; break;
            case 1: Result=JR+"=B"; break;
            case 2: Result=JR+"=C"; break;
            case 3: Result=JR+"=D"; break;
            case 4: Result=JR+"=1"; break;
            case 6: Result=JR+"=R5L"; break;
            case 7: Result=JR+"=R5"; break;
        default:Result="Error: Invalid Store Register";
          //Exit
          break;
      }
  }
  else
  {
    switch(K) {
      case 0: if (N==0) Result=JR+"+A"; else Result=JR+"-A"; break;
      case 1: if (N==0) Result=JR+"+B"; else Result=JR+"-B"; break;
      case 2: if (N==0) Result=JR+"+C"; else Result=JR+"-C"; break;
      case 3: if (N==0) Result=JR+"+D"; else Result=JR+"-D"; break;
      case 4: if (N==0) Result=JR+"+1"; else Result=JR+"-1"; break;
      case 5: if (N==0) Result=JR+"SL"; else Result=JR+"SR"; break;
      case 6: if (N==0) Result=JR+"+R5L"; else Result=JR+"-R5L"; break;
      case 7: if (N==0) Result=JR+"+R5"; else Result=JR+"-R5"; break;
    }
    switch(L) {
      case 0: Result.prepend(JR+"="); break;
      case 1: switch (K) {
           case 0: Result.prepend("A="); break;
           case 1: Result.prepend("B="); break;
           case 2: Result.prepend("C="); break;
           case 3: Result.prepend("D="); break;
           default: Result="Error: Invalid Arithmetic Result Register";
            //Exit
            break;
        }
      case 2: if (K >5) {
            Result="Error: Invalid Suppress Operation";
            //Exit
        }
        break;
    }
  }
  Result.append(" "+M);

  return Result;
}

QString Cdebug_ti57cpu::MiscOP(TI57regs *r) {
UINT8 P,Q;
QString Result="";
P = r->OP & 0x0F;
Q = (r->OP & 0xF0) >> 4;
  switch(P) {
    case 0x00: Result="STYA"; break;
    case 0x01: Result.append("RAB %1").arg(Q & 0x07); break;
    case 0x02: Result="BR R5"; break;
    case 0x03: Result="RTN"; break;
    case 0x04: Result="STAX"; break;
    case 0x05: Result="STXA"; break;
    case 0x06: Result="STAY"; break;
    case 0x07: Result="DISP"; break;
    case 0x08: Result="BCDS"; break;
    case 0x09: Result="BCDR"; break;
    case 0x0A: Result="RAB R5"; break;
    case 0x0B: Result="POWOFF"; break;
    case 0x0C: Result="STAX MAEX"; break;
    case 0x0D: Result="?KEY"; break;
    case 0x0E: Result="STAY MAEX"; break;
    default: if (r->OP==0x0EFF) Result="NOP";
        else Result="Error: Invalid Misc Instruction";
      break;
  }

  return Result;
}

QString Cdebug_ti57cpu::Decode(TI57regs *r) {
    QString Result;

//    qWarning()<<QString("OP:%1").arg(r->OP,4,16,QChar('0'));

    if (r->OP & 0x1000) {
        if (r->OP & 0x0800) Result=BranchOP(r);
        else Result=CallOP(r);
    }
    else {
        r->MF = (r->OP >> 8);
        switch (r->MF) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 7:
        case 8:
        case 9:
        case 0x0a:
        case 0x0d:
        case 0x0f: Result = MaskOP(r); break;
        case 0x0C: Result = FlagOP(r); break;
        case 0x0E: Result = MiscOP(r); break;
        default: Result="Error: Invalid Instruction"; break;
        }
    }
    Result.prepend(IntToHex(r->PC).append(" ").append(IntToHex(r->OP,4)).append(" "));

    return Result;
}

QString Cdebug_ti57cpu::Reg(TRegister R) {
    QString Result="";
  for (int i=15;i>=0;i--) Result.append(IntToHex(R[i],1));

  return Result;
}

void Cdebug_ti57cpu::Analyze(TI57regs *r) {

  int a[16484];

  memset(a,0,sizeof(a));
  for (int i=0; i<= MAXROM;i++)
      if ( (i>=0x400) && (pPC->Get_PC(i)>=0x1800)) a[8192+pPC->Get_PC(i)]++;
      else a[pPC->Get_PC(i)]++;
//  AssignFile(f,'D:\Temp\OP.A'); Rewrite(f);
  QFile file("OP.trc");
  file.open(QIODevice::WriteOnly);
  for (int i=0;i<16384;i++) {
    if (i>=8192)
        r->PC=0x400;
    else
        r->PC=0;
    if (a[i]!=0) {
        r->OP=i & 0x1FFF;
        file.write(Decode(r).toLatin1());
    }
  }
  r->PC=0x0052;
  file.close();
}

void Cdebug_ti57cpu::Disassemble(TI57regs *r) {

    QFile file("TI57.dsm");
    file.open(QIODevice::WriteOnly);
    r->PC=0;
    do {
        r->OP=pPC->Get_PC(r->PC);
        file.write(Decode(r).append("\n").toLatin1());
        r->PC++;
    } while (r->PC<MAXROM);
    r->PC=0;
    file.close();
}

  QString Cdebug_ti57cpu::Debugging(TI57regs *r) {
      QString s="";
  s=QString("A\t= ").append(Reg(r->RA)).append("\n");
  s.append("B\t= ").append(Reg(r->RB)).append("\n");
  s.append("C\t= ").append(Reg(r->RC)).append("\n");
  s.append("D\t= ").append(Reg(r->RD)).append("\n");
  s.append("COND\t= %1").arg(r->COND).append("\n");
  s.append("BASE\t= %1\n").arg(r->BASE);
  s.append("R5\t= ").append(IntToHex(r->R5,2)).append("\n");
  s.append("RAB\t= ").append(IntToHex(r->RAB,1)).append("\n\n");
  s.append("ST\t= ").append(IntToHex(r->ST[0],3)).append(" ").append(IntToHex(r->ST[1],3)).append(" ").append(IntToHex(r->ST[2],3)).append("\n\n");
  for (int i=0; i<8;i++)
      s.append(QString("X%1 = %2   Y%3 = %4\n").arg(i).arg(Reg(r->RX[i])).arg(i).arg(Reg(r->RY[i])));
  //if Application.MessageBox(PChar(s),PChar(Decode),MB_OKCancel)<>IDOK then
//Debugger=False;
}

  QString Cdebug_ti57cpu::Tracing(TI57regs *r) {
      QString  s,ss;
      s=Decode(r).append(" ");

      ss = QString("%1").arg(pPC->Get_16(r->PC<<1),4,16,QChar('0'));

//      s.append(QString("  A=%1 B=%2 C=%3 D=%4 ").arg(Reg(r->RA)).arg(Reg(r->RB)).arg(Reg(r->RC)).arg(Reg(r->RD)));
//      s.append(QString("  COND=%1 BASE=%2 R5=%3 RAB=%4 ST=%5 %6 %7 ").
//              arg(r->COND).arg(r->BASE).arg(IntToHex(r->R5,2)).arg(IntToHex(r->RAB,1)).arg(IntToHex(r->ST[0],3)).
//              arg(IntToHex(r->ST[1],3)).arg(IntToHex(r->ST[2],3)));
      //  Write(Tracer,s)

      return s;
  }


UINT32 Cdebug_ti57cpu::DisAsm_1(UINT32 oldpc)
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

    TI57regs r;
    r.PC = pc;
    r.OP = pPC->Get_16(r.PC<<1);

//    sprintf(Buffer," %06X:%02X",pc,op);
    QString t = Tracing(&r);

//    qWarning()<<"**"<<t<<"**";

    sprintf(Buffer,"%s",t.toLatin1().data());
    DasmAdr = oldpc;
    NextDasmAdr = oldpc+len;
    debugged = true;
    return NextDasmAdr;
}

UINT32 Cdebug_hpnut::DisAsm_1(UINT32 oldpc)
{
    //return (oldpc);
    Buffer[0] = '\0';
}
