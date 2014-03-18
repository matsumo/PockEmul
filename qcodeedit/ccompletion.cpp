#include "ccompletion.h"
#include "qcalltip.h"
#include "qeditor.h"
#include "qdocumentline.h"
#include "qcodecompletionwidget.h"
#include <QTime>
#include <QAction>

//#include "mainwindowpockemul.h"
#include "ui/windowide.h"
#include "qcodemodel2/qcodenode.h"


CCompletion::CCompletion(QObject *p)
: QCodeCompletionEngine(p),pPopup(0)
{
    addTrigger("(");

    QCodeCompletionEngine::triggerAction()->setShortcut(tr("Ctrl+Space"));
}

QString CCompletion::language() const
{
    return "C++";
}

QStringList CCompletion::extensions() const
{
    QStringList l;

    l	<< "c"
        << "h";

    return l;
}

QCodeCompletionEngine * CCompletion::clone()
{
    CCompletion *e = new CCompletion();

    foreach ( QString t, triggers() )
        e->addTrigger(t);

    emit cloned(e);

    return e;
}

/*!
 \brief

 \fn CCompletion::getLastToken
 \param c
 \return QString
*/
QString CCompletion::getLastToken(const QDocumentCursor &c) {

    QString line = c.line().text();
    QString Token = c.selectedText();
    if (Token.isEmpty()) {
        if (line.size()>1) {
            int i = c.columnNumber()-1;
            while (QString("_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ").contains(line.at(i).toUpper())) {
                Token = line.at(i)+Token;
                i--;
                if (i<0) break;
            }
        }
    }
    return Token;
}

/*!
 \brief

 \fn CCompletion::complete
 \param c
 \param trigger
*/
void CCompletion::complete(const QDocumentCursor &c, const QString &trigger)
{
    // test if there is selected text
    // etendre le texte selectionner au mot complet
    // si il a pour next char une ( alors afficher un calltips



    if ( (trigger == "(" ) || !c.selectedText().isEmpty())
    {
        QStringList tips;

        //qDebug("fn %s", fn.constData());
        QList<QCodeNode*> nodes = mainwindow->windowide->completionScan(editor());

        tips = mainwindow->windowide->getProc(getLastToken(c));

        if ( tips.count() )
        {
            CDOxyItem * di = mainwindow->windowide->getDOxygenInfo(getLastToken(c).trimmed());
            if (di) {

                tips[0] += QString("\n")+di->brief;
                if (di->params.count()) {
                    for (int j=0;j<di->params.size();j++) {
                        tips[0] += QString("\n")+di->params.at(j);
                    }
                }
                if (!(di->returnTyp.isEmpty())) tips[0]+=QString("\nReturn value : ")+di->returnTyp;
                if (tips[0].right(1)=="\n") tips[0].chop(1);
            }
            QRect r = editor()->cursorRect();
            QDocumentCursor cursor = editor()->cursor();
            QDocumentLine line = cursor.line();

            int hx = editor()->horizontalOffset(),
                cx = line.cursorToX(cursor.columnNumber());

            QCallTip *ct = new QCallTip(editor()->viewport());
            ct->move(cx - hx, r.y() + r.height());
            ct->setTips(tips);
            ct->show();
            ct->setFocus();

            #ifdef TRACE_COMPLETION
            qDebug("parsing + scoping + search + pre-display : elapsed %i ms", time.elapsed());
            #endif
        }
    }
    else {
    if ( pPopup && pPopup->editor() != editor() )
    {
        delete pPopup;
        pPopup = 0;
    }

    if ( !pPopup )
    {
        pPopup = new QCodeCompletionWidget(editor());
    }

    pPopup->clear();
    pPopup->setCursor(editor()->cursor());

    QTime time;
    time.start();

    QList<QCodeNode*> nodes = mainwindow->windowide->completionScan(editor());


    pPopup->setPrefix(getLastToken(c));


    pPopup->setCompletions(nodes);

    pPopup->update();
    pPopup->popup();
}
#if 1

#endif

}
