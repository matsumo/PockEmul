/*** POCKEMUL [PC1450.CPP] ********************************/
/* PC1450 emulator main class                             */
/**********************************************************/

#include	<stdlib.h>

#include	"common.h"
#include	"pc1450.h"
#include "cextension.h"
#include	"Keyb.h"
#include    "Keyb1450.h"
#include "Lcdc_pc1450.h"



Cpc1450::Cpc1450(CPObject *parent)	: Cpc1350(parent)
{								//[constructor]
    setfrequency(768000/3);
    setcfgfname("pc1450");

    SessionHeader	= "PC1450PKM";
    Initial_Session_Fname ="pc1450.pkm";

    BackGroundFname	= P_RES(":/pc1450/pc1450.png");
//    LcdFname		= P_RES(":/pc1450/1450lcd.png");
//    SymbFname		= P_RES(":/pc1450/1450symb.png");
    memsize			= 0x10000;

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	P_RES(":/pc1450/cpu-1450.rom"), "pc-1450/cpu-1450.rom" , CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(24, 0x2000 ,	""						, "pc-1450/R1-1450.ram" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(32, 0x8000 ,	P_RES(":/pc1450/bas-1450.rom"), "pc-1450/bas-1450.rom" , CSlot::ROM , "BASIC ROM"));

    KeyMap = KeyMap1450;
    KeyMapLenght = KeyMap1450Lenght;

    delete pLCDC;   pLCDC = new Clcdc_pc1450(this,
                                             QRect(130,53,192,20),
                                             QRect(130,44,196,35),
                                             P_RES(":/pc1450/1450lcd.png"),
                                             P_RES(":/pc1450/1450symb.png"));
    delete pKEYB;   pKEYB = new Ckeyb(this,"pc1450.map",scandef_pc1450);

//    Lcd_X		= 130;
//    Lcd_Y		= 53;
//    Lcd_DX		= 96;
//    Lcd_DY		= 10;
//    Lcd_ratio_X	= 2;
//    Lcd_ratio_Y	= 2;

//    Lcd_Symb_X	= 130;
//    Lcd_Symb_Y	= 44;
//    Lcd_Symb_DX	= 196;
//    Lcd_Symb_DY	= 35;

    //initExtension();
}

//FIXME: Extension

void	Cpc1450::initExtension(void)
{
    ext_MemSlot1 = new CExtensionArray("Memory Slot 1","Add memory credit card");
	// Add possible memory card to ext_MemSlot1
    ext_MemSlot1->setAvailable(ID_CE201M,true);ext_MemSlot1->setChecked(ID_CE202M,true);
    ext_MemSlot1->setAvailable(ID_CE202M,true);
    ext_MemSlot1->setAvailable(ID_CE203M,true);
    ext_MemSlot1->setAvailable(ID_CE210M,true);
	ext_MemSlot1->setAvailable(ID_CE211M,true);
	ext_MemSlot1->setAvailable(ID_CE212M,true);
	ext_MemSlot1->setAvailable(ID_CE2H16M,true);
	ext_MemSlot1->setAvailable(ID_CE2H32M,true);
//	ext_MemSlot1->setAvailable(ID_CE2H64M,true);

	addExtMenu(ext_MemSlot1);

    extensionArray[0] = ext_11pins;
    extensionArray[1] = ext_MemSlot1;
    extensionArray[2] = ext_Serial;
}

BYTE	Cpc1450::Get_PortA(void)
{
    BYTE data = pKEYB->Read(IO_A);

//	if (! Japan) PUT_BIT(IO_A,7,IO_F & 0x01);

    return (data);
}



/*****************************************************************************/
/* Check Address ROM or RAM ?												 */
/*  ENTRY :DWORD d=Address													 */
/*  RETURN:BOOL (1=RAM,0=ROM)												 */
/*****************************************************************************/
// Virtual Fonction


bool Cpc1450::Chk_Adr(UINT32 *d,UINT32 data)
{
    if (                 (*d<=0x1FFF) ) return(0);			// ROM area(0000-1fff)

    if ( (*d>=0x2000) && (*d<=0x3FFF) && (EXTENSION_CE201M_CHECK | EXTENSION_CE212M_CHECK) ) {
				*d+=0x2000;
				return(1);}	// 8Kb Ram Card Image
	if ( (*d>=0x2000) && (*d<=0x3FFF) )	{ 
				return(EXTENSION_CE202M_CHECK | EXTENSION_CE203M_CHECK); }						// ROM area(0000-3FFF) 16K
	if ( (*d>=0x4000) && (*d<=0x5FFF) )	{ 
				return( EXTENSION_CE201M_CHECK | EXTENSION_CE202M_CHECK | EXTENSION_CE203M_CHECK); }	// ROM area(0000-3FFF) 16K
    if ( (*d>=0x7000) && (*d<=0x708F) ) {pLCDC->SetDirtyBuf(*d-0x7000);return(1);	}
    if ( (*d>=0x7E00) && (*d<=0x7FFF) )	{pKEYB->Set_KS( (BYTE) data & 0x7F );return(1);}
	if ( (*d>=0x6000) && (*d<=0x7FFF) ) return(1);			// Internal RAM area(6000-8000)
	if ( (*d>=0x8000) && (*d<=0xFFFF) ) return(0);			// ROM area(8000-ffff) 

	return(0);
}

bool Cpc1450::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
    Q_UNUSED(d)
    Q_UNUSED(data)

    return(1);
}




