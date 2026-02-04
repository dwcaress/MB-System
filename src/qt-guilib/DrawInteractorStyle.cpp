// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
/// SPDX-License-Identifier: BSD-3-Clause
/// #define QT_NO_DEBUG_OUTPUT

#include "DrawInteractorStyle.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLState.h"
#include "vtkAbstractPropPicker.h"
#include "vtkAreaPicker.h"
#include "vtkAssemblyPath.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkUnsignedCharArray.h"
#include "vtkPointPicker.h"
#include <vtkPlane.h>
#include <vtkCutter.h>
#include <vtkChartXY.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>
#include <vtkContextActor.h>
#include <vtkTable.h>
#include <vtkFloatArray.h>
#include <vtkPlot.h>
#include <vtkAxis.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPointGaussianMapper.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkHandleWidget.h>
#include <vtkFixedSizeHandleRepresentation3D.h>
#include "TopoDataItem.h"

using namespace mb_system;

vtkStandardNewMacro(DrawInteractorStyle);

//------------------------------------------------------------------------------
DrawInteractorStyle::DrawInteractorStyle()
{
  drawingMode_ = DrawingMode::Rectangle;
  drawEnabled_ = true;
}

//-----------------------------------------------------------------
DrawInteractorStyle::~DrawInteractorStyle()
{
}


//-----------------------------------------------------------------
void DrawInteractorStyle::OnLeftButtonUp() {

  Superclass::OnLeftButtonUp();
  
  // Record current position
  int eventPos[2];
  Interactor->GetEventPosition(eventPos);

  if (eventPos[0] != downEventPos_[0] ||
      eventPos[1] != downEventPos_[1]) {
    // Mouse was moved between button-down and button-up
    return;
  }

  // Get world coordinates of this point
  vtkNew<vtkPointPicker> picker;
  if (picker->Pick(eventPos[0], eventPos[1], 0, GetDefaultRenderer())) {
    double worldCoords[3];
    picker->GetPickPosition(worldCoords);

    std::array<double, 3> point;
    point[0] = worldCoords[0];
    point[1] = worldCoords[1];
    point[2] = worldCoords[2];
    userPath_.push_back(point);
  }
  else {
    qWarning() << "Unable to pick point";
    return;
  }

  if (userPath_.size() != 2) {
    return;
  }

  // Find intersection of vertical plane with surface, between
  // the two defined path points
  double pt1[3];
  std::copy(userPath_[0].begin(), userPath_[0].end(), pt1);
  double pt2[3];
  std::copy(userPath_[1].begin(), userPath_[1].end(), pt2);
  
  qDebug() << "pt1: " << pt1[0] << ", " << pt1[1] << ", " << pt1[2] << "\n";

  qDebug() << "pt2: " << pt2[0] << ", " << pt2[1] << ", " << pt2[2] << "\n";  


  // Clear out line points
  userPath_.clear();

  computeElevationProfile(pt1, pt2);
  
}



//---------------------------------------------------------------
void DrawInteractorStyle::OnLeftButtonDown() {
  Superclass::OnLeftButtonDown();

  if (!Interactor) {
    return;
  }

  Interactor->GetEventPosition(downEventPos_);

  return;
}


void DrawInteractorStyle::initializeOverlay() {
  if (!Interactor || overlayInitialized_) {
    return;
  }
    
  vtkRenderWindow* renWin = Interactor->GetRenderWindow();
    
  // Set up overlay renderer
  renWin->AddRenderer(overlayRenderer_);
  overlayRenderer_->SetLayer(1);           // Use overlay layer 1
  overlayRenderer_->InteractiveOff();
  renWin->SetNumberOfLayers(2);
    
  // Match the viewport of the main renderer
  overlayRenderer_->SetViewport(0.0, 0.0, 1.0, 1.0);
    
  // Initialize polydata
  rubberBandPolyData_->Initialize();
  // Rubber band polydata is source for overlayed pipeline
  rubberBandMapper_->SetInputData(rubberBandPolyData_);
    
  // CRITICAL: Set the overlayed actor coordinate system to DISPLAY
  transformCoordinate_->SetCoordinateSystemToDisplay();
  rubberBandMapper_->SetTransformCoordinate(transformCoordinate_);
    
  overlayInitialized_ = true;
}

