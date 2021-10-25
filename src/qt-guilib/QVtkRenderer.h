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
#include <vtkSmartPointer.h>
#include <vtkElevationFilter.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkNamedColors.h>
#include "GmtGridReader.h"
#include "DisplayProperties.h"
#include "PickerInteractorStyle.h"

namespace mb_system {

  class QVtkItem;
  
  /**
     QVtkRenderer and QVtkItem coordinate with one another to render VTK scenes 
     within a QQuickItem specified in QML. The QML QVtkItem  instantiates a
     C++ QVtkItem, and QVtkItem::createRenderer() creates a QVtkRenderer object.
     QVtkRenderer code runs in the app's renderer thread, and is responsible for setting 
     up the scene in the VTK pipeline, rendering the scene, and modifying the scene
     based on user inputs such as mouse zoom, rotate, pan, etc.  Those user inputs
     are made available through the QVtkItem interface and are accessed by 
     QVtkRenderer::synchronize(), which is only called when the main thread is blocked.

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
    const DisplayProperties* getDisplayProperties() {
      return (const DisplayProperties *)displayProperties_;
    }

    
    /// Connect pipeline components (static method for standalone testing)
    static bool assemblePipelineTest(mb_system::GmtGridReader *reader,
                                     const char *gridFilename,
                                     vtkElevationFilter *elevColorizer,
                                     vtkRenderer *renderer,
                                     vtkPolyDataMapper *surfaceMapper,
                                     vtkGenericOpenGLRenderWindow *renderWindow,
                                     vtkGenericRenderWindowInteractor *intactor,
                                     PickerInteractorStyle *interactorStyle,
                                     vtkActor *surfaceActor,
                                     vtkCubeAxesActor *axesActor,
                                     const DisplayProperties *properties);

    /// Get item member
    QVtkItem *getItem() {
      return item_;
    }
    
                                                                 
  public slots:

    /// Called when worker thread finishes
    void handleFileLoaded();
    
    
  protected:

    /// Display properties copied from QVtkItem
    const DisplayProperties *displayProperties_;
    
    /// Initilize VTK pipeline member objects, then assemble (connect) ; return
    /// false on error.
    bool initializePipeline(const char *grdFilename);

    /// Connect pipeline components
    bool assemblePipeline();

    /// Initialize renderer; build VTK pipeline
    void initialize();

    /// Initialize OpenGL state
    virtual void initializeOpenGLState();

    /// Setup axes
    static void setupAxes(vtkCubeAxesActor *axesActor,
                          vtkColor3d &axisColor,
                          double *surfaceBounds,
                          double *gridBounds,
                          const char *xUnits, const char *yUnits,
                          const char *zUnits);

    /// If item_ grid filename differs from gridFilename_, copy it and
    /// return true, else return false
    bool gridFilenameChanged(char *filename);
    
    /// Item being rendered
    QVtkItem *item_;
    
    /// Flag indicates if rendered scene has been initialized
    bool initialized_;

    /// GMT grid reader
    vtkSmartPointer<GmtGridReader> gridReader_;

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

    /// Most recent displayed z-scale
    float prevZScale_;
    
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
