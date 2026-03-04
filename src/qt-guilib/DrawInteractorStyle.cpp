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

  int pointIndex = userPath_.size() - 1;

  qDebug() << "userPath_.size() = " << userPath_.size();  
  if (userPath_.size() == 1) {
    qDebug() << "Clear existing widgets and profile line";
    
    // First point of profile defined; clear existing widgets
    topoDataItem_->clearAddedActors();
    pinWidgets_.clear();
    pinRepresentations_.clear();
  }
  
  double point[3];
  std::copy(userPath_[pointIndex].begin(), userPath_[pointIndex].end(), point);
  
  // Put a pin marker at selected point
  vtkNew<vtkHandleWidget> pinWidget;
  pinWidgets_.push_back(pinWidget);
  
  pinWidget->SetInteractor(Interactor);

  vtkNew<vtkFixedSizeHandleRepresentation3D> pin;
  pinRepresentations_.push_back(pin);
    
  pin->SetWorldPosition(point);
  pin->SetHandleSizeInPixels(30);
  pin->GetProperty()->SetColor(1., 0., 0.);  
  pinWidget->SetRepresentation(pin);
  pinWidget->EnabledOn();
  // Render the pin
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

  // Clear out line points
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



void DrawInteractorStyle::computeElevationProfile(double *startPoint,
						  double *endPoint) {

  // Compute normal to elevation profile plane; elevation profile plane is
  // vertical, so normal to plane is horizontal
  double normal[3];
  normal[0] = -(endPoint[1] - startPoint[1]);
  normal[1] = endPoint[0] - startPoint[0];
  normal[2] = 0.0;     // normal to z-axis is horizontal

  vtkMath::Normalize(normal);

  // Create the elevation profile plane
  profilePlane_->SetOrigin(endPoint);
  profilePlane_->SetNormal(normal);

  // Add cutter filter
  profileCutter_->SetInputData(topoDataItem_->getPolyData());
  profileCutter_->SetCutFunction(profilePlane_);
  profileCutter_->Update();

  // Display profile on main 3D surface 
  // Elevation profile mapper
  profileMapper_->SetInputConnection(profileCutter_->GetOutputPort());

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
