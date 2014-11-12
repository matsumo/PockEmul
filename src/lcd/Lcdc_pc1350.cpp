#include <QPainter>

#include "Lcdc_pc1350.h"
#include "Lcdc_symb.h"
#include "pcxxxx.h"

///////////////////////////////////////////////////////////////////////
//
//  PC 1350
//
///////////////////////////////////////////////////////////////////////

Clcdc_pc1350::Clcdc_pc1350(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname)
{						//[constructor]
    Color_Off = Qt::transparent;
    baseAdr = 0x7000;
    pixelSize = 4;
    pixelGap = 1;
    internalSize = QSize(150,32);

    symbList << ClcdSymb(1, 3,  S_PRINT	,0x783C	,0x04)
             << ClcdSymb(8, 3,  S_RUN	,0x783C	,0x10)
             << ClcdSymb(4, 11, S_PRO	,0x783C	,0x20)
             << ClcdSymb(6, 21, S_JAP	,0x783C	,0x40)
             << ClcdSymb(4, 29, S_SML	,0x783C	,0x80)
             << ClcdSymb(1, 39, S_SHIFT	,0x783C	,0x01)
             << ClcdSymb(4, 47, S_DEF	,0x783C	,0x02);
}

void Clcdc_pc1350::disp(void)
{
    BYTE co,li,ind,b,data,x,y;
    WORD adr;

    if (!updated) return;
    //    if (!On) return;

    lock.lock();
    Refresh = false;
    updated = false;

    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    for (co=0; co<5; co++)
    {	for (li=0; li<4; li++)
        {	for (ind=0; ind<30; ind++)
            {
                adr = baseAdr + ind + (0x0200 * co);
                switch (li)
                {
                    case 0 : break;
                    case 1 : adr += 0x40; break;
                    case 2 : adr += 0x1E; break;
                    case 3 : adr += 0x5E; break;
                }
                if (DirtyBuf[adr-baseAdr])
                {
                    Refresh = true;
                    x = ind + (co * 30);
                    y = 8 * li;

//                    data = ( On ? (BYTE) pPC->Get_8(adr) : 0 );
                    data = (BYTE) pPC->Get_8(adr);

                    for (b=0; b<8;b++)
                    {
                        drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off);
                    }
                    DirtyBuf[adr-baseAdr]=0;
                }
            }
        }
    }

    painter.end();

    lock.unlock();
}
///////////////////////////////////////////////////////////////////////
//
//  PC 1360
//
///////////////////////////////////////////////////////////////////////
Clcdc_pc1360::Clcdc_pc1360(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc_pc1350(parent,_lcdRect,_symbRect,_lcdfname,_symbfname)
{						//[constructor]
    for (int i=0; i< symbList.count();i++)
        symbList[i].addr = 0x303C;
    baseAdr = 0x2800;
}


///////////////////////////////////////////////////////////////////////
//
//  PC 2500
//
///////////////////////////////////////////////////////////////////////

Clcdc_pc2500::Clcdc_pc2500(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc_pc1350(parent,_lcdRect,_symbRect,_lcdfname,_symbfname)
{						//[constructor]
    symbList.clear();

    symbList << ClcdSymb(0, 0,  S_BUSY	,0x787C	,0x80)
             << ClcdSymb(223, 0,  S_RUN	,0x783C	,0x10)
             << ClcdSymb(260, 0, S_PRO	,0x783C	,0x20)
             << ClcdSymb(120, 0, S_JAP	,0x783C	,0x40)
             << ClcdSymb(60, 0, S_CAPS	,0x783C	,0x80)
             << ClcdSymb(185, 0, S_DEF	,0x783C	,0x02);
}

