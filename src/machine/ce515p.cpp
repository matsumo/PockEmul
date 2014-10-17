
#include <QPainter>
#include <QResource>
#include <QDebug>

#include "common.h"
#include "ce515p.h"
#include "pcxxxx.h"
#include "paperwidget.h"
#include "Keyb.h"
#include "clink.h"
#include "Inter.h"
#include "dialoganalog.h"

extern bool soundEnabled;

#define NO_MOVE	0
#define RI_MOVE	1
#define LE_MOVE 2
#define UP_MOVE 1
#define DN_MOVE 2


#define GET_PIN(n)		pCONNECTOR->Get_pin(n)
#define SET_PIN(n,v)	pCONNECTOR->Set_pin(n,v)

#define DOWN	0
#define UP		1

#define TRANSX(x) (orig_X + x)
#define TRANSY(y) (orig_Y + y)

#define TICKS_BDS	(pTIMER->pPC->getfrequency()/1200)

// 75mm/s => 5*75 step/s
#define PRINTER_TICKS	(pTIMER->pPC->getfrequency()/375)

Cce515p::Cce515p(CPObject *parent):Cprinter(parent)
{
    //[constructor]
    BackGroundFname	= P_RES(":/ext/ce-150.jpg");
    PaperFname		= "ext/ce-150paper.jpg";
    setcfgfname(QString("ce515p"));
    Paper_X = 100;  Paper_DX = 500;
    Paper_Y = 100;
    //PaperWidgetRect = QRect(80,46,167,170);
    setDX(960);
    setDY(320);
    SnapPts = QPoint(388,0);
    //pCONNECTOR	= new Cconnector(this,5,"Internal connector 5 pins",true);	publish(pCONNECTOR);
    pTIMER		= new Ctimer(this);
    KeyMap		= KeyMapce150;
    KeyMapLenght= KeyMapce150Lenght;
    pKEYB		= new Ckeyb(this,"ce150.map",0);

    Print_Mode = 0;

    Pen_X = 0;
    Pen_Y = 0;
    orig_X = 0;
    orig_Y = 100;
    Pen_Z = 0;
    prev_Pen_X = 0;
    prev_Pen_Y = 0;
    prev_Pen_Z = 0;
    Pen_Status = PEN_UP;
    Pen_Color = 0;
    Rot = 0;
//960,320,388,0)
    ce515pbuf=0;
    ce515pdisplay=0;
    needRedraw = true;

    setPaperPos(QRect(75,46,380,170));
#ifndef NO_SOUND
    clac = 0;
#endif

    StartRot = false;
    Change_Color = true;
    Sii_wait			= 0;
    oldstate_run = 0;
    printer_oldstate_draw = 0;
    printer_oldstate_paperfeed=0;
    t=c=0;
    waitbitstart=1;
    waitbitstop=0;

    lastX = 0;
    margin = 0;
}

Cce515p::~Cce515p() {
    delete ce515pbuf;
    delete ce515pdisplay;
    delete ce515ppen;
#ifndef NO_SOUND
    delete clac;
#endif
}

void Cce515p::set_SD(quint8 val) {
    SD = val;
}

void Cce515p::PaperFeed(void) {
    int printer_deltastate=0;
    if (printer_oldstate_paperfeed==0) printer_oldstate_paperfeed	= pTIMER->state;

    printer_deltastate = pTIMER->state - printer_oldstate_paperfeed;
    if (printer_deltastate >= PRINTER_TICKS){
        printer_oldstate_paperfeed	= pTIMER->state;
        ProcessMultiPointCommand("R0,1");
        // Increment origin
        ProcessMultiPointCommand("I");
    }
}

