#ifndef MBQUICKITEM_H
#define MBQUICKITEM_H

#include <QQuickItem>
#include <QQmlApplicationEngine>
#include "GmtGridSurface.h"

namespace mb_system {

  class SurfaceRenderer;
  class Camera;

  /**
     MBQuickItem connects GUI thread with render thread by responding to signals
     emitted by QML renderer. It encapsulates a SurfaceRenderer object, which renders
     a non-QML 3D surface using "native" OpenGL calls. The SurfaceRenderer drawing is 
     rendered first, and underlays QML GUI elements.
     This class inherits QQuickItem so that it may be incoporated by 
     ApplicationWindow (which can have a MenuBar).
     Based on D'Angelo's MyQuickView example:
     https://www.kdab.com/integrate-opengl-code-qt-quick-2-applications-part-2/
  */
  class MBQuickItem : public QQuickItem
  {
    Q_OBJECT

  
  public:
    explicit MBQuickItem();

    /// Build GMT grid surface from data in specified file. If successful,
    /// marks newSurface_ flag, which is checked the next time sync() runs,
    /// before the next QML rendering. This function can be
    /// invoked by QML code, e.g. from File->Open menu item
    Q_INVOKABLE bool setGridSurface(QUrl fileURL);

    /// Create and register singleton, processing command line args as needed.
    /// Return true on success, else false
    static bool registerSingleton(int argc, char **argv, QQmlEngine *qmlEngine);				   

  public slots:

    /// Called on QQuickWindow::beforeSynchronizing signal,
    // while main thread is blocked.
    /// Create, initialze and connect surface renderer to beforeRender
    // signal if new surface
    /// has been created; copy camera parameter values to renderer.
    void synchronizeUnderlay();
  
    /// Delete surface renderer
    void cleanup();

  
  protected:

    /// Create underlay surface, initialize underlay renderer
    void initializeUnderlay();


    /// Free resources associated with underlay
    void invalidateUnderlay();

    /// Set maximum viewing distance on QML GUI. Return true on success,
    /// false on error
    bool setMaxViewDistance();


    void testUpdate();
  
    /// Camera object that views surface
    mb_system::Camera *camera_;

    /// Calls openGL functions to render surface
    SurfaceRenderer *renderer_;

    /// Current GMT grid surface
    mb_system::GmtGridSurface *surface_;

    /// Name of current GMT grid file
    const char *gridFilename_;

    /// Indicates when new surface has been created; flag is checked in sync()
    /// method before each main-renderer synchronization, and new renderer
    /// created when new surface has been created.
    bool newSurface_;
  
    /// Singleton instance
    static MBQuickItem *instance_;
	

  protected slots:
    /// Connect sync() method with beforeSynchronizing signal, cleanup()
    /// method with sceneGraphInvalidated signal
    void handleWindowChanged(QQuickWindow *window);

    /// Invoke renderer's render() method
    void renderUnderlay();


  };

  extern QQuickWindow *g_rootWindow;
  extern QQmlApplicationEngine *g_appEngine;
}

#endif // MBQUICKITEM_H

