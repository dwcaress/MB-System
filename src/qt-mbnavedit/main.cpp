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
#include "Emitter.h"

Backend *theBackend_ = nullptr;

void interruptHandler(int sig) {
  // std::cerr << "interruptHandler(): sig " << sig << "\n";
  fprintf(stdout, "interruptHandler(): got sig %d\n", sig);
  fflush(stdout);
  write(1,"Hello World!", 12);
  
  if (theBackend_) {
    theBackend_->onMainWindowDestroyed();
  }
  
  exit(1);
}

int main(int argc, char *argv[]) {

  // Handle interruption
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
    theBackend_ = &backend;
    
    QQmlApplicationEngine engine;
    
    // Make backend object and methods accessible to QML
    engine.setInitialProperties({
	{ "backend", QVariant::fromValue(&backend) }
      });    


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
    
    // QML notifies C++ when root window is destroyed
    QObject::connect(rootObject, SIGNAL(destroyed()),
		     &backend, SLOT(onMainWindowDestroyed()));

    // Backend C++ signals QML with message to display
    if (!QObject::connect(&backend.emitter_,
			  SIGNAL(showMessage(QVariant)),
			  rootObject, SLOT(showInfoDialog(QVariant)))) {

      qWarning() << "**Failed to connect showMessage() signal to QML";
    }
    else {
      qDebug() << "connected to emitter";
    }

    if (!backend.initialize(rootObject, argc, argv)) {
      qWarning() << "failed to initialize backend";
      exit(1);
    }

    return app.exec();
}



