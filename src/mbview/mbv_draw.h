/*--------------------------------------------------------------------
 *    The MB-system:  mbv_draw.h  11/13/2026
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
 * The drawing interface of the overlays, the profile view and the 3D soundings
 * decorations: one call for each OpenGL call those functions make. With no
 * batch as the target (the default, and always in the Motif programs) every
 * call makes exactly the OpenGL call it stands for, so the pictures are those
 * of OpenGL by construction. With a batch as the target nothing is drawn: the
 * call is recorded (mbv_draw_batch.h). The calls are inline, so OpenGL mode
 * costs one test of the target per call. Only the drawing code of src/mbview
 * includes this file, after the OpenGL headers it already uses.
 */

#ifndef MBVIEW_MBV_DRAW_H
#define MBVIEW_MBV_DRAW_H

#include "mbv_gl.h"
#include "mbv_draw_batch.h"

/* the target, NULL for OpenGL (mbv_draw.c) */
extern struct mbv_draw_batch *mbv_draw_target;

/* the primitives keep OpenGL's numbers */
#if GL_POINTS != MBV_DRAW_POINTS || GL_LINES != MBV_DRAW_LINES || GL_LINE_LOOP != MBV_DRAW_LINE_LOOP || \
    GL_LINE_STRIP != MBV_DRAW_LINE_STRIP || GL_TRIANGLES != MBV_DRAW_TRIANGLES || GL_QUADS != MBV_DRAW_QUADS
#error "mbv_draw_batch.h: the primitive numbers differ from OpenGL's"
#endif

/* true when the calls draw with OpenGL: code that draws without this
   interface (clears, cameras, the terrain, the soundings points) runs only
   then */
static inline bool mbv_draw_opengl(void) { return mbv_draw_target == NULL; }

static inline void mbv_draw_begin(int primitive) {
	if (mbv_draw_target == NULL)
		glBegin((GLenum)primitive);
	else
		mbv_draw_batch_begin(mbv_draw_target, primitive);
}

static inline void mbv_draw_end(void) {
	if (mbv_draw_target == NULL)
		glEnd();
	else
		mbv_draw_batch_end(mbv_draw_target);
}

static inline void mbv_draw_color3f(float r, float g, float b) {
	if (mbv_draw_target == NULL)
		glColor3f(r, g, b);
	else
		mbv_draw_batch_color3f(mbv_draw_target, r, g, b);
}

static inline void mbv_draw_vertex3f(float x, float y, float z) {
	if (mbv_draw_target == NULL)
		glVertex3f(x, y, z);
	else
		mbv_draw_batch_vertex3f(mbv_draw_target, x, y, z);
}

static inline void mbv_draw_line_width(float width) {
	if (mbv_draw_target == NULL)
		glLineWidth(width);
	else
		mbv_draw_batch_line_width(mbv_draw_target, width);
}

static inline void mbv_draw_point_size(float size) {
	if (mbv_draw_target == NULL)
		glPointSize(size);
	else
		mbv_draw_batch_point_size(mbv_draw_target, size);
}

static inline void mbv_draw_line_stipple(int factor, unsigned short pattern) {
	if (mbv_draw_target == NULL)
		glLineStipple(factor, pattern);
	else
		mbv_draw_batch_line_stipple(mbv_draw_target, factor, pattern);
}

/* glEnable and glDisable of GL_DEPTH_TEST and GL_LINE_STIPPLE */
static inline void mbv_draw_enable(int state) {
	if (mbv_draw_target == NULL)
		glEnable(state == MBV_DRAW_DEPTH_TEST ? GL_DEPTH_TEST : GL_LINE_STIPPLE);
	else
		mbv_draw_batch_enable(mbv_draw_target, state, true);
}

static inline void mbv_draw_disable(int state) {
	if (mbv_draw_target == NULL)
		glDisable(state == MBV_DRAW_DEPTH_TEST ? GL_DEPTH_TEST : GL_LINE_STIPPLE);
	else
		mbv_draw_batch_enable(mbv_draw_target, state, false);
}

/* glMatrixMode(GL_MODELVIEW) and glLoadIdentity(): what follows is drawn
   without the view's camera rotations and translation */
static inline void mbv_draw_identity_modelview(void) {
	if (mbv_draw_target == NULL) {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	} else {
		mbv_draw_batch_identity_modelview(mbv_draw_target);
	}
}

/* where something not in the batch is drawn (MBV_DRAW_SLOT_*); nothing with
   OpenGL */
static inline void mbv_draw_slot(int slot) {
	if (mbv_draw_target != NULL)
		mbv_draw_batch_slot(mbv_draw_target, slot);
}

#endif /* MBVIEW_MBV_DRAW_H */
