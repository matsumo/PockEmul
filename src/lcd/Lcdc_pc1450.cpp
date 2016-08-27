#include <QPainter>
#include "pcxxxx.h"
#include "Lcdc_pc1450.h"
#include "Lcdc_symb.h"




/*******************/
/***	1450	****/
/*******************/

static const struct {
    int x,y;
} pc1450_pos[17]={
{0  , 0},		// BUSY
{20 , 0},		// DEF
{45 , 0},		// SHIFT
{67 , 0},		// HYP
{105 , 0},		// SML
{125, 0},		// DE
{133, 0},		// G
{138, 0},		// RAD
{170, 0},		// OPEN_BRACKET,
    {174, 0},		// CLOSE_BRACKET
    {177, 0},		// M
    {185, 0},		// E
    {10 , 32},		// CAL
    {26 , 32},		// RUN
    {41 , 32},		// PRO
    {175, 32},		// STAT
    {190, 32}		// PRINT

};


#define SYMB1_ADR_1450	0x703C
#define SYMB2_ADR_1450	0x703D
#define SYMB3_ADR_1450	0x707C
#define SYMB1_1450	(pPC->Get_8(SYMB1_ADR_1450))
#define SYMB2_1450	(pPC->Get_8(SYMB2_ADR_1450))
#define SYMB3_1450	(pPC->Get_8(SYMB3_ADR_1450))

void Clcdc_pc1450::disp_symb(void)
{

    if (DirtyBuf[SYMB1_ADR_1450-0x7000] || DirtyBuf[SYMB2_ADR_1450-0x7000] || DirtyBuf[SYMB3_ADR_1450-0x7000])
    {

    disp_one_symb(S_BUSY,		COLOR(SYMB2_1450&0x01),	pc1450_pos[0].x,	pc1450_pos[0].y);
    disp_one_symb(S_DEF,		COLOR(SYMB2_1450&0x02),	pc1450_pos[1].x,	pc1450_pos[1].y);
    disp_one_symb(S_SHIFT,	COLOR(SYMB2_1450&0x04),	pc1450_pos[2].x,	pc1450_pos[2].y);
    disp_one_symb(S_HYP,		COLOR(SYMB2_1450&0x08),	pc1450_pos[3].x,	pc1450_pos[3].y);
    disp_one_symb(S_SML,		COLOR(SYMB1_1450&0x04),	pc1450_pos[4].x,	pc1450_pos[4].y);
    disp_one_symb(S_DE,		COLOR(SYMB3_1450&0x20),	pc1450_pos[5].x,	pc1450_pos[5].y);
    disp_one_symb(S_G,		COLOR(SYMB3_1450&0x10),	pc1450_pos[6].x,	pc1450_pos[6].y);
    disp_one_symb(S_RAD,		COLOR(SYMB3_1450&0x08),	pc1450_pos[7].x,	pc1450_pos[7].y);
    disp_one_symb(S_O_BRA,	COLOR(SYMB3_1450&0x04),	pc1450_pos[8].x,	pc1450_pos[8].y);
    disp_one_symb(S_C_BRA,	COLOR(SYMB3_1450&0x04),	pc1450_pos[9].x,	pc1450_pos[9].y);
    disp_one_symb(S_REV_M,	COLOR(SYMB3_1450&0x02),	pc1450_pos[10].x,	pc1450_pos[10].y);
    disp_one_symb(S_E,		COLOR(SYMB3_1450&0x01),	pc1450_pos[11].x,	pc1450_pos[11].y);

// CAL
    disp_one_symb(S_BAR35,	COLOR(SYMB2_1450&0x40),	pc1450_pos[12].x,	pc1450_pos[12].y);
// RUN
    disp_one_symb(S_BAR35,	COLOR(SYMB2_1450&0x20),	pc1450_pos[13].x,	pc1450_pos[13].y);
// PRO
    disp_one_symb(S_BAR35,	COLOR(SYMB2_1450&0x10),	pc1450_pos[14].x,	pc1450_pos[14].y);
// STAT
    disp_one_symb(S_BAR35,	COLOR(SYMB1_1450&0x08),	pc1450_pos[15].x,	pc1450_pos[15].y);
// PRINT
    disp_one_symb(S_BAR35,	COLOR(SYMB3_1450&0x40),	pc1450_pos[16].x,	pc1450_pos[16].y);

    DirtyBuf[SYMB1_ADR_1450-0x7000] = 0;
    DirtyBuf[SYMB2_ADR_1450-0x7000] = 0;
    DirtyBuf[SYMB3_ADR_1450-0x7000] = 0;
    Refresh = true;

    }

    Clcdc::disp_symb();
}

Clcdc_pc1450::Clcdc_pc1450(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]

    internalSize = QSize(96,7);
    pixelSize = 4;
    pixelGap = 1;
}

bool	Clcdc_pc1450::init(void)
{
    Clcdc::init();

#define LCDX1 (5 * 12)
#define LCDX2 (5 * 16)

#define LCD1 0x7000
#define LCD2 0x703c
#define LCD3 0x7068
#define LCD4 0x707c

    int x;
    for (x = 0; x < LCDX1; x++) {
    x2a[x] = LCD1 + x;
    }
    for (x = LCDX1; x < LCDX2; x++) {
    x2a[x] = LCD3 + (LCDX2 - x - 1);
    }

    return(1);

}

void Clcdc_pc1450::disp(void)
{
    BYTE b,data,x,y;
    int ind;
    WORD adr;

    lock.lock();

    Refresh = false;
    disp_symb();
    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    for (ind=0; ind<80; ind++)
    {
        adr = x2a[ind];
        if (DirtyBuf[adr-0x7000])
        {
            Refresh = true;
            data = (On ? (BYTE) pPC->Get_8(adr) : 0);

            x =ind + (ind/5);			// +1 every 5 cols
            y = 0;

            for (b=0; b<7;b++)
            {
                drawPixel(&painter,x,y+b,((data>>b)&0x01) ? Color_On : Color_Off);
            }
            DirtyBuf[adr-0x7000] = 0;
        }
    }

    painter.end();

    lock.unlock();
}


