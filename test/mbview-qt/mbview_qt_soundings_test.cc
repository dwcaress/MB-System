/*--------------------------------------------------------------------
 *    The MB-system:  mbview_qt_soundings_test.cc  11/7/2026
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
 * Test program of the 3D soundings window of mbview-qt.
 *
 *   mbview_qt_soundings_test --null       checks without drawing (null
 *                                         backend), for ctest
 *   mbview_qt_soundings_test --gl         draws synthetic soundings with the
 *                                         OpenGL backend and edits and
 *                                         inspects soundings with mouse events
 *   mbview_qt_soundings_test --time N     draw times of N soundings with and
 *                                         without the bounding box, whose
 *                                         stippled lines make Apple's OpenGL
 *                                         fall back to software vertex
 *                                         processing
 *   mbview_qt_soundings_test --show [N]   a window to use by hand; Dismiss or
 *                                         closing the window ends the program
 *                                         (MBVIEW_QT_TEST_DISMISS: Dismiss after
 *                                         half a second)
 *
 * The synthetic soundings are pings of beams across a sloping, rippled
 * seafloor, every 23rd sounding flagged.
 */

#include <QAction>
#include <QApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QImage>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QRadioButton>
#include <QSlider>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbview.h"
}
#include "mbvq.h"
#include "mbvq_bridge.h"
#include "mbvq_soundings.h"

/* from the private soundings header, which needs the Motif-shaped headers */
extern "C" void mb3dsoundings_setviewtoggle(int toggle, bool state);
constexpr int kViewToggleBoundingBox = 2; /* MBS_VIEWTOGGLE_BOUNDINGBOX */
constexpr int kViewToggleFlagged = 0;     /* MBS_VIEWTOGGLE_FLAGGED */

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

/* the calling program's side, as mbeditviz */
struct mb3dsoundings_struct soundings;
std::vector<struct mb3dsoundings_sounding_struct> storage;
int dismiss_calls = 0;
/* --show: Dismiss ends the program (in mbeditviz the main window stays) */
bool quit_on_dismiss = false;
int edit_calls = 0;
int edit_ifile = -1, edit_iping = -1, edit_ibeam = -1;
char edit_flag = 0;
int info_calls = 0;

void dismiss_notify() {
  dismiss_calls++;
  if (quit_on_dismiss)
    QTimer::singleShot(0, qApp, &QApplication::quit);
}
void edit_notify(int ifile, int iping, int ibeam, char beamflag, int flush) {
  /* FLUSHPREVIOUS only ends a series of edits; NOFLUSH and FLUSH carry one */
  if (flush == MB3DSDG_EDIT_FLUSHPREVIOUS)
    return;
  edit_calls++;
  edit_ifile = ifile;
  edit_iping = iping;
  edit_ibeam = ibeam;
  edit_flag = beamflag;
}
int bias_calls = 0, biasapply_calls = 0;
double bias_values[5] = {};
void bias_notify(double roll, double pitch, double heading, double timelag, double snell) {
  bias_calls++;
  bias_values[0] = roll;
  bias_values[1] = pitch;
  bias_values[2] = heading;
  bias_values[3] = timelag;
  bias_values[4] = snell;
}
void biasapply_notify(double roll, double pitch, double heading, double timelag, double snell) {
  (void)roll;
  (void)pitch;
  (void)heading;
  (void)timelag;
  (void)snell;
  biasapply_calls++;
}
int sparse_cells = -1, sparse_n = -1;
void sparse_notify(int cells, int n) {
  sparse_cells = cells;
  sparse_n = n;
}
int color_notified = -1;
void color_notify(int color) { color_notified = color; }
int optimize_mode = -1;
void optimize_notify(int mode, double *roll, double *pitch, double *heading, double *timelag, double *snell) {
  (void)pitch;
  (void)heading;
  (void)timelag;
  (void)snell;
  optimize_mode = mode;
  *roll = 1.25;
}
void info_notify(int ifile, int iping, int ibeam, char *infostring) {
  info_calls++;
  std::snprintf(infostring, MB_PATH_MAXLINE, "Sounding: file %d ping %d beam %d", ifile, iping, ibeam);
}

