/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hsldeoih.c	2/11/93
 *	$Id: mbr_hsldeoih.c,v 4.14 1999-07-16 19:24:15 caress Exp $
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
 * mbr_hsldeoih.c contains the functions for reading and writing
 * multibeam data in the HSLDEOIH format.  
 * These functions include:
 *   mbr_alm_hsldeoih	- allocate read/write memory
 *   mbr_dem_hsldeoih	- deallocate read/write memory
 *   mbr_rt_hsldeoih	- read and translate data
 *   mbr_wt_hsldeoih	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 11, 1993
 * $Log: not supported by cvs2svn $
 * Revision 4.13  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.12  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.11  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.10  1996/04/24  01:14:38  caress
 * Code now keeps any water sound velocity or position offset
 * data encountered in memory.
 *
 * Revision 4.10  1996/04/24  01:14:38  caress
 * Code now keeps any water sound velocity or position offset
 * data encountered in memory.
 *
 * Revision 4.9  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.8  1995/07/26  14:45:39  caress
 * Fixed problems related to shallow water data.
 *
 * Revision 4.7  1995/03/08  18:13:53  caress
 * Fixed another bug regarding shallow water data.
 *
 * Revision 4.6  1995/03/08  13:31:09  caress
 * Fixed bug related to handling of shallow water data and the depth scale.
 *
 * Revision 4.5  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.4  1995/02/22  21:55:10  caress
 * Fixed reading of amplitude data from existing data.
 *
 * Revision 4.3  1994/10/21  12:20:01  caress
 * Release V4.0
 *
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
 * Revision 4.2  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.1  1994/03/03  03:15:16  caress
 * Fixed bug relating to processed amplitude data.
 *
 * Revision 4.0  1994/02/21  03:59:50  caress
 * First cut at new version. Altered to be consistent
 * with passing of three types of data: bathymetry,
 * amplitude, and sidescan.
 *
 * Revision 3.0  1993/05/14  22:56:08  sohara
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
#include "../../include/mbf_hsldeoih.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* local defines */
#define ZERO_ALL    0
#define ZERO_SOME   1

