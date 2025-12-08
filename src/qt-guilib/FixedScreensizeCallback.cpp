#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <qtLogging>
#include <qDebug>
#include "FixedScreensizeCallback.h"

using namespace mb_system;

void FixedScreensizeCallback::Execute(vtkObject* caller, unsigned long, void*) {
  // The actor we want to scale is stored in this->Actor
  // NOT the caller (which is the renderer)
  if (!actor_ || !renderer_ || !pixelSize_) {
    qWarning() << "actor, renderer and/or pixelSize not specified";
    return;
  }

  vtkCamera* camera = renderer_->GetActiveCamera();
        
  // Get THIS specific actor's position
  double* pos = actor_->GetPosition();
  double* camPos = camera->GetPosition();

  // Calculate distance from camera to THIS actor
  double dx = pos[0] - camPos[0];
  double dy = pos[1] - camPos[1];
  double dz = pos[2] - camPos[2];
  double distance = sqrt(dx*dx + dy*dy + dz*dz);

  // Get viewport size
  int* size = renderer_->GetRenderWindow()->GetSize();
  double viewAngle = camera->GetViewAngle();

  // Calculate world size for desired pixel size
  double worldHeight = 2.0 * distance * tan(viewAngle * 3.14159 / 360.0);
  double pixelWorldSize = worldHeight / size[1];
  double scale = pixelSize_ * pixelWorldSize;

  // Scale THIS specific actor only
  actor_->SetScale(scale, scale, scale);
}
