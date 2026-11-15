/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_backend_rhi.cc  11/15/2026
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
 * QRhi backend: the views are QRhiWidgets (Metal on macOS, Direct3D 11 on
 * Windows, OpenGL elsewhere, or the API MB_RENDERER names). The core never
 * draws with OpenGL here: a view fills a batch with what the core records
 * (mbv_draw_batch.h) in every frame and draws it, the terrain from the
 * core's node arrays (mbvq_rhi_frame.h, mbvq_rhi_renderer.h).
 *
 * - Every drawing request of the core is deferred to the view's next frame
 *   (defer_draw always returns true, also for views without a widget: a false
 *   return would make the core call OpenGL without a context). Qt merges the
 *   update() calls, so the core's low, high and full requests add no frames.
 * - Terrain picks and the 3D view bounds are found on the CPU
 *   (mbvq_pick_cpu.c), as with the null backend, with the camera the view
 *   draws with: what is picked is what is seen. Input reaches the core only
 *   after the view's first frame, as with OpenGL's isValid().
 * - A view that cannot be drawn (no QRhi on this platform, a QRhi without
 *   instancing, a resource that cannot be made) shows a label saying so and
 *   writes one line per process to stderr; nothing falls back by itself.
 * - The profile and 3D soundings views are null views for now.
 */

#include <QEvent>
#include <QLabel>
#include <QResizeEvent>
#include <QRhiWidget>
#include <QSet>
#include <cmath>
#include <cstdio>
#include <memory>

#include <rhi/qrhi.h>

#include "mbv_input.h"
#include "mbv_render.h"
#include "mbvq_backend.h"
#include "mbvq_bridge.h"
#include "mbvq_input.h"
#include "mbvq_rhi.h"
#include "mbvq_rhi_frame.h"
#include "mbvq_rhi_renderer.h"

namespace mbvq {
namespace rhi_backend {

namespace {

/* the failure line is written once per process */
int failure_lines = 0;
/* callbacks of QRhiWidget removed from a QRhi a view had left (QTBUG-151834) */
int stale_callbacks = 0;

QRhiWidget::Api widget_api(RhiApi api) {
  switch (api) {
  case RhiApi::OpenGL:
    return QRhiWidget::Api::OpenGL;
  case RhiApi::Metal:
    return QRhiWidget::Api::Metal;
  case RhiApi::Vulkan:
    return QRhiWidget::Api::Vulkan;
  case RhiApi::Direct3D11:
    return QRhiWidget::Api::Direct3D11;
  case RhiApi::Direct3D12:
    return QRhiWidget::Api::Direct3D12;
  case RhiApi::Default:
    break;
  }
  /* the platform's API, set explicitly: Metal on Apple, Direct3D 11 on Windows, OpenGL elsewhere */
#if defined(__APPLE__)
  return QRhiWidget::Api::Metal;
#elif defined(_WIN32)
  return QRhiWidget::Api::Direct3D11;
#else
  return QRhiWidget::Api::OpenGL;
#endif
}

const char *widget_api_name(QRhiWidget::Api api) {
  switch (api) {
  case QRhiWidget::Api::Null:
    return "Null";
  case QRhiWidget::Api::OpenGL:
    return "OpenGL";
  case QRhiWidget::Api::Metal:
    return "Metal";
  case QRhiWidget::Api::Vulkan:
    return "Vulkan";
  case QRhiWidget::Api::Direct3D11:
    return "Direct3D 11";
  case QRhiWidget::Api::Direct3D12:
    return "Direct3D 12";
  }
  return "QRhi";
}

/* What the three views share: the QRhi life cycle and the failure label.
   The renderer is made in initialize() for the view's current QRhi and
   dropped when the QRhi changes (a dock floated or docked again) or dies;
   it is only a cache of the core's arrays. */
class RhiView : public QRhiWidget {
 public:
  explicit RhiView(QWidget *parent);
  ~RhiView() override;
  ViewState state() const;

 protected:
  void initialize(QRhiCommandBuffer *cb) override;
  void releaseResources() override;
  void resizeEvent(QResizeEvent *event) override;

