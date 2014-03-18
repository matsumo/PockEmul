
//TODO: Key management
#include <QDebug>

#include <QPainter>

#include "hp82143A.h"
#include "Connect.h"
#include "Keyb.h"
#include "dialogconsole.h"
#include "init.h"
#include "Inter.h"
#include "Log.h"
#include "paperwidget.h"
#include "bus.h"

#define DOWN	0
#define UP		1

quint8 hp82143a_chargen [128][5]= {
  { 0x08, 0x1c, 0x3e, 0x1c, 0x08 },  // undefined - use diamond
  { 0x00, 0x14, 0x08, 0x14, 0x00 },  // small x
  { 0x44, 0x29, 0x11, 0x29, 0x44 },  // x-bar
  { 0x08, 0x1c, 0x2a, 0x08, 0x08 },  // left arrow
  { 0x38, 0x44, 0x44, 0x38, 0x44 },  // LC alpha
  { 0x7e, 0x15, 0x25, 0x25, 0x1a },  // UC beta
  { 0x7f, 0x01, 0x01, 0x01, 0x03 },  // UC gamma
  { 0x10, 0x30, 0x7f, 0x30, 0x10 },  // down arrow
  { 0x60, 0x18, 0x06, 0x18, 0x60 },  // UC delta
  { 0x38, 0x44, 0x44, 0x3c, 0x04 },  // LC sigma
  { 0x08, 0x1c, 0x3e, 0x1c, 0x08 },  // diamond
  { 0x62, 0x14, 0x08, 0x10, 0x60 },  // LC lambda
  { 0x40, 0x3c, 0x20, 0x20, 0x1c },  // LC mu
  { 0x60, 0x50, 0x58, 0x64, 0x42 },  // angle
  { 0x10, 0x18, 0x78, 0x04, 0x02 },  // LC tau
  { 0x08, 0x55, 0x77, 0x55, 0x08 },  // UC phi
  { 0x3e, 0x49, 0x49, 0x49, 0x3e },  // UC theta
  { 0x5e, 0x61, 0x01, 0x61, 0x5e },  // UC omega
  { 0x30, 0x4a, 0x4d, 0x49, 0x30 },  // LC delta
  { 0x78, 0x14, 0x15, 0x14, 0x78 },  // UC A dot
  { 0x38, 0x44, 0x45, 0x3e, 0x44 },  // LC a dot
  { 0x78, 0x15, 0x14, 0x15, 0x78 },  // UC A umlaut
  { 0x38, 0x45, 0x44, 0x7d, 0x40 },  // LC a umlaut
  { 0x3c, 0x43, 0x42, 0x43, 0x3c },  // UC O umlaut
  { 0x38, 0x45, 0x44, 0x45, 0x38 },  // LC o umlaut
  { 0x3e, 0x41, 0x40, 0x41, 0x3e },  // UC U umlaut
  { 0x3c, 0x41, 0x40, 0x41, 0x3c },  // LC u umlaut
  { 0x7e, 0x09, 0x7f, 0x49, 0x49 },  // UC AE
  { 0x38, 0x44, 0x38, 0x54, 0x58 },  // LC ae
  { 0x14, 0x34, 0x1c, 0x16, 0x14 },  // not equal
  { 0x48, 0x7e, 0x49, 0x41, 0x22 },  // pound sterling
  { 0x55, 0x2a, 0x55, 0x2a, 0x55 },  // ?
  { 0x00, 0x00, 0x00, 0x00, 0x00 },  // space
  { 0x00, 0x00, 0x5f, 0x00, 0x00 },  // bang
  { 0x00, 0x03, 0x00, 0x03, 0x00 },  // double quote
  { 0x14, 0x7f, 0x14, 0x7f, 0x14 },  // hash (pound, octothorpe)
  { 0x24, 0x2a, 0x7f, 0x2a, 0x12 },  // dollar
  { 0x23, 0x13, 0x08, 0x64, 0x62 },  // percent
  { 0x36, 0x49, 0x56, 0x20, 0x50 },  // ampersand
  { 0x00, 0x00, 0x03, 0x00, 0x00 },  // single quote
  { 0x00, 0x1c, 0x22, 0x41, 0x00 },  // left parenthesis
  { 0x00, 0x41, 0x22, 0x1c, 0x00 },  // right parenthesis
  { 0x14, 0x08, 0x3e, 0x08, 0x14 },  // asterisk
  { 0x08, 0x08, 0x3e, 0x08, 0x08 },  // plus
  { 0x00, 0x40, 0x30, 0x00, 0x00 },  // comma
  { 0x08, 0x08, 0x08, 0x08, 0x08 },  // hyphen
  { 0x00, 0x60, 0x60, 0x00, 0x00 },  // period
  { 0x20, 0x10, 0x08, 0x04, 0x02 },  // slash
  { 0x3e, 0x51, 0x49, 0x45, 0x3e },  // zero
  { 0x00, 0x42, 0x7e, 0x40, 0x00 },  // one
  { 0x62, 0x51, 0x49, 0x49, 0x46 },  // two
  { 0x21, 0x41, 0x49, 0x4d, 0x33 },  // three
  { 0x18, 0x14, 0x12, 0x7f, 0x10 },  // four
  { 0x27, 0x45, 0x45, 0x45, 0x39 },  // five
  { 0x3c, 0x4a, 0x49, 0x48, 0x30 },  // six
  { 0x01, 0x71, 0x09, 0x05, 0x03 },  // seven
  { 0x36, 0x49, 0x49, 0x49, 0x36 },  // eight
  { 0x06, 0x49, 0x49, 0x29, 0x1e },  // nine
  { 0x00, 0x00, 0x24, 0x00, 0x00 },  // colon
  { 0x00, 0x40, 0x34, 0x00, 0x00 },  // semicolon
  { 0x08, 0x14, 0x22, 0x41, 0x00 },  // less than
  { 0x14, 0x14, 0x14, 0x14, 0x14 },  // equal
  { 0x00, 0x41, 0x22, 0x14, 0x08 },  // greater than
  { 0x02, 0x01, 0x51, 0x09, 0x06 },  // question mark
  { 0x3e, 0x41, 0x5d, 0x5d, 0x1e },  // at
  { 0x7e, 0x11, 0x11, 0x11, 0x7e },  // UC A
  { 0x7f, 0x49, 0x49, 0x49, 0x36 },  // UC B
  { 0x3e, 0x41, 0x41, 0x41, 0x22 },  // UC C
  { 0x41, 0x7f, 0x41, 0x41, 0x3e },  // UC D
  { 0x7f, 0x49, 0x49, 0x49, 0x41 },  // UC E
  { 0x7f, 0x09, 0x09, 0x09, 0x01 },  // UC F
  { 0x3e, 0x41, 0x41, 0x51, 0x72 },  // UC G
  { 0x7f, 0x08, 0x08, 0x08, 0x7f },  // UC H
  { 0x00, 0x41, 0x7f, 0x41, 0x00 },  // UC I
  { 0x20, 0x40, 0x40, 0xef, 0x00 },  // UC J
  { 0x7f, 0x08, 0x14, 0x22, 0x41 },  // UC K
  { 0x7f, 0x40, 0x40, 0x40, 0x40 },  // UC L
  { 0x7f, 0x02, 0x0c, 0x02, 0x7f },  // UC M
  { 0x7f, 0x04, 0x08, 0x10, 0x7f },  // UC N
  { 0x3e, 0x41, 0x41, 0x41, 0x3e },  // UC O
  { 0x7f, 0x09, 0x09, 0x09, 0x06 },  // UC P
  { 0x3e, 0x41, 0x51, 0x21, 0x5e },  // UC Q
  { 0x7f, 0x09, 0x19, 0x29, 0x46 },  // UC R
  { 0x26, 0x49, 0x49, 0x49, 0x32 },  // UC S
  { 0x01, 0x01, 0x7f, 0x01, 0x01 },  // UC T
  { 0x3f, 0x40, 0x40, 0x40, 0x3f },  // UC U
  { 0x07, 0x18, 0x60, 0x18, 0x07 },  // UC V
  { 0x7f, 0x20, 0x18, 0x20, 0x7f },  // UC W
  { 0x63, 0x14, 0x08, 0x14, 0x63 },  // UC X
  { 0x03, 0x04, 0x78, 0x04, 0x03 },  // UC Y
  { 0x61, 0x51, 0x49, 0x45, 0x43 },  // UC Z
  { 0x00, 0x7f, 0x41, 0x41, 0x00 },  // left bracket
  { 0x02, 0x04, 0x08, 0x10, 0x20 },  // backslash
  { 0x00, 0x41, 0x41, 0x7f, 0x00 },  // right bracket
  { 0x04, 0x02, 0x7f, 0x02, 0x04 },  // up arrow
  { 0x40, 0x40, 0x40, 0x40, 0x40 },  // underscore
  { 0x00, 0x01, 0x07, 0x01, 0x00 },  // superscript T
  { 0x20, 0x54, 0x54, 0x54, 0x78 },  // LC a
  { 0x7f, 0x48, 0x44, 0x44, 0x38 },  // LC b
  { 0x38, 0x44, 0x44, 0x44, 0x20 },  // LC c
  { 0x38, 0x44, 0x44, 0x48, 0x7f },  // LC d
  { 0x38, 0x54, 0x54, 0x54, 0x08 },  // LC e
  { 0x08, 0x7c, 0x0a, 0x01, 0x02 },  // LC f
  { 0x08, 0x14, 0x54, 0x54, 0x38 },  // LC g
  { 0x7f, 0x10, 0x08, 0x08, 0x70 },  // LC h
  { 0x00, 0x44, 0x7d, 0x40, 0x00 },  // LC i
  { 0x20, 0x40, 0x40, 0x3d, 0x00 },  // LC j
  { 0x00, 0x7f, 0x28, 0x44, 0x00 },  // LC k
  { 0x00, 0x41, 0x7f, 0x40, 0x00 },  // LC l
  { 0x78, 0x04, 0x18, 0x04, 0x78 },  // LC m
  { 0x7c, 0x08, 0x04, 0x04, 0x78 },  // LC n
  { 0x38, 0x44, 0x44, 0x44, 0x38 },  // LC o
  { 0x7c, 0x14, 0x24, 0x24, 0x18 },  // LC p
  { 0x18, 0x24, 0x24, 0x7c, 0x40 },  // LC q
  { 0x7c, 0x08, 0x04, 0x04, 0x08 },  // LC r
  { 0x48, 0x54, 0x54, 0x54, 0x24 },  // LC s
  { 0x04, 0x3e, 0x44, 0x20, 0x00 },  // LC t
  { 0x3c, 0x40, 0x40, 0x20, 0x7c },  // LC u
  { 0x1c, 0x20, 0x40, 0x20, 0x1c },  // LC v
  { 0x3c, 0x40, 0x30, 0x40, 0x3c },  // LC w
  { 0x44, 0x28, 0x10, 0x28, 0x44 },  // LC x
  { 0x44, 0x28, 0x10, 0x08, 0x04 },  // LC y
  { 0x44, 0x64, 0x54, 0x4c, 0x44 },  // LC z
  { 0x08, 0x78, 0x08, 0x78, 0x04 },  // LC pi
  { 0x00, 0x00, 0x7f, 0x00, 0x00 },  // vertical bar
  { 0x08, 0x08, 0x2a, 0x1c, 0x08 },  // right arrow
  { 0x63, 0x55, 0x49, 0x41, 0x63 },  // UC sigma
  { 0x7f, 0x08, 0x08, 0x08, 0x08 },  // lazy T
};


