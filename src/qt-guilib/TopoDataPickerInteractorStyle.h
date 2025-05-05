#ifndef TopoDataPickerInteractorStyle_H
#define TopoDataPickerInteractorStyle_H
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkPolyData.h>

namespace mb_system {

  /// Forward declaration
  class TopoDataItem;
  
    /** Catch mouse events */
    class TopoDataPickerInteractorStyle :
    public vtkInteractorStyleTrackballCamera {

    public:

      vtkTypeMacro(TopoDataPickerInteractorStyle,
		   vtkInteractorStyleTrackballCamera);

      /// Get a new PickerInteractorStyle object
      /// For use with vtkSmartPointer
      static TopoDataPickerInteractorStyle *New() {
	return new TopoDataPickerInteractorStyle();
      }

      /// Constructor
      TopoDataPickerInteractorStyle();

      /// Initialize - REQUIRED
      void initialize(mb_system::TopoDataItem *item,
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
      mb_system::TopoDataItem *item_;
  
      /// Associated interactor
      vtkRenderWindowInteractor *interactor_;

      /// Staring mouse position when left button down
      int startMousePos_[2];


      void testPoints(int x, int y, vtkRenderer *renderer);
    };
    
}

#endif

