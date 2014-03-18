#ifndef DIALOGIMPL_H
#define DIALOGIMPL_H
//
#include "ui_startup.h"
//
class DialogImpl : public QDialog, public Ui::Dialog
{
Q_OBJECT
public:
	DialogImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
private slots:
	void on_okButton_clicked();
	void on_cancelButton_clicked();
};
#endif
