/*--------------------------------------------------------------------
 *    The MB-system:  mbnavedit_qt_canvas_test.cc  9/11/2026
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

/* Drives the plots of mbnavedit-qt with pointer and key events, without a screen. */

#include <cmath>
#include <cstdio>

#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>

#include "mb_status.h"
#include "mbne_canvas.h"

using namespace mbne;

namespace {

int failures = 0;

#define CHECK(condition)                                                    \
  do {                                                                      \
    if (!(condition)) {                                                     \
      printf("FAILED line %d: %s\n", __LINE__, #condition);                 \
      failures++;                                                           \
    }                                                                       \
  } while (0)

void mouse(QWidget &w, QEvent::Type type, const QPoint &at, Qt::MouseButton button, Qt::MouseButtons buttons) {
  QMouseEvent event(type, QPointF(at), QPointF(w.mapToGlobal(at)), button, buttons, Qt::NoModifier);
  QCoreApplication::sendEvent(&w, &event);
}

void click(QWidget &w, const QPoint &at, Qt::MouseButton button = Qt::LeftButton) {
  mouse(w, QEvent::MouseButtonPress, at, button, button);
  mouse(w, QEvent::MouseButtonRelease, at, button, Qt::NoButton);
}

void key(QWidget &w, int code, Qt::KeyboardModifiers modifiers = Qt::NoModifier) {
  QKeyEvent event(QEvent::KeyPress, code, modifiers);
  QCoreApplication::sendEvent(&w, &event);
}

QPoint point(const Session &s, int index, int type) {
  const Record &r = s.records()[index];
  return QPoint(r.pixel[2 * type], r.pixel[2 * type + 1]);
}

int selected(const Session &s, int type) {
  int n = 0;
  for (const Record &r : s.records())
    n += r.isSelected(type) ? 1 : 0;
  return n;
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s datafile\n", argv[0]);
    return 2;
  }
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication application(argc, argv);

  Session s;
  if (s.open(argv[1], 261, false, OUTPUT_BROWSE) != MB_SUCCESS) {
    printf("cannot open %s\n", argv[1]);
    return 1;
  }
  const int n = s.nbuff();
  CHECK(n >= 4);

  Canvas canvas(s);
  canvas.resize(900, 900);
  canvas.show();
  QCoreApplication::processEvents();
  CHECK(!s.plots().empty());

  /* every point is inside the frame of its plot */
  for (const Plot &p : s.plots()) {
    if (p.type >= EDITABLE_PLOTS)
      continue;
    for (int i = 0; i < n; i++) {
      const QPoint at = point(s, i, p.type);
      CHECK(at.x() >= p.ixmin && at.x() <= p.ixmax && at.y() >= p.iymax && at.y() <= p.iymin);
    }
  }

  /* Pick turns one point on and off */
  CHECK(canvas.mode() == Canvas::MODE_PICK);
  click(canvas, point(s, 1, PLOT_LONGITUDE));
  CHECK(s.records()[1].isSelected(PLOT_LONGITUDE));
  CHECK(selected(s, PLOT_LONGITUDE) == 1);
  click(canvas, point(s, 1, PLOT_LONGITUDE));
  CHECK(selected(s, PLOT_LONGITUDE) == 0);

  /* several picks are one step of undo, and only the undo takes them back */
  click(canvas, point(s, 0, PLOT_LONGITUDE));
  click(canvas, point(s, 1, PLOT_LONGITUDE));
  click(canvas, point(s, 2, PLOT_LONGITUDE));
  CHECK(selected(s, PLOT_LONGITUDE) == 3);
  CHECK(s.canUndo() && s.undoLabel() == "Selection");
  CHECK(!s.unsaved());
  s.undo();
  CHECK(selected(s, PLOT_LONGITUDE) == 0);
  CHECK(!s.canUndo());
  s.redo();
  CHECK(selected(s, PLOT_LONGITUDE) == 3);

  /* pick, flag, interpolate: each Undo goes back one of them, so nothing has to be cleared by hand */
  s.flag();
  CHECK(s.records()[1].lonlat_flag);
  s.interpolate();
  CHECK(s.undoLabel() == "Interpolate" || s.undoLabel() == "Flag");
  if (s.undoLabel() == "Interpolate")
    s.undo();
  CHECK(s.undoLabel() == "Flag");
  s.undo();
  CHECK(!s.records()[1].lonlat_flag);
  CHECK(selected(s, PLOT_LONGITUDE) == 3);
  CHECK(s.undoLabel() == "Selection");
  s.undo();
  CHECK(selected(s, PLOT_LONGITUDE) == 0);
  CHECK(!s.canUndo() && s.canRedo());

  /* toggling a point on and off leaves nothing to undo */
  s.deselectAll(PLOT_LONGITUDE);
  click(canvas, point(s, 1, PLOT_LONGITUDE));
  click(canvas, point(s, 1, PLOT_LONGITUDE));
  CHECK(selected(s, PLOT_LONGITUDE) == 0);
  CHECK(!s.canUndo());

  /* the keys of the modes, both hands */
  int reported = -1;
  QObject::connect(&canvas, &Canvas::modeChanged, [&reported](int mode) { reported = mode; });
  const struct {
    int code;
    Canvas::Mode mode;
  } keys[] = {{Qt::Key_U, Canvas::MODE_SELECT},       {Qt::Key_W, Canvas::MODE_SELECT},    {Qt::Key_I, Canvas::MODE_DESELECT},
              {Qt::Key_E, Canvas::MODE_DESELECT},     {Qt::Key_O, Canvas::MODE_SELECTALL}, {Qt::Key_R, Canvas::MODE_SELECTALL},
              {Qt::Key_P, Canvas::MODE_DESELECTALL},  {Qt::Key_T, Canvas::MODE_DESELECTALL}, {Qt::Key_Y, Canvas::MODE_PICK},
              {Qt::Key_Q, Canvas::MODE_PICK}};
  for (const auto &k : keys) {
    key(canvas, k.code);
    CHECK(canvas.mode() == k.mode);
    CHECK(reported == k.mode);
  }
  key(canvas, Qt::Key_U, Qt::ControlModifier);
  CHECK(canvas.mode() == Canvas::MODE_PICK);

  /* Select and Deselect */
  key(canvas, Qt::Key_U);
  click(canvas, point(s, 2, PLOT_LATITUDE));
  CHECK(s.records()[2].isSelected(PLOT_LATITUDE));
  key(canvas, Qt::Key_I);
  click(canvas, point(s, 2, PLOT_LATITUDE));
  CHECK(selected(s, PLOT_LATITUDE) == 0);

  /* a drag along the line takes every point, however fast the pointer moves */
  key(canvas, Qt::Key_U);
  mouse(canvas, QEvent::MouseButtonPress, point(s, 0, PLOT_LONGITUDE), Qt::LeftButton, Qt::LeftButton);
  mouse(canvas, QEvent::MouseMove, point(s, n - 1, PLOT_LONGITUDE), Qt::NoButton, Qt::LeftButton);
  mouse(canvas, QEvent::MouseButtonRelease, point(s, n - 1, PLOT_LONGITUDE), Qt::LeftButton, Qt::NoButton);
  CHECK(selected(s, PLOT_LONGITUDE) == n);

  /* a move after the release does nothing */
  key(canvas, Qt::Key_I);
  mouse(canvas, QEvent::MouseMove, point(s, 1, PLOT_LONGITUDE), Qt::NoButton, Qt::NoButton);
  CHECK(selected(s, PLOT_LONGITUDE) == n);

  /* Deselect All, and Select All in the plot under the pointer */
  key(canvas, Qt::Key_P);
  click(canvas, point(s, 0, PLOT_LONGITUDE));
  CHECK(selected(s, PLOT_LONGITUDE) == 0);
  key(canvas, Qt::Key_O);
  click(canvas, point(s, 0, PLOT_SPEED));
  CHECK(selected(s, PLOT_SPEED) == n);
  key(canvas, Qt::Key_P);
  click(canvas, point(s, 0, PLOT_SPEED));
  CHECK(selected(s, PLOT_SPEED) == 0);

  /* after a resize a click still lands on the point drawn there */
  key(canvas, Qt::Key_Y);
  canvas.resize(640, 1300);
  QCoreApplication::processEvents();
  click(canvas, point(s, n - 1, PLOT_HEADING));
  CHECK(s.records()[n - 1].isSelected(PLOT_HEADING));
  click(canvas, point(s, n - 1, PLOT_HEADING));
  CHECK(selected(s, PLOT_HEADING) == 0);

  /* right steps forward and middle back */
  const double total = s.records()[n - 1].file_time_d - s.records()[0].file_time_d;
  s.data_show_size = std::max((int)std::ceil(total / 2), 1);
  s.data_step_size = 1;
  s.start();
  canvas.layoutChanged();
  int views = 0;
  QObject::connect(&canvas, &Canvas::viewChanged, [&views]() { views++; });
  const double start = s.plotStartTime();
  const QPoint middle(canvas.width() / 2, canvas.height() / 2);
  click(canvas, middle, Qt::RightButton);
  CHECK(views == 1);
  CHECK(s.plotStartTime() > start);
  click(canvas, middle, Qt::MiddleButton);
  CHECK(views == 2);
  CHECK(std::fabs(s.plotStartTime() - start) < 1e-6);

  /* Pick Zoom: the left button for one edge and for the other, the right button to apply */
  s.showAll();
  canvas.layoutChanged();
  CHECK(s.nplot() == n);
  const int a = n / 4, b = n - 1 - n / 4;
  CHECK(a < b);
  canvas.setIntervalMode(true);
  click(canvas, point(s, a, PLOT_TINT));
  CHECK(s.intervalSet(0) && !s.intervalSet(1));
  click(canvas, point(s, b, PLOT_TINT));
  CHECK(s.intervalSet(1));
  CHECK(selected(s, PLOT_TINT) == 0);
  click(canvas, middle, Qt::RightButton);
  CHECK(!canvas.intervalMode());
  CHECK(!s.intervalSet(0) && !s.intervalSet(1));
  CHECK(s.nplot() < n);

  /* the middle button sets the second edge, and leaving the mode forgets the edges */
  s.showAll();
  canvas.layoutChanged();
  canvas.setIntervalMode(true);
  click(canvas, point(s, a, PLOT_TINT));
  click(canvas, point(s, b, PLOT_TINT), Qt::MiddleButton);
  CHECK(s.intervalSet(0) && s.intervalSet(1));
  canvas.setIntervalMode(false);
  CHECK(!s.intervalSet(0) && !s.intervalSet(1));

  /* roll and pitch are selected like the others, and heave is only shown */
  s.show.roll = s.show.pitch = s.show.heave = true;
  s.recountPlots();
  s.showAll();
  canvas.layoutChanged();
  key(canvas, Qt::Key_Y);
  click(canvas, point(s, 1, PLOT_ROLL));
  CHECK(s.records()[1].isSelected(PLOT_ROLL));
  click(canvas, point(s, 1, PLOT_PITCH));
  CHECK(s.records()[1].isSelected(PLOT_PITCH));
  CHECK(!s.records()[1].isSelected(PLOT_ROLL));
  bool heave_found = false;
  for (const Plot &p : s.plots()) {
    if (p.type != PLOT_HEAVE)
      continue;
    heave_found = true;
    click(canvas, QPoint((p.ixmin + p.ixmax) / 2, (p.iymin + p.iymax) / 2));
    CHECK(selected(s, PLOT_PITCH) == 0);
    int any = 0;
    for (const Record &r : s.records())
      any += r.selected != 0 ? 1 : 0;
    CHECK(any == 0);
  }
  CHECK(heave_found);
  s.show.roll = s.show.pitch = s.show.heave = false;
  s.recountPlots();
  canvas.layoutChanged();

  /* the plots follow the palette of the application */
  QPalette light = canvas.palette();
  light.setColor(QPalette::Base, Qt::white);
  canvas.setPalette(light);
  CHECK(canvas.grab().toImage().pixelColor(0, 0).lightness() > 200);
  QPalette dark = canvas.palette();
  dark.setColor(QPalette::Base, QColor(30, 30, 30));
  canvas.setPalette(dark);
  CHECK(canvas.grab().toImage().pixelColor(0, 0).lightness() < 60);

  s.closeDiscard();
  /* with nothing open the plots are empty, and a click does nothing */
  canvas.layoutChanged();
  click(canvas, middle);
  click(canvas, middle, Qt::RightButton);
  CHECK(canvas.grab().toImage().pixelColor(0, 0).lightness() < 60);

  printf("%s\n", failures == 0 ? "ok" : "FAILED");
  return failures == 0 ? 0 : 1;
}
