#include <QPainter>
#include "pcxxxx.h"
#include "Lcdc_pc1403.h"
#include "Lcdc_symb.h"


///////////////////////////////////////////////////////////////////////
//
//  PC 1403
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
} pc1403_pos[17]={
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
    {20 , 32},		// CAL
    {36 , 32},		// RUN
    {51 , 32},		// PRO
    {162, 32},		// STAT
    {180, 32}		// PRINT

};


#define SYMB1_ADR_1403	0x303C
#define SYMB2_ADR_1403	0x303D
#define SYMB3_ADR_1403	0x307C
#define SYMB1_1403	(pPC->Get_8(SYMB1_ADR_1403))
#define SYMB2_1403	(pPC->Get_8(SYMB2_ADR_1403))
#define SYMB3_1403	(pPC->Get_8(SYMB3_ADR_1403))

void Clcdc_pc1403::disp_symb(void)
{


    if (DirtyBuf[SYMB1_ADR_1403-0x3000] || DirtyBuf[SYMB2_ADR_1403-0x3000] || DirtyBuf[SYMB3_ADR_1403-0x3000])
    {

    disp_one_symb(S_BUSY,	COLOR(SYMB2_1403&0x01),	pc1403_pos[0].x,	pc1403_pos[0].y);
    disp_one_symb(S_DEF,	COLOR(SYMB2_1403&0x02),	pc1403_pos[1].x,	pc1403_pos[1].y);
    disp_one_symb(S_SHIFT,	COLOR(SYMB2_1403&0x04),	pc1403_pos[2].x,	pc1403_pos[2].y);
    disp_one_symb(S_HYP,	COLOR(SYMB2_1403&0x08),	pc1403_pos[3].x,	pc1403_pos[3].y);
    disp_one_symb(S_DE,		COLOR(SYMB3_1403&0x20),	pc1403_pos[5].x,	pc1403_pos[5].y);
    disp_one_symb(S_G,		COLOR(SYMB3_1403&0x10),	pc1403_pos[6].x,	pc1403_pos[6].y);
    disp_one_symb(S_RAD,	COLOR(SYMB3_1403&0x08),	pc1403_pos[7].x,	pc1403_pos[7].y);
    disp_one_symb(S_O_BRA,	COLOR(SYMB3_1403&0x04),	pc1403_pos[8].x,	pc1403_pos[8].y);
    disp_one_symb(S_C_BRA,	COLOR(SYMB3_1403&0x04),	pc1403_pos[9].x,	pc1403_pos[9].y);
    disp_one_symb(S_REV_M,	COLOR(SYMB3_1403&0x02),	pc1403_pos[10].x,	pc1403_pos[10].y);
    disp_one_symb(S_E,		COLOR(SYMB3_1403&0x01),	pc1403_pos[11].x,	pc1403_pos[11].y);

// CAL
    disp_one_symb(S_BAR35,	COLOR(SYMB2_1403&0x40),	pc1403_pos[12].x,	pc1403_pos[12].y);
// RUN
    disp_one_symb(S_BAR35,	COLOR(SYMB2_1403&0x20),	pc1403_pos[13].x,	pc1403_pos[13].y);
// PRO
    disp_one_symb(S_BAR35,	COLOR(SYMB2_1403&0x10),	pc1403_pos[14].x,	pc1403_pos[14].y);
// STAT
    disp_one_symb(S_BAR35,	COLOR(SYMB1_1403&0x08),	pc1403_pos[15].x,	pc1403_pos[15].y);
// PRINT
    disp_one_symb(S_BAR35,	COLOR(SYMB3_1403&0x40),	pc1403_pos[16].x,	pc1403_pos[16].y);

    DirtyBuf[SYMB1_ADR_1403-0x3000] = 0;
    DirtyBuf[SYMB2_ADR_1403-0x3000] = 0;
    DirtyBuf[SYMB3_ADR_1403-0x3000] = 0;

    Refresh = true;
    }

//    Clcdc::disp_symb();

}

Clcdc_pc1403::Clcdc_pc1403(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]

    internalSize = QSize(144,7);
    pixelSize = 4;
    pixelGap = 1;
}

