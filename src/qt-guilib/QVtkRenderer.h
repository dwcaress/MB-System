#ifndef QVTKRENDERER_H
#define QVTKRENDERER_H

#include <QObject>
#include <QQuickFramebufferObject>
#include <QOpenGLFunctions>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkCubeAxesActor.h>
#include <vtkSmartPointer.h>
#include <vtkElevationFilter.h>
#include <vtkPolyDataMapper.h>
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

  class QVtkRenderer : public QQuickFramebufferObject::Renderer,
		       protected QOpenGLFunctions
  {
  public:
    QVtkRenderer();

    /// Create rendering surface
    QOpenGLFramebufferObject * createFramebufferObject(const QSize &size) override;

    /// Copy new user inputs received by the accompanying QVtkItem
    /// running in the GUI thread. The synchronize() function is
    /// called while the GUI thread is blocked following a call
    /// to QVtkItem::update(), so is thread-safe.
    void synchronize(QQuickFramebufferObject *) override;

    /// Render the VTK scene
    void render() override;


  protected:

    /// Display properties copied from QVtkItem
    const DisplayProperties *displayProperties_;
    
    /// Initialize renderer; build VTK pipeline
    void initialize();

    /// Initilize VTK pipeline; returns true on success, false on error.
    bool initializePipeline(const char *grdFilename);

    /// Initialize OpenGL state
    virtual void initializeOpenGLState();

    /// Setup axes
    void setupAxes();
    
    // Item being rendered
    QVtkItem *item_;
    
    /// Flag indicates if rendered scene has been initialized
    bool initialized_;

    /// GMT grid reader
    vtkSmartPointer<GmtGridReader> gridReader_;

    /// Elevation color filter
    vtkSmartPointer<vtkElevationFilter> elevColorizer_;

    /// VTK mapper
    vtkSmartPointer<vtkPolyDataMapper> mapper_;

    /// Grid surface actor
    vtkSmartPointer<vtkActor> surfaceActor_;

    /// Grid axes actor
    vtkSmartPointer<vtkCubeAxesActor> axesActor_;    

    /// VTK renderer
    vtkSmartPointer<vtkRenderer> renderer_;

    /// VTK render window
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow_;

    /// VTK mouse/key interactor
    vtkSmartPointer<vtkGenericRenderWindowInteractor> renderWindowInteractor_;

    /// VTK interactor style
    vtkSmartPointer<PickerInteractorStyle> interactorStyle_;
    
    /// Name of associated grid file
    char *gridFilename_;

    /// Latest wheel event
    std::shared_ptr<QWheelEvent> wheelEvent_;

    /// Latest mouse button event
    std::shared_ptr<QMouseEvent> mouseButtonEvent_;

    /// Latest mouse move event
    std::shared_ptr<QMouseEvent> mouseMoveEvent_;

  };

}

#endif // QVTKRENDERER_H
