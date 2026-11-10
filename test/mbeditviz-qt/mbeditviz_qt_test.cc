/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_qt_test.cc  11/8/2026
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
 * Test of mbeditviz-qt without drawing (null backend):
 *   mbeditviz_qt_test              the windows, menus, buttons and dialogs
 *                                  without data (for ctest)
 *   mbeditviz_qt_test [--gl] --data LIST  also with swath data from the datalist
 *                                  LIST, in Browse mode (nothing is written):
 *                                  the file list and status, View All, the
 *                                  grid parameters dialog, Apply (grid and
 *                                  map view), Generate New Grid, and Quit,
 *                                  which unloads and unlocks the files;
 *                                  with --gl (OpenGL backend, needs a display)
 *                                  also an area picked in the map opens the 3D
 *                                  soundings view with the soundings inside it
 *   mbeditviz_qt_test --parity-qt LIST EDITS N [OFFSET]
 *                                  edit parity, Qt side (OpenGL, needs a
 *                                  display): Edit mode, View All, Apply, an
 *                                  area pick, then N mouse clicks in Toggle
 *                                  mode on soundings of the 3D view, Quit;
 *                                  writes each edit made (file, ping, beam,
 *                                  new flag) to EDITS
 *   mbeditviz_qt_test --parity-ref LIST EDITS
 *                                  edit parity, reference side (no drawing):
 *                                  the same files in Edit mode, View All,
 *                                  Apply, then the edits of EDITS given
 *                                  straight to mbeditviz_mb3dsoundings_edit(),
 *                                  which the Motif soundings window calls too,
 *                                  then Quit
 *   Run the two on fresh copies of the same data; their .esf and .esf.stream
 *   files must be identical.
 */

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QMouseEvent>
#include <QMenuBar>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QTimer>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <cstring>
#include <cmath>
#include <string>

#include "mbeditviz_qt_bridge.h"
#include "mbvq_bridge.h"
#include "mbvq_soundings.h"
#include "mbeditviz_qt_window.h"
#include "mbvq.h"

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbv_xtypes.h"
#include "mbview.h"
#include "mbeditviz.h"
/* mbview_area(), the area pick of the mouse */
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"
#include "mbviewprivate.h"
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

QAction *menu_action(QMainWindow *window, const char *menu, const char *text) {
  for (QAction *top : window->menuBar()->actions())
    if (top->text() == QString::fromLatin1(menu) && top->menu() != nullptr)
      for (QAction *action : top->menu()->actions())
        if (action->text() == QString::fromLatin1(text))
          return action;
  return nullptr;
}

QPushButton *button(QWidget *window, const QString &text) {
  for (QPushButton *b : window->findChildren<QPushButton *>())
    if (b->text() == text)
      return b;
  return nullptr;
}

QRadioButton *radio(QWidget *window, const char *text) {
  for (QRadioButton *b : window->findChildren<QRadioButton *>())
    if (b->text() == QString::fromLatin1(text))
      return b;
  return nullptr;
}

QString label_containing(QWidget *window, const char *text) {
  for (QLabel *label : window->findChildren<QLabel *>())
    if (label->text().contains(QString::fromLatin1(text)))
      return label->text();
  return QString();
}

QDialog *visible_dialog(QWidget *window, const char *title) {
  for (QDialog *d : window->findChildren<QDialog *>())
    if (d->windowTitle() == QString::fromLatin1(title) && d->isVisible())
      return d;
  return nullptr;
}

/* the Motif button labels, converted as the window converts them */
const QString kViewAll = mbvq::label_text(":::t\"View\":t\"All\"\"Files\"");
const QString kViewSelected = mbvq::label_text(":::t\"View\":t\"Selected\"\"Files\"");
const QString kRemove = mbvq::label_text(":::t\"Remove\":t\"Selected\"\"Files\"");

}  // namespace

/* open the datalist, Edit mode, View All, Apply: files loaded and gridded */
bool parity_open(MbeditvizWindow &window, const char *datalist) {
  char program[] = "mbeditviz-qt", help[] = "", usage[] = "";
  char opt_i[] = "-I";
  std::string list = datalist;
  char *args[] = {program, opt_i, list.data(), nullptr};
  mbeditviz_init(3, args, program, help, usage, &do_mbeditviz_message_on, &do_mbeditviz_message_off,
                 &do_mbeditviz_update_gui, &do_error_dialog);
  spin(200);
  mbeditviz_qt_set_edit_mode(true);
  window.show_mode();
  button(&window, kViewAll)->click();
  spin(200);
  QDialog *grid = visible_dialog(&window, "Grid Parameters");
  if (grid == nullptr)
    return false;
  button(grid, QStringLiteral("Apply"))->click();
  spin(300);
  return mbev_grid.status == MBEV_GRID_VIEWED && mbev_num_files_loaded == mbev_num_files;
}

