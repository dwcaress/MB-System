/*--------------------------------------------------------------------
 *    The MB-system:  mbedit_qt_window_test.cc  2/11/2026
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
 * Drives the main window of mbedit-qt with mouse and key events: the edit modes,
 * the key macros, the slider, resizing, light and dark palettes, the dialogs and
 * the question when the window closes. Arguments: a swath file and a second one.
 * They are edited, so they must be scratch copies.
 */

#include <cmath>
#include <cstdio>
#include <functional>
#include <string>

#include <QApplication>
#include <QAbstractButton>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPalette>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSplitter>
#include <QStatusBar>
#include <QTableWidget>
#include <QTimer>
#include <QWheelEvent>

#include "mbe_window.h"
#include "mb_define.h"
#include "mb_status.h"

using mbe::Canvas;
using MainWindow = mbe::EditorWindow;
using mbe::ScaleControl;
using mbe::SuspectStrip;
using mbe::TimeSeriesPanel;
using mbe::TrackMap;

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

void mouse(QWidget *w, QEvent::Type type, const QPoint &p, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers mods = Qt::NoModifier) {
  QMouseEvent event(type, QPointF(p), QPointF(w->mapToGlobal(p)), button, buttons, mods);
  QApplication::sendEvent(w, &event);
}

void click(QWidget *w, const QPoint &p, Qt::MouseButton button = Qt::LeftButton) {
  mouse(w, QEvent::MouseButtonPress, p, button, button);
  mouse(w, QEvent::MouseButtonRelease, p, button, Qt::NoButton);
}

void key(QWidget *w, char c, QEvent::Type type = QEvent::KeyPress) {
  const QString text(QChar::fromLatin1(c));
  QKeyEvent event(type, QKeySequence(text)[0].key(), Qt::NoModifier, text);
  QApplication::sendEvent(w, &event);
}

void tap(QWidget *w, char c) {
  key(w, c);
  key(w, c, QEvent::KeyRelease);
}

void wait_for(MainWindow &window) {
  QElapsedTimer timer;
  timer.start();
  QApplication::processEvents();
  while (window.busy() && timer.elapsed() < 60000)
    QApplication::processEvents(QEventLoop::AllEvents, 20);
  QApplication::processEvents();
}

/* answers the next message box with the given button */
void answer(QMessageBox::StandardButton button, bool *asked = nullptr) {
  QTimer::singleShot(50, [button, asked] {
    auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
    if (box != nullptr) {
      if (asked != nullptr)
        *asked = true;
      if (box->button(button) != nullptr)
        box->button(button)->click();
    }
  });
}

/* lets an answer that was not needed expire, so that it cannot answer a later question */
void expire_answers() {
  QElapsedTimer timer;
  timer.start();
  while (timer.elapsed() < 100)
    QApplication::processEvents(QEventLoop::AllEvents, 10);
}

QPoint where(MainWindow &window, int i, int j) {
  const mbe::Ping &p = window.session().pings()[i];
  return QPoint(p.bath_x[j], p.bath_y[j]);
}

char flag(MainWindow &window, int i, int j) { return window.session().pings()[i].beamflag[j]; }

/* a beam in the view with a good sounding */
bool good_beam(MainWindow &window, int i, int *beam) {
  const mbe::Ping &p = window.session().pings()[i];
  for (int j = p.beams_bath / 4; j < p.beams_bath; j++) {
    if (mb_beam_ok(p.beamflag[j])) {
      *beam = j;
      return true;
    }
  }
  return false;
}

template <typename T> T *named(MainWindow &window, const char *name) { return window.findChild<T *>(name); }

QAction *menu_action(MainWindow &window, const QString &text) {
  for (QAction *top : window.menuBar()->actions())
    for (QAction *action : top->menu()->actions())
      if (action->text() == text)
        return action;
  return nullptr;
}

