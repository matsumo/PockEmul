#ifndef LCDC_CC40_H
#define LCDC_CC40_H



#include "Lcdc.h"
#include "hd44780.h"


class Clcdc_cc40:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_cc40");}

    int symbSL(int x);
    int x2a[100];


    Clcdc_cc40(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_cc40()
    {						//[constructor]
    }

    HD44780info *info;
};



#endif // LCDC_CC40_H
