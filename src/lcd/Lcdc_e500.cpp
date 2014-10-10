#include <QPainter>
#include "common.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "sharp/e500.h"
#include "Lcdc_e500.h"
#include "Lcdc_symb.h"

Clcdc_e500::Clcdc_e500(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]

    internalSize = QSize(240,32);
    pixelSize = 4;
    pixelGap = 1;
}

static const struct {
    int x,y;
} e500_pos[15]={
    {7, 10},     // BUSY
    {60, 10},    // RUN
    {79, 10},    // PRO
    {35, 10},    // DBL
    {257 ,10},    // SMALL
    {238 ,10},    // KANA
    {202 ,0},    // HYP
    {200,10},    // CAPS
    {173,10},    // 2ndF
    {241,0},    // PRINT
    {300,10},    // E
    {181,0},    // RAD
    {176,0},    // G
    {168,0},    // DEG
    {314,9}    // BATT
};
/* busy  shift   small   de g rad   run  pro  reserve  def  i ii iii battery */
/* japanese? */


//      b7      b6      b5      b4      b3      b2      b1      b0
// S1   KBII                            S       x       CTRL    BATT    F3C7
// S2           RUN     PRO     RESERVE         RAD     G       DE      F64F
// S3   DEF     I       II      III     SML     x       SHIFT   BUSY    F64E

#define SYMB1_e500		(((Ce500 *)pPC)->pHD61102_1->info.imem[symbSL(0)])
#define SYMB2_e500		(((Ce500 *)pPC)->pHD61102_1->info.imem[symbSL(4)])
#define SYMB3_e500		(((Ce500 *)pPC)->pHD61102_1->info.imem[symbSL(5)])

void Clcdc_e500::disp_symb(void)
{
    disp_one_symb( S_BUSY,		COLOR(SYMB1_e500&1),	e500_pos[0].x,	e500_pos[0].y);
    disp_one_symb( S_RUN,         COLOR(SYMB1_e500&2),	e500_pos[1].x,	e500_pos[1].y);
    disp_one_symb( S_PRO,			COLOR(SYMB1_e500&4),	e500_pos[2].x,	e500_pos[2].y);
    disp_one_symb( S_DBL,         COLOR(SYMB1_e500&8),	e500_pos[3].x,	e500_pos[3].y);
    disp_one_symb( S_SMALL,		COLOR(SYMB2_e500&0x01),	e500_pos[4].x,	e500_pos[4].y);
    disp_one_symb( S_JAP, 		COLOR(SYMB2_e500&0x02),	e500_pos[5].x,	e500_pos[5].y);
    disp_one_symb( S_HYP,			COLOR(SYMB2_e500&0x04),	e500_pos[6].x,	e500_pos[6].y);
    disp_one_symb( S_CAPS,		COLOR(SYMB2_e500&0x08),	e500_pos[7].x,	e500_pos[7].y);
    disp_one_symb( S_SECF,		COLOR(SYMB2_e500&0x10),	e500_pos[8].x,	e500_pos[8].y);
    disp_one_symb( S_PRINTL,		COLOR(SYMB3_e500&0x01),	e500_pos[9].x,	e500_pos[9].y);
    disp_one_symb( S_E,			COLOR(SYMB3_e500&0x02),	e500_pos[10].x,	e500_pos[10].y);
    disp_one_symb( S_RAD,         COLOR(SYMB3_e500&0x04),	e500_pos[11].x,	e500_pos[11].y);
    disp_one_symb( S_G,       	COLOR(SYMB3_e500&0x08),	e500_pos[12].x,	e500_pos[12].y);
    disp_one_symb( S_DEG,         COLOR(SYMB3_e500&0x10),	e500_pos[13].x,	e500_pos[13].y);
    disp_one_symb( S_REV_BATT,    COLOR(SYMB3_e500&0x80),	e500_pos[14].x,	e500_pos[14].y);

//    Clcdc::disp_symb();
}
INLINE int Clcdc_e500::symbSL(int x)
{
//    Ce500 *p = (Ce500 *)pPC;
    int l=(x+((Ce500 *)pPC)->pHD61102_2->info.displaySL/8);
    l = (l>=8) ? l-8 : l;
    return l*0x40+63;
}

INLINE int Clcdc_e500::computeSL(CHD61102* pCtrl,int ord)
{
    int y = ord;
    y -= pCtrl->info.displaySL;
    if (y < 0) y += 64;
    return y;
}

