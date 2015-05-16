/*!
    \file windowide.cpp
    \brief Implementation of the WindowIDE class
*/

#include <QFile>
#include <QFileDialog>
#include <QProcess>

#include "windowide.h"
#include "ui_windowide.h"

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
#include "qeditconfig.h"
#include "qlanguagefactory.h"
#include "qlanguagedefinition.h"
#include "qhexpanel.h"
#include "qoutpanel.h"
#include "ccompletion.h"

#include "qcodemodel2/qcodenode.h"

extern QList<CPObject *> listpPObject; /*!< TODO */
extern QString workDir;

#define NB_FILENAME (Qt::UserRole+1)
#define NB_PREBUILD (Qt::UserRole+2)
#define NB_EXT (Qt::UserRole+3)
#define NB_OUTPUT (Qt::UserRole+4)
#define NB_BIN (Qt::UserRole+5)
#define NB_MODEL (Qt::UserRole+6)
#define NB_BUILDER (Qt::UserRole+7)

/*!
    \class WindowIDE
    \brief IDE main class

    The WindowIDE class ...
*/

/*!
 \brief IDE main class constructor

 \param parent
*/
WindowIDE::WindowIDE(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WindowIDE)
{
    ui->setupUi(this);

    tabifyDockWidget(ui->filesDockWidget,ui->projectDockWidget);
    tabifyDockWidget(ui->projectDockWidget,ui->configDockWidget);
    ui->filesDockWidget->raise();

//    tabifyDockWidget(ui->configDockWidget,ui->projectDockWidget);
//    tabifyDockWidget(ui->projectDockWidget,ui->filesDockWidget);
    this->setWindowTitle(tr("PockEmul Integrated Development Environment"));
    this->setAttribute(Qt::WA_DeleteOnClose,true);

//    ui->verticalLayout_2->addWidget(new QEditConfig());

    devDir.mkpath(workDir+"/dev/");
    devDir.setPath(workDir+"/dev/");

    setupEditor();

    connect(ui->actionCompile, SIGNAL(triggered()), this, SLOT(compile()));
    connect(ui->actionSave,SIGNAL(triggered()),this,SLOT(save()));
    connect(ui->actionSave_All,SIGNAL(triggered()),this,SLOT(saveAll()));
    //connect(ui->listWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(load(QListWidgetItem*)));
    connect(ui->treeView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(load(QModelIndex)));
    connect(ui->tabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(closeEditorTab(int)));
    connect(ui->outputtabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(closeOutputTab(int)));
    connect(ui->actionNew,SIGNAL(triggered()),this,SLOT(newFile()));

    connect(ui->pbOpenBuider,SIGNAL(clicked()),this,SLOT(OpenNewBuilder()));
    connect(ui->pbNBAdd,SIGNAL(clicked()),this,SLOT(AddNewBuilder()));
    connect(ui->pbNBRemove,SIGNAL(clicked()),this,SLOT(removeBuilder()));
    connect(ui->pbNBUpdate,SIGNAL(clicked()),this,SLOT(updateBuilder()));
    connect(ui->lwBuilders,SIGNAL(itemSelectionChanged()),this,SLOT(SelectBuilder()));

    connect(ui->pbAddMB,SIGNAL(clicked()),this,SLOT(AddModelBuilder()));
    connect(ui->pbRemoveMB,SIGNAL(clicked()),this,SLOT(RemoveModelBuilder()));

    connect(mainwindow,SIGNAL(DestroySignal(CPObject*)),this,SLOT(DestroySlot(CPObject*)));


    loadConfig();

}

/*!
 \brief desctructeur

*/
WindowIDE::~WindowIDE()
{
    saveConfig();

    delete m_formats;
    delete m_languages;
    foreach (CEditorWidget *value, editorMap) {
         delete value;
    }
    delete ui;

}




/*!
 \brief

*/
void WindowIDE::setupEditor()
{

    m_formats = new QFormatScheme(":/QXS/formats.qxf", this);
    QDocument::setDefaultFormatScheme(m_formats);

    QLineMarksInfoCenter::instance()->loadMarkTypes(":QXS/marks.qxm");

    m_languages = new QLanguageFactory(m_formats, this);
    m_languages->addDefinitionPath(":QXS");

    refreshFileList();

    ui->vardockWidget->hide();
    ui->procdockWidget->hide();

}

