#include "common.h"
#include "cmotor.h"
#include "Log.h"


Cmotor::Cmotor(void) {
    LastStepDirection = NO_MOVE;
    Step = 0;
    this->CurrentPhase = 0;

    for (int x = 0; x<0x10; x++) {
        for (int y = 0; y <0x10; y++)
            phaseList[x][y] = NO_MOVE;
    }

    phaseList[9][3] = LE_MOVE;
    phaseList[9][1] = LE_MOVE_MID;
    phaseList[9][8] = RI_MOVE_MID;
    phaseList[9][0x0c] = RI_MOVE;

    phaseList[8][1] = LE_MOVE;
    phaseList[8][9] = LE_MOVE_MID;
    phaseList[8][0x0c] = RI_MOVE_MID;
    phaseList[8][4] = RI_MOVE;

    phaseList[0x0c][9] = LE_MOVE;
    phaseList[0x0c][8] = LE_MOVE_MID;
    phaseList[0x0c][4] = RI_MOVE_MID;
    phaseList[0x0c][6] = RI_MOVE;

    phaseList[4][8] = LE_MOVE;
    phaseList[4][0x0c] = LE_MOVE_MID;
    phaseList[4][6] = RI_MOVE_MID;
    phaseList[4][2] = RI_MOVE;

    phaseList[6][0x0c] = LE_MOVE;
    phaseList[6][4] = LE_MOVE_MID;
    phaseList[6][2] = RI_MOVE_MID;
    phaseList[6][3] = RI_MOVE;

    phaseList[2][4] = LE_MOVE;
    phaseList[2][6] = LE_MOVE_MID;
    phaseList[2][3] = RI_MOVE_MID;
    phaseList[2][1] = RI_MOVE;

    phaseList[3][6] = LE_MOVE;
    phaseList[3][2] = LE_MOVE_MID;
    phaseList[3][1] = RI_MOVE_MID;
    phaseList[3][9] = RI_MOVE;

    phaseList[1][2] = LE_MOVE;
    phaseList[1][3] = LE_MOVE_MID;
    phaseList[1][9] = RI_MOVE_MID;
    phaseList[1][8] = RI_MOVE;

}

BYTE Cmotor::SendPhase(BYTE Phase)		// Next step, return direction: 0 no move, 1 Right,2 left
{
    LastStepDirection=NO_MOVE;
    if ((Phase==0)||(Phase == this->CurrentPhase)) return(LastStepDirection);
    LastStepDirection = phaseList[this->CurrentPhase][Phase];
    this->CurrentPhase = Phase;
    return (LastStepDirection);

}

