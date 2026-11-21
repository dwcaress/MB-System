/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_rhi_model.h  11/15/2026
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
 * The QRhi backend of mbview-qt, model layer: what the renderer decides
 * without any Qt GUI or QRhi type, so that tests check it on real mbview
 * instances without drawing.
 *
 * - The fill policy: the resolution at which a frame fills the map's batch.
 * - The terrain plan: the terrain of the map in tiles of 128 x 128 cells
 *   (129 x 129 nodes, the far edge shared with the next tile). A tile has a
 *   level for each stride 1, 2, 4 ... 128, anchored at node 0: the lattice of
 *   every stride-th node, with a compact copy for the GPU (positions, 3
 *   floats per node; colours, RGBA, 4 bytes per node; the triangles of the
 *   lattice's full cells as 16-bit indices, with the core's A/B split). A
 *   frame draws one stride for every tile (no cracks): the stride of the
 *   zoom, which keeps a cell under a pixel, made coarser only by the drag
 *   budget and the levels the GPU-time fallback marked while a map mouse
 *   button is down, and by the memory budget. Tiles
 *   outside the view are left out. The plan reads the core's change marks
 *   (mbv_marks.h) after each fill and says which levels must be prepared
 *   (mbview_prepare_nodes), gathered and uploaded within an upload budget per
 *   frame; a level is drawn only when every stream of it is as new as the
 *   marks. A level that does not fit in a frame's uploads is completed over
 *   the next frames while the frame draws a coarser stride that is complete.
 * - The terrain cache: the GPU memory of the levels of every map of the
 *   process under one budget, the least recently drawn levels evicted first.
 * - The style: the physical sizes of the line widths, point sizes and
 *   stipple factors the core asks for in logical pixels.
 * - The batch plan: the commands of a view's batch (mbv_draw_batch.h) as
 *   the draws of the renderer, in stream order.
 * - The soundings plan: the soundings of the 3D soundings view in chunks of
 *   GPU buffers, what each holds and what a frame writes.
 */

#ifndef MBVIEW_QT_MBVQ_RHI_MODEL_H
#define MBVIEW_QT_MBVQ_RHI_MODEL_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "mbv_draw_batch.h"
#include "mbvq_bridge.h"

