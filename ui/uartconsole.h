#ifndef UARTCONSOLE_H
#define UARTCONSOLE_H

#include <QDialog>

namespace Ui {
    class CUartConsole;
}
class Cuart;

class CUartConsole : public QDialog
{
    Q_OBJECT

public:
    explicit CUartConsole(QWidget *parent = 0);
    ~CUartConsole();
public slots:
    void newOutputByte(qint8 data);
    void newInputByte();
private:
    Ui::CUartConsole *ui;
    Cuart *pUart;
};

#endif // UARTCONSOLE_H
