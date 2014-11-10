//TODO  resize screen to avoid aliasing
#include <QDebug>

#include "common.h"
#include "Log.h"
#include "Connect.h"
#include "Keyb.h"

#include "pc1475.h"
#include "cextension.h"
#include "Lcdc_pc1475.h"
#include "cpu.h"

extern TransMap KeyMap1450[];
extern int KeyMap1450Lenght;
extern BYTE scandef_pc1450[];

Cpc1475::Cpc1475(CPObject *parent)	: Cpc1360(parent)
{								//[constructor]
    setfrequency(768000/3);
    setcfgfname("pc1475");

    SessionHeader	= "PC1475PKM";
    Initial_Session_Fname ="pc1475.pkm";

    BackGroundFname	= P_RES(":/pc1475/pc1475.png");
    memsize			= 0x40000;

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	P_RES(":/pc1475/cpu-1475.rom")	, "" ,CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(8 , 0x2000 ,	""								, "" , CSlot::RAM , "SYSTEM RAM"));
    SlotList.append(CSlot(16, 0x4000 ,	P_RES(":/pc1475/b0-1475.rom")	, "" , CSlot::ROM , "ROM"));
    SlotList.append(CSlot(32, 0x8000 ,	""								, "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x10000 ,	P_RES(":/pc1475/b0-1475.rom")	, "" , CSlot::ROM , "BANK 0"));
    SlotList.append(CSlot(16, 0x14000 ,	P_RES(":/pc1475/b1-1475.rom")	, "" , CSlot::ROM , "BANK 1"));
    SlotList.append(CSlot(16, 0x18000 ,	P_RES(":/pc1475/b2-1475.rom")	, "" , CSlot::ROM , "BANK 2"));
    SlotList.append(CSlot(16, 0x1C000 ,	P_RES(":/pc1475/b3-1475.rom")	, "" , CSlot::ROM , "BANK 3"));
    SlotList.append(CSlot(16, 0x20000 ,	P_RES(":/pc1475/b4-1475.rom")	, "" , CSlot::ROM , "BANK 4"));
    SlotList.append(CSlot(16, 0x24000 ,	P_RES(":/pc1475/b5-1475.rom")	, "" , CSlot::ROM , "BANK 5"));
    SlotList.append(CSlot(16, 0x28000 ,	P_RES(":/pc1475/b6-1475.rom")	, "" , CSlot::ROM , "BANK 6"));
    SlotList.append(CSlot(16, 0x2C000 ,	P_RES(":/pc1475/b7-1475.rom")	, "" , CSlot::ROM , "BANK 7"));
    SlotList.append(CSlot(32, 0x30000 ,	""								, "" , CSlot::RAM , "RAM S1"));
    SlotList.append(CSlot(32, 0x38000 ,	""								, "" , CSlot::RAM , "RAM S2"));

    KeyMap = KeyMap1450;
    KeyMapLenght = KeyMap1450Lenght;

    delete pLCDC;		pLCDC		= new Clcdc_pc1475(this,
                                                       QRect(111,48,264*.9,30),
                                                       QRect(111,38,264*.9,45));
    delete pKEYB;		pKEYB		= new Ckeyb(this,"pc1475.map",scandef_pc1450);

}

bool Cpc1475::Chk_Adr(UINT32 *d,UINT32 data)
{

    if (                 (*d<=0x1FFF) )	return(0);	// ROM area(0000-1fff)
    if ( (*d>=0x2000) && (*d<=0x27FF) )	return(0);

    if ( (*d>=0x2800) && (*d<=0x2B7B) ) { pLCDC->SetDirtyBuf(*d-0x2800);pLCDC->updated = true;return(1);			/* LCDC (0200x) */	}
	if ( (*d>=0x2800) && (*d<=0x33FF) ) return(1);
    if ( (*d>=0x3400) && (*d<=0x35FF) )	{ RomBank = data &0x07; return(1); }
    if ( (*d>=0x3C00) && (*d<=0x3DFF) )
    {
//		AddLog(LOG_MASTER,"Write Slot Register %04X=%02X",*d,mem[*d]);

//        RamBank = (data == 0x04 ? 0 : 1);
//        qWarning()<<"Rambank:"<<RamBank<< " ("<<data<<")";
        return(1);
    }
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

    if ( (*d>=0x2800) && (*d<=0x3FFF) )	return(1);

    if ( (*d>=0x4000) && (*d<=0x7FFF) )	{ *d += 0xC000 + ( RomBank * 0x4000 );	return(0); }


	if ( (*d>=0x8000) && (*d<=0xFFFF) )
    {
        UINT32 _addr = *d &0x7FFF;
        //        qWarning()<<(RamBank ? "S2:" : "S1:");
        writeBus(RamBank ? busS2 : busS1 ,&_addr,data);
	}

    return(0);
}

bool Cpc1475::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
    Q_UNUSED(data)

    if ( (*d>=0x4000) && (*d<=0x7FFF) )	{ *d += 0xC000 + ( RomBank * 0x4000 ); return(1);}	// Manage ROM Bank
    if ( (*d>=0x8000) && (*d<=0xFFFF) )	{
        UINT32 _addr = *d &0x7FFF;
//        qWarning()<<(RamBank ? "S2:" : "S1:");
        readBus(RamBank ? busS2 : busS1 ,&_addr,data);
        return false;
    }
	return(1); 
}


