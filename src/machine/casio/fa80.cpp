

#include <QPainter>
#include <QTime>
#include <QFileDialog>
#include <QThread>
#include <QPixmap>
#include <QBitmap>

#include "common.h"

#include "fa80.h"
#include "Connect.h"
#include "Inter.h"
#include "init.h"
#include "Log.h"
#include "dialoganalog.h"
#include "bus.h"
#include "hd61710.h"


Cfa80::Cfa80(CPObject *parent):CPObject(parent)
{								//[constructor]
    setfrequency( 0);

    BackGroundFname	= P_RES(":/fx8000g/fa80.png");
    setcfgfname("fa80");

    TopFname = P_RES(":/fx8000g/fa80top.png");
    LeftFname = P_RES(":/fx8000g/fa80left.png");

    pTIMER		= new Ctimer(this);

    pHD61710 = new CHD61710(this);


    setDX(714);
    setDY(693);

    setDXmm(200);
    setDYmm(194);
    setDZmm(37);

    port = 0x08;
    bus = new Cbus();
    adrBus=0;

}

Cfa80::~Cfa80() {
    delete pCONNECTOR;
    delete pCENTCONNECTOR;
}

bool Cfa80::UpdateFinalImage(void) {
    CPObject::UpdateFinalImage();


    QPainter painter;
    painter.begin(FinalImage);

    // POWER LED
//    if (READ_BIT(port,3)==0) {
//        painter.fillRect(698,550,18,9,QColor(Qt::green));
//    }

    painter.end();
    return true;

}



bool Cfa80::init(void)
{
    CPObject::init();

    setfrequency( 0);

    pCONNECTOR	   = new Cconnector(this,30,0,Cconnector::Casio_30,"Connector 30 pins",true,QPoint(414,375));	publish(pCONNECTOR);
    pCENTCONNECTOR = new Cconnector(this,36,1,Cconnector::Centronics_36,"Centronic 36 pins",false,QPoint(417,13)); publish(pCENTCONNECTOR);

    WatchPoint.add(&pCONNECTOR_value,64,30,this,"Standard 30pins connector");
    WatchPoint.add(&pCENTCONNECTOR_value,64,36,this,"Centronic 36pins connector");

    AddLog(LOG_PRINTER,tr("FA-80 initializing..."));

    if(pTIMER)	pTIMER->init();


    data = 0;

    return true;
}


/*****************************************************/
/* Exit PRINTER										 */
/*****************************************************/
bool Cfa80::exit(void)
{
    AddLog(LOG_PRINTER,"FA-80 Closing...");
    AddLog(LOG_PRINTER,"done.");
    CPObject::exit();
    return true;
}

bool Cfa80::Get_Connector(void) {
    Get_MainConnector();
    Get_CentConnector();

    return true;
}
bool Cfa80::Set_Connector(void) {
    Set_CentConnecor();
    Set_MainConnector();

    return true;
}

void Cfa80::Get_CentConnector(void) {
    pHD61710->Get_CentConnector(pCENTCONNECTOR);

}

void Cfa80::Set_CentConnecor(void) {
    pHD61710->Set_CentConnecor(pCENTCONNECTOR);

}

#define PIN(x)    (pCONNECTOR->Get_pin(x))
bool Cfa80::Get_MainConnector(void) {

    bus->fromUInt64(pCONNECTOR->Get_values());

    return true;
}

#define PORT(x)  (port << (x))
bool Cfa80::Set_MainConnector(void) {

    pCONNECTOR->Set_values(bus->toUInt64());

    return true;
}

#define DOWN	0
#define UP		1

#define LOG { \
    qWarning()<<(bus->isWrite()?"WRITE ":"READ :")<<QString("%1").arg(adrBus,4,16,QChar('0')) \
              << "data:"<<QString("%1").arg(data,4,16,QChar('0'))<<QChar(data); \
        }

