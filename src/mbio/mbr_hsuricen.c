/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hsuricen.c	2/2/93
 *	$Id: mbr_hsuricen.c,v 4.3 1994-10-21 12:20:01 caress Exp $
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
 * mbr_hsuricen.c contains the functions for reading and writing
 * multibeam data in the HSURICEN format.  
 * These functions include:
 *   mbr_alm_hsuricen	- allocate read/write memory
 *   mbr_dem_hsuricen	- deallocate read/write memory
 *   mbr_rt_hsuricen	- read and translate data
 *   mbr_wt_hsuricen	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
 * $Log: not supported by cvs2svn $
 * Revision 4.2  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.1  1994/05/21  02:23:29  caress
 * Made sure that mb_io_ptr->new_bath_alongtrack is set to zero on reading.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/21  03:59:50  caress
 * First cut at new version. Altered to be consistent
 * with passing of three types of data: bathymetry,
 * amplitude, and sidescan.
 *
 * Revision 3.0  1993/05/14  22:56:29  sohara
 * initial version
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mbsys_hsds.h"
#include "../../include/mbf_hsuricen.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/*--------------------------------------------------------------------*/
int mbr_alm_hsuricen(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_hsuricen.c,v 4.3 1994-10-21 12:20:01 caress Exp $";
	char	*function_name = "mbr_alm_hsuricen";
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
	mb_io_ptr->structure_size = sizeof(struct mbf_hsuricen_struct);
	mb_io_ptr->data_structure_size = 
		sizeof(struct mbf_hsuricen_data_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_hsds_struct),
				&mb_io_ptr->store_data,error);

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
int mbr_dem_hsuricen(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_hsuricen";
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
int mbr_rt_hsuricen(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_hsuricen";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsuricen_struct *dataplus;
	struct mbf_hsuricen_data_struct *data;
	struct mbsys_hsds_struct *store;
	char	*datacomment;
	int	time_j[5];
	int	i, j, k;
	int	id;
	double	scale;

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
	dataplus = (struct mbf_hsuricen_struct *) mb_io_ptr->raw_data;
	data = &(dataplus->data);
	datacomment = (char *) data;
	dataplus->kind = MB_DATA_DATA;
	store = (struct mbsys_hsds_struct *) store_ptr;

	/* read next record from file */
	if ((status = fread(data,1,mb_io_ptr->data_structure_size,
			mb_io_ptr->mbfp)) == mb_io_ptr->data_structure_size) 
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS && data->sec != 25443)
		{
		data->sec = mb_swap_short(data->sec);
		data->min = mb_swap_short(data->min);
		data->day = mb_swap_short(data->day);
		data->year = mb_swap_short(data->year);
		data->lat = mb_swap_long(data->lat);
		data->lon = mb_swap_long(data->lon);
		data->hdg = mb_swap_short(data->hdg);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->pitch = mb_swap_short(data->pitch);
		data->scale = mb_swap_short(data->scale);
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			data->dist[i] = mb_swap_short(data->dist[i]);
			data->deph[i] = mb_swap_short(data->deph[i]);
			}
		}
