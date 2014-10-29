//FIXME: modify updatefinalimage to draw paper.

#include <QtGui>
#include <QTime>
#include <QFileDialog>
#include <QThread>
#include <QDebug>

#include "common.h"
#include "fluidlauncher.h"

#include "rlp1004a.h"
#include "rlh1000.h"
#include "pcxxxx.h"
#include "Log.h"
#include "watchpoint.h"
#include "paperwidget.h"
#include "Keyb.h"
#include "Connect.h"
#include "buspanasonic.h"

#define DOWN    0
#define UP              1

TransMap KeyMaprlp1004a[]={
    {1, "FEED  ",       K_PFEED,34,234, 9},
    {2, "POWER ON",     K_POW_ON,34,234,        9},
    {3, "POWER OFF",K_POW_OFF,34,234,   9}
};
int KeyMaprlp1004aLenght = 3;

Crlp1004a::Crlp1004a(CPObject *parent):Cprinter(this)
{ //[constructor]
    Q_UNUSED(parent)

    setfrequency( 0);
    paperbuf    = 0;
    paperdisplay= 0;
    //bells             = 0;
    charTable = 0;
    margin = 25;
    BackGroundFname     = P_RES(":/rlh1000/rlp1004a.png");
    setcfgfname("rlp1004a");

    settop(10);
    setposX(0);

    pTIMER              = new Ctimer(this);
    pKEYB               = new Ckeyb(this,"rlp1004a.map");

    setDXmm(113);
    setDYmm(95);
    setDZmm(51);
 // Ratio = 3,57
    setDX(440);//Pc_DX  = 75;
    setDY(340);//Pc_DY  = 20;

//    setPaperPos(QRect(53,0,291,216));
    setPaperPos(QRect(57,-20,318,236));

    ctrl_char = false;
    t = 0;
    c = 0;
    rmtSwitch = false;

    rotate = false;
    INTrequest = false;
    printing = false;
    receiveMode = false;
    CRLFPending = false;

    memsize             = 0x2000;
    InitMemValue        = 0x7f;
    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,  P_RES(":/rlh1000/rlp1004a.bin")    , "" , CSlot::ROM , "Printer ROM"));

}

Crlp1004a::~Crlp1004a() {
    delete paperbuf;
    delete paperdisplay;
    delete pCONNECTOR;
    delete pTAPECONNECTOR;
    delete charTable;
//    delete bells;
}


