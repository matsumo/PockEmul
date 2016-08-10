#include <QPainter>

#include "common.h"

#include "pc1245.h"
#include "Lcdc_pc1250.h"
#include "Keyb.h"

Cpc1245::Cpc1245(CPObject *parent)	: Cpc1250(parent)
{								//[constructor]
	setcfgfname("pc1245");

    SessionHeader	= "PC1245PKM";
	Initial_Session_Fname ="pc1245.pkm";
	
    BackGroundFname	= P_RES(":/pc1245/pc1245.png");

	memsize			= 0x10000;

	SlotList.clear();
    SlotList.append(CSlot(8	, 0x0000 ,	P_RES(":/pc1245/cpu-1245.rom")	, "pc1245/cpu-1245.rom"	, CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(8 , 0x2000 ,	""								, "pc1245/R1-1245.ram"	, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x4000 ,	P_RES(":/pc1245/bas-1245.rom")	, "pc1245/bas-1245.rom"	, CSlot::ROM , "BASIC ROM"));
    SlotList.append(CSlot(32, 0x8000 ,	""								, "pc1245/R2-1245.ram" 	, CSlot::RAM , "RAM"));

    delete pLCDC;	pLCDC		= new Clcdc_pc1245(this,
                                                   QRect(50,56,192,16),
                                                   QRect(50,45,173,5));
    pKEYB->setMap("pc1260.map");

}

bool Cpc1245::UpdateFinalImage(void)
{
    CpcXXXX::UpdateFinalImage();

    // Draw
    QPainter painter;
    painter.begin(FinalImage);

    QPoint ptPower(448*internalImageRatio,32*internalImageRatio);

    switch (PowerSwitch)
    {
    case PS_RUN :	painter.drawImage(ptPower,iPowerRUN.scaled(iPowerRUN.size()*internalImageRatio)); break;
    case PS_PRO :	painter.drawImage(ptPower,iPowerPRO.scaled(iPowerPRO.size()*internalImageRatio)); break;
    case PS_OFF :	painter.drawImage(ptPower,iPowerOFF.scaled(iPowerOFF.size()*internalImageRatio)); break;
    }

    painter.end();

    return true;
}

bool Cpc1245::InitDisplay(void)
{

    CpcXXXX::InitDisplay();

    iPowerOFF.load( P_RES(":/pc1245/powerOFF.png"));
    iPowerRUN.load( P_RES(":/pc1245/powerRUN.png"));
    iPowerPRO.load( P_RES(":/pc1245/powerPRO.png"));

    return(1);
}

BYTE	Cpc1245::Get_PortB(void)
{
	int data=IO_B & 0x0f;
	int retval = 0;

	if (KEY(K_MOD) && ( previous_key != K_MOD) )
	{
		switch (PowerSwitch)
		{
		case PS_RUN : PowerSwitch = PS_PRO; break;
		case PS_PRO : PowerSwitch = PS_RUN; break;
		}
	}
    previous_key = pKEYB->LastKey;

	if (IO_B&8)
	{
		switch (PowerSwitch)
		{
		case PS_PRO : retval|=2; break;
		}
	}

	if ( (IO_B&2) && (PowerSwitch==PS_PRO) ) retval|=8;


	int ret = (retval & ~data) | (IO_B & 0xf0);
	return (ret);
}

Cmc2200::Cmc2200(CPObject *parent)	: Cpc1245(parent)
{								//[constructor]
    setcfgfname("mc2200");

    SessionHeader	= "MC2200PKM";
    Initial_Session_Fname ="mc2200.pkm";

    BackGroundFname	= P_RES(":/pc1245/mc2200.png");

    pLCDC->rect.moveTo(125,54);
    pLCDC->symbRect.moveTo(125,40);
}
