/*** POCKEMUL [PC1401.CPP] ********************************/
/* PC1401 emulator main class                             */
/**********************************************************/
//#include <string.h>
//#include <stdlib.h>
 
#include "common.h"
#include "pc1401.h"
#include "Lcdc_pc1401.h"
#include "Log.h"
#include "Inter.h"
#include "Keyb.h"
//#include "Keyb1401.h"
#include "cextension.h"
#include "sc61860.h"
#include "Keyb.h"
#include "Connect.h"

Cpc1401::Cpc1401(CPObject *parent)	: CpcXXXX(parent)
{								//[constructor]
    setfrequency( (int) 576000/3);
    setcfgfname("pc1401");

    SessionHeader	= "PC1401PKM";
    Initial_Session_Fname ="pc1401.pkm";

    BackGroundFname	= P_RES(":/pc1401/pc1401.png");
    RightFname  = P_RES(":/pc1250/PC1250Right.png");
    BackFname   = P_RES(":/pc1401/pc1401Back.png");
    TopFname    = P_RES(":/pc1401/pc1401Top.png");
    BottomFname = P_RES(":/pc1401/pc1401Top.png");

    memsize			= 0x10000;
    LeftFname = P_RES(":/pc1250/125xLeft.png");

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	P_RES(":/pc1401/cpu-1401.rom"), "pc-1401/cpu-1401.rom" , CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(24, 0x2000 ,	"",								"pc-1401/R1-1401.ram" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(32, 0x8000 ,	P_RES(":/pc1401/bas-1401.rom"), "pc-1401/bas-1401.rom" , CSlot::ROM , "BASIC ROM"));

//    KeyMap		= KeyMap1401;
//    KeyMapLenght= KeyMap1401Lenght;

    setDXmm(170);
    setDYmm(72);
    setDZmm(10);

    setDX(633);
    setDY(252);

    cnt = 0;

    pLCDC		= new Clcdc_pc1401(this,
                                   QRect(119,53,206,21),
                                   QRect(119,44,210,35));
    pCPU		= new CSC61860(this);
    pTIMER		= new Ctimer(this);
    pKEYB->setMap("pc1401.map");


}

bool Cpc1401::CheckUpdateExtension(CExtension *ext)
{
		if (ext)
			if (ext->Id == "ce-125")
			{
				switch(QMessageBox::warning(mainwindow, "PockEmul",
	                                    "This model does not fit correctly into the CE-125.\n"
	                                    "You risk connector damage !!!\n "
	                                    "Are you sure ????\n",
	                                    "Yes",
	                                    "Abort", 0, 0, 1)) {
	        	case 0: // The user clicked the Continue again button or pressed Enter
	            // try again
	            	return true;
	        	    break;
	        	case 1: // The user clicked the Cancel or pressed Escape
	        	    // exit
	        	    return false;
	        	    break;
	        	}
	       	}
       	return true;
}


void Cpc1401::TurnON(void)
{
	CpcXXXX::TurnON();


}

bool Cpc1401::init()
{
    CpcXXXX::init();
    pCONNECTOR	= new Cconnector(this,11,0,Cconnector::Sharp_11,"Connector 11 pins",false,QPoint(0,90));	publish(pCONNECTOR);
    return true;
}


//BYTE	Cpc1401::Get_PortA(void)
//{
//    qint8 data = pKEYB->Read(IO_A);
//    return(data);
//}
//BYTE scandef_pc1401[] = {
////+0		+1			+2			+3			+4			+5			+6			+7
//K_SIGN,		'8',		'2',		'5',		K_CAL,		'q',		'a',		'z',
//'.',		'9',		'3',		'6',		K_BASIC,	'w',		's',		'x',
//'+',		'/',		'-',		'*',		K_CTRL,		'e',		'd',		'c',
//')',		'(',		K_SQR,		K_ROOT,		K_POT,		K_EXP,		K_XM,		'=',
//K_STAT,		K_1X,		K_LOG,		K_LN,		K_DEG,		K_HEX,		NUL,		K_MPLUS,
//K_CCE,		K_FSE,		K_TAN,		K_COS,		K_SIN,		K_HYP,		K_SHT,		K_RM,
//NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,
//NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,

