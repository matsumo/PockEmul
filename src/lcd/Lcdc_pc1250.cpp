#include <QPainter>
#include <QDebug>

#include "pcxxxx.h"

#include "Lcdc_pc1250.h"
#include "Lcdc_symb.h"

///////////////////////////////////////////////////////////////////////
//
//  PC 1250
//
///////////////////////////////////////////////////////////////////////

Clcdc_pc1250::Clcdc_pc1250(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname)
{						//[constructor]
    Color_Off = Qt::transparent;
    baseAdr = 0xF800;
    internalSize = QSize(144,8);
    pixelSize = 4;
    pixelGap = 1;

    symbList << ClcdSymb(250,0, S_SHIFT, 0xF83D, 0x02)	// SHIFT
             << ClcdSymb(48, 0, S_DEF,   0xF83C, 0x01)	// DEF
             << ClcdSymb(72, 0, S_PRO,   0xF83E, 0x01)	// PRO
             << ClcdSymb(100,0, S_RUN,   0xF83E, 0x02)	// RUN
             << ClcdSymb(118,0, S_RESERVE,0xF83E,0x04)	// RESERVE
             << ClcdSymb(158,0, S_DE,    0xF83C, 0x08)	// DE
             << ClcdSymb(166,0, S_G,     0xF83C, 0x04)	// G
             << ClcdSymb(171,0, S_RAD,   0xF83D, 0x04)	// RAD
             << ClcdSymb(290,0, S_E,     0xF83D, 0x08)	// E
             << ClcdSymb(30, 0, S_PRINT, 0xF83C, 0x02)	// P
             << ClcdSymb(0,  0, S_BUSY,  0xF83D, 0x01);	// BUSY
}


void Clcdc_pc1250::disp(void)
{
    BYTE b,data,x;
    int ind;
    WORD adr;

    if (!updated) return;
    //    if (!On) return;

    lock.lock();
    Refresh = false;
    updated = false;

    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    for (ind=0; ind<0x3c; ind++)
    {	adr = baseAdr + ind;
        if (DirtyBuf[adr-baseAdr])
        {
            Refresh = true;
            data = pPC->Get_8(adr);

            x =ind + (ind/5);			// +1 every 5 cols

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x,b,((data>>b)&0x01) ? Color_On : Color_Off);
            }
            DirtyBuf[adr-baseAdr] = 0;
        }
    }

    for (ind=0x3B; ind>=0; ind--)
    {	adr = baseAdr + 0x40 + ind;
        if (DirtyBuf[adr-baseAdr])
        {
            Refresh = true;
            data = pPC->Get_8(adr);

            x = 142 - ind - (ind/5);			// +2 every 5 cols

            for (b=0; b<7;b++) {
                drawPixel(&painter,x,b,((data>>b)&0x01) ? Color_On : Color_Off);
            }
            DirtyBuf[adr-baseAdr] = 0;
        }
    }

    painter.end();

    lock.unlock();
}

///////////////////////////////////////////////////////////////////////
//
//  PC 1245
//
///////////////////////////////////////////////////////////////////////

