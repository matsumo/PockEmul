#ifndef HD44102
#define HD44102

#include "common.h"

typedef struct {
    UINT8 imem[4][50];				// display memory

    UINT8 m_status;					// status register
    UINT8 m_output;					// output register

    int m_cs2;						// chip select
    int m_page;						// display start page
    int m_x;						// X address
    int m_y;						// Y address

//    BYTE    on_off;
//    BYTE    displaySL;
//    BYTE    Xadr;
//    BYTE    Yadr;
//    BYTE    status;
//    BYTE    imem[0x200];
} HD44102info;

class CPObject;
class QXmlStreamReader;
class QXmlStreamWriter;

class CHD44102 :	public QObject
{
public:
    const char*	GetClassName(){ return("CHD44102");}
    CPObject		*pPC;

    bool	init(void);						//initialize
    bool	exit(void);						//end
    void	Reset(void);
    BYTE    cmd_status();
    virtual void    cmd_write(UINT8 data);

    BYTE    get8();
    void    set8(BYTE val);

    HD44102info info;
    bool    updated;

    void	Load_Internal(QXmlStreamReader *xmlIn);
    void	save_internal(QXmlStreamWriter *xmlOut);

    // construction/destruction
    CHD44102(CPObject *parent);

    inline void count_up_or_down();


};

#endif // HD44102

