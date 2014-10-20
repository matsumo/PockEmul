#ifndef POSTIT_H
#define POSTIT_H

#include <QDebug>
#include <QTextEdit>
#include <QContextMenuEvent>

#include "pobject.h"


class QVBoxLayout;

class CpostitTextEdit:public QTextEdit {
    Q_OBJECT
public:
    CpostitTextEdit(QWidget * parent):QTextEdit(parent){}


    void wheelEvent(QWheelEvent *e) {
        QTextEdit::wheelEvent(e);
        e->accept();
        qWarning()<<"OK";
    }

    void contextMenuEvent ( QContextMenuEvent * event )
    {
        QMenu *menupocket = QTextEdit::createStandardContextMenu();
        menupocket->addSeparator();
        menupocket->addAction(tr("Bold"),this,SLOT(boldText()));
        menupocket->addAction(tr("Link"),this,SLOT(urlText()));



    //    menu->setStyleSheet("QMenu { color: black }");
        //menu->exec(event->globalPos () );
    menupocket->popup(event->globalPos () );
        event->accept();
    }

public slots:
    void
    boldText() //this is the SLOT for the button trigger(bool)
    {
        bool isBold= true;
        QTextCharFormat fmt;
        fmt.setFontWeight(isBold ? QFont::Bold : QFont::Normal);
        mergeFormatOnWordOrSelection(fmt);
    }
    void
    urlText() //this is the SLOT for the button trigger(bool)
    {

        QTextCharFormat fmt;
        qWarning()<<"url:"<<acceptRichText();
        QTextCursor cursor = textCursor();
        if (!cursor.hasSelection())
            cursor.select(QTextCursor::WordUnderCursor);

        fmt.setAnchor(true);
        fmt.setAnchorHref("http://pockemul.free.fr");
        cursor.mergeCharFormat(fmt);
        mergeCurrentCharFormat(fmt);
        QString txt = tr("<a href='http://pockemul.free.fr'>%1</a>").arg(cursor.selectedText());
        cursor.removeSelectedText();
        cursor.insertHtml(txt);

//        QString text_all(toHtml());
//        setHtml(text_all);
    }

public:
    void
    mergeFormatOnWordOrSelection(const QTextCharFormat &format)
     {
         QTextCursor cursor = textCursor();
         if (!cursor.hasSelection())
             cursor.select(QTextCursor::WordUnderCursor);
         cursor.mergeCharFormat(format);
         mergeCurrentCharFormat(format);
     }

};

class Cpostit:public CPObject {
    Q_OBJECT
public:
    const char*	GetClassName(){ return("Cpostit");}

    bool run(void);
    bool init(void);				//initialize
    bool exit(void);				//end
    virtual bool	InitDisplay(void);
    virtual bool	UpdateFinalImage(void);
    virtual bool    SaveSession_File(QXmlStreamWriter *xmlOut);
    virtual bool	LoadSession_File(QXmlStreamReader *xmlIn);
    Cpostit(CPObject *parent = 0);
    virtual ~Cpostit();

protected:
    void paintEvent(QPaintEvent *);
protected slots:
    void contextMenuEvent ( QContextMenuEvent * );

    void slotDblSize();

private:
    CpostitTextEdit *edit;
    QVBoxLayout *HBL,*mainLayout;
    float sizeFactor;

};

#endif // POSTIT_H
