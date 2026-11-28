/*--------------------------------------------------------------------
 *    The MB-system:  mbview_qt_embed_test.cc  11/10/2026
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
 * Maps and the 3D soundings editor inside another program's window, with no
 * windows of their own: what a future unified application will use. Maps in
 * docks of one main window: float, attach, two maps, close one, open another.
 *
 *   mbview_qt_embed_test --null   no drawing (null backend), for ctest
 *   mbview_qt_embed_test --gl     also checks the drawing with OpenGL: the
 *                                 contours survive moving and floating the
 *                                 map; needs a display
 *   mbview_qt_embed_test --rhi    also checks the drawing with QRhi (where it
 *                                 is built; 77, skipped, elsewhere): the
 *                                 terrain is drawn docked, floating (a new
 *                                 QRhi) and docked again, and a map closed
 *                                 while its dock floats; needs a display
 *
 * The modes that draw have a watchdog. The summary comes after the main
 * window is destroyed, so that a crash there comes before it.
 */

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QEventLoop>
#include <QImage>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QOpenGLWidget>
#include <QTimer>
#include <QWindow>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

extern "C" {
#include "mb_define.h"
#include "mb_status.h"
#include "mbv_xtypes.h"
#include "mbview.h"
}
#include "mbvq.h"
#include "mbvq_bridge.h"
#include "mbvq_soundings.h"
#include "mbvq_window.h"
#ifdef MBVQ_HAVE_RHI
#include <QRhiWidget>

#include "mbvq_rhi.h"
#endif

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

/* the test ends after seconds even if it hangs */
void start_watchdog(int seconds, const char *mode) {
  std::thread([seconds, mode] {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    std::fprintf(stderr, "mbview_qt_embed_test %s: watchdog, stopped after %d s\n", mode, seconds);
    std::fflush(stdout);
    std::_Exit(4);
  }).detach();
}

/* the window of a view is exposed (a first frame that does not come is then
   not the window system's doing) */
bool exposed(QWidget *view) {
  const QWindow *window = view != nullptr ? view->window()->windowHandle() : nullptr;
  return window != nullptr && window->isExposed();
}

int dismissed = 0;
int dismiss_map(size_t instance) {
  (void)instance;
  dismissed++;
  return MB_SUCCESS;
}

/* an mbview instance on a small synthetic grid */
bool open_map(size_t *instance) {
  const int columns = 241, rows = 181;
  const double xmin = 10.0, xmax = 10.2, ymin = 54.0, ymax = 54.15;
  const double dx = (xmax - xmin) / (columns - 1), dy = (ymax - ymin) / (rows - 1);
  std::vector<float> grid(static_cast<size_t>(columns) * rows);
  double zmin = 1.0e9, zmax = -1.0e9;
  for (int i = 0; i < columns; i++)
    for (int j = 0; j < rows; j++) {
      const double a = (xmin + i * dx - 10.1) / 0.04, b = (ymin + j * dy - 54.075) / 0.03;
      const double z = -1000.0 + 300.0 * std::exp(-(a * a + b * b));
      grid[static_cast<size_t>(i) * rows + j] = static_cast<float>(z);
      zmin = std::min(zmin, z);
      zmax = std::max(zmax, z);
    }
  int error = MB_ERROR_NO_ERROR;
  if (mbview_init(0, instance, &error) != MB_SUCCESS)
    return false;
  char title[] = "embedded map";
  char geographic[] = "EPSG:4326";
  char display[] = "LTM10.10000/54.07500";
  mbview_setwindowparms(0, *instance, &dismiss_map, title, 100, 100, 640, 480, 100, 500, 5, 1, &error);
  mbview_setviewcontrols(0, *instance, MBV_DISPLAY_2D, MBV_MOUSE_MOVE, MBV_GRID_VIEW_PRIMARY, false, false, false,
                         MBV_SHADE_VIEW_SLOPE, MBV_SHADE_VIEW_NONE, MBV_SHADE_VIEW_NONE, MBV_VIEW_OFF, MBV_VIEW_OFF,
                         MBV_VIEW_OFF, MBV_VIEW_OFF, MBV_VIEW_OFF, MBV_VIEW_OFF, MBV_VIEW_OFF, 1.0, 90.0, 0.0, 90.0, 0.0,
                         1.0, 5.0, 90.0, 1.0, 1.0, 0.0, MBV_COLORTABLE_NORMAL, 10.0, MBV_PROJECTION_PROJECTED, display,
                         &error);
  mbview_setprimarygrid(0, *instance, MBV_PROJECTION_GEOGRAPHIC, geographic, -99999.0f, columns, rows, zmin, zmax, xmin, xmax,
                        ymin, ymax, dx, dy, grid.data(), &error);
  mbview_setprimarycolortable(0, *instance, MBV_COLORTABLE_HAXBY, MBV_COLORTABLE_NORMAL, zmin, zmax, &error);
  mbview_setslopecolortable(0, *instance, MBV_COLORTABLE_HAXBY, MBV_COLORTABLE_REVERSED, 0.0, 0.5, &error);
  return mbview_open(0, *instance, &error) == MB_SUCCESS;
}

