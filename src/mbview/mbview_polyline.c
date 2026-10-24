/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_polyline.c	10/23/2026
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
 * Display of GIS layers and polylines (e.g. imported from shapefiles).
 * GIS polylines, polygons, and points are displayed but not edited; they
 * are shared by all mbview instances like sites and routes. This file also
 * holds the functions shared by mbview_polygon.c and mbview_gispoint.c and
 * the conversions between polylines and routes.
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
/* Check that parts start at increasing point indices beginning with 0 and that
    each part has at least minpartpoints points. Returns true if valid. */
int mbview_gis_checkparts(int npart, const int *partstart, int npoint, int minpartpoints) {
	if (npart < 1 || partstart == NULL || npoint < 1 || partstart[0] != 0)
		return (false);
	for (int i = 0; i < npart; i++) {
		const int end = i < npart - 1 ? partstart[i + 1] : npoint;
		if (end - partstart[i] < minpartpoints)
			return (false);
	}
	return (true);
}

/*------------------------------------------------------------------------------*/
/* Set a GIS vertex in grid and display coordinates for this instance and all
    other active instances. The vertex is draped on the primary grid of this
    instance; vertices off the grid or on cells without data are placed at the
    top of the grid's depth range so that they are never hidden by the grid. */
void mbview_gis_setpointw(size_t instance, double lon, double lat, struct mbview_pointw_struct *pointw) {
	struct mbview_struct *data = &(mbviews[instance].data);

	pointw->xlon = lon;
	pointw->ylat = lat;
	double zdata;
	if (mbview_projectll2xyzgrid(instance, lon, lat, &(pointw->xgrid[instance]), &(pointw->ygrid[instance]), &zdata) !=
	    MB_SUCCESS)
		zdata = data->primary_max;
	pointw->zdata = zdata;
	mbview_projectll2display(instance, lon, lat, zdata, &(pointw->xdisplay[instance]), &(pointw->ydisplay[instance]),
	                         &(pointw->zdisplay[instance]));
	mbview_updatepointw(instance, pointw);
}

/*------------------------------------------------------------------------------*/
/* Drape the segments joining the points of each part on the primary grid;
    the segment joining the last point of a part to the first point of the
    next part is not draped or drawn. */
int mbview_gis_drapesegments(size_t instance, int npart, const int *partstart, int npoints, struct mbview_pointw_struct *points,
                             struct mbview_linesegmentw_struct *segments) {
	for (int i = 0; i < npart; i++) {
		const int start = partstart[i];
		const int end = i < npart - 1 ? partstart[i + 1] : npoints;
		for (int j = start; j < end - 1; j++) {
			segments[j].endpoints[0] = points[j];
			segments[j].endpoints[1] = points[j + 1];
			mbview_drapesegmentw(instance, &(segments[j]));
			mbview_updatesegmentw(instance, &(segments[j]));
		}
		if (end < npoints)
			segments[end - 1].nls = 0;
	}
	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
void mbview_gis_freesegments(int nsegments, struct mbview_linesegmentw_struct *segments) {
	int error = MB_ERROR_NO_ERROR;
	for (int j = 0; j < nsegments; j++) {
		if (segments[j].lspoints != NULL)
			mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(segments[j].lspoints), &error);
		segments[j].nls = 0;
		segments[j].nls_alloc = 0;
	}
}

/*------------------------------------------------------------------------------*/
/* Draw each part as a line strip of draped segments; as with routes, this is
    done at every rendering resolution so the lines are not degraded while the
    topography is redrawn at low resolution during interaction. */
void mbview_gis_drawlines(size_t instance, int npart, const int *partstart, int npoints,
                          struct mbview_pointw_struct *points, struct mbview_linesegmentw_struct *segments, int icolor,
                          int size) {
	glColor3f(colortable_object_red[icolor], colortable_object_green[icolor], colortable_object_blue[icolor]);
	glLineWidth((float)size);
	for (int i = 0; i < npart; i++) {
		const int start = partstart[i];
		const int end = i < npart - 1 ? partstart[i + 1] : npoints;
		glBegin(GL_LINE_STRIP);
		for (int j = start; j < end - 1; j++) {
			/* segments within one grid cell are not draped */
			if (segments[j].nls > 0) {
				for (int k = 0; k < segments[j].nls; k++)
					glVertex3f((float)(segments[j].lspoints[k].xdisplay[instance]),
					           (float)(segments[j].lspoints[k].ydisplay[instance]),
					           (float)(segments[j].lspoints[k].zdisplay[instance]));
			}
			else {
				glVertex3f((float)(points[j].xdisplay[instance]), (float)(points[j].ydisplay[instance]),
				           (float)(points[j].zdisplay[instance]));
				glVertex3f((float)(points[j + 1].xdisplay[instance]), (float)(points[j + 1].ydisplay[instance]),
				           (float)(points[j + 1].zdisplay[instance]));
			}
		}
		glEnd();
	}
}