bool Cce515p::run(void)
{
//    bool bit = false;
//    bool has_moved = false;

    quint64 deltastate=0;
    if (oldstate_run == 0)	oldstate_run = pTIMER->state;

    //	0	START BIT
    //	0-1	b0 0=set,1=not set
    //	0-1	b1 0=set,1=not set
    //	0-1	b2 0=set,1=not set
    //	0-1	b3 0=set,1=not set
    //	0-1	b4 0=set,1=not set
    //	0-1	b5 0=set,1=not set
    //	0-1	b6 0=set,1=not set
    //	0-1	b7 0=set,1=not set
    //	1	STOP BIT

        deltastate = pTIMER->state - oldstate_run;
        if (deltastate < Sii_wait){
            return(1);
        }

//        if (!(ER && RS))
//        {
//            oldstate_run	= pTIMER->state;
//            Sii_wait	= 0;
//            return;
//        }
    //	Sii_wait	= TICKS_BDS;
        oldstate_run	= pTIMER->state;
    //	oldstate	+= Sii_wait;


        if (waitbitstop && (SD==0))
        {
            waitbitstop = 0;waitbitstart=1;
    //		Bit STOP
           // AddLog(LOG_PRINTER,tr("STOP BIT"));
            Sii_wait = 0;
        }
        else if (waitbitstart && SD)
        {
            waitbitstart = 0;
    //		Bit START
            //AddLog(LOG_PRINTER,tr("START BIT"));
            Sii_wait	= TICKS_BDS;

        }
        else if (!waitbitstart)
        {
            t>>=1;
            if(SD==0) t|=0x80;
           // AddLog(LOG_PRINTER,tr("Bit = %1").arg(SD));
            if((c=(++c)&7)==0)
            {
                AddLog(LOG_PRINTER,tr("Byte = %1").arg(t,2,16,QChar('0')));
                //ExportByte(t);
                Command(t);

                t=0;
                waitbitstop = 1;
            }
        }


    //---------------------------------------------------
    // Draw printer
    //---------------------------------------------------
    //if (has_moved) Print();
#if 0
        printer_deltastate = pTIMER->state - printer_oldstate;
        if (printer_deltastate >= PRINTER_TICKS){
            printer_oldstate	= pTIMER->state;
            if (moveBuffer.length()>0) {
                Print(moveBuffer.at(0));
                moveBuffer.removeFirst();
            }
        }
#else
        Draw();
#endif

    return(1);
}

void Cce515p::Draw(void) {
    int printer_deltastate=0;
    if (printer_oldstate_draw==0) printer_oldstate_draw = pTIMER->state;
    //---------------------------------------------------
    // Draw printer
    //---------------------------------------------------
    //if (has_moved) Print();
        printer_deltastate = pTIMER->state;// - printer_oldstate_draw;
//        if (printer_deltastate >= PRINTER_TICKS){
        if (pTIMER->usElapsed(printer_oldstate_draw)>3846) {    // 52mm/s  1step=.2mm -> 260steps/s  3,846ms/step
            printer_oldstate_draw	= pTIMER->state;
            if (moveBuffer.length()>0) {
                lastX = moveBuffer.at(0).X;
                Print(moveBuffer.at(0));
                moveBuffer.removeFirst();

                // Expand paper size if limit reached
                ce515pbuf = checkPaper(ce515pbuf,Pen_Y);
            }
        }
}

bool Cce515p::Next_Color(void)
{
    Pen_Color++;
    Change_Color = true;
    if (Pen_Color == 4)
    {
        Pen_Color = 0;
        return(true);
    }
    return(false);
}


void Cce515p::clearPaper(void)
{
    // Fill it blank
    ce515pbuf->fill(PaperColor.rgba());
    ce515pdisplay->fill(QColor(0,0,0,0).rgba());
    Pen_Y = 0;
}

bool Cce515p::exit(void)
{
#if 0
    FSOUND_Sample_Free(clac);
#endif

    CPObject::exit();

    return true;
}

bool Cce515p::init(void)
{
    CPObject::init();
#ifndef NO_SOUND
    clac = new QSound(":/ext/clac2.wav");
#endif

    setfrequency( 0);

    //WatchPoint.add(&pCONNECTOR_value,8,5,this,"Internal connector");

    AddLog(LOG_PRINTER,tr("PRT initializing..."));

    if(pKEYB)	pKEYB->init();
    if(pTIMER)	pTIMER->init();

    // Create CE-150 Paper Image
    ce515pbuf	= new QImage(QSize(Paper_DX, 600),QImage::Format_ARGB32);
    ce515pdisplay= new QImage(QSize(Paper_DX, 567),QImage::Format_ARGB32);
    ce515ppen	= new QImage(P_RES(":/ext/ce-150pen.png"));
    // Fill it blank
    clearPaper();

    // Create a paper widget
    paperWidget = new CpaperWidget(pos,ce515pbuf,pTIMER->pPC);
    paperWidget->show();

    Sii_wait = 0;
    Pen_Y=20;//-100;
    charSize = 2;
    escMode = false;
    mode = TEXT;
    orig_X = 0;
    orig_Y = 0;
    lineType = 0;
    mainRot = 0;

    initsound();
    return(1);

}

