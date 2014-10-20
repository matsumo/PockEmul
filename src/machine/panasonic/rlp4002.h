#ifndef RLP4002_H
#define RLP4002_H

#include <QTime>
#ifndef NO_SOUND
#include <QSound>
#endif

#include <QTcpSocket>

#include "common.h"

#include "Inter.h"

class Cconnector;

class Crlp4002:public CPObject{
    Q_OBJECT
public:

    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    virtual	bool run(void);					//check access

    virtual bool	Set_Connector(void);
    virtual bool	Get_Connector(void);

    virtual bool    SaveSession_File(QXmlStreamWriter *xmlOut);
    virtual bool	LoadSession_File(QXmlStreamReader *xmlIn);
    virtual bool	InitDisplay(void);

    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0);

    Cconnector	*pCONNECTOR;		qint64 pCONNECTOR_value;

    Crlp4002(CPObject *parent = 0);

    virtual ~Crlp4002();

public:
    bool rts,cts;

public slots:
    void addModule(QString item, CPObject *pPC);
protected:
    void paintEvent(QPaintEvent *);
protected slots:
    void contextMenuEvent ( QContextMenuEvent * );
    void Rotate();
    void readData();

private:
    bool rotate;
    bool slotChanged;
    bool connected;
    bool xon;
    QByteArray inBuffer,outBuffer,_trans;

    quint8 statusReg;
    quint8 outputReg;
    quint8 controlReg;
    quint8 commandReg;

    QTcpSocket soc;

};


#endif // RLP4002_H
