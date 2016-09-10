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


class CPocketThread:public QThread
{
Q_OBJECT

public:
    void run();
	QTimer *threadTimer;
    bool PcThreadRunning;
    bool PcThreadSuspended;
	void SendSignal_AddLogItem(QString);
	void SendSignal_Resize(QSize,CPObject * );
	

    CPocketThread(QObject * parent):QThread(parent)
	{
//		pPC = material;
        PcThreadSuspended = false;
	}
	
signals:
	void AddLogItem(QString);
	void Resize(QSize,CPObject * );
	void Destroy(CPObject * );
    
};

extern CPocketThread* PcThread;


class CPocketThreadRun:public QThread
{
Q_OBJECT

public:
    void run();
    QTimer *threadTimer;



    CPObject *pPC;

    CPocketThreadRun(CPObject * parent):QThread((QObject*)parent)
    {
        pPC = parent;
    }



signals:
    void AddLogItem(QString);
    void Resize(QSize,CPObject * );
    void Destroy(CPObject * );

};

#endif