namespace mbvq {
namespace rhi {

/* the clock of the times in the renderer's statistics (RenderStats); the
   drag budget also measures with it */
using Clock = std::chrono::steady_clock;
/* milliseconds from one time of that clock to another */
inline double milliseconds(Clock::time_point from, Clock::time_point to) {
  return std::chrono::duration<double, std::milli>(to - from).count();
}
/* milliseconds of a time of that clock since the clock's epoch */
inline double milliseconds(Clock::time_point at) {
  return std::chrono::duration<double, std::milli>(at.time_since_epoch()).count();
}

/* The resolution at which a frame fills the map's batch: full resolution,
   unless contours are shown and those of full resolution are not made yet;
   then the resolution asked for (the pending request, else the resolution
   drawn last, NONE counting as LOW), so that the whole-grid contour pass at
   full resolution runs only when the core asks for full resolution, as with
   OpenGL. pending is MBVQ_REZ_* or -1 for none. */
int fill_resolution(int pending, int lastdrawrez, bool contours_shown, bool full_contours_made);
/* The same, with the fallback while a mouse button is down (button_down: in
   the map view itself or in any map view of the process,
   rhi::map_buttons_down): once the frame's plots at full resolution would
   take longer than fallback_ms (full_ms: the map's last full-resolution
   plot, the core's plot alone without the 3D pick-mark sizing, twice that
   for a frame that fills twice on the spheroid; < 0 for none yet), a frame
   fills at the resolution asked for, so that costly overlays (swath bounds
   draped at stride 1 in 3D) do not slow a drag. */
constexpr double kFillFallbackMs = 12.0;
int fill_resolution(int pending, int lastdrawrez, bool contours_shown, bool full_contours_made, bool button_down,
                    double full_ms, double fallback_ms = kFillFallbackMs);

/* cells per side of a terrain tile */
constexpr int kTileCells = 128;
/* the levels of a tile: strides 1, 2, 4 ... 128, anchored at node 0 */
constexpr int kLevels = 8;
constexpr int level_stride(int level) { return 1 << level; }

/* the upload budget per frame (64 MB): more is uploaded in later frames */
constexpr size_t kUploadBudget = static_cast<size_t>(64) << 20;
/* the GPU memory of the tile levels of all maps of the process (512 MB), by
   default; every copy a QRhi keeps of a buffer counts */
constexpr size_t kCacheBudget = static_cast<size_t>(512) << 20;

/* The drag budget: while a map mouse button is down, a frame aims at 60 Hz.
   The time left for preparing and gathering nodes is a frame of 60 Hz, less
   the previous frame's measured cost outside its preparing and gathering
   (during a drag the latest such cost of a frame at the stride in question,
   if larger, since uploads, recording and the GPU wait grow with the
   stride's bytes), less a margin for the wait before the paint and the
   spread of the frames, and at least the minimum. */
constexpr double kDragFrameMs = 1000.0 / 60.0;
constexpr double kDragMarginMs = 2.0;
constexpr double kDragMinimumMs = 4.0;
double drag_budget_ms(double outside_ms);

/* The GPU-time fallback. During a view's press (the map: a map mouse button
   down; the 3D soundings: a camera press, the middle or right button down
   and the left up) a frame is GPU-slow when its total (paint start to the
   window's frameSubmitted) exceeds a frame of 60 Hz less the drag margin,
   the paint's wait after render() is at least kGpuMinimumMs, and the total
   without that wait stays under the same mark (else the CPU or the
   composition fills the frame, which no level shortens). Each zoom's level
   (the map's stride of the zoom; the soundings have one) has a rule of its
   own (GpuFallback): kGpuSlowFrames GPU-slow frames in a row at a level of
   that zoom mark the level there for the rest of the press, and its next
   frames draw a coarser level; a step to a level coarser than the one last
   marked there must cut the wait of the frames that marked it by
   kGpuGainShare or more, or that zoom keeps its level. */
constexpr double kGpuMinimumMs = 4.0;
constexpr int kGpuSlowFrames = 2;
constexpr double kGpuGainShare = 0.1;

/* The APIs whose offscreen frames wait for the GPU in endOffscreenFrame():
   Metal, Vulkan and Direct3D 12. On OpenGL and Direct3D 11 that wait is the
   driver's CPU work for the frame, which no coarser level shortens, so the
   fallback acts there only with a test's fake times (fake). */
enum class WaitApi { Null, OpenGL, Direct3D11, Direct3D12, Metal, Vulkan };
bool gpu_wait_counts(WaitApi api, bool fake);

/* The rule of the GPU-time fallback for one view, over the levels 0 to
   coarsest (finer to coarser), one for each zoom's level (0 to kLevels - 1:
   the map's stride of the zoom; the soundings have only zoom 0), each on
   its own, so that waits are compared only at one scale of the drawing and
   what one zoom found never changes another's levels. At a zoom, GPU-slow
   frames in a row at one level mark it (frames at other zooms neither count
   nor reset there); a step to a level coarser than the one last marked
   there is judged by the waits that marked that one, and a step without the
   gain stops that zoom (stuck) at the level reached; a frame at a finer
   level (where 6b's drag budget or the memory budget admits one again) is
   no step and counts as any other frame. The coarsest level is never
   marked, but a step into it is judged. Under constant waits a zoom so ends
   at the next level the budgets admit past the finest one they admit there
   (one level past its own where they admit every level; they can skip a
   level, since one with nothing to prepare costs nothing in 6b's drag
   test), as long as they admit no finer level again in the press: a finer
   level they admit after two GPU-slow frames at a coarser one is marked
   too, without a judged step, so budgets that relax in such steps can take
   the zoom further, up to the coarsest level. The caller feeds the frames
   of a press only, and calls start() at each press. */
class GpuFallback {
 public:
  explicit GpuFallback(int coarsest) : coarsest_(coarsest) { start(); }
  /* a press starts: no level marked, nothing counted, at every zoom */
  void start();
  /* a frame of the press drawn at level, its paint's wait and its total
     (ms), at the zoom's level (0 to kLevels - 1, others clamped); skipped:
     its wait holds uploads (GPU copies), so it neither counts nor resets */
  void frame(int level, double wait_ms, double total_ms, bool skipped = false, int zoom = 0);
  /* the levels marked at a zoom, bit l for level l */
  unsigned marks(int zoom = 0) const;
  /* the finest level not marked at a zoom */
  int level(int zoom = 0) const;
  /* a step at a zoom did not cut the wait: no more marks there in this
     press */
  bool stuck(int zoom = 0) const;
  /* a GPU-slow frame, by its wait and total */
  static bool slow(double wait_ms, double total_ms);

 private:
  /* the rule at one zoom's level */
  struct Zoom {
    unsigned marks = 0;
    int count_level = -1;     /* the level of the slow frames counted */
    int count = 0;            /* GPU-slow frames in a row at it */
    double count_wait = 0.0;  /* their summed wait */
    double reference = -1.0;  /* the mean wait of the frames that marked the last level (-1: none) */
    int reference_level = -1; /* ... and that level */
    bool stuck = false;
  };
  static int zoom_index(int zoom);

  int coarsest_;
  Zoom zooms_[kLevels];
};

/* The stride of the zoom: 1 where a stride-1 cell covers kCellPixels
   physical pixels or more, otherwise the largest power of two up to 128 that
   keeps a cell under it. The level of that stride for the size of a cell in
   physical pixels (infinite: level 0). */
constexpr double kCellPixels = 1.0;
int zoom_level(double cell_pixels);

/* What the camera (projection x modelview, OpenGL's convention,
   column-major) makes of a box of display coordinates: whether any of it is
   inside the six planes of the clip volume (Gribb and Hartmann), and the
   most physical pixels one display unit can cover inside it, for a
   displacement in x and y and one in z: the largest singular values of the
   screen mapping's Jacobian where the box is seen, at its nearest depth. */
class ViewGeometry {
 public:
  ViewGeometry(const double camera[16], int width, int height);
  bool visible(const double lo[3], const double hi[3]) const;
  /* false where the scale is unbounded: a box reaching the near plane of a
     perspective camera; horizontal and vertical are the pixels per unit of
     x and y and of z, any of a displacement in any direction */
  bool scale(const double lo[3], const double hi[3], double *horizontal, double *vertical, double *any) const;

