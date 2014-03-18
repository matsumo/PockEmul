#ifndef DIALOGABOUT_H
#define DIALOGABOUT_H
//
#include "ui_about.h"
//
class DialogAbout : public QDialog, public Ui::DialogAbout
{
Q_OBJECT
public:
    DialogAbout( QWidget * parent = 0, Qt::WindowFlags f = 0 );
private slots:
};
#endif
