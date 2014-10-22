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
    QList<QRect>	getkeyFoundRect(void);
	
	bool	keyFound;
    QList<CKey>::iterator keyIter;
    QList<QRect> listRect;

    CKey findKey(QString desc);
private slots:
    void	slotSelectKey();
	void	slotInitSize();
	void	slotDelKey();
    void    slotApplySize();

    void moveUp(void);
    void moveDown(void);
    void moveLeft(void);
    void moveRight(void);

    void showAll(void);

	
protected:
	void closeEvent(QCloseEvent *event);
	
private:
	CPObject *pPC;

};
#endif





