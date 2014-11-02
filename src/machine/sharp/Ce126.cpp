/********************************************************************************************************
 * PROGRAM      : test
 * DATE - TIME  : samedi 28 octobre 2006 - 12h40
 * AUTHOR       :  (  )
 * FILENAME     : Ce126.cpp
 * LICENSE      : GPL
 * COMMENTARY   : 
 ********************************************************************************************************/



#include <QPainter>
#include <QTime>
#include <QFileDialog>
#include <QThread>
#include <QDebug>

#include "common.h"

#include "Ce126.h"
#include "pcxxxx.h"
#include "Log.h"
#include "dialoganalog.h"
#include "paperwidget.h"
#include "Keyb.h"
#include "Connect.h"
#include "watchpoint.h"

#define DOWN	0
#define UP		1

TransMap KeyMapce126[]={
    {1,	"FEED  ",	K_PFEED,34,234,	9},
    {2,	"RMT ON",	K_RMT_ON,34,234,9},
    {3,	"RMT OFF",	K_RMT_OFF,34,234,9},
    {4,	"POWER ON",	K_POW_ON,34,234,	9},
    {5,	"POWER OFF",K_POW_OFF,34,234,	9}
};
int KeyMapce126Lenght = 5;

Cce126::Cce126(CPObject *parent):Cprinter(parent)
{								//[constructor]
    Q_UNUSED(parent)

    setfrequency( 0);
    ce126buf	= 0;
    ce126display= 0;
    
//bells		= 0;
    charTable = 0;
    margin = 25;
    ToDestroy	= false;
    BackGroundFname	= P_RES(":/ext/ce-126p.png");
    setcfgfname("ce126p");

    settop(10);
    setposX(0);    pTIMER		= new Ctimer(this);
    KeyMap      = KeyMapce126;
    KeyMapLenght= KeyMapce126Lenght;
    pKEYB		= new Ckeyb(this,"ce126.map");
    setDX(620);
    setDY(488);

    setDXmm(116);
    setDYmm(140);
    setDZmm(23);

    setPaperPos(QRect(150,-3,207,149));

    ctrl_char = false;
    t = 0;
    c = 0;
    rmtSwitch = false;

    internal_device_code = 0x0f;
}

Cce126::~Cce126() {
    delete ce126buf;
    delete ce126display;
    delete pCONNECTOR;
    delete pTAPECONNECTOR;
    delete charTable;
//    delete bells;
}

void Cce126::ComputeKey(KEYEVENT ke,int scancode)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)

    if (pKEYB->LastKey == K_PFEED) {
        RefreshCe126(0x0d);
    }
    if (pKEYB->LastKey == K_RMT_ON) {
        rmtSwitch = true;
    }
    if (pKEYB->LastKey == K_RMT_OFF) {
        rmtSwitch = false;
    }
}


void Cce126::SaveAsText(void)
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

void Cce126::RefreshCe126(qint8 data)
{
 
	QPainter painter;

//if (posX==0) bells->play();
	
// copy ce126buf to ce126display
// The final paper image is 207 x 149 at (277,0) for the ce125

// grab data char to byteArray
	TextBuffer += data;

	if (data == 0x0d){
		top+=10; 
		posX=0;
//        qWarning()<<"CR PRINTED";
	}
	else
	{
		painter.begin(ce126buf);
		int x = ((data>>4) & 0x0F)*6;
		int y = (data & 0x0F) * 8;
        painter.drawImage(	QPointF( margin + (7 * posX),top),
							*charTable,
							QRectF( x , y , 5,7));
		posX++;
		painter.end();
	}
	

	painter.begin(ce126display);
	
	painter.drawImage(QRectF(0,MAX(149-top,0),207,MIN(top,149)),*ce126buf,QRectF(0,MAX(0,top-149),207,MIN(top,149)));	

// Draw printer head
	painter.fillRect(QRect(0 , 147,207,2),QBrush(QColor(0,0,0)));
	painter.fillRect(QRect(21 + (7 * posX) , 147,14,2),QBrush(QColor(255,255,255)));

	painter.end();
	
	Refresh_Display = true;
	
	paperWidget->setOffset(QPoint(0,top));
    paperWidget->updated = true;
	
}


/*****************************************************/
/* Initialize PRINTER								 */
/*****************************************************/
void Cce126::clearPaper(void)
{
	// Fill it blank
	ce126buf->fill(PaperColor.rgba());
	ce126display->fill(QColor(255,255,255,0).rgba());
	settop(10);
	setposX(0);
	// empty TextBuffer
	TextBuffer.clear();
    paperWidget->updated = true;
}


