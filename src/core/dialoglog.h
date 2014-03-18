#ifndef DIALOGLOG_H
#define DIALOGLOG_H
//
#include "ui_dialoglog.h"
//
class DialogLog : public QDialog, public Ui::DialogLog
{
    Q_OBJECT



public:
    DialogLog( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	int LogLevel;

private slots:
	void additem( QString );
    void addconsole( QString );
	int  computeLogLevel( QListWidgetItem * item );

private:
	void addCkeckableItem(QString,int);
	void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);
};
#endif
