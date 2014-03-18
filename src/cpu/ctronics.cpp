#include <QDebug>

#include "ctronics.h"
#include "pcxxxx.h"
#include "Inter.h"


#define STROBE_DELAY 5
#define DEFAULT_BUFFERSIZE 100

Cctronics::Cctronics(CPObject *parent):CPObject(parent)
{

}


bool Cctronics::init()
{
    CPObject::init();
    STROBE=INIT=ACK=BUSY=sERROR=false;
    DATA=0;
    mode = READY_TO_SEND;
    bufferSize = DEFAULT_BUFFERSIZE;
    pTIMER = pPC->pTIMER;
    stateStrobeDelay = pTIMER->state;
    return true;
}

bool Cctronics::exit()
{
    CPObject::exit();

    return true;
}

void Cctronics::setBufferSize(int s)
{
    bufferSize = s;
}

bool Cctronics::isAvailable()
{
    return (outputBuffer.size()< bufferSize);
}


bool Cctronics::run()
{
    if (!pTIMER) return true;

    switch (mode) {

    case STROBE_UP:
        if (pTIMER->usElapsed(stateStrobeDelay) > STROBE_DELAY) {
            STROBE = false;
            stateAckDelay = pTIMER->state;
            mode = WAIT_FOR_ACK_UP;
        }
        break;

    case WAIT_FOR_ACK_UP:
        if (ACK) {
            mode = WAIT_FOR_ACK_DOWN;
        }else if (pTIMER->msElapsed(stateAckDelay)>100){
            // WAIT FOR ACK too long init ACK
            mode = READY_TO_SEND;
        }
        break;

    case WAIT_FOR_ACK_DOWN:
        if (!ACK)
            mode = READY_TO_SEND;
        break;

     case READY_TO_SEND:
        if (!BUSY && !outputBuffer.isEmpty()) {
            DATA = outputBuffer.at(0);
            outputBuffer.remove(0,1);
            STROBE = true;
            stateStrobeDelay = pTIMER->state;
            mode = STROBE_UP;
        }
        break;
    }

    return true;
}


void Cctronics::newOutChar(quint8 c)
{
    outputBuffer.append(c);
}


void Cctronics::clearOutput()
{
    outputBuffer.clear();
}
