#ifndef FP200_H
#define FP200_H

class CPObject;
class Ci8085;
class Cuart;
class Cctronics;
class CRP5C01;

#include "pcxxxx.h"

class Cfp200 : public CpcXXXX
{
    Q_OBJECT

public:
    Cfp200(CPObject *parent = 0);
    virtual ~Cfp200();

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual UINT8 in(UINT8 address,QString sender=QString());
    virtual UINT8 out(UINT8 address,UINT8 value,QString sender=QString());
    virtual UINT8 in8(UINT16 Port, QString sender=QString());
    virtual UINT8 out8(UINT16 address, UINT8 x, QString sender=QString());
    virtual UINT16 in16(UINT16 Port, QString sender=QString());
    virtual UINT16 out16(UINT16 Port, UINT16 value, QString sender=QString());

    virtual bool	Set_Connector(Cbus *_bus = 0);
    virtual bool	Get_Connector(Cbus *_bus = 0);

    bool init();

    virtual bool run();

    virtual void Reset();

    void TurnON();
    void TurnOFF();
    bool SaveConfig(QXmlStreamWriter *xmlOut);
    bool LoadConfig(QXmlStreamReader *xmlIn);

    virtual void    ExtChanged(void);

//    UINT16 getKey();

    Cconnector *pCENTCONNECTOR;    qint64      pCENTCONNECTOR_value;

    Ci8085 *i85cpu;
//    Cuart    *pUART;
    Cctronics *pCENT;
    CRP5C01 *pRP5C01;

    quint8 ks;
    quint16 getKey();
    bool Cetl;
    quint8 sid;

    int lastKeyBufSize;
    bool newKey;
//    void keyReleaseEvent(QKeyEvent *event);
//    void keyPressEvent(QKeyEvent *event);
    UINT8 matrixResult;
    virtual void initExtension(void);
private:
    bool Get_MainConnector(void);
    bool Set_MainConnector(void);
    void Get_CentConnector(void);
    void Set_CentConnecor(void);
    void Get_SIOConnector(void);
    void Set_SIOConnector(void);


};


#endif // FP200_H
