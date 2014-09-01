#include <QPainter>

#include "ti/cc40.h"
#include "Lcdc_cc40.h"
#include "Lcdc_symb.h"

#define BIT(x,n) (((x)>>(n))&1)

Clcdc_cc40::Clcdc_cc40(CPObject *parent)	: Clcdc(parent){						//[constructor]

    Color_Off.setRgb(
                        (int) (90*contrast),
                        (int) (108*contrast),
                        (int) (99*contrast));
}


void Clcdc_cc40::disp_symb(void)
{
    Refresh = true;

    Clcdc::disp_symb();
}



void Clcdc_cc40::disp(void)
{
    if (!redraw) return;
    if (!ready) return;
    if (!((Ccc40 *)pPC)->pHD44780 ) return;
    Refresh = true;
    info = ((Ccc40*) pPC)->pHD44780->getInfo();
//    disp_symb();

    QPainter painter(pPC->LcdImage);

    UINT8 cw = info.m_char_width;


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
                            UINT8 d = CHD44780::compute_newval((info.m_cursor_status>>5) & 0x07, info.m_video_ram[a][py*16*cw + px*cw + c + info.m_scroll * 48], info.m_cursor[c]);
                            for (int b=0; b<8; b++)
                            {
                                painter.setPen((BIT(d, 7-b)) ? Color_On : Color_Off );
                                painter.drawPoint( a*cw*16 + px*cw + c, py*8 + b );
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
                                painter.setPen((BIT(d, 7-b)) ? Color_On : Color_Off );
                                painter.drawPoint( a*cw*16 + px*cw + c, py*8 + b );
                            }
                        }
                    }
    }

    redraw = 0;
    painter.end();
}


