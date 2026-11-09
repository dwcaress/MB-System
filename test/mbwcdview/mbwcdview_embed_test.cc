/*--------------------------------------------------------------------
 *    The MB-system:  mbwcdview_embed_test.cc  9/11/2026
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
 * The tool as a widget inside another program's window, with no window of its
 * own: what a future unified application will use. Two of them in docks of one
 * main window. It reads what is drawn from the data of the plot, so it needs no
 * OpenGL. Arguments: the .kmall files of two copies of a line, each with its
 * .kmwcd beside it, and a directory to write in.
 */

#include <QApplication>
#include <QCheckBox>
#include <QDockWidget>
#include <QElapsedTimer>
#include <QMainWindow>
#include <QMenuBar>
#include <QSlider>
#include <QStatusBar>

#include <cstdio>
#include <cstring>
#include <initializer_list>

#include "WcdView.h"
#include "mb_define.h"
#include "mb_status.h"
#include "wcd_widget.h"

using wcd::WaterColumnWidget;

namespace {

int checks = 0, failures = 0;

#define CHECK(condition)                                  \
  do {                                                    \
    checks++;                                             \
    if (!(condition)) {                                   \
      failures++;                                         \
      printf("FAIL line %d: %s\n", __LINE__, #condition); \
    }                                                     \
  } while (0)

struct Notes {
  int opened = 0, failed = 0, busy_changes = 0, pings = 0, picks = 0, statuses = 0, diagnostics = 0;
  bool busy = false;
  QString opened_path, failed_path;
  int step = -1, ping = -1, ping_number = -1, swath = -1;
  double time_d = 0.0;
  QStringList pick;
};

void watch(WaterColumnWidget *v, Notes &n) {
  QObject::connect(v, &WaterColumnWidget::statusMessage, [&n](const QString &, int) { n.statuses++; });
  QObject::connect(v, &WaterColumnWidget::diagnostic, [&n](const QString &) { n.diagnostics++; });
  QObject::connect(v, &WaterColumnWidget::busyChanged, [&n](bool busy) { n.busy_changes++; n.busy = busy; });
  QObject::connect(v, &WaterColumnWidget::fileOpened, [&n](const QString &path) { n.opened++; n.opened_path = path; });
  QObject::connect(v, &WaterColumnWidget::fileOpenFailed, [&n](const QString &path) { n.failed++; n.failed_path = path; });
  QObject::connect(v, &WaterColumnWidget::pingChanged, [&n](int step, int ping, int number, int swath, double time_d) {
    n.pings++;
    n.step = step;
    n.ping = ping;
    n.ping_number = number;
    n.swath = swath;
    n.time_d = time_d;
  });
  QObject::connect(v, &WaterColumnWidget::pickAdded, [&n](const QStringList &row) { n.picks++; n.pick = row; });
}

void wait_for(std::initializer_list<WaterColumnWidget *> tools) {
  QElapsedTimer timer;
  timer.start();
  bool busy = true;
  while (busy && timer.elapsed() < 60000) {
    QApplication::processEvents(QEventLoop::AllEvents, 20);
    busy = false;
    for (WaterColumnWidget *v : tools)
      busy = busy || v->busy();
  }
  QApplication::processEvents();
}

const wcd::PlotData &data_of(WaterColumnWidget *v) { return v->findChild<wcd::WcdView *>()->plot().data(); }

/* NaN marks the cells with nothing to draw, so the values are compared as bytes */
bool same(const std::vector<float> &x, const std::vector<float> &y) {
  return x.size() == y.size() && std::memcmp(x.data(), y.data(), x.size() * sizeof(float)) == 0;
}

bool same_plot(const wcd::PlotData &x, const wcd::PlotData &y) {
  return same(x.across, y.across) && same(x.depth, y.depth) && same(x.amplitude, y.amplitude);
}

QCheckBox *box(WaterColumnWidget *v, const QString &text) {
  for (QCheckBox *b : v->findChildren<QCheckBox *>())
    if (b->text() == text)
      return b;
  return nullptr;
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "usage: mbwcdview_embed_test file1.kmall file2.kmall directory\n");
    return 2;
  }
  QApplication app(argc, argv);
  const QString a_path = argv[1], b_path = argv[2], dir = argv[3];

  /* what a program with several tools has to do */
  int error = MB_ERROR_NO_ERROR;
  mb_mem_list_disable(0, &error);

  /* one tool alone, for comparison */
  wcd::PlotData alone;
  {
    WaterColumnWidget solo;
    CHECK(solo.openFile(a_path));
    wait_for({&solo});
    CHECK(solo.loaded());
    alone = data_of(&solo);
    CHECK(!alone.across.empty());
  }

