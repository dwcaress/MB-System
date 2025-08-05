#ifndef PointCloudEditor_H
#define PointCloudEditor_H
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
#include <vtkCubeSource.h>
#include <vtkVersion.h>
#include <vtkPointData.h>
#include <vtkIdTypeArray.h>
#include <vtkProperty2D.h>
#include <vtkIntArray.h>
#include <vtkIdFilter.h>
#include <vtkLookupTable.h>
#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtksys/SystemTools.hxx>
#include <vtkActor2D.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkSliderWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkCamera.h>
#include "ZScaleCallback.h"
#include "PointsSelectInteractorStyle.h"
#include "Utilities.h"
#include "TopoDataReader.h"
#include "RadioButtonGroup.h"

#define VTKISRBP_ORIENT 0
#define VTKISRBP_SELECT 1

#define ORIGINAL_IDS "originalIds"

// Quality flag values
#define DATA_QUALITY_NAME "dataQuality"
#define GOOD 1
#define BAD 0

// Edit modes
typedef enum {
  EraseMode,
  RestoreMode

} EditMode;


class EditModeGroup;

class PointCloudEditor {

public:

  PointCloudEditor(void);

  /// Get point cloud vtkPolyData
  vtkPolyData *polyData() {
    return polyData_;
  }

  /// Get interactor
  vtkRenderWindowInteractor *interactor() {
    return renderWindowInteractor_;
  }
  
  /// Visualize the point cloud
  void visualize(void);

  /// Instantiate GUI elements; note that GUI elements should be class
  /// members so they aren't garbage-collected outside of this
  /// functions' scope.
  void buildWidgets();

  /// Read point cloud data from specified file
  bool readPolyData(const char* fileName);

  /// Set vertical exaggeration value - must be positive number else
  /// return false
  bool setVerticalExagg(double value) {
    verticalExagg_ = value;
  }

  /// Create a color image
  static vtkNew<vtkImageData> createColorImage(std::string const& color);

  // Set edit mode
  void setEditMode(EditMode mode) {
    editMode_ = mode;
  }

  // Get edit mode
  EditMode getEditMode() {
    return editMode_;
  }

  
protected:
  vtkNew<vtkAreaPicker> areaPicker_;
  vtkNew<vtkRenderWindow> renderWindow_;
  vtkNew<vtkDataSetSurfaceFilter> surfaceFilter_;
  vtkNew<vtkLookupTable> qualityLUT_;
  vtkNew<vtkIdFilter> idFilter_;  
  vtkNew<vtkNamedColors> colors_;
  vtkNew<vtkPolyDataMapper> mapper_;
  vtkNew<vtkActor> actor_;  
  vtkNew<vtkRenderer> renderer_;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor_;
  vtkNew<vtkTransform> scaleTransform_;
  vtkNew<vtkTransformFilter> scaleTransformFilter_;
  vtkNew<PointsSelectInteractorStyle> style_;
  vtkNew<mb_system::TopoDataReader> reader_;
  vtkSmartPointer<vtkPolyData> polyData_;
  
  vtkNew<vtkSliderRepresentation2D> sliderRep_;
  vtkNew<vtkSliderWidget> sliderWidget_;
  vtkNew<EditModeGroup> editModeGroup_;
  
  // data quality array for input vtkPolyData
  vtkSmartPointer<vtkIntArray> quality_ =
    vtkSmartPointer<vtkIntArray>::New();

  EditMode editMode_;
  double verticalExagg_;

  bool firstRender_;
  
};


class EditModeGroup : public mb_system::RadioButtonGroup {
public:
  static EditModeGroup* New() { return new EditModeGroup; }  

  void setEditor(PointCloudEditor *editor) {
    editor_ = editor;
  }
  
  bool processAction(int selectedIndex) override {
    if (selectedIndex == 0) {
      editor_->setEditMode(EraseMode);
    }
    else {
      editor_->setEditMode(RestoreMode);
    }
  }
  
  protected:
    PointCloudEditor *editor_;
};

  




#endif
