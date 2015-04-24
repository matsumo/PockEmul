#include <QPainter>
#include <QDebug>

#include "common.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "jr800.h"
#include "hd44102.h"


#include "lcdc_jr800.h"
#include "Lcdc_symb.h"
#include "Log.h"

Clcdc_jr800::Clcdc_jr800(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]

    internalSize = QSize(192,64);
    pixelSize = 4;
    pixelGap = 1;


}

void Clcdc_jr800::disp_symb(void)
{
    Cjr800 *jr800 = (Cjr800*)pPC;

    int _val =  (jr800->hd44102[0]->info.imem[0][46] ? 0x01 : 0) |
                (jr800->hd44102[0]->info.imem[1][46] ? 0x02 : 0) |
                (jr800->hd44102[0]->info.imem[2][46] ? 0x04 : 0) |
                (jr800->hd44102[0]->info.imem[3][46] ? 0x08 : 0) |
                (jr800->hd44102[4]->info.imem[0][3]  ? 0x10 : 0) |
                (jr800->hd44102[4]->info.imem[1][3]  ? 0x20 : 0) |
                (jr800->hd44102[4]->info.imem[2][3]  ? 0x40 : 0) |
                (jr800->hd44102[4]->info.imem[3][3]  ? 0x80 : 0) ;
    for (int i=0; i<8; i++) {
        disp_one_symb( S_LA, COLOR( _val& (1<<i)),	0,	14.3*i+3);
    }

    int _val2 = (jr800->hd44102[3]->info.imem[0][3]  ? 0x01 : 0) |
                (jr800->hd44102[3]->info.imem[1][3]  ? 0x02 : 0) |
                (jr800->hd44102[3]->info.imem[2][3]  ? 0x04 : 0) |
                (jr800->hd44102[3]->info.imem[3][3]  ? 0x08 : 0) |
                (jr800->hd44102[7]->info.imem[0][46] ? 0x10 : 0) |
                (jr800->hd44102[7]->info.imem[1][46] ? 0x20 : 0) |
                (jr800->hd44102[7]->info.imem[2][46] ? 0x40 : 0) |
                (jr800->hd44102[7]->info.imem[3][46] ? 0x80 : 0) ;
    for (int i=0; i<8; i++) {
        disp_one_symb( S_RA, COLOR(_val2 & (1<<i)),	354,	14.3*i+3);
    }
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

    // DRIVER 0
    if (jr800->hd44102[0]->updated) {
        Refresh = true;
        for (int x = 0; x<46;x++) {
            for (int y=0; y<4;y++) {
                int sy = (jr800->hd44102[0]->info.m_page + y)%4;
                int sx = 45-x;
                UINT8 data = jr800->hd44102[0]->info.imem[sy][x];
                for (b=0;b<8;b++)
                {
                    QColor _col = ((data>>b)&0x01) ? Color_On : Color_Off ;
                    drawPixel(&painter,sx , sy*8 + b,_col);
                }
            }
        }
        jr800->hd44102[0]->updated = false;
    }

    // DRIVER 1
    if (jr800->hd44102[1]->updated) {
        Refresh = true;
        for (int x = 0; x< 50;x++)
        {
            int z = jr800->hd44102[1]->info.m_page;
            for (int y=0; y<4;y++)
            {
                int sy = (z + y)%4;
                int sx = 49-x;
                UINT8 data = jr800->hd44102[1]->info.imem[sy][x];

                for (b=0;b<8;b++)
                {
                    QColor _col = ((data>>b)&0x01) ? Color_On : Color_Off ;
                    drawPixel(&painter,sx + 50 - 4,  sy*8 + b,_col);
                }
            }
        }
        jr800->hd44102[1]->updated = false;
    }

    // DRIVER 2
    if (jr800->hd44102[2]->updated) {
        Refresh = true;
        for (int x = 0; x< 50;x++)
        {
            int z = jr800->hd44102[2]->info.m_page;
            for (int y=0; y<4;y++)
            {
                int sy = (z + y)%4;
                int sx = 49-x;
                UINT8 data = jr800->hd44102[2]->info.imem[sy][x];

                for (b=0;b<8;b++)
                {
                    QColor _col = ((data>>b)&0x01) ? Color_On : Color_Off ;
                    drawPixel(&painter,sx + 100 - 4,  sy*8 + b,_col);
                }
            }
        }
        jr800->hd44102[2]->updated = false;
    }

    // DRIVER 3
    if (jr800->hd44102[3]->updated) {
        Refresh = true;
        for (int x = 4; x< 50;x++)
        {
            int z = jr800->hd44102[3]->info.m_page;
            for (int y=0; y<4;y++)
            {
                int sy = (z + y)%4;
                int sx = 49-x;
                UINT8 data = jr800->hd44102[3]->info.imem[sy][x];

                for (b=0;b<8;b++)
                {
                    QColor _col = ((data>>b)&0x01) ? Color_On : Color_Off ;
                    drawPixel(&painter,sx + 150 - 4,  sy*8 + b,_col);
                }
            }
        }
       jr800->hd44102[3]->updated = false;
    }

    // DRIVER 4
    if (jr800->hd44102[4]->updated) {
        Refresh = true;
        for (int x = 4; x<50;x++) {
            for (int y=0; y<4;y++) {
                int sy = (jr800->hd44102[4]->info.m_page + y)%4;
                int sx = x-4;
                UINT8 data = jr800->hd44102[4]->info.imem[sy][x];
                for (b=0;b<8;b++)
                {
                    QColor _col = ((data>>b)&0x01) ? Color_On : Color_Off ;
                    drawPixel(&painter,sx , 32 + sy*8 + b,_col);
                }
            }
        }
        jr800->hd44102[4]->updated = false;
    }

    // DRIVER 5
    if (jr800->hd44102[5]->updated) {
        Refresh = true;
        for (int x = 0; x< 50;x++)
        {
            int z = jr800->hd44102[5]->info.m_page;
            for (int y=0; y<4;y++)
            {
                int sy = (z + y)%4;
                int sx = x;
                UINT8 data = jr800->hd44102[5]->info.imem[sy][x];

                for (b=0;b<8;b++)
                {
                    QColor _col = ((data>>b)&0x01) ? Color_On : Color_Off ;
                    drawPixel(&painter,sx + 50 - 4, 32 +  sy*8 + b,_col);
                }
            }
        }
        jr800->hd44102[5]->updated = false;
    }

    // DRIVER 6
    if (jr800->hd44102[6]->updated) {
        Refresh = true;
        for (int x = 0; x< 50;x++)
        {
            int z = jr800->hd44102[6]->info.m_page;
            for (int y=0; y<4;y++)
            {
                int sy = (z + y)%4;
                int sx = x;
                UINT8 data = jr800->hd44102[6]->info.imem[sy][x];

                for (b=0;b<8;b++)
                {
                    QColor _col = ((data>>b)&0x01) ? Color_On : Color_Off ;
                    drawPixel(&painter,sx + 100 - 4,32 +   sy*8 + b,_col);
                }
            }
        }
        jr800->hd44102[6]->updated = false;
    }

    // DRIVER 7
    if (jr800->hd44102[7]->updated) {
        Refresh = true;
        for (int x = 0; x< 46;x++)
        {
            int z = jr800->hd44102[7]->info.m_page;
            for (int y=0; y<4;y++)
            {
                int sy = (z + y)%4;
                int sx = x;
                UINT8 data = jr800->hd44102[7]->info.imem[sy][x];

                for (b=0;b<8;b++)
                {
                    QColor _col = ((data>>b)&0x01) ? Color_On : Color_Off ;
                    drawPixel(&painter,sx + 150 - 4, 32 +  sy*8 + b,_col);
                }
            }
        }
        jr800->hd44102[7]->updated = false;
    }

    painter.end();
}


