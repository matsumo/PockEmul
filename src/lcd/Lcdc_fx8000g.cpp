#include <QPainter>
#include "common.h"
#include "pcxxxx.h"
#include "casio/fx8000g.h"
#include "hd44352.h"
#include "Lcdc_fx8000g.h"
#include "Lcdc_symb.h"


#define BIT(x,n) (((x)>>(n))&1)

Clcdc_fx8000g::Clcdc_fx8000g(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]

    Color_Off = Qt::transparent;
    internalSize = QSize(96,64);
    pixelSize = 4;
    pixelGap = 1;
}


void Clcdc_fx8000g::disp_symb(void)
{
    Refresh = true;

    Clcdc::disp_symb();
}

bool Clcdc_fx8000g::init()
{
    Clcdc::init();
    ((Cfx8000g*) pPC)->pHD44352->Reset();
    return true;
}

void Clcdc_fx8000g::TurnOFF()
{
    Clcdc::TurnOFF();

    ((Cfx8000g*) pPC)->pHD44352->getInfoRef()->m_lcd_on = 0;
    Refresh = true;
    redraw = true;
    pPC->InitDisplay();

}

void Clcdc_fx8000g::disp(void)
{
    if (!redraw) return;
    if (!ready) return;
    if (!((Cfx8000g *)pPC)->pHD44352 ) return;
    redraw = 0;
    info = ((Cfx8000g*) pPC)->pHD44352->getInfo();



    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    UINT8 cw = info.m_char_width;

    if (info.m_control_lines&0x80 && info.m_lcd_on)
    {
        for (int a=0; a<2; a++) {
            for (int py=0; py<4; py++) {
                for (int px=0; px<16; px++) {
                    if (
                            !BIT(info.m_cursor_status, 1)
                            &&
                            px == (info.cursor[a].m_cursor_x/cw/2)
                            &&
                            py == info.cursor[a].m_cursor_y
                            &&
                            0 == info.cursor[a].m_cursor_lcd
                            )
                    {
//                        qWarning()<<"********* draw cursor"<<a<<px<<(info.cursor[a].m_cursor_x/cw)<<" lcd:"<<info.cursor[a].m_cursor_lcd;
                        //draw the cursor
                        for (int c=0; c<cw; c++)
                        {
//                            UINT8 d1 = CHD44352::compute_newval((info.m_cursor_status>>5) & 0x07,
//                                                               info.m_video_ram[a][2*(py*16*cw + px*cw + c + info.m_scroll * 48)]& 0x0f,
//                                                                info.cursor[a].m_cursor[2*c]);
//                            UINT8 d2 = CHD44352::compute_newval((info.m_cursor_status>>5) & 0x07,
//                                                               info.m_video_ram[a][2*(py*16*cw + px*cw + c + info.m_scroll * 48)+1]& 0x0f,
//                                                                info.cursor[a].m_cursor[2*c+1]);
                            UINT8 d1 = info.cursor[a].m_cursor[2*c];
                            UINT8 d2 = info.cursor[a].m_cursor[2*c+1];
                            UINT8 d = d1 | (d2<<4);
                            for (int b=0; b<8; b++)
                            {
                                drawPixel(&painter,px*cw + c,a*32 + py*8 + b,(BIT(d, 7-b)) ? Color_On : Color_Off);
                            }
                        }
                    }
                    else
                    {
                        for (int c=0; c<cw; c++)
                        {
                            UINT8 d1 = info.m_video_ram[a][2*(py*16*cw + px*cw + c + info.m_scroll * 48)] & 0x0f;
                            UINT8 d2 = info.m_video_ram[a][2*(py*16*cw + px*cw + c + info.m_scroll * 48)+1] & 0x0f;
                            UINT8 d = d1 | (d2<<4);
                            for (int b=0; b<8; b++)
                            {
                                drawPixel(&painter,px*cw + c,a*32 + py*8 + b,(BIT(d, 7-b)) ? Color_On : Color_Off);
                            }
                        }
                    }
                }
            }
        }
    }
    Refresh = true;

    painter.end();
}


