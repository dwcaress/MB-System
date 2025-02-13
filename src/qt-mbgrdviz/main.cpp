#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <vtk-9.3/QQuickVTKItem.h>
#include <vtk-9.3/vtkPolyDataMapper.h>
#include <vtk-9.3/vtkActor.h>
#include <vtk-9.3/vtkRenderer.h>
#include <vtk-9.3/vtkConeSource.h>
#include <vtk-9.3/vtkRenderWindow.h>
#include "TopoDataItem.h"
#include "SharedConstants.h"

using namespace mb_system;

#define TopoDataItemName "topoDataItem"

int main(int argc, char* argv[])
{
  char *gridFilename = nullptr;
  if (argc > 1) {
    gridFilename = argv[1];
  }

  // Sets the graphics API to OpenGLRhi and sets up the surface format for
  // intermixed VTK and QtQuick rendering. 
  QQuickVTKItem::setGraphicsApi();
  
  QGuiApplication app(argc, argv);
 
  QQmlApplicationEngine engine;

  // Register TopoDataItem type
  qmlRegisterType<TopoDataItem>("VTK", 9, 3, "TopoDataItem");

  // Register SharedConstants type
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

  // Specifiy input file for TopoDataItem that was specified on command line
  // (could be nullptr); will be loaded and displayed when item is
  // initialized (if not nullptr)
  item->setGridFilename(gridFilename);
  
  QQuickWindow* window = qobject_cast<QQuickWindow*>(topLevel);
  window->show();

   app.exec();
}
