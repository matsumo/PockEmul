#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include <QQuickWidget>
#include <QQuickView>
#include "cloud/cloudwindow.h"
class CPObject;
class CViewObject;

class CrenderView : public QQuickWidget
{
    Q_OBJECT

public:
    CrenderView(QWidget *parent = 0);

//    QObject *object;
    QWidget * parent;
    Cloud cloud;


    Q_INVOKABLE bool keyAt(QString Id, int x, int y);
public slots:
    Q_INVOKABLE void newpocketSlot();
    Q_INVOKABLE void newextSlot();
    Q_INVOKABLE void loadSlot();

    Q_INVOKABLE void warning(QString msg);
    Q_INVOKABLE void keypressed(QString Id, int k, int m, int scan);
    Q_INVOKABLE void keyreleased(QString Id, int k, int m, int scan);
    Q_INVOKABLE void contextMenu(QString Id, int x, int y);
    Q_INVOKABLE void rotpocket(QString Id, int x);
    Q_INVOKABLE void click(QString Id, int x, int y);
    Q_INVOKABLE void unclick(QString Id, int x, int y);
    Q_INVOKABLE void dblclick(QString Id, int x, int y);
    Q_INVOKABLE void setzoom(int x,int y,int z);
    Q_INVOKABLE void movepocket(QString Id, int x, int y);
    Q_INVOKABLE void moveallpocket(int x, int y);
    void pocketUpdated(CViewObject *pObject);
    void newPObject(CPObject *pObject);
    void delPObject(CPObject *pObject);
    void movePObject(CViewObject*pObject, QPointF pos);
    void sizePObject(CViewObject*pObject, QSizeF size);
    void showPObject(CViewObject *pObject);
    void hidePObject(CViewObject *pObject);
    void stackPosChanged();
    void cloudClose();


};

#endif // RENDERVIEW_H
