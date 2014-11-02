#ifndef _CCABLE_H_
#define _CCABLE_H_

#include "pobject.h"


class Cconnector;

class Ccable:public CPObject{
public:
    const char*	GetClassName(){ return("Ccable");}

		
    Cconnector	*pCONNECTOR_A;  qint64 pCONNECTOR_A_value;
    Cconnector	*pCONNECTOR_B;  qint64 pCONNECTOR_B_value;

	bool run(void);
	bool init(void);				//initialize
	bool exit(void);				//end
    bool Adapt(Cconnector *A,Cconnector *B);
    Ccable(CPObject *parent = 0);
 
    bool standard;
	
    virtual ~Ccable();

    bool UpdateFinalImage();
private:

};


#endif
