#ifndef LCDC_PC1450_H
#define LCDC_PC1450_H

#include "Lcdc.h"



class Clcdc_pc1450:public Clcdc{
public:
    bool init(void);
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_pc1450");}


    int x2a[100];

    Clcdc_pc1450(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_pc1450()
    {						//[constructor]
    }

};


#endif // LCDC_PC1450_H
