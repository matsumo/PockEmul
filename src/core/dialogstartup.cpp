#include <QtGui>

#include "dialogstartup.h"
#include "init.h"
#include "mainwindowpockemul.h"

extern MainWindowPockemul *mainwindow;

// TODO: populate tabs by parsin config.xml file

DialogStartup::DialogStartup( QWidget * parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	setupUi(this);
    populateListWidget(listWidget,P_RES(":/pockemul/config.xml"));
    populateListWidget(listWidget_2,P_RES(":/pockemul/configExt.xml"));
}

bool DialogStartup::populateListWidget(QListWidget* lw,QString configPath) {
    QFile xmlFile(configPath);

    if (!xmlFile.exists() || (xmlFile.error() != QFile::NoError)) {
        qDebug() << "ERROR: Unable to open config file " << configPath;
        return false;
    }

    xmlFile.open(QIODevice::ReadOnly);
    QXmlStreamReader reader(&xmlFile);
    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement()) {
            if (reader.name() == "demos")
                while (!reader.atEnd()) {
                    reader.readNext();
                    if (reader.isStartElement() && reader.name() == "example") {
                        QXmlStreamAttributes attrs = reader.attributes();
                        QStringRef filename = attrs.value("filename");
                        if (!filename.isEmpty()) {
                            QStringRef name = attrs.value("name");
                            QStringRef image = attrs.value("image");
                            QStringRef args = attrs.value("args");
                            QStringRef idpocket = attrs.value("idpocket");
                            QStringRef desc = attrs.value("desc");
                            QStringRef _connectortype = attrs.value("connectortype");
                            QStringRef _conngender = attrs.value("conngender");

                            QListWidgetItem *newItem = new QListWidgetItem;
                            newItem->setText(name.toString());
                            newItem->setIcon(QIcon(image.toString()));
                            newItem->setData(Qt::UserRole,idpocket.toString());

                            if (!idpocket.startsWith("#"))
                                lw->addItem( newItem);

                            // filter on connectors type and gender

//                            if (!connType.isEmpty() && (_connectortype.indexOf(connType)==-1)) continue;
           //                 qWarning()<<connType<<" found:"<<_connectortype;
//                            if (!connGender.isEmpty() && (_conngender.indexOf(connGender)==-1)) continue;
           //                 qWarning()<<"included";


                        }
                    } else if(reader.isEndElement() && reader.name() == "demos") {
                        return true;
                    }
                }
        }
    }

    if (reader.hasError()) {
       qDebug() << QString("Error parsing %1 on line %2 column %3: \n%4")
                .arg(configPath)
                .arg(reader.lineNumber())
                .arg(reader.columnNumber())
                .arg(reader.errorString());
    }

    return true;
}

//
void DialogStartup::on_listWidget_itemDoubleClicked()
{
    QString ItemText = listWidget->currentItem()->data(Qt::UserRole).toString();
    launch(ItemText);
}

void DialogStartup::on_listWidget_2_itemDoubleClicked()
{
    QString ItemText = listWidget_2->currentItem()->data(Qt::UserRole).toString();
    launch(ItemText);
}

void DialogStartup::launch(QString ItemText)
{
    int result = 0;

    if (mainwindow->objtable.contains(ItemText))
        result = mainwindow->objtable.value(ItemText);

    if (result != 0)	{
		this->done(result);
	}

}

