


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

#define DOWN	0
#define UP		1

TransMap KeyMapm160[]={
    {1,	"FEED  ",	K_PFEED,34,234,	9},
    {2,	"POWER ON",	K_POW_ON,34,234,	9},
    {3,	"POWER OFF",K_POW_OFF,34,234,	9}
};
int KeyMapm160Lenght = 3;

Cm160::Cm160(CPObject *parent):Cprinter(parent)
{								//[constructor]
    setfrequency( 32767 );
    m160buf	= 0;
    m160display= 0;
    //bells		= 0;
    charTable = 0;
    margin = 25;
    ToDestroy	= false;
    BackGroundFname	= P_RES(":/ext/pc-2021.png");
    setcfgfname("m160");

    settop(10);
    setposX(0);

    pTIMER		= pPC->pTIMER; //new Ctimer(this);
    KeyMap      = KeyMapm160;
    KeyMapLenght= KeyMapm160Lenght;
    pKEYB		= new Ckeyb(this,"m160.map");
    setDXmm(108);
    setDYmm(130);
    setDZmm(43);

    setDX(386);
    setDY(464);

    setPaperPos(QRect(70,-3,275,149));

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
//    delete bells;
}

void Cm160::ComputeKey(KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)

    if (pKEYB->LastKey == K_PFEED) {
        Refreshm160(0x0d);
    }
}


void Cm160::SaveAsText(void)
{
    mainwindow->releaseKeyboard();

    QString s = QFileDialog::getSaveFileName(
                    mainwindow,
                    tr("Choose a filename to save under"),
                    ".",
                   tr("Text File (*.txt)"));

    QFile file(s);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    file.write(TextBuffer);

    mainwindow->grabKeyboard();
        AddLog(LOG_PRINTER,TextBuffer.data());
}

