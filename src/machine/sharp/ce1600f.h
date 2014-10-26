#ifndef CE1600F_H
#define CE1600F_H

#include "pobject.h"

class CbusPc1500;

class Cce1600f:public CPObject{
    Q_OBJECT
public:
    Cce1600f(CPObject *parent=0);
    ~Cce1600f();

    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    virtual	bool run(void);					//check access

    Cconnector	*pCONNECTOR;		qint64 pCONNECTOR_value;

    CbusPc1500  *bus;

    void fddCmd(BYTE _data);
    void fddSetSector(BYTE _data);
    BYTE fddGetSector();
    void fddSetMotor(BYTE );
    void fddWriteData(BYTE _data);
    BYTE fddReadData();
    WORD fddStatus();

    BYTE currentCMD;
    BYTE checksum;
    bool error;
    bool ready;
    bool ChangedDisk;
    bool busy;
    bool ack;

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

#endif // CE1600F_H
