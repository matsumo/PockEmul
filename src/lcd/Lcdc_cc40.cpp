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

    ((Ccc40 *)pPC)->pHD44780->screen_update(&painter);


    redraw = 0;
    painter.end();
}


