#ifndef MATERIALFLOW_H
#define MATERIALFLOW_H

#include <QStringList>
#include "pictureflow.h"

class MaterialFlow: public PictureFlow
{
public:
  MaterialFlow(QWidget* parent= 0);

public:
    QStringList findFiles(const QString& path = QString());
    void keyPressEvent(QKeyEvent* event);
};

#endif // MATERIALFLOW_H
