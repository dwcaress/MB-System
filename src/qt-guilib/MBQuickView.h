#ifndef MBQUICKVIEW_H
#define MBQUICKVIEW_H

#include <QQuickView>
#include "GmtGridSurface.h"

namespace mb_system {
  
  class SurfaceRenderer;
  class Camera;

  /**
     MBQuickView connects GUI thread with render thread by responding to signals
     emitted by QML renderer, and invoking renderer for non-QML "underlay" OpenGL code at appropriate
     times. Based on D'Angelo's MyQuickView.
  */
  class MBQuickView : public QQuickView
  {
    Q_OBJECT
  
  public:
    explicit MBQuickView(const char *qmlResource, QWindow *parent = 0);


    /// Set grid surface from data in specified file. This function may be
    /// invoked by QML code, e.g. from File->Open menu item
    void setGridSurface(QUrl fileURL);


    /// Set QML file resource
    void setQmlSource(const char *qmlResource);
    
  protected:

    /// Create underlay surface, initialize underlay renderer
    void initializeUnderlay();

    void synchronizeUnderlay();

    void renderUnderlay();

    /// Free resources associated with underlay
    void invalidateUnderlay();

    /// Camera object that views surface
    Camera *camera_;

    /// Calls openGL functions
    SurfaceRenderer *renderer_;

    /// Current GMT grid surface
    GmtGridSurface *surface_;

    /// Name of current GMT grid file
    const char *gridFilename_;
  
  
  };
}

#endif // MBQUICKVIEW_H

