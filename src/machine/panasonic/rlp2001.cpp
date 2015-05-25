#include <QtGui>
#include <QFileDialog>

#include "common.h"
#include "fluidlauncher.h"
#include "Log.h"
#include "pcxxxx.h"
#include "Connect.h"
#include "Keyb.h"
#include "rlp2001.h"
#include "buspanasonic.h"
#include "mc6847.h"
#include "Inter.h"

// TODO: performance issue with this module

Crlp2001::Crlp2001(CPObject *parent)   : CPObject(parent)
{                                                       //[constructor]
    Q_UNUSED(parent)

    setfrequency( 0);
    BackGroundFname     = P_RES(":/rlh1000/rlp2001.png");
    setDXmm(227);
    setDYmm(95);
    setDZmm(31);

 // Ratio = 3,57
    setDX(848);
    setDY(340);

    rotate = false;
    SlotList.clear();
    InitMemValue = 0x7f;



    memsize      = 0x2000;
    SlotList.append(CSlot(4 , 0x0000 , P_RES(":/rlh1000/rlp2001.bin") , ""        , CSlot::ROM , "ROM 4Ko"));
    SlotList.append(CSlot(4 , 0x1000 , "" , ""        , CSlot::RAM , "Ram Video"));

    INTrequest = false;

    pMC6847 = new MC6847();

    screen = new QImage(256,192,QImage::Format_ARGB32);
    oldState = 0;

}

Crlp2001::~Crlp2001(){
    delete(pCONNECTOR);
}

bool Crlp2001::run(void)
{
    CbusPanasonic bus;

    bus.fromUInt64(pCONNECTOR->Get_values());

    if (pTIMER) {
        if (oldState==0) oldState = pTIMER->state;
        if (screenUpdated && (pTIMER->msElapsed(oldState)>20) ) {
            pMC6847->draw_screen();
            oldState = pTIMER->state;
            screenUpdated = false;
            Refresh_Display = true;
        }
    }


    if (bus.getFunc()==BUS_SLEEP) return true;

    if ( (bus.getDest()!=0) && (bus.getDest()!=30)) return true;

    bus.setDest(0);

    if ( (bus.getFunc()==BUS_LINE0) && !bus.isWrite() ) {
        bus.setData(0x00);
        bus.setFunc(BUS_READDATA);
        pCONNECTOR->Set_values(bus.toUInt64());
        return true;
    }

    if ( (bus.getFunc()==BUS_LINE2) && bus.isWrite() ) {
//        qWarning()<<"2001 BUS SELECT:"<<bus.getData();

        switch (bus.getData()) {
        case 1: Power = true; break;
        default: Power = false; break;
        }
        if (Power)
        {
            bus.setFunc(BUS_READDATA);
            bus.setData(0x01);
            pCONNECTOR->Set_values(bus.toUInt64());
        }
        return true;
    }

    if ( (bus.getFunc()==BUS_LINE3) && bus.isWrite() ) {
            switch(bus.getData()) {
            case 0x00:
//                qWarning()<<"BUS_TOUCH:"<<bus.toLog();
                INTEnabled = false;
                break;
            case 0x80: //
//                qWarning()<<"BUS_TOUCH:"<<bus.toLog();
                INTEnabled = true;
                INTrequest = true;
                break;
            default: qWarning()<<"BUS_TOUCH:"<<bus.toLog();
                break;
            }
            bus.setFunc(BUS_ACK);
    }

    if ( (bus.getFunc()==BUS_LINE3) && !bus.isWrite() ) {
        if (INTEnabled && INTrequest) {
//            qWarning()<<"INTREQUEST:true";
            bus.setINT(true);
            bus.setData(0x00);
            INTrequest = false;
        }
        else {
//            qWarning()<<"INTREQUEST:false";
            bus.setData(0xff);
        }
        bus.setFunc(BUS_READDATA);
        pCONNECTOR->Set_values(bus.toUInt64());
        return true;
       }

    if ( (bus.getFunc()==BUS_LINE0) && bus.isWrite() ) {
        // Analyse command
        controlReg = bus.getData();
        pMC6847->displaySL = (controlReg & 0x0f) * 12;
//        pMC6847->set_vram_ptr(&mem[0x1000+(64*(controlReg & 0x0f))],0x400);
        qWarning()<<"Control Register set: "<<controlReg;

        bus.setFunc(BUS_ACK);
    }
    if ( (bus.getFunc()==BUS_LINE0) && !bus.isWrite() ) {
        qWarning()<<"Read data LINE 0:";
        bus.setFunc(BUS_ACK);
    }

    if (!Power) return true;

    quint32 adr = bus.getAddr();
//    quint8 data = bus.getData();

    switch (bus.getFunc()) {
    case BUS_SLEEP: break;
    case BUS_WRITEDATA:
        if((adr>=0x3000) && (adr < 0x4000)) {
            if (mem[adr-0x2000] != bus.getData()) {
                mem[adr-0x2000] = bus.getData();
//                qWarning()<<"Write video:"<<(adr-0x3000)<<"="<<QString("%1").arg(bus.getData(),2,16,QChar('0'));
//                pMC6847->draw_screen();
                screenUpdated = true;
            }
            INTrequest = true;

//            qWarning()<<"Write video:"<<(adr-0x3000)<<"="<<QString("%1").arg(bus.getData(),2,16,QChar('0'));
        }
        else {
            qWarning()<<"**** Write unh video:"<<QString("%1").arg(adr,4,16,QChar('0'))<<"="<<QString("%1").arg(bus.getData(),2,16,QChar('0'));
        }
        break;
    case BUS_READDATA:
        if ( (adr>=0x2000) && (adr<0x4000) ) {
            bus.setData(mem[adr-0x2000]);
        }
        else {
            bus.setData(0x7f);
            qWarning()<<"*****READ "<<QString("%1").arg(adr,4,16,QChar('0'));
        }
        break;
    default:
//        qWarning()<<bus.toLog();
        break;

    }

    pCONNECTOR->Set_values(bus.toUInt64());
    return true;

}

