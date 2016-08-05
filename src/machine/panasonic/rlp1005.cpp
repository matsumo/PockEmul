//FIXME: modify updatefinalimage to draw paper.

#include <QtGui>
#include <QTime>
#include <QFileDialog>
#include <QThread>
#include <QDebug>

#include <math.h>

#include "common.h"
#include "fluidlauncher.h"

#include "rlp1005.h"
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

#include "renderView.h"

extern CrenderView* view;

Crlp1005::Crlp1005(CPObject *parent):Cprinter(parent)
{ //[constructor]
    Q_UNUSED(parent)

    setfrequency( 0);
    paperbuf    = 0;
    paperdisplay= 0;
    //bells             = 0;
    charTable = 0;
    margin = 25;
    BackGroundFname     = P_RES(":/rlh1000/rlp1005.png");
    setcfgfname("rlp1005");

    settop(10);
    setposX(0);

    pTIMER              = new Ctimer(this);
    pKEYB               = new Ckeyb(this,"rlp1005.map");

    setDXmm(299);
    setDYmm(219);
    setDZmm(30);
 // Ratio = 3,57
    setDX(1068);
    setDY(781);

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
    bank = 0;

    memsize             = 0x22000;
    InitMemValue        = 0x7f;
    SlotList.clear();

    SlotList.append(CSlot(16 , 0x0000 , ""  , ""        , CSlot::CUSTOM_ROM , "ROM bank 1"));
    SlotList.append(CSlot(16 , 0x4000 , ""  , ""        , CSlot::CUSTOM_ROM , "ROM bank 2"));
    SlotList.append(CSlot(16 , 0x8000 , ""  , ""        , CSlot::CUSTOM_ROM , "ROM bank 3"));
    SlotList.append(CSlot(16 , 0xC000 , ""  , ""        , CSlot::CUSTOM_ROM , "ROM bank 4"));
    SlotList.append(CSlot(16 , 0x10000 , "" , ""        , CSlot::CUSTOM_ROM , "ROM bank 5"));
    SlotList.append(CSlot(16 , 0x14000 , "" , ""        , CSlot::CUSTOM_ROM , "ROM bank 6"));
    SlotList.append(CSlot(16 , 0x18000 , "" , ""        , CSlot::CUSTOM_ROM , "ROM bank 7"));
    SlotList.append(CSlot(16 , 0x1C000 , "" , ""        , CSlot::CUSTOM_ROM , "ROM bank 8"));
    SlotList.append(CSlot(8  , 0x20000 ,  P_RES(":/rlh1000/rlp1005.bin")    , "" , CSlot::ROM , "Printer ROM"));

}

Crlp1005::~Crlp1005() {
    delete paperbuf;
    delete paperdisplay;
    delete pCONNECTOR;
    delete pTAPECONNECTOR;
    delete charTable;
//    delete bells;
}


