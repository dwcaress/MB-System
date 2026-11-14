/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_profile.c	3/8/2006
 *
 *    Copyright (c) 2006-2025 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 *
 * Author:	D. W. Caress
 * Date:	March 8, 2006
 *
 *
 */
/*------------------------------------------------------------------------------*/

/* Standard includes for builtins. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#	ifndef WIN32
#		define WIN32
#	endif
#	include <WinSock2.h>
#include <windows.h>
#endif

/* Motif required Headers */
#ifndef MBVIEW_NO_MOTIF
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
#else
#include "mbv_xtypes.h"
#endif
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"

/* OpenGL include files */
#include "mbv_gl.h"
#ifndef MBVIEW_NO_MOTIF
#include "mb_glwdrawa.h"
#endif

/* MBIO include files */
#include "mb_status.h"
#include "mb_define.h"

/* mbview include */
#include "mbview.h"
#include "mbviewprivate.h"
#include "mbv_render.h"
#include "mbv_camera.h"
#include "mbv_draw.h"

//#define MBV_DEBUG_GLX 1
//#define MBV_GET_GLX_ERRORS 1

/*------------------------------------------------------------------------------*/

/* library variables */
extern int mbv_verbose;
extern int mbv_ninstance;
extern Widget parent_widget;
extern XtAppContext app_context;
extern struct mbview_world_struct mbviews[MBV_MAX_WINDOWS];
extern char *mbsystem_library_name;

/* local variables */
static char value_text[2*MB_PATH_MAXLINE];


/*------------------------------------------------------------------------------*/
int mbview_getprofilecount(int verbose, size_t instance, int *npoints, int *error)

{
	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get number of profiles */
	*npoints = data->profile.npoints;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       npoints:                   %d\n", *npoints);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_allocprofilepoints(int verbose, int npoints, struct mbview_profilepoint_struct **points, int *error)

{
	/* local variables */
	int status = MB_SUCCESS;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       npoints:                   %d\n", npoints);
		fprintf(stderr, "dbg2       points:                    %p\n", *points);
	}

	/* allocate the arrays using mb_reallocd */
	status =
	    mb_reallocd(verbose, __FILE__, __LINE__, npoints * sizeof(struct mbview_profilepoint_struct), (void **)points, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       points:                    %p\n", *points);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_freeprofilepoints(int verbose, double **points, int *error)

{
	/* local variables */
	int status = MB_SUCCESS;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       points:                    %p\n", *points);
	}

	/* free the arrays using mb_freed */
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)points, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       points:                    %p\n", *points);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_allocprofilearrays(int verbose, int npoints, double **distance, double **zdata, int **boundary, double **xlon,
                              double **ylat, double **distovertopo, double **bearing, double **slope, int *error)

