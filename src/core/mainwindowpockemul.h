#ifndef MAINWINDOWPOCKEMUL_H
#define MAINWINDOWPOCKEMUL_H

#include <QMutex>
#include <QPoint>
#include <QGesture>
#include <QGestureEvent>
#include <QXmlStreamReader>


//
#include "ui_pockemul.h"
#include "common.h"
#include "init.h"


class DialogLog;
class CpaperWidget;
class dialogAnalog;
class CPObject;
class CDirectLink;
class CKey;
class CpcXXXX;
class Cconnector;
#ifdef P_IDE
class WindowIDE;
#endif
class LaunchButtonWidget;
class QCommandLine;
class DownloadManager;
class ServeurTcp;
class CloudWindow;
class CrenderView;

#if QT_VERSION >= 0x050000
class QSensor;
#endif
#ifdef P_AVOID
namespace Avoid {
        class Router;
        class ShapeRef;
}
#endif


class MainWindowPockemul : public QMainWindow, public Ui::MainWindow
{
Q_OBJECT
public:
    QMutex audioMutex,analogMutex;

    MainWindowPockemul( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~MainWindowPockemul();
#ifdef P_AVOID
    Avoid::Router *router;
    QHash<CPObject *,Avoid::ShapeRef *> shapeRefList;
#endif

    void setZoom(int );
    void setCpu(int );

    DialogLog		*dialoglog;
    dialogAnalog	*dialoganalogic;
#ifdef P_IDE
    WindowIDE       *windowide;
#endif
    ServeurTcp      *server;
    CloudWindow     *cloud;
    CrenderView     *view;


    //	CpaperWidget	*paperWidget;

    QTimer *FrameTimer;

    CDirectLink		*pdirectLink;
    void SendSignal_AddLogItem(QString str);
    void SendSignal_AddLogConsole(QString str);

    void slotUnlink(CPObject *);

    quint64	rawclk;
    CPObject * LoadPocket(int result);
    CPObject *LoadPocket(QString Id);
    CPObject * InitApp(int idPC);

    ASKYN saveAll;
    float	zoom;

    QSlider *zoomSlider;

    void doZoom(QPoint point, float delta, int step=10);

    bool event(QEvent *event);
    bool gestureEvent(QGestureEvent *event);
    void pinchTriggered(QPinchGesture *gesture);
    void MoveAll(QPoint p);

    void initCommandLine();

    QCommandLine *cmdline;

    QHash<QString, int> objtable;

    void initObjectTable();

#if QT_VERSION >= 0x050000
    QSensor *sensor;
#endif
    QString saveassessionString();
public slots:
    QString saveassession();
    void saveassession(QXmlStreamWriter *xml);
    void opensession(QString sessionFN=QString());
    void opensession(QXmlStreamReader *xml);
    void quitPockEmul();

private slots:

    void checkReading();
    void gestureDetected(QString);
    void about();
    void Log();
    void IDE();
    void CloudSlot();
    void Analogic();
    void CheckUpdates();
    void Minimize_All();
    bool Close_All();
    void resetZoom();
    void SelectPocket(QAction *);
    int  newsession();

    void updateTimer();
    void updateFrameTimer();
    void updateTimeTimer();
    void resizeSlot(QSize size , CPObject *pObject);
    void DestroySlot(CPObject *pObject);
    void slotNewLink(QAction *);
    void slotWebLink(QAction *);
    void slotDocument(QAction *);
    void slotUnLink(QAction *);
    void slotUnLink(Cconnector *);
    void slotMsgError(QString);

    void parseError(const QString &error);
    void paramFound(const QString &name, const QVariant &value);
    void optionFound(const QString &name, const QVariant &value);
    void switchFound(const QString &name);

protected:
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent( QMouseEvent * event );
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *event);



signals:
    void AddLogItem(QString);
    void AddLogConsole(QString);
    void DestroySignal(CPObject *pObject);
    void NewPObjectsSignal(CPObject *pObject);
    void resizeSignal();

private:
    QList<CKey>::iterator keyIterator;
    bool startKeyDrag;
    QPoint KeyDrag;
    bool startPosDrag;
    QPoint PosDrag;
    qreal scaleFactor;



};
#endif
