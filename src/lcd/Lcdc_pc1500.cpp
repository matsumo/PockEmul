#include <QPainter>
#include "pcxxxx.h"

#include "Lcdc_pc1500.h"
#include "Lcdc_symb.h"


///////////////////////////////////////////////////////////////////////
//
//  PC 1500
//
///////////////////////////////////////////////////////////////////////

#define LOW(b)		( (b) & 0x0f)
#define HIGHT(b)	( (b) >> 4)


static const struct {
    int x,y;
} pc1500_pos[15]={
    {0, 0},
    {30, 0},
    {60, 0},
    {80, 0},
    {112,0}, //de
    {120,0},
    {125,0},
    {160,0}, // run
    {190,0},
    {210,0},
    {250,0},
    {276,0}, // i
    {282,0},
    {292,0},
    {312,0}
};



/* busy  shift   small   de g rad   run  pro  reserve  def  i ii iii battery */
/* japanese? */
#define SYMB1_ADR_1500	0x764E
#define SYMB2_ADR_1500	0x764F
#define SYMB1_1500		(pPC->Get_8(SYMB1_ADR_1500))
#define SYMB2_1500		(pPC->Get_8(SYMB2_ADR_1500))

void Clcdc_pc1500::disp_symb(void)
{



    if (DirtyBuf[SYMB1_ADR_1500-0x7600] || DirtyBuf[SYMB2_ADR_1500-0x7600])
    {
        disp_one_symb( S_BUSY,		COLOR(SYMB1_1500&1),	pc1500_pos[0].x,	pc1500_pos[0].y);
        disp_one_symb( S_SHIFT,		COLOR(SYMB1_1500&2),	pc1500_pos[1].x,	pc1500_pos[1].y);
        disp_one_symb( S_JAP,			COLOR(SYMB1_1500&4),	pc1500_pos[2].x,	pc1500_pos[2].y);
        disp_one_symb( S_SMALL,		COLOR(SYMB1_1500&8),	pc1500_pos[3].x,	pc1500_pos[3].y);
        disp_one_symb( S_DE,			COLOR(SYMB2_1500&0x01),	pc1500_pos[4].x,	pc1500_pos[4].y);
        disp_one_symb( S_G,			COLOR(SYMB2_1500&0x02),	pc1500_pos[5].x,	pc1500_pos[5].y);
        disp_one_symb( S_RAD,			COLOR(SYMB2_1500&0x04),	pc1500_pos[6].x,	pc1500_pos[6].y);
        disp_one_symb( S_RUN,			COLOR(SYMB2_1500&0x40),	pc1500_pos[7].x,	pc1500_pos[7].y);
        disp_one_symb( S_PRO,			COLOR(SYMB2_1500&0x20),	pc1500_pos[8].x,	pc1500_pos[8].y);
        disp_one_symb( S_RESERVE,		COLOR(SYMB2_1500&0x10),	pc1500_pos[9].x,	pc1500_pos[9].y);
        disp_one_symb( S_DEF,			COLOR(SYMB1_1500&0x80),	pc1500_pos[10].x,	pc1500_pos[10].y);
        disp_one_symb( S_ROMEAN_I,	COLOR(SYMB1_1500&0x40),	pc1500_pos[11].x,	pc1500_pos[11].y);
        disp_one_symb( S_ROMEAN_II,	COLOR(SYMB1_1500&0x20),	pc1500_pos[12].x,	pc1500_pos[12].y);
        disp_one_symb( S_ROMEAN_III,	COLOR(SYMB1_1500&0x10),	pc1500_pos[13].x,	pc1500_pos[13].y);
        disp_one_symb( S_BATTERY,		COLOR(1),				pc1500_pos[14].x,	pc1500_pos[14].y);

        DirtyBuf[SYMB1_ADR_1500-0x7600] = 0;
        DirtyBuf[SYMB2_ADR_1500-0x7600] = 0;

        Refresh = true;
    }

    Clcdc::disp_symb();
}

Clcdc_pc1500::Clcdc_pc1500(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]
    internalSize = QSize(156,7);
    pixelSize = 4;
    pixelGap = 1;
}



void Clcdc_pc1500::disp(void)
{

    BYTE b,data,x;
    int ind;
    WORD adr;

    Refresh = false;

    disp_symb();

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    for (ind=0; ind<0x4D; ind+=2)
    {	adr = 0x7600 + ind;
        if ( (DirtyBuf[adr-0x7600]))
        {
            Refresh = true;
            DirtyBuf[adr-0x7600] = 0;
            if (On)
            {
                data = (BYTE) ( LOW(pPC->Get_8(adr)) | ( LOW(pPC->Get_8(adr+1)) << 4) );
            }
            else
            {
                data = 0;
            }

            x = ind >> 1;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x,b,((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, b);
            }

            if (On)
            {
                data = (BYTE) ( HIGHT(pPC->Get_8(adr)) | ( HIGHT(pPC->Get_8(adr+1)) << 4) );
            }
            else
            {
                data = 0;
            }

            x += 78;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x,b,((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, b);
            }

        }
    }

    for (ind=0; ind<0x4D; ind+=2)
    {	adr = 0x7700 + ind;
        if ( (DirtyBuf[adr-0x7600]) )
        {
            Refresh = true;
            DirtyBuf[adr-0x7600] = 0;
            if (On)
            {
                data = (BYTE) ( LOW(pPC->Get_8(adr)) | ( LOW(pPC->Get_8(adr+1)) << 4) );
            }
            else
            {
                data = 0;
            }

            x = (ind >> 1) + 39;

            for (b=0; b<7;b++)
                {
                    drawPixel(&painter,x,b,((data>>b)&0x01) ? Color_On : Color_Off );
//                    painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                    painter.drawPoint( x, b);
                }

            if (On)
            {
                data = (BYTE) ( HIGHT(pPC->Get_8(adr)) | ( HIGHT(pPC->Get_8(adr+1)) << 4) );
            }
            else
            {
                data = 0;
            }

            x += 78;

            for (b=0; b<7;b++) {
                drawPixel(&painter,x,b,((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
//                painter.drawPoint( x, b);
            }

        }
    }

    redraw = 0;
    painter.end();
}
