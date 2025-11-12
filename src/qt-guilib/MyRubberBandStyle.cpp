// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
/// SPDX-License-Identifier: BSD-3-Clause
#define QT_NO_DEBUG_OUTPUT

#include "MyRubberBandStyle.h"
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

#include <thread>

using namespace mb_system;

VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(MyRubberBandStyle);

//------------------------------------------------------------------------------
MyRubberBandStyle::MyRubberBandStyle()
{
  drawingMode_ = DrawingMode::Rectangle;
  startPosition_[0] = startPosition_[1] = 0;
  endPosition_[0] = endPosition_[1] = 0;
  moving_ = 0;
  pixelArray_ = vtkUnsignedCharArray::New();
  drawEnabled_ = false;
}

//------------------------------------------------------------------------------
MyRubberBandStyle::~MyRubberBandStyle()
{
  pixelArray_->Delete();
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::startSelect()
{
  qDebug() << "MyRubberBandStyle::StartSelect()";
  ///  drawingMode_ = DrawingMode::DrawRectangle; // ????
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::OnChar()
{
  qDebug() << "OnChar(): drawingMode_=" << drawingMode_;
  switch (Interactor->GetKeyCode())
  {
    case 'r':
    case 'R':
      // r toggles the drawing rubber band
      drawEnabled_ = !drawEnabled_;
      if (drawEnabled_) {
	qDebug() << "OnChar(): drawEnabled now true, reinitialize overlay";
	overlayInitialized_ = false;
      }
      else
      {
	qDebug() << "OnChar(): drawenabled now false, clear overlay";
	ClearOverlay();
      }
      break;
    case 'p':
    case 'P':
    {
      vtkRenderWindowInteractor* rwi = Interactor;
      int* eventPos = rwi->GetEventPosition();
      FindPokedRenderer(eventPos[0], eventPos[1]);
      startPosition_[0] = eventPos[0];
      startPosition_[1] = eventPos[1];
      endPosition_[0] = eventPos[0];
      endPosition_[1] = eventPos[1];
      Pick();
      break;
    }
    default:
      Superclass::OnChar();
  }
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::OnLeftButtonDown()
{
  if (!drawEnabled_)
  {
    // if not in rubber band mode, let the parent class handle it
    Superclass::OnLeftButtonDown();
    return;
  }

  if (!Interactor)
  {
    return;
  }

  // otherwise record the rubber band starting coordinate

  moving_ = 1;

  vtkRenderWindow* renWin = Interactor->GetRenderWindow();

  startPosition_[0] = Interactor->GetEventPosition()[0];
  startPosition_[1] = Interactor->GetEventPosition()[1];
  endPosition_[0] = startPosition_[0];
  endPosition_[1] = startPosition_[1];

  pixelArray_->Initialize();
  pixelArray_->SetNumberOfComponents(4);
  const int* size = renWin->GetSize();
  pixelArray_->SetNumberOfTuples(size[0] * size[1]);

  renWin->GetRGBACharPixelData(0, 0, size[0] - 1, size[1] - 1, 1, pixelArray_);

  FindPokedRenderer(startPosition_[0], startPosition_[1]);
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::OnMouseMove()
{
  if (!drawEnabled_)  {
    // if not in rubber band mode,  let the parent class handle it
    Superclass::OnMouseMove();
    return;
  }

  if (!Interactor || !moving_)
  {
    return;
  }

  endPosition_[0] = Interactor->GetEventPosition()[0];
  endPosition_[1] = Interactor->GetEventPosition()[1];
  const int* size = Interactor->GetRenderWindow()->GetSize();
  if (endPosition_[0] > (size[0] - 1))
  {
    endPosition_[0] = size[0] - 1;
  }
  if (endPosition_[0] < 0)
  {
    endPosition_[0] = 0;
  }
  if (endPosition_[1] > (size[1] - 1))
  {
    endPosition_[1] = size[1] - 1;
  }
  if (endPosition_[1] < 0)
  {
    endPosition_[1] = 0;
  }

  // Queue VtkStyle render rubber band box updates via associated QQuickVTKItem
  if (qquickVTKItem_) {
    // Queue VtkStyle render updates via QQuickVTKItem
    // Dispatch lambda function to redraw rubber band in Qt render thread
    qquickVTKItem_->dispatch_async([this](vtkRenderWindow *renderWindow,
					  vtkSmartPointer<vtkObject> userData) {
    redrawRubberBand();
  });
    
  }
  else {
    qWarning() << "qquickVtkItem_ has not been set!";
    redrawRubberBand();
  }
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::OnLeftButtonUp()
{
  if (!drawEnabled_) {
    // if not in rubber band mode,  let the parent class handle it
    Superclass::OnLeftButtonUp();
    return;
  }

  if (!Interactor || !moving_)
  {
    return;
  }


  // otherwise record the rubber band end coordinate and then fire off a pick
  if ((startPosition_[0] != endPosition_[0]) ||
    (startPosition_[1] != endPosition_[1]))
  {
    Pick();
  }
  moving_ = 0;

  overlayInitialized_ = false;
  InitializeOverlay();
}


/* ***** Claude.ai implementation ******* */
void MyRubberBandStyle::redrawRubberBand() 
{
  if (!drawEnabled_) {
    return;
  }
    if (!overlayInitialized_) {
      qDebug() << "overlay not yet initialized\n";
      InitializeOverlay();
      if (!overlayInitialized_) {
	qWarning() << "FAILED to initialize overlay";	  
	return;
      }
    }
    
    // Get window size
    const int* size = Interactor->GetRenderWindow()->GetSize();
    
    // Clamp coordinates to window bounds; ensures that rubber band rectangle
    // stays within window boundaries (0 to size-1), i.e. prevent rubber band
    /// rectangle from extending beyond visible area.
    double x1 = std::max(0, std::min(startPosition_[0], size[0] - 1));
    double y1 = std::max(0, std::min(startPosition_[1], size[1] - 1));
    double x2 = std::max(0, std::min(endPosition_[0], size[0] - 1));
    double y2 = std::max(0, std::min(endPosition_[1], size[1] - 1));

    // Create rectangle/line geometry
    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> lines;
    
    if (drawingMode_ == DrawingMode::Rectangle) {
      // Use DISPLAY coordinates directly (pixel coordinates)
      points->InsertNextPoint(x1, y1, 0);  // bottom left
      points->InsertNextPoint(x2, y1, 0);  // bottom right
      points->InsertNextPoint(x2, y2, 0);  // top right 
      points->InsertNextPoint(x1, y2, 0);  // top left

      // Create closed rectanglular path polyline
      // (5 points to close the loop); 
      vtkIdType rect[] = {0, 1, 2, 3, 0};
      lines->InsertNextCell(5, rect);
    }
    else if (drawingMode_ == DrawingMode::Line) {
      // Draw line from bottom left to upper right
      // Create two points for the line
      points->InsertNextPoint(x1, y1, 0);  // Start point
      points->InsertNextPoint(x2, y2, 0);  // End point
    
      // Create a line connecting the two points
      vtkIdType line[] = {0, 1};
      lines->InsertNextCell(2, line);
    }
    else {
      qWarning() << "redrawRubberBand(): unknowing drawing mode";
      return;
    }
    
    // Update rubber band polydata geometry
    rubberBandPolyData_->SetPoints(points);
    rubberBandPolyData_->SetLines(lines);
    // Mark polydata as modified so it will be redrawn when window is 
    // next rendered.
    rubberBandPolyData_->Modified();      
    
    // Render the window
    Interactor->GetRenderWindow()->Render();
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::Pick()
{
  // find rubber band lower left, upper right and center
  double rbcenter[3];
  const int* size = Interactor->GetRenderWindow()->GetSize();
  int min[2], max[2];
  min[0] =
    startPosition_[0] <= endPosition_[0] ? startPosition_[0] : endPosition_[0];
  if (min[0] < 0)
  {
    min[0] = 0;
  }
  if (min[0] >= size[0])
  {
    min[0] = size[0] - 2;
  }

  min[1] =
    startPosition_[1] <= endPosition_[1] ? startPosition_[1] : endPosition_[1];
  if (min[1] < 0)
  {
    min[1] = 0;
  }
  if (min[1] >= size[1])
  {
    min[1] = size[1] - 2;
  }

  max[0] =
    endPosition_[0] > startPosition_[0] ? endPosition_[0] : startPosition_[0];
  if (max[0] < 0)
  {
    max[0] = 0;
  }
  if (max[0] >= size[0])
  {
    max[0] = size[0] - 2;
  }

  max[1] =
    endPosition_[1] > startPosition_[1] ? endPosition_[1] : startPosition_[1];
  if (max[1] < 0)
  {
    max[1] = 0;
  }
  if (max[1] >= size[1])
  {
    max[1] = size[1] - 2;
  }

  rbcenter[0] = (min[0] + max[0]) / 2.0;
  rbcenter[1] = (min[1] + max[1]) / 2.0;
  rbcenter[2] = 0;

  if (State == VTKIS_NONE)
  {
    // tell the RenderWindowInteractor's picker to make it happen
    vtkRenderWindowInteractor* rwi = Interactor;

    vtkAssemblyPath* path = nullptr;
    rwi->StartPickCallback();
    vtkAbstractPropPicker* picker = vtkAbstractPropPicker::SafeDownCast(rwi->GetPicker());
    if (picker != nullptr)
    {
      vtkAreaPicker* areaPicker = vtkAreaPicker::SafeDownCast(picker);
      if (areaPicker != nullptr)
      {
        areaPicker->AreaPick(min[0], min[1], max[0], max[1], CurrentRenderer);
      }
      else
      {
        picker->Pick(rbcenter[0], rbcenter[1], 0.0, CurrentRenderer);
      }
      path = picker->GetPath();
    }
    if (path == nullptr)
    {
      HighlightProp(nullptr);
      PropPicked = 0;
    }
    else
    {
      // highlight the one prop that the picker saved in the path
      // HighlightProp(path->GetFirstNode()->GetViewProp());
      PropPicked = 1;
    }
    rwi->EndPickCallback();
  }

  Interactor->Render();
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}


/* ***** Claude.ai ******* */
void MyRubberBandStyle::InitializeOverlay() {
    if (!Interactor || overlayInitialized_) {
        return;
    }
    
    vtkRenderWindow* renWin = Interactor->GetRenderWindow();
    
    // Set up overlay renderer
    renWin->AddRenderer(overlayRenderer_);
    overlayRenderer_->SetLayer(1); // Overlay layer
    overlayRenderer_->InteractiveOff();
    renWin->SetNumberOfLayers(2);
    
    // Match the viewport of the main renderer
    overlayRenderer_->SetViewport(0.0, 0.0, 1.0, 1.0);
    
    // Initialize polydata
    rubberBandPolyData_->Initialize();
    rubberBandMapper_->SetInputData(rubberBandPolyData_);
    
    // CRITICAL: Set the coordinate system to DISPLAY
    transformCoordinate_->SetCoordinateSystemToDisplay();
    rubberBandMapper_->SetTransformCoordinate(transformCoordinate_);
    
    // Configure the actor
    rubberBandActor_->SetMapper(rubberBandMapper_);
    rubberBandActor_->GetProperty()->SetColor(1.0, 0.0, 0.0); // Red
    rubberBandActor_->GetProperty()->SetLineWidth(2.0);
    rubberBandActor_->GetProperty()->SetOpacity(1.0);
    
    overlayRenderer_->AddActor2D(rubberBandActor_);
    
    overlayInitialized_ = true;
}

void MyRubberBandStyle::ClearOverlay() {
  overlayRenderer_->RemoveAllViewProps();
  Interactor->GetRenderWindow()->Render();
}


void MyRubberBandStyle::SetInteractor(vtkRenderWindowInteractor* interactor) {
  Superclass::SetInteractor(interactor);
  InitializeOverlay();
}



VTK_ABI_NAMESPACE_END
