#ifndef DAILOGANALOG_H
#define DAILOGANALOG_H
//
#include "ui_dialoganalog.h"
//
#include "common.h"
#include "analog.h"
#include "init.h"
#include "Log.h"
class CpcXXXX;
class CPObject;





class dialogAnalog : public QDialog, public Ui::DialogAnalog
{
Q_OBJECT
public:
    dialogAnalog( int nbbits,QWidget * parent = 0, Qt::WindowFlags f = 0 );
	
//	CAnalog plotview;
	
	void	plot(bool,QSize);
	void	setCapture(bool);
	bool	capture(void);
	void	captureData(void);
	
	//// marker handling ------------------------------------------------------------------------
	int		getLeftMarker() ;
	void	setLeftMarker(int markerPosition);
	int		getRightMarker() ;
	void	setRightMarker(int markerPosition);

    void    setMarker(quint8);
    quint8   getMarker(void);

	void	drawMarkers(QPainter* painter);
	void	drawLeftMarker(QPainter* painter);
	void	drawRightMarker(QPainter* painter);
	void	ComputeMarkersLength(void);

	void	ComputeScrollBar(void);
	long	StateToX(long plotState);
	long	XToState(long x);
	
	QPixmap	lastPixmap;
	QPixmap	screenPixmap;
	CData	dataplot;
	CData	dataplot2;


	void	fill_twWatchPoint(void);

	
private slots:
	void	updatecapture(int);
	void	zoomin(void);
	void	zoomout(void);
	void	fitmarkers(void);
	void	scroll(int);
	void	slotSave();
	void	slotLoad();
	void	slotMarker();
	void	slotChangeWatchPoint( QTreeWidgetItem * , QTreeWidgetItem * );
    void    DestroySlot(CPObject *pObject);

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent* evt);
	void wheelEvent( QWheelEvent * event );

private:
	long	m_leftMarker;
	long	m_rightMarker;
	float	m_zoom;
	float	MarkersLength;
	bool	Capture;	
	void	initPixmap(QSize);
	void	fillPixmap(CData *, QPen *);
	int		NbBits;
	CPObject *pPC;
    qint64 * currentWatchPoint;
    qint8	currentWatchPointSize;
    QHash<int,QString> currentlabels;

};
#endif