//NUL,		'7',		'1',		'4',		K_DA,		'r',		'f',		'v',
//NUL,		NUL,		',',		'p',		K_UA,		't',		'g',		'b',
//NUL,		NUL,		NUL,		'o',		K_LA,		'y',		'h',		'n',
//NUL,		NUL,		NUL,		NUL,		K_RA,		'u',		'j',		'm',
//NUL,		NUL,		NUL,		NUL,		NUL,		'i',		'k',		' ',
//NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		'l',		K_RET,
//NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		NUL,		'0',

//};



BYTE	Cpc1401::Get_PortA(void)
{
    int data = 0;

    if (IO_B & 1) {
        if (KEY(K_SIGN))			data|=0x01;
        if (KEY('8'))			data|=0x02;
        if (KEY('2'))			data|=0x04;
        if (KEY('5'))			data|=0x08;
        if (KEY(K_CAL))			data|=0x10;
        if (KEY('Q'))			data|=0x20;
        if (KEY('A'))			data|=0x40;
        if (KEY('Z'))			data|=0x80;
    }
    if (IO_B & 2) {
        if (KEY('.'))			data|=0x01;
        if (KEY('9'))			data|=0x02;
        if (KEY('3'))			data|=0x04;
        if (KEY('6'))			data|=0x08;
        if (KEY(K_BASIC))			data|=0x10;
        if (KEY('W'))			data|=0x20;
        if (KEY('S'))			data|=0x40;
        if (KEY('X'))			data|=0x80;
    }
    if (IO_B & 4) {
        if (KEY('+'))			data|=0x01;
        if (KEY('/'))			data|=0x02;
        if (KEY('-'))			data|=0x04;
        if (KEY('*'))			data|=0x08;
        if (KEY(K_CTRL))			data|=0x10;
        if (KEY('E'))			data|=0x20;
        if (KEY('D'))			data|=0x40;
        if (KEY('C'))			data|=0x80;
    }
    if (IO_B & 8) {
        if (KEY(')'))			data|=0x01;
        if (KEY('('))			data|=0x02;
        if (KEY(K_SQR))			data|=0x04;
        if (KEY(K_ROOT))			data|=0x08;
        if (KEY(K_POT))			data|=0x10;
        if (KEY(K_EXP))			data|=0x20;
        if (KEY(K_XM))			data|=0x40;
        if (KEY('='))			data|=0x80;
    }
    if (IO_B & 0x10) {
        if (KEY(K_STAT))			data|=0x01;
        if (KEY(K_1X))			data|=0x02;
        if (KEY(K_LOG))			data|=0x04;
        if (KEY(K_LN))			data|=0x08;
        if (KEY(K_DEG))			data|=0x10;
        if (KEY(K_HEX))			data|=0x20;
//		if (KEY(''))			data|=0x40;
        if (KEY(K_MPLUS))			data|=0x80;
    }
    if (IO_B & 0x20) {
        if (KEY(K_CCE))			data|=0x01;
        if (KEY(K_FSE))			data|=0x02;
        if (KEY(K_TAN))			data|=0x04;
        if (KEY(K_COS))			data|=0x08;
        if (KEY(K_SIN))			data|=0x10;
        if (KEY(K_HYP))			data|=0x20;
        if (KEY(K_SHT))			data|=0x40;
        if (KEY(K_RM))			data|=0x80;
    }


    if (IO_A & 0x01) {
        if (KEY('7'))			data|=0x02;
        if (KEY('1'))			data|=0x04;
        if (KEY('4'))			data|=0x08;
        if (KEY(K_DA))			data|=0x10;
        if (KEY('R'))			data|=0x20;
        if (KEY('F'))			data|=0x40;
        if (KEY('V'))			data|=0x80;
    }
    if (IO_A & 0x02) {
        if (KEY(','))			data|=0x04;
        if (KEY('P'))			data|=0x08;
        if (KEY(K_UA))			data|=0x10;
        if (KEY('T'))			data|=0x20;
        if (KEY('G'))			data|=0x40;
        if (KEY('B'))			data|=0x80;
    }
    if (IO_A & 0x04) {
        if (KEY('O'))			data|=0x08;
        if (KEY(K_LA))			data|=0x10;
        if (KEY('Y'))			data|=0x20;
        if (KEY('H'))			data|=0x40;
        if (KEY('N'))			data|=0x80;
    }
    if (IO_A & 0x08) {
        if (KEY(K_RA))			data|=0x10;
        if (KEY('U'))			data|=0x20;
        if (KEY('J'))			data|=0x40;
        if (KEY('M'))			data|=0x80;
    }
    if (IO_A & 0x10) {
        if (KEY('I'))			data|=0x20;
        if (KEY('K'))			data|=0x40;
        if (KEY(' '))			data|=0x80;
    }
    if (IO_A & 0x20) {
        if (KEY('L'))			data|=0x40;
        if (KEY(K_RET))			data|=0x80;
    }
    if (IO_A & 0x40) {
        if (KEY('0'))			data|=0x80;
    }

    return data;
}

