 /********************************************************************************************************
 * PROGRAM      : 
 * DATE - TIME  : samedi 28 octobre 2006 - 12h42
 * AUTHOR       :  (  )
 * FILENAME     : init.cpp
 * LICENSE      : GPL
 * COMMENTARY   : 
 ********************************************************************************************************/
#include <QtGui>
#include <QString>
#include <QPainter>

#include "init.h"
#include "mainwindowpockemul.h"

#ifdef P_AVOID
#include "libavoid.h"
#endif

#include "common.h"

#include "dialoganalog.h"


#include "pcxxxx.h" 
#include "Inter.h"
#include "clink.h"


extern QList<CPObject *> listpPObject;  
//extern QTimer *timer;

CWatchPoint WatchPoint;

bool	UpdateDisplayRunning;
CPocketThread* PcThread;
QList<CPocketThread*> listPcThread;


void CPocketThread::run()
{
    bool pause = true;

#ifdef NEWTIMER
    QElapsedTimer timer;
    timer.start();
#endif

#ifdef EMSCRIPTEN
    int j=0;
    while((j++)<1000)
#else
    while(true)
#endif
    {
        //int j=0;
        //while ((j++) < 20)
        {
            pause = true;
            for (int i=0;i<listpPObject.size();i++)
            {
                CPObject *pPC = listpPObject.at(i);
                // si objet maitre
                int f = pPC->getfrequency();
                if ( f != 0)
                {
                    // test si en retard
                    quint64 cs = pPC->pTIMER->currentState();
//                    qWarning()<< cs - pPC->pTIMER->state;
                    if (pPC->pTIMER->state < cs)
                    {
//                        qWarning()<<pPC->pTIMER->state << cs;
                        if (pPC->ioFreq > 0) {
                            quint64 step = MIN((quint64)(f / pPC->ioFreq - pPC->pTIMER->deltaStep),cs - pPC->pTIMER->state);
                            pPC->pTIMER->deltaStep = pPC->runRange(step) - step;
                        }
                        else {
                            pPC->run();
                        }
                        // WRITE the LINK BOX Connector
                        if ( ( dynamic_cast<CpcXXXX *>(pPC) ) &&  !((CpcXXXX*)pPC)->DasmFlag) {
                            mainwindow->pdirectLink->Cascade(pPC);
                        }

                        pause = false;

                        if ((mainwindow->dialoganalogic) && mainwindow->dialoganalogic->capture()) {
                            mainwindow->dialoganalogic->captureData();
                        }

                    }
                    else {
//                        qWarning()<<pPC->pTIMER->state << cs;
                    }
                }
                if (pPC->toDestroy)
                {
                    // Unlink before destroy
                    mainwindow->slotUnlink(pPC);

                    listpPObject.removeAt(i);
                    i--;
                    emit Destroy(pPC);
                }
            }
        }
#ifdef EMSCRIPTEN
        if (pause) return;
#else
        if (pause) {
//            qWarning()<<"pause";
            msleep(10);
        }
#endif
#ifdef NEWTIMER
        mainwindow->rawclk += timer.nsecsElapsed();
        timer.restart();
#endif
    }
}

void CPocketThread::SendSignal_AddLogItem(QString str)
{
	emit AddLogItem(str);
}

void CPocketThread::SendSignal_Resize(QSize size,CPObject *pPObject)
{
	emit Resize(size,pPObject);
}
