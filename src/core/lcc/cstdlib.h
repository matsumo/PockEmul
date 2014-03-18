#ifndef CSTDLIB_H
#define CSTDLIB_H

#include <QtGui>

class Cstdlib{
public:
    Cstdlib(void);
    Cstdlib(QString targetmodel);

    void setModel(QString m);
    QString getModel(void) { return model;}

    void LoadLibs(void);
    bool contains(QString op);
    QByteArray getLib(QString op);


private:
    QString model;
    QMap<QString,QByteArray> libmap;

};

#endif // CSTDLIB_H