/* pings along y, beams across x, 2 m spacing; depth negative */
void make_soundings(int npings, int nbeams) {
  storage.assign(static_cast<size_t>(npings) * nbeams, {});
  const double dx = 2.0, dy = 2.0;
  const double xhalf = 0.5 * dx * (nbeams - 1), yhalf = 0.5 * dy * (npings - 1);
  int flagged = 0;
  for (int iping = 0; iping < npings; iping++)
    for (int ibeam = 0; ibeam < nbeams; ibeam++) {
      struct mb3dsoundings_sounding_struct &s = storage[static_cast<size_t>(iping) * nbeams + ibeam];
      s.ifile = 0;
      s.iping = iping;
      s.iprofile = iping;
      s.ibeam = ibeam;
      s.beamcolor = MBV_COLOR_BLACK;
      s.x = -xhalf + dx * ibeam;
      s.y = -yhalf + dy * iping;
      s.z = -1000.0 + 0.05 * s.x + 3.0 * std::sin(s.y / 15.0);
      s.a = 50.0 + 10.0 * std::cos(s.x / 20.0);
      const bool flag = (iping * nbeams + ibeam) % 23 == 0;
      s.beamflag = flag ? static_cast<char>(MB_FLAG_FLAG | MB_FLAG_MANUAL) : static_cast<char>(MB_FLAG_NONE);
      s.beamflagorg = s.beamflag;
      flagged += flag ? 1 : 0;
    }
  std::memset(&soundings, 0, sizeof(soundings));
  soundings.xmin = -xhalf;
  soundings.xmax = xhalf;
  soundings.ymin = -yhalf;
  soundings.ymax = yhalf;
  soundings.bearing = 90.0;
  soundings.sinbearing = 1.0;
  soundings.cosbearing = 0.0;
  const double dxy = std::sqrt(4.0 * xhalf * xhalf + 4.0 * yhalf * yhalf);
  soundings.scale = 2.0 / dxy;
  soundings.zscale = soundings.scale;
  soundings.num_soundings = static_cast<int>(storage.size());
  soundings.num_soundings_alloc = soundings.num_soundings;
  soundings.num_soundings_flagged = flagged;
  soundings.num_soundings_unflagged = soundings.num_soundings - flagged;
  soundings.soundings = storage.data();
}

void open_soundings() {
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_open(0, &soundings, &error);
  mb3dsoundings_set_dismiss_notify(0, &dismiss_notify, &error);
  mb3dsoundings_set_edit_notify(0, &edit_notify, &error);
  mb3dsoundings_set_info_notify(0, &info_notify, &error);
  mb3dsoundings_set_bias_notify(0, &bias_notify, &error);
  mb3dsoundings_set_biasapply_notify(0, &biasapply_notify, &error);
  mb3dsoundings_set_flagsparsevoxels_notify(0, &sparse_notify, &error);
  mb3dsoundings_set_colorsoundings_notify(0, &color_notify, &error);
  mb3dsoundings_set_optimizebiasvalues_notify(0, &optimize_notify, &error);
}

QSlider *slider_after(QWidget *window, const char *label_text) {
  /* the slider in the same box as its label */
  for (QLabel *label : window->findChildren<QLabel *>())
    if (label->text() == QString::fromLatin1(label_text))
      for (QSlider *slider : label->parentWidget()->findChildren<QSlider *>()) {
        const int lx = label->mapTo(window, QPoint(0, 0)).x();
        const int sx = slider->mapTo(window, QPoint(0, 0)).x();
        if (std::abs(lx - sx) < 5)
          return slider;
      }
  return nullptr;
}

QAction *menu_action(QMainWindow *window, const char *menu, const char *text) {
  for (QAction *top : window->menuBar()->actions())
    if (top->text() == QString::fromLatin1(menu) && top->menu() != nullptr)
      for (QAction *action : top->menu()->actions())
        if (action->text() == QString::fromLatin1(text))
          return action;
  return nullptr;
}