//void	Cpc1401::Set_PortA(BYTE data)
//{


//	if ((IO_A != 0) && (data == 0))
//	{
//		++cnt;
//        if (cnt > 2)
//		{
//			pKEYB->keyscan();
//			cnt = 0;
//		}
//	}
//	IO_A = data;
//}
			
//void	Cpc1401::Set_PortB(BYTE data)
//{
//	IO_B = data;
//	pKEYB->Set_KS(data);
//}

// PIN_MT_OUT2	1
// PIN_GND		2
// PIN_VGG		3
// PIN_BUSY		4
// PIN_D_OUT	5
// PIN_MT_IN	6
// PIN_MT_OUT1	7
// PIN_D_IN		8
// PIN_ACK		9
// PIN_SEL2		10
// PIN_SEL1		11

bool Cpc1401::Set_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    pCONNECTOR->Set_pin(PIN_MT_OUT2	,0);
	pCONNECTOR->Set_pin(PIN_VGG		,1);
    pCONNECTOR->Set_pin(PIN_BUSY	,GET_PORT_BIT(PORT_F,1));		// F01
    pCONNECTOR->Set_pin(PIN_D_OUT	,GET_PORT_BIT(PORT_F,2));		// F02
	pCONNECTOR->Set_pin(PIN_MT_OUT1	,pCPU->Get_Xout());
    pCONNECTOR->Set_pin(PIN_SEL2	,0);
    pCONNECTOR->Set_pin(PIN_SEL1	,0);

    return(1);
}

bool Cpc1401::Get_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

	Set_Port_Bit(PORT_B,8,pCONNECTOR->Get_pin(PIN_D_IN));	// DIN	:	IB7
	Set_Port_Bit(PORT_B,7,pCONNECTOR->Get_pin(PIN_ACK));	// ACK	:	IB8
	pCPU->Set_Xin(pCONNECTOR->Get_pin(PIN_MT_IN));

    return(1);
}




/*****************************************************************************/
/* Check Address ROM or RAM ?												 */
/*  ENTRY :DWORD d=Address													 */
/*  RETURN:bool (1=RAM,0=ROM)												 */
/*****************************************************************************/
// Virtual Fonction

bool Cpc1401::Chk_Adr(UINT32 *d,UINT32 data)
{
    Q_UNUSED(data)

    if (                 (*d<=0x1FFF) )	return(0);			// ROM area(0000-1fff)
	if ( (*d>=0x8000) && (*d<=0xFFFF) )	return(0);			// ROM area(8000-ffff) 
	if ( (*d>=0x6000) && (*d<=0x67FF) ) {	pLCDC->SetDirtyBuf(*d-0x6000);	return(1);	}
	if ( (*d>=0x2800) && (*d<=0x47FF) )	return(1);
	if ( (*d>=0x6800) && (*d<=0x685F) )	return(1); 

	return(0);
}

bool Cpc1401::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
    Q_UNUSED(d)
    Q_UNUSED(data)

    return true;
}



