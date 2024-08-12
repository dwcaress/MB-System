#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <vtk-9.3/QQuickVTKItem.h>
#include <vtk-9.3/vtkPolyDataMapper.h>
#include <vtk-9.3/vtkActor.h>
#include <vtk-9.3/vtkRenderer.h>
#include <vtk-9.3/vtkConeSource.h>
#include <vtk-9.3/vtkRenderWindow.h>


struct MyVtkItem : QQuickVTKItem {

  // Q_OBJECT
  
  struct Data : vtkObject
  {
    static Data* New();
    vtkTypeMacro(Data, vtkObject);
  };
  
  vtkUserData initializeVTK(vtkRenderWindow* renderWindow) override  {
    auto vtk = vtkNew<Data>();

    // Create a cone pipeline and add it to the view
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkActor> actor;
    vtkNew<vtkPolyDataMapper> mapper;
    vtkNew<vtkConeSource> cone;
    renderWindow->AddRenderer(renderer);
    mapper->SetInputConnection(cone->GetOutputPort());
    actor->SetMapper(mapper);
    renderer->AddActor(actor);
    renderer->ResetCamera();
    renderer->SetBackground2(0.7, 0.7, 0.7);
    renderer->SetGradientBackground(true);

    return vtk;
  }
  
};

vtkStandardNewMacro(MyVtkItem::Data);


int main(int argc, char* argv[])
{
  // Sets the graphics API to OpenGLRhi and sets up the surface format for
  // intermixed VTK and QtQuick rendering. 
  QQuickVTKItem::setGraphicsApi();
  
  QGuiApplication app(argc, argv);
 
  QQmlApplicationEngine engine;
  qmlRegisterType<MyVtkItem>("VTK", 9, 3, "MyVtkItem");
  engine.load(QUrl("qrc:/main.qml"));
 
  QObject* topLevel = engine.rootObjects().value(0);
  QQuickWindow* window = qobject_cast<QQuickWindow*>(topLevel);
 
  window->show();

  /// Find myVtkItem in QML tree
  //  MyVtkItem *myVtkItem =
  // topLevel->findChild<MyVtkItem*>("ConeView");
  
  app.exec();
}
