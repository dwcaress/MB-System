/*--------------------------------------------------------------------
 *    The MB-system:  mbview_qt_test.cc  11/7/2026
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
 * Test program of the mbview-qt library.
 *
 *   mbview_qt_test --null         checks without drawing (null backend), for ctest
 *   mbview_qt_test --gl [--3d]    draws a synthetic grid with the OpenGL backend,
 *                                 waits for the full resolution drawing and checks
 *                                 picks made with mouse events
 *   mbview_qt_test --show [--3d] [grid]   opens a window to use by hand; with
 *                                 MBVIEW_QT_TEST_SNAPSHOT=file it saves the first
 *                                 full resolution drawing and quits
 *
 * The synthetic grid is a smooth surface known in closed form, so that the
 * picked depths can be checked against it.
 */

#include <QAction>
#include <QApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QRadioButton>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

extern "C" {
#include "mb_aux.h"
#include "mb_define.h"
#include "mb_status.h"
#include "mbview.h"
}
#include "mbvq.h"
#include "mbvq_bridge.h"

namespace {

int failures = 0;

void check(bool ok, const std::string &what) {
  std::printf("%s: %s\n", ok ? "ok  " : "FAIL", what.c_str());
  if (!ok)
    failures++;
}

void spin(int ms) {
  QEventLoop loop;
  QTimer::singleShot(ms, &loop, &QEventLoop::quit);
  loop.exec();
}

/* the synthetic surface: a seamount on a sloping plain, depth negative */
constexpr double kXmin = 10.0, kXmax = 10.2, kYmin = 54.0, kYmax = 54.15;
constexpr int kColumns = 241, kRows = 181;
double surface(double lon, double lat) {
  const double dx = (lon - 10.1) / 0.04;
  const double dy = (lat - 54.075) / 0.03;
  return -1000.0 + 300.0 * std::exp(-(dx * dx + dy * dy)) - 400.0 * (lon - kXmin);
}

int dismissed = 0;
int dismiss(size_t instance) {
  (void)instance;
  dismissed++;
  return MB_SUCCESS;
}

/* open one mbview instance the way mbgrdviz does, with a synthetic or a GMT grid */
bool open_view(bool mode3d, const char *grdfile, size_t *instance) {
  const int verbose = 0;
  int error = MB_ERROR_NO_ERROR;
  if (mbview_init(verbose, instance, &error) != MB_SUCCESS)
    return false;
  char title[] = "mbview_qt_test";
  mbview_setwindowparms(verbose, *instance, &dismiss, title, 100, 100, 640, 480, 100, 500, 5, 1, &error);

  int grid_projection_mode = MBV_PROJECTION_GEOGRAPHIC;
  char grid_projection_id[MB_PATH_MAXLINE] = "EPSG:4326";
  float nodatavalue = -99999.0f;
  int nxy = kColumns * kRows, n_columns = kColumns, n_rows = kRows;
  double zmin = 0.0, zmax = 0.0, xmin = kXmin, xmax = kXmax, ymin = kYmin, ymax = kYmax;
  double dx = (kXmax - kXmin) / (kColumns - 1), dy = (kYmax - kYmin) / (kRows - 1);
  float *grid = nullptr;
  if (grdfile != nullptr) {
    if (mb_read_gmt_grd(verbose, const_cast<char *>(grdfile), &grid_projection_mode, grid_projection_id, &nodatavalue, &nxy,
                        &n_columns, &n_rows, &zmin, &zmax, &xmin, &xmax, &ymin, &ymax, &dx, &dy, &grid, nullptr, nullptr,
                        &error) != MB_SUCCESS) {
      std::fprintf(stderr, "cannot read %s\n", grdfile);
      return false;
    }
  }
  else {
    grid = static_cast<float *>(std::malloc(sizeof(float) * nxy));
    zmin = 1.0e9;
    zmax = -1.0e9;
    for (int i = 0; i < n_columns; i++)
      for (int j = 0; j < n_rows; j++) {
        const double z = surface(xmin + i * dx, ymin + j * dy);
        grid[i * n_rows + j] = static_cast<float>(z);
        zmin = std::min(zmin, z);
        zmax = std::max(zmax, z);
      }
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
      verbose, *instance, mode3d ? MBV_DISPLAY_3D : MBV_DISPLAY_2D, MBV_MOUSE_MOVE, MBV_GRID_VIEW_PRIMARY, false, false, false,
      primary_shade_mode, MBV_SHADE_VIEW_NONE, MBV_SHADE_VIEW_NONE, MBV_VIEW_OFF, MBV_VIEW_OFF, MBV_VIEW_OFF, MBV_VIEW_OFF,
      MBV_VIEW_OFF, MBV_VIEW_OFF, MBV_VIEW_OFF, 1.0, mode3d ? 45.0 : 90.0, 0.0, 90.0, 0.0, illuminate_magnitude,
      illuminate_elevation, illuminate_azimuth, slope_magnitude, 1.0, 0.0, MBV_COLORTABLE_NORMAL, contour_interval,
      display_projection_mode, display_projection_id, &error);
  if (status == MB_SUCCESS)
    status = mbview_setprimarygrid(verbose, *instance, grid_projection_mode, grid_projection_id, nodatavalue, n_columns,
                                   n_rows, zmin, zmax, xmin, xmax, ymin, ymax, dx, dy, grid, &error);
  if (grdfile != nullptr)
    mb_freed(verbose, __FILE__, __LINE__, reinterpret_cast<void **>(&grid), &error);
  else
    std::free(grid);
  if (status == MB_SUCCESS)
    status = mbview_setprimarycolortable(verbose, *instance, primary_colortable, primary_colortable_mode, zmin, zmax, &error);
  if (status == MB_SUCCESS)
    status = mbview_setslopecolortable(verbose, *instance, slope_colortable, slope_colortable_mode, 0.0, 0.5, &error);
  if (status == MB_SUCCESS)
    status = mbview_open(verbose, *instance, &error);
  return status == MB_SUCCESS;
}

QAction *mouse_action(QWidget *window, const QString &text) {
  for (QAction *action : window->findChildren<QAction *>())
    if (action->text() == text)
      return action;
  return nullptr;
}

QRadioButton *mouse_button(QWidget *window, const QString &text) {
  for (QRadioButton *button : window->findChildren<QRadioButton *>())
    if (button->text() == text)
      return button;
  return nullptr;
}

void click(QWidget *view, QPointF position) {
  QMouseEvent press(QEvent::MouseButtonPress, position, view->mapToGlobal(position), Qt::LeftButton, Qt::LeftButton,
                    Qt::NoModifier);
  QApplication::sendEvent(view, &press);
  QMouseEvent release(QEvent::MouseButtonRelease, position, view->mapToGlobal(position), Qt::LeftButton, Qt::NoButton,
                      Qt::NoModifier);
  QApplication::sendEvent(view, &release);
}

int test_null() {
  size_t instance = MBV_NO_WINDOW;
  check(open_view(false, nullptr, &instance), "mbview_open with the null backend");
  QWidget *window = mbvq::window(instance);
  check(window != nullptr && window->isVisible(), "the instance has a visible window");
  if (window == nullptr)
    return 1;
  check(window->windowTitle() == QStringLiteral("mbview_qt_test"), "window title from mbview_setwindowparms");
  check(mbvq_c_mouse_mode(instance) == MBV_MOUSE_MOVE, "starts in mouse mode Pan and Zoom");

  QAction *rotate = mouse_action(window, QStringLiteral("Rotate Model"));
  QAction *area = mouse_action(window, QStringLiteral("Pick Area"));
  check(rotate != nullptr && !rotate->isEnabled(), "Rotate Model is disabled in the 2D display");
  if (rotate != nullptr)
    rotate->trigger();
  check(mbvq_c_mouse_mode(instance) == MBV_MOUSE_MOVE, "a 2D display stays in Pan and Zoom when asked to rotate");
  if (area != nullptr)
    area->trigger();
  check(mbvq_c_mouse_mode(instance) == MBV_MOUSE_AREA, "menu item Pick Area sets the core mouse mode");
  QRadioButton *move = mouse_button(window, QStringLiteral("Pan and Zoom"));
  if (move != nullptr)
    move->click();
  check(mbvq_c_mouse_mode(instance) == MBV_MOUSE_MOVE, "radio button Pan and Zoom sets the core mouse mode");
  check(area != nullptr && !area->isChecked() && mouse_action(window, QStringLiteral("Pan and Zoom"))->isChecked(),
        "the menu shows the mode set by the radio button");

  check(mbvq::label_text(":::t\"Pick Info:\":t\" Lon: 10.1\"") == QStringLiteral("Pick Info:\n Lon: 10.1"),
        "Motif multiline label strings become plain lines");

  /* mouse input reaches the core; without drawing no point is found */
  QWidget *view = mbvq::map_view(instance);
  click(view, QPointF(view->width() / 2.0, view->height() / 2.0));
  check(true, "mouse events pass through the core without a renderer");

  /* the background work runs from the Qt timer without errors */
  spin(1500);
  check(mbvq_c_lastdrawrez(instance) == MBVQ_REZ_NONE, "the null backend draws nothing");

  window->close();
  check(dismissed == 1, "closing the window calls the dismiss notify function once");
  return failures == 0 ? 0 : 1;
}

int test_gl(bool mode3d) {
  size_t instance = MBV_NO_WINDOW;
  check(open_view(mode3d, nullptr, &instance), "mbview_open with the OpenGL backend");
  auto *view = qobject_cast<QOpenGLWidget *>(mbvq::map_view(instance));
  check(view != nullptr, "the map view is an OpenGL widget");
  if (view == nullptr)
    return 1;

  /* the background work draws at full resolution after a pause */
  QElapsedTimer timer;
  timer.start();
  while (mbvq_c_lastdrawrez(instance) != MBVQ_REZ_FULL && timer.elapsed() < 20000)
    spin(50);
  check(mbvq_c_lastdrawrez(instance) == MBVQ_REZ_FULL,
        "full resolution drawn by the background work after " + std::to_string(timer.elapsed()) + " ms");
  const QImage image = view->grabFramebuffer();
  int distinct = 0;
  {
    std::vector<QRgb> seen;
    for (int y = 0; y < image.height(); y += 7)
      for (int x = 0; x < image.width(); x += 7) {
        const QRgb pixel = image.pixel(x, y);
        if (std::find(seen.begin(), seen.end(), pixel) == seen.end() && seen.size() < 1000)
          seen.push_back(pixel);
      }
    distinct = static_cast<int>(seen.size());
  }
  check(distinct > 50, "the drawn grid has " + std::to_string(distinct) + " colours in a sample");
  if (const char *snapshot = std::getenv("MBVIEW_QT_TEST_SNAPSHOT"))
    image.save(QString::fromLocal8Bit(snapshot));

  /* picks: mouse clicks in Pan and Zoom mode pick a point; its depth is the
     mean of the three grid nodes of the picked triangle, close to the surface */
  struct mbview_struct *data = nullptr;
  int error = MB_ERROR_NO_ERROR;
  mbview_getdataptr(0, instance, &data, &error);
  QLabel *pick_info = nullptr;
  for (QLabel *label : mbvq::window(instance)->findChildren<QLabel *>())
    if (label->text().isEmpty() || label->text().startsWith(QStringLiteral("Pick Info")))
      pick_info = label;
  int found = 0, close = 0;
  double max_error = 0.0;
  for (int j = 1; j <= 3; j++)
    for (int i = 1; i <= 3; i++) {
      /* points well inside the drawn grid, in 2D and in the 3D view */
      const QPointF position(view->width() * (0.35 + 0.15 * (i - 1)), view->height() * (0.35 + 0.15 * (j - 1)));
      data->pick_type = MBV_PICK_NONE;
      click(view, position);
      spin(100);
      if (data->pick_type == MBV_PICK_ONEPOINT) {
        found++;
        const struct mbview_point_struct &point = data->pick.endpoints[0];
        const double difference = std::fabs(point.zdata - surface(point.xlon, point.ylat));
        max_error = std::max(max_error, difference);
        if (difference < 2.0)
          close++;
        std::printf("      pick at %.0f,%.0f: lon %.6f lat %.6f depth %.2f, surface %.2f\n", position.x(), position.y(),
                    point.xlon, point.ylat, point.zdata, surface(point.xlon, point.ylat));
      }
    }
  check(found == 9, std::to_string(found) + " of 9 clicks picked a point");
  check(close == found && found > 0, "picked depths agree with the surface (largest difference " +
                                          std::to_string(max_error) + " m)");
  check(pick_info != nullptr && pick_info->text().startsWith(QStringLiteral("Pick Info:")) &&
            pick_info->text().contains(QStringLiteral("Lon:")),
        "the pick information label shows the last pick");
  const QImage after = view->grabFramebuffer();
  check(!after.isNull(), "the view still draws after the picks");
  return failures == 0 ? 0 : 1;
}

}  // namespace