Chp82143A::Chp82143A(CPObject *parent):Cprinter(this) {

    setcfgfname(QString("hp82143a"));
    BackGroundFname	= P_RES(":/hp41/hp82143a.png");

    pKEYB = new Ckeyb(this,"hp82143a.map");

    setDXmm(180);
    setDYmm(130);
    setDZmm(60);

    setDX(643);
    setDY(464);


    margin = 30;
    paperWidth = 24*7+margin+margin;
    setPaperPos(QRect(355,16,216,250));

    Mode = TRACE_MODE;
    flow = fdwid = fprint = fpadv = fgraph = feol = frjust = fignADV = false;
    isready = false;
    intensity = 0;
}

Chp82143A::~Chp82143A() {
}

bool Chp82143A::init(void) {

    Cprinter::init();

    charTable = new QImage(P_RES(":/ext/ce126ptable.bmp"));
    charsize = 1;

    pCONNECTOR = new Cconnector(this,64,0,Cconnector::hp41,"hp41 Module",true,QPoint(631,468));
    publish(pCONNECTOR);

    if(pKEYB)	pKEYB->init();
    if(pTIMER)	pTIMER->init();

    // Create  Paper Image
    // The final paper image is 207 x 149 at (277,0) for the ce125
    printerbuf	= new QImage(QSize(paperWidth, 3000),QImage::Format_ARGB32);
    printerdisplay= new QImage(QSize(paperWidth, 250),QImage::Format_ARGB32);

    paperWidget = new CpaperWidget(PaperPos(),printerbuf,this);
    paperWidget->hide();
//    paperWidget->show();
//    paperWidget->hide();

    // Fill it blank
    clearPaper();

    settop(10);
    setposX(0);

    isready = true;
    return true;
}

