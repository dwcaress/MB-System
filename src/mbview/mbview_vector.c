/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_vector.c	1/11/2012
 *    $Id$
 *
 *    Copyright (c) 2012-2013 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *------------------------------------------------------------------------------*/
/*
 *
 * Author:	D. W. Caress
 * Date:	October 28, 2003
 *
 * $Log: mbview_vector.c,v $
 *
 */
/*------------------------------------------------------------------------------*/

/* Standard includes for builtins. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* Motif required Headers */
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

/* OpenGL include files */
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include "mb_glwdrawa.h"

/* MBIO include files */
#include "mb_status.h"
#include "mb_define.h"

/* mbview include */
#include "mbview.h"
#include "mbviewprivate.h"

/*------------------------------------------------------------------------------*/

/* local variables */
static char rcs_id[]="$Id$";

/*------------------------------------------------------------------------------*/
int mbview_getvectorcount(int verbose, size_t instance,
			int *nvector,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_getvectorcount";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %zu\n", instance);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get number of vecs */
	*nvector = shared.shareddata.nvector;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nvector:                      %d\n", *nvector);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_getvectorpointcount(int verbose, size_t instance,
			int	vec,
			int	*npoint,
			int	*nintpoint,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_getvectorpointcount";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	i;

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %zu\n", instance);
		fprintf(stderr,"dbg2       vec:                     %d\n", vec);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get number of points in specified vec */
	*npoint = 0;
	*nintpoint = 0;
	if (vec >= 0 && vec < shared.shareddata.nvector)
		{
		*npoint = shared.shareddata.vectors[vec].npoints;
		for (i=0;i<*npoint-1;i++)
			{
			if (shared.shareddata.vectors[vec].segments[i].nls > 2)
				*nintpoint += shared.shareddata.vectors[vec].segments[i].nls - 2;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       npoint:                    %d\n", *npoint);
		fprintf(stderr,"dbg2       nintpoint:                 %d\n", *nintpoint);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);

}