#define LATENCY (pTIMER->pPC->getfrequency()/3200)
bool Crlp1004a::run(void)
{

    static quint64 _state=0;

    pTAPECONNECTOR->Set_pin(3,true);       // RMT
    pTAPECONNECTOR->Set_pin(2,tapeOutput);    // Out
    tapeInput = pTAPECONNECTOR->Get_pin(1);      // In

    pTAPECONNECTOR_value = pTAPECONNECTOR->Get_values();

    CbusPanasonic bus;

    bus.fromUInt64(pCONNECTOR->Get_values());

    if (printing) {
        if (!buffer.isEmpty()) {
            // Print char column one by one from buffer
            // Wait 20char/sec -> 140 cols / Sec -> 7,142ms / col
            if (pTIMER->usElapsed(_state)>=7142)
            {
                _state = pTIMER->state;
                drawGraph(buffer.at(0));
//                qWarning()<<"Printing data:"<<buffer.at(0)<<":"<<QChar(buffer.at(0));
                buffer.remove(0,1);
            }
        }
        else {
            printing = false;
        }
    }
    else {
        if (CRLFPending) {
            qWarning()<<"1004a: Send CRLF";
            CRLFPending = false;
            top+=10;
            posX=0;
//                bus.setINT(true);
            INTrequest = true;
            pCONNECTOR->Set_values(bus.toUInt64());
            Refresh();
        }
    }

    if (bus.getFunc()==BUS_SLEEP) return true;

    if (bus.getDest()!=0) return true;
    bus.setDest(0);

//    qWarning()<<"PRINTER:"<<bus.toLog();
    if ( (bus.getFunc()==BUS_LINE0) && !bus.isWrite() ) {
        bus.setData(0x00);
        bus.setFunc(BUS_READDATA);
        pCONNECTOR->Set_values(bus.toUInt64());
        if (pPC->pTIMER->pPC->fp_log) fprintf(pPC->pTIMER->pPC->fp_log,"RL-P1004A BUS_QUERY\n");
        return true;
    }

    if ( (bus.getFunc()==BUS_LINE2) && bus.isWrite() ) {
//        qWarning()<<"1004A BUS SELECT:"<<bus.getData();

        switch (bus.getData()) {
        case 1: Power = true; break;
        default: Power = false; break;
        }
        if (Power)
        {
            bus.setFunc(BUS_READDATA);
            bus.setData(0x01);
            pCONNECTOR->Set_values(bus.toUInt64());
        }
        return true;
    }

    if ( (bus.getFunc()==BUS_LINE3) && bus.isWrite() ) {
            switch(bus.getData()) {
            case 0: // Print
                qWarning()<<"1004a: BUS_TOUCH:"<<bus.getData()<<  "PRINTING "<<buffer.size()<<" chars";
    //            Refresh(0);
                printing = true;
    //            buffer.clear();
                INTrequest = false;
                break;
            case 5: //
                qWarning()<<"1004a: BUS_TOUCH:"<<bus.getData();
                buffer.clear();
                receiveMode = true;
                INTrequest = true;
    //            receiveMode = true;
                break;
            case 4: // CR/LF
    //            Refresh(0x0d);
                qWarning()<<"1004a: BUS_TOUCH:"<<bus.getData();

    //            printing = true;
                CRLFPending = true;
    //            INTrequest = true;
                break;
            default: qWarning()<<"1004a: BUS_TOUCH:"<<bus.getData();
                break;
            }
            bus.setFunc(BUS_ACK);
    }

    if ( (bus.getFunc()==BUS_LINE3) && !bus.isWrite() ) {
        if (INTrequest) {
//            qWarning()<<"INTREQUEST:true";
            bus.setINT(true);
            bus.setData(0x00);
            INTrequest = false;
        }
        else {
//            qWarning()<<"INTREQUEST:false";
            bus.setData(0xff);
        }
        bus.setFunc(BUS_READDATA);
        pCONNECTOR->Set_values(bus.toUInt64());
        return true;
       }

    if ( (bus.getFunc()==BUS_LINE1) && bus.isWrite() ) {
        if (receiveMode) {
            buffer.append(bus.getData());
            qWarning()<<"1004a: Receive data:"<<bus.getData();

            INTrequest = true;
        }
        bus.setFunc(BUS_ACK);
    }



    if (!Power) return true;

    quint32 adr = bus.getAddr();
//    quint8 data = bus.getData();

    switch (bus.getFunc()) {
    case BUS_SLEEP: break;
    case BUS_WRITEDATA:
        switch (adr) {
        case 0x3020: // flip flop K7 output
            tapeOutput = !tapeOutput;
//            qWarning()<<pTIMER->state<<" - "<<tapeOutput;
            bus.setData(0x00);
            bus.setFunc(BUS_READDATA);
            break;
        }
        break;


    case BUS_READDATA:
        if ( (adr>=0x2000) && (adr<0x3000) ) bus.setData(mem[adr-0x2000]);
        else if (adr == 0x3060){
            bus.setData(tapeInput? 0x80 : 0x00);
        }
        else bus.setData(0x7f);
        break;
    default: break;

    }



    pCONNECTOR->Set_values(bus.toUInt64());
    return true;

}


void Crlp1004a::SaveAsText(void)
{
    QMessageBox::warning(mainwindow, "PockEmul",
                          tr("This printer is a pure graphic printer (yes it is!!!)\n") +
                          tr("Saving output as text is irrelevant") );
}

void Crlp1004a::drawGraph(quint8 data) {
    QPainter painter;

    painter.begin(paperbuf);
    for (int b=0; b<8;b++)
    {
        if ((data>>b)&0x01) painter.drawPoint( posX, top+b);
    }
    posX++;
    if (posX>320) posX=0;
    painter.end();

    Refresh();
}

void Crlp1004a::Refresh()
{
    QPainter painter;

    painter.begin(paperdisplay);
    painter.drawImage(QRectF(0,MAX(149-top,0),165,MIN(top,159)),*paperbuf,QRectF(0,MAX(0,top-149),170,MIN(top,149)));

    // Draw printer head
    //    painter.fillRect(QRect(0 , 147,407,2),QBrush(QColor(0,0,0)));
    //    painter.fillRect(QRect(21 + (7 * posX) , 137,14,2),QBrush(QColor(128,0,0)));

    painter.end();

    Refresh_Display = true;

    paperWidget->setOffset(QPoint(0,top));
    paperWidget->updated = true;

}


/*****************************************************/
/* Initialize PRINTER                                                            */
/*****************************************************/
void Crlp1004a::clearPaper(void)
{
    // Fill it blank
    paperbuf->fill(PaperColor.rgba());
    paperdisplay->fill(QColor(255,255,255,0).rgba());
    settop(10);
    setposX(0);
    // empty TextBuffer
    TextBuffer.clear();
    paperWidget->updated = true;
}


