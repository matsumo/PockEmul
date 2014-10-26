#include <QPainter>
#include <QResource>

#include "common.h"
#include "ce150.h"
#include "ce1600p.h"
#include "cpu.h"
#include "pcxxxx.h"
#include "paperwidget.h"
#include "Keyb.h"
#include "clink.h"
#include "dialoganalog.h"
#include "buspc1500.h"

#define NO_MOVE	0
#define RI_MOVE	1
#define LE_MOVE 2
#define RI_MOVE_MID	3
#define LE_MOVE_MID 4
#define UP_MOVE 1
#define DN_MOVE 2
#define UP_MOVE_MID 3
#define DN_MOVE_MID 4

#define PORT_MOTOR_X (pTIMER->pPC->pCPU->imem[0x83] & 0x0f)
#define PORT_MOTOR_Y ((pTIMER->pPC->pCPU->imem[0x83] >> 4) & 0x0f)
#define PORT_MOTOR_Z (pTIMER->pPC->pCPU->imem[0x82] & 0x0f)

//#define MACRO_ADD_LOG	AddLog(LOG_PRINTER,tr("X=%1 Y=%2 Rot=%3 Color=%4,   IF=%5").arg(Pen_X,Pen_Y,Rot,Pen_Color,pLH5810->lh5810.r_if))
#define MACRO_ADD_LOG	if (pTIMER->pPC->fp_log) fprintf(pTIMER->pPC->fp_log,"X=%d Y=%d Z=%d Rot=%d Color=%d\n",Pen_X,Pen_Y,Pen_Z,Rot,Pen_Color)

Cce1600p::Cce1600p(CPObject *parent) : Cce150(this),fdd(this)
{
    //[constructor]
    BackGroundFname	= P_RES(":/ext/ce-1600p.png");

    setcfgfname(QString("ce1600p"));

    setDXmm(317);
    setDYmm(222);
    setDZmm(48);

    setDX(1100);
    setDY(780);

    delete pKEYB;
    pKEYB		= new Ckeyb(this,"ce1600p.map",0);

    bus = new CbusPc1500();

    memsize			= 0x8000;

    SlotList.clear();
    SlotList.append(CSlot(16, 0x0000 ,P_RES(":/pc1600/romce1600-1.bin"), "" , CSlot::ROM , "ROM"));
    SlotList.append(CSlot(16, 0x4000 ,P_RES(":/pc1600/romce1600-2.bin"), "" , CSlot::ROM , "ROM"));

    ccKeyInt = pfKeyInt = revpfKeyInt = fdKeyInt = swKeyInt = crKeyInt= false;

}

bool Cce1600p::init(void)
{

        CPObject::init();

    #if 0
        QResource res(P_RES(":/ext/clac2.wav"));
        clac = FSOUND_Sample_Load(FSOUND_FREE, (const char*) res.data(), FSOUND_LOADMEMORY, 0, res.size());
    #endif

        setfrequency( 0);

        pCONNECTOR	= new Cconnector(this,60,0,Cconnector::Sharp_60,"Connector 60 pins",true,QPoint(424,536));	publish(pCONNECTOR);
        pEXTCONNECTOR= new Cconnector(this,60,1,Cconnector::Sharp_60,"Connector 60 pins Ext",false,QPoint(565,0),Cconnector::NORTH);	publish(pEXTCONNECTOR);
        pTAPECONNECTOR= new Cconnector(this,3,2,Cconnector::Jack,"Line in / Rec / Rmt",false);	publish(pTAPECONNECTOR);

        WatchPoint.add(&pTAPECONNECTOR_value,64,2,this,"Line In / Rec");
        WatchPoint.add(&pCONNECTOR_value,64,60,this,"Standard 60pins connector");

        AddLog(LOG_PRINTER,tr("PRT initializing..."));

        if(pKEYB)	pKEYB->init();
        if(pTIMER)	pTIMER->init();

        // Create CE-150 Paper Image
        ce150buf	= new QImage(QSize(1920, 1000),QImage::Format_ARGB32);
        ce150display= new QImage(QSize(1920, 567),QImage::Format_ARGB32);
        ce150pen	= new QImage(P_RES(":/ext/ce-150pen.png"));
        // Fill it blank
        clearPaper();

        // Create a paper widget
        setPaperPos(QRect(195,30,695,170));
        paperWidget = new CpaperWidget(PaperPos(),ce150buf,this);
        paperWidget->hide();

        initsound();
        Refresh_Display = true;

    return true;
}

/* disk
& 78
bit 3   0: drive is empty
        1: drive contains disk
Bit 6   0: The drive is or the write protection is active
        1: drive is running and write protection is inactive
Bit 7   0: Drive is running
        1: drive is
& 79
Bit 0 The three bits together form the.
Bit 1 number of the currently selected bit 2 Track

7A &
Bit 7 Write:
        0: off drive motor
        1: Drive motor on

*/


