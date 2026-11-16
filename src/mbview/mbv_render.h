/*--------------------------------------------------------------------
 *    The MB-system:  mbv_render.h  11/7/2026
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
 * Renderer interface of the mbview library.
 *
 * The mbview drawing code (mbview_plot, mbview_findpointrez,
 * mbview_viewbounds, mbview_plotprofile, mb3dsoundings_plot) does not
 * manage OpenGL contexts or buffers itself. It asks the renderer through
 * the functions below. The Motif programs install the GLX renderer
 * (mbv_render_glx.c) in mbview_startup(), which does exactly what the
 * drawing code did before. Other toolkits can install their own.
 *
 * Every operation is optional: a NULL entry does nothing, and a NULL
 * defer_draw means "draw now". A renderer that does not draw with OpenGL
 * gives pick_terrain and view_bounds, which find on the CPU what the
 * colour-coded drawing of mbview_findpointrez and mbview_viewbounds finds;
 * NULL entries keep that drawing. While a batch is filled (mbv_draw.h)
 * defer_draw, make_current, present and the read-back are not asked for, and
 * without view_bounds the 3D view bounds are the whole grid, not drawn.
 */

#ifndef MBVIEW_MBV_RENDER_H
#define MBVIEW_MBV_RENDER_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* the views mbview draws */
#define MBV_RENDER_MAP 0       /* the 2D/3D map view of an mbview instance */
#define MBV_RENDER_PROFILE 1   /* the profile view of an mbview instance */
#define MBV_RENDER_SOUNDINGS 2 /* the 3D soundings view (instance unused) */

struct mbv_render_ops {
	/* create the drawing context of a view, once its window exists */
	void (*context_create)(int view, size_t instance);
	/* destroy the drawing context of a view */
	void (*context_destroy)(int view, size_t instance);
	/* make the drawing context of a view current */
	void (*make_current)(int view, size_t instance);
	/* show the frame just drawn */
	void (*present)(int view, size_t instance);
	/* select the buffer that colour-coded picking draws into for reading,
	   and switch back afterwards */
	void (*readback_begin)(int view, size_t instance);
	void (*readback_end)(int view, size_t instance);
	/* return true if the renderer has queued the drawing for later, so that
	   the caller must not draw now; rez is MBV_REZ_* for the map view */
	bool (*defer_draw)(int view, size_t instance, int rez);
	/* pick the terrain of the map view at a pixel without drawing, the pixel
	   counted as glReadPixels counts it (origin at the bottom left, physical
	   pixels); return true and the grid position and data value of the centroid
	   of the full-resolution triangle seen at the pixel, false if none is seen */
	bool (*pick_terrain)(size_t instance, int xpixel, int ypixel, double *xgrid, double *ygrid, double *zdata);
	/* set the grid bounds seen in the 3D map view without drawing */
	void (*view_bounds)(size_t instance, int viewbounds[4]);
};

/* install a renderer; the table is copied */
void mbv_render_set_ops(const struct mbv_render_ops *ops);

/* the GLX renderer used by the Motif programs (mbv_render_glx.c) */
void mbv_render_use_glx(void);

/* calls used by the drawing code */
void mbv_render_context_create(int view, size_t instance);
void mbv_render_context_destroy(int view, size_t instance);
void mbv_render_make_current(int view, size_t instance);
void mbv_render_present(int view, size_t instance);
void mbv_render_readback_begin(int view, size_t instance);
void mbv_render_readback_end(int view, size_t instance);
bool mbv_render_defer_draw(int view, size_t instance, int rez);

/* the picks of a renderer that finds them without drawing; each returns false
   if the renderer has none, and the caller then draws */
bool mbv_render_pick_terrain(size_t instance, int xpixel, int ypixel, bool *found, double *xgrid, double *ygrid,
                             double *zdata);
bool mbv_render_view_bounds(size_t instance, int viewbounds[4]);

#ifdef __cplusplus
}
#endif

#endif /* MBVIEW_MBV_RENDER_H */
