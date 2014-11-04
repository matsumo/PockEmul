#include "pc1253.h"
#include "Keyb.h"
#include "Lcdc_pc1250.h"
#include "Lcdc_symb.h"

Cpc1253::Cpc1253(CPObject *parent)	: Cpc1251(parent)
{								//[constructor]
    Q_UNUSED(parent)

    setcfgfname("pc1253");

    SessionHeader	= "PC1253PKM";
    Initial_Session_Fname ="pc1253.pkm";

    BackGroundFname	= P_RES(":/pc1251/pc1253.png");

    memsize			= 0x30000;
    SlotList.clear();
    SlotList.append(CSlot(8	, 0x0000 ,	P_RES(":/pc1251/sc61860A04.rom")  , "", CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(16, 0x4000 ,	P_RES(":/pc1251/bas-1253.rom")  , "", CSlot::ROM , "BASIC ROM"));
    SlotList.append(CSlot(32, 0x8000 ,	""                              , "" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(128,0x10000 ,	""                              , "" , CSlot::RAM , "RAM"));

    delete pKEYB;
    pKEYB		= new Ckeyb(this,"pc1253.map");

    pLCDC->symbList.clear();
     pLCDC->symbList
              << ClcdSymb(0,  0, S_BUSY,  0xF83D, 0x01) // BUSY
              << ClcdSymb(30, 0, S_PRINT, 0xF83C, 0x02)	// P             << ClcdSymb(48, 0, S_DEF,   0xF83C, 0x01)	// DEF
              << ClcdSymb(158,0, S_DE,    0xF83C, 0x08)	// DE
              << ClcdSymb(166,0, S_G,     0xF83C, 0x04)	// G
              << ClcdSymb(171,0, S_RAD,   0xF83D, 0x04)	// RAD
              << ClcdSymb(250,0, S_SHIFT, 0xF83D, 0x02)	// SHIFT
              << ClcdSymb(290,0, S_E,     0xF83D, 0x08)	// E
              << ClcdSymb(300,0, S_REV_M, 0xF83D, 0x10)	// M
              ;
}

bool Cpc1253::Chk_Adr(UINT32 *d,UINT32 data)
{
//	Cpc1250::Mem_Mirror(d);
if ( (*d>=0x8000) && (*d<=0xC7FF) )	return(1);

    if ( (*d>=0xB000) && (*d<=0xC7FF) )	{
        if (RomBank>0) {
            *d -= 0xB000;
            *d += 0x10000 + RomBank * 0x2000;
        }
        return(1);			// RAM area(C800-C7ff)
    }

    return (Cpc1250::Chk_Adr(d,data));
}

bool Cpc1253::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
    if ( (*d>=0x8000) && (*d<=0xC7FF) )	return(1);

    if ( (*d>=0xB000) && (*d<=0xC7FF) )	{
        if (RomBank>0) {
            *d -= 0xB800;
            *d += 0x10000 + RomBank * 0x2000;
        }
        return true;
    }
    return(Cpc1250::Chk_Adr_R(d,data));
}

BYTE	Cpc1253::Get_PortA(void)
{
    int data = 0;


    if (IO_B & 0x1) {
        if (KEY('R'))			data|=0x01;
        if (KEY(K_IN))			data|=0x02;
        if (KEY('J'))			data|=0x04;   // FUNC
        if (KEY('A'))			data|=0x08; // FUNC
        if (KEY(K_CM))			data|=0x10;
        if (KEY(K_RM))			data|=0x20;
        if (KEY(K_MPLUS))		data|=0x40;
        if (KEY(K_MIN))			data|=0x80;
    }
    if (IO_B & 2) {
        if (KEY('M'))			data|=0x04; // FUNC
        if (KEY('7'))			data|=0x08;
    }
    if (IO_A & 1) {
        if (KEY(K_OUT))			data|=0x02;
        if (KEY('K'))			data|=0x04; // FUNC
        if (KEY('B'))			data|=0x08; // FUNC
        if (KEY(K_CLR))			data|=0x10;
        if (KEY(K_CE))			data|=0x20;
        if (KEY('='))			data|=0x40;
        if (KEY(K_SQR))			data|=0x80;
    }
    if (IO_A & 2) {
        if (KEY('L'))			data|=0x04; // FUNC
        if (KEY('C'))			data|=0x08; // FUNC
        if (KEY('/'))			data|=0x10;
        if (KEY('*'))			data|=0x20;
        if (KEY('+'))			data|=0x40;
        if (KEY('-'))			data|=0x80;
    }
    if (IO_A & 4) {
        if (KEY('D'))			data|=0x08;     // FUNC
        if (KEY('9'))			data|=0x10;
        if (KEY('6'))			data|=0x20;
        if (KEY('%'))			data|=0x40;
        if (KEY('3'))			data|=0x80;
    }
    if (IO_A & 8) {
        if (KEY('8'))			data|=0x10;
        if (KEY('5'))			data|=0x20;
        if (KEY('.'))			data|=0x40;
        if (KEY('2'))			data|=0x80;
    }
    if (IO_A & 0x10) {
        if (KEY('4'))			data|=0x20;
        if (KEY('0'))			data|=0x40;
        if (KEY('1'))			data|=0x80;
    }
    if (IO_A & 0x20) {
        if (KEY(K_RET))			data|=0x40;
        if (KEY('N'))			data|=0x80;
    }
    if (IO_A & 0x40) {
        if (KEY('O'))			data|=0x80;
    }

    return data;
}

//TODO: draw the Power switch (no mode switch)
bool Cpc1253::UpdateFinalImage(void)
{
    Cpc1251::UpdateFinalImage();
}
