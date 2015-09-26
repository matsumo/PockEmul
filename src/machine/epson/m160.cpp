


#include <QPainter>
#include <QTime>

#include <QFileDialog>
#include <QThread>
#include <QDebug>

#include "common.h"

#include "m160.h"
#include "pcxxxx.h"
#include "Log.h"
#include "watchpoint.h"
#include "paperwidget.h"
#include "Keyb.h"
#include "Connect.h"
#include "dialoganalog.h"

extern int ask(QWidget *parent,QString msg,int nbButton);

Cm160::Cm160(CPObject *parent):Cprinter(parent)
{								//[constructor]
    setfrequency( 32767 );
    m160buf	= 0;
    m160display= 0;
    charTable = 0;
    margin = 15;
    ToDestroy	= false;
    BackGroundFname	= P_RES(":/ext/pc-2021.png");
    setcfgfname("m160");

    settop(10);
    setposX(0);

    pTIMER		= pPC->pTIMER; //new Ctimer(this);

    setDXmm(108);
    setDYmm(130);
    setDZmm(43);

    setDX(386);
    setDY(464);

    setPaperPos(QRect(90,0,195,180));

    ctrl_char = false;
    t = 0;
    c = 0;
    rmtSwitch = false;

    internal_device_code = 0x0f;

    Motor = MotorP = MotorM = TS = RS = false;
}

Cm160::~Cm160() {
    delete m160buf;
    delete m160display;
    delete pCONNECTOR;
    delete charTable;
}


void Cm160::SaveAsText(void)
{
    ask(mainwindow,
        tr("This printer is a pure graphic printer\nSaving output as text is irrelevant"),
        1);
}


/*****************************************************/
/* Initialize PRINTER								 */
/*****************************************************/
void Cm160::clearPaper(void)
{
    // Fill it blank
    m160buf->fill(PaperColor.rgba());
    m160display->fill(QColor(255,255,255,0).rgba());
    settop(10);
    setposX(0);
    // empty TextBuffer
    TextBuffer.clear();
    paperWidget->updated = true;
}


bool Cm160::init(void)
{
    CPObject::init();

    pCONNECTOR	= new Cconnector(this,64,1,Cconnector::Custom,"Printer",false);
    WatchPoint.add(&pCONNECTOR_value,64,11,pPC,"Printer connector");
    AddLog(LOG_PRINTER,tr("PRT initializing..."));

    if(pKEYB)	pKEYB->init();
//    if(pTIMER)	pTIMER->init();

    // Create Paper Image
    m160buf	= new QImage(QSize(margin+144+margin, 1000),QImage::Format_ARGB32);
    m160display= new QImage(QSize(margin+144+margin, 180),QImage::Format_ARGB32);

// Create a paper widget

    paperWidget = new CpaperWidget(PaperPos(),m160buf,this);
//    paperWidget->updated = true;
    paperWidget->show();

    // Fill it blank
    clearPaper();

    run_oldstate = -1;

    TPIndex = pTIMER->initTP(1667/2);

    return true;
}



/*****************************************************/
/* Exit PRINTER										 */
/*****************************************************/
bool Cm160::exit(void)
{
    AddLog(LOG_PRINTER,"PRT Closing...");
    AddLog(LOG_PRINTER,"done.");
    Cprinter::exit();
    return true;
}


#define		WAIT ( pPC->frequency / 10000*6)

#define RECEIVE_MODE	1
#define SEND_MODE		2
#define TEST_MODE		3



