/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_rhi_model.cc  11/15/2026
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
 * The QRhi backend of mbview-qt, model layer: the fill policy, the terrain
 * plan and the terrain cache, the style, the batch plan and the soundings
 * plan (mbvq_rhi_model.h).
 */

#include "mbvq_rhi_model.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <utility>

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbview.h"
}

#include "mbvq_rhi.h"

namespace mbvq {
namespace rhi {

namespace {

/* a colour byte as mb3dsoundings_color_byte() makes it (mbv_soundings_core.c),
   the same rule for the terrain and the soundings */
unsigned char colour_byte(float c) {
  if (!(c > 0.0f))
    return 0;
  if (c >= 1.0f)
    return 255;
  return static_cast<unsigned char>(std::lround(static_cast<double>(c) * 255.0));
}

/* the version of a range of nodes, ends included */
uint64_t node_version(size_t instance, const TerrainPlan::Tile &tile) {
  uint64_t version = 0;
  int error = MB_ERROR_NO_ERROR;
  mbview_getnodeversion(0, instance, tile.i0, tile.i1, tile.j0, tile.j1, &version, &error);
  return version;
}

/* the grid modes of the cost classes (TerrainPlan::cost_class): the primary
   grid (and its slope), the secondary grid on the primary's nodes, and a
   secondary grid apart, whose values cost PROJ calls (mbview_getsecondaryvalue) */
enum Mode { kPrimary = 0, kSecondary = 1, kSecondaryApart = 2 };
int mode_of(int cost) { return cost % 3; }

/* The costs per lattice node (ns) the moving averages start from. Primary
   grid mode, measured in 6a (M3 Max, slope shading): colours 12-18 ns in
   shading drags and 24-27 ns after idleness, z only 2.5-3.4 ns, gathering
   1.2-2.8 ns; a node prepared already costs the look at its status bits.
   The secondary grid mode was not measured: on the primary's nodes it
   recolours every node at stride 1 (a z-only mark costs a colour mark,
   mbview_prepare_nodes); with a secondary grid apart each secondary value
   costs PROJ calls, so it starts high and learns. On the spheroid every z
   is reprojected (trigonometry, and PROJ for a projected grid; about 27 ns
   a node measured on a geographic grid), and overlay shading of a secondary
   grid apart reads a secondary value for each colour (PROJ for projected
   grids; 133 against 20 ns a node measured with one PROJ call): both start
   high and learn. */
constexpr double kSeedPrepare[3][3] = {{18.0, 3.4, 1.0}, {25.0, 4.0, 2.0}, {150.0, 150.0, 150.0}};
constexpr double kSeedGather[2] = {1.6, 2.8}; /* positions, colours */
constexpr double kSeedIndex[3] = {3.0, 3.0, 150.0};
constexpr double kSeedProj = 150.0;
/* the averages learn from the nodes of a frame in a context, if at least these many */
constexpr double kLearnNodes = 4096.0;
constexpr double kLearnRate = 0.3;
/* the lattice of a tile's box on the spheroid: every 32nd node, and the
   last; finer where a lattice cell would span more than kSphereSpan degrees
   of longitude or latitude */
constexpr int kSphereStep = 32;
constexpr double kSphereSpan = 45.0;

constexpr double kInfinity = std::numeric_limits<double>::infinity();

/* the largest singular value of a 2 x 3 matrix with rows a and b */
double singular(const double a[3], const double b[3]) {
  const double aa = a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
  const double bb = b[0] * b[0] + b[1] * b[1] + b[2] * b[2];
  const double ab = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
  const double half = 0.5 * (aa - bb);
  return std::sqrt(std::max(0.0, 0.5 * (aa + bb) + std::sqrt(half * half + ab * ab)));
}

}  // namespace

/*--------------------------------------------------------------------*/
/* The terrain cache: the GPU memory of the tile levels of every plan of the
   process, under one budget (set_cache_budget). Everything runs on the GUI
   thread. It outlives every plan (never deleted). */
class TerrainCache {
 public:
  static TerrainCache &get() {
    static TerrainCache *cache = new TerrainCache;
    return *cache;
  }

  /* bytes every plan holds, with those evicted until their plan's next frame */
  size_t held() const {
    size_t bytes = 0;
    for (const TerrainPlan *plan : plans)
      bytes += plan->held_bytes();
    return bytes;
  }

  /* bytes every plan holds once the evictions made are carried out */
  size_t resident() const {
    size_t bytes = 0;
    for (const TerrainPlan *plan : plans)
      bytes += plan->resident_bytes_;
    return bytes;
  }

  /* A plan let go of levels it kept from eviction (hidden, gone, or drawing
     less): the other visible plans whose latest frame the memory budget made
     coarser are asked for a frame, which may be finer now. */
  void freed(const TerrainPlan *by) {
    for (TerrainPlan *plan : plans)
      if (plan != by && plan->visible_ && plan->memory_coarser_ && plan->evicted_callback_)
        plan->evicted_callback_();
  }

  /* bytes the other visible plans keep from eviction: the levels they drew in
     their latest frame, those they upload over frames, and, while a map
     mouse button is down, those of their last frame before the press */
  size_t pinned(const TerrainPlan *self) const {
    size_t bytes = 0;
    for (const TerrainPlan *plan : plans)
      if (plan != self && plan->visible_)
        bytes += plan->pinned_bytes_;
    return bytes;
  }

  /* Evict the least recently drawn levels until incoming bytes more fit in
     the budget: never a level a visible plan drew in its latest frame,
     uploads over frames or keeps through a drag (another plan's), nor one
     self draws or uploads now. Levels of self
     go now (result's drops); those of another plan go in its next frame,
     and their bytes count until then (held(), cache_bytes()). The room is
     reckoned as if every eviction made were carried out (resident()), here
     and in later frames, so that no more is evicted than the frames need,
     also while a hidden plan keeps the bytes evicted from it. */
  void evict(TerrainPlan *self, size_t incoming, PlanResult *result) {
    const size_t need = resident() + incoming;
    if (need <= budget)
      return;
    size_t over = need - budget;
    struct Candidate {
      uint64_t used;
      size_t plan;
      int tile;
      int level;
    };
    std::vector<Candidate> candidates;
    for (size_t p = 0; p < plans.size(); p++) {
      const TerrainPlan *plan = plans[p];
      const bool keep = plan == self || plan->visible_;
      for (int t = 0; t < static_cast<int>(plan->tiles_.size()); t++)
        for (int l = 0; l < kLevels; l++) {
          const TerrainPlan::Level &level = plan->tiles_[t].levels[l];
          const bool pinned_level = level.drawn || level.uploading || (plan != self && level.kept);
          if (level.resident && level.bytes > 0 && !(keep && pinned_level))
            candidates.push_back({level.used, p, t, l});
        }
    }
    std::sort(candidates.begin(), candidates.end(), [](const Candidate &a, const Candidate &b) {
      if (a.used != b.used)
        return a.used < b.used;
      if (a.plan != b.plan)
        return a.plan < b.plan;
      return a.tile != b.tile ? a.tile < b.tile : a.level < b.level;
    });
    std::vector<char> told(plans.size(), 0);
    for (const Candidate &c : candidates) {
      if (over == 0)
        break;
      TerrainPlan *plan = plans[c.plan];
      TerrainPlan::Level &level = plan->tiles_[c.tile].levels[c.level];
      const size_t bytes = level.bytes;
      plan->drop_level(&level);
      plan->evictions_++;
      if (plan == self) {
        result->drops.push_back({c.tile, c.level});
        result->stats.evicted_own++;
      }
      else {
        plan->evicted_.push_back({c.tile, c.level});
        plan->evicted_bytes_ += bytes;
        result->stats.evicted_other++;
        told[c.plan] = 1;
      }
      over = bytes >= over ? 0 : over - bytes;
    }
    /* a visible plan frees them in its next frame: ask for it */
    for (size_t p = 0; p < plans.size(); p++)
      if (told[p] && plans[p]->evicted_callback_)
        plans[p]->evicted_callback_();
  }

