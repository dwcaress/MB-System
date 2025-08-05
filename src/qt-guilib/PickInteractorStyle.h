#ifndef PickInteractorStyle_H
#define PickInteractorStyle_H
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkPolyData.h>
#include "InteractorStyleIF.h"

namespace mb_system {

  /// Forward declaration
  class TopoDataItem;
  
  /** Catch mouse events */
  class PickInteractorStyle :
    public vtkInteractorStyleTrackballCamera,
    public mb_system::InteractorStyleIF {

  public:

    vtkTypeMacro(PickInteractorStyle,
		 vtkInteractorStyleTrackballCamera);

    /* ***
    /// Get a new PickerInteractorStyle object
    /// For use with vtkSmartPointer
    static PickInteractorStyle *New() {
      return new PickInteractorStyle();
    }
    *** */

    /// Constructor
    PickInteractorStyle(mb_system::TopoDataItem *item) {
      item_ = item;
    }

    /// Print help message describing mouse actions
    virtual const char *printHelp() {
      return "PickInteractorStyle help message goes here";
    }
    
    /// Pick cell
    virtual void OnLeftButtonDown() override;

    /// Pick cell
    virtual void OnLeftButtonUp() override;    


  protected:

    /// Staring mouse position when left button down
    int startMousePos_[2];

    void testPoints(int x, int y, vtkRenderer *renderer);

    mb_system::TopoDataItem *item_;
    
  };
    
}

#endif

