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

using namespace mb_system;

/// Define global references
QQuickWindow *mb_system::g_rootWindow;
QQmlApplicationEngine *mb_system::g_appEngine;


/// Initialize static singleton instance
MBQuickItem *MBQuickItem::instance_ = nullptr;

MBQuickItem::MBQuickItem()
  : QQuickItem(),
    camera_(new Camera()),
    renderer_(nullptr),
    surface_(nullptr),
    gridFilename_(nullptr),
    newSurface_(false)
{
  connect(this, &QQuickItem::windowChanged, this, &MBQuickItem::handleWindowChanged);

  qDebug() << "MBQuickItem::MBQuickItem() - don't conect camera signals to QQuickItem::update()";

  // Find 'camera' property in QML context
  QQmlContext *context = g_appEngine->rootContext();
  context->setContextProperty("camera", camera_);

  qDebug() << "MBQuickItem::MBQuickItem(): renderer_=" << renderer_;
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
    connect(camera_, &Camera::xOffsetChanged, window, &QQuickWindow::update);

    connect(camera_, &Camera::yOffsetChanged, window, &QQuickWindow::update);    

    connect(camera_, &Camera::azimuthChanged, window, &QQuickWindow::update);

    connect(camera_, &Camera::elevationChanged, window, &QQuickWindow::update);

    connect(camera_, &Camera::distanceChanged, window, &QQuickWindow::update);

    connect(camera_, &Camera::forceRenderChanged, window, &QQuickWindow::update);    
    
    // Don't clear before QML rendering, since we want surface to "underlay" the GUI, i.e.
    // will draw surface before QML is drawn.
    window->setClearBeforeRendering(false);
    window->setPersistentOpenGLContext(true);
  }
}


void MBQuickItem::cleanup() {
  if (renderer_) {
    delete renderer_;
    renderer_ = nullptr;
  }
}


void MBQuickItem::synchronizeUnderlay() {

  qDebug() << "MBQuickItem::synchronizeUnderlay()";

  // This method is called before main thread synchronizes with render thread

  if (newSurface_) {
    /// A new surface has been created. Delete the current renderer 
    qDebug() << "MBQuickItem::sync() - newSurface_ is true";
    if (renderer_) {
      qDebug() << "MBQuickItem::sync() - delete renderer_";
      delete renderer_;
      renderer_ = nullptr;
    }    
    newSurface_ = false;
  }
  
  if (!renderer_) {
    /// A new surface is available for rendering
    qDebug() << "MBQuickItem::sync() - create renderer";
    renderer_ = new SurfaceRenderer();
    
    /// Initialize renderer
    qDebug() << "MBQuickItem::sync() - initializeUnderlay()";
    initializeUnderlay();

    // Connect signal so that surface gets rendered before QML is rendered
    qDebug() << "MBQuickItem::sync() - connect SurfaceRenderer::render()";
    connect(window(), &QQuickWindow::beforeRendering, this, &MBQuickItem::renderUnderlay,
	    Qt::DirectConnection);
  }
  
  /// Update renderer with current camera parameters
  renderer_->setView(camera_->azimuth(), camera_->elevation(),
		      camera_->distance(),
		      camera_->xOffset(), camera_->yOffset());
  
}




bool MBQuickItem::setGridSurface(QUrl fileURL) {
  if (gridFilename_) {
    free((void *)gridFilename_);
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
  gridFilename_ = (const char *)strdup(gridFilename);
  free((void *)gridFilename);

  if (surface_) {
    delete surface_;
  }
  surface_ = surface;

  // Set flag to indicate that new surface has been created; this will be checked
  // by sync() when it's invoked before next sync between main and renderer threads
  newSurface_ = true;

  return true;
}


void MBQuickItem::initializeUnderlay()
{
  if (!renderer_) {
    renderer_ = new SurfaceRenderer();
  }
  
  if (!gridFilename_) {
    qInfo() << "No grid file loaded";
    return;
  }

  if (!surface_) {
    qInfo() << "No surface has been created";
    return;
  }
  
    
  renderer_->initialize(surface_);
  window()->resetOpenGLState();

  // Calculate maximum viewing distance
  //  float maxDistance = 10 * renderer_->surface()->xSpan();
  //  qDebug() << "initializeUnderlay(): max view distance = " << maxDistance;
  setMaxViewDistance();
}


void MBQuickItem::renderUnderlay()
{
  qDebug() << "MBQuickItem::renderUnderlay()";
  renderer_->render();
  window()->resetOpenGLState();
}


void MBQuickItem::invalidateUnderlay()
{
  renderer_->invalidate();
  window()->resetOpenGLState();
}

class CleanupJob : public QRunnable
{
public:
  CleanupJob(SurfaceRenderer *renderer) : renderer_(renderer) { }
  void run() override { delete renderer_; }
private:
  SurfaceRenderer *renderer_;
};

bool MBQuickItem::setMaxViewDistance() {
  // Calculate maximum viewing distance
  float min, max;
  float maxDistance = 10 * renderer_->surface()->xSpan(&min, &max);

  
  QObject *object = g_rootWindow->findChild<QObject *>("distanceSlider");
  if (!object) {
    qCritical() << "Can't find distanceSlider";
    return false;
  }
  qDebug() << "Found distanceSlider";
  object->setProperty("from", 0.001);  
  object->setProperty("to", maxDistance);

  camera_->setMaxDistance(maxDistance);

  return true;
}


bool MBQuickItem::registerSingleton(int argc, char **argv,
				    QQmlEngine *qmlEngine) {

  if (! instance_) {
    qInfo() << "MBQuickItem::registerSingleton(): Delete existing instance";
    delete instance_;
  }
  instance_ = new MBQuickItem();

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

      instance_->setGridSurface(qUrl);
      free((void *)fullPath);
    }
    else {
      fprintf(stderr, "Unknown/incomplete option: %s\n", argv[i]);
      error = true;
    }
  }
  if (error) {
    delete instance_;
    instance_ = nullptr;
    fprintf(stderr, "usage: %s [-I gridfile]\n", argv[0]);
    return false;
  }
  QQmlContext *rootContext = qmlEngine->rootContext();
  rootContext->setContextProperty("BackEnd", instance_);
  return true;
}
