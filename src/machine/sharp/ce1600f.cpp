/*
 * Specification of FDU 250
1) Memory capacity: -
    64KB (512 Bytes/sector, 8 sectors/track)
2) Recording method:
    GCR (4/5) -
3) Transfer speed:
    250K bits (25K Bytes/sec)
4) Track density:
    48TPI
5) Total tracks:
    16
6) Revolutions:
    270 rpm
7) Access time:
    One step 80 milliseconds from track 00 to track 15.
    170 milliseconds to restore from track 15 to track 00.
    Settling time:
    50 milliseconds
8) Motor startup time:
    0.5 second
NOTE: GCR is an abbreviation of of Group Coded Recording.
A single byte, 8 bits, data are divided into
two 4-bit data whicis is also converted onto a 5-bit
data. Tisus, s single byte (8 bits) is recorded on tise
media as a 10-bit data.
 */

#include <QDebug>

#include "ce1600f.h"
#include "Inter.h"
#include "Connect.h"
#include "Log.h"
#include "Keyb.h"
#include "cpu.h"
#include "buspc1500.h"


Cce1600f::Cce1600f(CPObject *parent):CPObject(parent)
{
    BackGroundFname	= P_RES(":/ext/ce-1600f.png");

    setcfgfname(QString("ce1600f"));

    setDXmm(95);
    setDYmm(120);
    setDZmm(39);

    setDX(339);
    setDY(428);

    delete pKEYB;
    pKEYB		= new Ckeyb(this,"ce1600f.map",0);

    bus = new CbusPc1500();

    error = false;
    ready = true;
    ChangedDisk = false;
    busy = false;
    ack = false;

    isDisk = true;
    writeProtect = false;
    motorRunning = false;

    sector = 0;
    startMotorState = 0;
    countWrite = 0;

    memset(data,0,sizeof(data));
}

Cce1600f::~Cce1600f()
{

}

bool Cce1600f::init()
{
    CPObject::init();
    setfrequency( 0);
    pCONNECTOR	   = new Cconnector(this,50,0,Cconnector::Sharp_50,"Connector 50 pins",false,QPoint(339,225),Cconnector::EAST);	publish(pCONNECTOR);
    WatchPoint.add(&pCONNECTOR_value,64,50,this,"50 pins connector");

    AddLog(LOG_PRINTER,tr("CE-1600F initializing..."));

    if(pKEYB)	pKEYB->init();
    if(pTIMER)	pTIMER->init();

    return true;
}

bool Cce1600f::exit()
{

}

// (40 = Read, Write = 60, A0 = format)
void Cce1600f::fddCmd(BYTE _data)
{
    CpcXXXX * _pc = pTIMER->pPC;
    qWarning()<<"FDD - fddCmd:"<<QString("%1").arg(_data,2,16,QChar('0'))
                <<QString("%1").arg(pTIMER->pPC->pCPU->get_PC(),4,16,QChar('0'));
#if 0
    pTIMER->pPC->pCPU->logsw = true;
    pTIMER->pPC->pCPU->Check_Log();
#endif
    currentCMD = _data;
    switch(_data) {
    case 0x01:
        ack = false;
        offset = 0;
        break;
    case 0x20:
        ack = true;
        offset = 0;
        break;
    case 0x40:
        qWarning()<<"READ";
        busy = true;
        break;
    case 0x60:
        qWarning()<<"WRITE";
        error = false;
        break;
    case 0x80:
        ack = true;
//        pTIMER->pPC->pCPU->logsw = true;
//        pTIMER->pPC->pCPU->Check_Log();
        break;
    case 0xA0:
        qWarning()<<"format";
        offset=0;
        pTIMER->pPC->pCPU->logsw = true;
        pTIMER->pPC->pCPU->Check_Log();
        busy = true;
        ack = false;
        break;
    default:
//        qWarning()<<"ERROR:"<<QString("%1").arg(_data,2,16,QChar('0'));
        ack = false;
        break;
    }
}

void Cce1600f::fddSetSector(BYTE _data)
{
    qWarning()<<QString("FDD - fddSetSector:%1").arg(_data,2,16,QChar('0'));

    sector = _data;
    offset = 0;
}

BYTE Cce1600f::fddGetSector()
{
    qWarning()<<QString("FDD - fddGetSector:%1").arg(sector,2,16,QChar('0'));

    return sector;
}

