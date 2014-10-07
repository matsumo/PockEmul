/*** PC-XXXX Emulator [LCDC.CPP] **************************/
/* LCD Controller emulation                               */
/**********************************************************/
//#include <string.h>
#include <stdlib.h>
#include <QPainter>
#include <QDebug>

#include "common.h"

#include "pcxxxx.h"

#include "Lcdc.h"
#include "Lcdc_symb.h"

#include "Inter.h"
#include "Log.h"

Clcdc::Clcdc(CPObject *parent)
{						//[constructor]
    pPC = (CpcXXXX*) parent;

    redraw		= true;				//display redraw?(0:not need, 1:need)
    On			= true;
    Refresh		= false;
    updated     = false;
    contrast = 1;

    memset(&DirtyBuf,0,sizeof(DirtyBuf));
    Color_On  = QColor(0,0,0,255);
    Color_Off = QColor(0,0,0,0);
    ready = false;
}

Clcdc::~Clcdc() {

}

void Clcdc::Contrast(int command)
{

	switch (command)
	{
	case 0: contrast = (float) 1;	break;
	case 1: contrast = (float) 0.95;break;
	case 2: contrast = (float) 0.9;	break;
	case 3: contrast = (float) 0.7;	break;
	case 4: contrast = (float) 0.5;	break;
	}
    if ( (Color_Off.red()==0) && (Color_Off.green()==0) && (Color_Off.blue()==0)) {
        qWarning()<<"trabs";
        Color_Off.setAlphaF(1-contrast);
    }
    else {
        qWarning()<<"classic";
        Color_Off = QColor( (int) ( origColor_Off.red() * contrast ),
                            (int) ( origColor_Off.green() * contrast ),
                            (int) ( origColor_Off.blue() * contrast ),
                            origColor_Off.alpha());
    }

	Update();
}

void Clcdc::disp_one_symb(QPainter *painter, const char *figure, QColor color, int x, int y)
{
    int j, xi=0;
    painter->setPen(color);
    for (j=0; figure[j]; j++) {
        switch (figure[j]) {
        case '#':	painter->drawPoint( x+xi, y );
                    xi++;		break;
        case ' ':	xi++;		break;
        case '\r':	xi=0; y++;	break;
        };
    }
}

