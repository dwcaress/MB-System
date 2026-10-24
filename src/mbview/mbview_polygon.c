/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_polygon.c	10/23/2026
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
 * Display of GIS polygons (e.g. imported from shapefiles). Polygons are
 * displayed as the outlines of their rings but not edited; they are shared
 * by all mbview instances like sites and routes. The functions shared with
 * polylines are in mbview_polyline.c.
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
int mbview_getpolygoncount(int verbose, size_t instance, int *npolygon, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	*npolygon = shared.shareddata.npolygon;
	*error = MB_ERROR_NO_ERROR;
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       npolygon:                  %d\n", *npolygon);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_getpolygonpointcount(int verbose, size_t instance, int ipolygon, int *npart, int *npoint, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       ipolygon:                  %d\n", ipolygon);
	}

	int status = MB_SUCCESS;
	*npart = 0;
	*npoint = 0;
	*error = MB_ERROR_NO_ERROR;
	if (ipolygon >= 0 && ipolygon < shared.shareddata.npolygon) {
		*npart = shared.shareddata.polygons[ipolygon].npart;
		*npoint = shared.shareddata.polygons[ipolygon].npoints;
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
int mbview_allocpolygonarrays(int verbose, int npart, int npoint, int **partstart, int **parttype, double **lon, double **lat,
                              double **z, int *error) {
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
		status = mb_reallocd(verbose, __FILE__, __LINE__, npart * sizeof(int), (void **)parttype, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npoint * sizeof(double), (void **)lon, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npoint * sizeof(double), (void **)lat, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npoint * sizeof(double), (void **)z, error);
	if (status == MB_FAILURE) {
		int error_free = MB_ERROR_NO_ERROR;
		mbview_freepolygonarrays(verbose, partstart, parttype, lon, lat, z, &error_free);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       partstart:                 %p\n", (void *)*partstart);
		fprintf(stderr, "dbg2       parttype:                  %p\n", (void *)*parttype);
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
int mbview_freepolygonarrays(int verbose, int **partstart, int **parttype, double **lon, double **lat, double **z, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       partstart:                 %p\n", (void *)*partstart);
		fprintf(stderr, "dbg2       parttype:                  %p\n", (void *)*parttype);
		fprintf(stderr, "dbg2       lon:                       %p\n", (void *)*lon);
		fprintf(stderr, "dbg2       lat:                       %p\n", (void *)*lat);
		fprintf(stderr, "dbg2       z:                         %p\n", (void *)*z);
	}

	/* free the arrays using mb_freed */
	int status = mb_freed(verbose, __FILE__, __LINE__, (void **)partstart, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)parttype, error);
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
/* Add a polygon of npart rings starting at the point indices in partstart
    (each with at least three points), of the types in parttype (outer or hole,
    NULL for all outer), in the GIS layer layer, which must be a polygon layer.
    Rings that are not closed are closed by repeating their first point.
    z holds the source z values (may be NULL); the display drapes the outlines
    on the primary grid of instance. fillcolor is MBV_GISCOLOR_NONE or a color. */
int mbview_addpolygon(int verbose, size_t instance, int npart, int *partstart, int *parttype, int npoint, double *lon,
                      double *lat, double *z, int color, int size, int fillcolor, const char *name, int layer, int *ipolygon,
                      int *error) {
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
		fprintf(stderr, "dbg2       fillcolor:                 %d\n", fillcolor);
		fprintf(stderr, "dbg2       name:                      %s\n", name != NULL ? name : "");
		fprintf(stderr, "dbg2       layer:                     %d\n", layer);
	}

	int status = MB_SUCCESS;
	*ipolygon = -1;
	*error = MB_ERROR_NO_ERROR;

	/* check the arguments */
	if (instance >= MBV_MAX_WINDOWS || !mbview_gis_checkparts(npart, partstart, npoint, 3) || lon == NULL || lat == NULL ||
	    color < MBV_COLOR_BLACK || color > MBV_COLOR_PURPLE || size < 1 || fillcolor < MBV_GISCOLOR_NONE ||
	    fillcolor > MBV_COLOR_PURPLE || layer < 0 || layer >= shared.shareddata.ngislayer ||
	    shared.shareddata.gislayers[layer].type != MBV_GISLAYER_POLYGON) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}
	for (int i = 0; status == MB_SUCCESS && parttype != NULL && i < npart; i++) {
		if (parttype[i] != MBV_GISPART_OUTER && parttype[i] != MBV_GISPART_HOLE) {
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_PARAMETER;
		}
	}

	/* count the rings that must be closed */
	int nclose = 0;
	for (int i = 0; status == MB_SUCCESS && i < npart; i++) {
		const int start = partstart[i];
		const int end = i < npart - 1 ? partstart[i + 1] : npoint;
		if (lon[start] != lon[end - 1] || lat[start] != lat[end - 1])
			nclose++;
	}
	const int npointuse = npoint + nclose;

	/* allocate memory for a new polygon if required */
	if (status == MB_SUCCESS && shared.shareddata.npolygon_alloc < shared.shareddata.npolygon + 1) {
		const int nalloc = shared.shareddata.npolygon_alloc + MBV_ALLOC_NUM;
		status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc * sizeof(struct mbview_polygon_struct),
		                     (void **)&(shared.shareddata.polygons), error);
		if (status == MB_SUCCESS) {
			memset(&(shared.shareddata.polygons[shared.shareddata.npolygon_alloc]), 0,
			       (nalloc - shared.shareddata.npolygon_alloc) * sizeof(struct mbview_polygon_struct));
			shared.shareddata.npolygon_alloc = nalloc;
		}
		else {
			shared.shareddata.npolygon = 0;
			shared.shareddata.npolygon_alloc = 0;
		}
	}

	/* allocate the polygon arrays */
	struct mbview_polygon_struct *polygon = NULL;
	if (status == MB_SUCCESS) {
		polygon = &(shared.shareddata.polygons[shared.shareddata.npolygon]);
		memset(polygon, 0, sizeof(struct mbview_polygon_struct));
		status = mb_mallocd(verbose, __FILE__, __LINE__, npart * sizeof(int), (void **)&(polygon->partstart), error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, npart * sizeof(int), (void **)&(polygon->parttype), error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, npointuse * sizeof(double), (void **)&(polygon->zsource), error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, npointuse * sizeof(struct mbview_pointw_struct),
			                    (void **)&(polygon->points), error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, npointuse * sizeof(struct mbview_linesegmentw_struct),
			                    (void **)&(polygon->segments), error);
		if (status == MB_SUCCESS) {
			memset(polygon->points, 0, npointuse * sizeof(struct mbview_pointw_struct));
			memset(polygon->segments, 0, npointuse * sizeof(struct mbview_linesegmentw_struct));
		}
		else {
			int error_free = MB_ERROR_NO_ERROR;
			mb_freed(verbose, __FILE__, __LINE__, (void **)&(polygon->partstart), &error_free);
			mb_freed(verbose, __FILE__, __LINE__, (void **)&(polygon->parttype), &error_free);
			mb_freed(verbose, __FILE__, __LINE__, (void **)&(polygon->zsource), &error_free);
			mb_freed(verbose, __FILE__, __LINE__, (void **)&(polygon->points), &error_free);
			mb_freed(verbose, __FILE__, __LINE__, (void **)&(polygon->segments), &error_free);
		}
	}

	/* add the polygon, closing rings as needed */
	if (status == MB_SUCCESS) {
		polygon->active = true;
		polygon->color = color;
		polygon->size = size;
		polygon->fillcolor = fillcolor;
		polygon->layer = layer;
		snprintf(polygon->name, sizeof(mb_path), "%s", name != NULL ? name : "");
		polygon->npart = npart;
		int k = 0;
		for (int i = 0; i < npart; i++) {
			const int start = partstart[i];
			const int end = i < npart - 1 ? partstart[i + 1] : npoint;
			polygon->partstart[i] = k;
			polygon->parttype[i] = parttype != NULL ? parttype[i] : MBV_GISPART_OUTER;
			for (int j = start; j < end; j++) {
				polygon->zsource[k] = z != NULL ? z[j] : 0.0;
				mbview_gis_setpointw(instance, lon[j], lat[j], &(polygon->points[k]));
				k++;
			}
			if (lon[start] != lon[end - 1] || lat[start] != lat[end - 1]) {
				polygon->zsource[k] = z != NULL ? z[start] : 0.0;
				polygon->points[k] = polygon->points[polygon->partstart[i]];
				k++;
			}
		}
		polygon->npoints = k;
		polygon->npoints_alloc = npointuse;
		mbview_gis_drapesegments(instance, npart, polygon->partstart, polygon->npoints, polygon->points, polygon->segments);
		*ipolygon = shared.shareddata.npolygon;
		shared.shareddata.npolygon++;

		/* make polygons viewable in this instance */
		mbviews[instance].data.polygon_view_mode = MBV_VIEW_ON;
		set_mbview_polygon_view_mode(instance, MBV_VIEW_ON);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ipolygon:                  %d\n", *ipolygon);
		fprintf(stderr, "dbg2       npolygon:                  %d\n", shared.shareddata.npolygon);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
/* Get a polygon; the arrays must hold the numbers of rings and points
    returned by mbview_getpolygonpointcount(). z returns the source z values. */
int mbview_getpolygon(int verbose, size_t instance, int ipolygon, int *npart, int *partstart, int *parttype, int *npoint,
                      double *lon, double *lat, double *z, int *color, int *size, int *fillcolor, mb_path name, int *layer,
                      int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       ipolygon:                  %d\n", ipolygon);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	if (ipolygon < 0 || ipolygon >= shared.shareddata.npolygon) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}
	else {
		const struct mbview_polygon_struct *polygon = &(shared.shareddata.polygons[ipolygon]);
		*npart = polygon->npart;
		memcpy(partstart, polygon->partstart, polygon->npart * sizeof(int));
		memcpy(parttype, polygon->parttype, polygon->npart * sizeof(int));
		*npoint = polygon->npoints;
		for (int j = 0; j < polygon->npoints; j++) {
			lon[j] = polygon->points[j].xlon;
			lat[j] = polygon->points[j].ylat;
			z[j] = polygon->zsource[j];
		}
		*color = polygon->color;
		*size = polygon->size;
		*fillcolor = polygon->fillcolor;
		snprintf(name, sizeof(mb_path), "%s", polygon->name);
		*layer = polygon->layer;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		if (status == MB_SUCCESS) {
			fprintf(stderr, "dbg2       npart:                     %d\n", *npart);
			fprintf(stderr, "dbg2       npoint:                    %d\n", *npoint);
			fprintf(stderr, "dbg2       color:                     %d\n", *color);
			fprintf(stderr, "dbg2       size:                      %d\n", *size);
			fprintf(stderr, "dbg2       fillcolor:                 %d\n", *fillcolor);
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
int mbview_deletepolygon(int verbose, size_t instance, int ipolygon, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       ipolygon:                  %d\n", ipolygon);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	if (ipolygon < 0 || ipolygon >= shared.shareddata.npolygon) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}
	else {
		/* free memory for the deleted polygon */
		struct mbview_polygon_struct *polygon = &(shared.shareddata.polygons[ipolygon]);
		int error_free = MB_ERROR_NO_ERROR;
		mbview_gis_freesegments(polygon->npoints, polygon->segments);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(polygon->partstart), &error_free);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(polygon->parttype), &error_free);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(polygon->zsource), &error_free);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(polygon->points), &error_free);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(polygon->segments), &error_free);

		/* move the following polygons down */
		for (int i = ipolygon; i < shared.shareddata.npolygon - 1; i++)
			shared.shareddata.polygons[i] = shared.shareddata.polygons[i + 1];
		shared.shareddata.npolygon--;
		memset(&(shared.shareddata.polygons[shared.shareddata.npolygon]), 0, sizeof(struct mbview_polygon_struct));

		/* keep the selection pointing at the same polygon */
		if (shared.shareddata.polygon_selected == ipolygon)
			shared.shareddata.polygon_selected = MBV_SELECT_NONE;
		else if (shared.shareddata.polygon_selected > ipolygon)
			shared.shareddata.polygon_selected--;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       npolygon:                  %d\n", shared.shareddata.npolygon);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_deleteallpolygons(int verbose, size_t instance, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	int status = MB_SUCCESS;
	while (status == MB_SUCCESS && shared.shareddata.npolygon > 0)
		status = mbview_deletepolygon(verbose, instance, shared.shareddata.npolygon - 1, error);

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
int mbview_enableviewpolygons(int verbose, size_t instance, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* set values */
	shared.shareddata.polygon_mode = MBV_POLYGON_VIEW;

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
int mbview_drawpolygon(size_t instance, int rez) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       rez:              %d\n", rez);
	}

	struct mbview_struct *data = &(mbviews[instance].data);

	/* draw the ring outlines; fill is not yet supported */
	if (shared.shareddata.polygon_mode != MBV_POLYGON_OFF && data->polygon_view_mode == MBV_VIEW_ON &&
	    shared.shareddata.npolygon > 0) {
		for (int i = 0; i < shared.shareddata.npolygon; i++) {
			struct mbview_polygon_struct *polygon = &(shared.shareddata.polygons[i]);
			if (polygon->active && shared.shareddata.gislayers[polygon->layer].active) {
				const int icolor = i == shared.shareddata.polygon_selected ? MBV_COLOR_RED : polygon->color;
				mbview_gis_drawlines(instance, polygon->npart, polygon->partstart, polygon->npoints, polygon->points,
				                     polygon->segments, icolor, polygon->size);
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