void Clcdc_pc1403::disp(void)
{

    BYTE b,data,x,y;
    int ind;
    WORD adr;

    lock.lock();

    Refresh = false;

    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    // 1-6
    for (ind=0; ind<0x1E; ind++)
    {
        adr = 0x3000 + ind;
        if (DirtyBuf[adr-0x3000])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0 );
            x =ind + (ind/5);			// +1 every 5 cols
            y = 0;
            for (b=0; b<8;b++)
            {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, y+b);
            }
            DirtyBuf[adr-0x3000] = 0;
        }
    }

    // 7-9
    for (ind=0; ind<0x0f; ind++)
    {
        adr = 0x302D + ind;
        if (DirtyBuf[adr-0x3000])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0 );


            x = 36 + ind + (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<8;b++)
            {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, y+b);
            }
            DirtyBuf[adr-0x3000] = 0;
        }
    }

    // 10-12
    for (ind=0; ind<0x0f; ind++)
    {
        adr = 0x301E + ind;
        if (DirtyBuf[adr-0x3000])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0 );

            x = 54 + ind + (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<8;b++)
            {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, y+b);
            }
            DirtyBuf[adr-0x3000] = 0;
        }
    }

    // 13-15
    for (ind=0x0e; ind>=0; ind--)
    {
        adr = 0x3000+0x5e + ind;
        if (DirtyBuf[adr-0x3000])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0 );

            x = 88 - ind - (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<8;b++)
            {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, y+b);
            }
            DirtyBuf[adr-0x3000] = 0;
        }
    }

    // 16-18
    for (ind=0x0e; ind>=0; ind--)
    {
        adr = 0x3000+0x6d + ind;
        if (DirtyBuf[adr-0x3000])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0 );

            x = 106 - ind - (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<8;b++)
            {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, y+b);
            }
            DirtyBuf[adr-0x3000] = 0;
        }
    }

    //	19-24

    for (ind=0x1d; ind>=0; ind--)
    {
        adr = 0x3000 + 0x40 + ind;
        if (DirtyBuf[adr-0x3000])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0 );
            x = 142 - ind - (ind/5);			// +1 every 5 cols
            y = 0;
            for (b=0; b<8;b++)
            {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, y+b);
            }
            DirtyBuf[adr-0x3000] = 0;
        }
    }


    painter.end();

    lock.unlock();

}


///////////////////////////////////////////////////////////////////////
//
//  PC 1425
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
} pc1425_pos[17]={
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
    {16 , 32},		// CAL
    {45 , 32},		// RUN
    {60 , 32},		// PRO
    {2, 32},		// STAT
    {180, 32}		// PRINT

};


#define SYMB1_ADR_1425	0x303C
#define SYMB2_ADR_1425	0x303D
#define SYMB3_ADR_1425	0x307C
#define SYMB1_1425	(pPC->Get_8(SYMB1_ADR_1425))
#define SYMB2_1425	(pPC->Get_8(SYMB2_ADR_1425))
#define SYMB3_1425	(pPC->Get_8(SYMB3_ADR_1425))

void Clcdc_pc1425::disp_symb(void)
{


    if (DirtyBuf[SYMB1_ADR_1425-0x3000] || DirtyBuf[SYMB2_ADR_1425-0x3000] || DirtyBuf[SYMB3_ADR_1425-0x3000])
    {

    disp_one_symb(S_BUSY,	COLOR(SYMB2_1425&0x01),	pc1425_pos[0].x,	pc1425_pos[0].y);
    disp_one_symb(S_DEF,	COLOR(SYMB2_1425&0x02),	pc1425_pos[1].x,	pc1425_pos[1].y);
    disp_one_symb(S_SHIFT,	COLOR(SYMB2_1425&0x04),	pc1425_pos[2].x,	pc1425_pos[2].y);
    disp_one_symb(S_HYP,	COLOR(SYMB2_1425&0x08),	pc1425_pos[3].x,	pc1425_pos[3].y);
    disp_one_symb(S_DE,		COLOR(SYMB3_1425&0x20),	pc1425_pos[5].x,	pc1425_pos[5].y);
    disp_one_symb(S_G,		COLOR(SYMB3_1425&0x10),	pc1425_pos[6].x,	pc1425_pos[6].y);
    disp_one_symb(S_RAD,	COLOR(SYMB3_1425&0x08),	pc1425_pos[7].x,	pc1425_pos[7].y);
    disp_one_symb(S_O_BRA,	COLOR(SYMB3_1425&0x04),	pc1425_pos[8].x,	pc1425_pos[8].y);
    disp_one_symb(S_C_BRA,	COLOR(SYMB3_1425&0x04),	pc1425_pos[9].x,	pc1425_pos[9].y);
    disp_one_symb(S_REV_M,	COLOR(SYMB3_1425&0x02),	pc1425_pos[10].x,	pc1425_pos[10].y);
    disp_one_symb(S_E,		COLOR(SYMB3_1425&0x01),	pc1425_pos[11].x,	pc1425_pos[11].y);

// CAL
    disp_one_symb(S_BAR35,	COLOR(SYMB2_1425&0x40),	pc1425_pos[12].x,	pc1425_pos[12].y);
// RUN
    disp_one_symb(S_BAR35,	COLOR(SYMB2_1425&0x10),	pc1425_pos[13].x,	pc1425_pos[13].y);
// PRO
    disp_one_symb(S_BAR35,	COLOR(SYMB1_1425&0x20),	pc1425_pos[14].x,	pc1425_pos[14].y);
// STAT
    disp_one_symb(S_BAR35,	COLOR(SYMB1_1425&0x40),	pc1425_pos[15].x,	pc1425_pos[15].y);
// PRINT
    disp_one_symb(S_BAR35,	COLOR(SYMB3_1425&0x40),	pc1425_pos[16].x,	pc1425_pos[16].y);

    DirtyBuf[SYMB1_ADR_1425-0x3000] = 0;
    DirtyBuf[SYMB2_ADR_1425-0x3000] = 0;
    DirtyBuf[SYMB3_ADR_1425-0x3000] = 0;

    Refresh = true;
    }

    Clcdc::disp_symb();

}

Clcdc_pc1425::Clcdc_pc1425(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc_pc1403(parent,_lcdRect,_symbRect,_lcdfname,_symbfname)
{						//[constructor]

}