  size_t budget = kCacheBudget;
  uint64_t frame = 0; /* frames planned by every plan, for the least recently drawn */
  std::vector<TerrainPlan *> plans;
};

/*--------------------------------------------------------------------*/
int fill_resolution(int pending, int lastdrawrez, bool contours_shown, bool full_contours_made) {
  int requested = pending >= 0 ? pending : lastdrawrez;
  if (requested == MBVQ_REZ_NONE)
    requested = MBVQ_REZ_LOW;
  if (contours_shown && !full_contours_made)
    return requested;
  return MBVQ_REZ_FULL;
}

int fill_resolution(int pending, int lastdrawrez, bool contours_shown, bool full_contours_made, bool button_down,
                    double full_ms, double fallback_ms) {
  const int rez = fill_resolution(pending, lastdrawrez, contours_shown, full_contours_made);
  if (rez == MBVQ_REZ_FULL && button_down && full_ms > fallback_ms) {
    /* the resolution asked for, as with contours due */
    int requested = pending >= 0 ? pending : lastdrawrez;
    if (requested == MBVQ_REZ_NONE)
      requested = MBVQ_REZ_LOW;
    return requested;
  }
  return rez;
}

double drag_budget_ms(double outside_ms) { return std::max(kDragMinimumMs, kDragFrameMs - outside_ms - kDragMarginMs); }

bool gpu_wait_counts(WaitApi api, bool fake) {
  return fake || api == WaitApi::Metal || api == WaitApi::Vulkan || api == WaitApi::Direct3D12;
}

ChunkUse chunk_use(WaitApi api, bool compute, int threads_per_group, int thread_group_x) {
  /* a limit of 0 is one the backend does not report (Null) */
  const auto enough = [](int limit) { return limit == 0 || limit >= kSplatGroup; };
  ChunkUse use;
  use.storage = compute &&
                (api == WaitApi::Metal || api == WaitApi::Vulkan || api == WaitApi::OpenGL || api == WaitApi::Null) &&
                enough(threads_per_group) && enough(thread_group_x);
  use.dynamic = !use.storage && api == WaitApi::Metal;
  return use;
}

bool splat_size(int physical, bool available, bool quads, Splat mode) {
  return available && !quads && physical >= 1 &&
         (mode == Splat::Auto ? physical <= kSplatLargest : mode == Splat::On && physical <= 2);
}

int splat_groups(uint64_t items, int limit) {
  const int cap = limit > 0 ? std::min(limit, kSplatGroupCap) : kSplatGroupCap;
  const uint64_t groups = (items + static_cast<uint64_t>(kSplatGroup) - 1) / static_cast<uint64_t>(kSplatGroup);
  return std::max(1, static_cast<int>(std::min(groups, static_cast<uint64_t>(cap))));
}

int stride_phase(int first, int stride) {
  const int k = std::max(1, stride);
  return (k - first % k) % k;
}

int stride_count(int first, int points, int stride) {
  const int k = std::max(1, stride);
  const int phase = stride_phase(first, k);
  return points > phase ? (points - phase + k - 1) / k : 0;
}

bool splat_flip(bool y_up_ndc, bool y_up_framebuffer) { return y_up_ndc != y_up_framebuffer; }

SplatBlock splat_block(const SplatFrame &frame, SplatPass pass, int first, int points, int stride) {
  SplatBlock b = {};
  std::copy(frame.mvp, frame.mvp + 16, b.mvp);
  b.viewport[0] = static_cast<float>(frame.width);
  b.viewport[1] = static_cast<float>(frame.height);
  b.viewport[2] = static_cast<float>(frame.size);
  b.viewport[3] = frame.flip ? 1.0f : 0.0f;
  const uint64_t pixels =
      static_cast<uint64_t>(std::max(0, frame.width)) * static_cast<uint64_t>(std::max(0, frame.height));
  b.counts[1] = static_cast<uint32_t>(pixels);
  b.counts[3] = frame.zero_to_one ? 1u : 0u;
  b.grid[3] = static_cast<uint32_t>(pass);
  switch (pass) {
  case SplatPass::Clear:
    b.grid[0] = static_cast<uint32_t>(splat_groups(pixels, frame.group_limit));
    b.grid[1] = 1;
    break;
  case SplatPass::Depth:
  case SplatPass::Winner:
  case SplatPass::Colour: {
    /* the level's soundings of the chunk, from its phase (the rule the
       points path draws too, record_soundings) */
    const int k = std::max(1, stride);
    const int count = stride_count(first, points, k);
    b.counts[0] = static_cast<uint32_t>(count);
    b.counts[2] = static_cast<uint32_t>(first);
    b.grid[0] = count > 0 ? static_cast<uint32_t>(splat_groups(static_cast<uint64_t>(count), frame.group_limit)) : 0u;
    b.grid[1] = static_cast<uint32_t>(k);
    b.grid[2] = static_cast<uint32_t>(stride_phase(first, k));
    break;
  }
  case SplatPass::Resolve:
    b.grid[1] = 1;
    break;
  }
  return b;
}

bool GpuFallback::slow(double wait_ms, double total_ms) {
  const double mark = kDragFrameMs - kDragMarginMs;
  return total_ms > mark && wait_ms >= kGpuMinimumMs && total_ms - wait_ms < mark;
}

void GpuFallback::start() {
  for (Zoom &z : zooms_)
    z = Zoom();
}

int GpuFallback::zoom_index(int zoom) { return std::min(std::max(zoom, 0), kLevels - 1); }

void GpuFallback::frame(int level, double wait_ms, double total_ms, bool skipped, int zoom) {
  /* the coarsest level is never marked, but a step into it is judged
     (below) */
  if (skipped || level < 0 || level > coarsest_)
    return;
  Zoom &z = zooms_[zoom_index(zoom)];
  /* a stopped zoom marks nothing more, nor is a marked level counted again */
  if (z.stuck || (z.marks >> level & 1u) != 0)
    return;
  /* GPU-slow frames count in a row at one level of this zoom: any other
     frame at it starts the count again */
  if (level != z.count_level || !slow(wait_ms, total_ms)) {
    z.count_level = level;
    z.count = 0;
    z.count_wait = 0.0;
    if (!slow(wait_ms, total_ms))
      return;
  }
  /* after a mark at this zoom, a step to a coarser level must cut the wait
     by a share, or the zoom keeps its level */
  if (z.reference >= 0.0 && level > z.reference_level && wait_ms > (1.0 - kGpuGainShare) * z.reference) {
    z.stuck = true;
    return;
  }
  /* the coarsest level is never marked */
  if (level == coarsest_)
    return;
  z.count++;
  z.count_wait += wait_ms;
  if (z.count >= kGpuSlowFrames) {
    z.marks |= 1u << level;
    z.reference = z.count_wait / z.count;
    z.reference_level = level;
    z.count_level = -1;
    z.count = 0;
    z.count_wait = 0.0;
  }
}

unsigned GpuFallback::marks(int zoom) const { return zooms_[zoom_index(zoom)].marks; }

int GpuFallback::level(int zoom) const {
  const unsigned marks = zooms_[zoom_index(zoom)].marks;
  int level = 0;
  while (level < coarsest_ && (marks >> level & 1u) != 0)
    level++;
  return level;
}

bool GpuFallback::stuck(int zoom) const { return zooms_[zoom_index(zoom)].stuck; }

int zoom_level(double cell_pixels) {
  /* a cell of a pixel or more, an unbounded one, or none known: stride 1 */
  if (!(cell_pixels < kCellPixels))
    return 0;
  int level = 0;
  while (level + 1 < kLevels && level_stride(level + 1) * cell_pixels < kCellPixels)
    level++;
  return level;
}

/*--------------------------------------------------------------------*/
ViewGeometry::ViewGeometry(const double camera[16], int width, int height) {
  std::copy(camera, camera + 16, m_);
  /* element (row r, column c) of the column-major matrix */
  const auto at = [this](int r, int c) { return m_[4 * c + r]; };
  /* the planes of the clip volume, inside where w + x, w - x, w + y, w - y,
     w + z and w - z are not negative (the prototype's routine) */
  for (int p = 0; p < 6; p++) {
    const int r = p / 2;
    const double sign = p % 2 == 0 ? 1.0 : -1.0;
    for (int c = 0; c < 4; c++)
      planes_[p][c] = at(3, c) + sign * at(r, c);
  }
  affine_ = at(3, 0) == 0.0 && at(3, 1) == 0.0 && at(3, 2) == 0.0;
  half_width_ = 0.5 * width;
  half_height_ = 0.5 * height;
}

void ViewGeometry::singular_values(double nx, double ny, double *horizontal, double *vertical, double *any) const {
  /* The screen mapping's Jacobian times the clip w at a point of normalised
     device coordinates (nx, ny): rows (W/2)(row x - nx row w) and
     (H/2)(row y - ny row w) over the columns x, y and z (an affine camera
     has no row w) */
  double jx[3], jy[3];
  for (int c = 0; c < 3; c++) {
    jx[c] = half_width_ * (m_[4 * c] - nx * m_[4 * c + 3]);
    jy[c] = half_height_ * (m_[4 * c + 1] - ny * m_[4 * c + 3]);
  }
  const double hx[3] = {jx[0], jx[1], 0.0};
  const double hy[3] = {jy[0], jy[1], 0.0};
  *horizontal = singular(hx, hy);
  *vertical = std::hypot(jx[2], jy[2]);
  *any = singular(jx, jy);
}

bool ViewGeometry::visible(const double lo[3], const double hi[3]) const {
  /* outside if the corner of the box furthest inside a plane is outside it */
  for (const double *plane : planes_) {
    const double x = plane[0] >= 0.0 ? hi[0] : lo[0];
    const double y = plane[1] >= 0.0 ? hi[1] : lo[1];
    const double z = plane[2] >= 0.0 ? hi[2] : lo[2];
    if (plane[0] * x + plane[1] * y + plane[2] * z + plane[3] < 0.0)
      return false;
  }
  return true;
}

bool ViewGeometry::scale(const double lo[3], const double hi[3], double *horizontal, double *vertical,
                         double *any) const {
  if (affine_) {
    /* the same everywhere */
    if (!(m_[15] > 0.0))
      return false;
    singular_values(0.0, 0.0, horizontal, vertical, any);
    *horizontal /= m_[15];
    *vertical /= m_[15];
    *any /= m_[15];
    return true;
  }
  /* the box reaches the near plane (w + z < 0 somewhere): no bound (not named
     near, which <windows.h> defines away) */
  const double *near_plane = planes_[4];
  const double nearest = near_plane[0] * (near_plane[0] >= 0.0 ? lo[0] : hi[0]) +
                         near_plane[1] * (near_plane[1] >= 0.0 ? lo[1] : hi[1]) +
                         near_plane[2] * (near_plane[2] >= 0.0 ? lo[2] : hi[2]) + near_plane[3];
  if (!(nearest >= 0.0))
    return false;
  /* the smallest depth (clip w) in the box, positive beyond the near plane */
  const double w = m_[3] * (m_[3] >= 0.0 ? lo[0] : hi[0]) + m_[7] * (m_[7] >= 0.0 ? lo[1] : hi[1]) +
                   m_[11] * (m_[11] >= 0.0 ? lo[2] : hi[2]) + m_[15];
  if (!(w > 0.0))
    return false;
  /* where the box is seen: the rectangle of the device coordinates of its
     corners (its image lies in their hull), within the view */
  double range[2][2] = {{kInfinity, -kInfinity}, {kInfinity, -kInfinity}};
  for (int corner = 0; corner < 8; corner++) {
    const double p[3] = {corner & 1 ? hi[0] : lo[0], corner & 2 ? hi[1] : lo[1], corner & 4 ? hi[2] : lo[2]};
    double c[4];
    for (int r = 0; r < 4; r++)
      c[r] = m_[r] * p[0] + m_[4 + r] * p[1] + m_[8 + r] * p[2] + m_[12 + r];
    for (int a = 0; a < 2; a++) {
      range[a][0] = std::min(range[a][0], c[a] / c[3]);
      range[a][1] = std::max(range[a][1], c[a] / c[3]);
    }
  }
  for (auto &axis : range) {
    axis[0] = std::min(std::max(axis[0], -1.0), 1.0);
    axis[1] = std::min(std::max(axis[1], axis[0]), 1.0);
  }
  /* The largest singular values are norms of entries affine in nx and ny,
     so the largest over the rectangle is at one of its corners; divided by
     the smallest depth. */
  *horizontal = *vertical = *any = 0.0;
  for (int corner = 0; corner < 4; corner++) {
    double h, v, a;
    singular_values(range[0][corner & 1], range[1][corner >> 1], &h, &v, &a);
    *horizontal = std::max(*horizontal, h / w);
    *vertical = std::max(*vertical, v / w);
    *any = std::max(*any, a / w);
  }
  return true;
}

/*--------------------------------------------------------------------*/
/* what building a level costs: the streams it needs, the bytes uploaded,
   the GPU bytes of buffers it makes, and the time estimated */
struct TerrainPlan::Work {
  bool any = false;
  bool positions = false;
  bool colours = false;
  bool indices = false;
  int context = kCheck;
  size_t nodes = 0;
  size_t bytes = 0;
  size_t new_bytes = 0;
  double ns = 0.0;
};

int TerrainPlan::cost_class(const struct mbvq_grid &grid) {
  int cost = grid.grid_mode != MBV_GRID_VIEW_SECONDARY ? kPrimary
             : grid.secondary_sameas_primary          ? kSecondary
                                                      : kSecondaryApart;
  if (grid.projection_mode == MBV_PROJECTION_SPHEROID)
    cost += 3;
  if (grid.shade_mode == MBV_SHADE_VIEW_OVERLAY && !grid.secondary_sameas_primary)
    cost += 6;
  return cost;
}

TerrainPlan::TerrainPlan() {
  for (int cost = 0; cost < kCostClasses; cost++) {
    const int mode = mode_of(cost);
    for (int c = 0; c < 3; c++) {
      double seed = kSeedPrepare[mode][c];
      /* the spheroid's classes (3 to 5, 9 to 11) and overlay shading's (6 to 11) */
      if (cost / 3 % 2 == 1 && c != kCheck)
        seed = std::max(seed, kSeedProj);
      if (cost / 6 == 1 && c == kColour)
        seed = std::max(seed, kSeedProj);
      costs_.prepare[cost][c] = seed;
    }
    costs_.index[cost] = kSeedIndex[mode];
  }
  costs_.gather[0] = kSeedGather[0];
  costs_.gather[1] = kSeedGather[1];
  TerrainCache::get().plans.push_back(this);
}

TerrainPlan::~TerrainPlan() {
  TerrainCache &cache = TerrainCache::get();
  cache.plans.erase(std::remove(cache.plans.begin(), cache.plans.end(), this), cache.plans.end());
  /* the levels it kept from eviction go with it */
  if (visible_ && pinned_bytes_ > 0)
    cache.freed(this);
}

void TerrainPlan::set_visible(bool visible) {
  const bool hidden = visible_ && !visible;
  visible_ = visible;
  /* a hidden plan keeps no level from eviction */
  if (hidden && pinned_bytes_ > 0)
    TerrainCache::get().freed(this);
}

std::vector<TerrainPlan::Tile> TerrainPlan::layout(int n_columns, int n_rows) {
  std::vector<Tile> tiles;
  if (n_columns < 2 || n_rows < 2)
    return tiles;
  for (int i0 = 0; i0 < n_columns - 1; i0 += kTileCells) {
    for (int j0 = 0; j0 < n_rows - 1; j0 += kTileCells) {
      Tile tile;
      tile.i0 = i0;
      tile.i1 = std::min(i0 + kTileCells, n_columns - 1);
      tile.j0 = j0;
      tile.j1 = std::min(j0 + kTileCells, n_rows - 1);
      tile.ni = tile.i1 - tile.i0 + 1;
      tile.nj = tile.j1 - tile.j0 + 1;
      /* the lattice of each stride: its full cells only, as the drawing's
         loops at a stride leave out a last partial cell */
      for (int l = 0; l < kLevels; l++) {
        tile.levels[l].ni = (tile.ni - 1) / level_stride(l) + 1;
        tile.levels[l].nj = (tile.nj - 1) / level_stride(l) + 1;
      }
      tiles.push_back(tile);
    }
  }
  return tiles;
}

void TerrainPlan::build_indices(const unsigned char *valid, int ni, int nj, std::vector<uint16_t> *index) {
  index->clear();
  for (int a = 0; a + 1 < ni; a++) {
    for (int b = 0; b + 1 < nj; b++) {
      const int k = a * nj + b;
      const int l = (a + 1) * nj + b;
      const int m = k + 1;
      const int n = l + 1;
      if (valid[k] && valid[l] && valid[m])
        index->insert(index->end(), {static_cast<uint16_t>(k), static_cast<uint16_t>(l), static_cast<uint16_t>(m)});
      if (valid[l] && valid[n] && valid[m])
        index->insert(index->end(), {static_cast<uint16_t>(l), static_cast<uint16_t>(n), static_cast<uint16_t>(m)});
    }
  }
}

void TerrainPlan::reset() {
  if (!tiles_.empty())
    generation_++;
  tiles_.clear();
  have_marks_ = false;
  n_columns_ = n_rows_ = 0;
  projection_mode_ = -1;
  resident_bytes_ = evicted_bytes_ = 0;
  evicted_.clear();
  marked_.clear();
  kept_.clear();
  dragging_ = false;
  const bool kept = visible_ && pinned_bytes_ > 0;
  pinned_bytes_ = 0;
  memory_coarser_ = false;
  if (kept)
    TerrainCache::get().freed(this);
}

void TerrainPlan::reset_layout(const TerrainInput &input) {
  tiles_ = layout(input.grid.n_columns, input.grid.n_rows);
  n_columns_ = input.grid.n_columns;
  n_rows_ = input.grid.n_rows;
  block_nodes_ = input.block_nodes;
  nblock_i_ = input.nblock_i;
  nblock_j_ = input.nblock_j;
  /* the versions the tiles have now: a later rise names the tiles changed */
  for (Tile &tile : tiles_)
    tile.version = node_version(input.instance, tile);
  /* the renderer drops every GPU copy of the old tiles */
  resident_bytes_ = evicted_bytes_ = 0;
  evicted_.clear();
  marked_.clear();
  kept_.clear();
  pinned_bytes_ = 0;
  generation_++;
}

bool TerrainPlan::ready(int tile, int level) const {
  if (tile < 0 || tile >= static_cast<int>(tiles_.size()) || level < 0 || level >= kLevels)
    return false;
  const Level &l = tiles_[tile].levels[level];
  return !l.need_indices && (l.empty || (!l.need_positions && !l.need_colours));
}

bool TerrainPlan::drawable(int tile, int level) const {
  return ready(tile, level) && !tiles_[tile].levels[level].empty && tiles_[tile].levels[level].index_count > 0;
}

void TerrainPlan::drop_level(Level *level) {
  /* its GPU copies go: built again from the core when drawn */
  if (level->resident)
    resident_bytes_ -= std::min(resident_bytes_, level->bytes);
  level->resident = false;
  level->bytes = 0;
  level->need_positions = true;
  level->need_colours = true;
  level->need_indices = true;
}

/*--------------------------------------------------------------------*/
void TerrainPlan::mark_stale(const TerrainInput &input) {
  const struct mbvq_grid &grid = input.grid;
  const int ntile = static_cast<int>(tiles_.size());

  /* what went stale since the last frame (mbv_marks.h): positions and
     colours of every node, which nodes hold data, single cells */
  const bool position_rose = have_marks_ && input.position != position_;
  const bool colour_rose = have_marks_ && input.color != color_;
  const bool mask_rose = have_marks_ && input.mask != mask_;
  const bool version_rose = have_marks_ && input.version != version_;
  /* in the secondary grid mode with a secondary grid whose nodes are not
     those of the primary, a secondary cell edit stamps blocks at secondary
     indices (mbview_updatesecondarygridcell), which name the wrong tiles */
  const bool secondary_apart = grid.grid_mode == MBV_GRID_VIEW_SECONDARY && !grid.secondary_sameas_primary;

  std::vector<char> changed(ntile, 0);
  bool any_changed = false;
  if (version_rose) {
    for (int t = 0; t < ntile; t++) {
      Tile &tile = tiles_[t];
      /* no blocks: node changes come as position and colour marks from now on
         (mbv_marks.h), so their first appearance is a whole-view change */
      const uint64_t version = input.version == UINT64_MAX ? UINT64_MAX : node_version(input.instance, tile);
      if (input.version == UINT64_MAX || version != tile.version) {
        changed[t] = input.version != UINT64_MAX;
        any_changed = any_changed || changed[t];
        for (Level &level : tile.levels) {
          level.need_positions = true;
          level.need_colours = true;
        }
        /* the tile's data changed: its range, its box, its preparation */
        tile.data_stale = true;
        tile.prepared_z = tile.prepared_colour = 0;
        tile.version = version;
      }
    }
  }
  if (position_rose) {
    for (Tile &tile : tiles_) {
      for (Level &level : tile.levels)
        level.need_positions = true;
      tile.prepared_z = 0;
      /* x and y of a box change with a projection; on the spheroid every
         coordinate moves with the exaggeration */
      tile.box_stale = true;
    }
  }
  if (colour_rose) {
    for (Tile &tile : tiles_) {
      for (Level &level : tile.levels)
        level.need_colours = true;
      tile.prepared_colour = 0;
    }
  }
  if (mask_rose) {
    /* The mask rises without the colour or position marks only with cells
       edited across no data, which also stamp their blocks
       (mbview_updateprimarygridcell): then only the tiles whose version rose
       can have new triangles. Anything else counts for every tile, and so does
       every mask rise in the secondary grid mode with a secondary grid apart.
       A level whose triangles may change gathers every stream again: a node
       that was not drawn can come into use with copies older than the marks,
       because the preparation leaves the nodes it would not draw as they were
       (in the secondary grid mode those without secondary data, whose z
       predates an exaggeration change made there). */
    const bool per_tile = !secondary_apart && !position_rose && !colour_rose && any_changed;
    for (int t = 0; t < ntile; t++) {
      if (!per_tile || changed[t]) {
        for (Level &level : tiles_[t].levels) {
          level.need_indices = true;
          level.need_positions = true;
          level.need_colours = true;
        }
      }
    }
  }
  /* A secondary cell edit there raises no colour mark and clears no status
     bit, while OpenGL's full drawing in the secondary grid mode colours every
     node again: so every level is prepared and its colours gathered again. */
  if (secondary_apart && version_rose && !position_rose && !colour_rose)
    for (Tile &tile : tiles_)
      for (Level &level : tile.levels)
        level.need_colours = true;
  /* The data of the whole grid can change without a block stamp: a grid
     update or a new grid raise the mask with the position mark, and without
     blocks node changes come as position and colour marks. The range of every
     tile is taken again then (with blocks never on an exaggeration change,
     which raises the position mark alone; without them on every position or
     colour rise, exaggeration and shading drags included). */
  if ((mask_rose && position_rose) || (input.version == UINT64_MAX && (position_rose || colour_rose)))
    for (Tile &tile : tiles_)
      tile.data_stale = true;

  have_marks_ = true;
  position_ = input.position;
  color_ = input.color;
  mask_ = input.mask;
  version_ = input.version;
}

void TerrainPlan::scan_data(const struct mbvq_grid &grid, Tile *tile) const {
  /* the range of the tile's own data, far edge included, no data skipped,
     and the largest step between neighbours (the vertical cell edge) */
  tile->data = false;
  tile->data_min = tile->data_max = tile->data_step = 0.0f;
  for (int i = tile->i0; i <= tile->i1; i++) {
    for (int j = tile->j0; j <= tile->j1; j++) {
      const int k = i * grid.n_rows + j;
      const float value = grid.data[k];
      if (value == grid.nodata || !std::isfinite(value))
        continue;
      if (!tile->data) {
        tile->data_min = tile->data_max = value;
        tile->data = true;
      }
      else {
        tile->data_min = std::min(tile->data_min, value);
        tile->data_max = std::max(tile->data_max, value);
      }
      if (i < tile->i1) {
        const float next = grid.data[k + grid.n_rows];
        if (next != grid.nodata && std::isfinite(next))
          tile->data_step = std::max(tile->data_step, std::fabs(next - value));
      }
      if (j < tile->j1) {
        const float next = grid.data[k + 1];
        if (next != grid.nodata && std::isfinite(next))
          tile->data_step = std::max(tile->data_step, std::fabs(next - value));
      }
    }
  }
}

void TerrainPlan::sphere_box(const TerrainInput &input, Tile *tile) {
  /* On the spheroid a node lies where its longitude, latitude and value put
     it, every coordinate moving with the exaggeration. The box holds the
     tile's whole area at its lowest and at its highest value (a node lies on
     the radius between the two, mbview_projectll2display), whichever nodes
     have data and whatever the grid mode draws: the box of a lattice of
     every 32nd node and the last, enlarged by the longest diagonal of a
     lattice cell, since every place of a cell lies within that of one of its
     corners. That holds while a lattice cell spans well under half the
     globe: so the step is halved until a lattice cell spans at most
     kSphereSpan degrees of longitude and latitude, from the largest step
     between neighbouring nodes at the tile's corners (a longitude step taken
     the short way round, as a grid in another projection may jump at the
     antimeridian; a grid in longitude and latitude has the same step
     everywhere). Nothing is prepared. The largest cell edge is a lattice
     edge per cell plus the largest step between neighbours, scaled
     radially. */
  const struct mbvq_grid &grid = input.grid;
  const double radial = std::fabs(grid.scale * grid.exaggeration);
  if (tile->lattice_step == 0 || tile->lonlat.empty()) {
    const int ci[4] = {tile->i0, tile->i1, tile->i0, tile->i1};
    const int cj[4] = {tile->j0, tile->j0, tile->j1, tile->j1};
    int ii[12], jj[12];
    for (int c = 0; c < 4; c++) {
      /* a corner, its neighbour along i and its neighbour along j */
      ii[3 * c] = ii[3 * c + 2] = ci[c];
      jj[3 * c] = jj[3 * c + 1] = cj[c];
      ii[3 * c + 1] = ci[c] == tile->i0 ? ci[c] + 1 : ci[c] - 1;
      jj[3 * c + 2] = cj[c] == tile->j0 ? cj[c] + 1 : cj[c] - 1;
    }
    double corners[24];
    mbvq_c_node_lonlat(input.instance, 12, ii, jj, corners);
    double degrees = 0.0;
    for (int c = 0; c < 4; c++) {
      for (int k = 1; k < 3; k++) {
        double lon = std::fmod(std::fabs(corners[2 * (3 * c + k)] - corners[6 * c]), 360.0);
        lon = lon > 180.0 ? 360.0 - lon : lon;
        const double lat = std::fabs(corners[2 * (3 * c + k) + 1] - corners[6 * c + 1]);
        degrees = std::max(degrees, std::max(lon, lat));
      }
    }
    int step = kSphereStep;
    while (step > 1 && step * degrees > kSphereSpan)
      step /= 2;
    tile->lattice_step = step;
    tile->lonlat.clear();
  }
  const int step = tile->lattice_step;
  int is[kTileCells + 1], js[kTileCells + 1];
  int ni = 0, nj = 0;
  for (int i = tile->i0; i < tile->i1; i += step)
    is[ni++] = i;
  is[ni++] = tile->i1;
  for (int j = tile->j0; j < tile->j1; j += step)
    js[nj++] = j;
  js[nj++] = tile->j1;
  const int n = ni * nj;
  if (tile->lonlat.size() != 2 * static_cast<size_t>(n)) {
    std::vector<int> ii(n), jj(n);
    for (int a = 0; a < ni; a++)
      for (int b = 0; b < nj; b++) {
        ii[a * nj + b] = is[a];
        jj[a * nj + b] = js[b];
      }
    tile->lonlat.resize(2 * static_cast<size_t>(n));
    mbvq_c_node_lonlat(input.instance, n, ii.data(), jj.data(), tile->lonlat.data());
  }
  points_.resize(6 * static_cast<size_t>(n));
  mbvq_c_display_points(input.instance, n, tile->lonlat.data(), tile->data_min, points_.data());
  mbvq_c_display_points(input.instance, n, tile->lonlat.data(), tile->data_max, points_.data() + 3 * n);
  const auto at = [this, nj, n](int value, int a, int b) { return &points_[3 * (value * n + a * nj + b)]; };
  const auto distance = [](const double *p, const double *q) {
    return std::sqrt((p[0] - q[0]) * (p[0] - q[0]) + (p[1] - q[1]) * (p[1] - q[1]) + (p[2] - q[2]) * (p[2] - q[2]));
  };
  double lo[3] = {kInfinity, kInfinity, kInfinity};
  double hi[3] = {-kInfinity, -kInfinity, -kInfinity};
  double margin = 0.0, edge = 0.0;
  bool finite = true;
  for (int value = 0; value < 2; value++) {
    for (int a = 0; a < ni; a++) {
      for (int b = 0; b < nj; b++) {
        const double *p = at(value, a, b);
        for (int c = 0; c < 3; c++) {
          finite = finite && std::isfinite(p[c]);
          lo[c] = std::min(lo[c], p[c]);
          hi[c] = std::max(hi[c], p[c]);
        }
        if (a + 1 < ni)
          edge = std::max(edge, distance(p, at(value, a + 1, b)) / (is[a + 1] - is[a]));
        if (b + 1 < nj)
          edge = std::max(edge, distance(p, at(value, a, b + 1)) / (js[b + 1] - js[b]));
        if (a + 1 < ni && b + 1 < nj)
          margin = std::max(margin, std::max(distance(p, at(value, a + 1, b + 1)),
                                             distance(at(value, a + 1, b), at(value, a, b + 1))));
      }
    }
  }
  if (!finite) {
    /* places the projection could not make: always in the view, stride 1 */
    for (int c = 0; c < 3; c++) {
      tile->lo[c] = -kInfinity;
      tile->hi[c] = kInfinity;
    }
    tile->edge = kInfinity;
    return;
  }
  for (int c = 0; c < 3; c++) {
    tile->lo[c] = lo[c] - margin;
    tile->hi[c] = hi[c] + margin;
  }
  tile->edge = edge + radial * tile->data_step;
}

void TerrainPlan::update_bounds(const TerrainInput &input, bool spheroid) {
  const struct mbvq_grid &grid = input.grid;
  for (Tile &tile : tiles_) {
    if (tile.data_stale) {
      scan_data(grid, &tile);
      tile.data_stale = false;
      tile.box_stale = true;
      /* also a new grid of the same size: its places taken again */
      tile.lonlat.clear();
      tile.lattice_step = 0;
    }
    if (!tile.box_stale)
      continue;
    tile.box_stale = false;
    if (!tile.data)
      continue;
    if (spheroid) {
      sphere_box(input, &tile);
      continue;
    }
    /* x and y from the corner and mid-edge nodes, whose positions the
       projection set for every node (mbview_projectdata), and the largest
       cell edge from the tile's sides; then a margin of one cell */
    const int is[3] = {tile.i0, (tile.i0 + tile.i1) / 2, tile.i1};
    const int js[3] = {tile.j0, (tile.j0 + tile.j1) / 2, tile.j1};
    double x[3][3], y[3][3];
    bool finite = true;
    double lo[2] = {kInfinity, kInfinity};
    double hi[2] = {-kInfinity, -kInfinity};
    for (int a = 0; a < 3; a++) {
      for (int b = 0; b < 3; b++) {
        const int k = is[a] * grid.n_rows + js[b];
        x[a][b] = grid.x[k];
        y[a][b] = grid.y[k];
        finite = finite && std::isfinite(x[a][b]) && std::isfinite(y[a][b]);
        if (a == 1 && b == 1)
          continue;
        lo[0] = std::min(lo[0], x[a][b]);
        lo[1] = std::min(lo[1], y[a][b]);
        hi[0] = std::max(hi[0], x[a][b]);
        hi[1] = std::max(hi[1], y[a][b]);
      }
    }
    double edge = 0.0;
    for (int side = 0; side < 3; side += 2) {
      /* along i at the first and last j, along j at the first and last i */
      const double along_i = std::hypot(x[1][side] - x[0][side], y[1][side] - y[0][side]) +
                             std::hypot(x[2][side] - x[1][side], y[2][side] - y[1][side]);
      const double along_j = std::hypot(x[side][1] - x[side][0], y[side][1] - y[side][0]) +
                             std::hypot(x[side][2] - x[side][1], y[side][2] - y[side][1]);
      edge = std::max(edge, along_i / (tile.i1 - tile.i0));
      edge = std::max(edge, along_j / (tile.j1 - tile.j0));
    }
    if (!finite) {
      /* positions the projection could not make: always in the view, stride 1 */
      tile.lo[0] = tile.lo[1] = -kInfinity;
      tile.hi[0] = tile.hi[1] = kInfinity;
      tile.edge = kInfinity;
      continue;
    }
    for (int c = 0; c < 2; c++) {
      tile.lo[c] = lo[c] - edge;
      tile.hi[c] = hi[c] + edge;
    }
    tile.edge = edge;
  }
}

void TerrainPlan::box(const struct mbvq_grid &grid, bool spheroid, const Tile &tile, double lo[3], double hi[3],
                      double *edge_h, double *edge_z) const {
  *edge_h = tile.edge;
  *edge_z = 0.0;
  if (!tile.data) {
    for (int c = 0; c < 3; c++) {
      lo[c] = kInfinity;
      hi[c] = -kInfinity;
    }
    return;
  }
  for (int c = 0; c < 3; c++) {
    lo[c] = tile.lo[c];
    hi[c] = tile.hi[c];
  }
  if (spheroid)
    return;
  /* z as the core scales it, from the tile's own data */
  const double a = grid.scale * (grid.exaggeration * tile.data_min - grid.zorigin);
  const double b = grid.scale * (grid.exaggeration * tile.data_max - grid.zorigin);
  lo[2] = std::min(a, b);
  hi[2] = std::max(a, b);
  *edge_z = std::fabs(grid.scale * grid.exaggeration) * tile.data_step;
}

size_t TerrainPlan::level_bytes(const Tile &tile, int l) const {
  const Level &level = tile.levels[l];
  if (level.resident)
    return level.bytes;
  if (level.built && level.index_count == 0 && !level.need_indices)
    return 0;
  const size_t nodes = static_cast<size_t>(level.ni) * static_cast<size_t>(level.nj);
  const size_t indices =
      level.built ? level.index_count : 6 * static_cast<size_t>(level.ni - 1) * static_cast<size_t>(level.nj - 1);
  return ((12 + (float_colours_ ? 16 : 4)) * nodes + 2 * indices) * static_cast<size_t>(copies_);
}

TerrainPlan::Work TerrainPlan::level_work(const Tile &tile, int l, int cost, const PlanOptions &options) const {
  Work work;
  const Level &level = tile.levels[l];
  if (!level.need_indices && (level.empty || (!level.need_positions && !level.need_colours)))
    return work;
  work.any = true;
  const int stride = level_stride(l);
  work.nodes = static_cast<size_t>(level.ni) * static_cast<size_t>(level.nj);
  work.positions = level.need_positions;
  work.colours = level.need_colours;
  work.indices = level.need_indices;
  const size_t colour_bytes = float_colours_ ? 16 : 4;
  const size_t indices =
      level.built ? level.index_count : 6 * static_cast<size_t>(level.ni - 1) * static_cast<size_t>(level.nj - 1);
  work.bytes = (work.positions ? 12 * work.nodes : 0) + (work.colours ? colour_bytes * work.nodes : 0) +
               (work.indices ? 2 * indices : 0);
  if (!level.resident)
    work.new_bytes = ((12 + colour_bytes) * work.nodes + 2 * indices) * static_cast<size_t>(copies_);

  /* the context of the preparation: the nodes of the lattice need colours,
     z only, or neither (prepared at this stride or a finer one since the
     marks); the secondary grid mode at stride 1 computes every node again */
  const bool colour = work.colours && !(tile.prepared_colour > 0 && stride % tile.prepared_colour == 0);
  const bool z = work.positions && !(tile.prepared_z > 0 && stride % tile.prepared_z == 0);
  const bool again = mode_of(cost) != kPrimary && stride == 1 && (work.positions || work.colours);
  work.context = colour || again ? kColour : z ? kZ : kCheck;

  const double fake = work.context == kColour ? options.node_ns_colour : options.node_ns_z;
  if (fake >= 0.0) {
    work.ns = static_cast<double>(work.nodes) * fake;
  }
  else {
    double per_node = 0.0;
    if (work.positions || work.colours)
      per_node += costs_.prepare[cost][work.context];
    per_node += (work.positions ? costs_.gather[0] : 0.0) + (work.colours ? costs_.gather[1] : 0.0);
    per_node += work.indices ? costs_.index[cost] : 0.0;
    work.ns = static_cast<double>(work.nodes) * per_node;
  }
  return work;
}

/*--------------------------------------------------------------------*/
bool TerrainPlan::build(const TerrainInput &input, int t, int l, int cost, const PlanOptions &options, size_t cap,
                        bool must, PlanResult *result, size_t *uploaded) {
  const struct mbvq_grid &grid = input.grid;
  PlanStats &stats = result->stats;
  Tile &tile = tiles_[t];
  Level &level = tile.levels[l];
  const Work work = level_work(tile, l, cost, options);
  if (!work.any)
    return true;
  const int stride = level_stride(l);
  const size_t nodes = work.nodes;
  const size_t colour_bytes = float_colours_ ? 4 * sizeof(float) : 4;

  /* the index list first: it says whether the level holds anything */
  std::vector<uint16_t> index;
  const bool rebuild = level.need_indices;
  if (rebuild) {
    const Clock::time_point start = Clock::now();
    valid_.assign(nodes, 0);
    mbvq_c_node_validity(input.instance, tile.i0, tile.i1, tile.j0, tile.j1, stride, valid_.data());
    build_indices(valid_.data(), level.ni, level.nj, &index);
    const double ms = milliseconds(start, Clock::now());
    sum_index_ms_ += ms;
    sum_index_nodes_ += static_cast<double>(nodes);
    stats.work_ms += ms;
  }
  const bool empty = rebuild ? index.empty() : level.empty;
  size_t bytes = 0;
  if (!empty) {
    bytes += level.need_positions ? 12 * nodes : 0;
    bytes += level.need_colours ? colour_bytes * nodes : 0;
    bytes += rebuild ? 2 * index.size() : 0;
  }
  /* the uploads of a frame stop at the budget (the first always goes) */
  if (!must && *uploaded > 0 && *uploaded + bytes > cap)
    return false;

  TileUpload upload;
  upload.tile = t;
  upload.level = l;
  if (empty) {
    /* no triangle: the level holds nothing on the GPU (an empty index list
       drops its buffers) and gathers again when triangles come */
    upload.indices = true;
    if (level.resident)
      resident_bytes_ -= std::min(resident_bytes_, level.bytes);
    level.resident = false;
    level.bytes = 0;
    level.empty = true;
    level.index_count = 0;
    level.built = true;
    level.need_positions = true;
    level.need_colours = true;
    level.need_indices = false;
  }
  else {
    if (level.need_positions || level.need_colours) {
      int error = MB_ERROR_NO_ERROR;
      const Clock::time_point start = Clock::now();
      mbview_prepare_nodes(0, input.instance, tile.i0, tile.i1, tile.j0, tile.j1, stride, &error);
      const double ms = milliseconds(start, Clock::now());
      stats.prepare_ms += ms;
      stats.work_ms += ms;
      stats.tiles_prepared++;
      sum_prepare_ms_[work.context] += ms;
      sum_prepare_nodes_[work.context] += static_cast<double>(nodes);
      tile.prepared_z = tile.prepared_z == 0 ? stride : std::min(tile.prepared_z, stride);
      tile.prepared_colour = tile.prepared_colour == 0 ? stride : std::min(tile.prepared_colour, stride);
    }
    if (level.need_positions) {
      const Clock::time_point start = Clock::now();
      upload.positions = true;
      upload.xyz.resize(3 * nodes);
      float *xyz = upload.xyz.data();
      for (int a = 0; a < level.ni; a++) {
        for (int b = 0; b < level.nj; b++, xyz += 3) {
          const int k = (tile.i0 + a * stride) * grid.n_rows + tile.j0 + b * stride;
          const bool data = grid.data[k] != grid.nodata;
          xyz[0] = data ? grid.x[k] : 0.0f;
          xyz[1] = data ? grid.y[k] : 0.0f;
          xyz[2] = data ? grid.z[k] : 0.0f;
        }
      }
      const double ms = milliseconds(start, Clock::now());
      sum_gather_ms_[0] += ms;
      sum_gather_nodes_[0] += static_cast<double>(nodes);
      stats.work_ms += ms;
      stats.uploads_positions++;
      stats.bytes_positions += 12 * nodes;
    }
    if (level.need_colours) {
      const Clock::time_point start = Clock::now();
      upload.colours = true;
      if (float_colours_) {
        /* the core's floats as they are (a comparison without the bytes) */
        upload.rgba_float.resize(4 * nodes);
        float *rgba = upload.rgba_float.data();
        for (int a = 0; a < level.ni; a++) {
          for (int b = 0; b < level.nj; b++, rgba += 4) {
            const int k = (tile.i0 + a * stride) * grid.n_rows + tile.j0 + b * stride;
            const bool data = grid.data[k] != grid.nodata;
            rgba[0] = data ? grid.r[k] : 0.0f;
            rgba[1] = data ? grid.g[k] : 0.0f;
            rgba[2] = data ? grid.b[k] : 0.0f;
            rgba[3] = data ? 1.0f : 0.0f;
          }
        }
      }
      else {
        upload.rgba.resize(4 * nodes);
        unsigned char *rgba = upload.rgba.data();
        for (int a = 0; a < level.ni; a++) {
          for (int b = 0; b < level.nj; b++, rgba += 4) {
            const int k = (tile.i0 + a * stride) * grid.n_rows + tile.j0 + b * stride;
            const bool data = grid.data[k] != grid.nodata;
            rgba[0] = data ? colour_byte(grid.r[k]) : 0;
            rgba[1] = data ? colour_byte(grid.g[k]) : 0;
            rgba[2] = data ? colour_byte(grid.b[k]) : 0;
            rgba[3] = data ? 255 : 0;
          }
        }
      }
      const double ms = milliseconds(start, Clock::now());
      sum_gather_ms_[1] += ms;
      sum_gather_nodes_[1] += static_cast<double>(nodes);
      stats.work_ms += ms;
      stats.uploads_colours++;
      stats.bytes_colours += colour_bytes * nodes;
    }
    if (rebuild) {
      upload.indices = true;
      level.index_count = static_cast<uint32_t>(index.size());
      level.built = true;
      stats.uploads_indices++;
      stats.bytes_indices += 2 * index.size();
      upload.index.swap(index);
    }
    level.empty = false;
    level.need_positions = false;
    level.need_colours = false;
    level.need_indices = false;
    /* the GPU copies the renderer holds for it: positions, colours and
       indices, as many times as the QRhi keeps each buffer */
    const size_t held = ((12 + colour_bytes) * nodes + 2 * static_cast<size_t>(level.index_count)) *
                        static_cast<size_t>(copies_);
    if (level.resident)
      resident_bytes_ -= std::min(resident_bytes_, level.bytes);
    resident_bytes_ += held;
    level.bytes = held;
    level.resident = true;
  }
  stats.estimated_ms += 1.0e-6 * work.ns;
  result->uploads.push_back(std::move(upload));
  *uploaded += bytes;
  return true;
}

void TerrainPlan::learn(int cost) {
  /* the moving averages of the costs per node of the frame's cost class,
     from this frame's work */
  for (int c = 0; c < 3; c++) {
    double &average = costs_.prepare[cost][c];
    if (sum_prepare_nodes_[c] >= kLearnNodes)
      average += kLearnRate * (1.0e6 * sum_prepare_ms_[c] / sum_prepare_nodes_[c] - average);
    sum_prepare_ms_[c] = sum_prepare_nodes_[c] = 0.0;
  }
  for (int s = 0; s < 2; s++) {
    if (sum_gather_nodes_[s] >= kLearnNodes)
      costs_.gather[s] += kLearnRate * (1.0e6 * sum_gather_ms_[s] / sum_gather_nodes_[s] - costs_.gather[s]);
    sum_gather_ms_[s] = sum_gather_nodes_[s] = 0.0;
  }
  if (sum_index_nodes_ >= kLearnNodes)
    costs_.index[cost] += kLearnRate * (1.0e6 * sum_index_ms_ / sum_index_nodes_ - costs_.index[cost]);
  sum_index_ms_ = sum_index_nodes_ = 0.0;
}

/*--------------------------------------------------------------------*/
bool TerrainPlan::plan(const TerrainInput &input, const PlanOptions &options, PlanResult *result) {
  const struct mbvq_grid &grid = input.grid;
  *result = PlanResult();
  PlanStats &stats = result->stats;
  TerrainCache &cache = TerrainCache::get();
  const uint64_t frame = ++cache.frame;
  const bool drag = options.drag && !options.force_stride1;

  /* While a map mouse button is down, the levels of the last frame before
     the press stay kept from other plans' evictions and count among the
     pinned bytes (with their estimated size once this plan evicted them
     itself): a frame the drag budget makes coarser does not let go of their
     memory, so that the frame after the release can draw the stride of the
     zoom again. */
  if (drag && !dragging_) {
    for (const LevelKey &key : marked_) {
      Level &level = tiles_[key.tile].levels[key.level];
      if ((level.drawn || level.uploading) && !level.kept) {
        level.kept = true;
        kept_.push_back(key);
      }
    }
  }
  else if (!drag) {
    for (const LevelKey &key : kept_)
      tiles_[key.tile].levels[key.level].kept = false;
    kept_.clear();
  }
  dragging_ = drag;

  /* the levels of the latest frame are not kept from eviction any more; the
     levels other plans evicted since then go now, which frees their bytes */
  for (const LevelKey &key : marked_) {
    Level &level = tiles_[key.tile].levels[key.level];
    level.drawn = false;
    level.uploading = false;
  }
  marked_.clear();
  const size_t pinned_before = pinned_bytes_;
  pinned_bytes_ = 0;
  result->drops = evicted_;
  evicted_.clear();
  evicted_bytes_ = 0;

  if (grid.data == nullptr || grid.n_columns < 2 || grid.n_rows < 2) {
    if (!tiles_.empty())
      generation_++;
    tiles_.clear();
    kept_.clear();
    n_columns_ = n_rows_ = 0;
    resident_bytes_ = 0;
    memory_coarser_ = false;
    if (visible_ && pinned_before > 0)
      cache.freed(this);
    return false;
  }

  /* a new grid size or block grid: every copy goes (a new grid raises every
     mark as well) */
  if (tiles_.empty() || grid.n_columns != n_columns_ || grid.n_rows != n_rows_ || input.block_nodes != block_nodes_ ||
      input.nblock_i != nblock_i_ || input.nblock_j != nblock_j_)
    reset_layout(input);
  const bool spheroid = grid.projection_mode == MBV_PROJECTION_SPHEROID;
  if (grid.projection_mode != projection_mode_) {
    for (Tile &tile : tiles_)
      tile.box_stale = true;
    projection_mode_ = grid.projection_mode;
  }
  mark_stale(input);
  /* the bounds count as the frame's work, before the stride is chosen (the
     spheroid's boxes are taken again on every position mark) */
  const Clock::time_point bounds_start = Clock::now();
  update_bounds(input, spheroid);
  stats.bounds_ms = milliseconds(bounds_start, Clock::now());
  stats.work_ms = stats.estimated_ms = stats.bounds_ms;

  /* The tiles the frame plans: those with data in the view, every tile
     without culling. The zoom comes from the tiles in the view: c is the
     largest size of a stride-1 cell there in physical pixels, unbounded
     where a box reaches the near plane. */
  const bool view = input.width > 0 && input.height > 0;
  const bool cull = options.culling && view;
  const ViewGeometry geometry(input.camera, input.width, input.height);
  std::vector<int> planned;
  planned.reserve(tiles_.size());
  double cell = 0.0;
  bool unbounded = false;
  for (int t = 0; t < static_cast<int>(tiles_.size()); t++) {
    const Tile &tile = tiles_[t];
    double lo[3], hi[3], edge_h, edge_z;
    box(grid, spheroid, tile, lo, hi, &edge_h, &edge_z);
    const bool something = tile.data && lo[0] <= hi[0];
    const bool in_view = something && (!view || geometry.visible(lo, hi));
    if (cull && !in_view) {
      stats.tiles_culled += something ? 1 : 0;
      continue;
    }
    planned.push_back(t);
    if (!in_view)
      continue;
    stats.tiles_visible++;
    double horizontal = 0.0, vertical = 0.0, any = 0.0;
    if (!view)
      unbounded = true;
    else if (!geometry.scale(lo, hi, &horizontal, &vertical, &any))
      unbounded = true;
    else
      cell = std::max(cell, spheroid ? any * edge_h : horizontal * edge_h + vertical * edge_z);
  }
  const int zoom = options.force_stride1 || unbounded ? 0 : zoom_level(cell);
  stats.stride_zoom = level_stride(zoom);

  /* The stride: the finest from the zoom's on whose drawn set fits beside
     the levels other visible plans keep, and, while a map mouse button is
     down, whose estimated preparing and gathering fits the drag budget of
     that stride beside the bounds taken, whose uploads fit in this frame,
     and which the GPU-time fallback has not marked at the frame's zoom's
     level in this press (marks made at other zooms do not apply). Stride
     128 if none does. */
  const int cost = cost_class(grid);
  const auto budget_of = [&options](int l) {
    return options.level_budget_ms[l] >= 0.0 ? options.level_budget_ms[l] : options.budget_ms;
  };
  const size_t pinned = cache.pinned(this);
  int level = -1;
  bool memory_fits = true;
  for (int l = zoom; l < kLevels && level < 0; l++) {
    if (options.force_stride1) {
      level = 0;
      break;
    }
    size_t drawn_bytes = 0, upload_bytes = 0;
    double estimate = 0.0;
    for (int t : planned) {
      drawn_bytes += level_bytes(tiles_[t], l);
      const Work work = level_work(tiles_[t], l, cost, options);
      upload_bytes += work.bytes;
      estimate += work.ns;
    }
    memory_fits = pinned + drawn_bytes <= cache.budget;
    if (!memory_fits) {
      stats.memory_coarser = true;
      continue;
    }
    if (drag && (stats.bounds_ms + 1.0e-6 * estimate > budget_of(l) || upload_bytes > options.upload_budget)) {
      stats.drag_coarser = true;
      continue;
    }
    /* a level the GPU-time fallback marked at this frame's zoom's level in
       this press */
    if (drag && (options.gpu_marks[zoom] >> l & 1u) != 0) {
      stats.gpu_coarser = true;
      continue;
    }
    level = l;
  }
  if (level < 0) {
    level = kLevels - 1;
    stats.over_memory = !memory_fits;
  }
  stats.stride_target = level_stride(level);
  stats.budget_ms = drag ? budget_of(level) : -1.0;

  /* Uploads above the budget are split over frames, one stride per frame:
     while the planned stride is incomplete, the frame draws the finest
     coarser stride complete for every planned tile, or first makes the
     finest coarser one whose uploads fit (stride 128 if none). Forced to
     stride 1, the complete tiles are drawn and the others wait. */
  size_t upload_bytes = 0;
  for (int t : planned)
    upload_bytes += level_work(tiles_[t], level, cost, options).bytes;
  const bool split = !options.force_stride1 && upload_bytes > options.upload_budget && level < kLevels - 1;
  int draw = level;
  bool fallback = false;
  if (split) {
    draw = -1;
    for (int l = level + 1; l < kLevels && draw < 0; l++) {
      bool complete = true;
      for (int t : planned)
        complete = complete && ready(t, l);
      if (complete)
        draw = l;
    }
    if (draw < 0) {
      fallback = true;
      for (int l = level + 1; l < kLevels && draw < 0; l++) {
        size_t bytes = 0;
        for (int t : planned)
          bytes += level_work(tiles_[t], l, cost, options).bytes;
        if (bytes <= options.upload_budget)
          draw = l;
      }
      if (draw < 0)
        draw = kLevels - 1;
    }
  }

  /* the levels of this frame are kept from eviction: those drawn, and those
     of a split upload; then room is made for the new ones */
  size_t incoming = 0;
  for (int t : planned) {
    Level &drawn = tiles_[t].levels[draw];
    drawn.drawn = true;
    marked_.push_back({t, draw});
    if (!split || fallback)
      incoming += level_work(tiles_[t], draw, cost, options).new_bytes;
    if (split) {
      Level &fine = tiles_[t].levels[level];
      fine.uploading = true;
      marked_.push_back({t, level});
      incoming += level_work(tiles_[t], level, cost, options).new_bytes;
    }
  }
  cache.evict(this, incoming, result);

  /* prepare, gather and hand out the uploads: the stride drawn, then, while
     split, the planned stride level by level within the budget */
  size_t uploaded = 0;
  if (!split) {
    bool full = false;
    for (int t : planned) {
      if (!full && build(input, t, level, cost, options, options.upload_budget, !options.force_stride1, result, &uploaded)) {
        tiles_[t].levels[level].used = frame;
        continue;
      }
      /* forced to stride 1: the levels beyond the budget wait */
      full = true;
      if (!ready(t, level))
        stats.tiles_waiting++;
    }
  }
  else {
    if (fallback)
      for (int t : planned)
        build(input, t, draw, cost, options, options.upload_budget, true, result, &uploaded);
    bool full = false;
    for (int t : planned) {
      if (!full && build(input, t, level, cost, options, options.upload_budget, false, result, &uploaded))
        tiles_[t].levels[level].used = frame;
      else
        full = true;
      if (!ready(t, level))
        stats.tiles_waiting++;
    }
    /* The planned stride's own bytes can fit in the frame after all, where
       its estimate did not (an index list not made yet counts 12 bytes a
       cell, cells without data none): complete for every planned tile, it is
       drawn now, its uploads in this frame's batch, rather than a coarser
       stride that no later frame would replace. */
    if (stats.tiles_waiting == 0) {
      for (int t : planned) {
        tiles_[t].levels[draw].drawn = false;
        tiles_[t].levels[level].drawn = true;
        tiles_[t].levels[level].uploading = false;
      }
      draw = level;
    }
  }
  stats.split = draw != level;
  result->more = stats.tiles_waiting > 0;

  /* the levels drawn, of one stride; the least recently drawn go first */
  for (int t : planned) {
    if (!drawable(t, draw))
      continue;
    tiles_[t].levels[draw].used = frame;
    result->drawn.push_back({t, draw});
  }
  stats.stride = result->drawn.empty() ? 0 : level_stride(draw);
  for (const LevelKey &key : marked_) {
    const Tile &tile = tiles_[key.tile];
    const Level &marked = tile.levels[key.level];
    if (marked.drawn || marked.uploading)
      pinned_bytes_ += marked.resident ? marked.bytes : marked.uploading ? level_bytes(tile, key.level) : 0;
  }
  for (const LevelKey &key : kept_) {
    const Tile &tile = tiles_[key.tile];
    const Level &kept = tile.levels[key.level];
    if (!kept.drawn && !kept.uploading)
      pinned_bytes_ += level_bytes(tile, key.level);
  }
  learn(cost);

  /* the memory budget made this frame coarser: the plan is asked for a frame
     when another lets go of levels it kept; and this plan keeping less now
     may let others draw finer (during a drag the levels kept from before
     the press stay among them; they go in the frame after the release) */
  memory_coarser_ = stats.memory_coarser;
  if (visible_ && pinned_bytes_ < pinned_before)
    cache.freed(this);
  return result->more;
}

/*--------------------------------------------------------------------*/
int Style::physical(double logical, double ratio, double factor) {
  const double size = std::floor(logical * ratio * factor + 0.25);
  if (!(size >= 1.0))
    return 1;
  return size > 4096.0 ? 4096 : static_cast<int>(size);
}

SlotSpec BatchPlan::slot_spec(const struct mbv_draw_command &command, const Style &style) {
  SlotSpec slot;
  slot.camera = command.camera;
  slot.depth_test = command.depth_test;
  slot.width = command.kind == MBV_DRAW_PRIMITIVE && command.primitive == MBV_DRAW_POINTS ? style.point(command.point_size)
                                                                                          : style.line(command.line_width);
  slot.stipple = command.stipple;
  slot.factor = style.stipple(command.stipple_factor);
  slot.pattern = command.stipple_pattern;
  return slot;
}

int BatchPlan::add_slot(const struct mbv_draw_command &command, const Style &style) {
  slots_.push_back(slot_spec(command, style));
  return static_cast<int>(slots_.size()) - 1;
}

namespace {

/* a point in window coordinates (physical pixels, y up) through a matrix of
   OpenGL's convention; false behind the camera */
bool window(const double m[16], const struct mbv_draw_vertex &v, int width, int height, double *x, double *y) {
  const double c[4] = {m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12], m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13],
                       m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14], m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15]};
  if (!(c[3] > 0.0))
    return false;
  *x = (c[0] / c[3] + 1.0) * 0.5 * width;
  *y = (c[1] / c[3] + 1.0) * 0.5 * height;
  return true;
}

