/****************************************************************************
 **
 ** Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company.
 ** Author: Giuseppe D'Angelo
 ** Contact: info@kdab.com
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/

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
    Camera *m_camera;

    /// Calls openGL functions
    SurfaceRenderer *m_renderer;

    /// Current GMT grid surface
    GmtGridSurface *m_surface;

    /// Name of current GMT grid file
    const char *m_gridFilename;
  
  
  };
}

#endif // MBQUICKVIEW_H

