/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_secondary.c	9/25/2003
 *
 *    Copyright (c) 2003-2023 by
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
 * Date:	September 25, 2003
 *
 * Note:	This code was broken out of mbview_callbacks.c, which was
 *		begun on October 7, 2002
 */
/*------------------------------------------------------------------------------*/

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
#include <Xm/TextF.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
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

static char value_text[MB_PATH_MAXLINE];

/*------------------------------------------------------------------------------*/
int mbview_setsecondarygrid(int verbose, size_t instance, int secondary_grid_projection_mode, char *secondary_grid_projection_id,
                            float secondary_nodatavalue, int secondary_n_columns, int secondary_n_rows, double secondary_min,
                            double secondary_max, double secondary_xmin, double secondary_xmax, double secondary_ymin,
                            double secondary_ymax, double secondary_dx, double secondary_dy, float *secondary_data, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       secondary_grid_projection_mode:   %d\n", secondary_grid_projection_mode);
		fprintf(stderr, "dbg2       secondary_grid_projection_id:     %s\n", secondary_grid_projection_id);
		fprintf(stderr, "dbg2       secondary_nodatavalue:       %f\n", secondary_nodatavalue);
		fprintf(stderr, "dbg2       secondary_n_columns:         %d\n", secondary_n_columns);
		fprintf(stderr, "dbg2       secondary_n_rows:            %d\n", secondary_n_rows);
		fprintf(stderr, "dbg2       secondary_min:               %f\n", secondary_min);
		fprintf(stderr, "dbg2       secondary_max:               %f\n", secondary_max);
		fprintf(stderr, "dbg2       secondary_xmin:              %f\n", secondary_xmin);
		fprintf(stderr, "dbg2       secondary_xmax:              %f\n", secondary_xmax);
		fprintf(stderr, "dbg2       secondary_ymin:              %f\n", secondary_ymin);
		fprintf(stderr, "dbg2       secondary_ymax:              %f\n", secondary_ymax);
		fprintf(stderr, "dbg2       secondary_dx:                %g\n", secondary_dx);
		fprintf(stderr, "dbg2       secondary_dy:                %g\n", secondary_dy);
		fprintf(stderr, "dbg2       secondary_data:              %p\n", secondary_data);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set values */
	data->secondary_grid_projection_mode = secondary_grid_projection_mode;
	strcpy(data->secondary_grid_projection_id, secondary_grid_projection_id);
	data->secondary_nodatavalue = secondary_nodatavalue;
	data->secondary_nxy = secondary_n_columns * secondary_n_rows;
	data->secondary_n_columns = secondary_n_columns;
	data->secondary_n_rows = secondary_n_rows;
	data->secondary_min = secondary_min;
	data->secondary_max = secondary_max;
	data->secondary_xmin = secondary_xmin;
	data->secondary_xmax = secondary_xmax;
	data->secondary_ymin = secondary_ymin;
	data->secondary_ymax = secondary_ymax;
	data->secondary_dx = secondary_dx;
	data->secondary_dy = secondary_dy;

	/* allocate required arrays */
	int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * data->secondary_nxy, (void **)&data->secondary_data, error);
	if (status != MB_SUCCESS) {
		fprintf(stderr, "\nUnable to allocate memory to store secondary grid data\n");
		fprintf(stderr, "\nProgram terminated in function <%s>.\n", __func__);
		exit(*error);
	}

	/* copy grid */
	memcpy(data->secondary_data, secondary_data, data->secondary_nxy * sizeof(float));

	/* check if secondary grid has same bounds and dimensions as primary grid so
	    that overlay calculations are trivial */
	if (data->secondary_n_columns == data->primary_n_columns && data->secondary_n_rows == data->primary_n_rows &&
	    (fabs(data->secondary_xmin - data->primary_xmin) < 0.1 * data->primary_dx) &&
	    (fabs(data->secondary_xmax - data->primary_xmax) < 0.1 * data->primary_dx) &&
	    (fabs(data->secondary_ymin - data->primary_ymin) < 0.1 * data->primary_dy) &&
	    (fabs(data->secondary_ymax - data->primary_ymax) < 0.1 * data->primary_dy))
		data->secondary_sameas_primary = true;
	else
		data->secondary_sameas_primary = false;

	/* set projection for secondary grid if needed */
	if (data->secondary_nxy > 0 && data->secondary_grid_projection_mode == MBV_PROJECTION_PROJECTED) {
		/* set projection for getting lon lat */
		const int proj_status = mb_proj_init(verbose, data->secondary_grid_projection_id, &(view->secondary_pjptr), error);
		if (proj_status == MB_SUCCESS)
			view->secondary_pj_init = true;

		/* quit if projection fails */
		if (proj_status != MB_SUCCESS) {
			char *message;
			mb_error(verbose, *error, &message);
			fprintf(stderr, "\nMBIO Error initializing projection:\n%s\n", message);
			fprintf(stderr, "\nProgram terminated in <%s>\n", __func__);
			mb_memory_clear(mbv_verbose, error);
			exit(*error);
		}
	}

	/* reset histogram flag */
	view->secondary_histogram_set = false;

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
int mbview_updatesecondarygrid(int verbose, size_t instance, int secondary_n_columns, int secondary_n_rows, float *secondary_data,
                               int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                      %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                     %zu\n", instance);
		fprintf(stderr, "dbg2       secondary_n_columns:          %d\n", secondary_n_columns);
		fprintf(stderr, "dbg2       secondary_n_rows:             %d\n", secondary_n_rows);
		fprintf(stderr, "dbg2       secondary_data:               %p\n", secondary_data);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);


	/* set value */
	if (secondary_n_columns == data->secondary_n_columns && secondary_n_rows == data->secondary_n_rows) {
		bool first = true;
		for (int k = 0; k < data->secondary_n_columns * data->secondary_n_rows; k++) {
			data->secondary_data[k] = secondary_data[k];
			if (first && secondary_data[k] != data->secondary_nodatavalue) {
				data->secondary_min = data->secondary_data[k];
				data->secondary_max = data->secondary_data[k];
				first = false;
			}
			else if (secondary_data[k] != data->secondary_nodatavalue) {
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
	view->secondary_histogram_set = false;

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
int mbview_updatesecondarygridcell(int verbose, size_t instance, int secondary_ix, int secondary_jy, float value, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                      %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                     %zu\n", instance);
		fprintf(stderr, "dbg2       secondary_ix:                 %d\n", secondary_ix);
		fprintf(stderr, "dbg2       secondary_jy:                 %d\n", secondary_jy);
		fprintf(stderr, "dbg2       value:                        %f\n", value);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set value */
	if (secondary_ix >= 0 && secondary_ix < data->secondary_n_columns && secondary_jy >= 0 && secondary_jy < data->secondary_n_rows) {
		/* update the cell value */
		const int k = secondary_ix * data->secondary_n_rows + secondary_jy;
		data->secondary_data[k] = value;
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
int mbview_setsecondarycolortable(int verbose, size_t instance, int secondary_colortable, int secondary_colortable_mode,
                                  double secondary_colortable_min, double secondary_colortable_max,
                                  double overlay_shade_magnitude, double overlay_shade_center, int overlay_shade_mode, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       secondary_colortable:      %d\n", secondary_colortable);
		fprintf(stderr, "dbg2       secondary_colortable_mode: %d\n", secondary_colortable_mode);
		fprintf(stderr, "dbg2       secondary_colortable_min:  %f\n", secondary_colortable_min);
		fprintf(stderr, "dbg2       secondary_colortable_max:  %f\n", secondary_colortable_max);
		fprintf(stderr, "dbg2       overlay_shade_magnitude:   %f\n", overlay_shade_magnitude);
		fprintf(stderr, "dbg2       overlay_shade_center:      %f\n", overlay_shade_center);
		fprintf(stderr, "dbg2       overlay_shade_mode:        %d\n", overlay_shade_mode);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set values */
	data->secondary_colortable = secondary_colortable;
	data->secondary_colortable_mode = secondary_colortable_mode;
	data->secondary_colortable_min = secondary_colortable_min;
	data->secondary_colortable_max = secondary_colortable_max;
	data->overlay_shade_magnitude = overlay_shade_magnitude;
	data->overlay_shade_center = overlay_shade_center;
	data->overlay_shade_mode = overlay_shade_mode;

	/* set secondary color control widgets if managed */
	if (XtIsManaged(view->mb3dview.mbview_textField_overlaymin)) {
		sprintf(value_text, "%g", data->secondary_colortable_min);
		XmTextFieldSetString(view->mb3dview.mbview_textField_overlaymin, value_text);
		sprintf(value_text, "%g", data->secondary_colortable_max);
		XmTextFieldSetString(view->mb3dview.mbview_textField_overlaymax, value_text);
		if (data->secondary_colortable_mode == MBV_COLORTABLE_NORMAL) {
			XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_ctoh, TRUE, TRUE);
		}
		else {
			XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_htoc, TRUE, TRUE);
		}
	}
	if (XtIsManaged(view->mb3dview.mbview_textField_overlay_amp)) {
		sprintf(value_text, "%g", data->overlay_shade_magnitude);
		XmTextFieldSetString(view->mb3dview.mbview_textField_overlay_amp, value_text);
		sprintf(value_text, "%g", data->overlay_shade_center);
		XmTextFieldSetString(view->mb3dview.mbview_textField_overlay_center, value_text);
		if (data->overlay_shade_mode == MBV_COLORTABLE_NORMAL) {
			XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_shade_ctoh, TRUE, TRUE);
		}
		else {
			XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_shade_htoc, TRUE, TRUE);
		}
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

int mbview_setsecondaryname(int verbose, size_t instance, char *name, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       name:                      %s\n", name);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* set secondary grid labels */
	if (XtIsManaged(view->mb3dview.mbview_toggleButton_data_secondary)) {
		Boolean argok = False;
		Cardinal ac = 0;
		XmString tmp0 = (XmString)BX_CONVERT(view->mb3dview.mbview_toggleButton_data_secondary, (char *)name, XmRXmString, 0, &argok);
		Arg args[256];
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetValues(view->mb3dview.mbview_toggleButton_data_secondary, args, ac);
		XmStringFree((XmString)tmp0);

		ac = 0;
		sprintf(value_text, "Shading by %s", name);
		tmp0 = (XmString)BX_CONVERT(view->mb3dview.mbview_toggleButton_overlay_secondary, (char *)value_text, XmRXmString, 0,
		                            &argok);
		XtSetArg(args[ac], XmNlabelString, tmp0);
		if (argok)
			ac++;
		XtSetValues(view->mb3dview.mbview_toggleButton_overlay_secondary, args, ac);
		XmStringFree((XmString)tmp0);
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
