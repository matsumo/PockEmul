#include <QDebug>
#include <QPainter>

#include "Lcdc_rlh1000.h"
#include "Lcdc_symb.h"
#include "pcxxxx.h"

static const struct {
    int x,y;
} rlh1000_pos[15]={
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
#define SYMB1_ADR_rlh1000	0x764E
#define SYMB2_ADR_rlh1000	0x764F
#define SYMB1_rlh1000		(pPC->Get_8(SYMB1_ADR_rlh1000))
#define SYMB2_rlh1000		(pPC->Get_8(SYMB2_ADR_rlh1000))

void Clcdc_rlh1000::disp_symb(void)
{



    if (DirtyBuf[SYMB1_ADR_rlh1000-0x7600] || DirtyBuf[SYMB2_ADR_rlh1000-0x7600])
    {
        disp_one_symb( S_BUSY,		COLOR(SYMB1_rlh1000&1),	rlh1000_pos[0].x,	rlh1000_pos[0].y);
        disp_one_symb( S_SHIFT,		COLOR(SYMB1_rlh1000&2),	rlh1000_pos[1].x,	rlh1000_pos[1].y);
        disp_one_symb( S_JAP,			COLOR(SYMB1_rlh1000&4),	rlh1000_pos[2].x,	rlh1000_pos[2].y);
        disp_one_symb( S_SMALL,		COLOR(SYMB1_rlh1000&8),	rlh1000_pos[3].x,	rlh1000_pos[3].y);
        disp_one_symb( S_DE,			COLOR(SYMB2_rlh1000&0x01),	rlh1000_pos[4].x,	rlh1000_pos[4].y);
        disp_one_symb( S_G,			COLOR(SYMB2_rlh1000&0x02),	rlh1000_pos[5].x,	rlh1000_pos[5].y);
        disp_one_symb( S_RAD,			COLOR(SYMB2_rlh1000&0x04),	rlh1000_pos[6].x,	rlh1000_pos[6].y);
        disp_one_symb( S_RUN,			COLOR(SYMB2_rlh1000&0x40),	rlh1000_pos[7].x,	rlh1000_pos[7].y);
        disp_one_symb( S_PRO,			COLOR(SYMB2_rlh1000&0x20),	rlh1000_pos[8].x,	rlh1000_pos[8].y);
        disp_one_symb( S_RESERVE,		COLOR(SYMB2_rlh1000&0x10),	rlh1000_pos[9].x,	rlh1000_pos[9].y);
        disp_one_symb( S_DEF,			COLOR(SYMB1_rlh1000&0x80),	rlh1000_pos[10].x,	rlh1000_pos[10].y);
        disp_one_symb( S_ROMEAN_I,	COLOR(SYMB1_rlh1000&0x40),	rlh1000_pos[11].x,	rlh1000_pos[11].y);
        disp_one_symb( S_ROMEAN_II,	COLOR(SYMB1_rlh1000&0x20),	rlh1000_pos[12].x,	rlh1000_pos[12].y);
        disp_one_symb( S_ROMEAN_III,	COLOR(SYMB1_rlh1000&0x10),	rlh1000_pos[13].x,	rlh1000_pos[13].y);
        disp_one_symb( S_BATTERY,		COLOR(1),				rlh1000_pos[14].x,	rlh1000_pos[14].y);

        DirtyBuf[SYMB1_ADR_rlh1000-0x7600] = 0;
        DirtyBuf[SYMB2_ADR_rlh1000-0x7600] = 0;

        Refresh = true;
    }

    Clcdc::disp_symb();
}



void Clcdc_rlh1000::disp(void)
{

    BYTE b,data;
    int ind;

    Refresh = false;

//	disp_symb();

    QPainter painter(pPC->LcdImage);

On=true;
    for (ind=0; ind<0xA0; ind++)
    {
        if ( (DirtyBuf[ind]) )
        {

            Refresh = true;
            if (On)
            {
                data = mem[ind];
                DirtyBuf[ind] = 0;
            }
            else
            {
                data = 0;
                DirtyBuf[ind] = 0;
            }

            for (b=0; b<8;b++)
            {
                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
                painter.drawPoint( ind, b);
            }
        }
    }

    redraw = 0;
    painter.end();
}

