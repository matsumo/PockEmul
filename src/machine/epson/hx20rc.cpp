

#include <QDebug>

#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickView>
#include <QtQuickWidgets/QQuickWidget>

#include "hx20rc.h"
#include "Connect.h"
#include "Keyb.h"
#include "Inter.h"
#include "Log.h"
#include "watchpoint.h"

typedef struct
    {
        BYTE	name[8];
        BYTE	ext[3];
        BYTE	progType,dataType;
        BYTE	dummy[3];
        BYTE	start[4];
        BYTE	end[4];
        BYTE	creationDate[6];
        BYTE	version[2];
    }HX20CARTRIDGE_SLOT;

typedef struct
    {
        HX20CARTRIDGE_SLOT	slot[32];
        BYTE	mem[0x1800];
    }HX20CARTRIDGE;

Chx20RC::Chx20RC(CPObject *parent ):CPObject(parent)
{
    setDXmm(42);
    setDYmm(54);
    setDZmm(30);

    setDX(230);
    setDY(330);

    BackGroundFname	= P_RES(":/h20rc/h20rc.png");
    setcfgfname(QString("h20rc"));

    InitMemValue = 0x00;
    memsize      = 0x8000;
    SlotList.clear();
    SlotList.append(CSlot(32, 0x0000 , "" , ""        , CSlot::CUSTOM_ROM , "ROM 32KB"));

    ShiftRegisterOutput=false;
    ClearCounter=prevClearCounter=false;
    PowerOn=prevPowerOn=false;
    ShiftRegisterClear=prevShiftRegisterClear=false;
    Clock=prevClock=false;
    ShiftLoad=prevShiftLoad=false;

    EMView=0;
}

Chx20RC::~Chx20RC()
{

}

bool Chx20RC::init()
{
    CPObject::init();
    setfrequency( 0);

    pCONNECTOR = new Cconnector(this,14,1,Cconnector::Epson_CN8,"Micro K7 / ROM Cartrifge Connector",false,
                                QPoint(0,113),Cconnector::WEST);
    publish(pCONNECTOR);
    WatchPoint.add(&pCONNECTOR_value,64,14,this,"Micro K7 / ROM Cartrifge Connector");
    if(pKEYB)	pKEYB->init();

    return true;
}

bool Chx20RC::exit()
{
    return true;
}

bool Chx20RC::run()
{

    ClearCounter = pCONNECTOR->Get_pin(9);
    PowerOn = pCONNECTOR->Get_pin(8);
    ShiftRegisterClear = pCONNECTOR->Get_pin(7);
    Clock = pCONNECTOR->Get_pin(4);
    ShiftLoad = pCONNECTOR->Get_pin(3);

    pCONNECTOR->Set_pin(11,false);
    pCONNECTOR->Set_pin(12,false);
    pCONNECTOR->Set_pin(13,false);
    pCONNECTOR->Set_pin(14,false);

    if (PowerOn) {

        if (PowerOn && ! prevPowerOn) {
            Counter = 0;
            qWarning()<<"Power ON";
        }

        if (ClearCounter) {
            Counter=0;
            qWarning()<<"Clear Counter";
        }

        if (prevShiftLoad && !ShiftLoad) {
            Counter++;
            qWarning()<<"Counter ++:"<<Counter;
        }

        if (!ShiftLoad && (!prevClock && Clock)) {
            ShiftRegister = mem[Counter];
            ShiftRegisterOutput = ShiftRegister & 0x80;
            qWarning()<<tr("Load Shift Register:%1").arg(ShiftRegister,2,16,QChar('0'))<<ShiftRegisterOutput;
        }
        if (ShiftLoad && (prevClock && !Clock)) {
            ShiftRegister <<= 1;
            ShiftRegisterOutput = ShiftRegister & 0x80;
            qWarning()<<"Shift Load : "<<ShiftRegisterOutput;
        }
        if (!ShiftRegisterClear) {
            ShiftRegister = 0x00;
        }
    }
    prevClearCounter = ClearCounter;
    prevPowerOn = PowerOn;
    prevShiftRegisterClear = ShiftRegisterClear;
    prevClock = Clock;
    prevShiftLoad = ShiftLoad;

    pCONNECTOR->Set_pin(5,ShiftRegisterOutput);


    pCONNECTOR_value = pCONNECTOR->Get_values();
    return true;
}