/* whether a point in window coordinates lies inside the diamond of its
   pixel, |x - xc| + |y - yc| < 1/2 around the pixel's centre (xc, yc)
   (OpenGL 2.1, 3.4.1) */
bool in_diamond(double x, double y) {
  return std::fabs(x - std::floor(x) - 0.5) + std::fabs(y - std::floor(y) - 0.5) < 0.5;
}

/* group g's entries in a layout of the runs (BatchPlan), given each group's
   first entry: up to the next group's, or the end; none for a group the
   layout does not have */
void layout_range(const std::vector<uint32_t> &layout, const std::vector<uint32_t> &starts, size_t g, size_t *begin,
                  size_t *end) {
  *begin = 0;
  *end = 0;
  if (g >= starts.size())
    return;
  *begin = starts[g];
  *end = g + 1 < starts.size() ? starts[g + 1] : layout.size();
}

}  // namespace

void BatchPlan::add_stipple(const struct mbv_draw_batch &batch, int c, int slot, const double matrix[16], int width,
                            int height) {
  /* OpenGL's stipple counter s counts the fragments of a line in the order
     they are made, from the first fragment at the start: it goes on along a
     strip or loop and starts again with every primitive and every segment of
     separate lines (mbv_draw_batch.h). A line of one pixel makes one fragment
     in each column of its major axis (each row of a steep line), and the
     diamond-exit rule (OpenGL 2.1, 3.4.1) gives the first and the last. With
     u the major coordinate in window pixels, mirrored so that it grows from
     the start a to the end b, and v the minor one: the first fragment lies in
     the column of ua, or in the next one when ua lies past that column's
     centre and outside its diamond; the last lies in the column of ub only
     when ub lies past its centre and outside its diamond, else in the column
     before. The vertices carry the counter along the major axis, set so that
     the centre of the k-th fragment (from 0) gets s + k + 1/2: the fragment
     shader's floor() gives s + k, with half a pixel to spare for the
     rasterisation and the interpolation. The counter carried to the next
     segment is reduced modulo 16 times the factor, which leaves the
     pattern's bits as they are and keeps the floats small along long strips
     and loops. The columns of a wide line take the value at their major
     coordinate, so each column is drawn or left out whole, and the counter
     is that of the line one pixel wide, as Apple's legacy OpenGL counts it.
     The rule assumes lines of one pixel rasterised by the diamond-exit rule,
     as Metal and Direct3D draw them (Vulkan's default lines are unverified).
     A segment that crosses the window's edge is counted from its vertex, as
     Mesa does, while Apple's legacy OpenGL restarts the counter at the edge
     (OpenGL 2.1 leaves the counter of a clipped segment indeterminate), so
     the dots of such a segment can differ from Apple's. The values are
     interpolated linearly in window coordinates, as the counter needs, only
     under an orthographic projection (the soundings view, the only stippled
     drawing); under a perspective one the interpolation follows depth, so
     the pattern stretches towards the nearer end of a segment and crowds
     towards the farther one. */
  const struct mbv_draw_command &command = batch.commands[c];
  const int n = command.count;
  const struct mbv_draw_vertex *v = &batch.vertices[command.first];
  std::vector<std::pair<int, int>> pairs;
  bool restart = false;
  if (command.primitive == MBV_DRAW_LINES) {
    for (int k = 0; k + 1 < n; k += 2)
      pairs.emplace_back(k, k + 1);
    restart = true;
  }
  else if (command.primitive == MBV_DRAW_LINE_STRIP || command.primitive == MBV_DRAW_LINE_LOOP) {
    for (int k = 0; k + 1 < n; k++)
      pairs.emplace_back(k, k + 1);
    if (command.primitive == MBV_DRAW_LINE_LOOP && n >= 2)
      pairs.emplace_back(n - 1, 0);
  }
  if (pairs.empty())
    return;
  const uint32_t first = static_cast<uint32_t>(stipple_.size());
  double counter = 0.0;
  for (const std::pair<int, int> &pair : pairs) {
    if (restart)
      counter = 0.0;
    double xa = 0.0, ya = 0.0, xb = 0.0, yb = 0.0;
    double da = counter, db = counter;
    if (window(matrix, v[pair.first], width, height, &xa, &ya) && window(matrix, v[pair.second], width, height, &xb, &yb)) {
      const bool steep = std::fabs(yb - ya) > std::fabs(xb - xa);
      const double mirror = (steep ? yb < ya : xb < xa) ? -1.0 : 1.0;
      const double ua = mirror * (steep ? ya : xa), ub = mirror * (steep ? yb : xb);
      const double va = steep ? xa : ya, vb = steep ? xb : yb;
      const double ca = std::floor(ua), cb = std::floor(ub);
      const double head = ua > ca + 0.5 && !in_diamond(ua, va) ? ca + 1.0 : ca;
      const double tail = ub > cb + 0.5 && !in_diamond(ub, vb) ? cb : cb - 1.0;
      da = counter + ua - head;
      db = da + (ub - ua);
      counter += std::max(0.0, tail - head + 1.0);
      counter = std::fmod(counter, 16.0 * slots_[slot].factor);
    }
    const struct mbv_draw_vertex &a = v[pair.first];
    const struct mbv_draw_vertex &b = v[pair.second];
    stipple_.push_back({a.x, a.y, a.z, a.r, a.g, a.b, static_cast<float>(da)});
    stipple_.push_back({b.x, b.y, b.z, b.r, b.g, b.b, static_cast<float>(db)});
  }
  const uint32_t segments = static_cast<uint32_t>(pairs.size());
  DrawItem item;
  item.command = c;
  item.slot = slot;
  item.group = groups_;
  item.source = DrawSource::Stipple;
  if (Style::thin(slots_[slot].width)) {
    item.kind = DrawKind::StippleLines;
    item.first = first;
    item.count = 2 * segments;
  }
  else {
    item.kind = DrawKind::StippleWide;
    item.offset = static_cast<uint32_t>(first * sizeof(StippleVertex));
    item.count = segments;
  }
  segments_ += segments;
  items_.push_back(item);
}

