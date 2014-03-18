#ifndef PC1421_H
#define PC1421_H


#include "pc1401.h"

class Cpc1421:public Cpc1401{
    Q_OBJECT
public:
    const char*	GetClassName(){ return("Cpc1421");}

    Cpc1421(CPObject *parent = 0);

private:
};


#endif // PC1421_H
