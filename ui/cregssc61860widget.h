#ifndef CREGSSC61860WIDGET_H
#define CREGSSC61860WIDGET_H

#include <QWidget>

#include "ui/cregcpu.h"
class CCPU;
class CPObject;

namespace Ui {
    class Cregssc61860Widget;
}

class Cregssc61860Widget : public CregCPU
{
    Q_OBJECT

public:
    explicit Cregssc61860Widget(CPObject *parent = 0,CCPU *pCPU = 0);
    ~Cregssc61860Widget();

    void refresh();

private:
    Ui::Cregssc61860Widget *ui;
};

#endif // CREGSSC61860WIDGET_H
