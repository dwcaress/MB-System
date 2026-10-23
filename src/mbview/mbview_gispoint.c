/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_gispoint.c	10/23/2026
 *
 *    Copyright (c) 2026-2026 by
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
 * Display of GIS points (e.g. imported from shapefiles). The points are
 * displayed as triangles (spheres in 3D) to distinguish them from sites;
 * they are not edited and are shared by all mbview instances like sites.
 * The functions shared with polylines are in mbview_polyline.c.
 *
 * Author:	D. W. Caress
 * Date:	October 23, 2026
 */

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
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
#include <Xm/List.h>
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

/*------------------------------------------------------------------------------*/
int mbview_getgispointcount(int verbose, size_t instance, int *ngispoint, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	*ngispoint = shared.shareddata.ngispoint;
	*error = MB_ERROR_NO_ERROR;
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ngispoint:                 %d\n", *ngispoint);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_allocgispointarrays(int verbose, int npoint, double **lon, double **lat, double **z, int **color, int **size,
                               int **layer, mb_path **name, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       npoint:                    %d\n", npoint);
	}

	/* allocate the arrays using mb_reallocd */
	int status = mb_reallocd(verbose, __FILE__, __LINE__, npoint * sizeof(double), (void **)lon, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npoint * sizeof(double), (void **)lat, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npoint * sizeof(double), (void **)z, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npoint * sizeof(int), (void **)color, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npoint * sizeof(int), (void **)size, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npoint * sizeof(int), (void **)layer, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npoint * sizeof(mb_path), (void **)name, error);
	if (status == MB_FAILURE) {
		int error_free = MB_ERROR_NO_ERROR;
		mbview_freegispointarrays(verbose, lon, lat, z, color, size, layer, name, &error_free);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       lon:                       %p\n", (void *)*lon);
		fprintf(stderr, "dbg2       lat:                       %p\n", (void *)*lat);
		fprintf(stderr, "dbg2       z:                         %p\n", (void *)*z);
		fprintf(stderr, "dbg2       color:                     %p\n", (void *)*color);
		fprintf(stderr, "dbg2       size:                      %p\n", (void *)*size);
		fprintf(stderr, "dbg2       layer:                     %p\n", (void *)*layer);
		fprintf(stderr, "dbg2       name:                      %p\n", (void *)*name);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_freegispointarrays(int verbose, double **lon, double **lat, double **z, int **color, int **size, int **layer,
                              mb_path **name, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       lon:                       %p\n", (void *)*lon);
		fprintf(stderr, "dbg2       lat:                       %p\n", (void *)*lat);
		fprintf(stderr, "dbg2       z:                         %p\n", (void *)*z);
		fprintf(stderr, "dbg2       color:                     %p\n", (void *)*color);
		fprintf(stderr, "dbg2       size:                      %p\n", (void *)*size);
		fprintf(stderr, "dbg2       layer:                     %p\n", (void *)*layer);
		fprintf(stderr, "dbg2       name:                      %p\n", (void *)*name);
	}

	/* free the arrays using mb_freed */
	int status = mb_freed(verbose, __FILE__, __LINE__, (void **)lon, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)lat, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)z, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)color, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)size, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)layer, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)name, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
/* Add npoint GIS points, each in a GIS layer that must be a point layer.
    z holds the source z values (may be NULL); the display drapes the points
    on the primary grid of instance. Nothing is added if any point is invalid. */
