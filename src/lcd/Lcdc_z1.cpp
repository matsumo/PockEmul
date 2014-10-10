#include <QPainter>
#include "common.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "casio/z1.h"
#include "hd66108.h"
#include "Lcdc_z1.h"
#include "Log.h"

Clcdc_z1::Clcdc_z1(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]
    Color_Off.setRgb(
                        (int) (92*contrast),
                        (int) (120*contrast),
                        (int) (103*contrast));
}


#define pset(x,y,dot) {    painter.setPen( dot ? Color_On : Color_Off ); \
                           painter.drawPoint( x, y ); \
                        }

void Clcdc_z1::disp(void)
{

    Cz1 *z1 = (Cz1*)pPC;

    BYTE b;

    Refresh = false;


    if (!ready) return;
    if (!z1->pHD66108) return;
    if (!z1->pHD66108->updated) return;
    z1->pHD66108->updated = false;

    AddLog(LOG_DISPLAY,"-----------REFRESH DISP");
    if(z1->pCPU->fp_log) fprintf(z1->pCPU->fp_log,"REFRESH DISP\n");
    Refresh = true;

    QPainter painter(LcdImage);

    int x, y;
    UINT8 *p = z1->pHD66108->vram;

    for(y = 0; y < LCD_HEIGHT; y++)
        for(x = 0; x < LCD_WIDTH; x += 8) {
            pset(x + 0, y, *p & 0x80);
            pset(x + 1, y, *p & 0x40);
            pset(x + 2, y, *p & 0x20);
            pset(x + 3, y, *p & 0x10);
            pset(x + 4, y, *p & 0x08);
            pset(x + 5, y, *p & 0x04);
            pset(x + 6, y, *p & 0x02);
            pset(x + 7, y, *p & 0x01);
            p++;
        }

    redraw = 0;
    painter.end();
}



