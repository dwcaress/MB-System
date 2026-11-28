/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_render_core.c  11/15/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * What a renderer that draws the core's arrays without OpenGL (the QRhi
 * backend) needs from the private part of the mbview core: the camera of the
 * map view, the node arrays of the primary grid with the modes they are drawn
 * in, which lattice nodes a triangle of the drawing may use, where nodes lie
 * (their longitude and latitude, and the display place of a longitude,
 * latitude and value, for the spheroid's tile boxes), and which contours are
 * made. Only plain C types cross to the Qt half (mbvq_bridge.h).
 * Compiled in every build, like the CPU pick (mbvq_pick_cpu.c); nothing here
 * draws or changes the core's state, apart from the view extents that
 * mbview_camera() sets as a drawing does.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

/* the private headers keep a Windows OpenGL context type (HGLRC) */
#ifdef _WIN32
#include <windows.h>
#endif

#include "mbv_xtypes.h"
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"
#include "Mb3dsdg.h"
#include "mb3dsoundingsprivate.h"
#include "mbview.h"
#include "mbviewprivate.h"

#include "mbvq_bridge.h"

/* Under MBV_GET_GLX_ERRORS (a switch in mbviewprivate.h, or a -D flag) the
   soundings fill calls mbview_glerrorcheck(), that is glGetError(), without
   asking whether it draws with OpenGL (mbv_soundings_core.c). A QRhi view
   fills its batch without any OpenGL context, so the switch is for builds
   with the OpenGL backend only. */
#if defined(MBV_GET_GLX_ERRORS) && defined(MBVQ_HAVE_RHI)
#error "MBV_GET_GLX_ERRORS calls OpenGL while a QRhi view fills its batch: do not define it in a build with the QRhi backend"
#endif

/*------------------------------------------------------------------------------*/
int mbvq_c_camera(size_t instance, double projection[16], double modelview[16]) {
	return (mbview_camera(instance, projection, modelview));
}
/*------------------------------------------------------------------------------*/
void mbvq_c_grid(size_t instance, struct mbvq_grid *grid) {
	const struct mbview_struct *data = &(mbviews[instance].data);

	memset(grid, 0, sizeof(*grid));
	if (data->primary_data != NULL && data->primary_nxy > 0) {
		grid->n_columns = data->primary_n_columns;
		grid->n_rows = data->primary_n_rows;
		grid->data = data->primary_data;
		grid->x = data->primary_x;
		grid->y = data->primary_y;
		grid->z = data->primary_z;
		grid->r = data->primary_r;
		grid->g = data->primary_g;
		grid->b = data->primary_b;
	}
	grid->nodata = data->primary_nodatavalue;
	grid->grid_mode = data->grid_mode;
	/* the shading the colouring applies, as mbview_setcolorparms() takes it */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
		grid->shade_mode = data->slope_shade_mode;
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
		grid->shade_mode = data->secondary_shade_mode;
	else
		grid->shade_mode = data->primary_shade_mode;
	grid->display_mode = data->display_mode;
	grid->projection_mode = data->display_projection_mode;
	grid->secondary_sameas_primary = data->secondary_sameas_primary;
	grid->contours = data->grid_contour_mode == MBV_VIEW_ON;
	for (int i = 0; i < 4; i++)
		grid->viewbounds[i] = data->viewbounds[i];
	grid->scale = mbviews[instance].scale;
	grid->exaggeration = data->exaggeration;
	grid->zorigin = mbviews[instance].zorigin;
}
/*------------------------------------------------------------------------------*/
void mbvq_c_node_validity(size_t instance, int i0, int i1, int j0, int j1, int stride, unsigned char *valid) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	if (stride < 1 || i1 < i0 || j1 < j0)
		return;
	const int nj = (j1 - j0) / stride + 1;
	const bool secondary = data->grid_mode == MBV_GRID_VIEW_SECONDARY;
	int a = 0;
	for (int i = i0; i <= i1; i += stride, a++) {
		int b = 0;
		for (int j = j0; j <= j1; j += stride, b++) {
			unsigned char *v = &valid[(size_t)a * nj + b];
			*v = 0;
			if (data->primary_data == NULL || i < 0 || i >= data->primary_n_columns || j < 0 || j >= data->primary_n_rows)
				continue;
			const int k = i * data->primary_n_rows + j;
			if (data->primary_data[k] == data->primary_nodatavalue)
				continue;

			/* in the secondary grid mode a node needs secondary data too, as in
			   the drawing's secondary loop (mbview_drawdata) */
			if (secondary) {
				if (data->secondary_data == NULL)
					continue;
				double secondary_value;
				if (data->secondary_sameas_primary)
					secondary_value = data->secondary_data[k];
				else
					mbview_getsecondaryvalue(view, data, i, j, &secondary_value);
				if (secondary_value == data->secondary_nodatavalue)
					continue;
			}
			*v = 1;
		}
	}
}
/*------------------------------------------------------------------------------*/
void mbvq_c_node_lonlat(size_t instance, int n, const int *i, const int *j, double *lonlat) {
	const struct mbview_struct *data = &(mbviews[instance].data);

	for (int k = 0; k < n; k++) {
		const double xgrid = data->primary_xmin + i[k] * data->primary_dx;
		const double ygrid = data->primary_ymin + j[k] * data->primary_dy;
		mbview_projectgrid2ll(instance, xgrid, ygrid, &lonlat[2 * k], &lonlat[2 * k + 1]);
	}
}
/*------------------------------------------------------------------------------*/
void mbvq_c_display_points(size_t instance, int n, const double *lonlat, double value, double *xyz) {
	for (int k = 0; k < n; k++)
		mbview_projectll2display(instance, lonlat[2 * k], lonlat[2 * k + 1], value, &xyz[3 * k], &xyz[3 * k + 1],
		                         &xyz[3 * k + 2]);
}
/*------------------------------------------------------------------------------*/
bool mbvq_c_contours_made(size_t instance, int rez) {
	const struct mbview_world_struct *view = &(mbviews[instance]);
	if (rez == MBV_REZ_FULL)
		return (view->contourfullrez);
	if (rez == MBV_REZ_HIGH)
		return (view->contourhirez);
	if (rez == MBV_REZ_LOW)
		return (view->contourlorez);
	return (false);
}
/*------------------------------------------------------------------------------*/
