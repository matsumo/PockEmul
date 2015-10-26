
#include <QDebug>

#include "common.h"
#include "pc1360.h"
#include "cextension.h"
#include "Lcdc_pc1350.h"
#include "sc61860.h"
#include "Inter.h"
#include "Keyb.h"
#include "Keyb1360.h"
#include "Log.h"
#include "bus.h"
#include "fluidlauncher.h"
#include "renderView.h"
#include "clink.h"

#include "modelids.h"
#include "sharp/ce2xxx.h"

extern int ask(QWidget *parent,QString msg,int nbButton);

Cpc1360::Cpc1360(CPObject *parent)	: Cpc13XX(parent)
{								//[constructor]
    setcfgfname("pc1360");

    SessionHeader	= "PC1360PKM";
    Initial_Session_Fname ="pc1360.pkm";

    BackGroundFname	= P_RES(":/pc1360/pc1360.png");

    BackFname   = P_RES(":/pc1360/pc1360Back.png");


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

    pLCDC		= new Clcdc_pc1360(this,
                                    QRect(75,48,300,64),
                                    QRect(50,48,30,64));
    pKEYB		= new Ckeyb(this,"pc1360.map",scandef_pc1360);
    pCPU		= new CSC61860(this);
    pTIMER		= new Ctimer(this);

    busS2 = new Cbus("RAM Bus S2");
    backdoorS2Open = false;
    backdoorS1Open = false;
    backdoorFlipping = false;
    m_backdoorS1Angle = 0;
    m_backdoorS2Angle = 0;
}

Cpc1360::~Cpc1360()
{								//[constructor]
    delete busS2;
}

bool Cpc1360::init()
{
    Cpc13XX::init();

    pS2CONNECTOR = new Cconnector(this,35,3,Cconnector::Sharp_35,"Memory SLOT 2",true,QPoint(0,90));
    publish(pS2CONNECTOR);


    return true;
}

