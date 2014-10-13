#include <QtGui>
#include <QFileDialog>

#include "common.h"
#include "fluidlauncher.h"
#include "Log.h"
#include "pcxxxx.h"
#include "Connect.h"
#include "Keyb.h"
#include "rlp9001.h"
#include "buspanasonic.h"

Crlp9001::Crlp9001(CPObject *parent ,Models mod)   : CPObject(this)
{                                                       //[constructor]
    Q_UNUSED(parent)

    setfrequency( 0);
    BackGroundFname     = P_RES(":/rlh1000/rlp9002.png");
    model = mod;
    setDXmm(113);
    setDYmm(95);
    setDZmm(51);
 // Ratio = 3,57
    setDX(440);//Pc_DX  = 75;
    setDY(340);//Pc_DY  = 20;

    rotate = false;
    romSwitch = false;
    SlotList.clear();
    InitMemValue = 0x7f;
    bank = 0;
    slotChanged = false;

    switch(model) {
    case RLP9001:
        memsize      = 0x1000;
        SlotList.append(CSlot(4 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 4Ko"));
        break;
    case RLP9002:
        memsize      = 0x2000;
        SlotList.append(CSlot(8 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 8Ko"));
        break;
    case RLP9003:
        memsize      = 0x4000;
        SlotList.append(CSlot(16 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 16Ko"));
        break;
    case RLP9003R:
        BackGroundFname     = P_RES(":/rlh1000/rlp9003r.png");
        memsize      = 0x4000;
        SlotList.append(CSlot(16 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 16Ko"));
        break;
    case RLP9004:
        memsize      = 0x8000;
        SlotList.append(CSlot(16 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 16Ko bank 1"));
        SlotList.append(CSlot(16 , 0x4000 , "" , ""        , CSlot::RAM , "RAM 16Ko bank 2"));
        break;
    case RLP9005:
        memsize      = 0x20000;
        SlotList.append(CSlot(16 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 16Ko bank 1"));
        SlotList.append(CSlot(16 , 0x4000 , "" , ""        , CSlot::RAM , "RAM 16Ko bank 2"));
        SlotList.append(CSlot(16 , 0x8000 , "" , ""        , CSlot::RAM , "RAM 16Ko bank 3"));
        SlotList.append(CSlot(16 , 0xC000 , "" , ""        , CSlot::RAM , "RAM 16Ko bank 4"));
        SlotList.append(CSlot(16 , 0x10000 , "" , ""        , CSlot::RAM , "RAM 16Ko bank 5"));
        SlotList.append(CSlot(16 , 0x14000 , "" , ""        , CSlot::RAM , "RAM 16Ko bank 6"));
        SlotList.append(CSlot(16 , 0x18000 , "" , ""        , CSlot::RAM , "RAM 16Ko bank 7"));
        SlotList.append(CSlot(16 , 0x1C000 , "" , ""        , CSlot::RAM , "RAM 16Ko bank 8"));
        break;
    case RLP9006:
        pKEYB               = new Ckeyb(this,"rlp9006.map");
        BackGroundFname     = P_RES(":/rlh1000/rlp9006.png");
        memsize      = 0x20000;
        SlotList.append(CSlot(16 , 0x0000 , ""  , ""        , CSlot::CUSTOM_ROM , "ROM bank 1"));
        SlotList.append(CSlot(16 , 0x4000 , ""  , ""        , CSlot::CUSTOM_ROM , "ROM bank 2"));
        SlotList.append(CSlot(16 , 0x8000 , ""  , ""        , CSlot::CUSTOM_ROM , "ROM bank 3"));
        SlotList.append(CSlot(16 , 0xC000 , ""  , ""        , CSlot::CUSTOM_ROM , "ROM bank 4"));
        SlotList.append(CSlot(16 , 0x10000 , "" , ""        , CSlot::CUSTOM_ROM , "ROM bank 5"));
        SlotList.append(CSlot(16 , 0x14000 , "" , ""        , CSlot::CUSTOM_ROM , "ROM bank 6"));
        SlotList.append(CSlot(16 , 0x18000 , "" , ""        , CSlot::CUSTOM_ROM , "ROM bank 7"));
        SlotList.append(CSlot(16 , 0x1C000 , "" , ""        , CSlot::CUSTOM_ROM , "ROM bank 8"));
        break;
    default: break;
    }

}

Crlp9001::~Crlp9001(){
    delete(pCONNECTOR);
}



bool Crlp9001::run(void)
{

    CbusPanasonic bus;

    bus.fromUInt64(pCONNECTOR->Get_values());
    if (bus.getFunc()==BUS_SLEEP) return true;

    if ( (bus.getDest()!=0) && (bus.getDest()!=30)) return true;

//    bus.setDest(0);

    if ( (bus.getFunc()==BUS_LINE0) && !bus.isWrite() ) {
        bus.setData(0xFB);
        if (romSwitch) bus.setData(0xf7);
        if (model==RLP9006) bus.setData(0xf7);
        bus.setFunc(BUS_READDATA);
        pCONNECTOR->Set_values(bus.toUInt64());
        return true;
    }

    if ( (bus.getFunc()==BUS_LINE2) && bus.isWrite() ) {
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
    case BUS_LINE3:
        bus.setData(0xff);
        bus.setFunc(BUS_READDATA);
        break;
    default: break;
    }

    pCONNECTOR->Set_values(bus.toUInt64());
    return true;
}




/*****************************************************************************/
/* Initialize Potar                                                                                                                      */
/*****************************************************************************/
bool Crlp9001::init(void)
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
bool Crlp9001::exit(void)
{
    CPObject::exit();
    return true;
}

bool Crlp9001::SaveSession_File(QXmlStreamWriter *xmlOut)
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

bool Crlp9001::LoadSession_File(QXmlStreamReader *xmlIn)
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


void Crlp9001::paintEvent(QPaintEvent *event)
{
    CPObject::paintEvent(event);
}

void Crlp9001::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu = new QMenu(this);
    BuildContextMenu(menu);

    menu->addAction(tr("Dump Memory"),this,SLOT(Dump()));
    menu->addSeparator();

    menu->addAction(tr("Rotate 180"),this,SLOT(Rotate()));

    if (model==RLP9003R) {
        menu->addAction(tr("ROM Switch"),this,SLOT(ROMSwitch()));
        menu->addAction(tr("ROM Save"),this,SLOT(exportROM()));
    }

    menu->popup(event->globalPos () );
    event->accept();
}

void Crlp9001::Rotate()
{
    rotate = ! rotate;

    if (pKEYB) {
        // rotate Key pos
        //
        for (int i=0; i <pKEYB->Keys.size();i++) {
            QSize _size = pKEYB->Keys[i].Rect.size();
            pKEYB->Keys[i].Rect.setTop(this->getDY()-pKEYB->Keys[i].Rect.bottom());
            pKEYB->Keys[i].Rect.setLeft(this->getDX()-pKEYB->Keys[i].Rect.right());
            pKEYB->Keys[i].Rect.setSize(_size);
            //pKEYB->Keys[i].Rect=pKEYB->Keys[i].Rect.translated(rotate?-42:42,rotate?13:-13);
        }
    }
    InitDisplay();
}

void Crlp9001::ROMSwitch()
{
   romSwitch = !romSwitch;

   if (romSwitch) {
       // analyse memory to find the snap code offset
       // 0x0D filename lenght
       romoffset = 0x0E + mem[0x0d];
       if (mem[romoffset+1]!=0x43) {
           // no ROM found
           QMessageBox::critical(this,"ERROR","No ROM found in this module");
           romSwitch = false;
       }
   }
   qWarning()<<"romSwitch:"<<romSwitch;
}

void Crlp9001::exportROM() {
    if (romSwitch) {
        int size = mem[0x0a] + mem[0x0b]*0x100;
        qWarning()<<"size="<<size<<"  - "<<mem[0x0a]<<"-"<<mem[0x0b];
        QString fn = QFileDialog::getSaveFileName(
                    mainwindow,
                    tr("Choose a filename to save ROM file"),
                    ".",
                    tr("ROM File (*.bin)"));

        QFileInfo fi( fn );
                if (fi.suffix().isEmpty())
                {
                        // no suffix, adding .pml  - BUG For Android
                    fn.append(".bin");
                }
        QFile f(fn);
        if (f.open(QFile::WriteOnly | QFile::Truncate)) {
            QDataStream out(&f);
            out.writeRawData( (char *) &mem[romoffset],size );
            f.close();
        }
    }
    else {
        QMessageBox::warning(this,"ERROR","Please, switch to ROM simulator mode before trying to export the file");
    }
}

extern int ask(QWidget *parent,QString msg,int nbButton);
#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))
void Crlp9001::ComputeKey()
{
    if (model==RLP9006) {
        int _slot = -1;
        if (KEY(0x240)) _slot = 0;
        if (KEY(0x241)) _slot = 1;
        if (KEY(0x242)) _slot = 2;
        if (KEY(0x243)) _slot = 3;
        if (KEY(0x244)) _slot = 4;
        if (KEY(0x245)) _slot = 5;
        if (KEY(0x246)) _slot = 6;
        if (KEY(0x247)) _slot = 7;

        qWarning()<<"ComputKey:"<<_slot;
        pKEYB->keyPressedList.removeAll(0x240);
        pKEYB->keyPressedList.removeAll(0x241);
        pKEYB->keyPressedList.removeAll(0x242);
        pKEYB->keyPressedList.removeAll(0x243);
        pKEYB->keyPressedList.removeAll(0x244);
        pKEYB->keyPressedList.removeAll(0x245);
        pKEYB->keyPressedList.removeAll(0x246);
        pKEYB->keyPressedList.removeAll(0x247);
        if (_slot == -1) return;
        int _response = 0;
        BYTE* capsule = &mem[_slot*0x4000];
        if (!SlotList[_slot].isEmpty() || (capsule[0]!=0x7f)) {
            _response=ask(this,
                          "The "+SlotList[_slot].getLabel()+ " capsule is already plugged in this slot.\nDo you want to unplug it ?",
                          2);
        }

        if (_response == 1) {
            SlotList[_slot].setEmpty(true);

            memset((void *)capsule ,0x7f,0x4000);
            SlotList[_slot].setLabel(QString("ROM bank %1").arg(_slot+1));

            slotChanged = true;
        }
        if (_response==2) return;

        currentSlot = _slot;
        FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                                    QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                                    FluidLauncher::PictureFlowType,
                                                    "Panasonic_Capsule");
        connect(launcher,SIGNAL(Launched(QString,CPObject *)),this,SLOT(addModule(QString,CPObject *)));
        launcher->show();
    }
}

void Crlp9001::addModule(QString item,CPObject *pPC)
{
    Q_UNUSED(pPC)

    qWarning()<<"Add Module:"<< item;
    if ( (currentSlot<0) || (currentSlot>7)) return;

    int _res = 0;
    QString moduleName;
    if (item=="SNAPBASIC") moduleName = P_RES(":/rlh1000/SnapBasic.bin");
    if (item=="SNAPFORTH") moduleName = P_RES(":/rlh1000/SnapForth.bin");
    if (item=="MSBASIC")   moduleName = P_RES(":/rlh1000/HHCbasic.bin");
    if (item=="PANACAPSFILE") {
        moduleName = QFileDialog::getOpenFileName(
                    mainwindow,
                    tr("Choose a Capsule file"),
                    ".",
                    tr("Module File (*.bin)"));
//        customModule = CSlot::CUSTOM_ROM;
    }

    if (moduleName.isEmpty()) return;

    bool result = true; // check this is a true capsule

    qWarning()<<"loaded:"<<_res;
    if (result) {
        SlotList[currentSlot].setEmpty(false);
        SlotList[currentSlot].setResID(moduleName);
        Mem_Load(currentSlot);
        // Analyse capsule
        // 0x01 = 'C'
        // 0x01 - 0x28 : Copyright
        // 0x2C : title lenght
        // 0x2D - .. : title

        BYTE* capsule = &mem[currentSlot*0x4000];
        if (capsule[1]=='C') {
            QString copyright = QString::fromLocal8Bit(QByteArray((const char*)&capsule[1],0x26));
            QString title  = QString::fromLocal8Bit(QByteArray((const char*)&capsule[0x2d],capsule[0x2c]));
            qWarning()<<"title:"<<title;
            SlotList[currentSlot].setLabel(title);
        }

        slotChanged = true;
    }

    currentSlot = -1;

}

bool Crlp9001::InitDisplay(void)
{

//    CPObject::InitDisplay();
    slotChanged = true;

    delete BackgroundImageBackup;
    BackgroundImageBackup = CreateImage(QSize(getDX(), getDY()),BackGroundFname,false,false,rotate?180:0);
    delete BackgroundImage;
    BackgroundImage = new QImage(*BackgroundImageBackup);
    delete FinalImage;
    FinalImage = new QImage(*BackgroundImageBackup);

    pCONNECTOR->setSnap(rotate?QPoint(406,72):QPoint(34,72));

    pCONNECTOR->setDir(rotate?Cconnector::EAST:Cconnector::WEST);
    mask = QPixmap::fromImage(*BackgroundImageBackup).scaled(getDX()*mainwindow->zoom/100,getDY()*mainwindow->zoom/100);
    setMask(mask.mask());

    return true;
}

bool Crlp9001::UpdateFinalImage(void) {

//    CPObject::UpdateFinalImage();



    // on TOP view, draw installed modules
    if ((model == RLP9006) && (currentView == FRONTview) && slotChanged) {
        InitDisplay();
        slotChanged = false;
        QPainter painter;
        painter.begin(FinalImage);

        QMatrix matrix;
        matrix.rotate(rotate?180:0);
        QImage _rotCapsule = QImage(QString(P_RES(":/rlh1000/capsule.png"))).transformed(matrix).scaled(pKEYB->Keys[0].Rect.size());
        for (int i=0;i<8;i++) {
            if (!SlotList[i].isEmpty()) {
                // draw capsule

                painter.drawImage(pKEYB->Keys[i].Rect.left(),
                                  pKEYB->Keys[i].Rect.top(),
                                  _rotCapsule);
            }
        }
        painter.end();
    }

    emit updatedPObject(this);
    return true;
}