/*------------------------------------------------------------------------------*/
int mbview_addgislayer(int verbose, size_t instance, int type, bool hasz, const char *name, int *ilayer, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       type:                      %d\n", type);
		fprintf(stderr, "dbg2       hasz:                      %d\n", hasz);
		fprintf(stderr, "dbg2       name:                      %s\n", name != NULL ? name : "");
	}

	int status = MB_SUCCESS;
	*ilayer = -1;
	*error = MB_ERROR_NO_ERROR;
	if (type < MBV_GISLAYER_POLYLINE || type > MBV_GISLAYER_GISPOINT) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}

	/* allocate memory for a new layer if required */
	if (status == MB_SUCCESS && shared.shareddata.ngislayer_alloc < shared.shareddata.ngislayer + 1) {
		const int nalloc = shared.shareddata.ngislayer_alloc + MBV_ALLOC_NUM;
		status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc * sizeof(struct mbview_gislayer_struct),
		                     (void **)&(shared.shareddata.gislayers), error);
		if (status == MB_SUCCESS) {
			shared.shareddata.ngislayer_alloc = nalloc;
		}
		else {
			shared.shareddata.ngislayer = 0;
			shared.shareddata.ngislayer_alloc = 0;
		}
	}

	/* add the layer */
	if (status == MB_SUCCESS) {
		struct mbview_gislayer_struct *layer = &(shared.shareddata.gislayers[shared.shareddata.ngislayer]);
		layer->active = true;
		layer->type = type;
		layer->hasz = hasz;
		snprintf(layer->name, sizeof(mb_path), "%s", name != NULL ? name : "");
		*ilayer = shared.shareddata.ngislayer;
		shared.shareddata.ngislayer++;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ilayer:                    %d\n", *ilayer);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_getgislayercount(int verbose, size_t instance, int *nlayer, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	*nlayer = shared.shareddata.ngislayer;
	*error = MB_ERROR_NO_ERROR;
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nlayer:                    %d\n", *nlayer);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_getgislayer(int verbose, size_t instance, int ilayer, int *type, bool *hasz, bool *active, mb_path name, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       ilayer:                    %d\n", ilayer);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	if (ilayer < 0 || ilayer >= shared.shareddata.ngislayer) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}
	else {
		const struct mbview_gislayer_struct *layer = &(shared.shareddata.gislayers[ilayer]);
		*type = layer->type;
		*hasz = layer->hasz;
		*active = layer->active;
		snprintf(name, sizeof(mb_path), "%s", layer->name);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		if (status == MB_SUCCESS) {
			fprintf(stderr, "dbg2       type:                      %d\n", *type);
			fprintf(stderr, "dbg2       hasz:                      %d\n", *hasz);
			fprintf(stderr, "dbg2       active:                    %d\n", *active);
			fprintf(stderr, "dbg2       name:                      %s\n", name);
		}
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_setgislayeractive(int verbose, size_t instance, int ilayer, bool active, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       ilayer:                    %d\n", ilayer);
		fprintf(stderr, "dbg2       active:                    %d\n", active);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	if (ilayer < 0 || ilayer >= shared.shareddata.ngislayer) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}
	else {
		shared.shareddata.gislayers[ilayer].active = active;
	}

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
int mbview_getpolylinecount(int verbose, size_t instance, int *npolyline, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	*npolyline = shared.shareddata.npolyline;
	*error = MB_ERROR_NO_ERROR;
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       npolyline:                 %d\n", *npolyline);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_getpolylinepointcount(int verbose, size_t instance, int ipolyline, int *npart, int *npoint, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       ipolyline:                 %d\n", ipolyline);
	}

	int status = MB_SUCCESS;
	*npart = 0;
	*npoint = 0;
	*error = MB_ERROR_NO_ERROR;
	if (ipolyline >= 0 && ipolyline < shared.shareddata.npolyline) {
		*npart = shared.shareddata.polylines[ipolyline].npart;
		*npoint = shared.shareddata.polylines[ipolyline].npoints;
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       npart:                     %d\n", *npart);
		fprintf(stderr, "dbg2       npoint:                    %d\n", *npoint);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_allocpolylinearrays(int verbose, int npart, int npoint, int **partstart, double **lon, double **lat, double **z,
                               int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       npart:                     %d\n", npart);
		fprintf(stderr, "dbg2       npoint:                    %d\n", npoint);
	}

	/* allocate the arrays using mb_reallocd */
	int status = mb_reallocd(verbose, __FILE__, __LINE__, npart * sizeof(int), (void **)partstart, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npoint * sizeof(double), (void **)lon, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npoint * sizeof(double), (void **)lat, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npoint * sizeof(double), (void **)z, error);
	if (status == MB_FAILURE) {
		int error_free = MB_ERROR_NO_ERROR;
		mbview_freepolylinearrays(verbose, partstart, lon, lat, z, &error_free);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       partstart:                 %p\n", (void *)*partstart);
		fprintf(stderr, "dbg2       lon:                       %p\n", (void *)*lon);
		fprintf(stderr, "dbg2       lat:                       %p\n", (void *)*lat);
		fprintf(stderr, "dbg2       z:                         %p\n", (void *)*z);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_freepolylinearrays(int verbose, int **partstart, double **lon, double **lat, double **z, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       partstart:                 %p\n", (void *)*partstart);
		fprintf(stderr, "dbg2       lon:                       %p\n", (void *)*lon);
		fprintf(stderr, "dbg2       lat:                       %p\n", (void *)*lat);
		fprintf(stderr, "dbg2       z:                         %p\n", (void *)*z);
	}

	/* free the arrays using mb_freed */
	int status = mb_freed(verbose, __FILE__, __LINE__, (void **)partstart, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)lon, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)lat, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)z, error);

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
/* Add a polyline of npart parts starting at the point indices in partstart
    (each part with at least two points) in the GIS layer layer, which must be
    a polyline layer. z holds the source z values (may be NULL); the display
    drapes the polyline on the primary grid of instance. */