/*------------------------------------------------------------------------------*/
int mbview_allocvectorarrays(int verbose,
			int	npointtotal,
			double	**veclon,
			double	**veclat,
			double	**vecz,
			double	**vecdata,
			int 	*error)
{
	/* local variables */
	char	*function_name = "mbview_allocvectorarrays";
	int	status = MB_SUCCESS;

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       npointtotal:               %d\n", npointtotal);
		fprintf(stderr,"dbg2       veclon:                    %p\n", *veclon);
		fprintf(stderr,"dbg2       veclat:                    %p\n", *veclat);
		fprintf(stderr,"dbg2       vecz:                      %p\n", *vecz);
		fprintf(stderr,"dbg2       vecdata:                   %p\n", *vecdata);
		}

	/* allocate the arrays using mb_realloc */
	status = mb_reallocd(verbose,__FILE__,__LINE__,npointtotal*sizeof(double),(void **)veclon,error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose,__FILE__,__LINE__,npointtotal*sizeof(double),(void **)veclat,error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose,__FILE__,__LINE__,npointtotal*sizeof(double),(void **)vecz,error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose,__FILE__,__LINE__,npointtotal*sizeof(double),(void **)vecdata,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       veclon:                    %p\n", *veclon);
		fprintf(stderr,"dbg2       veclat:                    %p\n", *veclat);
		fprintf(stderr,"dbg2       vecz:                      %p\n", *vecz);
		fprintf(stderr,"dbg2       vecdata:                   %p\n", *vecdata);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_freevectorarrays(int verbose,
			double	**veclon,
			double	**veclat,
			double	**vecz,
			double	**vecdata,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_freevectorarrays";
	int	status = MB_SUCCESS;

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       veclon:                    %p\n", *veclon);
		fprintf(stderr,"dbg2       veclat:                    %p\n", *veclat);
		fprintf(stderr,"dbg2       vecz:                      %p\n", *vecz);
		fprintf(stderr,"dbg2       vecdata:                   %p\n", *vecdata);
		}

	/* free the arrays using mb_free */
	status = mb_freed(verbose,__FILE__,__LINE__,(void **)veclon,error);
	status = mb_freed(verbose,__FILE__,__LINE__,(void **)veclat,error);
	status = mb_freed(verbose,__FILE__,__LINE__,(void **)vecz,error);
	status = mb_freed(verbose,__FILE__,__LINE__,(void **)vecdata,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       veclon:                    %p\n", *veclon);
		fprintf(stderr,"dbg2       veclat:                    %p\n", *veclat);
		fprintf(stderr,"dbg2       vecz:                      %p\n", *vecz);
		fprintf(stderr,"dbg2       vecdata:                   %p\n", *vecdata);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_addvector(int verbose, size_t instance,
			int	npoint,
			double	*veclon,
			double	*veclat,
			double	*vecz,
			double	*vecdata,
			int	veccolor,
			int	vecsize,
			mb_path	vecname,
			double	vecdatamin,
			double	vecdatamax,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_addvector";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	ivec;
	int	recalculate_minmax = MB_NO;
	int	i, j;

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %zu\n", instance);
		fprintf(stderr,"dbg2       npoint:                    %d\n", npoint);
		for (i=0;i<npoint;i++)
			{
			fprintf(stderr,"dbg2       point:%d lon:%f lat:%f z:%f data:%f\n",
					i, veclon[i], veclat[i], vecz[i], vecdata[i]);
			}
		fprintf(stderr,"dbg2       veccolor:                  %d\n", veccolor);
		fprintf(stderr,"dbg2       vecsize:                   %d\n", vecsize);
		fprintf(stderr,"dbg2       vecname:                   %s\n", vecname);
		fprintf(stderr,"dbg2       vecdatamin:                %f\n", vecdatamin);
		fprintf(stderr,"dbg2       vecdatamax:                %f\n", vecdatamax);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* make sure no vec is selected */
	shared.shareddata.vector_selected = MBV_SELECT_NONE;
	shared.shareddata.vector_point_selected = MBV_SELECT_NONE;

	/* set vec id so that new vec is created */
	ivec = shared.shareddata.nvector;

	/* allocate memory for a new vec if required */
	if (shared.shareddata.nvector_alloc < shared.shareddata.nvector + 1)
		{
		shared.shareddata.nvector_alloc = shared.shareddata.nvector + 1;
		status = mb_realloc(mbv_verbose,
			    	shared.shareddata.nvector_alloc * sizeof(struct mbview_vector_struct),
			    	(void **)&(shared.shareddata.vectors), error);
		if (status == MB_FAILURE)
			{
			shared.shareddata.nvector_alloc = 0;
			}
		else
			{
			for (i=shared.shareddata.nvector;i<shared.shareddata.nvector_alloc;i++)
				{
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
	if (shared.shareddata.vectors[ivec].npoints_alloc < npoint)
		{
		shared.shareddata.vectors[ivec].npoints_alloc = npoint;
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__,
			    	shared.shareddata.vectors[ivec].npoints_alloc * sizeof(struct mbview_vectorpointw_struct),
			    	(void **)&(shared.shareddata.vectors[ivec].vectorpts), error);
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__,
			    	shared.shareddata.vectors[ivec].npoints_alloc * sizeof(struct mbview_linesegmentw_struct),
			    	(void **)&(shared.shareddata.vectors[ivec].segments), error);
		for (j=0;j<shared.shareddata.vectors[ivec].npoints_alloc-1;j++)
			{
			shared.shareddata.vectors[ivec].segments[j].nls = 0;
			shared.shareddata.vectors[ivec].segments[j].nls_alloc = 0;
			shared.shareddata.vectors[ivec].segments[j].lspoints = NULL;
			shared.shareddata.vectors[ivec].segments[j].endpoints[0] = shared.shareddata.vectors[ivec].vectorpts[j].point;
			shared.shareddata.vectors[ivec].segments[j].endpoints[1] = shared.shareddata.vectors[ivec].vectorpts[j+1].point;
			}
		}

	/* add the new vec */
	if (status == MB_SUCCESS)
		{
		/* set nvector */
		shared.shareddata.nvector++;

		/* set color size and name for new vec */
		shared.shareddata.vectors[ivec].color = veccolor;
		shared.shareddata.vectors[ivec].size = vecsize;
		strcpy(shared.shareddata.vectors[ivec].name,vecname);
		shared.shareddata.vectors[ivec].datamin = vecdatamin;
		shared.shareddata.vectors[ivec].datamax = vecdatamax;
		if (vecdatamin == vecdatamax)
			recalculate_minmax = MB_YES;

		/* loop over the points in the new vec */
		shared.shareddata.vectors[ivec].npoints = npoint;
		for (i=0;i<npoint;i++)
			{
			/* set status values */
			shared.shareddata.vectors[ivec].vectorpts[i].selected = MB_NO;

			/* set data */
			shared.shareddata.vectors[ivec].vectorpts[i].data = vecdata[i];

			/* get min max of data if necessary */
			if (recalculate_minmax == MB_YES)
				{
				if (i == 0)
					{
					shared.shareddata.vectors[ivec].datamin = vecdata[i];
					shared.shareddata.vectors[ivec].datamax = vecdata[i];
					}
				else
					{
					shared.shareddata.vectors[ivec].datamin = MIN(vecdata[i], shared.shareddata.vectors[ivec].datamin);
					shared.shareddata.vectors[ivec].datamax = MAX(vecdata[i], shared.shareddata.vectors[ivec].datamax);
					}
				}

			/* ************************************************* */
			/* get vec positions in grid and display coordinates */
			shared.shareddata.vectors[ivec].vectorpts[i].point.xlon = veclon[i];
			shared.shareddata.vectors[ivec].vectorpts[i].point.ylat = veclat[i];
			shared.shareddata.vectors[ivec].vectorpts[i].point.zdata = vecz[i];
			status = mbview_projectfromlonlat(instance,
					shared.shareddata.vectors[ivec].vectorpts[i].point.xlon,
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
		fprintf(stderr,"Added %d point vector with data bounds: min:%f max:%f\n",
			shared.shareddata.vectors[ivec].npoints,
			shared.shareddata.vectors[ivec].datamin,
			shared.shareddata.vectors[ivec].datamax);
		}

	/* print vec debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  vec data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  vec values:\n");
		fprintf(stderr,"dbg2       vector_mode:        %d\n",shared.shareddata.vector_mode);
		fprintf(stderr,"dbg2       vector_view_mode:      %d\n",data->vector_view_mode);
		fprintf(stderr,"dbg2       nvector:               %d\n",shared.shareddata.nvector);
		fprintf(stderr,"dbg2       nvector_alloc:         %d\n",shared.shareddata.nvector_alloc);
		fprintf(stderr,"dbg2       vector_selected:       %d\n",shared.shareddata.vector_selected);
		fprintf(stderr,"dbg2       vector_point_selected: %d\n",shared.shareddata.vector_point_selected);
		for (i=0;i<shared.shareddata.nvector;i++)
			{
			fprintf(stderr,"dbg2       vec %d color:         %d\n",i,shared.shareddata.vectors[i].color);
			fprintf(stderr,"dbg2       vec %d size:          %d\n",i,shared.shareddata.vectors[i].size);
			fprintf(stderr,"dbg2       vec %d name:          %s\n",i,shared.shareddata.vectors[i].name);
			fprintf(stderr,"dbg2       vec %d npoints:       %d\n",i,shared.shareddata.vectors[i].npoints);
			fprintf(stderr,"dbg2       vec %d npoints_alloc: %d\n",i,shared.shareddata.vectors[i].npoints_alloc);
			fprintf(stderr,"dbg2       vec %d nselected:     %d\n",i,shared.shareddata.vectors[i].nselected);
			for (j=0;j<shared.shareddata.vectors[i].npoints;j++)
				{
				fprintf(stderr,"dbg2       vec %d %d selected: %d\n",i,j,shared.shareddata.vectors[i].vectorpts[j].selected);
				fprintf(stderr,"dbg2       vec %d %d data:     %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].data);

				fprintf(stderr,"dbg2       vec %d %d xgrid:    %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.xgrid[instance]);
				fprintf(stderr,"dbg2       vec %d %d ygrid:    %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.ygrid[instance]);
				fprintf(stderr,"dbg2       vec %d %d xlon:     %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.xlon);
				fprintf(stderr,"dbg2       vec %d %d ylat:     %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.ylat);
				fprintf(stderr,"dbg2       vec %d %d zdata:    %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.zdata);
				fprintf(stderr,"dbg2       vec %d %d xdisplay: %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.xdisplay[instance]);
				fprintf(stderr,"dbg2       vec %d %d ydisplay: %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.ydisplay[instance]);
				fprintf(stderr,"dbg2       vec %d %d zdisplay: %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.zdisplay[instance]);
				}
			for (j=0;j<shared.shareddata.vectors[i].npoints-1;j++)
				{
				fprintf(stderr,"dbg2       vec %d %d nls:          %d\n",i,j,shared.shareddata.vectors[i].segments[j].nls);
				fprintf(stderr,"dbg2       vec %d %d nls_alloc:    %d\n",i,j,shared.shareddata.vectors[i].segments[j].nls_alloc);
				fprintf(stderr,"dbg2       vec %d %d endpoints[0]: %p\n",i,j,&shared.shareddata.vectors[i].segments[j].endpoints[0]);
				fprintf(stderr,"dbg2       vec %d %d endpoints[1]: %p\n",i,j,&shared.shareddata.vectors[i].segments[j].endpoints[1]);
				}
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:                     %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n",status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_enableviewvectors(int verbose, size_t instance,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_enableviewvectors";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %zu\n", instance);
		}

	/* set values */
        shared.shareddata.vector_mode = MBV_VECTOR_VIEW;

	/* set widget sensitivity on all active instances */
	for (instance=0;instance<MBV_MAX_WINDOWS;instance++)
		{
		/* get view */
		view = &(mbviews[instance]);
		data = &(view->data);

		/* if instance active reset action sensitivity */
		if (data->active == MB_YES)
			mbview_update_sensitivity(verbose, instance, error);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:                     %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_pick_vector_select(size_t instance, int select, int which, int xpixel, int ypixel)
{

	/* local variables */
	char	*function_name = "mbview_pick_vector_select";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	found;
	double	xgrid, ygrid;
	double	xlon, ylat, zdata;
	double	xdisplay, ydisplay, zdisplay;
	double	xx, yy, rr, rrmin;
	int	i, j;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %zu\n",instance);
		fprintf(stderr,"dbg2       select:           %d\n",select);
		fprintf(stderr,"dbg2       which:            %d\n",which);
		fprintf(stderr,"dbg2       xpixel:           %d\n",xpixel);
		fprintf(stderr,"dbg2       ypixel:           %d\n",ypixel);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* only select one vector point if enabled and not in move mode */
	if (shared.shareddata.vector_mode != MBV_VECTOR_OFF
		&& shared.shareddata.nvector > 0
		&& (which == MBV_PICK_DOWN
			|| shared.shareddata.vector_selected == MBV_SELECT_NONE))
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel,
				&found,
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);

		/* look for nearest vec point */
		if (found)
			{
			rrmin = 1000000000.0;
			shared.shareddata.vector_selected = MBV_SELECT_NONE;
			shared.shareddata.vector_point_selected = MBV_SELECT_NONE;

			for (i=0;i<shared.shareddata.nvector;i++)
				{
				for (j=0;j<shared.shareddata.vectors[i].npoints;j++)
					{
					xx = xgrid - shared.shareddata.vectors[i].vectorpts[j].point.xgrid[instance];
					yy = ygrid - shared.shareddata.vectors[i].vectorpts[j].point.ygrid[instance];
					rr = sqrt(xx * xx + yy * yy);
					if (rr < rrmin)
						{
						rrmin = rr;
						shared.shareddata.vector_selected = i;
						shared.shareddata.vector_point_selected = j;
						}
					}
				}
			}
		else
			{
			/* unselect vec pick */
			data->pickinfo_mode = data->pick_type;
			shared.shareddata.vector_selected = MBV_SELECT_NONE;
			shared.shareddata.vector_point_selected = MBV_SELECT_NONE;
			XBell(view->dpy,100);
			}
		}

	/* else beep */
	else
		{
		shared.shareddata.vector_selected = MBV_SELECT_NONE;
		shared.shareddata.vector_point_selected = MBV_SELECT_NONE;
		XBell(view->dpy,100);
		for (i=0;i<shared.shareddata.nvector;i++)
			{
			for (j=0;j<shared.shareddata.vectors[i].npoints;j++)
				{
				shared.shareddata.vectors[i].vectorpts[j].selected = MB_NO;
				}
			}
		}

	/* set what kind of pick to annotate */
	if (shared.shareddata.vector_selected != MBV_SELECT_NONE)
		{
		data->pickinfo_mode = MBV_PICK_VECTOR;
		}
	else
		{
		data->pickinfo_mode = data->pick_type;
		}

	/* set pick annotation */
	mbview_pick_text(instance);

	/* call pick notify if defined */
	if (which == MBV_PICK_UP && shared.shareddata.vector_selected != MBV_SELECT_NONE
		&& data->mbview_pickvector_notify != NULL)
		{
		(data->mbview_pickvector_notify)(instance);
		}

	/* print vec debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  vec data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  vec values:\n");
		fprintf(stderr,"dbg2       vector_mode:           %d\n",shared.shareddata.vector_mode);
		fprintf(stderr,"dbg2       vector_view_mode:         %d\n",data->vector_view_mode);
		fprintf(stderr,"dbg2       nvector:                  %d\n",shared.shareddata.nvector);
		fprintf(stderr,"dbg2       nvector_alloc:            %d\n",shared.shareddata.nvector_alloc);
		fprintf(stderr,"dbg2       vector_selected:       %d\n",shared.shareddata.vector_selected);
		fprintf(stderr,"dbg2       vector_point_selected: %d\n",shared.shareddata.vector_point_selected);
		for (i=0;i<shared.shareddata.nvector;i++)
			{
			fprintf(stderr,"dbg2       vec %d color:         %d\n",i,shared.shareddata.vectors[i].color);
			fprintf(stderr,"dbg2       vec %d size:          %d\n",i,shared.shareddata.vectors[i].size);
			fprintf(stderr,"dbg2       vec %d name:          %s\n",i,shared.shareddata.vectors[i].name);
			fprintf(stderr,"dbg2       vec %d npoints:       %d\n",i,shared.shareddata.vectors[i].npoints);
			fprintf(stderr,"dbg2       vec %d npoints_alloc: %d\n",i,shared.shareddata.vectors[i].npoints_alloc);
			fprintf(stderr,"dbg2       vec %d nselected:     %d\n",i,shared.shareddata.vectors[i].nselected);
			for (j=0;j<shared.shareddata.vectors[i].npoints;j++)
				{
				fprintf(stderr,"dbg2       vec %d %d selected: %d\n",i,j,shared.shareddata.vectors[i].vectorpts[j].selected);
				fprintf(stderr,"dbg2       vec %d %d data:     %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].data);

				fprintf(stderr,"dbg2       vec %d %d xgrid:    %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.xgrid[instance]);
				fprintf(stderr,"dbg2       vec %d %d ygrid:    %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.ygrid[instance]);
				fprintf(stderr,"dbg2       vec %d %d xlon:     %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.xlon);
				fprintf(stderr,"dbg2       vec %d %d ylat:     %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.ylat);
				fprintf(stderr,"dbg2       vec %d %d zdata:    %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.zdata);
				fprintf(stderr,"dbg2       vec %d %d xdisplay: %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.xdisplay[instance]);
				fprintf(stderr,"dbg2       vec %d %d ydisplay: %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.ydisplay[instance]);
				fprintf(stderr,"dbg2       vec %d %d zdisplay: %f\n",i,j,shared.shareddata.vectors[i].vectorpts[j].point.zdisplay[instance]);

				}
			for (j=0;j<shared.shareddata.vectors[i].npoints-1;j++)
				{
				fprintf(stderr,"dbg2       vec %d %d nls:          %d\n",i,j,shared.shareddata.vectors[i].segments[j].nls);
				fprintf(stderr,"dbg2       vec %d %d nls_alloc:    %d\n",i,j,shared.shareddata.vectors[i].segments[j].nls_alloc);
				fprintf(stderr,"dbg2       vec %d %d endpoints[0]: %p\n",i,j,&shared.shareddata.vectors[i].segments[j].endpoints[0]);
				fprintf(stderr,"dbg2       vec %d %d endpoints[1]: %p\n",i,j,&shared.shareddata.vectors[i].segments[j].endpoints[1]);
				}
			}
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_vector_delete(size_t instance, int ivec)
{

	/* local variables */
	char	*function_name = "mbview_vector_delete";
	int	error = MB_ERROR_NO_ERROR;
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	i;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       ivec:            %d\n",ivec);
		fprintf(stderr,"dbg2       instance:         %zu\n",instance);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* delete vec if its the same as previously selected */
	if (ivec >= 0 && ivec < shared.shareddata.nvector)
		{
		/* free memory for deleted vec */
		mb_freed(mbv_verbose,__FILE__,__LINE__,
				(void **)&(shared.shareddata.vectors[ivec].vectorpts), &error);
		mb_freed(mbv_verbose,__FILE__,__LINE__,
				(void **)&(shared.shareddata.vectors[ivec].segments), &error);

		/* move vec data if necessary */
		for (i=ivec;i<shared.shareddata.nvector-1;i++)
			{
			shared.shareddata.vectors[i] = shared.shareddata.vectors[i+1];
			}

		/* rest last vec */
		shared.shareddata.vectors[shared.shareddata.nvector-1].color = MBV_COLOR_RED;
		shared.shareddata.vectors[shared.shareddata.nvector-1].size = 4;
		shared.shareddata.vectors[shared.shareddata.nvector-1].name[0] = '\0';
		shared.shareddata.vectors[shared.shareddata.nvector-1].npoints = 0;
		shared.shareddata.vectors[shared.shareddata.nvector-1].npoints_alloc = 0;
		shared.shareddata.vectors[shared.shareddata.nvector-1].vectorpts = NULL;
		shared.shareddata.vectors[shared.shareddata.nvector-1].segments = NULL;

		/* set nvector */
		shared.shareddata.nvector--;

		/* no selection */
		shared.shareddata.vector_selected = MBV_SELECT_NONE;
		}
	else
		{
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_drawvector(size_t instance, int rez)
{
	/* local variables */
	char	*function_name = "mbview_drawvector";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	GLUquadricObj *globj;
	int	stride;
	int	icolor;
	int	ivec, jpoint;
	float	red, green, blue;
	double	xx, yy;
	double	ballsize;
	int	k0, k1;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %zu\n",instance);
		fprintf(stderr,"dbg2       rez:              %d\n",rez);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set decimation */
	if (rez == MBV_REZ_FULL)
	    stride = 1;
	else if (rez == MBV_REZ_HIGH)
	    stride = data->hirez_navdecimate;
	else
	    stride = data->lorez_navdecimate;

	/* draw vectors */
	if (shared.shareddata.vector_mode != MBV_VECTOR_OFF
		&& data->vector_view_mode == MBV_VIEW_ON
		&& shared.shareddata.nvector > 0)
		{
		/* get size according to viewbounds */
		k0 = data->viewbounds[0] * data->primary_ny + data->viewbounds[2];
		k1 = data->viewbounds[1] * data->primary_ny + data->viewbounds[3];
		xx = data->primary_x[k1] - data->primary_x[k0];
		yy = data->primary_y[k1] - data->primary_y[k0];
		ballsize = 0.001 * sqrt(xx * xx + yy * yy);

		/* make list for ball */
	    	glNewList((GLuint)MBV_GLLIST_VECTORBALL, GL_COMPILE);
		globj = gluNewQuadric();
		gluSphere(globj, ballsize, 10, 10);
		gluDeleteQuadric(globj);
		glEndList();

		/* loop over the vecs plotting xyz vectors */
		for (ivec=0;ivec<shared.shareddata.nvector;ivec++)
			{
			icolor = shared.shareddata.vectors[ivec].color;

			/* plot lines */
			/* glLineWidth((float)(shared.shareddata.vectors[ivec].size));
			glBegin(GL_LINE_STRIP); */

			/* plot balls */
			for (jpoint=0;jpoint<shared.shareddata.vectors[ivec].npoints;jpoint+=stride)
				{
				/* set color */
				mbview_getcolor(shared.shareddata.vectors[ivec].vectorpts[jpoint].data,
						shared.shareddata.vectors[ivec].datamax,
						shared.shareddata.vectors[ivec].datamin,
						MBV_COLORTABLE_NORMAL,
	    				    (float) 0.0, (float) 0.0, (float) 1.0,
	    				    (float) 0.0, (float) 0.0, (float) 0.0,
					    colortable_bright_red,
					    colortable_bright_green,
					    colortable_bright_blue,
					    &red, &green, &blue);
				if (shared.shareddata.vectors[ivec].vectorpts[jpoint].selected == MB_YES
					|| (jpoint < shared.shareddata.vectors[ivec].npoints - 1
						&& shared.shareddata.vectors[ivec].vectorpts[jpoint+1].selected == MB_YES))
					{
					glColor3f(colortable_object_red[MBV_COLOR_RED],
						colortable_object_green[MBV_COLOR_RED],
						colortable_object_blue[MBV_COLOR_RED]);
					}
				else
					{
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

#ifdef MBV_GETERRORS
mbview_glerrorcheck(instance, 1, function_name);
#endif

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}
/*------------------------------------------------------------------------------*/