Clcdc_pc1245::Clcdc_pc1245(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc_pc1250(parent,_lcdRect,_symbRect,_lcdfname,_symbfname)
{						//[constructor]

    internalSize = QSize(96,8);
    symbList.clear();
    symbList << ClcdSymb(  0,0, S_SHIFT, 0xF83D, 0x02)	// SHIFT
             << ClcdSymb( 20,0, S_DEF,   0xF83C, 0x01)	// DEF
             << ClcdSymb( 38,0, S_PRO,   0xF83E, 0x01)	// PRO
             << ClcdSymb( 53,0, S_RUN,   0xF83E, 0x02)	// RUN
             << ClcdSymb(125,0, S_PRINT, 0xF83C, 0x02)	// P
             << ClcdSymb(134,0, S_DE,    0xF83C, 0x08)	// DE
             << ClcdSymb(142,0, S_G,     0xF83C, 0x04)	// G
             << ClcdSymb(147,0, S_RAD,   0xF83D, 0x04)	// RAD
                ;
}

void Clcdc_pc1245::disp(void)
{
    BYTE b,data,x,y;
    int ind;
    WORD adr;

    if (!updated) return;
    //    if (!On) return;

    lock.lock();
    Refresh = false;
    updated = false;

    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    for (ind=0; ind<0x3c; ind++)
    {	adr = 0xF800 + ind;
        if (DirtyBuf[adr-0xF800])
        {
            Refresh = true;
//            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);
            data = pPC->Get_8(adr);

            x =ind + (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<7;b++) {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off);
            }
            DirtyBuf[adr-0xF800] = 0;
        }
    }

    for (ind=0x3B; ind>=0x28; ind--)
    {	adr = 0xF800 + 0x40 + ind;
        if (DirtyBuf[adr-0xF800])
        {
            Refresh = true;
//            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);
            data = pPC->Get_8(adr);

            x = 142 - ind - (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<7;b++) {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off);
            }
            DirtyBuf[adr-0xF800] = 0;
        }
    }

    painter.end();

    lock.unlock();
}

///////////////////////////////////////////////////////////////////////
//
//  PC 1260
//
///////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////
//
//  PC 1260
//
///////////////////////////////////////////////////////////////////////
Clcdc_pc1260::Clcdc_pc1260(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc_pc1250(parent,_lcdRect,_symbRect,_lcdfname,_symbfname)
{
    Color_Off = Qt::transparent;

    internalSize = QSize(156,16);
    pixelSize = 4;
    pixelGap = 1;
    baseAdr = 0x2000;
    symbList.clear();
    symbList << ClcdSymb( 27,31, S_BAR25, 0x203D, 0x01)	// BUSY
             << ClcdSymb( 67,31, S_BAR25, 0x203D, 0x02)	// PRINT
             << ClcdSymb(107,31, S_BAR25, 0x207C, 0x01)	// DEG
             << ClcdSymb(133,31, S_BAR25, 0x207C, 0x02)	// RAD
             << ClcdSymb(160,31, S_BAR25, 0x207C, 0x04)	// GRAD
             << ClcdSymb(200,31, S_BAR25, 0x207C, 0x20)	// ERROR
             << ClcdSymb(0  , 0, S_JAP,   0x203D, 0x08)	// JAP
             << ClcdSymb(0  , 8, S_SMALL, 0x203D, 0x10)	// SMALL
             << ClcdSymb(0  ,16, S_SHIFT, 0x203D, 0x20)	// SHIFT
             << ClcdSymb(0  ,24, S_DEF,   0x203D, 0x40)	// DEF
                ;
}

void Clcdc_pc1260::disp(void)
{
    BYTE b,data;
    int x,y;
    int ind;
    WORD adr;


    if (!updated) return;
    //    if (!On) return;

    lock.lock();
    Refresh = false;
    updated = false;

    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    for (int area = 0; area < 4; area++) {
        int memOffset=0;
        int xOffset = 0;
        int yOffset = 0;

        switch (area) {
        case 0: memOffset = 0x000; xOffset = 0;  yOffset = 0; break;
        case 1: memOffset = 0x040; xOffset = 0;  yOffset = 8; break;
        case 2: memOffset = 0x800; xOffset = 78; yOffset = 0; break;
        case 3: memOffset = 0x840; xOffset = 78; yOffset = 8; break;
        }

        for (ind=0; ind<0x3c; ind++)
        {
            adr = memOffset + baseAdr + ind;
            if (DirtyBuf[adr-baseAdr])
            {
                Refresh = true;
                data = pPC->Get_8(adr);

                x = xOffset + ind /*+ (ind/5)*/;			// +1 every 5 cols
                y = yOffset;

                for (b=0; b<7;b++) {
                    drawPixel(&painter,x + (ind/5)*1.5f ,y+b,((data>>b)&0x01) ? Color_On : Color_Off);
                }
                DirtyBuf[adr-baseAdr] = 0;
            }
        }
    }

    painter.end();

    lock.unlock();
}



