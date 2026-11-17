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
#include <QPointer>
#include <QRegularExpression>
#include <QString>
#include <QTimer>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
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
RhiApi current_api = RhiApi::Default;
bool backend_installed = false;
bool fill_active_flag = false;
double point_size = 3.0; /* the point-size control of the 3D soundings view */
MapWidget *widgets[MBVQ_MAX_WINDOWS] = {};
MapWindow *windows[MBVQ_MAX_WINDOWS] = {}; /* none for a map placed by a host */
std::function<void(MapWidget *)> map_placement;

/* Closed windows or widgets, hidden. They are deleted at a safe point, never by
   a posted deferred deletion: on macOS the Quit menu calls exit() from inside
   the event loop, and a window that held an OpenGL view cannot be deleted then.
   A host may have deleted the parent of a placed widget before that. */
std::vector<QPointer<QWidget>> closed_windows;
bool quit_connected = false;

void delete_closed_windows() {
  for (const QPointer<QWidget> &window : closed_windows)
    delete window.data();
  closed_windows.clear();
}

void install_backend() {
  if (backend_installed)
    return;
  if (current_backend == Backend::LegacyGL)
    gl_backend::install();
#ifdef MBVQ_HAVE_RHI
  else if (current_backend == Backend::Rhi)
    rhi_backend::install();
#endif
  else
    null_backend::install();
  backend_installed = true;
}

/* the MB_RENDERER names of the QRhi APIs this platform can have */
std::string possible_api_names() {
  const struct {
    RhiApi api;
    const char *name;
  } names[] = {{RhiApi::Metal, "metal"},   {RhiApi::Direct3D11, "d3d11"}, {RhiApi::Direct3D12, "d3d12"},
               {RhiApi::Vulkan, "vulkan"}, {RhiApi::OpenGL, "opengl"}};
  std::string text;
  for (const auto &entry : names) {
    if (!rhi_api_possible(entry.api))
      continue;
    if (!text.empty())
      text += ", ";
    text += entry.name;
  }
  return text;
}

/* the API that Default stands for on this platform */
RhiApi platform_api() {
#if defined(__APPLE__)
  return RhiApi::Metal;
#elif defined(_WIN32)
  return RhiApi::Direct3D11;
#else
  return RhiApi::OpenGL;
#endif
}

MapWidget *widget_of(size_t instance) { return instance < MBVQ_MAX_WINDOWS ? widgets[instance] : nullptr; }

/* the standalone window if there is one, otherwise the widget */
QWidget *outermost(size_t instance) {
  if (instance >= MBVQ_MAX_WINDOWS)
    return nullptr;
  return windows[instance] != nullptr ? static_cast<QWidget *>(windows[instance]) : widgets[instance];
}

}  // namespace

void set_backend(Backend backend, RhiApi api) {
  if (!backend_built(backend)) {
    std::fprintf(stderr, "mbview-qt: this build has no QRhi drawing (it needs Qt 6.8 or later with Qt Shader Tools); "
                         "the views are drawn with OpenGL\n");
    backend = Backend::LegacyGL;
    api = RhiApi::Default;
  }
  current_backend = backend;
  current_api = api;
  backend_installed = false;
  /* only the OpenGL backend sets OpenGL attributes: QRhi sets none */
  if (backend == Backend::LegacyGL)
    gl_backend::configure();
  install_backend();
}

Backend backend() { return current_backend; }

bool backend_built(Backend backend) {
#ifdef MBVQ_HAVE_RHI
  (void)backend;
  return true;
#else
  return backend != Backend::Rhi;
#endif
}

void choose_renderer(const char *value, Backend *backend, RhiApi *api, std::string *message) {
  /* the default: the OpenGL backend (QRhi is chosen with MB_RENDERER) */
  *backend = Backend::LegacyGL;
  *api = RhiApi::Default;
  message->clear();
  if (value == nullptr || value[0] == '\0')
    return;
  const std::string name(value);
  if (!parse_renderer(value, backend, api)) {
    *backend = Backend::LegacyGL;
    *api = RhiApi::Default;
    *message = "mbview-qt: MB_RENDERER=" + name +
               " is not one of auto, metal, d3d11, d3d12, vulkan, opengl, legacy-gl and null; the views are drawn "
               "with OpenGL";
  }
  else if (*backend == Backend::Rhi && !backend_built(Backend::Rhi)) {
    *backend = Backend::LegacyGL;
    *api = RhiApi::Default;
    *message = "mbview-qt: this build has no QRhi drawing (it needs Qt 6.8 or later with Qt Shader Tools); the views "
               "are drawn with OpenGL";
  }
  else if (*backend == Backend::Rhi && !rhi_api_possible(*api)) {
    *api = RhiApi::Default;
    *message = "mbview-qt: MB_RENDERER=" + name + " is not possible on this platform, which has " +
               possible_api_names() + "; the views are drawn with QRhi and " + rhi_api_name(RhiApi::Default);
  }
}

