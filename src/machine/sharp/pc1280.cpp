//TODO  Save session needs to be checked
#include <QtGui>

#include "common.h"
#include "Log.h"

#include "pc1280.h"
#include "Lcdc_pc1475.h"
#include "cextension.h"
#include "cpu.h"
#include "watchpoint.h"
#include "Keyb1280.h"
#include "clink.h"
#include "bus.h"

Cpc1280::Cpc1280(CPObject *parent)	: Cpc1360(parent)
{								//[constructor]
    setfrequency(768000/3);
    setcfgfname("pc1280");

    SessionHeader	= "PC1280PKM";
    Initial_Session_Fname ="pc1280.pkm";

    BackGroundFname	= P_RES(":/pc1280/pc-1280.png");
    back = new QImage(P_RES(":/pc1280/pc-1280back.png"));
    LeftFname = RightFname = TopFname = BottomFname = BackFname = "";

    memsize			= 0x40000;

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	P_RES(":/pc1280/cpu-1280.rom")	, "pc-1280/cpu-1280.rom" , CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(8 , 0x2000 ,	""								, "pc-1280/R1-1280.ram" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x4000 ,	P_RES(":/pc1280/b0-1280.rom")	, "pc-1280/b0-1280.rom" , CSlot::ROM , "BANK 1"));
    SlotList.append(CSlot(32, 0x8000 ,	""								, "pc-1280/R2-1280.ram" , CSlot::RAM , "RAM"));
    SlotList.append(CSlot(16, 0x10000 ,	P_RES(":/pc1280/b0-1280.rom")	, "pc-1280/b0-1280.rom" , CSlot::ROM , "BANK 1"));
    SlotList.append(CSlot(16, 0x14000 ,	P_RES(":/pc1280/b1-1280.rom")	, "pc-1280/b1-1280.rom" , CSlot::ROM , "BANK 2"));
    SlotList.append(CSlot(16, 0x18000 ,	P_RES(":/pc1280/b2-1280.rom")	, "pc-1280/b2-1280.rom" , CSlot::ROM , "BANK 3"));
    SlotList.append(CSlot(16, 0x1C000 ,	P_RES(":/pc1280/b3-1280.rom")	, "pc-1280/b3-1280.rom" , CSlot::ROM , "BANK 4"));
    SlotList.append(CSlot(16, 0x20000 ,	P_RES(":/pc1280/b4-1280.rom")	, "pc-1280/b4-1280.rom" , CSlot::ROM , "BANK 1"));
    SlotList.append(CSlot(16, 0x24000 ,	P_RES(":/pc1280/b5-1280.rom")	, "pc-1280/b5-1280.rom" , CSlot::ROM , "BANK 2"));
    SlotList.append(CSlot(16, 0x28000 ,	P_RES(":/pc1280/b6-1280.rom")	, "pc-1280/b6-1280.rom" , CSlot::ROM , "BANK 3"));
    SlotList.append(CSlot(16, 0x2C000 ,	P_RES(":/pc1280/b7-1280.rom")	, "pc-1280/b7-1280.rom" , CSlot::ROM , "BANK 4"));
    SlotList.append(CSlot(32, 0x30000 ,	""								, "pc-1280/B0-1280.ram" , CSlot::RAM , "RAM S1"));
    SlotList.append(CSlot(32, 0x38000 ,	""								, "pc-1280/B1-1280.ram" , CSlot::RAM , "RAM S2"));

    KeyMap = KeyMap1280;
    KeyMapLenght = KeyMap1280Lenght;

    setDXmm(135);
    setDYmm(141);
    setDZmm(10);

    setDX(483);
    setDY(502);

    delete pLCDC;
    pLCDC		= new Clcdc_pc1280(this,
                                   QRect(80,48,328,30*1.22),
                                   QRect(80,40,328,48*1.22));
    delete pKEYB;
    pKEYB		= new Ckeyb(this,"pc1280.map",scandef_pc1280);

    remove(pSIOCONNECTOR);

    closed = false;
    flipping = false;
    m_angle = -180;
    m_zoom = 1;

}

Cpc1280::~Cpc1280() {
    delete back;
}

bool Cpc1280::init(void) {
    Cpc1360::init();
    WatchPoint.remove(&pSIOCONNECTOR_value);
//    delete pSIOCONNECTOR;

    closed = false;
    return true;
}

bool Cpc1280::InitDisplay()
{
    Cpc13XX::InitDisplay();

    return true;
}