 private:
  /* the largest singular values of the Jacobian times the clip w at the
     normalised device coordinates nx, ny */
  void singular_values(double nx, double ny, double *horizontal, double *vertical, double *any) const;

  double m_[16];
  double planes_[6][4];
  bool affine_ = true;
  double half_width_ = 0.0;
  double half_height_ = 0.0;
};

/* the state of the core a frame reads after its fill */
struct TerrainInput {
  size_t instance = 0;
  struct mbvq_grid grid = {};
  /* the change marks (mbview_getmarks) and the version of the whole grid
     (mbview_getnodeversion over every node) */
  uint64_t position = 0;
  uint64_t color = 0;
  uint64_t mask = 0;
  uint64_t version = 0;
  /* the block grid of the node versions (mbview_getmarkblocks) */
  int block_nodes = 0;
  int nblock_i = 0;
  int nblock_j = 0;
  /* the camera: projection x modelview of mbview_camera(), OpenGL's
     convention, column-major; and the size of the drawing in physical
     pixels, 0 when not known */
  double camera[16] = {};
  int width = 0;
  int height = 0;
};

/* how a frame plans the terrain */
struct PlanOptions {
  /* the most bytes uploaded in a frame */
  size_t upload_budget = kUploadBudget;
  /* stride 1 in every frame (comparisons): no zoom stride, no budget makes
     it coarser, and while uploads are split over frames the tiles complete
     at stride 1 are drawn and the others wait */
  bool force_stride1 = false;
  /* false: every tile is planned and drawn, also outside the view, at the
     stride the tiles in the view give (comparisons) */
  bool culling = true;
  /* a map mouse button is down: the drag budget applies */
  bool drag = false;
  /* the drag budget's time for preparing and gathering nodes (ms), with the
     tiles' bounds taken in the frame; per stride level where the frame's
     other costs depend on the stride (< 0: budget_ms) */
  double budget_ms = kDragFrameMs;
  static_assert(kLevels == 8, "level_budget_ms starts with one -1 per level");
  double level_budget_ms[kLevels] = {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0};
  /* the cost of preparing and gathering a lattice node (ns) with colours
     and without them, in place of the measured moving averages (tests);
     < 0: the averages */
  double node_ns_colour = -1.0;
  double node_ns_z = -1.0;
  /* per zoom's level, the levels the GPU-time fallback marked there in
     this press (bit l for level l): while dragged, the stride passes over
     the marks of the zoom's level the plan finds for the frame */
  unsigned gpu_marks[kLevels] = {};
};

/* what one tile level uploads in a frame; positions and colours have one
   entry per lattice node (node (a, b) of the level at a * nj + b), indices
   three per triangle */
struct TileUpload {
  int tile = 0;
  int level = 0;
  bool positions = false;
  bool colours = false;
  bool indices = false;
  std::vector<float> xyz;
  std::vector<unsigned char> rgba;
  std::vector<float> rgba_float; /* the colours as floats instead (set_float_colours) */
  std::vector<uint16_t> index;
};

/* a level of a tile */
struct LevelKey {
  int tile = 0;
  int level = 0;
};

/* what a plan did in one frame */
struct PlanStats {
  int tiles_prepared = 0; /* tile levels prepared */
  int uploads_positions = 0;
  int uploads_colours = 0;
  int uploads_indices = 0;
  size_t bytes_positions = 0;
  size_t bytes_colours = 0;
  size_t bytes_indices = 0;
  int tiles_waiting = 0; /* tile levels of the stride planned left for later frames */
  /* the time in mbview_prepare_nodes (ms), for measurements */
  double prepare_ms = 0.0;
  int stride = 0;        /* the stride drawn (0: nothing) */
  int stride_zoom = 0;   /* the stride of the zoom */
  int stride_target = 0; /* the stride planned: while uploads are split, the one completed over the frames */
  int tiles_visible = 0; /* tiles with data in the view */
  int tiles_culled = 0;  /* tiles with data outside it */
  bool drag_coarser = false;   /* the drag budget made the stride coarser than the zoom's */
  bool memory_coarser = false; /* the memory budget did */
  bool gpu_coarser = false;    /* the GPU-time fallback's marks did */
  bool over_memory = false;    /* stride 128 above the memory budget */
  bool split = false;          /* uploads split: drawn at another stride than planned */
  double budget_ms = -1.0;     /* the drag budget of the stride planned (-1: no button down) */
  double bounds_ms = 0.0;      /* the tiles' bounds taken (the spheroid's boxes), measured */
  /* the preparing and gathering of the levels built, estimated, and measured;
     both with bounds_ms, which the frame took before it chose the stride */
  double estimated_ms = 0.0;
  double work_ms = 0.0;
  int evicted_own = 0;         /* levels of this plan the frame evicted */
  int evicted_other = 0;       /* levels of other plans it evicted */
};

/* what a frame of the plan gives the renderer */
struct PlanResult {
  std::vector<TileUpload> uploads;
  /* levels whose GPU copies go now, before the uploads: evicted by this frame
     or, since the last frame, by another plan */
  std::vector<LevelKey> drops;
  /* the levels drawn, all of one stride, in tile order */
  std::vector<LevelKey> drawn;
  /* uploads wait for later frames */
  bool more = false;
  PlanStats stats;
};

class TerrainPlan {
 public:
  struct Level {
    int ni = 0; /* lattice nodes along i and j */
    int nj = 0;
    /* streams that must be gathered and uploaded before the level is drawn */
    bool need_positions = true;
    bool need_colours = true;
    bool need_indices = true;
    /* the last index list had no triangle: the level holds nothing on the GPU */
    bool empty = false;
    uint32_t index_count = 0; /* of the last index list, kept as a size hint when the level goes */
    bool built = false;       /* an index list was made once (index_count is known) */
    bool resident = false;    /* the GPU holds the level's buffers (in date or not) */
    size_t bytes = 0;         /* GPU bytes of the level, every copy counted */
    uint64_t used = 0;        /* the cache's number of the last frame that drew it */
    bool drawn = false;       /* drawn in the plan's latest frame */
    bool uploading = false;   /* the stride the latest frame planned, being uploaded over frames */
    /* drawn or uploading in the plan's last frame before a map mouse button
       went down: kept from other plans' evictions until the frame after the
       release, so that the release frame can draw the stride of the zoom */
    bool kept = false;
  };
  struct Tile {
    int i0 = 0; /* first and last node, ends included */
    int i1 = 0;
    int j0 = 0;
    int j1 = 0;
    int ni = 0; /* nodes along i and j */
    int nj = 0;
    uint64_t version = 0; /* node version of the tile when it was last compared */
    /* the tile's primary data: range and largest step between neighbours,
       taken again when its version rose */
    bool data = false;
    bool data_stale = true;
    float data_min = 0.0f;
    float data_max = 0.0f;
    float data_step = 0.0f;
    /* the box in display coordinates (outside the spheroid x and y only,
       z follows from the data), and the largest cell edge */
    bool box_stale = true;
    double lo[3] = {};
    double hi[3] = {};
    double edge = 0.0;
    /* on the spheroid: the longitude and latitude of the lattice of the
       tile's box (sphere_box) and its step in nodes, made once for the
       tile's data */
    std::vector<double> lonlat;
    int lattice_step = 0;
    /* the finest stride whose lattice was prepared since the marks (z, and
       colours), 0 for none: what preparing a level is expected to cost */
    int prepared_z = 0;
    int prepared_colour = 0;
    Level levels[kLevels];
  };

