#ifndef POCKETIMAGEPROVIDER
#define POCKETIMAGEPROVIDER

#include <QImage>
#include <QQuickImageProvider>

class PocketImageProvider : public QObject, public QQuickImageProvider
{
Q_OBJECT

public:
    PocketImageProvider(QObject*);
    ~PocketImageProvider();
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);
//    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize);

};

#endif // POCKETIMAGEPROVIDER

