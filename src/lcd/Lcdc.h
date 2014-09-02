/*** PC-XXXX Emulator [LCDC.H] ****************************/
/* LCD Controller emulation class                         */
/**********************************************************/


#ifndef LCDC_H
#define LCDC_H

#include <QColor>

#include "common.h"

class CpcXXXX;
class CPObject;

class Clcdc:public QObject{
Q_OBJECT

public:
	CpcXXXX *pPC;
	
	bool	redraw;
	void	check(void);			// LCDC access check
	void	TurnON(void);			// LCD On
	void	TurnOFF(void);			// LCD Off

	bool	On;
	bool	Refresh;
	float	contrast;
    bool    updated;
	bool	DirtyBuf[0x1000];
    bool    ready;

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

    Clcdc(CPObject *parent);

    virtual ~Clcdc();


protected:
	QColor origColor_Off;
};


//////////////////////////////////////////////////////
// LCDC emulation Windows///////////////////////////
//////////////////////////////////////////////////
class Clcdc_pc1350:public Clcdc
{
public:
	void disp(void);				//display LCDC data to screen
	void disp_symb(void);

	Clcdc_pc1350(CPObject *parent)	: Clcdc(parent)
	{						//[constructor]
		Color_Off.setRgb(	
                            (int) (101*contrast),
                            (int) (109*contrast),
                            (int) (94*contrast));
	};
	virtual ~Clcdc_pc1350()
	{						//[constructor]
	};

};

class Clcdc_pc1360:public Clcdc{
public:
	void disp(void);				//display LCDC data to screen
	void disp_symb(void);

	
	Clcdc_pc1360(CPObject *parent = 0)	: Clcdc(parent)
	{						//[constructor]
		Color_Off.setRgb(
							(int) (0x58*contrast),
							(int) (0x6c*contrast),
							(int) (0x60*contrast));
	};
	virtual ~Clcdc_pc1360()
	{						//[constructor]
	};

};
class Clcdc_pc2500:public Clcdc
{
public:
    void disp(void);				//display LCDC data to screen
    void disp_symb(void);

    Clcdc_pc2500(CPObject *parent)	: Clcdc(parent)
    {						//[constructor]
        Color_Off.setRgb(
                            (int) (0x4f*contrast),
                            (int) (0x5b*contrast),
                            (int) (0x51*contrast));
    };
    virtual ~Clcdc_pc2500()
    {						//[constructor]
    };

};
class Clcdc_pc1250:public Clcdc{
public:
	void disp(void);				//display LCDC data to screen
	void disp_symb(void);

	Clcdc_pc1250(CPObject *parent = 0)	: Clcdc(parent)
	{					//[constructor]

        Color_Off.setRgb(	(int) ( 0x4a*contrast ),
								(int) ( 0x63*contrast ),
                                (int) ( 0x63*contrast ) );
    }
	virtual ~Clcdc_pc1250()
	{						//[constructor]
    }

};

class Clcdc_pc1245:public Clcdc_pc1250{
public:
	void disp_symb(void);
	void disp(void);				//display LCDC data to screen
    const char*	GetClassName(){ return("Clcdc_pc1245");}

	Clcdc_pc1245(CPObject *parent = 0)	: Clcdc_pc1250(parent)
	{						//[constructor]

		Color_Off.setRgb(
							(int) (0x7e*contrast),
							(int) (0x94*contrast),
							(int) (0x90*contrast));
    }
	virtual ~Clcdc_pc1245()
	{						//[constructor]
    }

};


class Clcdc_pc1255:public Clcdc_pc1250{
public:
    const char*	GetClassName(){ return("Clcdc_pc1255");}

	Clcdc_pc1255(CPObject *parent = 0)	: Clcdc_pc1250(parent){						//[constructor]
		Color_Off.setRgb(
                            (int) (0x63*contrast),
                            (int) (0x6e*contrast),
                            (int) (0x5e*contrast));
	};
	virtual ~Clcdc_pc1255()
	{						//[constructor]
	};

};

class Clcdc_pc1260:public Clcdc_pc1250{
public:
	void disp_symb(void);
	void disp(void);				//display LCDC data to screen
    const char*	GetClassName(){ return("Clcdc_pc1260");}