QRadioButton *radio(QWidget *window, const char *text) {
  for (QRadioButton *button : window->findChildren<QRadioButton *>())
    if (button->text() == QString::fromLatin1(text))
      return button;
  return nullptr;
}

QString label_containing(QWidget *window, const char *text) {
  for (QLabel *label : window->findChildren<QLabel *>())
    if (label->text().contains(QString::fromLatin1(text)))
      return label->text();
  return QString();
}

void click(QWidget *view, QPointF position, Qt::MouseButton button = Qt::LeftButton) {
  QMouseEvent press(QEvent::MouseButtonPress, position, view->mapToGlobal(position), button, button, Qt::NoModifier);
  QApplication::sendEvent(view, &press);
  QMouseEvent release(QEvent::MouseButtonRelease, position, view->mapToGlobal(position), button, Qt::NoButton,
                      Qt::NoModifier);
  QApplication::sendEvent(view, &release);
}

int edit_mode() {
  int edit = -1, mouse = -1;
  mbvq_c_soundings_modes(&edit, &mouse);
  return edit;
}

int mouse_mode() {
  int edit = -1, mouse = -1;
  mbvq_c_soundings_modes(&edit, &mouse);
  return mouse;
}

/*------------------------------------------------------------------------------*/
int test_null() {
  make_soundings(100, 50);
  open_soundings();
  mbvq::SoundingsWindow *window = mbvq::soundings_window();
  check(window != nullptr && window->isVisible(), "mb3dsoundings_open shows the soundings window");
  if (window == nullptr)
    return 1;
  check(window->windowTitle() == QStringLiteral("3D Soundings"), "the window title is that of Motif");
  check(radio(window, "Toggle") != nullptr && radio(window, "Toggle")->isChecked(), "the Toggle edit mode is set");
  check(label_containing(window, "Mouse Mode:").contains(QStringLiteral("L: Edit (Toggle)")),
        "the mouse mode label shows the Motif default text");

  /* the edit modes */
  radio(window, "Erase")->click();
  check(edit_mode() == MBVQ_SDG_EDIT_ERASE, "the Erase button sets the erase mode");
  check(label_containing(window, "Mouse Mode:").contains(QStringLiteral("L: Edit (Erase)")),
        "the mouse mode label names the erase mode");
  check(window->widget()->view()->cursor().shape() == Qt::PointingHandCursor, "the erase mode has the exchange cursor");
  radio(window, "Info")->click();
  check(edit_mode() == MBVQ_SDG_EDIT_INFO, "the Info button sets the info mode");

  /* the mouse modes, in the menu and the buttons */
  QAction *panzoom = menu_action(window, "Mouse", "Pan and Zoom");
  check(panzoom != nullptr, "the Mouse menu has Pan and Zoom");
  if (panzoom != nullptr)
    panzoom->trigger();
  check(mouse_mode() == MBVQ_SDG_MOUSE_PANZOOM && radio(window, "Pan and Zoom")->isChecked(),
        "Mouse > Pan and Zoom sets the mode and the button");
  check(label_containing(window, "Mouse Mode:").contains(QStringLiteral("M: Pan")), "the label names Pan and Zoom");
  radio(window, "Rotate Soundings")->click();
  check(mouse_mode() == MBVQ_SDG_MOUSE_ROTATE && menu_action(window, "Mouse", "Rotate Soundings")->isChecked(),
        "the Rotate Soundings button sets the mode and the menu");

  /* Reset View: the status line */
  radio(window, "Toggle")->click();
  mbvq_c_soundings_resetview();
  const QString status = label_containing(window, "Tot:");
  check(status.contains(QStringLiteral("Tot:5000")) && status.contains(QStringLiteral("Flagged:218")),
        "Reset View shows the sounding counts in the status line: " + status.toStdString());

  /* the soundings as data, for a renderer without OpenGL
     (src/mbview/mbv_soundings_core.c): what changed, the colours and display
     positions it draws, and the screen positions a pick needs - which the null
     backend never draws, so they are filled on demand */
  {
    int error = MB_ERROR_NO_ERROR;
    uint64_t position = 0, color = 0, mask = 0;
    mb3dsoundings_getmarks(0, &position, &color, &mask, &error);
    uint64_t position2 = 0, color2 = 0, mask2 = 0;
    mb3dsoundings_getmarks(0, &position2, &color2, &mask2, &error);
    check(position > 0 && color > 0 && mask > 0, "handing over the soundings raised every mark");
    check(position2 == position && color2 == color && mask2 == mask, "reading the marks changes nothing");

    int count = 0;
    mb3dsoundings_getcount(0, &count, &error);
    check(count == static_cast<int>(storage.size()), "the count of the soundings shown (" + std::to_string(count) + ")");
    int manual = 0;
    for (const auto &sounding : storage)
      manual += (sounding.beamflag & MB_FLAG_MANUAL) != 0 ? 1 : 0;

    /* the flag colour mode with flagged soundings shown: the unflagged ones in
       their beam colour (black here), the manually flagged ones red */
    std::vector<unsigned char> rgba(4 * storage.size());
    int n = 0;
    int colors_status = mb3dsoundings_colors(0, count, rgba.data(), &n, &error);
    int black = 0, red = 0, other = 0;
    for (size_t i = 0; i < storage.size(); i++) {
      const unsigned char *c = &rgba[4 * i];
      const bool flagged_manual = (storage[i].beamflag & MB_FLAG_MANUAL) != 0;
      if (!flagged_manual && c[0] == 0 && c[1] == 0 && c[2] == 0 && c[3] == 255)
        black++;
      else if (flagged_manual && c[0] == 255 && c[1] == 0 && c[2] == 0 && c[3] == 255)
        red++;
      else
        other++;
    }
    check(colors_status == MB_SUCCESS && n == count && red == manual && black == count - manual && other == 0,
          "the colours of the flag mode: " + std::to_string(black) + " unflagged black, " + std::to_string(red) +
              " flagged red");

    /* a buffer for fewer soundings than there are is refused untouched */
    std::vector<unsigned char> small(4, 7);
    colors_status = mb3dsoundings_colors(0, 1, small.data(), &n, &error);
    check(colors_status == MB_FAILURE && error == MB_ERROR_BAD_PARAMETER && n == count && small[0] == 7,
          "a buffer for fewer soundings than there are is refused");

    /* hiding the flagged soundings raises only the colour mark, and exactly
       those soundings get alpha 0 */
    mb3dsoundings_setviewtoggle(kViewToggleFlagged, false);
    uint64_t position3 = 0, color3 = 0, mask3 = 0;
    mb3dsoundings_getmarks(0, &position3, &color3, &mask3, &error);
    check(color3 > color && position3 == position && mask3 == mask,
          "hiding the flagged soundings raises the colour mark only");
    mb3dsoundings_colors(0, count, rgba.data(), &n, &error);
    int hidden = 0;
    bool hidden_flagged = true;
    for (size_t i = 0; i < storage.size(); i++) {
      const bool flagged_manual = (storage[i].beamflag & MB_FLAG_MANUAL) != 0;
      if (rgba[4 * i + 3] == 0) {
        hidden++;
        hidden_flagged = hidden_flagged && flagged_manual;
      } else if (flagged_manual) {
        hidden_flagged = false;
      }
    }
    check(hidden_flagged && hidden == manual,
          "with flagged soundings hidden exactly those " + std::to_string(hidden) + " have alpha 0");
    mb3dsoundings_setviewtoggle(kViewToggleFlagged, true);

    /* the display positions are those the drawing uses */
    std::vector<float> xyz(3 * storage.size());
    mb3dsoundings_positions(0, count, xyz.data(), &n, &error);
    bool same = n == count;
    for (size_t i = 0; same && i < storage.size(); i++)
      same = xyz[3 * i] == storage[i].glx && xyz[3 * i + 1] == storage[i].gly && xyz[3 * i + 2] == storage[i].glz;
    check(same, "the display positions are glx/gly/glz of every sounding");

    /* the view reset rescales z: the position mark rises, the colour mark not */
    mb3dsoundings_getmarks(0, &position3, &color3, &mask3, &error);
    mbvq_c_soundings_resetview();
    uint64_t position4 = 0, color4 = 0, mask4 = 0;
    mb3dsoundings_getmarks(0, &position4, &color4, &mask4, &error);
    check(position4 > position3 && color4 == color3, "a view reset raises the position mark and not the colour mark");

    /* a click edits the sounding under it although nothing has been drawn:
       the screen positions are filled for the current camera on demand */
    QWidget *view = window->widget()->view();
    const double dpr = view->devicePixelRatioF();
    const int gl_width = static_cast<int>(std::lround(view->width() * dpr));
    const int gl_height = static_cast<int>(std::lround(view->height() * dpr));
    mb3dsoundings_screen_positions(0, &error);
    size_t target = storage.size();
    double best = 1.0e30;
    for (size_t i = 0; i < storage.size(); i++) {
      if (!mb_beam_ok(storage[i].beamflag))
        continue;
      const double d = std::hypot(storage[i].winx - 0.5 * gl_width, storage[i].winy - 0.5 * gl_height);
      if (d < best) {
        best = d;
        target = i;
      }
    }
    check(target < storage.size() && best < 0.25 * std::min(gl_width, gl_height),
          "after filling the screen positions a sounding lies near the middle of the view");
    if (target < storage.size()) {
      const int x = storage[target].winx, y = storage[target].winy;
      mb3dsoundings_getmarks(0, &position3, &color3, &mask3, &error);
      const int calls = edit_calls;
      click(view, QPointF(x / dpr, (gl_height - 1 - y) / dpr));
      spin(50);
      size_t edited = storage.size();
      for (size_t i = 0; i < storage.size(); i++)
        if (storage[i].ifile == edit_ifile && storage[i].iping == edit_iping && storage[i].ibeam == edit_ibeam)
          edited = i;
      check(edit_calls == calls + 1 && edited < storage.size() && std::abs(storage[edited].winx - x) <= 1 &&
                std::abs(storage[edited].winy - y) <= 1 && !mb_beam_ok(storage[edited].beamflag),
            "a click flags the sounding under it, with nothing drawn");
      mb3dsoundings_getmarks(0, &position4, &color4, &mask4, &error);
      check(color4 > color3 && position4 == position3, "the edit raises the colour mark only");
      click(view, QPointF(x / dpr, (gl_height - 1 - y) / dpr));
      spin(50);
      check(edit_calls == calls + 2 && edited < storage.size() && mb_beam_ok(storage[edited].beamflag),
            "a second click restores it");
    }

    /* the program recoloured the soundings itself, as mbeditviz does when its
       colour table changes */
    mb3dsoundings_getmarks(0, &position3, &color3, &mask3, &error);
    mb3dsoundings_colors_changed(0, &error);
    mb3dsoundings_getmarks(0, &position4, &color4, &mask4, &error);
    check(color4 > color3 && position4 == position3 && mask4 == mask3,
          "the program telling of new colours raises the colour mark only");
  }

  /* keys reach the core: ',' flags all soundings in view */
  int unflagged = 0;
  for (const auto &s : storage)
    unflagged += mb_beam_ok(s.beamflag) ? 1 : 0;
  edit_calls = 0;
  window->widget()->view()->setFocus();
  QKeyEvent comma(QEvent::KeyPress, Qt::Key_Comma, Qt::NoModifier, QStringLiteral(","));
  QApplication::sendEvent(window->widget()->view(), &comma);
  QKeyEvent comma_up(QEvent::KeyRelease, Qt::Key_Comma, Qt::NoModifier, QStringLiteral(","));
  QApplication::sendEvent(window->widget()->view(), &comma_up);
  /* ',' flags every unflagged sounding, whatever its screen position */
  check(edit_calls == unflagged, "the ',' key flags the soundings in view (" +
                                                             std::to_string(edit_calls) + " edits)");

  /* the View menu shows and sets the core state */
  struct mbvq_soundings_state state;
  QAction *box = menu_action(window, "View", "Show Bounding Box");
  QAction *flagged = menu_action(window, "View", "Show Flagged Soundings");
  check(box != nullptr && box->isChecked() && flagged != nullptr && flagged->isChecked() &&
            menu_action(window, "View", "Show No Profiles")->isChecked() &&
            menu_action(window, "View", "Color by Flag State")->isChecked(),
        "the View menu shows the initial state (box, flagged, no profiles, colour by flag)");
  flagged->trigger();
  menu_action(window, "View", "Show All Profiles")->trigger();
  menu_action(window, "View", "Color by Topography")->trigger();
  mbvq_c_soundings_state(&state);
  check(!state.view[MBVQ_SDG_VIEW_FLAGGED] && state.profiles == MBVQ_SDG_PROFILES_ALL &&
            state.color == MBVQ_SDG_COLOR_TOPO,
        "View menu items set flagged, profiles and colouring");

  /* the Action menu calls the program */
  menu_action(window, "Action", "Color Unflagged Soundings Red")->trigger();
  check(color_notified == MBV_COLOR_RED, "Color Unflagged Soundings Red asks the program for red");
  menu_action(window, "Action", "Filter by sparse voxels (1 X cell, n<10)")->trigger();
  check(sparse_cells == 1 && sparse_n == 10, "the first sparse voxel filter asks for 1 cell, n<10");
  menu_action(window, "Action", "Apply Bias Values to Grid")->trigger();
  check(biasapply_calls == 1, "Apply Bias Values to Grid calls the program");
  int actions = 0;
  for (QAction *top : window->menuBar()->actions())
    if (top->text() == QStringLiteral("Action"))
      for (QAction *action : top->menu()->actions())
        actions += action->isSeparator() ? 0 : 1;
  check(actions == MBVQ_SDG_ACTIONS, "the Action menu has the 21 Motif items");

  /* optimisation: the program returns the new roll bias, the slider shows it */
  QSlider *roll = slider_after(window, "Roll Bias (degrees)");
  check(roll != nullptr, "a Roll Bias slider");
  menu_action(window, "Action", "Optimize Bias Values (roll)")->trigger();
  check(optimize_mode == MB3DSDG_OPTIMIZEBIASVALUES_R && roll != nullptr && roll->value() == 125 &&
            roll->minimum() == 0 && roll->maximum() == 200,
        "Optimize Bias Values (roll) sets the slider to 1.25 with the Motif range 0 to 2");

  /* a released slider value reaches the program; at an end the range moves */
  QSlider *pitch = slider_after(window, "Pitch Bias (degrees)");
  const int calls = bias_calls;
  pitch->setValue(50);
  check(bias_calls == calls + 1 && std::fabs(bias_values[1] - 0.5) < 1e-9 && std::fabs(bias_values[0] - 1.25) < 1e-9,
        "a pitch slider value of 0.50 reaches the program with the roll bias 1.25");
  pitch->setValue(pitch->maximum());
  check(pitch->minimum() == 0 && pitch->maximum() == 200, "at the end of its range the slider recentres on the value");
  QSlider *snell = slider_after(window, "Snell Correction");
  check(snell != nullptr && snell->value() == 10000 && snell->minimum() == 9900 && snell->maximum() == 10100,
        "the Snell slider runs from 0.99 to 1.01 at 1.0000");

  /* Dismiss hides the window and tells the program; open shows it again */
  QAction *dismiss = menu_action(window, "Dismiss", "Dismiss");
  check(dismiss != nullptr, "the Dismiss menu has Dismiss");
  uint64_t before_dismiss[3] = {0, 0, 0}, after_dismiss[3] = {0, 0, 0};
  {
    int error = MB_ERROR_NO_ERROR;
    mb3dsoundings_getmarks(0, &before_dismiss[0], &before_dismiss[1], &before_dismiss[2], &error);
  }
  if (dismiss != nullptr)
    dismiss->trigger();
  check(!window->isVisible() && dismiss_calls == 1, "Dismiss hides the window and calls the dismiss notify");
  {
    int error = MB_ERROR_NO_ERROR;
    mb3dsoundings_getmarks(0, &after_dismiss[0], &after_dismiss[1], &after_dismiss[2], &error);
  }
  check(after_dismiss[0] > before_dismiss[0] && after_dismiss[1] > before_dismiss[1] &&
            after_dismiss[2] > before_dismiss[2],
        "Dismiss raises every mark: the program releases the soundings");
  open_soundings();
  check(mbvq::soundings_window() == window && window->isVisible(), "a second open shows the same window");
  window->close();
  check(!window->isVisible() && dismiss_calls == 2, "closing the window works as Dismiss");

  /* end: the window is destroyed */
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
  check(mbvq::soundings_window() == nullptr, "mb3dsoundings_end destroys the window");
  open_soundings();
  check(mbvq::soundings_window() != nullptr && mbvq::soundings_window()->isVisible(),
        "open after end creates and shows a window again");
  mb3dsoundings_end(0, &error);
  return failures == 0 ? 0 : 1;
}

