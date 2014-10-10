#ifndef LCDC_TPC8300_H
#define LCDC_TPC8300_H

#include "Lcdc.h"

class Clcdc_tpc8300:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_tpc8300");}

    Clcdc_tpc8300(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_tpc8300()
    {						//[constructor]
    }

};

#endif // LCDC_TPC8300_H
