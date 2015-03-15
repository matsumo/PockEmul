#ifndef LCDC_JR800
#define LCDC_JR800

#include "Lcdc.h"

class Clcdc_jr800:public Clcdc{
public:
    void disp(void);
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_jr800");}

    Clcdc_jr800(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_jr800()
    {						//[constructor]
    }

};

#endif // LCDC_JR800