/*------------------------------------------------------------------------------*/
bool wait_drawn(int timeout_ms) {
  QElapsedTimer timer;
  timer.start();
  while (timer.elapsed() < timeout_ms) {
    for (const auto &s : storage)
      if (s.winx != 0 || s.winy != 0)
        return true;
    spin(50);
  }
  return false;
}

/* the unflagged sounding nearest to the middle of the view (window pixels) */
int middle_sounding(QWidget *view) {
  const double ratio = view->devicePixelRatioF();
  const double cx = 0.5 * view->width() * ratio, cy = 0.5 * view->height() * ratio;
  int best = -1;
  double best_d = 1e30;
  for (size_t i = 0; i < storage.size(); i++) {
    if (storage[i].beamflag != MB_FLAG_NONE)
      continue;
    const double d = std::hypot(storage[i].winx - cx, storage[i].winy - cy);
    if (d < best_d) {
      best_d = d;
      best = static_cast<int>(i);
    }
  }
  return best;
}

QPointF window_position(QWidget *view, const struct mb3dsoundings_sounding_struct &s) {
  const double ratio = view->devicePixelRatioF();
  const int height = static_cast<int>(std::lround(view->height() * ratio));
  return QPointF(s.winx / ratio, (height - 1 - s.winy) / ratio);
}