{
	/* local variables */
	int status = MB_SUCCESS;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       npoints:                   %d\n", npoints);
		fprintf(stderr, "dbg2       distance:                  %p\n", *distance);
		fprintf(stderr, "dbg2       zdata:                     %p\n", *zdata);
		fprintf(stderr, "dbg2       boundary:                  %p\n", *boundary);
		fprintf(stderr, "dbg2       xlon:                      %p\n", *xlon);
		fprintf(stderr, "dbg2       ylat:                      %p\n", *ylat);
		fprintf(stderr, "dbg2       distovertopo:              %p\n", *distovertopo);
		fprintf(stderr, "dbg2       bearing:                   %p\n", *bearing);
		fprintf(stderr, "dbg2       slope:                     %p\n", *slope);
	}

	/* allocate the arrays using mb_reallocd */
	status = mb_reallocd(verbose, __FILE__, __LINE__, npoints * sizeof(double), (void **)distance, error);
	status = mb_reallocd(verbose, __FILE__, __LINE__, npoints * sizeof(double), (void **)zdata, error);
	status = mb_reallocd(verbose, __FILE__, __LINE__, npoints * sizeof(double), (void **)boundary, error);
	status = mb_reallocd(verbose, __FILE__, __LINE__, npoints * sizeof(double), (void **)xlon, error);
	status = mb_reallocd(verbose, __FILE__, __LINE__, npoints * sizeof(double), (void **)ylat, error);
	status = mb_reallocd(verbose, __FILE__, __LINE__, npoints * sizeof(double), (void **)distovertopo, error);
	status = mb_reallocd(verbose, __FILE__, __LINE__, npoints * sizeof(double), (void **)bearing, error);
	status = mb_reallocd(verbose, __FILE__, __LINE__, npoints * sizeof(double), (void **)slope, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       distance:                  %p\n", *distance);
		fprintf(stderr, "dbg2       zdata:                     %p\n", *zdata);
		fprintf(stderr, "dbg2       boundary:                  %p\n", *boundary);
		fprintf(stderr, "dbg2       xlon:                      %p\n", *xlon);
		fprintf(stderr, "dbg2       ylat:                      %p\n", *ylat);
		fprintf(stderr, "dbg2       distovertopo:              %p\n", *distovertopo);
		fprintf(stderr, "dbg2       bearing:                   %p\n", *bearing);
		fprintf(stderr, "dbg2       slope:                     %p\n", *slope);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_freeprofilearrays(int verbose, double **distance, double **zdata, int **boundary, double **xlon, double **ylat,
                             double **distovertopo, double **bearing, double **slope, int *error)

{
	/* local variables */
	int status = MB_SUCCESS;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       distance:                  %p\n", *distance);
		fprintf(stderr, "dbg2       zdata:                     %p\n", *zdata);
		fprintf(stderr, "dbg2       boundary:                  %p\n", *boundary);
		fprintf(stderr, "dbg2       xlon:                      %p\n", *xlon);
		fprintf(stderr, "dbg2       ylat:                      %p\n", *ylat);
		fprintf(stderr, "dbg2       distovertopo:              %p\n", *distovertopo);
		fprintf(stderr, "dbg2       bearing:                   %p\n", *bearing);
		fprintf(stderr, "dbg2       slope:                     %p\n", *slope);
	}

	/* free the arrays using mb_freed */
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)distance, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)zdata, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)boundary, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)xlon, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)ylat, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)distovertopo, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)bearing, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)slope, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       distance:                  %p\n", *distance);
		fprintf(stderr, "dbg2       zdata:                     %p\n", *zdata);
		fprintf(stderr, "dbg2       boundary:                  %p\n", *boundary);
		fprintf(stderr, "dbg2       xlon:                      %p\n", *xlon);
		fprintf(stderr, "dbg2       ylat:                      %p\n", *ylat);
		fprintf(stderr, "dbg2       distovertopo:              %p\n", *distovertopo);
		fprintf(stderr, "dbg2       bearing:                   %p\n", *bearing);
		fprintf(stderr, "dbg2       slope:                     %p\n", *slope);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_getprofile(int verbose, size_t instance, mb_path source_name, double *length, double *zmin, double *zmax, int *npoints,
                      double *distance, double *zdata, int *boundary, double *xlon, double *ylat, double *distovertopo,
                      double *bearing, double *slope, int *error)

