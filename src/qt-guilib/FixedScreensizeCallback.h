#ifndef FixedScreensizeCallback_h
#define FixedScreensizeCallback_h
#include <vtkCommand.h>
#include <vtkActor.h>
#include <vtkRenderer.h>

namespace mb_system {

  /** Scale specific vtkActor such that it always has specified size in
      device coordinates. Useful for actors that specify size in world coordinates. */
  
  class FixedScreensizeCallback : public vtkCommand {

  public:
    
    static FixedScreensizeCallback * New() {
      return new FixedScreensizeCallback();
    }

    void setRenderer(vtkRenderer *renderer) {
      renderer_ = renderer;
    }

    void setActorPixelSize(int pixelSize) {
      pixelSize_ = pixelSize;
    }
    
    void setActor(vtkActor *actor) {
      actor_ = actor;
    }


    /// Scale specified actor such that actor has specified fixed
    /// size in pixels.
    void Execute(vtkObject* caller, unsigned long, void*) override;
    
  protected:

    vtkActor *actor_;
    int pixelSize_;
    vtkRenderer *renderer_;
    
    
  };

}


#endif
