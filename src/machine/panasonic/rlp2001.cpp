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

Crlp2001::Crlp2001(CPObject *parent)   : CPObject(this)
{                                                       //[constructor]
    Q_UNUSED(parent)

    setfrequency( 0);
    BackGroundFname     = P_RES(":/rlh1000/rlp2001.png");
    setDXmm(227);
    setDYmm(95);
    setDZmm(31);

 // Ratio = 3,57
    setDX(848);//Pc_DX  = 75;
    setDY(340);//Pc_DY  = 20;

    rotate = false;
    SlotList.clear();
    InitMemValue = 0x7f;



    memsize      = 0x1000;
    SlotList.append(CSlot(4 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 4Ko"));

}

Crlp2001::~Crlp2001(){
    delete(pCONNECTOR);
}



bool Crlp2001::run(void)
{
    return true;

#if 0
    Cbus bus;

    bus.fromUInt64(pCONNECTOR->Get_values());
    if (bus.getFunc()==BUS_SLEEP) return true;

    if ( (bus.getDest()!=0) && (bus.getDest()!=30)) return true;

//    bus.setDest(0);

    if (bus.getFunc()==BUS_QUERY) {
        bus.setData(0xFB);
        if (romSwitch) bus.setData(0xf7);
        if (model==RLP9006) bus.setData(0xf7);
        bus.setFunc(BUS_READDATA);
        pCONNECTOR->Set_values(bus.toUInt64());
        return true;
    }

    if (bus.getFunc()==BUS_SELECT) {
        Power = false;
        bank=0;
//        qWarning()<<"9001 BUS SELECT:"<<bus.getData();
        if (romSwitch) {
            Power = false;
            if ( bus.getData()==128 ) {
                Power = true;
                bank = 0;
            }
        }
        else if (model==RLP9006) {
            switch (bus.getData()) {
            case 1: Power = true; bank = 7; break;
            case 2: Power = true; bank = 6;  break;
            case 4: Power = true; bank = 5;  break;
            case 8: Power = true; bank = 4;  break;
            case 16: Power = true; bank = 3;  break;
            case 32: Power = true; bank = 2;  break;
            case 64: Power = true; bank = 1;  break;
            case 128: Power = true; bank = 0;  break;
            default: Power = false; break;
            }
        }
        else {
            switch (bus.getData()) {
            case 1: Power = true; bank = 0; break;
            case 2: if ((model == RLP9004)||(model == RLP9005)) { Power = true; bank = 1; } break;
            case 4: if (model == RLP9005) { Power = true; bank = 2; } break;
            case 8: if (model == RLP9005) { Power = true; bank = 3; } break;
            case 16: if (model == RLP9005) { Power = true; bank = 4; } break;
            case 32: if (model == RLP9005) { Power = true; bank = 5; } break;
            case 64: if (model == RLP9005) { Power = true; bank = 6; } break;
            case 128: if (model == RLP9005) { Power = true; bank = 7; } break;
            default: Power = false; break;
            }
        }

        if (Power)
        {
            bus.setFunc(BUS_READDATA);
            bus.setData(0x00);
            pCONNECTOR->Set_values(bus.toUInt64());
        }
        return true;
    }

    if (!Power) return true;



    quint32 adr = bus.getAddr();

    switch (bus.getFunc()) {
    case BUS_SLEEP:
    case BUS_ACK: break;
    case BUS_WRITEDATA:

        switch (model) {
        case RLP9001: if((adr>=0x8000) && (adr < 0x9000)) mem[adr-0x8000] = bus.getData(); break;
        case RLP9002: if((adr>=0x8000) && (adr < 0xa000)) mem[adr-0x8000] = bus.getData(); break;
        case RLP9003: if((adr>=0x8000) && (adr < 0xc000)) mem[adr-0x8000] = bus.getData(); break;
        case RLP9003R:if((adr>=0x8000) && (adr < 0xc000)) mem[adr-0x8000] = bus.getData(); break;
        case RLP9004:
        case RLP9005: if((adr>=0x8000) && (adr < 0xc000)) mem[(adr-0x8000)+bank*0x4000] = bus.getData(); break;
        default: break;
        }

        break;
    case BUS_READDATA:

//        bus.setData(0x7f);
        switch (model) {
        case RLP9001: if((adr>=0x8000) && (adr < 0x9000)) bus.setData(mem[adr-0x8000]); break;
        case RLP9002: if((adr>=0x8000) && (adr < 0xa000)) bus.setData(mem[adr-0x8000]); break;
        case RLP9003: if((adr>=0x8000) && (adr < 0xc000)) bus.setData(mem[adr-0x8000]); break;
        case RLP9003R:
            if (romSwitch && (adr>=0x4000) && (adr < 0x8000)) {
//                qWarning()<<"ROM SIMUL:"<<adr<<"="<<mem[adr-0x4000+0x14];
                bus.setFunc(BUS_ACK);
                bus.setData(mem[adr-0x4000+romoffset]);
            }
            else if((adr>=0x8000) && (adr < 0xc000)) bus.setData(mem[adr-0x8000]);
            break;
        case RLP9004:
        case RLP9005: if((adr>=0x8000) && (adr < 0xc000)) bus.setData(mem[adr-0x8000+bank*0x4000]); break;
        case RLP9006:
            if ( (adr>=0x4000) && (adr < 0x8000)) {
//                qWarning()<<"ROM SIMUL:"<<adr<<"="<<mem[adr-0x4000+0x14];
                bus.setFunc(BUS_ACK);
                bus.setData(mem[adr-0x4000+bank*0x4000]);
            }
            break;
        default: break;
        }

        break;
    case BUS_INTREQUEST:
        bus.setData(0xff);
        bus.setFunc(BUS_READDATA);
        break;
    default: break;
    }

    pCONNECTOR->Set_values(bus.toUInt64());
    return true;

#endif
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


