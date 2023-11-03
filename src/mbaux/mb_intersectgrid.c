/*--------------------------------------------------------------------
 *    The MB-system:	mb_intersectgrid.c	10/20/2012
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
 * Functions to handle reading a topographic grid and then calculate
 * the intersection location of arbitrary vectors with that grid.
 * Given a sonar location and a 3D vector "look" direction, this code calculates
 * the xyz location of the intersection of the vector with the topography.
 * This is used for laying out sidescan on the seafloor and for sidescan
 * mosaicing.
 *
 * Author:	D. W. Caress
 * Date:	October 20, 2012
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_status.h"

/*--------------------------------------------------------------------*/
int mb_topogrid_init(int verbose, mb_path topogridfile, int *lonflip, void **topogrid_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       topogridfile:              %s\n", topogridfile);
		fprintf(stderr, "dbg2       lonflip:                   %d\n", *lonflip);
		fprintf(stderr, "dbg2       topogrid:                  %p\n", *topogrid_ptr);
	}

	/* allocate memory for topogrid structure */
	int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mb_topogrid_struct), (void **)topogrid_ptr, error);

	/* get pointer to topogrid structure */
	struct mb_topogrid_struct *topogrid = (struct mb_topogrid_struct *)*topogrid_ptr;

	/* read in the data */
	strcpy(topogrid->file, topogridfile);
	topogrid->data = NULL;
	status = mb_read_gmt_grd(verbose, topogrid->file, &topogrid->projection_mode, topogrid->projection_id, &topogrid->nodatavalue,
	                         &topogrid->nxy, &topogrid->n_columns, &topogrid->n_rows, &topogrid->min, &topogrid->max, &topogrid->xmin,
	                         &topogrid->xmax, &topogrid->ymin, &topogrid->ymax, &topogrid->dx, &topogrid->dy, &topogrid->data,
	                         NULL, NULL, error);

	/* check for reasonable results */
	if (topogrid->nxy <= 0 || topogrid->data == NULL) {
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
	}

	/* rationalize topogrid bounds and lonflip */
	if (status == MB_SUCCESS) {
		if (*lonflip == -1) {
			if (topogrid->xmax > 180.0) {
				topogrid->xmin -= 360.0;
				topogrid->xmax -= 360.0;
			}
		}
		else if (*lonflip == 0) {
			if (topogrid->xmin > 180.0) {
				topogrid->xmin -= 360.0;
				topogrid->xmax -= 360.0;
			}
			else if (topogrid->xmax < -180.0) {
				topogrid->xmin += 360.0;
				topogrid->xmax += 360.0;
			}
		}
		else if (*lonflip == 1) {
			if (topogrid->xmin < -180.0) {
				topogrid->xmin += 360.0;
				topogrid->xmax += 360.0;
			}
		}
		if (topogrid->xmax > 180.0) {
			*lonflip = 1;
		}
		else if (topogrid->xmin < -180.0) {
			*lonflip = -1;
		}
		else {
			*lonflip = 0;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MB7K2SS function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       lonflip:                   %d\n", *lonflip);
		fprintf(stderr, "dbg2       topogrid:                  %p\n", topogrid);
		fprintf(stderr, "dbg2       topogrid->file:            %s\n", topogrid->file);
		fprintf(stderr, "dbg2       topogrid->projection_mode: %d\n", topogrid->projection_mode);
		fprintf(stderr, "dbg2       topogrid->projection_id:   %s\n", topogrid->projection_id);
		fprintf(stderr, "dbg2       topogrid->nodatavalue:     %f\n", topogrid->nodatavalue);
		fprintf(stderr, "dbg2       topogrid->nxy:             %d\n", topogrid->nxy);
		fprintf(stderr, "dbg2       topogrid->n_columns:       %d\n", topogrid->n_columns);
		fprintf(stderr, "dbg2       topogrid->n_rows:          %d\n", topogrid->n_rows);
		fprintf(stderr, "dbg2       topogrid->min:             %f\n", topogrid->min);
		fprintf(stderr, "dbg2       topogrid->max:             %f\n", topogrid->max);
		fprintf(stderr, "dbg2       topogrid->xmin:            %f\n", topogrid->xmin);
		fprintf(stderr, "dbg2       topogrid->xmax:            %f\n", topogrid->xmax);
		fprintf(stderr, "dbg2       topogrid->ymin:            %f\n", topogrid->ymin);
		fprintf(stderr, "dbg2       topogrid->ymax:            %f\n", topogrid->ymax);
		fprintf(stderr, "dbg2       topogrid->dx:              %f\n", topogrid->dx);
		fprintf(stderr, "dbg2       topogrid->dy               %f\n", topogrid->dy);
		fprintf(stderr, "dbg2       topogrid->data:            %p\n", topogrid->data);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_topogrid_deall(int verbose, void **topogrid_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       topogrid_ptr:              %p\n", topogrid_ptr);
		fprintf(stderr, "dbg2       topogrid:                  %p\n", *topogrid_ptr);
	}

	/* deallocate the topogrid structure */
	struct mb_topogrid_struct *topogrid = (struct mb_topogrid_struct *)*topogrid_ptr;
	int status = MB_SUCCESS;
	if (topogrid->data != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(topogrid->data), error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)topogrid_ptr, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MB7K2SS function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_topogrid_topo(int verbose, void *topogrid_ptr, double navlon, double navlat, double *topo, int *error) {
	struct mb_topogrid_struct *topogrid = (struct mb_topogrid_struct *)topogrid_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       navlon:                    %f\n", navlon);
		fprintf(stderr, "dbg2       navlat:                    %f\n", navlat);
		fprintf(stderr, "dbg2       topogrid:                  %p\n", topogrid);
		fprintf(stderr, "dbg2       topogrid->projection_mode: %d\n", topogrid->projection_mode);
		fprintf(stderr, "dbg2       topogrid->projection_id:   %s\n", topogrid->projection_id);
		fprintf(stderr, "dbg2       topogrid->nodatavalue:     %f\n", topogrid->nodatavalue);
		fprintf(stderr, "dbg2       topogrid->nxy:             %d\n", topogrid->nxy);
		fprintf(stderr, "dbg2       topogrid->n_columns:       %d\n", topogrid->n_columns);
		fprintf(stderr, "dbg2       topogrid->n_rows:          %d\n", topogrid->n_rows);
		fprintf(stderr, "dbg2       topogrid->min:             %f\n", topogrid->min);
		fprintf(stderr, "dbg2       topogrid->max:             %f\n", topogrid->max);
		fprintf(stderr, "dbg2       topogrid->xmin:            %f\n", topogrid->xmin);
		fprintf(stderr, "dbg2       topogrid->xmax:            %f\n", topogrid->xmax);
		fprintf(stderr, "dbg2       topogrid->ymin:            %f\n", topogrid->ymin);
		fprintf(stderr, "dbg2       topogrid->ymax:            %f\n", topogrid->ymax);
		fprintf(stderr, "dbg2       topogrid->dx:              %f\n", topogrid->dx);
		fprintf(stderr, "dbg2       topogrid->dy               %f\n", topogrid->dy);
		fprintf(stderr, "dbg2       topogrid->data:            %p\n", topogrid->data);
	}

	/* get topography at specified location */
	int nfound = 0;
	*topo = 0.0;
	const int i = (int)((navlon - topogrid->xmin) / topogrid->dx);
	const int j = (int)((navlat - topogrid->ymin) / topogrid->dy);
	if (i >= 0 && i < topogrid->n_columns - 1 && j >= 0 && j < topogrid->n_rows - 1) {
		for (int ii = i; ii <= i + 1; ii++)
			for (int jj = j; jj <= j + 1; jj++) {
				const int k = ii * topogrid->n_rows + jj;
				if (topogrid->data[k] != topogrid->nodatavalue) {
					nfound++;
					*topo += topogrid->data[k];
				}
			}
	}

	int status = MB_SUCCESS;
	if (nfound > 0) {
		*topo /= (double)nfound;
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MB7K2SS function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       topo:            %f\n", *topo);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_topogrid_bounds(int verbose, void *topogrid_ptr, double bounds[4], int *error) {
	struct mb_topogrid_struct *topogrid = (struct mb_topogrid_struct *)topogrid_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       topogrid:                  %p\n", topogrid);
		fprintf(stderr, "dbg2       topogrid->projection_mode: %d\n", topogrid->projection_mode);
		fprintf(stderr, "dbg2       topogrid->projection_id:   %s\n", topogrid->projection_id);
		fprintf(stderr, "dbg2       topogrid->nodatavalue:     %f\n", topogrid->nodatavalue);
		fprintf(stderr, "dbg2       topogrid->nxy:             %d\n", topogrid->nxy);
		fprintf(stderr, "dbg2       topogrid->n_columns:       %d\n", topogrid->n_columns);
		fprintf(stderr, "dbg2       topogrid->n_rows:          %d\n", topogrid->n_rows);
		fprintf(stderr, "dbg2       topogrid->min:             %f\n", topogrid->min);
		fprintf(stderr, "dbg2       topogrid->max:             %f\n", topogrid->max);
		fprintf(stderr, "dbg2       topogrid->xmin:            %f\n", topogrid->xmin);
		fprintf(stderr, "dbg2       topogrid->xmax:            %f\n", topogrid->xmax);
		fprintf(stderr, "dbg2       topogrid->ymin:            %f\n", topogrid->ymin);
		fprintf(stderr, "dbg2       topogrid->ymax:            %f\n", topogrid->ymax);
		fprintf(stderr, "dbg2       topogrid->dx:              %f\n", topogrid->dx);
		fprintf(stderr, "dbg2       topogrid->dy               %f\n", topogrid->dy);
		fprintf(stderr, "dbg2       topogrid->data:            %p\n", topogrid->data);
	}

	/* get topogrid bounds */
    if (topogrid != NULL) {
        bounds[0] = topogrid->xmin;
        bounds[1] = topogrid->xmax;
        bounds[2] = topogrid->ymin;
        bounds[3] = topogrid->ymax;
    }

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MB7K2SS function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       bounds[0]:       %f\n", topogrid->xmin);
		fprintf(stderr, "dbg2       bounds[1]:       %f\n", topogrid->xmax);
		fprintf(stderr, "dbg2       bounds[2]:       %f\n", topogrid->ymin);
		fprintf(stderr, "dbg2       bounds[3]:       %f\n", topogrid->ymax);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_topogrid_intersect(int verbose, void *topogrid_ptr, double navlon, double navlat, double altitude, double sensordepth,
                          double mtodeglon, double mtodeglat, double vx, double vy, double vz, double *lon, double *lat,
                          double *topo, double *range, int *error) {
	struct mb_topogrid_struct *topogrid = (struct mb_topogrid_struct *)topogrid_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       navlon:                    %f\n", navlon);
		fprintf(stderr, "dbg2       navlat:                    %f\n", navlat);
		fprintf(stderr, "dbg2       altitude:                  %f\n", altitude);
		fprintf(stderr, "dbg2       sensordepth:               %f\n", sensordepth);
		fprintf(stderr, "dbg2       mtodeglon:                 %f\n", mtodeglon);
		fprintf(stderr, "dbg2       mtodeglat:                 %f\n", mtodeglat);
		fprintf(stderr, "dbg2       vx:                        %f\n", vx);
		fprintf(stderr, "dbg2       vy:                        %f\n", vy);
		fprintf(stderr, "dbg2       vz:                        %f\n", vz);
		fprintf(stderr, "dbg2       topogrid:                  %p\n", topogrid);
		fprintf(stderr, "dbg2       topogrid->projection_mode: %d\n", topogrid->projection_mode);
		fprintf(stderr, "dbg2       topogrid->projection_id:   %s\n", topogrid->projection_id);
		fprintf(stderr, "dbg2       topogrid->nodatavalue:     %f\n", topogrid->nodatavalue);
		fprintf(stderr, "dbg2       topogrid->nxy:             %d\n", topogrid->nxy);
		fprintf(stderr, "dbg2       topogrid->n_columns:       %d\n", topogrid->n_columns);
		fprintf(stderr, "dbg2       topogrid->n_rows:          %d\n", topogrid->n_rows);
		fprintf(stderr, "dbg2       topogrid->min:             %f\n", topogrid->min);
		fprintf(stderr, "dbg2       topogrid->max:             %f\n", topogrid->max);
		fprintf(stderr, "dbg2       topogrid->xmin:            %f\n", topogrid->xmin);
		fprintf(stderr, "dbg2       topogrid->xmax:            %f\n", topogrid->xmax);
		fprintf(stderr, "dbg2       topogrid->ymin:            %f\n", topogrid->ymin);
		fprintf(stderr, "dbg2       topogrid->ymax:            %f\n", topogrid->ymax);
		fprintf(stderr, "dbg2       topogrid->dx:              %f\n", topogrid->dx);
		fprintf(stderr, "dbg2       topogrid->dy               %f\n", topogrid->dy);
		fprintf(stderr, "dbg2       topogrid->data:            %p\n", topogrid->data);
	}

	int status = MB_SUCCESS;

	bool done = false;

	/* if altitude specified use it for initial guess */
	double dr = 0.0;
	double r = 0.0;
	double rmax = 0.0;
	if (altitude > 0.0) {
		dr = altitude / 20;
		r = altitude / vz - dr;
		rmax = 4 * altitude / vz;
	} else {
		// if altitude not specified use altitude at location
		int nfound = 0;
		double topog = 0.0;
		const int i = (int)((navlon - topogrid->xmin) / topogrid->dx);
		const int j = (int)((navlat - topogrid->ymin) / topogrid->dy);
		if (i >= 0 && i < topogrid->n_columns - 1 && j >= 0 && j < topogrid->n_rows - 1) {
			for (int ii = i; ii <= i + 1; ii++)
				for (int jj = j; jj <= j + 1; jj++) {
					const int k = ii * topogrid->n_rows + jj;
					if (topogrid->data[k] != topogrid->nodatavalue) {
						nfound++;
						topog += topogrid->data[k];
					}
				}
		}
		if (nfound > 0) {
			topog /= (double)nfound;
			altitude = -sensordepth - topog;
			dr = altitude / 20;
			r = altitude / vz - dr;
			rmax = 4 * altitude / vz;
		} else {
			done = true;
			status = MB_FAILURE;
			*error = MB_ERROR_NOT_ENOUGH_DATA;
		}
	}

	double rmin = 0.0;
	double dtopo = 0.0;
	int iteration = 0;
	const int iteration_max = 50;
	double topotolerance = 0.05 * (topogrid->dx / mtodeglon + topogrid->dy / mtodeglat);

	/* test different ranges along the vector until the grid is intersected */
	while (!done && iteration < iteration_max) {
		/* update the range to be tested */
		r += dr;

		/* get position of range estimate projected along the vector */
		const double lontest = navlon + mtodeglon * vx * r;
		const double lattest = navlat + mtodeglat * vy * r;
		const double topotest = -sensordepth - vz * r;

		/* get topography value at that point */
		int nfound = 0;
		double topog = 0.0;
		const int i = (int)((lontest - topogrid->xmin) / topogrid->dx);
		const int j = (int)((lattest - topogrid->ymin) / topogrid->dy);
		if (i >= 0 && i < topogrid->n_columns - 1 && j >= 0 && j < topogrid->n_rows - 1) {
			for (int ii = i; ii <= i + 1; ii++)
				for (int jj = j; jj <= j + 1; jj++) {
					const int k = ii * topogrid->n_rows + jj;
					if (topogrid->data[k] != topogrid->nodatavalue) {
						nfound++;
						topog += topogrid->data[k];
					}
				}
		} else {
			done = true;
			status = MB_FAILURE;
			*error = MB_ERROR_NOT_ENOUGH_DATA;
		}
		if (nfound > 0) {
			topog /= (double)nfound;
		}

		/* compare topographies at projected position */
		if (nfound > 0) {
			dtopo = topotest - topog;
			if (fabs(dtopo) < topotolerance) {
				done = true;
			}
			else {
				/* get bounds on where vector crosses the grid */
				if (dtopo < 0.0)
					rmax = MIN(rmax, r);
				else if (dtopo > 0.0)
					rmin = MIN(rmin, r);

				/* estimate distance to crossing point */
				dr = dtopo / vz;

				/* make sure we don't overshoot the bounds */
				if (r + dr >= rmax)
					dr = 0.5 * (rmax - r);
				if (r + dr <= rmin)
					dr = 0.5 * (rmin - r);
			}
		}
		/* keep trying */
		else {
		}

		/* keep count of iterations */
		iteration++;
	}

	/* if success return the result */
	*lon = navlon + mtodeglon * vx * r;
	*lat = navlat + mtodeglat * vy * r;
	*topo = -sensordepth - vz * r;
	*range = r;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MB7K2SS function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       lon:             %f\n", *lon);
		fprintf(stderr, "dbg2       lat:             %f\n", *lat);
		fprintf(stderr, "dbg2       topo:            %f\n", *topo);
		fprintf(stderr, "dbg2       range:           %f\n", *range);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_topogrid_getangletable(int verbose, void *topogrid_ptr, int nangle, double angle_min, double angle_max, double navlon,
                              double navlat, double heading, double altitude, double sensordepth, double pitch,
                              double *table_angle, double *table_xtrack, double *table_ltrack, double *table_altitude,
                              double *table_range, int *error) {
	struct mb_topogrid_struct *topogrid = (struct mb_topogrid_struct *)topogrid_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       nangle:                    %d\n", nangle);
		fprintf(stderr, "dbg2       angle_min:                 %f\n", angle_min);
		fprintf(stderr, "dbg2       angle_max:                 %f\n", angle_max);
		fprintf(stderr, "dbg2       navlon:                    %f\n", navlon);
		fprintf(stderr, "dbg2       navlat:                    %f\n", navlat);
		fprintf(stderr, "dbg2       heading:                   %f\n", heading);
		fprintf(stderr, "dbg2       altitude:                  %f\n", altitude);
		fprintf(stderr, "dbg2       sensordepth:               %f\n", sensordepth);
		fprintf(stderr, "dbg2       pitch:                     %f\n", pitch);
		fprintf(stderr, "dbg2       topogrid:                  %p\n", topogrid);
		fprintf(stderr, "dbg2       topogrid->projection_mode: %d\n", topogrid->projection_mode);
		fprintf(stderr, "dbg2       topogrid->projection_id:   %s\n", topogrid->projection_id);
		fprintf(stderr, "dbg2       topogrid->nodatavalue:     %f\n", topogrid->nodatavalue);
		fprintf(stderr, "dbg2       topogrid->nxy:             %d\n", topogrid->nxy);
		fprintf(stderr, "dbg2       topogrid->n_columns:       %d\n", topogrid->n_columns);
		fprintf(stderr, "dbg2       topogrid->n_rows:          %d\n", topogrid->n_rows);
		fprintf(stderr, "dbg2       topogrid->min:             %f\n", topogrid->min);
		fprintf(stderr, "dbg2       topogrid->max:             %f\n", topogrid->max);
		fprintf(stderr, "dbg2       topogrid->xmin:            %f\n", topogrid->xmin);
		fprintf(stderr, "dbg2       topogrid->xmax:            %f\n", topogrid->xmax);
		fprintf(stderr, "dbg2       topogrid->ymin:            %f\n", topogrid->ymin);
		fprintf(stderr, "dbg2       topogrid->ymax:            %f\n", topogrid->ymax);
		fprintf(stderr, "dbg2       topogrid->dx:              %f\n", topogrid->dx);
		fprintf(stderr, "dbg2       topogrid->dy               %f\n", topogrid->dy);
		fprintf(stderr, "dbg2       topogrid->data:            %p\n", topogrid->data);
	}

	int status = MB_SUCCESS;

	/* loop over all of the angles */
	double mtodeglon;
	double mtodeglat;
	mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
	double dangle = (angle_max - angle_min) / (nangle - 1);
	double alpha = pitch;
	int nset = 0;
	for (int i = 0; i < nangle; i++) {
		/* get angles in takeoff coordinates */
		table_angle[i] = angle_min + dangle * i;
		const double beta = 90.0 - table_angle[i];
		double theta;
		double phi;
		mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);

		/* calculate unit vector relative to the vehicle */
		const double vz = cos(DTR * theta);
		double vx = sin(DTR * theta) * cos(DTR * phi);
		double vy = sin(DTR * theta) * sin(DTR * phi);

		/* rotate unit vector by vehicle heading */
		vx = vx * cos(DTR * heading) + vy * sin(DTR * heading);
		vy = -vx * sin(DTR * heading) + vy * cos(DTR * heading);

		double lon;
		double lat;
		double topo;
		double rr;
		/* find the range where this vector intersects the grid */
		status = mb_topogrid_intersect(verbose, topogrid_ptr, navlon, navlat, altitude, sensordepth, mtodeglon, mtodeglat, vx, vy,
		                               vz, &lon, &lat, &topo, &rr, error);

		/* get the position from successful intersection with the grid */
		if (status == MB_SUCCESS) {
			const double zz = rr * cos(DTR * theta);
			const double xx = rr * sin(DTR * theta);
			table_xtrack[i] = xx * cos(DTR * phi);
			table_ltrack[i] = xx * sin(DTR * phi);
			table_altitude[i] = zz;
			table_range[i] = rr;
			nset++;
		}

		/* zero table values for the moment */
		else {
			table_range[i] = 0.0;
		}
	}

	/* now deal with any unset table entries */
	if (nset < nangle) {
		/* find first and last table entries set if possible */
		if (nset > 0) {
			int first = nangle;
			int last = -1;
			for (int i = 0; i < nangle; i++) {
				if (table_range[i] > 0.0) {
					first = MIN(i, first);
					last = MAX(i, last);
				}
			}

			/* apply flat bottom calculation to unset entries */
			for (int i = 0; i < nangle; i++) {
				if (table_range[i] <= 0.0) {
					/* get angles in takeoff coordinates */
					table_angle[i] = angle_min + dangle * i;
					const double beta = 90.0 - table_angle[i];
					double theta;
					double phi;
					mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);

					if (nset == 0) {
						table_altitude[i] = altitude;
					}
					else if (i < first) {
						table_altitude[i] = table_altitude[first];
					}
					else if (i > last) {
						table_altitude[i] = table_altitude[last];
					}
					else {
						table_altitude[i] = 0.5 * (table_altitude[first] + table_altitude[last]);
					}

					table_range[i] = table_altitude[first] / cos(DTR * theta);
					const double xx = table_range[i] * sin(DTR * theta);
					table_xtrack[i] = xx * cos(DTR * phi);
					table_ltrack[i] = xx * sin(DTR * phi);
					nset++;
				}
			}
		}
	}

	/* reset error condition */
	if (nset >= nangle) {
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MB7K2SS function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       Lookup tables:\n");
		for (int i = 0; i < nangle; i++)
			fprintf(stderr, "dbg2         %d %f %f %f %f %f\n", i, table_angle[i], table_xtrack[i], table_ltrack[i],
			        table_altitude[i], table_range[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
