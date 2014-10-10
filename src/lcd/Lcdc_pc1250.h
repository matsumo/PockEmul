#ifndef LCDC_PC1250_H
#define LCDC_PC1250_H

#include "Lcdc.h"



class Clcdc_pc1250:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen

    Clcdc_pc1250(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_pc1250()
    {						//[constructor]
    }

};

class Clcdc_pc1245:public Clcdc_pc1250{
public:
    void disp_symb(void);
    void disp(void);				//display LCDC data to screen
    const char*	GetClassName(){ return("Clcdc_pc1245");}

    Clcdc_pc1245(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_pc1245()
    {						//[constructor]
    }

};

class Clcdc_pc1260:public Clcdc_pc1250{
public:
    void disp_symb(void);
    void disp(void);				//display LCDC data to screen
    const char*	GetClassName(){ return("Clcdc_pc1260");}

    Clcdc_pc1260(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_pc1260()
    {						//[constructor]
    }

};

#endif // LCDC_PC1250_H
