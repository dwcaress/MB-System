#include <vtkPointPicker.h>
#include "PickerInteractorStyle.h"
#include "QVtkRenderer.h"
#include "QVtkItem.h"

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

using namespace mb_system;

PickerInteractorStyle::PickerInteractorStyle():
  qVtkRenderer_(nullptr) {
  selectedMapper_ = vtkSmartPointer<vtkDataSetMapper>::New();
  selectedActor_ = vtkSmartPointer<vtkActor>::New();
}


void PickerInteractorStyle::OnLeftButtonDown() {
  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];
  
  std::cout << "Pixel x,y: " << x
            << " " << y << std::endl;


  vtkRenderer *renderer;

  renderer = 
    this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();

  int *rendererSize = renderer->GetSize();
 std:cout << "FirstRenderer size: " << rendererSize[0] <<  " " <<
          rendererSize[1] << std::endl;

  rendererSize = interactor_->FindPokedRenderer(x, y)->GetSize();
  
  std::cout << "poked renderer size: " << rendererSize[0] << " " <<
            rendererSize[1] << std::endl;
  
  std::cout << "renderer w: " << rendererSize[0] <<
    ", renderer h: " << rendererSize[1] <<
    std::endl;

  std::cout << "rendererSize[1]: " << rendererSize[1] << " y: " << y <<
    std::endl;

  // Correct y coordinate for y-axis flip between Qt and VTK coordinate
  // systems
  int yCorr = rendererSize[1] - y + 1;
    
  std::cout << "Corr Pixel x,y: " << x
            << " " << yCorr << std::endl;  

  vtkNew<vtkPointPicker> picker;
    
  picker->Pick(x, yCorr,
               0, // always zero.
               renderer);


  vtkIdType pointId = picker->GetPointId();
  
  std::cout << "PointId: " << pointId << std::endl;
    
  double *worldCoord = picker->GetPickPosition();
    
  std::cout << "WorldCoord value: " << worldCoord[0] << " " << worldCoord[1] << " "
            << worldCoord[2] << std::endl;

  char buf[256];
  if (pointId != -1) {

      sprintf(buf, "%.1f, %.1f, %.1f",
              worldCoord[0], worldCoord[1], worldCoord[2]);

  }
  else {
    sprintf(buf, "unknown position");
  }
  QString coordMsg(buf);
  qVtkRenderer_->getItem()->setPickedPoint(coordMsg);

  /// Print picker results for range of y values
  //  testPoints(x, y, renderer);
  
  // Forward event
  vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}


void PickerInteractorStyle::testPoints(int x, int y,
                                       vtkRenderer *renderer) {
  vtkNew<vtkPointPicker> picker;
  
  for (int y1 = 0; y1 < 1000; y1++) {
    picker->Pick(x, y1, 0, renderer);
    vtkIdType pointId = picker->GetPointId();
    double *worldCoord = picker->GetPickPosition();

    std::cout << "x: " << x << " y: " << y1 << "  pointId: " << pointId <<
      " worldCoord: " << worldCoord[0] << " " << worldCoord[1] << " " <<
      worldCoord[2] << std::endl;
  }
  
}