	Clcdc_pc1260(CPObject *parent = 0)	: Clcdc_pc1250(parent){						//[constructor]
		Color_Off.setRgb(
                            (int) (0x63*contrast),
                            (int) (0x6D*contrast),
                            (int) (0x62*contrast));
	};
	virtual ~Clcdc_pc1260()
	{						//[constructor]
	};

};

class Clcdc_pc1401:public Clcdc{
public:
	void disp(void);				//display LCDC data to screen
	void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_pc1401");}



	Clcdc_pc1401(CPObject *parent = 0)	: Clcdc(parent){						//[constructor]
		Color_Off.setRgb(
							(int) (0x61*contrast),
							(int) (0x6D*contrast),
							(int) (0x61*contrast));
	};
	virtual ~Clcdc_pc1401()
	{						//[constructor]
	};

};

class Clcdc_pc1403:public Clcdc{
public:
	void disp(void);				//display LCDC data to screen
	void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_pc1403");}



	Clcdc_pc1403(CPObject *parent = 0)	: Clcdc(parent){						//[constructor]
		Color_Off.setRgb(
							(int) (0x5d*contrast),
							(int) (0x71*contrast),
							(int) (0x6a*contrast));
    }
	virtual ~Clcdc_pc1403()
	{						//[constructor]
    }

};

class Clcdc_pc1425:public Clcdc_pc1403{
public:
    void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_pc1425");}



    Clcdc_pc1425(CPObject *parent = 0)	: Clcdc_pc1403(parent){						//[constructor]
        Color_Off.setRgb(
                            (int) (0x5d*contrast),
                            (int) (0x71*contrast),
                            (int) (0x6a*contrast));
    }
    virtual ~Clcdc_pc1425()
    {						//[constructor]
    }

};


class Clcdc_pc1450:public Clcdc{
public:
	bool init(void);
	void disp(void);				//display LCDC data to screen
	void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_pc1450");}


	int x2a[100];

	Clcdc_pc1450(CPObject *parent = 0)	: Clcdc(parent){						//[constructor]
		Color_Off.setRgb(
							(int) (0x5d*contrast),
							(int) (0x71*contrast),
							(int) (0x6a*contrast));
	};
	virtual ~Clcdc_pc1450()
	{						//[constructor]
	};

};

class Clcdc_pc1475:public Clcdc{
public:
	bool init(void);
	void disp(void);				//display LCDC data to screen
	void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_pc1475");}


	int x2a[100];

	Clcdc_pc1475(CPObject *parent = 0)	: Clcdc(parent){						//[constructor]
		Color_Off.setRgb(
							(int) (0x5d*contrast),
							(int) (0x71*contrast),
							(int) (0x6a*contrast));
	};
	virtual ~Clcdc_pc1475()
	{						//[constructor]
    }

};

class Clcdc_pc1500:public Clcdc{
public:
	void disp(void);				//display LCDC data to screen
	void disp_symb(void);
    const char*	GetClassName(){ return("Clcdc_pc1500");}


	int x2a[100];


	Clcdc_pc1500(CPObject *parent = 0)	: Clcdc(parent){						//[constructor]
		Color_Off.setRgb(
							(int) (0x6e*contrast),
							(int) (0x80*contrast),
							(int) (0x80*contrast));
    }
	virtual ~Clcdc_pc1500()
	{						//[constructor]
    }

};
class Clcdc_trspc2:public Clcdc_pc1500{
public:
    const char*	GetClassName(){ return("Clcdc_trspc2");}

	Clcdc_trspc2(CPObject *parent = 0)	: Clcdc_pc1500(parent){						//[constructor]
		Color_Off.setRgb(
							(int) (0x53*contrast),
							(int) (0x62*contrast),
							(int) (0x4f*contrast));
	};
	virtual ~Clcdc_trspc2()
	{						//[constructor]
    }

};

class Clcdc_pc1500A:public Clcdc_pc1500{
public:
    const char*	GetClassName(){ return("Clcdc_pc1500A");}

	Clcdc_pc1500A(CPObject *parent = 0)	: Clcdc_pc1500(parent){						//[constructor]
		Color_Off.setRgb(
							(int) (0x60*contrast),
							(int) (0x6a*contrast),
							(int) (0x60*contrast));
	};
	virtual ~Clcdc_pc1500A()
	{						//[constructor]
    }

};




///////////////////////

#define COLOR(b)	( ( (b)&&On ) ? Color_On : Color_Off)






#endif
