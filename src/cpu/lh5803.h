#ifndef LH5803_H
#define LH5803_H

#include "lh5801.h"

class CLH5803:public CLH5801{

public:
    CLH5803(CPObject *parent)	: CLH5801(parent)
    {
        fn_status="LH5803.sta";
    }

    virtual void	step(void);
};


#endif // LH5803_H
