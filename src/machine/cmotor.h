#ifndef CMOTOR_H
#define CMOTOR_H

#include <QObject>

#include "common.h"

#define NO_MOVE	0
#define RI_MOVE	1
#define LE_MOVE 2
#define RI_MOVE_MID	3
#define LE_MOVE_MID 4

class Cmotor:public QObject{
public:
    BYTE	SendPhase(BYTE Phase);		// Next step, return direction: 0 no move, 1 Right,2 left
    long	Step;

    int		LastStepDirection;		// TRUE= Right   FALSE=Left
    Cmotor(void);

private:
    BYTE    phaseList[0x10][0x10];
    BYTE	CurrentPhase;
};


#endif // CMOTOR_H
