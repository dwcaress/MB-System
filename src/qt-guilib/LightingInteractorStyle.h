#ifndef LightingInteractorStyle_H
#define LightingInteractorStyle_H
#include <vtkActor.h>
#include <vtkCubeSource.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLight.h>
#include <vtkMath.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkCamera.h>
#include "InteractorStyleIF.h"

namespace mb_system {

  /// Forward declaration
  class TopoDataItem;
  
  /** Custom interactor style for light position control */
  class LightingInteractorStyle :
    public vtkInteractorStyleTrackballCamera,
    public mb_system::InteractorStyleIF {

  public:

    /// RTTI support
    vtkTypeMacro(LightingInteractorStyle, vtkInteractorStyleTrackballCamera);

    LightingInteractorStyle(TopoDataItem *item);

    /// Print help message
    const char *printHelp() override {
      return "shift-L-drag: change light position  shift-R-drag: change light intensity";
    }
  
    virtual void OnLeftButtonDown() override;

    virtual void OnLeftButtonUp() override;

    virtual void OnRightButtonDown() override;

    virtual void OnRightButtonUp() override;
  
    virtual void OnMouseMove() override;

  private:

    mb_system::TopoDataItem *topoDataItem_;
    
    bool lightMoving_;
    bool intensityChanging_;  
    int startMousePosition_[2];
  };
}    // namespace

#endif