void Cm160::Refreshm160(qint8 data)
{

    QPainter painter;

//if (posX==0) bells->play();

// copy ce126buf to ce126display
// The final paper image is 207 x 149 at (277,0) for the ce125

// grab data char to byteArray
    if ( (data == 0xff) || (data==0x0a)) return;

    TextBuffer += data;

    if (data == 0x0d){
        top+=10;
        posX=0;
//        qWarning()<<"CR PRINTED";
    }
    else
    {
        painter.begin(m160buf);
        int x = ((data>>4) & 0x0F)*6;
        int y = (data & 0x0F) * 8;
        painter.drawImage(	QPointF( margin + (7 * posX),top),
                            *charTable,
                            QRectF( x , y , 5,7));
        posX++;
        painter.end();
    }

    if (posX >= 40) {
        posX=0;
        top+=10;
    }

    painter.begin(m160display);

    painter.drawImage(QRectF(0,MAX(149-top,0),330,MIN(top,149)),*m160buf,QRectF(0,MAX(0,top-149),340,MIN(top,149)));

// Draw printer head
//    painter.fillRect(QRect(0 , 147,407,2),QBrush(QColor(0,0,0)));
//    painter.fillRect(QRect(21 + (7 * posX) , 147,14,2),QBrush(QColor(255,255,255)));

    painter.end();

    Refresh_Display = true;

    paperWidget->setOffset(QPoint(0,top));
    paperWidget->updated = true;

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
    // The final paper image is 207 x 149 at (277,0) for the ce125
    m160buf	= new QImage(QSize(340, 1000),QImage::Format_ARGB32);
    m160display= new QImage(QSize(340, 149),QImage::Format_ARGB32);


//TODO Update the chartable with upd16343 char table
    charTable = new QImage(P_RES(":/ext/ce126ptable.bmp"));

//	bells	 = new QSound("ce.wav");

// Create a paper widget

    paperWidget = new CpaperWidget(PaperPos(),m160buf,this);
//    paperWidget->updated = true;
    paperWidget->show();

    // Fill it blank
    clearPaper();

    run_oldstate = -1;

    TPIndex = pTIMER->initTP(1667);

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


/*****************************************************/
/* CE-126P PRINTER emulation						 */
/*****************************************************/

void Cm160::Printer(qint8 d)
{
    if(ctrl_char && d==0x20) {
        ctrl_char=false;
        Refreshm160(d);
    }
    else
    {
        if(d==0xf || d==0xe || d==0x03)
            ctrl_char=true;
        else
        {
            Refreshm160(d);
        }
    }
}


#define		WAIT ( pPC->frequency / 10000*6)

#define RECEIVE_MODE	1
#define SEND_MODE		2
#define TEST_MODE		3



bool Cm160::Get_Connector(Cbus *_bus) {

    H1 = pCONNECTOR->Get_pin(1);  // P10 H1
    H2 = pCONNECTOR->Get_pin(2);  // P11 H2
    H3 = pCONNECTOR->Get_pin(3);  // P12 H3
    H4 = pCONNECTOR->Get_pin(4);  // P13 H4

    bool _prevMotor = Motor;
    MotorP = pCONNECTOR->Get_pin(5);  // P14 M+
    MotorM = pCONNECTOR->Get_pin(6);  // P41 M-
    Motor = !MotorP;// && MotorM;
    if (Motor && !_prevMotor) {
        pTIMER->resetTP(TPIndex);
    }

    MotorBreak = pCONNECTOR->Get_pin(6);  // P17 Motor Break

    return true;
}

bool Cm160::Set_Connector(Cbus *_bus) {
    pCONNECTOR->Set_pin(7,TS);
    pCONNECTOR->Set_pin(8,RS);

    return true;
}


#define m160LATENCY (pTIMER->pPC->getfrequency()/3200)

bool Cm160::run(void)
{
    static int count = 0;
    QPainter painter;

    Get_Connector();

    bool _prevTS = TS;
    if (Motor) {
        // generate TS
        TS = pTIMER->GetTP(TPIndex);
    }
    else {
        TS = false;
        count=0;
        Set_Connector();
        pCONNECTOR_value = pCONNECTOR->Get_values();
        return true;
    }

    if ( (TS != _prevTS))  {
        // increment count
        count++;
        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(1);
    }
    else {
        Set_Connector();
        pCONNECTOR_value = pCONNECTOR->Get_values();
        return true;
    }

    if (!RS && (count == 100)) {
        RS = true;
        count=1;
        posX=0;
//        top++;

    }
    if (!RS && (count==252)) {
        RS = true;
        count=1;
        posX=0;
    }


//    if (count > 150) {
//        RS = false;
//        count=0;
//    }

//    if (RS /*&& (count%2==0)*/)
    {
        int _offset = -1;
        if (H1) _offset = 3;
        if (H2) _offset = 2;
        if (H3) _offset = 1;
        if (H4) _offset = 0;
        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(4);
        if (_offset>=0) {
            int _x = posX+(_offset * 36);
            painter.begin(m160buf);
            painter.setPen(Qt::black);
            painter.drawPoint( margin + _x, top);
            qWarning()<<"drawPoint( "<<margin + _x<<", "<<top<<")";
            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(5);
            painter.end();
        }

        if (count%4==0) {
            posX++;
//            qWarning()<<"posX:"<<posX<<" top="<<top;
            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(3);
        }

        if (count==100) {
            RS = false;
        }

        if (count==144) {
            RS=false;
            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(6);
                        posX=0;
                        top++;
                        m160buf = checkPaper(m160buf,top);

                        painter.begin(m160display);
                        painter.drawImage(QRectF(0,MAX(149-top,0),330,MIN(top,149)),*m160buf,QRectF(0,MAX(0,top-149),340,MIN(top,149)));
                        painter.end();

                        Refresh_Display = true;

                        paperWidget->setOffset(QPoint(0,top));
                        paperWidget->updated = true;

        }
    }





    Set_Connector();

    pCONNECTOR_value = pCONNECTOR->Get_values();

    return true;
}