#endif

	/* check for comment or unintelligible records */
	if (status == MB_SUCCESS)
		{
		if (data->sec == 25443)
			{
			dataplus->kind = MB_DATA_COMMENT;
			}
		else if (data->year == 0)
			{
			dataplus->kind = MB_DATA_NONE;
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		else
			{
			dataplus->kind = MB_DATA_DATA;
			}
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = dataplus->kind;
	mb_io_ptr->new_error = *error;

	/* translate values to current ping variables 
		in mbio descriptor structure */
	if (status == MB_SUCCESS 
		&& dataplus->kind == MB_DATA_DATA)
		{
		/* get time */
		time_j[0] = data->year;
		time_j[1] = data->day;
		time_j[2] = data->min;
		time_j[3] = data->sec/100;
		time_j[4] = 10000*(data->sec - 100*time_j[3]);
		mb_get_itime(verbose,time_j,mb_io_ptr->new_time_i);
		mb_get_time(verbose,mb_io_ptr->new_time_i,
			&(mb_io_ptr->new_time_d));

		/* get navigation */
		mb_io_ptr->new_lon = 0.0000001*data->lon;
		mb_io_ptr->new_lat = 0.0000001*data->lat;
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
		mb_io_ptr->new_heading = 0.1*data->hdg;

		/* get speed (convert 100Xnm/hr to km/hr) */
		mb_io_ptr->new_speed = 0.018333333*data->speed;

		/* read distance and depth values into storage arrays */
		/* switch order of data as it is read into the global arrays */
		id = mb_io_ptr->beams_bath - 1;
		if (data->scale != 100 && data->scale < 0)
			{
			scale = 0.01*data->scale;
			for (i=0;i<mb_io_ptr->beams_bath;i++)
				{
				mb_io_ptr->new_bath[id-i] 
					= scale*data->deph[i];
				mb_io_ptr->new_bath_acrosstrack[id-i] 
					= scale*data->dist[i];
				mb_io_ptr->new_bath_alongtrack[id-i] = 0.0;
				}
			}
		else
			{
			for (i=0;i<mb_io_ptr->beams_bath;i++)
				{
				mb_io_ptr->new_bath[id-i] = data->deph[i];
				mb_io_ptr->new_bath_acrosstrack[id-i] 
					= data->dist[i];
				mb_io_ptr->new_bath_alongtrack[id-i] = 0.0;
				}
			}

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				mb_io_ptr->new_error);
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
			fprintf(stderr,"dbg4       speed:      %f\n",
				mb_io_ptr->new_speed);
			fprintf(stderr,"dbg4       heading:    %f\n",
				mb_io_ptr->new_heading);
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

		/* done translating values */

		}
	else if (status == MB_SUCCESS 
		&& dataplus->kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(mb_io_ptr->new_comment,&datacomment[2],253);

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
		store->kind = dataplus->kind;

		/* position (all records ) */
		store->lon = mb_io_ptr->new_lon;
		store->lat = mb_io_ptr->new_lat;
		if (store->lon > 180.) 
			store->lon = store->lon - 360.;
		else if (store->lon < -180.)
			store->lon = store->lon + 360.;

		/* time stamp (all records ) */
		store->year = mb_io_ptr->new_time_i[0];
		store->month = mb_io_ptr->new_time_i[1];
		store->day = mb_io_ptr->new_time_i[2];
		store->hour = mb_io_ptr->new_time_i[3];
		store->minute = mb_io_ptr->new_time_i[4];
		store->second = mb_io_ptr->new_time_i[5];
		store->alt_minute = 0;
		store->alt_second = 0;

		/* additional navigation and depths (ERGNMESS and ERGNEICH) */
		store->course_true = mb_io_ptr->new_heading;
		store->speed_transverse = 0.0;
		store->speed = 0.005092593*data->speed;
		store->speed_reference[0] = data->speed_ref;
		store->pitch = 0.1*data->pitch;
		store->track = 0;
		store->depth_center 
			= mb_io_ptr->new_bath[mb_io_ptr->beams_bath/2];
		store->depth_scale = 0.01*data->scale;
		store->spare = 1;
		id = MBSYS_HSDS_BEAMS - 1;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			store->distance[id-i] = data->dist[i];
			store->depth[id-i] = data->deph[i];
			}

		/* travel time data (ERGNSLZT) */
		store->course_ground = 0.1*data->course;
		store->speed_ground = 0.0;
		store->heave = 0.0;
		store->roll = 0.0;
		store->time_center = 0.0;
		store->time_scale = 0.0;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			store->time[i] = 0;
		for (i=0;i<11;i++)
			store->gyro[i] = 0.0;

		/* amplitude data (ERGNAMPL) */
		store->mode[0] = '\0';
		store->trans_strbd = 0;
		store->trans_vert = 0;
		store->trans_port = 0;
		store->pulse_len_strbd = 0;
		store->pulse_len_vert = 0;
		store->pulse_len_port = 0;
		store->gain_start = 0;
		store->r_compensation_factor = 0;
		store->compensation_start = 0;
		store->increase_start = 0;
		store->tvc_near = 0;
		store->tvc_far = 0;
		store->increase_int_near = 0;
		store->increase_int_far = 0;
		store->gain_center = 0;
		store->filter_gain = 0.0;
		store->amplitude_center = 0;
		store->echo_duration_center = 0;
		store->echo_scale_center = 0;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			store->amplitude[i] = 0;
			store->echo_duration[i] = 0;
			}
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			store->gain[i] = 0;
			store->echo_scale[i] = 0;
			}

		/* mean velocity (ERGNHYDI) */
		store->draught = 0.0;
		store->vel_mean = 0.0;
		store->vel_keel = 0.0;
		store->tide = 0.0;

		/* water velocity profile (HS_ERGNCTDS) */
		store->num_vel = 0.0;

		/* navigation source (ERGNPOSI) */
		store->pos_corr_x = 0.0;
		store->pos_corr_y = 0.0;
		strncpy(store->sensors,"\0",8);

		/* comment (LDEOCMNT) */
		strncpy(store->comment,mb_io_ptr->new_comment,
			MBSYS_HSDS_MAXLINE);

		/* processed backscatter */
		store->back_scale = 0.0;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			store->back[i] = 0;
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
int mbr_wt_hsuricen(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_hsuricen";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsuricen_struct *dataplus;
	struct mbf_hsuricen_data_struct *data;
	struct mbsys_hsds_struct *store;
	char	*datacomment;
	int	time_i[7];
	int	time_j[5];
	double	lon, lat;
	int	i, j;
	int	id;

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
	dataplus = (struct mbf_hsuricen_struct *) mb_io_ptr->raw_data;
	data = &(dataplus->data);
	datacomment = (char *) data;
	store = (struct mbsys_hsds_struct *) store_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Status at beginning of MBIO function <%s>\n",
			function_name);
		if (store != NULL)
			fprintf(stderr,"dbg5       store->kind:    %d\n",
				store->kind);
		fprintf(stderr,"dbg5       new_kind:       %d\n",
			mb_io_ptr->new_kind);
		fprintf(stderr,"dbg5       new_error:      %d\n",
			mb_io_ptr->new_error);
		fprintf(stderr,"dbg5       error:          %d\n",*error);
		fprintf(stderr,"dbg5       status:         %d\n",status);
		}

	/* first set some plausible amounts for some of the 
		variables in the HSURICEN record */
	data->course = 0;
	data->pitch = 0;
	data->scale = 100;	/* this is a unit scale factor */
	data->speed_ref = 'B';	/* assume speed is over the ground */
	data->quality = 0;

	/* second translate values from hydrosweep data storage structure */
	if (store != NULL)
		{
		dataplus->kind = store->kind;
		if (store->kind == MB_DATA_DATA)
			{
			/* position */
			if (store->lon < -180.0)
				store->lon = store->lon + 360.0;
			if (store->lon > 180.0)
				store->lon = store->lon - 360.0;
			data->lon = (int)(0.5 + 10000000.0*store->lon);
			data->lat = (int)(0.5 + 10000000.0*store->lat);

			/* time stamp */
			time_i[0] = store->year;
			time_i[1] = store->month;
			time_i[2] = store->day;
			time_i[3] = store->hour;
			time_i[4] = store->minute;
			time_i[5] = store->second;
			time_i[6] = 0;
			mb_get_jtime(verbose,time_i,time_j);
			data->year = time_j[0];
			data->day = time_j[1];
			data->min = time_j[2];
			data->sec = 100*time_j[3] + 0.0001*time_j[4];

			/* additional navigation and depths */
			data->hdg = 10.0*store->course_true;
			data->course = 10.0*store->course_ground;
			data->speed = 196.36363636363*store->speed;
			data->speed_ref = store->speed_reference[0];
			data->pitch = 10.0*store->pitch;
			data->scale = 100*store->depth_scale;
			id = MBSYS_HSDS_BEAMS - 1;
			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				{
				data->dist[i] = store->distance[id-i];
				data->deph[i] = store->depth[id-i];
				}
			}

		/* comment */
		else if (store->kind == MB_DATA_COMMENT)
			{
			strcpy(datacomment,"cc");
			strncat(datacomment,store->comment,MBSYS_HSDS_MAXLINE);
			}
		}

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		dataplus->kind = mb_io_ptr->new_kind;

	/* check for comment */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		strcpy(datacomment,"cc");
		strncat(datacomment,mb_io_ptr->new_comment,
			mb_io_ptr->data_structure_size-3);
		}

	/* else translate current ping data to hsuricen data structure */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/* get time */
		mb_get_jtime(verbose,mb_io_ptr->new_time_i,time_j);
		data->year = time_j[0];
		data->day = time_j[1];
		data->min = time_j[2];
		data->sec = 100*time_j[3] + 0.0001*time_j[4];

		/* get navigation */
		if (mb_io_ptr->new_lon < -180.0)
			mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.0;
		if (mb_io_ptr->new_lon > 180.0)
			mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.0;
		data->lon = (int)(0.5 + 10000000.0*mb_io_ptr->new_lon);
		data->lat = (int)(0.5 + 10000000.0*mb_io_ptr->new_lat);

		/* get heading */
		data->hdg = (int)(0.5 + 10.*mb_io_ptr->new_heading);

		/* get speed (convert km/hr to 100Xnm/hr) */
		data->speed = 54.54545454*mb_io_ptr->new_speed;

		/* put distance and depth values 
			into hsuricen data structure */
		/* switch order of data as it is read 
			into the output arrays */
		id = mb_io_ptr->beams_bath - 1;
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			data->deph[i] = mb_io_ptr->new_bath[id-i];
			data->dist[i] = mb_io_ptr->new_bath_acrosstrack[id-i];
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Ready to write data in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",
			dataplus->kind);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (dataplus->kind == MB_DATA_DATA)
		{
		data->sec = mb_swap_short(data->sec);
		data->min = mb_swap_short(data->min);
		data->day = mb_swap_short(data->day);
		data->year = mb_swap_short(data->year);
		data->lat = mb_swap_long(data->lat);
		data->lon = mb_swap_long(data->lon);
		data->hdg = mb_swap_short(data->hdg);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->pitch = mb_swap_short(data->pitch);
		data->scale = mb_swap_short(data->scale);
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			data->dist[i] = mb_swap_short(data->dist[i]);
			data->deph[i] = mb_swap_short(data->deph[i]);
			}
		}
#endif

	/* write next record to file */
	if (dataplus->kind == MB_DATA_DATA
		|| dataplus->kind == MB_DATA_COMMENT)
		{
		if ((status = fwrite(data,1,mb_io_ptr->data_structure_size,
				mb_io_ptr->mbfp)) 
				== mb_io_ptr->data_structure_size) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		}
	else
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		if (verbose >= 5)
			fprintf(stderr,"\ndbg5  No data written in MBIO function <%s>\n",
				function_name);
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
