#ifndef LCDC_TI57_H
#define LCDC_TI57_H

#include <QTimer>

#include "Lcdc.h"

class Clcd7
{
// Construction
public:
    Clcd7();
    int  gap, width;
    int  num, oldnum;
    int  low, high;
    int  firstpaint;

//	CPoint oldpoint;

    bool seg1, seg2, seg3, seg4, seg5, seg6, seg7;

// Attributes
public:

// Operations
public:

// Implementation
public:
    virtual ~Clcd7();

    static void draw(UINT8 code, QPainter *painter, QRect rect, int width, int gap);
};


class Clcdc_ti57:public Clcdc{
Q_OBJECT
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_ti57");}

    QTimer *blinkTimer;
    bool blinkState;

    Clcdc_ti57(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_ti57()
    {						//[constructor]
    }

public slots:
    void blink();
};

#endif // LCDC_TI57_H
