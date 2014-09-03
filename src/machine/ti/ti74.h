#ifndef TI74_H
#define TI74_H


class CPObject;
class Ctms7000;
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

    Ctms7000 *ptms7000cpu;
    CHD44780 *pHD44780;

    quint8 ks;
    quint8 getKey();
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
    void bus_control_w(UINT8 data);
    UINT8 bus_control_r();
    void sysram_w(UINT16 offset, UINT8 data);
    UINT8 sysram_r(UINT16 offset);
    void init_sysram(int chip, UINT16 size);
};


#endif // TI74_H
