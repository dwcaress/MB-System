/*--------------------------------------------------------------------
 *    The MB-system:  mbv_render_glx.c  11/7/2026
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
 * GLX renderer of the mbview library, used by the Motif programs
 * (mbgrdviz, mbeditviz, mbnavadjust). The glX calls here were moved
 * unchanged from mbview_plot.c, mbview_profile.c,
 * mb3dsoundings_callbacks.c and mbview_callbacks.c. The X11 geometry
 * debugging function mbview_debug_glgeometry() moved here too.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "mb_status.h"
#include "mb_define.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#	ifndef WIN32
#		define WIN32
#	endif
#	include <WinSock2.h>
#include <windows.h>
#endif

#include <X11/StringDefs.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/DialogS.h>
#include <Xm/RepType.h>
#include <Xm/MwmUtil.h>
#include <Xm/BulletinB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include "Mb3dsdg.h"
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"

#include <GL/gl.h>
#include <GL/glu.h>
#ifndef WIN32
#include <GL/glx.h>
#endif
#include "mb_glwdrawa.h"

#include "mbview.h"
#include "mbviewprivate.h"
#include "mbv_render.h"

/* The soundings view's GLX calls are in mb3dsoundings_callbacks.c, which owns
   its state (mb3dsoundingsprivate.h may only be included there). */
void mb3dsoundings_glx_context_create(void);
void mb3dsoundings_glx_context_destroy(void);
void mb3dsoundings_glx_make_current(void);
void mb3dsoundings_glx_present(void);

/*------------------------------------------------------------------------------*/
/* Diagnostic: report - unconditionally, to stderr - what MB-System believes
    the OpenGL drawing area's size is (data->width/height) versus what X11
    and GLX/Mesa actually report for the glwmda window and its parent
    drawingArea widget: their sizes, their positions relative to their
    parent, their absolute on-screen positions, the GLX drawable size Mesa
    itself reports, and the OpenGL viewport currently in effect. Used to
    localize a rendered-image/pick offset (such as the one seen with some
    Mesa versions on macOS) to a specific layer: Xt/Motif widget geometry,
    the X11 window tree, the GLX drawable, or the GL viewport state. */
void mbview_debug_glgeometry(size_t instance, const char *tag) {
	if (mbv_verbose <= 0)
		return;

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	fprintf(stderr, "MBVIEW GEOMETRY [%s] instance:%zu data->width:%d data->height:%d\n", tag, instance, data->width,
	        data->height);

	if (!view->glx_init || !XtIsRealized(view->glwmda)) {
		fprintf(stderr, "MBVIEW GEOMETRY [%s] glwmda not realized/initialized yet\n", tag);
		return;
	}

	Window root;
	int x, y;
	unsigned int uwidth, uheight, border, depth;
	Window child;

	if (XGetGeometry(view->dpy, XtWindow(view->glwmda), &root, &x, &y, &uwidth, &uheight, &border, &depth)) {
		int abs_x = 0, abs_y = 0;
		XTranslateCoordinates(view->dpy, XtWindow(view->glwmda), root, 0, 0, &abs_x, &abs_y, &child);
		fprintf(stderr,
		        "MBVIEW GEOMETRY [%s] glwmda X window: local-to-parent x:%d y:%d  w:%u h:%u border:%u depth:%u  "
		        "screen-absolute x:%d y:%d\n",
		        tag, x, y, uwidth, uheight, border, depth, abs_x, abs_y);
	}
	else {
		fprintf(stderr, "MBVIEW GEOMETRY [%s] XGetGeometry failed for glwmda window\n", tag);
	}

	if (XGetGeometry(view->dpy, XtWindow(view->mb3dview.mbview_drawingArea_mbview), &root, &x, &y, &uwidth, &uheight, &border,
	                  &depth)) {
		int abs_x = 0, abs_y = 0;
		XTranslateCoordinates(view->dpy, XtWindow(view->mb3dview.mbview_drawingArea_mbview), root, 0, 0, &abs_x, &abs_y,
		                       &child);
		fprintf(stderr,
		        "MBVIEW GEOMETRY [%s] drawingArea X window: local-to-parent x:%d y:%d  w:%u h:%u border:%u depth:%u  "
		        "screen-absolute x:%d y:%d\n",
		        tag, x, y, uwidth, uheight, border, depth, abs_x, abs_y);
	}
	else {
		fprintf(stderr, "MBVIEW GEOMETRY [%s] XGetGeometry failed for drawingArea window\n", tag);
	}

#if defined(GLX_VERSION_1_3)
	{
		unsigned int glxwidth = 0, glxheight = 0;
		glXQueryDrawable(view->dpy, XtWindow(view->glwmda), GLX_WIDTH, &glxwidth);
		glXQueryDrawable(view->dpy, XtWindow(view->glwmda), GLX_HEIGHT, &glxheight);
		fprintf(stderr, "MBVIEW GEOMETRY [%s] GLX drawable (Mesa's own idea of the render-target size): w:%u h:%u\n", tag,
		        glxwidth, glxheight);
	}
#endif

	{
		GLint viewport[4] = {-1, -1, -1, -1};
		glGetIntegerv(GL_VIEWPORT, viewport);
		fprintf(stderr, "MBVIEW GEOMETRY [%s] current GL_VIEWPORT: x:%d y:%d w:%d h:%d\n", tag, viewport[0], viewport[1],
		        viewport[2], viewport[3]);
	}
}

