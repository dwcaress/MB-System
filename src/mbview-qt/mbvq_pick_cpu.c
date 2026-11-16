/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_pick_cpu.c  11/13/2026
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
 * Terrain pick and 3D view bounds of the map view on the CPU, for renderers
 * that do not draw with OpenGL (the pick_terrain and view_bounds entries of
 * struct mbv_render_ops). They find what the colour-coded drawing of
 * mbview_findpointrez() and mbview_viewbounds() finds. A ray through the
 * centre of the pixel, from the camera of mbview_camera(), is cut with the
 * triangles that drawing shows: two per grid cell, A = (i,j)(i+s,j)(i,j+s)
 * and B = (i+s,j)(i+s,j+s)(i,j+s), each where its three nodes hold data.
 * In 3D the nearest triangle wins, as with the depth test; in 2D, which is
 * drawn without one, the last triangle drawn. Blocks of 16 x 16 cells with
 * their bounds in display coordinates keep the search short. The bounds are
 * found again for every call, because the core does not yet mark what has
 * changed.
 *
 * The pick works at full resolution and returns the centroid of the triangle
 * seen, with the formula of mbview_findpointrez(). The drawing pick narrows a
 * search window from low resolution and keeps a coarse result when the full
 * resolution shows nothing in that window; this pick has no coarse stage.
 */

#include <float.h>
#include <math.h>
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

/* cells per side of a search block */
#define MBVQ_PICK_BLOCK 16

/* a block the ray passes, with the ray parameter where it enters */
struct mbvq_pick_entry {
	double t;
	int block;
};

/* the triangles of one resolution: cells starting at i = 0, s, 2s, ... and
   j = 0, s, 2s, ..., in blocks of MBVQ_PICK_BLOCK x MBVQ_PICK_BLOCK cells */
struct mbvq_pick_mesh {
	size_t instance;
	int stride;
	int ncell_i;
	int ncell_j;
	int nblock_i;
	int nblock_j;
	float *bounds;                  /* per block: minimum x, y, z, then maximum x, y, z */
	struct mbvq_pick_entry *entries; /* blocks passed by the current ray */
};

/* the triangle seen at a pixel */
struct mbvq_pick_hit {
	bool found;
	double t;   /* on the ray: 0 at the near plane, 1 at the far plane */
	long order; /* in the order the triangles are drawn */
	int i;      /* first node of the cell */
	int j;
	bool b;     /* triangle B, else A */
};

/*------------------------------------------------------------------------------*/
/* r = a * b for column-major 4 x 4 matrices */
static void mbvq_pick_multiply(const double a[16], const double b[16], double r[16]) {
	for (int column = 0; column < 4; column++)
		for (int row = 0; row < 4; row++)
			r[4 * column + row] = a[row] * b[4 * column] + a[4 + row] * b[4 * column + 1] + a[8 + row] * b[4 * column + 2] +
			                      a[12 + row] * b[4 * column + 3];
}
/*------------------------------------------------------------------------------*/
/* inverse of a column-major 4 x 4 matrix (Gauss-Jordan with partial
   pivoting); false if the matrix is singular */
static bool mbvq_pick_invert(const double m[16], double inv[16]) {
	double a[4][8];
	for (int row = 0; row < 4; row++)
		for (int column = 0; column < 4; column++) {
			a[row][column] = m[4 * column + row];
			a[row][4 + column] = (row == column) ? 1.0 : 0.0;
		}
	for (int column = 0; column < 4; column++) {
		int pivot = column;
		for (int row = column + 1; row < 4; row++)
			if (fabs(a[row][column]) > fabs(a[pivot][column]))
				pivot = row;
		if (a[pivot][column] == 0.0)
			return false;
		if (pivot != column)
			for (int k = 0; k < 8; k++) {
				const double swap = a[pivot][k];
				a[pivot][k] = a[column][k];
				a[column][k] = swap;
			}
		const double scale = 1.0 / a[column][column];
		for (int k = 0; k < 8; k++)
			a[column][k] *= scale;
		for (int row = 0; row < 4; row++)
			if (row != column && a[row][column] != 0.0) {
				const double factor = a[row][column];
				for (int k = 0; k < 8; k++)
					a[row][k] -= factor * a[column][k];
			}
	}
	for (int row = 0; row < 4; row++)
		for (int column = 0; column < 4; column++)
			inv[4 * column + row] = a[row][4 + column];
	return true;
}
/*------------------------------------------------------------------------------*/
/* the inverse of projection x model-view of the map view; mbview_camera()
   also sets the view's left, right, bottom and top as the drawing does */
