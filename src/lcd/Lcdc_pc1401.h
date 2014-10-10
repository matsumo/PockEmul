#ifndef LCDC_PC1401_H
#define LCDC_PC1401_H

#include "Lcdc.h"

class Clcdc_pc1401:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_pc1401");}



    Clcdc_pc1401(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_pc1401()
    {						//[constructor]
    }

};

#endif // LCDC_PC1401_H
