#include "sizegrip.h"
#include <QPainter>
#include <QDebug>
#include <QTextItem>


#include "postit.h"

//#include "common.h"
#include "Log.h"


//FIXME: on Android, background color issue

Cpostit::Cpostit(CPObject *parent )   : CPObject(this)
{
    Q_UNUSED(parent)

//    setWindowFlags(Qt::SubWindow);
//    setParent(parent);

    setfrequency( 0);
    BackGroundFname     = P_RES(":/core/PostIt.png");

    setDXmm(85);
    setDYmm(318);
    setDZmm(51);
 // Ratio = 3,57
    setDX(150);
    setDY(150);

    sizeFactor = 1;

}

Cpostit::~Cpostit(){

}


bool Cpostit::run(void)
{
    Refresh_Display = true;
    return true;
}




/*****************************************************************************/
/* Initialize                                                                                                                    */
/*****************************************************************************/
bool Cpostit::init(void)
{


    AddLog(LOG_MASTER,"PostIt initializing...");
    qWarning()<<"PostIt initializing...";
    CPObject::init();
    mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0,0,1,11);


    HBL = new QVBoxLayout();
    HBL->setContentsMargins(10,20,10,0);



    edit = new CpostitTextEdit(this);
    edit->viewport()->setAutoFillBackground(false);
    edit->setFrameStyle(QFrame::NoFrame);
    edit->setTabStopWidth(edit->font().pointSize()*4);
    HBL->addWidget(edit);

    mainLayout->addLayout(HBL);
    // The QSizeGrip position (here Bottom Right Corner) determines its
    // orientation too
    mainLayout->addWidget(new CSizeGrip(this,":/core/size-black.png"), 0, Qt::AlignBottom | Qt::AlignRight);
    this->setLayout(mainLayout);
    AddLog(LOG_MASTER,"done.\n");



    return true;
}

/*****************************************************************************/
/* Exit                                                                                                                                          */
/*****************************************************************************/
bool Cpostit::exit(void)
{
    return true;
}

bool Cpostit::InitDisplay()
{

    CPObject::InitDisplay();

    return true;
}

bool Cpostit::UpdateFinalImage()
{
    CPObject::UpdateFinalImage();
    QPainter painter;
    painter.begin(FinalImage);
    QFont font("Arial", 6);
    painter.setFont(font);
    painter.setPen(Qt::black);
    QRect _rect = QRect(QPoint(50,50),this->size());
    painter.drawText(_rect,Qt::TextWordWrap,"TEXT:"+edit->toPlainText());

    painter.end();

    return true;
}


void Cpostit::paintEvent(QPaintEvent *event)
{
    float coeffx = getDX()*mainwindow->zoom/100/150.0;
    float coeffy = getDY()*mainwindow->zoom/100/150.0;
//    qWarning()<<"new margin:"<<newmargin<<"dx="<<getDX();
    mainLayout->setContentsMargins(0,0,1*coeffx,11*coeffy);
    HBL->setContentsMargins(coeffx*10,coeffy*20,coeffx*10,coeffy*0);
    CPObject::paintEvent(event);
}

void Cpostit::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu= new QMenu(this);

    BuildContextMenu(menu);

    menu->popup(event->globalPos () );
    event->accept();
}


bool Cpostit::SaveSession_File(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("session");
        xmlOut->writeAttribute("version", "2.0");
        QByteArray ba(edit->toHtml().toUtf8());
        xmlOut->writeTextElement("text",ba.toBase64());
    xmlOut->writeEndElement();  // session
    return true;
}

bool Cpostit::LoadSession_File(QXmlStreamReader *xmlIn)
{
    if (xmlIn->name()=="session") {
        if (xmlIn->readNextStartElement()) {
            if (xmlIn->name() == "text" ) {
                QByteArray ba = QByteArray::fromBase64(xmlIn->readElementText().toUtf8());
                edit->setHtml(QString(ba.data()));
            }
//            xmlIn->skipCurrentElement();
        }
    }

    UpdateFinalImage();
    emit updatedPObject(this);

    return true;
}



void Cpostit::slotDblSize()
{
    sizeFactor = 2;
    int w=getDX()*2;
    int h=getDY()*2;
    setDX(w);
    setDY(h);
    changeGeometry(posx(),posy(),getDX(),getDY());
}
