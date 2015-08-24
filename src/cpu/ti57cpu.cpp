/*
 * this code is based on TI-57E emulator
 * http://www.hrastprogrammer.com/ti57e/
 */



#include <QDebug>
#include <QXmlStreamReader>

#include "ti57cpu.h"
#include "ti57cpu_dasm.h"
#include "pcxxxx.h"
#include "Lcdc.h"
#include "ui/cregsz80widget.h"


QString Cti57cpu::IntToHex(int val,int nb) {
    return QString("%1").arg(val,nb,16,QChar('0'));
}

void Cti57cpu::Error(QString msg) {
    qWarning()<< msg <<" :OP="<<IntToHex(r->OP,4)<<" PC="<<IntToHex(r->PC-1,3);
  r->Run=false;
}

Cti57cpu::Cti57cpu(CPObject *parent) : CCPU(parent)
{
    fn_log="ti57cpu.log";
    r = new TI57regs;
    pDEBUG = new Cdebug_ti57cpu(this);
    regwidget = (CregCPU*) new Cregsz80Widget(0,this);
}

Cti57cpu::~Cti57cpu()
{

}

bool Cti57cpu::init() {
    Check_Log();

    Reset();

    return true;
}

bool Cti57cpu::exit()
{

    return true;
}

void Cti57cpu::step()
{
    if (!halt)
        Emulate();
}

void Cti57cpu::Reset()
{
    bool WarmStart = false;
    r->BASE=10; //Warning: What mode is active at power on: BCD or HEX?}
    r->COND=0;
    r->KEYR=0;
    r->MF=0;
    r->OP=0;
    r->PC=0;
    r->R5=0;
    r->RAB=0;
    r->SIGMA=false;
    r->ST[0]=0; r->ST[1]=0; r->ST[2]=0;
    r->Power= true;
    halt = false;

    memset(&(r->RA),0,sizeof(r->RA));
    memset(&(r->RB),0,sizeof(r->RB));
    memset(&(r->RC),0,sizeof(r->RC));
    memset(&(r->RD),0,sizeof(r->RD));

    if (!WarmStart) {
        memset(&(r->RX),0,sizeof(r->RX));
        memset(&(r->RY),0,sizeof(r->RY));
    }
}

void Cti57cpu::Load_Internal(QXmlStreamReader *xmlIn)
{
    qWarning()<<"Load internal ti57cpu";
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "ti57")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &r[0],ba_reg.data(),sizeof(TI57regs));
            qWarning()<<"regs read ti57cpu";
        }
        xmlIn->skipCurrentElement();
    }
}

void Cti57cpu::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","ti57");
        QByteArray ba_reg((char*)&r[0],sizeof(TI57regs));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
    xmlOut->writeEndElement();
}

void Cti57cpu::Regs_Info(UINT8)
{
    QString s;
    sprintf(Regs_String,"");
    s.append(QString("  A=%1 B=%2 C=%3 D=%4 ").arg(Cdebug_ti57cpu::Reg(r->RA)).arg(Cdebug_ti57cpu::Reg(r->RB)).arg(Cdebug_ti57cpu::Reg(r->RC)).arg(Cdebug_ti57cpu::Reg(r->RD)));
    s.append(QString("  COND=%1 BASE=%2 R5=%3 RAB=%4 ST=%5 %6 %7 ").
            arg(r->COND).arg(r->BASE).arg(IntToHex(r->R5,2)).arg(IntToHex(r->RAB,1)).arg(IntToHex(r->ST[0],3)).
            arg(IntToHex(r->ST[1],3)).arg(IntToHex(r->ST[2],3)));
    sprintf(Regs_String,"%s",s.toLatin1().data());
}

void Cti57cpu::BranchOP() {
    if (r->COND==( (r->OP & 0x0400)>>10)) {
        r->PC=(r->PC & 0x0400) | (r->OP & 0x03FF);
        if (fp_log) {
            sprintf(Log_String,"%s Branchcondition =%04x ",Log_String,r->PC);
        }
    }
    if (fp_log) {
        sprintf(Log_String,"%s reset cond",Log_String);
    }
    r->COND=0;
}

void Cti57cpu::CallOP() {
    r->ST[2] = r->ST[1];
    r->ST[1] = r->ST[0];
    r->ST[0] = r->PC;
    r->COND = 0;
    r->PC = r->OP & 0x07FF;
}

