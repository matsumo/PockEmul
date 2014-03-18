#ifndef QHEXPANEL_H
#define QHEXPANEL_H

#include <QComboBox>

#include "qpanel.h"
#include "bineditor/bineditor.h"

class QDocumentLine;
class CPObject;
class CpcXXXX;

class QCE_EXPORT QHexPanel : public QPanel
{
    Q_OBJECT

public:
    Q_PANEL(QHexPanel, "Hex Panel")

    QHexPanel(QWidget *p = 0);
    virtual ~QHexPanel();

    virtual QString type() const;
    BINEditor::BinEditor *hexeditor;
    qint32 startadr;

    void filltargetCB();
protected:
    virtual void editorChange(QEditor *e);
    virtual bool paint(QPainter *p, QEditor *e);

    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

public slots:
    void newPocket(CPObject *);
    void removePocket(CPObject *);
    void install();
signals:
    void installTo(CpcXXXX *,qint32 adr, QByteArray data);

private:
    int m_conflictSpot;
    QComboBox *cbinstallTo;
    QLineEdit *leTargetAdr;


};


#endif // QHEXPANEL_H
