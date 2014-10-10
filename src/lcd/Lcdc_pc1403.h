#ifndef LCDC_PC1403_H
#define LCDC_PC1403_H

#include "Lcdc.h"

class Clcdc_pc1403:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_pc1403");}



    Clcdc_pc1403(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_pc1403()
    {						//[constructor]
    }

};

class Clcdc_pc1425:public Clcdc_pc1403{
public:
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_pc1425");}

    Clcdc_pc1425(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_pc1425()
    {						//[constructor]
    }

};
#endif // LCDC_PC1403_H
