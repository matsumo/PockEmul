#include "Lcdc_x07.h"
#include "cx07.h"
#include "ct6834.h"


Clcdc_x07::Clcdc_x07(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]
//    Color_Off.setRgb(
//                        (int) (95*contrast),
//                        (int) (107*contrast),
//                        (int) (97*contrast));
}

void Clcdc_x07::disp()
{

//    if (((Cx07 *)pPC)->pT6834)
//        ((Cx07 *)pPC)->pT6834->RefreshVideo();
    redraw = false;
}