void Clcdc::disp_one_symb(const char *figure, QColor color, int x, int y)
{
	QPainter painter(pPC->SymbImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    Clcdc::disp_one_symb(&painter,figure,color,x,y);
	painter.end();
}


void Clcdc::disp_symb(void)
{
}

/*****************************************************************************/
/* LCDC																		 */
/* (main)																	 */
/*  ENTRY :none																 */
/*  RETURN: 0=error, 1=success												 */
/*****************************************************************************/
void	Clcdc::check(void)  {}

void	Clcdc::TurnON(void) {

    On = true;
    Refresh = true;
    redraw = true;
    Update();
}

void	Clcdc::TurnOFF(void){ On = false; }

bool	Clcdc::init(void)
{
	On = false;
    ready = true;
    redraw = true;

    origColor_Off = Color_Off;
    Contrast(3);
    AddLog(LOG_MASTER,"Lcd INIT");
	return(true);
}

void	Clcdc::init_screen(void)
{
//	if (!(pPC->LcdImage)) 
//		pPC->LcdImage->load(pPC->LcdFname);
}

bool	Clcdc::exit(void) {	return(true); }

void Clcdc::SetDirtyBuf(WORD index)
{
	DirtyBuf[index]=true;
	Refresh=true;
}

///////////////////////////////////////////////////////////////////////
//
//  PC 1350
//
///////////////////////////////////////////////////////////////////////
#define SYMB1_ADR_1350	0x783C

static const struct {
    int x,y;
    const char *symb;
    UINT32	addr;
    int	bit;
} pc1350_pos[7]={
    {1, 3,  S_PRINT	,SYMB1_ADR_1350	,0x04},
    {8, 3,  S_RUN		,SYMB1_ADR_1350	,0x10},
    {4, 11, S_PRO		,SYMB1_ADR_1350	,0x20},
    {6, 21, S_JAP		,SYMB1_ADR_1350	,0x40},
    {4, 29, S_SML		,SYMB1_ADR_1350	,0x80},
    {1, 39, S_SHIFT	,SYMB1_ADR_1350	,0x01},
    {4, 47, S_DEF		,SYMB1_ADR_1350	,0x02}
};

void Clcdc_pc1350::disp_symb(void)
{

    if (DirtyBuf[SYMB1_ADR_1350-0x7000] )
    {
        for (int ii=0;ii<7;ii++)
            disp_one_symb( pc1350_pos[ii].symb,		COLOR((pPC->Get_8(pc1350_pos[ii].addr)) & pc1350_pos[ii].bit),	pc1350_pos[ii].x,	pc1350_pos[ii].y);

        DirtyBuf[SYMB1_ADR_1350-0x7000] = 0;

        Refresh = true;
    }

    Clcdc::disp_symb();

}

Clcdc_pc1350::Clcdc_pc1350(CPObject *parent)	: Clcdc(parent)
{						//[constructor]
    Color_Off = Qt::transparent;
}

#define PIXEL_SIZE 4
#define PIXEL_GAP 1
void Clcdc_pc1350::disp(void)
{
    BYTE co,li,ind,b,data,x,y;
    WORD adr;

    Refresh = false;

    disp_symb();

    QPainter painter(pPC->LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    for (co=0; co<5; co++)
    {	for (li=0; li<4; li++)
        {	for (ind=0; ind<30; ind++)
            {
                adr = ind + (0x0200 * co);
                switch (li)
                {
                    case 0 : adr += 0x7000; break;
                    case 1 : adr += 0x7040; break;
                    case 2 : adr += 0x701E; break;
                    case 3 : adr += 0x705E; break;
                }
                if (DirtyBuf[adr-0x7000])
                {
                    Refresh = true;
                    x = ind + (co * 30);
                    y = 8 * li;

                    data = ( On ? (BYTE) pPC->Get_8(adr) : 0 );

                    for (b=0; b<8;b++)
                    {
                        painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
                        painter.setBrush(((data>>b)&0x01) ? Color_On : Color_Off);
                        //painter.drawPoint( x, y+b);
                        painter.drawRect(x*(PIXEL_SIZE+PIXEL_GAP),
                                         (y+b)*(PIXEL_SIZE+PIXEL_GAP),
                                         PIXEL_SIZE-1,
                                         PIXEL_SIZE-1);
                    }
                    DirtyBuf[adr-0x7000]=0;
                }
            }
        }
    }
    redraw = 0;
    painter.end();
}


///////////////////////////////////////////////////////////////////////
//
//  PC 2500
//
///////////////////////////////////////////////////////////////////////
#define SYMB1_ADR_2500	0x783C
#define SYMB2_ADR_2500	0x787C
static const struct {
	int x,y;
	const char *symb;
	UINT32	addr;
	int	bit;
} pc2500_pos[6]={
    {0, 0,  S_BUSY	,SYMB2_ADR_2500	,0x80},
    {223, 0,  S_RUN	,SYMB1_ADR_2500	,0x10},
    {260, 0, S_PRO	,SYMB1_ADR_2500	,0x20},
    {120, 0, S_JAP	,SYMB1_ADR_2500	,0x40},
    {60, 0, S_CAPS	,SYMB1_ADR_2500	,0x80},
    {185, 0, S_DEF	,SYMB1_ADR_2500	,0x02}
};

void Clcdc_pc2500::disp_symb(void)
{

    if (DirtyBuf[SYMB1_ADR_2500-0x7000] || DirtyBuf[SYMB2_ADR_2500-0x7000])
	{
        for (int ii=0;ii<6;ii++)
            disp_one_symb( pc2500_pos[ii].symb,		COLOR((pPC->Get_8(pc2500_pos[ii].addr)) & pc2500_pos[ii].bit),	pc2500_pos[ii].x,	pc2500_pos[ii].y);

        DirtyBuf[SYMB1_ADR_2500-0x7000] = 0;

        Refresh = true;
	}
	
	Clcdc::disp_symb();

}


void Clcdc_pc2500::disp(void)
{
	BYTE co,li,ind,b,data,x,y;
	WORD adr;

    Refresh = false;

	disp_symb();

	QPainter painter(pPC->LcdImage); 
	
	for (co=0; co<5; co++)
	{	for (li=0; li<4; li++)
		{	for (ind=0; ind<30; ind++)
			{	
				adr = ind + (0x0200 * co);
				switch (li)
				{
					case 0 : adr += 0x7000; break;
					case 1 : adr += 0x7040; break;
					case 2 : adr += 0x701E; break;
					case 3 : adr += 0x705E; break;
				}
				if (DirtyBuf[adr-0x7000])
				{
                    Refresh = true;
					x = ind + (co * 30);
					y = 8 * li;

					data = ( On ? (BYTE) pPC->Get_8(adr) : 0 );

					for (b=0; b<8;b++)
					{
						painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
						painter.drawPoint( x, y+b);
					}
					DirtyBuf[adr-0x7000]=0;				
				}
			}
		}
	}
	redraw = 0;
	painter.end();
}

///////////////////////////////////////////////////////////////////////
//
//  PC 1360
//
///////////////////////////////////////////////////////////////////////

static const struct {
	int x,y;
} pc1360_pos[7]={
    {8, 3},		// RUN
	{4, 11},	// PRO
	{6, 21},	// JAP
	{4, 29},	// SML
	{1, 39},	// SHIFT
    {4, 47},	// DEF
    {1, 3}      // PRINT
};


#define SYMB1_ADR_1360	0x303C
#define SYMB1_1360		(pPC->Get_8(SYMB1_ADR_1360))

void Clcdc_pc1360::disp_symb(void)
{

	if (DirtyBuf[SYMB1_ADR_1360-0x2800] )
	{

    disp_one_symb(S_RUN,		COLOR(SYMB1_1360&0x10),	pc1360_pos[0].x,	pc1360_pos[0].y);
    disp_one_symb(S_PRO,		COLOR(SYMB1_1360&0x20),	pc1360_pos[1].x,	pc1360_pos[1].y);
    disp_one_symb(S_JAP,		COLOR(SYMB1_1360&0x40),	pc1360_pos[2].x,	pc1360_pos[2].y);
    disp_one_symb(S_SML,		COLOR(SYMB1_1360&0x80),	pc1360_pos[3].x,	pc1360_pos[3].y);
    disp_one_symb(S_SHIFT,	COLOR(SYMB1_1360&0x01),	pc1360_pos[4].x,	pc1360_pos[4].y);
    disp_one_symb(S_DEF,		COLOR(SYMB1_1360&0x02),	pc1360_pos[5].x,	pc1360_pos[5].y);
    disp_one_symb(S_PRINT,	COLOR(SYMB1_1360&0x04),	pc1360_pos[6].x,	pc1360_pos[6].y);
	
    DirtyBuf[SYMB1_ADR_1360-0x2800] = false;

    Refresh = true;
	}
	
	Clcdc::disp_symb();
}


void Clcdc_pc1360::disp(void)
{
	
	BYTE co,li,ind,b,data,x,y;
	WORD adr;

    Refresh = false;
	
	disp_symb();
	
	QPainter painter(pPC->LcdImage);

	for (co=0; co<5; co++)
	{	for (li=0; li<4; li++)
		{	for (ind=0; ind<30; ind++)
			{	
				adr = 0x2800 + ind + (0x0200 * co);
				switch (li)
				{
					case 0 : break;
					case 1 : adr += 0x0040; break;
					case 2 : adr += 0x001E; break;
					case 3 : adr += 0x005E; break;
				}
				if (DirtyBuf[adr-0x2800])
				{
                    Refresh = true;
					x = ind + (co * 30);
					y = 8 * li;
					data = ( On ? (BYTE) pPC->Get_8(adr) : 0 );
			
					for (b=0; b<8;b++)
					{
						painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
						painter.drawPoint( x, y+b);
					}
					DirtyBuf[adr-0x2800]=0;				
				}
			}
		}
	}
	redraw = 0;
	painter.end();
}
///////////////////////////////////////////////////////////////////////
//
//  PC 1250
//
///////////////////////////////////////////////////////////////////////


static const struct {
	int x,y;
} pc1250_pos[11]={
    {250,0},	// SHIFT
    {48, 0},	// DEF
    {72, 0},	// PRO
    {100, 0},	// RUN
    {118, 0},	// RESERVE
    {158,0},	// DE
    {166,0},	// G
    {171,0},	// RAD
    {290,0},	// E
    {30,0},		// P
	{0,0}		// BUSY
};


#define SYMB1_ADR_1250	0xF83C
#define SYMB2_ADR_1250	0xF83D
#define SYMB3_ADR_1250	0xF83E
#define SYMB1_1250		(pPC->Get_8(SYMB1_ADR_1250))
#define SYMB2_1250		(pPC->Get_8(SYMB2_ADR_1250))
#define SYMB3_1250		(pPC->Get_8(SYMB3_ADR_1250))

void Clcdc_pc1250::disp_symb(void)
{

	if ( (DirtyBuf[SYMB1_ADR_1250-0xF800]) ||
		 (DirtyBuf[SYMB2_ADR_1250-0xF800]) ||
		 (DirtyBuf[SYMB3_ADR_1250-0xF800])
		)
    {

        disp_one_symb(S_SHIFT,	COLOR(SYMB2_1250&0x02),	pc1250_pos[0].x,	pc1250_pos[0].y);
        disp_one_symb(S_DEF,		COLOR(SYMB1_1250&0x01),	pc1250_pos[1].x,	pc1250_pos[1].y);
        disp_one_symb(S_PRO,		COLOR(SYMB3_1250&0x01),	pc1250_pos[2].x,	pc1250_pos[2].y);
        disp_one_symb(S_RUN,		COLOR(SYMB3_1250&0x02),	pc1250_pos[3].x,	pc1250_pos[3].y);
        disp_one_symb(S_RESERVE,	COLOR(SYMB3_1250&0x04),	pc1250_pos[4].x,	pc1250_pos[4].y);
        disp_one_symb(S_DE,		COLOR(SYMB1_1250&0x08),	pc1250_pos[5].x,	pc1250_pos[5].y);
        disp_one_symb(S_G,		COLOR(SYMB1_1250&0x04),	pc1250_pos[6].x,	pc1250_pos[6].y);
        disp_one_symb(S_RAD,		COLOR(SYMB2_1250&0x04),	pc1250_pos[7].x,	pc1250_pos[7].y);
        disp_one_symb(S_E,		COLOR(SYMB2_1250&0x08),	pc1250_pos[8].x,	pc1250_pos[8].y);
        disp_one_symb(S_PRINT,	COLOR(SYMB1_1250&0x02),	pc1250_pos[9].x,	pc1250_pos[9].y);
        disp_one_symb(S_BUSY,	COLOR(SYMB2_1250&0x01),	pc1250_pos[10].x,	pc1250_pos[10].y);

        DirtyBuf[SYMB1_ADR_1250-0xF800] = false;
        DirtyBuf[SYMB2_ADR_1250-0xF800] = false;
        DirtyBuf[SYMB3_ADR_1250-0xF800] = false;

        Refresh = true;
    }
	
	Clcdc::disp_symb();

}

void Clcdc_pc1250::disp(void)
{
    BYTE b,data,x,y;
    int ind;
    WORD adr;

    Refresh = false;

    disp_symb();

    QPainter painter(pPC->LcdImage);

    //	hr = g_pDDSTwo->GetDC(&hdc);

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
                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
                painter.drawPoint( x, y+b);
            }
            DirtyBuf[adr-0xF800] = 0;
        }
    }

    for (ind=0x3B; ind>=0; ind--)
    {	adr = 0xF800 + 0x40 + ind;
        if (DirtyBuf[adr-0xF800])
        {
            Refresh = true;
            data = ( On ? (BYTE) pPC->Get_8(adr) : 0);

            x = 142 - ind - (ind/5);			// +2 every 5 cols
            y = 0;

            for (b=0; b<7;b++)
            {
                painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
                painter.drawPoint( x, y+b);
            }
            DirtyBuf[adr-0xF800] = 0;
        }
    }

    redraw = 0;

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
	
	QPainter painter(pPC->LcdImage);

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
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, y+b);
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
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, y+b);
			}
			DirtyBuf[adr-0xF800] = 0;				
		}
	}
	redraw = 0;
	painter.end();
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
	
	Clcdc::disp_symb();

}

	
void Clcdc_pc1260::disp(void)
{
	BYTE b,data;
    int x,y;
	int ind;
	WORD adr;

    Refresh = false;

	disp_symb();

	QPainter painter(pPC->LcdImage);
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
                    painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
                    painter.setBrush(((data>>b)&0x01) ? Color_On : Color_Off);
                    //painter.drawPoint( x, y+b);
                    painter.drawRect(x*(PIXEL_SIZE+PIXEL_GAP) + (ind/5)*(PIXEL_SIZE+PIXEL_GAP)*3/2,
                                     (y+b)*(PIXEL_SIZE+PIXEL_GAP),
                                     PIXEL_SIZE-1,
                                     PIXEL_SIZE-1);
                }
                DirtyBuf[adr-0x2000] = 0;
            }
        }
    }


	redraw = 0;
	painter.end();

}

