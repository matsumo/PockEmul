#ifndef HX20_H
#define HX20_H

class CPObject;

class CUPD16434;
class Cconnector;
class Cmc6800;
class Cm160;

#include "pcxxxx.h"


class Chx20 : public CpcXXXX
{
    Q_OBJECT

public:
    Chx20(CPObject *parent = 0);
    virtual ~Chx20();

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual UINT8 in(UINT8 addr,QString sender=QString());
    virtual UINT8 out(UINT8 addr, UINT8 data,QString sender=QString());
    virtual UINT16 out16(UINT16 address,UINT16 value);

    virtual bool	Set_Connector(Cbus *_bus = 0);
    virtual bool	Get_Connector(Cbus *_bus = 0);

    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0,QMouseEvent *event=0);

    virtual bool init();
    virtual bool run();
    virtual void Reset();

    virtual bool UpdateFinalImage(void);

    bool Set_PrinterConnector(Cconnector *_conn);
    bool Get_PrinterConnector(Cconnector *_conn);

    bool Set_CN8(Cconnector *_conn);
    bool Get_CN8(Cconnector *_conn);

    void TurnON();
    void TurnOFF();
    bool SaveConfig(QXmlStreamWriter *xmlOut);
    bool LoadConfig(QXmlStreamReader *xmlIn);

    UINT16 getKey();

    CUPD16434 *upd16434[6];

    Cm160 *pM160;

    Cconnector	*pTAPECONNECTOR;	qint64 pTAPECONNECTOR_value;
    Cconnector	*pPRINTERCONNECTOR;	qint64 pPRINTERCONNECTOR_value;
    Cconnector	*pCN8;	qint64 pCN8_value;

    void send_to_main(quint8 val);
    void send_to_slave(quint8 val);
protected slots:
    void contextMenuEvent (QContextMenuEvent * e);
private:
    UINT16 kstrobe;
    UINT8 lcd_select;
    QList<UINT8> cmd_buf;

    int int_status;
    int int_mask;
    int key_intmask;
    UINT16 key_data;

    Cmc6800 *pmc6301,*pSlaveCPU;

    bool special_cmd_masked;
    bool targetSlave;
    void run(CCPU *_cpu);
    bool printerSW;
    bool lcdBit;
};


#endif // HX20_H

