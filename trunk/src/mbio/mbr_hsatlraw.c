/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hsatlraw.c	2/11/93
 *	$Id: mbr_hsatlraw.c,v 4.9 1996-04-22 13:21:19 caress Exp $
 *
 *    Copyright (c) 1993, 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbr_hsatlraw.c contains the functions for reading and writing
 * multibeam data in the HSATLRAW format.  
 * These functions include:
 *   mbr_alm_hsatlraw	- allocate read/write memory
 *   mbr_dem_hsatlraw	- deallocate read/write memory
 *   mbr_rt_hsatlraw	- read and translate data
 *   mbr_wt_hsatlraw	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 11, 1993
 * $Log: not supported by cvs2svn $
 * Revision 4.8  1995/07/26  14:45:39  caress
 * Fixed problems related to shallow water data.
 *
 * Revision 4.7  1995/07/13  21:40:28  caress
 * Fixed problem with scaling of center beam depths.
 *
 * Revision 4.6  1995/03/17  15:12:59  caress
 * Changes related to handling early, problematic
 * Ewing Hydrosweep data.
 *
 * Revision 4.5  1995/03/08  13:31:09  caress
 * Fixed bug related to handling of shallow water data and the depth scale.
 *
 * Revision 4.4  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.3  1995/02/22  21:55:10  caress
 * Fixed reading of amplitude data from existing data.
 *
 * Revision 4.2  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.1  1994/05/21  02:23:29  caress
 * Made sure that mb_io_ptr->new_bath_alongtrack is set to zero on reading.
 *
 * Revision 4.1  1994/05/21  02:23:29  caress
 * Made sure that mb_io_ptr->new_bath_alongtrack is set to zero on reading.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.3  1994/03/04  22:27:09  caress
 * Reduced output amplitude values by a factor of 10 so that
 * general range should be between 20 and 500, hopefully
 * enough to insure actual range between 1 and 999.
 *
 * Revision 4.2  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.1  1994/03/03  03:15:16  caress
 * Added simple processing of amplitude data as part of
 * reading.  This will have to be improved, but its a start.
 *
 * Revision 4.0  1994/02/21  03:59:50  caress
 * First cut at new version. Altered to be consistent
 * with passing of three types of data: bathymetry,
 * amplitude, and sidescan.
 *
 * Revision 3.0  1993/05/14  22:55:16  sohara
 * initial version
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
#include "../../include/mb_define.h"
#include "../../include/mbsys_hsds.h"
#include "../../include/mbf_hsatlraw.h"

