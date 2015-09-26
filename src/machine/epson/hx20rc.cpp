

#include <QDebug>

#include "hx20rc.h"
#include "Connect.h"
#include "Keyb.h"
#include "Inter.h"
#include "Log.h"
#include "watchpoint.h"

Chx20RC::Chx20RC(CPObject *parent ):CPObject(parent)
{
    setDXmm(42);
    setDYmm(54);
    setDZmm(30);

    setDX(230);
    setDY(330);

    BackGroundFname	= P_RES(":/hx20/h20rc.png");
    setcfgfname(QString("H20RC"));

    InitMemValue = 0x00;
    memsize      = 0x8000;
    SlotList.clear();
    SlotList.append(CSlot(32, 0x0000 , "" , ""        , CSlot::RAM , "ROM 32KB"));

    ShiftRegisterOutput=false;
    ClearCounter=prevClearCounter=false;
    PowerOn=prevPowerOn=false;
    ShiftRegisterClear=prevShiftRegisterClear=false;
    Clock=prevClock=false;
    ShiftLoad=prevShiftLoad=false;
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

    if (!PowerOn) return true;

    if (PowerOn && ! prevPowerOn) {
        Counter = 0;
        qWarning()<<"Power ON";
    }

    if (!ClearCounter) {
        Counter=0;
//        qWarning()<<"Clear Counter";
    }

    if (prevShiftLoad && !ShiftLoad) {
        Counter++;
        qWarning()<<"Counter ++";
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

    prevClearCounter = pCONNECTOR->Get_pin(9);
    prevPowerOn = pCONNECTOR->Get_pin(8);
    prevShiftRegisterClear = pCONNECTOR->Get_pin(7);
    prevClock = pCONNECTOR->Get_pin(4);
    prevShiftLoad = pCONNECTOR->Get_pin(3);

    pCONNECTOR->Set_pin(5,ShiftRegisterOutput);


    pCONNECTOR_value = pCONNECTOR->Get_values();
    return true;
}

bool Chx20RC::SaveSession_File(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("session");
        xmlOut->writeAttribute("version", "2.0");
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
    if (xmlIn->name()=="session") {

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


