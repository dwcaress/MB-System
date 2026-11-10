/*--------------------------------------------------------------------
 *    The MB-system:  mbnavedit_qt_window_test.cc  9/11/2026
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

/* Drives the window of mbnavedit-qt without a screen: the buttons, menus and dialogs, and the
   questions about saving. Arguments: two swath files and a datalist of them, which are edited,
   so they must be scratch copies. With MBNAVEDIT_TEST_MBPROCESS set it runs mbprocess on saving. */

#include <cmath>
#include <cstdio>
#include <functional>
#include <string>

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSlider>
#include <QTableWidget>
#include <QThread>
#include <QTimer>
#include <QWheelEvent>

#include "mb_status.h"
#include "mbne_window.h"

using namespace mbne;

namespace {

int failures = 0;

#define CHECK(condition)                                    \
  do {                                                      \
    if (!(condition)) {                                     \
      printf("FAILED line %d: %s\n", __LINE__, #condition); \
      failures++;                                           \
    }                                                       \
  } while (0)

bool wait_until(const std::function<bool()> &condition, int ms = 20000) {
  QElapsedTimer timer;
  timer.start();
  while (!condition() && timer.elapsed() < ms) {
    QApplication::processEvents(QEventLoop::AllEvents, 20);
    QThread::msleep(2);
  }
  return condition();
}

/* answers the next question of the program */
void answer(QMessageBox::StandardButton button) {
  auto *timer = new QTimer;
  timer->setInterval(5);
  QObject::connect(timer, &QTimer::timeout, [timer, button] {
    auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
    if (box == nullptr)
      return;
    timer->stop();
    timer->deleteLater();
    box->button(button)->click();
  });
  timer->start();
}

void mouse(QWidget &w, QEvent::Type type, const QPoint &at, Qt::MouseButton button, Qt::MouseButtons buttons) {
  QMouseEvent event(type, QPointF(at), QPointF(w.mapToGlobal(at)), button, buttons, Qt::NoModifier);
  QCoreApplication::sendEvent(&w, &event);
}

void click(QWidget &w, const QPoint &at, Qt::MouseButton button = Qt::LeftButton) {
  mouse(w, QEvent::MouseButtonPress, at, button, button);
  mouse(w, QEvent::MouseButtonRelease, at, button, Qt::NoButton);
}

void key(QWidget &w, int code) {
  QKeyEvent event(QEvent::KeyPress, code, Qt::NoModifier);
  QCoreApplication::sendEvent(&w, &event);
}

QPoint point(const Session &s, int index, int type) {
  const Record &r = s.records()[index];
  return QPoint(r.pixel[2 * type], r.pixel[2 * type + 1]);
}

template <class T> T *child(QWidget &w, const char *name) {
  T *found = w.findChild<T *>(name);
  if (found == nullptr) {
    printf("FAILED: no widget %s\n", name);
    failures++;
    exit(1);
  }
  return found;
}

QAction *menu_action(NavEditWidget *editor, const QString &start) {
  for (QMenu *menu : editor->menus())
    for (QAction *action : menu->actions())
      if (action->text().startsWith(start))
        return action;
  printf("FAILED: no menu item %s\n", start.toUtf8().constData());
  failures++;
  exit(1);
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "usage: %s file1 file2 datalist\n", argv[0]);
    return 2;
  }
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication application(argc, argv);
  const QString first = argv[1], second = argv[2];

  NavEditWindow window;
  NavEditWidget *editor = window.editor();
  Session &s = window.session();
  Canvas *canvas = window.canvas();
  if (qEnvironmentVariableIsSet("MBNAVEDIT_TEST_MBPROCESS"))
    editor->setRunMbprocess(true);
  editor->addInput(argv[3], -1);
  window.show();
  CHECK(window.windowTitle() == "mbnavedit-qt");
  CHECK(child<QLabel>(window, "position")->text() == "No file open");
  CHECK(!child<QPushButton>(window, "interpolate")->isEnabled());

