
//TODO: Key management
#include <QDebug>

#include <QPainter>

#include "fp40.h"
#include "Connect.h"
#include "Keyb.h"
#include "dialogconsole.h"
#include "init.h"
#include "Inter.h"
#include "Log.h"
#include "paperwidget.h"

#define DOWN	0
#define UP		1


Cfp40::Cfp40(CPObject *parent):CprinterCtronics(this) {
    Q_UNUSED(parent)

    setcfgfname(QString("fp40"));
    BackGroundFname	= P_RES(":/ext/fp40.png");

    delete pKEYB; pKEYB		= new Ckeyb(this,"fp40.map");

    setDXmm(210);//Pc_DX_mm = 256;
    setDYmm(145);//Pc_DY_mm = 185;
    setDZmm(42);//Pc_DZ_mm = 42;

    setDX(750);//Pc_DX	= 895;
    setDY(518);//Pc_DY	= 615;


    margin = 40;
    paperWidth = 640;
    setPaperPos(QRect(90,26,400,300));

    escapeSeq= NONE;
    readCounter= -1;
    paperfeedsize = 10;
}

Cfp40::~Cfp40() {
}

bool Cfp40::init(void) {

    charTable = new QImage(P_RES(":/ext/ce126ptable.bmp"));
    charsize = 2;

    CprinterCtronics::init();

//    qWarning()<<"init done";
    return true;
}

bool Cfp40::run(void) {

    CprinterCtronics::run();

    return true;
}

bool Cfp40::exit(void) {

    CprinterCtronics::exit();

    return true;
}


void Cfp40::ComputeKey(void)
{
    if (pKEYB->LastKey == K_PFEED) {
        Printer(0x0d);
    }
    if (pKEYB->LastKey == K_PRT_COND) {
        if (charsize==1) charsize=2;
        else charsize = 1;
        update();
    }
//    if (pKEYB->LastKey == K_PRINT_ON) {
//        printerSwitch = true;
//    }
//    if (pKEYB->LastKey == K_PRINT_OFF) {
//        printerSwitch = false;
//    }
}


void Cfp40::Printer(quint8 data) {
    QPainter painter;

//    qWarning()<<"RECIEDVED:"<<data<<"="<<QChar(data);
    switch (escapeSeq) {
    case NONE :
        if (data == 0x0a) break;
        if (data == 0xff) break;
        if (data == 27) {
            escapeSeq = WAITCMD;
//            qWarning()<<"ESC RECIEDVED";
            break;
        }

        if (data == 0x0d){
            top+=paperfeedsize*2;//charsize;
            setposX(0);
            TextBuffer += data;
            //        qWarning()<<"CR PRINTED";
        }
        else
        {
            if (posX >= (6*2*80/charsize)) {
                top+=paperfeedsize*2;//charsize;
                setposX(0);
                TextBuffer += 0x0d;
            }
            //        qWarning()<<"CHAR PRINTED:"<<QChar(data);
            TextBuffer += data;
            painter.begin(printerbuf);
            int x = ((data>>4) & 0x0F)*6;
            int y = (data & 0x0F) * 8;
            painter.drawImage(	QRectF( margin + posX,top,5*charsize,7*2/*charsize*/),
                                *charTable,
                                QRectF( x , y , 5,7));
            posX += 6*charsize;
            painter.end();
        }
        break;
    case WAITCMD:
        switch (TOUPPER(data)) {
        case 'A': escapeSeq = CMD_A; break;
        case 'K': escapeSeq = CMD_K; break;
        case 'L': escapeSeq = CMD_L; break;
        default: // ERROR
            break;
        }
        n1=n2=-1;
        currentCMD = escapeSeq;
        break;
    case CMD_A: if (data==8) paperfeedsize = 8;
        if (data==12) paperfeedsize = 12;
//        qWarning()<<"paperfeedsize="<<paperfeedsize;
        escapeSeq = NONE;
        break;
    case CMD_K:
    case CMD_L:
        if (n1>=0) {
            n2 = data;
            readCounter = n1 + 256*n2;
//            qWarning()<<"n2="<<n2<<"  nb="<<readCounter;
            escapeSeq = READ_DATA;
            n1=n2=-1;
        }
        else {
            n1 = data;
//            qWarning()<<"n1="<<n1;
        }

        break;
    case READ_DATA:
//        qWarning()<<"read DATA:"<<readCounter;
        readCounter--;
        readData.append(data);
        if (readCounter==0) {
            escapeSeq= NONE;
            readCounter= -1;
            // execute command
            painter.begin(printerbuf);
            painter.setPen(Qt::black);
            for (int i=0;i<readData.size();i++) {
                for (int b=0; b<8;b++)
                {
                    if ((readData.at(i)>>(7-b))&0x01) {
                        painter.drawPoint( margin + posX, top+2*b);
                        painter.drawPoint( margin + posX, top+2*b+1);

                        painter.drawPoint( margin + posX+1, top+2*b);
                        painter.drawPoint( margin + posX+1, top+2*b+1);
                    }
                }
                posX++; posX++;
            }
            painter.end();
            readData.clear();
        }
        break;
    }



    painter.begin(printerdisplay);

    painter.drawImage(QRectF(0,MAX(149-top,0),paperWidth/charsize,MIN(top,149)),
                      *printerbuf,
                      QRectF(0,MAX(0,top-149),paperWidth/charsize,MIN(top,149)));

// Draw printer head
//    painter.fillRect(QRect(0 , 147,207,2),QBrush(QColor(0,0,0)));
//    painter.fillRect(QRect(21 + (7 * posX) , 147,14,2),QBrush(QColor(255,255,255)));



    painter.end();

    Refresh_Display = true;

    paperWidget->setOffset(QPoint(0,top));
    paperWidget->updated = true;


}

bool Cfp40::UpdateFinalImage(void) {

    CprinterCtronics::UpdateFinalImage();

    QPainter painter;
    painter.begin(FinalImage);


    painter.drawImage(650,280,BackgroundImageBackup->copy(650,280,33,60).mirrored(false,charsize==1?false:true));
    painter.end();

    emit updatedPObject(this);
    return true;
}