bool Cce126::init(void)
{
	CPObject::init();
	
	setfrequency( 0);

    pCONNECTOR	= new Cconnector(this,11,0,Cconnector::Sharp_11,"Connector 11 pins",true,QPoint(594,238));	publish(pCONNECTOR);
    pTAPECONNECTOR	= new Cconnector(this,3,1,Cconnector::Jack,"Line in / Rec / Rmt",false);	publish(pTAPECONNECTOR);

	WatchPoint.add(&pCONNECTOR_value,64,11,this,"Standard 11pins connector");
	WatchPoint.add(&pTAPECONNECTOR_value,64,2,this,"Line In / Rec");

	AddLog(LOG_PRINTER,tr("PRT initializing..."));

	if(pKEYB)	pKEYB->init();
	if(pTIMER)	pTIMER->init();

	// Create CE-126 Paper Image
	// The final paper image is 207 x 149 at (277,0) for the ce125
    ce126buf	= new QImage(QSize(207, 1000),QImage::Format_ARGB32);
	ce126display= new QImage(QSize(207, 149),QImage::Format_ARGB32);



    charTable = new QImage(P_RES(":/ext/ce126ptable.bmp"));
	
//	bells	 = new QSound("ce.wav");	

// Create a paper widget

    paperWidget = new CpaperWidget(PaperPos(),ce126buf,this);
    paperWidget->updated = true;
//	paperWidget->show();

    // Fill it blank
    clearPaper();

    Previous_BUSY = GET_PIN(PIN_BUSY);
    Previous_MT_OUT1 = GET_PIN(PIN_MT_OUT1);
	time.start();
	
    run_oldstate = 0;

    code_transfer_step = 0;
    device_code = 0;

    MT_OUT2	= false;
    BUSY    = false;
    D_OUT	= false;
    MT_IN	= false;
    MT_OUT1	= false;
    D_IN	= false;
    ACK		= false;
    SEL2	= false;
    SEL1	= false;

	return true;
}



/*****************************************************/
/* Exit PRINTER										 */
/*****************************************************/
bool Cce126::exit(void)
{
	AddLog(LOG_PRINTER,"PRT Closing...");
	AddLog(LOG_PRINTER,"done.");
    Cprinter::exit();
	return true;
}


/*****************************************************/
/* CE-126P PRINTER emulation						 */
/*****************************************************/

void Cce126::Printer(qint8 d)
{
    if(ctrl_char && d==0x20) {
        ctrl_char=false;
        RefreshCe126(d);
    }
	else
	{
        if(d==0xf || d==0xe || d==0x03)
			ctrl_char=true;
		else
		{
            RefreshCe126(d);
		}
	}
}

//********************************************************/
//* Check for E-PORT and Get data						 */
//********************************************************/
// PIN_MT_OUT2	1
// PIN_GND		2
// PIN_VGG		3
// PIN_BUSY		4
// PIN_D_OUT	5
// PIN_MT_IN	6
// PIN_MT_OUT1	7
// PIN_D_IN		8
// PIN_ACK		9
// PIN_SEL2		10
// PIN_SEL1		11
//********************************************************/



#define		WAIT ( pPC->frequency / 10000*6)

#define RECEIVE_MODE	1
#define SEND_MODE		2
#define TEST_MODE		3

extern int LogLevel;
//
// no bit start
// no bit stop
void Cce126::pulldownsignal(void)
{
}

bool Cce126::Get_Connector(Cbus *_bus) {
    MT_OUT2	= GET_PIN(PIN_MT_OUT2);
    BUSY    = GET_PIN(PIN_BUSY);
    D_OUT	= GET_PIN(PIN_D_OUT);
    MT_OUT1	= GET_PIN(PIN_MT_OUT1);
    SEL2	= GET_PIN(PIN_SEL2);
    SEL1	= GET_PIN(PIN_SEL1);

    return true;
}

bool Cce126::Set_Connector(Cbus *_bus) {
    SET_PIN(PIN_MT_IN,MT_IN);
    SET_PIN(PIN_D_IN,D_IN);
    SET_PIN(PIN_ACK,ACK);

    return true;
}

#define INIT_MODE   0
#define BASIC_MODE  10

#define CE126LATENCY (pTIMER->pPC->getfrequency()/3200)
#define RAISE_MT_OUT1   ((Previous_MT_OUT1 == DOWN) && (MT_OUT1 == UP ))

