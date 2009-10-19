/*--------------------------------------------------------------------
 *    The MB-system:	mb_readgrd.c	12/10/2007
 *    $Id$
 *
 *    Copyright (c) 2007-2009 by
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
 * Standalone function to read a GMT grid for programs not interfacing
 * with other GMT functionality.
 *
 * Author:	D. W. Caress
 * Date:	September 3, 2007
 *
 * $Log: mb_readgrd.c,v $
 * Revision 5.1  2008/07/10 06:43:40  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.0  2007/10/08 05:47:27  caress
 * Added grd reading function.
 *
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

/* GMT include files */
#include "gmt.h"

/* get NaN detector */
#if defined(isnanf)
#define check_fnan(x) isnanf((x))
#elif defined(isnan)
#define check_fnan(x) isnan((double)(x))
#elif HAVE_ISNANF == 1
#define check_fnan(x) isnanf(x)
extern int isnanf(float x);
#elif HAVE_ISNAN == 1
#define check_fnan(x) isnan((double)(x))
#elif HAVE_ISNAND == 1
#define check_fnan(x) isnand((double)(x))
#else
#define check_fnan(x) ((x) != (x))
#endif

/* Projection defines */
#define ModelTypeProjected	     1
#define ModelTypeGeographic	     2
#define GCS_WGS_84		  4326

/* default no data value define */
#define	DEFAULT_NODATA		-9999999.9

static char rcs_id[] = "$Id$";

/* global variables */
static char program_name[] = "mb_readgrd";
static int	pargc;
static char	**pargv;

/*--------------------------------------------------------------------------*/
int mb_readgrd(int verbose, char *grdfile,
			int	*grid_projection_mode,
			char	*grid_projection_id,
			float	*nodatavalue,
			int	*nxy,
			int	*nx,
			int	*ny,
			double	*min,
			double	*max,
			double	*xmin,
			double	*xmax,
			double	*ymin,
			double	*ymax,
			double	*dx,
			double	*dy,
			float	**data,
			float	**data_dzdx,
			float	**data_dzdy,
			int	*error)
{
	char function_name[] = "mb_readgrd";
	int	status = MB_SUCCESS;
	struct GRD_HEADER header;
	int	modeltype;
	int	projectionid;
        mb_path    projectionname;
	int	off;
	GMT_LONG	pad[4];
	int	nscan;
	int	utmzone;
	char	NorS;
	float	*rawdata;
	float	*usedata;
	double	mtodeglon, mtodeglat;
	double	ddx, ddy;
	int	kx0, kx2, ky0, ky2;
	int	i, j, k, ii, jj, kk;
	
	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n", verbose);
		fprintf(stderr,"dbg2       grdfile:         %s\n", grdfile);
		}
	
	/* do required initialization */