  /* the first file of the datalist */
  editor->loadFirstFile();
  CHECK(editor->busy());
  CHECK(wait_until([&] { return !editor->busy(); }));
  CHECK(s.isOpen());
  CHECK(QString(s.file()) == first);
  const int n = s.nbuff();
  CHECK(n >= 4);
  CHECK(window.windowTitle().contains("[*]"));
  CHECK(!window.isWindowModified());
  CHECK(child<QPushButton>(window, "interpolate")->isEnabled());
  CHECK(child<QLabel>(window, "position")->text() == QString("Records 1 to %1 of %1").arg(n));

  /* file list: both files of the datalist, the first one loaded */
  menu_action(editor, "File Selection List")->trigger();
  auto *table = child<QTableWidget>(*editor, "filelist");
  CHECK(table->rowCount() == 2);
  CHECK(table->item(0, 0)->text() == "<loaded>");
  CHECK(table->item(1, 2)->text() == second);
  child<QPushButton>(*editor, "filelist_dismiss")->click();

  /* an edit from the keyboard, the plots and a button, and its undo */
  key(*canvas, Qt::Key_U);
  CHECK(child<QRadioButton>(window, "mode_select")->isChecked());
  click(*canvas, point(s, 2, PLOT_DRAFT));
  CHECK(s.records()[2].isSelected(PLOT_DRAFT));
  CHECK(menu_action(editor, "Undo")->text() == "Undo Selection");
  CHECK(!window.isWindowModified());  // a selection is not an edit
  s.recordsForTest()[2].draft = 90.0;
  child<QPushButton>(window, "interpolate")->click();
  CHECK(s.records()[2].draft < 80.0);
  CHECK(window.isWindowModified());
  CHECK(menu_action(editor, "Undo")->text() == "Undo Interpolate");
  menu_action(editor, "Undo")->trigger();
  CHECK(s.records()[2].draft == 90.0);
  CHECK(!window.isWindowModified());
  /* the next Undo takes back the selection, and Redo brings both back */
  CHECK(s.records()[2].isSelected(PLOT_DRAFT));
  CHECK(menu_action(editor, "Undo")->text() == "Undo Selection");
  menu_action(editor, "Undo")->trigger();
  CHECK(!s.records()[2].isSelected(PLOT_DRAFT));
  CHECK(!menu_action(editor, "Undo")->isEnabled());
  CHECK(!window.isWindowModified());
  menu_action(editor, "Redo")->trigger();
  CHECK(s.records()[2].isSelected(PLOT_DRAFT));
  menu_action(editor, "Redo")->trigger();
  CHECK(s.records()[2].draft < 80.0);
  child<QRadioButton>(window, "mode_deselectall")->click();
  CHECK(canvas->mode() == Canvas::MODE_DESELECTALL);
  key(*canvas, Qt::Key_Y);
  CHECK(child<QRadioButton>(window, "mode_pick")->isChecked());

  /* time stepping, the buttons and the slider */
  menu_action(editor, "Time Stepping")->trigger();
  auto *span = child<ScaleControl>(*editor, "time_span");
  auto *step = child<ScaleControl>(*editor, "time_step");
  const int half = std::max((int)std::ceil(0.5 * (s.records()[n - 1].file_time_d - s.records()[0].file_time_d)), 1);
  span->findChild<QDoubleSpinBox *>()->setValue(half);
  step->findChild<QDoubleSpinBox *>()->setValue(1);
  CHECK(s.data_show_size == half);
  CHECK(s.data_step_size == 1);
  CHECK(s.nplot() < n);
  child<QPushButton>(*editor, "timestepping_dismiss")->click();
  const double begin = s.plotStartTime();
  child<QPushButton>(window, "forward")->click();
  CHECK(s.plotStartTime() > begin);
  child<QPushButton>(window, "reverse")->click();
  CHECK(std::fabs(s.plotStartTime() - begin) < 1e-6);
  auto *slider = child<QSlider>(window, "time_slider");
  CHECK(slider->maximum() == n - 1);
  slider->setValue(1);
  CHECK(s.current() == 1);
  child<QPushButton>(window, "end")->click();
  CHECK(slider->value() == s.current());
  child<QPushButton>(window, "start")->click();
  CHECK(s.current() == 0);
  CHECK(slider->value() == 0);