void set_backend_from_environment() {
  Backend chosen = Backend::LegacyGL;
  RhiApi api = RhiApi::Default;
  std::string message;
  choose_renderer(std::getenv("MB_RENDERER"), &chosen, &api, &message);
  if (!message.empty())
    std::fprintf(stderr, "%s\n", message.c_str());
  set_backend(chosen, api);
}

bool parse_renderer(const char *value, Backend *backend, RhiApi *api) {
  const struct {
    const char *name;
    Backend backend;
    RhiApi api;
  } names[] = {{"auto", Backend::Rhi, RhiApi::Default},        {"metal", Backend::Rhi, RhiApi::Metal},
               {"d3d11", Backend::Rhi, RhiApi::Direct3D11},    {"d3d12", Backend::Rhi, RhiApi::Direct3D12},
               {"vulkan", Backend::Rhi, RhiApi::Vulkan},       {"opengl", Backend::Rhi, RhiApi::OpenGL},
               {"legacy-gl", Backend::LegacyGL, RhiApi::Default}, {"null", Backend::Null, RhiApi::Default}};
  if (value == nullptr)
    return false;
  for (const auto &entry : names) {
    if (std::strcmp(value, entry.name) == 0) {
      *backend = entry.backend;
      *api = entry.api;
      return true;
    }
  }
  return false;
}

bool rhi_api_possible(RhiApi api) {
  switch (api) {
  case RhiApi::Default:
    return true;
  case RhiApi::Metal:
#if defined(__APPLE__)
    return true;
#else
    return false;
#endif
  case RhiApi::Direct3D11:
  case RhiApi::Direct3D12:
#if defined(_WIN32)
    return true;
#else
    return false;
#endif
  case RhiApi::Vulkan:
#if QT_CONFIG(vulkan)
    return true;
#else
    return false;
#endif
  case RhiApi::OpenGL:
#if QT_CONFIG(opengl)
    return true;
#else
    return false;
#endif
  }
  return false;
}

RhiApi rhi_api() { return current_api; }

const char *rhi_api_name(RhiApi api) {
  switch (api == RhiApi::Default ? platform_api() : api) {
  case RhiApi::OpenGL:
    return "OpenGL";
  case RhiApi::Metal:
    return "Metal";
  case RhiApi::Vulkan:
    return "Vulkan";
  case RhiApi::Direct3D11:
    return "Direct3D 11";
  case RhiApi::Direct3D12:
    return "Direct3D 12";
  case RhiApi::Default:
    break;
  }
  return "QRhi";
}

QWidget *create_map_view(size_t instance, QWidget *parent) {
  if (current_backend == Backend::LegacyGL)
    return gl_backend::create_map_view(instance, parent);
#ifdef MBVQ_HAVE_RHI
  if (current_backend == Backend::Rhi)
    return rhi_backend::create_map_view(instance, parent);
#endif
  return null_backend::create_map_view(instance, parent);
}

QWidget *create_profile_view(size_t instance, QWidget *parent) {
  if (current_backend == Backend::LegacyGL)
    return gl_backend::create_profile_view(instance, parent);
#ifdef MBVQ_HAVE_RHI
  if (current_backend == Backend::Rhi)
    return rhi_backend::create_profile_view(instance, parent);
#endif
  return null_backend::create_profile_view(instance, parent);
}

QWidget *create_soundings_view(QWidget *parent) {
  if (current_backend == Backend::LegacyGL)
    return gl_backend::create_soundings_view(parent);
#ifdef MBVQ_HAVE_RHI
  if (current_backend == Backend::Rhi)
    return rhi_backend::create_soundings_view(parent);
#endif
  return null_backend::create_soundings_view(parent);
}

void cap_profile_view(QWidget *view) {
#ifdef MBVQ_HAVE_RHI
  if (current_backend == Backend::Rhi)
    rhi_backend::cap_profile_view(view);
#else
  (void)view;
#endif
}

void release_soundings_view(QWidget *view) {
#ifdef MBVQ_HAVE_RHI
  if (current_backend == Backend::Rhi)
    rhi_backend::release_soundings_view(view);
#else
  (void)view;
#endif
}

