
#include <QPainter>
#include <QResource>

#include "common.h"
#include "ce162e.h"
#include "buspc1500.h"
#include "cpu.h"

Cce162e::Cce162e(CPObject *parent):CPObject(parent)
{
    Q_UNUSED(parent)

    //[constructor]
    BackGroundFname	= P_RES(":/pc1500/ce162e.png");
    setcfgfname(QString("ce162e"));

    setDXmm(140);
    setDYmm(90);
    setDZmm(26);

    setDX(485);
    setDY(299);


    pTIMER		= new Ctimer(this);

    pLH5810_1   = new CLH5810(this);
    pLH5810_2   = new CLH5810(this);
//    pKEYB		= new Ckeyb(this,"ce150.map");

    bus = new CbusPc1500();

    memsize			= 0x2000;

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	P_RES(":/pc1500/ce162e.bin")	, "" , CSlot::ROM , "CE-162E ROM"));

}

Cce162e::~Cce162e() {
    delete pCONNECTOR;
    delete bus;
    delete pLH5810_1;
    delete pLH5810_2;

}

bool Cce162e::init(void)
{
    CPObject::init();

    setfrequency( 0);
    pCONNECTOR	= new Cconnector(this,60,0,Cconnector::Sharp_60,"Connector 60 pins",true,QPoint(388,72));	publish(pCONNECTOR);

    if(pKEYB)	pKEYB->init();
    if(pTIMER)	pTIMER->init();
    pLH5810_1->init();
    pLH5810_2->init();

    return(1);

}

bool Cce162e::run(void)
{
    bus->fromUInt64(pCONNECTOR->Get_values());

    quint16 addr = bus->getAddr();

    if (bus->isEnable() &&
        !bus->isME1() &&
//        !bus->isPV() &&
        !bus->isWrite() &&
        (addr >=0xA000) && (addr < 0xC000) )
    {
        bus->setData(mem[addr - 0xA000]);
//        qWarning()<<"Read ROM "<<QString("%1").arg(addr,4,16,QChar('0'))<<"="<<QString("%1").arg(bus->getData(),2,16,QChar('0'));

        bus->setEnable(false);
        pCONNECTOR->Set_values(bus->toUInt64());
        return true;
    }

    if (bus->isEnable() &&
        bus->isME1() &&
        (addr >= 0x8000) &&
        (addr <= 0x800f) &&
        (bus->isWrite()) )
    {
        qWarning()<<"Write "<<QString("%1").arg(addr,4,16,QChar('0'))<<"="<<QString("%1").arg(bus->getData(),2,16,QChar('0'));
        lh5810_1_write();
    }
    if (bus->isEnable() &&
        bus->isME1() &&
        (addr >= 0xB000) &&
        (addr <= 0xB00f) &&
        (bus->isWrite()) )
    {
        qWarning()<<"Write "<<QString("%1").arg(addr,4,16,QChar('0'))<<"="<<QString("%1").arg(bus->getData(),2,16,QChar('0'));
        lh5810_2_write();
    }


    ////////////////////////////////////////////////////////////////////
    //	RMT ON/OFF
    ////////////////////////////////////////////////////////////////////
    // Service manual PA 1234. Take a look
//    if (pLH5810->lh5810.r_opa & 0x02)	((Cpc15XX *)pPC->pTIMER->pPC)->pce152->paused = false;	// RMT 0 ON
//    if (pLH5810->lh5810.r_opa & 0x04)	((Cpc15XX *)pPC->pTIMER->pPC)->pce152->paused = true;	// RMT 0 OFF
//    pLH5810_2->SetRegBit(CLH5810::OPB,0,true);
//    pLH5810_2->SetRegBit(CLH5810::OPB,1,true);
//    pLH5810_2->SetRegBit(CLH5810::OPB,2,true);
    pLH5810_2->SetRegBit(CLH5810::OPB,3,false);
//    pLH5810_2->SetRegBit(CLH5810::OPB,4,true);
//    pLH5810_2->SetRegBit(CLH5810::OPB,5,true);
//    pLH5810_2->SetRegBit(CLH5810::OPB,6,true);
    pLH5810_2->SetRegBit(CLH5810::OPB,7,true);  // Centronic BUSY ???

    pLH5810_1->step();
    pLH5810_2->step();

//    bus->setINT(pLH5810->INT);

    if (bus->isEnable() &&
            !bus->isWrite() &&
            bus->isME1() &&
            (addr >= 0x8000) &&
            (addr <= 0x800f)) {
        lh5810_1_read();
        qWarning()<<"Read "<<QString("%1").arg(addr,4,16,QChar('0'))<<"="<<QString("%1").arg(bus->getData(),2,16,QChar('0'));

    }
    if (bus->isEnable() &&
            !bus->isWrite() &&
            bus->isME1() &&
            (addr >= 0xB000) &&
            (addr <= 0xB00f)) {
        lh5810_2_read();
        qWarning()<<"Read "<<QString("%1").arg(addr,4,16,QChar('0'))<<"="<<QString("%1").arg(bus->getData(),2,16,QChar('0'))<<"("<<QString("%1").arg(pTIMER->pPC->pCPU->get_PC(),4,16,QChar('0'));
    }

    bus->setEnable(false);
    pCONNECTOR->Set_values(bus->toUInt64());

    return true;
}


