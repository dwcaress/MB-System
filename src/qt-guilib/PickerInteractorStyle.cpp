#include <vtkPointPicker.h>
#include "PickerInteractorStyle.h"
#include "QVtkItem.h"

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

using namespace mb_system;

PickerInteractorStyle::PickerInteractorStyle():
  item_(nullptr) {
  selectedMapper_ = vtkSmartPointer<vtkDataSetMapper>::New();
  selectedActor_ = vtkSmartPointer<vtkActor>::New();
}


void PickerInteractorStyle::OnLeftButtonDown() {
    std::cout << "OnLeftButtonDown():" << std::endl;
    if (!item_) {
      std::cerr <<
        "PickerInteractorStyle::onLeftButtonDown(): null item! Did you call initialize()?"
                << std::endl;
    }
    
    vtkNew<vtkNamedColors> colors;

    // Get the location of the click (in window coordinates)
    int* pos = this->GetInteractor()->GetEventPosition();

    vtkNew<vtkPointPicker> picker;
    
    // y-position (pos[1]) has been flipped by windowInteractor in
    // QVtkRenderer::render() - restore it
    int *size = interactor_->GetSize();
    int y = size[1] - pos[1] - 1;

    std::cout << "renderWindow: w=" << size[0] << ", h=" << size[1] << std::endl;
    std::cout << "Picker: pos[0]=" << pos[0] <<
      ", pos[1]=" << pos[1] << ", y=" << y << std::endl;

    // Pick from this location.
    picker->Pick(pos[0], y, 0, this->GetDefaultRenderer());
    double* worldPosition = picker->GetPickPosition();

    
    if (picker->GetPointId() != -1) {
      // Picked a point in the polygon dataset - emit signal from item
      char buf[256];
      sprintf(buf, "%.1f, %.1f, %.1f",
              worldPosition[0], worldPosition[1], worldPosition[2]);

      QString coordMsg(buf);
      
      item_->setPickedPoint(coordMsg);
    }
    
    std::cout << "Picker: pointId=" << picker->GetPointId() <<
      ", world[0]=" << worldPosition[0] <<
      ", worldPosition[1]=" << worldPosition[1] <<
      ", worldPos[3]=" << worldPosition[2] << std::endl;

    // Forward events
    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}


    
