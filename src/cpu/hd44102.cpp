/**********************************************************************

    HD44102 Dot Matrix Liquid Crystal Graphic Display Column Driver emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/
#include <QDebug>

#include "hd44102.h"
#include "pobject.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


#define CONTROL_DISPLAY_OFF			0x38
#define CONTROL_DISPLAY_ON			0x39
#define CONTROL_COUNT_DOWN_MODE		0x3a
#define CONTROL_COUNT_UP_MODE		0x3b
#define CONTROL_Y_ADDRESS_MASK		0x3f
#define CONTROL_X_ADDRESS_MASK		0xc0
#define CONTROL_DISPLAY_START_PAGE	0x3e


#define STATUS_BUSY					0x80	/* not supported */
#define STATUS_COUNT_UP				0x40
#define STATUS_DISPLAY_OFF			0x20
#define STATUS_RESET				0x10	/* not supported */


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  count_up_or_down -
//-------------------------------------------------

inline void CHD44102::count_up_or_down()
{
    if (info.m_status & STATUS_COUNT_UP)
    {
        if (++info.m_y > 49) info.m_y = 0;
    }
    else
    {
        if (--info.m_y < 0) info.m_y = 49;
    }
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  CHD44102 - constructor
//-------------------------------------------------

CHD44102::CHD44102(CPObject *parent)
{
    pPC = parent;


}


//-------------------------------------------------
//  static_set_config - configuration helper
//-------------------------------------------------

//void CHD44102::static_set_config(device_t &device, const char *screen_tag, int sx, int sy)
//{
//    CHD44102 &hd44102 = downcast<CHD44102 &>(device);

//    assert(screen_tag != NULL);

//    hd44102.m_screen_tag = screen_tag;
//    hd44102.m_sx = sx;
//    hd44102.m_sy = sy;
//}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

bool CHD44102::init()
{
    memset(info.imem,0,sizeof(info.imem));
    info.m_cs2=0;
    info.m_page=0;
    info.m_x=0;
    info.m_y=0;
    updated = true;

    return true;
}

bool CHD44102::exit()
{
    return true;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void CHD44102::Reset()
{
    info.m_status = STATUS_DISPLAY_OFF | STATUS_COUNT_UP;
}

BYTE CHD44102::cmd_status()
{
//    qWarning()<<"status:"<<info.m_status;
    return 0;
    return info.m_status;
}


//-------------------------------------------------
//  data_r - data read
//-------------------------------------------------
BYTE CHD44102::get8()
{
    UINT8 data = info.imem[info.m_x][info.m_y];
    count_up_or_down();

    return data;
}

//-------------------------------------------------
//  data_w - data write
//-------------------------------------------------
void CHD44102::set8( BYTE val)
{
    info.imem[info.m_x][info.m_y] = val;
    count_up_or_down();
    updated = true;
}

void CHD44102::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "hd44102")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &info,ba_reg.data(),sizeof(info));
            updated = true;
        }
        xmlIn->skipCurrentElement();
    }
}

void CHD44102::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","hd44102");
        QByteArray ba_reg((char*)&info,sizeof(info));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
    xmlOut->writeEndElement();
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

//READ8_MEMBER( CHD44102::read )
//{
//    UINT8 data = 0;

//    if (m_cs2)
//    {
//        data = (offset & 0x01) ? data_r(space, offset) : status_r(space, offset);
//    }

//    return data;
//}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

//WRITE8_MEMBER( CHD44102::write )
//{
//    if (m_cs2)
//    {
//        (offset & 0x01) ? data_w(space, offset, data) : control_w(space, offset, data);
//    }
//}



//-------------------------------------------------
//  control_w - control write
//-------------------------------------------------
void CHD44102::cmd_write(UINT8 data)
{

    if (info.m_status & STATUS_BUSY) return;

    switch (data)
    {
    case CONTROL_DISPLAY_OFF:
        info.m_status |= STATUS_DISPLAY_OFF;
//        if (LOG) logerror("HD44102 '%s' Display Off\n", tag());

        break;

    case CONTROL_DISPLAY_ON:
//        if (LOG) logerror("HD44102 '%s' Display On\n", tag());

        info.m_status &= ~STATUS_DISPLAY_OFF;
        break;

    case CONTROL_COUNT_DOWN_MODE:
//        if (LOG) logerror("HD44102 '%s' Count Down Mode\n", tag());

        info.m_status &= ~STATUS_COUNT_UP;
        break;

    case CONTROL_COUNT_UP_MODE:
//        if (LOG) logerror("HD44102 '%s' Count Up Mode\n", tag());

        info.m_status |= STATUS_COUNT_UP;
        break;

    default:
        {
        int x = (data & CONTROL_X_ADDRESS_MASK) >> 6;
        int y = data & CONTROL_Y_ADDRESS_MASK;

        if ((data & CONTROL_Y_ADDRESS_MASK) == CONTROL_DISPLAY_START_PAGE)
        {
//            if (LOG) logerror("HD44102 '%s' Display Start Page %u\n", tag(), x);

            info.m_page = x;
            qWarning()<<"PAGE="<<x;
        }
        else if (y > 49)
        {
//            logerror("HD44102 '%s' Invalid Address X %u Y %u (%02x)!\n", tag(), data, x, y);
        }
        else
        {
//            if (LOG) logerror("HD44102 '%s' Address X %u Y %u (%02x)\n", tag(), data, x, y);

            info.m_x = x;
            info.m_y = y;
            qWarning()<<"m_x="<<x<<"  m_y="<<y;
        }
        }
    }

    updated = true;
}


//-------------------------------------------------
//  cs2_w - chip select 2 write
//-------------------------------------------------

//WRITE_LINE_MEMBER( CHD44102::cs2_w )
//{
//    m_cs2 = state;
//}