void BatchPlan::build(const struct mbv_draw_batch &batch, const Style &style, const double camera[16],
                      const double projection[16], int width, int height, Merge merge) {
  items_.clear();
  slots_.clear();
  closings_.clear();
  triangles_.clear();
  stipple_.clear();
  groups_ = 0;
  markers_ = 0;
  segments_ = 0;
  /* the runs' layout: the previous build's kept for the comparison at the
     end, this build's made in the other vectors, whose capacity stays;
     group 0 begins before the first group command */
  layout_.swap(previous_layout_);
  group_start_.swap(previous_group_start_);
  layout_.clear();
  group_start_.assign(1, 0);
  index_entries_.assign(1, 0);
  flag_entries_.assign(1, 0);
  runs_ = 0;
  run_parts_ = 0;
  const auto add = [this](DrawKind kind, DrawSource source, int c, int slot, uint32_t first, uint32_t count,
                          uint32_t offset) {
    DrawItem item;
    item.kind = kind;
    item.source = source;
    item.command = c;
    item.slot = slot;
    item.first = first;
    item.count = count;
    item.offset = offset;
    item.group = groups_;
    items_.push_back(item);
  };
  constexpr uint32_t kVertex = sizeof(struct mbv_draw_vertex);
  /* the open run of strips: its draw (-1: none), its first vertex, the end
     of its last strip, its strips and index entries, and its first entry in
     the layout */
  int run = -1;
  uint32_t run_first = 0;
  uint32_t run_end = 0;
  uint32_t run_strips = 0;
  uint32_t run_entries = 0;
  size_t run_at = 0;
  /* the open run ends: of two strips or more it becomes one draw, after its
     group's earlier runs of its kind, of its index entries (thin: each
     strip's vertices, a restart between strips) or instances (wide: the
     run's vertices less one); of one strip it stays that strip's draw, out
     of the layout */
  const auto end_run = [&]() {
    if (run < 0)
      return;
    DrawItem &item = items_[static_cast<size_t>(run)];
    run = -1;
    if (run_strips < 2) {
      layout_.resize(run_at);
      return;
    }
    const bool thin = item.kind == DrawKind::LineStrip;
    size_t &entries = (thin ? index_entries_ : flag_entries_)[static_cast<size_t>(item.group)];
    item.kind = thin ? DrawKind::StripRun : DrawKind::WideRun;
    item.first = 0;
    item.offset = run_first * kVertex;
    item.count = thin ? run_entries : run_end - run_first - 1;
    item.list = static_cast<uint32_t>(entries);
    item.parts = static_cast<int>(run_strips);
    entries += item.count;
    layout_[run_at + 1] = run_strips;
    runs_++;
    run_parts_ += static_cast<int>(run_strips);
  };
  for (int c = 0; c < batch.num_commands; c++) {
    const struct mbv_draw_command &command = batch.commands[c];
    /* a line strip of two vertices or more, not stippled: it joins the open
       run where its slot would equal the run's, it begins after the run's
       last strip (as the core records them) and, thin, its last place stays
       within merge.max_place; else it ends that run, gets its own draw and
       slot as any strip, and opens a run where its width is merged */
    if (command.kind == MBV_DRAW_PRIMITIVE && command.primitive == MBV_DRAW_LINE_STRIP && command.count >= 2 &&
        !command.stipple) {
      const SlotSpec spec = slot_spec(command, style);
      const bool thin = Style::thin(spec.width);
      const uint32_t first = static_cast<uint32_t>(command.first);
      const uint32_t n = static_cast<uint32_t>(command.count);
      segments_ += n - 1;
      if (run >= 0 && spec == slots_[static_cast<size_t>(items_[static_cast<size_t>(run)].slot)] &&
          first >= run_end && (!thin || first + n - 1 - run_first <= merge.max_place)) {
        layout_.push_back(first - run_first);
        layout_.push_back(n);
        run_end = first + n;
        run_strips++;
        run_entries += n + 1;
        continue;
      }
      end_run();
      slots_.push_back(spec);
      const int slot = static_cast<int>(slots_.size()) - 1;
      if (thin)
        add(DrawKind::LineStrip, DrawSource::Batch, c, slot, first, n, 0);
      else
        add(DrawKind::WideStrip, DrawSource::Batch, c, slot, 0, n - 1, first * kVertex);
      if (thin ? merge.thin : merge.wide) {
        run = static_cast<int>(items_.size()) - 1;
        run_first = first;
        run_end = first + n;
        run_strips = 1;
        run_entries = n;
        run_at = layout_.size();
        layout_.insert(layout_.end(), {thin ? 0u : 1u, 1u, 0u, n});
      }
      continue;
    }
    const size_t items_before = items_.size();
    const size_t slots_before = slots_.size();
    switch (command.kind) {
    case MBV_DRAW_GROUP:
      /* a run ends with its group; the next group's lists begin */
      end_run();
      groups_++;
      group_start_.push_back(static_cast<uint32_t>(layout_.size()));
      index_entries_.push_back(0);
      flag_entries_.push_back(0);
      break;
    case MBV_DRAW_SHAPE:
      /* defines the mesh of later runs: a run names its shape by index */
      break;
    case MBV_DRAW_SLOT:
      if (command.primitive == MBV_DRAW_SLOT_TERRAIN) {
        add(DrawKind::Terrain, DrawSource::None, c, add_slot(command, style), 0, 0, 0);
      }
      else if (command.primitive == MBV_DRAW_SLOT_CONTOURS_LOW || command.primitive == MBV_DRAW_SLOT_CONTOURS_HIGH ||
               command.primitive == MBV_DRAW_SLOT_CONTOURS_FULL) {
        /* black, 1 logical pixel wide (mbview_contour) */
        const int slot = add_slot(command, style);
        slots_[slot].width = style.contour();
        slots_[slot].stipple = false;
        add(DrawKind::Contours, DrawSource::None, c, slot, 0, 0, 0);
        items_.back().shape = command.primitive;
      }
      else if (command.primitive == MBV_DRAW_SLOT_SOUNDINGS) {
        /* the soundings, points of the slot's point size (mb3dsoundings_plot) */
        const int slot = add_slot(command, style);
        slots_[slot].width = style.point(command.point_size);
        slots_[slot].stipple = false;
        add(DrawKind::Soundings, DrawSource::None, c, slot, 0, 0, 0);
      }
      else if (command.primitive == kSlotProfileLines) {
        /* the profile lines, black and red (mb3dsoundings_plot), of the
           LINES command's width: two slots, the red one after the black */
        const int black = add_slot(command, style);
        slots_[black].stipple = false;
        const int red = add_slot(command, style);
        slots_[red].stipple = false;
        slots_[red].colour[0] = 1.0f;
        add(DrawKind::ProfileLines, DrawSource::None, c, black, 0, 0, 0);
      }
      break;
    case MBV_DRAW_MARKERS: {
      if (command.count <= 0 || command.primitive < 0 || command.primitive >= batch.num_shapes)
        break;
      const int slot = add_slot(command, style);
      const struct mbv_draw_shape &shape = batch.shapes[command.primitive];
      std::copy(shape.ring_color, shape.ring_color + 3, slots_[slot].colour);
      add(DrawKind::Markers, DrawSource::Instances, c, slot, 0, static_cast<uint32_t>(command.count),
          static_cast<uint32_t>(command.first) * static_cast<uint32_t>(sizeof(struct mbv_draw_instance)));
      items_.back().shape = command.primitive;
      markers_ += command.count;
      break;
    }
    case MBV_DRAW_PRIMITIVE: {
      const int n = command.count;
      if (n <= 0)
        break;
      const uint32_t first = static_cast<uint32_t>(command.first);
      const int primitive = command.primitive;
      const bool line = primitive == MBV_DRAW_LINES || primitive == MBV_DRAW_LINE_STRIP || primitive == MBV_DRAW_LINE_LOOP;
      if (line && command.stipple) {
        const int slot = add_slot(command, style);
        add_stipple(batch, c, slot, command.camera ? camera : projection, width, height);
        break;
      }
      if (primitive == MBV_DRAW_POINTS) {
        add(DrawKind::Points, DrawSource::Batch, c, add_slot(command, style), first, static_cast<uint32_t>(n), 0);
      }
      else if (primitive == MBV_DRAW_LINES) {
        const uint32_t pairs = static_cast<uint32_t>(n / 2);
        if (pairs == 0)
          break;
        const int slot = add_slot(command, style);
        if (Style::thin(slots_[slot].width))
          add(DrawKind::Lines, DrawSource::Batch, c, slot, first, 2 * pairs, 0);
        else
          add(DrawKind::WidePairs, DrawSource::Batch, c, slot, 0, pairs, first * kVertex);
        segments_ += pairs;
      }
      else if (primitive == MBV_DRAW_LINE_STRIP || primitive == MBV_DRAW_LINE_LOOP) {
        if (n < 2)
          break;
        const int slot = add_slot(command, style);
        const bool thin = Style::thin(slots_[slot].width);
        if (thin)
          add(DrawKind::LineStrip, DrawSource::Batch, c, slot, first, static_cast<uint32_t>(n), 0);
        else
          add(DrawKind::WideStrip, DrawSource::Batch, c, slot, 0, static_cast<uint32_t>(n - 1), first * kVertex);
        segments_ += static_cast<size_t>(n - 1);
        if (primitive == MBV_DRAW_LINE_LOOP) {
          /* the closing segment from the last vertex to the first */
          const uint32_t pair = static_cast<uint32_t>(closings_.size() / 2);
          closings_.push_back(batch.vertices[command.first + n - 1]);
          closings_.push_back(batch.vertices[command.first]);
          if (thin)
            add(DrawKind::Lines, DrawSource::Closings, c, slot, 2 * pair, 2, 0);
          else
            add(DrawKind::WidePairs, DrawSource::Closings, c, slot, 0, 1, 2 * pair * kVertex);
          segments_++;
        }
      }
      else if (primitive == MBV_DRAW_TRIANGLES) {
        const uint32_t count = static_cast<uint32_t>(n - n % 3);
        if (count > 0)
          add(DrawKind::Triangles, DrawSource::Batch, c, add_slot(command, style), first, count, 0);
      }
      else if (primitive == MBV_DRAW_QUADS) {
        /* each quad (a, b, c, d) as (a, b, c) and (a, c, d): one colour per
           quad in the drawing (mbview_profile.c), so the split shows nowhere */
        const uint32_t start = static_cast<uint32_t>(triangles_.size());
        for (int q = 0; q + 3 < n; q += 4) {
          const struct mbv_draw_vertex *v = &batch.vertices[command.first + q];
          triangles_.insert(triangles_.end(), {v[0], v[1], v[2], v[0], v[2], v[3]});
        }
        const uint32_t count = static_cast<uint32_t>(triangles_.size()) - start;
        if (count > 0)
          add(DrawKind::Triangles, DrawSource::Triangles, c, add_slot(command, style), start, count, 0);
      }
      break;
    }
    default:
      break;
    }
    /* a command that adds a draw or a slot ends the open run (a stippled
       primitive too short to draw adds a slot); one that adds neither (a
       shape, an unstippled primitive too short to draw, an empty or unknown
       slot or marker command) does not */
    if (items_.size() != items_before || slots_.size() != slots_before)
      end_run();
  }
  end_run();

  /* each group's layout against the same group's in the previous build (a
     group that build did not have compares as empty): only a group whose
     layout changed needs its lists written again */
  const size_t group_count = group_start_.size();
  changed_.assign(group_count, 0);
  for (size_t g = 0; g < group_count; g++) {
    size_t begin = 0, end = 0, before = 0, after = 0;
    layout_range(layout_, group_start_, g, &begin, &end);
    layout_range(previous_layout_, previous_group_start_, g, &before, &after);
    changed_[g] = !std::equal(layout_.data() + begin, layout_.data() + end, previous_layout_.data() + before,
                              previous_layout_.data() + after);
  }
}