int mbview_addpolyline(int verbose, size_t instance, int npart, int *partstart, int npoint, double *lon, double *lat, double *z,
                       int color, int size, const char *name, int layer, int *ipolyline, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       npart:                     %d\n", npart);
		fprintf(stderr, "dbg2       npoint:                    %d\n", npoint);
		fprintf(stderr, "dbg2       color:                     %d\n", color);
		fprintf(stderr, "dbg2       size:                      %d\n", size);
		fprintf(stderr, "dbg2       name:                      %s\n", name != NULL ? name : "");
		fprintf(stderr, "dbg2       layer:                     %d\n", layer);
	}

	int status = MB_SUCCESS;
	*ipolyline = -1;
	*error = MB_ERROR_NO_ERROR;

	/* check the arguments */
	if (instance >= MBV_MAX_WINDOWS || !mbview_gis_checkparts(npart, partstart, npoint, 2) || lon == NULL || lat == NULL ||
	    color < MBV_COLOR_BLACK || color > MBV_COLOR_PURPLE || size < 1 || layer < 0 || layer >= shared.shareddata.ngislayer ||
	    shared.shareddata.gislayers[layer].type != MBV_GISLAYER_POLYLINE) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}

	/* allocate memory for a new polyline if required */
	if (status == MB_SUCCESS && shared.shareddata.npolyline_alloc < shared.shareddata.npolyline + 1) {
		const int nalloc = shared.shareddata.npolyline_alloc + MBV_ALLOC_NUM;
		status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc * sizeof(struct mbview_polyline_struct),
		                     (void **)&(shared.shareddata.polylines), error);
		if (status == MB_SUCCESS) {
			memset(&(shared.shareddata.polylines[shared.shareddata.npolyline_alloc]), 0,
			       (nalloc - shared.shareddata.npolyline_alloc) * sizeof(struct mbview_polyline_struct));
			shared.shareddata.npolyline_alloc = nalloc;
		}
		else {
			shared.shareddata.npolyline = 0;
			shared.shareddata.npolyline_alloc = 0;
		}
	}

	/* allocate the polyline arrays */
	struct mbview_polyline_struct *polyline = NULL;
	if (status == MB_SUCCESS) {
		polyline = &(shared.shareddata.polylines[shared.shareddata.npolyline]);
		memset(polyline, 0, sizeof(struct mbview_polyline_struct));
		status = mb_mallocd(verbose, __FILE__, __LINE__, npart * sizeof(int), (void **)&(polyline->partstart), error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, npoint * sizeof(double), (void **)&(polyline->zsource), error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, npoint * sizeof(struct mbview_pointw_struct),
			                    (void **)&(polyline->points), error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, npoint * sizeof(struct mbview_linesegmentw_struct),
			                    (void **)&(polyline->segments), error);
		if (status == MB_SUCCESS) {
			memset(polyline->points, 0, npoint * sizeof(struct mbview_pointw_struct));
			memset(polyline->segments, 0, npoint * sizeof(struct mbview_linesegmentw_struct));
		}
		else {
			int error_free = MB_ERROR_NO_ERROR;
			mb_freed(verbose, __FILE__, __LINE__, (void **)&(polyline->partstart), &error_free);
			mb_freed(verbose, __FILE__, __LINE__, (void **)&(polyline->zsource), &error_free);
			mb_freed(verbose, __FILE__, __LINE__, (void **)&(polyline->points), &error_free);
			mb_freed(verbose, __FILE__, __LINE__, (void **)&(polyline->segments), &error_free);
		}
	}

	/* add the polyline */
	if (status == MB_SUCCESS) {
		polyline->active = true;
		polyline->color = color;
		polyline->size = size;
		polyline->layer = layer;
		snprintf(polyline->name, sizeof(mb_path), "%s", name != NULL ? name : "");
		polyline->npart = npart;
		memcpy(polyline->partstart, partstart, npart * sizeof(int));
		polyline->npoints = npoint;
		polyline->npoints_alloc = npoint;
		for (int j = 0; j < npoint; j++) {
			polyline->zsource[j] = z != NULL ? z[j] : 0.0;
			mbview_gis_setpointw(instance, lon[j], lat[j], &(polyline->points[j]));
		}
		mbview_gis_drapesegments(instance, npart, polyline->partstart, npoint, polyline->points, polyline->segments);
		*ipolyline = shared.shareddata.npolyline;
		shared.shareddata.npolyline++;

		/* make polylines viewable in this instance */
		mbviews[instance].data.polyline_view_mode = MBV_VIEW_ON;
		set_mbview_polyline_view_mode(instance, MBV_VIEW_ON);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ipolyline:                 %d\n", *ipolyline);
		fprintf(stderr, "dbg2       npolyline:                 %d\n", shared.shareddata.npolyline);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
/* Get a polyline; the arrays must hold the numbers of parts and points
    returned by mbview_getpolylinepointcount(). z returns the source z values. */
int mbview_getpolyline(int verbose, size_t instance, int ipolyline, int *npart, int *partstart, int *npoint, double *lon,
                       double *lat, double *z, int *color, int *size, mb_path name, int *layer, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       ipolyline:                 %d\n", ipolyline);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	if (ipolyline < 0 || ipolyline >= shared.shareddata.npolyline) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}
	else {
		const struct mbview_polyline_struct *polyline = &(shared.shareddata.polylines[ipolyline]);
		*npart = polyline->npart;
		memcpy(partstart, polyline->partstart, polyline->npart * sizeof(int));
		*npoint = polyline->npoints;
		for (int j = 0; j < polyline->npoints; j++) {
			lon[j] = polyline->points[j].xlon;
			lat[j] = polyline->points[j].ylat;
			z[j] = polyline->zsource[j];
		}
		*color = polyline->color;
		*size = polyline->size;
		snprintf(name, sizeof(mb_path), "%s", polyline->name);
		*layer = polyline->layer;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		if (status == MB_SUCCESS) {
			fprintf(stderr, "dbg2       npart:                     %d\n", *npart);
			fprintf(stderr, "dbg2       npoint:                    %d\n", *npoint);
			fprintf(stderr, "dbg2       color:                     %d\n", *color);
			fprintf(stderr, "dbg2       size:                      %d\n", *size);
			fprintf(stderr, "dbg2       name:                      %s\n", name);
			fprintf(stderr, "dbg2       layer:                     %d\n", *layer);
		}
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_deletepolyline(int verbose, size_t instance, int ipolyline, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       ipolyline:                 %d\n", ipolyline);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	if (ipolyline < 0 || ipolyline >= shared.shareddata.npolyline) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}
	else {
		/* free memory for the deleted polyline */
		struct mbview_polyline_struct *polyline = &(shared.shareddata.polylines[ipolyline]);
		int error_free = MB_ERROR_NO_ERROR;
		mbview_gis_freesegments(polyline->npoints, polyline->segments);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(polyline->partstart), &error_free);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(polyline->zsource), &error_free);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(polyline->points), &error_free);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(polyline->segments), &error_free);

		/* move the following polylines down */
		for (int i = ipolyline; i < shared.shareddata.npolyline - 1; i++)
			shared.shareddata.polylines[i] = shared.shareddata.polylines[i + 1];
		shared.shareddata.npolyline--;
		memset(&(shared.shareddata.polylines[shared.shareddata.npolyline]), 0, sizeof(struct mbview_polyline_struct));

		/* keep the selection pointing at the same polyline */
		if (shared.shareddata.polyline_selected == ipolyline)
			shared.shareddata.polyline_selected = MBV_SELECT_NONE;
		else if (shared.shareddata.polyline_selected > ipolyline)
			shared.shareddata.polyline_selected--;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       npolyline:                 %d\n", shared.shareddata.npolyline);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_deleteallpolylines(int verbose, size_t instance, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	int status = MB_SUCCESS;
	while (status == MB_SUCCESS && shared.shareddata.npolyline > 0)
		status = mbview_deletepolyline(verbose, instance, shared.shareddata.npolyline - 1, error);

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
int mbview_enableviewpolylines(int verbose, size_t instance, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* set values */
	shared.shareddata.polyline_mode = MBV_POLYLINE_VIEW;

	/* the route list can now convert routes to polylines */
	mbview_gis_routelistbutton();

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
/* Convert each part of a polyline to an editable route with a simple waypoint
    at each vertex; the polyline is kept. Vertices that cannot be placed in the
    display of instance are dropped, as are parts left with fewer than two
    points; nroute returns the number of routes added and ndropped the number
    of vertices not used. */
int mbview_polylinetoroutes(int verbose, size_t instance, int ipolyline, int *nroute, int *ndropped, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       ipolyline:                 %d\n", ipolyline);
	}

	int status = MB_SUCCESS;
	*nroute = 0;
	*ndropped = 0;
	*error = MB_ERROR_NO_ERROR;
	if (instance >= MBV_MAX_WINDOWS || ipolyline < 0 || ipolyline >= shared.shareddata.npolyline) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}

	/* work arrays large enough for any part */
	double *routelon = NULL;
	double *routelat = NULL;
	int *waypoint = NULL;
	struct mbview_polyline_struct *polyline = NULL;
	if (status == MB_SUCCESS) {
		polyline = &(shared.shareddata.polylines[ipolyline]);
		status = mb_mallocd(verbose, __FILE__, __LINE__, polyline->npoints * sizeof(double), (void **)&routelon, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, polyline->npoints * sizeof(double), (void **)&routelat, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, polyline->npoints * sizeof(int), (void **)&waypoint, error);
	}

	for (int i = 0; status == MB_SUCCESS && i < polyline->npart; i++) {
		const int start = polyline->partstart[i];
		const int end = i < polyline->npart - 1 ? polyline->partstart[i + 1] : polyline->npoints;

		/* keep the vertices that mbview_addroute() can place in the display */
		int n = 0;
		for (int j = start; j < end; j++) {
			const double xlon = polyline->points[j].xlon;
			const double ylat = polyline->points[j].ylat;
			double xgrid, ygrid, zdata, xdisplay, ydisplay, zdisplay;
			mbview_projectll2xyzgrid(instance, xlon, ylat, &xgrid, &ygrid, &zdata);
			mbview_projectll2display(instance, xlon, ylat, zdata, &xdisplay, &ydisplay, &zdisplay);
			if (fabs(xdisplay) < 1000.0 && fabs(ydisplay) < 1000.0 && fabs(zdisplay) < 1000.0) {
				routelon[n] = xlon;
				routelat[n] = ylat;
				waypoint[n] = MBV_ROUTE_WAYPOINT_SIMPLE;
				n++;
			}
		}
		if (n < 2) {
			*ndropped += end - start;
			continue;
		}
		*ndropped += end - start - n;

		mb_path routename;
		if (polyline->npart > 1)
			snprintf(routename, sizeof(routename), "%.1000s part %d", polyline->name, i + 1);
		else
			snprintf(routename, sizeof(routename), "%s", polyline->name);
		int iroute;
		status = mbview_addroute(verbose, instance, n, routelon, routelat, waypoint, polyline->color, polyline->size, true,
		                         routename, &iroute, error);
		if (status == MB_SUCCESS)
			(*nroute)++;
	}

	int error_free = MB_ERROR_NO_ERROR;
	mb_freed(verbose, __FILE__, __LINE__, (void **)&routelon, &error_free);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&routelat, &error_free);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&waypoint, &error_free);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nroute:                    %d\n", *nroute);
		fprintf(stderr, "dbg2       ndropped:                  %d\n", *ndropped);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
