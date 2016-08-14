//TODO  Several tabs to record different input at the same time

#include <QDebug>
#include <QPainter> 
#include <QMouseEvent>
#include <QFileDialog>

#include "dialoganalog.h"
#include "Log.h"
#include "common.h"
#include "pcxxxx.h"
#include "Inter.h"
#include "Connect.h"
#include "sio.h"
#include "watchpoint.h"

#define DEFAULT_POINTS_PER_SAMPLE 10
//
class CPObject;
extern QList<CPObject *> listpPObject; 

dialogAnalog::dialogAnalog( int nbbits,QWidget * parent, Qt::WindowFlags f) : QDialog(parent, f),
    QQuickImageProvider(QQuickImageProvider::Pixmap)
{
	setupUi(this);

    connect(chkBCapture, SIGNAL(stateChanged(int)), this, SLOT(updatecapture(int))); 
    connect(pbZoomIn,   SIGNAL(clicked()), this, SLOT(zoomin()));
    connect(pbZoomOut,  SIGNAL(clicked()), this, SLOT(zoomout()));
    connect(pbFit,      SIGNAL(clicked()), this, SLOT(fitmarkers()));
    connect(hlScrollBar,SIGNAL(valueChanged(int)), this, SLOT(scroll(int)));
    connect(pbSave,     SIGNAL(clicked()), this, SLOT(slotSave()));
    connect(pbLoad,     SIGNAL(clicked()), this, SLOT(slotLoad()));
    connect(pbMarker,   SIGNAL(clicked()), this, SLOT(slotMarker()));
    connect(twWatchPoint,SIGNAL(currentItemChanged ( QTreeWidgetItem * , QTreeWidgetItem * )), this, SLOT(slotChangeWatchPoint( QTreeWidgetItem * , QTreeWidgetItem * )));

    connect(mainwindow,SIGNAL(DestroySignal(CPObject*)),this,SLOT(DestroySlot(CPObject*)));

	Capture = false; 
    NbBits = nbbits;
    m_zoom = 1.0;
    pPC = 0;
    currentWatchPoint = 0;
    currentWatchPointSize=8;
    fill_twWatchPoint();
    twWatchPoint->expandAll();
}

QPixmap dialogAnalog::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size)
    Q_UNUSED(requestedSize)

    qWarning()<<"size"<<*size<<requestedSize;
    if (requestedSize.isEmpty()) return QPixmap();

    size->setWidth(requestedSize.width());
    size->setHeight(requestedSize.height());
    plot(true,requestedSize);

    return screenPixmap;
}



void dialogAnalog::slotChangeWatchPoint( QTreeWidgetItem * current , QTreeWidgetItem * previous)
{
    if (current) {
        int pos = current->data(0,Qt::UserRole).toInt();
        currentWatchPoint = WatchPoint.items.at( pos ).Point;
        currentWatchPointSize = WatchPoint.items.at( pos ).PointSize;
        NbBits = WatchPoint.items.at( pos ).nbBits;
        pPC = WatchPoint.items.at( pos ).PObject;
        currentlabels = WatchPoint.items.at( pos ).Labels;
    }
}

void dialogAnalog::DestroySlot(CPObject *pObject)
{
    fill_twWatchPoint();
    update();
}

void dialogAnalog::fill_twWatchPoint(void)
{
    twWatchPoint->clear();
	twWatchPoint->setColumnCount(1);

	
	for (int i = 0; i < listpPObject.size(); i++)
	{
		QTreeWidgetItem *material = new QTreeWidgetItem(twWatchPoint,QStringList(listpPObject.at(i)->getName()));
        for (int j = 0; j < WatchPoint.items.size(); j++)
		{
            if (listpPObject.at(i) == WatchPoint.items.at(j).PObject)
			{
                QTreeWidgetItem *Point = new QTreeWidgetItem(material,QStringList(WatchPoint.items.at(j).WatchPointName));
				Point->setData(0,Qt::UserRole,j);
			}
		}
	}
    if (twWatchPoint->topLevelItemCount()>3) {
        twWatchPoint->collapseAll();
    }
    twWatchPoint->header()->hide();
}

void dialogAnalog::slotMarker(void)
{
	dataplot.Marker = 1;
}

void    dialogAnalog::setMarker(quint8 val) {
    dataplot.Marker = val;
}