static bool mbvq_pick_camera(size_t instance, double inverse[16]) {
	double projection[16];
	double modelview[16];
	double product[16];
	mbview_camera(instance, projection, modelview);
	mbvq_pick_multiply(projection, modelview, product);
	return mbvq_pick_invert(product, inverse);
}
/*------------------------------------------------------------------------------*/
/* the ray through the centre of a pixel (counted as glReadPixels counts it),
   from the near plane (t = 0) to the far plane (t = 1), in display
   coordinates */
static bool mbvq_pick_ray(const double inverse[16], int width, int height, int xpixel, int ypixel, double origin[3],
                          double direction[3]) {
	if (width <= 0 || height <= 0)
		return false;
	const double ndc[2] = {2.0 * (xpixel + 0.5) / width - 1.0, 2.0 * (ypixel + 0.5) / height - 1.0};
	double ends[2][3];
	for (int e = 0; e < 2; e++) {
		const double in[4] = {ndc[0], ndc[1], e == 0 ? -1.0 : 1.0, 1.0};
		double out[4];
		for (int row = 0; row < 4; row++)
			out[row] = inverse[row] * in[0] + inverse[4 + row] * in[1] + inverse[8 + row] * in[2] + inverse[12 + row] * in[3];
		if (out[3] == 0.0)
			return false;
		for (int a = 0; a < 3; a++)
			ends[e][a] = out[a] / out[3];
	}
	for (int a = 0; a < 3; a++) {
		origin[a] = ends[0][a];
		direction[a] = ends[1][a] - ends[0][a];
	}
	return true;
}
/*------------------------------------------------------------------------------*/
/* where the ray enters a box, if it meets the box between t = 0 and t = 1 */
static bool mbvq_pick_box(const double origin[3], const double direction[3], const float *bounds, double *tenter) {
	double t0 = 0.0;
	double t1 = 1.0;
	for (int a = 0; a < 3; a++) {
		if (direction[a] == 0.0) {
			if (origin[a] < bounds[a] || origin[a] > bounds[3 + a])
				return false;
		}
		else {
			double ta = (bounds[a] - origin[a]) / direction[a];
			double tb = (bounds[3 + a] - origin[a]) / direction[a];
			if (ta > tb) {
				const double swap = ta;
				ta = tb;
				tb = swap;
			}
			if (ta > t0)
				t0 = ta;
			if (tb < t1)
				t1 = tb;
			if (t0 > t1)
				return false;
		}
	}
	*tenter = t0;
	return true;
}
/*------------------------------------------------------------------------------*/
/* where the ray meets the triangle of nodes k1, k2, k3, if it does between
   t = 0 and t = 1 (Moeller-Trumbore, edges included) */
