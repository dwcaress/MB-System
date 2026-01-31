#ifndef RestrictCameraStyle_h
#define RestrictCameraStyle_h

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

namespace mb_system {
// Custom interactor style that restricts camera to positive Z
class RestrictCameraStyle : public vtkInteractorStyleTrackballCamera {
public:
    static RestrictCameraStyle * New();
    vtkTypeMacro(RestrictCameraStyle, vtkInteractorStyleTrackballCamera);

    void SetMinimumZ(double z) { this->MinZ = z; }
    double GetMinimumZ() const { return this->MinZ; }

    // Override camera manipulation methods
  virtual void Rotate() override;

  virtual void Pan() override;

  virtual void Dolly() override;

  virtual void Spin() override;

protected:
    RestrictCameraStyle() : MinZ(0.1) {}
    ~RestrictCameraStyle() override = default;

  void RestrictCamera();


private:
    double MinZ;
  double PreviousZ;
    RestrictCameraStyle(const RestrictCameraStyle&) = delete;
    void operator=(const RestrictCameraStyle&) = delete;
};

}   // end namespace


#endif
