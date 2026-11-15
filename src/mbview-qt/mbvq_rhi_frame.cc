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

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbview.h"
}

#include "mbvq_backend.h"

namespace mbvq {
namespace rhi {

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

bool MapFrame::fill() {
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

  /* the fill policy: full resolution, unless the full-resolution contours are
     due; then the resolution asked for */
  struct mbvq_grid grid;
  mbvq_c_grid(instance_, &grid);
  const int rez = fill_resolution(pending_, mbvq_c_lastdrawrez(instance_), grid.contours,
                                  mbvq_c_contours_made(instance_, MBVQ_REZ_FULL));
  pending_ = -1;

  /* the core records the map: mbvq_c_plot() also clears the node colours
     before a view's first drawing, projects the data, makes the contours and
     sets the view bounds and extents */
  mbv_draw_batch_clear(&batch_);
  set_fill_active(true);
  mbv_draw_set_target(&batch_);
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
  fill_ms_ = milliseconds(start, recorded);

  /* a fill can project the data and so raise the marks: read them after it */
  read_marks(marks_);
  marks_ms_ += milliseconds(recorded, Clock::now());
  return true;
}

bool MapFrame::begin(bool continued, int width, int height, FrameInput *input) {
  fill_ms_ = 0.0;
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

  TerrainInput &terrain = input->terrain;
  terrain.instance = instance_;
  mbvq_c_grid(instance_, &terrain.grid);
  terrain.position = marks[0];
  terrain.color = marks[1];
  terrain.mask = marks[2];
  terrain.version = marks[4];
  int error = MB_ERROR_NO_ERROR;
  mbview_getmarkblocks(0, instance_, &terrain.block_nodes, &terrain.nblock_i, &terrain.nblock_j, &error);

  /* the camera the core would set with OpenGL */
  double projection[16];
  double modelview[16];
  mbvq_c_camera(instance_, projection, modelview);
  multiply(projection, modelview, input->camera);

  /* where the batch draws the terrain, and with which depth test */
  for (int c = 0; c < batch_.num_commands; c++) {
    const struct mbv_draw_command &command = batch_.commands[c];
    if (command.kind == MBV_DRAW_SLOT && command.primitive == MBV_DRAW_SLOT_TERRAIN) {
      input->terrain_slot = true;
      input->depth_test = command.depth_test;
      break;
    }
  }
}

}  // namespace rhi
}  // namespace mbvq