static bool mbvq_pick_triangle(const struct mbview_struct *data, const double origin[3], const double direction[3], int k1,
                               int k2, int k3, double *t) {
	const double p1[3] = {data->primary_x[k1], data->primary_y[k1], data->primary_z[k1]};
	const double e1[3] = {data->primary_x[k2] - p1[0], data->primary_y[k2] - p1[1], data->primary_z[k2] - p1[2]};
	const double e2[3] = {data->primary_x[k3] - p1[0], data->primary_y[k3] - p1[1], data->primary_z[k3] - p1[2]};
	const double p[3] = {direction[1] * e2[2] - direction[2] * e2[1], direction[2] * e2[0] - direction[0] * e2[2],
	                     direction[0] * e2[1] - direction[1] * e2[0]};
	const double det = e1[0] * p[0] + e1[1] * p[1] + e1[2] * p[2];
	if (det == 0.0)
		return false;
	const double s[3] = {origin[0] - p1[0], origin[1] - p1[1], origin[2] - p1[2]};
	const double u = (s[0] * p[0] + s[1] * p[1] + s[2] * p[2]) / det;
	if (u < 0.0 || u > 1.0)
		return false;
	const double q[3] = {s[1] * e1[2] - s[2] * e1[1], s[2] * e1[0] - s[0] * e1[2], s[0] * e1[1] - s[1] * e1[0]};
	const double v = (direction[0] * q[0] + direction[1] * q[1] + direction[2] * q[2]) / det;
	if (v < 0.0 || u + v > 1.0)
		return false;
	*t = (e2[0] * q[0] + e2[1] * q[1] + e2[2] * q[2]) / det;
	return *t >= 0.0 && *t <= 1.0;
}
/*------------------------------------------------------------------------------*/
static void mbvq_pick_mesh_free(struct mbvq_pick_mesh *mesh) {
	int error = MB_ERROR_NO_ERROR;
	if (mesh->bounds != NULL)
		mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&mesh->bounds, &error);
	if (mesh->entries != NULL)
		mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&mesh->entries, &error);
}
/*------------------------------------------------------------------------------*/
/* the mesh of cells starting below ilimit and jlimit with the given stride,
   and the bounds of its blocks; zscale: first scale the z of every node with
   data that is not scaled yet, as the drawing pick does for its triangles */
static bool mbvq_pick_mesh_init(size_t instance, int stride, int ilimit, int jlimit, bool zscale,
                                struct mbvq_pick_mesh *mesh) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	memset(mesh, 0, sizeof(*mesh));
	mesh->instance = instance;
	mesh->stride = stride;
	if (stride <= 0 || ilimit <= 0 || jlimit <= 0 || data->primary_data == NULL)
		return false;
	mesh->ncell_i = (ilimit + stride - 1) / stride;
	mesh->ncell_j = (jlimit + stride - 1) / stride;
	mesh->nblock_i = (mesh->ncell_i + MBVQ_PICK_BLOCK - 1) / MBVQ_PICK_BLOCK;
	mesh->nblock_j = (mesh->ncell_j + MBVQ_PICK_BLOCK - 1) / MBVQ_PICK_BLOCK;
	const size_t nblock = (size_t)mesh->nblock_i * (size_t)mesh->nblock_j;
	int error = MB_ERROR_NO_ERROR;
	if (mb_mallocd(mbv_verbose, __FILE__, __LINE__, 6 * nblock * sizeof(float), (void **)&mesh->bounds, &error) !=
	        MB_SUCCESS ||
	    mesh->bounds == NULL ||
	    mb_mallocd(mbv_verbose, __FILE__, __LINE__, nblock * sizeof(struct mbvq_pick_entry), (void **)&mesh->entries,
	               &error) != MB_SUCCESS ||
	    mesh->entries == NULL) {
		mbvq_pick_mesh_free(mesh);
		return false;
	}

	/* the bounds of the nodes of each block, from its first cell's first node
	   to its last cell's last node; widened a little so that rounding in the
	   ray test cannot drop a triangle that touches a face of the box */
	for (int bi = 0; bi < mesh->nblock_i; bi++) {
		for (int bj = 0; bj < mesh->nblock_j; bj++) {
			float *bounds = &mesh->bounds[6 * ((size_t)bi * mesh->nblock_j + bj)];
			bounds[0] = bounds[1] = bounds[2] = FLT_MAX;
			bounds[3] = bounds[4] = bounds[5] = -FLT_MAX;
			const int ci1 = MIN((bi + 1) * MBVQ_PICK_BLOCK, mesh->ncell_i);
			const int cj1 = MIN((bj + 1) * MBVQ_PICK_BLOCK, mesh->ncell_j);
			for (int ci = bi * MBVQ_PICK_BLOCK; ci <= ci1; ci++) {
				for (int cj = bj * MBVQ_PICK_BLOCK; cj <= cj1; cj++) {
					const int k = ci * stride * data->primary_n_rows + cj * stride;
					if (data->primary_data[k] == data->primary_nodatavalue)
						continue;
					if (zscale && !(data->primary_stat_z[k / 8] & statmask[k % 8]))
						mbview_zscalegridpoint(instance, k);
					const float xyz[3] = {data->primary_x[k], data->primary_y[k], data->primary_z[k]};
					for (int a = 0; a < 3; a++) {
						bounds[a] = MIN(bounds[a], xyz[a]);
						bounds[3 + a] = MAX(bounds[3 + a], xyz[a]);
					}
				}
			}
			if (bounds[0] <= bounds[3]) {
				for (int a = 0; a < 3; a++) {
					const float margin = 1.0e-6f * (1.0f + MAX(fabsf(bounds[a]), fabsf(bounds[3 + a])));
					bounds[a] -= margin;
					bounds[3 + a] += margin;
				}
			}
		}
	}
	return true;
}
/*------------------------------------------------------------------------------*/
/* keep a triangle met by the ray if it is the one the drawing shows: the
   nearest in 3D (depth test GL_LESS, so the first drawn of equal depths),
   the last drawn in 2D (no depth test) */