void Clcdc_e500::disp(void)
{

    BYTE b;

    Refresh = false;

    if (!ready) return;
    if (!((Ce500 *)pPC)->pHD61102_1 || !((Ce500 *)pPC)->pHD61102_2) return;
    if (!(((Ce500 *)pPC)->pHD61102_1->updated || ((Ce500 *)pPC)->pHD61102_2->updated)) return;

    ((Ce500 *)pPC)->pHD61102_1->updated = false;
    ((Ce500 *)pPC)->pHD61102_2->updated = false;

    Refresh = true;

    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    if (((Ce500 *)pPC)->pHD61102_2->info.on_off) {
        for (int i = 0 ; i < 64; i++)
        {
            for (int j = 0 ; j < 8 ; j++)
            {
                BYTE data = ((Ce500 *)pPC)->pHD61102_2->info.imem[ (j * 0x40) + i ];
                for (b=0; b<8;b++)
                {
                    //if (((data>>b)&0x01) && (pPC->pCPU->fp_log)) fprintf(pPC->pCPU->fp_log,"PSET [%i,%i]\n",i,j*8+b);
//                    painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );

                    int y = computeSL(((Ce500 *)pPC)->pHD61102_2,j*8+b);
                    if ((y>=0)&&(y < 32))
                        drawPixel(&painter,i, y,((data>>b)&0x01) ? Color_On : Color_Off );
//                    painter.drawPoint( i, y );
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

    if (((Ce500 *)pPC)->pHD61102_1->info.on_off) {
        for (int i = 0 ; i < 56; i++)
        {
            for (int j = 0 ; j < 8 ; j++)
            {
                BYTE data = ((Ce500 *)pPC)->pHD61102_1->info.imem[ (j * 0x40) + i ];
                for (b=0; b<8;b++)
                {
                    //if (((data>>b)&0x01) && (pPC->pCPU->fp_log)) fprintf(pPC->pCPU->fp_log,"PSET [%i,%i]\n",64+i,j*8+b);
//                    painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
    //                painter.drawPoint( 64+i, computeSL(j*8+b));
                    int y = computeSL(((Ce500 *)pPC)->pHD61102_1,j*8+b);
                     if ((y>=0)&&(y < 32))
                         drawPixel(&painter,64+i, y,((data>>b)&0x01) ? Color_On : Color_Off );
//                     painter.drawPoint( 64+i, y );
                }
            }
        }
    }
    else {
        // Turn off screen
        for (int i=0;i<56;i++)
            for (int j=0;j<32;j++)
                drawPixel(&painter,64+i,j,Color_Off);
    }

    if (((Ce500 *)pPC)->pHD61102_1->info.on_off) {

        for (int i = 0 ; i < 56; i++)
        {
            for (int j = 0 ; j < 8 ; j++)
            {
                BYTE data = ((Ce500 *)pPC)->pHD61102_1->info.imem[ (j * 0x40) + i ];
                for (b=0; b<8;b++)
                {
    //                if (((data>>b)&0x01) && (pPC->pCPU->fp_log)) fprintf(pPC->pCPU->fp_log,"PSET [%i,%i]\n",128+i,(j-4)*8+b);
//                    painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
                    int y = computeSL(((Ce500 *)pPC)->pHD61102_1,j*8+b-32);
                    if ((y>=0)&&(y < 32))
                        drawPixel(&painter,175-i, y,((data>>b)&0x01) ? Color_On : Color_Off );
//                        painter.drawPoint( 175-i, y );

                }
            }
        }
    }
    else {
        // Turn off screen
        for (int i=0;i<56;i++)
            for (int j=0;j<32;j++)
                drawPixel(&painter,175-i,j,Color_Off);
    }



    if (((Ce500 *)pPC)->pHD61102_2->info.on_off) {

        for (int i = 0 ; i < 64; i++)
        {
            for (int j = 0 ; j < 8 ; j++)
            {
                BYTE data = ((Ce500 *)pPC)->pHD61102_2->info.imem[ (j * 0x40) + i ];
                for (b=0; b<8;b++)
                {
    //                if (((data>>b)&0x01) && (pPC->pCPU->fp_log)) fprintf(pPC->pCPU->fp_log,"PSET [%i,%i]\n",128+i,(j-4)*8+b);
//                    painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
                    int y = computeSL(((Ce500 *)pPC)->pHD61102_2,j*8+b-32);
                    if ((y>=0)&&(y < 32))
                        drawPixel(&painter,239-i, y,((data>>b)&0x01) ? Color_On : Color_Off );
//                    painter.drawPoint( 239-i, y );

                }
            }
        }
    }
    else {
        // Turn off screen
        for (int i=0;i<64;i++)
            for (int j=0;j<32;j++)
                drawPixel(&painter,239-i,j,Color_Off);
    }


    redraw = 0;
    painter.end();
}

