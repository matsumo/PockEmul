#include	"common.h"

#include "pc1251.h"
#include "Lcdc.h"

Cpc1251::Cpc1251(CPObject *parent)	: Cpc1250(this)
{								//[constructor]
    Q_UNUSED(parent)

    setcfgfname("pc1251");

    SessionHeader	= "PC1251PKM";
    Initial_Session_Fname ="pc1251.pkm";

    BackGroundFname	= P_RES(":/pc1251/pc1251.png"); //":/pc1251/pc1251hd.jpg"; //
//    pLCDC->LcdFname		= P_RES(":/pc1251/1251lcd.png");
//    pLCDC->SymbFname		= P_RES(":/pc1251/1251symb.png");


    memsize			= 0x10000;
//		NbSlot		= 4;

    SlotList.clear();
    SlotList.append(CSlot(8	, 0x0000 ,	P_RES(":/pc1251/cpu-1251.rom"), "pc-1251/cpu-1251.rom", CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(8 , 0x2000 ,	""						, "pc-1251/R1-1251.ram"	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x4000 ,	P_RES(":/pc1251/bas-1251.rom"), "pc-1251/bas-1251.rom", CSlot::ROM , "BASIC ROM"));
    SlotList.append(CSlot(32, 0x8000 ,	""						, "pc-1251/R2-1251.ram" , CSlot::RAM , "RAM"));

}
Cpc1253::Cpc1253(CPObject *parent)	: Cpc1251(this)
{								//[constructor]
    Q_UNUSED(parent)

    setcfgfname("pc1253");

    SessionHeader	= "PC1253PKM";
    Initial_Session_Fname ="pc1253.pkm";

    BackGroundFname	= P_RES(":/pc1251/pc1253.png");

    SlotList.clear();
    SlotList.append(CSlot(8	, 0x0000 ,	P_RES(":/pc1251/cpu-1251.rom"), "", CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(16, 0x4000 ,	P_RES(":/pc1251/bas-1253.rom"), "", CSlot::ROM , "BASIC ROM"));
    SlotList.append(CSlot(32, 0x8000 ,	""						, "" , CSlot::RAM , "RAM"));

}

bool Cpc1251::Chk_Adr(UINT32 *d,UINT32 data)
{

	if ( (*d>=0xB000) && (*d<=0xB7FF) )	{ *d+=0x800;}
	if ( (*d>=0xB800) && (*d<=0xC7FF) )	return(1);		// RAM area(B800-C7ff) 

	return (Cpc1250::Chk_Adr(d,data));
}

bool Cpc1251::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
	if ( (*d>=0xB000) && (*d<=0xB7FF) )	{ *d+=0x800;return(1);	}
	return(Cpc1250::Chk_Adr_R(d,data));
}