void Chp82143A::clearPaper(void)
{
    // Fill it blank
    printerbuf->fill(PaperColor.rgba());
    printerdisplay->fill(QColor(255,255,255,0).rgba());
    settop(10);
    setposX(0);
    // empty TextBuffer
    TextBuffer.clear();
    paperWidget->updated = true;
}

#define PRINTER_TRACE       (1 << 15) // if TRACE and NORM are both
#define PRINTER_NORM        (1 << 14) //     zero, the switch is set to MAN
#define PRINTER_PRINTDOWN   (1 << 13) // PRinT KEY is pressed
#define PRINTER_ADVDOWN     (1 << 12) // paper ADVance KEY is pressed
#define PRINTER_OUTPAPER    (1 << 11) // paper out
#define PRINTER_BATLOW      (1 << 10) // low battery
#define PRINTER_IDLE        (1 << 9)  // not printing or advancing paper
#define PRINTER_BUFEMPTY    (1 << 8)  // Buffer Empty
#define PRINTER_LOWERCASE   (1 << 7)  // Lower Case mode
#define PRINTER_GRAPHIC     (1 << 6)  // Graphics mode (bit-mapped columns)
#define PRINTER_DOUBLE      (1 << 5)  // Double Wide mode
#define PRINTER_RIGHTJUST   (1 << 4)  // Type of End of Line is Right Justify
#define PRINTER_EOLSENT     (1 << 3)  // Last byte was End Of Line
#define PRINTER_NOADV       (1 << 2)  // IGNore paper advance key
#define PRINTER_NOTUSED_1   (1 << 1)
#define PRINTER_NOTUSED_0   (1 << 0)