bool Cce1600p::run(void)
{
    bool has_moved_X = false;
    bool has_moved_Y = false;
    bool has_moved_Z = false;
    bool forwardBus = true;
    bool keyEvent = false;

    bus->fromUInt64(pCONNECTOR->Get_values());

    ////////////////////////////////////////////////////////////////////
    //	VOLTAGE OK :-)
    //////////////////////////////////////////////////////////////////
//	pLH5810->SetRegBit(LH5810_OPB,6,FALSE);

    ////////////////////////////////////////////////////////////////////
    //	PRINT MODE
    //////////////////////////////////////////////////////////////////
    if (pKEYB->LastKey==K_PRINT_ON)
    {
        Print_Mode = true;
        pKEYB->LastKey = 0;
        keyEvent = true;
    }
    if (pKEYB->LastKey==K_PRINT_OFF)
    {
        Print_Mode = false;
        pKEYB->LastKey = 0;
        keyEvent = true;
    }
    if (pKEYB->LastKey)
    {
        pKEYB->LastKey = 0;
        keyEvent = true;
    }

    ////////////////////////////////////////////////////////////////////
    //	RMT ON/OFF
    ////////////////////////////////////////////////////////////////////
//	if (pLH5810->lh5810.r_opa & 0x02)	((Cpc15XX *)pPC->pTIMER->pPC)->pce152->paused = FALSE;	// RMT 0 ON
//	if (pLH5810->lh5810.r_opa & 0x04)	((Cpc15XX *)pPC->pTIMER->pPC)->pce152->paused = TRUE;	// RMT 0 OFF

#if 1
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
        case 0x80:
            qWarning()<<QString("write [%1]=%2").arg(addr,4,16,QChar('0')).arg(bus->getData(),2,16,QChar('0'));

            ccKeyInt = _data & 0x01;
            pfKeyInt = _data & 0x02;
            revpfKeyInt = _data & 0x04;
            fdKeyInt = _data & 0x08;
            swKeyInt = _data & 0x10;
            crKeyInt = _data & 0x20;
            break;
        case 0x81: if ( !(_data & 0x01)) fdd.fddReset();
            break;
        case 0x82:
            // rmtOn = _data & 10;
            // rmtOff = _data & 20;
            // cmtInEnable = _data & 80;
            motorZ = _data & 0x0f; break;
        case 0x83:
            motorX = _data & 0x0f;
            motorY = (_data >> 4) & 0x0f; break;
        }
        forwardBus = false;
    }
#endif

#if 1   // FDD

    if (bus->isEnable() &&
        bus->isME1() &&
        bus->isWrite() &&
        (addr >= 0x78) && (addr <= 0x7F) )
    {
        BYTE _data = bus->getData();

        fdd.fddwrite(addr,_data);
        forwardBus = false;
    }

    if (bus->isEnable() &&
        bus->isME1() &&
        bus->isWrite() &&
        (addr >= 0x70) && (addr <= 0x77) )
    {
        fdd.fddwrite(addr,bus->getData());
       qWarning()<<QString("write [%1]=%2").arg(addr,4,16,QChar('0')).arg(bus->getData(),2,16,QChar('0'));

    }