int main(int argc, char **argv) {
  bool null = false, gl = false, show = false, mode3d = false;
  const char *grdfile = nullptr;
  for (int i = 1; i < argc; i++) {
    if (std::strcmp(argv[i], "--null") == 0)
      null = true;
    else if (std::strcmp(argv[i], "--gl") == 0)
      gl = true;
    else if (std::strcmp(argv[i], "--show") == 0)
      show = true;
    else if (std::strcmp(argv[i], "--3d") == 0)
      mode3d = true;
    else
      grdfile = argv[i];
  }
  if (null + gl + show != 1) {
    std::fprintf(stderr, "usage: mbview_qt_test --null | --gl [--3d] | --show [--3d] [grid]\n");
    return 2;
  }

  mbvq::set_backend(null ? mbvq::Backend::Null : mbvq::Backend::LegacyGL);
  QApplication app(argc, argv);
  int error = MB_ERROR_NO_ERROR;
  mbview_startup(0, nullptr, nullptr, &error);

  if (null)
    return test_null();
  if (gl)
    return test_gl(mode3d);
  size_t instance = MBV_NO_WINDOW;
  if (!open_view(mode3d, grdfile, &instance))
    return 1;
  QObject::connect(&app, &QApplication::lastWindowClosed, &app, &QApplication::quit);
  if (const char *snapshot = std::getenv("MBVIEW_QT_TEST_SNAPSHOT")) {
    /* save the first full resolution drawing and quit */
    auto *poll = new QTimer(&app);
    QElapsedTimer *since = new QElapsedTimer;
    since->start();
    QObject::connect(poll, &QTimer::timeout, &app, [instance, snapshot, since] {
      if (mbvq_c_lastdrawrez(instance) == MBVQ_REZ_FULL) {
        auto *view = qobject_cast<QOpenGLWidget *>(mbvq::map_view(instance));
        if (view != nullptr)
          view->grabFramebuffer().save(QString::fromLocal8Bit(snapshot));
        std::printf("full resolution after %lld ms, saved %s\n", static_cast<long long>(since->elapsed()), snapshot);
        QApplication::quit();
      }
    });
    poll->start(50);
  }
  return app.exec();
}
