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
 *   mbview_qt_test --null         checks without drawing (null backend; picks are
 *                                 found on the CPU), for ctest
 *   mbview_qt_test --gl [--3d]    draws a synthetic grid with the OpenGL backend,
 *                                 waits for the full resolution drawing and checks
 *                                 picks made with mouse events
 *   mbview_qt_test --show [--3d] [--edit] [grid]   opens a window to use by hand
 *                                 (--edit: sites, routes and polylines enabled); with
 *                                 MBVIEW_QT_TEST_SNAPSHOT=file it saves the first
 *                                 full resolution drawing and quits; with
 *                                 MBVIEW_QT_TEST_CLOSE also set, it picks and
 *                                 closes the window instead (=exit: then calls
 *                                 exit() as the macOS Quit menu does)
 *
 * The synthetic grid is a smooth surface known in closed form, so that the
 * picked depths can be checked against it.
 */

#include <QAction>
#include <QApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QImage>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QScrollArea>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QTimer>
#include <QWindow>
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
#include "mbvq_dialogs.h"
#include "mbvq_lists.h"
#include "mbvq_profile.h"
#include "mbvq_window.h"

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
  /* menu items only, not the title actions of the menus */
  for (QAction *action : window->findChildren<QAction *>())
    if (action->text() == text && action->menu() == nullptr)
      return action;
  return nullptr;
}

QRadioButton *mouse_button(QWidget *window, const QString &text) {
  for (QRadioButton *button : window->findChildren<QRadioButton *>())
    if (button->text() == text)
      return button;
  return nullptr;
}