bool Chx20RC::SaveSession_File(QXmlStreamWriter *xmlOut)
{

    xmlOut->writeStartElement("session");
        xmlOut->writeAttribute("version", "2.0");

        xmlOut->writeStartElement("slots");

        for (int i=0; i< 32; i++) {
            HX20CARTRIDGE_SLOT *slot = &(((HX20CARTRIDGE*) mem)->slot[i]);

            xmlOut->writeStartElement("slotitem");
            xmlOut->writeAttribute("id",QString("%1").arg(i));
            xmlOut->writeAttribute("status",QString("%1").arg(slot->name[0],2,16,QChar('0')));
            xmlOut->writeAttribute("name",QString(QByteArray((char*)slot->name,8)));
            xmlOut->writeAttribute("ext",QString(QByteArray((char*)slot->ext,3)));
            xmlOut->writeAttribute("progtype",QString("%1").arg(slot->progType,2,16,QChar('0')));
            xmlOut->writeAttribute("datatype",QString("%1").arg(slot->dataType,2,16,QChar('0')));
            xmlOut->writeAttribute("startadr",QString(QByteArray((char*)slot->start,4)));
            xmlOut->writeAttribute("endadr",QString(QByteArray((char*)slot->end,4)));
            xmlOut->writeAttribute("date",QString(QByteArray((char*)slot->creationDate,6)));
            xmlOut->writeEndElement();  // slotitem
        }
        xmlOut->writeEndElement();  // slots
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

bool Chx20RC::LoadSession_File(QXmlStreamReader *xmlIn)
{
    qWarning()<<"LoadSession_File";
    if (xmlIn->name()=="session") {
        xmlIn->readNextStartElement();
        if ( xmlIn->name() == "slots" ) {
            qWarning()<<"slots";
            xmlIn->skipCurrentElement();
            xmlIn->readNextStartElement();
        }
        if (xmlIn->name() == "memory" ) {
            AddLog(LOG_MASTER,"Load Memory");

            qWarning()<<"Load Memory";
            for (int s=0; s<SlotList.size(); s++)                               // Save Memory
            {
                switch (SlotList[s].getType()) {
                case CSlot::RAM:
                case CSlot::CUSTOM_ROM:
                    qWarning()<<"CUSTOM_ROM";
                    AddLog(LOG_MASTER,"    Load Slot"+SlotList[s].getLabel());
                    Mem_Load(xmlIn,s); break;
                default: break;
                }
            }
            xmlIn->skipCurrentElement();
        }
    }
    return true;
}

void Chx20RC::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu= new QMenu(this);

    BuildContextMenu(menu);

    menu->addSeparator();

    menu->addAction(tr("Show EPROM manager"),this,SLOT(ShowEM()));
    menu->addAction(tr("Hide EPROM manager"),this,SLOT(HideEM()));

    menu->popup(event->globalPos () );
    event->accept();
}

void Chx20RC::mouseDoubleClickEvent(QMouseEvent *)
{
    ShowEM();
}

void Chx20RC::ShowEM(void)
{
    if(EMView==0) {
        EMView = new QQuickWidget();
        EMView->rootContext()->setContextProperty("hx20rc", this);
        EMView->setSource(QUrl("qrc:/hx20rc.qml"));
        EMView->setResizeMode(QQuickWidget::SizeRootObjectToView);
        connect((QObject*) EMView->rootObject(), SIGNAL(close()), this,SLOT(closeQuick()));

        QVBoxLayout *windowLayout = new QVBoxLayout(mainwindow->centralwidget);
        windowLayout->addWidget(EMView);
        windowLayout->setMargin(0);
    }
    else {
        EMView->show();
    }

    EMView->raise();
}

void Chx20RC::HideEM(void) {
    if(EMView) EMView->hide();
}

void Chx20RC::closeQuick()
{
    if(EMView) EMView->hide();
}

QString Chx20RC::serializeEprom() {
    QString s;
    QXmlStreamWriter *xml = new QXmlStreamWriter(&s);

    xml->writeStartElement("slots");

    for (int i=0; i< 32; i++) {
        HX20CARTRIDGE_SLOT *slot = &(((HX20CARTRIDGE*) mem)->slot[i]);

        xml->writeStartElement("slotitem");
        xml->writeAttribute("id",QString("%1").arg(i));
        xml->writeAttribute("status",QString("%1").arg(slot->name[0],2,16,QChar('0')));
        xml->writeAttribute("name",QString(QByteArray((char*)slot->name,8)));
        xml->writeAttribute("ext",QString(QByteArray((char*)slot->ext,3)));
        xml->writeAttribute("progtype",QString("%1").arg(slot->progType,2,16,QChar('0')));
        xml->writeAttribute("datatype",QString("%1").arg(slot->dataType,2,16,QChar('0')));
        xml->writeAttribute("startadr",QString(QByteArray((char*)slot->start,4)));
        xml->writeAttribute("endadr",QString(QByteArray((char*)slot->end,4)));
        xml->writeAttribute("date",QString(QByteArray((char*)slot->creationDate,6)));
        xml->writeEndElement();  // slotitem
    }
    Mem_Save(xml,0);

    xml->writeEndElement();  // slots

    return s;
}
