

#include <QPainter>
#include <QtGui>

#include "uart.h"
#include "common.h"
#include "Inter.h"
#include "Log.h"
#include "sio.h"
#include "pcxxxx.h"
#include "Connect.h"
#include "dialogconsole.h"
#include "init.h"
#include "ui/uartconsole.h"

#define SIO_GET_PIN(n)		pSIOCONNECTOR->Get_pin(getPinId(n))
#define SIO_SET_PIN(n,v)	pSIOCONNECTOR->Set_pin(getPinId(n),v)


#define SIO_GNDP	1
#define SIO_SD 		2
#define SIO_RD		3
#define SIO_RS		4
#define SIO_CS		5

#define SIO_GND		7
#define SIO_CD		8
#define SIO_VC1		10
#define SIO_RR		11
#define SIO_PAK		12
#define SIO_VC2		13
#define SIO_ER		14
#define SIO_PRQ		15





Cuart::Cuart(CPObject *parent)	: CPObject(parent)
{							//[constructor]

    currentBit=Bit_0;
    oldstate_in=0;
    Start_Bit_Sent = false;
    t=c=0;waitbitstart=1;waitbitstop=0;

    baudrate = 1200;

    bitNb = 0;
    Sii_ndx				= 0;
    Sii_wait			= 0;
    Sii_startbitsent	= false;
    Sii_stopbitsent		= true;
    Sii_TransferStarted = false;
    Sii_TextLength		= 0;
    Sii_Bit_Nb			= 0;
    Sii_LfWait			= 500;

    pUartConsole = new CUartConsole(this);
    connect(this,SIGNAL(newByteSent(qint8)),pUartConsole,SLOT(newOutputByte(qint8)));



}

bool Cuart::Get_CD(void)		{ return(CD);	}
bool Cuart::Get_CS(void)		{ return(CS);	}
bool Cuart::Get_RR(void)		{ return(RR);	}
bool Cuart::Get_RS(void)		{ return(RS);	}
bool Cuart::Get_ER(void)		{ return(ER);	}
bool Cuart::Get_SD(void)		{ return(SD);	}
bool Cuart::Get_RD(void)		{ return(RD);	}

void Cuart::Set_SD(bool val) { SD = val;	}
void Cuart::Set_CD(bool val) { CD = val;	}
void Cuart::Set_CS(bool val) { CS = val;	}
void Cuart::Set_RD(bool val) { RD = val;	}
void Cuart::Set_RR(bool val) { RR = val;	}
void Cuart::Set_ER(bool val) { ER = val;	}
void Cuart::Set_RS(bool val) { RS = val;	}

void Cuart::Set_BaudRate(int br) {
    baudrate = br;
    AddLog(LOG_SIO,tr("new baudrate = %1").arg(br))
}
int  Cuart::Get_BaudRate(void) {return baudrate;}


bool Cuart::run(void)
{

    if (outputBuffer.size()) {
        Set_RS(true);
    }

    if (RS && CS)	{ Set_SD( (sendBit()==Bit_1) ? true:false);	}

    if (CD) { Set_RR(1); Set_CD(1); }
    bitToByte();

    return true;
}

void Cuart::startTransfer(void)
{
    Sii_wait			= 0;
    Sii_ndx				= 0;
    Sii_startbitsent	= false;
    Sii_stopbitsent		= true;
    Sii_TransferStarted = true;
    byteBufferSize = outputBuffer.size();
    Set_CD(1);

}

void Cuart::clearInput(void)
{
    Sii_ndx = 0;
    outputBuffer.clear();
}

Cuart::BIT_TYPE Cuart::sendBit(void)
{

    if (oldstate_in	== 0) oldstate_in = pTIMER->state;
    BYTE		data		= 0;
    int			deltastate	= 0;

    Sii_LfWait = 100;
    Sii_wait	= TICKS_BDS;
    if (! Sii_TransferStarted) return Bit_0;

    // If there are new data in baInput
    if (outputBuffer.size())
    {
        deltastate = pTIMER->state - oldstate_in;

        if (deltastate < Sii_wait) return(currentBit);

//        oldstate_in	= pTIMER->state;
        oldstate_in	+= deltastate;

        data	= outputBuffer.at(0);

        data = (data == 0x0A ? 0x0D : data);
        currentBit		= byteToBit(data);

        switch (currentBit)
        {
        case Bit_Start:	currentBit = Bit_1;
                        Sii_wait = TICKS_BDS;
                        return(currentBit);		// START BIT
        case Bit_0:
        case Bit_1:     AddLog(LOG_SIO,tr("Envoie bit = %1").arg(currentBit==Bit_1?1:0));
                        Sii_wait = TICKS_BDS;
                        return(currentBit);		// DATA BIT

        case Bit_Stop:	currentBit = Bit_0;
                        Sii_wait = TICKS_BDS;
                        if (data == 0x0D)
                        {
                            Sii_wait=Sii_LfWait*pTIMER->pPC->getfrequency()/1000;
                            AddLog(LOG_SIO,tr("LF found, wait %1 ms").arg(Sii_LfWait));
                        }
                        outputBuffer.remove(0,1);										// Next Char
                        return(currentBit);
        }
    }

    if (
        Sii_TransferStarted &&
        (outputBuffer.size() == 0) )
    {
        Sii_startbitsent	= false;
        Sii_stopbitsent		= true;
        Sii_TransferStarted	= false;
        Set_CD(0);
        AddLog(LOG_SIO,tr("END TRANSMISSION"));
    }

    return(Bit_0);
}