//#define CE126LATENCY (getfrequency()/3200)
bool Cce126::run(void)
{

    Get_Connector();

	bool bit = false;
    ce126_Mode=0;//=RECEIVE_MODE;

    pTAPECONNECTOR->Set_pin(3,(rmtSwitch ? SEL1:true));       // RMT
    pTAPECONNECTOR->Set_pin(2,MT_OUT1);    // Out
    MT_IN = pTAPECONNECTOR->Get_pin(1);      // In

	pCONNECTOR_value = pCONNECTOR->Get_values();
	pTAPECONNECTOR_value = pTAPECONNECTOR->Get_values();

#if 1
// Try to introduce a latency 
    if (run_oldstate == 0) run_oldstate = pTIMER->state;
    quint64 deltastate = pTIMER->state - run_oldstate;
	if (deltastate < CE126LATENCY ) return true;
	run_oldstate	= pTIMER->state;
#endif


    //FIXME: Still not working with old generation pockets
#if 0

    if ( RAISE_MT_OUT1 && (D_OUT==DOWN))
    {
        code_transfer_step = BASIC_MODE;
        pTIMER->resetTimer(0);
//                    AddLog(LOG_PRINTER,tr("XIN from low to HIGHT"));
        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(50);
    }

    if (code_transfer_step == BASIC_MODE) {

                if (D_OUT==UP) {
                    code_transfer_step=INIT_MODE;
                    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(51);
                }
                else if ( (Previous_MT_OUT1 == UP) && (MT_OUT1 == UP ) && (pTIMER->msElapsedId(0)>2))
                {
                    t=0;
                    c=0;
                    if (BUSY == DOWN )
                    {
                        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(2);
                        ACK = UP;
//                        AddLog(LOG_PRINTER,tr("CHANGE ACK TO %1").arg(GET_PIN(PIN_ACK)?"1":"0"));
//                        Previous_BUSY = GET_PIN(PIN_BUSY);
//						return;
                    }
                    else {
                        // Works for 1350 but fail with others
                        ACK = DOWN;
                    }
                }
    }
#endif

    switch (code_transfer_step) {
    case INIT_MODE :    if ((MT_OUT1 == UP))// && (D_OUT==UP))
                {
                    pTIMER->resetTimer(1);//lastState = pTIMER->state; //time.restart();
                    code_transfer_step=1;
                    t=0; c=0;
                    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(8);
                }
                break;
    case 1 :    if ((MT_OUT1 == UP))// && (D_OUT==UP))
                {
                    if (pTIMER->msElapsedId(1) > 30) {
                        // Code transfer sequence started
                        // Raise ACK
                        code_transfer_step = 2;
                        AddLog(LOG_CONSOLE,tr("step : %1\n").arg(code_transfer_step,2,10) );
                        ACK = UP;
                        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(9);
                    }
                }
                else {
                    code_transfer_step=INIT_MODE;
                    //if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(7);
                }
                break;
    case 2:     if (BUSY == UP )	//check for BUSY
                {
                    if(D_OUT == UP)
                    {
                        bit = true;;
                    } else
                    {
                        bit = false;
                    }
                    t>>=1;
                    if (bit) t|=0x80;
                    if((c=(++c)&7)==0)
                    {
                        AddLog(LOG_CONSOLE,tr("device code printer : %1\n").arg(t,2,16) );

                        //Printer(t);
                        ACK = DOWN;
                        device_code = t;
                        if ((t == internal_device_code)) {
                            device_code = t;
                            code_transfer_step=4;
                            AddLog(LOG_CONSOLE,tr("step : %1\n").arg(code_transfer_step,2,10) );
                        }
                        else {
                            code_transfer_step = 6;
                            AddLog(LOG_CONSOLE,tr("step : %1\n").arg(code_transfer_step,2,10) );
                            pTIMER->resetTimer(1);
                            //lastState = pTIMER->state;
                        }
                        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(17);
                        t=0; c=0;
                    }
                    else {
                        ACK = DOWN;
                        code_transfer_step=3;
                        AddLog(LOG_CONSOLE,tr("step : %1\n").arg(code_transfer_step,2,10) );
                        pTIMER->resetTimer(1);//lastState=pTIMER->state;
                        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(18);
                    }
                }
                break;
    case 3:     if (pTIMER->msElapsedId(1)>2) {
                    code_transfer_step=2;
                    AddLog(LOG_CONSOLE,tr("step : %1\n").arg(code_transfer_step,2,10) );
                    // wait 2 ms and raise ACK
                    ACK = UP;
                    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(16);
                }
                break;
    case 4:     if ((BUSY == DOWN)){//&&(MT_OUT1 == DOWN)) {
                    ACK = UP;
                    code_transfer_step=5;
                    AddLog(LOG_CONSOLE,tr("step : %1\n").arg(code_transfer_step,2,10) );
                    pTIMER->resetTimer(1);//lastState=pTIMER->state;//time.restart();
                    t=0; c=0;
                    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(15);
                }
                break;
    case 5:     if (pTIMER->msElapsedId(1)>9) {
                    ACK = DOWN;
                    code_transfer_step=INIT_MODE;
                    AddLog(LOG_CONSOLE,tr("step : %1\n").arg(code_transfer_step,2,10) );
                    pTIMER->resetTimer(1);//lastState=pTIMER->state;
                    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(14);
                }
                break;
    case 6:     if ((pTIMER->msElapsedId(1)>2) && (GET_PIN(PIN_BUSY) == UP ) ){
                    ACK = DOWN;
                    code_transfer_step=INIT_MODE;
                    AddLog(LOG_CONSOLE,tr("step : %1\n").arg(code_transfer_step,2,10) );
//                    pTIMER->resetTimer(1);
                    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(19);


                }
                break;
            }

   if ((code_transfer_step==BASIC_MODE) || (code_transfer_step == INIT_MODE)) {

        switch (device_code)
        {
            case 0x00:  // only ce-125 et ce-126p specific mode
            case 0xff:
            case 0x0f:
            case 0x21:
            case 0x45:

                if ( (BUSY == Previous_BUSY ) && (Previous_BUSY == DOWN) &&
                     (MT_OUT1 == Previous_MT_OUT1) &&	(Previous_MT_OUT1 == DOWN) &&
                     (ACK == UP) &&
                     (pTIMER->msElapsedId(1) > 2 ) )
				{
					AddLog(LOG_PRINTER,tr("ACK timeout"));
                    pTIMER->resetTimer(1);//lastState=pTIMER->state;//time.restart();
                    ACK = DOWN;
                    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(1);
				}

                if (SEL1 == UP )
                {
                    t=0;
                    c=0;
                    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(10);
                }

                if (BUSY != Previous_BUSY )	//check for BUSY  - F03
				{
                    //AddLog(LOG_PRINTER,tr("PIN_BUSY CHANGE from %1 TO %2").arg(Previous_PIN_BUSY?"1":"0").arg(GET_PIN(PIN_BUSY)?"1":"0"));
                    Previous_BUSY = BUSY;
                    if (ACK == UP) //check for ACK  - F03
					{
                        ACK = DOWN;
                        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(3);
					}
					else
					{
                        ACK = UP;
                        pTIMER->resetTimer(1);//lastState=pTIMER->state;
                        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(4);
					}
                    //AddLog(LOG_PRINTER,tr("CHANGE ACK TO %1").arg(GET_PIN(PIN_ACK)?"1":"0"));
	
                    if (BUSY == UP )	//check for BUSY  - F03
					{
                        if(D_OUT == UP)
						{
							bit = true;;
						} else
						{
							bit = false;
						}
						t>>=1;
						if (bit) t|=0x80;
						if((c=(++c)&7)==0)
						{
                            AddLog(LOG_CONSOLE,tr("send char to printer : %1").arg(t,2,16) );
							Printer(t);
							t=0; c=0;
                            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(5);
						}
                        else {
                            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(6);
                        }
					} 
				}
                Previous_BUSY = BUSY;
                Previous_MT_OUT1 = MT_OUT1;
				pulldownsignal();
        }
    }

   ce126buf = checkPaper(ce126buf,top);

	pCONNECTOR_value = pCONNECTOR->Get_values();
	pTAPECONNECTOR_value = pTAPECONNECTOR->Get_values();

    Set_Connector();

	return true;
}


