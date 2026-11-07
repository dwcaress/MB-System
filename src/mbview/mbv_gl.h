/*--------------------------------------------------------------------
 *    The MB-system:  mbv_gl.h  11/7/2026
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
 * OpenGL headers for the mbview drawing code.
 *
 * The Motif build gets exactly the headers the mbview files included before
 * (GL, GLU and, except on Windows, GLX). A build without Motif
 * (MBVIEW_NO_MOTIF) gets the platform's own OpenGL headers and no GLX:
 * on macOS that is the OpenGL framework that Qt uses.
 */

#ifndef MBVIEW_MBV_GL_H
#define MBVIEW_MBV_GL_H

#ifndef MBVIEW_NO_MOTIF
#include <GL/gl.h>
#include <GL/glu.h>
#ifndef WIN32
#include <GL/glx.h>
#endif
#else
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif

#endif /* MBVIEW_MBV_GL_H */
