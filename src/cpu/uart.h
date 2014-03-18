#ifndef UART_H
#define UART_H

#include "pobject.h"
#define TICKS_BDS	(pTIMER->pPC->getfrequency()/baudrate)

class CUartConsole;

class Cuart : public CPObject
{
Q_OBJECT

public:
    enum SIGNAME {
        S_SD,S_RD,S_RS,S_CS,S_GND,S_CD,S_RR,S_ER
    };

    enum BIT_TYPE {
        Bit_Start,Bit_Stop,Bit_0,Bit_1
    };

    const char*	GetClassName(){ return("Cuart");}


    qint64  bitbuffer;

    virtual bool run(void);

    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end

    virtual void byteRecv(qint8);

    void Set_BaudRate(int);
    int  Get_BaudRate(void);

    virtual void Set_CD(bool);
    virtual void Set_CS(bool);
    virtual void Set_RD(bool);
    virtual void Set_RR(bool);
    virtual void Set_RS(bool);
    virtual void Set_ER(bool);
    virtual void Set_SD(bool);

    bool Get_CD(void);
    bool Get_CS(void);
    bool Get_RD(void);
    bool Get_RR(void);
    bool Get_RS(void);
    bool Get_ER(void);
    bool Get_SD(void);

    BIT_TYPE 	byteToBit(qint8 data);
    BIT_TYPE	sendBit(void);
    void	startTransfer(void);

    void	bitToByte(void);

    bool	CD,CS,RD,RR,RS,ER,SD;

    void	clearInput(void);

    Cuart(CPObject *parent = 0);

    virtual ~Cuart(){
    }

    void sendByte(UINT8 data);
    quint8 getInputByte();
    bool isInputByte();
    quint8 popInputByte();
    void newInputByte(qint8 data);
    void newInputArray(QByteArray badata);
    void clearInputBuffer(void);

    CUartConsole *pUartConsole;

protected slots:

    void ShowConsole(void);
    void HideConsole(void);

signals:
    void newByteRecv(qint8);
    void newByteSent(qint8);

private:
    int		bitNb;
    int		Sii_ndx;
    int		Sii_wait;
    bool	Sii_startbitsent;
    bool	Sii_stopbitsent;
    bool	Sii_TransferStarted;
    int		Sii_TextLength;
    int		Sii_Bit_Nb;
    int		Sii_LfWait;
    int     baudrate;

    BIT_TYPE   currentBit;
    quint64  oldstate_in;
    quint64  oldstate_out;
    bool    Start_Bit_Sent;
    unsigned char	t,c,waitbitstart,waitbitstop;
    int byteBufferSize;

    QByteArray inputBuffer;
    QByteArray outputBuffer;
};



#endif // UART_H

