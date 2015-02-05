/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_primary.c	9/25/2003
 *    $Id$
 *
 *    Copyright (c) 2003-2015 by
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
 * Date:	September 25, 2003
 *
 * Note:	This code was broken out of mbview_callbacks.c, which was
 *		begun on October 7, 2002
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
int mbview_setprimarygrid(int verbose, size_t instance,
			int	primary_grid_projection_mode,
			char	*primary_grid_projection_id,
			float	primary_nodatavalue,
			int	primary_nx,
			int	primary_ny,
			double	primary_min,
			double	primary_max,
			double	primary_xmin,
			double	primary_xmax,
			double	primary_ymin,
			double	primary_ymax,
			double	primary_dx,
			double	primary_dy,
			float	*primary_data,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_setprimarygrid";
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
		fprintf(stderr,"dbg2       verbose:                      %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                     %zu\n", instance);
		fprintf(stderr,"dbg2       primary_grid_projection_mode: %d\n", primary_grid_projection_mode);
		fprintf(stderr,"dbg2       primary_grid_projection_id:   %s\n", primary_grid_projection_id);
		fprintf(stderr,"dbg2       primary_nodatavalue:          %f\n", primary_nodatavalue);
		fprintf(stderr,"dbg2       primary_nx:                   %d\n", primary_nx);
		fprintf(stderr,"dbg2       primary_ny:                   %d\n", primary_ny);
		fprintf(stderr,"dbg2       primary_min:                  %f\n", primary_min);
		fprintf(stderr,"dbg2       primary_max:                  %f\n", primary_max);
		fprintf(stderr,"dbg2       primary_xmin:                 %f\n", primary_xmin);
		fprintf(stderr,"dbg2       primary_xmax:                 %f\n", primary_xmax);
		fprintf(stderr,"dbg2       primary_ymin:                 %f\n", primary_ymin);
		fprintf(stderr,"dbg2       primary_ymax:                 %f\n", primary_ymax);
		fprintf(stderr,"dbg2       primary_dx:                   %f\n", primary_dx);
		fprintf(stderr,"dbg2       primary_dy:                   %f\n", primary_dy);
		fprintf(stderr,"dbg2       primary_data:                 %p\n", primary_data);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set values */
        data->primary_grid_projection_mode = primary_grid_projection_mode;
        strcpy(data->primary_grid_projection_id, primary_grid_projection_id);
        data->primary_nodatavalue = primary_nodatavalue;
        data->primary_nxy = primary_nx * primary_ny;
        data->primary_nx = primary_nx;
        data->primary_ny = primary_ny;
        data->primary_min = primary_min;
        data->primary_max = primary_max;
        data->primary_xmin = primary_xmin;
        data->primary_xmax = primary_xmax;
        data->primary_ymin = primary_ymin;
        data->primary_ymax = primary_ymax;
        data->primary_dx = primary_dx;
        data->primary_dy = primary_dy;
	data->viewbounds[0] = 0;
	data->viewbounds[1] = data->primary_nx;
	data->viewbounds[2] = 0;
	data->viewbounds[3] = data->primary_ny;

	/* allocate required arrays */
    	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * data->primary_nxy,
    				(void **)&data->primary_data, error);
    	if (status == MB_SUCCESS)
	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * data->primary_nxy,
    				(void **)&data->primary_x, error);
    	if (status == MB_SUCCESS)
    	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * data->primary_nxy,
    				(void **)&data->primary_y, error);
    	if (status == MB_SUCCESS)
    	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * data->primary_nxy,
    				(void **)&data->primary_z, error);
    	if (status == MB_SUCCESS)
    	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * data->primary_nxy,
    				(void **)&data->primary_dzdx, error);
    	if (status == MB_SUCCESS)
    	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * data->primary_nxy,
    				(void **)&data->primary_dzdy, error);
    	if (status == MB_SUCCESS)
    	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * data->primary_nxy,
    				(void **)&data->primary_r, error);
    	if (status == MB_SUCCESS)
    	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * data->primary_nxy,
    				(void **)&data->primary_g, error);
     	if (status == MB_SUCCESS)
   	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * data->primary_nxy,
    				(void **)&data->primary_b, error);
     	if (status == MB_SUCCESS)
   	status = mb_mallocd(verbose, __FILE__, __LINE__, (data->primary_nxy / 8) + 1,
    				(void **)&data->primary_stat_color, error);
     	if (status == MB_SUCCESS)
   	status = mb_mallocd(verbose, __FILE__, __LINE__, (data->primary_nxy / 8) + 1,
    				(void **)&data->primary_stat_z, error);
	if (status != MB_SUCCESS)
	    {
	    fprintf(stderr,"\nUnable to allocate memory to store primary grid data\n");
	    fprintf(stderr,"\nProgram terminated in function <%s>.\n",
		    function_name);
	    exit(*error);
	    }

	/* copy grid */
	memcpy(data->primary_data, primary_data, data->primary_nxy * sizeof(float));

	/* reset contours and histograms */
	view->contourlorez = MB_NO;
	view->contourhirez = MB_NO;
	view->contourfullrez = MB_NO;
	view->primary_histogram_set = MB_NO;
	view->primaryslope_histogram_set = MB_NO;

	/* set status bit arrays */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);
	mbview_zscaleclear(instance);

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
int mbview_updateprimarygrid(int verbose, size_t instance,
			int	primary_nx,
			int	primary_ny,
			float	*primary_data,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_updateprimarygrid";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	first;
	int	i, j, k;

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                      %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                     %zu\n", instance);
		fprintf(stderr,"dbg2       primary_nx:                   %d\n", primary_nx);
		fprintf(stderr,"dbg2       primary_ny:                   %d\n", primary_ny);
		fprintf(stderr,"dbg2       primary_data:                 %p\n", primary_data);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set value and calculate derivative */
	if (primary_nx == data->primary_nx
		&& primary_ny == data->primary_ny)
		{
		first = MB_YES;
		for (k=0;k<data->primary_nx*data->primary_ny;k++)
			{
			data->primary_data[k] = primary_data[k];
			if (first == MB_YES && primary_data[k] != data->primary_nodatavalue)
				{
				data->primary_min = data->primary_data[k];
				data->primary_max = data->primary_data[k];
				first = MB_NO;
				}
			else if (primary_data[k] != data->primary_nodatavalue)
				{
				data->primary_min = MIN(data->primary_min, data->primary_data[k]);
				data->primary_max = MAX(data->primary_max, data->primary_data[k]);
				}
			}
		for (i=0;i<data->primary_nx;i++)
			{
			for (j=0;j<data->primary_ny;j++)
				{
				mbview_derivative(instance, i, j);
				}
			}
		}

	/* reset plotting and colors */
	view->lastdrawrez = MBV_REZ_NONE;
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* reset contour and histogram flags */
	view->contourlorez = MB_NO;
	view->contourhirez = MB_NO;
	view->contourfullrez = MB_NO;
	view->primary_histogram_set = MB_NO;
	view->primaryslope_histogram_set = MB_NO;

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
int mbview_updateprimarygridcell(int verbose, size_t instance,
			int	primary_ix,
			int	primary_jy,
			float	value,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_updateprimarygridcell";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	k;

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                      %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                     %zu\n", instance);
		fprintf(stderr,"dbg2       primary_ix:                   %d\n", primary_ix);
		fprintf(stderr,"dbg2       primary_jy:                   %d\n", primary_jy);
		fprintf(stderr,"dbg2       value:                        %f\n", value);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set value */
	if (primary_ix >= 0 && primary_ix < data->primary_nx
		&& primary_jy >= 0 && primary_jy < data->primary_ny)
		{
		/* update the cell value */
		k = primary_ix * data->primary_ny + primary_jy;
		data->primary_data[k] = value;
		data->primary_stat_z[k/8] = data->primary_stat_z[k/8] & (255 - statmask[k%8]);
		data->primary_stat_color[k/8] = data->primary_stat_color[k/8] & (255 - statmask[k%8]);

		/* calculate new derivative */
		mbview_derivative(instance, primary_ix, primary_jy);

		/* reset contour flags */
		view->contourlorez = MB_NO;
		view->contourhirez = MB_NO;
		view->contourfullrez = MB_NO;
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
int mbview_setprimarycolortable(int verbose, size_t instance,
			int	primary_colortable,
			int	primary_colortable_mode,
			double	primary_colortable_min,
			double	primary_colortable_max,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_setprimarycolortable";
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
		fprintf(stderr,"dbg2       primary_colortable:        %d\n", primary_colortable);
		fprintf(stderr,"dbg2       primary_colortable_mode:   %d\n", primary_colortable_mode);
		fprintf(stderr,"dbg2       primary_colortable_min:    %f\n", primary_colortable_min);
		fprintf(stderr,"dbg2       primary_colortable_max:    %f\n", primary_colortable_max);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set values */
        data->primary_colortable = primary_colortable;
        data->primary_colortable_mode = primary_colortable_mode;
        data->primary_colortable_min = primary_colortable_min;
        data->primary_colortable_max = primary_colortable_max;

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
int mbview_setslopecolortable(int verbose, size_t instance,
			int	slope_colortable,
			int	slope_colortable_mode,
			double	slope_colortable_min,
			double	slope_colortable_max,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_setslopecolortable";
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
		fprintf(stderr,"dbg2       slope_colortable:          %d\n", slope_colortable);
		fprintf(stderr,"dbg2       slope_colortable_mode:     %d\n", slope_colortable_mode);
		fprintf(stderr,"dbg2       slope_colortable_min:      %f\n", slope_colortable_min);
		fprintf(stderr,"dbg2       slope_colortable_max:      %f\n", slope_colortable_max);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set values */
        data->slope_colortable = slope_colortable;
        data->slope_colortable_mode = slope_colortable_mode;
        data->slope_colortable_min = slope_colortable_min;
        data->slope_colortable_max = slope_colortable_max;

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
