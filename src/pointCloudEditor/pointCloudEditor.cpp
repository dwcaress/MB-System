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

namespace {
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

  void SetPolyData(vtkSmartPointer<vtkPolyData> polyData)
  {
    polyData_ = polyData;
  }

private:
  vtkSmartPointer<vtkPolyData> polyData_;
  vtkSmartPointer<vtkActor> selectedActor_;
  vtkSmartPointer<vtkDataSetMapper> selectedMapper_;  
};
  
  vtkStandardNewMacro(HighlightInteractorStyle);

  vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName);
} // namespace

int main(int argc, char* argv[])
{
  // Get polyData from specified source
  auto polyData = ReadPolyData(argc > 1 ? argv[1] : "");

  vtkNew<vtkNamedColors> colors;

  vtkNew<vtkIdFilter> idFilter;

  // Associate id's with original polyData
  idFilter->SetInputData(polyData);

  // Specify name by which to retrieve id's
  idFilter->SetCellIdsArrayName(ORIGINAL_IDS);
  idFilter->SetPointIdsArrayName(ORIGINAL_IDS);


  idFilter->Update();

  // This is needed to convert the ouput of vtkIdFilter (vtkDataSet) back to
  // vtkPolyData.
  vtkNew<vtkDataSetSurfaceFilter> surfaceFilter;
  surfaceFilter->SetInputConnection(idFilter->GetOutputPort());
  surfaceFilter->Update();

  vtkPolyData* input = surfaceFilter->GetOutput();

  // Set lookup table
  vtkNew<vtkLookupTable> lut;
  lut->SetNumberOfTableValues(2);
  lut->SetRange(0, 1);
  lut->SetTableValue(0, 1.0, 0.0, 0.0, 1.0);
  lut->SetTableValue(1, 0.0, 1.0, 0.0, 1.0);  
  lut->Build();
  
  // Create a mapper and actor.
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputData(polyData);
  mapper->ScalarVisibilityOff();

  // Configure mapper to use LUT stuff
  mapper->SetLookupTable(lut);
  mapper->SetScalarModeToUsePointData();
  mapper->SetColorModeToMapScalars();
  mapper->SetScalarRange(0, 1);
  
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetPointSize(5);


  actor->GetProperty()->SetDiffuseColor(
      colors->GetColor3d("Peacock").GetData());

  // Visualize
  vtkNew<vtkRenderer> renderer;
  renderer->UseHiddenLineRemovalOn();

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(640, 480);
  renderWindow->SetWindowName("HighlightSelection");

  vtkNew<vtkAreaPicker> areaPicker;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetPicker(areaPicker);
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("Tan").GetData());

  renderWindow->Render();

  // Add data quality array to input vtkPolyData
  vtkSmartPointer<vtkIntArray> quality =
    vtkSmartPointer<vtkIntArray>::New();

  quality->SetName(DATA_QUALITY_NAME);
  quality->SetNumberOfTuples(input->GetNumberOfPoints());

  // First assume all points are good
  for (int i = 0; i < input->GetNumberOfPoints(); i++) {
    quality->SetValue(i, GOOD);
  }

  // Associate quality array with original poly data
  input->GetPointData()->AddArray(quality);

  vtkNew<HighlightInteractorStyle> style;
  style->SetPolyData(input);
  renderWindowInteractor->SetInteractorStyle(style);

  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
namespace {
vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName)
{
  vtkSmartPointer<vtkPolyData> polyData;
  std::string extension =
      vtksys::SystemTools::GetFilenameLastExtension(std::string(fileName));
  if (extension == ".ply")
  {
    vtkNew<vtkPLYReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".vtp")
  {
    vtkNew<vtkXMLPolyDataReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".obj")
  {
    vtkNew<vtkOBJReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".stl")
  {
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".vtk")
  {
    vtkNew<vtkPolyDataReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".g")
  {
    vtkNew<vtkBYUReader> reader;
    reader->SetGeometryFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".grd" || extension == ".mb88") {
    std::cout << "read mb-system grid\n";
    vtkSmartPointer<mb_system::TopoDataReader> reader =
      vtkSmartPointer<mb_system::TopoDataReader>::New();
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else
  {
    std::cout << "input file: " << fileName << "\n";
    std::cout << "read sphere source\n";
    vtkNew<vtkSphereSource> source;
    source->SetPhiResolution(21);
    source->SetThetaResolution(40);
    source->Update();
    polyData = source->GetOutput();
  }
  return polyData;
}
} // namespace
