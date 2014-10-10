#include <QPainter>
#include "common.h"
#include "pcxxxx.h"
#include "sharp/pc1600.h"
#include "Lcdc_pc1600.h"
#include "Lcdc_symb.h"

static const struct {
    int x,y;
} pc1600_pos[16]={
    {0, 0},     // BUSY
    {18, 0},    // SHIFT
    {40, 0},    // JAP
    {50, 0},    // SMALL
    {75 ,0},    // DE
    {83 ,0},    // G
    {88 ,0},    // RAD
    {160,0},    // RUN
    {175,0},    // PRO
    {190,0},    // RESERVE
    {220,0},    // DEF
    {240,0},    // ROMEAN_I
    {245,0},    // ROMEAN_II
    {255,0},    // ROMEAN_III
    {265,0},    // CTRL
    {290,0}     // REV_BAT
};
/* busy  shift   small   de g rad   run  pro  reserve  def  i ii iii battery */
/* japanese? */


//      b7      b6      b5      b4      b3      b2      b1      b0
// S1   KBII                            S       x       CTRL    BATT    F3C7
// S2           RUN     PRO     RESERVE         RAD     G       DE      F64F
// S3   DEF     I       II      III     SML     x       SHIFT   BUSY    F64E

#define SYMB1_1600		((Cpc1600 *)pPC)->pHD61102_2->info.imem[symbSL(4)]
#define SYMB2_1600		((Cpc1600 *)pPC)->pHD61102_2->info.imem[symbSL(6)]
#define SYMB3_1600		((Cpc1600 *)pPC)->pHD61102_2->info.imem[symbSL(7)]

void Clcdc_pc1600::disp_symb(void)
{
    disp_one_symb( S_BUSY,		COLOR(SYMB3_1600&1),	pc1600_pos[0].x,	pc1600_pos[0].y);
    disp_one_symb( S_SHIFT,		COLOR(SYMB3_1600&2),	pc1600_pos[1].x,	pc1600_pos[1].y);
    disp_one_symb( S_JAP,			COLOR(SYMB3_1600&4),	pc1600_pos[2].x,	pc1600_pos[2].y);
    disp_one_symb( S_SMALL,		COLOR(SYMB3_1600&8),	pc1600_pos[3].x,	pc1600_pos[3].y);
    disp_one_symb( S_DE,			COLOR(SYMB2_1600&0x01),	pc1600_pos[4].x,	pc1600_pos[4].y);
    disp_one_symb( S_G,			COLOR(SYMB2_1600&0x02),	pc1600_pos[5].x,	pc1600_pos[5].y);
    disp_one_symb( S_RAD,			COLOR(SYMB2_1600&0x04),	pc1600_pos[6].x,	pc1600_pos[6].y);
    disp_one_symb( S_RUN,			COLOR(SYMB2_1600&0x40),	pc1600_pos[7].x,	pc1600_pos[7].y);
    disp_one_symb( S_PRO,			COLOR(SYMB2_1600&0x20),	pc1600_pos[8].x,	pc1600_pos[8].y);
    disp_one_symb( S_RESERVE,		COLOR(SYMB2_1600&0x10),	pc1600_pos[9].x,	pc1600_pos[9].y);
    disp_one_symb( S_DEF,			COLOR(SYMB3_1600&0x80),	pc1600_pos[10].x,	pc1600_pos[10].y);
    disp_one_symb( S_ROMEAN_I,	COLOR(SYMB3_1600&0x40),	pc1600_pos[11].x,	pc1600_pos[11].y);
    disp_one_symb( S_ROMEAN_II,	COLOR(SYMB3_1600&0x20),	pc1600_pos[12].x,	pc1600_pos[12].y);
    disp_one_symb( S_ROMEAN_III,	COLOR(SYMB3_1600&0x10),	pc1600_pos[13].x,	pc1600_pos[13].y);
    disp_one_symb( S_CTRL,        COLOR(SYMB1_1600&0x02),	pc1600_pos[14].x,	pc1600_pos[14].y);
    disp_one_symb( S_REV_BATT,    COLOR(0),               pc1600_pos[15].x,	pc1600_pos[15].y);



    Clcdc::disp_symb();
}
INLINE int Clcdc_pc1600::symbSL(int x)
{
    int l=(x+((Cpc1600 *)pPC)->pHD61102_2->info.displaySL/8);
    l = (l>=8) ? l-8 : l;
    return l*0x40+63;
}

