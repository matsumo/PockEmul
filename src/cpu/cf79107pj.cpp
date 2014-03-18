#include "cf79107pj.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "Log.h"

#define VAR_X 0x400
#define VAR_Y 0x410
#define VAR_W 0x420

CCF79107PJ::CCF79107PJ(CpcXXXX *parent)
{
    pPC = parent;

}

CCF79107PJ::~CCF79107PJ()
{
}


bool CCF79107PJ::init()
{
    return true;
}

bool CCF79107PJ::exit()
{
    return true;
}

void CCF79107PJ::Reset()
{
}

bool CCF79107PJ::step()
{
    return true;
}
UINT8 CCF79107PJ::get_status()
{
    return (BCDz?0x40:0x00) | (BCDaf?0x10:0x00) | (BCDc?0x1:0x00);
}
bool CCF79107PJ::instruction1(UINT8 cmd)
{
    if (pPC->fp_log) fprintf(pPC->fp_log,"\nbefore CCF79107[1]=%02x\tpc=%08x\n",cmd,pPC->pCPU->get_PC());
    dumpXYW();

    switch(cmd) {
    case 0x04: Write_Reg(VAR_X); break;
    case 0x05: Write_Reg(VAR_Y); break;
    case 0x08: Exch_Reg(VAR_X); break;
    case 0x0e:
        ////////////////////////////////////////////////////////
        // BUG: UNACCEPTABLE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  //
        if (pPC->pCPU->get_PC()==0xf0004f24) Exch_Reg(VAR_X); // ROM Z-1
        if (pPC->pCPU->get_PC()==0xf0004f66) Exch_Reg(VAR_X); // ROM Z-1GR
        ////////////////////////////////////////////////////////
//          Exch_Reg(VAR_X);
        break;
    case 0x10: memset((char*)&(pPC->mem[VAR_X]),0,0x09); break;     // X <- 0
    case 0x11: memset((char*)&(pPC->mem[VAR_Y]),0,0x09); break;     // Y <- 0
    case 0x80: //  X <-> Y. is reg used ? What is the value of reg after the exchange ?
        for(int i = 0x410; i <= 0x418; i++) {
            quint8 _t = pPC->mem[i-0x10];
            pPC->mem[i-0x10] = pPC->mem[i];
            pPC->mem[i] = _t;
        }
        break;
    default:
        if (pPC->fp_log) fprintf(pPC->fp_log,"UNKNOWN");
    }

    if (pPC->fp_log) fprintf(pPC->fp_log,"after CCF79107[1]=%02x\tpc=%08x\n",cmd,pPC->pCPU->get_PC());
    dumpXYW();

    return true;
}

bool CCF79107PJ::instruction2(UINT8 cmd)
{
    if (pPC->fp_log) fprintf(pPC->fp_log,"\nbefore CCF79107[1]=%02x\tpc=%08x\n",cmd,pPC->pCPU->get_PC());
    dumpXYW();


    switch (cmd & 0x02) {
    case 0x00: Read_Reg(VAR_X); break;
    case 0x02: Read_Reg(VAR_Y); break;
    }

    switch (cmd & 0x0c) {
    case 0x08: cmd_shiftR_mantisse(); break;
    case 0x0c: cmd_shiftL_mantisse(); break;
    }

    switch (cmd & 0x11) {
    case 0x10: cmd_inc_exp(); break;
    case 0x11: cmd_dec_exp(); break;
    }

    switch (cmd) {
    case 0x00: cmd_add_exp();         break;
    case 0x01: cmd_sub_exp();         break;
    case 0x41: cmd_sub_mantisseXY();  break;
    case 0x43: cmd_sub_mantisseYX();  break;
    case 0x48: /*cmd_shiftR_mantisse();*/ break;
    case 0x4a: /*cmd_shiftR_mantisse();*/ break;
    case 0x4c: /*cmd_shiftL_mantisse();*/ break;
    case 0x4e: /*cmd_shiftL_mantisse();*/ break;
    case 0x90: /*cmd_inc_exp();*/               break;
    case 0x91: /*cmd_dec_exp();*/               break;
    case 0x99: /*cmd_shiftR_mantisse();*/
//               cmd_dec_exp();
               break;
    case 0x9c: /*cmd_shiftL_mantisse();*/
//               cmd_inc_exp();
               break;
    case 0xc0: cmd_add_mantisse();    break;
    case 0xc1: cmd_sub_mantisseXY();  break;
    case 0xc2: cmd_add_mantisse();    break;
    case 0xc3: cmd_sub_mantisseYX();  break;
    case 0xd0: // X -> Y
                for(int i = 0; i <= 8; i++)
                    reg[i] = pPC->mem[i+VAR_X];
                break;
    default:
        if (pPC->fp_log) fprintf(pPC->fp_log,"UNKNOWN");
    }


    if (pPC->fp_log) fprintf(pPC->fp_log,"after CCF79107[1]=%02x\tpc=%08x\n",cmd,pPC->pCPU->get_PC());
    dumpXYW();
    return true;
}


