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
  class QVtkRenderer;

  /** Catch mouse events */
  class PickerInteractorStyle : public vtkInteractorStyleTrackballCamera {

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
    void initialize(mb_system::QVtkRenderer *renderer,
		    vtkRenderWindowInteractor *interactor) {
      qVtkRenderer_ = renderer;
      interactor_ = interactor;
    }
  
    /// Pick cell
    virtual void OnLeftButtonDown() override;

    /// Pick cell
    virtual void OnLeftButtonUp() override;    

    vtkSmartPointer<vtkPolyData> polyData_;
    vtkSmartPointer<vtkDataSetMapper> selectedMapper_;
    vtkSmartPointer<vtkActor> selectedActor_;

  protected:

    /// Print point id, world coords for a range of y-values
    void testPoints(int x, int y, vtkRenderer *renderer);

    /// Associated renderer
    mb_system::QVtkRenderer *qVtkRenderer_;
  
    /// Associated interactor
    vtkRenderWindowInteractor *interactor_;

    /// Staring mouse position when left button down
    int startMousePos_[2];
    
  };


} // namespace
    
#endif