  /* what preparing a lattice node computes: colours (and z where stale), z
     only, or nothing (its status bits say it was prepared) */
  enum Context { kColour = 0, kZ = 1, kCheck = 2 };
  /* The classes of the cost averages: the grid mode (0 the primary grid and
     its slope, 1 the secondary grid on the primary's nodes, 2 a secondary
     grid apart, whose values cost PROJ calls), plus 3 on the spheroid (every
     z reprojected), plus 6 with overlay shading of a secondary grid apart
     (PROJ calls for each colour). */
  static constexpr int kCostClasses = 12;
  static int cost_class(const struct mbvq_grid &grid);
  /* the costs of a lattice node in ns, moving averages: preparing it per
     cost class and context, gathering its position and its colour, and
     making the index list per cost class */
  struct Costs {
    double prepare[kCostClasses][3];
    double gather[2];
    double index[kCostClasses];
  };

  TerrainPlan();
  ~TerrainPlan();
  TerrainPlan(const TerrainPlan &) = delete;
  TerrainPlan &operator=(const TerrainPlan &) = delete;

  /* Plan a frame: compare the marks with those of the last frame, mark the
     streams that went stale, cull the tiles, choose the stride, then
     prepare, gather and hand out the uploads within the budgets. Returns
     true when uploads wait for later frames. */
  bool plan(const TerrainInput &input, const PlanOptions &options, PlanResult *result);
  /* forget every tile: the next plan lays them out and uploads them again
     (the renderer dropped its GPU copies) */
  void reset();

