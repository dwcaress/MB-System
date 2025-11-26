#include <vtkNamedColors.h>
#include <vtkPointPicker.h>
#include <vtkAreaPicker.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPlane.h>
#include <vtkCutter.h>
#include <vtkSphereSource.h>
#include <vtkChartXY.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>
#include <vtkContextActor.h>
#include <vtkTable.h>
#include <vtkFloatArray.h>
#include <vtkPlot.h>
#include <vtkAxis.h>

#include "PointsSelectInteractorStyle.h"
#include "PointCloudEditor.h"

#define VTKISRBP_ORIENT 0
#define VTKISRBP_SELECT 1

vtkStandardNewMacro(PointsSelectInteractorStyle);

void PointsSelectInteractorStyle::OnChar() {
  int startingMode = CurrentMode;
  vtkInteractorStyleRubberBandPick::OnChar();
  if (startingMode == VTKISRBP_SELECT &&
      CurrentMode != VTKISRBP_SELECT) {
    // Just left select mode
    std::cerr << "Just left select mode\n";
    // Remove selected area actor
    GetInteractor()
      ->GetRenderWindow()
      ->GetRenderers()
      ->GetFirstRenderer()
      ->RemoveActor(selectedActor_);
    editor_->visualize();
  }
}

void PointsSelectInteractorStyle::OnLeftButtonUp() {
  // Forward events
  vtkInteractorStyleRubberBandPick::OnLeftButtonUp();
  if (CurrentMode == VTKISRBP_SELECT) {

    if (selectMode_ == SelectionMode::ElevSlice) {
      computeElevationProfile();
      return;
    }
    
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

	// Set selected point data quality
	if (quality) {
	  if (editor_->getEditMode() == EraseMode) {
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
    std::cerr << "Visualize data\n";
    editor_->visualize();
  }

}



void PointsSelectInteractorStyle::computeElevationProfile() {
  vtkRenderer *renderer = editor_->getRenderer();

  // Find world coordinates of start and end points
  vtkNew<vtkPointPicker> picker;
  double startPoint[3], endPoint[3];

  std::cerr << "StartPosition[0]= " << StartPosition[0] <<
    ", StartPosition[1]= " << StartPosition[1] << "\n";

  std::cerr << "EndPosition[0]= " << EndPosition[0] <<
    ", EndPosition[1]= " << EndPosition[1] << "\n";  
  
  double *p;
  if (picker->Pick(static_cast<double>(StartPosition[0]),
		   static_cast<double>(StartPosition[1]), 0, renderer)) {
    p = picker->GetPickPosition();
    std::copy(p, p+3, startPoint);
    std::cerr << "start world x=" << p[0] <<", y=" << p[1] <<
      ", z=" << p[2] << "\n";
  }
  else {
    std::cerr << "Could not pick StartPosition\n";
    return;
  }
      
  if (picker->Pick(static_cast<double>(EndPosition[0]),
		   static_cast<double>(EndPosition[1]), 0, renderer)) {
    p = picker->GetPickPosition();	
    std::copy(p, p+3, endPoint);
    std::cerr << "end world x=" << p[0] <<", y=" << p[1] <<
      ", z=" << p[2] << "\n";    
  }
  else {
    std::cerr << "Could not pick EndPosition\n";
    return;
  }

  // Put a little sphhere ("pin") at start and end points
  vtkNew<vtkSphereSource> startPin;
  vtkNew<vtkSphereSource> endPin;

  startPin->SetCenter(startPoint[0], startPoint[1], startPoint[2]);
  startPin->SetRadius(50.);
  startPin->SetPhiResolution(50);
  startPin->SetThetaResolution(50);
  vtkNew<vtkPolyDataMapper> startPinMapper;
  startPinMapper->SetInputConnection(startPin->GetOutputPort());
  vtkNew<vtkActor> startPinActor;
  startPinActor->SetMapper(startPinMapper);
  startPinActor->GetProperty()->SetColor(1., 0., 0.);
  startPinActor->GetProperty()->SetLineWidth(3.);
  editor_->addActor(startPinActor);

  endPin->SetCenter(endPoint[0], endPoint[1], endPoint[2]);
  endPin->SetRadius(50.);
  endPin->SetPhiResolution(50);
  endPin->SetThetaResolution(50);
  vtkNew<vtkPolyDataMapper> endPinMapper;
  endPinMapper->SetInputConnection(endPin->GetOutputPort());
  vtkNew<vtkActor> endPinActor;
  endPinActor->SetMapper(endPinMapper);
  endPinActor->GetProperty()->SetColor(1., 0., 0.);
  endPinActor->GetProperty()->SetLineWidth(3.);
  editor_->addActor(endPinActor);  
  
  // Compute normal to elevation profile plane; elevation profile plane is vertical,
  // so normal to plane is horizontal
  double normal[3];
  normal[0] = -(endPoint[1] - startPoint[1]);
  normal[1] = endPoint[0] - startPoint[0];
  normal[2] = 0.0;     // normal to z-axis is horizontal

  vtkMath::Normalize(normal);

  // Create the elevation profile plane
  vtkNew<vtkPlane> plane;
  plane->SetOrigin(endPoint);
  plane->SetNormal(normal);

  // Create the cutter filter
  vtkNew<vtkCutter> cutter;
  cutter->SetInputData(editor_->polyData());
  cutter->SetCutFunction(plane);
  cutter->Update();

  // Display profile on main 3D surface 
  // Elevation profile mapper
  vtkNew<vtkPolyDataMapper> profileMapper;
  profileMapper->SetInputConnection(cutter->GetOutputPort());

  // Elevation profile actor
  vtkNew<vtkActor> profileActor;
  profileActor->SetMapper(profileMapper);
  profileActor->GetProperty()->SetColor(1., 0., 0.);
  profileActor->GetProperty()->SetLineWidth(3.);

  // Redraw point cloud, including elevation profile
  editor_->addActor(profileActor);

  editor_->setSurfaceOpacity(0.3);

  std::cerr << "Now extract and graph profile data\n";
  
  // Extract elev profile data for display in 2D graph
  vtkPolyData *profilePolyData = cutter->GetOutput();
  vtkPoints *points = profilePolyData->GetPoints();
  if (!points || points->GetNumberOfPoints() == 0) {
    std::cerr << "No elevation profile intersection found!\n";
    return;
  }

  // Get profile direction vector
  double direction[3];
  direction[0] = endPoint[0] - startPoint[0];
  direction[1] = endPoint[1] - startPoint[1];
  direction[2] = endPoint[2] - startPoint[2];
  vtkMath::Normalize(direction);

  // Fill profileData with sorted (x,y) data for plotting
  // x: distance along profile
  // y: elevation
  std::vector<std::pair<double, double>> profileData;
  
  for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++) {
    double point[3];
    points->GetPoint(i, point);

    if ((point[0] >= startPoint[0] && point[0] <= endPoint[0]) ||
	(point[0] >= endPoint[0] && point[0] <= startPoint[0])) {
      // This point lies on line between startPoint and endPoint;
      // add it to profileData
      double vec[3];
      // Compute coordinates relative to starting point
      vec[0] = point[0] - startPoint[0];
      vec[1] = point[1] - startPoint[1];
      vec[2] = point[2] - startPoint[2];

      // Compute this point's distance along profile = dot product with
      // direction vector
      double distAlongProfile = vtkMath::Dot(vec, direction);
      double elevation = point[2];
      profileData.push_back({distAlongProfile, elevation});
    }
  }

  // Sort by distance along profile
  std::sort(profileData.begin(), profileData.end());

  // Now ready to plot profileData; 
  // x: distance along profile
  // y: elevation

  // Create table for chart
  vtkNew<vtkTable> table;

  vtkNew<vtkFloatArray> xArray;
  xArray->SetName("Distance");
  table->AddColumn(xArray);

  vtkNew<vtkFloatArray> yArray;
  yArray->SetName("Elevation (m)");
  table->AddColumn(yArray);

  table->SetNumberOfRows(profileData.size());
  for (size_t i = 0; i < profileData.size(); i++) {
    table->SetValue(i, 0, profileData[i].first);   // distance
    table->SetValue(i, 1, profileData[i].second);  // elevation
  }

  vtkNew<vtkRenderer> renderer2D;
  renderer2D->SetViewport(0.5, 0.0, 1.0, 1.0);
  renderer2D->SetBackground(1., 1., 1.);
  editor_->getRenderWindow()->AddRenderer(renderer2D);

  vtkNew<vtkChartXY> chart;
  vtkNew<vtkContextScene> scene;
  vtkNew<vtkContextActor> actor;
  
  scene->AddItem(chart);
  actor->SetScene(scene);
  renderer2D->AddActor(actor);
  
  // Add profile data to the chart
  vtkPlot* line = chart->AddPlot(vtkChart::LINE);
  line->SetInputData(table, 0, 1);
  line->SetColor(0, 0, 255, 255); // blue
  line->SetWidth(2.0);

  chart->SetShowLegend(false);
  chart->GetAxis(vtkAxis::BOTTOM)->SetTitle("Distance");
  chart->GetAxis(vtkAxis::BOTTOM)->GetTitleProperties()->SetFontSize(20);
  chart->GetAxis(vtkAxis::BOTTOM)->GetLabelProperties()->SetFontSize(20);
  chart->GetAxis(vtkAxis::LEFT)->SetTitle("Elevation (m)");
  chart->GetAxis(vtkAxis::LEFT)->GetTitleProperties()->SetFontSize(20);
  chart->GetAxis(vtkAxis::LEFT)->GetLabelProperties()->SetFontSize(20);  

  // Assemble pipeline and start event loop
  editor_->visualize();


}