#define LATENCY (pTIMER->pPC->getfrequency()/3200)
bool Crlp1005::run(void)
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
        if (pTIMER->pPC->fp_log) fprintf(pTIMER->pPC->fp_log,"RL-P1004A BUS_QUERY\n");
        return true;
    }
    if ( (bus.getFunc()==BUS_LINE1) && !bus.isWrite() ) {
        qWarning()<<"Read data LINE 1:";
        bus.setFunc(BUS_ACK);
    }
    if ( (bus.getFunc()==BUS_LINE2) && !bus.isWrite() ) {
//        qWarning()<<"Read data LINE 2:";
        // BUSY ?
        bus.setData(rand()&0x01);
        bus.setFunc(BUS_READDATA);
        pCONNECTOR->Set_values(bus.toUInt64());
        return true;
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

    if ( (bus.getFunc()==BUS_LINE0) && bus.isWrite() ) {
        qWarning()<<"1005: write BUS_LINE0:"<<bus.getData();
        bus.setFunc(BUS_ACK);
    }
    if ( (bus.getFunc()==BUS_LINE1) && bus.isWrite() ) {
        qWarning()<<"1005: write BUS_LINE1:"<<bus.getData();
        if (receiveMode) {
            buffer.append(bus.getData());


            INTrequest = true;
        }
        bus.setFunc(BUS_ACK);
    }
    if ( (bus.getFunc()==BUS_LINE2) && bus.isWrite() ) {
//        qWarning()<<"1004A BUS SELECT:"<<bus.getData();

        switch (bus.getData()) {
        case 1: Power = true; bank = 7; break;
        case 2: Power = true; bank = 6;  break;
        case 4: Power = true; bank = 5;  break;
        case 8: Power = true; bank = 4;  break;
        case 16: Power = true; bank = 3;  break;
        case 32: Power = true; bank = 2;  break;
        case 64: Power = true; bank = 1;  break;
        case 128: Power = true; bank = 0;  break;
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
            qWarning()<<"1005: write BUS_LINE3:"<<bus.getData()<<  "  PRINTING "<<buffer.size()<<" chars";
//            Refresh(0);
            printing = true;
//            buffer.clear();
            INTrequest = false;
            break;
        case 5: //
            qWarning()<<"1005: write BUS_LINE3:"<<bus.getData();
            buffer.clear();
            receiveMode = true;
            INTrequest = true;
//            receiveMode = true;
            break;
        case 4: // CR/LF
//            Refresh(0x0d);
            qWarning()<<"1005: write BUS_LINE3:"<<bus.getData();

//            printing = true;
            CRLFPending = true;
//            INTrequest = true;
            break;
        default: qWarning()<<"1005: write BUS_LINE3:"<<bus.getData();
            break;
        }
        bus.setFunc(BUS_ACK);

    }



    if (!Power) return true;

    quint32 adr = bus.getAddr();
//    quint8 data = bus.getData();

    switch (bus.getFunc()) {
    case BUS_SLEEP: break;
    case BUS_ACK: break;
    case BUS_WRITEDATA:
        qWarning()<<tr("write:%1=%2").arg(adr,4,16,QChar('0')).arg(bus.getData(),2,16,QChar('0'));
        break;

    case BUS_READDATA:
        if ( (adr>=0x2000) && (adr<0x3000) ) {
            bus.setData(mem[0x20000 + adr - 0x2000]);
//            qWarning()<<tr("read:%1=%2").arg(adr,4,16,QChar('0')).arg(bus.getData(),2,16,QChar('0'));
        }
        else if ( (adr>=0x4000) && (adr < 0x8000)) {
//            qWarning()<<"ROM SIMUL:"<<adr<<"="<<mem[adr-0x4000+0x14];
            bus.setFunc(BUS_ACK);
            bus.setData(mem[adr-0x4000+bank*0x4000]);
        }
        //        else if (adr == 0x3060){
        //            bus.setData(tapeInput? 0x80 : 0x00);
        //        }
        else bus.setData(0x7f);
        break;
    default: break;

    }



    pCONNECTOR->Set_values(bus.toUInt64());
    return true;

}


void Crlp1005::SaveAsText(void)
{
    QMessageBox::warning(mainwindow, "PockEmul",
                          tr("This printer is a pure graphic printer (yes it is!!!)\n") +
                          tr("Saving output as text is irrelevant") );
}

void Crlp1005::drawGraph(quint8 data) {
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

void Crlp1005::Refresh()
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
void Crlp1005::clearPaper(void)
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


bool Crlp1005::init(void)
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
bool Crlp1005::exit(void)
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



bool Crlp1005::Get_Connector(Cbus *_bus) {
    return true;
}

bool Crlp1005::Set_Connector(Cbus *_bus) {
    return true;
}





void Crlp1005::paintEvent(QPaintEvent *event)
{
    CPObject::paintEvent(event);
}

void Crlp1005::BuildContextMenu(QMenu * menu) {
    Cprinter::BuildContextMenu(menu);
    menu->addAction(tr("Dump Memory"),this,SLOT(Dump()));
    menu->addSeparator();
    menu->addAction(tr("Rotate 180"),this,SLOT(Rotate()));
}


void Crlp1005::Rotate()
{
    rotate = ! rotate;

    delete BackgroundImageBackup;
    BackgroundImageBackup = CreateImage(QSize(getDX(), getDY()),BackGroundFname,false,false,rotate?180:0);
    delete BackgroundImage;
    BackgroundImage = new QImage(*BackgroundImageBackup);
    delete FinalImage;
    FinalImage = new QImage(*BackgroundImageBackup);

    pCONNECTOR->setSnap(rotate?QPoint(34,72):QPoint(406,72));

    pCONNECTOR->setDir(rotate?Cconnector::WEST:Cconnector::EAST);
    mask = QPixmap::fromImage(*BackgroundImageBackup).scaled(getDX()*mainwindow->zoom,
                                                             getDY()*mainwindow->zoom);
    setMask(mask.mask());


    update();

        // adapt SNAP connector
}


extern int ask(QWidget *parent,QString msg,int nbButton);

void Crlp1005::ComputeKey(KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)

   if (pKEYB->LastKey == K_PFEED) {
        top+=10;
        Refresh();
    }

    // Manage left connector click
    if (KEY(0x240) && (currentView==FRONTview)) {
        pKEYB->keyPressedList.remove(0x240);
//        FluidLauncher *launcher = new FluidLauncher(mainwindow,
//                                     QStringList()<<P_RES(":/pockemul/configExt.xml"),
//                                     FluidLauncher::PictureFlowType,QString(),
//                                     "Jack_3");
//        launcher->show();

        view->pickExtensionConnector("Jack_3");
    }

}

