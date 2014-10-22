#ifndef DIALOGKEYLIST_H
#define DIALOGKEYLIST_H
//
#include "ui_dialogkeylist.h"

//
class CKey;
class CPObject;

class DialogKeyList : public QDialog, public Ui::DialogKeyList
{
Q_OBJECT
public:
    DialogKeyList( CPObject * parent = 0, Qt::WindowFlags f = 0 );
	
	void	InsertKeys(void);
	void	FindKey(QListWidgetItem * item);
	QRect	getkeyFoundRect(void);
	
	bool	keyFound;
    QList<CKey>::iterator keyIter;

private slots:
	void	slotSelectKey(QListWidgetItem * item,QListWidgetItem *);
	void	slotInitSize();
	void	slotDelKey();
    void	slotHorResize(int width);
    void	slotVerResize(int height);

    void moveUp(void);
    void moveDown(void);
    void moveLeft(void);
    void moveRight(void);

	
protected:
	void closeEvent(QCloseEvent *event);
	
private:
	CPObject *pPC;

};
#endif





