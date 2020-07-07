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
#include <unistd.h>
#include <cstdio>
#include <QSurfaceFormat>
#include <QQmlContext>
#include <QQuickWindow>
#include <QRunnable>
#include <QQmlEngine>
#include <QQmlContext>
#include "MBQuickItem.h"
#include "Camera.h"
#include "SurfaceRenderer.h"
#include "GmtGridSurface.h"

/// Initialize static singleton instance
MBQuickItem *MBQuickItem::m_instance = nullptr;

MBQuickItem::MBQuickItem()
  : QQuickItem(),
    m_camera(new Camera()),
    m_renderer(nullptr),
    m_surface(nullptr),
    m_gridFilename(nullptr),
    m_newSurface(false)
{
  fprintf(stderr, "%s:%d:%s: %p %p\n", __FILE__, __LINE__, __func__, g_appEngine, g_rootWindow);

  connect(this, &QQuickItem::windowChanged, this, &MBQuickItem::handleWindowChanged);

  qDebug() << "MBQuickItem::MBQuickItem() - don't conect camera signals to QQuickItem::update()";

  // Find 'camera' property in QML context
  QQmlContext *context = g_appEngine->rootContext();
  context->setContextProperty("camera", m_camera);

  qDebug() << "MBQuickItem::MBQuickItem(): m_renderer=" << m_renderer;
}


void MBQuickItem::handleWindowChanged(QQuickWindow *window) {
  qDebug() << "MBQuickItem::handleWindowChanged()";
  if (window) {
    qDebug() << "MBQuickItem::handleWindowChanged(); connect signals";

    // Invoke synchronizeUnderlay() when main thread is blocked before scene QML graph synchronization
    connect(window, &QQuickWindow::beforeSynchronizing, this, &MBQuickItem::synchronizeUnderlay,
	    Qt::DirectConnection);

    // Clean up when GUI is being destroyed
    connect(window, &QQuickWindow::sceneGraphInvalidated, this, &MBQuickItem::cleanup,
	    Qt::DirectConnection);

    // Trigger repaint when camera properties are changed
    connect(m_camera, &Camera::xOffsetChanged, window, &QQuickWindow::update);

    connect(m_camera, &Camera::yOffsetChanged, window, &QQuickWindow::update);

    connect(m_camera, &Camera::azimuthChanged, window, &QQuickWindow::update);

    connect(m_camera, &Camera::elevationChanged, window, &QQuickWindow::update);

    connect(m_camera, &Camera::distanceChanged, window, &QQuickWindow::update);

    connect(m_camera, &Camera::forceRenderChanged, window, &QQuickWindow::update);

    // Don't clear before QML rendering, since we want surface to "underlay" the GUI, i.e.
    // will draw surface before QML is drawn.
    window->setClearBeforeRendering(false);
    window->setPersistentOpenGLContext(true);
  }
}


void MBQuickItem::cleanup() {
  if (m_renderer) {
    delete m_renderer;
    m_renderer = nullptr;
  }
}


void MBQuickItem::synchronizeUnderlay() {

  qDebug() << "MBQuickItem::synchronizeUnderlay()";

  // This method is called before main thread synchronizes with render thread

  if (m_newSurface) {
    /// A new surface has been created. Delete the current renderer
    qDebug() << "MBQuickItem::sync() - m_newSurface is true";
    if (m_renderer) {
      qDebug() << "MBQuickItem::sync() - delete m_renderer";
      delete m_renderer;
      m_renderer = nullptr;
    }
    m_newSurface = false;
  }

  if (!m_renderer) {
    /// A new surface is available for rendering
    qDebug() << "MBQuickItem::sync() - create renderer";
    m_renderer = new SurfaceRenderer();

    /// Initialize renderer
    qDebug() << "MBQuickItem::sync() - initializeUnderlay()";
    initializeUnderlay();

    // Connect signal so that surface gets rendered before QML is rendered
    qDebug() << "MBQuickItem::sync() - connect SurfaceRenderer::render()";
    connect(window(), &QQuickWindow::beforeRendering, this, &MBQuickItem::renderUnderlay,
	    Qt::DirectConnection);
  }

  /// Update renderer with current camera parameters
  m_renderer->setView(m_camera->azimuth(), m_camera->elevation(),
		      m_camera->distance(),
		      m_camera->xOffset(), m_camera->yOffset());

}