void Cce515p::Print(CMove point)
{
    static bool old_penDown = true;
    QPainter painter;

    pPC->Refresh_Display = true;

    if (point.changeColor) {
        Pen_Color = point.newColor;
    }
    else {
        if (point.penDown)
        {
            painter.begin(ce515pbuf);
            switch (Pen_Color)
            {
                case 0 : painter.setPen( Qt::black); break;
                case 1 : painter.setPen( Qt::blue); break;
                case 2 : painter.setPen( Qt::green); break;
                case 3 : painter.setPen( Qt::red); break;
            }
            painter.drawPoint( point.X + margin, point.Y );
            painter.end();

        }
        // Check is pen up/down status change to play the CLAC
        if (point.penDown != old_penDown) {
#ifndef NO_SOUND
//            clac->play();
            if (soundEnabled) {
                if (getfrequency()>0) {
                    fillSoundBuffer(0xFF);
                    fillSoundBuffer(0x00);
                }
                else {
                    //                int ps = m_audioOutput->periodSize();
                    mainwindow->audioMutex.lock();
                    QByteArray buff;
                    buff.append(0xFF);
                    buff.append((char)0);
                    m_output->write(buff.constData(),2);
                    mainwindow->audioMutex.unlock();
                }
            }
#endif
            old_penDown = point.penDown;
        }
        paperWidget->setOffset(QPoint(0,point.Y));
        pPC->Refresh_Display = true;
    }



}

void Cce515p::SaveAsText(void)
{
    QMessageBox::warning(mainwindow, "PockEmul",
                         tr("This printer is a pure graphic printer\n") +
                         tr("Saving output as text is irrelevant") );
}

#define BS  0x08
#define LF  0x0A
#define LU  0x0B
#define CR  0x0D
#define DC1 0x11
#define DC2 0x12

void Cce515p::Command(quint8 t) {
    if (escMode) {
        escCommand+=QChar(t);
        AddLog(LOG_PRINTER,"Esc Command:"+escCommand);

        switch (TOLOWER(t)) {
            case 'a' :
            case 'b' :
            case 'c' :
            case 'd' :
            case 'e' :
            case 'f' :
            case 'g' :
            case 'h' :
            case 'i' :
            case 'j' :
            case 'k' :
            case 'l' :
            case 'm' :
            case 'n' :
            case 'o' :
            case '0' :
            case '1' :
            case '2' :
            case '3' :  ProcessEscCommand();
                        escCommand="";
                        escMode=false;
                        return;
                        break;
        }

    }

    switch (mode) {
    case TEXT :
        switch (t) {
            case 0x08: ProcessMultiPointCommand(QString("R%1,0").arg(-6*charSize)); break;
            case 0x0A: ProcessMultiPointCommand(QString("R0,%1").arg(12*charSize));
                //Pen_Y+=(12*charSize);
                break;
            case 0x0B: ProcessMultiPointCommand(QString("R0,%1").arg(-12*charSize));
                //Pen_Y-=(12*charSize);
                break;
            case 0x0D:  ProcessMultiPointCommand(QString("R0,%1").arg(12*charSize));
                        ProcessMultiPointCommand(QString("R%1,0").arg(-1*TRANSX(Pen_X)));

                        //Pen_Y+=(12*charSize); Pen_X = 0;
                break;
            case 0x1B: escMode = true; break;
            case DC1:   break;
            case DC2:   mode = GRAPH; break;
            default: drawChar(t);break;
        }
        break;
    case GRAPH :
        switch (t) {
            case DC1:   mode = TEXT;
                        orig_X = 0 ;            Pen_X = 0;
                        orig_Y = TRANSY(Pen_Y); Pen_Y = 0;

                        break;
            case 0x1B: escMode = true; break;
            case 0x0D:
                    if (escMode) {
                        ProcessEscCommand();
                        escCommand="";
                        escMode=false;
                        return;
                    }
                    AddLog(LOG_PRINTER,tr("Pen_X = %1   Pen_Y = %2").arg(TRANSX(Pen_X),5,10,QChar('0')).arg(TRANSY(Pen_Y),5,10,QChar('0')));
                        ProcessGraphCommand();
                        graphCommand = "";
                        break;
            default:    graphCommand+=QChar(t);
                        break;
        }
    }

}

