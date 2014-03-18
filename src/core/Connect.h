/*** PC-XXXX Emulator [CONNECT.H] ***********/
/* Connector emulation class				*/
/********************************************/


#ifndef _CONNECT_H
#define _CONNECT_H

#include <QObject>
#include <QPoint>

#include "common.h"

#define GET_PIN(n)		(pCONNECTOR->Get_pin(n))
#define SET_PIN(n,v)	(pCONNECTOR->Set_pin(n,v))

class CPObject;

#ifdef P_AVOID
namespace Avoid { class ShapeConnectionPin; }
#endif

class Cconnector:public QObject
{
    Q_OBJECT
public:
	CPObject *Parent;
	
    enum ConnectorType
    {
        Sharp_9,
        Sharp_15,
        Sharp_11,
        Sharp_60,
        Jack,
        Optical,
        Canon_15,
        Canon_9,
        Canon_40,
        Canon_Din,
        Casio_30,
        Centronics_36,
        DB_25,
        DIN_8,
        General_20,
        Panasonic_44,
        Panasonic_Capsule,
        hp41
    };

    enum ConnectorDir
    {
        NORTH,
        EAST,
        SOUTH,
        WEST
    };

	bool	refresh;

        int Id;
	
	QString	Desc;
	
#ifdef P_AVOID
    Avoid::ShapeConnectionPin *shapeconnectionpin;
#endif

    virtual bool init(void){return true;}
    virtual bool exit(void){return true;}
    Q_INVOKABLE qint64 Get_values(void)
    {
        return values;
    }
    Q_INVOKABLE void Set_values(qint64 val)
    {
        values = val;
    }
     // Return Pin value : pin number from 1 to N
    Q_INVOKABLE bool Get_pin(qint8 PinId)
    {
        return ((values >>(PinId-1)) & 0x01);
    }
     // Set Pin value : pin from 1 to N
    Q_INVOKABLE void Set_pin(qint8 PinId,bool Value)
    {
        PUT_BIT(values, (PinId-1), Value);
    }
	void	Dump_pin(void);
	char	dump[100];

    Q_INVOKABLE bool	getGender();
    Q_INVOKABLE void	setGender(bool);

    Q_INVOKABLE ConnectorType	getType();
    Q_INVOKABLE void	setType(ConnectorType);

	void	ConnectTo(Cconnector *);

    void    setDir(ConnectorDir d) { dir = d; }
    ConnectorDir getDir() { return dir;}
    void  setOppDir(ConnectorDir d) {
        switch (d) {
        case NORTH : dir = SOUTH; break;
        case EAST  : dir = WEST; break;
        case SOUTH : dir = NORTH;break;
        case WEST : dir = EAST;break;
        }
    }

    void    setSnap(QPoint p);
    QPoint  getSnap(void) {return snap;}
    int    getNbpins(void) {return nbpins;}
    void   setNbpins(int nb) { nbpins = nb;}

    static bool arePluggable(Cconnector *a,Cconnector *b);
	
    Cconnector(CPObject *parent , QString desc, bool newGender = false);
    Cconnector(CPObject *parent ,
               int nbpins,
               int Id,
               ConnectorType type,
               QString desc,
               bool newGender = false,
               QPoint snap=QPoint(0,0),
               ConnectorDir dir = WEST);

    virtual ~Cconnector(){}

    QPoint pos();
private:
	bool	gender;		// Male = true   Female = false  :-)
    ConnectorType Type;
    int		nbpins;
    qint64	values;
    QPoint  snap;
    ConnectorDir dir;
};

#endif		// _CONNECT_H
