#ifndef LCDC_E500_H
#define LCDC_E500_H


#include "Lcdc.h"
#include "hd61102.h"

class Clcdc_e500:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_e500");}

    int computeSL(CHD61102* pCtrl,int ord);
    int symbSL(int x);
    int x2a[100];


    Clcdc_e500(CPObject *parent = 0);
    virtual ~Clcdc_e500()
    {						//[constructor]
    }

};



#endif // LCDC_E500_H
