// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MyRubberBandStyle_h
#define MyRubberBandStyle_h

#include "vtkInteractionStyleModule.h" // For export macro
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkRenderer.h"
#include "vtkActor2D.h"
#include "vtkProperty2D.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPolyData.h"
#include <QQuickVTKItem.h>

/* ***
#define ORIENT_MODE 0
#define SELECT_MODE 1
*** */

VTK_ABI_NAMESPACE_BEGIN
class vtkUnsignedCharArray;


namespace mb_system {

  /* **
     Code based on vtkInteractorStyleRubberBandPick, with modifications to work properly with
     QQuickVTKItem.
     
     Like TrackBallCamera, but this can pick props underneath a rubber band selection rectangle.
     
     This interactor style allows the user to draw a rectangle or line in the render window. The user
     presses the 'r' key to toggle in and out of draw mode. When in draw mode, the user presses the left
     mouse button and drags to define upper-left/lower-right rectangle corners or line start and end points.
     When in DrawingMode::Rectangle and the left mouse button is released, the attached picker operates
     on the pixel in the center of the selection rectangle. If the picker happens to be a vtkAreaPicker it will
     operate on the entire selection rectangle. When the 'p' key is hit the above pick operation occurs on a
     1x1 rectangle. In other respects it behaves the same as its parent class.
     This class is based on vtkInteractorStyleRubberBandPicker code, with changes
     suggested by humans and Claude.ai to fix bugs arising from use with 
     QQuickVTKItem. In particular VTK 'drawing' operations of lines and boxes
     must be deferred to the the Qt render thread, via
     QQuickVTKItem::dispatch_async(); therefore this class includes a
     reference to the associated QQuickVTKItem object.
     Note that shapes (box, line) defined by the mouse are defined and drawn into the
     'overlay' (i.e. layer-1 of the associated vtkRenderWindow), in display coordinates.

  */
class VTKINTERACTIONSTYLE_EXPORT MyRubberBandStyle
  : public vtkInteractorStyleTrackballCamera
{
public:

  enum class DrawingMode {
    Rectangle, Line
  };

  /// Return draw-enabled state
  bool drawEnabled() { return drawEnabled_; }

  /// Set draw-enabled state
  void setDrawEnable(bool enabled) { drawEnabled_ = enabled; }
  void setDrawingMode(DrawingMode mode) {
    drawingMode_ = mode;
  }
  
  static MyRubberBandStyle* New();
  vtkTypeMacro(MyRubberBandStyle, vtkInteractorStyleTrackballCamera);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void startSelect();

  ///@{
  /**
   * Event bindings
   */
  void OnMouseMove() override;
  void OnLeftButtonDown() override;
  void OnLeftButtonUp() override;
  void OnChar() override;
  ///@}

  /// Set QQuickVTKItem
  void setQQuickVTKItem(QQuickVTKItem *item) {
    qquickVTKItem_ = item;
  }

  /// Redraw mouse-traced shape
  /// Public, so it can be queued by QQuickVtkItem::async_update()
  void redrawRubberBand();

  void SetInteractor(vtkRenderWindowInteractor* interactor) override;

  
protected:
  MyRubberBandStyle();
  ~MyRubberBandStyle() override;

  virtual void Pick();

  bool drawEnabled_;
  
  int startPosition_[2];
  int endPosition_[2];

  int moving_;

  vtkUnsignedCharArray* pixelArray_;

  /// Current drawing mode
  DrawingMode drawingMode_;

  QQuickVTKItem *qquickVTKItem_;

  vtkNew<vtkRenderer> overlayRenderer_;
  vtkNew<vtkActor2D> rubberBandActor_;
  vtkNew<vtkPolyDataMapper2D> rubberBandMapper_;
  vtkNew<vtkPolyData> rubberBandPolyData_;
  vtkNew<vtkCoordinate> transformCoordinate_;
  bool overlayInitialized_ = false;

  void InitializeOverlay();

  /// Clear overlay contents (selection rectangle)
  void ClearOverlay();
  
private:
  MyRubberBandStyle(const MyRubberBandStyle&) = delete;
  void operator=(const MyRubberBandStyle&) = delete;
};

}
VTK_ABI_NAMESPACE_END
#endif