size_t BatchPlan::run_index_entries(int group) const {
  return group >= 0 && static_cast<size_t>(group) < index_entries_.size() ? index_entries_[static_cast<size_t>(group)]
                                                                           : 0;
}

size_t BatchPlan::run_flag_entries(int group) const {
  return group >= 0 && static_cast<size_t>(group) < flag_entries_.size() ? flag_entries_[static_cast<size_t>(group)]
                                                                          : 0;
}

bool BatchPlan::runs_changed(int group) const {
  return group >= 0 && static_cast<size_t>(group) < changed_.size() && changed_[static_cast<size_t>(group)] != 0;
}

void BatchPlan::run_indices(int group, uint32_t *out) const {
  /* per thin run of the group, per strip its places, 0xffffffff between
     its strips */
  if (group < 0)
    return;
  size_t k = 0, end = 0;
  layout_range(layout_, group_start_, static_cast<size_t>(group), &k, &end);
  while (k < end) {
    const uint32_t kind = layout_[k];
    const uint32_t strips = layout_[k + 1];
    const uint32_t *strip = &layout_[k + 2];
    k += 2 + 2 * static_cast<size_t>(strips);
    if (kind != 0)
      continue;
    for (uint32_t s = 0; s < strips; s++, strip += 2) {
      if (s > 0)
        *out++ = 0xffffffffu;
      for (uint32_t v = 0; v < strip[1]; v++)
        *out++ = strip[0] + v;
    }
  }
}