struct mb3dsoundings_struct soundings;
std::vector<struct mb3dsoundings_sounding_struct> storage;
int soundings_dismissed = 0;
void dismiss_soundings() { soundings_dismissed++; }

void open_soundings() {
  storage.assign(400, {});
  for (int i = 0; i < 400; i++) {
    auto &s = storage[i];
    s.iping = i / 20;
    s.ibeam = i % 20;
    s.beamcolor = MBV_COLOR_BLACK;
    s.x = -20.0 + 2.0 * (i % 20);
    s.y = -20.0 + 2.0 * (i / 20);
    s.z = -1000.0 + 0.1 * s.x;
  }
  std::memset(&soundings, 0, sizeof(soundings));
  soundings.xmin = soundings.ymin = -20.0;
  soundings.xmax = soundings.ymax = 20.0;
  soundings.bearing = 90.0;
  soundings.sinbearing = 1.0;
  soundings.scale = soundings.zscale = 0.03;
  soundings.num_soundings = soundings.num_soundings_alloc = soundings.num_soundings_unflagged = 400;
  soundings.soundings = storage.data();
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_open(0, &soundings, &error);
  mb3dsoundings_set_dismiss_notify(0, &dismiss_soundings, &error);
}

/* the pixels of a QRhi map view that are not white: the terrain;
   grabFramebuffer() draws a new frame for it, a fill with its side effects */
int terrain_pixels(size_t instance) {
#ifdef MBVQ_HAVE_RHI
  auto *view = qobject_cast<QRhiWidget *>(mbvq::map_view(instance));
  if (view == nullptr)
    return -1;
  const QImage image = view->grabFramebuffer();
  int n = 0;
  for (int y = 0; y < image.height(); y++)
    for (int x = 0; x < image.width(); x++)
      if ((image.pixel(x, y) & 0xffffffu) != 0xffffffu)
        n++;
  return n;
#else
  (void)instance;
  return -1;
#endif
}

/* the QRhi map view holds a renderer, made for its current QRhi */
bool rhi_renderer(size_t instance) {
#ifdef MBVQ_HAVE_RHI
  return mbvq::view_state(mbvq::map_view(instance)).renderer;
#else
  (void)instance;
  return false;
#endif
}

/* callbacks of QRhiWidget that view destructors removed from a QRhi their view
   had left (QTBUG-151834) */
int stale_callbacks() {
#ifdef MBVQ_HAVE_RHI
  return mbvq::stale_callbacks_removed();
#else
  return -1;
#endif
}

/* dark pixels of a map view: the contours */
int dark_pixels(size_t instance) {
  auto *view = qobject_cast<QOpenGLWidget *>(mbvq::map_view(instance));
  const QImage image = view->grabFramebuffer();
  int n = 0;
  for (int y = 0; y < image.height(); y++)
    for (int x = 0; x < image.width(); x++)
      if (qRed(image.pixel(x, y)) < 45 && qGreen(image.pixel(x, y)) < 45 && qBlue(image.pixel(x, y)) < 45)
        n++;
  return n;
}

