#ifndef LCDC_FX8000G_H
#define LCDC_FX8000G_H


#include "Lcdc.h"
#include "hd44352.h"
//#include "fx8000g.h"

class Clcdc_fx8000g:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_fx8000g");}

    virtual bool	init(void);
    virtual void	TurnOFF(void);			// LCD Off


    Clcdc_fx8000g(CPObject *parent = 0);
    virtual ~Clcdc_fx8000g()
    {						//[constructor]
    }

    HD44352info info;
};

#endif // LCDC_FX8000G_H
