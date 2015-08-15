#ifndef HX20_H
#define HX20_H

class CPObject;

class CUPD16434;
class Cconnector;

#include "pcxxxx.h"


class Chx20 : public CpcXXXX
{
    Q_OBJECT

public:
    Chx20(CPObject *parent = 0);
    virtual ~Chx20();

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual UINT8 in(UINT8 addr);
    virtual UINT8 out(UINT8 addr, UINT8 data);
    virtual UINT16 out16(UINT16 address,UINT16 value);

    virtual bool	Set_Connector(Cbus *_bus = 0);
    virtual bool	Get_Connector(Cbus *_bus = 0);



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
private:
    UINT16 kstrobe;
    UINT8 lcd_select;
    UINT8 lcd_clk_counter;
};


#endif // HX20_H

