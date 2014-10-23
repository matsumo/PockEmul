#ifndef LCDC_PC1475_H
#define LCDC_PC1475_H

#include "Lcdc.h"

class Clcdc_pc1475:public Clcdc{
public:
    bool init(void);
    void disp(void);				//display LCDC data to screen

    const char*	GetClassName(){ return("Clcdc_pc1475");}


    int x2a[100];

    Clcdc_pc1475(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_pc1475()
    {						//[constructor]
    }

};

#endif // LCDC_PC1475_H
