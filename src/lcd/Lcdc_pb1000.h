#ifndef LCDC_PB1000_H
#define LCDC_PB1000_H


#include "Lcdc.h"
#include "hd44352.h"
//#include "pb1000.h"

class Clcdc_pb1000:public Clcdc{
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_pb1000");}

//    int computeSL(CHD61102* pCtrl,int ord);
    int symbSL(int x);
    int x2a[100];


    Clcdc_pb1000(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_pb1000()
    {						//[constructor]
    }

    HD44352info info;
};

#endif // LCDC_PB1000_H
