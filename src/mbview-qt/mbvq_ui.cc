/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_ui.cc  11/7/2026
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
 * The Qt half of the user interface functions (mbvq_bridge.h) and the
 * public functions of mbvq.h.
 */

#include <QApplication>
#include <QRegularExpression>
#include <QString>
#include <QTimer>
#include <vector>

#include "mbvq.h"
#include "mbvq_backend.h"
#include "mbvq_bridge.h"
#include "mbvq_dialogs.h"
#include "mbvq_lists.h"
#include "mbvq_window.h"

namespace mbvq {

namespace {

Backend current_backend = Backend::LegacyGL;
bool backend_installed = false;
MapWindow *windows[MBVQ_MAX_WINDOWS] = {};

/* Closed windows, hidden. They are deleted at a safe point, never by a posted
   deferred deletion: on macOS the Quit menu calls exit() from inside the event
   loop, and a window that held an OpenGL view cannot be deleted then. */
std::vector<MapWindow *> closed_windows;
bool quit_connected = false;

void delete_closed_windows() {
  for (MapWindow *window : closed_windows)
    delete window;
  closed_windows.clear();
}

void install_backend() {
  if (backend_installed)
    return;
  if (current_backend == Backend::LegacyGL)
    gl_backend::install();
  else
    null_backend::install();
  backend_installed = true;
}

MapWindow *window_of(size_t instance) { return instance < MBVQ_MAX_WINDOWS ? windows[instance] : nullptr; }

}  // namespace

void set_backend(Backend backend) {
  current_backend = backend;
  backend_installed = false;
  if (backend == Backend::LegacyGL)
    gl_backend::configure();
  install_backend();
}

Backend backend() { return current_backend; }

QWidget *window(size_t instance) { return window_of(instance); }

QWidget *map_view(size_t instance) {
  MapWindow *window = window_of(instance);
  return window != nullptr ? window->map_view() : nullptr;
}

QString label_text(const char *bxstring) {
  /* The quoted parts are the text; ":t" before a part ends the line after
     it, as in the Motif (Builder Xcessory) compound strings. Font tags are
     dropped. */
  QString text;
  static const QRegularExpression part(QStringLiteral("(:t)?\"([^\"]*)\""));
  auto match = part.globalMatch(QString::fromLocal8Bit(bxstring));
  while (match.hasNext()) {
    const QRegularExpressionMatch m = match.next();
    text += m.captured(2);
    if (!m.captured(1).isEmpty())
      text += QLatin1Char('\n');
  }
  while (text.endsWith(QLatin1Char('\n')))
    text.chop(1);
  return text;
}

}  // namespace mbvq

using mbvq::window_of;

extern "C" {

bool mbvq_qt_drawable(void) {
  mbvq::install_backend();
  return mbvq::current_backend == mbvq::Backend::LegacyGL;
}

void mbvq_qt_open_window(size_t instance) {
  if (instance >= MBVQ_MAX_WINDOWS)
    return;
  mbvq::install_backend();
  /* a safe point: called by the program, not from an event of a closed window */
  mbvq::delete_closed_windows();
  if (!mbvq::quit_connected) {
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, qApp, [] { mbvq::delete_closed_windows(); });
    mbvq::quit_connected = true;
  }
  if (mbvq::windows[instance] == nullptr)
    mbvq::windows[instance] = new mbvq::MapWindow(instance);
  mbvq::windows[instance]->refresh_modes();
  mbvq::windows[instance]->show();
}

void mbvq_qt_close_window(size_t instance) {
  if (instance >= MBVQ_MAX_WINDOWS || mbvq::windows[instance] == nullptr)
    return;
  /* Called from mbview_destroy(), possibly inside an event of this window: the
     window is hidden now and deleted at a safe point (see closed_windows). Its
     OpenGL map view and context are freed now. */
  mbvq::MapWindow *window = mbvq::windows[instance];
  mbvq::windows[instance] = nullptr;
  window->hide();
  window->delete_map_view();
  mbvq::closed_windows.push_back(window);
}

void mbvq_qt_message(size_t instance, const char *message, bool on) {
  if (mbvq::MapWindow *window = window_of(instance))
    window->show_message(QString::fromLocal8Bit(message), on);
}

void mbvq_qt_status(size_t instance, const char *message) {
  if (mbvq::MapWindow *window = window_of(instance))
    window->set_status(QString::fromLocal8Bit(message));
}

void mbvq_qt_label(size_t instance, int label, const char *bxstring) {
  if (mbvq::MapWindow *window = window_of(instance))
    window->set_label(label, mbvq::label_text(bxstring));
}

void mbvq_qt_modes_changed(size_t instance) {
  if (mbvq::MapWindow *window = window_of(instance))
    window->refresh_modes();
}

void mbvq_qt_sensitivity_changed(size_t instance) {
  if (mbvq::MapWindow *window = window_of(instance))
    window->refresh_modes();
}

void mbvq_qt_list_changed(int list) { mbvq::list_changed(list); }

void mbvq_qt_lists_closed(void) { mbvq::close_lists(); }

void mbvq_qt_add_action(size_t instance, int index, const char *label) {
  if (mbvq::MapWindow *window = window_of(instance))
    window->add_action(index, QString::fromLocal8Bit(label));
}

void mbvq_qt_add_action_separator(size_t instance) {
  if (mbvq::MapWindow *window = window_of(instance))
    window->add_action_separator();
}

void mbvq_qt_action_enabled(size_t instance, int index, bool enabled) {
  if (mbvq::MapWindow *window = window_of(instance))
    window->set_action_enabled(index, enabled);
}

void mbvq_qt_bell(void) { QApplication::beep(); }

void mbvq_qt_cursor(size_t instance, int cursor) {
  if (mbvq::MapWindow *window = window_of(instance))
    window->set_map_cursor(cursor);
}

void mbvq_qt_values_changed(size_t instance, int values) {
  if (mbvq::MapWindow *window = window_of(instance))
    window->dialogs()->values_changed(values);
}

void mbvq_qt_schedule_work(size_t instance, int delay_ms) {
  QTimer::singleShot(delay_ms, qApp, [instance] { mbvq_c_work(instance); });
}

}  // extern "C"
