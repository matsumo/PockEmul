#include <QPainter>
#include <QDebug>

#include "common.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "general/lbc1100.h"
#include "upd16434.h"


#include "Lcdc_lbc1100.h"
#include "Log.h"

Clcdc_lbc1100::Clcdc_lbc1100(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]


    internalSize = QSize(240,21);
    pixelSize = 4;
    pixelGap = 1;
}

void Clcdc_lbc1100::disp_symb(void)
{

    Clcdc::disp_symb();
}


void Clcdc_lbc1100::disp(void)
{

    Clbc1100 *lbc1100 = (Clbc1100*)pPC;

    BYTE b;

    Refresh = false;


    if (!ready) return;
    if (!lbc1100->upd16434[0]) return;


    if (redraw) {
        for (int i = 0 ; i<4; i++)
        {
            lbc1100->upd16434[i]->updated=true;
        }
    }
    redraw=false;


    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    for (int i = 0 ; i<4; i++)
    {
        if (!lbc1100->upd16434[i]->updated) continue;
        Refresh = true;
    //AddLog(LOG_DISPLAY,"DISP");
        lbc1100->upd16434[i]->updated = false;
        for (int j = 0; j< 0x32;j++)
        {
            BYTE data = lbc1100->upd16434[i]->info.imem[0x31 - j];
            for (b=0;b<8;b++)
            {
                drawPixel(&painter,j + j/5 + i*60,b + (b==7),((data>>b)&0x01) ? Color_On : Color_Off );
            }
        }
        for (int j = 0; j< 0x32;j++)
        {
            BYTE data = lbc1100->upd16434[i]->info.imem[0x71-j];
            for (b=0;b<8;b++)
            {
                drawPixel(&painter,j + j/5 + i*60,b + (b==7)+12,((data>>b)&0x01) ? Color_On : Color_Off );
            }
        }
    }

    painter.end();
}