void Cpc1360::TurnON()
{
    // Check if there is a memory cartd connected to the PS1Connnector before turning ON
    CPObject * S1PC = pS1CONNECTOR->LinkedToObject();
    if (!S1PC){
        // If not, ask for connecting the default 8Ko card.
        int result = ask(this,"No memory card in SLOT 1. Do you want to add the default 8Ko RAM Card ?",2);
        if (result == 1) {
            // Create ram card PObject
            CPObject *cardPC = mainwindow->LoadPocket(CE212M);
            // link the memory card to PS1Connector
            // and move memory card to correct coordinates
            currentSlot = 1;
            linkObject("",cardPC);

            // Hide memory card
            cardPC->hideObject();
        }
    }

    Cpc13XX::TurnON();
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
extern CrenderView* view;
void Cpc1360::PreFlip(Direction dir, View targetView)
{
    Q_UNUSED(dir)
    Q_UNUSED(targetView)

    // hide memory cards
    animateBackDoorS1(false);
    animateBackDoorS2(false);
    manageCardVisibility();
}



void Cpc1360::manageCardVisibility() {
    Cpc13XX::manageCardVisibility();

    if (currentView == BACKview) {
        // show memory cards
        CPObject * S2PC = pS2CONNECTOR->LinkedToObject();
        if (S2PC) {
            if (backdoorS2Open) {
                S2PC->showObject();
            }
            else {
                S2PC->hideObject();
            }
        }
    }
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

bool Cpc1360::Set_Connector(Cbus *_bus)
{
    if (_bus == busS1) {
        pS1CONNECTOR->Set_values(busS1->toUInt64());
        return true;
    }
    if (_bus == busS2) {
        pS2CONNECTOR->Set_values(busS2->toUInt64());
        return true;
    }

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

bool Cpc1360::Get_Connector(Cbus *_bus)
{
    if (_bus == busS1) {
        busS1->fromUInt64(pS1CONNECTOR->Get_values());
        busS1->setEnable(false);
        return true;
    }
    if (_bus == busS2) {
        busS2->fromUInt64(pS2CONNECTOR->Get_values());
        busS2->setEnable(false);
        return true;
    }

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
//    if ( (*d>=0x2800) && (*d<=0x33FF) ) { pLCDC->SetDirtyBuf(*d-0x2800); return(1);			/* LCDC (0200x) */	}
    if ( ( (*d>=0x2800) && (*d<=0x287C) ) ||
         ( (*d>=0x2A00) && (*d<=0x2A7C) ) ||
         ( (*d>=0x2C00) && (*d<=0x2C7C) ) ||
         ( (*d>=0x2E00) && (*d<=0x2E7C) ) ||
         ( (*d>=0x3000) && (*d<=0x307C) ))	{
        if (mem[*d] != data) {
            pLCDC->updated = true;
            pLCDC->SetDirtyBuf(*d-0x2800);
        }
        return(1);
    }
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


bool Cpc1360::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
    Q_UNUSED(data)

	// Manage ROM Bank
	if ( (*d>=0x4000) && (*d<=0x7FFF) )	{ *d += 0xC000 + ( RomBank * 0x4000 ); return (1); }
	// Manage ram bank
    if ( (*d>=0x8000) && (*d<=0xFFFF) )	{
//        *d += 0x28000 + ( RamBank * 0x8000 );
//        return true;
        UINT32 _addr = *d &0x7FFF;

//        qWarning()<<(RamBank ? "S2:" : "S1:");
        readBus(RamBank ? busS2 : busS1 ,&_addr,data);
        return false;
    }
	

	return(1);
}

extern int ask(QWidget *parent,QString msg,int nbButton);
#define KEY(c)	((pKEYB->keyPressedList.contains(TOUPPER(c)) || \
                  pKEYB->keyPressedList.contains(c) || \
                  pKEYB->keyPressedList.contains(TOLOWER(c)))?1:0)
void Cpc1360::ComputeKey(KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)

    // Manage left connector click
    if ((currentView==LEFTview) && KEY(0x240) ) {
        FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                     QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                     FluidLauncher::PictureFlowType,QString(),
                                     "Sharp_11");
        launcher->show();
    }

    if ((currentView==BACKview) &&
            KEY(0x241) &&
            backdoorS1Open) {
        if (!pS1CONNECTOR->isLinked()) {
            pKEYB->keyPressedList.removeAll(0x241);
            FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                                        QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                                        FluidLauncher::PictureFlowType,QString(),
                                                        "Sharp_35");
            connect(launcher,SIGNAL(Launched(QString,CPObject *)),this,SLOT(linkObject(QString,CPObject *)));
            currentSlot = 1;
            launcher->show();
        }
        else {
            // unlink card
        }
    }
    if ((currentView==BACKview) &&
            KEY(0x242) &&
            backdoorS2Open) {
        if (!pS2CONNECTOR->isLinked()) {
            pKEYB->keyPressedList.removeAll(0x242);
            FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                                        QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                                        FluidLauncher::PictureFlowType,QString(),
                                                        "Sharp_35");
            connect(launcher,SIGNAL(Launched(QString,CPObject *)),this,SLOT(linkObject(QString,CPObject *)));
            currentSlot = 2;
            launcher->show();
        }
        else {
            // unlink card
        }
    }
    if ((currentView==BACKview) &&
            KEY(0x243)) {
        // open S1 & close S2
        pKEYB->keyPressedList.removeAll(0x243);

        animateBackDoorS1(true);
        animateBackDoorS2(false);

    }
    if ((currentView==BACKview) &&
            KEY(0x244)) {
        // close S1 & close S2
        pKEYB->keyPressedList.removeAll(0x244);

        animateBackDoorS1(false);
        animateBackDoorS2(false);

    }
    if ((currentView==BACKview) &&
            KEY(0x245)) {
        // close S1 & open S2
        pKEYB->keyPressedList.removeAll(0x245);

        animateBackDoorS1(false);
        animateBackDoorS2(true);

    }

}

void Cpc1360::linkObject(QString item,CPObject *pPC)
{
    qWarning()<<"currentslot:"<<currentSlot;
    Cconnector *_conn = 0;
    QRect _rect;
    if (currentSlot==1) {
        _conn = pS1CONNECTOR;
        _rect = pKEYB->getKey(0x241).Rect;
    }
    else {
        _conn = pS2CONNECTOR;
        _rect = pKEYB->getKey(0x242).Rect;
    }
    mainwindow->pdirectLink->addLink(_conn,pPC->ConnList.at(0),true);
    pPC->setPosX(posx()+_rect.left()*mainwindow->zoom);
    pPC->setPosY(posy()+_rect.top()*mainwindow->zoom);
    pPC->raise();
    if (currentSlot==1) {
        pPC->setRotation(180);
    }
    emit stackPosChanged();
}