void Cce515p::ProcessGraphCommand() {
    bool ok;
    AddLog(LOG_PRINTER,"Graph Command:"+graphCommand);
    switch (graphCommand.at(0).toUpper().toLatin1()) {
    case 'S':   { //size 0-15 (Canon x-710)
                    qint8 tmpcharSize = graphCommand.mid(1).toInt() +1;
                    if ((tmpcharSize >0) && (tmpcharSize <= 16))
                        charSize = tmpcharSize;
                }
                break;
    case 'C':   moveBuffer.append( CMove(Pen_Color,graphCommand.mid(1).toInt(0,10)));
                break;
    case 'A' : // Back to TEXT Mode
                mode = TEXT;
                orig_X = 0 ;            Pen_X = 0;
                orig_Y = TRANSY(Pen_Y); Pen_Y = 0;
                break;
    case 'D' : // Trace absolute figure

    case 'J' : // Trace relative figure
    case 'M' : // Absolute Move position (penDown=false), if P print point
    case 'R' : // Relative move position (penDown=false), if P print point
                ProcessMultiPointCommand(graphCommand);
                break;
    case 'L' : // Traits pointillé
                lineType = graphCommand.mid(1).toInt(&ok,10);
                break;
    case 'I' : // Define current position as origin
                orig_X = TRANSX(Pen_X); Pen_X = 0;
                orig_Y = TRANSY(Pen_Y); Pen_Y = 0;
                break;
    case 'P' : // Print character
                for (int i=1; i < graphCommand.size(); i++) {
                    drawChar(graphCommand.at(i).toLatin1(),mainRot);
                }
                break;
    case 'Q' : // Main rotation
                mainRot = graphCommand.mid(1,1).toInt(&ok,10);
                break;
    }
}

void Cce515p::DrawTest(void) {
    //Pen_X = 0;
    //orig_X = 0;
    Command(0x0D);
    //ProcessMultiPointCommand("M0,0");
    moveBuffer.append( CMove(Pen_Color,0));
    Pen_Color = 0;

    ProcessMultiPointCommand("J0,25,25,0,0,-25,-25,0,0,25,25,0,0,-25,-25,0");
    ProcessMultiPointCommand("R30,0");
    moveBuffer.append( CMove(Pen_Color,1));
    Pen_Color = 1;
    ProcessMultiPointCommand("J0,25,25,0,0,-25,-25,0,0,25,25,0,0,-25,-25,0");
    ProcessMultiPointCommand("R30,0");
    moveBuffer.append( CMove(Pen_Color,2));
    Pen_Color = 2;
    ProcessMultiPointCommand("J0,25,25,0,0,-25,-25,0,0,25,25,0,0,-25,-25,0");
    ProcessMultiPointCommand("R30,0");
    moveBuffer.append( CMove(Pen_Color,3));
    Pen_Color = 3;
    ProcessMultiPointCommand("J0,25,25,0,0,-25,-25,0,0,25,25,0,0,-25,-25,0");
    //ProcessMultiPointCommand("R0,40");
    moveBuffer.append( CMove(Pen_Color,0));
    Pen_Color = 0;
    //Pen_X = 0;
    //Pen_Y += 40;
    Command(0x0D);
    Command(0x0D);
}