void BatchPlan::run_flags(int group, float *out) const {
  /* per wide run of the group, per segment of its vertices (its last
     strip's end less one): 1 inside a strip, 0 at a joint and beside a
     vertex that no strip draws */
  if (group < 0)
    return;
  size_t k = 0, end = 0;
  layout_range(layout_, group_start_, static_cast<size_t>(group), &k, &end);
  while (k < end) {
    const uint32_t kind = layout_[k];
    const uint32_t strips = layout_[k + 1];
    const uint32_t *strip = &layout_[k + 2];
    k += 2 + 2 * static_cast<size_t>(strips);
    if (kind != 1)
      continue;
    const uint32_t *last = strip + 2 * (strips - 1);
    const uint32_t segments = last[0] + last[1] - 1;
    std::fill(out, out + segments, 0.0f);
    for (uint32_t s = 0; s < strips; s++, strip += 2)
      std::fill(out + strip[0], out + strip[0] + strip[1] - 1, 1.0f);
    out += segments;
  }
}

/*--------------------------------------------------------------------*/
void SoundingsPlan::reset(int count, int chunk, size_t budget) {
  count_ = std::max(0, count);
  chunk_ = std::max(1, chunk);
  chunks_ = static_cast<int>((static_cast<int64_t>(count_) + chunk_ - 1) / chunk_);
  budget_ = std::max<size_t>(1, budget);
  list_bytes_.assign(static_cast<size_t>(chunks_), 0);
  for (int s = 0; s < kStreams; s++) {
    for (std::vector<uint64_t> &set : held_[s])
      set.assign(static_cast<size_t>(chunks_), 0);
    front_[s] = 0;
    jobs_[s] = Job();
    writes_[s] = 0;
  }
  swaps_ = 0;
}

