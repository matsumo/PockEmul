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


//static const struct {
//	int x,y;
//} pc1250_pos[11]={
//    {250,0},	// SHIFT
//    {48, 0},	// DEF
//    {72, 0},	// PRO
//    {100, 0},	// RUN
//    {118, 0},	// RESERVE
//    {158,0},	// DE
//    {166,0},	// G
//    {171,0},	// RAD
//    {290,0},	// E
//    {30,0},		// P
//	{0,0}		// BUSY
//};


//#define SYMB1_ADR_1250	0xF83C
//#define SYMB2_ADR_1250	0xF83D
//#define SYMB3_ADR_1250	0xF83E
//#define SYMB1_1250		(pPC->Get_8(SYMB1_ADR_1250))
//#define SYMB2_1250		(pPC->Get_8(SYMB2_ADR_1250))
//#define SYMB3_1250		(pPC->Get_8(SYMB3_ADR_1250))

//void Clcdc_pc1250::disp_symb(void)
//{

//	if ( (DirtyBuf[SYMB1_ADR_1250-0xF800]) ||
//		 (DirtyBuf[SYMB2_ADR_1250-0xF800]) ||
//		 (DirtyBuf[SYMB3_ADR_1250-0xF800])
//		)
//    {

//        disp_one_symb(S_SHIFT,	COLOR(SYMB2_1250&0x02),	pc1250_pos[0].x,	pc1250_pos[0].y);
//        disp_one_symb(S_DEF,		COLOR(SYMB1_1250&0x01),	pc1250_pos[1].x,	pc1250_pos[1].y);
//        disp_one_symb(S_PRO,		COLOR(SYMB3_1250&0x01),	pc1250_pos[2].x,	pc1250_pos[2].y);
//        disp_one_symb(S_RUN,		COLOR(SYMB3_1250&0x02),	pc1250_pos[3].x,	pc1250_pos[3].y);
//        disp_one_symb(S_RESERVE,	COLOR(SYMB3_1250&0x04),	pc1250_pos[4].x,	pc1250_pos[4].y);
//        disp_one_symb(S_DE,		COLOR(SYMB1_1250&0x08),	pc1250_pos[5].x,	pc1250_pos[5].y);
//        disp_one_symb(S_G,		COLOR(SYMB1_1250&0x04),	pc1250_pos[6].x,	pc1250_pos[6].y);
//        disp_one_symb(S_RAD,		COLOR(SYMB2_1250&0x04),	pc1250_pos[7].x,	pc1250_pos[7].y);
//        disp_one_symb(S_E,		COLOR(SYMB2_1250&0x08),	pc1250_pos[8].x,	pc1250_pos[8].y);
//        disp_one_symb(S_PRINT,	COLOR(SYMB1_1250&0x02),	pc1250_pos[9].x,	pc1250_pos[9].y);
//        disp_one_symb(S_BUSY,	COLOR(SYMB2_1250&0x01),	pc1250_pos[10].x,	pc1250_pos[10].y);

//        DirtyBuf[SYMB1_ADR_1250-0xF800] = false;
//        DirtyBuf[SYMB2_ADR_1250-0xF800] = false;
//        DirtyBuf[SYMB3_ADR_1250-0xF800] = false;

//        Refresh = true;
//    }

//	Clcdc::disp_symb();

//}

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
    BYTE b,data,x,y;
    int ind;
    WORD adr;

    Refresh = false;

    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    for (ind=0; ind<0x3c; ind++)
    {	adr = baseAdr + ind;
        if (DirtyBuf[adr-baseAdr])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);

            x =ind + (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off);
            }
            DirtyBuf[adr-baseAdr] = 0;
        }
    }

    for (ind=0x3B; ind>=0; ind--)
    {	adr = baseAdr + 0x40 + ind;
        if (DirtyBuf[adr-baseAdr])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);

            x = 142 - ind - (ind/5);			// +2 every 5 cols
            y = 0;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off);
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, y+b);
            }
            DirtyBuf[adr-baseAdr] = 0;
        }
    }


    painter.end();
}

///////////////////////////////////////////////////////////////////////
//
//  PC 1245
//
///////////////////////////////////////////////////////////////////////


