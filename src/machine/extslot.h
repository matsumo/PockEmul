#ifndef EXTSLOT
#define EXTSLOT

#include <QObject>

class Cconnector;
class QImage;

class CextSlot:public QObject {
    Q_OBJECT
            Q_PROPERTY(int doorangle READ doorAngle WRITE setDoorAngle)
public:

    CextSlot(QObject *parent);

    Cconnector *pConn;
    bool hasDoor;
    QImage *doorImage;

    void setDoorAngle(int value) { m_backdoorAngle = value; }
    int doorAngle() const { return m_backdoorAngle; }
    int m_backdoorAngle;

    bool doorOpen;
    bool doorFlipping;
    void animateDoor(bool _open);
    void manageCardVisibility();
public slots:
    void endDoorAnimation();
};

#endif // EXTSLOT

