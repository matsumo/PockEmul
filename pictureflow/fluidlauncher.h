/****************************************************************************
 **
 ** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the demonstration applications of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** GNU Lesser General Public License Usage
 ** This file may be used under the terms of the GNU Lesser General Public
 ** License version 2.1 as published by the Free Software Foundation and
 ** appearing in the file LICENSE.LGPL included in the packaging of this
 ** file. Please review the following information to ensure the GNU Lesser
 ** General Public License version 2.1 requirements will be met:
 ** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Nokia gives you certain additional
 ** rights. These rights are described in the Nokia Qt LGPL Exception
 ** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU General
 ** Public License version 3.0 as published by the Free Software Foundation
 ** and appearing in the file LICENSE.GPL included in the packaging of this
 ** file. Please review the following information to ensure the GNU General
 ** Public License version 3.0 requirements will be met:
 ** http://www.gnu.org/copyleft/gpl.html.
 **
 ** Other Usage
 ** Alternatively, this file may be used in accordance with the terms and
 ** conditions contained in a signed written agreement between you and Nokia.
 **
 **
 **
 **
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

 #ifndef FLUID_LAUNCHER_H
 #define FLUID_LAUNCHER_H

#include <qglobal.h>
#if QT_VERSION >= 0x050000
#   include <QtWidgets>
#else
#   include <QtCore>
#   include <QtGui>
#endif
 #include <QTimer>
 #include <QStringRef>

 #include "pictureflow.h"
 #include "pictureflow/launcher.h"

class CPObject;

 class FluidLauncher : public QStackedWidget
 {
     Q_OBJECT

 public:

     enum LaunchType
     {
         PictureFlowType,
         FileBrowserType
     };

     FluidLauncher(QWidget *,QStringList,LaunchType,QString connType=QString(),QString connGender=QString());
     ~FluidLauncher();

     void populateFileBrowser(QStringList param);
     QImage ExtractImage(QFileInfo fileInfo);
     QWidget *parentWidget;

 public slots:
     void launchApplication(int index);
     void demoFinished();
     void exitSlot();
     void computeresize();

 signals:
     void Launched(QString,CPObject *);
     void exit();

 private:

     QDir dir;

     PictureFlow* pictureFlowWidget;

     QList<Launcher*> demoList;

     LaunchType Type;

     bool loadConfig(QStringList config);
     void populatePictureFlow();
     void parseDemos(QXmlStreamReader& reader);

     QStringList Config;
    QString connType,connGender;
 };

 #endif