static const struct {
    int x,y;
} pc1245_pos[9]={
    {0, 0},		// SHIFT
    {20, 0},	// DEF
    {38, 0},	// PRO
    {53, 0},	// RUN
    {68, 0},	// RESERVE
    {134,0},	// DE
    {142,0},	// G
    {147,0},	// RAD
    {125,0}
};


#define SYMB1_ADR_1245	0xF83C
#define SYMB2_ADR_1245	0xF83D
#define SYMB3_ADR_1245	0xF83E
#define SYMB1_1245		(pPC->Get_8(SYMB1_ADR_1245))
#define SYMB2_1245		(pPC->Get_8(SYMB2_ADR_1245))
#define SYMB3_1245		(pPC->Get_8(SYMB3_ADR_1245))

void Clcdc_pc1245::disp_symb(void)
{


    if ( (DirtyBuf[SYMB1_ADR_1245-0xF800]) ||
         (DirtyBuf[SYMB2_ADR_1245-0xF800]) ||
         (DirtyBuf[SYMB3_ADR_1245-0xF800])
        )
    {

    disp_one_symb(S_SHIFT,	COLOR(SYMB2_1245&0x02),	pc1245_pos[0].x,	pc1245_pos[0].y);
    disp_one_symb(S_DEF,		COLOR(SYMB1_1245&0x01),	pc1245_pos[1].x,	pc1245_pos[1].y);
    disp_one_symb(S_PRO,		COLOR(SYMB3_1245&0x01),	pc1245_pos[2].x,	pc1245_pos[2].y);
    disp_one_symb(S_RUN,		COLOR(SYMB3_1245&0x02),	pc1245_pos[3].x,	pc1245_pos[3].y);
//	disp_one_symb(S_RESERVE,	COLOR(SYMB3_1245&0x04),	pc1245_pos[4].x,	pc1245_pos[4].y);
    disp_one_symb(S_DE,		COLOR(SYMB1_1245&0x08),	pc1245_pos[5].x,	pc1245_pos[5].y);
    disp_one_symb(S_G,		COLOR(SYMB1_1245&0x04),	pc1245_pos[6].x,	pc1245_pos[6].y);
    disp_one_symb(S_RAD,		COLOR(SYMB2_1245&0x04),	pc1245_pos[7].x,	pc1245_pos[7].y);
    disp_one_symb(S_PRINT,	COLOR(SYMB1_1245&0x02),	125,	0);

    DirtyBuf[SYMB1_ADR_1245-0xF800] = false;
    DirtyBuf[SYMB2_ADR_1245-0xF800] = false;
    DirtyBuf[SYMB3_ADR_1245-0xF800] = false;

        Refresh = true;

    }

    Clcdc::disp_symb();
}

void Clcdc_pc1245::disp(void)
{
    BYTE b,data,x,y;
    int ind;
    WORD adr;

    Refresh = false;

    disp_symb();

    QPainter painter(LcdImage);

    for (ind=0; ind<0x3c; ind++)
    {	adr = 0xF800 + ind;
        if (DirtyBuf[adr-0xF800])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);

            x =ind + (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off);
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, y+b);
            }
            DirtyBuf[adr-0xF800] = 0;
        }
    }

    for (ind=0x3B; ind>=0x28; ind--)
    {	adr = 0xF800 + 0x40 + ind;
        if (DirtyBuf[adr-0xF800])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);

            x = 142 - ind - (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off);
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, y+b);
            }
            DirtyBuf[adr-0xF800] = 0;
        }
    }

    painter.end();
}

