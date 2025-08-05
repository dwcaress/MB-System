#include <vtkActor.h>
#include <vtkAreaPicker.h>
#include <vtkDataSetMapper.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkExtractPolyDataGeometry.h>
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
#include <vtkTextActor.h>
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
#include "PointCloudEditor.h"

#define ORIGINAL_IDS "originalIds"

// Quality flag values
#define DATA_QUALITY_NAME "dataQuality"
#define GOOD 1
#define BAD 0


PointCloudEditor::PointCloudEditor(void) {

  verticalExagg_ = 1.;

  firstRender_ = true;
  
  // Build color lookup table
  qualityLUT_->SetNumberOfTableValues(2);
  qualityLUT_->SetRange(0, 1);
  
  qualityLUT_->SetTableValue(BAD, 1.0, 0.0, 0.0, 1.0);
  qualityLUT_->SetTableValue(GOOD, 0.0, 1.0, 0.0, 1.0);  
  qualityLUT_->Build();

  style_->setEditor(this);
  editModeGroup_->setEditor(this);
}


/// Visualize the point cloud
void PointCloudEditor::visualize(void) {

  std::cerr << "visualize()\n";
    
  // Set vertical exaggeration
  float zScale = verticalExagg_;
  scaleTransform_->Scale(1., 1., zScale);
  scaleTransformFilter_->SetTransform(scaleTransform_);
  scaleTransformFilter_->SetInputData(polyData_);

  mapper_->SetInputData(polyData_);  // before zscale filter immplementation
    
  /// mapper_->SetInputConnection(scaleTransformFilter_->GetOutputPort());

  // Configure mapper to use LUT stuff
  mapper_->SetLookupTable(qualityLUT_);

  polyData_->GetPointData()->SetActiveScalars(DATA_QUALITY_NAME);
    
  mapper_->SetScalarModeToUsePointData();
  mapper_->SetColorModeToMapScalars();
  mapper_->SetScalarRange(0, 1);

  actor_->GetProperty()->SetPointSize(5);
  actor_->SetMapper(mapper_);
  actor_->SetScale(1., 1., zScale);

  renderer_->UseHiddenLineRemovalOn();

  renderWindow_->AddRenderer(renderer_);
  ///renderWindow_->SetSize(640, 480);
  renderWindow_->SetSize(1000, 1000);  
  renderWindow_->SetWindowName("HighlightSelection");

  renderWindowInteractor_->SetPicker(areaPicker_);
  renderWindowInteractor_->SetRenderWindow(renderWindow_);

  renderer_->AddActor(actor_);
  renderer_->SetBackground(colors_->GetColor3d("Tan").GetData());

  renderWindow_->Render();

  if (firstRender_) {
    // Build GUI elements
    buildWidgets();
    firstRender_ = false;
  }

  /// actor_->SetScale(1.0, 1.0, verticalExagg_);

  renderWindowInteractor_->SetInteractorStyle(style_);

  renderWindowInteractor_->Start();

  // Apply scale transform to camera too
  vtkCamera *camera = renderer_->GetActiveCamera();
  camera->SetModelTransformMatrix(scaleTransform_->GetMatrix());
}

