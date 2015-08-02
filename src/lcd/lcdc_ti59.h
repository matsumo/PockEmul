#ifndef LCDC_TI59_H
#define LCDC_TI59_H

#include <QTimer>

#include "Lcdc.h"
#include "Lcdc_ti57.h"

class Clcdc_ti59:public Clcdc{
Q_OBJECT
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_ti59");}

    QTimer *blinkTimer;
    bool blinkState;

    Clcdc_ti59(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());
    virtual ~Clcdc_ti59()
    {						//[constructor]
    }

public slots:
    void blink();
};

#endif // LCDC_TI59_H

