#ifndef CE122_H
#define CE122_H

#include "Ce126.h"

class Cce122:public Cce126
{
public:
    Cce122(CPObject *parent=0);
    bool run();
    void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0,QMouseEvent *event=0);

    bool UpdateFinalImage();
    virtual void TurnON();
    virtual void TurnOFF();

    bool printSwitch;
    bool init();
};


#endif // CE122_H
