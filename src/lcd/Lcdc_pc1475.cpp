#include <QPainter>
#include "pcxxxx.h"
#include "Lcdc_pc1475.h"
#include "Lcdc_symb.h"

// TODO: seems to be very close of the PC-1262. Try to mix them.

/*******************/
/***	1475	****/
/*******************/

static const struct {
    int x,y;
} pc1475_pos[23]={
    {245, 0},		// REV_BATT
    {210 , 0},		// O_BRA
    {215 , 0},		// C_BRA
    {195 , 0},		// HYP
    {50  , 43},		// RSV
    {102, 43},		// PRO
    {82, 43},		// RUN
    {25, 43},		// CAL

    {0, 0},			// BUSY,
    {44, 0},		// DEF,
    {61, 0},		// SHIFT
    {24, 0},		// DBL,

    {238 , 0},		// E,
    {228 , 0},		// REV_M
    {160 , 0},		// RAD,
    {155, 0},		// G,
    {205, 43},		// MATRIX
    {220, 43},		// STAT
    {240, 43},		// PRINT

    {90, 0},		// JAP,
    {105, 0},		// JAP2,
    {125, 0},		// SML,
    {147, 0}		// DE,
};


#define SYMB1_ADR_1475	0x283C
#define SYMB2_ADR_1475	0x283D
#define SYMB3_ADR_1475	0x287C
#define SYMB4_ADR_1475	0x287D

#define SYMB1_1475	(pPC->Get_8(SYMB1_ADR_1475))
#define SYMB2_1475	(pPC->Get_8(SYMB2_ADR_1475))
#define SYMB3_1475	(pPC->Get_8(SYMB3_ADR_1475))
#define SYMB4_1475	(pPC->Get_8(SYMB4_ADR_1475))

void Clcdc_pc1475::disp_symb(void)
{

    if (DirtyBuf[SYMB1_ADR_1475-0x2800] ||
        DirtyBuf[SYMB2_ADR_1475-0x2800] ||
        DirtyBuf[SYMB3_ADR_1475-0x2800] ||
        DirtyBuf[SYMB4_ADR_1475-0x2800])
    {

    disp_one_symb( S_REV_BATT,COLOR(SYMB1_1475&0x01),	pc1475_pos[0].x,	pc1475_pos[0].y);
    disp_one_symb( S_O_BRA,	COLOR(SYMB1_1475&0x02),	pc1475_pos[1].x,	pc1475_pos[1].y);
    disp_one_symb( S_C_BRA,	COLOR(SYMB1_1475&0x02),	pc1475_pos[2].x,	pc1475_pos[2].y);
    disp_one_symb( S_HYP,		COLOR(SYMB1_1475&0x04),	pc1475_pos[3].x,	pc1475_pos[3].y);
    disp_one_symb( S_BAR25,	COLOR(SYMB1_1475&0x08),	pc1475_pos[4].x,	pc1475_pos[4].y);	// RSV
    disp_one_symb( S_BAR25,	COLOR(SYMB1_1475&0x10),	pc1475_pos[5].x,	pc1475_pos[5].y);	// PRO
    disp_one_symb( S_BAR25,	COLOR(SYMB1_1475&0x20),	pc1475_pos[6].x,	pc1475_pos[6].y);	// RUN
    disp_one_symb( S_BAR25,	COLOR(SYMB1_1475&0x40),	pc1475_pos[7].x,	pc1475_pos[7].y);	// CAL

    disp_one_symb( S_BUSY,	COLOR(SYMB2_1475&0x01),	pc1475_pos[8].x,	pc1475_pos[8].y);
    disp_one_symb( S_DEF,		COLOR(SYMB2_1475&0x02),	pc1475_pos[9].x,	pc1475_pos[9].y);
    disp_one_symb( S_SHIFT,	COLOR(SYMB2_1475&0x04),	pc1475_pos[10].x,	pc1475_pos[10].y);
    disp_one_symb( S_DBL,		COLOR(SYMB2_1475&0x08),	pc1475_pos[11].x,	pc1475_pos[11].y);

    disp_one_symb( S_E,		COLOR(SYMB3_1475&0x01),	pc1475_pos[12].x,	pc1475_pos[12].y);
    disp_one_symb( S_REV_M,	COLOR(SYMB3_1475&0x02),	pc1475_pos[13].x,	pc1475_pos[13].y);
    disp_one_symb( S_RAD,		COLOR(SYMB3_1475&0x04),	pc1475_pos[14].x,	pc1475_pos[14].y);
    disp_one_symb( S_G,		COLOR(SYMB3_1475&0x08),	pc1475_pos[15].x,	pc1475_pos[15].y);
    disp_one_symb( S_BAR25,	COLOR(SYMB3_1475&0x10),	pc1475_pos[16].x,	pc1475_pos[16].y);	// MATRIX
    disp_one_symb( S_BAR25,	COLOR(SYMB3_1475&0x20),	pc1475_pos[17].x,	pc1475_pos[17].y);	// STAT
    disp_one_symb( S_BAR25,	COLOR(SYMB3_1475&0x40),	pc1475_pos[18].x,	pc1475_pos[18].y);	// PRINT

    disp_one_symb( S_JAP,		COLOR(SYMB4_1475&0x01),	pc1475_pos[19].x,	pc1475_pos[19].y);
    disp_one_symb( S_JAP2,	COLOR(SYMB4_1475&0x02),	pc1475_pos[20].x,	pc1475_pos[20].y);
    disp_one_symb( S_SML,		COLOR(SYMB4_1475&0x04),	pc1475_pos[21].x,	pc1475_pos[21].y);
    disp_one_symb( S_DE,		COLOR(SYMB4_1475&0x08),	pc1475_pos[22].x,	pc1475_pos[22].y);

    DirtyBuf[SYMB1_ADR_1475-0x2800] = 0;
    DirtyBuf[SYMB2_ADR_1475-0x2800] = 0;
    DirtyBuf[SYMB3_ADR_1475-0x2800] = 0;
    DirtyBuf[SYMB4_ADR_1475-0x2800] = 0;

    Refresh = true;
    }

//    Clcdc::disp_symb();
}