Clcdc_pc1245::Clcdc_pc1245(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc_pc1250(parent,_lcdRect,_symbRect,_lcdfname,_symbfname)
{						//[constructor]

    internalSize = QSize(96,8);
}

///////////////////////////////////////////////////////////////////////
//
//  PC 1260
//
///////////////////////////////////////////////////////////////////////

static const struct {
    int x,y;
} pc1260_pos[11]={
    {27, 31},	// BUSY
    {67, 31},	// PRINT
    {107, 31},	// DEG
    {133,31},	// RAD
    {160,31},	// GRAD
    {200,31},	// ERROR
    {0,0},		// JAP
    {0,8},		// SMALL
    {0,16},		// SHIFT
    {0,24}		// DEF
};


#define SYMB1_ADR_1260	0x207C
#define SYMB2_ADR_1260	0x203D
#define SYMB3_ADR_1260	0x283E
#define SYMB1_1260		(pPC->Get_8(SYMB1_ADR_1260))
#define SYMB2_1260		(pPC->Get_8(SYMB2_ADR_1260))
#define SYMB3_1260		(pPC->Get_8(SYMB3_ADR_1260))

void Clcdc_pc1260::disp_symb(void)
{

    if ( (DirtyBuf[SYMB1_ADR_1260-0x2000]) ||
         (DirtyBuf[SYMB2_ADR_1260-0x2000]) ||
         (DirtyBuf[SYMB3_ADR_1260-0x2000])
        )
    {

    disp_one_symb(S_BAR25,	COLOR(SYMB2_1260&0x01),	pc1260_pos[0].x,	pc1260_pos[0].y);
    disp_one_symb(S_BAR25,	COLOR(SYMB2_1260&0x02),	pc1260_pos[1].x,	pc1260_pos[1].y);
    disp_one_symb(S_BAR25,	COLOR(SYMB1_1260&0x01),	pc1260_pos[2].x,	pc1260_pos[2].y);
    disp_one_symb(S_BAR25,	COLOR(SYMB1_1260&0x02),	pc1260_pos[3].x,	pc1260_pos[3].y);
    disp_one_symb(S_BAR25,	COLOR(SYMB1_1260&0x04),	pc1260_pos[4].x,	pc1260_pos[4].y);
    disp_one_symb(S_BAR25,	COLOR(SYMB1_1260&0x20),	pc1260_pos[5].x,	pc1260_pos[5].y);
    disp_one_symb(S_JAP,	COLOR(SYMB2_1260&0x08),	pc1260_pos[6].x,	pc1260_pos[6].y);
    disp_one_symb(S_SMALL,	COLOR(SYMB2_1260&0x10),	pc1260_pos[7].x,	pc1260_pos[7].y);
    disp_one_symb(S_SHIFT,	COLOR(SYMB2_1260&0x20),	pc1260_pos[8].x,	pc1260_pos[8].y);
    disp_one_symb(S_DEF,	COLOR(SYMB2_1260&0x40),	pc1260_pos[9].x,	pc1260_pos[9].y);

    DirtyBuf[SYMB1_ADR_1260-0x2000] = false;
    DirtyBuf[SYMB2_ADR_1260-0x2000] = false;
    DirtyBuf[SYMB3_ADR_1260-0x2000] = false;

    Refresh = true;
    }

//    Clcdc::disp_symb();

}


void Clcdc_pc1260::disp(void)
{
    BYTE b,data;
    int x,y;
    int ind;
    WORD adr;

    Refresh = false;

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
        {	adr = memOffset + 0x2000 + ind;
            if (DirtyBuf[adr-0x2000])
            {
                Refresh = true;
                data = ( On ? (BYTE) pPC->Get_8(adr) : 0);

                x = xOffset + ind /*+ (ind/5)*/;			// +1 every 5 cols
                y = yOffset;

                for (b=0; b<7;b++)
                {
                    drawPixel(&painter,x + (ind/5)*1.5f ,y+b,((data>>b)&0x01) ? Color_On : Color_Off);
//                    painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                    if (pixelSize>1) {
//                        painter.setBrush(((data>>b)&0x01) ? Color_On : Color_Off);
//                        painter.drawRect(x*(pixelSize+pixelGap) + (ind/5)*(pixelSize+pixelGap)*3/2,
//                                         (y+b)*(pixelSize+pixelGap),
//                                         pixelSize-1,
//                                         pixelSize-1);
//                    }
//                    else {
//                        painter.drawPoint( x, y+b);
//                    }
                }
                DirtyBuf[adr-0x2000] = 0;
            }
        }
    }

    painter.end();

}

Clcdc_pc1260::Clcdc_pc1260(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc_pc1250(parent,_lcdRect,_symbRect,_lcdfname,_symbfname)
{
    Color_Off = Qt::transparent;

    internalSize = QSize(156,16);
    pixelSize = 4;
    pixelGap = 1;

}


