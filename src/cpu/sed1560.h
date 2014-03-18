#ifndef SED1560_H
#define SED1560_H

#include "pobject.h"

#define IMEMSIZE    0x5D6

typedef struct {
    bool    busy;
    bool    ADC;
    bool    reset;
    bool    on_off;
    bool    ReadModifyWrite;
    BYTE    ColAdrReg;
    BYTE    PgAdrReg;
    BYTE    displaySL;
    BYTE    ElectCtrl;
    BYTE    imem[IMEMSIZE];
} SED1560info;

class CSED1560:public QObject{



public:
    const char*	GetClassName(){ return("CSED1560");}
    CpcXXXX		*pPC;

    bool	init(void);						//initialize
    bool	exit(void);						//end
    void	Reset(void);
    bool	step(void);
    BYTE    instruction(qint16 cmd);

    void    cmd_on_off(qint16 cmd);
    void    cmd_displaySL(qint16 cmd);
    BYTE    cmd_status(qint16 cmd);
    void    cmd_write(qint16 cmd);
    BYTE    cmd_read(qint16 cmd);

    BYTE    get8(qint16 adr);
    void    set8(qint16 adr,BYTE val);

    SED1560info info;
    bool    updated;

    void	Load_Internal(QXmlStreamReader *);
    void	save_internal(QXmlStreamWriter *);

    CSED1560(CpcXXXX *parent);
    virtual ~CSED1560();

    void	addretrace (void);




private:
    void cmd_ColAdrLo(qint16 cmd);
    void cmd_ColAdrHi(qint16 cmd);
    void cmd_setPgAdr(qint16 cmd);
    void cmd_Reset(qint16 cmd);
    void cmd_RevLineSet(qint16 cmd);
    void cmd_RevLineRst(qint16 cmd);
    void cmd_NormRevDsp(qint16 cmd);
    void cmd_AllIndic(qint16 cmd);
    void cmd_ADCSel(qint16 cmd);
    void cmd_BuildInPow(qint16 cmd);
    void cmd_OutStatusRegSet(qint16 cmd);
    void cmd_DutyPlus(qint16 cmd);
    void cmd_DutySel(qint16 cmd);
    void cmd_ElecCtrlReg(qint16 cmd);



};

#endif // SED1560_H
