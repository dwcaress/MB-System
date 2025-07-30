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
#include "Utilities.h"
#include "TopoDataReader.h"

#define VTKISRBP_ORIENT 0
#define VTKISRBP_SELECT 1

#define ORIGINAL_IDS "originalIds"

// Quality flag values
#define DATA_QUALITY_NAME "dataQuality"
#define GOOD 1
#define BAD 0


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

vtkStandardNewMacro(PointsSelectInteractorStyle);


class PointCloudEditor {

public:

  PointCloudEditor(void) {

    verticalExagg_ = 1.;
    
    // Build color lookup table
    lut_->SetNumberOfTableValues(2);
    lut_->SetRange(0, 1);
    lut_->SetTableValue(0, 1.0, 0.0, 0.0, 1.0);
    lut_->SetTableValue(1, 0.0, 1.0, 0.0, 1.0);  
    lut_->Build();

    style_->setEditor(this);
  }


  /// Get point cloud vtkPolyData
  vtkPolyData *polyData() {
    return polyData_;
  }

  /// Get interactor
  vtkRenderWindowInteractor *interactor() {
    return renderWindowInteractor_;
  }
  
  /// Visualize the point cloud
  void visualize(void) {

    std::cerr << "visualize()\n";
    
    // Set vertical exaggeration
    float zScale = verticalExagg_;
    scaleTransform_->Scale(1., 1., zScale);
    scaleTransformFilter_->SetTransform(scaleTransform_);
    scaleTransformFilter_->SetInputData(polyData_);

    mapper_->SetInputData(polyData_);  // before zscale filter immplementation
    
    /// mapper_->SetInputConnection(scaleTransformFilter_->GetOutputPort());

    // Configure mapper to use LUT stuff
    mapper_->SetLookupTable(lut_);

    polyData_->GetPointData()->SetActiveScalars(DATA_QUALITY_NAME);
    
    mapper_->SetScalarModeToUsePointData();
    mapper_->SetColorModeToMapScalars();
    mapper_->SetScalarRange(0, 1);

    actor_->GetProperty()->SetPointSize(5);
    actor_->SetMapper(mapper_);
    actor_->SetScale(1., 1., zScale);

    renderer_->UseHiddenLineRemovalOn();

    renderWindow_->AddRenderer(renderer_);
    renderWindow_->SetSize(640, 480);
    renderWindow_->SetWindowName("HighlightSelection");

    renderWindowInteractor_->SetPicker(areaPicker_);
    renderWindowInteractor_->SetRenderWindow(renderWindow_);

    renderer_->AddActor(actor_);
    renderer_->SetBackground(colors_->GetColor3d("Tan").GetData());

    renderWindow_->Render();

    // Build GUI elements
    buildGui();

    /// actor_->SetScale(1.0, 1.0, verticalExagg_);

    renderWindowInteractor_->SetInteractorStyle(style_);

    renderWindowInteractor_->Start();

    // Apply scale transform to camera too
    vtkCamera *camera = renderer_->GetActiveCamera();
    camera->SetModelTransformMatrix(scaleTransform_->GetMatrix());
  }

