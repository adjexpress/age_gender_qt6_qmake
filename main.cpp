/*
 * Application main entry point:
*/

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "cvcustomdetector.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // set application name,organization name and domain
    app.setOrganizationName("AgeGender");
    app.setOrganizationDomain("JazzVinay");
    app.setApplicationName("upwork");


    // set media playback backend
#ifdef Q_OS_ANDROID
    // cv::imwrite("first.png",first);
    qputenv("QT_MEDIA_BACKEND", "android");
#endif
    // setenv("QT_MEDIA_BACKEND", "ffmpeg");

    // register cpp components to be used in Qml
    qmlRegisterType<CVcustomDetector>("CVcustomDetector", 1, 0, "CustomDetector");
    qRegisterMetaType<QList<QRect>>("QList<QRect>");


    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/age_gender_qt6_qmake/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
