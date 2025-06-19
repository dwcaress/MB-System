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
#include "LightingInteractorStyle.h"

vtkStandardNewMacro(LightingInteractorStyle);

LightingInteractorStyle::LightingInteractorStyle() {
  this->lightMoving_ = false;
  this->intensityChanging_ = false;  
  this->startMousePosition_[0] = 0;
  this->startMousePosition_[1] = 0;
}

void LightingInteractorStyle::setRenderer(vtkRenderer* renderer) {
  this->renderer_ = renderer;
}

void LightingInteractorStyle::OnLeftButtonDown() {
  std::cerr << "LightingInteractorStyle::OnLeftButtonDown()\n";
  if (this->Interactor->GetShiftKey()) {
    std::cerr << "start moving the light!\n";
    this->lightMoving_ = true;
    this->Interactor->GetEventPosition(this->startMousePosition_[0],
				       this->startMousePosition_[1]);
  }
  else {
    std::cerr << "Do not move the light\n";
  }
  vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

void LightingInteractorStyle::OnLeftButtonUp() {
  this->lightMoving_ = false;
  vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
}


void LightingInteractorStyle::OnRightButtonDown() {
  std::cerr << "LightingInteractorStyle::OnRightButtonDown()\n";
  if (this->Interactor->GetShiftKey()) {
    std::cerr << "start changing intensity!\n";
    this->intensityChanging_ = true;
    this->Interactor->GetEventPosition(this->startMousePosition_[0],
				       this->startMousePosition_[1]);
  }
  else {
    std::cerr << "Do not move the light\n";
  }
  vtkInteractorStyleTrackballCamera::OnRightButtonDown();
}

void LightingInteractorStyle::OnRightButtonUp() {
  this->intensityChanging_ = false;
  vtkInteractorStyleTrackballCamera::OnRightButtonUp();
}

void LightingInteractorStyle::OnMouseMove() {
  if (this->lightMoving_ && this->Interactor->GetShiftKey())  {
    // Get current mouse position
    int position[2];
    this->Interactor->GetEventPosition(position[0], position[1]);
            
    // Calculate change from previous position
    double dx = position[0] - this->startMousePosition_[0];
    double dy = position[1] - this->startMousePosition_[1];
            
    // Get current camera parameters to relate mouse movement to 3D space
    vtkCamera* camera = this->renderer_->GetActiveCamera();
            
    // Get camera vectors
    double forward[3];
    camera->GetDirectionOfProjection(forward);
            
    double up[3];
    camera->GetViewUp(up);
            
    double right[3];
    vtkMath::Cross(forward, up, right);
    vtkMath::Normalize(right);
            
    // Scale movement based on renderer size
    int* size = this->Interactor->GetRenderWindow()->GetSize();
    dx = dx / size[0] * 5.0;
    dy = dy / size[1] * 5.0;
            
    // Get current light position
    double current_pos[3];
    this->lightSource_->GetPosition(current_pos);
            
    // Calculate new position
    double new_pos[3] = {
      current_pos[0] + dx * right[0] - dy * up[0],
      current_pos[1] + dx * right[1] - dy * up[1],
      current_pos[2] + dx * right[2] - dy * up[2]
    };

    std::cerr << "set light position:  x= " << new_pos[0] <<
      ", y=: " << new_pos[1] <<
      ", z= " << new_pos[2] << "\n";
      
    // Update light position
    this->lightSource_->SetPosition(new_pos);

    // Store new position as starting position for next move
    this->startMousePosition_[0] = position[0];
    this->startMousePosition_[1] = position[1];
            
    // Trigger render to update scene with new lighting
    this->Interactor->Render();
  }
  else {
    // Pass the event to the parent class for standard camera manipulation
    vtkInteractorStyleTrackballCamera::OnMouseMove();
  }
        
}