  /* gather the colours as the core's floats, 16 bytes per node, instead of
     bytes (RenderOptions::float_terrain_colours); set before the first plan */
  void set_float_colours(bool on) { float_colours_ = on; }
  /* the copies a QRhi keeps of each buffer, which the memory budget counts */
  void set_copies(int copies) { copies_ = copies < 1 ? 1 : copies; }
  /* the plan's view is shown: the levels it drew last and those it uploads
     are kept from other views' evictions while it is */
  void set_visible(bool visible);
  /* Called when this plan's view should draw a frame for the memory budget:
     another plan evicted levels of this one, whose GPU copies go, and whose
     bytes count as free, only after this plan's next frame; or the memory
     budget made this plan's latest frame coarser and another plan let go of
     levels it kept (hidden, gone, or drawing less). */
  void set_evicted_callback(std::function<void()> callback) { evicted_callback_ = std::move(callback); }
  /* the cost averages the drag budget's estimates use */
  const Costs &costs() const { return costs_; }

  const std::vector<Tile> &tiles() const { return tiles_; }
  /* rises when the tiles are laid out again (a new grid size or block grid):
     every GPU copy of the old tiles goes */
  int generation() const { return generation_; }
  /* the level can be drawn: every stream as new as the marks, with triangles */
  bool drawable(int tile, int level = 0) const;
  /* the level needs nothing before a frame draws it (it may hold no triangle) */
  bool ready(int tile, int level) const;
  /* GPU bytes the plan's levels hold now, every copy counted, with those of
     levels evicted by other plans until this plan's next frame drops them */
  size_t held_bytes() const { return resident_bytes_ + evicted_bytes_; }
  /* levels of this plan evicted so far, by this or another plan */
  int evictions() const { return evictions_; }
  /* the tile ranges of a grid, for tests */
  static std::vector<Tile> layout(int n_columns, int n_rows);

  /* the index list of a lattice from the validity of its nodes (one byte per
     node, a * nj + b): for each cell A = (a,b)(a+1,b)(a,b+1) and
     B = (a+1,b)(a+1,b+1)(a,b+1), each where its three nodes are valid */
  static void build_indices(const unsigned char *valid, int ni, int nj, std::vector<uint16_t> *index);

 private:
  friend class TerrainCache;
  struct Work;

  void reset_layout(const TerrainInput &input);
  void mark_stale(const TerrainInput &input);
  void update_bounds(const TerrainInput &input, bool spheroid);
  void scan_data(const struct mbvq_grid &grid, Tile *tile) const;
  void sphere_box(const TerrainInput &input, Tile *tile);
  void box(const struct mbvq_grid &grid, bool spheroid, const Tile &tile, double lo[3], double hi[3],
           double *edge_h, double *edge_z) const;
  size_t level_bytes(const Tile &tile, int level) const;
  Work level_work(const Tile &tile, int level, int cost, const PlanOptions &options) const;
  /* prepare and gather a level; false (its index list made, nothing else) if
     must is false and its bytes do not fit in cap beside those uploaded */
  bool build(const TerrainInput &input, int t, int level, int cost, const PlanOptions &options, size_t cap, bool must,
             PlanResult *result, size_t *uploaded);
  void drop_level(Level *level);
  void learn(int cost);

  std::vector<Tile> tiles_;
  int generation_ = 0;
  bool float_colours_ = false;
  int copies_ = 1;
  bool visible_ = true;
  std::function<void()> evicted_callback_;
  bool have_marks_ = false;
  int n_columns_ = 0;
  int n_rows_ = 0;
  int block_nodes_ = 0;
  int nblock_i_ = 0;
  int nblock_j_ = 0;
  uint64_t position_ = 0;
  uint64_t color_ = 0;
  uint64_t mask_ = 0;
  uint64_t version_ = 0;
  int projection_mode_ = -1;
  /* the cache's accounting: bytes of the resident levels, and of levels
     evicted by other plans whose GPU copies go in this plan's next frame */
  size_t resident_bytes_ = 0;
  size_t evicted_bytes_ = 0;
  std::vector<LevelKey> evicted_;    /* levels evicted by other plans, to drop */
  std::vector<LevelKey> marked_;     /* levels with drawn or uploading set */
  std::vector<LevelKey> kept_;       /* levels with kept set */
  bool dragging_ = false;            /* the latest frame was planned with a map mouse button down */
  /* bytes of the levels drawn last, uploading and kept (reserved) */
  size_t pinned_bytes_ = 0;
  bool memory_coarser_ = false;      /* the memory budget made the latest frame coarser */
  int evictions_ = 0;
  /* the cost averages, and this frame's sums for them (per context, of
     gathering positions and colours, and of index lists) */
  Costs costs_;
  double sum_prepare_ms_[3] = {};
  double sum_prepare_nodes_[3] = {};
  double sum_gather_ms_[2] = {};
  double sum_gather_nodes_[2] = {};
  double sum_index_ms_ = 0.0;
  double sum_index_nodes_ = 0.0;
  std::vector<unsigned char> valid_; /* scratch */
  std::vector<double> points_;       /* scratch */
};

/*--------------------------------------------------------------------*/
/* The physical sizes of what the core asks for in logical pixels (line
   widths, point sizes, stipple factors): physical = max(1, floor(logical x
   ratio x factor + 0.25)), rounded up only from three quarters, so that thin
   lines stay thin at fractional ratios (1 px stays 1 px below ratio 1.75;
   a whole size at a whole ratio is its exact multiple), with ratio the view's
   devicePixelRatioF() (RenderOptions::pixel_ratio offscreen), never the
   render target's, which is 1 in a QRhiWidget. One factor per kind, all 1,
   so that a kind can be changed in one place. Marker sizes are display units
   and are not scaled. The points are scaled by the point-size control of the
   3D soundings view as well: the core draws them 3 logical pixels big, and
   the control's value takes the place of those 3 (point_size; 3 changes
   nothing, as in the map views). */
struct Style {
  double ratio = 1.0;
  double line_factor = 1.0;
  double contour_factor = 1.0;
  double point_factor = 1.0;
  double stipple_factor = 1.0;
  double point_size = 3.0;