#endif


    Direction = Motor_X.SendPhase(motorX);

    switch (Direction)
    {
        case RI_MOVE:   Pen_X++;
        case RI_MOVE_MID:	Pen_X++;
                        has_moved_X=true;
                        MACRO_ADD_LOG;
                        break;
        case LE_MOVE:   Pen_X--;
        case LE_MOVE_MID:   Pen_X--;
                        has_moved_X=true;
                        MACRO_ADD_LOG;
                        break;
    }

    Direction = Motor_Y.SendPhase(motorY);

    switch (Direction)
    {
    case UP_MOVE:       Pen_Y--;
    case UP_MOVE_MID:   Pen_Y--;
                        has_moved_Y=true;
                        MACRO_ADD_LOG;
                        break;
    case DN_MOVE:       Pen_Y++;
    case DN_MOVE_MID:	Pen_Y++;
                        has_moved_Y=true;
                        MACRO_ADD_LOG;
                        break;
    }

    Direction = Motor_Z.SendPhase(motorZ);

    switch (Direction)
    {
    case UP_MOVE:       Pen_Z--;
    case UP_MOVE_MID:   Pen_Z--;
                        has_moved_Z=true;
                        MACRO_ADD_LOG;
                        break;
    case DN_MOVE:       Pen_Z++;
    case DN_MOVE_MID:	Pen_Z++;
                        has_moved_Z=true;
                        MACRO_ADD_LOG;
                        break;
    }

    if (has_moved_Z)
    {
        if (Pen_Z < 0) Pen_Z = 0;
        if (Pen_Z >50) Pen_Z = 50;
        if (Pen_Z == 6) // Pen up
        {
#ifndef NO_SOUND
//            clac->play();
            if (getfrequency()>0) {
                fillSoundBuffer(0xFF);
                fillSoundBuffer(0x00);
            }
            else {
//                int ps = m_audioOutput->periodSize();
                mainwindow->audioMutex.lock();
                QByteArray buff;
                buff.append(0xFF);
                buff.append((char)0);
                m_output->write(buff.constData(),2);
                mainwindow->audioMutex.unlock();
            }
#endif

            Pen_Status = PEN_UP;
            AddLog(LOG_PRINTER,"PEN UP");
        }
        if (Pen_Z == 0) {   // Pen down
#if 0
            int iChanIndex = FSOUND_PlaySoundEx(FSOUND_FREE, clac,0,true);
            FSOUND_SetVolumeAbsolute(iChanIndex,255);
            FSOUND_SetPaused(iChanIndex,false);
#endif
#ifndef NO_SOUND
//            clac->play();
            if (getfrequency()>0) {
                fillSoundBuffer(0xFF);
                fillSoundBuffer(0x00);
            }
            else {
//                int ps = m_audioOutput->periodSize();
                mainwindow->audioMutex.lock();
                QByteArray buff;
                buff.append(0xFF);
                buff.append((char)0);
                m_output->write(buff.constData(),2);
                mainwindow->audioMutex.unlock();
            }
#endif
            Pen_Status = PEN_DOWN;
            AddLog(LOG_PRINTER,"PEN DOWN");
        }

        if (Pen_Z == 20) { // change color position
            StartRot = true;	// rotation pin engaged
        }
        if ((Pen_Z == 50) && StartRot) {
            Rot++;
            StartRot = false;
            // If the third 1/12 rotation, then put next color
            if (Rot == 8) {
                Rot = 0;
                Next_Color();
            }
        }

    }

    //---------------------------------------------------
    // Draw printer
    //---------------------------------------------------
    if (has_moved_Y || (has_moved_X && (Pen_Status==PEN_DOWN))) Print();

    fdd.step();

    if (bus->isEnable() &&
        !bus->isME1() &&
        !bus->isPU() &&
         bus->isPT() &&
        !bus->isWrite() &&
        (addr >=0x4000) && (addr < 0x8000) )
    {
//        qWarning()<<QString("Read ROM[%1]=%2").arg(addr,4,16).arg(mem[addr - (bus->isPV() ? 0:0x4000)],2,16,QChar('0'));
        bus->setData(mem[addr - 0x4000 + (bus->isPV() ? 0x4000 : 0x00)]);

        forwardBus = false;
        bus->setEnable(false);
        pCONNECTOR->Set_values(bus->toUInt64());
        return true;
    }

    // Left position detection
    if (bus->isEnable() &&
        bus->isME1() &&
        !bus->isWrite() &&
        (addr >= 0x80) && (addr <= 0x83) )
    {
        BYTE _val = 0;
        switch (addr) {
        case 0x81:
            _val |= (pKEYB->LastKey==K_PFEED) ? 0x02 : 0x00; //	PAPER FEED
            _val |= (pKEYB->LastKey==K_PBFEED) ? 0x04 : 0x00;//	REVERSE PAPER FEED
            _val |= Print_Mode ? 0x10 : 0x00;
            _val |= (Pen_X <= 0) ? 0x20 : 0x00;
            qWarning()<<QString("return %1=%2").arg(addr,4,16,QChar('0')).arg(_val,2,16,QChar('0'));

            break;
        default: _val = 0xff; break;
        }

        bus->setData(_val);
        forwardBus = false;
        bus->setEnable(false);
        pCONNECTOR->Set_values(bus->toUInt64());
        return true;
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
        BYTE _val = fdd.fddRead(addr);


        bus->setData(_val);
        forwardBus = false;
        bus->setEnable(false);
        pCONNECTOR->Set_values(bus->toUInt64());
        return true;
    }

    if (bus->isEnable() &&
        bus->isME1() &&
        !bus->isWrite() &&
        (addr >= 0x70) && (addr <= 0x77) )
    {
        BYTE _val = fdd.fddRead(addr);
        qWarning()<<QString("read [%1]").arg(addr,4,16,QChar('0'));

        bus->setData(_val);
        forwardBus = false;
        bus->setEnable(false);
        pCONNECTOR->Set_values(bus->toUInt64());
        return true;
    }

    // Manage EXT Connector
    if (forwardBus) {
        // copy MainConnector to Ext Connectors
        pEXTCONNECTOR->Set_values(bus->toUInt64());
        // execute Ext
        mainwindow->pdirectLink->outConnector(pEXTCONNECTOR);
        bus->fromUInt64(pEXTCONNECTOR->Get_values());
    }

    bus->setEnable(false);
    pCONNECTOR->Set_values(bus->toUInt64());

    return(1);
}


