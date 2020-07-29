#ifndef QVTKITEM_H
#define QVTKITEM_H

#include <QObject>
#include <QQuickFramebufferObject>
#include "QVtkRenderer.h"

/**
QVtkItem and QVtkRenderer coordinate with one another to
render VTK scenes within a QQuickItem specified in QML. A QVtkItem object
is created when specified in QML, and creates an accompanying 
QVtkRenderer object. The QVtkItem object runs in the GUI thread, is
responsible for accepting user input (mouse zoom, rotate, etc) and passing
those inputs to its accompanying QVtkRenderer object running in the render
thead. 
*/
class QVtkItem : public QQuickFramebufferObject
{
    Q_OBJECT

public:

    QVtkItem();

    /// Create renderer
    QQuickFramebufferObject::Renderer *createRenderer() const override;

    /// Set grid file name
    void setGridFilename(char *gridFilename);

    /// Get name of grid file
    char *getGridFilename() {
        return gridFilename_;
    }

    /// Return latest wheel event.
    /// Called by the QVtkRenderer during QVtkRenderer::synchronize()
    QWheelEvent *latestWheelEvent() {
        return wheelEvent_.get();
    }

    /// Return latest mouse button press event.
    /// Called by the QVtkRenderer during QVtkRenderer::synchronize()  
    QMouseEvent *latestMouseButtonEvent() {
        return mouseButtonEvent_.get();
    }

    /// Return latest mouse move event.
    /// Called by the QVtkRenderer during QVtkRenderer::synchronize()  
    QMouseEvent *latestMouseMoveEvent() {
        return mouseMoveEvent_.get();
    }

protected:

    /// Handle mouse wheel event
    virtual void wheelEvent(QWheelEvent *event) override;

    /// Handle mouse button press event
    virtual void mousePressEvent(QMouseEvent *event) override;

    /// Handle mouse button release event
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

    /// Handle mouse move event
    virtual void mouseMoveEvent(QMouseEvent *event) override;

    /// Name of associated grid file
    char *gridFilename_;

    /// Latest wheel event
    std::shared_ptr<QWheelEvent> wheelEvent_;

    /// Latest mouse button event
    std::shared_ptr<QMouseEvent> mouseButtonEvent_;

    /// Latest mouse move event
    std::shared_ptr<QMouseEvent> mouseMoveEvent_;

};

#endif // QVTKITEM_H
