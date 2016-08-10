

#include	<stdlib.h>


#include	"common.h"
#include	"pc1403.h"
#include "Log.h"

#include	"Inter.h"
#include	"Keyb.h"
#include "Keyb1403.h"
#include "Lcdc_pc1403.h"
//#include	"ce126.h"
//#include	"sound.h"

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

/*
MT2		------------------------------------------------------------------------------------------------------------- MT_OUT2	1
 		------------------------------------------------------------------------------------------------------------- GND		2
 		------------------------------------------------------------------------------------------------------------- VGG		3
 		------------------------------------------------------------------------------------------------------------- BUSY		4
 		------------------------------------->|-----+---------------------------------------------------------------- D_OUT		5
 		--------------------------------------------)---------------------------------------------------------------- MT_IN		6
 		--------------------------------------------)---------------------------------------------------------------- MT_OUT1	7
 		--------------------------------------------)-----------+---------------------------------------------------- D_IN		8
 		--------------------------------------------)-----------)---------------------------------------------------- ACK		9
 		----------------------------+---------------)-----------)---------------------------------------------------- SEL2		10
 		----------------+-----------)---------------)-----------)---------------------------------------------------- SEL1		11
						|			|				|			|
IB5 --------------|<-----			|				|			|
									|				|			|
IB6 ---------------------------|<----				|			|
													|			|
IB7 ------------------------------------------|<-----			|
																|	
IB8 ----------------------------------------------------|<-------
*/

Cpc1403::Cpc1403(CPObject *parent)	: Cpc1401(parent)
{											//[constructor]
    setfrequency( (int) 768000/3);
    setcfgfname("pc1403");

    SessionHeader	= "PC1403PKM";
    Initial_Session_Fname ="pc1403.pkm";

    BackGroundFname	= P_RES(":/pc1403/pc1403.png");

    memsize			= 0x20000;

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	P_RES(":/pc1403/cpu-1403.rom")	, "" , CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(8 , 0x2000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x4000 ,	P_RES(":/pc1403/ba1-1403.rom")	, "" , CSlot::ROM , "BANK 1"));
    SlotList.append(CSlot(32, 0x8000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x10000 ,	P_RES(":/pc1403/ba1-1403.rom")	, "" , CSlot::ROM , "BANK 1"));
    SlotList.append(CSlot(16, 0x14000 ,	P_RES(":/pc1403/ba2-1403.rom")	, "" , CSlot::ROM , "BANK 2"));
    SlotList.append(CSlot(16, 0x18000 ,	P_RES(":/pc1403/ba3-1403.rom")	, "" , CSlot::ROM , "BANK 3"));
    SlotList.append(CSlot(16, 0x1C000 ,	P_RES(":/pc1403/ba4-1403.rom")	, "" , CSlot::ROM , "BANK 4"));

    RomBank = 0;

    KeyMap		= KeyMap1403;
    KeyMapLenght= KeyMap1403Lenght;

    delete pLCDC;	pLCDC		= new Clcdc_pc1403(this,
                                                   QRect(116,53,144*1.5,14),
                                                   QRect(119,44,210,35));
    pKEYB->setMap("pc1403.map",scandef_pc1403);


    memOffset = 0xC000;

}


bool Cpc1403::Set_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

	int port1 = Get_8(0x3A00);
	
	pCONNECTOR->Set_pin(PIN_MT_OUT2	,0);
	pCONNECTOR->Set_pin(PIN_VGG		,1);
	pCONNECTOR->Set_pin(PIN_BUSY	,GET_PORT_BIT(PORT_F,3));		// F03
    if (!pCONNECTOR->Get_pin(PIN_ACK)) pCONNECTOR->Set_pin(PIN_D_OUT,READ_BIT(port1,2));		// F02
    if (!pCONNECTOR->Get_pin(PIN_ACK)) pCONNECTOR->Set_pin(PIN_D_IN	,READ_BIT(port1,3));		// F02
	pCONNECTOR->Set_pin(PIN_MT_OUT1	,pCPU->Get_Xout());
    if (!pCONNECTOR->Get_pin(PIN_ACK)) pCONNECTOR->Set_pin(PIN_SEL2	,READ_BIT(port1,1));		// B06
    if (!pCONNECTOR->Get_pin(PIN_ACK)) pCONNECTOR->Set_pin(PIN_SEL1	,READ_BIT(port1,0));		// B05

	return(1);
}

