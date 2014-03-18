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

 #include <QXmlStreamReader>
#if QT_VERSION >= 0x050000
#   include <QtConcurrent/QtConcurrent>
#else

#endif

 #include "fluidlauncher.h"
#include "init.h"
#include "mainwindowpockemul.h"

extern MainWindowPockemul *mainwindow;
extern void m_openURL(QUrl url);
extern void Vibrate();

 #define DEFAULT_INPUT_TIMEOUT 10000
 #define SIZING_FACTOR_HEIGHT 6/12
 #define SIZING_FACTOR_WIDTH 6/12


FluidLauncher::FluidLauncher(QWidget * parent, QStringList config, LaunchType type,QString connType,QString connGender):QStackedWidget(parent)
{
//    qWarning("CFL 1\n");

    Type = type;
    Config = config;
    this->connType = connType;
    this->connGender = connGender;

    pictureFlowWidget = new PictureFlow();

    addWidget(pictureFlowWidget);

    setCurrentWidget(pictureFlowWidget);
    pictureFlowWidget->setFocus();
    QObject::connect(pictureFlowWidget, SIGNAL(itemActivated(int)), this, SLOT(launchApplication(int)));

    parentWidget = pictureFlowWidget;
//    qWarning()<<"computeResise:"<<parent->size();
    QRect screen_size = parent->geometry();//QApplication::desktop()->screenGeometry();
    resize(parent->size());

//    qWarning()<<screen_size;
    //    qWarning()<<mainwindow->centralwidget->geometry();

    const int h = screen_size.height() * SIZING_FACTOR_HEIGHT;
    const int w = screen_size.width() * SIZING_FACTOR_WIDTH;

    const int hh = qMin(h, w);
    const int ww = hh / 3 * 4;
    pictureFlowWidget->setSlideSize(QSize(ww, hh));

    connect(mainwindow,SIGNAL(resizeSignal()),this,SLOT(computeresize()));

    if (type == PictureFlowType) {
        bool success;
//qWarning()<<"Berfore LoadConfig";
        success = loadConfig(Config);
//qWarning()<<"After LoadConfig";
        if (success) {
#ifdef Q_OS_ANDROID
qWarning()<<"Berfore PopulatePictureFlow";
    QtConcurrent::run(this,&FluidLauncher::populatePictureFlow);
qWarning()<<"After PopulatePictureFlow";
#else
            populatePictureFlow();
#endif
            //        qWarning("CFL 4\n");
            //        show();
        }else { pictureFlowWidget->close();  }
    }
    else
        if (type == FileBrowserType) {
            populateFileBrowser(Config);
        }
}




 FluidLauncher::~FluidLauncher()
 {
//     qWarning("Delete pictureFlowWidget");
     delete pictureFlowWidget;
     // Delete demolist
     for (int i=0;i<demoList.count();i++) {
         delete demoList.at(i);
     }
 }

 void FluidLauncher::computeresize() {

     if (pictureFlowWidget) {
//         qWarning()<<"computeResise:"<<pictureFlowWidget->size();
         QRect screen_size = mainwindow->geometry();//QApplication::desktop()->screenGeometry();
         this->resize(mainwindow->size());
         pictureFlowWidget->resize(mainwindow->size());

//         qWarning()<<screen_size;
         //    qWarning()<<mainwindow->centralwidget->geometry();



         const int h = screen_size.height() * SIZING_FACTOR_HEIGHT;
         const int w = screen_size.width() * SIZING_FACTOR_WIDTH;

         const int hh = qMin(h, w);
         const int ww = hh / 3 * 4;
         if (pictureFlowWidget)
             pictureFlowWidget->setSlideSize(QSize(ww, hh));
     }
 }

 bool FluidLauncher::loadConfig(QStringList config)
 {
     QString configPath = config.at(0);
//     qWarning("loadConfig \n");
     QFile xmlFile(configPath);

     if (!xmlFile.exists() || (xmlFile.error() != QFile::NoError)) {
         qDebug() << "ERROR: Unable to open config file " << configPath;
         return false;
     }

     xmlFile.open(QIODevice::ReadOnly);
     QXmlStreamReader reader(&xmlFile);
     while (!reader.atEnd()) {
         reader.readNext();

         if (reader.isStartElement()) {
             if (reader.name() == "demos")
                 parseDemos(reader);
         }
     }

     if (reader.hasError()) {
        qDebug() << QString("Error parsing %1 on line %2 column %3: \n%4")
                 .arg(configPath)
                 .arg(reader.lineNumber())
                 .arg(reader.columnNumber())
                 .arg(reader.errorString());
     }

     // Append an exit Item
//     Launcher* exitItem = new Launcher("EXIT",QString(), QLatin1String("Exit"), ":/cover/exit.png", QStringList());
//     demoList.append(exitItem);

//     qWarning("nb slide:%i   %i\n",pictureFlowWidget->slideCount(),demoList.count());
     return true;
 }

 void FluidLauncher::parseDemos(QXmlStreamReader& reader)
 {
     while (!reader.atEnd()) {
         reader.readNext();
         if (reader.isStartElement() && reader.name() == "example") {
             QXmlStreamAttributes attrs = reader.attributes();
             QStringRef filename = attrs.value("filename");
             if (!filename.isEmpty()) {
                 QStringRef name = attrs.value("name");
                 QStringRef image = attrs.value("image");
                 QStringRef args = attrs.value("args");
                 QStringRef idpocket = attrs.value("idpocket");
                 QStringRef desc = attrs.value("desc");
                 QStringRef _connectortype = attrs.value("connectortype");
                 QStringRef _conngender = attrs.value("conngender");

                 // filter on connectors type and gender
                 if (!connType.isEmpty() && (_connectortype.indexOf(connType)==-1)) continue;
//                 qWarning()<<connType<<" found:"<<_connectortype;
                 if (!connGender.isEmpty() && (_conngender.indexOf(connGender)==-1)) continue;
//                 qWarning()<<"included";
                 Launcher* newDemo = new Launcher(
                             idpocket.toString(),
                             filename.toString(),
                             name.isEmpty() ? "Unnamed Demo" : name.toString(),
                             image.toString(),
                             args.toString().split(" "),
                             desc.toString());
                 demoList.append(newDemo);

             }
         } else if(reader.isEndElement() && reader.name() == "demos") {
             return;
         }
     }
 }

 QImage FluidLauncher::ExtractImage(QFileInfo fileInfo) {
     QImage img;

     if (fileInfo.completeSuffix()=="pdf") {
         return QImage(":/core/pdfthumbnail.png");
     }

     if (fileInfo.completeSuffix()=="pml") {
         QFile file(fileInfo.fileName());

         file.open(QIODevice::ReadOnly);
         QXmlStreamReader xml;
         xml.setDevice(&file);
         while (!xml.atEnd()) {
             if (xml.readNextStartElement()) {
                 QString elt = xml.name().toString();
                 if (elt=="snapshot")  {
                     if (xml.attributes().value("format").toString()=="JPG")
                         img.loadFromData(QByteArray::fromBase64(xml.readElementText().toLatin1()),"JPG");
                     else
                         img.loadFromData(QByteArray::fromBase64(xml.readElementText().toLatin1()),"PNG");
                     break;
                 }
             }
         }
         file.close();
     }
     return img;
 }


 void FluidLauncher::populateFileBrowser(QStringList config) {
//qWarning("populateFileBrowser\n");

     Config = config;

     QString path = Config.at(0);
     QString filter = Config.at(1);
     dir.cd(path);
     //     QMessageBox::information(mainwindow,"new path",dir.absolutePath(),QMessageBox::Ok);
     dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::AllDirs | QDir::NoDot);
     dir.setSorting(QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

     QFileInfoList list = dir.entryInfoList(QStringList() << filter);

     pictureFlowWidget->setSlideCount(list.count());

     for (int i = 0; i < list.size(); ++i) {
         QFileInfo fileInfo = list.at(i);
         QImage *img = new QImage();

         if (fileInfo.isFile()) {
             img = new QImage(ExtractImage(fileInfo));
         }
         else if (fileInfo.isDir()) {
             img->load(":/core/folder.png");
         }

         QStringList sl;
         if (fileInfo.isDir()) sl.append("Dir");
         if (fileInfo.isFile()) sl.append("File");

         Launcher* newDemo = new Launcher(
                     list.at(i).fileName(),
                     list.at(i).fileName(),
                     list.at(i).fileName(),
                     "",
                     sl);
         demoList.append(newDemo);
         pictureFlowWidget->setSlide(i, *img);
         pictureFlowWidget->setSlideCaption(i, list.at(i).fileName());
         delete img;
     }
     pictureFlowWidget->setCurrentSlide(list.count()/2);
 }

 void FluidLauncher::populatePictureFlow()
 {

     pictureFlowWidget->setSlideCount(demoList.count());

     for (int i=demoList.count()-1; i>=0; --i) {
//         qWarning()<<"Before reading image:";
         QImage *img = demoList.at(i)->getImage();
//         qWarning()<<"After reading image:";
         pictureFlowWidget->setSlide(i, *img);
         pictureFlowWidget->setSlideCaption(i, demoList[i]->getCaption());
         pictureFlowWidget->setSlideDescription(i,demoList[i]->getDescription());
         delete img;
//         qWarning()<<"After assigning image:";
     }

     pictureFlowWidget->setCurrentSlide(demoList.count()/2);

 }

 void FluidLauncher::launchApplication(int index)
 {

Vibrate();
     // NOTE: Clearing the caches will free up more memory for the demo but will cause
     // a delay upon returning, as items are reloaded.
     //pictureFlowWidget->clearCaches();

     if ( (index==-1) )//||(index == demoList.size() -1))
     {
         hide();
         return;
     }

#if 0
     QObject::connect(demoList[index], SIGNAL(demoFinished()), this, SLOT(demoFinished()));

     demoList[index]->launch();
#endif

     if (Type == PictureFlowType) {
         int result = EMPTY;
         QString ItemText = demoList[index]->getIdPocket();

         //TODO: try to implement sub level
         // if ItemText start with a specic tag, load the corresponding xml and display the new pictureflow.
         // Esc should exit


         CPObject *_pc=0;

         if (mainwindow->objtable.contains(ItemText))
             result = mainwindow->objtable.value(ItemText);

         if (result != EMPTY)	{
             _pc=mainwindow->LoadPocket(result);
             //         parentWidget()->close();
         }
         emit Launched(ItemText,_pc);
         close();
     }

     if (Type == FileBrowserType) {
//         qWarning("launchApplication: %i  t=%i\n",index,demoList.size());
         if (index >= demoList.size()) {
             QMessageBox::information(mainwindow,"ERREUR",QString("%1").arg(index),QMessageBox::Ok);
             return;
         }
         if (demoList.at(index)->getArguments().contains("Dir")) {
             pictureFlowWidget->clear();
             QString newpath = demoList.at(index)->getIdPocket();
//             QMessageBox::information(mainwindow,"test",newpath,QMessageBox::Ok);
             demoList.clear();
             Config[0]+="/"+newpath;
             populateFileBrowser(Config);
         }
         else {
             if (Config.at(1)=="*.pml") {
                 mainwindow->opensession(pictureFlowWidget->getSlideCaption(index));
             }
             if (Config.at(1)=="*.pdf") {
//                 qWarning()<<"open pdf:"<<Config.at(0)+"/"+pictureFlowWidget->getSlideCaption(index);
                 QUrl url = QUrl::fromLocalFile(Config.at(0)+"/"+pictureFlowWidget->getSlideCaption(index));
                 //QDesktopServices::openUrl(url);
                 m_openURL(url);
             }
             close();
         }
     }

 }

 void FluidLauncher::exitSlot()
 {
//     parentWidget()->close();
     emit exit();
     close();
 }

 void FluidLauncher::demoFinished()
 {
     setCurrentWidget(pictureFlowWidget);

     // Bring the Fluidlauncher to the foreground to allow selecting another demo
     raise();
     activateWindow();
 }


