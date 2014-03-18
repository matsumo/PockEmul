#include <QMenu>
#include <QClipboard>
#include <QFileDialog>

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

void Cprinter::resizeEvent ( QResizeEvent * ) {
    float ratio = (float)this->width()/getDX() ;

    QRect rect = this->paperWidget->baseRect;
    this->paperWidget->setGeometry( rect.x()*ratio,
                                    rect.y()*ratio,
                                    rect.width()*ratio,
                                    rect.height()*ratio);
    this->paperWidget->updated=true;
}

void Cprinter::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu= new QMenu(this);

    BuildContextMenu(menu);

    QMenu * menuPaper = menu->addMenu(tr("Paper"));
    menuPaper->addAction(tr("Copy Image"),paperWidget,SLOT(paperCopy()));
    menuPaper->addAction(tr("Copy Text"),paperWidget,SLOT(paperCopyText()));
    menuPaper->addAction(tr("Cut"),paperWidget,SLOT(paperCut()));
    menuPaper->addAction(tr("Save Image ..."),paperWidget,SLOT(paperSaveImage()));
    menuPaper->addAction(tr("Save Text ..."),paperWidget,SLOT(paperSaveText()));

    menu->popup(event->globalPos () );
    event->accept();
}

void Cprinter::moveEvent ( QMoveEvent * event ) {
    this->paperWidget->updated=true;
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
    paperWidget->updated = true;
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
