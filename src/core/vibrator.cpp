#include "vibrator.h"
#include <QDebug>

Vibrator::Vibrator(QObject *parent) : QObject(parent)
{
#if defined(Q_OS_ANDROID)
    QAndroidJniObject vibroString = QAndroidJniObject::fromString("vibrator");
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative", "activity", "()Landroid/app/Activity;");
    QAndroidJniObject appctx = activity.callObjectMethod("getApplicationContext","()Landroid/content/Context;");
    vibratorService = appctx.callObjectMethod("getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;", vibroString.object<jstring>());
#endif
}

#if defined(Q_OS_ANDROID)

void Vibrator::vibrate(int milliseconds) {
    if (vibratorService.isValid()) {
        jlong ms = milliseconds;
        jboolean hasvibro = vibratorService.callMethod<jboolean>("hasVibrator", "()Z");
        vibratorService.callMethod<void>("vibrate", "(J)V", ms);
    } else {
        qDebug() << "No vibrator service available";
    }
}

#else
void Vibrator::vibrate(int milliseconds) {
    Q_UNUSED(milliseconds);
}

#endif
