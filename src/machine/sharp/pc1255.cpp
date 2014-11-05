#include	"common.h"

#include "pc1255.h"
#include "Lcdc_pc1250.h"

Cpc1255::Cpc1255(CPObject *parent)	: Cpc1250(parent)
{								//[constructor]
    setcfgfname("pc1255");

    SessionHeader	= "PC1255PKM";
    Initial_Session_Fname ="pc1255.pkm";

    BackGroundFname	= P_RES(":/pc1255/pc1255.png");



    memsize			= 0x10000;
    SlotList.clear();
    SlotList.append(CSlot(8	, 0x0000 ,	P_RES(":/pc1255/cpu-1255.rom")	, "pc1255/cpu-1255.rom"	, CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(8 , 0x2000 ,	""									, "pc1255/R1-1255.ram"	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x4000 ,	P_RES(":/pc1255/bas-1255.rom")	, "pc1255/bas-1255.rom"	, CSlot::ROM , "BASIC ROM"));
    SlotList.append(CSlot(32, 0x8000 ,	""									, "pc1255/R2-1255.ram" 	, CSlot::RAM , "RAM"));

}

bool Cpc1255::Chk_Adr(UINT32 *d,UINT32 data)
{
	Cpc1250::Mem_Mirror(d);
	
	if ( (*d>=0xA000) && (*d<=0xC7FF) )	return(1);			// RAM area(C800-C7ff) 

	return (Cpc1250::Chk_Adr(d,data));
}

Cpc1251H::Cpc1251H(CPObject *parent)	: Cpc1250(parent)
{								//[constructor]
    Q_UNUSED(parent)

    setcfgfname("pc1251H");

    SessionHeader	= "PC1251HPKM";
    Initial_Session_Fname ="pc1251h.pkm";

    BackGroundFname	= ":/pc1251/pc1251h.png";
}

bool Cpc1251H::Chk_Adr(UINT32 *d,UINT32 data)
{

    if ( (*d>=0x8000) && (*d<=0xC7FF) )	return(1);			// RAM area(C800-C7ff)

    return (Cpc1250::Chk_Adr(d,data));
}
