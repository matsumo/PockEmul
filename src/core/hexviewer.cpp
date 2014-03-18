// $Id: HexViewer.cpp 134 2006-09-08 22:38:19Z edwinv $

#include <QScrollBar>
#include <QPaintEvent>
#include <QPainter>
#include <cmath>

#include "hexviewer.h"


HexViewer::HexViewer(QWidget* parent)
	: QFrame(parent)
{
	setFrameStyle(WinPanel | Sunken);
	setFocusPolicy(Qt::StrongFocus);
	setBackgroundRole(QPalette::Base);

	setFont(QFont("Courier New", 10));

	horBytes = 16;
	hexTopAddress = 0;
	offsetAddress = 0;
	waitingForData = false;
	hexDataLength = 0;
	
	vertScrollBar = new QScrollBar(Qt::Vertical, this);
	vertScrollBar->hide();
	
	frameL = frameT = frameB = frameWidth();
	frameR = frameL + vertScrollBar->sizeHint().width();

	connect(vertScrollBar, SIGNAL(valueChanged(int)), this, SLOT(setLocation(int)));
}

void HexViewer::wheelEvent( QWheelEvent * event )
{
	int numDegrees = event->delta() / 8;
	int numSteps = numDegrees / 15;

	vertScrollBar->setValue( vertScrollBar->value() -  numSteps * vertScrollBar->singleStep());
}

void HexViewer::setScrollBarValues()
{
	vertScrollBar->setMinimum(0);

	visibleLines = double(height() - frameT - frameB) / fontMetrics().height();
	
	int maxLine = int(ceil(double(hexDataLength) / horBytes)) - int(visibleLines);
	if (maxLine < 0) maxLine = 0;
	vertScrollBar->setMaximum(maxLine);
	vertScrollBar->setSingleStep(1);
	vertScrollBar->setPageStep(int(visibleLines));
}

void HexViewer::resizeEvent(QResizeEvent* e)
{
	QFrame::resizeEvent(e);

	setScrollBarValues();
	vertScrollBar->setGeometry(width() - frameR, frameT,
	                           vertScrollBar->sizeHint().width(),
	                           height() - frameT - frameB);
	vertScrollBar->show();
	// calc the number of lines that can be displayed
	// partial lines count as a whole
}

void HexViewer::paintEvent(QPaintEvent* e)
{
	// call parent for drawing the actual frame
	QFrame::paintEvent(e);

	QPainter p(this);
	int h = fontMetrics().height();
	int d = fontMetrics().descent();

	// set font
	p.setPen(Qt::black);

	// calc and set drawing bounds
	QRect r(e->rect());
	if (r.left() < frameL) r.setLeft(frameL);
	if (r.top()  < frameT) r.setTop (frameT);
	if (r.right()  > width()  - frameR - 1) r.setRight (width()  - frameR - 1);
	if (r.bottom() > height() - frameB - 1) r.setBottom(height() - frameB - 1);
	p.setClipRect(r);

	// redraw background
	p.fillRect( r, palette().color(QPalette::Base) );

	// calc layout (not optimal)
	int charWidth = fontMetrics().width("A");
	int spacing = charWidth / 2;
	int xAddr = frameL + 8;
	int xHex1 = xAddr + 12 * charWidth;
	int dHex = 2 * charWidth + spacing;

	int y = frameT + h - 1;
	
	p.drawLine(xHex1 - spacing, 2 , xHex1 - spacing , height() - 2);
	
	int address = hexTopAddress;

	for (int i = 0; i < int(ceil(visibleLines)); ++i) {
		// print address
		QString hexStr;
		hexStr.sprintf("%05X:%05X", offsetAddress + address, address);
		p.drawText(xAddr, y - d, hexStr);
		// print bytes
		int x = xHex1;
		for (int addr = address; addr < address + horBytes; ++addr) {
			// at extra spacing halfway
			if (!(horBytes & 1)) {
				if (addr - address == horBytes / 2) {
					x += spacing;
				}
			}
			// print data (if there still is any)
			if (addr < hexDataLength) {
				hexStr.sprintf("%02X", hexData[addr]);
				p.drawText(x, y - d, hexStr);
			}
			x += dHex;
		}
		x += 2 * spacing;
		hexStr.clear();
		for (int addr = address; addr < address + horBytes; ++addr) {
			if (addr >= hexDataLength) break;
			unsigned char chr = hexData[addr];
			if (chr < 32 || chr > 127) chr = '.';
			hexStr += chr;
		}
		p.drawText(x, y - d, hexStr);
		y += h;
		address += horBytes;
		if (address >= hexDataLength) break;
	}
}

void HexViewer::setData(const char* name, unsigned char* datPtr, int datLength, int offset)
{
	dataName = name;
	hexData = datPtr;
	hexDataLength = datLength;
	offsetAddress = offset;
	setScrollBarValues();
}

void HexViewer::setLocation(int addr)
{	
	if (!waitingForData)
	{
		int start = addr * horBytes;
		int size = horBytes * int(ceil(visibleLines));

		if (start + size > hexDataLength)
		{
			size = hexDataLength - start;
		}

		hexTopAddress = start;
		update();
	}
}

void HexViewer::refresh()
{
	setLocation(vertScrollBar->value());
}
