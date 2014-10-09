#include <QPainter>
#include "common.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "sharp/ce1560.h"
#include "Lcdc_ce1560.h"
#include "Lcdc_symb.h"

Clcdc_ce1560::Clcdc_ce1560(CPObject *parent )	: Clcdc(parent){						//[constructor]
    Color_Off.setRgb(
                        (int) (64*contrast),
                        (int) (86*contrast),
                        (int) (79*contrast));
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
    if (!(((Cce1560 *)pPC)->ps6b0108[0]->updated ||
          ((Cce1560 *)pPC)->ps6b0108[1]->updated ||
          ((Cce1560 *)pPC)->ps6b0108[2]->updated)) return;

    ((Cce1560 *)pPC)->ps6b0108[0]->updated = false;
    ((Cce1560 *)pPC)->ps6b0108[1]->updated = false;
    ((Cce1560 *)pPC)->ps6b0108[2]->updated = false;

    Refresh = true;

    QPainter painter(pPC->LcdImage);
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
                        //if (((data>>b)&0x01) && (pPC->pCPU->fp_log)) fprintf(pPC->pCPU->fp_log,"PSET [%i,%i]\n",i,j*8+b);
                        painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );

                        int y = computeSL(((Cce1560 *)pPC)->ps6b0108[_m],j*8+b);
                        if ((y>=0)&&(y < 64)) painter.drawPoint( _m*64+i, y );
                    }
                }
            }
        }
        else {
            // Turn off screen
            painter.setPen( Color_Off );
            painter.setBrush(Color_Off);
            painter.drawRect( _m*64, 0,64,64);
        }
    }

    redraw = 0;
    painter.end();
}


