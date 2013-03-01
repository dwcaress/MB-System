/*--------------------------------------------------------------------
 *    The MB-system:	mb_intersectgrid.c	10/20/2012
 *    $Id:  $
 *
 *    Copyright (c) 2012-2012 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
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
 *
 * $Log: mb_intersectgrid.c,v $
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_aux.h"

static char rcs_id[] = "$Id: mb_intersectgrid.c 1917 2012-01-10 19:25:33Z caress $";

/*--------------------------------------------------------------------*/
int mb_topogrid_init(int verbose, mb_path topogridfile, int *lonflip,
			void **topogrid_ptr, int *error)
{
	char	*function_name = "mb_topogrid_init";
	int	status = MB_SUCCESS;
	struct mb_topogrid_struct *topogrid;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       topogridfile:              %s\n", topogridfile);
		fprintf(stderr,"dbg2       lonflip:                   %d\n", *lonflip);
		fprintf(stderr,"dbg2       topogrid:                  %lu\n", (size_t)topogrid);
		}

	/* allocate memory for topogrid structure */
	status = mb_mallocd(verbose,__FILE__,__LINE__,sizeof(struct mb_topogrid_struct),(void **)topogrid_ptr,error);

	/* get pointer to topogrid structure */
	topogrid = (struct mb_topogrid_struct *) *topogrid_ptr;

	/* read in the data */
	strcpy(topogrid->file, topogridfile);
	topogrid->data = NULL;
	status = mb_readgrd(verbose, topogrid->file, &topogrid->projection_mode, topogrid->projection_id, &topogrid->nodatavalue,
				&topogrid->nxy, &topogrid->nx, &topogrid->ny, &topogrid->min, &topogrid->max,
				&topogrid->xmin, &topogrid->xmax, &topogrid->ymin, &topogrid->ymax,
				&topogrid->dx, &topogrid->dy, &topogrid->data, NULL, NULL, error);

	/* check for reasonable results */
	if (topogrid->nxy <= 0 || topogrid->data == NULL)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
		}

	/* rationalize topogrid bounds and lonflip */
	if (status == MB_SUCCESS)
		{
		if (*lonflip == -1)
			{
			if (topogrid->xmax > 180.0)
				{
				topogrid->xmin -= 360.0;
				topogrid->xmax -= 360.0;
				}
			}
		else if (*lonflip == 0)
			{
			if (topogrid->xmin > 180.0)
				{
				topogrid->xmin -= 360.0;
				topogrid->xmax -= 360.0;
				}
			else if (topogrid->xmax < -180.0)
				{
				topogrid->xmin += 360.0;
				topogrid->xmax += 360.0;
				}
			}
		else if (*lonflip == 1)
			{
			if (topogrid->xmin < -180.0)
				{
				topogrid->xmin += 360.0;
				topogrid->xmax += 360.0;
				}
			}
		if (topogrid->xmax > 180.0)
			{
			*lonflip = 1;
			}
		else if (topogrid->xmin < -180.0)
			{
			*lonflip = -1;
			}
		else
			{
			*lonflip = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MB7K2SS function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       lonflip:                   %d\n", *lonflip);
		fprintf(stderr,"dbg2       topogrid:                  %lu\n", (size_t)topogrid);
		fprintf(stderr,"dbg2       topogrid->file:            %s\n", topogrid->file);
		fprintf(stderr,"dbg2       topogrid->projection_mode: %d\n", topogrid->projection_mode);
		fprintf(stderr,"dbg2       topogrid->projection_id:   %s\n", topogrid->projection_id);
		fprintf(stderr,"dbg2       topogrid->nodatavalue:     %f\n", topogrid->nodatavalue);
		fprintf(stderr,"dbg2       topogrid->nxy:             %d\n", topogrid->nxy);
		fprintf(stderr,"dbg2       topogrid->nx:              %d\n", topogrid->nx);
		fprintf(stderr,"dbg2       topogrid->ny:              %d\n", topogrid->ny);
		fprintf(stderr,"dbg2       topogrid->min:             %f\n", topogrid->min);
		fprintf(stderr,"dbg2       topogrid->max:             %f\n", topogrid->max);
		fprintf(stderr,"dbg2       topogrid->xmin:            %f\n", topogrid->xmin);
		fprintf(stderr,"dbg2       topogrid->xmax:            %f\n", topogrid->xmax);
		fprintf(stderr,"dbg2       topogrid->ymin:            %f\n", topogrid->ymin);
		fprintf(stderr,"dbg2       topogrid->ymax:            %f\n", topogrid->ymax);
		fprintf(stderr,"dbg2       topogrid->dx:              %f\n", topogrid->dx);
		fprintf(stderr,"dbg2       topogrid->dy               %f\n", topogrid->dy);
		fprintf(stderr,"dbg2       topogrid->data:            %lu\n", (size_t)topogrid->data);
		fprintf(stderr,"dbg2       error:                     %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_topogrid_deall(int verbose, void **topogrid_ptr, int *error)
{
	char	*function_name = "mb_intersecttopogrid";
	int	status = MB_SUCCESS;
	struct mb_topogrid_struct *topogrid;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       topogrid_ptr:              %lu\n", (size_t)topogrid_ptr);
		fprintf(stderr,"dbg2       topogrid:                  %lu\n", (size_t)(*topogrid_ptr));
		}

	/* deallocate the topogrid structure */
	topogrid = (struct mb_topogrid_struct *) *topogrid_ptr;
	if (topogrid->data != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(topogrid->data), error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)topogrid_ptr,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MB7K2SS function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:                     %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_topogrid_intersect(int verbose, void *topogrid_ptr,
			double navlon, double navlat,
			double altitude, double sonardepth,
			double mtodeglon, double mtodeglat,
			double vx, double vy, double vz,
			double *lon, double *lat, double *topo, double *range,
			int *error)
{
	char	*function_name = "mb_intersecttopogrid";
	int	status = MB_SUCCESS;
	struct mb_topogrid_struct *topogrid;
	int	done;
	int	iteration;
	int	iteration_max = 25;
	double	topotolerance = 0.1;
	double	dr, r, lontest, lattest;
	double	rmin, rmax;
	double	topotest, topog, dtopo;
	int	nfound;
	int	i, j, ii, jj, k;

	/* get pointer to topogrid structure */
	topogrid = (struct mb_topogrid_struct *) topogrid_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       navlon:                    %f\n", navlon);
		fprintf(stderr,"dbg2       navlat:                    %f\n", navlat);
		fprintf(stderr,"dbg2       altitude:                  %f\n", altitude);
		fprintf(stderr,"dbg2       sonardepth:                %f\n", sonardepth);
		fprintf(stderr,"dbg2       mtodeglon:                 %f\n", mtodeglon);
		fprintf(stderr,"dbg2       mtodeglat:                 %f\n", mtodeglat);
		fprintf(stderr,"dbg2       vx:                        %f\n", vx);
		fprintf(stderr,"dbg2       vy:                        %f\n", vy);
		fprintf(stderr,"dbg2       vz:                        %f\n", vz);
		fprintf(stderr,"dbg2       topogrid:                  %lu\n", (size_t)topogrid);
		fprintf(stderr,"dbg2       topogrid->projection_mode: %d\n", topogrid->projection_mode);
		fprintf(stderr,"dbg2       topogrid->projection_id:   %s\n", topogrid->projection_id);
		fprintf(stderr,"dbg2       topogrid->nodatavalue:     %f\n", topogrid->nodatavalue);
		fprintf(stderr,"dbg2       topogrid->nxy:             %d\n", topogrid->nxy);
		fprintf(stderr,"dbg2       topogrid->nx:              %d\n", topogrid->nx);
		fprintf(stderr,"dbg2       topogrid->ny:              %d\n", topogrid->ny);
		fprintf(stderr,"dbg2       topogrid->min:             %f\n", topogrid->min);
		fprintf(stderr,"dbg2       topogrid->max:             %f\n", topogrid->max);
		fprintf(stderr,"dbg2       topogrid->xmin:            %f\n", topogrid->xmin);
		fprintf(stderr,"dbg2       topogrid->xmax:            %f\n", topogrid->xmax);
		fprintf(stderr,"dbg2       topogrid->ymin:            %f\n", topogrid->ymin);
		fprintf(stderr,"dbg2       topogrid->ymax:            %f\n", topogrid->ymax);
		fprintf(stderr,"dbg2       topogrid->dx:              %f\n", topogrid->dx);
		fprintf(stderr,"dbg2       topogrid->dy               %f\n", topogrid->dy);
		fprintf(stderr,"dbg2       topogrid->data:            %lu\n", (size_t)topogrid->data);
		}

	/* test different ranges along the vector until the grid is intersected */
	done = MB_NO;
	iteration = 0;
	dr = altitude / 20;
	r = altitude / vz - dr;
	topog = 0.0;
	topotest = 0.0;
	dtopo = 0.0;
	rmin = 0.0;
	rmax = 4 * altitude / vz;
	while (done == MB_NO && iteration < iteration_max)
		{
		/* update the range to be tested */
		r += dr;

		/* get position of range estimate projected along the vector */
		lontest = navlon + mtodeglon * vx * r;
		lattest = navlat + mtodeglat * vy * r;
		topotest = -sonardepth - vz * r;

		/* get topography value at that point */
		nfound = 0;
		topog = 0.0;
		i = (int)((lontest - topogrid->xmin) / topogrid->dx);
		j = (int)((lattest - topogrid->ymin) / topogrid->dy);
		if (i >= 0 && i < topogrid->nx - 1
		    && j >= 0 && j < topogrid->ny - 1)
			{
			for (ii=i;ii<=i+1;ii++)
			for (jj=j;jj<=j+1;jj++)
			    {
			    k = ii * topogrid->ny + jj;
			    if (topogrid->data[k] != topogrid->nodatavalue)
				{
				nfound++;
				topog += topogrid->data[k];
				}
			    }
			}
		else
			{
			done = MB_YES;
			status = MB_FAILURE;
			*error = MB_ERROR_NOT_ENOUGH_DATA;
			}
		if (nfound > 0)
			{
			topog /= (double)nfound;
			}

		/* compare topographies at projected position */
		if (nfound > 0)
			{
			dtopo = topotest - topog;
			if (fabs(dtopo) < topotolerance)
				{
				done = MB_YES;
				}
			else
				{
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
		else
			{
			}

		/* keep count of iterations */
		iteration++;
		}

	/* if success return the result */
	*lon = navlon + mtodeglon * vx * r;
	*lat = navlat + mtodeglat * vy * r;
	*topo = -sonardepth - vz * r;
	*range = r;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MB7K2SS function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       lon:             %f\n",*lon);
		fprintf(stderr,"dbg2       lat:             %f\n",*lat);
		fprintf(stderr,"dbg2       topo:            %f\n",*topo);
		fprintf(stderr,"dbg2       range:           %f\n",*range);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_topogrid_getangletable(int verbose, void *topogrid_ptr,
				int nangle, double angle_min, double angle_max,
				double navlon, double navlat, double heading,
				double altitude, double sonardepth, double pitch,
				double *table_angle, double *table_xtrack,
				double *table_ltrack, double *table_altitude,
				double *table_range, int *error)
{
	char	*function_name = "mb_topogrid_getangletable";
	int	status = MB_SUCCESS;
	struct mb_topogrid_struct *topogrid;
	double	mtodeglon, mtodeglat;
	double	dangle;
	double	rr, xx, zz;
	double	alpha, beta, theta, phi;
	double	vx, vy, vz;
	double	lon, lat, topo;
	int	nset, first, last;
	int	i;

	/* get pointer to topogrid structure */
	topogrid = (struct mb_topogrid_struct *) topogrid_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       nangle:                    %d\n", nangle);
		fprintf(stderr,"dbg2       angle_min:                 %f\n", angle_min);
		fprintf(stderr,"dbg2       angle_max:                 %f\n", angle_max);
		fprintf(stderr,"dbg2       navlon:                    %f\n", navlon);
		fprintf(stderr,"dbg2       navlat:                    %f\n", navlat);
		fprintf(stderr,"dbg2       heading:                   %f\n", heading);
		fprintf(stderr,"dbg2       altitude:                  %f\n", altitude);
		fprintf(stderr,"dbg2       sonardepth:                %f\n", sonardepth);
		fprintf(stderr,"dbg2       pitch:                     %f\n", pitch);
		fprintf(stderr,"dbg2       topogrid:                  %lu\n", (size_t)topogrid);
		fprintf(stderr,"dbg2       topogrid->projection_mode: %d\n", topogrid->projection_mode);
		fprintf(stderr,"dbg2       topogrid->projection_id:   %s\n", topogrid->projection_id);
		fprintf(stderr,"dbg2       topogrid->nodatavalue:     %f\n", topogrid->nodatavalue);
		fprintf(stderr,"dbg2       topogrid->nxy:             %d\n", topogrid->nxy);
		fprintf(stderr,"dbg2       topogrid->nx:              %d\n", topogrid->nx);
		fprintf(stderr,"dbg2       topogrid->ny:              %d\n", topogrid->ny);
		fprintf(stderr,"dbg2       topogrid->min:             %f\n", topogrid->min);
		fprintf(stderr,"dbg2       topogrid->max:             %f\n", topogrid->max);
		fprintf(stderr,"dbg2       topogrid->xmin:            %f\n", topogrid->xmin);
		fprintf(stderr,"dbg2       topogrid->xmax:            %f\n", topogrid->xmax);
		fprintf(stderr,"dbg2       topogrid->ymin:            %f\n", topogrid->ymin);
		fprintf(stderr,"dbg2       topogrid->ymax:            %f\n", topogrid->ymax);
		fprintf(stderr,"dbg2       topogrid->dx:              %f\n", topogrid->dx);
		fprintf(stderr,"dbg2       topogrid->dy               %f\n", topogrid->dy);
		fprintf(stderr,"dbg2       topogrid->data:            %lu\n", (size_t)topogrid->data);
		}

	/* loop over all of the angles */
	mb_coor_scale(verbose,navlat, &mtodeglon, &mtodeglat);
	dangle = (angle_max - angle_min) / (nangle - 1);
	alpha = pitch;
	nset = 0;
	for (i=0;i<nangle;i++)
		{
		/* get angles in takeoff coordinates */
		table_angle[i] = angle_min + dangle * i;
		beta = 90.0 - table_angle[i];
		mb_rollpitch_to_takeoff(
			verbose,
			alpha, beta,
			&theta, &phi,
			error);

		/* calculate unit vector relative to the vehicle */
		vz = cos(DTR * theta);
		vx = sin(DTR * theta) * cos(DTR * phi);
		vy = sin(DTR * theta) * sin(DTR * phi);

		/* rotate unit vector by vehicle heading */
		vx = vx * cos(DTR * heading) + vy * sin(DTR * heading);
		vy = -vx * sin(DTR * heading) + vy * cos(DTR * heading);

		/* find the range where this vector intersects the grid */
		status = mb_topogrid_intersect(verbose, topogrid_ptr,
						navlon, navlat, altitude, sonardepth,
						mtodeglon, mtodeglat, vx, vy, vz,
						&lon, &lat, &topo, &rr, error);

		/* get the position from successful intersection with the grid */
		if (status == MB_SUCCESS)
			{
			zz = rr * cos(DTR * theta);
			xx = rr * sin(DTR * theta);
			table_xtrack[i] = xx * cos(DTR * phi);
			table_ltrack[i] = xx * sin(DTR * phi);
			table_altitude[i] = zz;
			table_range[i] = rr;
			nset++;
			}

		/* zero table values for the moment */
		else
			{
			table_range[i] = 0.0;
			}
		}

	/* now deal with any unset table entries */
	if (nset < nangle)
		{
		/* find first and last table entries set if possible */
		if (nset > 0)
			{
			first = nangle;
			last = -1;
			for (i=0;i<nangle;i++)
				{
				if (table_range[i] > 0.0)
					{
					first = MIN(i,first);
					last = MAX(i,last);
					}
				}

			/* apply flat bottom calculation to unset entries */
			for (i=0;i<nangle;i++)
				{
				if (table_range[i] <= 0.0)
					{
					/* get angles in takeoff coordinates */
					table_angle[i] = angle_min + dangle * i;
					beta = 90.0 - table_angle[i];
					mb_rollpitch_to_takeoff(
						verbose,
						alpha, beta,
						&theta, &phi,
						error);

					if (nset == 0)
						{
						table_altitude[i] = altitude;
						}
					else if (i < first)
						{
						table_altitude[i] = table_altitude[first];
						}
					else if (i > last)
						{
						table_altitude[i] = table_altitude[last];
						}
					else
						{
						table_altitude[i] = 0.5 * (table_altitude[first] + table_altitude[last]);
						}

					table_range[i]  = table_altitude[first] / cos(DTR * theta);
					xx = table_range[i] * sin(DTR * theta);
					table_xtrack[i] = xx * cos(DTR * phi);
					table_ltrack[i] = xx * sin(DTR * phi);
					nset++;
					}
				}
			}
		}

	/* reset error condition */
	if (nset >= nangle)
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MB7K2SS function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       Lookup tables:\n");
		for (i=0;i<nangle;i++)
			fprintf(stderr,"dbg2         %d %f %f %f %f %f\n",
				i, table_angle[i], table_xtrack[i], table_ltrack[i], table_altitude[i], table_range[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
