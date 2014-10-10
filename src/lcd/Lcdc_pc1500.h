#ifndef LCDC_PC1500_H
#define LCDC_PC1500_H

#include "Lcdc.h"


class Clcdc_pc1500:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_pc1500");}


    int x2a[100];


    Clcdc_pc1500(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_pc1500()
    {						//[constructor]
    }

};


#endif // LCDC_PC1500_H
