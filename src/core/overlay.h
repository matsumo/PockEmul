#ifndef OVERLAY_H
#define OVERLAY_H

#include <QtGui>

class COverlay{
public:
    const char*	GetClassName(){ return("COverlay"); }

    QString Title;
    QImage overlayImage;
    QRect overlayRect;

    COverlay(QString _title, QImage _image, QRect _rect);               //[constructor]

};



#endif // OVERLAY_H
