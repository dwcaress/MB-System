/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_qt_embed_test.cc  11/10/2026
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
 * mbeditviz inside another program's window, with no window of its own: what
 * a future unified application will use. The host shows the menus of the
 * controller, puts the file list in a dock and the map in another. Runs
 * without drawing (null backend):
 *   mbeditviz_qt_embed_test             without data (for ctest)
 *   mbeditviz_qt_embed_test --data LIST also with the swath data of the
 *                                       datalist LIST, in Browse mode
 *                                       (nothing is written): View Selected,
 *                                       the map in its dock, Generate New
 *                                       Grid and prepareToClose
 */

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QDockWidget>
#include <QEventLoop>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QRadioButton>
#include <QTimer>
#include <cstdio>
#include <cstring>
#include <string>

#include "mbeditviz_qt_controller.h"
#include "mbeditviz_qt_filelist.h"
#include "mbvq.h"
#include "mbvq_window.h"

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbv_xtypes.h"
#include "mbview.h"
#include "mbeditviz.h"
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

QRadioButton *radio(QWidget *widget, const char *text) {
  for (QRadioButton *b : widget->findChildren<QRadioButton *>())
    if (b->text() == QString::fromLatin1(text))
      return b;
  return nullptr;
}

QDialog *visible_dialog(QWidget *window, const char *title) {
  for (QDialog *d : window->findChildren<QDialog *>())
    if (d->windowTitle() == QString::fromLatin1(title) && d->isVisible())
      return d;
  return nullptr;
}

QPushButton *button(QWidget *widget, const QString &text) {
  for (QPushButton *b : widget->findChildren<QPushButton *>())
    if (b->text() == text)
      return b;
  return nullptr;
}

}  // namespace

