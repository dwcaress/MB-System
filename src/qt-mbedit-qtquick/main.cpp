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
#include "GuiNames.h"
#include "Backend.h"
#include "PixmapImage.h"
#include "PixmapContainer.h"
#include "Emitter.h"

void interruptHandler(int sig) {
  // std::cerr << "interruptHandler(): sig " << sig << "\n";
  fprintf(stdout, "interruptHandler(): got sig %d\n", sig);
  fflush(stdout);
  write(1,"Hello World!", 12); 
  qWarning() << "In interrupt handler";
  exit(1);
}

int main(int argc, char *argv[]) {


  struct sigaction signalAction;
  signalAction.sa_handler = &interruptHandler;
  sigemptyset(&signalAction.sa_mask);
  signalAction.sa_flags = 0;
  sigaction(SIGINT, &signalAction, nullptr);
  
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    Backend backend(argc, argv);
    
    QQmlApplicationEngine engine;
    
    // Make backend object and methods accessible to QML
    engine.setInitialProperties({
	{ "backend", QVariant::fromValue(&backend) }
      });    

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
    
    // Notify C++ when main window is destroyed
    QObject::connect(rootObject, SIGNAL(destroyed()),
		     &backend, SLOT(onMainWindowDestroyed()));

    /* ****
    // Notify QML when Backend has a message to display
    if (!QObject::connect(&backend.staticEmitter_,
			  SIGNAL(showMessage(QString &msg)),
			  rootObject, SLOT(showInfoDialog(QString &msg)))) {

      qWarning() << "*** Failed to connect showMessage() signal to QML";
    }

    *** */


    // TEST TEST TEST
    Emitter emitter;
    // TEST TEST TEST - using non-static emitter
    if (!QObject::connect(&emitter,
			  SIGNAL(showMessage(QVariant)),
			  rootObject, SLOT(showInfoDialog(QVariant)))) {

      qWarning() << "**Failed to connect stand-alone emitter to QML";
    }
    else {
      qDebug() << "connected stand-alone emitter!";
    }

    
    // Notify QML when Backend has a message to display
    // TEST TEST TEST - using non-static emitter
    if (!QObject::connect(&backend.staticEmitter_,
			  SIGNAL(showMessage(QVariant)),
			  rootObject, SLOT(showInfoDialog(QVariant)))) {

      qWarning() << "**Failed to connect static showMessage() signal to QML";
    }
    else {
      qDebug() << "connected to static emitter";
    }

    if (!backend.initialize(rootObject, argc, argv)) {
      qWarning() << "failed to initialize backend";
      exit(1);
    }

    
    return app.exec();

}



