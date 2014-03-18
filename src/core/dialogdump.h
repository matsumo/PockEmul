#ifndef DIALOGDUMP_H
#define DIALOGDUMP_H
//
#include "ui_dialogdump.h"
//#include "qhexedit.h"
#include "bineditor/bineditor.h"

//class HexViewer;
class CpcXXXX;
class CPObject;
//
class DialogDump : public QDialog, public Ui::DialogDump
{
Q_OBJECT
public:
    DialogDump( QWidget * parent = 0, Qt::WindowFlags f = 0 );


    BINEditor::BinEditor *hexeditor;


private slots:
	void slotDump( QTableWidgetItem * , QTableWidgetItem * );
	void LoadBin( void );
	void SaveBin( void );
    void Find( void );
    void FindNext( void );
    void FindPrevious(void);
    void JumpTo( void);
    void Refresh(void);
    void Fill(void);
    void Update(int adr, uchar val);

protected:
	void resizeEvent(QResizeEvent *);

private:
//	HexWidget *hexeditor;
	void LoadSlot(void);
    CPObject *pPC;
    int findpos;
};
#endif





