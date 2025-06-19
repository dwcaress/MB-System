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
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkCamera.h>
#include "InteractorStyle.h"

/** Custom interactor style for light position control */
class LightingInteractorStyle : public mb_system::InteractorStyle
{
public:
    static LightingInteractorStyle* New();
    vtkTypeMacro(LightingInteractorStyle, vtkInteractorStyleTrackballCamera);

  LightingInteractorStyle();

  void setRenderer(vtkRenderer* renderer);

  virtual void OnLeftButtonDown() override;

  virtual void OnLeftButtonUp() override;

  virtual void OnRightButtonDown() override;

  virtual void OnRightButtonUp() override;
  
  virtual void OnMouseMove() override;

private:

  vtkRenderer* renderer_;
  bool lightMoving_;
  bool intensityChanging_;  
  int startMousePosition_[2];
};

#endif
