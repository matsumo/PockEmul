#include	"common.h"

#include "pc1251.h"
#include "Lcdc.h"
#include "Keyb.h"
#include "Lcdc_symb.h"

extern BYTE scandef_pc1253[];

Cpc1251::Cpc1251(CPObject *parent)	: Cpc1250(parent)
{								//[constructor]
    Q_UNUSED(parent)

    setcfgfname("pc1251");

    SessionHeader	= "PC1251PKM";
    Initial_Session_Fname ="pc1251.pkm";

    BackGroundFname	= P_RES(":/pc1251/pc1251.png");

    BackFname = P_RES(":/pc1251/PC1251Back.png");

    memsize			= 0x10000;

    SlotList.clear();
    SlotList.append(CSlot(8	, 0x0000 ,	P_RES(":/pc1251/cpu-1251.rom"), "", CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(8 , 0x2000 ,	""						, ""	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x4000 ,	P_RES(":/pc1251/bas-1251.rom"), "", CSlot::ROM , "BASIC ROM"));
    SlotList.append(CSlot(32, 0x8000 ,	""						, "" , CSlot::RAM , "RAM"));

}

bool Cpc1251::Chk_Adr(UINT32 *d,UINT32 data)
{
//    if ( (*d>=0xA000) && (*d<=0xC7FF) )	return(1);  // 18KB RAM
	if ( (*d>=0xB000) && (*d<=0xB7FF) )	{ *d+=0x800;}
	if ( (*d>=0xB800) && (*d<=0xC7FF) )	return(1);		// RAM area(B800-C7ff) 

	return (Cpc1250::Chk_Adr(d,data));
}

bool Cpc1251::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
//    if ( (*d>=0xA000) && (*d<=0xC7FF) )	return(1);  // 18KB RAM

	if ( (*d>=0xB000) && (*d<=0xB7FF) )	{ *d+=0x800;return(1);	}
	return(Cpc1250::Chk_Adr_R(d,data));
}


