

#include <QDebug>

#include "cemem.h"
#include "Connect.h"
#include "Keyb.h"
#include "buspc1500.h"
#include "Inter.h"
#include "Log.h"

Ccemem::Ccemem(CPObject *parent ,Models mod):CPObject(parent)
{


    setDXmm(42);
    setDYmm(54);
    setDZmm(3);

    setDX(150);
    setDY(192);

    SlotList.clear();
    InitMemValue = 0x00;

    setDXmm(33);
    setDYmm(40);
    setDZmm(7);

    setDX(303);
    setDY(192);

    model = mod;
    switch(model) {
    case CE151:    // 4KB RAM module
        BackGroundFname	= P_RES(":/cemem/ce151.png");
        setcfgfname(QString("ce151"));
        memsize      = 0x1000;
        SlotList.append(CSlot(4 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 4KB"));
        break;
    case CE155:    // 8KB RAM module
        BackGroundFname	= P_RES(":/cemem/ce155.png");
        setcfgfname(QString("ce155"));
        memsize      = 0x2000;
        SlotList.append(CSlot(8 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 8KB"));
        break;
    case CE160:    // 16KB RAM module
        setDXmm(41);
        setDYmm(48);
        setDZmm(9);

        setDX(303);
        setDY(192);
        BackGroundFname	= P_RES(":/cemem/ce160.png");
        setcfgfname(QString("ce160"));
        memsize      = 0x4000;
        SlotList.append(CSlot(16 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 16KB"));
        break;
    case CE163:    // 32KB RAM module
        setDXmm(41);
        setDYmm(48);
        setDZmm(9);

        setDX(303);
        setDY(192);
        BackGroundFname	= P_RES(":/cemem/ce163.png");
        setcfgfname(QString("ce163"));
        memsize      = 0x8000;
        SlotList.append(CSlot(32 , 0x0000 , "" , ""        , CSlot::RAM , "RAM 32KB"));
        break;
    default: break;
    }

    bus1500 = new CbusPc1500();
}

Ccemem::~Ccemem()
{

}

bool Ccemem::init()
{
    CPObject::init();
    setfrequency( 0);

    pCONNECTOR = new Cconnector(this,40,3,Cconnector::Sharp_40,"Memory connector",false,QPoint(0,90));	publish(pCONNECTOR);

    if(pKEYB)	pKEYB->init();
    if(pTIMER)	pTIMER->init();

    return true;
}

bool Ccemem::exit()
{
    return true;
}

bool Ccemem::run()
{

    bus1500->fromUInt64(pCONNECTOR->Get_values());

    if (!bus1500->isEnable()) {
        return true;
    }

    quint16 addr = bus1500->getAddr();

    if ( (model == CE151) &&
         (addr >=0x4800) && (addr <=0x57FF) ) {
        if (bus1500->isWrite()) {
            //        qWarning()<<QString("Write [%1]=%2").arg(addr,4,16,QChar('0')).arg(bus->getData(),2,16,QChar('0'));
            mem[addr - 0x4800] = bus1500->getData();
        }
        else {
            bus1500->setData(mem[addr - 0x4800]);
            //        qWarning()<<QString("Read [%1]=%2").arg(addr,4,16,QChar('0')).arg(bus->getData(),2,16,QChar('0'));
        }
    }

    if (model == CE155) {
        if ((addr >=0x3800) && (addr <=0x4FFF) ) {
            if (bus1500->isWrite()) {
                mem[addr - 0x3800] = bus1500->getData();
            }
            else {
                bus1500->setData(mem[addr - 0x3800]);
            }
        }
        if ((addr >=0x4800) && (addr <=0x5FFF) ) {
            if (bus1500->isWrite()) {
                mem[addr - 0x4800 + 0x800] = bus1500->getData();
            }
            else {
                bus1500->setData(mem[addr - 0x4800 + 0x800]);
            }
        }
    }

    bus1500->setEnable(false);
    pCONNECTOR->Set_values(bus1500->toUInt64());

    return true;
}

bool Ccemem::SaveSession_File(QXmlStreamWriter *xmlOut)
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

bool Ccemem::LoadSession_File(QXmlStreamReader *xmlIn)
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

