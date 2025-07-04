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
#include "DataSelectInteractorStyle.h"

#define VTKISRBP_ORIENT 0
#define VTKISRBP_SELECT 1
#define ORIGINAL_POINT_IDS "vtkOriginalPointIds"

using namespace mb_system;

DataSelectInteractorStyle::DataSelectInteractorStyle() {
  selectedMapper_ = vtkSmartPointer<vtkDataSetMapper>::New();
  selectedActor_ = vtkSmartPointer<vtkActor>::New();
  selectedActor_->SetMapper(selectedMapper_);
}

void DataSelectInteractorStyle::OnLeftButtonUp() {
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
      
      // Add array containing original data point IDs
      vtkSmartPointer<vtkIdTypeArray> originalPointIds =
	vtkSmartPointer<vtkIdTypeArray>::New();

      originalPointIds->SetName(ORIGINAL_POINT_IDS);
      originalPointIds->SetNumberOfTuples(polyData_->GetNumberOfPoints());
      for (int i = 0; i < polyData_->GetNumberOfPoints(); i++) {
	originalPointIds->SetValue(i, i);
      }

      // Associate point ID array with original poly data
      polyData_->GetPointData()->SetScalars(originalPointIds);
      
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
					      colors->GetColor3d("Tomato").GetData());
      
      selectedActor_->GetProperty()->SetPointSize(5);
      // selectedActor_->GetProperty()->SetRepresentationToWireframe();
      selectedActor_->GetProperty()->SetRepresentationToSurface();

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
				     GetArray(ORIGINAL_POINT_IDS));

      if (filteredPointIds) {
	std::cout << "Got original point IDs\n";
	for (int i = 0; i < extractedData->GetNumberOfPoints(); i++) {
	  std::cout << "subset point " << i << " -> original point " <<
	    filteredPointIds->GetValue(i) << "\n";
	}
      }
      else {
	std::cout << "Couldn't get original point Ids\n";
      }

    }
}


void DataSelectInteractorStyle::setPolyData(vtkPolyData* polyData) {
  polyData_ = polyData;
}

  