QList<QCodeNode *> WindowIDE::completionScan(QEditor *e) {

    QList<QCodeNode *> nodes;

#ifndef Q_OS_ANDROID

    QMap<QString,QByteArray> mapSRC;
    QMap<QString,QByteArray> mapPP;
    QMap<QString,QByteArray> mapASM;
    //CEditorWidget *locEditorWidget = ((CEditorWidget*)ui->tabWidget->currentWidget());

    QString source = "#include <internal.h>\r\n"+e->text();
    QString sourcefname=e->fileName();
    QFileInfo fInfo(sourcefname);

    if (e->languageDefinition()->language()=="C++") {
        mapSRC[sourcefname] = source.toLatin1();
        Clcpp *lcpp = new Clcpp(&mapSRC,&mapPP,ui->targetComboBox->currentText(),false);
        lcpp->pStdLibs->LoadLibs();
        lcpp->run();
        this->doxygenlist = lcpp->getDoxygenList();
        //createEditorTab(fInfo.baseName()+".xml",mapPP["DOxygen"]);
        //createOutputTab("PP Compiler :"+fInfo.fileName(),mapPP["output"]);
        Clcc *lcc = new Clcc(&mapPP,&mapASM,false);
        lcc->FirstScan(mapPP[sourcefname]);
        //lcc->run();

        // recuperer var et proc
        //lcc->varlist

        //lcc->proclist
        lcc->printvarlist("variables");
        lcc->printproclist("procedures");

        this->varlist.clear();
        this->proclist.clear();

        this->varlist = lcc->varlist;
        this->proclist = lcc->proclist;




        QCodeNode *n = new QCodeNode();

        for (int i=0;i<varlist.size();i++) {
            Cvar v = varlist.at(i);
            QCodeNode *vn = new QCodeNode();
            vn->roles = "v@"+v.varname+"@"+v.typ;
            vn->attach(n);
        }

        //    fct->roles = QByteArray("f@")
        //                + name
        //                + "@"
        //                + retval
        //                + "@"
        //                + QByteArray::number(visibility)
        //                + "@"
        //                ;

        for (int i=0;i<proclist.size();i++) {
            Cproc p = proclist.at(i);
            QCodeNode *pn = new QCodeNode();
            //name
            pn->roles = "f@"+p.ProcName+"@";
            //return
            if (p.hasreturn) {
                if (p.ReturnIsWord) pn->roles+="word";
                else pn->roles+="byte";
            }
            //visibility + template
            pn->roles+="@@@";
            //arguments
            if (p.ParCnt > 0) pn->roles+=p.Params+"@";

            pn->attach(n);
        }
        nodes.append(n);

        ui->vartextEdit->setText(mapASM["variables"]);
        ui->proctextEdit->setText(mapASM["procedures"]);

        return nodes;
    }
#endif
    return nodes;
}

QStringList WindowIDE::getProc(QString s) {
    QString sr = "";
    for (int i= 0; i< proclist.size();i++) {
        if (proclist.at(i).ProcName == s) {
            if (proclist.at(i).hasreturn) {
                if (proclist.at(i).ReturnIsWord) sr+="word ";
                else sr+="byte ";
            }
            sr += proclist.at(i).ProcName+"( ";

            if (proclist.at(i).ParCnt > 0) sr+=proclist.at(i).Params;
            sr+=")";
            return QStringList() << sr;
        }
    }
    return QStringList();
}

