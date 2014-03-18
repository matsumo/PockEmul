
#include <QDebug>

#include	"common.h"
#include	"pc1360.h"
#include "cextension.h"
#include "Lcdc.h"
#include "sc61860.h"
#include	"Inter.h"
//#include	"debug.h"
#include	"Keyb.h"
#include "Keyb1360.h"
#include	"Log.h"

Cpc1360::Cpc1360(CPObject *parent)	: Cpc13XX(parent)
{								//[constructor]
    setcfgfname("pc1360");

    SessionHeader	= "PC1360PKM";
    Initial_Session_Fname ="pc1360.pkm";

    BackGroundFname	= P_RES(":/pc1360/pc1360.png");
    LcdFname		= P_RES(":/pc1360/1360lcd.png");
    SymbFname		= P_RES(":/pc1360/1360symb.png");
    memsize			= 0x40000;
//    InitMemValue    = 0xff;

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	P_RES(":/pc1360/cpu-1360.rom")	, "pc-1360/cpu-1360.rom", CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(8 , 0x2000 ,	""									, "pc-1360/R1-1360.ram"	, CSlot::RAM , "SYSTEM RAM"));
    SlotList.append(CSlot(16, 0x4000 ,	P_RES(":/pc1360/B0-1360.rom")	, "pc-1360/B0-1360.rom" , CSlot::ROM , "CURRENT ROM BANK"));
    SlotList.append(CSlot(32, 0x8000 ,	""									, "pc-1360/R2-1360.ram"	, CSlot::RAM , "CURRENT RAM SLOT"));
    SlotList.append(CSlot(16, 0x10000 ,	P_RES(":/pc1360/B0-1360.rom")	, "pc-1360/B0-1360.rom" , CSlot::ROM , "ROM BANK 0"));
    SlotList.append(CSlot(16, 0x14000 ,	P_RES(":/pc1360/B1-1360.rom")	, "pc-1360/B1-1360.rom" , CSlot::ROM , "ROM BANK 1"));
    SlotList.append(CSlot(16, 0x18000 ,	P_RES(":/pc1360/B2-1360.rom")	, "pc-1360/B2-1360.rom" , CSlot::ROM , "ROM BANK 2"));
    SlotList.append(CSlot(16, 0x1C000 ,	P_RES(":/pc1360/B3-1360.rom")	, "pc-1360/B3-1360.rom" , CSlot::ROM , "ROM BANK 3"));
    SlotList.append(CSlot(16, 0x20000 ,	P_RES(":/pc1360/B4-1360.rom")	, "pc-1360/B4-1360.rom" , CSlot::ROM , "ROM BANK 4"));
    SlotList.append(CSlot(16, 0x24000 ,	P_RES(":/pc1360/B5-1360.rom")	, "pc-1360/B5-1360.rom" , CSlot::ROM , "ROM BANK 5"));
    SlotList.append(CSlot(16, 0x28000 ,	P_RES(":/pc1360/B6-1360.rom")	, "pc-1360/B6-1360.rom" , CSlot::ROM , "ROM BANK 6"));
    SlotList.append(CSlot(16, 0x2C000 ,	P_RES(":/pc1360/B7-1360.rom")	, "pc-1360/B7-1360.rom" , CSlot::ROM , "ROM BANK 7"));
    SlotList.append(CSlot(32, 0x30000 ,	""									, "pc-1360/B0-1360.ram" , CSlot::RAM , "RAM SLOT 1"));
    SlotList.append(CSlot(32, 0x38000 ,	""									, "pc-1360/B1-1360.ram" , CSlot::RAM , "RAM SLOT 2"));

    KeyMap = KeyMap1360;
    KeyMapLenght = KeyMap1360Lenght;

    pLCDC		= new Clcdc_pc1360(this);
    pKEYB		= new Ckeyb(this,"pc1360.map",scandef_pc1360);
    pCPU		= new CSC61860(this);
    pTIMER		= new Ctimer(this);


}


