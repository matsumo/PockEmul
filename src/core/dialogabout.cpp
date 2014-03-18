#include "dialogabout.h"

DialogAbout::DialogAbout(QWidget * parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	setupUi(this);
	
    textBrowser_2->setSource(QUrl("qrc:/pockemul/release_notes.html"));
}

