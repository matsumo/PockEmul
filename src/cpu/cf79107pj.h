#ifndef CF79107PJ_H
#define CF79107PJ_H

#include <QObject>
#include "common.h"
class CpcXXXX;
class QXmlStreamReader;
class QXmlStreamWriter;



class CCF79107PJ:public QObject{

public:

    const char*	GetClassName(){ return("CCF79107PJ");}
    CpcXXXX		*pPC;

    bool	init(void);						//initialize
    bool	exit(void);						//end
    void	Reset(void);
    bool	step(void);
    bool    instruction1(UINT8 cmd);
    bool    instruction2(UINT8 cmd);
    UINT8    get_status(void);

    void	Load_Internal(QXmlStreamReader *);
    void	save_internal(QXmlStreamWriter *);

    CCF79107PJ(CpcXXXX *parent);
    virtual ~CCF79107PJ();

    UINT16 make_bcd_sub(UINT8 arg1, UINT8 arg2);
    UINT16 make_bcd_add(UINT8 arg1, UINT8 arg2);
    void cmd_add_mantisse(void);
    void dumpXYW();
    void cmd_add_exp(void);
    void cmd_inc_exp(void);
    void cmd_sub_exp(void);
    void cmd_dec_exp(void);
    void cmd_0e();
    void cmd_sub_mantisseXY(void);
    void cmd_shiftL_mantisse(void);
    void cmd_shiftR_mantisse(void);
    void Read_Reg(UINT16 adr);
    void Write_Reg(UINT16 adr);
    void Exch_Reg(UINT16 adr);
    void cmd_sub_mantisseYX(void);
    void push(UINT16 adr);
    void pop(UINT16 adr);
private:


    UINT8 BCDret;
    bool BCDz,BCDc,BCDaf;
    UINT8 reg[0x10];

};


#endif // CF79107PJ_H


