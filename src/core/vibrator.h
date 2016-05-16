#ifndef VIBRATOR_H
#define VIBRATOR_H

#include <QObject>

#if defined(Q_OS_ANDROID)
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#endif

class Vibrator : public QObject
{
    Q_OBJECT
public:
    explicit Vibrator(QObject *parent = 0);
signals:
public slots:
    void vibrate(int milliseconds);
private:
#if defined(Q_OS_ANDROID)
    QAndroidJniObject vibratorService;
#endif
};

#endif // VIBRATOR_H
