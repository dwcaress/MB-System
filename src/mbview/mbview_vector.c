/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_vector.c	1/11/2012
 *
 *    Copyright (c) 2012-2023 by
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
 * Author:	D. W. Caress
 * Date:	October 28, 2003
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
int mbview_getvectorcount(int verbose, size_t instance, int *nvector, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* get number of vecs */
	*nvector = shared.shareddata.nvector;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nvector:                      %d\n", *nvector);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_getvectorpointcount(int verbose, size_t instance, int vec, int *npoint, int *nintpoint, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       vec:                     %d\n", vec);
	}

	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* get number of points in specified vec */
	*npoint = 0;
	*nintpoint = 0;
	if (vec >= 0 && vec < shared.shareddata.nvector) {
		*npoint = shared.shareddata.vectors[vec].npoints;
		for (int i = 0; i < *npoint - 1; i++) {
			if (shared.shareddata.vectors[vec].segments[i].nls > 2)
				*nintpoint += shared.shareddata.vectors[vec].segments[i].nls - 2;
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       npoint:                    %d\n", *npoint);
		fprintf(stderr, "dbg2       nintpoint:                 %d\n", *nintpoint);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_allocvectorarrays(int verbose, int npointtotal, double **veclon, double **veclat, double **vecz, double **vecdata,
                             int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       npointtotal:               %d\n", npointtotal);
		fprintf(stderr, "dbg2       veclon:                    %p\n", *veclon);
		fprintf(stderr, "dbg2       veclat:                    %p\n", *veclat);
		fprintf(stderr, "dbg2       vecz:                      %p\n", *vecz);
		fprintf(stderr, "dbg2       vecdata:                   %p\n", *vecdata);
	}

	/* allocate the arrays using mb_reallocd */
	int status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)veclon, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)veclat, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)vecz, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)vecdata, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       veclon:                    %p\n", *veclon);
		fprintf(stderr, "dbg2       veclat:                    %p\n", *veclat);
		fprintf(stderr, "dbg2       vecz:                      %p\n", *vecz);
		fprintf(stderr, "dbg2       vecdata:                   %p\n", *vecdata);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_freevectorarrays(int verbose, double **veclon, double **veclat, double **vecz, double **vecdata, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       veclon:                    %p\n", *veclon);
		fprintf(stderr, "dbg2       veclat:                    %p\n", *veclat);
		fprintf(stderr, "dbg2       vecz:                      %p\n", *vecz);
		fprintf(stderr, "dbg2       vecdata:                   %p\n", *vecdata);
	}

	/* free the arrays using mb_freed */
	int status = mb_freed(verbose, __FILE__, __LINE__, (void **)veclon, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)veclat, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)vecz, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)vecdata, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       veclon:                    %p\n", *veclon);
		fprintf(stderr, "dbg2       veclat:                    %p\n", *veclat);
		fprintf(stderr, "dbg2       vecz:                      %p\n", *vecz);
		fprintf(stderr, "dbg2       vecdata:                   %p\n", *vecdata);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_addvector(int verbose, size_t instance, int npoint, double *veclon, double *veclat, double *vecz, double *vecdata,
                     int veccolor, int vecsize, mb_path vecname, double vecdatamin, double vecdatamax, int *error) {
	if (verbose >= 0) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       npoint:                    %d\n", npoint);
		for (int i = 0; i < npoint; i++) {
			fprintf(stderr, "dbg2       point:%d lon:%f lat:%f z:%f data:%f\n", i, veclon[i], veclat[i], vecz[i], vecdata[i]);
		}
		fprintf(stderr, "dbg2       veccolor:                  %d\n", veccolor);
		fprintf(stderr, "dbg2       vecsize:                   %d\n", vecsize);
		fprintf(stderr, "dbg2       vecname:                   %s\n", vecname);
		fprintf(stderr, "dbg2       vecdatamin:                %f\n", vecdatamin);
		fprintf(stderr, "dbg2       vecdatamax:                %f\n", vecdatamax);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* make sure no vec is selected */
	shared.shareddata.vector_selected = MBV_SELECT_NONE;
	shared.shareddata.vector_point_selected = MBV_SELECT_NONE;

	/* set vec id so that new vec is created */
	const int ivec = shared.shareddata.nvector;

	int status = MB_SUCCESS;

	/* allocate memory for a new vec if required */
	if (shared.shareddata.nvector_alloc < shared.shareddata.nvector + 1) {
		shared.shareddata.nvector_alloc = shared.shareddata.nvector + 1;
		status =
		    mb_reallocd(mbv_verbose, __FILE__, __LINE__, shared.shareddata.nvector_alloc * sizeof(struct mbview_vector_struct),
		                (void **)&(shared.shareddata.vectors), error);
		if (status == MB_FAILURE) {
			shared.shareddata.nvector_alloc = 0;
		} else {
			for (int i = shared.shareddata.nvector; i < shared.shareddata.nvector_alloc; i++) {
				shared.shareddata.vectors[i].active = false;
				shared.shareddata.vectors[i].color = MBV_COLOR_RED;
				shared.shareddata.vectors[i].size = 4;
				shared.shareddata.vectors[i].name[0] = '\0';
				shared.shareddata.vectors[i].npoints = 0;
				shared.shareddata.vectors[i].npoints_alloc = 0;
				shared.shareddata.vectors[i].nselected = 0;
				shared.shareddata.vectors[i].vectorpts = NULL;
				shared.shareddata.vectors[i].segments = NULL;
			}
		}
	}

	/* allocate memory to for vec arrays */
	if (shared.shareddata.vectors[ivec].npoints_alloc < npoint) {
		shared.shareddata.vectors[ivec].npoints_alloc = npoint;
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__,
		                     shared.shareddata.vectors[ivec].npoints_alloc * sizeof(struct mbview_vectorpointw_struct),
		                     (void **)&(shared.shareddata.vectors[ivec].vectorpts), error);
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__,
		                     shared.shareddata.vectors[ivec].npoints_alloc * sizeof(struct mbview_linesegmentw_struct),
		                     (void **)&(shared.shareddata.vectors[ivec].segments), error);
		for (int j = 0; j < shared.shareddata.vectors[ivec].npoints_alloc - 1; j++) {
			shared.shareddata.vectors[ivec].segments[j].nls = 0;
			shared.shareddata.vectors[ivec].segments[j].nls_alloc = 0;
			shared.shareddata.vectors[ivec].segments[j].lspoints = NULL;
			shared.shareddata.vectors[ivec].segments[j].endpoints[0] = shared.shareddata.vectors[ivec].vectorpts[j].point;
			shared.shareddata.vectors[ivec].segments[j].endpoints[1] = shared.shareddata.vectors[ivec].vectorpts[j + 1].point;
		}
	}

	/* add the new vec */
	if (status == MB_SUCCESS) {
		/* set nvector */
		shared.shareddata.nvector++;

		/* set color size and name for new vec */
		shared.shareddata.vectors[ivec].active = true;
		shared.shareddata.vectors[ivec].color = veccolor;
		shared.shareddata.vectors[ivec].size = vecsize;
		strcpy(shared.shareddata.vectors[ivec].name, vecname);
		shared.shareddata.vectors[ivec].datamin = vecdatamin;
		shared.shareddata.vectors[ivec].datamax = vecdatamax;
		const bool recalculate_minmax = vecdatamin == vecdatamax;

		/* loop over the points in the new vec */
		shared.shareddata.vectors[ivec].npoints = npoint;
		for (int i = 0; i < npoint; i++) {
			/* set status values */
			shared.shareddata.vectors[ivec].vectorpts[i].selected = false;

			/* set data */
			shared.shareddata.vectors[ivec].vectorpts[i].data = vecdata[i];

			/* get min max of data if necessary */
			if (recalculate_minmax) {
				if (i == 0) {
					shared.shareddata.vectors[ivec].datamin = vecdata[i];
					shared.shareddata.vectors[ivec].datamax = vecdata[i];
				}
				else {
					shared.shareddata.vectors[ivec].datamin = MIN(vecdata[i], shared.shareddata.vectors[ivec].datamin);
					shared.shareddata.vectors[ivec].datamax = MAX(vecdata[i], shared.shareddata.vectors[ivec].datamax);
				}
			}

			/* ************************************************* */
			/* get vec positions in grid and display coordinates */
			shared.shareddata.vectors[ivec].vectorpts[i].point.xlon = veclon[i];
			shared.shareddata.vectors[ivec].vectorpts[i].point.ylat = veclat[i];
			shared.shareddata.vectors[ivec].vectorpts[i].point.zdata = vecz[i];
			status = mbview_projectfromlonlat(instance, shared.shareddata.vectors[ivec].vectorpts[i].point.xlon,
			                                  shared.shareddata.vectors[ivec].vectorpts[i].point.ylat,
			                                  shared.shareddata.vectors[ivec].vectorpts[i].point.zdata,
			                                  &(shared.shareddata.vectors[ivec].vectorpts[i].point.xgrid[instance]),
			                                  &(shared.shareddata.vectors[ivec].vectorpts[i].point.ygrid[instance]),
			                                  &(shared.shareddata.vectors[ivec].vectorpts[i].point.xdisplay[instance]),
			                                  &(shared.shareddata.vectors[ivec].vectorpts[i].point.ydisplay[instance]),
			                                  &(shared.shareddata.vectors[ivec].vectorpts[i].point.zdisplay[instance]));
			mbview_updatepointw(instance, &(shared.shareddata.vectors[ivec].vectorpts[i].point));

			/*fprintf(stderr,"Depth: llz:%f %f %f   grid:%f %f   dpy:%f %f %f\n",
			shared.shareddata.vectors[ivec].vectorpts[i].point.xlon,
			shared.shareddata.vectors[ivec].vectorpts[i].point.ylat,
			shared.shareddata.vectors[ivec].vectorpts[i].point.zdata,
			shared.shareddata.vectors[ivec].vectorpts[i].point.xgrid[instance],
			shared.shareddata.vectors[ivec].vectorpts[i].point.ygrid[instance],
			shared.shareddata.vectors[ivec].vectorpts[i].point.xdisplay[instance],
			shared.shareddata.vectors[ivec].vectorpts[i].point.ydisplay[instance],
			shared.shareddata.vectors[ivec].vectorpts[i].point.zdisplay[instance]);*/

			/* ************************************************* */
		}

		/* make vecs viewable */
		data->vector_view_mode = MBV_VIEW_ON;

		/* some info to terminal */
		fprintf(stderr, "Added %d point vector with data bounds: min:%f max:%f\n", shared.shareddata.vectors[ivec].npoints,
		        shared.shareddata.vectors[ivec].datamin, shared.shareddata.vectors[ivec].datamax);
	}

	/* print vec debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  vec data altered in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  vec values:\n");
		fprintf(stderr, "dbg2       vector_mode:        %d\n", shared.shareddata.vector_mode);
		fprintf(stderr, "dbg2       vector_view_mode:      %d\n", data->vector_view_mode);
		fprintf(stderr, "dbg2       nvector:               %d\n", shared.shareddata.nvector);
		fprintf(stderr, "dbg2       nvector_alloc:         %d\n", shared.shareddata.nvector_alloc);
		fprintf(stderr, "dbg2       vector_selected:       %d\n", shared.shareddata.vector_selected);
		fprintf(stderr, "dbg2       vector_point_selected: %d\n", shared.shareddata.vector_point_selected);
		for (int i = 0; i < shared.shareddata.nvector; i++) {
			fprintf(stderr, "dbg2       vec %d active:        %d\n", i, shared.shareddata.vectors[i].active);
			fprintf(stderr, "dbg2       vec %d color:         %d\n", i, shared.shareddata.vectors[i].color);
			fprintf(stderr, "dbg2       vec %d size:          %d\n", i, shared.shareddata.vectors[i].size);
			fprintf(stderr, "dbg2       vec %d name:          %s\n", i, shared.shareddata.vectors[i].name);
			fprintf(stderr, "dbg2       vec %d npoints:       %d\n", i, shared.shareddata.vectors[i].npoints);
			fprintf(stderr, "dbg2       vec %d npoints_alloc: %d\n", i, shared.shareddata.vectors[i].npoints_alloc);
			fprintf(stderr, "dbg2       vec %d nselected:     %d\n", i, shared.shareddata.vectors[i].nselected);
			for (int j = 0; j < shared.shareddata.vectors[i].npoints; j++) {
				fprintf(stderr, "dbg2       vec %d %d selected: %d\n", i, j, shared.shareddata.vectors[i].vectorpts[j].selected);
				fprintf(stderr, "dbg2       vec %d %d data:     %f\n", i, j, shared.shareddata.vectors[i].vectorpts[j].data);

				fprintf(stderr, "dbg2       vec %d %d xgrid:    %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.xgrid[instance]);
				fprintf(stderr, "dbg2       vec %d %d ygrid:    %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.ygrid[instance]);
				fprintf(stderr, "dbg2       vec %d %d xlon:     %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.xlon);
				fprintf(stderr, "dbg2       vec %d %d ylat:     %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.ylat);
				fprintf(stderr, "dbg2       vec %d %d zdata:    %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.zdata);
				fprintf(stderr, "dbg2       vec %d %d xdisplay: %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.xdisplay[instance]);
				fprintf(stderr, "dbg2       vec %d %d ydisplay: %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.ydisplay[instance]);
				fprintf(stderr, "dbg2       vec %d %d zdisplay: %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.zdisplay[instance]);
			}
			for (int j = 0; j < shared.shareddata.vectors[i].npoints - 1; j++) {
				fprintf(stderr, "dbg2       vec %d %d nls:          %d\n", i, j, shared.shareddata.vectors[i].segments[j].nls);
				fprintf(stderr, "dbg2       vec %d %d nls_alloc:    %d\n", i, j,
				        shared.shareddata.vectors[i].segments[j].nls_alloc);
				fprintf(stderr, "dbg2       vec %d %d endpoints[0]: %p\n", i, j,
				        &shared.shareddata.vectors[i].segments[j].endpoints[0]);
				fprintf(stderr, "dbg2       vec %d %d endpoints[1]: %p\n", i, j,
				        &shared.shareddata.vectors[i].segments[j].endpoints[1]);
			}
		}
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
int mbview_enableviewvectors(int verbose, size_t instance, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* set values */
	shared.shareddata.vector_mode = MBV_VECTOR_VIEW;

	/* set widget sensitivity on all active instances */
	for (instance = 0; instance < MBV_MAX_WINDOWS; instance++) {
		struct mbview_world_struct *view = &(mbviews[instance]);
		struct mbview_struct *data = &(view->data);

		/* if instance active reset action sensitivity */
		if (data->active)
			mbview_update_sensitivity(verbose, instance, error);
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
int mbview_pick_vector_select(size_t instance, int select, int which, int xpixel, int ypixel) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       select:           %d\n", select);
		fprintf(stderr, "dbg2       which:            %d\n", which);
		fprintf(stderr, "dbg2       xpixel:           %d\n", xpixel);
		fprintf(stderr, "dbg2       ypixel:           %d\n", ypixel);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);


	/* only select one vector point if enabled and not in move mode */
	if (shared.shareddata.vector_mode != MBV_VECTOR_OFF && shared.shareddata.nvector > 0 &&
	    (which == MBV_PICK_DOWN || shared.shareddata.vector_selected == MBV_SELECT_NONE)) {
		bool found;
		double xgrid;
		double ygrid;
		double xlon;
		double ylat;
		double zdata;
		double xdisplay;
		double ydisplay;
		double zdisplay;
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		/* look for nearest vec point */
		if (found) {
			double rrmin = 1000000000.0;
			shared.shareddata.vector_selected = MBV_SELECT_NONE;
			shared.shareddata.vector_point_selected = MBV_SELECT_NONE;

			for (int i = 0; i < shared.shareddata.nvector; i++) {
        if (shared.shareddata.vectors[i].active) {
  				for (int j = 0; j < shared.shareddata.vectors[i].npoints; j++) {
  					const double xx = xgrid - shared.shareddata.vectors[i].vectorpts[j].point.xgrid[instance];
  					const double yy = ygrid - shared.shareddata.vectors[i].vectorpts[j].point.ygrid[instance];
  					const double rr = sqrt(xx * xx + yy * yy);
  					if (rr < rrmin) {
  						rrmin = rr;
  						shared.shareddata.vector_selected = i;
  						shared.shareddata.vector_point_selected = j;
  					}
  				}
        }
			}
		}
		else {
			/* unselect vec pick */
			data->pickinfo_mode = data->pick_type;
			shared.shareddata.vector_selected = MBV_SELECT_NONE;
			shared.shareddata.vector_point_selected = MBV_SELECT_NONE;
			XBell(view->dpy, 100);
		}
	}

	/* else beep */
	else {
		shared.shareddata.vector_selected = MBV_SELECT_NONE;
		shared.shareddata.vector_point_selected = MBV_SELECT_NONE;
		XBell(view->dpy, 100);
		for (int i = 0; i < shared.shareddata.nvector; i++) {
			for (int j = 0; j < shared.shareddata.vectors[i].npoints; j++) {
				shared.shareddata.vectors[i].vectorpts[j].selected = false;
			}
		}
	}

	/* set what kind of pick to annotate */
	if (shared.shareddata.vector_selected != MBV_SELECT_NONE) {
		data->pickinfo_mode = MBV_PICK_VECTOR;
	}
	else {
		data->pickinfo_mode = data->pick_type;
	}

	/* set pick annotation */
	mbview_pick_text(instance);

	/* call pick notify if defined */
	if (which == MBV_PICK_UP && shared.shareddata.vector_selected != MBV_SELECT_NONE && data->mbview_pickvector_notify != NULL) {
		(data->mbview_pickvector_notify)(instance);
	}

	/* print vec debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  vec data altered in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  vec values:\n");
		fprintf(stderr, "dbg2       vector_mode:           %d\n", shared.shareddata.vector_mode);
		fprintf(stderr, "dbg2       vector_view_mode:         %d\n", data->vector_view_mode);
		fprintf(stderr, "dbg2       nvector:                  %d\n", shared.shareddata.nvector);
		fprintf(stderr, "dbg2       nvector_alloc:            %d\n", shared.shareddata.nvector_alloc);
		fprintf(stderr, "dbg2       vector_selected:       %d\n", shared.shareddata.vector_selected);
		fprintf(stderr, "dbg2       vector_point_selected: %d\n", shared.shareddata.vector_point_selected);
		for (int i = 0; i < shared.shareddata.nvector; i++) {
			fprintf(stderr, "dbg2       vec %d active:       %d\n", i, shared.shareddata.vectors[i].active);
			fprintf(stderr, "dbg2       vec %d color:         %d\n", i, shared.shareddata.vectors[i].color);
			fprintf(stderr, "dbg2       vec %d size:          %d\n", i, shared.shareddata.vectors[i].size);
			fprintf(stderr, "dbg2       vec %d name:          %s\n", i, shared.shareddata.vectors[i].name);
			fprintf(stderr, "dbg2       vec %d npoints:       %d\n", i, shared.shareddata.vectors[i].npoints);
			fprintf(stderr, "dbg2       vec %d npoints_alloc: %d\n", i, shared.shareddata.vectors[i].npoints_alloc);
			fprintf(stderr, "dbg2       vec %d nselected:     %d\n", i, shared.shareddata.vectors[i].nselected);
			for (int j = 0; j < shared.shareddata.vectors[i].npoints; j++) {
				fprintf(stderr, "dbg2       vec %d %d selected: %d\n", i, j, shared.shareddata.vectors[i].vectorpts[j].selected);
				fprintf(stderr, "dbg2       vec %d %d data:     %f\n", i, j, shared.shareddata.vectors[i].vectorpts[j].data);

				fprintf(stderr, "dbg2       vec %d %d xgrid:    %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.xgrid[instance]);
				fprintf(stderr, "dbg2       vec %d %d ygrid:    %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.ygrid[instance]);
				fprintf(stderr, "dbg2       vec %d %d xlon:     %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.xlon);
				fprintf(stderr, "dbg2       vec %d %d ylat:     %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.ylat);
				fprintf(stderr, "dbg2       vec %d %d zdata:    %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.zdata);
				fprintf(stderr, "dbg2       vec %d %d xdisplay: %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.xdisplay[instance]);
				fprintf(stderr, "dbg2       vec %d %d ydisplay: %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.ydisplay[instance]);
				fprintf(stderr, "dbg2       vec %d %d zdisplay: %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.zdisplay[instance]);
			}
			for (int j = 0; j < shared.shareddata.vectors[i].npoints - 1; j++) {
				fprintf(stderr, "dbg2       vec %d %d nls:          %d\n", i, j, shared.shareddata.vectors[i].segments[j].nls);
				fprintf(stderr, "dbg2       vec %d %d nls_alloc:    %d\n", i, j,
				        shared.shareddata.vectors[i].segments[j].nls_alloc);
				fprintf(stderr, "dbg2       vec %d %d endpoints[0]: %p\n", i, j,
				        &shared.shareddata.vectors[i].segments[j].endpoints[0]);
				fprintf(stderr, "dbg2       vec %d %d endpoints[1]: %p\n", i, j,
				        &shared.shareddata.vectors[i].segments[j].endpoints[1]);
			}
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_vector_delete(size_t instance, int ivec) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       ivec:            %d\n", ivec);
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* delete vec if its the same as previously selected */
	if (ivec >= 0 && ivec < shared.shareddata.nvector) {
		/* free memory for deleted vec */
		int error = MB_ERROR_NO_ERROR;
		mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.vectors[ivec].vectorpts), &error);
		mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.vectors[ivec].segments), &error);

		/* move vec data if necessary */
		for (int i = ivec; i < shared.shareddata.nvector - 1; i++) {
			shared.shareddata.vectors[i] = shared.shareddata.vectors[i + 1];
		}

		/* rest last vec */
		shared.shareddata.vectors[shared.shareddata.nvector - 1].active = false;
		shared.shareddata.vectors[shared.shareddata.nvector - 1].color = MBV_COLOR_RED;
		shared.shareddata.vectors[shared.shareddata.nvector - 1].size = 4;
		shared.shareddata.vectors[shared.shareddata.nvector - 1].name[0] = '\0';
		shared.shareddata.vectors[shared.shareddata.nvector - 1].npoints = 0;
		shared.shareddata.vectors[shared.shareddata.nvector - 1].npoints_alloc = 0;
		shared.shareddata.vectors[shared.shareddata.nvector - 1].vectorpts = NULL;
		shared.shareddata.vectors[shared.shareddata.nvector - 1].segments = NULL;

		/* set nvector */
		shared.shareddata.nvector--;

		/* no selection */
		shared.shareddata.vector_selected = MBV_SELECT_NONE;
	} else {
		status = MB_FAILURE;
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_drawvector(size_t instance, int rez) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       rez:              %d\n", rez);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set decimation */
	int stride;
	if (rez == MBV_REZ_FULL)
		stride = 1;
	else if (rez == MBV_REZ_HIGH)
		stride = data->hirez_navdecimate;
	else
		stride = data->lorez_navdecimate;


	if (shared.shareddata.vector_mode != MBV_VECTOR_OFF && data->vector_view_mode == MBV_VIEW_ON &&
	    shared.shareddata.nvector > 0) {
		/* get size according to viewbounds */
		const int k0 = data->viewbounds[0] * data->primary_n_rows + data->viewbounds[2];
		const int k1 = data->viewbounds[1] * data->primary_n_rows + data->viewbounds[3];
		const double xx = data->primary_x[k1] - data->primary_x[k0];
		const double yy = data->primary_y[k1] - data->primary_y[k0];
		const double ballsize = 0.001 * sqrt(xx * xx + yy * yy);

		/* make list for ball */
		glNewList((GLuint)MBV_GLLIST_VECTORBALL, GL_COMPILE);
		GLUquadricObj *globj = gluNewQuadric();
		gluSphere(globj, ballsize, 10, 10);
		gluDeleteQuadric(globj);
		glEndList();

		/* loop over the vecs plotting xyz vectors */
		for (int ivec = 0; ivec < shared.shareddata.nvector; ivec++) {
      if (shared.shareddata.vectors[ivec].active) {
  			// const int icolor = shared.shareddata.vectors[ivec].color;

  			/* plot lines */
  			/* glLineWidth((float)(shared.shareddata.vectors[ivec].size));
  			glBegin(GL_LINE_STRIP); */

  			/* plot balls */
  			for (int jpoint = 0; jpoint < shared.shareddata.vectors[ivec].npoints; jpoint += stride) {
  				/* set color */
  				float red;
  				float green;
  				float blue;
  				mbview_getcolor(shared.shareddata.vectors[ivec].vectorpts[jpoint].data, shared.shareddata.vectors[ivec].datamin,
  				                shared.shareddata.vectors[ivec].datamax, MBV_COLORTABLE_NORMAL, (float)0.0, (float)0.0,
  				                (float)1.0, (float)0.0, (float)0.0, (float)0.0, colortable_bright_red, colortable_bright_green,
  				                colortable_bright_blue, &red, &green, &blue);
  				if (shared.shareddata.vectors[ivec].vectorpts[jpoint].selected ||
  				    (jpoint < shared.shareddata.vectors[ivec].npoints - 1 &&
  				     shared.shareddata.vectors[ivec].vectorpts[jpoint + 1].selected)) {
  					glColor3f(colortable_object_red[MBV_COLOR_RED], colortable_object_green[MBV_COLOR_RED],
  					          colortable_object_blue[MBV_COLOR_RED]);
  				}
  				else {
  					glColor3f(red, green, blue);
  				}

  				/* draw points in line */
  				/* glVertex3f((float)(shared.shareddata.vectors[ivec].vectorpts[jpoint].point.xdisplay[instance]),
  				        (float)(shared.shareddata.vectors[ivec].vectorpts[jpoint].point.ydisplay[instance]),
  				        (float)(shared.shareddata.vectors[ivec].vectorpts[jpoint].point.zdisplay[instance])); */

  				/* draw points as balls */
  				glTranslatef((float)(shared.shareddata.vectors[ivec].vectorpts[jpoint].point.xdisplay[instance]),
  				             (float)(shared.shareddata.vectors[ivec].vectorpts[jpoint].point.ydisplay[instance]),
  				             (float)(shared.shareddata.vectors[ivec].vectorpts[jpoint].point.zdisplay[instance]));
  				glCallList((GLuint)MBV_GLLIST_VECTORBALL);
  				glTranslatef((float)(-shared.shareddata.vectors[ivec].vectorpts[jpoint].point.xdisplay[instance]),
  				             (float)(-shared.shareddata.vectors[ivec].vectorpts[jpoint].point.ydisplay[instance]),
  				             (float)(-shared.shareddata.vectors[ivec].vectorpts[jpoint].point.zdisplay[instance]));
  			}
  			/* glEnd();*/
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
