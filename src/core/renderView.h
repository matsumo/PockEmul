#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include <QQuickWidget>
#include <QQuickView>
#include <QMutex>

#include "cloud/cloudwindow.h"
#include "vibrator.h"

class CPObject;
class CViewObject;
class BinaryData;

class CrenderView : public QQuickWidget
{
    Q_OBJECT

public:
    CrenderView(QWidget *parent = 0);

//    QObject *object;
    QWidget * parent;
    Cloud cloud;
    Vibrator vibrator;

    Q_INVOKABLE bool keyAt(QString Id, int x, int y);
    Q_INVOKABLE void flip(QString Id, int dir);
    Q_INVOKABLE double getZoom();
    Q_INVOKABLE QString getReleaseNotes(QString _fn);
    Q_INVOKABLE void test();
    Q_INVOKABLE QString getRes(QString _fn);
    Q_INVOKABLE void changeGeo(int,int,int,int);

    Q_INVOKABLE void sendTrackingEvent(const QString &cat = QString(),
                                       const QString &action = QString(),
                                       const QString &label = QString(),
                                       const QVariant &value =QVariant());

    Q_INVOKABLE void loadSlot(QString Id, int slotNumber, BinaryData* display);

    void fillSlotList();
public slots:
    Q_INVOKABLE void newpocketSlot();
    Q_INVOKABLE void newextSlot();
    Q_INVOKABLE void loadSlot();
    Q_INVOKABLE void saveSlot();
    Q_INVOKABLE void bookcaseSlot();
    Q_INVOKABLE void checkSlot();

    Q_INVOKABLE void warning(QString msg);
    Q_INVOKABLE void keypressed(QString Id, int k, int m, int scan);
    Q_INVOKABLE void keyreleased(QString Id, int k, int m, int scan);
    Q_INVOKABLE void contextMenu(QString Id, int x, int y);
    Q_INVOKABLE void rotpocket(QString Id, int x);
    Q_INVOKABLE void click(QString Id, int touchId, int x, int y);
    Q_INVOKABLE void unclick(QString Id, int touchId, int x, int y);
    Q_INVOKABLE void dblclick(QString Id, int x, int y);
    Q_INVOKABLE void setzoom(int x, int y, double z);
    Q_INVOKABLE void movepocket(QString Id, int x, int y);
    Q_INVOKABLE void moveallpocket(int x, int y);
    Q_INVOKABLE void maximize(QString Id);
    Q_INVOKABLE void minimize(QString Id);
    Q_INVOKABLE void fit();
    void pocketUpdated(CViewObject *pObject);
    void newPObject(CPObject *pObject);
    void delPObject(CPObject *pObject);
    void movePObject(CViewObject*pObject, QPointF pos);
    void sizePObject(CViewObject*pObject, QSizeF size);
    void showPObject(CViewObject *pObject);
    void hidePObject(CViewObject *pObject);
    void pickExtension(QString brand);
    void pickExtensionConnector(QString brand);
    void stackPosChanged();
    void cloudClose();
    void rotPObject(CViewObject *pObject,int angle);

    void LoadPocket(QString id);
    Q_INVOKABLE void disableKeyboard(QString Id);
    Q_INVOKABLE void enableKeyboard(QString Id);

signals:
    void Launched(QString,CPObject *);

private:
     QMap<int,QPoint> mapTouch;

     QMutex lockClick;
};

#endif // RENDERVIEW_H
