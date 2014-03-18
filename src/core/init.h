/********************************************************************************************************
 * PROGRAM      : test
 * DATE - TIME  : samedi 28 octobre 2006 - 12h42
 * AUTHOR       :  (  )
 * FILENAME     : init.h
 * LICENSE      : GPL
 * COMMENTARY   : 
 ********************************************************************************************************/
#ifndef _INIT_H
#define _INIT_H

#include <QEvent>
#include <QResizeEvent>
//#include <QCustomEvent>
#include <QThread>
#include <QTimer>
#include <QStringList>


class CPocketThread;
class CWatchPoint;
class CPObject;

typedef qint64 pwatchfonc(void);

extern QList<CPocketThread*> listPcThread;	
extern CWatchPoint WatchPoint;

class CWatchPointItem {
public:
    CWatchPointItem(qint64* Point,qint8 PointSize,qint8 nbBits,CPObject* PObject,QString WatchPointName,QHash<int,QString> Labels) {
        this->Point = Point;
        this->PointSize = PointSize;
        this->nbBits = nbBits;
        this->PObject = PObject;
        this->WatchPointName = WatchPointName;
        this->Labels = Labels;
    }

    qint64 *Point;
    qint8   PointSize;
    qint8   nbBits;
    CPObject* PObject;
    QString WatchPointName;
    QHash<int,QString> Labels;
};

class CWatchPoint
{
public:
    QList<CWatchPointItem> items;
	
	void remove(CPObject * object)
	{
        for (int i=items.size()-1;i>=0;i--)
		{
            if (items.at(i).PObject == object)
			{
                items.removeAt(i);

			}
		}
    }
    typedef QHash<int,QString> lblType;
    void add( qint64 * watchpoint, qint8 ptrSize,qint8 nbbits,CPObject * object, QString name,QHash<int,QString> lblList = lblType())
    {
        CWatchPointItem * item = new CWatchPointItem(watchpoint,ptrSize,nbbits,object,name,lblList);
        items.append(*item);
        delete item;

    }

    void remove( qint64 * watchpoint)
    {
        for (int i =0;i< items.size();i++) {
            if (items.at(i).Point == watchpoint) {
                items.removeAt(i);
                return;
            }
        }

    }
};

class CPocketThread:public QThread
{
Q_OBJECT

public:
    void run();
	QTimer *threadTimer;
	void SendSignal_AddLogItem(QString);
	void SendSignal_Resize(QSize,CPObject * );
	

    CPocketThread(QObject * parent):QThread(parent)
	{
//		pPC = material;
	}
	
signals:
	void AddLogItem(QString);
	void Resize(QSize,CPObject * );
	void Destroy(CPObject * );
    
};

extern CPocketThread* PcThread;


#endif
