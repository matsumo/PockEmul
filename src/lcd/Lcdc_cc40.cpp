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

//    QImage loc = QImage(QSize(16*6,18),QImage::Format_ARGB32);
    QImage loc = QImage(pPC->LcdFname).scaled(QSize(16*6,18),Qt::IgnoreAspectRatio,Qt::SmoothTransformation).convertToFormat(QImage::Format_ARGB32);
    QPainter painter(&loc);

    info->m_lines = 2;
    info->m_chars = 16;
    ((Ccc40 *)pPC)->pHD44780->screen_update(&painter);
    painter.end();

    QPainter painterFinal(pPC->LcdImage);

    painterFinal.drawImage(QRect(0,0,16*6,9),loc,QRect(0,0,16*6,9));
    painterFinal.drawImage(QRect(16*6,0,16*6,9),loc,QRect(0,9,16*6,9));
    painterFinal.end();

    redraw = 0;

}