void CCF79107PJ::cmd_shiftL_mantisse(void) {

    for(int i = 0; i < 6; i++) {
        quint8 _tmp = reg[i]&0xf0;
        reg[i] = (reg[i+1]&0x0f)<<4;
        reg[i] |= _tmp>>4;
    }
    reg[6] = (reg[6]&0xf0)>>4;

}

void CCF79107PJ::cmd_shiftR_mantisse(void) {

    for(int i = 6; i > 0; i--){
        quint8 _tmp = reg[i]&0x0f;
        reg[i] = (reg[i-1]&0xf0)>>4;
        reg[i] |= _tmp<<4;
    }
    reg[0]=(reg[0]&0x0f)<<4;

}

void CCF79107PJ::Read_Reg(UINT16 adr) {
    memcpy((char*)&reg,(char*)&(pPC->mem[adr]),0x10);
}

void CCF79107PJ::Exch_Reg(UINT16 adr) {
    if (adr != 0) {
        UINT8 _tmp[0x10];
        memcpy((char*)&_tmp,(char*)&(pPC->mem[adr]),0x10);
        memcpy((char*)&(pPC->mem[adr]),(char*)&reg,0x10);
        memcpy((char*)&reg,(char*)&_tmp,0x10);
    }
}

void CCF79107PJ::Write_Reg(UINT16 adr) {
    if (adr != 0) {
        memcpy((char*)&(pPC->mem[adr]),(char*)&reg,0x10);
    }
}

void CCF79107PJ::cmd_inc_exp()
{
    UINT16 res0, res1;

    res0 = make_bcd_add(reg[7], 1);
    reg[7] = res0 & 0xff;
    res1 = (res0>0xff) ? 1 : 0 ;
    res1 = make_bcd_add(reg[8],  res1);
    reg[8] = res1 & 0xff;

    BCDz = ((res0 || res1)==0 );
    BCDc = (res1 > 0xff);

}

void CCF79107PJ::cmd_dec_exp(void)
{
    UINT16 res0, res1;

    res0 = make_bcd_sub(reg[7], 1);
    reg[7] = res0 & 0xff;
    res1 = (res0>0xff) ? 1 : 0 ;
    res1 = make_bcd_sub(reg[8],  res1);
    reg[8] = res1 & 0xff;

    BCDz = ((res0 || res1)==0);
    BCDc = (res1 > 0xff);

}

void CCF79107PJ::cmd_add_exp(void)
{
    UINT16 res0, res1;


    res0 = make_bcd_add(pPC->mem[VAR_X+7], pPC->mem[VAR_Y+7]);
    reg[7] = res0 & 0xff;
    res1 = (res0>0xff) ? 1 : 0 ;
    res1 = make_bcd_add(pPC->mem[VAR_X+8], pPC->mem[VAR_Y+8] + res1);
    reg[8] = res1 & 0xff;

    BCDz = ((res0 || res1)==0);
    BCDc = (res1 > 0xff);

}

void CCF79107PJ::cmd_sub_exp()
{
    UINT16 res0, res1;

    res0 = make_bcd_sub(pPC->mem[VAR_X+7], pPC->mem[VAR_Y+7]);
    reg[7] = res0 & 0xff;
    res1 = (res0>0xff) ? 1 : 0 ;
#if 1
    res1 = make_bcd_sub(pPC->mem[VAR_X+8], pPC->mem[VAR_Y+8] + res1);
    reg[8] = res1 & 0xff;
#else
    reg[8] = 0x00;
#endif
    BCDz = ((res0 || res1)==0);
    BCDc = (res1 > 0xff);

}


void CCF79107PJ::cmd_add_mantisse(void) {

    UINT8 c, f;
    UINT16 res = 0;

    f = 0;
    c = 0;
    for (int n=0; n<7; n++)
    {
        res = make_bcd_add(pPC->mem[VAR_X+n], pPC->mem[VAR_Y+n] + c);
        c = (res > 0xff) ? 1 : 0;
        f |= (res&0xff);
        reg[n] = res&0xff;
        if (pPC->fp_log) fprintf(pPC->fp_log,"mem[%04x]=%02X  f=%c c=%c\n",n,res&0xff,(f==0?'1':'0'),res>0xff?'1':'0');
    }
    BCDz = (f==0);
    BCDc = (res > 0xff);

}

