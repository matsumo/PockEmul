#ifndef _PAPERWIDGET_H
#define _PAPERWIDGET_H

#include <QWidget>

class CPObject;

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

    bool event(QEvent *event);

private slots:
	void contextMenuEvent ( QContextMenuEvent * event );
	void paperCopy();
    void paperCopyText();
	void paperCut();
	void paperSaveImage();
    void paperSaveText();

protected:
	void paintEvent(QPaintEvent *);
	
private:
    QPoint Offset;
};


#endif

