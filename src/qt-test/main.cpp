#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQuickItem>
#include <QAbstractButton>
#include <QList>
#include <QButtonGroup>
#include <QQmlContext>
#include <QVariant>
#include "PixmapImage.h"
#include "PixmapDrawer.h"

#define PIXMAP_NAME "swathPixmapObj"

int main(int argc, char *argv[]) {

  
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    
    // Boilerplate...
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    // QML instantiates a PixmapImage in GUI, and C++ will draw to
    // that - so register PixmapImage class with QML
    qmlRegisterType<mb_system::PixmapImage>("PixmapImage", 1, 0,
					    "PixmapImage");    
    
    engine.load(url);

    QObject *rootObject = engine.rootObjects().value(0);

    // Create pixmap
    int width = 500;
    int height = 500;
    
    QPixmap qPixmap(width, height);

    QPainter qPainter(&qPixmap);

    // Find the PixmapImage in QML
    mb_system::PixmapImage *pixmapImage =
      rootObject->findChild<mb_system::PixmapImage*>(PIXMAP_NAME);

    if (!pixmapImage) {
      qWarning() << "Couldn't find PixmapImage " << PIXMAP_NAME;
      exit(1);
    }

    std::cerr << "Found PixmapImage " << PIXMAP_NAME << "\n";
    pixmapImage->setImage(&qPixmap);
    qDebug() << "qPixmap w=" << qPixmap.width() << ", h=" << qPixmap.height();
    qDebug() << PIXMAP_NAME << " w=" << pixmapImage->width() <<
      ", h=" << pixmapImage->height();
    
    mb_system::PixmapDrawer::fillRectangle(&qPainter, 0, 0, 80, 80,
					   GREEN, SOLID_LINE);
    

    return app.exec();
}



