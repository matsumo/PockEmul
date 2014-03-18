#ifndef DIALOGSIMULATOR_H
#define DIALOGSIMULATOR_H



#include "ui_simulatorconsole.h"
#include "cesimu.h"


class DialogSimulator : public QDialog, public Ui::DialogSimulator
{
Q_OBJECT
public:
    DialogSimulator( QWidget * parent = 0, Qt::WindowFlags f = 0 );

    void refresh( void);

private slots:
    void	clicked( QAbstractButton *);

protected:
    void closeEvent(QCloseEvent *event);
    int currentIndex;
public:
    Ccesimu *   pCeSimu;
};

#endif // DIALOGSIMULATOR_H