/* Convert a route to a one part polyline through its waypoints, with the
    route color, size, and name and the topography at the waypoints as z. If
    layer is negative the polyline goes in a polyline layer named
    MBV_GISLAYER_FROMROUTES, which is added if necessary. The route is kept. */
int mbview_routetopolyline(int verbose, size_t instance, int iroute, int layer, int *ipolyline, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       iroute:                    %d\n", iroute);
		fprintf(stderr, "dbg2       layer:                     %d\n", layer);
	}

	int status = MB_SUCCESS;
	*ipolyline = -1;
	*error = MB_ERROR_NO_ERROR;
	if (instance >= MBV_MAX_WINDOWS || iroute < 0 || iroute >= shared.shareddata.nroute ||
	    shared.shareddata.routes[iroute].npoints < 2) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}

	/* find or add the layer for converted routes */
	if (status == MB_SUCCESS && layer < 0) {
		for (int i = 0; i < shared.shareddata.ngislayer && layer < 0; i++) {
			if (shared.shareddata.gislayers[i].type == MBV_GISLAYER_POLYLINE &&
			    strcmp(shared.shareddata.gislayers[i].name, MBV_GISLAYER_FROMROUTES) == 0)
				layer = i;
		}
		if (layer < 0)
			status = mbview_addgislayer(verbose, instance, MBV_GISLAYER_POLYLINE, true, MBV_GISLAYER_FROMROUTES, &layer, error);
	}

	/* copy the waypoints */
	double *lon = NULL;
	double *lat = NULL;
	double *z = NULL;
	int *partstart = NULL;
	const struct mbview_route_struct *route = status == MB_SUCCESS ? &(shared.shareddata.routes[iroute]) : NULL;
	if (status == MB_SUCCESS)
		status = mbview_allocpolylinearrays(verbose, 1, route->npoints, &partstart, &lon, &lat, &z, error);
	if (status == MB_SUCCESS) {
		partstart[0] = 0;
		for (int j = 0; j < route->npoints; j++) {
			lon[j] = route->points[j].xlon;
			lat[j] = route->points[j].ylat;
			z[j] = route->points[j].zdata;
		}
		status = mbview_addpolyline(verbose, instance, 1, partstart, route->npoints, lon, lat, z, route->color,
		                            MAX(route->size, 1), route->name, layer, ipolyline, error);
	}
	int error_free = MB_ERROR_NO_ERROR;
	mbview_freepolylinearrays(verbose, &partstart, &lon, &lat, &z, &error_free);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ipolyline:                 %d\n", *ipolyline);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_drawpolyline(size_t instance, int rez) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       rez:              %d\n", rez);
	}

	struct mbview_struct *data = &(mbviews[instance].data);

	if (shared.shareddata.polyline_mode != MBV_POLYLINE_OFF && data->polyline_view_mode == MBV_VIEW_ON &&
	    shared.shareddata.npolyline > 0) {
		for (int i = 0; i < shared.shareddata.npolyline; i++) {
			struct mbview_polyline_struct *polyline = &(shared.shareddata.polylines[i]);
			if (polyline->active && shared.shareddata.gislayers[polyline->layer].active) {
				const int icolor = i == shared.shareddata.polyline_selected ? MBV_COLOR_RED : polyline->color;
				mbview_gis_drawlines(instance, polyline->npart, polyline->partstart, polyline->npoints, polyline->points,
				                     polyline->segments, icolor, polyline->size);
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
