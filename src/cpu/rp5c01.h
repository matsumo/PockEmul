#ifndef RP5C01_H
#define RP5C01_H

/**********************************************************************

    Ricoh RP5C01(A) Real Time Clock With Internal RAM emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   _CS   1 |*    \_/     | 18  Vcc
                    CS   2 |             | 17  OSCOUT
                   ADJ   3 |             | 16  OSCIN
                    A0   4 |   RP5C01    | 15  _ALARM
                    A1   5 |   RP5C01A   | 14  D3
                    A2   6 |   RF5C01A   | 13  D2
                    A3   7 |   TC8521    | 12  D1
                   _RD   8 |             | 11  D0
                   GND   9 |_____________| 10  _WR

**********************************************************************/

#if 1

#include <QObject>

class CPObject;


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class CRP5C01:public QObject {
    Q_OBJECT
public:

    // registers
    enum
    {
        REGISTER_1_SECOND = 0,
        REGISTER_10_SECOND,
        REGISTER_1_MINUTE,
        REGISTER_10_MINUTE,
        REGISTER_1_HOUR,
        REGISTER_10_HOUR,
        REGISTER_DAY_OF_THE_WEEK,
        REGISTER_1_DAY,
        REGISTER_10_DAY,
        REGISTER_1_MONTH,
        REGISTER_10_MONTH, REGISTER_12_24_SELECT = REGISTER_10_MONTH,
        REGISTER_1_YEAR, REGISTER_LEAP_YEAR = REGISTER_1_YEAR,
        REGISTER_10_YEAR,
        REGISTER_MODE,
        REGISTER_TEST,
        REGISTER_RESET
    };

    const char*	GetClassName(){ return("CRP5C01");}
    // construction/destruction
//    CRP5C01(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
    CRP5C01(CPObject *parent);
    ~CRP5C01();

    bool	init(void);						//initialize
    bool	exit(void);						//end
    void	Reset(void);
    bool	step(void);

    quint8 read(quint8 offset);
    void write(quint8 offset, quint8 data);

protected:

    virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second);

    CPObject *pPC;

private:
    inline void set_alarm_line();
    inline int read_counter(int counter);
    inline void write_counter(int counter, int value);
    inline void check_alarm();



    quint8 m_reg[2][13];			// clock registers
    quint8 m_ram[13];			// RAM

    quint8 m_mode;				// mode register
    quint8 m_reset;				// reset register
    int m_alarm;				// alarm output
    int m_alarm_on;				// alarm condition
    int m_1hz;					// 1 Hz condition
    int m_16hz;					// 16 Hz condition

    quint64 statelog;

};

#endif

#endif // RP5C01_H
