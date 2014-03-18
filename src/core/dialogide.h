#ifndef DIALOGIDE_H
#define DIALOGIDE_H

//

#include "ui_dialogide.h"

#include "ide/highlighter.h"
#include "lcc/lcpp.h"
#include "lcc/lcc.h"

#include "qcodeedit.h"
#include "ide/editorwidget.h"

class QFormatScheme;
class QLanguageFactory;
//
class DialogIDE : public QDialog, public Ui::DialogIDE
{
Q_OBJECT
public:
    DialogIDE( QWidget * parent = 0, Qt::WFlags f = 0 );

    void setupEditor(void);
    void fill_inject(void);
    void refreshFileList(void);

    Highlighter *highlighter;
//    Clcpp *lcpp;
//    Clcc *lcc;

    QMap<QString,QByteArray> mapSRC;
    QMap<QString,QByteArray> mapPP;
    QMap<QString,QByteArray> mapASM;
    QMap<QString,QByteArray> mapLM;

    QMap<QString,CEditorWidget *> editorMap;

private slots:
    void start(void);
    void output(QString,QString);
    void inject(void);
    void save(void);
    void load(QListWidgetItem* id);
    void closeTab(int);

protected:
    void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *);

    QFormatScheme *m_formats;
    QLanguageFactory *m_languages;


};


#endif // DIALOGIDE_H