Cce123::Cce123()
{								//[constructor]
    BackGroundFname	= P_RES(":/ext/ce-123p.jpg");
    setcfgfname("ce123p");
    setDX(708);
    setDY(566);
    setDXmm(196);
    setDYmm(155);
    setDZmm(33);
    SnapPts = QPoint(215,307);
    setPaperPos(QRect(89,185-149,207,149));

    delete pKEYB; pKEYB=new Ckeyb(this,"ce123.map");
}
bool Cce123::init(void) {
    Cce126::init();
    pCONNECTOR->setSnap(QPoint(215,397));
    return true;
}

Cce129::Cce129()
{								//[constructor]
    BackGroundFname	= P_RES(":/ext/ce-129p.jpg");
    setcfgfname("ce129p");
    setDX(708);
    setDY(566);

    setDXmm(196);
    setDYmm(155);
    setDZmm(33);

    SnapPts = QPoint(88,288);

    setPaperPos(QRect(89,185-149,207,149));
    delete pKEYB; pKEYB=new Ckeyb(this,"ce129.map");

}
bool Cce129::init(void) {
    Cce126::init();
    pCONNECTOR->setSnap(QPoint(88,378));
    return true;
}
C263591::C263591()
{								//[constructor]
    BackGroundFname	= P_RES(":/ext/26-3591.jpg");
    setcfgfname("263591");
    setDX(854);
    setDY(349);
    SnapPts = QPoint(373,0);
    setDXmm(236);
    setDYmm(96);
    setDZmm(33);

    setPaperPos(QRect(78,0,207,149));

    delete pKEYB; pKEYB=new Ckeyb(this,"263591.map");
}
bool C263591::init(void) {
    Cce126::init();
    pCONNECTOR->setSnap(QPoint(373,90));
    return true;
}
