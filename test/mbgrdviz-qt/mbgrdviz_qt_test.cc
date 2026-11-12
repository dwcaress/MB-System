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
 *   mbgrdviz_qt_test [--gl] <directory> <grid> <site file> <route file> [<risi script>]
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
 *     Without drawing the route checks are skipped. --gl also writes all 12
 *     route exports, and compares the RISI script without heading with
 *     <risi script>, saved by the Motif mbgrdviz from the same route. With --gl it also picks an
 *     area and generates a survey route through the survey dialog, and picks
 *     a region and opens it as a new view.
 *   Both forms also check which programs the Action menu starts: the Qt
 *   versions of mbedit, mbeditviz and mbvelocitytool, found in the CMake build
 *   tree.
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <QAction>
#include <QApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QEventLoop>
#include <QTimer>
#include <QLabel>
#include <QMenuBar>
#include <QPushButton>

#include "mbgrdviz_qt_bridge.h"
#include "mbgrdviz_qt_controller.h"
#include "mbgrdviz_qt_dialogs.h"
#include "mbgrdviz_qt_window.h"
#include "mbvq_bridge.h"
#include "mbvq.h"

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbgrdviz_core.h"
/* mbview_area() and mbview_region(), the area and region picks */
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"
#include "mbviewprivate.h"

extern int mbgrdviz_verbose;
extern int mbgrdviz_working_route;
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