void Cce515p::ProcessMultiPointCommand(QString command) {
    bool ok;
    AddLog(LOG_PRINTER,command);
    QStringList coordList = command.mid(1).split(",");
    //AddLog(LOG_PRINTER,tr("size = %1").arg(coordList.size(),2,16,QChar('0')));
    if (!(coordList.size() % 2)) { // we need a odd parameters count
        for (int ind = 0; ind < coordList.size(); ind += 2)
        {
            //QLocale::setDefault(QLocale::C);
            QLocale c(QLocale::C);
            //int d = c.toFloat("501.5", &ok );
            int x = c.toFloat(coordList.at(ind),&ok);
            int y = c.toFloat(coordList.at(ind+1),&ok);
            //int x = coordList.at(ind).toInt(&ok,10);
            //int y = coordList.at(ind+1).toInt(&ok,10);
            //AddLog(LOG_PRINTER,tr("test convert 501 =%1").arg(d));
            //AddLog(LOG_PRINTER,"first param:"+coordList.at(ind));
            //AddLog(LOG_PRINTER,"second param:"+coordList.at(ind+1));
            AddLog(LOG_PRINTER,tr("draw to [%1,%2]").arg(TRANSX(x),4,10,QChar('0')).arg(TRANSY(y),4,10,QChar('0')));
            switch (command.at(0).toUpper().toLatin1()) {
            case 'D' :  penDown = true;
                        DrawLine(TRANSX(Pen_X),TRANSY(Pen_Y),TRANSX(x),TRANSY(y));
                        Pen_X = (x);
                        Pen_Y = (y);
                        break;
            case 'J' :  penDown = true;
                        DrawLine(TRANSX(Pen_X),TRANSY(Pen_Y),TRANSX(Pen_X+x),TRANSY(Pen_Y+y));
                        Pen_X = (Pen_X+x);
                        Pen_Y = (Pen_Y+y);
                        break;
            case 'M' :  penDown = false;
                        DrawLine(TRANSX(Pen_X),TRANSY(Pen_Y),TRANSX(x),TRANSY(y));
                        Pen_X = (x);
                        Pen_Y = (y);
                        break;
            case 'R' :  penDown = false;
                        DrawLine(TRANSX(Pen_X),TRANSY(Pen_Y),TRANSX(Pen_X+x),TRANSY(Pen_Y+y));
                        Pen_X = (Pen_X+x);
                        Pen_Y = (Pen_Y+y);
                        break;
            }


        }
    }
}

void Cce515p::ProcessEscCommand() {
    qint8 tmpcharSize = 0;
    switch (escCommand.at(0).toUpper().toLatin1()) {
    case '?': //size 2 mode a-o or 1- 63 (for ce-140p)

        if ( (escCommand.at(1).toLatin1()>=97) && (escCommand.at(1).toLatin1()<=111) )
            charSize = escCommand.at(1).toLatin1()-96;
        else { // CE-140P Mode
            tmpcharSize = escCommand.mid(1).toInt();
            if ((tmpcharSize >0) && (tmpcharSize < 64))
                charSize = tmpcharSize;
        }

        break;
    case 'A': // text mode
        mode = TEXT;
        orig_X = 0 ;            Pen_X = 0;
        orig_Y = TRANSY(Pen_Y); Pen_Y = 0;
        break;
    case 'G': // TEST
        DrawTest();
        break;
    case 'B': // graphic mode
        if (mode == TEXT) {
            mode = GRAPH;
            orig_X = TRANSX(Pen_X); Pen_X = 0;
            orig_Y = TRANSY(Pen_Y); Pen_Y = 0;
        }
        break;
    case '0':
    case '1':
    case '2':
    case '3': // change color
        bool ok;
        //Pen_Color = escCommand.toInt(&ok,10);
        moveBuffer.append( CMove(Pen_Color,escCommand.toInt(&ok,10)));
        break;

    }
}

void Cce515p::drawChar(quint8 data, qint8 rot) {
    QString str = graph[data];
    int origX = Pen_X;
    int origY = Pen_Y;
    qint8 finalDir;

    AddLog(LOG_PRINTER,str);
    // parse str
    QStringList list = str.split(" ");
    // Fetch QStringList
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
    {
        QString step = (QString)*it;
        bool ok;
        qint8 val = step.toInt(&ok, 16);
        switch (val & 0xC0) {
        case 0x80 : // angular mode
            {
                int l = (val & 0x07) * charSize;
                int d = (val>>3) & 0x07;
                dir = dir+(d==0)-(d==4);
                if (dir >7) dir=0;
                if (dir <0) dir=7;
                finalDir= dir - (rot*2);
                if (finalDir <0) finalDir += 8;
                DrawMove(charSize,finalDir,true);
                dir = dir+(d==0)-(d==4);
                if (dir >7) dir=0;
                if (dir <0) dir=7;
                finalDir= dir - (rot*2);
                if (finalDir <0) finalDir += 8;
                DrawMove(l,finalDir,true);
            }
            break;
        default : // Standard mode
                lenght = (val & 0x07) * charSize;
                dir = (val>>3) & 0x07;
                finalDir= dir - (rot*2);
                if (finalDir <0) finalDir += 8;
                penDown = val & 0x40;
                DrawMove(lenght,finalDir,penDown);
                break;
        }
    }
    //Pen_X = origX + (6 * charSize);
    //Pen_Y = origY;
    Pen_X=origX + (rot==0)*(6*charSize) + (rot==2)*(-6*charSize);
    Pen_Y=origY + (rot==1)*(6*charSize) + (rot==3)*(-6*charSize);

    if ( (Pen_X + (6*charSize)) >= (Paper_DX-2*margin)) {
        Command(0x0D);
//        Pen_X = 0;
//        Pen_Y += 12*charSize;
    }
    ProcessMultiPointCommand(QString("M%1,%2").arg(Pen_X).arg(Pen_Y));
}

