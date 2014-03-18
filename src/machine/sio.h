#ifndef _SIO_H
#define _SIO_H

/*** POCKEMUL [SIO.H] *************************************/
/* UART(serial port) emulation class                      */
/**********************************************************/
#include <QMenu>
#include <QMutex>

#include "pobject.h"
#include "Connect.h"


class DialogConsole;

class Csio:public CPObject{
Q_OBJECT

public:
    enum SIGNAME {
        S_SD,S_RD,S_RS,S_CS,S_GND,S_CD,S_RR,S_ER
    };

    const char*	GetClassName(){ return("Csio");}

	QByteArray baOutput;
	QByteArray baInput;
		
    Cconnector	*pSIOCONNECTOR; qint64 pSIOCONNECTOR_value;

    virtual bool run(void);

    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
	void Set_SI(void);				//serial in 
	void Put_SO(void);				//serial out

	void ExportBit(bool);
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

	void	Set_Sii_bit(qint8);
	qint8	Get_Sii_bit(void);
	
    qint8	byteToBit(qint8 data);
	bool	transmit(void);
	void	startTransfer(void);

    void	receive(void);


	bool	CD,CS,RD,RR,RS,ER,SD;
	qint8	Sii_bit;
	bool	si,so;					//SI,SO port access flag
	bool	plink;					//plink server ON/OFF
	qint8	plinkmode;				//select plink client
	char	*filename[2];			//SI&SO data filename
	FILE	*fp_si,*fp_so;			//SI,SO file handle
	qint8	exportbit;				//export bit a bit
	qint8	exportbyte;				//export Byte a Byte
	qint8	convCRLF;

	bool	ToDestroy;
	
	void	clearInput(void);
    DialogConsole *dialogconsole;
    QMap<SIGNAME,qint8> signalMap;
	
    Csio(CPObject *parent = 0);

    virtual ~Csio();

public:
	void paintEvent(QPaintEvent *);

    bool initSignalMap(Cconnector::ConnectorType type);
    quint8 getPinId(SIGNAME signal);

    virtual bool    SaveSession_File(QXmlStreamWriter *xmlOut);
    virtual bool	LoadSession_File(QXmlStreamReader *);

    void initConnectorType(QString type);

    void updateMapConsole();
    bool SIO_GET_PIN(SIGNAME signal);
protected slots:
    void contextMenuEvent ( QContextMenuEvent * );
    void ShowConsole(void);
    void HideConsole(void);
    void slotConnType(QAction *);
signals:
    void valueChanged(int v);
    void newByteRecv(qint8);

private:
    QMutex SMapMutex;
    int		outBitNb;
	int		Sii_ndx;
    int		Sii_wait,Sii_wait_recv;
	bool	Sii_startbitsent;
	bool	Sii_stopbitsent;
	bool	Sii_TransferStarted;
	int		Sii_TextLength;
	int		Sii_Bit_Nb;
	int		Sii_LfWait;	
    int     baudrate;

    qint8   currentBit;
    quint64  oldstate_in;
    quint64  oldstate_out;
    bool    Start_Bit_Sent;
    unsigned char	t,c,waitbitstart,waitbitstop,waitparity;
    int byteBufferSize;
};



#endif  // _SIO_H