/*--------------------------------------------------------------------*/
int mbr_alm_hsatlraw(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_hsatlraw.c,v 4.9 1996-04-22 13:21:19 caress Exp $";
	char	*function_name = "mbr_alm_hsatlraw";
	int	status = MB_SUCCESS;
	int	i;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsatlraw_struct *data;
	char	*data_ptr;

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
	mb_io_ptr->structure_size = sizeof(struct mbf_hsatlraw_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_hsds_struct),
				&mb_io_ptr->store_data,error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_hsatlraw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;

	/* initialize everything to zeros */
	mbr_zero_hsatlraw(verbose,data_ptr,error);

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
int mbr_dem_hsatlraw(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_hsatlraw";
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
int mbr_zero_hsatlraw(verbose,data_ptr,error)
int	verbose;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_zero_hsatlraw";
	int	status = MB_SUCCESS;
	struct mbf_hsatlraw_struct *data;
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
	data = (struct mbf_hsatlraw_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		/* type of data record */
		data->kind = MB_DATA_NONE;

		/* position (all records ) */
		data->lon = 0.0;
		data->lat = 0.0;

		/* time stamp (all records ) */
		data->year = 0;
		data->month = 0;
		data->day = 0;
		data->hour = 0;
		data->minute = 0;
		data->second = 0;
		data->alt_minute = 0;
		data->alt_second = 0;

		/* additional navigation and depths (ERGNMESS and ERGNEICH) */
		data->course_true = 0.0;
		data->speed_transverse = 0.0;
		data->speed = 0.0;
		data->speed_reference[0] = '\0';
		data->pitch = 0.0;
		data->track = 0;
		data->depth_center = 0.0;
		data->depth_scale = 0.0;
		data->spare = 0;
		for (i=0;i<MBF_HSATLRAW_BEAMS;i++)
			{
			data->distance[i] = 0;
			data->depth[i] = 0;
			}

		/* travel time data (ERGNSLZT) */
		data->course_ground = 0.0;
		data->speed_ground = 0.0;
		data->heave = 0.0;
		data->roll = 0.0;
		data->time_center = 0.0;
		data->time_scale = 0.0;
		for (i=0;i<MBF_HSATLRAW_BEAMS;i++)
			data->time[i] = 0;
		for (i=0;i<11;i++)
			data->gyro[i] = 0.0;

		/* amplitude data (ERGNAMPL) */
		data->mode[0] = '\0';
		data->trans_strbd = 0;
		data->trans_vert = 0;
		data->trans_port = 0;
		data->pulse_len_strbd = 0;
		data->pulse_len_vert = 0;
		data->pulse_len_port = 0;
		data->gain_start = 0;
		data->r_compensation_factor = 0;
		data->compensation_start = 0;
		data->increase_start = 0;
		data->tvc_near = 0;
		data->tvc_far = 0;
		data->increase_int_near = 0;
		data->increase_int_far = 0;
		data->gain_center = 0;
		data->filter_gain = 0.0;
		data->amplitude_center = 0;
		data->echo_duration_center = 0;
		data->echo_scale_center = 0;
		for (i=0;i<16;i++)
			{
			data->gain[i] = 0;
			data->echo_scale[i] = 0;
			}
		for (i=0;i<MBF_HSATLRAW_BEAMS;i++)
			{
			data->amplitude[i] = 0;
			data->echo_duration[i] = 0;
			}

		/* mean velocity (ERGNHYDI) */
		data->draught = 0.0;
		data->vel_mean = 0.0;
		data->vel_keel = 0.0;
		data->tide = 0.0;

		/* water velocity profile */
		data->num_vel = 0;
		for (i=0;i<MBF_HSATLRAW_MAXVEL;i++)
			{
			data->depth[i] = 0;
			data->velocity[i] = 0;
			}

		/* navigation source (ERGNPOSI) */
		data->pos_corr_x = 0.0;
		data->pos_corr_y = 0.0;
		strncpy(data->sensors,"POS",9);

		/* comment (LDEOCOMM) */
		strncpy(data->comment,"\0",MBF_HSATLRAW_MAXLINE);

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
int mbr_rt_hsatlraw(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_hsatlraw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsatlraw_struct *data;
	struct mbsys_hsds_struct *store;
	double	gain_beam, factor;
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
	data = (struct mbf_hsatlraw_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_hsds_struct *) store_ptr;

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

	/* read next data from file */
	status = mbr_hsatlraw_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate time and navigation values to current 
		ping variables in mbio descriptor structure */
	if (status == MB_SUCCESS
		&& data->kind != MB_DATA_COMMENT)
		{
		/* get time */
		mb_io_ptr->new_time_i[0] = data->year;
		mb_io_ptr->new_time_i[1] = data->month;
		mb_io_ptr->new_time_i[2] = data->day;
		mb_io_ptr->new_time_i[3] = data->hour;
		mb_io_ptr->new_time_i[4] = data->minute;
		mb_io_ptr->new_time_i[5] = data->second;
		mb_io_ptr->new_time_i[6] = 0;
		mb_get_time(verbose,mb_io_ptr->new_time_i,
			&mb_io_ptr->new_time_d);

		/* get navigation */
		mb_io_ptr->new_lon = data->lon;
		mb_io_ptr->new_lat = data->lat;
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

		/* print debug statements */
		if (verbose >= 5)
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
			fprintf(stderr,"dbg4       longitude:  %f\n",
				mb_io_ptr->new_lon);
			fprintf(stderr,"dbg4       latitude:   %f\n",
				mb_io_ptr->new_lat);
			}
		}

	/* translate heading and speed values to current ping variables 
		in mbio descriptor structure */
	if (status == MB_SUCCESS 
		&& (data->kind == MB_DATA_DATA
		|| data->kind == MB_DATA_CALIBRATE
		|| data->kind == MB_DATA_STANDBY))
		{
		/* get heading */
		mb_io_ptr->new_heading = data->course_true;

		/* get speed (convert m/s to km/hr) */
		mb_io_ptr->new_speed = 3.6*data->speed;

		/* print more debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"dbg4       speed:      %f\n",
				mb_io_ptr->new_speed);
			fprintf(stderr,"dbg4       heading:    %f\n",
				mb_io_ptr->new_heading);
			}
		}

	/* translate bathymetry values to current ping variables 
		in mbio descriptor structure */
	if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_DATA)
		{
		/* read distance and depth values into storage arrays */
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			mb_io_ptr->new_bath[i] 
				= data->depth_scale*data->depth[i];
			mb_io_ptr->new_bath_acrosstrack[i] 
				= data->depth_scale*data->distance[i];
			mb_io_ptr->new_bath_alongtrack[i] = 0.0;
			}
		mb_io_ptr->new_bath[29] = data->depth_center;
		mb_io_ptr->new_bath_acrosstrack[29] = 0.0;

		/* process beam amplitudes */
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			gain_beam = 6*data->gain[which_gain[i]];
			factor = 100.*pow(10.,(-0.05*gain_beam));
			mb_io_ptr->new_amp[i] 
					= factor*data->amplitude[i];
			if (data->depth[i] < 0)
				mb_io_ptr->new_amp[i] = 
					-mb_io_ptr->new_amp[i];
			}

		/* print more debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"dbg4       beams_bath: %d\n",
				mb_io_ptr->beams_bath);
			fprintf(stderr,"dbg4       beams_amp: %d\n",
				mb_io_ptr->beams_amp);
			for (i=0;i<mb_io_ptr->beams_bath;i++)
			  fprintf(stderr,"dbg4       bath[%d]: %f  amp[%d]: %f  bathdist[%d]: %f\n",
				i,mb_io_ptr->new_bath[i],
				i,mb_io_ptr->new_amp[i],
				i,mb_io_ptr->new_bath_acrosstrack[i]);
			}

		}

	/* copy comment to mbio descriptor structure */
	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(mb_io_ptr->new_comment,data->comment,
			MBF_HSATLRAW_MAXLINE);

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

	/* translate values to hydrosweep data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* type of data record */
		store->kind = data->kind;

		/* position (all records ) */
		store->lon = data->lon;
		store->lat = data->lat;

		/* time stamp (all records ) */
		store->year = data->year;
		store->month = data->month;
		store->day = data->day;
		store->hour = data->hour;
		store->minute = data->minute;
		store->second = data->second;
		store->alt_minute = data->alt_minute;
		store->alt_second = data->alt_second;

		/* additional navigation and depths (ERGNMESS and ERGNEICH) */
		store->course_true = data->course_true;
		store->speed_transverse = data->speed_transverse;
		store->speed = data->speed;
		store->speed_reference[0] = data->speed_reference[0];
		store->pitch = data->pitch;
		store->track = data->track;
		store->depth_center = data->depth_center;
		store->depth_scale = data->depth_scale;
		store->spare = data->spare;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			store->distance[i] = data->distance[i];
			store->depth[i] = data->depth[i];
			}

		/* travel time data (ERGNSLZT) */
		store->course_ground = data->course_ground;
		store->speed_ground = data->speed_ground;
		store->heave = data->heave;
		store->roll = data->roll;
		store->time_center = data->time_center;
		store->time_scale = data->time_scale;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			store->time[i] = data->time[i];
		for (i=0;i<11;i++)
			store->gyro[i] = data->gyro[i];

		/* amplitude data (ERGNAMPL) */
		store->mode[0] = data->mode[0];
		store->trans_strbd = data->trans_strbd;
		store->trans_vert = data->trans_vert;
		store->trans_port = data->trans_port;
		store->pulse_len_strbd = data->pulse_len_strbd;
		store->pulse_len_vert = data->pulse_len_vert;
		store->pulse_len_port = data->pulse_len_port;
		store->gain_start = data->gain_start;
		store->r_compensation_factor = data->r_compensation_factor;
		store->compensation_start = data->compensation_start;
		store->increase_start = data->increase_start;
		store->tvc_near = data->tvc_near;
		store->tvc_far = data->tvc_far;
		store->increase_int_near = data->increase_int_near;
		store->increase_int_far = data->increase_int_far;
		store->gain_center = data->gain_center;
		store->filter_gain = data->filter_gain;
		store->amplitude_center = data->amplitude_center;
		store->echo_duration_center = data->echo_duration_center;
		store->echo_scale_center = data->echo_scale_center;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			store->amplitude[i] = data->amplitude[i];
			store->echo_duration[i] = data->echo_duration[i];
			}
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			store->gain[i] = data->gain[i];
			store->echo_scale[i] = data->echo_scale[i];
			}

		/* mean velocity (ERGNHYDI) */
		store->draught = data->draught;
		store->vel_mean = data->vel_mean;
		store->vel_keel = data->vel_keel;
		store->tide = data->tide;

		/* water velocity profile (HS_ERGNCTDS) */
		store->num_vel = data->num_vel;
		for (i=0;i<data->num_vel;i++)
			{
			store->vdepth[i] = data->vdepth[i];
			store->velocity[i] = data->velocity[i];
			}

		/* navigation source (ERGNPOSI) */
		store->pos_corr_x = data->pos_corr_x;
		store->pos_corr_y = data->pos_corr_y;
		strncpy(store->sensors,data->sensors,8);

		/* comment (LDEOCMNT) */
		strncpy(store->comment,data->comment,MBSYS_HSDS_MAXLINE);

		/* processed backscatter */
		store->back_scale = 1.0;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			store->back[i] = mb_io_ptr->new_amp[i];
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
int mbr_wt_hsatlraw(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_hsatlraw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsatlraw_struct *data;
	char	*data_ptr;
	struct mbsys_hsds_struct *store;
	double	scalefactor;
	int	i;

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
	data = (struct mbf_hsatlraw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	store = (struct mbsys_hsds_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		/* type of data record */
		data->kind = store->kind;

		/* position (all records ) */
		data->lon = store->lon;
		data->lat = store->lat;

		/* time stamp (all records ) */
		data->year = store->year;
		data->month = store->month;
		data->day = store->day;
		data->hour = store->hour;
		data->minute = store->minute;
		data->second = store->second;
		data->alt_minute = store->alt_minute;
		data->alt_second = store->alt_second;

		/* additional navigation and depths (ERGNMESS and ERGNEICH) */
		data->course_true = store->course_true;
		data->speed_transverse = store->speed_transverse;
		data->speed = store->speed;
		data->speed_reference[0] = store->speed_reference[0];
		data->pitch = store->pitch;
		data->track = store->track;
		data->depth_center = store->depth_center;
		data->depth_scale = store->depth_scale;
		data->spare = store->spare;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			data->distance[i] = store->distance[i];
			data->depth[i] = store->depth[i];
			}

		/* travel time data (ERGNSLZT) */
		data->course_ground = store->course_ground;
		data->speed_ground = store->speed_ground;
		data->heave = store->heave;
		data->roll = store->roll;
		data->time_center = store->time_center;
		data->time_scale = store->time_scale;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			data->time[i] = store->time[i];
		for (i=0;i<11;i++)
			data->gyro[i] = store->gyro[i];

		/* amplitude data (ERGNAMPL) */
		data->mode[0] = store->mode[0];
		data->trans_strbd = store->trans_strbd;
		data->trans_vert = store->trans_vert;
		data->trans_port = store->trans_port;
		data->pulse_len_strbd = store->pulse_len_strbd;
		data->pulse_len_vert = store->pulse_len_vert;
		data->pulse_len_port = store->pulse_len_port;
		data->gain_start = store->gain_start;
		data->r_compensation_factor = store->r_compensation_factor;
		data->compensation_start = store->compensation_start;
		data->increase_start = store->increase_start;
		data->tvc_near = store->tvc_near;
		data->tvc_far = store->tvc_far;
		data->increase_int_near = store->increase_int_near;
		data->increase_int_far = store->increase_int_far;
		data->gain_center = store->gain_center;
		data->filter_gain = store->filter_gain;
		data->amplitude_center = store->amplitude_center;
		data->echo_duration_center = store->echo_duration_center;
		data->echo_scale_center = store->echo_scale_center;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			data->amplitude[i] = store->amplitude[i];
			data->echo_duration[i] = store->echo_duration[i];
			}
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			data->gain[i] = store->gain[i];
			data->echo_scale[i] = store->echo_scale[i];
			}

		/* mean velocity (ERGNHYDI) */
		data->draught = store->draught;
		data->vel_mean = store->vel_mean;
		data->vel_keel = store->vel_keel;
		data->tide = store->tide;

		/* water velocity profile (HS_ERGNCTDS) */
		data->num_vel = store->num_vel;
		for (i=0;i<store->num_vel;i++)
			{
			data->vdepth[i] = store->vdepth[i];
			data->velocity[i] = store->velocity[i];
			}

		/* navigation source (ERGNPOSI) */
		data->pos_corr_x = store->pos_corr_x;
		data->pos_corr_y = store->pos_corr_y;
		strncpy(data->sensors,store->sensors,8);

		/* comment (LDEOCMNT) */
		strncpy(data->comment,store->comment,MBSYS_HSDS_MAXLINE);

		}

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		data->kind = mb_io_ptr->new_kind;

	/* check for comment to be copied from mb_io_ptr */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		strncpy(data->comment,mb_io_ptr->new_comment,
			MBF_HSATLRAW_MAXLINE);
		}

	/* else check for ping data to be copied from mb_io_ptr */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/* get time */
		data->year = mb_io_ptr->new_time_i[0];
		data->month = mb_io_ptr->new_time_i[1];
		data->day = mb_io_ptr->new_time_i[2];
		data->hour = mb_io_ptr->new_time_i[3];
		data->minute = mb_io_ptr->new_time_i[4];
		data->second = mb_io_ptr->new_time_i[5];

		/* get navigation */
		data->lon = mb_io_ptr->new_lon;
		data->lat = mb_io_ptr->new_lat;

		/* get heading */
		data->course_true = mb_io_ptr->new_heading;

		/* get speed (convert km/hr to m/s) */
		data->speed = 0.2777777778*mb_io_ptr->new_speed;

		/* put distance and depth values 
			into hsatlraw data structure */
		if (data->depth_scale > 0.0)
			scalefactor = 1.0/data->depth_scale;
		else
			scalefactor = 1.0;
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			data->depth[i] = scalefactor*mb_io_ptr->new_bath[i];
			data->distance[i] 
				= scalefactor*mb_io_ptr->new_bath_acrosstrack[i];
			}
		data->depth_center = mb_io_ptr->new_bath[29];

		/* add some plausible amounts for some of the 
			variables in the HSATLRAW record */
		data->speed_reference[0] = 'B';	/* assume speed over ground */
		if (data->depth_scale <= 0.0)
			data->depth_scale = 1.0;	/* this is a unit scale factor */
		data->spare = 1;
		}

	/* check that no bathymetry values are negative */
	for (i=0;i<mb_io_ptr->beams_bath;i++)
		{
		if (data->depth[i] < 0)
			data->depth[i] = 0;
		}

	/* write next data to file */
	status = mbr_hsatlraw_wr_data(verbose,mbio_ptr,data_ptr,error);

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
int mbr_hsatlraw_rd_data(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsatlraw_struct *data;
	char	*data_ptr;
	FILE	*mbfp;
	int	done;
	int	expect;

	static int line_save_flag = MB_NO;
	static char raw_line[MBF_HSATLRAW_MAXLINE] = "\0";
	static int type = MBF_HSATLRAW_NONE;
	static int shift = 0;

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
	data = (struct mbf_hsatlraw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	mbfp = mb_io_ptr->mbfp;

	/* initialize everything to zeros */
	mbr_zero_hsatlraw(verbose,data_ptr,error);

	done = MB_NO;
	expect = MBF_HSATLRAW_NONE;
	while (done == MB_NO)
		{

		/* get next record label */
		if (line_save_flag == MB_NO)
			status = mbr_hsatlraw_rd_label(verbose,mbfp,
				raw_line,&type,&shift,error);
		else
			line_save_flag = MB_NO;

		/* read the appropriate data records */
		if (status == MB_FAILURE && expect == MBF_HSATLRAW_NONE)
			{
			done = MB_YES;
			}
		else if (status == MB_FAILURE && expect != MBF_HSATLRAW_NONE)
			{
			done = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else if (expect != MBF_HSATLRAW_NONE && expect != type)
			{
			done = MB_YES;
			line_save_flag = MB_YES;
			}
		else if (type == MBF_HSATLRAW_RAW_LINE)
			{
			strcpy(data->comment,raw_line+shift);
			done = MB_YES;
			data->kind = MB_DATA_RAW_LINE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			status = MB_FAILURE;
			}
		else if (type == MBF_HSATLRAW_ERGNHYDI)
			{
			status = mbr_hsatlraw_rd_ergnhydi(
				verbose,mbfp,data,shift,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_MEAN_VELOCITY;
				}
			}
		else if (type == MBF_HSATLRAW_ERGNPARA)
			{
			status = mbr_hsatlraw_rd_ergnpara(
				verbose,mbfp,data,shift,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_STANDBY;
				}
			}
		else if (type == MBF_HSATLRAW_ERGNPOSI)
			{
			status = mbr_hsatlraw_rd_ergnposi(
				verbose,mbfp,data,shift,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_NAV_SOURCE;
				}
			}
		else if (type == MBF_HSATLRAW_ERGNEICH)
			{
			status = mbr_hsatlraw_rd_ergneich(
				verbose,mbfp,data,shift,error);
			if (status == MB_SUCCESS)
				{
				done = MB_NO;
				data->kind = MB_DATA_CALIBRATE;
				expect = MBF_HSATLRAW_ERGNSLZT;
				}
			}
		else if (type == MBF_HSATLRAW_ERGNMESS)
			{
			status = mbr_hsatlraw_rd_ergnmess(
				verbose,mbfp,data,shift,error);
			if (status == MB_SUCCESS)
				{
				done = MB_NO;
				data->kind = MB_DATA_DATA;
				expect = MBF_HSATLRAW_ERGNSLZT;
				}
			}
		else if (type == MBF_HSATLRAW_ERGNSLZT)
			{
			status = mbr_hsatlraw_rd_ergnslzt(
				verbose,mbfp,data,shift,error);
			if (status == MB_SUCCESS && expect == MBF_HSATLRAW_ERGNSLZT)
				{
				done = MB_NO;
				expect = MBF_HSATLRAW_ERGNAMPL;
				}
			else if (status == MB_SUCCESS)
				{
				done = MB_YES;
				expect = MBF_HSATLRAW_NONE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
				}
			}
		else if (type == MBF_HSATLRAW_ERGNCTDS)
			{
			status = mbr_hsatlraw_rd_ergnctds(
				verbose,mbfp,data,shift,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_VELOCITY_PROFILE;
				}
			}
		else if (type == MBF_HSATLRAW_ERGNAMPL)
			{
			status = mbr_hsatlraw_rd_ergnampl(
				verbose,mbfp,data,shift,error);
			if (status == MB_SUCCESS 
				&& expect == MBF_HSATLRAW_ERGNAMPL)
				{
				done = MB_YES;
				expect = MBF_HSATLRAW_NONE;
				}
			else if (status == MB_SUCCESS)
				{
				done = MB_YES;
				expect = MBF_HSATLRAW_NONE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
				}
			}
		else if (type == MBF_HSATLRAW_LDEOCMNT)
			{
			status = mbr_hsatlraw_rd_ldeocmnt(
				verbose,mbfp,data,shift,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_COMMENT;
				}
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
int	mbr_hsatlraw_rd_label(verbose,mbfp,line,type,shift,error)
int	verbose;
FILE	*mbfp;
char	*line;
int	*type;
int	*shift;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_rd_label";
	int	status = MB_SUCCESS;
	int	i;
	int	icmp;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		}

	/* read next line in file */
	status = mbr_hsatlraw_read_line(verbose,mbfp,0,line,error);

	/* see if we just encountered an identifier record */
	if (status == MB_SUCCESS)
		{
		*type = MBF_HSATLRAW_RAW_LINE;
		*shift = 0;
		for (i=1;i<MBF_HSATLRAW_RECORDS;i++)
			{
			icmp = strncmp(line,mbf_hsatlraw_labels[i],8);
			if (icmp == 0) 
				*type = i;
			}
		}

	/* didn't find one with zero shift, try shift = 4 in case this
		is tape data */
	if (status == MB_SUCCESS && *type == MBF_HSATLRAW_RAW_LINE)
		{
		*shift = 4;
		for (i=1;i<MBF_HSATLRAW_RECORDS;i++)
			{
			icmp = strncmp(line+4,mbf_hsatlraw_labels[i],8);
			if (icmp == 0) 
				*type = i;
			}
		if (*type == MBF_HSATLRAW_RAW_LINE)
			*shift = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       line:       %s\n",line);
		fprintf(stderr,"dbg2       type:       %d\n",*type);
		fprintf(stderr,"dbg2       shift:      %d\n",*shift);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int	mbr_hsatlraw_read_line(verbose,mbfp,minimum_size,line,error)
int	verbose;
FILE	*mbfp;
int	minimum_size;
char	*line;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_read_line";
	int	status = MB_SUCCESS;
	int	nchars;
	int	done;
	char	*result;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		}

	/* read next good line in file */
	done = MB_NO;
	do
		{
		/* read next line in file */
		strncpy(line,"\0",MBF_HSATLRAW_MAXLINE);
		result = fgets(line,MBF_HSATLRAW_MAXLINE,mbfp);

		/* check size of line */
		nchars = strlen(line);

		/* check for eof */
		if (result == line)
			{
			if (nchars >= minimum_size)
				done = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			done = MB_YES;
			*error = MB_ERROR_EOF;
			status = MB_FAILURE;
			}

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  New line read in function <%s>\n",
				function_name);
			fprintf(stderr,"dbg5       line:       %s\n",line);
			fprintf(stderr,"dbg5       chars:      %d\n",nchars);
			}

		}
	while (done == MB_NO);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       line:       %s\n",line);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_hsatlraw_rd_ergnhydi(verbose,mbfp,data,shift,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsatlraw_struct *data;
int	shift;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_rd_ergnhydi";
	int	status = MB_SUCCESS;
	char	line[MBF_HSATLRAW_MAXLINE];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       shift:      %d\n",shift);
		}

	/* read event record from file */
	status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error);

	/* parse data from first line */
	if (status == MB_SUCCESS)
		{
		mb_get_double( &(data->lon),        line+shift,    12);
		mb_get_double( &(data->lat),        line+12+shift, 12);
		mb_get_int(    &(data->year),       line+24+shift,  4);
		mb_get_int(    &(data->month),      line+28+shift,  2);
		mb_get_int(    &(data->day),        line+30+shift,  2);
		mb_get_int(    &(data->hour),       line+32+shift,  2);
		mb_get_int(    &(data->minute),     line+34+shift,  2);
		mb_get_int(    &(data->second),     line+36+shift,  2);
		mb_get_int(    &(data->alt_minute), line+38+shift,  5);
		mb_get_int(    &(data->alt_second), line+43+shift,  2);
		mb_get_double( &(data->draught),    line+45+shift,  4);
		mb_get_double( &(data->vel_mean),   line+49+shift,  7);
		mb_get_double( &(data->vel_keel),   line+56+shift,  7);
		mb_get_double( &(data->tide),       line+63+shift,  6);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       alt minute:       %d\n",
			data->alt_minute);
		fprintf(stderr,"dbg5       alt second:       %d\n",
			data->alt_second);
		fprintf(stderr,"dbg5       draught:          %f\n",
			data->draught);
		fprintf(stderr,"dbg5       mean velocity:    %f\n",
			data->vel_mean);
		fprintf(stderr,"dbg5       keel velocity:    %f\n",
			data->vel_keel);
		fprintf(stderr,"dbg5       tide:             %f\n",data->tide);
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
int mbr_hsatlraw_rd_ergnpara(verbose,mbfp,data,shift,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsatlraw_struct *data;
int	shift;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_rd_ergnpara";
	int	status = MB_SUCCESS;
	char	line[MBF_HSATLRAW_MAXLINE];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       shift:      %d\n",shift);
		}

	/* read event record from file */
	status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error);

	/* parse data from first line */
	if (status == MB_SUCCESS)
		{
		mb_get_double( &(data->lon),        line+shift,    12);
		mb_get_double( &(data->lat),        line+12+shift, 12);
		mb_get_int(    &(data->year),       line+24+shift,  4);
		mb_get_int(    &(data->month),      line+28+shift,  2);
		mb_get_int(    &(data->day),        line+30+shift,  2);
		mb_get_int(    &(data->hour),       line+32+shift,  2);
		mb_get_int(    &(data->minute),     line+34+shift,  2);
		mb_get_int(    &(data->second),     line+36+shift,  2);
		mb_get_int(    &(data->alt_minute), line+38+shift,  5);
		mb_get_int(    &(data->alt_second), line+43+shift,  2);
		mb_get_double( &(data->course_true),line+45+shift,  5);
		mb_get_double( &(data->speed_transverse), line+50+shift, 9);
		mb_get_double( &(data->speed),      line+59+shift,  9);
		data->speed_reference[0] = line[68+shift];
		mb_get_double( &(data->pitch),      line+69+shift,  4);
		mb_get_int(    &(data->track),      line+73+shift,  4);
		mb_get_double( &(data->depth_center),line+77+shift, 7);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       alt minute:       %d\n",
			data->alt_minute);
		fprintf(stderr,"dbg5       alt second:       %d\n",
			data->alt_second);
		fprintf(stderr,"dbg5       course_true:      %f\n",
			data->course_true);
		fprintf(stderr,"dbg5       speed_transverse: %f\n",
			data->speed_transverse);
		fprintf(stderr,"dbg5       speed:            %f\n",
			data->speed);
		fprintf(stderr,"dbg5       speed_reference:  %c\n",
			data->speed_reference[0]);
		fprintf(stderr,"dbg5       pitch:            %f\n",
			data->pitch);
		fprintf(stderr,"dbg5       track:            %d\n",
			data->track);
		fprintf(stderr,"dbg5       depth_center:     %f\n",
			data->depth_center);
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
int mbr_hsatlraw_rd_ergnposi(verbose,mbfp,data,shift,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsatlraw_struct *data;
int	shift;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_rd_ergnposi";
	int	status = MB_SUCCESS;
	char	line[MBF_HSATLRAW_MAXLINE];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       shift:      %d\n",shift);
		}

	/* read event record from file */
	status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error);

	/* parse data from first line */
	if (status == MB_SUCCESS)
		{
		mb_get_double( &(data->lon),        line+shift,    12);
		mb_get_double( &(data->lat),        line+12+shift, 12);
		mb_get_int(    &(data->year),       line+24+shift,  4);
		mb_get_int(    &(data->month),      line+28+shift,  2);
		mb_get_int(    &(data->day),        line+30+shift,  2);
		mb_get_int(    &(data->hour),       line+32+shift,  2);
		mb_get_int(    &(data->minute),     line+34+shift,  2);
		mb_get_int(    &(data->second),     line+36+shift,  2);
		mb_get_int(    &(data->alt_minute), line+38+shift,  5);
		mb_get_int(    &(data->alt_second), line+43+shift,  2);
		mb_get_double( &(data->pos_corr_x), line+45+shift,  7);
		mb_get_double( &(data->pos_corr_y), line+52+shift,  7);
		strncpy(data->sensors,line+59+shift,8);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       alt minute:       %d\n",
			data->alt_minute);
		fprintf(stderr,"dbg5       alt second:       %d\n",
			data->alt_second);
		fprintf(stderr,"dbg5       pos_corr_x:       %f\n",
			data->pos_corr_x);
		fprintf(stderr,"dbg5       pos_corr_y:       %f\n",
			data->pos_corr_y);
		fprintf(stderr,"dbg5       sensors:          %s\n",
			data->sensors);
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
int mbr_hsatlraw_rd_ergneich(verbose,mbfp,data,shift,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsatlraw_struct *data;
int	shift;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_rd_ergneich";
	int	status = MB_SUCCESS;
	char	line[MBF_HSATLRAW_MAXLINE];
	int	i;
	int	numvals;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       shift:      %d\n",shift);
		}

	/* read event record from file */
	status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error);

	/* parse data from event record */
	if (status == MB_SUCCESS)
		{
		mb_get_double( &(data->lon),        line+shift,    12);
		mb_get_double( &(data->lat),        line+12+shift, 12);
		mb_get_int(    &(data->year),       line+24+shift,  4);
		mb_get_int(    &(data->month),      line+28+shift,  2);
		mb_get_int(    &(data->day),        line+30+shift,  2);
		mb_get_int(    &(data->hour),       line+32+shift,  2);
		mb_get_int(    &(data->minute),     line+34+shift,  2);
		mb_get_int(    &(data->second),     line+36+shift,  2);
		mb_get_int(    &(data->alt_minute), line+38+shift,  5);
		mb_get_int(    &(data->alt_second), line+43+shift,  2);
		mb_get_double( &(data->course_true),line+45+shift,  5);
		mb_get_double( &(data->speed_transverse), line+50+shift, 9);
		mb_get_double( &(data->speed),      line+59+shift,  9);
		data->speed_reference[0] = line[68+shift];
		mb_get_double( &(data->pitch),      line+69+shift,  4);
		mb_get_int(    &(data->track),      line+73+shift,  4);
		mb_get_double( &(data->depth_center),line+77+shift, 7);
		mb_get_double( &(data->depth_scale),line+84+shift,  4);
		mb_get_int( &(data->spare),line+88+shift,  2);
		if (data->depth_scale > 0.0)
			data->depth[29] = 
				(int) (data->depth_center/data->depth_scale);
		else
			data->depth[29] = (int) data->depth_center;
		data->distance[29] = 0;
		}

	/* read and parse data from first data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error)) 
		== MB_SUCCESS)
		{
		mb_get_int(&numvals,line+shift,2);
		if (numvals == 29)
		for (i=0;i<numvals;i++)
			mb_get_int(&(data->distance[i+30]),line+i*4+2+shift,4);
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read and parse data from second data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error)) 
		== MB_SUCCESS)
		{
		mb_get_int(&numvals,line+shift,2);
		if (numvals == 29)
		for (i=0;i<numvals;i++)
			mb_get_int(&(data->depth[i+30]),line+i*4+2+shift,4);
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read and parse data from third data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error)) 
		== MB_SUCCESS)
		{
		mb_get_int(&numvals,line+shift,2);
		if (numvals == 29)
		for (i=0;i<numvals;i++)
			{
			mb_get_int(&(data->distance[28-i]),line+i*4+2+shift,4);
			data->distance[28-i] = -data->distance[28-i];
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read and parse data from fourth data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error)) 
		== MB_SUCCESS)
		{
		mb_get_int(&numvals,line+shift,2);
		if (numvals == 29)
		for (i=0;i<numvals;i++)
			mb_get_int(&(data->depth[28-i]),line+i*4+2+shift,4);
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       alt minute:       %d\n",
			data->alt_minute);
		fprintf(stderr,"dbg5       alt second:       %d\n",
			data->alt_second);
		fprintf(stderr,"dbg5       course_true:      %f\n",
			data->course_true);
		fprintf(stderr,"dbg5       speed_transverse: %f\n",
			data->speed_transverse);
		fprintf(stderr,"dbg5       speed:            %f\n",
			data->speed);
		fprintf(stderr,"dbg5       speed_reference:  %c\n",
			data->speed_reference[0]);
		fprintf(stderr,"dbg5       pitch:            %f\n",
			data->pitch);
		fprintf(stderr,"dbg5       track:            %d\n",
			data->track);
		fprintf(stderr,"dbg5       depth_center:     %f\n",
			data->depth_center);
		fprintf(stderr,"dbg5       depth_scale:      %f\n",
			data->depth_scale);
		fprintf(stderr,"dbg5       spare:            %d\n",
			data->spare);
		fprintf(stderr,"dbg5       distances and depths:\n");
		for (i=0;i<MBF_HSATLRAW_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->distance[i],data->depth[i]);
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
int mbr_hsatlraw_rd_ergnmess(verbose,mbfp,data,shift,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsatlraw_struct *data;
int	shift;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_rd_ergnmess";
	int	status = MB_SUCCESS;
	char	line[MBF_HSATLRAW_MAXLINE];
	int	numvals;
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
		fprintf(stderr,"dbg2       shift:      %d\n",shift);
		}

	/* read event record from file */
	status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error);

	/* parse data from event record */
	if (status == MB_SUCCESS)
		{
		mb_get_double( &(data->lon),        line+shift,    12);
		mb_get_double( &(data->lat),        line+12+shift, 12);
		mb_get_int(    &(data->year),       line+24+shift,  4);
		mb_get_int(    &(data->month),      line+28+shift,  2);
		mb_get_int(    &(data->day),        line+30+shift,  2);
		mb_get_int(    &(data->hour),       line+32+shift,  2);
		mb_get_int(    &(data->minute),     line+34+shift,  2);
		mb_get_int(    &(data->second),     line+36+shift,  2);
		mb_get_int(    &(data->alt_minute), line+38+shift,  5);
		mb_get_int(    &(data->alt_second), line+43+shift,  2);
		mb_get_double( &(data->course_true),line+45+shift,  5);
		mb_get_double( &(data->speed_transverse), line+50+shift, 9);
		mb_get_double( &(data->speed),      line+59+shift,  9);
		data->speed_reference[0] = line[68+shift];
		mb_get_double( &(data->pitch),      line+69+shift,  4);
		mb_get_int(    &(data->track),      line+73+shift,  4);
		mb_get_double( &(data->depth_center),line+77+shift, 7);
		mb_get_double( &(data->depth_scale),line+84+shift,  4);
		mb_get_int( &(data->spare),line+88+shift,  2);
		if (data->depth_scale > 0.0)
			data->depth[29] = 
				(int) (data->depth_center/data->depth_scale);
		else
			data->depth[29] = (int) data->depth_center;
		data->distance[29] = 0;
		}

	/* read and parse data from first data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,
		shift+7,line,error)) == MB_SUCCESS)
		{
		mb_get_int(&numvals,line+shift,2);
		if (numvals == 29)
			for (i=0;i<numvals;i++)
				mb_get_int(&(data->distance[i+30]),
					line+i*4+2+shift,4);
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read and parse data from second data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,
		shift+7,line,error)) == MB_SUCCESS)
		{
		mb_get_int(&numvals,line+shift,2);
		if (numvals == 29)
			for (i=0;i<numvals;i++)
				{
				mb_get_int(&(data->depth[i+30]),
					line+i*4+2+shift,4);
				}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read and parse data from third data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,
		shift+7,line,error)) == MB_SUCCESS)
		{
		mb_get_int(&numvals,line+shift,2);
		if (numvals == 29)
			for (i=0;i<numvals;i++)
				{
				mb_get_int(&(data->distance[28-i]),
					line+i*4+2+shift,4);
				data->distance[28-i] = 
					-data->distance[28-i];
				}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read and parse data from fourth data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,
		shift+7,line,error)) == MB_SUCCESS)
		{
		mb_get_int(&numvals,line+shift,2);
		if (numvals == 29)
			for (i=0;i<numvals;i++)
				{
				mb_get_int(&(data->depth[28-i]),
					line+i*4+2+shift,4);
				}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       alt minute:       %d\n",
			data->alt_minute);
		fprintf(stderr,"dbg5       alt second:       %d\n",
			data->alt_second);
		fprintf(stderr,"dbg5       course_true:      %f\n",
			data->course_true);
		fprintf(stderr,"dbg5       speed_transverse: %f\n",
			data->speed_transverse);
		fprintf(stderr,"dbg5       speed:            %f\n",
			data->speed);
		fprintf(stderr,"dbg5       speed_reference:  %c\n",
			data->speed_reference[0]);
		fprintf(stderr,"dbg5       pitch:            %f\n",
			data->pitch);
		fprintf(stderr,"dbg5       track:            %d\n",
			data->track);
		fprintf(stderr,"dbg5       depth_center:     %f\n",
			data->depth_center);
		fprintf(stderr,"dbg5       depth_scale:      %f\n",
			data->depth_scale);
		fprintf(stderr,"dbg5       spare:            %d\n",
			data->spare);
		fprintf(stderr,"dbg5       distances and depths:\n");
		for (i=0;i<MBF_HSATLRAW_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->distance[i],data->depth[i]);
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
int mbr_hsatlraw_rd_ergnslzt(verbose,mbfp,data,shift,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsatlraw_struct *data;
int	shift;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_rd_ergnslzt";
	int	status = MB_SUCCESS;
	char	line[MBF_HSATLRAW_MAXLINE];
	int	i;
	int	numvals;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       shift:      %d\n",shift);
		}

	/* read event record from file */
	status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error);

	/* parse data from event record */
	if (status == MB_SUCCESS)
		{
		mb_get_double( &(data->lon),        line+shift,    12);
		mb_get_double( &(data->lat),        line+12+shift, 12);
		mb_get_int(    &(data->year),       line+24+shift,  4);
		mb_get_int(    &(data->month),      line+28+shift,  2);
		mb_get_int(    &(data->day),        line+30+shift,  2);
		mb_get_int(    &(data->hour),       line+32+shift,  2);
		mb_get_int(    &(data->minute),     line+34+shift,  2);
		mb_get_int(    &(data->second),     line+36+shift,  2);
		mb_get_double( &(data->course_true),line+38+shift,  5);
		mb_get_double( &(data->course_ground),line+43+shift,5);
		mb_get_double( &(data->speed_ground),line+48+shift, 9);
		mb_get_double( &(data->heave),      line+57+shift,  6);
		mb_get_double( &(data->pitch),      line+63+shift,  4);
		mb_get_double( &(data->roll),       line+67+shift,  5);
		mb_get_double( &(data->time_center),line+72+shift,  6);
		mb_get_double( &(data->time_scale), line+78+shift,  6);
		data->time[29] = (int) (0.0001*(data->time_center)
					/(data->time_scale));
		}

	/* read and parse data from first data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error)) 
		== MB_SUCCESS)
		{
		mb_get_int(&numvals,line+shift,2);
		if (numvals == 29)
		for (i=0;i<numvals;i++)
			mb_get_int(&(data->time[i+30]),line+i*4+2+shift,4);
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read and parse data from second data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error)) 
		== MB_SUCCESS)
		{
		mb_get_int(&numvals,line+shift,2);
		if (numvals == 29)
		for (i=0;i<numvals;i++)
			mb_get_int(&(data->time[28-i]),line+i*4+2+shift,4);
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read and parse data from third data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error)) 
		== MB_SUCCESS)
		{
		for (i=0;i<11;i++)
			mb_get_double(&(data->gyro[i]),line+i*5+shift,5);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       course_true:      %f\n",
			data->course_true);
		fprintf(stderr,"dbg5       course_ground:    %f\n",
			data->course_ground);
		fprintf(stderr,"dbg5       speed_ground:     %f\n",
			data->speed_ground);
		fprintf(stderr,"dbg5       heave:            %f\n",
			data->heave);
		fprintf(stderr,"dbg5       pitch:            %f\n",
			data->pitch);
		fprintf(stderr,"dbg5       roll:             %f\n",
			data->roll);
		fprintf(stderr,"dbg5       time_center:      %f\n",
			data->depth_center);
		fprintf(stderr,"dbg5       time_scale:       %f\n",
			data->time_scale);
		fprintf(stderr,"dbg5       travel times:\n");
		for (i=0;i<MBF_HSATLRAW_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d\n",
				i,data->time[i]);
		fprintf(stderr,"dbg5       gyro headings:\n");
		for (i=0;i<11;i++)
			fprintf(stderr,"dbg5         %d  %f\n",
				i,data->gyro[i]);
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
int mbr_hsatlraw_rd_ergnctds(verbose,mbfp,data,shift,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsatlraw_struct *data;
int	shift;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_rd_ergnctds";
	int	status = MB_SUCCESS;
	char	line[MBF_HSATLRAW_MAXLINE];
	int	i, j, k;
	int	nlines;
	int	numvals;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       shift:      %d\n",shift);
		}

	/* read event record from file */
	status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error);

	/* parse data from event record */
	if (status == MB_SUCCESS)
		{
		mb_get_double( &(data->lon),        line+shift,    12);
		mb_get_double( &(data->lat),        line+12+shift, 12);
		mb_get_int(    &(data->year),       line+24+shift,  4);
		mb_get_int(    &(data->month),      line+28+shift,  2);
		mb_get_int(    &(data->day),        line+30+shift,  2);
		mb_get_int(    &(data->hour),       line+32+shift,  2);
		mb_get_int(    &(data->minute),     line+34+shift,  2);
		mb_get_int(    &(data->second),     line+36+shift,  2);
		mb_get_double( &(data->num_vel),    line+38+shift,  2);
		}

	if (status == MB_SUCCESS)
		{

		/* figure out how many lines to read */
		if (data->num_vel > MBF_HSATLRAW_MAXVEL)
			data->num_vel = MBF_HSATLRAW_MAXVEL;
		nlines = data->num_vel/10;
		if (data->num_vel%10 > 0) nlines++;
		}

	/* read and parse data records from file */
	for (i=0;i<nlines;i++)
		{
		if ((status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error)) 
			== MB_SUCCESS)
			{
			numvals = 10;
			if (i == nlines - 1) numvals = data->num_vel%10;
			for (j=0;j<numvals;j++)
				{
				k = j + i*10;
				mb_get_double(&(data->vdepth[k]),
					line+j*11+shift,5);
				mb_get_double(&(data->velocity[k]),
					line+j*11+5+shift,6);
				}
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       num_vel:          %d\n",
			data->num_vel);
		fprintf(stderr,"dbg5       water depths and velocities:\n");
		for (i=0;i<11;i++)
			fprintf(stderr,"dbg5         %d  %f  %f\n",
				i,data->vdepth[i],data->velocity[i]);
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
int mbr_hsatlraw_rd_ergnampl(verbose,mbfp,data,shift,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsatlraw_struct *data;
int	shift;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_rd_ergnampl";
	int	status = MB_SUCCESS;
	char	line[MBF_HSATLRAW_MAXLINE];
	int	i;
	int	numvals;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       shift:      %d\n",shift);
		}

	/* read event record from file */
	status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error);

	/* parse data from event record */
	if (status == MB_SUCCESS)
		{
		mb_get_double( &(data->lon),        line+shift,    12);
		mb_get_double( &(data->lat),        line+12+shift, 12);
		mb_get_int(    &(data->year),       line+24+shift,  4);
		mb_get_int(    &(data->month),      line+28+shift,  2);
		mb_get_int(    &(data->day),        line+30+shift,  2);
		mb_get_int(    &(data->hour),       line+32+shift,  2);
		mb_get_int(    &(data->minute),     line+34+shift,  2);
		mb_get_int(    &(data->second),     line+36+shift,  2);
		data->mode[0] = line[38+shift];
		mb_get_int(    &(data->trans_strbd),line+39+shift,  3);
		mb_get_int(    &(data->trans_vert), line+42+shift,  3);
		mb_get_int(    &(data->trans_port), line+45+shift,  3);
		mb_get_int(    &(data->pulse_len_strbd),line+48+shift,2);
		mb_get_int(    &(data->pulse_len_vert),line+50+shift,2);
		mb_get_int(    &(data->pulse_len_port),line+52+shift,2);
		mb_get_int(    &(data->gain_start), line+54+shift,  2);
		mb_get_int(    &(data->r_compensation_factor),line+56+shift,2);
		mb_get_int(    &(data->compensation_start),line+58+shift,4);
		mb_get_int(    &(data->increase_start),line+62+shift,5);
		mb_get_int(    &(data->tvc_near),   line+67+shift,  2);
		mb_get_int(    &(data->tvc_far),    line+69+shift,  2);
		mb_get_int(    &(data->increase_int_near),line+71+shift,3);
		mb_get_int(    &(data->increase_int_far),line+74+shift,3);
		mb_get_int(    &(data->gain_center),line+77+shift,  1);
		mb_get_double( &(data->filter_gain),line+78+shift,  5);
		mb_get_int(    &(data->amplitude_center),line+83+shift,3);
		mb_get_int(    &(data->echo_duration_center),line+86+shift,3);
		mb_get_int(    &(data->echo_scale_center),line+89+shift,1);
		data->amplitude[29] = data->amplitude_center;
		data->echo_duration[29] = (int) data->echo_duration_center;
		}

	/* read and parse data from first data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error)) 
		== MB_SUCCESS)
		{
		for (i=0;i<8;i++)
			mb_get_int(&(data->gain[i+8]),line+i+shift,1);
		mb_get_int(&numvals,line+8+shift,2);
		if (numvals == 29)
		for (i=0;i<numvals;i++)
			mb_get_int(&(data->amplitude[i+30]),
				line+i*3+10+shift,3);
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read and parse data from second data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error)) 
		== MB_SUCCESS)
		{
		for (i=0;i<8;i++)
			mb_get_int(&(data->gain[i]),line+i+shift,1);
		mb_get_int(&numvals,line+8+shift,2);
		if (numvals == 29)
		for (i=0;i<numvals;i++)
			mb_get_int(&(data->amplitude[28-i]),
				line+i*3+10+shift,3);
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read and parse data from third data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error)) 
		== MB_SUCCESS)
		{
		for (i=0;i<8;i++)
			mb_get_int(&(data->echo_scale[i+8]),line+1+shift,1);
		mb_get_int(&numvals,line+8+shift,2);
		if (numvals == 29)
		for (i=0;i<numvals;i++)
			mb_get_int(&(data->echo_duration[i+30]),
				line+i*3+10+shift,3);
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read and parse data from fourth data record */
	if ((status = mbr_hsatlraw_read_line(verbose,mbfp,shift+7,line,error)) 
		== MB_SUCCESS)
		{
		for (i=0;i<8;i++)
			mb_get_int(&(data->echo_scale[i]),line+1+shift,1);
		mb_get_int(&numvals,line+8+shift,2);
		if (numvals == 29)
		for (i=0;i<numvals;i++)
			mb_get_int(&(data->echo_duration[28-i]),
				line+i*3+10+shift,3);
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       mode:             %c\n",
			data->mode[0]);
		fprintf(stderr,"dbg5       trans_strbd:      %d\n",
			data->trans_strbd);
		fprintf(stderr,"dbg5       trans_vert:       %d\n",
			data->trans_vert);
		fprintf(stderr,"dbg5       trans_port:       %d\n",
			data->trans_port);
		fprintf(stderr,"dbg5       pulse_len_strbd:  %d\n",
			data->pulse_len_strbd);
		fprintf(stderr,"dbg5       pulse_len_vert:   %d\n",
			data->pulse_len_vert);
		fprintf(stderr,"dbg5       pulse_len_port:   %d\n",
			data->pulse_len_port);
		fprintf(stderr,"dbg5       gain_start:       %d\n",
			data->gain_start);
		fprintf(stderr,"dbg5       r_comp_factor:    %d\n",
			data->r_compensation_factor);
		fprintf(stderr,"dbg5       comp_start:       %d\n",
			data->compensation_start);
		fprintf(stderr,"dbg5       increase_start:   %d\n",
			data->increase_start);
		fprintf(stderr,"dbg5       tvc_near:         %d\n",
			data->tvc_near);
		fprintf(stderr,"dbg5       increase_start:   %d\n",
			data->increase_start);
		fprintf(stderr,"dbg5       tvc_far:          %d\n",
			data->tvc_far);
		fprintf(stderr,"dbg5       increase_int_near:%d\n",
			data->increase_int_near);
		fprintf(stderr,"dbg5       increase_int_far: %d\n",
			data->increase_int_far);
		fprintf(stderr,"dbg5       gain_center:      %d\n",
			data->gain_center);
		fprintf(stderr,"dbg5       filter_gain:      %f\n",
			data->filter_gain);
		fprintf(stderr,"dbg5       amplitude_center: %d\n",
			data->amplitude_center);
		fprintf(stderr,"dbg5       echo_dur_center:  %d\n",
			data->echo_duration_center);
		fprintf(stderr,"dbg5       echo_scal_center: %d\n",
			data->echo_scale_center);
		fprintf(stderr,"dbg5       amplitudes and echo durations:\n");
		for (i=0;i<MBF_HSATLRAW_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->amplitude[i],data->echo_duration[i]);
		fprintf(stderr,"dbg5       gains and echo scales:\n");
		for (i=0;i<16;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->gain[i],data->echo_scale[i]);
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
int mbr_hsatlraw_rd_ldeocmnt(verbose,mbfp,data,shift,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsatlraw_struct *data;
int	shift;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_rd_ldeocmnt";
	int	status = MB_SUCCESS;
	char	line[MBF_HSATLRAW_MAXLINE];
	int	nchars;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       shift:      %d\n",shift);
		}

	/* read comment record from file */
	status = mbr_hsatlraw_read_line(verbose,mbfp,shift,line,error);

	/* copy comment into data structure */
	if (status == MB_SUCCESS)
		{
		nchars = strlen(line+shift);
		strncpy(data->comment,line+shift,nchars-1);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Value read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",
			data->comment);
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
int mbr_hsatlraw_wr_data(verbose,mbio_ptr,data_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsatlraw_struct *data;
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
	data = (struct mbf_hsatlraw_struct *) data_ptr;
	mbfp = mb_io_ptr->mbfp;

	if (data->kind == MB_DATA_RAW_LINE)
		{
		status = mbr_hsatlraw_wr_rawline(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_DATA)
		{
		status = mbr_hsatlraw_wr_ergnmess(verbose,mbfp,data,error);
		status = mbr_hsatlraw_wr_ergnslzt(verbose,mbfp,data,error);
		status = mbr_hsatlraw_wr_ergnampl(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_CALIBRATE)
		{
		status = mbr_hsatlraw_wr_ergneich(verbose,mbfp,data,error);
		status = mbr_hsatlraw_wr_ergnslzt(verbose,mbfp,data,error);
		status = mbr_hsatlraw_wr_ergnampl(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_MEAN_VELOCITY)
		{
		status = mbr_hsatlraw_wr_ergnhydi(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_VELOCITY_PROFILE)
		{
		status = mbr_hsatlraw_wr_ergnctds(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_STANDBY)
		{
		status = mbr_hsatlraw_wr_ergnpara(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_NAV_SOURCE)
		{
		status = mbr_hsatlraw_wr_ergnposi(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_COMMENT)
		{
		status = mbr_hsatlraw_wr_ldeocmnt(verbose,mbfp,data,error);
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
int	mbr_hsatlraw_wr_label(verbose,mbfp,type,error)
int	verbose;
FILE	*mbfp;
char	type;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_wr_label";
	int	status = MB_SUCCESS;
	char	line[MBF_HSATLRAW_MAXLINE];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       type:       %d\n",type);
		}

	/* write label in file */
	sprintf(line,"%8s\n",mbf_hsatlraw_labels[type]);
	status = mbr_hsatlraw_write_line(verbose,mbfp,line,error);

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
int	mbr_hsatlraw_write_line(verbose,mbfp,line,error)
int	verbose;
FILE	*mbfp;
char	*line;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_write_line";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       line:       %s\n",line);
		}

	/* write next line in file */
	if ((status = fputs(line,mbfp)) != EOF)
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}
	else
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
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
int mbr_hsatlraw_wr_rawline(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_wr_rawline";
	int	status = MB_SUCCESS;
	struct mbf_hsatlraw_struct *data;

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
	data = (struct mbf_hsatlraw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       raw line:         %s\n",
			data->comment);
		}

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the line */
		status = fprintf(mbfp,"%s\n",data->comment);
		if (status >= 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergnhydi(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_wr_ergnhydi";
	int	status = MB_SUCCESS;
	struct mbf_hsatlraw_struct *data;

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
	data = (struct mbf_hsatlraw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       alt minute:       %d\n",
			data->alt_minute);
		fprintf(stderr,"dbg5       alt second:       %d\n",
			data->alt_second);
		fprintf(stderr,"dbg5       draught:          %f\n",
			data->draught);
		fprintf(stderr,"dbg5       mean velocity:    %f\n",
			data->vel_mean);
		fprintf(stderr,"dbg5       keel velocity:    %f\n",
			data->vel_keel);
		fprintf(stderr,"dbg5       tide:             %f\n",data->tide);
		}

	/* write the record label */
	status = mbr_hsatlraw_wr_label(verbose,mbfp,
		MBF_HSATLRAW_ERGNHYDI,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the event line */
		status = fprintf(mbfp,"%+12.7f",data->lon);
		status = fprintf(mbfp,"%+12.7f",data->lat);
		status = fprintf(mbfp,"%4.4d",data->year);
		status = fprintf(mbfp,"%2.2d",data->month);
		status = fprintf(mbfp,"%2.2d",data->day);
		status = fprintf(mbfp,"%2.2d",data->hour);
		status = fprintf(mbfp,"%2.2d",data->minute);
		status = fprintf(mbfp,"%2.2d",data->second);
		status = fprintf(mbfp,"%5d",data->alt_minute);
		status = fprintf(mbfp,"%2d",data->alt_second);
		status = fprintf(mbfp,"%4.1f",data->draught);
		status = fprintf(mbfp,"%7.2f",data->vel_mean);
		status = fprintf(mbfp,"%7.2f",data->vel_keel);
		status = fprintf(mbfp,"%+06.2f\n",data->tide);

		/* check for an error */
		if (status > 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergnpara(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_wr_ergnpara";
	int	status = MB_SUCCESS;
	struct mbf_hsatlraw_struct *data;

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
	data = (struct mbf_hsatlraw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       alt minute:       %d\n",
			data->alt_minute);
		fprintf(stderr,"dbg5       alt second:       %d\n",
			data->alt_second);
		fprintf(stderr,"dbg5       course_true:      %f\n",
			data->course_true);
		fprintf(stderr,"dbg5       speed_transverse: %f\n",
			data->speed_transverse);
		fprintf(stderr,"dbg5       speed:            %f\n",
			data->speed);
		fprintf(stderr,"dbg5       speed_reference:  %c\n",
			data->speed_reference[0]);
		fprintf(stderr,"dbg5       pitch:            %f\n",
			data->pitch);
		fprintf(stderr,"dbg5       track:            %d\n",
			data->track);
		fprintf(stderr,"dbg5       depth_center:     %f\n",
			data->depth_center);
		}

	/* write the record label */
	status = mbr_hsatlraw_wr_label(verbose,mbfp,
		MBF_HSATLRAW_ERGNPARA,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the event line */
		status = fprintf(mbfp,"%+12.7f",data->lon);
		status = fprintf(mbfp,"%+12.7f",data->lat);
		status = fprintf(mbfp,"%4.4d",data->year);
		status = fprintf(mbfp,"%2.2d",data->month);
		status = fprintf(mbfp,"%2.2d",data->day);
		status = fprintf(mbfp,"%2.2d",data->hour);
		status = fprintf(mbfp,"%2.2d",data->minute);
		status = fprintf(mbfp,"%2.2d",data->second);
		status = fprintf(mbfp,"%5d",data->alt_minute);
		status = fprintf(mbfp,"%2d",data->alt_second);
		status = fprintf(mbfp,"%5.1f",data->course_true);
		status = fprintf(mbfp,"%+9.1f",data->speed_transverse);
		status = fprintf(mbfp,"%+9.1f",data->speed);
		status = fprintf(mbfp,"%c",data->speed_reference[0]);
		status = fprintf(mbfp,"%+4.1f",data->pitch);
		status = fprintf(mbfp,"%4.4d",data->track);
		status = fprintf(mbfp,"%7.1f\n",data->depth);

		/* check for an error */
		if (status > 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergnposi(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_wr_ergnposi";
	int	status = MB_SUCCESS;
	struct mbf_hsatlraw_struct *data;

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
	data = (struct mbf_hsatlraw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       alt minute:       %d\n",
			data->alt_minute);
		fprintf(stderr,"dbg5       alt second:       %d\n",
			data->alt_second);
		fprintf(stderr,"dbg5       pos_corr_x:       %f\n",
			data->pos_corr_x);
		fprintf(stderr,"dbg5       pos_corr_y:       %f\n",
			data->pos_corr_y);
		fprintf(stderr,"dbg5       sensors:          %s\n",
			data->sensors);
		}

	/* write the record label */
	status = mbr_hsatlraw_wr_label(verbose,mbfp,
		MBF_HSATLRAW_ERGNPOSI,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the event line */
		status = fprintf(mbfp,"%+12.7f",data->lon);
		status = fprintf(mbfp,"%+12.7f",data->lat);
		status = fprintf(mbfp,"%4.4d",data->year);
		status = fprintf(mbfp,"%2.2d",data->month);
		status = fprintf(mbfp,"%2.2d",data->day);
		status = fprintf(mbfp,"%2.2d",data->hour);
		status = fprintf(mbfp,"%2.2d",data->minute);
		status = fprintf(mbfp,"%2.2d",data->second);
		status = fprintf(mbfp,"%5d",data->alt_minute);
		status = fprintf(mbfp,"%2d",data->alt_second);
		status = fprintf(mbfp,"%7.0f",data->pos_corr_x);
		status = fprintf(mbfp,"%7.0f",data->pos_corr_y);
		status = fprintf(mbfp,"%8s\n",data->sensors);

		/* check for an error */
		if (status > 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergneich(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_wr_ergneich";
	int	status = MB_SUCCESS;
	struct mbf_hsatlraw_struct *data;
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
	data = (struct mbf_hsatlraw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       alt minute:       %d\n",
			data->alt_minute);
		fprintf(stderr,"dbg5       alt second:       %d\n",
			data->alt_second);
		fprintf(stderr,"dbg5       course_true:      %f\n",
			data->course_true);
		fprintf(stderr,"dbg5       speed_transverse: %f\n",
			data->speed_transverse);
		fprintf(stderr,"dbg5       speed:            %f\n",
			data->speed);
		fprintf(stderr,"dbg5       speed_reference:  %c\n",
			data->speed_reference[0]);
		fprintf(stderr,"dbg5       pitch:            %f\n",
			data->pitch);
		fprintf(stderr,"dbg5       track:            %d\n",
			data->track);
		fprintf(stderr,"dbg5       depth_center:     %f\n",
			data->depth_center);
		fprintf(stderr,"dbg5       depth_scale:      %f\n",
			data->depth_scale);
		fprintf(stderr,"dbg5       spare:            %d\n",
			data->spare);
		fprintf(stderr,"dbg5       distances and depths:\n");
		for (i=0;i<MBF_HSATLRAW_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->distance[i],data->depth[i]);
		}

	/* write the record label */
	status = mbr_hsatlraw_wr_label(verbose,mbfp,
		MBF_HSATLRAW_ERGNEICH,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the event line */
		status = fprintf(mbfp,"%+12.7f",data->lon);
		status = fprintf(mbfp,"%+12.7f",data->lat);
		status = fprintf(mbfp,"%4.4d",data->year);
		status = fprintf(mbfp,"%2.2d",data->month);
		status = fprintf(mbfp,"%2.2d",data->day);
		status = fprintf(mbfp,"%2.2d",data->hour);
		status = fprintf(mbfp,"%2.2d",data->minute);
		status = fprintf(mbfp,"%2.2d",data->second);
		status = fprintf(mbfp,"%5d",data->alt_minute);
		status = fprintf(mbfp,"%2d",data->alt_second);
		status = fprintf(mbfp,"%5.1f",data->course_true);
		status = fprintf(mbfp,"%+9.1f",data->speed_transverse);
		status = fprintf(mbfp,"%+9.1f",data->speed);
		status = fprintf(mbfp,"%c",data->speed_reference[0]);
		status = fprintf(mbfp,"%+4.1f",data->pitch);
		status = fprintf(mbfp,"%4.4d",data->track);
		status = fprintf(mbfp,"%7.1f",data->depth_center);
		status = fprintf(mbfp,"%4.2f",data->depth_scale);
		status = fprintf(mbfp,"%2d\n",data->spare);

		/* output forward distances */
		status = fprintf(mbfp,"29");
		for (i=0;i<29;i++)
			status = fprintf(mbfp,"%4.4d",data->distance[i+30]);
		status = fprintf(mbfp,"\n");

		/* output forward depths */
		status = fprintf(mbfp,"29");
		for (i=0;i<29;i++)
			status = fprintf(mbfp,"%4.4d",data->depth[i+30]);
		status = fprintf(mbfp,"\n");

		/* output aft distances */
		status = fprintf(mbfp,"29");
		for (i=0;i<29;i++)
			status = fprintf(mbfp,"%4.4d",(-data->distance[28-i]));
		status = fprintf(mbfp,"\n");

		/* output aft depths */
		status = fprintf(mbfp,"29");
		for (i=0;i<29;i++)
			status = fprintf(mbfp,"%4.4d",data->depth[28-i]);
		status = fprintf(mbfp,"\n");

		/* check for an error */
		if (status > 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergnmess(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_wr_ergnmess";
	int	status = MB_SUCCESS;
	struct mbf_hsatlraw_struct *data;
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
	data = (struct mbf_hsatlraw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       alt minute:       %d\n",
			data->alt_minute);
		fprintf(stderr,"dbg5       alt second:       %d\n",
			data->alt_second);
		fprintf(stderr,"dbg5       course_true:      %f\n",
			data->course_true);
		fprintf(stderr,"dbg5       speed_transverse: %f\n",
			data->speed_transverse);
		fprintf(stderr,"dbg5       speed:            %f\n",
			data->speed);
		fprintf(stderr,"dbg5       speed_reference:  %c\n",
			data->speed_reference[0]);
		fprintf(stderr,"dbg5       pitch:            %f\n",
			data->pitch);
		fprintf(stderr,"dbg5       track:            %d\n",
			data->track);
		fprintf(stderr,"dbg5       depth_center:     %f\n",
			data->depth_center);
		fprintf(stderr,"dbg5       depth_scale:      %f\n",
			data->depth_scale);
		fprintf(stderr,"dbg5       spare:            %d\n",
			data->spare);
		fprintf(stderr,"dbg5       distances and depths:\n");
		for (i=0;i<MBF_HSATLRAW_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->distance[i],data->depth[i]);
		}

	/* write the record label */
	status = mbr_hsatlraw_wr_label(verbose,mbfp,
		MBF_HSATLRAW_ERGNMESS,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the event line */
		status = fprintf(mbfp,"%+12.7f",data->lon);
		status = fprintf(mbfp,"%+12.7f",data->lat);
		status = fprintf(mbfp,"%4.4d",data->year);
		status = fprintf(mbfp,"%2.2d",data->month);
		status = fprintf(mbfp,"%2.2d",data->day);
		status = fprintf(mbfp,"%2.2d",data->hour);
		status = fprintf(mbfp,"%2.2d",data->minute);
		status = fprintf(mbfp,"%2.2d",data->second);
		status = fprintf(mbfp,"%5d",data->alt_minute);
		status = fprintf(mbfp,"%2d",data->alt_second);
		status = fprintf(mbfp,"%5.1f",data->course_true);
		status = fprintf(mbfp,"%+9.1f",data->speed_transverse);
		status = fprintf(mbfp,"%+9.1f",data->speed);
		status = fprintf(mbfp,"%c",data->speed_reference[0]);
		status = fprintf(mbfp,"%+4.1f",data->pitch);
		status = fprintf(mbfp,"%4.4d",data->track);
		status = fprintf(mbfp,"%7.1f",data->depth_center);
		status = fprintf(mbfp,"%4.2f",data->depth_scale);
		status = fprintf(mbfp,"%2d\n",data->spare);

		/* output starboard crosstrack distances */
		status = fprintf(mbfp,"29");
		for (i=0;i<29;i++)
			status = fprintf(mbfp,"%4.4d",data->distance[i+30]);
		status = fprintf(mbfp,"\n");

		/* output starboard crosstrack depths */
		status = fprintf(mbfp,"29");
		for (i=0;i<29;i++)
			status = fprintf(mbfp,"%4.4d",data->depth[i+30]);
		status = fprintf(mbfp,"\n");

		/* output port crosstrack distances */
		status = fprintf(mbfp,"29");
		for (i=0;i<29;i++)
			status = fprintf(mbfp,"%4.4d",(-data->distance[28-i]));
		status = fprintf(mbfp,"\n");

		/* output port crosstrack depths */
		status = fprintf(mbfp,"29");
		for (i=0;i<29;i++)
			status = fprintf(mbfp,"%4.4d",data->depth[28-i]);
		status = fprintf(mbfp,"\n");

		/* check for an error */
		if (status > 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergnslzt(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_wr_ergnslzt";
	int	status = MB_SUCCESS;
	struct mbf_hsatlraw_struct *data;
	int	datacheck;
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
	data = (struct mbf_hsatlraw_struct *) data_ptr;

	/* check if there is data to output */
	datacheck = MB_NO;
	for (i=0;i<MBF_HSATLRAW_BEAMS;i++)
		if (data->time[i] > 0)
			datacheck = MB_YES;

	/* print debug statements */
	if (verbose >= 5 && datacheck == MB_NO)
		{
		fprintf(stderr,"\ndbg5  No values to be written in MBIO function <%s>\n",
			function_name);
		}

	/* print debug statements */
	if (verbose >= 5 && datacheck == MB_YES)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       course_true:      %f\n",
			data->course_true);
		fprintf(stderr,"dbg5       course_ground:    %f\n",
			data->course_ground);
		fprintf(stderr,"dbg5       speed_ground:     %f\n",
			data->speed_ground);
		fprintf(stderr,"dbg5       heave:            %f\n",
			data->heave);
		fprintf(stderr,"dbg5       pitch:            %f\n",
			data->pitch);
		fprintf(stderr,"dbg5       roll:             %f\n",
			data->roll);
		fprintf(stderr,"dbg5       time_center:      %f\n",
			data->depth_center);
		fprintf(stderr,"dbg5       time_scale:       %f\n",
			data->time_scale);
		fprintf(stderr,"dbg5       travel times:\n");
		for (i=0;i<MBF_HSATLRAW_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d\n",
				i,data->time[i]);
		fprintf(stderr,"dbg5       gyro headings:\n");
		for (i=0;i<11;i++)
			fprintf(stderr,"dbg5         %d  %d\n",
				i,data->gyro[i]);
		}

	/* write the record label */
	if (datacheck == MB_YES)
		status = mbr_hsatlraw_wr_label(verbose,mbfp,
			MBF_HSATLRAW_ERGNSLZT,error);

	/* write out the data */
	if (status == MB_SUCCESS && datacheck == MB_YES)
		{
		/* output the event line */
		status = fprintf(mbfp,"%+12.7f",data->lon);
		status = fprintf(mbfp,"%+12.7f",data->lat);
		status = fprintf(mbfp,"%4.4d",data->year);
		status = fprintf(mbfp,"%2.2d",data->month);
		status = fprintf(mbfp,"%2.2d",data->day);
		status = fprintf(mbfp,"%2.2d",data->hour);
		status = fprintf(mbfp,"%2.2d",data->minute);
		status = fprintf(mbfp,"%2.2d",data->second);
		status = fprintf(mbfp,"%5.1f",data->course_true);
		status = fprintf(mbfp,"%5.1f",data->course_ground);
		status = fprintf(mbfp,"%+9.1f",data->speed_ground);
		status = fprintf(mbfp,"%+6.2f",data->heave);
		status = fprintf(mbfp,"%+4.1f",data->pitch);
		status = fprintf(mbfp,"%+5.1f",data->roll);
		status = fprintf(mbfp,"%06.0f",data->time_center);
		status = fprintf(mbfp,"%6.4f\n",data->time_scale);

		/* output starboard crosstrack travel times */
		status = fprintf(mbfp,"29");
		for (i=0;i<29;i++)
			status = fprintf(mbfp,"%4.4d",data->time[i+30]);
		status = fprintf(mbfp,"\n");

		/* output port crosstrack travel times */
		status = fprintf(mbfp,"29");
		for (i=0;i<29;i++)
			status = fprintf(mbfp,"%4.4d",data->time[28-i]);
		status = fprintf(mbfp,"\n");

		/* output gyro headings */
		for (i=0;i<11;i++)
			status = fprintf(mbfp,"%05.1f",data->gyro[i]);
		status = fprintf(mbfp,"\n");

		/* check for an error */
		if (status > 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergnctds(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_wr_ergnctds";
	int	status = MB_SUCCESS;
	struct mbf_hsatlraw_struct *data;
	int	i;
	int	nline, nrem;

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
	data = (struct mbf_hsatlraw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       num_vel:          %d\n",
			data->num_vel);
		fprintf(stderr,"dbg5       water depths and velocities:\n");
		for (i=0;i<11;i++)
			fprintf(stderr,"dbg5         %d  %f  %f\n",
				i,data->vdepth[i],data->velocity[i]);
		}

	/* write the record label */
	status = mbr_hsatlraw_wr_label(verbose,mbfp,
		MBF_HSATLRAW_ERGNCTDS,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the event line */
		status = fprintf(mbfp,"%+12.7f",data->lon);
		status = fprintf(mbfp,"%+12.7f",data->lat);
		status = fprintf(mbfp,"%4.4d",data->year);
		status = fprintf(mbfp,"%2.2d",data->month);
		status = fprintf(mbfp,"%2.2d",data->day);
		status = fprintf(mbfp,"%2.2d",data->hour);
		status = fprintf(mbfp,"%2.2d",data->minute);
		status = fprintf(mbfp,"%2.2d",data->second);
		status = fprintf(mbfp,"%2d\n",data->num_vel);

		/* figure number of velocity lines to write */
		nline = data->num_vel/10;
		nrem = data->num_vel%10;

		/* write all of the full lines */
		for (i=0;i<nline;i++)
			{
			for (i=0;i<10;i++)
				status = fprintf(mbfp,"%5.0f%6.1f",
					data->depth[i],data->velocity[i]);
			status = fprintf(mbfp,"\n");
			}

		/* write the last line as needed */
		if (nrem > 0)
			{
			for (i=0;i<nrem;i++)
				status = fprintf(mbfp,"%5.0f%6.1f",
					data->depth[i],data->velocity[i]);
			for (i=0;i<(10-nrem);i++)
				status = fprintf(mbfp,"           ");
			status = fprintf(mbfp,"\n");
			}

		/* check for an error */
		if (status > 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergnampl(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_wr_ergnampl";
	int	status = MB_SUCCESS;
	struct mbf_hsatlraw_struct *data;
	int	datacheck;
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
	data = (struct mbf_hsatlraw_struct *) data_ptr;

        /* check if there is data to output */
	datacheck = MB_NO;
	for (i=0;i<MBF_HSATLRAW_BEAMS;i++)
		if (data->amplitude[i] > 0)
			datacheck = MB_YES;

	/* print debug statements */
	if (verbose >= 5 && datacheck == MB_NO)
		{
		fprintf(stderr,"\ndbg5  No values to be written in MBIO function <%s>\n",
			function_name);
		}

	/* print debug statements */
	if (verbose >= 5 && datacheck == MB_YES)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->lat);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       second:           %d\n",
			data->second);
		fprintf(stderr,"dbg5       mode:             %c\n",
			data->mode[0]);
		fprintf(stderr,"dbg5       trans_strbd:      %d\n",
			data->trans_strbd);
		fprintf(stderr,"dbg5       trans_vert:       %d\n",
			data->trans_vert);
		fprintf(stderr,"dbg5       trans_port:       %d\n",
			data->trans_port);
		fprintf(stderr,"dbg5       pulse_len_strbd:  %d\n",
			data->pulse_len_strbd);
		fprintf(stderr,"dbg5       pulse_len_vert:   %d\n",
			data->pulse_len_vert);
		fprintf(stderr,"dbg5       pulse_len_port:   %d\n",
			data->pulse_len_port);
		fprintf(stderr,"dbg5       gain_start:       %d\n",
			data->gain_start);
		fprintf(stderr,"dbg5       r_comp_factor:    %d\n",
			data->r_compensation_factor);
		fprintf(stderr,"dbg5       comp_start:       %d\n",
			data->compensation_start);
		fprintf(stderr,"dbg5       increase_start:   %d\n",
			data->increase_start);
		fprintf(stderr,"dbg5       tvc_near:         %d\n",
			data->tvc_near);
		fprintf(stderr,"dbg5       increase_start:   %d\n",
			data->increase_start);
		fprintf(stderr,"dbg5       tvc_far:          %d\n",
			data->tvc_far);
		fprintf(stderr,"dbg5       increase_int_near:%d\n",
			data->increase_int_near);
		fprintf(stderr,"dbg5       increase_int_far: %d\n",
			data->increase_int_far);
		fprintf(stderr,"dbg5       gain_center:      %d\n",
			data->gain_center);
		fprintf(stderr,"dbg5       filter_gain:      %f\n",
			data->filter_gain);
		fprintf(stderr,"dbg5       amplitude_center: %d\n",
			data->amplitude_center);
		fprintf(stderr,"dbg5       echo_dur_center:  %d\n",
			data->echo_duration_center);
		fprintf(stderr,"dbg5       echo_scal_center: %d\n",
			data->echo_scale_center);
		fprintf(stderr,"dbg5       amplitudes and echo durations:\n");
		for (i=0;i<MBF_HSATLRAW_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->amplitude[i],data->echo_duration[i]);
		fprintf(stderr,"dbg5       gains and echo scales:\n");
		for (i=0;i<16;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->gain[i],data->echo_scale[i]);
		}

	/* write the record label */
	if (datacheck == MB_YES)
		status = mbr_hsatlraw_wr_label(verbose,mbfp,
			MBF_HSATLRAW_ERGNAMPL,error);

	/* write out the data */
	if (status == MB_SUCCESS && datacheck == MB_YES)
		{
		/* output the event line */
		status = fprintf(mbfp,"%+12.7f",data->lon);
		status = fprintf(mbfp,"%+12.7f",data->lat);
		status = fprintf(mbfp,"%4.4d",data->year);
		status = fprintf(mbfp,"%2.2d",data->month);
		status = fprintf(mbfp,"%2.2d",data->day);
		status = fprintf(mbfp,"%2.2d",data->hour);
		status = fprintf(mbfp,"%2.2d",data->minute);
		status = fprintf(mbfp,"%2.2d",data->second);
		status = fprintf(mbfp,"%c",data->mode[0]);
		status = fprintf(mbfp,"%3.3d",data->trans_strbd);
		status = fprintf(mbfp,"%3.3d",data->trans_vert);
		status = fprintf(mbfp,"%3.3d",data->trans_port);
		status = fprintf(mbfp,"%2.2d",data->pulse_len_strbd);
		status = fprintf(mbfp,"%2.2d",data->pulse_len_vert);
		status = fprintf(mbfp,"%2.2d",data->pulse_len_port);
		status = fprintf(mbfp,"%2.2d",data->gain_start);
		status = fprintf(mbfp,"%2.2d",data->r_compensation_factor);
		status = fprintf(mbfp,"%4.4d",data->compensation_start);
		status = fprintf(mbfp,"%5.5d",data->increase_start);
		status = fprintf(mbfp,"%2.2d",data->tvc_near);
		status = fprintf(mbfp,"%2.2d",data->tvc_far);
		status = fprintf(mbfp,"%3.3d",data->increase_int_near);
		status = fprintf(mbfp,"%3.3d",data->increase_int_far);
		status = fprintf(mbfp,"%1d",data->gain_center);
		status = fprintf(mbfp,"%+5.1f",data->filter_gain);
		status = fprintf(mbfp,"%3.3d",data->amplitude_center);
		status = fprintf(mbfp,"%3.3d",data->echo_duration_center);
		status = fprintf(mbfp,"%1d\n",data->echo_scale_center);

		/* output starboard amplitudes */
		for (i=0;i<8;i++)
			status = fprintf(mbfp,"%1.1d",data->gain[i+8]);
		status = fprintf(mbfp,"29");
		for (i=0;i<29;i++)
			status = fprintf(mbfp,"%3.3d",data->amplitude[i+30]);
		status = fprintf(mbfp,"\n");

		/* output port amplitudes */
		for (i=0;i<8;i++)
			status = fprintf(mbfp,"%1.1d",data->gain[i]);
		status = fprintf(mbfp,"29");
		for (i=0;i<29;i++)
			status = fprintf(mbfp,"%3.3d",data->amplitude[28-i]);
		status = fprintf(mbfp,"\n");

		/* output starboard echo durations */
		for (i=0;i<8;i++)
			status = fprintf(mbfp,"%1.1d",data->echo_scale[i+8]);
		status = fprintf(mbfp,"29");
		for (i=0;i<29;i++)
			status = fprintf(mbfp,"%3.3d",
				data->echo_duration[i+30]);
		status = fprintf(mbfp,"\n");

		/* output port echo durations */
		for (i=0;i<8;i++)
			status = fprintf(mbfp,"%1.1d",data->echo_scale[i]);
		status = fprintf(mbfp,"29");
		for (i=0;i<29;i++)
			status = fprintf(mbfp,"%3.3d",
				data->echo_duration[28-i]);
		status = fprintf(mbfp,"\n");

		/* check for an error */
		if (status > 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ldeocmnt(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_hsatlraw_wr_ldeocmnt";
	int	status = MB_SUCCESS;
	struct mbf_hsatlraw_struct *data;

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
	data = (struct mbf_hsatlraw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",
			data->comment);
		}

	/* write the record label */
	status = mbr_hsatlraw_wr_label(verbose,mbfp,
		MBF_HSATLRAW_LDEOCMNT,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the event line */
		status = fprintf(mbfp,"%s\n",data->comment);

		/* check for an error */
		if (status >= 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
