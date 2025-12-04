#ifndef DrawInteractorStyle_h
#define DrawInteractorStyle_h
#include "vtkInteractionStyleModule.h" // For export macro
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkRenderer.h"
#include "vtkActor2D.h"
#include "vtkProperty2D.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPolyData.h"
#include <QObject>
#include <QQuickVTKItem.h>
#include "Point.h"

class vtkUnsignedCharArray;

namespace mb_system {

  class TopoDataItem;
  
  /* **
     Like TrackBallCamera, but user can define path/polygon/etc
     by clicking left mouse button.

     Note that shapes (path, polygon, etc) defined by the mouse are
     drawn into the 'overlay' renderer (i.e. layer-1 of the associated
     vtkRenderWindow), in display coordinates.

  */
  class DrawInteractorStyle
    : public QObject, public vtkInteractorStyleTrackballCamera {

    Q_OBJECT

    
  public:

    enum class DrawingMode {
      Line, Path, Polygon, Rectangle
    };

    /// Return draw-enabled state
    bool drawEnabled() { return drawEnabled_; }

    /// Enable/disable drawing
    void setDrawEnable(bool enabled) { drawEnabled_ = enabled; }

    /// Set drawing type
    void setDrawingMode(DrawingMode mode) {
      drawingMode_ = mode;
    }
  
    static DrawInteractorStyle* New();
    vtkTypeMacro(DrawInteractorStyle,
		 vtkInteractorStyleTrackballCamera);


    ///@{
    /**
     * Event bindings
     */
    /// Define point
    void OnLeftButtonDown() override;
    void OnLeftButtonUp() override;


    /// Set QQuickVTKItem
    void setTopoDataItem(TopoDataItem *item) {
      topoDataItem_ = item;
    }

  
  protected:

    void computeElevationProfile(double pt1[3], double pt2[3]);
    
    DrawInteractorStyle();
    ~DrawInteractorStyle() override;

    void initializeOverlay();

    /// Clear overlay contents (selection rectangle)
    void clearOverlay();

    /// Set interactor, initialize overlay
    void SetInteractor(vtkRenderWindowInteractor* interactor) override;

    /// Associated TopoDataItem
    TopoDataItem *topoDataItem_;

    bool drawEnabled_;

    /// Current drawing mode
    DrawingMode drawingMode_;
  
    std::vector<std::array<double, 3>> userPath_;
    
    QQuickVTKItem *qquickVTKItem_;

    vtkNew<vtkRenderer> overlayRenderer_;
    vtkNew<vtkActor2D> rubberBandActor_;
    vtkNew<vtkPolyDataMapper2D> rubberBandMapper_;
    vtkNew<vtkPolyData> rubberBandPolyData_;
    vtkNew<vtkCoordinate> transformCoordinate_;
    bool overlayInitialized_ = false;

    /// Last mousebutton-down event position
    int downEventPos_[2];
    
  private:
    DrawInteractorStyle(const DrawInteractorStyle&) = delete;
    void operator=(const DrawInteractorStyle&) = delete;
  };
}   // end namespace

#endif
