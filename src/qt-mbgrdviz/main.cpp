#include <sstream>
#include <thread>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtGraphs/QAbstractAxis>    /// TEST
#include <QQuickVTKItem.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkConeSource.h>
#include <vtkRenderWindow.h>
#include "TopoDataItem.h"
#include "SharedConstants.h"

using namespace std;
using namespace mb_system;

#define TopoDataItemName "topoDataItem"

int main(int argc, char* argv[])
{

#if defined(Q_OS_MACOS)
  // Do not use native MacOS menu stuff, as this app's QML file
  // assigns tooltips to menu items
  
  // For older Qt versions, this may be required.
    QGuiApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    // This is the modern and more precise attribute for disabling
    // native menu windows.
    QGuiApplication::setAttribute(Qt::AA_DontUseNativeMenuWindows);
#endif

  std::cerr << "main() thread: " <<
    std::this_thread::get_id() << "\n";
  
  char *topoDataFile = nullptr;

  bool error = false;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-I") && i <= argc-2) {
      topoDataFile = argv[++i];
    }
    else if (!strcmp(argv[i], "-testpoints") && i <= argc-2) {
      // Parse two points from next argument:
      // beginX,beginY,endX,endY
      std::stringstream input(argv[++i]);
      string token;
      char delim = ',';
      int nToken = 0;
      while (getline(input, token, delim)) {
	cout << token << " ";
	nToken++;
      }
      cout << "\n";
      if (nToken != 4) {
	cerr << "Expecting 4 coords\n";
      }
      
    }
    else {
      cerr << "Unknown option: " << argv[i] << "\n";
      error = true;
    }
  }

  if (error) {
    cerr << "usage: " << argv[0] << "[-I inputFle][-testpoints x1,y1,x2,y2]\n";
    exit(1);
  }

  // Sets the graphics API to OpenGLRhi and sets up the surface format for
  // intermixed VTK and QtQuick rendering. 
  QQuickVTKItem::setGraphicsApi();

  /// DEBUG -
  // Check the platform name
  if (QGuiApplication::platformName() == QLatin1String("xcb")) {
    std::cerr << "Qt 6 is running on X11 (xcb plugin) on macOS.\n";
  } else {
    std::cerr << "Qt 6 is not running on X11\n";
  }
  
  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;

  // Register TopoDataItem type
  qmlRegisterType<TopoDataItem>("VTK", 9, 3, "TopoDataItem");

  qmlRegisterType<SharedConstants>("SharedConstants", 1, 1, "SharedConstants");
  
  engine.load(QUrl("qrc:/main.qml"));
 
  QObject* topLevel = engine.rootObjects().value(0);

  // Find the TopoDataItem instantiatd by QML and load specified grid input
  TopoDataItem *item = topLevel->findChild<TopoDataItem*>(TopoDataItemName);
  if (!item) {
    qFatal() << "Couldn't find TopoDataItem " << TopoDataItemName
	     << " in QML";
    return 1;
  }

  // Specify input file for TopoDataItem that was specified on command line
  // (could be nullptr); will be loaded and displayed when item is
  // initialized (if not nullptr)
  item->setDataFilename(topoDataFile);
  
  QQuickWindow* window = qobject_cast<QQuickWindow*>(topLevel);
  window->show();

   app.exec();
}
