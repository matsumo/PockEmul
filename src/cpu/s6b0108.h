#ifndef S6B0108_H
#define S6B0108_H

#include <QFile>


// Display Driver
#include "hd61102.h"


class CS6B0108:public CHD61102{

public:
    const char*	GetClassName(){ return("CS6B0108");}

    CS6B0108(CPObject *parent);
    virtual ~CS6B0108();

    virtual void    cmd_write(qint16 cmd);
    virtual BYTE    cmd_read(qint16 cmd);

};



#endif // S6B0108_H
