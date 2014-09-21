#include <QDebug>

#include "hd61710.h"
#include "Log.h"
#include "dialoganalog.h"
#include "Connect.h"
#include "uart.h"

CHD61710::CHD61710(CPObject *parent, Cconnector *pCENT, Cconnector *pTAPE, Cconnector *pSIO) {
    pPC = parent;
    pCENTCONNECTOR = pCENT;
    pTAPECONNECTOR = pTAPE;
    pSIOCONNECTOR = pSIO;
}

CHD61710::~CHD61710()
{

}

void CHD61710::linkConnectors(Cconnector *pCENT, Cconnector *pTAPE, Cconnector *pSIO)
{
    pCENTCONNECTOR = pCENT;
    pTAPECONNECTOR = pTAPE;
    pSIOCONNECTOR = pSIO;
}

bool CHD61710::init()
{
    uart.init();
    uart.pTIMER = pPC->pTIMER;
}

bool CHD61710::step()
{
    uart.run();

}


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

#define LOGWRITE { \
    qWarning()<<"WRITE :"<<QString("%1").arg(adr,4,16,QChar('0')) \
              << "data:"<<QString("%1").arg(data,4,16,QChar('0'))<<QChar(data); \
        }
#define LOGREAD { \
    qWarning()<<"READ :"<<QString("%1").arg(adr,4,16,QChar('0')); \
        }

void CHD61710::regWrite(qint8 adr, quint8 data)
{
        switch (adr) {
        case 0x00: WriteSioInitReg(data); break;
        case 0x01:
            //            0	0	1	Write	serial port control register
            //                                bit 0 - transmitter enable
            //                                bit 1 - receiver enable
            //                                bit 2 - state of the RTS output
            //                                bit 3 - state of the DTR output
            if (data & 0x01) info.sioTransEnabled = true;
            if (data & 0x02) info.sioReceiEnabled = true;
            uart.Set_RS(data & 0x04);   // RTS
            //            uart.Set_(data & 0x08);   // DTR
            break;
        case 0x02: // serial port control register
            break;
        case 0x03: // serial port transmit data register
            uart.sendByte(data);
            qWarning()<<"sio send byte:"
                     <<QString("%1").arg(data,2,16,QChar('0'))
                    <<QChar(data);
            break;
        case 0x04: // Write	general purpose output port PA
            break;
        case 0x05: // Write	printer data port PD
            LOGWRITE;
            WritePrtDataPort(data);
            break;
        case 0x06:
//            1	1	0	Write	printer control port
//                                bit 0 - state of the STROBE output
//                                bit 1 - state of the INIT output
//                                bit 2 - logical 1 resets the ACK latch
            LOGWRITE;
            WritePrtCrlPort(data);
            break;
        case 0x07: LOGWRITE;
            break;
        }
}

BYTE CHD61710::regRead(qint8 adr)
{
    BYTE _result = 0;

        switch (adr) {
        case 0x00:
//            0	0	0	Read	serial port status register
//                                bit 0 - set when TX buffer full, cleared after the byte has been transmitted
//                                bit 1 - set when RX buffer full, cleared after reading the RX data register 010
//                                bit 2 - state of the CTS input
//                                bit 3 - state of the DSR input
//                                bit 4 - state of the DCD input
            LOGREAD;break;
        case 0x01:
//            0	0	1	Read	serial port status register
//                                bit 0 - set when RX parity error
//                                bit 1 - cleared when RX framing or overrun error
//                                bit 3 - state of the SW0 input
//                                bit 4 - state of the SW1 input
//                                bit 5 - state of the SW2 input
            LOGREAD;break;
        case 0x02: // Read	serial port receive data register
            LOGREAD;
            break;
        case 0x03: // Read	general purpose input port PB
            LOGREAD;
            _result = 0xF4;
            break;
        case 0x04:
//            1	0	0	Read	printer status port
//                                bit 0 - state of the BUSY input
//                                bit 1 - state of the FAULT input
//                                bit 2 - set by a pulse on the ACK input, reset by writing logical 1 to the bit 2 of register 110
            LOGREAD;
            _result = ReadPrtStatusPort();
            break;
        case 0x05: LOGREAD;break;
        case 0x06: LOGREAD;break;
        case 0x07: LOGREAD;break;
        }

        return _result;
}

void CHD61710::WriteSioInitReg(BYTE data) {
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

    info.sioMode = data & 0x01;
    qWarning()<<"sioMode:"<<info.sioMode;
//    info.sioParity = data & 0x02 ? PARITY_ODD : PARITY_EVEN;
    info.sioParityEnabled = data & 0x04;
    info.sioDataLength = data & 0x08 ? 8 : 7;
    info.sioStopBit = data & 0x10 ? 1 : 2;
    info.sioBds = 9600 >> ((data >> 4)&0x07);
    uart.Set_BaudRate(info.sioBds);
    info.sioCTRLReg = data;

}

