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
#include "vtkPlane.h"
#include "vtkChartXY.h"
#include "vtkContextView.h"
#include "vtkContextScene.h"
#include "vtkContextActor.h"
#include "vtkTable.h"
#include "vtkFloatArray.h"
#include "vtkPlot.h"
#include "vtkAxis.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkPointGaussianMapper.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkHandleWidget.h"
#include "vtkFixedSizeHandleRepresentation3D.h"
#include "vtkCallbackCommand.h"
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

  // Left-mouse button clicked without moving;
  // Get world coordinates of this point
  vtkNew<vtkPointPicker> picker;
  if (picker->Pick(eventPos[0], eventPos[1], 0, GetDefaultRenderer())) {
    // Successfully picked world coordinates
    double worldCoords[3];
    picker->GetPickPosition(worldCoords);

    std::array<double, 3> point;
    point[0] = worldCoords[0];
    point[1] = worldCoords[1];
    point[2] = worldCoords[2];
    userPath_.push_back(point);
  }
  else {
    qWarning() << "ERROR: Unable to pick point";
    return;
  }

  // Which point in user-defined path?
  int pointIndex = userPath_.size() - 1;

  qDebug() << "userPath_.size() = " << userPath_.size();  
  if (userPath_.size() == 1) {
    // Defining first point in path
    qDebug() << "Clear existing widgets and profile line";
    
    // First point of profile defined; clear existing widgets
    topoDataItem_->clearAddedActors();

    // Detach each widget from the Interactor before releasing memory
    for (auto &w : pinWidgets_) {
      w->EnabledOff();
      w->SetInteractor(nullptr);
    }
    
    pinWidgets_.clear();
    pinRepresentations_.clear();
  }

  // Note: vtkHandleRepresentation takes world position as 'c-style' double
  // array, so must copy vector of std::array<double, 3> to this double array
  double point[3];
  // Copy coordinates of this point to double array
  std::copy(userPath_[pointIndex].begin(), userPath_[pointIndex].end(), point);
  
  // Put a pin marker at selected point
  auto pinWidget = vtkSmartPointer<vtkHandleWidget>::New();
  auto pin = vtkSmartPointer<vtkFixedSizeHandleRepresentation3D>::New();

  pin->SetWorldPosition(point);    // C-style double array input
  pin->SetHandleSizeInPixels(30);
  pin->GetProperty()->SetColor(1., 0., 0.);  
  pinWidget->SetInteractor(Interactor);
  pinWidget->SetRepresentation(pin);
  pinWidget->EnabledOn();

  pinWidgets_.push_back(pinWidget);  // pinWidget should persist
  pinRepresentations_.push_back(pin); // pin representation should persist

  // Render the pins
  Interactor->GetRenderWindow()->Render();
  
  if (userPath_.size() != 2) {
    return;
  }

  // Find intersection of vertical plane with surface, between
  // the two defined path points
  double pt1[3];
  std::copy(userPath_[0].begin(), userPath_[0].end(), pt1);
  double pt2[3];
  std::copy(userPath_[1].begin(), userPath_[1].end(), pt2);
  
  computeElevationProfile(pt1, pt2);

  // Clear user-defined profile points
  userPath_.clear();

  
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


void DrawInteractorStyle::SetInteractor(vtkRenderWindowInteractor
					*interactor) {
  Superclass::SetInteractor(interactor);
}



