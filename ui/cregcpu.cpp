#include "cregcpu.h"

class CPObject;

CregCPU::CregCPU(CPObject *parent,CCPU *pCPU)
{
    this->pCPU = pCPU;
    this->setParent((QWidget*)parent);
}
