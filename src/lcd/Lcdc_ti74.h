#ifndef LCDC_TI74_H
#define LCDC_TI74_H


#include "Lcdc.h"
#include "hd44780.h"


class Clcdc_ti74:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_ti74");}

    int symbSL(int x);
    int x2a[100];


    Clcdc_ti74(CPObject *parent = 0);
    virtual ~Clcdc_ti74()
    {						//[constructor]
    }

    HD44780info *info;
};

#endif // LCDC_TI74_H