/*!
 \brief lance le processus de compilation adéquate en fonction de l'extension du fichier

*/
void WindowIDE::compileINTERNAL() {
#ifndef Q_OS_ANDROID

    CEditorWidget *locEditorWidget = ((CEditorWidget*)ui->tabWidget->currentWidget());

    QString sourcefname = locEditorWidget->m_editControl->editor()->fileName();
    QString source = "#include <internal.h>\r\n"+locEditorWidget->m_editControl->editor()->text();

    QString _path = QFileInfo(sourcefname).absolutePath();
    QFileInfo fInfo(sourcefname);

    if (locEditorWidget->m_editControl->editor()->languageDefinition()->language()=="C++") {
        mapSRC[sourcefname] = source.toLatin1();
        Clcpp *lcpp = new Clcpp(&mapSRC,&mapPP,ui->targetComboBox->currentText());
        lcpp->pStdLibs->LoadLibs();
        lcpp->run();
        //createEditorTab(fInfo.baseName()+".pp",mapPP[sourcefname]);
        //createOutputTab("PP Compiler :"+fInfo.fileName(),mapPP["output"]);
#if 1
        Clcc *lcc = new Clcc(&mapPP,&mapASM);
        lcc->run();
        createEditorTab(_path+"/"+fInfo.baseName()+".log",mapASM["LOG"]);

        createEditorTab(_path+"/"+fInfo.baseName()+".asm",mapASM[fInfo.baseName()+".asm"]);

        createOutputTab("C Compiler :"+fInfo.fileName(),mapASM["output"]);
#endif
    }

    if (locEditorWidget->m_editControl->editor()->languageDefinition()->language()=="ASM") {
        CEditorWidget *currentWidget = locEditorWidget;

        mapSRC[sourcefname] = source.toLatin1();

            Cpasm * pasm = new Cpasm(&mapSRC,&mapLM);
            pasm->run("BAS",mapSRC[sourcefname]);
            pasm->savefile("BAS");
            pasm->savefile("BIN");
            pasm->savefile("HEX");

            createEditorTab(_path+"/"+fInfo.baseName()+".bas",mapLM["BAS"]);

            createOutputTab("ASM Compiler :"+fInfo.fileName(),mapLM["output"]);


        if (! mapLM["BIN"].isEmpty()) {
            currentWidget = ((CEditorWidget*)ui->tabWidget->currentWidget());
            QHexPanel *hexpanel = new QHexPanel();


            currentWidget->m_editControl
                    ->addPanel(hexpanel, QCodeEdit::South, true);
            hexpanel->startadr = mapLM["_ORG"].trimmed().toLong();
            hexpanel->hexeditor->setData(mapLM["BIN"],hexpanel->startadr);
            hexpanel->hexeditor->setCursorPosition(0,BINEditor::BinEditor::MoveAnchor);
            connect(this,SIGNAL(newEmulatedPocket(CPObject*)),hexpanel,SLOT(newPocket(CPObject*)));
            connect(this,SIGNAL(removeEmulatedPocket(CPObject*)),hexpanel,SLOT(removePocket(CPObject*)));
            connect(hexpanel,SIGNAL(installTo(CpcXXXX*,qint32,QByteArray)),this,SLOT(installTo(CpcXXXX*,qint32,QByteArray)));

        }

//        MSG_ERROR("*"+mapLM["_ORG"]+"*");
//        MSG_ERROR(QString("%1").arg(hexpanel->startadr));
    }
#endif
}

