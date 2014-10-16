#include <QMenu>
#include <QClipboard>
#include <QFileDialog>
#include <QPainter>
#include <QDebug>

#include "cprinter.h"
#include "paperwidget.h"
#include "Log.h"

//TODO  Lorsque pointeur positionne sur papier, afficher scroolbar verticale et gerer mousewheel
Cprinter::Cprinter(CPObject *parent):CPObject(parent)
{
	PaperColor = QColor(255,255,255);
	paperWidget = 0;
    pos=QRect(0,0,0,0);

}

Cprinter::~Cprinter()
{
	delete paperWidget;
}

bool Cprinter::UpdateFinalImage(void) {

    CPObject::UpdateFinalImage();

    QPainter painter;
    painter.begin(FinalImage);


    float ratio = ( (float) paperWidget->width() ) / ( paperWidget->bufferImage->width() - paperWidget->getOffset().x() );

//    ratio *= charsize;
    QRect source = QRect( QPoint(paperWidget->getOffset().x() ,
                                 paperWidget->getOffset().y()  - paperWidget->height() / ratio ) ,
                          QPoint(paperWidget->bufferImage->width(),
                                 paperWidget->getOffset().y() +10)
                          );
//    MSG_ERROR(QString("%1 - %2").arg(source.width()).arg(PaperPos().width()));

    QRect _target = QRect(PaperPos().topLeft()*internalImageRatio,PaperPos().size()*internalImageRatio);
    painter.drawImage(_target,
                      paperWidget->bufferImage->copy(source).scaled(_target.size(),Qt::IgnoreAspectRatio, Qt::SmoothTransformation )
                      );

    painter.end();

    emit updatedPObject(this);

    return true;
}

void Cprinter::resizeEvent ( QResizeEvent * ) {
    float ratio = (float)this->width()/getDX() ;

    if (!paperWidget) return;

    QRect rect = this->paperWidget->baseRect;
    this->paperWidget->setGeometry( rect.x()*ratio,
                                    rect.y()*ratio,
                                    rect.width()*ratio,
                                    rect.height()*ratio);
    this->paperWidget->updated=true;
}

QImage * Cprinter::checkPaper(QImage *printerbuf,int top) {
    int _height = printerbuf->height();
    if (top >= (_height-500)) {
        qWarning()<<"increase size:"<<_height;
        QImage *_tmp = printerbuf;
        printerbuf = new QImage(_tmp->width(),_height+500,QImage::Format_ARGB32);
        printerbuf->fill(PaperColor.rgba());

        qWarning()<<"increased size:"<<printerbuf->size();
        QPainter painter(printerbuf);
        painter.drawImage(0,0,*_tmp);
        painter.end();
        paperWidget->bufferImage = printerbuf;
        delete _tmp;

    }
    return printerbuf;
}

void Cprinter::BuildContextMenu(QMenu *menu)
{
    CPObject::BuildContextMenu(menu);

    QMenu * menuPaper = menu->addMenu(tr("Paper"));
    menuPaper->addAction(tr("Copy Image"),paperWidget,SLOT(paperCopy()));
    menuPaper->addAction(tr("Copy Text"),paperWidget,SLOT(paperCopyText()));
    menuPaper->addAction(tr("Cut"),paperWidget,SLOT(paperCut()));
    menuPaper->addAction(tr("Save Image ..."),paperWidget,SLOT(paperSaveImage()));
    menuPaper->addAction(tr("Save Text ..."),paperWidget,SLOT(paperSaveText()));

}

void Cprinter::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu= new QMenu(this);

    BuildContextMenu(menu);

    menu->popup(event->globalPos () );
    event->accept();
}

void Cprinter::moveEvent ( QMoveEvent * event ) {
    if (paperWidget) paperWidget->updated=true;
}

void Cprinter::setPaperPos(QRect pos)
{
    this->pos = pos;
}

QRect Cprinter::PaperPos()
{
    return(pos);
}

void Cprinter::raise()
{
    if (paperWidget) paperWidget->updated = true;
    AddLog(LOG_TEMP,"RAISE");
    CPObject::raise();

}

bool Cprinter::SaveSession_File(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("session");
        xmlOut->writeAttribute("version", "2.0");
        xmlOut->writeAttribute("posx",QString("%1").arg(pos.x()));
        xmlOut->writeAttribute("posy",QString("%1").arg(pos.y()));
        xmlOut->writeAttribute("buffer",QString(TextBuffer.toBase64()));

    xmlOut->writeEndElement();  // session
    return true;
}

bool Cprinter::LoadSession_File(QXmlStreamReader *xmlIn)
{
    if (xmlIn->name()=="session") {
        pos.setX( xmlIn->attributes().value("posx").toString().toInt());
        pos.setY( xmlIn->attributes().value("posy").toString().toInt());
        TextBuffer.clear();
        TextBuffer = QByteArray::fromBase64(xmlIn->attributes().value("posy").toString().toLatin1());
    }
    return true;
}
