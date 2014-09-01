#include <QFile>
#include "hd44780.h"



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
//  CHD44780 - constructor
//-------------------------------------------------


CHD44780::CHD44780(QString fnCharSet,QObject *parent) :
    QObject(parent)
{
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
    in.readRawData ((char *) &charset,0x10000 );
    return true;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void CHD44780::Reset()
{
    memset(info.m_video_ram, 0x00, sizeof(info.m_video_ram));
    memset(info.m_par, 0x00, sizeof(info.m_par));
    memset(info.m_custom_char, 0x00, sizeof(info.m_custom_char));
    memset(info.m_cursor, 0x00, sizeof(info.m_cursor));
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
    info.m_cursor_x = 0;
    info.m_cursor_y = 0;
    info.m_cursor_lcd = 0;
    info.m_contrast = 0;
    on_timer_rate = 8192;
}



//**************************************************************************
//  device interface
//**************************************************************************
#if 0
int CHD44780::video_update(bitmap_t &bitmap, const rectangle &cliprect)
{
    UINT8 cw = info.m_char_width;

    bitmap_fill(&bitmap, &cliprect, 0);

    if (info.m_control_lines&0x80 && info.m_lcd_on)
    {
        for (int a=0; a<2; a++)
            for (int py=0; py<4; py++)
                for (int px=0; px<16; px++)
                    if (BIT(info.m_cursor_status, 4) && px == info.m_cursor_x && py == info.m_cursor_y && a == info.m_cursor_lcd)
                    {
                        //draw the cursor
                        for (int c=0; c<cw; c++)
                        {
                            UINT8 d = compute_newval((info.m_cursor_status>>5) & 0x07, info.m_video_ram[a][py*16*cw + px*cw + c + info.m_scroll * 48], info.m_cursor[c]);
                            for (int b=0; b<8; b++)
                            {
                                *BITMAP_ADDR16(&bitmap, py*8 + b, a*cw*16 + px*cw + c) = BIT(d, 7-b);
                            }
                        }
                    }
                    else
                    {
                        for (int c=0; c<cw; c++)
                        {
                            UINT8 d = info.m_video_ram[a][py*16*cw + px*cw + c + info.m_scroll * 48];
                            for (int b=0; b<8; b++)
                            {
                                *BITMAP_ADDR16(&bitmap, py*8 + b, a*cw*16 + px*cw + c) = BIT(d, 7-b);
                            }
                        }
                    }
    }

    return 0;
}
#endif

void CHD44780::control_write(UINT8 data)
{
    if(info.m_control_lines == data)
        info.m_state = 0;

    info.m_control_lines = data;
}

UINT8 CHD44780::compute_newval(UINT8 type, UINT8 oldval, UINT8 newval)
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

UINT8 CHD44780::get_char(UINT16 pos)
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

void CHD44780::data_write(UINT8 data)
{
    // verify that controller is active
    if (!(info.m_control_lines&0x80))
        return;

    if (info.m_control_lines & 0x01)
    {
        if (!(info.m_control_lines&0x02) && !(info.m_control_lines&0x04))
            return;

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
                    if (info.m_state == 1)
                        info.m_cursor_status = data;
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
                    if (info.m_state == 1)
                        info.m_cursor_lcd = BIT(data, 4);	//0:left lcd 1:right lcd;
                    else if (info.m_state == 2)
                        info.m_cursor_x = ((data>>1)&0x3f) % 48 + (BIT(data,7) * 48);
                    else if (info.m_state == 3)
                    {
                        info.m_cursor_y = data & 0x03;
                        info.m_state = 0;
                    }

                    info.m_data_bus = 0xff;
                }
                break;
        }

        info.m_byte_count = 0;
        info.m_data_bus = 0xff;
    }
    else
    {
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
                    info.m_video_ram[info.m_bank][info.m_offset] = compute_newval((info.m_par[0]>>5) & 0x07, info.m_video_ram[info.m_bank][info.m_offset], data);
                    info.m_offset++; info.m_byte_count++;

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
                    info.m_cursor[info.m_byte_count] = data;
                    info.m_byte_count++;
                    info.m_data_bus = 0xff;
                }
                break;
            case LCD_CURSOR_CHAR:
                if (info.m_byte_count<1)
                {
                    UINT8 char_code = ((data<<4)&0xf0) | ((data>>4)&0x0f);

                    for (int i=0; i<8; i++)
                        info.m_cursor[i] = get_char(char_code*8 + i);

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

UINT8 CHD44780::data_read()
{
    return info.m_data_bus;
}

HD44780info CHD44780::getInfo()
{
    return info;
}