{
	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int i;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* check that the array pointers are not NULL */
	if (distance == NULL || zdata == NULL || boundary == NULL || xlon == NULL || ylat == NULL || distovertopo == NULL ||
	    slope == NULL || bearing == NULL) {
		status = MB_FAILURE;
		*error = MB_ERROR_DATA_NOT_INSERTED;
	}

	/* otherwise go get the profile data */
	else {
		/* loop over the profiles */
		strcpy(source_name, data->profile.source_name);
		*length = data->profile.length;
		*zmin = data->profile.zmin;
		*zmax = data->profile.zmax;
		*npoints = data->profile.npoints;
		for (i = 0; i < data->profile.npoints; i++) {
			distance[i] = data->profile.points[i].distance;
			zdata[i] = data->profile.points[i].zdata;
			boundary[i] = data->profile.points[i].boundary;
			xlon[i] = data->profile.points[i].xlon;
			ylat[i] = data->profile.points[i].ylat;
			distovertopo[i] = data->profile.points[i].distovertopo;
			bearing[i] = data->profile.points[i].bearing;
			slope[i] = data->profile.points[i].slope;
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       source_name:                %s\n", source_name);
		fprintf(stderr, "dbg2       length:                     %f\n", *length);
		fprintf(stderr, "dbg2       zmin:                       %f\n", *zmin);
		fprintf(stderr, "dbg2       zmax:                       %f\n", *zmax);
		fprintf(stderr, "dbg2       npoints:                    %d\n", *npoints);
		for (i = 0; i < *npoints; i++) {
			fprintf(stderr,
			        "dbg2       %d distance:%f zdata:%f boundary:%d xlon:%f ylat:%f distovertopo:%f bearing:%f slope:%f\n", i,
			        distance[i], zdata[i], boundary[i], xlon[i], ylat[i], distovertopo[i], bearing[i], slope[i]);
		}
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}


/*------------------------------------------------------------------------------*/
int mbview_destroy_prglx(size_t instance) {
	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* If profile view enabled update opengl plotting widget */
	if (data->profile_view_mode == MBV_VIEW_ON) {
		/* delete old glx_context if it exists */
		if (view->prglx_init) {
			mbv_render_context_destroy(MBV_RENDER_PROFILE, instance);
			view->prglx_init = false;
		}
	}

	/* print output debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                %d\n", status);
		fprintf(stderr, "dbg2       view->dpy:             %p\n", view->dpy);
		fprintf(stderr, "dbg2       view->prvi:            %p\n", view->prvi);
		fprintf(stderr, "dbg2       view->prglwmda:        %p\n", view->prglwmda);
		fprintf(stderr, "dbg2       view->prglx_context:   %p\n", view->prglx_context);
		fprintf(stderr, "dbg2       view->prglx_init:      %d\n", view->prglx_init);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_plotprofile(size_t instance) {
	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	float left, right, top, bottom;
	double zcenter, zmin, zmax;
	double scale;
	float yzmin, yzmax;
	float x, y;
	int clip;
	int i;

	/* print starting debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* If there is a profile plot it, unless the renderer draws later by itself;
	   filling a batch (mbv_draw.h) is the renderer drawing now, without OpenGL */
	const bool opengl = mbv_draw_opengl();
	if (data->profile_view_mode == MBV_VIEW_ON && (!opengl || !mbv_render_defer_draw(MBV_RENDER_PROFILE, instance, 0))) {

		/* get scaling */
		scale = MBV_OPENGL_WIDTH / data->profile.length;
		left = -0.1 * MBV_OPENGL_WIDTH;
		right = 1.1 * MBV_OPENGL_WIDTH;
		zcenter = 0.5 * (data->profile.zmax + data->profile.zmin);
		top = 0.5 * (right - left) * view->praspect_ratio;
		bottom = -top;
		zmin = zcenter - 0.5 * view->praspect_ratio * data->profile.length / data->profile_exaggeration;
		zmax = zcenter + 0.5 * view->praspect_ratio * data->profile.length / data->profile_exaggeration;
		yzmin = scale * data->profile_exaggeration * (zmin - zcenter);
		yzmax = scale * data->profile_exaggeration * (zmax - zcenter);
		clip = false;

		if (opengl) {
/* set projection to 2D */
#ifdef MBV_DEBUG_GLX
			fprintf(stderr, "%s:%d:%s instance:%zu glXMakeCurrent(%p,%p,%p)\n", __FILE__, __LINE__, __func__, instance,
			        view->dpy, XtWindow(view->prglwmda), view->prglx_context);
#endif
			mbv_render_make_current(MBV_RENDER_PROFILE, instance);

#ifdef MBV_GET_GLX_ERRORS
			mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(left, right, bottom, top, MBV_OPENGL_ZMIN2D, MBV_OPENGL_ZMAX2D);

			/* set up translations */
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glTranslated(0.0, 0.0, MBV_OPENGL_ZMIN2D);

			/* set background color */
			glClearColor(1.0, 1.0, 1.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		/* draw profile */
		mbv_draw_line_width(1.0);
		mbv_draw_begin(MBV_DRAW_QUADS);
		for (i = 0; i < data->profile.npoints - 1; i++) {
			if (data->profile.points[i].boundary == false || data->profile.points[i + 1].boundary == false) {
				if (data->profile.points[i].slope < data->profile_slopethreshold) {
					mbv_draw_color3f(colortable_object_red[MBV_COLOR_BLACK], colortable_object_green[MBV_COLOR_BLACK],
					          colortable_object_blue[MBV_COLOR_BLACK]);
				}
				else {
					mbv_draw_color3f(colortable_object_red[MBV_COLOR_RED], colortable_object_green[MBV_COLOR_RED],
					          colortable_object_blue[MBV_COLOR_RED]);
				}
				x = scale * data->profile.points[i].distance;
				y = scale * data->profile_exaggeration * (data->profile.points[i].zdata - zcenter);
				if (y < yzmin) {
					clip = true;
					y = yzmin;
				}
				if (y > yzmax) {
					clip = true;
					y = yzmax;
				}
				mbv_draw_vertex3f(x, yzmin, MBV_OPENGL_ZPROFILE1);
				mbv_draw_vertex3f(x, y, MBV_OPENGL_ZPROFILE1);
				x = scale * data->profile.points[i + 1].distance;
				y = scale * data->profile_exaggeration * (data->profile.points[i + 1].zdata - zcenter);
				if (y < yzmin) {
					clip = true;
					y = yzmin;
				}
				if (y > yzmax) {
					clip = true;
					y = yzmax;
				}
				mbv_draw_vertex3f(x, y, MBV_OPENGL_ZPROFILE1);
				mbv_draw_vertex3f(x, yzmin, MBV_OPENGL_ZPROFILE1);
			}
		}
		mbv_draw_end();

		/* draw boundaries */
		mbv_draw_color3f(colortable_object_red[MBV_COLOR_GREEN], colortable_object_green[MBV_COLOR_GREEN],
		          colortable_object_blue[MBV_COLOR_GREEN]);
		mbv_draw_line_width(2.0);
		mbv_draw_begin(MBV_DRAW_LINES);
		for (i = 0; i < data->profile.npoints; i++) {
			if (data->profile.points[i].boundary) {
				x = scale * data->profile.points[i].distance;
				mbv_draw_vertex3f(x, yzmin, MBV_OPENGL_ZPROFILE1);
				mbv_draw_vertex3f(x, yzmax, MBV_OPENGL_ZPROFILE1);
			}
		}
		mbv_draw_end();

		/* draw box */
		if (clip == false)
			mbv_draw_color3f(colortable_object_red[MBV_COLOR_BLACK], colortable_object_green[MBV_COLOR_BLACK],
			          colortable_object_blue[MBV_COLOR_BLACK]);
		else
			mbv_draw_color3f(colortable_object_red[MBV_COLOR_RED], colortable_object_green[MBV_COLOR_RED],
			          colortable_object_blue[MBV_COLOR_RED]);
		mbv_draw_line_width(2.0);
		mbv_draw_begin(MBV_DRAW_LINE_LOOP);
		mbv_draw_vertex3f((float)(0.0), yzmin, (float)(MBV_OPENGL_ZPROFILE1));
		mbv_draw_vertex3f((float)(MBV_OPENGL_WIDTH), yzmin, (float)(MBV_OPENGL_ZPROFILE1));
		mbv_draw_vertex3f((float)(MBV_OPENGL_WIDTH), yzmax, (float)(MBV_OPENGL_ZPROFILE1));
		mbv_draw_vertex3f((float)(0.0), yzmax, (float)(MBV_OPENGL_ZPROFILE1));
		mbv_draw_end();
#ifdef MBV_GETERRORS
		mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif

		if (opengl) {
			/* flush opengl buffers */
			glFlush();
#ifdef MBV_GETERRORS
			mbview_glerrorcheck(instance, 2, __func__);
#endif

			/* swap opengl buffers */
			mbv_render_present(MBV_RENDER_PROFILE, instance);
#ifdef MBV_GETERRORS
			mbview_glerrorcheck(instance, 3, __func__);
#endif
		}

		/* update info label */
		mbview_profile_text(instance);
	}

	/* print output debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return */
	return (status);
}
/*------------------------------------------------------------------------------*/
/* The camera of the profile view, as mbview_plotprofile() sets it up with
   OpenGL (glOrtho, then glTranslated), for a renderer that draws its batch
   (mbv_draw_batch.h) without OpenGL. The edges are floats, as in the drawing. */
int mbview_profile_camera(size_t instance, double projection[16], double modelview[16]) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	const float left = -0.1 * MBV_OPENGL_WIDTH;
	const float right = 1.1 * MBV_OPENGL_WIDTH;
	const float top = 0.5 * (right - left) * view->praspect_ratio;
	const float bottom = -top;
	mbv_camera_identity(projection);
	mbv_camera_ortho(projection, left, right, bottom, top, MBV_OPENGL_ZMIN2D, MBV_OPENGL_ZMAX2D);
	mbv_camera_identity(modelview);
	mbv_camera_translate(modelview, 0.0, 0.0, MBV_OPENGL_ZMIN2D);

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_profile_text(size_t instance) {

	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* update profile info */
	if (data->profile.npoints > 0 && data->profile.source != MBV_PROFILE_NONE) {
		snprintf(value_text, sizeof(value_text), ":::t\"Profile Source: %s\":t\" Length: %.2f m\":t\" Vertical Range: \":t\" %.2f to %.2f m\"",
		        data->profile.source_name, data->profile.points[data->profile.npoints - 1].distance, data->profile.zmin,
		        data->profile.zmax);
	}
	else {
		snprintf(value_text, sizeof(value_text), ":::t\"Profile Source: None\":t\"No Profile\"");
	}
	set_mbview_label_multiline_string(view->mb3dview.mbview_profile_label_info, value_text);

	/* print output debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
