/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_rhi_frame.cc  11/15/2026
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
 * The QRhi backend of mbview-qt, frame layer: the map view's side of the
 * protocol of mbv_draw_batch.h (mbvq_rhi_frame.h).
 */

#include "mbvq_rhi_frame.h"

#include <algorithm>
#include <cstdio>
#include <vector>

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbview.h"
}

#include "mbvq_backend.h"

namespace mbvq {
namespace rhi {

namespace {

/* the map views with a mouse button down */
std::vector<const void *> button_owners;

}  // namespace

void set_map_buttons(const void *owner, bool down) {
  const auto at = std::find(button_owners.begin(), button_owners.end(), owner);
  if (down && at == button_owners.end())
    button_owners.push_back(owner);
  else if (!down && at != button_owners.end())
    button_owners.erase(at);
}

bool map_buttons_down() { return !button_owners.empty(); }

void multiply(const double a[16], const double b[16], double r[16]) {
  for (int column = 0; column < 4; column++)
    for (int row = 0; row < 4; row++)
      r[4 * column + row] = a[row] * b[4 * column] + a[4 + row] * b[4 * column + 1] + a[8 + row] * b[4 * column + 2] +
                            a[12 + row] * b[4 * column + 3];
}

MapFrame::MapFrame(size_t instance) : instance_(instance) { mbv_draw_batch_init(&batch_); }

MapFrame::~MapFrame() { mbv_draw_batch_free(&batch_); }

void MapFrame::request(int rez) { pending_ = std::max(pending_, rez); }

bool MapFrame::size_core(int width, int height) {
  if (width == width_ && height == height_)
    return false;
  width_ = width;
  height_ = height;
  resized_ = true;
  mbvq_c_resize(instance_, width, height);
  return true;
}

void MapFrame::read_marks(uint64_t marks[5]) const {
  int error = MB_ERROR_NO_ERROR;
  mbview_getmarks(0, instance_, &marks[0], &marks[1], &marks[2], &marks[3], &error);
  struct mbvq_grid grid;
  mbvq_c_grid(instance_, &grid);
  marks[4] = 0;
  if (grid.data != nullptr)
    mbview_getnodeversion(0, instance_, 0, grid.n_columns - 1, 0, grid.n_rows - 1, &marks[4], &error);
}

bool MapFrame::fill(bool again) {
  /* the batch target is one for the whole process: no fill inside another */
  if (mbv_draw_get_target() != nullptr) {
    refusals_++;
    if (!warned_refused_) {
      std::fprintf(stderr, "mbview-qt: map %zu was asked to draw while another map filled its drawing; it draws in "
                           "the next frame\n", instance_);
      warned_refused_ = true;
    }
    return false;
  }
  const Clock::time_point start = Clock::now();

  /* the position mark before the plot: a plot that raises it projected the
     data (a view's first drawing, a new exaggeration or projection) */
  uint64_t position = 0;
  {
    uint64_t color = 0;
    uint64_t mask = 0;
    uint64_t drawn = 0;
    int error = MB_ERROR_NO_ERROR;
    mbview_getmarks(0, instance_, &position, &color, &mask, &drawn, &error);
  }

  /* The fill policy: full resolution, unless the full-resolution contours
     are due; then the resolution asked for. While a mouse button is down in
     this view or in any other map view (the core redraws this map during
     some drags there), the fallback applies too. A frame is judged by the
     plots it will make: one, or on the spheroid two when the position mark
     rose since the marker nodes were last prepared (refill()), so twice the
     time of one full-resolution plot. */
  struct mbvq_grid grid;
  mbvq_c_grid(instance_, &grid);
  const int lastdrawrez = mbvq_c_lastdrawrez(instance_);
  const bool full_contours = mbvq_c_contours_made(instance_, MBVQ_REZ_FULL);
  int rez = rez_;
  if (!again) {
    const bool down = drag_ || map_buttons_down();
    const bool twice = last_full_ms_ > 0.0 && grid.projection_mode == MBV_PROJECTION_SPHEROID &&
                       grid.data != nullptr && (!refill_known_ || position != refill_position_);
    rez = fill_resolution(pending_, lastdrawrez, grid.contours, full_contours, down,
                          twice ? 2.0 * last_full_ms_ : last_full_ms_, fallback_ms_);
    fell_back_ = rez != fill_resolution(pending_, lastdrawrez, grid.contours, full_contours);
    if (fell_back_)
      fallback_fills_++;
  }
  pending_ = -1;

  /* the core records the map: mbvq_c_plot() also clears the node colours
     before a view's first drawing, projects the data, makes the contours and
     sets the view bounds and extents. In 3D at full resolution the pick
     marks are sized first from the zoomed view's bounds, as the plots of low
     and high resolution do, since that plot takes the whole grid as view
     bounds (for the markers). */
  mbv_draw_batch_clear(&batch_);
  set_fill_active(true);
  mbv_draw_set_target(&batch_);
  const bool sized = rez == MBVQ_REZ_FULL && grid.display_mode == MBV_DISPLAY_3D && mbvq_c_pick_size(instance_);
  if (sized)
    pick_sizings_++;
  /* the plot alone is timed for the fallback */
  const Clock::time_point plot = Clock::now();
  mbvq_c_plot(instance_, rez);
  mbv_draw_set_target(nullptr);
  set_fill_active(false);
  rez_ = rez;
  fills_++;
  resized_ = false;
  if (batch_.failed && !warned_failed_) {
    std::fprintf(stderr, "mbview-qt: not enough memory for the drawing of map %zu: it is drawn as far as it goes\n",
                 instance_);
    warned_failed_ = true;
  }
  const Clock::time_point recorded = Clock::now();
  fill_ms_ += milliseconds(start, recorded);
  const double plot_time = milliseconds(plot, recorded);
  plot_ms_ += plot_time;

  /* a fill can project the data and so raise the marks: read them after it */
  read_marks(marks_);
  marks_ms_ += milliseconds(recorded, Clock::now());

  /* The time of one full-resolution plot for the fallback, without the
     one-off costs: a fill that projected the data or made the
     full-resolution contours keeps the time measured before (none at first:
     the first drag after opening a view fills at full resolution, and the
     plot of the fill it makes is measured). The sizing of the 3D pick marks
     before the plot (every MBV_BOUNDSFREQUENCY mouse events, and in the frame
     after most releases) is left out of the time, so that the frame after a
     release measures it again. The spheroid's second fill is not timed: a
     frame that will fill twice is judged by twice this time (above). */
  const bool projected = marks_[0] != position;
  const bool contoured = grid.contours && !full_contours && mbvq_c_contours_made(instance_, MBVQ_REZ_FULL);
  const bool timed = rez == MBVQ_REZ_FULL && !projected && !contoured && !again;
  if (timed)
    last_full_ms_ = plot_time;
  return true;
}

void MapFrame::refill() {
  /* On the spheroid the markers take their sizes from the nodes at the
     corners of the view bounds (mbview_site.c), whose x and y move with the
     exaggeration (mbview_zscalegridpoint); OpenGL prepares them while it
     draws the terrain, before the overlays, but a fill draws no terrain.
     After a fill that raised the position mark (an exaggeration change, a
     projection), those two nodes are prepared and the map filled again. */
  struct mbvq_grid grid;
  mbvq_c_grid(instance_, &grid);
  if (grid.projection_mode != MBV_PROJECTION_SPHEROID || grid.data == nullptr) {
    refill_known_ = false;
    return;
  }
  if (refill_known_ && marks_[0] == refill_position_)
    return;
  const int *vb = grid.viewbounds;
  int error = MB_ERROR_NO_ERROR;
  const int corners[2][2] = {{vb[0], vb[2]}, {vb[1], vb[3]}};
  for (const auto &corner : corners)
    if (corner[0] >= 0 && corner[0] < grid.n_columns && corner[1] >= 0 && corner[1] < grid.n_rows)
      mbview_prepare_nodes(0, instance_, corner[0], corner[0], corner[1], corner[1], 1, &error);
  if (fill(true))
    refills_++;
  refill_position_ = marks_[0];
  refill_known_ = true;
}

bool MapFrame::begin(bool continued, int width, int height, FrameInput *input) {
  fill_ms_ = 0.0;
  plot_ms_ = 0.0;
  marks_ms_ = 0.0;
  size_core(width, height);

  /* The marks are read once in a frame (an O(blocks) query of the whole grid):
     before the decision of a frame that continues uploads, and after a fill,
     which can raise them; nothing calls the core in between. */
  uint64_t marks[5] = {};
  bool have_marks = false;
  bool reuse = false;
  if (continued && !resized_ && !requested() && fills_ > 0) {
    const Clock::time_point start = Clock::now();
    read_marks(marks);
    marks_ms_ += milliseconds(start, Clock::now());
    have_marks = true;
    reuse = std::equal(marks, marks + 5, marks_);
  }
  bool filled = false;
  if (!reuse) {
    filled = fill();
    if (filled) {
      refill();
      std::copy(marks_, marks_ + 5, marks);
      have_marks = true;
    }
  }
  const Clock::time_point start = Clock::now();
  if (!have_marks)
    read_marks(marks);
  make_input(input, filled, marks);
  marks_ms_ += milliseconds(start, Clock::now());
  return reuse || filled;
}

void MapFrame::input(FrameInput *input, bool filled) {
  uint64_t marks[5];
  read_marks(marks);
  make_input(input, filled, marks);
}

void MapFrame::make_input(FrameInput *input, bool filled, const uint64_t marks[5]) const {
  *input = FrameInput();
  input->batch = &batch_;
  input->filled = filled;
  input->rez = rez_;
  input->drag = drag_;

  TerrainInput &terrain = input->terrain;
  terrain.instance = instance_;
  mbvq_c_grid(instance_, &terrain.grid);
  terrain.position = marks[0];
  terrain.color = marks[1];
  terrain.mask = marks[2];
  terrain.version = marks[4];
  int error = MB_ERROR_NO_ERROR;
  mbview_getmarkblocks(0, instance_, &terrain.block_nodes, &terrain.nblock_i, &terrain.nblock_j, &error);

  /* the camera the core would set with OpenGL, and the size of the drawing
     it was made for, for the culling and the stride of the terrain */
  double projection[16];
  double modelview[16];
  mbvq_c_camera(instance_, projection, modelview);
  multiply(projection, modelview, input->camera);
  std::copy(input->camera, input->camera + 16, terrain.camera);
  terrain.width = width_;
  terrain.height = height_;

  std::copy(projection, projection + 16, input->projection);

  /* where the batch draws the terrain, and with which depth test; whether
     it draws contours */
  bool contour_slot = false;
  for (int c = 0; c < batch_.num_commands; c++) {
    const struct mbv_draw_command &command = batch_.commands[c];
    if (command.kind == MBV_DRAW_SLOT && command.primitive == MBV_DRAW_SLOT_TERRAIN && !input->terrain_slot) {
      input->terrain_slot = true;
      input->depth_test = command.depth_test;
    }
    if (command.kind == MBV_DRAW_SLOT &&
        (command.primitive == MBV_DRAW_SLOT_CONTOURS_LOW || command.primitive == MBV_DRAW_SLOT_CONTOURS_HIGH ||
         command.primitive == MBV_DRAW_SLOT_CONTOURS_FULL))
      contour_slot = true;
  }

  /* the finest contours made, judged by the core's flags: a reset leaves
     the old segments in memory (mbview_contour_segments). A level made
     without segments (an interval above the range of z) is no contours,
     as OpenGL's empty list, not a reason to take a coarser level. */
  if (contour_slot) {
    const int levels[3] = {MBVQ_REZ_FULL, MBVQ_REZ_HIGH, MBVQ_REZ_LOW};
    for (const int rez : levels) {
      if (!mbvq_c_contours_made(instance_, rez))
        continue;
      const float *xyz = nullptr;
      size_t n = 0;
      uint64_t generation = 0;
      if (mbview_contour_segments(0, instance_, rez, &xyz, &n, &generation, &error) == MB_SUCCESS && xyz != nullptr &&
          n >= 2) {
        input->contour_rez = rez;
        input->contours = xyz;
        input->contour_vertices = n;
        input->contour_generation = generation;
      }
      break;
    }
  }
}

}  // namespace rhi
}  // namespace mbvq
