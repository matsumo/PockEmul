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
#include "viewobject.h"

Clcdc::Clcdc(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname)
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
    baseAdr = 0;
    pixelSize = 1;
    pixelGap = 0;
    LcdImage = 0;
    SymbImage = 0;

    LcdRatio = 1;

    LcdFname = _lcdfname;
    if (LcdFname.isEmpty()) LcdFname = ":/pockemul/transparent.png";
    rect = _lcdRect;
    Lcd_ratio_X	= 1;
    Lcd_ratio_Y	= 1;

    SymbFname = _symbfname;
    if (SymbFname.isEmpty()) SymbFname = ":/pockemul/transparent.png";
    symbRect = _symbRect;
    Lcd_Symb_ratio_X = Lcd_Symb_ratio_Y	= 1;

}

Clcdc::~Clcdc() {
    delete LcdImage;
    delete SymbImage;

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
    QPainter painter(SymbImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    Clcdc::disp_one_symb(&painter,figure,color,x,y);
	painter.end();
}


void Clcdc::disp_symb(void)
{
    for (int ii=0;ii<symbList.count();ii++) {
        if (DirtyBuf[symbList.at(ii).addr-baseAdr] )
        {
            disp_one_symb( symbList.at(ii).symb,
                           COLOR((pPC->Get_8(symbList.at(ii).addr)) & symbList.at(ii).bit),
                           symbList.at(ii).x,
                           symbList.at(ii).y);
        }
        DirtyBuf[symbList.at(ii).addr-baseAdr] = 0;

        Refresh = true;
    }
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
//	if (!(LcdImage))
//		LcdImage->load(pPC->LcdFname);
}

bool	Clcdc::exit(void) {	return(true); }

void Clcdc::SetDirtyBuf(WORD index)
{
	DirtyBuf[index]=true;
    Refresh=true;
}

void Clcdc::InitDisplay()
{
    delete LcdImage;
    LcdImage = CViewObject::CreateImage(rect.size() * LcdRatio,LcdFname,false,false,0);
    if (symbRect.isValid()) {
        delete SymbImage;
        SymbImage	= CViewObject::CreateImage(symbRect.size(),SymbFname);
    }

//    pLCDC->Color_Off.setRgb(LcdImage->pixel(0,0));
//    pLCDC->Color_Off.setAlphaF(1-pLCDC->contrast);

}




ClcdSymb::ClcdSymb(int _x, int _y, const char *_symb, UINT32 _addr, int _bit)
{
    x=_x;
    y=_y;
    symb = _symb;
    addr=_addr;
    bit=_bit;
}