quint16 Chp82143A::getStatus(void) {

    quint16 status=0;

    switch (Mode) {
    case TRACE_MODE: status |= PRINTER_TRACE; break;
    case NORM_MODE: status |= PRINTER_NORM; break;
    }

    if (flow) status |= PRINTER_LOWERCASE;
    if (fdwid) status |= PRINTER_DOUBLE;
    if (fprint) status |= PRINTER_PRINTDOWN;
    if (fpadv) status |= PRINTER_ADVDOWN;
    if (feol) status |= PRINTER_EOLSENT;
    if (frjust) status |= PRINTER_RIGHTJUST;
    if (fignADV) status |= PRINTER_NOADV;

    return status | PRINTER_IDLE | PRINTER_BUFEMPTY;
}

bool Chp82143A::run(void) {

    if (!isready) return true;

    Cbus bus;

    bus.fromUInt64(pCONNECTOR->Get_values());

    if (!bus.isEnable()) return true;
#if 0
    switch (bus.getFunc()) {
    case BUS_SLEEP: break;
    case BUS_READDATA:
        if (bus.getAddr()==0x3a) {
            bus.setData(getStatus() >> 8);
            bus.setFunc(BUS_READDATA);
        }
        if (bus.getAddr()==0x3b) {
            bus.setData(getStatus() & 0xff);
            bus.setFunc(BUS_READDATA);
        }
        break;
    case BUS_WRITEDATA: Printer(bus.getData());
        bus.setFunc(BUS_SLEEP);
        break;
    }
#else
    if (bus.isWrite()) {
        Printer(bus.getData());
//        bus.setFunc(BUS_SLEEP);
    }
    else {
        if (bus.getAddr()==0x3a) {
            bus.setData(getStatus() >> 8);
//            bus.setFunc(BUS_READDATA);
        }
        if (bus.getAddr()==0x3b) {
            bus.setData(getStatus() & 0xff);
//            bus.setFunc(BUS_READDATA);
        }
    }

#endif
    pCONNECTOR->Set_values(bus.toUInt64());
    return true;
}

