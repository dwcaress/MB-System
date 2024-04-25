#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQuickItem>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QQmlContext>
#include "GuiNames.h"
#include "Backend.h"
#include "PixmapImage.h"
#include "PixmapContainer.h"

int main(int argc, char *argv[]) {
  
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    qmlRegisterType<PixmapContainer>("PixmapImage", 1, 0, "PixmapContainer");
    qmlRegisterType<PixmapImage>("PixmapImage", 1, 0, "PixmapImage");    
    
    engine.load(url);

    QObject *rootObject = engine.rootObjects().value(0);
    
    Backend backend(rootObject, argc, argv);
    
    // Make backend object and methods accessible to QML
    engine.rootContext()->setContextProperty("backend", &backend);
    
    return app.exec();
}