bool Cpc1403::Get_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    pCPU->Set_Xin(pCONNECTOR->Get_pin(PIN_MT_IN));

    Set_Port_Bit(PORT_B,5,pCONNECTOR->Get_pin(PIN_SEL1));	// DIN	:	IB1
    Set_Port_Bit(PORT_B,6,pCONNECTOR->Get_pin(PIN_SEL2));	// DIN	:	IB2
    Set_Port_Bit(PORT_B,7,pCONNECTOR->Get_pin(PIN_D_OUT));	// DIN	:	IB2
    Set_Port_Bit(PORT_B,8,pCONNECTOR->Get_pin(PIN_D_IN));	// DIN	:	IB8

    int port = Get_8(0x3C00);
    PUT_BIT(port,7,pCONNECTOR->Get_pin(PIN_ACK));
    Set_8(0x3C00,port);

	return(1);
}


/*****************************************************************************/
/* Check Address ROM or RAM ?												 */
/*  ENTRY :DWORD d=Address													 */
/*  RETURN:BOOL (1=RAM,0=ROM)												 */
/*****************************************************************************/
// Virtual Fonction

bool Cpc1403::Chk_Adr(UINT32 *d,UINT32 data)
{

    if (                 (*d<=0x1FFF) )	{ return(0); }		// CPU ROM
	if ( (*d>=0x2000) && (*d<=0x2FFF) )	{ return(0); }		// unused
	if ( (*d>=0x3000) && (*d<=0x30FF) )	{ pLCDC->SetDirtyBuf(*d-0x3000);return(1); }
	if ( (*d>=0x3100) && (*d<=0x3BFF) )	{ return(1); }
	if ( (*d>=0x3C00) && (*d<=0x3DFF) )
	{
        switch (data)
        {
            case 0x00:
            case 0x08:	RomBank = 0; break;
            case 0x09:	RomBank = 1; break;
            case 0x0A:	RomBank = 2; break;
            case 0x0B:	RomBank = 3; break;

        }
        memOffset = 0xC000 + (RomBank * 0x4000);
//        if (pCPU->fp_log) fprintf(pCPU->fp_log,"BK:%02X = %i",data,RomBank);
		return(1);
	}
    if ( (*d>=0x3E00) && (*d<=0x3FFF) )	{
        out5 = data & 0x80;
        pKEYB->Set_KS( (BYTE) data & 0x7F );/*ShowPortsAuto(0);*/
        return(1);
    }
//    if ( (*d>=0x4000) && (*d<=0x7FFF) )	{*d += 0xC000 + (RomBank * 0x4000); return(0); }
    if ( (*d>=0x4000) && (*d<=0x7FFF) )	{*d += memOffset; return(0); }
    if ( (*d>=0x8000) && (*d<=0xFFFF) )	{ *d = (*d & 0x1FFF) | 0xE000; return(1); }

	return(0);

}

bool Cpc1403::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
    Q_UNUSED(data)

    if ( (*d>=0x4000) && (*d<=0x7FFF) )	{
//        *d += 0xC000 + (RomBank * 0x4000);
        *d += memOffset;
        return(1);
    }
    if ( (*d>=0x8000) && (*d<=0xFFFF) )	{ *d = (*d & 0x1FFF) | 0xE000; return(1); }
	return(1);	
}

BYTE	Cpc1403::Get_PortA(void)
{
    BYTE data = Cpc1401::Get_PortA();

    data |= out5;

    return (data);
}

Cpc1403H::Cpc1403H(CPObject *parent) : Cpc1403(parent)
{											//[constructor]
    setfrequency( (int) 768000/3);
    setcfgfname("pc1403H");

    SessionHeader	= "PC1403HPKM";
    Initial_Session_Fname ="pc1403H.pkm";

    BackGroundFname	= P_RES(":/pc1403/pc1403h.png");

}

bool Cpc1403H::Chk_Adr(UINT32 *d,UINT32 data)
{
	if ( (*d>=0x8000) && (*d<=0xdFFF) )	{ return(1); }
	return (Cpc1403::Chk_Adr(d,data));
}

bool Cpc1403H::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
    if ( (*d>=0x8000) && (*d<=0xdFFF) )	{ return(1); }
	return(Cpc1403::Chk_Adr_R(d,data));	
}


