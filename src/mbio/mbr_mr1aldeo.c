/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mr1aldeo.c	10/24/95
 *	$Id: mbr_mr1aldeo.c,v 1.2 1996-03-12 17:21:55 caress Exp $
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
 * mbr_mr1aldeo.c contains the functions for reading and writing
 * multibeam data in the MR1ALDEO format.  
 * These functions include:
 *   mbr_alm_mr1aldeo	- allocate read/write memory
 *   mbr_dem_mr1aldeo	- deallocate read/write memory
 *   mbr_rt_mr1aldeo	- read and translate data
 *   mbr_wt_mr1aldeo	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	October 24, 1995
 * $Log: not supported by cvs2svn $
 * Revision 1.1  1996/01/26  21:23:30  caress
 * Initial revision
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mbsys_mr1.h"
#include "../../include/mbf_mr1aldeo.h"

/* angle conversion define */
#define RTD (180./M_PI)

/*--------------------------------------------------------------------*/
int mbr_alm_mr1aldeo(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	static char res_id[]="$Id: mbr_mr1aldeo.c,v 1.2 1996-03-12 17:21:55 caress Exp $";
	char	*function_name = "mbr_alm_mr1aldeo";
	int	status = MB_SUCCESS;
	int	i;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mr1aldeo_struct *data;
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
	mb_io_ptr->structure_size = sizeof(struct mbf_mr1aldeo_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_mr1_struct),
				&mb_io_ptr->store_data,error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_mr1aldeo_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;

	/* initialize everything to zeros */
	mbr_zero_mr1aldeo(verbose,data_ptr,error);
	mb_io_ptr->fileheader = MB_NO;
	mb_io_ptr->hdr_comment_size = 0;
	mb_io_ptr->hdr_comment = NULL;

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
int mbr_dem_mr1aldeo(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_mr1aldeo";
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
int mbr_zero_mr1aldeo(verbose,data_ptr,error)
int	verbose;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_zero_mr1aldeo";
	int	status = MB_SUCCESS;
	struct mbf_mr1aldeo_struct *data;
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
	data = (struct mbf_mr1aldeo_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		/* type of data record */
		data->kind = MB_DATA_NONE;

		/* file header info */
		data->mf_magic = 6666;
		data->mf_count = 0;
		if (data->mf_log == NULL)

		/* ping header */
		data->sec = 0;
		data->usec = 0;
		data->png_lon = 0.0;
		data->png_lat = 0.0;
		data->png_course = 0.0;
		data->png_compass = 0.0;
		data->png_prdepth = 0.0;
		data->png_alt = 0.0;
		data->png_pitch = 0.0;
		data->png_roll = 0.0;
		data->png_temp = 0.0;
		data->png_atssincr = 0.0;
		data->png_tt = 0.0;

		/* port settings */
		data->port_trans[0] = 0.0;
		data->port_trans[1] = 0.0;
		data->port_gain = 0.0;
		data->port_pulse = 0.0;
		data->port_btycount = 0;
		data->port_btypad = 0;
		data->port_ssoffset = 0.0;
		data->port_sscount = 0;
		data->port_sspad = 0;

		/* starboard settings */
		data->stbd_trans[0] = 0.0;
		data->stbd_trans[1] = 0.0;
		data->stbd_gain = 0.0;
		data->stbd_pulse = 0.0;
		data->stbd_btycount = 0;
		data->stbd_btypad = 0;
		data->stbd_ssoffset = 0.0;
		data->stbd_sscount = 0;
		data->stbd_sspad = 0;

		/* bathymetry */
		for (i=0;i<MBF_MR1ALDEO_BEAMS_SIDE;i++)
			{
			data->bath_acrosstrack_port[i] = 0.0;
			data->bath_port[i] = 0.0;
			data->tt_port[i] = 0.0;
			data->angle_port[i] = 0.0;
			data->bath_acrosstrack_stbd[i] = 0.0;
			data->bath_stbd[i] = 0.0;
			data->tt_stbd[i] = 0.0;
			data->angle_stbd[i] = 0.0;
			}

		/* sidescan */
		for (i=0;i<MBF_MR1ALDEO_PIXELS_SIDE;i++)
			{
			data->ss_port[i] = 0.0;
			data->ss_stbd[i] = 0.0;
			}

		/* comment */
		strncpy(data->comment,"\0",MBF_MR1ALDEO_MAXLINE);

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
int mbr_rt_mr1aldeo(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_mr1aldeo";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mr1aldeo_struct *data;
	struct mbsys_mr1_struct *store;
	int	beam_center, pixel_center;
	double	xtrack, depth;
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
	data = (struct mbf_mr1aldeo_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_mr1_struct *) store_ptr;

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
	status = mbr_mr1aldeo_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate time values to current 
		ping variables in mbio descriptor structure */
	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_DATA)
		{
		/* get time */
		mb_io_ptr->new_time_d = data->sec + 0.000001*data->usec;
		mb_get_date(verbose,mb_io_ptr->new_time_d,
			mb_io_ptr->new_time_i);

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

		/* get navigation */
		mb_io_ptr->new_lon = data->png_lon;
		mb_io_ptr->new_lat = data->png_lat;
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
		mb_io_ptr->new_heading = data->png_compass;
		/*mb_io_ptr->new_heading = data->png_course;*/

		/* get speed */
		mb_io_ptr->new_speed = 0.0;

		/* read beam and pixel values into storage arrays */
		beam_center = mb_io_ptr->beams_bath/2;
		pixel_center = mb_io_ptr->pixels_ss/2;
		for (i=0;i<data->port_btycount;i++)
			{
			j = beam_center - i - 2;
			mb_io_ptr->new_bath[j] 
				= data->bath_port[i];
			mb_io_ptr->new_bath_acrosstrack[j] 
				= -data->bath_acrosstrack_port[i];
			mb_io_ptr->new_bath_alongtrack[j] = 0.0;
			}
		for (i=0;i<3;i++)
			{
			j = beam_center + i - 1;
			if (j == beam_center)
				{
				if (data->png_alt > 0.0)
				    mb_io_ptr->new_bath[j] 
					= data->png_prdepth + data->png_alt;
				else if (data->png_alt < 0.0)
				    mb_io_ptr->new_bath[j] 
					= -data->png_prdepth + data->png_alt;
				else
				    mb_io_ptr->new_bath[j] = 0.0;
				}
			else
				mb_io_ptr->new_bath[j] = 0.0;
			mb_io_ptr->new_bath_acrosstrack[j] = 0.0;
			mb_io_ptr->new_bath_alongtrack[j] = 0.0;
			}
		for (i=0;i<data->stbd_btycount;i++)
			{
			j = beam_center + 2 + i;
			mb_io_ptr->new_bath[j] 
				= data->bath_stbd[i];
			mb_io_ptr->new_bath_acrosstrack[j] 
				= data->bath_acrosstrack_stbd[i];
			mb_io_ptr->new_bath_alongtrack[j] = 0.0;
			}
		for (i=0;i<data->port_sscount;i++)
			{
			j = pixel_center - i - 2;
			mb_io_ptr->new_ss[j] 
				= data->ss_port[i];
			mb_io_ptr->new_ss_acrosstrack[j] 
				= -data->port_ssoffset - i*data->png_atssincr;
			mb_io_ptr->new_ss_alongtrack[j] = 0.0;
			}
		for (i=0;i<3;i++)
			{
			j = pixel_center + i - 1;
			mb_io_ptr->new_ss[j] = 0.0;
			mb_io_ptr->new_ss_acrosstrack[j] = 0.0;
			mb_io_ptr->new_ss_alongtrack[j] = 0.0;
			}
		for (i=0;i<data->stbd_sscount;i++)
			{
			j = pixel_center + 2 + i;
			mb_io_ptr->new_ss[j] 
				= data->ss_stbd[i];
			mb_io_ptr->new_ss_acrosstrack[j] 
				= data->stbd_ssoffset + i*data->png_atssincr;
			mb_io_ptr->new_ss_alongtrack[j] = 0.0;
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
			  fprintf(stderr,"dbg4       beam:%d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
				i,mb_io_ptr->new_bath[i],
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
			MBF_MR1ALDEO_MAXLINE);

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

		/* file header info */
		store->mf_magic = data->mf_magic;
		store->mf_count = data->mf_count;

		/* ping header */
		store->sec = data->sec;
		store->usec = data->usec;
		store->png_lon = data->png_lon;
		store->png_lat = data->png_lat;
		store->png_course = data->png_course;
		store->png_compass = data->png_compass;
		store->png_prdepth = data->png_prdepth;
		store->png_alt = data->png_alt;
		store->png_pitch = data->png_pitch;
		store->png_roll = data->png_roll;
		store->png_temp = data->png_temp;
		store->png_atssincr = data->png_atssincr;
		store->png_tt = data->png_tt;

		/* port settings */
		store->port_trans[0] = data->port_trans[0];
		store->port_trans[1] = data->port_trans[1];
		store->port_gain = data->port_gain;
		store->port_pulse = data->port_pulse;
		store->port_btycount = data->port_btycount;
		store->port_btypad = data->port_btypad;
		store->port_ssoffset = data->port_ssoffset;
		store->port_sscount = data->port_sscount;
		store->port_sspad = data->port_sspad;

		/* starboard settings */
		store->stbd_trans[0] = data->stbd_trans[0];
		store->stbd_trans[1] = data->stbd_trans[1];
		store->stbd_gain = data->stbd_gain;
		store->stbd_pulse = data->stbd_pulse;
		store->stbd_btycount = data->stbd_btycount;
		store->stbd_btypad = data->stbd_btypad;
		store->stbd_ssoffset = data->stbd_ssoffset;
		store->stbd_sscount = data->stbd_sscount;
		store->stbd_sspad = data->stbd_sspad;

		/* bathymetry */
		for (i=0;i<store->port_btycount;i++)
			{
			store->bath_acrosstrack_port[i] 
				= data->bath_acrosstrack_port[i];
			store->bath_port[i] = data->bath_port[i];
			store->tt_port[i] = data->tt_port[i];
			store->angle_port[i] = data->angle_port[i];
			}
		for (i=0;i<store->stbd_btycount;i++)
			{
			store->bath_acrosstrack_stbd[i] 
				= data->bath_acrosstrack_stbd[i];
			store->bath_stbd[i] = data->bath_stbd[i];
			store->tt_stbd[i] = data->tt_stbd[i];
			store->angle_stbd[i] = data->angle_stbd[i];
			}

		/* sidescan */
		for (i=0;i<store->port_sscount;i++)
			{
			store->ss_port[i] = data->ss_port[i];
			}
		for (i=0;i<store->stbd_sscount;i++)
			{
			store->ss_stbd[i] = data->ss_stbd[i];
			}

		/* comment */
		strncpy(store->comment,data->comment,MBF_MR1ALDEO_MAXLINE);

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
int mbr_wt_mr1aldeo(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_mr1aldeo";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mr1aldeo_struct *data;
	char	*data_ptr;
	struct mbsys_mr1_struct *store;
	int	beam_center, pixel_center;
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
	data = (struct mbf_mr1aldeo_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	store = (struct mbsys_mr1_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		/* type of data record */
		data->kind = store->kind;

		/* file header info */
		data->mf_magic = store->mf_magic;
		data->mf_count = store->mf_count;

		/* ping header */
		data->sec = store->sec;
		data->usec = store->usec;
		data->png_lon = store->png_lon;
		data->png_lat = store->png_lat;
		data->png_course = store->png_course;
		data->png_compass = store->png_compass;
		data->png_prdepth = store->png_prdepth;
		data->png_alt = store->png_alt;
		data->png_pitch = store->png_pitch;
		data->png_roll = store->png_roll;
		data->png_temp = store->png_temp;
		data->png_atssincr = store->png_atssincr;
		data->png_tt = store->png_tt;

		/* port settings */
		data->port_trans[0] = store->port_trans[0];
		data->port_trans[1] = store->port_trans[1];
		data->port_gain = store->port_gain;
		data->port_pulse = store->port_pulse;
		data->port_btycount = store->port_btycount;
		data->port_btypad = store->port_btypad;
		data->port_ssoffset = store->port_ssoffset;
		data->port_sscount = store->port_sscount;
		data->port_sspad = store->port_sspad;

		/* starboard settings */
		data->stbd_trans[0] = store->stbd_trans[0];
		data->stbd_trans[1] = store->stbd_trans[1];
		data->stbd_gain = store->stbd_gain;
		data->stbd_pulse = store->stbd_pulse;
		data->stbd_btycount = store->stbd_btycount;
		data->stbd_btypad = store->stbd_btypad;
		data->stbd_ssoffset = store->stbd_ssoffset;
		data->stbd_sscount = store->stbd_sscount;
		data->stbd_sspad = store->stbd_sspad;

		/* bathymetry */
		for (i=0;i<data->port_btycount;i++)
			{
			data->bath_acrosstrack_port[i] 
				= store->bath_acrosstrack_port[i];
			data->bath_port[i] = store->bath_port[i];
			data->tt_port[i] = store->tt_port[i];
			data->angle_port[i] = store->angle_port[i];
			}
		for (i=0;i<data->stbd_btycount;i++)
			{
			data->bath_acrosstrack_stbd[i] 
				= store->bath_acrosstrack_stbd[i];
			data->bath_stbd[i] = store->bath_stbd[i];
			data->tt_stbd[i] = store->tt_stbd[i];
			data->angle_stbd[i] = store->angle_stbd[i];
			}

		/* sidescan */
		for (i=0;i<data->port_sscount;i++)
			{
			data->ss_port[i] = store->ss_port[i];
			}
		for (i=0;i<data->stbd_sscount;i++)
			{
			data->ss_stbd[i] = store->ss_stbd[i];
			}

		/* comment */
		strncpy(data->comment,store->comment,MBF_MR1ALDEO_MAXLINE);

		}

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		data->kind = mb_io_ptr->new_kind;

	/* set times from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		{
		/* get time */
		data->sec = (int) mb_io_ptr->new_time_d;
		data->usec = (int) 1000000.0*
			(mb_io_ptr->new_time_d - data->sec);
		}

	/* check for comment to be copied from mb_io_ptr */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		strncpy(data->comment,mb_io_ptr->new_comment,
			MBF_MR1ALDEO_MAXLINE);
		}

	/* else check for ping data to be copied from mb_io_ptr */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/* get navigation */
		data->png_lon = mb_io_ptr->new_lon;
		data->png_lat = mb_io_ptr->new_lat;

		/* get heading */
		data->png_compass = mb_io_ptr->new_heading;
		/*data->png_course = mb_io_ptr->new_heading;*/

		/* get port bathymetry */
		beam_center = mb_io_ptr->beams_bath/2;
		for (i=0;i<data->port_btycount;i++)
			{
			j = beam_center - 2 - i;
			data->bath_port[i] 
				= mb_io_ptr->new_bath[j];
			data->bath_acrosstrack_port[i] 
				= -mb_io_ptr->new_bath_acrosstrack[j];
			}

		/* get center beam bathymetry */
		if (mb_io_ptr->new_bath[beam_center] > 0.0)
			{
			data->png_alt = mb_io_ptr->new_bath[beam_center] 
				- data->png_prdepth;
			}
		else if (mb_io_ptr->new_bath[beam_center] < 0.0)
			{
			data->png_alt = mb_io_ptr->new_bath[beam_center] 
				+ data->png_prdepth;
			}
		else
			{
			data->png_alt = 0.0;
			}

		/* get starboard bathymetry */
		for (i=0;i<data->stbd_btycount;i++)
			{
			j = beam_center + 2 + i;
			data->bath_stbd[i] 
				= mb_io_ptr->new_bath[j];
			data->bath_acrosstrack_stbd[i] 
				= mb_io_ptr->new_bath_acrosstrack[j];
			}

		/* get port sidescan */
		pixel_center = mb_io_ptr->pixels_ss/2;
		for (i=0;i<data->port_sscount;i++)
			{
			j = pixel_center - 2 - i;
			data->ss_port[i] 
				= mb_io_ptr->new_ss[j];
			}

		/* get starboard sidescan */
		for (i=0;i<data->stbd_sscount;i++)
			{
			j = pixel_center + 2 + i;
			data->ss_stbd[i] 
				= mb_io_ptr->new_ss[j];
			}
		}

	/* write next data to file */
	status = mbr_mr1aldeo_wr_data(verbose,mbio_ptr,data_ptr,error);

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
int mbr_mr1aldeo_rd_data(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_mr1aldeo_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mr1aldeo_struct *data;
	char	*data_ptr;
	char	*xdrs;
	int	read_size;

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
	data = (struct mbf_mr1aldeo_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	xdrs = mb_io_ptr->xdrs;

	/* initialize everything to zeros */
	mbr_zero_mr1aldeo(verbose,data_ptr,error);

	/* if first time through read file header */
	if (mb_io_ptr->fileheader == MB_NO)
		{
		status = mbr_mr1aldeo_rd_hdr(verbose,xdrs,data,
			&mb_io_ptr->hdr_comment,error);
		if (status == MB_SUCCESS)
			{
			mb_io_ptr->fileheader = MB_YES;
			if (mb_io_ptr->hdr_comment == NULL)
				mb_io_ptr->hdr_comment_size = 0;
			else
				mb_io_ptr->hdr_comment_size 
					= strlen(mb_io_ptr->hdr_comment);
			mb_io_ptr->hdr_comment_loc = 0;
			if (mb_io_ptr->hdr_comment_size > 80)
				read_size = 80;
			else
				read_size = mb_io_ptr->hdr_comment_size;
			strncpy(data->comment,mb_io_ptr->hdr_comment,read_size);
			mb_io_ptr->hdr_comment_loc = read_size;
			data->kind = MB_DATA_COMMENT;
			}
		}

	/* if comments are still held in mb_io_ptr->hdr_comment then
		extract comment and return */
	else if (mb_io_ptr->hdr_comment_size > mb_io_ptr->hdr_comment_loc)
		{
		if (mb_io_ptr->hdr_comment_size - mb_io_ptr->hdr_comment_loc 
			> 80)
			read_size = 80;
		else
			read_size = mb_io_ptr->hdr_comment_size 
				- mb_io_ptr->hdr_comment_loc;
		strncpy(data->comment,
			&mb_io_ptr->hdr_comment[mb_io_ptr->hdr_comment_loc],
			read_size);
		mb_io_ptr->hdr_comment_loc += read_size;
		data->kind = MB_DATA_COMMENT;
		}

	/* else read data */
	else
		{
		status = mbr_mr1aldeo_rd_ping(verbose,xdrs,data,error);
		if (status == MB_SUCCESS)
			{
			data->kind = MB_DATA_DATA;
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
int mbr_mr1aldeo_rd_hdr(verbose,xdrs,data,hdr_comment,error)
int	verbose;
char	*xdrs;
struct mbf_mr1aldeo_struct *data;
char	**hdr_comment;
int	*error;
{
	char	*function_name = "mbr_mr1aldeo_rd_hdr";
	int	status = MB_SUCCESS;
	int	len, ulen;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       xdrs:       %d\n",xdrs);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       hdr_comment:%d\n",*hdr_comment);
		}

	/* set status and error */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* read magic number */
	status = xdr_int(xdrs, &data->mf_magic);
		
	/* read ping count */
	if (status == MB_SUCCESS)
		status = xdr_int(xdrs, &data->mf_count);

	/* read header comment */
	if (status == MB_SUCCESS)
		status = xdr_int(xdrs, &len);
	if (status == MB_SUCCESS)
		{
		if (len > 0)
			{
			status = mb_malloc(verbose,len+1,hdr_comment,error);
			status = xdr_bytes(xdrs,hdr_comment,
					&ulen,(unsigned long)(len + 1));
			}
		else if (len < 0)
			status = MB_FAILURE;
		}

	if (status == MB_FAILURE && *error == MB_ERROR_NO_ERROR)
		*error = MB_ERROR_EOF;
	else if (data->mf_magic != 6666)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       mf_magic:         %d\n",data->mf_magic);
		fprintf(stderr,"dbg5       mf_count:         %d\n",data->mf_count);
		fprintf(stderr,"dbg5       hdr_comment:\n%s\n",*hdr_comment);
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
int mbr_mr1aldeo_rd_ping(verbose,xdrs,data,error)
int	verbose;
char	*xdrs;
struct mbf_mr1aldeo_struct *data;
int	*error;
{
	char	*function_name = "mbr_mr1aldeo_rd_ping";
	int	status = MB_SUCCESS;
	int	i;
	int	dummy_count;
	float	dummy;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       xdrs:       %d\n",xdrs);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		}

	/* read ping header */
	status = xdr_long(xdrs, &data->sec);
	status = xdr_long(xdrs, &data->usec);
	status = xdr_double(xdrs, &data->png_lon);
	status = xdr_double(xdrs, &data->png_lat);
	status = xdr_float(xdrs, &data->png_course);
	status = xdr_float(xdrs, &data->png_compass);
	status = xdr_float(xdrs, &data->png_prdepth);
	status = xdr_float(xdrs, &data->png_alt);
	status = xdr_float(xdrs, &data->png_pitch);
	status = xdr_float(xdrs, &data->png_roll);
	status = xdr_float(xdrs, &data->png_temp);
	status = xdr_float(xdrs, &data->png_atssincr);
	status = xdr_float(xdrs, &data->png_tt);

	/* read port side header */
	status = xdr_float(xdrs, &data->port_trans[0]);
	status = xdr_float(xdrs, &data->port_trans[1]);
	status = xdr_float(xdrs, &data->port_gain);
	status = xdr_float(xdrs, &data->port_pulse);
	status = xdr_int(xdrs, &data->port_btycount);
	status = xdr_float(xdrs, &data->port_ssoffset);
	status = xdr_int(xdrs, &data->port_sscount);

	/* read starboard side header */
	status = xdr_float(xdrs, &data->stbd_trans[0]);
	status = xdr_float(xdrs, &data->stbd_trans[1]);
	status = xdr_float(xdrs, &data->stbd_gain);
	status = xdr_float(xdrs, &data->stbd_pulse);
	status = xdr_int(xdrs, &data->stbd_btycount);
	status = xdr_float(xdrs, &data->stbd_ssoffset);
	status = xdr_int(xdrs, &data->stbd_sscount);
	
	/* read bathymetry and sidescan data 
		- handle more data than allowed by MBIO by
		  throwing away the excess */

	/* do port bathymetry */
	if (data->port_btycount > MBF_MR1ALDEO_BEAMS_SIDE)
		{
		if (verbose > 0)
			{
			fprintf(stderr, "Port bathymetry count exceeds MBIO maximum: %d %d\n", 
				data->port_btycount, MBF_MR1ALDEO_BEAMS_SIDE);
			}
		dummy_count = data->port_btycount 
			- MBF_MR1ALDEO_BEAMS_SIDE;
		data->port_btycount = MBF_MR1ALDEO_BEAMS_SIDE;
		}
	else
		dummy_count = 0;
	for (i=0;i<data->port_btycount;i++)
		{
		status = xdr_float(xdrs,&data->bath_acrosstrack_port[i]);
		status = xdr_float(xdrs,&data->bath_port[i]);
		status = xdr_float(xdrs,&data->tt_port[i]);
		status = xdr_float(xdrs,&data->angle_port[i]);
		}
	for (i=0;i<dummy_count;i++)
		{
		status = xdr_float(xdrs,&dummy);
		status = xdr_float(xdrs,&dummy);
		status = xdr_float(xdrs,&dummy);
		status = xdr_float(xdrs,&dummy);
		}

	/* do port sidescan */
	if (data->port_sscount > MBF_MR1ALDEO_PIXELS_SIDE)
		{
		if (verbose > 0)
			{
			fprintf(stderr, "Port sidescan count exceeds MBIO maximum: %d %d\n", 
				data->port_sscount, MBF_MR1ALDEO_PIXELS_SIDE);
			}
		dummy_count = data->port_sscount 
			- MBF_MR1ALDEO_PIXELS_SIDE;
		data->port_sscount = MBF_MR1ALDEO_PIXELS_SIDE;
		}
	else
		dummy_count = 0;
	for (i=0;i<data->port_sscount;i++)
		{
		status = xdr_float(xdrs,&data->ss_port[i]);
		}
	for (i=0;i<dummy_count;i++)
		{
		status = xdr_float(xdrs,&dummy);
		}

	/* do starboard bathymetry */
	if (data->stbd_btycount > MBF_MR1ALDEO_BEAMS_SIDE)
		{
		/* output debug messages */
		if (verbose > 0)
			{
			fprintf(stderr, "Starboard bathymetry count exceeds MBIO maximum: %d %d\n", 
				data->stbd_btycount, MBF_MR1ALDEO_BEAMS_SIDE);
			}
		dummy_count = data->stbd_btycount 
			- MBF_MR1ALDEO_BEAMS_SIDE;
		data->stbd_btycount = MBF_MR1ALDEO_BEAMS_SIDE;
		}
	else
		dummy_count = 0;
	for (i=0;i<data->stbd_btycount;i++)
		{
		status = xdr_float(xdrs,&data->bath_acrosstrack_stbd[i]);
		status = xdr_float(xdrs,&data->bath_stbd[i]);
		status = xdr_float(xdrs,&data->tt_stbd[i]);
		status = xdr_float(xdrs,&data->angle_stbd[i]);
		}
	for (i=0;i<dummy_count;i++)
		{
		status = xdr_float(xdrs,&dummy);
		status = xdr_float(xdrs,&dummy);
		status = xdr_float(xdrs,&dummy);
		status = xdr_float(xdrs,&dummy);
		}

	/* do starboard sidescan */
	if (data->stbd_sscount > MBF_MR1ALDEO_PIXELS_SIDE)
		{
		/* output debug messages */
		if (verbose > 0)
			{
			fprintf(stderr, "Starboard sidescan count exceeds MBIO maximum: %d %d\n", 
				data->stbd_sscount, MBF_MR1ALDEO_PIXELS_SIDE);
			}
		dummy_count = data->stbd_sscount 
			- MBF_MR1ALDEO_PIXELS_SIDE;
		data->stbd_sscount = MBF_MR1ALDEO_PIXELS_SIDE;
		}
	else
		dummy_count = 0;
	for (i=0;i<data->stbd_sscount;i++)
		{
		status = xdr_float(xdrs,&data->ss_stbd[i]);
		}
	for (i=0;i<dummy_count;i++)
		{
		status = xdr_float(xdrs,&dummy);
		}

	if (status == MB_FAILURE)
		*error = MB_ERROR_EOF;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       sec:              %d\n",data->sec);
		fprintf(stderr,"dbg5       usec:             %d\n",data->usec);
		fprintf(stderr,"dbg5       longitude:        %f\n",
			data->png_lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",
			data->png_lat);
		fprintf(stderr,"dbg5       course:           %f\n",
			data->png_course);
		fprintf(stderr,"dbg5       heading:          %f\n",
			data->png_compass);
		fprintf(stderr,"dbg5       pressure depth:   %f\n",
			data->png_prdepth);
		fprintf(stderr,"dbg5       altitude:         %f\n",
			data->png_alt);
		fprintf(stderr,"dbg5       pitch:            %f\n",
			data->png_pitch);
		fprintf(stderr,"dbg5       roll:             %f\n",
			data->png_roll);
		fprintf(stderr,"dbg5       temperature:      %f\n",
			data->png_temp);
		fprintf(stderr,"dbg5       pixel spacing:    %f\n",
			data->png_atssincr);
		fprintf(stderr,"dbg5       nadir travel time:%f\n",
			data->png_tt);
		fprintf(stderr,"dbg5       port transmit 0:  %f\n",
			data->port_trans[0]);
		fprintf(stderr,"dbg5       port transmit 1:  %f\n",
			data->port_trans[1]);
		fprintf(stderr,"dbg5       port gain:        %f\n",
			data->port_gain);
		fprintf(stderr,"dbg5       port pulse:       %f\n",
			data->port_pulse);
		fprintf(stderr,"dbg5       port bath count:  %d\n",
			data->port_btycount);
		fprintf(stderr,"dbg5       port ss offset:   %f\n",
			data->port_ssoffset);
		fprintf(stderr,"dbg5       port ss count:    %d\n",
			data->port_sscount);
		fprintf(stderr,"dbg5       stbd transmit 0:  %f\n",
			data->stbd_trans[0]);
		fprintf(stderr,"dbg5       stbd transmit 1:  %f\n",
			data->stbd_trans[1]);
		fprintf(stderr,"dbg5       stbd gain:        %f\n",
			data->stbd_gain);
		fprintf(stderr,"dbg5       stbd pulse:       %f\n",
			data->stbd_pulse);
		fprintf(stderr,"dbg5       stbd bath count:  %d\n",
			data->stbd_btycount);
		fprintf(stderr,"dbg5       stbd ss offset:   %f\n",
			data->stbd_ssoffset);
		fprintf(stderr,"dbg5       stbd ss count:    %d\n",
			data->stbd_sscount);
		fprintf(stderr,"\n");
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"dbg5       port_beam  depth   xtrack    tt   angle\n");
		for (i=0;i<data->port_btycount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g %12.4g %12.4g %12.4g\n",
			i,data->bath_port[i],data->bath_acrosstrack_port[i], 
			data->tt_port[i],data->angle_port[i]);
		  }
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       stbd_beam  depth   xtrack    tt   angle\n");
		for (i=0;i<data->stbd_btycount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g %12.4g %12.4g %12.4g\n",
			i,data->bath_stbd[i],data->bath_acrosstrack_stbd[i],
			data->tt_stbd[i],data->angle_stbd[i]);
		  }
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       port_pixel  sidescan\n");
		for (i=0;i<data->port_sscount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g\n",
			i,data->ss_port[i]);
		  }
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       stbd_pixel  sidescan\n");
		for (i=0;i<data->stbd_sscount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g\n",
			i,data->ss_stbd[i]);
		  }
		fprintf(stderr,"\n");
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
int mbr_mr1aldeo_wr_data(verbose,mbio_ptr,data_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_mr1aldeo_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mr1aldeo_struct *data;
	char	*xdrs;
	char	*tmp;
	int	lenc, lenhc, len;

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
	data = (struct mbf_mr1aldeo_struct *) data_ptr;
	xdrs = mb_io_ptr->xdrs;

	/* if comment and file header not written */
	if (mb_io_ptr->fileheader == MB_NO && data->kind == MB_DATA_COMMENT)
		{
		/* add comment to string mb_io_ptr->hdr_comment
			to be be written in file header */
		lenc = strlen(data->comment);
		lenhc = 0;
		if (mb_io_ptr->hdr_comment != NULL)
			lenhc = strlen(mb_io_ptr->hdr_comment);
		len = lenc + lenhc + 1;
		status = mb_malloc(verbose,len,&tmp,error);
		strcpy(tmp,"\0");
		if (lenhc > 0) strcpy(tmp,mb_io_ptr->hdr_comment);
		if (lenc > 0) strcat(tmp,data->comment);
		if (mb_io_ptr->hdr_comment != NULL)
			mb_free(verbose,&mb_io_ptr->hdr_comment,error);
		mb_io_ptr->hdr_comment = tmp;
		}

	/* if data and file header not written */
	else if (mb_io_ptr->fileheader == MB_NO 
		&& data->kind != MB_DATA_COMMENT)
		{
		/* write file header */
		status = mbr_mr1aldeo_wr_hdr(verbose,xdrs,data,
				&mb_io_ptr->hdr_comment,error);
		mb_io_ptr->fileheader = MB_YES;

		/* write data */
		status = mbr_mr1aldeo_wr_ping(verbose,xdrs,data,error);
		}

	/* if data and file header written */
	else if (mb_io_ptr->fileheader == MB_YES 
		&& data->kind == MB_DATA_DATA)
		{
		/* write data */
		status = mbr_mr1aldeo_wr_ping(verbose,xdrs,data,error);
		}

	/* if not data and file header written */
	else if (mb_io_ptr->fileheader == MB_YES 
		&& data->kind != MB_DATA_DATA)
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
int mbr_mr1aldeo_wr_hdr(verbose,xdrs,data_ptr,hdr_comment,error)
int	verbose;
char	*xdrs;
char	*data_ptr;
char	**hdr_comment;
int	*error;
{
	char	*function_name = "mbr_mr1aldeo_wr_hdr";
	int	status = MB_SUCCESS;
	struct mbf_mr1aldeo_struct *data;
	int	len, ulen;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       xdrs:       %d\n",xdrs);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		fprintf(stderr,"dbg2       hdr_comment:%d\n",*hdr_comment);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_mr1aldeo_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       mf_magic:         %d\n",data->mf_magic);
		fprintf(stderr,"dbg5       mf_count:         %d\n",data->mf_count);
		fprintf(stderr,"dbg5       hdr_comment:\n%s\n",*hdr_comment);
		}

	/* set status and error */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* write magic number */
	status = xdr_int(xdrs, &data->mf_magic);
		
	/* write ping count */
	if (status == MB_SUCCESS)
		{
		status = xdr_int(xdrs, &data->mf_count);
		}

	/* write header comment */
	if (status == MB_SUCCESS)
		{
		if (*hdr_comment == NULL)
			len = 0;
		else
			len = strlen(*hdr_comment);
		status = xdr_int(xdrs, &len);
		}
	if (status == MB_SUCCESS && len > 0)
		{
		ulen = len;
		status = xdr_bytes(xdrs,hdr_comment,
				&ulen,(unsigned long)len);
		}

	/* check for an error */
	if (status != MB_SUCCESS)
		*error = MB_ERROR_WRITE_FAIL;

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
int mbr_mr1aldeo_wr_ping(verbose,xdrs,data_ptr,error)
int	verbose;
char	*xdrs;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_mr1aldeo_wr_ping";
	int	status = MB_SUCCESS;
	struct mbf_mr1aldeo_struct *data;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       xdrs:       %d\n",xdrs);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_mr1aldeo_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       sec:              %d\n",data->sec);
		fprintf(stderr,"dbg5       usec:             %d\n",data->usec);
		fprintf(stderr,"dbg5       longitude:        %f\n",
			data->png_lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",
			data->png_lat);
		fprintf(stderr,"dbg5       course:           %f\n",
			data->png_course);
		fprintf(stderr,"dbg5       heading:          %f\n",
			data->png_compass);
		fprintf(stderr,"dbg5       pressure depth:   %f\n",
			data->png_prdepth);
		fprintf(stderr,"dbg5       altitude:         %f\n",
			data->png_alt);
		fprintf(stderr,"dbg5       pitch:            %f\n",
			data->png_pitch);
		fprintf(stderr,"dbg5       roll:             %f\n",
			data->png_roll);
		fprintf(stderr,"dbg5       temperature:      %f\n",
			data->png_temp);
		fprintf(stderr,"dbg5       pixel spacing:    %f\n",
			data->png_atssincr);
		fprintf(stderr,"dbg5       nadir travel time:%f\n",
			data->png_tt);
		fprintf(stderr,"dbg5       port transmit 0:  %f\n",
			data->port_trans[0]);
		fprintf(stderr,"dbg5       port transmit 1:  %f\n",
			data->port_trans[1]);
		fprintf(stderr,"dbg5       port gain:        %f\n",
			data->port_gain);
		fprintf(stderr,"dbg5       port pulse:       %f\n",
			data->port_pulse);
		fprintf(stderr,"dbg5       port bath count:  %d\n",
			data->port_btycount);
		fprintf(stderr,"dbg5       port ss offset:   %f\n",
			data->port_ssoffset);
		fprintf(stderr,"dbg5       port ss count:    %d\n",
			data->port_sscount);
		fprintf(stderr,"dbg5       stbd transmit 0:  %f\n",
			data->stbd_trans[0]);
		fprintf(stderr,"dbg5       stbd transmit 1:  %f\n",
			data->stbd_trans[1]);
		fprintf(stderr,"dbg5       stbd gain:        %f\n",
			data->stbd_gain);
		fprintf(stderr,"dbg5       stbd pulse:       %f\n",
			data->stbd_pulse);
		fprintf(stderr,"dbg5       stbd bath count:  %d\n",
			data->stbd_btycount);
		fprintf(stderr,"dbg5       stbd ss offset:   %f\n",
			data->stbd_ssoffset);
		fprintf(stderr,"dbg5       stbd ss count:    %d\n",
			data->stbd_sscount);
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       port_beam  depth   xtrack    tt   angle\n");
		for (i=0;i<data->port_btycount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g %12.4g %12.4g %12.4g\n",
			i,data->bath_port[i],data->bath_acrosstrack_port[i], 
			data->tt_port[i],data->angle_port[i]);
		  }
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       stbd_beam  depth   xtrack    tt   angle\n");
		for (i=0;i<data->stbd_btycount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g %12.4g %12.4g %12.4g\n",
			i,data->bath_stbd[i],data->bath_acrosstrack_stbd[i],
			data->tt_stbd[i],data->angle_stbd[i]);
		  }
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       port_pixel  sidescan\n");
		for (i=0;i<data->port_sscount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g\n",
			i,data->ss_port[i]);
		  }
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       stbd_pixel  sidescan\n");
		for (i=0;i<data->stbd_sscount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g\n",
			i,data->ss_stbd[i]);
		  }
		fprintf(stderr,"\n");
		}

	/* write ping header */
	status = xdr_long(xdrs, &data->sec);
	status = xdr_long(xdrs, &data->usec);
	status = xdr_double(xdrs, &data->png_lon);
	status = xdr_double(xdrs, &data->png_lat);
	status = xdr_float(xdrs, &data->png_course);
	status = xdr_float(xdrs, &data->png_compass);
	status = xdr_float(xdrs, &data->png_prdepth);
	status = xdr_float(xdrs, &data->png_alt);
	status = xdr_float(xdrs, &data->png_pitch);
	status = xdr_float(xdrs, &data->png_roll);
	status = xdr_float(xdrs, &data->png_temp);
	status = xdr_float(xdrs, &data->png_atssincr);
	status = xdr_float(xdrs, &data->png_tt);

	/* write port side header */
	status = xdr_float(xdrs, &data->port_trans[0]);
	status = xdr_float(xdrs, &data->port_trans[1]);
	status = xdr_float(xdrs, &data->port_gain);
	status = xdr_float(xdrs, &data->port_pulse);
	status = xdr_int(xdrs, &data->port_btycount);
	status = xdr_float(xdrs, &data->port_ssoffset);
	status = xdr_int(xdrs, &data->port_sscount);

	/* write starboard side header */
	status = xdr_float(xdrs, &data->stbd_trans[0]);
	status = xdr_float(xdrs, &data->stbd_trans[1]);
	status = xdr_float(xdrs, &data->stbd_gain);
	status = xdr_float(xdrs, &data->stbd_pulse);
	status = xdr_int(xdrs, &data->stbd_btycount);
	status = xdr_float(xdrs, &data->stbd_ssoffset);
	status = xdr_int(xdrs, &data->stbd_sscount);

	/* write bathymetry and sidescan data */
	for (i=0;i<data->port_btycount;i++)
		{
		status = xdr_float(xdrs,&data->bath_acrosstrack_port[i]);
		status = xdr_float(xdrs,&data->bath_port[i]);
		status = xdr_float(xdrs,&data->tt_port[i]);
		status = xdr_float(xdrs,&data->angle_port[i]);
		}
	for (i=0;i<data->port_sscount;i++)
		{
		status = xdr_float(xdrs,&data->ss_port[i]);
		}
	for (i=0;i<data->stbd_btycount;i++)
		{
		status = xdr_float(xdrs,&data->bath_acrosstrack_stbd[i]);
		status = xdr_float(xdrs,&data->bath_stbd[i]);
		status = xdr_float(xdrs,&data->tt_stbd[i]);
		status = xdr_float(xdrs,&data->angle_stbd[i]);
		}
	for (i=0;i<data->stbd_sscount;i++)
		{
		status = xdr_float(xdrs,&data->ss_stbd[i]);
		}
	if (status == MB_FAILURE)
		*error = MB_ERROR_WRITE_FAIL;

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
