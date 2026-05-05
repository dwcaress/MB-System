#ifndef PointsSelectInteractorStyle_H
#define PointsSelectInteractorStyle_H
#include <vtkInteractorStyleRubberBandPick.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include "MyRubberBandStyle.h"

namespace mb_system {

// Edit modes
typedef enum {
  EraseMode,
  RestoreMode

} EditMode;
  
class TopoDataItem;

  /// User can select data points in rectangular 'rubber band' area, by
  /// left-dragging mouse.
  /// Subclass of MyRubberBandStyle for proper QtQuick integration of
  /// rubber band rectangle-draw during mouse drag.
  class PointsSelectInteractorStyle : public MyRubberBandStyle {
  public:
    static PointsSelectInteractorStyle* New();
    vtkTypeMacro(PointsSelectInteractorStyle, vtkInteractorStyleRubberBandPick);

    PointsSelectInteractorStyle() : MyRubberBandStyle()  {
      editMode_ = EditMode::EraseMode;
      selectedMapper_ = vtkSmartPointer<vtkDataSetMapper>::New();
      selectedActor_ = vtkSmartPointer<vtkActor>::New();
      selectedActor_->SetMapper(selectedMapper_);
      topoDataItem_ = nullptr;
    }

    virtual void OnLeftButtonUp() override;

    /// Set associated TopoDataItem
    void setTopoDataItem(TopoDataItem *item);

  protected:

    EditMode editMode_;
    
    TopoDataItem *topoDataItem_;
    vtkSmartPointer<vtkActor> selectedActor_;
    vtkSmartPointer<vtkDataSetMapper> selectedMapper_;
  };
}   // namespace

#endif

