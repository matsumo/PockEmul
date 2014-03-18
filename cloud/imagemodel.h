#include <enginiomodel.h>
#include "imageobject.h"

#ifndef IMAGEMODEL_H
#define IMAGEMODEL_H
class ImageModel : public EnginioModel
{
    Q_OBJECT
public:
    ImageModel(QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role) const;

    enum Roles {
        Id = Qt::UserRole + 1,
        Image,
        FileName,
        FileSize,
        CreationTime
    };

public slots:
    void updateRows(const QModelIndex &, int start, int end);
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void reset();
    void imageChanged(const QString &id);

private:
    QMap<QString, ImageObject*> m_images;

};

class PMLModel : public EnginioModel
{
    Q_OBJECT
public:
    PMLModel(QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role) const;

    enum Roles {
        Id = Qt::UserRole + 1,
        Object,
        FileId,
        Name,
        Description,
        ScreenShot,
        FileSize,
        CreationTime
    };

    PMLObject * getPMLObject(QString id);
public slots:
    void updateRows(const QModelIndex &, int start, int end);
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void reset();
    void PMLChanged(const QString &id);

private:
    QMap<QString, PMLObject*> m_pmlFiles;

};
#endif