int SoundingsPlan::points(int c) const {
  return c < 0 || c >= chunks_ ? 0 : std::min(chunk_, count_ - first(c));
}

int SoundingsPlan::positions(int c) const {
  return c < 0 || c >= chunks_ ? 0 : points(c) + (c + 1 < chunks_ ? 1 : 0);
}

int SoundingsPlan::pairs(int c) const {
  return c < 0 || c >= chunks_ ? 0 : std::max(0, std::min(chunk_, count_ - 1 - first(c)));
}

size_t SoundingsPlan::bytes(int stream, int c) const {
  switch (stream) {
  case kPositions:
    return 12 * static_cast<size_t>(positions(c));
  case kColours:
    return 4 * static_cast<size_t>(points(c));
  case kLists:
    return c >= 0 && static_cast<size_t>(c) < list_bytes_.size() ? list_bytes_[static_cast<size_t>(c)] : 0;
  case kPairColours:
    return 4 * static_cast<size_t>(pairs(c));
  default:
    return 0;
  }
}

bool SoundingsPlan::double_buffered(int stream) const {
  if (stream != kPositions && stream != kColours)
    return false;
  size_t total = 0;
  for (int c = 0; c < chunks_; c++)
    total += bytes(stream, c);
  return total > budget_;
}

bool SoundingsPlan::complete(int stream, int set) const {
  for (const uint64_t version : held_[stream][set])
    if (version == 0)
      return false;
  return chunks_ > 0;
}

