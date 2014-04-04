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
#if QT_VERSION >= 0x050000
class QSensor;
#endif
#ifdef P_AVOID
namespace Avoid {
        class Router;
        class ShapeRef;
}
#endif

enum Models {
    EMPTY,

    PC1211,
    PC1245,
    MC2200,
    PC1250,
    PC1251,
    PC1255,
    TandyPC3,
    TandyPC3EXT,
    PC1260,
    PC1261,
    PC1262,
    PC1280,
    PC1350,
    PC1360,
    PC1401,
    PC1402,
    PC1403,
    PC1421,
    PC1403H,
    PC1450,
    PC1475,
    PC1425,
    PC1500,
    PC1500A,
    TandyPC2,
    PC1251H,
    PC1600,
    PC2500,

    CE120P,
    CE122,
    CE125,
    MP220,
    CE126P,
    CE123P,
    CE129P,
    CE140P,
    CE140F,
    CE150,
    CE152,
    CE153,
    CE162E,
    CE127R,
    TANDY263591,

    SerialConsole,
    CABLE11Pins,
    POTAR,
    Simulator,

    CE1600P,
    X07,
    X710,
    E500,
    E500S,
    E550,
    G850V,
    PB1000,
    PB2000,
    MD100,
    FP100,
    FP40,
    FP200,
    FX890P,
    Z1,
    Z1GR,
    PC2001,
    PC2021,
    PC2081,
    LBC1100,
    CL1000,
    TPC8300,
    TP83,
    RLH1000,
    RLP4002,
    RLP6001,
    RLP9001,
    RLP9002,
    RLP9003,
    RLP9003R,
    RLP9004,
    RLP9005,
    RLP9006,
    RLP1004A,
    RLP1002,
    RLP2001,
    POSTIT,
    TI57,
    HP41,
    HP82143A,
    CE1560
};

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

    QHash<QString, Models> objtable;

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