void Cce1600f::fddSetMotor(BYTE _data)
{
    qWarning()<<QString("FDD - fddSetMotor:%1").arg(_data,2,16,QChar('0'))<<" running:"<<motorRunning;
    if (_data & 0x80) {
        if (!motorRunning) {
            // Start timer and set motorRunning after 500ms
            qWarning()<<"Start TIMER"<<pTIMER;
            startMotorState = pTIMER->state;
            busy = true;
        }
    }
    else {
        motorRunning = false;
        startMotorState = 0;
    }
}

void Cce1600f::fddWriteData(BYTE _data)
{
    qWarning()<<QString("FDD - Write (%1):%2").arg(offset).arg(_data,2,16,QChar('0'));

    switch (currentCMD) {
    case 0x01:
        checksum = _data;
        ack = true;
        break;
    case 0xA0:
        data[sector][offset] = _data;
        checksum += _data;
        offset++;
        if (offset==24) {
            ack = true;
        }
        break;
    }

}

BYTE Cce1600f::fddReadData()
{
    BYTE _data = 0;
    qWarning()<<QString("FDD - Read (%1):%2").arg(offset).arg(_data,2,16,QChar('0'));

    switch(currentCMD) {
    case 0x40:
        _data = data[sector][offset];
        offset++;
        if (offset==24) {
            ack = true;
        }
    }


    return _data;
}


//78r: Motor and disk status HB
//    b7: Engine not started
//    b6: no write protection
//    b3: disk in the drive
//7Ar: Status LB
//        b7: busy ????
//        b6: changed Disk
//        b1: Ready
//        b0: Error (inverted)
WORD Cce1600f::fddStatus()
{
    WORD _val = 0;

//    _val |= error       ? 0x00 : 0x01;
    _val |= ack         ? 0x00 : 0x01;
    _val |= ready       ? 0x02 : 0x00;
    _val |= ChangedDisk ? 0x40 : 0x00;
    _val |= busy        ? 0x80 : 0x00;

    _val |= isDisk          ? 0x0800 : 0x00;
    _val |= (motorRunning&&!writeProtect)   ? 0x4000 : 0x00;
    _val |= !motorRunning   ? 0x8000 : 0x00;

//    qWarning()<<"fddStatus:"<<QString("%1").arg(_val,2,16,QChar('0'));
    return _val;
}

void Cce1600f::fddwrite(BYTE _offset,BYTE _data) {
    switch (_offset & 0x07) {
    case 0x00: // 78w: command (40 = Read, Write = 60, A0 = format)
        qWarning()<<((_offset & 0x08) ? "X: ":"Y: ");
        fddCmd(_data);
        break;
    case 0x01: // 79w: define Sector
        qWarning()<<((_offset & 0x08) ? "X: ":"Y: ");
        fddSetSector(_data);
        break;
    case 0x02: // 7Aw: Motor b7: motor on
        qWarning()<<((_offset & 0x08) ? "X: ":"Y: ");
        fddSetMotor(_data );
        break;
    case 0x03: // 7B: write data
        qWarning()<<((_offset & 0x08) ? "X: ":"Y: ");
        fddWriteData(_data);
        break;
    }
}

BYTE Cce1600f::fddRead(BYTE _offset) {
    BYTE _val = 0;
    switch (_offset & 0x07) {
    case 0x00:
//                78r: Motor and disk status
//                    b7: Engine not started
//                    b6: no write protection
//                    b3: disk in the drive
        _val = fddStatus() >> 8;
        qWarning()<<((_offset & 0x08) ? "X: ":"Y: ")
                  <<QString("read [%1] = %2 - ").arg(_offset,2,16,QChar('0')).arg(_val,2,16,QChar('0'))
                  <<QString("%1").arg(pTIMER->pPC->pCPU->get_PC(),4,16,QChar('0'));
        break;
    case 0x01: // 79r: Read current Sector
        qWarning()<<((_offset & 0x08) ? "X: ":"Y: ");
        _val = fddGetSector();
        break;
    case 0x02:
//                    7Ar: Status
//                            b7: busy ????
//                            b6: changed Disk
//                            b1: Ready
//                            b0: Error (inverted)
        _val = fddStatus() & 0xff;
        qWarning()<<((_offset & 0x08) ? "X: ":"Y: ")
                  <<QString("read [%1] = %2 - ").arg(_offset,2,16,QChar('0')).arg(_val,2,16,QChar('0'))
                  <<QString("%1").arg(pTIMER->pPC->pCPU->get_PC(),4,16,QChar('0'));
        break;
    case 0x03: // 7B: read data
        qWarning()<<((_offset & 0x08) ? "X: ":"Y: ");
        _val = fddReadData();
        break;
    default: _val = 0xff; break;
    }

    return _val;
}

