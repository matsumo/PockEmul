#ifndef LCDC_HX20_H
#define LCDC_HX20_H

#include "Lcdc.h"

class Clcdc_hx20:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_hx20");}

    Clcdc_hx20(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_hx20()
    {						//[constructor]
    }

};

#endif // LCDC_HX20_H

