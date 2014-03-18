#ifndef LAUNCHBUTTONWIDGET_H
#define LAUNCHBUTTONWIDGET_H

#include <QWidget>


class QTabWidget;

#include "fluidlauncher.h"


class LaunchButtonWidget:public QWidget
{
    Q_OBJECT

public:

    enum LaunchType
    {
        PictureFlow,
        FileBrowser,
        Action
    };



    LaunchButtonWidget(QWidget* parent,LaunchType type,QStringList param,QString img);
    ~LaunchButtonWidget();

    void mousePressEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *event);
    FluidLauncher* launcher;

    QStringList config;
    QString image;
    LaunchType type;

    void resizeEvent(QResizeEvent *event);
signals:
    void clicked();


};


#endif // LAUNCHBUTTONWIDGET_H