struct Edit {
  int ifile, iping, ibeam, flag;
};

/* the Qt side: real mouse clicks in Toggle mode on soundings of the 3D view */
int parity_qt(const char *datalist, const char *edits_file, int count, int offset) {
  int qt_argc = 1;
  char name[] = "mbeditviz_qt_test";
  char *qt_argv[] = {name, nullptr};
  QApplication app(qt_argc, qt_argv);
  MbeditvizWindow window;
  window.show();
  mbeditviz_qt_start(&window);
  check(parity_open(window, datalist), "Edit mode, View All and Apply load and grid the files");

  /* an area across the middle of the map opens the 3D soundings view */
  QElapsedTimer timer;
  timer.start();
  while (mbvq_c_lastdrawrez(0) != MBVQ_REZ_FULL && timer.elapsed() < 20000)
    spin(50);
  struct mbview_struct *data = nullptr;
  int error = MB_ERROR_NO_ERROR;
  mbview_getdataptr(0, 0, &data, &error);
  mbview_area(0, MBV_AREALENGTH_DOWN, data->width * 2 / 5, data->height / 2);
  mbview_area(0, MBV_AREALENGTH_MOVE, data->width * 3 / 5, data->height / 2);
  mbview_area(0, MBV_AREALENGTH_UP, data->width * 3 / 5, data->height / 2);
  spin(500);
  mbvq::SoundingsWindow *soundings = mbvq::soundings_window();
  QWidget *view = soundings != nullptr ? soundings->view() : nullptr;
  check(view != nullptr && mbev_selected.num_soundings > 0,
        "the soundings view opens with " + std::to_string(mbev_selected.num_soundings) + " soundings");
  if (view == nullptr || mbev_selected.num_soundings == 0)
    return 1;
  timer.restart();
  bool drawn = false;
  while (!drawn && timer.elapsed() < 20000) {
    for (int i = 0; i < mbev_selected.num_soundings && !drawn; i++)
      drawn = mbev_selected.soundings[i].winx != 0 || mbev_selected.soundings[i].winy != 0;
    spin(50);
  }

  /* clicks on soundings spread over the selection; each click must change
     exactly one sounding, which is recorded */
  const double ratio = view->devicePixelRatioF();
  const int width = static_cast<int>(std::lround(view->width() * ratio));
  const int height = static_cast<int>(std::lround(view->height() * ratio));
  std::vector<Edit> edits;
  std::vector<char> before(static_cast<size_t>(mbev_selected.num_soundings));
  const int n = mbev_selected.num_soundings;
  for (int k = 0; k < count; k++) {
    int i = (offset + (k * 9973) + k * n / count) % n;
    /* the next sounding inside the view */
    int tries = 0;
    while (tries < n && (mbev_selected.soundings[i].winx < 5 || mbev_selected.soundings[i].winx > width - 5 ||
                         mbev_selected.soundings[i].winy < 5 || mbev_selected.soundings[i].winy > height - 5)) {
      i = (i + 1) % n;
      tries++;
    }
    for (int j = 0; j < n; j++)
      before[j] = mbev_selected.soundings[j].beamflag;
    const QPointF position(mbev_selected.soundings[i].winx / ratio, (height - 1 - mbev_selected.soundings[i].winy) / ratio);
    QMouseEvent press(QEvent::MouseButtonPress, position, view->mapToGlobal(position), Qt::LeftButton, Qt::LeftButton,
                      Qt::NoModifier);
    QApplication::sendEvent(view, &press);
    QMouseEvent release(QEvent::MouseButtonRelease, position, view->mapToGlobal(position), Qt::LeftButton,
                        Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(view, &release);
    spin(150);
    int changed = 0;
    for (int j = 0; j < n; j++)
      if (mbev_selected.soundings[j].beamflag != before[j]) {
        changed++;
        const auto &s = mbev_selected.soundings[j];
        edits.push_back({s.ifile, s.iping, s.ibeam, static_cast<unsigned char>(s.beamflag)});
      }
    if (changed != 1)
      std::printf("      click %d changed %d soundings\n", k, changed);
  }
  check(static_cast<int>(edits.size()) == count, std::to_string(edits.size()) + " soundings edited by " +
                                                     std::to_string(count) + " clicks");
  std::ofstream out(edits_file);
  for (const Edit &e : edits)
    out << e.ifile << ' ' << e.iping << ' ' << e.ibeam << ' ' << e.flag << '\n';
  out.close();

  /* Dismiss, then Quit: the edits are written */
  mbvq_c_soundings_dismiss();
  window.close();
  spin(300);
  check(mbev_num_files_loaded == 0, "Quit unloads the files");
  std::printf("%d failures\n", failures);
  return failures == 0 ? 0 : 1;
}

/* the reference side: the recorded edits given straight to the edit function */
int parity_ref(const char *datalist, const char *edits_file) {
  int qt_argc = 1;
  char name[] = "mbeditviz_qt_test";
  char *qt_argv[] = {name, nullptr};
  QApplication app(qt_argc, qt_argv);
  MbeditvizWindow window;
  window.show();
  mbeditviz_qt_start(&window);
  check(parity_open(window, datalist), "Edit mode, View All and Apply load and grid the files");
  std::ifstream in(edits_file);
  Edit e;
  int count = 0;
  while (in >> e.ifile >> e.iping >> e.ibeam >> e.flag) {
    mbeditviz_mb3dsoundings_edit(e.ifile, e.iping, e.ibeam, static_cast<char>(e.flag), MB3DSDG_EDIT_FLUSH);
    count++;
  }
  check(count > 0, std::to_string(count) + " edits replayed");
  window.close();
  spin(300);
  check(mbev_num_files_loaded == 0, "Quit unloads the files");
  std::printf("%d failures\n", failures);
  return failures == 0 ? 0 : 1;
}

int main(int argc, char **argv) {
  std::setvbuf(stdout, nullptr, _IOLBF, 0);
  if (argc >= 5 && std::strcmp(argv[1], "--parity-qt") == 0) {
    mbvq::set_backend(mbvq::Backend::LegacyGL);
    return parity_qt(argv[2], argv[3], std::atoi(argv[4]), argc > 5 ? std::atoi(argv[5]) : 0);
  }
  if (argc == 4 && std::strcmp(argv[1], "--parity-ref") == 0) {
    mbvq::set_backend(mbvq::Backend::Null);
    return parity_ref(argv[2], argv[3]);
  }
  const char *datalist = nullptr;
  const bool gl = argc > 1 && std::strcmp(argv[1], "--gl") == 0;
  if (gl) {
    argc--;
    argv++;
  }
  if (argc == 3 && std::strcmp(argv[1], "--data") == 0)
    datalist = argv[2];
  else if (argc != 1 || gl) {
    std::fprintf(stderr, "usage: mbeditviz_qt_test [[--gl] --data datalist]\n");
    return 2;
  }
  mbvq::set_backend(gl ? mbvq::Backend::LegacyGL : mbvq::Backend::Null);
  int qt_argc = 1;
  QApplication app(qt_argc, argv);
  MbeditvizWindow window;
  window.show();
  mbeditviz_qt_start(&window);

  /* the window as Motif: menus, mode, buttons, status */
  check(window.windowTitle() == QStringLiteral("MBeditviz"), "the window title is that of Motif");
  check(menu_action(&window, "File", "Open Swath Data") != nullptr && menu_action(&window, "File", "Quit") != nullptr &&
            menu_action(&window, "Options", "Generate New Grid") != nullptr &&
            menu_action(&window, "Help", "About") != nullptr,
        "File, Options and Help menus with the Motif items");
  check(button(&window, kViewAll) != nullptr && button(&window, kViewSelected) != nullptr &&
            button(&window, kRemove) != nullptr,
        "the View All, View Selected and Remove buttons");
  check(radio(&window, "Edit Bathymetry") != nullptr && radio(&window, "Edit Bathymetry")->isChecked(),
        "Edit Bathymetry is the starting mode");
  check(label_containing(&window, "Available Files: 0").contains(QStringLiteral("Grid Not Generated")),
        "the status shows no files and no grid");
  if (button(&window, kViewAll) == nullptr) {
    std::printf("%d failures\n", failures + 1);
    return 1;
  }
  check(!button(&window, kViewAll)->isEnabled() && menu_action(&window, "File", "Open Swath Data")->isEnabled(),
        "without files only opening is possible");

  /* the mode: in the main window only with files (Motif), always in the open
     dialog, and shared between both */
  check(!radio(&window, "Browse Only")->isEnabled(), "without files the main window mode buttons are disabled");
  menu_action(&window, "File", "Open Swath Data")->trigger();
  QDialog *open = visible_dialog(&window, "Open Swath Data");
  check(open != nullptr, "File > Open Swath Data shows the open dialog");
  if (open != nullptr) {
    radio(open, "Browse Only")->click();
    check(!mbeditviz_qt_edit_mode() && radio(&window, "Browse Only")->isChecked(),
          "Browse Only in the open dialog sets the mode, the main window shows it");
    open->hide();
  }

  if (datalist != nullptr) {
    /* open the datalist as -I does */
    char program[] = "mbeditviz-qt", help[] = "", usage[] = "";
    char opt_i[] = "-I";
    std::string list = datalist;
    char *args[] = {program, opt_i, list.data(), nullptr};
    mbeditviz_init(3, args, program, help, usage, &do_mbeditviz_message_on, &do_mbeditviz_message_off,
                   &do_mbeditviz_update_gui, &do_error_dialog);
    spin(200);
    const int files = mbev_num_files;
    check(files > 0 && window.list()->count() == files,
          "the file list shows the " + std::to_string(files) + " files of the datalist");
    check(files > 0 && window.list()->item(0)->text().contains(QStringLiteral(".mb")),
          "a list row names its file: " + (files > 0 ? window.list()->item(0)->text().toStdString() : std::string()));
    const QString status = label_containing(&window, "Available Files");
    check(status.contains(QStringLiteral("Available Files: %1").arg(files)) && status.contains(QStringLiteral("Loaded Files: 0")),
          "the status counts the files, none loaded");
    check(button(&window, kViewAll)->isEnabled() && !menu_action(&window, "Options", "Generate New Grid")->isEnabled(),
          "with files the view buttons are enabled");

    /* View All loads the files and asks for the grid parameters */
    button(&window, kViewAll)->click();
    spin(200);
    QDialog *grid = visible_dialog(&window, "Grid Parameters");
    check(grid != nullptr, "View All shows the grid parameters dialog");
    check(mbev_num_files_loaded == files &&
              label_containing(&window, "Loaded Files").contains(QStringLiteral("Loaded Files: %1").arg(files)),
          "View All loads all files");
    if (grid != nullptr) {
      check(label_containing(grid, "Suggested Grid Parameters").contains(QStringLiteral("Cell Size")),
            "the dialog shows the grid bounds and suggested parameters");
      QSlider *cell = grid->findChild<QSlider *>();
      const int columns = mbev_grid_n_columns;
      cell->setValue(cell->value() * 2);
      check(mbev_grid_n_columns < columns && label_containing(grid, "Selected Grid Parameters")
                                                 .contains(QStringLiteral("Dimensions: %1").arg(mbev_grid_n_columns)),
            "a doubled cell size shrinks the grid (" + std::to_string(columns) + " to " +
                std::to_string(mbev_grid_n_columns) + " columns)");

      /* Apply makes the grid and opens the map view */
      button(grid, QStringLiteral("Apply"))->click();
      spin(300);
      check(mbev_grid.status == MBEV_GRID_VIEWED && mbvq::window(0) != nullptr, "Apply makes the grid and opens the map");
      check(label_containing(&window, "Grid:").contains(QStringLiteral("Dimensions:")), "the status shows the grid");
      check(!button(&window, kViewAll)->isEnabled() && menu_action(&window, "Options", "Generate New Grid")->isEnabled(),
            "with a grid only Generate New Grid is possible");

      /* an area picked in the map opens the 3D soundings view */
      if (gl) {
        QElapsedTimer timer;
        timer.start();
        while (mbvq_c_lastdrawrez(0) != MBVQ_REZ_FULL && timer.elapsed() < 20000)
          spin(50);
        struct mbview_struct *data = nullptr;
        int error = MB_ERROR_NO_ERROR;
        mbview_getdataptr(0, 0, &data, &error);
        const int x0 = data->width * 2 / 5, x1 = data->width * 3 / 5, y = data->height / 2;
        mbview_area(0, MBV_AREALENGTH_DOWN, x0, y);
        mbview_area(0, MBV_AREALENGTH_MOVE, x1, y);
        mbview_area(0, MBV_AREALENGTH_UP, x1, y);
        spin(500);
        check(data->area_type == MBV_AREA_QUAD && mbev_selected.num_soundings > 0 &&
                  mbvq::soundings_window() != nullptr && mbvq::soundings_window()->isVisible(),
              "an area pick opens the soundings view with " + std::to_string(mbev_selected.num_soundings) +
                  " soundings");
      }

      /* Generate New Grid asks for the grid parameters again */
      menu_action(&window, "Options", "Generate New Grid")->trigger();
      spin(200);
      grid = visible_dialog(&window, "Grid Parameters");
      check(grid != nullptr, "Generate New Grid shows the grid parameters dialog");
      if (grid != nullptr)
        button(grid, QStringLiteral("Dismiss"))->click();
    }

    /* Quit unloads the files and removes their locks */
    window.close();
    spin(200);
    check(mbev_num_files_loaded == 0, "Quit unloads the files");
    const QDir dir = QFileInfo(QString::fromLocal8Bit(datalist)).absoluteDir();
    check(dir.entryList({QStringLiteral("*.lck")}).isEmpty(), "no lock files are left");
  }

  std::printf("%d failures\n", failures);
  return failures == 0 ? 0 : 1;
}
