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

class QString;
class QWidget;

namespace mbvq {

/* The renderer of the program. One backend per process: call
   set_backend() in main() before creating the QApplication, because the
   OpenGL backend sets the default surface format there. */
enum class Backend {
  LegacyGL, /* the mbview OpenGL drawing code in a 2.1 compatibility context */
  Null      /* draws nothing, for tests without a display */
};
void set_backend(Backend backend);
Backend backend();

/* the window of an mbview instance, nullptr if it has none */
QWidget *window(size_t instance);

/* the map view of an mbview instance, nullptr if it has none */
QWidget *map_view(size_t instance);

/* the text of a label in Motif "BX" form (:::t"line":t"line"...) */
QString label_text(const char *bxstring);

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_H */
