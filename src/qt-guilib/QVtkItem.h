#ifndef QVTKITEM_H
#define QVTKITEM_H

#include <QObject>
#include <QString>
#include <QQuickFramebufferObject>
#include <qqml.h>
#include "QVtkRenderer.h"
#include "DisplayProperties.h"

namespace mb_system {
  /**
     DEPRECATED: superseded by TopoDataItem/QQuickVTKItem.

     QVtkRenderer and QVtkItem coordinate with one another to render VTK scenes 
     within a QVtkItem declared in QML. QVtkItem is registered in the QML
     system by a call to qmlRegisterType() within an application's main() 
     function, and thus a QML declaration of QVtkItem instantiates a C++ 
     QvtkItem, which in turn instantiates a QVtkRenderer 
     (QVtkItem::createRenderer()).
     QVtkRenderer code runs in the app's renderer thread, and is responsible 
     for setting up the scene in the VTK pipeline, rendering the scene, and 
     modifying the scene based on user inputs such as mouse zoom, rotate, pan, 
     etc.  Those user inputs are made available through the QVtkItem interface 
     and are accessed by QVtkRenderer::synchronize(), which is only called 
     when the main thread is blocked.

     See https://www.qt.io/blog/2015/05/11/integrating-custom-opengl-rendering-with-qt-quick-via-qquickframebufferobject
  */
  class QVtkItem : public QQuickFramebufferObject
  {
    Q_OBJECT

    Q_PROPERTY(QString pickedPoint READ getPickedPoint
               WRITE setPickedPoint NOTIFY pickedPointChanged)

    /// Indicate whether app task is busy
    Q_PROPERTY(bool busy READ getAppBusy WRITE setAppBusy NOTIFY busyChanged)

    Q_PROPERTY(QVtkItem::EditState editState READ getEditState
	       WRITE setEditState
	       NOTIFY editStateChanged)

    Q_PROPERTY(int testInt READ getTestInt WRITE setTestInt
	       NOTIFY testIntChanged)

  public:
    enum class EditState : int {
				ViewOnly,
				EditRoute,
				EditPoints,
				EditOverlay
    };

    Q_ENUM(EditState)


    QVtkItem();

    /// Create and return renderer.
    /// Called by Qt framework, overrides
    /// QQuickFramebufferObject::createRenderer()
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
      return wheelEvent_;
    }

    /// Return latest mouse button press event.
    /// Called by the QVtkRenderer during QVtkRenderer::synchronize()  
    QMouseEvent *latestMouseButtonEvent() {
      return mouseButtonEvent_;
    }

    /// Return latest mouse move event.
    /// Called by the QVtkRenderer during QVtkRenderer::synchronize()  
    QMouseEvent *latestMouseMoveEvent() {
      return mouseMoveEvent_;
    }

    /// Get display properties
    DisplayProperties *displayProperties() {
      return &displayProperties_;
    }

    /// Toggle axes display
    void showAxes(bool show) {
      displayProperties_.showAxes(show);
      displayProperties_.changed(true);
    }

    /// Set site file
    void setSiteFile(char *siteFile) {
      displayProperties_.siteFile(siteFile);
      displayProperties_.changed(true);
    }

    
    /// Vertical exaggeration
    void setVerticalExagg(float verticalExagg) {
      qDebug() << "setVerticalExagg() " << verticalExagg;
      displayProperties_.verticalExagg(verticalExagg);
      displayProperties_.changed(true);
    }

    /// Set topo colormap scheme; return true if colorMapName corresponds
    /// to a supported colorMap, else return false.
    bool setColorMapScheme(const char *colorMapname);

    /// Clear DisplayProperites changed flag
    void clearPropertyChangedFlag() {
      displayProperties_.changed(false);
    }
    
  signals:

    /// User picked a point on the vtk surface
    void pickedPointChanged(QString msg);

    /// App busy state changed
    void busyChanged(bool busy);

    /// Edit state changed
    void editStateChanged(int state);

    /// Test int state changed
    void testIntChanged(int val);
    
  public slots:

  public:
    
    /// Set user-picked point coordinates
    void setPickedPoint(QString msg);

    /// Get string representation of picked point
    QString getPickedPoint();

    /// Set app busy status
    void setAppBusy(bool busy);

    /// Get app busy status; return true if busy, else false
    bool getAppBusy();

    /// Get edit statte
    EditState getEditState() {
      return editState_;
    }

    /// Set edit state
    void setEditState(EditState state) {
      editState_ = state;
      std::cout << "setEditState() to " << (int )state << "\n";
      emit editStateChanged((int )state);
    }

    ///  TEST 
    void setTestInt(int val) {
      testInt_ = val;
      std::cout << "setTestInt() to " << val << "\n";
      emit testIntChanged(val);
    }

    /// TEST
    int getTestInt() {
      return testInt_;
    }

    
  protected:

    /// Display properties, e.g. visible axes, etc.
    DisplayProperties displayProperties_;

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
    QWheelEvent *wheelEvent_;

    /// Latest mouse button event
    QMouseEvent *mouseButtonEvent_;

    /// Latest mouse move event
    QMouseEvent *mouseMoveEvent_;

    /// Latest user-picked point coordinates
    QString pickedCoords_;

    /// Indicate to gui whether app task is busy
    bool appTaskBusy_;


    /// Editing state
    EditState editState_;

    /// TEST 
    int testInt_;
    
    
  };


}

#endif // QVTKITEM_H
