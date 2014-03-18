#ifndef CREGSZ80WIDGET_H
#define CREGSZ80WIDGET_H

#include <QWidget>

#include "ui/cregcpu.h"
class CCPU;

namespace Ui {
    class Cregsz80Widget;
}

class Cregsz80Widget : public CregCPU
{
    Q_OBJECT

public:
    explicit Cregsz80Widget(CPObject *parent = 0,CCPU *pCPU = 0);
    ~Cregsz80Widget();

    void refresh();
private:
    Ui::Cregsz80Widget *ui;
};

#endif // CREGSZ80WIDGET_H
