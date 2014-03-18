#include "qoutpanel.h"

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
#include <QTextEdit>

#include "pobject.h"
#include "pcxxxx.h"

extern QList<CPObject *> listpPObject;

QCE_AUTO_REGISTER(QOutPanel)

/*!
    \brief Constructor
*/
QOutPanel::QOutPanel(QWidget *p)
 : QPanel(p)
{
    setFixedHeight(fontMetrics().lineSpacing()*10 + 4);

    out = new QTextEdit(this);
    QHBoxLayout *hboxLayout = new QHBoxLayout(this);
    hboxLayout->setContentsMargins(0,0,0,0);
    hboxLayout->addWidget(out);
}

/*!
    \brief Empty destructor
*/
QOutPanel::~QOutPanel()
{

}

/*!

*/
QString QOutPanel::type() const
{
    return "Out";
}

/*!

*/
void QOutPanel::editorChange(QEditor *e)
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
bool QOutPanel::paint(QPainter *p, QEditor *e)
{
//    qWarning("drawing Hex panel... [%i, %i, %i, %i]",
//            geometry().x(),
//            geometry().y(),
//            geometry().width(),
//            geometry().height());


    return true;
}

/*!

*/
void QOutPanel::mousePressEvent(QMouseEvent *e)
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
void QOutPanel::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e)

    editor()->setFocus();
}

/*! @} */
