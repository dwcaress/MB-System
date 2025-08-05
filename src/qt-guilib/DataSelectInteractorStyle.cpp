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
#include <QQuickVTKItem.h>
#include "DataSelectInteractorStyle.h"
#include "TopoDataItem.h"

#define VTKISRBP_ORIENT 0
#define VTKISRBP_SELECT 1
#define ORIGINAL_POINT_IDS "vtkOriginalPointIds"

using namespace mb_system;
//// vtkStandardNewMacro(DataSelectInteractorStyle);

DataSelectInteractorStyle::DataSelectInteractorStyle(TopoDataItem *item) {
  topoDataItem_ = item;
}


void DataSelectInteractorStyle::OnMouseMove()
{
  std::cout << "DataSelectInteractorStyle::OnMouseMove()\n";
  
  if (CurrentMode != VTKISRBP_SELECT)
  {
    // if not in rubber band mode,  let the parent class handle it
    Superclass::OnMouseMove();
    return;
  }

  if (!Interactor || !Moving)
  {
    return;
  }

  EndPosition[0] = Interactor->GetEventPosition()[0];
  EndPosition[1] = Interactor->GetEventPosition()[1];
  const int* size = Interactor->GetRenderWindow()->GetSize();
  if (EndPosition[0] > (size[0] - 1))
  {
    EndPosition[0] = size[0] - 1;
  }
  if (EndPosition[0] < 0)
  {
    EndPosition[0] = 0;
  }
  if (EndPosition[1] > (size[1] - 1))
  {
    this->EndPosition[1] = size[1] - 1;
  }
  if (this->EndPosition[1] < 0)
  {
    this->EndPosition[1] = 0;
  }


  std::cerr << "skip RedrawRubberBand()\n";
  /* ***
  std::cerr << "DataSelectInteractorStyle dispatch RedrawRubberBand()\n";
  // Dispatch lambda function to redraw rubber band in Qt render thread
  topoDataItem_->dispatch_async([this](vtkRenderWindow *renderWindow,
				       vtkSmartPointer<vtkObject> userData) {
    this->RedrawRubberBand();
  });
  *** */
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

      vtkPolyData *polyData =
	topoDataItem_->getPipeline()->topoReader_->GetOutput();
      
      vtkNew<vtkExtractPolyDataGeometry> extractor;
      extractor->SetInputData(polyData);
      
      // Add array containing original data point IDs
      vtkSmartPointer<vtkIdTypeArray> originalPointIds =
	vtkSmartPointer<vtkIdTypeArray>::New();

      originalPointIds->SetName(ORIGINAL_POINT_IDS);
      originalPointIds->SetNumberOfTuples(polyData->GetNumberOfPoints());
      for (int i = 0; i < polyData->GetNumberOfPoints(); i++) {
	originalPointIds->SetValue(i, i);
      }

      // Fill point ID array with original poly data
      polyData->GetPointData()->SetScalars(originalPointIds);
      
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
      topoDataItem_->getPipeline()->surfaceMapper_->SetInputData(extractedData);
      // selectedMapper_->SetInputData(extractedData);
      topoDataItem_->getPipeline()->surfaceMapper_->ScalarVisibilityOff();
      // selectedMapper_->ScalarVisibilityOff();

      std::cerr << "DataSelectInteractor: commented out code!!!!!\n";
      topoDataItem_->getPipeline()->
	surfaceActor_->GetProperty()->
	SetColor(colors->GetColor3d("Tomator").GetData());
      
      /// selectedActor_->GetProperty()->SetColor(
      ///	      colors->GetColor3d("Tomato").GetData());

      topoDataItem_->getPipeline()->
	surfaceActor_->GetProperty()->SetPointSize(5);
      
      ///      selectedActor_->GetProperty()->SetPointSize(5);

      topoDataItem_->getPipeline()->
	surfaceActor_->GetProperty()->SetRepresentationToSurface();
      /// selectedActor_->GetProperty()->SetRepresentationToSurface();

      
      GetInteractor()
	->GetRenderWindow()
	->GetRenderers()
	->GetFirstRenderer()
	->AddActor(topoDataItem_->getPipeline()->
		   surfaceActor_);

      GetInteractor()->GetRenderWindow()->Render();

      // Highlight the selected area
      HighlightProp(nullptr);

      // Get the points
      std::cerr << "GET THE POINTS\n";
      
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

