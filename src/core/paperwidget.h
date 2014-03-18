#ifndef _PAPERWIDGET_H
#define _PAPERWIDGET_H

#include <QWidget>

class CPObject;
class TapAndHoldGesture;

class CpaperWidget:public QWidget{
Q_OBJECT
public:

	QImage *bufferImage;
	CPObject * pPC;
	
	void setOffset(QPoint);
	QPoint getOffset();
    QRect baseRect;
    bool updated;
	
    CpaperWidget(QRect rect,QImage * buffer,QWidget * parent=0);
	~CpaperWidget()
	{

	}
private slots:
	void contextMenuEvent ( QContextMenuEvent * event );
	void paperCopy();
    void paperCopyText();
	void paperCut();
	void paperSaveImage();
	void paperSaveText();
    void tapAndHold(QMouseEvent *);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent( QMouseEvent * event );
	void paintEvent(QPaintEvent *);
	
private:
	QPoint Offset;
    TapAndHoldGesture* _gestureHandler;
};


#endif

