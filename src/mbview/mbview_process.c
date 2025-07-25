/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_process.c	9/25/2003
 *
 *    Copyright (c) 2003-2025 by
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

#include <ctype.h>
#include <math.h>
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
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"

/* OpenGL include files */
#include <GL/gl.h>
#include <GL/glu.h>
#ifndef WIN32
#include <GL/glx.h>
#endif
#include "mb_glwdrawa.h"

#include "mbview.h"
#include "mbviewprivate.h"

/*------------------------------------------------------------------------------*/
int mbview_projectdata(size_t instance) {
	int error = MB_ERROR_NO_ERROR;
	int proj_status = MB_SUCCESS;
	double xgrid, ygrid, xlon, ylat, xdisplay, ydisplay, zdisplay;
	double xlonmin, xlonmax, ylatmin, ylatmax;
	int i, j, k;
	char *message;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}
	if (mbv_verbose >= 2)
		fprintf(stderr, "mbview_projectdata: %zu\n", instance);

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* delete old projections if necessary */
	if (view->primary_pj_init && view->primary_pjptr != NULL) {
		mb_proj_free(mbv_verbose, &(view->primary_pjptr), &error);
		view->primary_pj_init = false;
	}
	if (view->secondary_pj_init && view->secondary_pjptr != NULL) {
		mb_proj_free(mbv_verbose, &(view->secondary_pjptr), &error);
		view->secondary_pj_init = false;
	}
	if (view->display_pj_init && view->display_pjptr != NULL) {
		mb_proj_free(mbv_verbose, &(view->display_pjptr), &error);
		view->display_pj_init = false;
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
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED &&
	    data->display_projection_mode == MBV_PROJECTION_PROJECTED &&
	    strcmp(data->primary_grid_projection_id, data->display_projection_id) == 0) {
		/* reset modes */
		data->primary_grid_projection_mode = MBV_PROJECTION_ALREADYPROJECTED;
		data->display_projection_mode = MBV_PROJECTION_ALREADYPROJECTED;

		/* get bounds */
		view->xmin = data->primary_xmin;
		view->xmax = data->primary_xmax;
		view->ymin = data->primary_ymin;
		view->ymax = data->primary_ymax;

		/* get origin */
		view->xorigin = 0.5 * (view->xmin + view->xmax);
		view->yorigin = 0.5 * (view->ymin + view->ymax);
		view->zorigin = data->exageration * 0.5 * (data->primary_min + data->primary_max);

		/* set projection for getting lon lat */
		proj_status = mb_proj_init(mbv_verbose, data->primary_grid_projection_id, &(view->primary_pjptr), &error);
		if (proj_status == MB_SUCCESS) {
			view->primary_pj_init = true;
			proj_status = mb_proj_init(mbv_verbose, data->display_projection_id, &(view->display_pjptr), &error);
			if (proj_status == MB_SUCCESS)
				view->display_pj_init = true;
		}

		/* quit if projection fails */
		if (proj_status != MB_SUCCESS) {
			mb_error(mbv_verbose, error, &message);
			fprintf(stderr, "\nMBIO Error initializing projection:\n%s\n", message);
			fprintf(stderr, "\nProgram terminated in <%s>\n", __func__);
			mb_memory_clear(mbv_verbose, &error);
			exit(error);
		}
	}

	/* else set up projections as needed */
	else {
		/* first go from grid coordinates to lon lat */
		if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED) {
			/* set projection */
			proj_status = mb_proj_init(mbv_verbose, data->primary_grid_projection_id, &(view->primary_pjptr), &error);
			if (proj_status == MB_SUCCESS)
				view->primary_pj_init = true;

			/* quit if projection fails */
			if (proj_status != MB_SUCCESS) {
				mb_error(mbv_verbose, error, &message);
				fprintf(stderr, "\nMBIO Error initializing projection:\n%s\n", message);
				fprintf(stderr, "\nProgram terminated in <%s>\n", __func__);
				mb_memory_clear(mbv_verbose, &error);
				exit(error);
			}

			/* get initial bounds */
			proj_status = mb_proj_inverse(mbv_verbose, view->primary_pjptr, data->primary_xmin, data->primary_ymin, &xlonmin,
			                              &ylatmin, &error);
			proj_status = mb_proj_inverse(mbv_verbose, view->primary_pjptr, data->primary_xmax, data->primary_ymax, &xlonmax,
			                              &ylatmax, &error);
		}
		else {
			/* already lon lat - just copy initial bounds */
			xlonmin = data->primary_xmin;
			xlonmax = data->primary_xmax;
			ylatmin = data->primary_ymin;
			ylatmax = data->primary_ymax;
		}

		/* now go from lon lat to display coordinates */
		if (data->display_projection_mode == MBV_PROJECTION_PROJECTED) {
			/* set projection */
			proj_status = mb_proj_init(mbv_verbose, data->display_projection_id, &(view->display_pjptr), &error);
			if (proj_status == MB_SUCCESS)
				view->display_pj_init = true;

			/* quit if projection fails */
			if (proj_status != MB_SUCCESS) {
				mb_error(mbv_verbose, error, &message);
				fprintf(stderr, "\nMBIO Error initializing projection:\n%s\n", message);
				fprintf(stderr, "\nProgram terminated in <%s>\n", __func__);
				mb_memory_clear(mbv_verbose, &error);
				exit(error);
			}

			/* get bounds */
if (ylatmin > 90.0 || ylatmax > 90.0) {
fprintf(stderr, "%s:%d:%s: Warning: calling mb_proj_forward with invalid latitude: lon: %f %f lat: %f %f\n",
__FILE__, __LINE__, __FUNCTION__, xlonmin, xlonmax, ylatmin, ylatmax);
}
			proj_status = mb_proj_forward(mbv_verbose, view->display_pjptr, xlonmin, ylatmin, &view->xmin, &view->ymin, &error);
			proj_status = mb_proj_forward(mbv_verbose, view->display_pjptr, xlonmax, ylatmax, &view->xmax, &view->ymax, &error);

			/* get origin */
			view->xorigin = 0.5 * (view->xmin + view->xmax);
			view->yorigin = 0.5 * (view->ymin + view->ymax);
			view->zorigin = data->exageration * 0.5 * (data->primary_min + data->primary_max);
		}
		else if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
			/* set up geographic pseduo-projection */
			mb_coor_scale(mbv_verbose, 0.5 * (ylatmin + ylatmax), &(view->mtodeglon), &(view->mtodeglat));

			/* get bounds */
			view->xmin = xlonmin / view->mtodeglon;
			view->xmax = xlonmax / view->mtodeglon;
			view->ymin = ylatmin / view->mtodeglat;
			view->ymax = ylatmax / view->mtodeglat;

			/* get origin */
			view->xorigin = 0.5 * (view->xmin + view->xmax);
			view->yorigin = 0.5 * (view->ymin + view->ymax);
			view->zorigin = data->exageration * 0.5 * (data->primary_min + data->primary_max);
		}
		else if (data->display_projection_mode == MBV_PROJECTION_SPHEROID) {
			/* get bounds */
			if (xlonmax - xlonmin >= 180.0 || ylatmax - ylatmin >= 90.0) {
				/* setup spheroid 3D projection with view towards the center of the grid */
				mbview_sphere_setup(instance, true, 0.5 * (xlonmin + xlonmax), 0.5 * (ylatmin + ylatmax));

				view->xmin = -MBV_SPHEROID_RADIUS;
				view->xmax = MBV_SPHEROID_RADIUS;
				view->ymin = -MBV_SPHEROID_RADIUS;
				view->ymax = MBV_SPHEROID_RADIUS;

				/* get reference */
				view->sphere_refx = 0.0;
				view->sphere_refy = 0.0;
				view->sphere_refz = 0.0;

				/* get origin */
				view->xorigin = 0.0;
				view->yorigin = 0.0;
				view->zorigin = 0.0;
			}
			else {
				/* setup spheroid 3D projection with view towards the center of the grid */
				mbview_sphere_setup(instance, false, 0.5 * (xlonmin + xlonmax), 0.5 * (ylatmin + ylatmax));

				/* get origin */
				mbview_sphere_forward(instance, 0.5 * (xlonmin + xlonmax), 0.5 * (ylatmin + ylatmax), &view->sphere_refx,
				                      &view->sphere_refy, &view->sphere_refz);

				mbview_sphere_forward(instance, xlonmin, ylatmin, &view->xmin, &view->ymin, &zdisplay);
				mbview_sphere_forward(instance, xlonmax, ylatmax, &view->xmax, &view->ymax, &zdisplay);
				view->xmin -= view->sphere_refx;
				view->xmax -= view->sphere_refx;
				view->ymin -= view->sphere_refy;
				view->ymax -= view->sphere_refy;

				/* get origin */
				mbview_sphere_forward(instance, 0.5 * (xlonmin + xlonmax), 0.5 * (ylatmin + ylatmax), &view->xorigin,
				                      &view->yorigin, &view->zorigin);

				view->xorigin -= view->sphere_refx;
				view->yorigin -= view->sphere_refy;
				view->zorigin += 0.5 * (data->primary_min + data->primary_max) - view->sphere_refz;
			}
		}
	}

	/* get origin and scaling */
	view->scale = MIN((1.75 * MBV_OPENGL_WIDTH / (view->xmax - view->xmin)),
	                  (1.75 * MBV_OPENGL_WIDTH / view->aspect_ratio / (view->ymax - view->ymin)));
	view->size2d = 1.0;

	/*fprintf(stderr,"Projections:\n");
	fprintf(stderr,"  Grid: mode:%d id:%s\n",
	data->primary_grid_projection_mode, data->primary_grid_projection_id);
	fprintf(stderr,"  Display: mode:%d id:%s\n",
	data->display_projection_mode, data->display_projection_id);
	fprintf(stderr,"  Display min max: %f %f %f %f\n",
	view->xmin, view->xmax, view->ymin, view->ymax);
	fprintf(stderr,"  Display origin: %f %f %f\n", view->xorigin, view->yorigin, view->zorigin);
	fprintf(stderr,"  Display scale: %f\n", view->scale);*/

	/* set x and y arrays */
	for (i = 0; i < data->primary_n_columns; i++) {
		for (j = 0; j < data->primary_n_rows; j++) {
			/* get raw values in grid */
			k = i * data->primary_n_rows + j;
			xgrid = data->primary_xmin + i * data->primary_dx;
			ygrid = data->primary_ymin + j * data->primary_dy;

			/* reproject positions into display coordinates */
			mbview_projectforward(instance, false, xgrid, ygrid, data->primary_data[k], &xlon, &ylat, &xdisplay, &ydisplay,
			                      &zdisplay);

			/* insert into plotting arrays */
			data->primary_x[k] = (float)xdisplay;
			data->primary_y[k] = (float)ydisplay;
			data->primary_z[k] = (float)zdisplay;
		}

		/* set projection for secondary grid if needed */
		if (data->secondary_nxy > 0 && data->secondary_grid_projection_mode == MBV_PROJECTION_PROJECTED) {
			/* set projection for getting lon lat */
			proj_status = mb_proj_init(mbv_verbose, data->secondary_grid_projection_id, &(view->secondary_pjptr), &error);
			if (proj_status == MB_SUCCESS)
				view->secondary_pj_init = true;

			/* quit if projection fails */
			if (proj_status != MB_SUCCESS) {
				mb_error(mbv_verbose, error, &message);
				fprintf(stderr, "\nMBIO Error initializing projection:\n%s\n", message);
				fprintf(stderr, "\nProgram terminated in <%s>\n", __func__);
				mb_memory_clear(mbv_verbose, &error);
				exit(error);
			}
		}

		/* check for pending event */
		if (!view->plot_done && view->plot_interrupt_allowed && i % MBV_EVENTCHECKCOARSENESS == 0)
			do_mbview_xevents();

		/* dump out of loop if plotting already done at a higher recursion */
		if (view->plot_done)
			i = data->primary_n_columns;
	}

	/* calculate derivatives of primary data */
	for (i = 0; i < data->primary_n_columns; i++) {
		for (j = 0; j < data->primary_n_rows; j++) {
			mbview_derivative(instance, i, j);
		}

		/* check for pending event */
		if (!view->plot_done && view->plot_interrupt_allowed && i % MBV_EVENTCHECKCOARSENESS == 0)
			do_mbview_xevents();

		/* dump out of loop if plotting already done at a higher recursion */
		if (view->plot_done)
			i = data->primary_n_columns;
	}

	/* clear zscale for grid */
	mbview_zscaleclear(instance);

	/* project and scale data other than the grid */
	mbview_zscale(instance);

	/* set projected flag only if plotting not done */
	if (!view->plot_done) {
		view->projected = true;
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_derivative(size_t instance, int i, int j) {
	bool derivative_ok;
	double dx, dy;
	int k, k1, k2;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       i:                %d\n", i);
		fprintf(stderr, "dbg2       j:                %d\n", j);
	}
	if (mbv_verbose >= 2)
		fprintf(stderr, "mbview_derivative: %zu\n", instance);

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* figure if x derivative can be calculated */
	derivative_ok = false;
	k = i * data->primary_n_rows + j;
	if (i == 0) {
		k1 = i * data->primary_n_rows + j;
		k2 = (i + 1) * data->primary_n_rows + j;
		if (data->primary_data[k1] != data->primary_nodatavalue && data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = true;
	}
	else if (i == data->primary_n_columns - 1) {
		k1 = (i - 1) * data->primary_n_rows + j;
		k2 = i * data->primary_n_rows + j;
		if (data->primary_data[k1] != data->primary_nodatavalue && data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = true;
	}
	else {
		k1 = (i - 1) * data->primary_n_rows + j;
		k2 = (i + 1) * data->primary_n_rows + j;
		if (data->primary_data[k1] != data->primary_nodatavalue && data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = true;
		else if (data->primary_data[k1] != data->primary_nodatavalue && data->primary_data[k] != data->primary_nodatavalue) {
			derivative_ok = true;
			k2 = k;
		}
		else if (data->primary_data[k] != data->primary_nodatavalue && data->primary_data[k2] != data->primary_nodatavalue) {
			derivative_ok = true;
			k1 = k;
		}
	}

	/* calculate x derivative */
	if (derivative_ok) {
		dx = (data->primary_x[k2] - data->primary_x[k1]);
		if (dx != 0.0)
			data->primary_dzdx[k] = view->scale * (data->primary_data[k2] - data->primary_data[k1]) / dx;
		else
			data->primary_dzdx[k] = 0.0;
	}
	else
		data->primary_dzdx[k] = 0.0;

	/* figure if y derivative can be calculated */
	derivative_ok = false;
	if (j == 0) {
		k1 = i * data->primary_n_rows + j;
		k2 = i * data->primary_n_rows + (j + 1);
		if (data->primary_data[k1] != data->primary_nodatavalue && data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = true;
	}
	else if (i == data->primary_n_rows - 1) {
		k1 = i * data->primary_n_rows + (j - 1);
		k2 = i * data->primary_n_rows + j;
		if (data->primary_data[k1] != data->primary_nodatavalue && data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = true;
	}
	else {
		k1 = i * data->primary_n_rows + (j - 1);
		k = i * data->primary_n_rows + j;
		k2 = i * data->primary_n_rows + (j + 1);
		if (data->primary_data[k1] != data->primary_nodatavalue && data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = true;
		else if (data->primary_data[k1] != data->primary_nodatavalue && data->primary_data[k] != data->primary_nodatavalue) {
			derivative_ok = true;
			k2 = k;
		}
		else if (data->primary_data[k] != data->primary_nodatavalue && data->primary_data[k2] != data->primary_nodatavalue) {
			derivative_ok = true;
			k1 = k;
		}
	}

	/* calculate y derivative */
	if (derivative_ok) {
		dy = (data->primary_y[k2] - data->primary_y[k1]);
		if (dy != 0.0)
			data->primary_dzdy[k] =
			    view->scale * (data->primary_data[k2] - data->primary_data[k1]) / (data->primary_y[k2] - data->primary_y[k1]);
		else
			data->primary_dzdy[k] = 0.0;
	}
	else
		data->primary_dzdy[k] = 0.0;

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_projectglobaldata(size_t instance) {
	struct mbview_pointw_struct *pointw;
	int i, j, k;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}
	if (mbv_verbose >= 2)
		fprintf(stderr, "mbview_projectglobaldata: %zu\n", instance);

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* can only project if projections are set up */
	if (view->projected) {
		/* handle navpicks */
		if (shared.shareddata.navpick_type != MBV_PICK_NONE) {
			pointw = &(shared.shareddata.navpick.endpoints[0]);
			status = mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata, &(pointw->xgrid[instance]),
			                                  &(pointw->ygrid[instance]), &(pointw->xdisplay[instance]),
			                                  &(pointw->ydisplay[instance]), &(pointw->zdisplay[instance]));
			for (i = 0; i < 4; i++) {
				pointw = &(shared.shareddata.navpick.xpoints[i]);
				status = mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata, &(pointw->xgrid[instance]),
				                                  &(pointw->ygrid[instance]), &(pointw->xdisplay[instance]),
				                                  &(pointw->ydisplay[instance]), &(pointw->zdisplay[instance]));
			}
			for (i = 0; i < 2; i++) {
				if (shared.shareddata.navpick.xsegments[i].nls > 0) {
					for (j = 0; j < shared.shareddata.navpick.xsegments[i].nls; j++) {
						pointw = &(shared.shareddata.navpick.xsegments[i].lspoints[j]);
						status = mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata,
						                                  &(pointw->xgrid[instance]), &(pointw->ygrid[instance]),
						                                  &(pointw->xdisplay[instance]), &(pointw->ydisplay[instance]),
						                                  &(pointw->zdisplay[instance]));
					}
				}
			}
		}
		if (shared.shareddata.navpick_type == MBV_PICK_TWOPOINT) {
			pointw = &(shared.shareddata.navpick.endpoints[1]);
			status = mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata, &(pointw->xgrid[instance]),
			                                  &(pointw->ygrid[instance]), &(pointw->xdisplay[instance]),
			                                  &(pointw->ydisplay[instance]), &(pointw->zdisplay[instance]));
			for (i = 4; i < 8; i++) {
				pointw = &(shared.shareddata.navpick.xpoints[i]);
				status = mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata, &(pointw->xgrid[instance]),
				                                  &(pointw->ygrid[instance]), &(pointw->xdisplay[instance]),
				                                  &(pointw->ydisplay[instance]), &(pointw->zdisplay[instance]));
			}
			for (i = 2; i < 4; i++) {
				if (shared.shareddata.navpick.xsegments[i].nls > 0) {
					for (j = 0; j < shared.shareddata.navpick.xsegments[i].nls; j++) {
						pointw = &(shared.shareddata.navpick.xsegments[i].lspoints[j]);
						status = mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata,
						                                  &(pointw->xgrid[instance]), &(pointw->ygrid[instance]),
						                                  &(pointw->xdisplay[instance]), &(pointw->ydisplay[instance]),
						                                  &(pointw->zdisplay[instance]));
					}
				}
			}
		}

		/* handle sites */
		if (shared.shareddata.nsite > 0) {
			for (i = 0; i < shared.shareddata.nsite; i++) {
				pointw = &(shared.shareddata.sites[i].point);
				status = mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata, &(pointw->xgrid[instance]),
				                                  &(pointw->ygrid[instance]), &(pointw->xdisplay[instance]),
				                                  &(pointw->ydisplay[instance]), &(pointw->zdisplay[instance]));
			}
		}

		/* handle routes */
		if (shared.shareddata.nroute > 0) {
			for (i = 0; i < shared.shareddata.nroute; i++) {
				for (j = 0; j < shared.shareddata.routes[i].npoints; j++) {
					pointw = &(shared.shareddata.routes[i].points[j]);
					status =
					    mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata, &(pointw->xgrid[instance]),
					                             &(pointw->ygrid[instance]), &(pointw->xdisplay[instance]),
					                             &(pointw->ydisplay[instance]), &(pointw->zdisplay[instance]));
				}
				for (j = 0; j < shared.shareddata.routes[i].npoints - 1; j++) {
					for (k = 0; k < shared.shareddata.routes[i].segments[j].nls; k++) {
						pointw = &(shared.shareddata.routes[i].segments[j].lspoints[k]);
						status = mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata,
						                                  &(pointw->xgrid[instance]), &(pointw->ygrid[instance]),
						                                  &(pointw->xdisplay[instance]), &(pointw->ydisplay[instance]),
						                                  &(pointw->zdisplay[instance]));
					}
				}
			}
		}

		/* handle nav */
		if (shared.shareddata.nnav > 0) {
			for (i = 0; i < shared.shareddata.nnav; i++) {
				for (j = 0; j < shared.shareddata.navs[i].npoints; j++) {
					pointw = &(shared.shareddata.navs[i].navpts[j].point);
					status =
					    mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata, &(pointw->xgrid[instance]),
					                             &(pointw->ygrid[instance]), &(pointw->xdisplay[instance]),
					                             &(pointw->ydisplay[instance]), &(pointw->zdisplay[instance]));
					pointw = &(shared.shareddata.navs[i].navpts[j].pointport);
					status =
					    mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata, &(pointw->xgrid[instance]),
					                             &(pointw->ygrid[instance]), &(pointw->xdisplay[instance]),
					                             &(pointw->ydisplay[instance]), &(pointw->zdisplay[instance]));
					pointw = &(shared.shareddata.navs[i].navpts[j].pointcntr);
					status =
					    mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata, &(pointw->xgrid[instance]),
					                             &(pointw->ygrid[instance]), &(pointw->xdisplay[instance]),
					                             &(pointw->ydisplay[instance]), &(pointw->zdisplay[instance]));
					pointw = &(shared.shareddata.navs[i].navpts[j].pointstbd);
					status =
					    mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata, &(pointw->xgrid[instance]),
					                             &(pointw->ygrid[instance]), &(pointw->xdisplay[instance]),
					                             &(pointw->ydisplay[instance]), &(pointw->zdisplay[instance]));
				}
				for (j = 0; j < shared.shareddata.navs[i].npoints - 1; j++) {
					for (k = 0; k < shared.shareddata.navs[i].segments[j].nls; k++) {
						pointw = &(shared.shareddata.navs[i].segments[j].lspoints[k]);
						status = mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata,
						                                  &(pointw->xgrid[instance]), &(pointw->ygrid[instance]),
						                                  &(pointw->xdisplay[instance]), &(pointw->ydisplay[instance]),
						                                  &(pointw->zdisplay[instance]));
					}
				}
			}
		}

		/* handle vectors */
		if (shared.shareddata.nvector > 0) {
			for (i = 0; i < shared.shareddata.nvector; i++) {
				for (j = 0; j < shared.shareddata.vectors[i].npoints; j++) {
					pointw = &(shared.shareddata.vectors[i].vectorpts[j].point);
					status =
					    mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata, &(pointw->xgrid[instance]),
					                             &(pointw->ygrid[instance]), &(pointw->xdisplay[instance]),
					                             &(pointw->ydisplay[instance]), &(pointw->zdisplay[instance]));
				}
				for (j = 0; j < shared.shareddata.navs[i].npoints - 1; j++) {
					for (k = 0; k < shared.shareddata.navs[i].segments[j].nls; k++) {
						pointw = &(shared.shareddata.navs[i].segments[j].lspoints[k]);
						status = mbview_projectfromlonlat(instance, pointw->xlon, pointw->ylat, pointw->zdata,
						                                  &(pointw->xgrid[instance]), &(pointw->ygrid[instance]),
						                                  &(pointw->xdisplay[instance]), &(pointw->ydisplay[instance]),
						                                  &(pointw->zdisplay[instance]));
					}
				}
			}
		}

		/* set globalprojected flag */
		view->globalprojected = true;
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_zscalegridpoint(size_t instance, int k) {
	double xgrid, ygrid;
	double xlon, ylat;
	double xdisplay, ydisplay, zdisplay;
	int i, j;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       k:                %d\n", k);
	}
	if (mbv_verbose >= 2)
		fprintf(stderr, "mbview_zscalegridpoint: %d\n", k);

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* scale z value */
	if (data->display_projection_mode == MBV_PROJECTION_PROJECTED ||
	    data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED ||
	    data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
		/* scale z value alone */
		data->primary_z[k] = (float)(view->scale * (data->exageration * data->primary_data[k] - view->zorigin));
	}
	else if (data->display_projection_mode == MBV_PROJECTION_SPHEROID) {
		/* must reproject everything in this case */
		i = k / data->primary_n_rows;
		j = k % data->primary_n_rows;
		xgrid = data->primary_xmin + i * data->primary_dx;
		ygrid = data->primary_ymin + j * data->primary_dy;

		/* reproject positions into display coordinates */
		mbview_projectforward(instance, false, xgrid, ygrid, data->primary_data[k], &xlon, &ylat, &xdisplay, &ydisplay,
		                      &zdisplay);

		/* insert into plotting arrays */
		data->primary_x[k] = (float)xdisplay;
		data->primary_y[k] = (float)ydisplay;
		data->primary_z[k] = (float)zdisplay;
	}

	/* set zscale status bit */
	data->primary_stat_z[k / 8] = data->primary_stat_z[k / 8] | statmask[k % 8];

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_zscalepoint(size_t instance, int globalview, double offset_factor, struct mbview_point_struct *point) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       globalview:       %d\n", globalview);
		fprintf(stderr, "dbg2       offset_factor:    %f\n", offset_factor);
	}
	if (mbv_verbose >= 2)
		fprintf(stderr, "mbview_zscalepoint: %zu\n", instance);

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* scale z value */
	if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
		/* scale z value alone */
		point->zdisplay = view->scale * (data->exageration * point->zdata - view->zorigin) + offset_factor;
	}
	else {
		/* reproject positions into display coordinates */
		mbview_projectforward(instance, false, point->xgrid, point->ygrid, point->zdata, &point->xlon, &point->ylat,
		                      &point->xdisplay, &point->ydisplay, &point->zdisplay);

		if (!globalview) {
			point->zdisplay += offset_factor;
		}
		else {
			point->xdisplay += point->xdisplay * offset_factor;
			point->ydisplay += point->ydisplay * offset_factor;
			point->zdisplay += point->zdisplay * offset_factor;
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_zscalepointw(size_t instance, int globalview, double offset_factor, struct mbview_pointw_struct *pointw) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       globalview:       %d\n", globalview);
		fprintf(stderr, "dbg2       offset_factor:    %f\n", offset_factor);
	}
	if (mbv_verbose >= 2)
		fprintf(stderr, "mbview_zscalepointw: %zu\n", instance);

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* scale z value */
	if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
		/* scale z value alone */
		pointw->zdisplay[instance] = view->scale * (data->exageration * pointw->zdata - view->zorigin) + offset_factor;
	}
	else {
		/* reproject positions into display coordinates */
		mbview_projectforward(instance, false, pointw->xgrid[instance], pointw->ygrid[instance], pointw->zdata, &(pointw->xlon),
		                      &(pointw->ylat), &(pointw->xdisplay[instance]), &(pointw->ydisplay[instance]),
		                      &(pointw->zdisplay[instance]));

		if (!globalview) {
			pointw->zdisplay[instance] += offset_factor;
		}
		else {
			pointw->xdisplay[instance] += pointw->xdisplay[instance] * offset_factor;
			pointw->ydisplay[instance] += pointw->ydisplay[instance] * offset_factor;
			pointw->zdisplay[instance] += pointw->zdisplay[instance] * offset_factor;
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_updatepointw(size_t instance, struct mbview_pointw_struct *pointw) {
	size_t i;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}
	if (mbv_verbose >= 2)
		fprintf(stderr, "mbview_updatepointw: %zu\n", instance);

	int status = MB_SUCCESS;

	/* update grid and display coordinates for pointw for all
	    active instances other than instance, which has
	    already been set */
	for (i = 0; i < MBV_MAX_WINDOWS; i++) {
		struct mbview_world_struct *view = &(mbviews[i]);
		if (i != instance && view->init != MBV_WINDOW_NULL) {
                  // struct mbview_struct *data = &(view->data);

			/* get positions in grid coordinates */
			status = mbview_projectll2xygrid(i, pointw->xlon, pointw->ylat, &(pointw->xgrid[i]), &(pointw->ygrid[i]));

			/* get positions in display coordinates */
			status = mbview_projectll2display(i, pointw->xlon, pointw->ylat, pointw->zdata, &(pointw->xdisplay[i]),
			                                  &(pointw->ydisplay[i]), &(pointw->zdisplay[i]));
		}
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_updatesegmentw(size_t instance, struct mbview_linesegmentw_struct *segmentw) {
	int i;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}
	if (mbv_verbose >= 2)
		fprintf(stderr, "mbview_updatesegmentw: %zu\n", instance);

	/* update grid and display coordinates for segmentw for all
	    active instances other than instance, which has
	    already been set */
	for (i = 0; i < segmentw->nls; i++) {
		mbview_updatepointw(instance, &(segmentw->lspoints[i]));
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_zscale(size_t instance) {
	int i, j, k;
	int globalview;
	double offset_factor;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}
	if (mbv_verbose >= 2)
		fprintf(stderr, "mbview_zscale: %zu\n", instance);

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* check if the contour offset needs to be applied in a global spherical direction or just up */
	if (data->display_projection_mode == MBV_PROJECTION_SPHEROID && view->sphere_refx == 0.0 && view->sphere_refy == 0.0 &&
	    view->sphere_refz == 0.0) {
		globalview = true;
		offset_factor = 10.0 * MBV_OPENGL_3D_CONTOUR_OFFSET / (view->scale * MBV_SPHEROID_RADIUS);
	}
	else {
		globalview = false;
		offset_factor = MBV_OPENGL_3D_CONTOUR_OFFSET;
	}

	/* handle picks */
	if (data->pick_type != MBV_PICK_NONE) {
		mbview_zscalepoint(instance, globalview, offset_factor, &(data->pick.endpoints[0]));
		for (i = 0; i < 4; i++) {
			mbview_zscalepoint(instance, globalview, offset_factor, &(data->pick.xpoints[i]));
		}
		for (i = 0; i < 2; i++) {
			for (j = 0; j < data->pick.xsegments[i].nls; j++) {
				mbview_zscalepoint(instance, globalview, offset_factor, &(data->pick.xsegments[i].lspoints[j]));
			}
		}
	}
	if (data->pick_type == MBV_PICK_TWOPOINT) {
		mbview_zscalepoint(instance, globalview, offset_factor, &(data->pick.endpoints[1]));
		for (i = 4; i < 8; i++) {
			mbview_zscalepoint(instance, globalview, offset_factor, &(data->pick.xpoints[i]));
		}
		for (i = 2; i < 4; i++) {
			for (j = 0; j < data->pick.xsegments[i].nls; j++) {
				mbview_zscalepoint(instance, globalview, offset_factor, &(data->pick.xsegments[i].lspoints[j]));
			}
		}
		if (data->pick.segment.nls > 0) {
			for (j = 0; j < data->pick.segment.nls; j++) {
				mbview_zscalepoint(instance, globalview, offset_factor, &(data->pick.segment.lspoints[j]));
			}
		}
	}

	/* handle area */
	if (data->area_type == MBV_AREA_QUAD) {
		for (i = 0; i < 2; i++) {
			mbview_zscalepoint(instance, globalview, offset_factor, &(data->area.endpoints[i]));
		}
		for (j = 0; j < data->area.segment.nls; j++) {
			mbview_zscalepoint(instance, globalview, offset_factor, &(data->area.segment.lspoints[j]));
		}
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 2; j++) {
				mbview_zscalepoint(instance, globalview, offset_factor, &(data->area.segments[i].endpoints[j]));
			}
			for (j = 0; j < data->area.segments[i].nls; j++) {
				mbview_zscalepoint(instance, globalview, offset_factor, &(data->area.segments[i].lspoints[j]));
			}
		}
	}

	/* handle region */
	if (data->region_type == MBV_REGION_QUAD) {
		for (i = 0; i < 4; i++) {
			mbview_zscalepoint(instance, globalview, offset_factor, &(data->region.cornerpoints[i]));
			for (j = 0; j < data->region.segments[i].nls; j++) {
				mbview_zscalepoint(instance, globalview, offset_factor, &(data->region.segments[i].lspoints[j]));
			}
		}
	}

	/* handle navpicks */
	if (shared.shareddata.navpick_type != MBV_PICK_NONE) {
		mbview_zscalepointw(instance, globalview, offset_factor, &(shared.shareddata.navpick.endpoints[0]));
		for (i = 0; i < 4; i++) {
			mbview_zscalepointw(instance, globalview, offset_factor, &(shared.shareddata.navpick.xpoints[i]));
		}
		for (i = 0; i < 2; i++) {
			if (shared.shareddata.navpick.xsegments[i].nls > 0) {
				for (j = 0; j < shared.shareddata.navpick.xsegments[i].nls; j++) {
					mbview_zscalepointw(instance, globalview, offset_factor,
					                    &(shared.shareddata.navpick.xsegments[i].lspoints[j]));
				}
			}
		}
	}
	if (shared.shareddata.navpick_type == MBV_PICK_TWOPOINT) {
		mbview_zscalepointw(instance, globalview, offset_factor, &(shared.shareddata.navpick.endpoints[1]));
		for (i = 4; i < 8; i++) {
			mbview_zscalepointw(instance, globalview, offset_factor, &(shared.shareddata.navpick.xpoints[i]));
		}
		for (i = 2; i < 4; i++) {
			if (shared.shareddata.navpick.xsegments[i].nls > 0) {
				for (j = 0; j < shared.shareddata.navpick.xsegments[i].nls; j++) {
					mbview_zscalepointw(instance, globalview, offset_factor,
					                    &(shared.shareddata.navpick.xsegments[i].lspoints[j]));
				}
			}
		}
	}

	/* handle sites */
	if (shared.shareddata.nsite > 0) {
		for (i = 0; i < shared.shareddata.nsite; i++) {
			mbview_zscalepointw(instance, globalview, offset_factor, &(shared.shareddata.sites[i].point));
		}
	}

	/* handle routes */
	if (shared.shareddata.nroute > 0) {
		for (i = 0; i < shared.shareddata.nroute; i++) {
			for (j = 0; j < shared.shareddata.routes[i].npoints; j++) {
				mbview_zscalepointw(instance, globalview, offset_factor, &(shared.shareddata.routes[i].points[j]));
			}
			for (j = 0; j < shared.shareddata.routes[i].npoints - 1; j++) {
				for (k = 0; k < shared.shareddata.routes[i].segments[j].nls; k++) {
					mbview_zscalepointw(instance, globalview, offset_factor,
					                    &(shared.shareddata.routes[i].segments[j].lspoints[k]));
				}
			}
		}
	}

	/* handle nav */
	if (shared.shareddata.nnav > 0) {
		for (i = 0; i < shared.shareddata.nnav; i++) {
			for (j = 0; j < shared.shareddata.navs[i].npoints; j++) {
				mbview_zscalepointw(instance, globalview, offset_factor, &(shared.shareddata.navs[i].navpts[j].point));
				mbview_zscalepointw(instance, globalview, offset_factor, &(shared.shareddata.navs[i].navpts[j].pointport));
				mbview_zscalepointw(instance, globalview, offset_factor, &(shared.shareddata.navs[i].navpts[j].pointcntr));
				mbview_zscalepointw(instance, globalview, offset_factor, &(shared.shareddata.navs[i].navpts[j].pointstbd));
			}
			for (j = 0; j < shared.shareddata.navs[i].npoints - 1; j++) {
				for (k = 0; k < shared.shareddata.navs[i].segments[j].nls; k++) {
					mbview_zscalepointw(instance, globalview, offset_factor,
					                    &(shared.shareddata.navs[i].segments[j].lspoints[k]));
				}
			}
		}
	}

	/* handle vector */
	if (shared.shareddata.nvector > 0) {
		for (i = 0; i < shared.shareddata.nvector; i++) {
			for (j = 0; j < shared.shareddata.vectors[i].npoints; j++) {
				mbview_zscalepointw(instance, globalview, offset_factor, &(shared.shareddata.vectors[i].vectorpts[j].point));
			}
			for (j = 0; j < shared.shareddata.vectors[i].npoints - 1; j++) {
				for (k = 0; k < shared.shareddata.vectors[i].segments[j].nls; k++) {
					mbview_zscalepointw(instance, globalview, offset_factor,
					                    &(shared.shareddata.vectors[i].segments[j].lspoints[k]));
				}
			}
		}
	}

	/* set rez flags only if plotting not done */
	if (!view->plot_done) {
		view->contourlorez = false;
		view->contourhirez = false;
		view->contourfullrez = false;
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectforward(size_t instance, bool needlonlat, double xgrid, double ygrid, double zdata, double *xlon, double *ylat,
                          double *xdisplay, double *ydisplay, double *zdisplay) {
	double xx, yy, zz;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       needlonlat:       %d\n", needlonlat);
		fprintf(stderr, "dbg2       xgrid:            %f\n", xgrid);
		fprintf(stderr, "dbg2       ygrid:            %f\n", ygrid);
		fprintf(stderr, "dbg2       zdata:            %f\n", zdata);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* get positions into geographic coordinates if necessary */
	if (needlonlat || data->primary_grid_projection_mode != MBV_PROJECTION_ALREADYPROJECTED) {
		status = mbview_projectgrid2ll(instance, xgrid, ygrid, xlon, ylat);
	}

	/* get positions in the display projection */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
		xx = xgrid;
		yy = ygrid;
		zz = data->exageration * zdata;
		*xdisplay = view->scale * (xx - view->xorigin);
		*ydisplay = view->scale * (yy - view->yorigin);
		*zdisplay = view->scale * (zz - view->zorigin);
	}
	else {
		status = mbview_projectll2display(instance, *xlon, *ylat, zdata, xdisplay, ydisplay, zdisplay);
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       xlon:        %f\n", *xlon);
		fprintf(stderr, "dbg2       ylat:        %f\n", *ylat);
		fprintf(stderr, "dbg2       xdisplay:    %f\n", *xdisplay);
		fprintf(stderr, "dbg2       ydisplay:    %f\n", *ydisplay);
		fprintf(stderr, "dbg2       zdisplay:    %f\n", *zdisplay);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectinverse(size_t instance, bool needlonlat, double xdisplay, double ydisplay, double zdisplay, double *xlon,
                          double *ylat, double *xgrid, double *ygrid) {
	double xx, yy;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       needlonlat:       %d\n", needlonlat);
		fprintf(stderr, "dbg2       xdisplay:         %f\n", xdisplay);
		fprintf(stderr, "dbg2       ydisplay:         %f\n", ydisplay);
		fprintf(stderr, "dbg2       zdisplay:         %f\n", zdisplay);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* get positions in geographic coordinates */
	if (needlonlat || data->primary_grid_projection_mode != MBV_PROJECTION_ALREADYPROJECTED) {
		status = mbview_projectdisplay2ll(instance, xdisplay, ydisplay, zdisplay, xlon, ylat);
	}

	/* get positions into grid coordinates */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
		xx = xdisplay / view->scale + view->xorigin;
		yy = ydisplay / view->scale + view->yorigin;
		*xgrid = xx;
		*ygrid = yy;
	}
	else {
		status = mbview_projectll2xygrid(instance, *xlon, *ylat, xgrid, ygrid);
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       xlon:         %f\n", *xlon);
		fprintf(stderr, "dbg2       ylat:         %f\n", *ylat);
		fprintf(stderr, "dbg2       xgrid:        %f\n", *xgrid);
		fprintf(stderr, "dbg2       ygrid:        %f\n", *ygrid);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectfromlonlat(size_t instance, double xlon, double ylat, double zdata, double *xgrid, double *ygrid,
                             double *xdisplay, double *ydisplay, double *zdisplay) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       xlon:             %f\n", xlon);
		fprintf(stderr, "dbg2       ylat:             %f\n", ylat);
		fprintf(stderr, "dbg2       zdata:            %f\n", zdata);
	}

	/* get positions into grid coordinates */
	int status = mbview_projectll2xygrid(instance, xlon, ylat, xgrid, ygrid);

	/* get positions in the display projection */
	status = mbview_projectll2display(instance, xlon, ylat, zdata, xdisplay, ydisplay, zdisplay);

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       xgrid:       %f\n", *xgrid);
		fprintf(stderr, "dbg2       ygrid:       %f\n", *ygrid);
		fprintf(stderr, "dbg2       xdisplay:    %f\n", *xdisplay);
		fprintf(stderr, "dbg2       ydisplay:    %f\n", *ydisplay);
		fprintf(stderr, "dbg2       zdisplay:    %f\n", *zdisplay);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectgrid2ll(size_t instance, double xgrid, double ygrid, double *xlon, double *ylat) {
	int error = MB_ERROR_NO_ERROR;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       xgrid:            %f\n", xgrid);
		fprintf(stderr, "dbg2       ygrid:            %f\n", ygrid);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get positions into geographic coordinates */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED ||
	    data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
		mb_proj_inverse(mbv_verbose, view->primary_pjptr, xgrid, ygrid, xlon, ylat, &error);
	}
	else if (data->primary_grid_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
		*xlon = xgrid;
		*ylat = ygrid;
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       xlon:             %f\n", *xlon);
		fprintf(stderr, "dbg2       ylat:             %f\n", *ylat);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectll2xygrid(size_t instance, double xlon, double ylat, double *xgrid, double *ygrid) {
	int error = MB_ERROR_NO_ERROR;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       xlon:             %f\n", xlon);
		fprintf(stderr, "dbg2       ylat:             %f\n", ylat);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get positions into grid coordinates */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED ||
	    data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
if (ylat > 90.0) {
fprintf(stderr, "%s:%d:%s: Warning: calling mb_proj_forward with invalid latitude: lon: %f lat: %f\n",
__FILE__, __LINE__, __FUNCTION__, xlon, ylat);
}
		mb_proj_forward(mbv_verbose, view->primary_pjptr, xlon, ylat, xgrid, ygrid, &error);
	}
	else {
		if (data->primary_grid_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
			if (data->primary_xmin < -180.0 && xlon > 0.0)
				xlon -= 360.0;
			if (data->primary_xmax > 180.0 && xlon < 0.0)
				xlon += 360.0;
		}
		*xgrid = xlon;
		*ygrid = ylat;
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       xgrid:       %f\n", *xgrid);
		fprintf(stderr, "dbg2       ygrid:       %f\n", *ygrid);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectll2xyzgrid(size_t instance, double xlon, double ylat, double *xgrid, double *ygrid, double *zdata) {
	int error = MB_ERROR_NO_ERROR;
	int nfound;
	int i, j, k, ii, jj;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       xlon:             %f\n", xlon);
		fprintf(stderr, "dbg2       ylat:             %f\n", ylat);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get positions into grid coordinates */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED ||
	    data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
if (ylat > 90.0) {
fprintf(stderr, "%s:%d:%s: Warning: calling mb_proj_forward with invalid latitude: lon: %f lat: %f\n",
__FILE__, __LINE__, __FUNCTION__, xlon, ylat);
}
		mb_proj_forward(mbv_verbose, view->primary_pjptr, xlon, ylat, xgrid, ygrid, &error);
	}
	else {
		if (data->primary_grid_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
			if (data->primary_xmin < -180.0 && xlon > 0.0)
				xlon -= 360.0;
			if (data->primary_xmax > 180.0 && xlon < 0.0)
				xlon += 360.0;
		}
		*xgrid = xlon;
		*ygrid = ylat;
	}

	/* now get zdata  from primary grid */
	nfound = 0;
	*zdata = 0.0;
	i = (int)((*xgrid - data->primary_xmin) / data->primary_dx);
	j = (int)((*ygrid - data->primary_ymin) / data->primary_dy);
	if (i >= 0 && i < data->primary_n_columns - 1 && j >= 0 && j < data->primary_n_rows - 1) {
		for (ii = i; ii <= i + 1; ii++)
			for (jj = j; jj <= j + 1; jj++) {
				k = ii * data->primary_n_rows + jj;
				if (data->primary_data[k] != data->primary_nodatavalue) {
					nfound++;
					*zdata += data->primary_data[k];
				}
			}
	}

	int status = MB_SUCCESS;
	if (nfound > 0) {
		*zdata /= (double)nfound;
		status = MB_SUCCESS;
	}
	else {
		*zdata = 0.0;
		status = MB_FAILURE;
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       xgrid:       %f\n", *xgrid);
		fprintf(stderr, "dbg2       ygrid:       %f\n", *ygrid);
		fprintf(stderr, "dbg2       zdata:       %f\n", *zdata);
		fprintf(stderr, "dbg2       data->primary_nodatavalue:       %f\n", data->primary_nodatavalue);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectll2display(size_t instance, double xlon, double ylat, double zdata, double *xdisplay, double *ydisplay,
                             double *zdisplay) {
	int error = MB_ERROR_NO_ERROR;
	double xx, yy, zz;
	double effective_topography;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       xlon:             %f\n", xlon);
		fprintf(stderr, "dbg2       ylat:             %f\n", ylat);
		fprintf(stderr, "dbg2       zdata:            %f\n", zdata);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get positions in the display projection */
	if (data->display_projection_mode == MBV_PROJECTION_PROJECTED ||
	    data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
if (ylat > 90.0) {
fprintf(stderr, "%s:%d:%s: Warning: calling mb_proj_forward with invalid latitude: lon: %f lat: %f\n",
__FILE__, __LINE__, __FUNCTION__, xlon, ylat);
}
		mb_proj_forward(mbv_verbose, view->display_pjptr, xlon, ylat, &xx, &yy, &error);
		zz = data->exageration * zdata;
		/* fprintf(stderr,"pos: %f %f %f   raw: %f %f %f ",
		xlon, ylat, zdata, xx, yy, zz); */
	}
	else if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
		xx = xlon / view->mtodeglon;
		yy = ylat / view->mtodeglat;
		zz = data->exageration * zdata;
	}
	else /*if (data->display_projection_mode == MBV_PROJECTION_SPHEROID) */
	{
		mbview_sphere_forward(instance, xlon, ylat, &xx, &yy, &zz);
		effective_topography = data->exageration * (zdata - 0.5 * (data->primary_min + data->primary_max)) +
		                       0.5 * (data->primary_min + data->primary_max);
		/* fprintf(stderr,"pos: %f %f %f   raw: %f %f %f  topo:%f ",
		xlon, ylat, zdata, xx, yy, zz, effective_topography); */

		xx += (effective_topography * xx / MBV_SPHEROID_RADIUS) - view->sphere_refx;
		yy += (effective_topography * yy / MBV_SPHEROID_RADIUS) - view->sphere_refy;
		zz += (effective_topography * zz / MBV_SPHEROID_RADIUS) - view->sphere_refz;
		/* fprintf(stderr,"unscaled: %f %f %f",
		xx, yy, zz); */
	}

	/* get final positions in display coordinates */
	*xdisplay = view->scale * (xx - view->xorigin);
	*ydisplay = view->scale * (yy - view->yorigin);
	*zdisplay = view->scale * (zz - view->zorigin);
	if (isnan(*xdisplay)) {
		fprintf(stderr, "NaN alert!!\n");
		//fprintf(stderr,"pos: %f %f %f   raw: %f %f %f  topo:%f   scale:%f   scaled: %f %f %f\n",
		//xlon, ylat, zdata, xx, yy, zz, effective_topography, view->scale, *xdisplay, *ydisplay, *zdisplay);
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       xdisplay:    %f\n", *xdisplay);
		fprintf(stderr, "dbg2       ydisplay:    %f\n", *ydisplay);
		fprintf(stderr, "dbg2       zdisplay:    %f\n", *zdisplay);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectdisplay2ll(size_t instance, double xdisplay, double ydisplay, double zdisplay, double *xlon, double *ylat) {
	int error = MB_ERROR_NO_ERROR;
	double xx, yy, zz;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       xdisplay:         %f\n", xdisplay);
		fprintf(stderr, "dbg2       ydisplay:         %f\n", ydisplay);
		fprintf(stderr, "dbg2       zdisplay:         %f\n", zdisplay);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get positions in display projection */
	xx = xdisplay / view->scale + view->xorigin;
	yy = ydisplay / view->scale + view->yorigin;
	zz = zdisplay / view->scale + view->zorigin;

	/* get positions in geographic coordinates */
	if (data->display_projection_mode == MBV_PROJECTION_PROJECTED ||
	    data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
		mb_proj_inverse(mbv_verbose, view->display_pjptr, xx, yy, xlon, ylat, &error);
	}
	else if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
		*xlon = xx * view->mtodeglon;
		*ylat = yy * view->mtodeglat;
	}
	else if (data->display_projection_mode == MBV_PROJECTION_SPHEROID) {
		xx += view->sphere_refx;
		yy += view->sphere_refy;
		zz += view->sphere_refz;
		mbview_sphere_inverse(instance, xx, yy, zz, xlon, ylat);
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       xlon:             %f\n", *xlon);
		fprintf(stderr, "dbg2       ylat:             %f\n", *ylat);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_projectdistance(size_t instance, double xlon1, double ylat1, double zdata1, double xlon2, double ylat2, double zdata2,
                           double *distancelateral, double *distanceoverground, double *slope) {
	int error = MB_ERROR_NO_ERROR;
	double xx1, yy1, zz1;
	double xx2, yy2, zz2;
	double dx, dy, dz;
	double bearing;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       xlon1:            %f\n", xlon1);
		fprintf(stderr, "dbg2       ylat1:            %f\n", ylat1);
		fprintf(stderr, "dbg2       zdata1:           %f\n", zdata1);
		fprintf(stderr, "dbg2       xlon2:            %f\n", xlon2);
		fprintf(stderr, "dbg2       ylat2:            %f\n", ylat2);
		fprintf(stderr, "dbg2       zdata2:           %f\n", zdata2);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get positions in display projection without scaling or exageration */
	if (data->display_projection_mode == MBV_PROJECTION_PROJECTED ||
	    data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
		/* point 1 */
if (ylat1 > 90.0) {
fprintf(stderr, "%s:%d:%s: Warning: calling mb_proj_forward with invalid latitude: lon: %f lat: %f\n",
__FILE__, __LINE__, __FUNCTION__, xlon1, ylat1);
}
		mb_proj_forward(mbv_verbose, view->display_pjptr, xlon1, ylat1, &xx1, &yy1, &error);
		zz1 = zdata1;

		/* point 2 */
if (ylat2 > 90.0) {
fprintf(stderr, "%s:%d:%s: Warning: calling mb_proj_forward with invalid latitude: lon: %f lat: %f\n",
__FILE__, __LINE__, __FUNCTION__, xlon2, ylat2);
}
		mb_proj_forward(mbv_verbose, view->display_pjptr, xlon2, ylat2, &xx2, &yy2, &error);
		zz2 = zdata2;

		/* distance and slope */
		dx = xx2 - xx1;
		dy = yy2 - yy1;
		dz = zz2 - zz1;
		*distancelateral = sqrt(dx * dx + dy * dy);
		*distanceoverground = sqrt(dx * dx + dy * dy + dz * dz);
		if (*distancelateral > 0.0)
			*slope = dz / (*distancelateral);
		else
			*slope = 0.0;
	}
	else if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
		/* point 1 */
		xx1 = xlon1 / view->mtodeglon;
		yy1 = ylat1 / view->mtodeglat;
		zz1 = zdata1;

		/* point 2 */
		xx2 = xlon2 / view->mtodeglon;
		yy2 = ylat2 / view->mtodeglat;
		zz2 = zdata2;

		/* distance and slope */
		dx = xx2 - xx1;
		dy = yy2 - yy1;
		dz = zz2 - zz1;
		*distancelateral = sqrt(dx * dx + dy * dy);
		*distanceoverground = sqrt(dx * dx + dy * dy + dz * dz);
		if (*distancelateral > 0.0)
			*slope = dz / (*distancelateral);
		else
			*slope = 0.0;
	}
	else if (data->display_projection_mode == MBV_PROJECTION_SPHEROID) {
		/* point 1 */
		mbview_sphere_forward(instance, xlon1, ylat1, &xx1, &yy1, &zz1);

		/* point 2 */
		mbview_sphere_forward(instance, xlon2, ylat2, &xx2, &yy2, &zz2);

		/* lateral distance */
		mbview_greatcircle_distbearing(instance, xlon1, ylat1, xlon2, ylat2, &bearing, distancelateral);

		/* distance over ground */
		xx1 += zdata1 * xx1 / MBV_SPHEROID_RADIUS;
		yy1 += zdata1 * yy1 / MBV_SPHEROID_RADIUS;
		zz1 += zdata1 * zz1 / MBV_SPHEROID_RADIUS;
		xx2 += zdata2 * xx2 / MBV_SPHEROID_RADIUS;
		yy2 += zdata2 * yy2 / MBV_SPHEROID_RADIUS;
		zz2 += zdata2 * zz2 / MBV_SPHEROID_RADIUS;
		dx = xx2 - xx1;
		dy = yy2 - yy1;
		dz = zz2 - zz1;
		*distanceoverground = sqrt(dx * dx + dy * dy + dz * dz);

		/* slope */
		if (*distancelateral > 0.0)
			*slope = (zdata2 - zdata1) / (*distancelateral);
		else
			*slope = 0.0;
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       distancelateral:     %f\n", *distancelateral);
		fprintf(stderr, "dbg2       distanceoverground:  %f\n", *distanceoverground);
		fprintf(stderr, "dbg2       slope:               %f\n", *slope);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_sphere_setup(size_t instance, bool earthcentered, double xlon, double ylat) {
	double phi, theta, psi;
	int j;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       earthcentered:    %d\n", earthcentered);
		fprintf(stderr, "dbg2       xlon:             %f\n", xlon);
		fprintf(stderr, "dbg2       ylat:             %f\n", ylat);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* The initial spherical coordinate system is defined as:
	        x = r * cos(longitude) * cos(latitude)
	        y = r * sin(longitude) * cos(latitude)
	        z = r * sin(latitude)
	   which is equivalent to:
	        x = r * cos(longitude) * sin(colatitude)
	        y = r * sin(longitude) * sin(colatitude)
	        z = r * cos(colatitude)
	   where:
	        colatitude = PI/2 - latitude

	   Euler's rotation theorem proves than any general rotation may be
	   described by three successive rotations about the axes. One convention
	   is to use first a rotation about the z-axis (angle phi), then a
	   rotation about the x'-axis (angle theta), and finally a rotation
	   about the z''-axis (angle psi).

	   The euler rotation matrix becomes:
	        |	a11	a12	a13	|
	        |	a21	a22	a23	|
	        |	a31	a32	a33	|
	   where:
	        a11 = cos(phi) * cos(psi) - sin(phi) * cos(theta) * sin(psi)
	        a12 = sin(phi) * cos(psi) + cos(phi) * cos(theta) * sin(psi)
	        a13 = sin(theta) * sin (psi)
	        a21 = -cos(phi) * sin(psi) - sin(phi) * cos(theta) * cos(psi)
	        a22 = -sin(phi) * sin(psi) + cos(phi) * cos(theta) * cos(psi)
	        a23 = sin(theta) * cos(psi)
	        a31 = sin(phi) * sin(theta)
	        a32 = -cos(phi) * sin(theta)
	        a33 = cos(theta)

	   We wish to rotate the coordinate system so that the reference position
	   defined by xlon and ylat are located on the positive z-axis. The forward
	   rotation is accomplished using:
	        phi = -PI/2 + xlon
	        theta = -ycolat = ylat - PI/2
	        psi = PI
	   The reverse rotation is accomplished using:
	        phi = -PI
	        theta = ycolat = PI/2 - ylat
	        psi = xlon - PI/2

	  The relevant equations derived in part from:
	    http://mathworld.wolfram.com/EulerAngles.html
	  which were viewed on January 19, 2004
	    */

	/* create forward rotation matrix */
	phi = DTR * xlon - 0.5 * M_PI;
	theta = DTR * ylat - 0.5 * M_PI;
	psi = M_PI;
	mbview_sphere_matrix(phi, theta, psi, view->sphere_eulerforward);

	/* create reverse rotation matrix */
	phi = -M_PI;
	theta = 0.5 * M_PI - DTR * ylat;
	psi = 0.5 * M_PI - DTR * xlon;
	mbview_sphere_matrix(phi, theta, psi, view->sphere_eulerreverse);

	/* now get reference location in rotated coordinates */
	view->sphere_reflon = xlon;
	view->sphere_reflat = ylat;
	view->sphere_refx = 0.0;
	view->sphere_refy = 0.0;
	view->sphere_refz = 0.0;
	if (!earthcentered) {
		mbview_sphere_forward(instance, xlon, ylat, &view->sphere_refx, &view->sphere_refy, &view->sphere_refz);
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Internal results:\n");
		fprintf(stderr, "dbg2       view->sphere_reflon:      %f\n", view->sphere_reflon);
		fprintf(stderr, "dbg2       view->sphere_reflat:      %f\n", view->sphere_reflat);
		fprintf(stderr, "dbg2       view->sphere_refx:        %f\n", view->sphere_refx);
		fprintf(stderr, "dbg2       view->sphere_refy:        %f\n", view->sphere_refy);
		fprintf(stderr, "dbg2       view->sphere_refz:        %f\n", view->sphere_refz);
		fprintf(stderr, "dbg2       view->sphere_eulerforward:\n");
		for (j = 0; j < 3; j++) {
			fprintf(stderr, "dbg2                         %f %f %f\n", view->sphere_eulerforward[0 + 3 * j],
			        view->sphere_eulerforward[1 + 3 * j], view->sphere_eulerforward[2 + 3 * j]);
		}
		fprintf(stderr, "dbg2       view->sphere_eulerreverse:\n");
		for (j = 0; j < 3; j++) {
			fprintf(stderr, "dbg2                         %f %f %f\n", view->sphere_eulerreverse[0 + 3 * j],
			        view->sphere_eulerreverse[1 + 3 * j], view->sphere_eulerreverse[2 + 3 * j]);
		}
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_sphere_forward(size_t instance, double xlon, double ylat, double *xx, double *yy, double *zz) {
	double sinlon, coslon, sinlat, coslat;
	double posu[3], posr[3];

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       xlon:             %f\n", xlon);
		fprintf(stderr, "dbg2       ylat:             %f\n", ylat);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* get position in initial cartesian coordinates */
	sinlon = sin(DTR * xlon);
	coslon = cos(DTR * xlon);
	sinlat = sin(DTR * ylat);
	coslat = cos(DTR * ylat);
	posu[0] = MBV_SPHEROID_RADIUS * coslon * coslat;
	posu[1] = MBV_SPHEROID_RADIUS * sinlon * coslat;
	posu[2] = MBV_SPHEROID_RADIUS * sinlat;

	/* apply rotation to coordinates with the reference location
	    at the center of the view, on the positive z-axis. */
	mbview_sphere_rotate(view->sphere_eulerforward, posu, posr);

	/* make relative to reference location */
	*xx = posr[0];
	*yy = posr[1];
	*zz = posr[2];

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       posu[0]:     %f\n", posu[0]);
		fprintf(stderr, "dbg2       posu[1]:     %f\n", posu[1]);
		fprintf(stderr, "dbg2       posu[2]:     %f\n", posu[2]);
		fprintf(stderr, "dbg2       posr[0]:     %f\n", posr[0]);
		fprintf(stderr, "dbg2       posr[1]:     %f\n", posr[1]);
		fprintf(stderr, "dbg2       posr[2]:     %f\n", posr[2]);
		fprintf(stderr, "dbg2       xx:          %f\n", *xx);
		fprintf(stderr, "dbg2       yy:          %f\n", *yy);
		fprintf(stderr, "dbg2       zz:          %f\n", *zz);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_sphere_inverse(size_t instance, double xx, double yy, double zz, double *xlon, double *ylat) {
	double posu[3], posr[3];

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       xx:               %f\n", xx);
		fprintf(stderr, "dbg2       yy:               %f\n", yy);
		fprintf(stderr, "dbg2       zz:               %f\n", zz);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* get position in cartesian spheroid coordinates */
	posr[0] = xx;
	posr[1] = yy;
	posr[2] = zz;

	/* unrotate position */
	mbview_sphere_rotate(view->sphere_eulerreverse, posr, posu);

	/* get longitude and latitude */
	*xlon = RTD * atan2(posu[1], posu[0]);
	*ylat = 90.0 - RTD * (atan2(sqrt(posu[0] * posu[0] + posu[1] * posu[1]), posu[2]));

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       posr[0]:     %f\n", posr[0]);
		fprintf(stderr, "dbg2       posr[1]:     %f\n", posr[1]);
		fprintf(stderr, "dbg2       posr[2]:     %f\n", posr[2]);
		fprintf(stderr, "dbg2       posu[0]:     %f\n", posu[0]);
		fprintf(stderr, "dbg2       posu[1]:     %f\n", posu[1]);
		fprintf(stderr, "dbg2       posu[2]:     %f\n", posu[2]);
		fprintf(stderr, "dbg2       xlon:        %f\n", *xlon);
		fprintf(stderr, "dbg2       ylat:        %f\n", *ylat);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_sphere_matrix(double phi, double theta, double psi, double *eulermatrix) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       phi:              %f\n", phi);
		fprintf(stderr, "dbg2       theta:            %f\n", theta);
		fprintf(stderr, "dbg2       psi:              %f\n", psi);
	}

	/* The initial spherical coordinate system is defined as:
	        x = r * cos(longitude) * cos(latitude)
	        y = r * sin(longitude) * cos(latitude)
	        z = r * sin(latitude)
	   which is equivalent to:
	        x = r * cos(longitude) * sin(colatitude)
	        y = r * sin(longitude) * sin(colatitude)
	        z = r * cos(colatitude)
	   where:
	        colatitude = PI/2 - latitude

	   Euler's rotation theorem proves than any general rotation may be
	   described by three successive rotations about the axes. One convention
	   is to use first a rotation about the z-axis (angle phi), then a
	   rotation about the x'-axis (angle theta), and finally a rotation
	   about the z''-axis (angle psi).

	   The euler rotation matrix becomes:
	        |	a11	a12	a13	|
	        |	a21	a22	a23	|
	        |	a31	a32	a33	|
	   where:
	        a11 = cos(phi) * cos(psi) - sin(phi) * cos(theta) * sin(psi)
	        a12 = sin(phi) * cos(psi) + cos(phi) * cos(theta) * sin(psi)
	        a13 = sin(theta) * sin (psi)
	        a21 = -cos(phi) * sin(psi) - sin(phi) * cos(theta) * cos(psi)
	        a22 = -sin(phi) * sin(psi) + cos(phi) * cos(theta) * cos(psi)
	        a23 = sin(theta) * cos(psi)
	        a31 = sin(phi) * sin(theta)
	        a32 = -cos(phi) * sin(theta)
	        a33 = cos(theta)

	   We wish to rotate the coordinate system so that the reference position
	   defined by xlon and ylat are located on the positive z-axis. The forward
	   rotation is accomplished using:
	        phi = -PI/2 + xlon
	        theta = -ycolat = ylat - PI/2
	        psi = PI
	   The reverse rotation is accomplished using:
	        phi = -PI
	        theta = ycolat = PI/2 - ylat
	        psi = xlon - PI/2

	  The relevant equations derived in part from:
	    http://mathworld.wolfram.com/EulerAngles.html
	  which were viewed on January 19, 2004
	    */

	/* create forward rotation matrix */
	eulermatrix[0] = cos(phi) * cos(psi) - sin(phi) * cos(theta) * sin(psi);
	eulermatrix[1] = sin(phi) * cos(psi) + cos(phi) * cos(theta) * sin(psi);
	eulermatrix[2] = sin(theta) * sin(psi);
	eulermatrix[3] = -cos(phi) * sin(psi) - sin(phi) * cos(theta) * cos(psi);
	eulermatrix[4] = -sin(phi) * sin(psi) + cos(phi) * cos(theta) * cos(psi);
	eulermatrix[5] = sin(theta) * cos(psi);
	eulermatrix[6] = sin(phi) * sin(theta);
	eulermatrix[7] = -cos(phi) * sin(theta);
	eulermatrix[8] = cos(theta);

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       eulermatrix       %f %f %f\n", eulermatrix[0], eulermatrix[1], eulermatrix[2]);
		fprintf(stderr, "dbg2       eulermatrix       %f %f %f\n", eulermatrix[3], eulermatrix[4], eulermatrix[5]);
		fprintf(stderr, "dbg2       eulermatrix       %f %f %f\n", eulermatrix[6], eulermatrix[7], eulermatrix[8]);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_sphere_rotate(double *eulermatrix, double *v, double *vr) {
	int i, j;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       eulermatrix       %f %f %f\n", eulermatrix[0], eulermatrix[1], eulermatrix[2]);
		fprintf(stderr, "dbg2       eulermatrix       %f %f %f\n", eulermatrix[3], eulermatrix[4], eulermatrix[5]);
		fprintf(stderr, "dbg2       eulermatrix       %f %f %f\n", eulermatrix[6], eulermatrix[7], eulermatrix[8]);
		fprintf(stderr, "dbg2       -----------\n");
		fprintf(stderr, "dbg2       v:                %f %f %f\n", v[0], v[1], v[3]);
	}

	/* get original view direction in cartesian coordinates */
	for (i = 0; i < 3; i++)
		vr[i] = 0.0;
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			vr[j] += v[i] * eulermatrix[i + 3 * j];
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       vr:               %f %f %f\n", vr[0], vr[1], vr[3]);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_greatcircle_distbearing(size_t instance, double lon1, double lat1, double lon2, double lat2, double *bearing,
                                   double *distance) {
	double rlon1, rlat1, rlon2, rlat2, rbearing;
	double t1, t2, t3, dd;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       lon1:             %f\n", lon1);
		fprintf(stderr, "dbg2       lat1:             %f\n", lat1);
		fprintf(stderr, "dbg2       lon2:             %f\n", lon2);
		fprintf(stderr, "dbg2       lat2:             %f\n", lat2);
	}

	/* get view */
	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* note: these equations derive in part from source code read at:
	    http://simgear.org/doxygen/polar3d_8hxx-source.html
	    on 17 February 2004 by D.W. Caress
	    The source code found at this location is licensed under the LGPL */

	/* get great circle distance */
	rlon1 = DTR * lon1;
	rlat1 = DTR * lat1;
	rlon2 = DTR * lon2;
	rlat2 = DTR * lat2;
	t1 = sin(0.5 * (rlon1 - rlon2));
	t2 = sin(0.5 * (rlat1 - rlat2));
	dd = 2.0 * asin(sqrt(t2 * t2 + cos(rlat1) * cos(rlat2) * t1 * t1));
	*distance = MBV_SPHEROID_RADIUS * dd;

	/* get great circle bearing */

	/* first check if at poles */
	if (fabs(1.0 - sin(rlat1)) < 0.000001) {
		/* at north pole therefore heading south */
		if (lat1 > 0.0) {
			*bearing = 180.0;
		}

		/* at south pole therefore heading north */
		else {
			*bearing = 0.0;
		}
	}

	/* handle position away from poles */
	else {
		t3 = (sin(rlat2) - sin(rlat1) * cos(dd)) / (sin(dd) * cos(rlat1));
		rbearing = acos(MAX(MIN(t3, 1.0), -1.0));
		if (t1 <= 0.0) {
			*bearing = RTD * rbearing;
		}
		else {
			*bearing = 360.0 - RTD * rbearing;
		}
		if (*bearing < 0.0)
			*bearing += 360.0;
	}

	int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       t3:          %f\n", t3);
		fprintf(stderr, "dbg2       bearing:     %f\n", *bearing);
		fprintf(stderr, "dbg2       distance:    %f\n", *distance);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_greatcircle_dist(size_t instance, double lon1, double lat1, double lon2, double lat2, double *distance) {
	double rlon1, rlat1, rlon2, rlat2;
	double t1, t2, dd;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       lon1:             %f\n", lon1);
		fprintf(stderr, "dbg2       lat1:             %f\n", lat1);
		fprintf(stderr, "dbg2       lon2:             %f\n", lon2);
		fprintf(stderr, "dbg2       lat2:             %f\n", lat2);
	}

	/* get view */
	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* note: these equations derive in part from source code read at:
	    http://simgear.org/doxygen/polar3d_8hxx-source.html
	    on 17 February 2004 by D.W. Caress
	    The source code found at this location is licensed under the LGPL */

	/* get great circle distance */
	rlon1 = DTR * lon1;
	rlat1 = DTR * lat1;
	rlon2 = DTR * lon2;
	rlat2 = DTR * lat2;
	t1 = sin(0.5 * (rlon1 - rlon2));
	t2 = sin(0.5 * (rlat1 - rlat2));
	dd = 2.0 * asin(sqrt(t2 * t2 + cos(rlat1) * cos(rlat2) * t1 * t1));
	*distance = MBV_SPHEROID_RADIUS * dd;

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       distance:    %f\n", *distance);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_greatcircle_endposition(size_t instance, double lon1, double lat1, double bearing, double distance, double *lon2,
                                   double *lat2) {
	double rd, rbearing, rlon1, rlat1, rlat2;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       lon1:             %f\n", lon1);
		fprintf(stderr, "dbg2       lat1:             %f\n", lat1);
		fprintf(stderr, "dbg2       bearing:          %f\n", bearing);
		fprintf(stderr, "dbg2       distance:         %f\n", distance);
	}

	/* get view */
	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* note: these equations derive in part from source code read at:
	    http://simgear.org/doxygen/polar3d_8hxx-source.html
	    on 17 February 2004 by D.W. Caress
	    The source code found at this location is licensed under the LGPL */

	/* scale angles to radians */
	rd = distance / MBV_SPHEROID_RADIUS;
	rbearing = DTR * (360.0 - bearing);
	rlon1 = DTR * lon1;
	rlat1 = DTR * lat1;

	/* calculate latitude */
	rlat2 = asin(sin(rlat1) * cos(rd) + cos(rlat1) * sin(rd) * cos(rbearing));
	*lat2 = RTD * rlat2;

	/* calculate longitude */
	if (cos(rlat2) < 0.000001) {
		*lon2 = lon1;
	}
	else {
		*lon2 = RTD * (fmod(rlon1 - asin(sin(rbearing) * sin(rd) / cos(rlat2)) + M_PI, 2.0 * M_PI) - M_PI);
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       lon2:             %f\n", *lon2);
		fprintf(stderr, "dbg2       lat2:             %f\n", *lat2);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_colorclear(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "mbview_colorclear: %zu\n", instance);

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set status bit arrays */
	view->colordonecount = 0;
	if (data->primary_stat_color != NULL)
		memset(data->primary_stat_color, 0, (data->primary_nxy / 8) + 1);

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_zscaleclear(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "mbview_zscaleclear: %zu\n", instance);

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set status bit arrays */
	view->zscaledonecount = 0;
	if (data->primary_stat_z != NULL)
		memset(data->primary_stat_z, 0, (data->primary_nxy / 8) + 1);

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_setcolorparms(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get min max values for coloring */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		view->colortable = data->primary_colortable;
		view->colortable_mode = data->primary_colortable_mode;
		view->min = data->primary_colortable_min;
		view->max = data->primary_colortable_max;
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		view->colortable = data->slope_colortable;
		view->colortable_mode = data->slope_colortable_mode;
		view->min = data->slope_colortable_min;
		view->max = data->slope_colortable_max;
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
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
	if (view->shade_mode == MBV_SHADE_VIEW_ILLUMINATION) {
		view->illum_x = sin(DTR * data->illuminate_azimuth) * cos(DTR * data->illuminate_elevation);
		view->illum_y = cos(DTR * data->illuminate_azimuth) * cos(DTR * data->illuminate_elevation);
		view->illum_z = sin(DTR * data->illuminate_elevation);
		view->mag2 = data->illuminate_magnitude * data->illuminate_magnitude;
		/*fprintf(stderr,"ILLUMRAW: %f %f %f\n",
		data->illuminate_azimuth, data->illuminate_elevation, data->illuminate_magnitude);
		fprintf(stderr,"ILLUMLGT: %f %f %f %f\n",view->illum_x, view->illum_y, view->illum_z, view->mag2);*/
	}

	/* get sign of overlay shading if necessary */
	view->sign = 1.0;
	if (view->shade_mode == MBV_SHADE_VIEW_OVERLAY) {
		if (data->overlay_shade_mode == MBV_COLORTABLE_NORMAL)
			view->sign = 1.0;
		else
			view->sign = -1.0;
	}

	/* get colortable */
	if (view->colortable == MBV_COLORTABLE_HAXBY) {
		view->colortable_red = colortable_haxby_red;
		view->colortable_blue = colortable_haxby_blue;
		view->colortable_green = colortable_haxby_green;
	}
	else if (view->colortable == MBV_COLORTABLE_BRIGHT) {
		view->colortable_red = colortable_bright_red;
		view->colortable_blue = colortable_bright_blue;
		view->colortable_green = colortable_bright_green;
	}
	else if (view->colortable == MBV_COLORTABLE_MUTED) {
		view->colortable_red = colortable_muted_red;
		view->colortable_blue = colortable_muted_blue;
		view->colortable_green = colortable_muted_green;
	}
	else if (view->colortable == MBV_COLORTABLE_GRAY) {
		view->colortable_red = colortable_gray_red;
		view->colortable_blue = colortable_gray_blue;
		view->colortable_green = colortable_gray_green;
	}
	else if (view->colortable == MBV_COLORTABLE_FLAT) {
		view->colortable_red = colortable_flat_red;
		view->colortable_blue = colortable_flat_blue;
		view->colortable_green = colortable_flat_green;
	}
	else if (view->colortable == MBV_COLORTABLE_SEALEVEL1) {
		view->colortable_red = colortable_haxby_red;
		view->colortable_blue = colortable_haxby_blue;
		view->colortable_green = colortable_haxby_green;
	}
	else if (view->colortable == MBV_COLORTABLE_SEALEVEL2) {
		view->colortable_red = colortable_haxby_red;
		view->colortable_blue = colortable_haxby_blue;
		view->colortable_green = colortable_haxby_green;
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_make_histogram(struct mbview_world_struct *view, struct mbview_struct *data, int which_data) {
	int binned_counts[MBV_RAW_HISTOGRAM_DIM];
	int nbinned, nbinnedneg, nbinnedpos;
	int bindimminusone;
	float min, max, dhist;
	float slope;
	float *histogram;
	int binnedsum, target, jbinzero;
	int i, jbin, khist;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       view:             %p\n", view);
		fprintf(stderr, "dbg2       data:             %p\n", data);
		fprintf(stderr, "dbg2       which_data:       %d\n", which_data);
	}

	/* get ranges for histogram */
	if (which_data == MBV_DATA_PRIMARY) {
		histogram = view->primary_histogram;
		min = data->primary_colortable_min;
		max = data->primary_colortable_max;
		view->primary_histogram_set = true;
	}
	else if (which_data == MBV_DATA_PRIMARYSLOPE) {
		histogram = view->primaryslope_histogram;
		min = data->slope_colortable_min;
		max = data->slope_colortable_max;
		view->primaryslope_histogram_set = true;
	}
	else /* if (which_data == MBV_DATA_SECONDARY) */
	{
		histogram = view->secondary_histogram;
		min = data->secondary_colortable_min;
		max = data->secondary_colortable_max;
		view->secondary_histogram_set = true;
	}
	dhist = (max - min) / (MBV_RAW_HISTOGRAM_DIM - 1);

	/* initialize histograms */
	for (i = 0; i < 3 * MBV_NUM_COLORS; i++)
		histogram[i] = 0.0;

	/* initialize bins */
	for (i = 0; i < MBV_RAW_HISTOGRAM_DIM; i++)
		binned_counts[i] = 0;

	/* loop over all values binning quantities */
	bindimminusone = MBV_RAW_HISTOGRAM_DIM - 1;
	nbinned = 0;
	nbinnedneg = 0;
	nbinnedpos = 0;
	if (which_data == MBV_DATA_PRIMARY) {
		for (i = 0; i < data->primary_nxy; i++) {
			if (data->primary_data[i] != data->primary_nodatavalue) {
				jbin = (data->primary_data[i] - min) / dhist;
				if (jbin >= 0 && jbin <= bindimminusone) {
					binned_counts[jbin]++;
					nbinned++;
					if (data->primary_data[i] < 0.0)
						nbinnedneg++;
					else
						nbinnedpos++;
				}
			}
		}
	}
	else if (which_data == MBV_DATA_PRIMARYSLOPE) {
		for (i = 0; i < data->primary_nxy; i++) {
			if (data->primary_data[i] != data->primary_nodatavalue) {
				slope = sqrt(data->primary_dzdx[i] * data->primary_dzdx[i] + data->primary_dzdy[i] * data->primary_dzdy[i]);
				jbin = (slope - min) / dhist;
				if (jbin >= 0 && jbin <= bindimminusone) {
					binned_counts[jbin]++;
					nbinned++;
					nbinnedpos++;
				}
			}
		}
	}
	else if (which_data == MBV_DATA_SECONDARY) {
		for (i = 0; i < data->secondary_nxy; i++) {
			if (data->secondary_data[i] != data->secondary_nodatavalue) {
				jbin = (data->secondary_data[i] - min) / dhist;
				if (jbin >= 0 && jbin <= bindimminusone) {
					binned_counts[jbin]++;
					nbinned++;
					if (data->secondary_data[i] < 0.0)
						nbinnedneg++;
					else
						nbinnedpos++;
				}
			}
		}
	}

	/* construct histogram equalization for full data range */
	histogram[0] = min;
	histogram[MBV_NUM_COLORS - 1] = max;
	binnedsum = 0;
	khist = 1;
	for (jbin = 0; jbin < MBV_RAW_HISTOGRAM_DIM; jbin++) {
		target = (khist * nbinned) / (MBV_NUM_COLORS - 1);
		binnedsum += binned_counts[jbin];
		if (binnedsum >= target && khist < MBV_NUM_COLORS - 1) {
			histogram[khist] = min + jbin * dhist;
			khist++;
		}
	}

	/* construct histogram equalization for data < 0.0 */
	if (nbinnedneg > MBV_NUM_COLORS) {
		jbinzero = MIN(-min / dhist, MBV_RAW_HISTOGRAM_DIM - 1);
		histogram[MBV_NUM_COLORS] = MIN(0.0, min);
		histogram[2 * MBV_NUM_COLORS - 1] = MIN(0.0, max);
		binnedsum = 0;
		khist = 1;
		for (jbin = 0; jbin < jbinzero; jbin++) {
			target = (khist * nbinnedneg) / (MBV_NUM_COLORS - 1);
			binnedsum += binned_counts[jbin];
			if (binnedsum >= target && khist < MBV_NUM_COLORS - 1) {
				histogram[MBV_NUM_COLORS + khist] = min + jbin * dhist;
				khist++;
			}
		}
	}

	/* construct histogram equalization for data >= 0.0 */
	if (nbinnedpos > MBV_NUM_COLORS) {
		jbinzero = -min / dhist;
		histogram[2 * MBV_NUM_COLORS] = MAX(0.0, min);
		histogram[3 * MBV_NUM_COLORS - 1] = MAX(0.0, max);
		binnedsum = 0;
		khist = 1;
		for (jbin = jbinzero; jbin < MBV_RAW_HISTOGRAM_DIM; jbin++) {
			target = (khist * nbinnedpos) / (MBV_NUM_COLORS - 1);
			binnedsum += binned_counts[jbin];
			if (binnedsum >= target && khist < MBV_NUM_COLORS - 1) {
				histogram[2 * MBV_NUM_COLORS + khist] = min + jbin * dhist;
				khist++;
			}
		}
	}

	int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       Primary histogram:\n");
		for (i = 0; i < MBV_NUM_COLORS; i++)
			fprintf(stderr, "dbg2       value[%d]:   %f\n", i, histogram[i]);
		fprintf(stderr, "dbg2       Negative histogram for sea level colortable:\n");
		for (i = 0; i < MBV_NUM_COLORS; i++)
			fprintf(stderr, "dbg2       value[%d]:   %f\n", i, histogram[MBV_NUM_COLORS + i]);
		fprintf(stderr, "dbg2       Positive histogram for sea level colortable:\n");
		for (i = 0; i < MBV_NUM_COLORS; i++)
			fprintf(stderr, "dbg2       value[%d]:   %f\n", i, histogram[2 * MBV_NUM_COLORS + i]);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}


/*------------------------------------------------------------------------------*/
int mbview_colorvalue_instance(size_t instance, double value, float *r, float *g, float *b) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       value:            %f\n", value);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get histogram equalization if in use */
	float *histogram = NULL;
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY && data->primary_histogram) {
		histogram = view->primary_histogram;
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE && data->primaryslope_histogram) {
		histogram = view->primaryslope_histogram;
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY && data->secondary_histogram) {
		histogram = view->secondary_histogram;
	}

	// get color value using relevant data and histogram
	const int status = mbview_colorvalue(view, data, histogram, value, r, g, b);

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       r:       %f\n", *r);
		fprintf(stderr, "dbg2       g:       %f\n", *g);
		fprintf(stderr, "dbg2       b:       %f\n", *b);
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_colorvalue(struct mbview_world_struct *view, struct mbview_struct *data,
                      float *histogram, double value, float *r, float *g, float *b) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       view:             %p\n", view);
		fprintf(stderr, "dbg2       data:             %p\n", data);
		fprintf(stderr, "dbg2       histogram:        %p\n", histogram);
		fprintf(stderr, "dbg2       value:            %f\n", value);
	}

	/* get color without histogram */
  if (histogram == NULL) {
  	if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE && view->colortable < MBV_COLORTABLE_SEALEVEL1) {
  		mbview_getcolor(value, view->min, view->max, view->colortable_mode, (float)0.0, (float)0.0, (float)1.0, (float)1.0,
  						(float)0.0, (float)0.0, view->colortable_red, view->colortable_green, view->colortable_blue,
  						r, g, b);
  	}
  	else if (view->colortable < MBV_COLORTABLE_SEALEVEL1) {
  		mbview_getcolor(value, view->min, view->max, view->colortable_mode, view->colortable_red[0], view->colortable_green[0],
  						view->colortable_blue[0], view->colortable_red[MBV_NUM_COLORS - 1],
  						view->colortable_green[MBV_NUM_COLORS - 1], view->colortable_blue[MBV_NUM_COLORS - 1],
  						view->colortable_red, view->colortable_green, view->colortable_blue, r, g, b);
  	}
  	else if (view->colortable == MBV_COLORTABLE_SEALEVEL1) {
  		if (value > 0.0) {
  			if (view->colortable_mode == MBV_COLORTABLE_NORMAL) {
  				mbview_getcolor(value, 0.0, view->max, view->colortable_mode, colortable_abovesealevel1_red[0],
  								colortable_abovesealevel1_green[0], colortable_abovesealevel1_blue[0],
  								colortable_abovesealevel1_red[MBV_NUM_COLORS - 1],
  								colortable_abovesealevel1_green[MBV_NUM_COLORS - 1],
  								colortable_abovesealevel1_blue[MBV_NUM_COLORS - 1], colortable_abovesealevel1_red,
  								colortable_abovesealevel1_green, colortable_abovesealevel1_blue, r, g, b);
  			}
  			else {
  				mbview_getcolor(value, -view->max / 11.0, view->max, view->colortable_mode, colortable_haxby_red[0],
  								colortable_haxby_green[0], colortable_haxby_blue[0], colortable_haxby_red[MBV_NUM_COLORS - 1],
  								colortable_haxby_green[MBV_NUM_COLORS - 1], colortable_haxby_blue[MBV_NUM_COLORS - 1],
  								colortable_haxby_red, colortable_haxby_green, colortable_haxby_blue, r,
  								g, b);
  			}
  		}
  		else {
  			if (view->colortable_mode == MBV_COLORTABLE_REVERSED) {
  				mbview_getcolor(value, view->min, 0.0, view->colortable_mode, colortable_abovesealevel1_red[0],
  								colortable_abovesealevel1_green[0], colortable_abovesealevel1_blue[0],
  								colortable_abovesealevel1_red[MBV_NUM_COLORS - 1],
  								colortable_abovesealevel1_green[MBV_NUM_COLORS - 1],
  								colortable_abovesealevel1_blue[MBV_NUM_COLORS - 1], colortable_abovesealevel1_red,
  								colortable_abovesealevel1_green, colortable_abovesealevel1_blue, r, g, b);
  			}
  			else {
  				mbview_getcolor(value, view->min, -view->min / 11.0, view->colortable_mode, colortable_haxby_red[0],
  								colortable_haxby_green[0], colortable_haxby_blue[0], colortable_haxby_red[MBV_NUM_COLORS - 1],
  								colortable_haxby_green[MBV_NUM_COLORS - 1], colortable_haxby_blue[MBV_NUM_COLORS - 1],
  								colortable_haxby_red, colortable_haxby_green, colortable_haxby_blue, r, g, b);
  			}
  		}
  	}
  	else if (view->colortable == MBV_COLORTABLE_SEALEVEL2) {
  		if (value > 0.0) {
  			if (view->colortable_mode == MBV_COLORTABLE_NORMAL) {
  				mbview_getcolor(value, 0.0, view->max, view->colortable_mode, colortable_abovesealevel2_red[0],
  								colortable_abovesealevel2_green[0], colortable_abovesealevel2_blue[0],
  								colortable_abovesealevel2_red[MBV_NUM_COLORS - 1],
  								colortable_abovesealevel2_green[MBV_NUM_COLORS - 1],
  								colortable_abovesealevel2_blue[MBV_NUM_COLORS - 1], colortable_abovesealevel2_red,
  								colortable_abovesealevel2_green, colortable_abovesealevel2_blue, r, g, b);
  			}
  			else {
  				mbview_getcolor(value, -view->max / 11.0, view->max, view->colortable_mode, colortable_haxby_red[0],
  								colortable_haxby_green[0], colortable_haxby_blue[0], colortable_haxby_red[MBV_NUM_COLORS - 1],
  								colortable_haxby_green[MBV_NUM_COLORS - 1], colortable_haxby_blue[MBV_NUM_COLORS - 1],
  								colortable_haxby_red, colortable_haxby_green, colortable_haxby_blue, r, g, b);
  			}
  		}
  		else {
  			if (view->colortable_mode == MBV_COLORTABLE_REVERSED) {
  				mbview_getcolor(value, view->min, 0.0, view->colortable_mode, colortable_abovesealevel2_red[0],
  								colortable_abovesealevel2_green[0], colortable_abovesealevel2_blue[0],
  								colortable_abovesealevel2_red[MBV_NUM_COLORS - 1],
  								colortable_abovesealevel2_green[MBV_NUM_COLORS - 1],
  								colortable_abovesealevel2_blue[MBV_NUM_COLORS - 1], colortable_abovesealevel2_red,
  								colortable_abovesealevel2_green, colortable_abovesealevel2_blue, r, g, b);
  			}
  			else {
  				mbview_getcolor(value, view->min, -view->min / 11.0, view->colortable_mode, colortable_haxby_red[0],
  								colortable_haxby_green[0], colortable_haxby_blue[0], colortable_haxby_red[MBV_NUM_COLORS - 1],
  								colortable_haxby_green[MBV_NUM_COLORS - 1], colortable_haxby_blue[MBV_NUM_COLORS - 1],
  								colortable_haxby_red, colortable_haxby_green, colortable_haxby_blue, r, g, b);
  			}
  		}
  	}
  }

  /* get color using histogram */
  else {
  	if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE && view->colortable < MBV_COLORTABLE_SEALEVEL1) {
  		mbview_getcolor_histogram(value, view->min, view->max, view->colortable_mode, (float)0.0, (float)0.0, (float)1.0,
  		                          (float)1.0, (float)0.0, (float)0.0, view->colortable_red, view->colortable_green,
  		                          view->colortable_blue, histogram, r, g, b);
  	}
  	else if (view->colortable < MBV_COLORTABLE_SEALEVEL1) {
  		mbview_getcolor_histogram(value, view->min, view->max, view->colortable_mode, view->colortable_red[0],
  		                          view->colortable_green[0], view->colortable_blue[0], view->colortable_red[MBV_NUM_COLORS - 1],
  		                          view->colortable_green[MBV_NUM_COLORS - 1], view->colortable_blue[MBV_NUM_COLORS - 1],
  		                          view->colortable_red, view->colortable_green, view->colortable_blue, histogram,
  		                          r, g, b);
  	}
  	else if (view->colortable == MBV_COLORTABLE_SEALEVEL1) {
  		if (value > 0.0) {
  			if (view->colortable_mode == MBV_COLORTABLE_NORMAL) {
  				mbview_getcolor_histogram(
  				    value, 0.0, view->max, view->colortable_mode, colortable_abovesealevel1_red[0],
  				    colortable_abovesealevel1_green[0], colortable_abovesealevel1_blue[0],
  				    colortable_abovesealevel1_red[MBV_NUM_COLORS - 1], colortable_abovesealevel1_green[MBV_NUM_COLORS - 1],
  				    colortable_abovesealevel1_blue[MBV_NUM_COLORS - 1], colortable_abovesealevel1_red,
  				    colortable_abovesealevel1_green, colortable_abovesealevel1_blue, &(histogram[2 * MBV_NUM_COLORS]),
  				    r, g, b);
  			}
  			else {
  				mbview_getcolor_histogram(value, -view->max / 11.0, view->max, view->colortable_mode, colortable_haxby_red[0],
  				                          colortable_haxby_green[0], colortable_haxby_blue[0],
  				                          colortable_haxby_red[MBV_NUM_COLORS - 1], colortable_haxby_green[MBV_NUM_COLORS - 1],
  				                          colortable_haxby_blue[MBV_NUM_COLORS - 1], colortable_haxby_red, colortable_haxby_green,
  				                          colortable_haxby_blue, &(histogram[2 * MBV_NUM_COLORS]), r, g, b);
  			}
  		}
  		else {
  			if (view->colortable_mode == MBV_COLORTABLE_REVERSED) {
  				mbview_getcolor_histogram(
  				    value, view->min, 0.0, view->colortable_mode, colortable_abovesealevel1_red[0],
  				    colortable_abovesealevel1_green[0], colortable_abovesealevel1_blue[0],
  				    colortable_abovesealevel1_red[MBV_NUM_COLORS - 1], colortable_abovesealevel1_green[MBV_NUM_COLORS - 1],
  				    colortable_abovesealevel1_blue[MBV_NUM_COLORS - 1], colortable_abovesealevel1_red,
  				    colortable_abovesealevel1_green, colortable_abovesealevel1_blue, &(histogram[MBV_NUM_COLORS]),
  				    r, g, b);
  			}
  			else {
  				mbview_getcolor_histogram(value, view->min, -view->min / 11.0, view->colortable_mode, colortable_haxby_red[0],
  				                          colortable_haxby_green[0], colortable_haxby_blue[0],
  				                          colortable_haxby_red[MBV_NUM_COLORS - 1], colortable_haxby_green[MBV_NUM_COLORS - 1],
  				                          colortable_haxby_blue[MBV_NUM_COLORS - 1], colortable_haxby_red, colortable_haxby_green,
  				                          colortable_haxby_blue, &(histogram[MBV_NUM_COLORS]), r, g, b);
  			}
  		}
  	}
  	else if (view->colortable == MBV_COLORTABLE_SEALEVEL2) {
  		if (value > 0.0) {
  			if (view->colortable_mode == MBV_COLORTABLE_NORMAL) {
  				mbview_getcolor_histogram(
  				    value, 0.0, view->max, view->colortable_mode, colortable_abovesealevel2_red[0],
  				    colortable_abovesealevel2_green[0], colortable_abovesealevel2_blue[0],
  				    colortable_abovesealevel2_red[MBV_NUM_COLORS - 1], colortable_abovesealevel2_green[MBV_NUM_COLORS - 1],
  				    colortable_abovesealevel2_blue[MBV_NUM_COLORS - 1], colortable_abovesealevel2_red,
  				    colortable_abovesealevel2_green, colortable_abovesealevel2_blue, &(histogram[2 * MBV_NUM_COLORS]),
  				    r, g, b);
  			}
  			else {
  				mbview_getcolor_histogram(value, -view->max / 11.0, view->max, view->colortable_mode, colortable_haxby_red[0],
  				                          colortable_haxby_green[0], colortable_haxby_blue[0],
  				                          colortable_haxby_red[MBV_NUM_COLORS - 1], colortable_haxby_green[MBV_NUM_COLORS - 1],
  				                          colortable_haxby_blue[MBV_NUM_COLORS - 1], colortable_haxby_red, colortable_haxby_green,
  				                          colortable_haxby_blue, &(histogram[2 * MBV_NUM_COLORS]), r, g, b);
  			}
  		}
  		else {
  			if (view->colortable_mode == MBV_COLORTABLE_REVERSED) {
  				mbview_getcolor_histogram(
  				    value, view->min, 0.0, view->colortable_mode, colortable_abovesealevel2_red[0],
  				    colortable_abovesealevel2_green[0], colortable_abovesealevel2_blue[0],
  				    colortable_abovesealevel2_red[MBV_NUM_COLORS - 1], colortable_abovesealevel2_green[MBV_NUM_COLORS - 1],
  				    colortable_abovesealevel2_blue[MBV_NUM_COLORS - 1], colortable_abovesealevel2_red,
  				    colortable_abovesealevel2_green, colortable_abovesealevel2_blue, &(histogram[MBV_NUM_COLORS]),
  				    r, g, b);
  			}
  			else {
  				mbview_getcolor_histogram(value, view->min, -view->min / 11.0, view->colortable_mode, colortable_haxby_red[0],
  				                          colortable_haxby_green[0], colortable_haxby_blue[0],
  				                          colortable_haxby_red[MBV_NUM_COLORS - 1], colortable_haxby_green[MBV_NUM_COLORS - 1],
  				                          colortable_haxby_blue[MBV_NUM_COLORS - 1], colortable_haxby_red, colortable_haxby_green,
  				                          colortable_haxby_blue, &(histogram[MBV_NUM_COLORS]), r, g, b);
  			}
  		}
  	}
  }

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       r:       %f\n", *r);
		fprintf(stderr, "dbg2       g:       %f\n", *g);
		fprintf(stderr, "dbg2       b:       %f\n", *b);
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_colorpoint(struct mbview_world_struct *view, struct mbview_struct *data,
                                float *histogram, int i, int j, int k) {
	double value = 00;
  double svalue, dd;
	double intensity;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       view:             %p\n", view);
		fprintf(stderr, "dbg2       data:             %p\n", data);
		fprintf(stderr, "dbg2       histogram:        %p\n", histogram);
		fprintf(stderr, "dbg2       i:                %d\n", i);
		fprintf(stderr, "dbg2       j:                %d\n", j);
		fprintf(stderr, "dbg2       k:                %d\n", k);
	}

	/* get values for coloring */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		value = data->primary_data[k];
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		value = sqrt(data->primary_dzdx[k] * data->primary_dzdx[k] + data->primary_dzdy[k] * data->primary_dzdy[k]);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		if (data->secondary_sameas_primary) {
			value = data->secondary_data[k];
    }
		else {
			mbview_getsecondaryvalue(view, data, i, j, &value);
    }
	}

	/* get color for value using current color mode, color table, and histogram */
	int status = mbview_colorvalue(view, data, histogram, value, &data->primary_r[k], &data->primary_g[k], &data->primary_b[k]);

	/* get values for shading */
	if (view->shade_mode != MBV_SHADE_VIEW_NONE) {
		if (view->shade_mode == MBV_SHADE_VIEW_ILLUMINATION) {
			dd = sqrt(view->mag2 * data->primary_dzdx[k] * data->primary_dzdx[k] +
			          view->mag2 * data->primary_dzdy[k] * data->primary_dzdy[k] + 1.0);
			intensity = data->illuminate_magnitude * view->illum_x * data->primary_dzdx[k] / dd +
			            data->illuminate_magnitude * view->illum_y * data->primary_dzdy[k] / dd + view->illum_z / dd - 0.5;
			/*if (j==25)
			fprintf(stderr,"intensity:%f  dzdx:%f  dzdy:%f\n",
			intensity,data->primary_dzdx[k], data->primary_dzdy[k]);
			*/

			mbview_applyshade(intensity, &data->primary_r[k], &data->primary_g[k], &data->primary_b[k]);
		}
		else if (view->shade_mode == MBV_SHADE_VIEW_SLOPE) {
			intensity = -data->slope_magnitude *
			            sqrt(data->primary_dzdx[k] * data->primary_dzdx[k] + data->primary_dzdy[k] * data->primary_dzdy[k]);
			intensity = MAX(intensity, -1.0);
			mbview_applyshade(intensity, &data->primary_r[k], &data->primary_g[k], &data->primary_b[k]);
		}
		else if (view->shade_mode == MBV_SHADE_VIEW_OVERLAY) {
			if (data->secondary_sameas_primary)
				svalue = data->secondary_data[k];
			else
				mbview_getsecondaryvalue(view, data, i, j, &svalue);
			if (svalue != data->secondary_nodatavalue) {
				intensity = view->sign * data->overlay_shade_magnitude * (svalue - data->overlay_shade_center) /
				            (data->secondary_max - data->secondary_min);
				mbview_applyshade(intensity, &data->primary_r[k], &data->primary_g[k], &data->primary_b[k]);
			}
		}
	}

	/* set color status bit */
	data->primary_stat_color[k / 8] = data->primary_stat_color[k / 8] | statmask[k % 8];

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_getcolor(double value, double min, double max, int colortablemode, float below_red, float below_green,
                    float below_blue, float above_red, float above_green, float above_blue, float *colortable_red,
                    float *colortable_green, float *colortable_blue, float *red, float *green, float *blue) {
	int i;
	double ff, factor;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       value:            %f\n", value);
		fprintf(stderr, "dbg2       min:              %f\n", min);
		fprintf(stderr, "dbg2       max:              %f\n", max);
		fprintf(stderr, "dbg2       colortablemode:   %d\n", colortablemode);
		fprintf(stderr, "dbg2       below_red:        %f\n", below_red);
		fprintf(stderr, "dbg2       below_green:      %f\n", below_green);
		fprintf(stderr, "dbg2       below_blue:       %f\n", below_blue);
		fprintf(stderr, "dbg2       above_red:        %f\n", above_red);
		fprintf(stderr, "dbg2       above_green:      %f\n", above_green);
		fprintf(stderr, "dbg2       above_blue:       %f\n", above_blue);
		for (i = 0; i < MBV_NUM_COLORS; i++)
			fprintf(stderr, "dbg2       colortable_red[%d]:   %f\n", i, colortable_red[i]);
		for (i = 0; i < MBV_NUM_COLORS; i++)
			fprintf(stderr, "dbg2       colortable_green[%d]: %f\n", i, colortable_green[i]);
		for (i = 0; i < MBV_NUM_COLORS; i++)
			fprintf(stderr, "dbg2       colortable_blue[%d]:  %f\n", i, colortable_blue[i]);
	}

	/* get color */
  if (max <= min)
    factor = 0.5;
  else if (colortablemode == MBV_COLORTABLE_NORMAL)
		factor = (max - value) / (max - min);
	else
		factor = (value - min) / (max - min);
	if (factor >= 1.0) {
		*red = above_red;
		*green = above_green;
		*blue = above_blue;
	}
	else if (factor <= 0.0) {
		*red = below_red;
		*green = below_green;
		*blue = below_blue;
	}
	else {
		i = (int)(factor * (MBV_NUM_COLORS - 1));
		ff = factor * (MBV_NUM_COLORS - 1) - i;
		*red = colortable_red[i] + ff * (colortable_red[i + 1] - colortable_red[i]);
		*green = colortable_green[i] + ff * (colortable_green[i + 1] - colortable_green[i]);
		*blue = colortable_blue[i] + ff * (colortable_blue[i + 1] - colortable_blue[i]);
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       red:     %f\n", *red);
		fprintf(stderr, "dbg2       green:   %f\n", *green);
		fprintf(stderr, "dbg2       blue:    %f\n", *blue);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_getcolor_histogram(double value, double min, double max, int colortablemode, float below_red, float below_green,
                              float below_blue, float above_red, float above_green, float above_blue, float *colortable_red,
                              float *colortable_green, float *colortable_blue, float *histogram, float *red, float *green,
                              float *blue) {
	double ff, factor;
	bool found;
	int i, ii;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       value:            %f\n", value);
		fprintf(stderr, "dbg2       min:              %f\n", min);
		fprintf(stderr, "dbg2       max:              %f\n", max);
		fprintf(stderr, "dbg2       colortablemode:   %d\n", colortablemode);
		fprintf(stderr, "dbg2       below_red:        %f\n", below_red);
		fprintf(stderr, "dbg2       below_green:      %f\n", below_green);
		fprintf(stderr, "dbg2       below_blue:       %f\n", below_blue);
		fprintf(stderr, "dbg2       above_red:        %f\n", above_red);
		fprintf(stderr, "dbg2       above_green:      %f\n", above_green);
		fprintf(stderr, "dbg2       above_blue:       %f\n", above_blue);
		for (i = 0; i < MBV_NUM_COLORS; i++) {
			fprintf(stderr, "dbg2       colortable:       r:%f g:%f b:%f histogram: %f\n", colortable_red[i], colortable_green[i],
			        colortable_blue[i], histogram[i]);
		}
	}

	/* get color */
	if (colortablemode == MBV_COLORTABLE_NORMAL)
		factor = (max - value) / (max - min);
	else
		factor = (value - min) / (max - min);
	/* factor = MAX(factor, 0.000001);
	factor = MIN(factor, 0.999999);*/
	if (factor <= 0.0) {
		*red = below_red;
		*green = below_green;
		*blue = below_blue;
	}
	else if (factor >= 1.0) {
		*red = above_red;
		*green = above_green;
		*blue = above_blue;
	}
	else {
		/* find place in histogram */
		found = false;
		for (i = 0; i < MBV_NUM_COLORS - 1 && !found; i++) {
			if (value >= histogram[i] && value <= histogram[i + 1]) {
				ii = i;
				found = true;
			}
		}

		/* get color */
		if (colortablemode == MBV_COLORTABLE_NORMAL) {
			ff = (histogram[ii + 1] - value) / (histogram[ii + 1] - histogram[ii]);
			ii = MBV_NUM_COLORS - 2 - ii;
		}
		else {
			ff = (value - histogram[ii]) / (histogram[ii + 1] - histogram[ii]);
		}
		*red = colortable_red[ii] + ff * (colortable_red[ii + 1] - colortable_red[ii]);
		*green = colortable_green[ii] + ff * (colortable_green[ii + 1] - colortable_green[ii]);
		*blue = colortable_blue[ii] + ff * (colortable_blue[ii + 1] - colortable_blue[ii]);
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       red:     %f\n", *red);
		fprintf(stderr, "dbg2       green:   %f\n", *green);
		fprintf(stderr, "dbg2       blue:    %f\n", *blue);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_applyshade(double intensity, float *r, float *g, float *b) {
	/* note - this correction algorithm is taken from the GMT Technical
	   Reference and Cookbook by Wessel and Smith - you can find it in
	   Appendix I: Color Space - The final frontier */
	double h, s, v;
	double vmax, vmin, dv, idv;
	double rmod, gmod, bmod;
	double f, p, q, t;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       intensity:           %f\n", intensity);
	}

	/* change the initial rgb into hsv space */
	vmax = MAX(MAX(*r, *g), *b);
	vmin = MIN(MIN(*r, *g), *b);
	dv = vmax - vmin;
	v = vmax;
	if (vmax == 0.0)
		s = 0.0;
	else
		s = dv / vmax;
	h = 0.0;
	if (s > 0.0) {
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
	if (intensity > 0) {
		if (s != 0.0)
			s = (1.0 - intensity) * s + intensity * 0.1;
		v = (1.0 - intensity) * v + intensity;
	}
	else {
		if (s != 0.0)
			s = (1.0 + intensity) * s - intensity;
		v = (1.0 + intensity) * v - intensity * 0.3;
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
	if (s == 0.0) {
		*r = v;
		*g = v;
		*b = v;
	}
	else {
		while (h >= 360.0)
			h -= 360.0;
		h /= 60.0;
		f = h - ((int)h);
		p = v * (1.0 - s);
		q = v * (1.0 - (s * f));
		t = v * (1.0 - (s * (1.0 - f)));
		switch (((int)h)) {
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

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       red:     %f\n", *r);
		fprintf(stderr, "dbg2       green:   %f\n", *g);
		fprintf(stderr, "dbg2       blue:    %f\n", *b);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_getsecondaryvalue(struct mbview_world_struct *view, struct mbview_struct *data, int i, int j,
                             double *secondary_value) {
	int error = MB_ERROR_NO_ERROR;
	double xlon, ylat;
	double xgrid, ygrid;
	double xsgrid, ysgrid;
	int ii, jj, kk;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       view:             %p\n", view);
		fprintf(stderr, "dbg2       data:             %p\n", data);
		fprintf(stderr, "dbg2       i:                %d\n", i);
		fprintf(stderr, "dbg2       j:                %d\n", j);
	}

	/* get position in primary grid */
	xgrid = data->primary_xmin + i * data->primary_dx;
	ygrid = data->primary_ymin + j * data->primary_dy;

	/* get lon and lat of desired position */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED ||
	    data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
		mb_proj_inverse(mbv_verbose, view->primary_pjptr, xgrid, ygrid, &xlon, &ylat, &error);
	}
	else {
		xlon = xgrid;
		ylat = ygrid;
	}

	/* get position in secondary grid coordinates */
	if (data->secondary_grid_projection_mode == MBV_PROJECTION_PROJECTED ||
	    data->secondary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
if (ylat > 90.0) {
fprintf(stderr, "%s:%d:%s: Warning: calling mb_proj_forward with invalid latitude: lon: %f lat: %f\n",
__FILE__, __LINE__, __FUNCTION__, xlon, ylat);
}
		mb_proj_forward(mbv_verbose, view->secondary_pjptr, xlon, ylat, &xsgrid, &ysgrid, &error);
	}
	else {
		xsgrid = xlon;
		ysgrid = ylat;
	}

	/* get rounded location in secondary grid */
	ii = (xsgrid - data->secondary_xmin) / data->secondary_dx;
	jj = (ysgrid - data->secondary_ymin) / data->secondary_dy;

	/* answer only defined within grid bounds */
	if (ii < 0 || ii >= data->secondary_n_columns || jj < 0 || jj >= data->secondary_n_rows) {
		*secondary_value = data->secondary_nodatavalue;
	} else {
		kk = ii * data->secondary_n_rows + jj;
		*secondary_value = data->secondary_data[kk];
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       secondary_value:  %f\n", *secondary_value);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_contour(size_t instance, int rez) {
	int i, j, k, l, kk;
	int stride;
	int vertex[4];
	bool triangleA;
	bool triangleB;
	int nlevel, level_min, level_max;
	int nvertex, nside;
	float level_value, datamin, datamax;
	float factor;
	float xx[2], yy[2], zz[2];
	bool global;
	double contour_offset_factor;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       rez:              %d\n", rez);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set stride for looping over data */
	if (rez == MBV_REZ_FULL)
		stride = 1;
	else if (rez == MBV_REZ_HIGH)
		stride = MAX((int)ceil(((double)data->primary_n_columns) / ((double)data->hirez_dimension)),
		             (int)ceil(((double)data->primary_n_rows) / ((double)data->hirez_dimension)));
	else
		stride = MAX((int)ceil(((double)data->primary_n_columns) / ((double)data->lorez_dimension)),
		             (int)ceil(((double)data->primary_n_rows) / ((double)data->lorez_dimension)));

	/* start openGL list */
	if (rez == MBV_REZ_FULL) {
		glNewList((GLuint)(3 * instance + 3), GL_COMPILE);
	}
	else if (rez == MBV_REZ_HIGH) {
		glNewList((GLuint)(3 * instance + 2), GL_COMPILE);
	}
	else {
		glNewList((GLuint)(3 * instance + 1), GL_COMPILE);
	}
	glColor3f(0.0, 0.0, 0.0);
	glLineWidth(1.0);
	glBegin(GL_LINES);

	if (mbv_verbose >= 2)
		fprintf(stderr, "mbview_contour: instance:%zu rez:%d stride:%d contour interval:%f\n", instance, rez, stride,
		        data->contour_interval);

	/* check if the contour offset needs to be applied in a global spherical direction or just up */
	if (data->display_projection_mode == MBV_PROJECTION_SPHEROID && view->sphere_refx == 0.0 && view->sphere_refy == 0.0 &&
	    view->sphere_refz == 0.0) {
		global = true;
		contour_offset_factor = MBV_OPENGL_3D_CONTOUR_OFFSET / (view->scale * MBV_SPHEROID_RADIUS);
	}
	else {
		global = false;
		contour_offset_factor = MBV_OPENGL_3D_CONTOUR_OFFSET;
	}

	/* construct the contour segments in each triangle */
	for (i = 0; i < data->primary_n_columns - stride; i += stride) {
		for (j = 0; j < data->primary_n_rows - stride; j += stride) {
			/* get vertex id's */
			vertex[0] = i * data->primary_n_rows + j;
			vertex[1] = (i + stride) * data->primary_n_rows + j;
			vertex[2] = i * data->primary_n_rows + j + stride;
			vertex[3] = (i + stride) * data->primary_n_rows + j + stride;

			/* check if either triangle can be contoured */
			triangleA = false;
			triangleB = false;
			if (data->primary_data[vertex[0]] != data->primary_nodatavalue &&
			    data->primary_data[vertex[1]] != data->primary_nodatavalue &&
			    data->primary_data[vertex[2]] != data->primary_nodatavalue)
				triangleA = true;
			if (data->primary_data[vertex[1]] != data->primary_nodatavalue &&
			    data->primary_data[vertex[3]] != data->primary_nodatavalue &&
			    data->primary_data[vertex[2]] != data->primary_nodatavalue)
				triangleB = true;

			/* if at least one triangle is valid, contour it */
			if (triangleA || triangleB) {
				/* get min max values and number of contours */
				nvertex = 0;
				datamin = 0.0;
				datamax = 0.0;
				for (kk = 0; kk < 4; kk++) {
					k = vertex[kk];
					if (data->primary_data[k] != data->primary_nodatavalue) {
						if (!(data->primary_stat_z[k / 8] & statmask[k % 8]))
							mbview_zscalegridpoint(instance, k);
						if (nvertex == 0) {
							datamin = data->primary_data[k];
							datamax = data->primary_data[k];
						}
						else {
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
				if (nlevel > 0) {
					for (l = level_min; l <= level_max; l++) {
						level_value = l * data->contour_interval;

						/* deal with triangle A - vertexes 0, 1, and 2 */
						if (triangleA) {
							nside = 0;
							if ((data->primary_data[vertex[0]] > level_value && data->primary_data[vertex[1]] < level_value) ||
							    (data->primary_data[vertex[0]] < level_value && data->primary_data[vertex[1]] > level_value)) {
								factor = (level_value - data->primary_data[vertex[0]]) /
								         (data->primary_data[vertex[1]] - data->primary_data[vertex[0]]);
								xx[nside] = data->primary_x[vertex[0]] +
								            factor * (data->primary_x[vertex[1]] - data->primary_x[vertex[0]]);
								yy[nside] = data->primary_y[vertex[0]] +
								            factor * (data->primary_y[vertex[1]] - data->primary_y[vertex[0]]);
								zz[nside] = data->primary_z[vertex[0]] +
								            factor * (data->primary_z[vertex[1]] - data->primary_z[vertex[0]]);
								nside++;
							}
							if ((data->primary_data[vertex[1]] > level_value && data->primary_data[vertex[2]] < level_value) ||
							    (data->primary_data[vertex[1]] < level_value && data->primary_data[vertex[2]] > level_value)) {
								factor = (level_value - data->primary_data[vertex[1]]) /
								         (data->primary_data[vertex[2]] - data->primary_data[vertex[1]]);
								xx[nside] = data->primary_x[vertex[1]] +
								            factor * (data->primary_x[vertex[2]] - data->primary_x[vertex[1]]);
								yy[nside] = data->primary_y[vertex[1]] +
								            factor * (data->primary_y[vertex[2]] - data->primary_y[vertex[1]]);
								zz[nside] = data->primary_z[vertex[1]] +
								            factor * (data->primary_z[vertex[2]] - data->primary_z[vertex[1]]);
								nside++;
							}
							if (nside < 2 &&
							    ((data->primary_data[vertex[2]] > level_value && data->primary_data[vertex[0]] < level_value) ||
							     (data->primary_data[vertex[2]] < level_value && data->primary_data[vertex[0]] > level_value))) {
								factor = (level_value - data->primary_data[vertex[2]]) /
								         (data->primary_data[vertex[0]] - data->primary_data[vertex[2]]);
								xx[nside] = data->primary_x[vertex[2]] +
								            factor * (data->primary_x[vertex[0]] - data->primary_x[vertex[2]]);
								yy[nside] = data->primary_y[vertex[2]] +
								            factor * (data->primary_y[vertex[0]] - data->primary_y[vertex[2]]);
								zz[nside] = data->primary_z[vertex[2]] +
								            factor * (data->primary_z[vertex[0]] - data->primary_z[vertex[2]]);
								nside++;
							}
							if (nside == 2) {
								if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
									zz[0] += contour_offset_factor;
									zz[1] += contour_offset_factor;
								}
								else if (global) {
									xx[0] += xx[0] * contour_offset_factor;
									yy[0] += yy[0] * contour_offset_factor;
									zz[0] += zz[0] * contour_offset_factor;
									xx[1] += xx[1] * contour_offset_factor;
									yy[1] += yy[1] * contour_offset_factor;
									zz[1] += zz[1] * contour_offset_factor;
								}
								else {
									zz[0] += contour_offset_factor;
									zz[1] += contour_offset_factor;
								}
								glVertex3f(xx[0], yy[0], zz[0]);
								glVertex3f(xx[1], yy[1], zz[1]);
							}
						}

						/* deal with triangle B - vertexes 1, 3, and 2 */
						if (triangleB) {
							nside = 0;
							if ((data->primary_data[vertex[1]] > level_value && data->primary_data[vertex[3]] < level_value) ||
							    (data->primary_data[vertex[1]] < level_value && data->primary_data[vertex[3]] > level_value)) {
								factor = (level_value - data->primary_data[vertex[1]]) /
								         (data->primary_data[vertex[3]] - data->primary_data[vertex[1]]);
								xx[nside] = data->primary_x[vertex[1]] +
								            factor * (data->primary_x[vertex[3]] - data->primary_x[vertex[1]]);
								yy[nside] = data->primary_y[vertex[1]] +
								            factor * (data->primary_y[vertex[3]] - data->primary_y[vertex[1]]);
								zz[nside] = data->primary_z[vertex[1]] +
								            factor * (data->primary_z[vertex[3]] - data->primary_z[vertex[1]]);
								nside++;
							}
							if ((data->primary_data[vertex[3]] > level_value && data->primary_data[vertex[2]] < level_value) ||
							    (data->primary_data[vertex[3]] < level_value && data->primary_data[vertex[2]] > level_value)) {
								factor = (level_value - data->primary_data[vertex[3]]) /
								         (data->primary_data[vertex[2]] - data->primary_data[vertex[3]]);
								xx[nside] = data->primary_x[vertex[3]] +
								            factor * (data->primary_x[vertex[2]] - data->primary_x[vertex[3]]);
								yy[nside] = data->primary_y[vertex[3]] +
								            factor * (data->primary_y[vertex[2]] - data->primary_y[vertex[3]]);
								zz[nside] = data->primary_z[vertex[3]] +
								            factor * (data->primary_z[vertex[2]] - data->primary_z[vertex[3]]);
								nside++;
							}
							if (nside < 2 &&
							    ((data->primary_data[vertex[2]] > level_value && data->primary_data[vertex[1]] < level_value) ||
							     (data->primary_data[vertex[2]] < level_value && data->primary_data[vertex[1]] > level_value))) {
								factor = (level_value - data->primary_data[vertex[2]]) /
								         (data->primary_data[vertex[1]] - data->primary_data[vertex[2]]);
								xx[nside] = data->primary_x[vertex[2]] +
								            factor * (data->primary_x[vertex[1]] - data->primary_x[vertex[2]]);
								yy[nside] = data->primary_y[vertex[2]] +
								            factor * (data->primary_y[vertex[1]] - data->primary_y[vertex[2]]);
								zz[nside] = data->primary_z[vertex[2]] +
								            factor * (data->primary_z[vertex[1]] - data->primary_z[vertex[2]]);
								nside++;
							}
							if (nside == 2) {
								if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
									zz[0] += contour_offset_factor;
									zz[1] += contour_offset_factor;
								}
								else if (global) {
									xx[0] += xx[0] * contour_offset_factor;
									yy[0] += yy[0] * contour_offset_factor;
									zz[0] += zz[0] * contour_offset_factor;
									xx[1] += xx[1] * contour_offset_factor;
									yy[1] += yy[1] * contour_offset_factor;
									zz[1] += zz[1] * contour_offset_factor;
								}
								else {
									zz[0] += contour_offset_factor;
									zz[1] += contour_offset_factor;
								}
								glVertex3f(xx[0], yy[0], zz[0]);
								glVertex3f(xx[1], yy[1], zz[1]);
							}
						}
					}
				}
			}
		}

		/* check for pending event */
		if (!view->plot_done && view->plot_interrupt_allowed && i % MBV_EVENTCHECKCOARSENESS == 0)
			do_mbview_xevents();

		/* dump out of loop if plotting already done at a higher recursion */
		if (view->plot_done)
			i = data->primary_n_columns;
	}

	/* end openGL list */
	glEnd();
	glEndList();

	/* set rez flag only if plotting not done */
	if (!view->plot_done) {
		if (rez == MBV_REZ_FULL) {
			view->contourfullrez = true;
		}
		else if (rez == MBV_REZ_HIGH) {
			view->contourhirez = true;
		}
		else {
			view->contourlorez = true;
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_getzdata(size_t instance, double xgrid, double ygrid, bool *found, double *zdata) {
	int nsum;
	double zdatasum;
	int i, j, k, l, m, n;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       xgrid:            %f\n", xgrid);
		fprintf(stderr, "dbg2       ygrid:            %f\n", ygrid);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get location in grid */
	i = (int)((xgrid - data->primary_xmin) / data->primary_dx);
	j = (int)((ygrid - data->primary_ymin) / data->primary_dy);

	/* fail if outside grid */
	if (i < 0 || i >= data->primary_n_columns - 1 || j < 0 || j >= data->primary_n_rows - 1) {
		*found = false;
		*zdata = 0.0;
	}

	/* check all four points and average the good ones */
	else {
		k = i * data->primary_n_rows + j;
		l = (i + 1) * data->primary_n_rows + j;
		m = i * data->primary_n_rows + j + 1;
		n = (i + 1) * data->primary_n_rows + j + 1;
		nsum = 0;
		zdatasum = 0.0;
		if (data->primary_data[k] != data->primary_nodatavalue) {
			zdatasum += data->primary_data[k];
			nsum++;
		}
		if (data->primary_data[l] != data->primary_nodatavalue) {
			zdatasum += data->primary_data[l];
			nsum++;
		}
		if (data->primary_data[m] != data->primary_nodatavalue) {
			zdatasum += data->primary_data[m];
			nsum++;
		}
		if (data->primary_data[n] != data->primary_nodatavalue) {
			zdatasum += data->primary_data[n];
			nsum++;
		}
		if (nsum > 0) {
			*zdata = zdatasum / nsum;
			*found = true;
		}
		else {
			*zdata = 0.0;
			*found = false;
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       found:           %d\n", *found);
		fprintf(stderr, "dbg2       zdata:           %f\n", *zdata);
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
