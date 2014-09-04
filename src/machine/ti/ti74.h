#ifndef TI74_H
#define TI74_H


class CPObject;
class Ctms70c46;
class CHD44780;

#include "pcxxxx.h"

class Cti74 : public CpcXXXX
{
    Q_OBJECT

public:
    Cti74(CPObject *parent = 0);
    virtual ~Cti74();

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
    virtual UINT8 in(UINT8 address);
    virtual UINT8 out(UINT8 address,UINT8 value);

    virtual bool	Set_Connector(void);
    virtual bool	Get_Connector(void);

    bool init();

    virtual bool run();

    virtual void Reset();

    void TurnON();
    void TurnOFF();
    bool SaveConfig(QXmlStreamWriter *xmlOut);
    bool LoadConfig(QXmlStreamReader *xmlIn);

    virtual void ComputeKey();

    Ctms70c46 *ptms70c46cpu;
    CHD44780 *pHD44780;

    quint8 ks;
    virtual quint8 getKey();
    UINT8 m_clock_control;
    UINT8 m_banks;
    UINT8 m_power;
    UINT8 m_bus_control;

    virtual void initExtension(void);

    UINT8 *m_sysram[2];
    UINT16 m_sysram_size[2];
    UINT16 m_sysram_end[2];
    UINT16 m_sysram_mask[2];

    void clock_w(UINT8 data);
    UINT8 clock_r();
    void power_w(UINT8 data);
    bool slotChanged;
    int currentSlot;

protected slots:
        void addModule(QString item, CPObject *pPC);
};

class Cti95 : public Cti74
{
    Q_OBJECT

public:
    Cti95(CPObject *parent = 0);
    virtual ~Cti95();

    virtual quint8 getKey();
};

#endif // TI74_H
