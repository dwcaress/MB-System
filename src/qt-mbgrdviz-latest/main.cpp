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
  qmlRegisterType<SharedConstants>("SharedConstants", 1, 1, "Constants");  

  
  engine.load(QUrl("qrc:/main.qml"));
 
  QObject* topLevel = engine.rootObjects().value(0);
  QQuickWindow* window = qobject_cast<QQuickWindow*>(topLevel);


  // Find the TopoGridItem
  TopoGridItem *item = topLevel->findChild<TopoGridItem*>(TopoGridItemName);
  if (!item) {
    qFatal() << "Couldn't find TopoGridItem " << TopoGridItemName
	     << " in QML";
    return 1;
  }
  else {
    qDebug() << "found TopoGridItem " << TopoGridItemName;
  }

  item->setGridFilename(gridFilename);
  item->update();
  
  window->show();

   app.exec();
}
