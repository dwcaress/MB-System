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
 * plan and the terrain cache (mbvq_rhi_model.h).
 */

#include "mbvq_rhi_model.h"

#include <algorithm>
#include <cmath>
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

double drag_budget_ms(double outside_ms) { return std::max(kDragMinimumMs, kDragFrameMs - outside_ms - kDragMarginMs); }

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
     that stride beside the bounds taken, and whose uploads fit in this
     frame. Stride 128 if none does. */
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

}  // namespace rhi

/*--------------------------------------------------------------------*/
/* the process-wide memory budget of the tile levels (mbvq_rhi.h) */
void set_cache_budget(size_t bytes) { rhi::TerrainCache::get().budget = bytes; }

size_t cache_budget() { return rhi::TerrainCache::get().budget; }

size_t cache_bytes() { return rhi::TerrainCache::get().held(); }

}  // namespace mbvq