  /* the view's renderer for a QRhi, not made yet */
  virtual std::unique_ptr<rhi::ViewRenderer> make_renderer(QRhi *qrhi) = 0;
  /* the core gets the size of the drawing in physical pixels */
  virtual void size_core(QSize pixels) = 0;
  /* a new QRhi: anything tied to the old window goes */
  virtual void new_rhi() {}

  /* the view cannot be drawn: one line per process on stderr, and a label on
     this view */
  void report_failure();
  /* the view draws again: the label goes */
  void hide_failure();

  std::unique_ptr<rhi::ViewRenderer> renderer_;

 private:
  void render_failed();

  QRhi *rhi_ = nullptr; /* the QRhi of renderer_ */
  QSet<QRhi *> seen_;   /* every QRhi this view met that is still alive */
  QLabel *failure_ = nullptr;
  int failures_ = 0;
};

RhiView::RhiView(QWidget *parent) : QRhiWidget(parent) {
  setApi(widget_api(rhi_api()));
  /* renderFailed can come many times: this connection is made once, here */
  connect(this, &QRhiWidget::renderFailed, this, &RhiView::render_failed);
}

RhiView::~RhiView() {
  /* Up to Qt 6.12.0 (QTBUG-151834), a view moved to another window by floating
     or docking its dock leaves the cleanup callback of QRhiWidget in the QRhi of
     the window it left, which would then call this deleted widget when that
     window closes. Remove it from every earlier QRhi still alive, together with
     this view's own callbacks. */
  for (QRhi *r : std::as_const(seen_)) {
    r->removeCleanupCallback(&seen_);
    if (r != rhi()) {
      r->removeCleanupCallback(static_cast<QRhiWidget *>(this));
      stale_callbacks++;
    }
  }
  renderer_.reset();
}

ViewState RhiView::state() const {
  ViewState state;
  state.rhi_view = true;
  state.failure_shown = failure_ != nullptr && !failure_->isHidden();
  state.failures = failures_;
  state.renderer = renderer_ != nullptr;
  return state;
}

void RhiView::releaseResources() { renderer_.reset(); }

void RhiView::initialize(QRhiCommandBuffer *) {
  /* called for the first frame, after a resize, and in another window: the
     resources of a QRhi left behind went with it or are dropped here */
  if (rhi() != rhi_) {
    renderer_.reset();
    new_rhi();
    rhi_ = rhi();
    if (rhi_ != nullptr && !seen_.contains(rhi_)) {
      seen_.insert(rhi_);
      rhi_->addCleanupCallback(&seen_, [this](QRhi *dying) {
        seen_.remove(dying);
        if (dying == rhi_) {
          renderer_.reset();
          rhi_ = nullptr;
        }
      });
    }
  }
  if (rhi_ == nullptr)
    return;
  /* the core's size first: picks and the camera use it */
  size_core(renderTarget()->pixelSize());
  /* the pipelines stay over resizes while the render targets stay compatible */
  if (renderer_ != nullptr && !renderer_->compatible(renderTarget()->renderPassDescriptor()))
    renderer_.reset();
  if (renderer_ == nullptr) {
    std::unique_ptr<rhi::ViewRenderer> renderer = make_renderer(rhi_);
    if (!renderer->create(renderTarget()->renderPassDescriptor(), renderTarget()->sampleCount())) {
      /* tried again at the next initialize() */
      report_failure();
      return;
    }
    renderer_ = std::move(renderer);
  }
  /* a working QRhi again (after a failure, a dock or a lost device) */
  hide_failure();
}

void RhiView::resizeEvent(QResizeEvent *event) {
  QRhiWidget::resizeEvent(event);
  if (failure_ != nullptr)
    failure_->setGeometry(rect());
}

void RhiView::render_failed() {
  renderer_.reset();
  report_failure();
}

void RhiView::report_failure() {
  failures_++;
  const QString text =
      QStringLiteral("mbview-qt: the views could not be drawn with QRhi (%1); MB_RENDERER=legacy-gl draws them with OpenGL")
          .arg(QString::fromLatin1(widget_api_name(api())));
  if (failure_lines == 0) {
    std::fprintf(stderr, "%s\n", qPrintable(text));
    failure_lines++;
  }
  if (failure_ == nullptr) {
    failure_ = new QLabel(this);
    failure_->setAlignment(Qt::AlignCenter);
    failure_->setWordWrap(true);
    failure_->setAutoFillBackground(true);
    failure_->setMargin(12);
  }
  failure_->setText(text);
  failure_->setGeometry(rect());
  failure_->show();
  failure_->raise();
}

void RhiView::hide_failure() {
  if (failure_ != nullptr)
    failure_->hide();
}

/* The map view of an mbview instance. */
class RhiMapView final : public RhiView {
 public:
  RhiMapView(size_t instance, QWidget *parent);
  ~RhiMapView() override;
  /* a drawing the core asked for: in the next frame */
  void request(int rez);
  ViewState state() const;
  RenderStats stats() const;

