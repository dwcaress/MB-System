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
 * - OffscreenRenderer: a map, its profile or the 3D soundings drawn with a
 *   QRhi of its own into an image, without a window, by the frame driver and
 *   the renderer of the view.
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
struct mbv_draw_batch;

namespace mbvq {

/* how the marker shapes are tessellated (mbvq_rhi_glu.h): as Mesa's GLU (the
   SGI sample implementation; the views), or as Apple's GLU, for comparisons
   with the OpenGL views on macOS */
enum class GluVariant { Sgi, Apple };

struct RenderOptions {
  /* the most bytes uploaded in one frame (64 MB); the rest follows in the next frames */
  size_t upload_budget = static_cast<size_t>(64) << 20;
  /* create() of the renderer fails after it made its first resources, as when
     the GPU memory runs out */
  bool fail_create = false;
  /* no buffer can be made, as when the GPU memory runs out: the frame fails
     and draws nothing, and the renderer drops its tiles, the batch's buffers
     and the contours but keeps its pipelines, so that a later frame uploads
     everything again */
  bool fail_buffers = false;
  /* the uniform ring of the batch cannot grow beyond its first 64 slots, as
     when memory runs out: a frame with more drawable commands fails */
  bool fail_slot_growth = false;
  /* ... and cannot be made again at its old size either: the renderer then
     lets go of its pipelines, bindings and ring and makes them again in its
     next frame */
  bool fail_slot_restore = false;
  /* the 3D soundings' positions, colours and profile pairs cannot be read,
     as if the core no longer had the soundings the frame counted: the frame
     fails as when a buffer cannot be made */
  bool fail_reads = false;
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
  /* the frames are drawn as while a map mouse button is down: the drag
     budget and the fill fallback apply (a view takes this from its mouse
     buttons; the fill fallback also applies while a button is down in any
     map view of the process) */
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
  /* the device pixel ratio the core's line widths, point sizes and stipple
     factors (logical pixels) are scaled by: physical = max(1, floor(logical
     x ratio + 0.25)); a view takes its devicePixelRatioF() instead */
  double pixel_ratio = 1.0;
  /* the tessellation of the marker shapes */
  GluVariant glu_variant = GluVariant::Sgi;
  /* while a map mouse button is down (drag, or a button down in any map
     view of the process), a frame fills at the resolution the core asked
     for instead of full resolution once its plots at full resolution would
     take longer than this (ms): the map's last full-resolution plot (the
     core's plot alone, without the 3D pick-mark sizing), twice that for a
     frame that fills twice on the spheroid */
  double fill_fallback_ms = 12.0;
  /* points (the 3D soundings, a batch's POINTS) drawn as quads, as where the
     API has no point size in the vertex shader (Direct3D); taken when the
     renderer or its pipelines are made */
  bool point_quads = false;
  /* the point-size control of the 3D soundings view in logical pixels (0.5
     to 8; 3 draws the core's points as they are); a view takes the
     program's (set_soundings_point_size) */
  double soundings_point_size = 3.0;
  /* soundings per chunk of the soundings' GPU buffers (2^21, the views'),
     smaller in tests, so that small sets take several chunks and frames */
  int soundings_chunk = 1 << 21;
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
  /* ... of them its pipelines, bindings, uniform buffer and render pass (the
     same in every frame), and the buffers of the batch and the marker
     meshes; the rest are tile buffers */
  int fixed_resources = 0;
  int batch_resources = 0;
  /* the last frame drew nothing: the renderer, one of its buffers (tiles,
     batch, contours, uniform ring) or, after a ring it could not restore,
     its pipelines could not be made */
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
     buffer (so a static buffer uploaded in every frame is copied twice per
     frame; a dynamic one written whole in every frame, as the batch's, once,
     since a whole write drops the writes still waiting for the other one) */
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
  /* The map's batch (mbv_draw_batch.h) in the last frame: the device pixel
     ratio its sizes were scaled with; its draws (lines thin and wide,
     points, triangles, marker runs, the terrain and contours; of them the
     draws of points as quads), uniform slots
     and markers; the contour level drawn (MBVQ_REZ_*, 0 none) and its
     vertices. Totals: batch uploads and their bytes, those above 64 MB (one
     batch is never split over frames), contour uploads and bytes, marker
     meshes made, second fills on the spheroid, fills at the requested
     resolution while a button was down in any map view (the 12 ms
     fallback), and the 3D pick-mark sizings before a full-resolution fill. */
  double pixel_ratio = 1.0;
  int draws = 0;
  int thin_draws = 0;
  int wide_draws = 0;
  int marker_draws = 0;
  int point_quad_draws = 0;
  int uniform_slots = 0;
  int markers = 0;
  int contour_rez = 0;
  size_t contour_vertices = 0;
  int batch_uploads = 0;
  size_t batch_bytes = 0;
  size_t batch_largest = 0;
  int batch_over_cap = 0;
  int contour_uploads = 0;
  size_t contour_bytes = 0;
  int meshes = 0;
  int refills = 0;
  int fallback_fills = 0;
  int pick_sizings = 0;
  /* The bytes the batch's buffers (its vertices and instances, the closing
     segments of loops, the quads as triangles and the stipple pairs) and the
     contours hold after the last frame, every copy the QRhi keeps counted
     (dynamic_copies): one per frame in flight on Metal, Vulkan and
     Direct3D 12, the buffer and a copy in host memory on Direct3D 11, one on
     OpenGL and Null. Besides, Qt keeps the latest write of such a buffer
     until a frame on each frame slot has used it. They follow the size of
     the batch and of the contours drawn, and lie outside the memory budget
     of the terrain levels (set_cache_budget). The QRhi's frame slot of the
     last frame (QRhi::currentFrameSlot(); always 0 with one frame in
     flight). The tile levels are static buffers, whose uploads Qt's Metal
     backend keeps for each frame slot until a frame on that slot draws
     them: tile streams uploaded again in consecutive frames (shading,
     exaggeration and colour changes) still pile up there, one upload per
     frame, while something else puts every frame of a view on one frame
     slot (a widget of its window that repaints between them), and the
     uploads still pending for a level no longer drawn stay until that level
     is drawn on that slot again or released. */
  size_t batch_held_bytes = 0;
  size_t contour_held_bytes = 0;
  int dynamic_copies = 1;
  int frame_slot = 0;
  /* the bytes written into the uniform ring in the last frame: the whole
     ring, the slots beyond the commands as zeros, so that Qt keeps one
     pending write of it per frame slot (0 when the frame failed) */
  size_t ring_bytes = 0;
  /* writes of the stipple values alone, for a frame that draws the batch it
     has with a new camera */
  int stipple_uploads = 0;
  /* The 3D soundings view (the profile view has only its batch). Of the last
     frame: the soundings and chunks of the allocation, the soundings whose
     chunks are drawn (in the first upload a prefix) and those drawn as
     points or quads, whether points are quads and the profile lines wide,
     the profile lines' indices (thin) and instances (wide) drawn, the GPU
     bytes of the soundings' buffers, every copy counted. Totals: the
     allocations, the reads of the core's positions, colours and pairs, and
     per stream (positions, colours, index lists, colours per pair) the
     chunks written and their bytes; the swaps of a double-buffered stream. */
  int soundings = 0;
  int sounding_chunks = 0;
  int soundings_shown = 0;
  int soundings_drawn = 0;
  bool point_quads = false;
  bool profile_wide = false;
  int profile_indices = 0;
  int profile_instances = 0;
  size_t sounding_held_bytes = 0;
  bool soundings_waiting = false; /* writes of the soundings wait for later frames */
  int sounding_allocations = 0;
  int sounding_reads[3] = {};
  int sounding_writes[4] = {};
  size_t sounding_bytes[4] = {};
  int sounding_swaps = 0;
  /* the time of the renderer's create() in ms, from its start to its
     return, for measurements; -1 before it. Each create() is that of a new
     renderer, which a view makes at its first frame and again when it needs
     one (for example when it is floated or docked: a new QRhi); the view
     drops the old renderer before, outside this time. A create() that
     failed is timed too where the renderer's statistics are kept (the
     offscreen renderer); a view drops such a renderer, and its statistics
     then give -1 */
  double create_ms = -1.0;
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
  /* Render a batch a test made itself (mbv_draw_batch.h) as the map's batch
     is drawn, without a map, terrain or contours: one frame, with camera
     (projection x modelview) and projection (for commands drawn without the
     camera), OpenGL's convention, column-major. False if the renderer or the
     frame could not be made. */
  bool render_batch(const struct mbv_draw_batch *batch, const double camera[16], const double projection[16],
                    int width, int height, const RenderOptions &options, QImage *image);
  /* Render a frame of the profile view of an instance, as the view draws
     it (its batch, filled in every frame, with the profile's camera). False
     if the renderer or the frame could not be made. */
  bool render_profile(size_t instance, int width, int height, const RenderOptions &options, QImage *image);
  /* Render frames of the 3D soundings view as the view draws them, until
     every write is done or max_frames were rendered; image gets the last.
     A call after another continues the writes left (the first frame of a
     call fills, the others only continue). False if the renderer or a frame
     could not be made. */
  bool render_soundings(int width, int height, const RenderOptions &options, QImage *image, int max_frames = 16);
  /* the batch of the last soundings frame as the core recorded it, without
     the profile lines (null before the first) */
  const struct mbv_draw_batch *soundings_batch() const;
  /* A frame of the QRhi that the renderer does not draw, as a frame of a
     window in which another widget repainted and the map did not (the QRhi
     moves to its next frame slot). False without a QRhi, or before a frame
     made the render target. */
  bool empty_frame();
  /* frames rendered by the last render_map(), render_batch(),
     render_profile() or render_soundings() */
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
  bool drawn = false;         /* it has drawn a frame, so input reaches the core (map, soundings) */
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
  /* the largest texture of the QRhi the view drew with last (0 before its
     first frame, or after that QRhi died), which caps the profile view */
  int texture_size_max = 0;
};
ViewState view_state(QWidget *view);
/* for tests: mark whether a QRhi map view's last fill fell back
   (rhi::MapFrame), as a fill during a drag in another map view would leave
   it; nothing for any other widget */
void set_fell_back(QWidget *view, bool fell_back);
/* the statistics of the renderer of a QRhi view (map, profile or 3D
   soundings), with the times of its last frame (empty for any other view, or
   while it has no renderer) */
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
   beside those is drawn at the finest stride that fits. For tests. The
   budget covers the terrain levels only: the buffers of the maps' batches
   and contours lie outside it (RenderStats::batch_held_bytes and
   contour_held_bytes), as do the uniform rings and the marker meshes. */
void set_cache_budget(size_t bytes);
size_t cache_budget();
/* the bytes it holds now, every copy counted */
size_t cache_bytes();

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_RHI_H */