void WindowIDE::compile() {
    mapSRC.clear();
    mapPP.clear();
    mapASM.clear();
    mapLM.clear();

    save();

    CEditorWidget *currentWidget = ((CEditorWidget*)ui->tabWidget->currentWidget());
    QString fn = currentWidget->m_editControl->editor()->fileName();
    qWarning()<<"src fileName:"<<fn;
    QString _path = QFileInfo(fn).absolutePath();
    QString _filename = QFileInfo(fn).fileName();
    QString _ext = QFileInfo(fn).suffix();
    QString _basename = QFileInfo(fn).baseName();
    QString _model=ui->targetComboBox->currentText();

    QString prebuild = getBuilder(_model,_ext,NB_PREBUILD);
    // replace $2 by basename and $1 by filename
    prebuild.replace("$1",_filename);
    prebuild.replace("$2",_basename);

    if (!prebuild.isEmpty()) {
        QProcess *myProcess = new QProcess();
        myProcess->setWorkingDirectory(_path);

        myProcess->setReadChannel(QProcess::StandardOutput);
        myProcess->start(prebuild);

        if (!myProcess->waitForStarted()){
            qWarning()<<"ERROR PREBUILD START";
            return;
        }
        if (!myProcess->waitForFinished()) {
            qWarning()<<"ERROR PREBUILD FINISH";
            return;
        }
    }


    QString program = getBuilder(_model,_ext,NB_FILENAME);
    QString binFileName = getBuilder(_model,_ext,NB_BIN).replace("$1",_filename).replace("$2",_basename);
    QString outputFileName = getBuilder(_model,_ext,NB_OUTPUT).replace("$1",_filename).replace("$2",_basename);

    // replace $2 by basename and $1 by filename
    program.replace("$1",_filename);
    program.replace("$2",_basename);

//    QString program = QCoreApplication::applicationDirPath()+"/lhasm.exe";
    qWarning()<<"exe:"<<program;
    qWarning()<<"Working dir:"<<_path;

    if ( (program == "LCC") || (program == "PASM")) {
        compileINTERNAL();
        return;
    }

    QProcess *myProcess = new QProcess();
    myProcess->setWorkingDirectory(_path);

    myProcess->setReadChannel(QProcess::StandardOutput);
    myProcess->start(program);

    if (!myProcess->waitForStarted()){
        qWarning()<<"ERROR START";
        return;
    }
    if (!myProcess->waitForFinished()) {
        qWarning()<<"ERROR FINISH";
        return;
    }
    qWarning()<<"EXEC:"<<myProcess->exitStatus();
    QByteArray stdOutput = myProcess->readAll();
    qWarning()<<"EXEC result:"<< stdOutput;

    QString binFn = _path+"/"+binFileName;
    qWarning()<<binFn;
    QFile _f(binFn);

    if (_f.open(QIODevice::ReadOnly)) {
        mapLM["BIN"]=_f.readAll();
        _f.close();
    }

    QString logFn = _path+"/"+outputFileName;
    qWarning()<<logFn;
    QFile _f_log(logFn);
    if (_f_log.open(QIODevice::ReadOnly)) {
        createOutputTab("lhASM output",_f_log.readAll().append("\n").append(stdOutput));
//        QByteArray _logResult = _f_log.readAll();
        _f_log.close();
    }

    if (! mapLM["BIN"].isEmpty()) {

        QHexPanel *hexpanel = new QHexPanel();


        currentWidget->m_editControl->addPanel(hexpanel, QCodeEdit::South, true);
        hexpanel->startadr = mapLM["_ORG"].trimmed().toLong();
        hexpanel->hexeditor->setData(mapLM["BIN"],hexpanel->startadr);
        hexpanel->hexeditor->setCursorPosition(0,BINEditor::BinEditor::MoveAnchor);
        connect(this,SIGNAL(newEmulatedPocket(CPObject*)),hexpanel,SLOT(newPocket(CPObject*)));
        connect(this,SIGNAL(removeEmulatedPocket(CPObject*)),hexpanel,SLOT(removePocket(CPObject*)));
        connect(hexpanel,SIGNAL(installTo(CpcXXXX*,qint32,QByteArray)),this,SLOT(installTo(CpcXXXX*,qint32,QByteArray)));

        update();
    }

}

/*!
 \brief Add a new tab to the Editor panel

 \param fname   Filename of the new document
 \param text    document content
 \param load    if true then load the text from disk
*/
CEditorWidget * WindowIDE::createEditorTab(QString fname, QString text,bool load) {

    CEditorWidget *locEditorWidget = new CEditorWidget(this);
    ui->tabWidget->insertTab(0,locEditorWidget,QFileInfo(fname).fileName());
    ui->tabWidget->setTabToolTip(0,QFileInfo(fname).absoluteFilePath());
    m_languages->setLanguage(locEditorWidget->m_editControl->editor(), fname);

    e = new CCompletion();
    e->triggerAction()->setShortcut(tr("Ctrl+Space"));

    //connect(locEditorWidget->m_editControl->editor(),SIGNAL(cursorPositionChanged()),e,SLOT(complete()));

    //e->setEditor(locEditorWidget->m_editControl->editor());
    m_languages->addCompletionEngine(e);
    locEditorWidget->m_editControl->editor()->setCompletionEngine(e->clone());

    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(14);
    locEditorWidget->m_editControl->editor()->setFont(font);

    locEditorWidget->m_editControl->editor()->setText(text);
    locEditorWidget->m_editControl->editor()->setFileName(fname);
    if (load) locEditorWidget->m_editControl->editor()->load(fname);
    editorMap.insert(fname,locEditorWidget);
    ui->tabWidget->setCurrentIndex(0);

    return locEditorWidget;
}