Clcdc_pc1600::Clcdc_pc1600(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname)
{						//[constructor]
    internalSize = QSize(156,32);
    pixelSize = 4;
    pixelGap = 1;
}

INLINE int Clcdc_pc1600::computeSL(CHD61102* pCtrl,int ord)
{
    int y = ord;
    y -= pCtrl->info.displaySL;
    if (y < 0) y += 64;
    return y;
}

void Clcdc_pc1600::disp(void)
{

    BYTE b;

    Refresh = false;

    if (!ready) return;
    if (!((Cpc1600 *)pPC)->pHD61102_1 || !((Cpc1600 *)pPC)->pHD61102_2) return;
    if (!(((Cpc1600 *)pPC)->pHD61102_1->updated || ((Cpc1600 *)pPC)->pHD61102_2->updated)) return;

    ((Cpc1600 *)pPC)->pHD61102_1->updated = false;
    ((Cpc1600 *)pPC)->pHD61102_2->updated = false;
    Refresh = true;

    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    if (((Cpc1600 *)pPC)->pHD61102_1->info.on_off) {
        for (int i = 0 ; i < 64; i++)
        {
            for (int j = 0 ; j < 8 ; j++)
            {
                BYTE data = ((Cpc1600 *)pPC)->pHD61102_1->info.imem[ (j * 0x40) + i ];
                for (b=0; b<8;b++)
                {
                    int y = computeSL(((Cpc1600 *)pPC)->pHD61102_1,j*8+b);
                    if ((y>=0)&&(y < 32))
                        drawPixel(&painter,i,y,((data>>b)&0x01) ? Color_On : Color_Off );
                }
            }
        }
    }
    else {
        // Turn off screen
        for (int i=0;i<64;i++)
            for (int j=0;j<32;j++)
                drawPixel(&painter,i,j,Color_Off);
    }


    if (((Cpc1600 *)pPC)->pHD61102_2->info.on_off) {
        for (int i = 0 ; i < 64; i++)
        {
            for (int j = 0 ; j < 8 ; j++)
            {
                BYTE data = ((Cpc1600 *)pPC)->pHD61102_2->info.imem[ (j * 0x40) + i ];
                for (b=0; b<8;b++)
                {
                    int y = computeSL(((Cpc1600 *)pPC)->pHD61102_2,j*8+b);
                     if ((y>=0)&&(y < 32))
                         drawPixel(&painter,64+i,y,((data>>b)&0x01) ? Color_On : Color_Off );
                }
            }
        }
    }
    else {
        // Turn off screen
        for (int i=0;i<64;i++)
            for (int j=0;j<32;j++)
                drawPixel(&painter,64+i,j,Color_Off);
    }


    if (((Cpc1600 *)pPC)->pHD61102_1->info.on_off) {

        for (int i = 0 ; i < 28; i++)
        {
            for (int j = 0 ; j < 8 ; j++)
            {
                BYTE data = ((Cpc1600 *)pPC)->pHD61102_1->info.imem[ (j * 0x40) + i ];
                for (b=0; b<8;b++)
                {
                    int y = computeSL(((Cpc1600 *)pPC)->pHD61102_1,j*8+b-32);
                    if ((y>=0)&&(y < 32))
                        drawPixel(&painter,128+i,y,((data>>b)&0x01) ? Color_On : Color_Off );
                }
            }
        }
    }
    else {
        // Turn off screen
        for (int i=0;i<64;i++)
            for (int j=0;j<32;j++)
                drawPixel(&painter,128+i,j,Color_Off);
    }


    redraw = 0;
    painter.end();
}
