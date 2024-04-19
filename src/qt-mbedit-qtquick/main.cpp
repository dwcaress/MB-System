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
    /* ***
    QQuickItem *qqItem =
      rootObject->findChild<QQuickItem*>(XTRACK_SLIDER_NAME);

    QObject::connect((QObject *)qqItem, SIGNAL(valueChanged()),
                     &mainWindow, SLOT(onXtrackSliderChanged()));

		     *** */

    // Can we find and connect as just QObject type?
    QObject *obj2 =
      rootObject->findChild<QObject*>(XTRACK_SLIDER_NAME);

    QObject::connect((QObject *)obj2, SIGNAL(valueChanged()),
                     &mainWindow, SLOT(onXtrackSliderChanged()));
    
    
    QQuickItem *qqItem =
      rootObject->findChild<QQuickItem*>(PINGS_SHOWN_SLIDER_NAME);
    
    QObject::connect((QObject *)qqItem, SIGNAL(valueChanged()),
                     &mainWindow, SLOT(onPingsShownSliderChanged()));
    
    qqItem =
      rootObject->findChild<QQuickItem*>(VERTICAL_EXAGG_SLIDER_NAME);
    
    QObject::connect((QObject *)qqItem, SIGNAL(valueChanged()),
                     &mainWindow, SLOT(onVerticalExaggSliderChanged()));

    qqItem =
      rootObject->findChild<QQuickItem*>(PING_STEP_SLIDER_NAME);

    qDebug() << "qqItem object info:";
    qqItem->dumpObjectInfo();
    
    QObject::connect((QObject *)qqItem,
		     SIGNAL(valueChanged()),
		     &mainWindow, SLOT(onPingStepSliderChanged()));

    qDebug() << "*** TEST connect " << PING_STEP_SLIDER_NAME <<
      " to baselineOffsetChanged(double)";

    if (!QObject::connect((QObject *)qqItem,
			  SIGNAL(baselineOffsetChanged(double)),
			  &mainWindow, SLOT(testSlot(double)))) {
      qWarning() << "connect() to testSlot() failed";
    }
    else {
      qDebug() << "connect() to testSlot() OK";
    }
    
    // NOTE: can find QButtonGroup only if cast to QObject *, for some odd
    // reason
    QObject *buttonGroup =
      rootObject->findChild<QObject*>(EDIT_MODES_NAME);

    if (!buttonGroup) {
      qWarning() << "Couldn't find QML object named " << EDIT_MODES_NAME;
    }

    qDebug() << "buttonGroup object info:";
    buttonGroup->dumpObjectInfo();

    qDebug() << "connect to ButtonGroup clicked signal";
    if (!QObject::connect((QObject *)buttonGroup,
			  SIGNAL(clicked()),
			  &mainWindow, SLOT(onEditModesChanged()))) {
      qWarning() << "connect() to clicked signal failed";
    }

    // Connect QML-emitted 'custom' signals
    QObject::connect(rootObject, SIGNAL(qmlSignal(QString)),
                     &mainWindow, SLOT(qmlSigSlot(QString)));

    /* ***
    MainWindow::connectRadioButton(rootObject, &mainWindow, TOGGLE_EDIT_NAME);
    MainWindow::connectRadioButton(rootObject, &mainWindow, PICK_EDIT_NAME);
    MainWindow::connectRadioButton(rootObject, &mainWindow, ERASE_EDIT_NAME);
    MainWindow::connectRadioButton(rootObject, &mainWindow, RESTORE_EDIT_NAME);
    MainWindow::connectRadioButton(rootObject, &mainWindow, GRAB_EDIT_NAME);    
    *** */
    return app.exec();
}