void Cpc1280::initExtension(void)
{

    // initialise ext_MemSlot1
    ext_MemSlot1 = new CExtensionArray("Memory Slot 1","Add memory credit card");
    ext_MemSlot1->setAvailable(ID_CE210M,true);
    ext_MemSlot1->setAvailable(ID_CE211M,true);
    ext_MemSlot1->setAvailable(ID_CE212M,true);
    ext_MemSlot1->setAvailable(ID_CE2H16M,true);
    ext_MemSlot1->setAvailable(ID_CE2H32M,true);
    ext_MemSlot1->setAvailable(ID_CE2H64M,true);

    ext_MemSlot1->setChecked(ID_CE2H32M,true);

    addExtMenu(ext_MemSlot1);

    extensionArray[0] = ext_11pins;
    extensionArray[1] = ext_MemSlot1;

}

bool Cpc1280::Chk_Adr(UINT32 *d,UINT32 data)
{


    if (                 (*d<=0x1FFF) )	return(0);	// ROM area(0000-1fff)
    if ( (*d>=0x2000) && (*d<=0x27FF) )	return(0);
    if ( (*d>=0x2800) && (*d<=0x2B7B) ) { pLCDC->SetDirtyBuf(*d-0x2800); pLCDC->updated = true; return(1);	}
    if ( (*d>=0x3400) && (*d<=0x35FF) )	{ RomBank = data &0x07;	return(1); }
#if 0
    if ( (*d>=0x3C00) && (*d<=0x3DFF) )
    {
//		AddLog(LOG_MASTER,"Write Slot Register %04X=%02X",*d,mem[*d]);
        if (pCPU->fp_log) fprintf(pCPU->fp_log,"NEW BANK CHANGE =%i\n",RamBank);

        RamBank = (data == 0x04 ? 0 : 1);
        if (pCPU->fp_log) fprintf(pCPU->fp_log,"NEW BANK CHANGE =%i from data (%02x)\n",RamBank,data);
        return(1);
    }
#endif
    if ( (*d>=0x3E00) && (*d<=0x3FFF) )
    {
        int K=0;
        switch (data & 0x0F)
        {
        case 0x00: K=0x00;	break;
        case 0x01: K=0x01;	break;
        case 0x02: K=0x02;	break;
        case 0x03: K=0x04;	break;
        case 0x04: K=0x08;	break;
        case 0x05: K=0x10;	break;
        case 0x06: K=0x20;	break;
        case 0x07: K=0x40;	break;
        case 0x08: K=0x80;	break;
        case 0x09: K=0x00;	break;
        case 0x0A: K=0x00;	break;
        case 0x0B: K=0x00;	break;
        case 0x0C: K=0x00;	break;
        case 0x0D: K=0x00;	break;
        case 0x0E: K=0x00;	break;
        case 0x0F: K=0x7F;	break;
        }
        pKEYB->Set_KS( K );
        return(1);
    }




    if ( (*d>=0x2800) && (*d<=0x3FFF) )	return(1);
    if ( (*d>=0x4000) && (*d<=0x7FFF) )	{ *d += 0xC000 + ( RomBank * 0x4000 );	return(0); }


    if ( (*d>=0x8000) && (*d<=0xFFFF) )
    {
        int od = *d;
        bool ret = false;
        *d += 0x28000 + (RamBank * 0x8000);
        if ( (*d>=0x30000) && (*d<=0x33FFF) )	ret=(S1_EXTENSION_CE2H64M_CHECK | S1_EXTENSION_CE2H32M_CHECK);
        if ( (*d>=0x34000) && (*d<=0x35FFF) )	ret=(S1_EXTENSION_CE2H64M_CHECK | S1_EXTENSION_CE2H32M_CHECK | S1_EXTENSION_CE2H16M_CHECK);
        if ( (*d>=0x36000) && (*d<=0x36FFF) )	ret=(S1_EXTENSION_CE2H64M_CHECK | S1_EXTENSION_CE2H32M_CHECK | S1_EXTENSION_CE2H16M_CHECK | S1_EXTENSION_CE212M_CHECK);
        if ( (*d>=0x37000) && (*d<=0x377FF) )	ret=(S1_EXTENSION_CE2H64M_CHECK | S1_EXTENSION_CE2H32M_CHECK | S1_EXTENSION_CE2H16M_CHECK | S1_EXTENSION_CE212M_CHECK | S1_EXTENSION_CE211M_CHECK);
        if ( (*d>=0x37800) && (*d<=0x37FFF) )	ret=(S1_EXTENSION_CE2H64M_CHECK | S1_EXTENSION_CE2H32M_CHECK | S1_EXTENSION_CE2H16M_CHECK | S1_EXTENSION_CE212M_CHECK | S1_EXTENSION_CE211M_CHECK | S1_EXTENSION_CE210M_CHECK);
        if ( (*d>=0x38000) && (*d<=0x3FFFF) )	ret=(S1_EXTENSION_CE2H64M_CHECK);
        if (pCPU->fp_log) fprintf(pCPU->fp_log,"BANK (%i) ECRITURE [%04x](%05x)=%02x (%c) : %s\n",RamBank,od,*d,data,data,ret?"OK":"NO");
        return (ret);
//        if ( (*d>=0x38000) && (*d<=0x3BFFF) )	return(S2_EXTENSION_CE2H32M_CHECK);
//        if ( (*d>=0x3C000) && (*d<=0x3DFFF) )	return(S2_EXTENSION_CE2H32M_CHECK | S2_EXTENSION_CE2H16M_CHECK);
//        if ( (*d>=0x3E000) && (*d<=0x3EFFF) )	return(S2_EXTENSION_CE2H32M_CHECK | S2_EXTENSION_CE2H16M_CHECK | S2_EXTENSION_CE212M_CHECK);
//        if ( (*d>=0x3F000) && (*d<=0x3F7FF) )	return(S2_EXTENSION_CE2H32M_CHECK | S2_EXTENSION_CE2H16M_CHECK | S2_EXTENSION_CE212M_CHECK | S2_EXTENSION_CE211M_CHECK);
//        if ( (*d>=0x3F800) && (*d<=0x3FFFF) )	return(S2_EXTENSION_CE2H32M_CHECK | S2_EXTENSION_CE2H16M_CHECK | S2_EXTENSION_CE212M_CHECK | S2_EXTENSION_CE211M_CHECK | S2_EXTENSION_CE210M_CHECK);

    }

    //    if ( (*d>=0x8000) && (*d<=0xFFFF)) {
    //        *d += (RamBank * 0x8000);
    //        UINT32 _addr = *d &0xDFFF;
    //        writeBus(busS1 ,&_addr,data);

    return(0);
}