void Cce515p::DrawMove(int lenght,int dir,bool penDown) {


    for (int step=0;step < lenght;step++) {
        moveBuffer.append( CMove(TRANSX(Pen_X),TRANSY(Pen_Y),penDown));
        switch (dir) {
            case 0 : Pen_X++; break;
            case 1 : Pen_X++; Pen_Y--; break;
            case 2 : Pen_Y--; break;
            case 3 : Pen_X--; Pen_Y--; break;
            case 4 : Pen_X--; break;
            case 5 : Pen_X--; Pen_Y++; break;
            case 6 : Pen_Y++; break;
            case 7 : Pen_X++; Pen_Y++; break;
        }

        moveBuffer.append( CMove(TRANSX(Pen_X),TRANSY(Pen_Y),penDown));

    }
}

const QString Cce515p::graph[] = {
    /*0x00*/        "","","","","","","","",
    /*0x08*/        "","","","","","","","",
    /*0x10*/        "","","","","","","","",
    /*0x18*/        "","","","","","","","",
    /*0x20*/        "C0",
                    "02 51 12 D3",
                    "14 01 52 02 F2",
                    "01 56 02 76 52 41 64 12 C4",
                    "12 79 42 49 59 62 59 49 42 79 1A F6",
                    "11 4C 24 51 41 71 61 3C 61 71 41 D1",
                    "04 5C 51 A1 79 6B 71 81 CA",
                    "C0",
    /*0x28*/        "03 5A 52 CA",
                    "01 4A 52 DA",
                    "02 56 21 29 7C 14 EC",
                    "13 44 1A F4",
                    "31 49 52 61 71 C1",
                    "13 C4",
                    "01 41 51 61 71 C1",
                    "11 CC",
    /*0x30*/        "11 79 42 84 82 84 CC",
                    "01 42 61 56 E9",
                    "4C 51 82 69 35 C4",
                    "11 79 42 81 82 42 81 82 E9",
                    "03 56 6B 71 C4",
                    "11 79 42 82 83 52 C4",
                    "13 A2 A2 A2 A4 A2 F9",
                    "4C 52 E4",
    /*0x38*/        "01 42 81 82 42 81 82 81 79 69 71 F9",
                    "11 79 42 84 82 81 82 C9",
                    "09 41 51 61 71 13 41 51 61 F1",
                    "39 49 52 61 71 41 13 51 61 71 C1",
                    "03 5B CB",
                    "12 44 12 E4",
                    "01 4B DB",
                    "02 51 11 51 41 81 82 69 F1",
    /*0x40*/        "04 63 A4 A2 A4 59 52 61 81 81 C9",
                    "54 4A 7A 74 52 E4",
                    "43 81 83 43 81 83 F6",
                    "0C 11 82 84 82 C9",
                    "43 84 83 F6",
                    "44 16 64 76 53 C3",
                    "56 44 33 21 E3",
                    "0A 11 42 73 51 69 62 A4 A2 F9",
    /*0x48*/        "56 73 44 53 F6",
                    "01 42 61 56 41 E2",
                    "11 79 41 85 41 E2",
                    "56 04 6C 49 FB",
                    "44 64 D6",
                    "56 7A 4A F6",
                    "56 71 7C 71 D6",
                    "11 54 A2 A4 A2 D9",
    /*0x50*/        "56 43 A1 69 E3",
                    "0A 7A 55 82 84 82 C9",
                    "56 43 A1 A3 41 FB",
                    "11 79 42 81 82 A1 A2 F9",
                    "02 56 42 E4",
                    "16 75 82 49 D5",
                    "16 74 7A 4A D4",
                    "16 76 4A 51 71 7A D6",
    /*0x58*/        "51 4C 51 36 51 5C D1",
                    "02 53 5A 51 04 71 EA",
                    "04 64 51 4C 51 E4",
                    "12 7A 56 C2",
                    "02 54 5A 04 6A 62 44 32 E4",
                    "49 55 61 44 61 75 F9",
                    "12 51 4A 7A F1",
                    "C0",
    /*0x60*/        "C0",
                    "04 59 53 62 82 81 C9",
                    "56 74 4A 41 A2 69 E3",
                    "0B 01 59 62 82 82 C9",
                    "13 72 83 56 74 5A 61 E9",
                    "03 62 A2 A1 A1 E3",
                    "02 53 41 62 41 52 49 F9",
                    "31 42 83 81 81 81 C9",
    /*0x68*/        "56 73 49 41 79 F3",
                    "02 53 12 D1",
                    "79 41 49 54 61 12 C1",
                    "56 01 3A 6B 49 FA",
                    "02 D6",
                    "53 49 79 73 53 49 79 F3",
                    "54 72 4A 41 79 F3",
                    "01 42 82 82 82 F9",
    /*0x70*/        "31 55 43 A1 69 E3",
                    "03 39 55 63 81 79 C3",
                    "54 72 4A FA",
                    "11 79 41 49 59 61 59 49 41 F9",
                    "14 42 19 74 79 C9",
                    "14 73 81 4A 52 F4",
                    "14 72 7A 4A D2",
                    "14 73 79 49 53 73 79 49 D3",
    /*0x78*/        "4C 24 FC",
                    "39 42 84 72 A2 59 D2",
                    "14 44 6C C4",
                    "C0",
                    "C0",
                    "C0",
                    "13 49 7A C9",
                    "",
    /*0x80*/        "","","","","","","","",
    /*0x88*/        "","","","","","","","",
    /*0x90*/        "","","","","","","","",
    /*0x98*/        "","","","","","","","",
    /*0xA0*/        "","","","","","","","",
    /*0xA8*/        "","","","","","","","",
    /*0xB0*/        "","","","","","","","",
    /*0xB8*/        "","","","","","","","",
    /*0xC0*/        "","","","","","","","",
    /*0xC8*/        "","","","","","","","",
    /*0xD0*/        "","","","","","","","",
    /*0xD8*/        "","","","","","","","",
    /*0xE0*/        "","","","","","","","",
    /*0xE8*/        "","","","","","","","",
    /*0xF0*/        "","","","","","","","",
    /*0xF8*/        "","","","","","","",""
                };