  static int physical(double logical, double ratio, double factor);
  int line(double logical) const { return physical(logical, ratio, line_factor); }
  /* the contours are drawn 1 logical pixel wide (mbview_contour) */
  int contour() const { return physical(1.0, ratio, contour_factor); }
  /* multiplied before the division, so that the control's steps of 0.5 give
     exact sizes (3 x 2.5 / 3 is 2.5, which at ratio 1.5 gives 4) */
  int point(double logical) const { return physical(logical * point_size / 3.0, ratio, point_factor); }
  int stipple(int factor) const { return physical(factor, ratio, stipple_factor); }
  /* a line of one physical pixel is drawn with the line topology (as
     OpenGL's 1 px lines at ratio 1); a wider one as quads */
  static bool thin(int physical_width) { return physical_width <= 1; }
};

/* what a draw of the batch plan draws */
enum class DrawKind {
  Terrain,      /* the terrain slot (TerrainPlan) */
  Contours,     /* a contour slot: the finest contours made */
  Lines,        /* thin lines, pairs of vertices */
  LineStrip,    /* a thin strip */
  Points,       /* points */
  Triangles,    /* triangles */
  WideStrip,    /* a strip of wide lines: quads instanced per segment, two bindings one vertex apart */
  WidePairs,    /* wide lines from pairs of vertices: quads instanced per pair */
  StippleLines, /* thin stippled lines from the stipple buffer (pairs) */
  StippleWide,  /* wide stippled lines from the stipple buffer (quads per pair) */
  Markers,      /* a run of markers of one shape, instanced */
  Soundings,    /* the soundings slot of the 3D soundings view (SoundingsPlan) */
  ProfileLines  /* the profile lines of the 3D soundings view: its slot, black, and the next one, red */
};

/* The slot of the profile lines in the decorations of the 3D soundings view
   (SoundingsFrame): it stands where the core records the LINES command of
   the profile lines, which the frame's fill leaves out (two vertices per
   pair of soundings, 48 bytes for every sounding, in every fill); the
   renderer draws those lines from the soundings' positions with index
   lists of its own. A slot of mbview-qt only: the core has none of this
   number (mbv_draw_batch.h). */
constexpr int kSlotProfileLines = 100;
/* above the core's highest slot id: if the core adds slots, compare with its
   new highest here, and raise kSlotProfileLines should that reach it */
static_assert(kSlotProfileLines > MBV_DRAW_SLOT_CONTOURS_FULL,
              "kSlotProfileLines must lie above the core's slot ids (mbv_draw_batch.h)");

/* the buffer a draw reads its vertices or instances from */
enum class DrawSource {
  None,      /* the terrain and contours: their own buffers */
  Batch,     /* the batch's vertices (24 bytes each) */
  Closings,  /* the closing segments of loops: pairs of batch vertices */
  Triangles, /* the quads of the batch as triangles: batch vertices */
  Stipple,   /* the stipple buffer (StippleVertex) */
  Instances  /* the batch's marker instances (24 bytes each) */
};

/* one draw, in stream order */
struct DrawItem {
  DrawKind kind = DrawKind::Lines;
  DrawSource source = DrawSource::None;
  int command = -1; /* the batch command it comes from */
  int slot = 0;     /* its uniform slot */
  /* the first vertex and the vertex count of a draw of vertices; for
     instanced draws (wide lines, markers) the instance count, and the byte
     offset of the first instance in the source */
  uint32_t first = 0;
  uint32_t count = 0;
  uint32_t offset = 0;
  int shape = -1; /* markers: the shape in the batch; contours: the slot (MBV_DRAW_SLOT_CONTOURS_*) */
};

/* the state a slot carries: one per drawable command */
struct SlotSpec {
  bool camera = true;     /* the view's camera, or the projection alone */
  bool depth_test = false;
  float colour[3] = {0.0f, 0.0f, 0.0f}; /* contours: black; markers: the ring colour */
  int width = 1;          /* line width or point size in physical pixels */
  bool stipple = false;
  int factor = 1;         /* stipple factor in physical pixels */
  unsigned pattern = 0xffff;
};

/* a vertex of the stipple buffer: a batch vertex and OpenGL's stipple
   counter at it, in physical pixels along the major axis, such that the
   centre of each fragment gets its count plus 1/2, less a multiple of 16
   times the factor that is the same along a segment (add_stipple) */
struct StippleVertex {
  float x, y, z;
  float r, g, b;
  float d;
};

/* The draws of the map's batch, in stream order: each drawable command gets
   a uniform slot; lines of one physical pixel are drawn with the line
   topology straight from the batch's vertices, wider ones as quads
   instanced over them (a strip of n vertices n - 1 segments, a loop n with
   its closing segment, n vertices of LINES n / 2); a loop's closing segment
   comes from a small buffer of vertex pairs; quads become triangles;
   stippled lines go through a buffer of vertex pairs that carry OpenGL's
   stipple counter, which counts the fragments, goes on along a strip or
   loop and starts again at every LINES segment and every primitive;
   markers are drawn one run per command, instanced over the batch's
   instances. Shapes define the meshes of later runs; groups are counted.
   The slots of the 3D soundings view become draws of its renderer: the
   soundings with the slot's point size, and the profile lines
   (kSlotProfileLines) with two slots, black and then red. */
class BatchPlan {
 public:
  /* camera: projection x modelview, projection: the projection alone
     (OpenGL's convention, column-major), width x height: the drawing in
     physical pixels; they place the stipple only */
  void build(const struct mbv_draw_batch &batch, const Style &style, const double camera[16],
             const double projection[16], int width, int height);