quint8 dialogAnalog::getMarker(void) {
    return dataplot.Marker;
}

void dialogAnalog::slotSave(void)
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                            ".",
                            tr("Analogic Sample (*.ana)"));

	QFile file(fileName);
 	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);

	// Write a header with a "magic number" and a version
//	out << (quint32)0xA0B0C0D0;
//	out << (qint32)123;

    //out.setVersion(QDataStream::Qt_4_0);

	// Write the data
    out << NbBits;
	out << dataplot;
}

void dialogAnalog::slotLoad(void)
{
    if (capture()) {
        QMessageBox::warning(mainwindow, "PockEmul",
                             tr("Capture is running.\n") +
                             tr("Stop capture before loading a sample") );
        return;
    }
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                 ".",
                                                 tr("Analogic Sample (*.ana)"));


	QFile file(fileName);
	file.open(QIODevice::ReadOnly);
	QDataStream in(&file);

    mainwindow->analogMutex.lock();
	// Read the data
    in >> NbBits;
	in >> dataplot;
    mainwindow->analogMutex.unlock();

	m_zoom = 1.0;
	ComputeScrollBar();
	this->update();
}


void dialogAnalog::paintEvent(QPaintEvent *event)
{
	QPainter p;
	
	plot(true,frame_dataview->size());

    p.begin(this);
    p.drawPixmap(frame_dataview->mapTo ( this,QPoint(0,0) ), screenPixmap); 
    p.end();

}

// -------------------------------------------------------------------------------------------------
void dialogAnalog::mousePressEvent(QMouseEvent* evt)
{
    if (evt->button() == Qt::LeftButton || evt->button() == Qt::RightButton)
    {
        if (evt->button() == Qt::LeftButton)
        {
            setLeftMarker(frame_dataview->mapFrom( this,QPoint(evt->x(),0)).x());
        }
        else
        {
            setRightMarker(frame_dataview->mapFrom( this,QPoint(evt->x(),0)).x());
        }
        plot(false,frame_dataview->size());
        this->update();
    }
}

void dialogAnalog::wheelEvent( QWheelEvent * event )
{
	int numDegrees = event->delta() / 8;
	int numSteps = numDegrees / 15;

	hlScrollBar->setValue( hlScrollBar->value() -  numSteps * hlScrollBar->singleStep());
}

void dialogAnalog::scroll(int value)
{
	this->update();
}

void dialogAnalog::updatecapture(int state)
{
    mainwindow->analogMutex.lock();

	if (state == Qt::Checked )
	{
		setCapture(true);
		dataplot.Clear();
		pbZoomIn->setEnabled(false);
		pbZoomOut->setEnabled(false);
		pbFit->setEnabled(false);
		pbSave->setEnabled(false);
		pbLoad->setEnabled(false);
		groupBox->setEnabled(false);
		chkBShowMarker->setEnabled(false);
		
	}
	else
	{
		setCapture(false);
		// Store timeref
        dataplot.timeUnit = ( pPC->pTIMER->CPUSpeed * pPC->getfrequency() );
		ComputeScrollBar();
		
		pbZoomIn->setEnabled(true);
		pbZoomOut->setEnabled(true);
		pbFit->setEnabled(true);
		pbSave->setEnabled(true);
		pbLoad->setEnabled(true);
		groupBox->setEnabled(true);
		chkBShowMarker->setEnabled(true);

		this->update();
	}
    mainwindow->analogMutex.unlock();
}

void dialogAnalog::captureData(void)
{
	if ( !pPC) return;
    mainwindow->analogMutex.lock();
	if (Capture)
	{

		switch (currentWatchPointSize)
		{
			case 8 : dataplot.Write(*((qint8 *) currentWatchPoint),pPC->pTIMER->state); break;
			case 16: dataplot.Write(*((qint16*) currentWatchPoint),pPC->pTIMER->state); break;
			case 32: dataplot.Write(*((qint32*) currentWatchPoint),pPC->pTIMER->state); break;
			case 64: dataplot.Write(*currentWatchPoint,pPC->pTIMER->state); break;
		}
	}
    mainwindow->analogMutex.unlock();
}
	