void wait_drawn(size_t instance) {
  for (int i = 0; i < 400 && mbvq_c_lastdrawrez(instance) != MBVQ_REZ_FULL; i++)
    spin(50);
}

int run(bool gl, bool rhi) {
  /* the backends that draw */
  const bool draws = gl || rhi;
  QMainWindow host;
  host.setWindowTitle(QStringLiteral("host"));
  host.setCentralWidget(new QLabel(QStringLiteral("the program")));
  host.resize(1500, 900);
  host.show();
  spin(100);

  mbvq::start(0);

  /* the host decides where the maps go */
  std::vector<QDockWidget *> docks;
  std::vector<mbvq::MapWidget *> placed;
  mbvq::set_map_placement([&](mbvq::MapWidget *map) {
    auto *dock = new QDockWidget(map->title());
    dock->setWidget(map);
    host.addDockWidget(Qt::LeftDockWidgetArea, dock);
    dock->show();
    docks.push_back(dock);
    placed.push_back(map);
  });

  size_t first = MBV_NO_WINDOW;
  check(open_map(&first), "the first map opens");
  check(placed.size() == 1 && placed[0]->instance() == first, "the host places the map");
  mbvq::MapWidget *map = placed[0];
  check(mbvq::map_widget(first) == map && mbvq::window(first) == map, "a placed map has no window of its own");
  check(map->window() == &host && map->isVisible(), "the map is visible inside the host");
  check(map->title() == QStringLiteral("embedded map"), "the title comes from mbview_setwindowparms");
  QStringList names;
  for (QMenu *menu : map->menus())
    names << menu->title();
  check(names == QStringList({"View", "Controls", "Mouse", "Action", "Dismiss"}), "menus(): View, Controls, Mouse, Action, Dismiss");
  check(map->controlPanel() != nullptr && map->isAncestorOf(map->controlPanel()), "the left panel is available to the host");

  QString title_seen, status_seen;
  size_t closed_instance = MBV_NO_WINDOW;
  QObject::connect(map, &mbvq::MapWidget::titleChanged, [&](const QString &t) { title_seen = t; });
  QObject::connect(map, &mbvq::MapWidget::statusMessage, [&](const QString &t) { status_seen = t; });
  QObject::connect(map, &mbvq::MapWidget::closed, [&](size_t i) { closed_instance = i; });
  map->setTitle(QStringLiteral("new title"));
  check(title_seen == QStringLiteral("new title") && map->title() == title_seen, "setTitle and titleChanged");
  mbvq_qt_status(first, "working");
  check(status_seen == QStringLiteral("working"), "the status goes to the host");
  mbvq_qt_status(first, "");

  check(map->findChild<QMenuBar *>() == nullptr, "no menu bar inside the map by default");
  map->setMenuBarShown(true);
  auto *bar = map->findChild<QMenuBar *>();
  check(bar != nullptr && bar->isVisible() && !bar->isNativeMenuBar() && bar->actions().size() == 5, "setMenuBarShown(true): a bar inside the map");
  map->setMenuBarShown(false);
  check(!bar->isVisible(), "setMenuBarShown(false) hides it");

  int dark_docked = 0;
  QOpenGLContext *context = nullptr;
  if (gl) {
    host.resizeDocks({docks[0]}, {900}, Qt::Horizontal);
    wait_drawn(first);
    mbvq_c_view(first, MBVQ_VIEW_CONTOUR, true);
    spin(1500);
    wait_drawn(first);
    context = qobject_cast<QOpenGLWidget *>(mbvq::map_view(first))->context();
    dark_docked = dark_pixels(first);
    check(dark_docked > 0, "OpenGL: contours are drawn in the dock (" + std::to_string(dark_docked) + " dark pixels)");
  }
  if (rhi) {
    host.resizeDocks({docks[0]}, {900}, Qt::Horizontal);
    wait_drawn(first);
    const int docked = terrain_pixels(first);
    check(docked > 0 && rhi_renderer(first), "QRhi: the terrain is drawn in the dock (" + std::to_string(docked) +
                                                 " pixels, window exposed: " +
                                                 std::to_string(exposed(mbvq::map_view(first))) + ")");
  }

  /* float and attach again */
  docks[0]->setFloating(true);
  spin(draws ? 1500 : 100);
  check(docks[0]->isFloating() && map->isVisible() && mbvq::map_widget(first) == map, "the dock floats with the map");
  if (gl) {
    mbvq_c_resetview(first);
    spin(1500);
    wait_drawn(first);
    check(qobject_cast<QOpenGLWidget *>(mbvq::map_view(first))->context() == context, "OpenGL: floating keeps the context");
    check(dark_pixels(first) > 0, "OpenGL: the contours survive floating");
  }
  if (rhi) {
    mbvq_c_resetview(first);
    spin(1500);
    wait_drawn(first);
    check(terrain_pixels(first) > 0 && rhi_renderer(first), "QRhi: floating, the terrain is drawn with the new QRhi");
  }
  docks[0]->setFloating(false);
  spin(draws ? 1500 : 100);
  check(!docks[0]->isFloating() && map->isVisible(), "the dock attaches again");
  if (gl) {
    mbvq_c_resetview(first);
    spin(1500);
    wait_drawn(first);
    check(qobject_cast<QOpenGLWidget *>(mbvq::map_view(first))->context() == context, "OpenGL: attaching keeps the context");
    check(dark_pixels(first) > 0, "OpenGL: the contours survive attaching");
  }
  if (rhi) {
    mbvq_c_resetview(first);
    spin(1500);
    wait_drawn(first);
    check(terrain_pixels(first) > 0 && rhi_renderer(first), "QRhi: attached again, the terrain is drawn");
  }

  /* a second map, close one, open another */
  size_t second = MBV_NO_WINDOW;
  check(open_map(&second) && second != first && placed.size() == 2, "a second map in another dock");
  spin(draws ? 800 : 50);
  host.resize(1500, 901);
  mbvq::start(0);
  check(mbvq::map_widget(first) == map && mbvq::map_widget(second) == placed[1], "start() again changes nothing");

  int dismissed_before = dismissed;
  map->dismiss();
  spin(50);
  check(closed_instance == first && dismissed == dismissed_before + 1, "dismiss() tells the host and the program");
  check(mbvq::map_widget(first) == nullptr && mbvq::window(first) == nullptr, "the instance is free");
  check(host.isVisible() && mbvq::map_widget(second) != nullptr, "the host and the other map stay");
  delete docks[0]; /* the host removes its dock; the library still deletes the map safely */
  docks[0] = nullptr;

  size_t third = MBV_NO_WINDOW;
  check(open_map(&third) && third == first && placed.size() == 3, "the next map reuses the instance");
  spin(draws ? 800 : 50);
  if (gl) {
    wait_drawn(third);
    check(mbvq::map_view(third)->isVisible(), "OpenGL: the new map draws");
  }
  if (rhi) {
    wait_drawn(third);
    check(terrain_pixels(third) > 0, "QRhi: the new map draws");

    /* a map closed while its dock floats: its view leaves the QRhi of the main
       window behind (QTBUG-151834, the workaround in the view's destructor
       removes QRhiWidget's callback from it); the main window is destroyed at
       the end of the test */
    docks[1]->setFloating(true);
    spin(1500);
    wait_drawn(second);
    const bool floated = docks[1]->isFloating() && terrain_pixels(second) > 0;
    const int removed = stale_callbacks();
    mbvq::map_widget(second)->dismiss();
    spin(100);
    delete docks[1];
    docks[1] = nullptr;
    spin(100);
    check(floated && mbvq::map_widget(second) == nullptr, "QRhi: a map closed while its dock floats");
    check(stale_callbacks() == removed + 1,
          "QRhi: its view removed the callback it left in the main window's QRhi (QTBUG-151834; " +
              std::to_string(stale_callbacks() - removed) + " removed)");
  }

  /* without a placement a map gets its own window, as in the standalone programs */
  mbvq::set_map_placement(nullptr);
  size_t alone = MBV_NO_WINDOW;
  check(open_map(&alone), "a map without a placement opens");
  auto *window = qobject_cast<QMainWindow *>(mbvq::window(alone));
  check(window != nullptr && window->isVisible() && mbvq::map_widget(alone)->window() == window,
        "it has a window of its own");
  check(window != nullptr && window->windowTitle() == QStringLiteral("embedded map") && window->menuBar()->actions().size() == 5,
        "with the title and the menus");
  window->close();
  spin(50);
  check(mbvq::window(alone) == nullptr, "closing that window dismisses the map");

  /* the 3D soundings editor */
  QDockWidget *soundings_dock = nullptr;
  mbvq::SoundingsWidget *editor = nullptr;
  mbvq::set_soundings_placement([&](mbvq::SoundingsWidget *widget) {
    soundings_dock = new QDockWidget(widget->title());
    soundings_dock->setWidget(widget);
    host.addDockWidget(Qt::BottomDockWidgetArea, soundings_dock);
    soundings_dock->show();
    editor = widget;
  });
  int opened = 0, closed = 0;
  open_soundings();
  check(editor != nullptr && mbvq::soundings_widget() == editor && mbvq::soundings_window() == nullptr, "the host places the 3D editor");
  check(editor != nullptr && editor->isVisible() && editor->window() == &host, "it is visible inside the host");
  if (editor == nullptr)
    return 1;
  QObject::connect(editor, &mbvq::SoundingsWidget::opened, [&] { opened++; });
  QObject::connect(editor, &mbvq::SoundingsWidget::closed, [&] { closed++; });
  names.clear();
  for (QMenu *menu : editor->menus())
    names << menu->title();
  check(names == QStringList({"View", "Mouse", "Action", "Dismiss"}), "menus(): View, Mouse, Action, Dismiss");
  editor->setMenuBarShown(true);
  check(editor->findChild<QMenuBar *>() != nullptr && editor->findChild<QMenuBar *>()->isVisible(), "its menus in a bar inside it");
  editor->dismiss();
  spin(20);
  check(closed == 1 && soundings_dismissed == 1 && !editor->isVisible(), "dismiss() tells the host and the program");
  open_soundings();
  check(opened == 1 && editor->isVisible() && mbvq::soundings_widget() == editor, "opening again shows the same widget");
  check(editor->prepareToClose() && closed == 2, "prepareToClose dismisses the editor");
  int error = MB_ERROR_NO_ERROR;
  mb3dsoundings_end(0, &error);
  spin(20);
  check(mbvq::soundings_widget() == nullptr, "mb3dsoundings_end removes the widget");

  mbview_quit(0, &error);
  spin(20);
  /* the main window and its docks are destroyed on return */
  return failures == 0 ? 0 : 1;
}

}  // namespace

