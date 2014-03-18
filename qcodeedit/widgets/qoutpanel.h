#ifndef QOUTPANEL_H
#define QOUTPANEL_H



#include "qpanel.h"
#include "bineditor/bineditor.h"

class QDocumentLine;
class CPObject;
class QComboBox;
class QTextEdit;

class QCE_EXPORT QOutPanel : public QPanel
{
    Q_OBJECT

public:
    Q_PANEL(QOutPanel, "Out Panel")

    QOutPanel(QWidget *p = 0);
    virtual ~QOutPanel();

    virtual QString type() const;
    QTextEdit *out;

    void filltargetCB();
protected:
    virtual void editorChange(QEditor *e);
    virtual bool paint(QPainter *p, QEditor *e);

    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

public slots:


private:
    int m_conflictSpot;

};


#endif // QOUTPANEL_H
