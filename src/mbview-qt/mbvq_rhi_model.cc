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
 * The QRhi backend of mbview-qt, model layer: the fill policy and the terrain
 * plan (mbvq_rhi_model.h).
 */

#include "mbvq_rhi_model.h"

#include <algorithm>
#include <cmath>
#include <utility>

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbview.h"
}

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

}  // namespace

/*--------------------------------------------------------------------*/
int fill_resolution(int pending, int lastdrawrez, bool contours_shown, bool full_contours_made) {
  int requested = pending >= 0 ? pending : lastdrawrez;
  if (requested == MBVQ_REZ_NONE)
    requested = MBVQ_REZ_LOW;
  if (contours_shown && !full_contours_made)
    return requested;
  return MBVQ_REZ_FULL;
}

/*--------------------------------------------------------------------*/
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

void TerrainPlan::reset_layout(const TerrainInput &input) {
  tiles_ = layout(input.grid.n_columns, input.grid.n_rows);
  n_columns_ = input.grid.n_columns;
  n_rows_ = input.grid.n_rows;
  block_nodes_ = input.block_nodes;
  nblock_i_ = input.nblock_i;
  nblock_j_ = input.nblock_j;
  generation_++;
}

bool TerrainPlan::drawable(int tile) const {
  if (tile < 0 || tile >= static_cast<int>(tiles_.size()))
    return false;
  const Tile &t = tiles_[tile];
  return !t.need_positions && !t.need_colours && !t.need_indices && !t.empty && t.index_count > 0;
}

