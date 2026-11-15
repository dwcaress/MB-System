/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_rhi_frame.h  11/15/2026
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
 * The QRhi backend of mbview-qt, frame layer: how a frame of the map view
 * talks to the core, without any QRhi type. MapFrame keeps the requests the
 * core made (defer_draw), gives the core the size of the drawing, fills the
 * map's batch (mbv_draw_batch.h) at the resolution of the fill policy, and
 * reads what the renderer needs after the fill: the node arrays, the change
 * marks and the camera. Several users can follow one view, because reading
 * the marks changes nothing (mbv_marks.h): a view and a test's offscreen
 * renderer each have their own MapFrame.
 *
 * The batch target is one for the whole process (mbv_draw.c): a fill sets it,
 * calls the core and sets it back to none. A fill asked for while another one
 * runs (a slot of the program drawing a view from inside a fill) is refused,
 * and the frame draws the batch it has.
 */

#ifndef MBVIEW_QT_MBVQ_RHI_FRAME_H
#define MBVIEW_QT_MBVQ_RHI_FRAME_H

#include <cstddef>
#include <cstdint>

#include "mbv_draw_batch.h"
#include "mbvq_bridge.h"
#include "mbvq_rhi_model.h"

namespace mbvq {
namespace rhi {

/* what one frame of the map view gives the renderer; the pointers are valid
   only during the render() that made it */
struct FrameInput {
  const struct mbv_draw_batch *batch = nullptr;
  TerrainInput terrain;
  /* projection x modelview of mbview_camera(), OpenGL's convention, column-major */
  double camera[16] = {};
  bool terrain_slot = false; /* the batch draws the terrain here */
  bool depth_test = false;   /* ... with the depth test (3D, or on the spheroid) */
  bool filled = false;       /* the batch was filled for this frame */
  int rez = MBVQ_REZ_NONE;   /* the resolution of the batch */
};

class MapFrame {
 public:
  explicit MapFrame(size_t instance);
  ~MapFrame();
  MapFrame(const MapFrame &) = delete;
  MapFrame &operator=(const MapFrame &) = delete;

  size_t instance() const { return instance_; }

  /* a drawing the core asked for (defer_draw): the highest resolution asked
     for since the last fill is kept */
  void request(int rez);
  bool requested() const { return pending_ >= 0; }

  /* give the core the size of the drawing in physical pixels if it differs
     from the size given last; true if it did. The next frame then fills
     (begin()), whoever gave the size (also QRhiWidget::initialize()). */
  bool size_core(int width, int height);

  /* fill the batch at the resolution of the fill policy; false if another
     fill runs (the batch and the request stay as they were) */
  bool fill();

  /* The start of every frame of the map, the same for a view and the
     offscreen renderer of the tests: the core's size (a window moved to a
     screen of another ratio changes the pixel size without a resize), then
     the batch, filled again unless this frame only continues uploads
     (continued) and nothing changed since the last fill: no request, no risen
     mark, no new size. Then the renderer's input, with the marks read once
     in this frame. False if the fill was refused (another fill runs): the
     frame draws the batch it has and asks for another frame. */
  bool begin(bool continued, int width, int height, FrameInput *input);

  /* the renderer's input for this frame, from the batch filled last */
  void input(FrameInput *input, bool filled);

  const struct mbv_draw_batch &batch() const { return batch_; }
  int fills() const { return fills_; }
  int refusals() const { return refusals_; }
  /* a size was given to the core since the last fill */
  bool resized() const { return resized_; }
  /* the CPU times of the last begin() in ms, for measurements: the fill (0
     when the batch was reused or the fill refused), and the marks and the
     renderer's input read (RenderStats) */
  double fill_ms() const { return fill_ms_; }
  double marks_ms() const { return marks_ms_; }

 private:
  void read_marks(uint64_t marks[5]) const;
  void make_input(FrameInput *input, bool filled, const uint64_t marks[5]) const;

  size_t instance_;
  int pending_ = -1;
  int width_ = 0;
  int height_ = 0;
  bool resized_ = false;
  struct mbv_draw_batch batch_;
  int rez_ = MBVQ_REZ_NONE;
  int fills_ = 0;
  int refusals_ = 0;
  uint64_t marks_[5] = {}; /* position, colour, mask, drawn and the grid's version at the last fill */
  bool warned_refused_ = false;
  bool warned_failed_ = false;
  double fill_ms_ = 0.0;
  double marks_ms_ = 0.0;
};

/* r = a x b for column-major 4 x 4 matrices */
void multiply(const double a[16], const double b[16], double r[16]);

}  // namespace rhi
}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_RHI_FRAME_H */
