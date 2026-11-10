/*--------------------------------------------------------------------
 *    The MB-system:  mbv_render.c  11/7/2026
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
 * Renderer interface of the mbview library (see mbv_render.h). This file
 * uses no toolkit and no OpenGL.
 */

#include <stdbool.h>
#include <stddef.h>

#include "mbv_render.h"

/* the installed renderer; all entries NULL until one is installed */
static struct mbv_render_ops mbv_render = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

/*------------------------------------------------------------------------------*/
void mbv_render_set_ops(const struct mbv_render_ops *ops) {
	if (ops != NULL)
		mbv_render = *ops;
}
/*------------------------------------------------------------------------------*/
void mbv_render_context_create(int view, size_t instance) {
	if (mbv_render.context_create != NULL)
		mbv_render.context_create(view, instance);
}
/*------------------------------------------------------------------------------*/
void mbv_render_context_destroy(int view, size_t instance) {
	if (mbv_render.context_destroy != NULL)
		mbv_render.context_destroy(view, instance);
}
/*------------------------------------------------------------------------------*/
void mbv_render_make_current(int view, size_t instance) {
	if (mbv_render.make_current != NULL)
		mbv_render.make_current(view, instance);
}
/*------------------------------------------------------------------------------*/
void mbv_render_present(int view, size_t instance) {
	if (mbv_render.present != NULL)
		mbv_render.present(view, instance);
}
/*------------------------------------------------------------------------------*/
void mbv_render_readback_begin(int view, size_t instance) {
	if (mbv_render.readback_begin != NULL)
		mbv_render.readback_begin(view, instance);
}
/*------------------------------------------------------------------------------*/
void mbv_render_readback_end(int view, size_t instance) {
	if (mbv_render.readback_end != NULL)
		mbv_render.readback_end(view, instance);
}
/*------------------------------------------------------------------------------*/
bool mbv_render_defer_draw(int view, size_t instance, int rez) {
	if (mbv_render.defer_draw != NULL)
		return mbv_render.defer_draw(view, instance, rez);
	return false;
}
/*------------------------------------------------------------------------------*/
