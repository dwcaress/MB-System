/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_backend.h  11/7/2026
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
 * The backends of mbview-qt. A backend installs the mbview renderer
 * operations (mbv_render.h) and creates the map views. The legacy OpenGL
 * backend (mbvq_backend_gl.cc) holds all OpenGL code of the Qt half; the
 * null backend (mbvq_backend_null.cc) draws nothing. A VTK backend can be
 * added beside them later.
 */

#ifndef MBVIEW_QT_MBVQ_BACKEND_H
#define MBVIEW_QT_MBVQ_BACKEND_H

#include <cstddef>

class QWidget;

namespace mbvq {

namespace gl_backend {
/* set the default surface format; before QApplication exists */
void configure();
/* install the renderer operations */
void install();
QWidget *create_map_view(size_t instance, QWidget *parent);
}  // namespace gl_backend

namespace null_backend {
void install();
QWidget *create_map_view(size_t instance, QWidget *parent);
}  // namespace null_backend

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_BACKEND_H */
