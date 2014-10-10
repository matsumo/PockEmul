#ifndef LCDC_PC1350_H
#define LCDC_PC1350_H

#include "Lcdc.h"

class Clcdc_pc1350:public Clcdc
{
public:
    void disp(void);				//display LCDC data to screen

    Clcdc_pc1350(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_pc1350()
    {						//[constructor]
    }

};

class Clcdc_pc1360:public Clcdc_pc1350{
public:

    Clcdc_pc1360(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_pc1360()
    {						//[constructor]
    }

};

class Clcdc_pc2500:public Clcdc_pc1350
{
public:
Clcdc_pc2500(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_pc2500()
    {						//[constructor]
    }
};
#endif // LCDC_PC1350_H
