#ifndef _CE150_H_
#define _CE150_H_


#include "Log.h"
#include "lh5810.h"
#include "cprinter.h"
#include "Connect.h"
#include "Inter.h"
#include "Keyb.h"
#include "cmotor.h"



#define PEN_UP	0
#define PEN_DOWN 1

class Cprinter;
class CbusPc1500;

class Cce150:public Cprinter{						
public:

	bool		run(void);
	void		Print(void);
	bool		init(void);
	bool		exit(void);

	bool		Print_Mode;
	bool		needRedraw;
	Cconnector	*pCONNECTOR;	qint64 pCONNECTOR_value;
    Cconnector	*pEXTCONNECTOR;	qint64 pEXTCONNECTOR_value;
    Cconnector	*pTAPECONNECTOR;qint64 pTAPECONNECTOR_value;

    CbusPc1500  *bus;
    CLH5810		*pLH5810;

	bool lh5810_write(void);
	bool lh5810_read(void);
	
	QImage *ce150buf;
	QImage *ce150display;
	QImage *ce150pen;

	void	clearPaper(void);
	void	SaveAsText(void);

    Cmotor		Motor_X;
    Cmotor		Motor_Y;


    Cce150(CPObject *parent = 0);
    virtual ~Cce150();

public:

    void PlayClac();
protected slots:
    void contextMenuEvent ( QContextMenuEvent * );

protected:
    bool		Next_Color(void);
    int			Pen_X,Pen_Y,Pen_Z,prev_Pen_X,prev_Pen_Y,prev_Pen_Z;
	bool		Pen_Status;			// UP or Down Position
	int			Pen_Color;			// Color : 0 to 3
	UINT32		Offset_Paper_View;
	BYTE		Rot;


	bool		Change_Color;
	bool	StartRot;
	BYTE	Direction;
    bool rmt;

};

#endif