#if 1
// Algorithme de Brensenham
// ATTENTION , tenir compte du pointill

void Cce515p::DrawLine(int xa, int ya, int xb, int yb) {
/* pour tracer une droite a partir de 2 points : (xa,ya) et (xb,yb) */
 int x,y,dx,dy,incrmX,incrmY,dp,NE,SE;
 bool local_penDown;

 local_penDown = penDown;
 dx = xb-xa;
 dy = yb-ya;

 if (dx>0)
  incrmX = 1;
 else {
  incrmX = -1;
  dx *= -1;
 }

 if (dy>0)
  incrmY = 1;
 else {
  incrmY = -1;
  dy *= -1;
 }


 if (dx>=dy) {
  dp=2*dy-dx;
  SE=2*dy;
  NE=2*(dy-dx);

  y=ya;
  for(x=xa;x!=xb;x+=incrmX) {
   /* affichePixel(x,y,couleur) */
   //XDrawPoint(display,win,gc,x, y);
      moveBuffer.append( CMove(x,y,local_penDown));

   if (dp<=0) /* on choisit le pixel E */
    dp += SE;
   else {
    dp += NE;
    y+=incrmY;
       }
  }
 }
 else if (dx<dy) {
  dp=2*dx-dy;
  SE=2*dx;
  NE=2*(dx-dy);

  x=xa;
  for(y=ya;y!=yb;y+=incrmY) {
   /* affichePixel(x,y,couleur) */
   //XDrawPoint(display,win,gc,x, y);
      moveBuffer.append( CMove(x,y,local_penDown));

   if (dp<=0) /* on choisit le pixel E */
    dp += SE;
   else {
    dp += NE;
    x+=incrmX;
   }
  }
 }
} /* Brensenham */

#endif