bool Cpc1280::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
    Q_UNUSED(data)

    if (                 (*d<=0x1FFF) )	return(1);	// ROM area(0000-1fff)

    if ( (*d>=0x4000) && (*d<=0x7FFF) )	{ *d += 0xC000 + ( RomBank * 0x4000 ); return(1); }	// Manage ROM Bank
    if (pCPU->fp_log) fprintf(pCPU->fp_log,"LECTURE [%04x]\n",*d);
    if ( (*d>=0x8000) && (*d<=0xFFFF) )	{ *d += 0x28000 + ( RamBank * 0x8000 ); return(1);}	// Manage ram bank


    //    UINT32 _addr = *d &0x7FFF;
    //    readBus(busS1 ,&_addr,data);

    return(1);
}

bool Cpc1280::Set_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

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
    pCONNECTOR->Set_pin(PIN_MT_OUT1	,pCPU->Get_Xout());


    return(1);
}

bool Cpc1280::Get_Connector(Cbus *_bus)
{
    Q_UNUSED(_bus)

    if (_bus == busS1) {
        busS1->fromUInt64(pS1CONNECTOR->Get_values());
        busS1->setEnable(false);
        return true;
    }

    Set_Port_Bit(PORT_B,1,pCONNECTOR->Get_pin(PIN_SEL1));	// DIN	:	IB1
    Set_Port_Bit(PORT_B,2,pCONNECTOR->Get_pin(PIN_SEL2));	// DIN	:	IB2
    Set_Port_Bit(PORT_B,3,pCONNECTOR->Get_pin(PIN_D_OUT));	// DIN	:	IB2


    Set_Port_Bit(PORT_B,4,pCONNECTOR->Get_pin(PIN_D_IN));	// DIN	:	IB8
    Set_Port_Bit(PORT_B,5,pCONNECTOR->Get_pin(PIN_ACK));	// ACK	:	IB7
    pCPU->Set_Xin(pCONNECTOR->Get_pin(PIN_MT_IN));

    return(1);
}

void Cpc1280::TurnON(void)
{
#if 1
    CpcXXXX::TurnON();
#else
    if (!Power && pKEYB->LastKey == K_BRK) {
        Initial_Session_Load();
        off = 0;
        Power = true;
        PowerSwitch = PS_RUN;
        if (pLCDC) pLCDC->TurnON();
    }
#endif
}

void Cpc1280::TurnOFF(void) {
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = ASK;
    AddLog(LOG_TEMP,"TURN OFF");
}

#define RATIO (251.0/502.0)

void Cpc1280::paintEvent(QPaintEvent *event)
{
    if (closed | flipping) {
        QPainter painter;

        if (!AnimatedImage) renderAnimation();
        if (AnimatedImage) {
        painter.begin(this);
        painter.drawImage(0,0,AnimatedImage->scaled(this->size()));
        painter.end();
        }
        else {
            qWarning()<<"ERROR AnimaedImage NULL.";
        }
    }
    else {
        CPObject::paintEvent(event);
    }
}

