#include <vtkNamedColors.h>
#include <vtkAreaPicker.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkPlanes.h>
#include <vtkPointData.h>
#include "PointsSelectInteractorStyle.h"
#include "TopoDataItem.h"

#define VTKISRBP_ORIENT 0
#define VTKISRBP_SELECT 1

#define DATA_QUALITY_NAME "dataQuality"
#define ORIGINAL_IDS "originalIDs"
#define BAD 0
#define GOOD 1

using namespace mb_system;

vtkStandardNewMacro(PointsSelectInteractorStyle);

void PointsSelectInteractorStyle::OnLeftButtonUp() {
  // Forward events
  MyRubberBandStyle::OnLeftButtonUp();
  if (CurrentMode == SELECT_MODE) {
    vtkNew<vtkNamedColors> colors;

    vtkPlanes* frustum =
      static_cast<vtkAreaPicker*>(GetInteractor()->GetPicker())
      ->GetFrustum();

    vtkNew<vtkExtractPolyDataGeometry> extractor;
    extractor->SetInputData(topoDataItem_->getPipeline()->topoReader_->GetOutput());

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

    selectedActor_->
      GetProperty()->SetColor(colors->GetColor3d("Black").GetData());
      
    selectedActor_->GetProperty()->SetPointSize(1);
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

      vtkPoints *points =
	topoDataItem_->getPipeline()->topoReader_->GetOutput()->GetPoints();
      
      vtkIntArray* quality =
	vtkIntArray::SafeDownCast(topoDataItem_->getPipeline()->topoReader_->
				  GetOutput()->GetPointData()->
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

	// Set selected point data quality
	if (quality) {
	  if (editMode_ == EraseMode) {
	    std::cerr << "set value to BAD\n";
	    quality->SetValue(pointId, BAD);
	  }
	  else {
	    std::cerr << "set value to GOOD\n";	    
	    quality->SetValue(pointId, GOOD);	    
	  }
	}

      }
    }
    else {
      std::cerr << "Couldn't get original point Ids\n";
    }
    std::cerr << "redraw data\n";
    topoDataItem_->update();
  }

}


void PointsSelectInteractorStyle::setTopoDataItem(TopoDataItem *item) {
  topoDataItem_ = item;
  MyRubberBandStyle::setQQuickVTKItem(item);
}
