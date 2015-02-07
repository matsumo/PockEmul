#include <QPainter>
#include <QDebug>

#include "common.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "jr800.h"
#include "hd44102.h"


#include "Lcdc_jr800.h"
//#include "Lcdc_symb.h"
#include "Log.h"

Clcdc_jr800::Clcdc_jr800(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]

    internalSize = QSize(192,64);
    pixelSize = 4;
    pixelGap = 1;
}

void Clcdc_jr800::disp_symb(void)
{

    Clcdc::disp_symb();
}

void Clcdc_jr800::disp(void)
{

    Cjr800 *jr800 = (Cjr800*)pPC;

    BYTE b;

//    Refresh = false;


    if (!ready) return;
    if (!jr800->hd44102[0]) return;


    if (redraw) {
        for (int i = 0 ; i<8; i++)
        {
            jr800->hd44102[i]->updated=true;
        }
    }
    redraw=false;
    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    bool _toRefresh = false;
    for (int i = 0 ; i<8; i++)
    {
        if (!jr800->hd44102[i]->updated) continue;
        Refresh = true;
        for (int l=0; l<4;l++)
        {
            for (int j = 0; j< 50;j++)
            {
                UINT8 data = jr800->hd44102[i]->info.imem[l][j];
                for (b=0;b<8;b++)
                {
                    drawPixel(&painter,j + i*50, i/4*32 + l*8 + b,((data>>b)&0x01) ? Color_On : Color_Off );
                }
            }
        }
        _toRefresh = true;
        jr800->hd44102[i]->updated = false;
    }

//    Refresh = _toRefresh;

    painter.end();
}