Clcdc_pc1260::Clcdc_pc1260(CPObject *parent)	: Clcdc_pc1250(parent){						//[constructor]

    Color_Off = QColor(0,0,0,0);

}


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

Clcdc_pc1401::Clcdc_pc1401(CPObject *parent)	: Clcdc(parent){						//[constructor]
    Color_Off.setRgb(
                (int) (0x61*contrast),
                (int) (0x6D*contrast),
                (int) (0x61*contrast));

}

void Clcdc_pc1401::disp(void)
{

    BYTE b,data,x,y;
    int ind;
    WORD adr;

    Refresh = false;

    disp_symb();

    QPainter painter(pPC->LcdImage);

    for (ind=0; ind<0x28; ind++)
    {
        adr = 0x6000 + ind;
		if ( DirtyBuf[adr-0x6000] )
		{	
            Refresh = true;
			data = ( On ? (BYTE) pPC->Get_8(adr) : 0 );
			
			x =ind + (ind/5);			// +1 every 5 cols
			y = 0;
			
			for (b=0; b<7;b++)
			{
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x,	y+b	);
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
			
			for (b=0; b<7;b++)
			{
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x,	y+b	);
			}
			DirtyBuf[adr-0x6000] = 0;				
		}
	}


	redraw = 0;
	painter.end();

}


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

	Clcdc::disp_symb();

}

