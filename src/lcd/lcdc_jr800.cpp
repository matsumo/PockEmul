#include <QPainter>
#include <QDebug>

#include "common.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "jr800.h"
#include "hd44102.h"


#include "lcdc_jr800.h"
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
        qWarning()<<"Draw driver:"<<i;
        Refresh = true;

        for (int x = 0; x< 50;x++)
        {
            int z = jr800->hd44102[i]->info.m_page;
            for (int y=0; y<4;y++)
            {
                int sy = (z + y)%4;
                int sx = 49-x;
                UINT8 data = jr800->hd44102[i]->info.imem[sy][x];

                for (b=0;b<8;b++)
                {
                    QColor _col = ((data>>b)&0x01) ? Color_On : Color_Off ;
                    drawPixel(&painter,sx + (i%4)*50, (i/4)*32 + sy*8 + b,_col);
                }
            }
        }
        _toRefresh = true;
        jr800->hd44102[i]->updated = false;
    }

//    Refresh = _toRefresh;

    painter.end();
}


//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------
#if 0
UINT32 CHD44102::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
    for (int y = 0; y < 50; y++)
    {
        int z = m_page << 3;

        for (int x = 0; x < 32; x++)
        {
            UINT8 data = m_ram[z / 8][y];

            int sy = m_sy + z;
            int sx = m_sx + y;

            if (cliprect.contains(sx, sy))
            {
                int color = (m_status & STATUS_DISPLAY_OFF) ? 0 : BIT(data, z % 8);

                bitmap.pix16(sy, sx) = color;
            }

            z++;
            z %= 32;
        }
    }
    return 0;
}
#endif

