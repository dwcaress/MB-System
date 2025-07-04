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
#include "InteractorStyle.h"

namespace mb_system {

  class DataSelectInteractorStyle : public vtkInteractorStyleRubberBandPick {
  public:
    static DataSelectInteractorStyle* New();
    vtkTypeMacro(DataSelectInteractorStyle, vtkInteractorStyleRubberBandPick);

    DataSelectInteractorStyle();

    void OnLeftButtonUp(void) override;

    void setPolyData(vtkPolyData *polyData);

  private:
    vtkSmartPointer<vtkPolyData> polyData_;
    vtkSmartPointer<vtkActor> selectedActor_;
    vtkSmartPointer<vtkDataSetMapper> selectedMapper_;  
  };
  
  vtkStandardNewMacro(DataSelectInteractorStyle);

  vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName);
} // namespace


#endif
