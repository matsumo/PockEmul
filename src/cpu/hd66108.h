#ifndef HD66108_H
#define HD66108_H

#include <QFile>

#define VRAM_HEIGHT	65
#define VRAM_WIDTH	32
#define LCD_HEIGHT	32
#define LCD_WIDTH	192

// Display Driver
#include "pobject.h"

typedef struct {
    BYTE    on_off;
    BYTE    displaySL;
    BYTE    Xadr;
    BYTE    Yadr;
    BYTE    status;
    BYTE    imem[0x200];
} HD66108info;

class CHD66108:public QObject{



public:
    const char*	GetClassName(){ return("CHD66108");}
    CpcXXXX		*pPC;

    bool	init(void);						//initialize
    bool	exit(void);						//end
    void	Reset(void);
    bool	step(void);


    HD66108info info;
    bool    updated;

    void	Load_Internal(QXmlStreamReader *);
    void	save_internal(QXmlStreamWriter *);

    CHD66108(CpcXXXX *parent);
    virtual ~CHD66108();

    void	addretrace (void);

    UINT8 readVram(int p);
    void writeVram(int p, UINT8 v);
    void pset(UINT8 *vram, int x, int y, int pix);
//private:
    UINT8   reg,fcr,xar,yar;
    UINT8 vram[(LCD_WIDTH / 8) * LCD_HEIGHT];
    UINT8 ram[VRAM_WIDTH][VRAM_HEIGHT];
};

#endif
