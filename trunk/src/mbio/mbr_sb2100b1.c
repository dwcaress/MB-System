/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sb2100b1.c	3/3/94
 *	$Id: mbr_sb2100b1.c,v 4.0 1997-04-21 17:01:19 caress Exp $
 *
 *    Copyright (c) 1997 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbr_sb2100b1.c contains the functions for reading and writing
 * multibeam data in the SB2100B1 format.  
 * These functions include:
 *   mbr_alm_sb2100b1	- allocate read/write memory
 *   mbr_dem_sb2100b1	- deallocate read/write memory
 *   mbr_rt_sb2100b1	- read and translate data
 *   mbr_wt_sb2100b1	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 3, 1994
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1997/04/17  15:11:34  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 1.1  1997/04/17  15:07:36  caress
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
#include "../../include/mb_define.h"
#include "../../include/mbsys_sb2100.h"
#include "../../include/mbf_sb2100b1.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/*--------------------------------------------------------------------*/
int mbr_alm_sb2100b1(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	static char res_id[]="$Id: mbr_sb2100b1.c,v 4.0 1997-04-21 17:01:19 caress Exp $";
	char	*function_name = "mbr_alm_sb2100b1";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_sb2100_struct *store;

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
	mb_io_ptr->structure_size = sizeof(struct mbf_sb2100b1_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_sb2100_struct),
				&mb_io_ptr->store_data,error);

	/* get store structure pointer */
	store = (struct mbsys_sb2100_struct *) mb_io_ptr->store_data;
				
	/* set comment pointer */
	store->comment = (char *) &(store->roll_bias_port);

	/* initialize everything to zeros */
	mbr_zero_sb2100b1(verbose,mb_io_ptr->raw_data,error);

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
int mbr_dem_sb2100b1(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_sb2100b1";
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
int mbr_zero_sb2100b1(verbose,data_ptr,error)
int	verbose;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_zero_sb2100b1";
	int	status = MB_SUCCESS;
	struct mbf_sb2100b1_struct *data;
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
	data = (struct mbf_sb2100b1_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		/* type of data record */
		data->kind = MB_DATA_NONE;
	
		/* set comment pointer */
		data->comment = (char *) &data->pr_year;
	
		/* sonar parameters (SB21BIPR) */
		data->pr_year = 0;
		data->pr_jday = 0;
		data->pr_hour = 0;
		data->pr_minute = 0;
		data->pr_sec = 0;
		data->pr_msec = 0;
		data->roll_bias_port = 0.0;		/* deg */
		data->roll_bias_starboard = 0.0;	/* deg */
		data->pitch_bias = 0.0;			/* deg */
		data->ship_draft = 0.0;			/* m */
		data->offset_x = 0.0;			/* m */
		data->offset_y = 0.0;			/* m */
		data->offset_z = 0.0;			/* m */
		data->num_svp = 0;
		for (i=0;i<MBF_SB2100B1_MAXVEL;i++)
		    {
		    data->svp[i].depth = 0.0;
		    data->svp[i].velocity = 0.0;
		    }
		
		/* sonar data header (SB21BIDH) */
		data->year = 0;
		data->jday = 0;
		data->hour = 0;
		data->minute = 0;
		data->sec = 0;
		data->msec = 0;
		data->longitude = 0.0;			/* degrees */
		data->latitude = 0.0;			/* degrees */
		data->heading = 0.0;			/* degrees */
		data->speed = 0.0;			/* m/sec */
		data->roll = 0.0;			/* degrees */
		data->pitch = 0.0;			/* degrees */
		data->heave = 0.0;			/* m */
		data->ssv = 0.0;			/* m/sec */
		data->frequency = 'L';			/* L=12kHz; H=36kHz */
		data->depth_gate_mode = 'A';		/* A=Auto, M=Manual */
		data->ping_gain = 0;			/* dB */
		data->ping_pulse_width = 0;		/* msec */
		data->transmitter_attenuation = 0;	/* dB */
		data->ssv_source = 'M';			/* V=Velocimeter, M=Manual, 
							    T=Temperature */
		data->svp_correction = 'T';		/* 0=None; A=True Xtrack 
							    and Apparent Depth;
							    T=True Xtrack and True Depth */
		data->pixel_algorithm = 'L';		/* pixel intensity algorithm
							    D = logarithm, L = linear */
		data->pixel_size = 0.0;			/* m */
		data->nbeams = 0;			/* up to 151 */
		data->npixels = 0;			/* up to 2000 */
		data->spare1 = 0;
		data->spare2 = 0;
		data->spare3 = 0;
		data->spare4 = 0;
		data->spare5 = 0;
		data->spare6 = 0;
	
		/* bathymetry record (SB21BIBR) */
		for (i=0;i<MBF_SB2100B1_BEAMS;i++)
			{
			data->beams[i].depth = 0.0;		/* m */
			data->beams[i].acrosstrack = 0.0;	/* m */
			data->beams[i].alongtrack = 0.0;	/* m */
			data->beams[i].range = 0.0;		/* seconds */
			data->beams[i].angle_across = 0.0;	/* degrees */
			data->beams[i].angle_forward = 0.0;	/* degrees */
			data->beams[i].amplitude = 0;	/* 0.25 dB */
			data->beams[i].signal_to_noise = 0;	/* dB */
			data->beams[i].echo_length = 0;		/* samples */
			data->beams[i].quality = '0';		/* 0=no data, 
							    Q=poor quality, 
							    blank otherwise */
			data->beams[i].source = 'W';		/* B=BDI, W=WMT */
			}
	
		/* sidescan record (SB21BISR) */
		for (i=0;i<MBF_SB2100B1_PIXELS;i++)
			{
			data->pixels[i].amplitude = 0;		/* sidescan value */
			data->pixels[i].alongtrack = 0;		/* 0.1 m */
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
int mbr_rt_sb2100b1(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_sb2100b1";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100b1_struct *data;
	struct mbsys_sb2100_struct *store;
	int	time_j[5];
	double	gain_db;
	double	gain_factor;
	int	center_pixel;
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
	data = (struct mbf_sb2100b1_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_sb2100_struct *) store_ptr;

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
	status = mbr_sb2100b1_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate time values to current 
		ping variables in mbio descriptor structure */
	if (status == MB_SUCCESS 
	    && (data->kind == MB_DATA_DATA
		|| data->kind == MB_DATA_PARAMETER))
		{
		/* get time */
		if (data->kind == MB_DATA_DATA)
		    {
		    time_j[0] = data->year;
		    time_j[1] = data->jday;
		    time_j[2] = 60 * data->hour + data->minute;
		    time_j[3] = data->sec;
		    time_j[4] = 1000 * data->msec;
		    mb_get_itime(verbose,time_j,mb_io_ptr->new_time_i);
		    mb_get_time(verbose,mb_io_ptr->new_time_i,
			    &mb_io_ptr->new_time_d);
		    }
		else if (data->kind == MB_DATA_PARAMETER)
		    {
		    time_j[0] = data->pr_year;
		    time_j[1] = data->pr_jday;
		    time_j[2] = 60 * data->pr_hour + data->pr_minute;
		    time_j[3] = data->pr_sec;
		    time_j[4] = 1000 * data->pr_msec;
		    mb_get_itime(verbose,time_j,mb_io_ptr->new_time_i);
		    mb_get_time(verbose,mb_io_ptr->new_time_i,
			    &mb_io_ptr->new_time_d);
		    }

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
		mb_io_ptr->new_heading = data->heading;

		/* get speed */
		mb_io_ptr->new_speed = 0.18553167*data->speed;

		/* read beam and pixel values into storage arrays */
		mb_io_ptr->beams_bath = data->nbeams;
		mb_io_ptr->beams_amp = data->nbeams;
		mb_io_ptr->pixels_ss = data->npixels;
		center_pixel = data->npixels / 2;
		gain_db = data->ping_gain 
			- data->transmitter_attenuation
			+ 10.0 * log10( data->ping_pulse_width / 5.0)
			- 30.0;
		gain_factor = pow(10.0, (-gain_db / 20.0));
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			mb_io_ptr->new_bath[i] 
				= data->beams[i].depth;
			mb_io_ptr->new_bath_acrosstrack[i] 
				= data->beams[i].acrosstrack;
			mb_io_ptr->new_bath_alongtrack[i] 
				= data->beams[i].alongtrack;
			}
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			mb_io_ptr->new_amp[i] 
				= 0.25 * data->beams[i].amplitude 
				    - gain_db;
			}
		for (i=0;i<mb_io_ptr->pixels_ss;i++)
			{
			mb_io_ptr->new_ss[i] 
				= gain_factor * (double) data->pixels[i].amplitude;
			mb_io_ptr->new_ss_acrosstrack[i] 
				= data->pixel_size *
					(i - center_pixel);
			mb_io_ptr->new_ss_alongtrack[i] 
				= 0.1 * (double) data->pixels[i].alongtrack;
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
			fprintf(stderr,"dbg4       pixels_ss:  %d\n",
				mb_io_ptr->pixels_ss);
			for (i=0;i<mb_io_ptr->pixels_ss;i++)
			  fprintf(stderr,"dbg4       pixel:%d  ss:%f  acrosstrack:%f  alongtrack:%f\n",
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
			MBF_SB2100B1_MAXLINE);

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
		
		/* copy comment if required */
		if (data->kind == MB_DATA_COMMENT)
		    strncpy(store->comment,data->comment,
			    MBSYS_SB2100_MAXLINE);
		    
		/* else copy data */
		else
		    {
		    
		    /* sonar parameters (SB21BIPR) */
		    if (data->kind == MB_DATA_PARAMETER)
			{
			store->year = data->pr_year;
			store->jday = data->pr_jday;
			store->hour = data->pr_hour;
			store->minute = data->pr_minute;
			store->sec = data->pr_sec;
			store->msec = data->pr_msec;
			}
		    store->roll_bias_port = data->roll_bias_port;
		    store->roll_bias_starboard = data->roll_bias_starboard;
		    store->pitch_bias = data->pitch_bias;
		    store->ship_draft = data->ship_draft;
		    store->offset_x = data->offset_x;
		    store->offset_y = data->offset_y;
		    store->offset_z = data->offset_z;
		    store->num_svp = data->num_svp;
		    for (i=0;i<MBF_SB2100B1_MAXVEL;i++)
			{
			store->svp[i].depth = data->svp[i].depth;
			store->svp[i].velocity = data->svp[i].velocity;
			}
		    
		    /* sonar data header (SB21BIDH) */
		    if (data->kind != MB_DATA_PARAMETER)
			{
			store->year = data->year;
			store->jday = data->jday;
			store->hour = data->hour;
			store->minute = data->minute;
			store->sec = data->sec;
			store->msec = data->msec;
			}
		    store->longitude = data->longitude;
		    store->latitude = data->latitude;
		    store->heading = data->heading;
		    store->speed = data->speed;
		    store->roll = data->roll;
		    store->pitch = data->pitch;
		    store->heave = data->heave;
		    store->ssv = data->ssv;
		    store->frequency = data->frequency;
		    store->depth_gate_mode = data->depth_gate_mode;
		    store->ping_gain = data->ping_gain;
		    store->ping_pulse_width = data->ping_pulse_width;
		    store->transmitter_attenuation = data->transmitter_attenuation;
		    store->ssv_source = data->ssv_source;
		    store->svp_correction = data->svp_correction;
		    store->pixel_algorithm = data->pixel_algorithm;
		    store->pixel_size = data->pixel_size;
		    store->nbeams = data->nbeams;
		    store->npixels = data->npixels;
		    store->spare1 = data->spare1;
		    store->spare2 = data->spare2;
		    store->spare3 = data->spare3;
		    store->spare4 = data->spare4;
		    store->spare5 = data->spare5;
		    store->spare6 = data->spare6;
		    
		    /* bathymetry record (SB21BIBR) */
		    for (i=0;i<MBF_SB2100B1_BEAMS;i++)
			    {
			    store->beams[i].depth = data->beams[i].depth;
			    store->beams[i].acrosstrack = data->beams[i].acrosstrack;
			    store->beams[i].alongtrack = data->beams[i].alongtrack;
			    store->beams[i].range = data->beams[i].range;
			    store->beams[i].angle_across = data->beams[i].angle_across;
			    store->beams[i].angle_forward = data->beams[i].angle_forward;
			    store->beams[i].amplitude = data->beams[i].amplitude;
			    store->beams[i].signal_to_noise = data->beams[i].signal_to_noise;
			    store->beams[i].echo_length = data->beams[i].echo_length;
			    store->beams[i].quality = data->beams[i].quality;
			    store->beams[i].source = data->beams[i].source;
			    }
		    
		    /* sidescan record (SB21BISR) */
		    for (i=0;i<MBF_SB2100B1_PIXELS;i++)
			    {
			    store->pixels[i].amplitude 
				= (float) data->pixels[i].amplitude;
			    store->pixels[i].alongtrack 
				= 0.1 * (float) data->pixels[i].alongtrack;
			    }
			    
		    /* parameters for MBF_SB2100RW format */
		    store->range_scale = ' ';
		    store->spare_dr[0] = ' ';
		    store->spare_dr[1] = ' ';
		    store->num_algorithms = 1;
		    for (i=0;i<4;i++)
			store->algorithm_order[i] = ' ';
		    store->svp_corr_ss = 0;
		    store->ss_data_length = 4 * MBSYS_SB2100_PIXELS;
		    store->pixel_size_scale = 'D';
		    store->spare_ss = ' ';
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
int mbr_wt_sb2100b1(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_sb2100b1";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100b1_struct *data;
	char	*data_ptr;
	struct mbsys_sb2100_struct *store;
	int	time_j[5];
	int	set_pixel_size;
	double	gain_db;
	double	gain_factor;
	int	center_pixel;
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
	data = (struct mbf_sb2100b1_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	store = (struct mbsys_sb2100_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		/* type of data record */
		data->kind = store->kind;
		
		/* copy comment if required */
		if (store->kind == MB_DATA_COMMENT)
		    strncpy(data->comment,store->comment,
			    MBF_SB2100B1_MAXLINE);
		    
		/* else copy data */
		else
		    {
		    
		    /* sonar parameters (SB21BIPR) */
		    if (data->kind == MB_DATA_PARAMETER)
			{
			data->pr_year = store->year;
			data->pr_jday = store->jday;
			data->pr_hour = store->hour;
			data->pr_minute = store->minute;
			data->pr_sec = store->sec;
			data->pr_msec = store->msec;
			}
		    data->roll_bias_port = store->roll_bias_port;
		    data->roll_bias_starboard = store->roll_bias_starboard;
		    data->pitch_bias = store->pitch_bias;
		    data->ship_draft = store->ship_draft;
		    data->offset_x = store->offset_x;
		    data->offset_y = store->offset_y;
		    data->offset_z = store->offset_z;
		    data->num_svp = store->num_svp;
		    for (i=0;i<MBF_SB2100B1_MAXVEL;i++)
			{
			data->svp[i].depth = store->svp[i].depth;
			data->svp[i].velocity = store->svp[i].velocity;
			}
		    
		    /* sonar data header (SB21BIDH) */
		    if (data->kind != MB_DATA_PARAMETER)
			{
			data->year = store->year;
			data->jday = store->jday;
			data->hour = store->hour;
			data->minute = store->minute;
			data->sec = store->sec;
			data->msec = store->msec;
			}
		    data->longitude = store->longitude;
		    data->latitude = store->latitude;
		    data->heading = store->heading;
		    data->speed = store->speed;
		    data->roll = store->roll;
		    data->pitch = store->pitch;
		    data->heave = store->heave;
		    data->ssv = store->ssv;
		    data->frequency = store->frequency;
		    data->depth_gate_mode = store->depth_gate_mode;
		    data->ping_gain = store->ping_gain;
		    data->ping_pulse_width = store->ping_pulse_width;
		    data->transmitter_attenuation = store->transmitter_attenuation;
		    data->ssv_source = store->ssv_source;
		    data->svp_correction = store->svp_correction;
		    data->pixel_algorithm = store->pixel_algorithm;
		    data->pixel_size = store->pixel_size;
		    data->nbeams = store->nbeams;
		    data->npixels = store->npixels;
		    data->spare1 = store->spare1;
		    data->spare2 = store->spare2;
		    data->spare3 = store->spare3;
		    data->spare4 = store->spare4;
		    data->spare5 = store->spare5;
		    data->spare6 = store->spare6;
		    
		    /* bathymetry record (SB21BIBR) */
		    for (i=0;i<MBF_SB2100B1_BEAMS;i++)
			    {
			    data->beams[i].depth = store->beams[i].depth;
			    data->beams[i].acrosstrack = store->beams[i].acrosstrack;
			    data->beams[i].alongtrack = store->beams[i].alongtrack;
			    data->beams[i].range = store->beams[i].range;
			    data->beams[i].angle_across = store->beams[i].angle_across;
			    data->beams[i].angle_forward = store->beams[i].angle_forward;
			    data->beams[i].amplitude = store->beams[i].amplitude;
			    data->beams[i].signal_to_noise = store->beams[i].signal_to_noise;
			    data->beams[i].echo_length = store->beams[i].echo_length;
			    data->beams[i].quality = store->beams[i].quality;
			    data->beams[i].source = store->beams[i].source;
			    }
		    
		    /* sidescan record (SB21BISR) */
		    for (i=0;i<MBF_SB2100B1_PIXELS;i++)
			    {
			    data->pixels[i].amplitude 
				= (unsigned short) store->pixels[i].amplitude;
			    data->pixels[i].alongtrack 
				= (short) (10 * store->pixels[i].alongtrack);
			    }
		    }
		}

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		data->kind = mb_io_ptr->new_kind;

	/* set times from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR 
		&& data->kind == MB_DATA_DATA)
		{
		/* get time */
		mb_get_jtime(verbose,mb_io_ptr->new_time_i,time_j);
		data->year = time_j[0];
		data->jday = time_j[1];
		data->hour = time_j[2]/60;
		data->minute = time_j[2] - 60*data->hour;
		data->sec = time_j[3];
		data->msec = (int) (0.001 * time_j[4]);
		}

	/* check for comment to be copied from mb_io_ptr */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		strncpy(data->comment,mb_io_ptr->new_comment,
			MBF_SB2100B1_MAXLINE);
		}

	/* else check for ping data to be copied from mb_io_ptr */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/* get number of beams and pixels */
		data->nbeams = mb_io_ptr->beams_bath;
		data->npixels = mb_io_ptr->pixels_ss;
		center_pixel = data->npixels / 2;

		/* get navigation */
		data->longitude = mb_io_ptr->new_lon;
		data->latitude = mb_io_ptr->new_lat;

		/* get heading */
		data->heading = mb_io_ptr->new_heading;

		/* get speed */
		data->speed = 5.3899155 * mb_io_ptr->new_speed;

		/* read beam and pixel values into storage arrays */
		gain_db = data->ping_gain 
			- data->transmitter_attenuation
			+ 10.0 * log10( data->ping_pulse_width / 5.0)
			- 30.0;
		gain_factor = pow(10.0, (gain_db / 20.0));
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			 data->beams[i].depth
				= mb_io_ptr->new_bath[i];
			 data->beams[i].acrosstrack
				= mb_io_ptr->new_bath_acrosstrack[i];
			 data->beams[i].alongtrack
				= mb_io_ptr->new_bath_alongtrack[i];
			}
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			 data->beams[i].amplitude
				= 4.0 * (mb_io_ptr->new_amp[i] + gain_db);
			}
		if (data->pixel_size <= 0.0)
			set_pixel_size = MB_YES;
		else
			set_pixel_size = MB_NO;
		for (i=0;i<mb_io_ptr->pixels_ss;i++)
			{
			data->pixels[i].amplitude
				= (unsigned short) (gain_factor 
					* mb_io_ptr->new_ss[i]);
			data->pixels[i].alongtrack
				= (short) (10 * mb_io_ptr->new_ss_alongtrack[i]);
			if (set_pixel_size == MB_YES
				&& mb_io_ptr->new_ss_acrosstrack[i] > 0)
				{
				data->pixel_size = 
					mb_io_ptr->new_ss_acrosstrack[i]/
					(i - center_pixel);
				set_pixel_size = MB_NO;
				}
			}
		}

	/* write next data to file */
	status = mbr_sb2100b1_wr_data(verbose,mbio_ptr,data_ptr,error);

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
int mbr_sb2100b1_rd_data(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_sb2100b1_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100b1_struct *data;
	char	*data_ptr;
	FILE	*mbfp;
	int	done;
	char	*label;
	int	*label_save_flag;
	int	type;
	int	expect;
	short	record_length;
	char	*record_length_ptr;
	char	record_length_fh_str[8];
	int	record_length_fh;
	int	i;

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
	data = (struct mbf_sb2100b1_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	mbfp = mb_io_ptr->mbfp;
	
	/* get saved values */
	label = (char *) mb_io_ptr->save_label;
	label_save_flag = (int *) &mb_io_ptr->save_label_flag;
	record_length_ptr = (char *) &record_length;

	/* initialize everything to zeros */
	mbr_zero_sb2100b1(verbose,data_ptr,error);

	done = MB_NO;
	expect = MBF_SB2100B1_NONE;
	while (done == MB_NO)
		{
		/* if no label saved get next record label */
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		if (*label_save_flag == MB_NO)
			{
			/* get next 10 bytes */
			if ((status = fread(&label[0],
				10, 
				1, mbfp)) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}

			/* if not a format 42 label read individual 
			    bytes until label found or eof */
			while (status == MB_SUCCESS
			    && strncmp(label, "SB21BI", 6) != 0)
			    {
			    for (i=0;i<9;i++)
				label[i] = label[i+1];
			    if ((status = fread(&label[9],
				    1, 1, mbfp)) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			    }
			}
		
		/* else use saved label */
		else
			*label_save_flag = MB_NO;
			
		/* get the label type */
		if (status == MB_SUCCESS)
			{
			/* get type */
			type = MBF_SB2100B1_NONE;
			for (i=1;i<=MBF_SB2100B1_RECORDS;i++)
				if (strncmp(label, mbf_sb2100b1_labels[i], 8) == 0)
				    type = i;
			
			/* get the record length */
			if (type != MBF_SB2100B1_FH)
			    {
#ifndef BYTESWAPPED
			    record_length_ptr[0] = label[8];
			    record_length_ptr[1] = label[9];
#else
			    record_length_ptr[0] = label[9];
			    record_length_ptr[1] = label[8];
#endif
			    }
			else
			    {
			    record_length_fh_str[0] = label[8];
			    record_length_fh_str[1] = label[9];
			    if ((status = fread(&record_length_fh_str[2],
				    4, 1, mbfp)) != 1)
				    {
				    status = MB_FAILURE;
				    *error = MB_ERROR_EOF;
				    }
			    record_length_fh_str[6] = 0;
			    record_length_fh_str[7] = 0;
			    sscanf(record_length_fh_str, "%d", &record_length_fh);
			    }
			}

		/* read the appropriate data records */
		if ((status == MB_FAILURE || type == MBF_SB2100B1_NONE)
			&& expect == MBF_SB2100B1_NONE)
			{
			done = MB_YES;
			}
		else if ((status == MB_FAILURE || type == MBF_SB2100B1_NONE)
			&& expect != MBF_SB2100B1_NONE)
			{
			done = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else if (expect != MBF_SB2100B1_NONE && expect != type)
			{
			done = MB_YES;
			expect = MBF_SB2100B1_NONE;
			*label_save_flag = MB_YES;
			}
		else if (type == MBF_SB2100B1_FH)
			{
			status = mbr_sb2100b1_rd_fh(
				verbose,mbfp,record_length_fh,error);
			if (status == MB_SUCCESS)
				{
				done = MB_NO;
				expect = MBF_SB2100B1_NONE;
				data->kind = MB_DATA_NONE;
				}
			}
		else if (type == MBF_SB2100B1_PR)
			{
			status = mbr_sb2100b1_rd_pr(
				verbose,mbfp,data,record_length,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_PARAMETER;
				}
			}
		else if (type == MBF_SB2100B1_TR)
			{
			status = mbr_sb2100b1_rd_tr(
				verbose,mbfp,data,record_length,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_COMMENT;
				}
			}
		else if (type == MBF_SB2100B1_DH)
			{
			status = mbr_sb2100b1_rd_dh(
				verbose,mbfp,data,record_length,error);
			if (status == MB_SUCCESS)
				{
				done = MB_NO;
				data->kind = MB_DATA_DATA;
				expect = MBF_SB2100B1_BR;
				}
			}
		else if (type == MBF_SB2100B1_BR)
			{
			status = mbr_sb2100b1_rd_br(
				verbose,mbfp,data,record_length,error);
			if (status == MB_SUCCESS 
				&& expect == MBF_SB2100B1_BR)
				{
				done = MB_NO;
				data->kind = MB_DATA_DATA;
				expect = MBF_SB2100B1_SR;
				}
			else if (status == MB_SUCCESS)
				{
				done = MB_YES;
				expect = MBF_SB2100B1_NONE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
				}
			else if (status == MB_FAILURE)
				{
				done = MB_YES;
				expect = MBF_SB2100B1_NONE;
				}
			}
		else if (type == MBF_SB2100B1_SR)
			{
			status = mbr_sb2100b1_rd_sr(
				verbose,mbfp,data,record_length,error);
			if (status == MB_SUCCESS 
				&& expect == MBF_SB2100B1_SR)
				{
				done = MB_YES;
				}
			else if (status == MB_SUCCESS)
				{
				done = MB_YES;
				expect = MBF_SB2100B1_NONE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
				}
			else if (status == MB_FAILURE
				&& *error ==  MB_ERROR_UNINTELLIGIBLE
				&& expect == MBF_SB2100B1_SR)
				{
				/* this preserves the bathymetry
				   that has already been read */
				done = MB_YES;
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
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
int mbr_sb2100b1_rd_fh(verbose,mbfp,record_length,error)
int	verbose;
FILE	*mbfp;
int	record_length;
int	*error;
{
	char	*function_name = "mbr_sb2100b1_rd_fh";
	int	status = MB_SUCCESS;
	int	read_length;
	char	read_buffer[100];
	int	nread;
	int	nlast;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       record_len: %d\n",record_length);
		}

	/* check record size */
	if (record_length > 100000)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	/* if success read rest of record */
	if (status == MB_SUCCESS)
		{
		/* read data into buffer */
		nread = record_length / 100;
		nlast = record_length % 100;
		for (i=0;i<nread;i++)		    
		    if ((status = fread(read_buffer,
			    100, 1, mbfp)) != 1)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_EOF;
			    }
		if (nlast > 0)		    
		    if ((status = fread(read_buffer,
			    nlast, 1, mbfp)) != 1)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_EOF;
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
int mbr_sb2100b1_rd_pr(verbose,mbfp,data,record_length,error)
int	verbose;
FILE	*mbfp;
struct mbf_sb2100b1_struct *data;
short	record_length;
int	*error;
{
	char	*function_name = "mbr_sb2100b1_rd_pr";
	int	status = MB_SUCCESS;
	int	read_length;
	unsigned int	*checksum_read;
	unsigned int	checksum;
	char	*checksum_ptr;
	char	eor_read[6];
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
		fprintf(stderr,"dbg2       record_len: %d\n",record_length);
		}

	/* check record size */
	if (record_length != MBF_SB2100B1_PR_WRITE_LEN + 6)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	/* if success read rest of record */
	if (status == MB_SUCCESS)
		{
		/* read data into structure */
		read_length = MBF_SB2100B1_PR_WRITE_LEN;
		if ((status = fread(&(data->pr_year),
			read_length, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* read checksum and eor */
		if ((status = fread(&eor_read[0],
			6, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		checksum_read = (unsigned int *) &eor_read[0];
#ifdef BYTESWAPPED
		*checksum_read = (unsigned int) mb_swap_long(*checksum_read);
#endif

		/* do checksum */
		if (verbose > 1)
		    {
		    checksum = 0;
		    checksum_ptr = (char *) &(data->pr_year);
		    for (i=0;i<read_length;i++)
			    checksum += (unsigned int) checksum_ptr[i];
    
		    /* check checksum */
		    if (checksum != *checksum_read)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    }
		    }
			
#ifdef BYTESWAPPED
		/* byte swap everything */
		data->pr_year =	    (short int) mb_swap_short(data->pr_year);
		data->pr_jday =	    (short int) mb_swap_short(data->pr_jday);
		data->pr_hour =	    (short int) mb_swap_short(data->pr_hour);
		data->pr_minute =   (short int) mb_swap_short(data->pr_minute);
		data->pr_sec =	    (short int) mb_swap_short(data->pr_sec);
		data->pr_msec =	    (short int) mb_swap_short(data->pr_msec);
		mb_swap_float(&(data->roll_bias_port));
		mb_swap_float(&(data->roll_bias_starboard));
		mb_swap_float(&(data->pitch_bias));
		mb_swap_float(&(data->ship_draft));
		mb_swap_float(&(data->offset_x));
		mb_swap_float(&(data->offset_y));
		mb_swap_float(&(data->offset_z));
		data->num_svp =	    (int) mb_swap_long(data->num_svp);
		for (i=0;i<data->num_svp;i++)
			{
			mb_swap_float(&(data->svp[i].depth));
			mb_swap_float(&(data->svp[i].velocity));
			}
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->pr_year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->pr_jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->pr_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->pr_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->pr_sec);
		fprintf(stderr,"dbg5       msec:             %d\n",data->pr_msec);
		fprintf(stderr,"dbg5       roll_bias_port:   %f\n",data->roll_bias_port);
		fprintf(stderr,"dbg5       roll_bias_strbrd: %f\n",data->roll_bias_starboard);
		fprintf(stderr,"dbg5       pitch_bias:       %f\n",data->pitch_bias);
		fprintf(stderr,"dbg5       ship_draft:       %f\n",data->ship_draft);
		fprintf(stderr,"dbg5       offset_x:         %f\n",data->offset_x);
		fprintf(stderr,"dbg5       offset_y:         %f\n",data->offset_y);
		fprintf(stderr,"dbg5       offset_z:         %f\n",data->offset_z);
		fprintf(stderr,"dbg5       num_svp:          %d\n",data->num_svp);
		fprintf(stderr,"dbg5       Sound Velocity Profile:\n");
		for (i=0;i<data->num_svp;i++)
			fprintf(stderr,"dbg5       %d  depth:%f  velocity:%f\n",
				i,data->svp[i].depth,data->svp[i].velocity);
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
int mbr_sb2100b1_rd_tr(verbose,mbfp,data,record_length,error)
int	verbose;
FILE	*mbfp;
struct mbf_sb2100b1_struct *data;
short	record_length;
int	*error;
{
	char	*function_name = "mbr_sb2100b1_rd_tr";
	int	status = MB_SUCCESS;
	int	read_length;
	unsigned int	*checksum_read;
	unsigned int	checksum;
	char	*checksum_ptr;
	char	eor_read[6];
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
		fprintf(stderr,"dbg2       record_len: %d\n",record_length);
		}

	/* check record size */
	if (record_length > MBF_SB2100B1_MAXLINE + 6)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	/* if success read rest of record */
	if (status == MB_SUCCESS)
		{
		/* read data into structure */
		read_length = record_length - 6;
		if ((status = fread(data->comment,
			read_length, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* read checksum and eor */
		if ((status = fread(&eor_read[0],
			6, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		checksum_read = (unsigned int *) &eor_read[0];
#ifdef BYTESWAPPED
		*checksum_read = (unsigned int) mb_swap_long(*checksum_read);
#endif

		/* do checksum */
		if (verbose > 1)
		    {
		    checksum = 0;
		    checksum_ptr = (char *) data->comment;
		    for (i=0;i<read_length;i++)
			    checksum += (unsigned int) checksum_ptr[i];
    
		    /* check checksum */
		    if (checksum != *checksum_read)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    }
		    }
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
int mbr_sb2100b1_rd_dh(verbose,mbfp,data,record_length,error)
int	verbose;
FILE	*mbfp;
struct mbf_sb2100b1_struct *data;
short	record_length;
int	*error;
{
	char	*function_name = "mbr_sb2100b1_rd_dh";
	int	status = MB_SUCCESS;
	int	read_length;
	unsigned int	*checksum_read;
	unsigned int	checksum;
	char	*checksum_ptr;
	char	eor_read[6];
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
		fprintf(stderr,"dbg2       record_len: %d\n",record_length);
		}

	/* check record size */
	if (record_length != MBF_SB2100B1_DH_WRITE_LEN + 6)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	/* if success read rest of record */
	if (status == MB_SUCCESS)
		{
		/* read data into structure */
		read_length = MBF_SB2100B1_DH_WRITE_LEN;
		if ((status = fread(&(data->year),
			read_length, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* read checksum and eor */
		if ((status = fread(&eor_read[0],
			6, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		checksum_read = (unsigned int *) &eor_read[0];
#ifdef BYTESWAPPED
		*checksum_read = (unsigned int) mb_swap_long(*checksum_read);
#endif

		/* do checksum */
		if (verbose > 1)
		    {
		    checksum = 0;
		    checksum_ptr = (char *) &(data->year);
		    for (i=0;i<read_length;i++)
			    checksum += (unsigned int) checksum_ptr[i];
    
		    /* check checksum */
		    if (checksum != *checksum_read)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    }
		    }
		
#ifdef BYTESWAPPED
		/* byte swap everything */
		data->year =	    (short int) mb_swap_short(data->year);
		data->jday =	    (short int) mb_swap_short(data->jday);
		data->hour =	    (short int) mb_swap_short(data->hour);
		data->minute =   (short int) mb_swap_short(data->minute);
		data->sec =	    (short int) mb_swap_short(data->sec);
		data->msec =	    (short int) mb_swap_short(data->msec);
		mb_swap_double(&(data->longitude));
		mb_swap_double(&(data->latitude));
		mb_swap_float(&(data->heading));
		mb_swap_float(&(data->speed));
		mb_swap_float(&(data->roll));
		mb_swap_float(&(data->pitch));
		mb_swap_float(&(data->heave));
		mb_swap_float(&(data->ssv));
		mb_swap_float(&(data->pixel_size));
		data->nbeams =	    (int) mb_swap_long(data->nbeams);
		data->npixels =	    (int) mb_swap_long(data->npixels);
		data->spare1 =	    (int) mb_swap_short(data->spare1);
		data->spare2 =	    (int) mb_swap_short(data->spare2);
		data->spare3 =	    (int) mb_swap_short(data->spare3);
		data->spare4 =	    (int) mb_swap_short(data->spare4);
		data->spare5 =	    (int) mb_swap_short(data->spare5);
		data->spare6 =	    (int) mb_swap_short(data->spare6);
#endif
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
		fprintf(stderr,"dbg5       sec:              %d\n",data->sec);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->longitude);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->latitude);
		fprintf(stderr,"dbg5       heading:          %f\n",data->heading);
		fprintf(stderr,"dbg5       speed:            %f\n",data->speed);
		fprintf(stderr,"dbg5       roll:             %f\n",data->roll);
		fprintf(stderr,"dbg5       pitch:            %f\n",data->pitch);
		fprintf(stderr,"dbg5       heave:            %f\n",data->heave);
		fprintf(stderr,"dbg5       ssv:              %f\n",data->ssv);
		fprintf(stderr,"dbg5       frequency:        %c\n",data->frequency);
		fprintf(stderr,"dbg5       depth_gate_mode:  %d\n",data->depth_gate_mode);
		fprintf(stderr,"dbg5       ping_gain:        %d\n",data->ping_gain);
		fprintf(stderr,"dbg5       ping_pulse_width: %d\n",data->ping_pulse_width);
		fprintf(stderr,"dbg5       trans_atten:      %d\n",data->transmitter_attenuation);
		fprintf(stderr,"dbg5       ssv_source:       %c\n",data->ssv_source);
		fprintf(stderr,"dbg5       svp_correction:   %c\n",data->svp_correction);
		fprintf(stderr,"dbg5       pixel_algorithm:  %c\n",data->pixel_algorithm);
		fprintf(stderr,"dbg5       pixel_size:       %f\n",data->pixel_size);
		fprintf(stderr,"dbg5       nbeams:           %d\n",data->nbeams);
		fprintf(stderr,"dbg5       npixels:          %d\n",data->npixels);
		fprintf(stderr,"dbg5       spare1:           %d\n",data->spare1);
		fprintf(stderr,"dbg5       spare2:           %d\n",data->spare2);
		fprintf(stderr,"dbg5       spare3:           %d\n",data->spare3);
		fprintf(stderr,"dbg5       spare4:           %d\n",data->spare4);
		fprintf(stderr,"dbg5       spare5:           %d\n",data->spare5);
		fprintf(stderr,"dbg5       spare6:           %d\n",data->spare6);
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
int mbr_sb2100b1_rd_br(verbose,mbfp,data,record_length,error)
int	verbose;
FILE	*mbfp;
struct mbf_sb2100b1_struct *data;
short	record_length;
int	*error;
{
	char	*function_name = "mbr_sb2100b1_rd_br";
	int	status = MB_SUCCESS;
	int	read_length;
	unsigned int	*checksum_read;
	unsigned int	checksum;
	char	*checksum_ptr;
	char	eor_read[6];
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
		fprintf(stderr,"dbg2       record_len: %d\n",record_length);
		}

	/* check record size */
	if (record_length != data->nbeams * MBF_SB2100B1_BR_WRITE_LEN + 6)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	/* if success read rest of record */
	if (status == MB_SUCCESS)
		{
		/* read data into structure */
		read_length = data->nbeams * MBF_SB2100B1_BR_WRITE_LEN;
		if (read_length > 0)
		if ((status = fread(data->beams,
			read_length, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* read checksum and eor */
		if ((status = fread(&eor_read[0],
			6, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		checksum_read = (unsigned int *) &eor_read[0];
#ifdef BYTESWAPPED
		*checksum_read = (unsigned int) mb_swap_long(*checksum_read);
#endif

		/* do checksum */
		if (verbose > 1)
		    {
		    checksum = 0;
		    checksum_ptr = (char *) data->beams;
		    for (i=0;i<read_length;i++)
			    checksum += (unsigned int) checksum_ptr[i];
    
		    /* check checksum */
		    if (checksum != *checksum_read)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    }
		    }
			
#ifdef BYTESWAPPED
		/* byte swap everything */
		for (i=0;i<data->nbeams;i++)
			{
			mb_swap_float(&(data->beams[i].depth));
			mb_swap_float(&(data->beams[i].acrosstrack));
			mb_swap_float(&(data->beams[i].alongtrack));
			mb_swap_float(&(data->beams[i].range));
			mb_swap_float(&(data->beams[i].angle_across));
			mb_swap_float(&(data->beams[i].angle_forward));
			data->beams[i].amplitude 
			    = (short int) mb_swap_short(data->beams[i].amplitude);
			data->beams[i].signal_to_noise 
			    = (short int) mb_swap_short(data->beams[i].signal_to_noise);
			data->beams[i].echo_length 
			    = (int) mb_swap_short(data->beams[i].echo_length);
			}
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       beam depth xtrack ltrack tt angle angfor amp sig2noise echo src quality\n");
		for (i=0;i<data->nbeams;i++)
		  {
		  fprintf(stderr,"dbg5       %3d %8.2f %9.2f %8.2f %6.3f %7.3f %7.3f %3d %3d %3d %c %c\n",
			i,
			data->beams[i].depth,
			data->beams[i].acrosstrack,
			data->beams[i].alongtrack,
			data->beams[i].range,
			data->beams[i].angle_across,
			data->beams[i].angle_forward,
			data->beams[i].amplitude,
			data->beams[i].signal_to_noise,
			data->beams[i].echo_length,
			data->beams[i].source,
			data->beams[i].quality);
		  }
		}

	/* apply quality flags */
	if (status == MB_SUCCESS)
		{
		for (i=0;i<data->nbeams;i++)
			{
			if (data->beams[i].quality != ' '
			    && data->beams[i].depth > 0.0)
			    {
			    data->beams[i].depth = -data->beams[i].depth;
			    data->beams[i].amplitude = -data->beams[i].amplitude;
			    }
			else if (data->beams[i].quality == ' '
			    && data->beams[i].depth < 0.0)
			    {
			    data->beams[i].depth = -data->beams[i].depth;
			    data->beams[i].amplitude = -data->beams[i].amplitude;
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
int mbr_sb2100b1_rd_sr(verbose,mbfp,data,record_length,error)
int	verbose;
FILE	*mbfp;
struct mbf_sb2100b1_struct *data;
short	record_length;
int	*error;
{
	char	*function_name = "mbr_sb2100b1_rd_sr";
	int	status = MB_SUCCESS;
	int	read_length;
	unsigned int	*checksum_read;
	unsigned int	checksum;
	char	*checksum_ptr;
	char	eor_read[6];
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
		fprintf(stderr,"dbg2       record_len: %d\n",record_length);
		}

	/* check record size */
	if (record_length != data->npixels * MBF_SB2100B1_SR_WRITE_LEN + 6)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	/* if success read rest of record */
	if (status == MB_SUCCESS)
		{
		/* read data into structure */
		read_length = data->npixels * MBF_SB2100B1_SR_WRITE_LEN;
		if (read_length > 0)
		if ((status = fread(data->pixels,
			read_length, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* read checksum and eor */
		if ((status = fread(&eor_read[0],
			6, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		checksum_read = (unsigned int *) &eor_read[0];
#ifdef BYTESWAPPED
		*checksum_read = (unsigned int) mb_swap_long(*checksum_read);
#endif

		/* do checksum */
		if (verbose > 1)
		    {
		    checksum = 0;
		    checksum_ptr = (char *) data->pixels;
		    for (i=0;i<read_length;i++)
			    checksum += (unsigned int) checksum_ptr[i];
    
		    /* check checksum */
		    if (checksum != *checksum_read)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    }
		    }
			
#ifdef BYTESWAPPED
		/* byte swap everything */
		for (i=0;i<data->npixels;i++)
			{
			data->pixels[i].amplitude
			    = (unsigned short) mb_swap_short(
				    data->pixels[i].amplitude);
			data->pixels[i].alongtrack
			    = (short) mb_swap_short(
				    data->pixels[i].alongtrack);
			}
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       pixel amplitude alongtrack\n");
		for (i=0;i<data->npixels;i++)
		  {
		  fprintf(stderr,"dbg5       %3d   %5d   %5d\n",
			i,
			data->pixels[i].amplitude,
			data->pixels[i].alongtrack);
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
int mbr_sb2100b1_wr_data(verbose,mbio_ptr,data_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_sb2100b1_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100b1_struct *data;
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
	data = (struct mbf_sb2100b1_struct *) data_ptr;
	mbfp = mb_io_ptr->mbfp;
	
	/* write file header if not written yet */
	if (mb_io_ptr->save_flag == MB_NO)
		{
		status = mbr_sb2100b1_wr_fh(verbose,mbfp,error);
		mb_io_ptr->save_flag = MB_YES;
		}

	/* write the data */
	if (data->kind == MB_DATA_PARAMETER)
		{
		status = mbr_sb2100b1_wr_pr(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_COMMENT)
		{
		status = mbr_sb2100b1_wr_tr(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_DATA)
		{
		status = mbr_sb2100b1_wr_dh(verbose,mbfp,data,error);
		status = mbr_sb2100b1_wr_br(verbose,mbfp,data,error);
		status = mbr_sb2100b1_wr_sr(verbose,mbfp,data,error);
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
int mbr_sb2100b1_wr_fh(verbose,mbfp,error)
int	verbose;
FILE	*mbfp;
int	*error;
{
	char	*function_name = "mbr_sb2100b1_wr_fh";
	int	status = MB_SUCCESS;
	int	record_length;
	char	record_length_str[8];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       file_header_text: \n%s\n", 
			mbf_sb2100b1_file_header_text);
		}

	/* write the record label */
	if (fwrite(mbf_sb2100b1_labels[MBF_SB2100B1_FH],
		MBF_SB2100B1_LABEL_LEN, 1, mbfp) != 1)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* write the record length */
	if (status == MB_SUCCESS)
		{
		record_length = strlen(mbf_sb2100b1_file_header_text);
		sprintf(record_length_str, "%6d", record_length);
		if (fwrite(record_length_str, 6, 1, mbfp) != 1)
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

	/* write out the data */
	if (status == MB_SUCCESS)
		{			
		/* write the data */
		if (fwrite(mbf_sb2100b1_file_header_text, record_length, 
			1, mbfp) != 1)
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
int mbr_sb2100b1_wr_pr(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_sb2100b1_wr_pr";
	int	status = MB_SUCCESS;
	struct mbf_sb2100b1_struct *data;
	short	record_length;
	int	write_length;
	unsigned int	checksum;
	char	*checksum_ptr;
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
	data = (struct mbf_sb2100b1_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->pr_year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->pr_jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->pr_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->pr_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->pr_sec);
		fprintf(stderr,"dbg5       msec:             %d\n",data->pr_msec);
		fprintf(stderr,"dbg5       roll_bias_port:   %f\n",data->roll_bias_port);
		fprintf(stderr,"dbg5       roll_bias_strbrd: %f\n",data->roll_bias_starboard);
		fprintf(stderr,"dbg5       pitch_bias:       %f\n",data->pitch_bias);
		fprintf(stderr,"dbg5       ship_draft:       %f\n",data->ship_draft);
		fprintf(stderr,"dbg5       offset_x:         %f\n",data->offset_x);
		fprintf(stderr,"dbg5       offset_y:         %f\n",data->offset_y);
		fprintf(stderr,"dbg5       offset_z:         %f\n",data->offset_z);
		fprintf(stderr,"dbg5       num_svp:          %d\n",data->num_svp);
		fprintf(stderr,"dbg5       Sound Velocity Profile:\n");
		for (i=0;i<data->num_svp;i++)
			fprintf(stderr,"dbg5       %d  depth:%f  velocity:%f\n",
				i,data->svp[i].depth,data->svp[i].velocity);
		}

	/* write the record label */
	if (fwrite(mbf_sb2100b1_labels[MBF_SB2100B1_PR],
		MBF_SB2100B1_LABEL_LEN, 1, mbfp) != 1)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* write the record length */
	if (status == MB_SUCCESS)
		{
		record_length = MBF_SB2100B1_PR_WRITE_LEN + 6;
#ifdef BYTESWAPPED
		/* byte swap record length */
		record_length =	    (int) mb_swap_short(record_length);
#endif
		if (fwrite(&record_length, 2, 1, mbfp) != 1)
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

	/* write out the data */
	if (status == MB_SUCCESS)
		{			
#ifdef BYTESWAPPED
		/* byte swap everything */
		data->pr_year =	    (short int) mb_swap_short(data->pr_year);
		data->pr_jday =	    (short int) mb_swap_short(data->pr_jday);
		data->pr_hour =	    (short int) mb_swap_short(data->pr_hour);
		data->pr_minute =   (short int) mb_swap_short(data->pr_minute);
		data->pr_sec =	    (short int) mb_swap_short(data->pr_sec);
		data->pr_msec =	    (short int) mb_swap_short(data->pr_msec);
		mb_swap_float(&(data->roll_bias_port));
		mb_swap_float(&(data->roll_bias_starboard));
		mb_swap_float(&(data->pitch_bias));
		mb_swap_float(&(data->ship_draft));
		mb_swap_float(&(data->offset_x));
		mb_swap_float(&(data->offset_y));
		mb_swap_float(&(data->offset_z));
		data->num_svp =	    (int) mb_swap_long(data->num_svp);
		for (i=0;i<data->num_svp;i++)
			{
			mb_swap_float(&(data->svp[i].depth));
			mb_swap_float(&(data->svp[i].velocity));
			}
#endif

		/* do checksum */
		write_length = MBF_SB2100B1_PR_WRITE_LEN;
		checksum = 0;
		checksum_ptr = (char *) &(data->pr_year);
		for (i=0;i<write_length;i++)
			checksum += (unsigned int) checksum_ptr[i];
#ifdef BYTESWAPPED
		checksum = (unsigned int) mb_swap_long(checksum);
#endif
		
		/* write the data */
		if (fwrite(&(data->pr_year), write_length, 
			1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the checksum */
		if (fwrite(&checksum, 4, 1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the eor */
		if (fwrite(mbf_sb2100b1_eor, 2, 1, mbfp) != 1)
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
int mbr_sb2100b1_wr_tr(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_sb2100b1_wr_tr";
	int	status = MB_SUCCESS;
	struct mbf_sb2100b1_struct *data;
	short	record_length;
	int	write_length;
	unsigned int	checksum;
	char	*checksum_ptr;
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
	data = (struct mbf_sb2100b1_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",
			data->comment);
		}

	/* write the record label */
	if (fwrite(mbf_sb2100b1_labels[MBF_SB2100B1_TR],
		MBF_SB2100B1_LABEL_LEN, 1, mbfp) != 1)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* write the record length */
	if (status == MB_SUCCESS)
		{
		record_length = strlen(data->comment) + 1;
		if (record_length >= MBF_SB2100B1_MAXLINE)
			{
			data->comment[MBF_SB2100B1_MAXLINE-1] = '\0';
			record_length = MBF_SB2100B1_MAXLINE;
			}
		record_length += 6;
#ifdef BYTESWAPPED
		/* byte swap record length */
		record_length =	    (int) mb_swap_short(record_length);
#endif
		if (fwrite(&record_length, 2, 1, mbfp) != 1)
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

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* do checksum */
		write_length = strlen(data->comment) + 1;
		checksum = 0;
		checksum_ptr = (char *) data->comment;
		for (i=0;i<write_length;i++)
			checksum += (unsigned int) checksum_ptr[i];
#ifdef BYTESWAPPED
		checksum = (unsigned int) mb_swap_long(checksum);
#endif

		/* write the data */
		if (fwrite(&(data->pr_year), write_length, 
			1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the checksum */
		if (fwrite(&checksum, 4, 1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the eor */
		if (fwrite(mbf_sb2100b1_eor, 2, 1, mbfp) != 1)
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
int mbr_sb2100b1_wr_dh(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_sb2100b1_wr_dh";
	int	status = MB_SUCCESS;
	struct mbf_sb2100b1_struct *data;
	short	record_length;
	int	write_length;
	unsigned int	checksum;
	char	*checksum_ptr;
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
	data = (struct mbf_sb2100b1_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->sec);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->longitude);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->latitude);
		fprintf(stderr,"dbg5       heading:          %f\n",data->heading);
		fprintf(stderr,"dbg5       speed:            %f\n",data->speed);
		fprintf(stderr,"dbg5       roll:             %f\n",data->roll);
		fprintf(stderr,"dbg5       pitch:            %f\n",data->pitch);
		fprintf(stderr,"dbg5       heave:            %f\n",data->heave);
		fprintf(stderr,"dbg5       ssv:              %f\n",data->ssv);
		fprintf(stderr,"dbg5       frequency:        %c\n",data->frequency);
		fprintf(stderr,"dbg5       depth_gate_mode:  %d\n",data->depth_gate_mode);
		fprintf(stderr,"dbg5       ping_gain:        %d\n",data->ping_gain);
		fprintf(stderr,"dbg5       ping_pulse_width: %d\n",data->ping_pulse_width);
		fprintf(stderr,"dbg5       trans_atten:      %d\n",data->transmitter_attenuation);
		fprintf(stderr,"dbg5       ssv_source:       %c\n",data->ssv_source);
		fprintf(stderr,"dbg5       svp_correction:   %c\n",data->svp_correction);
		fprintf(stderr,"dbg5       pixel_algorithm:  %c\n",data->pixel_algorithm);
		fprintf(stderr,"dbg5       pixel_size:       %f\n",data->pixel_size);
		fprintf(stderr,"dbg5       nbeams:           %d\n",data->nbeams);
		fprintf(stderr,"dbg5       npixels:          %d\n",data->npixels);
		fprintf(stderr,"dbg5       spare1:           %d\n",data->spare1);
		fprintf(stderr,"dbg5       spare2:           %d\n",data->spare2);
		fprintf(stderr,"dbg5       spare3:           %d\n",data->spare3);
		fprintf(stderr,"dbg5       spare4:           %d\n",data->spare4);
		fprintf(stderr,"dbg5       spare5:           %d\n",data->spare5);
		fprintf(stderr,"dbg5       spare6:           %d\n",data->spare6);
		}

	/* write the record label */
	if (fwrite(mbf_sb2100b1_labels[MBF_SB2100B1_DH],
		MBF_SB2100B1_LABEL_LEN, 1, mbfp) != 1)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* write the record length */
	if (status == MB_SUCCESS)
		{
		record_length = MBF_SB2100B1_DH_WRITE_LEN + 6;
#ifdef BYTESWAPPED
		/* byte swap record length */
		record_length =	    (int) mb_swap_short(record_length);
#endif
		if (fwrite(&record_length, 2, 1, mbfp) != 1)
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

	/* write out the data */
	if (status == MB_SUCCESS)
		{			
#ifdef BYTESWAPPED
		/* byte swap everything */
		data->year =	    (short int) mb_swap_short(data->year);
		data->jday =	    (short int) mb_swap_short(data->jday);
		data->hour =	    (short int) mb_swap_short(data->hour);
		data->minute =   (short int) mb_swap_short(data->minute);
		data->sec =	    (short int) mb_swap_short(data->sec);
		data->msec =	    (short int) mb_swap_short(data->msec);
		mb_swap_double(&(data->longitude));
		mb_swap_double(&(data->latitude));
		mb_swap_float(&(data->heading));
		mb_swap_float(&(data->speed));
		mb_swap_float(&(data->roll));
		mb_swap_float(&(data->pitch));
		mb_swap_float(&(data->heave));
		mb_swap_float(&(data->ssv));
		mb_swap_float(&(data->pixel_size));
		data->nbeams =	    (int) mb_swap_long(data->nbeams);
		data->npixels =	    (int) mb_swap_long(data->npixels);
		data->spare1 =	    (int) mb_swap_short(data->spare1);
		data->spare2 =	    (int) mb_swap_short(data->spare2);
		data->spare3 =	    (int) mb_swap_short(data->spare3);
		data->spare4 =	    (int) mb_swap_short(data->spare4);
		data->spare5 =	    (int) mb_swap_short(data->spare5);
		data->spare6 =	    (int) mb_swap_short(data->spare6);
#endif

		/* do checksum */
		write_length = MBF_SB2100B1_DH_WRITE_LEN;
		checksum = 0;
		checksum_ptr = (char *) &(data->year);
		for (i=0;i<write_length;i++)
			checksum += (unsigned int) checksum_ptr[i];
#ifdef BYTESWAPPED
		checksum = (unsigned int) mb_swap_long(checksum);
#endif
		
		/* write the data */
		if (fwrite(&(data->year), write_length, 
			1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the checksum */
		if (fwrite(&checksum, 4, 1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the eor */
		if (fwrite(mbf_sb2100b1_eor, 2, 1, mbfp) != 1)
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
int mbr_sb2100b1_wr_br(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_sb2100b1_wr_br";
	int	status = MB_SUCCESS;
	struct mbf_sb2100b1_struct *data;
	short	record_length;
	int	write_length;
	unsigned int	checksum;
	char	*checksum_ptr;
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
	data = (struct mbf_sb2100b1_struct *) data_ptr;

	/* set quality flags */
	for (i=0;i<data->nbeams;i++)
		{
		if (data->beams[i].depth < 0.0
		    && data->beams[i].quality == ' ')
		    {
		    data->beams[i].quality = 'F';
		    data->beams[i].depth = -data->beams[i].depth;
		    }
		else if (data->beams[i].depth < 0.0)
		    {
		    data->beams[i].depth = -data->beams[i].depth;
		    }
		else if (data->beams[i].depth == 0.0)
		    {
		    data->beams[i].quality = '0';
		    }
		else if (data->beams[i].depth > 0.0)
		    {
		    data->beams[i].quality = ' ';
		    }
		if (data->beams[i].amplitude < 0.0)
		    {
		    data->beams[i].amplitude = -data->beams[i].amplitude;
		    }
		}		

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       beam depth xtrack ltrack tt angle angfor amp sig2noise echo src quality\n");
		for (i=0;i<data->nbeams;i++)
		  {
		  fprintf(stderr,"dbg5       %3d %8.2f %9.2f %8.2f %6.3f %7.3f %7.3f %3d %3d %3d %c %c\n",
			i,
			data->beams[i].depth,
			data->beams[i].acrosstrack,
			data->beams[i].alongtrack,
			data->beams[i].range,
			data->beams[i].angle_across,
			data->beams[i].angle_forward,
			data->beams[i].amplitude,
			data->beams[i].signal_to_noise,
			data->beams[i].echo_length,
			data->beams[i].source,
			data->beams[i].quality);
		  }
		}

	/* write the record label */
	if (fwrite(mbf_sb2100b1_labels[MBF_SB2100B1_BR],
		MBF_SB2100B1_LABEL_LEN, 1, mbfp) != 1)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* write the record length */
	if (status == MB_SUCCESS)
		{
		record_length = data->nbeams * MBF_SB2100B1_BR_WRITE_LEN + 6;
#ifdef BYTESWAPPED
		/* byte swap record length */
		record_length =	    (int) mb_swap_short(record_length);
#endif
		if (fwrite(&record_length, 2, 1, mbfp) != 1)
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

	/* write out the data */
	if (status == MB_SUCCESS)
		{		
#ifdef BYTESWAPPED
		/* byte swap everything */
		for (i=0;i<data->nbeams;i++)
			{
			mb_swap_float(&(data->beams[i].depth));
			mb_swap_float(&(data->beams[i].acrosstrack));
			mb_swap_float(&(data->beams[i].alongtrack));
			mb_swap_float(&(data->beams[i].range));
			mb_swap_float(&(data->beams[i].angle_across));
			mb_swap_float(&(data->beams[i].angle_forward));
			data->beams[i].amplitude 
			    = (short int) mb_swap_short(data->beams[i].amplitude);
			data->beams[i].signal_to_noise 
			    = (short int) mb_swap_short(data->beams[i].signal_to_noise);
			data->beams[i].echo_length 
			    = (int) mb_swap_short(data->beams[i].echo_length);
			}
#endif

		/* do checksum */
		write_length = data->nbeams * MBF_SB2100B1_BR_WRITE_LEN;
		checksum = 0;
		checksum_ptr = (char *) data->beams;
		for (i=0;i<write_length;i++)
			checksum += (unsigned int) checksum_ptr[i];
#ifdef BYTESWAPPED
		checksum = (unsigned int) mb_swap_long(checksum);
#endif
		
		/* write the data */
		if (fwrite(data->beams, write_length, 
			1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the checksum */
		if (fwrite(&checksum, 4, 1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the eor */
		if (fwrite(mbf_sb2100b1_eor, 2, 1, mbfp) != 1)
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
int mbr_sb2100b1_wr_sr(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_sb2100b1_wr_sr";
	int	status = MB_SUCCESS;
	struct mbf_sb2100b1_struct *data;
	short	record_length;
	int	write_length;
	unsigned int	checksum;
	char	*checksum_ptr;
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
	data = (struct mbf_sb2100b1_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       pixel amplitude alongtrack\n");
		for (i=0;i<data->npixels;i++)
		  {
		  fprintf(stderr,"dbg5       %3d   %5d   %5d\n",
			i,
			data->pixels[i].amplitude,
			data->pixels[i].alongtrack);
		  }
		}

	/* write the record label */
	if (fwrite(mbf_sb2100b1_labels[MBF_SB2100B1_SR],
		MBF_SB2100B1_LABEL_LEN, 1, mbfp) != 1)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* write the record length */
	if (status == MB_SUCCESS)
		{
		record_length = data->npixels * MBF_SB2100B1_SR_WRITE_LEN + 6;
#ifdef BYTESWAPPED
		/* byte swap record length */
		record_length =	    (int) mb_swap_short(record_length);
#endif
		if (fwrite(&record_length, 2, 1, mbfp) != 1)
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

	/* write out the data */
	if (status == MB_SUCCESS)
		{			
#ifdef BYTESWAPPED
		/* byte swap everything */
		for (i=0;i<data->npixels;i++)
			{
			data->pixels[i].amplitude
			    = (unsigned short) mb_swap_short(
				    data->pixels[i].amplitude);
			data->pixels[i].alongtrack
			    = (short) mb_swap_short(
				    data->pixels[i].alongtrack);
			}
#endif

		/* do checksum */
		write_length = data->npixels * MBF_SB2100B1_SR_WRITE_LEN;
		checksum = 0;
		checksum_ptr = (char *) data->pixels;
		for (i=0;i<write_length;i++)
			checksum += (unsigned int) checksum_ptr[i];
#ifdef BYTESWAPPED
		checksum = (unsigned int) mb_swap_long(checksum);
#endif
		
		/* write the data */
		if (fwrite(data->pixels, write_length, 
			1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the checksum */
		if (fwrite(&checksum, 4, 1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the eor */
		if (fwrite(mbf_sb2100b1_eor, 2, 1, mbfp) != 1)
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