void Cti57cpu::FlagOP() {
    UINT8 F,D,J,B;
    UINT8 *JR;

    J=(r->OP & 0xC0) >> 6;
    D=(r->OP & 0x30) >> 4;
    B=(r->OP & 0x0C) >> 2;
    F=r->OP & 0x03;
    JR=0;
    switch(J) {
    case 0: JR=&(r->RA)[0]; break;
    case 1: JR=&(r->RB)[0]; break;
    case 2: JR=&(r->RC)[0]; break;
    case 3: JR=&(r->RD)[0]; break;
    }
    B =(1<<B);
    if (D==0)
        Error("Flag Operation with D=0");
    else D+=12;

    switch(F) {
    case 0: /*SF*/ JR[D] |= B; break;
    case 1: /*ZF*/ JR[D] &= ~B; break;
    case 2: /*?TF*/ if (JR[D] & B) r->COND=1; break;
    case 3: /*XF*/ JR[D] ^= B; break;
    }
    //Warning: Do Flag Operations update R5?
}

void Cti57cpu::MaskOP() {
  UINT8 CY,F0,F1,J,K,L,LN,N;
  UINT8 *JR,*KR;
  TRegister R,T;

  F0=0; F1=0; JR=0; KR=0;
  UINT8 begin[0x10] = {12, 0, 2, 0, 2, 0,99, 0,14,13,14,99,99,13,99,15};
  UINT8 end[0x10]   = {12,15,12,12, 2, 1,99,13,14,15,15,99,99,13,99,15};
  F0 = begin[r->MF];
  F1 = end[r->MF];
  if ((F0==99) || (F1==99)) {
  }


  J=(r->OP & 0xC0) >> 6;

  LN=r->OP & 0x07;
  L=LN >> 1;
  N=LN & 0x01;
  switch(J) {
  case 0: JR = &(r->RA)[0]; break;
  case 1: JR = &(r->RB)[0]; break;
  case 2: JR = &(r->RC)[0]; break;
  case 3: JR = &(r->RD)[0]; break;
  }

  memset(&R,0,sizeof(R));
  memset(&T,0,sizeof(T));

  K=(r->OP & 0x38) >> 3;
  switch (K) {
    case 0: /*ADD/SUB A*/ KR = &(r->RA)[0]; break;
    case 1: /*ADD/SUB B*/ KR = &(r->RB)[0]; break;
    case 2: /*ADD/SUB C*/ KR = &(r->RC)[0]; break;
    case 3: /*ADD/SUB D*/ KR = &(r->RD)[0]; break;
    case 4: /*INC/DEC*/ T[F0]=1; break;
    case 5: /*SHL/SHR*/; break;
    case 6: /*ADD/SUB R5L*/ T[F0] = (r->R5 & 0x0F);  break;;
    case 7: /*ADD/SUB R5*/  T[F0] = (r->R5 & 0x0F);
                            if ((F0<15) && (F0<F1)) T[F0+1] = (r->R5 >> 4);
                            break;
  }

  if (KR)
      for (int i=F0; i<=F1;i++)
          T[i]=KR[i];

  if (L==3) {
      if (N==0) { //EXCHANGE}
          if ((J==0) && (K<4))
              for (int i=F0;i<=F1;i++) {
                  UINT8 D=r->RA[i];
                  r->RA[i]=KR[i];
                  KR[i]=D;
              }
          else Error("Invalid Exchange Register");
      }
    else
      { //{STORE}
      if (K!=5)
          for (int i=F0; i<=F1;i++)
              JR[i]=T[i];
      else {
          //Error('Invalid Store Register')
      }
    }
    r->R5 = T[F0];
    if ((F0<15) && (F0<F1))
        r->R5 |= (T[F0+1] << 4); // {Warning: What if F0=15?}
  }
  else  {
    for (int i=F0;i<=F1;i++) R[i]=JR[i];
    if (K!=5) {
        UINT8 B = r->BASE;
        CY=0;
        if (r->MF >= 8) B=16;
        if (N==0)
            for (int i=F0;i<=F1;i++) {
                R[i] += T[i]+CY;
                if (R[i] >= B) { R[i]-=B; CY=1; }
                else CY=0;
            }
        else {
            for (int i=F0;i<=F1;i++)
            {
                R[i] = R[i]-T[i]-CY;
                if (R[i]>=B) {R[i]+=B; CY=1;}
                else CY=0;
            }
        }
        if (CY) r->COND=CY;
    }
    else {
      if (N==0) {
        for (int i=F1 ; i>= F0;i--)
            if (i==F0) R[i]=0;
            else R[i]=R[i-1];
      }
      else
        for (int i=F0;i<= F1;i++) {
            if (i==F1) R[i]=0;
            else R[i]=R[i+1];
        }
  }
    r->R5 = R[F0];
    if ((F0<15) && (F0<F1)) r->R5|= (R[F0+1] << 4);// {Warning: What if F0=15?}
    switch (L) {
      case 0: for (int i=F0;i<=F1;i++) JR[i]=R[i]; break;
      case 1: if (K <4)
            for (int i=F0;i<=F1;i++)
                KR[i]=R[i];
        else Error("Invalid Arithmetic Result Register");
        break;
      case 2: if (K <5) { /*Nothing*/ }
         else if (K==5)
            for (int i=F0;i<=F1;i++) JR[i]=R[i];
        else Error("Invalid Suppress Operation");
    }
  }
}

