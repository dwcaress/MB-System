#ifndef CELLPICKERINTERACTORSTYLE_H
#define CELLPICKERINTERACTORSTYLE_H
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkCellPicker.h>
#include <vtkCommand.h>
#include <vtkDataSetMapper.h>
#include <vtkExtractSelection.h>
#include <vtkIdTypeArray.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkSmartPointer.h>
#include <vtkTriangleFilter.h>
#include <vtkUnstructuredGrid.h>


namespace mb_system {

// Catch mouse events
class CellPickerInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static CellPickerInteractorStyle* New();

  CellPickerInteractorStyle()
  {
    selectedMapper_ = vtkSmartPointer<vtkDataSetMapper>::New();
    selectedActor_ = vtkSmartPointer<vtkActor>::New();
  }

  virtual void OnLeftButtonDown() override
  {
    std::cout << "OnLeftButtonDown():" << std::endl;
    vtkNew<vtkNamedColors> colors;

    // Get the location of the click (in window coordinates)
    int* pos = this->GetInteractor()->GetEventPosition();

    vtkNew<vtkCellPicker> picker;
    picker->SetTolerance(0.0005);

    std::cout << "CellPicker: pos[0]=" << pos[0] <<
      ", pos[1]=" << pos[1] << std::endl;
    
    // Pick from this location.
    picker->Pick(pos[0], pos[1], 0, this->GetDefaultRenderer());

    double* worldPosition = picker->GetPickPosition();

    std::cout << "CellPicker: world[0]=" << worldPosition[0] <<
      ", worldPosition[1]=" << worldPosition[1] <<
      ", worldPos[3]=" << worldPosition[2] << std::endl;

    
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
    // Forward events
    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
  }

  vtkSmartPointer<vtkPolyData> polyData_;
  vtkSmartPointer<vtkDataSetMapper> selectedMapper_;
  vtkSmartPointer<vtkActor> selectedActor_;
};

vtkStandardNewMacro(CellPickerInteractorStyle);

} // namespace
    
#endif
