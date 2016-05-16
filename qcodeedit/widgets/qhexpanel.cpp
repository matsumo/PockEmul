#include "qhexpanel.h"

#include "qeditor.h"

#include "qdocument.h"
#include "qdocumentline.h"
#include "qdocumentcursor.h"

#include <QTimer>
#include <QPainter>
#include <QDateTime>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QApplication>
#include <QLayout>
#include <QtGui>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

#include "pobject.h"
#include "pcxxxx.h"

extern QList<CPObject *> listpPObject;

QCE_AUTO_REGISTER(QHexPanel)

/*!
    \brief Constructor
*/
QHexPanel::QHexPanel(QWidget *p)
 : QPanel(p)
{
    setFixedHeight(fontMetrics().lineSpacing()*10 + 4);
    setAttribute(Qt::WA_DeleteOnClose, true);
    hexeditor = new BINEditor::BinEditor(this);

    QHBoxLayout *hboxLayout = new QHBoxLayout(this);
    hboxLayout->setContentsMargins(0,0,0,0);
    hboxLayout->addWidget(hexeditor);
    QSpacerItem *spacerItem = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

    hboxLayout->addItem(spacerItem);

    QVBoxLayout *vboxLayout = new QVBoxLayout(this);
    hboxLayout->addLayout(vboxLayout);


    QPushButton *pbClose = new QPushButton();
    pbClose->setText("Close");
    pbClose->setMinimumSize(QSize(60, 0));
    pbClose->setMaximumSize(QSize(100, 16777215));
    vboxLayout->addWidget(pbClose);
    connect(pbClose,SIGNAL(clicked()),this,SLOT(close()));

    cbinstallTo = new QComboBox();
    cbinstallTo->setMinimumSize(QSize(60, 0));
    cbinstallTo->setMaximumSize(QSize(100, 16777215));
    vboxLayout->addWidget(cbinstallTo);

//    QHBoxLayout *hboxAdrLayout= new QHBoxLayout(this);
//    vboxLayout->addLayout(hboxAdrLayout);
//    hboxAdrLayout->addWidget(new QLabel("Adr:"));
    leTargetAdr = new QLineEdit();
    leTargetAdr->setMinimumSize(QSize(60, 0));
    leTargetAdr->setMaximumSize(QSize(100, 16777215));
//    hboxAdrLayout->addWidget(leTargetAdr);
//    hboxAdrLayout->setSizeConstraint();
    vboxLayout->addWidget(leTargetAdr);


    QPushButton *pbInstall = new QPushButton();
    pbInstall->setText("Install");
    pbInstall->setMinimumSize(QSize(60, 0));
    pbInstall->setMaximumSize(QSize(100, 16777215));
    vboxLayout->addWidget(pbInstall);

    connect(pbInstall,SIGNAL(clicked()),this,SLOT(install()));

    filltargetCB();
}

/*!
    \brief Empty destructor
*/
QHexPanel::~QHexPanel()
{
    delete hexeditor;
}

/*!

*/
QString QHexPanel::type() const
{
    return "Hex";
}

/*!

*/
void QHexPanel::editorChange(QEditor *e)
{
    //erase content ????
//	if ( editor() )
//	{
//		disconnect(	editor(), SIGNAL( cursorPositionChanged() ),
//					this	, SLOT  ( update() ) );

//	}

//	if ( e )
//	{
//		connect(e	, SIGNAL( cursorPositionChanged() ),
//				this, SLOT  ( update() ) );

//	}
}

/*!

*/
bool QHexPanel::paint(QPainter *p, QEditor *e)
{
//    qWarning("drawing Hex panel... [%i, %i, %i, %i]",
//            geometry().x(),
//            geometry().y(),
//            geometry().width(),
//            geometry().height());

#if 1
    //hexeditor->resize(geometry().size());

#else
    static QPixmap _warn(":/warning.png"), _mod(":/save.png");

    QString s;
    int xpos = 10;
    QDocumentCursor c = e->cursor();
    const QFontMetrics fm(fontMetrics());

    const int ls = fm.lineSpacing();
    const int ascent = fm.ascent() + 3;

    s = tr("Line : %1 Visual column : %2 Text column : %3")
            .arg(c.lineNumber() + 1)
            .arg(c.visualColumnNumber())
            .arg(c.columnNumber());

    p->drawText(xpos, ascent, s);
    xpos += fm.width(s) + 10;

    int sz = qMin(height(), _mod.height());
    //int lastMod = d->lastModified().secsTo(QDateTime::currentDateTime());
    //QString timeDiff = tr("(%1 min %2 s ago)").arg(lastMod / 60).arg(lastMod % 60);

    //xpos += 10;
    if ( e->isContentModified() )
    {
        p->drawPixmap(xpos, (height() - sz) / 2, sz, sz, _mod);
        //xpos += sz;
        //xpos += 10;
        //p->drawText(xpos, ascent, timeDiff);
    }
    xpos += sz + 10;
    //xpos += fm.width(timeDiff);
    //xpos += 20;

//	s = editor()->flag(QEditor::Overwrite) ? tr("OVERWRITE") : tr("INSERT");
//	p->drawText(xpos, ascent, s);
//	xpos += fm.width(s) + 10;


#endif
    return true;
}

/*!

*/
void QHexPanel::mousePressEvent(QMouseEvent *e)
{
    if ( !editor() || (e->button() != Qt::LeftButton) || !m_conflictSpot || e->x() < m_conflictSpot )
    {
        editor()->setFocus();
        return;
    }

    editor()->save();
}

/*!

*/
void QHexPanel::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e)

    editor()->setFocus();
}

void QHexPanel::newPocket(CPObject *pc)
{
    cbinstallTo->addItem(pc->getName(),tr("%1").arg((qlonglong)pc));
}

void QHexPanel::removePocket(CPObject *pc)
{
    cbinstallTo->removeItem(cbinstallTo->findData(tr("%1").arg((qlonglong)pc)));
}

void QHexPanel::install()
{
    int index = cbinstallTo->currentIndex();
    CpcXXXX *pc = (CpcXXXX *) cbinstallTo->itemData(index).toString().toLongLong();

    quint32 _adr = startadr;
    if (!leTargetAdr->text().isEmpty()) {
        _adr = leTargetAdr->text().toULong(0,16);
    }
    emit installTo(pc,_adr,hexeditor->data());
//    QDataStream in(hexeditor->data());
//    in.readRawData ((char *) &pc->mem[startadr],hexeditor->data().size() );
//    QMessageBox::about(this,"Transfert",tr("LM stored at %1").arg(startadr));
}

void QHexPanel::filltargetCB(void) {
    // update the injectCB ComboBox
    cbinstallTo->clear();
    for (int i = 0; i < listpPObject.size();i++) {
        CPObject *p = listpPObject.at(i);
        cbinstallTo->addItem(p->getName(),tr("%1").arg((qlonglong)p));
    }
}

/*! @} */