bool Crlp2001::UpdateFinalImage(void) {

//    CPObject::UpdateFinalImage();

    // Draw switch by 180 rotation
    QPainter painter;

    // PRINTER SWITCH
    painter.begin(FinalImage);

    painter.drawImage(200,20,screen->scaled(screen->size()*1.5));

    painter.end();

//    Refresh_Display = true;
    emit updatedPObject(this);
    return true;
}


/*****************************************************************************/
/* Initialize Potar                                                                                                                      */
/*****************************************************************************/
bool Crlp2001::init(void)
{
    AddLog(LOG_MASTER,"RL-P9001 initializing...");

    CPObject::init();

    pCONNECTOR = new Cconnector(this,44,0,
                                    Cconnector::Panasonic_44,
                                    "44 pins conector",
                                    true,
                                    QPoint(30,72),
                                    Cconnector::WEST);
    publish(pCONNECTOR);

    if(pKEYB)   pKEYB->init();

    Power = false;


    if (pMC6847) {
        pMC6847->set_vram_ptr(&mem[0x1000],0x400);
        pMC6847->set_screen_ptr(screen);
        pMC6847->load_font_bin(":/rlh1000/rlp2001char.bin");
        pMC6847->init();
    }

    AddLog(LOG_MASTER,"done.\n");

    return true;
}

/*****************************************************************************/
/* Exit Potar                                                                                                                                    */
/*****************************************************************************/
bool Crlp2001::exit(void)
{
    CPObject::exit();
    return true;
}

bool Crlp2001::SaveSession_File(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("session");
        xmlOut->writeAttribute("version", "2.0");
        xmlOut->writeAttribute("rotate",QString("%1").arg(rotate));
        xmlOut->writeStartElement("memory");
            for (int s=0; s<SlotList.size(); s++)                               // Save Memory
            {
                switch (SlotList[s].getType()) {
                case CSlot::RAM:
                case CSlot::CUSTOM_ROM: Mem_Save(xmlOut,s); break;
                default: break;
                }
            }
        xmlOut->writeEndElement();  // memory
    xmlOut->writeEndElement();  // session
    return true;
}

bool Crlp2001::LoadSession_File(QXmlStreamReader *xmlIn)
{
    if (xmlIn->name()=="session") {
        bool rot = xmlIn->attributes().value("rotate").toString().toInt(0,16);
        if (rotate != rot) Rotate();
        if (xmlIn->readNextStartElement() && xmlIn->name() == "memory" ) {
            AddLog(LOG_MASTER,"Load Memory");
            for (int s=0; s<SlotList.size(); s++)                               // Save Memory
            {
                switch (SlotList[s].getType()) {
                case CSlot::RAM:
                case CSlot::CUSTOM_ROM:
                    AddLog(LOG_MASTER,"    Load Slot"+SlotList[s].getLabel());
                    Mem_Load(xmlIn,s); break;
                default: break;
                }
            }
        }
    }
    return true;
}


void Crlp2001::paintEvent(QPaintEvent *event)
{
    CPObject::paintEvent(event);
}

void Crlp2001::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu = new QMenu(this);
    BuildContextMenu(menu);

    menu->addAction(tr("Dump Memory"),this,SLOT(Dump()));
    menu->addSeparator();

    menu->addAction(tr("Rotate 180"),this,SLOT(Rotate()));

    menu->popup(event->globalPos () );
    event->accept();
}

void Crlp2001::Rotate()
{
    rotate = ! rotate;

    InitDisplay();
}


bool Crlp2001::InitDisplay(void)
{

//    CPObject::InitDisplay();

    delete BackgroundImageBackup;
    BackgroundImageBackup = CreateImage(QSize(getDX(), getDY()),BackGroundFname,false,false,rotate?180:0);
    delete BackgroundImage;
    BackgroundImage = new QImage(*BackgroundImageBackup);
    delete FinalImage;
    FinalImage = new QImage(*BackgroundImageBackup);

    pCONNECTOR->setSnap(rotate?QPoint(811,72):QPoint(37,72));

    pCONNECTOR->setDir(rotate?Cconnector::EAST:Cconnector::WEST);

    mask = QPixmap::fromImage(*BackgroundImageBackup).scaled(getDX()*mainwindow->zoom/100,getDY()*mainwindow->zoom/100);
    setMask(mask.mask());

    return true;
}


