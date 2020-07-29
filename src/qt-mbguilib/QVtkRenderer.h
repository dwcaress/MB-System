#ifndef QVTKRENDERER_H
#define QVTKRENDERER_H

#include <QObject>
#include <QQuickFramebufferObject>
#include <QOpenGLFunctions>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkSmartPointer.h>
#include <vtkElevationFilter.h>
#include <vtkPolyDataMapper.h>
#include "GmtGridReader.h"


class QVtkItem;

/**
QVtkRenderer and QVtkItem coordinate with one another to
render VTK scenes within a QQuickItem specified in QML.
A QVtkRenderer object is created by an accompanying QVtkItem object 
and runs in the application's "render" thread; QVtkRenderer
is responsible for setting up the scene in the VTK pipeline,
rendering the scene, and making scene adjustments based on user
inputs (zoom, rotate, pan...) received by its accompanying QVtkItem
running in the GUI thread. 
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

    /// Initialize renderer
    void initialize();

    /// Initilize VTK pipeline; returns true on success, false on error.
    bool initializePipeline(const char *grdFilename);

    /// Initialize OpenGL state
    virtual void initializeOpenGLState();

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

    /// VTK renderer
    vtkSmartPointer<vtkRenderer> renderer_;

    /// VTK render window
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow_;

    /// VTK mouse/key interactor
    vtkSmartPointer<vtkGenericRenderWindowInteractor> renderWindowInteractor_;

    /// Name of associated grid file
    char *gridFilename_;

    /// Latest wheel event
    std::shared_ptr<QWheelEvent> wheelEvent_;

    /// Latest mouse button event
    std::shared_ptr<QMouseEvent> mouseButtonEvent_;

    /// Latest mouse move event
    std::shared_ptr<QMouseEvent> mouseMoveEvent_;

};

#endif // QVTKRENDERER_H