void	Cpc1360::initExtension(void)
{

	// initialise ext_MemSlot1
	ext_MemSlot1 = new CExtensionArray("Memory Slot 1","Add memory credit card");
    ext_MemSlot1->setAvailable(ID_CE210M,true);		ext_MemSlot1->setChecked(ID_CE2H32M,true);
	ext_MemSlot1->setAvailable(ID_CE211M,true);
	ext_MemSlot1->setAvailable(ID_CE212M,true);
	ext_MemSlot1->setAvailable(ID_CE2H16M,true);
	ext_MemSlot1->setAvailable(ID_CE2H32M,true);
	ext_MemSlot1->setAvailable(ID_CE2H64M,true);

	// initialise ext_MemSlot2
	ext_MemSlot2 = new CExtensionArray("Memory Slot 2","Add memory credit card");
	ext_MemSlot2->setAvailable(ID_CE210M,true);		ext_MemSlot2->setChecked(ID_CE2H32M,false);
	ext_MemSlot2->setAvailable(ID_CE211M,true);
	ext_MemSlot2->setAvailable(ID_CE212M,true);
	ext_MemSlot2->setAvailable(ID_CE2H16M,true);
	ext_MemSlot2->setAvailable(ID_CE2H32M,true);
	ext_MemSlot2->setAvailable(ID_CE2H64M,true);
	
	addExtMenu(ext_MemSlot1);
	addExtMenu(ext_MemSlot2);

    extensionArray[0] = ext_11pins;
    extensionArray[1] = ext_MemSlot1;
    extensionArray[2] = ext_MemSlot2;
    extensionArray[3] = ext_Serial;
	
}

BYTE	Cpc1360::Get_PortA(void)
{
    BYTE data = Cpc13XX::Get_PortA();
    data |= ( pSIOCONNECTOR->Get_pin(12) ? 0x80 : 0x00);
    return (data);
}