/*--------------------------------------------------------------------*/
int mbr_alm_hsldeoih(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_hsldeoih.c,v 4.14 1999-07-16 19:24:15 caress Exp $";
	char	*function_name = "mbr_alm_hsldeoih";
	int	status = MB_SUCCESS;
	int	i;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsldeoih_struct *data;
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
	mb_io_ptr->structure_size = sizeof(struct mbf_hsldeoih_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_hsds_struct),
				&mb_io_ptr->store_data,error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_hsldeoih_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;

	/* initialize everything to zeros */
	mbr_zero_hsldeoih(verbose,data_ptr,ZERO_ALL,error);

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
int mbr_dem_hsldeoih(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_hsldeoih";
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
int mbr_zero_hsldeoih(verbose,data_ptr,mode,error)
int	verbose;
char	*data_ptr;
int	mode;
int	*error;
{
	char	*function_name = "mbr_zero_hsldeoih";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_struct *data;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		fprintf(stderr,"dbg2       mode:       %d\n",mode);
		}

	/* get pointer to data descriptor */
	data = (struct mbf_hsldeoih_struct *) data_ptr;

	/* initialize almost everything to zeros */
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			data->amplitude[i] = 0;
			data->echo_duration[i] = 0;
			}

		/* these values zeroed only when structure
			is first allocated - this allows
			these values to be remembered internally 
			once one of these occasional data 
			records is encountered */
		if (mode == ZERO_ALL)
			{
			/* mean velocity (ERGNHYDI) */
			data->draught = 0.0;
			data->vel_mean = 0.0;
			data->vel_keel = 0.0;
			data->tide = 0.0;

			/* water velocity profile */
			data->num_vel = 0;
			for (i=0;i<MBF_HSLDEOIH_MAXVEL;i++)
				{
				data->depth[i] = 0;
				data->velocity[i] = 0;
				}

			/* navigation source (ERGNPOSI) */
			data->pos_corr_x = 0.0;
			data->pos_corr_y = 0.0;
			strncpy(data->sensors,"POS",9);
			}

		/* comment (LDEOCOMM) */
		strncpy(data->comment,"\0",MBF_HSLDEOIH_MAXLINE);

		/* processed backscatter */
		data->back_scale = 0.0;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			data->back[i] = 0;
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
int mbr_rt_hsldeoih(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_hsldeoih";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsldeoih_struct *data;
	struct mbsys_hsds_struct *store;
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
	data = (struct mbf_hsldeoih_struct *) mb_io_ptr->raw_data;
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
		mb_io_ptr->new_beamflag[i] = MB_FLAG_NULL;
		mb_io_ptr->new_bath[i] = 0.0;
		mb_io_ptr->new_bath_acrosstrack[i] = 0.0;
		mb_io_ptr->new_bath_alongtrack[i] = 0.0;
		}
	for (i=0;i<mb_io_ptr->beams_amp;i++)
		{
		mb_io_ptr->new_amp[i] = 0.0;
		}

	/* read next data from file */
	status = mbr_hsldeoih_rd_data(verbose,mbio_ptr,error);

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
			if (data->depth[i] > 0.0)
			    {
			    mb_io_ptr->new_beamflag[i] = MB_FLAG_NONE;
			    mb_io_ptr->new_bath[i] 
				    = data->depth_scale*data->depth[i];
			    }
			else if (data->depth[i] < 0.0)
			    {
			    mb_io_ptr->new_beamflag[i] = 
				    MB_FLAG_MANUAL + MB_FLAG_FLAG;
			    mb_io_ptr->new_bath[i] 
				    = -data->depth_scale*data->depth[i];
			    }
			else
			    {
			    mb_io_ptr->new_beamflag[i] = MB_FLAG_NULL;
			    mb_io_ptr->new_bath[i] 
				    = data->depth_scale*data->depth[i];
			    }
			mb_io_ptr->new_bath_acrosstrack[i] 
				= data->depth_scale*data->distance[i];
			mb_io_ptr->new_bath_alongtrack[i] = 0;
			}
		if (data->depth_center > 0.0)
		    {
		    mb_io_ptr->new_beamflag[29] = MB_FLAG_NONE;
		    mb_io_ptr->new_bath[29] = data->depth_center;
		    }
		else if (data->depth_center < 0.0)
		    {
		    mb_io_ptr->new_beamflag[29] = 
			    MB_FLAG_MANUAL + MB_FLAG_FLAG;
		    mb_io_ptr->new_bath[29] = data->depth_center;
		    }
		else
		    {
		    mb_io_ptr->new_beamflag[29] = MB_FLAG_NULL;
		    mb_io_ptr->new_bath[29] = data->depth_center;
		    }
		mb_io_ptr->new_bath_acrosstrack[29] = 0.0;

		/* read processed amplitude values into storage arrays */
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			mb_io_ptr->new_amp[i] 
				= data->back_scale*data->back[i];
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
			MBF_HSLDEOIH_MAXLINE);

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
		for (i=0;i<MBF_HSLDEOIH_MAXVEL;i++)
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
		store->back_scale = data->back_scale;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			store->back[i] = data->back[i];
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
int mbr_wt_hsldeoih(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_hsldeoih";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsldeoih_struct *data;
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
	data = (struct mbf_hsldeoih_struct *) mb_io_ptr->raw_data;
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
		for (i=0;i<MBF_HSLDEOIH_MAXVEL;i++)
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

		/* processed backscatter */
		data->back_scale = store->back_scale;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			data->back[i] = store->back[i];
			}
		}

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		data->kind = mb_io_ptr->new_kind;

	/* check for comment to be copied from mb_io_ptr */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		strncpy(data->comment,mb_io_ptr->new_comment,
			MBF_HSLDEOIH_MAXLINE);
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
			into hsldeoih data structure */
		if (data->depth_scale <= 0.0)
			data->depth_scale = 1.0;
		scalefactor = 1.0/data->depth_scale;
			
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			if (mb_beam_check_flag(mb_io_ptr->new_beamflag[i]))
			    data->depth[i] = -scalefactor*mb_io_ptr->new_bath[i];
			else
			    data->depth[i] = scalefactor*mb_io_ptr->new_bath[i];
			data->distance[i] 
				= scalefactor*mb_io_ptr->new_bath_acrosstrack[i];
			}
		if (mb_beam_check_flag(mb_io_ptr->new_beamflag[29]))
		    data->depth_center = -mb_io_ptr->new_bath[29];
		else
		    data->depth_center = mb_io_ptr->new_bath[29];

		/* put processed amplitude values 
			into hsldeoih data structure */
		if (data->back_scale > 0.0)
			scalefactor = 1.0/data->back_scale;
		else
			scalefactor = 1.0;
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			data->back[i] = (int) scalefactor*mb_io_ptr->new_amp[i];
			}

		/* add some plausible amounts for some of the 
			variables in the HSLDEOIH record */
		if (data->speed_reference[0] == ' ')
			data->speed_reference[0] = 'B';	/* assume speed over ground */
		if (data->spare == 0)
			data->spare = 1;
		}

	/* write next data to file */
	status = mbr_hsldeoih_wr_data(verbose,mbio_ptr,data_ptr,error);

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
int mbr_hsldeoih_rd_data(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsldeoih_struct *data;
	char	*data_ptr;
	FILE	*mbfp;
	int	label;
	int	record_size;
	short int	tmp;

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
	data = (struct mbf_hsldeoih_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	mbfp = mb_io_ptr->mbfp;

	/* initialize everything to zeros */
	mbr_zero_hsldeoih(verbose,data_ptr,ZERO_SOME,error);

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* get next record type */
	if ((status = fread(&label,1,sizeof(int),mbfp)) == sizeof(int)) 
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* swap bytes if necessary */
#ifdef BYTESWAPPED
	label = mb_swap_int(label);
#endif

	/* see if we just encountered a record label */
	if (status == MB_SUCCESS && label != MBF_HSLDEOIH_LABEL)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	/* read what size and kind of record it is */
	if (status == MB_SUCCESS)
		{
		if ((status = fread(&tmp,1,sizeof(short int),
			mbfp)) == sizeof(short int))
			{
#ifdef BYTESWAPPED
			data->kind = (int) mb_swap_short(tmp);
#else
			data->kind = (int) tmp;
#endif
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	if (status == MB_SUCCESS)
		{
		if ((status = fread(&tmp,1,sizeof(short int),mbfp)) 
			== sizeof(short int))
			{
#ifdef BYTESWAPPED
			record_size = (int) mb_swap_short(tmp);
#else
			record_size = (int) tmp;
#endif
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Read record label in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4       label:      %d\n",label);
		fprintf(stderr,"dbg4       size:       %d\n",record_size);
		fprintf(stderr,"dbg4       kind:       %d\n",data->kind);
		fprintf(stderr,"dbg4       error:      %d\n",*error);
		fprintf(stderr,"dbg4       status:     %d\n",status);
		}

	/* read the data */
	if (status == MB_SUCCESS)
		{
		if (data->kind == MB_DATA_DATA)
			status = mbr_hsldeoih_rd_survey(
				verbose,mbfp,data,error);
		else if (data->kind == MB_DATA_COMMENT)
			status = mbr_hsldeoih_rd_comment(
				verbose,mbfp,data,error);
		else if (data->kind == MB_DATA_CALIBRATE)
			status = mbr_hsldeoih_rd_calibrate(
				verbose,mbfp,data,error);
		else if (data->kind == MB_DATA_MEAN_VELOCITY)
			status = mbr_hsldeoih_rd_mean_velocity(
				verbose,mbfp,data,error);
		else if (data->kind == MB_DATA_VELOCITY_PROFILE)
			status = mbr_hsldeoih_rd_velocity_profile(
				verbose,mbfp,data,error);
		else if (data->kind == MB_DATA_STANDBY)
			status = mbr_hsldeoih_rd_standby(
				verbose,mbfp,data,error);
		else if (data->kind == MB_DATA_NAV_SOURCE)
			status = mbr_hsldeoih_rd_nav_source(
				verbose,mbfp,data,error);
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}
		
	/* get file position */
	mb_io_ptr->file_bytes = ftell(mbfp);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_hsldeoih_rd_nav_source(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsldeoih_struct *data;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_rd_nav_source";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_nav_source_struct read_data;
	int	read_size;

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

	/* read record from file */
	read_size = sizeof(read_data);
	if ((status = fread(&read_data,1,sizeof(read_data),
		mbfp)) != read_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	else 
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		mb_swap_float(&read_data.lon);
		mb_swap_float(&read_data.lat);
		read_data.year = mb_swap_short(read_data.year);
		read_data.month = mb_swap_short(read_data.month);
		read_data.day = mb_swap_short(read_data.day);
		read_data.hour = mb_swap_short(read_data.hour);
		read_data.minute = mb_swap_short(read_data.minute);
		read_data.second = mb_swap_short(read_data.second);
		read_data.alt_minute = mb_swap_short(read_data.alt_minute);
		read_data.alt_second = mb_swap_short(read_data.alt_second);
		mb_swap_float(&read_data.pos_corr_x);
		mb_swap_float(&read_data.pos_corr_y);
		}
#endif

	/* copy data to internal storage */
	if (status == MB_SUCCESS)
		{
		data->lon = read_data.lon;
		data->lat = read_data.lat;
		data->year = read_data.year;
		data->month = read_data.month;
		data->day = read_data.day;
		data->hour = read_data.hour;
		data->minute = read_data.minute;
		data->second = read_data.second;
		data->alt_minute = read_data.alt_minute;
		data->alt_second = read_data.alt_second;
		data->pos_corr_x = read_data.pos_corr_x;
		data->pos_corr_y = read_data.pos_corr_y;
		strncpy(data->sensors,read_data.sensors,8);
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
int mbr_hsldeoih_rd_mean_velocity(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsldeoih_struct *data;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_rd_mean_velocity";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_mean_velocity_struct read_data;
	int	read_size;

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

	/* read record from file */
	read_size = sizeof(read_data);
	if ((status = fread(&read_data,1,sizeof(read_data),
		mbfp)) != read_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
                }

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		mb_swap_float(&read_data.lon);
		mb_swap_float(&read_data.lat);
		read_data.year = mb_swap_short(read_data.year);
		read_data.month = mb_swap_short(read_data.month);
		read_data.day = mb_swap_short(read_data.day);
		read_data.hour = mb_swap_short(read_data.hour);
		read_data.minute = mb_swap_short(read_data.minute);
		read_data.second = mb_swap_short(read_data.second);
		read_data.alt_minute = mb_swap_short(read_data.alt_minute);
		read_data.alt_second = mb_swap_short(read_data.alt_second);
		mb_swap_float(&read_data.draught);
		mb_swap_float(&read_data.vel_mean);
		mb_swap_float(&read_data.vel_keel);
		mb_swap_float(&read_data.tide);
		}
#endif

	/* copy data to internal storage */
	if (status == MB_SUCCESS)
		{
		data->lon = read_data.lon;
		data->lat = read_data.lat;
		data->year = read_data.year;
		data->month = read_data.month;
		data->day = read_data.day;
		data->hour = read_data.hour;
		data->minute = read_data.minute;
		data->second = read_data.second;
		data->alt_minute = read_data.alt_minute;
		data->alt_second = read_data.alt_second;
		data->draught = read_data.draught;
		data->vel_mean = read_data.vel_mean;
		data->vel_keel = read_data.vel_keel;
		data->tide = read_data.tide;
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
int mbr_hsldeoih_rd_velocity_profile(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsldeoih_struct *data;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_rd_velocity_profile";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_velocity_profile_struct read_data;
	int	read_size;
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

	/* read record from file */
	read_size = sizeof(read_data);
	if ((status = fread(&read_data,1,sizeof(read_data),
		mbfp)) != read_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
                }

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		mb_swap_float(&read_data.lon);
		mb_swap_float(&read_data.lat);
		read_data.year = mb_swap_short(read_data.year);
		read_data.month = mb_swap_short(read_data.month);
		read_data.day = mb_swap_short(read_data.day);
		read_data.hour = mb_swap_short(read_data.hour);
		read_data.minute = mb_swap_short(read_data.minute);
		read_data.second = mb_swap_short(read_data.second);
		data->num_vel = mb_swap_short(read_data.num_vel);
		for (i=0;i<MBF_HSLDEOIH_MAXVEL;i++)
			{
			mb_swap_float(&read_data.vdepth[i]);
			mb_swap_float(&read_data.velocity[i]);
			}
		}
#endif

	/* copy data to internal storage */
	if (status == MB_SUCCESS)
		{
		data->lon = read_data.lon;
		data->lat = read_data.lat;
		data->year = read_data.year;
		data->month = read_data.month;
		data->day = read_data.day;
		data->hour = read_data.hour;
		data->minute = read_data.minute;
		data->second = read_data.second;
		data->num_vel = read_data.num_vel;
		for (i=0;i<MBF_HSLDEOIH_MAXVEL;i++)
			{
			data->vdepth[i] = read_data.vdepth[i];
			data->velocity[i] = read_data.velocity[i];
			}
		}
		
	/* check for sensible numbers of velocity-depth pairs */
	if (read_data.num_vel < 0
		|| read_data.num_vel > MBF_HSLDEOIH_MAXVEL)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
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
int mbr_hsldeoih_rd_standby(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsldeoih_struct *data;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_rd_standby";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_standby_struct read_data;
	int	read_size;

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

	/* read record from file */
	read_size = sizeof(read_data);
	if ((status = fread(&read_data,1,sizeof(read_data),
		mbfp)) != read_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
                }

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		mb_swap_float(&read_data.lon);
		mb_swap_float(&read_data.lat);
		read_data.year = mb_swap_short(read_data.year);
		read_data.month = mb_swap_short(read_data.month);
		read_data.day = mb_swap_short(read_data.day);
		read_data.hour = mb_swap_short(read_data.hour);
		read_data.minute = mb_swap_short(read_data.minute);
		read_data.second = mb_swap_short(read_data.second);
		read_data.alt_minute = mb_swap_short(read_data.alt_minute);
		read_data.alt_second = mb_swap_short(read_data.alt_second);
		mb_swap_float(&read_data.course_true);
		mb_swap_float(&read_data.speed_transverse);
		mb_swap_float(&read_data.speed);
		mb_swap_float(&read_data.pitch);
		read_data.track = mb_swap_short(read_data.track);
		mb_swap_float(&read_data.depth_center);
		}
#endif

	/* copy data to internal storage */
	if (status == MB_SUCCESS)
		{
		data->lon = read_data.lon;
		data->lat = read_data.lat;
		data->year = read_data.year;
		data->month = read_data.month;
		data->day = read_data.day;
		data->hour = read_data.hour;
		data->minute = read_data.minute;
		data->second = read_data.second;
		data->alt_minute = read_data.alt_minute;
		data->alt_second = read_data.alt_second;
		data->course_true = read_data.course_true;
		data->speed_transverse = read_data.speed_transverse;
		data->speed = read_data.speed;
		data->pitch = read_data.pitch;
		data->track = read_data.track;
		data->depth_center = read_data.depth_center;
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
int mbr_hsldeoih_rd_survey(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsldeoih_struct *data;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_rd_survey";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_survey_struct read_data;
	int	read_size;
	int	need_back,  gain_ok;
	int	gain_inner, gain_outer;
	double	gain_beam, factor;
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

	/* read record from file */
	read_size = sizeof(read_data);
	if ((status = fread(&read_data,1,sizeof(read_data),
		mbfp)) != read_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
                }

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		/* position */
		mb_swap_float(&read_data.lon);
		mb_swap_float(&read_data.lat);

		/* time stamp */
		read_data.year = mb_swap_short(read_data.year);
		read_data.month = mb_swap_short(read_data.month);
		read_data.day = mb_swap_short(read_data.day);
		read_data.hour = mb_swap_short(read_data.hour);
		read_data.minute = mb_swap_short(read_data.minute);
		read_data.second = mb_swap_short(read_data.second);
		read_data.alt_minute = mb_swap_short(read_data.alt_minute);
		read_data.alt_second = mb_swap_short(read_data.alt_second);

		/* additional navigation and depths */
		mb_swap_float(&read_data.course_true);
		mb_swap_float(&read_data.speed_transverse);
		mb_swap_float(&read_data.speed);
		mb_swap_float(&read_data.pitch);
		read_data.track = mb_swap_short(read_data.track);
		mb_swap_float(&read_data.depth_center);
		mb_swap_float(&read_data.depth_scale);
		read_data.spare = mb_swap_short(read_data.spare);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			read_data.distance[i] = 
				mb_swap_short(read_data.distance[i]);
			read_data.depth[i] = 
				mb_swap_short(read_data.depth[i]);
			}

		/* travel time data */
		mb_swap_float(&read_data.course_ground);
		mb_swap_float(&read_data.speed_ground);
		mb_swap_float(&read_data.heave);
		mb_swap_float(&read_data.roll);
		mb_swap_float(&read_data.time_center);
		mb_swap_float(&read_data.time_scale);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			read_data.time[i] = mb_swap_short(read_data.time[i]);
		for (i=0;i<11;i++)
			mb_swap_float(&read_data.gyro[i]);

		/* amplitude data */
		read_data.trans_strbd = mb_swap_short(read_data.trans_strbd);
		read_data.trans_vert = mb_swap_short(read_data.trans_vert);
		read_data.trans_port = mb_swap_short(read_data.trans_port);
		read_data.pulse_len_strbd 
			= mb_swap_short(read_data.pulse_len_strbd);
		read_data.pulse_len_vert 
			= mb_swap_short(read_data.pulse_len_vert);
		read_data.pulse_len_port 
			= mb_swap_short(read_data.pulse_len_port);
		read_data.gain_start 
			= mb_swap_short(read_data.gain_start);
		read_data.r_compensation_factor 
			= mb_swap_short(read_data.r_compensation_factor);
		read_data.compensation_start 
			= mb_swap_short(read_data.compensation_start);
		read_data.increase_start 
			= mb_swap_short(read_data.increase_start);
		read_data.tvc_near = mb_swap_short(read_data.tvc_near);
		read_data.tvc_far = mb_swap_short(read_data.tvc_far);
		read_data.increase_int_near 
			= mb_swap_short(read_data.increase_int_near);
		read_data.increase_int_far 
			= mb_swap_short(read_data.increase_int_far);
		read_data.gain_center = mb_swap_short(read_data.gain_center);
		mb_swap_float(&read_data.filter_gain);
		read_data.amplitude_center 
			= mb_swap_short(read_data.amplitude_center);
		read_data.echo_duration_center 
			= mb_swap_short(read_data.echo_duration_center);
		read_data.echo_scale_center 
			= mb_swap_short(read_data.echo_scale_center);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			read_data.amplitude[i] 
				= mb_swap_short(read_data.amplitude[i]);
			read_data.echo_duration[i] 
				= mb_swap_short(read_data.echo_duration[i]);
			}
		for (i=0;i<16;i++)
			{
			read_data.gain[i] 
				= mb_swap_short(read_data.gain[i]);
			read_data.echo_scale[i] 
				= mb_swap_short(read_data.echo_scale[i]);
			}

		/* processed backscatter data */
		mb_swap_float(&read_data.back_scale);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			read_data.back[i] = mb_swap_short(read_data.back[i]);
		}
#endif

	/* copy data to internal storage */
	if (status == MB_SUCCESS)
		{
		/* position */
		data->lon = read_data.lon;
		data->lat = read_data.lat;

		/* time stamp */
		data->year = read_data.year;
		data->month = read_data.month;
		data->day = read_data.day;
		data->hour = read_data.hour;
		data->minute = read_data.minute;
		data->second = read_data.second;
		data->alt_minute = read_data.alt_minute;
		data->alt_second = read_data.alt_second;

		/* additional navigation and depths */
		data->course_true = read_data.course_true;
		data->speed_transverse = read_data.speed_transverse;
		data->speed = read_data.speed;
		data->speed_reference[0] = read_data.speed_reference[0];
		data->pitch = read_data.pitch;
		data->track = read_data.track;
		data->depth_center = read_data.depth_center;
		data->depth_scale = read_data.depth_scale;
		data->spare = read_data.spare;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			data->distance[i] = read_data.distance[i];
			data->depth[i] = read_data.depth[i];
			}

		/* travel time data */
		data->course_ground = read_data.course_ground;
		data->speed_ground = read_data.speed_ground;
		data->heave = read_data.heave;
		data->roll = read_data.roll;
		data->time_center = read_data.time_center;
		data->time_scale = read_data.time_scale;
		data->mode[0] = read_data.mode[0];
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			data->time[i] = read_data.time[i];
		for (i=0;i<11;i++)
			data->gyro[i] = read_data.gyro[i];

		/* amplitude data */
		data->trans_strbd = read_data.trans_strbd;
		data->trans_vert = read_data.trans_vert;
		data->trans_port = read_data.trans_port;
		data->pulse_len_strbd = read_data.pulse_len_strbd;
		data->pulse_len_vert = read_data.pulse_len_vert;
		data->pulse_len_port = read_data.pulse_len_port;
		data->gain_start = read_data.gain_start;
		data->r_compensation_factor = read_data.r_compensation_factor;
		data->compensation_start = read_data.compensation_start;
		data->increase_start = read_data.increase_start;
		data->tvc_near = read_data.tvc_near;
		data->tvc_far = read_data.tvc_far;
		data->increase_int_near = read_data.increase_int_near;
		data->increase_int_far = read_data.increase_int_far;
		data->gain_center = read_data.gain_center;
		data->filter_gain = read_data.filter_gain;
		data->amplitude_center = read_data.amplitude_center;
		data->echo_duration_center = read_data.echo_duration_center;
		data->echo_scale_center = read_data.echo_scale_center;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			data->amplitude[i] = read_data.amplitude[i];
			data->echo_duration[i] = read_data.echo_duration[i];
			}
		for (i=0;i<16;i++)
			{
			data->gain[i] = read_data.gain[i];
			data->echo_scale[i] = read_data.echo_scale[i];
			}

		/* processed backscatter data */
		data->back_scale = read_data.back_scale;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			data->back[i] = read_data.back[i];
		}

	/* now fix some possible problems with processed 
		beam amplitudes */
	if (status == MB_SUCCESS)
		{

		/* see if gain values are messed up */
		gain_ok = MB_NO;
		i = 0;
		while (i < 8 && gain_ok == MB_NO)
			{
			if (data->gain[i] != data->gain[0])
				gain_ok = MB_YES;
			if (data->gain[i+8] != data->gain[8])
				gain_ok = MB_YES;
			i++;
			}

		/* fix gain values if needed */
		if (gain_ok == MB_NO)
			{
			gain_outer = data->gain[0];
			gain_inner = data->gain[8];
			for (i=0;i<16;i++)
				{
				if (i<4 || i > 11)
					data->gain[i] = gain_outer;
				else
					data->gain[i] = gain_inner;
				}
			}

		/* see if processed beam amplitude values 
			are available */
		need_back = MB_YES;
		i = 0;
		while (i < MBF_HSLDEOIH_BEAMS && need_back == MB_YES)
			{
			if (data->back[i] != 0)
				need_back = MB_NO;
			i++;
			}

		/* get beam amplitude values if needed */
		if (need_back == MB_YES)
			{
			data->back_scale = 1.0;
			for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
				{
				gain_beam = 6*data->gain[which_gain[i]];
				factor = 100.*pow(10.,(-0.05*gain_beam));
				data->back[i] = factor*data->amplitude[i];
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->distance[i],data->depth[i]);
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d\n",
				i,data->time[i]);
		fprintf(stderr,"dbg5       gyro headings:\n");
		for (i=0;i<11;i++)
			fprintf(stderr,"dbg5         %d  %f\n",
				i,data->gyro[i]);
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->amplitude[i],data->echo_duration[i]);
		fprintf(stderr,"dbg5       gains and echo scales:\n");
		for (i=0;i<16;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->gain[i],data->echo_scale[i]);
		fprintf(stderr,"dbg5       back_scale:       %f\n",
			data->back_scale);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d\n",
				i,data->back[i]);
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
int mbr_hsldeoih_rd_calibrate(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsldeoih_struct *data;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_rd_calibrate";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_calibrate_struct read_data;
	int	read_size;
	int	need_back,  gain_ok;
	int	gain_inner, gain_outer;
	double	gain_beam, factor;
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

	/* read record from file */
	read_size = sizeof(read_data);
	if ((status = fread(&read_data,1,sizeof(read_data),
		mbfp)) != read_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
                }

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		/* position */
		mb_swap_float(&read_data.lon);
		mb_swap_float(&read_data.lat);

		/* time stamp */
		read_data.year = mb_swap_short(read_data.year);
		read_data.month = mb_swap_short(read_data.month);
		read_data.day = mb_swap_short(read_data.day);
		read_data.hour = mb_swap_short(read_data.hour);
		read_data.minute = mb_swap_short(read_data.minute);
		read_data.second = mb_swap_short(read_data.second);
		read_data.alt_minute = mb_swap_short(read_data.alt_minute);
		read_data.alt_second = mb_swap_short(read_data.alt_second);

		/* additional navigation and depths */
		mb_swap_float(&read_data.course_true);
		mb_swap_float(&read_data.speed_transverse);
		mb_swap_float(&read_data.speed);
		mb_swap_float(&read_data.pitch);
		read_data.track = mb_swap_short(read_data.track);
		mb_swap_float(&read_data.depth_center);
		mb_swap_float(&read_data.depth_scale);
		read_data.spare = mb_swap_short(read_data.spare);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			read_data.distance[i] = 
				mb_swap_short(read_data.distance[i]);
			read_data.depth[i] = 
				mb_swap_short(read_data.depth[i]);
			}

		/* travel time data */
		mb_swap_float(&read_data.course_ground);
		mb_swap_float(&read_data.speed_ground);
		mb_swap_float(&read_data.heave);
		mb_swap_float(&read_data.roll);
		mb_swap_float(&read_data.time_center);
		mb_swap_float(&read_data.time_scale);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			read_data.time[i] = mb_swap_short(read_data.time[i]);
		for (i=0;i<11;i++)
			mb_swap_float(&read_data.gyro[i]);

		/* amplitude data */
		read_data.trans_strbd = mb_swap_short(read_data.trans_strbd);
		read_data.trans_vert = mb_swap_short(read_data.trans_vert);
		read_data.trans_port = mb_swap_short(read_data.trans_port);
		read_data.pulse_len_strbd 
			= mb_swap_short(read_data.pulse_len_strbd);
		read_data.pulse_len_vert 
			= mb_swap_short(read_data.pulse_len_vert);
		read_data.pulse_len_port 
			= mb_swap_short(read_data.pulse_len_port);
		read_data.gain_start 
			= mb_swap_short(read_data.gain_start);
		read_data.r_compensation_factor 
			= mb_swap_short(read_data.r_compensation_factor);
		read_data.compensation_start 
			= mb_swap_short(read_data.compensation_start);
		read_data.increase_start 
			= mb_swap_short(read_data.increase_start);
		read_data.tvc_near = mb_swap_short(read_data.tvc_near);
		read_data.tvc_far = mb_swap_short(read_data.tvc_far);
		read_data.increase_int_near 
			= mb_swap_short(read_data.increase_int_near);
		read_data.increase_int_far 
			= mb_swap_short(read_data.increase_int_far);
		read_data.gain_center = mb_swap_short(read_data.gain_center);
		mb_swap_float(&read_data.filter_gain);
		read_data.amplitude_center 
			= mb_swap_short(read_data.amplitude_center);
		read_data.echo_duration_center 
			= mb_swap_short(read_data.echo_duration_center);
		read_data.echo_scale_center 
			= mb_swap_short(read_data.echo_scale_center);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			read_data.amplitude[i] 
				= mb_swap_short(read_data.amplitude[i]);
			read_data.echo_duration[i] 
				= mb_swap_short(read_data.echo_duration[i]);
			}
		for (i=0;i<16;i++)
			{
			read_data.gain[i] 
				= mb_swap_short(read_data.gain[i]);
			read_data.echo_scale[i] 
				= mb_swap_short(read_data.echo_scale[i]);
			}

		/* processed backscatter data */
		mb_swap_float(&read_data.back_scale);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			read_data.back[i] = mb_swap_short(read_data.back[i]);
		}
#endif

	/* copy data to internal storage */
	if (status == MB_SUCCESS)
		{
		/* position */
		data->lon = read_data.lon;
		data->lat = read_data.lat;

		/* time stamp */
		data->year = read_data.year;
		data->month = read_data.month;
		data->day = read_data.day;
		data->hour = read_data.hour;
		data->minute = read_data.minute;
		data->second = read_data.second;
		data->alt_minute = read_data.alt_minute;
		data->alt_second = read_data.alt_second;

		/* additional navigation and depths */
		data->course_true = read_data.course_true;
		data->speed_transverse = read_data.speed_transverse;
		data->speed = read_data.speed;
		data->speed_reference[0] = read_data.speed_reference[0];
		data->pitch = read_data.pitch;
		data->track = read_data.track;
		data->depth_center = read_data.depth_center;
		data->depth_scale = read_data.depth_scale;
		data->spare = read_data.spare;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			data->distance[i] = read_data.distance[i];
			data->depth[i] = read_data.depth[i];
			}

		/* travel time data */
		data->course_ground = read_data.course_ground;
		data->speed_ground = read_data.speed_ground;
		data->heave = read_data.heave;
		data->roll = read_data.roll;
		data->time_center = read_data.time_center;
		data->time_scale = read_data.time_scale;
		data->mode[0] = read_data.mode[0];
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			data->time[i] = read_data.time[i];
		for (i=0;i<11;i++)
			data->gyro[i] = read_data.gyro[i];

		/* amplitude data */
		data->trans_strbd = read_data.trans_strbd;
		data->trans_vert = read_data.trans_vert;
		data->trans_port = read_data.trans_port;
		data->pulse_len_strbd = read_data.pulse_len_strbd;
		data->pulse_len_vert = read_data.pulse_len_vert;
		data->pulse_len_port = read_data.pulse_len_port;
		data->gain_start = read_data.gain_start;
		data->r_compensation_factor = read_data.r_compensation_factor;
		data->compensation_start = read_data.compensation_start;
		data->increase_start = read_data.increase_start;
		data->tvc_near = read_data.tvc_near;
		data->tvc_far = read_data.tvc_far;
		data->increase_int_near = read_data.increase_int_near;
		data->increase_int_far = read_data.increase_int_far;
		data->gain_center = read_data.gain_center;
		data->filter_gain = read_data.filter_gain;
		data->amplitude_center = read_data.amplitude_center;
		data->echo_duration_center = read_data.echo_duration_center;
		data->echo_scale_center = read_data.echo_scale_center;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			data->amplitude[i] = read_data.amplitude[i];
			data->echo_duration[i] = read_data.echo_duration[i];
			}
		for (i=0;i<16;i++)
			{
			data->gain[i] = read_data.gain[i];
			data->echo_scale[i] = read_data.echo_scale[i];
			}

		/* processed backscatter data */
		data->back_scale = read_data.back_scale;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			data->back[i] = read_data.back[i];
		}

	/* now fix some possible problems with processed 
		beam amplitudes */
	if (status == MB_SUCCESS)
		{

		/* see if gain values are messed up */
		gain_ok = MB_NO;
		i = 0;
		while (i < 8 && gain_ok == MB_NO)
			{
			if (data->gain[i] != data->gain[0])
				gain_ok = MB_YES;
			if (data->gain[i+8] != data->gain[8])
				gain_ok = MB_YES;
			i++;
			}

		/* fix gain values if needed */
		if (gain_ok == MB_NO)
			{
			gain_outer = data->gain[0];
			gain_inner = data->gain[8];
			for (i=0;i<16;i++)
				{
				if (i<4 || i > 11)
					data->gain[i] = gain_outer;
				else
					data->gain[i] = gain_inner;
				}
			}

		/* see if processed beam amplitude values 
			are available */
		need_back = MB_YES;
		i = 0;
		while (i < MBF_HSLDEOIH_BEAMS && need_back == MB_YES)
			{
			if (data->back[i] != 0)
				need_back = MB_NO;
			i++;
			}

		/* get beam amplitude values if needed */
		if (need_back == MB_YES)
			{
			data->back_scale = 1.0;
			for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
				{
				gain_beam = 6*data->gain[which_gain[i]];
				factor = 100.*pow(10.,(-0.05*gain_beam));
				data->back[i] = factor*data->amplitude[i];
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->distance[i],data->depth[i]);
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d\n",
				i,data->time[i]);
		fprintf(stderr,"dbg5       gyro headings:\n");
		for (i=0;i<11;i++)
			fprintf(stderr,"dbg5         %d  %f\n",
				i,data->gyro[i]);
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->amplitude[i],data->echo_duration[i]);
		fprintf(stderr,"dbg5       gains and echo scales:\n");
		for (i=0;i<16;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->gain[i],data->echo_scale[i]);
		fprintf(stderr,"dbg5       back_scale:       %f\n",
			data->back_scale);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d\n",
				i,data->back[i]);
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
int mbr_hsldeoih_rd_comment(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsldeoih_struct *data;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_rd_comment";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_comment_struct read_data;
	int	read_size;

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

	/* read record from file */
	read_size = sizeof(read_data);
	if ((status = fread(&read_data,1,sizeof(read_data),
		mbfp)) != read_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
                }

	/* copy data to internal storage */
	if (status == MB_SUCCESS)
		{
		strncpy(data->comment,read_data.comment,MBF_HSLDEOIH_MAXLINE);
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
int mbr_hsldeoih_wr_data(verbose,mbio_ptr,data_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsldeoih_struct *data;
	FILE	*mbfp;
	int	label;
	short int shortkind;

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
	data = (struct mbf_hsldeoih_struct *) data_ptr;
	mbfp = mb_io_ptr->mbfp;

	/* print output debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Data record kind in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4       kind:       %d\n",data->kind);
		}

	/* write record label to file */
	label = MBF_HSLDEOIH_LABEL;
#ifdef BYTESWAPPED
	label = mb_swap_int(label);
#endif
	if ((status = fwrite(&label,1,sizeof(int),mbfp)) != sizeof(int))
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	else
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	shortkind = data->kind;
#ifdef BYTESWAPPED
	shortkind = mb_swap_short(shortkind);
#endif
	if ((status = fwrite(&shortkind,1,sizeof(short int),mbfp)) 
		!= sizeof(short int))
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	else
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}

	/* write the data */
	if (status == MB_SUCCESS)
		{
		if (data->kind == MB_DATA_DATA)
			status = mbr_hsldeoih_wr_survey(verbose,
				mbfp,data,error);
		else if (data->kind == MB_DATA_COMMENT)
			status = mbr_hsldeoih_wr_comment(verbose,
				mbfp,data,error);
		else if (data->kind == MB_DATA_CALIBRATE)
			status = mbr_hsldeoih_wr_calibrate(verbose,
				mbfp,data,error);
		else if (data->kind == MB_DATA_MEAN_VELOCITY)
			status = mbr_hsldeoih_wr_mean_velocity(verbose,
				mbfp,data,error);
		else if (data->kind == MB_DATA_VELOCITY_PROFILE)
			status = mbr_hsldeoih_wr_velocity_profile(verbose,
				mbfp,data,error);
		else if (data->kind == MB_DATA_STANDBY)
			status = mbr_hsldeoih_wr_standby(verbose,
				mbfp,data,error);
		else if (data->kind == MB_DATA_NAV_SOURCE)
			status = mbr_hsldeoih_wr_nav_source(verbose,
				mbfp,data,error);
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_KIND;
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
int mbr_hsldeoih_wr_nav_source(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsldeoih_struct *data;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_wr_nav_source";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_nav_source_struct write_data;
	int	write_size;
	short int write_size_short;

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

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to write in MBIO function <%s>\n",
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

	/* copy data from internal storage */
	if (status == MB_SUCCESS)
		{
		write_data.lon = data->lon;
		write_data.lat = data->lat;
		write_data.year = data->year;
		write_data.month = data->month;
		write_data.day = data->day;
		write_data.hour = data->hour;
		write_data.minute = data->minute;
		write_data.second = data->second;
		write_data.alt_minute = data->alt_minute;
		write_data.alt_second = data->alt_second;
		write_data.pos_corr_x = data->pos_corr_x;
		write_data.pos_corr_y = data->pos_corr_y;
		strncpy(write_data.sensors,data->sensors,8);
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		mb_swap_float(&write_data.lon);
		mb_swap_float(&write_data.lat);
		write_data.year = mb_swap_short(write_data.year);
		write_data.month = mb_swap_short(write_data.month);
		write_data.day = mb_swap_short(write_data.day);
		write_data.hour = mb_swap_short(write_data.hour);
		write_data.minute = mb_swap_short(write_data.minute);
		write_data.second = mb_swap_short(write_data.second);
		write_data.alt_minute = mb_swap_short(write_data.alt_minute);
		write_data.alt_second = mb_swap_short(write_data.alt_second);
		mb_swap_float(&write_data.pos_corr_x);
		mb_swap_float(&write_data.pos_corr_y);
		}
#endif

	/* write record to file */
	write_size = sizeof(write_data);
	write_size_short = write_size;
#ifdef BYTESWAPPED
	write_size_short = mb_swap_short(write_size_short);
#endif
	if ((status = fwrite(&write_size_short,1,sizeof(short int),mbfp))
		!= sizeof(short int))
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	else
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	if ((status = fwrite(&write_data,1,sizeof(write_data),mbfp)) 
		!= write_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
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
int mbr_hsldeoih_wr_mean_velocity(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsldeoih_struct *data;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_wr_mean_velocity";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_mean_velocity_struct write_data;
	int	write_size;
	short int write_size_short;

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

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to write in MBIO function <%s>\n",
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

	/* copy data from internal storage */
	if (status == MB_SUCCESS)
		{
		write_data.lon = data->lon;
		write_data.lat = data->lat;
		write_data.year = data->year;
		write_data.month = data->month;
		write_data.day = data->day;
		write_data.hour = data->hour;
		write_data.minute = data->minute;
		write_data.second = data->second;
		write_data.alt_minute = data->alt_minute;
		write_data.alt_second = data->alt_second;
		write_data.draught = data->draught;
		write_data.vel_mean = data->vel_mean;
		write_data.vel_keel = data->vel_keel;
		write_data.tide = data->tide;
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		mb_swap_float(&write_data.lon);
		mb_swap_float(&write_data.lat);
		write_data.year = mb_swap_short(write_data.year);
		write_data.month = mb_swap_short(write_data.month);
		write_data.day = mb_swap_short(write_data.day);
		write_data.hour = mb_swap_short(write_data.hour);
		write_data.minute = mb_swap_short(write_data.minute);
		write_data.second = mb_swap_short(write_data.second);
		write_data.alt_minute = mb_swap_short(write_data.alt_minute);
		write_data.alt_second = mb_swap_short(write_data.alt_second);
		mb_swap_float(&write_data.draught);
		mb_swap_float(&write_data.vel_mean);
		mb_swap_float(&write_data.vel_keel);
		mb_swap_float(&write_data.tide);
		}
#endif

	/* write record to file */
	write_size = sizeof(write_data);
	write_size_short = write_size;
#ifdef BYTESWAPPED
	write_size_short = mb_swap_short(write_size_short);
#endif
	if ((status = fwrite(&write_size_short,1,sizeof(short int),mbfp))
		!= sizeof(short int))
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
                }
	if ((status = fwrite(&write_data,1,sizeof(write_data),
		mbfp)) != write_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
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
int mbr_hsldeoih_wr_velocity_profile(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsldeoih_struct *data;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_wr_velocity_profile";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_velocity_profile_struct write_data;
	int	write_size;
	short int write_size_short;
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

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to write in MBIO function <%s>\n",
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

	/* copy data from internal storage */
	if (status == MB_SUCCESS)
		{
		write_data.lon = data->lon;
		write_data.lat = data->lat;
		write_data.year = data->year;
		write_data.month = data->month;
		write_data.day = data->day;
		write_data.hour = data->hour;
		write_data.minute = data->minute;
		write_data.second = data->second;
		write_data.num_vel = data->num_vel;
		for (i=0;i<MBF_HSLDEOIH_MAXVEL;i++)
			{
			write_data.vdepth[i] = data->vdepth[i];
			write_data.velocity[i] = data->velocity[i];
			}
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		mb_swap_float(&write_data.lon);
		mb_swap_float(&write_data.lat);
		write_data.year = mb_swap_short(write_data.year);
		write_data.month = mb_swap_short(write_data.month);
		write_data.day = mb_swap_short(write_data.day);
		write_data.hour = mb_swap_short(write_data.hour);
		write_data.minute = mb_swap_short(write_data.minute);
		write_data.second = mb_swap_short(write_data.second);
		data->num_vel = mb_swap_short(write_data.num_vel);
		for (i=0;i<MBF_HSLDEOIH_MAXVEL;i++)
			{
			mb_swap_float(&write_data.vdepth[i]);
			mb_swap_float(&write_data.velocity[i]);
			}
		}
#endif

	/* write record to file */
	write_size = sizeof(write_data);
	write_size_short = write_size;
#ifdef BYTESWAPPED
	write_size_short = mb_swap_short(write_size_short);
#endif
	if ((status = fwrite(&write_size_short,1,sizeof(short int),mbfp))
		!= sizeof(short int))
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
                }
	if ((status = fwrite(&write_data,1,sizeof(write_data),
		mbfp)) != write_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
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
int mbr_hsldeoih_wr_standby(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsldeoih_struct *data;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_wr_standby";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_standby_struct write_data;
	int	write_size;
	short int write_size_short;

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

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to write in MBIO function <%s>\n",
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

	/* copy data from internal storage */
	if (status == MB_SUCCESS)
		{
		write_data.lon = data->lon;
		write_data.lat = data->lat;
		write_data.year = data->year;
		write_data.month = data->month;
		write_data.day = data->day;
		write_data.hour = data->hour;
		write_data.minute = data->minute;
		write_data.second = data->second;
		write_data.alt_minute = data->alt_minute;
		write_data.alt_second = data->alt_second;
		write_data.course_true = data->course_true;
		write_data.speed_transverse = data->speed_transverse;
		write_data.speed = data->speed;
		write_data.speed_reference[0] = data->speed_reference[0];
		write_data.pitch = data->pitch;
		write_data.track = data->track;
		write_data.depth_center = data->depth_center;
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		mb_swap_float(&write_data.lon);
		mb_swap_float(&write_data.lat);
		write_data.year = mb_swap_short(write_data.year);
		write_data.month = mb_swap_short(write_data.month);
		write_data.day = mb_swap_short(write_data.day);
		write_data.hour = mb_swap_short(write_data.hour);
		write_data.minute = mb_swap_short(write_data.minute);
		write_data.second = mb_swap_short(write_data.second);
		write_data.alt_minute = mb_swap_short(write_data.alt_minute);
		write_data.alt_second = mb_swap_short(write_data.alt_second);
		mb_swap_float(&write_data.course_true);
		mb_swap_float(&write_data.speed_transverse);
		mb_swap_float(&write_data.speed);
		mb_swap_float(&write_data.pitch);
		write_data.track = mb_swap_short(write_data.track);
		mb_swap_float(&write_data.depth_center);
		}
#endif

	/* write record to file */
	write_size = sizeof(write_data);
	write_size_short = write_size;
#ifdef BYTESWAPPED
	write_size_short = mb_swap_short(write_size_short);
#endif
	if ((status = fwrite(&write_size_short,1,sizeof(short int),mbfp))
		!= sizeof(short int))
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
                }
	if ((status = fwrite(&write_data,1,sizeof(write_data),
		mbfp)) != write_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
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
int mbr_hsldeoih_wr_survey(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsldeoih_struct *data;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_wr_survey";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_survey_struct write_data;
	int	write_size;
	short int write_size_short;
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

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to write in MBIO function <%s>\n",
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->distance[i],data->depth[i]);
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d\n",
				i,data->time[i]);
		fprintf(stderr,"dbg5       gyro headings:\n");
		for (i=0;i<11;i++)
			fprintf(stderr,"dbg5         %d  %f\n",
				i,data->gyro[i]);
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->amplitude[i],data->echo_duration[i]);
		fprintf(stderr,"dbg5       gains and echo scales:\n");
		for (i=0;i<16;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->gain[i],data->echo_scale[i]);
		fprintf(stderr,"dbg5       back_scale:       %f\n",
			data->back_scale);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d\n",
				i,data->back[i]);
		}

	/* copy data from internal storage */
	if (status == MB_SUCCESS)
		{
		/* position */
		write_data.lon = data->lon;
		write_data.lat = data->lat;

		/* time stamp */
		write_data.year = data->year;
		write_data.month = data->month;
		write_data.day = data->day;
		write_data.hour = data->hour;
		write_data.minute = data->minute;
		write_data.second = data->second;
		write_data.alt_minute = data->alt_minute;
		write_data.alt_second = data->alt_second;

		/* additional navigation and depths */
		write_data.course_true = data->course_true;
		write_data.speed_transverse = data->speed_transverse;
		write_data.speed = data->speed;
		write_data.speed_reference[0] = data->speed_reference[0];
		write_data.pitch = data->pitch;
		write_data.track = data->track;
		write_data.depth_center = data->depth_center;
		write_data.depth_scale = data->depth_scale;
		write_data.spare = data->spare;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			write_data.distance[i] = data->distance[i];
			write_data.depth[i] = data->depth[i];
			}

		/* travel time data */
		write_data.course_ground = data->course_ground;
		write_data.speed_ground = data->speed_ground;
		write_data.heave = data->heave;
		write_data.roll = data->roll;
		write_data.time_center = data->time_center;
		write_data.time_scale = data->time_scale;
		write_data.mode[0] = data->mode[0];
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			write_data.time[i] = data->time[i];
		for (i=0;i<11;i++)
			write_data.gyro[i] = data->gyro[i];

		/* amplitude data */
		write_data.trans_strbd = data->trans_strbd;
		write_data.trans_vert = data->trans_vert;
		write_data.trans_port = data->trans_port;
		write_data.pulse_len_strbd = data->pulse_len_strbd;
		write_data.pulse_len_vert = data->pulse_len_vert;
		write_data.pulse_len_port = data->pulse_len_port;
		write_data.gain_start = data->gain_start;
		write_data.r_compensation_factor = data->r_compensation_factor;
		write_data.compensation_start = data->compensation_start;
		write_data.increase_start = data->increase_start;
		write_data.tvc_near = data->tvc_near;
		write_data.tvc_far = data->tvc_far;
		write_data.increase_int_near = data->increase_int_near;
		write_data.increase_int_far = data->increase_int_far;
		write_data.gain_center = data->gain_center;
		write_data.filter_gain = data->filter_gain;
		write_data.amplitude_center = data->amplitude_center;
		write_data.echo_duration_center = data->echo_duration_center;
		write_data.echo_scale_center = data->echo_scale_center;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			write_data.amplitude[i] = data->amplitude[i];
			write_data.echo_duration[i] = data->echo_duration[i];
			}
		for (i=0;i<16;i++)
			{
			write_data.gain[i] = data->gain[i];
			write_data.echo_scale[i] = data->echo_scale[i];
			}

		/* processed backscatter data */
		write_data.back_scale = data->back_scale;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			write_data.back[i] = data->back[i];
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		/* position */
		mb_swap_float(&write_data.lon);
		mb_swap_float(&write_data.lat);

		/* time stamp */
		write_data.year = mb_swap_short(write_data.year);
		write_data.month = mb_swap_short(write_data.month);
		write_data.day = mb_swap_short(write_data.day);
		write_data.hour = mb_swap_short(write_data.hour);
		write_data.minute = mb_swap_short(write_data.minute);
		write_data.second = mb_swap_short(write_data.second);
		write_data.alt_minute = mb_swap_short(write_data.alt_minute);
		write_data.alt_second = mb_swap_short(write_data.alt_second);

		/* additional navigation and depths */
		mb_swap_float(&write_data.course_true);
		mb_swap_float(&write_data.speed_transverse);
		mb_swap_float(&write_data.speed);
		mb_swap_float(&write_data.pitch);
		write_data.track = mb_swap_short(write_data.track);
		mb_swap_float(&write_data.depth_center);
		mb_swap_float(&write_data.depth_scale);
		write_data.spare = mb_swap_short(write_data.spare);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			write_data.distance[i] = 
				mb_swap_short(write_data.distance[i]);
			write_data.depth[i] = 
				mb_swap_short(write_data.depth[i]);
			}

		/* travel time data */
		mb_swap_float(&write_data.course_ground);
		mb_swap_float(&write_data.speed_ground);
		mb_swap_float(&write_data.heave);
		mb_swap_float(&write_data.roll);
		mb_swap_float(&write_data.time_center);
		mb_swap_float(&write_data.time_scale);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			write_data.time[i] = mb_swap_short(write_data.time[i]);
		for (i=0;i<11;i++)
			mb_swap_float(&write_data.gyro[i]);

		/* amplitude data */
		write_data.trans_strbd = mb_swap_short(write_data.trans_strbd);
		write_data.trans_vert = mb_swap_short(write_data.trans_vert);
		write_data.trans_port = mb_swap_short(write_data.trans_port);
		write_data.pulse_len_strbd 
			= mb_swap_short(write_data.pulse_len_strbd);
		write_data.pulse_len_vert 
			= mb_swap_short(write_data.pulse_len_vert);
		write_data.pulse_len_port 
			= mb_swap_short(write_data.pulse_len_port);
		write_data.gain_start 
			= mb_swap_short(write_data.gain_start);
		write_data.r_compensation_factor 
			= mb_swap_short(write_data.r_compensation_factor);
		write_data.compensation_start 
			= mb_swap_short(write_data.compensation_start);
		write_data.increase_start 
			= mb_swap_short(write_data.increase_start);
		write_data.tvc_near = mb_swap_short(write_data.tvc_near);
		write_data.tvc_far = mb_swap_short(write_data.tvc_far);
		write_data.increase_int_near 
			= mb_swap_short(write_data.increase_int_near);
		write_data.increase_int_far 
			= mb_swap_short(write_data.increase_int_far);
		write_data.gain_center = mb_swap_short(write_data.gain_center);
		mb_swap_float(&write_data.filter_gain);
		write_data.amplitude_center 
			= mb_swap_short(write_data.amplitude_center);
		write_data.echo_duration_center 
			= mb_swap_short(write_data.echo_duration_center);
		write_data.echo_scale_center 
			= mb_swap_short(write_data.echo_scale_center);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			write_data.amplitude[i] 
				= mb_swap_short(write_data.amplitude[i]);
			write_data.echo_duration[i] 
				= mb_swap_short(write_data.echo_duration[i]);
			}
		for (i=0;i<16;i++)
			{
			write_data.gain[i] 
				= mb_swap_short(write_data.gain[i]);
			write_data.echo_scale[i] 
				= mb_swap_short(write_data.echo_scale[i]);
			}

		/* processed backscatter data */
		mb_swap_float(&write_data.back_scale);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			write_data.back[i] = mb_swap_short(write_data.back[i]);
		}