 protected:
  std::unique_ptr<rhi::ViewRenderer> make_renderer(QRhi *qrhi) override;
  void size_core(QSize pixels) override;
  void new_rhi() override;
  void initialize(QRhiCommandBuffer *cb) override;
  void render(QRhiCommandBuffer *cb) override;
  bool event(QEvent *qevent) override;

 private:
  size_t instance_;
  rhi::MapFrame frame_;
  bool drawn_ = false;         /* a frame was drawn: input may reach the core */
  int buttons_ = 0;            /* mouse buttons down: bit 0 left, 1 middle, 2 right */
  bool continued_ = false;     /* the next frame is one this view asked for to continue uploads */
  bool upload_failed_ = false; /* a tile buffer could not be made: wait for the core's next request */
};

RhiMapView *views[MBVQ_MAX_WINDOWS] = {};

RhiMapView::RhiMapView(size_t instance, QWidget *parent) : RhiView(parent), instance_(instance), frame_(instance) {
  setFocusPolicy(Qt::StrongFocus);
  views[instance_] = this;
}

RhiMapView::~RhiMapView() {
  if (views[instance_] == this)
    views[instance_] = nullptr;
}

void RhiMapView::request(int rez) {
  frame_.request(rez);
  update();
}

ViewState RhiMapView::state() const {
  ViewState state = RhiView::state();
  state.drawn = drawn_;
  state.buttons = buttons_;
  return state;
}

RenderStats RhiMapView::stats() const {
  return renderer_ != nullptr ? static_cast<const rhi::MapRenderer *>(renderer_.get())->stats() : RenderStats();
}

std::unique_ptr<rhi::ViewRenderer> RhiMapView::make_renderer(QRhi *qrhi) {
  return std::make_unique<rhi::MapRenderer>(qrhi, RenderOptions());
}

void RhiMapView::size_core(QSize pixels) { frame_.size_core(pixels.width(), pixels.height()); }

void RhiMapView::new_rhi() {
  /* a release in the old window will not come */
  buttons_ = 0;
}

void RhiMapView::initialize(QRhiCommandBuffer *cb) {
  RhiView::initialize(cb);
  /* a working renderer (a new size or a new QRhi): uploads that failed are
     tried again in the next frame */
  if (renderer_ != nullptr)
    upload_failed_ = false;
}

void RhiMapView::render(QRhiCommandBuffer *cb) {
  const rhi::Clock::time_point start = rhi::Clock::now();
  QRhiResourceUpdateBatch *updates = rhi()->nextResourceUpdateBatch();
  auto *renderer = static_cast<rhi::MapRenderer *>(renderer_.get());
  /* Nothing to draw without a renderer, nor while uploads that failed wait
     for the core's next request (a user action or new data): trying again in
     every frame would loop where the GPU memory ran out. */
  const QSize pixels = renderTarget()->pixelSize();
  if (renderer == nullptr || (upload_failed_ && !frame_.requested())) {
    /* picks use the core's size, also while nothing is drawn */
    if (renderer != nullptr)
      frame_.size_core(pixels.width(), pixels.height());
    cb->beginPass(renderTarget(), Qt::white, {1.0f, 0}, updates);
    cb->endPass();
    return;
  }

  /* Every frame fills the batch, as paintGL() draws in every paint: frames
     from exposures and shows stay right. The core's size comes first, also
     when a window moved to a screen of another ratio changed the pixel size
     at the same logical size. A frame this view asked for only to continue
     uploads reuses the batch while nothing changed (MapFrame::begin). */
  const rhi::Prepared prepared = renderer->frame(&frame_, updates, continued_, pixels);
  continued_ = false;
  if (prepared == rhi::Prepared::Failed) {
    /* a tile buffer could not be made: nothing is drawn; the renderer keeps
       its pipelines and uploads everything again at the next request */
    cb->beginPass(renderTarget(), Qt::white, {1.0f, 0});
    cb->endPass();
    upload_failed_ = true;
    report_failure();
    return;
  }
  if (upload_failed_) {
    upload_failed_ = false;
    hide_failure();
  }

  /* opaque white, as the OpenGL views show it */
  const rhi::Clock::time_point pass = rhi::Clock::now();
  cb->beginPass(renderTarget(), Qt::white, {1.0f, 0}, updates);
  const rhi::Clock::time_point commands = rhi::Clock::now();
  renderer->record(cb, pixels);
  cb->endPass();
  const rhi::Clock::time_point end = rhi::Clock::now();
  /* for measurements: QRhiWidget waits for its frame after render(), so the
     GPU time the QRhi reports now is that of the frame before */
  const double gpu = cb->lastCompletedGpuTime();
  renderer->set_pass_times(rhi::milliseconds(pass, commands), rhi::milliseconds(commands, end),
                           rhi::milliseconds(start, end), gpu > 0.0 ? 1000.0 * gpu : -1.0, rhi::milliseconds(start),
                           rhi::milliseconds(end));
  /* input from now on, once a fill has projected the data */
  drawn_ = drawn_ || frame_.fills() > 0;
  /* uploads wait, or another view was filling: another frame */
  if (prepared == rhi::Prepared::More) {
    continued_ = true;
    update();
  }
}

bool RhiMapView::event(QEvent *qevent) {
  /* a release lost to a grab, a hide or another window: no button is down */
  switch (qevent->type()) {
  case QEvent::UngrabMouse:
  case QEvent::Hide:
  case QEvent::WindowDeactivate:
    buttons_ = 0;
    break;
  default:
    break;
  }
  struct mbv_input_event event;
  if (translate_event(qevent, devicePixelRatioF(), &event)) {
    if (event.type == MBV_INPUT_PRESS)
      buttons_ |= 1 << (event.button - 1);
    else if (event.type == MBV_INPUT_RELEASE)
      buttons_ &= ~(1 << (event.button - 1));
    /* nothing is drawn before the first frame, so nothing can be picked: a
       pick then would project the data outside a drawing */
    if (drawn_)
      mbview_input(instance_, &event);
    return true;
  }
  return RhiView::event(qevent);
}

bool defer_draw(int view, size_t instance, int rez) {
  /* always deferred: there is no OpenGL context to draw in now */
  if (view == MBV_RENDER_MAP && instance < MBVQ_MAX_WINDOWS && views[instance] != nullptr)
    views[instance]->request(rez);
  return true;
}

}  // namespace

void install() {
  /* no contexts to manage; terrain picks and 3D view bounds on the CPU */
  const struct mbv_render_ops ops = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, defer_draw,
                                     mbvq_pick_cpu_terrain, mbvq_pick_cpu_viewbounds};
  mbv_render_set_ops(&ops);
}

QWidget *create_map_view(size_t instance, QWidget *parent) { return new RhiMapView(instance, parent); }

}  // namespace rhi_backend

ViewState view_state(QWidget *view) {
  if (auto *map = dynamic_cast<rhi_backend::RhiMapView *>(view))
    return map->state();
  if (auto *rhi_view = dynamic_cast<rhi_backend::RhiView *>(view))
    return rhi_view->state();
  return ViewState();
}

RenderStats view_stats(QWidget *view) {
  if (auto *map = dynamic_cast<rhi_backend::RhiMapView *>(view))
    return map->stats();
  return RenderStats();
}

int failure_messages() { return rhi_backend::failure_lines; }

int stale_callbacks_removed() { return rhi_backend::stale_callbacks; }

}  // namespace mbvq