/*!
 \brief Add a new tab to the Output Panel

 \param fname   The tab name
 \param text    Text displayed into the new tab
*/
void WindowIDE::createOutputTab(QString fname, QString text) {
    QTextEdit *locTextEdit = new QTextEdit();
    ui->outputtabWidget->insertTab(0,locTextEdit,fname);
    locTextEdit->setText(text);
    ui->outputtabWidget->setCurrentIndex(0);
}


/*!
 \brief

 \param pc
*/
void WindowIDE::addtargetCB(CPObject *pc) {
    emit newEmulatedPocket(pc);
}

void WindowIDE::removetargetCB(CPObject *pc) {
    emit removeEmulatedPocket(pc);
}

/*!
 \brief Charge le binaire data dans la memoire du Pocket emule.

 \param pc      Le pocket cible.
 \param adr     Adresse de chargement des donnees.
 \param data    Donnees binaires a charger.
*/
void WindowIDE::installTo(CpcXXXX * pc,qint32 adr, QByteArray data ) {
qint32 targetAdr = adr;
//    if (!(ui->adrLineEdit->text().isEmpty())) {
//        targetAdr = ui->adrLineEdit->text().toLong(0,16);
//    }
    if (pc->Mem_Load(targetAdr,data)) {
        QMessageBox::about(mainwindow,"Transfert",tr("LM stored at %1").arg(targetAdr));
    }
}

void WindowIDE::DestroySlot(CPObject *pObject)
{
    removetargetCB(pObject);
}

void WindowIDE::OpenNewBuilder()
{
    // open openfilebox
    QString fn = QFileDialog::getOpenFileName(
            this,
            tr("Choose the builder file"),
            devDir.path());
    if (fn.isEmpty()) return;

    ui->leNewBuilderFileName->setText(fn);
}


void WindowIDE::AddNewBuilder()
{
    QListWidgetItem *_item = new QListWidgetItem(ui->leNewBuilderTitle->text());
    _item->setToolTip(ui->leNewBuilderFileName->text());
    _item->setData(NB_FILENAME,ui->leNewBuilderFileName->text());
    _item->setData(NB_PREBUILD,ui->leNewBuilderPreBuild->text());
    _item->setData(NB_EXT,ui->leNewBuilderExt->text());
    _item->setData(NB_OUTPUT,ui->leNewBuilderOutput->text());
    _item->setData(NB_BIN,ui->leNewBuilderBinFiles->text());
    _item->setCheckState(Qt::Checked);

    ui->lwBuilders->addItem(_item);
}

void WindowIDE::updateBuilder()
{
    QListWidgetItem *_item = ui->lwBuilders->currentItem();
    if (_item ==0) return;
    if (_item->toolTip()=="INTERNAL") return;

    _item->setText(ui->leNewBuilderTitle->text());
    _item->setToolTip(ui->leNewBuilderFileName->text());
    _item->setData(NB_FILENAME,ui->leNewBuilderFileName->text());
    _item->setData(NB_PREBUILD,ui->leNewBuilderPreBuild->text());
    _item->setData(NB_EXT,ui->leNewBuilderExt->text());
    _item->setData(NB_OUTPUT,ui->leNewBuilderOutput->text());
    _item->setData(NB_BIN,ui->leNewBuilderBinFiles->text());
}


void WindowIDE::AddModelBuilder()
{
    QListWidgetItem *_builderItem = ui->lwBuilders->currentItem();
    if (_builderItem == 0) return; // msgbox select builder first

    QString _model = ui->cbMB->currentText();
    QListWidgetItem *_item = new QListWidgetItem();
    _item->setText(_model+" -> "+_builderItem->text());
    _item->setData(NB_MODEL,_model);
    _item->setData(NB_BUILDER,_builderItem->text());
    _item->setCheckState(Qt::Checked);
    ui->lwModelBuilder->addItem(_item);

}

void WindowIDE::RemoveModelBuilder()
{
    if (ui->lwModelBuilder->currentRow()>=0)
        ui->lwModelBuilder->takeItem(ui->lwModelBuilder->currentRow());
}

void WindowIDE::SelectBuilder()
{
    QListWidgetItem *_item = ui->lwBuilders->currentItem();

    ui->leNewBuilderTitle->setText(_item->text());
    ui->leNewBuilderFileName->setText(_item->data(NB_FILENAME).toString());
    ui->leNewBuilderPreBuild->setText(_item->data(NB_PREBUILD).toString());
    ui->leNewBuilderExt->setText(_item->data(NB_EXT).toString());
    ui->leNewBuilderOutput->setText(_item->data(NB_OUTPUT).toString());
    ui->leNewBuilderBinFiles->setText(_item->data(NB_BIN).toString());

//    ui->lwBuilders->openPersistentEditor(_item);

}