#endif

	/* write record to file */
	write_size = sizeof(write_data);
	write_size_short = write_size;
#ifdef BYTESWAPPED
	write_size_short = mb_swap_short(write_size_short);
#endif
	if ((status = fwrite(&write_size_short,1,sizeof(short int),mbfp))
		!= sizeof(short int))
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
                }
	if ((status = fwrite(&write_data,1,sizeof(write_data),
		mbfp)) != write_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
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
int mbr_hsldeoih_wr_calibrate(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsldeoih_struct *data;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_wr_calibrate";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_calibrate_struct write_data;
	int	write_size;
	short int write_size_short;
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

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to write in MBIO function <%s>\n",
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->distance[i],data->depth[i]);
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d\n",
				i,data->time[i]);
		fprintf(stderr,"dbg5       gyro headings:\n");
		for (i=0;i<11;i++)
			fprintf(stderr,"dbg5         %d  %f\n",
				i,data->gyro[i]);
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
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->amplitude[i],data->echo_duration[i]);
		fprintf(stderr,"dbg5       gains and echo scales:\n");
		for (i=0;i<16;i++)
			fprintf(stderr,"dbg5         %d  %d  %d\n",
				i,data->gain[i],data->echo_scale[i]);
		fprintf(stderr,"dbg5       back_scale:       %f\n",
			data->back_scale);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			fprintf(stderr,"dbg5         %d  %d\n",
				i,data->back[i]);
		}

	/* copy data from internal storage */
	if (status == MB_SUCCESS)
		{
		/* position */
		write_data.lon = data->lon;
		write_data.lat = data->lat;

		/* time stamp */
		write_data.year = data->year;
		write_data.month = data->month;
		write_data.day = data->day;
		write_data.hour = data->hour;
		write_data.minute = data->minute;
		write_data.second = data->second;
		write_data.alt_minute = data->alt_minute;
		write_data.alt_second = data->alt_second;

		/* additional navigation and depths */
		write_data.course_true = data->course_true;
		write_data.speed_transverse = data->speed_transverse;
		write_data.speed = data->speed;
		write_data.speed_reference[0] = data->speed_reference[0];
		write_data.pitch = data->pitch;
		write_data.track = data->track;
		write_data.depth_center = data->depth_center;
		write_data.depth_scale = data->depth_scale;
		write_data.spare = data->spare;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			write_data.distance[i] = data->distance[i];
			write_data.depth[i] = data->depth[i];
			}

		/* travel time data */
		write_data.course_ground = data->course_ground;
		write_data.speed_ground = data->speed_ground;
		write_data.heave = data->heave;
		write_data.roll = data->roll;
		write_data.time_center = data->time_center;
		write_data.time_scale = data->time_scale;
		write_data.mode[0] = data->mode[0];
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			write_data.time[i] = data->time[i];
		for (i=0;i<11;i++)
			write_data.gyro[i] = data->gyro[i];

		/* amplitude data */
		write_data.trans_strbd = data->trans_strbd;
		write_data.trans_vert = data->trans_vert;
		write_data.trans_port = data->trans_port;
		write_data.pulse_len_strbd = data->pulse_len_strbd;
		write_data.pulse_len_vert = data->pulse_len_vert;
		write_data.pulse_len_port = data->pulse_len_port;
		write_data.gain_start = data->gain_start;
		write_data.r_compensation_factor = data->r_compensation_factor;
		write_data.compensation_start = data->compensation_start;
		write_data.increase_start = data->increase_start;
		write_data.tvc_near = data->tvc_near;
		write_data.tvc_far = data->tvc_far;
		write_data.increase_int_near = data->increase_int_near;
		write_data.increase_int_far = data->increase_int_far;
		write_data.gain_center = data->gain_center;
		write_data.filter_gain = data->filter_gain;
		write_data.amplitude_center = data->amplitude_center;
		write_data.echo_duration_center = data->echo_duration_center;
		write_data.echo_scale_center = data->echo_scale_center;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			write_data.amplitude[i] = data->amplitude[i];
			write_data.echo_duration[i] = data->echo_duration[i];
			}
		for (i=0;i<16;i++)
			{
			write_data.gain[i] = data->gain[i];
			write_data.echo_scale[i] = data->echo_scale[i];
			}

		/* processed backscatter data */
		write_data.back_scale = data->back_scale;
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			write_data.back[i] = data->back[i];
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		/* position */
		mb_swap_float(&write_data.lon);
		mb_swap_float(&write_data.lat);

		/* time stamp */
		write_data.year = mb_swap_short(write_data.year);
		write_data.month = mb_swap_short(write_data.month);
		write_data.day = mb_swap_short(write_data.day);
		write_data.hour = mb_swap_short(write_data.hour);
		write_data.minute = mb_swap_short(write_data.minute);
		write_data.second = mb_swap_short(write_data.second);
		write_data.alt_minute = mb_swap_short(write_data.alt_minute);
		write_data.alt_second = mb_swap_short(write_data.alt_second);

		/* additional navigation and depths */
		mb_swap_float(&write_data.course_true);
		mb_swap_float(&write_data.speed_transverse);
		mb_swap_float(&write_data.speed);
		mb_swap_float(&write_data.pitch);
		write_data.track = mb_swap_short(write_data.track);
		mb_swap_float(&write_data.depth_center);
		mb_swap_float(&write_data.depth_scale);
		write_data.spare = mb_swap_short(write_data.spare);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			write_data.distance[i] = 
				mb_swap_short(write_data.distance[i]);
			write_data.depth[i] = 
				mb_swap_short(write_data.depth[i]);
			}

		/* travel time data */
		mb_swap_float(&write_data.course_ground);
		mb_swap_float(&write_data.speed_ground);
		mb_swap_float(&write_data.heave);
		mb_swap_float(&write_data.roll);
		mb_swap_float(&write_data.time_center);
		mb_swap_float(&write_data.time_scale);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			write_data.time[i] = mb_swap_short(write_data.time[i]);
		for (i=0;i<11;i++)
			mb_swap_float(&write_data.gyro[i]);

		/* amplitude data */
		write_data.trans_strbd = mb_swap_short(write_data.trans_strbd);
		write_data.trans_vert = mb_swap_short(write_data.trans_vert);
		write_data.trans_port = mb_swap_short(write_data.trans_port);
		write_data.pulse_len_strbd 
			= mb_swap_short(write_data.pulse_len_strbd);
		write_data.pulse_len_vert 
			= mb_swap_short(write_data.pulse_len_vert);
		write_data.pulse_len_port 
			= mb_swap_short(write_data.pulse_len_port);
		write_data.gain_start 
			= mb_swap_short(write_data.gain_start);
		write_data.r_compensation_factor 
			= mb_swap_short(write_data.r_compensation_factor);
		write_data.compensation_start 
			= mb_swap_short(write_data.compensation_start);
		write_data.increase_start 
			= mb_swap_short(write_data.increase_start);
		write_data.tvc_near = mb_swap_short(write_data.tvc_near);
		write_data.tvc_far = mb_swap_short(write_data.tvc_far);
		write_data.increase_int_near 
			= mb_swap_short(write_data.increase_int_near);
		write_data.increase_int_far 
			= mb_swap_short(write_data.increase_int_far);
		write_data.gain_center = mb_swap_short(write_data.gain_center);
		mb_swap_float(&write_data.filter_gain);
		write_data.amplitude_center 
			= mb_swap_short(write_data.amplitude_center);
		write_data.echo_duration_center 
			= mb_swap_short(write_data.echo_duration_center);
		write_data.echo_scale_center 
			= mb_swap_short(write_data.echo_scale_center);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			{
			write_data.amplitude[i] 
				= mb_swap_short(write_data.amplitude[i]);
			write_data.echo_duration[i] 
				= mb_swap_short(write_data.echo_duration[i]);
			}
		for (i=0;i<16;i++)
			{
			write_data.gain[i] 
				= mb_swap_short(write_data.gain[i]);
			write_data.echo_scale[i] 
				= mb_swap_short(write_data.echo_scale[i]);
			}

		/* processed backscatter data */
		mb_swap_float(&write_data.back_scale);
		for (i=0;i<MBF_HSLDEOIH_BEAMS;i++)
			write_data.back[i] = mb_swap_short(write_data.back[i]);
		}
