#include <QPainter>

#include "ti/ti74.h"
#include "Lcdc_ti74.h"
#include "Lcdc_symb.h"

#define BIT(x,n) (((x)>>(n))&1)


Clcdc_ti74::Clcdc_ti74(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]

    internalSize = QSize(186,10);
    pixelSize = 4;
    pixelGap = 1;
}

Clcdc_ti95::Clcdc_ti95(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc_ti74(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){

    internalSize = QSize(130,30);
    pixelSize = 4;
    pixelGap = 1;
}

void Clcdc_ti74::disp_symb(void)
{
    Refresh = true;

    Clcdc::disp_symb();


}

#undef COLOR
#define COLOR(b)	( ( (b) ) ? Color_On : Color_Off)
HD44780_PIXEL_UPDATE(Cti74_update_pixel_symb)
{
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    if (line == 1 && pos == 15)
    {
        // TI-74 ref._________________...
        // output#  |10     11     12     13     14      2      3      4
        // above    | <    SHIFT   CTL    FN     I/O    UCL    _LOW    >
        // ---- raw lcd screen here ----
        // under    |      BASIC   CALC   DEG    RAD    GRAD   STAT
        // output#  |       63     64      1     62     53     54
        int id = y*10+x;
        switch (id) {
        case 10: Clcdc::disp_one_symb(painter,S_INF, COLOR(state),	  0,	0); break;
        case 11: Clcdc::disp_one_symb(painter,S_SHIFT, COLOR(state), 20,	0); break;
        case 12: Clcdc::disp_one_symb(painter,S_CTL, COLOR(state),	 50,	0); break;
        case 13: Clcdc::disp_one_symb(painter,S_FN,  COLOR(state),	 80,	0); break;
        case 14:  Clcdc::disp_one_symb(painter,S_IO,  COLOR(state),	110,	0); break;
        case 2:  Clcdc::disp_one_symb(painter,S_UCL, COLOR(state),	140,	0); break;
        case 3:  Clcdc::disp_one_symb(painter,S_LOW, COLOR(state),	170,	0); break;
        case 4:  Clcdc::disp_one_symb(painter,S_SUP, COLOR(state),	205,	0); break;


//        case 63: Clcdc::disp_one_symb(painter,S_BASIC, COLOR(state), 20,	18); break;
//        case 64: Clcdc::disp_one_symb(painter,S_CALC, COLOR(state),	 50,	18); break;
        case 1: Clcdc::disp_one_symb(painter,S_DEG, COLOR(state),	 80,	18); break;
        case 62: Clcdc::disp_one_symb(painter,S_RAD, COLOR(state),	110,	18); break;
        case 53: Clcdc::disp_one_symb(painter,S_G,   COLOR(state),	140,	18);
                 Clcdc::disp_one_symb(painter,S_RAD, COLOR(state),	145,	18); break;
        case 54: Clcdc::disp_one_symb(painter,S_STAT, COLOR(state),	170,	18); break;

        }
    }
}

HD44780_PIXEL_UPDATE(Cti74_update_pixel)
{
    // char size is 5x7 + cursor
    if (x > 4 || y > 7)
        return;
    if (line == 1 && pos == 15)
    {
    }
    else if (line < 2 && pos < 16)
    {
        // internal: 2*16, external: 1*31 + indicators
        if (y == 7) y++;
//        painter->setPen(COLOR(state));
//        painter->drawPoint( 1 + line*16*6 + pos*6 + x, 1 + y );
        plcd->drawPixel(painter, 1 + line*16*6 + pos*6 + x, 1 + y ,COLOR(state));
    }
}



HD44780_PIXEL_UPDATE(Cti95_update_pixel_symb)
{
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    if (line == 1 && pos == 15)
    {
        // output#  |  40   43     41   44   42     12  11  10/13/14  0    1    2
        // above    | _LOW _ERROR  2nd  INV  ALPHA  LC  INS  DEGRAD  HEX  OCT  I/O
        // screen-  | _P{70} <{71}                                             RUN{3}
        //   area   .                                                          SYS{4}

        int id = y*10+x;
        switch (id) {
        case 40: Clcdc::disp_one_symb(painter,S_LOW, COLOR(state),	  0,	0); break;
        case 43: Clcdc::disp_one_symb(painter,S_ERROR, COLOR(state), 20,	0); break;
        case 41: Clcdc::disp_one_symb(painter,S_SEC, COLOR(state),	 50,	0); break;
//        case 44: Clcdc::disp_one_symb(painter,S_INV,  COLOR(state),	 80,	0); break;
//        case 42:  Clcdc::disp_one_symb(painter,S_ALPHA,  COLOR(state),	110,	0); break;
//        case 12:  Clcdc::disp_one_symb(painter,S_LC, COLOR(state),	140,	0); break;
//        case 11:  Clcdc::disp_one_symb(painter,S_INS, COLOR(state),	140,	0); break;
        case 10:  Clcdc::disp_one_symb(painter,S_DEG, COLOR(state),	170,	0); break;
        case 13:  Clcdc::disp_one_symb(painter,S_G, COLOR(state),	170,	0); break;
        case 14:  Clcdc::disp_one_symb(painter,S_RAD, COLOR(state),	170,	0); break;
//        case 0:  Clcdc::disp_one_symb(painter,S_HEX, COLOR(state),	170,	0); break;
//        case 1:  Clcdc::disp_one_symb(painter,S_OCT, COLOR(state),	170,	0); break;
        case 2:  Clcdc::disp_one_symb(painter,S_IO, COLOR(state),	205,	0); break;

        case 3:  Clcdc::disp_one_symb(painter,S_LOW, COLOR(state),	170,	0); break;
        case 4:  Clcdc::disp_one_symb(painter,S_LOW, COLOR(state),	170,	0); break;
        case 70:  Clcdc::disp_one_symb(painter,S_LOW, COLOR(state),	170,	0); break;
        case 71:  Clcdc::disp_one_symb(painter,S_LOW, COLOR(state),	170,	0); break;


//        case 63: Clcdc::disp_one_symb(painter,S_BASIC, COLOR(state), 20,	18); break;
//        case 64: Clcdc::disp_one_symb(painter,S_CALC, COLOR(state),	 50,	18); break;
        case 1: Clcdc::disp_one_symb(painter,S_DEG, COLOR(state),	 80,	18); break;
        case 62: Clcdc::disp_one_symb(painter,S_RAD, COLOR(state),	110,	18); break;
        case 53: Clcdc::disp_one_symb(painter,S_G,   COLOR(state),	140,	18);
                 Clcdc::disp_one_symb(painter,S_RAD, COLOR(state),	145,	18); break;
        case 54: Clcdc::disp_one_symb(painter,S_STAT, COLOR(state),	170,	18); break;

        }
    }
}

HD44780_PIXEL_UPDATE(Cti95_update_pixel)
{
    // char size is 5x7 + cursor
    if (x > 4 || y > 7)
        return;

    if (line == 1 && pos == 15)
    {
    }
    else if (line == 0 && pos < 16)
    {
        // 1st line is simply 16 chars
        if (y == 7) y++;
        plcd->drawPixel(painter, 15 + pos*6 + x, 7 + y ,COLOR(state));
    }
    else if (line == 1 && pos < 15 && y < 7)
    {
        // 2nd line is segmented into 5 groups of 3 chars, there is no cursor
        int group = pos / 3;
        int offset[5]={1,5,9,14,18};
        plcd->drawPixel(painter,  7 +offset[group] + pos*6 + x, 20 + y  ,COLOR(state));
    }
}


void Clcdc_ti74::disp(void)
{
    CHD44780 * pHD44780 = ((Cti74*) pPC)->pHD44780;
    if (!ready) return;
    if (!pHD44780 ) return;
    if (!updated) return;
    updated = false;
    lock.lock();
    Refresh = true;
    info = pHD44780->getInfo();
//    disp_symb();


    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    info->m_lines = 2;
    info->m_chars = 16;
    pHD44780->set_pixel_update_cb(&Cti74_update_pixel);
    pHD44780->screen_update(this,&painter,Color_On,Color_Off);
    painter.end();


    QPainter painterSymb(SymbImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    info->m_lines = 2;
    info->m_chars = 16;
    pHD44780->set_pixel_update_cb(&Cti74_update_pixel_symb);
    pHD44780->screen_update(this,&painterSymb,Color_On,Color_Off);
    painterSymb.end();



    lock.unlock();
}


void Clcdc_ti95::disp(void)
{
    CHD44780 * pHD44780 = ((Cti95*) pPC)->pHD44780;
    if (!ready) return;
    if (!pHD44780 ) return;
    if (!redraw) {
        if (!updated) return;
    }
    if (!updated) return;
    updated = false;

    lock.lock();
    redraw = false;
    Refresh = true;
    info = pHD44780->getInfo();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    info->m_lines = 2;
    info->m_chars = 16;
    pHD44780->set_pixel_update_cb(&Cti95_update_pixel);
    pHD44780->screen_update(this,&painter,Color_On,Color_Off);
    painter.end();


    lock.unlock();
}
