/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_backend_gl.cc  11/7/2026
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
/*
 * Legacy OpenGL backend: the map view is a QOpenGLWidget with a 2.1
 * compatibility context, in which the mbview drawing functions run
 * unchanged.
 *
 * - The core draws only inside paintGL(). A plot request from anywhere
 *   else is queued (defer_draw) and painted in order, so that
 *   mbview_plotlowhigh() gives a low then a high resolution paint.
 * - Mouse and key events reach the core inside a pick scope: the context
 *   is current and a private framebuffer without multisampling is bound,
 *   because mbview_findpointrez() draws colour-coded triangles and reads
 *   them back. The widget's own framebuffer is bound again afterwards.
 * - data->width and data->height are physical pixels.
 */

#include <QApplication>
#include <QEvent>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QSurfaceFormat>
#include <cmath>
#include <deque>
#include <memory>

#include "mbv_gl.h"
#include "mbv_input.h"
#include "mbv_render.h"
#include "mbvq_backend.h"
#include "mbvq_bridge.h"
#include "mbvq_input.h"

namespace mbvq {
namespace gl_backend {

namespace {

class GlMapView final : public QOpenGLWidget {
 public:
  GlMapView(size_t instance, QWidget *parent);
  ~GlMapView() override;
  /* queue a drawing at resolution rez; false if it must be drawn now */
  bool defer(int rez);

 protected:
  void initializeGL() override;
  void resizeGL(int width, int height) override;
  void paintGL() override;
  bool event(QEvent *qevent) override;

 private:
  int pixel_width() const { return static_cast<int>(std::lround(width() * devicePixelRatioF())); }
  int pixel_height() const { return static_cast<int>(std::lround(height() * devicePixelRatioF())); }
  void deliver(const struct mbv_input_event &event);
  /* free the OpenGL objects of this view while its context still exists */
  void release_gl();

  size_t instance_;
  std::deque<int> queue_;
  bool painting_ = false;
  std::unique_ptr<QOpenGLFramebufferObject> pick_fbo_;
};

GlMapView *views[MBVQ_MAX_WINDOWS] = {};

GlMapView::GlMapView(size_t instance, QWidget *parent) : QOpenGLWidget(parent), instance_(instance) {
  setFocusPolicy(Qt::StrongFocus);
  views[instance_] = this;
}

GlMapView::~GlMapView() {
  views[instance_] = nullptr;
  release_gl();
}

void GlMapView::initializeGL() {
  connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, [this] { release_gl(); }, Qt::DirectConnection);
}

void GlMapView::release_gl() {
  if (!pick_fbo_)
    return;
  makeCurrent();
  pick_fbo_.reset();
  doneCurrent();
}

bool GlMapView::defer(int rez) {
  if (painting_)
    return false;
  if (queue_.empty() || queue_.back() != rez)
    queue_.push_back(rez);
  update();
  return true;
}

void GlMapView::resizeGL(int width, int height) {
  (void)width;
  (void)height;
  mbvq_c_resize(instance_, pixel_width(), pixel_height());
}

void GlMapView::paintGL() {
  int rez = mbvq_c_lastdrawrez(instance_);
  if (!queue_.empty()) {
    rez = queue_.front();
    queue_.pop_front();
  }
  if (rez == MBVQ_REZ_NONE)
    rez = MBVQ_REZ_LOW;
  glViewport(0, 0, pixel_width(), pixel_height());
  painting_ = true;
  mbvq_c_plot(instance_, rez);
  painting_ = false;
  if (!queue_.empty())
    update();
}

bool GlMapView::event(QEvent *qevent) {
  struct mbv_input_event event;
  if (translate_event(qevent, devicePixelRatioF(), &event)) {
    deliver(event);
    return true;
  }
  return QOpenGLWidget::event(qevent);
}

void GlMapView::deliver(const struct mbv_input_event &event) {
  /* nothing is drawn before the first paint, so nothing can be picked */
  if (!isValid())
    return;
  makeCurrent();
  const int width = pixel_width();
  const int height = pixel_height();
  if (!pick_fbo_ || pick_fbo_->size() != QSize(width, height)) {
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::Depth);
    format.setSamples(0);
    pick_fbo_ = std::make_unique<QOpenGLFramebufferObject>(width, height, format);
  }
  pick_fbo_->bind();
  glViewport(0, 0, width, height);
  mbview_input(instance_, &event);
  pick_fbo_->release();
  /* release() can leave framebuffer 0 bound */
  context()->functions()->glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
  doneCurrent();
}

bool defer_draw(int view, size_t instance, int rez) {
  /* the profile and 3D soundings views come in later steps: never draw them */
  if (view != MBV_RENDER_MAP)
    return true;
  if (instance >= MBVQ_MAX_WINDOWS || views[instance] == nullptr)
    return true;
  return views[instance]->defer(rez);
}

}  // namespace

void configure() {
  QSurfaceFormat format;
  format.setVersion(2, 1);
  format.setProfile(QSurfaceFormat::CompatibilityProfile);
  format.setDepthBufferSize(24);
  format.setSamples(0);
  QSurfaceFormat::setDefaultFormat(format);
#ifdef _WIN32
  QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif
}

void install() {
  /* The contexts belong to the QOpenGLWidgets: create, destroy, make
     current and present need nothing, and picks draw into a private
     framebuffer, so there is no read buffer to switch. */
  const struct mbv_render_ops ops = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, defer_draw};
  mbv_render_set_ops(&ops);
}

QWidget *create_map_view(size_t instance, QWidget *parent) { return new GlMapView(instance, parent); }

}  // namespace gl_backend
}  // namespace mbvq