void Cuart::sendByte(UINT8 data) {
    outputBuffer.append(data);
    emit newByteSent(data);
}

// Byte to bit
//
// Take in account number of start, stop and parity bits
//

Cuart::BIT_TYPE Cuart::byteToBit(qint8 data)
{
    BIT_TYPE bit = Bit_0;

        if (!Start_Bit_Sent)
        {
            Start_Bit_Sent = true;
            AddLog(LOG_SIO,tr("START BIT : %1=(%2)").arg(data,2,16,QChar('0')).arg(QChar(data)));
            return(Bit_Start);  // Startbit = 1
        }
        else
        if (bitNb<8)
        {
            bit = ( ((data >> bitNb) & 0x01) ? Bit_0 : Bit_1 );
            bitNb++;
            return(bit);
        }
        else
        if (bitNb == 8)
        {
            bitNb = 0;
            AddLog(LOG_SIO,tr("STOP BIT"));
            Start_Bit_Sent	= false;
            return(Bit_Stop);	// STOPBIT : To be converted to 0
        }
    return(Bit_Start);
}

quint8 Cuart::getInputByte(void) {
    if (inputBuffer.size()) return inputBuffer.at(0);
    return 0;
}

bool Cuart::isInputByte(void) {
    return !inputBuffer.isEmpty();
}

quint8 Cuart::popInputByte(void) {
    if (!inputBuffer.isEmpty()) {
        qint8 data = inputBuffer.at(0);
        inputBuffer.remove(0,1);
        return data;
    }
    return 0;
}

void Cuart::byteRecv(qint8 data)
{
    inputBuffer.append( (char) data);

    // Emit signal new data
    emit newByteRecv(data);

    Refresh_Display = true;
}

// convert bit to Byte
void Cuart::bitToByte(void)
{
    int deltastate=0;
    if (oldstate_out == 0) oldstate_out	= pTIMER->state;

    deltastate = pTIMER->state - oldstate_out;
    if (deltastate < Sii_wait) return;

    if (!RR)
    {
        oldstate_out	= pTIMER->state;
        Sii_wait	= 0;
        return;
    }
//	Sii_wait	= TICKS_BDS;
    oldstate_out	+= Sii_wait;
//	oldstate	+= Sii_wait;


    if (waitbitstop && !RD)
    {
        waitbitstop = 0;waitbitstart=1;
//		Bit STOP
        AddLog(LOG_SIO,tr("STOP BIT"));
        Sii_wait = 0;
    }
    else if (waitbitstart && RD)
    {
        waitbitstart = 0;
//		Bit START
        AddLog(LOG_SIO,tr("START BIT"));
        Sii_wait	= TICKS_BDS;

    }
    else if (!waitbitstart)
    {
        t>>=1;
        if(!RD) t|=0x80;
        AddLog(LOG_SIO,tr("Bit = %1").arg(RD));
        if((c=(++c)&7)==0)
        {
            AddLog(LOG_SIO,tr("Byte = %1").arg(t,2,16,QChar('0')));
            byteRecv(t);
            t=0;
            waitbitstop = 1;
        }
    }
}


/*****************************************************************************/
/* Initialize SIO															 */
/*****************************************************************************/
bool Cuart::init(void)
{

    AddLog(LOG_MASTER,"UART initializing...");

    CD = CS = ER = RD = RR = RS = SD = 0;

    CPObject::init();

    AddLog(LOG_MASTER,"done.\n");

    return true;
}

/*****************************************************************************/
/* Exit SIO																	 */
/*****************************************************************************/
bool Cuart::exit(void)
{
    return true;
}

void Cuart::ShowConsole(void) {
    pUartConsole->show();
}
void Cuart::HideConsole(void) {
    pUartConsole->hide();
}

void Cuart::newInputByte(qint8 data)
{
    inputBuffer.append(data);
}

void Cuart::clearInputBuffer()
{
    inputBuffer.clear();
}

void Cuart::newInputArray(QByteArray badata)
{
    inputBuffer.append(badata);
}
