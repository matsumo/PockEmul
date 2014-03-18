#ifndef WEBLINKSPARSER_H
#define WEBLINKSPARSER_H

#include <QXmlDefaultHandler>
#include "pobject.h"

class WebLinksParser : public QXmlDefaultHandler
{
public:
  bool startElement( const QString&, const QString&, const QString &name, const QXmlAttributes &attrs );

    WebLinksParser(CPObject *parent = 0){
        this->parent = parent;
        model="";
    }

public:
    CPObject *parent;
    QString  model;
};


#endif // WEBLINKSPARSER_H
