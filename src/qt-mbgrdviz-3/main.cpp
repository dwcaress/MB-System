#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlComponent>
#include <QObjectList>
#include "MBQuickItem.h"

QQuickWindow *g_rootWindow;
QQmlApplicationEngine *g_appEngine;

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    //    qmlRegisterSingletonType<BackEnd>("MbSystem.BackEnd", 1, 0, "BackEnd", BackEnd::qmlInstance);
    qmlRegisterType<MBQuickItem>("mbsystem.MBQuickItem", 1, 0, "MBQuickItem");

    // QQmlApplicationEngine engine;
    g_appEngine = new QQmlApplicationEngine();

    g_appEngine->load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (g_appEngine->rootObjects().isEmpty())
        return -1;

    g_rootWindow = qobject_cast<QQuickWindow*>(g_appEngine->rootObjects().value(0));


    
    /* ***    
    // Create singleton and register singleton, process command line args
    if (!MBQuickItem::registerSingleton(argc, argv, g_appEngine)) {
      return -1;
    }
    *** */
    
    // Can we find MBQuickItem objects yet?
    QObject *object = g_rootWindow->findChild<QObject *>("mbQuickItem");
    if (!object) {
      qDebug() << "can't find mbQuickItem";
      return -1;
    }
    MBQuickItem *mbQuickItem = (MBQuickItem *)object;
    
    // Stupid-simple command line processing for now.
    // If arguments, last one specifies grid file to load
    if (argc > 1) {
      char *gridFilename = argv[argc-1];
      char *fullPath = realpath(gridFilename, nullptr);
      if (!fullPath) {
	fprintf(stderr, "Grid file \"%s\" not found\n", gridFilename);
	return -1;
      }
      QUrl qUrl(QString("file://" + QString(fullPath)));
      mbQuickItem->setGridSurface(qUrl);
    }


    qDebug("call app.exec");
    return app.exec();
}
