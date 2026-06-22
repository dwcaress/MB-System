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
#include "TopoDataset.h"
#include "TopoDataItem.h"
#include "SurfaceDataItem.h"
#include "EditDataItem.h"
#include "SharedConstants.h"

using namespace std;
using namespace mb_system;

#define APP_NAME "Mbeditviz"

// objectName values assigned in main.qml
static constexpr const char *SurfaceItemName = "surfaceDataItem";
static constexpr const char *EditItemName    = "editDataItem";

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

  QQmlApplicationEngine engine;

  // ── QML type registration ─────────────────────────────────────────────────
  // SurfaceDataItem and EditDataItem are the concrete types instantiated by
  // QML.  TopoDataItem is registered so any QML code that references the base
  // type (e.g. property declarations) continues to compile.
  qmlRegisterType<TopoDataItem>    (APP_NAME, 1, 0, "TopoDataItem");
  qmlRegisterType<SurfaceDataItem> (APP_NAME, 1, 0, "SurfaceDataItem");
  qmlRegisterType<EditDataItem>    (APP_NAME, 1, 0, "EditDataItem");

  // Dataset is created in C++; QML only holds a pointer to it via properties.
  qmlRegisterUncreatableType<TopoDataset>(
      APP_NAME, 1, 0, "TopoDataset",
      "TopoDataset instances are created in C++, not QML");

  auto *sharedConstants = new SharedConstants();
  qmlRegisterSingletonInstance(APP_NAME, 1, 0, "SharedConstants",
                               sharedConstants);

  // ── Shared dataset ────────────────────────────────────────────────────────
  // Single source of truth for topo data and the quality array.
  // Both view items hold a non-owning pointer to this object.
  auto *dataset = new TopoDataset(&app);

  engine.load(QUrl("qrc:/main.qml"));

  QObject *topLevel = engine.rootObjects().value(0);
  if (!topLevel) {
    qFatal() << "Failed to load QML root object";
    return 1;
  }

  // ── Locate view items ─────────────────────────────────────────────────────
  auto *surfaceItem = topLevel->findChild<SurfaceDataItem*>(SurfaceItemName);
  if (!surfaceItem) {
    qFatal() << "Couldn't find SurfaceDataItem" << SurfaceItemName << "in QML";
    return 1;
  }

  // Edit item lives inside a child Window in QML; findChild searches the full
  // object tree so it is found regardless of nesting depth.
  auto *editItem = topLevel->findChild<EditDataItem*>(EditItemName);
  if (!editItem) {
    qFatal() << "Couldn't find EditDataItem" << EditItemName << "in QML";
    return 1;
  }

  // ── Bind shared dataset to both views ─────────────────────────────────────
  // Each call connects three signals on the dataset:
  //   dataLoaded    → rebuild that view's rendering pipeline
  //   qualityChanged → re-render that view (the shared polyData is already
  //                    modified in place by TopoDataset::setPointQuality)
  //   errorOccurred → forwarded to each view's own errorOccurred signal,
  //                   which QML connects to the error dialog
  surfaceItem->setDataset(dataset);
  editItem->setDataset(dataset);

  // ── Wire selection → edit window ──────────────────────────────────────────
  // Connection type is platform-dependent due to Qt render-loop differences:
  //
  // Ubuntu / X11 (xcb): VTK interactor fires OnLeftButtonUp() on the QSG
  //   render thread.  QueuedConnection posts setEditBounds() to the main-thread
  //   event queue so it runs safely on the main thread.
  //
  // macOS / Windows: VTK events fire on the main thread.  DirectConnection
  //   delivers setEditBounds() synchronously inside the emit call — before
  //   editWindow.visible=true triggers the first render frame — so
  //   connectDataset() sees boundsSet_=true and applies the clip immediately.
  //   QueuedConnection would defer past the first frame; dispatch_async() cannot
  //   reliably re-trigger a new Metal frame after that, leaving the window black.
  const Qt::ConnectionType editConnType =
      (QGuiApplication::platformName() == QLatin1String("xcb"))
      ? Qt::QueuedConnection
      : Qt::DirectConnection;

  QObject::connect(surfaceItem, &SurfaceDataItem::editBoundsChanged,
                   editItem,    &EditDataItem::setEditBounds,
                   editConnType);

  // ── Load command-line file (if given) ─────────────────────────────────────
  // loadFile() emits dataLoaded(); pipeline_ is still null on both items at
  // this point so onDatasetLoaded() is a no-op.  initializeVTK() fires from
  // the render thread on the first frame and calls connectDataset(), which
  // finds the data already loaded and wires up both pipelines immediately.
  if (topoDataFile) {
    dataset->loadFile(QString::fromLocal8Bit(topoDataFile));
  }

  QQuickWindow *window = qobject_cast<QQuickWindow*>(topLevel);
  window->show();

  return app.exec();
}
