#ifndef CE162E_H
#define CE162E_H


#include "Log.h"
#include "lh5810.h"
#include "cprinter.h"
#include "Connect.h"
#include "Inter.h"
#include "Keyb.h"
#include "cmotor.h"

#include "s6b0108.h"

class CbusPc1500;
class CLH5810;

class Cce162e:public CPObject{
public:

    virtual bool		run(void);
    virtual bool		init(void);
    virtual bool		exit(void);

    Cconnector	*pCONNECTOR;	qint64 pCONNECTOR_value;
    CbusPc1500    *bus;
    CLH5810		*pLH5810_1;
    CLH5810		*pLH5810_2;

    bool lh5810_1_write(void);
    bool lh5810_1_read(void);
    bool lh5810_2_write(void);
    bool lh5810_2_read(void);

    Cce162e(CPObject *parent = 0);
    virtual ~Cce162e();

};



#endif // CE162E_H
