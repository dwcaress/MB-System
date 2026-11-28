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
 * - While a map mouse button is down the terrain is planned within the drag
 *   budget; after the last release the view asks for a frame without it.
 *   The views share their buttons (rhi::set_map_buttons): while a button is
 *   down in any map, a map's fill falls back to the resolution asked for once
 *   the full-resolution plots of its frame would take longer than 12 ms
 *   (mbvq_rhi_frame.h), and when the last button goes up the maps whose last
 *   fill fell back ask for a frame as well.
 * - The core's line widths, point sizes and stipple factors are scaled by
 *   the view's devicePixelRatioF(), never the render target's ratio; the
 *   points of the 3D soundings also by the program's point-size control
 *   (set_soundings_point_size).
 * - The profile view of a map lies in its window's scroll area at the full
 *   size of the drawing, at most as wide as the QRhi's largest texture at the
 *   view's pixel ratio; it is filled in every frame and takes no input, as in
 *   Motif.
 * - The 3D soundings view draws the soundings from buffers written only where
 *   the core's change marks rose (mbvq_rhi_renderer.h), and takes input after
 *   its first frame, as the map does. Dismissed, it lets go of them as soon
 *   as the program has released them.
 * - The map and 3D soundings views let go of their renderers' buffers at once
 *   when they are deleted (mbview_destroy, mb3dsoundings_end), since their
 *   windows, hidden until a safe point, draw no frame that would.
 */

#include <QEvent>
#include <QFocusEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QRhiWidget>
#include <QSet>
#include <algorithm>
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

/* What the three views share: the QRhi life cycle and the failure label,
   and for the GPU-time fallback the timing of their painted frames
   (paintEvent, frameSubmitted) with the tests' fake wait; the map and 3D
   soundings views also share the button helpers. The renderer is made in
   initialize() for the view's current QRhi and dropped when the QRhi
   changes (a dock floated or docked again) or dies; it is only a cache of
   the core's arrays. */
class RhiView : public QRhiWidget {
 public:
  explicit RhiView(QWidget *parent);
  ~RhiView() override;
  virtual ViewState state() const;
  /* the statistics of the view's renderer (empty without one) */
  virtual RenderStats stats() const;
  /* for tests (mbvq::set_gpu_wait): the GPU-time fallback's fake wait in
     ms, handed to the renderer in every frame (< 0: measured) */
  void set_gpu_wait(double wait_ms) { gpu_wait_ms_ = wait_ms; }

 protected:
  void initialize(QRhiCommandBuffer *cb) override;
  void releaseResources() override;
  void resizeEvent(QResizeEvent *event) override;
  /* the paint timed for the GPU-time fallback (BatchRenderer::frame_painted) */
  void paintEvent(QPaintEvent *event) override;

  /* the view's renderer for a QRhi, not made yet */
  virtual std::unique_ptr<rhi::BatchRenderer> make_renderer(QRhi *qrhi) = 0;
  /* the core gets the size of the drawing in physical pixels */
  virtual void size_core(QSize pixels) = 0;
  /* a new QRhi, or the view's QRhi died: anything tied to the old window goes */
  virtual void new_rhi() {}

  /* the view cannot be drawn: one line per process on stderr, and a label on
     this view */
  void report_failure();
  /* the view draws again: the label goes */
  void hide_failure();
  /* The QRhi the view drew with last while it lives, null before the first
     frame and after it died: outside initialize() and render(),
     QRhiWidget::rhi() can name a QRhi that died after the view moved to
     another window. The QRhi of renderer_ whenever there is one. */
  QRhi *view_rhi() const { return rhi_; }
  double gpu_wait_ms() const { return gpu_wait_ms_; }

  /* The mouse buttons of the map and 3D soundings views (bit 0 left, 1
     middle, 2 right). A release lost to a grab, a hide, another window, a
     modal window or a popup: no button is down until a mouse event reports
     one again (lost_release). The buttons follow what each mouse event
     reports: a button it no longer reports was released elsewhere (another
     widget's grab, which sends this view nothing), and a button cleared
     that is still down (a popup closed while it was held) comes back with
     the next mouse event that reports it (reported_buttons). The event's
     own press or release is applied after that, also where buttons()
     disagrees with it, a synthetic event (event_buttons). */
  static bool lost_release(QEvent *qevent);
  static int reported_buttons(QEvent *qevent, int buttons);
  static int event_buttons(const struct mbv_input_event &event, int buttons);

