/*--------------------------------------------------------------------
 *    The MB-system:  mbv_camera.h  11/12/2026
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
 * Camera matrices of the mbview views, computed on the CPU in double
 * precision as the fixed-function OpenGL calls compute them (glOrtho,
 * gluPerspective, glTranslated, glRotated). The matrices are column-major,
 * as in OpenGL, and every function multiplies its matrix on the right, as
 * those calls do. mbview_camera() (map view) and mb3dsoundings_camera()
 * (3D soundings view) build their cameras from these functions, so that
 * OpenGL, picks on the CPU and renderers without OpenGL share one camera.
 * mbv_camera_project() maps a point to window coordinates as gluProject
 * does, for the screen positions of the soundings when nothing draws them
 * with OpenGL. This file uses no toolkit and no OpenGL.
 */

#ifndef MBVIEW_MBV_CAMERA_H
#define MBVIEW_MBV_CAMERA_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* m = identity */
void mbv_camera_identity(double m[16]);

/* m = m * orthographic projection, as glOrtho */
void mbv_camera_ortho(double m[16], double left, double right, double bottom, double top, double znear, double zfar);

/* m = m * perspective projection, as gluPerspective (fovy in degrees) */
void mbv_camera_perspective(double m[16], double fovy, double aspect, double znear, double zfar);

/* m = m * translation, as glTranslated */
void mbv_camera_translate(double m[16], double x, double y, double z);

/* m = m * rotation by angle degrees about the axis (x, y, z), as glRotated */
void mbv_camera_rotate(double m[16], double angle, double x, double y, double z);

/* the window coordinates of the point (x, y, z), as gluProject computes them:
   the modelview, then the projection, the division by w, and the mapping to
   the viewport (x, y, width, height), with window y up as in OpenGL. Returns
   false, leaving the outputs alone, when w is zero. */
bool mbv_camera_project(const double modelview[16], const double projection[16], const int viewport[4], double x,
                        double y, double z, double *winx, double *winy, double *winz);

#ifdef __cplusplus
}
#endif

#endif /* MBVIEW_MBV_CAMERA_H */