  /* two tools in docks of one main window */
  QMainWindow host;
  host.setWindowTitle("host");
  auto *a = new WaterColumnWidget;
  auto *b = new WaterColumnWidget;
  auto *dock_a = new QDockWidget("A");
  auto *dock_b = new QDockWidget("B");
  dock_a->setWidget(a);
  dock_b->setWidget(b);
  host.addDockWidget(Qt::LeftDockWidgetArea, dock_a);
  host.addDockWidget(Qt::RightDockWidgetArea, dock_b);
  host.resize(1800, 900);
  Notes na, nb;
  watch(a, na);
  watch(b, nb);
  /* never shown: on a platform without OpenGL, such as offscreen, Qt 6.4 crashes showing a window that holds an OpenGL widget */
  QApplication::processEvents();

  /* nothing of a window of its own */
  CHECK(host.findChild<QMenuBar *>() == nullptr);
  CHECK(host.findChild<QStatusBar *>() == nullptr);
  CHECK(!a->showTime(0.0));

  /* loading at the same time */
  CHECK(a->openFile(a_path));
  CHECK(b->openFile(b_path));
  CHECK(na.busy && nb.busy);
  CHECK(!a->openFile(b_path));
  wait_for({a, b});
  CHECK(na.opened == 1 && nb.opened == 1);
  CHECK(na.opened_path == a_path && nb.opened_path == b_path);
  CHECK(na.failed == 0);
  CHECK(na.busy_changes == 2 && !na.busy);
  CHECK(na.statuses > 0);
  CHECK(na.diagnostics == 2);  // the files of the line, and the summary
  CHECK(host.windowTitle() == "host");
  CHECK(same_plot(data_of(a), alone));
  CHECK(same_plot(data_of(b), alone));

  /* the first ping is reported */
  CHECK(na.pings == 1 && na.step == 0);
  const double first_time = na.time_d;
  auto *slider = a->findChild<QSlider *>();
  const int last = slider->maximum();
  CHECK(last > 2);

  /* moving one tool moves only that one */
  a->setStep(last);
  CHECK(na.pings == 2 && na.step == last);
  CHECK(nb.pings == 1 && nb.step == 0);
  CHECK(na.time_d > first_time && na.ping > 0);
  CHECK(na.ping_number > 0);
  const double last_time = na.time_d;
  CHECK(same_plot(data_of(b), alone));

  /* a redraw of the same ping is not reported */
  box(a, "Bottom")->click();
  CHECK(na.pings == 2);
  box(a, "Bottom")->click();

  /* moving from outside */
  CHECK(a->showTime(first_time));
  CHECK(na.step == 0 && na.time_d == first_time);
  CHECK(a->showTime(last_time));
  CHECK(na.time_d == last_time);
  CHECK(a->showTime(first_time + 0.4 * (last_time - first_time)));
  CHECK(na.time_d > first_time && na.time_d < last_time);
  const int pings_before = na.pings;
  CHECK(!a->showTime(first_time - 100.0));
  CHECK(!a->showTime(last_time + 100.0));
  CHECK(na.pings == pings_before);
  a->showPing(2);
  CHECK(na.ping == 2);

  /* a pick */
  a->setStep(0);
  const wcd::PlotData &data = data_of(a);
  size_t cell = 0;
  while (cell < data.amplitude.size() && !(data.amplitude[cell] == data.amplitude[cell] && data.across[cell] == data.across[cell] && data.depth[cell] == data.depth[cell]))
    cell++;
  CHECK(cell < data.amplitude.size());
  const QStringList row = a->pick(data.across[cell], data.depth[cell]);
  CHECK(na.picks == 1 && na.pick == row && nb.picks == 0);
  CHECK(a->findChild<wcd::PickDialog *>()->isVisible());
  CHECK(b->findChild<wcd::PickDialog *>() == nullptr);

  /* a file that cannot be read */
  const QString missing = dir + "/missing.kmall";
  CHECK(a->openFile(missing));
  wait_for({a});
  CHECK(na.failed == 1 && na.failed_path == missing);
  CHECK(na.opened == 1);
  CHECK(!a->loaded() && !a->showTime(first_time));
  CHECK(!na.busy);

  /* and the same tool opens a good one after it */
  CHECK(a->openFile(a_path));
  wait_for({a});
  CHECK(na.opened == 2 && a->loaded());
  CHECK(na.step == 0);

  /* removing a tool from the host with work under way waits for it */
  CHECK(a->openFile(a_path));
  CHECK(a->busy());
  delete dock_a;
  CHECK(same_plot(data_of(b), alone));

  printf("%d checks, %d failed\n", checks, failures);
  return failures != 0;
}