Clcdc_pc1475::Clcdc_pc1475(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]


    internalSize = QSize(132,15);
    pixelSize = 4;
    pixelGap = 1;
}

bool	Clcdc_pc1475::init(void)
{

    Clcdc::init();

    return true;
}

void Clcdc_pc1475::disp(void)
{
    BYTE b,data;
    int x,y;
    int ind;
    WORD adr;

    if (!redraw) {
        if (!updated) return;
    }
    redraw = false;
    updated = false;

    Refresh = false;
    disp_symb();
    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

#if 1
    for (ind=0; ind<0x3c; ind++)
    {	adr = 0x2800 + ind;
        if (DirtyBuf[adr-0x2800])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);

            x =(ind*2) + (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x/2.0f,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, y+2*b);
//                painter.drawPoint( x, y+2*b+1);
//                painter.drawPoint( x+1, y+2*b);
//                painter.drawPoint( x+1, y+2*b+1);
            }
            DirtyBuf[adr-0x2000] = 0;
        }
    }
#endif
#if 1
    for (ind=0; ind<0x3c; ind++)
    {	adr = 0x2800 + 0x40 + ind;
        if (DirtyBuf[adr-0x2800])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);

            x =(ind*2) + (ind/5);			// +1 every 5 cols
            y = 8;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x/2.0f,y+b,((data>>b)&0x01) ? Color_On : Color_Off );

//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, y+2*b);
//                painter.drawPoint( x, y+2*b+1);
//                painter.drawPoint( x+1, y+2*b);
//                painter.drawPoint( x+1, y+2*b+1);
            }
            DirtyBuf[adr-0x2000] = 0;
        }
    }
#endif
#if 1
    for (ind=0; ind<0x3c; ind++)
    {	adr = 0x2A00 + ind;
        if (DirtyBuf[adr-0x2800])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);

            x = 132 + (ind*2) + (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x/2.0f,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, y+2*b);
//                painter.drawPoint( x, y+2*b+1);
//                painter.drawPoint( x+1, y+2*b);
//                painter.drawPoint( x+1, y+2*b+1);
            }
            DirtyBuf[adr-0x2000] = 0;
        }
    }
#endif
#if 1
    for (ind=0; ind<0x3c; ind++)
    {	adr = 0x2A00 + 0x40 + ind;
        if (DirtyBuf[adr-0x2800])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);

            x = 132 + (ind*2) + (ind/5);			// +1 every 5 cols
            y = 8;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x/2.0f,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, y+2*b);
//                painter.drawPoint( x, y+2*b+1);
//                painter.drawPoint( x+1, y+2*b);
//                painter.drawPoint( x+1, y+2*b+1);
            }
            DirtyBuf[adr-0x2000] = 0;
        }
    }
#endif
    updated = 0;
    painter.end();
}


