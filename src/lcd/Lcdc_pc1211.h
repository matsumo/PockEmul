#ifndef LCDC_PC1211_H
#define LCDC_PC1211_H

#include "Lcdc.h"

class Cpc1211;

class Clcdc_pc1211:public Clcdc{
public:

    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_pc1211");}

    Clcdc_pc1211(CPObject *parent = 0);
    virtual ~Clcdc_pc1211()
    {						//[constructor]
    }

    Cpc1211 * pPC1211;
    QByteArray line;
    void DrawChar(quint8 c,int x);

//    quint8 cursorPos;
    bool cursorBlink;
    quint64 blinkState;

};

#endif // LCDC_PC1211_H