/* an item of the menus shown in the menu bar */
QAction *action(MbgrdvizWindow &window, const char *text) {
  for (QAction *top : window.menuBar()->actions())
    if (top->menu() != nullptr)
      for (QAction *a : top->menu()->actions())
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
  if (argc != 2 && argc != 5 && argc != 6) {
    std::fprintf(stderr, "usage: mbgrdviz_qt_test [--gl] <directory> [<grid> <site file> <route file> [<risi script>]]\n");
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
  if (argc >= 5) {
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

        /* every route export of the Motif File menu of the route list */
        const int exports[] = {MBGRDVIZ_SAVEROUTEREVERSED,     MBGRDVIZ_SAVERISISCRIPTHEADING,
                               MBGRDVIZ_SAVERISISCRIPTNOHEADING, MBGRDVIZ_SAVERISI2SCRIPTHEADING,
                               MBGRDVIZ_SAVERISI2SCRIPTNOHEADING, MBGRDVIZ_SAVEDEGDECMIN,
                               MBGRDVIZ_SAVELNW,               MBGRDVIZ_SAVEGREENSEAYML,
                               MBGRDVIZ_SAVETECDISLST,         MBGRDVIZ_SAVEKONGSBERGDP,
                               MBGRDVIZ_SAVESISASCIIPLAN1,     MBGRDVIZ_SAVESISASCIIPLAN2};
        int written = 0;
        for (int mode : exports) {
          std::string out = dir + "/route_export_" + std::to_string(mode);
          mbgrdviz_openfile(static_cast<size_t>(mode), instance, out.data());
          std::ifstream in(out);
          if (in.peek() != std::ifstream::traits_type::eof())
            written++;
        }
        check(written == 12, "all 12 route exports written (" + std::to_string(written) + ")");

        /* the RISI script without heading, as the Motif mbgrdviz saved it from
           the picked route: equal apart from the date and the number of route
           points (a route read from a file is densified again). The route file
           stores longitude and latitude with 6 decimals (0.11 m), so the north
           and east offsets of the POS lines agree within 0.15 m */
        if (argc == 6) {
          const auto lines = [](const std::string &path) {
            std::ifstream in(path);
            std::vector<std::string> out;
            std::string line;
            while (std::getline(in, line))
              if (line.rfind("## Run by user", 0) != 0 && line.rfind("## ROUTEPOINTS", 0) != 0)
                out.push_back(line);
            return out;
          };
          const std::vector<std::string> mine =
              lines(dir + "/route_export_" + std::to_string(MBGRDVIZ_SAVERISISCRIPTNOHEADING));
          const std::vector<std::string> motif = lines(argv[5]);
          bool same = !mine.empty() && mine.size() == motif.size();
          double largest = 0.0;
          for (size_t i = 0; same && i < mine.size(); i++) {
            if (mine[i] == motif[i])
              continue;
            double a[5], b[5];
            same = mine[i].rfind("POS,", 0) == 0 && motif[i].rfind("POS,", 0) == 0 &&
                   std::sscanf(mine[i].c_str(), "POS, %lf, %lf, %lf, %lf, %lf", &a[0], &a[1], &a[2], &a[3], &a[4]) == 5 &&
                   std::sscanf(motif[i].c_str(), "POS, %lf, %lf, %lf, %lf, %lf", &b[0], &b[1], &b[2], &b[3], &b[4]) == 5 &&
                   a[2] == b[2] && a[3] == b[3] && a[4] == b[4];
            for (int k = 0; same && k < 2; k++)
              largest = std::max(largest, std::fabs(a[k] - b[k]));
          }
          char text[64];
          std::snprintf(text, sizeof(text), "%.3f", largest);
          check(same && largest < 0.15,
                std::string("RISI script as the Motif mbgrdviz saved it (POS offsets within ") + text + " m)");
        }

        /* a survey from an area picked across the middle of the view */
        struct mbview_struct *data = nullptr;
        mbview_getdataptr(0, instance, &data, &error);
        const int x0 = data->width * 2 / 5, x1 = data->width * 3 / 5, y = data->height / 2;
        mbview_area(instance, MBV_AREALENGTH_DOWN, x0, y);
        mbview_area(instance, MBV_AREALENGTH_MOVE, x1, y);
        mbview_area(instance, MBV_AREALENGTH_UP, x1, y);
        check(data->area_type == MBV_AREA_QUAD, "area picked");
        clear_sites_routes();
        do_mbgrdviz_make_survey(nullptr, reinterpret_cast<XtPointer>(instance), nullptr);
        SurveyDialog *dialog = SurveyDialog::instance(nullptr);
        check(dialog->isVisible(), "survey dialog shown");
        QLabel *info = nullptr;
        for (QLabel *label : dialog->findChildren<QLabel *>())
          if (label->text().startsWith(QStringLiteral("Current Area")))
            info = label;
        check(info != nullptr && info->text().contains(QStringLiteral("Length:")), "survey dialog shows the area");
        struct mbgrdviz_qt_survey survey;
        mbgrdviz_qt_survey_get(&survey);
        survey.linespacing = 500;
        survey.crosslines = 1;
        snprintf(survey.name, sizeof(survey.name), "Test survey");
        mbgrdviz_qt_survey_set(&survey);
        mbgrdviz_qt_survey_generate();
        int nsurvey = 0;
        mbview_getroutecount(0, instance, &nsurvey, &error);
        check(nsurvey == 1 && mbgrdviz_working_route == 0, "Generate Route made one route");
        check(info != nullptr && info->text().contains(QStringLiteral("Test survey")), "survey dialog shows the new route");
        std::string survey_out = dir + "/survey_saved.rte";
        mbgrdviz_openfile(MBGRDVIZ_SAVEROUTE, instance, survey_out.data());
        const std::string survey_text = body(survey_out);
        int waypoints_saved = 0;
        {
          std::istringstream in(survey_text);
          std::string line;
          while (std::getline(in, line))
            if (line.size() > 21 && line.compare(line.size() - 21, 21, "## WAYPOINT STARTLINE") == 0)
              waypoints_saved++;
        }
        check(survey_text.find("> ## STARTROUTE") != std::string::npos &&
                  survey_text.find("Test survey") != std::string::npos && waypoints_saved > 0,
              "survey route saved with its name and " + std::to_string(waypoints_saved) + " line waypoints");
        QPushButton *dismiss = nullptr;
        for (QPushButton *button : dialog->findChildren<QPushButton *>())
          if (button->text() == QStringLiteral("Dismiss"))
            dismiss = button;
        if (dismiss != nullptr)
          dismiss->click();
        check(!dialog->isVisible() && mbgrdviz_working_route == -1, "Dismiss closes the dialog and ends the route");

        /* a region opened as a new view */
        mbview_region(instance, MBV_REGION_DOWN, x0, data->height * 2 / 5);
        mbview_region(instance, MBV_REGION_MOVE, x1, data->height * 3 / 5);
        mbview_region(instance, MBV_REGION_UP, x1, data->height * 3 / 5);
        check(data->region_type == MBV_REGION_QUAD, "region picked");
        int before = 0, after = 0;
        for (int i = 0; i < MBV_MAX_WINDOWS; i++)
          before += mbgrdviz_mbview_id[i] ? 1 : 0;
        do_mbgrdviz_open_region(nullptr, reinterpret_cast<XtPointer>(instance), nullptr);
        for (int i = 0; i < MBV_MAX_WINDOWS; i++)
          after += mbgrdviz_mbview_id[i] ? 1 : 0;
        check(after == before + 1, "Open Region as New View opens a new instance");
      }
    }
  }

  /* the tools that the Action menu starts: the Qt versions of mbedit,
     mbeditviz and mbvelocitytool, found beside mbgrdviz-qt in the CMake build
     tree */
  const QString program_dir = QCoreApplication::applicationDirPath() + QStringLiteral("/../../src/mbgrdviz-qt");
  const auto tool_is = [&](const char *tool, const char *expected) {
    const QString found = mbgvq::Controller::findTool(QString::fromLatin1(tool), program_dir);
    return QFileInfo(found).completeBaseName() == QString::fromLatin1(expected);
  };
  check(tool_is("mbedit", "mbedit-qt"), "Open mbedit starts mbedit-qt");
  check(tool_is("mbvelocitytool", "mbvelocitytool-qt"), "Open mbvelocitytool starts mbvelocitytool-qt");
  check(tool_is("mbeditviz", "mbeditviz-qt"), "Open mbeditviz starts mbeditviz-qt");
  check(mbgvq::Controller::findTool(QStringLiteral("mbgrdviz_no_such_tool"), program_dir).isEmpty(),
        "a missing tool is not found");

  /* Quit closes all instances */
  window.close();
  bool none = true;
  for (int i = 0; i < MBV_MAX_WINDOWS; i++)
    none = none && !mbgrdviz_mbview_id[i];
  check(none, "Quit closes every instance");

  std::printf("%d of %d checks passed\n", checks - failures, checks);
  return failures == 0 ? 0 : 1;
}