int main(int argc, char **argv) {
  std::setvbuf(stdout, nullptr, _IOLBF, 0);
  const char *datalist = nullptr;
  if (argc == 3 && std::strcmp(argv[1], "--data") == 0)
    datalist = argv[2];
  else if (argc != 1) {
    std::fprintf(stderr, "usage: mbeditviz_qt_embed_test [--data datalist]\n");
    return 2;
  }
  mbvq::set_backend(mbvq::Backend::Null);
  int qt_argc = 1;
  QApplication app(qt_argc, argv);

  QMainWindow host;
  host.setWindowTitle(QStringLiteral("host"));
  host.setCentralWidget(new QLabel(QStringLiteral("the program")));
  host.resize(1400, 900);
  host.show();

  /* the controller: its menus go into the bar of the host, its dialogs to the
     host; a host reads no command line */
  mbevq::SwathController controller;
  controller.setDialogParent(&host);
  check(mbevq::SwathController::current() == &controller, "the controller of the program");
  controller.init();
  QStringList names;
  for (QMenu *menu : controller.menus()) {
    names << menu->title();
    host.menuBar()->addMenu(menu);
  }
  check(names == QStringList({"File", "Options", "Help"}), "menus(): File, Options, Help");
  check(controller.status().contains(QStringLiteral("Available Files: 0")) &&
            controller.status().contains(QStringLiteral("Grid Not Generated")),
        "status(): no files and no grid");
  check(controller.editMode() && controller.openEnabled() && !controller.filesEnabled() &&
            item(host.menuBar(), "Open Swath Data")->isEnabled(),
        "Edit mode; only opening is possible");

  /* the file list in a dock of the host */
  auto *list_dock = new QDockWidget(QStringLiteral("Swath Files"));
  auto *files = new mbevq::FileListWidget(&controller);
  list_dock->setWidget(files);
  host.addDockWidget(Qt::LeftDockWidgetArea, list_dock);
  list_dock->show();
  check(files->window() == &host && files->list()->count() == 0, "the file list in a dock of the host, empty");
  check(radio(files, "Edit Bathymetry")->isChecked() && !radio(files, "Browse Only")->isEnabled(),
        "it shows the mode, disabled without files");

  /* the mode is shared between the controller, the file list and the open
     dialog, which belongs to the host */
  int modes = 0;
  QObject::connect(&controller, &mbevq::SwathController::modeChanged, [&] { modes++; });
  controller.setEditMode(false);
  check(modes == 1 && !controller.editMode() && radio(files, "Browse Only")->isChecked(),
        "setEditMode: modeChanged, the file list follows");
  item(host.menuBar(), "Open Swath Data")->trigger();
  QDialog *open = visible_dialog(&host, "Open Swath Data");
  check(open != nullptr && radio(open, "Browse Only")->isChecked(), "Open Swath Data: a dialog of the host in the mode");
  if (open != nullptr) {
    radio(open, "Output Edits")->click();
    check(controller.editMode() && radio(files, "Edit Bathymetry")->isChecked(),
          "the mode set in the dialog reaches the file list");
    radio(open, "Browse Only")->click();
    open->hide();
  }
  check(!controller.openFile(QString()), "openFile without a path fails");

  /* the map in a dock of the host */
  QDockWidget *map_dock = nullptr;
  mbvq::set_map_placement([&](mbvq::MapWidget *map) {
    map_dock = new QDockWidget(map->title().trimmed());
    map_dock->setWidget(map);
    host.addDockWidget(Qt::RightDockWidgetArea, map_dock);
    map_dock->show();
  });

  if (datalist != nullptr) {
    int lists = 0;
    QObject::connect(&controller, &mbevq::SwathController::filesChanged, [&] { lists++; });
    check(controller.openFile(QString::fromLocal8Bit(datalist)), "openFile: the datalist");
    const int count = static_cast<int>(controller.files().size());
    check(count > 0 && count == mbev_num_files && lists > 0 && files->list()->count() == count,
          "filesChanged: the file list shows the " + std::to_string(count) + " files");
    check(controller.filesEnabled() && radio(files, "Browse Only")->isEnabled() && !controller.editMode(),
          "with files: the buttons and the mode are enabled, still Browse");

    /* View Selected loads the selected file */
    files->list()->item(count - 1)->setSelected(true);
    check(controller.selectedRows() == QList<int>({count - 1}), "the selection of the file list reaches the controller");
    controller.viewSelected();
    spin(200);
    QDialog *grid = visible_dialog(&host, "Grid Parameters");
    check(grid != nullptr && mbev_num_files_loaded == 1 && mbev_files[count - 1].load_status,
          "viewSelected: only that file loaded, the grid parameters dialog of the host");
    if (grid != nullptr) {
      button(grid, QStringLiteral("Apply"))->click();
      spin(300);
      check(mbev_grid.status == MBEV_GRID_VIEWED && map_dock != nullptr && mbvq::map_widget(0) != nullptr &&
                mbvq::map_widget(0)->window() == &host,
            "Apply: the grid and its map, in a dock of the host");
      check(controller.status().contains(QStringLiteral("Dimensions:")) && !controller.filesEnabled() &&
                item(host.menuBar(), "Generate New Grid")->isEnabled(),
            "with a grid: the status shows it, only Generate New Grid is possible");

      /* Generate New Grid closes the map and asks again */
      controller.updateGrid();
      spin(200);
      grid = visible_dialog(&host, "Grid Parameters");
      check(grid != nullptr && mbvq::map_widget(0) == nullptr && mbev_grid.status == MBEV_GRID_NONE,
            "updateGrid: the map closed, the grid parameters dialog again");
      if (grid != nullptr)
        button(grid, QStringLiteral("Dismiss"))->click();
    }

    /* the files are unloaded and unlocked, the host stays */
    check(controller.prepareToClose(), "prepareToClose");
    spin(200);
    check(mbev_num_files_loaded == 0 && host.isVisible(), "the files are unloaded, the host stays");
    const QDir dir = QFileInfo(QString::fromLocal8Bit(datalist)).absoluteDir();
    check(dir.entryList({QStringLiteral("*.lck")}).isEmpty(), "no lock files are left");
  }

  /* File > Quit asks the host */
  bool close_requested = false;
  QObject::connect(&controller, &mbevq::SwathController::closeRequested, [&] { close_requested = true; });
  item(host.menuBar(), "Quit")->trigger();
  check(close_requested && host.isVisible(), "Quit asks the host, which decides");
  check(controller.prepareToClose() && host.isVisible(), "prepareToClose; the host stays");

  std::printf("%d failures\n", failures);
  return failures == 0 ? 0 : 1;
}
