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
    
    std::cout << "Picker: pos[0]=" << pos[0] <<
      ", pos[1]=" << pos[1] << std::endl;
    
    // Pick from this location.
    picker->Pick(pos[0], pos[1], 0, this->GetDefaultRenderer());
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

    /* ***
    std::cout << "Cell id is: " << picker->GetCellId() << std::endl;
    std::cout << "Clipping plane id: " << picker->GetClippingPlaneId() << std::endl;
    
    if (picker->GetCellId() != -1)
    {
      std::cout << "Pick position is: " << worldPosition[0] << " "
                << worldPosition[1] << " " << worldPosition[2] << endl;

      vtkNew<vtkIdTypeArray> ids;
      ids->SetNumberOfComponents(1);
      ids->InsertNextValue(picker->GetCellId());

      vtkNew<vtkSelectionNode> selectionNode;
      selectionNode->SetFieldType(vtkSelectionNode::CELL);
      selectionNode->SetContentType(vtkSelectionNode::INDICES);
      selectionNode->SetSelectionList(ids);

      vtkNew<vtkSelection> selection;
      selection->AddNode(selectionNode);

      vtkNew<vtkExtractSelection> extractSelection;
      extractSelection->SetInputData(0, this->polyData_);
      extractSelection->SetInputData(1, selection);
      extractSelection->Update();

      // In selection
      vtkNew<vtkUnstructuredGrid> selected;
      selected->ShallowCopy(extractSelection->GetOutput());

      std::cout << "There are " << selected->GetNumberOfPoints()
                << " points in the selection." << std::endl;
      std::cout << "There are " << selected->GetNumberOfCells()
                << " cells in the selection." << std::endl;
      selectedMapper_->SetInputData(selected);
      selectedActor_->SetMapper(selectedMapper_);
      selectedActor_->GetProperty()->EdgeVisibilityOn();
      selectedActor_->GetProperty()->SetColor(
          colors->GetColor3d("Red").GetData());

      selectedActor_->GetProperty()->SetLineWidth(3);

      this->Interactor->GetRenderWindow()
          ->GetRenderers()
          ->GetFirstRenderer()
          ->AddActor(selectedActor_);
    }
    *** */
    // Forward events
    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}


    
