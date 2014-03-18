// $Id: HexViewer.h 103 2006-05-16 19:00:53Z m9710797 $

#ifndef HEXVIEWER_H
#define HEXVIEWER_H

#include <QFrame>

class QScrollBar;
class QPaintEvent;

class HexViewer : public QFrame
{
	Q_OBJECT;
public:
	HexViewer(QWidget* parent = 0);

	void setData(const char* name, unsigned char* datPtr, int datLength,int offsetAddress = 0);
	void refresh();

public slots:
	void setLocation(int addr);

protected:
	void resizeEvent(QResizeEvent* e);
	void paintEvent(QPaintEvent* e);
	void wheelEvent( QWheelEvent * event );

private:
	void setScrollBarValues();

	QScrollBar* vertScrollBar;

	int frameL, frameR, frameT, frameB;
	short horBytes;
	double visibleLines;
	bool waitingForData;

	QString dataName;
	int hexTopAddress;
	unsigned char* hexData;
	int hexDataLength;
	int offsetAddress;

};

#endif // HEXVIEWER_H
