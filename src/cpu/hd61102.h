#ifndef HD61102_H
#define HD61102_H

#include <QFile>


// Display Driver
#include "pobject.h"

typedef struct {
    BYTE    on_off;
    BYTE    displaySL;
    BYTE    Xadr;
    BYTE    Yadr;
    BYTE    status;
    BYTE    imem[0x200];
} HD61102info;

class CHD61102:public QObject{



public:
    const char*	GetClassName(){ return("CHD61102");}
    CPObject		*pPC;

    bool	init(void);						//initialize
    bool	exit(void);						//end
    void	Reset(void);
    bool	step(void);
    BYTE    instruction(qint16 cmd);

    void    cmd_on_off(qint16 cmd);
    void    cmd_displaySL(qint16 cmd);
    void    cmd_setX(qint16 cmd);
    void    cmd_setY(qint16 cmd);
    BYTE    cmd_status(qint16 cmd);
    virtual void    cmd_write(qint16 cmd);
    virtual BYTE    cmd_read(qint16 cmd);

    BYTE    get8(qint16 adr);
    void    set8(qint16 adr,BYTE val);

    HD61102info info;
    bool    updated;

    void	Load_Internal(QFile *);
    void	save_internal(QFile *);
    void	Load_Internal(QXmlStreamReader *);
    void	save_internal(QXmlStreamWriter *);

    CHD61102(CPObject *parent);
    virtual ~CHD61102();

    void	addretrace (void);

protected:
    int outputRegister;

private:
    quint64 last_state_cmd;
};

#endif // HD61102_H