void Cti57cpu::MiscOP() {

    UINT8 F1,P,Q;

    F1 = r->SIGMA ? 12 : 15;
    P = r->OP & 0x0F;
    Q =(r->OP & 0xF0) >> 4;
    switch(P) {

    case 0x00: /*STYA*/
        for (int i=0;i<16;i++)
            r->RA[i] = r->RY[r->RAB][i];
        break;

    case 0x01: /*RAB*/
        r->RAB=Q & 0x07;
        break;

    case 0x02: /*BR R5*/
        r->SIGMA = (r->R5==0x39);
        r->PC = r->R5;
        break; /*Warning: Does BR R5 clear COND?*/

    case 0x03: /*RTN*/
        r->COND=0;
        r->PC=r->ST[0];
        r->ST[0] = r->ST[1];
        r->ST[1] = r->ST[2];
        r->ST[2] = 0;
        break;

    case 0x04: /*STAX*/
        for (int i=0;i<16;i++)
            r->RX[r->RAB][i]=r->RA[i];
        break;

    case 0x05: /*STXA*/
        for (int i=0;i<16;i++)
            r->RA[i]=r->RX[r->RAB][i];
        break;

    case 0x06: /*STAY*/
        for (int i=0;i<16;i++)
            r->RY[r->RAB][i]=r->RA[i];
        break;

    case 0x07: /*DISP*/
    {
        bool overflow = true;
        for (int i=0; i < 12;i++) {
            if (r->RB[i] != 9) overflow=false;
        }
        if (!overflow || !r->EMPTY) {
            r->EMPTY = overflow;
            pPC->pLCDC->updated = true;
        }
    }
        break;

  case 0x08: /*BCDS*/
        r->BASE=10;
        break;

  case 0x09: /*BCDR*/
        r->BASE=16;
        break;

  case 0x0A: /*RAB R5*/
        r->RAB=r->R5 & 0x07;
        break;

  case 0x0B: /*POWOFF*/
        r->Power = false;
        halt = true;
        break;

  case 0x0C: /*STAX MAEX*/
        for (int i=0 ; i<= F1;i++)
            r->RX[r->RAB][i]=r->RA[i];
        break;

  case 0x0D: /*?KEY*/
        if (r->KEYR) {
            r->COND = 1;
            r->R5 = r->KEYR;
            r->Timer = 0;
        }
        break;

    case 0x0E: /*STAY MAEX*/
        for (int i=0; i<= F1;i++)
            r->RY[r->RAB][i] = r->RA[i];
        break;

    default:
        if (r->OP != 0x0EFF)
            Error("Invalid Misc Instruction");
        break;
  }

}



void Cti57cpu::Emulate() {
  if (r->PC > MAXROM) {
      Error("Out of ROM");
      return;
  }
  r->OP = pPC->Get_16(r->PC<<1);
  r->PC++;
  if (r->OP & 0x1000) {
      if (r->OP & 0x0800) BranchOP();
      else CallOP();
  }
  else
  {
    r->MF = (r->OP >> 8);
    switch (r->MF) {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x07:
    case 0x08:
    case 0x09:
    case 0x0A:
    case 0x0D:
    case 0x0F: MaskOP(); break;
    case 0x0C: FlagOP(); break;
    case 0x0E: MiscOP(); break;
    default: Error("Invalid Instruction");
    }
  }
}

void Cti57cpu::Execute() {

}




