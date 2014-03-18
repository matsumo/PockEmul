#ifndef CCOMPLETION_H
#define CCOMPLETION_H

#include "qcodecompletionengine.h"

class QCodeCompletionWidget;

class CCompletion : public QCodeCompletionEngine
{
    Q_OBJECT
public:
    explicit CCompletion(QObject *p = 0);

    virtual QCodeCompletionEngine* clone();

    virtual QString language() const ;
    virtual QStringList extensions() const;
    QString getLastToken(const QDocumentCursor &c);
signals:

public slots:

protected:
    virtual void complete(const QDocumentCursor& c, const QString& trigger);

private:
    QCodeCompletionWidget *pPopup;


};

#endif // CCOMPLETION_H