#define TIMER_ACK 9
#define TIMER_BUSY 8
void CHD61710::WritePrtCrlPort(BYTE value)
{
//    bit 0 - state of the STROBE output
//    bit 1 - state of the INIT output
//    bit 2 - logical 1 resets the ACK latch
    AddLog(LOG_PRINTER,tr("PRINTER controlPort= %1").arg(value,2,16,QChar('0')));
    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(8);

    info.printerSTROBE = (value & 0x01)?true:false;
    info.printerINIT = (value & 0x02)?true:false;


    if (value & 0x04) {
        info.printerACK = false;
//        pTIMER->resetTimer(9);
        AddLog(LOG_PRINTER,tr("PRINTER controlPort RESET ACK"))
    }

    info.prev_printerSTROBE = info.printerSTROBE;
    info.prev_printerINIT = info.printerINIT;
}

BYTE CHD61710::ReadPrtStatusPort()
{
//    bit 0 - state of the BUSY input
//    bit 1 - state of the FAULT input
//    bit 2 - set by a pulse on the ACK input, reset by writing logical 1 to the bit 2 of register 110


    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(9);
    BYTE ret = 0;

    if (info.printerBUSY) ret |= 0x01;
//    if (printerERROR)
        ret |= 0x02;        // FAULT to 1
    if (!info.printerACK) ret |= 0x04;

    if (ret != info.prev_printerStatusPort) {
        AddLog(LOG_PRINTER,tr("PRINTER return status PORT : %1").arg(ret,2,16,QChar('0')));
        info.prev_printerStatusPort = ret;
    }

    return (ret & 0x07);
}

void CHD61710::WritePrtDataPort(BYTE value)
{
    if (value != 0xff) {
        AddLog(LOG_PRINTER,tr("PRINTER data : %1").arg(value,2,16,QChar('0')));
        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(7);
        info.printerDATA = value;
    }
}

void CHD61710::Get_CentConnector(void) {
    if (pCENTCONNECTOR) {
        info.printerACK = pCENTCONNECTOR->Get_pin(10);
        info.printerBUSY = pCENTCONNECTOR->Get_pin(11);
        info.printerERROR=pCENTCONNECTOR->Get_pin(32);
    }
}

void CHD61710::Set_CentConnector(void) {

    if (pCENTCONNECTOR) {
        pCENTCONNECTOR->Set_pin((1) ,info.printerSTROBE);

        pCENTCONNECTOR->Set_pin(2	,READ_BIT(info.printerDATA,0));
        pCENTCONNECTOR->Set_pin(3	,READ_BIT(info.printerDATA,1));
        pCENTCONNECTOR->Set_pin(4	,READ_BIT(info.printerDATA,2));
        pCENTCONNECTOR->Set_pin(5	,READ_BIT(info.printerDATA,3));
        pCENTCONNECTOR->Set_pin(6	,READ_BIT(info.printerDATA,4));
        pCENTCONNECTOR->Set_pin(7	,READ_BIT(info.printerDATA,5));
        pCENTCONNECTOR->Set_pin(8	,READ_BIT(info.printerDATA,6));
        pCENTCONNECTOR->Set_pin(9	,READ_BIT(info.printerDATA,7));

        pCENTCONNECTOR->Set_pin(31	,info.printerINIT);
    }
}

void CHD61710::Get_TAPEConnector(void) {
    if (pTAPECONNECTOR) {
//        MT_IN = pTAPECONNECTOR->Get_pin(1);      // In
    }
}

void CHD61710::Set_TAPEConnector(void) {

    if (pTAPECONNECTOR) {
        //        pTAPECONNECTOR->Set_pin(3,(rmtSwitch ? SEL1:true));       // RMT
        if (info.sioMode) {
            pTAPECONNECTOR->Set_pin(2,uart.Get_SD());    // Out
        }
    }
}


void CHD61710::Get_SIOConnector(void) {
    if (pSIOCONNECTOR) {

    }
}

void CHD61710::Set_SIOConnector(void) {

    if (pSIOCONNECTOR) {

    }
}

void CHD61710::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "hd61710")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &info,ba_reg.data(),sizeof(info));
        }
        xmlIn->skipCurrentElement();
    }
}

void CHD61710::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","hd61710");
        QByteArray ba_reg((char*)&info,sizeof(info));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
    xmlOut->writeEndElement();
}
