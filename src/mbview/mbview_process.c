/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_process.c	9/25/2003
 *    $Id: mbview_process.c,v 1.3 2003-12-01 20:55:48 caress Exp $
 *
 *    Copyright (c) 2003 by
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
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2003/11/25 01:43:19  caress
 * MBview version generated during EW0310.
 *
 *
 */
/*------------------------------------------------------------------------------*/

/* Standard includes for builtins. */
#include <stdio.h>
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

/* OpenGL include files */
#include <GL/gl.h>
#include <GL/glu.h>
#include "GL/GLwMDrawA.h" 
#include <GL/glx.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"

/* mbview include */
#include "mbview.h"
#include "mbviewprivate.h"

/*------------------------------------------------------------------------------*/

/* local variables */
Cardinal 	ac;
Arg      	args[256];
char		value_text[MB_PATH_MAXLINE];

static char rcs_id[]="$Id: mbview_process.c,v 1.3 2003-12-01 20:55:48 caress Exp $";

/*------------------------------------------------------------------------------*/
int mbview_projectdata(int instance)
{
	/* local variables */
	char	*function_name = "mbview_projectdata";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	proj_status = MB_SUCCESS;
	double	mtodeglon, mtodeglat;
	double	xgrid, ygrid, xlon, ylat, xdisplay, ydisplay;
	double	xlonmin, xlonmax, ylatmin, ylatmax;
	int	derivative_ok;
	double	dx, dy;
	int	i, j, k, k1, k2;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_projectdata: %d\n", instance);
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* delete old projections if necessary */
	if (view->primary_pj_init == MB_YES
		&& view->primary_pjptr != NULL)
		{
		mb_proj_free(mbv_verbose, &(view->primary_pjptr), &error);
		view->primary_pj_init = MB_NO;
		}
	if (view->secondary_pj_init == MB_YES
		&& view->secondary_pjptr != NULL)
		{
		mb_proj_free(mbv_verbose, &(view->secondary_pjptr), &error);
		view->secondary_pj_init = MB_NO;
		}
	if (view->display_pj_init == MB_YES
		&& view->display_pjptr != NULL)
		{
		mb_proj_free(mbv_verbose, &(view->display_pjptr), &error);
		view->display_pj_init = MB_NO;
		}
	if (data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		data->primary_grid_projection_mode = MBV_PROJECTION_PROJECTED;
	if (data->secondary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		data->secondary_grid_projection_mode = MBV_PROJECTION_PROJECTED;
	if (data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		data->display_projection_mode = MBV_PROJECTION_PROJECTED;

	/* check for case where primary grid is already projected but displayed
	   in that same projection 
	   - use same bounds info */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED
		&& data->display_projection_mode == MBV_PROJECTION_PROJECTED
		&& strcmp(data->primary_grid_projection_id,
				data->display_projection_id) == 0)
		{
		/* reset modes */
		data->primary_grid_projection_mode = MBV_PROJECTION_ALREADYPROJECTED;
		data->display_projection_mode = MBV_PROJECTION_ALREADYPROJECTED;
		
		/* get bounds */
		view->xmin = data->primary_xmin;
		view->xmax = data->primary_xmax;
		view->ymin = data->primary_ymin;
		view->ymax = data->primary_ymax;

		/* set projection for getting lon lat */
		proj_status = mb_proj_init(mbv_verbose, 
					data->primary_grid_projection_id,
					&(view->primary_pjptr),
					&error);
		if (proj_status == MB_SUCCESS)
			view->primary_pj_init = MB_YES;
		}
	   
	/* else set up projections as needed */
	else 
		{
		/* first go from grid coordinates to lon lat */
		if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED)
			{
			/* set projection */
			proj_status = mb_proj_init(mbv_verbose, 
						data->primary_grid_projection_id,
						&(view->primary_pjptr),
						&error);
			if (proj_status == MB_SUCCESS)
				view->primary_pj_init = MB_YES;
				
			/* get initial bounds */
			proj_status = mb_proj_inverse(mbv_verbose,
						view->primary_pjptr,
						data->primary_xmin,
						data->primary_ymin,
						&xlonmin, &ylatmin,
						&error);						
			proj_status = mb_proj_inverse(mbv_verbose,
						view->primary_pjptr,
						data->primary_xmax,
						data->primary_ymax,
						&xlonmax, &ylatmax,
						&error);						
			}
		else
			{
			/* already lon lat - just copy initial bounds */
			xlonmin = data->primary_xmin;
			xlonmax = data->primary_xmax;
			ylatmin = data->primary_ymin;
			ylatmax = data->primary_ymax;
			}
			
		/* now go from lon lat to display coordinates */
		if (data->display_projection_mode == MBV_PROJECTION_PROJECTED)
			{
			/* set projection */
			proj_status = mb_proj_init(mbv_verbose, 
						data->display_projection_id,
						&(view->display_pjptr),
						&error);
			if (proj_status == MB_SUCCESS)
				view->display_pj_init = MB_YES;
				
			/* get bounds */
			proj_status = mb_proj_forward(mbv_verbose,
						view->display_pjptr,
						xlonmin, ylatmin,
						&view->xmin, 
						&view->ymin,
						&error);						
			proj_status = mb_proj_forward(mbv_verbose,
						view->display_pjptr,
						xlonmax, ylatmax,
						&view->xmax, 
						&view->ymax,
						&error);						
			}
		else if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC)
			{
			/* set up geographic pseduo-projection */
			mb_coor_scale(mbv_verbose, 
	 			0.5*(ylatmin + ylatmax),
	 			&(view->mtodeglon), &(view->mtodeglat));

			/* get bounds */
			view->xmin = xlonmin / view->mtodeglon;
			view->xmax = xlonmax / view->mtodeglon;
			view->ymin = ylatmin / view->mtodeglat;
			view->ymax = ylatmax / view->mtodeglat;
			}
		}
fprintf(stderr,"Projections:\n");
fprintf(stderr,"  Grid: mode:%d id:%s\n", 
data->primary_grid_projection_mode, data->primary_grid_projection_id);
fprintf(stderr,"  Display: mode:%d id:%s\n", 
data->display_projection_mode, data->display_projection_id);
fprintf(stderr,"  Display min max: %f %f %f %f\n",
view->xmin, view->xmax, view->ymin, view->ymax);

	/* set projection for secondary grid if needed */
	if (data->secondary_nxy > 0
		&& data->secondary_grid_projection_mode == MBV_PROJECTION_PROJECTED)
		{
		/* set projection for getting lon lat */
		proj_status = mb_proj_init(mbv_verbose, 
					data->secondary_grid_projection_id,
					&(view->secondary_pjptr),
					&error);
		if (proj_status == MB_SUCCESS)
			view->secondary_pj_init = MB_YES;
		}
		
	/* get origin and scaling */
	view->xorigin = 0.5 * (view->xmin + view->xmax);
	view->yorigin = 0.5 * (view->ymin + view->ymax);
	view->zorigin = 0.5 * (data->primary_min + data->primary_max);
	view->scale  = MIN((1.75 * MBV_OPENGL_WIDTH 
					/ (view->xmax - view->xmin)),
				(1.75 * MBV_OPENGL_WIDTH  
					/ view->aspect_ratio
					/ (view->ymax - view->ymin)));
	view->zscale = data->exageration * view->scale;
	view->size2d = 1.0;

	/* set x and y arrays */
	for (i=0;i<data->primary_nx;i++)
	{
	for (j=0;j<data->primary_ny;j++)
		{
		/* get raw values in grid */
		k = i * data->primary_ny + j;
		xgrid = data->primary_xmin + i * data->primary_dx;
		ygrid = data->primary_ymin + j * data->primary_dy;
		
		/* reproject positions into display coordinates */
		mbview_projectforward(instance, MB_NO,
					xgrid, ygrid,
					&xlon, &ylat,
					&xdisplay, &ydisplay);
					
		/* insert into plotting arrays */
		data->primary_x[k] = (float)xdisplay;
		data->primary_y[k] = (float)ydisplay;
		}
		
	/* check for pending event */
	if (view->plot_done == MB_NO 
		&& view->plot_interrupt_allowed == MB_YES 
		&& i % MBV_EVENTCHECKCOARSENESS == 0)
		do_mbview_xevents();
		
	/* dump out of loop if plotting already done at a higher recursion */
	if (view->plot_done == MB_YES)
		i = data->primary_nx;
	}

	/* calculate derivatives of primary data */
	for (i=0;i<data->primary_nx;i++)
	{
	for (j=0;j<data->primary_ny;j++)
		{
		/* figure if x derivative can be calculated */
		derivative_ok = MB_NO;
		if (i == 0)
		    {
		    k1 = i * data->primary_ny + j;
		    k2 = (i + 1) * data->primary_ny + j;
		    if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = MB_YES;
		    }
		else if (i == data->primary_nx - 1)
		    {
		    k1 = (i - 1) * data->primary_ny + j;
		    k2 = i * data->primary_ny + j;
		    if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = MB_YES;
		    }
		else
		    {
		    k1 = (i - 1) * data->primary_ny + j;
		    k = i * data->primary_ny + j;
		    k2 = (i + 1) * data->primary_ny + j;
		    if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = MB_YES;
		    else if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k] != data->primary_nodatavalue)
			{
			derivative_ok = MB_YES;
			k2 = k;
			}
		    else if (data->primary_data[k] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			{
			derivative_ok = MB_YES;
			k1 = k;
			}
		    }
		
		/* calculate x derivative */
		if (derivative_ok == MB_YES)
			{
			dx = (data->primary_x[k2] 
				- data->primary_x[k1]);
			if (dx != 0.0)
			data->primary_dzdx[k] 
				= view->scale * 
					(data->primary_data[k2] 
						- data->primary_data[k1])
					/ dx;
			else
			data->primary_dzdx[k] = 0.0;
			}
		else
			data->primary_dzdx[k] = 0.0;
		
		/* figure if y derivative can be calculated */
		derivative_ok = MB_NO;
		if (j == 0)
		    {
		    k1 = i * data->primary_ny + j;
		    k2 = i * data->primary_ny + (j + 1);
		    if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = MB_YES;
		    }
		else if (i == data->primary_ny - 1)
		    {
		    k1 = i * data->primary_ny + (j - 1);
		    k2 = i * data->primary_ny + j;
		    if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = MB_YES;
		    }
		else
		    {
		    k1 = i * data->primary_ny + (j - 1);
		    k = i * data->primary_ny + j;
		    k2 = i * data->primary_ny + (j + 1);
		    if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = MB_YES;
		    else if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k] != data->primary_nodatavalue)
			{
			derivative_ok = MB_YES;
			k2 = k;
			}
		    else if (data->primary_data[k] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			{
			derivative_ok = MB_YES;
			k1 = k;
			}
		    }
		
		/* calculate y derivative */
		if (derivative_ok == MB_YES)
			{
			dy = (data->primary_y[k2] 
				- data->primary_y[k1]);
			if (dy != 0.0)
			data->primary_dzdy[k] 
				= view->scale * 
					(data->primary_data[k2] 
						- data->primary_data[k1])
					/ (data->primary_y[k2] 
						- data->primary_y[k1]);
			else
			data->primary_dzdy[k] = 0.0;
			}
		else
			data->primary_dzdy[k] = 0.0;
		}
		
	/* check for pending event */
	if (view->plot_done == MB_NO 
		&& view->plot_interrupt_allowed == MB_YES 
		&& i % MBV_EVENTCHECKCOARSENESS == 0)
		do_mbview_xevents();
		
	/* dump out of loop if plotting already done at a higher recursion */
	if (view->plot_done == MB_YES)
		i = data->primary_nx;
	}
	
	/* handle picks */
	if (data->pick_type != MBV_PICK_NONE)
		{
		mbview_projectforward(instance, MB_YES,
				data->pick.endpoints[0].xgrid,
				data->pick.endpoints[0].ygrid,
				&(data->pick.endpoints[0].xlon),
				&(data->pick.endpoints[0].ylat),
				&(data->pick.endpoints[0].xdisplay),
				&(data->pick.endpoints[0].ydisplay));
		for (i=0;i<4;i++)
			{
			mbview_projectforward(instance, MB_YES,
				data->pick.xpoints[i].xgrid,
				data->pick.xpoints[i].ygrid,
				&(data->pick.xpoints[i].xlon),
				&(data->pick.xpoints[i].ylat),
				&(data->pick.xpoints[i].xdisplay),
				&(data->pick.xpoints[i].ydisplay));
			}
		for (i=0;i<2;i++)
			{
			for (j=0;j<data->pick.xsegments[i].nls;j++)
				{
				mbview_projectforward(instance, MB_YES,
						data->pick.xsegments[i].lspoints[j].xgrid,
						data->pick.xsegments[i].lspoints[j].ygrid,
						&(data->pick.xsegments[i].lspoints[j].xlon),
						&(data->pick.xsegments[i].lspoints[j].ylat),
						&(data->pick.xsegments[i].lspoints[j].xdisplay),
						&(data->pick.xsegments[i].lspoints[j].ydisplay));
				}
			}
		}
	if (data->pick_type == MBV_PICK_TWOPOINT)
		{
		mbview_projectforward(instance, MB_YES,
				data->pick.endpoints[1].xgrid,
				data->pick.endpoints[1].ygrid,
				&(data->pick.endpoints[1].xlon),
				&(data->pick.endpoints[1].ylat),
				&(data->pick.endpoints[1].xdisplay),
				&(data->pick.endpoints[1].ydisplay));
		for (i=4;i<8;i++)
			{
			mbview_projectforward(instance, MB_YES,
				data->pick.xpoints[i].xgrid,
				data->pick.xpoints[i].ygrid,
				&(data->pick.xpoints[i].xlon),
				&(data->pick.xpoints[i].ylat),
				&(data->pick.xpoints[i].xdisplay),
				&(data->pick.xpoints[i].ydisplay));
			}
		for (i=2;i<4;i++)
			{
			for (j=0;j<data->pick.xsegments[i].nls;j++)
				{
				mbview_projectforward(instance, MB_YES,
						data->pick.xsegments[i].lspoints[j].xgrid,
						data->pick.xsegments[i].lspoints[j].ygrid,
						&(data->pick.xsegments[i].lspoints[j].xlon),
						&(data->pick.xsegments[i].lspoints[j].ylat),
						&(data->pick.xsegments[i].lspoints[j].xdisplay),
						&(data->pick.xsegments[i].lspoints[j].ydisplay));
				}
			}
		if (data->pick.segment.nls > 0)
			{
			for (j=0;j<data->pick.segment.nls;j++)
				{
				mbview_projectforward(instance, MB_YES,
						data->pick.segment.lspoints[j].xgrid,
						data->pick.segment.lspoints[j].ygrid,
						&(data->pick.segment.lspoints[j].xlon),
						&(data->pick.segment.lspoints[j].ylat),
						&(data->pick.segment.lspoints[j].xdisplay),
						&(data->pick.segment.lspoints[j].ydisplay));
				}
			}
		}
		
	/* handle area */
	if (data->area_type == MBV_AREA_QUAD)
		{
		for (i=0;i<2;i++)
			{
			mbview_projectforward(instance, MB_YES,
					data->area.endpoints[i].xgrid,
					data->area.endpoints[i].ygrid,
					&(data->area.endpoints[i].xlon),
					&(data->area.endpoints[i].ylat),
					&(data->area.endpoints[i].xdisplay),
					&(data->area.endpoints[i].ydisplay));
			}
		for (j=0;j<data->area.segment.nls;j++)
			{
			mbview_projectforward(instance, MB_YES,
					data->area.segment.lspoints[j].xgrid,
					data->area.segment.lspoints[j].ygrid,
					&(data->area.segment.lspoints[j].xlon),
					&(data->area.segment.lspoints[j].ylat),
					&(data->area.segment.lspoints[j].xdisplay),
					&(data->area.segment.lspoints[j].ydisplay));
			}
		for (i=0;i<4;i++)
			{
			for (j=0;j<2;j++)
				{
				mbview_projectforward(instance, MB_YES,
						data->area.segments[i].endpoints[j]->xgrid,
						data->area.segments[i].endpoints[j]->ygrid,
						&(data->area.segments[i].endpoints[j]->xlon),
						&(data->area.segments[i].endpoints[j]->ylat),
						&(data->area.segments[i].endpoints[j]->xdisplay),
						&(data->area.segments[i].endpoints[j]->ydisplay));
				}
			for (j=0;j<data->area.segments[i].nls;j++)
				{
				mbview_projectforward(instance, MB_YES,
						data->area.segments[i].lspoints[j].xgrid,
						data->area.segments[i].lspoints[j].ygrid,
						&(data->area.segments[i].lspoints[j].xlon),
						&(data->area.segments[i].lspoints[j].ylat),
						&(data->area.segments[i].lspoints[j].xdisplay),
						&(data->area.segments[i].lspoints[j].ydisplay));
				}
			}
		}
		
	/* clear zscale for grid */
	mbview_zscaleclear(instance);
		
	/* scale data other than the grid */
	mbview_zscale(instance);
	
	/* set projected flag only if plotting not done */
	if (view->plot_done == MB_NO)
		{
		view->projected = MB_YES;
		}

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
int mbview_zscalepoint(
	struct mbview_world_struct *view,
	struct mbview_struct *data,
	int k)
{
	/* local variables */
	char	*function_name = "mbview_zscalepoint";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	l;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       view:             %d\n",view);
		fprintf(stderr,"dbg2       data:             %d\n",data);
		fprintf(stderr,"dbg2       k:                %d\n",k);
		}
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_zscalepoint: %d\n", k);

	/* scale z array */
	data->primary_z[k] 
		= (float)(view->zscale 
			* (data->primary_data[k] - view->zorigin));

	/* set zscale status bit */
	data->primary_stat_z[k/8] 
		= data->primary_stat_z[k/8] | statmask[k%8];

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
int mbview_zscale(int instance)
{
	/* local variables */
	char	*function_name = "mbview_zscale";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	i, j, k, l;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_zscale: %d\n", instance);
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set z scale */		
	view->zscale = data->exageration * view->scale;
	
	/* handle picks */
	if (data->pick_type != MBV_PICK_NONE)
		{
		data->pick.endpoints[0].zdisplay 
				= (float)(view->zscale 
					* (data->pick.endpoints[0].zdata 
						- view->zorigin)
						+ MBV_OPENGL_3D_LINE_OFFSET);
		for (i=0;i<4;i++)
			{
			data->pick.xpoints[i].zdisplay 
					= (float)(view->zscale 
						* (data->pick.xpoints[i].zdata 
							- view->zorigin)
							+ MBV_OPENGL_3D_LINE_OFFSET);
			}
		for(i=0;i<2;i++)
			{
			if (data->pick.xsegments[i].nls > 0)
				{
				for (j=0;j<data->pick.xsegments[i].nls;j++)
					{
					data->pick.xsegments[i].lspoints[j].zdisplay 
							= (float)(view->zscale 
								* (data->pick.xsegments[i].lspoints[j].zdata 
									- view->zorigin)
									+ MBV_OPENGL_3D_LINE_OFFSET);
					}
				}
			}
		}
	if (data->pick_type == MBV_PICK_TWOPOINT)
		{
		data->pick.endpoints[1].zdisplay 
				= (float)(view->zscale 
					* (data->pick.endpoints[1].zdata 
						- view->zorigin)
						+ MBV_OPENGL_3D_LINE_OFFSET);
		for (i=4;i<8;i++)
			{
			data->pick.xpoints[i].zdisplay 
					= (float)(view->zscale 
						* (data->pick.xpoints[i].zdata 
							- view->zorigin)
							+ MBV_OPENGL_3D_LINE_OFFSET);
			}
		for(i=2;i<4;i++)
			{
			if (data->pick.xsegments[i].nls > 0)
				{
				for (j=0;j<data->pick.xsegments[i].nls;j++)
					{
					data->pick.xsegments[i].lspoints[j].zdisplay 
							= (float)(view->zscale 
								* (data->pick.xsegments[i].lspoints[j].zdata 
									- view->zorigin)
									+ MBV_OPENGL_3D_LINE_OFFSET);
					}
				}
			}
		if (data->pick.segment.nls > 0)
			{
			for (i=0;i<data->pick.segment.nls;i++)
				{
				data->pick.segment.lspoints[i].zdisplay 
						= (float)(view->zscale 
							* (data->pick.segment.lspoints[i].zdata 
								- view->zorigin)
								+ MBV_OPENGL_3D_LINE_OFFSET);
				}
			}
		}
		
	/* handle area */
	if (data->area_type == MBV_AREA_QUAD)
		{
		for (i=0;i<2;i++)
			{
			data->area.endpoints[i].zdisplay 
				= (float)(view->zscale 
					* (data->area.endpoints[i].zdata 
						- view->zorigin)
						+ MBV_OPENGL_3D_LINE_OFFSET);
			}
		for (j=0;j<data->area.segment.nls;j++)
			{
			data->area.segment.lspoints[j].zdisplay 
				= (float)(view->zscale 
					* (data->area.segment.lspoints[j].zdata 
						- view->zorigin)
						+ MBV_OPENGL_3D_LINE_OFFSET);
			}
		for (i=0;i<4;i++)
			{
			data->area.cornerpoints[i].zdisplay 
				= (float)(view->zscale 
					* (data->area.cornerpoints[i].zdata 
						- view->zorigin)
						+ MBV_OPENGL_3D_LINE_OFFSET);
			for (j=0;j<data->area.segments[i].nls;j++)
				{
				data->area.segments[i].lspoints[j].zdisplay 
					= (float)(view->zscale 
						* (data->area.segments[i].lspoints[j].zdata 
							- view->zorigin)
							+ MBV_OPENGL_3D_LINE_OFFSET);
				}
			}
		}
	
	/* handle navpicks */
	if (data->navpick_type != MBV_PICK_NONE)
		{
		data->navpick.endpoints[0].zdisplay 
				= (float)(view->zscale 
					* (data->navpick.endpoints[0].zdata 
						- view->zorigin)
						+ MBV_OPENGL_3D_LINE_OFFSET);
		for (i=0;i<4;i++)
			{
			data->navpick.xpoints[i].zdisplay 
					= (float)(view->zscale 
						* (data->navpick.xpoints[i].zdata 
							- view->zorigin)
							+ MBV_OPENGL_3D_LINE_OFFSET);
			}
		for(i=0;i<2;i++)
			{
			if (data->navpick.xsegments[i].nls > 0)
				{
				for (j=0;j<data->navpick.xsegments[i].nls;j++)
					{
					data->navpick.xsegments[i].lspoints[j].zdisplay 
							= (float)(view->zscale 
								* (data->navpick.xsegments[i].lspoints[j].zdata 
									- view->zorigin)
									+ MBV_OPENGL_3D_LINE_OFFSET);
					}
				}
			}
		}
	if (data->navpick_type == MBV_PICK_TWOPOINT)
		{
		data->navpick.endpoints[1].zdisplay 
				= (float)(view->zscale 
					* (data->navpick.endpoints[1].zdata 
						- view->zorigin)
						+ MBV_OPENGL_3D_LINE_OFFSET);
		for (i=4;i<8;i++)
			{
			data->navpick.xpoints[i].zdisplay 
					= (float)(view->zscale 
						* (data->navpick.xpoints[i].zdata 
							- view->zorigin)
							+ MBV_OPENGL_3D_LINE_OFFSET);
			}
		for(i=2;i<4;i++)
			{
			if (data->navpick.xsegments[i].nls > 0)
				{
				for (j=0;j<data->navpick.xsegments[i].nls;j++)
					{
					data->navpick.xsegments[i].lspoints[j].zdisplay 
							= (float)(view->zscale 
								* (data->navpick.xsegments[i].lspoints[j].zdata 
									- view->zorigin)
									+ MBV_OPENGL_3D_LINE_OFFSET);
					}
				}
			}
		}
		
	/* handle sites */
	if (data->nsite > 0)
		{
		for (i=0;i<data->nsite;i++)
			{
			data->sites[i].point.zdisplay 
				= (float)(view->zscale 
					* (data->sites[i].point.zdata 
						- view->zorigin)
						+ MBV_OPENGL_3D_LINE_OFFSET);
			}
		}
		
	/* handle routes */
	if (data->nroute > 0)
		{
		for (i=0;i<data->nroute;i++)
		    {
		    for (j=0;j<data->routes[i].npoints;j++)
			{
			data->routes[i].points[j].zdisplay 
				= (float)(view->zscale 
					* (data->routes[i].points[j].zdata 
						- view->zorigin)
						+ MBV_OPENGL_3D_LINE_OFFSET);
			}
		    for (j=0;j<data->routes[i].npoints-1;j++)
			{
			for (k=0;k<data->routes[i].segments[j].nls;k++)
				{
				data->routes[i].segments[j].lspoints[k].zdisplay 
					= (float)(view->zscale 
						* (data->routes[i].segments[j].lspoints[k].zdata 
							- view->zorigin)
							+ MBV_OPENGL_3D_LINE_OFFSET);
				}
			}
		    }
		}
		
	/* handle nav */
	if (data->nnav > 0)
		{
		for (i=0;i<data->nnav;i++)
		    {
		    for (j=0;j<data->navs[i].npoints;j++)
			{
			data->navs[i].navpts[j].point.zdisplay 
				= (float)(view->zscale 
					* (data->navs[i].navpts[j].point.zdata 
						- view->zorigin)
						+ MBV_OPENGL_3D_LINE_OFFSET);
			data->navs[i].navpts[j].pointport.zdisplay 
				= (float)(view->zscale 
					* (data->navs[i].navpts[j].pointport.zdata 
						- view->zorigin)
						+ MBV_OPENGL_3D_LINE_OFFSET);
			data->navs[i].navpts[j].pointcntr.zdisplay 
				= (float)(view->zscale 
					* (data->navs[i].navpts[j].pointcntr.zdata 
						- view->zorigin)
						+ MBV_OPENGL_3D_LINE_OFFSET);
			data->navs[i].navpts[j].pointstbd.zdisplay 
				= (float)(view->zscale 
					* (data->navs[i].navpts[j].pointstbd.zdata 
						- view->zorigin)
						+ MBV_OPENGL_3D_LINE_OFFSET);
			}
		    for (j=0;j<data->navs[i].npoints-1;j++)
			{
			for (k=0;k<data->navs[i].segments[j].nls;k++)
				{
				data->navs[i].segments[j].lspoints[k].zdisplay 
					= (float)(view->zscale 
						* (data->navs[i].segments[j].lspoints[k].zdata 
							- view->zorigin)
							+ MBV_OPENGL_3D_LINE_OFFSET);
				}
			}
		    }
		}
	
	/* set rez flags only if plotting not done */
	if (view->plot_done == MB_NO)
		{
        	view->contourlorez = MB_NO;
        	view->contourhirez = MB_NO;
        	view->contourfullrez = MB_NO;
		}

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
int mbview_projectforward(int instance, int needlonlat,
				double xgrid, double ygrid,
				double *xlon, double *ylat,
				double *xdisplay, double *ydisplay)
{
	/* local variables */
	char	*function_name = "mbview_projectforward";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	proj_status = MB_SUCCESS;
	double	xx, yy;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       needlonlat:       %d\n",needlonlat);
		fprintf(stderr,"dbg2       xgrid:            %f\n",xgrid);
		fprintf(stderr,"dbg2       ygrid:            %f\n",ygrid);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
		
	/* get positions into geographic coordinates if necessary */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED)
		{
		mb_proj_inverse(mbv_verbose, view->primary_pjptr,
					xgrid, ygrid, xlon, ylat, &error);
		}
	else if (data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		if (needlonlat == MB_YES)
			{
			mb_proj_inverse(mbv_verbose, view->primary_pjptr,
					xgrid, ygrid, xlon, ylat, &error);
			}
		}
	else if (data->primary_grid_projection_mode == MBV_PROJECTION_GEOGRAPHIC)
		{
		*xlon = xgrid;
		*ylat = ygrid;
		}

	/* get positions in the display projection */
	if (data->display_projection_mode == MBV_PROJECTION_PROJECTED)
		{
		mb_proj_forward(mbv_verbose, view->display_pjptr,
				*xlon, 
				*ylat, 
				&xx,
				&yy,
				&error);
		}
	else if (data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		xx = xgrid;
		yy = ygrid;
		}
	else if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC)
		{
		xx = *xlon / view->mtodeglon;
		yy = *ylat / view->mtodeglat;
		}

	/* get final positions in display coordinates */
	*xdisplay = view->scale * (xx - view->xorigin);
	*ydisplay = view->scale * (yy - view->yorigin);

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       xlon:        %f\n",*xlon);
		fprintf(stderr,"dbg2       ylat:        %f\n",*ylat);
		fprintf(stderr,"dbg2       xdisplay:    %f\n",*xdisplay);
		fprintf(stderr,"dbg2       ydisplay:    %f\n",*ydisplay);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectinverse(int instance, int needlonlat,
				double xdisplay, double ydisplay,
				double *xlon, double *ylat,
				double *xgrid, double *ygrid)
{
	/* local variables */
	char	*function_name = "mbview_projectinverse";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	proj_status = MB_SUCCESS;
	double	xx, yy;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       needlonlat:       %d\n",needlonlat);
		fprintf(stderr,"dbg2       xdisplay:         %f\n",xdisplay);
		fprintf(stderr,"dbg2       ydisplay:         %f\n",ydisplay);
		}

		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get positions in display projection */
	xx = xdisplay / view->scale + view->xorigin;
	yy = ydisplay / view->scale + view->yorigin;

	/* get positions in geographic coordinates */
	if (data->display_projection_mode == MBV_PROJECTION_PROJECTED)
		{
		mb_proj_inverse(mbv_verbose, view->display_pjptr,
				xx, yy, xlon, ylat, &error);
		}
	else if (data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		if (needlonlat == MB_YES)
			mb_proj_inverse(mbv_verbose, view->display_pjptr,
					xx, yy, xlon, ylat, &error);
		}
	else if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC)
		{
		*xlon = xx * view->mtodeglon;
		*ylat = yy * view->mtodeglat;
		}

	/* get positions into grid coordinates */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED)
		{
		mb_proj_forward(mbv_verbose, view->primary_pjptr,
				*xlon, *ylat, xgrid, ygrid, &error);
		}
	else if (data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		*xgrid = xx;
		*ygrid = yy;
		}
	else
		{
		*xgrid = *xlon;
		*ygrid = *ylat;
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       xlon:         %f\n",*xlon);
		fprintf(stderr,"dbg2       ylat:         %f\n",*ylat);
		fprintf(stderr,"dbg2       xgrid:        %f\n",*xgrid);
		fprintf(stderr,"dbg2       ygrid:        %f\n",*ygrid);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:       %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectfromlonlat(int instance,
				double xlon, double ylat,
				double *xgrid, double *ygrid,
				double *xdisplay, double *ydisplay)
{
	/* local variables */
	char	*function_name = "mbview_projectfromlonlat";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	proj_status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	xx, yy;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xlon:             %f\n",xlon);
		fprintf(stderr,"dbg2       ylat:             %f\n",ylat);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get positions into grid coordinates */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED
		|| data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		mb_proj_forward(mbv_verbose, view->primary_pjptr,
				xlon, ylat, xgrid, ygrid, &error);
		}
	else
		{
		*xgrid = xlon;
		*ygrid = ylat;
		}

	/* get positions in the display projection */
	if (data->display_projection_mode == MBV_PROJECTION_PROJECTED)
		{
		mb_proj_forward(mbv_verbose, view->display_pjptr,
				xlon, 
				ylat, 
				&xx,
				&yy,
				&error);
		}
	else if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC)
		{
		xx = xlon / view->mtodeglon;
		yy = ylat / view->mtodeglat;
		}

	/* get final positions in display coordinates */
	*xdisplay = view->scale * (xx - view->xorigin);
	*ydisplay = view->scale * (yy - view->yorigin);

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       xgrid:       %f\n",*xgrid);
		fprintf(stderr,"dbg2       ygrid:       %f\n",*ygrid);
		fprintf(stderr,"dbg2       xdisplay:    %f\n",*xdisplay);
		fprintf(stderr,"dbg2       ydisplay:    %f\n",*ydisplay);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectll2meters(int instance,
				double xlon, double ylat,
				double *xx, double *yy)
{
	/* local variables */
	char	*function_name = "mbview_projectfromlonlat";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	proj_status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xlon:             %f\n",xlon);
		fprintf(stderr,"dbg2       ylat:             %f\n",ylat);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get positions in unscaled display projection */
	if (data->display_projection_mode == MBV_PROJECTION_PROJECTED)
		{
		mb_proj_forward(mbv_verbose, view->display_pjptr,
				xlon, 
				ylat, 
				xx,
				yy,
				&error);
		}
	else if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC)
		{
		*xx = xlon / view->mtodeglon;
		*yy = ylat / view->mtodeglat;
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       xx:          %f\n",*xx);
		fprintf(stderr,"dbg2       yy:          %f\n",*yy);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_colorclear(int instance)
{
	/* local variables */
	char	*function_name = "mbview_colorclear";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
		
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_colorclear: %d\n", instance);
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set status bit arrays */
	view->colordonecount = 0;
	if (data->primary_stat_color != NULL)
		memset(data->primary_stat_color, 0, (data->primary_nxy / 8) + 1);

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_zscaleclear(int instance)
{
	/* local variables */
	char	*function_name = "mbview_zscaleclear";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
		
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_zscaleclear: %d\n", instance);
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set status bit arrays */
	view->zscaledonecount = 0;
	if (data->primary_stat_z != NULL)
		memset(data->primary_stat_z, 0, (data->primary_nxy / 8) + 1);

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_setcolorparms(int instance)
{
	/* local variables */
	char	*function_name = "mbview_setcolorparms";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get min max values for coloring */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
		{
		view->colortable = data->primary_colortable;
		view->colortable_mode = data->primary_colortable_mode;
		view->min = data->primary_colortable_min;
		view->max = data->primary_colortable_max;
		}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
		{
		view->colortable = data->slope_colortable;
		view->colortable_mode = data->slope_colortable_mode;
		view->min = data->slope_colortable_min;
		view->max = data->slope_colortable_max;
		}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
		{
		view->colortable = data->secondary_colortable;
		view->colortable_mode = data->secondary_colortable_mode;
		view->min = data->secondary_colortable_min;
		view->max = data->secondary_colortable_max;
		}
		
	/* get illumination vector if necessary */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
	    view->shade_mode = data->primary_shade_mode;
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
	    view->shade_mode = data->slope_shade_mode;
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
	    view->shade_mode = data->secondary_shade_mode;
	view->illum_x = 0.0;
	view->illum_y = 0.0;
	view->illum_z = 0.0;
	view->mag2 = 0.0;
	if (view->shade_mode == MBV_SHADE_VIEW_ILLUMINATION)
		{
        	view->illum_x = sin(DTR * data->illuminate_azimuth) 
				* cos(DTR * data->illuminate_elevation);
       	 	view->illum_y = cos(DTR * data->illuminate_azimuth) 
				* cos(DTR * data->illuminate_elevation);
        	view->illum_z = sin(DTR * data->illuminate_elevation);
		view->mag2 = data->illuminate_magnitude
			* data->illuminate_magnitude;
/*fprintf(stderr,"ILLUMRAW: %f %f %f\n",
data->illuminate_azimuth, data->illuminate_elevation, data->illuminate_magnitude);
fprintf(stderr,"ILLUMLGT: %f %f %f %f\n",view->illum_x, view->illum_y, view->illum_z, view->mag2);*/
		}
		
	/* get sign of overlay shading if necessary */
	view->sign = 1.0;
	if (view->shade_mode == MBV_SHADE_VIEW_OVERLAY)
		{
		if (data->overlay_shade_mode == MBV_COLORTABLE_NORMAL)
			view->sign = 1.0;
		else
			view->sign = -1.0;
		}
	
	/* get colortable */
	if (view->colortable == MBV_COLORTABLE_HAXBY)
		{
		view->colortable_red = colortable_haxby_red;
		view->colortable_blue = colortable_haxby_blue;
		view->colortable_green = colortable_haxby_green;
		}
	else if (view->colortable == MBV_COLORTABLE_BRIGHT)
		{
		view->colortable_red = colortable_bright_red;
		view->colortable_blue = colortable_bright_blue;
		view->colortable_green = colortable_bright_green;
		}
	else if (view->colortable == MBV_COLORTABLE_MUTED)
		{
		view->colortable_red = colortable_muted_red;
		view->colortable_blue = colortable_muted_blue;
		view->colortable_green = colortable_muted_green;
		}
	else if (view->colortable == MBV_COLORTABLE_GRAY)
		{
		view->colortable_red = colortable_gray_red;
		view->colortable_blue = colortable_gray_blue;
		view->colortable_green = colortable_gray_green;
		}
	else if (view->colortable == MBV_COLORTABLE_FLAT)
		{
		view->colortable_red = colortable_flat_red;
		view->colortable_blue = colortable_flat_blue;
		view->colortable_green = colortable_flat_green;
		}
	else if (view->colortable == MBV_COLORTABLE_SEALEVEL)
		{
		view->colortable_red = colortable_haxby_red;
		view->colortable_blue = colortable_haxby_blue;
		view->colortable_green = colortable_haxby_green;
		}

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
int mbview_colorpoint(
	struct mbview_world_struct *view,
	struct mbview_struct *data,
	int i, int j, int k)
{
	/* local variables */
	char	*function_name = "mbview_colorpoint";
	int	status = MB_SUCCESS;
	double	value, svalue, factor, dd;
	double	xlon, ylat;
	double	intensity;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       view:             %d\n",view);
		fprintf(stderr,"dbg2       data:             %d\n",data);
		fprintf(stderr,"dbg2       i:                %d\n",i);
		fprintf(stderr,"dbg2       j:                %d\n",j);
		fprintf(stderr,"dbg2       k:                %d\n",k);
		}
		
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_colorpoint: %d %d %d\n", i, j, k);
		
	/* get values for coloring */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
		{
		value = data->primary_data[k];
		}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
		{
		value = sqrt(data->primary_dzdx[k] 
					* data->primary_dzdx[k]
				+ data->primary_dzdy[k] 
					* data->primary_dzdy[k]);
		}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
		{
		if (data->secondary_sameas_primary == MB_YES)
			value = data->secondary_data[k];
		else
			mbview_getsecondaryvalue(view, data, i, j, &value);
		}

	/* get color */
	if (view->colortable != MBV_COLORTABLE_SEALEVEL)
	    mbview_getcolor(value, view->min, view->max, view->colortable_mode, 
			view->colortable_red,
			view->colortable_green,
			view->colortable_blue,
			&data->primary_r[k],
			&data->primary_g[k],
			&data->primary_b[k]);

	else
	    {
	    if (value > 0.0)
		{
		if (view->colortable_mode == MBV_COLORTABLE_NORMAL)
		    {
		    mbview_getcolor(value, 0.0, view->max, view->colortable_mode, 
			colortable_abovesealevel_red,
			colortable_abovesealevel_green,
			colortable_abovesealevel_blue,
			&data->primary_r[k],
			&data->primary_g[k],
			&data->primary_b[k]);
		    }
		else
		    {
		    mbview_getcolor(value, -view->max / 11.0, view->max, view->colortable_mode, 
			colortable_haxby_red,
			colortable_haxby_green,
			colortable_haxby_blue,
			&data->primary_r[k],
			&data->primary_g[k],
			&data->primary_b[k]);
		    }
		}
	    else
		{
		if (view->colortable_mode == MBV_COLORTABLE_REVERSED)
		    {
		    mbview_getcolor(value, view->min, 0.0, view->colortable_mode, 
			colortable_abovesealevel_red,
			colortable_abovesealevel_green,
			colortable_abovesealevel_blue,
			&data->primary_r[k],
			&data->primary_g[k],
			&data->primary_b[k]);
		    }
		else
		    {
		    view->colortable_red = colortable_haxby_red;
		    view->colortable_green = colortable_haxby_green;
		    view->colortable_blue = colortable_haxby_blue;
		    mbview_getcolor(value, view->min, -view->min / 11.0, view->colortable_mode, 
			colortable_haxby_red,
			colortable_haxby_green,
			colortable_haxby_blue,
			&data->primary_r[k],
			&data->primary_g[k],
			&data->primary_b[k]);
		    }
		}
	    }

	/* get values for shading */
	if (view->shade_mode != MBV_SHADE_VIEW_NONE)
	    {
	    if (view->shade_mode == MBV_SHADE_VIEW_ILLUMINATION)
		{
		dd = sqrt(view->mag2 * data->primary_dzdx[k] 
				* data->primary_dzdx[k]
			+ view->mag2 * data->primary_dzdy[k] 
				* data->primary_dzdy[k]
				+ 1.0);
		intensity = data->illuminate_magnitude 
			    * view->illum_x * data->primary_dzdx[k] / dd
			+ data->illuminate_magnitude 
			    * view->illum_y * data->primary_dzdy[k] / dd
			+ view->illum_z / dd
			- 0.5;
/*if (j==25)
fprintf(stderr,"intensity:%f  dzdx:%f  dzdy:%f\n",
intensity,data->primary_dzdx[k], data->primary_dzdy[k]);
*/

		mbview_applyshade(intensity,
			&data->primary_r[k],
			&data->primary_g[k],
			&data->primary_b[k]);
		}
	    else if (view->shade_mode == MBV_SHADE_VIEW_SLOPE)
		{
		intensity = -data->slope_magnitude
			* sqrt(data->primary_dzdx[k] 
					* data->primary_dzdx[k]
				+ data->primary_dzdy[k] 
					* data->primary_dzdy[k]);
		intensity = MAX(intensity, -1.0);
		mbview_applyshade(intensity,
			&data->primary_r[k],
			&data->primary_g[k],
			&data->primary_b[k]);
		}
	    else if (view->shade_mode == MBV_SHADE_VIEW_OVERLAY)
		{
		if (data->secondary_sameas_primary == MB_YES)
			svalue = data->secondary_data[k];
		else
			mbview_getsecondaryvalue(view, data, i, j, &svalue);
		if (svalue != data->secondary_nodatavalue)
			{
			intensity = view->sign * data->overlay_shade_magnitude 
				* (svalue - data->overlay_shade_center)
				/ (data->secondary_max - data->secondary_min);
			mbview_applyshade(intensity,
				&data->primary_r[k],
				&data->primary_g[k],
				&data->primary_b[k]);
			}
		}
	    }

	/* set color status bit */
	data->primary_stat_color[k/8] 
		= data->primary_stat_color[k/8] | statmask[k%8];

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
int mbview_colordata(int instance, int rez)
{
	/* local variables */
	char	*function_name = "mbview_colordata";
	int	status = MB_SUCCESS;
	int	stride;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	i, j, k, ii;
	double	value, svalue, factor, dd;
	double	xlon, ylat;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       rez:              %d\n",rez);
		}
		
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_colordata: %d %d\n", instance, rez);
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* set stride for looping over data */
	if (rez == MBV_REZ_FULL)
	    stride = 1;
	else if (rez == MBV_REZ_HIGH)
	    stride = MAX((int)ceil(((double)data->primary_nx) 
				/ ((double)data->hirez_dimension)), 
			(int)ceil(((double)data->primary_ny) 
				/ ((double)data->hirez_dimension)));
	else
	    stride = MAX((int)ceil(((double)data->primary_nx) 
				/ ((double)data->lorez_dimension)), 
			(int)ceil(((double)data->primary_ny) 
				/ ((double)data->lorez_dimension)));
		
	/* color the data */
	for (i=0;i<data->primary_nx;i+=stride)
		{
		for (j=0;j<data->primary_ny;j+=stride)
			{
			k = i * data->primary_ny + j;

			if (data->primary_data[k] != data->primary_nodatavalue
				&& !(data->primary_stat_color[k/8] & statmask[k%8]))
				{
				mbview_colorpoint(view, data, i, j, k);
				}
			}

		/* check for pending event */
		if (view->plot_done == MB_NO 
			&& view->plot_interrupt_allowed == MB_YES 
			&& i % MBV_EVENTCHECKCOARSENESS == 0)
			do_mbview_xevents();

		/* dump out of loop if plotting already done at a higher recursion */
		if (view->plot_done == MB_YES)
			i = data->primary_nx;
		}

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
int mbview_getsecondaryvalue(struct mbview_world_struct *view,
				struct mbview_struct *data,
				int i, int j, 
				double *secondary_value)
{
	/* local variables */
	char	*function_name = "mbview_getsecondaryvalue";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	double	xlon, ylat;
	double	xgrid, ygrid;
	double	xsgrid, ysgrid;
	int	ii, jj, kk;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       view:             %d\n",view);
		fprintf(stderr,"dbg2       data:             %d\n",data);
		fprintf(stderr,"dbg2       i:                %d\n",i);
		fprintf(stderr,"dbg2       j:                %d\n",j);
		}
	
	/* get position in primary grid */
	xgrid = data->primary_xmin + i * data->primary_dx;
	ygrid = data->primary_ymin + j * data->primary_dy;
		
	/* get lon and lat of desired position */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED
		|| data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		mb_proj_inverse(mbv_verbose, view->primary_pjptr,
				xgrid, ygrid, &xlon, &ylat, &error);
		}
	else
		{
		xlon = xgrid;
		ylat = ygrid;
		}

	/* get position in secondary grid coordinates */
	if (data->secondary_grid_projection_mode == MBV_PROJECTION_PROJECTED
		|| data->secondary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		mb_proj_forward(mbv_verbose, view->secondary_pjptr,
				xlon, ylat, &xsgrid, &ysgrid, &error);
		}
	else
		{
		xsgrid = xlon;
		ysgrid = ylat;
		}

	/* get rounded location in secondary grid */
	ii = (xsgrid - data->secondary_xmin) / data->secondary_dx;
	jj = (ysgrid - data->secondary_ymin) / data->secondary_dy;
	
	/* answer only defined within grid bounds */
	if (ii < 0 || ii >= data->secondary_nx
		|| jj < 0 || jj > data->secondary_ny)
		{
		*secondary_value = data->secondary_nodatavalue;
		}
	else
		{
		kk = ii * data->secondary_ny + jj;
		*secondary_value = data->secondary_data[kk];
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       secondary_value:  %f\n",*secondary_value);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_getcolor(double value, double min, double max,
			int colortablemode, 
			float *colortable_red,
			float *colortable_green,
			float *colortable_blue,
			float *red, float *green, float *blue)
{
	/* local variables */
	char	*function_name = "mbview_getcolor";
	int	status = MB_SUCCESS;
	int	ii;
	double	ff, factor;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       value:            %f\n",value);
		fprintf(stderr,"dbg2       min:              %f\n",min);
		fprintf(stderr,"dbg2       max:              %f\n",max);
		fprintf(stderr,"dbg2       colortablemode:   %d\n",colortablemode);
		fprintf(stderr,"dbg2       colortable_red:   %d\n",colortable_red);
		fprintf(stderr,"dbg2       colortable_green: %d\n",colortable_green);
		fprintf(stderr,"dbg2       colortable_blue:  %d\n",colortable_blue);
		}
		
	/* get color */
	if (colortablemode == MBV_COLORTABLE_NORMAL)
		factor = (max - value) / (max - min);
	else
		factor = (value -  min) / (max - min);
	factor = MAX(factor, 0.000001);
	factor = MIN(factor, 0.999999);
	ii = (int) (factor * (MBV_NUM_COLORS - 1));
	ff = factor * (MBV_NUM_COLORS - 1) - ii;
	*red = colortable_red[ii]
		+ ff * (colortable_red[ii+1] - colortable_red[ii]);
	*green = colortable_green[ii]
		+ ff * (colortable_green[ii+1] - colortable_green[ii]);
	*blue = colortable_blue[ii]
		+ ff * (colortable_blue[ii+1] - colortable_blue[ii]);

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       red:     %f\n",*red);
		fprintf(stderr,"dbg2       green:   %f\n",*green);
		fprintf(stderr,"dbg2       blue:    %f\n",*blue);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_applyshade(double intensity, float *r, float *g, float *b)
{
	/* note - this correction algorithm is taken from the GMT Technical
	   Reference and Cookbook by Wessel and Smith - you can find it in
	   Appendix I: Color Space - The final frontier */
	   
	/* local variables */
	char	*function_name = "mbview_applyshade";
	int	status = MB_SUCCESS;
	double	h, s, v;
	double	vmax, vmin, dv, idv;
	double	rmod, gmod, bmod;
	double	f, p, q, t;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       intensity:           %f\n",intensity);
		}
	
	/* change the initial rgb into hsv space */
	vmax = MAX (MAX (*r, *g), *b);
	vmin = MIN (MIN (*r, *g), *b);
	dv = vmax - vmin;
	v = vmax;
	if (vmax == 0.0)
		s = 0.0;
	else
		s = dv / vmax;
	h = 0.0;
	if (s > 0.0)
		{
		idv = 1.0 / dv;
		rmod = (vmax - *r) * idv;
		gmod = (vmax - *g) * idv;
		bmod = (vmax - *b) * idv;
		if (*r == vmax)
			h = bmod - gmod;
		else if (*g == vmax)
			h = 2.0 + rmod - bmod;
		else
			h = 4.0 + gmod - rmod;
		h *= 60.0;
		if (h < 0.0) 
			h += 360.0;
		}

	/* apply the shade to the color */
	if (intensity > 0) 
		{
		if (s != 0.0) 
			s = (1.0 - intensity) * s 
				+ intensity * 0.1;
		v = (1.0 - intensity) * v 
			+ intensity;
		}
	else 
		{
		if (s != 0.0) 
			s = (1.0 + intensity) * s 
				- intensity;
		v = (1.0 + intensity) * v 
			- intensity * 0.3;
		}
	if (v < 0.0) 
		v = 0.0;
	if (s < 0.0) 
		s = 0.0;
	if (v > 1.0) 
		v = 1.0;
	if (s > 1.0) 
		s = 1.0;
	
	/* change the corrected hsv values back into rgb */
	if (s == 0.0)
		{
		*r = v;
		*g = v;
		*b = v;
		}
	else
		{
		while (h >= 360.0) 
			h -= 360.0;
		h /= 60.0;
		f = h - ((int)h);
		p = v * (1.0 - s);
		q = v * (1.0 - (s * f));
		t = v * (1.0 - (s * (1.0 - f)));
		switch (((int)h)) 
			{
			case 0:
				*r = v;	
				*g = t;	
				*b = p;
				break;
			case 1:
				*r = q;	
				*g = v;	
				*b = p;
				break;
			case 2:
				*r = p;	
				*g = v;	
				*b = t;
				break;
			case 3:
				*r = p;	
				*g = q;	
				*b = v;
				break;
			case 4:
				*r = t;	
				*g = p;	
				*b = v;
				break;
			case 5:
				*r = v;	
				*g = p;	
				*b = q;
				break;
			}
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       red:     %d\n",*r);
		fprintf(stderr,"dbg2       green:   %d\n",*g);
		fprintf(stderr,"dbg2       blue:    %d\n",*b);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}
/*------------------------------------------------------------------------------*/
int mbview_contour(int instance, int rez)
{
	/* local variables */
	char	*function_name = "mbview_contour";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	i, j, k, l, kk;
	int	stride;
	int	vertex[4];
	int	triangleA, triangleB;
	int	nlevel, level_min, level_max;
	int	nvertex, nside;
	float	level_value, datamin, datamax;
	float	factor;
	float	xx[2], yy[2], zz;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       rez:              %d\n",rez);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set stride for looping over data */
	if (rez == MBV_REZ_FULL)
	    	stride = 1;
	else if (rez == MBV_REZ_HIGH)
	    	stride = MAX((int)ceil(((double)data->primary_nx) 
				/ ((double)data->hirez_dimension)), 
			(int)ceil(((double)data->primary_ny) 
				/ ((double)data->hirez_dimension)));
	else
	    	stride = MAX((int)ceil(((double)data->primary_nx) 
				/ ((double)data->lorez_dimension)), 
			(int)ceil(((double)data->primary_ny) 
				/ ((double)data->lorez_dimension)));

	/* start openGL list */
	if (rez == MBV_REZ_FULL)
		{
	    	glNewList((GLuint)(3*instance+3), GL_COMPILE);
		}
	else if (rez == MBV_REZ_HIGH)
		{
	    	glNewList((GLuint)(3*instance+2), GL_COMPILE);
		}
	else
		{
	    	glNewList((GLuint)(3*instance+1), GL_COMPILE);
		}
	glColor3f(0.0, 0.0, 0.0);
	glLineWidth(1.0);
	glBegin(GL_LINES);
	
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_contour: instance:%d rez:%d stride:%d contour interval:%f\n",
instance, rez, stride, data->contour_interval);

	/* construct the contour segments in each triangle */
	for (i=0;i<data->primary_nx-stride;i+=stride)
	{
	for (j=0;j<data->primary_ny-stride;j+=stride)
		{
		/* get vertex id's */
		vertex[0] = i * data->primary_ny + j;
		vertex[1] = (i + stride) * data->primary_ny + j;
	        vertex[2] = i * data->primary_ny + j + stride;
		vertex[3] = (i + stride) * data->primary_ny + j + stride;
		
		/* check if either triangle can be contoured */
		triangleA = MB_NO;
		triangleB = MB_NO;
		if (data->primary_data[vertex[0]] != data->primary_nodatavalue
			&& data->primary_data[vertex[1]] != data->primary_nodatavalue
			&& data->primary_data[vertex[2]] != data->primary_nodatavalue)
			triangleA = MB_YES;
		if (data->primary_data[vertex[1]] != data->primary_nodatavalue
			&& data->primary_data[vertex[3]] != data->primary_nodatavalue
			&& data->primary_data[vertex[2]] != data->primary_nodatavalue)
			triangleB = MB_YES;
			
		/* if at least one triangle is valid, contour it */
		if (triangleA == MB_YES || triangleB == MB_YES)
			{
			/* get min max values and number of contours */
			nvertex = 0;
			datamin = 0.0;
			datamax = 0.0;
			for (kk=0;kk<4;kk++)
				{
				k = vertex[kk];
				if (data->primary_data[k] != data->primary_nodatavalue) 
					{
					if (nvertex == 0)
						{
						datamin = data->primary_data[k];
						datamax = data->primary_data[k];
						}
					else
						{
						datamin = MIN(datamin, data->primary_data[k]);
						datamax = MAX(datamax, data->primary_data[k]);
						}
					nvertex++;
					}
				}

			/* get start, end, and number of contour levels in contour_interval units */
			level_min = (int)ceil(datamin / data->contour_interval);
			level_max = (int)floor(datamax / data->contour_interval);
			nlevel = level_max - level_min + 1;
				
			/* now if contours are needed loop over the contour levels */
			if (nlevel > 0)
				{
				for (l=level_min;l<=level_max;l++)
					{
					level_value = l * data->contour_interval;
					
					/* deal with triangle A - vertexes 0, 1, and 2 */
					if (triangleA == MB_YES)
					{
					nside = 0;
					if ((data->primary_data[vertex[0]] > level_value
							&& data->primary_data[vertex[1]] < level_value)
						|| (data->primary_data[vertex[0]] < level_value
							&& data->primary_data[vertex[1]] > level_value))
						{
						factor = (level_value - data->primary_data[vertex[0]])
										/ (data->primary_data[vertex[1]]
											- data->primary_data[vertex[0]]);
						xx[nside] = data->primary_x[vertex[0]]
									+ factor * (data->primary_x[vertex[1]]
											- data->primary_x[vertex[0]]);
						yy[nside] = data->primary_y[vertex[0]]
									+ factor * (data->primary_y[vertex[1]]
											- data->primary_y[vertex[0]]);
						nside++;
						}
					if ((data->primary_data[vertex[1]] > level_value
							&& data->primary_data[vertex[2]] < level_value)
						|| (data->primary_data[vertex[1]] < level_value
							&& data->primary_data[vertex[2]] > level_value))
						{
						factor = (level_value - data->primary_data[vertex[1]])
										/ (data->primary_data[vertex[2]]
											- data->primary_data[vertex[1]]);
						xx[nside] = data->primary_x[vertex[1]]
									+ factor * (data->primary_x[vertex[2]]
											- data->primary_x[vertex[1]]);
						yy[nside] = data->primary_y[vertex[1]]
									+ factor * (data->primary_y[vertex[2]]
											- data->primary_y[vertex[1]]);
						nside++;
						}
					if (nside < 2 &&
						((data->primary_data[vertex[2]] > level_value
							&& data->primary_data[vertex[0]] < level_value)
						|| (data->primary_data[vertex[2]] < level_value
							&& data->primary_data[vertex[0]] > level_value)))
						{
						factor = (level_value - data->primary_data[vertex[2]])
										/ (data->primary_data[vertex[0]]
											- data->primary_data[vertex[2]]);
						xx[nside] = data->primary_x[vertex[2]]
									+ factor * (data->primary_x[vertex[0]]
											- data->primary_x[vertex[2]]);
						yy[nside] = data->primary_y[vertex[2]]
									+ factor * (data->primary_y[vertex[0]]
											- data->primary_y[vertex[2]]);
						nside++;
						}
					if (nside == 2)
						{
						zz = (float)(view->zscale * (level_value - view->zorigin) 
									+ MBV_OPENGL_3D_CONTOUR_OFFSET);
						glVertex3f(xx[0], yy[0], zz);
						glVertex3f(xx[1], yy[1], zz);
						}
					}
					
					/* deal with triangle B - vertexes 1, 3, and 2 */
					if (triangleB == MB_YES)
					{
					nside = 0;
					if ((data->primary_data[vertex[1]] > level_value
							&& data->primary_data[vertex[3]] < level_value)
						|| (data->primary_data[vertex[1]] < level_value
							&& data->primary_data[vertex[3]] > level_value))
						{
						factor = (level_value - data->primary_data[vertex[1]])
										/ (data->primary_data[vertex[3]]
											- data->primary_data[vertex[1]]);
						xx[nside] = data->primary_x[vertex[1]]
									+ factor * (data->primary_x[vertex[3]]
											- data->primary_x[vertex[1]]);
						yy[nside] = data->primary_y[vertex[1]]
									+ factor * (data->primary_y[vertex[3]]
											- data->primary_y[vertex[1]]);
						nside++;
						}
					if ((data->primary_data[vertex[3]] > level_value
							&& data->primary_data[vertex[2]] < level_value)
						|| (data->primary_data[vertex[3]] < level_value
							&& data->primary_data[vertex[2]] > level_value))
						{
						factor = (level_value - data->primary_data[vertex[3]])
										/ (data->primary_data[vertex[2]]
											- data->primary_data[vertex[3]]);
						xx[nside] = data->primary_x[vertex[3]]
									+ factor * (data->primary_x[vertex[2]]
											- data->primary_x[vertex[3]]);
						yy[nside] = data->primary_y[vertex[3]]
									+ factor * (data->primary_y[vertex[2]]
											- data->primary_y[vertex[3]]);
						nside++;
						}
					if (nside < 2 &&
						((data->primary_data[vertex[2]] > level_value
							&& data->primary_data[vertex[1]] < level_value)
						|| (data->primary_data[vertex[2]] < level_value
							&& data->primary_data[vertex[1]] > level_value)))
						{
						factor = (level_value - data->primary_data[vertex[2]])
										/ (data->primary_data[vertex[1]]
											- data->primary_data[vertex[2]]);
						xx[nside] = data->primary_x[vertex[2]]
									+ factor * (data->primary_x[vertex[1]]
											- data->primary_x[vertex[2]]);
						yy[nside] = data->primary_y[vertex[2]]
									+ factor * (data->primary_y[vertex[1]]
											- data->primary_y[vertex[2]]);
						nside++;
						}
					if (nside == 2)
						{
						zz = (float)(view->zscale * (level_value - view->zorigin) 
									+ MBV_OPENGL_3D_CONTOUR_OFFSET);
						glVertex3f(xx[0], yy[0], zz);
						glVertex3f(xx[1], yy[1], zz);
						}
					}
					
					}
				}
			}
		
		}
		
	/* check for pending event */
	if (view->plot_done == MB_NO 
		&& view->plot_interrupt_allowed == MB_YES 
		&& i % MBV_EVENTCHECKCOARSENESS == 0)
		do_mbview_xevents();
		
	/* dump out of loop if plotting already done at a higher recursion */
	if (view->plot_done == MB_YES)
		i = data->primary_nx;
	}
	
	/* end openGL list */
	glEnd();
	glEndList();

	/* set rez flag only if plotting not done */
	if (view->plot_done == MB_NO)
		{
		if (rez == MBV_REZ_FULL)
			{
			view->contourfullrez = MB_YES;
			}
		else if (rez == MBV_REZ_HIGH)
			{
			view->contourhirez = MB_YES;
			}
		else
			{
			view->contourlorez = MB_YES;
			}
		}

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
int mbview_getzdata(int instance, 
			double xgrid, double ygrid,
			int *found, double *zdata)
{

	/* local variables */
	char	*function_name = "mbview_getzdata";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	nsum;
	double	zdatasum;
	int	i,j, k, l, m, n;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xgrid:            %f\n",xgrid);
		fprintf(stderr,"dbg2       ygrid:            %f\n",ygrid);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get location in grid */
	i = (int)((xgrid - data->primary_xmin) / data->primary_dx);
	j = (int)((ygrid - data->primary_ymin) / data->primary_dy);
	
	/* fail if outside grid */
	if (i < 0 || i >= data->primary_nx - 1
		|| j < 0 || j >= data->primary_ny - 1)
		{
		*found = MB_NO;
		*zdata = 0.0;
		}
		
	/* check all four points and average the good ones */
	else
		{
		k = i * data->primary_ny + j;
		l = (i + 1) * data->primary_ny + j;
		m = i * data->primary_ny + j + 1;
		n = (i + 1) * data->primary_ny + j + 1;
		nsum = 0;
		zdatasum = 0.0;
		if (data->primary_data[k] != data->primary_nodatavalue)
			{
			zdatasum += data->primary_data[k];
			nsum++;
			}
		if (data->primary_data[l] != data->primary_nodatavalue)
			{
			zdatasum += data->primary_data[l];
			nsum++;
			}
		if (data->primary_data[m] != data->primary_nodatavalue)
			{
			zdatasum += data->primary_data[m];
			nsum++;
			}
		if (data->primary_data[n] != data->primary_nodatavalue)
			{
			zdatasum += data->primary_data[n];
			nsum++;
			}
		if (nsum > 0)
			{
			*zdata = zdatasum / nsum;
			*found = MB_YES;
			}
		else
			{
			*zdata = 0.0;
			*found = MB_NO;
			}
		}
	
	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       found:           %d\n",*found);
		fprintf(stderr,"dbg2       zdata:           %f\n",*zdata);
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
