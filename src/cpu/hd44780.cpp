#include <QDebug>
#include <QFile>
#include <QPainter>


#include "hd44780.h"
#include "pcxxxx.h"

#define LOG 0

/***************************************************************************

        Hitachi HD44780 LCD controller

***************************************************************************/



#define BIT(x,n) (((x)>>(n))&1)

//-------------------------------------------------
//  CHD44780 - constructor
//-------------------------------------------------


CHD44780::CHD44780(QString fnCharSet,QObject *parent) :
    QObject(parent)
{
    pPC = (CpcXXXX*)parent;
    this->fncharset = fnCharSet;
    Reset();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

bool CHD44780::init()
{

    Reset();

    QFile file;
    file.setFileName(fncharset);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    in.readRawData ((char *) &charset,0x1000 );
    qWarning()<<charset;
    info.m_cgrom = (UINT8 *)&charset;

    return true;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void CHD44780::Reset()
{
    qWarning()<<"CHD44780::Reset()";

    memset(info.m_ddram, 0x20, sizeof(info.m_ddram)); // can't use 0 here as it would show CGRAM instead of blank space on a soft reset
    memset(info.m_cgram, 0, sizeof(info.m_cgram));

    info.m_ac         = 0;
    info.m_dr         = 0;
    info.m_ir         = 0;
    info.m_active_ram = DDRAM;
    info.m_display_on = false;
    info.m_cursor_on  = false;
    info.m_blink_on   = false;
    info.m_shift_on   = false;
    info.m_direction  = 1;
    info.m_data_len   = 8;
    info.m_num_line   = 1;
    info.m_char_size  = 8;
    info.m_disp_shift = 0;
    info.m_blink      = false;
    info.m_nibble     = false;
    info.m_first_cmd  = true;
    info.m_rs_state   = 0;
    info.m_rw_state   = 0;

    set_busy_flag(1520);
}

void CHD44780::set_busy_flag(UINT16 usec)
{
    info.m_busy_flag = false;
//    info.m_busy_timer->adjust( attotime::from_usec( usec ) );
}

void CHD44780::set_charset_type(int type)
{
    info.m_charset_type = type;
}

void CHD44780::update_ac(int direction)
{
    if (info.m_active_ram == DDRAM)
    {
        if(direction == 1)
        {
            if(info.m_num_line == 2 && info.m_ac == 0x27)
                info.m_ac = 0x40;
            else if((info.m_num_line == 2 && info.m_ac == 0x67) || (info.m_num_line == 1 && info.m_ac == 0x4f))
                info.m_ac = 0x00;
            else
                info.m_ac = (info.m_ac + direction) & 0x7f;
        }
        else
        {
            if(info.m_num_line == 2 && info.m_ac == 0x00)
                info.m_ac = 0x67;
            else if(info.m_num_line == 1 && info.m_ac == 0x00)
                info.m_ac = 0x4f;
            else if(info.m_num_line == 2 && info.m_ac == 0x40)
                info.m_ac = 0x27;
            else
                info.m_ac = (info.m_ac + direction) & 0x7f;
        }
    }
    else
    {
        info.m_ac = (info.m_ac + direction) & 0x3f;
    }
}

void CHD44780::shift_display(int direction)
{
    if (direction == 1)
    {
        if(info.m_disp_shift == 0x4f)
            info.m_disp_shift = 0x00;
        else
            info.m_disp_shift++;
    }
    else
    {
        if(info.m_disp_shift == 0x00)
            info.m_disp_shift = 0x4f;
        else
            info.m_disp_shift--;
    }
}

void CHD44780::update_nibble(int rs, int rw)
{
    if (info.m_rs_state != rs || info.m_rw_state != rw)
    {
        info.m_rs_state = rs;
        info.m_rw_state = rw;
        info.m_nibble = false;
    }

    info.m_nibble = !info.m_nibble;
}

inline void CHD44780::pixel_update(Clcdc *plcd,QPainter *painter, UINT8 line, UINT8 pos, UINT8 y, UINT8 x, int state,QColor color_ON,QColor color_OFF)
{
    if (info.m_pixel_update_func != NULL)
    {
        info.m_pixel_update_func(plcd,painter, line, pos, y, x, state, color_ON, color_OFF);
    }
    else
    {
        UINT8 line_heigh = (info.m_char_size == 8) ? info.m_char_size : info.m_char_size + 1;

        if (info.m_lines <= 2)
        {
            if (pos < info.m_chars) {
                painter->setPen(state ? color_ON : color_OFF );
//qWarning()<<"line:"<<line;
                painter->drawPoint( pos * 6 + x, line * (line_heigh+1) + y );
//                bitmap.pix16(line * (line_heigh+1) + y, pos * 6 + x) = state;
            }
        }
        else if (info.m_lines <= 4)
        {
            if (pos < info.m_chars*2)
            {
                if (pos >= info.m_chars)
                {
                    line += 2;
                    pos -= info.m_chars;
                }

                if (line < info.m_lines){
                    qWarning()<<"drawpoint:"<<pos * 6 + x<<","<< line * (line_heigh+1) + y<<"color:"<<state;
                    painter->setPen(state ? color_ON : color_OFF );
                    painter->drawPoint( pos * 6 + x, line * (line_heigh+1) + y );
    //                bitmap.pix16(line * (line_heigh+1) + y, pos * 6 + x) = state;
                }
            }
        }
        else
        {
            qWarning()<<"use a custom callback for this LCD configuration:"<< info.m_lines<<","<< info.m_chars;
        }
    }
}


UINT32 CHD44780::screen_update(Clcdc *plcd, QPainter *painter, QColor color_ON, QColor color_OFF)
{
//    bitmap.fill(0, cliprect);
//    painter->fillRect();

    if (info.m_display_on)
    {
        UINT8 line_size = 80 / info.m_num_line;

        for (int line=0; line<info.m_num_line; line++)
        {
            for (int pos=0; pos<line_size; pos++)
            {
                UINT16 char_pos = line * 0x40 + ((pos + info.m_disp_shift) % line_size);

                int char_base = 0;
                if (info.m_ddram[char_pos] < 0x10)
                {
                    // draw CGRAM characters
                    if (info.m_char_size == 8)
                        char_base = (info.m_ddram[char_pos] & 0x07) * 8;
                    else
                        char_base = ((info.m_ddram[char_pos]>>1) & 0x03) * 16;
                }
                else
                {
                    // draw CGROM characters
                    char_base = info.m_ddram[char_pos] * 0x10;
                }

                for (int y=0; y<info.m_char_size; y++)
                {
                    UINT8 * charset = (info.m_ddram[char_pos] < 0x10) ? info.m_cgram : info.m_cgrom;

                    for (int x=0; x<5; x++)
                        pixel_update(plcd,painter, line, pos, y, x, BIT(charset[char_base + y], 4 - x),color_ON,color_OFF);
                }

                // if is the correct position draw cursor and blink
                if (char_pos == info.m_ac)
                {
                    // draw the cursor
                    UINT8 cursor_pos = (info.m_char_size == 8) ? info.m_char_size : info.m_char_size + 1;
                    if (info.m_cursor_on)
                        for (int x=0; x<5; x++)
                            pixel_update(plcd,painter, line, pos, cursor_pos - 1, x, 1,color_ON,color_OFF);

                    if (!info.m_blink && info.m_blink_on)
                        for (int y=0; y<(cursor_pos - 1); y++)
                            for (int x=0; x<5; x++)
                                pixel_update(plcd,painter, line, pos, y, x, 1,color_ON,color_OFF);
                }
            }
        }
    }

    return 0;
}

UINT8 CHD44780::control_read()
{
    if (info.m_data_len == 4)
    {
//        if (!space.debugger_access())
            update_nibble(0, 1);

        if (info.m_nibble)
            return (info.m_busy_flag ? 0x80 : 0) | (info.m_ac & 0x70);
        else
            return (info.m_ac<<4) & 0xf0;
    }
    else
    {
        return (info.m_busy_flag ? 0x80 : 0) | (info.m_ac & 0x7f);
    }
}

void CHD44780::control_write(UINT8 data)
{
    if (info.m_data_len == 4)
    {
        update_nibble(0, 0);

        if (info.m_nibble)
        {
            info.m_ir = data & 0xf0;
            return;
        }
        else
        {
            info.m_ir |= ((data>>4) & 0x0f);
        }
    }
    else
    {
        info.m_ir = data;
    }

    if (BIT(info.m_ir, 7))           // set DDRAM address
    {
        info.m_active_ram = DDRAM;
        info.m_ac =info. m_ir & 0x7f;
        set_busy_flag(37);

        if (LOG) qWarning()<<"HD44780 : set DDRAM address "<<info.m_ac;
    }
    else if (BIT(info.m_ir, 6))      // set CGRAM address
    {
        info.m_active_ram = CGRAM;
        info.m_ac = info.m_ir & 0x3f;
        set_busy_flag(37);

        if (LOG) qWarning()<<"HD44780 : set CGRAM address "<<info.m_ac;
    }
    else if (BIT(info.m_ir, 5))      // function set
    {
        if (!info.m_first_cmd &&
             info.m_data_len == (BIT(info.m_ir, 4) ? 8 : 4) &&
            (info.m_char_size!= (BIT(info.m_ir, 2) ? 10 : 8) || info.m_num_line != (BIT(info.m_ir, 3) + 1)))
        {
            qWarning() << "HD44780 : function set cannot be executed after other instructions unless the interface data length is changed";
            return;
        }

        info.m_char_size = BIT(info.m_ir, 2) ? 10 : 8;
        info.m_data_len  = BIT(info.m_ir, 4) ? 8 : 4;
        info.m_num_line  = BIT(info.m_ir, 3) + 1;
        set_busy_flag(37);

//        if (LOG) logerror("HD44780 '%s': char size 5x%d, data len %d, lines %d\n", tag(),info.m_char_size, info.m_data_len, info.m_num_line);
        return;
    }
    else if (BIT(info.m_ir, 4))      // cursor or display shift
    {
        int direct = (BIT(info.m_ir, 2)) ? +1 : -1;

//        if (LOG) logerror("HD44780 '%s': %s shift %d\n", tag(), BIT(info.m_ir, 3) ? "display" : "cursor",  direct);

        if (BIT(info.m_ir, 3))
            shift_display(direct);
        else
            update_ac(direct);

        set_busy_flag(37);
    }
    else if (BIT(info.m_ir, 3))      // display on/off control
    {
        info.m_display_on = BIT(info.m_ir, 2);
        info.m_cursor_on  = BIT(info.m_ir, 1);
        info.m_blink_on   = BIT(info.m_ir, 0);
        set_busy_flag(37);

        if (LOG) qWarning()<<"HD44780 : display "<<info.m_display_on<<", cursor "<<info.m_cursor_on<<", blink "<< info.m_blink_on;
    }
    else if (BIT(info.m_ir, 2))      // entry mode set
    {
        info.m_direction = (BIT(info.m_ir, 1)) ? +1 : -1;
        info.m_shift_on  = BIT(info.m_ir, 0);
        set_busy_flag(37);

        if (LOG) qWarning()<<"HD44780 : entry mode set: direction "<<info.m_direction<<", shift "<< info.m_shift_on;
    }
    else if (BIT(info.m_ir, 1))      // return home
    {
        if (LOG) qWarning()<<"HD44780 : return home";

        info.m_ac         = 0;
        info.m_active_ram = DDRAM;
        info.m_direction  = 1;
        info.m_disp_shift = 0;
        set_busy_flag(1520);
    }
    else if (BIT(info.m_ir, 0))      // clear display
    {
        if (LOG) qWarning()<<"HD44780 : clear display";

        info.m_ac         = 0;
        info.m_active_ram = DDRAM;
        info.m_direction  = 1;
        info.m_disp_shift = 0;
        memset(info.m_ddram, 0x20, sizeof(info.m_ddram));
        set_busy_flag(1520);
    }

    info.m_first_cmd = false;

}


void CHD44780::data_write(UINT8 data)
{

    if (info.m_busy_flag)
    {
//		qWarning()<<"HD44780 : Ignoring data write due of busy flag\n", tag(), data);
        return;
    }

    if (info.m_data_len == 4)
    {
        update_nibble(1, 0);

        if (info.m_nibble)
        {
            info.m_dr = data & 0xf0;
            return;
        }
        else
        {
            info.m_dr |= ((data>>4) & 0x0f);
        }
    }
    else
    {
        info.m_dr = data;
    }

//	if (LOG) logerror("HD44780 '%s': %sRAM write %x %x '%c'\n", tag(), m_active_ram == DDRAM ? "DD" : "CG", m_ac, m_dr, isprint(m_dr) ? m_dr : '.');

    if (info.m_active_ram == DDRAM)
        info.m_ddram[info.m_ac] = info.m_dr;
    else
        info.m_cgram[info.m_ac] = info.m_dr;

    update_ac(info.m_direction);
    if (info.m_shift_on)
        shift_display(info.m_direction);
    set_busy_flag(41);
}

UINT8 CHD44780::data_read()
{
    UINT8 data = (info.m_active_ram == DDRAM) ? info.m_ddram[info.m_ac] : info.m_cgram[info.m_ac];

//	if (LOG) logerror("HD44780 '%s': %sRAM read %x %c\n", tag(), m_active_ram == DDRAM ? "DD" : "CG", m_ac, data);

    if (info.m_data_len == 4)
    {
//        if (!space.debugger_access())
            update_nibble(1, 1);

        if (info.m_nibble)
            return data & 0xf0;
        else
            data = (data<<4) & 0xf0;
    }

//    if (!space.debugger_access())
    {
        update_ac(info.m_direction);
        set_busy_flag(41);
    }

    return data;
}

HD44780info * CHD44780::getInfo()
{
    return &info;
}

void CHD44780::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "hd44780")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &info,ba_reg.data(),sizeof(info));
            info.m_cgrom = (UINT8 *)&charset;
//            updated = true;
        }
        xmlIn->skipCurrentElement();
    }
}

void CHD44780::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","hd44780");
        QByteArray ba_reg((char*)&info,sizeof(info));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
    xmlOut->writeEndElement();
}
