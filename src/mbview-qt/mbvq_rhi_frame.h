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
 * core made (defer_draw) and whether a map mouse button is down, gives the
 * core the size of the drawing, fills the map's batch (mbv_draw_batch.h) at
 * the resolution of the fill policy, and reads what the renderer needs after
 * the fill: the node arrays, the change marks and the camera. Several users
 * can follow one view, because reading the marks changes nothing
 * (mbv_marks.h): a view and a test's offscreen renderer each have their own
 * MapFrame.
 *
 * From 6c a fill of a 3D map at full resolution first sizes the pick marks
 * from the zoomed view's bounds every MBV_BOUNDSFREQUENCY plots
 * (mbvq_c_pick_size), as the core's low and high resolution plots do; while
 * a mouse button is down in any map view of the process (the core redraws
 * the other maps during some drags, mbview_plotlowall) a frame fills at the
 * resolution asked for once the plots it will make at full resolution would
 * take longer than 12 ms: its map's last full-resolution plot (the sizing of
 * the pick marks left out), twice that for a frame that fills twice on the
 * spheroid; and on the spheroid, where the marker sizes read nodes that
 * move with the exaggeration, a frame whose fill raised the position mark
 * prepares those nodes and fills again. The frame names the finest contours
 * made, for the contour slot.
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
  bool drag = false;         /* a map mouse button is down: the drag budget applies */
  /* the projection alone of mbview_camera() (commands drawn without the
     camera), OpenGL's convention, column-major */
  double projection[16] = {};
  /* The contours of the contour slot: the finest level made (MBVQ_REZ_FULL,
     _HIGH, _LOW; MBVQ_REZ_NONE for none, or without a contour slot in the
     batch), its segments as pairs of 3 floats (valid only during this
     render(): the core frees them at its next drawing), their vertices and
     their generation (mbview_contour_segments). */
  int contour_rez = MBVQ_REZ_NONE;
  const float *contours = nullptr;
  size_t contour_vertices = 0;
  uint64_t contour_generation = 0;
};

/* Whether a mouse button is down in some map view of the process, for the
   fill fallback of every map: the core redraws the other maps during some
   drags (mbview_plotlowall), and those frames fall back as the dragged
   map's do. Each view says when its buttons go down and up (owner: the
   view), up when its buttons are cleared and when it is destroyed; saying
   the same twice changes nothing. A view's buttons follow what each of its
   mouse events reports. It clears them when a release is lost to a grab, a
   hide, another window, a modal window or a popup (UngrabMouse, Hide,
   WindowDeactivate, WindowBlocked, FocusOut for a popup) and with a new or
   lost QRhi; a button that is still down comes back with the next mouse
   event that reports it (a drag that goes on after a popup closed). A
   release lost to another widget's grab, which sends the view nothing, is
   noticed at the view's next mouse event (its next press: the view has no
   mouse tracking), whose buttons no longer include that button. */
void set_map_buttons(const void *owner, bool down);
bool map_buttons_down();

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

  /* whether a map mouse button is down in the next frames (the view's
     buttons): the terrain is planned within the drag budget then, and the
     fill falls back as while a button is down in any map view
     (map_buttons_down; a test and the offscreen renderer, which have no
     view, take this for a button down) */
  void set_drag(bool down) { drag_ = down; }
  bool drag() const { return drag_; }
  /* the time of a frame's full-resolution plots above which a frame with a
     mouse button down in any map view fills at the resolution asked for
     (kFillFallbackMs) */
  void set_fill_fallback_ms(double ms) { fallback_ms_ = ms; }
  /* for tests: the time of the last full-resolution plot, as a fill that
     stalled would leave it */
  void set_last_full_ms(double ms) { last_full_ms_ = ms; }
  /* for tests: whether the last fill fell back, as a fill during a drag in
     another map view would leave it */
  void set_fell_back(bool fell_back) { fell_back_ = fell_back; }

  /* give the core the size of the drawing in physical pixels if it differs
     from the size given last; true if it did. The next frame then fills
     (begin()), whoever gave the size (also QRhiWidget::initialize()). */
  bool size_core(int width, int height);

  /* fill the batch at the resolution of the fill policy (again: at the
     resolution of the last fill, for the spheroid's second fill, which
     counts no fallback and is not timed); false if another fill runs (the
     batch and the request stay as they were) */
  bool fill(bool again = false);

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
     when the batch was reused or the fill refused), the core's plot in it
     (the fill without the sizing of the pick marks and the reading around
     the plot), and the marks and the renderer's input read (RenderStats) */
  double fill_ms() const { return fill_ms_; }
  double plot_ms() const { return plot_ms_; }
  double marks_ms() const { return marks_ms_; }
  /* totals: second fills on the spheroid, fills at the resolution asked for
     while a button was down (the fallback), pick-mark sizings before a 3D
     full-resolution fill; and the time of the last full-resolution plot of
     a fill that neither projected the data nor made the full-resolution
     contours (ms, < 0: none yet; on the spheroid the first fill's plot);
     whether the last fill fell back */
  int refills() const { return refills_; }
  int fallback_fills() const { return fallback_fills_; }
  int pick_sizings() const { return pick_sizings_; }
  double last_full_ms() const { return last_full_ms_; }
  bool fell_back() const { return fell_back_; }

 private:
  void read_marks(uint64_t marks[5]) const;
  /* the spheroid's second fill after a fill that raised the position mark */
  void refill();
  void make_input(FrameInput *input, bool filled, const uint64_t marks[5]) const;

  size_t instance_;
  int pending_ = -1;
  int width_ = 0;
  int height_ = 0;
  bool resized_ = false;
  bool drag_ = false;
  struct mbv_draw_batch batch_;
  int rez_ = MBVQ_REZ_NONE;
  int fills_ = 0;
  int refusals_ = 0;
  uint64_t marks_[5] = {}; /* position, colour, mask, drawn and the grid's version at the last fill */
  bool warned_refused_ = false;
  bool warned_failed_ = false;
  double fill_ms_ = 0.0;
  double plot_ms_ = 0.0;
  double marks_ms_ = 0.0;
  double fallback_ms_ = kFillFallbackMs;
  double last_full_ms_ = -1.0;
  bool fell_back_ = false;
  int refills_ = 0;
  int fallback_fills_ = 0;
  int pick_sizings_ = 0;
  /* the position mark after the last fill whose spheroid marker nodes were
     prepared */
  uint64_t refill_position_ = 0;
  bool refill_known_ = false;
};

/* r = a x b for column-major 4 x 4 matrices */
void multiply(const double a[16], const double b[16], double r[16]);

}  // namespace rhi
}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_RHI_FRAME_H */
