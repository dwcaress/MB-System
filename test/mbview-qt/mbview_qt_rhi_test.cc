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
 *        draw on the offscreen platform)
 *   mbview_qt_rhi_test --widgets       the QRhi views without drawing: the API,
 *        the deferred drawing, the failure label and message, input before
 *        the first frame, the mouse buttons after a lost release; returns 77
 *        (skipped) on a platform where a QRhi can be made
 *   mbview_qt_rhi_test --render-null   the renderer with a Null QRhi: its
 *        resources, upload counts, split uploads, a failed create(), tile
 *        buffers that cannot be made, a tile without data, a grid of another
 *        size, the QRhi deleted under it
 *   mbview_qt_rhi_test --render-metal  the renderer with a Metal QRhi:
 *        pixels against the node colours, holes, edits, pick against
 *        picture, incremental against from scratch, determinism, float
 *        colours; returns 77 (skipped) where no Metal QRhi can be made;
 *        --save file keeps the first picture
 *   mbview_qt_rhi_test --display [--3d] [--save file] [grid]   a map window
 *        drawn with QRhi (needs a display, not in ctest): waits for the first
 *        frames, grabs the view, picks with the mouse, shows and hides the
 *        failure label, closes the window
 *   mbview_qt_rhi_test --quit close|quit|exit|float [grid]   a program with a
 *        QRhi map and its profile window open ends (needs a display, not in
 *        ctest): the map window closed as by its close button, then the event
 *        loop left (the closed window is deleted at aboutToQuit); the event
 *        loop left with the map open (QApplication::quit(), as Cmd-Q); the
 *        window closed and exit() called inside the event loop, as the macOS
 *        Quit menu does; or the event loop left with the map in a floating
 *        dock of a main window, which is deleted then (QTBUG-151834)
 *   mbview_qt_rhi_test --bench [--3d] [--frames n] [--size WxH] [grid]
 *        frame times of a map window (needs a display, not in ctest; a
 *        measurement, not a test): the time to the first complete frame, then
 *        drags through the view's input, n measured motions each (60) - in 2D
 *        a pan, a zoom and a shading drag, in 3D an orbit and an exaggeration
 *        drag - and colour table changes; medians and 95th percentiles of the
 *        frames with every value, the parts of a frame before and after the
 *        view's paint, under QRhi with the renderer's phases, the waits
 *        around them, the GPU time and the nodes prepared, and the process
 *        memory. MB_RENDERER=legacy-gl measures the OpenGL
 *        backend with the same script; WxH is the window size given to
 *        mbview (640x480)
 *
 * The headless modes run on the offscreen platform unless QT_QPA_PLATFORM
 * names another; the modes with a window draw with the QRhi API that
 * MB_RENDERER names (the platform's API otherwise), the bench with the
 * OpenGL backend for MB_RENDERER=legacy-gl. The synthetic grid is a
 * seamount on a sloping plain, known in closed form, with a block of nodes
 * without data and some single ones.
 */

#include <QApplication>
#include <QDockWidget>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QImage>
#include <QLabel>
#include <QMainWindow>
#include <QMetaMethod>
#include <QMouseEvent>
#include <QRhiWidget>
#include <QScreen>
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
#include <map>
#include <string>
#include <thread>
#include <vector>
#if defined(__APPLE__)
#include <mach/mach.h>
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
#include "mbvq_rhi_model.h"
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

Planned plan_frame(rhi::MapFrame &frame, rhi::TerrainPlan &plan, size_t budget = rhi::kUploadBudget) {
  Planned planned;
  frame.fill();
  rhi::FrameInput input;
  frame.input(&input, true);
  planned.more = plan.plan(input.terrain, budget, &planned.uploads, &planned.stats);
  return planned;
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

    std::string message;
    mbvq::choose_renderer(nullptr, &backend, &api, &message);
    bool chosen = backend == Backend::LegacyGL && api == RhiApi::Default && message.empty();
    mbvq::choose_renderer("", &backend, &api, &message);
    chosen = chosen && backend == Backend::LegacyGL && message.empty();
    check(chosen, "MB_RENDERER unset or empty: the OpenGL backend, without a message");
    mbvq::choose_renderer("auto", &backend, &api, &message);
    check(backend == Backend::Rhi && api == RhiApi::Default && message.empty(), "MB_RENDERER=auto: QRhi, the platform's API");
    mbvq::choose_renderer("legacy-gl", &backend, &api, &message);
    chosen = backend == Backend::LegacyGL && message.empty();
    mbvq::choose_renderer("null", &backend, &api, &message);
    check(chosen && backend == Backend::Null && message.empty(), "MB_RENDERER=legacy-gl and null: those backends");
    mbvq::choose_renderer("nonsense", &backend, &api, &message);
    check(backend == Backend::LegacyGL && message.find("not one of auto, metal") != std::string::npos,
          "an unknown MB_RENDERER: the OpenGL backend and a line naming the values");
    const char *impossible = apple ? "d3d11" : "metal";
    mbvq::choose_renderer(impossible, &backend, &api, &message);
    check(backend == Backend::Rhi && api == RhiApi::Default &&
              message.find("not possible on this platform") != std::string::npos &&
              message.find(apple ? "metal" : "opengl") != std::string::npos,
          std::string("MB_RENDERER=") + impossible + " here: QRhi with the platform's API, and one line (" + message + ")");
    mbvq::choose_renderer(apple ? "metal" : "opengl", &backend, &api, &message);
    check(backend == Backend::Rhi && api == (apple ? RhiApi::Metal : RhiApi::OpenGL) && message.empty(),
          "a possible API is taken as named");
    check(mbvq::backend_built(Backend::Rhi) && mbvq::backend_built(Backend::LegacyGL) && mbvq::backend_built(Backend::Null),
          "this build has the QRhi, OpenGL and null backends");
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

  /* a fill: the terrain slot and the eleven overlay groups; the status texts
     of the fill reach the program after it */
  {
    mbvq::MapWidget *widget = mbvq::map_widget(instance);
    int during = 0, after = 0;
    bool inside = false, active = false;
    QObject::connect(widget, &mbvq::MapWidget::statusMessage, widget, [&](const QString &) {
      (inside ? during : after)++;
      active = active || mbvq::fill_active();
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
    check(during == 0 && after > 0 && !active,
          "status signals raised inside a fill reach the program after it (" + std::to_string(after) + " after, " +
              std::to_string(during) + " during)");
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
     (mbview_viewbounds, the batch fallback; logs/TODO.md) */
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
              !empty->colours && plan.tiles()[t].empty && !plan.drawable(t) && tiles_with_triangles(plan).size() == 5,
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
              plan.drawable(t) && !plan.tiles()[t].empty,
          "cell edits that give it a triangle gather its positions, colours and index list: it is drawn");
    mbview_updateprimarygridcell(0, fourth, 10, 151, data4->primary_nodatavalue, &error);
    Planned emptied = plan_frame(frame4, plan);
    const rhi::TileUpload *none = upload_of(emptied);
    check(emptied.tiles(2) == std::vector<int>{t} && none != nullptr && none->index.empty() && plan.tiles()[t].empty &&
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
  return result();
}

/*--------------------------------------------------------------------*/
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
  size_t second = MBV_NO_WINDOW;
  check(open_grid(grid, nullptr, &second), "a second map under QRhi");
  QWidget *view2 = mbvq::map_view(second);
  timer.restart();
  while (!mbvq::view_state(view2).failure_shown && timer.elapsed() < 3000)
    spin(20);
  check(mbvq::view_state(view2).failure_shown && mbvq::failure_messages() == 1,
        "each failing view shows its label, the line comes once per process");
  close_instance(second);

  /* input before the first frame does not reach the core */
  struct mbview_struct *data = data_of(instance);
  data->pick_type = MBV_PICK_NONE;
  const QPointF centre(view->width() / 2.0, view->height() / 2.0);
  QMouseEvent press(QEvent::MouseButtonPress, centre, view->mapToGlobal(centre), Qt::LeftButton, Qt::LeftButton,
                    Qt::NoModifier);
  QApplication::sendEvent(view, &press);
  const int down = mbvq::view_state(view).buttons;
  QMouseEvent release(QEvent::MouseButtonRelease, centre, view->mapToGlobal(centre), Qt::LeftButton, Qt::NoButton,
                      Qt::NoModifier);
  QApplication::sendEvent(view, &release);
  check(data->pick_type == MBV_PICK_NONE && down == 1 && mbvq::view_state(view).buttons == 0,
        "a click before the first frame picks nothing; the button is tracked");

  /* a lost release: the button state is cleared */
  bool cleared = true;
  for (QEvent::Type type : {QEvent::UngrabMouse, QEvent::Hide, QEvent::WindowDeactivate}) {
    QMouseEvent middle(QEvent::MouseButtonPress, centre, view->mapToGlobal(centre), Qt::MiddleButton, Qt::MiddleButton,
                       Qt::NoModifier);
    QApplication::sendEvent(view, &middle);
    const bool held = mbvq::view_state(view).buttons == 2;
    QEvent lost(type);
    QApplication::sendEvent(view, &lost);
    cleared = cleared && held && mbvq::view_state(view).buttons == 0;
  }
  check(cleared, "UngrabMouse, Hide and WindowDeactivate clear the mouse buttons");

  /* the profile and soundings views are null views for now */
  check(mbvq::view_state(mbvq::map_widget(instance)->profile()->view()).rhi_view == false,
        "the profile view is not a QRhi view yet");

  /* closing the window deletes the view that never had a QRhi */
  close_instance(instance);
  check(mbvq::window(instance) == nullptr, "a QRhi view that never drew is closed and deleted");
  return result();
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

  /* split uploads over frames; the frames after the first reuse the batch */
  renderer.release();
  options.upload_budget = 600 * 1024;
  rendered = renderer.render_map(instance, 320, 240, options, nullptr, 20);
  stats = renderer.stats();
  check(rendered && renderer.last_frames() > 1 && stats.tiles_waiting == 0 && stats.fills == 1 &&
            stats.tiles_drawn == stats.uploads_positions,
        "a 600 kB budget: " + std::to_string(renderer.last_frames()) +
            " frames, one fill, every tile drawn in the end (largest frame " +
            std::to_string(stats.bytes_largest_frame / 1024) + " kB)");
  options.upload_budget = mbvq::RenderOptions().upload_budget;

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

  /* tile buffers that cannot be made (the GPU memory ran out): the frame
     fails and draws nothing; the renderer keeps its five pipeline resources
     and no tile, and the next frame uploads every tile again */
  renderer.release();
  options.fail_buffers = true;
  rendered = renderer.render_map(instance, 320, 240, options, nullptr);
  const mbvq::RenderStats failed = renderer.stats();
  options.fail_buffers = false;
  rendered = !rendered && renderer.render_map(instance, 320, 240, options, nullptr);
  stats = renderer.stats();
  check(failed.failed && failed.resources == 5 && failed.tiles_drawn == 0 && rendered && !stats.failed &&
            stats.uploads_positions - failed.uploads_positions == tiles_uploaded && stats.tiles_drawn == tiles_uploaded &&
            stats.resources == 5 + 3 * tiles_uploaded,
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
  check(rendered && stats.tiles == 4 && stats.tiles_drawn == 4 && stats.resources == 5 + 3 * 4,
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
    check(rendered && empty.tiles == 6 && empty.tiles_drawn == 5 && empty.resources == 5 + 3 * 5 &&
              triangle.tiles_drawn == 6 && triangle.triangles == empty.triangles + 1 &&
              triangle.resources == 5 + 3 * 6 && stats.tiles_drawn == 5 && stats.resources == 5 + 3 * 5,
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
  return result();
}

/*--------------------------------------------------------------------*/
/* pixels of the Metal mode */
constexpr int kWidth = 400;
constexpr int kHeight = 300;
constexpr uint32_t kWhite = 0xffffff;

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

/* A program with a QRhi map and its profile window open ends (--quit). */
int test_quit(const char *how, const char *grdfile) {
  const bool floating = std::strcmp(how, "float") == 0;
  /* "float": the map in a dock of the program's main window */
  QMainWindow *host = nullptr;
  QDockWidget *dock = nullptr;
  if (floating) {
    host = new QMainWindow;
    host->setCentralWidget(new QLabel(QStringLiteral("the program")));
    host->resize(1200, 800);
    host->show();
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
  /* its profile window too (a null view under QRhi for now) */
  map->profile()->open();
  check(wait_full(instance, 20000) && terrain_pixels(instance) > 0,
        std::string("the map drawn (window exposed: ") + (exposed(mbvq::map_view(instance)) ? "1" : "0") + ")");
  if (floating) {
    dock->setFloating(true);
    spin(1500);
    mbvq_c_resetview(instance);
    spin(500);
    check(dock->isFloating() && terrain_pixels(instance) > 0 && mbvq::view_state(mbvq::map_view(instance)).renderer,
          "the map's dock floats: drawn with the QRhi of the floating window");
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
      /* the macOS Quit menu of the programs: exit() inside the event loop */
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
  if (floating) {
    /* the program ends its maps, then its main window goes, which held the
       map before it floated (QTBUG-151834) */
    const int removed = mbvq::stale_callbacks_removed();
    int error = MB_ERROR_NO_ERROR;
    mbview_quit(0, &error);
    spin(100);
    delete host;
    spin(100);
    check(mbvq::stale_callbacks_removed() == removed + 1,
          "the floated map's callback in the main window's QRhi was removed when its view went, before the main "
          "window (QTBUG-151834)");
  }
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

/* The times of single frames: from the paint to the submission, by the
   resolution drawn; the view's paint event, and from its end to the
   submission (the window's composition with its waits); under OpenGL from
   the paint until the window composes; under QRhi the renderer's phases,
   the parts of the paint before and after render() (QRhiWidget begins and
   ends its offscreen frame there, waiting for the GPU at the end), the GPU
   time, which the next frame reports, and the lattice nodes whose positions
   or colours the frame gathered, with the preparation and gathering per
   node. */
void report_frames(const char *scenario, const FrameProbe &probe, const std::vector<size_t> &indices, bool rhi) {
  std::vector<double> frame, by_rez[4], paint, paint_event, after_paint, render, fill, marks, prepare, gather, upload,
      record, gpu, before_render, after_render, beyond_gpu, nodes_k, prepare_ns, gather_ns;
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

int bench(bool mode3d, const char *grdfile, int steps, int width, int height) {
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
  bool arguments = true;
  for (int i = quit ? 3 : 2; i < argc; i++) {
    if (std::strcmp(argv[i], "--3d") == 0)
      mode3d = true;
    else if (std::strcmp(argv[i], "--save") == 0 && i + 1 < argc)
      save = argv[++i];
    else if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
      if (std::sscanf(argv[++i], "%d", &steps) != 1 || steps < 2)
        arguments = false;
    }
    else if (std::strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
      if (std::sscanf(argv[++i], "%dx%d", &width, &height) != 2 || width < 64 || height < 64)
        arguments = false;
    }
    else
      grdfile = argv[i];
  }
  const bool bench_mode = std::strcmp(mode, "--bench") == 0;
  const bool display = std::strcmp(mode, "--display") == 0 || quit || bench_mode;
  const bool known = arguments &&
                     (std::strcmp(mode, "--model") == 0 || std::strcmp(mode, "--widgets") == 0 ||
                      std::strcmp(mode, "--render-null") == 0 || std::strcmp(mode, "--render-metal") == 0 ||
                      std::strcmp(mode, "--display") == 0 || bench_mode ||
                      (quit && (std::strcmp(how, "close") == 0 || std::strcmp(how, "quit") == 0 ||
                                std::strcmp(how, "exit") == 0 || std::strcmp(how, "float") == 0)));
  if (!known) {
    std::fprintf(stderr, "usage: mbview_qt_rhi_test --model | --widgets | --render-null | --render-metal [--save file] "
                         "| --display [--3d] [--save file] [grid] | --quit close|quit|exit|float [grid] "
                         "| --bench [--3d] [--frames n] [--size WxH] [grid]\n");
    return 2;
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
    return test_quit(how, grdfile);
  if (bench_mode)
    return bench(mode3d, grdfile, steps, width, height);
  return test_display(mode3d, grdfile, save);
}
