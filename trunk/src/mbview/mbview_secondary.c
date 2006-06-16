/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_secondary.c	9/25/2003
 *    $Id: mbview_secondary.c,v 5.6 2006-06-16 19:30:58 caress Exp $
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
 * Revision 5.5  2006/01/24 19:21:32  caress
 * Version 5.0.8 beta.
 *
 * Revision 5.4  2005/11/05 01:11:47  caress
 * Much work over the past two months.
 *
 * Revision 5.3  2005/02/18 07:32:56  caress
 * Fixed nav display and button sensitivity.
 *
 * Revision 5.2  2005/02/08 22:37:43  caress
 * Heading towards 5.0.6 release.
 *
 * Revision 5.0  2003/12/02 20:38:32  caress
 * Making version number 5.0
 *
 * Revision 1.2  2003/11/25 01:43:19  caress
 * MBview version generated during EW0310.
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
static Cardinal 	ac;
static Arg      	args[256];
static char		value_text[MB_PATH_MAXLINE];

static char rcs_id[]="$Id: mbview_secondary.c,v 5.6 2006-06-16 19:30:58 caress Exp $";

/*------------------------------------------------------------------------------*/
int mbview_setsecondarygrid(int verbose, int instance,
			int	secondary_grid_projection_mode,
			char	*secondary_grid_projection_id,
			float	secondary_nodatavalue,
			int	secondary_nx,
			int	secondary_ny,
			double	secondary_min,
			double	secondary_max,
			double	secondary_xmin,
			double	secondary_xmax,
			double	secondary_ymin,
			double	secondary_ymax,
			double	secondary_dx,
			double	secondary_dy,
			float	*secondary_data,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_setsecondarygrid";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	proj_status;
	char	*message;

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %d\n", instance);
		fprintf(stderr,"dbg2       secondary_grid_projection_mode:   %d\n", secondary_grid_projection_mode);
		fprintf(stderr,"dbg2       secondary_grid_projection_id:     %s\n", secondary_grid_projection_id);
		fprintf(stderr,"dbg2       secondary_nodatavalue:       %d\n", secondary_nodatavalue);
		fprintf(stderr,"dbg2       secondary_nx:                %d\n", secondary_nx);
		fprintf(stderr,"dbg2       secondary_ny:                %d\n", secondary_ny);
		fprintf(stderr,"dbg2       secondary_min:               %f\n", secondary_min);
		fprintf(stderr,"dbg2       secondary_max:               %f\n", secondary_max);
		fprintf(stderr,"dbg2       secondary_xmin:              %f\n", secondary_xmin);
		fprintf(stderr,"dbg2       secondary_xmax:              %f\n", secondary_xmax);
		fprintf(stderr,"dbg2       secondary_ymin:              %f\n", secondary_ymin);
		fprintf(stderr,"dbg2       secondary_ymax:              %f\n", secondary_ymax);
		fprintf(stderr,"dbg2       secondary_dx:                %f\n", secondary_dx);
		fprintf(stderr,"dbg2       secondary_dy:                %f\n", secondary_dy);
		fprintf(stderr,"dbg2       secondary_data:              %d\n", secondary_data);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* set values */
        data->secondary_grid_projection_mode = secondary_grid_projection_mode;
        strcpy(secondary_grid_projection_id, secondary_grid_projection_id);
        data->secondary_nodatavalue = secondary_nodatavalue;
        data->secondary_nxy = secondary_nx * secondary_ny;
        data->secondary_nx = secondary_nx;
        data->secondary_ny = secondary_ny;
        data->secondary_min = secondary_min;
        data->secondary_max = secondary_max;
        data->secondary_xmin = secondary_xmin;
        data->secondary_xmax = secondary_xmax;
        data->secondary_ymin = secondary_ymin;
        data->secondary_ymax = secondary_ymax;
        data->secondary_dx = secondary_dx;
        data->secondary_dy = secondary_dy;
	
	/* allocate required arrays */
    	status = mb_malloc(verbose, sizeof(float) * data->secondary_nxy, 
    				&data->secondary_data, error);
	if (status != MB_SUCCESS)
	    {
	    fprintf(stderr,"\nUnable to allocate memory to store secondary grid data\n");
	    fprintf(stderr,"\nProgram terminated in function <%s>.\n",
		    function_name);
	    exit(*error);
	    }
	
	/* copy grid */
	memcpy(data->secondary_data, secondary_data, data->secondary_nxy * sizeof(float));
	
	/* check if secondary grid has same bounds and dimensions as primary grid so
		that overlay calculations are trivial */
	if (data->secondary_nx == data->primary_nx
		&& data->secondary_ny == data->primary_ny
		&& (fabs(data->secondary_xmin - data->primary_xmin) < 0.1 * data->primary_dx)
		&& (fabs(data->secondary_xmax - data->primary_xmax) < 0.1 * data->primary_dx)
		&& (fabs(data->secondary_ymin - data->primary_ymin) < 0.1 * data->primary_dy)
		&& (fabs(data->secondary_ymax - data->primary_ymax) < 0.1 * data->primary_dy))
		data->secondary_sameas_primary = MB_YES;
	else
		data->secondary_sameas_primary = MB_NO;

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
fprintf(stderr,"SECONDARY GRID PROJECTION:%d %d %s\n",
view->secondary_pj_init,view->secondary_pjptr,data->secondary_grid_projection_id);
			
		/* quit if projection fails */
		if (proj_status != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error initializing projection:\n%s\n",
				message);
			fprintf(stderr,"\nProgram terminated in <%s>\n",
				function_name);
			mb_memory_clear(mbv_verbose, &error);
			exit(error);
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
int mbview_setsecondarycolortable(int verbose, int instance,
			int	secondary_colortable,
			int	secondary_colortable_mode,
			double	secondary_colortable_min,
			double	secondary_colortable_max,
			double	overlay_shade_magnitude,
			double	overlay_shade_center,
			int	overlay_shade_mode,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_setsecondarycolortable";
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
		fprintf(stderr,"dbg2       instance:                  %d\n", instance);
		fprintf(stderr,"dbg2       secondary_colortable:      %d\n", secondary_colortable);
		fprintf(stderr,"dbg2       secondary_colortable_mode: %d\n", secondary_colortable_mode);
		fprintf(stderr,"dbg2       secondary_colortable_min:  %f\n", secondary_colortable_min);
		fprintf(stderr,"dbg2       secondary_colortable_max:  %f\n", secondary_colortable_max);
		fprintf(stderr,"dbg2       overlay_shade_magnitude:   %f\n", overlay_shade_magnitude);
		fprintf(stderr,"dbg2       overlay_shade_center:      %f\n", overlay_shade_center);
		fprintf(stderr,"dbg2       overlay_shade_mode:        %d\n", overlay_shade_mode);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* set values */
        data->secondary_colortable = secondary_colortable;
        data->secondary_colortable_mode = secondary_colortable_mode;
        data->secondary_colortable_min = secondary_colortable_min;
        data->secondary_colortable_max = secondary_colortable_max;
        data->overlay_shade_magnitude = overlay_shade_magnitude;
        data->overlay_shade_center = overlay_shade_center;
        data->overlay_shade_mode = overlay_shade_mode;

	/* set secondary color control widgets if managed */
	if (XtIsManaged(view->mb3dview.mbview_textField_overlaymin))
		{
		sprintf(value_text,"%g", data->secondary_colortable_min);
		XmTextFieldSetString(view->mb3dview.mbview_textField_overlaymin, 
				value_text);
		sprintf(value_text,"%g", data->secondary_colortable_max);
		XmTextFieldSetString(view->mb3dview.mbview_textField_overlaymax, 
				value_text);
		if (data->secondary_colortable_mode == MBV_COLORTABLE_NORMAL)
			{
	    		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_ctoh, 
				TRUE, TRUE);
			}
		else
			{
	    		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_htoc, 
				TRUE, TRUE);
			}
		}
	if (XtIsManaged(view->mb3dview.mbview_textField_overlay_amp))
		{
		sprintf(value_text,"%g", data->overlay_shade_magnitude);
		XmTextFieldSetString(view->mb3dview.mbview_textField_overlay_amp, 
				value_text);
		sprintf(value_text,"%g", data->overlay_shade_center);
		XmTextFieldSetString(view->mb3dview.mbview_textField_overlay_center, 
				value_text);
		if (data->overlay_shade_mode == MBV_COLORTABLE_NORMAL)
			{
	    		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_shade_ctoh, 
				TRUE, TRUE);
			}
		else
			{
	    		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_shade_htoc, 
				TRUE, TRUE);
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