bool Cpc1360::UpdateFinalImage(void) {
//    qWarning()<<"UpdateFinalImage";
    // Draw backdoor when not in frontview
    if (!flipping && (currentView != FRONTview) ) {
//        InitView(BACKview);
        delete BackImage;
        BackImage = new QImage(BackImageBackup);
    }

    Cpc13XX::UpdateFinalImage();

    if ((currentView != FRONTview) ) {
        QPainter painter;
        painter.begin(BackImage);
        painter.translate(19*internalImageRatio,30*internalImageRatio+backDoorImage->height()/2);
        QTransform matrix2;
        matrix2.rotate(m_backdoorS1Angle, Qt::YAxis);
        float sx = 0.55;
        if (m_backdoorS1Angle<45)
            sx = 1 - (float)m_backdoorS1Angle/100;
        else
            sx = .55 - (float)(m_backdoorS1Angle-45)/300;

        matrix2.scale(sx,1);
        painter.setTransform(matrix2,true);
        painter.drawImage(0,-backDoorImage->height()/2,*backDoorImage);
        painter.end();
    }

    if ((currentView != FRONTview) ) {
        QPainter painter;
        painter.begin(BackImage);
        painter.translate(284*internalImageRatio+backDoorImage->width(),30*internalImageRatio+backDoorImage->height()/2);
        QTransform matrix2;
        matrix2.rotate(m_backdoorS2Angle, Qt::YAxis);
        float sx = 0.55;
        if (m_backdoorS2Angle>-45)
            sx = 1 + (float)m_backdoorS2Angle/100;
        else
            sx = .55 + (float)(m_backdoorS2Angle+45)/300;

        matrix2.scale(sx,1);
        painter.setTransform(matrix2,true);
        painter.drawImage(-backDoorImage->width(),-backDoorImage->height()/2,backDoorImage->mirrored(true));
        painter.end();
    }
    return true;
}

bool Cpc1360::InitDisplay()
{
    Cpc13XX::InitDisplay();
    backDoorImage = new QImage(QImage(QString(P_RES(":/pc1360/backdoor.png"))).scaled(227*internalImageRatio,192*internalImageRatio));

    BackImageBackup = BackImage->copy();
    return true;
}

void Cpc1360::animateBackDoorS1(bool _open) {
    qWarning()<<"ANIMATE S1";
    if (backdoorS1Open == _open) return;

    backdoorS1Open = _open;


    QPropertyAnimation *animation1 = new QPropertyAnimation(this, "backdoorS1angle");
     animation1->setDuration(1500);
     if (backdoorS1Open) {
         animation1->setStartValue(m_backdoorS1Angle);
         animation1->setEndValue(70);
     }
     else {
         manageCardVisibility();
         animation1->setStartValue(m_backdoorS1Angle);
         animation1->setEndValue(0);
     }

     QParallelAnimationGroup *group = new QParallelAnimationGroup;
     group->addAnimation(animation1);

     connect(animation1,SIGNAL(valueChanged(QVariant)),this,SLOT(RefreshDisplay()));
     connect(animation1,SIGNAL(finished()),this,SLOT(endbackdoorAnimation()));
     backdoorFlipping = true;
     group->start();

}
void Cpc1360::animateBackDoorS2(bool _open) {
    qWarning()<<"ANIMATE S2";
    if (backdoorS2Open == _open) return;

    backdoorS2Open = _open;


    QPropertyAnimation *animation1 = new QPropertyAnimation(this, "backdoorS2angle");
     animation1->setDuration(1500);
     if (backdoorS2Open) {
         animation1->setStartValue(m_backdoorS2Angle);
         animation1->setEndValue(-70);
     }
     else {
         manageCardVisibility();
         animation1->setStartValue(m_backdoorS2Angle);
         animation1->setEndValue(0);
     }

     QParallelAnimationGroup *group = new QParallelAnimationGroup;
     group->addAnimation(animation1);

     connect(animation1,SIGNAL(valueChanged(QVariant)),this,SLOT(RefreshDisplay()));
     connect(animation1,SIGNAL(finished()),this,SLOT(endbackdoorAnimation()));
     backdoorFlipping = true;
     group->start();

}

void Cpc1360::setbackdoorS2Angle(int value) {
    this->m_backdoorS2Angle = value;
}



void Cpc1360::endbackdoorAnimation()
{
    backdoorFlipping = false;
    manageCardVisibility();
}
