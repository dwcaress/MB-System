/*--------------------------------------------------------------------
 *    The MB-system:  WcdView.cc  1/11/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#include "WcdView.h"

#include <vector>

#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QPainter>

namespace wcd {

WcdView::WcdView(QWidget *parent) : QOpenGLWidget(parent) { setMinimumSize(400, 300); }

WcdView::~WcdView() { releaseGL(); }

/* the GL resources belong to this widget's context */
void WcdView::releaseGL() {
  if (gl_ == nullptr)
    return;
  makeCurrent();
  gl_.reset();
  doneCurrent();
}

void WcdView::initializeGL() {
  /* a dock that is floated or docked again moves this widget to a new context: the old resources
     go with the old context, and the mesh must be sent to the new one */
  connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &WcdView::releaseGL);
  gl_ = std::make_unique<wcd::SwathGL>();
  gl_->init();
  plot_.requestMesh();
}

void WcdView::paintGL() {
  /* the painter must know of the OpenGL calls before it: in a new context it would otherwise draw text as blocks */
  QPainter painter(this);
  painter.beginNativePainting();
  QOpenGLFunctions *f = context()->functions();
  f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  f->glClear(GL_COLOR_BUFFER_BIT);
  if (plot_.meshChanged()) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    plot_.buildMesh(vertices, indices);
    gl_->upload(vertices, indices);
  }
  plot_.drawGL(*gl_, size(), devicePixelRatioF());
  painter.endNativePainting();
  plot_.drawOverlay(painter, size());
}

void WcdView::mousePressEvent(QMouseEvent *event) {
  double across, depth;
  if (event->button() == Qt::LeftButton && plot_.toData(event->position(), size(), across, depth))
    emit clicked(across, depth);
  QOpenGLWidget::mousePressEvent(event);
}

}  // namespace wcd
