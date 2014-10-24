#ifndef CE1600P_H
#define CE1600P_H

#include "ce150.h"



#define NO_MOVE	0
#define RI_MOVE	1
#define LE_MOVE 2
#define PEN_UP	0
#define PEN_DOWN 1

class Cprinter;
class Cmotor;
class Ccpu;


class Cce1600p:public Cce150{

public:
    bool    init(void);
    bool	run(void);
    void    Print(void);

     Cmotor		Motor_Z;

    Cce1600p(CPObject *parent = 0);

private:
    BYTE motorX,motorY,motorZ;
    bool ccKeyInt , pfKeyInt , revpfKeyInt , fdKeyInt , swKeyInt , crKeyInt;

};


#endif // CE1600P_H
