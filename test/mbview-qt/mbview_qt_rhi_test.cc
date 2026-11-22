/*--------------------------------------------------------------------
 *    The MB-system:  mbview_qt_rhi_test.cc  11/15/2026
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
 * Test program of the QRhi backend of mbview-qt (built where MBVQ_HAVE_RHI is
 * defined). Every mode has a watchdog and exits by itself.
 *
 *   mbview_qt_rhi_test --model         the choice of the backend, the C
 *        accessors, the frame driver, the fill policy and the terrain plan on
 *        synthetic grids, without drawing (the QRhi views exist, but cannot
 *        draw on the offscreen platform); the soundings bench's files and
 *        tile order
 *   mbview_qt_rhi_test --widgets       the QRhi views without drawing: the API,
 *        the deferred drawing, the failure label and message, input before
 *        the first frame, the mouse buttons after a lost release; returns 77
 *        (skipped) on a platform where a QRhi can be made
 *   mbview_qt_rhi_test --render-null   the renderer with a Null QRhi: its
 *        resources, upload counts, split uploads, a failed create(), tile
 *        buffers that cannot be made, a tile without data, a grid of another
 *        size, the QRhi deleted under it, the time of each renderer's create()
 *   mbview_qt_rhi_test --render-metal  the renderer with a Metal QRhi:
 *        pixels against the node colours, holes, edits, pick against
 *        picture, incremental against from scratch, determinism, float
 *        colours; returns 77 (skipped) where no Metal QRhi can be made;
 *        --save file keeps the first picture
 *   The model, widgets, Null and Metal modes also check the profile and 3D
 *   soundings views (6d): the soundings plan, the frame drivers, the views
 *   without drawing, the soundings' uploads, the failed frames of both
 *   views' renderers and the pixels of both views. The model, widgets and
 *   Null modes also check (7b) the drawing of the 3D soundings that a frame
 *   records with the profile mode hidden from the core, against a fill with
 *   the mode set, the mode after such fills (also in the View menu), and
 *   the profile lines of soundings from three files.
 *   mbview_qt_rhi_test --display [--3d] [--save file] [grid]   a map window
 *        drawn with QRhi (needs a display, not in ctest): waits for the first
 *        frames, grabs the view, picks with the mouse, shows and hides the
 *        failure label, closes the window; with the profile of a two-point
 *        pick (its frame's width at the view's ratio, the width cap, also
 *        for a view wider than it at a frame, its picture against the
 *        offscreen renderer's) and the 3D soundings in a dock of a main
 *        window (point and line sizes at the view's ratio, the picture
 *        against the offscreen renderer's, the point-size control, an edit
 *        by a click, Dismiss with the memory it frees and the next soundings,
 *        Dismiss while the dock is closed and while a host docks the
 *        floating dock again, floated, docked again, ended while floating),
 *        the soundings' positions read once when the standalone soundings
 *        window opens and when it opens again after Dismiss, then a million
 *        soundings in that window ended by mb3dsoundings_end with the
 *        memory it frees, and a second map of the grid in a window of its
 *        own closed by mbview_destroy with the memory its view frees
 *   mbview_qt_rhi_test --quit close|quit|exit|float [--maps-only] [grid]
 *        a program with a QRhi map, its profile window and the 3D soundings
 *        in a floating dock of its main window open ends (needs a display,
 *        not in ctest): the map window closed as by its close button, then
 *        the event loop left (the closed window is deleted at aboutToQuit);
 *        the event loop left with the map open (QApplication::quit(), as
 *        Cmd-Q); the window closed and exit() called inside the event loop,
 *        as the macOS Quit menu does; or the event loop left with the map in
 *        a floating dock of the main window too; the main window is deleted
 *        last (QTBUG-151834). --maps-only: the map and its profile window
 *        alone, without soundings or a main window (except for "float"), as
 *        mbgrdviz-qt has them
 *   mbview_qt_rhi_test --bench [--3d] [--frames n] [--size WxH] [--nav list] [--gis N] [grid]
 *        frame times of a map window (needs a display, not in ctest; a
 *        measurement, not a test): the time to the first complete frame, then
 *        drags through the view's input, n measured motions each (60) - in 2D
 *        a pan, a zoom and a shading drag, in 3D an orbit and an exaggeration
 *        drag - and colour table changes; medians and 95th percentiles of the
 *        frames with every value, the parts of a frame before and after the
 *        view's paint, under QRhi with the renderer's phases, the waits
 *        around them, the GPU time and the nodes prepared, and the process
 *        memory. MB_RENDERER=legacy-gl measures the legacy OpenGL
 *        backend with the same script, where the legacy OpenGL drawing is
 *        built (77, skipped, elsewhere); WxH is the window size given to
 *        mbview (640x480); --gis N adds a GIS layer of N synthetic polyline
 *        parts of 20 to 100 points each (a fixed seed, steps of the larger
 *        cell size; N at most 100,000) inside the grid after the map's first
 *        complete frame, as mbgrdviz adds a GIS file, and reports the time
 *        to the first complete frame with it
 *   mbview_qt_rhi_test --bench-soundings n [--frames n] [--point-size p]
 *        [--files k] [--order pings|tiles]   frame times of the
 *        3D soundings view with n synthetic soundings (needs a display, not
 *        in ctest; a measurement): a rotation, an erase drag, an exaggeration
 *        drag, bias changes, a rotation and an erase drag with profiles
 *        shown, n steps each (60); also for the legacy OpenGL backend with
 *        MB_RENDERER=legacy-gl, where the legacy OpenGL drawing is built (77,
 *        skipped, elsewhere); p the point size of the soundings window's
 *        control under QRhi (0.5 to 8 in steps of 0.5; 3 if not given), the
 *        pings split into k files that overlap in space (1), the soundings
 *        handed over in ping order or in the order of 64 x 64 spatial tiles
 *        (then the rotation and the exaggeration drag only)
 *   Both benches report the time of the renderer's create() under QRhi.
 *   --display prints it for the map, profile and soundings views after
 *   their first frame (the soundings in their dock) and for the soundings
 *   view after each move of its float, dock and float sequence; --quit for
 *   the map and profile views after their first frame, for the soundings
 *   view in its dock and after it floated (not with --maps-only), and with
 *   float for the map view after its dock floated.
 *
 * The headless modes run on the offscreen platform unless QT_QPA_PLATFORM
 * names another; the modes with a window draw with the QRhi API that
 * MB_RENDERER names (the platform's API otherwise), the bench with the
 * legacy OpenGL backend for MB_RENDERER=legacy-gl, where the legacy OpenGL
 * drawing is built. The synthetic grid is a
 * seamount on a sloping plain, known in closed form, with a block of nodes
 * without data and some single ones.
 */

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QImage>
#include <QLabel>
#include <QMainWindow>
#include <QMetaMethod>
#include <QMouseEvent>
#include <QPointer>
#include <QRhiWidget>
#include <QScreen>
#include <QScrollArea>
#include <QSlider>
#include <QStatusBar>
#include <QTimer>
#include <QWindow>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>
#if defined(__APPLE__)
#include <dlfcn.h>
#include <mach/mach.h>
#include <malloc/malloc.h>
#endif

extern "C" {
#include "mb_aux.h"
#include "mb_define.h"
#include "mb_status.h"
#include "mbview.h"
/* the private view structure: the view-bounds counter, the contour flags and
   the secondary value of a node */
#ifdef _WIN32
#include <windows.h>
#endif
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"
#include "mbviewprivate.h"
/* the 3D soundings window's state: its camera, view switches and grab */
#include "Mb3dsdg.h"
#include "mb3dsoundingsprivate.h"
}
#include "mbv_draw_batch.h"
#include "mbv_input.h"
#include "mbv_render.h"
#include "mbvq.h"
#include "mbvq_backend.h"
#include "mbvq_bridge.h"
#include "mbvq_profile.h"
#include "mbvq_rhi.h"
#include "mbvq_rhi_frame.h"
#include "mbvq_rhi_glu.h"
#include "mbvq_rhi_model.h"
#include "mbview_qt_rhi_glu_fixtures.h"
#include "mbvq_soundings.h"
#include "mbvq_window.h"

namespace {

namespace rhi = mbvq::rhi;

int failures = 0;
int checks = 0;

void check(bool ok, const std::string &what) {
  checks++;
  std::printf("%s: %s\n", ok ? "ok  " : "FAIL", what.c_str());
  if (!ok)
    failures++;
}

int result() {
  std::printf("%d of %d checks passed\n", checks - failures, checks);
  return failures == 0 ? 0 : 1;
}

void spin(int ms) {
  QEventLoop loop;
  QTimer::singleShot(ms, &loop, &QEventLoop::quit);
  loop.exec();
}

/* the test ends after seconds even if it hangs */
void start_watchdog(int seconds, const char *mode) {
  std::thread([seconds, mode] {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    std::fprintf(stderr, "mbview_qt_rhi_test %s: watchdog, stopped after %d s\n", mode, seconds);
    std::fflush(stdout);
    std::_Exit(4);
  }).detach();
}

std::string text(const char *format, double value) {
  char buffer[64];
  std::snprintf(buffer, sizeof(buffer), format, value);
  return buffer;
}

/*--------------------------------------------------------------------*/
/* synthetic grids */
struct Grid {
  int columns = 241;
  int rows = 181;
  double xmin = 10.0;
  double xmax = 10.2;
  double ymin = 54.0;
  double ymax = 54.15;
  bool holes = true; /* a block of nodes without data, and some single ones */
  bool empty_tile = false; /* no data in the second tile: nodes i 0 to 128, j 128 to 256 */
  bool mode3d = false;
  int window_width = 640; /* the map's size as mbview_setwindowparms gets it */
  int window_height = 480;
};

double surface(double lon, double lat) {
  const double dx = (lon - 10.1) / 0.04;
  const double dy = (lat - 54.075) / 0.03;
  return -1000.0 + 300.0 * std::exp(-(dx * dx + dy * dy)) - 400.0 * (lon - 10.0);
}

/* the block of nodes without data: columns 30 % to 40 %, rows 60 % to 70 % */
bool in_hole(const Grid &grid, int i, int j) {
  return grid.holes && i >= grid.columns * 3 / 10 && i < grid.columns * 4 / 10 && j >= grid.rows * 6 / 10 &&
         j < grid.rows * 7 / 10;
}

bool no_data(const Grid &grid, int i, int j) {
  return in_hole(grid, i, j) || (grid.holes && (i * 7 + j * 13) % 97 == 0) ||
         (grid.empty_tile && i <= rhi::kTileCells && j >= rhi::kTileCells && j <= 2 * rhi::kTileCells);
}

int dismiss(size_t) { return MB_SUCCESS; }

struct mbview_struct *data_of(size_t instance) {
  struct mbview_struct *data = nullptr;
  int error = MB_ERROR_NO_ERROR;
  mbview_getdataptr(0, instance, &data, &error);
  return data;
}

/* open one mbview instance the way mbgrdviz does, with a synthetic grid, or
   with a GMT grid file; primary_marks (may be null): whether
   mbview_setprimarygrid raised the position, colour and mask marks */
bool open_grid(const Grid &grid, const char *grdfile, size_t *instance, bool *primary_marks = nullptr) {
  const int verbose = 0;
  int error = MB_ERROR_NO_ERROR;
  if (mbview_init(verbose, instance, &error) != MB_SUCCESS)
    return false;
  char title[] = "mbview_qt_rhi_test";
  mbview_setwindowparms(verbose, *instance, &dismiss, title, 100, 100, grid.window_width, grid.window_height, 100, 500,
                        5, 1, &error);

  int grid_projection_mode = MBV_PROJECTION_GEOGRAPHIC;
  char grid_projection_id[MB_PATH_MAXLINE] = "EPSG:4326";
  float nodatavalue = -99999.0f;
  int nxy = grid.columns * grid.rows, n_columns = grid.columns, n_rows = grid.rows;
  double zmin = 1.0e9, zmax = -1.0e9, xmin = grid.xmin, xmax = grid.xmax, ymin = grid.ymin, ymax = grid.ymax;
  double dx = (grid.xmax - grid.xmin) / (grid.columns - 1), dy = (grid.ymax - grid.ymin) / (grid.rows - 1);
  float *values = nullptr;
  std::vector<float> synthetic;
  if (grdfile != nullptr) {
    if (mb_read_gmt_grd(verbose, const_cast<char *>(grdfile), &grid_projection_mode, grid_projection_id, &nodatavalue,
                        &nxy, &n_columns, &n_rows, &zmin, &zmax, &xmin, &xmax, &ymin, &ymax, &dx, &dy, &values, nullptr,
                        nullptr, &error) != MB_SUCCESS) {
      std::fprintf(stderr, "cannot read %s\n", grdfile);
      return false;
    }
  }
  else {
    synthetic.resize(static_cast<size_t>(nxy));
    for (int i = 0; i < n_columns; i++)
      for (int j = 0; j < n_rows; j++) {
        const double z = surface(xmin + i * dx, ymin + j * dy);
        synthetic[static_cast<size_t>(i) * n_rows + j] = no_data(grid, i, j) ? nodatavalue : static_cast<float>(z);
        zmin = std::min(zmin, z);
        zmax = std::max(zmax, z);
      }
    values = synthetic.data();
  }

  /* display projection as in do_mbgrdviz_openprimary */
  int display_projection_mode = MBV_PROJECTION_GEOGRAPHIC;
  char display_projection_id[MB_PATH_MAXLINE];
  std::snprintf(display_projection_id, sizeof(display_projection_id), "EPSG:4326");
  if (grid_projection_mode == MBV_PROJECTION_PROJECTED) {
    display_projection_mode = grid_projection_mode;
    std::snprintf(display_projection_id, sizeof(display_projection_id), "%s", grid_projection_id);
  }
  else if (ymax > -80.0 && ymin < 84.0 && xmax - xmin <= 20.0 && ymax - ymin <= 20.0) {
    display_projection_mode = MBV_PROJECTION_PROJECTED;
    double lon_origin = 0.5 * (xmin + xmax);
    if (lon_origin > 180.0)
      lon_origin -= 360.0;
    std::snprintf(display_projection_id, sizeof(display_projection_id), "LTM%.5f/%.5f", lon_origin, 0.5 * (ymin + ymax));
  }

  int primary_colortable = MBV_COLORTABLE_HAXBY, primary_colortable_mode = MBV_COLORTABLE_NORMAL;
  int primary_shade_mode = MBV_SHADE_VIEW_SLOPE;
  int slope_colortable = MBV_COLORTABLE_HAXBY, slope_colortable_mode = MBV_COLORTABLE_REVERSED;
  int secondary_colortable = MBV_COLORTABLE_HAXBY, secondary_colortable_mode = MBV_COLORTABLE_NORMAL;
  double illuminate_magnitude = 1.0, illuminate_elevation = 5.0, illuminate_azimuth = 90.0, slope_magnitude = 1.0;
  mb_mbview_defaults(verbose, &primary_colortable, &primary_colortable_mode, &primary_shade_mode, &slope_colortable,
                     &slope_colortable_mode, &secondary_colortable, &secondary_colortable_mode, &illuminate_magnitude,
                     &illuminate_elevation, &illuminate_azimuth, &slope_magnitude);
  const double contour_interval = std::pow(10.0, std::floor(std::log10(zmax - zmin)) - 1.0);
  int status = mbview_setviewcontrols(
      verbose, *instance, grid.mode3d ? MBV_DISPLAY_3D : MBV_DISPLAY_2D, MBV_MOUSE_MOVE, MBV_GRID_VIEW_PRIMARY, false,
      false, false, primary_shade_mode, MBV_SHADE_VIEW_NONE, MBV_SHADE_VIEW_NONE, MBV_VIEW_OFF, MBV_VIEW_OFF,
      MBV_VIEW_OFF, MBV_VIEW_OFF, MBV_VIEW_OFF, MBV_VIEW_OFF, MBV_VIEW_OFF, 1.0, grid.mode3d ? 45.0 : 90.0, 0.0, 90.0,
      0.0, illuminate_magnitude, illuminate_elevation, illuminate_azimuth, slope_magnitude, 1.0, 0.0,
      MBV_COLORTABLE_NORMAL, contour_interval, display_projection_mode, display_projection_id, &error);
  uint64_t before[4] = {}, after[4] = {};
  mbview_getmarks(verbose, *instance, &before[0], &before[1], &before[2], &before[3], &error);
  if (status == MB_SUCCESS)
    status = mbview_setprimarygrid(verbose, *instance, grid_projection_mode, grid_projection_id, nodatavalue, n_columns,
                                   n_rows, zmin, zmax, xmin, xmax, ymin, ymax, dx, dy, values, &error);
  mbview_getmarks(verbose, *instance, &after[0], &after[1], &after[2], &after[3], &error);
  if (primary_marks != nullptr)
    *primary_marks = after[0] > before[0] && after[1] > before[1] && after[2] > before[2];
  if (grdfile != nullptr)
    mb_freed(verbose, __FILE__, __LINE__, reinterpret_cast<void **>(&values), &error);
  if (status == MB_SUCCESS)
    status = mbview_setprimarycolortable(verbose, *instance, primary_colortable, primary_colortable_mode, zmin, zmax, &error);
  if (status == MB_SUCCESS)
    status = mbview_setslopecolortable(verbose, *instance, slope_colortable, slope_colortable_mode, 0.0, 0.5, &error);
  if (status == MB_SUCCESS)
    status = mbview_open(verbose, *instance, &error);
  return status == MB_SUCCESS;
}

/* an overlay grid over the primary's area: columns x rows nodes (the
   primary's size gives a secondary grid with the primary's nodes), no data
   in a block of its own */
bool add_secondary(size_t instance, const Grid &grid, int columns, int rows) {
  int error = MB_ERROR_NO_ERROR;
  const float nodatavalue = -88888.0f;
  std::vector<float> values(static_cast<size_t>(columns) * rows);
  for (int i = 0; i < columns; i++)
    for (int j = 0; j < rows; j++) {
      const bool hole = i >= columns * 6 / 10 && i < columns * 7 / 10 && j >= rows * 2 / 10 && j < rows * 3 / 10;
      values[static_cast<size_t>(i) * rows + j] = hole ? nodatavalue : static_cast<float>(10.0 + i % 17 + j % 11);
    }
  char projection_id[MB_PATH_MAXLINE] = "EPSG:4326";
  const double dx = (grid.xmax - grid.xmin) / (columns - 1), dy = (grid.ymax - grid.ymin) / (rows - 1);
  int status = mbview_setsecondarygrid(0, instance, MBV_PROJECTION_GEOGRAPHIC, projection_id, nodatavalue, columns, rows,
                                       10.0, 37.0, grid.xmin, grid.xmax, grid.ymin, grid.ymax, dx, dy, values.data(),
                                       &error);
  if (status == MB_SUCCESS)
    status = mbview_setsecondarycolortable(0, instance, MBV_COLORTABLE_HAXBY, MBV_COLORTABLE_NORMAL, 10.0, 37.0, 1.0,
                                           23.5, MBV_COLORTABLE_NORMAL, &error);
  if (status == MB_SUCCESS)
    status = mbview_update(0, instance, &error);
  return status == MB_SUCCESS;
}

void close_instance(size_t instance) {
  mbvq_c_destroy(instance);
  spin(20);
}

/* the 3D dialog's Apply with another exaggeration, as a user would set it */
void set_exaggeration(size_t instance, double exaggeration) {
  struct mbvq_controls controls;
  mbvq_c_controls(instance, &controls);
  std::vector<std::string> fields(MBVQ_FIELDS);
  std::vector<const char *> pointers(MBVQ_FIELDS);
  for (int f = 0; f < MBVQ_FIELDS; f++) {
    fields[f] = text("%.10g", f == MBVQ_F_EXAGGERATION ? exaggeration : controls.field[f]);
    pointers[f] = fields[f].c_str();
  }
  mbvq_c_apply(instance, MBVQ_DIALOG_VIEW3D, pointers.data(), controls.toggle);
}

void set_contour_interval(size_t instance, double interval) {
  struct mbvq_controls controls;
  mbvq_c_controls(instance, &controls);
  std::vector<std::string> fields(MBVQ_FIELDS);
  std::vector<const char *> pointers(MBVQ_FIELDS);
  for (int f = 0; f < MBVQ_FIELDS; f++) {
    fields[f] = text("%.10g", f == MBVQ_F_CONTOURS ? interval : controls.field[f]);
    pointers[f] = fields[f].c_str();
  }
  mbvq_c_apply(instance, MBVQ_DIALOG_COLORBOUNDS, pointers.data(), controls.toggle);
}

/* mbview_setviewcontrols with the controls the view has: nothing changes but
   the marks */
void set_view_controls(size_t instance) {
  struct mbview_struct *data = data_of(instance);
  /* the core copies the id into the field it comes from: a copy first */
  char projection_id[MB_PATH_MAXLINE];
  std::snprintf(projection_id, sizeof(projection_id), "%s", data->display_projection_id);
  int error = MB_ERROR_NO_ERROR;
  mbview_setviewcontrols(0, instance, data->display_mode, data->mouse_mode, data->grid_mode, data->primary_histogram,
                         data->primaryslope_histogram, data->secondary_histogram, data->primary_shade_mode,
                         data->slope_shade_mode, data->secondary_shade_mode, data->grid_contour_mode,
                         data->site_view_mode, data->route_view_mode, data->nav_view_mode,
                         data->navswathbounds_view_mode, data->navdrape_view_mode, data->vector_view_mode,
                         data->exaggeration, data->modelelevation3d, data->modelazimuth3d, data->viewelevation3d,
                         data->viewazimuth3d, data->illuminate_magnitude, data->illuminate_elevation,
                         data->illuminate_azimuth, data->slope_magnitude, data->overlay_shade_magnitude,
                         data->overlay_shade_center, data->overlay_shade_mode, data->contour_interval,
                         data->display_projection_mode, projection_id, &error);
}

/* the overlay grid of add_secondary() again, without its block of no data */
bool fill_secondary(size_t instance, int columns, int rows) {
  int error = MB_ERROR_NO_ERROR;
  std::vector<float> values(static_cast<size_t>(columns) * rows);
  for (int i = 0; i < columns; i++)
    for (int j = 0; j < rows; j++)
      values[static_cast<size_t>(i) * rows + j] = static_cast<float>(10.0 + i % 17 + j % 11);
  return mbview_updatesecondarygrid(0, instance, columns, rows, values.data(), &error) == MB_SUCCESS;
}

/* the secondary value of every primary node with data, as the drawing reads
   it (no-data value elsewhere) */
std::vector<double> secondary_values(size_t instance) {
  struct mbview_world_struct *view = &mbviews[instance];
  struct mbview_struct *data = &view->data;
  const int nc = data->primary_n_columns, nr = data->primary_n_rows;
  std::vector<double> values(static_cast<size_t>(nc) * nr, data->secondary_nodatavalue);
  for (int i = 0; i < nc; i++)
    for (int j = 0; j < nr; j++)
      if (data->primary_data[i * nr + j] != data->primary_nodatavalue)
        mbview_getsecondaryvalue(view, data, i, j, &values[static_cast<size_t>(i) * nr + j]);
  return values;
}

/*--------------------------------------------------------------------*/
/* the terrain plan on a real instance, without drawing */
struct Planned {
  rhi::PlanStats stats;
  std::vector<rhi::TileUpload> uploads;
  bool more = false;
  /* the tiles that uploaded a stream: 0 positions, 1 colours, 2 indices */
  std::vector<int> tiles(int stream) const {
    std::vector<int> out;
    for (const rhi::TileUpload &upload : uploads)
      if ((stream == 0 && upload.positions) || (stream == 1 && upload.colours) || (stream == 2 && upload.indices))
        out.push_back(upload.tile);
    return out;
  }
  size_t bytes() const { return stats.bytes_positions + stats.bytes_colours + stats.bytes_indices; }
};

/* a frame of a plan: every tile at stride 1 (6a's plan: no levels, no
   culling), unless options are given */
rhi::PlanOptions stride1_options(size_t budget = rhi::kUploadBudget) {
  rhi::PlanOptions options;
  options.upload_budget = budget;
  options.force_stride1 = true;
  options.culling = false;
  return options;
}

Planned plan_frame(rhi::MapFrame &frame, rhi::TerrainPlan &plan, const rhi::PlanOptions &options,
                   rhi::PlanResult *result = nullptr) {
  Planned planned;
  frame.fill();
  rhi::FrameInput input;
  frame.input(&input, true);
  rhi::PlanResult own;
  rhi::PlanResult &r = result != nullptr ? *result : own;
  planned.more = plan.plan(input.terrain, options, &r);
  planned.stats = r.stats;
  planned.uploads = r.uploads;
  return planned;
}

Planned plan_frame(rhi::MapFrame &frame, rhi::TerrainPlan &plan, size_t budget = rhi::kUploadBudget) {
  return plan_frame(frame, plan, stride1_options(budget));
}

/* the options of a view's frame: levels, culling, no button down */
rhi::PlanOptions view_options(size_t budget = rhi::kUploadBudget) {
  rhi::PlanOptions options;
  options.upload_budget = budget;
  return options;
}

/* the level of a stride */
int level_of(int stride) {
  int level = 0;
  while (level + 1 < rhi::kLevels && rhi::level_stride(level) < stride)
    level++;
  return level;
}

/* what a renderer's tile buffers would hold, kept on the CPU: the uploads of
   a plan, frame after frame */
struct GpuCopy {
  int generation = -1;
  std::vector<std::vector<float>> xyz;
  std::vector<std::vector<uint16_t>> index;

  void apply(const rhi::TerrainPlan &plan, const Planned &planned) {
    if (generation != plan.generation()) {
      xyz.assign(plan.tiles().size(), {});
      index.assign(plan.tiles().size(), {});
      generation = plan.generation();
    }
    for (const rhi::TileUpload &upload : planned.uploads) {
      if (upload.positions)
        xyz[upload.tile] = upload.xyz;
      if (upload.indices)
        index[upload.tile] = upload.index;
    }
  }

  /* the references of drawn triangles to nodes whose copied position is not
     the core's, after preparing the nodes as a drawing would */
  int stale(size_t instance, const rhi::TerrainPlan &plan) const {
    struct mbvq_grid grid;
    mbvq_c_grid(instance, &grid);
    int error = MB_ERROR_NO_ERROR;
    mbview_prepare_nodes(0, instance, 0, grid.n_columns - 1, 0, grid.n_rows - 1, 1, &error);
    int n = 0;
    for (int t = 0; t < static_cast<int>(plan.tiles().size()); t++) {
      if (!plan.drawable(t))
        continue;
      const rhi::TerrainPlan::Tile &tile = plan.tiles()[t];
      for (uint16_t v : index[t]) {
        const int k = (tile.i0 + v / tile.nj) * grid.n_rows + tile.j0 + v % tile.nj;
        const size_t p = 3 * static_cast<size_t>(v);
        if (xyz[t].size() < p + 3 || xyz[t][p] != grid.x[k] || xyz[t][p + 1] != grid.y[k] || xyz[t][p + 2] != grid.z[k])
          n++;
      }
    }
    return n;
  }
};

/* the tiles whose node ranges meet a range of nodes */
std::vector<int> tiles_meeting(const rhi::TerrainPlan &plan, int i0, int i1, int j0, int j1) {
  std::vector<int> out;
  for (int t = 0; t < static_cast<int>(plan.tiles().size()); t++) {
    const rhi::TerrainPlan::Tile &tile = plan.tiles()[t];
    if (tile.i0 <= i1 && tile.i1 >= i0 && tile.j0 <= j1 && tile.j1 >= j0)
      out.push_back(t);
  }
  return out;
}

/* the tiles of a plan with triangles */
std::vector<int> tiles_with_triangles(const rhi::TerrainPlan &plan) {
  std::vector<int> out;
  for (int t = 0; t < static_cast<int>(plan.tiles().size()); t++)
    if (plan.drawable(t))
      out.push_back(t);
  return out;
}

std::string list(const std::vector<int> &values) {
  std::string out;
  for (int v : values)
    out += (out.empty() ? "" : " ") + std::to_string(v);
  return "{" + out + "}";
}

/* whether the drawing may use a node, stated independently of the accessor:
   primary data, and in the secondary grid mode secondary data */
bool node_valid(size_t instance, int i, int j) {
  struct mbview_world_struct *view = &mbviews[instance];
  struct mbview_struct *data = &view->data;
  const int k = i * data->primary_n_rows + j;
  if (data->primary_data[k] == data->primary_nodatavalue)
    return false;
  if (data->grid_mode != MBV_GRID_VIEW_SECONDARY)
    return true;
  double value = data->secondary_nodatavalue;
  if (data->secondary_sameas_primary)
    value = data->secondary_data[k];
  else
    mbview_getsecondaryvalue(view, data, i, j, &value);
  return value != data->secondary_nodatavalue;
}

/* the triangles of a fresh plan's index lists as node triples of the whole
   grid, against the A/B triangles of every cell where their nodes are valid */
bool indices_match(size_t instance, rhi::MapFrame &frame, int *count) {
  rhi::TerrainPlan plan;
  Planned planned = plan_frame(frame, plan, static_cast<size_t>(-1));
  const struct mbview_struct *data = data_of(instance);
  const int nr = data->primary_n_rows;
  std::vector<std::array<int, 3>> mine;
  for (const rhi::TileUpload &upload : planned.uploads) {
    const rhi::TerrainPlan::Tile &tile = plan.tiles()[upload.tile];
    for (size_t n = 0; n + 2 < upload.index.size(); n += 3) {
      std::array<int, 3> triangle;
      for (int v = 0; v < 3; v++) {
        const int local = upload.index[n + v];
        triangle[v] = (tile.i0 + local / tile.nj) * nr + tile.j0 + local % tile.nj;
      }
      mine.push_back(triangle);
    }
  }
  std::vector<std::array<int, 3>> brute;
  for (int i = 0; i + 1 < data->primary_n_columns; i++)
    for (int j = 0; j + 1 < nr; j++) {
      const int k = i * nr + j, l = (i + 1) * nr + j, m = k + 1, n = l + 1;
      const bool vk = node_valid(instance, i, j), vl = node_valid(instance, i + 1, j);
      const bool vm = node_valid(instance, i, j + 1), vn = node_valid(instance, i + 1, j + 1);
      if (vk && vl && vm)
        brute.push_back({k, l, m});
      if (vl && vn && vm)
        brute.push_back({l, n, m});
    }
  std::sort(mine.begin(), mine.end());
  std::sort(brute.begin(), brute.end());
  *count = static_cast<int>(brute.size());
  return !brute.empty() && mine == brute;
}

/*--------------------------------------------------------------------*/
/* tile levels, culling and the budgets of the terrain plan (6b) */

/* the frame drew one stride, every level of it as new as the marks */
bool one_fresh_stride(const rhi::TerrainPlan &plan, const rhi::PlanResult &result) {
  for (const rhi::LevelKey &key : result.drawn)
    if (rhi::level_stride(key.level) != result.stats.stride || !plan.drawable(key.tile, key.level))
      return false;
  return result.drawn.empty() == (result.stats.stride == 0);
}

/* the levels of a stride the frame planned, as the drawn levels name them */
std::vector<int> drawn_tiles(const rhi::PlanResult &result) {
  std::vector<int> out;
  for (const rhi::LevelKey &key : result.drawn)
    out.push_back(key.tile);
  return out;
}

/* lattice nodes of a level over every tile of a plan */
size_t lattice_nodes(const rhi::TerrainPlan &plan, int level) {
  size_t n = 0;
  for (const rhi::TerrainPlan::Tile &tile : plan.tiles())
    n += static_cast<size_t>(tile.levels[level].ni) * static_cast<size_t>(tile.levels[level].nj);
  return n;
}

/* the bytes a frame uploads for a level as the plan estimates them before
   building it: the stale streams, an index list not made yet at 12 bytes a
   cell */
size_t estimated_upload(const rhi::TerrainPlan &plan, int level, bool positions_rose = false) {
  size_t bytes = 0;
  for (const rhi::TerrainPlan::Tile &tile : plan.tiles()) {
    const rhi::TerrainPlan::Level &l = tile.levels[level];
    const bool positions = l.need_positions || positions_rose;
    if (!l.need_indices && (l.empty || (!positions && !l.need_colours)))
      continue;
    const size_t nodes = static_cast<size_t>(l.ni) * static_cast<size_t>(l.nj);
    const size_t indices = l.built ? l.index_count : 6 * static_cast<size_t>(l.ni - 1) * static_cast<size_t>(l.nj - 1);
    bytes += (positions ? 12 * nodes : 0) + (l.need_colours ? 4 * nodes : 0) + (l.need_indices ? 2 * indices : 0);
  }
  return bytes;
}

/* the bytes of a frame's uploads */
size_t upload_bytes(const rhi::PlanResult &result) {
  return result.stats.bytes_positions + result.stats.bytes_colours + result.stats.bytes_indices;
}

/* a drag through the core's input in a mouse mode: the button pressed at
   (200, 200), then motions down by dy pixels each */
void core_drag(size_t instance, int mouse_mode, int button, int motions, int dy) {
  mbvq_c_set_mouse_mode(instance, mouse_mode);
  struct mbv_input_event event = {MBV_INPUT_PRESS, button, 200, 200, 0};
  mbview_input(instance, &event);
  event.type = MBV_INPUT_MOTION;
  for (int m = 1; m <= motions; m++) {
    event.y = 200 + m * dy;
    mbview_input(instance, &event);
  }
}

void core_release(size_t instance, int button) {
  struct mbv_input_event event = {MBV_INPUT_RELEASE, button, 200, 200, 0};
  mbview_input(instance, &event);
  mbvq_c_set_mouse_mode(instance, MBV_MOUSE_MOVE);
}

/* the 2D camera: zoom and offset in display units, as the core's own
   handlers set them (no mark) */
void camera_2d(size_t instance, double zoom, double x, double y) {
  mbviews[instance].size2d = static_cast<float>(zoom);
  mbviews[instance].offset2d_x = static_cast<float>(x);
  mbviews[instance].offset2d_y = static_cast<float>(y);
}

/* the camera's projection of a point of display coordinates: normalised
   device coordinates and the clip w */
void project(size_t instance, const double p[3], double *nx, double *ny, double *w) {
  double projection[16], modelview[16], camera[16];
  mbvq_c_camera(instance, projection, modelview);
  rhi::multiply(projection, modelview, camera);
  double c[4];
  for (int r = 0; r < 4; r++)
    c[r] = camera[r] * p[0] + camera[4 + r] * p[1] + camera[8 + r] * p[2] + camera[12 + r];
  *w = c[3];
  *nx = c[0] / c[3];
  *ny = c[1] / c[3];
}

/* The largest edge of a stride-1 cell in physical pixels at a drawing size,
   from where the core places the nodes themselves (every node prepared at
   stride 1), over the cells between nodes with data in front of the camera,
   one of them in the view: the size the stride of the zoom follows, found
   without the plan's boxes. */
double largest_cell(size_t instance, int width, int height) {
  struct mbvq_grid grid;
  mbvq_c_grid(instance, &grid);
  int error = MB_ERROR_NO_ERROR;
  mbview_prepare_nodes(0, instance, 0, grid.n_columns - 1, 0, grid.n_rows - 1, 1, &error);
  double projection[16], modelview[16], camera[16];
  mbvq_c_camera(instance, projection, modelview);
  rhi::multiply(projection, modelview, camera);
  const size_t nodes = static_cast<size_t>(grid.n_columns) * grid.n_rows;
  std::vector<double> sx(nodes, 0.0), sy(nodes, 0.0);
  std::vector<char> front(nodes, 0), inside(nodes, 0);
  for (size_t k = 0; k < nodes; k++) {
    if (grid.data[k] == grid.nodata)
      continue;
    double c[4];
    for (int r = 0; r < 4; r++)
      c[r] = camera[r] * grid.x[k] + camera[4 + r] * grid.y[k] + camera[8 + r] * grid.z[k] + camera[12 + r];
    if (!(c[3] > 0.0))
      continue;
    front[k] = 1;
    sx[k] = 0.5 * width * c[0] / c[3];
    sy[k] = 0.5 * height * c[1] / c[3];
    inside[k] = std::fabs(c[0] / c[3]) <= 1.0 && std::fabs(c[1] / c[3]) <= 1.0;
  }
  double largest = 0.0;
  for (int i = 0; i < grid.n_columns; i++)
    for (int j = 0; j < grid.n_rows; j++) {
      const size_t k = static_cast<size_t>(i) * grid.n_rows + j;
      if (!front[k])
        continue;
      const size_t next[2] = {k + grid.n_rows, k + 1};
      const bool exists[2] = {i + 1 < grid.n_columns, j + 1 < grid.n_rows};
      for (int d = 0; d < 2; d++)
        if (exists[d] && front[next[d]] && (inside[k] || inside[next[d]]))
          largest = std::max(largest, std::hypot(sx[next[d]] - sx[k], sy[next[d]] - sy[k]));
    }
  return largest;
}

/* The stride of the zoom against that size (E9): a stride above 1 only
   where its cells stay under a pixel and, where coarsest is asked (2D, the
   plan's metric is exact there), the coarsest stride that does; within 2 %,
   since the plan takes a tile's cell edge along its sides. */
bool zoom_matches(int stride, double cell, bool coarsest) {
  const bool fine = stride == 1 || stride * cell < 1.02;
  const bool coarse = !coarsest || stride == 128 || 2.0 * stride * cell >= 0.98;
  return stride >= 1 && fine && coarse;
}

/* the nodes with data outside their tile's box, from where the core places
   them (every node prepared at stride 1; in the primary grid mode every node
   with data is prepared) */
int outside_boxes(size_t instance, const rhi::TerrainPlan &plan, double *worst) {
  struct mbvq_grid grid;
  mbvq_c_grid(instance, &grid);
  int error = MB_ERROR_NO_ERROR;
  mbview_prepare_nodes(0, instance, 0, grid.n_columns - 1, 0, grid.n_rows - 1, 1, &error);
  int outside = 0;
  *worst = 0.0;
  for (const rhi::TerrainPlan::Tile &tile : plan.tiles()) {
    if (!tile.data)
      continue;
    for (int i = tile.i0; i <= tile.i1; i++)
      for (int j = tile.j0; j <= tile.j1; j++) {
        const int k = i * grid.n_rows + j;
        if (grid.data[k] == grid.nodata)
          continue;
        const double p[3] = {grid.x[k], grid.y[k], grid.z[k]};
        bool in = true;
        for (int c = 0; c < 3; c++) {
          in = in && p[c] >= tile.lo[c] && p[c] <= tile.hi[c];
          *worst = std::max(*worst, std::max(tile.lo[c] - p[c], p[c] - tile.hi[c]));
        }
        outside += in ? 0 : 1;
      }
  }
  return outside;
}

/* the tile with first nodes i0 and j0 */
int tile_at(const rhi::TerrainPlan &plan, int i0, int j0) {
  for (int t = 0; t < static_cast<int>(plan.tiles().size()); t++)
    if (plan.tiles()[t].i0 == i0 && plan.tiles()[t].j0 == j0)
      return t;
  return -1;
}

/* the GPU bytes of a level over the tiles with data, as the plan counts them
   (one copy, colours as bytes): what it holds, or what it would make (12
   bytes a cell for an index list not made yet) */
size_t level_set_bytes(const rhi::TerrainPlan &plan, int level) {
  size_t bytes = 0;
  for (const rhi::TerrainPlan::Tile &tile : plan.tiles()) {
    const rhi::TerrainPlan::Level &l = tile.levels[level];
    if (!tile.data || (l.built && l.index_count == 0 && !l.need_indices))
      continue;
    const size_t nodes = static_cast<size_t>(l.ni) * static_cast<size_t>(l.nj);
    const size_t indices = l.built ? l.index_count : 6 * static_cast<size_t>(l.ni - 1) * static_cast<size_t>(l.nj - 1);
    bytes += l.resident ? l.bytes : 16 * nodes + 2 * indices;
  }
  return bytes;
}

/* the bytes a plan that made nothing yet counts for a level of the same
   grid: positions, colours and 12 bytes a cell, over the tiles with data */
size_t fresh_set_bytes(const rhi::TerrainPlan &plan, int level) {
  size_t bytes = 0;
  for (const rhi::TerrainPlan::Tile &tile : plan.tiles()) {
    const rhi::TerrainPlan::Level &l = tile.levels[level];
    if (tile.data)
      bytes += 16 * static_cast<size_t>(l.ni) * static_cast<size_t>(l.nj) +
               12 * static_cast<size_t>(l.ni - 1) * static_cast<size_t>(l.nj - 1);
  }
  return bytes;
}

/* the geometry of the camera alone: the box test and the pixels per unit */
void model_geometry() {
  check(std::fabs(rhi::drag_budget_ms(3.5) - (1000.0 / 60.0 - 5.5)) < 1.0e-12 && rhi::drag_budget_ms(20.0) == 4.0 &&
            std::fabs(rhi::drag_budget_ms(0.0) - (1000.0 / 60.0 - 2.0)) < 1.0e-12,
        "the drag budget: 1000/60 ms less the other costs of the frame before and a margin of 2 ms, at least 4 ms");
  check(rhi::zoom_level(1.0) == 0 && rhi::zoom_level(3.0) == 0 && rhi::zoom_level(0.6) == 0 &&
            rhi::zoom_level(0.3) == 1 && rhi::zoom_level(0.24) == 2 && rhi::zoom_level(1.0e-6) == 7 &&
            rhi::zoom_level(std::numeric_limits<double>::infinity()) == 0 && rhi::zoom_level(std::nan("")) == 0,
        "the stride of the zoom: 1 where a cell covers a pixel or more (or the size is unbounded), else the largest power "
        "of two up to 128 that keeps a cell under a pixel");

  /* an orthographic camera: 600 x 400 px for x -3 to 3 and y -2 to 2 */
  double ortho[16] = {};
  ortho[0] = 1.0 / 3.0;
  ortho[5] = 0.5;
  ortho[10] = -2.0 / 1005.0;
  ortho[14] = -995.0 / 1005.0;
  ortho[15] = 1.0;
  const rhi::ViewGeometry flat(ortho, 600, 400);
  const double inside_lo[3] = {-0.5, -0.5, -1.0}, inside_hi[3] = {0.5, 0.5, 1.0};
  const double right_lo[3] = {3.1, -0.5, -1.0}, right_hi[3] = {4.0, 0.5, 1.0};
  double h = 0.0, v = -1.0, any = 0.0;
  const bool bounded = flat.scale(inside_lo, inside_hi, &h, &v, &any);
  check(flat.visible(inside_lo, inside_hi) && !flat.visible(right_lo, right_hi) && bounded &&
            std::fabs(h - 100.0) < 1.0e-9 && v == 0.0 && std::fabs(any - 100.0) < 1.0e-9,
        "an orthographic camera: a box in the view and one beside it; 100 px per unit horizontally, none for z (" +
            text("%.6f", h) + ", " + text("%.6f", v) + ")");

  /* a perspective camera (40 degrees, 4:3, near 0.03, far 3000) 10 units
     above the plane z = 0, looking down */
  const double f = 1.0 / std::tan(20.0 * std::acos(-1.0) / 180.0);
  double perspective[16] = {};
  perspective[0] = f / (4.0 / 3.0);
  perspective[5] = f;
  perspective[10] = -(3000.0 + 0.03) / (3000.0 - 0.03);
  perspective[11] = -1.0;
  perspective[14] = -2.0 * 3000.0 * 0.03 / (3000.0 - 0.03) - 10.0 * perspective[10];
  perspective[15] = 10.0;
  const rhi::ViewGeometry deep(perspective, 800, 600);
  const double centre_lo[3] = {-0.1, -0.1, -0.5}, centre_hi[3] = {0.1, 0.1, 0.5};
  const double near_lo[3] = {-0.1, -0.1, 9.0}, near_hi[3] = {0.1, 0.1, 9.999};
  const double behind_lo[3] = {-0.1, -0.1, 10.5}, behind_hi[3] = {0.1, 0.1, 11.0};
  const double aside_lo[3] = {100.0, -0.1, -0.5}, aside_hi[3] = {101.0, 0.1, 0.5};
  const bool scaled = deep.scale(centre_lo, centre_hi, &h, &v, &any);
  /* at the nearest depth 9.5 the image of a unit across the line of sight
     is f x 300 / 9.5 px; off the axis the bound is larger */
  const double across = f * 300.0 / 9.5;
  double h2 = 0.0, v2 = 0.0, any2 = 0.0;
  check(deep.visible(centre_lo, centre_hi) && scaled && h >= across && h < 1.05 * across && any >= h &&
            !deep.scale(near_lo, near_hi, &h2, &v2, &any2) && !deep.visible(behind_lo, behind_hi) &&
            !deep.visible(aside_lo, aside_hi),
        "a perspective camera: a box on the axis seen at " + text("%.2f", h) + " px per unit (" + text("%.2f", across) +
            " at its nearest depth); a box reaching the near plane unbounded; boxes behind the eye and beside the "
            "view outside");
}

/* tile levels and the stride of the zoom on a real instance */
void model_levels() {
  Grid grid;
  grid.columns = 300;
  grid.rows = 200;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "6b: a 300 x 200 grid opens");
  rhi::MapFrame frame(instance);
  frame.size_core(2400, 1800);

  /* the lattice of each stride: anchored at node 0, full cells only */
  {
    rhi::TerrainPlan plan;
    rhi::PlanResult result;
    plan_frame(frame, plan, view_options(), &result);
    const std::vector<rhi::TerrainPlan::Tile> &tiles = plan.tiles();
    check(tiles.size() == 6 && tiles[0].levels[0].ni == 129 && tiles[0].levels[3].ni == 17 &&
              tiles[0].levels[7].ni == 2 && tiles[4].ni == 44 && tiles[4].levels[3].ni == 6 &&
              tiles[1].nj == 72 && tiles[1].levels[3].nj == 9 && tiles[1].levels[7].nj == 1,
          "levels: the lattice of stride s has (n - 1) / s + 1 nodes per side, so a last partial cell is left out (a tile "
          "of 44 columns has 6 nodes at stride 8)");
    check(result.stats.stride_zoom == 1 && result.stats.stride == 1 && result.drawn.size() == 6 &&
              one_fresh_stride(plan, result),
          "cells of 7 px: the stride of the zoom is 1, every tile drawn at it");
  }

  /* zoomed out: a coarser stride for the whole frame, its index lists the
     A/B triangles of the stride's lattice over the grid */
  for (int zoom = 0; zoom < 3; zoom++) {
    camera_2d(instance, zoom == 0 ? 0.05 : zoom == 1 ? 0.02 : 0.004, 0.0, 0.0);
    rhi::TerrainPlan plan;
    rhi::PlanOptions options = view_options(SIZE_MAX);
    options.culling = false;
    rhi::PlanResult result;
    plan_frame(frame, plan, options, &result);
    const int stride = result.stats.stride;
    const int level = stride > 0 ? level_of(stride) : 0;
    std::vector<std::array<int, 3>> mine;
    bool levels_alike = stride > 1;
    for (const rhi::TileUpload &upload : result.uploads) {
      levels_alike = levels_alike && upload.level == level;
      const rhi::TerrainPlan::Tile &tile = plan.tiles()[upload.tile];
      const int nj = tile.levels[level].nj;
      for (size_t n = 0; n + 2 < upload.index.size(); n += 3) {
        std::array<int, 3> triangle;
        for (int c = 0; c < 3; c++) {
          const int local = upload.index[n + c];
          triangle[c] = (tile.i0 + (local / nj) * stride) * grid.rows + tile.j0 + (local % nj) * stride;
        }
        mine.push_back(triangle);
      }
    }
    std::vector<std::array<int, 3>> brute;
    for (int i = 0; stride > 0 && i + stride <= grid.columns - 1; i += stride)
      for (int j = 0; j + stride <= grid.rows - 1; j += stride) {
        const int k = i * grid.rows + j, l = (i + stride) * grid.rows + j, m = k + stride, n = l + stride;
        const bool vk = !no_data(grid, i, j), vl = !no_data(grid, i + stride, j);
        const bool vm = !no_data(grid, i, j + stride), vn = !no_data(grid, i + stride, j + stride);
        if (vk && vl && vm)
          brute.push_back({k, l, m});
        if (vl && vn && vm)
          brute.push_back({l, n, m});
      }
    std::sort(mine.begin(), mine.end());
    std::sort(brute.begin(), brute.end());
    const double cell = largest_cell(instance, 2400, 1800);
    check(levels_alike && !brute.empty() && mine == brute && one_fresh_stride(plan, result) &&
              result.stats.stride == result.stats.stride_zoom && zoom_matches(stride, cell, true),
          "zoomed out: the frame at stride " + std::to_string(stride) + ", the coarsest that keeps the largest cell, " +
              text("%.3f", cell) + " px at stride 1, under a pixel; its index lists the A/B triangles of that "
              "lattice's full cells (" + std::to_string(brute.size()) + ")");
  }
  camera_2d(instance, 1.0, 0.0, 0.0);

  /* Levels left stale while another stride is drawn, by what is uploaded
     (not the plan's own flags): zoomed out at stride 2, then in at stride 1
     where an exaggeration change and a colour change come, then out again:
     every tile drawn at stride 2 uploads its positions and its colours in
     that frame. */
  {
    rhi::TerrainPlan plan;
    camera_2d(instance, 0.05, 0.0, 0.0);
    rhi::PlanResult first;
    plan_frame(frame, plan, view_options(), &first);
    camera_2d(instance, 1.0, 0.0, 0.0);
    rhi::PlanResult in;
    plan_frame(frame, plan, view_options(), &in);
    set_exaggeration(instance, 2.0);
    plan_frame(frame, plan, view_options());
    mbvq_c_view(instance, MBVQ_VIEW_COLORTABLE_BRIGHT, true);
    plan_frame(frame, plan, view_options());
    camera_2d(instance, 0.05, 0.0, 0.0);
    rhi::PlanResult again;
    plan_frame(frame, plan, view_options(), &again);
    const int level = level_of(first.stats.stride);
    std::vector<int> positions, colours;
    for (const rhi::TileUpload &upload : again.uploads) {
      if (upload.level == level && upload.positions)
        positions.push_back(upload.tile);
      if (upload.level == level && upload.colours)
        colours.push_back(upload.tile);
    }
    const std::vector<int> drawn = drawn_tiles(again);
    check(first.stats.stride == 2 && in.stats.stride == 1 && again.stats.stride == 2 && drawn.size() == 6 &&
              positions == drawn && colours == drawn,
          "levels of stride 2 left while an exaggeration and a colour change came at stride 1: drawn again, every "
          "one of them uploads its positions (" + std::to_string(positions.size()) + ") and colours (" +
              std::to_string(colours.size()) + ") of " + std::to_string(drawn.size()) + " tiles");
    camera_2d(instance, 1.0, 0.0, 0.0);
    set_exaggeration(instance, 1.0);
    mbvq_c_view(instance, MBVQ_VIEW_COLORTABLE_HAXBY, true);
  }

  /* culling: zoomed into the lower left of the grid, the tiles outside the
     view are neither prepared, gathered nor drawn; after a colour change
     their nodes keep colours not computed */
  {
    rhi::TerrainPlan plan;
    plan_frame(frame, plan, view_options());
    camera_2d(instance, 4.0, 1.0, 1.2);
    rhi::PlanResult moved;
    const Planned camera = plan_frame(frame, plan, view_options(), &moved);
    const int visible = moved.stats.tiles_visible, culled = moved.stats.tiles_culled;
    mbvq_c_view(instance, MBVQ_VIEW_COLORTABLE_BRIGHT, true);
    rhi::PlanResult recoloured;
    const Planned colours = plan_frame(frame, plan, view_options(), &recoloured);
    const std::vector<int> in_view = drawn_tiles(recoloured);
    const struct mbview_struct *data = data_of(instance);
    int prepared_out = 0, prepared_in = 0, nodes_out = 0, nodes_in = 0;
    for (int t = 0; t < static_cast<int>(plan.tiles().size()); t++) {
      const rhi::TerrainPlan::Tile &tile = plan.tiles()[t];
      const bool seen = std::find(in_view.begin(), in_view.end(), t) != in_view.end();
      for (int i = tile.i0; i <= tile.i1; i += 4)
        for (int j = tile.j0; j <= tile.j1; j += 4) {
          /* the nodes inside the tile, off the edges it shares */
          if (i == tile.i0 || i == tile.i1 || j == tile.j0 || j == tile.j1 || no_data(grid, i, j))
            continue;
          const int k = i * grid.rows + j;
          const bool coloured = (data->primary_stat_color[k / 8] & statmask[k % 8]) != 0;
          (seen ? nodes_in : nodes_out)++;
          (seen ? prepared_in : prepared_out) += coloured ? 1 : 0;
        }
    }
    check(camera.uploads.empty() && visible >= 1 && culled >= 1 && visible + culled == 6 &&
              colours.tiles(1) == in_view && static_cast<int>(in_view.size()) == visible &&
              one_fresh_stride(plan, recoloured) && nodes_out > 0 && prepared_out == 0 && prepared_in == nodes_in,
          "culling: zoomed in, " + std::to_string(visible) + " tiles in the view, " + std::to_string(culled) +
              " left out; a colour change prepares and uploads those in the view only (" +
              std::to_string(prepared_in) + " of " + std::to_string(nodes_in) + " nodes coloured in them, " +
              std::to_string(prepared_out) + " of " + std::to_string(nodes_out) + " outside)");
    camera_2d(instance, 1.0, 0.0, 0.0);
    rhi::PlanResult back;
    const Planned whole = plan_frame(frame, plan, view_options(), &back);
    check(back.drawn.size() == 6 && static_cast<int>(whole.tiles(1).size()) == culled && one_fresh_stride(plan, back),
          "the whole grid in the view again: the tiles left out are prepared and uploaded then");
  }

  close_instance(instance);
}

/* A grid of 2 x 6 tiles for a cell edit beyond the grid's z range in a tile
   below the view, seen from 25 degrees above it in 3D. */
Grid tall_grid() {
  Grid grid;
  grid.columns = 200;
  grid.rows = 700;
  grid.ymax = 54.5;
  return grid;
}

/* a node of such a tile, left out of the view, and a value above the grid's
   range that would bring it into the view */
struct Beside {
  int tile = -1;
  int i = 0;
  int j = 0;
  float value = 0.0f;
  int culled = 0; /* tiles left out of the view */
};

/* the model moved down and zoomed in step by step until a tile below the
   view, left out by a frame of a plan, has a node whose raised value would be
   seen; the camera stays there */
Beside find_beside(size_t instance, const Grid &grid, rhi::MapFrame &frame) {
  Beside beside;
  mbvq_c_view(instance, MBVQ_VIEW_DISPLAY_3D, true);
  struct mbview_struct *data = data_of(instance);
  data->modelelevation3d = 25.0;
  rhi::TerrainPlan plan;
  struct mbvq_grid nodes;
  for (int step = 0; step < 40 && beside.tile < 0; step++) {
    mbviews[instance].offset3d_z = static_cast<float>(0.5 * (step % 8));
    mbviews[instance].offset3d_y = static_cast<float>(-0.25 * (step / 8 + 1));
    rhi::PlanResult before;
    plan_frame(frame, plan, view_options(), &before);
    if (before.stats.tiles_culled == 0 || before.stats.tiles_visible == 0)
      continue;
    mbvq_c_grid(instance, &nodes);
    const std::vector<int> seen = drawn_tiles(before);
    for (int t = 0; t < static_cast<int>(plan.tiles().size()) && beside.tile < 0; t++) {
      if (std::find(seen.begin(), seen.end(), t) != seen.end() || !plan.tiles()[t].data)
        continue;
      const rhi::TerrainPlan::Tile &tile = plan.tiles()[t];
      for (int i = tile.i0 + 2; i < tile.i1 - 1 && beside.tile < 0; i += 3)
        for (int j = tile.j0 + 2; j < tile.j1 - 1 && beside.tile < 0; j += 3) {
          if (no_data(grid, i, j))
            continue;
          for (double above = 50.0; above < 5000.0 && beside.tile < 0; above *= 1.5) {
            const float value = static_cast<float>(data->primary_max + above);
            const int k = i * grid.rows + j;
            const double p[3] = {nodes.x[k], nodes.y[k], nodes.scale * (nodes.exaggeration * value - nodes.zorigin)};
            double nx = 0.0, ny = 0.0, w = 0.0;
            project(instance, p, &nx, &ny, &w);
            if (w > 0.0 && std::fabs(nx) < 0.9 && std::fabs(ny) < 0.9)
              beside = {t, i, j, value, before.stats.tiles_culled};
          }
        }
    }
  }
  return beside;
}

/* The tile's own z range rises with the edit, so the tile comes into the
   view (the grid's range, which a cell edit does not update, would keep it
   out). */
void model_edit_beside() {
  const Grid grid = tall_grid();
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "6b: a 200 x 700 grid opens for an edit beside the view");
  rhi::MapFrame frame(instance);
  frame.size_core(1200, 900);
  const Beside beside = find_beside(instance, grid, frame);
  rhi::TerrainPlan plan;
  plan_frame(frame, plan, view_options());
  int error = MB_ERROR_NO_ERROR;
  if (beside.tile >= 0)
    mbview_updateprimarygridcell(0, instance, beside.i, beside.j, beside.value, &error);
  rhi::PlanResult after;
  plan_frame(frame, plan, view_options(), &after);
  const std::vector<int> drawn = drawn_tiles(after);
  const struct mbview_struct *data = data_of(instance);
  check(beside.tile >= 0 && beside.value > data->primary_max && plan.tiles()[beside.tile].data_max == beside.value &&
            std::find(drawn.begin(), drawn.end(), beside.tile) != drawn.end() && one_fresh_stride(plan, after),
        "a cell edit to " + text("%.0f", beside.value) + " m, above the grid's range (to " +
            text("%.0f", data->primary_max) + " m), in tile " + std::to_string(beside.tile) + " below the view (3D, " +
            std::to_string(beside.culled) + " tiles left out): the tile's own range rises and it is drawn");
  close_instance(instance);
}

/* The zoom stride in oblique and steep 3D views, where tiles differ in size
   and z projects: never coarser than the cells allow (the vertical edge
   counted, the largest cell over the tiles in the view), against the nodes
   the core places (T2-1). */
void model_oblique() {
  struct View {
    bool tall;
    double exaggeration;
    double elevation;
    double offset_z;
  };
  const View views[] = {{false, 20.0, 45.0, -20.0}, {false, 20.0, 10.0, -20.0}, {true, 1.0, 20.0, -2.0}};
  for (const View &view : views) {
    Grid grid = view.tall ? tall_grid() : Grid();
    grid.holes = false;
    size_t instance = MBV_NO_WINDOW;
    if (!open_grid(grid, nullptr, &instance)) {
      check(false, "a grid opens for an oblique 3D view");
      continue;
    }
    mbvq_c_view(instance, MBVQ_VIEW_DISPLAY_3D, true);
    if (view.exaggeration != 1.0)
      set_exaggeration(instance, view.exaggeration);
    data_of(instance)->modelelevation3d = view.elevation;
    mbviews[instance].offset3d_z = static_cast<float>(view.offset_z);
    rhi::MapFrame frame(instance);
    frame.size_core(400, 300);
    rhi::TerrainPlan plan;
    rhi::PlanResult r;
    plan_frame(frame, plan, view_options(), &r);
    const double cell = largest_cell(instance, 400, 300);
    check(r.stats.tiles_visible > 1 && zoom_matches(r.stats.stride_zoom, cell, false) && r.stats.stride == r.stats.stride_zoom,
          std::string("3D, ") + (view.tall ? "the 200 x 700 grid" : "the synthetic grid") + ", exaggeration " +
              text("%.0f", view.exaggeration) + ", looking " + text("%.0f", view.elevation) + " degrees down, " +
              std::to_string(r.stats.tiles_visible) + " tiles in the view: stride of the zoom " +
              std::to_string(r.stats.stride_zoom) + ", the largest stride-1 cell " + text("%.3f", cell) +
              " px (never coarser than the cells allow)");
    close_instance(instance);
  }
}

/* the drag budget with fake costs per node: preparing time in the primary
   and the secondary grid mode, and upload bytes */
void model_drag_budget() {
  Grid grid;
  grid.columns = 300;
  grid.rows = 200;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "6b: a 300 x 200 grid opens for the drag budget");
  rhi::MapFrame frame(instance);
  frame.size_core(2400, 1800);
  rhi::TerrainPlan plan;
  plan_frame(frame, plan, view_options());

  /* The fake costs leave several ms between the strides chosen and the
     budget, and every expectation takes the frame's own measured time for
     the tiles' bounds (T2-2): a frame descheduled there may draw coarser,
     never fail a check. */
  rhi::PlanOptions drag = view_options();
  drag.drag = true;
  drag.budget_ms = 6.0;
  drag.node_ns_colour = 2000.0;
  drag.node_ns_z = 1.0;
  /* the finest stride whose nodes cost at most the budget at a cost per
     node, beside the time the frame took for the tiles' bounds */
  const auto expected = [&](double ns, double bounds_ms) {
    for (int l = 0; l < rhi::kLevels; l++)
      if (bounds_ms + 1.0e-6 * ns * static_cast<double>(lattice_nodes(plan, l)) <= drag.budget_ms)
        return rhi::level_stride(l);
    return 128;
  };

  /* a shading drag: colours at 1000 ns a node */
  core_drag(instance, MBV_MOUSE_SHADE, 3, 1, 20);
  rhi::PlanResult shaded;
  plan_frame(frame, plan, drag, &shaded);
  const int shade_stride = expected(2000.0, shaded.stats.bounds_ms);
  check(shaded.stats.stride == shade_stride && shade_stride > 1 && shaded.stats.stride_zoom == 1 &&
            shaded.stats.drag_coarser && shaded.stats.budget_ms == drag.budget_ms &&
            shaded.stats.estimated_ms <= drag.budget_ms + 1.0e-9 && one_fresh_stride(plan, shaded),
        "the drag budget, primary grid mode: a shading drag at 2000 ns a node in 6 ms draws stride " +
            std::to_string(shaded.stats.stride) + " (the finest whose " +
            std::to_string(lattice_nodes(plan, level_of(shade_stride))) + " nodes fit beside " +
            text("%.4f", shaded.stats.bounds_ms) + " ms of bounds), estimated " +
            text("%.3f", shaded.stats.estimated_ms) + " ms");
  /* A budget per stride, as a view gives one where the latest frame at a
     stride cost more outside its preparing (R7): a stride is weighed with
     its own budget, and the stride planned reports it. */
  {
    rhi::PlanOptions own = drag;
    own.budget_ms = 1000.0;
    core_drag(instance, MBV_MOUSE_SHADE, 3, 1, 15);
    rhi::PlanResult open;
    plan_frame(frame, plan, own, &open);
    own.level_budget_ms[0] = 0.0;
    core_drag(instance, MBV_MOUSE_SHADE, 3, 1, 25);
    rhi::PlanResult weighed;
    plan_frame(frame, plan, own, &weighed);
    own.level_budget_ms[1] = 500.0;
    core_drag(instance, MBV_MOUSE_SHADE, 3, 1, 35);
    rhi::PlanResult reported;
    plan_frame(frame, plan, own, &reported);
    core_release(instance, 3);
    check(open.stats.stride == 1 && open.stats.budget_ms == 1000.0 && weighed.stats.stride == 2 &&
              weighed.stats.budget_ms == 1000.0 && reported.stats.stride == 2 && reported.stats.budget_ms == 500.0,
          "a budget per stride (a view's, where the latest frame at a stride cost more besides its preparing): "
          "stride 1 in a budget of 1000 ms, stride 2 when stride 1 alone has none; the stride planned reports its "
          "own budget (" + text("%.0f", reported.stats.budget_ms) + " ms)");
  }
  core_drag(instance, MBV_MOUSE_SHADE, 3, 1, 10);
  rhi::PlanResult again;
  plan_frame(frame, plan, drag, &again);
  core_release(instance, 3);
  rhi::PlanResult released;
  plan_frame(frame, plan, view_options(), &released);
  check(again.stats.stride == expected(2000.0, again.stats.bounds_ms) && released.stats.stride == 1 &&
            released.stats.budget_ms < 0.0 &&
            !released.stats.drag_coarser && released.stats.tiles_waiting == 0 && one_fresh_stride(plan, released),
        "the next motion likewise; after the release the frame plans at the stride of the zoom without the budget, in "
        "one frame");

  /* An exaggeration drag (3D only: in 2D the rotate mode is the move mode)
     raises the position mark alone: z at 1 ns a node. */
  mbvq_c_view(instance, MBVQ_VIEW_DISPLAY_3D, true);
  rhi::PlanResult three;
  plan_frame(frame, plan, view_options(), &three);
  check(three.stats.stride == 1 && three.stats.tiles_visible == 6,
        "in 3D, looking down on the whole grid, the stride of the zoom is 1");
  core_drag(instance, MBV_MOUSE_ROTATE, 3, 1, 20);
  rhi::PlanResult raised;
  plan_frame(frame, plan, drag, &raised);
  core_release(instance, 3);
  plan_frame(frame, plan, view_options());
  check(raised.stats.stride == expected(1.0, raised.stats.bounds_ms) && !raised.stats.drag_coarser &&
            raised.stats.estimated_ms <= drag.budget_ms &&
            one_fresh_stride(plan, raised),
        "the drag budget, primary grid mode: an exaggeration drag (z only, 1 ns a node, " +
            text("%.3f", raised.stats.estimated_ms) + " ms) stays at stride 1");

  /* the secondary grid mode recolours every node at stride 1, so a z-only
     mark costs a colour mark there (F4); stride 2 costs z only */
  check(add_secondary(instance, grid, grid.columns, grid.rows), "an overlay grid on the primary's nodes");
  mbvq_c_view(instance, MBVQ_VIEW_GRID_SECONDARY, true);
  plan_frame(frame, plan, view_options());
  core_drag(instance, MBV_MOUSE_ROTATE, 3, 1, 20);
  rhi::PlanResult secondary;
  plan_frame(frame, plan, drag, &secondary);
  core_release(instance, 3);
  plan_frame(frame, plan, view_options());
  const size_t nodes1 = lattice_nodes(plan, 0), nodes2 = lattice_nodes(plan, 1);
  check(secondary.stats.drag_coarser && 1.0e-6 * 2000.0 * static_cast<double>(nodes1) > drag.budget_ms &&
            (secondary.stats.stride == 2 ||
             secondary.stats.bounds_ms + 1.0e-6 * static_cast<double>(nodes2) > drag.budget_ms) &&
            one_fresh_stride(plan, secondary),
        "the drag budget, secondary grid mode: the same exaggeration drag costs a recolouring at stride 1 (" +
            std::to_string(nodes1) + " nodes at 2000 ns) and draws stride 2 (z only): stride " +
            std::to_string(secondary.stats.stride));
  mbvq_c_view(instance, MBVQ_VIEW_GRID_PRIMARY, true);
  plan_frame(frame, plan, view_options());

  /* the upload bytes: an exaggeration drag whose position uploads exceed a
     small budget draws one fresh uniform stride whose uploads fit, never a
     level older than the marks (time does not bind here) */
  rhi::PlanOptions bytes = drag;
  bytes.node_ns_colour = bytes.node_ns_z = 0.0;
  bytes.upload_budget = 200 * 1024;
  core_drag(instance, MBV_MOUSE_ROTATE, 3, 1, 20);
  int fits = -1;
  const size_t positions1 = estimated_upload(plan, 0, true);
  for (int l = 0; l < rhi::kLevels && fits < 0; l++)
    if (estimated_upload(plan, l, true) <= bytes.upload_budget)
      fits = l;
  rhi::PlanResult moved;
  plan_frame(frame, plan, bytes, &moved);
  core_release(instance, 3);
  bool older = false;
  for (const rhi::LevelKey &key : moved.drawn)
    older = older || !plan.ready(key.tile, key.level);
  check(fits > 0 &&
            (moved.stats.stride == rhi::level_stride(fits) || moved.stats.bounds_ms > bytes.budget_ms) &&
            moved.stats.drag_coarser &&
            upload_bytes(moved) <= bytes.upload_budget && moved.stats.tiles_waiting == 0 && !older &&
            moved.drawn.size() == 6 && one_fresh_stride(plan, moved),
        "the drag budget, upload bytes: positions of " + std::to_string(positions1 / 1024) +
            " kB at stride 1 over a 200 kB budget; drawn at stride " + std::to_string(moved.stats.stride) +
            ", all of it fresh, " + std::to_string(upload_bytes(moved) / 1024) + " kB uploaded");
  plan_frame(frame, plan, view_options());
  mbvq_c_view(instance, MBVQ_VIEW_DISPLAY_2D, true);
  close_instance(instance);
}

/* 7c-1: the rule of the GPU-time fallback (GpuFallback), and the APIs on
   which the paint's wait is the view's GPU time */
void model_gpu_fallback() {
  const double mark = rhi::kDragFrameMs - rhi::kDragMarginMs;
  const double e = 1.0e-9;
  check(!rhi::GpuFallback::slow(10.0, mark) && rhi::GpuFallback::slow(10.0, mark + e) &&
            rhi::GpuFallback::slow(rhi::kGpuMinimumMs, 16.0) &&
            !rhi::GpuFallback::slow(rhi::kGpuMinimumMs - e, 16.0) &&
            rhi::GpuFallback::slow(5.0 + e, mark + 5.0) && !rhi::GpuFallback::slow(5.0, mark + 5.0),
        "7c: a frame is GPU-slow when its total exceeds " + text("%.3f", mark) +
            " ms, its wait is 4 ms or more and the rest stays under " + text("%.3f", mark) +
            " ms (each bound held and crossed by 1e-9 ms)");

  rhi::GpuFallback two(4);
  two.start();
  two.frame(0, 10.0, 16.0);
  const unsigned after_one = two.marks();
  two.frame(0, 10.0, 16.0);
  const unsigned after_two = two.marks();
  const int level_two = two.level();
  rhi::GpuFallback reset(4);
  reset.start();
  reset.frame(0, 10.0, 16.0);
  reset.frame(0, 2.0, 10.0);
  reset.frame(0, 10.0, 16.0);
  rhi::GpuFallback between(4);
  between.start();
  between.frame(0, 10.0, 16.0);
  between.frame(1, 10.0, 16.0);
  between.frame(0, 10.0, 16.0);
  rhi::GpuFallback skipped(4);
  skipped.start();
  skipped.frame(0, 10.0, 16.0);
  skipped.frame(0, 1.0, 2.0, true);
  skipped.frame(0, 10.0, 16.0);
  rhi::GpuFallback skipped_only(4);
  skipped_only.start();
  skipped_only.frame(0, 10.0, 16.0, true);
  skipped_only.frame(0, 10.0, 16.0, true);
  check(after_one == 0 && after_two == 1u && level_two == 1 && reset.marks() == 0 && between.marks() == 0 &&
            skipped.marks() == 1u && skipped_only.marks() == 0,
        "7c: two GPU-slow frames in a row mark a level (marks " + std::to_string(after_two) + ", the next level " +
            std::to_string(level_two) + "), one does not; a frame not slow, or one at another level, between two "
            "slow ones starts the count again; a skipped frame (its wait holds uploads) neither counts nor resets");

  /* no finer level during a press; start() clears */
  for (int f = 0; f < 3; f++)
    two.frame(1, 1.0, 5.0);
  const int kept = two.level();
  two.start();
  check(kept == 1 && two.marks() == 0 && two.level() == 0 && !two.stuck(),
        "7c: fast frames after a mark keep the coarser level for the press (level " + std::to_string(kept) +
            "); start() at the next press clears the marks");

  /* the gain check: a step must cut the wait by a tenth */
  rhi::GpuFallback gain(4);
  gain.start();
  gain.frame(0, 10.0, 16.0);
  gain.frame(0, 10.0, 16.0);
  const double exact = (1.0 - rhi::kGpuGainShare) * 10.0;
  gain.frame(1, exact, 16.0);
  gain.frame(1, exact, 16.0);
  const unsigned gained = gain.marks();
  gain.frame(2, exact, 16.0);
  rhi::GpuFallback constant(4);
  constant.start();
  for (int f = 0; f < 6; f++)
    constant.frame(constant.level(), 12.0, 16.0);
  rhi::GpuFallback mixed(4);
  mixed.start();
  mixed.frame(0, 10.0, 16.0);
  mixed.frame(0, 10.0, 16.0);
  mixed.frame(1, 8.0, 16.0);
  mixed.frame(1, 9.5, 16.0);
  check(gained == 3u && gain.marks() == 3u && gain.level() == 2 && gain.stuck() && constant.marks() == 1u &&
            constant.level() == 1 && constant.stuck() && mixed.marks() == 1u && mixed.level() == 1 && mixed.stuck(),
        "7c: the gain check: a wait of exactly 0.9 x the marked level's counts (marks " + std::to_string(gained) +
            "), the same wait one level on stops the press there; constant waits allow one step; a slow frame with "
            "the gain, then one without: no further mark");

  /* the coarsest level is never marked: the soundings' 4, the map's 7 */
  rhi::GpuFallback soundings(4);
  soundings.start();
  rhi::GpuFallback map(rhi::kLevels - 1);
  map.start();
  /* waits falling by 15 % per level (the gain check asks for 10 %), two
     frames at each level, and still 4 ms or more at level 7 (4.49 ms), so
     that the frames at either coarsest level are GPU-slow and only the rule
     keeps it unmarked */
  double wait = 14.0;
  const bool coarsest_slow = rhi::GpuFallback::slow(14.0 * std::pow(0.85, rhi::kLevels - 1), 16.0);
  for (int l = 0; l < rhi::kLevels; l++) {
    for (int f = 0; f < rhi::kGpuSlowFrames; f++) {
      soundings.frame(soundings.level(), wait, 16.0);
      map.frame(map.level(), wait, 16.0);
    }
    wait *= 0.85;
  }
  check(coarsest_slow && soundings.marks() == 15u && soundings.level() == 4 && map.marks() == 127u &&
            map.level() == rhi::kLevels - 1,
        "7c: waits falling by 15 % per level walk the soundings to level 4 (every 16th sounding) and the map to "
        "level 7 (stride 128); neither coarsest level is ever marked, though their frames are GPU-slow (marks " +
            std::to_string(soundings.marks()) + " and " + std::to_string(map.marks()) + ")");

  /* within one zoom, a frame at a level finer than the one whose frames
     set the gain check's reference (6b's drag budget or the memory budget
     admitting a finer level again) is no step of the rule: it counts as any
     other frame, two such frames mark that level, and their waits become
     the reference, at their level, which judges the next step */
  bool finer_ok = true;
  unsigned finer_marks = 0;
  bool finer_stuck = false;
  for (const int zoom : {0, 1}) {
    rhi::GpuFallback finer(rhi::kLevels - 1);
    finer.start();
    finer.frame(3, 10.0, 16.0, false, zoom);
    finer.frame(3, 10.0, 16.0, false, zoom);
    finer.frame(4, 5.0, 9.0, false, zoom);
    finer.frame(1, 12.0, 16.0, false, zoom);
    const bool finer_first = !finer.stuck(zoom) && finer.marks(zoom) == 8u;
    finer.frame(1, 12.0, 16.0, false, zoom);
    finer_marks = finer.marks(zoom);
    finer.frame(2, 10.5, 16.0, false, zoom);
    const bool finer_free = !finer.stuck(zoom);
    finer.frame(2, 11.0, 16.0, false, zoom);
    finer_stuck = finer.stuck(zoom);
    finer_ok = finer_ok && finer_first && finer_marks == 10u && finer_free && finer_stuck &&
               finer.marks(zoom) == 10u && finer.marks(1 - zoom) == 0u && !finer.stuck(1 - zoom);
  }
  check(finer_ok,
        "7c: after level 3 is marked (10 ms), two GPU-slow frames at level 1 (12 ms) are no step of the rule: they "
        "mark level 1 (marks " + std::to_string(finer_marks) + "); a step to level 2 is then judged by level 1's "
        "12 ms: 10.5 ms passes, 11.0 ms stops that zoom (stuck " + std::to_string(finer_stuck) + "); the same at "
        "zoom 0 and at zoom 1, the other zoom untouched");

  /* each zoom's level has a rule of its own (the map's stride of the
     zoom): after level 1 is marked at zoom 1 (12 ms), the zoom's own level 2
     at zoom 2 (the map zoomed out) is judged by frames at zoom 2 alone,
     which mark it there with 11.5 ms (0.96 x 12 ms); a step at zoom 2 is
     then judged by those and stops zoom 2, not zoom 1 */
  rhi::GpuFallback zoomed(rhi::kLevels - 1);
  zoomed.start();
  zoomed.frame(1, 12.0, 16.0, false, 1);
  zoomed.frame(1, 12.0, 16.0, false, 1);
  zoomed.frame(2, 6.0, 10.0, false, 1);
  zoomed.frame(2, 11.5, 16.0, false, 2);
  zoomed.frame(2, 11.5, 16.0, false, 2);
  const unsigned zoomed_marks = zoomed.marks(2);
  const bool zoomed_free = !zoomed.stuck(2);
  zoomed.frame(3, 11.5, 16.0, false, 2);
  check(zoomed.marks(1) == 2u && zoomed_marks == 4u && zoomed_free && zoomed.stuck(2) && !zoomed.stuck(1) &&
            zoomed.marks(0) == 0u,
        "7c: after level 1 is marked at zoom 1, two GPU-slow frames at the zoom's own level 2 after a zoom out, "
        "with 0.96 x level 1's wait, are judged at zoom 2 alone and mark it there (marks " +
            std::to_string(zoomed.marks(1)) + " at zoom 1, " + std::to_string(zoomed_marks) + " at zoom 2); a step "
            "at zoom 2 with the same wait stops zoom 2, and zoom 1 goes on");

  /* GPU-slow frames count at each zoom on their own: a frame at level 1 at
     zoom 1, then one at zoom 2, mark nothing; a second at zoom 2 marks level
     1 there with the waits at zoom 2 alone (10 ms), so that a step there
     waiting 8.5 ms (15 % less) is free; a count across the zooms (8 and
     10 ms) would have marked with 9 ms and stopped it */
  rhi::GpuFallback restarted(rhi::kLevels - 1);
  restarted.start();
  restarted.frame(1, 8.0, 16.0, false, 1);
  restarted.frame(1, 10.0, 16.0, false, 2);
  const unsigned restarted_first = restarted.marks(1) | restarted.marks(2);
  restarted.frame(1, 10.0, 16.0, false, 2);
  const unsigned restarted_marks = restarted.marks(2);
  restarted.frame(2, 8.5, 16.0, false, 2);
  check(restarted_first == 0u && restarted_marks == 2u && restarted.marks(1) == 0u && !restarted.stuck(2),
        "7c: GPU-slow frames count at each zoom on their own: one at zoom 1 and one at zoom 2 mark nothing (marks " +
            std::to_string(restarted_first) + "), a second at zoom 2 marks level 1 there with zoom 2's 10 ms, and "
            "a step there waiting 8.5 ms is free (stuck " + std::to_string(restarted.stuck(2)) + ")");

  /* a zoom that changes back and forth under constant waits (12 ms, total
     16 ms): each zoom marks its own level in its own two frames, steps once
     and stops there, whatever the other zooms do, so no zoom ends more than
     one level past its own and none walks the levels; each frame draws the
     finest level not marked at its zoom from the zoom's level on, as the
     stride loop takes it */
  const auto flipping = [](const std::vector<int> &zooms) {
    rhi::GpuFallback g(rhi::kLevels - 1);
    g.start();
    std::vector<int> levels;
    for (const int zoom : zooms) {
      int level = zoom;
      while (level < rhi::kLevels - 1 && (g.marks(zoom) >> level & 1u) != 0)
        level++;
      levels.push_back(level);
      g.frame(level, 12.0, 16.0, false, zoom);
    }
    return levels;
  };
  const std::vector<int> every = flipping({0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0});
  const std::vector<int> pairs = flipping({0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1});
  const std::vector<int> apart = flipping({0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2});
  const std::vector<int> every_expected = {0, 0, 1, 1, 1, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1};
  const std::vector<int> pairs_expected = {0, 0, 1, 1, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2};
  const std::vector<int> apart_expected = {0, 0, 2, 2, 1, 1, 3, 3, 1, 1, 3, 3};
  check(every == every_expected && pairs == pairs_expected && apart == apart_expected,
        "7c: under constant waits each zoom marks its own level in its own two frames, steps once and stops: a "
        "zoom changing every frame or every second frame between 0 and 1 ends at level 1 at zoom 0 and level 2 at "
        "zoom 1, and between 0 and 2 at levels 1 and 3, one past each zoom's own level, never walking the levels "
        "(levels " + list(every) + ", " + list(pairs) + " and " + list(apart) + ")");

  /* a zoom in from zoom 6 (the map's stride of the zoom 64) under constant
     waits, three frames at zoom 6 and five at each finer zoom: at each zoom
     two frames mark its own level, the step after them is judged and stops
     that zoom (also the step into the coarsest level at zoom 6, which only
     its stop shows), its later frames keep the level reached, and a stop at
     one zoom does not block the next */
  rhi::GpuFallback swept(rhi::kLevels - 1);
  swept.start();
  std::vector<int> sweep_zooms = {6, 6, 6};
  std::vector<int> sweep_expected = {6, 6, 7};
  for (int zoom = 5; zoom >= 0; zoom--)
    for (const int f : {0, 0, 1, 1, 1}) {
      sweep_zooms.push_back(zoom);
      sweep_expected.push_back(zoom + f);
    }
  std::vector<int> sweep;
  for (const int zoom : sweep_zooms) {
    int level = zoom;
    while (level < rhi::kLevels - 1 && (swept.marks(zoom) >> level & 1u) != 0)
      level++;
    sweep.push_back(level);
    swept.frame(level, 12.0, 16.0, false, zoom);
  }
  std::vector<int> stops;
  for (int zoom = 0; zoom <= 6; zoom++)
    stops.push_back(swept.stuck(zoom) ? 1 : 0);
  check(sweep == sweep_expected && stops == std::vector<int>(7, 1),
        "7c: a zoom in from zoom 6 under constant waits, three frames at zoom 6 and five at each finer zoom: at "
        "each zoom two frames mark its own level, the step after them is judged and stops that zoom (also the "
        "step into level 7 at zoom 6), and its later frames keep the level reached (levels " + list(sweep) +
            "; stops at zooms 0 to 6 " + list(stops) + ")");

  /* a GPU-slow step into the coarsest level is judged like any other:
     after level 0 is marked (12 ms), a frame at level 7 (where 6b's drag
     budget passed over the levels between) waiting 12 ms stops the zoom, so
     that two frames at level 1 with 10 ms (a cut of 17 %) mark nothing;
     without that frame they mark level 1 */
  rhi::GpuFallback into(rhi::kLevels - 1), around(rhi::kLevels - 1);
  for (rhi::GpuFallback *g : {&into, &around}) {
    g->start();
    g->frame(0, 12.0, 16.0);
    g->frame(0, 12.0, 16.0);
  }
  into.frame(rhi::kLevels - 1, 12.0, 16.0);
  for (rhi::GpuFallback *g : {&into, &around}) {
    g->frame(1, 10.0, 16.0);
    g->frame(1, 10.0, 16.0);
  }
  check(into.stuck() && into.marks() == 1u && !around.stuck() && around.marks() == 3u,
        "7c: a GPU-slow step into level 7 without the gain stops the zoom (stuck " + std::to_string(into.stuck()) +
            ", marks " + std::to_string(into.marks()) + "), so two later frames at level 1 with a cut of 17 % mark "
            "nothing; without that frame they mark level 1 (marks " + std::to_string(around.marks()) + ")");

  using rhi::WaitApi;
  bool fake = true;
  for (WaitApi api : {WaitApi::Null, WaitApi::OpenGL, WaitApi::Direct3D11, WaitApi::Direct3D12, WaitApi::Metal,
                      WaitApi::Vulkan})
    fake = fake && rhi::gpu_wait_counts(api, true);
  check(rhi::gpu_wait_counts(WaitApi::Metal, false) && rhi::gpu_wait_counts(WaitApi::Vulkan, false) &&
            rhi::gpu_wait_counts(WaitApi::Direct3D12, false) && !rhi::gpu_wait_counts(WaitApi::OpenGL, false) &&
            !rhi::gpu_wait_counts(WaitApi::Direct3D11, false) && !rhi::gpu_wait_counts(WaitApi::Null, false) && fake,
        "7c: the fallback can act on Metal, Vulkan and Direct3D 12, whose offscreen frames wait for the GPU, not on "
        "OpenGL, Direct3D 11 or Null; with a test's fake times on all of them");
}

/* 7d1 (7d-14): how the soundings' position and colour chunks are made, per
   API and features: static with storage use where the backend can splat,
   else dynamic on Metal and static elsewhere */
void model_chunk_use() {
  using rhi::WaitApi;
  const auto is = [](rhi::ChunkUse use, bool storage, bool dynamic) {
    return use.storage == storage && use.dynamic == dynamic;
  };
  const int g = rhi::kSplatGroup;
  bool ok = rhi::kSplatGroup == 128;
  /* the APIs that can splat, with Compute and limits of 1024 (Null: 0, not
     reported) */
  ok = ok && is(rhi::chunk_use(WaitApi::Metal, true, 1024, 1024), true, false) &&
       is(rhi::chunk_use(WaitApi::Vulkan, true, 1024, 1024), true, false) &&
       is(rhi::chunk_use(WaitApi::OpenGL, true, 1024, 1024), true, false) &&
       is(rhi::chunk_use(WaitApi::Null, true, 0, 0), true, false);
  /* Direct3D with and without Compute, and every other API without it */
  ok = ok && is(rhi::chunk_use(WaitApi::Direct3D11, true, 1024, 1024), false, false) &&
       is(rhi::chunk_use(WaitApi::Direct3D12, true, 1024, 1024), false, false) &&
       is(rhi::chunk_use(WaitApi::Direct3D11, false, 1024, 1024), false, false) &&
       is(rhi::chunk_use(WaitApi::Direct3D12, false, 1024, 1024), false, false) &&
       is(rhi::chunk_use(WaitApi::Metal, false, 1024, 1024), false, true) &&
       is(rhi::chunk_use(WaitApi::Vulkan, false, 1024, 1024), false, false) &&
       is(rhi::chunk_use(WaitApi::OpenGL, false, 0, 0), false, false) &&
       is(rhi::chunk_use(WaitApi::Null, false, 0, 0), false, false);
  /* each limit held at the group size, crossed by one, and 0 (not reported) */
  for (WaitApi api : {WaitApi::Metal, WaitApi::Vulkan, WaitApi::OpenGL}) {
    const bool metal = api == WaitApi::Metal;
    ok = ok && is(rhi::chunk_use(api, true, g, 1024), true, false) &&
         is(rhi::chunk_use(api, true, 1024, g), true, false) &&
         is(rhi::chunk_use(api, true, g - 1, 1024), false, metal) &&
         is(rhi::chunk_use(api, true, 1024, g - 1), false, metal) &&
         is(rhi::chunk_use(api, true, 64, 64), false, metal) && is(rhi::chunk_use(api, true, 0, 1024), true, false) &&
         is(rhi::chunk_use(api, true, 1024, 0), true, false);
  }
  check(ok, "7d1: the soundings' chunks get storage use (static) on Metal, Vulkan, OpenGL and Null with Compute and "
            "both thread limits " +
                std::to_string(g) + " or more (or 0, not reported); not on Direct3D 11 or 12, without Compute, or "
                                    "with a limit of " +
                std::to_string(g - 1) + " (then dynamic on Metal, static elsewhere)");
}

/* 7c-2: the map's stride loop passes over the levels the GPU-time fallback
   marked at the frame's zoom's level, while dragged, never those of other
   zooms, and keeps every other rule */
void model_gpu_marks() {
  Grid grid;
  grid.columns = 300;
  grid.rows = 200;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "7c: a 300 x 200 grid opens for the GPU-time fallback's marks");
  rhi::MapFrame frame(instance);
  frame.size_core(2400, 1800);
  /* a drag in which 6b's budget never binds: no cost per node, a large budget */
  rhi::PlanOptions drag = view_options();
  drag.drag = true;
  drag.budget_ms = 1000.0;
  drag.node_ns_colour = drag.node_ns_z = 0.0;
  /* each case on a fresh plan whose first frame drew the zoom's stride 1 */
  const auto planned = [&](const rhi::PlanOptions &options) {
    rhi::TerrainPlan plan;
    plan_frame(frame, plan, view_options());
    rhi::PlanResult result;
    plan_frame(frame, plan, options, &result);
    return result.stats;
  };
  /* the marks of the zoom's level 0, the frame's here */
  rhi::PlanOptions zero = drag;
  zero.gpu_marks[0] = 1u;
  const rhi::PlanStats a = planned(zero);
  rhi::PlanOptions one = drag;
  one.gpu_marks[0] = 2u;
  const rhi::PlanStats b = planned(one);
  rhi::PlanOptions both = drag;
  both.gpu_marks[0] = 1u;
  both.level_budget_ms[1] = 0.0;
  both.node_ns_colour = both.node_ns_z = 1000.0;
  const rhi::PlanStats c = planned(both);
  check(a.stride == 2 && a.gpu_coarser && !a.drag_coarser && !a.memory_coarser && b.stride == 1 && !b.gpu_coarser &&
            c.stride == 4 && c.gpu_coarser && c.drag_coarser,
        "7c: while dragged, level 0 marked draws stride " + std::to_string(a.stride) +
            " (gpu_coarser, no other budget), level 1 marked leaves stride 1; level 0 marked and level 1 over its own "
            "drag budget draw stride " + std::to_string(c.stride) + " with both flags");

  /* level 0 marked with a cache budget only level 2 fits: the memory test
     comes first, so the levels it passes over are not reported as marked */
  {
    const size_t budget = mbvq::cache_budget();
    rhi::TerrainPlan plan;
    plan_frame(frame, plan, view_options());
    const size_t level1 = level_set_bytes(plan, 1), level2 = level_set_bytes(plan, 2);
    mbvq::set_cache_budget(level2 + (level1 - level2) / 2);
    rhi::PlanResult result;
    plan_frame(frame, plan, zero, &result);
    mbvq::set_cache_budget(budget);
    const rhi::PlanStats &m = result.stats;
    check(level2 < level1 && m.stride == 4 && m.memory_coarser && !m.gpu_coarser && !m.drag_coarser,
          "7c: level 0 marked with a cache budget only level 2 fits (" + std::to_string(level2 / 1024) + " of " +
              std::to_string((level2 + (level1 - level2) / 2) / 1024) + " kB): stride " + std::to_string(m.stride) +
              " for the memory alone (memory_coarser, not gpu_coarser)");
  }

  rhi::PlanOptions still = view_options();
  still.gpu_marks[0] = 1u;
  const rhi::PlanStats d = planned(still);
  rhi::PlanOptions forced = zero;
  forced.force_stride1 = true;
  const rhi::PlanStats f = planned(forced);
  rhi::PlanOptions all = drag;
  all.gpu_marks[0] = 0xffu;
  const rhi::PlanStats g = planned(all);
  /* marks of other zooms' levels apply only at those zooms */
  rhi::PlanOptions others = drag;
  for (int zoom = 1; zoom < rhi::kLevels; zoom++)
    others.gpu_marks[zoom] = 0xffu;
  const rhi::PlanStats o = planned(others);
  check(d.stride == 1 && !d.gpu_coarser && f.stride == 1 && !f.gpu_coarser && g.stride == 128 && g.gpu_coarser &&
            o.stride == 1 && !o.gpu_coarser,
        "7c: marks without a press, or with stride 1 forced, change nothing; with every level of the frame's zoom "
        "marked the loop falls back to stride 128; every level marked at the other zooms changes nothing here");

  /* zoomed out: the marks of the frame's zoom's level apply, from the
     zoom's level on; those of zoom 0 not */
  frame.size_core(150, 100);
  const rhi::PlanStats z = planned(zero);
  const int zl = level_of(z.stride_zoom);
  rhi::PlanOptions coarser = drag;
  coarser.gpu_marks[zl] = 1u << zl;
  const rhi::PlanStats zc = planned(coarser);
  rhi::PlanOptions finest = drag;
  finest.gpu_marks[zl] = 1u;
  const rhi::PlanStats zf = planned(finest);
  rhi::PlanOptions elsewhere = drag;
  elsewhere.gpu_marks[0] = 1u << zl;
  const rhi::PlanStats ze = planned(elsewhere);
  frame.size_core(2400, 1800);
  check(z.stride_zoom > 1 && z.stride == z.stride_zoom && !z.gpu_coarser && zc.stride == 2 * z.stride_zoom &&
            zc.gpu_coarser && zf.stride == z.stride_zoom && !zf.gpu_coarser && ze.stride == z.stride_zoom &&
            !ze.gpu_coarser,
        "7c: zoomed out to stride " + std::to_string(z.stride_zoom) + " (zoom level " + std::to_string(zl) +
            "): a mark of that zoom at its own level draws stride " + std::to_string(zc.stride) +
            "; a mark of that zoom at level 0, finer than the zoom, and a mark at the zoom's level kept for zoom 0, "
            "change nothing");
  close_instance(instance);
}

/* uploads above the budget outside a drag: one stride per frame, the
   coarser stride first, the planned one complete over the frames (F5) */
void model_split() {
  Grid grid;
  grid.columns = 300;
  grid.rows = 200;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "6b: a 300 x 200 grid opens for split uploads");
  rhi::MapFrame frame(instance);
  frame.size_core(2400, 1800);
  const size_t cap = 600 * 1024;

  /* the bytes of each tile at stride 1, and their estimate before the build */
  std::vector<size_t> actual, estimate;
  {
    rhi::TerrainPlan reference;
    const Planned all = plan_frame(frame, reference, view_options(SIZE_MAX));
    for (const rhi::TileUpload &upload : all.uploads) {
      actual.push_back(12 * upload.xyz.size() / 3 + upload.rgba.size() + 2 * upload.index.size());
      const rhi::TerrainPlan::Level &l = reference.tiles()[upload.tile].levels[0];
      estimate.push_back(16 * static_cast<size_t>(l.ni) * l.nj + 12 * static_cast<size_t>(l.ni - 1) * (l.nj - 1));
    }
  }

  rhi::TerrainPlan plan;
  rhi::PlanResult first;
  plan_frame(frame, plan, view_options(cap), &first);
  /* the coarser stride drawn first: the finest whose estimated uploads fit */
  int coarse = -1;
  {
    for (int l = 1; l < rhi::kLevels && coarse < 0; l++) {
      size_t bytes = 0;
      for (const rhi::TerrainPlan::Tile &tile : plan.tiles()) {
        const rhi::TerrainPlan::Level &level = tile.levels[l];
        bytes += 16 * static_cast<size_t>(level.ni) * level.nj + 12 * static_cast<size_t>(level.ni - 1) * (level.nj - 1);
      }
      if (bytes <= cap)
        coarse = l;
    }
  }
  size_t fallback = 0;
  for (const rhi::TileUpload &upload : first.uploads)
    if (upload.level == coarse)
      fallback += 12 * upload.xyz.size() / 3 + upload.rgba.size() + 2 * upload.index.size();

  /* the frames the rule takes: the coarser level first, then the tiles of
     stride 1 in order while they fit (the first of a frame always), until
     the estimate of those left fits, and then that frame uploads them all,
     or until a frame's tiles complete stride 1 */
  int expected = 0;
  {
    size_t next = 0;
    for (int f = 1; f < 40; f++) {
      expected = f;
      size_t left = 0;
      for (size_t t = next; t < estimate.size(); t++)
        left += estimate[t];
      if (left <= cap)
        break;
      size_t used = f == 1 ? fallback : 0;
      while (next < actual.size() && (used == 0 || used + actual[next] <= cap))
        used += actual[next++];
      if (next == actual.size())
        break;
    }
  }
  int frames = 1;
  bool uniform = one_fresh_stride(plan, first) && first.stats.split && first.stats.stride_target == 1 &&
                 first.stats.stride == rhi::level_stride(coarse);
  bool coarse_until_complete = true;
  rhi::PlanResult result = first;
  while (result.more && frames < 40) {
    plan_frame(frame, plan, view_options(cap), &result);
    frames++;
    uniform = uniform && one_fresh_stride(plan, result);
    coarse_until_complete = coarse_until_complete && (result.more ? result.stats.stride == rhi::level_stride(coarse)
                                                                  : result.stats.stride == 1);
  }
  check(coarse > 0 && uniform && coarse_until_complete && frames == expected && result.stats.stride == 1 &&
            result.drawn.size() == 6 && result.stats.tiles_waiting == 0,
        "a split upload (600 kB a frame, " + std::to_string(estimate.size()) + " tiles at stride 1): stride " +
            std::to_string(rhi::level_stride(coarse)) + " first, then stride 1 complete after " + std::to_string(frames) +
            " frames (the rule gives " + std::to_string(expected) + "), one fresh stride in every frame");

  /* A colour mark while a split upload runs: every later frame draws every
     planned tile at one stride, the coarse one with its colours uploaded
     again first, until stride 1 is complete with the new colours (CC-04,
     risk 4) */
  {
    rhi::TerrainPlan marked;
    rhi::PlanResult r;
    plan_frame(frame, marked, view_options(cap), &r);
    const bool split_first = r.more && r.stats.stride == rhi::level_stride(coarse);
    mbvq_c_view(instance, MBVQ_VIEW_COLORTABLE_BRIGHT, true);
    bool whole = true, recoloured = false;
    int after = 0;
    for (int f = 0; f < 40; f++) {
      plan_frame(frame, marked, view_options(cap), &r);
      after++;
      whole = whole && r.drawn.size() == 6 && r.stats.stride > 0 && one_fresh_stride(marked, r);
      if (f == 0) {
        int coarse_colours = 0;
        for (const rhi::TileUpload &upload : r.uploads)
          coarse_colours += upload.level == coarse && upload.colours ? 1 : 0;
        recoloured = r.stats.stride == rhi::level_stride(coarse) && coarse_colours == 6;
      }
      if (!r.more)
        break;
    }
    mbvq_c_view(instance, MBVQ_VIEW_COLORTABLE_HAXBY, true);
    check(split_first && whole && recoloured && !r.more && r.stats.stride == 1,
          "a colour mark during a split upload: every frame after it draws all 6 tiles at one fresh stride, stride " +
              std::to_string(rhi::level_stride(coarse)) + " recoloured first, stride 1 complete after " +
              std::to_string(after) + " frames");
  }
  close_instance(instance);
}

/* A split upload whose planned stride fits in the frame after all: the
   estimate of index lists not made yet (12 bytes a cell) is above the cap,
   the lists themselves are not, on a grid with many cells without data. The
   frame draws the planned stride, complete, and asks for no other. */
void model_split_complete() {
  Grid grid;
  grid.columns = 300;
  grid.rows = 200;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "6b: a 300 x 200 grid opens for a split that completes at once");
  {
    /* no data in a checkerboard of 8 x 8 node blocks */
    struct mbview_struct *data = data_of(instance);
    std::vector<float> values(data->primary_data, data->primary_data + data->primary_nxy);
    for (int i = 0; i < grid.columns; i++)
      for (int j = 0; j < grid.rows; j++)
        if ((i / 8 + j / 8) % 2 == 0)
          values[static_cast<size_t>(i) * grid.rows + j] = data->primary_nodatavalue;
    int error = MB_ERROR_NO_ERROR;
    mbview_updateprimarygrid(0, instance, grid.columns, grid.rows, values.data(), &error);
  }
  rhi::MapFrame frame(instance);
  frame.size_core(2400, 1800);

  /* the bytes of stride 1 and of stride 2, made without a cap */
  size_t actual = 0, actual2 = 0;
  {
    rhi::TerrainPlan reference;
    rhi::PlanResult r;
    plan_frame(frame, reference, view_options(SIZE_MAX), &r);
    actual = upload_bytes(r);
    camera_2d(instance, 0.05, 0.0, 0.0);
    rhi::TerrainPlan coarser;
    plan_frame(frame, coarser, view_options(SIZE_MAX), &r);
    actual2 = upload_bytes(r);
    camera_2d(instance, 1.0, 0.0, 0.0);
  }

  /* zoomed out at stride 2 first, then in, with a cap of stride 1's bytes */
  {
    rhi::TerrainPlan plan;
    camera_2d(instance, 0.05, 0.0, 0.0);
    rhi::PlanResult out;
    plan_frame(frame, plan, view_options(SIZE_MAX), &out);
    camera_2d(instance, 1.0, 0.0, 0.0);
    const size_t estimate = estimated_upload(plan, 0);
    rhi::PlanResult in;
    plan_frame(frame, plan, view_options(actual), &in);
    check(out.stats.stride == 2 && estimate > actual && in.stats.stride == 1 && in.stats.stride_target == 1 &&
              !in.stats.split && !in.more && in.drawn.size() == 6 && one_fresh_stride(plan, in),
          "zoomed in from stride 2 with a cap of stride 1's own bytes (" + std::to_string(actual / 1024) +
              " kB, estimated " + std::to_string(estimate / 1024) + " kB): stride " + std::to_string(in.stats.stride) +
              " drawn in that frame, complete, no other frame asked for (" + std::to_string(in.more) + ")");
  }

  /* a first load: the fallback stride and the planned one in the same frame */
  {
    const size_t cap = actual + actual2 + 1024;
    rhi::TerrainPlan plan;
    rhi::PlanResult first;
    plan_frame(frame, plan, view_options(cap), &first);
    /* the split's fallback was made too: the estimate did not fit */
    bool fallback = false;
    for (const rhi::TileUpload &upload : first.uploads)
      fallback = fallback || upload.level == 1;
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions options;
    options.upload_budget = cap;
    renderer.render_map(instance, 2400, 1800, options, nullptr, 20);
    const mbvq::RenderStats stats = renderer.stats();
    check(fallback && first.stats.stride == 1 && !first.more && first.drawn.size() == 6 &&
              one_fresh_stride(plan, first) && stats.stride == 1 && stats.stride_zoom == 1 &&
              renderer.last_frames() == 1,
          "a first load with a cap above both strides' bytes (" + std::to_string(cap / 1024) +
              " kB), the estimate above it: stride 2 made, stride 1 drawn in that frame, also by the renderer (" +
              std::to_string(renderer.last_frames()) + " frame, stride " + std::to_string(stats.stride) + ")");
  }
  close_instance(instance);
}

/* the memory budget of the process: eviction across views, the stride it
   coarsens, the levels it never takes */
void model_memory() {
  Grid grid;
  grid.columns = 300;
  grid.rows = 200;
  size_t a = MBV_NO_WINDOW, b = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &a) && open_grid(grid, nullptr, &b), "6b: two 300 x 200 grids open for the memory budget");
  rhi::MapFrame frame_a(a), frame_b(b);
  frame_a.size_core(2400, 1800);
  frame_b.size_core(2400, 1800);
  const size_t budget = mbvq::cache_budget();
  {
    rhi::TerrainPlan plan_a, plan_b;
    int asked = 0;
    plan_a.set_evicted_callback([&asked] { asked++; });

    /* view A draws stride 1, then zoomed out stride 4: both levels held */
    rhi::PlanResult a1;
    plan_frame(frame_a, plan_a, view_options(), &a1);
    const size_t stride1 = plan_a.held_bytes();
    camera_2d(a, 0.02, 0.0, 0.0);
    rhi::PlanResult a4;
    plan_frame(frame_a, plan_a, view_options(), &a4);
    const size_t both = plan_a.held_bytes();
    const int coarse = level_of(a4.stats.stride);
    check(a1.stats.stride == 1 && a4.stats.stride > 1 && both > stride1 && mbvq::cache_bytes() == both,
          "view A holds stride 1 and stride " + std::to_string(a4.stats.stride) + ": " + std::to_string(both / 1024) +
              " kB in the process");

    /* a budget for A's drawn set and one more at stride 1: view B's frame at
       stride 1 evicts A's level that A did not draw last (least recently
       drawn), never the one A drew; A's bytes count until A's next frame */
    mbvq::set_cache_budget(both - stride1 + stride1 + stride1 / 2);
    rhi::PlanResult b1;
    plan_frame(frame_b, plan_b, view_options(), &b1);
    bool a_drawn_kept = true;
    int a_fine_gone = 0;
    for (int t = 0; t < static_cast<int>(plan_a.tiles().size()); t++) {
      a_drawn_kept = a_drawn_kept && (plan_a.tiles()[t].levels[coarse].resident || plan_a.tiles()[t].levels[coarse].empty);
      a_fine_gone += plan_a.tiles()[t].levels[0].resident ? 0 : 1;
    }
    const size_t a_counted = plan_a.held_bytes();
    rhi::PlanResult a_next;
    plan_frame(frame_a, plan_a, view_options(), &a_next);
    check(b1.stats.stride == 1 && b1.stats.evicted_other > 0 && b1.stats.evicted_own == 0 && a_drawn_kept &&
              a_fine_gone == b1.stats.evicted_other && asked == 1 && a_counted == both &&
              a_next.drops.size() == static_cast<size_t>(b1.stats.evicted_other) && a_next.stats.evicted_own == 0 &&
              plan_a.held_bytes() < both && plan_a.evictions() == b1.stats.evicted_other,
          "view B's frame evicts A's stride-1 levels (" + std::to_string(b1.stats.evicted_other) + ", " +
              std::to_string(a_fine_gone) + " gone), not those A drew; A is asked for a frame (" + std::to_string(asked) +
              "), and its bytes count until that frame drops them (" + std::to_string(a_counted) + " of " +
              std::to_string(both) + " bytes; " + std::to_string(a_next.drops.size()) + " dropped, " +
              std::to_string(a_next.stats.evicted_own) + " of its own evicted)");

    /* a smaller budget: B's drawn set at stride 1 does not fit beside the
       levels A drew last, so B draws the finest stride that fits, and the
       least recently drawn levels go; once A's next frame dropped what B
       evicted of it, the process is within the budget */
    size_t a_drawn = 0;
    for (const rhi::TerrainPlan::Tile &tile : plan_a.tiles())
      a_drawn += tile.levels[coarse].resident ? tile.levels[coarse].bytes : 0;
    mbvq::set_cache_budget(a_drawn + stride1 / 2);
    /* what the stride below B's would need, made as the plan counts it */
    rhi::PlanResult b2;
    plan_frame(frame_b, plan_b, view_options(), &b2);
    const int below = level_of(b2.stats.stride) - 1;
    const size_t finer = below >= 0 ? level_set_bytes(plan_b, below) : 0;
    plan_frame(frame_a, plan_a, view_options());
    check(b2.stats.stride > 1 && b2.stats.memory_coarser && !b2.stats.over_memory && below >= 0 &&
              a_drawn + finer > mbvq::cache_budget() && b2.stats.evicted_own + b2.stats.evicted_other > 0 &&
              one_fresh_stride(plan_b, b2) && mbvq::cache_bytes() <= mbvq::cache_budget(),
          "a smaller budget: view B draws at stride " + std::to_string(b2.stats.stride) +
              ", the finest whose levels fit beside those A drew (the stride below needs " +
              std::to_string(finer / 1024) + " kB beside A's " + std::to_string(a_drawn / 1024) + " kB in " +
              std::to_string(mbvq::cache_budget() / 1024) + " kB), evicting " + std::to_string(b2.stats.evicted_own) +
              " of its own and " + std::to_string(b2.stats.evicted_other) + " of A's levels; then " +
              std::to_string(mbvq::cache_bytes() / 1024) + " of " + std::to_string(mbvq::cache_budget() / 1024) +
              " kB held");

    /* A hidden keeps nothing: B at stride 1 may evict what A drew */
    plan_a.set_visible(false);
    mbvq::set_cache_budget(stride1 + 1024);
    rhi::PlanResult b3;
    plan_frame(frame_b, plan_b, view_options(), &b3);
    int a_drawn_gone = 0;
    for (const rhi::TerrainPlan::Tile &tile : plan_a.tiles())
      a_drawn_gone += !tile.levels[coarse].empty && !tile.levels[coarse].resident ? 1 : 0;
    check(b3.stats.stride == 1 && b3.stats.evicted_other > 0 && a_drawn_gone > 0 && plan_a.held_bytes() > 0,
          "view A hidden: B's frame at stride 1 evicts levels A drew last (" + std::to_string(a_drawn_gone) +
              "); their bytes count until A draws again");
    plan_a.set_visible(true);

    /* no stride fits: stride 128, reported; the levels visible A drew in its
       latest frame stay, though B's frame is above the budget */
    mbvq::set_cache_budget(budget);
    plan_frame(frame_a, plan_a, view_options());
    std::vector<std::pair<int, int>> a_latest;
    for (int t = 0; t < static_cast<int>(plan_a.tiles().size()); t++)
      for (int l = 0; l < rhi::kLevels; l++)
        if (plan_a.tiles()[t].levels[l].drawn && plan_a.tiles()[t].levels[l].resident)
          a_latest.push_back({t, l});
    mbvq::set_cache_budget(1024);
    rhi::PlanResult tiny;
    plan_frame(frame_b, plan_b, view_options(), &tiny);
    bool a_kept = !a_latest.empty();
    for (const auto &key : a_latest)
      a_kept = a_kept && plan_a.tiles()[key.first].levels[key.second].resident;
    check(tiny.stats.stride == 128 && tiny.stats.over_memory && one_fresh_stride(plan_b, tiny) && a_kept,
          "a budget nothing fits in: stride 128, reported above the budget; the " + std::to_string(a_latest.size()) +
              " levels A drew in its latest frame are kept");
    mbvq::set_cache_budget(budget);
  }
  check(mbvq::cache_bytes() == 0, "the plans gone, the process holds nothing");
  camera_2d(a, 1.0, 0.0, 0.0);

  /* Two views uploading at once (a first load, or both given a new QRhi) on
     a budget that holds one drawn set at stride 1 but not both, with uploads
     split over frames: both settle within a bounded number of frames, neither
     evicts a level the other drew in its latest frame or is uploading, the
     split keeps its fine level, and once settled nothing is uploaded again. */
  {
    size_t one = 0;
    {
      rhi::TerrainPlan probe;
      plan_frame(frame_a, probe, view_options());
      one = probe.held_bytes();
    }
    mbvq::set_cache_budget(one + one / 2);
    rhi::TerrainPlan plan_a, plan_b;
    rhi::TerrainPlan *plans[2] = {&plan_a, &plan_b};
    rhi::MapFrame *frames[2] = {&frame_a, &frame_b};
    const size_t cap = 600 * 1024;
    bool kept = true, fine_kept = true;
    int settled_at = -1;
    int quiet = 0;
    /* the uploads of each level of each view, over the whole run */
    std::map<std::array<int, 3>, int> uploaded;
    for (int f = 0; f < 60 && settled_at < 0; f++) {
      const int me = f % 2;
      const rhi::TerrainPlan &other = *plans[1 - me];
      /* what the other view keeps, and this view's split upload so far */
      std::vector<std::pair<int, int>> pinned, uploading;
      for (int t = 0; t < static_cast<int>(other.tiles().size()); t++)
        for (int l = 0; l < rhi::kLevels; l++) {
          const rhi::TerrainPlan::Level &level = other.tiles()[t].levels[l];
          if ((level.drawn || level.uploading) && level.resident)
            pinned.push_back({t, l});
        }
      for (int t = 0; t < static_cast<int>(plans[me]->tiles().size()); t++)
        for (int l = 0; l < rhi::kLevels; l++) {
          const rhi::TerrainPlan::Level &level = plans[me]->tiles()[t].levels[l];
          if (level.uploading && level.resident)
            uploading.push_back({t, l});
        }
      rhi::PlanResult result;
      plan_frame(*frames[me], *plans[me], view_options(cap), &result);
      for (const rhi::TileUpload &upload : result.uploads)
        uploaded[{me, upload.tile, upload.level}]++;
      for (const auto &key : pinned)
        kept = kept && other.tiles()[key.first].levels[key.second].resident;
      for (const auto &key : uploading)
        fine_kept = fine_kept && (plans[me]->tiles()[key.first].levels[key.second].resident ||
                                  plans[me]->tiles()[key.first].levels[key.second].empty);
      quiet = result.uploads.empty() && !result.more ? quiet + 1 : 0;
      if (quiet >= 2)
        settled_at = f + 1;
    }
    /* settled: frames of both without a mark change upload nothing (the
       first pair may still drop levels the other evicted, none drawn) */
    rhi::PlanResult last_a, last_b;
    for (int f = 0; f < 2; f++) {
      plan_frame(frame_a, plan_a, view_options(cap), &last_a);
      plan_frame(frame_b, plan_b, view_options(cap), &last_b);
      for (const rhi::TileUpload &upload : last_a.uploads)
        uploaded[{0, upload.tile, upload.level}]++;
      for (const rhi::TileUpload &upload : last_b.uploads)
        uploaded[{1, upload.tile, upload.level}]++;
    }
    int again = 0;
    for (const auto &entry : uploaded)
      again += entry.second > 1 ? 1 : 0;
    check(settled_at > 0 && kept && fine_kept && last_a.uploads.empty() && last_b.uploads.empty() &&
              last_a.drops.empty() && last_b.drops.empty() && again == 0 && one_fresh_stride(plan_a, last_a) &&
              one_fresh_stride(plan_b, last_b),
          "two views uploading at once on a budget for one drawn set: settled after " + std::to_string(settled_at) +
              " frames at strides " + std::to_string(last_a.stats.stride) + " and " +
              std::to_string(last_b.stats.stride) + ", no level the other drew or uploads evicted (" +
              std::to_string(kept) + "), the split kept its fine level (" + std::to_string(fine_kept) +
              "); over the whole run no level uploaded twice (" + std::to_string(uploaded.size()) + " levels, " +
              std::to_string(again) + " again), and once settled nothing uploaded or dropped");
    check(mbvq::cache_bytes() <= mbvq::cache_budget() &&
              (last_a.stats.memory_coarser || last_b.stats.memory_coarser),
          "settled within the budget (" + std::to_string(mbvq::cache_bytes() / 1024) + " of " +
              std::to_string(mbvq::cache_budget() / 1024) + " kB), one view drawn coarser for it (strides " +
              std::to_string(last_a.stats.stride) + " and " + std::to_string(last_b.stats.stride) + ")");
    mbvq::set_cache_budget(budget);
  }

  /* Least recently drawn first: A draws strides 1, 2 and 4, one per frame;
     B's frame at stride 1 needs about A's stride-1 levels: those go, A's
     stride-2 levels stay, and its stride 4, drawn last, is kept. */
  {
    rhi::TerrainPlan plan_a, plan_b;
    const double zooms[3] = {1.0, 0.05, 0.02};
    int strides[3] = {};
    for (int s = 0; s < 3; s++) {
      camera_2d(a, zooms[s], 0.0, 0.0);
      rhi::PlanResult r;
      plan_frame(frame_a, plan_a, view_options(), &r);
      strides[s] = r.stats.stride;
    }
    camera_2d(a, 1.0, 0.0, 0.0);
    const size_t a_fine = level_set_bytes(plan_a, 0);
    const size_t b_fine = fresh_set_bytes(plan_a, 0); /* the same grid, nothing made yet */
    mbvq::set_cache_budget(plan_a.held_bytes() + b_fine - a_fine + 1024);
    rhi::PlanResult r;
    plan_frame(frame_b, plan_b, view_options(), &r);
    int fine_left = 0, middle_gone = 0, last_gone = 0;
    for (const rhi::TerrainPlan::Tile &tile : plan_a.tiles()) {
      fine_left += tile.levels[0].resident ? 1 : 0;
      middle_gone += !tile.levels[level_of(strides[1])].resident && !tile.levels[level_of(strides[1])].empty;
      last_gone += !tile.levels[level_of(strides[2])].resident && !tile.levels[level_of(strides[2])].empty;
    }
    check(strides[0] == 1 && strides[1] == 2 && strides[2] == 4 && r.stats.stride == 1 && fine_left == 0 &&
              middle_gone == 0 && last_gone == 0 && r.stats.evicted_other == 6,
          "least recently drawn first: A drew strides 1, 2 and 4; B's frame evicts A's stride-1 levels (" +
              std::to_string(r.stats.evicted_other) + ", " + std::to_string(fine_left) + " left), not stride 2 (" +
              std::to_string(middle_gone) + " gone) nor stride 4, drawn last (" + std::to_string(last_gone) + ")");
    mbvq::set_cache_budget(budget);
  }

  /* A split upload keeps its fine level from its own evictions: with a
     budget that holds stride 1 but not beside the coarser stride drawn, its
     next frame still plans stride 1 and makes no room from the tiles of it
     uploaded so far (CK-04) */
  {
    rhi::TerrainPlan plan_a;
    const size_t cap = 600 * 1024;
    /* the frames of the split until some tiles of stride 1 are uploaded */
    rhi::PlanResult split;
    std::vector<int> fine;
    for (int f = 0; f < 10 && fine.empty(); f++) {
      plan_frame(frame_a, plan_a, view_options(cap), &split);
      for (int t = 0; t < static_cast<int>(plan_a.tiles().size()); t++)
        if (plan_a.tiles()[t].levels[0].resident)
          fine.push_back(t);
    }
    mbvq::set_cache_budget(level_set_bytes(plan_a, 0));
    rhi::PlanResult next;
    plan_frame(frame_a, plan_a, view_options(cap), &next);
    bool fine_kept = !fine.empty();
    for (int t : fine)
      fine_kept = fine_kept && plan_a.tiles()[t].levels[0].resident;
    check(split.more && split.stats.stride > 1 && next.stats.stride_target == 1 && fine_kept &&
              next.stats.evicted_own == 0,
          "a split upload over the budget: its next frame plans stride 1 still and keeps the " +
              std::to_string(fine.size()) + " tiles of it uploaded so far (" +
              std::to_string(next.stats.evicted_own) + " of its own evicted)");
    mbvq::set_cache_budget(budget);
  }

  /* A split upload's levels not uploaded yet count for the other views'
     strides: a budget that holds A's drawn and planned levels and B's
     stride 1 but one byte coarsens B (CK-04) */
  {
    rhi::TerrainPlan plan_a, plan_b;
    const size_t cap = 600 * 1024;
    rhi::PlanResult first;
    plan_frame(frame_a, plan_a, view_options(cap), &first);
    int pending = 0;
    for (const rhi::TerrainPlan::Tile &tile : plan_a.tiles())
      pending += tile.levels[0].resident ? 0 : 1;
    /* what A keeps: the stride drawn, and stride 1 as uploaded so far and as
       estimated for the rest; B's stride 1 before B made anything */
    const size_t a_pinned = level_set_bytes(plan_a, level_of(first.stats.stride)) + level_set_bytes(plan_a, 0);
    mbvq::set_cache_budget(a_pinned + fresh_set_bytes(plan_a, 0) - 1);
    rhi::PlanResult b1;
    plan_frame(frame_b, plan_b, view_options(cap), &b1);
    check(first.more && pending > 0 && b1.stats.memory_coarser && b1.stats.stride > 1,
          "a split upload's levels still to come (" + std::to_string(pending) +
              " tiles) count for another view's stride: B drawn at stride " + std::to_string(b1.stats.stride) +
              " beside them");
    mbvq::set_cache_budget(budget);
  }

  /* A hidden view keeps the bytes evicted from it until it is shown: a
     visible view's frames do not evict again for them while nothing
     changes (R4); the process is over the budget meanwhile, and within it
     once the hidden view drew again. */
  {
    rhi::TerrainPlan plan_a, plan_b;
    for (rhi::TerrainPlan *plan : {&plan_a, &plan_b}) {
      rhi::MapFrame &f = plan == &plan_a ? frame_a : frame_b;
      const size_t instance = plan == &plan_a ? a : b;
      plan_frame(f, *plan, view_options());
      camera_2d(instance, 0.02, 0.0, 0.0);
      plan_frame(f, *plan, view_options());
      camera_2d(instance, 1.0, 0.0, 0.0);
      plan_frame(f, *plan, view_options());
    }
    const size_t a_fine = level_set_bytes(plan_a, 0);
    plan_a.set_visible(false);
    mbvq::set_cache_budget(plan_a.held_bytes() + plan_b.held_bytes() - a_fine / 4);
    int first = 0, later = 0;
    for (int f = 0; f < 12; f++) {
      rhi::PlanResult r;
      plan_frame(frame_b, plan_b, view_options(), &r);
      (f == 0 ? first : later) += r.stats.evicted_own + r.stats.evicted_other;
    }
    const bool over = mbvq::cache_bytes() > mbvq::cache_budget();
    const int a_evicted = plan_a.evictions();
    plan_a.set_visible(true);
    rhi::PlanResult shown;
    plan_frame(frame_a, plan_a, view_options(), &shown);
    check(first > 0 && later == 0 && over && a_evicted > 0 &&
              shown.drops.size() == static_cast<size_t>(a_evicted + shown.stats.evicted_own),
          "view A hidden, the budget a quarter of A's stride 1 below what both hold: B's first frame evicts " +
              std::to_string(first) + " levels, its next 11 frames none (" + std::to_string(later) +
              "); the process over the budget meanwhile (" + std::to_string(over) + "), A's evicted levels dropped "
              "when it draws again (" + std::to_string(shown.drops.size()) + " of " + std::to_string(a_evicted) + ")");
    mbvq::set_cache_budget(budget);
  }

  /* A view drawn coarser for the memory budget is asked for a frame when the
     levels another view kept go (hidden, or the view gone), and draws its
     zoom's stride then (R5, E28) */
  for (int how = 0; how < 2; how++) {
    size_t one = 0;
    {
      rhi::TerrainPlan probe;
      plan_frame(frame_a, probe, view_options());
      one = probe.held_bytes();
    }
    mbvq::set_cache_budget(one + one / 2);
    rhi::TerrainPlan plan_a;
    int asked = 0;
    plan_a.set_evicted_callback([&asked] { asked++; });
    std::unique_ptr<rhi::TerrainPlan> plan_b(new rhi::TerrainPlan);
    plan_frame(frame_b, *plan_b, view_options());
    rhi::PlanResult coarser;
    plan_frame(frame_a, plan_a, view_options(), &coarser);
    const int before = asked;
    if (how == 0)
      plan_b->set_visible(false);
    else
      plan_b.reset();
    rhi::PlanResult next;
    plan_frame(frame_a, plan_a, view_options(), &next);
    check(coarser.stats.memory_coarser && coarser.stats.stride > 1 && asked == before + 1 && next.stats.stride == 1 &&
              !next.stats.memory_coarser,
          std::string("a view drawn at stride ") + std::to_string(coarser.stats.stride) +
              " for the memory budget is asked for a frame when the other view is " + (how == 0 ? "hidden" : "gone") +
              " (" + std::to_string(asked - before) + "), and draws stride " + std::to_string(next.stats.stride) +
              " then");
    mbvq::set_cache_budget(budget);
  }

  /* A frame made coarser by the drag budget lets go of no memory: B was drawn
     coarser for the memory budget beside A's stride 1; while A's shading
     drag draws stride 2 for the time budget, B's frames (asked for or not)
     leave A's stride-1 levels alone, and A's release frame is back at the
     stride of the zoom, B at its stride as before (R2-REV-1). */
  {
    rhi::TerrainPlan plan_a, plan_b;
    int asked_b = 0;
    plan_b.set_evicted_callback([&asked_b] { asked_b++; });
    rhi::PlanResult a1;
    plan_frame(frame_a, plan_a, view_options(), &a1);
    const size_t stride1 = plan_a.held_bytes();
    const size_t stride2 = fresh_set_bytes(plan_a, 1);
    mbvq::set_cache_budget(stride1 + stride2 + stride2 / 2);
    rhi::PlanResult b2;
    plan_frame(frame_b, plan_b, view_options(), &b2);
    /* colours at 1000 ns a node: stride 2 fits the budget with 3 ms to
       spare, stride 1 (four times the nodes) does not */
    rhi::PlanOptions drag = view_options();
    drag.drag = true;
    drag.node_ns_colour = 1000.0;
    drag.node_ns_z = 1.0;
    drag.budget_ms = 1.0e-6 * 1000.0 * static_cast<double>(lattice_nodes(plan_a, 1)) + 3.0;
    bool coarser = true;
    int b_frames = 0, b_took = 0, b_stride = 0;
    for (int m = 1; m <= 4; m++) {
      core_drag(a, MBV_MOUSE_SHADE, 3, 1, 10 * m);
      rhi::PlanResult moved;
      plan_frame(frame_a, plan_a, drag, &moved);
      coarser = coarser && moved.stats.stride == 2 && moved.stats.drag_coarser;
      /* B draws a frame, as a view does when asked or for its own reasons */
      asked_b = 0;
      rhi::PlanResult rb;
      plan_frame(frame_b, plan_b, view_options(), &rb);
      b_frames++;
      b_took += rb.stats.evicted_other;
      b_stride = std::max(b_stride, rb.stats.stride);
    }
    core_release(a, 3);
    rhi::PlanResult released;
    plan_frame(frame_a, plan_a, view_options(), &released);
    const int asked_after = asked_b;
    rhi::PlanResult b_after;
    plan_frame(frame_b, plan_b, view_options(), &b_after);
    /* A's next frame drops what B's evicted */
    rhi::PlanResult a_after;
    plan_frame(frame_a, plan_a, view_options(), &a_after);
    check(a1.stats.stride == 1 && b2.stats.stride == 2 && b2.stats.memory_coarser && coarser && b_took == 0 &&
              b_stride > 1 && released.stats.stride == 1 && released.stats.stride_zoom == 1 &&
              !released.stats.memory_coarser && asked_after > 0 && b_after.stats.stride == 2 &&
              a_after.stats.stride == 1 && mbvq::cache_bytes() <= mbvq::cache_budget(),
          "B drawn at stride 2 for the memory budget beside A's stride 1; A's shading drag at stride 2 for the time "
          "budget: B's " + std::to_string(b_frames) + " frames meanwhile take none of A's levels (" +
              std::to_string(b_took) + ") and stay coarser (stride " + std::to_string(b_stride) +
              "); A's release frame draws stride " + std::to_string(released.stats.stride) + " (zoom " +
              std::to_string(released.stats.stride_zoom) + "), then B is asked and draws stride " +
              std::to_string(b_after.stats.stride));
    mbvq::set_cache_budget(budget);
  }
  close_instance(b);
  close_instance(a);
}

/* The cost averages of the drag budget: a frame's estimate is the nodes of
   the levels it builds times the averages of its cost class, which learn
   from that frame; the classes keep the grid modes, the spheroid and overlay
   shading of a secondary grid apart, the last two starting high. */
void model_costs() {
  using Plan = rhi::TerrainPlan;
  Grid grid;
  grid.columns = 300;
  grid.rows = 200;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "6b: a 300 x 200 grid opens for the cost averages");
  rhi::MapFrame frame(instance);
  frame.size_core(2400, 1800);
  Plan plan;
  plan_frame(frame, plan, view_options());
  const auto cost_now = [instance]() {
    struct mbvq_grid g;
    mbvq_c_grid(instance, &g);
    return Plan::cost_class(g);
  };
  /* a frame after a mark, against the averages before it: its estimate (in
     the colour context: colours stale, or the secondary grid mode at stride
     1) and what the averages learnt; nodes: the nodes prepared */
  struct Learnt {
    bool estimate = false; /* the estimate is the averages of the class times the nodes */
    bool moved = false;    /* the class's average of preparing moved toward the frame's cost */
    bool others = true;    /* no other class's averages moved */
    double before = 0.0;
    double after = 0.0;
    double sample = 0.0;
  };
  const auto frame_learns = [&](int cost) {
    Learnt learnt;
    const Plan::Costs before = plan.costs();
    rhi::PlanResult r;
    plan_frame(frame, plan, view_options(), &r);
    const Plan::Costs after = plan.costs();
    double ns = 0.0, nodes = 0.0;
    for (const rhi::TileUpload &upload : r.uploads) {
      const Plan::Level &l = plan.tiles()[upload.tile].levels[upload.level];
      const double n = static_cast<double>(l.ni) * static_cast<double>(l.nj);
      double per_node = upload.positions || upload.colours ? before.prepare[cost][Plan::kColour] : 0.0;
      per_node += (upload.positions ? before.gather[0] : 0.0) + (upload.colours ? before.gather[1] : 0.0);
      per_node += upload.indices ? before.index[cost] : 0.0;
      ns += n * per_node;
      nodes += upload.positions || upload.colours ? n : 0.0;
    }
    const double expected = r.stats.bounds_ms + 1.0e-6 * ns;
    learnt.estimate = !r.uploads.empty() && std::fabs(r.stats.estimated_ms - expected) <= 1.0e-9 * (1.0 + expected);
    learnt.before = before.prepare[cost][Plan::kColour];
    learnt.after = after.prepare[cost][Plan::kColour];
    learnt.sample = nodes > 0.0 ? 1.0e6 * r.stats.prepare_ms / nodes : 0.0;
    learnt.moved = learnt.after != learnt.before &&
                   std::fabs(learnt.after - learnt.sample) < std::fabs(learnt.before - learnt.sample);
    for (int k = 0; k < Plan::kCostClasses; k++)
      for (int c = 0; c < 3; c++)
        learnt.others = learnt.others && (k == cost || after.prepare[k][c] == before.prepare[k][c]) &&
                        (k == cost || after.index[k] == before.index[k]);
    return learnt;
  };

  /* a colour change in the primary grid mode */
  const int primary = cost_now();
  mbvq_c_view(instance, MBVQ_VIEW_COLORTABLE_BRIGHT, true);
  const Learnt colours = frame_learns(primary);
  check(primary == 0 && colours.estimate && colours.moved && colours.others,
        "the cost averages, primary grid mode (class " + std::to_string(primary) +
            "): a colour change estimated from the class's averages; its average of colouring a node moved from " +
            text("%.1f", colours.before) + " to " + text("%.1f", colours.after) + " ns toward the frame's " +
            text("%.1f", colours.sample) + " ns, no other class's");

  /* the secondary grid mode on the primary's nodes: its own averages */
  check(add_secondary(instance, grid, grid.columns, grid.rows), "an overlay grid on the primary's nodes");
  mbvq_c_view(instance, MBVQ_VIEW_GRID_SECONDARY, true);
  const int secondary = cost_now();
  const Learnt mode = frame_learns(secondary);
  check(secondary == 1 && mode.estimate && mode.moved && mode.others &&
            plan.costs().prepare[primary][Plan::kColour] == colours.after,
        "the secondary grid mode (class " + std::to_string(secondary) +
            "): estimated from its own averages, not the primary grid's; its average moved from " +
            text("%.1f", mode.before) + " to " + text("%.1f", mode.after) + " ns, the primary grid's stayed");

  /* overlay shading of a secondary grid apart in the primary grid mode: a
     class of its own that starts high (PROJ calls for each colour) */
  check(add_secondary(instance, grid, 157, 113), "an overlay grid apart (157 x 113 nodes)");
  mbvq_c_view(instance, MBVQ_VIEW_GRID_PRIMARY, true);
  mbvq_c_view(instance, MBVQ_VIEW_SHADE_OVERLAY, true);
  const int overlay = cost_now();
  const Learnt shaded = frame_learns(overlay);
  check(overlay == primary + 6 && shaded.before >= 100.0 && shaded.estimate && shaded.moved && shaded.others,
        "overlay shading of a grid apart (class " + std::to_string(overlay) + "): its own averages, starting at " +
            text("%.0f", shaded.before) + " ns a coloured node and moving to " + text("%.1f", shaded.after) +
            " ns, apart from plain shading's");

  /* the spheroid: classes of their own, z starting high (every z reprojected) */
  mbvq_c_view(instance, MBVQ_VIEW_SHADE_SLOPE, true);
  mbvq_c_setprojection(instance, 2);
  const int sphere = cost_now();
  const Plan::Costs now = plan.costs();
  check(sphere == primary + 3 && now.prepare[sphere][Plan::kZ] >= 100.0 &&
            now.prepare[sphere][Plan::kZ] > now.prepare[primary][Plan::kZ],
        "on the spheroid (class " + std::to_string(sphere) + "): z only starts at " +
            text("%.0f", now.prepare[sphere][Plan::kZ]) + " ns a node, against " +
            text("%.1f", now.prepare[primary][Plan::kZ]) + " ns off it");
  mbvq_c_setprojection(instance, 1);
  close_instance(instance);
}

/* On the spheroid every node with data lies inside its tile's box, wherever
   it lies between the box's lattice nodes, whichever grid mode the box was
   made in, and after an exaggeration change: an oracle from where the core
   places the nodes. A camera on a narrow strip of data far from the rest of
   its tile draws that tile. */
void model_spheroid() {
  Grid grid;
  grid.holes = false;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "6b: the synthetic grid opens for the spheroid's boxes");
  struct mbview_struct *data = data_of(instance);
  {
    /* tile (0, 0) holds a block and a strip of 5 columns between the lattice
       nodes, nothing else */
    std::vector<float> values(data->primary_data, data->primary_data + data->primary_nxy);
    for (int i = 0; i <= rhi::kTileCells; i++)
      for (int j = 0; j <= rhi::kTileCells; j++) {
        const bool block = i <= 40 && j <= 40;
        const bool strip = i >= 100 && i <= 104 && j >= 20 && j <= 100;
        if (!block && !strip)
          values[static_cast<size_t>(i) * grid.rows + j] = data->primary_nodatavalue;
      }
    int error = MB_ERROR_NO_ERROR;
    mbview_updateprimarygrid(0, instance, grid.columns, grid.rows, values.data(), &error);
    /* an overlay grid on the primary's nodes with data in columns 0 to 39
       only: in the secondary grid mode the tile holds the block alone */
    const float nodatavalue = -88888.0f;
    std::vector<float> secondary(static_cast<size_t>(grid.columns) * grid.rows);
    for (int i = 0; i < grid.columns; i++)
      for (int j = 0; j < grid.rows; j++)
        secondary[static_cast<size_t>(i) * grid.rows + j] = i < 40 ? static_cast<float>(10.0 + j % 11) : nodatavalue;
    char projection_id[MB_PATH_MAXLINE] = "EPSG:4326";
    const double dx = (grid.xmax - grid.xmin) / (grid.columns - 1), dy = (grid.ymax - grid.ymin) / (grid.rows - 1);
    int status = mbview_setsecondarygrid(0, instance, MBV_PROJECTION_GEOGRAPHIC, projection_id, nodatavalue,
                                         grid.columns, grid.rows, 10.0, 21.0, grid.xmin, grid.xmax, grid.ymin,
                                         grid.ymax, dx, dy, secondary.data(), &error);
    if (status == MB_SUCCESS)
      status = mbview_setsecondarycolortable(0, instance, MBV_COLORTABLE_HAXBY, MBV_COLORTABLE_NORMAL, 10.0, 21.0,
                                             1.0, 15.5, MBV_COLORTABLE_NORMAL, &error);
    if (status == MB_SUCCESS)
      status = mbview_update(0, instance, &error);
    check(status == MB_SUCCESS && data->secondary_sameas_primary, "a block and a far strip in tile (0, 0), an overlay "
                                                                  "grid with data in columns 0 to 39");
  }
  mbvq_c_setprojection(instance, 2);
  rhi::MapFrame frame(instance);
  frame.size_core(640, 480);
  rhi::TerrainPlan plan;
  /* the boxes made in the secondary grid mode, then the primary grid mode */
  mbvq_c_view(instance, MBVQ_VIEW_GRID_SECONDARY, true);
  plan_frame(frame, plan, view_options());
  mbvq_c_view(instance, MBVQ_VIEW_GRID_PRIMARY, true);
  plan_frame(frame, plan, view_options());
  double worst = 0.0, worst_raised = 0.0;
  const int outside = outside_boxes(instance, plan, &worst);
  set_exaggeration(instance, 4.0);
  plan_frame(frame, plan, view_options());
  const int outside_raised = outside_boxes(instance, plan, &worst_raised);

  /* the core's 2D camera on the middle of the strip */
  struct mbvq_grid g;
  mbvq_c_grid(instance, &g);
  const int kc = 102 * g.n_rows + 60, kn = 103 * g.n_rows + 60;
  const double cell = std::hypot(g.x[kn] - g.x[kc], g.y[kn] - g.y[kc]);
  camera_2d(instance, MBV_OPENGL_WIDTH / (12.0 * cell), -g.x[kc], -g.y[kc]);
  rhi::PlanResult strip;
  plan_frame(frame, plan, view_options(), &strip);
  const std::vector<int> drawn = drawn_tiles(strip);
  const int t0 = tile_at(plan, 0, 0);
  check(data->display_projection_mode == MBV_PROJECTION_SPHEROID && outside == 0 && outside_raised == 0 &&
            t0 >= 0 && std::find(drawn.begin(), drawn.end(), t0) != drawn.end(),
        "on the spheroid every node with data lies in its tile's box, a far strip between the lattice nodes "
        "included, after boxes made in the secondary grid mode (" +
            std::to_string(outside) + " outside, by up to " + text("%.5f", std::max(0.0, worst)) +
            ") and after an exaggeration change (" + std::to_string(outside_raised) +
            "); a camera on the strip draws its tile (" + std::to_string(strip.drawn.size()) + " tiles drawn)");
  camera_2d(instance, 1.0, 0.0, 0.0);
  set_exaggeration(instance, 1.0);
  mbvq_c_setprojection(instance, 1);
  close_instance(instance);

  /* A very coarse global band, 10 degrees a node: 32 nodes would span 320
     degrees, whose lattice corners lie only 40 degrees apart, so the box's
     lattice is finer there (R2-REV-2). */
  Grid band;
  band.columns = 37;
  band.rows = 3;
  band.xmin = 0.0;
  band.xmax = 360.0;
  band.ymin = -10.0;
  band.ymax = 10.0;
  band.holes = false;
  instance = MBV_NO_WINDOW;
  check(open_grid(band, nullptr, &instance), "a band of 37 x 3 nodes 10 degrees apart around the globe opens");
  mbvq_c_setprojection(instance, 2);
  {
    rhi::MapFrame band_frame(instance);
    band_frame.size_core(640, 480);
    rhi::TerrainPlan band_plan;
    plan_frame(band_frame, band_plan, view_options());
    double band_worst = 0.0, band_worst_raised = 0.0;
    const int band_outside = outside_boxes(instance, band_plan, &band_worst);
    set_exaggeration(instance, 4.0);
    plan_frame(band_frame, band_plan, view_options());
    const int band_outside_raised = outside_boxes(instance, band_plan, &band_worst_raised);
    const int step = band_plan.tiles().size() == 1 ? band_plan.tiles()[0].lattice_step : 0;
    check(data_of(instance)->display_projection_mode == MBV_PROJECTION_SPHEROID && step == 4 && band_outside == 0 &&
              band_outside_raised == 0,
          "on the spheroid, a band 10 degrees a node: the box's lattice every " + std::to_string(step) +
              " nodes, every node with data in its tile's box (" + std::to_string(band_outside) + " outside, by up to " +
              text("%.5f", std::max(0.0, band_worst)) + "), also at exaggeration 4 (" +
              std::to_string(band_outside_raised) + ")");
    set_exaggeration(instance, 1.0);
  }
  mbvq_c_setprojection(instance, 0);
  close_instance(instance);
}

/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
/* 6c: the map's batch - the style, the fill fallback, the batch plan, GLU's
   meshes, contours, the spheroid's second fill and the 3D pick marks */

/* a batch made by hand, freed with it */
struct TestBatch {
  struct mbv_draw_batch b;
  TestBatch() { mbv_draw_batch_init(&b); }
  ~TestBatch() { mbv_draw_batch_free(&b); }
  TestBatch(const TestBatch &) = delete;
  TestBatch &operator=(const TestBatch &) = delete;
  /* a primitive of points (x y z), one colour */
  void primitive(int mode, const std::vector<std::array<float, 3>> &points, float r, float g, float bl) {
    mbv_draw_batch_color3f(&b, r, g, bl);
    mbv_draw_batch_begin(&b, mode);
    for (const auto &p : points)
      mbv_draw_batch_vertex3f(&b, p[0], p[1], p[2]);
    mbv_draw_batch_end(&b);
  }
  /* a rectangle as two triangles */
  void rectangle(float x0, float y0, float x1, float y1, float z, float r, float g, float bl) {
    primitive(MBV_DRAW_TRIANGLES, {{x0, y0, z}, {x1, y0, z}, {x1, y1, z}, {x0, y0, z}, {x1, y1, z}, {x0, y1, z}}, r, g,
              bl);
  }
};

/* the identity camera: display coordinates are normalised device coordinates */
const double kIdentity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

/* the size rule in the test's own words: max(1, floor(logical x ratio +
   0.25)), rounded up only from three quarters, so that thin lines stay thin
   at fractional pixel ratios (chosen by eye at ratios 1, 1.5 and 2) */
int expected_px(double logical, double ratio) {
  return std::max(1, static_cast<int>(std::floor(logical * ratio + 0.25)));
}

void model_style() {
  /* the rule's table: widths 1, 2, 3 and 4, the stipple factor 1 and the
     point size 3; ratio 1.74, just below 1.75 where 1 px lines become 2 px,
     and ratio 1.25 pin the rule's 0.25 */
  struct Row {
    double ratio;
    int widths[4];
    int point;
  };
  const Row rows[] = {{1.0, {1, 2, 3, 4}, 3},
                      {1.25, {1, 2, 4, 5}, 4},
                      {1.5, {1, 3, 4, 6}, 4},
                      {1.74, {1, 3, 5, 7}, 5},
                      {1.75, {2, 3, 5, 7}, 5},
                      {2.0, {2, 4, 6, 8}, 6},
                      {3.0, {3, 6, 9, 12}, 9}};
  bool table = true;
  std::string text_rows;
  for (const Row &row : rows) {
    rhi::Style style;
    style.ratio = row.ratio;
    for (int w = 0; w < 4; w++)
      table = table && style.line(w + 1) == row.widths[w];
    table = table && style.point(3) == row.point && style.contour() == row.widths[0] &&
            style.stipple(1) == row.widths[0];
    text_rows += " " + text("%g:", row.ratio) + std::to_string(style.line(1)) + "," + std::to_string(style.line(2)) +
                 "," + std::to_string(style.line(3)) + "," + std::to_string(style.line(4));
  }
  check(table, "6c: the pixel-ratio rule max(1, floor(logical x ratio + 0.25)) for widths 1 to 4, contours, the "
               "stipple factor and points of 3 (" + text_rows + ")");
  rhi::Style one;
  rhi::Style two;
  two.ratio = 2.0;
  check(rhi::Style::thin(one.line(1)) && !rhi::Style::thin(two.line(1)) && !rhi::Style::thin(one.line(2)) &&
            one.line(0.0) == 1 && one.line(0.4) == 1,
        "6c: a 1 px line is thin at ratio 1 and wide at ratio 2; no width below 1");

  /* the fill fallback while a button is down (5.2.2) */
  using rhi::fill_resolution;
  check(fill_resolution(MBVQ_REZ_LOW, MBVQ_REZ_FULL, false, false, true, 13.0) == MBVQ_REZ_LOW &&
            fill_resolution(MBVQ_REZ_LOW, MBVQ_REZ_FULL, false, false, true, 11.0) == MBVQ_REZ_FULL &&
            fill_resolution(MBVQ_REZ_LOW, MBVQ_REZ_FULL, false, false, false, 13.0) == MBVQ_REZ_FULL &&
            fill_resolution(-1, MBVQ_REZ_NONE, false, false, true, 30.0) == MBVQ_REZ_LOW &&
            fill_resolution(MBVQ_REZ_HIGH, MBVQ_REZ_FULL, true, false, false, -1.0) == MBVQ_REZ_HIGH &&
            fill_resolution(MBVQ_REZ_LOW, MBVQ_REZ_FULL, false, false, true, -1.0) == MBVQ_REZ_FULL,
        "6c: while a button is down a fill falls back to the resolution asked for once a full-resolution plot took "
        "over 12 ms, never without a button or before a full-resolution plot");
}

/* a point of a width x height picture (x right and y down, in pixels) in
   normalised device coordinates, which the identity camera draws there */
std::array<float, 3> ndc(double x, double y, int width, int height) {
  return {static_cast<float>(2.0 * x / width - 1.0), static_cast<float>(1.0 - 2.0 * y / height), 0.0f};
}

/* the stipple counter values of a plan against OpenGL's counter, segment by
   segment: the first value equal modulo 16 x the factor (the plan carries
   the counter to the next segment modulo that, which leaves the pattern's
   bits as they are), the second as far from the first as expected */
bool same_counters(const std::vector<rhi::StippleVertex> &values, const float *expected, size_t n, int factor) {
  bool same = values.size() == n && n % 2 == 0;
  for (size_t k = 0; same && k < n; k += 2) {
    const double first = static_cast<double>(values[k].d) - expected[k];
    const double span = (static_cast<double>(values[k + 1].d) - values[k].d) -
                        (static_cast<double>(expected[k + 1]) - expected[k]);
    same = std::fabs(std::remainder(first, 16.0 * factor)) < 1.0e-3 && std::fabs(span) < 1.0e-3;
  }
  return same;
}

void model_batch_plan() {
  TestBatch t;
  struct mbv_draw_batch *b = &t.b;
  mbv_draw_batch_group(b, MBV_DRAW_GROUP_ROUTE);
  /* a strip of 5, a loop of 4, separate lines of 7 vertices 3 px wide */
  t.primitive(MBV_DRAW_LINE_STRIP, {{0, 0, 0}, {0.1f, 0, 0}, {0.2f, 0, 0}, {0.3f, 0, 0}, {0.4f, 0, 0}}, 1, 0, 0);
  t.primitive(MBV_DRAW_LINE_LOOP, {{0, 0, 0}, {0.1f, 0, 0}, {0.1f, 0.1f, 0}, {0, 0.1f, 0}}, 0, 1, 0);
  mbv_draw_batch_line_width(b, 3.0f);
  t.primitive(MBV_DRAW_LINES, {{0, 0, 0}, {0.1f, 0, 0}, {0.2f, 0, 0}, {0.3f, 0, 0}, {0.4f, 0, 0}, {0.5f, 0, 0}, {0.6f, 0, 0}},
              0, 0, 1);
  mbv_draw_batch_line_width(b, 1.0f);
  /* stippled: a strip, a loop and separate lines */
  mbv_draw_batch_line_stipple(b, 1, 0x1111);
  mbv_draw_batch_enable(b, MBV_DRAW_LINE_STIPPLE, true);
  t.primitive(MBV_DRAW_LINE_STRIP, {{0, 0, 0}, {0.1f, 0, 0}, {0.1f, 0.3f, 0}}, 0, 0, 0);
  t.primitive(MBV_DRAW_LINE_LOOP, {{0, 0, 0}, {0.2f, 0, 0}, {0.2f, 0.1f, 0}}, 0, 0, 0);
  t.primitive(MBV_DRAW_LINES, {{0, 0, 0}, {0.1f, 0, 0}, {0.5f, 0.5f, 0}, {0.5f, 0.9f, 0}}, 0, 0, 0);
  mbv_draw_batch_enable(b, MBV_DRAW_LINE_STIPPLE, false);
  /* markers: a disk shape, three markers, the same list defined again with
     another radius, two more */
  mbv_draw_batch_marker_disks(b, 7, 0.1, 0.08, 10, 10, 0.0f, 1.0f, 0.0f);
  for (int k = 0; k < 3; k++) {
    mbv_draw_batch_color3f(b, 1.0f, 0.0f, 0.0f);
    mbv_draw_batch_marker(b, 7, 0.1f * k, 0.0f, 0.0f);
  }
  mbv_draw_batch_marker_disks(b, 7, 0.2, 0.16, 10, 10, 0.0f, 0.0f, 1.0f);
  for (int k = 0; k < 2; k++) {
    mbv_draw_batch_color3f(b, 1.0f, 0.0f, 0.0f);
    mbv_draw_batch_marker(b, 7, 0.1f * k, 0.5f, 0.0f);
  }

  rhi::BatchPlan plan;
  rhi::Style one;
  plan.build(*b, one, kIdentity, kIdentity, 200, 200);
  const std::vector<rhi::DrawItem> &items = plan.items();
  auto kinds = [&](rhi::DrawKind kind) {
    int n = 0;
    for (const rhi::DrawItem &item : items)
      n += item.kind == kind;
    return n;
  };
  /* in stream order: strip, loop (strip and its closing pair), lines, three
     stippled, two marker runs */
  const bool order = items.size() == 9 && items[0].kind == rhi::DrawKind::LineStrip && items[0].count == 5 &&
                     items[1].kind == rhi::DrawKind::LineStrip && items[1].count == 4 &&
                     items[2].kind == rhi::DrawKind::Lines && items[2].source == rhi::DrawSource::Closings &&
                     items[2].count == 2 && items[2].slot == items[1].slot &&
                     items[3].kind == rhi::DrawKind::WidePairs && items[3].count == 3 &&
                     items[3].offset == static_cast<uint32_t>(b->commands[3].first) * 24 &&
                     items[4].kind == rhi::DrawKind::StippleLines && items[5].kind == rhi::DrawKind::StippleLines &&
                     items[6].kind == rhi::DrawKind::StippleLines && items[7].kind == rhi::DrawKind::Markers &&
                     items[8].kind == rhi::DrawKind::Markers;
  check(order && plan.groups() == 1 && plan.closings().size() == 2 && plan.closings()[0].x == 0.0f &&
            plan.closings()[0].y == 0.1f && plan.closings()[1].y == 0.0f,
        "6c: BatchPlan keeps the stream order; a loop's closing pair goes from its last vertex to its first, in its "
        "slot");
  check(plan.segments() == 4 + 4 + 3 + (2 + 3 + 2),
        "6c: a strip of n vertices gives n - 1 segments, a loop n, 2k + 1 separate vertices k (" +
            std::to_string(plan.segments()) + " segments)");

  /* the stipple counter in physical pixels along the major axis: on along a
     strip and a loop, from 0 at every separate segment; with every vertex on
     whole window coordinates (pixel corners) it is the summed length, since
     each segment's first fragment lies in its start's column and it makes
     one fragment per pixel of its length (compared modulo 16 x the factor,
     same_counters) */
  const std::vector<rhi::StippleVertex> &st = plan.stipple();
  const float expected[] = {0, 10, 10, 40, 0, 20, 20, 30, 30, 50, 0, 10, 0, 40};
  const bool distances = same_counters(st, expected, sizeof(expected) / sizeof(expected[0]), 1);
  check(distances && items[4].count == 4 && items[5].count == 6 && items[6].count == 4 && items[5].first == 4,
        "6c: stipple counter values continue along a strip (0 10 | 10 40) and a loop with its closing segment (0 20 "
        "| 20 30 | 30 50) and restart at each separate segment (0 10 | 0 40), modulo 16 x the factor");

  /* Off whole coordinates the counter is OpenGL's, which counts the
     fragments of the diamond-exit rule, not the summed length: a segment's
     first fragment lies in its start's column, or in the next one when the
     start lies past that column's centre and outside its diamond, and the
     centre of the k-th fragment gets s + k + 1/2. Window coordinates of a
     200 x 200 drawing: the box (20.7, 40.3)-(120.2, 140.6) as a loop (each
     side makes 100 fragments; summed lengths 0 99.5 | 99.5 199.8 | 199.8
     299.3 | 299.3 399.6), a strip with a corner (summed 0 19.5 | 19.5 49.6),
     and separate lines: rightward and leftward from inside the start's
     diamond past its centre, and rightward from outside it (summed 0 40 |
     0 39.7 | 0 29.5). The window's y runs up, the picture's of ndc() down. */
  {
    TestBatch f;
    mbv_draw_batch_line_stipple(&f.b, 1, 0x1111);
    mbv_draw_batch_enable(&f.b, MBV_DRAW_LINE_STIPPLE, true);
    f.primitive(MBV_DRAW_LINE_LOOP,
                {ndc(20.7, 200 - 40.3, 200, 200), ndc(120.2, 200 - 40.3, 200, 200), ndc(120.2, 200 - 140.6, 200, 200),
                 ndc(20.7, 200 - 140.6, 200, 200)},
                0, 0, 0);
    f.primitive(MBV_DRAW_LINE_STRIP,
                {ndc(10.7, 200 - 10.5, 200, 200), ndc(30.2, 200 - 10.5, 200, 200), ndc(30.2, 200 - 40.6, 200, 200)}, 0,
                0, 0);
    f.primitive(MBV_DRAW_LINES,
                {ndc(10.7, 200 - 160.5, 200, 200), ndc(50.7, 200 - 160.5, 200, 200), ndc(50.3, 200 - 180.5, 200, 200),
                 ndc(10.6, 200 - 180.5, 200, 200), ndc(30.8, 200 - 170.0, 200, 200), ndc(60.3, 200 - 170.0, 200, 200)},
                0, 0, 0);
    rhi::BatchPlan counted;
    counted.build(f.b, one, kIdentity, kIdentity, 200, 200);
    const float gl[] = {0.7f, 100.2f, 100.3f, 200.6f, 200.8f, 300.3f, 300.4f, 400.7f,
                        0.7f, 20.2f,  20.5f,  50.6f,
                        0.7f, 40.7f,  0.7f,   40.4f,  -0.2f,  29.3f};
    const std::vector<rhi::StippleVertex> &sc = counted.stipple();
    const bool fragments = same_counters(sc, gl, sizeof(gl) / sizeof(gl[0]), 1);
    std::string found;
    for (size_t k = 0; k < sc.size(); k++)
      found += text(k % 2 == 0 ? (k == 0 ? "%.1f" : " | %.1f") : " %.1f", sc[k].d);
    check(fragments, "6c: the stipple counter counts OpenGL's fragments, modulo 16 x the factor: around a loop 0.7 "
                     "100.2 | 100.3 200.6 | 200.8 300.3 | 300.4 400.7, along a strip 0.7 20.2 | 20.5 50.6, separate "
                     "lines 0.7 40.7 | 0.7 40.4 | -0.2 29.3 (" + found + ")");
  }

  /* The counter goes on modulo 16 x the factor, so it keeps its phase where
     a float would not: in a drawing 2^24 px wide a strip on whole window
     coordinates from x 0 to 2^24 (y 100), back 3 px and up 50 px makes 2^24
     fragments, then 3, then 50, so its last segment starts at 2^24 + 3,
     which is 3 modulo 16 and which a float rounds to 2^24 + 4 */
  {
    TestBatch f;
    mbv_draw_batch_line_stipple(&f.b, 1, 0x0001);
    mbv_draw_batch_enable(&f.b, MBV_DRAW_LINE_STIPPLE, true);
    const float back = 1.0f - 3.0f / 8388608.0f; /* window x 2^24 - 3 */
    f.primitive(MBV_DRAW_LINE_STRIP, {{-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {back, 0.0f, 0.0f}, {back, 0.5f, 0.0f}},
                0, 0, 0);
    rhi::BatchPlan beyond;
    beyond.build(f.b, one, kIdentity, kIdentity, 1 << 24, 200);
    const float expected_beyond[] = {0.0f, 16777216.0f, 0.0f, 3.0f, 3.0f, 53.0f};
    std::string found;
    for (const rhi::StippleVertex &vertex : beyond.stipple())
      found += text(found.empty() ? "%.0f" : " %.0f", vertex.d);
    check(same_counters(beyond.stipple(), expected_beyond, sizeof(expected_beyond) / sizeof(expected_beyond[0]), 1),
          "6c: the stipple counter keeps its phase beyond 2^24 fragments: a strip of 2^24, 3 and 50 fragments goes "
          "on at 0, 2^24 | 0, 3 | 3, 53 modulo 16 (" + found + ")");
  }

  /* the marker runs: instance offsets, and the shape each one names */
  bool runs = kinds(rhi::DrawKind::Markers) == 2 && plan.markers() == 5;
  int run = 0;
  for (const rhi::DrawItem &item : items) {
    if (item.kind != rhi::DrawKind::Markers)
      continue;
    const struct mbv_draw_shape &shape = b->shapes[item.shape];
    runs = runs && item.offset == (run == 0 ? 0u : 3u * 24u) && item.count == (run == 0 ? 3u : 2u) &&
           shape.outer == (run == 0 ? 0.1 : 0.2) &&
           plan.slot_specs()[item.slot].colour[2] == (run == 0 ? 0.0f : 1.0f);
    run++;
  }
  check(runs, "6c: marker runs become instance ranges (offsets 0 and 72 bytes); the shape defined again in "
              "mid-stream serves only the later run, with its own ring colour");

  /* at ratio 2 every line is wide: the strip instanced over its own
     vertices, the loop's closing segment as one pair */
  rhi::Style two;
  two.ratio = 2.0;
  plan.build(*b, two, kIdentity, kIdentity, 400, 400);
  const std::vector<rhi::DrawItem> &wide = plan.items();
  check(wide.size() == 9 && wide[0].kind == rhi::DrawKind::WideStrip && wide[0].count == 4 && wide[0].offset == 0 &&
            wide[1].kind == rhi::DrawKind::WideStrip && wide[1].count == 3 &&
            wide[2].kind == rhi::DrawKind::WidePairs && wide[2].source == rhi::DrawSource::Closings &&
            wide[2].count == 1 && wide[4].kind == rhi::DrawKind::StippleWide && wide[4].count == 2 &&
            plan.slot_specs()[wide[0].slot].width == 2 && plan.slot_specs()[wide[3].slot].width == 6 &&
            plan.slot_specs()[wide[4].slot].factor == 2 && std::fabs(plan.stipple()[3].d - 80.0f) < 1.0e-3f,
        "6c: at ratio 2 the 1 px lines are 2 px quads, the 3 px lines 6 px, the stipple factor 2 and its counter "
        "values doubled");

  /* a quad (a, b, c, d) becomes the triangles (a, b, c) and (a, c, d), in one
     draw of the triangles buffer */
  {
    TestBatch q;
    q.primitive(MBV_DRAW_QUADS, {{0, 0, 0}, {0.5f, 0, 0.1f}, {0.5f, 0.5f, 0.2f}, {0, 0.5f, 0.3f}}, 1, 0, 0);
    rhi::BatchPlan quads;
    quads.build(q.b, one, kIdentity, kIdentity, 200, 200);
    const std::vector<struct mbv_draw_vertex> &tri = quads.triangles();
    const int order[6] = {0, 1, 2, 0, 2, 3};
    bool split = tri.size() == 6 && quads.items().size() == 1 && quads.items()[0].kind == rhi::DrawKind::Triangles &&
                 quads.items()[0].count == 6;
    for (int k = 0; split && k < 6; k++) {
      const struct mbv_draw_vertex &v = q.b.vertices[order[k]];
      split = tri[k].x == v.x && tri[k].y == v.y && tri[k].z == v.z;
    }
    check(split, "6c: a quad (a, b, c, d) becomes the triangles (a, b, c) and (a, c, d)");
  }
}

void model_glu() {
  /* every marker call of the overlays, at three radii, against the vertices
     traced from Mesa's GLU and Apple's GLU, bit for bit */
  int matched[2] = {0, 0};
  int cases[2] = {0, 0};
  std::string mismatches;
  for (const glu_fixtures::Case &c : glu_fixtures::cases) {
    const mbvq::GluVariant variant = c.apple ? mbvq::GluVariant::Apple : mbvq::GluVariant::Sgi;
    const std::vector<rhi::GluPrimitive> mesh = c.disk ? rhi::glu_disk(c.inner * c.outer, c.outer, c.a, c.b, variant)
                                                       : rhi::glu_sphere(c.outer, c.a, c.b, variant);
    bool same = static_cast<int>(mesh.size()) == c.count;
    for (int p = 0; same && p < c.count; p++) {
      const glu_fixtures::Primitive &traced = glu_fixtures::primitives[c.first + p];
      same = mesh[p].mode == traced.mode && static_cast<int>(mesh[p].xyz.size()) == traced.count &&
             std::memcmp(mesh[p].xyz.data(), &glu_fixtures::words[traced.first], 4 * static_cast<size_t>(traced.count)) ==
                 0;
    }
    cases[c.apple]++;
    matched[c.apple] += same;
    if (!same)
      mismatches += std::string(" ") + (c.apple ? "apple " : "sgi ") + c.name + text("@%g", c.outer);
  }
  check(cases[0] == 24 && matched[0] == 24,
        "6c: GLU's disks and spheres as Mesa's GLU makes them, bit for bit (" + std::to_string(matched[0]) + " of " +
            std::to_string(cases[0]) + " traced calls" + mismatches + ")");
  check(cases[1] == 24 && matched[1] == 24,
        "6c: ... and as Apple's GLU makes them, for comparisons on macOS (" + std::to_string(matched[1]) + " of " +
            std::to_string(cases[1]) + ")");

  /* a disk shape's mesh: the fill (a fan of slices triangles), then its ring
     (a quad strip of 2 ring_slices triangles) flagged for the ring colour */
  struct mbv_draw_shape disks = {};
  disks.kind = MBV_DRAW_SHAPE_DISKS;
  disks.outer = 0.5;
  disks.inner = 0.4;
  disks.slices = 4;
  disks.ring_slices = 10;
  const std::vector<float> mesh = rhi::marker_mesh(disks, mbvq::GluVariant::Sgi);
  const size_t fill = 3 * 4, ring = 3 * 2 * 10;
  bool flags = mesh.size() == 4 * (fill + ring);
  for (size_t k = 0; flags && k < fill + ring; k++)
    flags = mesh[4 * k + 3] == (k < fill ? 0.0f : 1.0f);
  struct mbv_draw_shape sphere = {};
  sphere.kind = MBV_DRAW_SHAPE_SPHERE;
  sphere.outer = 0.5;
  sphere.slices = 10;
  sphere.stacks = 10;
  check(flags && rhi::marker_mesh(sphere, mbvq::GluVariant::Sgi).size() == 4 * 3 * (2 * 10 + 8 * 2 * 10),
        "6c: a disk shape's mesh is its fill, then its ring flagged for the ring colour; a sphere's its two fans "
        "and its bands");

  /* the quad strips cover their quads: the ring's triangles add up to the
     annulus between the two regular polygons, the fill's to the polygon, and
     no triangle of the disks or the sphere is degenerate (a triangulation
     that leaves half of each quad out fails here) */
  const auto area = [](const std::vector<float> &m, size_t first, size_t last, double *smallest) {
    double sum = 0.0;
    for (size_t t = first; t < last; t++) {
      const float *p = &m[12 * t];
      const double ux = p[4] - p[0], uy = p[5] - p[1], uz = p[6] - p[2];
      const double vx = p[8] - p[0], vy = p[9] - p[1], vz = p[10] - p[2];
      const double cx = uy * vz - uz * vy, cy = uz * vx - ux * vz, cz = ux * vy - uy * vx;
      const double a = 0.5 * std::sqrt(cx * cx + cy * cy + cz * cz);
      sum += a;
      *smallest = std::min(*smallest, a);
    }
    return sum;
  };
  const double kPi = 3.14159265358979323846;
  double smallest = 1.0e30;
  const double fill_area = area(mesh, 0, 4, &smallest), ring_area = area(mesh, 4, 4 + 20, &smallest);
  const double fill_expected = 0.5 * 4 * std::sin(2.0 * kPi / 4) * 0.5 * 0.5;
  const double ring_expected = 0.5 * 10 * std::sin(2.0 * kPi / 10) * (0.5 * 0.5 - 0.4 * 0.4);
  const std::vector<float> ball = rhi::marker_mesh(sphere, mbvq::GluVariant::Sgi);
  double ball_smallest = 1.0e30;
  area(ball, 0, ball.size() / 12, &ball_smallest);
  check(std::fabs(fill_area - fill_expected) <= 1.0e-6 && std::fabs(ring_area - ring_expected) <= 1.0e-6 &&
            smallest > 1.0e-4 && ball_smallest > 1.0e-5,
        "6c: the quad strips become triangles covering every quad: ring " + text("%.6f", ring_area) + " against " +
            text("%.6f", ring_expected) + ", fill " + text("%.6f", fill_area) + ", no degenerate triangle (smallest " +
            text("%.2g", std::min(smallest, ball_smallest)) + ")");
}

/* the renderer's view bounds on the CPU after a pause: a costly sizing of the
   3D pick marks */
constexpr double kSlowBoundsMs = 100.0;
void slow_viewbounds(size_t instance, int viewbounds[4]) {
  std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(kSlowBoundsMs)));
  mbvq_pick_cpu_viewbounds(instance, viewbounds);
}

/* the map drawings the core asks for (defer_draw), kept for the test's own
   frame drivers; true, as the QRhi backend's */
std::vector<std::pair<size_t, int>> asked_drawings;
bool record_drawing(int view, size_t instance, int rez) {
  if (view == MBV_RENDER_MAP)
    asked_drawings.emplace_back(instance, rez);
  return true;
}

/* the site marker radius in a map's batch (MBV_GLLIST_SITESMALL), and the one
   the core gives it from the corner nodes of the view bounds (vb0, vb2) and
   (vb1, vb3) as mbview_site.c computes it: float differences, then double */
double batch_site_radius(const struct mbv_draw_batch &batch) {
  double radius = -1.0;
  for (int s = 0; s < batch.num_shapes; s++)
    if (batch.shapes[s].list == MBV_GLLIST_SITESMALL)
      radius = batch.shapes[s].outer;
  return radius;
}
double corner_site_radius(const struct mbview_struct *data) {
  const int *vb = data->viewbounds;
  const int k0 = vb[0] * data->primary_n_rows + vb[2];
  const int k1 = vb[1] * data->primary_n_rows + vb[3];
  const double xx = data->primary_x[k1] - data->primary_x[k0];
  const double yy = data->primary_y[k1] - data->primary_y[k0];
  return 0.004 * std::sqrt(xx * xx + yy * yy);
}

void model_map_6c() {
  /* contours: the finest level made, judged by the core's flags */
  {
    Grid grid;
    size_t instance = MBV_NO_WINDOW;
    check(open_grid(grid, nullptr, &instance), "6c: a grid opens for the contours");
    mbvq_c_view(instance, MBVQ_VIEW_CONTOUR, true);
    rhi::MapFrame frame(instance);
    rhi::FrameInput input;
    frame.begin(false, 640, 480, &input);
    const int first = input.contour_rez;
    frame.request(MBVQ_REZ_FULL);
    frame.begin(false, 640, 480, &input);
    const bool full = input.contour_rez == MBVQ_REZ_FULL && input.contours != nullptr && input.contour_vertices > 0;
    const uint64_t generation = input.contour_generation;
    check(first == MBVQ_REZ_LOW && full,
          "6c: contours drawn at the level made: low before the full ones are due, full once asked for (" +
              std::to_string(input.contour_vertices) + " vertices)");
    set_exaggeration(instance, 2.0);
    frame.input(&input, false);
    const bool none = input.contour_rez == MBVQ_REZ_NONE && input.contours == nullptr;
    frame.request(MBVQ_REZ_LOW);
    frame.begin(false, 640, 480, &input);
    const int after = input.contour_rez;
    const bool made = mbvq_c_contours_made(instance, after) && input.contour_generation != generation;
    frame.request(MBVQ_REZ_FULL);
    frame.begin(false, 640, 480, &input);
    check(none && after == MBVQ_REZ_LOW && made && input.contour_rez == MBVQ_REZ_FULL,
          "6c: after an exaggeration change no stale contours are drawn; the level made next (low), then full");
    /* an interval above the range of z: the levels are made without
       segments, which is no contours, as OpenGL's empty list */
    set_contour_interval(instance, 1.0e7);
    frame.request(MBVQ_REZ_LOW);
    frame.begin(false, 640, 480, &input);
    const bool low_made = mbvq_c_contours_made(instance, MBVQ_REZ_LOW);
    const bool low_none = input.contours == nullptr && input.contour_rez == MBVQ_REZ_NONE;
    frame.request(MBVQ_REZ_FULL);
    frame.begin(false, 640, 480, &input);
    check(low_made && low_none && mbvq_c_contours_made(instance, MBVQ_REZ_FULL) && input.contours == nullptr &&
              input.contour_rez == MBVQ_REZ_NONE && input.contour_vertices == 0,
          "6c: contours made without segments (an interval above the range of z) give no contours");
    close_instance(instance);
  }

  /* the fill fallback times only the plots of fills without one-off costs:
     the first drag after opening a view fills at full resolution, and the
     plot of a fill made in it is measured (a fallback of 0 ms: any measured
     time is above it) */
  {
    Grid grid;
    size_t instance = MBV_NO_WINDOW;
    check(open_grid(grid, nullptr, &instance), "6c: a grid opens for the fill fallback");
    rhi::MapFrame frame(instance);
    frame.set_fill_fallback_ms(0.0);
    rhi::FrameInput input;
    frame.begin(false, 640, 480, &input);
    const bool opened = input.rez == MBVQ_REZ_FULL && frame.last_full_ms() < 0.0;
    frame.set_drag(true);
    frame.request(MBVQ_REZ_LOW);
    frame.begin(false, 640, 480, &input);
    const bool first = input.rez == MBVQ_REZ_FULL && frame.fallback_fills() == 0 && frame.last_full_ms() >= 0.0;
    frame.request(MBVQ_REZ_LOW);
    frame.begin(false, 640, 480, &input);
    const bool second = input.rez == MBVQ_REZ_LOW && frame.fallback_fills() == 1;
    /* a fill that makes the full contours after the release keeps the
       time measured before */
    frame.set_drag(false);
    const double measured = frame.last_full_ms();
    mbvq_c_view(instance, MBVQ_VIEW_CONTOUR, true);
    frame.request(MBVQ_REZ_FULL);
    frame.begin(false, 640, 480, &input);
    const bool contoured = input.rez == MBVQ_REZ_FULL && input.contour_rez == MBVQ_REZ_FULL;
    check(opened && first && second && contoured && frame.last_full_ms() == measured,
          "6c: the fill fallback times only the plots of full fills that neither project nor make the full "
          "contours: the first fill of a view is not timed, the first drag after it fills at full resolution and is "
          "timed, the next drag frame falls back, the fill that makes the full contours is not timed (" +
              text("%.3f", measured) + " ms)");
    close_instance(instance);
  }

  /* the spheroid: a second fill after a fill that raised the position mark */
  {
    Grid grid;
    size_t instance = MBV_NO_WINDOW;
    check(open_grid(grid, nullptr, &instance), "6c: a grid opens for the spheroid's second fill");
    mbvq_c_setprojection(instance, 2);
    rhi::MapFrame frame(instance);
    rhi::FrameInput input;
    frame.begin(false, 640, 480, &input);
    const int projected = frame.refills();
    frame.request(MBVQ_REZ_FULL);
    frame.begin(false, 640, 480, &input);
    const int still = frame.refills();
    set_exaggeration(instance, 3.0);
    frame.request(MBVQ_REZ_FULL);
    frame.begin(false, 640, 480, &input);
    const int exaggerated = frame.refills();
    frame.request(MBVQ_REZ_FULL);
    frame.begin(false, 640, 480, &input);
    check(projected == 1 && still == 1 && exaggerated == 2 && frame.refills() == 2,
          "6c: on the spheroid the map is filled again after the projection and after an exaggeration change, "
          "never otherwise (" + std::to_string(frame.refills()) + " second fills)");
    /* a second fill under an active fallback: the resolution of the first,
       one fallback counted, the time measured before kept */
    frame.set_fill_fallback_ms(0.0);
    frame.set_drag(true);
    const double measured = frame.last_full_ms();
    const int fallbacks = frame.fallback_fills();
    set_exaggeration(instance, 4.0);
    frame.request(MBVQ_REZ_LOW);
    frame.begin(false, 640, 480, &input);
    check(measured >= 0.0 && frame.refills() == 3 && input.rez == MBVQ_REZ_LOW &&
              frame.fallback_fills() == fallbacks + 1 && frame.last_full_ms() == measured,
          "6c: the spheroid's second fill under the fallback keeps the first fill's resolution and counts one "
          "fallback (" + std::to_string(frame.fallback_fills() - fallbacks) + ")");
    frame.set_drag(false);
    frame.set_fill_fallback_ms(rhi::kFillFallbackMs);
    mbvq_c_setprojection(instance, 1);
    frame.request(MBVQ_REZ_FULL);
    frame.begin(false, 640, 480, &input);
    check(frame.refills() == 3, "6c: no second fill outside the spheroid");
    close_instance(instance);
  }

  /* 3D: the pick marks sized from the zoomed view's bounds before a full
     fill, the markers from the whole grid (B10) */
  {
    Grid grid;
    grid.mode3d = true;
    grid.holes = false;
    size_t instance = MBV_NO_WINDOW;
    check(open_grid(grid, nullptr, &instance), "6c: a 3D grid opens for the pick marks");
    struct mbview_world_struct *view = &mbviews[instance];
    struct mbview_struct *data = &view->data;
    rhi::MapFrame frame(instance);
    rhi::FrameInput input;
    frame.begin(false, 640, 480, &input);
    mbview_pick(instance, MBV_PICK_DOWN, 320, 240);
    mbview_pick(instance, MBV_PICK_UP, 320, 240);
    const bool picked = data->pick_type == MBV_PICK_ONEPOINT;
    /* a drag with the middle button, the camera moved close to the grid (a
       zoom in), then the release, as the view gets them */
    core_drag(instance, MBV_MOUSE_MOVE, 2, 2, 1);
    view->viewoffset3d_z = 2.0f;
    core_release(instance, 2);
    const bool counter = view->viewboundscount >= MBV_BOUNDSFREQUENCY;
    int vb[4];
    mbvq_pick_cpu_viewbounds(instance, vb);
    const int sizings = frame.pick_sizings();
    frame.begin(false, 640, 480, &input);
    const double scalefactor = std::min(static_cast<double>(vb[1] - vb[0]) / data->primary_n_columns,
                                        static_cast<double>(vb[3] - vb[2]) / data->primary_n_rows);
    const double xlength = 0.05 * scalefactor;
    const double measured = data->pick.endpoints[0].xdisplay - data->pick.xpoints[0].xdisplay;
    const bool whole = data->viewbounds[0] == 0 && data->viewbounds[1] == data->primary_n_columns - 1 &&
                       data->viewbounds[2] == 0 && data->viewbounds[3] == data->primary_n_rows - 1;
    check(picked && counter && frame.pick_sizings() == sizings + 1 && view->viewboundscount == 0 &&
              input.rez == MBVQ_REZ_FULL && std::fabs(measured - xlength) <= 1.0e-4 * xlength && whole &&
              (vb[2] > 0 || vb[3] < data->primary_n_rows - 1),
          "6c: after a 3D pick, a zoom and its release the full fill sizes the pick marks from the zoomed view's "
          "bounds (" + text("%.5f", measured) + " against " + text("%.5f", xlength) +
              "), sets the whole grid for the markers and resets the counter");
    const int before = frame.pick_sizings();
    frame.request(MBVQ_REZ_FULL);
    frame.begin(false, 640, 480, &input);
    check(frame.pick_sizings() == before, "6c: no new sizing until the counter reaches the bounds frequency again");
    /* a drag with the pick shown, released: the frame after the release
       (no request of the core) sizes the marks from the final view. The
       sizing, made slow here, comes back every MBV_BOUNDSFREQUENCY mouse
       events and in the frame after most releases: the fill is timed for
       the fallback by its plot alone, which replaces the time before (that
       of a fill that stalled) */
    core_drag(instance, MBV_MOUSE_MOVE, 2, 2, 1);
    view->viewoffset3d_z = 3.0f;
    core_release(instance, 2);
    mbvq_pick_cpu_viewbounds(instance, vb);
    const struct mbv_render_ops slow = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                        [](int, size_t, int) { return true; }, mbvq_pick_cpu_terrain,
                                        slow_viewbounds};
    mbv_render_set_ops(&slow);
    frame.set_last_full_ms(1000.0);
    frame.begin(false, 640, 480, &input);
    mbvq::rhi_backend::install();
    const double final_length = 0.05 * std::min(static_cast<double>(vb[1] - vb[0]) / data->primary_n_columns,
                                                 static_cast<double>(vb[3] - vb[2]) / data->primary_n_rows);
    check(frame.pick_sizings() == before + 1 && vb[2] > 12 &&
              std::fabs(data->pick.endpoints[0].xdisplay - data->pick.xpoints[0].xdisplay - final_length) <=
                  1.0e-4 * final_length,
          "6c: the frame after a released 3D drag sizes the pick marks from the final view");
    check(frame.last_full_ms() == frame.plot_ms() && frame.last_full_ms() < kSlowBoundsMs &&
              frame.fill_ms() >= kSlowBoundsMs,
          "6c: a full fill that sized the 3D pick marks is timed for the fill fallback by its plot alone (" +
              text("%.3f", frame.last_full_ms()) + " ms replaces 1000 ms; the fill with a sizing of " +
              text("%.0f", kSlowBoundsMs) + " ms took " + text("%.1f", frame.fill_ms()) + " ms)");
    close_instance(instance);
  }

  /* 3D: one fill that stalled (its time set above a threshold that normal
     fills stay below) makes the frames of a drag fall back; the frame after
     the release sizes the marks and measures the plot again, so the next
     drag fills at full resolution */
  {
    Grid grid;
    grid.mode3d = true;
    grid.holes = false;
    size_t instance = MBV_NO_WINDOW;
    const bool opened = open_grid(grid, nullptr, &instance);
    rhi::MapFrame frame(instance);
    rhi::FrameInput input;
    frame.begin(false, 640, 480, &input);
    frame.request(MBVQ_REZ_FULL);
    frame.begin(false, 640, 480, &input);
    frame.set_fill_fallback_ms(500.0);
    frame.set_last_full_ms(1000.0);
    /* an orbit drag (the middle button in the rotate mode) */
    frame.set_drag(true);
    core_drag(instance, MBV_MOUSE_ROTATE, 2, 2, 1);
    frame.request(MBVQ_REZ_LOW);
    frame.begin(false, 640, 480, &input);
    const bool fell = input.rez == MBVQ_REZ_LOW && frame.fallback_fills() == 1;
    core_release(instance, 2);
    frame.set_drag(false);
    const int sizings = frame.pick_sizings();
    frame.begin(false, 640, 480, &input);
    const bool released = input.rez == MBVQ_REZ_FULL && frame.pick_sizings() == sizings + 1;
    const double measured = frame.last_full_ms();
    frame.set_drag(true);
    core_drag(instance, MBV_MOUSE_ROTATE, 2, 2, 1);
    frame.request(MBVQ_REZ_LOW);
    frame.begin(false, 640, 480, &input);
    check(opened && fell && released && measured < 500.0 && input.rez == MBVQ_REZ_FULL &&
              frame.fallback_fills() == 1,
          "6c: in 3D a stalled full fill makes a drag fall back until the frame after the release, which sizes the "
          "pick marks and is timed again (" + text("%.3f", measured) + " ms): the next drag fills at full resolution");
    core_release(instance, 2);
    close_instance(instance);
  }

  /* The spheroid: a frame is judged by the plots it will make. One that
     fills twice (the position mark rose: an exaggeration, Reset View)
     compares twice the time of one full-resolution plot, the first fill's
     plot, the only one timed; one that fills once compares one plot. With a
     threshold between one plot and two, a drag frame that fills once after
     a still frame that filled twice (the dialog's exaggeration, Reset View)
     stays at full resolution, and an exaggeration frame of a drag falls
     back, counted once. */
  {
    Grid grid;
    grid.mode3d = true;
    grid.holes = false;
    size_t instance = MBV_NO_WINDOW;
    const bool opened = open_grid(grid, nullptr, &instance);
    mbvq_c_setprojection(instance, 2);
    rhi::MapFrame frame(instance);
    rhi::FrameInput input;
    frame.begin(false, 640, 480, &input);
    frame.request(MBVQ_REZ_FULL);
    frame.begin(false, 640, 480, &input);
    const bool one = frame.refills() == 1 && frame.last_full_ms() == frame.plot_ms();
    bool stays = true;
    std::string drags;
    for (const bool reset : {false, true}) {
      /* a still frame that fills twice: its first plot is the time kept */
      const int refills = frame.refills();
      if (reset)
        mbvq_c_resetview(instance);
      else
        set_exaggeration(instance, 2.0);
      frame.request(MBVQ_REZ_FULL);
      frame.begin(false, 640, 480, &input);
      const double first = frame.last_full_ms();
      stays = stays && frame.refills() == refills + 1 && first > 0.0 && first < frame.plot_ms();
      /* the next drag frame fills once: one plot, under 1.5 plots */
      frame.set_fill_fallback_ms(1.5 * first);
      frame.set_drag(true);
      const int fallbacks = frame.fallback_fills();
      frame.request(MBVQ_REZ_LOW);
      frame.begin(false, 640, 480, &input);
      stays = stays && input.rez == MBVQ_REZ_FULL && frame.refills() == refills + 1 &&
              frame.fallback_fills() == fallbacks;
      drags += std::string(reset ? ", Reset View: " : " the exaggeration: ") +
               (input.rez == MBVQ_REZ_FULL ? "full" : "not full");
      frame.set_drag(false);
    }
    /* an exaggeration frame of a drag fills twice: two plots, over 1.5 */
    frame.set_fill_fallback_ms(1.5 * frame.last_full_ms());
    frame.set_drag(true);
    const int refills = frame.refills();
    const int fallbacks = frame.fallback_fills();
    set_exaggeration(instance, 3.0);
    frame.request(MBVQ_REZ_LOW);
    frame.begin(false, 640, 480, &input);
    const bool fell =
        input.rez == MBVQ_REZ_LOW && frame.refills() == refills + 1 && frame.fallback_fills() == fallbacks + 1;
    const int counted = frame.fallback_fills() - fallbacks;
    /* a frame whose first fill projected the data (back to the spheroid) is
       not timed, nor is its second fill: the time kept stays */
    frame.set_drag(false);
    mbvq_c_setprojection(instance, 1);
    frame.request(MBVQ_REZ_FULL);
    frame.begin(false, 640, 480, &input);
    mbvq_c_setprojection(instance, 2);
    frame.set_last_full_ms(100.0);
    frame.request(MBVQ_REZ_FULL);
    frame.begin(false, 640, 480, &input);
    const bool kept = frame.refills() == refills + 2 && frame.last_full_ms() == 100.0;
    check(opened && one && stays && fell && kept,
          "6c: on the spheroid a frame is judged by the plots it will make, one full-resolution plot (the first "
          "fill's) twice for a frame that fills twice: with a threshold between one plot and two, a drag frame that "
          "fills once after a still frame that filled twice stays at full resolution (after" + drags +
              "), an exaggeration frame of a drag falls back, counted once (" + std::to_string(counted) +
              "); a frame whose first fill projected is not timed (" + text("%.3f", frame.last_full_ms()) +
              " ms kept)");
    close_instance(instance);
  }

  /* the spheroid's second fill gives the site markers the size of the
     corner nodes as they are for the new exaggeration (mbview_site.c), in
     2D and in 3D: the radius in the batch equals the one from the corners
     prepared again now */
  for (const bool mode3d : {false, true}) {
    Grid grid;
    grid.mode3d = mode3d;
    grid.holes = false;
    size_t instance = MBV_NO_WINDOW;
    const bool opened = open_grid(grid, nullptr, &instance);
    struct mbview_struct *data = data_of(instance);
    int error = MB_ERROR_NO_ERROR;
    double lon[2] = {10.05, 10.15}, lat[2] = {54.05, 54.1}, topo[2] = {-1000.0, -1100.0};
    int color[2] = {MBV_COLOR_RED, MBV_COLOR_BLUE}, size[2] = {1, 1};
    mb_path name[2] = {"site_a", "site_b"};
    mbview_addsites(0, instance, 2, lon, lat, topo, color, size, name, &error);
    mbview_enableviewsites(0, instance, &error);
    mbvq_c_view(instance, MBVQ_VIEW_SITE, true);
    mbvq_c_setprojection(instance, 2);
    rhi::MapFrame frame(instance);
    rhi::FrameInput input;
    frame.begin(false, 640, 480, &input);
    frame.request(MBVQ_REZ_FULL);
    frame.begin(false, 640, 480, &input);
    for (const double exaggeration : {5.0, 30.0}) {
      const int refills = frame.refills();
      set_exaggeration(instance, exaggeration);
      frame.request(MBVQ_REZ_FULL);
      frame.begin(false, 640, 480, &input);
      const double recorded = batch_site_radius(frame.batch());
      const int *vb = data->viewbounds;
      mbview_prepare_nodes(0, instance, vb[0], vb[0], vb[2], vb[2], 1, &error);
      mbview_prepare_nodes(0, instance, vb[1], vb[1], vb[3], vb[3], 1, &error);
      const double fresh = corner_site_radius(data);
      check(opened && recorded > 0.0 && std::fabs(recorded - fresh) <= 1.0e-12 * fresh &&
                frame.refills() == refills + 1,
            std::string("6c: ") + (mode3d ? "3D" : "2D") + " on the spheroid, exaggeration " +
                text("%g", exaggeration) + ": the site radius in the batch " + text("%.10g", recorded) +
                " is that of the corner nodes prepared again now, " + text("%.10g", fresh) + " (difference " +
                text("%.3g", recorded - fresh) + ")");
    }
    close_instance(instance);
  }

  /* Several maps: the core draws the other maps during some drags
     (mbview_plotlowall, in the navigation, site and route modes), and a
     button down in any map view is one state of the process
     (rhi::set_map_buttons, which a view sets from its buttons). During a
     drag in map A in the navigation mode, map B's frames fill at the
     resolution asked for once B's full-resolution plot took longer than the
     threshold (0 ms: any time measured is above it), and at full resolution
     once no button is down. */
  {
    Grid grid;
    size_t a = MBV_NO_WINDOW, b = MBV_NO_WINDOW;
    const bool opened = open_grid(grid, nullptr, &a) && open_grid(grid, nullptr, &b);
    /* down while any owner is down; the same said twice changes nothing */
    int one = 0, two = 0;
    rhi::set_map_buttons(&one, true);
    rhi::set_map_buttons(&one, true);
    rhi::set_map_buttons(&two, true);
    rhi::set_map_buttons(&one, false);
    const bool held = rhi::map_buttons_down();
    rhi::set_map_buttons(&two, false);
    const bool shared = held && !rhi::map_buttons_down();

    const struct mbv_render_ops recording = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                             record_drawing, mbvq_pick_cpu_terrain, mbvq_pick_cpu_viewbounds};
    mbv_render_set_ops(&recording);
    rhi::MapFrame fa(a), fb(b);
    rhi::FrameInput input;
    fa.begin(false, 640, 480, &input);
    fb.begin(false, 640, 480, &input);
    fb.set_fill_fallback_ms(0.0);
    fb.request(MBVQ_REZ_FULL);
    fb.begin(false, 640, 480, &input);
    const bool timed = fb.last_full_ms() > 0.0 && fb.fallback_fills() == 0;
    mbvq_c_set_mouse_mode(a, MBV_MOUSE_NAV);
    asked_drawings.clear();
    /* each event as a view gets it: the buttons first, then the core; the
       frames of both maps after it, for the drawings the core asked for */
    int asked_b = 0, low_b = 0;
    const auto event = [&](int type, int y) {
      rhi::set_map_buttons(&fa, type != MBV_INPUT_RELEASE);
      fa.set_drag(type != MBV_INPUT_RELEASE);
      struct mbv_input_event e = {type, 2, 200, y, 0};
      mbview_input(a, &e);
      bool frame_a = false, frame_b = false;
      for (const auto &drawing : asked_drawings) {
        if (drawing.first == a) {
          fa.request(drawing.second);
          frame_a = true;
        }
        if (drawing.first == b) {
          fb.request(drawing.second);
          frame_b = true;
        }
      }
      asked_drawings.clear();
      if (frame_a)
        fa.begin(false, 640, 480, &input);
      if (frame_b) {
        fb.begin(false, 640, 480, &input);
        asked_b++;
        low_b += input.rez == MBVQ_REZ_LOW;
      }
    };
    event(MBV_INPUT_PRESS, 200);
    for (int m = 1; m <= 3; m++)
      event(MBV_INPUT_MOTION, 200 + 4 * m);
    const bool dragged = asked_b == 4 && low_b == 4 && fb.fallback_fills() == 4 && fb.fell_back() && !fb.drag();
    event(MBV_INPUT_RELEASE, 212);
    mbvq::rhi_backend::install();
    mbvq_c_set_mouse_mode(a, MBV_MOUSE_MOVE);
    check(opened && shared && timed && dragged && asked_b == 5 && input.rez == MBVQ_REZ_FULL &&
              fb.fallback_fills() == 4 && !fb.fell_back() && !rhi::map_buttons_down(),
          "6c: several maps: during a drag in another map (navigation mode) a map the core draws fills at the "
          "resolution asked for once its full-resolution plot took longer than the threshold (" +
              std::to_string(low_b) + " of " + std::to_string(asked_b - 1) +
              " frames), at full resolution once no button is down");
    close_instance(b);
    close_instance(a);
  }
}

/*--------------------------------------------------------------------*/
/* 6c on a Null QRhi: every kind of draw of the batch, the contours uploaded
   once */
void null_batch() {
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
  TestBatch t;
  struct mbv_draw_batch *b = &t.b;
  mbv_draw_batch_slot(b, MBV_DRAW_SLOT_TERRAIN);
  t.primitive(MBV_DRAW_LINE_STRIP, {{0, 0, 0}, {0.5f, 0, 0}, {0.5f, 0.5f, 0}}, 1, 0, 0);
  t.primitive(MBV_DRAW_LINE_LOOP, {{0, 0, 0}, {0.5f, 0, 0}, {0.5f, 0.5f, 0}}, 1, 0, 0);
  mbv_draw_batch_line_width(b, 3.0f);
  t.primitive(MBV_DRAW_LINE_STRIP, {{0, 0, 0}, {0.5f, 0, 0}, {0.5f, 0.5f, 0}}, 1, 0, 0);
  t.primitive(MBV_DRAW_LINES, {{0, 0, 0}, {0.5f, 0, 0}}, 1, 0, 0);
  mbv_draw_batch_point_size(b, 3.0f);
  t.primitive(MBV_DRAW_POINTS, {{0, 0, 0}, {0.5f, 0, 0}}, 1, 0, 0);
  t.rectangle(-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0, 1, 0);
  t.primitive(MBV_DRAW_QUADS, {{0, 0, 0}, {0.1f, 0, 0}, {0.1f, 0.1f, 0}, {0, 0.1f, 0}}, 0, 0, 1);
  mbv_draw_batch_line_width(b, 1.0f);
  mbv_draw_batch_line_stipple(b, 1, 0x1111);
  mbv_draw_batch_enable(b, MBV_DRAW_LINE_STIPPLE, true);
  t.primitive(MBV_DRAW_LINE_LOOP, {{0, 0, 0}, {0.5f, 0, 0}, {0.5f, 0.5f, 0}}, 0, 0, 0);
  mbv_draw_batch_line_width(b, 2.0f);
  t.primitive(MBV_DRAW_LINE_LOOP, {{0, 0, 0}, {0.5f, 0, 0}, {0.5f, 0.5f, 0}}, 0, 0, 0);
  mbv_draw_batch_enable(b, MBV_DRAW_LINE_STIPPLE, false);
  mbv_draw_batch_enable(b, MBV_DRAW_DEPTH_TEST, true);
  mbv_draw_batch_marker_disks(b, 3, 0.1, 0.08, 10, 10, 0, 1, 0);
  mbv_draw_batch_marker(b, 3, 0, 0, 0);
  mbv_draw_batch_marker_sphere(b, 4, 0.1, 10, 10);
  mbv_draw_batch_marker(b, 4, 0.2f, 0, 0);
  mbv_draw_batch_marker(b, 4, 0.3f, 0, 0);
  mbvq::RenderOptions options;
  const bool rendered = renderer.render_batch(b, kIdentity, kIdentity, 200, 200, options, nullptr);
  const mbvq::RenderStats stats = renderer.stats();
  /* strip; loop and closing; wide strip; wide pair; points; triangles;
     quads; stippled thin loop; stippled wide loop; two marker runs */
  check(rendered && !stats.failed && stats.draws == 12 && stats.thin_draws == 4 && stats.wide_draws == 3 &&
            stats.marker_draws == 2 && stats.markers == 3 && stats.meshes == 2 && stats.uniform_slots == 12,
        "6c: every kind of the batch's draws is recorded: " + std::to_string(stats.draws) + " draws (" +
            std::to_string(stats.thin_draws) + " thin, " + std::to_string(stats.wide_draws) + " wide, " +
            std::to_string(stats.marker_draws) + " marker runs), " + std::to_string(stats.uniform_slots) +
            " uniform slots");
  const bool again = renderer.render_batch(b, kIdentity, kIdentity, 200, 200, options, nullptr);
  check(again && renderer.stats().meshes == 2 && renderer.stats().batch_uploads == 2,
        "6c: the marker meshes are kept for later frames; the batch is uploaded in every frame that filled it");
  /* the same display list defined again with another outer radius, then
     another inner one (a zoom redefines the lists): each is a new mesh, not
     the one cached for the list's old radii */
  int remeshed[2] = {0, 0};
  for (int k = 0; k < 2; k++) {
    TestBatch r;
    mbv_draw_batch_enable(&r.b, MBV_DRAW_DEPTH_TEST, true);
    mbv_draw_batch_marker_disks(&r.b, 3, 0.2, k == 0 ? 0.08 : 0.12, 10, 10, 0, 1, 0);
    mbv_draw_batch_marker(&r.b, 3, 0, 0, 0);
    const bool ok = renderer.render_batch(&r.b, kIdentity, kIdentity, 200, 200, options, nullptr);
    remeshed[k] = ok ? renderer.stats().meshes : -1;
  }
  check(remeshed[0] == 3 && remeshed[1] == 4,
        "6c: the marker meshes are cached by the whole shape: a list defined again with another outer or inner "
        "radius gets its own mesh (" + std::to_string(remeshed[0]) + ", " + std::to_string(remeshed[1]) + " meshes)");

  /* a map with contours: their bytes go up once */
  Grid grid;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "6c: a grid opens for the contours under QRhi");
  mbvq_c_view(instance, MBVQ_VIEW_CONTOUR, true);
  mbvq::OffscreenRenderer map(mbvq::OffscreenRenderer::Api::Null);
  mbvq::RenderOptions full;
  mbvq_c_fullrender(instance);
  bool drawn = map.render_map(instance, 320, 240, full, nullptr);
  /* the full contours are due: a full fill makes them */
  drawn = drawn && map.render_map(instance, 320, 240, full, nullptr);
  const mbvq::RenderStats first = map.stats();
  drawn = drawn && map.render_map(instance, 320, 240, full, nullptr);
  const mbvq::RenderStats second = map.stats();
  check(drawn && first.contour_rez > 0 && first.contour_vertices > 0 && second.contour_uploads == first.contour_uploads &&
            second.contour_bytes == first.contour_bytes && second.fills > first.fills,
        "6c: the contours (" + std::to_string(first.contour_vertices) + " vertices at level " +
            std::to_string(first.contour_rez) + ") are uploaded when they change, not in a frame that fills again");

  /* contours made again at the same level with as many vertices (a new
     exaggeration): uploaded again, by their generation */
  set_exaggeration(instance, 2.0);
  mbvq_c_fullrender(instance);
  drawn = map.render_map(instance, 320, 240, full, nullptr);
  const mbvq::RenderStats remade = map.stats();
  check(drawn && remade.contour_rez == second.contour_rez && remade.contour_vertices == second.contour_vertices &&
            remade.contour_uploads == second.contour_uploads + 1,
        "6c: contours made again at the same level with the same count are uploaded again (" +
            std::to_string(remade.contour_uploads - second.contour_uploads) + " upload, level " +
            std::to_string(remade.contour_rez) + ", " + std::to_string(remade.contour_vertices) + " vertices)");

  /* an interval above the range of z: no contour drawn, not the old ones */
  set_contour_interval(instance, 1.0e7);
  mbvq_c_fullrender(instance);
  drawn = map.render_map(instance, 320, 240, full, nullptr);
  drawn = drawn && map.render_map(instance, 320, 240, full, nullptr);
  const mbvq::RenderStats empty = map.stats();
  check(drawn && empty.contour_rez == 0 && empty.contour_vertices == 0 && empty.draws == remade.draws - 1,
        "6c: contours made without segments draw nothing, not the contours of the old interval (" +
            std::to_string(remade.draws) + " draws before, " + std::to_string(empty.draws) + " after)");
  close_instance(instance);

  /* the uniform ring grows past its first 64 slots; if it cannot, the frame
     fails as for the other buffers */
  {
    TestBatch many;
    for (int k = 0; k < 70; k++)
      many.primitive(MBV_DRAW_LINE_STRIP, {{0, 0, 0}, {0.5f, 0.01f * k, 0}}, 1, 0, 0);
    mbvq::OffscreenRenderer ring(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions plain;
    const bool grown = ring.render_batch(&many.b, kIdentity, kIdentity, 200, 200, plain, nullptr);
    const mbvq::RenderStats grown_stats = ring.stats();
    mbvq::OffscreenRenderer stuck(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions fail;
    fail.fail_slot_growth = true;
    TestBatch few;
    few.primitive(MBV_DRAW_LINE_STRIP, {{0, 0, 0}, {0.5f, 0, 0}}, 1, 0, 0);
    const bool small_ok = stuck.render_batch(&few.b, kIdentity, kIdentity, 200, 200, fail, nullptr) &&
                          !stuck.stats().failed && stuck.stats().draws == 1;
    const bool stuck_drawn = stuck.render_batch(&many.b, kIdentity, kIdentity, 200, 200, fail, nullptr);
    const mbvq::RenderStats stuck_stats = stuck.stats();
    check(grown && !grown_stats.failed && grown_stats.uniform_slots == 70 && grown_stats.draws == 70 && small_ok &&
              stuck_stats.failed,
          "6c: the uniform ring grows to " + std::to_string(grown_stats.uniform_slots) +
              " slots; a ring that cannot grow fails the frame (render_batch " + (stuck_drawn ? "true" : "false") +
              ")");
  }
}

/*--------------------------------------------------------------------*/
/* 6c on Metal: pixels of hand-made batches */
uint32_t pixel_rgb(const QImage &image, int column, int row) {
  const QRgb pixel = image.pixel(column, row);
  return static_cast<uint32_t>(qRed(pixel)) | static_cast<uint32_t>(qGreen(pixel)) << 8 |
         static_cast<uint32_t>(qBlue(pixel)) << 16;
}

constexpr uint32_t kRed = 0x0000ff;
constexpr uint32_t kGreen = 0x00ff00;
constexpr uint32_t kBlue = 0xff0000;
constexpr uint32_t kWhite = 0xffffff;

/* the coloured pixels of each column (or row) of an image in a range */
std::vector<int> run_lengths(const QImage &image, uint32_t colour, bool columns, int from, int to) {
  std::vector<int> counts;
  for (int a = from; a < to; a++) {
    int n = 0;
    const int length = columns ? image.height() : image.width();
    for (int b = 0; b < length; b++)
      n += pixel_rgb(image, columns ? a : b, columns ? b : a) == colour;
    counts.push_back(n);
  }
  return counts;
}

bool all_equal(const std::vector<int> &values, int value) {
  return !values.empty() && std::all_of(values.begin(), values.end(), [value](int v) { return v == value; });
}

/* the red pixels of an image in window coordinates (column, row from the
   bottom), sorted */
std::vector<std::pair<int, int>> red_pixels(const QImage &image) {
  std::vector<std::pair<int, int>> found;
  for (int row = 0; row < image.height(); row++)
    for (int x = 0; x < image.width(); x++)
      if (pixel_rgb(image, x, row) == kRed)
        found.emplace_back(x, image.height() - 1 - row);
  std::sort(found.begin(), found.end());
  return found;
}

void metal_batch() {
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Metal);
  mbvq::RenderOptions one;
  mbvq::RenderOptions two;
  two.pixel_ratio = 2.0;
  QImage image;

  /* a 3 px line, nearly horizontal and nearly vertical: exactly 3 px per
     column (row) at ratio 1, 6 at ratio 2, by the minor-axis rule */
  for (int ratio = 1; ratio <= 2; ratio++) {
    TestBatch t;
    mbv_draw_batch_line_width(&t.b, 3.0f);
    t.primitive(MBV_DRAW_LINE_STRIP, {{-0.8f, -0.13f, 0}, {0.8f, 0.11f, 0}}, 1, 0, 0);
    t.primitive(MBV_DRAW_LINES, {{0.53f, -0.8f, 0}, {0.61f, 0.8f, 0}}, 0, 0, 1);
    const int size = 200 * ratio;
    const bool drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, size, size, ratio == 1 ? one : two, &image);
    /* columns well inside the horizontal line and away from the vertical one,
       rows likewise */
    const std::vector<int> columns = run_lengths(image, kRed, true, 30 * ratio, 140 * ratio);
    const std::vector<int> rows = run_lengths(image, kBlue, false, 30 * ratio, 90 * ratio);
    check(drawn && all_equal(columns, 3 * ratio) && all_equal(rows, 3 * ratio) && renderer.stats().wide_draws == 2,
          "6c Metal: a 3 px line covers exactly " + std::to_string(3 * ratio) + " px per column, and per row when "
          "steep, at ratio " + std::to_string(ratio) + " (" + std::to_string(columns.empty() ? 0 : columns.front()) +
              ", " + std::to_string(rows.empty() ? 0 : rows.front()) + ")");
  }

  /* a 1 px line: the line topology at ratio 1, a 2 px quad at ratio 2 */
  {
    TestBatch t;
    t.primitive(MBV_DRAW_LINE_STRIP, {{-0.8f, -0.13f, 0}, {0.8f, 0.11f, 0}}, 1, 0, 0);
    bool drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, 200, 200, one, &image);
    const mbvq::RenderStats thin = renderer.stats();
    const std::vector<int> thin_columns = run_lengths(image, kRed, true, 30, 170);
    drawn = drawn && renderer.render_batch(&t.b, kIdentity, kIdentity, 400, 400, two, &image);
    const mbvq::RenderStats wide = renderer.stats();
    const std::vector<int> wide_columns = run_lengths(image, kRed, true, 60, 340);
    check(drawn && thin.thin_draws == 1 && thin.wide_draws == 0 && all_equal(thin_columns, 1) &&
              wide.wide_draws == 1 && wide.thin_draws == 0 && all_equal(wide_columns, 2) && wide.pixel_ratio == 2.0,
          "6c Metal: a 1 px line is drawn with the line topology at ratio 1 (1 px per column) and as a 2 px quad at "
          "ratio 2");
  }

  /* 2D: without the depth test the later command covers the earlier one */
  {
    TestBatch t;
    t.rectangle(-0.5f, -0.5f, 0.2f, 0.2f, 0.5f, 1, 0, 0);
    t.rectangle(-0.2f, -0.2f, 0.5f, 0.5f, -0.5f, 0, 0, 1);
    t.rectangle(-0.2f, -0.2f, 0.5f, 0.5f, 0.9f, 0, 1, 0);
    const bool drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, 200, 200, one, &image);
    check(drawn && pixel_rgb(image, 100, 100) == kGreen && pixel_rgb(image, 60, 140) == kRed,
          "6c Metal: in 2D the stream order decides overlaps (the last drawn wins, whatever its depth)");
  }

  /* 3D: the depth test Less: the nearest wins in either order, the first at
     equal depth */
  {
    TestBatch t;
    mbv_draw_batch_enable(&t.b, MBV_DRAW_DEPTH_TEST, true);
    t.rectangle(-0.9f, -0.2f, -0.4f, 0.2f, 0.5f, 1, 0, 0);
    t.rectangle(-0.9f, -0.2f, -0.4f, 0.2f, -0.5f, 0, 0, 1);
    t.rectangle(-0.2f, -0.2f, 0.2f, 0.2f, -0.5f, 0, 0, 1);
    t.rectangle(-0.2f, -0.2f, 0.2f, 0.2f, 0.5f, 1, 0, 0);
    t.rectangle(0.4f, -0.2f, 0.9f, 0.2f, 0.0f, 1, 0, 0);
    t.rectangle(0.4f, -0.2f, 0.9f, 0.2f, 0.0f, 0, 0, 1);
    const bool drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, 200, 200, one, &image);
    check(drawn && pixel_rgb(image, 35, 100) == kBlue && pixel_rgb(image, 100, 100) == kBlue &&
              pixel_rgb(image, 165, 100) == kRed,
          "6c Metal: with the depth test the nearer wins whichever comes first, and the first at equal depth");
  }

  /* markers: a disk in the instance's colour with its ring in the shape's
     ring colour; a second disk's fill over the first one's ring */
  {
    TestBatch t;
    mbv_draw_batch_marker_disks(&t.b, 5, 0.2, 0.16, 10, 10, 0.0f, 1.0f, 0.0f);
    mbv_draw_batch_color3f(&t.b, 1, 0, 0);
    mbv_draw_batch_marker(&t.b, 5, -0.4f, 0.0f, 0.0f);
    mbv_draw_batch_color3f(&t.b, 1, 0, 0);
    mbv_draw_batch_marker(&t.b, 5, 0.3f, 0.0f, 0.0f);
    mbv_draw_batch_color3f(&t.b, 0, 0, 1);
    mbv_draw_batch_marker(&t.b, 5, 0.5f, 0.0f, 0.0f);
    one.glu_variant = mbvq::GluVariant::Sgi;
    const bool drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, 200, 200, one, &image);
    /* centre of the first at column 60, its rim 18 px right; the second's
       ring at x 0.48 (column 148) lies inside the third's fill */
    check(drawn && renderer.stats().marker_draws == 1 && pixel_rgb(image, 60, 100) == kRed &&
              pixel_rgb(image, 78, 100) == kGreen && pixel_rgb(image, 42, 100) == kGreen &&
              pixel_rgb(image, 148, 100) == kBlue && pixel_rgb(image, 112, 100) == kGreen,
          "6c Metal: a site's centre in its own colour and its rim in the ring colour; overlapping disks show the "
          "second fill over the first ring (one instanced draw)");
  }

  /* the near plane: a 3 px segment from in front of it to behind the eye
     (w < 0) under a perspective projection (near 1, far 10) is drawn up to
     its crossing of the near plane, 3 px per column, plain and stippled
     (pattern 0xffff); a segment wholly behind it draws nothing */
  {
    const double near_plane = 1.0, far_plane = 10.0;
    const double a = -(far_plane + near_plane) / (far_plane - near_plane);
    const double bz = -2.0 * far_plane * near_plane / (far_plane - near_plane);
    const double perspective[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, a, -1, 0, 0, bz, 0};
    TestBatch t;
    mbv_draw_batch_line_width(&t.b, 3.0f);
    /* NDC x from -0.8 to the crossing at -0.5; y 0.013 eye */
    t.primitive(MBV_DRAW_LINE_STRIP, {{-2.0f, 0.013f, -2.5f}, {2.0f, 0.013f, 1.5f}}, 1, 0, 0);
    /* behind the near plane at both ends */
    t.primitive(MBV_DRAW_LINE_STRIP, {{-1.0f, -0.3f, -0.5f}, {1.0f, -0.3f, 2.0f}}, 0, 1, 0);
    mbv_draw_batch_line_stipple(&t.b, 1, 0xffff);
    mbv_draw_batch_enable(&t.b, MBV_DRAW_LINE_STIPPLE, true);
    /* NDC y 0.4 along its visible part; reversed: the far end first */
    t.primitive(MBV_DRAW_LINE_STRIP, {{2.0f, -0.6f, 1.5f}, {-2.0f, 1.0f, -2.5f}}, 0, 0, 1);
    const bool drawn = renderer.render_batch(&t.b, perspective, perspective, 200, 200, one, &image);
    const std::vector<int> red_in = run_lengths(image, kRed, true, 25, 45);
    const std::vector<int> red_out = run_lengths(image, kRed, true, 56, 200);
    const std::vector<int> blue_in = run_lengths(image, kBlue, true, 25, 45);
    const std::vector<int> blue_out = run_lengths(image, kBlue, true, 56, 200);
    const std::vector<int> green = run_lengths(image, kGreen, true, 0, 200);
    check(drawn && all_equal(red_in, 3) && all_equal(red_out, 0) && all_equal(blue_in, 3) && all_equal(blue_out, 0) &&
              all_equal(green, 0),
          "6c Metal: wide lines are clipped at the near plane: 3 px per column up to the crossing, nothing beyond it "
          "or behind it, plain and stippled (" + std::to_string(red_in.front()) + ", " +
              std::to_string(*std::max_element(red_out.begin(), red_out.end())) + ", " +
              std::to_string(blue_in.front()) + ", " +
              std::to_string(*std::max_element(blue_out.begin(), blue_out.end())) + ")");
  }

  /* the stipple 0x1111 of factor 1 at ratio 1: 1 px on, 3 off */
  {
    TestBatch t;
    mbv_draw_batch_line_stipple(&t.b, 1, 0x1111);
    mbv_draw_batch_enable(&t.b, MBV_DRAW_LINE_STIPPLE, true);
    t.primitive(MBV_DRAW_LINE_STRIP, {{-0.8f, 0.005f, 0}, {0.8f, 0.005f, 0}}, 1, 0, 0);
    const bool drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, 200, 200, one, &image);
    int on = 0;
    for (int x = 40; x < 160; x++)
      on += pixel_rgb(image, x, 99) == kRed || pixel_rgb(image, x, 100) == kRed;
    check(drawn && on == 30, "6c Metal: a stippled line of pattern 0x1111 draws 1 pixel in 4 (" + std::to_string(on) +
                                 " of 120)");
  }

  /* The stipple's phase: OpenGL's counter starts at the first fragment of
     the diamond-exit rule, which lies in the start's column also when the
     start lies past that column's centre but inside its diamond, and goes on
     along a loop by the fragments each side makes. The pixels expected are
     those Apple's legacy OpenGL 2.1 (on Metal) draws for the same float
     coordinates, in window coordinates of the drawing (y up; the picture's
     of ndc() run down). */
  {
    using Dots = std::vector<std::pair<int, int>>;
    bool drawn = true;
    const auto stippled = [&](int mode, const std::vector<std::array<float, 3>> &points, unsigned short pattern,
                              int size, const mbvq::RenderOptions &options) {
      TestBatch t;
      mbv_draw_batch_line_stipple(&t.b, 1, pattern);
      mbv_draw_batch_enable(&t.b, MBV_DRAW_LINE_STIPPLE, true);
      t.primitive(mode, points, 1, 0, 0);
      drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, size, size, options, &image) && drawn;
      return red_pixels(image);
    };
    const auto columns = [](const Dots &dots) {
      std::string list;
      for (const std::pair<int, int> &dot : dots)
        list += " " + std::to_string(dot.first);
      return list;
    };

    /* 0x0001 from x 10.6 to 10.9 rightward (y 20.5): the first fragment in
       column 10, the start's, so the dots are in 10, 26 and 42, not 11, 27
       and 43 */
    std::string right;
    for (double start : {10.6, 10.7, 10.8, 10.9}) {
      const Dots dots = stippled(MBV_DRAW_LINES,
                                 {ndc(start, 200 - 20.5, 200, 200), ndc(start + 40.0, 200 - 20.5, 200, 200)}, 0x0001,
                                 200, one);
      if (right.empty() && dots != Dots{{10, 20}, {26, 20}, {42, 20}})
        right = text(" (from %.1f:", start) + columns(dots) + ")";
    }
    check(drawn && right.empty(), "6c Metal: a stippled line (0x0001) from x 10.6 to 10.9 rightward has its dots in "
                                  "columns 10, 26 and 42, from its start's column" + right);

    /* leftward from x 50.1 to 50.4 (y 40.5): from column 50, not 49 */
    std::string left;
    for (double start : {50.1, 50.2, 50.3, 50.4}) {
      const Dots dots = stippled(MBV_DRAW_LINES,
                                 {ndc(start, 200 - 40.5, 200, 200), ndc(start - 40.0, 200 - 40.5, 200, 200)}, 0x0001,
                                 200, one);
      if (left.empty() && dots != Dots{{18, 40}, {34, 40}, {50, 40}})
        left = text(" (from %.1f:", start) + columns(dots) + ")";
    }
    check(drawn && left.empty(), "6c Metal: a stippled line (0x0001) from x 50.1 to 50.4 leftward has its dots in "
                                 "columns 50, 34 and 18, from its start's column" + left);

    /* 0x1111 from x 20.7 to 140.7 (y 100.5): every fourth column from 20 to
       136, not from 21 */
    Dots fourth;
    for (int k = 0; k < 30; k++)
      fourth.emplace_back(20 + 4 * k, 100);
    const Dots line = stippled(MBV_DRAW_LINES, {ndc(20.7, 200 - 100.5, 200, 200), ndc(140.7, 200 - 100.5, 200, 200)},
                               0x1111, 200, one);
    check(drawn && line == fourth, "6c Metal: a stippled line (0x1111) from x 20.7 has a dot in every fourth column "
                                   "from 20 to 136 (" + std::to_string(line.size()) + " dots, the first in column " +
                                       (line.empty() ? std::string("none") : std::to_string(line.front().first)) +
                                       ")");

    /* the soundings box's case: 0x1111 around (20.7, 40.3)-(120.2, 140.6) as
       a loop; each side makes 100 fragments and the counter goes on from side
       to side, so every side's dots start at its first fragment: (20 + 4k,
       40) along the bottom, (120, 40 + 4k) up the right side, (120 - 4k, 140)
       along the top and (20, 140 - 4k) down the left, k from 0 to 24 */
    Dots box;
    for (int k = 0; k < 25; k++) {
      box.emplace_back(20 + 4 * k, 40);
      box.emplace_back(120, 40 + 4 * k);
      box.emplace_back(120 - 4 * k, 140);
      box.emplace_back(20, 140 - 4 * k);
    }
    std::sort(box.begin(), box.end());
    const Dots loop = stippled(MBV_DRAW_LINE_LOOP,
                               {ndc(20.7, 200 - 40.3, 200, 200), ndc(120.2, 200 - 40.3, 200, 200),
                                ndc(120.2, 200 - 140.6, 200, 200), ndc(20.7, 200 - 140.6, 200, 200)},
                               0x1111, 200, one);
    const auto in_place = std::count_if(loop.begin(), loop.end(), [&box](const std::pair<int, int> &dot) {
      return std::binary_search(box.begin(), box.end(), dot);
    });
    check(drawn && loop == box, "6c Metal: a stippled loop (0x1111) around (20.7, 40.3)-(120.2, 140.6) has OpenGL's "
                                "100 dots, every side's from its first fragment (" + std::to_string(in_place) + " of " +
                                    std::to_string(loop.size()) + " in place)");

    /* A loop around (64.8, 47.8)-(94.8, 117.5), its sides no multiple of the
       pattern's period: its lower corners lie past their columns' centres
       and outside their diamonds (|dx| + |dy| = 0.6), its upper corners on
       their rows' centres inside their diamonds (0.3), each 0.1 or more from
       a diamond's edge. The diamond-exit rule gives its fragments, in the
       order OpenGL makes them, as row 47 from column 65 to 94, column 94 from
       row 48 to 116, row 117 from column 94 to 65 and column 64 from row 117
       to 48. The counter goes on from side to side, so the dots lie where the
       pattern's bit of the count is set: 50 for 0x1111 and 39 for the
       asymmetric 0x0007, as Apple's legacy OpenGL draws them. */
    Dots around;
    for (int x = 65; x <= 94; x++)
      around.emplace_back(x, 47);
    for (int y = 48; y <= 116; y++)
      around.emplace_back(94, y);
    for (int x = 94; x >= 65; x--)
      around.emplace_back(x, 117);
    for (int y = 117; y >= 48; y--)
      around.emplace_back(64, y);
    for (const int pattern : {0x1111, 0x0007}) {
      Dots expected;
      for (size_t s = 0; s < around.size(); s++)
        if (((pattern >> (s % 16)) & 1) != 0)
          expected.push_back(around[s]);
      std::sort(expected.begin(), expected.end());
      const Dots dots = stippled(MBV_DRAW_LINE_LOOP,
                                 {ndc(64.8, 200 - 47.8, 200, 200), ndc(94.8, 200 - 47.8, 200, 200),
                                  ndc(94.8, 200 - 117.5, 200, 200), ndc(64.8, 200 - 117.5, 200, 200)},
                                 static_cast<unsigned short>(pattern), 200, one);
      const auto placed = std::count_if(dots.begin(), dots.end(), [&expected](const std::pair<int, int> &dot) {
        return std::binary_search(expected.begin(), expected.end(), dot);
      });
      check(drawn && dots == expected,
            std::string("6c Metal: a stippled loop (") + (pattern == 0x1111 ? "0x1111" : "0x0007") +
                ") around (64.8, 47.8)-(94.8, 117.5), its lower corners past their columns' centres and outside "
                "their diamonds, its upper corners on their rows' centres inside their diamonds, has OpenGL's " +
                std::to_string(expected.size()) + " dots: each side ends where the diamond-exit rule says and the "
                "counter goes on (" + std::to_string(placed) + " of " + std::to_string(dots.size()) + " in place)");
    }

    /* at ratio 2 the line is a 2 px quad and the factor 2, the counter in
       physical pixels: a line of 1 logical pixel from (10.85, 20.25), 0x0001,
       has its dots in the columns of Apple's 2 px line from (21.7, 40.5) with
       factor 2: 22 (the first dot's other column, 21, lies before the quad),
       53 and 54, 85 and 86, each column whole (2 px) */
    const Dots wide = stippled(MBV_DRAW_LINES, {ndc(10.85, 200 - 20.25, 200, 200), ndc(50.85, 200 - 20.25, 200, 200)},
                               0x0001, 400, two);
    std::map<int, int> per_column;
    for (const std::pair<int, int> &dot : wide)
      per_column[dot.first]++;
    const std::map<int, int> whole = {{22, 2}, {53, 2}, {54, 2}, {85, 2}, {86, 2}};
    std::string wide_found;
    for (const std::pair<const int, int> &column : per_column)
      wide_found += " " + std::to_string(column.first) + ":" + std::to_string(column.second);
    check(drawn && per_column == whole, "6c Metal: at ratio 2 a stippled line (2 px quads, factor 2) has its dots in "
                                        "OpenGL's columns 22, 53-54 and 85-86, each column whole (column:px" +
                                            wide_found + ")");
  }
}

/*--------------------------------------------------------------------*/
/* the buffers of the batch, the contours and the uniform ring: the bytes
   they hold, a ring that cannot grow, and frames that all land on one frame
   slot, with Null and Metal QRhis */
void process_memory(double *resident, double *peak, double *footprint);

/* count line strips of n vertices each, one colour */
void add_strips(TestBatch *t, int count, int n, float r, float g, float bl) {
  for (int k = 0; k < count; k++) {
    mbv_draw_batch_color3f(&t->b, r, g, bl);
    mbv_draw_batch_begin(&t->b, MBV_DRAW_LINE_STRIP);
    for (int v = 0; v < n; v++)
      mbv_draw_batch_vertex3f(&t->b, -0.9f + 1.8f * v / n, -0.9f + 1.8f * static_cast<float>(k % 997) / 997.0f, 0.0f);
    mbv_draw_batch_end(&t->b);
  }
}

/* the bytes of the batch's buffers for a batch, once: its vertices and
   instances, and the plan's loop closings, quads as triangles and stipple */
size_t batch_buffer_bytes(const struct mbv_draw_batch &b, int width, int height) {
  rhi::BatchPlan plan;
  plan.build(b, rhi::Style(), kIdentity, kIdentity, width, height);
  return (static_cast<size_t>(b.num_vertices) + plan.closings().size() + plan.triangles().size()) *
             sizeof(struct mbv_draw_vertex) +
         static_cast<size_t>(b.num_instances) * sizeof(struct mbv_draw_instance) +
         plan.stipple().size() * sizeof(rhi::StippleVertex);
}

/* A uniform ring that cannot grow fails the frame; if it cannot be made
   again at its old size either, the renderer lets go of its pipelines,
   bindings and ring. Either way the failed frame lets go of the batch's
   buffers and the contours and writes nothing into the ring, and a batch of
   fewer commands on the same renderer draws again, twice (on Metal: its line
   in the picture). render_batch() draws no contours: null_buffers checks a
   failed frame that uploaded them. */
void ring_failures(mbvq::OffscreenRenderer::Api api, const char *name) {
  const bool metal = api == mbvq::OffscreenRenderer::Api::Metal;
  TestBatch few;
  few.primitive(MBV_DRAW_LINE_STRIP, {{-0.8f, 0.005f, 0}, {0.8f, 0.005f, 0}}, 1, 0, 0);
  TestBatch many;
  for (int k = 0; k < 70; k++)
    many.primitive(MBV_DRAW_LINE_STRIP, {{-0.8f, -0.5f + 0.01f * k, 0}, {0.8f, -0.5f + 0.01f * k, 0}}, 0, 0, 1);
  bool ok = true;
  std::string kept;
  std::string held;
  std::string drawn_again;
  for (int restore = 0; restore < 2; restore++) {
    mbvq::OffscreenRenderer renderer(api);
    mbvq::RenderOptions fail;
    fail.fail_slot_growth = true;
    fail.fail_slot_restore = restore != 0;
    QImage image;
    QImage *picture = metal ? &image : nullptr;
    const bool first = renderer.render_batch(&few.b, kIdentity, kIdentity, 200, 200, fail, picture);
    const int fixed = renderer.stats().fixed_resources;
    const bool grown = renderer.render_batch(&many.b, kIdentity, kIdentity, 200, 200, fail, picture);
    const mbvq::RenderStats failed = renderer.stats();
    int again = 0;
    for (int k = 0; k < 2; k++) {
      image = QImage();
      const bool drawn = renderer.render_batch(&few.b, kIdentity, kIdentity, 200, 200, fail, picture);
      const mbvq::RenderStats stats = renderer.stats();
      again += drawn && !stats.failed && stats.draws == 1 && stats.fixed_resources == fixed &&
               (!metal || all_equal(run_lengths(image, kRed, true, 40, 160), 1));
    }
    ok = ok && first && fixed > 1 && !grown && failed.failed &&
         failed.fixed_resources == (restore != 0 ? 1 : fixed) && failed.batch_held_bytes == 0 &&
         failed.contour_held_bytes == 0 && failed.ring_bytes == 0 && again == 2;
    kept += (restore != 0 ? ", " : "") + std::to_string(failed.fixed_resources) + " of " + std::to_string(fixed);
    held += (restore != 0 ? "; " : "") + std::to_string(failed.batch_held_bytes) + " and " +
            std::to_string(failed.contour_held_bytes) + " bytes held, " + std::to_string(failed.ring_bytes) +
            " written";
    drawn_again += (restore != 0 ? ", " : "") + std::to_string(again) + " of 2";
  }
  check(ok, "6c " + std::string(name) +
                ": a uniform ring that cannot grow fails the frame, and one that cannot be made again at its old size "
                "either lets go of the pipelines, bindings and ring (" +
                kept + " pipelines, bindings, ring and pass kept); the failed frame lets go of the batch's buffers "
                       "and the contours and writes nothing into the ring (" +
                held + "); a smaller batch on the same renderer then draws again (" + drawn_again + " frames)");
}

void null_buffers() {
  ring_failures(mbvq::OffscreenRenderer::Api::Null, "Null");

  /* the batch's buffers have the size of the batch: a large batch (strips,
     a loop, quads and a stippled loop), then a small one (one strip) */
  {
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
    TestBatch large;
    add_strips(&large, 200, 500, 1, 0, 0);
    large.primitive(MBV_DRAW_LINE_LOOP, {{0, 0, 0}, {0.5f, 0, 0}, {0.5f, 0.5f, 0}}, 0, 1, 0);
    large.primitive(MBV_DRAW_QUADS, {{0, 0, 0}, {0.1f, 0, 0}, {0.1f, 0.1f, 0}, {0, 0.1f, 0}}, 0, 0, 1);
    mbv_draw_batch_line_stipple(&large.b, 1, 0x1111);
    mbv_draw_batch_enable(&large.b, MBV_DRAW_LINE_STIPPLE, true);
    large.primitive(MBV_DRAW_LINE_LOOP, {{0, 0, 0}, {0.5f, 0, 0}, {0.5f, 0.5f, 0}}, 0, 0, 0);
    TestBatch small_batch;
    small_batch.primitive(MBV_DRAW_LINE_STRIP, {{0, 0, 0}, {0.5f, 0, 0}, {0.5f, 0.5f, 0}}, 1, 0, 0);
    mbvq::RenderOptions options;
    bool drawn = renderer.render_batch(&large.b, kIdentity, kIdentity, 200, 200, options, nullptr);
    const mbvq::RenderStats big = renderer.stats();
    drawn = drawn && renderer.render_batch(&small_batch.b, kIdentity, kIdentity, 200, 200, options, nullptr);
    const mbvq::RenderStats little = renderer.stats();
    const size_t large_bytes = batch_buffer_bytes(large.b, 200, 200);
    const size_t small_bytes = batch_buffer_bytes(small_batch.b, 200, 200);
    check(drawn && big.dynamic_copies == 1 && big.batch_held_bytes == large_bytes &&
              little.batch_held_bytes == small_bytes && little.batch_resources == 2,
          "6c Null: the batch's buffers hold " + std::to_string(big.batch_held_bytes) + " bytes for a large batch (" +
              std::to_string(large_bytes) + ") and " + std::to_string(little.batch_held_bytes) +
              " for a small one after it (" + std::to_string(small_bytes) + "; corners and vertices only)");
  }

  /* the contour buffer goes in a frame without contours (the contours
     turned off), and is made and filled again when they come back: turned
     on again, and the same contours after a frame of the same renderer
     without them */
  {
    Grid grid;
    size_t instance = MBV_NO_WINDOW;
    const bool opened = open_grid(grid, nullptr, &instance);
    mbvq_c_view(instance, MBVQ_VIEW_CONTOUR, true);
    mbvq::OffscreenRenderer map(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions options;
    mbvq_c_fullrender(instance);
    bool drawn = opened && map.render_map(instance, 320, 240, options, nullptr);
    drawn = drawn && map.render_map(instance, 320, 240, options, nullptr);
    const mbvq::RenderStats with = map.stats();
    mbvq_c_view(instance, MBVQ_VIEW_CONTOUR, false);
    drawn = drawn && map.render_map(instance, 320, 240, options, nullptr);
    const mbvq::RenderStats without = map.stats();
    mbvq_c_view(instance, MBVQ_VIEW_CONTOUR, true);
    drawn = drawn && map.render_map(instance, 320, 240, options, nullptr);
    const mbvq::RenderStats back = map.stats();
    TestBatch none;
    none.primitive(MBV_DRAW_LINE_STRIP, {{0, 0, 0}, {0.5f, 0, 0}}, 1, 0, 0);
    drawn = drawn && map.render_batch(&none.b, kIdentity, kIdentity, 320, 240, options, nullptr);
    const mbvq::RenderStats between = map.stats();
    drawn = drawn && map.render_map(instance, 320, 240, options, nullptr);
    const mbvq::RenderStats same = map.stats();
    check(drawn && with.contour_vertices > 0 && with.contour_held_bytes == with.contour_vertices * 3 * sizeof(float) &&
              without.contour_vertices == 0 && without.contour_held_bytes == 0 &&
              without.batch_resources == with.batch_resources - 1 && back.contour_vertices > 0 &&
              back.contour_held_bytes == back.contour_vertices * 3 * sizeof(float) &&
              back.contour_uploads > with.contour_uploads && between.contour_held_bytes == 0 &&
              same.contour_rez == back.contour_rez && same.contour_vertices == back.contour_vertices &&
              same.contour_held_bytes == back.contour_held_bytes && same.contour_uploads == back.contour_uploads + 1,
          "6c Null: the contour buffer (" + std::to_string(with.contour_held_bytes) + " bytes) goes in a frame without "
          "contours (" + std::to_string(without.contour_held_bytes) + "), and is made and filled again when they come "
          "back (" + std::to_string(back.contour_held_bytes) + " bytes), also with the same contours after a frame "
          "without them (" + std::to_string(between.contour_held_bytes) + ", then " +
              std::to_string(same.contour_held_bytes) + " bytes, " +
              std::to_string(same.contour_uploads - back.contour_uploads) + " upload)");
    close_instance(instance);
  }

  /* A frame that uploads the contours and then fails, its uniform ring
     unable to grow beyond 64 slots for the terrain, the contours and 40
     routes (a marker run and a line each), holds no contour buffer, and the
     next frame uploads the same contours again: their upload went with the
     failed frame's updates, and the core made no new ones in between. */
  {
    Grid grid;
    size_t instance = MBV_NO_WINDOW;
    const bool opened = open_grid(grid, nullptr, &instance);
    struct mbview_struct *data = data_of(instance);
    int error = MB_ERROR_NO_ERROR;
    const int nc = grid.columns, nr = grid.rows;
    for (int r = 0; r < 40; r++) {
      const int ii[2] = {nc / 8, nc * 7 / 8};
      const int jj[2] = {nr * (r + 1) / 42, nr * (r + 1) / 42};
      double lonlat[4];
      mbvq_c_node_lonlat(instance, 2, ii, jj, lonlat);
      double lon[2] = {lonlat[0], lonlat[2]};
      double lat[2] = {lonlat[1], lonlat[3]};
      int waypoints[2] = {MBV_ROUTE_WAYPOINT_SIMPLE, MBV_ROUTE_WAYPOINT_SIMPLE};
      char name[] = "route";
      int route = 0;
      mbview_addroute(0, instance, 2, lon, lat, waypoints, MBV_COLOR_RED, 2, 0, name, &route, &error);
    }
    mbview_enableviewroutes(0, instance, &error);
    data->route_view_mode = MBV_VIEW_ON;
    mbvq_c_view(instance, MBVQ_VIEW_CONTOUR, true);
    mbvq_c_fullrender(instance);
    mbvq::OffscreenRenderer map(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions fail;
    fail.fail_slot_growth = true;
    const bool refused = opened && !map.render_map(instance, 320, 240, fail, nullptr, 1);
    const mbvq::RenderStats failed = map.stats();
    const bool drawn = map.render_map(instance, 320, 240, mbvq::RenderOptions(), nullptr, 1);
    const mbvq::RenderStats next = map.stats();
    check(refused && failed.failed && failed.uniform_slots > 64 && failed.contour_vertices > 0 &&
              failed.contour_uploads == 1 && failed.contour_held_bytes == 0 && failed.batch_held_bytes == 0 &&
              drawn && !next.failed && next.contour_rez == failed.contour_rez &&
              next.contour_vertices == failed.contour_vertices && next.contour_uploads == 2 &&
              next.contour_held_bytes == next.contour_vertices * 3 * sizeof(float),
          "6c Null: a frame that uploads the contours (" + std::to_string(failed.contour_vertices) +
              " vertices) and then fails (" + std::to_string(failed.uniform_slots) +
              " uniform slots for a ring that cannot grow beyond 64) holds " +
              std::to_string(failed.contour_held_bytes) +
              " contour bytes; the next frame uploads the same contours again (" +
              std::to_string(next.contour_uploads - failed.contour_uploads) + " upload, " +
              std::to_string(next.contour_held_bytes) + " bytes)");
    mbview_deleteallroutes(0, instance, &error);
    close_instance(instance);
  }

  /* The uniform ring is written whole in every frame, the slots beyond the
     commands as zeros: after a frame of 70 commands, which doubles the ring
     of 64 slots to 128, and after a frame of one command after it. A slot of
     112 bytes takes 256 at the 256-byte alignment of a Null QRhi's uniform
     buffers. This checks the size the frame passes to the ring's update
     (RenderStats::ring_bytes); the Metal heap check (metal_buffers) checks
     the write itself. */
  {
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
    TestBatch many;
    add_strips(&many, 70, 2, 0, 0, 1);
    TestBatch few;
    few.primitive(MBV_DRAW_LINE_STRIP, {{0, 0, 0}, {0.5f, 0, 0}}, 1, 0, 0);
    mbvq::RenderOptions options;
    bool drawn = renderer.render_batch(&many.b, kIdentity, kIdentity, 200, 200, options, nullptr);
    const mbvq::RenderStats grown = renderer.stats();
    drawn = drawn && renderer.render_batch(&few.b, kIdentity, kIdentity, 200, 200, options, nullptr);
    const mbvq::RenderStats after = renderer.stats();
    const size_t whole = static_cast<size_t>(128) * 256;
    check(drawn && grown.uniform_slots == 70 && after.uniform_slots == 1 && grown.ring_bytes == whole &&
              after.ring_bytes == whole,
          "6c Null: the uniform ring is written whole in every frame: " + std::to_string(grown.ring_bytes) +
              " bytes in a frame of " + std::to_string(grown.uniform_slots) + " commands and " +
              std::to_string(after.ring_bytes) + " in a frame of " + std::to_string(after.uniform_slots) +
              " after it (128 slots of 256 bytes, " + std::to_string(whole) + ")");
  }
}

/* the malloc heap in use in MB, which holds the writes Qt keeps for a frame
   slot (macOS, where the Metal checks run; 0 elsewhere) */
double heap_in_use() {
#if defined(__APPLE__)
  malloc_statistics_t heap;
  malloc_zone_statistics(nullptr, &heap);
  return static_cast<double>(heap.size_in_use) / 1048576.0;
#else
  return 0.0;
#endif
}

/* An autorelease pool around a frame on macOS, as the event loop puts one
   around each event, and so around each frame of a view: the Metal objects
   a frame autoreleases (its command buffer and encoders, many more with
   Metal's validation layers on) go at the end of the frame instead of
   staying in the malloc heap. libobjc's pool functions are looked up at run
   time, so that the test links nothing new; where they are not found,
   frames run without a pool. */
class FramePool {
 public:
  FramePool() {
    if (functions().push != nullptr)
      pool_ = functions().push();
  }
  ~FramePool() {
    if (pool_ != nullptr)
      functions().pop(pool_);
  }
  FramePool(const FramePool &) = delete;
  FramePool &operator=(const FramePool &) = delete;
  /* whether frames get a pool */
  static bool available() { return functions().push != nullptr; }

 private:
  struct Functions {
    void *(*push)() = nullptr;
    void (*pop)(void *) = nullptr;
  };
  static const Functions &functions() {
    static const Functions looked_up = [] {
      Functions f;
#if defined(__APPLE__)
      void *push = dlsym(RTLD_DEFAULT, "objc_autoreleasePoolPush");
      void *pop = dlsym(RTLD_DEFAULT, "objc_autoreleasePoolPop");
      if (push != nullptr && pop != nullptr) {
        f.push = reinterpret_cast<void *(*)()>(push);
        f.pop = reinterpret_cast<void (*)(void *)>(pop);
      }
#endif
      return f;
    }();
    return looked_up;
  }
  void *pool_ = nullptr;
};

void metal_buffers() {
  ring_failures(mbvq::OffscreenRenderer::Api::Metal, "Metal");

  /* Metal keeps a dynamic buffer once per frame in flight: the held bytes
     count both copies */
  {
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Metal);
    TestBatch t;
    add_strips(&t, 20, 100, 1, 0, 0);
    const bool drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, 200, 200, mbvq::RenderOptions(), nullptr);
    const mbvq::RenderStats stats = renderer.stats();
    check(drawn && stats.dynamic_copies == 2 && stats.batch_held_bytes == 2 * batch_buffer_bytes(t.b, 200, 200),
          "6c Metal: the batch's buffers are counted once per frame in flight (" +
              std::to_string(stats.dynamic_copies) + " copies, " + std::to_string(stats.batch_held_bytes) + " bytes)");
  }

  /* Every frame of a batch on the same frame slot, as when a window draws a
     frame of its own between two frames of the view (a status label that
     repainted after each fill did). The batch fills all five of its
     buffers: 1,900 strips of 25 vertices, 100 loops (their closing
     segments), 2,000 quads (as triangles), 20,000 markers in one run (their
     instances) and a stippled line of 20,000 vertices (its stipple pairs),
     with a uniform slot for each of their 2,003 commands. Each frame and
     the empty frame after it run in an autorelease pool (FramePool), so
     that the malloc heap in use holds Qt's pending writes and not the
     frames' autoreleased Metal objects (without a pool those add 0.5 MB
     over the 90 frames, 38 MB with MTL_DEBUG_LAYER=1 and 71 MB with
     MTL_SHADER_VALIDATION=1). Written whole in every frame, the five
     buffers (3.5 MB) and the ring of 2,048 slots (0.5 MB) leave Qt one
     pending write per slot, so the heap stays flat over the last 90 frames
     (at most 0.08 MB more on an Apple M3 Max, also with either variable
     set; 8 MB allowed). Before, each frame left its writes pending for the
     other slot, about 4 MB per frame: 165 MB more with the four buffers
     other than the vertices static, 45 MB with the ring written for the
     commands only. The footprint is shown for information only: it moves
     by more than that in the same frames. */
  {
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Metal);
    TestBatch t;
    add_strips(&t, 1900, 25, 1, 0, 0);
    for (int k = 0; k < 100; k++)
      t.primitive(MBV_DRAW_LINE_LOOP, {{-0.5f, -0.5f, 0}, {0.5f, -0.5f, 0}, {0.5f, 0.5f, 0}}, 0, 0, 1);
    std::vector<std::array<float, 3>> quads;
    for (int k = 0; k < 2000; k++) {
      const float x = -0.9f + 0.0008f * k;
      quads.insert(quads.end(), {{x, 0.0f, 0.0f}, {x + 0.1f, 0.0f, 0.0f}, {x + 0.1f, 0.1f, 0.0f}, {x, 0.1f, 0.0f}});
    }
    t.primitive(MBV_DRAW_QUADS, quads, 0, 1, 0);
    mbv_draw_batch_marker_disks(&t.b, 3, 0.01, 0.008, 10, 10, 0, 0, 0);
    mbv_draw_batch_color3f(&t.b, 0, 1, 0);
    for (int k = 0; k < 20000; k++)
      mbv_draw_batch_marker(&t.b, 3, -0.9f + 1.8f * (k % 200) / 200.0f, -0.9f + 1.8f * (k / 200) / 100.0f, 0.0f);
    mbv_draw_batch_line_stipple(&t.b, 1, 0x1111);
    mbv_draw_batch_enable(&t.b, MBV_DRAW_LINE_STIPPLE, true);
    std::vector<std::array<float, 3>> line;
    for (int v = 0; v < 20000; v++)
      line.push_back({-0.9f + 1.8f * v / 20000.0f, 0.3f, 0.0f});
    t.primitive(MBV_DRAW_LINE_STRIP, line, 0, 0, 0);
    rhi::BatchPlan plan;
    plan.build(t.b, rhi::Style(), kIdentity, kIdentity, 256, 256);
    const bool five = t.b.num_vertices > 0 && t.b.num_instances > 0 && !plan.closings().empty() &&
                      !plan.triangles().empty() && !plan.stipple().empty();
    mbvq::RenderOptions options;
    bool drawn = true;
    int on_slot[2] = {0, 0};
    double start = -1.0, resident = 0.0, peak = 0.0, heap_start = 0.0;
    for (int f = 0; f < 120 && drawn; f++) {
      {
        const FramePool pool;
        drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, 256, 256, options, nullptr);
        const int slot = renderer.stats().frame_slot;
        on_slot[slot == 0 ? 0 : 1]++;
        drawn = drawn && renderer.empty_frame();
      }
      if (f + 1 == 30) {
        process_memory(&resident, &peak, &start);
        heap_start = heap_in_use();
      }
    }
    const double heap = heap_in_use() - heap_start;
    double end = -1.0;
    process_memory(&resident, &peak, &end);
    const int slot = renderer.stats().frame_slot;
    const int ring_slots = renderer.stats().uniform_slots;
    /* two batch frames in a row: the second lands on the other slot */
    drawn = drawn && renderer.render_batch(&t.b, kIdentity, kIdentity, 256, 256, options, nullptr) &&
            renderer.render_batch(&t.b, kIdentity, kIdentity, 256, 256, options, nullptr);
    const int next = renderer.stats().frame_slot;
    check(drawn && five && ring_slots == 2003 && (on_slot[0] == 120 || on_slot[1] == 120) && next != slot &&
              heap_start > 0.0 && heap < 8.0,
          "6c Metal: 90 frames of a batch that fills all five of its buffers (" +
              text("%.1f", batch_buffer_bytes(t.b, 256, 256) / 1048576.0) + " MB: " +
              std::to_string(t.b.num_vertices) + " vertices, " + std::to_string(t.b.num_instances) +
              " instances, loop closings, quads as triangles and stipple pairs) and " + std::to_string(ring_slots) +
              " uniform slots, all on frame slot " + std::to_string(slot) +
              (FramePool::available() ? ", each in an autorelease pool" : ", without autorelease pools") +
              ", leave the malloc heap flat (" + text("%+.1f", heap) + " MB, less than 8 MB; footprint " +
              text("%.1f", start) + " to " + text("%.1f", end) + " MB); the next frame on slot " +
              std::to_string(next));
  }
}

/*--------------------------------------------------------------------*/
/* 6c: what one segment, one marker run and square pictures cannot show -
   the points pipeline and its size, the instance strides of wide lines and
   the closing segments of loops, the picture's width and height kept apart
   in the shaders, marker runs after the first, the contours drawn, and the
   marker meshes the renderer keeps */

/* The pixels of a colour across a segment w px wide from (x0, y0) to (x1,
   y1), in picture pixels: for each column of its middle part, w + 2 px from
   its ends (each row where the segment is steep), those within w + 3 px of
   the segment, so that segments further apart are counted each on its own. */
std::vector<int> across(const QImage &image, uint32_t colour, double x0, double y0, double x1, double y1, int w) {
  std::vector<int> counts;
  if (image.isNull())
    return counts;
  const bool steep = std::fabs(y1 - y0) > std::fabs(x1 - x0);
  /* u along the segment, v across it */
  const double u0 = steep ? y0 : x0, u1 = steep ? y1 : x1, v0 = steep ? x0 : y0, v1 = steep ? x1 : y1;
  const int length = steep ? image.height() : image.width(), breadth = steep ? image.width() : image.height();
  const int first = std::max(0, static_cast<int>(std::ceil(std::min(u0, u1) + w + 2)));
  const int last = std::min(length - 1, static_cast<int>(std::floor(std::max(u0, u1) - w - 2)));
  for (int u = first; u <= last; u++) {
    const double v = v0 + (u + 0.5 - u0) * (v1 - v0) / (u1 - u0);
    const int low = std::max(0, static_cast<int>(std::floor(v - w - 3)));
    const int high = std::min(breadth - 1, static_cast<int>(std::ceil(v + w + 3)));
    int n = 0;
    for (int c = low; c <= high; c++)
      n += pixel_rgb(image, steep ? c : u, steep ? u : c) == colour;
    counts.push_back(n);
  }
  return counts;
}

/* counts as "3", or as "1..4" where they differ; "none" without any */
std::string spread(const std::vector<int> &counts) {
  if (counts.empty())
    return "none";
  const auto range = std::minmax_element(counts.begin(), counts.end());
  return std::to_string(*range.first) + (*range.first == *range.second ? "" : ".." + std::to_string(*range.second));
}

/* a colour of the pictures by name */
std::string colour_name(uint32_t colour) {
  if (colour == kRed || colour == kGreen || colour == kBlue)
    return colour == kRed ? "red" : colour == kGreen ? "green" : "blue";
  if (colour == kWhite || colour == 0u)
    return colour == 0u ? "black" : "white";
  char buffer[16];
  std::snprintf(buffer, sizeof(buffer), "rgb %06x", static_cast<unsigned>(colour));
  return buffer;
}

/* Points of the batch: drawn by the points pipeline at the point size (3 px
   times the ratio), not at the line width (1 px for the first point, 4 px
   for the second). Their centres lie off the pixel edges (window x 100.125
   and 150.125, y 100.125 at ratio 1), so each covers an exact square:
   columns 99 to 101 and 149 to 151, image rows 98 to 100 at ratio 1, and
   6 x 6 px at ratio 2. */
void metal_points() {
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Metal);
  const float centres[2][2] = {{0.00125f, 0.00125f}, {0.50125f, 0.00125f}};
  for (int ratio = 1; ratio <= 2; ratio++) {
    TestBatch t;
    mbv_draw_batch_point_size(&t.b, 3.0f);
    mbv_draw_batch_line_width(&t.b, 1.0f);
    t.primitive(MBV_DRAW_POINTS, {{centres[0][0], centres[0][1], 0}}, 1, 0, 0);
    mbv_draw_batch_line_width(&t.b, 4.0f);
    t.primitive(MBV_DRAW_POINTS, {{centres[1][0], centres[1][1], 0}}, 1, 0, 0);
    mbvq::RenderOptions options;
    options.pixel_ratio = ratio;
    const int size = 200 * ratio, side = 3 * ratio;
    QImage image;
    const bool drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, size, size, options, &image);
    /* the first column and row whose pixel centres lie in a square */
    const auto first = [side](double centre) { return static_cast<int>(std::ceil(centre - 0.5 * side - 0.5)); };
    const int columns[2] = {first((centres[0][0] + 1.0) * 0.5 * size), first((centres[1][0] + 1.0) * 0.5 * size)};
    const int row = first((1.0 - centres[0][1]) * 0.5 * size);
    int inside = 0, outside = 0;
    for (int y = 0; drawn && y < size; y++)
      for (int x = 0; x < size; x++) {
        const bool square = y >= row && y < row + side &&
                            ((x >= columns[0] && x < columns[0] + side) || (x >= columns[1] && x < columns[1] + side));
        const bool red = pixel_rgb(image, x, y) == kRed;
        inside += red && square;
        outside += red && !square;
      }
    const mbvq::RenderStats stats = renderer.stats();
    check(drawn && inside == 2 * side * side && outside == 0 && stats.draws == 2 && stats.thin_draws == 0 &&
              stats.wide_draws == 0,
          "6c Metal: points of size 3, drawn with the line widths 1 and 4, cover " + std::to_string(side) + " x " +
              std::to_string(side) + " px each at ratio " + std::to_string(ratio) + ", from columns " +
              std::to_string(columns[0]) + " and " + std::to_string(columns[1]) + " and row " + std::to_string(row) +
              " (" + std::to_string(inside) + " px there, " + std::to_string(outside) + " elsewhere)");
  }
}

/* Wide lines on pictures that are not square, each segment measured across
   its middle part: a picture's width and height swapped in the shaders make
   lines too thin one way and too wide the other, and a wrong instance stride
   leaves a segment out.
   - a strip of three segments (a staircase), 3 px at ratio 1 and 1 px at
     ratio 2 (2 px quads): a strip's segments come from two bindings of its
     vertices, one vertex apart;
   - three separate 3 px lines, each a pair of vertices;
   - the 3 px staircase stippled with 0xffff, one pair of stipple vertices
     per segment;
   - a 3 px loop, whose closing side comes from the plan's closing pairs.
   The corners lie at a quarter pixel, so that the edges of the quads, w/2
   from them, pass a quarter pixel from the pixel centres. */
void metal_wide_layouts() {
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Metal);
  struct Corner {
    double x;
    double y;
  };
  /* a primitive through corners given in picture pixels */
  const auto primitive = [](TestBatch *t, int mode, const std::vector<Corner> &corners, int width, int height,
                            float r, float g, float bl) {
    std::vector<std::array<float, 3>> points;
    for (const Corner &c : corners)
      points.push_back(ndc(c.x, c.y, width, height));
    t->primitive(mode, points, r, g, bl);
  };
  /* whether each segment (corner to corner) is exactly w px across, with
     the counts of each in text */
  const auto exact = [](const QImage &image, uint32_t colour, const std::vector<std::pair<Corner, Corner>> &segments,
                        int w, std::string *counts_text) {
    bool all = true;
    for (const auto &segment : segments) {
      const std::vector<int> counts =
          across(image, colour, segment.first.x, segment.first.y, segment.second.x, segment.second.y, w);
      all = all && all_equal(counts, w);
      *counts_text += (counts_text->empty() ? "" : ", ") + spread(counts);
    }
    return all;
  };
  mbvq::RenderOptions one;
  mbvq::RenderOptions two;
  two.pixel_ratio = 2.0;

  /* the staircase: 3 px at ratio 1 on 320 x 160, 1 px at ratio 2 on 400 x 300 */
  for (int ratio = 1; ratio <= 2; ratio++) {
    const int width = ratio == 1 ? 320 : 400, height = ratio == 1 ? 160 : 300, w = ratio == 1 ? 3 : 2;
    const std::vector<Corner> stair =
        ratio == 1 ? std::vector<Corner>{{32.25, 112.25}, {144.25, 112.25}, {144.25, 48.25}, {288.25, 48.25}}
                   : std::vector<Corner>{{40.25, 210.25}, {180.25, 210.25}, {180.25, 90.25}, {360.25, 90.25}};
    TestBatch t;
    mbv_draw_batch_line_width(&t.b, ratio == 1 ? 3.0f : 1.0f);
    primitive(&t, MBV_DRAW_LINE_STRIP, stair, width, height, 1, 0, 0);
    QImage image;
    const bool drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, width, height, ratio == 1 ? one : two, &image);
    std::string counts;
    const bool segments =
        exact(image, kRed, {{stair[0], stair[1]}, {stair[1], stair[2]}, {stair[2], stair[3]}}, w, &counts);
    check(drawn && segments && renderer.stats().wide_draws == 1 && renderer.stats().thin_draws == 0,
          "6c Metal: a " + std::to_string(ratio == 1 ? 3 : 1) + " px strip of three segments (a staircase) at ratio " +
              std::to_string(ratio) + " on " + std::to_string(width) + " x " + std::to_string(height) +
              ": each segment exactly " + std::to_string(w) + " px across (" + counts + ")");
  }

  /* three separate 3 px lines on 160 x 320 */
  {
    const std::vector<Corner> lines = {{16.25, 64.25},   {144.25, 64.25}, {16.25, 160.25},
                                       {144.25, 160.25}, {16.25, 256.25}, {144.25, 256.25}};
    TestBatch t;
    mbv_draw_batch_line_width(&t.b, 3.0f);
    primitive(&t, MBV_DRAW_LINES, lines, 160, 320, 0, 0, 1);
    QImage image;
    const bool drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, 160, 320, one, &image);
    std::string counts;
    const bool pairs =
        exact(image, kBlue, {{lines[0], lines[1]}, {lines[2], lines[3]}, {lines[4], lines[5]}}, 3, &counts);
    check(drawn && pairs && renderer.stats().wide_draws == 1,
          "6c Metal: three separate 3 px lines on 160 x 320: each exactly 3 px across (" + counts + ")");
  }

  /* the 3 px staircase stippled with 0xffff on 160 x 320 */
  {
    const std::vector<Corner> stair = {{16.25, 256.25}, {64.25, 256.25}, {64.25, 64.25}, {144.25, 64.25}};
    TestBatch t;
    mbv_draw_batch_line_width(&t.b, 3.0f);
    mbv_draw_batch_line_stipple(&t.b, 1, 0xffff);
    mbv_draw_batch_enable(&t.b, MBV_DRAW_LINE_STIPPLE, true);
    primitive(&t, MBV_DRAW_LINE_STRIP, stair, 160, 320, 1, 0, 0);
    QImage image;
    const bool drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, 160, 320, one, &image);
    std::string counts;
    const bool segments =
        exact(image, kRed, {{stair[0], stair[1]}, {stair[1], stair[2]}, {stair[2], stair[3]}}, 3, &counts);
    check(drawn && segments && renderer.stats().wide_draws == 1,
          "6c Metal: the 3 px staircase stippled with 0xffff on 160 x 320: each segment exactly 3 px across (" +
              counts + ")");
  }

  /* a 3 px loop on 400 x 300, its closing side (the left one) last */
  {
    const std::vector<Corner> loop = {{100.25, 225.25}, {300.25, 225.25}, {300.25, 75.25}, {100.25, 75.25}};
    TestBatch t;
    mbv_draw_batch_line_width(&t.b, 3.0f);
    primitive(&t, MBV_DRAW_LINE_LOOP, loop, 400, 300, 0, 0, 1);
    QImage image;
    const bool drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, 400, 300, one, &image);
    std::string counts;
    const bool sides = exact(
        image, kBlue, {{loop[0], loop[1]}, {loop[1], loop[2]}, {loop[2], loop[3]}, {loop[3], loop[0]}}, 3, &counts);
    check(drawn && sides && renderer.stats().wide_draws == 2,
          "6c Metal: a 3 px loop on 400 x 300: each side exactly 3 px across, the closing side last (" + counts + ")");
  }
}

/* Three marker runs, the first shape used again after another, so that the
   third run starts at instance 3 (72 bytes into the instances): each
   marker's centre in its own colour (a run drawn from the first instance
   puts the first marker's colour at the first marker's place and leaves its
   own place white). */
void metal_marker_runs() {
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Metal);
  /* the list, the place (x; its column at y 0), the colour */
  struct Marker {
    int list;
    float x;
    int column;
    float r, g, b;
    uint32_t colour;
  };
  const Marker markers[] = {{5, -0.6f, 40, 1, 0, 0, kRed},
                            {5, -0.3f, 70, 1, 0, 0, kRed},
                            {6, 0.15f, 115, 0, 1, 0, kGreen},
                            {5, 0.55f, 155, 0, 0, 1, kBlue}};
  TestBatch t;
  mbv_draw_batch_marker_disks(&t.b, 5, 0.08, 0.064, 10, 10, 0.0f, 0.0f, 0.0f);
  mbv_draw_batch_marker_disks(&t.b, 6, 0.1, 0.08, 10, 10, 0.0f, 0.0f, 0.0f);
  for (const Marker &m : markers) {
    mbv_draw_batch_color3f(&t.b, m.r, m.g, m.b);
    mbv_draw_batch_marker(&t.b, m.list, m.x, 0.0f, 0.0f);
  }
  QImage image;
  const bool drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, 200, 200, mbvq::RenderOptions(), &image);
  bool centres = drawn;
  std::string found;
  for (const Marker &m : markers) {
    const uint32_t pixel = drawn ? pixel_rgb(image, m.column, 100) : 0xffffffu;
    centres = centres && pixel == m.colour;
    found += (found.empty() ? "" : ", ") + colour_name(pixel);
  }
  check(centres && renderer.stats().marker_draws == 3 && renderer.stats().markers == 4,
        "6c Metal: three marker runs (shapes 5, 6, then 5 again): each marker's centre in its own colour (" + found +
            "; red, red, green, blue expected), " + std::to_string(renderer.stats().marker_draws) + " runs");
}

/* The contours of the synthetic grid on 400 x 300, at ratio 1 (the line
   topology) and at ratio 2 (2 px quads), against the segments of the level
   drawn (mbview_contour_segments) projected with the core's camera: a black
   pixel within 1 px of the middle of every segment of 3 px or more, none
   further than 2.5 px from every segment, and one draw of the kind the width
   asks for. The terrain alone has no black pixel. */
void metal_contours() {
  const int width = 400, height = 300;
  Grid grid;
  size_t instance = MBV_NO_WINDOW;
  const bool opened = open_grid(grid, nullptr, &instance);
  mbvq::RenderOptions options;
  options.force_stride1 = true;
  int terrain_black = -1;
  if (opened) {
    mbvq::OffscreenRenderer metal(mbvq::OffscreenRenderer::Api::Metal);
    QImage terrain;
    if (metal.render_map(instance, width, height, options, &terrain)) {
      terrain_black = 0;
      for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
          terrain_black += pixel_rgb(terrain, x, y) == 0u;
    }
    mbvq_c_view(instance, MBVQ_VIEW_CONTOUR, true);
    mbvq_c_fullrender(instance);
  }
  for (int ratio = 1; ratio <= 2; ratio++) {
    mbvq::OffscreenRenderer metal(mbvq::OffscreenRenderer::Api::Metal);
    options.pixel_ratio = ratio;
    QImage picture;
    /* the second frame draws the contours the first one's fill made */
    bool drawn = opened && metal.render_map(instance, width, height, options, &picture);
    drawn = drawn && metal.render_map(instance, width, height, options, &picture);
    const mbvq::RenderStats stats = metal.stats();
    const float *xyz = nullptr;
    size_t n = 0;
    uint64_t generation = 0;
    int error = MB_ERROR_NO_ERROR;
    if (drawn)
      mbview_contour_segments(0, instance, stats.contour_rez, &xyz, &n, &generation, &error);
    double projection[16] = {}, modelview[16] = {}, camera[16] = {};
    if (drawn) {
      mbvq_c_camera(instance, projection, modelview);
      rhi::multiply(projection, modelview, camera);
    }
    /* a vertex in picture pixels (x right, y down); false behind the camera */
    const auto to_picture = [&](const float *p, double *x, double *y) {
      double c[4];
      for (int r = 0; r < 4; r++)
        c[r] = camera[r] * p[0] + camera[4 + r] * p[1] + camera[8 + r] * p[2] + camera[12 + r];
      if (!(c[3] > 0.0))
        return false;
      *x = (c[0] / c[3] + 1.0) * 0.5 * width;
      *y = (1.0 - c[1] / c[3]) * 0.5 * height;
      return true;
    };
    /* the pixels within 2.5 px of a segment (not named near, which
       <windows.h> defines away), and the black pixels at the segments'
       middles */
    std::vector<unsigned char> around(static_cast<size_t>(width) * height, 0);
    int middles = 0, marked = 0;
    for (size_t s = 0; xyz != nullptr && s + 1 < n; s += 2) {
      double ax, ay, bx, by;
      if (!to_picture(&xyz[3 * s], &ax, &ay) || !to_picture(&xyz[3 * s + 3], &bx, &by))
        continue;
      const double dx = bx - ax, dy = by - ay, length2 = dx * dx + dy * dy;
      const int x0 = std::max(0, static_cast<int>(std::floor(std::min(ax, bx) - 3.0)));
      const int x1 = std::min(width - 1, static_cast<int>(std::ceil(std::max(ax, bx) + 3.0)));
      const int y0 = std::max(0, static_cast<int>(std::floor(std::min(ay, by) - 3.0)));
      const int y1 = std::min(height - 1, static_cast<int>(std::ceil(std::max(ay, by) + 3.0)));
      for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++) {
          const double t =
              length2 > 0.0 ? std::clamp(((x + 0.5 - ax) * dx + (y + 0.5 - ay) * dy) / length2, 0.0, 1.0) : 0.0;
          if (std::hypot(x + 0.5 - (ax + t * dx), y + 0.5 - (ay + t * dy)) <= 2.5)
            around[static_cast<size_t>(y) * width + x] = 1;
        }
      const double mx = 0.5 * (ax + bx), my = 0.5 * (ay + by);
      if (length2 >= 9.0 && mx >= 4.0 && mx <= width - 4.0 && my >= 4.0 && my <= height - 4.0) {
        middles++;
        const int cx = static_cast<int>(std::floor(mx)), cy = static_cast<int>(std::floor(my));
        bool black = false;
        for (int y = cy - 1; y <= cy + 1; y++)
          for (int x = cx - 1; x <= cx + 1; x++)
            black = black || pixel_rgb(picture, x, y) == 0u;
        marked += black;
      }
    }
    int black = 0, stray = 0;
    for (int y = 0; drawn && y < height; y++)
      for (int x = 0; x < width; x++)
        if (pixel_rgb(picture, x, y) == 0u) {
          black++;
          stray += around[static_cast<size_t>(y) * width + x] == 0;
        }
    check(drawn && terrain_black == 0 && stats.contour_rez > 0 && middles > 200 && marked == middles && stray == 0 &&
              stats.thin_draws == (ratio == 1 ? 1 : 0) && stats.wide_draws == (ratio == 2 ? 1 : 0),
          "6c Metal: contours at ratio " + std::to_string(ratio) + " (level " + std::to_string(stats.contour_rez) +
              ", " + std::to_string(n / 2) + " segments): black near the middle of " + std::to_string(marked) +
              " of " + std::to_string(middles) + " segments of 3 px or more, " + std::to_string(stray) + " of " +
              std::to_string(black) + " black px beyond 2.5 px of every segment (" + std::to_string(terrain_black) +
              " without contours), " + std::to_string(stats.thin_draws) + " thin and " +
              std::to_string(stats.wide_draws) + " wide draws");
  }
  if (opened)
    close_instance(instance);
}

/* The marker meshes the renderer keeps (Null): beyond 32 (kMeshes of the
   renderer) those not drawn in the last frame go, the least recently drawn
   first. 41 frames each draw list 4 unchanged and list 3 at a new radius, the
   last one at the first frame's radius again; then a frame of 40 new shapes,
   drawn twice. The meshes held: the batch's buffers less its corners and its
   instances. */
void null_meshes() {
  constexpr int kHeld = 32 + 1;
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
  mbvq::RenderOptions options;
  const auto held = [&renderer]() { return renderer.stats().batch_resources - 2; };
  std::vector<int> kept, made;
  bool drawn = true;
  for (int f = 0; f <= 40; f++) {
    TestBatch t;
    const double radius = f < 40 ? 0.01 + 0.001 * f : 0.01;
    mbv_draw_batch_marker_disks(&t.b, 3, radius, 0.8 * radius, 10, 10, 0, 0, 0);
    mbv_draw_batch_marker_disks(&t.b, 4, 0.05, 0.04, 10, 10, 0, 0, 0);
    mbv_draw_batch_color3f(&t.b, 1, 0, 0);
    mbv_draw_batch_marker(&t.b, 3, 0.0f, 0.0f, 0.0f);
    mbv_draw_batch_color3f(&t.b, 0, 1, 0);
    mbv_draw_batch_marker(&t.b, 4, 0.5f, 0.0f, 0.0f);
    drawn = drawn && renderer.render_batch(&t.b, kIdentity, kIdentity, 200, 200, options, nullptr);
    kept.push_back(held());
    made.push_back(renderer.stats().meshes);
  }
  /* until frame 30 every mesh stays; from frame 31 on 33 are held: the 32
     and the one the frame made */
  bool bounded = drawn && kept[40] == kHeld;
  bool once = drawn;
  for (int f = 0; f < 40; f++) {
    bounded = bounded && kept[f] == std::min(f + 2, kHeld);
    once = once && made[f] == f + 2;
  }
  check(bounded && once && made[40] == 42,
        "6c: the marker meshes held stay at " + std::to_string(kHeld) + " (" + std::to_string(kept[0]) + ", " +
            std::to_string(kept[30]) + ", " + std::to_string(kept[31]) + ", " + std::to_string(kept[39]) + ", " +
            std::to_string(kept[40]) + " after frames 0, 30, 31, 39, 40) with a new radius in each frame; the shape "
            "drawn in every frame is never made again (" + std::to_string(made[39]) + " made in 40 frames), and the "
            "first frame's, evicted, is made again when it comes back (" + std::to_string(made[40]) + ")");

  /* meshes drawn in the last frame stay, beyond 32 as well (shapes unlike
     those above: the meshes are cached by the shape, not by the list) */
  {
    TestBatch t;
    for (int k = 0; k < 40; k++) {
      mbv_draw_batch_marker_disks(&t.b, 10 + k, 0.1 + 0.001 * k, 0.05, 10, 10, 0, 0, 0);
      mbv_draw_batch_color3f(&t.b, 1, 0, 0);
      mbv_draw_batch_marker(&t.b, 10 + k, 0.0f, 0.0f, 0.0f);
    }
    const bool first = renderer.render_batch(&t.b, kIdentity, kIdentity, 200, 200, options, nullptr);
    const int made_first = renderer.stats().meshes;
    const bool again = renderer.render_batch(&t.b, kIdentity, kIdentity, 200, 200, options, nullptr);
    check(drawn && first && again && made_first == made[40] + 40 && renderer.stats().meshes == made_first &&
              held() == 40,
          "6c: the meshes drawn in the last frame are kept, beyond 32 as well: a frame of 40 new shapes makes " +
              std::to_string(made_first - made[40]) + ", the same frame again " +
              std::to_string(renderer.stats().meshes - made_first) + ", and then holds those 40 alone (" +
              std::to_string(held()) + ")");
  }
}

/*--------------------------------------------------------------------*/
/* 6d: the profile and 3D soundings views */

/* Synthetic soundings, as mbview_qt_soundings_test makes them: npings pings
   along y of nbeams beams across x, 2 m apart, on a sloping, rippled
   seafloor; every flag_every-th sounding flagged by hand (0: none). The
   calling program's side, as mbeditviz. */
std::vector<struct mb3dsoundings_sounding_struct> sdg_storage;
struct mb3dsoundings_struct sdg_data;
int sdg_edits = 0;
/* the program's dismiss notification: where a test asks for it, it releases
   the soundings there, as mbeditviz does (the array stays in sdg_storage
   until the test frees it) */
bool sdg_release_on_dismiss = false;
void sdg_dismiss() {
  if (sdg_release_on_dismiss) {
    sdg_data.num_soundings = 0;
    sdg_data.soundings = nullptr;
  }
}
void sdg_edit(int, int, int, char, int flush) {
  if (flush != MB3DSDG_EDIT_FLUSHPREVIOUS)
    sdg_edits++;
}

void make_sdg(int npings, int nbeams, int flag_every) {
  sdg_storage.assign(static_cast<size_t>(npings) * nbeams, {});
  const double dx = 2.0, dy = 2.0;
  const double xhalf = 0.5 * dx * (nbeams - 1), yhalf = 0.5 * dy * (npings - 1);
  int flagged = 0;
  for (int iping = 0; iping < npings; iping++)
    for (int ibeam = 0; ibeam < nbeams; ibeam++) {
      const int k = iping * nbeams + ibeam;
      struct mb3dsoundings_sounding_struct &s = sdg_storage[static_cast<size_t>(k)];
      s.ifile = 0;
      s.iping = iping;
      s.iprofile = iping;
      s.ibeam = ibeam;
      s.beamcolor = MBV_COLOR_BLACK;
      s.x = -xhalf + dx * ibeam;
      s.y = -yhalf + dy * iping;
      s.z = -1000.0 + 0.05 * s.x + 3.0 * std::sin(s.y / 15.0);
      s.a = 50.0 + 10.0 * std::cos(s.x / 20.0);
      const bool flag = flag_every > 0 && k % flag_every == 0;
      s.beamflag = flag ? static_cast<char>(MB_FLAG_FLAG | MB_FLAG_MANUAL) : static_cast<char>(MB_FLAG_NONE);
      s.beamflagorg = s.beamflag;
      flagged += flag ? 1 : 0;
    }
  std::memset(&sdg_data, 0, sizeof(sdg_data));
  sdg_data.xmin = -xhalf;
  sdg_data.xmax = xhalf;
  sdg_data.ymin = -yhalf;
  sdg_data.ymax = yhalf;
  sdg_data.bearing = 90.0;
  sdg_data.sinbearing = 1.0;
  sdg_data.cosbearing = 0.0;
  const double dxy = std::sqrt(4.0 * xhalf * xhalf + 4.0 * yhalf * yhalf);
  sdg_data.scale = 2.0 / dxy;
  sdg_data.zscale = sdg_data.scale;
  sdg_data.num_soundings = static_cast<int>(sdg_storage.size());
  sdg_data.num_soundings_alloc = sdg_data.num_soundings;
  sdg_data.num_soundings_flagged = flagged;
  sdg_data.num_soundings_unflagged = sdg_data.num_soundings - flagged;
  sdg_data.soundings = sdg_storage.data();
}

/* the soundings put into an order in place (order[k] is the index of the
   sounding that goes to place k), along the permutation's cycles, so that
   no second copy of the soundings is made; the order is used up */
void permute_sdg(std::vector<struct mb3dsoundings_sounding_struct> *soundings, std::vector<size_t> *order) {
  std::vector<size_t> &from = *order;
  for (size_t k = 0; k < from.size(); k++) {
    if (from[k] == k)
      continue;
    const struct mb3dsoundings_sounding_struct held = (*soundings)[k];
    size_t at = k;
    while (from[at] != k) {
      const size_t next = from[at];
      (*soundings)[at] = (*soundings)[next];
      from[at] = at;
      at = next;
    }
    (*soundings)[at] = held;
    from[at] = at;
  }
}

/* The pings of make_sdg() split into files that overlap in space, as
   several lines over one area (the soundings bench's --files): file f holds
   the pings p with p % files == f, the array runs file by file, and the
   pings are numbered from 0 in each file, as mbeditviz numbers them. The
   soundings are moved in place (no second copy); of their fields, only the
   file and ping numbers change. iprofile stays make_sdg()'s ping number,
   unlike mbeditviz (0 or 1 in a ping), so no profile pair can join two
   files here: a test of pairs across a file boundary needs its own
   numbers (make_sdg_three_files). One file leaves the soundings as they
   are */
void split_sdg_files(int npings, int nbeams, int files) {
  if (files <= 1)
    return;
  std::vector<size_t> order;
  order.reserve(sdg_storage.size());
  for (int file = 0; file < files; file++)
    for (int iping = file; iping < npings; iping += files)
      for (int ibeam = 0; ibeam < nbeams; ibeam++) {
        const size_t k = static_cast<size_t>(iping) * nbeams + ibeam;
        sdg_storage[k].ifile = file;
        sdg_storage[k].iping = iping / files;
        order.push_back(k);
      }
  permute_sdg(&sdg_storage, &order);
  sdg_data.soundings = sdg_storage.data();
}

/* the row of the array that holds make_sdg()'s ping after
   split_sdg_files() into files: the pings of the files before its own,
   (npings - f + files - 1) / files in file f, then its place in its file;
   the ping itself with one file */
size_t sdg_ping_row(int npings, int files, int ping) {
  if (files <= 1)
    return static_cast<size_t>(ping);
  size_t row = static_cast<size_t>(ping / files);
  for (int file = 0; file < ping % files; file++)
    row += static_cast<size_t>((npings - file + files - 1) / files);
  return row;
}

/* 7b (F12): soundings from three files, as mbeditviz selects them from
   three lines: make_sdg()'s pings, pings_per_file to a file, each ping
   numbered by its place in its file (mbeditviz keeps that number:
   mbev_files[ifile].pings[iping]), the files' parts chosen so that a
   file's first ping has the number of the last ping of the file before (0
   to 3, 3 to 6 and 6 to 9 for four pings to a file), and one profile to a
   ping (iprofile 0, as mbeditviz numbers a ping without a dual profile).
   Two consecutive soundings across a file boundary then differ in their
   file alone, which only the "same file" part of the rule of the profile
   lines tells apart (mb3dsoundings_plot, mbvq_c_soundings_profile_pairs).
   At the first boundary both soundings are good, at the second the first
   sounding of the third file is flagged: without that part the first
   would give a black pair in the Good and All modes, the second a red one
   in the All mode. */
void make_sdg_three_files(int pings_per_file, int nbeams, int flag_every) {
  make_sdg(3 * pings_per_file, nbeams, flag_every);
  for (size_t k = 0; k < sdg_storage.size(); k++) {
    struct mb3dsoundings_sounding_struct &s = sdg_storage[k];
    const int ping = static_cast<int>(k) / nbeams;
    s.ifile = ping / pings_per_file;
    s.iping = ping - s.ifile;
    s.iprofile = 0;
  }
  const size_t file = static_cast<size_t>(pings_per_file) * nbeams;
  for (const size_t k : {file - 1, file, 2 * file - 1})
    sdg_storage[k].beamflag = static_cast<char>(MB_FLAG_NONE);
  sdg_storage[2 * file].beamflag = static_cast<char>(MB_FLAG_FLAG | MB_FLAG_MANUAL);
  int flagged = 0;
  for (struct mb3dsoundings_sounding_struct &s : sdg_storage) {
    s.beamflagorg = s.beamflag;
    flagged += mb_beam_ok(s.beamflag) ? 0 : 1;
  }
  sdg_data.num_soundings_flagged = flagged;
  sdg_data.num_soundings_unflagged = sdg_data.num_soundings - flagged;
}

/* the consecutive soundings of the array that differ in their file alone
   (the same ping and profile numbers): how many, and of them how many with
   both soundings good and how many with one flagged */
int sdg_file_boundaries(int *both_good, int *one_flagged) {
  int alike = 0;
  *both_good = 0;
  *one_flagged = 0;
  for (size_t k = 0; k + 1 < sdg_storage.size(); k++) {
    const struct mb3dsoundings_sounding_struct &a = sdg_storage[k];
    const struct mb3dsoundings_sounding_struct &b = sdg_storage[k + 1];
    if (a.ifile == b.ifile || a.iping != b.iping || a.iprofile != b.iprofile)
      continue;
    alike++;
    const int good = (mb_beam_ok(a.beamflag) ? 1 : 0) + (mb_beam_ok(b.beamflag) ? 1 : 0);
    *both_good += good == 2 ? 1 : 0;
    *one_flagged += good == 1 ? 1 : 0;
  }
  return alike;
}

/* 7a: the soundings in a spatially coherent order, to compare with ping
   order: the x and y bounds cut into 64 x 64 tiles, the soundings sorted by
   tile in row-major order (a counting sort), then shuffled inside each tile
   from a fixed seed (std::mt19937 with 12345; a Fisher-Yates shuffle of its
   own, so that the order is the same with every C++ library), so that a
   prefix of a tile is an even sample of it. The soundings change places
   only (in place; the tiles and the order take 12 bytes per sounding). */
void sdg_tile_order(std::vector<struct mb3dsoundings_sounding_struct> *soundings) {
  const int bins = 64;
  const size_t n = soundings->size();
  if (n == 0)
    return;
  double lo[2] = {1.0e30, 1.0e30}, hi[2] = {-1.0e30, -1.0e30};
  for (const struct mb3dsoundings_sounding_struct &s : *soundings) {
    lo[0] = std::min(lo[0], s.x);
    hi[0] = std::max(hi[0], s.x);
    lo[1] = std::min(lo[1], s.y);
    hi[1] = std::max(hi[1], s.y);
  }
  const double sx = bins / std::max(1.0e-6, hi[0] - lo[0]) * 0.999999;
  const double sy = bins / std::max(1.0e-6, hi[1] - lo[1]) * 0.999999;
  std::vector<unsigned int> tile(n);
  std::vector<size_t> start(static_cast<size_t>(bins) * bins + 1, 0);
  for (size_t k = 0; k < n; k++) {
    const struct mb3dsoundings_sounding_struct &s = (*soundings)[k];
    const int bx = std::min(bins - 1, static_cast<int>((s.x - lo[0]) * sx));
    const int by = std::min(bins - 1, static_cast<int>((s.y - lo[1]) * sy));
    tile[k] = static_cast<unsigned int>(by * bins + bx);
    start[tile[k] + 1]++;
  }
  for (size_t b = 1; b < start.size(); b++)
    start[b] += start[b - 1];
  std::vector<size_t> order(n);
  std::vector<size_t> fill(start.begin(), start.end() - 1);
  for (size_t k = 0; k < n; k++)
    order[fill[tile[k]]++] = k;
  std::mt19937 generator(12345);
  for (size_t b = 0; b + 1 < start.size(); b++)
    for (size_t i = start[b + 1] - start[b]; i > 1; i--)
      std::swap(order[start[b] + i - 1], order[start[b] + generator() % i]);
  permute_sdg(soundings, &order);
}

/* hand the soundings to the view, as mbeditviz does: the soundings window
   (with its QRhi view) is made the first time */
bool open_sdg() {
  static bool started = false;
  int error = MB_ERROR_NO_ERROR;
  if (!started) {
    mb3dsoundings_startup(0, nullptr, nullptr, &error);
    started = true;
  }
  mb3dsoundings_open(0, &sdg_data, &error);
  mb3dsoundings_set_dismiss_notify(0, &sdg_dismiss, &error);
  mb3dsoundings_set_edit_notify(0, &sdg_edit, &error);
  return mbvq::soundings_widget() != nullptr;
}

/* the camera and what the view shows, as the soundings window sets them */
void sdg_camera(float elevation, float azimuth, float size, float offx, float offy) {
  mb3dsoundings.elevation = elevation;
  mb3dsoundings.azimuth = azimuth;
  mb3dsoundings.gl_size = size;
  mb3dsoundings.gl_offset_x = offx;
  mb3dsoundings.gl_offset_y = offy;
}
void sdg_view(int color, bool flagged, int profiles, bool box) {
  mb3dsoundings.view_color = color;
  mb3dsoundings.view_flagged = flagged;
  mb3dsoundings.view_secondary = false;
  mb3dsoundings.view_profiles = profiles;
  mb3dsoundings.view_boundingbox = box;
  mb3dsoundings.edit_mode = MBS_EDIT_TOGGLE;
  mb3dsoundings.last_sounding_defined = false;
  mb3dsoundings.button1down = false;
  mb3dsoundings.grab_start_defined = false;
  mb3dsoundings.grab_end_defined = false;
  /* the colours changed (what the view switches raise) */
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_colors_changed(0, &error);
}

/* the pairs of a soundings batch's LINES command, in their order: the first
   sounding of each pair (found by its position) and whether it is red */
struct BatchPair {
  int first;
  bool red;
  bool found;
};
std::vector<BatchPair> batch_pairs(const struct mbv_draw_batch &batch, const std::vector<float> &xyz) {
  std::vector<BatchPair> pairs;
  for (int c = 0; c < batch.num_commands; c++) {
    const struct mbv_draw_command &command = batch.commands[c];
    if (command.kind != MBV_DRAW_PRIMITIVE || command.primitive != MBV_DRAW_LINES)
      continue;
    int at = 0;
    for (int v = command.first; v + 1 < command.first + command.count; v += 2) {
      const struct mbv_draw_vertex &a = batch.vertices[v];
      const struct mbv_draw_vertex &b = batch.vertices[v + 1];
      /* the soundings are in the order of the array: look from the last one */
      BatchPair pair = {-1, a.r > 0.5f, false};
      const size_t n = xyz.size() / 3;
      for (size_t i = static_cast<size_t>(at); i + 1 < n; i++)
        if (xyz[3 * i] == a.x && xyz[3 * i + 1] == a.y && xyz[3 * i + 2] == a.z && xyz[3 * i + 3] == b.x &&
            xyz[3 * i + 4] == b.y && xyz[3 * i + 5] == b.z) {
          pair.first = static_cast<int>(i);
          pair.found = a.r == b.r && a.g == 0.0f && a.b == 0.0f && (a.r == 0.0f || a.r == 1.0f);
          at = static_cast<int>(i) + 1;
          break;
        }
      pairs.push_back(pair);
    }
  }
  return pairs;
}

/* the pairs of the two lists of mbvq_c_soundings_profile_pairs merged in the
   order of the array, as a LINES command of the profile lines has them */
std::vector<BatchPair> list_pairs(const std::vector<uint32_t> &black, const std::vector<uint32_t> &red) {
  std::vector<BatchPair> pairs;
  size_t b = 0, r = 0;
  while (b + 1 < black.size() || r + 1 < red.size()) {
    const bool take_red = b + 1 >= black.size() || (r + 1 < red.size() && red[r] < black[b]);
    const std::vector<uint32_t> &list = take_red ? red : black;
    size_t &k = take_red ? r : b;
    pairs.push_back({static_cast<int>(list[k]), take_red, list[k + 1] == list[k] + 1});
    k += 2;
  }
  return pairs;
}

bool same_pairs(const std::vector<BatchPair> &a, const std::vector<BatchPair> &b) {
  if (a.size() != b.size())
    return false;
  for (size_t k = 0; k < a.size(); k++)
    if (a[k].first != b[k].first || a[k].red != b[k].red || !a[k].found || !b[k].found)
      return false;
  return true;
}

/* 7b: the drawing of the 3D soundings recorded into a batch of the test
   with the profile mode as set, as the legacy OpenGL backend draws it
   (mbvq_c_soundings_plot) and mbview_qt_soundings_test records it: the
   reference for a frame (SoundingsFrame), whose fill records the drawing
   with the mode hidden from the core (mbvq_c_soundings_plot_decorations).
   The grab rectangle is placed by the size of the drawing that a frame's
   begin() gives the core: where it is drawn, the reference is filled after
   the frame's begin(). */
void fill_reference(struct mbv_draw_batch *batch) {
  mbv_draw_batch_clear(batch);
  mbv_draw_set_target(batch);
  mbvq_c_soundings_plot();
  mbv_draw_set_target(nullptr);
}

/* the vertices of a batch's LINES commands, and the commands (the profile
   lines of a drawing of the 3D soundings: one command, two vertices per
   pair) */
int line_vertices(const struct mbv_draw_batch &batch, int *commands = nullptr) {
  int vertices = 0;
  int n = 0;
  for (int c = 0; c < batch.num_commands; c++)
    if (batch.commands[c].kind == MBV_DRAW_PRIMITIVE && batch.commands[c].primitive == MBV_DRAW_LINES) {
      vertices += batch.commands[c].count;
      n++;
    }
  if (commands != nullptr)
    *commands = n;
  return vertices;
}

/* the state of two commands alike: line width, point size, stipple (on,
   factor, pattern), depth test and camera */
bool same_state(const struct mbv_draw_command &p, const struct mbv_draw_command &q) {
  return p.line_width == q.line_width && p.point_size == q.point_size && p.stipple == q.stipple &&
         p.stipple_factor == q.stipple_factor && p.stipple_pattern == q.stipple_pattern &&
         p.depth_test == q.depth_test && p.camera == q.camera;
}

/* two commands of two batches alike: kind, primitive, count, state and, for
   a primitive, its vertices, positions and colours (first aside, which
   counts in each batch's own vertices) */
bool same_command(const struct mbv_draw_batch &a, const struct mbv_draw_command &p, const struct mbv_draw_batch &b,
                  const struct mbv_draw_command &q) {
  if (p.kind != q.kind || p.primitive != q.primitive || p.count != q.count || !same_state(p, q))
    return false;
  for (int v = 0; p.kind == MBV_DRAW_PRIMITIVE && v < p.count; v++) {
    const struct mbv_draw_vertex &s = a.vertices[p.first + v];
    const struct mbv_draw_vertex &t = b.vertices[q.first + v];
    if (s.x != t.x || s.y != t.y || s.z != t.z || s.r != t.r || s.g != t.g || s.b != t.b)
      return false;
  }
  return true;
}

/* 7b: what differs between a frame's drawing of the 3D soundings and the
   reference of the same state (fill_reference), or "". The frame's batch,
   which the core records with the profile mode hidden, is the reference
   without its LINES command (the profile lines, its only LINES command,
   right before the soundings slot). The decorations are the reference with
   that command replaced by the slot kSlotProfileLines without vertices, at
   its place and with its state; where the reference has no LINES command
   in the Good or All mode (no pair to draw), the slot stands right before
   the soundings slot with the state that command would have there (the
   soundings slot's, with the lines' width of 1 and the point size of 1
   from before the soundings set theirs); the None mode has no slot. */
std::string drawing_against(const struct mbv_draw_batch &batch, const struct mbv_draw_batch &decorations,
                            const struct mbv_draw_batch &reference, int mode) {
  const auto lines = [](const struct mbv_draw_command &c) {
    return c.kind == MBV_DRAW_PRIMITIVE && c.primitive == MBV_DRAW_LINES;
  };
  /* the frame's batch: the reference without its LINES command */
  int b = 0;
  for (int r = 0; r < reference.num_commands; r++) {
    if (lines(reference.commands[r]))
      continue;
    if (b >= batch.num_commands || !same_command(batch, batch.commands[b], reference, reference.commands[r]))
      return "the frame's batch differs at its command " + std::to_string(b);
    b++;
  }
  if (b != batch.num_commands)
    return "the frame's batch has " + std::to_string(batch.num_commands - b) + " commands more";
  /* the decorations: the reference with the slot for its LINES command */
  int r = 0;
  int profile_slots = 0;
  for (int c = 0; c < decorations.num_commands; c++) {
    const struct mbv_draw_command &d = decorations.commands[c];
    const struct mbv_draw_command *at = r < reference.num_commands ? &reference.commands[r] : nullptr;
    if (d.kind == MBV_DRAW_SLOT && d.primitive == rhi::kSlotProfileLines) {
      profile_slots++;
      if (mode == MBS_VIEW_PROFILES_NONE || profile_slots > 1 || d.count != 0 || at == nullptr)
        return "a profile slot in the None mode, a second one, one with vertices or one after the reference's end";
      struct mbv_draw_command expected = *at;
      if (lines(*at))
        r++;
      else if (at->kind == MBV_DRAW_SLOT && at->primitive == MBV_DRAW_SLOT_SOUNDINGS) {
        expected.line_width = 1.0f;
        expected.point_size = 1.0f;
      }
      else
        return "the profile slot (command " + std::to_string(c) +
               ") is neither at the LINES command's place nor right before the soundings slot";
      if (!same_state(d, expected))
        return "the profile slot's state differs from the LINES command's (command " + std::to_string(c) + ")";
      continue;
    }
    if (at == nullptr)
      return "the decorations have commands after the reference's end";
    if (lines(*at))
      return "no profile slot at the LINES command's place (the decorations' command " + std::to_string(c) + ")";
    if (!same_command(decorations, d, reference, *at))
      return "the decorations differ at their command " + std::to_string(c);
    r++;
  }
  if (r != reference.num_commands)
    return "the decorations lack the reference's last " + std::to_string(reference.num_commands - r) + " commands";
  if (mode != MBS_VIEW_PROFILES_NONE && profile_slots == 0)
    return "no profile slot in the Good or All mode";
  return std::string();
}

/* the soundings plan on its own (no QRhi): layout, the first upload in
   chunk order within the budget, the prefix, double buffering, streams
   written whole, busy streams, the pair across chunks */
void model_soundings_plan() {
  rhi::SoundingsPlan plan;
  plan.reset(10, 4, 1000);
  check(plan.chunks() == 3 && plan.points(0) == 4 && plan.points(2) == 2 && plan.positions(0) == 5 &&
            plan.positions(1) == 5 && plan.positions(2) == 2 && plan.pairs(0) == 4 && plan.pairs(1) == 4 &&
            plan.pairs(2) == 1 && plan.bytes(rhi::SoundingsPlan::kPositions, 0) == 60 &&
            plan.bytes(rhi::SoundingsPlan::kColours, 2) == 8 && plan.bytes(rhi::SoundingsPlan::kPairColours, 2) == 4,
        "6d: the soundings in chunks: 10 soundings in chunks of 4 hold 4, 4, 2 points, 5, 5, 2 positions (one into "
        "the next chunk) and 4, 4, 1 pairs");

  /* the first upload: chunk by chunk, positions and colours of a chunk
     before the next, within the budget (76 bytes of chunk 0 in 80) */
  plan.reset(10, 4, 80);
  plan.start(rhi::SoundingsPlan::kPositions);
  plan.start(rhi::SoundingsPlan::kColours);
  const bool second_set = plan.double_buffered(rhi::SoundingsPlan::kPositions) &&
                          !plan.double_buffered(rhi::SoundingsPlan::kColours);
  std::vector<int> shown;
  std::string order;
  bool whole_frames = true;
  for (int f = 0; f < 6 && plan.waiting(); f++) {
    size_t bytes = 0;
    const std::vector<rhi::SoundingsPlan::Upload> uploads = plan.frame();
    for (const rhi::SoundingsPlan::Upload &upload : uploads) {
      order += (upload.stream == rhi::SoundingsPlan::kPositions ? "P" : "C") + std::to_string(upload.chunk) + " ";
      bytes += upload.bytes;
      whole_frames = whole_frames && upload.set == 0;
    }
    order += "| ";
    whole_frames = whole_frames && (bytes <= 80 || uploads.size() == 1);
    shown.push_back(plan.soundings_drawn());
  }
  check(second_set && whole_frames && order == "P0 C0 | P1 C1 | P2 C2 | " && shown == std::vector<int>({4, 8, 10}) &&
            plan.last_pair_out(0) == false,
        "6d: the first upload writes chunk by chunk within the budget (" + order + "), drawing its prefix " +
            list(shown) + "; positions above the budget get a second set, colours below it do not");

  /* the prefix leaves out the pair that reaches into a chunk not drawn */
  plan.reset(10, 4, 76);
  plan.start(rhi::SoundingsPlan::kPositions);
  plan.start(rhi::SoundingsPlan::kColours);
  plan.frame();
  const bool out0 = plan.drawn(0) && !plan.drawn(1) && plan.last_pair_out(0);
  plan.frame();
  plan.frame();
  check(out0 && !plan.last_pair_out(0) && !plan.last_pair_out(2),
        "6d: during the first upload a drawn chunk's pair into the next chunk is left out until that chunk is drawn");

  /* later: positions above the budget go into the second set over frames,
     while the first set is drawn; a start while busy changes nothing; then
     the sets swap */
  const int front = plan.front(rhi::SoundingsPlan::kPositions);
  plan.start(rhi::SoundingsPlan::kPositions);
  std::vector<rhi::SoundingsPlan::Upload> uploads = plan.frame();
  const bool back = uploads.size() == 1 && uploads[0].set == 1 - front && plan.soundings_drawn() == 10 &&
                    plan.front(rhi::SoundingsPlan::kPositions) == front && plan.busy(rhi::SoundingsPlan::kPositions);
  const int writes = plan.writes(rhi::SoundingsPlan::kPositions);
  plan.start(rhi::SoundingsPlan::kPositions);
  uploads = plan.frame();
  const bool continued = uploads.size() == 1 && uploads[0].chunk == 1 && plan.swaps() == 0;
  uploads = plan.frame();
  check(back && continued && uploads.size() == 1 && uploads[0].chunk == 2 && plan.swaps() == 1 &&
            plan.front(rhi::SoundingsPlan::kPositions) == 1 - front && !plan.busy(rhi::SoundingsPlan::kPositions) &&
            plan.writes(rhi::SoundingsPlan::kPositions) == writes + 2 && plan.soundings_drawn() == 10,
        "6d: positions above the budget are written into the second set over frames while the first is drawn; a "
        "start while the upload runs does not restart it; then the sets swap");

  /* colours below the budget, after the first upload: whole in one frame */
  plan.start(rhi::SoundingsPlan::kColours);
  uploads = plan.frame();
  check(uploads.size() == 3 && uploads[0].set == plan.front(rhi::SoundingsPlan::kColours) && !plan.waiting(),
        "6d: a stream below the budget is written whole within one frame, into the drawn set");

  /* the index lists: chunk by chunk, whatever their size (an empty chunk
     costs nothing) */
  plan.set_list_bytes({40, 200, 0});
  plan.start(rhi::SoundingsPlan::kLists);
  uploads = plan.frame();
  const bool first_lists = uploads.size() == 1 && uploads[0].chunk == 0 && !plan.held(rhi::SoundingsPlan::kLists, 1);
  uploads = plan.frame();
  check(first_lists && uploads.size() == 2 && uploads[0].bytes == 200 && uploads[1].bytes == 0 && !plan.waiting() &&
            plan.held(rhi::SoundingsPlan::kLists, 2) && plan.lines(rhi::SoundingsPlan::kLists, 1),
        "6d: the index lists go chunk by chunk, a chunk above the budget alone in its frame (an empty one beside it)");
}

/* the frame driver of the 3D soundings view on the core: the decorations,
   the pairs of the profile lines against the LINES command of a fill with
   the profile mode set (fill_reference; the frame's own fill records the
   drawing with the mode hidden from the core, 7b) in the three profile
   modes, the lists made local to chunks and the colours per pair, the
   camera in every frame, and nothing read without soundings */
void model_soundings_frame() {
  make_sdg(30, 40, 7);
  check(open_sdg(), "6d: the soundings handed over (a soundings window with a QRhi view)");
  QWidget *view = mbvq::soundings_widget() != nullptr ? mbvq::soundings_widget()->view() : nullptr;
  check(view != nullptr && mbvq::view_state(view).rhi_view, "6d: the soundings view is a QRhi view");
  sdg_camera(35.0f, 125.0f, 1.3f, 0.05f, -0.05f);

  rhi::SoundingsFrame frame;
  TestBatch reference;
  bool modes = true;
  std::string counts;
  for (int mode : {MBS_VIEW_PROFILES_NONE, MBS_VIEW_PROFILES_UNFLAGGED, MBS_VIEW_PROFILES_ALL}) {
    sdg_view(MBS_VIEW_COLOR_FLAG, true, mode, true);
    rhi::SoundingsInput input;
    const bool filled = frame.begin(false, 400, 300, &input);
    fill_reference(&reference.b);
    std::vector<float> xyz;
    std::vector<uint32_t> black, red;
    const bool read = frame.positions(input.count, &xyz) && frame.pairs(input.count, &black, &red);
    const std::vector<BatchPair> from_batch = batch_pairs(reference.b, xyz);
    const std::vector<BatchPair> from_lists = list_pairs(black, red);
    /* the decorations: the reference's commands, the profile lines a slot without vertices */
    const struct mbv_draw_batch &d = frame.decorations();
    int profile_slots = 0, lines = 0;
    for (int c = 0; c < d.num_commands; c++) {
      profile_slots += d.commands[c].kind == MBV_DRAW_SLOT && d.commands[c].primitive == rhi::kSlotProfileLines &&
               d.commands[c].count == 0;
      lines += d.commands[c].kind == MBV_DRAW_PRIMITIVE && d.commands[c].primitive == MBV_DRAW_LINES;
    }
    const int vertices = reference.b.num_vertices - d.num_vertices;
    modes = modes && filled && read && input.profiles == mode && same_pairs(from_batch, from_lists) &&
            d.num_commands == reference.b.num_commands && lines == 0 &&
            profile_slots == (mode == MBS_VIEW_PROFILES_NONE ? 0 : 1) &&
            vertices == 2 * static_cast<int>(from_batch.size()) &&
            (mode == MBS_VIEW_PROFILES_NONE) == from_lists.empty() &&
            (mode != MBS_VIEW_PROFILES_UNFLAGGED || red.empty()) && (mode != MBS_VIEW_PROFILES_ALL || !red.empty());
    counts += std::to_string(black.size() / 2) + "+" + std::to_string(red.size() / 2) + " ";
  }
  check(modes, "6d: the pairs of the profile lines (black+red: " + counts +
                   "in the None, Good and All modes) equal the LINES command of a fill with the mode set, pairs and "
                   "colours, with the mode read from mbvq_c_soundings_state(); the decorations leave out its vertices");

  /* the lists made local to chunks of 7 and of 11 soundings, and the colours
     per pair (every 7th sounding is flagged: with chunks of 7 every pair
     across chunks is red, with chunks of 11 black ones cross too) */
  {
    rhi::SoundingsInput input;
    frame.begin(false, 400, 300, &input);
    bool local = true, coloured = true, inner = true;
    int across[2] = {0, 0};
    for (const int size : {7, 11}) {
      std::vector<uint32_t> black, red;
      frame.pairs(input.count, &black, &red);
      const std::vector<uint32_t> black_global = black, red_global = red;
      rhi::SoundingsPlan plan;
      plan.reset(input.count, size);
      std::vector<size_t> black_range, red_range;
      rhi::split_pairs(plan, &black, &black_range);
      rhi::split_pairs(plan, &red, &red_range);
      std::vector<unsigned char> colours;
      rhi::pair_colours(plan, black, black_range, red, red_range, &colours);
      local = local && black_range.size() == static_cast<size_t>(plan.chunks()) + 1 &&
              black_range.back() == black.size();
      for (int c = 0; c < plan.chunks() && local; c++)
        for (const auto &[list, global, range] :
             {std::tie(black, black_global, black_range), std::tie(red, red_global, red_range)})
          for (size_t k = range[static_cast<size_t>(c)]; k < range[static_cast<size_t>(c) + 1]; k++)
            local = local && list[k] + static_cast<uint32_t>(plan.first(c)) == global[k] &&
                    list[k] <= static_cast<uint32_t>(plan.points(c)) &&
                    (k % 2 == 1 || global[k] / static_cast<uint32_t>(size) == static_cast<uint32_t>(c));
      /* each pair (i, i + 1): black, red or none, as the lists say */
      coloured = coloured && colours.size() == 4 * static_cast<size_t>(input.count - 1);
      std::vector<int> kind(static_cast<size_t>(std::max(0, input.count - 1)), 0);
      for (size_t k = 0; k + 1 < black_global.size(); k += 2)
        kind[black_global[k]] = 1;
      for (size_t k = 0; k + 1 < red_global.size(); k += 2)
        kind[red_global[k]] = 2;
      for (size_t i = 0; i < kind.size() && coloured; i++) {
        const unsigned char *rgba = &colours[4 * i];
        coloured = kind[i] == 0   ? rgba[3] == 0
                   : kind[i] == 1 ? rgba[0] == 0 && rgba[3] == 255
                                  : rgba[0] == 255 && rgba[1] == 0 && rgba[3] == 255;
      }
      /* a chunk's pair into the next chunk is the last of its list, left out
         by the inner count, black and red */
      for (int c = 0; c + 1 < plan.chunks(); c++) {
        const int last = kind[static_cast<size_t>(plan.first(c) + plan.points(c) - 1)];
        const auto leaves_out = [&](const std::vector<uint32_t> &list, const std::vector<size_t> &range, int colour) {
          const size_t n = rhi::pair_entries(range, c);
          const size_t in = rhi::inner_pair_entries(plan, list, range, c);
          across[colour - 1] += last == colour;
          return in == (last == colour ? n - 2 : n);
        };
        const bool black_ok = leaves_out(black, black_range, 1);
        const bool red_ok = leaves_out(red, red_range, 2);
        inner = inner && black_ok && red_ok;
      }
    }
    check(local && coloured && inner && across[0] > 0 && across[1] > 0,
          "6d: the lists made local to chunks of 7 and of 11 soundings keep every pair in its chunk's positions; the "
          "colours per pair equal the two lists (black, red, none); a pair across chunks (" +
              std::to_string(across[0]) + " black, " + std::to_string(across[1]) +
              " red) is the one the inner count leaves out");
  }

  /* mb3dsoundings_reset() leaves left and right swapped and raises every
     mark: the next frame fills, which sets them (mb3dsoundings_plot), with or
     without the camera. Only the precondition of the check after it, which
     tests the camera in a frame that reuses the batch. */
  {
    mb3dsoundings_reset();
    const bool swapped = mb3dsoundings.left > mb3dsoundings.right;
    rhi::SoundingsInput input;
    frame.begin(true, 400, 300, &input);
    check(swapped && mb3dsoundings.left < mb3dsoundings.right,
          "6d: the frame after mb3dsoundings_reset() fills, which sets the extents the reset leaves swapped (the "
          "precondition of the next check)");
    /* the reset also let go of the program's notify functions */
    int error = MB_ERROR_NO_ERROR;
    mb3dsoundings_set_dismiss_notify(0, &sdg_dismiss, &error);
    mb3dsoundings_set_edit_notify(0, &sdg_edit, &error);
  }
  /* a frame that reuses the batch (continued, nothing changed) takes the
     camera as well: the extents set by hand to swapped are set again (the
     reset above raised every mark, so that frame filled) */
  {
    rhi::SoundingsInput input;
    frame.begin(false, 400, 300, &input);
    const int fills = frame.fills();
    mb3dsoundings.left = 1.0f;
    mb3dsoundings.right = -1.0f;
    const bool reused = frame.begin(true, 400, 300, &input) && frame.fills() == fills;
    check(reused && mb3dsoundings.left < mb3dsoundings.right,
          "6d: a frame that reuses the batch takes the camera too, which sets the view's extents");
  }

  /* Dismiss: the program releases the soundings (count 0); the frame fills
     nothing and reads nothing, where the information highlight would read
     the released array */
  {
    mb3dsoundings.edit_mode = MBS_EDIT_INFO;
    mb3dsoundings.last_sounding_defined = true;
    mb3dsoundings.last_sounding_edited = 5;
    mbvq_c_soundings_dismiss();
    sdg_data.num_soundings = 0;
    sdg_data.soundings = nullptr;
    const int fills = frame.fills();
    rhi::SoundingsInput input;
    const bool ok = frame.begin(false, 400, 300, &input);
    check(ok && input.count == 0 && input.frame.batch == nullptr && frame.fills() == fills,
          "6d: after Dismiss (count 0) the frame neither fills nor reads, and draws nothing");
  }
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
}

/* the profile view's frame on a map whose profile has no points: it sizes
   the core, fills the batch (only the frame, one loop) and takes the camera
   of the accessor; the quads, green boundaries and frame of a profile with
   points are checked in pixels by --render-metal (metal_profile) */
void model_profile_frame() {
  Grid grid;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "6d: a map for the profile");
  rhi::MapFrame map(instance);
  rhi::FrameInput map_input;
  map.begin(false, 640, 480, &map_input);
  mbvq_c_profile_show(instance, true);
  rhi::ProfileFrame frame(instance);
  rhi::FrameInput input;
  const bool filled = frame.begin(1200, 300, &input);
  int quads = 0, lines = 0, loops = 0;
  if (input.batch != nullptr)
    for (int c = 0; c < input.batch->num_commands; c++) {
      const struct mbv_draw_command &command = input.batch->commands[c];
      quads += command.primitive == MBV_DRAW_QUADS;
      lines += command.primitive == MBV_DRAW_LINES;
      loops += command.primitive == MBV_DRAW_LINE_LOOP;
    }
  double projection[16], modelview[16];
  mbvq_c_profile_camera(instance, projection, modelview);
  double camera[16];
  rhi::multiply(projection, modelview, camera);
  check(filled && input.batch != nullptr && loops == 1 && std::equal(camera, camera + 16, input.camera) &&
            std::equal(projection, projection + 16, input.projection) && data_of(instance)->prwidth == 1200 &&
            data_of(instance)->prheight == 300,
        "6d: the profile frame sizes the core, fills the profile (" + std::to_string(quads) + " quad, " +
            std::to_string(lines) + " line, " + std::to_string(loops) + " loop commands) and takes the camera of "
            "mbvq_c_profile_camera");
  mbvq_c_profile_show(instance, false);
  close_instance(instance);
}

/* 7a: the soundings bench's files overlap in space: each file's pings span
   the area, the array runs file by file with the pings numbered from 0 in
   each file, every sounding is equal to make_sdg()'s sounding of its ping
   and beam but for its file and ping numbers, and sdg_ping_row() finds
   make_sdg()'s pings in the array; one file changes nothing. Checked on 120
   pings in 3 files and on 122 pings in 4 files (31, 31, 30 and 30 pings) */
void model_split_files(int npings, int files) {
  using Sounding = struct mb3dsoundings_sounding_struct;
  const int nbeams = 50;
  make_sdg(npings, nbeams, 7);
  const std::vector<Sounding> one = sdg_storage;
  split_sdg_files(npings, nbeams, 1);
  const auto fields = [](const Sounding &s) {
    return std::tie(s.iprofile, s.ibeam, s.beamcolor, s.beamflag, s.beamflagorg, s.x, s.y, s.z, s.a, s.glx, s.gly,
                    s.glz, s.r, s.g, s.b, s.winx, s.winy);
  };
  bool unchanged = sdg_storage.size() == one.size();
  for (size_t k = 0; unchanged && k < one.size(); k++)
    unchanged = fields(sdg_storage[k]) == fields(one[k]) && sdg_storage[k].ifile == 0 &&
                sdg_storage[k].iping == one[k].iping;
  /* make_sdg()'s ping p and beam b at row sdg_ping_row(npings, k, p) of the
     array with k files (make_sdg() numbers iprofile by ping) */
  const auto rows_at = [&](int k) {
    bool at = true;
    for (int p = 0; at && p < npings; p++)
      for (int b = 0; at && b < nbeams; b++) {
        const size_t row = sdg_ping_row(npings, k, p) * nbeams + b;
        at = row < sdg_storage.size() && sdg_storage[row].iprofile == p && sdg_storage[row].ibeam == b;
      }
    return at;
  };
  bool rows = rows_at(1);
  make_sdg(npings, nbeams, 7);
  split_sdg_files(npings, nbeams, files);
  rows = rows && rows_at(files);
  /* every sounding once: make_sdg() numbers iprofile by ping */
  bool same = sdg_storage.size() == one.size();
  std::vector<int> seen(one.size(), 0);
  for (size_t k = 0; same && k < sdg_storage.size(); k++) {
    const Sounding &s = sdg_storage[k];
    const size_t at = static_cast<size_t>(s.iprofile) * nbeams + s.ibeam;
    same = at < one.size() && seen[at]++ == 0 && fields(s) == fields(one[at]);
  }
  /* file by file, the pings of a file numbered 0, 1, ... with all their
     beams (counted from where the file begins); file f holds the pings p
     with p % files == f, (npings - f + files - 1) / files of them; each file
     spans at least 90 % of the area along the pings, which holds while
     files is at most about npings / 20 (npings / 10 where files divides
     npings); each part found on its own, so that a failing one leaves the
     others as they are */
  bool runs = true, labels = true, sizes = true, spans = true;
  std::vector<size_t> held(files, 0);
  size_t within = 0;
  for (size_t k = 0; k < sdg_storage.size(); k++) {
    const Sounding &s = sdg_storage[k];
    if (k > 0 && s.ifile != sdg_storage[k - 1].ifile)
      within = 0;
    runs = runs && s.ifile >= 0 && s.ifile < files && (k == 0 || s.ifile >= sdg_storage[k - 1].ifile) &&
           s.iping == static_cast<int>(within / nbeams) && s.ibeam == static_cast<int>(within % nbeams);
    labels = labels && s.ifile == s.iprofile % files && s.iping == s.iprofile / files;
    if (s.ifile >= 0 && s.ifile < files)
      held[s.ifile]++;
    within++;
  }
  std::string counts;
  for (int file = 0; file < files; file++) {
    double ylo = 1.0e30, yhi = -1.0e30;
    for (const Sounding &s : sdg_storage)
      if (s.ifile == file) {
        ylo = std::min(ylo, s.y);
        yhi = std::max(yhi, s.y);
      }
    spans = spans && yhi - ylo >= 0.9 * (sdg_data.ymax - sdg_data.ymin);
    sizes = sizes && held[file] == static_cast<size_t>((npings - file + files - 1) / files) * nbeams;
    counts += (file > 0 ? ", " : "") + std::to_string(held[file] / nbeams);
  }
  const std::string nfiles = std::to_string(files);
  check(unchanged && same && runs && labels && sizes && spans && rows,
        "7a: the soundings bench's --files " + nfiles + " puts the " + std::to_string(npings) +
            " pings into files of " + counts + " pings that overlap in space (each spans the area: " +
            (spans ? "yes" : "no") + "), file f holding the pings p with p % " + nfiles + " == f (" +
            (labels ? "yes" : "no") + "; the sizes that p % " + nfiles + " gives: " + (sizes ? "yes" : "no") +
            "), the array file by file with the pings numbered from 0 in each (" + (runs ? "yes" : "no") +
            "), each sounding once and unchanged but for its file and ping (" + (same ? "yes" : "no") +
            "), every ping and beam at the row sdg_ping_row() gives, with one file and with " + nfiles + " (" +
            (rows ? "yes" : "no") + "); one file leaves them as they are (" + (unchanged ? "yes" : "no") + ")");
}

/* 7a: the soundings bench's tile order hands over the soundings of ping
   order, each once and unchanged (the same count, the same multiset of
   ids, positions, colours and flags), sorted by tile and shuffled inside the
   tiles, in the same order in a second run */
void model_tile_order() {
  using Sounding = struct mb3dsoundings_sounding_struct;
  const int npings = 120, nbeams = 50;
  make_sdg(npings, nbeams, 7);
  split_sdg_files(npings, nbeams, 3);
  const std::vector<Sounding> pings = sdg_storage;
  std::vector<Sounding> tiles = pings, again = pings;
  sdg_tile_order(&tiles);
  sdg_tile_order(&again);
  const auto fields = [](const Sounding &s) {
    return std::tie(s.ifile, s.iping, s.iprofile, s.ibeam, s.beamcolor, s.beamflag, s.beamflagorg, s.x, s.y, s.z, s.a,
                    s.glx, s.gly, s.glz, s.r, s.g, s.b, s.winx, s.winy);
  };
  const auto less = [&](const Sounding &p, const Sounding &q) { return fields(p) < fields(q); };
  const auto same = [&](const Sounding &p, const Sounding &q) { return fields(p) == fields(q); };
  std::vector<Sounding> sorted_pings = pings, sorted_tiles = tiles;
  std::sort(sorted_pings.begin(), sorted_pings.end(), less);
  std::sort(sorted_tiles.begin(), sorted_tiles.end(), less);
  const bool multiset = sorted_pings.size() == sorted_tiles.size() &&
                        std::equal(sorted_pings.begin(), sorted_pings.end(), sorted_tiles.begin(), same);
  /* the tiles as the sort cuts them (64 x 64 of the x and y bounds of the
     soundings handed over, with its factor 0.999999; written out here, not
     taken from the sort) in row-major order never go back along the array,
     and inside a tile some sounding comes before one that came before it in
     the array handed over, which runs by file, ping and beam (shuffled: the
     counting sort alone keeps that order inside a tile) */
  double lo[2] = {1.0e30, 1.0e30}, hi[2] = {-1.0e30, -1.0e30};
  for (const Sounding &s : pings) {
    lo[0] = std::min(lo[0], s.x);
    hi[0] = std::max(hi[0], s.x);
    lo[1] = std::min(lo[1], s.y);
    hi[1] = std::max(hi[1], s.y);
  }
  const double sx = 64.0 / std::max(1.0e-6, hi[0] - lo[0]) * 0.999999;
  const double sy = 64.0 / std::max(1.0e-6, hi[1] - lo[1]) * 0.999999;
  const auto cell = [&](const Sounding &s) {
    const int bx = std::min(63, static_cast<int>((s.x - lo[0]) * sx));
    const int by = std::min(63, static_cast<int>((s.y - lo[1]) * sy));
    return by * 64 + bx;
  };
  bool sorted = true, shuffled = false;
  int cells = tiles.empty() ? 0 : 1;
  for (size_t k = 1; k < tiles.size(); k++) {
    const int c0 = cell(tiles[k - 1]), c1 = cell(tiles[k]);
    sorted = sorted && c1 >= c0;
    cells += c1 != c0;
    if (c1 == c0 && std::make_tuple(tiles[k].ifile, tiles[k].iping, tiles[k].ibeam) <
                        std::make_tuple(tiles[k - 1].ifile, tiles[k - 1].iping, tiles[k - 1].ibeam))
      shuffled = true;
  }
  const bool repeated = again.size() == tiles.size() && std::equal(again.begin(), again.end(), tiles.begin(), same);
  check(tiles.size() == pings.size() && multiset && sorted && shuffled && repeated,
        "7a: the soundings bench's tile order hands over the same " + std::to_string(tiles.size()) + " of " +
            std::to_string(pings.size()) + " soundings (the same multiset of ids, positions, colours and flags: " +
            (multiset ? "yes" : "no") + "), sorted by tile (" + (sorted ? "yes" : "no") + ", " +
            std::to_string(cells) + " tiles), shuffled inside them (" + (shuffled ? "yes" : "no") +
            "), the same order in a second run (" + (repeated ? "yes" : "no") + ")");
}

/* 7b: a frame's drawing of the 3D soundings, which it records with the
   profile mode hidden from the core, against the reference of the same
   state (drawing_against), in every combination of the three profile
   modes, the bounding box off and on, the four colour modes and the
   information highlight and the grab rectangle (neither, either, both):
   96 drawings, 64 of them with a LINES command. The Good and All modes
   again with the box seen from behind, where the slot must carry the
   stipple pattern that the box's last face leaves. Then the Good mode with
   every sounding flagged, so that no pair is drawn: the reference has no
   LINES command, the slot stands right before the soundings slot, the
   lists are empty. */
void model_soundings_decorations() {
  make_sdg(30, 40, 7);
  const bool opened = open_sdg();
  sdg_camera(35.0f, 125.0f, 1.3f, 0.05f, -0.05f);
  rhi::SoundingsFrame frame;
  TestBatch reference;
  const char *const modes[3] = {"None", "Good", "All"};
  const char *const colours[4] = {"flag", "topography", "amplitude", "soundings"}; /* MBS_VIEW_COLOR_* */
  int drawings = 0, equal = 0, with_lines = 0;
  std::string difference;
  for (const int mode : {MBS_VIEW_PROFILES_NONE, MBS_VIEW_PROFILES_UNFLAGGED, MBS_VIEW_PROFILES_ALL})
    for (const bool box : {false, true})
      for (const int colour : {MBS_VIEW_COLOR_FLAG, MBS_VIEW_COLOR_TOPO, MBS_VIEW_COLOR_AMP, MBS_VIEW_COLOR_SOUNDING})
        for (int extras = 0; extras < 4; extras++) {
          sdg_view(colour, true, mode, box);
          /* the information highlight: a POINTS command after the soundings slot */
          if ((extras & 1) != 0) {
            mb3dsoundings.edit_mode = MBS_EDIT_INFO;
            mb3dsoundings.last_sounding_defined = true;
            mb3dsoundings.last_sounding_edited = 517;
          }
          /* the grab rectangle: a LINE_LOOP 3 px wide without the camera, last */
          if ((extras & 2) != 0) {
            mb3dsoundings.button1down = true;
            mb3dsoundings.grab_start_defined = true;
            mb3dsoundings.grab_end_defined = true;
            mb3dsoundings.grab_start_x = 101;
            mb3dsoundings.grab_start_y = 77;
            mb3dsoundings.grab_end_x = 287;
            mb3dsoundings.grab_end_y = 203;
          }
          rhi::SoundingsInput input;
          const bool filled = frame.begin(false, 400, 300, &input);
          fill_reference(&reference.b);
          const std::string why =
              filled ? drawing_against(frame.batch(), frame.decorations(), reference.b, mode) : "the fill was refused";
          int commands = 0;
          line_vertices(reference.b, &commands);
          drawings++;
          equal += why.empty() ? 1 : 0;
          with_lines += commands;
          if (!why.empty() && difference.empty())
            difference = std::string(modes[mode]) + " mode, box " + (box ? "on" : "off") + ", colour by " +
                         colours[colour] + ", highlight " + ((extras & 1) != 0 ? "on" : "off") + ", grab " +
                         ((extras & 2) != 0 ? "on" : "off") + ": " + why;
        }

  /* the box seen from behind (azimuth 300): its last face leaves the stipple
     pattern 0x1111 on the LINES command and the soundings slot, which the
     slot of the profile lines must carry as well (at azimuth 125 every
     command keeps the pattern of the batch's initial state) */
  sdg_camera(35.0f, 300.0f, 1.3f, 0.05f, -0.05f);
  int behind = 0, patterned = 0;
  for (const int mode : {MBS_VIEW_PROFILES_UNFLAGGED, MBS_VIEW_PROFILES_ALL}) {
    sdg_view(MBS_VIEW_COLOR_FLAG, true, mode, true);
    rhi::SoundingsInput input;
    const bool filled = frame.begin(false, 400, 300, &input);
    fill_reference(&reference.b);
    const std::string why =
        filled ? drawing_against(frame.batch(), frame.decorations(), reference.b, mode) : "the fill was refused";
    behind += why.empty() ? 1 : 0;
    for (int c = 0; c < reference.b.num_commands; c++) {
      const struct mbv_draw_command &command = reference.b.commands[c];
      patterned += command.kind == MBV_DRAW_PRIMITIVE && command.primitive == MBV_DRAW_LINES && !command.stipple &&
                           command.stipple_pattern == 0x1111
                       ? 1
                       : 0;
    }
    if (!why.empty() && difference.empty())
      difference = std::string(modes[mode]) + " mode, box on, seen from behind: " + why;
  }
  sdg_camera(35.0f, 125.0f, 1.3f, 0.05f, -0.05f);

  /* no pair to draw: every sounding flagged, the Good mode */
  for (struct mb3dsoundings_sounding_struct &sounding : sdg_storage)
    sounding.beamflag = static_cast<char>(MB_FLAG_FLAG | MB_FLAG_MANUAL);
  sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_UNFLAGGED, true);
  rhi::SoundingsInput input;
  const bool filled = frame.begin(false, 400, 300, &input);
  fill_reference(&reference.b);
  int commands = 0;
  line_vertices(reference.b, &commands);
  std::vector<uint32_t> black, red;
  const std::string why =
      filled ? drawing_against(frame.batch(), frame.decorations(), reference.b, MBS_VIEW_PROFILES_UNFLAGGED)
             : "the fill was refused";
  const bool without_pairs =
      why.empty() && commands == 0 && frame.pairs(input.count, &black, &red) && black.empty() && red.empty();
  if (!why.empty() && difference.empty())
    difference = "Good mode without a pair: " + why;
  check(opened && drawings == 96 && equal == drawings && with_lines == 64 && behind == 2 && patterned == 2 &&
            without_pairs,
        "7b: the 3D soundings' drawing of a frame against a fill of the same state with the profile mode set, in " +
            std::to_string(drawings) + " drawings (the None, Good and All modes, the bounding box off and on, the "
            "four colour modes, neither, either or both of the information highlight and the grab rectangle; " +
            std::to_string(with_lines) + " with a LINES command): the frame's batch is that fill without its LINES "
            "command, the decorations are that fill with the slot of the profile lines in its place and with its "
            "state, and no slot in the None mode (" + std::to_string(equal) + " equal); also with the box seen "
            "from behind, whose last face leaves the stipple pattern 0x1111 on the LINES command, in the Good and "
            "All modes (" + std::to_string(behind) + " equal, " + std::to_string(patterned) + " LINES commands with "
            "that pattern); in the Good mode with every sounding flagged (no pair) the slot stands right before "
            "the soundings slot and the lists are empty (" + (without_pairs ? "yes" : "no") + ")" +
            (difference.empty() ? std::string() : "; first difference: " + difference));
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
}

/* 7b: with all profiles shown the core's batch of the 3D soundings, which
   a frame records with the profile mode hidden from the core, holds the
   same vertices for 10,000 and for 100,000 soundings, no LINES command,
   and keeps no memory that grows with them (it is kept until Dismiss).
   The reference (fill_reference) records the profile lines as 6e's fill
   did: two vertices for each of the 249 pairs of a ping of 250 beams, of
   which the 400 pings have 360 more than the 40. A frame of its own,
   whose batch has not grown before. */
void model_soundings_fill_size() {
  rhi::SoundingsFrame frame;
  TestBatch reference;
  const int pings[2] = {40, 400};
  int vertices[2] = {}, allocated[2] = {}, lines[2] = {}, recorded[2] = {};
  bool filled = true;
  for (int k = 0; k < 2; k++) {
    make_sdg(pings[k], 250, 7);
    filled = open_sdg() && filled;
    sdg_camera(35.0f, 125.0f, 1.3f, 0.05f, -0.05f);
    sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, true);
    rhi::SoundingsInput input;
    filled = frame.begin(false, 400, 300, &input) && input.count == 250 * pings[k] && filled;
    vertices[k] = frame.batch().num_vertices;
    allocated[k] = frame.batch().num_vertices_alloc;
    line_vertices(frame.batch(), &lines[k]);
    fill_reference(&reference.b);
    recorded[k] = reference.b.num_vertices;
  }
  check(filled && vertices[0] == vertices[1] && allocated[0] == allocated[1] && lines[0] == 0 && lines[1] == 0 &&
            recorded[1] - recorded[0] == 2 * 249 * (pings[1] - pings[0]),
        "7b: with all profiles shown the core's batch of the 3D soundings holds " + std::to_string(vertices[0]) +
            " and " + std::to_string(vertices[1]) + " vertices for 10,000 and 100,000 soundings (" +
            std::to_string(allocated[0]) + " and " + std::to_string(allocated[1]) + " allocated) and " +
            std::to_string(lines[0] + lines[1]) + " LINES commands; a fill with the mode set records " +
            std::to_string(recorded[0]) + " and " + std::to_string(recorded[1]));
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
}

/* 7b (F12): the profile lines of soundings from three files
   (make_sdg_three_files), whose two file boundaries join soundings that
   differ in their file alone, the first both good, the second with one
   flagged (counted first, so that the check cannot pass for want of
   them): in the Good and All modes no pair of the lists
   (mbvq_c_soundings_profile_pairs) joins two files, and the pairs equal
   the LINES command of a fill with the mode set (fill_reference), pairs
   and colours */
void model_profile_files() {
  make_sdg_three_files(4, 25, 7);
  const bool opened = open_sdg();
  int both_good = 0, one_flagged = 0;
  const int alike = sdg_file_boundaries(&both_good, &one_flagged);
  sdg_camera(35.0f, 125.0f, 1.3f, 0.05f, -0.05f);
  rhi::SoundingsFrame frame;
  TestBatch reference;
  bool apart = true, same = true;
  std::string counts;
  for (const int mode : {MBS_VIEW_PROFILES_UNFLAGGED, MBS_VIEW_PROFILES_ALL}) {
    sdg_view(MBS_VIEW_COLOR_FLAG, true, mode, true);
    rhi::SoundingsInput input;
    const bool filled = frame.begin(false, 400, 300, &input);
    fill_reference(&reference.b);
    std::vector<float> xyz;
    std::vector<uint32_t> black, red;
    const bool read = frame.positions(input.count, &xyz) && frame.pairs(input.count, &black, &red);
    int across = 0;
    for (const std::vector<uint32_t> *list : {&black, &red})
      for (size_t k = 0; k + 1 < list->size(); k += 2)
        across += sdg_storage[(*list)[k]].ifile != sdg_storage[(*list)[k + 1]].ifile ? 1 : 0;
    apart = apart && filled && read && across == 0 && !black.empty() &&
            (mode == MBS_VIEW_PROFILES_ALL) == !red.empty();
    same = same && filled && read && same_pairs(batch_pairs(reference.b, xyz), list_pairs(black, red));
    counts += std::to_string(black.size() / 2) + "+" + std::to_string(red.size() / 2) +
              (mode == MBS_VIEW_PROFILES_ALL ? "" : " and ");
  }
  check(opened && alike == 2 && both_good == 1 && one_flagged == 1 && apart && same,
        "7b: soundings from three files, each file's first ping numbered as the last of the file before (" +
            std::to_string(alike) + " file boundaries between soundings of the same ping and profile numbers, " +
            std::to_string(both_good) + " of them both good, " + std::to_string(one_flagged) +
            " with one flagged): no profile pair joins two files (" + (apart ? "yes" : "no") +
            "), and the pairs (black+red: " + counts + " in the Good and All modes) equal the LINES command of a "
            "fill with the mode set, pairs and colours (" + (same ? "yes" : "no") + ")");
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
}

/* 7a: each renderer times its create() into RenderStats::create_ms: -1
   before a renderer was made, set after the first frame of the map, profile
   and soundings renderers, and again by a renderer made anew after
   release(), which counts only the frames of the call after release()
   (last_frames()) and draws what the first one drew; at least 0.01 ms, since
   the shaders are loaded and the pipelines made inside the time. A create()
   that fails after its first resources (RenderOptions::fail_create, in
   make_fixed()) is timed too; so is the return for a QRhi without
   instancing, which Null and Metal cannot reach (checked by reading only) */
void null_create_time() {
  Grid grid;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "7a Null: a map for the renderers' create() times");
  /* the map's slot may have held a profile: mbview_destroy frees its points
     but leaves their count, so the picks go first, which clears it */
  mbview_clearpicks(instance);
  mbvq_c_profile_show(instance, true);
  make_sdg(40, 25, 7);
  open_sdg();
  sdg_camera(90.0f, 0.0f, 1.0f, 0.0f, 0.0f);
  mbvq::RenderOptions options;
  struct Kind {
    const char *name;
    std::function<bool(mbvq::OffscreenRenderer &)> frame;
  };
  const Kind kinds[] = {
      {"map", [&](mbvq::OffscreenRenderer &r) { return r.render_map(instance, 320, 240, options, nullptr); }},
      {"profile", [&](mbvq::OffscreenRenderer &r) { return r.render_profile(instance, 600, 200, options, nullptr); }},
      {"soundings", [&](mbvq::OffscreenRenderer &r) { return r.render_soundings(200, 150, options, nullptr); }}};
  for (const Kind &kind : kinds) {
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
    const double before = renderer.stats().create_ms;
    const bool drawn = kind.frame(renderer);
    const mbvq::RenderStats first = renderer.stats();
    renderer.release();
    const bool again = kind.frame(renderer);
    const mbvq::RenderStats remade = renderer.stats();
    const bool anew = remade.frames == renderer.last_frames() && remade.draws == first.draws &&
                      remade.soundings_drawn == first.soundings_drawn && remade.resources == first.resources;
    check(before == -1.0 && drawn && !first.failed && first.create_ms >= 0.01 && again && !remade.failed &&
              remade.create_ms >= 0.01 && anew,
          std::string("7a Null: the ") + kind.name + " renderer's create() time is -1 before it is made (" +
              text("%.0f", before) + "), set after its first frame (" + text("%.3f ms", first.create_ms) +
              ") and by a renderer made anew after release() (" + text("%.3f ms", remade.create_ms) + "; its frames " +
              std::to_string(remade.frames) + " of " + std::to_string(renderer.last_frames()) +
              ", the same draws, soundings and resources: " + (anew ? "yes" : "no") + ")");
  }
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
  options.fail_create = true;
  const bool drawn = renderer.render_map(instance, 320, 240, options, nullptr);
  const mbvq::RenderStats failed = renderer.stats();
  check(!drawn && failed.failed && failed.create_ms >= 0.0,
        "7a Null: a create() that fails after its first resources (RenderOptions::fail_create, in make_fixed()) is "
        "timed too (" + text("%.3f ms", failed.create_ms) + ")");
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
  mbvq_c_profile_show(instance, false);
  close_instance(instance);
}

/* 6d with a Null QRhi: the soundings renderer reads and writes only what its
   marks say, keeps the drawn set during a split upload, rebuilds the lists
   when it must, and draws only the pairs of the prefix in a first upload */
void null_soundings() {
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
  make_sdg(40, 25, 7);
  check(open_sdg(), "6d Null: the soundings handed over");
  sdg_camera(90.0f, 0.0f, 1.0f, 0.0f, 0.0f);
  sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, true);
  mbvq::RenderOptions options;
  options.soundings_chunk = 300;
  const auto reads = [&](int stream) { return renderer.stats().sounding_reads[stream]; };
  const auto writes = [&](int stream) { return renderer.stats().sounding_writes[stream]; };
  const int P = rhi::SoundingsPlan::kPositions, C = rhi::SoundingsPlan::kColours, L = rhi::SoundingsPlan::kLists;

  bool drawn = renderer.render_soundings(200, 150, options, nullptr);
  mbvq::RenderStats s = renderer.stats();
  check(drawn && s.soundings == 1000 && s.sounding_chunks == 4 && s.soundings_shown == 1000 &&
            s.soundings_drawn == 1000 && reads(P) == 1 && reads(C) == 1 && reads(L) == 1 && writes(P) == 4 &&
            writes(C) == 4 && writes(L) == 4 && s.sounding_allocations == 1 && s.profile_indices > 0 &&
            !s.profile_wide && s.resources > s.fixed_resources + s.batch_resources,
        "6d Null: the first frames read every stream once and write 4 chunks of 300 soundings each (" +
            std::to_string(s.profile_indices) + " profile indices drawn, " + std::to_string(s.resources) +
            " resources)");
  renderer.render_soundings(200, 150, options, nullptr);
  check(reads(P) == 1 && reads(C) == 1 && reads(L) == 1 && writes(P) == 4 && writes(C) == 4 && writes(L) == 4,
        "6d Null: a frame without a risen mark reads and writes nothing");

  /* an exaggeration change: the position mark, positions only */
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings.exaggeration = 2.0f;
  mb3dsoundings_scalez(0, &error);
  renderer.render_soundings(200, 150, options, nullptr);
  check(reads(P) == 2 && reads(C) == 1 && reads(L) == 1 && writes(P) == 8 && writes(C) == 4 && writes(L) == 4,
        "6d Null: a position mark reads and writes the positions only");

  /* a flag edit: the colour mark, colours and the lists */
  sdg_storage[12].beamflag = static_cast<char>(MB_FLAG_FLAG | MB_FLAG_MANUAL);
  mb3dsoundings_colors_changed(0, &error);
  renderer.render_soundings(200, 150, options, nullptr);
  check(reads(P) == 2 && reads(C) == 2 && reads(L) == 2 && writes(P) == 8 && writes(C) == 8 && writes(L) == 8,
        "6d Null: a colour mark (a flag edit) reads and writes the colours and makes the profile lists again");

  /* another profile mode raises no mark: the lists alone */
  mb3dsoundings_setprofiles(MBS_VIEW_PROFILES_UNFLAGGED);
  renderer.render_soundings(200, 150, options, nullptr);
  check(reads(P) == 2 && reads(C) == 2 && reads(L) == 3 && writes(L) == 12,
        "6d Null: another profile mode makes the lists again, nothing else");

  /* new soundings of another count: everything again */
  make_sdg(20, 25, 9);
  open_sdg();
  renderer.render_soundings(200, 150, options, nullptr);
  s = renderer.stats();
  check(s.sounding_allocations == 2 && s.soundings == 500 && s.sounding_chunks == 2 && reads(P) == 3 && reads(C) == 3 &&
            reads(L) == 4 && s.soundings_shown == 500,
        "6d Null: another array (the mask) reallocates and reads everything again");

  /* Dismiss: count 0, the mask rose; nothing is read, the copies go */
  mbvq_c_soundings_dismiss();
  sdg_data.num_soundings = 0;
  sdg_data.soundings = nullptr;
  renderer.render_soundings(200, 150, options, nullptr);
  s = renderer.stats();
  check(reads(P) == 3 && reads(C) == 3 && reads(L) == 4 && s.soundings == 0 && s.soundings_drawn == 0 &&
            s.sounding_held_bytes == 0,
        "6d Null: after Dismiss nothing is read or drawn, and the soundings' buffers go");

  /* A split upload: 1000 soundings in chunks of 260 under a budget of
     5000 bytes (positions 12 KB, double-buffered); the chunks cut pings, so
     that profile pairs reach from one chunk into the next. The first upload
     draws its prefix frame by frame, with only the pairs inside it (thin
     lines at ratio 1, wide ones at ratio 2), counted here from the core's
     lists without the renderer's helpers. */
  const int kSplitChunk = 260;
  const int kSplitChunks = (1000 + kSplitChunk - 1) / kSplitChunk;
  for (int ratio = 1; ratio <= 2; ratio++) {
    /* a renderer of its own, whose totals count this allocation only */
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
    const auto reads = [&](int stream) { return renderer.stats().sounding_reads[stream]; };
    make_sdg(40, 25, 7);
    open_sdg();
    sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, false);
    mbvq::RenderOptions split;
    split.soundings_chunk = kSplitChunk;
    split.upload_budget = 5000;
    split.pixel_ratio = ratio;
    /* the core's pairs (i, i + 1), black and red */
    std::vector<uint32_t> black, red;
    rhi::SoundingsFrame reader;
    reader.pairs(1000, &black, &red);
    /* what chunk c draws: the entries (indices) of its pairs, or its pairs
       (instances), without the pair that reaches into the next chunk (out) */
    const auto chunk_lines = [&](int c, bool out) {
      const uint32_t first = static_cast<uint32_t>(c * kSplitChunk);
      const uint32_t end = std::min<uint32_t>(first + kSplitChunk, 1000);
      int n = 0;
      if (ratio == 1) {
        for (const std::vector<uint32_t> *pairs : {&black, &red})
          for (size_t k = 0; k + 1 < pairs->size(); k += 2)
            n += (*pairs)[k] >= first && (*pairs)[k] < end && !(out && (*pairs)[k + 1] >= end) ? 2 : 0;
      }
      else {
        for (uint32_t i = first; i < end && i + 1 < 1000; i++)
          n += out && i + 1 >= end ? 0 : 1;
      }
      return n;
    };
    int across = 0;
    for (const std::vector<uint32_t> *pairs : {&black, &red})
      for (size_t k = 0; k + 1 < pairs->size(); k += 2)
        across += (*pairs)[k] / kSplitChunk != (*pairs)[k + 1] / kSplitChunk;
    std::vector<int> prefix;
    bool pairs_ok = true;
    std::string drawn_pairs;
    for (int f = 0; f < 16; f++) {
      renderer.render_soundings(200, 150, split, nullptr, 1);
      s = renderer.stats();
      prefix.push_back(s.soundings_shown);
      /* the chunks whose lines are written (in the order of the chunks) and
         whose points are drawn (a prefix of the chunks); the last one without
         its pair into the next chunk while that one is not drawn */
      const int written = ratio == 1 ? s.sounding_writes[L] : s.sounding_writes[rhi::SoundingsPlan::kPairColours];
      int chunks_drawn = 0;
      while (chunks_drawn < kSplitChunks && std::min((chunks_drawn + 1) * kSplitChunk, 1000) <= s.soundings_shown)
        chunks_drawn++;
      int expected = 0;
      for (int c = 0; c < std::min(written, chunks_drawn); c++)
        expected += chunk_lines(c, c + 1 < kSplitChunks && c + 1 >= chunks_drawn);
      const int got = ratio == 1 ? s.profile_indices : s.profile_instances;
      pairs_ok = pairs_ok && got == expected && s.profile_wide == (ratio == 2);
      drawn_pairs += std::to_string(got) + " ";
      if (s.soundings_shown == 1000 && written == kSplitChunks)
        break;
    }
    const bool grows = prefix.size() >= 3 && prefix.front() > 0 && prefix.front() < 1000 && prefix.back() == 1000 &&
                       std::is_sorted(prefix.begin(), prefix.end());
    check(grows && pairs_ok && across > 0,
          "6d Null: the first split upload at ratio " + std::to_string(ratio) + " draws its prefix " + list(prefix) +
              " and only the pairs inside it (" + (ratio == 1 ? "indices " : "instances ") + drawn_pairs + "; " +
              std::to_string(across) + " pairs reach into the next chunk)");

    /* a position mark: the drawn set stays complete while the new positions
       go into the second set; a mark during that upload does not restart it */
    if (ratio == 1) {
      const int read_before = reads(P);
      const int swaps_before = renderer.stats().sounding_swaps;
      mb3dsoundings.exaggeration = 3.0f;
      mb3dsoundings_scalez(0, &error);
      renderer.render_soundings(200, 150, split, nullptr, 1);
      const mbvq::RenderStats first = renderer.stats();
      mb3dsoundings.exaggeration = 4.0f;
      mb3dsoundings_scalez(0, &error);
      std::vector<int> shown;
      int frames = 0, reads_during = 0;
      while (renderer.stats().sounding_swaps == swaps_before && frames < 10) {
        renderer.render_soundings(200, 150, split, nullptr, 1);
        shown.push_back(renderer.stats().soundings_shown);
        reads_during = reads(P);
        frames++;
      }
      check(first.soundings_shown == 1000 && first.sounding_reads[P] == read_before + 1 &&
                std::all_of(shown.begin(), shown.end(), [](int n) { return n == 1000; }) &&
                reads_during == read_before + 1 && renderer.stats().sounding_swaps >= swaps_before + 1,
            "6d Null: a position mark during a split upload keeps the previous complete set drawn (" + list(shown) +
                ") and does not restart it");
    }
  }

  /* The frames a view draws (the frame of the core's request, then frames
     while the renderer asks for more, and no other): a mark that rises while
     its stream's split upload runs is read and written in those frames,
     since nothing else asks for a frame (the last motion of a drag; the
     core draws nothing when the button is released). The positions
     (double-buffered), then the index lists (written chunk by chunk). */
  {
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
    const auto reads = [&](int stream) { return renderer.stats().sounding_reads[stream]; };
    make_sdg(40, 25, 7);
    open_sdg();
    sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, false);
    mbvq::RenderOptions split;
    split.soundings_chunk = 250;
    split.upload_budget = 5000;
    renderer.render_soundings(200, 150, split, nullptr, 50);
    const int positions_before = reads(P);
    const int swaps_before = renderer.stats().sounding_swaps;
    /* motion 1: the core's request, its frame */
    mb3dsoundings.exaggeration = 2.5f;
    mb3dsoundings_scalez(0, &error);
    renderer.render_soundings(200, 150, split, nullptr, 1);
    const bool running = renderer.stats().soundings_waiting;
    /* motion 2 before the next frame: the view's frames for it */
    mb3dsoundings.exaggeration = 3.5f;
    mb3dsoundings_scalez(0, &error);
    renderer.render_soundings(200, 150, split, nullptr, 50);
    const int position_frames = renderer.last_frames();
    const bool positions_ok = running && position_frames < 50 && !renderer.stats().soundings_waiting &&
                              reads(P) == positions_before + 2 &&
                              renderer.stats().sounding_swaps == swaps_before + 2 &&
                              renderer.stats().soundings_shown == 1000;
    const std::string positions_text = std::to_string(reads(P) - positions_before) + " reads and " +
                                       std::to_string(renderer.stats().sounding_swaps - swaps_before) + " swaps in " +
                                       std::to_string(position_frames) + " frames";
    /* the lists: a flag edit while the lists of the one before are written */
    const int lists_before = reads(L);
    sdg_storage[100].beamflag = static_cast<char>(MB_FLAG_FLAG | MB_FLAG_MANUAL);
    mb3dsoundings_colors_changed(0, &error);
    renderer.render_soundings(200, 150, split, nullptr, 1);
    const bool lists_running = renderer.stats().soundings_waiting;
    sdg_storage[600].beamflag = static_cast<char>(MB_FLAG_FLAG | MB_FLAG_MANUAL);
    mb3dsoundings_colors_changed(0, &error);
    renderer.render_soundings(200, 150, split, nullptr, 50);
    const int list_frames = renderer.last_frames();
    const bool lists_ok = lists_running && list_frames < 50 && !renderer.stats().soundings_waiting &&
                          reads(L) == lists_before + 2;
    check(positions_ok && lists_ok,
          "6d Null: in the frames a view draws, a mark that rose during its stream's split upload is read after it: "
          "positions " + positions_text + " (2 and 2 for two marks), lists made " +
              std::to_string(reads(L) - lists_before) + " times in " + std::to_string(list_frames) +
              " frames for two flag edits");
  }

  /* points as quads (forced, as on Direct3D) */
  {
    mbvq::OffscreenRenderer quads(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions forced;
    forced.point_quads = true;
    sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_NONE, true);
    mb3dsoundings.edit_mode = MBS_EDIT_INFO;
    mb3dsoundings.last_sounding_defined = true;
    mb3dsoundings.last_sounding_edited = 10;
    const bool ok = quads.render_soundings(200, 150, forced, nullptr);
    /* the information highlight is the batch's one POINTS command: one draw
       of quads (the soundings' quads are the view's own draw) */
    check(ok && quads.stats().point_quads && quads.stats().soundings_drawn == 1000 &&
              quads.stats().point_quad_draws == 1 && !quads.stats().failed,
          "6d Null: points as quads where forced: the soundings (" + std::to_string(quads.stats().soundings_drawn) +
              ") and the information highlight (" + std::to_string(quads.stats().point_quad_draws) +
              " draw of the batch's points as quads)");
  }
  mb3dsoundings_end(0, &error);
}

/* 6d with a Null QRhi: the failure paths of the soundings and profile
   renderers, which no view reaches unless the GPU memory runs out. (A lost
   uniform ring cannot happen in these views: their drawings have far fewer
   commands than the ring's first 64 slots.) */
void null_failures() {
  int error = MB_ERROR_NO_ERROR;
  const int P = rhi::SoundingsPlan::kPositions, C = rhi::SoundingsPlan::kColours, L = rhi::SoundingsPlan::kLists;
  make_sdg(40, 25, 7);
  check(open_sdg(), "6d Null failures: the soundings handed over");
  sdg_camera(90.0f, 0.0f, 1.0f, 0.0f, 0.0f);
  sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, true);

  /* A soundings frame whose buffers cannot be made (fail_buffers, after a
     flag edit whose chunk needs index lists of another size) fails at once
     and holds no GPU bytes, neither the soundings' nor the batch's; the
     next frame reads every stream once and draws every sounding. */
  {
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions options;
    options.soundings_chunk = 300;
    const bool drawn = renderer.render_soundings(200, 150, options, nullptr);
    const mbvq::RenderStats before = renderer.stats();
    sdg_storage[12].beamflag = static_cast<char>(MB_FLAG_FLAG | MB_FLAG_MANUAL);
    mb3dsoundings_colors_changed(0, &error);
    options.fail_buffers = true;
    const bool failed_frame = renderer.render_soundings(200, 150, options, nullptr);
    const mbvq::RenderStats failed = renderer.stats();
    const int failed_frames = renderer.last_frames();
    options.fail_buffers = false;
    const bool again = renderer.render_soundings(200, 150, options, nullptr);
    const mbvq::RenderStats after = renderer.stats();
    const int reads[3] = {after.sounding_reads[P] - failed.sounding_reads[P],
                          after.sounding_reads[C] - failed.sounding_reads[C],
                          after.sounding_reads[L] - failed.sounding_reads[L]};
    check(drawn && before.sounding_held_bytes > 0 && before.batch_held_bytes > 0 && !failed_frame && failed.failed &&
              failed_frames == 1 && failed.sounding_held_bytes == 0 && failed.batch_held_bytes == 0 &&
              failed.batch_resources == 0 && failed.resources == failed.fixed_resources && failed.soundings_shown == 0 &&
              again && !after.failed && reads[0] == 1 && reads[1] == 1 && reads[2] == 1 &&
              after.soundings_shown == 1000 && after.soundings_drawn == 1000 && after.sounding_held_bytes > 0,
          "6d Null: a soundings frame whose buffers cannot be made fails at once (" + std::to_string(failed_frames) +
              " frame) and holds no GPU bytes (" + std::to_string(failed.sounding_held_bytes) + " of the soundings, " +
              std::to_string(failed.batch_held_bytes) + " of the batch, against " +
              std::to_string(before.sounding_held_bytes) + " and " + std::to_string(before.batch_held_bytes) +
              " before); the next frame reads each stream once (" + std::to_string(reads[0]) + ", " +
              std::to_string(reads[1]) + ", " + std::to_string(reads[2]) + ") and draws " +
              std::to_string(after.soundings_drawn) + " soundings");
  }

  /* A read of the soundings' data that fails (fail_reads: none can today)
     fails its frame at once, as a buffer that cannot be made does, rather
     than asking for frames for ever while its mark stays ahead: the
     positions after an exaggeration change, the colours after a flag edit,
     the profile pairs after another profile mode; after each the next frame
     reads and draws everything again. */
  {
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions options;
    options.soundings_chunk = 300;
    bool ok = renderer.render_soundings(200, 150, options, nullptr);
    std::string frames;
    for (int stream = 0; stream < 3; stream++) {
      if (stream == 0) {
        mb3dsoundings.exaggeration = 2.0f;
        mb3dsoundings_scalez(0, &error);
      }
      else if (stream == 1) {
        sdg_storage[600].beamflag = static_cast<char>(MB_FLAG_FLAG | MB_FLAG_MANUAL);
        mb3dsoundings_colors_changed(0, &error);
      }
      else {
        mb3dsoundings_setprofiles(MBS_VIEW_PROFILES_UNFLAGGED);
      }
      const int reads_before = renderer.stats().sounding_reads[stream];
      options.fail_reads = true;
      const bool read_failed = !renderer.render_soundings(200, 150, options, nullptr);
      const mbvq::RenderStats failed = renderer.stats();
      const int failed_frames = renderer.last_frames();
      options.fail_reads = false;
      const bool again = renderer.render_soundings(200, 150, options, nullptr);
      const mbvq::RenderStats after = renderer.stats();
      ok = ok && read_failed && failed.failed && failed_frames == 1 && failed.sounding_reads[stream] == reads_before &&
           failed.sounding_held_bytes == 0 && failed.batch_held_bytes == 0 && again && !after.failed &&
           after.sounding_reads[stream] == reads_before + 1 && after.soundings_drawn == 1000;
      frames += (stream > 0 ? ", " : "") + std::to_string(failed_frames);
    }
    check(ok, "6d Null: a read that fails ends its frame at once, as a failed buffer does (positions, colours and "
              "profile pairs: " +
                  frames + " frames of 16 allowed), holding nothing; the next frame reads and draws everything again");
  }

  /* A failed frame hands its update batch back to the QRhi, whose pool
     holds 64: kept by failed frames, the batches would run out after 64 of
     them, and the frame that draws after these 70 would get none and
     crash. Here every read fails; a buffer that cannot be made takes the
     same path. */
  {
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions options;
    options.soundings_chunk = 300;
    const int tries = 70;
    options.fail_reads = true;
    int failures = 0;
    for (int f = 0; f < tries; f++)
      failures += !renderer.render_soundings(200, 150, options, nullptr) && renderer.stats().failed &&
                  renderer.last_frames() == 1;
    options.fail_reads = false;
    const bool again = renderer.render_soundings(200, 150, options, nullptr);
    const mbvq::RenderStats after = renderer.stats();
    check(failures == tries && again && !after.failed && after.soundings_drawn == 1000,
          "6d Null: failed soundings frames hand back their update batches (" + std::to_string(failures) + " of " +
              std::to_string(tries) + " failed in a row, a QRhi's pool holds 64): the next frame draws " +
              std::to_string(after.soundings_drawn) + " soundings");
  }
  mb3dsoundings_end(0, &error);

  /* A profile frame whose buffers cannot be made (fail_buffers, when its
     batch grows from the frame alone to a profile of seven points) fails,
     draws nothing and lets go of the batch's buffers; the next frame draws
     the profile again. */
  Grid grid;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "6d Null failures: a map for the profile");
  struct mbview_struct *data = data_of(instance);
  mbvq_c_profile_show(instance, true);
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
  mbvq::RenderOptions options;
  const bool first = renderer.render_profile(instance, 600, 200, options, nullptr);
  const mbvq::RenderStats framed = renderer.stats();
  const int n = 7;
  if (data->profile.npoints_alloc < n && mbview_allocprofilepoints(0, n, &data->profile.points, &error) == MB_SUCCESS)
    data->profile.npoints_alloc = n;
  if (data->profile.npoints_alloc >= n) {
    const double z[n] = {-1000.0, -985.0, -1012.0, -995.0, -1003.0, -990.0, -1008.0};
    data->profile.npoints = n;
    data->profile.length = 600.0;
    data->profile.zmin = -1012.0;
    data->profile.zmax = -985.0;
    for (int i = 0; i < n; i++) {
      struct mbview_profilepoint_struct &p = data->profile.points[i];
      p.boundary = i == 0 || i == 3 || i == n - 1;
      p.distance = 100.0 * i;
      p.zdata = z[i];
      p.slope = i % 2 == 0 ? 0.1 : 0.9;
    }
    data->profile_slopethreshold = 0.5;
    data->profile_exaggeration = 1.0;
  }
  options.fail_buffers = true;
  const bool failed_frame = renderer.render_profile(instance, 600, 200, options, nullptr);
  const mbvq::RenderStats failed = renderer.stats();
  options.fail_buffers = false;
  const bool again = renderer.render_profile(instance, 600, 200, options, nullptr);
  const mbvq::RenderStats drawn = renderer.stats();
  check(first && !framed.failed && framed.batch_resources > 0 && data->profile.npoints == n && !failed_frame &&
            failed.failed && failed.batch_resources == 0 && failed.batch_held_bytes == 0 &&
            failed.resources == failed.fixed_resources && again && !drawn.failed && drawn.batch_resources > 0 &&
            drawn.draws > framed.draws,
        "6d Null: a profile frame whose buffers cannot be made fails and lets go of the batch's buffers (" +
            std::to_string(failed.batch_resources) + " left, against " + std::to_string(framed.batch_resources) +
            " before); the next frame draws the profile of " + std::to_string(n) + " points (" +
            std::to_string(drawn.draws) + " draws, against " + std::to_string(framed.draws) + " for the frame alone)");

  /* So does a failed profile frame (see the soundings above): 70 in a row
     on another Null QRhi, then a frame that draws the profile. */
  {
    mbvq::OffscreenRenderer fresh(mbvq::OffscreenRenderer::Api::Null);
    const int tries = 70;
    options.fail_buffers = true;
    int failures = 0;
    for (int f = 0; f < tries; f++)
      failures += !fresh.render_profile(instance, 600, 200, options, nullptr) && fresh.stats().failed;
    options.fail_buffers = false;
    const bool redrawn = fresh.render_profile(instance, 600, 200, options, nullptr);
    const mbvq::RenderStats after = fresh.stats();
    check(failures == tries && redrawn && !after.failed && after.draws > framed.draws,
          "6d Null: failed profile frames hand back their update batches (" + std::to_string(failures) + " of " +
              std::to_string(tries) + " failed in a row): the next frame draws the profile (" +
              std::to_string(after.draws) + " draws, against " + std::to_string(framed.draws) +
              " for the frame alone)");
  }
  mbvq_c_profile_show(instance, false);
  close_instance(instance);
}

/* 7b with a Null QRhi: the profile mode that a fill hides from the core
   while it records the drawing (mbvq_c_soundings_plot_decorations) is the
   one set after each frame, also after a frame whose fill was refused
   (another batch the target, as while another view fills): the modes All,
   Good, None and All again, as the View menu sets them
   (mb3dsoundings_setprofiles). On soundings from three files
   (make_sdg_three_files) the renderer draws the core's pairs from its own
   lists: as many indices as the LINES command of a fill with the mode set
   (fill_reference, filled before the frame, so that a mode left hidden
   cannot empty it too) has vertices, none in the None mode, also in the
   refused frame. */
void null_profile_mode() {
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
  make_sdg_three_files(4, 25, 7);
  const bool opened = open_sdg();
  int both_good = 0, one_flagged = 0;
  const int alike = sdg_file_boundaries(&both_good, &one_flagged);
  sdg_camera(90.0f, 0.0f, 1.0f, 0.0f, 0.0f);
  sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_NONE, true);
  const mbvq::RenderOptions options;
  TestBatch reference;
  const char *const names[3] = {"None", "Good", "All"};
  bool kept = opened;
  bool drawn = opened && alike == 2 && both_good == 1 && one_flagged == 1;
  std::string modes, indices;
  for (const int mode : {MBS_VIEW_PROFILES_ALL, MBS_VIEW_PROFILES_UNFLAGGED, MBS_VIEW_PROFILES_NONE,
                         MBS_VIEW_PROFILES_ALL}) {
    mb3dsoundings_setprofiles(mode);
    fill_reference(&reference.b);
    const int expected = line_vertices(reference.b);
    const bool ok = renderer.render_soundings(200, 150, options, nullptr);
    const mbvq::RenderStats s = renderer.stats();
    const bool after = mb3dsoundings.view_profiles == mode;
    /* a frame whose fill is refused: it draws the batch it has (a failed
       frame would leave the statistics of the frame before) */
    bool refused_drawn = false;
    {
      TestBatch other;
      mbv_draw_set_target(&other.b);
      refused_drawn = renderer.render_soundings(200, 150, options, nullptr, 1);
      mbv_draw_set_target(nullptr);
    }
    const mbvq::RenderStats refused = renderer.stats();
    const bool after_refused = mb3dsoundings.view_profiles == mode;
    kept = kept && ok && !s.failed && after && refused_drawn && refused.fills == s.fills && !refused.failed &&
           after_refused;
    drawn = drawn && ok && refused_drawn && !refused.failed && !s.profile_wide && s.profile_indices == expected &&
            (mode == MBS_VIEW_PROFILES_NONE) == (expected == 0) && refused.profile_indices == expected;
    modes += std::string(modes.empty() ? "" : ", ") + names[mode] + " " + (after ? "yes" : "no") + "/" +
             (after_refused ? "yes" : "no") + (refused_drawn && !refused.failed ? "" : " (the refused frame failed)");
    indices += std::string(indices.empty() ? "" : ", ") + std::to_string(s.profile_indices) + " of " +
               std::to_string(expected) + " (refused " + std::to_string(refused.profile_indices) + ")";
  }
  check(kept, "7b Null: the profile mode that a fill hides from the core is the one set after each frame and after "
              "a frame whose fill was refused, another batch the target (" + modes + ")");
  check(drawn, "7b Null: on soundings from three files (" + std::to_string(alike) +
                   " file boundaries between soundings of the same ping and profile numbers) the renderer draws as "
                   "many profile indices as the LINES command of a fill with the mode set has vertices, in the All, "
                   "Good, None and All modes (" + indices + "), also in a frame whose fill was refused");
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
}

/* 7c without drawing: the 3D soundings view's camera press, the middle or
   right mouse button down and the left up, during which the GPU-time
   fallback may draw fewer soundings. When a camera press ends the view asks
   for a frame itself (update() while shown, counted in frame_requests),
   which draws every sounding: the core asks for none after a camera
   button's release. The left button, which edits, starts no camera press
   and asks for nothing (the core asks for an edit's drawing). A release
   lost to a grab, a hide, another window, a modal window or a popup ends
   the press as in the map view; hidden for real, the view asks for nothing
   (a hidden view draws nothing; its next frame comes when it is shown). The
   buttons are the view's own, never the state of the process that every
   map's fill fallback reads (rhi::map_buttons_down). The soundings window's
   view, shown and never drawn (no QRhi here), read through view_state(). */
void widgets_soundings_press(QWidget *view) {
  const QPointF centre(view->width() / 2.0, view->height() / 2.0);
  const QEvent::Type pressed = QEvent::MouseButtonPress, released = QEvent::MouseButtonRelease;
  /* a mouse event of the button, reporting these buttons down */
  const auto send_mouse = [&](QEvent::Type type, Qt::MouseButton button, Qt::MouseButtons buttons) {
    QMouseEvent event(type, centre, view->mapToGlobal(centre), button, buttons, Qt::NoModifier);
    QApplication::sendEvent(view, &event);
  };
  /* After a step: the view's buttons, its camera press and the frames it
     asked for in the step, against the expected ones; the step added to the
     text as "name (buttons on|off +frames)". No map button may be down
     after any step. */
  const mbvq::ViewState idle = mbvq::view_state(view);
  int requests = idle.frame_requests;
  int steps = 0;
  bool maps_up = !rhi::map_buttons_down();
  const auto state_after = [&](const std::string &name, int buttons, bool press, int asked, std::string *seen) {
    const mbvq::ViewState state = mbvq::view_state(view);
    const int new_requests = state.frame_requests - requests;
    requests = state.frame_requests;
    steps++;
    maps_up = maps_up && !rhi::map_buttons_down();
    *seen += std::string(seen->empty() || seen->back() == ' ' ? "" : ", ") + name + " (" +
             std::to_string(state.buttons) + (state.camera_press ? " on +" : " off +") +
             std::to_string(new_requests) + ")";
    return state.buttons == buttons && state.camera_press == press && new_requests == asked;
  };
  /* mouse events in a row, each a press or a release of a button with the
     buttons it reports down, then the expected buttons, camera press and
     frames asked for; a sequence's text starts with its label, if any */
  struct Step {
    const char *name;
    QEvent::Type type;
    Qt::MouseButton button;
    Qt::MouseButtons down;
    int buttons;
    bool press;
    int asked;
  };
  const auto run = [&](const char *label, const std::vector<Step> &sequence, std::string *seen) {
    if (*label != '\0')
      *seen += std::string(seen->empty() ? "" : "; ") + label + ": ";
    bool ok = true;
    for (const Step &step : sequence) {
      send_mouse(step.type, step.button, step.down);
      ok = state_after(step.name, step.buttons, step.press, step.asked, seen) && ok;
    }
    return ok;
  };

  /* one button at a time */
  std::string clicks;
  const bool clicked = run("", {{"L", pressed, Qt::LeftButton, Qt::LeftButton, 1, false, 0},
                                {"L up", released, Qt::LeftButton, Qt::NoButton, 0, false, 0},
                                {"M", pressed, Qt::MiddleButton, Qt::MiddleButton, 2, true, 0},
                                {"M up", released, Qt::MiddleButton, Qt::NoButton, 0, false, 1},
                                {"R", pressed, Qt::RightButton, Qt::RightButton, 4, true, 0},
                                {"R up", released, Qt::RightButton, Qt::NoButton, 0, false, 1}},
                           &clicks);
  check(view->isVisible() && idle.buttons == 0 && !idle.camera_press && idle.frame_requests == 0 && clicked,
        "7c: in the 3D soundings view, shown, a left click is no camera press and asks for no frame; a middle or a "
        "right click is one while its button is down, and its release asks for a frame (idle " +
            std::to_string(idle.buttons) + (idle.camera_press ? " on, " : " off, ") +
            std::to_string(idle.frame_requests) +
            " frames asked so far, the left click before the first frame included; after each step the buttons, the "
            "camera press and the frames asked for: " + clicks + ")");

  /* two buttons: the left one ends a camera press, the right one added to
     the middle keeps it, the middle one added to the left starts none */
  std::string two;
  const bool middle_left =
      run("M then L", {{"M", pressed, Qt::MiddleButton, Qt::MiddleButton, 2, true, 0},
                       {"L", pressed, Qt::LeftButton, Qt::LeftButton | Qt::MiddleButton, 3, false, 1},
                       {"L up", released, Qt::LeftButton, Qt::MiddleButton, 2, true, 0},
                       {"M up", released, Qt::MiddleButton, Qt::NoButton, 0, false, 1}},
          &two);
  const bool middle_right =
      run("M then R", {{"M", pressed, Qt::MiddleButton, Qt::MiddleButton, 2, true, 0},
                       {"R", pressed, Qt::RightButton, Qt::MiddleButton | Qt::RightButton, 6, true, 0},
                       {"R up", released, Qt::RightButton, Qt::MiddleButton, 2, true, 0},
                       {"M up", released, Qt::MiddleButton, Qt::NoButton, 0, false, 1}},
          &two);
  const bool left_middle =
      run("L then M", {{"L", pressed, Qt::LeftButton, Qt::LeftButton, 1, false, 0},
                       {"M", pressed, Qt::MiddleButton, Qt::LeftButton | Qt::MiddleButton, 3, false, 0},
                       {"M up", released, Qt::MiddleButton, Qt::LeftButton, 1, false, 0},
                       {"L up", released, Qt::LeftButton, Qt::NoButton, 0, false, 0}},
          &two);
  check(middle_left && middle_right && left_middle,
        "7c: two buttons in the 3D soundings view: the left one pressed during a camera press ends it and asks for "
        "a frame, its release starts the press again; the right one added to the middle keeps it, and the middle's "
        "release asks for the frame; the middle one added to the left starts none, and nothing is asked for (" +
            two + ")");

  /* a camera press, then a release lost to a grab, a hide, another window, a
     modal window or a popup, the view shown: a frame asked for after each;
     a focus lost for another reason keeps the press */
  QEvent ungrab(QEvent::UngrabMouse), hide_event(QEvent::Hide), deactivate(QEvent::WindowDeactivate),
      blocked(QEvent::WindowBlocked);
  QFocusEvent popup(QEvent::FocusOut, Qt::PopupFocusReason);
  const std::pair<const char *, QEvent *> losses[] = {{"UngrabMouse", &ungrab},
                                                      {"Hide", &hide_event},
                                                      {"WindowDeactivate", &deactivate},
                                                      {"WindowBlocked", &blocked},
                                                      {"FocusOut for a popup", &popup}};
  std::string lost_seen;
  bool lost = true;
  for (const auto &[name, event] : losses) {
    send_mouse(pressed, Qt::MiddleButton, Qt::MiddleButton);
    const bool held = state_after("M", 2, true, 0, &lost_seen);
    QApplication::sendEvent(view, event);
    const bool cleared = state_after(name, 0, false, 1, &lost_seen);
    lost = lost && held && cleared && view->isVisible();
  }
  QFocusEvent tab(QEvent::FocusOut, Qt::TabFocusReason);
  send_mouse(pressed, Qt::MiddleButton, Qt::MiddleButton);
  const bool tab_held = state_after("M", 2, true, 0, &lost_seen);
  QApplication::sendEvent(view, &tab);
  const bool kept = state_after("FocusOut for the tab key", 2, true, 0, &lost_seen);
  send_mouse(released, Qt::MiddleButton, Qt::NoButton);
  const bool kept_released = state_after("M up", 0, false, 1, &lost_seen);
  check(lost && tab_held && kept && kept_released,
        "7c: UngrabMouse, Hide, WindowDeactivate, WindowBlocked and FocusOut for a popup end a camera press in the "
        "3D soundings view, shown: the buttons cleared and a frame asked for after each; FocusOut for another reason "
        "keeps the press, whose release asks for the frame (" + lost_seen + ")");

  /* hidden for real during a camera press: the buttons cleared and no frame
     asked for (a hidden view draws nothing; its next frame comes when it is
     shown); shown again, the end of a camera press asks for one again */
  std::string hiding;
  send_mouse(pressed, Qt::MiddleButton, Qt::MiddleButton);
  const bool hide_held = state_after("M", 2, true, 0, &hiding);
  view->hide();
  const bool gone = !view->isVisible();
  const bool hidden = state_after("hide()", 0, false, 0, &hiding);
  view->show();
  const bool back = view->isVisible();
  const bool shown = state_after("show()", 0, false, 0, &hiding);
  const bool again = run("", {{"M", pressed, Qt::MiddleButton, Qt::MiddleButton, 2, true, 0},
                              {"M up", released, Qt::MiddleButton, Qt::NoButton, 0, false, 1}},
                         &hiding);
  check(hide_held && gone && hidden && back && shown && again,
        "7c: hidden for real during a camera press, the 3D soundings view clears its buttons and asks for no frame "
        "(a hidden view draws nothing); shown again, a middle click asks for one again (hidden " +
            std::string(gone ? "yes" : "no") + ", shown again " + (back ? "yes" : "no") + "; " + hiding + ")");

  check(maps_up, "7c: the 3D soundings view's buttons stay out of the state of the process that every map's fill "
                 "fallback reads: no map button down before or after any of its " +
                     std::to_string(steps) + " steps (rhi::map_buttons_down)");
}

/* 6d without drawing: the profile and soundings views are QRhi views; the
   profile's width cap; the soundings view's failure label, input before the
   first frame, its point-size control and its status during a fill; and
   (7b) the profile mode that a fill hides from the core: the View menu's
   item and the core's mode after a frame's fill, the core's mode after a
   refused fill, and the item when the core asks the window to show its
   state (mb3dsoundings_updategui); and (7c) the 3D soundings view's camera
   press (widgets_soundings_press) */
void widgets_views(size_t instance, QRhiWidget::Api expected) {
  mbvq::MapWidget *widget = mbvq::map_widget(instance);
  mbvq::ProfileWindow *window = widget != nullptr ? widget->profile() : nullptr;
  QWidget *profile = window != nullptr ? window->view() : nullptr;
  auto *profile_rhi = qobject_cast<QRhiWidget *>(profile);
  check(profile_rhi != nullptr && profile_rhi->api() == expected && mbvq::view_state(profile).rhi_view,
        "6d: the profile view is a QRhiWidget with the platform's API set explicitly");
  if (profile == nullptr)
    return;
  const double ratio = profile->devicePixelRatioF();
  const int cap = static_cast<int>(std::floor(16384 / ratio));
  check(profile->maximumWidth() == cap && profile->maximumHeight() == cap,
        "6d: before a QRhi exists the profile view is at most floor(16384 / ratio) = " + std::to_string(cap) +
            " logical px wide");
  /* five times a viewport wider than that: the drawing is clamped */
  window->open();
  window->resize(4000, 400);
  window->slider(0)->setValue(5);
  spin(50);
  const int clamped = profile->width();
  /* The cap taken again on a change of the pixel ratio or the screen, and
     when the window sizes the view. A smaller cap first, as on a screen of a
     higher ratio, shrinks the view (still wider than the viewport, so that
     the scroll bars stay and the viewport keeps its size); when the cap
     rises again (back on the first screen) the drawing is sized again, to
     the clamped width. */
  bool again = true;
  std::string widths;
  const QWidget *viewport = profile->parentWidget();
  const int smaller = (cap + (viewport != nullptr ? viewport->width() : 0)) / 2;
  for (QEvent::Type type : {QEvent::DevicePixelRatioChange, QEvent::ScreenChangeInternal}) {
    profile->setMaximumSize(smaller, smaller);
    spin(20);
    const bool shrunk = profile->width() == smaller && viewport != nullptr && viewport->width() < smaller;
    QEvent event(type);
    QApplication::sendEvent(profile, &event);
    const bool capped = profile->maximumWidth() == cap;
    spin(20);
    widths += std::to_string(profile->width()) + " ";
    again = again && shrunk && capped && profile->width() == clamped;
  }
  profile->setMaximumSize(100, 100);
  window->slider(0)->setValue(4);
  spin(20);
  check(clamped == cap && again && profile->maximumWidth() == cap,
        "6d: a profile 5 x 4000 px wide is clamped to the cap (" + std::to_string(clamped) +
            "), taken again on DevicePixelRatioChange and ScreenChangeInternal, after which the drawing is sized "
            "again (" + widths + "px), and by layout_profile()");
  window->dismiss();

  /* the soundings view */
  make_sdg(20, 25, 7);
  check(open_sdg(), "6d: the soundings window under QRhi");
  mbvq::SoundingsWidget *soundings = mbvq::soundings_widget();
  QWidget *view = soundings != nullptr ? soundings->view() : nullptr;
  auto *view_rhi = qobject_cast<QRhiWidget *>(view);
  check(view_rhi != nullptr && view_rhi->api() == expected && mbvq::view_state(view).rhi_view,
        "6d: the soundings view is a QRhiWidget with the platform's API set explicitly");
  if (view == nullptr)
    return;
  QElapsedTimer timer;
  timer.start();
  while (!mbvq::view_state(view).failure_shown && timer.elapsed() < 3000)
    spin(20);
  check(mbvq::view_state(view).failure_shown && mbvq::failure_messages() == 1,
        "6d: the soundings view that cannot be drawn shows its label; the line came once per process");
  /* A click right on a sounding before the first frame edits nothing (the
     core is given a size and the screen positions here, so that a click
     that reached it would edit that sounding). */
  {
    const double r = view->devicePixelRatioF();
    const int pw = static_cast<int>(std::lround(view->width() * r));
    const int ph = static_cast<int>(std::lround(view->height() * r));
    mbvq_c_soundings_resize(pw, ph);
    sdg_camera(90.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    int error = MB_ERROR_NO_ERROR;
    mb3dsoundings_screen_positions(0, &error);
    QPointF at(view->width() / 2.0, view->height() / 2.0);
    for (const struct mb3dsoundings_sounding_struct &sounding : sdg_storage)
      if (sounding.beamflag == MB_FLAG_NONE && sounding.winx > 5 && sounding.winx < pw - 5 && sounding.winy > 5 &&
          sounding.winy < ph - 5) {
        at = QPointF(sounding.winx / r, (ph - 1 - sounding.winy) / r);
        break;
      }
    const int edits = sdg_edits;
    QMouseEvent press(QEvent::MouseButtonPress, at, view->mapToGlobal(at), Qt::LeftButton, Qt::LeftButton,
                      Qt::NoModifier);
    QApplication::sendEvent(view, &press);
    QMouseEvent release(QEvent::MouseButtonRelease, at, view->mapToGlobal(at), Qt::LeftButton, Qt::NoButton,
                        Qt::NoModifier);
    QApplication::sendEvent(view, &release);
    check(sdg_edits == edits && !mbvq::view_state(view).drawn,
          "6d: a click on a sounding before the soundings view's first frame does not reach the core");
  }

  /* 7c: the soundings view's camera press, and the frame it asks for when
     one ends (widgets_soundings_press) */
  widgets_soundings_press(view);

  /* the point-size control, under QRhi only */
  QDoubleSpinBox *point_size = soundings->findChild<QDoubleSpinBox *>();
  QLabel *point_label = nullptr;
  for (QLabel *label : soundings->findChildren<QLabel *>())
    if (label->text() == QStringLiteral("Point size"))
      point_label = label;
  check(point_size != nullptr && point_size->minimum() == 0.5 && point_size->maximum() == 8.0 &&
            point_size->singleStep() == 0.5 && point_size->value() == 3.0 &&
            point_size->suffix() == QStringLiteral(" px") &&
            point_size->toolTip() == QStringLiteral("logical pixels, times the screen's pixel ratio") &&
            point_label != nullptr && point_label->buddy() == point_size && mbvq::soundings_point_size() == 3.0,
        "6d: the soundings window has the point-size control: 0.5 to 8 px in steps of 0.5, 3 by default");
  if (point_size != nullptr) {
    point_size->setValue(2.0);
    const double two = mbvq::soundings_point_size();
    point_size->setValue(2.3);
    const double snapped = mbvq::soundings_point_size();
    point_size->setValue(3.0);
    check(two == 2.0 && snapped == 2.5 && point_size->value() == 3.0 && mbvq::soundings_point_size() == 3.0,
          "6d: the control sets the program's point size (2), a typed 2.3 becomes 2.5");

    /* Keys as the user types them: a typed value counts once, on Enter or
       when the box loses the focus (keyboard tracking off; with it the first
       key set 2 and the third snapped the text to 2.5 under the cursor); the
       arrows and the wheel set the size at once. */
    const QString point = point_size->locale().decimalPoint();
    const auto type = [point_size](const QString &keys) {
      point_size->selectAll();
      for (const QChar c : keys) {
        QKeyEvent key(QEvent::KeyPress, 0, Qt::NoModifier, QString(c));
        QApplication::sendEvent(point_size, &key);
      }
    };
    type(QStringLiteral("2") + point + QStringLiteral("3"));
    const double typing = mbvq::soundings_point_size();
    QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(point_size, &enter);
    const double entered = mbvq::soundings_point_size();
    type(QStringLiteral("4") + point + QStringLiteral("2"));
    const double typing_again = mbvq::soundings_point_size();
    QFocusEvent leave(QEvent::FocusOut, Qt::TabFocusReason);
    QApplication::sendEvent(point_size, &leave);
    const double left = mbvq::soundings_point_size();
    QKeyEvent up(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QApplication::sendEvent(point_size, &up);
    const double arrow = mbvq::soundings_point_size();
    QWheelEvent wheel(QPointF(5, 5), point_size->mapToGlobal(QPointF(5, 5)), QPoint(), QPoint(0, 120), Qt::NoButton,
                      Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(point_size, &wheel);
    const double wheeled = mbvq::soundings_point_size();
    point_size->setValue(3.0);
    check(!point_size->keyboardTracking() && typing == 3.0 && entered == 2.5 && typing_again == 2.5 && left == 4.0 &&
              arrow == 4.5 && wheeled == 5.0 && mbvq::soundings_point_size() == 3.0,
          "6d: a size typed into the control counts once: 2" + point.toStdString() + "3 sets nothing (" +
              text("%.1f", typing) + ") until Enter, then 2.5 (" + text("%.1f", entered) + "); 4" +
              point.toStdString() + "2 nothing (" + text("%.1f", typing_again) + ") until the box loses the focus, " +
              "then 4 (" + text("%.1f", left) + "); the up arrow and the wheel at once (" + text("%.1f", arrow) +
              ", " + text("%.1f", wheeled) + ")");
  }

  /* the status during a QRhi fill: the label, the signal and the sliders
     once, after the fill; outside a fill at once */
  {
    std::vector<QString> sent;
    const QMetaObject::Connection connection =
        QObject::connect(soundings, &mbvq::SoundingsWidget::statusMessage, [&sent](const QString &t) { sent.push_back(t); });
    mbvq_qt_soundings_status("before the fill", true);
    QLabel *status = nullptr;
    for (QLabel *label : soundings->findChildren<QLabel *>())
      if (label->text() == QStringLiteral("before the fill"))
        status = label;
    QSlider *slider = soundings->findChild<QSlider *>();
    sent.clear();
    mbvq::set_fill_active(true);
    mbvq_qt_soundings_status("first", false);
    mbvq_qt_soundings_status("second", false);
    const bool during = status != nullptr && status->text() == QStringLiteral("before the fill") && sent.empty() &&
                        slider != nullptr && slider->isVisible();
    mbvq::set_fill_active(false);
    QApplication::processEvents();
    const bool after = status != nullptr && status->text() == QStringLiteral("second") && sent.size() == 1 &&
                       sent[0] == QStringLiteral("second") && !slider->isVisible();
    mbvq_qt_soundings_status("outside", true);
    const bool outside = status != nullptr && status->text() == QStringLiteral("outside") && sent.size() == 2 &&
                         slider->isVisible();
    QObject::disconnect(connection);
    check(during && after && outside,
          "6d: SoundingsWidget::set_status during a QRhi fill sets the label, the signal and the sliders once, after "
          "the fill; outside a fill at once");
  }

  /* 7b: the profile mode that a QRhi fill hides from the core while it
     records the drawing (mbvq_c_soundings_plot_decorations) is the one set
     again after the fill. The View menu's profile items, chosen as the user
     chooses them (All, Good, None, All): after a frame of the frame driver
     the item chosen is still the one checked (the fill did not show the
     window the hidden mode), the core has the mode after that fill and
     after a fill refused while another batch is the target, and the item
     is checked again when the core asks the window to show its state
     (mb3dsoundings_updategui) */
  {
    const char *const texts[3] = {"Show No Profiles", "Show Good Profiles", "Show All Profiles"};
    const char *const names[3] = {"None", "Good", "All"};
    QAction *items[3] = {};
    for (QAction *action : soundings->findChildren<QAction *>())
      for (int mode = 0; mode < 3; mode++)
        if (action->isCheckable() && action->text() == QString::fromLatin1(texts[mode]))
          items[mode] = action;
    const auto checked = [&items, &names]() {
      for (int mode = 0; mode < 3; mode++)
        if (items[mode] != nullptr && items[mode]->isChecked())
          return std::string(names[mode]);
      return std::string("none");
    };
    bool kept = items[0] != nullptr && items[1] != nullptr && items[2] != nullptr;
    std::string seen;
    rhi::SoundingsFrame frame;
    for (const int mode : {MBS_VIEW_PROFILES_ALL, MBS_VIEW_PROFILES_UNFLAGGED, MBS_VIEW_PROFILES_NONE,
                           MBS_VIEW_PROFILES_ALL}) {
      if (!kept)
        break;
      items[mode]->trigger();
      rhi::SoundingsInput input;
      const bool filled = frame.begin(false, 400, 300, &input);
      const std::string after_fill = checked();
      const bool core = mb3dsoundings.view_profiles == mode && input.profiles == mode;
      TestBatch other;
      mbv_draw_set_target(&other.b);
      const bool refused = !frame.begin(false, 400, 300, &input);
      mbv_draw_set_target(nullptr);
      const bool core_refused = mb3dsoundings.view_profiles == mode && input.profiles == mode;
      mb3dsoundings_updategui();
      const std::string refreshed = checked();
      kept = kept && filled && after_fill == names[mode] && core && refused && core_refused && refreshed == names[mode];
      seen += (seen.empty() ? "" : ", ") + std::string(names[mode]) + ": " + after_fill + ", " + refreshed;
    }
    check(kept, "7b: the View menu's profile item chosen (All, Good, None, All) is still the one checked after a frame "
                "of the frame driver, whose fill hides the mode from the core; the core has the mode after that fill "
                "and after a refused one, and the item is checked when the core asks the window to show its state "
                "(the mode chosen: checked after the fill, after the core's request: " + seen + ")");
  }
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
}

/* mbvq::backend() and rhi_api() before main's set_backend(): what a host that
   never calls set_backend() draws with (set in main) */
mbvq::Backend initial_backend = mbvq::Backend::Null;
mbvq::RhiApi initial_api = mbvq::RhiApi::Vulkan;

int test_model() {
  /* the names of MB_RENDERER, and what the environment chooses */
  {
    using mbvq::Backend;
    using mbvq::RhiApi;
    struct Case {
      const char *value;
      bool known;
      Backend backend;
      RhiApi api;
    };
    const Case cases[] = {{"auto", true, Backend::Rhi, RhiApi::Default},
                          {"metal", true, Backend::Rhi, RhiApi::Metal},
                          {"d3d11", true, Backend::Rhi, RhiApi::Direct3D11},
                          {"d3d12", true, Backend::Rhi, RhiApi::Direct3D12},
                          {"vulkan", true, Backend::Rhi, RhiApi::Vulkan},
                          {"opengl", true, Backend::Rhi, RhiApi::OpenGL},
                          {"legacy-gl", true, Backend::LegacyGL, RhiApi::Default},
                          {"null", true, Backend::Null, RhiApi::Default},
                          {"", false, Backend::LegacyGL, RhiApi::Default},
                          {"Metal", false, Backend::LegacyGL, RhiApi::Default},
                          {"legacy_gl", false, Backend::LegacyGL, RhiApi::Default},
                          {"gl", false, Backend::LegacyGL, RhiApi::Default},
                          {"auto ", false, Backend::LegacyGL, RhiApi::Default}};
    bool parsed = true;
    for (const Case &c : cases) {
      Backend backend = Backend::Null;
      RhiApi api = RhiApi::Vulkan;
      const bool known = mbvq::parse_renderer(c.value, &backend, &api);
      parsed = parsed && known == c.known && (!known || (backend == c.backend && api == c.api));
    }
    Backend backend = Backend::Null;
    RhiApi api = RhiApi::Vulkan;
    parsed = parsed && !mbvq::parse_renderer(nullptr, &backend, &api);
    check(parsed, "parse_renderer knows auto, metal, d3d11, d3d12, vulkan, opengl, legacy-gl and null, and nothing else");

#if defined(__APPLE__)
    const bool apple = true;
#else
    const bool apple = false;
#endif
#if defined(_WIN32)
    const bool windows = true;
#else
    const bool windows = false;
#endif
#if QT_CONFIG(vulkan)
    const bool vulkan = true;
#else
    const bool vulkan = false;
#endif
    check(mbvq::rhi_api_possible(RhiApi::Default) && mbvq::rhi_api_possible(RhiApi::Metal) == apple &&
              mbvq::rhi_api_possible(RhiApi::Direct3D11) == windows &&
              mbvq::rhi_api_possible(RhiApi::Direct3D12) == windows && mbvq::rhi_api_possible(RhiApi::Vulkan) == vulkan,
          std::string("the APIs possible on this platform (Metal ") + (apple ? "yes" : "no") + ", Direct3D " +
              (windows ? "yes" : "no") + ", Vulkan " + (vulkan ? "yes" : "no") + ")");

    /* The default of this build and platform: QRhi with the platform's API
       on Apple; elsewhere the legacy OpenGL backend where it is built, else
       QRhi. The legacy OpenGL backend only where MBVQ_HAVE_GL is defined. */
#if defined(__APPLE__) || !defined(MBVQ_HAVE_GL)
    const Backend expected = Backend::Rhi;
#else
    const Backend expected = Backend::LegacyGL;
#endif
#ifdef MBVQ_HAVE_GL
    const bool legacy = true;
#else
    const bool legacy = false;
#endif
    const std::string platform = mbvq::rhi_api_name(RhiApi::Default);
    const std::string no_legacy = "mbview-qt: this build has no legacy OpenGL drawing (it needs -DbuildQt6LegacyGL=ON); "
                                  "the views are drawn with QRhi and " +
                                  platform;
    const std::string drawn = expected == Backend::Rhi ? "; the views are drawn with QRhi and " + platform
                                                       : std::string("; the views are drawn with OpenGL");
    const auto ends_with = [](const std::string &text, const std::string &tail) {
      return text.size() >= tail.size() && text.compare(text.size() - tail.size(), tail.size(), tail) == 0;
    };

    std::string message = "before";
    mbvq::choose_renderer(nullptr, &backend, &api, &message);
    bool chosen = backend == expected && api == RhiApi::Default && message.empty();
    message = "before";
    mbvq::choose_renderer("", &backend, &api, &message);
    chosen = chosen && backend == expected && api == RhiApi::Default && message.empty();
    check(chosen, std::string("MB_RENDERER unset or empty: the default of this build, ") +
                      (expected == Backend::Rhi ? "QRhi with the platform's API" : "the legacy OpenGL backend") +
                      ", without a message");
    check(initial_backend == expected && initial_api == RhiApi::Default,
          "before any set_backend(): the default of this build with the platform's API, what a host that never "
          "calls it draws with");
    mbvq::choose_renderer("auto", &backend, &api, &message);
    check(backend == Backend::Rhi && api == RhiApi::Default && message.empty(), "MB_RENDERER=auto: QRhi, the platform's API");
    mbvq::choose_renderer("legacy-gl", &backend, &api, &message);
    if (legacy)
      check(backend == Backend::LegacyGL && api == RhiApi::Default && message.empty(),
            "MB_RENDERER=legacy-gl: the legacy OpenGL backend, without a message");
    else
      check(backend == Backend::Rhi && api == RhiApi::Default && message == no_legacy,
            "MB_RENDERER=legacy-gl in a build without the legacy OpenGL backend: QRhi with the platform's API, and "
            "one line (" +
                message + ")");
    mbvq::choose_renderer("null", &backend, &api, &message);
    check(backend == Backend::Null && message.empty(), "MB_RENDERER=null: the null backend");
    mbvq::choose_renderer("nonsense", &backend, &api, &message);
    check(backend == expected && api == RhiApi::Default && message.find("not one of auto, metal") != std::string::npos &&
              ends_with(message, drawn),
          "an unknown MB_RENDERER: the default, and a line naming the values and what draws (" + message + ")");
    const char *impossible = apple ? "d3d11" : "metal";
    mbvq::choose_renderer(impossible, &backend, &api, &message);
    check(backend == Backend::Rhi && api == RhiApi::Default &&
              message.find("not possible on this platform") != std::string::npos &&
              message.find(apple ? "metal" : "opengl") != std::string::npos,
          std::string("MB_RENDERER=") + impossible + " here: QRhi with the platform's API, and one line (" + message + ")");
    mbvq::choose_renderer(apple ? "metal" : "opengl", &backend, &api, &message);
    check(backend == Backend::Rhi && api == (apple ? RhiApi::Metal : RhiApi::OpenGL) && message.empty(),
          "a possible API is taken as named");
    check(mbvq::backend_built(Backend::Rhi) && mbvq::backend_built(Backend::Null) &&
              mbvq::backend_built(Backend::LegacyGL) == legacy,
          std::string("this build has the QRhi and null backends, ") + (legacy ? "and" : "but not") +
              " the legacy OpenGL backend");

    /* what set_backend() makes of a request (resolve_backend): a backend this
       build lacks gives the other one and a line, a built one stays as asked
       without a line */
    Backend asked = Backend::LegacyGL;
    RhiApi asked_api = RhiApi::Default;
    std::string line = "before";
    mbvq::resolve_backend(&asked, &asked_api, &line);
    if (legacy)
      check(asked == Backend::LegacyGL && asked_api == RhiApi::Default && line.empty(),
            "set_backend(LegacyGL): the legacy OpenGL backend, without a line");
    else
      check(asked == Backend::Rhi && asked_api == RhiApi::Default && line == no_legacy,
            "set_backend(LegacyGL) in a build without it: QRhi with the platform's API, and one line (" + line + ")");
    bool kept = true;
    for (const RhiApi possible : {RhiApi::Default, apple ? RhiApi::Metal : RhiApi::OpenGL}) {
      asked = Backend::Rhi;
      asked_api = possible;
      line = "before";
      mbvq::resolve_backend(&asked, &asked_api, &line);
      kept = kept && asked == Backend::Rhi && asked_api == possible && line.empty();
    }
    asked = Backend::Null;
    asked_api = RhiApi::Default;
    line = "before";
    mbvq::resolve_backend(&asked, &asked_api, &line);
    kept = kept && asked == Backend::Null && line.empty();
    check(kept, "set_backend(Rhi) with the platform's API or a named one, and set_backend(Null): kept, without a line");
    /* set_backend() itself, where that changes nothing here (the line goes to
       stderr): in a build without the legacy OpenGL backend it stays QRhi */
    if (!legacy) {
      mbvq::set_backend(Backend::LegacyGL);
      check(mbvq::backend() == Backend::Rhi && mbvq::rhi_api() == RhiApi::Default,
            "set_backend(LegacyGL) in a build without it draws with QRhi and the platform's API");
    }
  }

  check(mbvq::backend() == mbvq::Backend::Rhi && mbvq_qt_drawable(), "under QRhi the core fills the map (glx_init)");

  /* the fill policy */
  check(rhi::fill_resolution(-1, MBVQ_REZ_NONE, false, false) == MBVQ_REZ_FULL &&
            rhi::fill_resolution(MBVQ_REZ_LOW, MBVQ_REZ_FULL, false, false) == MBVQ_REZ_FULL &&
            rhi::fill_resolution(MBVQ_REZ_LOW, MBVQ_REZ_LOW, true, true) == MBVQ_REZ_FULL,
        "fill policy: full resolution without contours, and with the full-resolution contours made");
  check(rhi::fill_resolution(-1, MBVQ_REZ_NONE, true, false) == MBVQ_REZ_LOW &&
            rhi::fill_resolution(MBVQ_REZ_HIGH, MBVQ_REZ_LOW, true, false) == MBVQ_REZ_HIGH &&
            rhi::fill_resolution(-1, MBVQ_REZ_HIGH, true, false) == MBVQ_REZ_HIGH &&
            rhi::fill_resolution(MBVQ_REZ_FULL, MBVQ_REZ_LOW, true, false) == MBVQ_REZ_FULL,
        "fill policy: the resolution asked for while the full-resolution contours are due");

  /* a grid of 3 x 2 tiles, with holes */
  Grid grid;
  grid.columns = 300;
  grid.rows = 200;
  size_t instance = MBV_NO_WINDOW;
  bool primary_marks = false;
  check(open_grid(grid, nullptr, &instance, &primary_marks), "a 300 x 200 grid opens under QRhi");
  check(primary_marks, "mbview_setprimarygrid (a new grid) raises the mask with the position and colour marks");
  struct mbview_struct *data = data_of(instance);
  struct mbvq_grid accessed;
  mbvq_c_grid(instance, &accessed);
  check(accessed.n_columns == 300 && accessed.n_rows == 200 && accessed.data == data->primary_data &&
            accessed.x == data->primary_x && accessed.nodata == data->primary_nodatavalue &&
            accessed.grid_mode == MBV_GRID_VIEW_PRIMARY && accessed.display_mode == MBV_DISPLAY_2D && !accessed.contours,
        "mbvq_c_grid gives the node arrays and the modes");
  {
    std::vector<unsigned char> valid(static_cast<size_t>(21) * 11);
    mbvq_c_node_validity(instance, 80, 120, 115, 135, 2, valid.data());
    bool same = true;
    for (int a = 0; a < 21; a++)
      for (int b = 0; b < 11; b++)
        same = same && (valid[static_cast<size_t>(a) * 11 + b] != 0) == !no_data(grid, 80 + 2 * a, 115 + 2 * b);
    check(same, "mbvq_c_node_validity marks the nodes with data at a stride, the holes not");
  }

  /* the frame driver gives the core the size of the drawing when it changed */
  rhi::MapFrame frame(instance);
  bool resized = frame.size_core(320, 240);
  const bool first = resized && data->width == 320 && data->height == 240;
  resized = frame.size_core(320, 240);
  const bool same = !resized;
  resized = frame.size_core(640, 480);
  check(first && same && resized && data->width == 640 && data->height == 480,
        "the frame driver resizes the core when the pixel size differs from the size it gave last, only then");

  /* a fill: the terrain slot and the eleven overlay groups; the last status
     text of the fill reaches the program once, after it (a text of its own
     shown before, so that the fill's last text is a new one) */
  {
    mbvq::MapWidget *widget = mbvq::map_widget(instance);
    widget->set_status(QStringLiteral("before the fill"));
    QApplication::processEvents();
    int during = 0, after = 0;
    bool inside = false, active = false;
    QString last;
    QObject::connect(widget, &mbvq::MapWidget::statusMessage, widget, [&](const QString &message) {
      (inside ? during : after)++;
      active = active || mbvq::fill_active();
      last = message;
    });
    inside = true;
    const bool filled = frame.fill();
    inside = false;
    QApplication::processEvents();
    const struct mbv_draw_batch &batch = frame.batch();
    int groups = 0, terrain = 0;
    for (int c = 0; c < batch.num_commands; c++) {
      groups += batch.commands[c].kind == MBV_DRAW_GROUP;
      terrain += batch.commands[c].kind == MBV_DRAW_SLOT && batch.commands[c].primitive == MBV_DRAW_SLOT_TERRAIN;
    }
    check(filled && !batch.failed && terrain == 1 && groups == 11 && mbvq_c_lastdrawrez(instance) == MBVQ_REZ_FULL,
          "a fill records the terrain slot and the eleven groups, at full resolution");
    check(during == 0 && after == 1 && !active && !last.isEmpty() && last != QStringLiteral("before the fill"),
          "the last status text of a fill reaches the program once, after it (" + std::to_string(after) +
              " after, " + std::to_string(during) + " during: \"" + last.toStdString() + "\")");
    struct mbv_draw_batch other;
    mbv_draw_batch_init(&other);
    mbv_draw_set_target(&other);
    const bool refused = !frame.fill();
    mbv_draw_set_target(nullptr);
    mbv_draw_batch_free(&other);
    check(refused && frame.refusals() == 1, "a fill is refused while another batch is the target");
    QObject::disconnect(widget, &mbvq::MapWidget::statusMessage, widget, nullptr);
  }

  /* the start of a frame, the same for a view and the offscreen renderer
     (MapFrame::begin): a frame that only continues uploads reuses the batch
     while nothing changed; a size given to the core before it, also by
     QRhiWidget::initialize() after a resize, a request or a risen mark make
     it fill; a refused fill keeps the new size for the next frame */
  {
    rhi::FrameInput input;
    const int fills = frame.fills();
    bool ok = frame.begin(false, 640, 480, &input) && input.filled && frame.fills() == fills + 1;
    ok = ok && frame.begin(true, 640, 480, &input) && !input.filled && frame.fills() == fills + 1;
    check(ok, "a frame that only continues uploads reuses the batch while nothing changed");
    frame.size_core(700, 500);
    ok = frame.begin(true, 700, 500, &input) && input.filled && frame.fills() == fills + 2 && !frame.resized();
    check(ok, "a size given to the core before the frame (initialize()) makes a continuing frame fill");
    frame.request(MBVQ_REZ_LOW);
    ok = frame.begin(true, 700, 500, &input) && input.filled && frame.fills() == fills + 3;
    int error = MB_ERROR_NO_ERROR;
    mbview_updateprimarygridcell(0, instance, 5, 5, data->primary_data[5 * grid.rows + 5] + 1.0f, &error);
    ok = ok && frame.begin(true, 700, 500, &input) && input.filled && frame.fills() == fills + 4;
    check(ok, "a request or a risen mark makes a continuing frame fill");
    struct mbv_draw_batch other;
    mbv_draw_batch_init(&other);
    mbv_draw_set_target(&other);
    frame.size_core(640, 480);
    const bool refused = !frame.begin(true, 640, 480, &input) && !input.filled && frame.resized();
    mbv_draw_set_target(nullptr);
    mbv_draw_batch_free(&other);
    ok = refused && frame.begin(true, 640, 480, &input) && input.filled && frame.fills() == fills + 5;
    check(ok, "a refused fill keeps the new size: the next frame fills");
  }

  /* the 3D view bounds of a fill without the renderer's bounds: the whole grid
     (mbview_viewbounds' fallback while a batch is filled; the null backend
     never plots, so only a backend that fills batches reaches it) */
  {
    const struct mbv_render_ops ops = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                       [](int, size_t, int) { return true; }, nullptr, nullptr};
    mbv_render_set_ops(&ops);
    mbvq_c_view(instance, MBVQ_VIEW_DISPLAY_3D, true);
    data->viewbounds[0] = 1;
    data->viewbounds[1] = 2;
    data->viewbounds[2] = 3;
    data->viewbounds[3] = 4;
    mbviews[instance].viewboundscount = MBV_BOUNDSFREQUENCY;
    struct mbv_draw_batch batch;
    mbv_draw_batch_init(&batch);
    mbv_draw_set_target(&batch);
    mbvq_c_plot(instance, MBVQ_REZ_LOW);
    mbv_draw_set_target(nullptr);
    mbv_draw_batch_free(&batch);
    check(data->viewbounds[0] == 0 && data->viewbounds[1] == grid.columns - 1 && data->viewbounds[2] == 0 &&
              data->viewbounds[3] == grid.rows - 1 && mbviews[instance].viewboundscount == 0,
          "a 3D low-resolution fill without the renderer's view bounds takes the whole grid");
    mbvq::rhi_backend::install();
    mbvq_c_view(instance, MBVQ_VIEW_DISPLAY_2D, true);
  }

  /* the terrain plan */
  {
    rhi::TerrainPlan plan;
    Planned first_plan = plan_frame(frame, plan);
    const std::vector<int> with = tiles_with_triangles(plan);
    check(plan.tiles().size() == 6 && first_plan.stats.tiles_prepared == static_cast<int>(with.size()) &&
              first_plan.tiles(0) == with && first_plan.tiles(1) == with && first_plan.tiles(2).size() == 6 &&
              !first_plan.more,
          "the first plan prepares and uploads every tile with triangles (" + std::to_string(with.size()) + " of " +
              std::to_string(plan.tiles().size()) + ")");
    Planned second = plan_frame(frame, plan);
    check(second.uploads.empty() && second.stats.tiles_prepared == 0, "the second plan uploads nothing");

    /* a pan with the middle button (the 2D Controls dialog would recolour the
       nodes as well, mbview_2dparmsapply) */
    struct mbv_input_event pan = {MBV_INPUT_PRESS, 2, 100, 100, 0};
    mbview_input(instance, &pan);
    pan.type = MBV_INPUT_MOTION;
    pan.x = 160;
    pan.y = 130;
    mbview_input(instance, &pan);
    pan.type = MBV_INPUT_RELEASE;
    mbview_input(instance, &pan);
    Planned camera = plan_frame(frame, plan);
    check(camera.uploads.empty(), "a pan (a camera change) uploads nothing");

    frame.size_core(500, 400);
    Planned resize = plan_frame(frame, plan);
    check(resize.uploads.empty(), "a resize uploads nothing");

    /* a cell edit on node column 128, the edge between the first and the second
       column of tiles, not across no data: the tiles meeting its 3 x 3 nodes
       gather positions and colours, not indices */
    int error = MB_ERROR_NO_ERROR;
    const int ei = 128, ej = 64;
    mbview_updateprimarygridcell(0, instance, ei, ej, data->primary_data[ei * grid.rows + ej] + 5.0f, &error);
    Planned edit = plan_frame(frame, plan);
    const std::vector<int> meeting = tiles_meeting(plan, ei - 1, ei + 1, ej - 1, ej + 1);
    check(meeting.size() == 2 && edit.tiles(0) == meeting && edit.tiles(1) == meeting && edit.tiles(2).empty(),
          "a cell edit gathers the positions and colours of the tiles meeting its nodes " + list(meeting) +
              ", no indices (" + list(edit.tiles(0)) + ")");
    /* the same cell across no data: their indices too */
    mbview_updateprimarygridcell(0, instance, ei, ej, data->primary_nodatavalue, &error);
    Planned across = plan_frame(frame, plan);
    check(across.tiles(2) == meeting && across.tiles(0) == meeting,
          "an edit across no data rebuilds the index lists of those tiles only (" + list(across.tiles(2)) + ")");
    mbview_updateprimarygridcell(0, instance, ei, ej, -900.0f, &error);
    plan_frame(frame, plan);
    /* a cell inside one tile */
    mbview_updateprimarygridcell(0, instance, 60, 60, data->primary_data[60 * grid.rows + 60] - 3.0f, &error);
    Planned inner = plan_frame(frame, plan);
    check(inner.tiles(1) == tiles_meeting(plan, 59, 61, 59, 61) && inner.tiles(1).size() == 1,
          "an edit inside a tile gathers that tile only");

    mbvq_c_view(instance, MBVQ_VIEW_COLORTABLE_BRIGHT, true);
    Planned colours = plan_frame(frame, plan);
    check(colours.tiles(1) == tiles_with_triangles(plan) && colours.tiles(0).empty() && colours.tiles(2).empty() &&
              colours.stats.bytes_positions == 0 && colours.stats.bytes_indices == 0,
          "a colour table change uploads colours only");

    mbvq_c_view(instance, MBVQ_VIEW_GRID_PRIMARYSLOPE, true);
    Planned mode = plan_frame(frame, plan);
    check(mode.tiles(2).size() == plan.tiles().size() && mode.tiles(1) == tiles_with_triangles(plan),
          "a grid mode change rebuilds every index list and gathers every colour");
    mbvq_c_view(instance, MBVQ_VIEW_GRID_PRIMARY, true);
    plan_frame(frame, plan);

    set_exaggeration(instance, 2.0);
    Planned exaggeration = plan_frame(frame, plan);
    check(exaggeration.tiles(0) == tiles_with_triangles(plan) && exaggeration.tiles(2).empty(),
          "an exaggeration change gathers every position, no index list");

    /* split uploads: a small budget spreads the first upload over frames, and
       a tile is drawn only once every stream of it is uploaded */
    rhi::TerrainPlan split;
    Planned part = plan_frame(frame, split, 600 * 1024);
    int frames = 1;
    int drawable_first = static_cast<int>(tiles_with_triangles(split).size());
    while (part.more && frames < 20) {
      part = plan_frame(frame, split, 600 * 1024);
      frames++;
    }
    check(frames > 1 && !part.more && drawable_first > 0 && drawable_first < static_cast<int>(with.size()) &&
              tiles_with_triangles(split) == with,
          "a 600 kB budget uploads the tiles over " + std::to_string(frames) + " frames, drawing only complete tiles");

    /* a grid of another size (a new primary grid; here the same plan meets
       another instance): the tiles are laid out again, and every one is
       uploaded again */
    Grid small;
    size_t third = MBV_NO_WINDOW;
    check(open_grid(small, nullptr, &third), "a 241 x 181 grid opens under QRhi");
    rhi::MapFrame frame3(third);
    frame3.size_core(400, 300);
    const int generation = plan.generation();
    Planned other = plan_frame(frame3, plan);
    const std::vector<rhi::TerrainPlan::Tile> layout = rhi::TerrainPlan::layout(small.columns, small.rows);
    bool laid_out = layout.size() == plan.tiles().size();
    for (size_t t = 0; laid_out && t < layout.size(); t++)
      laid_out = layout[t].i0 == plan.tiles()[t].i0 && layout[t].i1 == plan.tiles()[t].i1 &&
                 layout[t].j0 == plan.tiles()[t].j0 && layout[t].j1 == plan.tiles()[t].j1;
    check(plan.generation() > generation && laid_out && other.tiles(2).size() == layout.size() &&
              other.tiles(0) == tiles_with_triangles(plan) && other.tiles(1) == tiles_with_triangles(plan),
          "a grid of another size lays the tiles out again (" + std::to_string(layout.size()) +
              " tiles) and uploads every one");
    close_instance(third);
  }

  /* a tile without data has no triangle: an empty index list, nothing held;
     cell edits that give it a triangle make it drawn, and the edit that takes
     the last one empties it again */
  {
    Grid blank = grid;
    blank.empty_tile = true;
    size_t fourth = MBV_NO_WINDOW;
    check(open_grid(blank, nullptr, &fourth), "a 300 x 200 grid with a tile without data opens under QRhi");
    struct mbview_struct *data4 = data_of(fourth);
    rhi::MapFrame frame4(fourth);
    frame4.size_core(400, 300);
    rhi::TerrainPlan plan;
    Planned first4 = plan_frame(frame4, plan);
    const int t = 1; /* nodes i 0 to 128, j 128 to 199 */
    const auto upload_of = [&](const Planned &planned) -> const rhi::TileUpload * {
      for (const rhi::TileUpload &upload : planned.uploads)
        if (upload.tile == t)
          return &upload;
      return nullptr;
    };
    const rhi::TileUpload *empty = upload_of(first4);
    check(plan.tiles().size() == 6 && empty != nullptr && empty->indices && empty->index.empty() && !empty->positions &&
              !empty->colours && plan.tiles()[t].levels[0].empty && !plan.drawable(t) && tiles_with_triangles(plan).size() == 5,
          "a tile without data uploads an empty index list and nothing else, and is not drawn");
    int error = MB_ERROR_NO_ERROR;
    const int nodes[3][2] = {{10, 150}, {11, 150}, {10, 151}};
    for (const auto &node : nodes) {
      const double lon = data4->primary_xmin + node[0] * data4->primary_dx;
      const double lat = data4->primary_ymin + node[1] * data4->primary_dy;
      mbview_updateprimarygridcell(0, fourth, node[0], node[1], static_cast<float>(surface(lon, lat)), &error);
    }
    Planned triangle = plan_frame(frame4, plan);
    const rhi::TileUpload *one = upload_of(triangle);
    check(triangle.tiles(0) == std::vector<int>{t} && triangle.tiles(1) == std::vector<int>{t} &&
              triangle.tiles(2) == std::vector<int>{t} && one != nullptr && one->index.size() == 3 &&
              plan.drawable(t) && !plan.tiles()[t].levels[0].empty,
          "cell edits that give it a triangle gather its positions, colours and index list: it is drawn");
    mbview_updateprimarygridcell(0, fourth, 10, 151, data4->primary_nodatavalue, &error);
    Planned emptied = plan_frame(frame4, plan);
    const rhi::TileUpload *none = upload_of(emptied);
    check(emptied.tiles(2) == std::vector<int>{t} && none != nullptr && none->index.empty() && plan.tiles()[t].levels[0].empty &&
              !plan.drawable(t),
          "the edit that takes its last triangle empties it again");
    close_instance(fourth);
  }

  /* the index lists against a brute-force A/B list: primary grid mode, then
     the secondary grid mode with a secondary grid on the primary's nodes and
     with one apart */
  {
    int count = 0;
    check(indices_match(instance, frame, &count),
          "the index lists are the A/B triangles of the primary grid (" + std::to_string(count) + ")");
    check(add_secondary(instance, grid, grid.columns, grid.rows) && data->secondary_sameas_primary,
          "an overlay grid on the primary's nodes");
    mbvq_c_view(instance, MBVQ_VIEW_GRID_SECONDARY, true);
    check(data->grid_mode == MBV_GRID_VIEW_SECONDARY && indices_match(instance, frame, &count),
          "the index lists in the secondary grid mode, nodes shared (" + std::to_string(count) + ")");

    /* the mask raisers that come without colour: a cell edit stamps its blocks,
       here at the shared indices, so only the tiles there are rebuilt */
    rhi::TerrainPlan plan;
    plan_frame(frame, plan);
    int error = MB_ERROR_NO_ERROR;
    const int si = 200, sj = 150;
    uint64_t p0, c0, m0, d0, p1, c1, m1, d1, v0 = 0, v1 = 0;
    mbview_getmarks(0, instance, &p0, &c0, &m0, &d0, &error);
    mbview_getnodeversion(0, instance, 0, grid.columns - 1, 0, grid.rows - 1, &v0, &error);
    mbview_updatesecondarygridcell(0, instance, si, sj, data->secondary_nodatavalue, &error);
    mbview_getmarks(0, instance, &p1, &c1, &m1, &d1, &error);
    mbview_getnodeversion(0, instance, 0, grid.columns - 1, 0, grid.rows - 1, &v1, &error);
    Planned stamp = plan_frame(frame, plan);
    check(m1 > m0 && v1 > v0 && c1 == c0 && p1 == p0, "mbview_updatesecondarygridcell across no data: mask and blocks");
    check(stamp.tiles(2) == tiles_meeting(plan, si, si, sj, sj),
          "with the secondary grid on the primary's nodes only the tiles at its stamp are rebuilt (" +
              list(stamp.tiles(2)) + ")");
    mbvq_c_view(instance, MBVQ_VIEW_GRID_PRIMARY, true);

    /* the other raisers bring the colour mark (or every mark) with the mask;
       mbview_setprimarygrid, which brings every mark, is checked above */
    struct Raiser {
      const char *name;
      bool every; /* the position mark rises too */
      std::function<void()> raise;
    };
    std::vector<float> copy(data->primary_data, data->primary_data + static_cast<size_t>(grid.columns) * grid.rows);
    std::vector<float> secondary(data->secondary_data,
                                 data->secondary_data + static_cast<size_t>(grid.columns) * grid.rows);
    const Raiser raisers[] = {
        {"mbview_updateprimarygrid", false,
         [&] { mbview_updateprimarygrid(0, instance, grid.columns, grid.rows, copy.data(), &error); }},
        {"mbview_updatesecondarygrid", false,
         [&] { mbview_updatesecondarygrid(0, instance, grid.columns, grid.rows, secondary.data(), &error); }},
        {"mbview_setgridmode", false, [&] { mbvq_c_view(instance, MBVQ_VIEW_GRID_PRIMARYSLOPE, true); }},
        {"mbview_setsecondarygrid", false, [&] { add_secondary(instance, grid, grid.columns, grid.rows); }},
        {"mbview_setviewcontrols", true, [&] { set_view_controls(instance); }},
        {"mbview_setdisplayprojection", true, [&] { mbvq_c_setprojection(instance, 0); }}};
    for (const Raiser &raiser : raisers) {
      mbview_getmarks(0, instance, &p0, &c0, &m0, &d0, &error);
      raiser.raise();
      mbview_getmarks(0, instance, &p1, &c1, &m1, &d1, &error);
      Planned after = plan_frame(frame, plan);
      check(m1 > m0 && c1 > c0 && (!raiser.every || p1 > p0) && after.tiles(2).size() == plan.tiles().size() &&
                after.tiles(0) == tiles_with_triangles(plan),
            std::string(raiser.name) + " raises the mask with the colour mark" + (raiser.every ? " and every other" : "") +
                ": every index list is rebuilt");
    }
    /* mbview_updateprimarygridcell across no data: blocks, no colour */
    mbvq_c_view(instance, MBVQ_VIEW_GRID_PRIMARY, true);
    plan_frame(frame, plan);
    mbview_getmarks(0, instance, &p0, &c0, &m0, &d0, &error);
    mbview_updateprimarygridcell(0, instance, 20, 20, data->primary_nodatavalue, &error);
    mbview_getmarks(0, instance, &p1, &c1, &m1, &d1, &error);
    Planned cell = plan_frame(frame, plan);
    check(m1 > m0 && c1 == c0 && cell.tiles(2) == tiles_meeting(plan, 19, 21, 19, 21),
          "mbview_updateprimarygridcell across no data raises the mask with blocks: only its tiles are rebuilt");
  }

  /* a secondary grid apart from the primary's nodes */
  {
    Grid apart = grid;
    size_t second = MBV_NO_WINDOW;
    check(open_grid(apart, nullptr, &second) && add_secondary(second, apart, 157, 113) &&
              !data_of(second)->secondary_sameas_primary,
          "a second instance with an overlay grid apart from its nodes");
    rhi::MapFrame frame2(second);
    frame2.size_core(400, 300);
    mbvq_c_view(second, MBVQ_VIEW_GRID_SECONDARY, true);
    int count = 0;
    check(indices_match(second, frame2, &count),
          "the index lists in the secondary grid mode, grids apart (" + std::to_string(count) + ")");
    rhi::TerrainPlan plan;
    plan_frame(frame2, plan);
    int error = MB_ERROR_NO_ERROR;
    struct mbview_struct *data2 = data_of(second);
    mbview_updatesecondarygridcell(0, second, 40, 30, data2->secondary_nodatavalue, &error);
    Planned across = plan_frame(frame2, plan);
    check(across.tiles(2).size() == plan.tiles().size(),
          "grids apart, secondary grid mode: a secondary edit across no data rebuilds every index list");
    const std::vector<double> before = secondary_values(second);
    mbview_updatesecondarygridcell(0, second, 80, 60, 30.0f, &error);
    const std::vector<double> after = secondary_values(second);
    Planned colour = plan_frame(frame2, plan);
    check(colour.tiles(1) == tiles_with_triangles(plan) && colour.stats.tiles_prepared ==
                                                                static_cast<int>(tiles_with_triangles(plan).size()) &&
              colour.tiles(2).empty(),
          "grids apart, secondary grid mode: a secondary edit not across no data prepares every tile again and "
          "gathers its colours");
    /* the edited place has its new colour, and nothing else changed: the
       colours of every tile before the edit (gathered by the edit across no
       data) against those after it */
    {
      std::map<int, const rhi::TileUpload *> earlier;
      for (const rhi::TileUpload &upload : across.uploads)
        if (upload.colours)
          earlier[upload.tile] = &upload;
      const int nr = data2->primary_n_rows;
      int edited = 0, recoloured = 0, others = 0, missing = 0;
      for (const rhi::TileUpload &upload : colour.uploads) {
        if (!upload.colours)
          continue;
        const auto found = earlier.find(upload.tile);
        if (found == earlier.end()) {
          missing++;
          continue;
        }
        const rhi::TerrainPlan::Tile &tile = plan.tiles()[upload.tile];
        for (int a = 0; a < tile.ni; a++)
          for (int b = 0; b < tile.nj; b++) {
            const size_t n = 4 * (static_cast<size_t>(a) * tile.nj + b);
            const size_t k = static_cast<size_t>(tile.i0 + a) * nr + tile.j0 + b;
            const bool changed = std::memcmp(&upload.rgba[n], &found->second->rgba[n], 4) != 0;
            if (before[k] != after[k]) {
              edited++;
              recoloured += changed;
            }
            else {
              others += changed;
            }
          }
      }
      check(edited > 0 && recoloured == edited && others == 0 && missing == 0,
            "grids apart, secondary grid mode: the place of the secondary edit has its new colour (" +
                std::to_string(recoloured) + " of " + std::to_string(edited) + " node copies), nothing else (" +
                std::to_string(others) + ")");
    }

    /* Nodes the preparation leaves as they were in the secondary grid mode
       (primary data, no secondary data) come into use with the mask: their
       copies are gathered again with it, so no drawn node keeps a position
       older than the marks. An exaggeration change in the secondary grid
       mode, then the primary grid mode; then the overlay grid's hole filled. */
    {
      rhi::TerrainPlan fresh;
      GpuCopy copy;
      copy.apply(fresh, plan_frame(frame2, fresh));
      set_exaggeration(second, 3.0);
      copy.apply(fresh, plan_frame(frame2, fresh));
      mbvq_c_view(second, MBVQ_VIEW_GRID_PRIMARY, true);
      copy.apply(fresh, plan_frame(frame2, fresh));
      const int primary = copy.stale(second, fresh);
      mbvq_c_view(second, MBVQ_VIEW_GRID_SECONDARY, true);
      copy.apply(fresh, plan_frame(frame2, fresh));
      set_exaggeration(second, 1.5);
      copy.apply(fresh, plan_frame(frame2, fresh));
      const bool filled = fill_secondary(second, 157, 113);
      copy.apply(fresh, plan_frame(frame2, fresh));
      const int update = copy.stale(second, fresh);
      check(primary == 0 && filled && update == 0,
            "nodes that come into use with the mask are drawn with positions as new as the marks (secondary grid mode, "
            "exaggeration, primary grid mode: " +
                std::to_string(primary) + " stale; then the overlay grid's hole filled: " + std::to_string(update) + ")");
    }
    close_instance(second);
  }

  close_instance(instance);

  /* 6b: levels, culling and the budgets */
  model_geometry();
  model_levels();
  model_edit_beside();
  model_oblique();
  model_drag_budget();
  model_costs();
  model_split();
  model_split_complete();
  model_memory();
  model_spheroid();
  model_style();
  model_batch_plan();
  model_glu();
  model_map_6c();
  /* 6d: the profile and 3D soundings views */
  model_soundings_plan();
  model_soundings_frame();
  model_profile_frame();
  /* 7a: the soundings bench's files (also of unequal sizes) and tile order */
  model_split_files(120, 3);
  model_split_files(122, 4);
  model_tile_order();
  /* 7b: a frame's drawing of the 3D soundings against a fill with the
     profile mode set, the core's batch at two sizes, and the profile lines
     of soundings from three files */
  model_soundings_decorations();
  model_soundings_fill_size();
  model_profile_files();
  /* 7c: the rule of the GPU-time fallback and the map's stride loop with
     its marks */
  model_gpu_fallback();
  model_gpu_marks();
  /* 7d1: how the soundings' position and colour chunks are made (7d-14) */
  model_chunk_use();
  return result();
}

/*--------------------------------------------------------------------*/
/* counts the paint events of a widget, as its event filter */
class PaintCount : public QObject {
 public:
  int paints = 0;
  bool eventFilter(QObject *object, QEvent *event) override {
    if (event->type() == QEvent::Paint)
      paints++;
    return QObject::eventFilter(object, event);
  }
};

int test_widgets() {
  /* the checks need a platform where no QRhi can be made */
  if (QGuiApplication::platformName() != QLatin1String("offscreen")) {
    std::printf("skipped: --widgets needs the offscreen platform, where no QRhi can be made (this is %s)\n",
                qPrintable(QGuiApplication::platformName()));
    return 77;
  }
  Grid grid;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "mbview_open under QRhi");
  QWidget *view = mbvq::map_view(instance);
  auto *rhi_widget = qobject_cast<QRhiWidget *>(view);
#if defined(__APPLE__)
  const QRhiWidget::Api expected = QRhiWidget::Api::Metal;
#elif defined(_WIN32)
  const QRhiWidget::Api expected = QRhiWidget::Api::Direct3D11;
#else
  const QRhiWidget::Api expected = QRhiWidget::Api::OpenGL;
#endif
  check(rhi_widget != nullptr && rhi_widget->api() == expected && mbvq::view_state(view).rhi_view,
        "the map view is a QRhiWidget with the platform's API set explicitly");
  check(mbv_render_defer_draw(MBV_RENDER_MAP, instance, MBVQ_REZ_LOW) &&
            mbv_render_defer_draw(MBV_RENDER_MAP, MBVQ_MAX_WINDOWS - 1, MBVQ_REZ_FULL) &&
            mbv_render_defer_draw(MBV_RENDER_PROFILE, instance, 0) && mbv_render_defer_draw(MBV_RENDER_SOUNDINGS, 0, 0),
        "every drawing is deferred, also for views without a widget");

  /* on the offscreen platform no QRhi can be made: the view says so */
  QElapsedTimer timer;
  timer.start();
  while (!mbvq::view_state(view).failure_shown && timer.elapsed() < 3000)
    spin(20);
  const mbvq::ViewState failed = mbvq::view_state(view);
  check(failed.failure_shown && failed.failures >= 1 && !failed.drawn && mbvq::failure_messages() == 1,
        "renderFailed: a label on the view and one line on stderr (label " + std::to_string(failed.failure_shown) +
            ", " + std::to_string(failed.failures) + " failures, " + std::to_string(mbvq::failure_messages()) +
            " lines)");
  /* the label (the line's text) names MB_RENDERER=legacy-gl only where the
     legacy OpenGL backend is built */
  {
#ifdef MBVQ_HAVE_GL
    const QString advice = QStringLiteral("; MB_RENDERER=legacy-gl draws them with OpenGL");
#else
    const QString advice = QStringLiteral("; this build has no legacy OpenGL drawing");
#endif
    const auto *label = view->findChild<QLabel *>(QString(), Qt::FindDirectChildrenOnly);
    const QString text = label != nullptr ? label->text() : QString();
    check(text == QStringLiteral("mbview-qt: the views could not be drawn with QRhi (") +
                      QString::fromLatin1(mbvq::rhi_api_name(mbvq::RhiApi::Default)) + QLatin1Char(')') + advice,
          "the label says what this build can draw with instead (" + text.toStdString() + ")");
  }
  size_t second = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &second), "a second map under QRhi");
  QWidget *view2 = mbvq::map_view(second);
  timer.restart();
  while (!mbvq::view_state(view2).failure_shown && timer.elapsed() < 3000)
    spin(20);
  check(mbvq::view_state(view2).failure_shown && mbvq::failure_messages() == 1,
        "each failing view shows its label, the line comes once per process");
  close_instance(second);

  /* Input before the first frame does not reach the core. After the
     release the view asks for a frame itself, which fills and plans without
     the drag budget (CK-01): the core asks for none after most releases. */
  struct mbview_struct *data = data_of(instance);
  data->pick_type = MBV_PICK_NONE;
  const mbvq::ViewState idle = mbvq::view_state(view);
  const QPointF centre(view->width() / 2.0, view->height() / 2.0);
  QMouseEvent press(QEvent::MouseButtonPress, centre, view->mapToGlobal(centre), Qt::LeftButton, Qt::LeftButton,
                    Qt::NoModifier);
  QApplication::sendEvent(view, &press);
  const mbvq::ViewState down = mbvq::view_state(view);
  /* the view's buttons are also the state of the process that every map's
     fill fallback reads (rhi::map_buttons_down) */
  bool shared = rhi::map_buttons_down();
  QMouseEvent release(QEvent::MouseButtonRelease, centre, view->mapToGlobal(centre), Qt::LeftButton, Qt::NoButton,
                      Qt::NoModifier);
  QApplication::sendEvent(view, &release);
  const mbvq::ViewState up = mbvq::view_state(view);
  shared = shared && !rhi::map_buttons_down();
  check(data->pick_type == MBV_PICK_NONE && down.buttons == 1 && up.buttons == 0,
        "a click before the first frame picks nothing; the button is tracked");
  check(!idle.fill_due && !down.fill_due && down.frame_requests == idle.frame_requests && up.fill_due &&
            up.frame_requests == idle.frame_requests + 1 && up.levels_kept,
        "after the last release the view asks for a frame (" + std::to_string(up.frame_requests - idle.frame_requests) +
            "), and that frame fills (CK-01)");

  /* a lost release: the button state is cleared, and a view that is shown
     asks for the frame after it */
  bool cleared = true;
  const int asked = mbvq::view_state(view).frame_requests;
  for (QEvent::Type type : {QEvent::UngrabMouse, QEvent::Hide, QEvent::WindowDeactivate}) {
    QMouseEvent middle(QEvent::MouseButtonPress, centre, view->mapToGlobal(centre), Qt::MiddleButton, Qt::MiddleButton,
                       Qt::NoModifier);
    QApplication::sendEvent(view, &middle);
    const bool held = mbvq::view_state(view).buttons == 2;
    shared = shared && rhi::map_buttons_down();
    QEvent lost(type);
    QApplication::sendEvent(view, &lost);
    cleared = cleared && held && mbvq::view_state(view).buttons == 0;
    shared = shared && !rhi::map_buttons_down();
  }
  const mbvq::ViewState lost = mbvq::view_state(view);
  check(cleared && lost.frame_requests == asked + 3 && lost.fill_due,
        "UngrabMouse, Hide and WindowDeactivate clear the mouse buttons; the view, shown, asks for a filled frame "
        "after each (" + std::to_string(lost.frame_requests - asked) + ")");

  /* Releases lost without those: to a modal window (WindowBlocked) and to a
     popup, which takes the focus (FocusOut, Qt::PopupFocusReason), clear the
     buttons, in the view and in the state of the process; a focus lost for
     another reason does not. A release lost to another widget's grab sends
     the view nothing: its next mouse event, a press of another button whose
     buttons() no longer has the middle one, drops that button, as does a
     press the core does not take or a move. The buttons follow what each
     mouse event reports: a button cleared while it is still down comes back
     with the next one that reports it. */
  {
    const auto press_button = [&](Qt::MouseButton button) {
      QMouseEvent event(QEvent::MouseButtonPress, centre, view->mapToGlobal(centre), button, button, Qt::NoModifier);
      QApplication::sendEvent(view, &event);
    };
    /* a mouse event of the button, reporting these buttons down */
    const auto send_mouse = [&](QEvent::Type type, Qt::MouseButton button, Qt::MouseButtons buttons) {
      QMouseEvent event(type, centre, view->mapToGlobal(centre), button, buttons, Qt::NoModifier);
      QApplication::sendEvent(view, &event);
    };
    bool blocked_popup = true;
    QEvent blocked(QEvent::WindowBlocked);
    QFocusEvent popup(QEvent::FocusOut, Qt::PopupFocusReason);
    for (QEvent *lost_by : {&blocked, static_cast<QEvent *>(&popup)}) {
      press_button(Qt::MiddleButton);
      const bool held = mbvq::view_state(view).buttons == 2 && rhi::map_buttons_down();
      QApplication::sendEvent(view, lost_by);
      blocked_popup = blocked_popup && held && mbvq::view_state(view).buttons == 0 && !rhi::map_buttons_down();
    }
    press_button(Qt::MiddleButton);
    QFocusEvent tab(QEvent::FocusOut, Qt::TabFocusReason);
    QApplication::sendEvent(view, &tab);
    const bool kept = mbvq::view_state(view).buttons == 2 && rhi::map_buttons_down();
    check(blocked_popup && kept, "6c: WindowBlocked and FocusOut for a popup clear the mouse buttons, in the view and "
                                 "in the state of the process; FocusOut for another reason keeps them");
    /* the middle button's release went elsewhere; the left button is pressed */
    press_button(Qt::LeftButton);
    const int pressed = mbvq::view_state(view).buttons;
    const bool buttons_down = rhi::map_buttons_down();
    send_mouse(QEvent::MouseButtonRelease, Qt::LeftButton, Qt::NoButton);
    check(kept && pressed == 1 && buttons_down && mbvq::view_state(view).buttons == 0 && !rhi::map_buttons_down(),
          "6c: a release lost to another widget's grab is noticed at the view's next mouse event: a press of the "
          "left button whose buttons() lack the middle one leaves the left button alone down (" +
              std::to_string(pressed) + "), and its release none, in the view and in the state of the process");

    /* The middle button's release lost again: a press of XButton1, which the
       core does not take (translate_event rejects it), and a move drop it
       too. */
    press_button(Qt::MiddleButton);
    send_mouse(QEvent::MouseButtonPress, Qt::XButton1, Qt::XButton1);
    const int extra = mbvq::view_state(view).buttons;
    const bool extra_down = rhi::map_buttons_down();
    send_mouse(QEvent::MouseButtonRelease, Qt::XButton1, Qt::NoButton);
    press_button(Qt::MiddleButton);
    send_mouse(QEvent::MouseMove, Qt::NoButton, Qt::LeftButton);
    const int moved = mbvq::view_state(view).buttons;
    const bool moved_down = rhi::map_buttons_down();
    send_mouse(QEvent::MouseButtonRelease, Qt::LeftButton, Qt::NoButton);
    check(extra == 0 && !extra_down && moved == 1 && moved_down && mbvq::view_state(view).buttons == 0 &&
              !rhi::map_buttons_down(),
          "6c: a release lost to another widget's grab is noticed also at a press that the core does not take and at "
          "a move: a press of XButton1 whose buttons() lack the middle button leaves none down (" +
              std::to_string(extra) + "); a move whose buttons() report the left button alone leaves that one alone "
              "down (" + std::to_string(moved) + "), and its release none, in the view and in the state of the "
              "process");

    /* A popup that closes again while the middle button stays down, the drag
       going on in the view: the popup's FocusOut clears the button, the next
       move that reports it brings it back, and its release clears it. */
    press_button(Qt::MiddleButton);
    QApplication::sendEvent(view, &popup);
    const bool popup_cleared = mbvq::view_state(view).buttons == 0 && !rhi::map_buttons_down();
    send_mouse(QEvent::MouseMove, Qt::NoButton, Qt::MiddleButton);
    const int back = mbvq::view_state(view).buttons;
    const bool back_down = rhi::map_buttons_down();
    send_mouse(QEvent::MouseButtonRelease, Qt::MiddleButton, Qt::NoButton);
    check(popup_cleared && back == 2 && back_down && mbvq::view_state(view).buttons == 0 && !rhi::map_buttons_down(),
          "6c: a drag that goes on after a popup closed: FocusOut for the popup clears the middle button, a move that "
          "reports it brings it back (" + std::to_string(back) + "), in the view and in the state of the process, "
          "and its release clears it");

    /* the event's own press or release counts also where its buttons()
       disagree, as in a synthetic event */
    send_mouse(QEvent::MouseButtonPress, Qt::RightButton, Qt::NoButton);
    const int own_press = mbvq::view_state(view).buttons;
    const bool own_down = rhi::map_buttons_down();
    send_mouse(QEvent::MouseButtonRelease, Qt::RightButton, Qt::RightButton);
    check(own_press == 4 && own_down && mbvq::view_state(view).buttons == 0 && !rhi::map_buttons_down(),
          "6c: a mouse event's own press or release counts also where its buttons() disagree: a press of the right "
          "button whose buttons() report none leaves it down (" + std::to_string(own_press) +
              "), its release whose buttons() still report it none, in the view and in the state of the process");
  }

  /* Hidden for real with a button down: no frame asked for (a hidden view
     draws nothing), and its tile levels are not kept from other views'
     evictions until it is shown again. */
  {
    QEvent shown(QEvent::Show);
    QApplication::sendEvent(view, &shown);
    const bool kept = mbvq::view_state(view).levels_kept;
    QMouseEvent middle(QEvent::MouseButtonPress, centre, view->mapToGlobal(centre), Qt::MiddleButton, Qt::MiddleButton,
                       Qt::NoModifier);
    QApplication::sendEvent(view, &middle);
    const int before = mbvq::view_state(view).frame_requests;
    shared = shared && rhi::map_buttons_down();
    view->hide();
    const mbvq::ViewState hidden = mbvq::view_state(view);
    shared = shared && !rhi::map_buttons_down();
    view->show();
    const mbvq::ViewState again = mbvq::view_state(view);
    check(kept && hidden.buttons == 0 && hidden.frame_requests == before && hidden.fill_due && !hidden.levels_kept &&
              again.levels_kept,
          "hidden with a button down: the buttons cleared, no frame asked for (" +
              std::to_string(hidden.frame_requests - before) + "), its levels not kept; shown again, they are");
  }

  /* The status label and a QRhi fill: the label (in the window's status
     bar) keeps its text while the fill runs and until events are processed,
     then shows the fill's last text; a last text equal to its own changes
     nothing, so the label does not repaint the window between two frames of
     the view. The status signal sends the fill's last text once, after the
     fill; a host's status bar showing it ignores a text equal to its own, so
     it does not repaint either, and a bar the host cleared shows the text
     again. */
  {
    mbvq::MapWidget *widget = mbvq::map_widget(instance);
    const QString before = QStringLiteral("before the fill");
    widget->set_status(before);
    QLabel *label = nullptr;
    for (QLabel *candidate : mbvq::window(instance)->findChildren<QLabel *>())
      if (candidate->text() == before && candidate->isVisible())
        label = candidate;
    if (label == nullptr)
      check(false, "6c: the status label is shown in the map's window and takes a text at once outside a fill");
    else {
      PaintCount paints;
      label->installEventFilter(&paints);
      std::vector<QString> sent;
      const QMetaObject::Connection connection = QObject::connect(
          widget, &mbvq::MapWidget::statusMessage, widget, [&sent](const QString &text) { sent.push_back(text); });
      /* a fill with two texts, the last a new one */
      spin(100);
      paints.paints = 0;
      mbvq::set_fill_active(true);
      widget->set_status(QStringLiteral("Drawing full rez..."));
      const QString during = label->text();
      widget->set_status(QStringLiteral("Done."));
      mbvq::set_fill_active(false);
      const QString after_fill = label->text();
      const size_t sent_in_fill = sent.size();
      QApplication::processEvents();
      const QString after_events = label->text();
      spin(100);
      const int changed_paints = paints.paints;
      check(during == before && after_fill == before && after_events == QStringLiteral("Done.") && sent_in_fill == 0 &&
                sent.size() == 1 && sent.back() == QStringLiteral("Done.") && changed_paints >= 1,
            "6c: in a fill the status label keeps its text (\"" + during.toStdString() + "\", \"" +
                after_fill.toStdString() + "\") and shows the last one after it (\"" + after_events.toStdString() +
                "\", " + std::to_string(changed_paints) + " paints); the last text signalled once after the fill (" +
                std::to_string(sent_in_fill) + " in it, " + std::to_string(sent.size()) + " in all)");
      /* fills whose last text the label shows already: the same texts as
         above, then two real full fills of the map, the second one with the
         texts of the first; the label does not repaint, and the signal sends
         each fill's last text once, after it */
      spin(100);
      paints.paints = 0;
      sent.clear();
      mbvq::set_fill_active(true);
      widget->set_status(QStringLiteral("Drawing full rez..."));
      const QString during_same = label->text();
      widget->set_status(QStringLiteral("Done."));
      mbvq::set_fill_active(false);
      const size_t same_in_fill = sent.size();
      QApplication::processEvents();
      spin(100);
      const int same_paints = paints.paints;
      const size_t same_sent = sent.size();
      const bool same_last = same_sent == 1 && sent.back() == QStringLiteral("Done.");
      rhi::MapFrame frame(instance);
      frame.size_core(320, 240);
      bool filled = frame.fill();
      QApplication::processEvents();
      spin(100);
      const QString shown = label->text();
      paints.paints = 0;
      sent.clear();
      filled = filled && frame.fill();
      const QString during_fill = label->text();
      const size_t fill_in_fill = sent.size();
      QApplication::processEvents();
      spin(100);
      const int fill_paints = paints.paints;
      check(during_same == QStringLiteral("Done.") && label->text() == QStringLiteral("Done.") && same_paints == 0 &&
                same_in_fill == 0 && same_last && filled && during_fill == shown && label->text() == shown &&
                fill_in_fill == 0 && sent.size() == 1 && sent.back() == shown && fill_paints == 0,
            "6c: a fill whose last text the label shows already does not repaint it (" + std::to_string(same_paints) +
                " paints) and signals that text once, after the fill (" + std::to_string(same_in_fill) + " in it, " +
                std::to_string(same_sent) + " in all); a second full fill (the last text \"" + shown.toStdString() +
                "\"): " + std::to_string(fill_paints) + " paints, " + std::to_string(fill_in_fill) + " in it, " +
                std::to_string(sent.size()) + " in all");

      /* a text set outside a fill after a fill's texts, before the update
         after the fill ran, stays: the label shows it, and the signal sent it
         last, with no text of the fill after it */
      sent.clear();
      mbvq::set_fill_active(true);
      widget->set_status(QStringLiteral("a text of the fill"));
      mbvq::set_fill_active(false);
      widget->set_status(QStringLiteral("a later text"));
      QApplication::processEvents();
      spin(50);
      std::string sent_texts;
      for (const QString &message : sent)
        sent_texts += (sent_texts.empty() ? "\"" : ", \"") + message.toStdString() + "\"";
      check(label->text() == QStringLiteral("a later text") && sent.size() == 1 &&
                sent.back() == QStringLiteral("a later text"),
            "6c: a text set outside a fill after the fill's texts stays: the label shows \"" +
                label->text().toStdString() + "\", the signal sent " + (sent.empty() ? "nothing" : sent_texts));

      /* A host that shows the status in its own status bar, as a program
         that places the map in its window does: each fill sends its last
         text once, after the fill, and the bar, which ignores a text equal
         to its own, does not repaint between two frames of the view when
         that is the text it shows; a fill whose last text is new shows it.
         A bar the host cleared, as a message that timed out or a status tip
         leaves it, shows the next fill's last text again. */
      {
        QMainWindow host;
        QStatusBar *bar = host.statusBar();
        const QMetaObject::Connection to_host = QObject::connect(
            widget, &mbvq::MapWidget::statusMessage, bar, [bar](const QString &message) { bar->showMessage(message); });
        widget->set_status(QStringLiteral("Done."));
        host.resize(400, 120);
        host.show();
        spin(100);
        PaintCount bar_paints;
        bar->installEventFilter(&bar_paints);
        /* the texts of a full fill, then two real full fills, each ending
           with the text the bar shows */
        bool host_filled = true;
        int once_after = 0;
        for (int f = 0; f < 3; f++) {
          const QString showing = bar->currentMessage();
          sent.clear();
          if (f == 0) {
            mbvq::set_fill_active(true);
            widget->set_status(QStringLiteral("Drawing full rez..."));
            widget->set_status(QStringLiteral("Done."));
            mbvq::set_fill_active(false);
          }
          else
            host_filled = host_filled && frame.fill();
          const size_t in_fill = sent.size();
          QApplication::processEvents();
          once_after += in_fill == 0 && sent.size() == 1 && sent.back() == showing;
        }
        spin(100);
        const int shown_paints = bar_paints.paints;
        /* a text from outside a fill, sent at once; then a real full fill */
        sent.clear();
        widget->set_status(QStringLiteral("before the fill"));
        const size_t outside_sent = sent.size();
        spin(100);
        bar_paints.paints = 0;
        host_filled = host_filled && frame.fill();
        const size_t in_fill_sent = sent.size();
        QApplication::processEvents();
        spin(100);
        const QString last = bar->currentMessage();
        check(host_filled && once_after == 3 && shown_paints == 0 && outside_sent == 1 && in_fill_sent == 1 &&
                  sent.size() == 2 && sent.back() == label->text() && last == label->text() &&
                  last != QStringLiteral("before the fill") && bar_paints.paints >= 1,
              "6c: a host's status bar showing the status signal: fills whose last text it shows already send that "
              "text once, after the fill (" +
                  std::to_string(once_after) + " of 3 fills), and do not repaint the bar (" +
                  std::to_string(shown_paints) +
                  " paints); a full fill after a text from outside a fill sends its last text once, after the fill (" +
                  std::to_string(in_fill_sent - outside_sent) + " in it, " +
                  std::to_string(sent.size() - in_fill_sent) + " after it; the bar shows \"" + last.toStdString() +
                  "\", " + std::to_string(bar_paints.paints) + " paints)");
        /* the bar cleared by the host, then a real full fill */
        bar->clearMessage();
        const QString cleared = bar->currentMessage();
        sent.clear();
        host_filled = host_filled && frame.fill();
        const size_t cleared_in_fill = sent.size();
        QApplication::processEvents();
        spin(50);
        const QString again = bar->currentMessage();
        check(host_filled && cleared.isEmpty() && cleared_in_fill == 0 && sent.size() == 1 && !again.isEmpty() &&
                  again == label->text(),
              "6c: a host's status bar cleared (as a message that timed out or a status tip leaves it) shows the "
              "last text of the next fill again (\"" +
                  cleared.toStdString() + "\", then \"" + again.toStdString() + "\"; " +
                  std::to_string(cleared_in_fill) + " texts in the fill, " +
                  std::to_string(sent.size() - cleared_in_fill) + " after it)");
        QObject::disconnect(to_host);
        bar->removeEventFilter(&bar_paints);
      }
      QObject::disconnect(connection);
      label->removeEventFilter(&paints);
    }
  }

  /* 6d: the profile and soundings views are QRhi views (this replaces the
     check of 6a that the profile view is not a QRhi view yet); 7b: the
     profile mode after fills that hide it, in the View menu */
  widgets_views(instance, expected);

  /* closing the window deletes the view that never had a QRhi; a button
     down in it goes with it */
  {
    QMouseEvent middle(QEvent::MouseButtonPress, centre, view->mapToGlobal(centre), Qt::MiddleButton, Qt::MiddleButton,
                       Qt::NoModifier);
    QApplication::sendEvent(view, &middle);
    shared = shared && rhi::map_buttons_down();
  }
  close_instance(instance);
  check(mbvq::window(instance) == nullptr, "a QRhi view that never drew is closed and deleted");
  check(shared && !rhi::map_buttons_down(),
        "6c: a map view's buttons are the state that every map's fill fallback reads: down from a press, up after "
        "the release, after UngrabMouse, Hide and WindowDeactivate, when hidden, and when its window closed with a "
        "button down");

  /* a map view deleted while it is shown, so that no hide comes before (a
     view of an instance without a map, on its own): a button down in it
     goes with it */
  {
    QWidget *alone = mbvq::rhi_backend::create_map_view(MBVQ_MAX_WINDOWS - 1, nullptr);
    alone->resize(200, 150);
    alone->show();
    spin(20);
    const QPointF point(100.0, 75.0);
    QMouseEvent middle(QEvent::MouseButtonPress, point, alone->mapToGlobal(point), Qt::MiddleButton, Qt::MiddleButton,
                       Qt::NoModifier);
    QApplication::sendEvent(alone, &middle);
    const bool held = alone->isVisible() && mbvq::view_state(alone).buttons == 2 && rhi::map_buttons_down();
    delete alone;
    check(held && !rhi::map_buttons_down(),
          "6c: a map view deleted while shown with a button down takes the button out of the state of the process");
  }

  /* When the last map button of the process goes up, the other map views
     whose last fill fell back ask for a frame that fills: the core asks for
     none after a lost release. Two map views of instances without a map,
     shown: a click in A asks nothing of B while B's last fill did not fall
     back. Once it did, a drag in A asks nothing of B while a button is down
     (its motions, and a second button pressed and released while the first
     is held), and a frame that fills after the last release. */
  {
    QWidget *a = mbvq::rhi_backend::create_map_view(MBVQ_MAX_WINDOWS - 1, nullptr);
    QWidget *b = mbvq::rhi_backend::create_map_view(MBVQ_MAX_WINDOWS - 2, nullptr);
    for (QWidget *shown : {a, b}) {
      shown->resize(200, 150);
      shown->show();
    }
    spin(20);
    /* a mouse event in A, dx pixels right of its centre */
    const auto mouse_a = [&](QEvent::Type type, Qt::MouseButton button, Qt::MouseButtons buttons, double dx) {
      const QPointF at(100.0 + dx, 75.0);
      QMouseEvent event(type, at, a->mapToGlobal(at), button, buttons, Qt::NoModifier);
      QApplication::sendEvent(a, &event);
    };
    const mbvq::ViewState before = mbvq::view_state(b);
    mouse_a(QEvent::MouseButtonPress, Qt::LeftButton, Qt::LeftButton, 0.0);
    mouse_a(QEvent::MouseButtonRelease, Qt::LeftButton, Qt::NoButton, 0.0);
    const mbvq::ViewState full = mbvq::view_state(b);
    mbvq::set_fell_back(b, true);
    /* the drag; B's frame requests after each step before the last release */
    bool quiet = true;
    std::string steps;
    const auto drag_step = [&](QEvent::Type type, Qt::MouseButton button, Qt::MouseButtons buttons, double dx) {
      mouse_a(type, button, buttons, dx);
      const mbvq::ViewState during = mbvq::view_state(b);
      quiet = quiet && during.frame_requests == full.frame_requests && during.fill_due == full.fill_due &&
              rhi::map_buttons_down();
      steps += " " + std::to_string(during.frame_requests - full.frame_requests);
    };
    drag_step(QEvent::MouseButtonPress, Qt::LeftButton, Qt::LeftButton, 0.0);
    drag_step(QEvent::MouseMove, Qt::NoButton, Qt::LeftButton, 4.0);
    drag_step(QEvent::MouseMove, Qt::NoButton, Qt::LeftButton, 8.0);
    drag_step(QEvent::MouseButtonPress, Qt::MiddleButton, Qt::LeftButton | Qt::MiddleButton, 8.0);
    drag_step(QEvent::MouseButtonRelease, Qt::MiddleButton, Qt::LeftButton, 8.0);
    drag_step(QEvent::MouseMove, Qt::NoButton, Qt::LeftButton, 12.0);
    mouse_a(QEvent::MouseButtonRelease, Qt::LeftButton, Qt::NoButton, 12.0);
    const mbvq::ViewState fell = mbvq::view_state(b);
    check(a->isVisible() && b->isVisible() && !before.fill_due && !full.fill_due &&
              full.frame_requests == before.frame_requests && quiet && fell.fill_due &&
              fell.frame_requests == before.frame_requests + 1 && !rhi::map_buttons_down(),
          "6c: after the last map button of the process goes up, another map view whose last fill fell back asks "
          "for a frame that fills (" + std::to_string(fell.frame_requests - full.frame_requests) +
              "), none while a button is down (a drag with three motions and a second button pressed and released, "
              "after each step:" + steps + "); one that did not fall back nothing (" +
              std::to_string(full.frame_requests - before.frame_requests) + ")");
    delete b;
    delete a;
  }
  return result();
}

/*--------------------------------------------------------------------*/
/* the release after a budgeted drag, and eviction across two renderers,
   with Null QRhis (6b) */
void null_levels() {
  Grid grid;
  grid.columns = 300;
  grid.rows = 200;
  size_t a = MBV_NO_WINDOW, b = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &a) && open_grid(grid, nullptr, &b), "6b: two 300 x 200 grids open under QRhi");

  /* A budgeted shading drag, then the release: the frame after the release
     draws the terrain at the stride of the zoom without the budget, in one
     frame (that a view asks for it, and that it fills, is --widgets'). */
  {
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions options;
    renderer.render_map(a, 2400, 1800, options, nullptr);
    const mbvq::RenderStats still = renderer.stats();
    /* stride 8 fits with about 4 ms to spare (T2-2), stride 4 does not */
    options.drag = true;
    options.prep_budget_ms = 6.0;
    options.node_ns_colour = 2000.0;
    options.node_ns_z = 1.0;
    core_drag(a, MBV_MOUSE_SHADE, 3, 1, 20);
    renderer.render_map(a, 2400, 1800, options, nullptr);
    const mbvq::RenderStats dragged = renderer.stats();
    core_release(a, 3);
    options.drag = false;
    renderer.render_map(a, 2400, 1800, options, nullptr);
    const mbvq::RenderStats released = renderer.stats();
    check(still.stride == 1 && still.stride_zoom == 1 && still.budget_ms < 0.0 && dragged.stride > 1 &&
              dragged.stride_zoom == 1 && dragged.drag_budget && dragged.budget_ms == 6.0 &&
              dragged.estimated_ms <= 6.0 && renderer.last_frames() == 1 && released.stride == 1 &&
              released.stride_zoom == 1 && released.budget_ms < 0.0 && !released.drag_budget &&
              released.tiles_waiting == 0 && released.tiles_drawn == 6,
          "a shading drag within a 6 ms budget drawn at stride " + std::to_string(dragged.stride) +
              "; the frame after the release draws stride 1 without the budget, in one frame");
  }

  /* The drag budget of the views (prep_budget_ms < 0): 1000/60 ms less the
     cost of the frame before outside its preparing and gathering (its whole
     time on the CPU less that work here, without a window), less 2 ms. */
  {
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions options;
    renderer.render_map(a, 2400, 1800, options, nullptr);
    options.drag = true;
    options.node_ns_colour = 1000.0;
    options.node_ns_z = 1.0;
    core_drag(a, MBV_MOUSE_SHADE, 3, 1, 20);
    renderer.render_map(a, 2400, 1800, options, nullptr);
    const mbvq::RenderStats first = renderer.stats();
    core_drag(a, MBV_MOUSE_SHADE, 3, 1, 30);
    renderer.render_map(a, 2400, 1800, options, nullptr);
    const mbvq::RenderStats second = renderer.stats();
    core_release(a, 3);
    options.drag = false;
    renderer.render_map(a, 2400, 1800, options, nullptr);
    const double outside = std::max(0.0, first.render_ms - first.work_ms);
    check(outside > 0.0 && second.outside_ms == outside && second.budget_ms == rhi::drag_budget_ms(outside) &&
              first.budget_ms == rhi::drag_budget_ms(first.outside_ms) && second.estimated_ms <= second.budget_ms,
          "the views' drag budget: the second frame of a drag leaves room for the first's " + text("%.3f", outside) +
              " ms outside its preparing and gathering: " + text("%.3f", second.budget_ms) + " ms (stride " +
              std::to_string(second.stride) + ")");
  }

  /* Eviction across two renderers: B's frame evicts A's least recently drawn
     levels, whose buffers A holds until its next frame drops them; the
     process counts their bytes until then. */
  {
    const size_t budget = mbvq::cache_budget();
    mbvq::OffscreenRenderer ra(mbvq::OffscreenRenderer::Api::Null), rb(mbvq::OffscreenRenderer::Api::Null);
    const mbvq::RenderOptions options;
    ra.render_map(a, 2400, 1800, options, nullptr);
    const size_t stride1 = ra.stats().resident_bytes;
    const int resources1 = ra.stats().resources;
    camera_2d(a, 0.02, 0.0, 0.0);
    ra.render_map(a, 2400, 1800, options, nullptr);
    const mbvq::RenderStats both = ra.stats();
    mbvq::set_cache_budget(both.resident_bytes + stride1 / 2);
    rb.render_map(b, 2400, 1800, options, nullptr);
    const mbvq::RenderStats b1 = rb.stats();
    const size_t counted = mbvq::cache_bytes();
    ra.render_map(a, 2400, 1800, options, nullptr);
    const mbvq::RenderStats a2 = ra.stats();
    check(both.stride > 1 && both.resources > resources1 && b1.stride == 1 &&
              counted == both.resident_bytes + b1.resident_bytes && a2.evictions > 0 &&
              a2.resources == both.resources - 3 * a2.evictions && a2.resident_bytes < both.resident_bytes &&
              mbvq::cache_bytes() <= mbvq::cache_budget() && a2.tiles_drawn == both.tiles_drawn,
          "two renderers on one budget: B's frame at stride 1 evicts " + std::to_string(a2.evictions) +
              " of A's stride-1 levels; A holds their buffers until its next frame (" + std::to_string(both.resources) +
              " then " + std::to_string(a2.resources) + " resources), the process counts them until then, and A draws "
              "its stride " + std::to_string(both.stride) + " as before");
    mbvq::set_cache_budget(budget);
    camera_2d(a, 1.0, 0.0, 0.0);
  }
  close_instance(b);
  close_instance(a);
}

/*--------------------------------------------------------------------*/
/* 7c: of 1000 soundings, those whose index is a multiple of 1, 2, 4, 8 and
   16: what the levels of detail 0 to 4 draw */
constexpr int kLodSoundings[5] = {1000, 500, 250, 125, 63};

/* 7c with a Null QRhi: the 3D soundings at a level of detail, forced
   (RenderOptions::soundings_lod), on the soundings of null_soundings (1000
   in chunks of 300), as points and, in a renderer of its own, as quads
   (forced). Level k draws every 2^k-th sounding: those whose index in the
   array is a multiple of 2^k, in each chunk from its first such sounding
   (the phase), with the positions bound 12 x 2^k bytes apart (binding 1 of
   the quads). A change of level reads, writes and allocates nothing, and
   the pipelines of the levels come with create(). */
void null_lod_levels() {
  make_sdg(40, 25, 7);
  check(open_sdg(), "7c Null: the soundings handed over for the levels of detail");
  sdg_camera(90.0f, 0.0f, 1.0f, 0.0f, 0.0f);
  sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, true);
  for (const bool quads : {false, true}) {
    const std::string kind = std::string("7c Null soundings as ") + (quads ? "quads (forced)" : "points");
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions options;
    options.soundings_chunk = 300;
    options.point_quads = quads;
    options.soundings_lod = 0;
    const bool primed = renderer.render_soundings(200, 150, options, nullptr);
    const mbvq::RenderStats first = renderer.stats();
    bool drawn = primed && !first.failed && first.point_quads == quads && first.sounding_chunks == 4 &&
                 first.soundings_lod == 0 && first.soundings_drawn == 1000 && first.soundings_stride == 12;
    bool kept = primed;
    std::vector<int> counts, strides;
    for (const int level : {1, 2, 3, 4, 0}) {
      options.soundings_lod = level;
      const bool ok = renderer.render_soundings(200, 150, options, nullptr);
      const mbvq::RenderStats s = renderer.stats();
      counts.push_back(s.soundings_drawn);
      strides.push_back(s.soundings_stride);
      drawn = drawn && ok && !s.failed && s.soundings_lod == level && s.soundings_drawn == kLodSoundings[level] &&
              s.soundings_stride == 12 * rhi::level_stride(level);
      kept = kept && ok && s.sounding_allocations == first.sounding_allocations &&
             std::equal(s.sounding_reads, s.sounding_reads + 3, first.sounding_reads) &&
             std::equal(s.sounding_writes, s.sounding_writes + 4, first.sounding_writes) &&
             s.sounding_held_bytes == first.sounding_held_bytes && s.resources == first.resources &&
             s.fixed_resources == first.fixed_resources && s.soundings_shown == 1000;
    }
    const int lod_frames = renderer.stats().lod_frames - first.lod_frames;
    check(drawn && lod_frames == 4,
          kind + ": the levels of detail 1, 2, 3, 4 and 0 forced draw " + list(counts) +
              " of 1000 soundings in chunks of 300 (those whose index is a multiple of 2, 4, 8, 16 and 1), their "
              "positions bound " + list(strides) + " bytes apart; " + std::to_string(lod_frames) +
              " frames counted at a level above 0");
    check(kept, kind + ": from level 0 through the levels 1, 2, 3, 4 and 0 nothing is read, written or allocated "
                       "again; the soundings' GPU bytes (" + std::to_string(first.sounding_held_bytes) +
                       ") and the resources (" + std::to_string(first.resources) + ", " +
                       std::to_string(first.fixed_resources) +
                       " fixed: the levels' pipelines come with create()) stay, all 1000 soundings shown");
  }
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
}

/* 7d1 (7d-14) with a Null QRhi, which reports Compute and no thread limits:
   the soundings' position and colour chunks have storage use, and the types
   of the other soundings' buffers with chunk_storage 0; switching the option
   on one offscreen renderer makes its soundings renderer again */
void null_chunk_storage() {
  make_sdg(40, 25, 7);
  check(open_sdg(), "7d1 Null: the soundings handed over for the chunks' storage use");
  sdg_camera(90.0f, 0.0f, 1.0f, 0.0f, 0.0f);
  sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, true);
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
  mbvq::RenderOptions options;
  options.soundings_chunk = 300;
  /* a renderer made in the call has rendered only that call's frames (its
     statistics are totals since it was made) */
  std::vector<int> storage, made, drawn, held;
  bool ok = true;
  for (const int option : {-1, 0, -1}) {
    options.chunk_storage = option;
    ok = renderer.render_soundings(200, 150, options, nullptr) && ok;
    const mbvq::RenderStats s = renderer.stats();
    ok = ok && !s.failed;
    storage.push_back(s.soundings_storage);
    made.push_back(s.frames == renderer.last_frames() ? 1 : 0);
    drawn.push_back(s.soundings_drawn);
    held.push_back(static_cast<int>(s.sounding_held_bytes));
  }
  check(ok && storage == std::vector<int>{1, 0, 1} && made == std::vector<int>{1, 1, 1} &&
            drawn == std::vector<int>{1000, 1000, 1000} && held[0] > 0 && held[0] == held[1] && held[1] == held[2],
        "7d1 Null: the soundings' chunks with storage use by default and without it with chunk_storage 0 (" +
            list(storage) + " for the default, 0, the default), the soundings renderer made again at each switch " +
            list(made) + ", " + list(drawn) + " soundings drawn, the same GPU bytes " + list(held) +
            " (Null keeps one copy of every buffer)");
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
}

/* 7c with a Null QRhi: the GPU-time fallback of the map and of the 3D
   soundings. A Null QRhi waits for no GPU: without fake times
   (RenderOptions::gpu_wait_ms and gpu_total_ms) no frame is measured and
   the fallback cannot act (gpu_rule); with them it can. The map on a
   300 x 200 grid at 2400 x 1800, where 6b's drag budget never binds
   (1000 ms, no cost per node), and the soundings of null_soundings; after a
   priming call one frame per call, so that each call feeds the rule once
   (every offscreen frame with fake times is measured). During a press
   (RenderOptions::drag, for both) two GPU-slow frames at a level mark it,
   and the next frames draw the next coarser level: the map's stride, the
   soundings' level of detail. */
void null_gpu_fallback() {
  Grid grid;
  grid.columns = 300;
  grid.rows = 200;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "7c Null: a 300 x 200 grid opens for the GPU-time fallback");
  make_sdg(40, 25, 7);
  check(open_sdg(), "7c Null: the soundings handed over for the GPU-time fallback");
  sdg_camera(90.0f, 0.0f, 1.0f, 0.0f, 0.0f);
  sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, true);
  mbvq::RenderOptions map_options;
  map_options.prep_budget_ms = 1000.0;
  map_options.node_ns_colour = 0.0;
  map_options.node_ns_z = 0.0;
  mbvq::RenderOptions soundings_options;
  soundings_options.soundings_chunk = 300;
  /* a call of the map's frames or of the soundings' */
  const auto render = [instance](mbvq::OffscreenRenderer &renderer, bool map, const mbvq::RenderOptions &options) {
    return map ? renderer.render_map(instance, 2400, 1800, options, nullptr)
               : renderer.render_soundings(200, 150, options, nullptr);
  };

  /* Without fake times a frame is not measured (wait_ms and frame_ms -1,
     wait_frames 0) and the fallback cannot act (gpu_rule false); a fake
     wait, or a fake total, lets it act, and the frame is measured with its
     own times, not the fake's: the wait from the end of its pass to after
     endOffscreenFrame(), the total from its start, so that the two differ
     by the pass's span; without them again nothing more is measured. */
  for (const bool map : {true, false}) {
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions options = map ? map_options : soundings_options;
    bool drawn = render(renderer, map, options);
    const mbvq::RenderStats none = renderer.stats();
    options.gpu_wait_ms = 1000.0;
    drawn = render(renderer, map, options) && drawn;
    const mbvq::RenderStats waited = renderer.stats();
    options.gpu_wait_ms = -1.0;
    options.gpu_total_ms = 16.0;
    drawn = render(renderer, map, options) && drawn;
    const mbvq::RenderStats totalled = renderer.stats();
    options.gpu_total_ms = -1.0;
    drawn = render(renderer, map, options) && drawn;
    const mbvq::RenderStats again = renderer.stats();
    const double pass = waited.render_end_ms - waited.render_begin_ms;
    check(drawn && !none.gpu_rule && none.wait_ms == -1.0 && none.frame_ms == -1.0 && none.wait_frames == 0 &&
              waited.gpu_rule && waited.wait_frames == 1 && waited.wait_ms >= 0.0 &&
              waited.wait_ms <= waited.frame_ms && waited.frame_ms < 1000.0 &&
              std::fabs(waited.frame_ms - waited.wait_ms - pass) < 1.0e-3 && totalled.gpu_rule &&
              totalled.wait_frames == 2 && !again.gpu_rule && again.wait_frames == 2 &&
              again.wait_ms == totalled.wait_ms && again.frame_ms == totalled.frame_ms,
          std::string("7c Null ") + (map ? "map" : "soundings") +
              ": without fake times no frame is measured (wait_ms " + text("%.0f", none.wait_ms) + ", frame_ms " +
              text("%.0f", none.frame_ms) + ") and the fallback cannot act (gpu_rule false); a fake wait of "
              "1000 ms lets it act and the frame measured with its own times, the wait after its pass and the "
              "total from its start (" + text("%.3f", waited.wait_ms) + " and " + text("%.3f", waited.frame_ms) +
              " ms, the pass " + text("%.3f", pass) + " ms), as does a fake total; without them again it cannot "
              "act and measures nothing (" + std::to_string(again.wait_frames) + " frames measured)");
  }

  /* A press with total 16 ms and a wait of 12 x 0.8^level ms: two GPU-slow
     frames at each level, each step cuts the wait by 20 % (the gain check
     asks for 10 %), and the wait stays 4 ms or more up to level 4, the
     soundings' coarsest. Then the release, a second press with constant
     times (one step, then the gain check stops it), and a third press with
     a frame without fake times between frames with them. */
  const std::vector<int> falling = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4};
  const std::vector<int> one_step = {0, 0, 1, 1, 1, 1};
  const std::vector<int> per_frame = {0, 0, 0, 1};
  for (const bool map : {true, false}) {
    const std::string name = std::string("7c Null ") + (map ? "map" : "soundings");
    mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions options = map ? map_options : soundings_options;
    const bool primed = render(renderer, map, options);
    /* a call after the priming one: one frame, during the press or not,
       with fake times (< 0: none) */
    bool one = primed;
    int calls = 0, faked = 0;
    const auto call = [&](bool press, double wait, double total) {
      options.drag = press;
      options.gpu_wait_ms = wait;
      options.gpu_total_ms = total;
      one = render(renderer, map, options) && one && renderer.last_frames() == 1 && !renderer.stats().failed;
      calls++;
      faked += wait >= 0.0 || total >= 0.0 ? 1 : 0;
      return renderer.stats();
    };
    /* a frame at a level: the map at its stride (the zoom's is 1), passing
       over the marked levels below it during a press; the soundings drawn
       at it, with their positions' stride */
    const auto at = [map](const mbvq::RenderStats &s, int level, bool press) {
      if (map)
        return s.stride == rhi::level_stride(level) && s.stride_zoom == 1 && s.gpu_coarser == (press && level > 0) &&
               !s.drag_budget && !s.memory_budget && s.tiles_waiting == 0;
      return s.soundings_lod == level && s.soundings_drawn == kLodSoundings[level] &&
             s.soundings_stride == 12 * rhi::level_stride(level) && s.soundings_shown == 1000;
    };
    /* what frames drew: the map's strides, the soundings' levels */
    const auto seen = [map](const std::vector<mbvq::RenderStats> &frames) {
      std::vector<int> out;
      for (const mbvq::RenderStats &s : frames)
        out.push_back(map ? s.stride : s.soundings_lod);
      return list(out);
    };
    const std::string what = map ? "the strides " : "the levels ";
    std::vector<mbvq::RenderStats> still, dragged, constant, mixed;
    for (int f = 0; f < 3; f++)
      still.push_back(call(false, 12.0, 16.0));
    const int lod_before = renderer.stats().lod_frames;
    for (const int level : falling)
      dragged.push_back(call(true, 12.0 * std::pow(0.8, level), 16.0));
    const int lod_press = renderer.stats().lod_frames - lod_before;
    const mbvq::RenderStats released = call(false, 12.0, 16.0);
    for (int f = 0; f < 6; f++)
      constant.push_back(call(true, 12.0, 16.0));
    call(false, 12.0, 16.0);
    for (int f = 0; f < 4; f++)
      mixed.push_back(f == 2 ? call(true, -1.0, -1.0) : call(true, 12.0, 16.0));
    call(false, -1.0, -1.0);
    const mbvq::RenderStats end = renderer.stats();

    check(one && end.wait_frames == faked,
          name + ": after a priming call each of " + std::to_string(calls) + " calls draws one frame, and each of " +
              "the " + std::to_string(faked) + " frames with fake times is measured once (" +
              std::to_string(end.wait_frames) + ")");
    bool ok = true;
    for (const mbvq::RenderStats &s : still)
      ok = ok && s.gpu_rule && at(s, 0, false);
    check(ok, name + ": three frames with slow fake times (wait 12 ms, total 16 ms) but no press draw " + what +
                  seen(still) + (map ? ", passing over no level" : ", all 1000 soundings") +
                  ": the fallback acts only during a press");
    ok = map || lod_press == 8;
    std::vector<int> coarser, counts, strides;
    for (size_t f = 0; f < dragged.size(); f++) {
      ok = ok && dragged[f].gpu_rule && at(dragged[f], falling[f], true);
      coarser.push_back(dragged[f].gpu_coarser ? 1 : 0);
      counts.push_back(dragged[f].soundings_drawn);
      strides.push_back(dragged[f].soundings_stride);
    }
    check(ok, name + ": a press with total 16 ms and a wait of 12 x 0.8^level ms draws " + what + seen(dragged) +
                  ", two GPU-slow frames marking each level (each step cuts the wait by 20 %); " +
                  (map ? "from the third frame it passes over the marked levels (gpu_coarser " + list(coarser) +
                             "), never at stride 128"
                       : list(counts) + " soundings, their positions " + list(strides) + " bytes apart, up to the "
                             "coarsest level, 4; " + std::to_string(lod_press) + " frames counted at a level above 0"));
    check(at(released, 0, false),
          name + ": the release's frame draws " +
              (map ? "stride 1 again (" + std::to_string(released.stride) + "), passing over no level"
                   : "level 0 again (" + std::to_string(released.soundings_lod) + "), " +
                         std::to_string(released.soundings_drawn) + " soundings, their positions " +
                         std::to_string(released.soundings_stride) + " bytes apart"));
    ok = true;
    for (size_t f = 0; f < constant.size(); f++)
      ok = ok && constant[f].gpu_rule && at(constant[f], one_step[f], true);
    check(ok, name + ": a second press starts at " + (map ? "stride 1" : "level 0") + ", and constant fake times " +
                  "(wait 12 ms, total 16 ms) take it one step only, since the step cuts no wait (the gain check): " +
                  what + seen(constant));
    ok = mixed[0].gpu_rule && mixed[1].gpu_rule && !mixed[2].gpu_rule && mixed[3].gpu_rule;
    std::vector<int> rules;
    for (size_t f = 0; f < mixed.size(); f++) {
      ok = ok && at(mixed[f], per_frame[f], true);
      rules.push_back(mixed[f].gpu_rule ? 1 : 0);
    }
    check(ok, name + ": in a third press, two GPU-slow frames with fake times, a frame without them and one with " +
                  "them again draw " + what + seen(mixed) + ": the frame without them " +
                  (map ? "stride 1" : "level 0") +
                  ", since on Null the fallback acts only with fake times, checked in each frame (gpu_rule " +
                  list(rules) + "), and the press's mark holds for the frame after it");
  }
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
  close_instance(instance);
}

/* 7c with a Null QRhi: the map's GPU-time fallback when the zoom moves to
   another stride during a press, with fake times (wait 12 ms and total
   16 ms GPU-slow, wait 6 ms and total 10 ms not), on the 300 x 200 grid at
   2400 x 1800 where 6b's drag budget never binds. Each stride of the zoom
   has a rule of its own, and the frame passes over the marks of its own
   zoom only. After a zoom in, two GPU-slow frames at the zoom's finer stride
   mark it there, and the next frame draws the stride after it. After a zoom
   out, the zoom's own stride, coarser than the one marked before, is judged
   by its own frames at the new zoom: 0.96 x the marked stride's wait,
   measured at the old zoom, would otherwise stop it there. Under constant
   waits each zoom marks its own stride in its own two frames, steps once
   and stops, whatever the other zooms do: with the zoom changing every
   second frame, each zoom's stride doubles once, where a rule that forgot
   its reference at each change would walk to stride 128; the same from the
   default view's stride 1, and in a zoom in from the stride of the zoom 64,
   where a stop at one zoom does not block the next. */
void null_gpu_zoom() {
  Grid grid;
  grid.columns = 300;
  grid.rows = 200;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "7c Null: a 300 x 200 grid opens for the zoom during a press");
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
  mbvq::RenderOptions options;
  options.prep_budget_ms = 1000.0;
  options.node_ns_colour = 0.0;
  options.node_ns_z = 0.0;
  /* the default view's zoom (stride 1), and the 2D zooms at which the
     stride of the zoom is 2, 4, 16, 32 and 64, searched outwards from it */
  const double zoom1 = 1.0;
  camera_2d(instance, zoom1, 0.0, 0.0);
  renderer.render_map(instance, 2400, 1800, options, nullptr);
  const int stride1 = renderer.stats().stride_zoom;
  double zoom2 = -1.0, zoom4 = -1.0, zoom16 = -1.0, zoom32 = -1.0, zoom64 = -1.0;
  for (double z = 1.0; z > 1.0e-4 && zoom64 < 0.0; z *= 0.8) {
    camera_2d(instance, z, 0.0, 0.0);
    renderer.render_map(instance, 2400, 1800, options, nullptr);
    const int stride = renderer.stats().stride_zoom;
    double *found = stride == 2 ? &zoom2 : stride == 4 ? &zoom4 : stride == 16 ? &zoom16 : stride == 32 ? &zoom32
                  : stride == 64 ? &zoom64 : nullptr;
    if (found != nullptr && *found < 0.0)
      *found = z;
  }
  bool one = stride1 == 1 && zoom2 > 0.0 && zoom4 > 0.0 && zoom16 > 0.0 && zoom32 > 0.0 && zoom64 > 0.0;
  /* a call of one frame with these fake times, pressed or not */
  const auto call = [&](bool press, double wait, double total) {
    options.drag = press;
    options.gpu_wait_ms = wait;
    options.gpu_total_ms = total;
    one = renderer.render_map(instance, 2400, 1800, options, nullptr) && one && renderer.last_frames() == 1 &&
          !renderer.stats().failed;
    return renderer.stats();
  };
  /* a press: frames at the first zoom, then at the second */
  const auto press = [&](double first, double second, double wait_after) {
    camera_2d(instance, first, 0.0, 0.0);
    call(false, -1.0, -1.0);
    std::vector<mbvq::RenderStats> frames;
    frames.push_back(call(true, 12.0, 16.0));
    frames.push_back(call(true, 12.0, 16.0));
    frames.push_back(call(true, 6.0, 10.0));
    camera_2d(instance, second, 0.0, 0.0);
    frames.push_back(call(true, wait_after, 16.0));
    frames.push_back(call(true, wait_after, 16.0));
    frames.push_back(call(true, 6.0, 10.0));
    call(false, -1.0, -1.0);
    return frames;
  };
  /* what the frames drew, against the strides, the strides of the zoom and
     the passed-over flags expected */
  const auto drew = [](const std::vector<mbvq::RenderStats> &frames, const std::vector<int> &strides,
                       const std::vector<int> &zooms, const std::vector<int> &coarser, std::string *seen) {
    bool ok = frames.size() == strides.size();
    std::vector<int> s, z, c;
    for (size_t f = 0; f < frames.size(); f++) {
      s.push_back(frames[f].stride);
      z.push_back(frames[f].stride_zoom);
      c.push_back(frames[f].gpu_coarser ? 1 : 0);
      ok = ok && f < strides.size() && frames[f].stride == strides[f] && frames[f].stride_zoom == zooms[f] &&
           (frames[f].gpu_coarser ? 1 : 0) == coarser[f] && frames[f].gpu_rule && !frames[f].drag_budget &&
           !frames[f].memory_budget && frames[f].tiles_waiting == 0;
    }
    *seen = "strides " + list(s) + ", strides of the zoom " + list(z) + ", gpu_coarser " + list(c);
    return ok;
  };
  std::string in_seen, out_seen;
  const bool in = drew(press(zoom4, zoom2, 12.0), {4, 4, 8, 2, 2, 4}, {4, 4, 4, 2, 2, 2}, {0, 0, 1, 0, 0, 1}, &in_seen);
  const bool out =
      drew(press(zoom2, zoom4, 11.5), {2, 2, 4, 4, 4, 8}, {2, 2, 2, 4, 4, 4}, {0, 0, 1, 0, 0, 1}, &out_seen);
  check(one && in,
        "7c Null map: a press that zooms in after marking the stride of the zoom (4): two GPU-slow frames at the "
        "new zoom's stride 2, with the wait that marked stride 4, mark it there, and the next frame draws stride 4, "
        "stride 4's mark belonging to the other zoom (" + in_seen + ")");
  check(one && out,
        "7c Null map: a press that zooms out after marking the stride of the zoom (2): the new zoom's own stride 4, "
        "GPU-slow with 0.96 x the wait that marked stride 2, is judged by its own frames, which mark it, and the "
        "next frame draws stride 8 (" + out_seen + ")");
  /* the zoom's stride changing every second frame, 2 and 4, under constant
     fake times (12 ms, total 16 ms) */
  camera_2d(instance, zoom2, 0.0, 0.0);
  call(false, -1.0, -1.0);
  std::vector<mbvq::RenderStats> flips;
  for (int f = 0; f < 12; f++) {
    camera_2d(instance, f / 2 % 2 == 0 ? zoom2 : zoom4, 0.0, 0.0);
    flips.push_back(call(true, 12.0, 16.0));
  }
  call(false, -1.0, -1.0);
  std::string flip_seen;
  const bool flipped = drew(flips, {2, 2, 4, 4, 4, 4, 8, 8, 4, 4, 8, 8}, {2, 2, 4, 4, 2, 2, 4, 4, 2, 2, 4, 4},
                            {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1}, &flip_seen);
  check(one && flipped,
        "7c Null map: a press whose stride of the zoom changes every second frame (2, 4) under constant fake times: "
        "each zoom marks its own stride in its own two frames, steps once and stops there, so the zoom of 2 draws "
        "4 and the zoom of 4 draws 8, never walking the levels (" + flip_seen + ")");
  /* the same two cases from the default view's stride 1 */
  std::string first_seen, first_flip_seen;
  const bool first_out =
      drew(press(zoom1, zoom2, 11.5), {1, 1, 2, 2, 2, 4}, {1, 1, 1, 2, 2, 2}, {0, 0, 1, 0, 0, 1}, &first_seen);
  camera_2d(instance, zoom1, 0.0, 0.0);
  call(false, -1.0, -1.0);
  std::vector<mbvq::RenderStats> first_flips;
  for (int f = 0; f < 12; f++) {
    camera_2d(instance, f / 2 % 2 == 0 ? zoom1 : zoom2, 0.0, 0.0);
    first_flips.push_back(call(true, 12.0, 16.0));
  }
  call(false, -1.0, -1.0);
  const bool first_flipped = drew(first_flips, {1, 1, 2, 2, 2, 2, 4, 4, 2, 2, 4, 4},
                                  {1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2}, {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
                                  &first_flip_seen);
  check(one && first_out && first_flipped,
        "7c Null map: from the default view's stride of the zoom 1, a zoom out after marking stride 1 is judged at "
        "the new zoom (" + first_seen + "), and with the zoom changing every second frame between 1 and 2 the zoom "
        "of 1 draws 2 and the zoom of 2 draws 4 (" + first_flip_seen + ")");
  /* a zoom in from the stride of the zoom 64 under constant fake times,
     three frames at 64 and five at each of 32 and 16: at each zoom two
     frames mark its own stride, the step after them is judged and stops
     that zoom, so its later frames keep that stride, and a stop at one zoom
     does not block the next (a rule that kept its marks for the whole press
     and never judged a step into the coarsest level would draw stride 128
     at every zoom) */
  camera_2d(instance, zoom64, 0.0, 0.0);
  call(false, -1.0, -1.0);
  std::vector<mbvq::RenderStats> sweep;
  for (const double z : {zoom64, zoom64, zoom64, zoom32, zoom32, zoom32, zoom32, zoom32, zoom16, zoom16, zoom16,
                         zoom16, zoom16}) {
    camera_2d(instance, z, 0.0, 0.0);
    sweep.push_back(call(true, 12.0, 16.0));
  }
  call(false, -1.0, -1.0);
  std::string sweep_seen;
  const bool swept = drew(sweep, {64, 64, 128, 32, 32, 64, 64, 64, 16, 16, 32, 32, 32},
                          {64, 64, 64, 32, 32, 32, 32, 32, 16, 16, 16, 16, 16},
                          {0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1}, &sweep_seen);
  check(one && swept,
        "7c Null map: a zoom in from the stride of the zoom 64 under constant fake times: at each zoom two frames "
        "mark its own stride, the step after them is judged and stops that zoom, so its later frames keep that "
        "stride, one past its own, and a stop at one zoom does not block the next (" + sweep_seen + ")");
  close_instance(instance);
}

/*--------------------------------------------------------------------*/
int test_render_null() {
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Null);
  check(renderer.valid(), std::string("a Null QRhi (") + renderer.backend_name() + ")");
  if (!renderer.valid())
    return result();
  Grid grid;
  grid.columns = 300;
  grid.rows = 200;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "a 300 x 200 grid opens under QRhi");

  mbvq::RenderOptions options;
  bool rendered = renderer.render_map(instance, 320, 240, options, nullptr);
  mbvq::RenderStats stats = renderer.stats();
  const int tiles_uploaded = stats.uploads_positions;
  check(rendered && !stats.failed && stats.tiles == 6 && stats.terrain_slot && stats.groups == 11 &&
            stats.fill_rez == MBVQ_REZ_FULL && stats.uploads_indices == 6 && stats.uploads_positions > 0 &&
            stats.uploads_positions == stats.uploads_colours && stats.tiles_drawn == stats.uploads_positions &&
            stats.resources >= 5,
        "a frame with every pipeline: " + std::to_string(stats.tiles_drawn) + " tiles drawn, " +
            std::to_string(stats.triangles) + " triangles, " + std::to_string(stats.resources) + " resources");
  /* the frame's times: disjoint parts of the whole frame, on one clock */
  const double parts =
      stats.fill_ms + stats.marks_ms + stats.prepare_ms + stats.gather_ms + stats.upload_ms + stats.record_ms;
  check(stats.fill_ms > 0.0 && stats.marks_ms > 0.0 && stats.prepare_ms > 0.0 && stats.gather_ms > 0.0 &&
            stats.upload_ms >= 0.0 && stats.record_ms >= 0.0 && parts <= stats.render_ms && stats.gpu_ms < 0.0 &&
            stats.render_begin_ms > 0.0 &&
            std::fabs(stats.render_end_ms - stats.render_begin_ms - stats.render_ms) < 1.0e-3,
        "the frame's times: fill, marks, preparation and gathering measured, together at most the whole frame (" +
            text("%.3f", parts) + " of " + text("%.3f", stats.render_ms) +
            " ms), which its begin and end on the clock span; no GPU time from a Null QRhi");
  const size_t bytes = stats.bytes_positions + stats.bytes_colours + stats.bytes_indices;
  rendered = renderer.render_map(instance, 320, 240, options, nullptr);
  stats = renderer.stats();
  check(rendered && stats.bytes_positions + stats.bytes_colours + stats.bytes_indices == bytes && stats.fills == 2,
        "a second frame fills again and uploads nothing");
  check(stats.prepare_ms == 0.0 && stats.fill_ms > 0.0, "... with a fill time and no preparation time");
  mbvq_c_view(instance, MBVQ_VIEW_COLORTABLE_GRAY, true);
  renderer.render_map(instance, 320, 240, options, nullptr);
  const mbvq::RenderStats grey = renderer.stats();
  check(grey.uploads_colours == 2 * stats.uploads_colours && grey.bytes_positions == stats.bytes_positions,
        "a colour table change uploads the colours again, not the positions");

  renderer.release();
  check(renderer.stats().resources == 0, "release() leaves no resource");
  rendered = renderer.render_map(instance, 320, 240, options, nullptr);
  stats = renderer.stats();
  check(rendered && stats.uploads_positions == tiles_uploaded && stats.resources >= 5 &&
            stats.bytes_positions + stats.bytes_colours + stats.bytes_indices == bytes,
        "made again, the renderer uploads everything again from the core's arrays");

  /* split uploads over frames; the frames after the first reuse the batch
     (forced to stride 1, whose tiles are drawn as they are complete) */
  renderer.release();
  options.upload_budget = 600 * 1024;
  options.force_stride1 = true;
  rendered = renderer.render_map(instance, 320, 240, options, nullptr, 20);
  stats = renderer.stats();
  check(rendered && renderer.last_frames() > 1 && stats.tiles_waiting == 0 && stats.fills == 1 &&
            stats.tiles_drawn == stats.uploads_positions,
        "a 600 kB budget: " + std::to_string(renderer.last_frames()) +
            " frames, one fill, every tile drawn in the end (largest frame " +
            std::to_string(stats.bytes_largest_frame / 1024) + " kB)");
  options.upload_budget = mbvq::RenderOptions().upload_budget;
  options.force_stride1 = false;

  /* a create() that fails: nothing held right after it, and the next one works */
  renderer.release();
  options.fail_create = true;
  rendered = renderer.render_map(instance, 320, 240, options, nullptr);
  stats = renderer.stats();
  check(!rendered && stats.failed && stats.held_after_failure == 0 && stats.resources == 0,
        "a failed create(): reported, no resource held after it (" + std::to_string(stats.held_after_failure) + ")");
  options.fail_create = false;
  rendered = renderer.render_map(instance, 320, 240, options, nullptr);
  stats = renderer.stats();
  check(rendered && !stats.failed && stats.resources >= 5, "the next create() works");
  /* the pipelines: the terrain's two and two for each of the eleven kinds
     of the batch's draws (with and without the depth test), the bindings,
     the uniform ring and the render pass */
  check(stats.fixed_resources == 2 + 2 * 11 + 3,
        "the renderer holds the pipelines of every draw of the batch (" + std::to_string(stats.fixed_resources) +
            " fixed resources)");

  /* tile buffers that cannot be made (the GPU memory ran out): the frame
     fails and draws nothing; the renderer keeps its pipelines and no tile
     and no buffer of the batch, and the next frame uploads every tile and
     the batch again */
  renderer.release();
  options.fail_buffers = true;
  rendered = renderer.render_map(instance, 320, 240, options, nullptr);
  const mbvq::RenderStats failed = renderer.stats();
  options.fail_buffers = false;
  rendered = !rendered && renderer.render_map(instance, 320, 240, options, nullptr);
  stats = renderer.stats();
  check(failed.failed && failed.resources == failed.fixed_resources && failed.batch_resources == 0 &&
            failed.tiles_drawn == 0 && rendered && !stats.failed &&
            stats.uploads_positions - failed.uploads_positions == tiles_uploaded && stats.tiles_drawn == tiles_uploaded &&
            stats.resources == stats.fixed_resources + stats.batch_resources + 3 * tiles_uploaded,
        "tile buffers that cannot be made: nothing drawn, the pipelines kept (" + std::to_string(failed.resources) +
            " resources); the next frame uploads and draws every tile (" + std::to_string(stats.tiles_drawn) + ")");

  /* float colours for comparisons: 16 bytes per node instead of 4 */
  {
    mbvq::OffscreenRenderer bytes(mbvq::OffscreenRenderer::Api::Null), floats(mbvq::OffscreenRenderer::Api::Null);
    mbvq::RenderOptions with_floats;
    with_floats.float_terrain_colours = true;
    const bool both = bytes.render_map(instance, 320, 240, mbvq::RenderOptions(), nullptr) &&
                      floats.render_map(instance, 320, 240, with_floats, nullptr);
    check(both && floats.stats().bytes_colours == 4 * bytes.stats().bytes_colours &&
              floats.stats().bytes_positions == bytes.stats().bytes_positions &&
              floats.stats().tiles_drawn == bytes.stats().tiles_drawn,
          "float_terrain_colours uploads the colours as floats (" + std::to_string(floats.stats().bytes_colours) +
              " against " + std::to_string(bytes.stats().bytes_colours) + " bytes)");
  }

  /* the same renderer for a map of another size: the tiles are laid out
     again and none of the old ones is held (the instances stay open, so that
     none is opened again under the number of another) */
  Grid small;
  size_t third = MBV_NO_WINDOW;
  check(open_grid(small, nullptr, &third), "a 241 x 181 grid opens under QRhi");
  rendered = renderer.render_map(third, 320, 240, options, nullptr);
  stats = renderer.stats();
  check(rendered && stats.tiles == 4 && stats.tiles_drawn == 4 && stats.resources == stats.fixed_resources + stats.batch_resources + 3 * 4,
        "a grid of another size: " + std::to_string(stats.tiles) + " tiles, " + std::to_string(stats.resources) +
            " resources, none of the old tiles");

  /* a tile without data holds no buffer; a triangle made by cell edits gives
     it its three, and the edit that takes it away drops them again */
  {
    Grid blank = grid;
    blank.empty_tile = true;
    size_t fourth = MBV_NO_WINDOW;
    check(open_grid(blank, nullptr, &fourth), "a 300 x 200 grid with a tile without data opens under QRhi");
    struct mbview_struct *data4 = data_of(fourth);
    rendered = renderer.render_map(fourth, 320, 240, options, nullptr);
    const mbvq::RenderStats empty = renderer.stats();
    int error = MB_ERROR_NO_ERROR;
    const int nodes[3][2] = {{10, 150}, {11, 150}, {10, 151}};
    for (const auto &node : nodes) {
      const double lon = data4->primary_xmin + node[0] * data4->primary_dx;
      const double lat = data4->primary_ymin + node[1] * data4->primary_dy;
      mbview_updateprimarygridcell(0, fourth, node[0], node[1], static_cast<float>(surface(lon, lat)), &error);
    }
    rendered = rendered && renderer.render_map(fourth, 320, 240, options, nullptr);
    const mbvq::RenderStats triangle = renderer.stats();
    mbview_updateprimarygridcell(0, fourth, 10, 151, data4->primary_nodatavalue, &error);
    rendered = rendered && renderer.render_map(fourth, 320, 240, options, nullptr);
    stats = renderer.stats();
    check(rendered && empty.tiles == 6 && empty.tiles_drawn == 5 && empty.resources == empty.fixed_resources + empty.batch_resources + 3 * 5 &&
              triangle.tiles_drawn == 6 && triangle.triangles == empty.triangles + 1 &&
              triangle.resources == triangle.fixed_resources + triangle.batch_resources + 3 * 6 &&
              stats.tiles_drawn == 5 && stats.resources == stats.fixed_resources + stats.batch_resources + 3 * 5,
          "a tile without data holds no buffer (" + std::to_string(empty.resources) + " resources), with a triangle " +
              "its three (" + std::to_string(triangle.resources) + "), without it none again (" +
              std::to_string(stats.resources) + ")");
    close_instance(fourth);
  }
  close_instance(third);

  /* the QRhi deleted while the renderer holds resources */
  renderer.destroy_rhi();
  check(!renderer.valid() && renderer.stats().resources == 0,
        "the QRhi deleted under the renderer: its resources went through the cleanup callback");

  close_instance(instance);
  null_levels();
  null_batch();
  null_buffers();
  null_meshes();
  null_soundings();
  null_failures();
  /* 7a: the renderers' create() times */
  null_create_time();
  /* 7b: the profile mode after fills that hide it, and the profile lines
     of soundings from three files */
  null_profile_mode();
  /* 7c: the 3D soundings' levels of detail, and the GPU-time fallback of
     the map and of the soundings with fake times */
  null_lod_levels();
  null_gpu_fallback();
  /* 7c: the map's fallback when the zoom moves to another stride in a press */
  null_gpu_zoom();
  /* 7d1: the chunks' storage use, and the option chunk_storage on one renderer */
  null_chunk_storage();
  return result();
}

/*--------------------------------------------------------------------*/
/* pixels of the Metal mode */
constexpr int kWidth = 400;
constexpr int kHeight = 300;

uint32_t rgb(const QImage &image, int column, int row) {
  const QRgb pixel = image.pixel(column, row);
  return static_cast<uint32_t>(qRed(pixel)) | static_cast<uint32_t>(qGreen(pixel)) << 8 |
         static_cast<uint32_t>(qBlue(pixel)) << 16;
}

/* the pixels whose colour differs between two pictures of one size; -1 if
   they cannot be compared */
int differing(const QImage &a, const QImage &b) {
  if (a.isNull() || b.isNull() || a.size() != b.size())
    return -1;
  int n = 0;
  for (int row = 0; row < a.height(); row++)
    for (int column = 0; column < a.width(); column++)
      n += rgb(a, column, row) != rgb(b, column, row);
  return n;
}

/* a triangle of the drawing: cell (i, j), A or B */
struct Triangle {
  bool found = false;
  int i = 0;
  int j = 0;
  bool b = false;
  bool operator==(const Triangle &o) const { return found == o.found && (!found || (i == o.i && j == o.j && b == o.b)); }
};

/* the triangle the CPU pick finds at a pixel (image row 0 at the top) */
Triangle pick(size_t instance, int column, int row) {
  Triangle t;
  double x = 0.0, y = 0.0, z = 0.0;
  t.found = mbvq_pick_cpu_terrain(instance, column, kHeight - 1 - row, &x, &y, &z);
  if (!t.found)
    return t;
  const struct mbview_struct *data = data_of(instance);
  /* the centroid formula: 3 i + 1 (A) or 3 i + 2 (B) thirds of a cell */
  const long fx = std::lround((x - data->primary_xmin) / data->primary_dx * 3.0);
  const long fy = std::lround((y - data->primary_ymin) / data->primary_dy * 3.0);
  t.i = static_cast<int>(fx / 3);
  t.j = static_cast<int>(fy / 3);
  t.b = fx % 3 == 2;
  return t;
}

/* the triangle an ID picture shows at a pixel */
Triangle shown(size_t instance, const QImage &image, int column, int row) {
  Triangle t;
  const uint32_t value = rgb(image, column, row);
  if (value == kWhite || value == 0)
    return t;
  const int nr = data_of(instance)->primary_n_rows;
  const uint32_t id = value - 1;
  t.found = true;
  t.b = (id & 1) != 0;
  t.i = static_cast<int>((id / 2) / static_cast<uint32_t>(nr - 1));
  t.j = static_cast<int>((id / 2) % static_cast<uint32_t>(nr - 1));
  return t;
}

/* the nodes of a triangle */
std::array<int, 3> nodes(size_t instance, const Triangle &t) {
  const int nr = data_of(instance)->primary_n_rows;
  const int k = t.i * nr + t.j, l = (t.i + 1) * nr + t.j, m = k + 1, n = l + 1;
  return t.b ? std::array<int, 3>{l, n, m} : std::array<int, 3>{k, l, m};
}

/* the distance in pixels from a pixel centre to the nearest edge of a
   triangle as the camera projects it */
double edge_distance(size_t instance, const Triangle &t, int column, int row) {
  if (!t.found)
    return 1.0e9;
  double projection[16], modelview[16], camera[16];
  mbvq_c_camera(instance, projection, modelview);
  rhi::multiply(projection, modelview, camera);
  const struct mbview_struct *data = data_of(instance);
  double wx[3], wy[3];
  const std::array<int, 3> k = nodes(instance, t);
  for (int v = 0; v < 3; v++) {
    const double p[4] = {data->primary_x[k[v]], data->primary_y[k[v]], data->primary_z[k[v]], 1.0};
    double c[4];
    for (int r = 0; r < 4; r++)
      c[r] = camera[r] * p[0] + camera[4 + r] * p[1] + camera[8 + r] * p[2] + camera[12 + r] * p[3];
    wx[v] = (c[0] / c[3] + 1.0) * 0.5 * kWidth;
    wy[v] = (c[1] / c[3] + 1.0) * 0.5 * kHeight;
  }
  const double px = column + 0.5, py = kHeight - 1 - row + 0.5;
  double nearest = 1.0e9;
  for (int e = 0; e < 3; e++) {
    const double ax = wx[e], ay = wy[e], bx = wx[(e + 1) % 3], by = wy[(e + 1) % 3];
    const double length = std::hypot(bx - ax, by - ay);
    if (length > 0.0)
      nearest = std::min(nearest, std::fabs((bx - ax) * (py - ay) - (by - ay) * (px - ax)) / length);
  }
  return nearest;
}

/* The rasteriser snaps vertices to a grid of 1/256 pixel (8 bits of sub-pixel
   precision), so a pixel centre that close to an edge shared by two triangles
   can fall to either: a rasterisation tie, not a difference. */
constexpr double kTie = 1.0 / 256.0;

/* pick against picture: the CPU pick names the triangle an ID picture shows,
   except at ties (kTie); returns the pixels compared, and the ties */
int pick_against_picture(mbvq::OffscreenRenderer &renderer, size_t instance, int *mismatches, int *ties) {
  mbvq::RenderOptions options;
  options.triangle_ids = true;
  QImage image;
  *mismatches = 0;
  *ties = 0;
  if (!renderer.render_map(instance, kWidth, kHeight, options, &image))
    return 0;
  int compared = 0;
  for (int row = 1; row < kHeight; row += 3) {
    for (int column = 2; column < kWidth; column += 3) {
      const Triangle cpu = pick(instance, column, row);
      const Triangle gpu = shown(instance, image, column, row);
      if (cpu == gpu) {
        compared++;
        continue;
      }
      if (edge_distance(instance, cpu, column, row) < kTie || edge_distance(instance, gpu, column, row) < kTie) {
        (*ties)++;
        continue;
      }
      if (*mismatches < 5)
        std::printf("      pixel %d,%d: CPU %d %d,%d%s, picture %d %d,%d%s, edges %.5f and %.5f px away\n", column, row,
                    cpu.found, cpu.i, cpu.j, cpu.b ? "B" : "A", gpu.found, gpu.i, gpu.j, gpu.b ? "B" : "A",
                    edge_distance(instance, cpu, column, row), edge_distance(instance, gpu, column, row));
      (*mismatches)++;
    }
  }
  return compared;
}

/* the CPU picks of two instances agree over the sample pixels */
bool picks_agree(size_t a, size_t b) {
  for (int row = 1; row < kHeight; row += 9)
    for (int column = 2; column < kWidth; column += 9)
      if (!(pick(a, column, row) == pick(b, column, row)))
        return false;
  return true;
}

/* the pixels that are not white */
int coloured(const QImage &image) {
  int n = 0;
  for (int row = 0; row < image.height(); row++)
    for (int column = 0; column < image.width(); column++)
      n += rgb(image, column, row) != kWhite;
  return n;
}

/* Levels and culling in pictures, with a Metal QRhi (6b): the levels against
   stride 1 where a cell covers a pixel, culling on against off, and a cell
   edit that brings a tile below the view into it, against a picture made
   from nothing. */
void metal_levels() {
  Grid grid;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "6b: the synthetic grid opens for the levels");
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Metal);
  mbvq::RenderOptions options, forced, everything;
  forced.force_stride1 = true;
  everything.culling = false;
  QImage levels, stride1;

  /* where a cell covers a pixel or more: stride 1, the same bytes */
  for (const bool mode3d : {false, true}) {
    if (mode3d)
      mbvq_c_view(instance, MBVQ_VIEW_DISPLAY_3D, true);
    renderer.render_map(instance, kWidth, kHeight, options, &levels);
    const mbvq::RenderStats stats = renderer.stats();
    renderer.render_map(instance, kWidth, kHeight, forced, &stride1);
    check(stats.stride == 1 && stats.stride_zoom == 1 && !levels.isNull() && differing(levels, stride1) == 0,
          std::string("levels against force_stride1 where a cell covers a pixel or more (") + (mode3d ? "3D" : "2D") +
              "): stride 1, " + std::to_string(differing(levels, stride1)) + " pixels differ");
  }
  mbvq_c_view(instance, MBVQ_VIEW_DISPLAY_2D, true);

  /* zoomed out: a coarser stride, against stride 1 (reported: the cells are
     below a pixel there, E9) */
  camera_2d(instance, 0.25, 0.0, 0.0);
  renderer.render_map(instance, kWidth, kHeight, options, &levels);
  const mbvq::RenderStats coarse = renderer.stats();
  renderer.render_map(instance, kWidth, kHeight, forced, &stride1);
  const int covered = coloured(stride1), covered_levels = coloured(levels);
  const double out_cell = largest_cell(instance, kWidth, kHeight);
  check(coarse.stride > 1 && coarse.stride == coarse.stride_zoom && zoom_matches(coarse.stride, out_cell, true) &&
            covered > 0 && std::abs(covered_levels - covered) <= covered / 10,
        "zoomed out: stride " + std::to_string(coarse.stride) + ", the coarsest that keeps the largest cell (" +
            text("%.3f", out_cell) + " px at stride 1) under a pixel: " + std::to_string(differing(levels, stride1)) +
            " of " + std::to_string(covered) + " terrain pixels differ from stride 1, " +
            std::to_string(covered_levels) + " pixels covered against " + std::to_string(covered));

  /* Metal keeps each tile buffer (static, without storage use) once per
     frame in flight, and the memory budget counts every copy (M2): a Metal
     renderer holds the Null one's bytes that many times for the same frame */
  {
    mbvq::OffscreenRenderer metal(mbvq::OffscreenRenderer::Api::Metal), null(mbvq::OffscreenRenderer::Api::Null);
    metal.render_map(instance, kWidth, kHeight, options, nullptr);
    null.render_map(instance, kWidth, kHeight, options, nullptr);
    const mbvq::RenderStats m = metal.stats(), n = null.stats();
    check(m.buffer_copies >= 2 && n.buffer_copies == 1 && n.resident_bytes > 0 &&
              m.resident_bytes == static_cast<size_t>(m.buffer_copies) * n.resident_bytes,
          "the memory budget counts every copy Metal keeps: " + std::to_string(m.resident_bytes) + " bytes against " +
              std::to_string(n.resident_bytes) + " with a Null QRhi, " + std::to_string(m.buffer_copies) +
              " copies");
  }

  /* Levels of a stride that was not drawn for a while: zoomed out (stride 2),
     then in (stride 1) where a cell edit across no data, a colour table
     change and an exaggeration change follow, then out again. The levels of
     stride 2 went stale meanwhile: the cached picture equals one made from
     nothing, in 2D and in 3D. */
  for (const bool mode3d : {false, true}) {
    if (mode3d)
      mbvq_c_view(instance, MBVQ_VIEW_DISPLAY_3D, true);
    /* zoomed out: in 2D by the map's scale, in 3D by moving the model away
       (not named far, which <windows.h> defines away) */
    const auto zoom_out = [&](bool away) {
      camera_2d(instance, away && !mode3d ? 0.25 : 1.0, 0.0, 0.0);
      mbviews[instance].offset3d_z = away && mode3d ? -13.0f : 0.0f;
    };
    zoom_out(true);
    renderer.render_map(instance, kWidth, kHeight, options, nullptr);
    const int out = renderer.stats().stride;
    /* never coarser than the cells allow (in 3D the plan's bound may be finer) */
    const bool out_allowed = zoom_matches(renderer.stats().stride_zoom, largest_cell(instance, kWidth, kHeight), !mode3d);
    zoom_out(false);
    int error = MB_ERROR_NO_ERROR;
    const int ei = mode3d ? 90 : 100, ej = 70;
    mbview_updateprimarygridcell(0, instance, ei, ej, data_of(instance)->primary_nodatavalue, &error);
    renderer.render_map(instance, kWidth, kHeight, options, nullptr);
    const int in = renderer.stats().stride;
    mbvq_c_view(instance, mode3d ? MBVQ_VIEW_COLORTABLE_HAXBY : MBVQ_VIEW_COLORTABLE_BRIGHT, true);
    renderer.render_map(instance, kWidth, kHeight, options, nullptr);
    set_exaggeration(instance, mode3d ? 2.0 : 1.5);
    renderer.render_map(instance, kWidth, kHeight, options, nullptr);
    zoom_out(true);
    QImage cached, scratch;
    renderer.render_map(instance, kWidth, kHeight, options, &cached);
    const mbvq::RenderStats again = renderer.stats();
    {
      mbvq::OffscreenRenderer fresh(mbvq::OffscreenRenderer::Api::Metal);
      fresh.render_map(instance, kWidth, kHeight, options, &scratch);
    }
    check(out > 1 && out_allowed && in == 1 && again.stride == out && coloured(cached) > 0 &&
              differing(cached, scratch) == 0,
          std::string("levels of stride ") + std::to_string(out) + " not drawn while an edit, a colour change and an "
              "exaggeration change came (" + (mode3d ? "3D" : "2D") + "): drawn again, the cached picture equals one "
              "made from nothing (" + std::to_string(differing(cached, scratch)) + " pixels differ)");
    zoom_out(false);
  }
  mbvq_c_view(instance, MBVQ_VIEW_DISPLAY_2D, true);

  /* culling on and off: the same picture, zoomed in (2D; 3D below) */
  {
    QImage culled, all;
    camera_2d(instance, 3.0, 0.8, 0.5);
    renderer.render_map(instance, kWidth, kHeight, options, &culled);
    const mbvq::RenderStats with = renderer.stats();
    renderer.render_map(instance, kWidth, kHeight, everything, &all);
    const mbvq::RenderStats without = renderer.stats();
    check(with.tiles_culled > 0 && without.tiles_culled == 0 && with.tiles_drawn < without.tiles_drawn &&
              with.stride == without.stride && coloured(culled) > 0 && differing(culled, all) == 0,
          "culling on and off give the same picture (2D, " + std::to_string(with.tiles_drawn) + " against " +
              std::to_string(without.tiles_drawn) + " tiles drawn): " + std::to_string(differing(culled, all)) +
              " pixels differ");
    camera_2d(instance, 1.0, 0.0, 0.0);
  }

  /* On the spheroid every coordinate moves with the exaggeration, and the
     boxes come from a prepared sample of each tile: culling on and off give
     the same picture, also after an exaggeration change, which equals a
     picture made from nothing. */
  {
    mbvq_c_setprojection(instance, 2);
    const bool spheroid = data_of(instance)->display_projection_mode == MBV_PROJECTION_SPHEROID;
    QImage globe, all, scratch;
    bool same = true, allowed = true;
    int pixels = 0, drawn = 0, zoom = 0;
    for (int change = 0; change < 2; change++) {
      if (change == 1)
        set_exaggeration(instance, 4.0);
      renderer.render_map(instance, kWidth, kHeight, options, &globe);
      drawn = renderer.stats().tiles_drawn;
      zoom = renderer.stats().stride_zoom;
      {
        mbvq::OffscreenRenderer whole(mbvq::OffscreenRenderer::Api::Metal), fresh(mbvq::OffscreenRenderer::Api::Metal);
        whole.render_map(instance, kWidth, kHeight, everything, &all);
        fresh.render_map(instance, kWidth, kHeight, options, &scratch);
      }
      same = same && differing(globe, all) == 0 && differing(globe, scratch) == 0;
      pixels = coloured(globe);
      allowed = allowed && zoom_matches(zoom, largest_cell(instance, kWidth, kHeight), false);
    }
    check(spheroid && drawn == 4 && pixels > 1000 && same && allowed,
          "on the spheroid: culling on and off, before and after an exaggeration change, give the same picture as "
          "one made from nothing (" +
              std::to_string(pixels) + " terrain pixels, " + std::to_string(drawn) + " tiles, stride " +
              std::to_string(zoom) + ", no coarser than its cells allow)");
    mbvq_c_setprojection(instance, 1);
    set_exaggeration(instance, 1.0);
  }
  close_instance(instance);

  /* On the spheroid, culling on and off give the same picture where a
     tile's data lie between its box's lattice nodes far from the rest (a
     narrow strip), and after a switch from the secondary grid mode, where
     the boxes were made, to the primary (S1, R2) */
  {
    Grid sparse;
    sparse.holes = false;
    size_t s = MBV_NO_WINDOW;
    check(open_grid(sparse, nullptr, &s), "6b: the synthetic grid opens for a strip on the spheroid");
    struct mbview_struct *data = data_of(s);
    std::vector<float> values(data->primary_data, data->primary_data + data->primary_nxy);
    for (int i = 0; i <= rhi::kTileCells; i++)
      for (int j = 0; j <= rhi::kTileCells; j++)
        if (!(i <= 40 && j <= 40) && !(i >= 100 && i <= 104 && j >= 20 && j <= 100))
          values[static_cast<size_t>(i) * sparse.rows + j] = data->primary_nodatavalue;
    int error = MB_ERROR_NO_ERROR;
    mbview_updateprimarygrid(0, s, sparse.columns, sparse.rows, values.data(), &error);
    /* an overlay grid on the primary's nodes with data in columns 0 to 39 */
    const float nodatavalue = -88888.0f;
    std::vector<float> secondary(static_cast<size_t>(sparse.columns) * sparse.rows);
    for (int i = 0; i < sparse.columns; i++)
      for (int j = 0; j < sparse.rows; j++)
        secondary[static_cast<size_t>(i) * sparse.rows + j] = i < 40 ? static_cast<float>(10.0 + j % 11) : nodatavalue;
    char projection_id[MB_PATH_MAXLINE] = "EPSG:4326";
    const double dx = (sparse.xmax - sparse.xmin) / (sparse.columns - 1);
    const double dy = (sparse.ymax - sparse.ymin) / (sparse.rows - 1);
    mbview_setsecondarygrid(0, s, MBV_PROJECTION_GEOGRAPHIC, projection_id, nodatavalue, sparse.columns, sparse.rows,
                            10.0, 21.0, sparse.xmin, sparse.xmax, sparse.ymin, sparse.ymax, dx, dy, secondary.data(),
                            &error);
    mbview_setsecondarycolortable(0, s, MBV_COLORTABLE_HAXBY, MBV_COLORTABLE_NORMAL, 10.0, 21.0, 1.0, 15.5,
                                  MBV_COLORTABLE_NORMAL, &error);
    mbview_update(0, s, &error);
    mbvq_c_setprojection(s, 2);
    struct mbvq_grid g;
    mbvq_c_grid(s, &g);
    /* the core's 2D camera on the middle of the strip */
    mbvq::OffscreenRenderer cached(mbvq::OffscreenRenderer::Api::Metal);
    cached.render_map(s, kWidth, kHeight, options, nullptr);
    const int kc = 102 * g.n_rows + 60, kn = 103 * g.n_rows + 60;
    const double cell = std::hypot(g.x[kn] - g.x[kc], g.y[kn] - g.y[kc]);
    camera_2d(s, MBV_OPENGL_WIDTH / (12.0 * cell), -g.x[kc], -g.y[kc]);
    QImage culled, all;
    cached.render_map(s, kWidth, kHeight, options, &culled);
    {
      mbvq::OffscreenRenderer whole(mbvq::OffscreenRenderer::Api::Metal);
      whole.render_map(s, kWidth, kHeight, everything, &all);
    }
    const int strip_pixels = coloured(all), strip_differ = differing(culled, all);
    camera_2d(s, 1.0, 0.0, 0.0);
    /* the boxes made in the secondary grid mode, then the primary */
    mbvq_c_view(s, MBVQ_VIEW_GRID_SECONDARY, true);
    cached.render_map(s, kWidth, kHeight, options, nullptr);
    mbvq_c_view(s, MBVQ_VIEW_GRID_PRIMARY, true);
    QImage switched, switched_all;
    cached.render_map(s, kWidth, kHeight, options, &switched);
    {
      mbvq::OffscreenRenderer whole(mbvq::OffscreenRenderer::Api::Metal);
      whole.render_map(s, kWidth, kHeight, everything, &switched_all);
    }
    const int switch_pixels = coloured(switched_all), switch_differ = differing(switched, switched_all);
    check(data->display_projection_mode == MBV_PROJECTION_SPHEROID && strip_pixels > 1000 && strip_differ == 0 &&
              switch_pixels > 1000 && switch_differ == 0,
          "on the spheroid, culling on and off give the same picture zoomed onto a strip far from the rest of its "
          "tile's data (" + std::to_string(strip_differ) + " of " + std::to_string(strip_pixels) +
              " pixels differ), and after a switch from the secondary grid mode to the primary (" +
              std::to_string(switch_differ) + " of " + std::to_string(switch_pixels) + ")");
    mbvq_c_setprojection(s, 1);
    close_instance(s);
  }

  /* a cell edit beyond the grid's z range in a tile below the view: drawn,
     and the cached picture equals one made from nothing */
  {
    const Grid tall = tall_grid();
    size_t t = MBV_NO_WINDOW;
    check(open_grid(tall, nullptr, &t), "6b: a 200 x 700 grid opens for an edit below the view");
    rhi::MapFrame frame(t);
    frame.size_core(kWidth, kHeight);
    const Beside beside = find_beside(t, tall, frame);
    mbvq::OffscreenRenderer cached(mbvq::OffscreenRenderer::Api::Metal);
    QImage before, after, scratch, all;
    cached.render_map(t, kWidth, kHeight, options, &before);
    const mbvq::RenderStats seen = cached.stats();
    {
      mbvq::OffscreenRenderer whole(mbvq::OffscreenRenderer::Api::Metal);
      whole.render_map(t, kWidth, kHeight, everything, &all);
      check(seen.tiles_culled > 0 && whole.stats().tiles_culled == 0 &&
                whole.stats().tiles_drawn > seen.tiles_drawn && seen.stride == whole.stats().stride &&
                coloured(before) > 0 && differing(before, all) == 0,
            "culling on and off give the same picture (3D, " + std::to_string(seen.tiles_drawn) + " against " +
                std::to_string(whole.stats().tiles_drawn) + " tiles drawn): " +
                std::to_string(differing(before, all)) + " pixels differ");
    }
    int error = MB_ERROR_NO_ERROR;
    if (beside.tile >= 0)
      mbview_updateprimarygridcell(0, t, beside.i, beside.j, beside.value, &error);
    cached.render_map(t, kWidth, kHeight, options, &after);
    const mbvq::RenderStats edited = cached.stats();
    {
      mbvq::OffscreenRenderer fresh(mbvq::OffscreenRenderer::Api::Metal);
      fresh.render_map(t, kWidth, kHeight, options, &scratch);
    }
    check(beside.tile >= 0 && seen.tiles_culled > 0 && edited.tiles_drawn == seen.tiles_drawn + 1 &&
              coloured(after) > coloured(before) && differing(after, scratch) == 0,
          "a cell edit above the grid's range in a tile below the view: the tile drawn (" +
              std::to_string(seen.tiles_drawn) + " then " + std::to_string(edited.tiles_drawn) + " tiles), " +
              std::to_string(coloured(after) - coloured(before)) + " pixels more, " +
              std::to_string(differing(after, scratch)) + " pixels differ from a picture made from nothing");
    close_instance(t);
  }
}

/*--------------------------------------------------------------------*/
/* 6d on Metal: the soundings and the profile in pixels */

/* where the camera puts a sounding in a drawing of width x height pixels:
   window coordinates (x right, y up), as gluProject computes them */
std::array<double, 2> sdg_window(const std::vector<float> &xyz, size_t i, int width, int height) {
  double projection[16], modelview[16], camera[16];
  mbvq_c_soundings_camera(projection, modelview);
  rhi::multiply(projection, modelview, camera);
  const double p[4] = {xyz[3 * i], xyz[3 * i + 1], xyz[3 * i + 2], 1.0};
  double c[4] = {};
  for (int r = 0; r < 4; r++)
    for (int k = 0; k < 4; k++)
      c[r] += camera[4 * k + r] * p[k];
  return {(c[0] / c[3] + 1.0) * 0.5 * width, (c[1] / c[3] + 1.0) * 0.5 * height};
}

/* the colour of an image at window coordinates (y up) */
uint32_t at_window(const QImage &image, double x, double y) {
  const int column = static_cast<int>(std::floor(x));
  const int row = image.height() - 1 - static_cast<int>(std::floor(y));
  if (column < 0 || row < 0 || column >= image.width() || row >= image.height())
    return 0xffffffffu;
  return pixel_rgb(image, column, row);
}

uint32_t rgb_bytes(const unsigned char *rgba) {
  return static_cast<uint32_t>(rgba[0]) | static_cast<uint32_t>(rgba[1]) << 8 | static_cast<uint32_t>(rgba[2]) << 16;
}

int non_white(const QImage &image) {
  int n = 0;
  for (int row = 0; row < image.height(); row++)
    for (int x = 0; x < image.width(); x++)
      n += pixel_rgb(image, x, row) != kWhite;
  return n;
}

/* 6d: a batch's points drawn as quads, as where the API has no point size
   in the vertex shader (Direct3D; the information highlight of the 3D
   soundings view), against points: the same pixels at ratios 1 and 2, for
   points of two sizes after a line, so that the points do not begin at the
   batch's first vertex (the quads take their points per instance from
   there), each square of its size */
void metal_point_quads() {
  mbvq::OffscreenRenderer points(mbvq::OffscreenRenderer::Api::Metal);
  mbvq::OffscreenRenderer quads(mbvq::OffscreenRenderer::Api::Metal);
  for (int ratio = 1; ratio <= 2; ratio++) {
    TestBatch t;
    mbv_draw_batch_line_width(&t.b, 1.0f);
    t.primitive(MBV_DRAW_LINES, {{-0.9f, -0.60125f, 0}, {-0.1f, -0.60125f, 0}}, 0, 0, 1);
    mbv_draw_batch_point_size(&t.b, 3.0f);
    t.primitive(MBV_DRAW_POINTS, {{0.00125f, 0.00125f, 0}, {0.50125f, 0.25125f, 0}}, 1, 0, 0);
    mbv_draw_batch_point_size(&t.b, 5.0f);
    t.primitive(MBV_DRAW_POINTS, {{-0.49875f, 0.50125f, 0}}, 0, 1, 0);
    mbvq::RenderOptions plain;
    plain.pixel_ratio = ratio;
    mbvq::RenderOptions forced = plain;
    forced.point_quads = true;
    const int size = 200 * ratio;
    QImage a, b;
    const bool drawn = points.render_batch(&t.b, kIdentity, kIdentity, size, size, plain, &a) &&
                       quads.render_batch(&t.b, kIdentity, kIdentity, size, size, forced, &b);
    const int differ = differing(a, b);
    int red = 0, green = 0;
    for (int y = 0; drawn && y < size; y++)
      for (int x = 0; x < size; x++) {
        red += pixel_rgb(b, x, y) == kRed;
        green += pixel_rgb(b, x, y) == kGreen;
      }
    const int side3 = 3 * ratio, side5 = 5 * ratio;
    check(drawn && differ == 0 && red == 2 * side3 * side3 && green == side5 * side5 &&
              quads.stats().point_quad_draws == 2 && points.stats().point_quad_draws == 0,
          "6d Metal: a batch's points as forced quads (" + std::to_string(quads.stats().point_quad_draws) +
              " draws, after a line) give the pixels of its points at ratio " + std::to_string(ratio) + " (" +
              std::to_string(differ) + " differ; " + std::to_string(red) + " px of two " + std::to_string(side3) +
              " px squares, " + std::to_string(green) + " of a " + std::to_string(side5) + " px one)");
  }
}

void metal_soundings() {
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Metal);
  const int W = 400, H = 300;
  QImage image;
  int error = MB_ERROR_NO_ERROR;

  /* A sparse set, 8 pings of 10 beams about 35 px apart, seen from above;
     beam colours and flag reasons varied. Sounding 0 is flagged and lies
     above sounding 11 (same place, shallower): it comes first, and is not
     drawn without the flagged soundings, so it must not hide 11. Sounding 79
     is flagged alone. The camera has non-dyadic offsets and size, so that no
     point square has an edge on a pixel centre (F7). */
  make_sdg(8, 10, 0);
  const unsigned char flags[] = {MB_FLAG_FLAG | MB_FLAG_MANUAL, MB_FLAG_FLAG | MB_FLAG_FILTER,
                                 MB_FLAG_FLAG | MB_FLAG_SONAR, MB_FLAG_FLAG | MB_FLAG_FILTER2};
  for (size_t i = 0; i < sdg_storage.size(); i++) {
    sdg_storage[i].beamcolor = static_cast<int>(i % 8);
    if (i % 9 == 4)
      sdg_storage[i].beamflag = static_cast<char>(flags[(i / 9) % 4]);
  }
  sdg_storage[0].beamflag = static_cast<char>(MB_FLAG_FLAG | MB_FLAG_MANUAL);
  sdg_storage[0].x = sdg_storage[11].x;
  sdg_storage[0].y = sdg_storage[11].y;
  sdg_storage[0].z = sdg_storage[11].z + 10.0;
  sdg_storage[79].beamflag = static_cast<char>(MB_FLAG_FLAG | MB_FLAG_MANUAL);
  check(open_sdg(), "6d Metal: the soundings handed over");
  sdg_camera(90.0f, 0.0f, 1.0137f, 0.00123f, -0.00431f);

  /* the colours of the flag mode, flagged soundings shown and not */
  bool colours_ok = true;
  int checked = 0;
  std::string where;
  for (int flagged = 1; flagged >= 0; flagged--) {
    sdg_view(MBS_VIEW_COLOR_FLAG, flagged != 0, MBS_VIEW_PROFILES_NONE, false);
    const bool drawn = renderer.render_soundings(W, H, mbvq::RenderOptions(), &image);
    std::vector<float> xyz;
    std::vector<unsigned char> rgba;
    rhi::SoundingsFrame reader;
    reader.positions(static_cast<int>(sdg_storage.size()), &xyz);
    reader.colours(static_cast<int>(sdg_storage.size()), &rgba);
    colours_ok = colours_ok && drawn;
    for (size_t i = 1; i < sdg_storage.size(); i++) {
      const std::array<double, 2> w = sdg_window(xyz, i, W, H);
      /* sounding 0 shares the place of 11: drawn first and nearer, it shows
         there while flagged soundings are shown */
      const size_t shown = i == 11 && rgba[3] != 0 ? 0 : i;
      const uint32_t expected = rgba[4 * shown + 3] != 0 ? rgb_bytes(&rgba[4 * shown]) : kWhite;
      const uint32_t got = at_window(image, w[0], w[1]);
      if (got != expected && where.size() < 120)
        where += std::to_string(i) + ":" + colour_name(got) + " ";
      colours_ok = colours_ok && got == expected;
      checked++;
    }
    /* flagged soundings not shown: 79 leaves the background, 0 does not hide 11 */
    if (flagged == 0) {
      const std::array<double, 2> w79 = sdg_window(xyz, 79, W, H);
      const std::array<double, 2> w11 = sdg_window(xyz, 11, W, H);
      colours_ok = colours_ok && rgba[3] == 0 && rgba[4 * 79 + 3] == 0 && at_window(image, w79[0], w79[1]) == kWhite &&
                   at_window(image, w11[0], w11[1]) == rgb_bytes(&rgba[4 * 11]);
    }
  }
  check(colours_ok, "6d Metal: every sounding shows the colour of mb3dsoundings_colors() at its place, flagged ones "
                    "in their flag's colour (" + std::to_string(checked) + " places" +
                        (where.empty() ? std::string() : ", differ: " + where) +
                        "); a sounding not drawn leaves the background and hides none behind it");

  /* point sizes 1 and 6 (the control), and points against quads */
  sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_NONE, false);
  mbvq::RenderOptions one;
  one.soundings_point_size = 1.0;
  mbvq::RenderOptions six;
  six.soundings_point_size = 6.0;
  renderer.render_soundings(W, H, one, &image);
  const int one_px = non_white(image);
  renderer.render_soundings(W, H, six, &image);
  const int six_px = non_white(image);
  check(one_px > 0 && six_px == 36 * one_px,
        "6d Metal: points of 6 px cover 36 times the pixels of points of 1 px (" + std::to_string(six_px) + " and " +
            std::to_string(one_px) + ")");
  {
    mbvq::OffscreenRenderer quads(mbvq::OffscreenRenderer::Api::Metal);
    mbvq::RenderOptions forced;
    forced.point_quads = true;
    QImage points_image, quads_image;
    bool same = true;
    std::string sizes;
    for (double size : {1.0, 3.0, 4.5}) {
      forced.soundings_point_size = size;
      mbvq::RenderOptions plain;
      plain.soundings_point_size = size;
      renderer.render_soundings(W, H, plain, &points_image);
      quads.render_soundings(W, H, forced, &quads_image);
      const int differ = differing(points_image, quads_image);
      sizes += std::to_string(differ) + " ";
      same = same && differ == 0 && quads.stats().point_quads && !renderer.stats().point_quads;
    }
    check(same, "6d Metal: points and forced point quads give the same pixels at 1, 3 and 4.5 logical px (" + sizes +
                    "pixels differ)");
  }

  /* the screen positions computed on the CPU (mb3dsoundings_screen_positions)
     against the drawn points */
  {
    renderer.render_soundings(W, H, mbvq::RenderOptions(), &image);
    mbvq_c_soundings_resize(W, H);
    mb3dsoundings_screen_positions(0, &error);
    std::vector<float> xyz;
    std::vector<unsigned char> rgba;
    rhi::SoundingsFrame reader;
    reader.positions(static_cast<int>(sdg_storage.size()), &xyz);
    reader.colours(static_cast<int>(sdg_storage.size()), &rgba);
    int inside = 0, total = 0;
    double worst = 0.0;
    for (size_t i = 1; i < sdg_storage.size(); i++) {
      /* not drawn, white on white, or under sounding 0 */
      if (rgba[4 * i + 3] == 0 || rgb_bytes(&rgba[4 * i]) == kWhite || i == 11)
        continue;
      const uint32_t colour = rgb_bytes(&rgba[4 * i]);
      const int x = sdg_storage[i].winx, y = sdg_storage[i].winy;
      /* the drawn square of 3 px around the point: its pixels of the colour */
      double sx = 0.0, sy = 0.0;
      int n = 0;
      for (int dy = -3; dy <= 3; dy++)
        for (int dx = -3; dx <= 3; dx++)
          if (at_window(image, x + dx + 0.5, y + dy + 0.5) == colour) {
            sx += x + dx + 0.5;
            sy += y + dy + 0.5;
            n++;
          }
      const std::array<double, 2> w = sdg_window(xyz, i, W, H);
      total++;
      if (n == 9) {
        worst = std::max(worst, std::max(std::fabs(sx / n - w[0]), std::fabs(sy / n - w[1])));
        inside += at_window(image, x + 0.5, y + 0.5) == colour && x == static_cast<int>(w[0]) &&
                  y == static_cast<int>(w[1]);
      }
    }
    check(total > 0 && inside == total && worst < 0.5,
          "6d Metal: the screen positions computed on the CPU name the pixel of every drawn point (" +
              std::to_string(inside) + " of " + std::to_string(total) + "; the drawn 3 x 3 squares centred within " +
              text("%.3f", worst) + " px of the projected places)");
  }

  /* the grab rectangle, drawn with the projection alone (camera == false):
     at its window corners whatever the camera */
  {
    sdg_camera(35.0f, 125.0f, 1.3f, 0.05f, -0.05f);
    /* no sounding yellow */
    for (struct mb3dsoundings_sounding_struct &sounding : sdg_storage)
      sounding.beamcolor = MBV_COLOR_BLACK;
    sdg_view(MBS_VIEW_COLOR_FLAG, false, MBS_VIEW_PROFILES_NONE, false);
    mbvq_c_soundings_resize(W, H);
    mb3dsoundings.edit_mode = MBS_EDIT_GRAB;
    mb3dsoundings.button1down = true;
    mb3dsoundings.grab_start_defined = true;
    mb3dsoundings.grab_end_defined = true;
    mb3dsoundings.grab_start_x = 101;
    mb3dsoundings.grab_start_y = 77;
    mb3dsoundings.grab_end_x = 287;
    mb3dsoundings.grab_end_y = 203;
    renderer.render_soundings(W, H, mbvq::RenderOptions(), &image);
    const uint32_t kYellow = 0x00ffff;
    int left = 0, right = 0, bottom = 0, top = 0, inner = 0;
    int xmin = W, xmax = -1, ymin = H, ymax = -1;
    for (int row = 0; row < H; row++)
      for (int x = 0; x < W; x++) {
        if (pixel_rgb(image, x, row) != kYellow)
          continue;
        const int y = H - 1 - row;
        xmin = std::min(xmin, x);
        xmax = std::max(xmax, x);
        ymin = std::min(ymin, y);
        ymax = std::max(ymax, y);
        left += std::abs(x - 101) <= 2;
        right += std::abs(x - 287) <= 2;
        bottom += std::abs(y - 77) <= 2;
        top += std::abs(y - 203) <= 2;
        inner += x > 104 && x < 284 && y > 80 && y < 200;
      }
    mb3dsoundings.button1down = false;
    mb3dsoundings.grab_start_defined = false;
    mb3dsoundings.grab_end_defined = false;
    mb3dsoundings.edit_mode = MBS_EDIT_TOGGLE;
    check(left > 3 * 100 && right > 3 * 100 && bottom > 3 * 150 && top > 3 * 150 && inner == 0 && xmin >= 99 &&
              xmax <= 289 && ymin >= 75 && ymax <= 205,
          "6d Metal: the grab rectangle (3 px, the projection alone) lies at its window corners 101 77 287 203 "
          "under a rotated camera (yellow from " + std::to_string(xmin) + " " + std::to_string(ymin) + " to " +
              std::to_string(xmax) + " " + std::to_string(ymax) + ")");
  }

  /* The profile lines: within a ping, pairs of beams 35 px apart; black
     where both soundings are good, red where one is flagged; none between
     pings. At ratio 1 one pixel thick from the index lists; at ratio 2 two
     pixels by the minor-axis rule, from the instanced quads. Seen at azimuth
     0 the lines run across the window (x-major), at azimuth 90 up it
     (y-major); the line's pixels are counted along its minor axis. */
  for (int view = 0; view < 4; view++) {
    const int ratio = 1 + view % 2;
    const float azimuth = view < 2 ? 0.0f : 90.0f;
    sdg_camera(90.0f, azimuth, 1.0137f, 0.00123f, -0.00431f);
    sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, false);
    mbvq::RenderOptions options;
    options.pixel_ratio = ratio;
    renderer.render_soundings(W * ratio, H * ratio, options, &image);
    const mbvq::RenderStats s = renderer.stats();
    std::vector<float> xyz;
    std::vector<uint32_t> black, red;
    rhi::SoundingsFrame reader;
    reader.positions(static_cast<int>(sdg_storage.size()), &xyz);
    reader.pairs(static_cast<int>(sdg_storage.size()), &black, &red);
    std::vector<int> kind(sdg_storage.size(), 0);
    for (size_t k = 0; k + 1 < black.size(); k += 2)
      kind[black[k]] = 1;
    for (size_t k = 0; k + 1 < red.size(); k += 2)
      kind[red[k]] = 2;
    int good = 0, bad = 0;
    std::string thick;
    for (size_t i = 1; i + 1 < sdg_storage.size(); i++) {
      /* the middle of the segment from i to i + 1 */
      const std::array<double, 2> a = sdg_window(xyz, i, W * ratio, H * ratio);
      const std::array<double, 2> b = sdg_window(xyz, i + 1, W * ratio, H * ratio);
      const double mx = 0.5 * (a[0] + b[0]), my = 0.5 * (a[1] + b[1]);
      const uint32_t expected = kind[i] == 1 ? 0x000000u : kind[i] == 2 ? kRed : kWhite;
      if (kind[i] == 0) {
        good += at_window(image, mx, my) == kWhite;
        bad += at_window(image, mx, my) != kWhite;
        continue;
      }
      /* the line's pixels across it in the middle: as many as its width */
      const bool xmajor = std::fabs(b[0] - a[0]) >= std::fabs(b[1] - a[1]);
      int n = 0;
      for (int d = -4 * ratio; d <= 4 * ratio; d++)
        n += (xmajor ? at_window(image, mx, my + d) : at_window(image, mx + d, my)) == expected;
      if (n == ratio && xmajor == (azimuth == 0.0f))
        good++;
      else {
        bad++;
        if (thick.size() < 80)
          thick += std::to_string(i) + ":" + std::to_string(n) + " ";
      }
    }
    check(bad == 0 && good > 0 && s.profile_wide == (ratio == 2) &&
              (ratio == 1 ? s.profile_indices > 0 && s.profile_instances == 0
                          : s.profile_instances > 0 && s.profile_indices == 0),
          "6d Metal: at ratio " + std::to_string(ratio) + " and azimuth " + std::to_string(static_cast<int>(azimuth)) +
              " the profile lines (" + (azimuth == 0.0f ? "x" : "y") + "-major) are " +
              (ratio == 1 ? "drawn thin from the index lists" : "drawn 2 px wide by the instanced quads") +
              ", black and red where the core's pairs are and nothing between pings (" + std::to_string(good) +
              " places right" + (thick.empty() ? std::string() : ", wrong: " + thick) + ")");
  }
  /* The stipple values follow the camera also in a frame that draws the
     batch it has: a fill refused (another fill runs) after the camera
     changed back draws the dotted box as a fill with that camera does (the
     same elevation and azimuth, so the same batch). */
  {
    sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_NONE, true);
    QImage expected;
    sdg_camera(35.0f, 125.0f, 1.3f, 0.05f, -0.05f);
    renderer.render_soundings(W, H, mbvq::RenderOptions(), &expected);
    sdg_camera(35.0f, 125.0f, 1.5f, 0.1f, 0.0f);
    renderer.render_soundings(W, H, mbvq::RenderOptions(), nullptr);
    const int uploads = renderer.stats().stipple_uploads;
    const int fills = renderer.stats().fills;
    sdg_camera(35.0f, 125.0f, 1.3f, 0.05f, -0.05f);
    TestBatch other;
    mbv_draw_set_target(&other.b);
    renderer.render_soundings(W, H, mbvq::RenderOptions(), &image);
    mbv_draw_set_target(nullptr);
    const int differ = differing(image, expected);
    check(differ == 0 && renderer.stats().stipple_uploads == uploads + 1 && renderer.stats().fills == fills,
          "6d Metal: after a refused fill the dotted box is drawn with the stipple of the new camera (" +
              std::to_string(differ) + " pixels differ from a fill with that camera; " +
              std::to_string(renderer.stats().stipple_uploads - uploads) + " stipple upload)");
  }

  /* Every soundings frame on the same frame slot (an empty frame of the
     QRhi after each, as a window that draws a frame of its own between two
     frames of the view), with new positions in every frame (an exaggeration
     drag over 50,000 soundings, 600 KB of positions), so the malloc heap
     stays flat over 90 frames (8 MB allowed). The positions are static
     buffers with storage use (7d1), which Qt's Metal backend keeps once, not
     per frame slot: each upload waits for the next bind, on either slot, and
     the frame binds it. Then again with the positions dynamic
     (chunk_storage 0, 6d's type): written whole, they leave one pending write
     for the other slot. Static buffers without storage use would keep every
     frame's upload pending there (54 MB). Each frame and the empty one after
     it run in an autorelease pool (FramePool, as 6c's batch check). */
  for (const int chunk_storage : {-1, 0}) {
    make_sdg(200, 250, 23);
    open_sdg();
    sdg_camera(35.0f, 125.0f, 1.3f, 0.05f, -0.05f);
    sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_NONE, false);
    mbvq::OffscreenRenderer slot_renderer(mbvq::OffscreenRenderer::Api::Metal);
    mbvq::RenderOptions slot_options;
    slot_options.chunk_storage = chunk_storage;
    bool drawn = true;
    int on_slot[2] = {0, 0};
    double heap_start = 0.0;
    const int writes = 0;
    for (int f = 0; f < 120 && drawn; f++) {
      {
        const FramePool pool;
        mb3dsoundings.exaggeration = 1.0f + 0.01f * (f % 50);
        mb3dsoundings_scalez(0, &error);
        drawn = slot_renderer.render_soundings(W, H, slot_options, nullptr, 1);
        on_slot[slot_renderer.stats().frame_slot == 0 ? 0 : 1]++;
        drawn = drawn && slot_renderer.empty_frame();
      }
      if (f + 1 == 30)
        heap_start = heap_in_use();
    }
    const double heap = heap_in_use() - heap_start;
    const mbvq::RenderStats st = slot_renderer.stats();
    const bool storage = chunk_storage != 0;
    const bool chunks_ok =
        st.soundings_storage == (storage ? 1 : 0) && st.soundings_applied == 0 && st.soundings_rebuilt == 0;
    check(drawn && (on_slot[0] == 120 || on_slot[1] == 120) && st.sounding_writes[rhi::SoundingsPlan::kPositions] ==
                                                                    writes + 120 &&
              heap_start > 0.0 && heap < 8.0 && chunks_ok,
          (storage ? std::string("6d Metal: 90 frames of new positions for 50,000 soundings, all on one frame slot")
                   : std::string("7d1 Metal: the same 90 frames with the positions dynamic (chunk_storage 0)")) +
              std::string(FramePool::available() ? ", each in an autorelease pool" : "") +
              ", leave the malloc heap flat (" + text("%+.1f", heap) + " MB, less than 8 MB)" +
              (chunks_ok ? std::string()
                         : "; storage " + std::to_string(st.soundings_storage) + ", applied " +
                               std::to_string(st.soundings_applied) + ", rebuilt " +
                               std::to_string(st.soundings_rebuilt)));
  }

  /* 7d1: the colours and the profile lines written on one frame slot, with a
     colour mark in every frame and the data unchanged, so that every buffer
     keeps its size and is written again, not made again: the colours
     (storage chunks), and the index lists at ratio 1 (thin lines) or the
     colours per pair at ratio 2 (wide), both dynamic */
  for (const double ratio : {1.0, 2.0}) {
    make_sdg(200, 250, 23);
    open_sdg();
    sdg_camera(35.0f, 125.0f, 1.3f, 0.05f, -0.05f);
    sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, false);
    mbvq::OffscreenRenderer colour_renderer(mbvq::OffscreenRenderer::Api::Metal);
    mbvq::RenderOptions colour_options;
    colour_options.pixel_ratio = ratio;
    bool drawn = true;
    int on_slot[2] = {0, 0};
    double heap_start = 0.0;
    mbvq::RenderStats at30;
    for (int f = 0; f < 120 && drawn; f++) {
      {
        const FramePool pool;
        mb3dsoundings_colors_changed(0, &error);
        drawn = colour_renderer.render_soundings(W, H, colour_options, nullptr, 1);
        on_slot[colour_renderer.stats().frame_slot == 0 ? 0 : 1]++;
        drawn = drawn && colour_renderer.empty_frame();
      }
      if (f + 1 == 30) {
        heap_start = heap_in_use();
        at30 = colour_renderer.stats();
      }
    }
    const double heap = heap_in_use() - heap_start;
    const mbvq::RenderStats st = colour_renderer.stats();
    const bool wide = ratio > 1.5;
    const int stream = wide ? rhi::SoundingsPlan::kPairColours : rhi::SoundingsPlan::kLists;
    const int colour_writes = st.sounding_writes[rhi::SoundingsPlan::kColours];
    const int line_writes = st.sounding_writes[stream];
    /* the lines drawn alike at frame 30 and at the end: the buffers kept their sizes */
    const bool same = wide ? st.profile_instances == at30.profile_instances && st.profile_instances > 0
                           : st.profile_indices == at30.profile_indices && st.profile_indices > 0;
    check(drawn && (on_slot[0] == 120 || on_slot[1] == 120) && st.profile_wide == wide && colour_writes == 120 &&
              line_writes == 120 && same && heap_start > 0.0 && heap < 8.0 && st.soundings_storage == 1,
          "7d1 Metal: 90 frames of new colours for 50,000 soundings with the profile lines " +
              std::string(wide ? "wide (the colours per pair" : "thin (the index lists") +
              " written in every frame too, their sizes kept), all on one frame slot (" +
              std::to_string(std::max(on_slot[0], on_slot[1])) + " of 120 frames), leave the malloc heap flat (" +
              text("%+.1f", heap) + " MB, less than 8 MB; " + std::to_string(colour_writes) + " colour and " +
              std::to_string(line_writes) + " line writes, storage " + std::to_string(st.soundings_storage) + ")");
  }

  /* 7d1: writes without a bind. 100,000 soundings in chunks of 36,666, a
     budget of 1,250,000 bytes, thin profile lines (ratio 1): a positions
     change before every frame from the allocation's first frame writes the
     positions whole in every frame from the third (1,200,024 bytes) and
     leaves too little of the budget for the last chunk's colours, so that
     chunk is not drawn while the drag lasts (6d's plan) and its positions
     are written in every frame without a draw. The frame binds them without
     a draw, so Qt copies each upload at once and the heap stays flat; once
     the drag stops, the last chunk is drawn. */
  {
    make_sdg(400, 250, 23);
    open_sdg();
    sdg_camera(35.0f, 125.0f, 1.3f, 0.05f, -0.05f);
    sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, false);
    mbvq::RenderOptions starve;
    starve.soundings_chunk = 36666;
    starve.upload_budget = 1250000;
    mbvq::OffscreenRenderer starved(mbvq::OffscreenRenderer::Api::Metal);
    bool drawn = true;
    double heap_start = 0.0;
    mbvq::RenderStats at30;
    for (int f = 0; f < 120 && drawn; f++) {
      {
        const FramePool pool;
        mb3dsoundings.exaggeration = 1.0f + 0.01f * (f % 50);
        mb3dsoundings_scalez(0, &error);
        drawn = starved.render_soundings(W, H, starve, nullptr, 1);
        drawn = drawn && starved.empty_frame();
      }
      if (f + 1 == 30) {
        heap_start = heap_in_use();
        at30 = starved.stats();
      }
    }
    const double heap = heap_in_use() - heap_start;
    const mbvq::RenderStats st = starved.stats();
    const int position_writes =
        st.sounding_writes[rhi::SoundingsPlan::kPositions] - at30.sounding_writes[rhi::SoundingsPlan::kPositions];
    const int applied = st.soundings_applied - at30.soundings_applied;
    QImage last, now;
    const bool still = starved.render_soundings(W, H, starve, &last, 50);
    const mbvq::RenderStats end = starved.stats();
    mbvq::OffscreenRenderer fresh(mbvq::OffscreenRenderer::Api::Metal);
    fresh.render_soundings(W, H, starve, &now, 50);
    const int differ = differing(last, now);
    check(drawn && heap_start > 0.0 && !st.profile_wide && position_writes == 270 && st.soundings_shown == 73332 &&
              applied == 90 && st.soundings_rebuilt == 0 && heap < 8.0 && still && end.soundings_shown == 100000 &&
              !end.soundings_waiting && differ == 0 && non_white(now) > 0,
          "7d1 Metal: 90 frames of a drag that starts with a first upload, whose last chunk's colours never fit "
          "(" + std::to_string(position_writes) + " position writes, " + std::to_string(st.soundings_shown) +
              " of 100,000 soundings drawn): the last chunk's positions bound without a draw in their frame (" +
              std::to_string(applied) + " times, " + std::to_string(st.soundings_rebuilt) +
              " buffers made again), the malloc heap flat (" + text("%+.1f", heap) +
              " MB, less than 8 MB); after the drag " + std::to_string(end.soundings_shown) + " drawn" +
              (end.soundings_waiting ? ", uploads still waiting" : "") + ", " + std::to_string(differ) +
              " px from a renderer made now");
  }

  /* 7d1: the soundings drawn alike with the storage chunks and with the
     chunks dynamic (chunk_storage 0): as points and as forced quads with the
     profile lines thin (ratio 1), and as points with them wide (ratio 2) */
  {
    make_sdg(40, 25, 7);
    open_sdg();
    sdg_camera(25.0f, 35.0f, 1.0f, 0.0f, 0.0f);
    sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, true);
    std::vector<int> differ, storage, drawn;
    bool ok = true;
    for (int c = 0; c < 3; c++) {
      QImage pictures[2];
      for (int k = 0; k < 2; k++) {
        mbvq::RenderOptions options;
        options.soundings_chunk = 300;
        options.point_quads = c == 1;
        options.pixel_ratio = c == 2 ? 2.0 : 1.0;
        options.chunk_storage = k == 0 ? -1 : 0;
        mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Metal);
        ok = renderer.render_soundings(W, H, options, &pictures[k], 50) && ok;
        const mbvq::RenderStats s = renderer.stats();
        storage.push_back(s.soundings_storage);
        drawn.push_back(s.soundings_drawn);
        ok = ok && s.soundings_storage == (k == 0 ? 1 : 0) && s.soundings_drawn == 1000 &&
             s.profile_wide == (c == 2) && s.point_quads == (c == 1);
      }
      differ.push_back(differing(pictures[0], pictures[1]));
      ok = ok && non_white(pictures[0]) > 0;
    }
    check(ok && differ == std::vector<int>{0, 0, 0},
          "7d1 Metal: the soundings drawn with the storage chunks and with the chunks dynamic (chunk_storage 0) are "
          "alike as points and as forced quads with thin profile lines and as points with wide ones (" +
              list(differ) + " px differ; storage " + list(storage) + ", soundings drawn " + list(drawn) + ")");
  }

  /* 7d1: the GPU bytes of the soundings' buffers on Metal, each buffer by
     its own copies: the positions and colours (storage) once, the index
     lists and colours per pair (dynamic) twice; with the chunks dynamic
     (chunk_storage 0) everything twice. 1000 soundings in 4 chunks of 300,
     below the budget (one set per stream), a renderer for each profile mode
     and no change mark between frames: 12 x (1000 + 3) + 4 x 1000 = 16,036
     bytes of positions and colours; the thin lines' lists 7,680 and the wide
     lines' colours per pair 3,996 */
  {
    make_sdg(40, 25, 7);
    open_sdg();
    sdg_camera(25.0f, 35.0f, 1.0f, 0.0f, 0.0f);
    const int modes[3] = {MBS_VIEW_PROFILES_NONE, MBS_VIEW_PROFILES_ALL, MBS_VIEW_PROFILES_ALL};
    const size_t expected[3][2] = {{16036, 32072}, {31396, 47432}, {24028, 40064}};
    std::vector<int> held;
    bool ok = true;
    for (int m = 0; m < 3; m++) {
      sdg_view(MBS_VIEW_COLOR_FLAG, true, modes[m], true);
      for (int k = 0; k < 2; k++) {
        mbvq::RenderOptions options;
        options.soundings_chunk = 300;
        options.pixel_ratio = m == 2 ? 2.0 : 1.0;
        options.chunk_storage = k == 0 ? -1 : 0;
        mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Metal);
        ok = renderer.render_soundings(W, H, options, nullptr, 50) && ok;
        const mbvq::RenderStats s = renderer.stats();
        held.push_back(static_cast<int>(s.sounding_held_bytes));
        ok = ok && s.sounding_held_bytes == expected[m][k] && !s.soundings_waiting;
      }
    }
    check(ok, "7d1 Metal: the soundings' GPU bytes with the storage chunks and with the chunks dynamic, without "
              "profile lines, thin and wide: " +
                  list(held) + " (expected 16036, 32072; 31396, 47432; 24028, 40064)");
  }

  /* The frames a view draws for a drag's last motion, which came while the
     split upload of the motion before ran (1000 soundings in chunks of 250,
     a budget of 5000 bytes): the last picture is the core's newest state,
     as a renderer made now draws it (the dotted box from the batch, the
     soundings from the newest positions). */
  {
    make_sdg(40, 25, 7);
    open_sdg();
    sdg_camera(25.0f, 35.0f, 1.0f, 0.0f, 0.0f);
    sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_NONE, true);
    mb3dsoundings.exaggeration = 3.0f;
    mb3dsoundings_scalez(0, &error);
    mbvq::RenderOptions split;
    split.soundings_chunk = 250;
    split.upload_budget = 5000;
    mbvq::OffscreenRenderer dragged(mbvq::OffscreenRenderer::Api::Metal);
    dragged.render_soundings(W, H, split, nullptr, 50);
    mb3dsoundings.exaggeration = 3.5f;
    mb3dsoundings_scalez(0, &error);
    dragged.render_soundings(W, H, split, nullptr, 1);
    const bool running = dragged.stats().soundings_waiting;
    mb3dsoundings.exaggeration = 4.0f;
    mb3dsoundings_scalez(0, &error);
    QImage last, now;
    const bool drawn = dragged.render_soundings(W, H, split, &last, 50);
    const int frames = dragged.last_frames();
    mbvq::OffscreenRenderer fresh(mbvq::OffscreenRenderer::Api::Metal);
    fresh.render_soundings(W, H, split, &now, 50);
    const int differ = differing(last, now);
    /* 7d1: the writes into the second set (storage chunks) bound without a
       draw in their frames: P0 of the 3.5 motion's job, P1 and P2 of it,
       then P0 to P2 of the 4.0 motion's job (each job's P3 swaps and is
       drawn) */
    const mbvq::RenderStats ds = dragged.stats();
    const bool second_ok = ds.soundings_applied == 6 && ds.soundings_rebuilt == 0;
    check(drawn && running && frames < 50 && !ds.soundings_waiting && differ == 0 && non_white(now) > 0 && second_ok,
          "6d Metal: the frames a view draws for a drag's last motion during the split upload of the one before end "
          "with the core's newest state (" + std::to_string(frames) + " frames; " + std::to_string(differ) +
              " px differ from a renderer made now)" +
              (second_ok ? std::string()
                         : "; applied " + std::to_string(ds.soundings_applied) + ", rebuilt " +
                               std::to_string(ds.soundings_rebuilt)));
  }

  /* 7d1: double-buffered colours, their second set bound without a draw in
     the frame of each write before the swap: 1000 soundings in chunks of
     250 and a budget of 3,500 bytes (positions 12,036 and colours 4,000
     bytes, both double-buffered, no profile lines); after the first upload a
     colour mark writes the colours into the second set, three chunks of
     1,000 bytes in one frame, bound without a draw, and the fourth in the
     next, which swaps; the last picture as a renderer made now draws it */
  {
    make_sdg(40, 25, 7);
    open_sdg();
    sdg_camera(25.0f, 35.0f, 1.0f, 0.0f, 0.0f);
    sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_NONE, true);
    mbvq::RenderOptions split;
    split.soundings_chunk = 250;
    split.upload_budget = 3500;
    mbvq::OffscreenRenderer coloured(mbvq::OffscreenRenderer::Api::Metal);
    const bool first = coloured.render_soundings(W, H, split, nullptr, 50);
    const mbvq::RenderStats before = coloured.stats();
    sdg_storage[12].beamflag = static_cast<char>(MB_FLAG_FLAG | MB_FLAG_MANUAL);
    mb3dsoundings_colors_changed(0, &error);
    QImage last, now;
    const bool drawn = coloured.render_soundings(W, H, split, &last, 50);
    const mbvq::RenderStats after = coloured.stats();
    mbvq::OffscreenRenderer fresh(mbvq::OffscreenRenderer::Api::Metal);
    fresh.render_soundings(W, H, split, &now, 50);
    const int differ = differing(last, now);
    const int writes =
        after.sounding_writes[rhi::SoundingsPlan::kColours] - before.sounding_writes[rhi::SoundingsPlan::kColours];
    const int applied = after.soundings_applied - before.soundings_applied;
    const int swaps = after.sounding_swaps - before.sounding_swaps;
    check(first && drawn && !before.soundings_waiting && !after.soundings_waiting && writes == 4 && swaps == 1 &&
              applied == 3 && after.soundings_rebuilt == 0 && differ == 0 && non_white(now) > 0,
          "7d1 Metal: a colour mark over double-buffered colours (1000 soundings in chunks of 250, a budget of 3,500 "
          "bytes): " + std::to_string(writes) + " colour chunks written into the second set, " +
              std::to_string(applied) + " of them bound without a draw before the swap (" + std::to_string(swaps) +
              " swap), " + std::to_string(after.soundings_rebuilt) + " buffers made again; " +
              std::to_string(differ) + " px from a renderer made now");
  }
  mb3dsoundings_end(0, &error);

  /* stipple 0x1111 along a horizontal line: 1 on and 3 off at ratio 1, 2 on
     and 6 off at ratio 2 (factor 2, the line 2 px wide) */
  for (int ratio = 1; ratio <= 2; ratio++) {
    TestBatch t;
    mbv_draw_batch_enable(&t.b, MBV_DRAW_LINE_STIPPLE, true);
    mbv_draw_batch_line_stipple(&t.b, 1, 0x1111);
    t.primitive(MBV_DRAW_LINE_STRIP, {{-0.9f, 0.013f, 0}, {0.9f, 0.013f, 0}}, 1, 0, 0);
    mbvq::RenderOptions options;
    options.pixel_ratio = ratio;
    const int size = 200 * ratio;
    const bool drawn = renderer.render_batch(&t.b, kIdentity, kIdentity, size, size, options, &image);
    /* the runs along the line, from its third on-run to its third-last */
    const int row = image.height() - 1 - static_cast<int>(std::floor((0.013 + 1.0) * 0.5 * size));
    std::vector<int> on, off;
    int run = 0;
    bool lit = false;
    for (int x = 0; x < image.width(); x++) {
      const bool red_here = pixel_rgb(image, x, row) == kRed;
      if (x > 0 && red_here != lit) {
        (lit ? on : off).push_back(run);
        run = 0;
      }
      lit = red_here;
      run++;
    }
    const bool runs = on.size() > 10 && off.size() > 10 &&
                      std::all_of(on.begin() + 2, on.end() - 2, [ratio](int n) { return n == ratio; }) &&
                      std::all_of(off.begin() + 3, off.end() - 2, [ratio](int n) { return n == 3 * ratio; });
    check(drawn && runs,
          "6d Metal: stipple 0x1111 runs " + std::to_string(ratio) + " on " + std::to_string(3 * ratio) +
              " off at ratio " + std::to_string(ratio) + " (" + std::to_string(on.size()) + " dots)");
  }
}

/* the profile view on Metal, of a profile made by hand: black and red quads
   by the slope threshold, a green boundary, the frame black, red when a
   value is clipped */
void metal_profile() {
  Grid grid;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "6d Metal: a map for the profile");
  struct mbview_struct *data = data_of(instance);
  const int n = 7;
  int error = MB_ERROR_NO_ERROR;
  if (data->profile.npoints_alloc < n &&
      mbview_allocprofilepoints(0, n, &data->profile.points, &error) == MB_SUCCESS)
    data->profile.npoints_alloc = n;
  const double z[n] = {-1000.0, -985.0, -1012.0, -995.0, -1003.0, -990.0, -1008.0};
  data->profile.npoints = n;
  data->profile.length = 600.0;
  data->profile.zmin = -1012.0;
  data->profile.zmax = -985.0;
  for (int i = 0; i < n; i++) {
    struct mbview_profilepoint_struct &p = data->profile.points[i];
    p.boundary = i == 0 || i == 3 || i == n - 1;
    p.distance = 100.0 * i;
    p.zdata = z[i];
    p.slope = i % 2 == 0 ? 0.1 : 0.9;
  }
  data->profile_slopethreshold = 0.5;
  data->profile_exaggeration = 1.0;
  mbvq_c_profile_show(instance, true);

  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Metal);
  const int W = 1200, H = 300;
  QImage image;
  for (int clipped = 0; clipped <= 1; clipped++) {
    if (clipped)
      data->profile.points[2].zdata = -1200.0;
    const bool drawn = renderer.render_profile(instance, W, H, mbvq::RenderOptions(), &image);
    /* the drawing's coordinates (mbview_plotprofile) to window pixels with
       the profile's camera */
    double projection[16], modelview[16], camera[16];
    mbvq_c_profile_camera(instance, projection, modelview);
    rhi::multiply(projection, modelview, camera);
    const auto window = [&](double x, double y) {
      const double cx = camera[0] * x + camera[4] * y + camera[8] * MBV_OPENGL_ZPROFILE1 + camera[12];
      const double cy = camera[1] * x + camera[5] * y + camera[9] * MBV_OPENGL_ZPROFILE1 + camera[13];
      const double cw = camera[3] * x + camera[7] * y + camera[11] * MBV_OPENGL_ZPROFILE1 + camera[15];
      return std::array<double, 2>{(cx / cw + 1.0) * 0.5 * W, (cy / cw + 1.0) * 0.5 * H};
    };
    const double scale = MBV_OPENGL_WIDTH / data->profile.length;
    const double zcenter = 0.5 * (data->profile.zmax + data->profile.zmin);
    const double aspect = static_cast<double>(H) / W;
    const double yzmin = scale * (-0.5 * aspect * data->profile.length);
    const double yzmax = -yzmin;
    int quads = 0;
    std::string wrong;
    for (int i = 0; i + 1 < n; i++) {
      const double y0 = std::min(std::max(scale * (data->profile.points[i].zdata - zcenter), yzmin), yzmax);
      const double y1 = std::min(std::max(scale * (data->profile.points[i + 1].zdata - zcenter), yzmin), yzmax);
      /* halfway between the bottom and the top edge in the middle of the quad */
      const std::array<double, 2> w = window(scale * (100.0 * i + 50.0), 0.5 * (yzmin + 0.5 * (y0 + y1)));
      const uint32_t expected = data->profile.points[i].slope < 0.5 ? 0x000000u : kRed;
      const bool ok = at_window(image, w[0], w[1]) == expected;
      quads += ok;
      if (!ok)
        wrong += std::to_string(i) + ":" + colour_name(at_window(image, w[0], w[1])) + " ";
    }
    /* the green boundary at point 3, 2 px wide, over the quads */
    const std::array<double, 2> boundary = window(scale * 300.0, 0.5 * (yzmin + yzmax));
    int green = 0;
    for (int dx = -3; dx <= 3; dx++)
      green += at_window(image, boundary[0] + dx, boundary[1]) == kGreen;
    /* the frame's top edge, 2 px, black or red */
    const std::array<double, 2> frame = window(0.5 * MBV_OPENGL_WIDTH, yzmax);
    int framed = 0;
    for (int dy = -3; dy <= 3; dy++)
      framed += at_window(image, frame[0], frame[1] + dy) == (clipped ? kRed : 0x000000u);
    check(drawn && quads == n - 1 && green == 2 && framed == 2,
          std::string("6d Metal: the profile") + (clipped ? " with a clipped value" : "") +
              " draws its quads black and red by the slope threshold (" + std::to_string(quads) + " of " +
              std::to_string(n - 1) + (wrong.empty() ? std::string() : ", wrong " + wrong) + "), the boundary green, " +
              std::to_string(green) + " px wide, and the frame " + (clipped ? "red" : "black") + ", " +
              std::to_string(framed) + " px wide");
  }
  mbvq_c_profile_show(instance, false);
  close_instance(instance);
}

/*--------------------------------------------------------------------*/
/* 7c on Metal: the 3D soundings at the levels of detail of the GPU-time
   fallback, in pixels */

/* metal_soundings()'s sparse set handed over again: 8 pings of 10 beams
   about 35 px apart seen from above, beam colours and flag reasons varied,
   flagged sounding 0 above sounding 11 (same place, shallower), sounding 79
   flagged alone */
bool lod_sparse_set() {
  make_sdg(8, 10, 0);
  const unsigned char flags[] = {MB_FLAG_FLAG | MB_FLAG_MANUAL, MB_FLAG_FLAG | MB_FLAG_FILTER,
                                 MB_FLAG_FLAG | MB_FLAG_SONAR, MB_FLAG_FLAG | MB_FLAG_FILTER2};
  for (size_t i = 0; i < sdg_storage.size(); i++) {
    sdg_storage[i].beamcolor = static_cast<int>(i % 8);
    if (i % 9 == 4)
      sdg_storage[i].beamflag = static_cast<char>(flags[(i / 9) % 4]);
  }
  sdg_storage[0].beamflag = static_cast<char>(MB_FLAG_FLAG | MB_FLAG_MANUAL);
  sdg_storage[0].x = sdg_storage[11].x;
  sdg_storage[0].y = sdg_storage[11].y;
  sdg_storage[0].z = sdg_storage[11].z + 10.0;
  sdg_storage[79].beamflag = static_cast<char>(MB_FLAG_FLAG | MB_FLAG_MANUAL);
  return open_sdg();
}

/* 7c-4: the 3D soundings at a forced level of detail (RenderOptions::
   soundings_lod: level k draws every 2^k-th sounding, by its index in the
   array) on two Metal renderers of their own, points and forced quads, both
   with soundings_chunk 27: the 80 soundings of the sparse set take three
   chunks of 27, 27 and 26 from soundings 0, 27 and 54, so that chunk 1
   begins off the stride at every level (its phase is 1, 1, 5 and 5
   soundings for k = 2, 4, 8 and 16). The chunk is read when the soundings
   are allocated and point_quads when the renderer is made, so
   metal_soundings()'s renderer could take neither; every call gives both,
   in case a renderer is made again. A level draws exactly the soundings
   whose index is a multiple of 2^k, each at its place (40, 20, 10 and 5 of
   the 80; 41, 21, 12 and 6 with every chunk drawn from its first
   sounding); the profile lines, the bounding box, the information
   highlight and the grab rectangle are drawn whole at every level (D11);
   the frame after a camera press's release draws every sounding again. */
void metal_lod() {
  const int W = 400, H = 300;
  const int n = 80;
  const int drawn_at[5] = {80, 40, 20, 10, 5};
  mbvq::OffscreenRenderer points(mbvq::OffscreenRenderer::Api::Metal);
  mbvq::OffscreenRenderer quads(mbvq::OffscreenRenderer::Api::Metal);
  mbvq::OffscreenRenderer *const renderers[2] = {&points, &quads};
  const char *const kinds[2] = {"points", "forced quads"};
  const auto lod_options = [](int r, int level) {
    mbvq::RenderOptions options;
    options.soundings_chunk = 27;
    options.point_quads = r == 1;
    options.soundings_lod = level;
    return options;
  };
  /* the places and colours of the soundings as the core gives them now */
  std::vector<float> xyz;
  std::vector<unsigned char> rgba;
  const auto read_soundings = [&]() {
    rhi::SoundingsFrame reader;
    return reader.positions(n, &xyz) && reader.colours(n, &rgba) && xyz.size() >= 3u * n && rgba.size() >= 4u * n;
  };
  int error = MB_ERROR_NO_ERROR;
  const bool opened = lod_sparse_set();
  sdg_camera(90.0f, 0.0f, 1.0137f, 0.00123f, -0.00431f);

  /* The pictures of levels 0 to 4, flagged soundings shown and not: the
     place of each sounding (that of 11 for sounding 0, drawn first and
     nearer) holds its colour where its index is a multiple of 2^k and it is
     drawn (alpha), else the background; the 3 x 3 px squares of those
     soundings are the picture's only coloured pixels (a sounding of the
     white beam colour shows as the background). */
  for (int r = 0; r < 2; r++) {
    mbvq::OffscreenRenderer &renderer = *renderers[r];
    bool counted = opened, placed = opened;
    std::string drawn, strides, where;
    int places = 0, squares = 0;
    for (int flagged = 1; flagged >= 0; flagged--) {
      sdg_view(MBS_VIEW_COLOR_FLAG, flagged != 0, MBS_VIEW_PROFILES_NONE, false);
      for (int level = 0; level <= 4; level++) {
        const size_t k = static_cast<size_t>(1) << level;
        const int lod_before = renderer.stats().lod_frames;
        QImage image;
        const bool ok = renderer.render_soundings(W, H, lod_options(r, level), &image) && read_soundings();
        const mbvq::RenderStats s = renderer.stats();
        counted = counted && ok && s.soundings == n && s.sounding_chunks == 3 && s.soundings_lod == level &&
                  s.soundings_drawn == drawn_at[level] && s.soundings_stride == 12 * static_cast<int>(k) &&
                  s.point_quads == (r == 1) &&
                  s.lod_frames - lod_before == (level > 0 ? renderer.last_frames() : 0);
        if (flagged == 1) {
          drawn += (level > 0 ? " " : "") + std::to_string(s.soundings_drawn);
          strides += (level > 0 ? " " : "") + std::to_string(s.soundings_stride);
        }
        placed = placed && ok;
        if (!ok)
          continue;
        int coloured = 0;
        for (size_t i = 1; i < static_cast<size_t>(n); i++) {
          const size_t at = i == 11 && rgba[3] != 0 ? 0 : i;
          const bool here = rgba[4 * at + 3] != 0 && at % k == 0;
          const uint32_t expected = here ? rgb_bytes(&rgba[4 * at]) : kWhite;
          const std::array<double, 2> w = sdg_window(xyz, i, W, H);
          const uint32_t got = at_window(image, w[0], w[1]);
          if (got != expected && where.size() < 120)
            where += "level " + std::to_string(level) + (flagged != 0 ? " " : " hidden ") + std::to_string(i) + ":" +
                     colour_name(got) + " ";
          placed = placed && got == expected;
          coloured += expected != kWhite ? 1 : 0;
          places++;
        }
        const int pixels = non_white(image);
        if (pixels != 9 * coloured && where.size() < 120)
          where += "level " + std::to_string(level) + (flagged != 0 ? ": " : " hidden: ") + std::to_string(pixels) +
                   " px for " + std::to_string(coloured) + " squares ";
        placed = placed && pixels == 9 * coloured;
        squares += coloured;
      }
    }
    check(counted && placed,
          std::string("7c Metal: forced levels 0 to 4 on 80 soundings in 3 chunks of 27, drawn as ") + kinds[r] +
              ", draw " + drawn + " soundings (41 21 12 6 at levels 1 to 4 if every chunk were drawn from its first "
              "sounding) at position strides of " + strides + " bytes, and each picture holds exactly the soundings "
              "whose index is a multiple of 1, 2, 4, 8 or 16, in their colours at their places, flagged soundings "
              "shown and hidden (" + std::to_string(places) + " places; " + std::to_string(squares) +
              " coloured squares of 9 px, no other coloured pixel)" +
              (where.empty() ? std::string() : "; differ: " + where));
  }

  /* The profile lines at levels 1 to 4 as metal_soundings() checks them at
     level 0, with its expectations: within a ping black and red where the
     core's pairs are, nothing between pings; at ratio 1 one pixel thick
     from the index lists, at ratio 2 two pixels by the instanced quads;
     x-major at azimuth 0, y-major at 90; counted across the middle of each
     segment, about 17 px from the soundings at its ends. */
  for (int r = 0; r < 2; r++) {
    mbvq::OffscreenRenderer &renderer = *renderers[r];
    int good = 0, bad = 0, frames_right = 0;
    std::string wrong;
    for (int level = 1; level <= 4; level++)
      for (int view = 0; view < 4; view++) {
        const int ratio = 1 + view % 2;
        const float azimuth = view < 2 ? 0.0f : 90.0f;
        sdg_camera(90.0f, azimuth, 1.0137f, 0.00123f, -0.00431f);
        sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, false);
        mbvq::RenderOptions options = lod_options(r, level);
        options.pixel_ratio = ratio;
        QImage image;
        const bool drawn = renderer.render_soundings(W * ratio, H * ratio, options, &image);
        const mbvq::RenderStats s = renderer.stats();
        std::vector<uint32_t> black, red;
        rhi::SoundingsFrame reader;
        const bool read = drawn && reader.positions(n, &xyz) && reader.pairs(n, &black, &red);
        const bool right = read && s.soundings_lod == level && s.soundings_drawn == drawn_at[level] &&
                           s.profile_wide == (ratio == 2) &&
                           (ratio == 1 ? s.profile_indices > 0 && s.profile_instances == 0
                                       : s.profile_instances > 0 && s.profile_indices == 0);
        frames_right += right ? 1 : 0;
        if (!right && wrong.size() < 100)
          wrong += "level " + std::to_string(level) + " ratio " + std::to_string(ratio) + " azimuth " +
                   std::to_string(static_cast<int>(azimuth)) + ": statistics ";
        if (!read)
          continue;
        std::vector<int> kind(static_cast<size_t>(n), 0);
        for (size_t p = 0; p + 1 < black.size(); p += 2)
          kind[black[p]] = 1;
        for (size_t p = 0; p + 1 < red.size(); p += 2)
          kind[red[p]] = 2;
        for (size_t i = 1; i + 1 < static_cast<size_t>(n); i++) {
          /* the middle of the segment from i to i + 1 */
          const std::array<double, 2> a = sdg_window(xyz, i, W * ratio, H * ratio);
          const std::array<double, 2> b = sdg_window(xyz, i + 1, W * ratio, H * ratio);
          const double mx = 0.5 * (a[0] + b[0]), my = 0.5 * (a[1] + b[1]);
          const uint32_t expected = kind[i] == 1 ? 0x000000u : kind[i] == 2 ? kRed : kWhite;
          if (kind[i] == 0) {
            good += at_window(image, mx, my) == kWhite;
            bad += at_window(image, mx, my) != kWhite;
            continue;
          }
          /* the line's pixels across it in the middle: as many as its width */
          const bool xmajor = std::fabs(b[0] - a[0]) >= std::fabs(b[1] - a[1]);
          int across = 0;
          for (int d = -4 * ratio; d <= 4 * ratio; d++)
            across += (xmajor ? at_window(image, mx, my + d) : at_window(image, mx + d, my)) == expected;
          if (across == ratio && xmajor == (azimuth == 0.0f))
            good++;
          else {
            bad++;
            if (wrong.size() < 100)
              wrong += "level " + std::to_string(level) + " ratio " + std::to_string(ratio) + " " + std::to_string(i) +
                       ":" + std::to_string(across) + " ";
          }
        }
      }
    check(frames_right == 16 && bad == 0 && good > 0,
          std::string("7c Metal: at levels 1 to 4 (") + kinds[r] + ", chunks of 27) the profile lines are drawn as "
              "at level 0: at ratios 1 and 2 and azimuths 0 and 90 thin from the index lists or 2 px wide by the "
              "instanced quads, black and red where the core's pairs are and nothing between pings (" +
              std::to_string(good) + " places right in " + std::to_string(frames_right) + " of 16 frames" +
              (wrong.empty() ? std::string() : "; wrong: " + wrong) + ")");
  }

  /* The bounding box and the information highlight on sounding 79, flagged
     and not shown, so never drawn and the highlight shown at every level
     (seen from above, at the box's corner): at levels 1 to 4 the picture
     equals level 0's outside the squares of the soundings shown at level 0
     (5 x 5 px around each place, the 3 x 3 px square and a pixel), the only
     pixels where fewer soundings may differ. The highlight is cyan in the
     flag colour mode. */
  sdg_camera(90.0f, 0.0f, 1.0137f, 0.00123f, -0.00431f);
  for (int r = 0; r < 2; r++) {
    mbvq::OffscreenRenderer &renderer = *renderers[r];
    const uint32_t kCyan = 0xffff00;
    sdg_view(MBS_VIEW_COLOR_FLAG, false, MBS_VIEW_PROFILES_NONE, true);
    mb3dsoundings.edit_mode = MBS_EDIT_INFO;
    mb3dsoundings.last_sounding_defined = true;
    mb3dsoundings.last_sounding_edited = 79;
    QImage full;
    bool drawn = renderer.render_soundings(W, H, lod_options(r, 0), &full) && read_soundings();
    /* the squares of the soundings shown at level 0, by image row and column */
    std::vector<char> masked(static_cast<size_t>(W) * H, 0);
    int shown = 0;
    for (size_t i = 0; drawn && i < static_cast<size_t>(n); i++) {
      if (rgba[4 * i + 3] == 0)
        continue;
      shown++;
      const std::array<double, 2> w = sdg_window(xyz, i, W, H);
      const int cx = static_cast<int>(std::floor(w[0])), cy = static_cast<int>(std::floor(w[1]));
      for (int y = cy - 2; y <= cy + 2; y++)
        for (int x = cx - 2; x <= cx + 2; x++)
          if (x >= 0 && x < W && y >= 0 && y < H)
            masked[static_cast<size_t>(H - 1 - y) * W + x] = 1;
    }
    /* the highlight's pixels around the place of 79, and the pixels of the
       box and the highlight outside the squares at level 0 */
    const std::array<double, 2> w79 = drawn ? sdg_window(xyz, 79, W, H) : std::array<double, 2>{-10.0, -10.0};
    const auto highlight = [&](const QImage &image) {
      int cyan = 0;
      for (int dy = -3; dy <= 3; dy++)
        for (int dx = -3; dx <= 3; dx++)
          cyan += at_window(image, w79[0] + dx, w79[1] + dy) == kCyan;
      return cyan;
    };
    const int cyan_full = drawn ? highlight(full) : 0;
    int decorations = 0;
    for (int row = 0; drawn && row < H; row++)
      for (int x = 0; x < W; x++)
        decorations += masked[static_cast<size_t>(row) * W + x] == 0 && pixel_rgb(full, x, row) != kWhite;
    int differ = 0, cyan_least = cyan_full;
    bool levels = true;
    for (int level = 1; drawn && level <= 4; level++) {
      QImage image;
      drawn = renderer.render_soundings(W, H, lod_options(r, level), &image);
      levels = levels && renderer.stats().soundings_lod == level && renderer.stats().soundings_drawn == drawn_at[level];
      for (int row = 0; drawn && row < H; row++)
        for (int x = 0; x < W; x++)
          differ +=
              masked[static_cast<size_t>(row) * W + x] == 0 && pixel_rgb(image, x, row) != pixel_rgb(full, x, row);
      cyan_least = drawn ? std::min(cyan_least, highlight(image)) : 0;
    }
    mb3dsoundings.edit_mode = MBS_EDIT_TOGGLE;
    mb3dsoundings.last_sounding_defined = false;
    check(drawn && levels && differ == 0 && cyan_least > 0 && decorations > cyan_full,
          std::string("7c Metal: with flagged soundings hidden, the bounding box and the information highlight on "
                      "flagged sounding 79 are drawn at levels 1 to 4 (") +
              kinds[r] + ") as at level 0: " + std::to_string(differ) + " px differ outside the squares of the " +
              std::to_string(shown) + " soundings shown at level 0 (" + std::to_string(decorations) +
              " px of the box and the highlight there; at least " + std::to_string(cyan_least) +
              " cyan px of the highlight at every level)");
  }

  /* The release frame (plan 6.8: it equals a still frame, 0 px), all
     profiles and the box shown: a still picture A; three frames of a camera
     press (RenderOptions::drag) with slow fake times (total 16 ms, wait 12
     ms: the fallback marks level 0 after two), the third at level 1 and
     unlike A; the frame after the release (drag off, the fake times kept)
     at level 0 with every sounding, 0 px from A; then a forced level 4 and
     the fallback's level again (-1): 0 px from A. Before, the still frame's
     wait is measured: Metal waits for the GPU in endOffscreenFrame(), so
     the fallback can act there without a fake (gpu_rule), and each frame
     measured counts. */
  sdg_camera(90.0f, 0.0f, 1.0137f, 0.00123f, -0.00431f);
  sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, true);
  bool measured = true;
  std::string waits;
  for (int r = 0; r < 2; r++) {
    mbvq::OffscreenRenderer &renderer = *renderers[r];
    const mbvq::RenderOptions plain = lod_options(r, -1);
    const int wait_frames = renderer.stats().wait_frames;
    QImage still;
    bool drawn = renderer.render_soundings(W, H, plain, &still);
    const mbvq::RenderStats a = renderer.stats();
    measured = measured && drawn && a.gpu_rule && a.wait_ms > 0.0 && a.frame_ms >= a.wait_ms &&
               a.wait_frames - wait_frames == renderer.last_frames() && renderer.last_frames() > 0;
    waits += std::string(r > 0 ? "; " : "") + kinds[r] + ": gpu_rule " + (a.gpu_rule ? "true" : "false") + ", " +
             text("%.3f", a.wait_ms) + " of " + text("%.3f", a.frame_ms) + " ms, wait_frames +" +
             std::to_string(a.wait_frames - wait_frames) + " for " + std::to_string(renderer.last_frames()) +
             (renderer.last_frames() == 1 ? " frame" : " frames");

    mbvq::RenderOptions press = plain;
    press.drag = true;
    press.gpu_total_ms = 16.0;
    press.gpu_wait_ms = 12.0;
    QImage pressed;
    std::string levels;
    for (int f = 0; f < 3; f++) {
      drawn = renderer.render_soundings(W, H, press, &pressed, 1) && drawn;
      levels += (f > 0 ? " " : "") + std::to_string(renderer.stats().soundings_lod);
    }
    const mbvq::RenderStats third = renderer.stats();
    mbvq::RenderOptions release = press;
    release.drag = false;
    QImage released;
    drawn = renderer.render_soundings(W, H, release, &released, 1) && drawn;
    const mbvq::RenderStats after = renderer.stats();
    mbvq::RenderOptions four = plain;
    four.soundings_lod = 4;
    QImage forced, again;
    drawn = renderer.render_soundings(W, H, four, &forced, 1) && drawn;
    const int forced_level = renderer.stats().soundings_lod;
    drawn = renderer.render_soundings(W, H, plain, &again, 1) && drawn;
    const mbvq::RenderStats back = renderer.stats();
    const int unlike = differing(pressed, still), after_release = differing(released, still);
    const int unlike_forced = differing(forced, still), after_forced = differing(again, still);
    check(drawn && levels == "0 0 1" && third.soundings_drawn == 40 && unlike > 0 && after.soundings_lod == 0 &&
              after.soundings_drawn == n && after_release == 0 && forced_level == 4 && unlike_forced > 0 &&
              back.soundings_lod == 0 && back.soundings_drawn == n && after_forced == 0,
          std::string("7c Metal: the release frame (") + kinds[r] + ", all profiles and the box): three frames of "
              "a camera press with slow fake times (total 16 ms, wait 12 ms) at levels " + levels + ", the third "
              "with " + std::to_string(third.soundings_drawn) + " soundings and " + std::to_string(unlike) +
              " px unlike the still picture; the frame after the release at level " +
              std::to_string(after.soundings_lod) + " with " + std::to_string(after.soundings_drawn) + " of 80, " +
              std::to_string(after_release) + " px from the still picture; forced level 4 (" +
              std::to_string(unlike_forced) + " px unlike), then the fallback's level again: " +
              std::to_string(after_forced) + " px from it");
  }
  check(measured, "7c Metal: a soundings frame without fake times is measured, since Metal waits for the GPU in "
                  "endOffscreenFrame(): the fallback can act (gpu_rule), the wait lies above 0 and within the frame's "
                  "total, and wait_frames rises by the frames drawn (" + waits + ")");

  /* the grab rectangle, drawn with the projection alone, at levels 1 to 4
     as metal_soundings() counts it at level 0: at its window corners under
     a rotated camera, nothing inside */
  {
    sdg_camera(35.0f, 125.0f, 1.3f, 0.05f, -0.05f);
    /* no sounding yellow */
    for (struct mb3dsoundings_sounding_struct &sounding : sdg_storage)
      sounding.beamcolor = MBV_COLOR_BLACK;
    sdg_view(MBS_VIEW_COLOR_FLAG, false, MBS_VIEW_PROFILES_NONE, false);
    mbvq_c_soundings_resize(W, H);
    mb3dsoundings.edit_mode = MBS_EDIT_GRAB;
    mb3dsoundings.button1down = true;
    mb3dsoundings.grab_start_defined = true;
    mb3dsoundings.grab_end_defined = true;
    mb3dsoundings.grab_start_x = 101;
    mb3dsoundings.grab_start_y = 77;
    mb3dsoundings.grab_end_x = 287;
    mb3dsoundings.grab_end_y = 203;
    const uint32_t kYellow = 0x00ffff;
    int frames_right = 0;
    int xmin = W, xmax = -1, ymin = H, ymax = -1;
    std::string wrong;
    for (int r = 0; r < 2; r++)
      for (int level = 1; level <= 4; level++) {
        QImage image;
        const bool drawn = renderers[r]->render_soundings(W, H, lod_options(r, level), &image);
        int left = 0, right = 0, bottom = 0, top = 0, inner = 0;
        int x0 = W, x1 = -1, y0 = H, y1 = -1;
        for (int row = 0; drawn && row < H; row++)
          for (int x = 0; x < W; x++) {
            if (pixel_rgb(image, x, row) != kYellow)
              continue;
            const int y = H - 1 - row;
            x0 = std::min(x0, x);
            x1 = std::max(x1, x);
            y0 = std::min(y0, y);
            y1 = std::max(y1, y);
            left += std::abs(x - 101) <= 2;
            right += std::abs(x - 287) <= 2;
            bottom += std::abs(y - 77) <= 2;
            top += std::abs(y - 203) <= 2;
            inner += x > 104 && x < 284 && y > 80 && y < 200;
          }
        const bool ok = drawn && renderers[r]->stats().soundings_lod == level && left > 3 * 100 && right > 3 * 100 &&
                        bottom > 3 * 150 && top > 3 * 150 && inner == 0 && x0 >= 99 && x1 <= 289 && y0 >= 75 &&
                        y1 <= 205;
        frames_right += ok ? 1 : 0;
        if (!ok && wrong.size() < 100)
          wrong += std::string(kinds[r]) + " level " + std::to_string(level) + ": " +
                   (x1 < 0 ? std::string("no yellow")
                           : std::to_string(x0) + " " + std::to_string(y0) + " to " + std::to_string(x1) + " " +
                                 std::to_string(y1)) +
                   " ";
        xmin = std::min(xmin, x0);
        xmax = std::max(xmax, x1);
        ymin = std::min(ymin, y0);
        ymax = std::max(ymax, y1);
      }
    mb3dsoundings.button1down = false;
    mb3dsoundings.grab_start_defined = false;
    mb3dsoundings.grab_end_defined = false;
    mb3dsoundings.edit_mode = MBS_EDIT_TOGGLE;
    check(frames_right == 8,
          "7c Metal: at levels 1 to 4 (points and forced quads) the grab rectangle (3 px, the projection alone) "
          "lies at its window corners 101 77 287 203 under a rotated camera, as at level 0 (" +
              std::to_string(frames_right) + " of 8 frames; " +
              (xmax < 0 ? std::string("no yellow")
                        : "yellow from " + std::to_string(xmin) + " " + std::to_string(ymin) + " to " +
                              std::to_string(xmax) + " " + std::to_string(ymax)) +
              (wrong.empty() ? std::string() : "; wrong: " + wrong) + ")");
  }
  mb3dsoundings_end(0, &error);
}

int test_render_metal(const char *save) {
  mbvq::OffscreenRenderer renderer(mbvq::OffscreenRenderer::Api::Metal);
  if (!renderer.valid()) {
    std::printf("skipped: no Metal QRhi can be made here\n");
    return 77;
  }
  std::printf("      a Metal QRhi (%s)\n", renderer.backend_name());
  Grid grid;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &instance), "the synthetic grid opens under QRhi");
  struct mbview_struct *data = data_of(instance);

  /* the data area within 2 levels of the node colours, the holes white */
  mbvq::RenderOptions options;
  QImage image;
  check(renderer.render_map(instance, kWidth, kHeight, options, &image), "a frame drawn and read back");
  if (save != nullptr)
    check(image.save(QString::fromLocal8Bit(save)), std::string("picture saved as ") + save);
  int inside = 0, off = 0, white = 0, holes = 0;
  for (int row = 0; row < kHeight; row += 2) {
    for (int column = 1; column < kWidth; column += 2) {
      const Triangle t = pick(instance, column, row);
      const uint32_t pixel = rgb(image, column, row);
      if (!t.found) {
        /* outside the data: white, unless the pixel is on a triangle's edge */
        white += pixel == kWhite;
        holes += pixel != kWhite;
        continue;
      }
      if (edge_distance(instance, t, column, row) < kTie)
        continue;
      bool within = true;
      for (int channel = 0; channel < 3; channel++) {
        int lowest = 255, highest = 0;
        for (int k : nodes(instance, t)) {
          const float *colour = channel == 0 ? data->primary_r : channel == 1 ? data->primary_g : data->primary_b;
          const int byte = colour[k] <= 0.0f ? 0 : colour[k] >= 1.0f ? 255 : static_cast<int>(std::lround(colour[k] * 255.0));
          lowest = std::min(lowest, byte);
          highest = std::max(highest, byte);
        }
        const int value = static_cast<int>((pixel >> (8 * channel)) & 255);
        within = within && value >= lowest - 2 && value <= highest + 2;
      }
      inside += within;
      off += !within;
    }
  }
  check(inside > 10000 && off == 0, "the terrain pixels lie within 2 levels of their triangle's node colours (" +
                                        std::to_string(inside) + " pixels, " + std::to_string(off) + " outside)");
  check(white > 100 && holes <= white / 200,
        "where the CPU pick finds no terrain the picture is white (" + std::to_string(white) + " pixels, " +
            std::to_string(holes) + " not)");
  const QImage before = image;

  /* an edited node without data: white after the edit */
  int error = MB_ERROR_NO_ERROR;
  const int ei = 150, ej = 60;
  const int k = ei * grid.rows + ej;
  double projection[16], modelview[16], camera[16];
  mbvq_c_camera(instance, projection, modelview);
  rhi::multiply(projection, modelview, camera);
  const double p[4] = {data->primary_x[k], data->primary_y[k], data->primary_z[k], 1.0};
  double c[4];
  for (int r = 0; r < 4; r++)
    c[r] = camera[r] * p[0] + camera[4 + r] * p[1] + camera[8 + r] * p[2] + camera[12 + r] * p[3];
  const int ecolumn = static_cast<int>(std::floor((c[0] / c[3] + 1.0) * 0.5 * kWidth));
  const int erow = kHeight - 1 - static_cast<int>(std::floor((c[1] / c[3] + 1.0) * 0.5 * kHeight));
  const bool coloured = rgb(before, ecolumn, erow) != kWhite;
  mbview_updateprimarygridcell(0, instance, ei, ej, data->primary_nodatavalue, &error);
  check(renderer.render_map(instance, kWidth, kHeight, options, &image) && coloured &&
            rgb(image, ecolumn, erow) == kWhite,
        "a node edited to no data is white after the edit, drawn from the incremental upload");

  /* pick against picture, in the primary grid mode: 2D, then after changes,
     each also against the picks of a freshly opened instance */
  std::vector<std::function<void(size_t)>> changes;
  auto agree_fresh = [&]() {
    size_t fresh = MBV_NO_WINDOW;
    if (!open_grid(grid, nullptr, &fresh))
      return false;
    for (const auto &change : changes)
      change(fresh);
    mbvq::OffscreenRenderer other(mbvq::OffscreenRenderer::Api::Metal);
    other.render_map(fresh, kWidth, kHeight, mbvq::RenderOptions(), nullptr);
    /* the core's size is the last a frame gave it: both are drawn at the same size */
    renderer.render_map(instance, kWidth, kHeight, mbvq::RenderOptions(), nullptr);
    const bool agree = picks_agree(instance, fresh);
    close_instance(fresh);
    return agree;
  };
  changes.push_back([&](size_t view) {
    int e = MB_ERROR_NO_ERROR;
    mbview_updateprimarygridcell(0, view, ei, ej, data_of(view)->primary_nodatavalue, &e);
  });
  /* pick against picture in the current state */
  auto pick_check = [&](int least, const std::string &what) {
    int mismatches = 0, ties = 0;
    const int compared = pick_against_picture(renderer, instance, &mismatches, &ties);
    check(compared > least && mismatches == 0, "pick against picture, " + what + ": " + std::to_string(compared) +
                                                   " pixels, " + std::to_string(mismatches) + " differ, " +
                                                   std::to_string(ties) + " ties");
  };
  pick_check(5000, "2D after a cell edit");
  check(agree_fresh(), "the picks equal those of a freshly opened instance with the same edit");

  changes.push_back([](size_t view) { mbvq_c_view(view, MBVQ_VIEW_DISPLAY_3D, true); });
  changes.back()(instance);
  pick_check(3000, "3D");

  changes.push_back([](size_t view) { set_exaggeration(view, 3.0); });
  changes.back()(instance);
  pick_check(3000, "3D after an exaggeration change");
  check(agree_fresh(), "the picks equal those of a freshly opened instance in the same state");

  changes.push_back([&](size_t view) {
    add_secondary(view, grid, grid.columns, grid.rows);
    mbvq_c_view(view, MBVQ_VIEW_GRID_SECONDARY, true);
  });
  changes.back()(instance);
  renderer.render_map(instance, kWidth, kHeight, options, nullptr);
  changes.push_back([](size_t view) { mbvq_c_view(view, MBVQ_VIEW_GRID_PRIMARY, true); });
  changes.back()(instance);
  pick_check(3000, "after the grid modes primary, secondary, primary");
  check(agree_fresh(), "the picks equal those of a freshly opened instance after the grid modes");

  changes.push_back([](size_t view) { mbvq_c_setprojection(view, 0); });
  changes.back()(instance);
  pick_check(3000, "after a change to the geographic display");
  check(agree_fresh(), "the picks equal those of a freshly opened instance after the projection change");

  /* the cached picture against one made from nothing, after edits, a colour
     table change and an exaggeration change */
  mbview_updateprimarygridcell(0, instance, 60, 120, data->primary_data[60 * grid.rows + 120] + 40.0f, &error);
  mbview_updateprimarygridcell(0, instance, 61, 120, data->primary_nodatavalue, &error);
  renderer.render_map(instance, kWidth, kHeight, options, nullptr);
  mbvq_c_view(instance, MBVQ_VIEW_COLORTABLE_BRIGHT, true);
  renderer.render_map(instance, kWidth, kHeight, options, nullptr);
  set_exaggeration(instance, 1.5);
  QImage cached, scratch, again;
  renderer.render_map(instance, kWidth, kHeight, options, &cached);
  {
    mbvq::OffscreenRenderer fresh(mbvq::OffscreenRenderer::Api::Metal);
    fresh.render_map(instance, kWidth, kHeight, options, &scratch);
  }
  check(!cached.isNull() && cached == scratch, "the cached picture equals one made from nothing");
  renderer.render_map(instance, kWidth, kHeight, options, &again);
  check(again == cached, "the same frame twice gives the same bytes");

  /* Nodes the preparation leaves as they were in the secondary grid mode (the
     overlay grid's hole) come into use with the mask: the cached picture
     still equals one made from nothing. */
  const auto against_fresh = [&](const std::string &what) {
    QImage now, fresh_image;
    renderer.render_map(instance, kWidth, kHeight, options, &now);
    {
      mbvq::OffscreenRenderer fresh(mbvq::OffscreenRenderer::Api::Metal);
      fresh.render_map(instance, kWidth, kHeight, options, &fresh_image);
    }
    const int differ = differing(now, fresh_image);
    check(!now.isNull() && differ == 0,
          "the cached picture equals one made from nothing after " + what + " (" + std::to_string(differ) +
              " pixels differ)");
  };
  add_secondary(instance, grid, grid.columns, grid.rows);
  mbvq_c_view(instance, MBVQ_VIEW_GRID_SECONDARY, true);
  renderer.render_map(instance, kWidth, kHeight, options, nullptr);
  set_exaggeration(instance, 2.5);
  renderer.render_map(instance, kWidth, kHeight, options, nullptr);
  mbvq_c_view(instance, MBVQ_VIEW_GRID_PRIMARY, true);
  against_fresh("an exaggeration change in the secondary grid mode, then the primary grid mode (3D)");
  mbvq_c_view(instance, MBVQ_VIEW_GRID_SECONDARY, true);
  renderer.render_map(instance, kWidth, kHeight, options, nullptr);
  set_exaggeration(instance, 1.2);
  renderer.render_map(instance, kWidth, kHeight, options, nullptr);
  fill_secondary(instance, grid.columns, grid.rows);
  against_fresh("an exaggeration change in the secondary grid mode, then the overlay grid's hole filled");
  add_secondary(instance, grid, grid.columns, grid.rows);
  mbvq_c_view(instance, MBVQ_VIEW_DISPLAY_2D, true);
  renderer.render_map(instance, kWidth, kHeight, options, nullptr);
  set_exaggeration(instance, 2.0);
  renderer.render_map(instance, kWidth, kHeight, options, nullptr);
  mbvq_c_view(instance, MBVQ_VIEW_GRID_PRIMARY, true);
  renderer.render_map(instance, kWidth, kHeight, options, nullptr);
  mbvq_c_view(instance, MBVQ_VIEW_DISPLAY_3D, true);
  against_fresh("the same in 2D, then 3D");

  /* the colours as floats: the same coverage, within 2 levels of the colours
     from bytes (the rounding of the node colours), and not the same */
  {
    QImage bytes, floats;
    mbvq::RenderOptions with_floats;
    with_floats.float_terrain_colours = true;
    renderer.render_map(instance, kWidth, kHeight, options, &bytes);
    renderer.render_map(instance, kWidth, kHeight, with_floats, &floats);
    int coverage = 0, beyond = 0, changed = 0;
    for (int row = 0; !bytes.isNull() && !floats.isNull() && row < kHeight; row++)
      for (int column = 0; column < kWidth; column++) {
        const uint32_t a = rgb(bytes, column, row), b = rgb(floats, column, row);
        coverage += (a == kWhite) != (b == kWhite);
        int largest = 0;
        for (int channel = 0; channel < 3; channel++)
          largest = std::max(largest, std::abs(static_cast<int>((a >> (8 * channel)) & 255) -
                                               static_cast<int>((b >> (8 * channel)) & 255)));
        beyond += largest > 2;
        changed += largest > 0;
      }
    check(!bytes.isNull() && !floats.isNull() && coverage == 0 && beyond == 0 && changed > 0,
          "float_terrain_colours: the same coverage, " + std::to_string(changed) +
              " pixels within 2 levels of the colours from bytes, none further (" + std::to_string(beyond) + ")");
  }
  /* reported, not checked: whether Metal gives GPU times depends on the device */
  std::printf("      the last frame: %.3f ms on the CPU; the GPU time of the frame before %.3f ms (-1: none)\n",
              renderer.stats().render_ms, renderer.stats().gpu_ms);

  close_instance(instance);
  metal_levels();
  metal_batch();
  metal_buffers();
  metal_points();
  metal_point_quads();
  metal_wide_layouts();
  metal_marker_runs();
  metal_contours();
  metal_soundings();
  /* 7c: the 3D soundings at the levels of detail of the GPU-time fallback:
     every 2^k-th sounding at its place, the decorations whole, the release
     frame equal to a still frame */
  metal_lod();
  metal_profile();
  return result();
}

/*--------------------------------------------------------------------*/
/* the window of a view is exposed (a first frame that does not come is then
   not the window system's doing) */
bool exposed(QWidget *view) {
  const QWindow *window = view->window()->windowHandle();
  return window != nullptr && window->isExposed();
}

/* wait until a QRhi map view drew at full resolution */
bool wait_full(size_t instance, int ms) {
  QElapsedTimer timer;
  timer.start();
  while ((!mbvq::view_state(mbvq::map_view(instance)).drawn || mbvq_c_lastdrawrez(instance) != MBVQ_REZ_FULL) &&
         timer.elapsed() < ms)
    spin(20);
  return mbvq::view_state(mbvq::map_view(instance)).drawn && mbvq_c_lastdrawrez(instance) == MBVQ_REZ_FULL;
}

int npoints_of(const struct mbview_struct *data) { return data->profile.npoints; }

/* A QRhi view's picture (W, grabFramebuffer) against the offscreen
   renderer's picture of the same drawing (C) at the view's size and pixel
   ratio; both Metal, so the same pixels (design 16). Elsewhere skipped. */
void same_as_offscreen(QRhiWidget *view, const QImage &picture, double ratio, const std::string &what,
                       const std::function<bool(mbvq::OffscreenRenderer &, const mbvq::RenderOptions &, QImage *)> &render) {
  if (view == nullptr || view->api() != QRhiWidget::Api::Metal) {
    std::printf("skipped: %s view's picture against the offscreen renderer's (Metal only)\n", what.c_str());
    return;
  }
  mbvq::OffscreenRenderer offscreen(mbvq::OffscreenRenderer::Api::Metal);
  mbvq::RenderOptions options;
  options.pixel_ratio = ratio;
  options.soundings_point_size = mbvq::soundings_point_size();
  QImage c;
  const bool drawn = offscreen.valid() && render(offscreen, options, &c);
  const int differ = drawn ? differing(picture, c) : -1;
  check(drawn && differ == 0, "6d: " + what + " view's picture (grabFramebuffer, " + std::to_string(picture.width()) +
                                  " x " + std::to_string(picture.height()) + ") equals the offscreen renderer's at "
                                  "its size and ratio " + text("%.2f", ratio) + " (" + std::to_string(differ) +
                                  " px differ)");
}

/* The MB that the Metal device holds (MTLDevice.currentAllocatedSize; the
   views' QRhis on Metal use the system's default device), -1 where there is
   none. Metal and libobjc are looked up at run time, as for FramePool, so
   that the test links nothing new. */
double metal_allocated_mb() {
#if defined(__APPLE__)
  static void *device = [] {
    void *create = dlsym(RTLD_DEFAULT, "MTLCreateSystemDefaultDevice");
    return create != nullptr ? reinterpret_cast<void *(*)()>(create)() : nullptr;
  }();
  void *send = dlsym(RTLD_DEFAULT, "objc_msgSend");
  void *selector = dlsym(RTLD_DEFAULT, "sel_registerName");
  if (device == nullptr || send == nullptr || selector == nullptr)
    return -1.0;
  void *size = reinterpret_cast<void *(*)(const char *)>(selector)("currentAllocatedSize");
  return static_cast<double>(reinterpret_cast<unsigned long (*)(void *, void *)>(send)(device, size)) / 1048576.0;
#else
  return -1.0;
#endif
}

/* 7a: the time of a view's last create() (RenderStats::create_ms), printed
   by --display and --quit at the moments the header comment names (a
   view's first frame, its dock floated or docked again; a measurement, not
   a check): -1 while the view has no renderer, also after a create() that
   failed, whose renderer the view drops */
void print_create_time(QWidget *view, const char *what) {
  if (view != nullptr)
    std::printf("info\tcreate()\t%s: %.2f ms\n", what, mbvq::view_stats(view).create_ms);
}

/* 7c: the hook of this test's application (TestApplication, defined with
   the bench below), called after each event it delivered, so that a test
   sees the end of a paint (an event filter sees only its beginning); an
   empty one: none */
void set_delivered(std::function<void(const QObject *, QEvent::Type)> hook);

int test_display(bool mode3d, const char *grdfile, const char *save) {
  Grid grid;
  grid.mode3d = mode3d;
  size_t instance = MBV_NO_WINDOW;
  QElapsedTimer opened;
  opened.start();
  check(open_grid(grid, grdfile, &instance), "mbview_open under QRhi");
  QWidget *view = mbvq::map_view(instance);
  auto *rhi_widget = qobject_cast<QRhiWidget *>(view);
  check(rhi_widget != nullptr, "the map view is a QRhiWidget");
  if (rhi_widget == nullptr)
    return result();

  /* the first frames, until the full-resolution fill */
  QElapsedTimer timer;
  timer.start();
  while ((!mbvq::view_state(view).drawn || mbvq_c_lastdrawrez(instance) != MBVQ_REZ_FULL) && timer.elapsed() < 20000)
    spin(20);
  const mbvq::ViewState drawn = mbvq::view_state(view);
  check(drawn.drawn && !drawn.failure_shown && drawn.renderer && mbvq_c_lastdrawrez(instance) == MBVQ_REZ_FULL,
        "drawn at full resolution " + std::to_string(opened.elapsed()) + " ms after opening (window exposed: " +
            std::to_string(exposed(view)) + ")");
  print_create_time(view, "the map view, first frame");
  spin(300);

  /* the picture: grabFramebuffer() renders a new frame, a fill with its side effects */
  const QImage image = rhi_widget->grabFramebuffer();
  int coloured = 0;
  std::vector<QRgb> seen;
  for (int y = 0; y < image.height(); y++)
    for (int x = 0; x < image.width(); x++) {
      const QRgb pixel = image.pixel(x, y) | 0xff000000u;
      if (pixel != 0xffffffffu)
        coloured++;
      if ((x % 5) == 0 && (y % 5) == 0 && seen.size() < 2000 && std::find(seen.begin(), seen.end(), pixel) == seen.end())
        seen.push_back(pixel);
    }
  check(image.width() > 0 && coloured > image.width() * image.height() / 100 && seen.size() > 50,
        "the grabbed picture (" + std::to_string(image.width()) + " x " + std::to_string(image.height()) + " px, ratio " +
            text("%.2f", view->devicePixelRatioF()) + ") shows the terrain: " + std::to_string(coloured) +
            " coloured pixels, " + std::to_string(seen.size()) + " colours in a sample");
  if (save != nullptr)
    check(image.save(QString::fromLocal8Bit(save)), std::string("picture saved as ") + save);

  /* a click picks: input reaches the core after the first frame */
  struct mbview_struct *data = data_of(instance);
  data->pick_type = MBV_PICK_NONE;
  const QPointF centre(view->width() / 2.0, view->height() / 2.0);
  QMouseEvent press(QEvent::MouseButtonPress, centre, view->mapToGlobal(centre), Qt::LeftButton, Qt::LeftButton,
                    Qt::NoModifier);
  QApplication::sendEvent(view, &press);
  QMouseEvent release(QEvent::MouseButtonRelease, centre, view->mapToGlobal(centre), Qt::LeftButton, Qt::NoButton,
                      Qt::NoModifier);
  QApplication::sendEvent(view, &release);
  spin(200);
  check(data->pick_type == MBV_PICK_ONEPOINT, "a click at the centre picks a point");

  /* A shading drag with the right button, then the release: while the button
     is down the frames are planned within the drag budget; the core asks for
     no frame after this release (its last drawing was at full resolution),
     so the view asks for one, drawn at the stride of the zoom without it. */
  {
    mbvq_c_set_mouse_mode(instance, MBV_MOUSE_SHADE);
    QMouseEvent down(QEvent::MouseButtonPress, centre, view->mapToGlobal(centre), Qt::RightButton, Qt::RightButton,
                     Qt::NoModifier);
    QApplication::sendEvent(view, &down);
    for (int m = 1; m <= 5; m++) {
      const QPointF at = centre + QPointF(0.0, 4.0 * m);
      QMouseEvent move(QEvent::MouseMove, at, view->mapToGlobal(at), Qt::NoButton, Qt::RightButton, Qt::NoModifier);
      QApplication::sendEvent(view, &move);
      spin(60);
    }
    const mbvq::RenderStats during = mbvq::view_stats(view);
    const QPointF end = centre + QPointF(0.0, 20.0);
    QMouseEvent up(QEvent::MouseButtonRelease, end, view->mapToGlobal(end), Qt::RightButton, Qt::NoButton,
                   Qt::NoModifier);
    QApplication::sendEvent(view, &up);
    spin(400);
    const mbvq::RenderStats after = mbvq::view_stats(view);
    mbvq_c_set_mouse_mode(instance, MBV_MOUSE_MOVE);
    check(during.budget_ms >= 0.0 && after.frames > during.frames && after.budget_ms < 0.0 &&
              after.stride == after.stride_zoom && after.tiles_waiting == 0,
          "a shading drag: frames within the drag budget (" + text("%.1f", during.budget_ms) +
              " ms, stride " + std::to_string(during.stride) + "); after the release a frame at the stride of the "
              "zoom (" + std::to_string(after.stride) + ") without it");
  }

  /* 7c: the map's GPU-time fallback in a window, under the hook's fake wait
     of 15 ms (T or more, so that every frame's total exceeds T): a pan with
     the middle button whose frames are GPU-slow where their measured rest
     (frame - wait) stays under T; two such frames at a stride mark it, and
     the next frames pass over it (gpu_coarser). Then a release and a new
     press sent back to back, with no frame between: the view ends the drag
     in its renderer at the release, so the new press starts afresh, and
     its first frame passes over no level. Where the rests stay at T or more
     (a slow composition) the rule stays inert: an info line. */
  {
    std::vector<mbvq::RenderStats> frames;
    int last = mbvq::view_stats(view).frames;
    const QMetaObject::Connection submitted =
        QObject::connect(rhi_widget, &QRhiWidget::frameSubmitted, rhi_widget, [&] {
          const mbvq::RenderStats s = mbvq::view_stats(view);
          if (s.frames > last) {
            last = s.frames;
            frames.push_back(s);
          }
        });
    const auto send = [&](QEvent::Type type, const QPointF &at, Qt::MouseButton button, Qt::MouseButtons buttons) {
      QMouseEvent event(type, at, view->mapToGlobal(at), button, buttons, Qt::NoModifier);
      QApplication::sendEvent(view, &event);
    };
    /* a motion with the middle button down, and the frame it brings */
    const auto pan = [&](const QPointF &at) {
      const size_t n = frames.size();
      send(QEvent::MouseMove, at, Qt::NoButton, Qt::MiddleButton);
      timer.restart();
      while (frames.size() == n && timer.elapsed() < 3000)
        spin(10);
      return frames.size() > n;
    };
    mbvq_c_set_mouse_mode(instance, MBV_MOUSE_MOVE);
    mbvq::set_gpu_wait(view, 15.0);
    spin(300);
    bool panned = true;
    send(QEvent::MouseButtonPress, centre, Qt::MiddleButton, Qt::MiddleButton);
    const size_t first = frames.size();
    for (int m = 1; m <= 4; m++)
      panned = pan(centre + QPointF(3.0 * m, 0.0)) && panned;
    const size_t second = frames.size();
    send(QEvent::MouseButtonRelease, centre + QPointF(12.0, 0.0), Qt::MiddleButton, Qt::NoButton);
    send(QEvent::MouseButtonPress, centre + QPointF(12.0, 0.0), Qt::MiddleButton, Qt::MiddleButton);
    const bool no_frame_between = frames.size() == second;
    for (int m = 1; m <= 2; m++)
      panned = pan(centre + QPointF(12.0 - 3.0 * m, 0.0)) && panned;
    send(QEvent::MouseButtonRelease, centre + QPointF(6.0, 0.0), Qt::MiddleButton, Qt::NoButton);
    spin(400);
    mbvq::set_gpu_wait(view, -1.0);
    QObject::disconnect(submitted);
    bool marked = false;
    std::string strides;
    for (size_t i = first; i < frames.size(); i++) {
      strides += std::string(i == first ? "" : i == second ? " | " : ", ") + std::to_string(frames[i].stride) +
                 (frames[i].gpu_coarser ? "*" : "");
      marked = marked || (i < second && frames[i].gpu_coarser);
    }
    if (!marked)
      std::printf("info\t7c\tthe map's first press passed over no level (strides %s): the rule stayed inert, so the "
                  "check below shows nothing\n",
                  strides.c_str());
    const bool fresh = frames.size() > second && !frames[second].gpu_coarser;
    check(panned && no_frame_between && fresh,
          "7c: a release and a new press of the map view sent back to back (no frame between: " +
              std::string(no_frame_between ? "yes" : "no") + ") start a new drag: after the press before " +
              (marked ? "passed over a marked level" : "passed over none (the rule inert)") +
              ", the new press's first frame passes over none (strides " + strides +
              ", * where a marked level was passed over, the new press after |)");
  }

  /* 6c: the overlays and contours of the batch, with the sizes scaled by the
     view's own pixel ratio (devicePixelRatioF(), not the render target's):
     contours (1 logical px) are thin while they stay 1 physical px (below
     ratio 1.75, expected_px()), and a route (2 logical px, with its waypoint
     disks) is wide from ratio 1 */
  {
    int error = MB_ERROR_NO_ERROR;
    const int nc = data->primary_n_columns, nr = data->primary_n_rows;
    const int ii[6] = {nc / 8, nc / 4, nc * 3 / 8, nc / 2, nc * 5 / 8, nc * 3 / 4};
    const int jj[6] = {nr / 3, nr / 2, nr / 3, nr / 2, nr / 3, nr / 2};
    double lonlat[12];
    mbvq_c_node_lonlat(instance, 6, ii, jj, lonlat);
    double lon[6], lat[6];
    int waypoints[6];
    for (int k = 0; k < 6; k++) {
      lon[k] = lonlat[2 * k];
      lat[k] = lonlat[2 * k + 1];
      waypoints[k] = MBV_ROUTE_WAYPOINT_SIMPLE;
    }
    char name[] = "test route";
    int route = 0;
    mbview_addroute(0, instance, 6, lon, lat, waypoints, MBV_COLOR_RED, 2, 0, name, &route, &error);
    /* a straight purple route of 2 logical px along a row of nodes, for its
       width in the view's own picture */
    {
      const int si[2] = {nc / 4, nc * 3 / 4};
      const int sj[2] = {nr * 3 / 4, nr * 3 / 4};
      double straight[4];
      mbvq_c_node_lonlat(instance, 2, si, sj, straight);
      double slon[2] = {straight[0], straight[2]};
      double slat[2] = {straight[1], straight[3]};
      int swaypoints[2] = {MBV_ROUTE_WAYPOINT_SIMPLE, MBV_ROUTE_WAYPOINT_SIMPLE};
      char sname[] = "straight route";
      int sroute = 0;
      mbview_addroute(0, instance, 2, slon, slat, swaypoints, MBV_COLOR_PURPLE, 2, 0, sname, &sroute, &error);
    }
    mbview_enableviewroutes(0, instance, &error);
    data->route_view_mode = MBV_VIEW_ON;
    mbvq_c_view(instance, MBVQ_VIEW_CONTOUR, true);
    mbvq_c_fullrender(instance);
    timer.restart();
    while ((mbvq::view_stats(view).contour_rez != MBVQ_REZ_FULL || mbvq::view_stats(view).marker_draws == 0) &&
           timer.elapsed() < 20000)
      spin(50);
    const mbvq::RenderStats overlays = mbvq::view_stats(view);
    const double ratio = view->devicePixelRatioF();
    const bool thin = expected_px(1.0, ratio) <= 1;
    check(overlays.pixel_ratio == ratio && overlays.contour_rez == MBVQ_REZ_FULL && overlays.marker_draws > 0 &&
              (thin ? overlays.thin_draws > 0 : overlays.thin_draws == 0 && overlays.wide_draws >= 2),
          "6c: contours (" + std::to_string(overlays.contour_vertices) + " vertices) and a route with its waypoints "
          "drawn at the view's pixel ratio " + text("%.2f", overlays.pixel_ratio) + ": " +
              std::to_string(overlays.thin_draws) + " thin and " + std::to_string(overlays.wide_draws) +
              " wide line draws, " + std::to_string(overlays.marker_draws) + " marker runs");
    /* the straight route's width in physical pixels in grabFramebuffer(),
       across its major axis over the middle half of its length:
       max(1, floor(2 x the view's ratio + 0.25)) by the minor-axis rule */
    const QImage with = rhi_widget->grabFramebuffer().convertToFormat(QImage::Format_RGB32);
    {
      const QRgb purple = qRgb(255, 0, 255);
      int x0 = with.width(), x1 = -1, y0 = with.height(), y1 = -1;
      for (int y = 0; y < with.height(); y++)
        for (int x = 0; x < with.width(); x++)
          if (with.pixel(x, y) == purple) {
            x0 = std::min(x0, x);
            x1 = std::max(x1, x);
            y0 = std::min(y0, y);
            y1 = std::max(y1, y);
          }
      const int expected = expected_px(2.0, ratio);
      const bool across_columns = x1 - x0 >= y1 - y0;
      const int from = across_columns ? x0 + (x1 - x0) / 4 : y0 + (y1 - y0) / 4;
      const int to = across_columns ? x1 - (x1 - x0) / 4 : y1 - (y1 - y0) / 4;
      int lines = 0, right = 0, wider = 0;
      for (int a = from; a < to; a++) {
        int n = 0;
        const int length = across_columns ? with.height() : with.width();
        for (int b = 0; b < length; b++)
          n += with.pixel(across_columns ? a : b, across_columns ? b : a) == purple;
        lines++;
        right += n == expected;
        wider += n > expected;
      }
      check(x1 >= 0 && lines > 20 && wider == 0 && right >= lines * 9 / 10,
            "6c: a 2 px route is " + std::to_string(expected) + " physical px wide in grabFramebuffer() at the view's "
            "ratio " + text("%.2f", ratio) + " (" + std::to_string(right) + " of " + std::to_string(lines) +
                (across_columns ? " columns" : " rows") + ", " + std::to_string(wider) + " wider)");
    }
    if (save != nullptr) {
      const std::string path = std::string(save) + ".overlays.png";
      check(with.save(QString::fromStdString(path)), "picture with the overlays saved as " + path);
    }
  }

  /* 6d: the profile view of a two-point pick across the map, in its window:
     drawn; its frame (2 logical px) max(1, floor(2 x ratio + 0.25)) physical px
     wide in grabFramebuffer(); at most floor(TextureSizeMax / ratio) logical
     px wide (Metal's 16384), so a drawing 5 times a wide window is clamped
     and the core's drawing is at most 16384 px */
  {
    /* from the centre, where the terrain is also in 3D (the click above
       picked there), to the first of a few places beside it with data */
    const int w = data->width, h = data->height;
    for (const auto &end : {std::array<int, 2>{w * 3 / 5, h / 2 + h / 20}, std::array<int, 2>{w * 2 / 5, h / 2},
                            std::array<int, 2>{w / 2, h * 3 / 5}, std::array<int, 2>{w / 2, h * 2 / 5}}) {
      mbview_pick(instance, MBV_PICK_DOWN, w / 2, h / 2);
      mbview_pick(instance, MBV_PICK_MOVE, end[0], end[1]);
      mbview_pick(instance, MBV_PICK_UP, end[0], end[1]);
      if (data->pickinfo_mode == MBV_PICK_TWOPOINT)
        mbview_extract_pick_profile(instance);
      if (npoints_of(data) > 2)
        break;
    }
    mbvq::ProfileWindow *window = mbvq::map_widget(instance)->profile();
    window->open();
    QWidget *profile = window->view();
    auto *profile_rhi = qobject_cast<QRhiWidget *>(profile);
    timer.restart();
    while (mbvq::view_stats(profile).frames == 0 && timer.elapsed() < 10000)
      spin(20);
    print_create_time(profile, "the profile view, first frame");
    spin(200);
    const double ratio = profile->devicePixelRatioF();
    QImage picture;
    if (profile_rhi != nullptr)
      picture = profile_rhi->grabFramebuffer().convertToFormat(QImage::Format_RGB32);
    int dark = 0;
    for (int y = 0; y < picture.height(); y++)
      for (int x = 0; x < picture.width(); x++)
        dark += qGray(picture.pixel(x, y)) < 128;
    /* the frame's top edge: the first run of black or red from the top of
       the middle column */
    int top = 0;
    const int column = picture.width() / 2;
    int y = 0;
    const auto framed = [&](int row) {
      const QRgb p = picture.pixel(column, row);
      return qGreen(p) < 64 && qBlue(p) < 64 && (qRed(p) < 64 || qRed(p) > 192);
    };
    while (y < picture.height() && !framed(y))
      y++;
    while (y < picture.height() && framed(y)) {
      top++;
      y++;
    }
    const int width = expected_px(2.0, ratio);
    check(profile_rhi != nullptr && npoints_of(data) > 2 && dark > 0 && top == width &&
              mbvq::view_stats(profile).pixel_ratio == ratio,
          "6d: the profile of a two-point pick (" + std::to_string(npoints_of(data)) + " points) drawn with QRhi: " +
              std::to_string(dark) + " dark pixels; its 2 px frame " + std::to_string(top) + " physical px wide at "
              "the view's ratio " + text("%.2f", ratio) + " (" + std::to_string(width) + " expected)");
    same_as_offscreen(profile_rhi, picture, ratio, "the profile", [&](mbvq::OffscreenRenderer &offscreen,
                                                                      const mbvq::RenderOptions &options, QImage *c) {
      return offscreen.render_profile(instance, picture.width(), picture.height(), options, c);
    });
    /* the cap from the view's QRhi; five times the visible width, clamped
       to it where that is wider (at ratio 2 here; at ratio 1 the clamp is
       checked by --widgets) */
    const int limit = mbvq::view_state(profile).texture_size_max;
    const int cap = static_cast<int>(std::floor(limit / ratio));
    window->resize(1800, 360);
    window->slider(0)->setValue(5);
    spin(500);
    const QWidget *viewport = profile->parentWidget();
    const int asked = 5 * (viewport != nullptr ? viewport->width() : 0);
    check(limit > 0 && profile->maximumWidth() == cap && profile->width() == std::min(cap, asked) &&
              data->prwidth <= limit,
          "6d: the profile view is at most floor(" + std::to_string(limit) + " / ratio) = " + std::to_string(cap) +
              " px wide: a drawing 5 times its " + std::to_string(asked / 5) + " px viewport is " +
              std::to_string(profile->width()) + " px (" + (asked > cap ? "clamped" : "below the cap") +
              "), the core's " + std::to_string(data->prwidth) + " physical px");
    if (save != nullptr) {
      const std::string path = std::string(save) + ".profile.png";
      check(picture.save(QString::fromStdString(path)), "the profile's picture saved as " + path);
    }
    window->slider(0)->setValue(1);
    spin(300);
    /* The view wider than its cap when a frame runs initialize(), as on a
       QRhi whose largest texture is below the 16384 assumed before the QRhi
       exists (OpenGL or Vulkan on some GPUs); here the maximum is lifted and
       the view made wider. A cap that lowered the maximum inside that paint
       would resize the view there, and QRhiWidget paints a resized view at
       once, inside the paint's frame: the cap comes after the paint. The
       horizontal scroll bar is kept out of the layout meanwhile: where it
       takes space (macOS with scroll bars always shown, Linux), its
       appearance resizes the viewport, and the window then caps the view
       and sizes it to the viewport itself (layout_profile), whatever
       initialize() did. Without it only initialize()'s cap brings the view
       to its cap exactly. */
    {
      auto *area = window->findChild<QScrollArea *>();
      const Qt::ScrollBarPolicy policy = area != nullptr ? area->horizontalScrollBarPolicy() : Qt::ScrollBarAsNeeded;
      if (area != nullptr)
        area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      spin(200);
      const int frames = mbvq::view_stats(profile).frames;
      const int wide = cap + 4000;
      profile->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
      profile->resize(wide, profile->height());
      spin(500);
      const int capped = profile->width();
      const int maximum = profile->maximumWidth();
      const int frames_drawn = mbvq::view_stats(profile).frames - frames;
      if (area != nullptr)
        area->setHorizontalScrollBarPolicy(policy);
      spin(200);
      check(area != nullptr && maximum == cap && capped == cap && frames_drawn > 0 &&
                !mbvq::view_state(profile).failure_shown,
            "6d: a profile view made " + std::to_string(wide) + " px wide, past its cap, draws and is capped after "
            "that frame, with its window's horizontal scroll bar off: " + std::to_string(capped) + " px wide, its "
            "cap " + std::to_string(cap) + " (" + std::to_string(frames_drawn) + " frames)");
    }
    window->dismiss();
  }

  /* 6d: the 3D soundings view in a dock of a main window: drawn, its points
     (3 logical px) and profile lines (1 logical px) at the view's pixel
     ratio in grabFramebuffer(), a click edits, the point-size control acts;
     then floated, docked again, floated and closed while floating
     (QTBUG-151834) */
  {
    auto *host = new QMainWindow;
    host->setCentralWidget(new QLabel(QStringLiteral("the program")));
    host->resize(1300, 900);
    host->show();
    QDockWidget *dock = nullptr;
    mbvq::set_soundings_placement([host, &dock](mbvq::SoundingsWidget *widget) {
      dock = new QDockWidget(widget->title());
      dock->setWidget(widget);
      host->addDockWidget(Qt::RightDockWidgetArea, dock);
      dock->show();
    });
    make_sdg(60, 40, 23);
    check(open_sdg() && dock != nullptr, "6d: the soundings in a dock of the program's main window");
    QWidget *sview = mbvq::soundings_widget() != nullptr ? mbvq::soundings_widget()->view() : nullptr;
    auto *sview_rhi = qobject_cast<QRhiWidget *>(sview);
    if (sview_rhi != nullptr) {
      sdg_camera(90.0f, 0.0f, 1.0f, 0.0f, 0.0f);
      sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_ALL, false);
      mbvq_c_soundings_plot();
      timer.restart();
      while (!mbvq::view_state(sview).drawn && timer.elapsed() < 10000)
        spin(20);
      print_create_time(sview, "the soundings view, first frame in its dock");
      spin(300);
      const double ratio = sview->devicePixelRatioF();
      const QImage picture = sview_rhi->grabFramebuffer().convertToFormat(QImage::Format_RGB32);
      int dark = 0;
      for (int y = 0; y < picture.height(); y++)
        for (int x = 0; x < picture.width(); x++)
          dark += qGray(picture.pixel(x, y)) < 128;
      /* a sounding in the middle, good, with good neighbours along its ping:
         its point's height in its column, and the line's height between it
         and the next sounding */
      int error = MB_ERROR_NO_ERROR;
      mb3dsoundings_screen_positions(0, &error);
      const int H = picture.height();
      size_t middle = sdg_storage.size() / 2 + 20;
      while (middle + 1 < sdg_storage.size() &&
             (sdg_storage[middle].beamflag != MB_FLAG_NONE || sdg_storage[middle + 1].beamflag != MB_FLAG_NONE ||
              sdg_storage[middle - 1].beamflag != MB_FLAG_NONE))
        middle++;
      const auto run = [&](int x, int y0) {
        const QRgb black = qRgb(0, 0, 0);
        int y = y0, n = 0;
        while (y > 0 && picture.pixel(x, y - 1) == black)
          y--;
        while (y < H && picture.pixel(x, y) == black) {
          n++;
          y++;
        }
        return n;
      };
      const int px = sdg_storage[middle].winx, py = H - 1 - sdg_storage[middle].winy;
      const int point = run(px, py);
      const int lx = (sdg_storage[middle].winx + sdg_storage[middle + 1].winx) / 2;
      const int ly = H - 1 - (sdg_storage[middle].winy + sdg_storage[middle + 1].winy) / 2;
      const int line = run(lx, ly);
      const int point_px = expected_px(3.0, ratio);
      const int line_px = expected_px(1.0, ratio);
      if (save != nullptr) {
        const std::string path = std::string(save) + ".soundings.png";
        check(picture.save(QString::fromStdString(path)), "the soundings' picture saved as " + path);
      }
      check(dark > 0 && point == point_px && line == line_px && mbvq::view_stats(sview).profile_wide == (line_px > 1),
            "6d: the soundings drawn with QRhi (" + std::to_string(dark) + " dark pixels): a 3 px point " +
                std::to_string(point) + " and a 1 px profile line " + std::to_string(line) +
                " physical px high at the view's ratio " + text("%.2f", ratio) + " (" + std::to_string(point_px) +
                " and " + std::to_string(line_px) + " expected)");
      same_as_offscreen(sview_rhi, picture, ratio, "the 3D soundings",
                        [&](mbvq::OffscreenRenderer &offscreen, const mbvq::RenderOptions &options, QImage *c) {
                          return offscreen.render_soundings(picture.width(), picture.height(), options, c, 50);
                        });
      /* the point-size control: 1 logical px */
      QDoubleSpinBox *control = mbvq::soundings_widget()->findChild<QDoubleSpinBox *>();
      if (control != nullptr)
        control->setValue(1.0);
      spin(300);
      const QImage smaller = sview_rhi->grabFramebuffer().convertToFormat(QImage::Format_RGB32);
      int small_point = 0;
      {
        const QRgb black = qRgb(0, 0, 0);
        int y = py;
        while (y > 0 && smaller.pixel(px, y - 1) == black)
          y--;
        while (y < H && smaller.pixel(px, y) == black) {
          small_point++;
          y++;
        }
      }
      if (control != nullptr)
        control->setValue(3.0);
      const int small_px = expected_px(1.0, ratio);
      check(control != nullptr && small_point == std::max(small_px, line_px),
            "6d: the point-size control at 1 px draws the points " + std::to_string(small_point) +
                " physical px high (" + std::to_string(small_px) + ", within a line " + std::to_string(line_px) +
                " high)");
      /* a click on a sounding edits it */
      const int edits = sdg_edits;
      const QPointF at(sdg_storage[middle].winx / ratio, (H - 1 - sdg_storage[middle].winy) / ratio);
      QMouseEvent down(QEvent::MouseButtonPress, at, sview->mapToGlobal(at), Qt::LeftButton, Qt::LeftButton,
                       Qt::NoModifier);
      QApplication::sendEvent(sview, &down);
      QMouseEvent up(QEvent::MouseButtonRelease, at, sview->mapToGlobal(at), Qt::LeftButton, Qt::NoButton,
                     Qt::NoModifier);
      QApplication::sendEvent(sview, &up);
      spin(200);
      check(sdg_edits == edits + 1, "6d: a click on a sounding after the first frame edits it");
      /* 7c-7: the GPU-time fallback of the 3D soundings view in a window.
         Its paints are timed from outside, as the bench times the map's
         (FrameProbe): an event filter sees a paint begin, the application's
         hook after the paint's delivery sees it end (set_delivered), and the
         window's next submission (frameSubmitted, after the view's own
         measurement) ends the frame, with the renderer's statistics then. */
      {
        struct PaintedFrame {
          double paint = 0.0;      /* the paint event began (ms since the epoch of rhi::Clock) */
          double painted = -1.0;   /* ... and ended */
          double submitted = 0.0;  /* the window submitted the frame */
          mbvq::RenderStats stats; /* the renderer's statistics then */
        };
        class PaintProbe : public QObject {
         public:
          explicit PaintProbe(QWidget *view) : view_(view) {}
          bool eventFilter(QObject *object, QEvent *event) override {
            if (object == view_ && event->type() == QEvent::Paint) {
              paints++;
              painting_ = true;
              frame_ = PaintedFrame();
              frame_.paint = rhi::milliseconds(rhi::Clock::now());
            }
            return QObject::eventFilter(object, event);
          }
          /* an event delivered (set_delivered): the end of a paint of the view */
          void delivered(const QObject *receiver, QEvent::Type type) {
            if (type == QEvent::Paint && painting_ && receiver == view_)
              frame_.painted = rhi::milliseconds(rhi::Clock::now());
          }
          /* the window submitted a frame, after a paint of the view or without one */
          void submitted() {
            submissions++;
            if (!painting_)
              return;
            painting_ = false;
            frame_.submitted = rhi::milliseconds(rhi::Clock::now());
            frame_.stats = mbvq::view_stats(view_);
            frames.push_back(frame_);
          }

          std::vector<PaintedFrame> frames; /* the painted frames, in order */
          int paints = 0;                   /* paint events of the view */
          int submissions = 0;              /* submissions of its window */

         private:
          QWidget *view_;
          bool painting_ = false;
          PaintedFrame frame_;
        };
        PaintProbe probe(sview);
        sview->installEventFilter(&probe);
        set_delivered([&probe](const QObject *receiver, QEvent::Type type) { probe.delivered(receiver, type); });
        QObject::connect(sview_rhi, &QRhiWidget::frameSubmitted, &probe, [&probe] { probe.submitted(); });
        /* frame_after() waits up to timeout_ms for a painted frame after the
           first n, quiet() until no frame came for 200 ms (at most 4 s) */
        const auto frame_after = [&](size_t n, int timeout_ms) {
          timer.restart();
          while (probe.frames.size() <= n && timer.elapsed() < timeout_ms)
            spin(20);
          return probe.frames.size() > n;
        };
        const auto quiet = [&] {
          for (int i = 0; i < 20; i++) {
            const size_t n = probe.frames.size();
            spin(200);
            if (probe.frames.size() == n)
              break;
          }
        };
        const auto mouse = [sview](QEvent::Type type, const QPointF &where, Qt::MouseButton button,
                                   Qt::MouseButtons buttons) {
          QMouseEvent event(type, where, sview->mapToGlobal(where), button, buttons, Qt::NoModifier);
          QApplication::sendEvent(sview, &event);
        };
        const int count = static_cast<int>(sdg_storage.size());
        /* the soundings drawn at a level of detail: every 2^level-th */
        const auto drawn_at = [count](int level) { return (count + (1 << level) - 1) >> level; };

        /* 1. Without the hook, a painted frame: its render() begins inside
           the paint; its wait (from the end of render() to the end of the
           paint, in which QRhiWidget ends the offscreen frame and waits for
           it) is above 0 and at most what is seen from outside; its total
           lies between that wait and the time from the paint's start to the
           submission seen here; it is measured once; and the fallback can
           act on the APIs that wait for the GPU there (Metal, Vulkan and
           Direct3D 12). */
        quiet();
        const mbvq::RenderStats unpainted = mbvq::view_stats(sview);
        const size_t first_from = probe.frames.size();
        sview->update();
        const bool repainted = frame_after(first_from, 5000);
        const PaintedFrame first = repainted ? probe.frames[first_from] : PaintedFrame();
        const mbvq::RenderStats &f = first.stats;
        const QRhiWidget::Api api = sview_rhi->api();
        const bool api_waits =
            api == QRhiWidget::Api::Metal || api == QRhiWidget::Api::Vulkan || api == QRhiWidget::Api::Direct3D12;
        check(repainted && f.frames > unpainted.frames && first.painted >= first.paint &&
                  f.render_begin_ms >= first.paint && f.wait_ms > 0.0 && f.wait_ms <= first.painted - f.render_end_ms &&
                  f.frame_ms >= f.wait_ms && f.frame_ms <= first.submitted - first.paint &&
                  f.wait_frames == unpainted.wait_frames + 1 && f.gpu_rule == api_waits,
              "7c: a paint of the 3D soundings view timed from outside: its render() began " +
                  text("%.3f", f.render_begin_ms - first.paint) + " ms after the paint; the wait " +
                  text("%.3f", f.wait_ms) + " ms, at most the " + text("%.3f", first.painted - f.render_end_ms) +
                  " ms from the end of render() to the end of the paint; the frame " + text("%.3f", f.frame_ms) +
                  " ms, at most the " + text("%.3f", first.submitted - first.paint) +
                  " ms to the submission; measured once (" + std::to_string(f.wait_frames - unpainted.wait_frames) +
                  "); the fallback can act: " + (f.gpu_rule ? "yes" : "no") + " (" +
                  mbvq::rhi_api_name(mbvq::rhi_api()) + ")");

        /* 7d1: the view's position and colour chunks static with storage use
           where the backend can splat, on Metal always; on the other APIs an
           information line names the choice (it follows their Compute
           feature and thread limits) */
        if (api == QRhiWidget::Api::Metal)
          check(repainted && f.soundings_storage == 1,
                "7d1: the 3D soundings view's position and colour chunks are static with storage use on Metal (" +
                    std::to_string(f.soundings_storage) + ")");
        else
          std::printf("info\t7d1\tthe soundings' chunks with storage use: %d (%s)\n", f.soundings_storage,
                      mbvq::rhi_api_name(mbvq::rhi_api()));

        /* A grab renders outside a paint, and the window submits frames
           without a paint of the view (its central label repainted): neither
           is measured. A flush that painted the view after all is tried
           again, up to three times. */
        mbvq::RenderStats ungrabbed, grabbed, flushed;
        bool grab_drawn = false, view_painted = true;
        int flushes = 0;
        for (int attempt = 0; attempt < 3 && view_painted; attempt++) {
          quiet();
          ungrabbed = mbvq::view_stats(sview);
          grab_drawn = !sview_rhi->grabFramebuffer().isNull();
          grabbed = mbvq::view_stats(sview);
          const int paints = probe.paints;
          const int submissions = probe.submissions;
          host->centralWidget()->update();
          timer.restart();
          while (probe.submissions == submissions && timer.elapsed() < 5000)
            spin(20);
          flushed = mbvq::view_stats(sview);
          flushes = probe.submissions - submissions;
          view_painted = probe.paints != paints;
        }
        check(grab_drawn && !view_painted && flushes > 0 && grabbed.frames > ungrabbed.frames &&
                  flushed.frames == grabbed.frames && flushed.wait_ms == ungrabbed.wait_ms &&
                  flushed.frame_ms == ungrabbed.frame_ms && flushed.wait_frames == ungrabbed.wait_frames,
              "7c: a grabFramebuffer() of the 3D soundings view (its render() outside a paint) and " +
                  std::to_string(flushes) + " submission" + (flushes == 1 ? "" : "s") +
                  " of its window without a paint of the view (the central label repainted) measure nothing: the "
                  "wait " + text("%.3f", flushed.wait_ms) + " ms, the frame " + text("%.3f", flushed.frame_ms) +
                  " ms and the " + std::to_string(flushed.wait_frames) + " frames measured stay");

        /* A paint of the view that runs no render() (sent while the view's
           updates are disabled, so that QRhiWidget returns at once, as it
           does without a size), then a submission of its window while they
           stay disabled (no paint of the view can come between): not
           measured, the paint having run no render(). Updates are enabled
           again after the reading, which asks for a frame. */
        mbvq::RenderStats unsent, sent;
        bool sent_paint = false, sent_view_painted = true;
        int sent_flushes = 0;
        for (int attempt = 0; attempt < 3 && (!sent_paint || sent_view_painted); attempt++) {
          quiet();
          unsent = mbvq::view_stats(sview);
          sview->setUpdatesEnabled(false);
          const int paints = probe.paints;
          QPaintEvent paint(sview->rect());
          QApplication::sendEvent(sview, &paint);
          sent_paint = probe.paints == paints + 1;
          const int submissions = probe.submissions;
          host->centralWidget()->update();
          timer.restart();
          while (probe.submissions == submissions && timer.elapsed() < 5000)
            spin(20);
          sent = mbvq::view_stats(sview);
          sent_flushes = probe.submissions - submissions;
          sent_view_painted = probe.paints != paints + 1;
          sview->setUpdatesEnabled(true);
        }
        quiet();
        check(sent_paint && !sent_view_painted && sent_flushes > 0 && sent.frames == unsent.frames &&
                  sent.wait_ms == unsent.wait_ms && sent.frame_ms == unsent.frame_ms &&
                  sent.wait_frames == unsent.wait_frames,
              "7c: a paint event of the 3D soundings view that runs no render() (sent while its updates are "
              "disabled) and " + std::to_string(sent_flushes) + " submission" + (sent_flushes == 1 ? "" : "s") +
                  " of its window after it measure nothing: the wait " + text("%.3f", sent.wait_ms) +
                  " ms, the frame " + text("%.3f", sent.frame_ms) + " ms and the " +
                  std::to_string(sent.wait_frames) + " frames measured stay (renderer frames " +
                  std::to_string(sent.frames - unsent.frames) + " more)");

        /* 2. The hook's fake wait of 15 ms (T or more, so that the total of
           every frame exceeds T): three still frames, without a press, draw
           every sounding, each measured once */
        mbvq::set_gpu_wait(sview, 15.0);
        const mbvq::RenderStats unhooked = mbvq::view_stats(sview);
        const size_t still_from = probe.frames.size();
        bool still = true;
        for (int i = 0; i < 3; i++) {
          const size_t n = probe.frames.size();
          mbvq_c_soundings_plot();
          still = still && frame_after(n, 5000);
        }
        for (size_t i = still_from; i < probe.frames.size(); i++) {
          const mbvq::RenderStats &s = probe.frames[i].stats;
          still = still && s.soundings_lod == 0 && s.soundings_drawn == count && s.soundings_stride == 12 && s.gpu_rule;
        }
        const mbvq::RenderStats hooked = mbvq::view_stats(sview);
        const int still_frames = static_cast<int>(probe.frames.size() - still_from);
        check(still && !mbvq::view_state(sview).camera_press && hooked.lod_frames == unhooked.lod_frames &&
                  hooked.wait_frames - unhooked.wait_frames == still_frames,
              "7c: with the hook's fake wait of 15 ms, three still frames of the 3D soundings view (" +
                  std::to_string(still_frames) + " painted, as many measured) draw all " + std::to_string(count) +
                  " soundings at level 0; frames at a level above 0: " + std::to_string(hooked.lod_frames));

        /* 3. A camera press (the middle button, rotating) and five motions,
           each waited for. Under the fake wait a frame is GPU-slow where its
           measured rest (frame - wait) stays under T: two such frames in a
           row at level 0 mark it, the next frames draw every 2nd sounding,
           and the constant wait stops the press at level 1 (the gain check).
           Each frame's level is checked against that rule for the rests
           measured: level 1 from the third frame on where the first two
           rests are under T; otherwise an info line, and no frame reduced
           while the rests stay at T or more (a slow composition). */
        mb3dsoundings.mouse_mode = MBS_MOUSE_ROTATE;
        const QPointF middle_of(sview->width() / 2.0, sview->height() / 2.0);
        quiet();
        const mbvq::RenderStats unpressed = mbvq::view_stats(sview);
        const size_t press_from = probe.frames.size();
        mouse(QEvent::MouseButtonPress, middle_of, Qt::MiddleButton, Qt::MiddleButton);
        const mbvq::ViewState pressed = mbvq::view_state(sview);
        bool moved = true;
        for (int m = 1; m <= 5; m++) {
          const size_t n = probe.frames.size();
          mouse(QEvent::MouseMove, middle_of + QPointF(6.0 * m, 0.0), Qt::NoButton, Qt::MiddleButton);
          moved = moved && frame_after(n, 3000);
        }
        const double mark = rhi::kDragFrameMs - rhi::kDragMarginMs;
        const int press_frames = static_cast<int>(probe.frames.size() - press_from);
        bool levels = moved && press_frames >= 5;
        int level = 0, slow = 0, reduced = 0;
        std::string lods, rests;
        for (size_t i = press_from; i < probe.frames.size(); i++) {
          const mbvq::RenderStats &s = probe.frames[i].stats;
          const double rest = s.frame_ms - s.wait_ms;
          levels = levels && s.soundings_lod == level && s.soundings_drawn == drawn_at(level) &&
                   s.soundings_stride == (12 << level) && s.gpu_rule;
          reduced += level > 0 ? 1 : 0;
          lods += (lods.empty() ? "" : ", ") + std::to_string(s.soundings_lod);
          rests += (rests.empty() ? "" : ", ") + text("%.2f", rest);
          /* the rule with a constant fake wait: two GPU-slow frames in a row
             at level 0 mark it, and level 1 stays */
          if (level == 0) {
            slow = rest < mark ? slow + 1 : 0;
            if (slow == rhi::kGpuSlowFrames)
              level = 1;
          }
        }
        const auto rest_of = [&](size_t i) {
          return i < probe.frames.size() ? probe.frames[i].stats.frame_ms - probe.frames[i].stats.wait_ms : mark;
        };
        const bool fast = rest_of(press_from) < mark && rest_of(press_from + 1) < mark;
        if (!fast)
          std::printf("info\t7c\trest >= T: the rule stays inert (the rests of the camera press's first two frames: "
                      "%.2f and %.2f ms, T %.3f ms)\n",
                      rest_of(press_from), rest_of(press_from + 1), mark);
        const mbvq::RenderStats press_end = mbvq::view_stats(sview);
        check(pressed.camera_press && pressed.buttons == 2 && levels &&
                  press_end.lod_frames - unpressed.lod_frames == reduced && (!fast || reduced == press_frames - 2),
              "7c: a camera press of the 3D soundings view (the middle button) with five motions under the fake wait "
              "of 15 ms: levels " + lods + " in its " + std::to_string(press_frames) + " frames (rests " + rests +
                  " ms, T " + text("%.3f", mark) + " ms): " +
                  (fast ? "from the third frame on every 2nd sounding (" + std::to_string(drawn_at(1)) + " of " +
                              std::to_string(count) + "), the constant wait stopping the press at level 1"
                        : std::string("the levels the rule gives for these rests, the first two not both under T")) +
                  "; " + std::to_string(press_end.lod_frames - unpressed.lod_frames) + " frames at a level above 0");

        /* 4. Quiet for 200 ms, then the release: the core asks for no frame
           after it, the view asks for one (frame_requests), drawn with every
           sounding */
        quiet();
        const mbvq::ViewState held = mbvq::view_state(sview);
        const mbvq::RenderStats unreleased = mbvq::view_stats(sview);
        const size_t release_from = probe.frames.size();
        mouse(QEvent::MouseButtonRelease, middle_of + QPointF(30.0, 0.0), Qt::MiddleButton, Qt::NoButton);
        const mbvq::ViewState released = mbvq::view_state(sview);
        const bool release_drawn = frame_after(release_from, 5000);
        const mbvq::RenderStats r = release_drawn ? probe.frames[release_from].stats : unreleased;
        check(held.camera_press && released.frame_requests == held.frame_requests + 1 && !released.camera_press &&
                  released.buttons == 0 && release_drawn && r.soundings_lod == 0 && r.soundings_drawn == count &&
                  r.soundings_stride == 12 && r.lod_frames == unreleased.lod_frames,
              "7c: the release of the camera press asks for a frame (" +
                  std::to_string(released.frame_requests - held.frame_requests) + " request), " +
                  (release_drawn ? "drawn within 5 s" : "none drawn within 5 s") + " at level " +
                  std::to_string(r.soundings_lod) + " with " + std::to_string(r.soundings_drawn) + " of " +
                  std::to_string(count) + " soundings; frames at a level above 0 still " +
                  std::to_string(r.lod_frames));

        /* 4b. A release and a new camera press sent back to back, with no
           frame between them: the view ends the press in its renderer at
           the release, so the new press starts afresh. A press with three
           motions (level 1 from its third frame where the rests are under
           T), then the release and the press together, then two motions:
           their frames draw every sounding again until two GPU-slow frames
           mark level 0 anew. */
        quiet();
        const size_t again_from = probe.frames.size();
        mouse(QEvent::MouseButtonPress, middle_of, Qt::MiddleButton, Qt::MiddleButton);
        bool again_moved = true;
        for (int m = 1; m <= 3; m++) {
          const size_t n = probe.frames.size();
          mouse(QEvent::MouseMove, middle_of + QPointF(6.0 * m, 0.0), Qt::NoButton, Qt::MiddleButton);
          again_moved = again_moved && frame_after(n, 3000);
        }
        const bool marked = probe.frames.size() > again_from + 2 && probe.frames.back().stats.soundings_lod == 1;
        const size_t repress_from = probe.frames.size();
        mouse(QEvent::MouseButtonRelease, middle_of + QPointF(18.0, 0.0), Qt::MiddleButton, Qt::NoButton);
        mouse(QEvent::MouseButtonPress, middle_of + QPointF(18.0, 0.0), Qt::MiddleButton, Qt::MiddleButton);
        const bool no_frame_between = probe.frames.size() == repress_from;
        const mbvq::ViewState repressed = mbvq::view_state(sview);
        for (int m = 1; m <= 2; m++) {
          const size_t n = probe.frames.size();
          mouse(QEvent::MouseMove, middle_of + QPointF(18.0 - 6.0 * m, 0.0), Qt::NoButton, Qt::MiddleButton);
          again_moved = again_moved && frame_after(n, 3000);
        }
        std::string again_lods;
        bool fresh = probe.frames.size() >= repress_from + 2;
        for (size_t i = again_from; i < probe.frames.size(); i++) {
          again_lods += std::string(i == again_from ? "" : i == repress_from ? " | " : ", ") +
                        std::to_string(probe.frames[i].stats.soundings_lod);
          if (i >= repress_from && i < repress_from + 2)
            fresh = fresh && probe.frames[i].stats.soundings_lod == 0 &&
                    probe.frames[i].stats.soundings_drawn == count;
        }
        mouse(QEvent::MouseButtonRelease, middle_of + QPointF(6.0, 0.0), Qt::MiddleButton, Qt::NoButton);
        quiet();
        if (!marked)
          std::printf("info\t7c\tthe press before the back-to-back release and press reached no level of detail "
                      "(levels %s): the rule stayed inert, so the check below shows nothing\n",
                      again_lods.c_str());
        check(again_moved && no_frame_between && repressed.camera_press && fresh,
              "7c: a release and a new camera press of the 3D soundings view sent back to back (no frame between: " +
                  std::string(no_frame_between ? "yes" : "no") + ") start a new press: after the press before " +
                  (marked ? "reached level 1" : "stayed at level 0 (the rule inert)") + ", the new press's first " +
                  "two frames draw all " + std::to_string(count) + " soundings at level 0 (levels " + again_lods +
                  ", the new press after |)");

        /* 5. The camera from above again, then an erase drag (the left
           button in the Erase mode): a press and five motions along ping
           15, each onto a good sounding at least the erase radius
           (MBS_ERASE_THRESHOLD px) and 2 px from those before, so that each
           erases and draws (edits and frames rise). No camera press: every
           sounding in every frame under the fake wait, and the release asks
           for no frame. */
        sdg_camera(90.0f, 0.0f, 1.0f, 0.0f, 0.0f);
        const size_t camera_from = probe.frames.size();
        mbvq_c_soundings_plot();
        frame_after(camera_from, 5000);
        quiet();
        mb3dsoundings.edit_mode = MBS_EDIT_ERASE;
        mb3dsoundings_screen_positions(0, &error);
        const int ping = 15;
        std::vector<size_t> walk;
        for (size_t k = 0; k < sdg_storage.size() && walk.size() < 6; k++) {
          const struct mb3dsoundings_sounding_struct &s = sdg_storage[k];
          bool apart = s.iping == ping && s.beamflag == MB_FLAG_NONE;
          for (const size_t w : walk) {
            const double dx = s.winx - sdg_storage[w].winx, dy = s.winy - sdg_storage[w].winy;
            apart = apart && std::sqrt(dx * dx + dy * dy) >= MBS_ERASE_THRESHOLD + 2.0;
          }
          if (apart)
            walk.push_back(k);
        }
        const auto on_screen = [&](size_t k) {
          return QPointF(sdg_storage[k].winx / ratio, (H - 1 - sdg_storage[k].winy) / ratio);
        };
        const mbvq::ViewState unerased_state = mbvq::view_state(sview);
        const mbvq::RenderStats unerased = mbvq::view_stats(sview);
        const int edits_before = sdg_edits;
        const size_t erase_from = probe.frames.size();
        bool erasing = walk.size() == 6, no_press = true;
        for (size_t i = 0; i < walk.size(); i++) {
          const int edited = sdg_edits;
          const size_t n = probe.frames.size();
          if (i == 0)
            mouse(QEvent::MouseButtonPress, on_screen(walk[i]), Qt::LeftButton, Qt::LeftButton);
          else
            mouse(QEvent::MouseMove, on_screen(walk[i]), Qt::NoButton, Qt::LeftButton);
          const mbvq::ViewState during = mbvq::view_state(sview);
          no_press = no_press && !during.camera_press && during.buttons == 1;
          erasing = erasing && sdg_edits > edited && frame_after(n, 3000);
        }
        if (!walk.empty())
          mouse(QEvent::MouseButtonRelease, on_screen(walk.back()), Qt::LeftButton, Qt::NoButton);
        const mbvq::ViewState erased_state = mbvq::view_state(sview);
        const int erase_frames = static_cast<int>(probe.frames.size() - erase_from);
        bool every = true;
        for (size_t i = erase_from; i < probe.frames.size(); i++) {
          const mbvq::RenderStats &s = probe.frames[i].stats;
          every = every && s.soundings_lod == 0 && s.soundings_drawn == count && s.soundings_stride == 12;
        }
        const mbvq::RenderStats erased = mbvq::view_stats(sview);
        mb3dsoundings.edit_mode = MBS_EDIT_TOGGLE;
        check(erasing && no_press && every && erased.lod_frames == unerased.lod_frames &&
                  erased_state.frame_requests == unerased_state.frame_requests && erased_state.buttons == 0 &&
                  !erased_state.camera_press,
              "7c: an erase drag of the 3D soundings view (the left button: a press and five motions along ping " +
                  std::to_string(ping) + ", " + std::to_string(sdg_edits - edits_before) + " soundings erased in " +
                  std::to_string(erase_frames) + " frames) is no camera press: all " + std::to_string(count) +
                  " soundings in every frame under the fake wait, frames at a level above 0 still " +
                  std::to_string(erased.lod_frames) + ", and its release asks for no frame (" +
                  std::to_string(erased_state.frame_requests - unerased_state.frame_requests) + " requests)");

        /* 6. The hook off again (measured waits), and the probe's hooks gone */
        mbvq::set_gpu_wait(sview, -1.0);
        sview->removeEventFilter(&probe);
        set_delivered(nullptr);
      }
      /* Dismiss, as mbeditviz-qt: the view is hidden, then the program
         releases its soundings in the notification, and the hidden view,
         which draws no frame that would drop them, lets go of what it holds
         of them in the same call. A million soundings, whose buffers the
         QRhi would otherwise free only in a frame. On Metal the GPU memory
         itself is measured: the device's allocations right before and right
         after the call, without an event between, after three more frames
         of the view (at least two required, Metal's two frame slots), so
         that the drop is the view's own: without them the call's finish()
         also frees what the window's earlier frames left. A Metal device
         that cannot be read fails the check; elsewhere only the renderer's
         count. */
      sdg_release_on_dismiss = true;
      sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_NONE, false);
      {
        make_sdg(2500, 400, 23);
        open_sdg();
        mbvq_c_soundings_plot();
        const int count = static_cast<int>(sdg_storage.size());
        timer.restart();
        while ((mbvq::view_stats(sview).soundings_drawn != count || mbvq::view_stats(sview).soundings_waiting) &&
               timer.elapsed() < 20000)
          spin(20);
        spin(300);
        const int drawn_frames = mbvq::view_stats(sview).frames;
        for (int i = 0; i < 3; i++) {
          sview->update();
          spin(150);
        }
        const mbvq::RenderStats shown = mbvq::view_stats(sview);
        const double gpu_before = metal_allocated_mb();
        mbvq_c_soundings_dismiss();
        const double gpu_after = metal_allocated_mb();
        spin(300);
        const mbvq::RenderStats hidden = mbvq::view_stats(sview);
        const bool visible = sview->isVisible();
        std::vector<struct mb3dsoundings_sounding_struct>().swap(sdg_storage);
        const double held = shown.sounding_held_bytes / 1048576.0;
        const int frames = shown.frames - drawn_frames;
        const bool metal = sview_rhi->api() == QRhiWidget::Api::Metal;
        const bool measured = metal && gpu_before >= 0.0 && gpu_after >= 0.0;
        check(shown.soundings_drawn == count && held > 0.0 && !visible && hidden.frames == shown.frames &&
                  hidden.soundings == 0 && hidden.sounding_held_bytes == 0 &&
                  (!metal || (measured && frames >= 2 && gpu_before - gpu_after >= 0.75 * held)),
              "6d: Dismiss lets go of the buffers of " + std::to_string(count) + " soundings (" + text("%.1f", held) +
                  " MB) in its call, without a frame of the hidden view: " +
                  (!metal     ? std::string("its renderer holds none (the GPU memory is measured on Metal only)")
                   : measured ? "the Metal device holds " + text("%.1f", gpu_before - gpu_after) + " MB less (after " +
                                    std::to_string(frames) + " more frames)"
                              : std::string("the Metal device's allocations could not be read")));
      }
      /* The next soundings drawn; then Dismiss while the dock is closed (its
         close button: the program keeps its soundings until a menu of its
         own dismisses them): the view, hidden already, gets no Hide event,
         and lets them go all the same. */
      {
        make_sdg(60, 40, 23);
        open_sdg();
        mbvq_c_soundings_plot();
        timer.restart();
        while (mbvq::view_stats(sview).soundings_drawn == 0 && timer.elapsed() < 10000)
          spin(20);
        spin(200);
        const mbvq::RenderStats shown = mbvq::view_stats(sview);
        dock->close();
        spin(300);
        const mbvq::RenderStats closed = mbvq::view_stats(sview);
        mbvq_c_soundings_dismiss();
        spin(300);
        const mbvq::RenderStats dismissed = mbvq::view_stats(sview);
        dock->show();
        spin(300);
        check(shown.soundings_drawn == static_cast<int>(sdg_storage.size()) && shown.sounding_held_bytes > 0 &&
                  closed.sounding_held_bytes == shown.sounding_held_bytes && dismissed.soundings == 0 &&
                  dismissed.sounding_held_bytes == 0,
              "6d: the next soundings shown are drawn (" + std::to_string(shown.soundings_drawn) + "); Dismiss "
              "while their dock is closed lets them go (GPU bytes " + std::to_string(closed.sounding_held_bytes) +
                  " while the dock is closed, " + std::to_string(dismissed.sounding_held_bytes) + " after Dismiss)");
      }
      /* A host that docks its floating soundings dock again when the editor
         tells it that it closed (SoundingsWidget::closed, at Dismiss): the
         floating window and its QRhi go in the middle of Dismiss, before
         the program releases the soundings. The view lets them go without
         that QRhi, and draws the next soundings in the main window. */
      {
        make_sdg(60, 40, 23);
        open_sdg();
        dock->setFloating(true);
        spin(1200);
        mbvq_c_soundings_plot();
        spin(400);
        const bool floated = dock->isFloating() && mbvq::view_state(sview).renderer &&
                             mbvq::view_stats(sview).soundings_drawn == static_cast<int>(sdg_storage.size());
        const QMetaObject::Connection redock = QObject::connect(
            mbvq::soundings_widget(), &mbvq::SoundingsWidget::closed, dock, [dock] { dock->setFloating(false); });
        mbvq::soundings_widget()->dismiss();
        QObject::disconnect(redock);
        const bool docked = !dock->isFloating();
        spin(800);
        const mbvq::RenderStats dismissed = mbvq::view_stats(sview);
        make_sdg(60, 40, 23);
        open_sdg();
        mbvq_c_soundings_plot();
        timer.restart();
        while (mbvq::view_stats(sview).soundings_drawn == 0 && timer.elapsed() < 10000)
          spin(20);
        const mbvq::RenderStats again = mbvq::view_stats(sview);
        check(floated && docked && dismissed.sounding_held_bytes == 0 && !dock->isFloating() &&
                  again.soundings_drawn == static_cast<int>(sdg_storage.size()) && mbvq::view_state(sview).renderer &&
                  !mbvq::view_state(sview).failure_shown,
              "6d: a host docks the floating soundings dock again at Dismiss (on SoundingsWidget::closed), which "
              "takes the floating window's QRhi: the next soundings are drawn in the main window (" +
                  std::to_string(again.soundings_drawn) + ")");
      }
      sdg_release_on_dismiss = false;
      /* float, dock again, float, then end the soundings while floating */
      bool moves = true;
      std::string drawn_after;
      for (bool floating : {true, false, true}) {
        dock->setFloating(floating);
        spin(1200);
        const int frames = mbvq::view_stats(sview).frames;
        mbvq_c_soundings_plot();
        /* the frame comes when the window is next composed: waited for, up
           to 5 s, rather than a fixed time */
        timer.restart();
        while (mbvq::view_stats(sview).frames == frames && timer.elapsed() < 5000)
          spin(20);
        const bool drew = mbvq::view_stats(sview).frames > frames && mbvq::view_state(sview).renderer &&
                          !mbvq::view_state(sview).failure_shown;
        print_create_time(sview, floating ? "the soundings view, its dock floated" : "the soundings view, docked again");
        drawn_after += drew ? "1" : "0";
        moves = moves && drew && dock->isFloating() == floating;
      }
      const int removed = mbvq::stale_callbacks_removed();
      mb3dsoundings_end(0, &error);
      spin(200);
      const int removed_after = mbvq::stale_callbacks_removed();
      delete host;
      spin(200);
      check(moves && removed_after > removed,
            "6d: the soundings dock floated, docked again and floated, drawing each time (" + drawn_after +
                "); ended while floating, its view removed QRhiWidget's callback from the QRhi it left (" +
                std::to_string(removed_after - removed) + "), then the main window went (QTBUG-151834)");
    }
    else {
      delete host;
    }
    mbvq::set_soundings_placement({});
  }

  /* 6d: mb3dsoundings_open() shows the soundings window, and a QRhi view
     paints at once when it is shown, inside the call: the call scales the
     soundings before it shows the window, so that the view reads their
     positions once, at their final scale, and writes them once. The first
     open of the standalone soundings window, as mbeditviz-qt has it, then
     Dismiss and the next area's open, as mbeditviz-qt opens each area (no
     plot call of its own); each must paint inside the call. */
  {
    /* the next area's 100,000 soundings opened; the frames the view drew inside the call */
    const auto open_area = [](int flag_every, int *inside) -> QWidget * {
      make_sdg(250, 400, flag_every);
      QWidget *previous = mbvq::soundings_widget() != nullptr ? mbvq::soundings_widget()->view() : nullptr;
      const int frames = previous != nullptr ? mbvq::view_stats(previous).frames : 0;
      QWidget *opened = open_sdg() ? mbvq::soundings_widget()->view() : nullptr;
      *inside = opened != nullptr ? mbvq::view_stats(opened).frames - (opened == previous ? frames : 0) : 0;
      return opened;
    };
    int inside_first = 0, inside_next = 0;
    QWidget *sview = open_area(23, &inside_first);
    const int count = static_cast<int>(sdg_storage.size());
    const auto wait_stats = [&] {
      timer.restart();
      while ((mbvq::view_stats(sview).soundings_drawn != count || mbvq::view_stats(sview).soundings_waiting) &&
             timer.elapsed() < 20000)
        spin(20);
      spin(200);
      return mbvq::view_stats(sview);
    };
    mbvq::RenderStats first_open, dismissed, next_open;
    QWidget *next_view = nullptr;
    if (qobject_cast<QRhiWidget *>(sview) != nullptr) {
      first_open = wait_stats();
      sdg_release_on_dismiss = true;
      mbvq_c_soundings_dismiss();
      sdg_release_on_dismiss = false;
      std::vector<struct mb3dsoundings_sounding_struct>().swap(sdg_storage);
      spin(300);
      dismissed = mbvq::view_stats(sview);
      next_view = open_area(19, &inside_next);
      next_open = wait_stats();
    }
    const size_t bytes = 12 * static_cast<size_t>(count);
    const int next_reads = next_open.sounding_reads[0] - dismissed.sounding_reads[0];
    const size_t next_bytes = next_open.sounding_bytes[0] - dismissed.sounding_bytes[0];
    check(next_view != nullptr && next_view == sview && first_open.soundings_drawn == count &&
              next_open.soundings_drawn == count && inside_first > 0 && inside_next > 0 &&
              first_open.sounding_reads[0] == 1 && first_open.sounding_bytes[0] == bytes && next_reads == 1 &&
              next_bytes == bytes,
          "6d: the soundings window opened by mb3dsoundings_open() paints inside the call (" +
              std::to_string(inside_first) + " and " + std::to_string(inside_next) + " frames) and reads the " +
              std::to_string(count) + " positions once: " + std::to_string(first_open.sounding_reads[0]) +
              " read, " + std::to_string(first_open.sounding_bytes[0]) + " bytes written at the first open, " +
              std::to_string(next_reads) + " read, " + std::to_string(next_bytes) + " bytes at the open after "
              "Dismiss (" + std::to_string(bytes) + " bytes each)");
    int error = MB_ERROR_NO_ERROR;
    mb3dsoundings_end(0, &error);
    sdg_data.num_soundings = 0;
    sdg_data.soundings = nullptr;
    std::vector<struct mb3dsoundings_sounding_struct>().swap(sdg_storage);
  }

  /* 6d: the program's end paths (Quit, View All, Generate New Grid, the map
     dismissed) call mb3dsoundings_end(), which deletes the soundings view at
     once and its hidden window only at a safe point (the next soundings, or
     Quit): the view lets go of its buffers when it is deleted, since that
     window draws no frame that would. A million soundings in the standalone
     soundings window, as mbeditviz-qt has it. On Metal the device's
     allocations right before and right after the call, without an event
     between, after three more frames of the view (at least two required,
     Metal's two frame slots), so that the drop is the view's own: without
     them the call's finish() also frees what the window's earlier frames
     left. A Metal device that cannot be read fails the check. On the other
     APIs nothing is measured (the view, and with it its renderer's count,
     goes in the call). */
  {
    make_sdg(2500, 400, 23);
    const bool opened = open_sdg();
    const QPointer<QWidget> swindow = mbvq::soundings_window();
    QWidget *sview = opened ? mbvq::soundings_widget()->view() : nullptr;
    auto *sview_rhi = qobject_cast<QRhiWidget *>(sview);
    const int count = static_cast<int>(sdg_storage.size());
    int drawn_frames = 0;
    if (swindow != nullptr && sview_rhi != nullptr) {
      sdg_camera(90.0f, 0.0f, 1.0f, 0.0f, 0.0f);
      sdg_view(MBS_VIEW_COLOR_FLAG, true, MBS_VIEW_PROFILES_NONE, false);
      mbvq_c_soundings_plot();
      timer.restart();
      while ((mbvq::view_stats(sview).soundings_drawn != count || mbvq::view_stats(sview).soundings_waiting) &&
             timer.elapsed() < 20000)
        spin(20);
      spin(300);
      drawn_frames = mbvq::view_stats(sview).frames;
      for (int i = 0; i < 3; i++) {
        sview->update();
        spin(150);
      }
    }
    const mbvq::RenderStats shown = sview_rhi != nullptr ? mbvq::view_stats(sview) : mbvq::RenderStats();
    /* the view goes in the call */
    const bool metal = sview_rhi != nullptr && sview_rhi->api() == QRhiWidget::Api::Metal;
    const double gpu_before = metal_allocated_mb();
    int error = MB_ERROR_NO_ERROR;
    mb3dsoundings_end(0, &error);
    const double gpu_after = metal_allocated_mb();
    /* the program releases its soundings after the call, as mbeditviz does */
    sdg_data.num_soundings = 0;
    sdg_data.soundings = nullptr;
    std::vector<struct mb3dsoundings_sounding_struct>().swap(sdg_storage);
    const bool waits = swindow != nullptr && !swindow->isVisible() && mbvq::soundings_widget() == nullptr;
    const double held = shown.sounding_held_bytes / 1048576.0;
    const int frames = shown.frames - drawn_frames;
    const bool measured = metal && gpu_before >= 0.0 && gpu_after >= 0.0;
    check(sview_rhi != nullptr && shown.soundings_drawn == count && held > 0.0 && waits &&
              (!metal || (measured && frames >= 2 && gpu_before - gpu_after >= 0.75 * held)),
          "6d: mb3dsoundings_end() (the program's end paths) deletes the view of " + std::to_string(count) +
              " soundings (" + text("%.1f", held) + " MB) of the standalone soundings window, which waits hidden for "
              "a safe point, and the view lets go of its buffers in that call: " +
              (!metal     ? std::string("not measured (the GPU memory is measured on Metal only)")
               : measured ? "the Metal device holds " + text("%.1f", gpu_before - gpu_after) + " MB less (after " +
                                std::to_string(frames) + " more frames)"
                          : std::string("the Metal device's allocations could not be read")));
  }

  /* 6d: mbview_destroy() (a map's Dismiss, the programs' Quit, a regrid in
     mbeditviz-qt) deletes the map view at once and its hidden standalone
     window only at a safe point (the next map, or Quit): the view lets go of
     its renderer's buffers when it is deleted, since that window draws no
     frame that would. A second map of the grid in a window of its own (the
     first is needed below), drawn completely. On Metal the device's
     allocations right before and right after the call, without an event
     between, after three more frames of the view (at least two required,
     Metal's two frame slots), so that the drop is the view's own: they must
     drop by at least 3/4 of the bytes its renderer held (the terrain's tile
     levels, the batch and the contours, every copy counted). A Metal device
     that cannot be read fails the check. On the other APIs nothing is
     measured (the view, and with it its renderer's count, goes in the
     call). */
  {
    size_t second = MBV_NO_WINDOW;
    const bool second_opened = open_grid(grid, grdfile, &second);
    const QPointer<QWidget> mwindow = second_opened ? mbvq::window(second) : nullptr;
    const QPointer<QWidget> mview = second_opened ? mbvq::map_view(second) : nullptr;
    auto *mview_rhi = qobject_cast<QRhiWidget *>(mview.data());
    int drawn_frames = 0;
    if (mwindow != nullptr && mview_rhi != nullptr) {
      timer.restart();
      while ((!mbvq::view_state(mview).drawn || mbvq_c_lastdrawrez(second) != MBVQ_REZ_FULL ||
              mbvq::view_stats(mview).tiles_drawn == 0 || mbvq::view_stats(mview).tiles_waiting > 0) &&
             timer.elapsed() < 20000)
        spin(20);
      spin(300);
      drawn_frames = mbvq::view_stats(mview).frames;
      for (int i = 0; i < 3; i++) {
        mview->update();
        spin(150);
      }
    }
    const mbvq::RenderStats shown = mview_rhi != nullptr ? mbvq::view_stats(mview) : mbvq::RenderStats();
    /* the view goes in the call */
    const bool metal = mview_rhi != nullptr && mview_rhi->api() == QRhiWidget::Api::Metal;
    const double gpu_before = metal_allocated_mb();
    int error = MB_ERROR_NO_ERROR;
    if (second_opened)
      mbview_destroy(0, second, true, &error);
    const double gpu_after = metal_allocated_mb();
    const bool waits = mwindow != nullptr && !mwindow->isVisible() && mview == nullptr &&
                       mbvq::map_widget(second) == nullptr;
    const double terrain = shown.resident_bytes / 1048576.0;
    const double held = (shown.resident_bytes + shown.batch_held_bytes + shown.contour_held_bytes) / 1048576.0;
    const int frames = shown.frames - drawn_frames;
    const bool measured = metal && gpu_before >= 0.0 && gpu_after >= 0.0;
    check(mview_rhi != nullptr && shown.tiles_drawn > 0 && shown.tiles_waiting == 0 && terrain > 0.0 && waits &&
              (!metal || (measured && frames >= 2 && gpu_before - gpu_after >= 0.75 * held)),
          "6d: mbview_destroy() deletes the view of a map drawn completely (" + std::to_string(shown.tiles_drawn) +
              " tiles; " + text("%.1f", held) + " MB, the terrain " + text("%.1f", terrain) + " MB) of a "
              "standalone map window, which waits hidden for a safe point, and the view lets go of its buffers in "
              "that call: " +
              (!metal     ? std::string("not measured (the GPU memory is measured on Metal only)")
               : measured ? "the Metal device holds " + text("%.1f", gpu_before - gpu_after) + " MB less (after " +
                                std::to_string(frames) + " more frames)"
                          : std::string("the Metal device's allocations could not be read")));
  }

  /* the failure label: shown on renderFailed, hidden again by a working initialize() */
  emit rhi_widget->renderFailed();
  const bool shown = mbvq::view_state(view).failure_shown;
  QWidget *top = mbvq::window(instance);
  top->resize(top->width() + 40, top->height() + 30);
  timer.restart();
  while (mbvq::view_state(view).failure_shown && timer.elapsed() < 5000)
    spin(20);
  check(shown && !mbvq::view_state(view).failure_shown && mbvq::view_state(view).renderer,
        "renderFailed shows the label; the next working initialize() hides it and makes the renderer again");

  close_instance(instance);
  check(mbvq::window(instance) == nullptr, "the window closes");
  /* leave the event loop as the programs do: aboutToQuit deletes the closed
     window, before QApplication goes (a QRhiWidget window left for that leaves
     the Vulkan swapchain behind its surface) */
  QTimer::singleShot(0, qApp, &QCoreApplication::quit);
  QApplication::exec();
  return result();
}

/*--------------------------------------------------------------------*/
/* the pixels of a QRhi map view that are not white; grabFramebuffer() draws a
   new frame for it, a fill with its side effects */
int terrain_pixels(size_t instance) {
  auto *view = qobject_cast<QRhiWidget *>(mbvq::map_view(instance));
  if (view == nullptr)
    return -1;
  const QImage image = view->grabFramebuffer();
  int n = 0;
  for (int y = 0; y < image.height(); y++)
    for (int x = 0; x < image.width(); x++)
      n += (image.pixel(x, y) & 0xffffffu) != 0xffffffu;
  return n;
}

/* A program with a QRhi map, its profile window and the 3D soundings in a
   floating dock of its main window open ends (--quit). With maps_only (the
   shape of mbgrdviz-qt): the map and its profile window alone, without
   soundings, and a main window only for "float" (its map in a dock). */
int test_quit(const char *how, const char *grdfile, bool maps_only) {
  const bool floating = std::strcmp(how, "float") == 0;
  /* the program's main window: the soundings in a dock of it (6d), floated
     below; with "float" the map too */
  QMainWindow *host = nullptr;
  if (!maps_only || floating) {
    host = new QMainWindow;
    host->setCentralWidget(new QLabel(QStringLiteral("the program")));
    host->resize(1200, 800);
    host->show();
  }
  QDockWidget *dock = nullptr;
  QDockWidget *soundings_dock = nullptr;
  if (!maps_only)
    mbvq::set_soundings_placement([host, &soundings_dock](mbvq::SoundingsWidget *widget) {
      soundings_dock = new QDockWidget(widget->title());
      soundings_dock->setWidget(widget);
      host->addDockWidget(Qt::RightDockWidgetArea, soundings_dock);
      soundings_dock->show();
    });
  if (floating) {
    mbvq::set_map_placement([host, &dock](mbvq::MapWidget *map) {
      dock = new QDockWidget(map->title());
      dock->setWidget(map);
      host->addDockWidget(Qt::LeftDockWidgetArea, dock);
      dock->show();
    });
  }
  Grid grid;
  size_t instance = MBV_NO_WINDOW;
  check(open_grid(grid, grdfile, &instance), "mbview_open under QRhi");
  mbvq::MapWidget *map = mbvq::map_widget(instance);
  if (map == nullptr)
    return result();
  /* its profile window too (a QRhi view since 6d) */
  map->profile()->open();
  check(wait_full(instance, 20000) && terrain_pixels(instance) > 0,
        std::string("the map drawn (window exposed: ") + (exposed(mbvq::map_view(instance)) ? "1" : "0") + ")");
  print_create_time(mbvq::map_view(instance), "the map view, first frame");
  /* the profile window draws on its own: its first frame waited for (up to
     10 s), as --display does, so that the time printed is its renderer's */
  QWidget *profile_view = map->profile()->view();
  QElapsedTimer profile_timer;
  profile_timer.start();
  while (profile_view != nullptr && mbvq::view_stats(profile_view).frames == 0 && profile_timer.elapsed() < 10000)
    spin(20);
  print_create_time(profile_view, "the profile view, first frame");
  if (floating) {
    dock->setFloating(true);
    spin(1500);
    mbvq_c_resetview(instance);
    spin(500);
    check(dock->isFloating() && terrain_pixels(instance) > 0 && mbvq::view_state(mbvq::map_view(instance)).renderer,
          "the map's dock floats: drawn with the QRhi of the floating window");
    print_create_time(mbvq::map_view(instance), "the map view, its dock floated");
  }
  /* 6d: the soundings, drawn in their dock, then floated */
  if (!maps_only) {
    make_sdg(60, 40, 23);
    open_sdg();
    QWidget *sview = mbvq::soundings_widget() != nullptr ? mbvq::soundings_widget()->view() : nullptr;
    QElapsedTimer timer;
    timer.start();
    while (sview != nullptr && !mbvq::view_state(sview).drawn && timer.elapsed() < 10000)
      spin(20);
    const bool docked_drawn = sview != nullptr && mbvq::view_state(sview).drawn;
    print_create_time(sview, "the soundings view, first frame in its dock");
    if (soundings_dock != nullptr)
      soundings_dock->setFloating(true);
    spin(1200);
    const int frames = sview != nullptr ? mbvq::view_stats(sview).frames : 0;
    mbvq_c_soundings_plot();
    /* the frame comes when the window is next composed: waited for, up to
       5 s, rather than a fixed time */
    timer.restart();
    while (sview != nullptr && mbvq::view_stats(sview).frames == frames && timer.elapsed() < 5000)
      spin(20);
    check(docked_drawn && soundings_dock != nullptr && soundings_dock->isFloating() &&
              mbvq::view_stats(sview).frames > frames && mbvq::view_state(sview).renderer,
          "6d: the soundings drawn in their dock, then floated and drawn with the QRhi of the floating window");
    print_create_time(sview, "the soundings view, its dock floated");
  }

  bool about_to_quit = false;
  QObject::connect(qApp, &QCoreApplication::aboutToQuit, qApp, [&about_to_quit] { about_to_quit = true; });
  QTimer::singleShot(200, qApp, [how, instance] {
    if (std::strcmp(how, "close") == 0) {
      /* the map window's close button, then the program leaves its event loop */
      mbvq::window(instance)->close();
      QApplication::quit();
    }
    else if (std::strcmp(how, "exit") == 0) {
      /* the macOS Quit menu of the programs: exit() inside the event loop,
         with the soundings (unless maps_only) still floating */
      mbvq::window(instance)->close();
      check(mbvq::window(instance) == nullptr, "the map window closes; exit() inside the event loop");
      std::exit(result());
    }
    else {
      /* Cmd-Q: the event loop is left with the map open */
      QApplication::quit();
    }
  });
  const int status = QApplication::exec();
  check(status == 0 && about_to_quit, "the event loop ends (aboutToQuit)");
  if (std::strcmp(how, "close") == 0)
    check(mbvq::window(instance) == nullptr, "the closed map window is gone (deleted at aboutToQuit)");
  /* without a main window the program returns (its windows go with the
     QApplication) */
  if (host == nullptr)
    return result();
  /* The program ends its soundings and maps, then its main window goes,
     which held the soundings (and with "float" the map) before they floated
     (QTBUG-151834): the views removed QRhiWidget's callbacks from its QRhi. */
  const int removed = mbvq::stale_callbacks_removed();
  int error = MB_ERROR_NO_ERROR;
  if (!maps_only)
    mb3dsoundings_end(0, &error);
  spin(100);
  mbview_quit(0, &error);
  spin(100);
  delete host;
  spin(100);
  const int expected = (floating ? 1 : 0) + (maps_only ? 0 : 1);
  const std::string floated = floating && !maps_only ? "map's and soundings' callbacks"
                              : floating             ? "map's callback"
                                                     : "soundings' callback";
  check(mbvq::stale_callbacks_removed() == removed + expected,
        "the floated " + floated +
            " in the main window's QRhi were removed when their views went, before the main window (QTBUG-151834)");
  return result();
}

/*--------------------------------------------------------------------*/
/* --bench: the frame times of a map in a window, driven through the view's
   input as a user drives it, with the backend MB_RENDERER names (QRhi with
   the platform's API when it is unset, as in the other modes with a window;
   legacy-gl for the OpenGL backend), so that both backends are measured by
   one script. A drag is a button pressed at the centre of the view, mouse
   motions along a closed path, and the release; after each motion the bench
   waits for the frame that shows it (under QRhi also for the uploads it
   started). Output: "info" lines, and for each measurement a line "bench
   scenario metric n median p95 max" and a line "values scenario metric"
   with every value, times in ms, tab-separated. */

using BenchClock = std::chrono::steady_clock;
BenchClock::time_point bench_start;

/* ms since the bench began */
double bench_ms() { return std::chrono::duration<double, std::milli>(BenchClock::now() - bench_start).count(); }

/* a time of the renderer's statistics (ms since the epoch of the same steady
   clock) in ms since the bench began */
double bench_ms_of(double steady_ms) {
  return steady_ms - std::chrono::duration<double, std::milli>(bench_start.time_since_epoch()).count();
}

/* The application of this test: QApplication, and after each event it
   delivered the bench's hook, which sees the end of a paint of the map view
   (an event filter sees only its beginning). */
class TestApplication : public QApplication {
 public:
  TestApplication(int &argc, char **argv) : QApplication(argc, argv) {}
  bool notify(QObject *receiver, QEvent *event) override {
    const QEvent::Type type = event->type();
    const bool result = QApplication::notify(receiver, event);
    if (delivered)
      delivered(receiver, type);
    return result;
  }
  /* the receiver (gone, possibly: only compared) and the type of the event */
  std::function<void(const QObject *, QEvent::Type)> delivered;
};

void set_delivered(std::function<void(const QObject *, QEvent::Type)> hook) {
  static_cast<TestApplication *>(qApp)->delivered = std::move(hook);
}

/* this process's resident size, its peak and its physical footprint (macOS)
   in MB; -1 where not known */
void process_memory(double *resident, double *peak, double *footprint) {
  *resident = *peak = *footprint = -1.0;
#if defined(__APPLE__)
  task_vm_info_data_t info;
  mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
  if (task_info(mach_task_self(), TASK_VM_INFO, reinterpret_cast<task_info_t>(&info), &count) == KERN_SUCCESS) {
    *resident = static_cast<double>(info.resident_size) / 1048576.0;
    *peak = static_cast<double>(info.resident_size_peak) / 1048576.0;
    *footprint = static_cast<double>(info.phys_footprint) / 1048576.0;
  }
#elif defined(__linux__)
  FILE *status = std::fopen("/proc/self/status", "r");
  if (status != nullptr) {
    char line[256];
    while (std::fgets(line, sizeof(line), status) != nullptr) {
      double kb = 0.0;
      if (std::sscanf(line, "VmRSS: %lf", &kb) == 1)
        *resident = kb / 1024.0;
      else if (std::sscanf(line, "VmHWM: %lf", &kb) == 1)
        *peak = kb / 1024.0;
    }
    std::fclose(status);
  }
#endif
}

void report_memory(const char *when) {
  double resident = 0.0, peak = 0.0, footprint = 0.0;
  process_memory(&resident, &peak, &footprint);
  std::printf("info\tmemory %s\tresident %.1f MB, peak %.1f MB, footprint %.1f MB\n", when, resident, peak, footprint);
}

/* a frame of the map view, seen from outside the backend */
struct BenchFrame {
  double paint = 0.0;      /* the view's paint event began (ms since the bench began) */
  double painted = -1.0;   /* ... and ended */
  double compose = -1.0;   /* OpenGL: the window began to compose (aboutToCompose) */
  double submitted = 0.0;  /* the window submitted the frame (frameSubmitted, frameSwapped) */
  int rez = MBVQ_REZ_NONE; /* the core's resolution drawn last, after the frame */
  mbvq::RenderStats stats; /* QRhi: its renderer's statistics after the frame */
};

/* connects a signal without arguments, named by its signature, to a function,
   through the timeout() signal of a timer that never runs: for a view whose
   class header the test does not include (QOpenGLWidget) */
bool relay(QObject *sender, const char *signature, QTimer *timer, std::function<void()> function) {
  const int index = sender->metaObject()->indexOfSignal(signature);
  if (index < 0 ||
      !QObject::connect(sender, sender->metaObject()->method(index), timer, QMetaMethod::fromSignal(&QTimer::timeout)))
    return false;
  QObject::connect(timer, &QTimer::timeout, timer, std::move(function));
  return true;
}

/* The frames of the map view of an instance: a paint event of the view
   begins one, the next submission of its window ends it
   (QRhiWidget::frameSubmitted, QOpenGLWidget::frameSwapped). The probe filters
   the application's events from before the map opens, and connects to the
   view's signals at its first event (a show, before any paint), so that no
   frame of the view is missed. */
class FrameProbe : public QObject {
 public:
  explicit FrameProbe(const size_t *instance) : instance_(instance) {}
  bool eventFilter(QObject *object, QEvent *event) override {
    if ((view_ == nullptr || event->type() == QEvent::Paint) && *instance_ < MBVQ_MAX_WINDOWS &&
        object == mbvq::map_view(*instance_)) {
      if (view_ == nullptr)
        attached = attach(static_cast<QWidget *>(object));
      if (event->type() == QEvent::Paint) {
        /* a paint whose window submitted no frame (before the window is
           exposed): kept with what the renderer did in it */
        if (painting_) {
          frame_.submitted = -1.0;
          frame_.rez = mbvq_c_lastdrawrez(*instance_);
          frame_.stats = mbvq::view_stats(view_);
          unsubmitted.push_back(frame_);
        }
        paints++;
        painting_ = true;
        frame_ = BenchFrame();
        frame_.paint = bench_ms();
      }
    }
    return QObject::eventFilter(object, event);
  }
  /* an event delivered (TestApplication): the end of a paint of the view */
  void delivered(const QObject *receiver, QEvent::Type type) {
    if (type == QEvent::Paint && painting_ && receiver == view_)
      frame_.painted = bench_ms();
  }

  std::vector<BenchFrame> frames;
  std::vector<BenchFrame> unsubmitted; /* paints of the view without a frame submitted */
  bool attached = false;      /* connected to the view's frame signal */
  int paints = 0;             /* paint events of the view */
  int unpainted = 0;          /* submissions without a paint of the view */
  QEventLoop *loop = nullptr; /* left after every frame */

 private:
  bool attach(QWidget *view) {
    view_ = view;
    const bool rhi = qobject_cast<QRhiWidget *>(view) != nullptr;
    if (!rhi)
      relay(view, "aboutToCompose()", &compose_relay_, [this] { composing(); });
    return relay(view, rhi ? "frameSubmitted()" : "frameSwapped()", &submit_relay_, [this] { submitted(); });
  }
  void composing() {
    if (painting_)
      frame_.compose = bench_ms();
  }
  void submitted() {
    if (!painting_) {
      unpainted++;
      return;
    }
    painting_ = false;
    frame_.submitted = bench_ms();
    frame_.rez = mbvq_c_lastdrawrez(*instance_);
    frame_.stats = mbvq::view_stats(view_);
    frames.push_back(frame_);
    if (loop != nullptr)
      loop->quit();
  }

  const size_t *instance_;
  QWidget *view_ = nullptr;
  bool painting_ = false;
  BenchFrame frame_;
  QTimer compose_relay_;
  QTimer submit_relay_;
};

/* runs the event loop until done() holds; false after timeout_ms */
bool wait_for(FrameProbe &probe, const std::function<bool()> &done, int timeout_ms) {
  QElapsedTimer timer;
  timer.start();
  while (!done()) {
    const qint64 left = timeout_ms - timer.elapsed();
    if (left <= 0)
      return false;
    QEventLoop loop;
    QTimer::singleShot(static_cast<int>(left), &loop, &QEventLoop::quit);
    probe.loop = &loop;
    loop.exec();
    probe.loop = nullptr;
  }
  return true;
}

/* a frame that shows the whole map at full resolution */
bool complete(const BenchFrame &frame, bool rhi) {
  return frame.rez == MBVQ_REZ_FULL && (!rhi || frame.stats.tiles_waiting == 0);
}

/* the first frame from index on that shows the whole map at full resolution;
   frames.size() if none */
size_t first_complete(const FrameProbe &probe, size_t index, bool rhi) {
  while (index < probe.frames.size() && !complete(probe.frames[index], rhi))
    index++;
  return index;
}

/* waits until the map shows completely at full resolution and no frame came
   for 300 ms (OpenGL: after the full plot of the background work); false
   after timeout_ms */
bool settle(FrameProbe &probe, bool rhi, int timeout_ms) {
  QElapsedTimer timer;
  timer.start();
  while (timer.elapsed() < timeout_ms) {
    const size_t n = probe.frames.size();
    spin(300);
    if (probe.frames.size() == n && n > 0 && complete(probe.frames.back(), rhi))
      return true;
  }
  return false;
}

/* one line: the median, the 95th percentile (nearest rank: the value of rank
   ceil(0.95 n) in ascending order, the largest of up to 19 values) and the
   maximum; then a line with every value in the order measured */
void report(const char *scenario, const char *metric, std::vector<double> values) {
  if (values.empty()) {
    std::printf("bench\t%s\t%s\t0\t-\t-\t-\n", scenario, metric);
    return;
  }
  std::printf("values\t%s\t%s\t", scenario, metric);
  for (size_t i = 0; i < values.size(); i++)
    std::printf(i == 0 ? "%.3f" : ",%.3f", values[i]);
  std::printf("\n");
  std::sort(values.begin(), values.end());
  const size_t n = values.size();
  const double median = n % 2 == 1 ? values[n / 2] : 0.5 * (values[n / 2 - 1] + values[n / 2]);
  const double p95 = values[(95 * n + 99) / 100 - 1];
  std::printf("bench\t%s\t%s\t%zu\t%.3f\t%.3f\t%.3f\n", scenario, metric, n, median, p95, values.back());
}

/* "stride 1: 3, stride 4: 57" for the strides of frames */
std::string stride_counts(const std::vector<double> &strides) {
  std::map<int, int> counts;
  for (double s : strides)
    counts[static_cast<int>(s)]++;
  std::string out;
  for (const auto &count : counts)
    out += (out.empty() ? "" : ", ") + std::string("stride ") + std::to_string(count.first) + ": " +
           std::to_string(count.second);
  return out.empty() ? "none" : out;
}

/* The terrain's levels in the frames of a scenario (QRhi), one value per
   frame in the order drawn: the stride drawn, that of the zoom and the one
   planned, the tiles in the view and those left out, whether the drag
   budget and the memory budget made the stride coarser (1) or not (0); in
   the frames with a mouse button down the drag budget and the other costs
   of the frame before it came from; the preparing and gathering estimated
   and measured; the GPU memory of the view's levels and of every view's
   (MB, every copy counted) and the levels evicted from the view so far;
   whether the frame passed over a level the GPU-time fallback marked (7c);
   a line with the frames that did and those in which it can act (its
   backend check, gpu_rule; it acts only in the frames of a press);
   and a line with the frames per stride. */
void report_levels(const char *scenario, const FrameProbe &probe, const std::vector<size_t> &indices) {
  std::vector<double> stride, zoom, target, visible, culled, drag, memory, budget, outside, estimated, work, resident,
      cache, evictions, gpu_coarser;
  int over = 0, gpu_rule = 0;
  for (size_t i : indices) {
    const mbvq::RenderStats &s = probe.frames[i].stats;
    stride.push_back(s.stride);
    zoom.push_back(s.stride_zoom);
    target.push_back(s.stride_target);
    visible.push_back(s.tiles_visible);
    culled.push_back(s.tiles_culled);
    drag.push_back(s.drag_budget ? 1.0 : 0.0);
    memory.push_back(s.memory_budget ? 1.0 : 0.0);
    if (s.budget_ms >= 0.0) {
      budget.push_back(s.budget_ms);
      outside.push_back(s.outside_ms);
    }
    estimated.push_back(s.estimated_ms);
    work.push_back(s.work_ms);
    resident.push_back(static_cast<double>(s.resident_bytes) / 1048576.0);
    cache.push_back(static_cast<double>(s.cache_bytes) / 1048576.0);
    evictions.push_back(s.evictions);
    over += s.over_memory_budget ? 1 : 0;
    gpu_coarser.push_back(s.gpu_coarser ? 1.0 : 0.0);
    gpu_rule += s.gpu_rule ? 1 : 0;
  }
  report(scenario, "stride", stride);
  report(scenario, "stride_zoom", zoom);
  report(scenario, "stride_target", target);
  report(scenario, "tiles_visible", visible);
  report(scenario, "tiles_culled", culled);
  report(scenario, "drag_budget", drag);
  report(scenario, "memory_budget", memory);
  report(scenario, "budget_ms", budget);
  report(scenario, "outside_ms", outside);
  report(scenario, "estimated_ms", estimated);
  report(scenario, "work_ms", work);
  report(scenario, "resident_mb", resident);
  report(scenario, "cache_mb", cache);
  report(scenario, "evictions", evictions);
  report(scenario, "gpu_coarser", gpu_coarser);
  std::printf("info\t%s\tGPU-time fallback\ta level passed over in %d, the rule on in %d of %zu frames\n", scenario,
              static_cast<int>(std::count(gpu_coarser.begin(), gpu_coarser.end(), 1.0)), gpu_rule, indices.size());
  std::printf("info\t%s\tstrides\t%s; zoom %s; drag budget in %d, memory budget in %d, above it in %d of %zu frames\n",
              scenario, stride_counts(stride).c_str(), stride_counts(zoom).c_str(),
              static_cast<int>(std::count(drag.begin(), drag.end(), 1.0)),
              static_cast<int>(std::count(memory.begin(), memory.end(), 1.0)), over, indices.size());
}

/* The times of single frames: from the paint to the submission, by the
   resolution drawn; the view's paint event, and from its end to the
   submission (the window's composition with its waits); under OpenGL from
   the paint until the window composes; under QRhi the renderer's phases,
   the parts of the paint before and after render() (QRhiWidget begins and
   ends its offscreen frame there, waiting for the GPU at the end), the GPU
   time, which the next frame reports, and the lattice nodes whose positions
   or colours the frame gathered, with the preparation and gathering per
   node; and (7a) the bytes the frame wrote into the uniform ring and into
   the batch's buffers (those queued for its five buffers: 0 for a batch
   reused or empty; not the contours, the marker meshes or a rewrite of the
   stipple values alone), its draws (draw items, the terrain once whatever
   its tiles) and its uniform slots; and (7c) the paint's wait after
   render() that the GPU-time fallback reads, in the frames it measured. */
void report_frames(const char *scenario, const FrameProbe &probe, const std::vector<size_t> &indices, bool rhi) {
  std::vector<double> frame, by_rez[4], paint, paint_event, after_paint, render, fill, marks, prepare, gather, upload,
      record, gpu, before_render, after_render, beyond_gpu, nodes_k, prepare_ns, gather_ns, ring_mb, batch_mb, draws,
      uniforms, wait_ms;
  for (size_t i : indices) {
    const BenchFrame &f = probe.frames[i];
    frame.push_back(f.submitted - f.paint);
    if (f.rez >= MBVQ_REZ_NONE && f.rez <= MBVQ_REZ_FULL)
      by_rez[f.rez].push_back(f.submitted - f.paint);
    if (f.painted >= f.paint) {
      paint_event.push_back(f.painted - f.paint);
      after_paint.push_back(f.submitted - f.painted);
    }
    if (!rhi) {
      if (f.compose >= 0.0)
        paint.push_back(f.compose - f.paint);
      continue;
    }
    render.push_back(f.stats.render_ms);
    fill.push_back(f.stats.fill_ms);
    marks.push_back(f.stats.marks_ms);
    prepare.push_back(f.stats.prepare_ms);
    gather.push_back(f.stats.gather_ms);
    upload.push_back(f.stats.upload_ms);
    record.push_back(f.stats.record_ms);
    ring_mb.push_back(static_cast<double>(f.stats.ring_bytes) / 1048576.0);
    draws.push_back(f.stats.draws);
    uniforms.push_back(f.stats.uniform_slots);
    /* the paint's wait after render() that the GPU-time fallback reads, in
       the frames it measured (7c) */
    if (i > 0 && f.stats.wait_frames > probe.frames[i - 1].stats.wait_frames)
      wait_ms.push_back(f.stats.wait_ms);
    /* the parts of the paint around render(), if render() ran in it */
    const double begun = bench_ms_of(f.stats.render_begin_ms);
    const double ended = bench_ms_of(f.stats.render_end_ms);
    const bool rendered = f.painted >= f.paint && begun >= f.paint && ended <= f.painted;
    if (rendered) {
      before_render.push_back(begun - f.paint);
      after_render.push_back(f.painted - ended);
    }
    if (i + 1 < probe.frames.size() && probe.frames[i + 1].stats.frames == f.stats.frames + 1 &&
        probe.frames[i + 1].stats.gpu_ms >= 0.0) {
      gpu.push_back(probe.frames[i + 1].stats.gpu_ms);
      /* the wait after render() beyond the GPU's own time for the frame */
      if (rendered)
        beyond_gpu.push_back(f.painted - ended - probe.frames[i + 1].stats.gpu_ms);
    }
    /* the lattice nodes gathered: the bytes uploaded since the renderer's
       frame before, 12 per node for positions and 4 for colours (the view's
       RGBA8 colours); every tile gathered was prepared */
    if (i > 0 && probe.frames[i - 1].stats.frames + 1 == f.stats.frames) {
      const mbvq::RenderStats &before = probe.frames[i - 1].stats;
      /* the bytes queued for the batch's buffers in this frame: their
         running total since the renderer's frame before (7a) */
      if (f.stats.batch_bytes >= before.batch_bytes)
        batch_mb.push_back(static_cast<double>(f.stats.batch_bytes - before.batch_bytes) / 1048576.0);
      double nodes = 0.0;
      if (f.stats.bytes_positions >= before.bytes_positions)
        nodes = static_cast<double>(f.stats.bytes_positions - before.bytes_positions) / 12.0;
      if (f.stats.bytes_colours >= before.bytes_colours)
        nodes = std::max(nodes, static_cast<double>(f.stats.bytes_colours - before.bytes_colours) / 4.0);
      if (nodes > 0.0) {
        nodes_k.push_back(nodes / 1000.0);
        prepare_ns.push_back(1.0e6 * f.stats.prepare_ms / nodes);
        gather_ns.push_back(1.0e6 * f.stats.gather_ms / nodes);
      }
    }
  }
  report(scenario, "frame", frame);
  const char *rez_names[4] = {"frame_none", "frame_low", "frame_high", "frame_full"};
  for (int rez = MBVQ_REZ_NONE; rez <= MBVQ_REZ_FULL; rez++)
    if (!by_rez[rez].empty() && by_rez[rez].size() < frame.size())
      report(scenario, rez_names[rez], by_rez[rez]);
  std::printf("info\t%s\tframes\t%zu: low %zu, high %zu, full %zu\n", scenario, frame.size(), by_rez[MBVQ_REZ_LOW].size(),
              by_rez[MBVQ_REZ_HIGH].size(), by_rez[MBVQ_REZ_FULL].size());
  report(scenario, "paint_event", paint_event);
  report(scenario, "after_paint", after_paint);
  if (!rhi) {
    report(scenario, "paint", paint);
    return;
  }
  /* the fills the 12 ms fallback made at the resolution asked for, in the
     frames measured (a running total of the view) */
  if (!indices.empty()) {
    const size_t first = indices.front() > 0 ? indices.front() - 1 : indices.front();
    std::printf("info\t%s\tfallback_fills\t%d in %zu frames\n", scenario,
                probe.frames[indices.back()].stats.fallback_fills - probe.frames[first].stats.fallback_fills,
                indices.size());
  }
  report(scenario, "render", render);
  report(scenario, "fill", fill);
  report(scenario, "marks", marks);
  report(scenario, "prepare", prepare);
  report(scenario, "gather", gather);
  report(scenario, "upload", upload);
  report(scenario, "record", record);
  report(scenario, "gpu", gpu);
  report(scenario, "before_render", before_render);
  report(scenario, "after_render", after_render);
  report(scenario, "after_render_beyond_gpu", beyond_gpu);
  report(scenario, "nodes_k", nodes_k);
  report(scenario, "prepare_ns", prepare_ns);
  report(scenario, "gather_ns", gather_ns);
  report(scenario, "ring_mb", ring_mb);
  report(scenario, "batch_mb", batch_mb);
  report(scenario, "draws", draws);
  report(scenario, "uniform_slots", uniforms);
  report(scenario, "wait_ms", wait_ms);
  report_levels(scenario, probe, indices);
}

/* A drag in a mouse mode with a button: a circle (pan, orbit) or up and back
   down (zoom, exaggeration, shading), warmup motions, then steps measured,
   then the release and the map complete again. */
struct Drag {
  const char *name;
  int mouse_mode; /* MBV_MOUSE_* */
  Qt::MouseButton button;
  bool circle;
};

void bench_drag(FrameProbe &probe, size_t instance, QWidget *view, bool rhi, const Drag &drag, int warmup, int steps) {
  if (mbvq_c_mouse_mode(instance) != drag.mouse_mode)
    mbvq_c_set_mouse_mode(instance, drag.mouse_mode);
  if (!settle(probe, rhi, 120000))
    std::printf("info\t%s\tthe map did not settle before the drag\n", drag.name);
  const QPointF centre(view->width() / 2.0, view->height() / 2.0);
  const double radius = 0.15 * std::min(view->width(), view->height());
  const double amplitude = 0.25 * view->height();
  const double pi = std::acos(-1.0);
  auto mouse = [view](QEvent::Type type, const QPointF &at, Qt::MouseButton button, Qt::MouseButtons buttons) {
    QMouseEvent event(type, at, view->mapToGlobal(at), button, buttons, Qt::NoModifier);
    QApplication::sendEvent(view, &event);
  };
  mouse(QEvent::MouseButtonPress, centre, drag.button, drag.button);

  /* each step: the motion sent, the core's handling of it, the wait for the
     view's paint, its frame submitted; the interval runs from one step's
     frame to the next one's, the gap from a frame to the next motion */
  std::vector<double> latency, input, wait, interval, gap;
  std::vector<size_t> measured;
  int timeouts = 0;
  size_t step_frames = 0;
  double last = -1.0;
  const int n = warmup + steps;
  for (int k = 1; k <= n; k++) {
    const double angle = 2.0 * pi * k / n;
    const QPointF at = centre + (drag.circle ? QPointF(radius * (std::cos(angle) - 1.0), radius * std::sin(angle))
                                             : QPointF(0.0, -amplitude * std::sin(angle)));
    const size_t before = probe.frames.size();
    const double sent = bench_ms();
    mouse(QEvent::MouseMove, at, Qt::NoButton, drag.button);
    const double handled = bench_ms();
    const bool shown = wait_for(
        probe, [&] { return probe.frames.size() > before && (!rhi || probe.frames.back().stats.tiles_waiting == 0); },
        20000);
    if (!shown) {
      timeouts += k > warmup;
      last = -1.0;
      continue;
    }
    const BenchFrame &frame = probe.frames.back();
    if (k > warmup) {
      latency.push_back(frame.submitted - sent);
      input.push_back(handled - sent);
      wait.push_back(probe.frames[before].paint - handled);
      if (last >= 0.0) {
        interval.push_back(frame.submitted - last);
        gap.push_back(sent - last);
      }
      for (size_t f = before; f < probe.frames.size(); f++)
        measured.push_back(f);
      step_frames += probe.frames.size() - before;
    }
    last = frame.submitted;
  }

  /* the release, and the map complete at full resolution again: OpenGL draws
     at high resolution after a low one, then at full resolution from the
     background work; QRhi needs no frame (it draws at full resolution) */
  const size_t before = probe.frames.size();
  const double released = bench_ms();
  mouse(QEvent::MouseButtonRelease, centre, drag.button, Qt::NoButton);
  const bool settled = settle(probe, rhi, 120000);
  const size_t done = first_complete(probe, before, rhi);

  std::printf("info\t%s\tsteps %d measured after %d, %d timed out, %.2f frames per step; %zu frames after the "
              "release\n",
              drag.name, steps, warmup, timeouts,
              latency.empty() ? 0.0 : static_cast<double>(step_frames) / static_cast<double>(latency.size()),
              probe.frames.size() - before);
  report(drag.name, "latency", latency);
  report(drag.name, "interval", interval);
  report(drag.name, "gap", gap);
  report(drag.name, "input", input);
  report(drag.name, "wait", wait);
  report_frames(drag.name, probe, measured, rhi);
  report(drag.name, "release_frame", probe.frames.size() > before
                                         ? std::vector<double>{probe.frames[before].submitted - released}
                                         : std::vector<double>());
  report(drag.name, "release_complete", settled && done < probe.frames.size()
                                            ? std::vector<double>{probe.frames[done].submitted - released}
                                            : std::vector<double>());
  if (rhi) {
    /* the frames after the release: the view asks for one, drawn at the
       stride of the zoom without the drag budget */
    std::vector<double> strides, budgets, renders;
    for (size_t f = before; f < probe.frames.size(); f++) {
      strides.push_back(probe.frames[f].stats.stride);
      budgets.push_back(probe.frames[f].stats.budget_ms);
      renders.push_back(probe.frames[f].stats.render_ms);
    }
    report(drag.name, "release_stride", strides);
    report(drag.name, "release_render", renders);
    std::printf("info\t%s\trelease\t%zu frames after it: %s; %s, stride of the zoom %d\n", drag.name,
                strides.size(), stride_counts(strides).c_str(),
                budgets.empty()              ? "no frame"
                : budgets.front() < 0.0      ? "the first without the drag budget"
                                             : "the first with the drag budget",
                probe.frames.size() > before ? probe.frames[before].stats.stride_zoom : 0);
  }
}

/* Colour table changes from the View menu, outside a drag: the first frame
   after each, and the map complete at full resolution again (OpenGL: a low
   and a high frame, then the full plot of the background work). */
void bench_colour(FrameProbe &probe, size_t instance, bool rhi, int warmup, int changes) {
  const char *name = "colour";
  if (mbvq_c_mouse_mode(instance) != MBV_MOUSE_MOVE)
    mbvq_c_set_mouse_mode(instance, MBV_MOUSE_MOVE);
  if (!settle(probe, rhi, 120000))
    std::printf("info\t%s\tthe map did not settle before the changes\n", name);
  std::vector<double> first, whole, input;
  std::vector<size_t> measured;
  int timeouts = 0;
  for (int i = 0; i < warmup + changes; i++) {
    const size_t before = probe.frames.size();
    const double sent = bench_ms();
    mbvq_c_view(instance, i % 2 == 0 ? MBVQ_VIEW_COLORTABLE_BRIGHT : MBVQ_VIEW_COLORTABLE_HAXBY, true);
    const double handled = bench_ms();
    const bool shown = wait_for(probe, [&] { return probe.frames.size() > before; }, 20000);
    const bool settled = settle(probe, rhi, 120000);
    if (i < warmup)
      continue;
    const size_t done = first_complete(probe, before, rhi);
    if (!shown || !settled || done >= probe.frames.size()) {
      timeouts++;
      continue;
    }
    input.push_back(handled - sent);
    first.push_back(probe.frames[before].submitted - sent);
    whole.push_back(probe.frames[done].submitted - sent);
    for (size_t f = before; f <= done; f++)
      measured.push_back(f);
  }
  std::printf("info\t%s\tchanges %d measured after %d, %d timed out\n", name, changes, warmup, timeouts);
  report(name, "input", input);
  report(name, "first", first);
  report(name, "complete", whole);
  report_frames(name, probe, measured, rhi);
}

/* The navigation of a file listing .fnv files, one per line (the processed
   navigation of mbnavlist), shown with its swath bounds, as mbgrdviz shows
   it (z minus the sensor depth); the points added */
long add_navigation(size_t instance, const char *list) {
  FILE *files = std::fopen(list, "r");
  if (files == nullptr)
    return 0;
  long total = 0;
  char path[MB_PATH_MAXLINE];
  int number = 0;
  while (std::fgets(path, sizeof(path), files) != nullptr) {
    path[std::strcspn(path, "\r\n")] = '\0';
    FILE *fp = std::fopen(path, "r");
    if (fp == nullptr)
      continue;
    std::vector<double> v[10];
    char line[1024];
    while (std::fgets(line, sizeof(line), fp) != nullptr) {
      int yr, mo, dy, hr, mn;
      double sec, td, lo, la, hd, sp, draft, roll, pitch, heave, pl, pa, sl, sa;
      if (std::sscanf(line, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &yr, &mo, &dy, &hr,
                      &mn, &sec, &td, &lo, &la, &hd, &sp, &draft, &roll, &pitch, &heave, &pl, &pa, &sl, &sa) != 19)
        continue;
      const double values[10] = {td, lo, la, -draft, hd, sp, pl, pa, sl, sa};
      for (int k = 0; k < 10; k++)
        v[k].push_back(values[k]);
    }
    std::fclose(fp);
    const int n = static_cast<int>(v[0].size());
    if (n == 0)
      continue;
    std::vector<unsigned int> zero(static_cast<size_t>(n), 0);
    char name[MB_PATH_MAXLINE];
    char raw[MB_PATH_MAXLINE];
    std::snprintf(name, sizeof(name), "nav%d", number++);
    std::snprintf(raw, sizeof(raw), "%s", path);
    int error = MB_ERROR_NO_ERROR;
    mbview_addnav(0, instance, n, v[0].data(), v[1].data(), v[2].data(), v[3].data(), v[4].data(), v[5].data(),
                  v[6].data(), v[7].data(), v[8].data(), v[9].data(), zero.data(), zero.data(), zero.data(),
                  MBV_COLOR_BLACK, 1, name, 0, raw, raw, 59, true, false, false, false, 1, &error);
    total += n;
  }
  std::fclose(files);
  int error = MB_ERROR_NO_ERROR;
  mbview_enableviewnavs(0, instance, &error);
  struct mbview_struct *data = data_of(instance);
  data->nav_view_mode = MBV_VIEW_ON;
  data->navswathbounds_view_mode = MBV_VIEW_ON;
  return total;
}

/* 7a: a synthetic GIS layer of one polyline of parts of 20 to 100 points
   each (--gis), as a shapefile's lines would be: random walks inside the
   grid's bounds with steps of the larger cell size, so that a step
   typically crosses one or two grid lines. The core keeps both ends of each
   draped segment and a point at each crossing whose two nodes have data,
   so the draped points per part depend on the cell shape and on the nodes
   without data (2.4 to 2.6 per point on em122_ref_clean, auv_topo and
   grid_geo); on em122_ref_clean about 45 % of the walk points lie over
   cells without data, which the core draws at the top of the depth range
   in 3D. From a fixed seed (std::mt19937's numbers taken as they are, so the
   random numbers are the same with every C++ library; the coordinates may
   differ in the last bit with another libm or FMA contraction, and the
   draped points a little: on grid_utm 143,442 on macOS, 143,376 on Ubuntu
   26.04), draped on the grid and drawn as an overlay of the map, each part
   a line strip; the points added (0 if the layer could not be), and in
   *draped the draped points the core keeps for them */
long add_gis(size_t instance, int parts, long *draped) {
  *draped = 0;
  const struct mbview_struct *data = data_of(instance);
  const double xmin = data->primary_xmin, xmax = data->primary_xmax;
  const double ymin = data->primary_ymin, ymax = data->primary_ymax;
  const double step = std::max(data->primary_dx, data->primary_dy);
  std::mt19937 generator(20261118);
  const auto unit = [&generator]() { return generator() / 4294967296.0; };
  std::vector<int> starts;
  std::vector<double> lon, lat;
  for (int part = 0; part < parts; part++) {
    starts.push_back(static_cast<int>(lon.size()));
    const int points = 20 + static_cast<int>(generator() % 81);
    double x = xmin + unit() * (xmax - xmin), y = ymin + unit() * (ymax - ymin);
    double heading = 2.0 * std::acos(-1.0) * unit();
    for (int p = 0; p < points; p++) {
      double plon = 0.0, plat = 0.0;
      mbview_projectgrid2ll(instance, x, y, &plon, &plat);
      lon.push_back(plon);
      lat.push_back(plat);
      heading += 0.5 * (unit() - 0.5);
      x = std::min(xmax, std::max(xmin, x + step * std::cos(heading)));
      y = std::min(ymax, std::max(ymin, y + step * std::sin(heading)));
    }
  }
  int error = MB_ERROR_NO_ERROR;
  int layer = -1, polyline = -1;
  if (mbview_addgislayer(0, instance, MBV_GISLAYER_POLYLINE, false, "bench layer", &layer, &error) != MB_SUCCESS ||
      mbview_addpolyline(0, instance, parts, starts.data(), static_cast<int>(lon.size()), lon.data(), lat.data(),
                         nullptr, MBV_COLOR_BLACK, 1, "bench lines", layer, &polyline, &error) != MB_SUCCESS)
    return 0;
  mbview_enableviewpolylines(0, instance, &error);
  struct mbview_shareddata_struct *shared = nullptr;
  if (mbview_getsharedptr(0, &shared, &error) == MB_SUCCESS && shared != nullptr && polyline >= 0 &&
      polyline < shared->npolyline) {
    const struct mbview_polyline_struct &line = shared->polylines[polyline];
    for (int j = 0; j < line.npoints; j++)
      *draped += line.segments[j].nls;
  }
  return static_cast<long>(lon.size());
}

int bench(bool mode3d, const char *grdfile, int steps, int width, int height, const char *navlist, int gis_parts) {
  const bool rhi = mbvq::backend() == mbvq::Backend::Rhi;
  bench_start = BenchClock::now();
  Grid grid;
  grid.mode3d = mode3d;
  grid.window_width = width;
  grid.window_height = height;
  size_t instance = MBV_NO_WINDOW;
  FrameProbe probe(&instance);
  qApp->installEventFilter(&probe);
  auto *application = static_cast<TestApplication *>(qApp);
  application->delivered = [&probe](const QObject *receiver, QEvent::Type type) { probe.delivered(receiver, type); };

  /* the map opened and drawn completely at full resolution */
  const double opening = bench_ms();
  if (!open_grid(grid, grdfile, &instance)) {
    std::printf("bench: the grid does not open\n");
    return 1;
  }
  const double opened = bench_ms();
  if (navlist != nullptr)
    std::printf("info\tnavigation\t%ld points from %s\n", add_navigation(instance, navlist), navlist);
  QWidget *view = mbvq::map_view(instance);
  if (view == nullptr || !probe.attached) {
    std::printf("bench: no map view, or no frame signal on it\n");
    return 1;
  }
  if (QWidget *top = mbvq::window(instance)) {
    top->raise();
    top->activateWindow();
  }
  const bool settled = settle(probe, rhi, 300000);
  const size_t done = first_complete(probe, 0, rhi);

  struct mbvq_grid nodes;
  mbvq_c_grid(instance, &nodes);
  const double ratio = view->devicePixelRatioF();
  QScreen *screen = view->screen();
  std::printf("info\tbackend\t%s\n",
              rhi ? (std::string("QRhi ") + mbvq::rhi_api_name(mbvq::rhi_api())).c_str() : "OpenGL (legacy)");
  std::printf("info\tqt\t%s\n", qVersion());
  std::printf("info\tgrid\t%s, %d x %d nodes\n", grdfile != nullptr ? grdfile : "synthetic", nodes.n_columns,
              nodes.n_rows);
  std::printf("info\tdisplay\t%s\n", mode3d ? "3d" : "2d");
  std::printf("info\tview\t%d x %d logical px, ratio %.2f, %ld x %ld physical px\n", view->width(), view->height(),
              ratio, std::lround(view->width() * ratio), std::lround(view->height() * ratio));
  std::printf("info\tscreen\t%s, %.1f Hz, ratio %.2f, window exposed %d\n",
              screen != nullptr ? qPrintable(screen->name()) : "none", screen != nullptr ? screen->refreshRate() : 0.0,
              screen != nullptr ? screen->devicePixelRatio() : 0.0, exposed(view) ? 1 : 0);
  std::printf("info\tshade\tmode %d (1 illumination, 2 slope)\n", data_of(instance)->primary_shade_mode);
  std::printf("info\topen\tsettled %d, %d paints, %zu frames, %zu paints without a frame submitted, %d submissions "
              "without a paint\n",
              settled ? 1 : 0, probe.paints, probe.frames.size(), probe.unsubmitted.size(), probe.unpainted);
  /* the paints until the map was complete, in order, with those whose window
     submitted no frame; the renderer's work in them summed (a paint in which
     render() did not run leaves the renderer's frame count as it was) */
  const double completed = done < probe.frames.size() ? probe.frames[done].paint : 1.0e30;
  std::vector<BenchFrame> paints;
  for (const BenchFrame &f : probe.unsubmitted)
    if (f.paint <= completed)
      paints.push_back(f);
  for (size_t f = 0; f < probe.frames.size() && f <= done; f++)
    paints.push_back(probe.frames[f]);
  std::sort(paints.begin(), paints.end(), [](const BenchFrame &a, const BenchFrame &b) { return a.paint < b.paint; });
  double sum[7] = {};
  int rendered = 0;
  for (const BenchFrame &f : paints) {
    std::printf("info\topen paint\tat %.1f ms, submitted %s, resolution %d, renderer frame %d, render %.1f ms, tiles "
                "waiting %d\n",
                f.paint - opening, f.submitted < 0.0 ? "never" : text("%.1f ms", f.submitted - opening).c_str(), f.rez,
                f.stats.frames, f.stats.render_ms, f.stats.tiles_waiting);
    if (f.stats.frames > rendered) {
      const mbvq::RenderStats &s = f.stats;
      const double part[7] = {s.render_ms, s.fill_ms, s.marks_ms, s.prepare_ms, s.gather_ms, s.upload_ms, s.record_ms};
      for (int p = 0; p < 7; p++)
        sum[p] += part[p];
      rendered = f.stats.frames;
    }
  }
  report("open", "open_grid", {opened - opening});
  report("open", "first",
         probe.frames.empty() ? std::vector<double>() : std::vector<double>{probe.frames[0].submitted - opening});
  report("open", "complete",
         done < probe.frames.size() ? std::vector<double>{probe.frames[done].submitted - opening} : std::vector<double>());
  if (rhi)
    std::printf("info\topen sums\trender %.1f, fill %.1f, marks %.1f, prepare %.1f, gather %.1f, upload %.1f, record "
                "%.1f ms in %d renderer frames\n",
                sum[0], sum[1], sum[2], sum[3], sum[4], sum[5], sum[6], rendered);
  report_memory("after opening");
  if (rhi) {
    /* what the GPU holds after the first complete frame: the tile streams
       uploaded so far, and the last frame's drawing */
    const mbvq::RenderStats s = mbvq::view_stats(view);
    std::printf("info\tgpu buffers\tpositions %.1f MB, colours %.1f MB, indices %.1f MB; %d tiles, %d drawn, %zu "
                "triangles\n",
                static_cast<double>(s.bytes_positions) / 1048576.0, static_cast<double>(s.bytes_colours) / 1048576.0,
                static_cast<double>(s.bytes_indices) / 1048576.0, s.tiles, s.tiles_drawn, s.triangles);
  }
  /* the GIS layer (7a), added to the map that is shown, as mbgrdviz adds a
     GIS file, so that the open figures are the map's alone: built, then
     drawn after a plot request (the core's add functions ask for none;
     mbgrdviz's mbview_update() also colours and z-scales the grid again),
     with the times from the request to the first frame and to the first
     complete frame (under the legacy backend the background work's full
     plot), and the memory after it */
  size_t shown = done;
  if (gis_parts > 0) {
    const double building = bench_ms();
    long draped = 0;
    const long points = add_gis(instance, gis_parts, &draped);
    if (points == 0 || draped == 0) {
      std::printf("bench: the GIS layer %s\n", points == 0 ? "could not be made" : "drapes no point");
      return 1;
    }
    std::printf("info\tgis\t%d parts, %ld points in one polyline of a GIS layer, %ld points draped on the grid, built "
                "in %.0f ms\n",
                gis_parts, points, draped, bench_ms() - building);
    report_memory("gis layer made");
    const size_t before = probe.frames.size();
    const double asked = bench_ms();
    mbview_plotlowhigh(instance);
    /* a frame first: until then the open's last frame, which is complete,
       would end settle() */
    const bool drawn =
        wait_for(probe, [&] { return probe.frames.size() > before; }, 20000) && settle(probe, rhi, 300000);
    shown = first_complete(probe, before, rhi);
    std::printf("info\tgis\tfirst frame with the layer %s, first complete frame with it %s after the plot request "
                "(settled %d)\n",
                before < probe.frames.size() ? text("%.1f ms", probe.frames[before].submitted - asked).c_str() : "none",
                shown < probe.frames.size() ? text("%.1f ms", probe.frames[shown].submitted - asked).c_str() : "none",
                drawn ? 1 : 0);
    report_memory("gis layer drawn");
  }
  if (rhi) {
    /* the renderer's create(), and the draws of the first complete frame
       (with a GIS layer, the first with the layer); the view's last frame
       if none was complete */
    const bool complete_frame = shown < probe.frames.size();
    const mbvq::RenderStats s = complete_frame ? probe.frames[shown].stats : mbvq::view_stats(view);
    std::printf("info\trenderer\tcreate() %.2f ms; %s %d draws (%d thin, %d wide, %d markers, %d other: the terrain "
                "once, triangles, points), %d uniform slots\n",
                s.create_ms, complete_frame ? "the first complete frame" : "the last frame (no complete frame)",
                s.draws, s.thin_draws, s.wide_draws, s.marker_draws,
                s.draws - s.thin_draws - s.wide_draws - s.marker_draws, s.uniform_slots);
  }

  const int warmup = 5;
  if (!mode3d) {
    bench_drag(probe, instance, view, rhi, {"pan", MBV_MOUSE_MOVE, Qt::MiddleButton, true}, warmup, steps);
    bench_drag(probe, instance, view, rhi, {"zoom", MBV_MOUSE_MOVE, Qt::RightButton, false}, warmup, steps);
    bench_drag(probe, instance, view, rhi, {"shade", MBV_MOUSE_SHADE, Qt::RightButton, false}, warmup, steps);
  }
  else {
    bench_drag(probe, instance, view, rhi, {"orbit", MBV_MOUSE_ROTATE, Qt::MiddleButton, true}, warmup, steps);
    bench_drag(probe, instance, view, rhi, {"exaggerate", MBV_MOUSE_ROTATE, Qt::RightButton, false}, warmup, steps);
  }
  bench_colour(probe, instance, rhi, 1, 8);
  std::printf("info\tprobe\t%d paints, %zu frames, %d submissions without a paint\n", probe.paints, probe.frames.size(),
              probe.unpainted);
  report_memory("at the end");

  qApp->removeEventFilter(&probe);
  application->delivered = nullptr;
  close_instance(instance);
  QTimer::singleShot(0, qApp, &QCoreApplication::quit);
  QApplication::exec();
  return 0;
}


/*--------------------------------------------------------------------*/
/* --bench-soundings n: the frame times of the 3D soundings view with n
   synthetic soundings (pings of 400 beams) in its window, driven through the
   view's input as a user drives it, with the backend MB_RENDERER names (QRhi
   with the platform's API when it is unset; legacy-gl for the OpenGL
   backend), so that both are measured by one script: a rotation (the middle
   button), an erase drag (the left button in the Erase mode: a colour mark
   per motion), an exaggeration drag (the right button: a position mark per
   motion), bias changes (the roll bias, as a slider release: colour and
   position marks; without a program's bias notify, so the view's own costs
   only), a rotation with all profiles shown, and last an erase drag with
   all profiles shown (along other pings than the first erase drag, so that
   it erases soundings too, with the default 60 steps and 100,000 soundings
   or more; 7a). The soundings window's point size, the files the pings are
   split into and the tile order of the soundings (only the rotation and the
   exaggeration drag then: tile order breaks the profile pairs and the erase
   drag's search by ping and beam, and the bias changes are left out too) as
   the options ask (7a). Each step is a motion (or
   a bias change) sent, then the wait for the view's next frame (latency),
   and under QRhi also for the frames that complete the writes it started
   (complete); per step the renderer's phases of its first frame and,
   summed over all its frames, step_render to step_record (7a), the GPU time
   of a step's last frame (which the first frame of the next step reports),
   the writes and the malloc heap and footprint; the GPU-time fallback's
   wait and total of a step's first frame, where it measured that frame,
   and the level of detail it drew, per scenario the frames measured and
   those drawn at a coarser level, and the level and the soundings drawn in
   the frame after a release (7c). Output as --bench: "bench scenario metric
   n median p95 max" and "values" lines. A measurement, not a test. */
int bench_soundings(int count, int steps, double point_size, int files, bool tiles) {
  const bool rhi = mbvq::backend() == mbvq::Backend::Rhi;
  std::printf("info\tbackend\t%s\n", rhi ? "QRhi" : "OpenGL");
  const int beams = 400;
  const int pings = std::max(1, count / beams);
  if (files > pings) {
    std::fprintf(stderr, "--files %d: more files than the %d pings\n", files, pings);
    return 2;
  }
  if (point_size > 0.0)
    mbvq::set_soundings_point_size(point_size);
  QElapsedTimer opening;
  opening.start();
  make_sdg(pings, beams, 23);
  /* the split into files and the sort are left out of the time to the
     first complete frame, so that every number of files and order compare;
     the memory lines are not corrected for them: their freed vectors (8 and
     12 bytes per sounding) stay resident on macOS and serve later
     allocations, so memory compares only within one setting of --files and
     --order */
  qint64 sort_ms = 0;
  if (files > 1 || tiles) {
    QElapsedTimer sorting;
    sorting.start();
    split_sdg_files(pings, beams, files);
    if (tiles)
      sdg_tile_order(&sdg_storage);
    sdg_data.soundings = sdg_storage.data();
    sort_ms = sorting.elapsed();
  }
  /* what ran of the split and the sort, with its time */
  std::string prepared;
  if (files > 1 || tiles)
    prepared = std::string(" (") + (files > 1 && tiles ? "split and sorted" : files > 1 ? "split" : "sorted") +
               " in " + std::to_string(static_cast<long long>(sort_ms)) + " ms)";
  /* the legacy OpenGL backend draws the core's 3 px, whatever the size */
  std::printf("info\tsoundings\t%d soundings, %d pings of %d beams in %d file%s%s, %s order%s, %s\n",
              static_cast<int>(sdg_storage.size()), pings, beams, files, files > 1 ? "s" : "",
              files > 1 ? " (overlapping in space)" : "", tiles ? "tile" : "ping", prepared.c_str(),
              rhi ? text("point size %.1f logical px", mbvq::soundings_point_size()).c_str() : "the core's point size");
  report_memory("soundings made");
  if (!open_sdg())
    return 1;
  mbvq::SoundingsWidget *widget = mbvq::soundings_widget();
  QWidget *view = widget != nullptr ? widget->view() : nullptr;
  if (view == nullptr)
    return 1;
  /* the view's frames: QRhiWidget::frameSubmitted, QOpenGLWidget::frameSwapped */
  int submitted = 0;
  double submitted_at = 0.0;
  QEventLoop *waiting = nullptr;
  QTimer relay_timer;
  relay(view, rhi ? "frameSubmitted()" : "frameSwapped()", &relay_timer, [&] {
    submitted++;
    submitted_at = bench_ms();
    if (waiting != nullptr)
      waiting->quit();
  });
  /* waits for the next frame after the one counted as seen; false after
     timeout_ms */
  const auto next_frame = [&](int seen, int timeout_ms) {
    QElapsedTimer timer;
    timer.start();
    while (submitted == seen && timer.elapsed() < timeout_ms) {
      QEventLoop loop;
      QTimer::singleShot(static_cast<int>(timeout_ms - timer.elapsed()), &loop, &QEventLoop::quit);
      waiting = &loop;
      loop.exec();
      waiting = nullptr;
    }
    return submitted > seen;
  };
  /* waits until QRhi's writes are done (frames the view asks for itself);
     each_frame, if given, sees the statistics after each frame waited for */
  const auto complete_frames = [&](int timeout_ms, const std::function<void(const mbvq::RenderStats &)> &each_frame) {
    QElapsedTimer timer;
    timer.start();
    while (rhi && mbvq::view_stats(view).soundings_waiting && timer.elapsed() < timeout_ms) {
      next_frame(submitted, 1000);
      if (each_frame)
        each_frame(mbvq::view_stats(view));
    }
    return !rhi || !mbvq::view_stats(view).soundings_waiting;
  };
  const auto complete = [&](int timeout_ms) { return complete_frames(timeout_ms, nullptr); };
  /* the first complete frame */
  next_frame(0, 120000);
  complete(120000);
  const mbvq::RenderStats first = mbvq::view_stats(view);
  std::printf("info\topen\t%d soundings, first complete frame %.0f ms after making the soundings began (their split "
              "and sort left out; view %d x %d logical px, ratio %.2f; %d chunks, %d shown)\n",
              sdg_data.num_soundings, static_cast<double>(opening.elapsed() - sort_ms), view->width(), view->height(),
              view->devicePixelRatioF(), first.sounding_chunks, first.soundings_shown);
  if (rhi) {
    std::printf("info\trenderer\tcreate() %.2f ms\n", first.create_ms);
    /* the soundings' size in physical pixels at the view's ratio, as the
       renderer takes it from the control (the core asks for 3 logical px) */
    rhi::Style style;
    style.ratio = view->devicePixelRatioF();
    style.point_size = mbvq::soundings_point_size();
    std::printf("info\tpoint size\t%.1f logical px, %d physical px at ratio %.2f\n", style.point_size, style.point(3.0),
                style.ratio);
  }
  report_memory("opened");
  spin(500);

  /* seen from above, so that the erase drag crosses soundings in every step */
  sdg_camera(90.0f, 0.0f, 1.0f, 0.0f, 0.0f);
  mbvq_c_soundings_plot();
  next_frame(submitted, 30000);
  complete(120000);
  const QPointF centre(view->width() / 2.0, view->height() / 2.0);
  const double pi = std::acos(-1.0);
  auto mouse = [view](QEvent::Type type, const QPointF &at, Qt::MouseButton button, Qt::MouseButtons buttons) {
    QMouseEvent event(type, at, view->mapToGlobal(at), button, buttons, Qt::NoModifier);
    QApplication::sendEvent(view, &event);
  };
  /* the erase drag goes along make_sdg()'s pings across the middle of the
     data (through sdg_ping_row() with several files), from the first to the
     last beam of a ping, the next motions along the pings after it, so that
     its motions erase soundings. With several million soundings the data
     are only a few tens of physical px wide: the new part of the erase
     circle of the last one or two motions along a ping lies outside them,
     and those motions erase nothing (counted as unchanged) */
  const auto ping_end = [&](int ping, int beam) {
    int error = MB_ERROR_NO_ERROR;
    mb3dsoundings_screen_positions(0, &error);
    const double ratio = view->devicePixelRatioF();
    const int height = static_cast<int>(std::lround(view->height() * ratio));
    const struct mb3dsoundings_sounding_struct &sounding =
        sdg_storage[sdg_ping_row(pings, files, ping) * beams + static_cast<size_t>(beam)];
    return QPointF(sounding.winx / ratio, (height - 1 - sounding.winy) / ratio);
  };
  struct Scenario {
    const char *name;
    Qt::MouseButton button; /* Qt::NoButton: bias changes */
    int edit_mode;
    int profiles;
  };
  const Scenario scenarios[] = {{"rotate", Qt::MiddleButton, MBS_EDIT_TOGGLE, MBS_VIEW_PROFILES_NONE},
                                {"erase", Qt::LeftButton, MBS_EDIT_ERASE, MBS_VIEW_PROFILES_NONE},
                                {"exaggerate", Qt::RightButton, MBS_EDIT_TOGGLE, MBS_VIEW_PROFILES_NONE},
                                {"bias", Qt::NoButton, MBS_EDIT_TOGGLE, MBS_VIEW_PROFILES_NONE},
                                {"rotate_profiles", Qt::MiddleButton, MBS_EDIT_TOGGLE, MBS_VIEW_PROFILES_ALL},
                                {"erase_profiles", Qt::LeftButton, MBS_EDIT_ERASE, MBS_VIEW_PROFILES_ALL}};
  /* the end lines: after rotate_profiles, where 6e's five scenarios end, so
     that they compare with 6e's; after erase_profiles again under keys of
     their own (7a); in tile order after its two scenarios, under the keys
     "end tiles" and "after tiles", the profile lines not shown. Each gives
     the GPU bytes of the soundings' buffers (every copy), the swaps and the
     allocations, and (7d1) the chunks' storage use, the chunks bound without
     a draw and those made again (RenderStats::soundings_storage,
     soundings_applied, soundings_rebuilt; totals since the view's renderer
     was made) */
  const auto report_end = [&](const char *key, const char *when) {
    const mbvq::RenderStats last = mbvq::view_stats(view);
    if (rhi)
      std::printf("info\t%s\tGPU bytes of the soundings' buffers %.1f MB (every copy), swaps %d, allocations %d, "
                  "storage %d, applied %d, rebuilt %d, profile lines %s\n",
                  key, static_cast<double>(last.sounding_held_bytes) / 1048576.0, last.sounding_swaps,
                  last.sounding_allocations, last.soundings_storage, last.soundings_applied, last.soundings_rebuilt,
                  tiles ? "not shown" : last.profile_wide ? "wide" : "thin");
    report_memory(when);
  };
  for (const Scenario &scenario : scenarios) {
    /* tile order: the rotation and the exaggeration drag only. The bias
       changes would run (they use neither the profile pairs nor a search by
       ping, and the bench sets no bias notify) but are left out too, so the
       orders are compared on the two drags only */
    if (tiles && (scenario.profiles != MBS_VIEW_PROFILES_NONE ||
                  (scenario.button != Qt::MiddleButton && scenario.button != Qt::RightButton)))
      continue;
    mb3dsoundings.mouse_mode = MBS_MOUSE_ROTATE;
    mb3dsoundings.edit_mode = scenario.edit_mode;
    if (mb3dsoundings.view_profiles != scenario.profiles) {
      mb3dsoundings_setprofiles(scenario.profiles);
      next_frame(submitted, 30000);
      complete(120000);
      spin(300);
    }
    std::vector<double> latency, done, frames, render, fill, prepare, gather, upload, record, gpu, written_mb, heap,
        footprint;
    /* the GPU-time fallback (7c): the wait and the total of each step's
       first frame, where that frame was measured, and the level it drew */
    std::vector<double> wait_ms, frame_ms, level;
    /* the writes per stream too (7a): positions, colours, index lists,
       colours per pair */
    std::vector<double> stream_mb[4];
    /* the phases summed over every renderer frame of a step, its first and
       those that complete its writes, which can span frames (7a); frames
       that came between two readings are counted, not summed. On Metal a
       frame's record also copies earlier writes into the buffers of its
       frame slot (RenderStats::record_ms): the index lists and colours per
       pair, kept once per frame slot, one copy of each byte written in a
       step of one frame and two in a step of several frames, part of them
       the step before's writes; the positions and colours, kept once
       (storage use, 7d1), one copy per write. In the erase drag with
       profiles at ratio 1 that is most of the step's record, not all */
    std::vector<double> step_phase[6];
    int phase_frames = 0, phase_missed = 0;
    int timeouts = 0, unchanged = 0;
    const double radius = 0.2 * std::min(view->width(), view->height());
    const int first_ping = std::max(0, pings / 4);
    /* the erase drag with profiles goes along the pings mirrored about the
       middle ping, so that it erases soundings the erase drag before it did
       not reach (with the default 60 steps and 100,000 soundings or more) */
    const auto erase_ping = [&](int ping) {
      return scenario.profiles == MBS_VIEW_PROFILES_NONE ? ping : pings - 1 - ping;
    };
    QPointF at = scenario.button == Qt::LeftButton ? ping_end(erase_ping(first_ping), 0) : centre;
    const mbvq::RenderStats pressed = mbvq::view_stats(view);
    if (scenario.button != Qt::NoButton)
      mouse(QEvent::MouseButtonPress, at, scenario.button, scenario.button);
    next_frame(submitted, 5000);
    complete(60000);
    for (int k = 0; k < steps; k++) {
      const mbvq::RenderStats before = mbvq::view_stats(view);
      const int seen = submitted;
      const int flagged = sdg_data.num_soundings_flagged;
      const double t0 = bench_ms();
      if (scenario.button == Qt::MiddleButton) {
        const double a = 2.0 * pi * (k + 1) / steps;
        at = centre + QPointF(radius * std::sin(a), radius * (1.0 - std::cos(a)));
      }
      else if (scenario.button == Qt::LeftButton) {
        /* ten motions along a ping, then a ping far enough away that its
           soundings lie apart on the screen (pings closer than a pixel are
           erased together) */
        const int ping = std::min(pings - 1, first_ping + (k + 1) / 10 * std::max(1, pings / 25));
        at = ping_end(erase_ping(ping), std::min(beams - 1, ((k + 1) % 10) * (beams - 1) / 9));
      }
      else if (scenario.button == Qt::RightButton)
        at = centre + QPointF(0.0, (k % 20 < 10 ? 1.0 : -1.0) * 4.0 * (k % 10 + 1));
      if (scenario.button != Qt::NoButton)
        mouse(QEvent::MouseMove, at, Qt::NoButton, scenario.button);
      else
        mbvq_c_soundings_bias(MBVQ_SDG_BIAS_ROLL, (k % 2 == 0 ? 10 : -10) * (k / 2 + 1));
      /* an erase motion over no unflagged sounding changes nothing: the core
         does not replot, so no frame comes */
      if (scenario.button == Qt::LeftButton && sdg_data.num_soundings_flagged == flagged) {
        unchanged++;
        continue;
      }
      if (!next_frame(seen, 3000)) {
        timeouts++;
        continue;
      }
      latency.push_back(submitted_at - t0);
      const mbvq::RenderStats after_first = mbvq::view_stats(view);
      /* the GPU time of the frame before, the last of the step before */
      if (rhi && k > 0 && after_first.gpu_ms >= 0.0)
        gpu.push_back(after_first.gpu_ms);
      double sums[6] = {};
      int counted = before.frames;
      const auto add_phases = [&](const mbvq::RenderStats &s) {
        if (s.frames <= counted)
          return;
        const double phases[6] = {s.render_ms, s.fill_ms, s.prepare_ms, s.gather_ms, s.upload_ms, s.record_ms};
        for (int p = 0; p < 6; p++)
          sums[p] += phases[p];
        phase_frames++;
        phase_missed += s.frames - counted - 1;
        counted = s.frames;
      };
      add_phases(after_first);
      complete_frames(120000, add_phases);
      done.push_back(submitted_at - t0);
      frames.push_back(submitted - seen);
      if (rhi) {
        if (after_first.wait_frames > before.wait_frames) {
          wait_ms.push_back(after_first.wait_ms);
          frame_ms.push_back(after_first.frame_ms);
        }
        level.push_back(after_first.soundings_lod);
        render.push_back(after_first.render_ms);
        fill.push_back(after_first.fill_ms);
        prepare.push_back(after_first.prepare_ms);
        gather.push_back(after_first.gather_ms);
        upload.push_back(after_first.upload_ms);
        record.push_back(after_first.record_ms);
        for (int p = 0; p < 6; p++)
          step_phase[p].push_back(sums[p]);
        /* a step assumes one renderer; a new one would restart the frame
           count and the totals (no run made one) */
        const mbvq::RenderStats after = mbvq::view_stats(view);
        size_t bytes = 0;
        for (int stream = 0; stream < 4; stream++) {
          bytes += after.sounding_bytes[stream] - before.sounding_bytes[stream];
          stream_mb[stream].push_back(
              static_cast<double>(after.sounding_bytes[stream] - before.sounding_bytes[stream]) / 1048576.0);
        }
        written_mb.push_back(static_cast<double>(bytes) / 1048576.0);
      }
      heap.push_back(heap_in_use());
      double resident = 0.0, peak = 0.0, foot = 0.0;
      process_memory(&resident, &peak, &foot);
      footprint.push_back(foot);
    }
    if (scenario.button != Qt::NoButton)
      mouse(QEvent::MouseButtonRelease, at, scenario.button, Qt::NoButton);
    const bool release_frame = next_frame(submitted, 5000);
    /* the level and the soundings drawn in the release's next frame (7c) */
    const mbvq::RenderStats released = mbvq::view_stats(view);
    complete(120000);
    spin(300);
    report(scenario.name, "latency", latency);
    report(scenario.name, "complete", done);
    report(scenario.name, "frames", frames);
    if (rhi) {
      report(scenario.name, "render", render);
      report(scenario.name, "fill", fill);
      report(scenario.name, "prepare", prepare);
      report(scenario.name, "gather", gather);
      report(scenario.name, "upload", upload);
      report(scenario.name, "record", record);
      report(scenario.name, "gpu", gpu);
      report(scenario.name, "written_mb", written_mb);
      const char *const streams[4] = {"written_positions_mb", "written_colours_mb", "written_lists_mb",
                                      "written_pair_colours_mb"};
      for (int stream = 0; stream < 4; stream++)
        report(scenario.name, streams[stream], stream_mb[stream]);
      const char *const phase_names[6] = {"step_render", "step_fill", "step_prepare", "step_gather", "step_upload",
                                          "step_record"};
      for (int p = 0; p < 6; p++)
        report(scenario.name, phase_names[p], step_phase[p]);
      report(scenario.name, "wait_ms", wait_ms);
      report(scenario.name, "frame_ms", frame_ms);
      report(scenario.name, "level", level);
      if (scenario.button != Qt::NoButton) {
        report(scenario.name, "release_lod",
               release_frame ? std::vector<double>{static_cast<double>(released.soundings_lod)} : std::vector<double>());
        report(scenario.name, "release_drawn",
               release_frame ? std::vector<double>{static_cast<double>(released.soundings_drawn)}
                             : std::vector<double>());
      }
      const mbvq::RenderStats last = mbvq::view_stats(view);
      std::printf("info\t%s\tGPU-time fallback\t%d frames measured, %d drawn at a coarser level, the rule %s in the "
                  "last frame\n",
                  scenario.name, last.wait_frames - pressed.wait_frames, last.lod_frames - pressed.lod_frames,
                  last.gpu_rule ? "on" : "off");
    }
    report(scenario.name, "heap_mb", heap);
    report(scenario.name, "footprint_mb", footprint);
    std::printf("info\t%s\ttimeouts\t%d\tunchanged\t%d\n", scenario.name, timeouts, unchanged);
    if (rhi)
      std::printf("info\t%s\tstep sums\t%d renderer frames summed, %d not seen\n", scenario.name, phase_frames,
                  phase_missed);
    if (std::strcmp(scenario.name, "rotate_profiles") == 0)
      report_end("end", "end");
  }
  if (tiles)
    report_end("end tiles", "after tiles");
  else
    report_end("end erase_profiles", "after erase_profiles");
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
  spin(200);
  return 0;
}
}  // namespace

int main(int argc, char **argv) {
  /* line by line, so that results before a crash are not lost */
  std::setvbuf(stdout, nullptr, _IOLBF, 0);
  const char *mode = argc > 1 ? argv[1] : "";
  const bool quit = std::strcmp(mode, "--quit") == 0;
  const char *how = quit && argc > 2 ? argv[2] : "";
  bool mode3d = false;
  const char *save = nullptr;
  const char *grdfile = nullptr;
  int steps = 60;
  int width = 640;
  int height = 480;
  const char *navlist = nullptr;
  bool maps_only = false;
  bool arguments = true;
  const bool soundings_bench = std::strcmp(mode, "--bench-soundings") == 0;
  /* the options of the benches (7a): recognised in every mode; in their
     bench neither the option nor its value is taken for the grid; outside
     it, or without a valid value, they are refused (the usage line, exit 2) */
  double point_size = 0.0;
  int files = 1;
  bool tiles = false;
  int gis_parts = 0;
  for (int i = quit || soundings_bench ? 3 : 2; i < argc; i++) {
    if (std::strcmp(argv[i], "--3d") == 0)
      mode3d = true;
    else if (quit && std::strcmp(argv[i], "--maps-only") == 0)
      maps_only = true;
    else if (std::strcmp(argv[i], "--save") == 0 && i + 1 < argc)
      save = argv[++i];
    else if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
      if (std::sscanf(argv[++i], "%d", &steps) != 1 || steps < 2)
        arguments = false;
    }
    else if (std::strcmp(argv[i], "--nav") == 0 && i + 1 < argc)
      navlist = argv[++i];
    else if (std::strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
      if (std::sscanf(argv[++i], "%dx%d", &width, &height) != 2 || width < 64 || height < 64)
        arguments = false;
    }
    else if (std::strcmp(argv[i], "--point-size") == 0) {
      /* as the soundings window's control allows: 0.5 to 8 in steps of 0.5 */
      char rest = 0;
      if (!soundings_bench || i + 1 >= argc || std::sscanf(argv[++i], "%lf%c", &point_size, &rest) != 1 ||
          !(point_size >= 0.5 && point_size <= 8.0) || std::floor(2.0 * point_size) != 2.0 * point_size)
        arguments = false;
    }
    else if (std::strcmp(argv[i], "--files") == 0) {
      char rest = 0;
      if (!soundings_bench || i + 1 >= argc || std::sscanf(argv[++i], "%d%c", &files, &rest) != 1 || files < 1)
        arguments = false;
    }
    else if (std::strcmp(argv[i], "--order") == 0) {
      /* the last one counts, as with the other options */
      if (!soundings_bench || i + 1 >= argc)
        arguments = false;
      else if (std::strcmp(argv[++i], "tiles") == 0 || std::strcmp(argv[i], "pings") == 0)
        tiles = std::strcmp(argv[i], "tiles") == 0;
      else
        arguments = false;
    }
    else if (std::strcmp(argv[i], "--gis") == 0) {
      /* at most 100,000 parts, about 17 GiB: a part takes about 160 KiB in
         the core (1,296 bytes per point, 424 per allocated draped point), and
         QRhi holds 12 to 17 KiB more per part than the legacy backend */
      char rest = 0;
      if (std::strcmp(mode, "--bench") != 0 || i + 1 >= argc ||
          std::sscanf(argv[++i], "%d%c", &gis_parts, &rest) != 1 || gis_parts < 1 || gis_parts > 100000)
        arguments = false;
    }
    else
      grdfile = argv[i];
  }
  int sounding_count = 1000000;
  if (soundings_bench && argc > 2 && std::sscanf(argv[2], "%d", &sounding_count) != 1)
    sounding_count = 0;
  const bool bench_mode = std::strcmp(mode, "--bench") == 0 || soundings_bench;
  const bool display = std::strcmp(mode, "--display") == 0 || quit || bench_mode;
  const bool known = arguments &&
                     (std::strcmp(mode, "--model") == 0 || std::strcmp(mode, "--widgets") == 0 ||
                      std::strcmp(mode, "--render-null") == 0 || std::strcmp(mode, "--render-metal") == 0 ||
                      std::strcmp(mode, "--display") == 0 || bench_mode ||
                      (quit && (std::strcmp(how, "close") == 0 || std::strcmp(how, "quit") == 0 ||
                                std::strcmp(how, "exit") == 0 || std::strcmp(how, "float") == 0)));
  if (!known || (soundings_bench && sounding_count < 1)) {
    std::fprintf(stderr, "usage: mbview_qt_rhi_test --model | --widgets | --render-null | --render-metal [--save file] "
                         "| --display [--3d] [--save file] [grid] | --quit close|quit|exit|float [--maps-only] [grid] "
                         "| --bench [--3d] [--frames n] [--size WxH] [--nav list] [--gis N] [grid] "
                         "| --bench-soundings n [--frames n] [--point-size p] [--files k] [--order pings|tiles]\n");
    return 2;
  }
  /* the bench of the legacy OpenGL backend needs it: a build without it
     would measure QRhi under that name */
  if (bench_mode && !mbvq::backend_built(mbvq::Backend::LegacyGL)) {
    const char *value = std::getenv("MB_RENDERER");
    if (value != nullptr && std::strcmp(value, "legacy-gl") == 0) {
      std::printf("skipped: this build has no legacy OpenGL drawing\n");
      return 77;
    }
  }
  /* 10 s for the headless modes (ctest gives 60 s), a minute with a window,
     a quarter of an hour for the bench */
  start_watchdog(bench_mode ? 900 : display ? 60 : 10, mode);

  /* the headless modes on the offscreen platform, as ctest runs them, unless
     another platform is named */
  if (!display && !qEnvironmentVariableIsSet("QT_QPA_PLATFORM"))
    qputenv("QT_QPA_PLATFORM", "offscreen");

  /* one backend per process, before the QApplication; the modes with a window
     take the QRhi API that MB_RENDERER names, as the programs do, and the
     bench also the OpenGL backend (legacy-gl) */
  mbvq::Backend backend = mbvq::Backend::Rhi;
  mbvq::RhiApi api = mbvq::RhiApi::Default;
  if (display) {
    const char *value = std::getenv("MB_RENDERER");
    mbvq::Backend named = mbvq::Backend::Rhi;
    std::string message;
    mbvq::choose_renderer(value, &named, &api, &message);
    if (!message.empty())
      std::fprintf(stderr, "%s\n", message.c_str());
    if (bench_mode && named == mbvq::Backend::LegacyGL && value != nullptr && std::strcmp(value, "legacy-gl") == 0)
      backend = mbvq::Backend::LegacyGL;
    if (named != mbvq::Backend::Rhi)
      api = mbvq::RhiApi::Default;
  }
  /* nothing above sets the backend (--model checks the initial one) */
  initial_backend = mbvq::backend();
  initial_api = mbvq::rhi_api();
  mbvq::set_backend(backend, api);
  /* the program's own arguments stay away from Qt, whose X11 platform would
     take the one after --display for the name of a display */
  int qt_argc = 1;
  TestApplication app(qt_argc, argv);
  int error = MB_ERROR_NO_ERROR;
  mbview_startup(0, nullptr, nullptr, &error);

  if (std::strcmp(mode, "--model") == 0)
    return test_model();
  if (std::strcmp(mode, "--widgets") == 0)
    return test_widgets();
  if (std::strcmp(mode, "--render-null") == 0)
    return test_render_null();
  if (std::strcmp(mode, "--render-metal") == 0)
    return test_render_metal(save);
  if (quit)
    return test_quit(how, grdfile, maps_only);
  if (soundings_bench)
    return bench_soundings(sounding_count, steps, point_size, files, tiles);
  if (bench_mode)
    return bench(mode3d, grdfile, steps, width, height, navlist, gis_parts);
  return test_display(mode3d, grdfile, save);
}
