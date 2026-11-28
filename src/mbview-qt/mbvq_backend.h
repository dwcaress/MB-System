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
 * operations (mbv_render.h) and creates the views. The legacy OpenGL backend
 * (mbvq_backend_gl.cc) holds all OpenGL code of the Qt half and runs the
 * core's OpenGL drawing. The QRhi backend (mbvq_backend_rhi.cc and the
 * mbvq_rhi_* files; built with Qt 6.8 or later and Qt Shader Tools, where
 * MBVQ_HAVE_RHI is defined) draws what the core records in a batch
 * (mbv_draw_batch.h) with Metal, Direct3D, Vulkan or OpenGL, without any
 * OpenGL call of the core. The null backend (mbvq_backend_null.cc) draws
 * nothing. One backend per process, chosen in main() before the QApplication
 * exists (mbvq.h); the views come from the backend chosen.
 */

#ifndef MBVIEW_QT_MBVQ_BACKEND_H
#define MBVIEW_QT_MBVQ_BACKEND_H

#include <cstddef>
#include <string>

#include "mbvq.h"

class QWidget;

namespace mbvq {

/* the backend and API that a value of MB_RENDERER names: auto, metal, d3d11,
   d3d12, vulkan and opengl (QRhi with that API; auto with the platform's),
   legacy-gl (the OpenGL backend) and null (the null backend); false for
   anything else */
bool parse_renderer(const char *value, Backend *backend, RhiApi *api);
/* whether this platform can have a QRhi API: Metal on Apple, Direct3D on
   Windows, Vulkan and OpenGL where Qt has them (Default always) */
bool rhi_api_possible(RhiApi api);
/* the backend and API that a value of MB_RENDERER chooses (null or empty when
   it is not set), and the line to write to stderr (empty if none): an unknown
   value gives the OpenGL backend, QRhi where this build has none the OpenGL
   backend, an API this platform cannot have QRhi with the platform's API.
   What set_backend_from_environment() asks before set_backend(). */
void choose_renderer(const char *value, Backend *backend, RhiApi *api, std::string *message);
/* the API chosen for the QRhi backend */
RhiApi rhi_api();
/* the name of an API, for messages ("Metal", "Direct3D 11", ...) */
const char *rhi_api_name(RhiApi api);

/* the views of the backend chosen */
QWidget *create_map_view(size_t instance, QWidget *parent);
QWidget *create_profile_view(size_t instance, QWidget *parent);
QWidget *create_soundings_view(QWidget *parent);
/* the profile window sizes its view: a QRhi profile view takes again the
   largest size its QRhi can draw at its pixel ratio; nothing for the other
   backends */
void cap_profile_view(QWidget *view);
/* the program released the soundings in its dismiss notification: a hidden
   QRhi soundings view, which draws no frame that would drop them, lets go of
   its copies of them; nothing for the other backends */
void release_soundings_view(QWidget *view);

/* true while a QRhi view fills its batch (mbv_draw_batch.h): the widgets then
   send their status signals after the fill, so that a slot of the program
   cannot draw another view in the middle of it. The OpenGL backend never
   sets it. */
bool fill_active();
void set_fill_active(bool active);

namespace gl_backend {
/* set the default surface format; before QApplication exists */
void configure();
/* install the renderer operations */
void install();
QWidget *create_map_view(size_t instance, QWidget *parent);
QWidget *create_profile_view(size_t instance, QWidget *parent);
QWidget *create_soundings_view(QWidget *parent);
}  // namespace gl_backend

namespace null_backend {
void install();
QWidget *create_map_view(size_t instance, QWidget *parent);
QWidget *create_profile_view(size_t instance, QWidget *parent);
QWidget *create_soundings_view(QWidget *parent);
}  // namespace null_backend

#ifdef MBVQ_HAVE_RHI
namespace rhi_backend {
void install();
QWidget *create_map_view(size_t instance, QWidget *parent);
QWidget *create_profile_view(size_t instance, QWidget *parent);
QWidget *create_soundings_view(QWidget *parent);
void cap_profile_view(QWidget *view);
void release_soundings_view(QWidget *view);
/* the point-size control of the 3D soundings view: the view draws again */
void set_soundings_point_size(double logical_px);
}  // namespace rhi_backend
#endif

}  // namespace mbvq

#endif /* MBVIEW_QT_MBVQ_BACKEND_H */
