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
#include "Keyb1401.h"
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

    memsize			= 0x10000;
    LeftFname = P_RES(":/pc1250/125xLeft.png");

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	P_RES(":/pc1401/cpu-1401.rom"), "pc-1401/cpu-1401.rom" , CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(24, 0x2000 ,	"",								"pc-1401/R1-1401.ram" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(32, 0x8000 ,	P_RES(":/pc1401/bas-1401.rom"), "pc-1401/bas-1401.rom" , CSlot::ROM , "BASIC ROM"));

    KeyMap		= KeyMap1401;
    KeyMapLenght= KeyMap1401Lenght;

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
    pKEYB		= new Ckeyb(this,"pc1401.map",scandef_pc1401);


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


BYTE	Cpc1401::Get_PortA(void)
{
    qint8 data = pKEYB->Read(IO_A);
    return(data);
}

void	Cpc1401::Set_PortA(BYTE data)
{


	if ((IO_A != 0) && (data == 0))
	{
		++cnt;
        if (cnt > 2)
		{
			pKEYB->keyscan();
			cnt = 0;
		}
	}
	IO_A = data;
}
			
void	Cpc1401::Set_PortB(BYTE data)
{
	IO_B = data;
	pKEYB->Set_KS(data);
}

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



