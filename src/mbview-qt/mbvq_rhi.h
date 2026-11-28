/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_rhi.h  11/15/2026
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
 * The QRhi backend of mbview-qt for tests and comparisons, without any QRhi
 * type, so that a test needs neither the private Gui module nor the QRhi
 * headers (built where MBVQ_HAVE_RHI is defined):
 * - RenderOptions: the renderer's switches. They are set only here, never
 *   read from the environment.
 * - OffscreenRenderer: a map drawn with a QRhi of its own into an image,
 *   without a window, by the frame driver and the renderer of the map view.
 * - view_state(): what a QRhi view of the backend shows of its state, and
 *   view_stats(): what its renderer did, with the times of its last frame.
 * - set_cache_budget(): the GPU memory budget of the terrain levels of every
 *   map of the process.
 */

#ifndef MBVIEW_QT_MBVQ_RHI_H
#define MBVIEW_QT_MBVQ_RHI_H

#include <cstddef>
#include <memory>

class QImage;
class QWidget;

namespace mbvq {

struct RenderOptions {
  /* the most bytes uploaded in one frame (64 MB); the rest follows in the next frames */
  size_t upload_budget = static_cast<size_t>(64) << 20;
  /* create() of the renderer fails after it made its first resources, as when
     the GPU memory runs out */
  bool fail_create = false;
  /* no tile buffer can be made, as when the GPU memory runs out: the frame
     fails and draws nothing, and the renderer drops its tiles but keeps its
     pipelines, so that a later frame uploads everything again */
  bool fail_buffers = false;
  /* the terrain colours as the core's floats (16 bytes per node) instead of
     bytes (4), for comparisons with OpenGL without the 8-bit rounding of the
     node colours; taken when the renderer is made */
  bool float_terrain_colours = false;
  /* draw each terrain triangle in a colour that names it instead of the
     terrain colours: red + 256 green + 65536 blue - 1 is 2 (i (n_rows - 1) + j)
     for triangle A of the cell (i, j) and one more for B; every node is
     prepared at stride 1 for it, in the view or not */
  bool triangle_ids = false;
  /* the terrain at stride 1 in every frame, for comparisons: no stride of the
     zoom, and no budget makes it coarser */
  bool force_stride1 = false;
  /* tiles outside the view are neither prepared nor drawn; false plans and
     draws every tile, at the stride of the tiles in the view (comparisons) */
  bool culling = true;
  /* the frames are drawn as while a map mouse button is down, so that the
     drag budget applies (a view takes this from its mouse buttons) */
  bool drag = false;
  /* the drag budget's time for preparing and gathering the nodes of a frame
     in ms; < 0: 1000/60 ms less the previous frame's other costs (during a
     drag those of the latest frame at the stride, if larger) and a margin,
     at least 4 ms (the views) */
  double prep_budget_ms = -1.0;
  /* the whole cost of a lattice node in ns (preparing, gathering and its
     share of the index list), where its colours are made (node_ns_colour)
     and where only z or nothing is computed (node_ns_z), in place of the
     measured moving averages; < 0: the averages */
  double node_ns_colour = -1.0;
  double node_ns_z = -1.0;
};

/* what the renderer did; totals since the renderer was made, except where
   "last frame" is said */
struct RenderStats {
  int frames = 0;            /* frames rendered */
  int fills = 0;             /* frames that filled the batch (the others reused it) */
  int fill_rez = 0;          /* resolution of the last fill (MBVQ_REZ_*) */
  bool terrain_slot = false; /* the last batch has the terrain slot */
  bool depth_test = false;   /* ... drawn with the depth test */
  int groups = 0;            /* group commands of the last batch */
  int tiles = 0;             /* tiles of the grid */
  int tiles_drawn = 0;       /* last frame */
  int tiles_waiting = 0;     /* last frame: tiles left for the next frames */
  size_t triangles = 0;      /* last frame */
  int tiles_prepared = 0;    /* tile levels prepared */
  int uploads_positions = 0; /* tile level streams uploaded */
  int uploads_colours = 0;
  int uploads_indices = 0;
  size_t bytes_positions = 0;
  size_t bytes_colours = 0;
  size_t bytes_indices = 0;
  size_t bytes_largest_frame = 0; /* the most bytes uploaded in one frame */
  int resources = 0;              /* QRhi resources the renderer holds now */
  /* the last frame drew nothing: the renderer or a tile buffer could not be made */
  bool failed = false;
  int held_after_failure = 0; /* resources a renderer held right after its create() failed */
  /* The terrain's levels in the last frame: the stride drawn (1, 2, 4 ... 128;
     0 if none), that of the zoom (1 where a cell covers a pixel or more), the
     one planned (another than the stride drawn while uploads are split over
     frames), the tiles with data in the view and those left out; whether the
     drag budget or the memory budget made the stride coarser than the zoom's,
     and whether stride 128 was drawn above the memory budget. */
  int stride = 0;
  int stride_zoom = 0;
  int stride_target = 0;
  int tiles_visible = 0;
  int tiles_culled = 0;
  bool drag_budget = false;
  bool memory_budget = false;
  bool over_memory_budget = false;
  /* the drag budget of the last frame's planned stride for preparing and
     gathering nodes in ms (-1: no map mouse button down), the other costs it
     came from (those of the frame before, or during a drag the latest of a
     frame at that stride if larger), and the last frame's preparing and
     gathering, estimated and measured, both with the time it took the
     tiles' bounds (bounds_ms) */
  double budget_ms = -1.0;
  double outside_ms = 0.0;
  double estimated_ms = 0.0;
  double work_ms = 0.0;
  double bounds_ms = 0.0;
  /* GPU bytes of this renderer's tile levels and of those of every renderer
     of the process, every copy a QRhi keeps counted (Metal keeps one per
     frame in flight, two: buffer_copies), with bytes evicted from a renderer
     until its next frame frees them, so that the process can hold more than
     the budget while a hidden renderer keeps evicted bytes; tile levels of
     this renderer evicted so far */
  size_t resident_bytes = 0;
  size_t cache_bytes = 0;
  int buffer_copies = 1;
  int evictions = 0;
  /* The last frame's times in ms, for measurements only: disjoint parts of
     the frame on the CPU, taken with one steady clock, so that their sum is
     at most render_ms. */
  double fill_ms = 0.0;    /* the core recorded the batch (0 when the frame reused it) */
  double marks_ms = 0.0;   /* the change marks, the grid's version and the frame's input read */
  double prepare_ms = 0.0; /* the core prepared the nodes of the tiles planned (mbview_prepare_nodes) */
  /* the rest of the plan (the tiles' bounds and culling, the stride, the
     evictions, index lists, nodes gathered), the uploads and the uniforms */
  double gather_ms = 0.0;
  /* the render pass begun with the frame's resource updates; on Metal this
     only queues the buffer uploads, which the batch copied in gather_ms */
  double upload_ms = 0.0;
  /* the drawing commands recorded and the pass ended; on Metal with the
     copies of the queued uploads into the buffers, made when a buffer is
     bound: Qt keeps each buffer once per frame in flight, copies into the
     current frame's one now and into the other when a later frame binds the
     buffer (so a buffer uploaded in every frame is copied twice per frame) */
  double record_ms = 0.0;
  double render_ms = 0.0; /* the whole frame on the CPU, up to the end of the pass */
  /* where the last frame's render() began and ended, in ms since the epoch
     of the same steady clock (std::chrono::steady_clock): for a caller that
     times the paint around render(), to tell the waits before and after it */
  double render_begin_ms = 0.0;
  double render_end_ms = 0.0;
  /* the GPU time of the last frame the QRhi completed
     (QRhiCommandBuffer::lastCompletedGpuTime), read while this frame is
     recorded, so that of the frame before (a frame is waited for only after
     it was recorded); -1 where the QRhi reports none (Metal reports it;
     other APIs only with QRhi::EnableTimestamps) */
  double gpu_ms = -1.0;
};

class OffscreenRenderer {
 public:
  enum class Api { Null, Metal };
  explicit OffscreenRenderer(Api api);
  ~OffscreenRenderer();
  OffscreenRenderer(const OffscreenRenderer &) = delete;
  OffscreenRenderer &operator=(const OffscreenRenderer &) = delete;