void WindowIDE::removeBuilder()
{
    if (ui->lwBuilders->currentRow()>=0)
        if (ui->lwBuilders->currentItem()->toolTip()!= "INTERNAL")      // Add a msg
            ui->lwBuilders->takeItem(ui->lwBuilders->currentRow());
}
/*!
 \brief Sauvegarde le fichier sur disque.

*/
void WindowIDE::save(void) {
    CEditorWidget *locEditorWidget = (CEditorWidget*)ui->tabWidget->currentWidget();
    if (locEditorWidget->m_editControl->editor()->isContentModified()) {
        // save it
        locEditorWidget->m_editControl->editor()->save();
    }
}

/*!
 \brief Sauvegarde tous les fichiers ouverts sur disque.

 \fn WindowIDE::saveAll
*/
void WindowIDE::saveAll(void) {
    for (int i=0; i < ui->tabWidget->count();i++) {
        CEditorWidget *locEditorWidget = (CEditorWidget*)ui->tabWidget->widget(i);
        if (locEditorWidget->m_editControl->editor()->isContentModified()) {
            // save it
            locEditorWidget->m_editControl->editor()->save();
        }
    }
}
/*!
 \brief

*/
void WindowIDE::refreshFileList(void) {
//    QDir dir;
//    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
//    dir.setSorting(QDir::Size | QDir::Reversed);

//    QFileInfoList list = dir.entryInfoList(QStringList() << "*.c" << "*.asm");
//    //QFileInfoList list = dir.entryInfoList();
//    for (int i = 0; i < list.size(); ++i) {
//        QFileInfo fileInfo = list.at(i);
//        ui->listWidget->addItem(fileInfo.fileName());
//    }
    model = new QFileSystemModel;
    model->setRootPath(devDir.path());
    model->setNameFilters(QStringList() << "*.c" << "*.asm" << "*.h"<<"*.sym"<<"*.log"<<"*.bin"<<"*.inc");
    model->setNameFilterDisables(false);
    ui->treeView->setModel(model);
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);
    ui->treeView->header()->hide();
    ui->treeView->setRootIndex(model->index(devDir.path()));
    qWarning()<<"devDir:"<<devDir.path();
}

/*!
 \brief

 \param id
*/
void WindowIDE::load(QModelIndex index) {
    //QMessageBox::about(this,"test",model->fileName(index)+" - "+model->filePath(index));
    // check if file still open
    if (editorMap.contains(model->filePath(index))) {
            ui->tabWidget->setCurrentWidget(editorMap.value(model->filePath(index)));
    }
    else {
    // If not create a new tab
        createEditorTab(model->filePath(index),"",true);

    }

}

/*!
 \brief

 \param index
*/
void WindowIDE::closeEditorTab(int index) {
    CEditorWidget *locEditorWidget = (CEditorWidget*)ui->tabWidget->widget(index);
    if (locEditorWidget->m_editControl->editor()->isContentModified()) {
        // save it
        locEditorWidget->m_editControl->editor()->save();
    }
    ui->tabWidget->removeTab(index);
    editorMap.remove(locEditorWidget->m_editControl->editor()->fileName());
}

/*!
 \brief

 \fn WindowIDE::closeOutputTab
 \param index
*/
void WindowIDE::closeOutputTab(int index) {
    ui->outputtabWidget->removeTab(index);
}
/*!
 \brief

 \param event
*/
void WindowIDE::closeEvent(QCloseEvent *event)
{
    //Check unsaved data
    mainwindow->windowide = 0;
    event->accept();

}

/*!
 \brief

*/
void WindowIDE::newFile()
{
    QString fileName = QFileDialog::getSaveFileName(
                        this,
                        tr("Choose a file"),
                        devDir.path(),
                        tr("C source (*.c *.h);;ASM source (*.asm)"),
                        new QString("(*.c)"));

    createEditorTab(fileName,"");

}

void WindowIDE::targetChange(QString m)
{
    Q_UNUSED(m)
}