/// Instantiate GUI elements; note that GUI elements should be class members so
/// they aren't garbage-collected outside of this functions' scope.
void PointCloudEditor::buildWidgets() {
  std::cerr << "**** buildWidgets()\n";

  sliderRep_->SetMinimumValue(1.0);
  sliderRep_->SetMaximumValue(20.0);
  sliderRep_->SetValue(verticalExagg_);
  sliderRep_->SetTitleText("vertical exaggeration");

  // Change the color of the knob that slides
  sliderRep_->GetSliderProperty()->SetColor(colors_->
					    GetColor3d("Green").GetData());

  // Change the color of the text indicating what the slider controls
  sliderRep_->GetTitleProperty()->SetColor(colors_->
					   GetColor3d("AliceBlue").GetData());

  // Change the color of the text displaying the value
  sliderRep_->GetLabelProperty()->SetColor(colors_->
					   GetColor3d("AliceBlue").GetData());

  // Change the color of the knob when the mouse is held on it
  sliderRep_->GetSelectedProperty()->SetColor(colors_->
					      GetColor3d("DeepPink").GetData());

  // Change the color of the bar
  sliderRep_->GetTubeProperty()->SetColor(colors_->
					  GetColor3d("MistyRose").GetData());

  // Change the color of the ends of the bar
  sliderRep_->GetCapProperty()->
    SetColor(colors_->GetColor3d("Yellow").GetData());

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
  sliderWidget_->AddObserver(vtkCommand::EndInteractionEvent,
			     callback);


  // 4. Create textures for button states (on and off).

  // (Assuming you have functions like CreateButtonOn and CreateButtonOff to
  // generate textures) For simplicity, we'll create simple filled squares as
  // textures. You would typically load image files for your desired radio
  // button appearance.
  std::array<std::string, 3> onColors = {"Gray", "Gray", "Gray"};
  std::array<std::string, 3> offColors = {"Silver", "Silver", "Silver"};
  std::array<vtkNew<vtkImageData>, 3> onImages;
  std::array<vtkNew<vtkImageData>, 3> offImages;

  editModeGroup_->setInteractor(renderWindowInteractor_);
  editModeGroup_->setActor(actor_);
  
  const int nRadioButtons = 2;  
  for (int i = 0; i < nRadioButtons; ++i) {
    onImages[i] = createColorImage(onColors[i]);
    offImages[i] = createColorImage(offColors[i]);
  }
  // 5. Create multiple vtkButtonWidget instances.
  std::vector<vtkSmartPointer<vtkButtonWidget>> radioButtons;

  for (int i = 0; i < nRadioButtons; ++i) {
    vtkNew<vtkTexturedButtonRepresentation2D> buttonRepresentation;
    vtkSmartPointer<vtkTextActor> textActor = vtkSmartPointer<vtkTextActor>::New();
    if (i == 0) {
      textActor->SetInput("ERASE");
    }
    else if (i == 1) {
      textActor->SetInput("RESTORE");
    }

    textActor->GetTextProperty()->SetColor(0., 0., 0.);
    
    buttonRepresentation->SetNumberOfStates(2); // Two states: on and off.
    buttonRepresentation->SetButtonTexture(0, offImages[i]); // State 0: off.
    buttonRepresentation->SetButtonTexture(1, onImages[i]);  // State 1: on.
    
    // Place the buttons in the scene.
    // std::array<double, 6> bounds{0.0, 0.0, 0.0, 100.0, 0.0, 0.0};
    std::array<double, 6> bounds{0.0, 0.0, 100.0, 250.0, 0.0, 0.0};    
    bounds[0] = 215.0 + i * 60.0; // Adjust for spacing.
    bounds[1] = bounds[0] + 50.0;
    buttonRepresentation->PlaceWidget(bounds.data());
    textActor->SetDisplayPosition(bounds[0]+12, bounds[2]+20);
    renderer_->AddActor2D(textActor);
    
    vtkNew<vtkButtonWidget> buttonWidget;
    buttonWidget->SetInteractor(renderWindowInteractor_);
    buttonWidget->SetRepresentation(buttonRepresentation);
    buttonWidget->EnabledOn(); // Enable the button.

    editModeGroup_->addButton(buttonWidget);
    buttonWidget->AddObserver(vtkCommand::StateChangedEvent, editModeGroup_);
    radioButtons.push_back(buttonWidget);
  }

}

  
bool PointCloudEditor::readPolyData(const char* fileName) {
    
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
  

vtkNew<vtkImageData> PointCloudEditor::createColorImage(std::string
							const& color) {
  vtkNew<vtkNamedColors> colors;

  std::array<unsigned char, 3> dc{0, 0, 0};
  auto c = colors->GetColor3ub(color).GetData();
  for (auto i = 0; i < 3; ++i)
  {
    dc[i] = c[i];
  }

  vtkNew<vtkImageData> image;
  // Specify the size of the image data.
  image->SetDimensions(10, 10, 1);
  image->AllocateScalars(VTK_UNSIGNED_CHAR, 3);

  auto dims = image->GetDimensions();

  // Fill the image with color.
  for (int y = 0; y < dims[1]; y++)
  {
    for (int x = 0; x < dims[0]; x++)
    {
      unsigned char* pixel =
          static_cast<unsigned char*>(image->GetScalarPointer(x, y, 0));
      for (int i = 0; i < 3; ++i)
      {
        pixel[i] = dc[i];
      }
    }
  }
  return image;
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