  const std::vector<DrawItem> &items() const { return items_; }
  const std::vector<SlotSpec> &slot_specs() const { return slots_; }
  const std::vector<struct mbv_draw_vertex> &closings() const { return closings_; }
  const std::vector<struct mbv_draw_vertex> &triangles() const { return triangles_; }
  const std::vector<StippleVertex> &stipple() const { return stipple_; }
  int groups() const { return groups_; }
  int markers() const { return markers_; }
  /* line segments in the plan (thin and wide) */
  size_t segments() const { return segments_; }

 private:
  int add_slot(const struct mbv_draw_command &command, const Style &style);
  void add_stipple(const struct mbv_draw_batch &batch, int c, int slot, const double matrix[16], int width,
                   int height);

  std::vector<DrawItem> items_;
  std::vector<SlotSpec> slots_;
  std::vector<struct mbv_draw_vertex> closings_;
  std::vector<struct mbv_draw_vertex> triangles_;
  std::vector<StippleVertex> stipple_;
  int groups_ = 0;
  int markers_ = 0;
  size_t segments_ = 0;
};

/*--------------------------------------------------------------------*/
/* The soundings of the 3D soundings view on the GPU (its renderer holds the
   buffers; this plan says what is in them and what to write): four streams,
   the soundings' positions (3 floats, 12 bytes per sounding) and colours
   (RGBA8, 4 bytes; alpha 0 for a sounding that is not drawn), and their
   profile lines as index lists (two 32-bit indices per pair, the black and
   the red pairs apart) or, for wide lines, a colour per pair (RGBA8: black,
   red, or alpha 0 for none). Every stream is cut into chunks of a fixed
   number of soundings, each chunk of each stream a buffer of its own that is
   always written whole, at most a frame's upload budget (64 MB) per frame:
   Qt's Metal backend keeps a buffer once per frame in flight and applies
   writes to a frame slot only when a frame on that slot uses the buffer, and
   a whole write of a dynamic buffer drops the writes still waiting, so no
   slot ever holds more than one write of a chunk. The positions of a chunk
   reach one sounding into the next chunk, so that the pair across the
   boundary lies in one buffer.
   - A new allocation (reset(): another count, or the core handed over or
     released the array) empties every stream. Its first upload writes the
     chunks in order, every stream of a chunk before the next chunk, and
     each frame draws the soundings whose positions and colours are both
     written (a prefix in the order of the array, which is spatially
     coherent in ping order) and the profile lines among them.
   - Later, a stream whose data changed (start()) is written again. Positions
     and colours above the budget are double-buffered: the new data go into
     the second set of buffers over as many frames as they need, while the
     frames draw the first, complete set; then the two swap. On Metal the
     writes reach a frame slot's buffers only when a frame on that slot binds
     them, so the second set, bound only from the swap on, is copied into
     them whole in the frame of the swap and again in the next frame on the
     other slot: the budget spreads the writes, not that copy. Below the
     budget a stream is written whole within one frame. The index lists and
     colours per pair are written chunk by chunk as the budget allows (each
     chunk's lists are complete in themselves). A stream whose upload runs
     is busy: its data, which the renderer keeps, must not change until the
     upload is done, so a new change mark waits (a position mark during a
     split upload does not restart it) and is read in a frame after it. */
class SoundingsPlan {
 public:
  enum Stream { kPositions = 0, kColours = 1, kLists = 2, kPairColours = 3 };
  static constexpr int kStreams = 4;
  /* soundings per chunk: 2^21, so that a chunk's streams take at most 56 MB
     (24 MB of positions, 8 MB of colours, 16 MB of indices, 8 MB of pair
     colours) */
  static constexpr int kChunk = 1 << 21;

