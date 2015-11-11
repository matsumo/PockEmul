#include "pc1425.h"
#include "Lcdc_pc1403.h"
#include "Log.h"

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
    pKEYB->fn_KeyMap = "pc1425.map";


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



