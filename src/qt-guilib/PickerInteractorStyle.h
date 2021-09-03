#ifndef PICKERINTERACTORSTYLE_H
#define PICKERINTERACTORSTYLE_H
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
#include <vtkRenderWindowInteractor.h>


namespace mb_system {

  /// Forward class declaration
  class QVtkItem;

// Catch mouse events
class PickerInteractorStyle : public vtkInteractorStyleTrackballCamera
{

public:

  vtkTypeMacro(PickerInteractorStyle, vtkInteractorStyleTrackballCamera);

  /// Get a new PickerInteractorStyle object
  /// For use with vtkSmartPointer
  static PickerInteractorStyle *New() {
    return new PickerInteractorStyle();
  }

  /// Constructor
  PickerInteractorStyle();

  /// Initialize - REQUIRED
  void initialize(mb_system::QVtkItem *item,
                  vtkRenderWindowInteractor *interactor) {
    item_ = item;
    interactor_ = interactor;
  }
  
  /// Pick cell
  virtual void OnLeftButtonDown() override;

  vtkSmartPointer<vtkPolyData> polyData_;
  vtkSmartPointer<vtkDataSetMapper> selectedMapper_;
  vtkSmartPointer<vtkActor> selectedActor_;

protected:

  /// QVtkItem associated with this 
  mb_system::QVtkItem *item_;

  /// Associated interactor
  vtkRenderWindowInteractor *interactor_;
  
};


} // namespace
    
#endif