  /* false if the QRhi could not be made (no Metal device, or not on Apple) */
  bool valid() const;
  const char *backend_name() const;

  /* Render frames of the map of an instance, width x height physical pixels,
     until every upload is done or max_frames were rendered; image (may be
     null) gets the last frame, top row first. Each frame runs the steps of a
     view's frame (MapRenderer::frame): a frame after the first is one that
     continues uploads, and reuses the batch unless something changed. False
     if the renderer or a frame could not be made. */
  bool render_map(size_t instance, int width, int height, const RenderOptions &options, QImage *image,
                  int max_frames = 16);
  /* frames rendered by the last render_map() */
  int last_frames() const;

  const RenderStats &stats() const;
  /* drop the renderer and its GPU resources: the next frame makes them again
     from the core's arrays */
  void release();
  /* delete the QRhi while the renderer holds resources: the renderer lets
     them go through the QRhi's cleanup callback, without a crash */
  void destroy_rhi();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

/* what a view of the QRhi backend shows of its state */
struct ViewState {
  bool rhi_view = false;      /* a view of the QRhi backend */
  bool failure_shown = false; /* its label says it cannot be drawn */
  bool drawn = false;         /* it has drawn a frame, so input reaches the core */
  int buttons = 0;            /* map mouse buttons down: bit 0 left, 1 middle, 2 right */
  int failures = 0;           /* failures it handled (renderFailed, a failed create()) */
  bool renderer = false;      /* it holds a renderer */
  /* the map view's next frame fills, without the drag budget (the frame
     after the last mouse button's release, or after the button state was
     cleared); frames the view asked for itself, for a release or the memory
     budget (update() while shown); and whether its latest tile levels are
     kept from other views' evictions (while shown) */
  bool fill_due = false;
  int frame_requests = 0;
  bool levels_kept = false;
};
ViewState view_state(QWidget *view);
/* the statistics of the renderer of a QRhi map view, with the times of its
   last frame (empty for any other view, or while it has no renderer) */
RenderStats view_stats(QWidget *view);
/* how often the failure line was written to stderr: once per process */
int failure_messages();
/* how often a view's destructor removed QRhiWidget's own cleanup callback
   from a QRhi the view had left, by floating or docking (QTBUG-151834) */
int stale_callbacks_removed();

/* The GPU memory budget of the terrain levels of every map of the process
   (512 MB), shared by the views and the offscreen renderers: beyond it the
   least recently drawn levels are evicted, never those another visible map
   drew in its latest frame or uploads, and a frame whose levels do not fit
   beside those is drawn at the finest stride that fits. For tests. */
void set_cache_budget(size_t bytes);
size_t cache_budget();
/* the bytes it holds now, every copy counted */
size_t cache_bytes();

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_RHI_H */