void DrawInteractorStyle::computeElevationProfile(double *p1,
						  double *p2) {

  // Compute normal to elevation profile plane; elevation profile plane is
  // vertical, so normal to plane is horizontal
  double normal[3];
  normal[0] = -(p2[1] - p1[1]);
  normal[1] = p2[0] - p1[0];
  normal[2] = 0.0;     // normal to z-axis is horizontal

  vtkMath::Normalize(normal);

  // Create the elevation profile plane
  profilePlane_->SetOrigin(p2);
  profilePlane_->SetNormal(normal);

  // Add cutter filter
  profileCutter_->SetInputData(topoDataItem_->getPolyData());
  profileCutter_->SetCutFunction(profilePlane_);
  profileCutter_->Update();

  // Clip the infinite intersection line outside the segment [p1, p2] ---
  // Compute the axis direction along p1->p2
  double dx = p2[0] - p1[0];
  double dy = p2[1] - p1[1];
  double len = std::sqrt(dx*dx + dy*dy);
  double ux = dx / len;   // unit vector along p1->p2
  double uy = dy / len;

  // We need a bounding box that:
  //   - spans p1..p2 along the horizontal axis
  //   - is wide enough in the perpendicular horizontal direction to include
  //     any numerical slop from the cutter (a thin slab)
  //   - spans the full Z range of the surface
  double slop = 1e-3;   // tiny perpendicular half-width; adjust if needed

  // Perpendicular unit vector (rotated 90° in XY)
  double px = -uy;
  double py =  ux;

  // Four XY corners of the slab
  // Corner = p1 or p2  ±  slop * perp
  double corners[4][2] = {
    { p1[0] + slop*px,  p1[1] + slop*py },
    { p1[0] - slop*px,  p1[1] - slop*py },
    { p2[0] + slop*px,  p2[1] + slop*py },
    { p2[0] - slop*px,  p2[1] - slop*py }
  };

  double xMin = corners[0][0], xMax = corners[0][0];
  double yMin = corners[0][1], yMax = corners[0][1];
  for (int i = 1; i < 4; ++i) {
    xMin = std::min(xMin, corners[i][0]);
    xMax = std::max(xMax, corners[i][0]);
    yMin = std::min(yMin, corners[i][1]);
    yMax = std::max(yMax, corners[i][1]);
  }

  // Z bounds: cover the full surface Z range (or use known terrain extents)
  double bounds[6];
  topoDataItem_->getPolyData()->GetBounds(bounds);
  double zMin = bounds[4];
  double zMax = bounds[5];

  // Build the box implicit function
  
  profileBox_->SetBounds(xMin, xMax, yMin, yMax, zMin, zMax);

  // Clip: keep only the part of the cut line INSIDE the box
  // (InsideOut=0 keeps points where box implicit function < 0, i.e. inside)
  profileClipper_->SetInputConnection(profileCutter_->GetOutputPort());
  profileClipper_->SetClipFunction(profileBox_);
  profileClipper_->InsideOutOn();   // keep the region inside the box
  profileClipper_->Update();
  
  // Display profile on TopoDataItem 3D surface 
  // Elevation profile mapper
  profileMapper_->SetInputConnection(profileClipper_->GetOutputPort());

  // Elevation profile actor
  profileActor_->SetMapper(profileMapper_);
  profileActor_->GetProperty()->SetColor(1., 0., 0.);
  profileActor_->GetProperty()->SetLineWidth(3.);

  // Add profileActor_ to topoDataItem renderer
  topoDataItem_->getRenderer()->AddActor(profileActor_);
  
  // editor_->setSurfaceOpacity(0.3);

  std::cerr << "Now extract and graph profile data\n";
  
  // Extract elev profile data for display in 2D graph
  vtkPolyData *profilePolyData = profileCutter_->GetOutput();
  vtkPoints *points = profilePolyData->GetPoints();
  if (!points || points->GetNumberOfPoints() == 0) {
    std::cerr << "No elevation profile intersection found!\n";
    return;
  }

  // Get profile direction vector
  double direction[3];
  direction[0] = p2[0] - p1[0];
  direction[1] = p2[1] - p1[1];
  direction[2] = p2[2] - p1[2];
  vtkMath::Normalize(direction);

  // Fill profileData with sorted (x,y) data for plotting
  // x: distance along profile
  // y: elevation
  std::vector<std::pair<double, double>> profileData;
  
  for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++) {
    double point[3];
    points->GetPoint(i, point);

    if ((point[0] >= p1[0] && point[0] <= p2[0]) ||
	(point[0] >= p2[0] && point[0] <= p1[0])) {
      // This point lies on line between p1 and p2;
      // add it to profileData
      double vec[3];
      // Compute coordinates relative to starting point
      vec[0] = point[0] - p1[0];
      vec[1] = point[1] - p1[1];
      vec[2] = point[2] - p1[2];

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

  qDebug() << "render() again";
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
