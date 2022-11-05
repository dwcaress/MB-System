#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "BackEnd.h"
#include "QVtkItem.h"
// These first three lines address
// issue described at
// https://stackoverflow.com/questions/18642155/no-override-found-for-vtkpolydatamapper
#include "vtkAutoInit.h"
VTK_MODULE_INIT(vtkRenderingOpenGL2); // VTK was built with vtkRenderingOpenGL2
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType)

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    app.setOrganizationName("MBARI");
    app.setOrganizationDomain("www.mbari.org");
    app.setApplicationName("my app");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    qmlRegisterType<mb_system::QVtkItem>("QVtk", 1, 0, "QVtkItem");
    engine.load(url);

    if (!BackEnd::registerSingleton(argc, argv, &engine)) {
        qCritical("Couldn't create/register BackEnd");
        exit(1);
    }

    return app.exec();
}


