#include <QtGui>
#include <QTime>
#include <QFileDialog>
#include <QThread>
#include <QDebug>

#include "common.h"
#include "fluidlauncher.h"

#include "rlp4002.h"

#include "Log.h"
#include "dialoganalog.h"
#include "paperwidget.h"
#include "Keyb.h"
#include "Connect.h"
#include "buspanasonic.h"
#include "cpu.h"

#define DOWN    0
#define UP              1


Crlp4002::Crlp4002(CPObject *parent):CPObject(this)
{ //[constructor]
    Q_UNUSED(parent)

    setfrequency( 0);

    BackGroundFname     = P_RES(":/rlh1000/rlp4002.png");
    setcfgfname("rlp4002");

    pTIMER              = new Ctimer(this);
    pKEYB               = new Ckeyb(this,"rlp4002.map");
    setDXmm(227);
    setDYmm(95);
    setDZmm(31);

 // Ratio = 3,57
    setDX(848);//Pc_DX  = 75;
    setDY(340);//Pc_DY  = 20;
    rotate = false;
    slotChanged = false;
    connected = false;
    rts = false;
    cts = false;

    memsize             = 0x2000;
    InitMemValue        = 0x7f;
    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 , P_RES(":/rlh1000/Telecomputing_2.bin")    , "" , CSlot::ROM , "ROM"));

}

Crlp4002::~Crlp4002() {
    delete pCONNECTOR;
}

#define RTS ( (commandReg & 0xa0)==0xa0 ? true: false)

bool Crlp4002::run(void)
{
    static quint64 _state=0;

    CbusPanasonic bus;

    bus.fromUInt64(pCONNECTOR->Get_values());

//    connected = true;
    if (connected &&
       (pTIMER->msElapsed(_state)>33) ) {   // 300 bds
        if (!_trans.isEmpty()) {
            inBuffer.append(_trans.at(0));
            _trans.remove(0,1);
        }
        _state = pTIMER->state;
    }

   if (xon && !inBuffer.isEmpty())  bus.setINT(true);

    if (bus.getFunc()==BUS_SLEEP) return true;

    if (bus.getDest()!=0) return true;
    bus.setDest(0);



//    qWarning()<<"MODEM:"<<bus.toLog();
    if ( (bus.getFunc()==BUS_LINE0) && !bus.isWrite() ) {
        bus.setData(0x00);
        bus.setFunc(BUS_READDATA);
        pCONNECTOR->Set_values(bus.toUInt64());
        if (pPC->pTIMER->pPC->fp_log) fprintf(pPC->pTIMER->pPC->fp_log,"RL-P1004A BUS_QUERY\n");
        return true;
    }

    if ( (bus.getFunc()==BUS_LINE2) && bus.isWrite() ) {
//        qWarning()<<"4002 BUS SELECT:"<<bus.getData();

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

        commandReg = bus.getData();

        cts = RTS;
        if (RTS) bus.setINT(true);

        if (bus.getData()!=0) {
            qWarning()<<"BUS_TOUCH:"<<QString("%1").arg(bus.getData(),2,16,QChar('0'))<<" rts:"<<RTS<<" cts:"<<cts;
        }

        bus.setFunc(BUS_ACK);
    }

    if ( (bus.getFunc()==BUS_LINE3) && !bus.isWrite() ) {
        // Status register ???
//        if (connected)
            statusReg = 0x30;
//        else statusReg = 0x10;
        if (cts) statusReg |= 0x40;
        else if (inBuffer.isEmpty()) statusReg |= 0x80;     // INPUT BUFFER EMPTY

        bus.setData(statusReg);

        if (!(statusReg&0x80)) {
            qWarning()<<"Read STATUS Reg:"<<QString("%1").arg(bus.getData(),2,16,QChar('0'))<<" pc="<<QString("%1").arg(pTIMER->pPC->pCPU->get_PC(),2,16,QChar('0'));
        }
        bus.setFunc(BUS_READDATA);
        pCONNECTOR->Set_values(bus.toUInt64());
        return true;
    }

    if ( (bus.getFunc()==BUS_LINE0) && bus.isWrite() ) {
        // Analyse command
        controlReg = bus.getData();
        qWarning()<<"Control Register set: "<<controlReg;

        bus.setFunc(BUS_ACK);
    }
    if ( (bus.getFunc()==BUS_LINE0) && !bus.isWrite() ) {
        qWarning()<<"Read data LINE 0:";
        bus.setFunc(BUS_ACK);
    }


    if ( (bus.getFunc()==BUS_LINE1) && bus.isWrite() ) {
        bus.setFunc(BUS_ACK);


        quint8 _c = bus.getData();

        qWarning()<<"Write data LINE 1:"<<_c<<" - "<<(_c>0?QChar(_c):' ')<<"  ** "<<outBuffer;
        //outBuffer.append(_c);
        switch (_c) {
        case 17: /*connected = false;*/
            rts=false;
            xon = true;
//            bus.setINT(true);
            break;
        case 19: /*connected = true;*/
            xon = false;
            break;
//        case 10:
        case 13:
            outBuffer.append(10);
            qWarning()<<"send data:"<<soc.write(outBuffer);
            outBuffer.clear();
            break;
        default: outBuffer.append(_c); break;
        }
    }

    if ( (bus.getFunc()==BUS_LINE1) && !bus.isWrite() ) {

        outputReg = 0;
        if (xon && !inBuffer.isEmpty()) {
            quint8 _c = inBuffer.at(0);
            inBuffer.remove(0,1);
            outputReg = _c;
            bus.setINT(true);
//                statusReg |= 0x88;
        }
        bus.setFunc(BUS_ACK);
        bus.setData(outputReg);

//        statusReg = 0x00;

        qWarning()<<connected<<"-Receive data LINE 1:"<<bus.getData()<<" - "<<(bus.getData()>0?QChar(bus.getData()):' ');
    }


    if (!Power) return true;

    quint32 adr = bus.getAddr();

    switch (bus.getFunc()) {
    case BUS_SLEEP: break;
    case BUS_WRITEDATA:
//        switch (adr) {
//        case 0x3020: // flip flop K7 output
//            tapeOutput = !tapeOutput;
        qWarning()<<pTIMER->state<<"WRITEDATA: "<<adr<<"="<<bus.getData();
//            bus.setData(0x00);
//            bus.setFunc(BUS_READDATA);
//            break;
//        }
        break;
    case BUS_READDATA:
        if ( (adr>=0x2000) && (adr<0x3000) ) bus.setData(mem[adr-0x2000]);
//        else if (adr == 0x3060){
//            bus.setData(tapeInput? 0x80 : 0x00);
//        }
        else bus.setData(0x7f);
        break;
    default: break;

    }

    pCONNECTOR->Set_values(bus.toUInt64());
    return true;

}