void CCF79107PJ::cmd_sub_mantisseXY(void) {

    UINT8 c, f;
    UINT16 res = 0;

    c = 0;
    f = 0;
    for (int n=0; n<7; n++)
    {
        res = make_bcd_sub(pPC->mem[VAR_X+n], pPC->mem[VAR_Y+n] + c);
        c = (res > 0xff) ? 1 : 0;
        f |= (res&0xff);
        reg[n] = res&0xff;
        if (pPC->fp_log) fprintf(pPC->fp_log,"mem[%04x]=%02X  f=%c c=%c af=%c\n",n,res&0xff,(f==0?'1':'0'),res>0xff?'1':'0',
                BCDaf?'1':'0');
    }
    BCDz = (f==0);
    BCDc = (res > 0xff);

}


void CCF79107PJ::cmd_sub_mantisseYX(void) {
    UINT8 c, f;
    UINT16 res = 0;


    c = 0;
    f = 0;
    for (int n=0; n<7; n++)
    {
        res = make_bcd_sub(pPC->mem[VAR_Y+n], pPC->mem[VAR_X+n] + c);
        c = (res > 0xff) ? 1 : 0;
        f |= (res&0xff);
        reg[n] = res&0xff;
        if (pPC->fp_log) fprintf(pPC->fp_log,"mem[%04x]=%02X  f=%c c=%c\n",n,res&0xff,(f==0?'1':'0'),res>0xff?'1':'0');
    }
    BCDz = (f==0);
    BCDc = (res > 0xff);

}



void CCF79107PJ::Load_Internal(QXmlStreamReader *)
{
}

void CCF79107PJ::save_internal(QXmlStreamWriter *)
{
}

inline UINT16 CCF79107PJ::make_bcd_add(UINT8 arg1, UINT8 arg2)
{
    quint32 ret = (arg1&0x0f) + (arg2&0x0f);
    UINT8 carry;

    if (ret > 0x09)
    {
        ret = (ret + 0x06) & 0x0f;
        carry = 1;
        BCDaf = true;
    }
    else {
        carry = 0;
        BCDaf = false;
    }

    ret += ((arg1&0xf0) + (arg2&0xf0) + (carry<<4));

    if (ret > 0x9f)
    {
        ret = (ret + 0x60) & 0x0ff;
        carry = 1;
    }
    else
        carry = 0;

    ret += (carry<<8);

    return ret;
}

inline UINT16 CCF79107PJ::make_bcd_sub(UINT8 arg1, UINT8 arg2)
{

    quint32 ret = (arg1&0x0f) - (arg2&0x0f);
    UINT8 carry;

    if (ret > 0x09) {
        ret = (ret - 0x06) & 0x0f;
        carry = 1;
        BCDaf = true;
    }
    else {
        carry = 0;
        BCDaf = false;
    }

    ret += ((arg1&0xf0) - (arg2&0xf0) - (carry<<4));
    if (ret > 0x9f) {
        ret = (ret - 0x60) & 0x0ff;
        carry = 1;
    }
    else carry = 0;

    ret -= (carry<<8);
    return ret;
}

void CCF79107PJ::dumpXYW(void) {
#if 0
    QString _tmp="";
    for(int i = 0; i <= 0x0f; i++)
        _tmp = _tmp+QString("%1 ").arg(reg[i],2,16,QChar('0'));
    AddLog(LOG_TEMP,"T="+_tmp);
    _tmp="";
    for(int i = 0x400; i <= 0x40f; i++)
        _tmp = _tmp+QString("%1 ").arg(pPC->mem[i],2,16,QChar('0'));
    AddLog(LOG_TEMP,"X="+_tmp);
    _tmp="";
    for(int i = 0x410; i <= 0x41f; i++)
        _tmp = _tmp+QString("%1 ").arg(pPC->mem[i],2,16,QChar('0'));
    AddLog(LOG_TEMP,"Y="+_tmp);
    _tmp="";
    for(int i = 0x420; i < 0x42f; i++)
        _tmp = _tmp+QString("%1 ").arg(pPC->mem[i],2,16,QChar('0'));
    AddLog(LOG_TEMP,"W="+_tmp);
#endif
    if (pPC->fp_log) {
        fprintf(pPC->fp_log,"BCDz=%02x  BCDc=%02x\n",BCDz,BCDc);
        fprintf(pPC->fp_log,"T=");
        for(int i = 0x0; i <= 0x0f; i++) {
            if (i == 7) fprintf(pPC->fp_log,":");
            fprintf(pPC->fp_log,"%02x ", reg[i]);
        }
        fprintf(pPC->fp_log,"\n");
        fprintf(pPC->fp_log,"X=");
        for(int i = 0x400; i <= 0x40f; i++) {
            if (i == 0x407) fprintf(pPC->fp_log,":");
            fprintf(pPC->fp_log,"%02x ", pPC->mem[i]);
        }
        fprintf(pPC->fp_log,"\n");
        fprintf(pPC->fp_log,"Y=");
        for(int i = 0x410; i <= 0x41f; i++) {
            if (i == 0x417) fprintf(pPC->fp_log,":");
            fprintf(pPC->fp_log,"%02x ", pPC->mem[i]);
        }
        fprintf(pPC->fp_log,"\n");
        fprintf(pPC->fp_log,"W=");
        for(int i = 0x420; i <= 0x42e; i++) {
            if (i == 0x427) fprintf(pPC->fp_log,":");
            fprintf(pPC->fp_log,"%02x ", pPC->mem[i]);
        }
        fprintf(pPC->fp_log,"\n");
    }

}


