#ifndef CLCDC_X07_H
#define CLCDC_X07_H

#include "Lcdc.h"

class Clcdc_x07:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void){}
    const char*	GetClassName(){ return("Clcdc_x07");}

    Clcdc_x07(CPObject *parent = 0);
    virtual ~Clcdc_x07()
    {						//[constructor]
    }


    QList<QImage> imac;

};

#endif // CLCDC_X07_H