int test_gl() {
  make_soundings(100, 50);
  open_soundings();
  mbvq::SoundingsWindow *window = mbvq::soundings_window();
  auto *view = window != nullptr ? qobject_cast<QOpenGLWidget *>(window->widget()->view()) : nullptr;
  check(view != nullptr, "the soundings view is an OpenGL widget");
  if (view == nullptr)
    return 1;
  check(wait_drawn(10000), "the soundings are drawn and have screen positions");
  const QImage image = view->grabFramebuffer();
  int dark = 0;
  for (int y = 0; y < image.height(); y += 3)
    for (int x = 0; x < image.width(); x += 3)
      if (qGray(image.pixel(x, y)) < 128)
        dark++;
  check(dark > 100, "the drawing shows soundings (" + std::to_string(dark) + " dark samples)");
  if (const char *snapshot = std::getenv("MBVIEW_QT_TEST_SNAPSHOT"))
    image.save(QString::fromLocal8Bit(snapshot));

  /* Toggle: a click flags the sounding under the mouse */
  const int i = middle_sounding(view);
  check(i >= 0, "an unflagged sounding near the middle of the view");
  if (i < 0)
    return 1;
  const struct mb3dsoundings_sounding_struct target = storage[i];
  click(view, window_position(view, target));
  spin(100);
  check(edit_calls == 1 && edit_iping == target.iping && edit_ibeam == target.ibeam,
        "a click in Toggle mode edits that sounding (ping " + std::to_string(edit_iping) + " beam " +
            std::to_string(edit_ibeam) + ")");
  check(mb_beam_check_flag_flagged(storage[i].beamflag), "the sounding is now flagged");
  click(view, window_position(view, storage[i]));
  spin(100);
  check(edit_calls == 2 && mb_beam_ok(storage[i].beamflag), "a second click unflags it");

  /* Info: a click asks the program for the sounding information */
  mbvq_c_soundings_edit_mode(MBVQ_SDG_EDIT_INFO);
  click(view, window_position(view, storage[i]));
  spin(100);
  check(info_calls >= 1 && label_containing(window, "Sounding: file 0").contains(QStringLiteral("beam")),
        "a click in Info mode shows the sounding information in the status line");
  QSlider *any_slider = window->findChild<QSlider *>();
  check(any_slider != nullptr && !any_slider->isVisible(), "the sliders give way to the sounding information");
  mbvq_c_soundings_edit_mode(MBVQ_SDG_EDIT_TOGGLE);
  check(any_slider != nullptr && any_slider->isVisible(), "the sliders come back in Toggle mode");

  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
  check(mbvq::soundings_window() == nullptr, "mb3dsoundings_end destroys the window");
  return failures == 0 ? 0 : 1;
}

