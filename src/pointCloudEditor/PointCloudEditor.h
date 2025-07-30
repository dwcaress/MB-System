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

#define VTKISRBP_ORIENT 0
#define VTKISRBP_SELECT 1

#define ORIGINAL_IDS "originalIds"

// Quality flag values
#define DATA_QUALITY_NAME "dataQuality"
#define GOOD 1
#define BAD 0


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
  void buildGui();

  /// Read point cloud data from specified file
  bool readPolyData(const char* fileName);

  /// Set vertical exaggeration value - must be positive number else
  /// return false
  bool setVerticalExagg(double value) {
    verticalExagg_ = value;
  }

protected:
  vtkNew<vtkAreaPicker> areaPicker_;
  vtkNew<vtkRenderWindow> renderWindow_;
  vtkNew<vtkDataSetSurfaceFilter> surfaceFilter_;
  vtkNew<vtkLookupTable> lut_;
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
  
  // data quality array for input vtkPolyData
  vtkSmartPointer<vtkIntArray> quality_ =
    vtkSmartPointer<vtkIntArray>::New();

  double verticalExagg_;
};



#endif
