#include <QPainter>
#include "common.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "sharp/g850v.h"
#include "sed1560.h"
#include "Lcdc_g850.h"
#include "Lcdc_symb.h"
#include "Log.h"

Clcdc_g850::Clcdc_g850(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]

    internalSize = QSize(144,48);
    pixelSize = 4;
    pixelGap = 1;
}

#if 0
static SDL_Color *Clcdc_g850::getLcdColors(SDL_Color *colors, int voltage)
{
    SDL_Color back = { 0 }, on = { 0 }, off = { 0 };
    int c1, c2;


    back.r = (colorBack >> 16) & 0xff;
    back.g = (colorBack >>  8) & 0xff;
    back.b = (colorBack >>  0) & 0xff;
    if(!lcdEffectReverse) {
        off.r  = (colorOff  >> 16) & 0xff;
        off.g  = (colorOff  >>  8) & 0xff;
        off.b  = (colorOff  >>  0) & 0xff;
        on.r   = (colorOn   >> 16) & 0xff;
        on.g   = (colorOn   >>  8) & 0xff;
        on.b   = (colorOn   >>  0) & 0xff;
    } else {
        off.r  = (colorOn  >> 16) & 0xff;
        off.g  = (colorOn  >>  8) & 0xff;
        off.b  = (colorOn  >>  0) & 0xff;
        on.r   = (colorOff >> 16) & 0xff;
        on.g   = (colorOff >>  8) & 0xff;
        on.b   = (colorOff >>  0) & 0xff;
    }
    if(voltage <= 0x0f) {
        c1 = 0x0f - voltage;
        c2 = voltage;
        colors[0]   = back;
        colors[1].r = (back.r * c1 + off.r * c2) / 0x0f;
        colors[1].g = (back.g * c1 + off.g * c2) / 0x0f;
        colors[1].b = (back.b * c1 + off.b * c2) / 0x0f;
        colors[2].r = (back.r * c1 + on.r  * c2) / 0x0f;
        colors[2].g = (back.g * c1 + on.g  * c2) / 0x0f;
        colors[2].b = (back.b * c1 + on.b  * c2) / 0x0f;
    } else {
        c1 = 0x20 - voltage;
        c2 = voltage - 0x0f;
        colors[0].r = (back.r * c1 + off.r * c2) / 0x11;
        colors[0].g = (back.g * c1 + off.g * c2) / 0x11;
        colors[0].b = (back.b * c1 + off.b * c2) / 0x11;
        colors[1].r = (off.r  * c1 + on.r  * c2) / 0x11;
        colors[1].g = (off.g  * c1 + on.g  * c2) / 0x11;
        colors[1].b = (off.b  * c1 + on.b  * c2) / 0x11;
        colors[2]   = on;
    }

    return colors;
}

#endif

static const struct {
    int x,y;
    const char *symb;
    int	Pg;
    int	bit;
} g850_pos[17]={
    {0, 80,   S_BUSY,	  5, 0x40},
    {0, 89,   S_REV_BATT, 5, 0x80},
    {306, 0,  S_RUN,      0, 0x02},
    {322, 0,  S_PRO,	  0, 0x08},
    {306 ,10, S_TEXT,	  0, 0x40},
    {306 ,20, S_CASL,     1, 0x08},
    {306 ,30, S_STAT,     2, 0x01},
    {306,40,  S_SECF,	  2, 0x20},
    {326,40,  S_REV_M,	  2, 0x80},
    {306,50,  S_CAPS,	  3, 0x04},
    {306,60,  S_JAP,      3, 0x80},
    {322,60,  S_JAP2,     4, 0x02},
    {306,70,  S_DE,		  4, 0x10},
    {314,70,  S_G,        4, 0x40},
    {319,70,  S_RAD,      5, 0x01},
    {306,80,  S_CONST,    5, 0x04},
    {306,89,  S_PRINTL,	  5, 0x10}
};
//#define SYMB_g850(x)		(((Cg850v *)pPC)->pSED1560->get8(x*0xA6 + 0x90))
#define SYMB_g850(x)		(((Cg850v *)pPC)->pSED1560->get8(symbSL(x)))
void Clcdc_g850::disp_symb(void)
{
    for (int i=0;i<17;i++) {
        disp_one_symb( g850_pos[i].symb, COLOR(SYMB_g850(g850_pos[i].Pg)&g850_pos[i].bit),	g850_pos[i].x,	g850_pos[i].y);
    }

    Clcdc::disp_symb();
}
INLINE int Clcdc_g850::symbSL(int x)
{

    int l=(x+((Cg850v *)pPC)->pSED1560->info.displaySL/8);
    l = (l>=8) ? l-8 : l;
    return l*0xA6+0x90;
}

INLINE int Clcdc_g850::computeSL(int ord)
{
    int y = ord;
    y -= ((Cg850v*)pPC)->pSED1560->info.displaySL;
    if (y < 0) y += 64;
    return y;
}


void Clcdc_g850::disp(void)
{

    Cg850v *g850 = (Cg850v*)pPC;

    BYTE b;

    Refresh = false;


    if (!ready) return;
    if (!g850->pSED1560) return;
    if (!g850->pSED1560->updated) return;
//AddLog(LOG_DISPLAY,"DISP");
    g850->pSED1560->updated = false;

    Refresh = true;

    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    if (g850->pSED1560->info.on_off)
    {
        for (int i = 0 ; i < 0x90; i++)
        {
            for (int j = 0 ; j < 8 ; j++)
            {
            BYTE data = g850->pSED1560->get8((j * 0xa6) + i); //info.imem[ (j * 0xa6) + i ];
                for (b=0; b<8;b++)
                {
                    int y = computeSL(j*8+b);
                    if ((y>=0)&&(y < 48)) {
                        drawPixel(&painter,i,y,((data>>b)&0x01) ? Color_On : Color_Off );
                    }
                }
            }
        }
    }


    redraw = 0;
    painter.end();
}


