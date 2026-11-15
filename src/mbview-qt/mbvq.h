/*--------------------------------------------------------------------
 *    The MB-system:  mbvq.h  11/7/2026
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
 * mbview-qt: the mbview library with a Qt6 user interface.
 *
 * Programs call the mbview API of mbview.h as the Motif programs do
 * (mbview_startup, mbview_init, mbview_setwindowparms, mbview_open, ...).
 * The drawing core is the shared mbview code, built here without Motif.
 * This header adds what only Qt programs need.
 */

#ifndef MBVIEW_QT_MBVQ_H
#define MBVIEW_QT_MBVQ_H

#include <cstddef>
#include <functional>

class QString;
class QWidget;

namespace mbvq {

class MapWidget;
class SoundingsWidget;

/* The renderer of the program. One backend per process: call
   set_backend_from_environment() or set_backend() in main() before creating
   the QApplication, because the OpenGL backend sets the default surface
   format there. */
enum class Backend {
  LegacyGL, /* the mbview OpenGL drawing code in a 2.1 compatibility context */
  Null,     /* draws nothing, for tests without a display */
  Rhi       /* QRhi: the drawing the core records, with Metal, Direct3D, Vulkan
               or OpenGL; built with Qt 6.8 or later and Qt Shader Tools */
};
/* the graphics API of the QRhi backend, as QRhiWidget::Api; Default is Metal
   on Apple, Direct3D 11 on Windows and OpenGL elsewhere */
enum class RhiApi { Default, OpenGL, Metal, Vulkan, Direct3D11, Direct3D12 };
/* a backend this build lacks gives a message and the other one */
void set_backend(Backend backend, RhiApi api = RhiApi::Default);
Backend backend();
/* whether this build has the backend (the OpenGL and null backends always) */
bool backend_built(Backend backend);
/* the backend that the environment variable MB_RENDERER names (auto, metal,
   d3d11, d3d12, vulkan, opengl, legacy-gl or null), else the OpenGL backend;
   in main() before creating the QApplication */
void set_backend_from_environment();

/* Starts mbview and the 3D soundings view, once however many tools in the
   program call it. mbview_startup() itself resets all maps and shared data. */
void start(int verbose);

/* For a host: called with each new map or 3D soundings widget, to put it
   somewhere. Without a placement each one gets a window of its own. The library
   shows and removes the widget; a host never deletes it. */
void set_map_placement(std::function<void(MapWidget *)> place);
void set_soundings_placement(std::function<void(SoundingsWidget *)> place);

/* the window of an mbview instance, or its widget if a host placed it;
   nullptr if it has none */
QWidget *window(size_t instance);

/* the widget of an mbview instance, nullptr if it has none */
MapWidget *map_widget(size_t instance);

/* the map view of an mbview instance, nullptr if it has none */
QWidget *map_view(size_t instance);

/* the text of a label in Motif "BX" form (:::t"line":t"line"...) */
QString label_text(const char *bxstring);

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_H */
