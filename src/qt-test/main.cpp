#include <vtkCubeSource.h>
#include <vtkNamedColors.h>
#include <vtkActor.h>
#include <vtkTextActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkLight.h>

#include "LightPositionInteractorStyle.h"

int main(int argc, char* argv[])
{
  // Create a named colors object for easier color access
  vtkNew<vtkNamedColors> colors;

  // Create a simple scene
  vtkNew<vtkCubeSource> cube;
  cube->SetXLength(1.0);
  cube->SetYLength(1.0);
  cube->SetZLength(1.0);
    
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(cube->GetOutputPort());
    
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(colors->GetColor3d("IndianRed").GetData());
    
  // Create renderer, render window, and interactor
  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("SteelBlue").GetData());
    
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(800, 600);
  renderWindow->SetWindowName("Light Position Control");
    
  vtkNew<vtkRenderWindowInteractor> interactor;
  interactor->SetRenderWindow(renderWindow);
    
  // Create and position a light source
  vtkNew<vtkLight> light;
  light->SetColor(1.0, 1.0, 1.0);
  light->SetPosition(5.0, 5.0, 5.0);
  light->SetFocalPoint(0.0, 0.0, 0.0);
  light->SetIntensity(1.0);
  renderer->AddLight(light);
    
  // Create custom interactor style
  vtkNew<LightPositionInteractorStyle> style;
  style->setLight(light);
  style->setRenderer(renderer);
  interactor->SetInteractorStyle(style);
    
  // Add instructions to the window
  vtkNew<vtkTextActor> textActor;
  textActor->SetInput("Hold Shift + Left Mouse Button and move to adjust light position");
  textActor->GetTextProperty()->SetFontSize(12);
  textActor->GetTextProperty()->SetColor(1.0, 1.0, 1.0);
  textActor->SetPosition(10, 10);
  renderer->AddActor2D(textActor);
    
  // Start the interaction
  renderWindow->Render();
  interactor->Start();
    
  return EXIT_SUCCESS;
}
