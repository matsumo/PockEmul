#ifndef DIALOGDASM_H
#define DIALOGDASM_H

#include <QDialog>
class QListWidgetItem;
class CpcXXXX;
class CregCPU;

namespace BINEditor {
    class BinEditor;
}

namespace Ui {
    class DialogDasm;
}

class DialogDasm : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDasm(QWidget *parent = 0);
    ~DialogDasm();


    bool IsAdrInList(qint32 adr);
    CpcXXXX * pPC;
    int		Index;
    quint32	MaxAdr;
    quint32	NextMaxAdr;
    bool imem;

    void resizeEvent(QResizeEvent *event);
    void selectRow(int index);
    void saveAll();
    void load();

    void refreshBreakPoints();
private:
    Ui::DialogDasm *ui;
    BINEditor::BinEditor *imemHexEditor;
    BINEditor::BinEditor *memHexEditor;
    CregCPU *regwidget;

public slots:
    void RefreshDasm();
    void ManualDasm();
    void loadImem();
    void loadMem();
    void start();
    void stop();
    void step();
    void stepOver();

    void Update(int adr, uchar val);

    void addBreakPoint();
    void breakPointChanged(QListWidgetItem* item);
    void addTraceRange();
    void traceRangeChanged(QListWidgetItem* item);

    void addSymbolFile();
    void removeSymbolFile();
    void loadSymbolMap();
    void removeBreakPoint();
    void removeTraceRange();
    void breakPointSelect();
    void evaluateCond();
};

#endif // DIALOGDASM_H
