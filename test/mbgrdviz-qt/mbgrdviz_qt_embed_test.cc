/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_qt_embed_test.cc  11/10/2026
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
 * mbgrdviz inside another program's window, with no window of its own: what
 * a future unified application will use. The host shows the menus of the
 * controller and puts each map in a dock. Runs without drawing (null
 * backend), for ctest.
 */

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QEventLoop>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QTimer>
#include <cstdio>
#include <string>
#include <vector>

#include "mbgrdviz_qt_controller.h"
#include "mbvq.h"
#include "mbvq_window.h"

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbgrdviz_core.h"
extern bool mbgrdviz_mbview_id[MBV_MAX_WINDOWS];
}

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

QAction *item(QMenuBar *bar, const char *text) {
  for (QAction *top : bar->actions())
    if (top->menu() != nullptr)
      for (QAction *a : top->menu()->actions())
        if (a->text() == QString::fromLatin1(text))
          return a;
  return nullptr;
}

}  // namespace

int main(int argc, char **argv) {
  std::setvbuf(stdout, nullptr, _IOLBF, 0);
  (void)argc;
  mbvq::set_backend(mbvq::Backend::Null);
  int qt_argc = 1;
  QApplication app(qt_argc, argv);

  QMainWindow host;
  host.setWindowTitle(QStringLiteral("host"));
  host.setCentralWidget(new QLabel(QStringLiteral("the program")));
  host.resize(1400, 900);
  host.show();

  /* the controller: its menus go into the bar of the host */
  mbgvq::Controller controller;
  controller.setDialogParent(&host);
  check(mbgvq::Controller::current() == &controller, "the controller of the program");
  QStringList names;
  for (QMenu *menu : controller.menus()) {
    names << menu->title();
    host.menuBar()->addMenu(menu);
  }
  check(names == QStringList({"File", "Realtime", "Help"}), "menus(): File, Realtime, Help");

  mbvq::start(0);
  for (int i = 0; i < MBV_MAX_WINDOWS; i++)
    mbgrdviz_mbview_id[i] = false;
  do_mbgrdviz_sensitivity();
  check(item(host.menuBar(), "Open Primary Grid")->isEnabled() && !item(host.menuBar(), "Open Site")->isEnabled(),
        "before a map: Open Primary Grid on, Open Site off");

  /* each map in a dock of the host */
  std::vector<QDockWidget *> docks;
  mbvq::set_map_placement([&](mbvq::MapWidget *map) {
    auto *dock = new QDockWidget(map->title().trimmed());
    dock->setWidget(map);
    host.addDockWidget(Qt::RightDockWidgetArea, dock);
    dock->show();
    docks.push_back(dock);
  });
  std::vector<size_t> opened, closed;
  QObject::connect(&controller, &mbgvq::Controller::mapOpened, [&](size_t i) { opened.push_back(i); });
  QObject::connect(&controller, &mbgvq::Controller::mapClosed, [&](size_t i) { closed.push_back(i); });

  const size_t first = controller.openPrimary(QString());
  spin(50);
  check(first != MBV_NO_WINDOW && docks.size() == 1 && mbvq::map_widget(first) != nullptr &&
            mbvq::map_widget(first)->window() == &host,
        "openPrimary: the test grid in a dock of the host");
  check(opened.size() == 1 && opened[0] == first, "mapOpened for it");
  check(item(host.menuBar(), "Open Site")->isEnabled(), "with a map: Open Site on");
  check(controller.openOverlay(first, QString()), "openOverlay on that map");
  check(!controller.openOverlay(7, QString()), "openOverlay on a map that is not open fails");

  const size_t second = controller.openPrimary(QString());
  spin(50);
  check(second != MBV_NO_WINDOW && second != first && docks.size() == 2, "a second map in another dock");

  /* the Action menu of a map starts other tools through the host */
  QString tool_seen;
  QStringList arguments_seen;
  controller.setToolOpener([&](const QString &tool, const QStringList &arguments) {
    tool_seen = tool;
    arguments_seen = arguments;
    return true;
  });
  controller.launch({QStringLiteral("mbnavedit"), QStringLiteral("-Ifile.mb-1")});
  check(tool_seen == QStringLiteral("mbnavedit") && arguments_seen == QStringList({"-Ifile.mb-1"}),
        "a tool goes to the tool opener of the host");

  /* the Dismiss menu of a map */
  mbvq::map_widget(first)->dismiss();
  spin(50);
  check(closed.size() == 1 && closed[0] == first && mbvq::map_widget(first) == nullptr, "a dismissed map: mapClosed");

  /* File > Quit asks the host; prepareToClose closes the maps */
  bool close_requested = false;
  QObject::connect(&controller, &mbgvq::Controller::closeRequested, [&] { close_requested = true; });
  item(host.menuBar(), "Quit")->trigger();
  check(close_requested && host.isVisible(), "Quit asks the host, which decides");
  check(controller.prepareToClose(), "prepareToClose");
  spin(50);
  check(closed.size() == 2 && closed[1] == second && mbvq::map_widget(second) == nullptr, "the other map closed: mapClosed");
  check(host.isVisible(), "the host stays");

  std::printf("%d failures\n", failures);
  return failures == 0 ? 0 : 1;
}
