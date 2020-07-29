#ifndef QVTKITEM_H
#define QVTKITEM_H

#include <QObject>
#include <QQuickFramebufferObject>
#include "QVtkRenderer.h"

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

    /// Return latest wheel event
    QWheelEvent *latestWheelEvent() {
        return wheelEvent_.get();
    }

    /// Return latest mouse button press event
    QMouseEvent *latestMouseButtonEvent() {
        return mouseButtonEvent_.get();
    }

    /// Return latest mouse move event
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
