#ifndef TOPOGRIDPICKERINTERACTORSTYLE_H
#define TOPOGRIDPICKERINTERACTORSTYLE_H
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtk/vtkGenericRenderWindowInteractor.h>
#include <vtk/vtkDataSetMapper.h>
#include <vtk/vtkActor.h>
#include <vtk/vtkPolyData.h>

namespace mb_system {

  /// Forward declaration
  class TopoGridItem;
  
    /** Catch mouse events */
    class TopoGridPickerInteractorStyle :
    public vtkInteractorStyleTrackballCamera {

    public:

      vtkTypeMacro(TopoGridPickerInteractorStyle,
		   vtkInteractorStyleTrackballCamera);

      /// Get a new PickerInteractorStyle object
      /// For use with vtkSmartPointer
      static TopoGridPickerInteractorStyle *New() {
	return new TopoGridPickerInteractorStyle();
      }

      /// Constructor
      TopoGridPickerInteractorStyle();

      /// Initialize - REQUIRED
      void initialize(mb_system::TopoGridItem *item,
		      vtkRenderWindowInteractor *interactor) {
	item_ = item;
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

      /// Associated renderer
      mb_system::TopoGridItem *item_;
  
      /// Associated interactor
      vtkRenderWindowInteractor *interactor_;

      /// Staring mouse position when left button down
      int startMousePos_[2];


      void testPoints(int x, int y, vtkRenderer *renderer);
    };
    
}

#endif

