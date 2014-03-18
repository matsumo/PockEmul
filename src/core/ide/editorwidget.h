#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QWidget>

#include "qcodeedit.h"

class CEditorWidget : public QWidget
{
    Q_OBJECT
public:

    QCodeEdit *m_editControl;

    explicit CEditorWidget(QWidget *parent = 0);
    ~CEditorWidget();

private slots:
        void resizeEvent(QResizeEvent *);

signals:

public slots:

};

#endif // EDITORWIDGET_H
