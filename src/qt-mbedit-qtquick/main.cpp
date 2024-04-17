#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQuickItem>
#include <QQmlContext>
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
    GuiNames guiNames;

    // Make GUI component names accessible to QML
    engine.rootContext()->setContextProperty("xTrackSliderObj",
					     guiNames.xTrackSlider());

    engine.rootContext()->setContextProperty("pingsShownSliderObj",
					     guiNames.pingsShownSlider());

    engine.rootContext()->setContextProperty("verticalExaggSliderObj",
					     guiNames.verticalExaggSlider());

    engine.rootContext()->setContextProperty("pingStepSliderObj",
					     guiNames.pingStepSlider());

    engine.rootContext()->setContextProperty("swathCanvasObj",
					     guiNames.swathCanvas());    
    
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    qmlRegisterType<PixmapContainer>("bogus", 1, 0, "PixmapContainer");
    qmlRegisterType<PixmapImage>("bogus", 1, 0, "PixmapImage");    
    
    engine.load(url);

    QObject *rootObject = engine.rootObjects().value(0);
    
    MainWindow mainWindow(rootObject);

    // MainWindow receives signals from GUI
    QQuickItem *slider =
      rootObject->findChild<QQuickItem*>(guiNames.xTrackSlider());

    QObject::connect((QObject *)slider, SIGNAL(valueChanged()),
                     &mainWindow, SLOT(onXtrackSliderChanged()));

    slider =
      rootObject->findChild<QQuickItem*>(guiNames.verticalExaggSlider());

    QObject::connect((QObject *)slider, SIGNAL(valueChanged()),
                     &mainWindow, SLOT(onVerticalExaggSliderChanged()));

    slider =
      rootObject->findChild<QQuickItem*>(guiNames.pingStepSlider());

    QObject::connect((QObject *)slider, SIGNAL(valueChanged()),
                     &mainWindow, SLOT(onPingStepSliderChanged()));
    
    
    return app.exec();
}

