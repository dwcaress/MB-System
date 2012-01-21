/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_secondary.c	9/25/2003
 *    $Id: mbview_secondary.c 1891 2011-05-04 23:46:30Z caress $
 *
 *    Copyright (c) 2003-2011 by
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
 * $Log: mbview_secondary.c,v $
 * Revision 5.10  2008/05/16 22:59:42  caress
 * Release 5.1.1beta18.
 *
 * Revision 5.9  2007/10/08 16:32:08  caress
 * Code status as of 8 October 2007.
 *
 * Revision 5.8  2007/06/17 23:27:30  caress
 * Added NBeditviz.
 *
 * Revision 5.7  2006/06/22 04:45:43  caress
 * Working towards 5.1.0
 *
 * Revision 5.6  2006/06/16 19:30:58  caress
 * Check in after the Santa Monica Basin Mapping AUV Expedition.
 *
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
#include <Xm/TextF.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
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
static char		value_text[MB_PATH_MAXLINE];
static char rcs_id[]="$Id: mbview_secondary.c 1891 2011-05-04 23:46:30Z caress $";

/*------------------------------------------------------------------------------*/
int mbview_setsecondarygrid(int verbose, size_t instance,
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
		fprintf(stderr,"dbg2       instance:                  %ld\n", instance);
		fprintf(stderr,"dbg2       secondary_grid_projection_mode:   %d\n", secondary_grid_projection_mode);
		fprintf(stderr,"dbg2       secondary_grid_projection_id:     %s\n", secondary_grid_projection_id);
		fprintf(stderr,"dbg2       secondary_nodatavalue:       %f\n", secondary_nodatavalue);
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
		fprintf(stderr,"dbg2       secondary_data:              %lu\n", (size_t)secondary_data);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* set values */
        data->secondary_grid_projection_mode = secondary_grid_projection_mode;
        strcpy(data->secondary_grid_projection_id, secondary_grid_projection_id);
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
    	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * data->secondary_nxy, 
    				(void **)&data->secondary_data, error);
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
					error);
		if (proj_status == MB_SUCCESS)
			view->secondary_pj_init = MB_YES;
/*fprintf(stderr,"SECONDARY GRID PROJECTION:%d %ld %s\n",
view->secondary_pj_init,(size_t)view->secondary_pjptr,data->secondary_grid_projection_id);*/
			
		/* quit if projection fails */
		if (proj_status != MB_SUCCESS)
			{
			mb_error(verbose,*error,&message);
			fprintf(stderr,"\nMBIO Error initializing projection:\n%s\n",
				message);
			fprintf(stderr,"\nProgram terminated in <%s>\n",
				function_name);
			mb_memory_clear(mbv_verbose, error);
			exit(*error);
			}
		}
		
	/* reset histogram flag */
	view->secondary_histogram_set = MB_NO;
		
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
int mbview_updatesecondarygrid(int verbose, size_t instance,
			int	secondary_nx,
			int	secondary_ny,
			float	*secondary_data,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_updatesecondarygrid";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	first;
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
		fprintf(stderr,"dbg2       instance:                     %ld\n", instance);
		fprintf(stderr,"dbg2       secondary_nx:                 %d\n", secondary_nx);
		fprintf(stderr,"dbg2       secondary_ny:                 %d\n", secondary_ny);
		fprintf(stderr,"dbg2       secondary_data:               %lu\n", (size_t)secondary_data);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* set value */
	if (secondary_nx == data->secondary_nx
		&& secondary_ny == data->secondary_ny)
		{
		first = MB_YES;
		for (k=0;k<data->secondary_nx*data->secondary_ny;k++)
			{
			data->secondary_data[k] = secondary_data[k];
			if (first == MB_YES && secondary_data[k] != data->secondary_nodatavalue)
				{
				data->secondary_min = data->secondary_data[k];
				data->secondary_max = data->secondary_data[k];
				first = MB_NO;	
				}
			else if (secondary_data[k] != data->secondary_nodatavalue)
				{
				data->secondary_min = MIN(data->secondary_min, data->secondary_data[k]);
				data->secondary_max = MAX(data->secondary_max, data->secondary_data[k]);
				}
			}
		}
		
	/* reset plotting and colors */
	view->lastdrawrez = MBV_REZ_NONE;
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* reset histogram flag */
	view->secondary_histogram_set = MB_NO;
		
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
int mbview_updatesecondarygridcell(int verbose, size_t instance,
			int	secondary_ix,
			int	secondary_jy,
			float	value,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_setsecondarygrid";
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
		fprintf(stderr,"dbg2       instance:                     %ld\n", instance);
		fprintf(stderr,"dbg2       secondary_ix:                 %d\n", secondary_ix);
		fprintf(stderr,"dbg2       secondary_jy:                 %d\n", secondary_jy);
		fprintf(stderr,"dbg2       value:                        %f\n", value);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* set value */
	if (secondary_ix >= 0 && secondary_ix < data->secondary_nx
		&& secondary_jy >= 0 && secondary_jy < data->secondary_ny)
		{
		/* update the cell value */
		k = secondary_ix * data->secondary_ny + secondary_jy;
		data->secondary_data[k] = value;
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
int mbview_setsecondarycolortable(int verbose, size_t instance,
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
		fprintf(stderr,"dbg2       instance:                  %ld\n", instance);
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

int mbview_setsecondaryname(int verbose, size_t instance,
				char *name, int *error)

{
	/* local variables */
	char	*function_name = "mbview_setsecondaryname";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
        XmString    tmp0;
	Cardinal ac = 0;
	Arg      args[256];
	Boolean  argok = False;

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %ld\n", instance);
		fprintf(stderr,"dbg2       name:                      %s\n", name);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* set secondary grid labels */
 	if (XtIsManaged(view->mb3dview.mbview_toggleButton_data_secondary))
		{
		ac = 0;
        	tmp0 = (XmString) BX_CONVERT(view->mb3dview.mbview_toggleButton_data_secondary, (char *)name, 
                	XmRXmString, 0, &argok);
        	XtSetArg(args[ac], XmNlabelString, tmp0); if (argok) ac++;
		XtSetValues(view->mb3dview.mbview_toggleButton_data_secondary, args, ac);
        	XmStringFree((XmString)tmp0);

		ac = 0;
		sprintf(value_text, "Shading by %s", name);
        	tmp0 = (XmString) BX_CONVERT(view->mb3dview.mbview_toggleButton_overlay_secondary, (char *)value_text, 
                	XmRXmString, 0, &argok);
        	XtSetArg(args[ac], XmNlabelString, tmp0); if (argok) ac++;
		XtSetValues(view->mb3dview.mbview_toggleButton_overlay_secondary, args, ac);
        	XmStringFree((XmString)tmp0);
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

