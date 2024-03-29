#ifndef QVTKRENDERER_H
#define QVTKRENDERER_H

#include <QObject>
#include <QQuickFramebufferObject>
#include <QOpenGLFunctions>
#include <QThread>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkCubeAxesActor.h>
#include <vtkCubeAxesActor2D.h>
#include <vtkSmartPointer.h>
#include <vtkElevationFilter.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkNamedColors.h>
#include "TopoGridReader.h"
#include "DisplayProperties.h"
#include "PickerInteractorStyle.h"

#define SELECTED_POINT_FILE "selectedPoint.txt"

namespace mb_system {

  class QVtkItem;
  
  /**
     QVtkRenderer and QVtkItem coordinate with one another to render VTK scenes 
     within a QQuickItem specified in QML. The QML QVtkItem  instantiates a
     C++ QVtkItem, and QVtkItem::createRenderer() creates a QVtkRenderer object.
     QVtkRenderer code runs in the app's renderer thread, and is responsible 
     for setting up the scene in the VTK pipeline, rendering the scene, and 
     modifying the scene based on user inputs such as mouse zoom, rotate, pan, 
     etc.  Those user inputs are made available through the QVtkItem interface 
     and are accessed by QVtkRenderer::synchronize(), which is only called 
     when the main thread is blocked.

     See https://www.qt.io/blog/2015/05/11/integrating-custom-opengl-rendering-with-qt-quick-via-qquickframebufferobject
  */
  class QVtkRenderer : public QObject,
                       public QQuickFramebufferObject::Renderer,
                       protected QOpenGLFunctions
  {
      Q_OBJECT
    
  public:
    QVtkRenderer();

    /// Create rendering surface
    QOpenGLFramebufferObject * createFramebufferObject(const QSize &size)
      override;

    /// Copy new user inputs received by the accompanying QVtkItem
    /// running in the GUI thread. The synchronize() function is
    /// called while the GUI thread is blocked following a call
    /// to QVtkItem::update(), so is thread-safe.
    void synchronize(QQuickFramebufferObject *) override;

    /// Render the VTK scene
    void render() override;

    /// Set grid filename
    void setGridFilename(char *filename) {
      if (gridFilename_) {
        free((void *)gridFilename_);
      }
      gridFilename_ = strdup(filename);
    }

    /// Set DisplayProperties
    void setDisplayProperties(DisplayProperties *properties) {
      displayProperties_ = properties;
    }

    /// Return pointer to DisplayProperties member
    DisplayProperties* getDisplayProperties() {
      return displayProperties_;
    }

    /// Get item member
    QVtkItem *getItem() {
      return item_;
    }
    
    /// Get grid reader
    TopoGridReader *getGridReader() {
      return gridReader_;
    }

    /// Set picked point coordinates
    void setPickedPoint(double *worldXYZ);

  public slots:

    /// Called when worker thread finishes
    void handleFileLoaded();
    
    
  protected:

    /// Display properties copied from QVtkItem
    mb_system::DisplayProperties *displayProperties_;
    
    /// Initilize VTK pipeline member objects, then assemble (connect) ; return
    /// false on error.
    bool initializePipeline(const char *grdFilename);

    /// Connect pipeline components
    bool assemblePipeline();

    /// Setup axes, using vtkCubeAxesActor2D
    void setupAxes(vtkCubeAxesActor2D *axesActor,
                   vtkNamedColors *namedColors,
                   double *surfaceBounds,
                   double *gridBounds,
                   const char *xUnits, const char *yUnits,
                   const char *zUnits);

    /// Setup axes, using vtkCubeAxesActor
    void setupAxes(vtkCubeAxesActor *axesActor,
                   vtkNamedColors *namedColors,
                   double *surfaceBounds,
                   double *gridBounds,
                   const char *xUnits, const char *yUnits,
                   const char *zUnits);

    
    /// If item_ grid filename differs from gridFilename_, copy it and
    /// return true, else return false
    bool gridFilenameChanged(char *filename);

    /// Assert renderWindow_ as current in response to WindowMakeCurrent event
    void makeCurrentCallback(vtkObject *, unsigned long eid, void *callData);
    
    /// Item being rendered
    QVtkItem *item_;
    
    /// Topo grid reader
    vtkSmartPointer<TopoGridReader> gridReader_;

    /// Elevation color filter
    vtkSmartPointer<vtkElevationFilter> elevColorizer_;

    /// Bathymetry lookup table
    vtkSmartPointer<vtkLookupTable> elevLookupTable_;
    
    /// Transform matrix
    vtkSmartPointer<vtkTransform> transform_;
    
    /// Transform filter
    vtkSmartPointer<vtkTransformFilter> transformFilter_;
    
    /// VTK mapper
    vtkSmartPointer<vtkPolyDataMapper> surfaceMapper_;

    /// Grid surface actor
    vtkSmartPointer<vtkActor> surfaceActor_;

    /// Grid axes actor
    vtkSmartPointer<vtkCubeAxesActor> axesActor_;        

    /// VTK renderer
    vtkSmartPointer<vtkRenderer> renderer_;

    /// VTK render window
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow_;

    /// VTK mouse/key interactor
    vtkSmartPointer<vtkGenericRenderWindowInteractor> windowInteractor_;

    /// VTK interactor style
    vtkSmartPointer<PickerInteractorStyle> interactorStyle_;

    /// VTK named colors
    vtkSmartPointer<vtkNamedColors> namedColors_;
    
    /// Name of associated grid file
    char *gridFilename_;

    /// Latest wheel event
    std::shared_ptr<QWheelEvent> wheelEvent_;

    /// Latest mouse button event
    std::shared_ptr<QMouseEvent> mouseButtonEvent_;

    /// Latest mouse move event
    std::shared_ptr<QMouseEvent> mouseMoveEvent_;

    /// Coordinates of latest selected point
    vtkSmartPointer<vtkPolyData> pickedPoint_;

    /// Indicate whether pickedPoint_ has been selected
    bool pointPicked_;

    bool newPointPicked_;
    
    /// Worker thread to load grid file
    class LoadFileWorker : public QThread {

    public:
      LoadFileWorker(QVtkRenderer &parent);

      /// Return true if OK to render FBO now, else false
      bool okToRender() {
        return okToRender_;
      }
      
      
    protected:
      void run() override;

      /// True if OK to render FBO now, else false
      bool okToRender_;
      
      QVtkRenderer &parent_;
      
    };

    /// Worker thread pointer
    LoadFileWorker *worker_;
    
  };

}

#endif // QVTKRENDERER_H