int mbview_addgispoints(int verbose, size_t instance, int npoint, double *lon, double *lat, double *z, int *color, int *size,
                        int *layer, mb_path *name, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       npoint:                    %d\n", npoint);
		for (int i = 0; i < npoint && lon != NULL && lat != NULL && color != NULL && size != NULL && layer != NULL && name != NULL;
		     i++)
			fprintf(stderr, "dbg2       point %d: lon:%f lat:%f color:%d size:%d layer:%d name:%s\n", i, lon[i], lat[i], color[i],
			        size[i], layer[i], name[i]);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* check the arguments */
	if (instance >= MBV_MAX_WINDOWS || npoint < 0 || (npoint > 0 && (lon == NULL || lat == NULL || color == NULL ||
	                                                                  size == NULL || layer == NULL || name == NULL))) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}
	for (int i = 0; status == MB_SUCCESS && i < npoint; i++) {
		if (color[i] < MBV_COLOR_BLACK || color[i] > MBV_COLOR_PURPLE || size[i] < 1 || layer[i] < 0 ||
		    layer[i] >= shared.shareddata.ngislayer || shared.shareddata.gislayers[layer[i]].type != MBV_GISLAYER_GISPOINT) {
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_PARAMETER;
		}
	}

	/* allocate memory for the new points if required */
	if (status == MB_SUCCESS && shared.shareddata.ngispoint_alloc < shared.shareddata.ngispoint + npoint) {
		const int nalloc = shared.shareddata.ngispoint + npoint + MBV_ALLOC_NUM;
		status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc * sizeof(struct mbview_gispoint_struct),
		                     (void **)&(shared.shareddata.gispoints), error);
		if (status == MB_SUCCESS) {
			memset(&(shared.shareddata.gispoints[shared.shareddata.ngispoint_alloc]), 0,
			       (nalloc - shared.shareddata.ngispoint_alloc) * sizeof(struct mbview_gispoint_struct));
			shared.shareddata.ngispoint_alloc = nalloc;
		}
		else {
			shared.shareddata.ngispoint = 0;
			shared.shareddata.ngispoint_alloc = 0;
		}
	}

	/* add the points */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < npoint; i++) {
			struct mbview_gispoint_struct *gispoint = &(shared.shareddata.gispoints[shared.shareddata.ngispoint]);
			memset(gispoint, 0, sizeof(struct mbview_gispoint_struct));
			gispoint->active = true;
			gispoint->zsource = z != NULL ? z[i] : 0.0;
			gispoint->color = color[i];
			gispoint->size = size[i];
			gispoint->layer = layer[i];
			snprintf(gispoint->name, sizeof(mb_path), "%s", name[i]);
			mbview_gis_setpointw(instance, lon[i], lat[i], &(gispoint->point));
			shared.shareddata.ngispoint++;
		}

		/* make points viewable in this instance */
		if (npoint > 0) {
			mbviews[instance].data.gispoint_view_mode = MBV_VIEW_ON;
			set_mbview_gispoint_view_mode(instance, MBV_VIEW_ON);
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ngispoint:                 %d\n", shared.shareddata.ngispoint);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
/* Get all GIS points; the arrays must hold mbview_getgispointcount() points.
    z returns the source z values. */
int mbview_getgispoints(int verbose, size_t instance, int *npoint, double *lon, double *lat, double *z, int *color, int *size,
                        int *layer, mb_path *name, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	*npoint = shared.shareddata.ngispoint;
	for (int i = 0; i < shared.shareddata.ngispoint; i++) {
		const struct mbview_gispoint_struct *gispoint = &(shared.shareddata.gispoints[i]);
		lon[i] = gispoint->point.xlon;
		lat[i] = gispoint->point.ylat;
		z[i] = gispoint->zsource;
		color[i] = gispoint->color;
		size[i] = gispoint->size;
		layer[i] = gispoint->layer;
		snprintf(name[i], sizeof(mb_path), "%s", gispoint->name);
	}
	*error = MB_ERROR_NO_ERROR;
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       npoint:                    %d\n", *npoint);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_deletegispoint(int verbose, size_t instance, int igispoint, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       igispoint:                 %d\n", igispoint);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	if (igispoint < 0 || igispoint >= shared.shareddata.ngispoint) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}
	else {
		/* move the following points down */
		for (int i = igispoint; i < shared.shareddata.ngispoint - 1; i++)
			shared.shareddata.gispoints[i] = shared.shareddata.gispoints[i + 1];
		shared.shareddata.ngispoint--;
		memset(&(shared.shareddata.gispoints[shared.shareddata.ngispoint]), 0, sizeof(struct mbview_gispoint_struct));

		/* keep the selection pointing at the same point */
		if (shared.shareddata.gispoint_selected == igispoint)
			shared.shareddata.gispoint_selected = MBV_SELECT_NONE;
		else if (shared.shareddata.gispoint_selected > igispoint)
			shared.shareddata.gispoint_selected--;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ngispoint:                 %d\n", shared.shareddata.ngispoint);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_deleteallgispoints(int verbose, size_t instance, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	if (shared.shareddata.ngispoint > 0)
		memset(shared.shareddata.gispoints, 0, shared.shareddata.ngispoint * sizeof(struct mbview_gispoint_struct));
	shared.shareddata.ngispoint = 0;
	shared.shareddata.gispoint_selected = MBV_SELECT_NONE;
	*error = MB_ERROR_NO_ERROR;
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_enableviewgispoints(int verbose, size_t instance, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* set values */
	shared.shareddata.gispoint_mode = MBV_GISPOINT_VIEW;

	/* set widget sensitivity on all active instances */
	for (size_t i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (mbviews[i].data.active)
			mbview_update_sensitivity(verbose, i, error);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_drawgispoint(size_t instance, int rez) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       rez:              %d\n", rez);
	}

	struct mbview_struct *data = &(mbviews[instance].data);

	if (shared.shareddata.gispoint_mode != MBV_GISPOINT_OFF && data->gispoint_view_mode == MBV_VIEW_ON &&
	    shared.shareddata.ngispoint > 0) {
		/* get size according to viewbounds, as for sites */
		const int k0 = data->viewbounds[0] * data->primary_n_rows + data->viewbounds[2];
		const int k1 = data->viewbounds[1] * data->primary_n_rows + data->viewbounds[3];
		const double xx = data->primary_x[k1] - data->primary_x[k0];
		const double yy = data->primary_y[k1] - data->primary_y[k0];
		const double pointsizesmall = 0.004 * sqrt(xx * xx + yy * yy);
		const double pointsizelarge = 1.4 * pointsizesmall;

		/* Use outlined triangles for 2D plotting */
		if (data->display_mode == MBV_DISPLAY_2D) {
			glNewList((GLuint)MBV_GLLIST_GISPOINTSMALL, GL_COMPILE);
			GLUquadricObj *globj = gluNewQuadric();
			gluDisk(globj, 0.0, pointsizesmall, 3, 1);
			gluDeleteQuadric(globj);
			int icolor = MBV_COLOR_BLACK;
			glColor3f(colortable_object_red[icolor], colortable_object_green[icolor], colortable_object_blue[icolor]);
			globj = gluNewQuadric();
			gluDisk(globj, 0.7 * pointsizesmall, pointsizesmall, 3, 1);
			gluDeleteQuadric(globj);
			glEndList();

			glNewList((GLuint)MBV_GLLIST_GISPOINTLARGE, GL_COMPILE);
			globj = gluNewQuadric();
			gluDisk(globj, 0.0, pointsizelarge, 3, 1);
			gluDeleteQuadric(globj);
			icolor = MBV_COLOR_BLACK;
			glColor3f(colortable_object_red[icolor], colortable_object_green[icolor], colortable_object_blue[icolor]);
			globj = gluNewQuadric();
			gluDisk(globj, 0.7 * pointsizelarge, pointsizelarge, 3, 1);
			gluDeleteQuadric(globj);
			glEndList();
		}

		/* Use coarse spheres for 3D plotting */
		else if (data->display_mode == MBV_DISPLAY_3D) {
			glNewList((GLuint)MBV_GLLIST_GISPOINTSMALL, GL_COMPILE);
			GLUquadricObj *globj = gluNewQuadric();
			gluSphere(globj, pointsizesmall, 3, 2);
			gluDeleteQuadric(globj);
			glEndList();

			glNewList((GLuint)MBV_GLLIST_GISPOINTLARGE, GL_COMPILE);
			globj = gluNewQuadric();
			gluSphere(globj, pointsizelarge, 3, 2);
			gluDeleteQuadric(globj);
			glEndList();
		}

		/* loop over the points */
		for (int i = 0; i < shared.shareddata.ngispoint; i++) {
			struct mbview_gispoint_struct *gispoint = &(shared.shareddata.gispoints[i]);
			if (gispoint->active && shared.shareddata.gislayers[gispoint->layer].active) {
				const int icolor = i == shared.shareddata.gispoint_selected ? MBV_COLOR_RED : gispoint->color;
				glColor3f(colortable_object_red[icolor], colortable_object_green[icolor], colortable_object_blue[icolor]);
				glTranslatef(gispoint->point.xdisplay[instance], gispoint->point.ydisplay[instance],
				             gispoint->point.zdisplay[instance]);
				if (i == shared.shareddata.gispoint_selected || gispoint->size > 1)
					glCallList((GLuint)MBV_GLLIST_GISPOINTLARGE);
				else
					glCallList((GLuint)MBV_GLLIST_GISPOINTSMALL);
				glTranslatef(-gispoint->point.xdisplay[instance], -gispoint->point.ydisplay[instance],
				             -gispoint->point.zdisplay[instance]);
			}
		}
	}

#ifdef MBV_GETERRORS
	mbview_glerrorcheck(instance, 1, __func__);
#endif

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
