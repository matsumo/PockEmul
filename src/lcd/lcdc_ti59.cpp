#include <QDebug>
#include <QPainter>
#include <QTimer>

#include "Lcdc_ti59.h"
#include "Lcdc_symb.h"
#include "ti/ti59.h"


void Clcdc_ti59::disp_symb(void)
{
}

Clcdc_ti59::Clcdc_ti59(CPObject *parent, QRect _lcdRect, QRect _symbRect, QString _lcdfname, QString _symbfname):
    Clcdc(parent,_lcdRect,_symbRect,_lcdfname,_symbfname){						//[constructor]
    Color_Off.setRgb(
                (int) (111*contrast),
                (int) (117*contrast),
                (int) (108*contrast));

    blinkTimer = new QTimer(this);
    connect(blinkTimer,SIGNAL(timeout()),this,SLOT(blink()));
    blinkState = false;

}

void Clcdc_ti59::disp(void)
{
    bool error = false;

    Refresh = false;

    if (!ready) return;
    if (!redraw) {
        if (!updated) return;
    }
    redraw = false;
    updated = false;
    Refresh= true;

    QPainter painter(LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
//    painter.setPen(Qt::transparent);
//    painter.setBrush(Qt::transparent);
    painter.fillRect(LcdImage->rect(),Qt::transparent);

    QFont font;
    font.setPixelSize(30);
    painter.setFont(font);

    QString s = ((Cti59 *)pPC)->Display();
    // Check for ERROR
    if (s.startsWith('E')) {
//        qWarning()<<"ERROR";
        s.remove(0,1);  // Remove the error
        // Start Blinking timer
        if (!blinkTimer->isActive()) {
            blinkTimer->start(500);
            blinkState = true;
//            qWarning()<<"start timer";
        }
        error = true;
    }
    else {
        blinkTimer->stop();
    }

//    qWarning()<<"display:"<<s;

#define YOFFSET 5

    if (!error || blinkState) {
        //        qWarning()<<"print text";
        painter.setPen(QColor(255,0,0));
#if 0
        painter.drawText(LcdImage->rect(),Qt::AlignCenter,s);
#else
        int pos = 0;
        int charSpace = 18;
        for (int i =0;i<s.length();i++) {
            int code = s.at(i).digitValue();
            if (s.at(i) == '-') code = 16;
            if (s.at(i) == '.') code = 17;
            if (s.at(i) == ' ') pos+=charSpace;

            if (code == 17) {
                QRect rect(pos- charSpace + 10,12,1,1);
                painter.drawRect(rect);
            }
            else
                if (code >=0) {

                    QRect rect(pos,0,8,13);

                    Clcd7::draw(code,&painter,rect,1,0);
                    pos += charSpace;
                }
        }
#endif
    }
    //On=true;
    Refresh = true;
    painter.end();
}

void Clcdc_ti59::blink() {


    blinkState = !blinkState;
//qWarning()<<"blink timer "<<blinkState;
    updated = true;
}


