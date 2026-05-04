#ifndef DataSelectInteractorStyle_H
#define DataSelectInteractorStyle_H
#include <vtkActor.h>
#include <vtkAreaPicker.h>
#include <vtkDataSetMapper.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkInteractorStyleRubberBandPick.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPlanes.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkVersion.h>
#include <vtkPointData.h>
#include <vtkIdTypeArray.h>
#include "InteractorStyleIF.h"

namespace mb_system {

  class TopoDataItem;
  
  class DataSelectInteractorStyle :
    public vtkInteractorStyleRubberBandPick,
    public mb_system::InteractorStyleIF {

  public:
    /// static DataSelectInteractorStyle* New();
    vtkTypeMacro(DataSelectInteractorStyle, vtkInteractorStyleRubberBandPick);

    DataSelectInteractorStyle(TopoDataItem *item);

    const char *printHelp() override {
      return "r: toggle data select mode    R-drag: select data";
    }
    
    void OnLeftButtonUp(void) override;
    void OnMouseMove() override;

    
  protected:
    void RedrawRubberBand()
    {
        // Do nothing - this prevents the OpenGL state conflicts
        // The selection will still work, just without visual feedback
    }

    
  private:

    TopoDataItem *topoDataItem_;
    
  };
  
  vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName);
} // namespace


#endif