/*
      0  /  1
Bit 0 : Add  / Sub
Bit 1 : source X / Y
Bit 2 : Right/Left
Bit 3 : no   / Shift
Bit 4 :   Exponent
Bit 5 :
Bit 6 :   Mantisse

00	00000000    Add exp
01	00000001    Sub exp
41	01000001	Sub mantisse flags
43	01000011	Sub mantisse flags and src Y
48	01001000	Shift Right
4A	01001010	Shift Right and src Y
4C	01001100	Shift Left
4E	01001110	Shift Leftand src Y
90	10010000	Add Exp 1
91	10010001	Sub Exp 1
99	10011001	Shift Right and sub Exp 1
9c	10011100	Shift Left and add Exp 1
C0	11000000	Add Mantisse
C1	11000001	Sub Mantisse
C2	11000010	Add Matisse and src Y
C3	11000011	Sub Matisse and src Y
D0	11010000	Cpy X to



bit 0 : X  /  Y
bit 2 : Write reg
bit 3 : exchange reg
bit 4 : clear

bit
port 221:
04	00000100	Write reg to X
05	00000101	Write reg to Y
08	00001000	exchange reg and X ???
0E	00001110	exchange reg and X ???
10	00010000	clear X
11	00010001	clear Y
80	10000000	swap X Y




20 DATA &H04,&H41,&H04,&H43,&H04,&H48,&H04,&H4A,&H04,&H4C,&H04,&H4E,&H04,&H90
30 DATA &H04,&H91,&H04,&H99,&H04,&H9C,&H04,&HC0,&H04,&HC1,&H04,&HC2,&H04,&HC3
40 DATA &H04,&HD0,&H05,&H41,&H05,&H43,&H05,&H48,&H05,&H4A,&H05,&H4C,&H05,&H4E
50 DATA &H05,&H90,&H05,&H91,&H05,&H99,&H05,&H9C,&H05,&HC0,&H05,&HC1,&H05,&HC2
60 DATA &H05,&HC3,&H05,&HD0,&H08,&H00,&H08,&H01
70 READ A,B:LPRINT  HEX$(A) HEX$(B)
80 POKE &H2030,A
90 POKE &H2031,B
100 CALL &H2032
110 FOR I=0 TO 2
120 FOR J=0 TO 15
130 LPRINT  RIGHT$(HEX$(PEEK(&H2000+I*&H10+J)),2);
140 NEXT J
150 LPRINT
160 NEXT I
170 IF NOT(A=&H08 AND B=&H01) THEN 70


 1  ORG 2000H
    2
    3 VARX    DB 10H DUP(?)
    4 VARY    DB 10H DUP(?)
    5 VARW    DB 10H DUP(?)
    6
    7 PORTH   DB 04H
    8 PORTL   DB 48H
    9
   10         CLD
   11         MOV     DI,400H
   12         MOV     SI,OFFSET INITX
   13         MOV     CX, 20H
   14         XOR AX,AX
   15         MOV ES,AX
   16         REP MOVSB
   17
   18         MOV     DX, 220H
   19         MOV     AL, PORTL
   20         MOV     AH, PORTH
   21         OUT     DX, AX
   22         CLD
   23         MOV     SI, 400H
   24         MOV     DI, OFFSET VARX
   25         MOV     CX, 2EH
   26         XOR AX,AX
   27         MOV ES,AX
   28         REP MOVSB
   29     IRET
   30
   31 INITX   DB 51H,62H,73H,84H,95H,06H,07H,01H
   32         DB 0,0,0,0,0,0,0,0
   33 INITY   DB 41H,52H,63H,74H,85H,09H,87H,05H
   34         DB 0,0,0,0,0,0,0,0
   35
   36  END


  */
