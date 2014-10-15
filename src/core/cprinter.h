#ifndef _CPRINTER_H
#define _CPRINTER_H

#include <QColor>
#include "pobject.h"

class CpaperWidget;

class Cprinter:public CPObject{

Q_OBJECT

public:
    virtual void clearPaper(void) {}
    virtual void SaveAsText(void) {}

	CpaperWidget *paperWidget;
	
	Cprinter(CPObject *parent);
	virtual ~Cprinter();

    virtual bool    SaveSession_File(QXmlStreamWriter *xmlOut);
    virtual bool	LoadSession_File(QXmlStreamReader *xmlIn);

    virtual void resizeEvent ( QResizeEvent * );
    virtual void moveEvent(QMoveEvent *event);
	
    QRect pos;
    QRect PaperWidgetRect;

    void setPaperPos(QRect);
    QRect PaperPos();

    virtual void raise();

    QByteArray	TextBuffer;
public slots:
    void contextMenuEvent ( QContextMenuEvent * );

protected:
	QColor	PaperColor;
};


#endif
