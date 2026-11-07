/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_qt_test.cc  11/7/2026
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
 * Test of mbgrdviz-qt without drawing (null backend):
 *   mbgrdviz_qt_test <directory>
 *     the main window menu sensitivity; two instances of the built-in test
 *     grid; sites and a route saved, cleared, read and saved again (identical
 *     files apart from the date line); quitting.
 *   mbgrdviz_qt_test [--gl] <directory> <grid> <site file> <route file>
 *     also reads the site and route files into the grid and saves them as
 *     <directory>/site_saved.ste and route_saved.rte, to compare with the
 *     files the Motif mbgrdviz saved: the site file must be identical apart
 *     from the date line. A route read from a file is not the route the user
 *     picked: its waypoints get depths interpolated from the grid instead of
 *     the picked depths, and its segments are densified again. So the route
 *     check is that the waypoint positions are those of the Motif file, and
 *     that the route saved, cleared, read and saved again is identical.
 *     Saved routes hold bearings and distances in display coordinates, which
 *     only drawing sets up (mbview_projectdata in mbview_plot): --gl draws
 *     with the OpenGL backend (needs a display), as the Motif program does.
 *     Without drawing the route checks are skipped.
 */

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#include <QAction>
#include <QApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>
#include <QMenuBar>

#include "mbgrdviz_qt_window.h"
#include "mbvq_bridge.h"
#include "mbvq.h"

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbgrdviz_core.h"

extern int mbgrdviz_verbose;
extern bool mbgrdviz_mbview_id[MBV_MAX_WINDOWS];
int mbview_site_delete(size_t instance, int isite);
}

namespace {

int failures = 0;
int checks = 0;

void spin(int ms) {
  QEventLoop loop;
  QTimer::singleShot(ms, &loop, &QEventLoop::quit);
  loop.exec();
}

void check(bool ok, const std::string &what) {
  checks++;
  if (!ok)
    failures++;
  std::printf("%s: %s\n", ok ? "ok" : "FAILED", what.c_str());
}

QAction *action(MbgrdvizWindow &window, const char *text) {
  for (QAction *a : window.menuBar()->findChildren<QAction *>())
    if (a->text() == QString::fromLatin1(text) && a->menu() == nullptr)
      return a;
  return nullptr;
}

bool enabled(MbgrdvizWindow &window, const char *text) {
  QAction *a = action(window, text);
  return a != nullptr && a->isEnabled();
}

/* a file without its "## Run by user ... at <date>" line */
std::string body(const std::string &path) {
  std::ifstream in(path);
  std::ostringstream out;
  std::string line;
  while (std::getline(in, line))
    if (line.rfind("## Run by user", 0) != 0)
      out << line << '\n';
  return out.str();
}

int count_lines(const std::string &text, const char *prefix, bool skip_comments) {
  std::istringstream in(text);
  std::string line;
  int n = 0;
  while (std::getline(in, line)) {
    if (skip_comments && line.rfind("#", 0) == 0)
      continue;
    if (line.rfind(prefix, 0) == 0)
      n++;
  }
  return n;
}

/* the longitude and latitude fields of the waypoint lines of a route file */
std::string waypoints(const std::string &path) {
  std::ifstream in(path);
  std::ostringstream out;
  std::string line;
  while (std::getline(in, line))
    if (line.size() > 11 && line.compare(line.size() - 11, 11, "## WAYPOINT") == 0) {
      const size_t comma = line.find(',', line.find(',') + 1);
      out << line.substr(0, comma) << '\n';
    }
  return out.str();
}

/* sites and routes are shared by all instances: remove them all */
void clear_sites_routes() {
  int error = MB_ERROR_NO_ERROR;
  int nsite = 0;
  mbview_getsitecount(0, 0, &nsite, &error);
  for (int i = nsite - 1; i >= 0; i--)
    mbview_site_delete(0, i);
  mbview_deleteallroutes(0, 0, &error);
}

bool save_and_compare(int save_mode, int open_mode, const std::string &first, const std::string &second) {
  /* save from instance 0, clear, read into instance 1, save from instance 1 */
  std::string a = first, b = second;
  mbgrdviz_openfile(static_cast<size_t>(save_mode), 0, a.data());
  clear_sites_routes();
  mbgrdviz_openfile(static_cast<size_t>(open_mode), 1, a.data());
  mbgrdviz_openfile(static_cast<size_t>(save_mode), 1, b.data());
  const std::string body_a = body(first);
  return !body_a.empty() && body_a == body(second);
}

}  // namespace

