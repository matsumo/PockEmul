#ifndef LCDC_TI74_H
#define LCDC_TI74_H


#include "Lcdc.h"
#include "hd44780.h"


class Clcdc_ti74:public Clcdc{
public:
    virtual void disp(void);				//display LCDC data to screen
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

class Clcdc_ti95:public Clcdc_ti74 {
public:
    const char*	GetClassName(){ return("Clcdc_ti95");}

    virtual void disp(void);

    Clcdc_ti95(CPObject *parent = 0);
    virtual ~Clcdc_ti95()
    {						//[constructor]
    }
};

#endif // LCDC_TI74_H
