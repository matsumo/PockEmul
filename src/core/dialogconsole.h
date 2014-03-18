#ifndef DIALOGCONSOLE_H
#define DIALOGCONSOLE_H
//
#include <QMutex>

#include "ui_serialconsole.h"

class Csio;
//
class DialogConsole : public QDialog, public Ui::DialogConsole
{
Q_OBJECT
public:
    DialogConsole( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	
	void refresh( void);

    QMutex refreshMutex;
    enum PARITY { NONE,EVEN,ODD};

    PARITY parity;
private slots:
    void parityToggle(bool checked);
	void	sendData( void);
    void    openFile(void);
    void	stopStream( void);
    void    changeBaudrate(QString);
    void    updateMapCD(QString);
    void    updateMapCS(QString);
    void    updateMapER(QString);
    void    updateMapRD(QString);
    void    updateMapRR(QString);
    void    updateMapRS(QString);
    void    updateMapSD(QString);


protected:
	void closeEvent(QCloseEvent *event);
	Csio *pSIO;
    int currentIndex;
};
#endif
