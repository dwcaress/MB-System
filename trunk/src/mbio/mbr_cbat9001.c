/*--------------------------------------------------------------------
 *    The MB-system:	mbr_cbat9001.c	8/8/94
 *	$Id: mbr_cbat9001.c,v 4.1 1995-03-06 19:38:54 caress Exp $
 *
 *    Copyright (c) 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbr_cbat9001.c contains the functions for reading and writing
 * multibeam data in the CBAT9001 format.  
 * These functions include:
 *   mbr_alm_cbat9001	- allocate read/write memory
 *   mbr_dem_cbat9001	- deallocate read/write memory
 *   mbr_rt_cbat9001	- read and translate data
 *   mbr_wt_cbat9001	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	August 8, 1994
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1994/10/21  12:34:54  caress
 * Release V4.0
 *
 * Revision 1.1  1994/10/21  12:20:01  caress
 * Initial revision
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mbsys_reson.h"
#include "../../include/mbf_cbat9001.h"

/* degrees to radians define */
#ifndef M_PI
#define	M_PI	3.14159265358979323846
#endif
#define DTR	(M_PI/180.)

/* include for byte swapping on little-endian machines */
#include "../../include/mb_swap.h"

/*--------------------------------------------------------------------*/
int mbr_alm_cbat9001(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	static char res_id[]="$Id: mbr_cbat9001.c,v 4.1 1995-03-06 19:38:54 caress Exp $";
	char	*function_name = "mbr_alm_cbat9001";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_cbat9001_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_reson_struct),
				&mb_io_ptr->store_data,error);

	/* initialize everything to zeros */
	mbr_zero_cbat9001(verbose,mb_io_ptr->raw_data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_dem_cbat9001(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_cbat9001";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mb_free(verbose,&mb_io_ptr->store_data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_zero_cbat9001(verbose,data_ptr,error)
int	verbose;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_zero_cbat9001";
	int	status = MB_SUCCESS;
	struct mbf_cbat9001_struct *data;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to data descriptor */
	data = (struct mbf_cbat9001_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		data->kind = MB_DATA_NONE;
		data->sonar = MBSYS_RESON_SEABAT9001;
		data->par_year = 0;
		data->par_month = 0;
		data->par_day = 0;
		data->par_hour = 0;
		data->par_minute = 0;
		data->par_second = 0;
		data->par_hundredth_sec = 0;
		data->par_thousandth_sec = 0;
		data->roll_offset = 0;	/* roll offset (degrees) */
		data->pitch_offset = 0;	/* pitch offset (degrees) */
		data->heading_offset = 0;	/* heading offset (degrees) */
		data->time_delay = 0;		/* positioning system 
							delay (sec) */
		data->transducer_depth = 0;	/* tranducer depth (meters) */
		data->transducer_height = 0;	/* reference height (meters) */
		data->transducer_x = 0;	/* reference athwartships 
							offset (meters) */
		data->transducer_y = 0;	/* reference  fore-aft
							offset (meters) */
		data->antenna_x = 0;		/* antenna athwartships 
							offset (meters) */
		data->antenna_y = 0;		/* antenna fore-aft 
							offset (meters) */
		data->antenna_z = 0;		/* antenna height (meters) */
		data->motion_sensor_x = 0;	/* motion sensor athwartships
							offset (meters) */
		data->motion_sensor_y = 0;	/* motion sensor fore-aft
							offset (meters) */
		data->motion_sensor_z = 0;	/* motion sensor height 
							offset (meters) */
		data->spare = 0;
		data->line_number = 0;
		data->start_or_stop = 0;
		data->transducer_serial_number = 0;
		for (i=0;i<MBF_CBAT9001_COMMENT_LENGTH;i++)
			data->comment[i] = '\0';

		/* position (position telegrams) */
		data->pos_year = 0;
		data->pos_month = 0;
		data->pos_day = 0;
		data->pos_hour = 0;
		data->pos_minute = 0;
		data->pos_second = 0;
		data->par_hundredth_sec = 0;
		data->pos_thousandth_sec = 0;
		data->pos_latitude = 0;
		data->pos_longitude = 0;
		data->utm_northing = 0;
		data->utm_easting = 0;
		data->utm_zone_lon = 0;
		data->utm_zone = 0;
		data->hemisphere = 0;
		data->ellipsoid = 0;
		data->pos_spare = 0;
		data->semi_major_axis = 0;
		data->other_quality = 0;

		/* sound velocity profile */
		data->svp_year = 0;
		data->svp_month = 0;
		data->svp_day = 0;
		data->svp_hour = 0;
		data->svp_minute = 0;
		data->svp_second = 0;
		data->svp_hundredth_sec = 0;
		data->svp_thousandth_sec = 0;
		data->svp_num = 0;
		for (i=0;i<100;i++)
			{
			data->svp_depth[i] = 0; /* 0.1 meters */
			data->svp_vel[i] = 0;	/* 0.1 meters/sec */
			}

		/* time stamp */
		data->year = 0;
		data->month = 0;
		data->day = 0;
		data->hour = 0;
		data->minute = 0;
		data->second = 0;
		data->hundredth_sec = 0;
		data->thousandth_sec = 0;
		data->roll = 0;
		data->pitch = 0;
		data->heading = 0;
		data->heave = 0;
		data->ping_number = 0;
		data->sound_vel = 0;
		data->mode = 0;
		data->gain1 = 0;
		data->gain2 = 0;
		data->gain3 = 0;
		data->beams_bath = MBSYS_RESON_MAXBEAMS;
		for (i=0;i<MBSYS_RESON_MAXBEAMS;i++)
			{
			data->bath[i] = 0;
			data->bath_acrosstrack[i] = 0;
			data->bath_alongtrack[i] = 0;
			data->tt[i] = 0;
			data->angle[i] = 0;
			data->quality[i] = 0;
			data->amp[i] = 0;
			}
		}

	/* assume success */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_cbat9001(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_cbat9001";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_cbat9001_struct *data;
	struct mbsys_reson_struct *store;
	short int *data_ss, *store_ss;
	short int *beam_ss;
	double	mtodeglon, mtodeglat;
	double	dx, dy, dt, speed, dd, headingx, headingy;
	double	depthscale, dacrscale, daloscale, reflscale;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_cbat9001_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_reson_struct *) store_ptr;

	/* reset values in mb_io_ptr */
	mb_io_ptr->new_kind = MB_DATA_NONE;
	mb_io_ptr->new_time_i[0] = 0;
	mb_io_ptr->new_time_i[1] = 0;
	mb_io_ptr->new_time_i[2] = 0;
	mb_io_ptr->new_time_i[3] = 0;
	mb_io_ptr->new_time_i[4] = 0;
	mb_io_ptr->new_time_i[5] = 0;
	mb_io_ptr->new_time_i[6] = 0;
	mb_io_ptr->new_time_d = 0.0;
	mb_io_ptr->new_lon = 0.0;
	mb_io_ptr->new_lat = 0.0;
	mb_io_ptr->new_heading = 0.0;
	mb_io_ptr->new_speed = 0.0;
	for (i=0;i<mb_io_ptr->beams_bath;i++)
		{
		mb_io_ptr->new_bath[i] = 0.0;
		mb_io_ptr->new_bath_acrosstrack[i] = 0.0;
		mb_io_ptr->new_bath_alongtrack[i] = 0.0;
		}
	for (i=0;i<mb_io_ptr->beams_amp;i++)
		{
		mb_io_ptr->new_amp[i] = 0.0;
		}
	for (i=0;i<mb_io_ptr->pixels_ss;i++)
		{
		mb_io_ptr->new_ss[i] = 0.0;
		mb_io_ptr->new_ss_acrosstrack[i] = 0.0;
		mb_io_ptr->new_ss_alongtrack[i] = 0.0;
		}

	/* read next data from file */
	status = mbr_cbat9001_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate time values to current 
		ping variables in mbio descriptor structure */
	if (status == MB_SUCCESS)
		{
		/* get time */
		if (data->kind == MB_DATA_DATA)
			{
			mb_io_ptr->new_time_i[0] = data->year + 1900;
			mb_io_ptr->new_time_i[1] = data->month;
			mb_io_ptr->new_time_i[2] = data->day;
			mb_io_ptr->new_time_i[3] = data->hour;
			mb_io_ptr->new_time_i[4] = data->minute;
			mb_io_ptr->new_time_i[5] = data->second;
			mb_io_ptr->new_time_i[6] = 10000*data->hundredth_sec
				+ 100*data->thousandth_sec;
			}
		else if (data->kind == MB_DATA_PARAMETER)
			{
			mb_io_ptr->new_time_i[0] = data->par_year + 1900;
			mb_io_ptr->new_time_i[1] = data->par_month;
			mb_io_ptr->new_time_i[2] = data->par_day;
			mb_io_ptr->new_time_i[3] = data->par_hour;
			mb_io_ptr->new_time_i[4] = data->par_minute;
			mb_io_ptr->new_time_i[5] = data->par_second;
			mb_io_ptr->new_time_i[6] = 10000*data->par_hundredth_sec
				+ 100*data->par_thousandth_sec;
			}
		else if (data->kind == MB_DATA_VELOCITY_PROFILE)
			{
			mb_io_ptr->new_time_i[0] = data->svp_year + 1900;
			mb_io_ptr->new_time_i[1] = data->svp_month;
			mb_io_ptr->new_time_i[2] = data->svp_day;
			mb_io_ptr->new_time_i[3] = data->svp_hour;
			mb_io_ptr->new_time_i[4] = data->svp_minute;
			mb_io_ptr->new_time_i[5] = data->svp_second;
			mb_io_ptr->new_time_i[6] = 10000*data->svp_hundredth_sec
				+ 100*data->svp_thousandth_sec;
			}
		else if (data->kind == MB_DATA_NAV)
			{
			mb_io_ptr->new_time_i[0] = data->pos_year + 1900;
			mb_io_ptr->new_time_i[1] = data->pos_month;
			mb_io_ptr->new_time_i[2] = data->pos_day;
			mb_io_ptr->new_time_i[3] = data->pos_hour;
			mb_io_ptr->new_time_i[4] = data->pos_minute;
			mb_io_ptr->new_time_i[5] = data->pos_second;
			mb_io_ptr->new_time_i[6] = 10000*data->pos_hundredth_sec
				+ 100*data->pos_thousandth_sec;
			}
		if (mb_io_ptr->new_time_i[0] < 1970)
			mb_io_ptr->new_time_d = 0.0;
		else
			mb_get_time(verbose,mb_io_ptr->new_time_i,
				&mb_io_ptr->new_time_d);

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				mb_io_ptr->new_error);
			fprintf(stderr,"dbg4       kind:       %d\n",
				mb_io_ptr->new_kind);
			fprintf(stderr,"dbg4       time_i[0]:  %d\n",
				mb_io_ptr->new_time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:  %d\n",
				mb_io_ptr->new_time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:  %d\n",
				mb_io_ptr->new_time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:  %d\n",
				mb_io_ptr->new_time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:  %d\n",
				mb_io_ptr->new_time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:  %d\n",
				mb_io_ptr->new_time_i[5]);
			fprintf(stderr,"dbg4       time_i[6]:  %d\n",
				mb_io_ptr->new_time_i[6]);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				mb_io_ptr->new_time_d);
			}
		}

	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_DATA)
		{
		/* get lon and lat if they are there */
		mb_io_ptr->new_lon = data->longitude*0.00000009;
		mb_io_ptr->new_lat = data->latitude*0.00000009;

		/* get heading */
		mb_io_ptr->new_heading = 0.01*data->heading;

		/* get speed  */
		mb_io_ptr->new_speed = 0.0;

		/* extrapolate nav from previous fixes if needed and possible */
		if (data->longitude == 0 
			&& data->latitude == 0
			&& mb_io_ptr->nfix > 1)
			{
			mb_coor_scale(verbose,
				mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
				&mtodeglon,&mtodeglat);
			dx = (mb_io_ptr->fix_lon[mb_io_ptr->nfix-1] 
				- mb_io_ptr->fix_lon[0])/mtodeglon;
			dy = (mb_io_ptr->fix_lat[mb_io_ptr->nfix-1] 
				- mb_io_ptr->fix_lat[0])/mtodeglat;
			dt = mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1] 
				- mb_io_ptr->fix_time_d[0];
			speed = sqrt(dx*dx + dy*dy)/dt; /* m/sec */
			dd = (mb_io_ptr->new_time_d 
				- mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
				*speed; /* meters */
			headingx = sin(DTR*mb_io_ptr->new_heading);
			headingy = cos(DTR*mb_io_ptr->new_heading);
			mb_io_ptr->new_lon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1] 
				+ headingx*mtodeglon*dd;
			mb_io_ptr->new_lat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1] 
				+ headingy*mtodeglat*dd;
			mb_io_ptr->speed = 3.6*speed;
			}

		if (mb_io_ptr->lonflip < 0)
			{
			if (mb_io_ptr->new_lon > 0.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -360.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (mb_io_ptr->new_lon > 180.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -180.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else
			{
			if (mb_io_ptr->new_lon > 360.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < 0.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}

		/* read beam and pixel values into storage arrays */
		mb_io_ptr->beams_bath = data->beams_bath;
		mb_io_ptr->beams_amp = data->beams_bath;
		mb_io_ptr->pixels_ss = 0;
		depthscale = 0.01;
		dacrscale  = 0.01;
		daloscale  = 0.01;
		reflscale  = 1.0;
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			mb_io_ptr->new_bath[i] = depthscale*data->bath[i];
			mb_io_ptr->new_bath_acrosstrack[i] 
				= dacrscale*data->bath_acrosstrack[i];
			mb_io_ptr->new_bath_alongtrack[i] 
				= daloscale*data->bath_alongtrack[i];
			}
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			mb_io_ptr->new_amp[i] = reflscale*data->amp[i];
			}

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"dbg4       longitude:  %f\n",
				mb_io_ptr->new_lon);
			fprintf(stderr,"dbg4       latitude:   %f\n",
				mb_io_ptr->new_lat);
			fprintf(stderr,"dbg4       speed:      %f\n",
				mb_io_ptr->new_speed);
			fprintf(stderr,"dbg4       heading:    %f\n",
				mb_io_ptr->new_heading);
			fprintf(stderr,"dbg4       beams_bath: %d\n",
				mb_io_ptr->beams_bath);
			fprintf(stderr,"dbg4       beams_amp:  %d\n",
				mb_io_ptr->beams_amp);
			for (i=0;i<mb_io_ptr->beams_bath;i++)
			  fprintf(stderr,"dbg4       beam:%d  bath:%f  amp:%f  acrosstrack:%f  alongtrack:%f\n",
				i,mb_io_ptr->new_bath[i],
				mb_io_ptr->new_amp[i],
				mb_io_ptr->new_bath_acrosstrack[i],
				mb_io_ptr->new_bath_alongtrack[i]);
			}
		}

	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_NAV)
		{
		mb_io_ptr->new_lon 
			= data->pos_longitude*0.00000009;
		mb_io_ptr->new_lat 
			= data->pos_latitude*0.00000009;
		if (mb_io_ptr->lonflip < 0)
			{
			if (mb_io_ptr->new_lon > 0.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -360.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (mb_io_ptr->new_lon > 180.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -180.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else
			{
			if (mb_io_ptr->new_lon > 360.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < 0.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}

		/* get heading */
		mb_io_ptr->new_heading = 0.0;

		/* get speed  */
		mb_io_ptr->new_speed = 0.0;

		if (mb_io_ptr->nfix >= 5)
			{
			mb_io_ptr->nfix = 4;
			for (i=0;i<mb_io_ptr->nfix;i++)
				{
				mb_io_ptr->fix_time_d[i] 
					= mb_io_ptr->fix_time_d[i+1];
				mb_io_ptr->fix_lon[i] = mb_io_ptr->fix_lon[i+1];
				mb_io_ptr->fix_lat[i] = mb_io_ptr->fix_lat[i+1];
				}
			}
		mb_io_ptr->fix_time_d[mb_io_ptr->nfix] = mb_io_ptr->new_time_d;
		mb_io_ptr->fix_lon[mb_io_ptr->nfix] = mb_io_ptr->new_lon;
		mb_io_ptr->fix_lat[mb_io_ptr->nfix] = mb_io_ptr->new_lat;
		mb_io_ptr->nfix++;
		}

	/* copy comment to mbio descriptor structure */
	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(mb_io_ptr->new_comment,data->comment,
			MBF_CBAT9001_COMMENT_LENGTH-1);

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				mb_io_ptr->new_error);
			fprintf(stderr,"dbg4       comment:    %s\n",
				mb_io_ptr->new_comment);
			}
		}

	/* translate values to reson data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		store->kind = data->kind;
		store->sonar = data->sonar;

		/* parameter telegram */
		store->par_year = data->par_year;
		store->par_month = data->par_month;
		store->par_day = data->par_day;
		store->par_hour = data->par_hour;
		store->par_minute = data->par_minute;
		store->par_second = data->par_second;
		store->par_hundredth_sec = data->par_hundredth_sec;
		store->par_thousandth_sec = data->par_thousandth_sec;
		store->roll_offset = data->roll_offset;
		store->pitch_offset = data->pitch_offset;
		store->heading_offset = data->heading_offset;
		store->time_delay = data->time_delay;
		store->transducer_depth = data->transducer_depth;
		store->transducer_height = data->transducer_height;
		store->transducer_x = data->transducer_x;
		store->transducer_y = data->transducer_y;
		store->antenna_x = data->antenna_x;
		store->antenna_y = data->antenna_y;
		store->antenna_z = data->antenna_z;
		store->motion_sensor_x = data->motion_sensor_x;
		store->motion_sensor_y = data->motion_sensor_y;
		store->motion_sensor_z = data->motion_sensor_z;
		store->spare = data->spare;
		store->line_number = data->line_number;
		store->start_or_stop = data->start_or_stop;
		store->transducer_serial_number 
			= data->transducer_serial_number;
		for (i=0;i<MBSYS_RESON_COMMENT_LENGTH;i++)
			store->comment[i] = data->comment[i];

		/* position (position telegrams) */
		store->pos_year = data->pos_year;
		store->pos_month = data->pos_month;
		store->pos_day = data->pos_day;
		store->pos_hour = data->pos_hour;
		store->pos_minute = data->pos_minute;
		store->pos_second = data->pos_second;
		store->pos_hundredth_sec = data->pos_hundredth_sec;
		store->pos_thousandth_sec = data->pos_thousandth_sec;
		store->pos_latitude = data->pos_latitude;
		store->pos_longitude = data->pos_longitude;
		store->utm_northing = data->utm_northing;
		store->utm_easting = data->utm_easting;
		store->utm_zone_lon = data->utm_zone_lon;
		store->utm_zone = data->utm_zone;
		store->hemisphere = data->hemisphere;
		store->ellipsoid = data->ellipsoid;
		store->pos_spare = data->pos_spare;
		store->semi_major_axis = data->semi_major_axis;
		store->other_quality = data->other_quality;

		/* sound velocity profile */
		store->svp_year = data->svp_year;
		store->svp_month = data->svp_month;
		store->svp_day = data->svp_day;
		store->svp_hour = data->svp_hour;
		store->svp_minute = data->svp_minute;
		store->svp_second = data->svp_second;
		store->svp_hundredth_sec = data->svp_hundredth_sec;
		store->svp_thousandth_sec = data->svp_thousandth_sec;
		store->svp_num = data->svp_num;
		for (i=0;i<500;i++)
			{
			store->svp_depth[i] = data->svp_depth[i];
			store->svp_vel[i] = data->svp_vel[i];
			}

		/* bathymetry */
		store->year = data->year;
		store->month = data->month;
		store->day = data->day;
		store->hour = data->hour;
		store->minute = data->minute;
		store->second = data->second;
		store->hundredth_sec = data->hundredth_sec;
		store->thousandth_sec = data->thousandth_sec;
		store->roll = data->roll;
		store->pitch = data->pitch;
		store->heading = data->heading;
		store->heave = data->heave;
		store->ping_number = data->ping_number;
		store->sound_vel = data->sound_vel;
		store->mode = data->mode;
		store->gain1 = data->gain1;
		store->gain2 = data->gain2;
		store->gain3 = data->gain3;
		store->beams_bath = data->beams_bath;
		for (i=0;i<store->beams_bath;i++)
			{
			store->bath[i] = data->bath[i];
			store->bath_acrosstrack[i] = data->bath_acrosstrack[i];
			store->bath_alongtrack[i] = data->bath_alongtrack[i];
			store->tt[i] = data->tt[i];
			store->angle[i] = data->angle[i];
			store->quality[i] = data->quality[i];
			store->amp[i] = data->amp[i];
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_wt_cbat9001(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_cbat9001";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_cbat9001_struct *data;
	char	*data_ptr;
	struct mbsys_reson_struct *store;
	double	scalefactor;
	int	time_j[5];
	double	depthscale, dacrscale, daloscale, ttscale, reflscale;
	int	iss;
	short int *data_ss;
	short int *store_ss;
	short int *beam_ss;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_cbat9001_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	store = (struct mbsys_reson_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		data->kind = store->kind;
		data->sonar = store->sonar;

		/* parameter telegram */
		data->par_year = store->par_year;
		data->par_month = store->par_month;
		data->par_day = store->par_day;
		data->par_hour = store->par_hour;
		data->par_minute = store->par_minute;
		data->par_second = store->par_second;
		data->par_hundredth_sec = store->par_hundredth_sec;
		data->par_thousandth_sec = store->par_thousandth_sec;
		data->roll_offset = store->roll_offset;
		data->pitch_offset = store->pitch_offset;
		data->heading_offset = store->heading_offset;
		data->time_delay = store->time_delay;
		data->transducer_depth = store->transducer_depth;
		data->transducer_height = store->transducer_height;
		data->transducer_x = store->transducer_x;
		data->transducer_y = store->transducer_y;
		data->antenna_x = store->antenna_x;
		data->antenna_y = store->antenna_y;
		data->antenna_z = store->antenna_z;
		data->motion_sensor_x = store->motion_sensor_x;
		data->motion_sensor_y = store->motion_sensor_y;
		data->motion_sensor_z = store->motion_sensor_z;
		data->spare = store->spare;
		data->line_number = store->line_number;
		data->start_or_stop = store->start_or_stop;
		data->transducer_serial_number 
			= store->transducer_serial_number;
		for (i=0;i<MBF_CBAT9001_COMMENT_LENGTH;i++)
			data->comment[i] = store->comment[i];

		/* position (position telegrams) */
		data->pos_year = store->pos_year;
		data->pos_month = store->pos_month;
		data->pos_day = store->pos_day;
		data->pos_hour = store->pos_hour;
		data->pos_minute = store->pos_minute;
		data->pos_second = store->pos_second;
		data->pos_hundredth_sec = store->pos_hundredth_sec;
		data->pos_thousandth_sec = store->pos_thousandth_sec;
		data->pos_latitude = store->pos_latitude;
		data->pos_longitude = store->pos_longitude;
		data->utm_northing = store->utm_northing;
		data->utm_easting = store->utm_easting;
		data->utm_zone_lon = store->utm_zone_lon;
		data->utm_zone = store->utm_zone;
		data->hemisphere = store->hemisphere;
		data->ellipsoid = store->ellipsoid;
		data->pos_spare = store->pos_spare;
		data->semi_major_axis = store->semi_major_axis;
		data->other_quality = store->other_quality;

		/* sound velocity profile */
		data->svp_year = store->svp_year;
		data->svp_month = store->svp_month;
		data->svp_day = store->svp_day;
		data->svp_hour = store->svp_hour;
		data->svp_minute = store->svp_minute;
		data->svp_second = store->svp_second;
		data->svp_hundredth_sec = store->svp_hundredth_sec;
		data->svp_thousandth_sec = store->svp_thousandth_sec;
		data->svp_num = store->svp_num;
		for (i=0;i<500;i++)
			{
			data->svp_depth[i] = store->svp_depth[i];
			data->svp_vel[i] = store->svp_vel[i];
			}

		/* bathymetry */
		data->year = store->year;
		data->month = store->month;
		data->day = store->day;
		data->hour = store->hour;
		data->minute = store->minute;
		data->second = store->second;
		data->hundredth_sec = store->hundredth_sec;
		data->thousandth_sec = store->thousandth_sec;
		data->roll = store->roll;
		data->pitch = store->pitch;
		data->heading = store->heading;
		data->heave = store->heave;
		data->ping_number = store->ping_number;
		data->sound_vel = store->sound_vel;
		data->mode = store->mode;
		data->gain1 = store->gain1;
		data->gain2 = store->gain2;
		data->gain3 = store->gain3;
		data->beams_bath = store->beams_bath;
		for (i=0;i<data->beams_bath;i++)
			{
			data->bath[i] = store->bath[i];
			data->bath_acrosstrack[i] = store->bath_acrosstrack[i];
			data->bath_alongtrack[i] = store->bath_alongtrack[i];
			data->tt[i] = store->tt[i];
			data->angle[i] = store->angle[i];
			data->quality[i] = store->quality[i];
			data->amp[i] = store->amp[i];
			}
		}

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		data->kind = mb_io_ptr->new_kind;

	/* set times from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		{
		/* get time */
		data->year = mb_io_ptr->new_time_i[0] - 1900;
		data->month = mb_io_ptr->new_time_i[1];
		data->day = mb_io_ptr->new_time_i[2];
		data->hour = mb_io_ptr->new_time_i[3];
		data->minute = mb_io_ptr->new_time_i[4];
		data->second = mb_io_ptr->new_time_i[5];
		data->hundredth_sec = mb_io_ptr->new_time_i[6]/10000;
		data->thousandth_sec = 
			(mb_io_ptr->new_time_i[6] - 10000*data->hundredth_sec)/100;
		}

	/* check for comment to be copied from mb_io_ptr */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		strncpy(data->comment,mb_io_ptr->new_comment,
			MBF_CBAT9001_COMMENT_LENGTH-1);
		}

	/* else check for ping data to be copied from mb_io_ptr */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/*get navigation */
		data->longitude = (int) mb_io_ptr->new_lon/0.00000009;
		data->latitude = (int) mb_io_ptr->new_lat/0.00000009;

		/* get heading */
		data->heading = (int) (mb_io_ptr->new_heading * 100);

		/* insert distance and depth values into storage arrays */
		data->beams_bath = mb_io_ptr->beams_bath;
		data->sonar = MBSYS_RESON_SEABAT9001;
		depthscale = 0.01;
		dacrscale  = 0.01;
		daloscale  = 0.01;
		reflscale  = 1.0;
		if (status == MB_SUCCESS)
			{
			for (i=0;i<mb_io_ptr->beams_bath;i++)
				{
				data->bath[i] = mb_io_ptr->new_bath[i]/depthscale;
				data->bath_acrosstrack[i]
					= mb_io_ptr->new_bath_acrosstrack[i]/dacrscale;
				data->bath_alongtrack[i] 
					= mb_io_ptr->new_bath_alongtrack[i]/daloscale;
				}
			for (i=0;i<mb_io_ptr->beams_bath;i++)
				{
				data->amp[i] = mb_io_ptr->new_amp[i]/reflscale;
				}
			}
		}

	/* write next data to file */
	status = mbr_cbat9001_wr_data(verbose,mbio_ptr,data_ptr,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_cbat9001_rd_data(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_cbat9001_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_cbat9001_struct *data;
	char	*data_ptr;
	FILE	*mbfp;
	int	done;
	short int *type;
	static char label[2];

	static int label_save_flag = MB_NO;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_cbat9001_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	mbfp = mb_io_ptr->mbfp;

	done = MB_NO;
	type = (short int *) label;
	*error = MB_ERROR_NO_ERROR;
	while (done == MB_NO)
		{
		/* get next record label */
		if (label_save_flag == MB_NO)
			{
			if ((status = fread(&label[0],1,1,mb_io_ptr->mbfp)) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			if (label[0] == 0x02)
			if ((status = fread(&label[1],1,1,mb_io_ptr->mbfp)) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			}
		else
			label_save_flag = MB_NO;

		/* swap bytes if necessary */
#ifdef BYTESWAPPED
		*type = (short int) mb_swap_short(*type);
#endif

/*		fprintf(stderr,"\nstart of mbr_cbat9001_rd_data loop:\n");
		fprintf(stderr,"done:%d\n",done);
		fprintf(stderr,"type:%x\n",*type);
		fprintf(stderr,"label_save_flag:%x\n",label_save_flag);*/

		/* read the appropriate data records */
		if (status == MB_FAILURE)
			{
	/*fprintf(stderr,"call nothing, read failure\n");*/
			done = MB_YES;
			}
		else if (*type == RESON_COMMENT)
			{
			status = mbr_cbat9001_rd_comment(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_COMMENT;
				}
			}
		else if (*type == RESON_PARAMETER)
			{
			status = mbr_cbat9001_rd_parameter(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_PARAMETER;
				}
			}
		else if (*type == RESON_POS)
			{
			status = mbr_cbat9001_rd_pos(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_NAV;
				}
			}
		else if (*type == RESON_SVP)
			{
			status = mbr_cbat9001_rd_svp(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_VELOCITY_PROFILE;
				}
			}
		else if (*type == RESON_BATH)
			{
			status = mbr_cbat9001_rd_bath(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_DATA;
				}
			}

		/* bail out if there is an error */
		if (status == MB_FAILURE)
			done == MB_YES;

/*		fprintf(stderr,"end of mbr_cbat9001_rd_data loop:\n");
		fprintf(stderr,"done:%d\n",done);
		fprintf(stderr,"type:%x\n",*type);
		fprintf(stderr,"label_save_flag:%x\n",label_save_flag);*/
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_cbat9001_rd_comment(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_cbat9001_struct *data;
int	*error;
{
	char	*function_name = "mbr_cbat9001_rd_comment";
	int	status = MB_SUCCESS;
	char	line[RESON_COMMENT_SIZE+3];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		}

	/* read record into char array */
	status = fread(line,1,RESON_COMMENT_SIZE+3,mbfp);
	if (status == RESON_COMMENT_SIZE+3)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		data->kind = MB_DATA_COMMENT;
		strncpy(data->comment,line,MBF_CBAT9001_COMMENT_LENGTH-1);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",data->comment);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_cbat9001_rd_parameter(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_cbat9001_struct *data;
int	*error;
{
	char	*function_name = "mbr_cbat9001_rd_parameter";
	int	status = MB_SUCCESS;
	char	line[RESON_PARAMETER_SIZE+3];
	short int *short_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		}

	/* read record into char array */
	status = fread(line,1,RESON_PARAMETER_SIZE+3,mbfp);
	if (status == RESON_PARAMETER_SIZE+3)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		data->kind = MB_DATA_PARAMETER;
		data->par_day =            (int) line[0];
		data->par_month =          (int) line[1];
		data->par_year =           (int) line[2];
		data->par_hour =           (int) line[3];
		data->par_minute =         (int) line[4];
		data->par_second =         (int) line[5];
		data->par_hundredth_sec =  (int) line[6];
		data->par_thousandth_sec = (int) line[7];
#ifndef BYTESWAPPED
		short_ptr = (short int *) &line[8];
		data->roll_offset = *short_ptr;
		short_ptr = (short int *) &line[10];
		data->pitch_offset = *short_ptr;
		short_ptr = (short int *) &line[12];
		data->heading_offset = *short_ptr;
		short_ptr = (short int *) &line[14];
		data->time_delay = *short_ptr;
		short_ptr = (short int *) &line[16];
		data->transducer_depth = *short_ptr;
		short_ptr = (short int *) &line[18];
		data->transducer_height = *short_ptr;
		short_ptr = (short int *) &line[20];
		data->transducer_x = *short_ptr;
		short_ptr = (short int *) &line[22];
		data->transducer_y = *short_ptr;
		short_ptr = (short int *) &line[24];
		data->antenna_z = *short_ptr;
		short_ptr = (short int *) &line[26];
		data->antenna_x = *short_ptr;
		short_ptr = (short int *) &line[28];
		data->antenna_y = *short_ptr;
		short_ptr = (short int *) &line[30];
		data->motion_sensor_x = *short_ptr;
		short_ptr = (short int *) &line[32];
		data->motion_sensor_y = *short_ptr;
		short_ptr = (short int *) &line[34];
		data->motion_sensor_z = *short_ptr;
		short_ptr = (short int *) &line[36];
		data->spare = *short_ptr;
		short_ptr = (short int *) &line[38];
		data->line_number = *short_ptr;
		short_ptr = (short int *) &line[40];
		data->start_or_stop = *short_ptr;
		short_ptr = (short int *) &line[42];
		data->transducer_serial_number = *short_ptr;
#else
		short_ptr = (short int *) &line[8];
		data->roll_offset = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[10];
		data->pitch_offset = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[12];
		data->heading_offset = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[14];
		data->time_delay = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[16];
		data->transducer_depth = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[18];
		data->transducer_height = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[20];
		data->transducer_x = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[22];
		data->transducer_y = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[24];
		data->antenna_z = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[26];
		data->antenna_x = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[28];
		data->antenna_y = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[30];
		data->motion_sensor_x = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[32];
		data->motion_sensor_y = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[34];
		data->motion_sensor_z = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[36];
		data->spare = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[38];
		data->line_number = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[40];
		data->start_or_stop = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[42];
		data->transducer_serial_number = (short int) mb_swap_short(*short_ptr);
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->par_year);
		fprintf(stderr,"dbg5       month:            %d\n",data->par_month);
		fprintf(stderr,"dbg5       day:              %d\n",data->par_day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->par_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->par_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->par_second);
		fprintf(stderr,"dbg5       hundredth_sec:    %d\n",data->par_hundredth_sec);
		fprintf(stderr,"dbg5       thousandth_sec:   %d\n",data->par_thousandth_sec);
		fprintf(stderr,"dbg5       roll_offset:      %d\n",data->roll_offset);
		fprintf(stderr,"dbg5       pitch_offset:     %d\n",data->pitch_offset);
		fprintf(stderr,"dbg5       heading_offset:   %d\n",data->heading_offset);
		fprintf(stderr,"dbg5       time_delay:       %d\n",data->time_delay);
		fprintf(stderr,"dbg5       transducer_depth: %d\n",data->transducer_depth);
		fprintf(stderr,"dbg5       transducer_height:%d\n",data->transducer_height);
		fprintf(stderr,"dbg5       transducer_x:     %d\n",data->transducer_x);
		fprintf(stderr,"dbg5       transducer_y:     %d\n",data->transducer_y);
		fprintf(stderr,"dbg5       antenna_x:        %d\n",data->antenna_x);
		fprintf(stderr,"dbg5       antenna_y:        %d\n",data->antenna_y);
		fprintf(stderr,"dbg5       antenna_z:        %d\n",data->antenna_z);
		fprintf(stderr,"dbg5       motion_sensor_x:  %d\n",data->motion_sensor_x);
		fprintf(stderr,"dbg5       motion_sensor_y:  %d\n",data->motion_sensor_y);
		fprintf(stderr,"dbg5       motion_sensor_z:  %d\n",data->motion_sensor_z);
		fprintf(stderr,"dbg5       spare:            %d\n",data->spare);
		fprintf(stderr,"dbg5       line_number:      %d\n",data->line_number);
		fprintf(stderr,"dbg5       start_or_stop:    %d\n",data->start_or_stop);
		fprintf(stderr,"dbg5       xducer_serial_num:%d\n",
				data->transducer_serial_number);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_cbat9001_rd_pos(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_cbat9001_struct *data;
int	*error;
{
	char	*function_name = "mbr_cbat9001_rd_pos";
	int	status = MB_SUCCESS;
	char	line[RESON_POS_SIZE+3];
	short int *short_ptr;
	int	*int_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		}

	/* read record into char array */
	status = fread(line,1,RESON_POS_SIZE+3,mbfp);
	if (status == RESON_POS_SIZE+3)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		data->kind = MB_DATA_NAV;
		data->pos_day =            (int) line[0];
		data->pos_month =          (int) line[1];
		data->pos_year =           (int) line[2];
		data->pos_hour =           (int) line[3];
		data->pos_minute =         (int) line[4];
		data->pos_second =         (int) line[5];
		data->pos_hundredth_sec =  (int) line[6];
		data->pos_thousandth_sec = (int) line[7];
#ifndef BYTESWAPPED
		int_ptr = (int *) &line[8];
		data->pos_latitude = *int_ptr;
		int_ptr = (int *) &line[12];
		data->pos_longitude = *int_ptr;
		int_ptr = (int *) &line[16];
		data->utm_northing = *int_ptr;
		int_ptr = (int *) &line[20];
		data->utm_easting = *int_ptr;
		int_ptr = (int *) &line[24];
		data->utm_zone_lon = *int_ptr;
		data->utm_zone = line[28];
		data->hemisphere = line[29];
		data->ellipsoid = line[30];
		data->pos_spare = line[31];
		short_ptr = (short int *) &line[32];
		data->semi_major_axis = (int) *short_ptr;
		short_ptr = (short int *) &line[34];
		data->other_quality = (int) *short_ptr;
#else
		int_ptr = (int *) &line[8];
		data->pos_latitude = (int) mb_swap_long(*int_ptr);
		int_ptr = (int *) &line[12];
		data->pos_longitude = (int) mb_swap_long(*int_ptr);
		int_ptr = (int *) &line[16];
		data->utm_northing = (int) mb_swap_long(*int_ptr);
		int_ptr = (int *) &line[20];
		data->utm_easting = (int) mb_swap_long(*int_ptr);
		int_ptr = (int *) &line[24];
		data->utm_zone_lon = (int) mb_swap_long(*int_ptr);
		data->utm_zone = line[28];
		data->hemisphere = line[29];
		data->ellipsoid = line[30];
		data->pos_spare = line[31];
		short_ptr = (short int *) &line[32];
		data->semi_major_axis = (int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[34];
		data->other_quality = (int) mb_swap_short(*short_ptr);
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->pos_year);
		fprintf(stderr,"dbg5       month:            %d\n",data->pos_month);
		fprintf(stderr,"dbg5       day:              %d\n",data->pos_day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->pos_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->pos_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->pos_second);
		fprintf(stderr,"dbg5       hundredth_sec:    %d\n",data->pos_hundredth_sec);
		fprintf(stderr,"dbg5       thousandth_sec:   %d\n",data->pos_thousandth_sec);
		fprintf(stderr,"dbg5       pos_latitude:     %d\n",data->pos_latitude);
		fprintf(stderr,"dbg5       pos_longitude:    %d\n",data->pos_longitude);
		fprintf(stderr,"dbg5       utm_northing:     %d\n",data->utm_northing);
		fprintf(stderr,"dbg5       utm_easting:      %d\n",data->utm_easting);
		fprintf(stderr,"dbg5       utm_zone_lon:     %d\n",data->utm_zone_lon);
		fprintf(stderr,"dbg5       utm_zone:         %c\n",data->utm_zone);
		fprintf(stderr,"dbg5       hemisphere:       %c\n",data->hemisphere);
		fprintf(stderr,"dbg5       ellipsoid:        %c\n",data->ellipsoid);
		fprintf(stderr,"dbg5       pos_spare:        %c\n",data->pos_spare);
		fprintf(stderr,"dbg5       semi_major_axis:  %d\n",data->semi_major_axis);
		fprintf(stderr,"dbg5       other_quality:    %d\n",data->other_quality);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_cbat9001_rd_svp(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_cbat9001_struct *data;
int	*error;
{
	char	*function_name = "mbr_cbat9001_rd_svp";
	int	status = MB_SUCCESS;
	char	line[RESON_SVP_SIZE+3];
	short int *short_ptr;
	short int *short_ptr2;
	int	*int_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		}

	/* read record into char array */
	status = fread(line,1,RESON_SVP_SIZE+3,mbfp);
	if (status == RESON_SVP_SIZE+3)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		data->kind = MB_DATA_VELOCITY_PROFILE;
		data->svp_day =            (int) line[0];
		data->svp_month =          (int) line[1];
		data->svp_year =           (int) line[2];
		data->svp_hour =           (int) line[3];
		data->svp_minute =         (int) line[4];
		data->svp_second =         (int) line[5];
		data->svp_hundredth_sec =  (int) line[6];
		data->svp_thousandth_sec = (int) line[7];
#ifndef BYTESWAPPED
		int_ptr = (int *) &line[8];
		data->svp_latitude = *int_ptr;
		int_ptr = (int *) &line[12];
		data->svp_longitude = *int_ptr;
#else
		int_ptr = (int *) &line[8];
		data->svp_latitude = (int) mb_swap_long(*int_ptr);
		int_ptr = (int *) &line[12];
		data->svp_latitude = (int) mb_swap_long(*int_ptr);
#endif
		data->svp_num = 0;
		for (i=0;i<500;i++)
			{
			short_ptr = (short int *) &line[16+4*i];
			short_ptr2 = (short int *) &line[18+4*i];
#ifndef BYTESWAPPED
			data->svp_depth[i] = *short_ptr;
			data->svp_vel[i] = *short_ptr2;
#else
			data->svp_depth[i] = (short int) mb_swap_short(*short_ptr);
			data->svp_vel[i] = (short int) mb_swap_short(*short_ptr2);
#endif
			if (data->svp_vel[i] > 0) data->svp_num = i + 1;
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->svp_year);
		fprintf(stderr,"dbg5       month:            %d\n",data->svp_month);
		fprintf(stderr,"dbg5       day:              %d\n",data->svp_day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->svp_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->svp_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->svp_second);
		fprintf(stderr,"dbg5       hundredth_sec:    %d\n",data->svp_hundredth_sec);
		fprintf(stderr,"dbg5       thousandth_sec:   %d\n",data->svp_thousandth_sec);
		fprintf(stderr,"dbg5       svp_latitude:     %d\n",data->svp_latitude);
		fprintf(stderr,"dbg5       svp_longitude:    %d\n",data->svp_longitude);
		fprintf(stderr,"dbg5       svp_num:          %d\n",data->svp_num);
		for (i=0;i<data->svp_num;i++)
			fprintf(stderr,"dbg5       depth: %f     vel: %f\n",
				data->svp_depth[i],data->svp_vel[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_cbat9001_rd_bath(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_cbat9001_struct *data;
int	*error;
{
	char	*function_name = "mbr_cbat9001_rd_bath";
	int	status = MB_SUCCESS;
	char	line[RESON_BATH_SIZE+3];
	char	*beamarray;
	unsigned char *char_ptr;
	short int *short_ptr;
	int	*int_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		}

	/* read record into char array */
	status = fread(line,1,RESON_BATH_SIZE+3,mbfp);
	if (status == RESON_BATH_SIZE+3)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		data->kind = MB_DATA_DATA;
		data->day =            (int) line[0];
		data->month =          (int) line[1];
		data->year =           (int) line[2];
		data->hour =           (int) line[3];
		data->minute =         (int) line[4];
		data->second =         (int) line[5];
		data->hundredth_sec =  (int) line[6];
		data->thousandth_sec = (int) line[7];
#ifndef BYTESWAPPED
		int_ptr = (int *) &line[8];
		data->latitude = *int_ptr;
		int_ptr = (int *) &line[12];
		data->longitude = *int_ptr;
		short_ptr = (short int *) &line[16];
		data->roll = (int) *short_ptr;
		short_ptr = (short int *) &line[18];
		data->pitch = (int) *short_ptr;
		short_ptr = (short int *) &line[20];
		data->heading = (int) *short_ptr;
		short_ptr = (short int *) &line[22];
		data->heave = (int) *short_ptr;
		short_ptr = (short int *) &line[24];
		data->ping_number = (int) *short_ptr;
		short_ptr = (short int *) &line[26];
		data->sound_vel = (int) *short_ptr;
#else
		int_ptr = (int *) &line[8];
		data->latitude = (int) mb_swap_long(*int_ptr);
		int_ptr = (int *) &line[12];
		data->latitude = (int) mb_swap_long(*int_ptr);
		short_ptr = (short int *) &line[16];
		data->roll = (int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[18];
		data->pitch = (int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[20];
		data->heading = (int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[22];
		data->heave = (int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[24];
		data->ping_number = (int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[26];
		data->sound_vel = (int) mb_swap_short(*short_ptr);
#endif
		data->mode = (int) line[28];
		data->gain1 = (int) line[29];
		data->gain2 = (int) line[30];
		data->gain3 = (int) line[31];
		data->beams_bath = MBF_CBAT9001_MAXBEAMS;
#ifndef BYTESWAPPED
		for (i=0;i<data->beams_bath;i++)
			{
			beamarray = line + 32 + 12*i;
			short_ptr = (short int *) beamarray; 
			data->bath[i] = *short_ptr;
			short_ptr = ((short int *) beamarray) + 1; 
			data->bath_acrosstrack[i] = *short_ptr;
			short_ptr = ((short int *) beamarray) + 2; 
			data->bath_alongtrack[i] = *short_ptr;
			short_ptr = ((short int *) beamarray) + 3; 
			data->tt[i] = *short_ptr;
			short_ptr = ((short int *) beamarray) + 4; 
			data->angle[i] = *short_ptr;
			char_ptr = (unsigned char *) (beamarray + 10); 
			data->quality[i] = (short int) *char_ptr;
			char_ptr = (unsigned char *) (beamarray + 11); 
			data->amp[i] = (short int) *char_ptr;
			}
#else
		for (i=0;i<data->beams_bath;i++)
			{
			beamarray = line + 32 + 12*i;
			short_ptr = (short int *) beamarray; 
			data->bath[i] = 
				(short int) mb_swap_short(*short_ptr);
			short_ptr = ((short int *) beamarray) + 1; 
			data->bath_acrosstrack[i] = 
				(short int) mb_swap_short(*short_ptr);
			short_ptr = ((short int *) beamarray) + 2; 
			data->bath_alongtrack[i] = 
				(short int) mb_swap_short(*short_ptr);
			short_ptr = ((short int *) beamarray) + 3; 
			data->tt[i] = (short int) mb_swap_short(*short_ptr);
			short_ptr = ((short int *) beamarray) + 4; 
			data->angle[i] = (short int) mb_swap_short(*short_ptr);
			char_ptr = beamarray + 10; 
			data->quality[i] = (short int) *char_ptr;
			char_ptr = beamarray + 11; 
			data->amp[i] = (short int) *char_ptr;
			}
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->second);
		fprintf(stderr,"dbg5       hundredth_sec:    %d\n",data->hundredth_sec);
		fprintf(stderr,"dbg5       thousandth_sec:   %d\n",data->thousandth_sec);
		fprintf(stderr,"dbg5       latitude:         %d\n",data->latitude);
		fprintf(stderr,"dbg5       longitude:        %d\n",data->longitude);
		fprintf(stderr,"dbg5       roll:             %d\n",data->roll);
		fprintf(stderr,"dbg5       pitch:            %d\n",data->pitch);
		fprintf(stderr,"dbg5       heading:          %d\n",data->heading);
		fprintf(stderr,"dbg5       heave:            %d\n",data->heave);
		fprintf(stderr,"dbg5       ping_number:      %d\n",data->ping_number);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       mode:             %d\n",data->mode);
		fprintf(stderr,"dbg5       gain1:            %d\n",data->gain1);
		fprintf(stderr,"dbg5       gain2:            %d\n",data->gain2);
		fprintf(stderr,"dbg5       gain3:            %d\n",data->gain3);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",data->beams_bath);
		fprintf(stderr,"dbg5       beam bath xtrack ltrack tt amp qual heave\n");
		for (i=0;i<data->beams_bath;i++)
			fprintf(stderr,"dbg5       beam:%d  bath:%d  xtrck:%d  ltrck:%d tt:%d  angle:%d amp:%d  qual:%d\n",
				i,data->bath[i],data->bath_acrosstrack[i],
				data->bath_alongtrack[i],data->tt[i],
				data->angle[i],data->amp[i],data->quality[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_cbat9001_wr_data(verbose,mbio_ptr,data_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_cbat9001_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_cbat9001_struct *data;
	FILE	*mbfp;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_cbat9001_struct *) data_ptr;
	mbfp = mb_io_ptr->mbfp;

	if (data->kind == MB_DATA_COMMENT)
		{
		status = mbr_cbat9001_wr_comment(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_PARAMETER)
		{
		status = mbr_cbat9001_wr_parameter(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_NAV)
		{
		status = mbr_cbat9001_wr_pos(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_VELOCITY_PROFILE)
		{
		status = mbr_cbat9001_wr_svp(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_DATA)
		{
		status = mbr_cbat9001_wr_bath(verbose,mbfp,data,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
		}

	/* print output debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data record kind in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",data->kind);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_cbat9001_wr_comment(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_cbat9001_wr_comment";
	int	status = MB_SUCCESS;
	struct mbf_cbat9001_struct *data;
	char	line[RESON_COMMENT_SIZE+3];
	short int label;
	int	len;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_cbat9001_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",data->comment);
		}

	/* write the record label */
	label = RESON_COMMENT;
#ifdef BYTESWAPPED
	label = (short) mb_swap_short(label);
#endif
	status = fwrite(&label,1,2,mbfp);
	if (status != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		len = strlen(data->comment);
		if (len > MBF_CBAT9001_COMMENT_LENGTH)
			len = MBF_CBAT9001_COMMENT_LENGTH;
		for (i=0;i<len;i++)
			line[i] = data->comment[i];
		for (i=len;i<MBF_CBAT9001_COMMENT_LENGTH;i++)
			line[i] = '\0';
		line[RESON_COMMENT_SIZE] = 0x03;
		line[RESON_COMMENT_SIZE+1] = '\0';
		line[RESON_COMMENT_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,RESON_COMMENT_SIZE+3,mbfp); 
		if (status != RESON_COMMENT_SIZE+3)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_cbat9001_wr_parameter(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_cbat9001_wr_parameter";
	int	status = MB_SUCCESS;
	struct mbf_cbat9001_struct *data;
	char	line[RESON_PARAMETER_SIZE+3];
	short int label;
	short int *short_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_cbat9001_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->par_year);
		fprintf(stderr,"dbg5       month:            %d\n",data->par_month);
		fprintf(stderr,"dbg5       day:              %d\n",data->par_day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->par_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->par_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->par_second);
		fprintf(stderr,"dbg5       hundredth_sec:    %d\n",data->par_hundredth_sec);
		fprintf(stderr,"dbg5       thousandth_sec:   %d\n",data->par_thousandth_sec);
		fprintf(stderr,"dbg5       roll_offset:      %d\n",data->roll_offset);
		fprintf(stderr,"dbg5       pitch_offset:     %d\n",data->pitch_offset);
		fprintf(stderr,"dbg5       heading_offset:   %d\n",data->heading_offset);
		fprintf(stderr,"dbg5       time_delay:       %d\n",data->time_delay);
		fprintf(stderr,"dbg5       transducer_depth: %d\n",data->transducer_depth);
		fprintf(stderr,"dbg5       transducer_height:%d\n",data->transducer_height);
		fprintf(stderr,"dbg5       transducer_x:     %d\n",data->transducer_x);
		fprintf(stderr,"dbg5       transducer_y:     %d\n",data->transducer_y);
		fprintf(stderr,"dbg5       antenna_x:        %d\n",data->antenna_x);
		fprintf(stderr,"dbg5       antenna_y:        %d\n",data->antenna_y);
		fprintf(stderr,"dbg5       antenna_z:        %d\n",data->antenna_z);
		fprintf(stderr,"dbg5       motion_sensor_x:  %d\n",data->motion_sensor_x);
		fprintf(stderr,"dbg5       motion_sensor_y:  %d\n",data->motion_sensor_y);
		fprintf(stderr,"dbg5       motion_sensor_z:  %d\n",data->motion_sensor_z);
		fprintf(stderr,"dbg5       spare:            %d\n",data->spare);
		fprintf(stderr,"dbg5       line_number:      %d\n",data->line_number);
		fprintf(stderr,"dbg5       start_or_stop:    %d\n",data->start_or_stop);
		fprintf(stderr,"dbg5       xducer_serial_num:%d\n",
				data->transducer_serial_number);
		}

	/* write the record label */
	label = RESON_PARAMETER;
#ifdef BYTESWAPPED
	label = (short) mb_swap_short(label);
#endif
	status = fwrite(&label,1,2,mbfp);
	if (status != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		line[0] = (char) data->par_day;
		line[1] = (char) data->par_month;
		line[2] = (char) data->par_year;
		line[3] = (char) data->par_hour;
		line[4] = (char) data->par_minute;
		line[5] = (char) data->par_second;
		line[6] = (char) data->par_hundredth_sec;
		line[7] = (char) data->par_thousandth_sec;
#ifndef BYTESWAPPED
		short_ptr = (short int *) &line[8];
		*short_ptr = data->roll_offset;
		short_ptr = (short int *) &line[10];
		*short_ptr = data->pitch_offset;
		short_ptr = (short int *) &line[12];
		*short_ptr = data->heading_offset;
		short_ptr = (short int *) &line[14];
		*short_ptr = data->time_delay;
		short_ptr = (short int *) &line[16];
		*short_ptr = data->transducer_depth;
		short_ptr = (short int *) &line[18];
		*short_ptr = data->transducer_height;
		short_ptr = (short int *) &line[20];
		*short_ptr = data->transducer_x;
		short_ptr = (short int *) &line[22];
		*short_ptr = data->transducer_y;
		short_ptr = (short int *) &line[24];
		*short_ptr = data->antenna_z;
		short_ptr = (short int *) &line[26];
		*short_ptr = data->antenna_x;
		short_ptr = (short int *) &line[28];
		*short_ptr = data->antenna_y;
		short_ptr = (short int *) &line[30];
		*short_ptr = data->motion_sensor_x;
		short_ptr = (short int *) &line[32];
		*short_ptr = data->motion_sensor_y;
		short_ptr = (short int *) &line[34];
		*short_ptr = data->motion_sensor_z;
		short_ptr = (short int *) &line[36];
		*short_ptr = data->spare;
		short_ptr = (short int *) &line[38];
		*short_ptr = data->line_number;
		short_ptr = (short int *) &line[40];
		*short_ptr = data->start_or_stop;
		short_ptr = (short int *) &line[42];
		*short_ptr = data->transducer_serial_number;
#else
		short_ptr = (short int *) &line[8];
		*short_ptr = (short int) mb_swap_short(data->roll_offset);
		short_ptr = (short int *) &line[10];
		*short_ptr = (short int) mb_swap_short(data->pitch_offset);
		short_ptr = (short int *) &line[12];
		*short_ptr = (short int) mb_swap_short(data->heading_offset);
		short_ptr = (short int *) &line[14];
		*short_ptr = (short int) mb_swap_short(data->time_delay);
		short_ptr = (short int *) &line[16];
		*short_ptr = (short int) mb_swap_short(data->transducer_depth);
		short_ptr = (short int *) &line[18];
		*short_ptr = (short int) mb_swap_short(data->transducer_height);
		short_ptr = (short int *) &line[20];
		*short_ptr = (short int) mb_swap_short(data->transducer_x);
		short_ptr = (short int *) &line[22];
		*short_ptr = (short int) mb_swap_short(data->transducer_y);
		short_ptr = (short int *) &line[24];
		*short_ptr = (short int) mb_swap_short(data->antenna_z);
		short_ptr = (short int *) &line[26];
		*short_ptr = (short int) mb_swap_short(data->antenna_x);
		short_ptr = (short int *) &line[28];
		*short_ptr = (short int) mb_swap_short(data->antenna_y);
		short_ptr = (short int *) &line[30];
		*short_ptr = (short int) mb_swap_short(data->motion_sensor_x);
		short_ptr = (short int *) &line[32];
		*short_ptr = (short int) mb_swap_short(data->motion_sensor_y);
		short_ptr = (short int *) &line[34];
		*short_ptr = (short int) mb_swap_short(data->motion_sensor_z);
		short_ptr = (short int *) &line[36];
		*short_ptr = (short int) mb_swap_short(data->spare);
		short_ptr = (short int *) &line[38];
		*short_ptr = (short int) mb_swap_short(data->line_number);
		short_ptr = (short int *) &line[40];
		*short_ptr = (short int) mb_swap_short(data->start_or_stop);
		short_ptr = (short int *) &line[42];
		*short_ptr = (short int) mb_swap_short(data->transducer_serial_number);
#endif
		line[RESON_PARAMETER_SIZE] = 0x03;
		line[RESON_PARAMETER_SIZE+1] = '\0';
		line[RESON_PARAMETER_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,RESON_PARAMETER_SIZE+3,mbfp);
		if (status != RESON_PARAMETER_SIZE+3)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_cbat9001_wr_pos(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_cbat9001_wr_pos";
	int	status = MB_SUCCESS;
	struct mbf_cbat9001_struct *data;
	char	line[RESON_POS_SIZE+3];
	short int label;
	short int *short_ptr;
	int	*int_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_cbat9001_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->pos_year);
		fprintf(stderr,"dbg5       month:            %d\n",data->pos_month);
		fprintf(stderr,"dbg5       day:              %d\n",data->pos_day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->pos_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->pos_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->pos_second);
		fprintf(stderr,"dbg5       hundredth_sec:    %d\n",data->pos_hundredth_sec);
		fprintf(stderr,"dbg5       thousandth_sec:   %d\n",data->pos_thousandth_sec);
		fprintf(stderr,"dbg5       pos_latitude:     %d\n",data->pos_latitude);
		fprintf(stderr,"dbg5       pos_longitude:    %d\n",data->pos_longitude);
		fprintf(stderr,"dbg5       utm_northing:     %d\n",data->utm_northing);
		fprintf(stderr,"dbg5       utm_easting:      %d\n",data->utm_easting);
		fprintf(stderr,"dbg5       utm_zone_lon:     %d\n",data->utm_zone_lon);
		fprintf(stderr,"dbg5       utm_zone:         %c\n",data->utm_zone);
		fprintf(stderr,"dbg5       hemisphere:       %c\n",data->hemisphere);
		fprintf(stderr,"dbg5       ellipsoid:        %c\n",data->ellipsoid);
		fprintf(stderr,"dbg5       pos_spare:        %c\n",data->pos_spare);
		fprintf(stderr,"dbg5       semi_major_axis:  %d\n",data->semi_major_axis);
		fprintf(stderr,"dbg5       other_quality:    %d\n",data->other_quality);
		}

	/* write the record label */
	label = RESON_POS;
#ifdef BYTESWAPPED
	label = (short) mb_swap_short(label);
#endif
	status = fwrite(&label,1,2,mbfp);
	if (status != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		line[0] = (char) data->pos_day;
		line[1] = (char) data->pos_month;
		line[2] = (char) data->pos_year;
		line[3] = (char) data->pos_hour;
		line[4] = (char) data->pos_minute;
		line[5] = (char) data->pos_second;
		line[6] = (char) data->pos_hundredth_sec;
		line[7] = (char) data->pos_thousandth_sec;
#ifndef BYTESWAPPED
		int_ptr = (int *) &line[8];
		*int_ptr = data->pos_latitude;
		int_ptr = (int *) &line[12];
		*int_ptr = data->pos_longitude;
		int_ptr = (int *) &line[16];
		*int_ptr = data->utm_northing;
		int_ptr = (int *) &line[20];
		*int_ptr = data->utm_easting;
		int_ptr = (int *) &line[24];
		*int_ptr = data->utm_zone_lon;
		line[28] = data->utm_zone;
		line[29] = data->hemisphere;
		line[30] = data->ellipsoid;
		line[31] = data->pos_spare;
		short_ptr = (short int *) &line[32];
		*short_ptr = (short int) data->semi_major_axis;
		short_ptr = (short int *) &line[34];
		*short_ptr = (short int) data->other_quality;
#else
		int_ptr = (int *) &line[8];
		*int_ptr = (int) mb_swap_long(data->pos_latitude);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_long(data->pos_longitude);
		int_ptr = (int *) &line[16];
		*int_ptr = (int) mb_swap_long(data->utm_northing);
		int_ptr = (int *) &line[20];
		*int_ptr = (int) mb_swap_long(data->utm_easting);
		int_ptr = (int *) &line[24];
		*int_ptr = (int) mb_swap_long(data->utm_zone_lon);
		line[28] = data->utm_zone;
		line[29] = data->hemisphere;
		line[30] = data->ellipsoid;
		line[31] = data->pos_spare;
		short_ptr = (short int *) &line[32];
		*short_ptr = (int) mb_swap_short(data->semi_major_axis);
		short_ptr = (short int *) &line[34];
		*short_ptr = (int) mb_swap_short(data->other_quality);
#endif
		line[RESON_POS_SIZE] = 0x03;
		line[RESON_POS_SIZE+1] = '\0';
		line[RESON_POS_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,RESON_POS_SIZE+3,mbfp);
		if (status != RESON_POS_SIZE+3)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_cbat9001_wr_svp(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_cbat9001_wr_svp";
	int	status = MB_SUCCESS;
	struct mbf_cbat9001_struct *data;
	char	line[RESON_SVP_SIZE+3];
	short int label;
	short int *short_ptr;
	short int *short_ptr2;
	int	*int_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_cbat9001_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->svp_year);
		fprintf(stderr,"dbg5       month:            %d\n",data->svp_month);
		fprintf(stderr,"dbg5       day:              %d\n",data->svp_day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->svp_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->svp_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->svp_second);
		fprintf(stderr,"dbg5       hundredth_sec:    %d\n",data->svp_hundredth_sec);
		fprintf(stderr,"dbg5       thousandth_sec:   %d\n",data->svp_thousandth_sec);
		fprintf(stderr,"dbg5       svp_latitude:     %d\n",data->svp_latitude);
		fprintf(stderr,"dbg5       svp_longitude:    %d\n",data->svp_longitude);
		fprintf(stderr,"dbg5       svp_num:          %d\n",data->svp_num);
		for (i=0;i<data->svp_num;i++)
			fprintf(stderr,"dbg5       depth: %f     vel: %f\n",
				data->svp_depth[i],data->svp_vel[i]);
		}

	/* write the record label */
	label = RESON_SVP;
#ifdef BYTESWAPPED
	label = (short) mb_swap_short(label);
#endif
	status = fwrite(&label,1,2,mbfp);
	if (status != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		line[0] = (char) data->svp_day;
		line[1] = (char) data->svp_month;
		line[2] = (char) data->svp_year;
		line[3] = (char) data->svp_hour;
		line[4] = (char) data->svp_minute;
		line[5] = (char) data->svp_second;
		line[6] = (char) data->svp_hundredth_sec;
		line[7] = (char) data->svp_thousandth_sec;
#ifndef BYTESWAPPED
		int_ptr = (int *) &line[8];
		*int_ptr = data->svp_latitude;
		int_ptr = (int *) &line[12];
		*int_ptr = data->svp_longitude;
#else
		int_ptr = (int *) &line[8];
		*int_ptr = (int) mb_swap_long(data->svp_latitude);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_long(data->svp_longitude);
#endif
		for (i=0;i<data->svp_num;i++)
			{
			short_ptr = (short int *) &line[16+4*i];
			short_ptr2 = (short int *) &line[18+4*i];
#ifndef BYTESWAPPED
			*short_ptr = (short int) data->svp_depth[i];
			*short_ptr2 = (short int) data->svp_vel[i];
#else
			*short_ptr = (short int) 
				mb_swap_short((short int)data->svp_depth[i]);
			*short_ptr2 = (short int) 
				mb_swap_short((short int)data->svp_vel[i]);
#endif
			}
		for (i=data->svp_num;i<500;i++)
			{
			short_ptr = (short int *) &line[16+4*i];
			short_ptr2 = (short int *) &line[18+4*i];
			*short_ptr = 0;
			*short_ptr2 = 0;
			}
		line[RESON_SVP_SIZE] = 0x03;
		line[RESON_SVP_SIZE+1] = '\0';
		line[RESON_SVP_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,RESON_SVP_SIZE+3,mbfp);
		if (status != RESON_SVP_SIZE+3)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_cbat9001_wr_bath(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_cbat9001_wr_bath";
	int	status = MB_SUCCESS;
	struct mbf_cbat9001_struct *data;
	char	line[RESON_BATH_SIZE+3];
	char	*beamarray;
	short int label;
	unsigned char *char_ptr;
	short int *short_ptr;
	int	*int_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_cbat9001_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->second);
		fprintf(stderr,"dbg5       hundredth_sec:    %d\n",data->hundredth_sec);
		fprintf(stderr,"dbg5       thousandth_sec:   %d\n",data->thousandth_sec);
		fprintf(stderr,"dbg5       latitude:         %d\n",data->latitude);
		fprintf(stderr,"dbg5       longitude:        %d\n",data->longitude);
		fprintf(stderr,"dbg5       roll:             %d\n",data->roll);
		fprintf(stderr,"dbg5       pitch:            %d\n",data->pitch);
		fprintf(stderr,"dbg5       heading:          %d\n",data->heading);
		fprintf(stderr,"dbg5       heave:            %d\n",data->heave);
		fprintf(stderr,"dbg5       ping_number:      %d\n",data->ping_number);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       mode:             %d\n",data->mode);
		fprintf(stderr,"dbg5       gain1:            %d\n",data->gain1);
		fprintf(stderr,"dbg5       gain2:            %d\n",data->gain2);
		fprintf(stderr,"dbg5       gain3:            %d\n",data->gain3);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",data->beams_bath);
		fprintf(stderr,"dbg5       beam bath xtrack ltrack tt amp qual heave\n");
		for (i=0;i<data->beams_bath;i++)
			fprintf(stderr,"dbg5       beam:%d  bath:%d  xtrck:%d  ltrck:%d tt:%d  angle:%d amp:%d  qual:%d\n",
				i,data->bath[i],data->bath_acrosstrack[i],
				data->bath_alongtrack[i],data->tt[i],
				data->angle[i],data->amp[i],data->quality[i]);
		}

	/* write the record label */
	label = RESON_BATH;
#ifdef BYTESWAPPED
	label = (short) mb_swap_short(label);
#endif
	status = fwrite(&label,1,2,mbfp);
	if (status != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		line[0] = (char) data->day;
		line[1] = (char) data->month;
		line[2] = (char) data->year;
		line[3] = (char) data->hour;
		line[4] = (char) data->minute;
		line[5] = (char) data->second;
		line[6] = (char) data->hundredth_sec;
		line[7] = (char) data->thousandth_sec;
#ifndef BYTESWAPPED
		int_ptr = (int *) &line[8];
		*int_ptr = data->latitude;
		int_ptr = (int *) &line[12];
		*int_ptr = data->longitude;
		short_ptr = (short int *) &line[16];
		*short_ptr = (short int) data->roll;
		short_ptr = (short int *) &line[18];
		*short_ptr = (short int) data->pitch;
		short_ptr = (short int *) &line[20];
		*short_ptr = (short int) data->heading;
		short_ptr = (short int *) &line[22];
		*short_ptr = (short int) data->heave;
		short_ptr = (short int *) &line[24];
		*short_ptr = (short int) data->ping_number;
		short_ptr = (short int *) &line[26];
		*short_ptr = (short int) data->sound_vel;
#else
		int_ptr = (int *) &line[8];
		*int_ptr = (int) mb_swap_long(data->latitude);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_long(data->longitude);
		short_ptr = (short int *) &line[16];
		*short_ptr = (int) mb_swap_short(data->roll);
		short_ptr = (short int *) &line[18];
		*short_ptr = (int) mb_swap_short(data->pitch);
		short_ptr = (short int *) &line[20];
		*short_ptr = (int) mb_swap_short(data->heading);
		short_ptr = (short int *) &line[22];
		*short_ptr = (int) mb_swap_short(data->heave);
		short_ptr = (short int *) &line[24];
		*short_ptr = (int) mb_swap_short(data->ping_number);
		short_ptr = (short int *) &line[26];
		*short_ptr = (int) mb_swap_short(data->sound_vel);
#endif
		line[28] = (char) data->mode;
		line[29] = (char) data->gain1;
		line[30] = (char) data->gain2;
		line[31] = (char) data->gain3;


#ifndef BYTESWAPPED
		for (i=0;i<MBF_CBAT9001_MAXBEAMS;i++)
			{
			beamarray = line + 32 + 12*i;
			short_ptr = (short int *) beamarray; 
			*short_ptr = (short int) data->bath[i];
			short_ptr = ((short int *) beamarray) + 1; 
			*short_ptr = (short int) data->bath_acrosstrack[i];
			short_ptr = ((short int *) beamarray) + 2; 
			*short_ptr = (short int) data->bath_alongtrack[i];
			short_ptr = ((short int *) beamarray) + 3; 
			*short_ptr = (short int) data->tt[i];
			short_ptr = ((short int *) beamarray) + 4; 
			*short_ptr = (short int) data->angle[i];
			char_ptr = (unsigned char *) (beamarray + 10); 
			*char_ptr = (char) data->quality[i];
			char_ptr = (unsigned char *) (beamarray + 11); 
			*char_ptr = (char) data->amp[i];
			}
#else
		for (i=0;i<MBF_CBAT9001_MAXBEAMS;i++)
			{
			beamarray = line + 32 + 12*i;
			short_ptr = (short int *) beamarray; 
			*short_ptr = (short int) 
				mb_swap_short((short int)data->bath[i]);
			short_ptr = ((short int *) beamarray) + 1; 
			*short_ptr = (short int) 
				mb_swap_short((short int)data->bath_acrosstrack[i]);
			short_ptr = ((short int *) beamarray) + 2; 
			*short_ptr = (short int) 
				mb_swap_short((short int)data->bath_alongtrack[i]);
			short_ptr = ((short int *) beamarray) + 3; 
			*short_ptr = (short int) 
				mb_swap_short((short int)data->tt[i]);
			short_ptr = ((short int *) beamarray) + 4; 
			*short_ptr = (short int) 
				mb_swap_short((short int)data->angle[i]);
			char_ptr = beamarray + 10; 
			*char_ptr = (char) data->quality[i];
			char_ptr = beamarray + 11; 
			*char_ptr = (unsigned char) data->amp[i];
			}
#endif
		line[RESON_BATH_SIZE] = 0x03;
		line[RESON_BATH_SIZE+1] = '\0';
		line[RESON_BATH_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,RESON_BATH_SIZE+3,mbfp);
		if (status != RESON_BATH_SIZE+3)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
