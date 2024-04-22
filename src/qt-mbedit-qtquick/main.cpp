#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQuickItem>
#include <QAbstractButton>
#include <QButtonGroup>
#include "GuiNames.h"
#include "MainWindow.h"
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
    
    MainWindow mainWindow(rootObject, argc, argv);

    // MainWindow receives signals from GUI

    QObject *obj =
      rootObject->findChild<QObject*>(XTRACK_SLIDER_NAME);

    QObject::connect(obj, SIGNAL(valueChanged()),
                     &mainWindow, SLOT(onXtrackSliderChanged()));
    
    obj = 
      rootObject->findChild<QObject*>(PINGS_SHOWN_SLIDER_NAME);
    
    QObject::connect(obj, SIGNAL(valueChanged()),
                     &mainWindow, SLOT(onPingsShownSliderChanged()));
    
    obj = 
      rootObject->findChild<QObject*>(VERTICAL_EXAGG_SLIDER_NAME);
    
    QObject::connect(obj, SIGNAL(valueChanged()),
                     &mainWindow, SLOT(onVerticalExaggSliderChanged()));

    obj = 
      rootObject->findChild<QObject*>(PING_STEP_SLIDER_NAME);

    QObject::connect(obj,
		     SIGNAL(valueChanged()),
		     &mainWindow, SLOT(onPingStepSliderChanged()));


    // Connect QML-emitted 'custom' signals
    QObject::connect(rootObject, SIGNAL(editModeSignal(QString)),
                     &mainWindow, SLOT(onEditModeChanged(QString)));

    QObject::connect(rootObject, SIGNAL(ancillDataSignal(QString)),
                     &mainWindow, SLOT(onAncillDataChanged(QString)));

    QObject::connect(rootObject, SIGNAL(sliceSignal(QString)),
                     &mainWindow, SLOT(onSliceChanged(QString)));    
    
    return app.exec();
}

