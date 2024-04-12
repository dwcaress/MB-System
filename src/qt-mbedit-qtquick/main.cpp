#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>

int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    QQuickView view(QUrl::fromLocalFile("main.qml"));
    QObject *item = view.rootObject();

    MainWindow mainWindow;
    
    QObject::connect(item, SIGNAL(qmlSignal(QString)),
                     &mainWindow, SLOT(cppSlot(QString)));

    view.show();
    return app.exec();
}
}