int main(int argc, char **argv) {
  std::setvbuf(stdout, nullptr, _IOLBF, 0);
  const bool gl = argc > 1 && std::strcmp(argv[1], "--gl") == 0;
  const bool rhi = argc > 1 && std::strcmp(argv[1], "--rhi") == 0;
  if (!gl && !rhi && !(argc > 1 && std::strcmp(argv[1], "--null") == 0)) {
    std::fprintf(stderr, "usage: mbview_qt_embed_test --null|--gl|--rhi\n");
    return 2;
  }
  /* a build without QRhi would draw with OpenGL instead */
  if (rhi && !mbvq::backend_built(mbvq::Backend::Rhi)) {
    std::printf("skipped: this build has no QRhi drawing\n");
    return 77;
  }
  /* the modes that draw end after 150 s even if they hang (their own waits
     add up to about 110 s at most) */
  if (gl || rhi)
    start_watchdog(150, argv[1]);
  mbvq::set_backend(gl ? mbvq::Backend::LegacyGL : rhi ? mbvq::Backend::Rhi : mbvq::Backend::Null);
  int qt_argc = 1;
  QApplication app(qt_argc, argv);
  const int result = run(gl, rhi);
  std::printf("%d failures\n", failures);
  std::fflush(stdout);
  return result;
}
