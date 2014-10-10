/*** PC-XXXX Emulator [LCDC.H] ****************************/
/* LCD Controller emulation class                         */
/**********************************************************/


#ifndef LCDC_H
#define LCDC_H

#include <QColor>

#include "common.h"

class CpcXXXX;
class CPObject;

class ClcdSymb {
public:
    ClcdSymb(int _x,int _y,const char* _symb,UINT32 _addr,int _bit);

    int x,y;
    const char *symb;
    UINT32	addr;
    int	bit;

};

class Clcdc:public QObject{
Q_OBJECT

public:
	CpcXXXX *pPC;
	
	bool	redraw;
	void	check(void);			// LCDC access check
	void	TurnON(void);			// LCD On
    virtual void	TurnOFF(void);			// LCD Off

	bool	On;
	bool	Refresh;
	float	contrast;
    bool    updated;
	bool	DirtyBuf[0x1000];
    bool    ready;
    QList<ClcdSymb> symbList;

	void	Update(void){
					for (int i=0 ; i<0x1000;i++) DirtyBuf[i]=true;
                    Refresh = true;
						}
									// Set the DirtyBuffer array to 1

	virtual bool	init(void);		// initialize
	virtual bool	exit(void);		// end
    virtual void	disp(void){}		//display LCDC data to screen
	virtual void	init_screen(void);		//initialize screen
    virtual void	exit_screen(void){}		//restore screen
    virtual void	disp_speed(void){}		//display CPU speed

	void disp_one_symb(const char *figure, QColor color, int x, int y);
    static void disp_one_symb(QPainter *painter, const char *figure, QColor color, int x, int y);
	virtual void disp_symb(void);
	void Contrast(int command);
    const char*	GetClassName(){ return("Clcdc");}
    void SetDirtyBuf(WORD);
	QColor	Color_On;
	QColor	Color_Off;

    QImage* LcdImage;
    QImage* SymbImage;
    float LcdRatio;

    void InitDisplay(void);

    Clcdc(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname=QString(), QString _symbfname=QString());

    virtual ~Clcdc();

    QRect rect;
    QRect symbRect;
    float	Lcd_ratio_X;
    float	Lcd_ratio_Y;

    float	Lcd_Symb_ratio_X;
    float	Lcd_Symb_ratio_Y;
    QString	LcdFname;
    QString	SymbFname;

    void drawPixel(QPainter *painter, float x, float y, QColor color);
protected:
    QColor origColor_Off;
    WORD baseAdr;

    BYTE pixelSize,pixelGap;

    QSize internalSize;



};

#define COLOR(b)	( ( (b)&&On ) ? Color_On : Color_Off)

#endif