bool Cce162e::exit(void)
{

    CPObject::exit();

    return true;
}


INLINE bool Cce162e::lh5810_1_write(void)
{
    switch (bus->getAddr()) {
    case 0x8008: pLH5810_1->SetReg(CLH5810::OPC,bus->getData()); break;
    case 0x8009: pLH5810_1->SetReg(CLH5810::G  ,bus->getData()); break;
    case 0x800A: pLH5810_1->SetReg(CLH5810::MSK,bus->getData()); break;
    case 0x800B: pLH5810_1->SetReg(CLH5810::IF ,bus->getData()); break;
    case 0x800C: pLH5810_1->SetReg(CLH5810::DDA,bus->getData()); break;
    case 0x800D: pLH5810_1->SetReg(CLH5810::DDB,bus->getData()); break;
    case 0x800E: pLH5810_1->SetReg(CLH5810::OPA,bus->getData()); break;
    case 0x800F: pLH5810_1->SetReg(CLH5810::OPB,bus->getData()); break;
    default:  break;
    }

    return true;
}
INLINE bool Cce162e::lh5810_2_write(void)
{
    switch (bus->getAddr()) {
    case 0xB008: pLH5810_2->SetReg(CLH5810::OPC,bus->getData()); break;
    case 0xB009: pLH5810_2->SetReg(CLH5810::G  ,bus->getData()); break;
    case 0xB00A: pLH5810_2->SetReg(CLH5810::MSK,bus->getData()); break;
    case 0xB00B: pLH5810_2->SetReg(CLH5810::IF ,bus->getData()); break;
    case 0xB00C: pLH5810_2->SetReg(CLH5810::DDA,bus->getData()); break;
    case 0xB00D: pLH5810_2->SetReg(CLH5810::DDB,bus->getData()); break;
    case 0xB00E: pLH5810_2->SetReg(CLH5810::OPA,bus->getData()); break;
    case 0xB00F: pLH5810_2->SetReg(CLH5810::OPB,bus->getData()); break;
    default:  break;
    }

    return true;
}

INLINE bool Cce162e::lh5810_1_read(void)
{
    switch (bus->getAddr()) {
    case 0x8008: bus->setData( pLH5810_1->GetReg(CLH5810::OPC) ); break;
    case 0x8009: bus->setData( pLH5810_1->GetReg(CLH5810::G)); break;
    case 0x800A: bus->setData( pLH5810_1->GetReg(CLH5810::MSK)); break;
    case 0x800B: bus->setData( pLH5810_1->GetReg(CLH5810::IF)); break;
    case 0x800C: bus->setData( pLH5810_1->GetReg(CLH5810::DDA)); break;
    case 0x800D: bus->setData( pLH5810_1->GetReg(CLH5810::DDB)); break;
    case 0x800E: bus->setData( pLH5810_1->GetReg(CLH5810::OPA)); break;
    case 0x800F: bus->setData( pLH5810_1->GetReg(CLH5810::OPB)); break;
    default: /*bus->setData(0x00);*/ break;
    }

    return true;
}

INLINE bool Cce162e::lh5810_2_read(void)
{
    switch (bus->getAddr()) {
    case 0xB008: bus->setData( pLH5810_2->GetReg(CLH5810::OPC) ); break;
    case 0xB009: bus->setData( pLH5810_2->GetReg(CLH5810::G)); break;
    case 0xB00A: bus->setData( pLH5810_2->GetReg(CLH5810::MSK)); break;
    case 0xB00B: bus->setData( pLH5810_2->GetReg(CLH5810::IF)); break;
    case 0xB00C: bus->setData( pLH5810_2->GetReg(CLH5810::DDA)); break;
    case 0xB00D: bus->setData( pLH5810_2->GetReg(CLH5810::DDB)); break;
    case 0xB00E: bus->setData( pLH5810_2->GetReg(CLH5810::OPA)); break;
    case 0xB00F: bus->setData( pLH5810_2->GetReg(CLH5810::OPB)); break;
    default: /*bus->setData(0x00);*/ break;
    }

    return true;
}


