#include "overlay.h"

COverlay::COverlay(QString _title, QImage _image, QRect _rect)
{
     Title = _title;
     overlayImage = _image;
     overlayRect = _rect;
}