  /* Pick Zoom from its button, which comes up again when it is applied */
  child<QPushButton>(window, "show_all")->click();
  CHECK(s.nplot() == n);
  auto *zoom = child<QPushButton>(window, "pick_zoom");
  zoom->click();
  CHECK(canvas->intervalMode());
  click(*canvas, point(s, n / 4, PLOT_TINT));
  click(*canvas, point(s, n - 1 - n / 4, PLOT_TINT));
  click(*canvas, point(s, 0, PLOT_TINT), Qt::RightButton);
  CHECK(!canvas->intervalMode());
  CHECK(!zoom->isChecked());
  CHECK(s.nplot() < n);
  child<QPushButton>(window, "show_all")->click();

  /* the plots shown */
  auto *vru = window.findChild<QAction *>("show_vru");
  CHECK(vru != nullptr && !vru->isChecked());
  vru->trigger();
  CHECK(s.numberPlots() == 9);
  CHECK(canvas->minimumHeight() == 9 * MIN_PLOT_HEIGHT);
  vru->trigger();
  CHECK(s.numberPlots() == 6);
  /* each plot keeps its height, and the wheel then moves the plots and not the time */
  auto *plots = child<QScrollArea>(window, "plots");
  CHECK(wait_until([&] { return canvas->height() == std::max(plots->viewport()->height(), 6 * MIN_PLOT_HEIGHT); }, 3000));
  for (const Plot &p : s.plots())
    CHECK(p.iymin < canvas->height());
  CHECK(canvas->height() > plots->viewport()->height());
  const int wheel_record = s.current();
  QWheelEvent wheel(QPointF(100, 100), QPointF(canvas->mapToGlobal(QPoint(100, 100))), QPoint(0, 0), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
  QCoreApplication::sendEvent(canvas, &wheel);
  CHECK(s.current() == wheel_record);
  CHECK(!wheel.isAccepted());  // left for the scroll area
  window.findChild<QAction *>("show_lon_org")->trigger();
  CHECK(!s.show.lon_org);
  window.findChild<QAction *>("show_lon_org")->trigger();

  /* the model */
  menu_action(editor, "Nav Modeling")->trigger();
  CHECK(!s.show.lon_dr);
  child<QComboBox>(*editor, "model_mode")->setCurrentIndex(2);
  CHECK(s.model_mode == MODEL_DR);
  CHECK(s.show.lon_dr && s.show.lat_dr);
  CHECK(window.findChild<QAction *>("show_lon_dr")->isChecked());
  child<QComboBox>(*editor, "model_mode")->setCurrentIndex(0);
  CHECK(s.model_mode == MODEL_OFF);
  child<QPushButton>(*editor, "model_dismiss")->click();

  /* the time tools have nothing to do with good times */
  menu_action(editor, "Time Interpolation")->trigger();
  CHECK(!child<QPushButton>(*editor, "dialog_time_interpolation_apply")->isEnabled());
  child<QPushButton>(*editor, "dialog_time_interpolation_dismiss")->click();
  CHECK(!child<QPushButton>(*editor, "dialog_delete_bad_apply")->isEnabled());

  /* the position offset */
  const double lon = s.records()[1].lon;
  menu_action(editor, "Position Offset")->trigger();
  child<QDoubleSpinBox>(*editor, "offset_lon")->setValue(0.01);
  child<QPushButton>(*editor, "offset_apply")->click();
  CHECK(std::fabs(s.records()[1].lon - lon - 0.01) < 1e-9);
  menu_action(editor, "Undo")->trigger();
  CHECK(std::fabs(s.records()[1].lon - lon) < 1e-9);
  child<QPushButton>(*editor, "offset_dismiss")->click();

  /* despike */
  s.recordsForTest()[1].draft = 200.0;
  menu_action(editor, "Despike")->trigger();
  child<QDoubleSpinBox>(*editor, "despike_window")->setValue(100000.0);
  child<QDoubleSpinBox>(*editor, "despike_threshold")->setValue(5.0);
  child<QPushButton>(*editor, "despike_interpolate")->click();
  CHECK(s.show.draft_org);  // nothing was selected by the filter yet
  child<QPushButton>(*editor, "despike_select")->click();
  CHECK(child<QLabel>(*editor, "despike_result")->text().startsWith("2 records") ||
        child<QLabel>(*editor, "despike_result")->text().startsWith("1 record"));
  CHECK(child<QLabel>(*editor, "despike_result")->text().contains("Largest distance"));
  CHECK(s.records()[1].isSelected(PLOT_DRAFT));
  child<QPushButton>(*editor, "despike_interpolate")->click();
  CHECK(s.records()[1].draft < 150.0);
  CHECK(!s.show.draft_org);
  CHECK(!window.findChild<QAction *>("show_draft_org")->isChecked());
  {
    /* the range now follows the depths that are left */
    double top = 0.0;
    for (const Plot &p : s.plots())
      if (p.type == PLOT_DRAFT)
        top = std::max(p.ymax, p.ymin);
    CHECK(top < 150.0);
  }
  window.findChild<QAction *>("show_draft_org")->trigger();
  CHECK(s.show.draft_org);
  child<QPushButton>(*editor, "despike_dismiss")->click();

  /* closing with edits asks, and Cancel keeps everything */
  CHECK(window.isWindowModified());
  answer(QMessageBox::Cancel);
  CHECK(!window.close());
  CHECK(window.isVisible());
  CHECK(s.isOpen());

  /* Save in the menu: at the end of the file; the file stays open */
  CHECK(menu_action(editor, "Save")->isEnabled());
  CHECK(editor->save());
  CHECK(!window.isWindowModified());
  CHECK(QFile::exists(first + ".nve"));
  CHECK(!menu_action(editor, "Save")->isEnabled());

  /* Done goes to the next file without asking */
  s.recordsForTest()[3].draft = 100.0;
  key(*canvas, Qt::Key_U);
  click(*canvas, point(s, 3, PLOT_DRAFT));
  child<QPushButton>(window, "interpolate")->click();
  child<QPushButton>(window, "done")->click();
  CHECK(wait_until([&] { return !editor->busy() && s.isOpen() && QString(s.file()) == second; }));

  /* opening a file with saved edits asks whether to use them */
  answer(QMessageBox::Yes);
  CHECK(editor->openFile(first, 261));
  CHECK(wait_until([&] { return !editor->busy() && s.isOpen() && QString(s.file()) == first; }));
  CHECK(s.records()[3].draft < 80.0);
  CHECK(s.show.draft);

  /* an edit, and Discard on leaving it for another file */
  s.recordsForTest()[3].draft = 150.0;
  key(*canvas, Qt::Key_U);
  click(*canvas, point(s, 3, PLOT_DRAFT));
  child<QPushButton>(window, "interpolate")->click();
  CHECK(window.isWindowModified());
  QFile nve(first + ".nve");
  CHECK(nve.open(QIODevice::ReadOnly));
  const QByteArray before = nve.readAll();
  nve.close();
  answer(QMessageBox::Discard);
  CHECK(editor->openFile(second, 261));
  CHECK(wait_until([&] { return !editor->busy() && s.isOpen() && QString(s.file()) == second; }));
  CHECK(nve.open(QIODevice::ReadOnly));
  CHECK(nve.readAll() == before);
  nve.close();

  /* Quit with edits and Save: the window closes when they are written */
  s.recordsForTest()[2].draft = 77.0;
  click(*canvas, point(s, 2, PLOT_DRAFT));
  child<QPushButton>(window, "interpolate")->click();
  CHECK(window.isWindowModified());
  answer(QMessageBox::Save);
  child<QPushButton>(window, "quit")->click();
  CHECK(wait_until([&] { return !window.isVisible(); }));
  CHECK(!s.isOpen());
  CHECK(QFile::exists(second + ".nve"));
  CHECK(!QFile::exists(second + ".nve.new"));

  printf("%s\n", failures == 0 ? "ok" : "FAILED");
  return failures == 0 ? 0 : 1;
}