  /// Instantiate GUI elements; note that GUI elements should be class members so
  /// they aren't garbage-collected outside of this functions' scope.
  void buildGui() {
    std::cerr << "**** buildGui()\n";

    sliderRep_->SetMinimumValue(1.0);
    sliderRep_->SetMaximumValue(20.0);
    sliderRep_->SetValue(verticalExagg_);
    sliderRep_->SetTitleText("vertical exaggeration");

    // Change the color of the knob that slides
    sliderRep_->GetSliderProperty()->SetColor(
					     colors_->GetColor3d("Green").GetData());
    // Change the color of the text indicating what the slider controls
    sliderRep_->GetTitleProperty()->SetColor(
					    colors_->GetColor3d("AliceBlue").GetData());
    // Change the color of the text displaying the value
    sliderRep_->GetLabelProperty()->SetColor(
					    colors_->GetColor3d("AliceBlue").GetData());
    // Change the color of the knob when the mouse is held on it
    sliderRep_->GetSelectedProperty()->SetColor(
					       colors_->GetColor3d("DeepPink").GetData());
    // Change the color of the bar
    sliderRep_->GetTubeProperty()->SetColor(
					   colors_->GetColor3d("MistyRose").GetData());
    // Change the color of the ends of the bar
    sliderRep_->GetCapProperty()->SetColor(colors_->GetColor3d("Yellow").GetData());

    sliderRep_->SetSliderLength(0.05);
    sliderRep_->SetSliderWidth(0.025);
    sliderRep_->SetEndCapLength(0.02);

    sliderRep_->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
    sliderRep_->GetPoint1Coordinate()->SetValue(0.2, 0.1);
    sliderRep_->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
    sliderRep_->GetPoint2Coordinate()->SetValue(0.8, 0.1);

    /// vtkNew<vtkSliderWidget> sliderWidget;
    sliderWidget_->SetInteractor(renderWindowInteractor_);
    sliderWidget_->SetRepresentation(sliderRep_);
    sliderWidget_->SetAnimationModeToAnimate();
    sliderWidget_->EnabledOn();

    ZScaleCallback *callback = new ZScaleCallback(this);
    sliderWidget_->AddObserver(vtkCommand::EndInteractionEvent, callback);
  }

  
  bool readPolyData(const char* fileName) {
    
    std::string extension =
      vtksys::SystemTools::GetFilenameLastExtension(std::string(fileName));

    if (extension == ".grd" || extension == ".mb88") {
      std::cerr << "read mb-system topo data\n";
      reader_->SetFileName(fileName);
      reader_->Update();
      polyData_ = reader_->GetOutput();
    }
    else {
      std::cerr << "unknown input file format: " << fileName << "\n";
      std::cerr << "read cube source\n";
      vtkNew<vtkCubeSource> source;
      //      source->SetPhiResolution(21);
      // source->SetThetaResolution(40);
      source->Update();
      polyData_ = source->GetOutput();
    }
    
    // Associate id's with original polyData
    idFilter_->SetInputData(polyData_);

    // Specify name by which to retrieve id's
    idFilter_->SetCellIdsArrayName(ORIGINAL_IDS);
    idFilter_->SetPointIdsArrayName(ORIGINAL_IDS);

    idFilter_->Update();    
    surfaceFilter_->SetInputConnection(idFilter_->GetOutputPort());
    surfaceFilter_->Update();
    polyData_ = surfaceFilter_->GetOutput();
    
    quality_->SetName(DATA_QUALITY_NAME);
    quality_->SetNumberOfTuples(polyData_->GetNumberOfPoints());
    
    // First assume all points are good
    for (int i = 0; i < polyData_->GetNumberOfPoints(); i++) {
      quality_->SetValue(i, GOOD);
    }

    // Associate quality array with original poly data
    polyData_->GetPointData()->AddArray(quality_);    

    return true;
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


void PointsSelectInteractorStyle::OnLeftButtonUp() {
  // Forward events
  vtkInteractorStyleRubberBandPick::OnLeftButtonUp();
  if (CurrentMode == VTKISRBP_SELECT) {
    vtkNew<vtkNamedColors> colors;

    vtkPlanes* frustum =
      static_cast<vtkAreaPicker*>(GetInteractor()->GetPicker())
      ->GetFrustum();

    vtkNew<vtkExtractPolyDataGeometry> extractor;
    extractor->SetInputData(editor_->polyData());

    // Extract cells that lie within the user-specified frustrum
    extractor->SetImplicitFunction(frustum);
    extractor->ExtractInsideOn();  // T.O'R.
    // Extract the cells
    extractor->Update();

    vtkPolyData *extractedData = extractor->GetOutput();
      
    std::cerr << "Extracted "
	      << extractedData->GetNumberOfCells()
	      << " cells." << std::endl;

    // Set mapper input to extracted cells
    // (Color is not controlled by scalar)
    selectedMapper_->SetInputData(extractedData);
    selectedMapper_->ScalarVisibilityOff();

    selectedActor_->GetProperty()->SetColor(
					    colors->GetColor3d("Red").GetData());
      
    selectedActor_->GetProperty()->SetPointSize(5);
    // selectedActor_->GetProperty()->SetRepresentationToWireframe();
    // selectedActor_->GetProperty()->SetRepresentationToSurface();
    selectedActor_->GetProperty()->SetRepresentationToPoints();

    GetInteractor()
      ->GetRenderWindow()
      ->GetRenderers()
      ->GetFirstRenderer()
      ->AddActor(selectedActor_);
      
    GetInteractor()->GetRenderWindow()->Render();

    // Highlight the selected area
    HighlightProp(nullptr);

    // Get the points
    vtkPoints *points = extractedData->GetPoints();
    std::cerr << "Got " << points->GetNumberOfPoints() << " points\n";
    for (int i = 0; i < points->GetNumberOfPoints(); i++) {
      double *point = points->GetPoint(i);
      std::cerr << "pt " << i << ": "
		<< point[0] << ", " << point[1] << ", " << point[2] << "\n";
    }


    // Get subsetted original point IDs
    vtkIdTypeArray *filteredPointIds =
      vtkIdTypeArray::SafeDownCast(extractedData->GetPointData()->
				   GetArray(ORIGINAL_IDS));

    if (filteredPointIds) {
      std::cerr << "Got original point IDs\n";
      vtkPoints *points = editor_->polyData()->GetPoints();

      vtkIntArray* quality =
	vtkIntArray::SafeDownCast(editor_->polyData()->GetPointData()->
				  GetArray(DATA_QUALITY_NAME));
      if (!quality) {
	std::cerr << "Couldn't find " << DATA_QUALITY_NAME << "!!\n";
      }
      else {
	std::cerr << "FOUND " << DATA_QUALITY_NAME << "!!\n";	  
      }
	
      for (int i = 0; i < extractedData->GetNumberOfPoints(); i++) {
	vtkIdType pointId = filteredPointIds->GetValue(i);
	std::cerr << "subset point " << i << " -> original point " <<
	  pointId << "\n";
	double xyz[3];
	points->GetPoint(pointId, xyz);
	std::cerr << "x: " << xyz[0] << ", y: " << xyz[1] <<
	  ", z: " << xyz[2] << "\n";

	// Set selected point data quality to BAD
	if (quality) {
	  std::cerr << "set value to BAD\n";
	  quality->SetValue(pointId, BAD);
	}

      }
    }
    else {
      std::cerr << "Couldn't get original point Ids\n";
    }
  }

  std::cerr << "Visualize data\n";
  editor_->visualize();
}


int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "usage: " << argv[0] << " <swath-or-gridFile>\n";
    return 1;
  }

  PointCloudEditor *editor = new PointCloudEditor();

  if (!editor->readPolyData(argv[1])) {
    std::cerr << "Couldn't process " << argv[1] << "\n";
    return 1;
  }

  




  editor->visualize();

  return 0;
}