void Clcdc_pc1403::disp(void)
{

	BYTE b,data,x,y;
	int ind;
	WORD adr;

    Refresh = false;

	disp_symb();

	QPainter painter(pPC->LcdImage);

#if 1
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
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, y+b);
			}
			DirtyBuf[adr-0x3000] = 0;				
		}
	}
#endif
#if 1
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
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, y+b);
			}
			DirtyBuf[adr-0x3000] = 0;				
		}
	}
#endif
#if 1
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
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, y+b);
			}
			DirtyBuf[adr-0x3000] = 0;				
		}
	}
#endif

#if 1
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
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, y+b);
			}
			DirtyBuf[adr-0x3000] = 0;				
		}
	}
#endif
#if 1
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
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, y+b);
			}
			DirtyBuf[adr-0x3000] = 0;				
		}
	}
#endif

#if 1
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
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, y+b);
			}
			DirtyBuf[adr-0x3000] = 0;				
		}
	}
#endif


	redraw = 0;
	painter.end();
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

    Refresh = false;
	disp_symb();
	QPainter painter(pPC->LcdImage);

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
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, y+b);
			}
			DirtyBuf[adr-0x7000] = 0;				
		}
	}

	redraw = 0;
	painter.end();
}


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

	Clcdc::disp_symb();
}

