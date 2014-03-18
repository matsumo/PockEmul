#ifndef LCDC_RLH1000_H
#define LCDC_RLH1000_H

#include "Lcdc.h"

class Clcdc_rlh1000:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_rlh1000");}

    Clcdc_rlh1000(CPObject *parent = 0)	: Clcdc(parent){						//[constructor]
        Color_Off.setRgb(
                            (int) (111*contrast),
                            (int) (117*contrast),
                            (int) (108*contrast));
        memset((void *)mem,0,sizeof(mem));
    }
    virtual ~Clcdc_rlh1000()
    {						//[constructor]
    }

    quint8 mem[0xA0];   // Lcd memory

};
#endif // LCDC_RLH1000_H
