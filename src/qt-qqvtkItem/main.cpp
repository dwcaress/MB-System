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

    /// Add RTTI to Data
    vtkTypeMacro(Data, vtkObject);  

    int i = 99;
    
    // Persistent VTK pipeline items
    vtkNew<vtkActor> actor_;
    vtkNew<vtkRenderer> renderer_;
    vtkNew<vtkPolyDataMapper> mapper_;
    vtkNew<vtkConeSource> cone_;    
  };
  
  vtkUserData initializeVTK(vtkRenderWindow* renderWindow) override  {
    
    // Allocate a Data object
    vtkNew<Data> vtk;

    std::cout << "vtk->i = " << vtk->i << "\n";
    renderWindow->AddRenderer(vtk->renderer_);

    // Add cone pipeline to the view
    vtk->mapper_->SetInputConnection(vtk->cone_->GetOutputPort());
    vtk->actor_->SetMapper(vtk->mapper_);
    vtk->renderer_->AddActor(vtk->actor_);
    vtk->renderer_->ResetCamera();
    vtk->renderer_->SetBackground2(0.7, 0.7, 0.7);
    vtk->renderer_->SetGradientBackground(true);

    return vtk;
  }
  
};

// Create static Data object
vtkStandardNewMacro(MyVtkItem::Data);


int main(int argc, char* argv[])
{

  vtkNew<MyVtkItem::Data> vtk;
  std::cout << "main(): vtk->i = " << vtk->i << "\n";
    
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

   app.exec();
}
