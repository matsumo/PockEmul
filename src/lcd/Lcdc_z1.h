#ifndef LCDC_Z1_H
#define LCDC_Z1_H

#include "Lcdc.h"

class Clcdc_z1:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen
    const char*	GetClassName(){ return("Clcdc_z1");}

    Clcdc_z1(CPObject *parent = 0);
    virtual ~Clcdc_z1()
    {						//[constructor]
    }


};
#endif // LCDC_Z1_H