bool Crlp1004a::init(void)
{
    CPObject::init();

    setfrequency( 0);

    pCONNECTOR  = new Cconnector(this,
                                 44,
                                 0,
                                 Cconnector::Panasonic_44,
                                 "Printer connector",
                                 true,
                                 QPoint(406,72),
                                 Cconnector::EAST);     publish(pCONNECTOR);
    WatchPoint.add(&pCONNECTOR_value,64,44,this,"Printer connector");
    AddLog(LOG_PRINTER,tr("PRT initializing..."));

    pTAPECONNECTOR	= new Cconnector(this,
                                     3,
                                     1,
                                     Cconnector::Jack,
                                     "Line in / Rec",false);	publish(pTAPECONNECTOR);
    WatchPoint.add(&pTAPECONNECTOR_value,64,2,this,"Line In / Rec");

    if(pKEYB)   pKEYB->init();
    if(pTIMER)  pTIMER->init();

    // Create CE-126 Paper Image
    // The final paper image is 207 x 149 at (277,0) for the ce125
    paperbuf    = new QImage(QSize(340, 3000),QImage::Format_ARGB32);
    paperdisplay= new QImage(QSize(200, 149),QImage::Format_ARGB32);


//TODO Update the chartable with upd16434 char table
    charTable = new QImage(P_RES(":/rlh1000/rlp1004atable.bmp"));

//      bells    = new QSound("ce.wav");

// Create a paper widget

    paperWidget = new CpaperWidget(PaperPos(),paperbuf,this);
    paperWidget->updated = true;
    paperWidget->show();

    // Fill it blank
    clearPaper();

    run_oldstate = -1;


    tapeOutput = tapeInput = false;
    return true;
}



/*****************************************************/
/* Exit PRINTER                                                                          */
/*****************************************************/
bool Crlp1004a::exit(void)
{
    AddLog(LOG_PRINTER,"PRT Closing...");
    AddLog(LOG_PRINTER,"done.");
    Cprinter::exit();
    return true;
}


/*****************************************************/
/* CE-126P PRINTER emulation                                             */
/*****************************************************/



#define         WAIT ( pPC->frequency / 10000*6)

#define RECEIVE_MODE    1
#define SEND_MODE               2
#define TEST_MODE               3



bool Crlp1004a::Get_Connector(Cbus *_bus) {
    return true;
}

bool Crlp1004a::Set_Connector(Cbus *_bus) {
    return true;
}





void Crlp1004a::paintEvent(QPaintEvent *event)
{
    CPObject::paintEvent(event);
}

void Crlp1004a::BuildContextMenu(QMenu * menu) {
    Cprinter::BuildContextMenu(menu);
    menu->addAction(tr("Dump Memory"),this,SLOT(Dump()));
    menu->addSeparator();
    menu->addAction(tr("Rotate 180"),this,SLOT(Rotate()));
}

bool Crlp1004a::UpdateFinalImage(void) {
    Cprinter::UpdateFinalImage();
//    QImage _locImg = FinalImage->mirrored(rotate,rotate);
//    delete FinalImage;
//    FinalImage = new QImage(_locImg);
    return true;
}

void Crlp1004a::Rotate()
{
    rotate = ! rotate;
#if 0
    delete BackgroundImageBackup;
    BackgroundImageBackup = CreateImage(QSize(getDX(), getDY()),BackGroundFname,false,false,rotate?180:0);
    delete BackgroundImage;
    BackgroundImage = new QImage(*BackgroundImageBackup);
    delete FinalImage;
    FinalImage = new QImage(*BackgroundImageBackup);

    pCONNECTOR->setSnap(rotate?QPoint(34,72):QPoint(406,72));

    pCONNECTOR->setDir(rotate?Cconnector::WEST:Cconnector::EAST);
    mask = QPixmap::fromImage(*BackgroundImageBackup).scaled(getDX()*mainwindow->zoom/100,getDY()*mainwindow->zoom/100);
    setMask(mask.mask());
#endif
    Refresh_Display = true;
    update();

        // adapt SNAP connector
}


extern int ask(QWidget *parent,QString msg,int nbButton);
#define KEY(c)	((pKEYB->keyPressedList.contains(TOUPPER(c)) || \
                  pKEYB->keyPressedList.contains(c) || \
                  pKEYB->keyPressedList.contains(TOLOWER(c)))?1:0)
void Crlp1004a::ComputeKey(KEYEVENT ke,int scancode)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)

    if (pKEYB->LastKey == K_PFEED) {
        top+=10;
        Refresh();
    }

    // Manage left connector click
    if (KEY(0x240) && (currentView==FRONTview)) {
        pKEYB->keyPressedList.removeAll(0x240);
        FluidLauncher *launcher = new FluidLauncher(mainwindow,
                                     QStringList()<<P_RES(":/pockemul/configExt.xml"),
                                     FluidLauncher::PictureFlowType,QString(),
                                     "Jack_3");
        launcher->show();
    }

}
