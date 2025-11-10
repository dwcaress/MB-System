// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
/// SPDX-License-Identifier: BSD-3-Clause
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
  this->CurrentMode = ORIENT_MODE;
  this->StartPosition[0] = this->StartPosition[1] = 0;
  this->EndPosition[0] = this->EndPosition[1] = 0;
  this->Moving = 0;
  this->PixelArray = vtkUnsignedCharArray::New();
}

//------------------------------------------------------------------------------
MyRubberBandStyle::~MyRubberBandStyle()
{
  this->PixelArray->Delete();
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::StartSelect()
{
  this->CurrentMode = SELECT_MODE;
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::OnChar()
{
  std::cerr << "OnChar(): CurrentMode=" << CurrentMode << "\n";
  switch (this->Interactor->GetKeyCode())
  {
    case 'r':
    case 'R':
      // r toggles the rubber band selection mode for mouse button 1
      if (this->CurrentMode == ORIENT_MODE)
      {
        this->CurrentMode = SELECT_MODE;
	std::cerr << "OnChar(): in ORIENT_MODE, toggle to SELECT_MODE\n";
	overlayInitialized_ = false;
      }
      else
      {
        this->CurrentMode = ORIENT_MODE;
	std::cerr << "OnChar(): in SELECT_MODE, toggle to ORIENT_MODE\n";
	ClearOverlay();
      }
      break;
    case 'p':
    case 'P':
    {
      vtkRenderWindowInteractor* rwi = this->Interactor;
      int* eventPos = rwi->GetEventPosition();
      this->FindPokedRenderer(eventPos[0], eventPos[1]);
      this->StartPosition[0] = eventPos[0];
      this->StartPosition[1] = eventPos[1];
      this->EndPosition[0] = eventPos[0];
      this->EndPosition[1] = eventPos[1];
      this->Pick();
      break;
    }
    default:
      this->Superclass::OnChar();
  }
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::OnLeftButtonDown()
{
  if (this->CurrentMode != SELECT_MODE)
  {
    // if not in rubber band mode, let the parent class handle it
    this->Superclass::OnLeftButtonDown();
    return;
  }

  std::cerr << "MyRubberBandStyle::onLeftButtonDown()\n";
  
  if (!this->Interactor)
  {
    return;
  }

  // otherwise record the rubber band starting coordinate

  this->Moving = 1;

  vtkRenderWindow* renWin = this->Interactor->GetRenderWindow();

  this->StartPosition[0] = this->Interactor->GetEventPosition()[0];
  this->StartPosition[1] = this->Interactor->GetEventPosition()[1];
  this->EndPosition[0] = this->StartPosition[0];
  this->EndPosition[1] = this->StartPosition[1];

  this->PixelArray->Initialize();
  this->PixelArray->SetNumberOfComponents(4);
  const int* size = renWin->GetSize();
  this->PixelArray->SetNumberOfTuples(size[0] * size[1]);

  renWin->GetRGBACharPixelData(0, 0, size[0] - 1, size[1] - 1, 1, this->PixelArray);

  this->FindPokedRenderer(this->StartPosition[0], this->StartPosition[1]);
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::OnMouseMove()
{
  if (this->CurrentMode != SELECT_MODE)
  {
    // if not in rubber band mode,  let the parent class handle it
    this->Superclass::OnMouseMove();
    return;
  }

  if (!this->Interactor || !this->Moving)
  {
    return;
  }

  this->EndPosition[0] = this->Interactor->GetEventPosition()[0];
  this->EndPosition[1] = this->Interactor->GetEventPosition()[1];
  const int* size = this->Interactor->GetRenderWindow()->GetSize();
  if (this->EndPosition[0] > (size[0] - 1))
  {
    this->EndPosition[0] = size[0] - 1;
  }
  if (this->EndPosition[0] < 0)
  {
    this->EndPosition[0] = 0;
  }
  if (this->EndPosition[1] > (size[1] - 1))
  {
    this->EndPosition[1] = size[1] - 1;
  }
  if (this->EndPosition[1] < 0)
  {
    this->EndPosition[1] = 0;
  }

  // Queue VtkStyle render rubber band box updates via associated QQuickVTKItem
  if (qquickVTKItem_) {
    // std::cerr << "Call RedrawRubberBand() in Qt render thread\n";
    // Queue VtkStyle render updates via QQuickVTKItem
    // Dispatch lambda function to redraw rubber band in Qt render thread
    qquickVTKItem_->dispatch_async([this](vtkRenderWindow *renderWindow,
					  vtkSmartPointer<vtkObject> userData) {
    this->RedrawRubberBand();
  });
    
  }
  else {
    this->RedrawRubberBand();
  }
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::OnLeftButtonUp()
{
  if (this->CurrentMode != SELECT_MODE)
  {
    // if not in rubber band mode,  let the parent class handle it
    this->Superclass::OnLeftButtonUp();
    return;
  }

  if (!this->Interactor || !this->Moving)
  {
    return;
  }


  // otherwise record the rubber band end coordinate and then fire off a pick
  if ((this->StartPosition[0] != this->EndPosition[0]) ||
    (this->StartPosition[1] != this->EndPosition[1]))
  {
    this->Pick();
  }
  this->Moving = 0;
  // this->CurrentMode = ORIENT_MODE;

  overlayInitialized_ = false;
  InitializeOverlay();
}


/* ***** Claude.ai implementation ******* */
void MyRubberBandStyle::RedrawRubberBand() 
{
    if (!overlayInitialized_) {
        std::cerr << "overlay not yet initialized\n";
        InitializeOverlay();
        if (!overlayInitialized_) {
            std::cerr << "FAILED to initialize overlay\n";	  
            return;
        }
    }
    
    // Get window size
    const int* size = this->Interactor->GetRenderWindow()->GetSize();
    
    // Clamp coordinates to window bounds
    double x1 = std::max(0, std::min(this->StartPosition[0], size[0] - 1));
    double y1 = std::max(0, std::min(this->StartPosition[1], size[1] - 1));
    double x2 = std::max(0, std::min(this->EndPosition[0], size[0] - 1));
    double y2 = std::max(0, std::min(this->EndPosition[1], size[1] - 1));

    /// std::cerr << "x1: " << x1 << ", y1: " << y1 <<
    ///      ", x2: " << x2 << ", y2: " << y2 << "\n";
    
    // Create new geometry
    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> lines;
    
    // Use DISPLAY coordinates directly (pixel coordinates)
    points->InsertNextPoint(x1, y1, 0);
    points->InsertNextPoint(x2, y1, 0);
    points->InsertNextPoint(x2, y2, 0);
    points->InsertNextPoint(x1, y2, 0);
    
    // Create closed rectangle (5 points to close the loop)
    vtkIdType rect[5] = {0, 1, 2, 3, 0};
    lines->InsertNextCell(5, rect);
    
    // Update polydata
    rubberBandPolyData_->SetPoints(points);
    rubberBandPolyData_->SetLines(lines);
    rubberBandPolyData_->Modified();
    
    // Render the window
    this->Interactor->GetRenderWindow()->Render();
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::Pick()
{
  // find rubber band lower left, upper right and center
  double rbcenter[3];
  const int* size = this->Interactor->GetRenderWindow()->GetSize();
  int min[2], max[2];
  min[0] =
    this->StartPosition[0] <= this->EndPosition[0] ? this->StartPosition[0] : this->EndPosition[0];
  if (min[0] < 0)
  {
    min[0] = 0;
  }
  if (min[0] >= size[0])
  {
    min[0] = size[0] - 2;
  }

  min[1] =
    this->StartPosition[1] <= this->EndPosition[1] ? this->StartPosition[1] : this->EndPosition[1];
  if (min[1] < 0)
  {
    min[1] = 0;
  }
  if (min[1] >= size[1])
  {
    min[1] = size[1] - 2;
  }

  max[0] =
    this->EndPosition[0] > this->StartPosition[0] ? this->EndPosition[0] : this->StartPosition[0];
  if (max[0] < 0)
  {
    max[0] = 0;
  }
  if (max[0] >= size[0])
  {
    max[0] = size[0] - 2;
  }

  max[1] =
    this->EndPosition[1] > this->StartPosition[1] ? this->EndPosition[1] : this->StartPosition[1];
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

  if (this->State == VTKIS_NONE)
  {
    // tell the RenderWindowInteractor's picker to make it happen
    vtkRenderWindowInteractor* rwi = this->Interactor;

    vtkAssemblyPath* path = nullptr;
    rwi->StartPickCallback();
    vtkAbstractPropPicker* picker = vtkAbstractPropPicker::SafeDownCast(rwi->GetPicker());
    if (picker != nullptr)
    {
      vtkAreaPicker* areaPicker = vtkAreaPicker::SafeDownCast(picker);
      if (areaPicker != nullptr)
      {
        areaPicker->AreaPick(min[0], min[1], max[0], max[1], this->CurrentRenderer);
      }
      else
      {
        picker->Pick(rbcenter[0], rbcenter[1], 0.0, this->CurrentRenderer);
      }
      path = picker->GetPath();
    }
    if (path == nullptr)
    {
      this->HighlightProp(nullptr);
      this->PropPicked = 0;
    }
    else
    {
      // highlight the one prop that the picker saved in the path
      // this->HighlightProp(path->GetFirstNode()->GetViewProp());
      this->PropPicked = 1;
    }
    rwi->EndPickCallback();
  }

  this->Interactor->Render();
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


/* ***** Claude.ai ******* */
void MyRubberBandStyle::InitializeOverlay() {
    if (!this->Interactor || overlayInitialized_) {
        return;
    }
    
    vtkRenderWindow* renWin = this->Interactor->GetRenderWindow();
    
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
  this->Interactor->GetRenderWindow()->Render();
}


void MyRubberBandStyle::SetInteractor(vtkRenderWindowInteractor* interactor) {
  this->Superclass::SetInteractor(interactor);
  InitializeOverlay();
}



VTK_ABI_NAMESPACE_END
