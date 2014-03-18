#include "cstdlib.h"

#include "mainwindowpockemul.h"
extern MainWindowPockemul *mainwindow; /*!< TODO */

/*!
 \brief  ctor

 \fn Cstdlib::Cstdlib
*/
Cstdlib::Cstdlib(void) {

}

/*!
 \brief ctor
    create the object and load the corresponding libs

 \fn Cstdlib::Cstdlib
 \param targetmodel
*/
Cstdlib::Cstdlib(QString targetmodel) {
    this->model = targetmodel;
    LoadLibs();
}

/*!
 \brief

 \fn Cstdlib::setModel
 \param m
*/
void Cstdlib::setModel(QString m) {
    model = m;

    LoadLibs();
}


/*!
 \brief

 \fn Cstdlib::LoadLibs
*/
void Cstdlib::LoadLibs(void) {
    libmap.clear();

    // Fetch all libs available start with ressources then current filesystem
    //
    QDir dir(":/lcc/model/");
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();

    dir.setPath(":/lcc/model/"+model);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    list << dir.entryInfoList();

    dir.setPath("lcc/model/");
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    list << dir.entryInfoList();

    dir.setPath("lcc/model/"+model);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    list << dir.entryInfoList();

    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        QFile file(fileInfo.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            QMessageBox::about(mainwindow,"ERROR","ERROR loading standard lib :"+fileInfo.fileName());
        QByteArray line;
        while (!file.atEnd()) {
            line.append(file.readLine()+"\r\n");
        }
        libmap[fileInfo.fileName()]=line;
    }
}

bool Cstdlib::contains(QString op) {
    return libmap.contains(op);
}

QByteArray Cstdlib::getLib(QString op) {
    return libmap.value(op);
}
