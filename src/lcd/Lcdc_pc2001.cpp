#include <QPainter>
#include <QDebug>

#include "common.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "pc2001.h"
#include "upd16434.h"


#include "Lcdc_pc2001.h"
//#include "Lcdc_symb.h"
#include "Log.h"

Clcdc_pc2001::Clcdc_pc2001(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]

    internalSize = QSize(240,21);
    pixelSize = 4;
    pixelGap = 1;
}

void Clcdc_pc2001::disp_symb(void)
{

    Clcdc::disp_symb();
}

void Clcdc_pc2001::disp(void)
{

    Cpc2001 *pc2001 = (Cpc2001*)pPC;

    BYTE b;

    Refresh = false;


    if (!ready) return;
    if (!pc2001->upd16434[0]) return;

    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    bool _toRefresh = false;
    for (int i = 0 ; i<4; i++)
    {
        if (!pc2001->upd16434[i]->updated) continue;
        Refresh = true;
        for (int j = 0; j< 0x32;j++)
        {
            BYTE data = pc2001->upd16434[i]->info.imem[0x31 - j];
            for (b=0;b<8;b++)
            {
                drawPixel(&painter,j + j/5 + i*60,b + (b==7),((data>>b)&0x01) ? Color_On : Color_Off );
            }
        }
        for (int j = 0; j< 0x32;j++)
        {
            BYTE data = pc2001->upd16434[i]->info.imem[0x71-j];
            for (b=0;b<8;b++)
            {
                drawPixel(&painter,j + j/5 + i*60,b + (b==7)+12,((data>>b)&0x01) ? Color_On : Color_Off );
            }
        }
        _toRefresh = true;
        pc2001->upd16434[i]->updated = false;
    }

//    Refresh = _toRefresh;
    redraw = 0;
    painter.end();
}