/*------------------------------------------------------------------------------*/
int test_time(int n) {
  const int nbeams = 400;
  make_soundings(std::max(1, n / nbeams), nbeams);
  open_soundings();
  mbvq::SoundingsWindow *window = mbvq::soundings_window();
  auto *view = window != nullptr ? qobject_cast<QOpenGLWidget *>(window->widget()->view()) : nullptr;
  if (view == nullptr || !wait_drawn(20000)) {
    std::printf("FAIL: not drawn\n");
    return 1;
  }
  for (bool box : {true, false, true, false}) {
    mb3dsoundings_setviewtoggle(kViewToggleBoundingBox, box);
    view->grabFramebuffer();
    QElapsedTimer timer;
    timer.start();
    const int frames = 5;
    for (int f = 0; f < frames; f++)
      view->grabFramebuffer();
    std::printf("%d soundings, bounding box %s: %.1f ms per drawing\n", soundings.num_soundings, box ? "on " : "off",
                static_cast<double>(timer.elapsed()) / frames);
  }
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
  return 0;
}

}  // namespace

int main(int argc, char **argv) {
  std::setvbuf(stdout, nullptr, _IOLBF, 0);
  const char *mode = argc > 1 ? argv[1] : "";
  const bool null = std::strcmp(mode, "--null") == 0;
  if (!null && std::strcmp(mode, "--gl") != 0 && std::strcmp(mode, "--time") != 0 && std::strcmp(mode, "--show") != 0) {
    std::fprintf(stderr, "usage: mbview_qt_soundings_test --null | --gl | --time N | --show [N]\n");
    return 2;
  }
  mbvq::set_backend(null ? mbvq::Backend::Null : mbvq::Backend::LegacyGL);
  QApplication app(argc, argv);
  int error = MB_ERROR_NO_ERROR;
  mbview_startup(0, nullptr, nullptr, &error);
  mb3dsoundings_startup(0, nullptr, nullptr, &error);

  if (null)
    return test_null();
  if (std::strcmp(mode, "--gl") == 0)
    return test_gl();
  if (std::strcmp(mode, "--time") == 0)
    return test_time(argc > 2 ? std::atoi(argv[2]) : 200000);
  const int n = argc > 2 ? std::atoi(argv[2]) : 20000;
  make_soundings(std::max(1, n / 200), 200);
  /* Dismiss only hides the window, as in Motif, so it ends the program here */
  quit_on_dismiss = true;
  open_soundings();
  /* MBVIEW_QT_TEST_DISMISS: press Dismiss after half a second (a check that
     Dismiss ends this program) */
  if (std::getenv("MBVIEW_QT_TEST_DISMISS") != nullptr)
    QTimer::singleShot(500, qApp, [] { mbvq_c_soundings_dismiss(); });
  const int status = app.exec();
  mb3dsoundings_end(0, &error);
  return status;
}