bool SoundingsPlan::held(int stream, int c) const {
  return c >= 0 && c < chunks_ && held_[stream][front_[stream]][static_cast<size_t>(c)] != 0;
}

int SoundingsPlan::soundings_drawn() const {
  int n = 0;
  for (int c = 0; c < chunks_; c++)
    n += drawn(c) ? points(c) : 0;
  return n;
}

void SoundingsPlan::start(int stream) {
  if (busy(stream) || chunks_ == 0)
    return;
  /* Into the second set where the stream has one and the drawn set is
     complete, so that the frames draw the drawn set until the new data are
     complete; else into the drawn set: whole within one frame where it is
     complete (positions and colours), so that no frame draws a mix of old
     and new data, chunk by chunk where it is not (the first upload) and for
     the profile lines. */
  Job &job = jobs_[stream];
  job.version = ++versions_;
  job.next = 0;
  const bool full = complete(stream, front_[stream]);
  const bool second = double_buffered(stream);
  job.set = second && full ? 1 - front_[stream] : front_[stream];
  job.whole = !second && full && (stream == kPositions || stream == kColours);
}

bool SoundingsPlan::waiting() const {
  for (const Job &job : jobs_)
    if (job.version != 0)
      return true;
  return false;
}

std::vector<SoundingsPlan::Upload> SoundingsPlan::frame() {
  std::vector<Upload> uploads;
  size_t remaining = budget_;
  bool any = false;
  const auto write = [&](int s, int c) {
    Upload upload;
    upload.stream = s;
    upload.set = jobs_[s].set;
    upload.chunk = c;
    upload.bytes = bytes(s, c);
    uploads.push_back(upload);
    held_[s][upload.set][static_cast<size_t>(c)] = jobs_[s].version;
    writes_[s]++;
    remaining -= std::min(remaining, upload.bytes);
    any = true;
  };
  const auto finish = [&](int s) {
    /* the second set complete: it is drawn from now on */
    if (jobs_[s].set != front_[s]) {
      front_[s] = jobs_[s].set;
      swaps_++;
    }
    jobs_[s] = Job();
  };

  /* the streams written whole: all their chunks in this frame, or none */
  for (int s = 0; s < kStreams; s++) {
    if (!busy(s) || !jobs_[s].whole)
      continue;
    size_t total = 0;
    for (int c = jobs_[s].next; c < chunks_; c++)
      total += bytes(s, c);
    if (any && total > remaining)
      continue;
    for (int c = jobs_[s].next; c < chunks_; c++)
      write(s, c);
    finish(s);
  }

  /* the others chunk by chunk, every stream of a chunk before the next
     chunk, until the budget is spent; at least one write per frame */
  for (int c = 0; c < chunks_; c++) {
    for (int s = 0; s < kStreams; s++) {
      if (!busy(s) || jobs_[s].whole || jobs_[s].next != c)
        continue;
      if (any && bytes(s, c) > remaining)
        return uploads;
      write(s, c);
      if (++jobs_[s].next == chunks_)
        finish(s);
    }
  }
  return uploads;
}

void split_pairs(const SoundingsPlan &plan, std::vector<uint32_t> *list, std::vector<size_t> *range) {
  const int chunks = plan.chunks();
  range->assign(static_cast<size_t>(chunks) + 1, list->size());
  size_t k = 0;
  for (int c = 0; c < chunks; c++) {
    (*range)[static_cast<size_t>(c)] = k;
    const uint32_t first = static_cast<uint32_t>(plan.first(c));
    const uint32_t end = first + static_cast<uint32_t>(plan.points(c));
    while (k + 1 < list->size() && (*list)[k] < end) {
      (*list)[k] -= first;
      (*list)[k + 1] -= first;
      k += 2;
    }
  }
  (*range)[static_cast<size_t>(chunks)] = k;
}

size_t pair_entries(const std::vector<size_t> &range, int c) {
  return range[static_cast<size_t>(c) + 1] - range[static_cast<size_t>(c)];
}

size_t inner_pair_entries(const SoundingsPlan &plan, const std::vector<uint32_t> &list, const std::vector<size_t> &range,
                          int c) {
  /* the pairs follow the order of the array: such a pair is the last */
  const size_t n = pair_entries(range, c);
  const size_t end = range[static_cast<size_t>(c) + 1];
  return n >= 2 && list[end - 1] >= static_cast<uint32_t>(plan.points(c)) ? n - 2 : n;
}

size_t chunk_list_entries(const std::vector<size_t> &black_range, const std::vector<size_t> &red_range, int c) {
  if (c < 0 || black_range.size() != red_range.size() || static_cast<size_t>(c) + 1 >= black_range.size())
    return 0;
  return pair_entries(black_range, c) + pair_entries(red_range, c);
}

void chunk_lists(const std::vector<uint32_t> &black, const std::vector<size_t> &black_range,
                 const std::vector<uint32_t> &red, const std::vector<size_t> &red_range, int c, void *out) {
  /* one copy per part that has entries: an empty list's data() may be null,
     and a copy from null is undefined even for 0 bytes */
  if (chunk_list_entries(black_range, red_range, c) == 0)
    return;
  const size_t nb = pair_entries(black_range, c);
  const size_t nr = pair_entries(red_range, c);
  char *to = static_cast<char *>(out);
  if (nb > 0)
    std::memcpy(to, black.data() + black_range[static_cast<size_t>(c)], 4 * nb);
  if (nr > 0)
    std::memcpy(to + 4 * nb, red.data() + red_range[static_cast<size_t>(c)], 4 * nr);
}

const uint32_t *chunk_list_piece(const std::vector<uint32_t> &black, const std::vector<size_t> &black_range,
                                 const std::vector<uint32_t> &red, const std::vector<size_t> &red_range, int c) {
  /* the entries chunk_lists() would write, where the lists hold them in
     one piece (split_pairs made them local in place) */
  if (chunk_list_entries(black_range, red_range, c) == 0)
    return nullptr;
  const size_t nb = pair_entries(black_range, c);
  const size_t nr = pair_entries(red_range, c);
  if (nb > 0 && nr > 0)
    return nullptr;
  return nb > 0 ? black.data() + black_range[static_cast<size_t>(c)] : red.data() + red_range[static_cast<size_t>(c)];
}

void pair_colours(const SoundingsPlan &plan, const std::vector<uint32_t> &black, const std::vector<size_t> &black_range,
                  const std::vector<uint32_t> &red, const std::vector<size_t> &red_range,
                  std::vector<unsigned char> *rgba) {
  rgba->assign(4 * static_cast<size_t>(std::max(0, plan.count() - 1)), 0);
  for (int c = 0; c < plan.chunks(); c++) {
    const auto mark = [&](const std::vector<uint32_t> &list, const std::vector<size_t> &range, unsigned char r) {
      for (size_t k = range[static_cast<size_t>(c)]; k + 1 < range[static_cast<size_t>(c) + 1]; k += 2) {
        unsigned char *colour = &(*rgba)[4 * (static_cast<size_t>(plan.first(c)) + list[k])];
        colour[0] = r;
        colour[1] = 0;
        colour[2] = 0;
        colour[3] = 255;
      }
    };
    mark(black, black_range, 0);
    mark(red, red_range, 255);
  }
}

}  // namespace rhi

/*--------------------------------------------------------------------*/
/* the process-wide memory budget of the tile levels (mbvq_rhi.h) */
void set_cache_budget(size_t bytes) { rhi::TerrainCache::get().budget = bytes; }

size_t cache_budget() { return rhi::TerrainCache::get().budget; }

size_t cache_bytes() { return rhi::TerrainCache::get().held(); }

}  // namespace mbvq
