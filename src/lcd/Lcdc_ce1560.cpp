#include <QPainter>
#include "common.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "sharp/ce1560.h"
#include "Lcdc_ce1560.h"
#include "Lcdc_symb.h"

//TODO: A way to improve performance is to keep a memory image of pixels to redraw only modified pixels.

Clcdc_ce1560::Clcdc_ce1560(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]

    pixelSize = 4;
    pixelGap = 1;
    internalSize = QSize(192,64);

}

bool	Clcdc_ce1560::init(void)
{
    Clcdc::init();
    TurnON();

    memset(pixels,-1,sizeof(pixels));

    return true;
}

void Clcdc_ce1560::disp_symb(void)
{
    Clcdc::disp_symb();
}
INLINE int Clcdc_ce1560::symbSL(int x)
{
    Q_UNUSED(x)

    return 0;
}

INLINE int Clcdc_ce1560::computeSL(CHD61102* pCtrl,int ord)
{
    int y = ord;
    y -= pCtrl->info.displaySL;
    if (y < 0) y += 64;
    return y;
}

void Clcdc_ce1560::disp(void)
{


    BYTE b;

    Refresh = false;

    if (!ready) return;
    if (!((Cce1560 *)pPC)->ps6b0108[0] ||
        !((Cce1560 *)pPC)->ps6b0108[1] ||
        !((Cce1560 *)pPC)->ps6b0108[2]) return;

    if (!redraw) {
        if (!(((Cce1560 *)pPC)->ps6b0108[0]->updated ||
              ((Cce1560 *)pPC)->ps6b0108[1]->updated ||
              ((Cce1560 *)pPC)->ps6b0108[2]->updated)) return;
    }

    redraw = false;
    ((Cce1560 *)pPC)->ps6b0108[0]->updated = false;
    ((Cce1560 *)pPC)->ps6b0108[1]->updated = false;
    ((Cce1560 *)pPC)->ps6b0108[2]->updated = false;

    Refresh = true;
    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    for (int _m=0; _m<3 ; _m++) {
        if (((Cce1560 *)pPC)->ps6b0108[_m]->info.on_off)
        {
            for (int i = 0 ; i < 64; i++)
            {
                for (int j = 0 ; j < 8 ; j++)
                {
                    BYTE data = ((Cce1560 *)pPC)->ps6b0108[_m]->info.imem[ (j * 0x40) + i ];
                    for (b=0; b<8;b++)
                    {
                        int y = computeSL(((Cce1560 *)pPC)->ps6b0108[_m],j*8+b);
                        if ((y>=0)&&(y < 64)) {
                            int _x = _m*64+i;
                            int _col = ((data>>b)&0x01);
                            if (pixels[_x][y] != _col)
                            {
                                drawPixel(&painter,_x, y,_col ? Color_On : Color_Off );
                                pixels[_x][y] = _col;
                            }
                        }
                    }
                }
            }
        }
        else {
            // Turn off screen
            for (int i=0;i<64;i++)
                for (int j=0;j<64;j++) {
                    int _x = _m*64+i;
                    if (pixels[_x][j]!=0) {
                        drawPixel(&painter,_x, j,Color_Off );
                        pixels[_x][j] = 0;
                    }
                }
        }
    }

    painter.end();
}


