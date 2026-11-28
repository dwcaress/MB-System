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
 *   (129 x 129 nodes, the far edge shared with the next tile), each with a
 *   compact copy for the GPU: positions (3 floats per node), colours (RGBA,
 *   4 bytes per node) and the triangles as 16-bit indices into the tile's
 *   nodes, with the core's A/B split of each cell. The plan reads the core's
 *   change marks (mbv_marks.h) after each fill and says which tiles must be
 *   prepared (mbview_prepare_nodes), gathered and uploaded, within an upload
 *   budget per frame; a tile is drawn only when every stream of it is as new
 *   as the marks. All tiles are drawn at full resolution (stride 1).
 */

#ifndef MBVIEW_QT_MBVQ_RHI_MODEL_H
#define MBVIEW_QT_MBVQ_RHI_MODEL_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "mbvq_bridge.h"

namespace mbvq {
namespace rhi {

/* the clock of the times in the renderer's statistics (RenderStats), which
   only measure; nothing is decided by them */
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

/* cells per side of a terrain tile */
constexpr int kTileCells = 128;

/* the upload budget per frame (64 MB): more is uploaded in later frames */
constexpr size_t kUploadBudget = static_cast<size_t>(64) << 20;

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
};

/* what one tile uploads in a frame; positions and colours have one entry per
   node of the tile (node (a, b) at a * nj + b), indices three per triangle */
struct TileUpload {
  int tile = 0;
  bool positions = false;
  bool colours = false;
  bool indices = false;
  std::vector<float> xyz;
  std::vector<unsigned char> rgba;
  std::vector<float> rgba_float; /* the colours as floats instead (set_float_colours) */
  std::vector<uint16_t> index;
};

/* what a plan did in one frame */
struct PlanStats {
  int tiles_prepared = 0;
  int uploads_positions = 0;
  int uploads_colours = 0;
  int uploads_indices = 0;
  size_t bytes_positions = 0;
  size_t bytes_colours = 0;
  size_t bytes_indices = 0;
  int tiles_waiting = 0; /* tiles left for later frames */
  /* the time in mbview_prepare_nodes (ms), for measurements */
  double prepare_ms = 0.0;
};

class TerrainPlan {
 public:
  struct Tile {
    int i0 = 0; /* first and last node, ends included */
    int i1 = 0;
    int j0 = 0;
    int j1 = 0;
    int ni = 0; /* nodes along i and j */
    int nj = 0;
    /* streams that must be gathered and uploaded before the tile is drawn */
    bool need_positions = true;
    bool need_colours = true;
    bool need_indices = true;
    /* the last index list had no triangle: the tile holds nothing on the GPU */
    bool empty = false;
    uint32_t index_count = 0;
    uint64_t version = 0; /* node version of the tile when it was gathered */
  };

  /* Plan a frame: compare the marks with those of the last frame, mark the
     streams that went stale, then prepare, gather and hand out the uploads of
     tiles in order until the budget is reached. Returns true when tiles wait
     for later frames. */
  bool plan(const TerrainInput &input, size_t budget, std::vector<TileUpload> *uploads, PlanStats *stats);

  /* gather the colours as the core's floats, 16 bytes per node, instead of
     bytes (RenderOptions::float_terrain_colours); set before the first plan */
  void set_float_colours(bool on) { float_colours_ = on; }

  const std::vector<Tile> &tiles() const { return tiles_; }
  /* rises when the tiles are laid out again (a new grid size or block grid):
     every GPU copy of the old tiles goes */
  int generation() const { return generation_; }
  /* the tile can be drawn: every stream as new as the marks, with triangles */
  bool drawable(int tile) const;
  /* the tile ranges of a grid, for tests */
  static std::vector<Tile> layout(int n_columns, int n_rows);

  /* the index list of a tile from the validity of its nodes (one byte per
     node, a * nj + b): for each cell A = (a,b)(a+1,b)(a,b+1) and
     B = (a+1,b)(a+1,b+1)(a,b+1), each where its three nodes are valid */
  static void build_indices(const unsigned char *valid, int ni, int nj, std::vector<uint16_t> *index);

 private:
  void reset_layout(const TerrainInput &input);

  std::vector<Tile> tiles_;
  int generation_ = 0;
  bool float_colours_ = false;
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
  std::vector<unsigned char> valid_; /* scratch */
};

}  // namespace rhi
}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_RHI_MODEL_H */