/*	GMT_begin (pargc, pargv);
	GMT_put_history(pargc, pargv);
	GMT_get_common_args (projection, xmin, xmax, ymin, ymax);*/
	project_info.degree[0] = 0;
	project_info.degree[1] = 0;
	GMT_program = program_name;
	GMT_grd_init (&header, pargc, pargv, FALSE);
	GMT_io_init ();
	GMT_grdio_init ();
	GMT_make_fnan (GMT_f_NaN);
	GMT_make_dnan (GMT_d_NaN);
	
	/* read input grd file header */
	if (GMT_read_grd_info (grdfile, &header)) 
		{
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		}
	    
	/* proceed if ok */
	if (status == MB_SUCCESS)
		{
		/* try to get projection from the grd file remark */
		if (strncmp(&(header.remark[2]), "Projection: ", 12) == 0)
			{
			if ((nscan = sscanf(&(header.remark[2]), "Projection: UTM%d%c", &utmzone, &NorS)) == 2)
				{
				if (NorS == 'N')
					{
					projectionid = 32600 + utmzone;
					}
				else if (NorS == 'S')
					{
					projectionid = 32700 + utmzone;
					}
				modeltype = ModelTypeProjected;
				sprintf(projectionname, "UTM%2.2d%c", utmzone, NorS);
				*grid_projection_mode = MB_PROJECTION_PROJECTED;
				sprintf(grid_projection_id, "epsg%d", projectionid);

				project_info.degree[0] = FALSE;
				}
			else if ((nscan = sscanf(&(header.remark[2]), "Projection: epsg%d", &projectionid)) == 1)
				{
				sprintf(projectionname, "epsg%d", projectionid);
				modeltype = ModelTypeProjected;
				*grid_projection_mode = MB_PROJECTION_PROJECTED;
				sprintf(grid_projection_id, "epsg%d", projectionid);

				project_info.degree[0] = FALSE;
				}
			else
				{
				strcpy(projectionname, "Geographic WGS84");
				modeltype = ModelTypeGeographic;
				projectionid = GCS_WGS_84;
				*grid_projection_mode = MB_PROJECTION_GEOGRAPHIC;
				sprintf(grid_projection_id, "epsg%d", projectionid);

				project_info.degree[0] = TRUE;
				GMT_io.in_col_type[0] = GMT_IS_LON;
				GMT_io.in_col_type[1] = GMT_IS_LAT;
				}
			}
		else
			{
			strcpy(projectionname, "Geographic WGS84");
			modeltype = ModelTypeGeographic;
			projectionid = GCS_WGS_84;
			*grid_projection_mode = MB_PROJECTION_GEOGRAPHIC;
			sprintf(grid_projection_id, "epsg%d", projectionid);

			project_info.degree[0] = TRUE;
			GMT_io.in_col_type[0] = GMT_IS_LON;
			GMT_io.in_col_type[1] = GMT_IS_LAT;
			}	

		/* set up internal arrays */
    		*nodatavalue = MIN(DEFAULT_NODATA, header.z_min - 10 * (header.z_max - header.z_min));
    		*nxy = header.nx * header.ny;
    		*nx = header.nx;
    		*ny = header.ny;
    		*xmin = header.x_min; 
    		*xmax = header.x_max; 
    		*ymin = header.y_min; 
    		*ymax = header.y_max; 
    		*dx = header.x_inc;
    		*dy = header.y_inc;
    		*min = header.z_min; 
    		*max = header.z_max; 

    		status = mb_mallocd(verbose, __FILE__,__LINE__, sizeof(float) * (*nxy), 
    					(void **)&rawdata, error);
    		if (status == MB_SUCCESS)
		status = mb_mallocd(verbose, __FILE__,__LINE__, sizeof(float) * (*nxy), 
    					(void **)&usedata, error);
		*data = usedata;
    		if (status == MB_SUCCESS && data_dzdx != NULL)
			{
			status = mb_mallocd(verbose, __FILE__,__LINE__, sizeof(float) * (*nxy), 
    					(void **)&usedata, error);
			*data_dzdx = usedata;
			}
    		if (status == MB_SUCCESS && data_dzdy != NULL)
			{
			status = mb_mallocd(verbose, __FILE__,__LINE__, sizeof(float) * (*nxy), 
    					(void **)&usedata, error);
			*data_dzdy = usedata;
			}
		}
	    
	/* proceed if ok */
	if (status == MB_SUCCESS)
		{
		/* Determine the wesn to be used to read the grdfile */
		off = (header.node_offset) ? 0 : 1;
		GMT_map_setup (*xmin, *xmax, *ymin, *ymax);
#ifdef GMT_MINOR_VERSION
		GMT_grd_setregion (&header, xmin,  xmax, ymin, ymax, BCR_BILINEAR);
#else
		GMT_grd_setregion (&header, xmin,  xmax, ymin, ymax);
#endif

		/* read the grid */
		pad[0] = 0;
		pad[1] = 0;
		pad[2] = 0;
		pad[3] = 0;
		if (GMT_read_grd (grdfile, &header, rawdata, 
				    *xmin, *xmax, *ymin, *ymax, 
				    pad, FALSE))
			{
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			}

		/* free GMT memory */
		GMT_free ((void *)GMT_io.skip_if_NaN);
		GMT_free ((void *)GMT_io.in_col_type);
		GMT_free ((void *)GMT_io.out_col_type);
		}
	    
	/* reorder grid to internal convention */
	if (status == MB_SUCCESS)
		{
		for (i=0;i<*nx;i++)
		for (j=0;j<*ny;j++)
			{
			k = i * *ny + j;
			kk = (*ny - 1 - j) * *nx + i;
			if (check_fnan(rawdata[kk]))
				(*data)[k] = *nodatavalue;
			else
				(*data)[k] = rawdata[kk];
			}
		mb_freed(verbose,__FILE__, __LINE__, (void **)&rawdata, error);
		}
	    
	/* calculate derivatives */
	if (status == MB_SUCCESS && data_dzdx != NULL && data_dzdy != NULL)
		{
		ddx = *dx;
		ddy = *dy;
		if (*grid_projection_mode == MB_PROJECTION_GEOGRAPHIC)
			{
			mb_coor_scale(verbose,0.5 * (*ymin + *ymax), &mtodeglon, &mtodeglat);
			ddx /= mtodeglon;
			ddy /= mtodeglon;
			}
		for (i=0;i<*nx;i++)
		for (j=0;j<*ny;j++)
			{
			k = i * (*ny) + j;
			ii = 0;
			jj = 0;
			if (i > 0)
				{
				kx0 = (i - 1) * (*ny) + j;
				ii++;
				}
			else
				kx0 = k;
			if (i < *nx - 1)
				{
				kx2 = (i + 1) * (*ny) + j;
				ii++;
				}
			else
				kx0 = k;
			if (j > 0)
				{
				ky0 = i * (*ny) + j + 1;
				jj++;
				}
			else
				ky0 = k;
			if (j < *ny - 1)
				{
				ky2 = i * (*ny) + j - 1;
				jj++;
				}
			else
				ky2 = k;
			if (ii > 0)
				(*data_dzdx)[k] = ((*data)[kx2] - (*data)[kx0]) / (((double)ii) * ddx);
			if (jj > 0)
				(*data_dzdy)[k] = ((*data)[ky2] - (*data)[ky0]) / (((double)jj) * ddy);
			}
		}

	/* print debug info */
	if (verbose > 0)
		{
		fprintf(stderr,"Grid read:\n");
		fprintf(stderr,"  Dimensions: %d %d\n", header.nx, header.ny);
		if (modeltype == ModelTypeProjected)
			{
			fprintf(stderr,"  Projected Coordinate System Name: %s\n", projectionname);
			fprintf(stderr,"  Projected Coordinate System ID:   %d\n", projectionid);
			fprintf(stderr,"  Easting:    %f %f  %f\n",
				header.x_min, header.x_max, header.x_inc);
			fprintf(stderr,"  Northing:   %f %f  %f\n",
				header.y_min, header.y_max, header.y_inc);
			}
		else
			{
			fprintf(stderr,"  Geographic Coordinate System Name: %s\n", projectionname);
			fprintf(stderr,"  Geographic Coordinate System ID:   %d\n", projectionid);
			fprintf(stderr,"  Longitude:  %f %f  %f\n",
				header.x_min, header.x_max, header.x_inc);
			fprintf(stderr,"  Latitude:   %f %f  %f\n",
				header.y_min, header.y_max, header.y_inc);
			}
		fprintf(stderr,"  Internal Grid Projection Mode:         %d\n", 
	    			*grid_projection_mode);
		fprintf(stderr,"  Internal Grid Projection ID:           %s\n", 
	    			grid_projection_id);

		fprintf(stderr,"Data Read:\n");
		fprintf(stderr,"  grid_projection_mode:     %d\n", *grid_projection_mode);
		fprintf(stderr,"  grid_projection_id:       %s\n", grid_projection_id);
		fprintf(stderr,"  nodatavalue:              %f\n", *nodatavalue);
		fprintf(stderr,"  nx:                       %d\n", *nx);
		fprintf(stderr,"  ny:                       %d\n", *ny);
		fprintf(stderr,"  min:                      %f\n", *min);
		fprintf(stderr,"  max:                      %f\n", *max);
		fprintf(stderr,"  xmin:                     %f\n", *xmin);
		fprintf(stderr,"  xmax:                     %f\n", *xmax);
		fprintf(stderr,"  ymin:                     %f\n", *ymin);
		fprintf(stderr,"  ymax:                     %f\n", *ymax);
		fprintf(stderr,"  dx:                       %f\n", *dx);
		fprintf(stderr,"  dy:                       %f\n", *dy);
		fprintf(stderr,"  data:                     %ld\n", (long)*data);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       Dimensions: %d %d\n", header.nx, header.ny);
		if (modeltype == ModelTypeProjected)
			{
			fprintf(stderr,"dbg2       Projected Coordinate System Name: %s\n", projectionname);
			fprintf(stderr,"dbg2       Projected Coordinate System ID:   %d\n", projectionid);
			fprintf(stderr,"dbg2       Easting:                  %f %f  %f\n",header.x_min, header.x_max, header.x_inc);
			fprintf(stderr,"dbg2       Northing:                 %f %f  %f\n",header.y_min, header.y_max, header.y_inc);
			}
		else
			{
			fprintf(stderr,"dbg2       Geographic Coordinate System Name: %s\n", projectionname);
			fprintf(stderr,"dbg2       Geographic Coordinate System ID:   %d\n", projectionid);
			fprintf(stderr,"dbg2       Longitude:                %f %f  %f\n",header.x_min, header.x_max, header.x_inc);
			fprintf(stderr,"dbg2       Latitude:                 %f %f  %f\n",header.y_min, header.y_max, header.y_inc);
			}
		fprintf(stderr,"dbg2       Internal Grid Projection Mode: %d\n", *grid_projection_mode);
		fprintf(stderr,"dbg2       Internal Grid Projection ID:   %s\n", grid_projection_id);
		fprintf(stderr,"Data Read:\n");
		fprintf(stderr,"dbg2       grid_projection_mode:     %d\n", *grid_projection_mode);
		fprintf(stderr,"dbg2       grid_projection_id:       %s\n", grid_projection_id);
		fprintf(stderr,"dbg2       nodatavalue:              %f\n", *nodatavalue);
		fprintf(stderr,"dbg2       nx:                       %d\n", *nx);
		fprintf(stderr,"dbg2       ny:                       %d\n", *ny);
		fprintf(stderr,"dbg2       min:                      %f\n", *min);
		fprintf(stderr,"dbg2       max:                      %f\n", *max);
		fprintf(stderr,"dbg2       xmin:                     %f\n", *xmin);
		fprintf(stderr,"dbg2       xmax:                     %f\n", *xmax);
		fprintf(stderr,"dbg2       ymin:                     %f\n", *ymin);
		fprintf(stderr,"dbg2       ymax:                     %f\n", *ymax);
		fprintf(stderr,"dbg2       dx:                       %f\n", *dx);
		fprintf(stderr,"dbg2       dy:                       %f\n", *dy);
		fprintf(stderr,"dbg2       data:                     %ld\n", (long)*data);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