int main(int argc, char **argv) {
  std::setvbuf(stdout, nullptr, _IOLBF, 0);
  const bool gl = argc > 1 && std::strcmp(argv[1], "--gl") == 0;
  if (gl) {
    argc--;
    argv++;
  }
  if (argc != 2 && argc != 5) {
    std::fprintf(stderr, "usage: mbgrdviz_qt_test [--gl] <directory> [<grid> <site file> <route file>]\n");
    return 2;
  }
  const std::string dir = argv[1];

  mbvq::set_backend(gl ? mbvq::Backend::LegacyGL : mbvq::Backend::Null);
  QApplication app(argc, argv);
  MbgrdvizWindow window;
  window.show();
  int error = MB_ERROR_NO_ERROR;
  mbgrdviz_verbose = 0;
  mbview_startup(0, nullptr, nullptr, &error);
  for (int i = 0; i < MBV_MAX_WINDOWS; i++)
    mbgrdviz_mbview_id[i] = false;
  do_mbgrdviz_sensitivity();

  /* before any grid: only Open Primary Grid */
  check(enabled(window, "Open Primary Grid") && !enabled(window, "Open Site") && !enabled(window, "Open Point") &&
            !enabled(window, "Save Site") && !enabled(window, "Save Route"),
        "menus before a grid is open");

  /* the built-in test grid, twice */
  do_mbgrdviz_openprimary(nullptr);
  do_mbgrdviz_openprimary(nullptr);
  check(mbgrdviz_mbview_id[0] && mbgrdviz_mbview_id[1] && mbvq::window(0) != nullptr && mbvq::window(1) != nullptr,
        "two test grid instances open");
  if (gl) {
    QElapsedTimer timer;
    timer.start();
    while ((mbvq_c_lastdrawrez(0) != MBVQ_REZ_FULL || mbvq_c_lastdrawrez(1) != MBVQ_REZ_FULL) && timer.elapsed() < 20000)
      spin(50);
    check(mbvq_c_lastdrawrez(0) == MBVQ_REZ_FULL && mbvq_c_lastdrawrez(1) == MBVQ_REZ_FULL,
          "test grids drawn at full resolution");
  }
  check(enabled(window, "Open Primary Grid") && enabled(window, "Open Site") && enabled(window, "Open Point") &&
            !enabled(window, "Save Site") && !enabled(window, "Save Route"),
        "menus with grids but no sites or routes");

  /* two sites and a route inside the test grid (-1..1 degrees) */
  double lon[3] = {-0.5, 0.25, 0.5};
  double lat[3] = {-0.25, 0.5, 0.25};
  double topo[3] = {0.0, 0.0, 0.0};
  int color[3] = {MBV_COLOR_GREEN, MBV_COLOR_RED, MBV_COLOR_GREEN};
  int size[3] = {1, 2, 1};
  mb_path name[3] = {"Site A", "Site B", "Site C"};
  mbview_addsites(0, 0, 2, lon, lat, topo, color, size, name, &error);
  int waypoint[3] = {MBV_ROUTE_WAYPOINT_SIMPLE, MBV_ROUTE_WAYPOINT_SIMPLE, MBV_ROUTE_WAYPOINT_SIMPLE};
  mb_path routename = "Route A";
  int iroute = 0;
  mbview_addroute(0, 0, 3, lon, lat, waypoint, MBV_COLOR_BLUE, 1, 0, routename, &iroute, &error);
  do_mbgrdviz_sensitivity();
  check(enabled(window, "Save Site") && enabled(window, "Save Route"), "Save Site and Save Route after adding");

  int nsite = 0, nroute = 0;
  mbview_getsitecount(0, 0, &nsite, &error);
  mbview_getroutecount(0, 0, &nroute, &error);
  check(nsite == 2 && nroute == 1, "two sites and one route added");
  /* the route file first: clearing deletes the sites too */
  check(save_and_compare(MBGRDVIZ_SAVEROUTE, MBGRDVIZ_OPENROUTE, dir + "/test_route_0.rte", dir + "/test_route_1.rte"),
        "route file saved, cleared, read and saved again is identical");
  check(count_lines(body(dir + "/test_route_0.rte"), "> ## STARTROUTE", false) == 1, "route file holds one route");
  mbview_addsites(0, 0, 2, lon, lat, topo, color, size, name, &error);
  check(save_and_compare(MBGRDVIZ_SAVESITE, MBGRDVIZ_OPENSITE, dir + "/test_site_0.ste", dir + "/test_site_1.ste"),
        "site file saved, cleared, read and saved again is identical");
  check(count_lines(body(dir + "/test_site_0.ste"), " ", true) == 2, "site file holds two sites");

  /* the Motif reference files */
  if (argc == 5) {
    do_mbgrdviz_openprimary(argv[2]);
    size_t instance = MBV_NO_WINDOW;
    for (size_t i = 0; i < MBV_MAX_WINDOWS && instance == MBV_NO_WINDOW; i++)
      if (i > 1 && mbgrdviz_mbview_id[i])
        instance = i;
    check(instance != MBV_NO_WINDOW, "grid opened");
    if (instance != MBV_NO_WINDOW) {
      clear_sites_routes();
      if (gl) {
        QElapsedTimer timer;
        timer.start();
        while (mbvq_c_lastdrawrez(instance) != MBVQ_REZ_FULL && timer.elapsed() < 20000)
          spin(50);
        check(mbvq_c_lastdrawrez(instance) == MBVQ_REZ_FULL, "grid drawn at full resolution");
      }
      std::string site = argv[3], route = argv[4];
      std::string site_out = dir + "/site_saved.ste", route_out = dir + "/route_saved.rte";
      mbgrdviz_openfile(MBGRDVIZ_OPENSITE, instance, site.data());
      mbgrdviz_openfile(MBGRDVIZ_OPENROUTE, instance, route.data());
      mbgrdviz_openfile(MBGRDVIZ_SAVESITE, instance, site_out.data());
      mbgrdviz_openfile(MBGRDVIZ_SAVEROUTE, instance, route_out.data());
      check(body(site_out) == body(site), "site file as the Motif mbgrdviz saved it");
      if (gl) {
        const std::string motif = waypoints(route);
        check(!motif.empty() && waypoints(route_out) == motif, "route waypoints at the positions of the Motif file");
        clear_sites_routes();
        std::string route_again = dir + "/route_saved_again.rte";
        mbgrdviz_openfile(MBGRDVIZ_OPENROUTE, instance, route_out.data());
        mbgrdviz_openfile(MBGRDVIZ_SAVEROUTE, instance, route_again.data());
        check(body(route_again) == body(route_out), "route saved, cleared, read and saved again is identical");
      }
    }
  }

  /* Quit closes all instances */
  window.close();
  bool none = true;
  for (int i = 0; i < MBV_MAX_WINDOWS; i++)
    none = none && !mbgrdviz_mbview_id[i];
  check(none, "Quit closes every instance");

  std::printf("%d of %d checks passed\n", checks - failures, checks);
  return failures == 0 ? 0 : 1;
}
