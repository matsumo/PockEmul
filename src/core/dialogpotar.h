#ifndef DIALOGPOTAR_H
#define DIALOGPOTAR_H


#include <QDialog>
#include "potar.h"

namespace Ui {
    class DialogPotar;
}

class DialogPotar : public QDialog {
    Q_OBJECT
public:
    Cpotar  *pPotar;

    DialogPotar(QWidget *parent = 0);
    ~DialogPotar();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DialogPotar *ui;

private slots:
    void slotSlider(int);
};


#endif // DIALOGPOTAR_H