Cproc WindowIDE::getProcObj(QString s)
{
    for (int i= 0; i< proclist.size();i++) {
        if (proclist.at(i).ProcName == s) {
            return proclist.at(i);
        }
    }
    return Cproc();
}

CDOxyItem * WindowIDE::getDOxygenInfo(QString s)
{
    for (int i=0;i< doxygenlist.size();i++) {
        if (doxygenlist.at(i)->fn.trimmed()==s) return doxygenlist.at(i);
    }
    return 0;
}

QString WindowIDE::getBuilder(QString model,QString ext,int role) {

    // fetch lwModelBuilder
    for (int i=0; i < ui->lwModelBuilder->count(); i++) {
        QString _model = ui->lwModelBuilder->item(i)->data(NB_MODEL).toString();
        QString _builder = ui->lwModelBuilder->item(i)->data(NB_BUILDER).toString();
        Qt::CheckState _checked = ui->lwModelBuilder->item(i)->checkState();

        if ( (_checked==Qt::Checked) && (_model == model) ){
            // check builder ext
            for (int i=0; i < ui->lwBuilders->count(); i++) {
                QString _title = ui->lwBuilders->item(i)->text();
                QString _fileName = ui->lwBuilders->item(i)->data(NB_FILENAME).toString();
                QString _prebuild = ui->lwBuilders->item(i)->data(NB_PREBUILD).toString();
                QString _extensions = ui->lwBuilders->item(i)->data(NB_EXT).toString();
                QString _output = ui->lwBuilders->item(i)->data(NB_OUTPUT).toString();
                QString _binFile = ui->lwBuilders->item(i)->data(NB_BIN).toString();
                Qt::CheckState _checked = ui->lwBuilders->item(i)->checkState();

                if ( (_builder==_title) &&
                     (_checked==Qt::Checked) &&
                     (_extensions.split(';').contains(ext))) {
                    // Found
                    switch (role) {
                    case 0: return _title;
                    case NB_FILENAME: return _fileName;
                    case NB_PREBUILD: return _prebuild;
                    case NB_EXT: return _extensions;
                    case NB_OUTPUT: return _output;
                    case NB_BIN: return _binFile;
                    }
                }
            }
        }
    }

    return QString();
}

void WindowIDE::saveConfig() {

    QSettings settings(workDir+"config.ini",QSettings::IniFormat);

    QString s;
    QXmlStreamWriter *xmlOut = new QXmlStreamWriter(&s);
    xmlOut->setAutoFormatting(true);

    xmlOut->writeStartElement("IDE");
        xmlOut->writeAttribute("version", "1.0");

        xmlOut->writeStartElement("builders");
            for (int i=0; i < ui->lwBuilders->count(); i++) {
                if (ui->lwBuilders->item(i)->toolTip()!="INTERNAL") {
                    QString _title = ui->lwBuilders->item(i)->text();
                    QString _fileName = ui->lwBuilders->item(i)->data(NB_FILENAME).toString();
                    QString _prebuild = ui->lwBuilders->item(i)->data(NB_PREBUILD).toString();
                    QString _extensions = ui->lwBuilders->item(i)->data(NB_EXT).toString();
                    QString _output = ui->lwBuilders->item(i)->data(NB_OUTPUT).toString();
                    QString _binFile = ui->lwBuilders->item(i)->data(NB_BIN).toString();
                    QString _checked = (ui->lwBuilders->item(i)->checkState()==Qt::Checked ? "true":"false");
                    xmlOut->writeStartElement("builder");
                    xmlOut->writeAttribute("title", _title);
                    xmlOut->writeAttribute("filename", _fileName);
                    xmlOut->writeAttribute("prebuild", _prebuild);
                    xmlOut->writeAttribute("ext", _extensions);
                    xmlOut->writeAttribute("output", _output);
                    xmlOut->writeAttribute("binfile", _binFile);
                    xmlOut->writeAttribute("checked", _checked);
                    xmlOut->writeEndElement();
                }
            }
        xmlOut->writeEndElement();  // builders

        xmlOut->writeStartElement("modelbuilders");
            for (int i=0; i < ui->lwModelBuilder->count(); i++) {
                QString _title = ui->lwModelBuilder->item(i)->text();
                QString _model = ui->lwModelBuilder->item(i)->data(NB_MODEL).toString();
                QString _builder = ui->lwModelBuilder->item(i)->data(NB_BUILDER).toString();
                QString _checked = (ui->lwModelBuilder->item(i)->checkState()==Qt::Checked ? "true":"false");
                xmlOut->writeStartElement("modelbuilder");
                xmlOut->writeAttribute("title", _title);
                xmlOut->writeAttribute("model", _model);
                xmlOut->writeAttribute("builder", _builder);
                xmlOut->writeAttribute("checked", _checked);
                xmlOut->writeEndElement();
            }
        xmlOut->writeEndElement();  // modelbuilders

    xmlOut->writeEndElement();  // dasm

    settings.setValue("IDE", s);
}

