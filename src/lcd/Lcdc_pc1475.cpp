#include <QPainter>
#include "pcxxxx.h"
#include "Lcdc_pc1475.h"
#include "Lcdc_symb.h"

// TODO: seems to be very close of the PC-1262. Try to mix them.

/*******************/
/***	1475	****/
/*******************/

Clcdc_pc1475::Clcdc_pc1475(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]


    internalSize = QSize(132,15);
    pixelSize = 4;
    pixelGap = 1;
    baseAdr = 0x2800;

    symbList.clear();
    symbList << ClcdSymb(0  , 0, S_BUSY	,0x283D	,0x01)
             << ClcdSymb(24 , 0, S_DBL	,0x283D	,0x08)
             << ClcdSymb(44 , 0, S_DEF	,0x283D	,0x02)
             << ClcdSymb(61 , 0, S_SHIFT,0x283D	,0x04)
             << ClcdSymb(90 , 0, S_JAP	,0x287D	,0x01)
             << ClcdSymb(105, 0, S_JAP2	,0x287D	,0x02)
             << ClcdSymb(115, 0, S_SML	,0x287D	,0x04)
             << ClcdSymb(137, 0, S_DE	,0x287D	,0x08)
             << ClcdSymb(145, 0, S_G	,0x287C	,0x08)
             << ClcdSymb(150, 0, S_RAD	,0x287C	,0x04)
             << ClcdSymb(175, 0, S_HYP	,0x283C	,0x04)
             << ClcdSymb(190, 0, S_O_BRA,0x283C	,0x02)
             << ClcdSymb(195, 0, S_C_BRA,0x283C	,0x02)
             << ClcdSymb(200, 0, S_REV_M,0x287C	,0x02)
             << ClcdSymb(210, 0, S_E    ,0x287C	,0x01)
             << ClcdSymb(220, 0, S_REV_BATT	,0x283C	,0x01)
             << ClcdSymb(15, 43, S_BAR25,0x283C	,0x40)      // CAL
             << ClcdSymb(40, 43, S_BAR25,0x283C	,0x08)      // RSV
             << ClcdSymb(67, 43, S_BAR25,0x283C	,0x20)      // RUN
             << ClcdSymb(87,43, S_BAR25,0x283C	,0x10)      // PRO
             << ClcdSymb(180,43, S_BAR25,0x287C	,0x10)      // MATRIX
             << ClcdSymb(200,43, S_BAR25,0x287C	,0x20)      // STAT
             << ClcdSymb(220,43, S_BAR25,0x287C	,0x40);     // PRINT
}

bool	Clcdc_pc1475::init(void)
{

    Clcdc::init();

    return true;
}

void Clcdc_pc1475::disp(void)
{
    BYTE b,data;
    int x,y;
    int ind;
    WORD adr;

    if (!redraw) {
        if (!updated) return;
    }
    redraw = false;
    updated = false;

    Refresh = false;
    disp_symb();
    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

#if 1
    for (ind=0; ind<0x3c; ind++)
    {	adr = 0x2800 + ind;
        if (DirtyBuf[adr-0x2800])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);

            x =(ind*2) + (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x/2.0f,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
            }
            DirtyBuf[adr-0x2000] = 0;
        }
    }
#endif
#if 1
    for (ind=0; ind<0x3c; ind++)
    {	adr = 0x2800 + 0x40 + ind;
        if (DirtyBuf[adr-0x2800])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);

            x =(ind*2) + (ind/5);			// +1 every 5 cols
            y = 8;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x/2.0f,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
            }
            DirtyBuf[adr-0x2000] = 0;
        }
    }
#endif
#if 1
    for (ind=0; ind<0x3c; ind++)
    {	adr = 0x2A00 + ind;
        if (DirtyBuf[adr-0x2800])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);

            x = 132 + (ind*2) + (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x/2.0f,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
            }
            DirtyBuf[adr-0x2000] = 0;
        }
    }
#endif
#if 1
    for (ind=0; ind<0x3c; ind++)
    {	adr = 0x2A00 + 0x40 + ind;
        if (DirtyBuf[adr-0x2800])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);

            x = 132 + (ind*2) + (ind/5);			// +1 every 5 cols
            y = 8;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x/2.0f,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
            }
            DirtyBuf[adr-0x2000] = 0;
        }
    }
#endif
    updated = 0;
    painter.end();
}


