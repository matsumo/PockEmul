#include <QDebug>

#include "weblinksparser.h"


bool WebLinksParser::startElement( const QString&, const QString&, const QString &name, const QXmlAttributes &attrs )
{
    if(name == "url" )
    {
        QString desc=attrs.value("desc");
        QString link=attrs.value("link");
        if (parent->getcfgfname()==model)
            parent->insertLinkAction(CPObject::WEBLINK,desc,link);
    }
    else if(name == "document" )
    {
        QString desc=attrs.value("desc");
        QString link=attrs.value("link");
        if (parent->getcfgfname()==model)
            parent->insertLinkAction(CPObject::DOCUMENT,desc,link);
    }
    else if( name == "model" )
        model = attrs.value("value");

    return true;
}
