#ifndef DIALOGSTARTUP_H
#define DIALOGSTARTUP_H
//
#include "ui_startup.h"
//
class DialogStartup : public QDialog, public Ui::DialogStartup
{
Q_OBJECT
public:
    DialogStartup( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	
	void create(QString);
	
private slots:
//	void on_okButton_clicked();
//	void on_cancelButton_clicked();
	void on_listWidget_itemDoubleClicked();
	void on_listWidget_2_itemDoubleClicked();
	void on_listWidget_3_itemDoubleClicked();
};
#endif