bool Cfa80::run(void)
{
    Get_Connector();

    if (!bus->isEnable()) {
//        if (keyEvent) {
//            pLH5810->step();
//            bus->setINT(pLH5810->INT);
//            pCONNECTOR->Set_values(bus->toUInt64());
//        }
        return true;
    }
    pCONNECTOR_value = pCONNECTOR->Get_values();
    pCENTCONNECTOR_value = pCENTCONNECTOR->Get_values();

    adrBus = bus->getAddr();
    data = bus->getData();

/*
Register addresses

A2	A1	A0	Access	Function
0	0	0	Write	serial port control register
                    bit 0 - MT/RS232C
                    bit 1 - Odd/Even parity
                    bit 2 - parity OFF/ON
                    bit 3 - 7/8 data length
                    bit 4 - 1/2 stop bit
                    bits 5..7 - baud rate
0	0	0	Read	serial port status register
                    bit 0 - set when TX buffer full, cleared after the byte has been transmitted
                    bit 1 - set when RX buffer full, cleared after reading the RX data register 010
                    bit 2 - state of the CTS input
                    bit 3 - state of the DSR input
                    bit 4 - state of the DCD input
0	0	1	Write	serial port control register
                    bit 0 - transmitter enable
                    bit 1 - receiver enable
                    bit 2 - state of the RTS output
                    bit 3 - state of the DTR output
0	0	1	Read	serial port status register
                    bit 0 - set when RX parity error
                    bit 1 - cleared when RX framing or overrun error
                    bit 3 - state of the SW0 input
                    bit 4 - state of the SW1 input
                    bit 5 - state of the SW2 input
0	1	0	Write	serial port control register
0	1	0	Read	serial port receive data register
0	1	1	Write	serial port transmit data register
0	1	1	Read	general purpose input port PA
1	0	0	Write	general purpose output port PB
1	0	0	Read	printer status port
                    bit 0 - state of the BUSY input
                    bit 1 - state of the FAULT input
                    bit 2 - set by a pulse on the ACK input, reset by writing logical 1 to the bit 2 of register 110
1	0	1	Write	printer data port PD
1	1	0	Write	printer control port
                    bit 0 - state of the STROBE output
                    bit 1 - state of the INIT output
                    bit 2 - logical 1 resets the ACK latch


1. La fréquence du porteuse (lead-in) est 4800 Hz (sinus).
2. Le zéro est une seule onde à 2400 Hz.
3. Le un est compris de deux ondes à 4800 Hz.
4. La vitesse résultante est donc 2400 baud

*/

    if (bus->isWrite()) {
#if 1
        pHD61710->regWrite(adrBus,data);
#else
        switch (adrBus) {
        case 0x00: // 01:MT, 40:2400bds
//            0	0	0	Write	serial port control register
//                                bit 0 - MT/RS232C
//                                bit 1 - Odd/Even parity
//                                bit 2 - parity OFF/ON
//                                bit 3 - 7/8 data length
//                                bit 4 - 1/2 stop bit
//                                bits 5..7 - baud rate
//                                        Baud rate selects
//                                        000	9600 baud
//                                        001	4800 baud
//                                        010	2400 baud
//                                        011	1200 baud
//                                        100	600 baud
//                                        101	300 baud
//                                        110	150 baud
//                                        111	75 baud

            break;
        case 0x01: break;
        case 0x02: // serial port control register
            break;
        case 0x03: // serial port transmit data register
            break;
        case 0x04: break;
        case 0x05: LOG;
            printerDataPort(data);
            break;
        case 0x06: LOG;
            printerControlPort(data);
            break;
        case 0x07: break;
        }
#endif
    }
#if 1
    bus->setData(pHD61710->regRead(adrBus));
#else
    else {
        switch (adrBus) {
        case 0x00: LOG;break;
        case 0x01: LOG;break;
        case 0x02: LOG;break;
        case 0x03: LOG;bus->setData(0xF4); break;
        case 0x04: LOG;bus->setData(printerStatusPort()); break;   // 0x02
        case 0x05: LOG;break;
        case 0x06: LOG;break;
        case 0x07: LOG;break;
        }
    }
//        else if (adrBus==0x05) {
//            printerDataPort(data);
//        }
//        else if ((adrBus==0x06))//&&(P2==UP))
//        {
//            printerControlPort(data);
//        }
//        else if (adrBus==0x04) {
//            // Send the printerStatusPort
//            sendData = true;
//            out_adrBus = 0x04;
//            data = printerStatusPort();
//        }
//        else if (adrBus == 0x00) {
#endif

    Set_Connector();
    pCONNECTOR_value = pCONNECTOR->Get_values();
    pCENTCONNECTOR_value = pCENTCONNECTOR->Get_values();
//    pSIOCONNECTOR_value = pSIOCONNECTOR->Get_values();

    return true;
}



