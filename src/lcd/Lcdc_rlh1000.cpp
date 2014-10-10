#include <QDebug>
#include <QPainter>

#include "Lcdc_rlh1000.h"
#include "Lcdc_symb.h"
#include "pcxxxx.h"

static const struct {
    int x,y;
} rlh1000_pos[15]={
    {0, 0},
    {30, 0},
    {60, 0},
    {80, 0},
    {112,0}, //de
    {120,0},
    {125,0},
    {160,0}, // run
    {190,0},
    {210,0},
    {250,0},
    {276,0}, // i
    {282,0},
    {292,0},
    {312,0}
};



/* busy  shift   small   de g rad   run  pro  reserve  def  i ii iii battery */
/* japanese? */
#define SYMB1_ADR_rlh1000	0x764E
#define SYMB2_ADR_rlh1000	0x764F
#define SYMB1_rlh1000		(pPC->Get_8(SYMB1_ADR_rlh1000))
#define SYMB2_rlh1000		(pPC->Get_8(SYMB2_ADR_rlh1000))

void Clcdc_rlh1000::disp_symb(void)
{

    Clcdc::disp_symb();
}

Clcdc_rlh1000::Clcdc_rlh1000(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]

    internalSize = QSize(159,8);
    pixelSize = 4;
    pixelGap = 1;
    memset((void *)mem,0,sizeof(mem));
}



void Clcdc_rlh1000::disp(void)
{

    BYTE b,data;
    int ind;

    Refresh = false;

    //	disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    On=true;
    for (ind=0; ind<0xA0; ind++)
    {
        if ( (DirtyBuf[ind]) )
        {

            Refresh = true;
            if (On)
            {
                data = mem[ind];
                DirtyBuf[ind] = 0;
            }
            else
            {
                data = 0;
                DirtyBuf[ind] = 0;
            }

            for (b=0; b<8;b++)
            {
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( ind, b);
                drawPixel(&painter,ind, b,((data>>b)&0x01) ? Color_On : Color_Off );
            }
        }
    }

    redraw = 0;
    painter.end();
}

