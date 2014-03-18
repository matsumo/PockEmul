
/**********************************************************************

    Ricoh RP5C01(A) Real Time Clock With Internal RAM emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

/*

    TODO:

    - 12 hour clock
    - test register
    - timer reset

*/
#if 1
#include <QDateTime>

#include "rp5c01.h"
#include "pobject.h"
#include "Inter.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


#define RAM_SIZE 13




// register write mask
static const int REGISTER_WRITE_MASK[2][16] =
{
    { 0xf, 0x7, 0xf, 0x7, 0xf, 0x3, 0x7, 0xf, 0x3, 0xf, 0x1, 0xf, 0xf, 0xf, 0xf, 0xf },
    { 0x0, 0x0, 0xf, 0x7, 0xf, 0x3, 0x7, 0xf, 0x3, 0x0, 0x1, 0x3, 0x0, 0xf, 0xf, 0xf }
};


// days per month
static const int DAYS_PER_MONTH[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


// modes
enum
{
    MODE00 = 0,
    MODE01,
    BLOCK10,
    BLOCK11
};


// mode register
#define MODE_MASK			0x03
#define MODE_ALARM_EN		0x04
#define MODE_TIMER_EN		0x08


// test register
#define TEST_0				0x01
#define TEST_1				0x02
#define TEST_2				0x04
#define TEST_3				0x08


// reset register
#define RESET_ALARM			0x01
#define RESET_TIMER			0x02
#define RESET_16_HZ			0x04
#define RESET_1_HZ			0x08



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_alarm_line -
//-------------------------------------------------

inline void CRP5C01::set_alarm_line()
{
    int alarm = ((m_mode & MODE_ALARM_EN) ? m_alarm_on : 1) &
                ((m_reset & RESET_16_HZ) ? 1 : m_16hz) &
                ((m_reset & RESET_1_HZ) ? 1 : m_1hz);

    if (m_alarm != alarm)
    {
//        if (LOG) logerror("RP5C01 '%s' Alarm %u\n", tag(), alarm);

//        m_out_alarm_func(alarm);
        m_alarm = alarm;
    }
}


//-------------------------------------------------
//  read_counter -
//-------------------------------------------------

inline int CRP5C01::read_counter(int counter)
{
    return (m_reg[MODE00][counter + 1] * 10) + m_reg[MODE00][counter];
}


//-------------------------------------------------
//  write_counter -
//-------------------------------------------------

inline void CRP5C01::write_counter(int counter, int value)
{
    m_reg[MODE00][counter] = value % 10;
    m_reg[MODE00][counter + 1] = value / 10;
}


//-------------------------------------------------
//  check_alarm -
//-------------------------------------------------

inline void CRP5C01::check_alarm()
{
    bool all_match = true;
    bool all_zeroes = true;

    for (int i = REGISTER_1_MINUTE; i < REGISTER_1_MONTH; i++)
    {
        if (m_reg[MODE01][i] != 0) all_zeroes = false;
        if (m_reg[MODE01][i] != m_reg[MODE00][i]) all_match = false;
    }

    m_alarm_on = (all_match || (!m_alarm_on && all_zeroes)) ? 0 : 1;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  rp5c01_device - constructor
//-------------------------------------------------

CRP5C01::CRP5C01(CPObject *parent)
    : m_alarm(1),
      m_alarm_on(1),
      m_1hz(1),
      m_16hz(1)
{
    pPC = parent;
}

CRP5C01::~CRP5C01()
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

bool CRP5C01::init()
{

    // allocate timers
//    m_clock_timer = timer_alloc(TIMER_CLOCK);
//    m_clock_timer->adjust(attotime::from_hz(clock() / 16384), 0, attotime::from_hz(clock() / 16384));

//    m_16hz_timer = timer_alloc(TIMER_16HZ);
//    m_16hz_timer->adjust(attotime::from_hz(clock() / 1024), 0, attotime::from_hz(clock() / 1024));

    QDateTime lastDateTime = QDateTime::currentDateTime();
    rtc_clock_updated(lastDateTime.date().year()%100,
                      lastDateTime.date().month(),
                      lastDateTime.date().day(),
                      lastDateTime.date().dayOfWeek(),
                      lastDateTime.time().hour(),
                      lastDateTime.time().minute(),
                      lastDateTime.time().second());


    statelog = pPC->pTIMER->state;

    return true;
}

bool CRP5C01::exit()
{
    return true;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void CRP5C01::Reset()
{
    //    set_current_time(machine());
}

bool CRP5C01::step()
{

    if (pPC->pTIMER->msElapsed(statelog)>1000) {
        QDateTime lastDateTime = QDateTime::currentDateTime();
        rtc_clock_updated(lastDateTime.date().year()%100,
                          lastDateTime.date().month(),
                          lastDateTime.date().day(),
                          lastDateTime.date().dayOfWeek(),
                          lastDateTime.time().hour(),
                          lastDateTime.time().minute(),
                          lastDateTime.time().second());
        statelog = pPC->pTIMER->state;
    }
    return true;
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------
#if 0
void CRP5C01::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
    switch (id)
    {
    case TIMER_CLOCK:
        if (m_1hz && (m_mode & MODE_TIMER_EN))
        {
            advance_seconds();
        }

        m_1hz = !m_1hz;
        set_alarm_line();
        break;

    case TIMER_16HZ:
        m_16hz = !m_16hz;
        set_alarm_line();
        break;
    }
}

#endif
//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void CRP5C01::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
    m_reg[MODE01][REGISTER_LEAP_YEAR] = year % 4;
    write_counter(REGISTER_1_YEAR, year);
    write_counter(REGISTER_1_MONTH, month);
    write_counter(REGISTER_1_DAY, day);
    m_reg[MODE00][REGISTER_DAY_OF_THE_WEEK] = day_of_week;
    write_counter(REGISTER_1_HOUR, hour);
    write_counter(REGISTER_1_MINUTE, minute);
    write_counter(REGISTER_1_SECOND, second);

    check_alarm();
    set_alarm_line();
}






//-------------------------------------------------
//  read -
//-------------------------------------------------

quint8 CRP5C01::read( quint8 offset)
{
    quint8 data = 0;

    switch (offset & 0x0f)
    {
    case REGISTER_MODE:
        data = m_mode;
        break;

    case REGISTER_TEST:
    case REGISTER_RESET:
        // write only
        break;

    default:
        data = m_reg[m_mode & MODE_MASK][offset];
        break;
    }

//    if (LOG) logerror("RP5C01 '%s' Register %u Read %02x\n", tag(), offset & 0x0f, data);

    return data & 0x0f;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void CRP5C01::write(quint8 offset,quint8 data)
{
    int mode = m_mode & MODE_MASK;

    switch (offset & 0x0f)
    {
    case REGISTER_MODE:
        m_mode = data & 0x0f;

        if (LOG)
        {
//            logerror("RP5C01 '%s' Mode %u\n", tag(), data & MODE_MASK);
//            logerror("RP5C01 '%s' Timer %s\n", tag(), (data & MODE_TIMER_EN) ? "enabled" : "disabled");
//            logerror("RP5C01 '%s' Alarm %s\n", tag(), (data & MODE_ALARM_EN) ? "enabled" : "disabled");
        }
        break;

    case REGISTER_TEST:
//        if (LOG) logerror("RP5C01 '%s' Test %u not supported!\n", tag(), data);
        break;

    case REGISTER_RESET:
        m_reset = data & 0x0f;

        if (data & RESET_ALARM)
        {
            int i;

            // reset alarm registers
            for (i = REGISTER_1_MINUTE; i < REGISTER_1_MONTH; i++)
            {
                m_reg[MODE01][i] = 0;
            }
        }

        if (LOG)
        {
//            if (data & RESET_ALARM) logerror("RP5C01 '%s' Alarm Reset\n", tag());
//            if (data & RESET_TIMER) logerror("RP5C01 '%s' Timer Reset not supported!\n", tag());
//            logerror("RP5C01 '%s' 16Hz Signal %s\n", tag(), (data & RESET_16_HZ) ? "disabled" : "enabled");
//            logerror("RP5C01 '%s' 1Hz Signal %s\n", tag(), (data & RESET_1_HZ) ? "disabled" : "enabled");
        }
        break;

    default:
        switch (mode)
        {
        case MODE00:
        case MODE01:
            m_reg[mode][offset & 0x0f] = data & REGISTER_WRITE_MASK[mode][offset & 0x0f];

//            set_time(false,
//                     read_counter(REGISTER_1_YEAR),
//                     read_counter(REGISTER_1_MONTH),
//                     read_counter(REGISTER_1_DAY),
//                     m_reg[MODE00][REGISTER_DAY_OF_THE_WEEK],
//                     read_counter(REGISTER_1_HOUR),
//                     read_counter(REGISTER_1_MINUTE),
//                     read_counter(REGISTER_1_SECOND));
            break;

        case BLOCK10:
            m_ram[offset & 0x0f] = (m_ram[offset & 0x0f] & 0xf0) | (data & 0x0f);
            break;

        case BLOCK11:
            m_ram[offset & 0x0f] = (data << 4) | (m_ram[offset & 0x0f] & 0x0f);
            break;
        }

//        if (LOG) logerror("RP5C01 '%s' Register %u Write %02x\n", tag(), offset & 0x0f, data);
        break;
    }
}

#endif