  std::unique_ptr<rhi::BatchRenderer> renderer_;

 private:
  void render_failed();

  QRhi *rhi_ = nullptr; /* the QRhi of renderer_ */
  QSet<QRhi *> seen_;   /* every QRhi this view met that is still alive */
  QLabel *failure_ = nullptr;
  int failures_ = 0;
  double gpu_wait_ms_ = -1.0;
};

RhiView::RhiView(QWidget *parent) : QRhiWidget(parent) {
  setApi(widget_api(rhi_api()));
  /* renderFailed can come many times: this connection is made once, here */
  connect(this, &QRhiWidget::renderFailed, this, &RhiView::render_failed);
  /* the window submitted a frame: the map's drag budget counts the frame's
     wait for its GPU work and the window's composition among its costs,
     and the GPU-time fallback measures the frame painted last */
  connect(this, &QRhiWidget::frameSubmitted, this, [this] {
    if (renderer_ != nullptr)
      renderer_->frame_submitted(rhi::milliseconds(rhi::Clock::now()));
  });
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
  state.texture_size_max = rhi_ != nullptr ? rhi_->resourceLimit(QRhi::TextureSizeMax) : 0;
  return state;
}

RenderStats RhiView::stats() const {
  return renderer_ != nullptr ? renderer_->stats() : RenderStats();
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
          new_rhi();
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
    std::unique_ptr<rhi::BatchRenderer> renderer = make_renderer(rhi_);
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

void RhiView::paintEvent(QPaintEvent *event) {
  /* QRhiWidget ends the offscreen frame after render() and waits for it, so
     the time from the end of render() to the end of the paint is the
     frame's GPU work where the API waits there. Only a paint that ran this
     view's render() counts: Qt's early returns, the view's own, and a grab
     (render() outside a paint) do not. The renderer may be another after
     the paint (a failure, a new QRhi). */
  const double start = rhi::milliseconds(rhi::Clock::now());
  QRhiWidget::paintEvent(event);
  const double end = rhi::milliseconds(rhi::Clock::now());
  if (renderer_ != nullptr && renderer_->stats().render_begin_ms >= start)
    renderer_->frame_painted(start, end);
}

bool RhiView::lost_release(QEvent *qevent) {
  switch (qevent->type()) {
  case QEvent::UngrabMouse:
  case QEvent::Hide:
  case QEvent::WindowDeactivate:
  case QEvent::WindowBlocked:
    return true;
  case QEvent::FocusOut:
    return static_cast<QFocusEvent *>(qevent)->reason() == Qt::PopupFocusReason;
  default:
    return false;
  }
}

int RhiView::reported_buttons(QEvent *qevent, int buttons) {
  switch (qevent->type()) {
  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonDblClick:
  case QEvent::MouseButtonRelease:
  case QEvent::MouseMove: {
    const Qt::MouseButtons down = static_cast<QMouseEvent *>(qevent)->buttons();
    return (down.testFlag(Qt::LeftButton) ? 1 : 0) | (down.testFlag(Qt::MiddleButton) ? 2 : 0) |
           (down.testFlag(Qt::RightButton) ? 4 : 0);
  }
  default:
    return buttons;
  }
}

int RhiView::event_buttons(const struct mbv_input_event &event, int buttons) {
  if (event.type == MBV_INPUT_PRESS)
    return buttons | 1 << (event.button - 1);
  if (event.type == MBV_INPUT_RELEASE)
    return buttons & ~(1 << (event.button - 1));
  return buttons;
}

void RhiView::render_failed() {
  renderer_.reset();
  report_failure();
}

void RhiView::report_failure() {
  failures_++;
  /* MB_RENDERER=legacy-gl is named only where this build has that backend */
  const QString advice = backend_built(Backend::LegacyGL) ? QStringLiteral("MB_RENDERER=legacy-gl draws them with OpenGL")
                                                          : QStringLiteral("this build has no legacy OpenGL drawing");
  const QString text = QStringLiteral("mbview-qt: the views could not be drawn with QRhi (%1); %2")
                           .arg(QString::fromLatin1(widget_api_name(api())), advice);
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

/* Qt frees the buffers a renderer released in a later frame of their QRhi,
   which a hidden window does not draw: now, on a QRhi that is alive (the
   QRhi of a renderer, alive while the renderer is). Metal, Vulkan and
   Direct3D 12 free them in finish(), OpenGL when a frame begins (an empty
   offscreen frame), Direct3D 11 at once; inside a frame of that QRhi a
   later frame of it frees them. */
void run_deferred_releases(QRhi *qrhi) {
  if (qrhi == nullptr || qrhi->isRecordingFrame())
    return;
  if (qrhi->backend() == QRhi::OpenGLES2) {
    QRhiCommandBuffer *cb = nullptr;
    if (qrhi->beginOffscreenFrame(&cb) == QRhi::FrameOpSuccess)
      qrhi->endOffscreenFrame();
  }
  else
    qrhi->finish();
}

/* The map view of an mbview instance. */
class RhiMapView final : public RhiView {
 public:
  RhiMapView(size_t instance, QWidget *parent);
  ~RhiMapView() override;
  /* a drawing the core asked for: in the next frame */
  void request(int rez);
  ViewState state() const override;
  RenderStats stats() const override;
  /* for tests (mbvq::set_fell_back, mbvq_rhi.h) */
  void set_fell_back(bool fell_back) { frame_.set_fell_back(fell_back); }

 protected:
  std::unique_ptr<rhi::BatchRenderer> make_renderer(QRhi *qrhi) override;
  void size_core(QSize pixels) override;
  void new_rhi() override;
  void initialize(QRhiCommandBuffer *cb) override;
  void render(QRhiCommandBuffer *cb) override;
  bool event(QEvent *qevent) override;

 private:
  /* the mouse buttons changed; after the last release, and when the state
     is cleared while the view is shown, a frame without the drag budget */
  void set_buttons(int buttons);
  /* the view's buttons in the state of the process (rhi::set_map_buttons);
     when the last map button of the process goes up, the other views whose
     last fill fell back fill again in their next frame */
  void share_buttons();
  /* a frame the view asks for itself, while it is shown */
  void ask_frame();

  size_t instance_;
  rhi::MapFrame frame_;
  bool drawn_ = false;         /* a frame was drawn: input may reach the core */
  int buttons_ = 0;            /* mouse buttons down: bit 0 left, 1 middle, 2 right */
  bool continued_ = false;     /* the next frame is one this view asked for to continue uploads */
  bool must_fill_ = false;     /* the next frame fills: the one after the last button's release */
  bool upload_failed_ = false; /* a buffer could not be made: wait for the core's next request */
  bool shown_ = true;          /* not hidden: the renderer's latest levels are kept from other views' evictions */
  int requests_ = 0;           /* frames asked for by ask_frame() */
};

RhiMapView *views[MBVQ_MAX_WINDOWS] = {};

RhiMapView::RhiMapView(size_t instance, QWidget *parent) : RhiView(parent), instance_(instance), frame_(instance) {
  setFocusPolicy(Qt::StrongFocus);
  views[instance_] = this;
}

RhiMapView::~RhiMapView() {
  if (views[instance_] == this)
    views[instance_] = nullptr;
  /* a button down here goes with the view */
  buttons_ = 0;
  share_buttons();
  /* deleted while its window lives on hidden until a safe point
     (mbview_destroy): once other views' plans can no longer ask this one
     for frames, the renderer's buffers go now */
  if (renderer_ != nullptr) {
    static_cast<rhi::MapRenderer *>(renderer_.get())->set_frame_request(nullptr);
    QRhi *qrhi = view_rhi();
    renderer_.reset();
    run_deferred_releases(qrhi);
  }
}

void RhiMapView::request(int rez) {
  frame_.request(rez);
  update();
}

ViewState RhiMapView::state() const {
  ViewState state = RhiView::state();
  state.drawn = drawn_;
  state.buttons = buttons_;
  state.fill_due = must_fill_;
  state.frame_requests = requests_;
  state.levels_kept = shown_;
  return state;
}

RenderStats RhiMapView::stats() const {
  return renderer_ != nullptr ? static_cast<const rhi::MapRenderer *>(renderer_.get())->stats() : RenderStats();
}

std::unique_ptr<rhi::BatchRenderer> RhiMapView::make_renderer(QRhi *qrhi) {
  auto renderer = std::make_unique<rhi::MapRenderer>(qrhi, RenderOptions());
  /* For the memory budget: another view evicted tile levels of this one,
     whose GPU memory a frame frees, or let go of levels while this view's
     latest frame was drawn coarser for the budget. A frame reuses the batch
     unless something changed. */
  renderer->set_frame_request([this] {
    if (!must_fill_)
      continued_ = true;
    ask_frame();
  });
  renderer->set_visible(shown_);
  return renderer;
}

void RhiMapView::ask_frame() {
  /* a hidden view draws nothing: the frame comes when it is shown */
  if (!isVisible())
    return;
  requests_++;
  update();
}

void RhiMapView::size_core(QSize pixels) { frame_.size_core(pixels.width(), pixels.height()); }

void RhiMapView::new_rhi() {
  /* a release in the old window will not come; the next frame (this
     paint's, or the first with a new QRhi after the old one died) fills,
     without the drag budget */
  buttons_ = 0;
  share_buttons();
  must_fill_ = true;
  continued_ = false;
}

void RhiMapView::set_buttons(int buttons) {
  const bool released = buttons_ != 0 && buttons == 0;
  buttons_ = buttons;
  share_buttons();
  /* the drag ends in the renderer now, not with the next frame without a
     button: a new press before that frame starts afresh */
  if (released && renderer_ != nullptr)
    renderer_->end_press();
  /* The core asks for no frame after most releases (its last drawing was
     at full resolution), so the frames of a drag drawn within the drag
     budget would stay: the view asks for a frame, which fills and draws the
     terrain at the stride of the zoom without the budget. */
  if (released) {
    must_fill_ = true;
    continued_ = false;
    ask_frame();
  }
}

void RhiMapView::share_buttons() {
  const bool down = rhi::map_buttons_down();
  rhi::set_map_buttons(this, buttons_ != 0);
  if (!down || rhi::map_buttons_down())
    return;
  /* The last map button of the process went up. A map that the core drew
     during the drag at the resolution asked for (the fill fallback) would
     keep that batch until the core asks for another drawing, which it does
     after a release in the navigation, site and route modes
     (mbview_plothighall), but not after a lost release: those maps ask for
     a frame, which fills at full resolution. */
  for (RhiMapView *view : views)
    if (view != nullptr && view != this && view->frame_.fell_back()) {
      view->must_fill_ = true;
      view->continued_ = false;
      view->ask_frame();
    }
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
     uploads reuses the batch while nothing changed (MapFrame::begin), but not
     the frame after a release. While a mouse button is down the terrain is
     planned within the drag budget. A frame is drawn only while the view is
     shown, whose levels are then kept from other views' evictions. */
  frame_.set_drag(buttons_ != 0);
  /* line widths, point sizes and stipple factors follow this view's pixel
     ratio, read in every frame (a window moved to another screen) */
  renderer->set_pixel_ratio(devicePixelRatioF());
  renderer->set_gpu_wait(gpu_wait_ms());
  shown_ = true;
  renderer->set_visible(true);
  const rhi::Prepared prepared = renderer->frame(&frame_, updates, continued_ && !must_fill_, pixels);
  continued_ = false;
  must_fill_ = false;
  if (prepared == rhi::Prepared::Failed) {
    /* a buffer could not be made: nothing is drawn; the renderer uploads
       everything again at the next request, after making its pipelines again
       if a uniform ring it could not restore took them */
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
  /* a release lost to a grab, a hide, another window, a modal window or a
     popup: no button is down until a mouse event reports one again; a hidden
     view keeps no tile level from other views' evictions */
  if (lost_release(qevent))
    set_buttons(0);
  if (qevent->type() == QEvent::Hide || qevent->type() == QEvent::Show) {
    shown_ = qevent->type() == QEvent::Show;
    if (renderer_ != nullptr)
      static_cast<rhi::MapRenderer *>(renderer_.get())->set_visible(shown_);
  }
  /* the buttons each mouse event reports, then its own press or release
     (RhiView) */
  const int buttons = reported_buttons(qevent, buttons_);
  struct mbv_input_event event;
  if (translate_event(qevent, devicePixelRatioF(), &event)) {
    set_buttons(event_buttons(event, buttons));
    /* nothing is drawn before the first frame, so nothing can be picked: a
       pick then would project the data outside a drawing */
    if (drawn_)
      mbview_input(instance_, &event);
    return true;
  }
  if (buttons != buttons_)
    set_buttons(buttons);
  return RhiView::event(qevent);
}

/* The profile view of a map: a QRhiWidget the size of the whole drawing in
   the scroll area of the profile window, at most as wide as the QRhi's
   largest texture at the view's pixel ratio. */
class RhiProfileView final : public RhiView {
 public:
  RhiProfileView(size_t instance, QWidget *parent);
  ~RhiProfileView() override;
  /* The widest and highest drawing whose colour buffer the QRhi can have,
     floor(TextureSizeMax / ratio) logical pixels with the view's
     devicePixelRatioF() (16384 before a QRhi exists): taken in
     initialize(), when the ratio or the screen changes, and by the profile
     window before it sizes the view (cap_profile_view). */
  void cap();

 protected:
  std::unique_ptr<rhi::BatchRenderer> make_renderer(QRhi *qrhi) override;
  void size_core(QSize pixels) override;
  void initialize(QRhiCommandBuffer *cb) override;
  void render(QRhiCommandBuffer *cb) override;
  bool event(QEvent *qevent) override;

 private:
  /* that size, logical pixels */
  int largest_size() const;

  size_t instance_;
  rhi::ProfileFrame frame_;
};

RhiProfileView *profile_views[MBVQ_MAX_WINDOWS] = {};

RhiProfileView::RhiProfileView(size_t instance, QWidget *parent) : RhiView(parent), instance_(instance), frame_(instance) {
  profile_views[instance_] = this;
  cap();
}

RhiProfileView::~RhiProfileView() {
  if (profile_views[instance_] == this)
    profile_views[instance_] = nullptr;
}

int RhiProfileView::largest_size() const {
  const int limit = view_rhi() != nullptr ? view_rhi()->resourceLimit(QRhi::TextureSizeMax) : 16384;
  const double ratio = devicePixelRatioF() > 0.0 ? devicePixelRatioF() : 1.0;
  return std::max(1, static_cast<int>(std::floor(limit / ratio)));
}

void RhiProfileView::cap() {
  const int largest = largest_size();
  if (maximumWidth() != largest || maximumHeight() != largest)
    setMaximumSize(largest, largest);
}

std::unique_ptr<rhi::BatchRenderer> RhiProfileView::make_renderer(QRhi *qrhi) {
  return std::make_unique<rhi::ProfileRenderer>(qrhi, RenderOptions());
}

void RhiProfileView::size_core(QSize pixels) { frame_.size_core(pixels.width(), pixels.height()); }

void RhiProfileView::initialize(QRhiCommandBuffer *cb) {
  RhiView::initialize(cb);
  /* The QRhi's largest texture is known now. A maximum below the view's
     size would resize the view at once, and QRhiWidget paints a resized
     view at once, inside this paint, where no frame can begin: that cap
     comes after the paint. */
  const int largest = largest_size();
  if (width() <= largest && height() <= largest)
    cap();
  else
    QMetaObject::invokeMethod(this, [this] { cap(); }, Qt::QueuedConnection);
}

void RhiProfileView::render(QRhiCommandBuffer *cb) {
  const rhi::Clock::time_point start = rhi::Clock::now();
  QRhiResourceUpdateBatch *updates = rhi()->nextResourceUpdateBatch();
  auto *renderer = static_cast<rhi::ProfileRenderer *>(renderer_.get());
  const QSize pixels = renderTarget()->pixelSize();
  if (renderer == nullptr) {
    cb->beginPass(renderTarget(), Qt::white, {1.0f, 0}, updates);
    cb->endPass();
    return;
  }
  /* filled in every frame (a few hundred vertices); the core's size first,
     also when a window moved to a screen of another ratio */
  renderer->set_pixel_ratio(devicePixelRatioF());
  const rhi::Prepared prepared = renderer->frame(&frame_, updates, pixels);
  if (prepared == rhi::Prepared::Failed) {
    /* a buffer could not be made: nothing is drawn, the next frame tries
       again */
    cb->beginPass(renderTarget(), Qt::white, {1.0f, 0});
    cb->endPass();
    report_failure();
    return;
  }
  hide_failure();
  const rhi::Clock::time_point pass = rhi::Clock::now();
  cb->beginPass(renderTarget(), Qt::white, {1.0f, 0}, updates);
  const rhi::Clock::time_point commands = rhi::Clock::now();
  renderer->record(cb, pixels);
  cb->endPass();
  const rhi::Clock::time_point end = rhi::Clock::now();
  const double gpu = cb->lastCompletedGpuTime();
  renderer->set_pass_times(rhi::milliseconds(pass, commands), rhi::milliseconds(commands, end),
                           rhi::milliseconds(start, end), gpu > 0.0 ? 1000.0 * gpu : -1.0, rhi::milliseconds(start),
                           rhi::milliseconds(end));
  /* another view was filling: another frame */
  if (prepared == rhi::Prepared::More)
    update();
}

bool RhiProfileView::event(QEvent *qevent) {
  /* a window moved to a screen of another ratio: the largest drawing again */
  if (qevent->type() == QEvent::DevicePixelRatioChange || qevent->type() == QEvent::ScreenChangeInternal)
    cap();
  return RhiView::event(qevent);
}

/* the point-size control of the 3D soundings view, logical pixels */
double soundings_point_size = 3.0;

/* The 3D soundings view (one per program). */
class RhiSoundingsView final : public RhiView {
 public:
  explicit RhiSoundingsView(QWidget *parent);
  ~RhiSoundingsView() override;
  /* a drawing the core asked for: in the next frame */
  void request();
  ViewState state() const override;
  /* the program released its soundings while the view is hidden (Dismiss):
     no frame comes, so what the view holds of them goes now */
  void release();

 protected:
  std::unique_ptr<rhi::BatchRenderer> make_renderer(QRhi *qrhi) override;
  void size_core(QSize pixels) override;
  void new_rhi() override;
  void initialize(QRhiCommandBuffer *cb) override;
  void render(QRhiCommandBuffer *cb) override;
  bool event(QEvent *qevent) override;

 private:
  /* the mouse buttons changed: the camera press follows them (the middle
     or right button down and the left up); when it ends, a frame */
  void set_buttons(int buttons);

  rhi::SoundingsFrame frame_;
  bool drawn_ = false;         /* a frame was filled: input may reach the core */
  bool continued_ = false;     /* the next frame is one this view asked for to continue writes */
  /* a buffer could not be made, or the soundings could not be read: wait
     for the core's next request */
  bool upload_failed_ = false;
  int buttons_ = 0;            /* mouse buttons down: bit 0 left, 1 middle, 2 right */
  bool camera_press_ = false;  /* the middle or right button down and the left up */
  int requests_ = 0;           /* frames asked for when a camera press ended */
};

RhiSoundingsView *soundings_view = nullptr;

RhiSoundingsView::RhiSoundingsView(QWidget *parent) : RhiView(parent) {
  setFocusPolicy(Qt::StrongFocus);
  soundings_view = this;
}

RhiSoundingsView::~RhiSoundingsView() {
  if (soundings_view == this)
    soundings_view = nullptr;
  /* deleted while its window lives on hidden until a safe point
     (mb3dsoundings_end): the renderer's buffers go now */
  if (renderer_ != nullptr) {
    QRhi *qrhi = view_rhi();
    renderer_.reset();
    run_deferred_releases(qrhi);
  }
}

void RhiSoundingsView::request() {
  frame_.request();
  update();
}

ViewState RhiSoundingsView::state() const {
  ViewState state = RhiView::state();
  state.drawn = drawn_;
  state.buttons = buttons_;
  state.camera_press = camera_press_;
  state.frame_requests = requests_;
  return state;
}

std::unique_ptr<rhi::BatchRenderer> RhiSoundingsView::make_renderer(QRhi *qrhi) {
  return std::make_unique<rhi::SoundingsRenderer>(qrhi, RenderOptions());
}

void RhiSoundingsView::size_core(QSize pixels) { frame_.size_core(pixels.width(), pixels.height()); }

void RhiSoundingsView::new_rhi() {
  /* the next frame fills and writes everything again; a release in the old
     window will not come, and that frame draws every sounding */
  continued_ = false;
  buttons_ = 0;
  camera_press_ = false;
}

void RhiSoundingsView::set_buttons(int buttons) {
  buttons_ = buttons;
  const bool press = (buttons & 6) != 0 && (buttons & 1) == 0;
  const bool ended = camera_press_ && !press;
  camera_press_ = press;
  /* the press ends in the renderer now, also where no frame without it
     comes before the next press (a release and a new press before the
     next paint, a frame that failed, a hidden view) */
  if (ended && renderer_ != nullptr)
    renderer_->end_press();
  /* The core asks for no drawing after a camera button's release, so the
     last frames of a press, which the GPU-time fallback may have drawn with
     fewer soundings, would stay: the view asks for a frame, which draws
     every sounding. A hidden view draws nothing; its next frame comes when
     it is shown. Edits ask the core for their drawing themselves. */
  if (ended && isVisible()) {
    requests_++;
    update();
  }
}

void RhiSoundingsView::initialize(QRhiCommandBuffer *cb) {
  RhiView::initialize(cb);
  if (renderer_ != nullptr)
    upload_failed_ = false;
}

void RhiSoundingsView::render(QRhiCommandBuffer *cb) {
  const rhi::Clock::time_point start = rhi::Clock::now();
  QRhiResourceUpdateBatch *updates = rhi()->nextResourceUpdateBatch();
  auto *renderer = static_cast<rhi::SoundingsRenderer *>(renderer_.get());
  const QSize pixels = renderTarget()->pixelSize();
  /* Nothing to draw without a renderer, nor while a frame that failed (a
     buffer that could not be made, or soundings that could not be read)
     waits for the core's next request: trying again in every frame would
     loop where the GPU memory ran out or the read keeps failing. */
  if (renderer == nullptr || (upload_failed_ && !frame_.requested())) {
    /* picks use the core's size, also while nothing is drawn */
    if (renderer != nullptr)
      frame_.size_core(pixels.width(), pixels.height());
    cb->beginPass(renderTarget(), Qt::white, {1.0f, 0}, updates);
    cb->endPass();
    return;
  }

  /* The core's size first, then the fill (unless this frame only continues
     writes and nothing changed), the soundings written where their marks
     rose, at the view's pixel ratio and the program's point size. */
  renderer->set_pixel_ratio(devicePixelRatioF());
  renderer->set_point_size(soundings_point_size);
  renderer->set_gpu_wait(gpu_wait_ms());
  /* during a camera press the GPU-time fallback may draw fewer soundings */
  frame_.set_drag(camera_press_);
  const rhi::Prepared prepared = renderer->frame(&frame_, updates, continued_, pixels);
  continued_ = false;
  if (prepared == rhi::Prepared::Failed) {
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

  /* opaque white, as the OpenGL view clears */
  const rhi::Clock::time_point pass = rhi::Clock::now();
  cb->beginPass(renderTarget(), Qt::white, {1.0f, 0}, updates);
  const rhi::Clock::time_point commands = rhi::Clock::now();
  renderer->record(cb, pixels);
  cb->endPass();
  const rhi::Clock::time_point end = rhi::Clock::now();
  const double gpu = cb->lastCompletedGpuTime();
  renderer->set_pass_times(rhi::milliseconds(pass, commands), rhi::milliseconds(commands, end),
                           rhi::milliseconds(start, end), gpu > 0.0 ? 1000.0 * gpu : -1.0, rhi::milliseconds(start),
                           rhi::milliseconds(end));
  /* input from now on, once the view is sized and filled */
  drawn_ = drawn_ || frame_.fills() > 0;
  /* writes wait, or another view was filling: another frame */
  if (prepared == rhi::Prepared::More) {
    continued_ = true;
    update();
  }
}

void RhiSoundingsView::release() {
  if (isVisible() || !frame_.release() || renderer_ == nullptr)
    return;
  static_cast<rhi::SoundingsRenderer *>(renderer_.get())->release_soundings();
  run_deferred_releases(view_rhi());
}

bool RhiSoundingsView::event(QEvent *qevent) {
  /* the buttons as the map view follows them (RhiView), for the camera
     press; never in the maps' state of the process (rhi::set_map_buttons) */
  if (lost_release(qevent))
    set_buttons(0);
  const int buttons = reported_buttons(qevent, buttons_);
  struct mbv_input_event event;
  if (translate_event(qevent, devicePixelRatioF(), &event)) {
    set_buttons(event_buttons(event, buttons));
    /* nothing is drawn before the first frame, so nothing can be picked, and
       the view's size is not known yet */
    if (drawn_)
      mb3dsoundings_input(&event);
    return true;
  }
  if (buttons != buttons_)
    set_buttons(buttons);
  return RhiView::event(qevent);
}

bool defer_draw(int view, size_t instance, int rez) {
  /* always deferred: there is no OpenGL context to draw in now */
  if (view == MBV_RENDER_MAP && instance < MBVQ_MAX_WINDOWS && views[instance] != nullptr)
    views[instance]->request(rez);
  else if (view == MBV_RENDER_PROFILE && instance < MBVQ_MAX_WINDOWS && profile_views[instance] != nullptr)
    profile_views[instance]->update();
  else if (view == MBV_RENDER_SOUNDINGS && soundings_view != nullptr)
    soundings_view->request();
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

QWidget *create_profile_view(size_t instance, QWidget *parent) { return new RhiProfileView(instance, parent); }

QWidget *create_soundings_view(QWidget *parent) { return new RhiSoundingsView(parent); }

void cap_profile_view(QWidget *view) {
  if (auto *profile = dynamic_cast<RhiProfileView *>(view))
    profile->cap();
}

void release_soundings_view(QWidget *view) {
  if (auto *soundings = dynamic_cast<RhiSoundingsView *>(view))
    soundings->release();
}

void set_soundings_point_size(double logical_px) {
  soundings_point_size = logical_px;
  if (soundings_view != nullptr)
    soundings_view->update();
}

}  // namespace rhi_backend

ViewState view_state(QWidget *view) {
  if (auto *rhi_view = dynamic_cast<rhi_backend::RhiView *>(view))
    return rhi_view->state();
  return ViewState();
}

void set_fell_back(QWidget *view, bool fell_back) {
  if (auto *map = dynamic_cast<rhi_backend::RhiMapView *>(view))
    map->set_fell_back(fell_back);
}

void set_gpu_wait(QWidget *view, double wait_ms) {
  if (auto *rhi_view = dynamic_cast<rhi_backend::RhiView *>(view))
    rhi_view->set_gpu_wait(wait_ms);
}

RenderStats view_stats(QWidget *view) {
  if (auto *rhi_view = dynamic_cast<rhi_backend::RhiView *>(view))
    return rhi_view->stats();
  return RenderStats();
}

int failure_messages() { return rhi_backend::failure_lines; }

int stale_callbacks_removed() { return rhi_backend::stale_callbacks; }

}  // namespace mbvq