  /* a write of one chunk of a stream into one of its two sets of buffers */
  struct Upload {
    int stream = 0;
    int set = 0;
    int chunk = 0;
    size_t bytes = 0;
  };

  /* a new allocation of count soundings, in chunks of chunk soundings, with
     an upload budget per frame: every stream empty, no upload running */
  void reset(int count, int chunk = kChunk, size_t budget = kUploadBudget);
  int count() const { return count_; }
  int chunk() const { return chunk_; }
  int chunks() const { return chunks_; }
  size_t budget() const { return budget_; }
  /* the first sounding of chunk c; its soundings (drawn as points); the
     positions its buffer holds (those, and the first of the next chunk); its
     pairs (i, i + 1) with i in the chunk (the instances of its wide lines) */
  int first(int c) const { return c * chunk_; }
  int points(int c) const;
  int positions(int c) const;
  int pairs(int c) const;
  /* the bytes of a chunk of a stream; the index lists' bytes are set for
     each upload (set_list_bytes), one entry per chunk */
  size_t bytes(int stream, int c) const;
  void set_list_bytes(const std::vector<size_t> &bytes) { list_bytes_ = bytes; }
  /* positions and colours above the budget have a second set of buffers */
  bool double_buffered(int stream) const;

  /* the data of a stream changed and were read again: their upload starts;
     not while the stream is busy */
  void start(int stream);
  bool busy(int stream) const { return jobs_[stream].version != 0; }
  /* the writes of this frame, at least one if any is waiting, within the
     budget; the plan takes them as made (a renderer that fails one drops
     everything and resets the plan) */
  std::vector<Upload> frame();
  /* writes wait for later frames */
  bool waiting() const;

  /* the set of a stream that is drawn (0 or 1) */
  int front(int stream) const { return front_[stream]; }
  /* the drawn set of a stream holds data for chunk c */
  bool held(int stream, int c) const;
  /* chunk c's points are drawn: its positions and colours are held */
  bool drawn(int c) const { return held(kPositions, c) && held(kColours, c); }
  /* the soundings drawn: the chunks drawn, in the first upload a prefix */
  int soundings_drawn() const;
  /* chunk c's profile lines are drawn from the stream (kLists or
     kPairColours): its positions and that stream held; without the pair that
     reaches into the next chunk while that chunk's points are not drawn
     (only in the first upload) */
  bool lines(int stream, int c) const { return held(kPositions, c) && held(stream, c); }
  bool last_pair_out(int c) const { return c + 1 < chunks_ && !drawn(c + 1); }

  /* totals since the last reset: chunks written per stream, swaps of the
     two sets */
  int writes(int stream) const { return writes_[stream]; }
  int swaps() const { return swaps_; }

 private:
  struct Job {
    uint64_t version = 0; /* the data being written (0: none) */
    int set = 0;          /* the set written */
    int next = 0;         /* the next chunk to write */
    bool whole = false;   /* written whole within one frame */
  };
  bool complete(int stream, int set) const;

  int count_ = 0;
  int chunk_ = kChunk;
  int chunks_ = 0;
  size_t budget_ = kUploadBudget;
  std::vector<size_t> list_bytes_;
  /* the data each chunk of each set holds (0: none) */
  std::vector<uint64_t> held_[kStreams][2];
  int front_[kStreams] = {};
  Job jobs_[kStreams];
  uint64_t versions_ = 0;
  int writes_[kStreams] = {};
  int swaps_ = 0;
};

/* The pairs of the profile lines, two entries per pair (i and i + 1, in the
   order of the sounding array, as mbvq_c_soundings_profile_pairs gives
   them), made local to the chunks of a plan, in place: each chunk's pairs
   (those whose first sounding lies in the chunk) follow one another, and
   their entries become indices from the chunk's first sounding, into the
   chunk's positions (which reach one sounding into the next chunk). range
   gets each chunk's first entry, and the end (chunks + 1 entries). */
void split_pairs(const SoundingsPlan &plan, std::vector<uint32_t> *list, std::vector<size_t> *range);
/* the entries of chunk c of a list made local (split_pairs), and of them
   those of the pairs inside the chunk's soundings: all but a last pair that
   reaches into the next chunk */
size_t pair_entries(const std::vector<size_t> &range, int c);
size_t inner_pair_entries(const SoundingsPlan &plan, const std::vector<uint32_t> &list, const std::vector<size_t> &range,
                          int c);
/* the colour of each pair (i, i + 1), count - 1 entries of RGBA8: black (0,
   0, 0, 255) for a black pair, red (255, 0, 0, 255) for a red one, none (0,
   0, 0, 0) for the others, from the two lists made local (split_pairs) */
void pair_colours(const SoundingsPlan &plan, const std::vector<uint32_t> &black, const std::vector<size_t> &black_range,
                  const std::vector<uint32_t> &red, const std::vector<size_t> &red_range,
                  std::vector<unsigned char> *rgba);

}  // namespace rhi
}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_RHI_MODEL_H */