bool MBQuickItem::setGridSurface(QUrl fileURL) {
  if (m_gridFilename) {
    free((void *)m_gridFilename);
  }
  qDebug() << "MBQuickItem::setGridSurface to " << fileURL;

  char *gridFilename = strdup(fileURL.toLocalFile().toLatin1().data());
  GmtGridSurface *surface = new GmtGridSurface();
  if (!surface->build(gridFilename)) {
    // Failed to build surface from gridFilename
    qCritical() << "Failed to build surface from " << gridFilename;
    delete surface;
    free((void*)gridFilename);
    return false;
  }

  // Set grid file name member
  m_gridFilename = (const char *)strdup(gridFilename);
  free((void *)gridFilename);

  if (m_surface) {
    delete m_surface;
  }
  m_surface = surface;

  // Set flag to indicate that new surface has been created; this will be checked
  // by sync() when it's invoked before next sync between main and renderer threads
  m_newSurface = true;

  return true;
}


void MBQuickItem::initializeUnderlay()
{
  if (!m_renderer) {
    m_renderer = new SurfaceRenderer();
  }

  if (!m_gridFilename) {
    qInfo() << "No grid file loaded";
    return;
  }

  if (!m_surface) {
    qInfo() << "No surface has been created";
    return;
  }


  m_renderer->initialize(m_surface);
  window()->resetOpenGLState();

  // Calculate maximum viewing distance
  //  float maxDistance = 10 * m_renderer->surface()->xSpan();
  //  qDebug() << "initializeUnderlay(): max view distance = " << maxDistance;
  setMaxViewDistance();
}


void MBQuickItem::renderUnderlay()
{
  qDebug() << "MBQuickItem::renderUnderlay()";
  m_renderer->render();
  window()->resetOpenGLState();
}


void MBQuickItem::invalidateUnderlay()
{
  m_renderer->invalidate();
  window()->resetOpenGLState();
}

class CleanupJob : public QRunnable
{
public:
  CleanupJob(SurfaceRenderer *renderer) : m_renderer(renderer) { }
  void run() override { delete m_renderer; }
private:
  SurfaceRenderer *m_renderer;
};

bool MBQuickItem::setMaxViewDistance() {
  // Calculate maximum viewing distance
  float min, max;
  float maxDistance = 10 * m_renderer->surface()->xSpan(&min, &max);


  QObject *object = g_rootWindow->findChild<QObject *>("distanceSlider");
  if (!object) {
    qCritical() << "Can't find distanceSlider";
    return false;
  }
  qDebug() << "Found distanceSlider";
  object->setProperty("from", 0.001);
  object->setProperty("to", maxDistance);

  m_camera->setMaxDistance(maxDistance);

  return true;
}


bool MBQuickItem::registerSingleton(int argc, char **argv,
				    QQmlApplicationEngine *appEngine) {

  if (! m_instance) {
    qInfo() << "MBQuickItem::registerSingleton(): Delete existing instance";
    delete m_instance;
  }
  m_instance = new MBQuickItem();

  g_appEngine = appEngine;
  g_rootWindow = qobject_cast<QQuickWindow*>(g_appEngine->rootObjects().value(0));

  bool error = false;
  for (int i = 1; i < argc; i++) {
    if ((!strcmp(argv[i], "-I") && i < argc-1) ||
	(i == argc -1 && argv[i][0] != '-')) {
      char *filename;
      if (i == argc-1) {
	// Last argument is grid file
	filename = argv[i];
      }
      else {
	// Argument following '-I' is grid file
	filename = argv[++i];
      }

      char *fullPath = realpath(argv[i], nullptr);
      if (!fullPath) {
	fprintf(stderr, "Grid file \"%s\" not found\n", filename);
	error = true;
	break;
      }

      QString urlstring("file://" + QString(fullPath));
      QUrl qUrl(urlstring);
      qDebug() << "registerSingleton(): urlstring - " << urlstring
	       << ", qUrl - " << qUrl;

      m_instance->setGridSurface(qUrl);
      free((void *)fullPath);
    }
    else {
      fprintf(stderr, "Unknown/incomplete option: %s\n", argv[i]);
      error = true;
    }
  }
  if (error) {
    delete m_instance;
    m_instance = nullptr;
    fprintf(stderr, "usage: %s [-I gridfile]\n", argv[0]);
    return false;
  }
  QQmlContext *rootContext = g_appEngine->rootContext();
  rootContext->setContextProperty("BackEnd", m_instance);
  return true;
}
