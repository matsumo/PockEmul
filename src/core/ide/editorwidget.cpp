#include <QAction>

#include "editorwidget.h"
#include "qcodeedit.h"
#include "qeditor.h"

CEditorWidget::CEditorWidget(QWidget *parent) :
    QWidget(parent)
{
    m_editControl = new QCodeEdit(this);
    m_editControl
        ->addPanel("Line Mark Panel", QCodeEdit::West, true)
        ->setShortcut(QKeySequence("F6"));

    m_editControl
        ->addPanel("Line Number Panel", QCodeEdit::West, true)
        ->setShortcut(QKeySequence("F11"));

    m_editControl
        ->addPanel("Fold Panel", QCodeEdit::West, true)
        ->setShortcut(QKeySequence("F9"));

    m_editControl
        ->addPanel("Line Change Panel", QCodeEdit::West, true);

    m_editControl
        ->addPanel("Status Panel", QCodeEdit::South, true);

    m_editControl
        ->addPanel("Search Replace Panel", QCodeEdit::South);

}

CEditorWidget::~CEditorWidget() {
    delete m_editControl;
}

void CEditorWidget::resizeEvent(QResizeEvent * event) {
    m_editControl->editor()->resize(this->size());
}