bool Cpc1360::run(void)
{
    pSIOCONNECTOR_value = pSIOCONNECTOR->Get_values();

    CpcXXXX::run();

    return true;
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
//#define GET_PORT_BIT(Port,Bit) ((Get_Port(Port) & (1<<((Bit)-1))) ? 1:0)
#define SIO_GNDP	1
#define SIO_SD 		2
#define SIO_RD		3
#define SIO_RS		4
#define SIO_CS		5

#define SIO_GND		7
#define SIO_CD		8
#define SIO_VC1		10
#define SIO_RR		11
#define SIO_PAK		12
#define SIO_VC2		13
#define SIO_ER		14
#define SIO_PRQ		15

bool Cpc1360::Set_Connector(void)
{
	int port1 = Get_8(0x3800);
	int port2 = Get_8(0x3A00);
	
	pCONNECTOR->Set_pin(PIN_MT_OUT2	,0);
	pCONNECTOR->Set_pin(PIN_VGG		,1);
	pCONNECTOR->Set_pin(PIN_BUSY	,READ_BIT(port1,0));
    if (!pCONNECTOR->Get_pin(PIN_ACK)) pCONNECTOR->Set_pin(PIN_D_OUT	,READ_BIT(port2,2));
    pCONNECTOR->Set_pin(PIN_MT_OUT1	,pCPU->Get_Xout());
    if (!pCONNECTOR->Get_pin(PIN_ACK)) pCONNECTOR->Set_pin(PIN_D_IN	,READ_BIT(port2,3));
    if (!pCONNECTOR->Get_pin(PIN_ACK)) pCONNECTOR->Set_pin(PIN_SEL2	,READ_BIT(port2,1));
    if (!pCONNECTOR->Get_pin(PIN_ACK)) pCONNECTOR->Set_pin(PIN_SEL1	,READ_BIT(port2,0));


	// MANAGE SERIAL CONNECTOR
	pSIOCONNECTOR->Set_pin(SIO_SD	,GET_PORT_BIT(PORT_F,2));
	pSIOCONNECTOR->Set_pin(SIO_RR	,READ_BIT(port1,2));
	pSIOCONNECTOR->Set_pin(SIO_RS	,READ_BIT(port1,3));
	pSIOCONNECTOR->Set_pin(SIO_ER	,READ_BIT(port1,1));

    pSIOCONNECTOR->Set_pin(SIO_PRQ	,READ_BIT(port2,2) | READ_BIT(port2,3));
	return(1);
}

bool Cpc1360::Get_Connector(void)
{
    Set_Port_Bit(PORT_B,1,pCONNECTOR->Get_pin(PIN_SEL1));	// DIN	:	IB1
    Set_Port_Bit(PORT_B,2,pCONNECTOR->Get_pin(PIN_SEL2));	// DIN	:	IB2
    Set_Port_Bit(PORT_B,3,pCONNECTOR->Get_pin(PIN_D_OUT));	// DIN	:	IB2
    Set_Port_Bit(PORT_B,4,pCONNECTOR->Get_pin(PIN_D_IN));	// DIN	:	IB8

	Set_Port_Bit(PORT_B,5,pCONNECTOR->Get_pin(PIN_ACK));	// ACK	:	IB7
	pCPU->Set_Xin(pCONNECTOR->Get_pin(PIN_MT_IN));


	// MANAGE SERIAL CONNECTOR
	Set_Port_Bit(PORT_B,6,pSIOCONNECTOR->Get_pin(SIO_RD));
	Set_Port_Bit(PORT_B,7,pSIOCONNECTOR->Get_pin(SIO_CS));
	Set_Port_Bit(PORT_B,8,pSIOCONNECTOR->Get_pin(SIO_CD));

    // Set A8 if and only if PAK is Hight
    //if (pSIOCONNECTOR->Get_pin(SIO_PAK)) Set_Port_Bit(PORT_A,8,pSIOCONNECTOR->Get_pin(SIO_PAK));
	// PAK

	return(1);
}

BYTE	Cpc1360::Get_PortB(void)
{
	return (IO_B);
}

void	Cpc1360::Set_PortF(BYTE data)
{
	IO_F = data;
	RamBank = GET_PORT_BIT(PORT_F,3);
//    qWarning()<<"RAMBANK F="<<RamBank;
}
void	Cpc1360::Set_PortC(BYTE data)
{
	IO_C = data;
}

bool Cpc1360::Chk_Adr(UINT32 *d,UINT32 data)
{
    if (                 (*d<=0x1FFF) )	{ return(0); }	// ROM area(0000-1fff)
	if ( (*d>=0x2000) && (*d<=0x27FF) )	{ return(0); }
	if ( (*d>=0x2800) && (*d<=0x33FF) ) { pLCDC->SetDirtyBuf(*d-0x2800); return(1);			/* LCDC (0200x) */	}
	if ( (*d>=0x3400) && (*d<=0x35FF) )	{ RomBank = data &0x07;	return(1); }
	if ( (*d>=0x3E00) && (*d<=0x3FFF) )
	{
        BYTE KStrobe=0;

		switch (data & 0x0F)
		{
            case 0x00: KStrobe=0x00;	break;
            case 0x01: KStrobe=0x01;	break;
            case 0x02: KStrobe=0x02;	break;
            case 0x03: KStrobe=0x04;	break;
            case 0x04: KStrobe=0x08;	break;
            case 0x05: KStrobe=0x10;	break;
            case 0x06: KStrobe=0x20;	break;
            case 0x07: KStrobe=0x40;	break;
            case 0x08: KStrobe=0x80;	break;
            case 0x09: KStrobe=0x00;	break;
            case 0x0A: KStrobe=0x00;	break;
            case 0x0B: KStrobe=0x00;	break;
            case 0x0C: KStrobe=0x00;	break;
            case 0x0D: KStrobe=0x00;	break;
            case 0x0E: KStrobe=0x00;	break;
            case 0x0F: KStrobe=0x7F;	break;
		}

        pKEYB->Set_KS( KStrobe );

		return(1);
	}

#if 0
	if ( (*d>=0x3C00) && (*d<=0x3DFF) )
	{
		AddLog(LOG_MASTER,tr("Write Slot Register %1=%2").arg(*d).arg(mem[*d]));	
		RamBank = (data == 0x04 ? 0 : 1);
		return(1);
	}
#endif

	if ( (*d>=0x2800) && (*d<=0x3FFF) )	return(1);
	if ( (*d>=0x4000) && (*d<=0x7FFF) )	{ *d += 0xC000 + ( RomBank * 0x4000 );	return(0); }

	if ( (*d>=0x8000) && (*d<=0xFFFF) )
	{
		*d += 0x28000 + (RamBank * 0x8000);
		if ( (*d>=0x30000) && (*d<=0x33FFF) )	return(S1_EXTENSION_CE2H32M_CHECK);
		if ( (*d>=0x34000) && (*d<=0x35FFF) )	return(S1_EXTENSION_CE2H32M_CHECK | S1_EXTENSION_CE2H16M_CHECK);
		if ( (*d>=0x36000) && (*d<=0x36FFF) )	return(S1_EXTENSION_CE2H32M_CHECK | S1_EXTENSION_CE2H16M_CHECK | S1_EXTENSION_CE212M_CHECK);
		if ( (*d>=0x37000) && (*d<=0x377FF) )	return(S1_EXTENSION_CE2H32M_CHECK | S1_EXTENSION_CE2H16M_CHECK | S1_EXTENSION_CE212M_CHECK | S1_EXTENSION_CE211M_CHECK);
		if ( (*d>=0x37800) && (*d<=0x37FFF) )	return(S1_EXTENSION_CE2H32M_CHECK | S1_EXTENSION_CE2H16M_CHECK | S1_EXTENSION_CE212M_CHECK | S1_EXTENSION_CE211M_CHECK | S1_EXTENSION_CE210M_CHECK);

		if ( (*d>=0x38000) && (*d<=0x3BFFF) )	return(S2_EXTENSION_CE2H32M_CHECK);
		if ( (*d>=0x3C000) && (*d<=0x3DFFF) )	return(S2_EXTENSION_CE2H32M_CHECK | S2_EXTENSION_CE2H16M_CHECK);
		if ( (*d>=0x3E000) && (*d<=0x3EFFF) )	return(S2_EXTENSION_CE2H32M_CHECK | S2_EXTENSION_CE2H16M_CHECK | S2_EXTENSION_CE212M_CHECK);
		if ( (*d>=0x3F000) && (*d<=0x3F7FF) )	return(S2_EXTENSION_CE2H32M_CHECK | S2_EXTENSION_CE2H16M_CHECK | S2_EXTENSION_CE212M_CHECK | S2_EXTENSION_CE211M_CHECK);
		if ( (*d>=0x3F800) && (*d<=0x3FFFF) )	return(S2_EXTENSION_CE2H32M_CHECK | S2_EXTENSION_CE2H16M_CHECK | S2_EXTENSION_CE212M_CHECK | S2_EXTENSION_CE211M_CHECK | S2_EXTENSION_CE210M_CHECK);
		
	}

	return(0);
}


bool Cpc1360::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
    Q_UNUSED(data)

	// Manage ROM Bank
	if ( (*d>=0x4000) && (*d<=0x7FFF) )	{ *d += 0xC000 + ( RomBank * 0x4000 ); return (1); }
	// Manage ram bank
	if ( (*d>=0x8000) && (*d<=0xFFFF) )	{ *d += 0x28000 + ( RamBank * 0x8000 ); return (1);}
	

	return(1);
}


