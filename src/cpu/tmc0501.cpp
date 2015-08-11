/*
 * this code is based on TI-5x emulator
 * (c) 2014 Hynek Sladky
 * http://hsl.wz.cz/ti_59.htm
 */



#include <QDebug>
#include <QXmlStreamReader>

#include "tmc0501.h"
#include "tmc0501_dasm.h"
#include "pcxxxx.h"
#include "Lcdc.h"
#include "ui/cregsz80widget.h"
#include "Inter.h"

#include "ti/ti59.h"

#define	MODE_PRINTER	0x0002
#define	MODE_CARD	0x0004
static unsigned mode_flags = MODE_PRINTER;

unsigned char CONSTANT[64][16] = {
  // constants
  {0xc,0x0,0x0,0x4,0x9,0x9,0x2,0x9,0x0,0x5,0x8,0x5,0x2,0x0,0x3,0x2},
  {0x5,0x4,0x9,0x9,0x5,0x5,0x0,0x8,0x1,0x7,0x4,0x1,0x3,0x9,0x6,0x0},
  {0x5,0x2,0x3,0x4,0x0,0x8,0x9,0x7,0x1,0x0,0x1,0x3,0x5,0x9,0x0,0x0},
  {0x8,0x6,0x1,0x3,0x5,0x8,0x0,0x3,0x3,0x0,0x5,0x9,0x9,0x0,0x0,0x0},
  {0x4,0x8,0x0,0x3,0x3,0x3,0x0,0x0,0x5,0x9,0x9,0x9,0x0,0x0,0x0,0x0},
  {0x3,0x3,0x3,0x0,0x0,0x0,0x5,0x9,0x9,0x9,0x9,0x0,0x0,0x0,0x0,0x0},
  {0x0,0x0,0x0,0x0,0x5,0x9,0x9,0x9,0x9,0x9,0x0,0x0,0x0,0x0,0x0,0x0},
  {0x0,0x0,0x5,0x9,0x9,0x9,0x9,0x9,0x9,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
  {0x0,0x5,0x4,0x7,0x9,0x3,0x3,0x6,0x1,0x8,0x9,0x3,0x5,0x8,0x7,0x0},
  {0x0,0x0,0x2,0x1,0x9,0x4,0x2,0x5,0x6,0x8,0x6,0x6,0x9,0x9,0x0,0x0},
  {0x0,0x7,0x6,0x6,0x8,0x6,0x6,0x6,0x6,0x9,0x9,0x9,0x9,0x0,0x0,0x0},
  {0x7,0x6,0x6,0x6,0x6,0x6,0x9,0x9,0x9,0x9,0x9,0x9,0x0,0x0,0x0,0x0},
  {0x7,0x6,0x6,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x9,0x0,0x0,0x0,0x0,0x0},
  {0xc,0x1,0x0,0x5,0x9,0x7,0x6,0x2,0x3,0x6,0x9,0x7,0x0,0x7,0x5,0x1},
  {0xc,0x0,0x0,0x0,0x9,0x5,0x3,0x5,0x6,0x2,0x9,0x5,0x1,0x4,0x1,0x3}, // pi
  {0xc,0x1,0x0,0x8,0x0,0x3,0x1,0x5,0x9,0x7,0x7,0x5,0x9,0x2,0x7,0x5},
  // program codes
  {0x2,0x8,0x8,0x0,0x3,0x5,0x3,0x5,0x3,0x4,0x6,0x0,0x5,0x7,0x3,0x4},
  {0x4,0x0,0x5,0x6,0x3,0x4,0x1,0x0,0x5,0x5,0x3,0x4,0x3,0x0,0x4,0x5},
  {0x5,0x5,0x3,0x5,0x3,0x4,0x5,0x0,0x5,0x7,0x3,0x4,0x4,0x0,0x3,0x3},
  {0x5,0x5,0x3,0x4,0x3,0x0,0x4,0x5,0x4,0x5,0x3,0x5,0x3,0x5,0x4,0x2},
  {0x5,0x6,0x2,0x3,0x3,0x4,0x4,0x0,0x4,0x9,0x5,0x8,0x3,0x4,0x1,0x0},
  {0x4,0x5,0x5,0x5,0x3,0x4,0x3,0x0,0x4,0x5,0x2,0x8,0x0,0x2,0x3,0x5},
  {0x3,0x5,0x4,0x9,0x5,0x8,0x2,0x8,0x8,0x1,0x4,0x5,0x5,0x5,0x2,0x3},
  {0x4,0x5,0x2,0x9,0x3,0x5,0x2,0x3,0x5,0x6,0x2,0x8,0x8,0x1,0x5,0x8},
  {0x2,0x3,0x4,0x5,0x2,0x9,0x3,0x5,0x3,0x4,0x4,0x0,0x5,0x5,0x3,0x4},
  {0x3,0x0,0x4,0x5,0x2,0x3,0x3,0x5,0x3,0x4,0x1,0x0,0x5,0x5,0x3,0x4},
  {0x3,0x0,0x4,0x5,0x2,0x8,0x0,0x2,0x3,0x3,0x3,0x5,0x4,0x9,0x5,0x8},
  {0x3,0x4,0x2,0x0,0x5,0x5,0x3,0x4,0x3,0x0,0x4,0x5,0x2,0x3,0x3,0x3},
  {0x3,0x5,0x4,0x9,0x5,0x8,0x3,0x4,0x5,0x0,0x5,0x5,0x3,0x4,0x3,0x0},
  {0x4,0x5,0x2,0x3,0x2,0x9,0x3,0x5,0x3,0x5,0x3,0x4,0x5,0x0,0x5,0x7},
  {0x3,0x4,0x4,0x0,0x3,0x3,0x5,0x5,0x3,0x4,0x3,0x0,0x4,0x5,0x5,0x5},
  {0x3,0x5,0x3,0x4,0x3,0x0,0x5,0x7,0x1,0x0,0x4,0x5,0x2,0x8,0x8,0x0},
  {0x4,0x5,0x4,0x3,0x2,0x3,0x3,0x5,0x3,0x5,0x3,0x4,0x2,0x0,0x5,0x7},
  {0x3,0x4,0x1,0x0,0x3,0x3,0x5,0x5,0x3,0x4,0x3,0x0,0x4,0x5,0x5,0x5},
  {0x2,0x8,0x8,0x1,0x4,0x5,0x4,0x3,0x2,0x9,0x3,0x5,0x3,0x5,0x3,0x4},
  {0x6,0x0,0x5,0x7,0x3,0x4,0x4,0x0,0x5,0x6,0x3,0x4,0x1,0x0,0x5,0x5},
  {0x3,0x4,0x3,0x0,0x4,0x5,0x5,0x5,0x3,0x5,0x3,0x5,0x3,0x4,0x5,0x0},
  {0x5,0x7,0x3,0x4,0x4,0x0,0x3,0x3,0x5,0x5,0x3,0x4,0x3,0x0,0x4,0x5},
  {0x5,0x6,0x3,0x5,0x3,0x4,0x2,0x0,0x5,0x7,0x3,0x4,0x1,0x0,0x3,0x3},
  {0x5,0x5,0x3,0x4,0x3,0x0,0x4,0x5,0x4,0x5,0x4,0x3,0x4,0x5,0x2,0x9},
  {0x4,0x4,0x1,0x0,0x2,0x8,0x8,0x0,0x3,0x3,0x4,0x4,0x2,0x0,0x2,0x3},
  {0x4,0x4,0x4,0x0,0x2,0x8,0x8,0x4,0x2,0x8,0x7,0x0,0x3,0x3,0x4,0x4},
  {0x5,0x0,0x1,0x0,0x1,0x6,0x2,0x0,0x6,0x3,0x4,0x9,0x4,0x4,0x1,0x0},
  {0x2,0x8,0x8,0x0,0x3,0x3,0x4,0x9,0x4,0x4,0x2,0x0,0x2,0x3,0x2,0x2},
  {0x4,0x4,0x4,0x0,0x2,0x8,0x8,0x4,0x2,0x8,0x7,0x0,0x3,0x3,0x4,0x9},
  {0x4,0x4,0x5,0x0,0x1,0x0,0x4,0x9,0x4,0x4,0x3,0x0,0x2,0x8,0x7,0x3},
  {0x2,0x8,0x8,0x1,0x4,0x4,0x6,0x0,0x2,0x8,0x7,0x1,0x2,0x3,0x3,0x4},
  {0x3,0x0,0x2,0x9,0x3,0x5,0x2,0x8,0x8,0x0,0x3,0x3,0x5,0x8,0x2,0x3},
  {0x2,0x8,0x7,0x0,0x3,0x3,0x4,0x5,0x4,0x3,0x2,0x3,0x3,0x5,0x3,0x5},
  {0x2,0x8,0x7,0x1,0x5,0x5,0x0,0x5,0x4,0x5,0x2,0x2,0x9,0x3,0x5,0x8},
  {0x3,0x5,0x2,0x8,0x8,0x1,0x5,0x5,0x2,0x8,0x7,0x1,0x4,0x5,0x2,0x2},
  {0x0,0x3,0x4,0x5,0x4,0x2,0x2,0x9,0x3,0x5,0x2,0x8,0x8,0x0,0x9,0x3},
  {0x5,0x6,0x2,0x3,0x2,0x8,0x7,0x0,0x4,0x5,0x2,0x3,0x3,0x5,0x2,0x8},
  {0x8,0x1,0x8,0x3,0x5,0x6,0x2,0x8,0x7,0x1,0x4,0x5,0x2,0x9,0x3,0x5},
  {0x3,0x5,0x3,0x5,0x2,0x8,0x8,0x0,0x9,0x5,0x5,0x6,0x6,0x0,0x0,0x0},
  {0x5,0x8,0x3,0x5,0x2,0x8,0x8,0x1,0x2,0x2,0x9,0x5,0x5,0x6,0x1,0x0},
  {0x0,0x0,0x0,0x0,0x4,0x5,0x2,0x8,0x8,0x0,0x9,0x5,0x4,0x5,0x5,0x6},
  {0x3,0x9,0x6,0x0,0x5,0x8,0x2,0x8,0x8,0x1,0x2,0x2,0x9,0x5,0x4,0x5},
  {0x5,0x5,0x3,0x0,0x6,0x0,0x4,0x5,0x2,0x9,0x3,0x5,0x3,0x5,0x2,0x8},
  {0x8,0x0,0x9,0x5,0x5,0x8,0x2,0x8,0x8,0x1,0x2,0x2,0x9,0x5,0x5,0x6},
  {0x3,0x9,0x6,0x0,0x4,0x5,0x5,0x6,0x1,0x0,0x0,0x0,0x0,0x0,0x4,0x5},
  {0x2,0x8,0x8,0x0,0x3,0x5,0x3,0x5,0x9,0x5,0x5,0x8,0x2,0x8,0x8,0x1},
  {0x2,0x2,0x9,0x5,0x5,0x6,0x3,0x9,0x6,0x0,0x4,0x5,0x5,0x6,0x3,0x9},
  {0x0,0x0,0x1,0x0,0x4,0x5,0x2,0x9,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}
};

static const unsigned char PRN_CODE[64] = {
  ' ','0','1','2','3','4','5','6',
  '7','8','9','A','B','C','D','E',
  '-','F','G','H','I','J','K','L',
  'M','N','O','P','Q','R','S','T',
  '.','U','V','W','X','Y','Z','+',
  'x','*','s','p','e','(',')',',',
  '^','%','_','/','=','\'',0x9E,'&',
  'z','?',':','!',']',0x7F,'[','S'
};

static const struct {
  unsigned char code;
  char str[4];
} PRN_STR[] = {
  {0x00, "   "},
  {0x11, " = "},
  {0x12, " - "},
  {0x13, " + "},
  {0x16, " / "},
  {0x17, " x "},
  {0x1A, "xsY"}, //x_sqrt_Y
  {0x1B, "Y^x"}, //Yx_
  {0x21, "CLR"},
  {0x22, "INV"},
  {0x23, "DPT"},
  {0x26, "CE "},
  {0x27, "+/-"},
  {0x2D, "EE "},
  {0x31, "e^x"}, // ex_
  {0x33, "x^2"}, // x2_
  {0x36, "1/x"},
  {0x3C, "sX "}, // sqrt_X_
  {0x3D, "X_Y"}, // X exchange Y ??
  {0x51, "LNX"},
  {0x53, "PRM"},
  {0x54, " % "},
  {0x56, "COS"},
  {0x57, "SIN"},
  {0x5D, "TAN"},
  {0x61, "SUM"},
  {0x66, "STO"},
  {0x67, "pi "}, //_pi_
  {0x68, "RCL"},
  {0x69, "S+ "},
  {0x70, "ERR"},
  {0x71, " { "},
  {0x72, " ) "},
  {0x73, "LRN"},
  {0x74, "RUN"},
  {0x76, "HLT"},
  {0x78, "STP"},
  {0x7A, "GTO"},
  {0x7C, "IF "},
  {0, {0}}
};

QString Ctmc0501::IntToHex(int val,int nb) {
    return QString("%1").arg(val,nb,16,QChar('0'));
}

void Ctmc0501::Error(QString ) {
//    qWarning()<< msg <<" :OP="<<IntToHex(r.OP,4)<<" PC="<<IntToHex(r.PC-1,3);
//  r.Run=false;
}

Ctmc0501::Ctmc0501(CPObject *parent, Models mod) : CCPU(parent)
{
    currentModel = mod;
    fn_log="tmc0501";
//    r = new TMC0501regs;
    pDEBUG = new Cdebug_tmc0501(this);
    regwidget = (CregCPU*) new Cregsz80Widget(0,this);

    strcpy(card_output,"card.bin");
}

Ctmc0501::~Ctmc0501()
{

}

bool Ctmc0501::init() {
    Check_Log();

    Reset();

    return true;
}

bool Ctmc0501::exit()
{

    return true;
}

void Ctmc0501::step()
{
    if (!halt)
        Emulate();
}

void Ctmc0501::Reset()
{
    memset (&r, 0, sizeof (TMC0501regs));
    r.flags |= FLG_COND | FLG_DISP;

}

void Ctmc0501::Load_Internal(QXmlStreamReader *xmlIn)
{
    if ((currentModel==TI58C)||(currentModel==TI59C)) {
        qWarning()<<"Load internal tmc0501";
        if (xmlIn->readNextStartElement()) {
            if ( (xmlIn->name()=="cpu") &&
                 (xmlIn->attributes().value("model").toString() == "tmc0501")) {
                QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
                memcpy((char *) &r,ba_reg.data(),sizeof(TMC0501regs));
                qWarning()<<"regs read ti57cpu";
            }
            xmlIn->skipCurrentElement();
        }
    }
}

void Ctmc0501::save_internal(QXmlStreamWriter *xmlOut)
{
    if ((currentModel==TI58C)||(currentModel==TI59C)) {
        xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","tmc0501");
        QByteArray ba_reg((char*)&r,sizeof(TMC0501regs));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
        xmlOut->writeEndElement();
    }
}

void Ctmc0501::Regs_Info(UINT8)
{
    QString s;
//    sprintf(Regs_String,"");
//    s.append(QString("  A=%1 B=%2 C=%3 D=%4 ").arg(Cdebug_tmc0501::Reg(r.RA)).arg(Cdebug_tmc0501::Reg(r.RB)).arg(Cdebug_tmc0501::Reg(r.RC)).arg(Cdebug_tmc0501::Reg(r.RD)));
//    s.append(QString("  COND=%1 BASE=%2 R5=%3 RAB=%4 ST=%5 %6 %7 ").
//            arg(r.COND).arg(r.BASE).arg(IntToHex(r.R5,2)).arg(IntToHex(r.RAB,1)).arg(IntToHex(r.ST[0],3)).
//            arg(IntToHex(r.ST[1],3)).arg(IntToHex(r.ST[2],3)));
    sprintf(Regs_String,"%s",s.toLatin1().data());
}



void Ctmc0501::Emulate() {
    WORD _op = pPC->Get_16r(r.addr<<1);
    execute (_op);

}

// ====================================
// ALU functions
// ====================================
// ALU block
// ------------------------------------
enum {ALU_ADD, ALU_SHL, ALU_SUB, ALU_SHR};
#define	ALU_SHIFT	ALU_SHL
// ------------------------------------
void Ctmc0501::Alu (unsigned char *dst, unsigned char *srcX, unsigned char *srcY, const mask_type *mask, unsigned char flags) {
unsigned char carry = 0;
unsigned char shl = 0;
int i;
//  if (log_flags & LOG_DEBUG) {
//    if (srcX) {LOG ("["); for (i = 15; i >= 0; i--) LOG ("%X", srcX[i]); LOG ("]");}
//    if (srcY) {LOG ("["); for (i = 15; i >= 0; i--) LOG ("%X", srcY[i]); LOG ("]");}
//  }
  for (i = 0; i <= 15; i++) {
    unsigned char sum = 0, shr = 0;
    if (i == mask->start)
      shl = carry = 0;
    if (srcY)
      sum = srcY[i];
    if (i == mask->cpos)
      sum |= mask->cval;
    shr = sum;
    sum += carry;
    if (flags >= ALU_SUB)
      sum = -sum;
    if (srcX) {
      sum += srcX[i];
      shr |= srcX[i];
    }
    r.Sout[i] = (sum & 0x0F);
    if (!i) {
      if ((carry = (sum >= 0x10)))
        sum &= 0x0F;
    } else {
      if ((carry = (sum >= 10))) {
    if (flags < ALU_SUB)
      sum -= 10;
    else
      sum += 10;
      }
    }
    // write result to destination
    if (i >= mask->start && i <= mask->end) {
      if (i == mask->start)
    r.R5 = sum;
      if (dst) {
    if (flags == ALU_SHL)
      dst[i] = shl;
    else
    if (flags == ALU_SHR) {
      if (i > mask->start)
        dst[i-1] = shr;
      if (i == mask->end)
        dst[i] = 0;
    } else
      dst[i] = sum;
    shl = sum;
      }
      if (i == mask->end && !(flags & ALU_SHIFT) && carry)
    r.flags &= ~FLG_COND;
    }
  }
}

// ====================================
// Exchange value
// ------------------------------------
void Ctmc0501::Xch (unsigned char *src1, unsigned char *src2, const mask_type *mask) {
int i;
  for (i = mask->start; i <= mask->end; i++) {
    unsigned char tmp;
    tmp = src1[i];
    src1[i] = src2[i];
    src2[i] = tmp;
  }
}

// ====================================
// main CPU function
// executes instructions
// ------------------------------------
int Ctmc0501::execute (unsigned short opcode) {
  // process PREG address change
  if (r.PREG & 0x1) {
    // PREG
    r.addr = r.PREG >> 3;
    r.PREG = 0;
    return 0;
  }
  // update digit counter
  if (r.digit)
    r.digit--;
  else
    r.digit = 15;

  // update instruction cycle counter
  if (r.flags & FLG_IDLE) {
    r.cycle += 4;
    pPC->pTIMER->state+=4;
  }
  else {
    r.cycle++;
    pPC->pTIMER->state++;
  }
  // clear HOLD bit
  r.flags &= ~FLG_HOLD;
  // process PREG bit
  if (r.KR & 0x2) {
    r.PREG = (r.KR >> 1) | (r.KR << 15);
    r.KR &= ~0x2;
  }
  // init EXT
  if (r.flags & FLG_EXT_VALID)
    r.flags &= ~FLG_EXT_VALID;
  else
    r.EXT = 0;

  // init IO (ALU out)
  if (r.flags & FLG_IO_VALID)
    r.flags &= ~FLG_IO_VALID;
  else
    memset (r.Sout, 0, sizeof (r.Sout));

  // process opcode
  if (opcode & 0x1000) {
    // ================================
    // jump
    // ================================
    r.flags |= FLG_JUMP;
    if (!((r.flags ^ opcode) & FLG_COND)) {
      // COND is same as bit in opcode
      unsigned short offs = (opcode >> 1) & 0x3FF;
      if (opcode & 0x0001)
    r.addr -= offs;
      else
        r.addr += offs;
    } else
      r.addr++;
    return 2;
  }
  if (r.flags & FLG_JUMP) {
    // COND is set again after last jump in series
    r.flags &= ~FLG_JUMP;
    r.flags |= FLG_COND;
  }
  switch (opcode & 0x0F00) {
    // ================================
    // flag operations
    // ================================
    case 0x0000:
      {
    unsigned bit = (opcode >> 4) & 0x000F;
    unsigned mask = 1 << bit;
    switch (opcode & 0x000F) {
      case 0x0000:
        // TEST FLAG A
        if (r.fA & mask)
          r.flags &= ~FLG_COND;
//        if (log_flags & LOG_DEBUG)
//          LOG ("FA=%04X ", r.fA);
//        if (log_flags & LOG_SHORT)
//          LOG ("COND=%u", (r.flags & FLG_COND) != 0);
        break;
      case 0x0001:
        // SET FLAG A
        r.fA |= mask;
//        if (log_flags & LOG_SHORT)
//          LOG ("FA=%04X", r.fA);
        break;
      case 0x0002:
        // ZERO FLAG A
        r.fA &= ~mask;
//        if (log_flags & LOG_SHORT)
//          LOG ("FA=%04X", r.fA);
        break;
      case 0x0003:
        // INVERT FLAG A
        r.fA ^= mask;
//        if (log_flags & LOG_SHORT)
//          LOG ("FA=%04X", r.fA);
        break;
      case 0x0004:
        // EXCH. FLAG A B
        if ((r.fA ^ r.fB) & mask) {
          r.fA ^= mask;
          r.fB ^= mask;
        }
//        if (log_flags & LOG_SHORT)
//          LOG ("FA=%04X FB=%04X", r.fA, r.fB);
        break;
      case 0x0005:
        // SET FLAG KR
        r.KR |= mask;
//        if (log_flags & LOG_SHORT)
//          LOG ("KR=%04X", r.KR);
        break;
      case 0x0006:
        // COPY FLAG B->A
        if ((r.fA ^ r.fB) & mask)
          r.fA ^= mask;
//        if (log_flags & LOG_SHORT)
//          LOG ("FA=%04X", r.fA);
        break;
      case 0x0007:
        // REG 5->FLAG A S0 S3
        r.fA = (r.fA & ~0x001E) | ((r.R5 & 0x000F) << 1);
//        if (log_flags & LOG_SHORT)
//          LOG ("FA=%04X", r.fA);
        break;
      case 0x0008:
        // TEST FLAG B
        if (r.fB & mask)
          r.flags &= ~FLG_COND;
//        if (log_flags & LOG_DEBUG)
//          LOG ("FB=%04X ", r.fB);
//        if (log_flags & LOG_SHORT)
//          LOG ("COND=%u", (r.flags & FLG_COND) != 0);
        break;
      case 0x0009:
        // SET FLAG B
        r.fB |= mask;
//        if (log_flags & LOG_SHORT)
//          LOG ("FB=%04X", r.fB);
        break;
      case 0x000A:
        // ZERO FLAG B
        r.fB &= ~mask;
//        if (log_flags & LOG_SHORT)
//          LOG ("FB=%04X", r.fB);
        break;
      case 0x000B:
        // INVERT FLAG B
        r.fB ^= mask;
//        if (log_flags & LOG_SHORT)
//          LOG ("FB=%04X", r.fB);
        break;
      case 0x000C:
        // COMPARE FLAG A B
        if (!((r.fA ^ r.fB) & mask))
          r.flags &= ~FLG_COND;
//        if (log_flags & LOG_DEBUG)
//          LOG ("FA=%04X FB=%04X ", r.fA, r.fB);
//        if (log_flags & LOG_SHORT)
//          LOG ("COND=%u", (r.flags & FLG_COND) != 0);
        break;
      case 0x000D:
        // ZERO FLAG KR
        r.KR &= ~mask;
//        if (log_flags & LOG_SHORT)
//          LOG ("KR=%04X", r.KR);
        break;
      case 0x000E:
        // COPY FLAG A->B
        if ((r.fA ^ r.fB) & mask)
          r.fB ^= mask;
//        if (log_flags & LOG_SHORT)
//          LOG ("FB=%04X", r.fB);
        break;
      case 0x000F:
        // REG 5->FLAG B S0 S3
        r.fB = (r.fB & ~0x001E) | ((r.R5 & 0x000F) << 1);
//        if (log_flags & LOG_SHORT)
//          LOG ("FB=%04X", r.fB);
        break;
    }
      }
      break;
    // ================================
    // keyboard operations
    // ================================
    case 0x0800:
  {
      unsigned char mask;
      // get pressed key(s) mask
      mask = (((opcode & 0x07) | ((opcode >> 1) & 0x78)) ^ 0x7F) & r.key[r.digit];
      // check if more than 1 key is pressed
      if (mask & (mask - 1))
          mask = 0;
      if (!(opcode & 0x0008)) {
          // scan all keyboard
          // scan current row
          if (r.key[r.digit] & mask) {
              unsigned char bit = 0;
              //        if (log_flags & LOG_DEBUG)
              //          LOG ("(K%d=%02X)", r.digit, r.key[r.digit] & mask);
              // get bit position
              while (!(mask & 1)) {
                  bit++;
                  mask >>= 1;
              }
              // clear COND
              r.flags &= ~FLG_COND;
              // set result to KR
              r.KR = /*(r.KR & ~0x07F0) |*/ (r.digit << 4) | ((bit << 8) & 0x0700);
              //        if (log_flags & LOG_SHORT)
              //          LOG ("KR=%04X COND=0", r.KR);
          } else
              if (r.digit) {
                  // wait for digit 0 counter - end of scan
                  r.flags |= FLG_HOLD;
                  return 11;
              }
      } else {
          // scan current row and update COND
          if (r.key[r.digit] & mask)
              r.flags &= ~FLG_COND;
          //      if (log_flags & LOG_DEBUG)
          //        LOG ("(K%d=%02X) ", r.digit, r.key[r.digit] & mask);
          //      if (log_flags & LOG_SHORT)
          //        LOG ("COND=%u", (r.flags & FLG_COND) != 0);
      }
  }
      break;
    // ================================
    // wait operations
    // ================================
    case 0x0A00:
      switch (opcode & 0x000F) {
    case 0x0000:
      // wait for digit
      if (r.digit != ((opcode >> 4) & 0x000F)) {
        r.flags |= FLG_HOLD;
        return 12;
      }
//      if (log_flags & LOG_DEBUG)
//        LOG ("(D=%u)", r.digit);
      break;
    case 0x0001:
      // Zero Idle
      r.flags &= ~FLG_IDLE;
//      if (log_flags & LOG_SHORT)
//        LOG ("IDLE=0");
      break;
    case 0x0002:
      // CLFA
      r.fA = 0;
//      if (log_flags & LOG_SHORT)
//        LOG ("FA=%04X", r.fA);
      break;
    case 0x0003:
      // Wait Busy
qWarning()<< "Unknown behaviour...";
      break;
    case 0x0004:
      // INCKR
      r.KR += 0x0010;
      if (!(r.KR & 0xFFF0))
        r.KR ^= 0x0001;
//      if (log_flags & LOG_SHORT)
//        LOG ("KR=%04X", r.KR);
      break;
    case 0x0005:
      // TKR
      if (r.KR & (1 << ((opcode >> 4) & 0x000F)))
        r.flags &= ~FLG_COND;
//      if (log_flags & LOG_DEBUG)
//        LOG ("KR=%04X ", r.KR);
//      if (log_flags & LOG_SHORT)
//        LOG ("COND=%u", (r.flags & FLG_COND) != 0);
      break;
    case 0x0006:
      // FLGR5
      if (opcode & 0x0010) {
        r.R5 = (r.fB >> 1) & 0x000F;
//        if (log_flags & LOG_DEBUG)
//          LOG ("FB=%04X ", r.fB);
//        if (log_flags & LOG_SHORT)
//          LOG ("R5=%01X", r.R5);
      } else {
        r.R5 = (r.fA >> 1) & 0x000F;
//        if (log_flags & LOG_DEBUG)
//          LOG ("FA=%04X ", r.fA);
//        if (log_flags & LOG_SHORT)
//          LOG ("R5=%01X", r.R5);
      }
      break;
    case 0x0007:
      // Number
      r.R5 = (opcode >> 4) & 0x000F;
//      if (log_flags & LOG_SHORT)
//        LOG ("R5=%01X", r.R5);
      break;
    case 0x0008:
      // KRR5/R5KR + peripherals
      switch (opcode & 0x00F0) {
        case 0x0000:
          // KRR5
          r.R5 = (r.KR >> 4) & 0x000F;
//          if (log_flags & LOG_SHORT)
//        LOG ("R5=%01X", r.R5);
          break;
        case 0x0010:
          // R5KR
          r.KR = (r.KR & ~0x00F0) | (r.R5 << 4);
//          if (log_flags & LOG_SHORT)
//        LOG ("KR=%04X", r.KR);
          break;
        case 0x0020:
          // READ
//	      r.EXT = (card_read () << 4);
          r.EXT = r.CRD_BUF[r.CRD_PTR++] << 4;
          r.flags |= FLG_EXT_VALID;
//          if (log_flags & LOG_SHORT)
//        LOG ("EXT=%04X", r.EXT);
          break;
      case 0x0030:
          // WRITE
          //	      card_write (r.KR >> 4)
          r.CRD_BUF[r.CRD_PTR++] = (r.KR >> 4);
          break;
      case 0x0040:
          // CRDOFF
          if (r.CRD_FLAGS & CRD_WRITE) {
              FILE *f;
              if ((f = fopen (card_output, "r+b")) == NULL) {
                  f = fopen (card_output, "wb");
                  qWarning()<<card_output<<" created.";
              }
              if (f == NULL) {
                  fprintf (stderr, "Can't access file %s!\n", card_output);
              } else {
                  fseek (f, CRD_LEN * ((r.CRD_BUF[2] & 0x0F) / 3), SEEK_SET);
                  fwrite (r.CRD_BUF, CRD_LEN, 1, f);
                  fclose (f);
              }
          } else
              if (r.CRD_FLAGS & CRD_READ) {
                  r.CRD_BUF[2] += 3;
              }
          r.CRD_FLAGS = 0;
          r.CRD_PTR = 0;
          // clear card switch
          if ((currentModel == TI59)||(currentModel == TI59C))
              // TI-59: D10-KR card switch normally closed
              r.key[10] |= (1 << KR_BIT);
          break;
        case 0x0050:
          // CRDREAD
          if (!r.CRD_FLAGS) {
        FILE *f;
        if ((f = fopen (card_output, "rb")) == NULL) {
          fprintf (stderr, "Can't open file %s!\n", card_output);
          break;
        }
        fseek (f, CRD_LEN * ((r.CRD_BUF[2] & 0x0F) / 3), SEEK_SET);
        fread (r.CRD_BUF, CRD_LEN, 1, f);
        fclose (f);
        r.CRD_FLAGS = CRD_READ;
        //r.CRD_PTR = 0;
          }
          break;
        case 0x0060:
          // LOAD
          //	      print_char ((r.KR >> 4) & 0x3F);
          if (mode_flags & MODE_PRINTER) {
              r.PRN_BUF[r.PRN_PTR++] = PRN_CODE[(r.KR >> 4) & 0x3F];
              ((Cti59*)pPC)->sendToPrinter =0x80 | ((r.KR >> 4) & 0x3F);
              if (r.PRN_PTR >= sizeof (r.PRN_BUF)) r.PRN_PTR = 0;
          }
          break;
      case 0x0070:
          // FUNCTION
          //	      print_func ((r.KR >> 4) & 0x7F);
          if (mode_flags & MODE_PRINTER) {
              int i;
              unsigned char code = (r.KR >> 4) & 0x7F;
              ((Cti59*)pPC)->sendToPrinter = code;
              for (i = 0; *PRN_STR[i].str; i++) {
                  if (code == PRN_STR[i].code) {
                      for (code = 3; code; ) {
                          char _code = PRN_STR[i].str[--code];
                          r.PRN_BUF[r.PRN_PTR++] = _code;
                          //              ((Cti59*)pPC)->sendToPrinter = _code;
                          if (r.PRN_PTR >= sizeof (r.PRN_BUF)) r.PRN_PTR = 0;
                      }
                  }
              }
          }
          break;
      case 0x0080:
          // CLEAR
          // clear printer buffer
          //	      print_clear ();
          if (mode_flags & MODE_PRINTER) {
              memset (r.PRN_BUF, ' ', sizeof (r.PRN_BUF));
              r.PRN_PTR = 0;
              ((Cti59*)pPC)->sendToPrinter = 0xF1; // Clear buffer
              qWarning()<<"***CLEAR BUFFER";
          }
          break;
      case 0x0090:
          // STEP
          // decrement printer position
          if (mode_flags & MODE_PRINTER) {
              r.PRN_BUF[r.PRN_PTR++] = ' ';
              ((Cti59*)pPC)->sendToPrinter = 0x80; // Spaceq
              qWarning()<<"***STEP";
              if (r.PRN_PTR >= sizeof (r.PRN_BUF)) r.PRN_PTR = 0;
              if (r.PRN_BUSY) {
                  if ((r.cycle - r.PRN_BUSY) < (unsigned)(150*pPC->getfrequency())) {
                      r.flags |= FLG_BUSY;
                  } else {
                      r.PRN_BUSY = 0;
                  }
              }
          }
          break;
      case 0x00A0:
          // PRINT
          // print data from buffer
          if (mode_flags & MODE_PRINTER) {
              int i;
              fprintf (stderr, "\r\t\t");
              for (i = 20; i; )
                  fputc (r.PRN_BUF[--i], stderr);
              // force display re-print
              r.flags ^= FLG_DISP;
          }
          ((Cti59*)pPC)->sendToPrinter = 0xF2; // Print buffer
          qWarning()<<"************Print buffer";
          break;
      case 0x00B0:
          qWarning()<<"**********PAPER ADVANCE";
          // scroll paper in printer
          if (mode_flags & MODE_PRINTER) {
              fputc ('\n', stderr);
              ((Cti59*)pPC)->sendToPrinter = 0xf3; // Paper advance
              // check if advance buttons (. or @) are pressed
              if ((r.key[9] & (1 << 3)) || (r.key[0xC] & (1 << 0))) {
                  // use real delay for button-driven paper feed
                  r.PRN_BUSY = r.cycle;
                  if (!r.PRN_BUSY) r.PRN_BUSY--;
              }
          }
          break;
      case 0x00C0:
          // CRDWRITE
          if (!r.CRD_FLAGS) {
              r.CRD_FLAGS = CRD_WRITE;
              //r.CRD_PTR = 0;
          }
          break;
      case 0x00F0:
          // RAMOP
          // next IO contains specification for RAM operation
          r.flags |= FLG_RAM_OP;
          break;
      }
          break;
      case 0x0009:
          // Set Idle
          r.flags |= FLG_IDLE;
          //      if (log_flags & LOG_SHORT)
          //        LOG ("IDLE=1");
          //      qWarning()<<"IDLE=1";
          break;
    case 0x000A:
      // CLFB
      r.fB = 0;
//      if (log_flags & LOG_SHORT)
//        LOG ("FB=%04X", r.fB);
      break;
    case 0x000B:
      // Test Busy
      if ((r.key[r.digit] & (1 << KR_BIT)) || (r.flags & FLG_BUSY))
        r.flags &= ~(FLG_COND | FLG_BUSY);
//      if (log_flags & LOG_SHORT)
//        LOG ("(K%d=%02X) COND=%u", r.digit, r.key[r.digit] & (1 << KR_BIT), (r.flags & FLG_COND) != 0);
      break;
    case 0x000C:
      // EXTKR
      r.KR = (r.KR & 0x000F) | r.EXT;
//      if (log_flags & LOG_SHORT)
//        LOG ("KR=%04X", r.KR);
      break;
    case 0x000D:
      // XKRSR
      {
        unsigned short tmp;
        tmp = r.KR;
        r.KR = r.SR;
        r.SR = tmp;
      }
//      if (log_flags & LOG_SHORT)
//        LOG ("KR=%04X SR=%04X", r.KR, r.SR);
      break;
    case 0x000E:
      // NO-OP + peripherals
      switch (opcode & 0x00F0) {
        case 0x0000:
          // FETCH
          r.EXT = get_mem(0x3000 + (r.LIB++),SIZE_8) /*LIB[r.LIB++]*/ << 4;
          r.flags |= FLG_EXT_VALID;
          r.LIB %= 5000;
//          if (log_flags & LOG_SHORT)
//        LOG ("EXT=%04X LIB.addr=%04d", r.EXT, r.LIB);
          break;
        case 0x0010:
          // LOAD PC
          r.LIB /= 10;
          r.LIB += ((r.KR >> 4) & 0xF) * 1000;
//          if (log_flags & LOG_SHORT)
//        LOG ("LIB.addr=%04d", r.LIB);
          break;
        case 0x0020:
          // UNLOAD PC
          r.EXT = (r.LIB % 10) << 4;
          r.flags |= FLG_EXT_VALID;
          //r.LIB = (r.LIB / 10) + ((r.LIB % 10) * 1000); // address is not wrapped around
          r.LIB /= 10;
//          if (log_flags & LOG_SHORT)
//        LOG ("EXT=%04X", r.EXT);
          break;
        case 0x0030:
          // FETCH HIGH
          r.EXT = get_mem(0x3000 + r.LIB,SIZE_8) /*LIB[r.LIB]*/ & 0xF0;
          r.flags |= FLG_EXT_VALID;
//          if (log_flags & LOG_SHORT)
//        LOG ("EXT=%04X", r.EXT);
          break;
      }
      break;
    case 0x000F:
      // Register
      switch (opcode & 0x00F0) {
        case 0x0000:
          // Store F
          r.flags |= FLG_STORE;
          // address is taken from last IO result - digit 0
          r.REG_ADDR = r.Sout[0];
//          if (log_flags & LOG_SHORT)
//        LOG ("STO.addr=%01X", r.REG_ADDR);
          break;
        case 0x0010:
          // Recall F
          r.flags |= FLG_RECALL;
          // address is taken from last IO result - digit 0
          r.REG_ADDR = r.Sout[0];
//          if (log_flags & LOG_SHORT)
//        LOG ("RCL.addr=%01X", r.REG_ADDR);
          break;
      }
      break;
      }
      break;
    // ================================
    // ALU operations
    // ================================
    default: {
    const mask_type *mask = &mask_info[(opcode >> 8) & 0x0F];
    struct {
      unsigned char *srcX, *srcY;
      unsigned char flags;
    } *alu_inp, ALU_OP[32] = {
      {r.A, 0, ALU_ADD},
      {r.A, 0, ALU_SUB},
      {0, r.B, ALU_ADD},
      {0, r.B, ALU_SUB},
      {r.C, 0, ALU_ADD},
      {r.C, 0, ALU_SUB},
      {0, r.D, ALU_ADD},
      {0, r.D, ALU_SUB},
      {r.A, 0, ALU_SHL},
      {r.A, 0, ALU_SHR},
      {0, r.B, ALU_SHL},
      {0, r.B, ALU_SHR},
      {r.C, 0, ALU_SHL},
      {r.C, 0, ALU_SHR},
      {0, r.D, ALU_SHL},
      {0, r.D, ALU_SHR},
      {r.A, r.B, ALU_ADD},
      {r.A, r.B, ALU_SUB},
      {r.C, r.B, ALU_ADD},
      {r.C, r.B, ALU_SUB},
      {r.C, r.D, ALU_ADD},
      {r.C, r.D, ALU_SUB},
      {r.A, r.D, ALU_ADD},
      {r.A, r.D, ALU_SUB},
    // following needs special approach...
    // -> variable pointers, RAM/SCOM access, R5 access
      {r.A, 0 /*CONSTANT[((r.KR >> 5) & 0x78) | ((r.KR >> 4) & 0x07)]*/, ALU_ADD}, // IO read
      {r.A, 0 /*CONSTANT[((r.KR >> 5) & 0x78) | ((r.KR >> 4) & 0x07)]*/, ALU_SUB}, // IO read
      {0, 0, ALU_ADD}, // IO read: 0 -> SCOM[r.REG_ADDR] | RAM[r.RAM_ADDR]
      {0, 0, ALU_SUB},
      {r.C, 0 /*CONSTANT[((r.KR >> 5) & 0x78) | ((r.KR >> 4) & 0x07)]*/, ALU_ADD}, // IO read
      {r.C, 0 /*CONSTANT[((r.KR >> 5) & 0x78) | ((r.KR >> 4) & 0x07)]*/, ALU_SUB}, // IO read
      {0, 0 /*r.R5*/, ALU_ADD}, // IO read ??
      {0, 0 /*r.R5*/, ALU_SUB} // IO read ??
    };
    struct {
      unsigned char *dst;
      char log[4];
    } *alu_out, ALU_DST[8] = {
      {r.A, "A"},
      {0, "IO"},
      {0, ""}, // Xch A,B
      {r.B, "B"},
      {r.C, "C"},
      {0, ""}, // Xch C,D
      {r.D, "D"},
      {0, ""}  // Xch A,E
    };
    alu_out = &ALU_DST[opcode & 0x07];
    if ((opcode & 0x07) == 0x01)
      r.flags |= FLG_IO_VALID;
    alu_inp = &ALU_OP[(opcode >> 3) & 0x1F];
    switch (opcode & 0x00F8) {
      default:
        // generic ALU operation
        Alu (alu_out->dst, alu_inp->srcX, alu_inp->srcY, mask, alu_inp->flags);
        break;
      // process special cases
      case 0x00C0: // A+-<io>
        Alu (alu_out->dst, r.A, CONSTANT[((r.KR >> 5) & 0x78) | ((r.KR >> 4) & 0x07)], mask, ALU_ADD);
        break;
      case 0x00C8: // A+-<io>
        Alu (alu_out->dst, r.A, CONSTANT[((r.KR >> 5) & 0x78) | ((r.KR >> 4) & 0x07)], mask, ALU_SUB);
        break;
      case 0x00D0: // NO-OP
        // LOAD from IO / mask
        if (r.flags & FLG_RECALL) {
          // load from register
          r.flags &= ~FLG_RECALL;
          Alu (alu_out->dst, 0, r.SCOM[r.REG_ADDR], mask, ALU_ADD);
//          if (alu_out->dst && (log_flags & LOG_DEBUG)) {
//        int i;
//        LOG ("[RCL.%u:", r.REG_ADDR); for (i = 15; i >= 0; i--) LOG ("%X", alu_out->dst[i]); LOG ("]");
//          }
        } else
        if ((r.flags & FLG_RAM_READ) && ((currentModel==TI59) || (currentModel==TI59C) || r.RAM_ADDR < 60)) {
          r.flags &= ~FLG_RAM_READ;
          Alu (alu_out->dst, 0, r.RAM[r.RAM_ADDR], mask, ALU_ADD);
//          if (alu_out->dst && (log_flags & LOG_DEBUG)) {
//        int i;
//        LOG ("[RAM.%u:", r.RAM_ADDR); for (i = 15; i >= 0; i--) LOG ("%X", alu_out->dst[i]); LOG ("]");
//          }
        } else {
          Alu (alu_out->dst, 0, 0, mask, ALU_ADD);
        }
        break;
      case 0x00D8:
        // LOAD (negative) from mask
        // never used for IO load but IMHO it will work the same way
        Alu (alu_out->dst, 0, 0, mask, ALU_SUB);
        break;
      case 0x00E0: // C+-<io>
        Alu (alu_out->dst, r.C, CONSTANT[((r.KR >> 5) & 0x78) | ((r.KR >> 4) & 0x07)], mask, ALU_ADD);
        break;
      case 0x00E8: // C+-<io>
        Alu (alu_out->dst, r.C, CONSTANT[((r.KR >> 5) & 0x78) | ((r.KR >> 4) & 0x07)], mask, ALU_SUB);
        break;
      case 0x00F0: // R5->Adder
      case 0x00F8: // not used in TI-58, probably different behavior...
        if (alu_out->dst) {
          int i;
          for (i = mask->start+1; i <= mask->end; i++)
        alu_out->dst[i] = 0;
          alu_out->dst[mask->cpos] = mask->cval;
          alu_out->dst[mask->start] = r.R5;
          // make BCD correction
          if (!(opcode & 0x0008))
        Alu (alu_out->dst, 0, alu_out->dst, mask, ALU_ADD);
          else
        Alu (alu_out->dst, 0, alu_out->dst, mask, ALU_SUB); // not sure with this...
        }
        break;
    }
    // EXCHANGE instructions
    switch (opcode & 0x0007) {
      case 0x0002: // A<->B
        Xch (r.A, r.B, mask);
//        if (log_flags & LOG_SHORT) {
//          int i;
//          LOG ("A="); for (i = 15; i >= 0; i--) LOG ("%X", r.A[i]);
//          LOG (" B="); for (i = 15; i >= 0; i--) LOG ("%X", r.B[i]);
//        }
        break;
      case 0x0005: // C<->D
        Xch (r.C, r.D, mask);
//        if (log_flags & LOG_SHORT) {
//          int i;
//          LOG ("C="); for (i = 15; i >= 0; i--) LOG ("%X", r.C[i]);
//          LOG (" D="); for (i = 15; i >= 0; i--) LOG ("%X", r.D[i]);
//        }
        break;
      case 0x0007: // A<->E
        Xch (r.A, r.E, mask);
//        if (log_flags & LOG_SHORT) {
//          int i;
//          LOG ("A="); for (i = 15; i >= 0; i--) LOG ("%X", r.A[i]);
//          LOG (" E="); for (i = 15; i >= 0; i--) LOG ("%X", r.E[i]);
//        }
        break;
    }
//    if (*alu_out->log && (log_flags & LOG_SHORT)) {
//      int i;
//      unsigned char *ptr = alu_out->dst;
//      if (!ptr)
//        ptr = r.Sout;
//      LOG ("%s=", alu_out->log); for (i = 15; i >= 0; i--) LOG ("%X", ptr[i]);
//    }
    // IO write control
    if (r.flags & FLG_STORE) {
      r.flags &= ~FLG_STORE;
      memcpy (r.SCOM[r.REG_ADDR], r.Sout, sizeof (r.Sout));
//      if (log_flags & LOG_SHORT) {
//        int i;
//        LOG ("STO.%u=", r.REG_ADDR); for (i = 15; i >= 0; i--) LOG ("%X", r.Sout[i]);
//      }
    }
    if (r.flags & FLG_RAM_OP) {
      r.flags &= ~FLG_RAM_OP;
      r.RAM_OP = r.Sout[0];
      r.RAM_ADDR = /*r.IO[5]*100 +*/ r.Sout[3]*10 + r.Sout[2];
      if (r.RAM_OP == 2) {
        // clear 1 memory cell
        memset (r.RAM[r.RAM_ADDR], 0, 16*1);
//        if (log_flags & LOG_DEBUG)
//          LOG ("[RAM.clr1.addr=%02d]", r.RAM_ADDR);
      } else
      if (r.RAM_OP == 4) {
        // clear 10 memory cells
        memset (r.RAM[r.RAM_ADDR], 0, 16*10);
//        if (log_flags & LOG_DEBUG)
//          LOG ("[RAM.clr10.addr=%02d]", r.RAM_ADDR);
      } else
      if (r.RAM_OP == 1) {
        r.flags |= FLG_RAM_WRITE;
//        if (log_flags & LOG_DEBUG)
//          LOG ("[RAM.wr.addr=%02d]", r.RAM_ADDR);
      } else
      if (r.RAM_OP == 0) {
        r.flags |= FLG_RAM_READ;
//        if (log_flags & LOG_DEBUG)
//          LOG ("[RAM.rd.addr=%02d]", r.RAM_ADDR);
      }
    } else
    if (r.flags & FLG_RAM_WRITE) {
      // store to RAM (ALL_MASK)
      r.flags &= ~FLG_RAM_WRITE;
      memcpy (r.RAM[r.RAM_ADDR], r.Sout, sizeof (r.Sout));
//      if (log_flags & LOG_SHORT) {
//        int i;
//        LOG ("RAM.%02u=", r.RAM_ADDR); for (i = 15; i >= 0; i--) LOG ("%X", r.Sout[i]);
//      }
    }
      }

  if (!(r.flags & FLG_IO_VALID))
    memset (r.Sout, 0, sizeof (r.Sout));

      break;
  }
  r.addr++;
  return 1;
}





