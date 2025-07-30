#ifndef PointsSelectInteractorStyle_H
#define PointsSelectInteractorStyle_H
#include <vtkInteractorStyleRubberBandPick.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>

class PointCloudEditor;


// Define interaction style
class PointsSelectInteractorStyle : public vtkInteractorStyleRubberBandPick {
public:
  static PointsSelectInteractorStyle* New();
  vtkTypeMacro(PointsSelectInteractorStyle, vtkInteractorStyleRubberBandPick);

  PointsSelectInteractorStyle() : vtkInteractorStyleRubberBandPick()  {
    selectedMapper_ = vtkSmartPointer<vtkDataSetMapper>::New();
    selectedActor_ = vtkSmartPointer<vtkActor>::New();
    selectedActor_->SetMapper(selectedMapper_);
  }

  virtual void OnLeftButtonUp() override;

  void setEditor(PointCloudEditor *editor) {
    editor_ = editor;
  }

private:
  
  PointCloudEditor *editor_;
  vtkSmartPointer<vtkActor> selectedActor_;
  vtkSmartPointer<vtkDataSetMapper> selectedMapper_;

  
  /// vtkSmartPointer<vtkPolyData> readPolyData(const char* fileName);

};

#endif

