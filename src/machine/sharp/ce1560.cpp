

#include <QPainter>
#include <QResource>
#include <QBitmap>
#include <QLabel>
#include <QLineEdit>

#include "common.h"
#include "ce1560.h"
#include "Lcdc_ce1560.h"
#include "buspc1500.h"
#include "clink.h"
#include "watchpoint.h"

Cce1560::Cce1560(CPObject *parent):CpcXXXX(parent)
{
    Q_UNUSED(parent)

    //[constructor]
    BackGroundFname	= P_RES(":/pc1500/ce1560_1.png");
    setcfgfname(QString("ce1560"));
//

    setDXmm(329);
    setDYmm(115);
    setDZmm(48);

    setDX(752);
    setDY(679);

    pLCDC		= new Clcdc_ce1560(this,
                                   QRect(188,60,384,128),
                                   QRect());
    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"ce1560.map");

    memsize		= 0x9A000;
    SlotList.clear();
    // Firmware: 2 page of 16Ko
    SlotList.append(CSlot(16 , 0x0000 ,	 ""	, "" , CSlot::RAM , "ROM Pg 0"));
    SlotList.append(CSlot(16 , 0x4000 ,	 ""	, "" , CSlot::RAM , "ROM Pg 1"));

    // RAMDISK: 16 x 32Ko
    SlotList.append(CSlot(32 , 0x10000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg 0"));
    SlotList.append(CSlot(32 , 0x18000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg 1"));
    SlotList.append(CSlot(32 , 0x20000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg 2"));
    SlotList.append(CSlot(32 , 0x28000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg 3"));
    SlotList.append(CSlot(32 , 0x30000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg 4"));
    SlotList.append(CSlot(32 , 0x38000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg 5"));
    SlotList.append(CSlot(32 , 0x40000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg 6"));
    SlotList.append(CSlot(32 , 0x48000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg 7"));
    SlotList.append(CSlot(32 , 0x50000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg 8"));
    SlotList.append(CSlot(32 , 0x58000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg 9"));
    SlotList.append(CSlot(32 , 0x60000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg A"));
    SlotList.append(CSlot(32 , 0x68000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg B"));
    SlotList.append(CSlot(32 , 0x70000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg C"));
    SlotList.append(CSlot(32 , 0x78000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg D"));
    SlotList.append(CSlot(32 , 0x80000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg E"));
    SlotList.append(CSlot(32 , 0x88000 ,	 ""	, "" , CSlot::RAM , "RAM Disk Pg F"));

    // ROM8000: 4 x 8Ko
    SlotList.append(CSlot(8 , 0x90000 ,	 ""	, "" , CSlot::RAM , "ROM8000 pg 0"));
    SlotList.append(CSlot(8 , 0x92000 ,	 ""	, "" , CSlot::RAM , "ROM8000 Pg 1"));
    SlotList.append(CSlot(8 , 0x94000 ,	 ""	, "" , CSlot::RAM , "ROM8000 Pg 2"));
    SlotList.append(CSlot(8 , 0x96000 ,	 ""	, "" , CSlot::RAM , "ROM8000 Pg 3"));


    //SlotList.append(CSlot(8 , 0x98000 ,	 ""	, "" , CSlot::RAM , "ROMA000 PV1"));


    bus = new CbusPc1500();

    for (int i=0;i<3;i++) ps6b0108[i]  = new CS6B0108(this);

    screenOpen = true;
    screenFlipping = false;
    m_screenAngle = 180;

    inhibitSwitch = false;
    resetSwitch = false;
    ramDiskPg = 0;
    firmwarePg= 0;
    rom8000Pg= 0;
    m_zoom = 1;
}

Cce1560::~Cce1560() {
    delete pCONNECTOR;
    delete bus;

//    for (int i=0;i<3;i++) delete ps6b0108[i];
}


bool Cce1560::init(void)
{
    CpcXXXX::init();

    setfrequency( 0);

    pCONNECTOR	= new Cconnector(this,60,0,Cconnector::Sharp_60,"Connector 60 pins",true,QPoint(62,450));	publish(pCONNECTOR);
    pEXTCONNECTOR= new Cconnector(this,60,1,Cconnector::Sharp_60,"Connector 60 pins Ext",false,QPoint(0,447),Cconnector::WEST);	publish(pEXTCONNECTOR);
    pTAPECONNECTOR= new Cconnector(this,3,2,Cconnector::Jack,"Line in / Rec / Rmt",false);	publish(pTAPECONNECTOR);

    WatchPoint.add(&pCONNECTOR_value,64,11,this,"Standard 60pins connector");
    WatchPoint.add(&pTAPECONNECTOR_value,64,2,this,"Line In / Rec");
    if(pKEYB)	pKEYB->init();
    if(pTIMER)	pTIMER->init();

    return(1);

}

bool Cce1560::run(void)
{
//static Cbus lastBus;
    bool forwardBus = true;

    bus->fromUInt64(pCONNECTOR->Get_values());

    bus->setINHIBIT(inhibitSwitch);
    if (resetSwitch && (pTIMER->msElapsedId(9)>5)) {
        resetSwitch = false;
        qWarning()<<"RESET Down";
    }
    bus->setRESET(resetSwitch);

    pTAPECONNECTOR->Set_pin(3,true);//(rmtSwitch ? SEL1:true));       // RMT
    pTAPECONNECTOR->Set_pin(2,bus->isCMTOUT());    // Out
    bus->setCMTIN( pTAPECONNECTOR->Get_pin(1));      // In

    pTAPECONNECTOR_value = pTAPECONNECTOR->Get_values();

//    if (lastBus.isEnable()!=bus->isEnable()) {
//        qWarning()<<"new status:"<<bus->isEnable();

//    }
//    lastBus.fromUInt64(bus->toUInt64());


    if (!bus->isEnable()) {
        pCONNECTOR->Set_values(bus->toUInt64());
        return true;
    }


    quint32 addr = bus->getAddr();

    // RAMDISK ACCESS
    if ( bus->isME1() &&
         (addr == 0xE300)) {
        ramDiskPg = bus->getData() & 0x0f;
     }
    // ROM8000 ACCESS
    if ( bus->isME1() &&
         (addr == 0xE400)) {
        rom8000Pg = bus->getData() & 0x03;
     }

    if ( (addr<=0x7FFF) &&
         bus->isME1() )
    {
        if (bus->isWrite()) {
            mem[0x10000 + addr + ramDiskPg*0x8000] = bus->getData();
            forwardBus = false;
            bus->setEnable(false);
            pCONNECTOR->Set_values(bus->toUInt64());
            return true;
        }
        else {
            bus->setData(mem[0x10000 + addr + ramDiskPg*0x8000]);
            forwardBus = false;
            bus->setEnable(false);
            pCONNECTOR->Set_values(bus->toUInt64());
            return true;
        }
    }

    if ( (bus->getAddr()>=0x8000) && (bus->getAddr()<=0x9FFF) &&
         !bus->isPV() &&
         !bus->isME1() )
    {
        //rom8000Pg = (bus->isPV() ? 0x02 : 00) | (bus->isPU() ? 0x01 : 00);
        if (!bus->isWrite()) {
            bus->setData(mem[0x90000 + addr - 0x8000 + rom8000Pg * 0x2000]);
            forwardBus = false;
            bus->setEnable(false);
            pCONNECTOR->Set_values(bus->toUInt64());
//            qWarning()<<"CE-1560 read["<<QString("%1").arg(addr - 0xC000 + firmwarePg * 0x4000,5,16,QChar('0'))<<"]="<<bus->getData();
            return true;
        }
        else {
            qWarning()<<"CE-1560 write["<<QString("%1").arg(0x90000 + addr - 0x8000 + rom8000Pg * 0x2000,5,16,QChar('0'))<<"]="<<bus->getData();
        }
    }

    if ( (bus->getAddr()>=0x8000) && (bus->getAddr()<=0xBFFF) &&
         !bus->isINHIBIT() &&
         bus->isPV() &&
         !bus->isME1() )
    {
        firmwarePg = (bus->isPU() ? 0x01 : 00);
        if (!bus->isWrite()) {
            bus->setData(mem[addr - 0x8000 + firmwarePg * 0x4000]);
            forwardBus = false;
            bus->setEnable(false);
            pCONNECTOR->Set_values(bus->toUInt64());
//            qWarning()<<"CE-1560 read["<<QString("%1").arg(addr - 0xC000 + firmwarePg * 0x4000,5,16,QChar('0'))<<"]="<<bus->getData();
            return true;
        }
        else {
            qWarning()<<"CE-1560 write["<<QString("%1").arg(addr - 0x8000 + firmwarePg * 0x4000,5,16,QChar('0'))<<"]="<<bus->getData();
        }
    }

    if ( (bus->getAddr()>=0xC000) && (bus->getAddr()<=0xFFFF) &&
         bus->isINHIBIT() &&
         !bus->isME1() )
    {
        if (!bus->isWrite()) {
            bus->setData(mem[addr - 0xC000 + firmwarePg * 0x4000]);
            forwardBus = false;
            bus->setEnable(false);
            pCONNECTOR->Set_values(bus->toUInt64());
//            qWarning()<<"CE-1560 read["<<QString("%1").arg(addr - 0xC000 + firmwarePg * 0x4000,5,16,QChar('0'))<<"]="<<bus->getData();
            return true;
        }
        else {
            qWarning()<<"CE-1560 write["<<QString("%1").arg(addr - 0xC000 + firmwarePg * 0x4000,5,16,QChar('0'))<<"]="<<bus->getData();
        }
    }


    if ( (bus->isME1() && (bus->getAddr()>=0xE200) && (bus->getAddr()<=0xE20F))
//        || (bus->isME1() && (bus->getAddr()>=0x0000) && (bus->getAddr()<=0x000F))
         )
    {

//        qWarning()<<bus->toLog();
        forwardBus = false;
        quint8 module = (bus->getAddr() >> 1 );
        quint8 reg = (bus->getAddr()&0x0f) - (module*2);


        if (bus->isWrite()) {
            quint16 cmd = ((reg==0)? 0x00 : 0x100) | bus->getData()  ;
//                    qWarning()<<"Write Data:"<<bus->toLog()<<"  module="<<module<<" reg="<<reg<< "cmd:"<<cmd;

            switch (module) {
            case 0:
            case 1:
            case 2: ps6b0108[module]->instruction(cmd); break;
            case 3: ps6b0108[0]->instruction(cmd); ps6b0108[1]->instruction(cmd); break;
            case 4: ps6b0108[0]->instruction(cmd); ps6b0108[2]->instruction(cmd); break;
            case 5: ps6b0108[1]->instruction(cmd); ps6b0108[2]->instruction(cmd); break;
            case 6: ps6b0108[0]->instruction(cmd); ps6b0108[1]->instruction(cmd); ps6b0108[2]->instruction(cmd); break;
            default:
                qWarning()<<"ERROR Write Data:"<<bus->toLog()<<"  module="<<module<<" reg="<<reg<<" data="<<bus->getData();
                break;
            }


        }
        else {
//            qWarning()<<"Read Data:"<<bus->getAddr()<<"="<<bus->toLog();
            quint16 cmd = ((reg==0)? 0x200 : 0x300);// | bus->getData();
//qWarning()<<tr("CE1560 CMD:%1").arg(cmd,4,16,QChar('0'));
            switch (module) {
            case 0:
            case 1:
            case 2:
                bus->setData(ps6b0108[module]->instruction(cmd)); break;
            case 3:
                bus->setData(ps6b0108[0]->instruction(cmd) | ps6b0108[1]->instruction(cmd)); break;
            case 4:
                bus->setData(ps6b0108[0]->instruction(cmd) | ps6b0108[2]->instruction(cmd)); break;
            case 5:
                bus->setData(ps6b0108[1]->instruction(cmd) | ps6b0108[2]->instruction(cmd)); break;
            case 6:
                bus->setData(ps6b0108[0]->instruction(cmd) |
                             ps6b0108[1]->instruction(cmd) |\
                             ps6b0108[2]->instruction(cmd));
                break;
            default:
                qWarning()<<"ERROR Read Data:"<<bus->toLog()<<"  module="<<module<<" reg="<<reg<<" data="<<bus->getData();
                break;
            }
        }

    }

    // Manage EXT Connector
    if (forwardBus) {
        // copy MainConnector to Ext Connectors
        pEXTCONNECTOR->Set_values(bus->toUInt64());
        // execute Ext
        mainwindow->pdirectLink->outConnector(pEXTCONNECTOR);
        bus->fromUInt64(pEXTCONNECTOR->Get_values());
    }

    bus->setINHIBIT(inhibitSwitch);
    bus->setRESET(resetSwitch);
    bus->setEnable(false);
    pCONNECTOR->Set_values(bus->toUInt64());

    return true;
}


bool Cce1560::exit(void)
{

    CPObject::exit();

    return true;
}

bool Cce1560::LoadConfig(QXmlStreamReader *xmlIn)
{

    if (xmlIn->readNextStartElement()) {
        if (xmlIn->name() == "config" && xmlIn->attributes().value("version") == "1.0") {
            for (int i=0; i<3; i++) {
                ps6b0108[i]->Load_Internal(xmlIn);
            }
            if (xmlIn->readNextStartElement() && xmlIn->name() == "internal" ) {
                ramDiskPg = xmlIn->attributes().value("RamBank").toString().toInt(0,16);
                firmwarePg = xmlIn->attributes().value("FirmBank").toString().toInt(0,16);
                inhibitSwitch = xmlIn->attributes().value("inhibitswitch").toString().toInt(0,16);
                screenOpen = xmlIn->attributes().value("open").toString().toInt(0,16);
                xmlIn->skipCurrentElement();
            }

        }
        xmlIn->skipCurrentElement();
    }

    return true;
}

bool Cce1560::SaveConfig(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("config");
    xmlOut->writeAttribute("version", "1.0");
        for (int i=0; i<3; i++) {
            ps6b0108[i]->save_internal(xmlOut);
        }
        xmlOut->writeStartElement("internal");
            xmlOut->writeAttribute("RamBank",QString("%1").arg(ramDiskPg,2,16));
            xmlOut->writeAttribute("FirmBank",QString("%1").arg(firmwarePg,2,16));
            xmlOut->writeAttribute("inhibitswitch",QString("%1").arg(inhibitSwitch));
            xmlOut->writeAttribute("open",QString("%1").arg(screenOpen));
        xmlOut->writeEndElement();
    xmlOut->writeEndElement();

    return true;
}

//extern int ask(QWidget *parent,QString msg,int nbButton);
#define KEY(c)	((pKEYB->keyPressedList.contains(TOUPPER(c)) || \
                  pKEYB->keyPressedList.contains(c) || \
                  pKEYB->keyPressedList.contains(TOLOWER(c)))?1:0)
void Cce1560::ComputeKey(KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)


    if (screenOpen && KEY(0x242)) {
        inhibitSwitch = false;
        resetSwitch = true;
        qWarning()<<"RESET Up";
        bool _b = pTIMER->resetTimer(9);
        qWarning()<<"INHIBIT FALSE";
        pKEYB->keyPressedList.removeAll(0x242);
        Refresh_Display = true;
        update();
    }
    if (screenOpen && KEY(0x243)) {
        inhibitSwitch = true;
        resetSwitch = true;
        qWarning()<<"RESET Up";
        bool _b = pTIMER->resetTimer(9);
        qWarning()<<"INHIBIT TRUE"<<_b;
        pKEYB->keyPressedList.removeAll(0x243);
        Refresh_Display = true;
        update();
    }
    if (screenOpen && KEY(0x244)) {
        firmwarePg = 0;
        qWarning()<<"Firmware Pg 0";
        pKEYB->keyPressedList.removeAll(0x244);
        Refresh_Display = true;
        update();
    }
    if (screenOpen && KEY(0x245)) {
        firmwarePg = 1;
        qWarning()<<"Firmware Pg 1";
        pKEYB->keyPressedList.removeAll(0x245);
        Refresh_Display = true;
        update();
    }


    // Manage backdoor click
//    qWarning()<<"lastkey="<<pKEYB->LastKey<<" -"<<pKEYB->keyPressedList<<" - "<<( (currentView==FRONTview) && ( (screenOpen && KEY(0x240) ) || (!screenOpen && KEY(0x241) )));
    if  ( (currentView==FRONTview) && ( (screenOpen && KEY(0x240) ) || (!screenOpen && KEY(0x241) ))) {
        pKEYB->keyPressedList.removeAll(0x240);
        pKEYB->keyPressedList.removeAll(0x241);
        screenOpen = !screenOpen;
//        qWarning()<<"compute key screen to:"<<screenOpen;
        animateScreen();
    }

}

#define RATIO (251.0/502.0)
bool Cce1560::UpdateFinalImage(void) {

    delete FinalImage;
    FinalImage = new QImage(*BackgroundImageBackup);
    pLCDC->Refresh=true;
    CpcXXXX::UpdateFinalImage();


    QPainter painter;
    painter.begin(FinalImage);

    // update inhibitSwitch
    if (inhibitSwitch) {
        painter.drawImage(164*internalImageRatio,258*internalImageRatio,
                          BackgroundImageBackup->copy(164*internalImageRatio,258*internalImageRatio,
                                                      56*internalImageRatio,27*internalImageRatio).mirrored(inhibitSwitch,false));
    }
    if (firmwarePg==1) {
        painter.drawImage(164*internalImageRatio,287*internalImageRatio,
                          BackgroundImageBackup->copy(164*internalImageRatio,287*internalImageRatio,
                                                              56*internalImageRatio,27*internalImageRatio).mirrored(firmwarePg==1,false));
    }



    QImage screenImage = FinalImage->copy(60*internalImageRatio,0,(getDX()-60)*internalImageRatio,getDY()*internalImageRatio/2).mirrored(false,true);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(60*internalImageRatio,0,
                     (getDX()-60)*internalImageRatio,getDY()*internalImageRatio/2,
                     Qt::transparent);

    painter.translate((getDX()+60*internalImageRatio)/2,
                      getDY()*internalImageRatio/2);

    QTransform matrix;
    matrix.scale(m_zoom,m_zoom);
    painter.setTransform(matrix,true);
//    painter.drawImage(QPoint(-w/2,0),
//                      FinalImage->scaled(QSize(w,h),Qt::IgnoreAspectRatio,Qt::SmoothTransformation),
//                      QRect(0,h*RATIO,w,h-h*RATIO));

    QTransform matrix2;
    matrix2.rotate(m_screenAngle, Qt::XAxis);
    painter.setTransform(matrix2,true);

    if (m_screenAngle<=90)
        screenImage = coverImage;

    painter.drawImage(-getDX()/2+60*internalImageRatio/2,0,screenImage);
    painter.end();
    mask = QPixmap::fromImage(*FinalImage).scaled(getDX()*mainwindow->zoom,
                                                  getDY()*mainwindow->zoom);

    setMask(mask.mask());

    if (!forceStackOver && (m_screenAngle <= 90)) {
        forceStackOver=true;
        QList<CPObject *> list;
        mainwindow->pdirectLink->findAllObj(this,&list);
        for (int i=0;i<list.size();i++) {
            list.at(i)->raise();
        }
        list.clear();

        manageStackPos(&list);
        emit stackPosChanged();

    }
    if ( forceStackOver && (m_screenAngle > 90)) {
        forceStackOver=false;
        QList<CPObject *> list;
        mainwindow->pdirectLink->findAllObj(this,&list);
        for (int i=0;i<list.size();i++) {
            list.at(i)->raise();
        }
        list.clear();

        manageStackPos(&list);
        emit stackPosChanged();
    }

    return true;
}

bool Cce1560::InitDisplay()
{
    CpcXXXX::InitDisplay();
    coverImage = QImage(P_RES(":/pc1500/ce1560_cover.png")).
            scaled((getDX()-60)*internalImageRatio,
                   getDY()*internalImageRatio/2);
//qWarning()<<coverImage.size();
    return true;
}

void Cce1560::animateScreen() {
//    qWarning()<<"ANIMATE";
    QPropertyAnimation *animation1 = new QPropertyAnimation(this, "screenangle");
//    QPropertyAnimation *animation2 = new QPropertyAnimation(this, "zoom");
     animation1->setDuration(1500);
//     animation2->setDuration(1500);
     if (!screenOpen) {
         animation1->setStartValue(m_screenAngle);
         animation1->setEndValue(0);

//         animation2->setKeyValueAt(0.0,1.0);
//         animation2->setKeyValueAt(0.5,.55);
//         animation2->setKeyValueAt(1.0,1.0);
         clearMask();
     }
     else {
         animation1->setStartValue(m_screenAngle);
         animation1->setEndValue(180);
//         animation2->setKeyValueAt(0,1.0);
//         animation2->setKeyValueAt(0.5,.55);
//         animation2->setKeyValueAt(1,1.0);
         clearMask();
         setGeometry(this->posx(),
                     this->posy(),
                     this->getDX()*mainwindow->zoom,
                     this->getDY()*mainwindow->zoom);

     }

     QParallelAnimationGroup *group = new QParallelAnimationGroup;
     group->addAnimation(animation1);
//     group->addAnimation(animation2);

     connect(animation1,SIGNAL(valueChanged(QVariant)),this,SLOT(updateAnimation()));
     connect(animation1,SIGNAL(finished()),this,SLOT(endscreenAnimation()));
     screenFlipping = true;
     group->start();

}

void Cce1560::endscreenAnimation()
{
    screenFlipping = false;
    if (screenOpen) {
//        pKEYB->Keys[backdoorKeyIndex].Rect.setSize(QSize(105,145));
    }
    else {
//        pKEYB->Keys[backdoorKeyIndex].Rect.setSize(QSize(365,145));
    }
}

void Cce1560::updateAnimation()
{
    Refresh_Display = true;
}
