#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
#include "datavisualizationqml2/declarativesurface_p.h"
#include "BackEnd.h"
#include "MyTimer.h"


QQuickWindow *g_rootWindow;
QQmlApplicationEngine *g_appEngine;


int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    qmlRegisterSingletonType<BackEnd>("MbSystem.BackEnd", 1, 0, "BackEnd", BackEnd::qmlInstance );

    g_appEngine = new QQmlApplicationEngine();
    g_appEngine->load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (g_appEngine->rootObjects().isEmpty()) {
        return -1;
    }

    g_rootWindow =
            qobject_cast<QQuickWindow*>(g_appEngine->rootObjects().value(0));

    if (!BackEnd::registerSingleton(argc, argv, g_appEngine)) {
        fprintf(stderr, "BackEnd::registerSingleton() failed\n");
        exit(1);
    }

    // Wait a bit for object load to complete...
  //  MyTimer timer(g_rootWindow);
  //  timer.start(5);

    return app.exec();
}
