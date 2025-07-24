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

#include "Utilities.h"
#include "TopoDataReader.h"

#define VTKISRBP_ORIENT 0
#define VTKISRBP_SELECT 1

#define ORIGINAL_IDS "originalIds"

// Quality flag values
#define DATA_QUALITY_NAME "dataQuality"
#define GOOD 1
#define BAD 0

// Define interaction style
class HighlightInteractorStyle : public vtkInteractorStyleRubberBandPick
{
public:
  static HighlightInteractorStyle* New();
  vtkTypeMacro(HighlightInteractorStyle, vtkInteractorStyleRubberBandPick);

  HighlightInteractorStyle() : vtkInteractorStyleRubberBandPick()
  {
    selectedMapper_ = vtkSmartPointer<vtkDataSetMapper>::New();
    selectedActor_ = vtkSmartPointer<vtkActor>::New();
    selectedActor_->SetMapper(selectedMapper_);
  }

  virtual void OnLeftButtonUp() override
  {
    // Forward events
    vtkInteractorStyleRubberBandPick::OnLeftButtonUp();

    if (CurrentMode == VTKISRBP_SELECT)
    {
      vtkNew<vtkNamedColors> colors;

      vtkPlanes* frustum =
          static_cast<vtkAreaPicker*>(GetInteractor()->GetPicker())
              ->GetFrustum();

      vtkNew<vtkExtractPolyDataGeometry> extractor;
      extractor->SetInputData(polyData_);

      // Extract cells that lie within the user-specified frustrum
      extractor->SetImplicitFunction(frustum);
      extractor->ExtractInsideOn();  // T.O'R.
      // Extract the cells
      extractor->Update();

      vtkPolyData *extractedData = extractor->GetOutput();
      
      std::cout << "Extracted "
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
      std::cout << "Got " << points->GetNumberOfPoints() << " points\n";
      for (int i = 0; i < points->GetNumberOfPoints(); i++) {
	double *point = points->GetPoint(i);
	std::cout << "pt " << i << ": "
		  << point[0] << ", " << point[1] << ", " << point[2] << "\n";
      }


      // Get subsetted original point IDs
      vtkIdTypeArray *filteredPointIds =
	vtkIdTypeArray::SafeDownCast(extractedData->GetPointData()->
				     GetArray(ORIGINAL_IDS));

      if (filteredPointIds) {
	std::cout << "Got original point IDs\n";
	vtkPoints *points = polyData_->GetPoints();
	vtkIntArray* quality =
	  vtkIntArray::SafeDownCast(polyData_->GetPointData()->
				    GetArray(DATA_QUALITY_NAME));

	if (!quality) {
	  std::cerr << "Couldn't find " << DATA_QUALITY_NAME << "!!\n";
	}
	else {
	  std::cerr << "FOUND " << DATA_QUALITY_NAME << "!!\n";	  
	}
	
	for (int i = 0; i < extractedData->GetNumberOfPoints(); i++) {
	  vtkIdType pointId = filteredPointIds->GetValue(i);
	  std::cout << "subset point " << i << " -> original point " <<
	    pointId << "\n";
	  double xyz[3];
	  points->GetPoint(pointId, xyz);
	  std::cout << "x: " << xyz[0] << ", y: " << xyz[1] <<
	    ", z: " << xyz[2] << "\n";

	  // Set selected point data quality to BAD
	  if (quality) {
	    quality->SetValue(pointId, BAD);
	  }

	}
      }
      else {
	std::cout << "Couldn't get original point Ids\n";
      }
    }
  }

  void setPolyData(vtkSmartPointer<vtkPolyData> polyData)
  {
    polyData_ = polyData;
  }

private:
  vtkSmartPointer<vtkPolyData> polyData_;
  vtkSmartPointer<vtkActor> selectedActor_;
  vtkSmartPointer<vtkDataSetMapper> selectedMapper_;  
};
  
vtkStandardNewMacro(HighlightInteractorStyle);

vtkSmartPointer<vtkPolyData> readPolyData(const char* fileName);


class PointCloudEditor {

public:

  PointCloudEditor(vtkPolyData *polyData) {

    polyData_ = polyData;
    
    // Build color lookup table
    lut_->SetNumberOfTableValues(2);
    lut_->SetRange(0, 1);
    lut_->SetTableValue(0, 1.0, 0.0, 0.0, 1.0);
    lut_->SetTableValue(1, 0.0, 1.0, 0.0, 1.0);  
    lut_->Build();

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
    
  }

  /// Visualize the point cloud
  void visualize(void) {

    mapper_->SetInputData(polyData_);
    mapper_->ScalarVisibilityOff();

    // Configure mapper to use LUT stuff
    mapper_->SetLookupTable(lut_);
    mapper_->SetScalarModeToUsePointData();
    mapper_->SetColorModeToMapScalars();
    mapper_->SetScalarRange(0, 1);
    actor_->SetMapper(mapper_);
    actor_->GetProperty()->SetPointSize(5);

    /* ***
    actor_->GetProperty()->
      SetDiffuseColor(colors_->GetColor3d("Peacock").GetData());
      *** */
    
    renderer_->UseHiddenLineRemovalOn();

    renderWindow_->AddRenderer(renderer_);
    renderWindow_->SetSize(640, 480);
    renderWindow_->SetWindowName("HighlightSelection");

    renderWindowInteractor_->SetPicker(areaPicker_);
    renderWindowInteractor_->SetRenderWindow(renderWindow_);

    renderer_->AddActor(actor_);
    renderer_->SetBackground(colors_->GetColor3d("Tan").GetData());

    renderWindow_->Render();

    style_->setPolyData(polyData_);
    renderWindowInteractor_->SetInteractorStyle(style_);

    renderWindowInteractor_->Start();
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

  vtkNew<HighlightInteractorStyle> style_;

  vtkSmartPointer<vtkPolyData> polyData_;
  
  // Add data quality array to input vtkPolyData
  vtkSmartPointer<vtkIntArray> quality_ =
    vtkSmartPointer<vtkIntArray>::New();

};


int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "usage: " << argv[0] << " <swath-or-gridFile>\n";
    return 1;
  }
  
  // Get polyData from specified source
  auto polyData = readPolyData(argv[1]);

  auto editor = new PointCloudEditor(polyData);
  editor->visualize();


  return 0;
}


vtkSmartPointer<vtkPolyData> readPolyData(const char* fileName)
{
  vtkSmartPointer<vtkPolyData> polyData;
  std::string extension =
      vtksys::SystemTools::GetFilenameLastExtension(std::string(fileName));

  if (extension == ".grd" || extension == ".mb88") {
    std::cout << "read mb-system grid\n";
    vtkSmartPointer<mb_system::TopoDataReader> reader =
      vtkSmartPointer<mb_system::TopoDataReader>::New();
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else
  {
    std::cout << "unknown input file format: " << fileName << "\n";
    std::cout << "read sphere source\n";
    vtkNew<vtkSphereSource> source;
    source->SetPhiResolution(21);
    source->SetThetaResolution(40);
    source->Update();
    polyData = source->GetOutput();
  }
  return polyData;
}

