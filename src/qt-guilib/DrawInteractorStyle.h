#ifndef DrawInteractorStyle_h
#define DrawInteractorStyle_h
#include "vtkInteractionStyleModule.h" // For export macro
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkRenderer.h"
#include "vtkActor2D.h"
#include "vtkProperty2D.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPolyData.h"
#include "vtkHandleWidget.h"
#include "vtkHandleRepresentation.h"
#include "vtkCutter.h"
#include "vtkClipPolyData.h"
#include "vtkBox.h"
#include "vtkRenderWindowInteractor.h"
#include <QObject>
#include <QQuickVTKItem.h>
#include "Point.h"

class vtkUnsignedCharArray;

namespace mb_system {

  /* **
     Need this subclass to work around a vtkHandleWidget bug which
     retults in null ptr error when 'shift' is pressed
  ** */
  class MyHandleWidget : public vtkHandleWidget {
  public:
    static MyHandleWidget* New();
    vtkTypeMacro(MyHandleWidget, vtkHandleWidget);

    void removeKeyObservers(vtkRenderWindowInteractor* interactor);
  };
  
  class TopoDataItem;
  
  /* **
     Like TrackBallCamera, but user can define path/polygon/etc
     by clicking left mouse button.
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

    void computeElevationProfile(double startPoint[3], double endPoint[3]);
    
    DrawInteractorStyle();
    ~DrawInteractorStyle() override;

    /// Set interactor
    void SetInteractor(vtkRenderWindowInteractor* interactor) override;

    /// Associated TopoDataItem
    TopoDataItem *topoDataItem_;

    /// Current drawing mode
    DrawingMode drawingMode_;

    /// Points in path defined by user mouse
    std::vector<std::array<double, 3>> userPath_;
    
    /// vtk pipeline objects for rendered elevation profile 
    vtkNew<vtkActor> profileActor_;
    vtkNew<vtkPlane> profilePlane_;
    vtkNew<vtkCutter> profileCutter_;
    vtkNew<vtkPolyDataMapper> profileMapper_;
    vtkNew<vtkClipPolyData> profileClipper_;
    vtkNew<vtkBox> profileBox_;

    bool drawEnabled_;
    
    /// Last mousebutton-down event position
    int downEventPos_[2];

    /// To simplify memory management, store HandleWidgets and
    /// HandleRepresentations as vectors of SmartPointers (SmartPointer type
    /// so that vector members are still valid when accessed by
    /// VTK rendering functions.)
    /// As class members, the individual vector elements
    /// will not be deallocated until the vectors are cleared. 
    std::vector<vtkSmartPointer<MyHandleWidget>> pinWidgets_;
    std::vector<vtkSmartPointer<vtkHandleRepresentation>> pinRepresentations_;

    
  private:
    DrawInteractorStyle(const DrawInteractorStyle&) = delete;
    void operator=(const DrawInteractorStyle&) = delete;
  };
}   // end namespace

#endif
