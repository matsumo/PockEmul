#include <QPainter>
#include <QDebug>

#include "common.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "casio/z1.h"
#include "hd66108.h"
#include "Lcdc_z1.h"
#include "Log.h"

Clcdc_z1::Clcdc_z1(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]

    internalSize = QSize(190,32);
    pixelSize = 4;
    pixelGap = 1;
}


#define pset(x,y,dot) {    drawPixel(&painter,(x),(y),(dot) ? Color_On : Color_Off ); \
                        }

void Clcdc_z1::disp(void)
{

    Cz1 *z1 = (Cz1*)pPC;

    Refresh = false;

    if (!ready) return;
    if (!z1->pHD66108) return;
    if (!z1->pHD66108->updated) return;
    z1->pHD66108->updated = false;

    if(z1->pCPU->fp_log) fprintf(z1->pCPU->fp_log,"REFRESH DISP\n");
    Refresh = true;

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    UINT8 *p = z1->pHD66108->vram;

    for(int y = 0; y < 32; y++)
        for(int x = 0; x < 192; x += 8) {
            for (int b=0;b<8;b++) {
                drawPixel(&painter,x+b,y,(*p & (0x01<<(7-b))) ? Color_On : Color_Off );
            }
            p++;
        }
    redraw = 0;
    painter.end();
}



