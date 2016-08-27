#include <QPainter>
#include "pcxxxx.h"
#include "Lcdc_pc1401.h"
#include "Lcdc_symb.h"



///////////////////////////////////////////////////////////////////////
//
//  PC 1401
//
///////////////////////////////////////////////////////////////////////
// pc140x
//   16 5x7 with space between char
//   6000 .. 6027, 6067.. 6040
//  603c: 3 STAT
//  603d: 0 BUSY,	1 DEF,	2 SHIFT,3 HYP,	4 PRO,	5 RUN,	6 CAL
//  607c: 0 E,		1 M,	2 (),	3 RAD,	4 G,	5 DE,	6 PRINT


static const struct {
    int x,y;
} pc1401_pos[17]={
    {0  , 0},		// BUSY
    {20 , 0},		// DEF
    {45 , 0},		// SHIFT
    {67 , 0},		// HYP
    {0  , 0},		// SML
    {145, 0},		// DE
    {153, 0},		// G
    {158, 0},		// RAD
    {190, 0},		// OPEN_BRACKET,
    {194, 0},		// CLOSE_BRACKET
    {197, 0},		// M
    {205, 0},		// E
    {10 , 32},		// CAL
    {26 , 32},		// RUN
    {41 , 32},		// PRO
    {162, 32},		// STAT
    {180, 32}		// PRINT

};


#define SYMB1_ADR_1401	0x603C
#define SYMB2_ADR_1401	0x603D
#define SYMB3_ADR_1401	0x607C
#define SYMB1_1401	(pPC->Get_8(SYMB1_ADR_1401))
#define SYMB2_1401	(pPC->Get_8(SYMB2_ADR_1401))
#define SYMB3_1401	(pPC->Get_8(SYMB3_ADR_1401))

void Clcdc_pc1401::disp_symb(void)
{


    if (DirtyBuf[SYMB1_ADR_1401-0x6000] || DirtyBuf[SYMB2_ADR_1401-0x6000] || DirtyBuf[SYMB3_ADR_1401-0x6000])
    {

    disp_one_symb( S_BUSY,	COLOR(SYMB2_1401&0x01),	pc1401_pos[0].x,	pc1401_pos[0].y);
    disp_one_symb( S_DEF,	COLOR(SYMB2_1401&0x02),	pc1401_pos[1].x,	pc1401_pos[1].y);
    disp_one_symb( S_SHIFT,	COLOR(SYMB2_1401&0x04),	pc1401_pos[2].x,	pc1401_pos[2].y);
    disp_one_symb( S_HYP,	COLOR(SYMB2_1401&0x08),	pc1401_pos[3].x,	pc1401_pos[3].y);
    disp_one_symb( S_DE,	COLOR(SYMB3_1401&0x20),	pc1401_pos[5].x,	pc1401_pos[5].y);
    disp_one_symb( S_G,		COLOR(SYMB3_1401&0x10),	pc1401_pos[6].x,	pc1401_pos[6].y);
    disp_one_symb( S_RAD,	COLOR(SYMB3_1401&0x08),	pc1401_pos[7].x,	pc1401_pos[7].y);
    disp_one_symb( S_O_BRA,	COLOR(SYMB3_1401&0x04),	pc1401_pos[8].x,	pc1401_pos[8].y);
    disp_one_symb( S_C_BRA,	COLOR(SYMB3_1401&0x04),	pc1401_pos[9].x,	pc1401_pos[9].y);
    disp_one_symb( S_REV_M,	COLOR(SYMB3_1401&0x02),	pc1401_pos[10].x,	pc1401_pos[10].y);
    disp_one_symb( S_E,		COLOR(SYMB3_1401&0x01),	pc1401_pos[11].x,	pc1401_pos[11].y);

// CAL
    disp_one_symb( S_BAR35,	COLOR(SYMB2_1401&0x40),	pc1401_pos[12].x,	pc1401_pos[12].y);
// RUN
    disp_one_symb( S_BAR35,	COLOR(SYMB2_1401&0x20),	pc1401_pos[13].x,	pc1401_pos[13].y);
// PRO
    disp_one_symb( S_BAR35,	COLOR(SYMB2_1401&0x10),	pc1401_pos[14].x,	pc1401_pos[14].y);
// STAT
    disp_one_symb( S_BAR35,	COLOR(SYMB1_1401&0x08),	pc1401_pos[15].x,	pc1401_pos[15].y);
// PRINT
    disp_one_symb( S_BAR35,	COLOR(SYMB3_1401&0x40),	pc1401_pos[16].x,	pc1401_pos[16].y);

    DirtyBuf[SYMB1_ADR_1401-0x6000] = 0;
    DirtyBuf[SYMB2_ADR_1401-0x6000] = 0;
    DirtyBuf[SYMB3_ADR_1401-0x6000] = 0;

    Refresh = true;
    }

    Clcdc::disp_symb();
}

Clcdc_pc1401::Clcdc_pc1401(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname)
{
    internalSize = QSize(96,7);
    pixelSize = 4;
    pixelGap = 1;
}

void Clcdc_pc1401::disp(void)
{

    BYTE b,data,x,y;
    int ind;
    WORD adr;

    lock.lock();

    Refresh = false;

    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    for (ind=0; ind<0x28; ind++)
    {
        adr = 0x6000 + ind;
        if ( DirtyBuf[adr-0x6000] )
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0 );

            x =ind + (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<7;b++) {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
            }
            DirtyBuf[adr-0x6000] = 0;
        }
    }
    for (ind=0x27; ind>=0; ind--)
    {
        adr = 0x6000 + 0x40 + ind;
        if ( DirtyBuf[adr-0x6000])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0 );

            x = 94 - ind - (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<7;b++) {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
            }
            DirtyBuf[adr-0x6000] = 0;
        }
    }

    painter.end();

    lock.unlock();

}


