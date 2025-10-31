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
#include "TopoDataItem.h"

#include <thread>

using namespace mb_system;

VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(MyRubberBandStyle);

#define VTKISRBP_ORIENT 0
#define VTKISRBP_SELECT 1


//------------------------------------------------------------------------------
MyRubberBandStyle::MyRubberBandStyle()
{
  this->CurrentMode = VTKISRBP_ORIENT;
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
  this->CurrentMode = VTKISRBP_SELECT;
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::OnChar()
{
  switch (this->Interactor->GetKeyCode())
  {
    case 'r':
    case 'R':
      // r toggles the rubber band selection mode for mouse button 1
      if (this->CurrentMode == VTKISRBP_ORIENT)
      {
        this->CurrentMode = VTKISRBP_SELECT;
      }
      else
      {
        this->CurrentMode = VTKISRBP_ORIENT;
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
  if (this->CurrentMode != VTKISRBP_SELECT)
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
  std::cerr << "MyRubberBandStyle::OnMouseMove() thread: " <<
    std::this_thread::get_id() << "\n";
  
  if (this->CurrentMode != VTKISRBP_SELECT)
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

  // Queue VtkStyle render updates via TopoDataItem
  if (topoDataItem_) {
    std::cerr << "Call RedrawRubberBand() in Qt render thread\n";
    // Queue VtkStyle render updates via TopoDataItem
    topoDataItem_->queueVtkStyleRender(this);
  }
  else {
    this->RedrawRubberBand();
  }

  std::cerr << "Leave MyRubberBandStyle::OnMouseMove()\n";  
}

//------------------------------------------------------------------------------
void MyRubberBandStyle::OnLeftButtonUp()
{
  if (this->CurrentMode != VTKISRBP_SELECT)
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
  // this->CurrentMode = VTKISRBP_ORIENT;
}


#ifdef OLD_REDRAW
//------------------------------------------------------------------------------
void MyRubberBandStyle::RedrawRubberBand()
{
  std::cerr << "MyRubberBandStyle::RedrawRubberBand() thread: " <<
    std::this_thread::get_id() << "\n";
  
  // update the rubber band on the screen
  const int* size = this->Interactor->GetRenderWindow()->GetSize();

  vtkUnsignedCharArray* tmpPixelArray = vtkUnsignedCharArray::New();
  tmpPixelArray->DeepCopy(this->PixelArray);
  unsigned char* pixels = tmpPixelArray->GetPointer(0);

  int min[2], max[2];

  min[0] =
    this->StartPosition[0] <= this->EndPosition[0] ? this->StartPosition[0] : this->EndPosition[0];
  if (min[0] < 0)
  {
    min[0] = 0;
  }
  if (min[0] >= size[0])
  {
    min[0] = size[0] - 1;
  }

  min[1] =
    this->StartPosition[1] <= this->EndPosition[1] ? this->StartPosition[1] : this->EndPosition[1];
  if (min[1] < 0)
  {
    min[1] = 0;
  }
  if (min[1] >= size[1])
  {
    min[1] = size[1] - 1;
  }

  max[0] =
    this->EndPosition[0] > this->StartPosition[0] ? this->EndPosition[0] : this->StartPosition[0];
  if (max[0] < 0)
  {
    max[0] = 0;
  }
  if (max[0] >= size[0])
  {
    max[0] = size[0] - 1;
  }

  max[1] =
    this->EndPosition[1] > this->StartPosition[1] ? this->EndPosition[1] : this->StartPosition[1];
  if (max[1] < 0)
  {
    max[1] = 0;
  }
  if (max[1] >= size[1])
  {
    max[1] = size[1] - 1;
  }

  int i;
  for (i = min[0]; i <= max[0]; i++)
  {
    pixels[4 * (min[1] * size[0] + i)] = 255 ^ pixels[4 * (min[1] * size[0] + i)];
    pixels[4 * (min[1] * size[0] + i) + 1] = 255 ^ pixels[4 * (min[1] * size[0] + i) + 1];
    pixels[4 * (min[1] * size[0] + i) + 2] = 255 ^ pixels[4 * (min[1] * size[0] + i) + 2];
    pixels[4 * (max[1] * size[0] + i)] = 255 ^ pixels[4 * (max[1] * size[0] + i)];
    pixels[4 * (max[1] * size[0] + i) + 1] = 255 ^ pixels[4 * (max[1] * size[0] + i) + 1];
    pixels[4 * (max[1] * size[0] + i) + 2] = 255 ^ pixels[4 * (max[1] * size[0] + i) + 2];
  }
  for (i = min[1] + 1; i < max[1]; i++)
  {
    pixels[4 * (i * size[0] + min[0])] = 255 ^ pixels[4 * (i * size[0] + min[0])];
    pixels[4 * (i * size[0] + min[0]) + 1] = 255 ^ pixels[4 * (i * size[0] + min[0]) + 1];
    pixels[4 * (i * size[0] + min[0]) + 2] = 255 ^ pixels[4 * (i * size[0] + min[0]) + 2];
    pixels[4 * (i * size[0] + max[0])] = 255 ^ pixels[4 * (i * size[0] + max[0])];
    pixels[4 * (i * size[0] + max[0]) + 1] = 255 ^ pixels[4 * (i * size[0] + max[0]) + 1];
    pixels[4 * (i * size[0] + max[0]) + 2] = 255 ^ pixels[4 * (i * size[0] + max[0]) + 2];
  }

  /* **** Recommended by Claude.ai *** */
  vtkOpenGLRenderWindow* renWin = 
    vtkOpenGLRenderWindow::SafeDownCast(this->Interactor->GetRenderWindow());

  vtkOpenGLState* ostate;
  if (renWin) {
    std::cerr << "got renWin\n";
    ostate = renWin->GetState();
        
    // Push current state
    ostate->Push();
  }
  /* ******************************* */
  std::cerr << "RedrawRubberBand(): set rgb pixel data\n";

  this->Interactor->
    GetRenderWindow()->SetRGBACharPixelData(
					    0, 0, size[0] - 1, size[1] - 1,
					    pixels, 0);

  
  /* **** Recommended by Claude.ai *** */
  if (renWin) {
    // Pop back to previous state
    ostate->Pop();
  }
  /* ******************************* */  
  
  std::cerr << "RedrawRubberBand(): get render window frame\n";
  this->Interactor->GetRenderWindow()->Frame();
  
  std::cerr << "RedrawRubberBand(): delete tmpPixelArray\n";
  tmpPixelArray->Delete();
  std::cerr << "RedrawRubberBand(): tmpPixelArray DELETED\n";  

  std::cerr << "Leave MyRubberBandStyle::RedrawRubberBand()\n";  
}
#endif

#ifdef OLD_REDRAW
void MyRubberBandStyle::RedrawRubberBand() {
  if (!overlayInitialized_) return;
        
  // Create rectangle geometry
  vtkNew<vtkPoints> points;
  vtkNew<vtkCellArray> lines;
        
  const int* size = this->Interactor->GetRenderWindow()->GetSize();
        
  // Convert to normalized coordinates
  double x1 = (double)this->StartPosition[0] / size[0];
  double y1 = (double)this->StartPosition[1] / size[1];
  double x2 = (double)this->EndPosition[0] / size[0];
  double y2 = (double)this->EndPosition[1] / size[1];

  std::cerr << "RedrawRubberBand(): x1=" << x1 << "y1=" << y1 << "\n";
  std::cerr << "RedrawRubberBand(): x2=" << x2 << "y2=" << y2 << "\n";
  
  // Create rectangle points
  points->InsertNextPoint(x1, y1, 0);
  points->InsertNextPoint(x2, y1, 0);
  points->InsertNextPoint(x2, y2, 0);
  points->InsertNextPoint(x1, y2, 0);
        
  // Create rectangle lines
  vtkIdType rect[5] = {0, 1, 2, 3, 0};
  lines->InsertNextCell(5, rect);
        
  // Update polydata
  vtkPolyData* polyData = rubberBandMapper_->GetInput();
  polyData->SetPoints(points);
  polyData->SetLines(lines);
  polyData->Modified();
        
  // Set color to red
  rubberBandActor_->GetProperty()->SetColor(1.0, 0.0, 0.0);
  rubberBandActor_->GetProperty()->SetLineWidth(2.0);
        
  // Render only the overlay
  overlayRenderer_->GetRenderWindow()->Render();
}

#endif

/* ***
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
    
    // Clear previous geometry
    rubberBandPolyData_->Reset();
    
    // Get window size for coordinate validation
    const int* size = this->Interactor->GetRenderWindow()->GetSize();
    
    // Clamp coordinates to window bounds
    double x1 = std::max(0, std::min(this->StartPosition[0], size[0] - 1));
    double y1 = std::max(0, std::min(this->StartPosition[1], size[1] - 1));
    double x2 = std::max(0, std::min(this->EndPosition[0], size[0] - 1));
    double y2 = std::max(0, std::min(this->EndPosition[1], size[1] - 1));

    std::cerr << "x1: " << x1 << ", y1: " << y1 <<
      ", x2: " << x2 << ", y2: " << y2 << "\n";
    
    // Only draw if we have a valid rectangle
    if (abs(x2 - x1) > 1 && abs(y2 - y1) > 1)
    {
        vtkNew<vtkPoints> points;
        vtkNew<vtkCellArray> lines;
        
        // Create 4 corners of rectangle
        points->InsertNextPoint(x1, y1, 0);
        points->InsertNextPoint(x2, y1, 0);
        points->InsertNextPoint(x2, y2, 0);
        points->InsertNextPoint(x1, y2, 0);
        
        // Create 4 lines for rectangle edges
        vtkIdType line1[2] = {0, 1}; lines->InsertNextCell(2, line1);
        vtkIdType line2[2] = {1, 2}; lines->InsertNextCell(2, line2);
        vtkIdType line3[2] = {2, 3}; lines->InsertNextCell(2, line3);
        vtkIdType line4[2] = {3, 0}; lines->InsertNextCell(2, line4);
        
        rubberBandPolyData_->SetPoints(points);
        rubberBandPolyData_->SetLines(lines);
        rubberBandPolyData_->Modified();
    }
    else {
      std::cerr << "INVALID RECTANGLE\n";
    }
    
    // Render
    this->Interactor->GetRenderWindow()->Render();
}
*** */

/* ***** Claude.ai ******* */
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

    std::cerr << "x1: " << x1 << ", y1: " << y1 <<
              ", x2: " << x2 << ", y2: " << y2 << "\n";
    
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


/* ***
void MyRubberBandStyle::InitializeOverlay() {
  if (!this->Interactor || overlayInitialized_) {
    return;
  }
        
  // Set up overlay renderer
  this->Interactor->GetRenderWindow()->AddRenderer(overlayRenderer_);
  overlayRenderer_->SetLayer(1); // Overlay layer
  overlayRenderer_->InteractiveOff();
  this->Interactor->GetRenderWindow()->SetNumberOfLayers(2);
        
  // Create rubber band actor
  vtkNew<vtkPolyData> polyData;
  rubberBandMapper_->SetInputData(polyData);
  rubberBandActor_->SetMapper(rubberBandMapper_);
  overlayRenderer_->AddActor(rubberBandActor_);
        
  overlayInitialized_ = true;
}
*** */

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

void MyRubberBandStyle::SetInteractor(vtkRenderWindowInteractor* interactor) {
  this->Superclass::SetInteractor(interactor);
  InitializeOverlay();
}



VTK_ABI_NAMESPACE_END