#endif

	/* write record to file */
	write_size = sizeof(write_data);
	write_size_short = write_size;
#ifdef BYTESWAPPED
	write_size_short = mb_swap_short(write_size_short);
#endif
	if ((status = fwrite(&write_size_short,1,sizeof(short int),mbfp))
		!= sizeof(short int))
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
                }
	if ((status = fwrite(&write_data,1,sizeof(write_data),
		mbfp)) != write_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
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
int mbr_hsldeoih_wr_comment(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_hsldeoih_struct *data;
int	*error;
{
	char	*function_name = "mbr_hsldeoih_wr_comment";
	int	status = MB_SUCCESS;
	struct mbf_hsldeoih_comment_struct write_data;
	int	write_size;
	short int write_size_short;

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

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to write in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",data->comment);
		}

	/* copy data from internal storage */
	if (status == MB_SUCCESS)
		{
		strncpy(write_data.comment,data->comment,
			MBF_HSLDEOIH_MAXLINE);
		}

	/* write record to file */
	write_size = sizeof(write_data);
	write_size_short = write_size;
#ifdef BYTESWAPPED
	write_size_short = mb_swap_short(write_size_short);
#endif
	if ((status = fwrite(&write_size_short,1,sizeof(short int),mbfp))
		!= sizeof(short int))
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
                }
	if ((status = fwrite(&write_data,1,sizeof(write_data),
		mbfp)) != write_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
        else
                {
                status = MB_SUCCESS;
                *error = MB_ERROR_NO_ERROR;
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