void set_soundings_point_size(double logical_px) {
  point_size = logical_px;
#ifdef MBVQ_HAVE_RHI
  if (current_backend == Backend::Rhi)
    rhi_backend::set_soundings_point_size(logical_px);
#endif
}

double soundings_point_size() { return point_size; }

bool fill_active() { return fill_active_flag; }

void set_fill_active(bool active) { fill_active_flag = active; }

void start(int verbose) { mbvq_c_start(verbose); }

void set_map_placement(std::function<void(MapWidget *)> place) { map_placement = std::move(place); }

QWidget *window(size_t instance) { return outermost(instance); }

MapWidget *map_widget(size_t instance) { return widget_of(instance); }

QWidget *map_view(size_t instance) {
  MapWidget *widget = widget_of(instance);
  return widget != nullptr ? widget->map_view() : nullptr;
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

using mbvq::widget_of;

extern "C" {

bool mbvq_qt_drawable(void) {
  mbvq::install_backend();
  /* the core fills the map only where a backend draws: with OpenGL, or with
     QRhi from what the core records (mbv_draw_batch.h) */
  return mbvq::current_backend == mbvq::Backend::LegacyGL || mbvq::current_backend == mbvq::Backend::Rhi;
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
  if (mbvq::widgets[instance] == nullptr) {
    mbvq::widgets[instance] = new mbvq::MapWidget(instance);
    if (mbvq::map_placement)
      mbvq::map_placement(mbvq::widgets[instance]);
    else
      mbvq::windows[instance] = new mbvq::MapWindow(mbvq::widgets[instance]);
  }
  mbvq::widgets[instance]->refresh_modes();
  mbvq::outermost(instance)->show();
}

void mbvq_qt_close_window(size_t instance) {
  if (instance >= MBVQ_MAX_WINDOWS || mbvq::widgets[instance] == nullptr)
    return;
  /* Called from mbview_destroy(), possibly inside an event of this window: the
     window is hidden now and deleted at a safe point (see closed_windows). Its
     drawing view goes now (an OpenGL map view with its context; a QRhi map
     view lets go of its GPU buffers as it is deleted). */
  mbvq::MapWidget *widget = mbvq::widgets[instance];
  QWidget *top = mbvq::outermost(instance);
  mbvq::widgets[instance] = nullptr;
  mbvq::windows[instance] = nullptr;
  top->hide();
  widget->delete_map_view();
  mbvq::closed_windows.push_back(top);
  emit widget->closed(instance);
}

void mbvq_qt_message(size_t instance, const char *message, bool on) {
  if (mbvq::MapWidget *window = widget_of(instance))
    window->show_message(QString::fromLocal8Bit(message), on);
}

void mbvq_qt_status(size_t instance, const char *message) {
  if (mbvq::MapWidget *window = widget_of(instance))
    window->set_status(QString::fromLocal8Bit(message));
}

void mbvq_qt_label(size_t instance, int label, const char *bxstring) {
  if (mbvq::MapWidget *window = widget_of(instance))
    window->set_label(label, mbvq::label_text(bxstring));
}

void mbvq_qt_modes_changed(size_t instance) {
  if (mbvq::MapWidget *window = widget_of(instance))
    window->refresh_modes();
}

void mbvq_qt_sensitivity_changed(size_t instance) {
  if (mbvq::MapWidget *window = widget_of(instance))
    window->refresh_modes();
}

void mbvq_qt_list_changed(int list) { mbvq::list_changed(list); }

void mbvq_qt_lists_closed(void) { mbvq::close_lists(); }

void mbvq_qt_add_action(size_t instance, int index, const char *label) {
  if (mbvq::MapWidget *window = widget_of(instance))
    window->add_action(index, QString::fromLocal8Bit(label));
}

void mbvq_qt_add_action_separator(size_t instance) {
  if (mbvq::MapWidget *window = widget_of(instance))
    window->add_action_separator();
}

void mbvq_qt_action_enabled(size_t instance, int index, bool enabled) {
  if (mbvq::MapWidget *window = widget_of(instance))
    window->set_action_enabled(index, enabled);
}

void mbvq_qt_bell(void) { QApplication::beep(); }

void mbvq_qt_cursor(size_t instance, int cursor) {
  if (mbvq::MapWidget *window = widget_of(instance))
    window->set_map_cursor(cursor);
}

void mbvq_qt_values_changed(size_t instance, int values) {
  if (mbvq::MapWidget *window = widget_of(instance))
    window->dialogs()->values_changed(values);
}

void mbvq_qt_schedule_work(size_t instance, int delay_ms) {
  QTimer::singleShot(delay_ms, qApp, [instance] { mbvq_c_work(instance); });
}

}  // extern "C"