void Cce1600f::fddReset()
{
    qWarning()<<"FDD - RESET";
    error = false;
    ready = true;
    ChangedDisk = false;
    busy = false;

    isDisk = true;
    writeProtect = false;
    motorRunning = false;

    sector = 0;
    startMotorState = 0;
    countWrite = 0;
}

bool Cce1600f::run()
{
    bool keyEvent = false;



    if ((startMotorState>0) && pTIMER) {
        if (pTIMER->msElapsed(startMotorState)>500) {
            qWarning()<<"MOTOR RUNNING";
            motorRunning = true;
            startMotorState = 0;
            busy = false;
        }
    }

    bus->fromUInt64(pCONNECTOR->Get_values());



    ////////////////////////////////////////////////////////////////////
    //	PRINT MODE
    //////////////////////////////////////////////////////////////////
//    if (pKEYB->LastKey==K_PRINT_ON)
//    {
//        Print_Mode = true;
//        pKEYB->LastKey = 0;
//        keyEvent = true;
//    }
//    if (pKEYB->LastKey==K_PRINT_OFF)
//    {
//        Print_Mode = false;
//        pKEYB->LastKey = 0;
//        keyEvent = true;
//    }
//    if (pKEYB->LastKey)
//    {
//        pKEYB->LastKey = 0;
//        keyEvent = true;
//    }

    ////////////////////////////////////////////////////////////////////
    //	RMT ON/OFF
    ////////////////////////////////////////////////////////////////////
//	if (pLH5810->lh5810.r_opa & 0x02)	((Cpc15XX *)pPC->pTIMER->pPC)->pce152->paused = FALSE;	// RMT 0 ON
//	if (pLH5810->lh5810.r_opa & 0x04)	((Cpc15XX *)pPC->pTIMER->pPC)->pce152->paused = TRUE;	// RMT 0 OFF


    if (!bus->isEnable()) {
        if (keyEvent) {
            qWarning()<<"send INT";
            bus->setINT(true);
            pCONNECTOR->Set_values(bus->toUInt64());
        }
        return true;
    }

    quint16 addr = bus->getAddr();

    if (bus->isEnable() &&
        bus->isME1() &&
        bus->isWrite() &&
        (addr >= 0x80) && (addr <= 0x83) )
    {
        BYTE _data = bus->getData();
        switch (addr) {
        case 0x81: if ( !(_data & 0x01)) {
                fddReset();
            }
            break;
        }

    }

    if (bus->isEnable() &&
        bus->isME1() &&
        bus->isWrite() &&
        (addr >= 0x78) && (addr <= 0x7F) )
    {
        BYTE _data = bus->getData();

        fddwrite(addr,_data);
    }

    if (bus->isEnable() &&
        bus->isME1() &&
        bus->isWrite() &&
        (addr >= 0x70) && (addr <= 0x77) )
    {
        fddwrite(addr,bus->getData());
       qWarning()<<QString("write [%1]=%2").arg(addr,4,16,QChar('0')).arg(bus->getData(),2,16,QChar('0'));

    }


    /*
     *70-77: Floppy II, otherwise known as 78-7F
    78-7F; floppy I
    78w: command (40 = Read, Write = 60, A0 = format)
    78r: Motor and disk status
            b7: Engine not started
            b6: no write protection
            b3: disk in the drive
    79w: define Sector
    79r: Read current Sector

    7Aw: Motor b7: motor on
    7Ar: Status
            b7: busy ????
            b6: changed Disk
            b1: Ready
            b0: Error (inverted)
    7B: read / write data
    7C-7F: unused
    */

    if (bus->isEnable() &&
        bus->isME1() &&
        !bus->isWrite() &&
        (addr >= 0x78) && (addr <= 0x7F) )
    {

        BYTE _val = fddRead(addr);

        bus->setData(_val);
        bus->setEnable(false);
        pCONNECTOR->Set_values(bus->toUInt64());
        return true;
    }

    if (bus->isEnable() &&
        bus->isME1() &&
        !bus->isWrite() &&
        (addr >= 0x70) && (addr <= 0x77) )
    {
        BYTE _val = fddRead(addr);
        qWarning()<<QString("read [%1]").arg(addr,4,16,QChar('0'));

        bus->setData(_val);
        bus->setEnable(false);
        pCONNECTOR->Set_values(bus->toUInt64());
        return true;
    }


    bus->setEnable(false);
    pCONNECTOR->Set_values(bus->toUInt64());

    return(1);
}