bool Chp82143A::exit(void) {

    Cprinter::exit();

    return true;
}

#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))
void Chp82143A::ComputeKey(void)
{
    if (KEY(K_PFEED)) {
        top+=10;
        fpadv = true;
        update();
    }
    else fpadv = false;

    if (KEY(K_PRINT)) {
        fprint = true;
    }
    else fprint = false;

    if (pKEYB->LastKey == K_PRT_INT_MIN) { intensity = -1; update(); }
    if (pKEYB->LastKey == K_PRT_INT_NORM) { intensity = 0; update(); }
    if (pKEYB->LastKey == K_PRT_INT_MAX) { intensity = 1; update(); }

    if (pKEYB->LastKey == K_PRT_NORM) { Mode = NORM_MODE; update(); }
    if (pKEYB->LastKey == K_PRT_TRACE) { Mode = TRACE_MODE; update(); }
    if (pKEYB->LastKey == K_PRT_MANUAL) { Mode = MANUAL_MODE; update(); }

//    if (pKEYB->LastKey == K_PRINT_ON) {
//        printerSwitch = true;
//    }
//    if (pKEYB->LastKey == K_PRINT_OFF) {
//        printerSwitch = false;
//    }
}

#define HIGHEST_VAL_TO_BUFFER 0xdf
void Chp82143A::Printer(quint8 data) {

    qWarning()<<"Received : "<<data<<" ("<<QChar(data)<<")";

    feol   = (data == 0xe0) || (data == 0xe8);
    frjust = (data >> 3) & 1;

    if (data <= HIGHEST_VAL_TO_BUFFER)
    {
        TextBuffer.append(data);

//        if (data>= 0xd0) {
//            flow= (data & 0x01);
//            fgraph = (data & 0x02);
//            fdwid= (data & 0x04);
//        }

        if (TextBuffer.size() == BUF_MAX)
        {
            feol = true;
        }
        if (feol) printLine(frjust);
    }
    else if (feol) printLine(frjust);
    else if (data >= 0xfe) {
        fignADV = data & PRINTER_NOADV;
    }
}


bool Chp82143A::addChar (quint8 c)
{
    if ( (BufferColumns.size()+ (fdwid ? 14:7)) > PRINTER_WIDTH) return false;

    if ( (flow) && ((c >= 'A') && (c <= 'Z'))) c += 0x20;

    BufferColumns.append((char)0);
    if (fdwid) BufferColumns.append((char)0);

    for (int col = 0; col < 5; col++) {
        quint8 col_data = hp82143a_chargen [c] [col];
        BufferColumns.append(col_data);
        if (fdwid) BufferColumns.append(col_data);
    }

    BufferColumns.append((char)0);
    if (fdwid) BufferColumns.append((char)0);

    return true;
}


