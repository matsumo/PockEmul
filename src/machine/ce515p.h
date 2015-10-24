#ifndef CE515P_H
#define CE515P_H

#ifndef NO_SOUND
#include <QSound>
#endif

#include "cprinter.h"




#define PEN_UP	0
#define PEN_DOWN 1


//extern TransMap KeyMapce150[];
//extern int KeyMapce150Lenght;

enum eMODE {GRAPH,TEXT};

class CMove
{
public:
    int		X;
    int     Y;
    bool    penDown;
    bool    changeColor;
    int     oldColor;
    int     newColor;
    CMove(int x,int y,bool pendown) {
        this->X = x;
        this->Y = y;
        this->penDown = pendown;
        changeColor = false;
    }
    CMove(int oldcolor,int newcolor) {
        this->oldColor = oldcolor;
        this->newColor = newcolor;
        this->changeColor = true;
    }

};

class Cce515p:public Cprinter{
Q_OBJECT
public:

    virtual bool		run(void);
    void		Print(CMove);
    virtual bool		init(void);
    virtual bool		exit(void);

    const char		*PaperFname;
    int			Paper_X,Paper_Y,Paper_DX,Paper_DY;

    bool		Print_Mode;
    bool		needRedraw;
    //Cconnector	*pCONNECTOR;	qint64 pCONNECTOR_value;

    QImage *ce515pbuf;
    QImage *ce515pdisplay;
    QImage *ce515ppen;

    void	clearPaper(void);
    void	SaveAsText(void);
    void    set_SD(quint8);
    void    drawChar(quint8 data,qint8 rot= 0);
    void    DrawMove(int lenght,int dir,bool penDown);
    virtual void    Command(quint8);
    virtual void    ProcessEscCommand(void);
    virtual void    ProcessGraphCommand(void);
    virtual void    ProcessMultiPointCommand(QString);
    void    DrawLine(int xa, int ya, int xb, int yb);
    void    DrawTest(void);
    void    PaperFeed(void);

    void    Draw(void);

    Cce515p(CPObject *parent = 0);
    virtual ~Cce515p();

    int   lastX;
    int margin;

protected:
    bool		Next_Color(void);
    int			Pen_X,Pen_Y,Pen_Z,prev_Pen_X,prev_Pen_Y,prev_Pen_Z;
    bool		Pen_Status;			// UP or Down Position
    int			Pen_Color;			// Color : 0 to 3
    UINT32		Offset_Paper_View;
    quint8		Rot;
    //quint8  	t,c;

    bool		Change_Color;

    bool	StartRot;
    quint8	Direction;
    quint8    SD;
    int		Sii_wait;
    static const QString graph[];

protected:
    QList<CMove> moveBuffer;
    int charSize;
    QString escCommand;
    QString graphCommand;
    bool    escMode;

    int     lenght,dir;
    bool    penDown;

    eMODE   mode;

    int orig_X;
    int orig_Y;
    int lineType;
    int mainRot;
    quint64  oldstate_run;
    quint64  printer_oldstate_draw;
    quint64  printer_oldstate_paperfeed;
    unsigned char	t,c,waitbitstart,waitbitstop;
#ifndef NO_SOUND
    QSound *clac;
#endif

};





#endif // CE515P_H
