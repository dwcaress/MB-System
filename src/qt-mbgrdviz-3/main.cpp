#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlComponent>
#include <QObjectList>
#include "MBQuickItem.h"

int main(int argc, char *argv[])
{
fprintf(stderr, "%s:%d:%s:\n", __FILE__, __LINE__, __func__);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
fprintf(stderr, "%s:%d:%s:\n", __FILE__, __LINE__, __func__);
    QGuiApplication app(argc, argv);
    //    qmlRegisterSingletonType<BackEnd>("MbSystem.BackEnd", 1, 0, "BackEnd", BackEnd::qmlInstance);
fprintf(stderr, "%s:%d:%s:\n", __FILE__, __LINE__, __func__);
    qmlRegisterType<MBQuickItem>("mbsystem.MBQuickItem", 1, 0, "MBQuickItem");
fprintf(stderr, "%s:%d:%s:\n", __FILE__, __LINE__, __func__);

    // QQmlApplicationEngine engine;
    QQmlApplicationEngine *g_appEngine = new QQmlApplicationEngine();
fprintf(stderr, "%s:%d:%s: %p\n", __FILE__, __LINE__, __func__, g_appEngine);

    QQuickWindow *g_rootWindow = qobject_cast<QQuickWindow*>(g_appEngine->rootObjects().value(0));
fprintf(stderr, "%s:%d:%s: %p %p\n", __FILE__, __LINE__, __func__, g_appEngine, g_rootWindow);

    // Can we find MBQuickItem objects yet?
    QObject *object = g_rootWindow->findChild<QObject *>("mbQuickItem");
fprintf(stderr, "%s:%d:%s: %p %p\n", __FILE__, __LINE__, __func__, g_appEngine, g_rootWindow);
    if (!object) {
      qDebug() << "can't find mbQuickItem";
      return -1;
    }
fprintf(stderr, "%s:%d:%s: %p %p\n", __FILE__, __LINE__, __func__, g_appEngine, g_rootWindow);
    MBQuickItem *mbQuickItem = (MBQuickItem *)object;
fprintf(stderr, "%s:%d:%s: %p %p\n", __FILE__, __LINE__, __func__, g_appEngine, g_rootWindow);

    // Create singleton and register singleton, process command line args
    if (!mbQuickItem->registerSingleton(argc, argv, g_appEngine)) {
      return -1;
    }
fprintf(stderr, "%s:%d:%s: %p %p\n", __FILE__, __LINE__, __func__, g_appEngine, g_rootWindow);

    g_appEngine->load(QUrl(QStringLiteral("qrc:/main.qml")));
fprintf(stderr, "%s:%d:%s: %p\n", __FILE__, __LINE__, __func__, g_appEngine);
    if (g_appEngine->rootObjects().isEmpty())
        return -1;
fprintf(stderr, "%s:%d:%s: %p\n", __FILE__, __LINE__, __func__, g_appEngine);

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
fprintf(stderr, "%s:%d:%s: %p %p\n", __FILE__, __LINE__, __func__, g_appEngine, g_rootWindow);


    qDebug("call app.exec");
    return app.exec();
}
