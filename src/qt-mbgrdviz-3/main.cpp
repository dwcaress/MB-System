#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlComponent>
#include <QObjectList>
#include "MBQuickItem.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    qmlRegisterType<mb_system::MBQuickItem>("mbsystem.MBQuickItem",
					    1, 0, "MBQuickItem");

    // QQmlApplicationEngine engine;
    mb_system::g_appEngine = new QQmlApplicationEngine();

    mb_system::g_appEngine->load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (mb_system::g_appEngine->rootObjects().isEmpty())
        return -1;

    mb_system::g_rootWindow =
      qobject_cast<QQuickWindow*>(mb_system::g_appEngine->rootObjects().value(0));

    
    // Can we find MBQuickItem objects yet?
    QObject *object =
      mb_system::g_rootWindow->findChild<QObject *>("mbQuickItem");
    if (!object) {
      qDebug() << "can't find mbQuickItem";
      return -1;
    }
    mb_system::MBQuickItem *mbQuickItem =
      (mb_system::MBQuickItem *)object;
    
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