void WindowIDE::loadConfig() {
    {
        QListWidgetItem *_item = new QListWidgetItem("LCC C Compiler for SC61860");
        _item->setToolTip("INTERNAL");
        _item->setData(NB_FILENAME,"LCC");
        _item->setData(NB_EXT,"c");
        //    _item->setData(NB_OUTPUT,_output);
        //    _item->setData(NB_BIN,_binFile);
        _item->setCheckState(Qt::Checked);
        ui->lwBuilders->addItem(_item);
    }
    {
        QListWidgetItem *_item = new QListWidgetItem("PASM Assembler for SC61860");
        _item->setToolTip("INTERNAL");
        _item->setData(NB_FILENAME,"PASM");
        _item->setData(NB_EXT,"asm");
        //    _item->setData(NB_OUTPUT,_output);
        //    _item->setData(NB_BIN,_binFile);
        _item->setCheckState(Qt::Checked);
        ui->lwBuilders->addItem(_item);
    }

    QSettings settings(workDir+"config.ini",QSettings::IniFormat);
    QString xmlData = settings.value("IDE").toString();
    if (xmlData.isEmpty()) return;

    QXmlStreamReader *xml = new QXmlStreamReader(xmlData);

    if (xml->readNextStartElement() && (xml->name() == "IDE")) {

        if (xml->readNextStartElement() &&
                (xml->name() == "builders")) {
            while (xml->readNextStartElement()) {
                QString eltname = xml->name().toString();
                //                            AddLog(LOG_TEMP,eltname);
                if (eltname == "builder") {
                    QString _title = xml->attributes().value("title").toString();
                    QString _fileName = xml->attributes().value("filename").toString();
                    QString _prebuild = xml->attributes().value("prebuild").toString();
                    QString _extensions = xml->attributes().value("ext").toString();
                    QString _output = xml->attributes().value("output").toString();
                    QString _binFile = xml->attributes().value("binfile").toString();
                    Qt::CheckState _checkstate = xml->attributes().value("checked").toString()=="true"?Qt::Checked : Qt::Unchecked;
                    QListWidgetItem *_item = new QListWidgetItem(_title);
                    _item->setToolTip(_fileName);
                    _item->setData(NB_FILENAME,_fileName);
                    _item->setData(NB_PREBUILD,_prebuild);
                    _item->setData(NB_EXT,_extensions);
                    _item->setData(NB_OUTPUT,_output);
                    _item->setData(NB_BIN,_binFile);
                    _item->setCheckState(_checkstate);
                    ui->lwBuilders->addItem(_item);
                }
                xml->skipCurrentElement();
            }
        }

        if (xml->readNextStartElement() &&
                (xml->name() == "modelbuilders")) {
            while (xml->readNextStartElement()) {
                QString eltname = xml->name().toString();
                //                            AddLog(LOG_TEMP,eltname);
                if (eltname == "modelbuilder") {
                    QString _title = xml->attributes().value("title").toString();
                    QString _model = xml->attributes().value("model").toString();
                    QString _builder = xml->attributes().value("builder").toString();
                    Qt::CheckState _checkstate = xml->attributes().value("checked").toString()=="true"?Qt::Checked : Qt::Unchecked;
                    QListWidgetItem *_item = new QListWidgetItem(_title);
//                    _item->setToolTip(_fileName);
                    _item->setData(NB_MODEL,_model);
                    _item->setData(NB_BUILDER,_builder);
                    _item->setCheckState(_checkstate);
                    ui->lwModelBuilder->addItem(_item);
                }
                xml->skipCurrentElement();
            }
        }

    }
}

