#ifndef CE1600P_H
#define CE1600P_H

#include "ce150.h"



#define NO_MOVE	0
#define RI_MOVE	1
#define LE_MOVE 2
#define PEN_UP	0
#define PEN_DOWN 1

class Cprinter;
class Cmotor;
class Ccpu;

class CFDD:public QObject{
    Q_OBJECT
public:
    CFDD(QObject *parent);
    ~CFDD();


    void fddCmd(BYTE _data);
    void fddSetSector(BYTE _data);
    BYTE fddGetSector();
    void fddSetMotor(BYTE );
    void fddWriteData(BYTE _data);
    BYTE fddReadData();
    WORD fddStatus();

    bool error;
    bool ready;
    bool ChangedDisk;
    bool busy;

    bool isDisk;
    bool writeProtect;
    bool motorRunning;

    void fddwrite(BYTE _offset, BYTE _data);
    BYTE fddRead(BYTE _offset);
    void fddReset();

    void step();

    quint64 startMotorState;

    int countWrite;

    BYTE data[128][512];

    BYTE sector;
    BYTE offset;
};

class Cce1600p:public Cce150{
Q_OBJECT

public:
    bool    init(void);
    bool	run(void);
    void    Print(void);

     Cmotor		Motor_Z;

    Cce1600p(CPObject *parent = 0);

private:
    BYTE motorX,motorY,motorZ;
    bool ccKeyInt , pfKeyInt , revpfKeyInt , fdKeyInt , swKeyInt , crKeyInt;

    CFDD fdd;

};


#endif // CE1600P_H
