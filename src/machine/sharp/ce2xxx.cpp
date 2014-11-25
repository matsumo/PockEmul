
#include <QDebug>

#include "ce2xxx.h"
#include "Connect.h"
#include "Keyb.h"
#include "bus.h"
#include "Inter.h"
#include "Log.h"

Cce2xxx::Cce2xxx(CPObject *parent ,Models mod):CPObject(parent)
{


    setDXmm(42);
    setDYmm(54);
    setDZmm(3);

    setDX(150);
    setDY(192);

    SlotList.clear();
    InitMemValue = 0x00;

    model = mod;
    switch(model) {
    case CE201M:    // 8KB Wide RAM card
        setDXmm(85);
        setDYmm(54);
        setDZmm(3);

        setDX(303);
        setDY(192);
        BackGroundFname	= P_RES(":/ce2xxx/ce201m.png");
        setcfgfname(QString("ce201m"));
        memsize      = 0x2000;
        SlotList.append(CSlot(8 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 8KB"));
        break;
    case CE202M:    // 16KB Wide RAM card
        setDXmm(85);
        setDYmm(54);
        setDZmm(3);

        setDX(303);
        setDY(192);
        BackGroundFname	= P_RES(":/ce2xxx/ce202m.png");
        setcfgfname(QString("ce202m"));
        memsize      = 0x4000;
        SlotList.append(CSlot(16 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 16KB"));
        break;
    case CE203M:    // 32KB Wide RAM card
        setDXmm(85);
        setDYmm(54);
        setDZmm(3);

        setDX(303);
        setDY(192);
        BackGroundFname	= P_RES(":/ce2xxx/ce203m.png");
        setcfgfname(QString("ce203m"));
        memsize      = 0x8000;
        SlotList.append(CSlot(32 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 32KB"));
        break;
    case CE210M:    // 2KB short RAM card
        BackGroundFname	= P_RES(":/ce2xxx/ce210m.png");
        setcfgfname(QString("ce210m"));
        memsize      = 0x0800;
        SlotList.append(CSlot(2 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 2KB"));
        break;
    case CE211M:    // 4KB short RAM card
        BackGroundFname	= P_RES(":/ce2xxx/ce211m.png");
        setcfgfname(QString("ce211m"));
        memsize      = 0x1000;
        SlotList.append(CSlot(4 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 4KB"));
        break;
    case CE212M:    // 8KB short RAM card
        BackGroundFname	= P_RES(":/ce2xxx/ce212m.png");
        setcfgfname(QString("ce212m"));
        memsize      = 0x2000;
        SlotList.append(CSlot(8 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 8KB"));
        break;
    case CE2H16M:    // 16KB short RAM card
        BackGroundFname	= P_RES(":/ce2xxx/ce2h16m.png");
        setcfgfname(QString("ce2h16m"));
        memsize      = 0x4000;
        SlotList.append(CSlot(16 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 16KB"));
        break;
    case CE2H32M:    // 32KB short RAM card
        BackGroundFname	= P_RES(":/ce2xxx/ce2h32m.png");
        setcfgfname(QString("ce2h32m"));
        memsize      = 0x8000;
        SlotList.append(CSlot(32 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 32KB"));
        break;
    case CE2H64M:    // 64KB short RAM card
        BackGroundFname	= P_RES(":/ce2xxx/ce2h64m.png");
        setcfgfname(QString("ce2h64m"));
        memsize      = 0x10000;
        SlotList.append(CSlot(64 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 64KB"));
        break;
    default: break;
    }

    bus = new Cbus();
}

Cce2xxx::~Cce2xxx()
{

}

bool Cce2xxx::init()
{
    CPObject::init();
    setfrequency( 0);

    pCONNECTOR = new Cconnector(this,35,3,Cconnector::Sharp_35,"Memory connector",false,QPoint(0,90));	publish(pCONNECTOR);

    if(pKEYB)	pKEYB->init();
    if(pTIMER)	pTIMER->init();

    return true;
}

bool Cce2xxx::exit()
{
    return true;
}

bool Cce2xxx::run()
{

    bus->fromUInt64(pCONNECTOR->Get_values());

    if (!bus->isEnable()) {
        return true;
    }

    quint16 addr = bus->getAddr() & (memsize - 1);

    if (bus->isWrite()) {
//        qWarning()<<QString("Write [%1]=%2").arg(addr,4,16,QChar('0')).arg(bus->getData(),2,16,QChar('0'));
        mem[addr] = bus->getData();
    }
    else {
        bus->setData(mem[addr]);
//        qWarning()<<QString("Read [%1]=%2").arg(addr,4,16,QChar('0')).arg(bus->getData(),2,16,QChar('0'));
    }

    bus->setEnable(false);
    pCONNECTOR->Set_values(bus->toUInt64());

    return true;
}

bool Cce2xxx::SaveSession_File(QXmlStreamWriter *xmlOut)
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

bool Cce2xxx::LoadSession_File(QXmlStreamReader *xmlIn)
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