//FIXME: Animation issue when zoom != 100%
void Cpc1280::renderAnimation()
{
    if (closed | flipping) {

        UpdateFinalImage();

        AnimatedImage->fill(Qt::transparent);
        QPainter painter;
        painter.begin(AnimatedImage);

        if (FinalImage)
        {
//            int w = getDX()*internalImageRatio;// * mainwindow->zoom;//this->width();
//            int h = getDY()*internalImageRatio;// * mainwindow->zoom;//this->height();
            int w = AnimatedImage->width();
            int h = AnimatedImage->height();
            painter.translate(w/2 ,(h*RATIO)*(1+m_angle/180.0));
//            AddLog(LOG_MASTER,tr("zoom%1").arg(m_zoom));

            QTransform matrix;
            matrix.scale(m_zoom,m_zoom);
            painter.setTransform(matrix,true);
            painter.drawImage(QPoint(-w/2,0),
                              FinalImage->scaled(QSize(w,h),Qt::IgnoreAspectRatio,Qt::SmoothTransformation),
                              QRect(0,h*RATIO,w,h-h*RATIO));

            QTransform matrix2;
            matrix2.rotate(m_angle, Qt::XAxis);
            painter.setTransform(matrix2,true);
            if (m_angle <-90) {
                painter.drawImage(QPoint(-w/2,-h*RATIO),
                                  back->scaled(QSize(w,h),Qt::KeepAspectRatio,Qt::SmoothTransformation)
                                  );

            }
            else {
                painter.drawImage(QPoint(-w/2,-h*RATIO),
                                  FinalImage->scaled(QSize(w,h),Qt::IgnoreAspectRatio,Qt::SmoothTransformation),
                                  QRect(0,0,w,h*RATIO));
            }
        }
        painter.end();
        emit updatedPObject(this);
        update();
    }

}

void Cpc1280::TurnCLOSE(void) {
    // IF CONNECTED to printer , exit
    if (pCONNECTOR->isLinked()) return;

    if (flipping) return;
    // Animate close
    closed = !closed;

    QSize _s = size();
    QPropertyAnimation *animation1 = new QPropertyAnimation(this, "angle");
    QPropertyAnimation *animation2 = new QPropertyAnimation(this, "zoom");
     animation1->setDuration(1500);
     animation2->setDuration(1500);
     if (closed) {
         animation1->setStartValue(0);
         animation1->setEndValue(-180);
         animation2->setKeyValueAt(0.0,1.0);
         animation2->setKeyValueAt(0.5,.55);
         animation2->setKeyValueAt(1.0,1.0);
         delete AnimatedImage;
         AnimatedImage = new QImage(*FinalImage);
         clearMask();
     }
     else {
         animation1->setStartValue(-180);
         animation1->setEndValue(0);
         animation2->setKeyValueAt(0,1.0);
         animation2->setKeyValueAt(0.5,.55);
         animation2->setKeyValueAt(1,1.0);
//         clearMask();
         _s = QSize(this->getDX()*mainwindow->zoom,
                    this->getDY()*mainwindow->zoom);
//         delete AnimatedImage;
//         AnimatedImage = new QImage(FinalImage);
     }

     QParallelAnimationGroup *group = new QParallelAnimationGroup;
     group->addAnimation(animation1);
     group->addAnimation(animation2);

     connect(animation1,SIGNAL(valueChanged(QVariant)),this,SLOT(renderAnimation()));
     connect(animation1,SIGNAL(finished()),this,SLOT(endAnimation()));
//     flipping = true;
//     delete AnimatedImage;
//     AnimatedImage = new QImage(_s*mainwindow->zoom,QImage::Format_ARGB32);
     flipping = true;
     emit updatedPObject(this);
     changeGeometry(this->posx(),
                    this->posy(),
                    _s.width()*mainwindow->zoom,
                    _s.height()*mainwindow->zoom);
     group->start();

}

void Cpc1280::setAngle(int value) {
    this->m_angle = value;
}

void Cpc1280::setZoom(qreal value)
{
    this->m_zoom = value;
}

void Cpc1280::endAnimation()
{
    flipping = false;
    if (closed) {
        setGeometry(posx(),
                    posy(),
                    this->getDX()*mainwindow->zoom,
                    this->getDY()*mainwindow->zoom);
    }
    else {
        changeGeometry(this->posx(),
                       this->posy(),
                       this->getDX()*mainwindow->zoom,
                       this->getDY()*mainwindow->zoom);
    }
}