void Cce1600p::Print(void)
{
    QPainter painter;

    pPC->Refresh_Display = true;

    ce150buf = checkPaper(ce150buf,Pen_Y);

    if (Pen_Status==PEN_DOWN)
    {
        painter.begin(ce150buf);
        switch (Pen_Color)
        {
            case 0 : painter.setPen( Qt::black); break;
            case 1 : painter.setPen( Qt::blue); break;
            case 2 : painter.setPen( Qt::green); break;
            case 3 : painter.setPen( Qt::red); break;
        }
        painter.drawPoint( Pen_X, Pen_Y );
        painter.end();
    }

    pPC->Refresh_Display = true;
    paperWidget->setOffset(QPoint(0,Pen_Y));
}



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
CFDD::CFDD(QObject *parent):QObject(parent)
{
    qWarning()<<"parent"<<parent;
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

CFDD::~CFDD()
{

}

// (40 = Read, Write = 60, A0 = format)
void CFDD::fddCmd(BYTE _data)
{
    CpcXXXX * _pc = ((CPObject*)parent())->pTIMER->pPC;
    qWarning()<<"FDD - fddCmd:"<<QString("%1").arg(_data,2,16,QChar('0'))
                <<QString("%1").arg(((CPObject*)parent())->pTIMER->pPC->pCPU->get_PC(),4,16,QChar('0'));
#if 0
    ((CPObject*)parent())->pTIMER->pPC->pCPU->logsw = true;
    ((CPObject*)parent())->pTIMER->pPC->pCPU->Check_Log();
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
//        ((CPObject*)parent())->pTIMER->pPC->pCPU->logsw = true;
//        ((CPObject*)parent())->pTIMER->pPC->pCPU->Check_Log();
        break;
    case 0xA0:
        qWarning()<<"format";
        offset=0;
        ((CPObject*)parent())->pTIMER->pPC->pCPU->logsw = true;
        ((CPObject*)parent())->pTIMER->pPC->pCPU->Check_Log();
        busy = true;
        ack = false;
        break;
    default:
//        qWarning()<<"ERROR:"<<QString("%1").arg(_data,2,16,QChar('0'));
        ack = false;
        break;
    }
}

void CFDD::fddSetSector(BYTE _data)
{
    qWarning()<<QString("FDD - fddSetSector:%1").arg(_data,2,16,QChar('0'));

    sector = _data;
    offset = 0;
}

BYTE CFDD::fddGetSector()
{
    qWarning()<<QString("FDD - fddGetSector:%1").arg(sector,2,16,QChar('0'));

    return sector;
}

void CFDD::fddSetMotor(BYTE _data)
{
    qWarning()<<QString("FDD - fddSetMotor:%1").arg(_data,2,16,QChar('0'))<<" running:"<<motorRunning;
    if (_data & 0x80) {
        if (!motorRunning) {
            // Start timer and set motorRunning after 500ms
            qWarning()<<"Start TIMER"<<((CPObject*)parent());
            startMotorState = ((CPObject*)parent())->pTIMER->state;
            busy = true;
        }
    }
    else {
        motorRunning = false;
        startMotorState = 0;
    }
}

void CFDD::fddWriteData(BYTE _data)
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

BYTE CFDD::fddReadData()
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
WORD CFDD::fddStatus()
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

void CFDD::fddwrite(BYTE _offset,BYTE _data) {

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

BYTE CFDD::fddRead(BYTE _offset) {

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
                  <<QString("%1").arg(((CPObject*)parent())->pTIMER->pPC->pCPU->get_PC(),4,16,QChar('0'));
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
                  <<QString("%1").arg(((CPObject*)parent())->pTIMER->pPC->pCPU->get_PC(),4,16,QChar('0'));
        break;
    case 0x03: // 7B: read data
        qWarning()<<((_offset & 0x08) ? "X: ":"Y: ");
        _val = fddReadData();
        break;
    default: _val = 0xff; break;
    }

    return _val;
}

void CFDD::fddReset()
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

void CFDD::step()
{
    CPObject *_pc = (CPObject*)parent();
    if ((startMotorState>0) && _pc->pTIMER) {
        if (_pc->pTIMER->msElapsed(startMotorState)>500) {
            qWarning()<<"MOTOR RUNNING";
            motorRunning = true;
            startMotorState = 0;
            busy = false;
        }
    }
}

