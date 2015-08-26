#ifndef HX20_H
#define HX20_H

class CPObject;

class CUPD16434;
class Cconnector;
class Cmc6800;

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

    void TurnON();
    void TurnOFF();
    bool SaveConfig(QXmlStreamWriter *xmlOut);
    bool LoadConfig(QXmlStreamReader *xmlIn);

    UINT16 getKey();

    CUPD16434 *upd16434[6];

    Cconnector	*pTAPECONNECTOR;	qint64 pTAPECONNECTOR_value;
    Cconnector	*pPRINTERCONNECTOR;	qint64 pPRINTERCONNECTOR_value;
    void send_to_main(quint8 val);
    void send_to_slave(quint8 val);
private:
    UINT16 kstrobe;
    UINT8 lcd_select;
    QList<UINT8> cmd_buf;

    int int_status;
    int int_mask;
    int key_intmask;
    int key_data;

    Cmc6800 *pmc6301,*pSlaveCPU;

    bool special_cmd_masked;
    bool runSlave(CCPU *);
    bool targetSlave;
};


#endif // HX20_H