bool Crlp4002::init(void)
{
    CPObject::init();

    setfrequency( 0);

    pCONNECTOR = new Cconnector(this,44,0,
                                    Cconnector::Panasonic_44,
                                    "44 pins conector",
                                    true,
                                    QPoint(30,72),
                                    Cconnector::WEST);
    publish(pCONNECTOR);
    WatchPoint.add(&pCONNECTOR_value,64,44,this,"Printer connector");
    AddLog(LOG_PRINTER,tr("PRT initializing..."));

    if(pKEYB)   pKEYB->init();
    if(pTIMER)  pTIMER->init();

//    inBuffer.append("PREMIERE COMMUNICATION SERIE");
    statusReg = 0;
    outputReg = 0;
    controlReg = 0;
    xon = true;

    soc.connectToHost("127.0.0.1",4000);

    QObject:: connect(&soc, SIGNAL(readyRead()), this, SLOT(readData()));



    return true;
}



/*****************************************************/
/* Exit PRINTER                                                                          */
/*****************************************************/
bool Crlp4002::exit(void)
{
    CPObject::exit();
    return true;
}

bool Crlp4002::Get_Connector(void) {
    return true;
}

bool Crlp4002::Set_Connector(void) {
    return true;
}


void Crlp4002::paintEvent(QPaintEvent *event)
{
    CPObject::paintEvent(event);
}

void Crlp4002::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu= new QMenu(this);

    BuildContextMenu(menu);


    menu->addAction(tr("Dump Memory"),this,SLOT(Dump()));
    menu->addSeparator();

    menu->addAction(tr("Rotate 180"),this,SLOT(Rotate()));
//    menu.addAction(tr("Hide console"),this,SLOT(HideConsole()));

    menu->popup(event->globalPos () );
    event->accept();
}

void Crlp4002::Rotate()
{
    rotate = ! rotate;

    InitDisplay();

}

void Crlp4002::readData()
{
    QString ligne;
    while(soc.bytesAvailable()) // tant qu'il y a quelque chose à lire dans la socket
    {
        char _c;
        soc.read(&_c,1);     // on lit une ligne
        //emit vers_IHM_texte(ligne); // on envoie à l'IHM
        _trans.append(_c);
    }
}

extern int ask(QWidget *parent,QString msg,int nbButton);
#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))
void Crlp4002::ComputeKey(KEYEVENT ke,int scancode)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)


    if (pKEYB->LastKey>0) {
        connected = !connected;
    }
}

void Crlp4002::addModule(QString item,CPObject *pPC)
{
    Q_UNUSED(pPC)

    qWarning()<<"Add Module:"<< item;

    int _res = 0;
    QString moduleName;
    if (item=="TELECOMP_I") moduleName = P_RES(":/rlh1000/Telecomputing_1.bin");
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
        SlotList[0].setEmpty(false);
        SlotList[0].setResID(moduleName);
        Mem_Load(0);
        // Analyse capsule
        // 0x01 = 'C'
        // 0x01 - 0x28 : Copyright
        // 0x2C : title lenght
        // 0x2D - .. : title

        BYTE* capsule = &mem[0];
        if (capsule[1]=='C') {
            QString copyright = QString::fromLocal8Bit(QByteArray((const char*)&capsule[1],0x26));
            QString title  = QString::fromLocal8Bit(QByteArray((const char*)&capsule[0x2d],capsule[0x2c]));
            qWarning()<<"title:"<<title;
            SlotList[0].setLabel(title);
        }

        slotChanged = true;
    }
}


bool Crlp4002::SaveSession_File(QXmlStreamWriter *xmlOut)
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

bool Crlp4002::LoadSession_File(QXmlStreamReader *xmlIn)
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

bool Crlp4002::InitDisplay(void)
{

//    CPObject::InitDisplay();
    slotChanged = true;

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
