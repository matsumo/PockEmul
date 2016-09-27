#include "pc1425.h"
#include "Lcdc_pc1403.h"
#include "Log.h"

BYTE scandef_pc1425[] = {
//+0		+1			+2			+3			+4			+5			+6			+7
'7',		'8',		'9',		'/',		K_XM,		NUL,		NUL,		NUL,
'4',		'5',		'6',		'*',		K_RM,		K_SHT,		K_CTRL,		K_SML,
'1',		'2',		'3',		'-',		K_MPLUS,	'q',		'a',		'z',
'0',		K_SIGN,		'.',		'+',		'=',		'w',		's',		'x',
K_HYP,		K_SIN,		K_COS,		K_TAN,		NUL,		'e',		'd',		'c',
K_HEX,		K_DEG,		K_LN,		K_LOG,		NUL,		'r',		'f',		'v',
K_EXP,		K_POT,		K_ROOT,		K_SQR,		NUL,		't',		'g',		'b',
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,

NUL,		K_CCE,		K_STAT,		K_FSE,		K_DA,		'y',		'h',		'n',
NUL,		NUL,		')',		K_1X,		K_UA,		'u',		'j',		'm',
NUL,		NUL,		NUL,		'(',		K_LA,		'i',		'k',		' ',
NUL,		NUL,		NUL,		NUL,		K_RA,		'o',		'l',		K_RET,
NUL,		NUL,		NUL,		NUL,		NUL,		'p',		',',		K_BASIC,
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		K_CAL,
NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
};

Cpc1425::Cpc1425(CPObject *parent) : Cpc1403(parent)
{											//[constructor]
    setfrequency( (int) 768000/3);
    setcfgfname("pc1425");

    SessionHeader	= "PC1425PKM";
    Initial_Session_Fname ="pc1425.pkm";

    setDZmm(16);

    BackGroundFname	= P_RES(":/pc1425/pc1425.png");

    RightFname = P_RES(":/pc1350/pc1350Right.png");
    LeftFname  = P_RES(":/pc1350/pc1350Left.png");
    TopFname   = P_RES(":/pc1350/pc1350Top.png");
    BottomFname= P_RES(":/pc1350/pc1350Bottom.png");

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	P_RES(":/pc1425/cpu-1425.bin")	, "" , CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(8 , 0x2000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x4000 ,	P_RES(":/pc1425/b0-1425.bin"), "" , CSlot::ROM , "BANK 1"));
    SlotList.append(CSlot(32, 0x8000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x10000 ,	P_RES(":/pc1425/b0-1425.bin"), "" , CSlot::ROM , "BANK 1"));
    SlotList.append(CSlot(16, 0x14000 ,	P_RES(":/pc1425/b1-1425.bin"), "" , CSlot::ROM , "BANK 2"));
    SlotList.append(CSlot(16, 0x18000 ,	P_RES(":/pc1425/b2-1425.bin"), "" , CSlot::ROM , "BANK 3"));
    SlotList.append(CSlot(16, 0x1C000 ,	P_RES(":/pc1425/b3-1425.bin"), "" , CSlot::ROM , "BANK 4"));

    delete pLCDC;	pLCDC = new Clcdc_pc1425(this,
                                             QRect(130,56,144*4.0/3,15),
                                             QRect(130,44,196,35));
    pKEYB->setMap("pc1425.map");


}

bool Cpc1425::Chk_Adr(UINT32 *d,UINT32 data)
{

    if ( (*d>=0x8000) && (*d<=0xFFFF) )	{ return(1); }
    return (Cpc1403::Chk_Adr(d,data));
}

bool Cpc1425::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
    if ( (*d>=0x8000) && (*d<=0xFFFF) )	{ return(1); }

    return(Cpc1403::Chk_Adr_R(d,data));
}


