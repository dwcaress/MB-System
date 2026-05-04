#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkSmartPointer.h>
#include "RestrictCameraStyle.h"

using namespace mb_system;

void RestrictCameraStyle::Rotate()     {
  vtkInteractorStyleTrackballCamera::Rotate();
  this->RestrictCamera();
}

void RestrictCameraStyle::Pan() 
{
  vtkInteractorStyleTrackballCamera::Pan();
  this->RestrictCamera();
}

void RestrictCameraStyle::Dolly() 
{
  vtkInteractorStyleTrackballCamera::Dolly();
  this->RestrictCamera();
}

void RestrictCameraStyle::Spin() 
{
  vtkInteractorStyleTrackballCamera::Spin();
  this->RestrictCamera();
}



void RestrictCameraStyle::RestrictCamera() {
        if (!this->CurrentRenderer)
            return;

        vtkCamera* camera = this->CurrentRenderer->GetActiveCamera();
        if (!camera)
            return;

        double pos[3];
        camera->GetPosition(pos);

        // Store initial position on first call
        if (this->PreviousZ == 0.0)
            this->PreviousZ = pos[2];

        // Only clamp if Z goes below minimum
        if (pos[2] < this->MinZ)
        {
            // Allow upward movement by clamping to MinZ, not PreviousZ
            pos[2] = this->MinZ;
            camera->SetPosition(pos);

            // Adjust focal point if needed
            double focal[3];
            camera->GetFocalPoint(focal);
            if (focal[2] >= pos[2])
            {
                focal[2] = pos[2] - 0.01;
                camera->SetFocalPoint(focal);
            }
        }
        
        // Update previous position
        this->PreviousZ = pos[2];
    }



vtkStandardNewMacro(RestrictCameraStyle);

