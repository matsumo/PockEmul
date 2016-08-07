/****************************************************************************
 **
 ** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** You may use this file under the terms of the BSD license as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
 **     the names of its contributors may be used to endorse or promote
 **     products derived from this software without specific prior written
 **     permission.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include <QtGui>

#include <stdio.h>
#include "downloadmanager.h"
#include "mainwindowpockemul.h"
#include "renderView.h"


extern MainWindowPockemul *mainwindow;
extern int ask(QWidget *parent,QString msg,int nbButton);
extern CrenderView *view;

 DownloadManager::DownloadManager()
 {
     connect(&manager, SIGNAL(finished(QNetworkReply*)),
             SLOT(downloadFinished(QNetworkReply*)));
     abortPB = new QPushButton(tr("Cancel"),mainwindow->centralWidget());
     abortPB->setEnabled(true);
     abortPB->setVisible(false);
     progress = new QProgressBar(mainwindow->centralwidget);
     progress->setVisible(false);
     progress->setFormat("%v / %m");
     connect(abortPB,SIGNAL(clicked()),this,SLOT(abort()));
     
     if (view) {
         // opengl qml
         QObject::connect(view->cloud.object, SIGNAL(sendDownloadAbort()), this, SLOT(abort()));
     }
     resize();
 }

 void DownloadManager::resize() {
     progress->setGeometry(0,0,mainwindow->centralwidget->width()-abortPB->sizeHint().width()-5,abortPB->sizeHint().height());
     abortPB->setGeometry(
                 QRect(QPoint(mainwindow->centralwidget->width()-abortPB->sizeHint().width(),0),
                       abortPB->sizeHint()));
 }

 void DownloadManager::doDownload(const QUrl &url)
 {
     QNetworkRequest request(url);
     QNetworkReply *reply = manager.get(request);

     connect(reply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(downloadProgress(qint64,qint64)));


     currentDownloads.append(reply);
     bytesReceived.insert(reply,0);
     bytesTotal.insert(reply,0);

     progress->show();
     abortPB->show();
     QMetaObject::invokeMethod(view->cloud.object, "setDownloadVisible",
                               Q_ARG(QVariant, true)
                               );
 }

 QString DownloadManager::saveFileName(const QUrl &url)
 {
     QString path = url.path();
     QString basename = targetDir+"/"+QFileInfo(path).fileName();

     if (basename.isEmpty())
         basename = "download";

     if (QFile::exists(basename)) {
         // already exists, don't overwrite
         int i = 0;
         basename += '.';
         while (QFile::exists(basename + QString::number(i)))
             ++i;

         basename += QString::number(i);
     }

     return basename;
 }

 bool DownloadManager::saveToDisk(const QString &filename, QIODevice *data)
 {
     QFile file(filename);
     if (!file.open(QIODevice::WriteOnly)) {
         QMessageBox::warning(mainwindow,
                              tr("Download Manager"),
                              tr("Could not open %1 for writing: %2").arg(filename).arg(file.errorString()));
         return false;
     }

     file.write(data->readAll());
     file.close();

     return true;
 }

 void DownloadManager::abort()
 {
     qWarning()<<"Abort Download";
     while (!currentDownloads.isEmpty()) {
         currentDownloads.first()->abort();
        currentDownloads.removeAt(0);
     }
 }

 void DownloadManager::downloadFinished(QNetworkReply *reply)
 {
     QUrl url = reply->url();
     if (reply->error()) {
         ask(mainwindow,tr("Download of %1 failed: %2").
             arg(url.toEncoded().constData()).arg(reply->errorString()),1);

//         QMessageBox::warning(mainwindow,tr("Download Manager"),
//                              tr("Download of %1 failed: %2").
//                              arg(url.toEncoded().constData()).arg(reply->errorString()));
     } else {
         QString filename = saveFileName(url);
         if (saveToDisk(filename, reply))
             ask(mainwindow,tr("Download of %1 succeeded (saved to %2)").
                                      arg(url.toEncoded().constData()).arg(filename),1);
//             QMessageBox::information(mainwindow,tr("Download Manager"),
//                                      tr("Download of %1 succeeded (saved to %2)").
//                                      arg(url.toEncoded().constData()).arg(filename));
     }

     currentDownloads.removeAll(reply);
     bytesReceived.remove(reply);
     bytesTotal.remove(reply);
     if (currentDownloads.isEmpty()) {
         progress->hide();
         abortPB->hide();
         QMetaObject::invokeMethod(view->cloud.object, "setDownloadVisible",
                                   Q_ARG(QVariant, false)
                                   );
     }
     reply->deleteLater();

 }

 void DownloadManager::downloadProgress( qint64 received, qint64 total )
 {
//     qWarning()<< "Transfert : "<<received<< "/" << total;
     bytesReceived[(QNetworkReply*)QObject::sender()] = received;
     bytesTotal[(QNetworkReply*)QObject::sender()] = total;

     qint64 Total = 0;
     qint64 Received=0;
     for (int i = 0;i<currentDownloads.size();i++) {
         Total+=bytesTotal[currentDownloads[i]];
         Received += bytesReceived[currentDownloads[i]];
     }
     progress->setMinimum(0);
     progress->setMaximum(Total);
     progress->setValue(Received);
     if (total >1000000) {
         progress->setFormat(QString("%1 / %2 Mb").arg(Received/1000000.0,0,'g',2).arg(Total/1000000.0,0,'g',2));
     }

     progress->update();

     QMetaObject::invokeMethod(view->cloud.object, "setDownloadProgress",
                               Q_ARG(QVariant, 0),
                               Q_ARG(QVariant, Total/1000),
                               Q_ARG(QVariant, Received/1000)
                               );
 }


