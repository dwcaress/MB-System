/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sb2100rw.c	3/3/94
 *	$Id: mbr_sb2100rw.c,v 4.3 1994-04-09 15:49:21 caress Exp $
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
 * mbr_sb2100rw.c contains the functions for reading and writing
 * multibeam data in the SB2100RW format.  
 * These functions include:
 *   mbr_alm_sb2100rw	- allocate read/write memory
 *   mbr_dem_sb2100rw	- deallocate read/write memory
 *   mbr_rt_sb2100rw	- read and translate data
 *   mbr_wt_sb2100rw	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 3, 1994
 * $Log: not supported by cvs2svn $
 * Revision 4.2  1994/03/25  14:02:38  caress
 * Made changes in accordance with latest iteration of
 * SeaBeam 2100 vendor format.
 *
 * Revision 4.1  1994/03/13  04:48:05  caress
 * Changed order in which parameters are read and written.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/05  02:13:52  caress
 * First cut for MBF_SB2100RW format.
 *
 * Revision 1.1  1994/03/05  02:09:29  caress
 * Initial revision
 *
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
#include "../../include/mbsys_sb2100.h"
#include "../../include/mbf_sb2100rw.h"

/*--------------------------------------------------------------------*/
int mbr_alm_sb2100rw(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	static char res_id[]="$Id: mbr_sb2100rw.c,v 4.3 1994-04-09 15:49:21 caress Exp $";
	char	*function_name = "mbr_alm_sb2100rw";
	int	status = MB_SUCCESS;
	int	i;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100rw_struct *data;
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
	mb_io_ptr->structure_size = sizeof(struct mbf_sb2100rw_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_sb2100_struct),
				&mb_io_ptr->store_data,error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_sb2100rw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;

	/* initialize everything to zeros */
	mbr_zero_sb2100rw(verbose,data_ptr,error);

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
int mbr_dem_sb2100rw(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_sb2100rw";
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
	status = mb_free(verbose,mb_io_ptr->raw_data,error);
	status = mb_free(verbose,mb_io_ptr->store_data,error);

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
int mbr_zero_sb2100rw(verbose,data_ptr,error)
int	verbose;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_zero_sb2100rw";
	int	status = MB_SUCCESS;
	struct mbf_sb2100rw_struct *data;
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
	data = (struct mbf_sb2100rw_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		/* type of data record */
		data->kind = MB_DATA_NONE;

		/* time stamp (all records ) */
		data->year = 0;
		data->jday = 0;
		data->hour = 0;
		data->minute = 0;
		data->msec = 0;

		/* sonar parameters (PR) */
		data->roll_bias_port = 0;
		data->roll_bias_starboard = 0;
		data->pitch_bias = 0;
		data->num_svp = 0;
		for (i=0;i<MBF_SB2100RW_MAXVEL;i++)
			{
			data->vdepth[i] = 0;
			data->velocity[i] = 0;
			}

		/* DR and SS header info */
		data->longitude = 0.0;
		data->latitude = 0.0;
		data->speed = 0.0;
		data->surface_sound_velocity = 0;
		data->ssv_source = 'U';
		data->depth_gate_mode = 'U';

		/* DR header info */
		data->num_beams = 0;
		data->svp_corr_beams = '0';
		for (i=0;i<8;i++)
			data->spare[i] = ' ';
		data->range_scale = 'D';
		data->num_algorithms = 1;
		for (i=0;i<4;i++)
			data->algorithm_order[i] = ' ';

		/* SS header info */
		data->ss_data_length = 0;
		data->pixel_algorithm = 'D';
		data->svp_corr_ss = '0';
		data->num_pixels_12khz = 0;
		data->pixel_size_12khz = 0;
		data->num_pixels_36khz = 0;
		data->pixel_size_36khz = 0;

		/* transmit parameters and navigation (DR and SS) */
		data->frequency[0] = 'L';
		data->frequency[1] = 'L';
		data->ping_gain_12khz = 0;
		data->ping_pulse_width_12khz = 0;
		data->transmitter_attenuation_12khz = 0;
		data->pitch_12khz = 0;
		data->roll_12khz = 0;
		data->heading_12khz = 0;
		data->ping_gain_36khz = 0;
		data->ping_pulse_width_36khz = 0;
		data->transmitter_attenuation_36khz = 0;
		data->pitch_36khz = 0;
		data->roll_36khz = 0;
		data->heading_36khz = 0;

		/* formed beam data (DR) */
		for (i=0;i<MBF_SB2100RW_BEAMS;i++)
			{
			data->source[i] = 'U';
			data->travel_time[i] = 0;
			data->angle_across[i] = 0;
			data->angle_forward[i] = 0;
			data->depth[i] = 0;
			data->acrosstrack_beam[i] = 0;
			data->alongtrack_beam[i] = 0;
			data->amplitude_beam[i] = 0;
			data->signal_to_noise[i] = 0;
			data->echo_length[i] = 0;
			data->quality[i] = 'U';
			}

		/* sidescan data (SS) */
		for (i=0;i<MBF_SB2100RW_PIXELS;i++)
			{
			data->amplitude_ss[i] = 0;
			data->alongtrack_ss[i] = 0;
			}

		/* comment (TR) */
		strncpy(data->comment,"\0",MBF_SB2100RW_MAXLINE);

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
int mbr_rt_sb2100rw(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_sb2100rw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100rw_struct *data;
	struct mbsys_sb2100_struct *store;
	int	time_j[4];
	double	scale;
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
	data = (struct mbf_sb2100rw_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_sb2100_struct *) store_ptr;

	/* reset values in mb_io_ptr */
	mb_io_ptr->new_kind = MB_DATA_NONE;
	mb_io_ptr->new_time_i[0] = 0;
	mb_io_ptr->new_time_i[1] = 0;
	mb_io_ptr->new_time_i[2] = 0;
	mb_io_ptr->new_time_i[3] = 0;
	mb_io_ptr->new_time_i[4] = 0;
	mb_io_ptr->new_time_i[5] = 0;
	mb_io_ptr->new_time_d = 0.0;
	mb_io_ptr->new_lon = 0.0;
	mb_io_ptr->new_lat = 0.0;
	mb_io_ptr->new_heading = 0.0;
	mb_io_ptr->new_speed = 0.0;
	for (i=0;i<mb_io_ptr->beams_bath;i++)
		{
		mb_io_ptr->new_bath[i] = 0;
		mb_io_ptr->new_bath_acrosstrack[i] = 0;
		mb_io_ptr->new_bath_alongtrack[i] = 0;
		}
	for (i=0;i<mb_io_ptr->beams_amp;i++)
		{
		mb_io_ptr->new_amp[i] = 0;
		}
	for (i=0;i<mb_io_ptr->pixels_ss;i++)
		{
		mb_io_ptr->new_ss[i] = 0;
		mb_io_ptr->new_ss_acrosstrack[i] = 0;
		mb_io_ptr->new_ss_alongtrack[i] = 0;
		}

	/* read next data from file */
	status = mbr_sb2100rw_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate time values to current 
		ping variables in mbio descriptor structure */
	if (status == MB_SUCCESS)
		{
		/* get time */
		time_j[0] = data->year;
		time_j[1] = data->jday;
		time_j[2] = 60*data->hour + data->minute;
		time_j[3] = 0.001*data->msec;
		mb_get_itime(verbose,time_j,mb_io_ptr->new_time_i);
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
			fprintf(stderr,"dbg4       time_d:     %f\n",
				mb_io_ptr->new_time_d);
			}
		}

	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_DATA)
		{
		/* get navigation */
		mb_io_ptr->new_lon = data->longitude;
		mb_io_ptr->new_lat = data->latitude;
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
		if (data->frequency[0] != 'H')
			mb_io_ptr->new_heading = 0.01*data->heading_12khz;
		else
			mb_io_ptr->new_heading = 0.01*data->heading_36khz;

		/* get speed */
		mb_io_ptr->new_speed = 360.0*data->speed;

		/* read beam and pixel values into storage arrays */
		mb_io_ptr->beams_bath = data->num_beams;
		mb_io_ptr->beams_amp = data->num_beams;
		mb_io_ptr->pixels_ss = data->num_pixels;
		if (data->range_scale == 'S')
			scale = 0.01;
		else
			scale = 1.0;
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			mb_io_ptr->new_bath[i] 
				= scale*data->depth[i];
			mb_io_ptr->new_bath_acrosstrack[i] 
				= scale*data->acrosstrack_beam[i];
			mb_io_ptr->new_bath_alongtrack[i] 
				= scale*data->alongtrack_beam[i];
			}
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			mb_io_ptr->new_amp[i] 
				= data->amplitude_beam[i];
			}
		for (i=0;i<mb_io_ptr->pixels_ss;i++)
			{
			mb_io_ptr->new_ss[i] 
				= data->amplitude_ss[i];
			mb_io_ptr->new_ss_acrosstrack[i] 
				= scale*data->pixel_size_12khz*
					(i - MBF_SB2100RW_CENTER_PIXEL);
			mb_io_ptr->new_ss_alongtrack[i] 
				= scale*data->alongtrack_ss[i];
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
			  fprintf(stderr,"dbg4       beam:%d  bath:%d  amp:%d  acrosstrack:%d  alongtrack:%d\n",
				i,mb_io_ptr->new_bath[i],
				mb_io_ptr->new_amp[i],
				mb_io_ptr->new_bath_acrosstrack[i],
				mb_io_ptr->new_bath_alongtrack[i]);
			fprintf(stderr,"dbg4       pixels_ss:  %d\n",
				mb_io_ptr->pixels_ss);
			for (i=0;i<mb_io_ptr->pixels_ss;i++)
			  fprintf(stderr,"dbg4       beam:%d  ss:%d  acrosstrack:%d  alongtrack:%d\n",
				i,mb_io_ptr->new_ss[i],
				mb_io_ptr->new_ss_acrosstrack[i],
				mb_io_ptr->new_ss_alongtrack[i]);
			}
		}

	/* copy comment to mbio descriptor structure */
	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(mb_io_ptr->new_comment,data->comment,
			MBF_SB2100RW_MAXLINE);

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

	/* translate values to sb2100 data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* type of data record */
		store->kind = data->kind;

		/* time stamp (all records ) */
		store->year = data->year;
		store->jday = data->jday;
		store->hour = data->hour;
		store->minute = data->minute;
		store->msec = data->msec;

		/* sonar parameters (PR) */
		store->roll_bias_port = data->roll_bias_port;
		store->roll_bias_starboard = data->roll_bias_starboard;
		store->pitch_bias = data->pitch_bias;
		store->num_svp = data->num_svp;
		for (i=0;i<MBF_SB2100RW_MAXVEL;i++)
			{
			store->vdepth[i] = data->vdepth[i];
			store->velocity[i] = data->velocity[i];
			}

		/* DR and SS header info */
		store->longitude = data->longitude;
		store->latitude = data->latitude;
		store->speed = data->speed;
		store->surface_sound_velocity = data->surface_sound_velocity;
		store->ssv_source = data->ssv_source;
		store->depth_gate_mode = data->depth_gate_mode;

		/* DR header info */
		store->num_beams = data->num_beams;
		store->svp_corr_beams = data->svp_corr_beams;
		for (i=0;i<8;i++)
			store->spare[i] = data->spare[i];
		store->range_scale = data->range_scale;
		store->num_algorithms = data->num_algorithms;
		for (i=0;i<4;i++)
			store->algorithm_order[i] = data->algorithm_order[i];

		/* SS header info */
		store->ss_data_length = data->ss_data_length;
		store->num_pixels = data->num_pixels;
		store->pixel_algorithm = data->pixel_algorithm;
		store->svp_corr_ss = data->svp_corr_ss;
		store->num_pixels_12khz = data->num_pixels_12khz;
		store->pixel_size_12khz = data->pixel_size_12khz;
		store->num_pixels_36khz = data->num_pixels_36khz;
		store->pixel_size_36khz = data->pixel_size_36khz;

		/* transmit parameters and navigation (DR and SS) */
		store->frequency[0] = data->frequency[0];
		store->frequency[1] = data->frequency[1];
		store->ping_gain_12khz = data->ping_gain_12khz;
		store->ping_pulse_width_12khz = data->ping_pulse_width_12khz;
		store->transmitter_attenuation_12khz 
			= data->transmitter_attenuation_12khz;
		store->pitch_12khz = data->pitch_12khz;
		store->roll_12khz = data->roll_12khz;
		store->heading_12khz = data->heading_12khz;
		store->ping_gain_36khz = data->ping_gain_36khz;
		store->ping_pulse_width_36khz = data->ping_pulse_width_36khz;
		store->transmitter_attenuation_36khz 
			= data->transmitter_attenuation_36khz;
		store->pitch_36khz = data->pitch_36khz;
		store->roll_36khz = data->roll_36khz;
		store->heading_36khz = data->heading_36khz;

		/* formed beam data (DR) */
		for (i=0;i<MBF_SB2100RW_BEAMS;i++)
			{
			store->source[i] = data->source[i];
			store->travel_time[i] = data->travel_time[i];
			store->angle_across[i] = data->angle_across[i];
			store->angle_forward[i] = data->angle_forward[i];
			store->depth[i] = data->depth[i];
			store->acrosstrack_beam[i] = data->acrosstrack_beam[i];
			store->alongtrack_beam[i] = data->alongtrack_beam[i];
			store->amplitude_beam[i] = data->amplitude_beam[i];
			store->signal_to_noise[i] = data->signal_to_noise[i];
			store->echo_length[i] = data->echo_length[i];
			store->quality[i] = data->quality[i];
			}

		/* sidescan data (SS) */
		for (i=0;i<MBF_SB2100RW_PIXELS;i++)
			{
			store->amplitude_ss[i] = data->amplitude_ss[i];
			store->alongtrack_ss[i] = data->alongtrack_ss[i];
			}

		/* comment (TR) */
		strncpy(store->comment,data->comment,MBF_SB2100RW_MAXLINE);

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
int mbr_wt_sb2100rw(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_sb2100rw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100rw_struct *data;
	char	*data_ptr;
	struct mbsys_sb2100_struct *store;
	double	scalefactor;
	int	time_j[4];
	double	scale;
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
	data = (struct mbf_sb2100rw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	store = (struct mbsys_sb2100_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		/* type of data record */
		data->kind = store->kind;

		/* time stamp (all records ) */
		data->year = store->year;
		data->jday = store->jday;
		data->hour = store->hour;
		data->minute = store->minute;
		data->msec = store->msec;

		/* sonar parameters (PR) */
		data->roll_bias_port = store->roll_bias_port;
		data->roll_bias_starboard = store->roll_bias_starboard;
		data->pitch_bias = store->pitch_bias;
		data->num_svp = store->num_svp;
		for (i=0;i<MBF_SB2100RW_MAXVEL;i++)
			{
			data->vdepth[i] = store->vdepth[i];
			data->velocity[i] = store->velocity[i];
			}

		/* DR and SS header info */
		data->longitude = store->longitude;
		data->latitude = store->latitude;
		data->speed = store->speed;
		data->surface_sound_velocity = store->surface_sound_velocity;
		data->ssv_source = store->ssv_source;
		data->depth_gate_mode = store->depth_gate_mode;

		/* DR header info */
		data->num_beams = store->num_beams;
		data->svp_corr_beams = store->svp_corr_beams;
		for (i=0;i<8;i++)
			data->spare[i] = store->spare[i];
		data->range_scale = store->range_scale;
		data->num_algorithms = store->num_algorithms;
		for (i=0;i<4;i++)
			data->algorithm_order[i] = store->algorithm_order[i];

		/* SS header info */
		data->ss_data_length = store->ss_data_length;
		data->num_pixels = store->num_pixels;
		data->pixel_algorithm = store->pixel_algorithm;
		data->svp_corr_ss = store->svp_corr_ss;
		data->num_pixels_12khz = store->num_pixels_12khz;
		data->pixel_size_12khz = store->pixel_size_12khz;
		data->num_pixels_36khz = store->num_pixels_36khz;
		data->pixel_size_36khz = store->pixel_size_36khz;

		/* transmit parameters and navigation (DR and SS) */
		data->frequency[0] = store->frequency[0];
		data->frequency[1] = store->frequency[1];
		data->ping_gain_12khz = store->ping_gain_12khz;
		data->ping_pulse_width_12khz = store->ping_pulse_width_12khz;
		data->transmitter_attenuation_12khz 
			= store->transmitter_attenuation_12khz;
		data->pitch_12khz = store->pitch_12khz;
		data->roll_12khz = store->roll_12khz;
		data->heading_12khz = store->heading_12khz;
		data->ping_gain_36khz = store->ping_gain_36khz;
		data->ping_pulse_width_36khz = store->ping_pulse_width_36khz;
		data->transmitter_attenuation_36khz 
			= store->transmitter_attenuation_36khz;
		data->pitch_36khz = store->pitch_36khz;
		data->roll_36khz = store->roll_36khz;
		data->heading_36khz = store->heading_36khz;

		/* formed beam data (DR) */
		for (i=0;i<MBF_SB2100RW_BEAMS;i++)
			{
			data->source[i] = store->source[i];
			data->travel_time[i] = store->travel_time[i];
			data->angle_across[i] = store->angle_across[i];
			data->angle_forward[i] = store->angle_forward[i];
			data->depth[i] = store->depth[i];
			data->acrosstrack_beam[i] = store->acrosstrack_beam[i];
			data->alongtrack_beam[i] = store->alongtrack_beam[i];
			data->amplitude_beam[i] = store->amplitude_beam[i];
			data->signal_to_noise[i] = store->signal_to_noise[i];
			data->echo_length[i] = store->echo_length[i];
			data->quality[i] = store->quality[i];
			}

		/* sidescan data (SS) */
		for (i=0;i<MBF_SB2100RW_PIXELS;i++)
			{
			data->amplitude_ss[i] = store->amplitude_ss[i];
			data->alongtrack_ss[i] = store->alongtrack_ss[i];
			}

		/* comment (TR) */
		strncpy(data->comment,store->comment,MBF_SB2100RW_MAXLINE);

		}

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		data->kind = mb_io_ptr->new_kind;

	/* set times from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		{
		/* get time */
		mb_get_jtime(verbose,mb_io_ptr->new_time_i,time_j);
		data->year = time_j[0];
		data->jday = time_j[1];
		data->hour = time_j[2]/60;
		data->minute = time_j[2] - 60*data->hour;
		data->msec = 1000*time_j[3];
		}

	/* check for comment to be copied from mb_io_ptr */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		strncpy(data->comment,mb_io_ptr->new_comment,
			MBF_SB2100RW_MAXLINE);
		}

	/* else check for ping data to be copied from mb_io_ptr */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/* get number of beams and pixels */
		data->num_beams = mb_io_ptr->beams_bath;
		data->num_pixels = mb_io_ptr->pixels_ss;

		/* get navigation */
		data->longitude = mb_io_ptr->new_lon;
		data->latitude = mb_io_ptr->new_lat;

		/* get heading */
		if (data->frequency[0] != 'H')
			data->heading_12khz = 100*mb_io_ptr->new_heading;
		else
			data->heading_36khz = 100*mb_io_ptr->new_heading;
		if (data->heading_12khz < 0)
			data->heading_12khz = data->heading_12khz + 36000;
		if (data->heading_36khz < 0)
			data->heading_36khz = data->heading_36khz + 36000;

		/* get speed */
		data->speed = mb_io_ptr->new_speed/360.0;

		/* read beam and pixel values into storage arrays */
		if (data->range_scale == 'S')
			scale = 100.0;
		else
			{
			scale = 1.0;
			data->range_scale = 'D';
			}
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			 data->depth[i]
				= scale*mb_io_ptr->new_bath[i];
			 data->acrosstrack_beam[i]
				= scale*mb_io_ptr->new_bath_acrosstrack[i];
			 data->alongtrack_beam[i]
				= scale*mb_io_ptr->new_bath_alongtrack[i];
			}
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			 data->amplitude_beam[i]
				= mb_io_ptr->new_amp[i];
			}
		for (i=0;i<mb_io_ptr->pixels_ss;i++)
			{
			data->amplitude_ss[i]
				= mb_io_ptr->new_ss[i];
			if (data->pixel_size_12khz <= 0
				&& mb_io_ptr->new_ss_acrosstrack[i] > 0)
				data->pixel_size_12khz = 
					mb_io_ptr->new_ss_acrosstrack[i]/
					(i - MBF_SB2100RW_CENTER_PIXEL);
			data->alongtrack_ss[i]
				= scale*mb_io_ptr->new_ss_alongtrack[i];
			}
		}

	/* write next data to file */
	status = mbr_sb2100rw_wr_data(verbose,mbio_ptr,data_ptr,error);

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
int mbr_sb2100rw_rd_data(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100rw_struct *data;
	char	*data_ptr;
	FILE	*mbfp;
	int	done;
	int	expect;

	static int line_save_flag = MB_NO;
	static char raw_line[MBF_SB2100RW_MAXLINE] = "\0";
	static int type = MBF_SB2100RW_NONE;

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
	data = (struct mbf_sb2100rw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	mbfp = mb_io_ptr->mbfp;

	/* initialize everything to zeros */
	mbr_zero_sb2100rw(verbose,data_ptr,error);

	done = MB_NO;
	expect = MBF_SB2100RW_NONE;
	while (done == MB_NO)
		{

		/* get next record label */
		if (line_save_flag == MB_NO)
			status = mbr_sb2100rw_rd_label(verbose,mbfp,
				raw_line,&type,error);
		else
			line_save_flag = MB_NO;

		/* read the appropriate data records */
		if (status == MB_FAILURE && expect == MBF_SB2100RW_NONE)
			{
			done = MB_YES;
			}
		else if (status == MB_FAILURE && expect != MBF_SB2100RW_NONE)
			{
			done = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else if (expect != MBF_SB2100RW_NONE && expect != type)
			{
			done = MB_YES;
				expect = MBF_SB2100RW_NONE;
			line_save_flag = MB_YES;
			}
		else if (type == MBF_SB2100RW_RAW_LINE)
			{
			strcpy(data->comment,raw_line);
			done = MB_YES;
			data->kind = MB_DATA_RAW_LINE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			status = MB_FAILURE;
			}
		else if (type == MBF_SB2100RW_PR)
			{
			status = mbr_sb2100rw_rd_pr(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_PARAMETER;
				}
			}
		else if (type == MBF_SB2100RW_TR)
			{
			status = mbr_sb2100rw_rd_tr(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_COMMENT;
				}
			}
		else if (type == MBF_SB2100RW_DR)
			{
			status = mbr_sb2100rw_rd_dr(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_NO;
				data->kind = MB_DATA_DATA;
				expect = MBF_SB2100RW_SS;
				}
			}
		else if (type == MBF_SB2100RW_SS)
			{
			status = mbr_sb2100rw_rd_ss(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS 
				&& expect == MBF_SB2100RW_SS)
				{
				done = MB_YES;
				}
			else if (status == MB_SUCCESS)
				{
				done = MB_YES;
				expect = MBF_SB2100RW_NONE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
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
int	mbr_sb2100rw_rd_label(verbose,mbfp,line,type,error)
int	verbose;
FILE	*mbfp;
char	*line;
int	*type;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_rd_label";
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
	status = mbr_sb2100rw_read_line(verbose,mbfp,1,line,error);

	/* see if we just encountered an identifier record */
	if (status == MB_SUCCESS)
		{
		*type = MBF_SB2100RW_RAW_LINE;
		for (i=1;i<MBF_SB2100RW_RECORDS;i++)
			{
			icmp = strncmp(line,mbf_sb2100rw_labels[i],8);
			if (icmp == 0) 
				*type = i;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       line:       %s\n",line);
		fprintf(stderr,"dbg2       type:       %d\n",*type);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int	mbr_sb2100rw_read_line(verbose,mbfp,minimum_size,line,error)
int	verbose;
FILE	*mbfp;
int	minimum_size;
char	*line;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_read_line";
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
		strncpy(line,"\0",MBF_SB2100RW_MAXLINE);
		result = fgets(line,MBF_SB2100RW_MAXLINE,mbfp);

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
int mbr_sb2100rw_rd_pr(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_sb2100rw_struct *data;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_rd_pr";
	int	status = MB_SUCCESS;
	char	line[MBF_SB2100RW_MAXLINE];
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

	/* read and parse data from first line of record */
	status = mbr_sb2100rw_read_line(verbose,mbfp,1,line,error);

	/* parse data from first line */
	if (status == MB_SUCCESS)
		{
		mb_get_int(&(data->year),                 line,     4);
		mb_get_int(&(data->jday),                 line+4,   3);
		mb_get_int(&(data->hour),                 line+7,   2);
		mb_get_int(&(data->minute),               line+9,   2);
		mb_get_int(&(data->msec),                 line+11,  5);
		mb_get_int(&(data->roll_bias_port),       line+16,  6);
		mb_get_int(&(data->roll_bias_starboard),  line+22,  6);
		mb_get_int(&(data->pitch_bias),           line+28,  6);
		mb_get_int(&(data->num_svp),              line+34,  2);
		}

	/* read and parse data from other lines of record */
	for (i=0;i<data->num_svp;i++)
		{
		if ((status = mbr_sb2100rw_read_line(verbose,mbfp,1,line,error))
			== MB_SUCCESS)
			{
			mb_get_int(&(data->vdepth[i]),line,7);
			mb_get_int(&(data->velocity[i]),line+7,6);
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->minute);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       roll_bias_port:   %d\n",data->roll_bias_port);
		fprintf(stderr,"dbg5       roll_bias_strbrd: %d\n",data->roll_bias_starboard);
		fprintf(stderr,"dbg5       pitch_bias:       %d\n",data->pitch_bias);
		fprintf(stderr,"dbg5       num_svp:          %d\n",data->num_svp);
		fprintf(stderr,"dbg5       Sound Velocity Profile:\n");
		for (i=0;i<data->num_svp;i++)
			fprintf(stderr,"dbg5       %d  depth:%d  velocity:%d\n",
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
int mbr_sb2100rw_rd_tr(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_sb2100rw_struct *data;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_rd_tr";
	int	status = MB_SUCCESS;
	char	line[MBF_SB2100RW_MAXLINE];
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
		}

	/* read comment record from file */
	status = mbr_sb2100rw_read_line(verbose,mbfp,1,line,error);

	/* copy comment into data structure */
	if (status == MB_SUCCESS)
		{
		nchars = strlen(line);
		strncpy(data->comment,line,nchars-1);
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
int mbr_sb2100rw_rd_dr(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_sb2100rw_struct *data;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_rd_dr";
	int	status = MB_SUCCESS;
	char	line[MBF_SB2100RW_MAXLINE];
	int	shift;
	char	ew, ns;
	int	degrees, minutes;
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

	/* read and parse data from first line of record */
	status = mbr_sb2100rw_read_line(verbose,mbfp,1,line,error);

	/* parse data from first line */
	if (status == MB_SUCCESS)
		{
		/* get time and navigation */
		mb_get_int(&(data->year),                 line,     4);
		mb_get_int(&(data->jday),                 line+4,   3);
		mb_get_int(&(data->hour),                 line+7,   2);
		mb_get_int(&(data->minute),               line+9,   2);
		mb_get_int(&(data->msec),                 line+11,  5);
		ns = line[16];
		mb_get_int(&degrees,    line+17,2);
		mb_get_int(&minutes,    line+19,6);
		data->latitude = degrees + 0.0001*minutes/60.;
		if (ns == 's' || ns == 'S')
			data->latitude = -data->latitude;
		ew = line[25];
		mb_get_int(&degrees,    line+26,3);
		mb_get_int(&minutes,    line+29,6);
		data->longitude = degrees + 0.0001*minutes/60.;
		if (ew == 'W' || ns == 'w')
			data->longitude = -data->longitude;
		mb_get_int(&(data->speed),                 line+35,7);

		/* now get other stuff */
		mb_get_int(&(data->num_beams),            line+42,  4);
		data->svp_corr_beams = line[46];
		data->frequency[0] = line[47];
		data->frequency[1] = line[48];
		for (i=0;i<8;i++)
			data->spare[i] = line[49+i];
		data->range_scale = line[57];
		mb_get_int(&(data->surface_sound_velocity),line+58,6);
		data->ssv_source = line[64];
		data->depth_gate_mode = line[65];

		/* handle 12 kHz parameters if not in 36 kHz mode */
		shift = 66;
		if (data->frequency[0] != 'H')
		  {
		  mb_get_int(&(data->ping_gain_12khz),    line+shift,    2);
		  mb_get_int(&(data->ping_pulse_width_12khz),
							  line+2+shift,  2);
		  mb_get_int(&(data->transmitter_attenuation_12khz),
							  line+4+shift,  2);
		  mb_get_int(&(data->pitch_12khz),        line+6+shift,  6);
		  mb_get_int(&(data->roll_12khz),         line+12+shift, 6);
		  mb_get_int(&(data->heading_12khz),      line+18+shift, 6);
		  shift = shift + 24;
		  }

		/* handle 36 kHz parameters if if in 36 kHz 
			or dual frequency mode */
		if (data->frequency[0] != 'L')
		  {
		  mb_get_int(&(data->ping_gain_36khz),    line+shift,    2);
		  mb_get_int(&(data->ping_pulse_width_36khz),
							  line+2+shift,  2);
		  mb_get_int(&(data->transmitter_attenuation_36khz),
							  line+4+shift,  2);
		  mb_get_int(&(data->pitch_36khz),        line+6+shift,  6);
		  mb_get_int(&(data->roll_36khz),         line+12+shift, 6);
		  mb_get_int(&(data->heading_36khz),      line+18+shift, 6);
		  shift = shift + 24;
		  }

		/* now get last things in header */
		mb_get_int(&(data->num_algorithms),       line+shift,  1);
		for (i=0;i<4;i++)
			data->algorithm_order[i] = line[1+shift+i];
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       latitude:         %f\n",
			data->latitude);
		fprintf(stderr,"dbg5       longitude:        %f\n",
			data->longitude);
		fprintf(stderr,"dbg5       speed:            %d\n",
			data->speed);
		fprintf(stderr,"dbg5       num_beams:        %d\n",
			data->num_beams);
		fprintf(stderr,"dbg5       svp_corr_beams:   %c\n",
			data->svp_corr_beams);
		fprintf(stderr,"dbg5       frequency:        %c%c\n",
			data->frequency[0],data->frequency[1]);
		fprintf(stderr,"dbg5       spare:            ");
		for (i=0;i<8;i++)
			fprintf(stderr,"%c",data->spare[i]);
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       range_scale:      %c\n",
			data->range_scale);
		fprintf(stderr,"dbg5       surface_sound_velocity: %d\n",
			data->surface_sound_velocity);
		fprintf(stderr,"dbg5       ssv_source:       %c\n",
			data->ssv_source);
		fprintf(stderr,"dbg5       depth_gate_mode:  %c\n",
			data->depth_gate_mode);
		fprintf(stderr,"dbg5       ping_gain_12khz:  %d\n",
			data->ping_gain_12khz);
		fprintf(stderr,"dbg5       ping_pulse_width_12khz:        %d\n",
			data->ping_pulse_width_12khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_12khz: %d\n",
			data->transmitter_attenuation_12khz);
		fprintf(stderr,"dbg5       pitch_12khz:      %d\n",
			data->pitch_12khz);
		fprintf(stderr,"dbg5       roll_12khz:       %d\n",
			data->roll_12khz);
		fprintf(stderr,"dbg5       heading_12khz:    %d\n",
			data->heading_12khz);
		fprintf(stderr,"dbg5       ping_gain_36khz:  %d\n",
			data->ping_gain_36khz);
		fprintf(stderr,"dbg5       ping_pulse_width_36khz:        %d\n",
			data->ping_pulse_width_36khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_36khz: %d\n",
			data->transmitter_attenuation_36khz);
		fprintf(stderr,"dbg5       pitch_36khz:      %d\n",
			data->pitch_36khz);
		fprintf(stderr,"dbg5       roll_36khz:       %d\n",
			data->roll_36khz);
		fprintf(stderr,"dbg5       heading_36khz:    %d\n",
			data->heading_36khz);
		fprintf(stderr,"dbg5       num_algorithms:   %d\n",
			data->num_algorithms);
		fprintf(stderr,"dbg5       algorithm_order:  ");
		for (i=0;i<4;i++)
			fprintf(stderr,"%c",data->algorithm_order[i]);
		fprintf(stderr,"\n");
		}

	/* read and parse data from subsequent lines of record
		- one line per beam */
	for (i=0;i<data->num_beams;i++)
	  {
	  if ((status = mbr_sb2100rw_read_line(verbose,mbfp,1,line,error)) 
		== MB_SUCCESS)
		{
		data->source[i] = line[0];
		mb_get_int(&(data->travel_time[i]),      line+1,  5);
		mb_get_int(&(data->angle_across[i]),     line+6,  6);
		mb_get_int(&(data->angle_forward[i]),    line+12, 5);
		mb_get_int(&(data->depth[i]),            line+17, 5);
		mb_get_int(&(data->acrosstrack_beam[i]), line+22, 6);
		mb_get_int(&(data->alongtrack_beam[i]),  line+28, 6);
		mb_get_int(&(data->amplitude_beam[i]),   line+34, 3);
		mb_get_int(&(data->signal_to_noise[i]),  line+37, 2);
		mb_get_int(&(data->echo_length[i]),      line+39, 3);
		data->quality[i] = line[42];
		}
	  }

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"dbg5       beam src tt angle angfor depth xtrack ltrack amp sig2noise echo quality\n");
		for (i=0;i<data->num_beams;i++)
		  {
		  fprintf(stderr,"dbg5       %3d %c %5d %6d %5d %5d %6d %6d %3d %2d %3d %c\n",
			i,
			data->source[i],
			data->travel_time[i],
			data->angle_across[i],
			data->angle_forward[i],
			data->depth[i],
			data->acrosstrack_beam[i],
			data->alongtrack_beam[i],
			data->amplitude_beam[i],
			data->signal_to_noise[i],
			data->echo_length[i],
			data->quality[i]);
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
int mbr_sb2100rw_rd_ss(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_sb2100rw_struct *data;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_rd_ss";
	int	status = MB_SUCCESS;
	char	line[MBF_SB2100RW_MAXLINE];
	int	shift;
	char	ew, ns;
	short int	read_ss[2*MBF_SB2100RW_PIXELS+2];
	int	degrees, minutes;
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

	/* read and parse data from first line of record */
	status = mbr_sb2100rw_read_line(verbose,mbfp,1,line,error);

	/* parse data from first line */
	if (status == MB_SUCCESS)
		{
		/* get time and navigation */
		mb_get_int(&(data->year),                 line,     4);
		mb_get_int(&(data->jday),                 line+4,   3);
		mb_get_int(&(data->hour),                 line+7,   2);
		mb_get_int(&(data->minute),               line+9,   2);
		mb_get_int(&(data->msec),                 line+11,  5);
		ns = line[16];
		mb_get_int(&degrees,    line+17,2);
		mb_get_int(&minutes,    line+19,6);
		data->latitude = degrees + 0.0001*minutes/60.;
		if (ns == 's' || ns == 'S')
			data->latitude = -data->latitude;
		ew = line[25];
		mb_get_int(&degrees,    line+26,3);
		mb_get_int(&minutes,    line+29,6);
		data->longitude = degrees + 0.0001*minutes/60.;
		if (ew == 'W' || ns == 'w')
			data->longitude = -data->longitude;
		mb_get_int(&(data->speed),                 line+35,  7);

		/* now get other stuff */
		mb_get_int(&(data->ss_data_length),        line+42,  4);
		data->num_pixels = (data->ss_data_length)/4;
		data->svp_corr_beams = line[46];
		data->frequency[0] = line[47];
		data->frequency[1] = line[48];
		for (i=0;i<8;i++)
			data->spare[i] = line[49+i];
		data->range_scale = line[57];
		data->pixel_algorithm = line[58];
		mb_get_int(&(data->surface_sound_velocity),line+59,6);
		data->ssv_source = line[65];
		data->depth_gate_mode = line[66];

		/* handle 12 kHz parameters if not in 36 kHz mode */
		shift = 67;
		if (data->frequency[0] != 'H')
		  {
		  mb_get_int(&(data->num_pixels_12khz),   line+shift,     4);
		  mb_get_int(&(data->pixel_size_12khz),   line+4+shift,   4);
		  mb_get_int(&(data->ping_gain_12khz),    line+8+shift,   2);
		  mb_get_int(&(data->ping_pulse_width_12khz),
							  line+10+shift,   2);
		  mb_get_int(&(data->transmitter_attenuation_12khz),
							  line+12+shift,  2);
		  mb_get_int(&(data->pitch_12khz),        line+14+shift,  6);
		  mb_get_int(&(data->roll_12khz),         line+20+shift,  6);
		  mb_get_int(&(data->heading_12khz),      line+26+shift,  6);
		  shift = shift + 32;
		  }

		/* handle 36 kHz parameters if if in 36 kHz 
			or dual frequency mode */
		if (data->frequency[0] != 'L')
		  {
		  mb_get_int(&(data->num_pixels_36khz),   line+shift,  4);
		  mb_get_int(&(data->pixel_size_36khz),   line+4+shift,  4);
		  mb_get_int(&(data->ping_gain_36khz),    line+8+shift,2);
		  mb_get_int(&(data->ping_pulse_width_36khz),
							  line+10+shift,2);
		  mb_get_int(&(data->transmitter_attenuation_36khz),
							  line+12+shift,2);
		  mb_get_int(&(data->pitch_36khz),        line+14+shift,6);
		  mb_get_int(&(data->roll_36khz),         line+20+shift,6);
		  mb_get_int(&(data->heading_36khz),      line+26+shift,6);
		  shift = shift + 32;
		  }
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       latitude:         %f\n",
			data->latitude);
		fprintf(stderr,"dbg5       longitude:        %f\n",
			data->longitude);
		fprintf(stderr,"dbg5       speed:            %d\n",
			data->speed);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       num_pixels:       %d\n",
			data->num_pixels);
		fprintf(stderr,"dbg5       svp_corr_beams:   %c\n",
			data->svp_corr_beams);
		fprintf(stderr,"dbg5       frequency:        %c%c\n",
			data->frequency[0],data->frequency[1]);
		fprintf(stderr,"dbg5       spare:            ");
		for (i=0;i<8;i++)
			fprintf(stderr,"%c",data->spare[i]);
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       range_scale:      %c\n",
			data->range_scale);
		fprintf(stderr,"dbg5       pixel_algorithm:  %c\n",
			data->pixel_algorithm);
		fprintf(stderr,"dbg5       surface_sound_velocity: %d\n",
			data->surface_sound_velocity);
		fprintf(stderr,"dbg5       ssv_source:       %c\n",
			data->ssv_source);
		fprintf(stderr,"dbg5       depth_gate_mode:  %c\n",
			data->depth_gate_mode);
		fprintf(stderr,"dbg5       num_pixels_12khz: %d\n",
			data->num_pixels_12khz);
		fprintf(stderr,"dbg5       pixel_size_12khz: %d\n",
			data->pixel_size_12khz);
		fprintf(stderr,"dbg5       ping_gain_12khz:  %d\n",
			data->ping_gain_12khz);
		fprintf(stderr,"dbg5       ping_pulse_width_12khz:        %d\n",
			data->ping_pulse_width_12khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_12khz: %d\n",
			data->transmitter_attenuation_12khz);
		fprintf(stderr,"dbg5       pitch_12khz:      %d\n",
			data->pitch_12khz);
		fprintf(stderr,"dbg5       roll_12khz:       %d\n",
			data->roll_12khz);
		fprintf(stderr,"dbg5       heading_12khz:    %d\n",
			data->heading_12khz);
		fprintf(stderr,"dbg5       num_pixels_36khz: %d\n",
			data->num_pixels_36khz);
		fprintf(stderr,"dbg5       pixel_size_36khz: %d\n",
			data->pixel_size_36khz);
		fprintf(stderr,"dbg5       ping_gain_36khz:  %d\n",
			data->ping_gain_36khz);
		fprintf(stderr,"dbg5       ping_pulse_width_36khz:        %d\n",
			data->ping_pulse_width_36khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_36khz: %d\n",
			data->transmitter_attenuation_36khz);
		fprintf(stderr,"dbg5       pitch_36khz:      %d\n",
			data->pitch_36khz);
		fprintf(stderr,"dbg5       roll_36khz:       %d\n",
			data->roll_36khz);
		fprintf(stderr,"dbg5       heading_36khz:    %d\n",
			data->heading_36khz);
		fprintf(stderr,"dbg5       ss_data_length:   %d\n",
			data->ss_data_length);
		}

	/* read sidescan data from character array */
	if ((status = fread(read_ss,1,data->ss_data_length+2,
		mbfp)) != data->ss_data_length+2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	else 
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	if (status == MB_SUCCESS)
		{
		for (i=0;i<data->num_pixels;i++)
			{
			data->amplitude_ss[i] = (int) read_ss[2*i];
			data->alongtrack_ss[i] = (int) read_ss[2*i+1];
	  		}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"dbg5       beam amp_ss ltrack\n");
		for (i=0;i<data->num_pixels;i++)
		  {
		  fprintf(stderr,"dbg5       %3d %6d %6d\n",
			i,
			data->amplitude_ss[i],
			data->alongtrack_ss[i]);
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
int mbr_sb2100rw_wr_data(verbose,mbio_ptr,data_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100rw_struct *data;
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
	data = (struct mbf_sb2100rw_struct *) data_ptr;
	mbfp = mb_io_ptr->mbfp;

	if (data->kind == MB_DATA_RAW_LINE)
		{
		status = mbr_sb2100rw_wr_rawline(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_PARAMETER)
		{
		status = mbr_sb2100rw_wr_pr(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_COMMENT)
		{
		status = mbr_sb2100rw_wr_tr(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_DATA)
		{
		status = mbr_sb2100rw_wr_dr(verbose,mbfp,data,error);
		status = mbr_sb2100rw_wr_ss(verbose,mbfp,data,error);
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
int	mbr_sb2100rw_wr_label(verbose,mbfp,type,error)
int	verbose;
FILE	*mbfp;
char	type;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_wr_label";
	int	status = MB_SUCCESS;
	char	line[MBF_SB2100RW_MAXLINE];

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
	sprintf(line,"%8s\r\n",mbf_sb2100rw_labels[type]);
	status = mbr_sb2100rw_write_line(verbose,mbfp,line,error);

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
int	mbr_sb2100rw_write_line(verbose,mbfp,line,error)
int	verbose;
FILE	*mbfp;
char	*line;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_write_line";
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
int mbr_sb2100rw_wr_rawline(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_wr_rawline";
	int	status = MB_SUCCESS;
	struct mbf_sb2100rw_struct *data;

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
	data = (struct mbf_sb2100rw_struct *) data_ptr;

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
int mbr_sb2100rw_wr_pr(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_wr_pr";
	int	status = MB_SUCCESS;
	struct mbf_sb2100rw_struct *data;
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
	data = (struct mbf_sb2100rw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->minute);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       roll_bias_port:   %d\n",data->roll_bias_port);
		fprintf(stderr,"dbg5       roll_bias_strbrd: %d\n",data->roll_bias_starboard);
		fprintf(stderr,"dbg5       pitch_bias:       %d\n",data->pitch_bias);
		fprintf(stderr,"dbg5       num_svp:          %d\n",data->num_svp);
		fprintf(stderr,"dbg5       Sound Velocity Profile:\n");
		for (i=0;i<data->num_svp;i++)
			fprintf(stderr,"dbg5       %d  depth:%d  velocity:%d\n",
				i,data->vdepth[i],data->velocity[i]);
		}

	/* write the record label */
	status = mbr_sb2100rw_wr_label(verbose,mbfp,
		MBF_SB2100RW_PR,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the first line */
		status = fprintf(mbfp,"%4d",data->year);
		status = fprintf(mbfp,"%3d",data->jday);
		status = fprintf(mbfp,"%2d",data->hour);
		status = fprintf(mbfp,"%2d",data->minute);
		status = fprintf(mbfp,"%5d",data->msec);
		status = fprintf(mbfp,"%6d",data->roll_bias_port);
		status = fprintf(mbfp,"%6d",data->roll_bias_starboard);
		status = fprintf(mbfp,"%6d",data->pitch_bias);
		status = fprintf(mbfp,"%2d",data->num_svp);
		status = fprintf(mbfp,"\r\n");

		/* output the second line */
		for (i=0;i<data->num_svp;i++)
			{
			status = fprintf(mbfp,"%7d",data->vdepth[i]);
			status = fprintf(mbfp,"%6d",data->velocity[i]);
			status = fprintf(mbfp,"\r\n");
			}
		status = fprintf(mbfp,"\r\n");

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
int mbr_sb2100rw_wr_tr(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_wr_tr";
	int	status = MB_SUCCESS;
	struct mbf_sb2100rw_struct *data;

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
	data = (struct mbf_sb2100rw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",
			data->comment);
		}

	/* write the record label */
	status = mbr_sb2100rw_wr_label(verbose,mbfp,
		MBF_SB2100RW_TR,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the event line */
		status = fprintf(mbfp,"%s\r\n",data->comment);

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
int mbr_sb2100rw_wr_dr(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_wr_dr";
	int	status = MB_SUCCESS;
	struct mbf_sb2100rw_struct *data;
	double	degrees;
	int	idegrees, minutes;
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
	data = (struct mbf_sb2100rw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       latitude:         %f\n",
			data->latitude);
		fprintf(stderr,"dbg5       longitude:        %f\n",
			data->longitude);
		fprintf(stderr,"dbg5       speed:            %d\n",
			data->speed);
		fprintf(stderr,"dbg5       num_beams:        %d\n",
			data->num_beams);
		fprintf(stderr,"dbg5       svp_corr_beams:   %c\n",
			data->svp_corr_beams);
		fprintf(stderr,"dbg5       frequency:        %c%c\n",
			data->frequency[0],data->frequency[1]);
		fprintf(stderr,"dbg5       spare:            ");
		for (i=0;i<8;i++)
			fprintf(stderr,"%c",data->spare[i]);
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       range_scale:      %c\n",
			data->range_scale);
		fprintf(stderr,"dbg5       num_algorithms:   %d\n",
			data->num_algorithms);
		fprintf(stderr,"dbg5       algorithm_order:  ");
		for (i=0;i<4;i++)
			fprintf(stderr,"%c",data->algorithm_order[i]);
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       ping_gain_12khz:  %d\n",
			data->ping_gain_12khz);
		fprintf(stderr,"dbg5       ping_pulse_width_12khz:        %d\n",
			data->ping_pulse_width_12khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_12khz: %d\n",
			data->transmitter_attenuation_12khz);
		fprintf(stderr,"dbg5       pitch_12khz:      %d\n",
			data->pitch_12khz);
		fprintf(stderr,"dbg5       roll_12khz:       %d\n",
			data->roll_12khz);
		fprintf(stderr,"dbg5       heading_12khz:    %d\n",
			data->heading_12khz);
		fprintf(stderr,"dbg5       ping_gain_36khz:  %d\n",
			data->ping_gain_36khz);
		fprintf(stderr,"dbg5       ping_pulse_width_36khz:        %d\n",
			data->ping_pulse_width_36khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_36khz: %d\n",
			data->transmitter_attenuation_36khz);
		fprintf(stderr,"dbg5       pitch_36khz:      %d\n",
			data->pitch_36khz);
		fprintf(stderr,"dbg5       roll_36khz:       %d\n",
			data->roll_36khz);
		fprintf(stderr,"dbg5       heading_36khz:    %d\n",
			data->heading_36khz);
		fprintf(stderr,"dbg5       surface_sound_velocity: %d\n",
			data->surface_sound_velocity);
		fprintf(stderr,"dbg5       ssv_source:       %c\n",
			data->ssv_source);
		fprintf(stderr,"dbg5       depth_gate_mode:  %c\n",
			data->depth_gate_mode);
		fprintf(stderr,"dbg5       beam src tt angle angfor depth xtrack ltrack amp sig2noise echo quality\n");
		for (i=0;i<data->num_beams;i++)
		  {
		  fprintf(stderr,"dbg5       %3d %c %5d %6d %5d %5d %6d %6d %3d %2d %3d %c\n",
			i,
			data->source[i],
			data->travel_time[i],
			data->angle_across[i],
			data->angle_forward[i],
			data->depth[i],
			data->acrosstrack_beam[i],
			data->alongtrack_beam[i],
			data->amplitude_beam[i],
			data->signal_to_noise[i],
			data->echo_length[i],
			data->quality[i]);
		  }
		}

	/* write the record label */
	status = mbr_sb2100rw_wr_label(verbose,mbfp,
		MBF_SB2100RW_DR,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the first line */
		status = fprintf(mbfp,"%4d",data->year);
		status = fprintf(mbfp,"%3d",data->jday);
		status = fprintf(mbfp,"%2d",data->hour);
		status = fprintf(mbfp,"%2d",data->minute);
		status = fprintf(mbfp,"%5d",data->msec);
		degrees = data->latitude;
		if (degrees < 0.0)
			{
			status = fprintf(mbfp,"S");
			degrees = -degrees;
			}
		else
			status = fprintf(mbfp,"N");
		idegrees = (int) degrees;
		minutes = (int) (600000.0*(degrees - idegrees));
		status = fprintf(mbfp,"%2d",idegrees);
		status = fprintf(mbfp,"%6d",minutes);
		degrees = data->longitude;
		if (degrees < -180.0)
			degrees = degrees + 360.0;
		if (degrees > 180.0)
			degrees = degrees - 360.0;
		if (degrees < 0.0)
			{
			status = fprintf(mbfp,"W");
			degrees = -degrees;
			}
		else
			status = fprintf(mbfp,"E");
		idegrees = (int) degrees;
		minutes = (int) (600000.0*(degrees - idegrees));
		status = fprintf(mbfp,"%3d",idegrees);
		status = fprintf(mbfp,"%6d",minutes);
		status = fprintf(mbfp,"%7d",data->speed);
		status = fprintf(mbfp,"%4d",data->num_beams);
		status = fprintf(mbfp,"%c",data->svp_corr_beams);
		status = fprintf(mbfp,"%c%c",
			data->frequency[0],data->frequency[1]);
		for (i=0;i<8;i++)
			status = fprintf(mbfp,"%c",data->spare[i]);
		status = fprintf(mbfp,"%c",data->range_scale);
		status = fprintf(mbfp,"%6d",data->surface_sound_velocity);
		status = fprintf(mbfp,"%c",data->ssv_source);
		status = fprintf(mbfp,"%c",data->depth_gate_mode);
		if (data->frequency[0] != 'H')
			{
			status = fprintf(mbfp,"%2d",data->ping_gain_12khz);
			status = fprintf(mbfp,"%2d",
				data->ping_pulse_width_12khz);
			status = fprintf(mbfp,"%2d",
				data->transmitter_attenuation_12khz);
			status = fprintf(mbfp,"%6d",data->pitch_12khz);
			status = fprintf(mbfp,"%6d",data->roll_12khz);
			status = fprintf(mbfp,"%6d",data->heading_12khz);
			}
		if (data->frequency[0] != 'L')
			{
			status = fprintf(mbfp,"%2d",data->ping_gain_36khz);
			status = fprintf(mbfp,"%2d",data->
				ping_pulse_width_36khz);
			status = fprintf(mbfp,"%2d",
				data->transmitter_attenuation_36khz);
			status = fprintf(mbfp,"%6d",data->pitch_36khz);
			status = fprintf(mbfp,"%6d",data->roll_36khz);
			status = fprintf(mbfp,"%6d",data->heading_36khz);
			}
		status = fprintf(mbfp,"%1d",data->num_algorithms);
		for (i=0;i<4;i++)
			status = fprintf(mbfp,"%c",data->algorithm_order[i]);
		status = fprintf(mbfp,"\r\n");

		/* output a line for each beam */
		for (i=0;i<data->num_beams;i++)
			{
			status = fprintf(mbfp,"%c",data->source[i]);
			status = fprintf(mbfp,"%5d",data->travel_time[i]);
			status = fprintf(mbfp,"%6d",data->angle_across[i]);
			status = fprintf(mbfp,"%5d",data->angle_forward[i]);
			status = fprintf(mbfp,"%5d",data->depth[i]);
			status = fprintf(mbfp,"%6d",data->acrosstrack_beam[i]);
			status = fprintf(mbfp,"%6d",data->alongtrack_beam[i]);
			status = fprintf(mbfp,"%3d",data->amplitude_beam[i]);
			status = fprintf(mbfp,"%2d",data->signal_to_noise[i]);
			status = fprintf(mbfp,"%3d",data->echo_length[i]);
			status = fprintf(mbfp,"%c",data->quality[i]);
			status = fprintf(mbfp,"\r\n");
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
int mbr_sb2100rw_wr_ss(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_sb2100rw_wr_ss";
	int	status = MB_SUCCESS;
	struct mbf_sb2100rw_struct *data;
	short int	write_ss[2*MBF_SB2100RW_PIXELS];
	double	degrees;
	int	idegrees, minutes;
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
	data = (struct mbf_sb2100rw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       latitude:         %f\n",
			data->latitude);
		fprintf(stderr,"dbg5       longitude:        %f\n",
			data->longitude);
		fprintf(stderr,"dbg5       speed:            %d\n",
			data->speed);
		fprintf(stderr,"dbg5       num_pixels:       %d\n",
			data->num_pixels);
		fprintf(stderr,"dbg5       ss_data_length:   %d\n",
			data->ss_data_length);
		fprintf(stderr,"dbg5       svp_corr_beams:   %c\n",
			data->svp_corr_beams);
		fprintf(stderr,"dbg5       frequency:        %c%c\n",
			data->frequency[0],data->frequency[1]);
		fprintf(stderr,"dbg5       spare:            ");
		for (i=0;i<8;i++)
			fprintf(stderr,"%c",data->spare[i]);
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       range_scale:      %c\n",
			data->range_scale);
		fprintf(stderr,"dbg5       pixel_algorithm:  %c\n",
			data->pixel_algorithm);
		fprintf(stderr,"dbg5       surface_sound_velocity: %d\n",
			data->surface_sound_velocity);
		fprintf(stderr,"dbg5       ssv_source:       %c\n",
			data->ssv_source);
		fprintf(stderr,"dbg5       depth_gate_mode:  %c\n",
			data->depth_gate_mode);
		fprintf(stderr,"dbg5       num_pixels_12khz: %d\n",
			data->num_pixels_12khz);
		fprintf(stderr,"dbg5       pixel_size_12khz: %d\n",
			data->pixel_size_12khz);
		fprintf(stderr,"dbg5       ping_gain_12khz:  %d\n",
			data->ping_gain_12khz);
		fprintf(stderr,"dbg5       ping_pulse_width_12khz:        %d\n",
			data->ping_pulse_width_12khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_12khz: %d\n",
			data->transmitter_attenuation_12khz);
		fprintf(stderr,"dbg5       pitch_12khz:      %d\n",
			data->pitch_12khz);
		fprintf(stderr,"dbg5       roll_12khz:       %d\n",
			data->roll_12khz);
		fprintf(stderr,"dbg5       heading_12khz:    %d\n",
			data->heading_12khz);
		fprintf(stderr,"dbg5       num_pixels_36khz: %d\n",
			data->num_pixels_36khz);
		fprintf(stderr,"dbg5       pixel_size_36khz: %d\n",
			data->pixel_size_36khz);
		fprintf(stderr,"dbg5       ping_gain_36khz:  %d\n",
			data->ping_gain_36khz);
		fprintf(stderr,"dbg5       ping_pulse_width_36khz:        %d\n",
			data->ping_pulse_width_36khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_36khz: %d\n",
			data->transmitter_attenuation_36khz);
		fprintf(stderr,"dbg5       pitch_36khz:      %d\n",
			data->pitch_36khz);
		fprintf(stderr,"dbg5       roll_36khz:       %d\n",
			data->roll_36khz);
		fprintf(stderr,"dbg5       heading_36khz:    %d\n",
			data->heading_36khz);
		fprintf(stderr,"dbg5       beam amp_ss ltrack\n");
		for (i=0;i<data->num_pixels;i++)
		  {
		  fprintf(stderr,"dbg5       %3d %6d %6d\n",
			i,
			data->amplitude_ss[i],
			data->alongtrack_ss[i]);
		  }
		}

	/* write the record label */
	status = mbr_sb2100rw_wr_label(verbose,mbfp,
		MBF_SB2100RW_SS,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the event line */
		status = fprintf(mbfp,"%4d",data->year);
		status = fprintf(mbfp,"%3d",data->jday);
		status = fprintf(mbfp,"%2d",data->hour);
		status = fprintf(mbfp,"%2d",data->minute);
		status = fprintf(mbfp,"%5d",data->msec);
		degrees = data->latitude;
		if (degrees < 0.0)
			{
			status = fprintf(mbfp,"S");
			degrees = -degrees;
			}
		else
			status = fprintf(mbfp,"N");
		idegrees = (int) degrees;
		minutes = (int) (600000.0*(degrees - idegrees));
		status = fprintf(mbfp,"%2d",idegrees);
		status = fprintf(mbfp,"%6d",minutes);
		degrees = data->longitude;
		if (degrees < -180.0)
			degrees = degrees + 360.0;
		if (degrees > 180.0)
			degrees = degrees - 360.0;
		if (degrees < 0.0)
			{
			status = fprintf(mbfp,"W");
			degrees = -degrees;
			}
		else
			status = fprintf(mbfp,"E");
		idegrees = (int) degrees;
		minutes = (int) (600000.0*(degrees - idegrees));
		status = fprintf(mbfp,"%3d",idegrees);
		status = fprintf(mbfp,"%6d",minutes);
		status = fprintf(mbfp,"%7d",data->speed);
		data->ss_data_length = 4*data->num_pixels;
		status = fprintf(mbfp,"%4d",data->ss_data_length);
		status = fprintf(mbfp,"%c",data->svp_corr_beams);
		status = fprintf(mbfp,"%c%c",
			data->frequency[0],data->frequency[1]);
		for (i=0;i<8;i++)
			status = fprintf(mbfp,"%c",data->spare[i]);
		status = fprintf(mbfp,"%c",data->range_scale);
		status = fprintf(mbfp,"%c",data->pixel_algorithm);
		status = fprintf(mbfp,"%6d",data->surface_sound_velocity);
		status = fprintf(mbfp,"%c",data->ssv_source);
		status = fprintf(mbfp,"%c",data->depth_gate_mode);
		if (data->frequency[0] != 'H')
			{
			status = fprintf(mbfp,"%4d",data->num_pixels_12khz);
			status = fprintf(mbfp,"%4d",data->pixel_size_12khz);
			status = fprintf(mbfp,"%2d",data->ping_gain_12khz);
			status = fprintf(mbfp,"%2d",
				data->ping_pulse_width_12khz);
			status = fprintf(mbfp,"%2d",
				data->transmitter_attenuation_12khz);
			status = fprintf(mbfp,"%6d",data->pitch_12khz);
			status = fprintf(mbfp,"%6d",data->roll_12khz);
			status = fprintf(mbfp,"%6d",data->heading_12khz);
			}
		if (data->frequency[0] != 'L')
			{
			status = fprintf(mbfp,"%4d",data->num_pixels_36khz);
			status = fprintf(mbfp,"%4d",data->pixel_size_36khz);
			status = fprintf(mbfp,"%2d",data->ping_gain_36khz);
			status = fprintf(mbfp,"%2d",data->
				ping_pulse_width_36khz);
			status = fprintf(mbfp,"%2d",
				data->transmitter_attenuation_36khz);
			status = fprintf(mbfp,"%6d",data->pitch_36khz);
			status = fprintf(mbfp,"%6d",data->roll_36khz);
			status = fprintf(mbfp,"%6d",data->heading_36khz);
			}
		status = fprintf(mbfp,"\r\n");

		/* construct and write out sidescan data */
		for (i=0;i<data->num_pixels;i++)
			{
			write_ss[2*i] = (short int) data->amplitude_ss[i];
			write_ss[2*i+1] 
				= (short int) data->alongtrack_ss[i];
	  		}
		if ((status = fwrite(write_ss,1,data->ss_data_length,mbfp))
			!= data->ss_data_length)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		else
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			status = fprintf(mbfp,"\r\n");
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
