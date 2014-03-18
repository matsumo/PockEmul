#ifndef CTRONICS_H
#define CTRONICS_H

#include "pobject.h"

class Cctronics : public CPObject {
    Q_OBJECT

    public:
        enum SIGNAME {
            S_BUSY,S_INIT,S_DATA,S_ACK,S_STROBE,S_ERROR
        };

        enum MODE {
            STROBE_UP,
            WAIT_FOR_ACK_UP,
            WAIT_FOR_ACK_DOWN,
            READY_TO_SEND
        };

        MODE mode;
        const char*	GetClassName(){ return("Cctronics");}

        virtual bool run(void);

        virtual bool init(void);				//initialize
        virtual bool exit(void);				//end

        virtual void Set_BUSY(bool v) { BUSY = v; }
        virtual void Set_ACK(bool v) { ACK = v; }

        virtual void Set_ERROR(bool v) { sERROR = v; }

        void setBufferSize(int s);

        bool isAvailable(void);

        bool Get_BUSY(void) {return BUSY; }
        bool Get_ACK(void) { return ACK; }
        quint8 Get_DATA(void) { return DATA; }
        bool Get_STROBE(void) { return STROBE; }
        bool Get_INIT(void) { return INIT; }

        void	clearOutput(void);

        void newOutChar(quint8 c);

        Cctronics(CPObject *parent = 0);

        virtual ~Cctronics(){
        }

    private:
        bool	STROBE,INIT,ACK,BUSY,sERROR;
        quint8  DATA;

        quint64  stateStrobeDelay,stateAckDelay;
        QByteArray outputBuffer;
        int bufferSize;
};

#endif // CTRONICS_H
