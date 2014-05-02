#ifndef RLP3001_H
#define RLP3001_H


#include <QTime>
#ifndef NO_SOUND
#include <QSound>
#endif

#include <QTcpSocket>

#include "common.h"

#include "Inter.h"

class Cconnector;

class Crlp3001:public CPObject{
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

    virtual void ComputeKey(void);

    Cconnector	*pCONNECTOR;		qint64 pCONNECTOR_value;

    Crlp3001(CPObject *parent = 0);

    virtual ~Crlp3001();

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


#endif // RLP3001_H