void DrawInteractorStyle::clearOverlay() {
  overlayRenderer_->RemoveAllViewProps();
  Interactor->GetRenderWindow()->Render();
}


void DrawInteractorStyle::SetInteractor(vtkRenderWindowInteractor
					*interactor) {
  Superclass::SetInteractor(interactor);
  initializeOverlay();
}



void DrawInteractorStyle::computeElevationProfile(double *startPoint,
						  double *endPoint) {

  topoDataItem_->clearAddedActors();
  handleWidgets_.clear();
  
  // Put a little marker at start and end points
  vtkNew<vtkHandleWidget> startWidget;
  handleWidgets_.push_back(startWidget);
  
  startWidget->SetInteractor(Interactor);

  vtkNew<vtkFixedSizeHandleRepresentation3D> startPin;
  startPin->SetWorldPosition(startPoint);
  startPin->SetHandleSizeInPixels(30);
  startPin->GetProperty()->SetColor(1., 0., 0.);  

  startWidget->SetRepresentation(startPin);
  startWidget->EnabledOn();
  
  vtkNew<vtkHandleWidget> endWidget;
  handleWidgets_.push_back(endWidget);
  
  endWidget->SetInteractor(Interactor);
  vtkNew<vtkFixedSizeHandleRepresentation3D> endPin;
  endPin->SetWorldPosition(endPoint);
  endPin->SetHandleSizeInPixels(30);  
  endPin->GetProperty()->SetColor(1., 0., 0.);
  
  endWidget->SetRepresentation(endPin);
  endWidget->EnabledOn();

  bool earlyReturn = false;
  if (earlyReturn) {
    qDebug() << "Early return for testing, after rendering widgets";
    Interactor->GetRenderWindow()->Render();
    return;
  }

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
  cutter->SetInputData(topoDataItem_->getPolyData());
  cutter->SetCutFunction(plane);
  cutter->Update();

  // Display profile on main 3D surface 
  // Elevation profile mapper
  vtkNew<vtkPolyDataMapper> profileMapper;
  profileMapper->SetInputConnection(cutter->GetOutputPort());

  // Elevation profile actor
  profileActor_->SetMapper(profileMapper);
  profileActor_->GetProperty()->SetColor(1., 0., 0.);
  profileActor_->GetProperty()->SetLineWidth(3.);


  // Add profileActor_ to topoData pipeline 
  topoDataItem_->addActor(profileActor_);

  // Add profileActor_ to topoDataItem renderer
  topoDataItem_->getRenderer()->AddActor(profileActor_);
  
  // editor_->setSurfaceOpacity(0.3);

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

  // Now ready to plot profileData with vtk graph
  // x: distance along profile
  // y: elevation
  
  /* ***
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
  renderer2D->SetViewport(0., 0., 1.0, 0.25);  
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
  *** */

  qDebug() << "render() again";
  // topoDataItem_->render();
  Interactor->GetRenderWindow()->Render();

  // Transfer profile X-Y data to QList<QVector2D, and include as signal
  // payload
  QList<QVector2D> qProfile;
  QVector2D qPoint;
  for (int i = 0; i < profileData.size(); i++) {
    std::pair<double, double> point = profileData.at(i);
    qPoint.setX((float )point.first);
    qPoint.setY((float )point.second);
    qProfile.append(qPoint);
  }

  qDebug() << "emit TopoDataItem::lineDefined()";
  // Emit signal with profile X-Y payload
  emit topoDataItem_->lineDefined(qProfile);
  
}
