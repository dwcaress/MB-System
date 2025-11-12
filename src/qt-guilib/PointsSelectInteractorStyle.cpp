#define QT_NO_DEBUG_OUTPUT

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

using namespace mb_system;

vtkStandardNewMacro(PointsSelectInteractorStyle);

void PointsSelectInteractorStyle::OnLeftButtonUp() {
  // Forward events
  MyRubberBandStyle::OnLeftButtonUp();

  /// TEST TEST TEST
  vtkIdTypeArray *ids =
    vtkIdTypeArray::SafeDownCast(topoDataItem_->getPolyData()->GetPointData()->
				   GetArray(ORIGINAL_IDS));
  if (ids) {
    qDebug() << "OnLeftButtonUp(): FOUND original IDs in topoDataItem polydata";
  }
  else {
    qDebug() << "OnLeftButtonUp(): COULD NOT FIND original IDs in topoDataItem polydata";    
  }
				 
  if (drawingMode_ == DrawingMode::Rectangle) {
    vtkNew<vtkNamedColors> colors;

    vtkPlanes* frustum =
      static_cast<vtkAreaPicker*>(GetInteractor()->GetPicker())
      ->GetFrustum();

    vtkNew<vtkExtractPolyDataGeometry> extractor;
    extractor->SetInputData(topoDataItem_->getPolyData());

    // Extract cells that lie within the user-specified frustrum
    extractor->SetImplicitFunction(frustum);
    extractor->ExtractInsideOn();  // T.O'R.
    // Extract the cells
    extractor->Update();

    vtkPolyData *extractedData = extractor->GetOutput();
      
    qDebug() << "Extracted "
	      << extractedData->GetNumberOfCells()
	     << " cells.";

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
    qDebug() << "Got " << points->GetNumberOfPoints() << " points";
    for (int i = 0; i < points->GetNumberOfPoints(); i++) {
      double *point = points->GetPoint(i);
    }


    // Get subsetted original point IDs
    vtkIdTypeArray *filteredPointIds =
      vtkIdTypeArray::SafeDownCast(extractedData->GetPointData()->
				   GetArray(ORIGINAL_IDS));

    if (filteredPointIds) {
      qDebug() << "Got original point IDs";

      vtkPoints *points =
	topoDataItem_->getPolyData()->GetPoints();
      
      vtkIntArray* quality =
	vtkIntArray::SafeDownCast(topoDataItem_->getPolyData()->GetPointData()->
				  GetArray(DATA_QUALITY_NAME));
      if (!quality) {
	qWarning() << "Couldn't find " << DATA_QUALITY_NAME << "!!";
      }
	
      for (int i = 0; i < extractedData->GetNumberOfPoints(); i++) {
	vtkIdType pointId = filteredPointIds->GetValue(i);
	double xyz[3];
	points->GetPoint(pointId, xyz);
	qDebug() << "x: " << xyz[0] << ", y: " << xyz[1] <<
	  ", z: " << xyz[2];

	// Set selected point data quality
	if (quality) {
	  if (editMode_ == EraseMode) {
	    qDebug() << "set value to BAD";
	    quality->SetValue(pointId, BAD_DATA);
	  }
	  else {
	    qDebug() << "set value to GOOD";
	    quality->SetValue(pointId, GOOD_DATA);	    
	  }
	}

      }
    }
    else {
      qDebug() << "Couldn't get original point Ids";
    }
    qDebug() << "redraw data";
    topoDataItem_->update();
  }

}


void PointsSelectInteractorStyle::setTopoDataItem(TopoDataItem *item) {
  topoDataItem_ = item;
  MyRubberBandStyle::setQQuickVTKItem(item);
}