void Chp82143A::printLine(bool rightJustified) {
    BufferColumns.clear();
    qWarning()<<"buffer:"<<TextBuffer<<"  ->"<<TextBuffer.toHex();
    flow = fgraph = fdwid= false;
    for (int i=0; i< TextBuffer.size();i++) {
        quint8 data = TextBuffer.at(i);
        if (data <= 0x80) {
            if (fgraph) BufferColumns.append(data);
            else  addChar(data);
        }
        else if ( (data == 0xa0) || (data == 0xb8) ) {
            fgraph = false;
        }
        else if ( (data > 0xa0) && (data<0xb8) ) {
            /* accumulate spaces */
            for (int j=0xa0;j<data;j++) {
                addChar(' ');
            }
            fgraph = false;
        }
        else if ( (data > 0xb8) && (data<0xc0) ) {
            for (int j=0xb8;j<data;j++) {
                BufferColumns.append((char)0);

            }
        }
        else {
                flow= (data & 0x01);
                fgraph = (data & 0x02);
                fdwid= (data & 0x04);
        }
    }
    if (rightJustified && (BufferColumns.size() < PRINTER_WIDTH))
    {
        // shift output data to right justify
        BufferColumns = BufferColumns.rightJustified(PRINTER_WIDTH,0,true);
    }
    TextBuffer.clear();

    QPainter painter;
    for (int i=0; i< BufferColumns.size();i++) {
        quint8 data = BufferColumns.at(i);
        painter.begin(printerbuf);
        QColor _color;
        if (intensity==0) _color = QColor(80,80,80);
        if (intensity==-1) _color = QColor(128,128,128);
        if (intensity==1) _color = QColor(0,0,0);
        painter.setPen(_color);
        for (int b=0; b<8;b++)
        {
            if (data & (1<<b))
                painter.drawPoint(  margin + (7 * posX*charsize) +i, top+b);
        }
        painter.end();
    }
    top+=10;
    BufferColumns.clear();
    painter.begin(printerdisplay);

    painter.drawImage(QRectF(0,MAX(149-top,0),paperWidth/charsize,MIN(top,149)),
                      *printerbuf,
                      QRectF(0,MAX(0,top-149),paperWidth/charsize,MIN(top,149)));

    painter.end();

    Refresh_Display = true;

    paperWidget->setOffset(QPoint(0,top));
    paperWidget->updated = true;



}

bool Chp82143A::UpdateFinalImage(void) {

    Cprinter::UpdateFinalImage();

    QPainter painter;
    painter.begin(FinalImage);

    // Draw switch

    if (intensity == -1)
        painter.drawImage(205,338,QImage(P_RES(":/hp41/hp82143a_mode_man.png")));
    if (intensity == 1)
        painter.drawImage(205,338,QImage(P_RES(":/hp41/hp82143a_mode_norm.png")));
    if (intensity == 0)
        painter.drawImage(205,338,BackgroundImageBackup->copy(205,338,69,24));


    // MODE SWITCH

    if (Mode == NORM_MODE)
        painter.drawImage(205,385,QImage(P_RES(":/hp41/hp82143a_mode_norm.png")));
    if (Mode == MANUAL_MODE)
        painter.drawImage(205,385,QImage(P_RES(":/hp41/hp82143a_mode_man.png")));
    if (Mode == TRACE_MODE)
        painter.drawImage(205,385,BackgroundImageBackup->copy(205,385,69,24));


    float ratio = ( (float) paperWidget->width() ) / ( paperWidget->bufferImage->width() - paperWidget->getOffset().x() );

//    ratio *= charsize;
    QRect source = QRect( QPoint(paperWidget->getOffset().x() ,
                                 paperWidget->getOffset().y()  - paperWidget->height() / ratio ) ,
                          QPoint(paperWidget->bufferImage->width(),
                                 paperWidget->getOffset().y() +10)
                          );
//    MSG_ERROR(QString("%1 - %2").arg(source.width()).arg(PaperPos().width()));
    painter.drawImage(PaperPos(),
                      paperWidget->bufferImage->copy(source).scaled(PaperPos().size(),Qt::IgnoreAspectRatio, Qt::SmoothTransformation )
                      );
//    painter.drawImage(350,15,QImage("://hp41/hp82143a_cover.png"));

    painter.end();

    return true;
}