void open_file(MainWindow &window, bool use_saved = false) {
  window.editor()->loadFirstFile(use_saved);
  wait_for(window);
}

}  // namespace

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "usage: %s swathfile swathfile2\n", argv[0]);
    return 2;
  }
  QApplication app(argc, argv);
  const std::string file = argv[1], file2 = argv[2];
  const QString esf = QString::fromStdString(file) + ".esf";

  MainWindow window;
  window.editor()->addInput(file, 0);
  window.show();
  QApplication::processEvents();
  open_file(window);
  mbe::Session &s = window.session();
  Canvas *canvas = window.canvas();
  auto *slider = named<QSlider>(window, "ping_slider");
  CHECK(s.isOpen());
  CHECK(s.nbuff() > 20);
  CHECK(slider->maximum() == s.nbuff() - 1);
  canvas->setFocus();

  /* every item of the menus of mbedit */
  for (const char *text :
       {"Open...", "File Selection List...", "Waterfall View", "Alongtrack View", "Acrosstrack View", "Show Flagged Soundings", "Show Flagged Profile",
        "Show Flag States", "Show Bottom Detect Algorithms", "Show Source Pulse Types", "Wide Bathymetry Profiles", "Print Time Stamps", "Plot Ping Interval",
        "Plot Longitude", "Plot Latitude", "Plot Heading", "Plot Speed", "Plot Center Beam Depth", "Plot Sonar Altitude", "Plot Sonar Depth", "Plot Roll",
        "Plot Pitch", "Plot Heave", "Go To Specified Time...", "Buffer Controls...", "Annotation...", "Filters...", "Reverse Right/Left Key Macros",
        "Reverse Mouse Buttons", "Browse Only", "Save"})
    CHECK(menu_action(window, text) != nullptr);
  for (const char *name : {"button_start", "button_reverse", "button_forward", "button_end", "button_next", "button_done", "button_flag_view",
                           "button_unflag_view", "button_unflag_forward", "button_quit"})
    CHECK(named<QPushButton>(window, name) != nullptr);

  /* the mode keys */
  const struct {
    const char *keys;
    Canvas::Mode mode;
    const char *radio;
  } modes[] = {{"QqUu", Canvas::MODE_TOGGLE, "mode_toggle"}, {"WwIi", Canvas::MODE_PICK, "mode_pick"},  {"EeOo", Canvas::MODE_ERASE, "mode_erase"},
               {"RrPp", Canvas::MODE_RESTORE, "mode_restore"}, {"Tt{[", Canvas::MODE_GRAB, "mode_grab"}, {"Yy}]", Canvas::MODE_INFO, "mode_info"},
               {"Hh", Canvas::MODE_LASSO, "mode_lasso"}};
  for (const auto &m : modes) {
    for (const char *k = m.keys; *k != '\0'; k++) {
      tap(canvas, 'q');
      tap(canvas, *k);
      CHECK(canvas->mode() == m.mode);
      CHECK(named<QRadioButton>(window, m.radio)->isChecked());
    }
  }
  named<QRadioButton>(window, "mode_erase")->click();
  CHECK(canvas->mode() == Canvas::MODE_ERASE);

  /* toggle, the click takes the nearest sounding */
  tap(canvas, 'q');
  int beam = 0;
  CHECK(good_beam(window, 3, &beam));
  click(canvas, where(window, 3, beam));
  CHECK(s.beamSaved() && s.savedPing() == 3);
  const int picked = s.savedBeam();
  CHECK(mb_beam_check_flag_manual(flag(window, 3, picked)));
  CHECK(window.isWindowModified());
  click(canvas, where(window, 3, picked));
  CHECK(mb_beam_ok(flag(window, 3, picked)));
  CHECK(s.changes() == 0);
  CHECK(!s.unsaved());

  /* far from every sounding nothing is taken */
  click(canvas, QPoint(2, 2));
  CHECK(s.changes() == 0);

  /* pick, and the keys for the whole ping and its sides */
  tap(canvas, 'w');
  click(canvas, where(window, 3, picked));
  CHECK(mb_beam_check_flag_manual(flag(window, 3, picked)));
  tap(canvas, 'z');
  for (int j = 0; j < s.pings()[3].beams_bath; j++)
    CHECK(!mb_beam_ok(flag(window, 3, j)));
  tap(canvas, 's');
  CHECK(mb_beam_ok(flag(window, 3, picked)));
  click(canvas, where(window, 3, picked));
  tap(canvas, 'a');
  CHECK(!mb_beam_ok(flag(window, 3, 0)));
  tap(canvas, 's');
  click(canvas, where(window, 3, picked));
  tap(canvas, 'd');
  CHECK(!mb_beam_ok(flag(window, 3, s.pings()[3].beams_bath - 1)));
  CHECK(mb_beam_ok(flag(window, 3, 0)));
  tap(canvas, 's');
  CHECK(s.changes() == 0);

  /* holding a key while clicking applies it to the picked sounding */
  key(canvas, 'z');
  CHECK(good_beam(window, 4, &beam));
  click(canvas, where(window, 4, beam));
  key(canvas, 'z', QEvent::KeyRelease);
  for (int j = 0; j < s.pings()[4].beams_bath; j++)
    CHECK(!mb_beam_ok(flag(window, 4, j)));
  tap(canvas, 'c');
  CHECK(s.changes() == 0);

  /* flag and unflag the view, by key and by button */
  tap(canvas, 'x');
  CHECK(!mb_beam_ok(flag(window, s.current() + 1, beam)));
  tap(canvas, 'c');
  CHECK(s.changes() == 0);
  named<QPushButton>(window, "button_flag_view")->click();
  CHECK(s.changes() > 0);
  named<QPushButton>(window, "button_unflag_view")->click();
  CHECK(s.changes() == 0);

  /* erase follows the drag, also between the positions reported */
  tap(canvas, 'e');
  const QPoint from = where(window, 2, 2), to = where(window, 2, 13);  // one swath of the dual swath
  mouse(canvas, QEvent::MouseButtonPress, from, Qt::LeftButton, Qt::LeftButton);
  mouse(canvas, QEvent::MouseMove, to, Qt::NoButton, Qt::LeftButton);
  mouse(canvas, QEvent::MouseButtonRelease, to, Qt::LeftButton, Qt::NoButton);
  int flagged = 0;
  for (int j = 2; j <= 13; j++)
    flagged += mb_beam_ok(flag(window, 2, j)) ? 0 : 1;
  CHECK(flagged == 12);
  tap(canvas, 'r');
  mouse(canvas, QEvent::MouseButtonPress, from, Qt::LeftButton, Qt::LeftButton);
  mouse(canvas, QEvent::MouseMove, to, Qt::NoButton, Qt::LeftButton);
  mouse(canvas, QEvent::MouseButtonRelease, to, Qt::LeftButton, Qt::NoButton);
  CHECK(s.changes() == 0);

  /* grab flags what is inside the rectangle */
  tap(canvas, 't');
  const QPoint corner = where(window, 2, 2);
  const QPoint other = where(window, 2, 13);
  const QRect rect = QRect(corner, other).normalized().adjusted(-1, -3, 1, 3);
  mouse(canvas, QEvent::MouseButtonPress, rect.topLeft(), Qt::LeftButton, Qt::LeftButton);
  mouse(canvas, QEvent::MouseMove, rect.center(), Qt::NoButton, Qt::LeftButton);
  mouse(canvas, QEvent::MouseMove, rect.bottomRight(), Qt::NoButton, Qt::LeftButton);
  mouse(canvas, QEvent::MouseButtonRelease, rect.bottomRight(), Qt::LeftButton, Qt::NoButton);
  CHECK(s.changes() > 0);
  CHECK(!mb_beam_ok(flag(window, 2, 5)));
  named<QPushButton>(window, "button_unflag_view")->click();
  CHECK(s.changes() == 0);

  /* undo and redo: one drag or one click with a key held is one step */
  tap(canvas, 'e');
  mouse(canvas, QEvent::MouseButtonPress, from, Qt::LeftButton, Qt::LeftButton);
  mouse(canvas, QEvent::MouseMove, to, Qt::NoButton, Qt::LeftButton);
  mouse(canvas, QEvent::MouseButtonRelease, to, Qt::LeftButton, Qt::NoButton);
  const long erased = s.changes();
  CHECK(erased >= 12);
  CHECK(menu_action(window, "Undo Erase") != nullptr);
  menu_action(window, "Undo Erase")->trigger();
  CHECK(s.changes() == 0);
  CHECK(menu_action(window, "Redo Erase") != nullptr);
  menu_action(window, "Redo Erase")->trigger();
  CHECK(s.changes() == erased);
  menu_action(window, "Undo Erase")->trigger();
  tap(canvas, 'q');
  key(canvas, 'z');
  CHECK(good_beam(window, 4, &beam));
  click(canvas, where(window, 4, beam));
  key(canvas, 'z', QEvent::KeyRelease);
  CHECK(s.changes() >= s.pings()[4].beams_bath);
  menu_action(window, "Undo Toggle")->trigger();
  CHECK(mb_beam_ok(flag(window, 4, beam)));
  named<QPushButton>(window, "button_unflag_view")->click();
  CHECK(menu_action(window, "Undo Unflag View") != nullptr || s.changes() == 0);

  /* lasso flags what is inside the outline, and is one step */
  tap(canvas, 'h');
  const QPoint spot = where(window, 2, 8);
  mouse(canvas, QEvent::MouseButtonPress, spot + QPoint(-3, -3), Qt::LeftButton, Qt::LeftButton);
  mouse(canvas, QEvent::MouseMove, spot + QPoint(3, -3), Qt::NoButton, Qt::LeftButton);
  mouse(canvas, QEvent::MouseMove, spot + QPoint(3, 3), Qt::NoButton, Qt::LeftButton);
  mouse(canvas, QEvent::MouseButtonRelease, spot + QPoint(-3, 3), Qt::LeftButton, Qt::NoButton);
  CHECK(!mb_beam_ok(flag(window, 2, 8)));
  CHECK(menu_action(window, "Undo Lasso") != nullptr);
  menu_action(window, "Undo Lasso")->trigger();
  CHECK(mb_beam_ok(flag(window, 2, 8)));
  tap(canvas, 'q');

  /* zoom around the mouse, exaggeration, pan, reset: the flags do not change */
  {
    const long before = s.changes();
    const int width0 = s.settings.plot_width, exagger0 = s.settings.exagger;
    const QPoint keep = where(window, 2, 8);
    auto wheel_at = [&](QPoint at, int delta, Qt::KeyboardModifiers mods) {
      QWheelEvent wheel(QPointF(at), QPointF(canvas->mapToGlobal(at)), QPoint(), QPoint(0, delta), Qt::NoButton, mods, Qt::NoScrollPhase, false);
      QApplication::sendEvent(canvas, &wheel);
    };
    wheel_at(keep, 240, Qt::ControlModifier);
    CHECK(s.settings.plot_width < width0);
    CHECK(s.current() == 0);
    CHECK(std::abs(where(window, 2, 8).x() - keep.x()) <= 2);
    CHECK(named<ScaleControl>(window, "scale_width")->value() == s.settings.plot_width);
    CHECK(s.zoomed());
    for (int i = s.current(); i < s.current() + s.nplot(); i++)
      CHECK(s.pings()[i].outbounds == mbe::OUTBOUNDS_NONE);

    /* zoomed: soundings outside the plot are hidden and safe */
    wheel_at(keep, 1200, Qt::ControlModifier);
    {
      const long flagged_before = s.changes();
      const mbe::Ping &ping = s.pings()[s.current() + 2];
      int outside = -1;
      for (int j = 0; j < ping.beams_bath; j++)
        if (ping.bath_x[j] < s.box().xmin)
          outside = j;
      CHECK(outside >= 0);
      if (outside >= 0) {
        named<QPushButton>(window, "button_flag_view")->click();
        CHECK(s.changes() > flagged_before);
        CHECK(mb_beam_ok(s.pings()[s.current() + 2].beamflag[outside]));
        menu_action(window, "Undo Flag View")->trigger();
        CHECK(s.changes() == flagged_before);
      }
    }
    tap(canvas, '0');
    CHECK(!s.zoomed());
    CHECK(s.settings.plot_width == width0);
    wheel_at(keep, 240, Qt::ControlModifier);
    wheel_at(keep, 120, Qt::ShiftModifier);
    CHECK(s.settings.exagger > exagger0);
    const QPoint before_pan = where(window, 2, 8);
    QKeyEvent space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
    QApplication::sendEvent(canvas, &space);
    mouse(canvas, QEvent::MouseButtonPress, QPoint(300, 300), Qt::LeftButton, Qt::LeftButton);
    mouse(canvas, QEvent::MouseMove, QPoint(340, 320), Qt::NoButton, Qt::LeftButton);
    mouse(canvas, QEvent::MouseButtonRelease, QPoint(340, 320), Qt::LeftButton, Qt::NoButton);
    QKeyEvent space_up(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
    QApplication::sendEvent(canvas, &space_up);
    CHECK(std::abs(where(window, 2, 8).x() - before_pan.x() - 40) <= 1);
    CHECK(std::abs(where(window, 2, 8).y() - before_pan.y() - 20) <= 1);
    CHECK(s.changes() == before);
    s.settings.exagger = exagger0;
    tap(canvas, '0');
    CHECK(s.settings.plot_width == width0);
    CHECK(where(window, 2, 8) == keep);
    wheel_at(keep, -12000, Qt::ControlModifier);
    CHECK(s.settings.plot_width == mbe::MAX_PLOT_WIDTH);
    tap(canvas, '0');
  }

  /* colours by amplitude and by angle, with their range */
  for (const char *text : {"Show Amplitude", "Show Beam Angle"}) {
    menu_action(window, text)->trigger();
    QApplication::processEvents();
    CHECK(s.settings.show_mode >= mbe::SHOW_AMPLITUDE);
    const double ratio = canvas->devicePixelRatioF();
    const QPoint at = where(window, 2, 8);
    const QColor color = canvas->grab().toImage().pixelColor((int)(at.x() * ratio), (int)(at.y() * ratio));
    CHECK(color.saturation() > 40);
  }
  menu_action(window, "Show Flag States")->trigger();

  /* info changes nothing */
  tap(canvas, 'y');
  CHECK(good_beam(window, 1, &beam));
  click(canvas, where(window, 1, beam));
  CHECK(s.info().set);
  CHECK(s.changes() == 0);
  CHECK(s.infoText().size() == 3);
  tap(canvas, 'q');

  /* stepping: keys, buttons, mouse buttons, wheel, and the big jump */
  named<QPushButton>(window, "button_start")->click();
  CHECK(s.current() == 0);
  tap(canvas, 'f');
  CHECK(s.current() == canvas->stepSize);
  tap(canvas, 'v');
  CHECK(s.current() == 0);
  click(canvas, QPoint(100, 100), Qt::RightButton);
  CHECK(s.current() == canvas->stepSize);
  click(canvas, QPoint(100, 100), Qt::MiddleButton);
  CHECK(s.current() == 0);
  menu_action(window, "Reverse Mouse Buttons")->trigger();
  click(canvas, QPoint(100, 100), Qt::LeftButton);
  CHECK(s.current() == canvas->stepSize);
  menu_action(window, "Reverse Mouse Buttons")->trigger();
  key(canvas, 'g');
  tap(canvas, 'f');
  CHECK(s.current() == s.nbuff() - 1);
  tap(canvas, 'v');
  CHECK(s.current() == 0);
  key(canvas, 'g', QEvent::KeyRelease);
  {
    QWheelEvent wheel(QPointF(100, 100), QPointF(canvas->mapToGlobal(QPoint(100, 100))), QPoint(), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(canvas, &wheel);
    CHECK(s.current() == canvas->stepSize);
  }
  named<QPushButton>(window, "button_end")->click();
  CHECK(s.current() == s.nbuff() - 1);
  named<QPushButton>(window, "button_start")->click();

  /* the slider */
  slider->setValue(7);
  CHECK(s.current() == 7);
  CHECK(named<QLabel>(window, "ping_position")->text().startsWith("8-"));
  tap(canvas, 'f');
  CHECK(slider->value() == 7 + canvas->stepSize);
  {
    const int before = slider->value();
    QWheelEvent wheel(QPointF(5, 5), QPointF(slider->mapToGlobal(QPoint(5, 5))), QPoint(), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(slider, &wheel);
    CHECK(slider->value() == before + 1);
    CHECK(s.current() == before + 1);
  }
  slider->setValue(0);
  CHECK(canvas->hasFocus() || !window.isActiveWindow());

  /* views and plot modes */
  tap(canvas, '3');
  CHECK(s.viewMode() == mbe::VIEW_ALONGTRACK);
  CHECK(menu_action(window, "Alongtrack View")->isChecked());
  tap(canvas, '4');
  CHECK(s.viewMode() == mbe::VIEW_ACROSSTRACK);
  tap(canvas, '2');
  CHECK(s.viewMode() == mbe::VIEW_WATERFALL);
  menu_action(window, "Plot Roll")->trigger();
  QApplication::processEvents();
  CHECK(s.settings.show_time == mbe::PLOT_ROLL);
  menu_action(window, "Wide Bathymetry Profiles")->trigger();
  const int xmin_wide = s.box().xmin;
  menu_action(window, "Print Time Stamps")->trigger();
  CHECK(s.box().xmin > xmin_wide);

  /* a resized window: the plot follows it, and a click still takes the same sounding */
  QApplication::processEvents();
  const int width_before = s.box().width;
  window.resize(window.width() - 200, window.height() - 100);
  QApplication::processEvents();
  CHECK(s.box().width == canvas->width());
  CHECK(s.box().width != width_before);
  CHECK(good_beam(window, 5, &beam));
  click(canvas, where(window, 5, beam));
  CHECK(s.savedPing() == 5);
  CHECK(!mb_beam_ok(flag(window, 5, s.savedBeam())));
  click(canvas, where(window, 5, s.savedBeam()));
  CHECK(s.changes() == 0);

  /* light and dark follow the palette, also with a file open */
  const QPalette original = QApplication::palette();
  QPalette dark = original, light = original;
  dark.setColor(QPalette::Base, QColor(20, 20, 20));
  dark.setColor(QPalette::Text, QColor(235, 235, 235));
  light.setColor(QPalette::Base, QColor(255, 255, 255));
  light.setColor(QPalette::Text, QColor(0, 0, 0));
  QApplication::setPalette(dark);
  QApplication::processEvents();
  CHECK(canvas->grab().toImage().pixelColor(3, 3).lightness() < 60);
  QApplication::setPalette(light);
  QApplication::processEvents();
  CHECK(canvas->grab().toImage().pixelColor(3, 3).lightness() > 200);

  /* a partly transparent text colour, as the macOS palette has: soundings are the colour over the background */
  QPalette translucent = dark;
  translucent.setColor(QPalette::Text, QColor(255, 255, 255, 216));
  QApplication::setPalette(translucent);
  QApplication::processEvents();
  {
    tap(canvas, 'q');
    CHECK(good_beam(window, s.current() + 1, &beam));
    const double ratio = canvas->devicePixelRatioF();
    const QPoint spot = where(window, s.current() + 1, beam);
    const QColor sounding = canvas->grab().toImage().pixelColor((int)(spot.x() * ratio), (int)(spot.y() * ratio));
    CHECK(sounding.alpha() == 255);
    CHECK(sounding.lightness() > 180);
  }
  QApplication::setPalette(original);
  QApplication::processEvents();

  /* the acrosstrack width stops at 40000 m */
  CHECK(named<ScaleControl>(window, "scale_width")->findChild<QDoubleSpinBox *>()->maximum() == mbe::MAX_PLOT_WIDTH);

  /* an exported picture has no transparent pixels */
  const QImage picture = canvas->render(QSize(600, 400), 2.0);
  CHECK(picture.width() == 1200 && picture.pixelColor(3, 3).alpha() == 255);

  /* soundings outside the plot get a zap box, which flags them */
  s.settings.plot_width = 10;
  s.plot(false);
  canvas->redraw();
  int zap_ping = -1;
  for (int i = s.current(); i < s.current() + s.nplot(); i++)
    if (s.pings()[i].outbounds == mbe::OUTBOUNDS_UNFLAGGED)
      zap_ping = i;
  CHECK(zap_ping >= 0);
  if (zap_ping >= 0) {
    const mbe::Ping &p = s.pings()[zap_ping];
    click(canvas, QPoint((p.zap_x1 + p.zap_x2) / 2, (p.zap_y1 + p.zap_y2) / 2));
    CHECK(s.changes() > 0);
    CHECK(s.pings()[zap_ping].outbounds != mbe::OUTBOUNDS_UNFLAGGED);
  }
  named<QPushButton>(window, "button_unflag_view")->click();
  CHECK(s.changes() == 0);
  s.plot(true);
  canvas->redraw();

  /* the filter dialog */
  menu_action(window, "Filters...")->trigger();
  auto *filters = named<QDialog>(window, "dialog_filters");
  CHECK(filters->isVisible());
  for (QCheckBox *box : filters->findChildren<QCheckBox *>())
    if (box->text() == "Flag by Beam Number")
      box->setChecked(true);
  named<QPushButton>(window, "filters_apply")->click();
  wait_for(window);
  CHECK(s.filters.cutbeam);
  CHECK(mb_beam_check_flag_filter2(flag(window, s.current(), 0)));
  filters->hide();
  s.filters = mbe::Filters();
  s.filterForward();
  canvas->redraw();
  CHECK(s.changes() == 0);

  /* the preview of the filters: outlines and numbers, and no flags change until Apply */
  {
    menu_action(window, "Filters...")->trigger();
    for (QCheckBox *box : filters->findChildren<QCheckBox *>())
      if (box->text() == "Flag by Beam Number")
        box->setChecked(true);
    QElapsedTimer waited;
    waited.start();
    auto *label = named<QLabel>(window, "filters_preview_label");
    while (label->text().isEmpty() && waited.elapsed() < 10000)
      QApplication::processEvents(QEventLoop::AllEvents, 10);
    CHECK(label->text().startsWith("Would flag"));
    CHECK(!label->text().startsWith("Would flag 0 "));
    CHECK(s.changes() == 0);
    canvas->grab();
    named<QCheckBox>(window, "filters_preview")->setChecked(false);
    waited.restart();
    while (!label->text().isEmpty() && waited.elapsed() < 5000)
      QApplication::processEvents(QEventLoop::AllEvents, 10);
    CHECK(label->text().isEmpty());
    named<QCheckBox>(window, "filters_preview")->setChecked(true);
    for (QCheckBox *box : filters->findChildren<QCheckBox *>())
      if (box->text() == "Flag by Beam Number")
        box->setChecked(false);
    filters->hide();
    QApplication::processEvents();
  }

  /* flags by reason */
  {
    tap(canvas, 'q');
    CHECK(good_beam(window, 2, &beam));
    click(canvas, where(window, 2, beam));
    const int ping = s.savedPing(), b = s.savedBeam();
    CHECK(!mb_beam_ok(flag(window, ping, b)));
    menu_action(window, "Flags by Reason...")->trigger();
    auto *dialog = named<QDialog>(window, "dialog_reasons");
    CHECK(dialog->isVisible());
    CHECK(named<QLabel>(window, "reason_count")->text() == "1 soundings");

    /* a hidden flag is not drawn and a click does not take it */
    named<QCheckBox>(window, "show_manual")->setChecked(false);
    CHECK(s.shown_reasons == (mbe::REASON_FILTER | mbe::REASON_SONAR));
    int other_ping = 0, other_beam = 0;
    CHECK(s.nearestBeam(s.pings()[ping].bath_x[b], s.pings()[ping].bath_y[b], false, &other_ping, &other_beam));
    CHECK(other_ping != ping || other_beam != b);
    named<QCheckBox>(window, "show_manual")->setChecked(true);
    for (QPushButton *button : dialog->findChildren<QPushButton *>())
      if (button->objectName() == "reason_unflag")
        button->click();
    CHECK(mb_beam_ok(flag(window, ping, b)));
    CHECK(s.changes() == 0);
    CHECK(menu_action(window, "Undo Unflag Manual") != nullptr);
    menu_action(window, "Undo Unflag Manual")->trigger();
    CHECK(!mb_beam_ok(flag(window, ping, b)));
    menu_action(window, "Undo Toggle")->trigger();
    CHECK(s.changes() == 0);
    dialog->hide();
  }

  /* mbclean through a scratch folder: nothing changes until Accept, and Undo takes it back */
  {
    menu_action(window, "Preview mbclean...")->trigger();
    auto *dialog = named<QDialog>(window, "dialog_mbclean");
    CHECK(dialog->isVisible());
    named<QLineEdit>(window, "mbclean_options")->setText("--mode=1 --max-acrosstrack=100");
    named<QPushButton>(window, "mbclean_run")->click();
    auto *status = named<QLabel>(window, "mbclean_status");
    QElapsedTimer waited;
    waited.start();
    while (status->text().startsWith("Running") && waited.elapsed() < 60000)
      QApplication::processEvents(QEventLoop::AllEvents, 20);
    CHECK(status->text().startsWith("mbclean would flag"));
    CHECK(!status->text().startsWith("mbclean would flag 0 "));
    CHECK(s.changes() == 0);
    CHECK(!QFile::exists(esf));
    CHECK(named<QPushButton>(window, "mbclean_accept")->isEnabled());
    named<QPushButton>(window, "mbclean_reject")->click();
    CHECK(s.changes() == 0);
    CHECK(!named<QPushButton>(window, "mbclean_accept")->isEnabled());
    named<QPushButton>(window, "mbclean_run")->click();
    waited.restart();
    while (status->text().startsWith("Running") && waited.elapsed() < 60000)
      QApplication::processEvents(QEventLoop::AllEvents, 20);
    named<QPushButton>(window, "mbclean_accept")->click();
    CHECK(s.changes() > 0);
    CHECK(menu_action(window, "Undo mbclean") != nullptr);
    menu_action(window, "Undo mbclean")->trigger();
    CHECK(s.changes() == 0);
    dialog->hide();
  }

  /* the review queue: the keys walk through it, the strip takes a click */
  {
    QElapsedTimer waited;
    waited.start();
    while (waited.elapsed() < 300)
      QApplication::processEvents(QEventLoop::AllEvents, 10);
    const std::vector<mbe::Suspect> suspects = s.findSuspects(s.copyFlags());
    if (!suspects.empty()) {
      tap(canvas, 'n');
      CHECK(s.current() == std::max(suspects[0].ping - s.settings.plot_size / 2, 0));
      CHECK(window.statusBar()->currentMessage().startsWith("Suspect 1 of"));
      tap(canvas, 'b');
    }
    auto *strip = named<SuspectStrip>(window, "suspect_strip");
    CHECK(strip != nullptr);
    CHECK(s.changes() == 0);
  }

  /* the other dialogs open */
  for (const char *text : {"Go To Specified Time...", "Buffer Controls...", "Annotation...", "File Selection List..."}) {
    menu_action(window, text)->trigger();
    QApplication::processEvents();
  }
  for (const char *name : {"dialog_goto", "dialog_buffer", "dialog_annotation", "dialog_filelist"}) {
    auto *dialog = named<QDialog>(window, name);
    CHECK(dialog->isVisible());
    dialog->hide();
  }
  CHECK(named<QTableWidget>(window, "filelist")->rowCount() == 1);
  CHECK(named<QTableWidget>(window, "filelist")->item(0, 0)->text() == "<loaded>");

  /* closing the window with no changes writes nothing and does not ask */
  CHECK(!QFile::exists(esf));
  bool asked = false;
  answer(QMessageBox::Cancel, &asked);
  CHECK(window.close());
  QApplication::processEvents();
  expire_answers();
  CHECK(!asked);
  CHECK(!QFile::exists(esf));
  CHECK(!QFile::exists(QString::fromStdString(file) + ".lck"));

  /* with changes: cancel keeps editing */
  window.show();
  open_file(window);
  CHECK(good_beam(window, 2, &beam));
  click(canvas, where(window, 2, beam));
  CHECK(s.unsaved());
  asked = false;
  answer(QMessageBox::Cancel, &asked);
  CHECK(!window.close());
  CHECK(asked);
  CHECK(s.isOpen() && s.unsaved());
  CHECK(window.isVisible());

  /* discard: nothing is written */
  asked = false;
  answer(QMessageBox::Discard, &asked);
  CHECK(window.close());
  CHECK(asked);
  CHECK(!s.isOpen());
  CHECK(!QFile::exists(esf));
  CHECK(!QFile::exists(QString::fromStdString(file) + ".par"));

  /* save: written after the window waited for it */
  window.show();
  open_file(window);
  click(canvas, where(window, 2, beam));
  const int saved_ping = s.savedPing(), saved_beam = s.savedBeam();
  asked = false;
  answer(QMessageBox::Save, &asked);
  window.close();
  QElapsedTimer timer;
  timer.start();
  while (window.isVisible() && timer.elapsed() < 60000)
    QApplication::processEvents(QEventLoop::AllEvents, 20);
  CHECK(asked);
  CHECK(!window.isVisible());
  CHECK(!s.isOpen());
  CHECK(QFile::exists(esf));
  CHECK(QFile::exists(QString::fromStdString(file) + ".par"));
  CHECK(!QFile::exists(esf + ".new"));
  CHECK(QFile(esf).size() == 1024 + 16);

  /* the next time: asks about the saved edits, and applies them on yes */
  window.show();
  asked = false;
  answer(QMessageBox::Yes, &asked);
  window.editor()->loadFirstFile(false);
  wait_for(window);
  CHECK(asked);
  CHECK(s.isOpen());
  CHECK(!mb_beam_ok(flag(window, saved_ping, saved_beam)));
  CHECK(!s.unsaved());

  /* File Save keeps the file open */
  click(canvas, where(window, 2, saved_beam + 1));
  CHECK(s.unsaved());
  menu_action(window, "Save")->trigger();
  CHECK(!s.unsaved());
  CHECK(s.isOpen());
  CHECK(QFile(esf).size() == 1024 + 2 * 16);

  /* a second file: Done moves on and saves without asking */
  window.editor()->addInput(file2, 0);
  QApplication::processEvents();
  CHECK(menu_action(window, "Browse Only") != nullptr);
  click(canvas, where(window, 2, saved_beam + 2));
  asked = false;
  answer(QMessageBox::Save, &asked);
  named<QPushButton>(window, "button_done")->click();
  wait_for(window);
  QElapsedTimer settle;
  settle.start();
  while ((window.busy() || !s.isOpen()) && settle.elapsed() < 30000)
    QApplication::processEvents(QEventLoop::AllEvents, 20);
  expire_answers();
  CHECK(!asked);
  CHECK(QFile(esf).size() == 1024 + 3 * 16);
  CHECK(std::string(s.file()) == file2);

  /* browse mode: no question and no files */
  CHECK(window.close());
  window.editor()->setBrowse(true);
  window.show();
  const QString esf2 = QString::fromStdString(file2) + ".esf";
  CHECK(!QFile::exists(esf2));
  window.editor()->addInput(file2, 0);
  QApplication::processEvents();
  window.editor()->loadFirstFile(true);
  wait_for(window);
  click(canvas, where(window, 1, 5));
  asked = false;
  answer(QMessageBox::Save, &asked);
  CHECK(window.close());
  QApplication::processEvents();
  expire_answers();
  CHECK(!asked);
  CHECK(!QFile::exists(esf2));

  /* holding the right button repeats after 2 seconds */
  window.editor()->setBrowse(false);
  window.show();
  bool applied = false;
  answer(QMessageBox::No, &applied);
  s.setBuffer(20, 5);
  window.editor()->loadFirstFile(false);
  wait_for(window);
  CHECK(applied);
  CHECK(s.isOpen() && s.nbuff() == 20);
  mouse(canvas, QEvent::MouseButtonPress, QPoint(100, 100), Qt::RightButton, Qt::RightButton);
  CHECK(s.current() == canvas->stepSize);
  QElapsedTimer hold;
  hold.start();
  while (hold.elapsed() < 2300)
    QApplication::processEvents(QEventLoop::AllEvents, 20);
  mouse(canvas, QEvent::MouseButtonRelease, QPoint(100, 100), Qt::RightButton, Qt::NoButton);
  CHECK(s.current() > canvas->stepSize);

  /* Go To and Next Buffer: the last 5 pings stay in memory and keep an edit */
  named<QPushButton>(window, "button_start")->click();
  menu_action(window, "Go To Specified Time...")->trigger();
  auto *goto_dialog = named<QDialog>(window, "dialog_goto");
  for (QPushButton *button : goto_dialog->findChildren<QPushButton *>())
    if (button->text() == "Apply")
      button->click();
  wait_for(window);
  CHECK(s.current() >= 1);
  CHECK(!goto_dialog->isVisible());
  slider->setValue(15);
  CHECK(good_beam(window, 16, &beam));
  click(canvas, where(window, 16, beam));
  const int retained_beam = s.savedBeam();
  CHECK(named<QPushButton>(window, "button_next")->isEnabled());
  named<QPushButton>(window, "button_next")->click();
  wait_for(window);
  CHECK(s.ndumpTotal() == 15);
  CHECK(s.nbuff() == 20);
  CHECK(slider->maximum() == 19);
  CHECK(!mb_beam_ok(flag(window, 1, retained_beam)));
  answer(QMessageBox::Discard);
  CHECK(window.close());
  QApplication::processEvents();
  CHECK(QFile(esf).size() == 1024 + 3 * 16);
  s.setBuffer(mbe::DEFAULT_BUFFER_SIZE, 30);

  /* after a crash: asks about the lock, then about the edits */
  QFile::copy(QString::fromStdString(file2), QString::fromStdString(file2) + ".unused");
  QFile lock(QString::fromStdString(file) + ".lck");
  CHECK(lock.open(QIODevice::WriteOnly));
  {
    char user[256] = "", host[256] = "", date[32] = "";
    int error = 0;
    mb_user_host_date(0, user, host, date, &error);
    lock.write(QString("Locking Program: mbedit-qt\nLocking User: %1\nLocking CPU: %2\nLocking Time: %3\nLocking Purpose ID: 2\n").arg(user, host, date).toUtf8());
  }
  lock.close();
  QFile::copy(esf, esf + ".new.stream");
  window.show();
  int prompts = 0;
  std::function<void()> yes = [&] {
    if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
      prompts++;
      box->button(QMessageBox::Yes)->click();
    }
    if (prompts < 2)
      QTimer::singleShot(50, yes);
  };
  QTimer::singleShot(50, yes);
  window.editor()->loadFirstFile(false);
  wait_for(window);
  CHECK(prompts == 2);
  CHECK(s.isOpen());
  CHECK(s.unsaved());
  CHECK(!mb_beam_ok(flag(window, saved_ping, saved_beam)));
  answer(QMessageBox::Discard);
  CHECK(window.close());
  QFile::remove(QString::fromStdString(file2) + ".unused");

  /* a reference surface, the panels, the history and the context menu */
  if (argc > 3) {
    MainWindow w;
    w.editor()->addInput(file, 0);
    w.show();
    w.resize(1100, 700);
    QApplication::processEvents();
    w.editor()->loadFirstFile(true);
    wait_for(w);
    mbe::Session &t = w.session();
    CHECK(t.isOpen());
    t.gui_mode = false;

    QAction *difference = menu_action(w, "Show Difference from Reference");
    CHECK(difference != nullptr);
    CHECK(!difference->isEnabled());
    CHECK(!menu_action(w, "Clear Reference Surface")->isEnabled());
    /* the warning comes when the thread ends, so watch for it (not on macOS, where the timer cannot answer it) */
    if (QApplication::platformName() == "offscreen") {
      bool asked = false;
      QTimer watch;
      QObject::connect(&watch, &QTimer::timeout, [&asked, &watch] {
        if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
          asked = true;
          watch.stop();
          box->button(QMessageBox::Ok)->click();
        }
      });
      watch.start(20);
      const QString missing = QString::fromStdString(file) + ".nothing.grd";
      w.editor()->loadReference(missing);
      QElapsedTimer box_timer;
      box_timer.start();
      while (!asked && box_timer.elapsed() < 20000)
        QApplication::processEvents(QEventLoop::AllEvents, 20);
      watch.stop();
      wait_for(w);
      CHECK(!t.hasReference());
      CHECK(asked);
    }

    w.editor()->loadReference(argv[3]);
    wait_for(w);
    CHECK(t.hasReference());
    CHECK(t.settings.show_mode == mbe::SHOW_RESIDUAL);
    CHECK(difference->isEnabled());
    CHECK(difference->isChecked());
    CHECK(t.referenceIsTopography());
    std::vector<double> values;
    t.residuals(t.current() + t.nplot() / 2, nullptr, values);
    int with_value = 0;
    for (double v : values)
      with_value += std::isnan(v) ? 0 : 1;
    CHECK(with_value > 0);
    CHECK(!w.canvas()->render(QSize(800, 600), 1.0).isNull());
    menu_action(w, "Clear Reference Surface")->trigger();
    QApplication::processEvents();
    CHECK(!t.hasReference());
    CHECK(t.settings.show_mode == mbe::SHOW_FLAG);
    CHECK(!difference->isEnabled());
    w.editor()->loadReference(argv[3]);
    wait_for(w);
    CHECK(t.settings.show_mode == mbe::SHOW_RESIDUAL);

    /* clicking the map and a time series moves to that ping */
    QAction *map_action = menu_action(w, "Show Track Map");
    CHECK(map_action != nullptr);
    auto *map = w.findChild<TrackMap *>("track_map");
    CHECK(!map->isVisible());
    map_action->trigger();
    QApplication::processEvents();
    CHECK(map->isVisible());
    CHECK(w.findChild<QSplitter *>()->sizes()[2] > 50);
    CHECK(!map->visibleRegion().isEmpty());
    int clicked = -1;
    QObject::connect(map, &TrackMap::pingClicked, [&clicked](int ping) { clicked = ping; });
    QPointF position;
    const int target = t.nbuff() - 3;
    CHECK(map->pingPosition(target, &position));
    t.step(-t.current());
    click(map, position.toPoint());
    CHECK(std::abs(clicked - target) <= 1);
    CHECK(t.current() <= target && target < t.current() + t.nplot());
    map_action->trigger();
    QApplication::processEvents();

    menu_action(w, "Plot Heading")->trigger();
    QApplication::processEvents();
    auto *panel = w.findChild<TimeSeriesPanel *>();
    CHECK(panel->isVisible());
    int panel_ping = -1;
    QObject::connect(panel, &TimeSeriesPanel::pingClicked, [&panel_ping](int ping) { panel_ping = ping; });
    t.step(-t.current());
    QApplication::processEvents();
    const mbe::PlotBox panel_box = t.box();
    click(panel, QPoint(30, panel_box.ymax - 2));
    CHECK(panel_ping == 0);
    click(panel, QPoint(30, panel_box.ymin - 50));
    CHECK(panel_ping == 0);
    menu_action(w, "Print Time Stamps")->trigger();
    QApplication::processEvents();

    /* Shift and the right button: flag the ping, one step of undo */
    /* a native menu on macOS cannot be answered from a timer */
    if (QApplication::platformName() == "offscreen") {
      t.step(5);
      QApplication::processEvents();
      const int menu_ping = t.current() + 1;
      int menu_beam = 0;
      CHECK(good_beam(w, menu_ping, &menu_beam));
      const long changes_before = t.changes();
      QStringList menu_items;
      bool chose = false;
      QTimer::singleShot(100, [&menu_items, &chose] {
        if (auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
          for (QAction *action : popup->actions())
            menu_items << action->text();
          for (QAction *action : popup->actions()) {
            if (action->text() == "Flag This Ping") {
              chose = true;
              action->trigger();
            }
          }
          popup->close();
        }
      });
      const QPoint at = where(w, menu_ping, menu_beam);
      mouse(w.canvas(), QEvent::MouseButtonPress, at, Qt::RightButton, Qt::RightButton, Qt::ShiftModifier);
      mouse(w.canvas(), QEvent::MouseButtonRelease, at, Qt::RightButton, Qt::NoButton, Qt::ShiftModifier);
      QApplication::processEvents();
      CHECK(chose);
      CHECK(menu_items.contains("Unflag This Ping"));
      CHECK(t.changes() > changes_before);
      CHECK(menu_action(w, "Undo Flag Ping") != nullptr);
      menu_action(w, "Undo Flag Ping")->trigger();
      QApplication::processEvents();
      CHECK(t.changes() == changes_before);
    }

    /* the history: two saves, a preview of the first and a restore */
    int edit_beam = 0;
    const int edit_ping = t.current();
    CHECK(good_beam(w, edit_ping, &edit_beam));
    w.canvas()->setMode(Canvas::MODE_TOGGLE);
    click(w.canvas(), where(w, edit_ping, edit_beam));
    QApplication::processEvents();
    menu_action(w, "Save")->trigger();
    wait_for(w);
    const int second_ping = t.current() + 1;
    int second_beam = 0;
    CHECK(good_beam(w, second_ping, &second_beam));
    click(w.canvas(), where(w, second_ping, second_beam));
    QApplication::processEvents();
    menu_action(w, "Save")->trigger();
    wait_for(w);
    CHECK(!mb_beam_ok(flag(w, second_ping, second_beam)));
    menu_action(w, "Edit History...")->trigger();
    QApplication::processEvents();
    auto *table = w.findChild<QTableWidget *>("history_table");
    auto *status = w.findChild<QLabel *>("history_status");
    const auto versions = mbe::Session::historyVersions(t.file());
    CHECK(table->rowCount() == (int)versions.size());
    CHECK(versions.size() >= 2);
    /* the newest version is the state before the last save, so it has the first edit only */
    const int earlier = (int)versions.size() - 1;
    if (earlier >= 0) {
      table->selectRow(earlier);
      w.findChild<QPushButton *>("history_preview")->click();
      QApplication::processEvents();
      CHECK(status->text().contains("would"));
      w.findChild<QPushButton *>("history_clear")->click();
      answer(QMessageBox::Yes);
      w.findChild<QPushButton *>("history_restore")->click();
      wait_for(w);
      CHECK(t.isOpen());
      CHECK(mb_beam_ok(flag(w, second_ping, second_beam)));
      CHECK(!mb_beam_ok(flag(w, edit_ping, edit_beam)));
      CHECK(mbe::Session::historyVersions(t.file()).size() > versions.size());
    }
    answer(QMessageBox::Discard);
    CHECK(w.close());
  }

  printf("%d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
