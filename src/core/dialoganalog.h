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


#include <QQuickImageProvider>



class dialogAnalog : public QDialog, public Ui::DialogAnalog, public QQuickImageProvider
{
Q_OBJECT
public:
    dialogAnalog( int nbbits,QWidget * parent = 0, Qt::WindowFlags f = 0 );
	
    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize);

    //	CAnalog plotview;
	
	void	plot(bool,QSize);
	void	setCapture(bool);
	bool	capture(void);
	void	captureData(void);
	
	//// marker handling ------------------------------------------------------------------------
    int		getLeftMarker() ;
    int		getRightMarker() ;

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


public slots:
    Q_INVOKABLE void	setLeftMarker(int markerPosition);
    Q_INVOKABLE void	setRightMarker(int markerPosition);

    Q_INVOKABLE void	updatecapture(int);
    Q_INVOKABLE void	zoomin(void);
    Q_INVOKABLE void	zoomout(void);
    Q_INVOKABLE void	fitmarkers(void);
	void	scroll(int);
    Q_INVOKABLE void	slotSave();
    Q_INVOKABLE void	slotLoad();
    Q_INVOKABLE void	slotMarker();
    Q_INVOKABLE void updateDrawMarkers(int value);
	void	slotChangeWatchPoint( QTreeWidgetItem * , QTreeWidgetItem * );
    Q_INVOKABLE void slotChangeWatchPoint(int pos);

    void DestroySlot(CPObject *);
    void CreateSlot(CPObject *_pc);

signals:
    Q_INVOKABLE void refreshLogic();
    Q_INVOKABLE void markersLengthChanged(QString);

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

    int csMarker;

};
#endif