int action_calls = 0;
size_t action_instance = MBV_NO_WINDOW;
void count_action(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  action_calls++;
  action_instance = reinterpret_cast<size_t>(client_data);
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
  QRadioButton *move = mouse_button(window, QStringLiteral("Pan && Zoom"));
  if (move != nullptr)
    move->click();
  check(mbvq_c_mouse_mode(instance) == MBV_MOUSE_MOVE, "radio button Pan and Zoom sets the core mouse mode");
  check(area != nullptr && !area->isChecked() && mouse_action(window, QStringLiteral("Pan && Zoom"))->isChecked(),
        "the menu shows the mode set by the radio button");

  check(mbvq::label_text(":::t\"Pick Info:\":t\" Lon: 10.1\"") == QStringLiteral("Pick Info:\n Lon: 10.1"),
        "Motif multiline label strings become plain lines");

  /* View menu: each item changes the core, and the menu shows the core state */
  struct mbview_struct *data = nullptr;
  int error = MB_ERROR_NO_ERROR;
  mbview_getdataptr(0, instance, &data, &error);
  auto view_item = [window](const char *text) { return mouse_action(window, QString::fromLatin1(text)); };
  check(view_item("Map Display") != nullptr && view_item("Map Display")->isChecked(), "View menu shows Map Display");
  view_item("3D Display")->trigger();
  check(mbvq_c_display_3d(instance) && view_item("3D Display")->isChecked() && rotate->isEnabled(),
        "3D Display sets the 3D display and enables Rotate Model");
  view_item("Map Display")->trigger();
  check(!mbvq_c_display_3d(instance) && !rotate->isEnabled(), "Map Display returns to 2D");
  view_item("Bright Rainbow Colortable")->trigger();
  check(data->primary_colortable == MBV_COLORTABLE_BRIGHT && view_item("Bright Rainbow Colortable")->isChecked() &&
            !view_item("Haxby Colortable")->isChecked(),
        "a colour table item sets the colour table of the grid shown");
  view_item("Shading by Illumination")->trigger();
  check(data->primary_shade_mode == MBV_SHADE_VIEW_ILLUMINATION, "a shading item sets the shading of the grid shown");
  view_item("Histogram Equalization")->trigger();
  check(data->primary_histogram && view_item("Histogram Equalization")->isChecked(), "Histogram Equalization toggles on");
  view_item("Topography Contours")->trigger();
  check(data->grid_contour_mode == MBV_VIEW_ON, "Topography Contours toggles on");
  view_item("Topography Slope")->trigger();
  check(data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE && !view_item("Histogram Equalization")->isChecked() &&
            view_item("Haxby Colortable")->isChecked(),
        "Topography Slope shows the slope grid with its own settings");
  view_item("Topography")->trigger();
  check(data->grid_mode == MBV_GRID_VIEW_PRIMARY && view_item("Bright Rainbow Colortable")->isChecked(),
        "Topography shows the primary grid with its settings kept");
  check(!view_item("Overlay")->isEnabled() && !view_item("Shading by Overlay")->isEnabled(),
        "overlay items are disabled without an overlay grid");
  check(!view_item("Sites")->isEnabled() && !view_item("Navigation")->isEnabled(),
        "Sites and Navigation are disabled when not enabled by the program");
  check(view_item("Profile")->isEnabled() && !view_item("Profile")->isChecked(), "Profile is available with a grid");

  /* Reset button: view defaults, as the Motif Reset button */
  data->exaggeration = 3.0;
  data->modelelevation3d = 30.0;
  for (QPushButton *button : window->findChildren<QPushButton *>())
    if (button->text() == QStringLiteral("Reset"))
      button->click();
  check(data->exaggeration == 1.0 && data->modelelevation3d == 90.0, "Reset restores the view defaults");

  /* Controls dialogs: they show the core values and apply through the core */
  auto *map_window = static_cast<mbvq::MapWindow *>(window);
  mbvq::ControlDialogs *dialogs = map_window->widget()->dialogs();
  view_item("Colors && Contours")->trigger();
  check(dialogs->dialog(MBVQ_DIALOG_COLORBOUNDS)->isVisible() &&
            dialogs->field(MBVQ_F_DATAMIN)->text() == QString::asprintf("%g", data->primary_colortable_min),
        "Colors & Contours shows the colour bounds of the core");
  check(!dialogs->field(MBVQ_F_OVERLAYMIN)->isEnabled(), "overlay bounds are disabled without an overlay grid");
  dialogs->field(MBVQ_F_DATAMAX)->setText(QStringLiteral("-500"));
  dialogs->field(MBVQ_F_CONTOURS)->setText(QStringLiteral("25"));
  dialogs->hot_to_cold(MBVQ_T_DATA_HTOC)->setChecked(true);
  for (QPushButton *b : dialogs->dialog(MBVQ_DIALOG_COLORBOUNDS)->findChildren<QPushButton *>())
    if (b->text() == QStringLiteral("Apply"))
      b->click();
  check(data->primary_colortable_max == -500.0 && data->contour_interval == 25.0 &&
            data->primary_colortable_mode == MBV_COLORTABLE_REVERSED,
        "Apply sets the colour bounds, contour interval and colour mode");

  view_item("3D Controls")->trigger();
  dialogs->field(MBVQ_F_EXAGGERATION)->setText(QStringLiteral("2.5"));
  dialogs->field(MBVQ_F_MODEL_ELEV)->setText(QStringLiteral("not a number"));
  for (QPushButton *b : dialogs->dialog(MBVQ_DIALOG_VIEW3D)->findChildren<QPushButton *>())
    if (b->text() == QStringLiteral("Apply"))
      b->click();
  check(data->exaggeration == 2.5 && data->modelelevation3d == 90.0,
        "3D Apply sets the exaggeration and ignores text that is not a number");
  for (QPushButton *button : window->findChildren<QPushButton *>())
    if (button->text() == QStringLiteral("Reset") && button->window() == window)
      button->click();
  check(dialogs->field(MBVQ_F_EXAGGERATION)->text() == QStringLiteral("1") &&
            dialogs->field(MBVQ_F_MODEL_ELEV)->text() == QStringLiteral("90"),
        "the open 3D dialog shows the values after Reset");

  view_item("Shading")->trigger();
  dialogs->field(MBVQ_F_ILLUM_AZI)->setText(QStringLiteral("45"));
  for (QPushButton *b : dialogs->dialog(MBVQ_DIALOG_SHADE)->findChildren<QPushButton *>())
    if (b->text() == QStringLiteral("Apply"))
      b->click();
  check(data->illuminate_azimuth == 45.0, "Shading Apply sets the illumination azimuth");

  view_item("2D Controls")->trigger();
  dialogs->field(MBVQ_F_ZOOM2D)->setText(QStringLiteral("2"));
  for (QPushButton *b : dialogs->dialog(MBVQ_DIALOG_VIEW2D)->findChildren<QPushButton *>())
    if (b->text() == QStringLiteral("Apply"))
      b->click();
  struct mbvq_controls controls;
  mbvq_c_controls(instance, &controls);
  check(controls.field[MBVQ_F_ZOOM2D] == 2.0, "2D Apply sets the zoom");

  view_item("Resolution")->trigger();
  auto *low = dialogs->dialog(MBVQ_DIALOG_RESOLUTION)->findChildren<QSlider *>().value(0);
  if (low != nullptr)
    low->setValue(112);
  check(data->lorez_dimension == 100 && low != nullptr && low->value() == 100,
        "Resolution rounds the low dimension to 100 and the slider shows it");

  view_item("Projections")->trigger();
  QRadioButton *decimal = nullptr, *spheroid = nullptr;
  for (QRadioButton *b : dialogs->dialog(MBVQ_DIALOG_PROJECTION)->findChildren<QRadioButton *>()) {
    if (b->text() == QStringLiteral("Decimal Degrees"))
      decimal = b;
    if (b->text() == QStringLiteral("Spheroid"))
      spheroid = b;
  }
  QLabel *projection_label = nullptr;
  for (QLabel *l : dialogs->dialog(MBVQ_DIALOG_PROJECTION)->findChildren<QLabel *>())
    if (l->text().startsWith(QStringLiteral("Primary Grid Projection:")))
      projection_label = l;
  check(projection_label != nullptr && projection_label->text().contains(QStringLiteral("Geographic")),
        "Projections shows the projection label of the core");
  if (decimal != nullptr)
    decimal->click();
  if (spheroid != nullptr)
    spheroid->click();
  mbvq_c_controls(instance, &controls);
  check(controls.lonlatstyle == 0 && controls.display_projection == 2 && projection_label != nullptr &&
            projection_label->text().contains(QStringLiteral("Spheroid")),
        "Projections sets the annotation style and the display projection");

  /* List windows: they show the rows of the core and pass the selected rows */
  struct mbview_shareddata_struct *sd = nullptr;
  mbview_getsharedptr(0, &sd, &error);
  auto list_button = [](QWidget *w, const char *text) -> QPushButton * {
    for (QPushButton *b : w->findChildren<QPushButton *>())
      if (b->text() == QString::fromLatin1(text))
        return b;
    return nullptr;
  };
  mbview_enableeditsites(0, instance, &error);
  double slon[3] = {10.05, 10.1, 10.15}, slat[3] = {54.05, 54.075, 54.1};
  double stopo[3] = {surface(slon[0], slat[0]), surface(slon[1], slat[1]), surface(slon[2], slat[2])};
  int scolor[3] = {MBV_COLOR_BLACK, MBV_COLOR_BLUE, MBV_COLOR_GREEN}, ssize[3] = {1, 1, 1};
  mb_path sname[3];
  for (int i = 0; i < 3; i++)
    std::snprintf(sname[i], sizeof(sname[i]), "Site %c", 'A' + i);
  mbview_addsites(0, instance, 3, slon, slat, stopo, scolor, ssize, sname, &error);
  view_item("Site List")->trigger();
  mbvq::ListWindow *sites = mbvq::list_window(MBVQ_LIST_SITE);
  check(sites->isVisible() && sites->view()->count() == 3 && sites->view()->item(1)->text().startsWith(QStringLiteral("  1 | ")) &&
            sites->view()->item(1)->text().endsWith(QStringLiteral("| Site B")),
        "Site List shows a row per site");
  sites->view()->item(1)->setSelected(true);
  spin(50);
  check(sd->site_selected == 1, "selecting a site row selects the site");
  list_button(sites, "Delete Selected Sites")->click();
  check(sd->nsite == 2 && sites->view()->count() == 2, "Delete Selected Sites deletes the selected site");

  mbview_enableeditroutes(0, instance, &error);
  mbview_enableviewpolylines(0, instance, &error);
  double rlon[3] = {10.05, 10.1, 10.15}, rlat[3] = {54.05, 54.06, 54.07};
  int waypoint[3] = {MBV_ROUTE_WAYPOINT_STARTLINE, MBV_ROUTE_WAYPOINT_TRANSIT, MBV_ROUTE_WAYPOINT_ENDLINE};
  mb_path rname;
  std::snprintf(rname, sizeof(rname), "Test Route");
  int iroute = -1;
  mbview_addroute(0, instance, 3, rlon, rlat, waypoint, MBV_COLOR_BLUE, 2, 1, rname, &iroute, &error);
  view_item("Route List")->trigger();
  mbvq::ListWindow *routes = mbvq::list_window(MBVQ_LIST_ROUTE);
  check(routes->view()->count() == 4 && routes->view()->item(0)->text().startsWith(QStringLiteral("Editable Route   0")) &&
            routes->view()->item(2)->text().endsWith(QStringLiteral("-TRANSIT-")),
        "Route List shows the route and its waypoints");
  routes->view()->item(2)->setSelected(true);
  spin(50);
  check(sd->route_selected == 0 && sd->route_point_selected == 1, "selecting a waypoint row selects the waypoint");

  int ilayer = -1;
  mbview_addgislayer(0, instance, MBV_GISLAYER_POLYLINE, false, "Test Layer", &ilayer, &error);
  int partstart[1] = {0};
  double plon[3] = {10.02, 10.1, 10.18}, plat[3] = {54.02, 54.1, 54.03}, pz[3] = {0.0, 0.0, 0.0};
  int ipolyline = -1;
  mbview_addpolyline(0, instance, 1, partstart, 3, plon, plat, pz, MBV_COLOR_BLACK, 1, "Test Polyline", ilayer, &ipolyline,
                     &error);
  view_item("Polyline List")->trigger();
  mbvq::ListWindow *polylines = mbvq::list_window(MBVQ_LIST_POLYLINE);
  check(polylines->view()->count() == 2 && polylines->view()->item(0)->text().startsWith(QStringLiteral("Layer ")) &&
            polylines->view()->item(1)->text().contains(QStringLiteral("| Shown |")),
        "Polyline List shows the layer and its polyline");
  polylines->view()->item(1)->setSelected(true);
  spin(50);
  check(sd->polyline_selected == 0, "selecting a polyline row selects the polyline");
  list_button(polylines, "Show/Hide Selected")->click();
  check(polylines->view()->item(1)->text().contains(QStringLiteral("| Hidden |")) && polylines->view()->item(1)->isSelected(),
        "Show/Hide hides the polyline and keeps the selection");

  QPushButton *topolylines = list_button(routes, "Convert Selected to Polylines");
  check(topolylines != nullptr && topolylines->isVisible(), "the route list offers the conversion when polylines are shown");
  {
    const QSignalBlocker block(routes->view());
    routes->view()->clearSelection();
    routes->view()->item(0)->setSelected(true);
  }
  topolylines->click();
  check(sd->npolyline == 2 && polylines->view()->count() == 4, "Convert Selected to Polylines adds a polyline in a new layer");

  /* Profile window: View > Profile, the sliders and Dismiss */
  view_item("Profile")->trigger();
  mbvq::ProfileWindow *profile = map_window->widget()->profile();
  check(profile->isVisible() && mbvq_c_profile_shown(instance) && view_item("Profile")->isChecked(),
        "View > Profile opens the profile window");
  profile->slider(2)->setValue(20);
  profile->slider(1)->setValue(50);
  profile->slider(0)->setValue(3);
  auto *scroll = profile->findChild<QScrollArea *>();
  check(data->profile_exaggeration == 2.0 && data->profile_slopethreshold == 0.5 && data->profile_widthfactor == 3 &&
            scroll != nullptr && profile->view()->width() == 3 * scroll->viewport()->width(),
        "the profile sliders set exaggeration, slope threshold and width multiple");
  for (QPushButton *b : profile->findChildren<QPushButton *>())
    if (b->text() == QStringLiteral("Dismiss"))
      b->click();
  check(!profile->isVisible() && !mbvq_c_profile_shown(instance) && !view_item("Profile")->isChecked(),
        "Dismiss closes the profile window");

  /* About and messages */
  view_item("About MBview...")->trigger();
  bool about = false, qt_author = false, marum = false;
  for (QDialog *d : window->findChildren<QDialog *>())
    if (d->isVisible() && d->windowTitle() == QStringLiteral("About MBview"))
      for (QLabel *l : d->findChildren<QLabel *>()) {
        if (l->text().startsWith(QStringLiteral("MB-System Release ")))
          about = true;
        if (l->text().contains(QStringLiteral("Christian dos Santos Ferreira")))
          qt_author = true;
        if (l->text().startsWith(QStringLiteral("MARUM\n")) && l->text().endsWith(QStringLiteral("University of Bremen")))
          marum = true;
      }
  check(about, "About MBview shows the MB-System release");
  check(qt_author && marum, "About MBview names the author of the Qt6 version and MARUM");
  char message[] = "Reading the grid...";
  do_mbview_message_on(message, instance);
  bool shown = false;
  for (QLabel *l : window->findChildren<QLabel *>())
    if (l->text() == QStringLiteral("Reading the grid...") && l->isVisible())
      shown = true;
  do_mbview_message_off(instance);
  bool hidden = true;
  for (QLabel *l : window->findChildren<QLabel *>())
    if (l->text() == QStringLiteral("Reading the grid...") && l->isVisible())
      hidden = false;
  check(shown && hidden, "do_mbview_message_on and _off show and hide the message");
  check(mbvq::label_text("::#TimesBold24\"MBview\":t\"\"#TimesBold18\"A 2D/3D Visualization API\"") ==
            QStringLiteral("MBview\nA 2D/3D Visualization API"),
        "only :t ends a line in Motif label strings");

  /* Action menu: items added by the program, enabled by the pick masks */
  mbview_addaction(0, instance, count_action, const_cast<char *>("Always"), MBV_PICKMASK_NONE, &error);
  mbview_addactionseparator(0, instance, &error);
  mbview_addaction(0, instance, count_action, const_cast<char *>("Needs One Pick"), MBV_PICKMASK_ONEPOINT, &error);
  QAction *always = mouse_action(window, QStringLiteral("Always"));
  QAction *needs_pick = mouse_action(window, QStringLiteral("Needs One Pick"));
  check(always != nullptr && always->isEnabled(), "an action without a pick mask is enabled");
  check(needs_pick != nullptr && !needs_pick->isEnabled(), "an action that needs a one-point pick is disabled");
  if (always != nullptr)
    always->trigger();
  check(action_calls == 1 && action_instance == instance, "an action calls the program with the instance as client data");

  /* mouse clicks in Pan and Zoom mode pick a point; without drawing, the null
     backend finds it on the CPU (mbvq_pick_cpu.c). In the 3D display, which
     takes no position from the pixel when no terrain is found, every click
     well inside the grid finds the terrain, and the depth is the mean of the
     three grid nodes of the picked triangle, close to the surface */
  QWidget *view = mbvq::map_view(instance);
  view_item("3D Display")->trigger();
  int found = 0, close = 0;
  double max_error = 0.0;
  for (int j = 1; j <= 3; j++)
    for (int i = 1; i <= 3; i++) {
      const QPointF position(view->width() * (0.35 + 0.15 * (i - 1)), view->height() * (0.35 + 0.15 * (j - 1)));
      data->pick_type = MBV_PICK_NONE;
      click(view, position);
      spin(50);
      if (data->pick_type == MBV_PICK_ONEPOINT) {
        found++;
        const struct mbview_point_struct &point = data->pick.endpoints[0];
        const double difference = std::fabs(point.zdata - surface(point.xlon, point.ylat));
        max_error = std::max(max_error, difference);
        if (difference < 2.0)
          close++;
      }
    }
  view_item("Map Display")->trigger();
  check(found == 9, std::to_string(found) + " of 9 clicks in the 3D display picked a point without drawing");
  check(close == found && found > 0,
        "picked depths agree with the surface (largest difference " + std::to_string(max_error) + " m)");

  /* the background work runs from the Qt timer without errors */
  spin(1500);
  check(mbvq_c_lastdrawrez(instance) == MBVQ_REZ_NONE, "the null backend draws nothing");

  /* Dismiss menu, as in Motif: mbview_destroy() closes the instance */
  mouse_action(window, QStringLiteral("Dismiss"))->trigger();
  spin(100);
  check(dismissed == 1, "Dismiss calls the dismiss notify function once");
  check(mbvq::window(instance) == nullptr && !data->active, "Dismiss removes the window and frees the instance");
  check(!mbvq::list_window(MBVQ_LIST_SITE)->isVisible() && !mbvq::list_window(MBVQ_LIST_ROUTE)->isVisible() &&
            !mbvq::list_window(MBVQ_LIST_POLYLINE)->isVisible(),
        "the list windows close with the last instance");

  /* a second instance closed with the window's close button */
  size_t second = MBV_NO_WINDOW;
  check(open_view(false, nullptr, &second), "a second instance opens after the first was dismissed");
  if (QWidget *window2 = mbvq::window(second))
    window2->close();
  spin(100);
  check(dismissed == 2 && mbvq::window(second) == nullptr, "closing the window dismisses the instance once");
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
  mbview_addaction(0, instance, count_action, const_cast<char *>("Needs One Pick"), MBV_PICKMASK_ONEPOINT, &error);
  QAction *needs_pick = mouse_action(mbvq::window(instance), QStringLiteral("Needs One Pick"));
  check(needs_pick != nullptr && needs_pick->isEnabled(), "after a pick the one-point action is enabled");
  for (QPushButton *button : mbvq::window(instance)->findChildren<QPushButton *>())
    if (button->text() == QStringLiteral("Clear"))
      button->click();
  check(data->pick_type == MBV_PICK_NONE && needs_pick != nullptr && !needs_pick->isEnabled(),
        "Clear removes the pick and disables the one-point action");
  check(close == found && found > 0, "picked depths agree with the surface (largest difference " +
                                          std::to_string(max_error) + " m)");
  check(pick_info != nullptr && pick_info->text().startsWith(QStringLiteral("Pick Info:")) &&
            pick_info->text().contains(QStringLiteral("Lon:")),
        "the pick information label shows the last pick");
  const QImage after = view->grabFramebuffer();
  check(!after.isNull(), "the view still draws after the picks");

  /* Full: draws at full resolution at once when the last drawing was coarser
     (Reset draws low then high; the background work would take about 1 s) */
  QPushButton *reset_button = nullptr, *full_button = nullptr;
  for (QPushButton *button : mbvq::window(instance)->findChildren<QPushButton *>()) {
    if (button->text() == QStringLiteral("Reset"))
      reset_button = button;
    if (button->text() == QStringLiteral("Full"))
      full_button = button;
  }
  reset_button->click();
  {
    /* wait for the low or high drawing of the reset */
    QElapsedTimer wait;
    wait.start();
    while (mbvq_c_lastdrawrez(instance) == MBVQ_REZ_NONE && wait.elapsed() < 2000)
      spin(10);
  }
  const int before_full = mbvq_c_lastdrawrez(instance);
  full_button->click();
  QElapsedTimer full_timer;
  full_timer.start();
  while (mbvq_c_lastdrawrez(instance) != MBVQ_REZ_FULL && full_timer.elapsed() < 3000)
    spin(10);
  /* the background work would need about a second of rest: well before that */
  check(before_full != MBVQ_REZ_NONE && before_full != MBVQ_REZ_FULL && mbvq_c_lastdrawrez(instance) == MBVQ_REZ_FULL &&
            full_timer.elapsed() < 700,
        "Full draws at full resolution at once after a coarser drawing (before: " + std::to_string(before_full) + ", " +
            std::to_string(full_timer.elapsed()) + " ms, window exposed: " +
            std::to_string(view->window()->windowHandle() != nullptr && view->window()->windowHandle()->isExposed()) + ")");

  full_button->click();
  QString status_text;
  for (QLabel *label : mbvq::window(instance)->findChildren<QLabel *>())
    if (label->text() == QStringLiteral("Already at full resolution"))
      status_text = label->text();
  check(!status_text.isEmpty(), "Full at full resolution says so in the status line");

  /* a two-point pick (drag) gives a profile, drawn in its own OpenGL view */
  {
    const QPointF a(view->width() * 0.3, view->height() * 0.5), b(view->width() * 0.7, view->height() * 0.5);
    QMouseEvent press(QEvent::MouseButtonPress, a, view->mapToGlobal(a), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(view, &press);
    for (int k = 1; k <= 10; k++) {
      const QPointF p = a + (b - a) * (k / 10.0);
      QMouseEvent move(QEvent::MouseMove, p, view->mapToGlobal(p), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
      QApplication::sendEvent(view, &move);
    }
    QMouseEvent release(QEvent::MouseButtonRelease, b, view->mapToGlobal(b), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(view, &release);
    spin(200);
  }
  check(data->pick_type == MBV_PICK_TWOPOINT && data->profile.npoints > 1,
        "a drag makes a two-point pick with a profile of " + std::to_string(data->profile.npoints) + " points");
  auto *gl_window = static_cast<mbvq::MapWindow *>(mbvq::window(instance));
  gl_window->widget()->profile()->open();
  spin(300);
  if (const char *snapshot = std::getenv("MBVIEW_QT_TEST_PROFILE"))
    qobject_cast<QOpenGLWidget *>(gl_window->widget()->profile()->view())->grabFramebuffer().save(QString::fromLocal8Bit(snapshot));
  auto *profile_view = qobject_cast<QOpenGLWidget *>(gl_window->widget()->profile()->view());
  check(profile_view != nullptr && !profile_view->grabFramebuffer().isNull() && data->prwidth > 0 && data->prheight > 0,
        "the profile view draws (" + std::to_string(data->prwidth) + " x " + std::to_string(data->prheight) + " pixels)");

  /* closing the window removes the OpenGL view and frees the instance */
  mbvq::window(instance)->close();
  spin(300);
  check(mbvq::window(instance) == nullptr && !data->active, "closing the OpenGL window dismisses the instance");
  return failures == 0 ? 0 : 1;
}

}  // namespace

int main(int argc, char **argv) {
  /* line by line, so that results before a crash are not lost */
  std::setvbuf(stdout, nullptr, _IOLBF, 0);
  bool null = false, gl = false, show = false, mode3d = false, edit = false;
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
    else if (std::strcmp(argv[i], "--edit") == 0)
      edit = true;
    else
      grdfile = argv[i];
  }
  if (null + gl + show != 1) {
    std::fprintf(stderr, "usage: mbview_qt_test --null | --gl [--3d] | --show [--3d] [--edit] [grid]\n");
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
  if (edit) {
    /* sites, routes and polylines as mbgrdviz enables them */
    mbview_enableeditsites(0, instance, &error);
    mbview_enableeditroutes(0, instance, &error);
    mbview_enableviewpolylines(0, instance, &error);
  }
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
        /* with MBVIEW_QT_TEST_CLOSE set, close the window as a user would */
        if (std::getenv("MBVIEW_QT_TEST_CLOSE") != nullptr && mbvq::window(instance) != nullptr) {
          /* pick first: the pick framebuffer must be freed safely at exit */
          QWidget *view = mbvq::map_view(instance);
          click(view, QPointF(view->width() / 2.0, view->height() / 2.0));
          mbvq::window(instance)->close();
          /* MBVIEW_QT_TEST_CLOSE=exit: then leave as the macOS Quit menu does,
             with exit() from inside the event loop */
          if (std::strcmp(std::getenv("MBVIEW_QT_TEST_CLOSE"), "exit") == 0)
            std::exit(0);
        }
        else
          QApplication::quit();
      }
    });
    poll->start(50);
  }
  return app.exec();
}