static void mbvq_pick_keep(struct mbvq_pick_hit *hit, bool nearest, double t, long order, int i, int j, bool b) {
	bool keep;
	if (!hit->found)
		keep = true;
	else if (nearest)
		keep = t < hit->t || (t == hit->t && order < hit->order);
	else
		keep = order > hit->order;
	if (keep) {
		hit->found = true;
		hit->t = t;
		hit->order = order;
		hit->i = i;
		hit->j = j;
		hit->b = b;
	}
}
/*------------------------------------------------------------------------------*/
static int mbvq_pick_entry_compare(const void *a, const void *b) {
	const double ta = ((const struct mbvq_pick_entry *)a)->t;
	const double tb = ((const struct mbvq_pick_entry *)b)->t;
	return (ta > tb) - (ta < tb);
}
/*------------------------------------------------------------------------------*/
/* the triangle of the mesh seen along the ray */
static void mbvq_pick_search(struct mbvq_pick_mesh *mesh, const double origin[3], const double direction[3], bool nearest,
                             struct mbvq_pick_hit *hit) {
	struct mbview_world_struct *view = &(mbviews[mesh->instance]);
	const struct mbview_struct *data = &(view->data);
	const int s = mesh->stride;

	memset(hit, 0, sizeof(*hit));

	/* the blocks the ray passes, nearest first */
	int nentry = 0;
	for (int block = 0; block < mesh->nblock_i * mesh->nblock_j; block++) {
		double tenter;
		const float *bounds = &mesh->bounds[6 * (size_t)block];
		if (bounds[0] <= bounds[3] && mbvq_pick_box(origin, direction, bounds, &tenter)) {
			mesh->entries[nentry].t = tenter;
			mesh->entries[nentry].block = block;
			nentry++;
		}
	}
	if (nearest)
		qsort(mesh->entries, nentry, sizeof(struct mbvq_pick_entry), mbvq_pick_entry_compare);

	for (int e = 0; e < nentry; e++) {
		/* no block entered beyond the nearest triangle found can hold a nearer one */
		if (nearest && hit->found && mesh->entries[e].t > hit->t)
			break;
		const int bi = mesh->entries[e].block / mesh->nblock_j;
		const int bj = mesh->entries[e].block % mesh->nblock_j;
		const int ci1 = MIN((bi + 1) * MBVQ_PICK_BLOCK, mesh->ncell_i);
		const int cj1 = MIN((bj + 1) * MBVQ_PICK_BLOCK, mesh->ncell_j);
		for (int ci = bi * MBVQ_PICK_BLOCK; ci < ci1; ci++) {
			for (int cj = bj * MBVQ_PICK_BLOCK; cj < cj1; cj++) {
				const int i = ci * s;
				const int j = cj * s;
				const int k = i * data->primary_n_rows + j;
				const int l = (i + s) * data->primary_n_rows + j;
				const int m = i * data->primary_n_rows + j + s;
				const int n = (i + s) * data->primary_n_rows + j + s;
				const long order = 2 * ((long)ci * mesh->ncell_j + cj);
				double t;
				if (data->primary_data[k] != data->primary_nodatavalue && data->primary_data[l] != data->primary_nodatavalue &&
				    data->primary_data[m] != data->primary_nodatavalue &&
				    mbvq_pick_triangle(data, origin, direction, k, l, m, &t))
					mbvq_pick_keep(hit, nearest, t, order, i, j, false);
				if (data->primary_data[l] != data->primary_nodatavalue && data->primary_data[m] != data->primary_nodatavalue &&
				    data->primary_data[n] != data->primary_nodatavalue &&
				    mbvq_pick_triangle(data, origin, direction, l, n, m, &t))
					mbvq_pick_keep(hit, nearest, t, order + 1, i, j, true);
			}
		}
	}
}
/*------------------------------------------------------------------------------*/
bool mbvq_pick_cpu_terrain(size_t instance, int xpixel, int ypixel, double *xgrid, double *ygrid, double *zdata) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       xpixel:           %d\n", xpixel);
		fprintf(stderr, "dbg2       ypixel:           %d\n", ypixel);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	*xgrid = 0.0;
	*ygrid = 0.0;
	*zdata = 0.0;
	bool found = false;

	/* apply projection if needed */
	if (!view->projected) {
		do_mbview_status("Projecting data...", instance);
		mbview_projectdata(instance);
	}

	/* the triangle seen at the pixel, at full resolution */
	double inverse[16];
	double origin[3];
	double direction[3];
	struct mbvq_pick_mesh mesh;
	struct mbvq_pick_hit hit;
	if (mbvq_pick_camera(instance, inverse) &&
	    mbvq_pick_ray(inverse, data->width, data->height, xpixel, ypixel, origin, direction) &&
	    mbvq_pick_mesh_init(instance, 1, data->primary_n_columns - 1, data->primary_n_rows - 1, true, &mesh)) {
		mbvq_pick_search(&mesh, origin, direction, data->display_mode != MBV_DISPLAY_2D, &hit);
		mbvq_pick_mesh_free(&mesh);

		/* its centroid, as mbview_findpointrez() computes it at full resolution */
		if (hit.found) {
			found = true;
			const int stride = 1;
			const int i = hit.i;
			const int j = hit.j;
			const int k = i * data->primary_n_rows + j;
			const int l = (i + stride) * data->primary_n_rows + j;
			const int m = i * data->primary_n_rows + j + stride;
			const int n = (i + stride) * data->primary_n_rows + j + stride;
			if (!hit.b) {
				*xgrid = data->primary_xmin + (3 * i + stride) * data->primary_dx / 3.0;
				*ygrid = data->primary_ymin + (3 * j + stride) * data->primary_dy / 3.0;
				*zdata = (data->primary_data[k] + data->primary_data[l] + data->primary_data[m]) / 3.0;
			}
			else {
				*xgrid = data->primary_xmin + (3 * i + 2 * stride) * data->primary_dx / 3.0;
				*ygrid = data->primary_ymin + (3 * j + 2 * stride) * data->primary_dy / 3.0;
				*zdata = (data->primary_data[l] + data->primary_data[n] + data->primary_data[m]) / 3.0;
			}
		}
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       xgrid:           %f\n", *xgrid);
		fprintf(stderr, "dbg2       ygrid:           %f\n", *ygrid);
		fprintf(stderr, "dbg2       zdata:           %f\n", *zdata);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       found:           %d\n", found);
	}

	return found;
}
/*------------------------------------------------------------------------------*/
void mbvq_pick_cpu_viewbounds(size_t instance, int viewbounds[4]) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* the low resolution and the pick blocks of mbview_viewbounds() */
	const int stride = MAX((int)ceil(((double)data->primary_n_columns) / ((double)data->lorez_dimension)),
	                       (int)ceil(((double)data->primary_n_rows) / ((double)data->lorez_dimension)));
	const int npickx = (data->primary_n_columns / stride);
	const int ipickstride = stride * (int)floor((npickx / MBV_PICK_DIVISION) + 1);
	const int npicky = (data->primary_n_rows / stride);
	const int jpickstride = stride * (int)floor((npicky / MBV_PICK_DIVISION) + 1);

	bool found = false;
	viewbounds[0] = 0;
	viewbounds[1] = data->primary_n_columns - 1;
	viewbounds[2] = 0;
	viewbounds[3] = data->primary_n_rows - 1;

	/* the block of the triangle seen at the pixels that mbview_viewbounds()
	   reads: every 20th of the width and height, then the four corners */
	double inverse[16];
	struct mbvq_pick_mesh mesh;
	if (mbvq_pick_camera(instance, inverse) &&
	    mbvq_pick_mesh_init(instance, stride, data->primary_n_columns - stride, data->primary_n_rows - stride, false,
	                        &mesh)) {
		const int iscreenstride = data->width / 20;
		const int jscreenstride = data->height / 20;
		for (int xpixel = 0; iscreenstride > 0 && xpixel < data->width; xpixel += iscreenstride) {
			for (int ypixel = 0; jscreenstride > 0 && ypixel < data->height; ypixel += jscreenstride) {
				double origin[3];
				double direction[3];
				struct mbvq_pick_hit hit;
				if (!mbvq_pick_ray(inverse, data->width, data->height, xpixel, ypixel, origin, direction))
					continue;
				mbvq_pick_search(&mesh, origin, direction, data->display_mode != MBV_DISPLAY_2D, &hit);
				if (hit.found) {
					const int i = ipickstride * (hit.i / ipickstride);
					const int j = jpickstride * (hit.j / jpickstride);
					if (found) {
						viewbounds[0] = MIN(i, viewbounds[0]);
						viewbounds[1] = MAX(i + stride, viewbounds[1]);
						viewbounds[2] = MIN(j, viewbounds[2]);
						viewbounds[3] = MAX(j + stride, viewbounds[3]);
					}
					else {
						viewbounds[0] = i;
						viewbounds[1] = i + stride;
						viewbounds[2] = j;
						viewbounds[3] = j + stride;
						found = true;
					}
				}
			}
		}
		for (int xpixel = 0; data->width > 1 && xpixel < data->width; xpixel += data->width - 1) {
			for (int ypixel = 0; data->height > 1 && ypixel < data->height; ypixel += data->height - 1) {
				double origin[3];
				double direction[3];
				struct mbvq_pick_hit hit;
				if (!mbvq_pick_ray(inverse, data->width, data->height, xpixel, ypixel, origin, direction))
					continue;
				mbvq_pick_search(&mesh, origin, direction, data->display_mode != MBV_DISPLAY_2D, &hit);
				if (hit.found) {
					int ijbounds[4];
					const int i = ipickstride * (hit.i / ipickstride);
					const int j = jpickstride * (hit.j / jpickstride);
					ijbounds[0] = i;
					ijbounds[2] = j;
					if (ipickstride == 1) {
						ijbounds[1] = i;
						ijbounds[3] = j;
					}
					else {
						ijbounds[1] = MIN(i + 2 * ipickstride - 1, data->primary_n_columns - 1);
						ijbounds[3] = MIN(j + 2 * jpickstride - 1, data->primary_n_rows - 1);
					}
					if (found) {
						viewbounds[0] = MIN(ijbounds[0], viewbounds[0]);
						viewbounds[1] = MAX(ijbounds[1], viewbounds[1]);
						viewbounds[2] = MIN(ijbounds[2], viewbounds[2]);
						viewbounds[3] = MAX(ijbounds[3], viewbounds[3]);
					}
					else {
						viewbounds[0] = ijbounds[0];
						viewbounds[1] = ijbounds[1];
						viewbounds[2] = ijbounds[2];
						viewbounds[3] = ijbounds[3];
						found = true;
					}
				}
			}
		}
		mbvq_pick_mesh_free(&mesh);
	}
	viewbounds[0] = MAX(viewbounds[0] - stride, 0);
	viewbounds[1] = MIN(viewbounds[1] + stride, data->primary_n_columns - 1);
	viewbounds[2] = MAX(viewbounds[2] - stride, 0);
	viewbounds[3] = MIN(viewbounds[3] + stride, data->primary_n_rows - 1);

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       viewbounds[0]:   %d\n", viewbounds[0]);
		fprintf(stderr, "dbg2       viewbounds[1]:   %d\n", viewbounds[1]);
		fprintf(stderr, "dbg2       viewbounds[2]:   %d\n", viewbounds[2]);
		fprintf(stderr, "dbg2       viewbounds[3]:   %d\n", viewbounds[3]);
	}
}
/*------------------------------------------------------------------------------*/
