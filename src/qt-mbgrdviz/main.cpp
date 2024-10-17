#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <vtk-9.3/QQuickVTKItem.h>
#include <vtk-9.3/vtkPolyDataMapper.h>
#include <vtk-9.3/vtkActor.h>
#include <vtk-9.3/vtkRenderer.h>
#include <vtk-9.3/vtkConeSource.h>
#include <vtk-9.3/vtkRenderWindow.h>
#include "TopoGridItem.h"
#include "SharedConstants.h"

using namespace mb_system;

#define TopoGridItemName "topoGridItem"

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

  // Register TopoGridItem type
  qmlRegisterType<TopoGridItem>("VTK", 9, 3, "TopoGridItem");

  // Register SharedConstants type
  qmlRegisterType<SharedConstants>("SharedConstants", 1, 1, "SharedConstants");
  
  engine.load(QUrl("qrc:/main.qml"));
 
  QObject* topLevel = engine.rootObjects().value(0);

  // Find the TopoGridItem instantiatd by QML and load specified grid input
  TopoGridItem *item = topLevel->findChild<TopoGridItem*>(TopoGridItemName);
  if (!item) {
    qFatal() << "Couldn't find TopoGridItem " << TopoGridItemName
	     << " in QML";
    return 1;
  }

  // Specifiy input file for TopoGridItem that was specified on command line
  // (could be nullptr); will be loaded and displayed when item is
  // initialized (if not nullptr)
  item->setGridFilename(gridFilename);
  
  QQuickWindow* window = qobject_cast<QQuickWindow*>(topLevel);
  window->show();

   app.exec();
}
