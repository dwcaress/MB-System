#include <sstream>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtGraphs/QAbstractAxis>    /// TEST
#include <vtk-9.3/QQuickVTKItem.h>
#include <vtk-9.3/vtkPolyDataMapper.h>
#include <vtk-9.3/vtkActor.h>
#include <vtk-9.3/vtkRenderer.h>
#include <vtk-9.3/vtkConeSource.h>
#include <vtk-9.3/vtkRenderWindow.h>
#include "TopoDataItem.h"
#include "SharedConstants.h"
#include "GuiNames.h"

using namespace std;
using namespace mb_system;

#define TopoDataItemName "topoDataItem"

int main(int argc, char* argv[])
{
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
  
  QGuiApplication app(argc, argv);
 
  QQmlApplicationEngine engine;

  // Register TopoDataItem type
  qmlRegisterType<TopoDataItem>("VTK", 9, 3, "TopoDataItem");

  qmlRegisterType<SharedConstants>("SharedConstants", 1, 1, "SharedConstants");

  // Register GuiNames type
  qmlRegisterType<GuiNames>("GuiNames", 1, 1, "GuiNames");

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