bool TerrainPlan::plan(const TerrainInput &input, size_t budget, std::vector<TileUpload> *uploads, PlanStats *stats) {
  const struct mbvq_grid &grid = input.grid;
  *stats = PlanStats();
  if (grid.data == nullptr || grid.n_columns < 2 || grid.n_rows < 2) {
    if (!tiles_.empty())
      generation_++;
    tiles_.clear();
    n_columns_ = n_rows_ = 0;
    return false;
  }

  /* a new grid size or block grid: every copy goes (a new grid raises every
     mark as well) */
  if (tiles_.empty() || grid.n_columns != n_columns_ || grid.n_rows != n_rows_ || input.block_nodes != block_nodes_ ||
      input.nblock_i != nblock_i_ || input.nblock_j != nblock_j_)
    reset_layout(input);
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
      if (input.version == UINT64_MAX || node_version(input.instance, tile) != tile.version) {
        changed[t] = input.version != UINT64_MAX;
        any_changed = any_changed || changed[t];
        tile.need_positions = true;
        tile.need_colours = true;
      }
    }
  }
  if (position_rose)
    for (Tile &tile : tiles_)
      tile.need_positions = true;
  if (colour_rose)
    for (Tile &tile : tiles_)
      tile.need_colours = true;
  if (mask_rose) {
    /* The mask rises without the colour or position marks only with cells
       edited across no data, which also stamp their blocks
       (mbview_updateprimarygridcell): then only the tiles whose version rose
       can have new triangles. Anything else counts for every tile, and so does
       every mask rise in the secondary grid mode with a secondary grid apart.
       A tile whose triangles may change gathers every stream again: a node
       that was not drawn can come into use with copies older than the marks,
       because the preparation leaves the nodes it would not draw as they were
       (in the secondary grid mode those without secondary data, whose z
       predates an exaggeration change made there). */
    const bool per_tile = !secondary_apart && !position_rose && !colour_rose && any_changed;
    for (int t = 0; t < ntile; t++) {
      if (!per_tile || changed[t]) {
        tiles_[t].need_indices = true;
        tiles_[t].need_positions = true;
        tiles_[t].need_colours = true;
      }
    }
  }
  /* A secondary cell edit there raises no colour mark and clears no status
     bit, while OpenGL's full drawing in the secondary grid mode colours every
     node again: so every tile is prepared and its colours gathered again. */
  if (secondary_apart && version_rose && !position_rose && !colour_rose)
    for (Tile &tile : tiles_)
      tile.need_colours = true;
  have_marks_ = true;
  position_ = input.position;
  color_ = input.color;
  mask_ = input.mask;
  version_ = input.version;

  /* prepare, gather and hand out the uploads, tile after tile, within the budget */
  const size_t colour_bytes = float_colours_ ? 4 * sizeof(float) : 4;
  size_t bytes = 0;
  bool full = false;
  std::vector<uint16_t> index;
  for (int t = 0; t < ntile; t++) {
    Tile &tile = tiles_[t];
    const bool work = tile.need_indices || (!tile.empty && (tile.need_positions || tile.need_colours));
    if (!work)
      continue;
    if (full) {
      stats->tiles_waiting++;
      continue;
    }

    /* the index list first: it says whether the tile holds anything */
    const size_t nodes = static_cast<size_t>(tile.ni) * static_cast<size_t>(tile.nj);
    const bool rebuild = tile.need_indices;
    if (rebuild) {
      valid_.assign(nodes, 0);
      mbvq_c_node_validity(input.instance, tile.i0, tile.i1, tile.j0, tile.j1, 1, valid_.data());
      build_indices(valid_.data(), tile.ni, tile.nj, &index);
    }
    const bool empty = rebuild ? index.empty() : tile.empty;
    size_t tile_bytes = 0;
    if (!empty) {
      tile_bytes += tile.need_positions ? 12 * nodes : 0;
      tile_bytes += tile.need_colours ? colour_bytes * nodes : 0;
      tile_bytes += rebuild ? 2 * index.size() : 0;
    }
    if (bytes > 0 && bytes + tile_bytes > budget) {
      full = true;
      stats->tiles_waiting++;
      continue;
    }

    TileUpload upload;
    upload.tile = t;
    if (empty) {
      /* no triangle: the tile holds nothing on the GPU (an empty index list
         drops its buffers) and gathers again when triangles come */
      upload.indices = true;
      tile.empty = true;
      tile.index_count = 0;
      tile.need_positions = true;
      tile.need_colours = true;
      tile.need_indices = false;
    }
    else {
      if (tile.need_positions || tile.need_colours) {
        int error = MB_ERROR_NO_ERROR;
        const Clock::time_point start = Clock::now();
        mbview_prepare_nodes(0, input.instance, tile.i0, tile.i1, tile.j0, tile.j1, 1, &error);
        stats->prepare_ms += milliseconds(start, Clock::now());
        stats->tiles_prepared++;
      }
      if (tile.need_positions) {
        upload.positions = true;
        upload.xyz.resize(3 * nodes);
        float *xyz = upload.xyz.data();
        for (int a = 0; a < tile.ni; a++) {
          for (int b = 0; b < tile.nj; b++, xyz += 3) {
            const int k = (tile.i0 + a) * grid.n_rows + tile.j0 + b;
            const bool data = grid.data[k] != grid.nodata;
            xyz[0] = data ? grid.x[k] : 0.0f;
            xyz[1] = data ? grid.y[k] : 0.0f;
            xyz[2] = data ? grid.z[k] : 0.0f;
          }
        }
        stats->uploads_positions++;
        stats->bytes_positions += 12 * nodes;
      }
      if (tile.need_colours && float_colours_) {
        /* the core's floats as they are (a comparison without the bytes) */
        upload.colours = true;
        upload.rgba_float.resize(4 * nodes);
        float *rgba = upload.rgba_float.data();
        for (int a = 0; a < tile.ni; a++) {
          for (int b = 0; b < tile.nj; b++, rgba += 4) {
            const int k = (tile.i0 + a) * grid.n_rows + tile.j0 + b;
            const bool data = grid.data[k] != grid.nodata;
            rgba[0] = data ? grid.r[k] : 0.0f;
            rgba[1] = data ? grid.g[k] : 0.0f;
            rgba[2] = data ? grid.b[k] : 0.0f;
            rgba[3] = data ? 1.0f : 0.0f;
          }
        }
        stats->uploads_colours++;
        stats->bytes_colours += colour_bytes * nodes;
      }
      else if (tile.need_colours) {
        upload.colours = true;
        upload.rgba.resize(4 * nodes);
        unsigned char *rgba = upload.rgba.data();
        for (int a = 0; a < tile.ni; a++) {
          for (int b = 0; b < tile.nj; b++, rgba += 4) {
            const int k = (tile.i0 + a) * grid.n_rows + tile.j0 + b;
            const bool data = grid.data[k] != grid.nodata;
            rgba[0] = data ? colour_byte(grid.r[k]) : 0;
            rgba[1] = data ? colour_byte(grid.g[k]) : 0;
            rgba[2] = data ? colour_byte(grid.b[k]) : 0;
            rgba[3] = data ? 255 : 0;
          }
        }
        stats->uploads_colours++;
        stats->bytes_colours += colour_bytes * nodes;
      }
      if (rebuild) {
        upload.indices = true;
        tile.index_count = static_cast<uint32_t>(index.size());
        stats->uploads_indices++;
        stats->bytes_indices += 2 * index.size();
        upload.index.swap(index);
      }
      tile.empty = false;
      tile.need_positions = false;
      tile.need_colours = false;
      tile.need_indices = false;
    }
    tile.version = node_version(input.instance, tile);
    uploads->push_back(std::move(upload));
    bytes += tile_bytes;
  }
  return full;
}

}  // namespace rhi
}  // namespace mbvq
