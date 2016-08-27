#include <QPainter>
#include <QDebug>

#include "common.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "epson/hx20.h"
#include "upd16434.h"


#include "Lcdc_hx20.h"
#include "Log.h"

Clcdc_hx20::Clcdc_hx20(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]

    internalSize = QSize(120,32);
    pixelSize = 4;
    pixelGap = 1;
}

void Clcdc_hx20::disp_symb(void)
{

    Clcdc::disp_symb();
}

void Clcdc_hx20::disp(void)
{

    Chx20 *hx20 = (Chx20*)pPC;

    BYTE b;

//    Refresh = false;


    if (!ready) return;
    if (!hx20->upd16434[0]) return;


    if (redraw) {
        for (int i = 0 ; i<6; i++)
        {
            hx20->upd16434[i]->updated=true;
        }
    }

    lock.lock();

    redraw=false;
    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    bool _toRefresh = false;
    for (int r = 0 ; r<2; r++)
    {
        for (int i = 0 ; i<3; i++)
        {
            if (!hx20->upd16434[r*3+i]->updated) continue;
            Refresh = true;
            for (int j = 0; j< 40;j++)
            {
                BYTE data = hx20->upd16434[r*3+i]->info.imem[j];
                for (b=0;b<8;b++)
                {
                    drawPixel(&painter,j + i*40,b + r*16,((data>>b)&0x01) ? Color_On : Color_Off );
                }
            }
            for (int j = 0; j< 40;j++)
            {
                BYTE data = hx20->upd16434[r*3+i]->info.imem[0x40+j];
                for (b=0;b<8;b++)
                {
                    drawPixel(&painter,j + i*40,b + r*16 + 8,((data>>b)&0x01) ? Color_On : Color_Off );
                }
            }
            _toRefresh = true;
            hx20->upd16434[r*3+i]->updated = false;
        }
    }

//    Refresh = _toRefresh;

    painter.end();

    lock.unlock();
}




