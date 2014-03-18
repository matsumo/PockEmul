#ifndef DIALOGSTRARTUP_H
#define DIALOGSTRARTUP_H
//
#include "ui_startup.h"
//
class DialogStrartup : public QDialog, public Ui::Dialog
{
Q_OBJECT
public:
	DialogStrartup( QWidget * parent = 0, Qt::WFlags f = 0 );
private slots:
};
#endif
