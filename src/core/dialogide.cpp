#include <QFile>


#include "dialogide.h"
#include "ide/highlighter.h"
#include "lcc/lcpp.h"
#include "lcc/lcc.h"
#include "lcc/pasm.h"
#include "pobject.h"
#include "pcxxxx.h"


#include "qformatscheme.h"
#include "qdocument.h"
#include "qlinemarksinfocenter.h"
#include "qeditor.h"
#include "qcodeedit.h"
#include "qlanguagefactory.h"





extern QList<CPObject *> listpPObject;

DialogIDE::DialogIDE( QWidget * parent, Qt::WFlags f) : QDialog(parent, f)
{



    setupUi(this);

    setupEditor();

    connect(startButton, SIGNAL(clicked()), this, SLOT(start()));
    connect(installPB,SIGNAL(clicked()),this,SLOT(inject()));
    connect(savePB,SIGNAL(clicked()),this,SLOT(save()));
    connect(listWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(load(QListWidgetItem*)));
    connect(tabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTab(int)));

    this->setWindowFlags(Qt::Window);




}

void DialogIDE::setupEditor()
{

    m_formats = new QFormatScheme("qxs/formats.qxf", this);
    QDocument::setDefaultFormatScheme(m_formats);

    QLineMarksInfoCenter::instance()->loadMarkTypes("qxs/marks.qxm");

    m_languages = new QLanguageFactory(m_formats, this);
    m_languages->addDefinitionPath("qxs");


    fill_inject();

    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);


    editor->setFont(font);
    editor->setTabStopWidth(40);

    highlighter = new Highlighter(editor->document());

//    QFile file("mainwindow.h");
//    if (file.open(QFile::ReadOnly | QFile::Text))
//        editor->setPlainText(file.readAll());
    refreshFileList();
}


void DialogIDE::resizeEvent( QResizeEvent * event )
{

}

void DialogIDE::start(void) {

    QString src = editor->toPlainText();

    mapSRC.clear();
    mapPP.clear();
    mapASM.clear();
    mapLM.clear();

    mapSRC["test"] = src.toAscii();

    Clcpp *lcpp = new Clcpp(&mapSRC,&mapPP,this->modelCB->currentText());
    lcpp->run();
    outputpp->setPlainText(mapPP["test"]);

    Clcc *lcc = new Clcc(&mapPP,&mapASM);
 //   connect(lcc,SIGNAL(outputSignal(QString,QString)),this,SLOT(output(QString,QString)));
    lcc->run();

    outputstd->setPlainText(mapASM["output"]);
    outputasm->setPlainText(mapASM["test.asm"]);

    Cpasm * pasm = new Cpasm(&mapASM,&mapLM);
    pasm->parsefile("BAS",mapASM["test.asm"]);
    pasm->savefile("BAS");
    pasm->savefile("BIN");

    outputlm->setPlainText(mapLM["BAS"]);
    outputstd->appendPlainText("\r\r\r"+mapLM["output"]);

}

void DialogIDE::output(QString f,QString s) {
    outputstd->insertPlainText(s);
    //outputstd->append(s);
    outputstd->update();
}

void DialogIDE::fill_inject(void) {
    // update the injectCB ComboBox
    injectCB->clear();
    for (int i = 0; i < listpPObject.size();i++) {
        CPObject *p = listpPObject.at(i);
        p->getName();
        injectCB->addItem(p->getName(),tr("%1").arg((long)p));
    }
}

void DialogIDE::inject(void) {

    int index = injectCB->currentIndex();
    CpcXXXX *pc = (CpcXXXX *) injectCB->itemData(index).toString().toULongLong();
    bool ok;
    int orig = origEdit->text().toInt(&ok,16);

    QDataStream in(mapLM["BIN"]);
    in.readRawData ((char *) &pc->mem[orig],
                    mapLM["BIN"].size() );
    QMessageBox::about(mainwindow,"Transfert",tr("LM stored at %1").arg(orig));

}

void DialogIDE::save(void) {

}

void DialogIDE::refreshFileList(void) {
    QDir dir;
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = dir.entryInfoList(QStringList("*.c"));
    //QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        listWidget->addItem(fileInfo.fileName());
    }
}

void DialogIDE::load(QListWidgetItem* id) {

    // check if file still open
    if (editorMap.contains(id->text())) {
            tabWidget->setCurrentWidget(editorMap.value(id->text()));
    }
    else {
    // If not create a new tab
        CEditorWidget *locEditorWidget = new CEditorWidget();
        tabWidget->insertTab(0,locEditorWidget,id->text());

        m_languages->setLanguage(locEditorWidget->m_editControl->editor(), id->text());
        locEditorWidget->m_editControl->editor()->load(id->text());
        editorMap.insert(id->text(),locEditorWidget);
        tabWidget->setCurrentIndex(0);
    }


}

void DialogIDE::closeTab(int index) {
    CEditorWidget *locEditControl = (CEditorWidget*)tabWidget->widget(index);
        if (locEditControl->m_editControl->editor()->isContentModified()) {
            // save it
            locEditControl->m_editControl->editor()->save();
        }
    tabWidget->removeTab(index);
    editorMap.remove(locEditControl->m_editControl->editor()->fileName());
}

void DialogIDE::closeEvent(QCloseEvent *event)
{
    // check unsaved data
    //event->accept();
}