/*------------------------------------------------------------------------------*/
static void mbv_glx_context_create(int which, size_t instance) {
	Cardinal ac;
	Arg args[256];

	if (which == MBV_RENDER_MAP) {
		struct mbview_world_struct *view = &(mbviews[instance]);
		ac = 0;
		XtSetArg(args[ac], mbGLwNvisualInfo, &(view->vi));
		ac++;
		XtGetValues(view->glwmda, args, ac);
#ifdef MBV_DEBUG_GLX
		fprintf(stderr, "%s:%d:%s instance:%zu glXCreateContext(%p,%p)\n", __FILE__, __LINE__, (char *) __FUNCTION__, instance,
		        view->dpy, view->vi);
#endif
		view->glx_context = glXCreateContext(view->dpy, view->vi, NULL, GL_TRUE);
	}
	else if (which == MBV_RENDER_PROFILE) {
		struct mbview_world_struct *view = &(mbviews[instance]);
		ac = 0;
		XtSetArg(args[ac], mbGLwNvisualInfo, &(view->prvi));
		ac++;
		XtGetValues(view->prglwmda, args, ac);
#ifdef MBV_DEBUG_GLX
		fprintf(stderr, "%s:%d:%s instance:%zu glXCreateContext(%p,%p)\n", __FILE__, __LINE__, __func__, instance, view->dpy,
		        view->prvi);
#endif
		view->prglx_context = glXCreateContext(view->dpy, view->prvi, NULL, GL_TRUE);
	}
	else if (which == MBV_RENDER_SOUNDINGS) {
		mb3dsoundings_glx_context_create();
	}
}
/*------------------------------------------------------------------------------*/
static void mbv_glx_context_destroy(int which, size_t instance) {
	if (which == MBV_RENDER_MAP) {
		struct mbview_world_struct *view = &(mbviews[instance]);
		glXDestroyContext(view->dpy, view->glx_context);
	}
	else if (which == MBV_RENDER_PROFILE) {
		struct mbview_world_struct *view = &(mbviews[instance]);
		glXDestroyContext(view->dpy, view->prglx_context);
	}
	else if (which == MBV_RENDER_SOUNDINGS) {
		mb3dsoundings_glx_context_destroy();
	}
}
/*------------------------------------------------------------------------------*/
static void mbv_glx_make_current(int which, size_t instance) {
	if (which == MBV_RENDER_MAP) {
		struct mbview_world_struct *view = &(mbviews[instance]);
		glXMakeCurrent(view->dpy, XtWindow(view->glwmda), view->glx_context);
	}
	else if (which == MBV_RENDER_PROFILE) {
		struct mbview_world_struct *view = &(mbviews[instance]);
		glXMakeCurrent(view->dpy, XtWindow(view->prglwmda), view->prglx_context);
	}
	else if (which == MBV_RENDER_SOUNDINGS) {
		mb3dsoundings_glx_make_current();
	}
}
/*------------------------------------------------------------------------------*/
static void mbv_glx_present(int which, size_t instance) {
	if (which == MBV_RENDER_MAP) {
		struct mbview_world_struct *view = &(mbviews[instance]);
		glXSwapBuffers(view->dpy, XtWindow(view->glwmda));
	}
	else if (which == MBV_RENDER_PROFILE) {
		struct mbview_world_struct *view = &(mbviews[instance]);
		glXSwapBuffers(XtDisplay(view->prglwmda), XtWindow(view->prglwmda));
	}
	else if (which == MBV_RENDER_SOUNDINGS) {
		mb3dsoundings_glx_present();
	}
}
/*------------------------------------------------------------------------------*/
static void mbv_glx_readback_begin(int which, size_t instance) {
	(void)which;
	(void)instance;
	glReadBuffer(GL_BACK);
}
/*------------------------------------------------------------------------------*/
static void mbv_glx_readback_end(int which, size_t instance) {
	(void)which;
	(void)instance;
	glReadBuffer(GL_FRONT);
}
/*------------------------------------------------------------------------------*/
void mbv_render_use_glx(void) {
	const struct mbv_render_ops ops = {mbv_glx_context_create, mbv_glx_context_destroy, mbv_glx_make_current, mbv_glx_present,
	                                   mbv_glx_readback_begin, mbv_glx_readback_end, NULL};
	mbv_render_set_ops(&ops);
}
/*------------------------------------------------------------------------------*/