void dialogAnalog::plot(bool forceRedraw,QSize size)
{
    QPainter p;
    screenPixmap = QPixmap( size);

	if (forceRedraw)
	{
		initPixmap(size);
        QColor greenColor(Qt::green);
        QPen dataPen1( greenColor);
		fillPixmap(&dataplot,&dataPen1);
#if 0
	    QPen dataPen2(QColor(Qt::red));
	    dataPen2.setStyle(Qt::DashDotDotLine);
		fillPixmap(&dataplot2,&dataPen2);
#endif
	}
	
    p.begin(&screenPixmap);
    p.drawPixmap(0, 0, lastPixmap);
    drawMarkers(&p);
    p.end();	// draw markers
	


}

#define READ_BIT(b,p)	( ((b)>>(p)) & 0x01 ? 1 :0 )
void dialogAnalog::fillPixmap(CData *data, QPen *dataPen)
{
    mainwindow->analogMutex.lock();
	TAnalog_Data plot,next_plot;
	QPainter painter;
    int heightPerField = lastPixmap.height() / NbBits;
    int dataview_width = frame_dataview->width();
	
	painter.begin(&lastPixmap);
	painter.setPen(*dataPen);

    QVector< QVector<QPoint> > polyline(64);

    Qt::CheckState csMarker = chkBShowMarker->checkState();

    for (int j=1;j<data->size();j++)
	{
		plot = data->Read(j-1);
		next_plot = data->Read(j);

		int current = heightPerField;
		int X1,Y1,X2,Y2;
		
		X1=StateToX(plot.state);
		X2=StateToX(next_plot.state);

        // Crop to the visible area
        if (!( (j>1) && ( (X2<0) || ( X1>dataview_width)))) {

            for (int jj=0;jj<NbBits;jj++)
            {
                //#define READ_BIT(b,p)	( ((b)>>(p)) & 0x01 ? 1 :0 )
                Y1= current - READ_BIT(plot.values,jj)* 3 * heightPerField / 5;
                Y2= current - READ_BIT(next_plot.values,jj)* 3 * heightPerField / 5;
                //painter.drawLine(X1,Y1,X2,Y1);
                //painter.drawLine(X2,Y1,X2,Y2);
                polyline[jj].append( QPoint(X1,Y1) );
                polyline[jj].append( QPoint(X2,Y1) );
                current += heightPerField;
            }

            // plot the Markers
            // Need to optimize
            if ( plot.marker && (csMarker == Qt::Checked) ) {
                QPen pen((Qt::white));
                pen.setStyle(Qt::DotLine);
                painter.setPen(pen);
                painter.drawLine(X1,12,X1,height());
                // set font ------------------------------------------------------------------------------------
                QFont textFont;
                textFont.setPixelSize(10);
                painter.setFont(textFont);
                painter.drawText(X1, 11, QString::number(plot.marker,10));
                painter.setPen(*dataPen);
            }
        }
	}
    if (polyline.size()) {
        for (int jj=0;jj<NbBits;jj++)
        {
            if (polyline[jj].size())
                painter.drawPolyline(polyline[jj].data(),polyline[jj].size());
        }
    }
	painter.end();
    mainwindow->analogMutex.unlock();
}

void dialogAnalog::initPixmap(QSize size)
{
	// resize the pixmap
	//	complete it
	// Draw plot
	QPainter painter;
    int heightPerField = size.height() / NbBits;
	
	lastPixmap=QPixmap( size.width(), size.height() );
	lastPixmap.fill(Qt::black);
	painter.begin(&lastPixmap);
	
    // set font ------------------------------------------------------------------------------------
    QFont textFont;
    textFont.setPixelSize(heightPerField / 2);
    painter.setFont(textFont);

    // set the needed pens -------------------------------------------------------------------------
	QPen linePen(QColor(100,100,100));
    QPen textPen(QColor(255, 255, 255));
    QPen gridPen(QColor(100, 100, 100));
    QColor greenColor(Qt::green);
    QPen dataPen(greenColor);

	// draw the fields and the text ----------------------------------------------------------------
    {
        int current = heightPerField;
        for (int i = 0; i < (NbBits - 1); i++)
        {
            painter.setPen(linePen);
            painter.drawLine(0, current, size.width(), current);
            
            painter.setPen(textPen);
            QString lbl = QString::number(i+1);
            if (currentlabels.contains(i+1)) lbl += "-"+currentlabels[i+1];
            painter.drawText(10, current - heightPerField / 3, lbl);
            current += heightPerField;
        }
        painter.setPen(textPen);
        painter.drawText(10, current - 15, QString::number(NbBits));
    }

	painter.end();
}

