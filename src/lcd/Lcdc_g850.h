#ifndef LCDC_G850_H
#define LCDC_G850_H


#include "Lcdc.h"

class Clcdc_g850:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_g850");}

    int computeSL(int ord);
    int symbSL(int x);

    Clcdc_g850(CPObject *parent = 0);
    virtual ~Clcdc_g850()
    {						//[constructor]
    }

};


#endif // LCDC_G850_H
