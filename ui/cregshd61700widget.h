#ifndef CREGSHD61700WIDGET_H
#define CREGSHD61700WIDGET_H

#include <QWidget>

#include "ui/cregcpu.h"
class CCPU;

namespace Ui {
    class Cregshd61700Widget;
}

class Cregshd61700Widget : public CregCPU
{
    Q_OBJECT

public:
    explicit Cregshd61700Widget(CPObject *parent = 0,CCPU *pCPU = 0);
    ~Cregshd61700Widget();

    void refresh();
private:
    Ui::Cregshd61700Widget *ui;
};

#endif // CREGSHD61700WIDGET_H
