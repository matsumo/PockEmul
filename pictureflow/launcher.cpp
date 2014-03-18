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

 #include <QDebug>
 #include <QDir>

#include "mainwindowpockemul.h"
extern MainWindowPockemul *mainwindow;
#include "common.h"
 #include "launcher.h"

 Launcher::Launcher(QString id,QString executableName, QString caption, QString imageName, QStringList args,QString description)
 {
     idPocket = id;
     imagePath = imageName;
     appCaption = caption;
     this->description = description;

     if (executableName[0] == QLatin1Char('/'))
         executablePath = executableName;
     else
         executablePath = QDir::cleanPath(QDir::currentPath() + QLatin1Char('/') + executableName);

     arguments = args;

     process.setProcessChannelMode(QProcess::ForwardedChannels);

     QObject::connect( &process, SIGNAL(finished(int,QProcess::ExitStatus)),
                       this, SLOT(processFinished(int,QProcess::ExitStatus)));

     QObject::connect( &process, SIGNAL(error(QProcess::ProcessError)),
                       this, SLOT(processError(QProcess::ProcessError)));

     QObject::connect( &process, SIGNAL(started()), this, SLOT(processStarted()));
 }

 void Launcher::launch()
 {
     process.start(executablePath, arguments);
 }

 QImage* Launcher::getImage()
 {
//     return new QImage();
//     qWarning()<<"Image:"<<imagePath;
     return new QImage(P_RES(imagePath));
 }

 QString Launcher::getIdPocket()
 {
     return idPocket;
 }

 QStringList Launcher::getArguments()
 {
     return arguments;
 }

 QString Launcher::getCaption()
 {
     return appCaption;
 }
 QString Launcher::getDescription()
 {
     return description;
 }
 void Launcher::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
 {
     Q_UNUSED(exitCode);
     Q_UNUSED(exitStatus);

     emit demoFinished();

     QObject::disconnect(this, SIGNAL(demoStarted()), 0, 0);
     QObject::disconnect(this, SIGNAL(demoFinished()), 0, 0);
 }

 void Launcher::processError(QProcess::ProcessError err)
 {
     qDebug() << "Process error: " << err;
     if (err == QProcess::Crashed)
         emit demoFinished();
 }

 void Launcher::processStarted()
 {
     emit demoStarted();
 }