bool	Clcdc_pc1475::init(void)
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

void Clcdc_pc1475::disp(void)
{
	BYTE b,data;
	int x,y;
	int ind;
	WORD adr;

    if (!redraw) return;
    redraw = false;

    Refresh = false;
	disp_symb();
	QPainter painter(pPC->LcdImage);
	
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
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, y+2*b);
				painter.drawPoint( x, y+2*b+1);
				painter.drawPoint( x+1, y+2*b);
				painter.drawPoint( x+1, y+2*b+1);
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
			y = 16;
	
			for (b=0; b<7;b++)
			{
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, y+2*b);
				painter.drawPoint( x, y+2*b+1);
				painter.drawPoint( x+1, y+2*b);
				painter.drawPoint( x+1, y+2*b+1);
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
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, y+2*b);
				painter.drawPoint( x, y+2*b+1);
				painter.drawPoint( x+1, y+2*b);
				painter.drawPoint( x+1, y+2*b+1);
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
			y = 16;
	
			for (b=0; b<7;b++)
			{
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, y+2*b);
				painter.drawPoint( x, y+2*b+1);
				painter.drawPoint( x+1, y+2*b);
				painter.drawPoint( x+1, y+2*b+1);
			}
			DirtyBuf[adr-0x2000] = 0;				
		}
	}
#endif
	redraw = 0;
	painter.end();
}


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



void Clcdc_pc1500::disp(void)
{
	
	BYTE b,data,x;
	int ind;
	WORD adr;

    Refresh = false;

	disp_symb();
	
	QPainter painter(pPC->LcdImage);


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
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, b);
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
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, b);
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
					painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
					painter.drawPoint( x, b);
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
				painter.setPen( ((data>>b)&0x01) ? Color_On : Color_Off );
				painter.drawPoint( x, b);
			}

		}
	}
	
	redraw = 0;
	painter.end();
}


