#include <qglobal.h>
#if QT_VERSION >= 0x050000
#   include <QtWidgets>
#else
#   include <QtCore>
#   include <QtGui>
#endif
#include "paperwidget.h"
#include "cprinter.h"
#include "Log.h"

CpaperWidget::CpaperWidget(QRect rect,QImage * buffer,QWidget * parent):QWidget(parent)
{
    pPC = (CPObject *)parent;
    bufferImage = buffer;

    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    resize(rect.width(),rect.height());
    move(rect.x(),rect.y());
    Offset = QPoint(0,0);
    this->baseRect = rect;
    grabGesture(Qt::TapAndHoldGesture);
    updated = true;
}

bool CpaperWidget::event(QEvent *event)
{

    if (event->type() == QEvent::Gesture) {

        if (QGesture *tap = (static_cast<QGestureEvent*>(event))->gesture(Qt::TapAndHoldGesture)) {
            const QPoint pos = (static_cast<QTapAndHoldGesture *>(tap))->position().toPoint();
            //             qWarning()<< (static_cast<QTapAndHoldGesture *>(tap))->timeout()<<pos<<tap->gestureType()<<tap->state();
            if (tap->state() == Qt::GestureStarted) {
                QContextMenuEvent cme(
                            QContextMenuEvent::Mouse,
                            pos,
                            (pos));
                QApplication::sendEvent(this,&cme);

                setCursor(Qt::ArrowCursor);

                event->accept();
            }
        }

        event->accept();
        return true;
    }
    return QWidget::event(event);
}

void CpaperWidget::setOffset(QPoint val)
{
	Offset = val;
}

QPoint CpaperWidget::getOffset()
{
	return Offset;
}

void CpaperWidget::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu = new QMenu(this);
    menu->addAction(tr("Copy Image"),this,SLOT(paperCopy()));
    menu->addAction(tr("Copy Text"),this,SLOT(paperCopyText()));
    menu->addAction(tr("Cut"),this,SLOT(paperCut()));
    menu->addAction(tr("Save Image ..."),this,SLOT(paperSaveImage()));
    menu->addAction(tr("Save Text ..."),this,SLOT(paperSaveText()));
    menu->popup(event->globalPos ());
}

void CpaperWidget::paperCopy()
{
	
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setImage(*bufferImage);
}

void CpaperWidget::paperCopyText()
{

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString(((Cprinter*)pPC)->TextBuffer));
}
void CpaperWidget::paperCut()
{
	paperCopy();
	// Erase paper
	// Initialise position
	((Cprinter*)pPC)->clearPaper ();
	
	update();
}
void CpaperWidget::paperSaveImage()
{

	QString s = QFileDialog::getSaveFileName(
                    this,
                    tr("Choose a filename to save under"),
                    ".",
                    "Images (*.png)");
                    
	bufferImage->save(s,"PNG",100);
	
}
void CpaperWidget::paperSaveText()
{
	// Call the printer SaveAsText Virtual function
	((Cprinter*)pPC)->SaveAsText();
	
}

void CpaperWidget::paintEvent(QPaintEvent *event)
{
//    if (!updated) return;
    if (bufferImage == 0) return;
    updated = false;

	float ratio = ( (float) width() ) / ( bufferImage->width() - Offset.x() );
	
	QRect source = QRect( QPoint(Offset.x() , Offset.y()  - height() / ratio ) , QPoint( bufferImage->width() , Offset.y() ) );
	
	QPainter painter;
	
	painter.begin(this);
	
	painter.drawImage(rect(),bufferImage->copy(source).scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation ));

	painter.end();
}
