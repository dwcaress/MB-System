#include <sstream>
#include <thread>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtGraphs/QAbstractAxis>
#include <QQuickVTKItem.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkConeSource.h>
#include <vtkRenderWindow.h>
#include "TopoDataItem.h"
#include "TopoDataset.h"
#include "SharedConstants.h"

using namespace std;
using namespace mb_system;

#define TopoDataItemName "topoDataItem"

int main(int argc, char* argv[])
{
#if defined(Q_OS_MACOS)
  QGuiApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
  QGuiApplication::setAttribute(Qt::AA_DontUseNativeMenuWindows);
#endif

  std::cerr << "main() thread: " << std::this_thread::get_id() << "\n";

  char *topoDataFile = nullptr;
  bool error = false;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-I") && i <= argc-2) {
      topoDataFile = argv[++i];
    }
    else {
      cerr << "Unknown option: " << argv[i] << "\n";
      error = true;
    }
  }
  if (error) {
    cerr << "usage: " << argv[0] << " [-I inputFile]\n";
    exit(1);
  }

  QQuickVTKItem::setGraphicsApi();

  if (QGuiApplication::platformName() == QLatin1String("xcb"))
    std::cerr << "Qt 6 running on X11 (xcb)\n";
  else
    std::cerr << "Qt 6 not running on X11\n";

  QGuiApplication app(argc, argv);
  app.setApplicationName("qt-mbgrdviz");

  QQmlApplicationEngine engine;

  // Register view type (instantiated by QML)
  qmlRegisterType<TopoDataItem>("VTK", 9, 3, "TopoDataItem");

  // Register dataset type so Qt's meta-object system knows the pointer type
  // used in TopoDataItem's dataset Q_PROPERTY.  QML does not create datasets;
  // the single instance is owned here in main().
  qmlRegisterUncreatableType<TopoDataset>(
      "Mbgrdviz", 1, 0, "TopoDataset",
      "TopoDataset instances are created in C++, not QML");

  auto *sharedConstants = new SharedConstants();
  qmlRegisterSingletonInstance("Mbgrdviz", 1, 0, "SharedConstants",
                               sharedConstants);

  // ── Shared dataset ────────────────────────────────────────────────────────
  // Single owner of all topo data.  Parented to app so it is destroyed with
  // the application.  Both the main-window and (future) edit-window
  // TopoDataItems will hold a non-owning pointer to this object.
  auto *dataset = new TopoDataset(&app);

  engine.load(QUrl("qrc:/main.qml"));

  QObject *topLevel = engine.rootObjects().value(0);
  if (!topLevel) {
    qFatal() << "Failed to load QML root object";
    return 1;
  }

  // ── Bind dataset to view ──────────────────────────────────────────────────
  TopoDataItem *item = topLevel->findChild<TopoDataItem*>(TopoDataItemName);
  if (!item) {
    qFatal() << "Couldn't find TopoDataItem" << TopoDataItemName << "in QML";
    return 1;
  }

  // Wire signals: dataLoaded → rebuild pipeline; qualityChanged → re-render;
  // errorOccurred → forwarded to QML error dialog.
  item->setDataset(dataset);

  // If a file was given on the command line, load it before showing the
  // window.  TopoDataset::loadFile() emits dataLoaded(); because pipeline_
  // is still null at this point, onDatasetLoaded() is a no-op.
  // initializeVTK() (called from the render thread on first frame) then
  // calls assemblePipeline() → connectDataset(), which finds the data
  // already loaded and wires up the pipeline immediately.
  if (topoDataFile) {
    dataset->loadFile(QString::fromLocal8Bit(topoDataFile));
    emit item->dataFilenameChanged(topoDataFile);
  }

  QQuickWindow *window = qobject_cast<QQuickWindow*>(topLevel);
  window->show();

  return app.exec();
}