bool Cm160::Get_Connector(Cbus *_bus) {
    Q_UNUSED(_bus)

    H1 = pCONNECTOR->Get_pin(1);  // P10 H1
    H2 = pCONNECTOR->Get_pin(2);  // P11 H2
    H3 = pCONNECTOR->Get_pin(3);  // P12 H3
    H4 = pCONNECTOR->Get_pin(4);  // P13 H4

    bool _prevMotor = Motor;
    MotorP = !pCONNECTOR->Get_pin(5);  // P14 M+
    MotorM = pCONNECTOR->Get_pin(6);  // P41 M-
    Motor = MotorP;// && MotorM;
    if (Motor && !_prevMotor) {
        pTIMER->resetTP(TPIndex);
    }

    MotorBreak = pCONNECTOR->Get_pin(6);  // P17 Motor Break

    return true;
}

bool Cm160::Set_Connector(Cbus *_bus) {
    Q_UNUSED(_bus)

    pCONNECTOR->Set_pin(7,TS);
    pCONNECTOR->Set_pin(8,RS);

    return true;
}


#define m160LATENCY (pTIMER->pPC->getfrequency()/3200)

bool Cm160::run(void)
{
    static int count = 0;
    static bool _print = false;
    static quint64 MotorCount=0;

    QPainter painter;

    if (!pPC->Power) return true;
    Get_Connector();

    bool _prevTS = TS;
    if (Motor) {
        // generate TS
        TS = pTIMER->GetTP(TPIndex);
    }
    else {
        TS = false;
//        RS = false;
//        _print = false;
//        count=0;
        Set_Connector();
        pCONNECTOR_value = pCONNECTOR->Get_values();
        return true;
    }

    if ( (TS != _prevTS))  {
        // increment count
        MotorCount++;
        if (_print) count++;

    }
    else {
        Set_Connector();
        pCONNECTOR_value = pCONNECTOR->Get_values();
        return true;
    }

    if ( !RS /*&& !_print*/ && ((MotorCount % 252)==0) ) {
        RS = true;
        _print = true;
        count=1;
        posX=0;
        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(1);
    }

    if (_print)
    {
        int _offset = -1;
        if (H1) _offset = 3;
        if (H2) _offset = 2;
        if (H3) _offset = 1;
        if (H4) _offset = 0;
//        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(4);
        if (_offset>=0) {
            int _x = posX+(_offset * 36);
            painter.begin(m160buf);
            painter.setPen(Qt::black);
            painter.drawPoint( margin + _x, top);
//            qWarning()<<"drawPoint( "<<margin + _x<<", "<<top<<")";
//            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(5);
            painter.end();
        }

        if ((count-2)%4==0) {
            posX++;
//            qWarning()<<"posX:"<<posX<<" top="<<top<<"  count="<<count;
            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(3);
        }

        if (count==20) {
            RS = false;
        }

//        if (count==144) {
//            _print = false;
//            RS=false;
//            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(6);
////            posX=0;
//            top++;
//            m160buf = checkPaper(m160buf,top);

//            painter.begin(m160display);
//            painter.drawImage(QRectF(0,MAX(180-top,0),
//                                     m160display->width(),MIN(top,180)),
//                              *m160buf,
//                              QRectF(0,MAX(0,top-180),
//                                     m160display->width(),MIN(top,180)));
//            painter.end();

//            Refresh_Display = true;
//            pPC->Refresh_Display = true;

//            paperWidget->setOffset(QPoint(0,top));
//            paperWidget->updated = true;

//        }

        if (count==252) {
//            RS = true;
//            _print = true;
//            count=1;
//            posX=0;
//            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(7);
            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(6);
//            posX=0;
            top++;
            m160buf = checkPaper(m160buf,top);

            painter.begin(m160display);
            painter.drawImage(QRectF(0,MAX(180-top,0),
                                     m160display->width(),MIN(top,180)),
                              *m160buf,
                              QRectF(0,MAX(0,top-180),
                                     m160display->width(),MIN(top,180)));
            painter.end();

            Refresh_Display = true;
            pPC->Refresh_Display = true;

            paperWidget->setOffset(QPoint(0,top));
            paperWidget->updated = true;
        }
    }





    Set_Connector();

    pCONNECTOR_value = pCONNECTOR->Get_values();

    return true;
}


