#ifndef LCDC_FP200_H
#define LCDC_FP200_H

#include <QtCore>
#include "Lcdc.h"


class Clcdc_fp200:public Clcdc{
public:
    bool init(void);
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_fp200");}

    Clcdc_fp200(CPObject *parent = 0);
    virtual ~Clcdc_fp200()
    {						//[constructor]
    }

    void Write(quint8 side,quint8 val);
    quint8 Read(quint8 side);
    //quint8 mem_video[160][8];
    quint8 mem_video[20][64];
    quint8 X,Y;

    quint8 Status;
    void AffCar(UINT8 x, UINT8 y, UINT8 Car);

    UINT8 charset[0x800];
    bool text;
    int displaySL[2];
    int computeSL(int side, int ord);
};

#endif // LCDC_FP200_H
