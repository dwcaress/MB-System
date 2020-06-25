/****************************************************************************
 **
 ** Copyright (C) 2015 Klarälvdalens Datakonsult AB, a KDAB Group company.
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
#include <string.h>
#include <QSurfaceFormat>
#include <QQmlContext>
#include "MBQuickView.h"
#include "Camera.h"
#include "SurfaceRenderer.h"
#include "GmtGridSurface.h"


MBQuickView::MBQuickView(const char *qmlResource, QWindow *parent)
  : QQuickView(parent),
    m_camera(new Camera()),
    m_renderer(new SurfaceRenderer(this)),
    m_surface(nullptr),
    m_gridFilename(nullptr)
{
  QSurfaceFormat format;
  format.setMajorVersion(3);
  format.setMinorVersion(3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  format.setSamples(4);
  setFormat(format);

  connect(this, &QQuickWindow::sceneGraphInitialized,
	  this, &MBQuickView::initializeUnderlay,
	  Qt::DirectConnection);

  connect(this, &QQuickWindow::beforeSynchronizing,
	  this, &MBQuickView::synchronizeUnderlay,
	  Qt::DirectConnection);

  connect(this, &QQuickWindow::beforeRendering,
	  this, &MBQuickView::renderUnderlay,
	  Qt::DirectConnection);

  connect(this, &QQuickWindow::sceneGraphInvalidated,
	  this, &MBQuickView::invalidateUnderlay,
	  Qt::DirectConnection);

  connect(m_camera, &Camera::azimuthChanged,
	  this, &QQuickWindow::update);

  connect(m_camera, &Camera::elevationChanged,
	  this, &QQuickWindow::update);

  connect(m_camera, &Camera::distanceChanged,
	  this, &QQuickWindow::update);

  connect(m_camera, &Camera::xOffsetChanged,
	  this, &QQuickWindow::update);

  connect(m_camera, &Camera::yOffsetChanged,
	  this, &QQuickWindow::update);    
  
  setClearBeforeRendering(false);
  setPersistentOpenGLContext(true);


  setResizeMode(SizeRootObjectToView);
  rootContext()->setContextProperty("camera", m_camera);
  /** 
  qDebug() << "set source to qml";
  setSource(QUrl("qrc:///main.qml"));
  *** */
  qDebug() << "set source to qml " << qmlResource;
  setSource(QUrl::fromLocalFile(qmlResource));
  qDebug() << "done with setSource()";
}


void MBQuickView::setQmlSource(const char *qmlResource) {
  qDebug() << "set source to qml";
  setSource(QUrl(qmlResource));
}


void MBQuickView::setGridSurface(QUrl fileURL) {
  if (m_gridFilename) {
    free((void *)m_gridFilename);
  }
  qDebug() << "MBQuickView::setGridSurface to " << fileURL;
  m_gridFilename = (const char *)strdup(fileURL.toLocalFile().toLatin1().data());
  initializeUnderlay();
}


void MBQuickView::initializeUnderlay()
{
  if (!m_gridFilename) {
    qInfo() << "No grid file loaded";
    return;
  }
  
  if (m_surface) {
    delete m_surface;
  }
  
  m_surface = new GmtGridSurface();
  if (!m_surface->build(m_gridFilename)) {
    // Failed to build surface from gridFilename
  }
  
  m_renderer->initialize(m_surface);
  resetOpenGLState();

  // Calculate maximum viewing distance
  float min, max;
  float maxDistance = 10 * m_renderer->surface()->xSpan(&min, &max);
  qDebug() << "initializeUnderlay(): max view distance = " << maxDistance;
  m_camera->setMaxDistance(maxDistance);
  QObject *object = this->findChild<QObject *>("distanceSlider");
  if (!object) {
    qCritical() << "Can't find distanceSlider";
    return;
  }
  qDebug() << "Found distanceSlider";
  object->setProperty("to", maxDistance);
    
}

void MBQuickView::synchronizeUnderlay()
{
  /// Update renderer with current camera parameters
  m_renderer->setView(m_camera->azimuth(), m_camera->elevation(),
		      m_camera->distance(),
		      m_camera->xOffset(), m_camera->yOffset());
}

void MBQuickView::renderUnderlay()
{
  m_renderer->render();
  resetOpenGLState();
}

void MBQuickView::invalidateUnderlay()
{
  m_renderer->invalidate();
  resetOpenGLState();
}