BYTE	Cpc1425::Get_PortA(void)
{
    UINT8 ks = pKEYB->Get_KS();
    int data = 0;

    if (ks & 1) {
        if (KEY('7'))			data|=0x01;
        if (KEY('8'))			data|=0x02;
        if (KEY('9'))			data|=0x04;
        if (KEY('/'))			data|=0x08;
        if (KEY(K_XM))			data|=0x10;
    }
    if (ks & 2) {
        if (KEY('4'))			data|=0x01;
        if (KEY('5'))			data|=0x02;
        if (KEY('6'))			data|=0x04;
        if (KEY('*'))			data|=0x08;
        if (KEY(K_RM))			data|=0x10;
        if (KEY(K_INS))			data|=0x20;
        if (KEY(K_CTRL))		data|=0x40;
        if (KEY(K_SML))			data|=0x80;
    }
    if (ks & 4) {
        if (KEY('1'))			data|=0x01;
        if (KEY('2'))			data|=0x02;
        if (KEY('3'))			data|=0x04;
        if (KEY('-'))			data|=0x08;
        if (KEY(K_MPLUS))			data|=0x10;
        if (KEY('Q'))			data|=0x20;
        if (KEY('A'))			data|=0x40;
        if (KEY('Z'))			data|=0x80;
    }
    if (ks & 8) {
        if (KEY('0'))			data|=0x01;
        if (KEY(K_SIGN))			data|=0x02;
        if (KEY('.'))			data|=0x04;
        if (KEY('+'))			data|=0x08;
        if (KEY('='))			data|=0x10;
        if (KEY('W'))			data|=0x20;
        if (KEY('S'))			data|=0x40;
        if (KEY('X'))			data|=0x80;
    }
    if (ks & 0x10) {
        if (KEY(K_SHT))			data|=0x01;
        if (KEY(K_SHT2))			data|=0x01;
        if (KEY(K_SIN))			data|=0x02;
        if (KEY(K_COS))			data|=0x04;
        if (KEY(K_TAN))			data|=0x08;
//        if (KEY())			data|=0x10;
        if (KEY('E'))			data|=0x20;
        if (KEY('D'))			data|=0x40;
        if (KEY('C'))			data|=0x80;
    }
    if (ks & 0x20) {
        if (KEY(K_HEX))			data|=0x01;
        if (KEY(K_DEG))			data|=0x02;
        if (KEY(K_LN))			data|=0x04;
        if (KEY(K_LOG))			data|=0x08;
//        if (KEY())			data|=0x10;
        if (KEY('R'))			data|=0x20;
        if (KEY('F'))			data|=0x40;
        if (KEY('V'))			data|=0x80;
    }
    if (ks & 0x40) {
        if (KEY(K_EXP))			data|=0x01;
        if (KEY(K_POT))			data|=0x02;
        if (KEY(K_ROOT))			data|=0x04;
        if (KEY(K_SQR))			data|=0x08;
//        if (KEY())			data|=0x10;
        if (KEY('T'))			data|=0x20;
        if (KEY('G'))			data|=0x40;
        if (KEY('B'))			data|=0x80;
    }


    if (IO_A & 0x01) {
        if (KEY(K_CCE))			data|=0x02;
        if (KEY(K_STAT))			data|=0x04;
        if (KEY(K_FSE))			data|=0x08;
        if (KEY(K_DA))			data|=0x10;
        if (KEY('Y'))			data|=0x20;
        if (KEY('H'))			data|=0x40;
        if (KEY('N'))			data|=0x80;
    }
    if (IO_A & 0x02) {
        if (KEY(')'))			data|=0x04;
        if (KEY(K_1X))			data|=0x08;
        if (KEY(K_UA))			data|=0x10;
        if (KEY('U'))			data|=0x20;
        if (KEY('J'))			data|=0x40;
        if (KEY('M'))			data|=0x80;
    }
    if (IO_A & 0x04) {
        if (KEY('('))			data|=0x08;
        if (KEY(K_LA))			data|=0x10;
        if (KEY('I'))			data|=0x20;
        if (KEY('K'))			data|=0x40;
        if (KEY(' '))			data|=0x80;
    }
    if (IO_A & 0x08) {
        if (KEY(K_RA))			data|=0x10;
        if (KEY('O'))			data|=0x20;
        if (KEY('L'))			data|=0x40;
        if (KEY(K_RET))			data|=0x80;
    }
    if (IO_A & 0x10) {
        if (KEY('P'))			data|=0x20;
        if (KEY(','))			data|=0x40;
        if (KEY(K_BASIC))			data|=0x80;
    }
    if (IO_A & 0x20) {
        if (KEY(K_CAL))			data|=0x80;
    }


    data |= out5;
    return data;
}
