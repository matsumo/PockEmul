#include <QFile>
#include <QDebug>

#include "hd44352.h"



/***************************************************************************

        Hitachi HD44352 LCD controller

***************************************************************************/

#define		LCD_BYTE_INPUT			0x01
#define 	LCD_BYTE_OUTPUT			0x02
#define 	LCD_CHAR_OUTPUT			0x03
#define 	LCD_ON_OFF				0x04
#define 	LCD_CURSOR_GRAPHIC		0x06
#define 	LCD_CURSOR_CHAR			0x07
#define 	LCD_SCROLL_CHAR_WIDTH	0x08
#define 	LCD_CURSOR_STATUS		0x09
#define 	LCD_USER_CHARACTER		0x0b
#define 	LCD_CONTRAST			0x0c
#define 	LCD_IRQ_FREQUENCY		0x0d
#define 	LCD_CURSOR_POSITION		0x0e

#define BIT(x,n) (((x)>>(n))&1)

//-------------------------------------------------
//  CHD44352 - constructor
//-------------------------------------------------


CHD44352::CHD44352(QString fnCharSet,QObject *parent) :
    QObject(parent)
{
    this->fncharset = fnCharSet;
    OP_bit = 0x01;
    Reset();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

bool CHD44352::init()
{
//    qWarning()<<"CHD44352::init"<<fncharset;
    Reset();

    QFile file;
    file.setFileName(fncharset);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    int _result = in.readRawData ((char *) &charset,0x800 );

    return true;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void CHD44352::Reset()
{
    memset(info.m_video_ram, 0x00, sizeof(info.m_video_ram));
    memset(info.m_par, 0x00, sizeof(info.m_par));
    memset(info.m_custom_char, 0x00, sizeof(info.m_custom_char));
    memset(info.cursor, 0x00, sizeof(info.cursor));
    info.m_control_lines = 0;
    info.m_data_bus = 0xff;
    info.m_state = 0;
    info.m_bank = 0;
    info.m_offset = 0;
    info.m_char_width = 6;
    info.m_lcd_on = 0;
    info.m_scroll = 0;
    info.m_byte_count = 0;
    info.m_cursor_status = 0;
    info.m_contrast = 0;
    on_timer_rate = 8192;
}



void CHD44352::control_write(UINT8 data)
{
//    if(info.m_control_lines == data)
//        info.m_state = 0;

    info.m_control_lines = data;
}

UINT8 CHD44352::compute_newval(UINT8 type, UINT8 oldval, UINT8 newval)
{
    switch(type & 0x07)
    {
        case 0x00:
            return (~oldval) & newval;
        case 0x01:
            return oldval ^ newval;
        case 0x03:
            return oldval & (~newval);
        case 0x04:
            return newval;
        case 0x05:
            return oldval | newval;
        case 0x07:
            return oldval;
        case 0x02:
        case 0x06:
        default:
            return 0;
    }
}

UINT8 CHD44352::get_char(UINT16 pos)
{
    switch ((UINT8)pos/8)
    {
        case 0xcf:
            return info.m_custom_char[0][pos%8];
        case 0xdf:
            return info.m_custom_char[1][pos%8];
        case 0xef:
            return info.m_custom_char[2][pos%8];
        case 0xff:
            return info.m_custom_char[3][pos%8];
        default:
            return charset[pos];
    }
}

void CHD44352::data_write(UINT8 data)
{
    // verify that controller is active
    if (!(info.m_control_lines & 0x80)) {
//        qWarning()<<"inactive";
        return;
    }

    if (info.m_control_lines & OP_bit)
    {
//        if (!(info.m_control_lines&0x02) && !(info.m_control_lines&0x04))
//            return;
qWarning()<<"state="<<info.m_state<<"  data="<<QString("%1").arg(data,2,16,QChar('0'));
        switch (info.m_state)
        {
            case 0:		//parameter 0
                info.m_par[info.m_state++] = data;
                break;
            case 1:		//parameter 1
                info.m_par[info.m_state++] = data;
                break;
            case 2:		//parameter 2
                info.m_par[info.m_state++] = data;
                break;
        }

        switch (info.m_par[0] & 0x0f)
        {
            case LCD_BYTE_INPUT:
            case LCD_CHAR_OUTPUT:
                {
                    if (info.m_state == 1)
                        info.m_bank = BIT(data, 4);
                    else if (info.m_state == 2)
                        info.m_offset = ((data>>1)&0x3f) % 48 + (BIT(data,7) * 48);
                    else if (info.m_state == 3)
                        info.m_offset += ((data & 0x03) * 96);
                }
                break;
            case LCD_BYTE_OUTPUT:
                {
                    if (info.m_state == 1)
                        info.m_bank = BIT(data, 4);
                    else if (info.m_state == 2)
                        info.m_offset = ((data>>1)&0x3f) % 48 + (BIT(data,7) * 48);
                    else if (info.m_state == 3)
                        info.m_offset += ((data & 0x03) * 96);
                }
                break;
            case LCD_ON_OFF:
                {
                    if (info.m_state == 1)
                        info.m_lcd_on = BIT(data, 4);
                    info.m_data_bus = 0xff;
                    info.m_state = 0;
                }
                break;
            case LCD_SCROLL_CHAR_WIDTH:
                {
                    if (info.m_state == 1)
                    {
                        info.m_char_width = 8-((data>>4)&3);
                        info.m_scroll = ((data>>6)&3);
                    }

                    info.m_data_bus = 0xff;
                    info.m_state = 0;
                }
                break;
            case LCD_CURSOR_STATUS:
                {
                    if (info.m_state == 1) {
                        info.m_cursor_status = data;
//                        qWarning()<<"LCD_CURSOR_STATUS:"<<data;
                    }
                    info.m_data_bus = 0xff;
                    info.m_state = 0;
                }
                break;
            case LCD_CONTRAST:
                {
                    if (info.m_state == 1)
                        info.m_contrast = (info.m_contrast & 0x00ffff) | (data<<16);
                    else if (info.m_state == 2)
                        info.m_contrast = (info.m_contrast & 0xff00ff) | (data<<8);
                    else if (info.m_state == 3)
                    {
                        info.m_contrast = (info.m_contrast & 0xffff00) | (data<<0);
                        info.m_state = 0;
                    }

                    info.m_data_bus = 0xff;
                }
                break;
            case LCD_IRQ_FREQUENCY:
                {
                    if (info.m_state == 1)
                    {


                        switch((data>>4) & 0x0f)
                        {
                            case 0x00:		on_timer_rate = 16384;		break;
                            case 0x01:		on_timer_rate = 8;			break;
                            case 0x02:		on_timer_rate = 16;			break;
                            case 0x03:		on_timer_rate = 32;			break;
                            case 0x04:		on_timer_rate = 64;			break;
                            case 0x05:		on_timer_rate = 128;		break;
                            case 0x06:		on_timer_rate = 256;		break;
                            case 0x07:		on_timer_rate = 512;		break;
                            case 0x08:		on_timer_rate = 1024;		break;
                            case 0x09:		on_timer_rate = 2048;		break;
                            case 0x0a:		on_timer_rate = 4096;		break;
                            case 0x0b:		on_timer_rate = 4096;		break;
                            default:		on_timer_rate = 8192;		break;
                        }

//                        m_on_timer->adjust(attotime::from_hz(m_clock/on_timer_rate), 0, attotime::from_hz(m_clock/on_timer_rate));
                    }
                    info.m_data_bus = 0xff;
                    info.m_state = 0;
                }
                break;
            case LCD_CURSOR_POSITION:
                {
                     int _cursorId = (info.m_par[0]>>4)&0x01;
                    if (info.m_state == 1)
                        info.cursor[_cursorId].m_cursor_lcd = BIT(data, 4);	//0:left lcd 1:right lcd;
                    else if (info.m_state == 2)
                        info.cursor[_cursorId].m_cursor_x = ((data>>1)&0x3f) % 48;// + (BIT(data,7) * 48);
                    else if (info.m_state == 3)
                    {
                        info.cursor[_cursorId].m_cursor_y = data & 0x03;
                        info.m_state = 0;
                    }

                    qWarning()<<"LCD_CURSOR_POSITION:"<<info.cursor[_cursorId].m_cursor_x<<info.cursor[_cursorId].m_cursor_y<<" page:"<<info.cursor[_cursorId].m_cursor_lcd;
                    info.m_data_bus = 0xff;
                }
                break;
        }

        info.m_byte_count = 0;
        info.m_data_bus = 0xff;
    }
    else
    {
//        qWarning()<<"data:"<<info.m_par[0]<<"  control:"<<info.m_control_lines;
        switch (info.m_par[0] & 0x0f)
        {
            case LCD_BYTE_INPUT:
                {
                    if (((info.m_par[0]>>5) & 0x07) != 0x03)
                        break;

                    info.m_offset %= 0x180;
                    info.m_data_bus = ((info.m_video_ram[info.m_bank][info.m_offset]<<4)&0xf0) | ((info.m_video_ram[info.m_bank][info.m_offset]>>4)&0x0f);
                    info.m_offset++; info.m_byte_count++;
                }
                break;
            case LCD_BYTE_OUTPUT:
                {
                    info.m_offset %= 0x180;
                    info.m_video_ram[info.m_bank][info.m_offset] =
                            compute_newval((info.m_par[0]>>5) & 0x07,
                                            info.m_video_ram[info.m_bank][info.m_offset],
                                            data);
                    info.m_offset++;
                    info.m_byte_count++;

                    info.m_data_bus = 0xff;
                }
                break;
            case LCD_CHAR_OUTPUT:
                {
                    int char_pos = data*8;

                    for (int i=0; i<info.m_char_width; i++)
                    {
                        info.m_offset %= 0x180;
                        info.m_video_ram[info.m_bank][info.m_offset] = compute_newval((info.m_par[0]>>5) & 0x07, info.m_video_ram[info.m_bank][info.m_offset], get_char(char_pos));
                        info.m_offset++; char_pos++;
                    }

                    info.m_byte_count++;
                    info.m_data_bus = 0xff;
                }
                break;
            case LCD_CURSOR_GRAPHIC:
                if (info.m_byte_count<8)
                {
                    int _cursorId = (info.m_par[0]>>4)&0x01;
                    info.cursor[_cursorId].m_cursor[info.m_byte_count] = data;
                    info.m_byte_count++;
                    info.m_data_bus = 0xff;
                }
                break;
            case LCD_CURSOR_CHAR:
                if (info.m_byte_count<1)
                {
                    int _cursorId = (info.m_par[0]>>4)&0x01;
                    UINT8 char_code = ((data<<4)&0xf0) | ((data>>4)&0x0f);

                    for (int i=0; i<8; i++)
                        info.cursor[_cursorId].m_cursor[i] = get_char(char_code*8 + i);

                    info.m_byte_count++;
                    info.m_data_bus = 0xff;
                }
                break;
            case LCD_USER_CHARACTER:
                if (info.m_byte_count<8)
                {
                    info.m_custom_char[(info.m_par[1]&0x03)][info.m_byte_count] = data;
                    info.m_byte_count++;
                    info.m_data_bus = 0xff;
                }
                break;
            default:
                info.m_data_bus = 0xff;
        }

        info.m_state=0;
    }
}

UINT8 CHD44352::data_read()
{
    return info.m_data_bus;
}

HD44352info CHD44352::getInfo()
{
    return info;
}
