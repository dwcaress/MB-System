/*--------------------------------------------------------------------
 *    The MB-system:	mb_grdio.c	12/10/2007
 *    $Id$
 *
 *    Copyright (c) 2007-2017 by
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
 * Standalone functions to read or write a GMT grid for programs not interfacing
 * with other GMT functionality.
 *
 * Author:	D. W. Caress
 * Date:	September 3, 2007
 *
 */

/* standard include files */
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

/* GMT include files */
#include "gmt.h"

/* MBIO include files */
#include "mb_status.h"
#include "mb_define.h"
#include "mb_aux.h"

/* Projection defines */
#define ModelTypeProjected 1
#define ModelTypeGeographic 2
#define GCS_WGS_84 4326

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------------*/
int mb_read_gmt_grd(int verbose, char *grdfile, int *grid_projection_mode, char *grid_projection_id, float *nodatavalue, int *nxy,
                    int *n_columns, int *n_rows, double *min, double *max, double *xmin, double *xmax, double *ymin, double *ymax,
                    double *dx, double *dy, float **data, float **data_dzdx, float **data_dzdy, int *error) {
	char function_name[] = "mb_read_gmt_grd";
	int status = MB_SUCCESS;
	void *API = NULL;               /* GMT API control structure pointer */
	struct GMT_GRID *G = NULL;      /* GMT grid structure pointer */
	struct GMT_GRID_HEADER *header; /* GMT grid header structure pointer */
	int modeltype;
	int projectionid;
	mb_path projectionname = "";
	struct stat file_status;
	int num_tries;
	int nscan;
	int utmzone;
	char NorS;
	float *usedata;
	double mtodeglon, mtodeglat;
	double ddx, ddy;
	int kx0, kx2, ky0, ky2;
	int i, j, k, ii, jj, kk;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBBA function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       grdfile:         %s\n", grdfile);
	}
	
	/* check if the file exists and is readable */
	if (stat(grdfile, &file_status) == 0
		&& (file_status.st_mode & S_IFMT) != S_IFDIR
		&& file_status.st_size > 0) {
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}
	else {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
	}

	/* if file exists proceed */
	if (status == MB_SUCCESS) {
		
		/* Initialize new GMT session */
		if ((API = GMT_Create_Session(function_name, 2U, 0U, NULL)) == NULL) {
			fprintf(stderr, "\nUnable to initialize a GMT session with GMT_Create_Session() in function %s\n", function_name);
			fprintf(stderr, "Unable to read GMT grid file %s\n",grdfile);
			fprintf(stderr, "Program terminated\n");
			exit(EXIT_FAILURE);
		}
	
		/* read in the grid */
		num_tries = 0;
		while (G == NULL && num_tries < 100) {
			if ((G = GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, grdfile, NULL)) == NULL) {
				num_tries++;
				usleep(1000);
				//fprintf(stderr,"Failed to read grid with GMT_Read_Data(), attempt %d\n", num_tries);
			} else {
				if (num_tries > 0) {
					fprintf(stderr, "Read grid on try %d\n", num_tries);
				}
			}
		}
		if (G == NULL) {
			fprintf(stderr, "\nUnable to read GMT grid file %s with GMT_Read_Data() after %d tries in function %s\n",
					grdfile, num_tries, function_name);
			fprintf(stderr, "Program terminated\n");
			exit(EXIT_FAILURE);
		}
	
		/* proceed if ok */
		if (status == MB_SUCCESS) {
			/* try to get projection from the grd file remark */
			header = G->header;
			if (strncmp(&(header->remark[2]), "Projection: ", 12) == 0) {
				if ((nscan = sscanf(&(header->remark[2]), "Projection: UTM%d%c", &utmzone, &NorS)) == 2) {
					if (NorS == 'N') {
						projectionid = 32600 + utmzone;
					}
					else if (NorS == 'S') {
						projectionid = 32700 + utmzone;
					}
					else {
						projectionid = 32600 + utmzone;
					}
					modeltype = ModelTypeProjected;
					sprintf(projectionname, "UTM%2.2d%c", utmzone, NorS);
					*grid_projection_mode = MB_PROJECTION_PROJECTED;
					sprintf(grid_projection_id, "epsg%d", projectionid);
				}
				else if ((nscan = sscanf(&(header->remark[2]), "Projection: epsg%d", &projectionid)) == 1) {
					sprintf(projectionname, "epsg%d", projectionid);
					modeltype = ModelTypeProjected;
					*grid_projection_mode = MB_PROJECTION_PROJECTED;
					sprintf(grid_projection_id, "epsg%d", projectionid);
				}
				else {
					strcpy(projectionname, "Geographic WGS84");
					modeltype = ModelTypeGeographic;
					projectionid = GCS_WGS_84;
					*grid_projection_mode = MB_PROJECTION_GEOGRAPHIC;
					sprintf(grid_projection_id, "epsg%d", projectionid);
				}
			}
			else {
				strcpy(projectionname, "Geographic WGS84");
				modeltype = ModelTypeGeographic;
				projectionid = GCS_WGS_84;
				*grid_projection_mode = MB_PROJECTION_GEOGRAPHIC;
				sprintf(grid_projection_id, "epsg%d", projectionid);
			}
	
			/* set up internal arrays */
			*nodatavalue = MIN(MB_DEFAULT_GRID_NODATA, header->z_min - 10 * (header->z_max - header->z_min));
			*nxy = header->n_columns * header->n_rows;
			*n_columns = header->n_columns;
			*n_rows = header->n_rows;
			*xmin = header->wesn[0];
			*xmax = header->wesn[1];
			*ymin = header->wesn[2];
			*ymax = header->wesn[3];
			*dx = header->inc[0];
			*dy = header->inc[1];
			*min = header->z_min;
			*max = header->z_max;
	
			status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * (*nxy), (void **)&usedata, error);
			if (status == MB_SUCCESS) {
				*data = usedata;
			}
			if (status == MB_SUCCESS && data_dzdx != NULL) {
				status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * (*nxy), (void **)&usedata, error);
				*data_dzdx = usedata;
			}
			if (status == MB_SUCCESS && data_dzdy != NULL) {
				status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * (*nxy), (void **)&usedata, error);
				*data_dzdy = usedata;
			}
		}
	
		/* copy grid data, reordering to internal convention */
		if (status == MB_SUCCESS) {
			for (i = 0; i < *n_columns; i++)
				for (j = 0; j < *n_rows; j++) {
					k = i * *n_rows + j;
					kk = (*n_rows + header->pad[2] + header->pad[3] - 1 - j) * (*n_columns + header->pad[0] + header->pad[1]) +
						 (i + header->pad[0]);
					if (MB_IS_FNAN(G->data[kk]))
						(*data)[k] = *nodatavalue;
					else
						(*data)[k] = G->data[kk];
				}
		}
	
		/* calculate derivatives */
		if (status == MB_SUCCESS && data_dzdx != NULL && data_dzdy != NULL) {
			ddx = *dx;
			ddy = *dy;
			if (*grid_projection_mode == MB_PROJECTION_GEOGRAPHIC) {
				mb_coor_scale(verbose, 0.5 * (*ymin + *ymax), &mtodeglon, &mtodeglat);
				ddx /= mtodeglon;
				ddy /= mtodeglon;
			}
			for (i = 0; i < *n_columns; i++)
				for (j = 0; j < *n_rows; j++) {
					k = i * (*n_rows) + j;
					ii = 0;
					jj = 0;
					if (i > 0) {
						kx0 = (i - 1) * (*n_rows) + j;
						ii++;
					}
					else
						kx0 = k;
					if (i < *n_columns - 1) {
						kx2 = (i + 1) * (*n_rows) + j;
						ii++;
					}
					else
						kx0 = k;
					if (j > 0) {
						ky0 = i * (*n_rows) + j + 1;
						jj++;
					}
					else
						ky0 = k;
					if (j < *n_rows - 1) {
						ky2 = i * (*n_rows) + j - 1;
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
	
		/* Destroy GMT session */
		if (GMT_Destroy_Session(API) != 0) {
			fprintf(stderr, "\nUnable to destroy a GMT session with GMT_Destroy_Session() in function %s\n", function_name);
			fprintf(stderr, "Unable to read GMT grid file %s\n",grdfile);
			fprintf(stderr, "Program terminated\n");
			exit(EXIT_FAILURE);
		}
	}

	/* print debug info */
	if (verbose > 0) {
		fprintf(stderr, "\nGrid read:\n");
		fprintf(stderr, "  Dimensions:     %d %d\n", header->n_columns, header->n_rows);
		fprintf(stderr, "  Registration:   %d\n", header->registration);
		if (modeltype == ModelTypeProjected) {
			fprintf(stderr, "  Projected Coordinate System Name: %s\n", projectionname);
			fprintf(stderr, "  Projected Coordinate System ID:   %d\n", projectionid);
			fprintf(stderr, "  Easting:    %f %f  %f\n", header->wesn[0], header->wesn[1], header->inc[0]);
			fprintf(stderr, "  Northing:   %f %f  %f\n", header->wesn[2], header->wesn[3], header->inc[1]);
		}
		else {
			fprintf(stderr, "  Geographic Coordinate System Name: %s\n", projectionname);
			fprintf(stderr, "  Geographic Coordinate System ID:   %d\n", projectionid);
			fprintf(stderr, "  Longitude:  %f %f  %f\n", header->wesn[0], header->wesn[1], header->inc[0]);
			fprintf(stderr, "  Latitude:   %f %f  %f\n", header->wesn[2], header->wesn[3], header->inc[1]);
		}
		fprintf(stderr, "  Grid Projection Mode:     %d\n", *grid_projection_mode);
		fprintf(stderr, "  Grid Projection ID:       %s\n", grid_projection_id);
		fprintf(stderr, "  Data Extrema:             %f %f\n", header->z_min, header->z_max);
		fprintf(stderr, "  Other Grid Parameters:\n");
		fprintf(stderr, "    z_scale_factor:         %f\n", header->z_scale_factor);
		fprintf(stderr, "    z_add_offset:           %f\n", header->z_add_offset);
		fprintf(stderr, "    type:                   %d\n", header->type);
		fprintf(stderr, "    bits:                   %d\n", header->bits);
		fprintf(stderr, "    complex_mode:           %d\n", header->complex_mode);
		fprintf(stderr, "    mx:                     %d\n", header->mx);
		fprintf(stderr, "    my:                     %d\n", header->my);
		fprintf(stderr, "    nm:                     %zu\n", header->nm);
		fprintf(stderr, "    size:                   %zu\n", header->size);
		fprintf(stderr, "    pad:                    %d %d %d %d\n", header->pad[0], header->pad[1], header->pad[2],
		        header->pad[3]);
		fprintf(stderr, "    data ptr:               %p\n", G->data);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBBA function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       Dimensions: %d %d\n", header->n_columns, header->n_rows);
		if (modeltype == ModelTypeProjected) {
			fprintf(stderr, "dbg2       Projected Coordinate System Name: %s\n", projectionname);
			fprintf(stderr, "dbg2       Projected Coordinate System ID:   %d\n", projectionid);
			fprintf(stderr, "dbg2       Easting:                  %f %f  %f\n", header->wesn[0], header->wesn[1], header->inc[0]);
			fprintf(stderr, "dbg2       Northing:                 %f %f  %f\n", header->wesn[2], header->wesn[3], header->inc[1]);
		}
		else {
			fprintf(stderr, "dbg2       Geographic Coordinate System Name: %s\n", projectionname);
			fprintf(stderr, "dbg2       Geographic Coordinate System ID:   %d\n", projectionid);
			fprintf(stderr, "dbg2       Longitude:                %f %f  %f\n", header->wesn[0], header->wesn[1], header->inc[0]);
			fprintf(stderr, "dbg2       Latitude:                 %f %f  %f\n", header->wesn[2], header->wesn[3], header->inc[1]);
		}
		fprintf(stderr, "dbg2       Internal Grid Projection Mode: %d\n", *grid_projection_mode);
		fprintf(stderr, "dbg2       Internal Grid Projection ID:   %s\n", grid_projection_id);
		fprintf(stderr, "Data Read:\n");
		fprintf(stderr, "dbg2       grid_projection_mode:     %d\n", *grid_projection_mode);
		fprintf(stderr, "dbg2       grid_projection_id:       %s\n", grid_projection_id);
		fprintf(stderr, "dbg2       nodatavalue:              %f\n", *nodatavalue);
		fprintf(stderr, "dbg2       n_columns:                %d\n", *n_columns);
		fprintf(stderr, "dbg2       n_rows:                   %d\n", *n_rows);
		fprintf(stderr, "dbg2       min:                      %f\n", *min);
		fprintf(stderr, "dbg2       max:                      %f\n", *max);
		fprintf(stderr, "dbg2       xmin:                     %f\n", *xmin);
		fprintf(stderr, "dbg2       xmax:                     %f\n", *xmax);
		fprintf(stderr, "dbg2       ymin:                     %f\n", *ymin);
		fprintf(stderr, "dbg2       ymax:                     %f\n", *ymax);
		fprintf(stderr, "dbg2       dx:                       %f\n", *dx);
		fprintf(stderr, "dbg2       dy:                       %f\n", *dy);
		fprintf(stderr, "dbg2       data:                     %p\n", *data);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	/* return status */
	return (status);
} /*--------------------------------------------------------------------*/
/*
 * function write_cdfgrd writes output grid to a
 * GMT version 2 netCDF grd file
 */
int mb_write_gmt_grd(int verbose, char *grdfile, float *grid, float nodatavalue, int n_columns, int n_rows, double xmin, double xmax,
                     double ymin, double ymax, double zmin, double zmax, double dx, double dy, char *xlab, char *ylab, char *zlab,
                     char *titl, char *projection, int argc, char **argv, int *error) {
	char *function_name = "mb_write_gmt_grd";
	int status = MB_SUCCESS;

	double wesn[4];
	double inc[2];
	unsigned int registration;
	int pad;
	void *API = NULL;               /* GMT API control structure pointer */
	struct GMT_GRID *G = NULL;      /* GMT grid structure pointer */
	struct GMT_GRID_HEADER *header; /* GMT grid header structure pointer */
	unsigned int mode = GMT_GRID_ALL;

	int modeltype;
	int projectionid;
	int grid_projection_mode;
	mb_path projectionname = "";
	mb_path grid_projection_id = "";
	mb_path program_name = "";
	mb_path remark = "";
	int nscan;
	int utmzone;
	char NorS;
	time_t right_now;
	char date[32], user[MB_PATH_MAXLINE], *user_ptr;
	char host[MB_PATH_MAXLINE] = {""}, *host_ptr;
	int first = MB_NO;
	double min = 0.0;
	double max = 0.0;
	double NaN;
	int nx_node_registration;
	int i, j, k, kk;
	char *ctime();
	char *getenv();

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       grdfile:    %s\n", grdfile);
		fprintf(stderr, "dbg2       grid:       %p\n", (void *)grid);
		fprintf(stderr, "dbg2       nodatavalue:%f\n", nodatavalue);
		fprintf(stderr, "dbg2       n_columns:  %d\n", n_columns);
		fprintf(stderr, "dbg2       n_rows:     %d\n", n_rows);
		fprintf(stderr, "dbg2       xmin:       %f\n", xmin);
		fprintf(stderr, "dbg2       xmax:       %f\n", xmax);
		fprintf(stderr, "dbg2       ymin:       %f\n", ymin);
		fprintf(stderr, "dbg2       ymax:       %f\n", ymax);
		fprintf(stderr, "dbg2       dx:         %f\n", dx);
		fprintf(stderr, "dbg2       dy:         %f\n", dy);
		fprintf(stderr, "dbg2       xlab:       %s\n", xlab);
		fprintf(stderr, "dbg2       ylab:       %s\n", ylab);
		fprintf(stderr, "dbg2       zlab:       %s\n", zlab);
		fprintf(stderr, "dbg2       projection: %s\n", projection);
		fprintf(stderr, "dbg2       titl:       %s\n", titl);
		fprintf(stderr, "dbg2       argc:       %d\n", argc);
		fprintf(stderr, "dbg2       *argv:      %p\n", (void *)*argv);
	}

	/* Initializing new GMT session */
	if ((API = GMT_Create_Session(function_name, 2U, 0U, NULL)) == NULL) {
		fprintf(stderr, "\nUnable to initialize a GMT session with GMT_Create_Session() in function %s\n", function_name);
		fprintf(stderr, "Unable to write GMT grid file %s\n",grdfile);
		fprintf(stderr, "Program terminated\n");
		exit(EXIT_FAILURE);
	}

	/* set grid creation control values */
	/* GMT_GRID_NODE_REG (0) for node grids, GMT_GRID_PIXEL_REG (1) for pixel grids */
	nx_node_registration = lround((xmax - xmin) / dx + 1);
	if (n_columns == nx_node_registration) {
		registration = GMT_GRID_NODE_REG;
	}
	else if (n_columns == nx_node_registration - 1) {
		registration = GMT_GRID_PIXEL_REG;
	}
	else {
		registration = GMT_GRID_DEFAULT_REG;
	}

	wesn[0] = xmin; /* Min/max x and y coordinates */
	wesn[1] = xmax; /* Min/max x and y coordinates */
	wesn[2] = ymin; /* Min/max x and y coordinates */
	wesn[3] = ymax; /* Min/max x and y coordinates */
	inc[0] = dx;    /* x and y increment */
	inc[1] = dy;    /* x and y increment */
	pad = 0;

	/* create structure for the grid */
	if ((G = GMT_Create_Data(API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, wesn, inc, registration, pad, NULL)) == NULL) {
		status = MB_FAILURE;
		*error = MB_ERROR_MEMORY_FAIL;
		return (status);
	}

	/* Get some projection and user info needed for the grid remark field */
	if ((nscan = sscanf(projection, "UTM%d%c", &utmzone, &NorS)) == 2) {
		if (NorS == 'N') {
			projectionid = 32600 + utmzone;
		}
		else if (NorS == 'S') {
			projectionid = 32700 + utmzone;
		}
		else {
			projectionid = 32600 + utmzone;
		}
		modeltype = ModelTypeProjected;
		sprintf(projectionname, "UTM%2.2d%c", utmzone, NorS);
		grid_projection_mode = MB_PROJECTION_PROJECTED;
		sprintf(grid_projection_id, "epsg%d", projectionid);
	}
	else if ((nscan = sscanf(projection, "epsg%d", &projectionid)) == 1) {
		sprintf(projectionname, "epsg%d", projectionid);
		modeltype = ModelTypeProjected;
		grid_projection_mode = MB_PROJECTION_PROJECTED;
		sprintf(grid_projection_id, "epsg%d", projectionid);
	}
	else {
		strcpy(projectionname, "Geographic WGS84");
		modeltype = ModelTypeGeographic;
		projectionid = GCS_WGS_84;
		grid_projection_mode = MB_PROJECTION_GEOGRAPHIC;
		sprintf(grid_projection_id, "epsg%d", projectionid);
	}
	if (argc > 0)
		strncpy(program_name, argv[0], MB_PATH_MAXLINE);
	else
		strcpy(program_name, "\0");
	right_now = time((time_t *)0);
	strcpy(date, ctime(&right_now));
	date[strlen(date) - 1] = '\0';
	if ((user_ptr = getenv("USER")) == NULL)
		if ((user_ptr = getenv("LOGNAME")) == NULL)
			user_ptr = getenv("USERNAME");
	if (user_ptr != NULL)
		strcpy(user, user_ptr);
	else
		strcpy(user, "unknown");
	gethostname(host, MB_PATH_MAXLINE);
	if (host[0] == '\0') /* Don't know why but the above fails on Win. So get the same info from ENV */
	{
		host_ptr = getenv("USERDOMAIN");
		if (host_ptr != NULL)
			strcpy(host, host_ptr);
	}
	sprintf(remark, "\n\tProjection: %s\n\tGrid created by %s\n\tMB-system Version %s\n\tRun by <%s> on <%s> at <%s>", projection,
	        program_name, MB_VERSION, user, host, date);

	/* set grid labels and remark */
	header = G->header;
	strcpy(header->command, program_name); /* name of generating command */
	strcpy(header->x_units, xlab);         /* units in x-direction */
	strcpy(header->y_units, ylab);         /* units in y-direction */
	strcpy(header->z_units, zlab);         /* grid value units */
	strcpy(header->title, titl);           /* name of data set */
	strncpy(header->remark, remark, GMT_GRID_REMARK_LEN160);

	/* recopy grid data, reordering from internal convention to grd file convention */
	if (status == MB_SUCCESS) {
		MB_MAKE_FNAN(NaN);
		for (i = 0; i < n_columns; i++)
			for (j = 0; j < n_rows; j++) {
				k = i * n_rows + j;
				kk = (n_rows - 1 - j) * n_columns + i;
				if (grid[k] == nodatavalue)
					G->data[kk] = NaN;
				else {
					G->data[kk] = grid[k];
					if (first == MB_YES) {
						min = grid[k];
						max = grid[k];
						first = MB_NO;
					}
					else {
						min = MIN(min, grid[k]);
						max = MAX(max, grid[k]);
					}
				}
			}
	}

/* create null grid mode flags if valid flags don't exist - Paul Wessel
 * indicated in December 2016 that these will appear with GMT 5.3.2
 * and fix the problem of generating geographic grids that can be
 * directly imported to ESRI ArcGIS */
#ifndef GMT_GRID_IS_GEO
#define GMT_GRID_IS_GEO 0
#endif
#ifndef GMT_GRID_IS_CARTESIAN
#define GMT_GRID_IS_CARTESIAN 0
#endif

	/* set GMT grid mode flag */
	if (modeltype == ModelTypeGeographic)
		mode = GMT_GRID_ALL | GMT_GRID_IS_GEO;
	else
		mode = GMT_GRID_ALL | GMT_GRID_IS_CARTESIAN;

	/* print info */
	if (verbose > 0) {
		fprintf(stderr, "\nGrid to be written:\n");
		fprintf(stderr, "  Dimensions:     %d %d\n", header->n_columns, header->n_rows);
		fprintf(stderr, "  Registration:   %d\n", header->registration);
		if (modeltype == ModelTypeProjected) {
			fprintf(stderr, "  Projected Coordinate System Name: %s\n", projectionname);
			fprintf(stderr, "  Projected Coordinate System ID:   %d\n", projectionid);
			fprintf(stderr, "  Easting:    %f %f  %f\n", header->wesn[0], header->wesn[1], header->inc[0]);
			fprintf(stderr, "  Northing:   %f %f  %f\n", header->wesn[2], header->wesn[3], header->inc[1]);
		}
		else {
			fprintf(stderr, "  Geographic Coordinate System Name: %s\n", projectionname);
			fprintf(stderr, "  Geographic Coordinate System ID:   %d\n", projectionid);
			fprintf(stderr, "  Longitude:  %f %f  %f\n", header->wesn[0], header->wesn[1], header->inc[0]);
			fprintf(stderr, "  Latitude:   %f %f  %f\n", header->wesn[2], header->wesn[3], header->inc[1]);
		}
		fprintf(stderr, "  Grid Projection Mode:     %d\n", grid_projection_mode);
		fprintf(stderr, "  Grid Projection ID:       %s\n", grid_projection_id);
		fprintf(stderr, "  Data Extrema:             %f %f\n", header->z_min, header->z_max);
		fprintf(stderr, "  Other Grid Parameters:\n");
		fprintf(stderr, "    z_scale_factor:         %f\n", header->z_scale_factor);
		fprintf(stderr, "    z_add_offset:           %f\n", header->z_add_offset);
		fprintf(stderr, "    type:                   %d\n", header->type);
		fprintf(stderr, "    bits:                   %d\n", header->bits);
		fprintf(stderr, "    complex_mode:           %d\n", header->complex_mode);
		fprintf(stderr, "    mx:                     %d\n", header->mx);
		fprintf(stderr, "    my:                     %d\n", header->my);
		fprintf(stderr, "    nm:                     %zu\n", header->nm);
		fprintf(stderr, "    size:                   %zu\n", header->size);
		fprintf(stderr, "    pad:                    %d %d %d %d\n", header->pad[0], header->pad[1], header->pad[2],
		        header->pad[3]);
		fprintf(stderr, "    data ptr:               %p\n", G->data);
	}

	/* write out the grid */
	if (GMT_Write_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, mode, NULL, grdfile, G) != 0) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		fprintf(stderr, "Unable to write GMT grid file %s with GMT_Write_Data() in function %s\n", grdfile, function_name);
	}

	/* Destroy GMT session */
	if (GMT_Destroy_Session(API) != 0) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		fprintf(stderr, "\nUnable to destroy a GMT session with GMT_Write_Data() in function %s\n", function_name);
		fprintf(stderr, "Unable to write GMT grid file %s\n",grdfile);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