void dialogAnalog::drawLeftMarker(QPainter* painter)
{
    
    if (m_leftMarker != -1)
    {
		QPen leftMarkerPen(QColor(0,255,0));
		painter->setPen(leftMarkerPen);
        painter->drawLine( m_leftMarker, 0, m_leftMarker, height());
    }
}

void dialogAnalog::drawRightMarker(QPainter* painter)
{
    if (m_rightMarker != -1)
    {
		QPen rightMarkerPen(QColor(255,0,0));
		painter->setPen(rightMarkerPen);
        painter->drawLine( m_rightMarker, 0, m_rightMarker, height());
    }
}

void dialogAnalog::drawMarkers(QPainter* painter)
{
	drawLeftMarker(painter);
	drawRightMarker(painter);
}

int dialogAnalog::getLeftMarker() { return m_leftMarker; }
int dialogAnalog::getRightMarker(){ return m_rightMarker;}

void dialogAnalog::setLeftMarker(int markerpos)
{
	m_leftMarker = markerpos;
	ComputeMarkersLength();
}

// -------------------------------------------------------------------------------------------------
void dialogAnalog::setRightMarker(int markerpos)
{
	m_rightMarker = markerpos;
	ComputeMarkersLength();
}

void dialogAnalog::ComputeMarkersLength(void)
{
	long LMarkerState,RMarkerState;
	
	if (m_leftMarker == -1)
		LMarkerState = dataplot.Read_state(0);
	else
		LMarkerState = XToState(m_leftMarker);

	if (m_rightMarker == -1)
		RMarkerState = dataplot.Read_state(dataplot.size()-1);
	else
		RMarkerState = XToState(m_rightMarker);

	long deltaState = qAbs(RMarkerState - LMarkerState);
	
    MarkersLength = deltaState / dataplot.timeUnit * 1000.0f;
	
	labelLength->setText(tr("%1 ms").arg(MarkersLength,0,'f',5));
}

void dialogAnalog::ComputeScrollBar(void)
{
	// define scrollbar min,max,step
	long min,max,pagestep;
	
	min = dataplot.Read_state(0);
	pagestep =  (long) ( ( dataplot.Read_state(dataplot.size()-1) - min ) / m_zoom );
	max = dataplot.Read_state(dataplot.size()-1) - pagestep;
	
	hlScrollBar->setMinimum(min);
	hlScrollBar->setMaximum(max);
	hlScrollBar->setPageStep(pagestep);
	hlScrollBar->setSingleStep (pagestep / 50 );
	
}

void dialogAnalog::fitmarkers(void)
{
	int new_pos = XToState(m_leftMarker);
	
	m_zoom = (float)( dataplot.Read_state(dataplot.size()-1) - dataplot.Read_state(0) ) / ( XToState(m_rightMarker) - XToState(m_leftMarker) );
	ComputeScrollBar();
	hlScrollBar->setValue(new_pos);
	setLeftMarker(0);
	setRightMarker(frame_dataview->width());
	this->update();
}

void dialogAnalog::zoomin(void)
{
	m_zoom *= 2;
	ComputeScrollBar();	
	this->update();	
}
void dialogAnalog::zoomout(void)
{
	m_zoom /= 2;
	if (m_zoom <1.0) m_zoom=1.0;
	ComputeScrollBar();		
	this->update();	
	
}

void dialogAnalog::setCapture(bool val)	{

    Capture = val;
}
bool dialogAnalog::capture(void)		{ return Capture; }

long dialogAnalog::StateToX(long plotState)
{ 
	float ratio = (float) frame_dataview->width() / hlScrollBar->pageStep();
    int loc_X;
	loc_X = (int) (( plotState-hlScrollBar->value() ) * ratio);
	
	return loc_X;
}

long dialogAnalog::XToState(long x)
{ 
    long loc_State;
	float ratio = (float) frame_dataview->width() / hlScrollBar->pageStep();
	loc_State= (long) (((float)x / ratio) + hlScrollBar->value());
    return loc_State;
}

