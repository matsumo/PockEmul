#include	"common.h"
 
#include "pc1260.h"
#include "Lcdc_pc1250.h"
#include "Keyb.h"

Cpc1260::Cpc1260(CPObject *parent)	: Cpc1250(parent)
{								//[constructor]
    Q_UNUSED(parent)

    setfrequency( (int) 768000/3);
    setcfgfname("pc1260");

    SessionHeader	= "PC1260PKM";
    Initial_Session_Fname ="pc1260.pkm";

    BackGroundFname	= P_RES(":/pc1260/pc1260.png");

    memsize			= 0x10000;

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	P_RES(":/pc1260/cpu-1260.rom")	, "" , CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(24, 0x2000 ,	""									, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(32, 0x8000 ,	P_RES(":/pc1260/bas-1260.rom")	, "" , CSlot::ROM , "BASIC ROM"));

    delete pLCDC;	pLCDC = new Clcdc_pc1260(this,
                                             QRect(79,37,312,32),
                                             QRect(53,37,288,33));
    pKEYB->fn_KeyMap = "pc1260.map";
}

BYTE	Cpc1260::Get_PortB(void)
{
	BYTE loc;
	loc = Cpc1250::Get_PortB();
	
	if (! Japan){
		if (IO_B & 4) loc = loc | 8;
	}
		
	return loc;
}

bool Cpc1260::Chk_Adr(UINT32 *d,UINT32 data)
{
    Q_UNUSED(data)

    if (                 (*d<=0x1FFF) )	return(0);				// ROM area(0000-1fff)
	if ( (*d>=0x5800) && (*d<=0x6800) )	return(1);				// ROM area(8000-ffff) 
	if ( (*d>=0x8000) && (*d<=0xFFFF) )	return(0);				// ROM area(8000-ffff) 
	if ( (*d>=0x2000) && (*d<=0x207D) )	{ pLCDC->SetDirtyBuf(*d-0x2000); return(1); }	/* LCDC (0200x) */
	if ( (*d>=0x2800) && (*d<=0x287D) )	{ pLCDC->SetDirtyBuf(*d-0x2000); return(1);	}	/* LCDC (0200x) */
	if ( (*d>=0x2000) && (*d<=0x20FF) )	return(1);				// ROM area(8000-ffff) 
	if ( (*d>=0x2800) && (*d<=0x28FF) )	return(1);				// ROM area(8000-ffff) 

	return(0);
}

Cpc1261::Cpc1261(CPObject *parent)	: Cpc1260(parent)
{								//[constructor]
    Q_UNUSED(parent)

    setfrequency( (int) 768000/3);
    setcfgfname("pc1261");

    SessionHeader	= "PC1261PKM";
    Initial_Session_Fname ="pc1261.pkm";

    BackGroundFname	= P_RES(":/pc1261/pc1261.png");

    memsize			= 0x10000;

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	P_RES(":/pc1261/cpu-1261.rom")	, "" , CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(24, 0x2000 ,	""									, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(32, 0x8000 ,	P_RES(":/pc1261/bas-1261.rom")	, "" , CSlot::ROM , "BASIC ROM"));

}
bool Cpc1261::Chk_Adr(UINT32 *d,UINT32 data)
{
	if ( (*d>=0x4000) && (*d<=0x6800) )	return(1);				// RAM area(4000-6800) 

	return(Cpc1260::Chk_Adr(d,data));
}

Cpc1262::Cpc1262(CPObject *parent)	: Cpc1261(parent)
{								//[constructor]
    Q_UNUSED(parent)

    setfrequency( (int) 768000/3);
    setcfgfname("pc1262");

    SessionHeader	= "PC1262PKM";
    Initial_Session_Fname ="pc1262.pkm";

    BackGroundFname	= P_RES(":/pc1262/pc1262.png");

    memsize			= 0x10000;

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	P_RES(":/pc1262/cpu-1262.rom")	, "" , CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(24, 0x2000 ,	""									, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(32, 0x8000 ,	P_RES(":/pc1262/bas-1262.rom")	, "" , CSlot::ROM , "BASIC ROM"));
}


