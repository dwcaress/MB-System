/*--------------------------------------------------------------------
 *    The MB-system:	mbr_em12draw.c	7/8/96
 *	$Id: mbr_em12draw.c,v 4.7 1999-02-04 23:52:54 caress Exp $
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
 * mbr_em12draw.c contains the functions for reading and writing
 * multibeam data in the EM12DRAW format.  
 * These functions include:
 *   mbr_alm_em12draw	- allocate read/write memory
 *   mbr_dem_em12draw	- deallocate read/write memory
 *   mbr_rt_em12draw	- read and translate data
 *   mbr_wt_em12draw	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	August 8, 1994
 * $Log: not supported by cvs2svn $
 * Revision 4.6  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.5  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.4  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.3  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.3  1997/04/17  15:07:36  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.2  1996/09/19  20:22:47  caress
 * Enabled the writing of new position records with mb_put_all
 * calls.
 *
 * Revision 4.1  1996/08/26  18:33:50  caress
 * Changed "signed char" to "char" for SunOs 4.1 compiler compatibility.
 *
 * Revision 4.0  1996/07/31  20:34:16  caress
 * Initial revision.
 *
 * Revision 4.0  1996/07/26  21:07:59  caress
 * Initial version.
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
#include "../../include/mbsys_simrad.h"
#include "../../include/mbf_em12draw.h"

/* include for byte swapping */
#include "../../include/mb_swap.h"

/*--------------------------------------------------------------------*/
int mbr_alm_em12draw(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	static char res_id[]="$Id: mbr_em12draw.c,v 4.7 1999-02-04 23:52:54 caress Exp $";
	char	*function_name = "mbr_alm_em12draw";
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
	mb_io_ptr->structure_size = sizeof(struct mbf_em12draw_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mbsys_simrad_alloc(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);

	/* initialize everything to zeros */
	mbr_zero_em12draw(verbose,mb_io_ptr->raw_data,error);

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
int mbr_dem_em12draw(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_em12draw";
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
	status = mbsys_simrad_deall(
			verbose,mbio_ptr,
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
int mbr_zero_em12draw(verbose,data_ptr,error)
int	verbose;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_zero_em12draw";
	int	status = MB_SUCCESS;
	struct mbf_em12draw_struct *data;
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
	data = (struct mbf_em12draw_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		data->kind = MB_DATA_NONE;
		data->sonar = MBSYS_SIMRAD_EM12D;

		/* parameter datagram */
		data->par_year = 0;
		data->par_month = 0;
		data->par_day = 0;
		data->par_hour = 0;
		data->par_minute = 0;
		data->par_second = 0;
		data->par_centisecond = 0;
		data->pos_type = 0;	/* positioning system type */
		data->pos_delay = 0.0;	/* positioning system delay (sec) */
		data->roll_offset = 0.0;	/* roll offset (degrees) */
		data->pitch_offset = 0.0;	/* pitch offset (degrees) */
		data->heading_offset = 0.0;	/* heading offset (degrees) */
		data->em100_td = 0.0;	/* EM-100 tranducer depth (meters) */
		data->em100_tx = 0.0;	/* EM-100 tranducer fore-aft 
						offset (meters) */
		data->em100_ty = 0.0;	/* EM-100 tranducer athwartships 
						offset (meters) */
		data->em12_td = 0.0;	/* EM-12 tranducer depth (meters) */
		data->em12_tx = 0.0;	/* EM-12 tranducer fore-aft 
						offset (meters) */
		data->em12_ty = 0.0;	/* EM-12 tranducer athwartships 
						offset (meters) */
		data->em1000_td = 0.0;	/* EM-1000 tranducer depth (meters) */
		data->em1000_tx = 0.0;	/* EM-1000 tranducer fore-aft 
						offset (meters) */
		data->em1000_ty = 0.0;	/* EM-1000 tranducer athwartships 
						offset (meters) */
		for (i=0;i<128;i++)
			data->spare_parameter[i] = '\0';
		data->survey_line = 0;
		for (i=0;i<80;i++)
			data->comment[i] = '\0';

		/* position (position datagrams) */
		data->pos_year = 0;
		data->pos_month = 0;
		data->pos_day = 0;
		data->pos_hour = 0;
		data->pos_minute = 0;
		data->pos_second = 0;
		data->pos_centisecond = 0;
		data->latitude = 0.0;
		data->longitude = 0.0;
		data->utm_northing = 0.0;
		data->utm_easting = 0.0;
		data->utm_zone = 0;
		data->utm_zone_lon = 0.0;
		data->utm_system = 0;
		data->pos_quality = 0;
		data->speed = 0.0;			/* meters/second */
		data->line_heading = 0.0;		/* degrees */

		/* sound velocity profile */
		data->svp_year = 0;
		data->svp_month = 0;
		data->svp_day = 0;
		data->svp_hour = 0;
		data->svp_minute = 0;
		data->svp_second = 0;
		data->svp_centisecond = 0;
		data->svp_num = 0;
		for (i=0;i<100;i++)
			{
			data->svp_depth[i] = 0; /* meters */
			data->svp_vel[i] = 0;	/* 0.1 meters/sec */
			}

		/* time stamp */
		data->year = 0;
		data->month = 0;
		data->day = 0;
		data->hour = 0;
		data->minute = 0;
		data->second = 0;
		data->centisecond = 0;
		data->swath_id = EM_SWATH_CENTER;
		data->ping_number = 0;
		data->beams_bath = MBF_EM12DRAW_MAXBEAMS;
		data->bath_mode = 0;
		data->bath_res = 0;
		data->bath_quality = 0;
		data->keel_depth = 0;
		data->heading = 0;
		data->roll = 0;
		data->pitch = 0;
		data->xducer_pitch = 0;
		data->ping_heave = 0;
		data->sound_vel = 0;
		data->pixels_ss = 0;
		data->ss_mode = 0;
		for (i=0;i<MBF_EM12DRAW_MAXBEAMS;i++)
			{
			data->bath[i] = 0;
			data->bath_acrosstrack[i] = 0;
			data->bath_alongtrack[i] = 0;
			data->tt[i] = 0;
			data->amp[i] = 0;
			data->quality[i] = 0;
			data->heave[i] = 0;
			data->beam_frequency[i] = 0;
			data->beam_samples[i] = 0;
			data->beam_center_sample[i] = 0;
			data->beam_start_sample[i] = 0;
			}
		for (i=0;i<MBF_EM12DRAW_MAXPIXELS;i++)
			{
			data->ss[i] = 0;
			data->ssp[i] = 0;
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
int mbr_rt_em12draw(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_em12draw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em12draw_struct *data;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	mb_s_char *data_ss;
	mb_s_char *store_ss;
	short int *data_ssp;
	short int *store_ssp;
	int	ntime_i[7];
	double	ntime_d;
	double	ss_spacing;
	double	dd, dt, dx, dy, speed;
	double	mtodeglon, mtodeglat;
	double	headingx, headingy;
	double	depthscale, dacrscale, daloscale, ttscale, reflscale;
	int	ifix;
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
	data = (struct mbf_em12draw_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_simrad_struct *) store_ptr;

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
	for (i=0;i<mb_io_ptr->pixels_ss;i++)
		{
		mb_io_ptr->new_ss[i] = 0.0;
		mb_io_ptr->new_ss_acrosstrack[i] = 0.0;
		mb_io_ptr->new_ss_alongtrack[i] = 0.0;
		}

	/* read next data from file */
	status = mbr_em12draw_rd_data(verbose,mbio_ptr,error);

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
			mb_io_ptr->new_time_i[6] = 10000*data->centisecond;
			}
		else if (data->kind == MB_DATA_COMMENT
			|| data->kind == MB_DATA_START
			|| data->kind == MB_DATA_STOP)
			{
			mb_io_ptr->new_time_i[0] = data->par_year + 1900;
			mb_io_ptr->new_time_i[1] = data->par_month;
			mb_io_ptr->new_time_i[2] = data->par_day;
			mb_io_ptr->new_time_i[3] = data->par_hour;
			mb_io_ptr->new_time_i[4] = data->par_minute;
			mb_io_ptr->new_time_i[5] = data->par_second;
			mb_io_ptr->new_time_i[6] = 10000*data->par_centisecond;
			}
		else if (data->kind == MB_DATA_VELOCITY_PROFILE)
			{
			mb_io_ptr->new_time_i[0] = data->svp_year + 1900;
			mb_io_ptr->new_time_i[1] = data->svp_month;
			mb_io_ptr->new_time_i[2] = data->svp_day;
			mb_io_ptr->new_time_i[3] = data->svp_hour;
			mb_io_ptr->new_time_i[4] = data->svp_minute;
			mb_io_ptr->new_time_i[5] = data->svp_second;
			mb_io_ptr->new_time_i[6] = 10000*data->svp_centisecond;
			}
		else if (data->kind == MB_DATA_NAV)
			{
			mb_io_ptr->new_time_i[0] = data->pos_year + 1900;
			mb_io_ptr->new_time_i[1] = data->pos_month;
			mb_io_ptr->new_time_i[2] = data->pos_day;
			mb_io_ptr->new_time_i[3] = data->pos_hour;
			mb_io_ptr->new_time_i[4] = data->pos_minute;
			mb_io_ptr->new_time_i[5] = data->pos_second;
			mb_io_ptr->new_time_i[6] = 10000*data->pos_centisecond;
			}
		if (mb_io_ptr->new_time_i[0] < 1970)
			mb_io_ptr->new_time_d = 0.0;
		else
			mb_get_time(verbose,mb_io_ptr->new_time_i,
				&mb_io_ptr->new_time_d);
				
		/* save fix if nav data */
		if (data->kind == MB_DATA_NAV)
			{
			/* make room for latest fix */
			if (mb_io_ptr->nfix >= MB_NAV_SAVE_MAX)
				{
				for (i=0;i<mb_io_ptr->nfix-1;i++)
					{
					mb_io_ptr->fix_time_d[i]
					    = mb_io_ptr->fix_time_d[i+1];
					mb_io_ptr->fix_lon[i]
					    = mb_io_ptr->fix_lon[i+1];
					mb_io_ptr->fix_lat[i]
					    = mb_io_ptr->fix_lat[i+1];
					}
				mb_io_ptr->nfix--;
				}
			
			/* add latest fix */
			mb_io_ptr->fix_time_d[mb_io_ptr->nfix] 
				= mb_io_ptr->new_time_d;
			mb_io_ptr->fix_lon[mb_io_ptr->nfix] 
				= data->longitude;
			mb_io_ptr->fix_lat[mb_io_ptr->nfix] 
				= data->latitude;
			mb_io_ptr->nfix++;
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
		/* interpolate from saved nav if possible */
		if (mb_io_ptr->nfix > 1)
			{
			/* get speed if necessary */
			if (data->speed <= 0.0)
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
                            speed = 3.6 * sqrt(dx*dx + dy*dy)/dt; /* km/hr */
			    }
			else
			    {
			    speed = 3.6 * data->speed;
			    }
			if (speed > 100.0)
			    speed = 0.0;

			/* interpolation possible */
			if (mb_io_ptr->new_time_d 
				>= mb_io_ptr->fix_time_d[0]
			    && mb_io_ptr->new_time_d
				<= mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
			    {
			    ifix = 0;
			    while (mb_io_ptr->new_time_d
				> mb_io_ptr->fix_time_d[ifix+1])
				ifix++;
			    mb_io_ptr->new_lon = mb_io_ptr->fix_lon[ifix]
				+ (mb_io_ptr->fix_lon[ifix+1] 
				    - mb_io_ptr->fix_lon[ifix])
				* (mb_io_ptr->new_time_d
				    - mb_io_ptr->fix_time_d[ifix])
				/ (mb_io_ptr->fix_time_d[ifix+1]
				    - mb_io_ptr->fix_time_d[ifix]);
			    mb_io_ptr->new_lat = mb_io_ptr->fix_lat[ifix]
				+ (mb_io_ptr->fix_lat[ifix+1] 
				    - mb_io_ptr->fix_lat[ifix])
				* (mb_io_ptr->new_time_d
				    - mb_io_ptr->fix_time_d[ifix])
				/ (mb_io_ptr->fix_time_d[ifix+1]
				    - mb_io_ptr->fix_time_d[ifix]);
			    }
			
			/* extrapolate from first fix */
			else if (mb_io_ptr->new_time_d 
				< mb_io_ptr->fix_time_d[0])
			    {
			    dd = (mb_io_ptr->new_time_d 
				- mb_io_ptr->fix_time_d[0])
				* data->speed;
			    mb_coor_scale(verbose,mb_io_ptr->fix_lat[0],
				&mtodeglon,&mtodeglat);
			    headingx = sin(DTR*data->line_heading);
			    headingy = cos(DTR*data->line_heading);
			    mb_io_ptr->new_lon = mb_io_ptr->fix_lon[0] 
				+ headingx*mtodeglon*dd;
			    mb_io_ptr->new_lat = mb_io_ptr->fix_lat[0] 
				+ headingy*mtodeglat*dd;
			    }
			
			/* extrapolate from last fix */
			else if (mb_io_ptr->new_time_d 
				> mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
			    {
			    dd = (mb_io_ptr->new_time_d 
				- mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
				* data->speed;
			    mb_coor_scale(verbose,mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
				&mtodeglon,&mtodeglat);
			    headingx = sin(DTR*data->line_heading);
			    headingy = cos(DTR*data->line_heading);
			    mb_io_ptr->new_lon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1] 
				+ headingx*mtodeglon*dd;
			    mb_io_ptr->new_lat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1] 
				+ headingy*mtodeglat*dd;
			    }
			}
			
		/* else extrapolate from only fix */
		else if (mb_io_ptr->nfix == 1
			&& data->speed > 0.0)
			{
			dd = (mb_io_ptr->new_time_d - mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
				* speed / 3.6;
			mb_coor_scale(verbose,mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
				&mtodeglon,&mtodeglat);
			headingx = sin(DTR*(data->line_heading));
			headingy = cos(DTR*(data->line_heading));
			mb_io_ptr->new_lon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1] 
				+ headingx*mtodeglon*dd;
			mb_io_ptr->new_lat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1] 
				+ headingy*mtodeglat*dd;
			mb_io_ptr->new_speed = 3.6 * data->speed;
			}

		/* else just take last position */
		else if (mb_io_ptr->nfix == 1)
			{
			mb_io_ptr->new_lon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1];
			mb_io_ptr->new_lat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1];
			mb_io_ptr->new_speed = 0.0;
			}
		else
			{
			mb_io_ptr->new_lon = 0.0;
			mb_io_ptr->new_lat = 0.0;
			mb_io_ptr->new_speed = 0.0;
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

		/* get heading */
		mb_io_ptr->new_heading = 0.1*data->heading;

		/* get speed  */
		mb_io_ptr->new_speed = 3.6*data->speed;

		/* read beam and pixel values into storage arrays */
		mb_io_ptr->beams_bath = data->beams_bath;
		mb_io_ptr->beams_amp = data->beams_bath;
		mb_io_ptr->pixels_ss = data->pixels_ss;
		if (data->bath_res == 1)
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			daloscale  = 0.2;
			reflscale  = 0.5;
			}
		else
			{
			depthscale = 0.2;
			dacrscale  = 0.5;
			daloscale  = 0.5;
			reflscale  = 0.5;
			}
		/* fix problems with some EM12 data */
		if (data->ss_mode < 1 || data->ss_mode > 5)
			{
			if (data->bath_res == 1)
				data->ss_mode = 1;
			else
				data->ss_mode = 2;
			}
		if (data->ss_mode == 1)
			ss_spacing = 0.6;
		else if (data->ss_mode == 2)
			ss_spacing = 2.4;
		else if (data->ss_mode == 3)
			ss_spacing = 0.3;
		else if (data->ss_mode == 4)
			ss_spacing = 0.3;
		else
			ss_spacing = 0.15;
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			if (data->bath[i] < 0)
			    {
			    mb_io_ptr->new_beamflag[i] 
				    = MB_FLAG_MANUAL + MB_FLAG_FLAG;
			    mb_io_ptr->new_bath[i] 
				    = -depthscale*data->bath[i];
			    }
			else if (data->bath[i] > 0)
			    {
			    mb_io_ptr->new_beamflag[i] 
				    = MB_FLAG_NONE;
			    mb_io_ptr->new_bath[i] 
				    = depthscale*data->bath[i];
			    }
			else
			    {
			    mb_io_ptr->new_beamflag[i] 
				    = MB_FLAG_NULL;
			    mb_io_ptr->new_bath[i] 
				    = depthscale*data->bath[i];
			    }
			mb_io_ptr->new_bath_acrosstrack[i] 
				= dacrscale*data->bath_acrosstrack[i];
			mb_io_ptr->new_bath_alongtrack[i] 
				= daloscale*data->bath_alongtrack[i];
			}
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			if (data->bath[i] != 0)
				mb_io_ptr->new_amp[i] = 
					reflscale * data->amp[i] + 64;
			else
				mb_io_ptr->new_amp[i] = 0.0;
			}
		mb_io_ptr->pixels_ss = 0;
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			data_ss = (mb_s_char *) &data->ss[data->beam_start_sample[i]];
			for (j=0;j<data->beam_samples[i];j++)
				{
				mb_io_ptr->new_ss[mb_io_ptr->pixels_ss] 
					= reflscale * data_ss[j] + 64;
				mb_io_ptr->new_ss_acrosstrack[mb_io_ptr->pixels_ss] 
					= dacrscale*data->bath_acrosstrack[i] 
					+ ss_spacing*
					(j - data->beam_center_sample[i]);
				mb_io_ptr->new_ss_alongtrack[mb_io_ptr->pixels_ss] 
					= daloscale*data->bath_alongtrack[i];
				mb_io_ptr->pixels_ss++;
				}
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
			  fprintf(stderr,"dbg4       beam:%d  flag:%3d  bath:%f  amp:%f  acrosstrack:%f  alongtrack:%f\n",
				i,mb_io_ptr->new_beamflag[i],
				mb_io_ptr->new_bath[i],
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

	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_NAV)
		{
		mb_io_ptr->new_lon 
			= data->longitude;
		mb_io_ptr->new_lat 
			= data->latitude;
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
		}

	/* copy comment to mbio descriptor structure */
	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(mb_io_ptr->new_comment,data->comment,80);

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

	/* translate values to simrad data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		store->kind = data->kind;
		store->sonar = data->sonar;

		/* parameter datagram */
		store->par_year = data->par_year;
		store->par_month = data->par_month;
		store->par_day = data->par_day;
		store->par_hour = data->par_hour;
		store->par_minute = data->par_minute;
		store->par_second = data->par_second;
		store->par_centisecond = data->par_centisecond;
		store->pos_type = data->pos_type;
		store->pos_delay = data->pos_delay;
		store->roll_offset = data->roll_offset;
		store->pitch_offset = data->pitch_offset;
		store->heading_offset = data->heading_offset;
		store->em100_td = data->em100_td;
		store->em100_tx = data->em100_tx;
		store->em100_ty = data->em100_ty;
		store->em12_td = data->em12_td;
		store->em12_tx = data->em12_tx;
		store->em12_ty = data->em12_ty;
		store->em1000_td = data->em1000_td;
		store->em1000_tx = data->em1000_tx;
		store->em1000_ty = data->em1000_ty;
		for (i=0;i<128;i++)
			store->spare_parameter[i] = data->spare_parameter[i];
		store->survey_line = data->survey_line;
		for (i=0;i<80;i++)
			store->comment[i] = data->comment[i];

		/* position (position datagrams) */
		store->pos_year = data->pos_year;
		store->pos_month = data->pos_month;
		store->pos_day = data->pos_day;
		store->pos_hour = data->pos_hour;
		store->pos_minute = data->pos_minute;
		store->pos_second = data->pos_second;
		store->pos_centisecond = data->pos_centisecond;
		store->latitude = data->latitude;
		store->longitude = data->longitude;
		store->utm_northing = data->utm_northing;
		store->utm_easting = data->utm_easting;
		store->utm_zone = data->utm_zone;
		store->utm_zone_lon = data->utm_zone_lon;
		store->utm_system = data->utm_system;
		store->pos_quality = data->pos_quality;
		store->speed = data->speed;
		store->line_heading = data->line_heading;

		/* sound velocity profile */
		store->svp_year = data->svp_year;
		store->svp_month = data->svp_month;
		store->svp_day = data->svp_day;
		store->svp_hour = data->svp_hour;
		store->svp_minute = data->svp_minute;
		store->svp_second = data->svp_second;
		store->svp_centisecond = data->svp_centisecond;
		store->svp_num = data->svp_num;
		for (i=0;i<100;i++)
			{
			store->svp_depth[i] = data->svp_depth[i];
			store->svp_vel[i] = data->svp_vel[i];
			}

		/* time stamp */
		store->year = data->year;
		store->month = data->month;
		store->day = data->day;
		store->hour = data->hour;
		store->minute = data->minute;
		store->second = data->second;
		store->centisecond = data->centisecond;
		
		/* allocate secondary data structure for
			survey data if needed */
		if (data->kind == MB_DATA_DATA
			&& store->ping == NULL)
			{
			status = mbsys_simrad_survey_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}
		
		/* deal with putting survey data into
		secondary data structure */
		if (status == MB_SUCCESS 
			&& data->kind == MB_DATA_DATA)
			{
			/* get data structure pointer */
			ping = (struct mbsys_simrad_survey_struct *) 
				store->ping;

			/* copy data */
			ping->swath_id = data->swath_id;
			ping->ping_number = data->ping_number;
			ping->beams_bath = data->beams_bath;
			ping->bath_mode = data->bath_mode;
			ping->bath_res = data->bath_res;
			ping->bath_quality = data->bath_quality;
			ping->keel_depth = data->keel_depth;
			ping->heading = data->heading;
			ping->roll = data->roll;
			ping->pitch = data->pitch;
			ping->xducer_pitch = data->xducer_pitch;
			ping->ping_heave = data->ping_heave;
			ping->sound_vel = data->sound_vel;
			ping->pixels_ss = data->pixels_ss;
			ping->ss_mode = data->ss_mode;
			for (i=0;i<ping->beams_bath;i++)
				{
				ping->bath[i] = data->bath[i];
				ping->bath_acrosstrack[i] = data->bath_acrosstrack[i];
				ping->bath_alongtrack[i] = data->bath_alongtrack[i];
				ping->tt[i] = data->tt[i];
				ping->amp[i] = data->amp[i];
				ping->quality[i] = data->quality[i];
				ping->heave[i] = data->heave[i];
				ping->beam_frequency[i] = data->beam_frequency[i];
				ping->beam_samples[i] = data->beam_samples[i];
				ping->beam_center_sample[i] 
					= data->beam_center_sample[i];
				ping->beam_start_sample[i] 
					= data->beam_start_sample[i];
				if (ping->beam_samples[i] > 0)
					{
					store_ss = (mb_s_char *) &ping->ss[data->beam_start_sample[i]];
					data_ss = (mb_s_char *) &data->ss[data->beam_start_sample[i]];
					store_ssp = &ping->ssp[data->beam_start_sample[i]];
					data_ssp = &data->ssp[data->beam_start_sample[i]];
					for (j=0;j<ping->beam_samples[i];j++)
						{
						store_ss[j] = data_ss[j];
						store_ssp[j] = data_ssp[j];
						}
					}
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
int mbr_wt_em12draw(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_em12draw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em12draw_struct *data;
	char	*data_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	double	scalefactor;
	int	time_j[5];
	double	depthscale, dacrscale, daloscale, ttscale, reflscale;
	int	iss;
	mb_s_char *data_ss;
	mb_s_char *store_ss;
	short int *data_ssp;
	short int *store_ssp;
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
	data = (struct mbf_em12draw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	store = (struct mbsys_simrad_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		data->kind = store->kind;
		data->sonar = store->sonar;

		/* parameter datagram */
		data->par_year = store->par_year;
		data->par_month = store->par_month;
		data->par_day = store->par_day;
		data->par_hour = store->par_hour;
		data->par_minute = store->par_minute;
		data->par_second = store->par_second;
		data->par_centisecond = store->par_centisecond;
		data->pos_type = store->pos_type;
		data->pos_delay = store->pos_delay;
		data->roll_offset = store->roll_offset;
		data->pitch_offset = store->pitch_offset;
		data->heading_offset = store->heading_offset;
		data->em100_td = store->em100_td;
		data->em100_tx = store->em100_tx;
		data->em100_ty = store->em100_ty;
		data->em12_td = store->em12_td;
		data->em12_tx = store->em12_tx;
		data->em12_ty = store->em12_ty;
		data->em1000_td = store->em1000_td;
		data->em1000_tx = store->em1000_tx;
		data->em1000_ty = store->em1000_ty;
		for (i=0;i<128;i++)
			data->spare_parameter[i] = store->spare_parameter[i];
		data->survey_line = store->survey_line;
		for (i=0;i<80;i++)
			data->comment[i] = store->comment[i];

		/* position (position datagrams) */
		data->pos_year = store->pos_year;
		data->pos_month = store->pos_month;
		data->pos_day = store->pos_day;
		data->pos_hour = store->pos_hour;
		data->pos_minute = store->pos_minute;
		data->pos_second = store->pos_second;
		data->pos_centisecond = store->pos_centisecond;
		data->latitude = store->latitude;
		data->longitude = store->longitude;
		data->utm_northing = store->utm_northing;
		data->utm_easting = store->utm_easting;
		data->utm_zone = store->utm_zone;
		data->utm_zone_lon = store->utm_zone_lon;
		data->utm_system = store->utm_system;
		data->pos_quality = store->pos_quality;
		data->speed = store->speed;
		data->line_heading = store->line_heading;

		/* sound velocity profile */
		data->svp_year = store->svp_year;
		data->svp_month = store->svp_month;
		data->svp_day = store->svp_day;
		data->svp_hour = store->svp_hour;
		data->svp_minute = store->svp_minute;
		data->svp_second = store->svp_second;
		data->svp_centisecond = store->svp_centisecond;
		data->svp_num = store->svp_num;
		for (i=0;i<100;i++)
			{
			data->svp_depth[i] = store->svp_depth[i];
			data->svp_vel[i] = store->svp_vel[i];
			}

		/* time stamp */
		data->year = store->year;
		data->month = store->month;
		data->day = store->day;
		data->hour = store->hour;
		data->minute = store->minute;
		data->second = store->second;
		data->centisecond = store->centisecond;
		
		/* deal with survey data 
			in secondary data structure */
		if (store->ping != NULL)
			{
			/* get data structure pointer */
			ping = (struct mbsys_simrad_survey_struct *) 
				store->ping;

			/* copy survey data */
			data->swath_id = ping->swath_id;
			data->ping_number = ping->ping_number;
			data->beams_bath = ping->beams_bath;
			data->bath_mode = ping->bath_mode;
			data->bath_res = ping->bath_res;
			data->bath_quality = ping->bath_quality;
			data->keel_depth = ping->keel_depth;
			data->heading = ping->heading;
			data->roll = ping->roll;
			data->pitch = ping->pitch;
			data->xducer_pitch = ping->xducer_pitch;
			data->ping_heave = ping->ping_heave;
			data->sound_vel = ping->sound_vel;
			data->pixels_ss = ping->pixels_ss;
			data->ss_mode = ping->ss_mode;
			for (i=0;i<data->beams_bath;i++)
				{
				data->bath[i] = ping->bath[i];
				data->bath_acrosstrack[i] = ping->bath_acrosstrack[i];
				data->bath_alongtrack[i] = ping->bath_alongtrack[i];
				data->tt[i] = ping->tt[i];
				data->amp[i] = ping->amp[i];
				data->quality[i] = ping->quality[i];
				data->heave[i] = ping->heave[i];
				data->beam_frequency[i] = ping->beam_frequency[i];
				data->beam_samples[i] = ping->beam_samples[i];
				data->beam_center_sample[i] 
					= ping->beam_center_sample[i];
				data->beam_start_sample[i] 
					= ping->beam_start_sample[i];
				if (data->beam_samples[i] > 0)
					{
					data_ss = (mb_s_char *) &data->ss[data->beam_start_sample[i]];
					store_ss = (mb_s_char *) &ping->ss[ping->beam_start_sample[i]];
					data_ssp = &data->ssp[data->beam_start_sample[i]];
					store_ssp = &ping->ssp[ping->beam_start_sample[i]];
					for (j=0;j<data->beam_samples[i];j++)
						{
						data_ss[j] = store_ss[j];
						data_ssp[j] = store_ssp[j];
						}
					}
				}
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
		data->centisecond = mb_io_ptr->new_time_i[6]/10000;
		}

	/* check for comment to be copied from mb_io_ptr */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		strncpy(data->comment,mb_io_ptr->new_comment,
			79);
		}

	/* else check for ping data to be copied from mb_io_ptr */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/* get heading */
		data->heading = (int) (mb_io_ptr->new_heading * 10);

		/* get speed  */
		data->speed = mb_io_ptr->new_speed/3.6;

		/* insert distance and depth values into storage arrays */
		data->beams_bath = mb_io_ptr->beams_bath;
		data->sonar = MBSYS_SIMRAD_EM12D;
		if (data->bath_res == 1)
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			daloscale  = 0.2;
			reflscale  = 0.5;
			}
		else
			{
			depthscale = 0.2;
			dacrscale  = 0.5;
			daloscale  = 0.5;
			reflscale  = 0.5;
			}
		if (status == MB_SUCCESS)
			{
			for (i=0;i<mb_io_ptr->beams_bath;i++)
				{
				if (mb_beam_check_flag(
					mb_io_ptr->new_beamflag[i]))
				    data->bath[i] 
					= -mb_io_ptr->new_bath[i]
						/depthscale;
				else
				    data->bath[i] 
					= mb_io_ptr->new_bath[i]
						/depthscale;
				data->bath_acrosstrack[i]
					= mb_io_ptr->new_bath_acrosstrack[i]/dacrscale;
				data->bath_alongtrack[i] 
					= mb_io_ptr->new_bath_alongtrack[i]/daloscale;
				}
			for (i=0;i<mb_io_ptr->beams_bath;i++)
				{
				if (data->bath[i] != 0)
					data->amp[i] = 
						(mb_io_ptr->new_amp[i] -64) 
						/ reflscale;
				else
					data->amp[i] = 0;
				}
			}
		if (status == MB_SUCCESS && mb_io_ptr->pixels_ss == data->pixels_ss)
			{
			for (i=0;i<data->pixels_ss;i++)
				{
				data->ss[i] = (mb_io_ptr->new_ss[i] - 64) / reflscale;
				}
			}
		}

	/* else check for nav data to be copied from mb_io_ptr */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_NAV)
		{
		/* get time */
		data->pos_year = mb_io_ptr->new_time_i[0] - 1900;
		data->pos_month = mb_io_ptr->new_time_i[1];
		data->pos_day = mb_io_ptr->new_time_i[2];
		data->pos_hour = mb_io_ptr->new_time_i[3];
		data->pos_minute = mb_io_ptr->new_time_i[4];
		data->pos_second = mb_io_ptr->new_time_i[5];
		data->pos_centisecond = mb_io_ptr->new_time_i[6]/10000;

		/* get position */
		data->longitude = mb_io_ptr->new_lon;
		data->latitude = mb_io_ptr->new_lat;

		/* get heading */
		data->line_heading = mb_io_ptr->new_heading;

		/* get speed  */
		data->speed = mb_io_ptr->new_speed/3.6;
		
		/* zero the rest */
		data->utm_northing = 0.0;
		data->utm_easting = 0.0;
		data->utm_zone = 0;
		data->utm_zone_lon = 0.0;
		data->utm_system = 0;
		data->pos_quality = 0;
		}

	/* write next data to file */
	status = mbr_em12draw_wr_data(verbose,mbio_ptr,data_ptr,error);

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
int mbr_em12draw_rd_data(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_em12draw_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em12draw_struct *data;
	char	*data_ptr;
	FILE	*mbfp;
	int	done;
	char	*label;
	int	*label_save_flag;
	short int expect;
	short int *type;
	short int first_type;
	int	first_ss;
	int	more_ss;

	short int *expect_save;
	int	*expect_save_flag;
	short int *first_type_save;
	int	*first_ss_save;
	int	*more_ss_save;

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
	data = (struct mbf_em12draw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	mbfp = mb_io_ptr->mbfp;
	
	/* get saved values */
	label = (char *) mb_io_ptr->save_label;
	type = (short int *) mb_io_ptr->save_label;
	label_save_flag = (int *) &mb_io_ptr->save_label_flag;
	expect_save_flag = (int *) &mb_io_ptr->save_flag;
	expect_save = (short int *) &mb_io_ptr->save1;
	first_type_save = (short int *) &mb_io_ptr->save2;
	first_ss_save = (int *) &mb_io_ptr->save3;
	more_ss_save = (int *) &mb_io_ptr->save4;
	if (*expect_save_flag == MB_YES)
		{
		expect = *expect_save;
		first_type = *first_type_save;
		first_ss = *first_ss_save;
		more_ss = *more_ss_save;
		*expect_save_flag = MB_NO;
		}
	else
		{
		expect = EM_NONE;
		first_type = EM_NONE;
		first_ss = MB_YES;
		more_ss = MB_NO;
		}

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* loop over reading data until a record is ready for return */
	done = MB_NO;
	*error = MB_ERROR_NO_ERROR;
	while (done == MB_NO)
		{
		/* if no label saved get next record label */
		if (*label_save_flag == MB_NO)
			{
			if ((status = fread(&label[0],
				1,1,mb_io_ptr->mbfp)) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			if (label[0] == 0x02)
			    if ((status = fread(&label[1],
				    1,1,mb_io_ptr->mbfp)) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			}
		
		/* else use saved label */
		else
			*label_save_flag = MB_NO;

		/* swap bytes if necessary */
#ifdef BYTESWAPPED
		*type = (short int) mb_swap_short(*type);
#endif

		/*fprintf(stderr,"\nstart of mbr_em12draw_rd_data loop:\n");
		fprintf(stderr,"done:%d\n",done);
		fprintf(stderr,"expect:%x\n",expect);
		fprintf(stderr,"type:%x\n",*type);
		fprintf(stderr,"first_type:%x\n",first_type);
		fprintf(stderr,"first_ss:%d\n",first_ss);
		fprintf(stderr,"more_ss:%d\n",more_ss);*/

		/* read the appropriate data records */
		if (status == MB_FAILURE && expect == EM_NONE)
			{
	/*fprintf(stderr,"call nothing, read failure, no expect\n");*/
			done = MB_YES;
			}
		else if (status == MB_FAILURE && expect != EM_NONE)
			{
	/*fprintf(stderr,"call nothing, read failure, expect %x\n",expect);*/
			done = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else if (*type != EM_START
			&& *type != EM_STOP
			&& *type != EM_PARAMETER
			&& *type != EM_POS
			&& *type != EM_SVP
			&& *type != EM_12DP_BATH
			&& *type != EM_12DP_SSP
			&& *type != EM_12DS_BATH
			&& *type != EM_12DS_SSP)
			{
	/*fprintf(stderr,"call nothing, try again\n");*/
			done = MB_NO;
			}
		else if (*type == EM_START)
			{
	/*fprintf(stderr,"call mbr_em12draw_rd_start type %x\n",*type);*/
			status = mbr_em12draw_rd_start(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_START;
				if (expect != EM_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					*first_ss_save = first_ss;
					*more_ss_save = more_ss;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM_STOP)
			{
	/*fprintf(stderr,"call mbr_em12draw_rd_stop type %x\n",*type);*/
			status = mbr_em12draw_rd_stop(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_STOP;
				if (expect != EM_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					*first_ss_save = first_ss;
					*more_ss_save = more_ss;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM_PARAMETER)
			{
	/*fprintf(stderr,"call mbr_em12draw_rd_parameter type %x\n",*type);*/
			status = mbr_em12draw_rd_parameter(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_COMMENT;
				if (expect != EM_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					*first_ss_save = first_ss;
					*more_ss_save = more_ss;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM_POS)
			{
	/*fprintf(stderr,"call mbr_em12draw_rd_pos type %x\n",*type);*/
			status = mbr_em12draw_rd_pos(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_NAV;
				if (expect != EM_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					*first_ss_save = first_ss;
					*more_ss_save = more_ss;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM_SVP)
			{
	/*fprintf(stderr,"call mbr_em12draw_rd_svp type %x\n",*type);*/
			status = mbr_em12draw_rd_svp(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_VELOCITY_PROFILE;
				if (expect != EM_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					*first_ss_save = first_ss;
					*more_ss_save = more_ss;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM_12DP_BATH 
			&& expect != EM_NONE 
			&& expect != EM_12DP_BATH)
			{
	/*fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);*/
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_12DP_BATH)
			{
	/*fprintf(stderr,"call mbr_em12draw_rd_bath type %x\n",*type);*/
			status = mbr_em12draw_rd_bath(
				verbose,mbfp,data,
				EM_SWATH_PORT,error);
			if (status == MB_SUCCESS)
				{
				data->kind = MB_DATA_DATA;
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12DP_BATH;
					expect = EM_12DP_SSP;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				}
			}
		else if (*type == EM_12DP_SSP 
			&& expect != EM_NONE 
			&& expect != EM_12DP_SSP)
			{
	/*fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);*/
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_12DP_SSP)
			{
	/*fprintf(stderr,"call mbr_em12draw_rd_ss type %x\n",*type);*/
			status = mbr_em12draw_rd_ss(
				verbose,mbfp,data,
				EM_SWATH_PORT,
				first_ss,&more_ss,error);
			if (status == MB_SUCCESS 
				&& more_ss == MB_NO)
				{
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12DP_SSP;
					expect = EM_12DP_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			else if (status == MB_SUCCESS 
				&& more_ss == MB_YES)
				{
				done = MB_NO;
				expect = EM_12DP_SSP;
				first_ss = MB_NO;
				}
			else if (status == MB_FAILURE)
				{
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12DP_SSP;
					expect = EM_12DP_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			}

		else if (*type == EM_12DS_BATH 
			&& expect != EM_NONE 
			&& expect != EM_12DS_BATH)
			{
	/*fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);*/
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_12DS_BATH)
			{
	/*fprintf(stderr,"call mbr_em12draw_rd_bath type %x\n",*type);*/
			status = mbr_em12draw_rd_bath(
				verbose,mbfp,data,
				EM_SWATH_STARBOARD,error);
			if (status == MB_SUCCESS)
				{
				data->kind = MB_DATA_DATA;
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12DS_BATH;
					expect = EM_12DS_SSP;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				}
			}
		else if (*type == EM_12DS_SSP 
			&& expect != EM_NONE 
			&& expect != EM_12DS_SSP)
			{
	/*fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);*/
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_12DS_SSP)
			{
	/*fprintf(stderr,"call mbr_em12draw_rd_ss type %x\n",*type);*/
			status = mbr_em12draw_rd_ss(
				verbose,mbfp,data,
				EM_SWATH_STARBOARD,
				first_ss,&more_ss,error);
			if (status == MB_SUCCESS 
				&& more_ss == MB_NO)
				{
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12DS_SSP;
					expect = EM_12DS_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			else if (status == MB_SUCCESS 
				&& more_ss == MB_YES)
				{
				done = MB_NO;
				expect = EM_12DS_SSP;
				first_ss = MB_NO;
				}
			else if (status == MB_FAILURE)
				{
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12DS_SSP;
					expect = EM_12DS_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			}

		/* bail out if there is an error */
		if (status == MB_FAILURE)
			done = MB_YES;

		/*fprintf(stderr,"end of mbr_em12draw_rd_data loop:\n");
		fprintf(stderr,"done:%d\n",done);
		fprintf(stderr,"expect:%x\n",expect);
		fprintf(stderr,"type:%x\n",*type);*/
		}
		
	/* get file position */
	if (*label_save_flag == MB_YES)
		mb_io_ptr->file_bytes = ftell(mbfp) - 2;
	else if (*expect_save_flag != MB_YES)
		mb_io_ptr->file_bytes = ftell(mbfp);

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
int mbr_em12draw_rd_start(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_em12draw_struct *data;
int	*error;
{
	char	*function_name = "mbr_em12draw_rd_start";
	int	status = MB_SUCCESS;
	char	line[EM_START_SIZE+3];
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
	status = fread(line,1,EM_START_SIZE+3,mbfp);
	if (status == EM_START_SIZE+3)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		data->kind = MB_DATA_START;
		mb_get_int(&(data->par_day),          line,      2);
		mb_get_int(&(data->par_month),        line+2,    2);
		mb_get_int(&(data->par_year),         line+4,    2);
		mb_get_int(&(data->par_hour),         line+7,    2);
		mb_get_int(&(data->par_minute),       line+9,    2);
		mb_get_int(&(data->par_second),       line+11,   2);
		mb_get_int(&(data->par_centisecond),  line+13,   2);
		mb_get_int(&(data->pos_type),         line+20,   1);
		mb_get_double(&(data->pos_delay),     line+26,   5);
		mb_get_double(&(data->roll_offset),   line+36,   5);
		mb_get_double(&(data->pitch_offset),  line+46,   5);
		mb_get_double(&(data->heading_offset),line+56,   5);
		mb_get_double(&(data->em100_td),      line+70,   5);
		mb_get_double(&(data->em100_tx),      line+84,   5);
		mb_get_double(&(data->em100_ty),      line+98,   5);
		mb_get_double(&(data->em12_td),       line+111,  5);
		mb_get_double(&(data->em12_tx),       line+124,  5);
		mb_get_double(&(data->em12_ty),       line+137,  5);
		mb_get_double(&(data->em1000_td),     line+152,  5);
		mb_get_double(&(data->em1000_tx),     line+167,  5);
		mb_get_double(&(data->em1000_ty),     line+182,  5);
		for(i=0;i<128;i++)
			data->spare_parameter[i] = line[188+i];
		mb_get_int(&(data->survey_line),      line+328,  4);
		for(i=0;i<80;i++)
			data->comment[i] = line[341+i];
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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->par_centisecond);
		fprintf(stderr,"dbg5       pos_type:         %d\n",data->pos_type);
		fprintf(stderr,"dbg5       pos_delay:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       roll_offset:      %f\n",data->roll_offset);
		fprintf(stderr,"dbg5       pitch_offset:     %f\n",data->pitch_offset);
		fprintf(stderr,"dbg5       heading_offset:   %f\n",data->heading_offset);
		fprintf(stderr,"dbg5       em100_td:         %f\n",data->em100_td);
		fprintf(stderr,"dbg5       em100_tx:         %f\n",data->em100_tx);
		fprintf(stderr,"dbg5       em100_ty:         %f\n",data->em100_ty);
		fprintf(stderr,"dbg5       em12_td:          %f\n",data->em12_td);
		fprintf(stderr,"dbg5       em12_tx:          %f\n",data->em12_tx);
		fprintf(stderr,"dbg5       em12_ty:          %f\n",data->em12_ty);
		fprintf(stderr,"dbg5       em1000_td:        %f\n",data->em1000_td);
		fprintf(stderr,"dbg5       em1000_tx:        %f\n",data->em1000_tx);
		fprintf(stderr,"dbg5       em1000_ty:        %f\n",data->em1000_ty);
		fprintf(stderr,"dbg5       survey_line:      %d\n",data->survey_line);
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
int mbr_em12draw_rd_stop(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_em12draw_struct *data;
int	*error;
{
	char	*function_name = "mbr_em12draw_rd_stop";
	int	status = MB_SUCCESS;
	char	line[EM_STOP_SIZE+3];
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
	status = fread(line,1,EM_STOP_SIZE+3,mbfp);
	if (status == EM_STOP_SIZE+3)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		data->kind = MB_DATA_STOP;
		mb_get_int(&(data->par_day),          line,      2);
		mb_get_int(&(data->par_month),        line+2,    2);
		mb_get_int(&(data->par_year),         line+4,    2);
		mb_get_int(&(data->par_hour),         line+7,    2);
		mb_get_int(&(data->par_minute),       line+9,    2);
		mb_get_int(&(data->par_second),       line+11,   2);
		mb_get_int(&(data->par_centisecond),  line+13,   2);
		mb_get_int(&(data->pos_type),         line+20,   1);
		mb_get_double(&(data->pos_delay),     line+26,   5);
		mb_get_double(&(data->roll_offset),   line+36,   5);
		mb_get_double(&(data->pitch_offset),  line+46,   5);
		mb_get_double(&(data->heading_offset),line+56,   5);
		mb_get_double(&(data->em100_td),      line+70,   5);
		mb_get_double(&(data->em100_tx),      line+84,   5);
		mb_get_double(&(data->em100_ty),      line+98,   5);
		mb_get_double(&(data->em12_td),       line+111,  5);
		mb_get_double(&(data->em12_tx),       line+124,  5);
		mb_get_double(&(data->em12_ty),       line+137,  5);
		mb_get_double(&(data->em1000_td),     line+152,  5);
		mb_get_double(&(data->em1000_tx),     line+167,  5);
		mb_get_double(&(data->em1000_ty),     line+182,  5);
		for(i=0;i<128;i++)
			data->spare_parameter[i] = line[188+i];
		mb_get_int(&(data->survey_line),      line+328,  4);
		for(i=0;i<80;i++)
			data->comment[i] = line[341+i];
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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->par_centisecond);
		fprintf(stderr,"dbg5       pos_type:         %d\n",data->pos_type);
		fprintf(stderr,"dbg5       pos_delay:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       roll_offset:      %f\n",data->roll_offset);
		fprintf(stderr,"dbg5       pitch_offset:     %f\n",data->pitch_offset);
		fprintf(stderr,"dbg5       heading_offset:   %f\n",data->heading_offset);
		fprintf(stderr,"dbg5       em100_td:         %f\n",data->em100_td);
		fprintf(stderr,"dbg5       em100_tx:         %f\n",data->em100_tx);
		fprintf(stderr,"dbg5       em100_ty:         %f\n",data->em100_ty);
		fprintf(stderr,"dbg5       em12_td:          %f\n",data->em12_td);
		fprintf(stderr,"dbg5       em12_tx:          %f\n",data->em12_tx);
		fprintf(stderr,"dbg5       em12_ty:          %f\n",data->em12_ty);
		fprintf(stderr,"dbg5       em1000_td:        %f\n",data->em1000_td);
		fprintf(stderr,"dbg5       em1000_tx:        %f\n",data->em1000_tx);
		fprintf(stderr,"dbg5       em1000_ty:        %f\n",data->em1000_ty);
		fprintf(stderr,"dbg5       survey_line:      %d\n",data->survey_line);
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
int mbr_em12draw_rd_parameter(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_em12draw_struct *data;
int	*error;
{
	char	*function_name = "mbr_em12draw_rd_parameter";
	int	status = MB_SUCCESS;
	char	line[EM_PARAMETER_SIZE+3];
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
	status = fread(line,1,EM_PARAMETER_SIZE+3,mbfp);
	if (status == EM_PARAMETER_SIZE+3)
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
		mb_get_int(&(data->par_day),          line,      2);
		mb_get_int(&(data->par_month),        line+2,    2);
		mb_get_int(&(data->par_year),         line+4,    2);
		mb_get_int(&(data->par_hour),         line+7,    2);
		mb_get_int(&(data->par_minute),       line+9,    2);
		mb_get_int(&(data->par_second),       line+11,   2);
		mb_get_int(&(data->par_centisecond),  line+13,   2);
		mb_get_int(&(data->pos_type),         line+20,   1);
		mb_get_double(&(data->pos_delay),     line+26,   5);
		mb_get_double(&(data->roll_offset),   line+36,   5);
		mb_get_double(&(data->pitch_offset),  line+46,   5);
		mb_get_double(&(data->heading_offset),line+56,   5);
		mb_get_double(&(data->em100_td),      line+70,   5);
		mb_get_double(&(data->em100_tx),      line+84,   5);
		mb_get_double(&(data->em100_ty),      line+98,   5);
		mb_get_double(&(data->em12_td),       line+111,  5);
		mb_get_double(&(data->em12_tx),       line+124,  5);
		mb_get_double(&(data->em12_ty),       line+137,  5);
		mb_get_double(&(data->em1000_td),     line+152,  5);
		mb_get_double(&(data->em1000_tx),     line+167,  5);
		mb_get_double(&(data->em1000_ty),     line+182,  5);
		for(i=0;i<128;i++)
			data->spare_parameter[i] = line[188+i];
		mb_get_int(&(data->survey_line),      line+328,  4);
		for(i=0;i<80;i++)
			data->comment[i] = line[341+i];
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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->par_centisecond);
		fprintf(stderr,"dbg5       pos_type:         %d\n",data->pos_type);
		fprintf(stderr,"dbg5       pos_delay:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       roll_offset:      %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       pitch_offset:     %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       heading_offset:   %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em100_td:         %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em100_tx:         %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em100_ty:         %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em12_td:          %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em12_tx:          %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em12_ty:          %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em1000_td:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em1000_tx:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em1000_ty:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       survey_line:      %d\n",data->survey_line);
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
int mbr_em12draw_rd_pos(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_em12draw_struct *data;
int	*error;
{
	char	*function_name = "mbr_em12draw_rd_pos";
	int	status = MB_SUCCESS;
	char	line[EM_POS_SIZE+3];
	int	degree;
	double	minute;
	char	hemisphere;
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
	status = fread(line,1,EM_POS_SIZE+3,mbfp);
	if (status == EM_POS_SIZE+3)
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
		mb_get_int(&(data->pos_day),          line,      2);
		mb_get_int(&(data->pos_month),        line+2,    2);
		mb_get_int(&(data->pos_year),         line+4,    2);
		mb_get_int(&(data->pos_hour),         line+7,    2);
		mb_get_int(&(data->pos_minute),       line+9,    2);
		mb_get_int(&(data->pos_second),       line+11,   2);
		mb_get_int(&(data->pos_centisecond),  line+13,   2);
		mb_get_int(&degree,                   line+16,   2);
		mb_get_double(&minute,                line+18,   7);
		hemisphere = line[25];
		data->latitude = degree + minute/60.0;
		if (hemisphere == 'S' || hemisphere == 's')
			data->latitude = -data->latitude;
		mb_get_int(&degree,                   line+27,   3);
		mb_get_double(&minute,                line+30,   7);
		hemisphere = line[37];
		data->longitude = degree + minute/60.0;
		if (hemisphere == 'W' || hemisphere == 'w')
			data->longitude = -data->longitude;
		mb_get_double(&(data->utm_northing),  line+39,  11);
		mb_get_double(&(data->utm_easting),   line+51,   9);
		mb_get_int(&(data->utm_zone),         line+61,   2);
		mb_get_int(&degree,                   line+64,   3);
		mb_get_double(&minute,                line+67,   7);
		hemisphere = line[74];
		data->utm_zone_lon = degree + minute/60.0;
		if (hemisphere == 'W' || hemisphere == 'w')
			data->utm_zone_lon = -data->utm_zone_lon;
		mb_get_int(&(data->utm_system),       line+76,   1);
		mb_get_int(&(data->pos_quality),      line+78,   1);
		mb_get_double(&(data->speed),         line+80,   4);
		mb_get_double(&(data->line_heading),  line+85,   5);
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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->pos_centisecond);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->longitude);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->latitude);
		fprintf(stderr,"dbg5       utm_northing:     %f\n",data->utm_northing);
		fprintf(stderr,"dbg5       utm_easting:      %f\n",data->utm_easting);
		fprintf(stderr,"dbg5       utm_zone:         %d\n",data->utm_zone);
		fprintf(stderr,"dbg5       utm_zone_lon:     %f\n",data->utm_zone_lon);
		fprintf(stderr,"dbg5       utm_system:       %d\n",data->utm_system);
		fprintf(stderr,"dbg5       pos_quality:      %d\n",data->pos_quality);
		fprintf(stderr,"dbg5       speed:            %f\n",data->speed);
		fprintf(stderr,"dbg5       line_heading:     %f\n",data->line_heading);
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
int mbr_em12draw_rd_svp(verbose,mbfp,data,error)
int	verbose;
FILE	*mbfp;
struct mbf_em12draw_struct *data;
int	*error;
{
	char	*function_name = "mbr_em12draw_rd_svp";
	int	status = MB_SUCCESS;
	char	line[EM_SVP_SIZE+3];
	short int *short_ptr;
	short int *short_ptr2;
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
	status = fread(line,1,EM_SVP_SIZE+3,mbfp);
	if (status == EM_SVP_SIZE+3)
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
		mb_get_int(&(data->svp_day),          line,      2);
		mb_get_int(&(data->svp_month),        line+2,    2);
		mb_get_int(&(data->svp_year),         line+4,    2);
		mb_get_int(&(data->svp_hour),         line+6,    2);
		mb_get_int(&(data->svp_minute),       line+8,    2);
		mb_get_int(&(data->svp_second),       line+10,   2);
		mb_get_int(&(data->svp_centisecond),  line+12,   2);
		short_ptr = (short int *) &line[14];
#ifdef BYTESWAPPED
		data->svp_num = *short_ptr;
#else
		data->svp_num = (short int) mb_swap_short(*short_ptr);
#endif
		for (i=0;i<data->svp_num;i++)
			{
			short_ptr = (short int *) &line[16+4*i];
			short_ptr2 = (short int *) &line[18+4*i];
#ifdef BYTESWAPPED
			data->svp_depth[i] = *short_ptr;
			data->svp_vel[i] = *short_ptr2;
#else
			data->svp_depth[i] = (short int) mb_swap_short(*short_ptr);
			data->svp_vel[i] = (short int) mb_swap_short(*short_ptr2);
#endif
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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->svp_centisecond);
		fprintf(stderr,"dbg5       svp_num:          %d\n",data->svp_num);
		for (i=0;i<data->svp_num;i++)
			fprintf(stderr,"dbg5       depth: %d     vel: %d\n",
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
int mbr_em12draw_rd_bath(verbose,mbfp,data,swath_id,error)
int	verbose;
FILE	*mbfp;
struct mbf_em12draw_struct *data;
int	swath_id;
int	*error;
{
	char	*function_name = "mbr_em12draw_rd_bath";
	int	status = MB_SUCCESS;
	char	line[EM_12DP_BATH_SIZE+3];
	char	beamarray[11];
	short int *short_ptr;
	char	*char_ptr;
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
		fprintf(stderr,"dbg2       swath_id:   %d\n",swath_id);
		}

	/* read record into char array */
	status = fread(line,1,EM_12DP_BATH_SIZE+3,mbfp);
	if (status == EM_12DP_BATH_SIZE+3)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		/* get ascii stuff */
		data->kind = MB_DATA_DATA;
		mb_get_int(&(data->day),              line,      2);
		mb_get_int(&(data->month),            line+2,    2);
		mb_get_int(&(data->year),             line+4,    2);
		mb_get_int(&(data->hour),             line+6,    2);
		mb_get_int(&(data->minute),           line+8,    2);
		mb_get_int(&(data->second),           line+10,   2);
		mb_get_int(&(data->centisecond),      line+12,   2);

		/* set swath id */
		data->swath_id = swath_id;

		/* get binary stuff */
#ifdef BYTESWAPPED
		short_ptr = (short int *) &line[14]; data->ping_number = *short_ptr;
		data->beams_bath = MBF_EM12DRAW_MAXBEAMS;
		char_ptr = &line[16]; data->bath_res = (int) *char_ptr;
		char_ptr = &line[17]; data->bath_quality = (int) *char_ptr;
		short_ptr = (short int *) &line[18]; data->keel_depth = *short_ptr;
		short_ptr = (short int *) &line[20]; data->heading = *short_ptr;
		short_ptr = (short int *) &line[22]; data->roll = *short_ptr;
		short_ptr = (short int *) &line[24]; data->pitch = *short_ptr;
		short_ptr = (short int *) &line[26]; data->ping_heave = *short_ptr;
		short_ptr = (short int *) &line[28]; data->sound_vel = *short_ptr;
		char_ptr = &line[30]; data->bath_mode = (int) *char_ptr;
		for (i=0;i<data->beams_bath;i++)
			{
			for (j=0;j<11;j++)
				beamarray[j] = line[32+11*i+j];
			short_ptr = (short int *) beamarray; 
			data->bath[i] = *short_ptr;
			short_ptr = ((short int *) beamarray) + 1; 
			data->bath_acrosstrack[i] = *short_ptr;
			short_ptr = ((short int *) beamarray) + 2; 
			data->bath_alongtrack[i] = *short_ptr;
			short_ptr = ((short int *) beamarray) + 3; 
			data->tt[i] = *short_ptr;
			char_ptr = beamarray + 8; 
			data->amp[i] = (mb_s_char) *char_ptr;
			char_ptr = beamarray + 9; 
			data->quality[i] = (mb_u_char) *char_ptr;
			char_ptr = beamarray + 10; 
			data->heave[i] = (mb_s_char) *char_ptr;
			}
#else
		short_ptr = (short int *) &line[14]; 
		data->ping_number = (short int) mb_swap_short(*short_ptr);
		data->beams_bath = MBF_EM12DRAW_MAXBEAMS;
		char_ptr = &line[16]; 
		data->bath_res = (int) *char_ptr;
		char_ptr = &line[17]; 
		data->bath_quality = (int) *char_ptr;
		short_ptr = (short int *) &line[18]; 
		data->keel_depth = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[20]; 
		data->heading = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[22]; 
		data->roll = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[24]; 
		data->pitch = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[26]; 
		data->ping_heave = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[28]; 
		data->sound_vel = (short int) mb_swap_short(*short_ptr);
		char_ptr = &line[30]; data->bath_mode = (int) *char_ptr;
		for (i=0;i<data->beams_bath;i++)
			{
			for (j=0;j<11;j++)
				beamarray[j] = line[32+11*i+j];
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
			char_ptr = beamarray + 8; 
			data->amp[i] = (mb_s_char) *char_ptr;
			char_ptr = beamarray + 9; 
			data->quality[i] = (mb_u_char) *char_ptr;
			char_ptr = beamarray + 10; 
			data->heave[i] = (mb_s_char) *char_ptr;
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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",data->ping_number);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",data->beams_bath);
		fprintf(stderr,"dbg5       bath_mode:        %d\n",data->bath_mode);
		fprintf(stderr,"dbg5       bath_res:         %d\n",data->bath_res);
		fprintf(stderr,"dbg5       bath_quality:     %d\n",data->ping_number);
		fprintf(stderr,"dbg5       keel_depth:       %d\n",data->keel_depth);
		fprintf(stderr,"dbg5       heading:          %d\n",data->heading);
		fprintf(stderr,"dbg5       roll:             %d\n",data->roll);
		fprintf(stderr,"dbg5       pitch:            %d\n",data->pitch);
		fprintf(stderr,"dbg5       ping_heave:       %d\n",data->ping_heave);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       beam bath xtrack ltrack tt amp qual heave\n");
		for (i=0;i<data->beams_bath;i++)
			fprintf(stderr,"dbg5       beam:%d  bath:%d  xtrck:%d  ltrck:%d tt:%d  amp:%d  qual:%d  heave:%d\n",
				i,data->bath[i],data->bath_acrosstrack[i],
				data->bath_alongtrack[i],data->tt[i],
				data->amp[i],data->quality[i],data->heave[i]);
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
int mbr_em12draw_rd_ss(verbose,mbfp,data,swath_id,first,more,error)
int	verbose;
FILE	*mbfp;
struct mbf_em12draw_struct *data;
int	swath_id;
int	first;
int	*more;
int	*error;
{
	char	*function_name = "mbr_em12draw_rd_ss";
	int	status = MB_SUCCESS;
	char	line[EM_12DP_SSP_SIZE+3];
	short int *short_ptr;
	char	*char_ptr;
	char	*char_ptr1, *char_ptr2;
	int	num_datagrams;
	int	datagram;
	int	num_beams;
	int	beamlist[MBF_EM12DRAW_MAXBEAMS];
	mb_s_char *beam_ss;
	short int *beam_ssp;
	int	ioffset;
	int	npixelsum;
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
		fprintf(stderr,"dbg2       swath_id:   %d\n",swath_id);
		fprintf(stderr,"dbg2       first:      %d\n",first);
		}

	/* if first call for current ping, initialize */
	if (first == MB_YES)
		{
		data->pixels_ss = 0;
		for (i=0;i<data->beams_bath;i++)
			{
			data->beam_samples[i] = 0;
			data->beam_center_sample[i] = 0;
			data->beam_start_sample[i] = 0;
			}
		}

	/* read first record into char array */
	status = fread(line,1,EM_12DP_SSP_SIZE+3,mbfp);
	if (status == EM_12DP_SSP_SIZE+3)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		/* get ascii stuff */
		data->kind = MB_DATA_DATA;
		mb_get_int(&(data->day),              line,      2);
		mb_get_int(&(data->month),            line+2,    2);
		mb_get_int(&(data->year),             line+4,    2);
		mb_get_int(&(data->hour),             line+6,    2);
		mb_get_int(&(data->minute),           line+8,    2);
		mb_get_int(&(data->second),           line+10,   2);
		mb_get_int(&(data->centisecond),      line+12,   2);

		/* set swath id */
		data->swath_id = swath_id;

		/* get binary stuff */
#ifdef BYTESWAPPED
		short_ptr = (short int *) &line[14]; data->ping_number = *short_ptr;
/*		short_ptr = (short int *) &line[16]; data->sound_vel = *short_ptr;*/
		char_ptr = &line[18]; data->ss_mode = (int) *char_ptr;
		char_ptr = &line[19]; num_datagrams = (int) *char_ptr;
		char_ptr = &line[20]; datagram = (int) *char_ptr;
		char_ptr = &line[21]; num_beams = (int) *char_ptr;
		npixelsum = 0;
		for (i=0;i<num_beams;i++)
			{
			char_ptr = &line[22+6*i]; 
				beamlist[i] = ((int) *char_ptr) - 1;
			char_ptr = &line[23+6*i]; 
				data->beam_frequency[beamlist[i]] = (int) *char_ptr;
			short_ptr = (short int *) &line[24+6*i]; 
				data->beam_samples[beamlist[i]] = *short_ptr;
			short_ptr = (short int *) &line[26+6*i]; 
				data->beam_center_sample[beamlist[i]] = *short_ptr;
			npixelsum += data->beam_samples[beamlist[i]];
			}
			
		/* check for bad numbers of pixels indicating a broken
		    record */
		if (npixelsum > 523)
		    for (i=0;i<num_beams;i++)
			{
			data->beam_samples[beamlist[i]] = 0;
			}
		    
		/* load up the sidescan for each beam */
		ioffset = 22+6*num_beams;
		for (i=0;i<num_beams;i++)
			{
			/* do not ever load more data than we can store */
			if (data->pixels_ss + data->beam_samples[beamlist[i]]
				> MBF_EM12DRAW_MAXPIXELS)
				data->beam_samples[beamlist[i]] = 0;
			
			/* get the sidescan */
			data->beam_start_sample[beamlist[i]] = data->pixels_ss;
			for (j=0;j<data->beam_samples[beamlist[i]];j++)
				{
				data->ss[data->pixels_ss] = (mb_s_char) line[ioffset];
				ioffset++;
				char_ptr1 = (char *) &data->ssp[data->pixels_ss];
				char_ptr2 = char_ptr1 + 1;
				*char_ptr1 = line[ioffset];
				ioffset++;
				*char_ptr2 = line[ioffset];
				ioffset++;
				data->pixels_ss++;
				}
			}
#else
		short_ptr = (short int *) &line[14]; 
		data->ping_number = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[16]; 
/*		data->sound_vel = (short int) mb_swap_short(*short_ptr);*/
		char_ptr = &line[18]; 
		data->ss_mode = (int) *char_ptr;
		char_ptr = &line[19]; 
		num_datagrams = (int) *char_ptr;
		char_ptr = &line[20]; 
		datagram = (int) *char_ptr;
		char_ptr = &line[21]; 
		num_beams = (int) *char_ptr;
		npixelsum = 0;
		for (i=0;i<num_beams;i++)
			{
			char_ptr = &line[22+6*i]; 
			beamlist[i] = ((int) *char_ptr) - 1;
			char_ptr = &line[23+6*i]; 
			data->beam_frequency[beamlist[i]] = 
				(int) *char_ptr;
			short_ptr = (short int *) &line[24+6*i]; 
			data->beam_samples[beamlist[i]] = 
				(short int) mb_swap_short(*short_ptr);
			short_ptr = (short int *) &line[26+6*i]; 
			data->beam_center_sample[beamlist[i]] = 
				(short int) mb_swap_short(*short_ptr);
			npixelsum += data->beam_samples[beamlist[i]];
			}
			
		/* check for bad numbers of pixels indicating a broken
		    record */
		if (npixelsum > 523)
		    for (i=0;i<num_beams;i++)
			{
			data->beam_samples[beamlist[i]] = 0;
			}
		    
		/* load up the sidescan for each beam */
		ioffset = 22+6*num_beams;
		for (i=0;i<num_beams;i++)
			{
			/* do not ever load more data than we can store */
			if (data->pixels_ss + data->beam_samples[beamlist[i]]
				> MBF_EM12DRAW_MAXPIXELS)
				data->beam_samples[beamlist[i]] = 0;
			
			/* get the sidescan */
			data->beam_start_sample[beamlist[i]] = data->pixels_ss;
			for (j=0;j<data->beam_samples[beamlist[i]];j++)
				{
				data->ss[data->pixels_ss] = (mb_s_char) line[ioffset];
				ioffset++;
				char_ptr1 = (char *) &data->ssp[data->pixels_ss];
				char_ptr2 = char_ptr1 + 1;
				*char_ptr2 = line[ioffset];
				ioffset++;
				*char_ptr1 = line[ioffset];
				ioffset++;
				data->pixels_ss++;
				}
			}
#endif
		}

	/* set flag if another sidescan record needs to be read */
	if (status == MB_SUCCESS && datagram < num_datagrams)
		*more = MB_YES;
	else
		*more = MB_NO;

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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",data->ping_number);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       ss_mode:          %d\n",data->ss_mode);
		fprintf(stderr,"dbg5       num_datagrams:    %d\n",num_datagrams);
		fprintf(stderr,"dbg5       datagram:         %d\n",datagram);
		fprintf(stderr,"dbg5       num_beams:        %d\n",num_beams);
		fprintf(stderr,"dbg5       beam frequency samples center\n");
		for (i=0;i<num_beams;i++)
			fprintf(stderr,"dbg5       beam:%d  frequency:%d  samples:%d  center:%d  start:%d\n",
				beamlist[i],data->beam_frequency[beamlist[i]],
				data->beam_samples[beamlist[i]],
				data->beam_center_sample[beamlist[i]],
				data->beam_start_sample[beamlist[i]]);
		for (i=0;i<num_beams;i++)
			{
			beam_ss = &data->ss[data->beam_start_sample[beamlist[i]]];
			beam_ssp = &data->ssp[data->beam_start_sample[beamlist[i]]];
			for (j=0;j<data->beam_samples[beamlist[i]];j++)
				fprintf(stderr,"dbg5       beam:%d pixel:%d  amp:%d phase:%d\n",
					beamlist[i],j,beam_ss[j],beam_ssp[j]);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       more:       %d\n",*more);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em12draw_wr_data(verbose,mbio_ptr,data_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em12draw_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em12draw_struct *data;
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
	data = (struct mbf_em12draw_struct *) data_ptr;
	mbfp = mb_io_ptr->mbfp;

	if (data->kind == MB_DATA_COMMENT)
		{
		status = mbr_em12draw_wr_parameter(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_START)
		{
		status = mbr_em12draw_wr_start(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_STOP)
		{
		status = mbr_em12draw_wr_stop(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_NAV)
		{
		status = mbr_em12draw_wr_pos(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_VELOCITY_PROFILE)
		{
		status = mbr_em12draw_wr_svp(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_DATA)
		{
		status = mbr_em12draw_wr_bath(verbose,mbfp,data,error);
		status = mbr_em12draw_wr_ss(verbose,mbfp,data,error);
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
int mbr_em12draw_wr_start(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em12draw_wr_start";
	int	status = MB_SUCCESS;
	struct mbf_em12draw_struct *data;
	char	line[EM_START_SIZE+3];
	short int label;
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
	data = (struct mbf_em12draw_struct *) data_ptr;

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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->par_centisecond);
		fprintf(stderr,"dbg5       pos_type:         %d\n",data->pos_type);
		fprintf(stderr,"dbg5       pos_delay:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       roll_offset:      %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       pitch_offset:     %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       heading_offset:   %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em100_td:         %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em100_tx:         %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em100_ty:         %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em12_td:          %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em12_tx:          %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em12_ty:          %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em1000_td:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em1000_tx:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em1000_ty:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       survey_line:      %d\n",data->survey_line);
		fprintf(stderr,"dbg5       comment:          %s\n",data->comment);
		}

	/* write the record label */
	label = EM_START;
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
		sprintf(line,"%2.2d%2.2d%2.2d,",
			data->par_day,data->par_month,data->par_year);
		sprintf(line+7,"%2.2d%2.2d%2.2d%2.2d,",
			data->par_hour,data->par_minute,
			data->par_second,data->par_centisecond);
		sprintf(line+16,  "PIS=%1d,",   data->pos_type);
		sprintf(line+22,  "PTD=%5.1f,",   data->pos_delay);
		sprintf(line+32,  "MSR=%5.2f,",   data->roll_offset);
		sprintf(line+42,  "MSP=%5.2f,",   data->pitch_offset);
		sprintf(line+52,  "MSG=%5.2f,",   data->heading_offset);
		sprintf(line+62,  "EM100TD=%5.1f,",   data->em100_td);
		sprintf(line+76,  "EM100TX=%5.1f,",   data->em100_tx);
		sprintf(line+90,  "EM100TY=%5.1f,",   data->em100_ty);
		sprintf(line+104, "EM12TD=%5.1f,",   data->em12_td);
		sprintf(line+117, "EM12TX=%5.1f,",   data->em12_tx);
		sprintf(line+130, "EM12TY=%5.1f,",   data->em12_ty);
		sprintf(line+143, "EM1000TD=%5.1f,",   data->em1000_td);
		sprintf(line+158, "EM1000TX=%5.1f,",   data->em1000_tx);
		sprintf(line+173, "EM1000TY=%5.1f,",   data->em1000_ty);
		for (i=0;i<128;i++)
			line[188+i] = data->spare_parameter[i];
		sprintf(line+316, "SURVEY_LINE_%4.4d,", data->survey_line);
		sprintf(line+333, "COMMENT:");
		for (i=0;i<80;i++)
			line[341+i] = data->comment[i];
		line[EM_START_SIZE] = 0x03;
		line[EM_START_SIZE+1] = '\0';
		line[EM_START_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,EM_START_SIZE+3,mbfp);
		if (status != EM_START_SIZE+3)
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
int mbr_em12draw_wr_stop(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em12draw_wr_stop";
	int	status = MB_SUCCESS;
	struct mbf_em12draw_struct *data;
	char	line[EM_STOP_SIZE+3];
	short int label;
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
	data = (struct mbf_em12draw_struct *) data_ptr;

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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->par_centisecond);
		fprintf(stderr,"dbg5       pos_type:         %d\n",data->pos_type);
		fprintf(stderr,"dbg5       pos_delay:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       roll_offset:      %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       pitch_offset:     %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       heading_offset:   %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em100_td:         %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em100_tx:         %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em100_ty:         %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em12_td:          %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em12_tx:          %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em12_ty:          %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em1000_td:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em1000_tx:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em1000_ty:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       survey_line:      %d\n",data->survey_line);
		fprintf(stderr,"dbg5       comment:          %s\n",data->comment);
		}

	/* write the record label */
	label = EM_STOP;
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
		sprintf(line,"%2.2d%2.2d%2.2d,",
			data->par_day,data->par_month,data->par_year);
		sprintf(line+7,"%2.2d%2.2d%2.2d%2.2d,",
			data->par_hour,data->par_minute,
			data->par_second,data->par_centisecond);
		sprintf(line+16,  "PIS=%1d,",   data->pos_type);
		sprintf(line+22,  "PTD=%5.1f,",   data->pos_delay);
		sprintf(line+32,  "MSR=%5.2f,",   data->roll_offset);
		sprintf(line+42,  "MSP=%5.2f,",   data->pitch_offset);
		sprintf(line+52,  "MSG=%5.2f,",   data->heading_offset);
		sprintf(line+62,  "EM100TD=%5.1f,",   data->em100_td);
		sprintf(line+76,  "EM100TX=%5.1f,",   data->em100_tx);
		sprintf(line+90,  "EM100TY=%5.1f,",   data->em100_ty);
		sprintf(line+104, "EM12TD=%5.1f,",   data->em12_td);
		sprintf(line+117, "EM12TX=%5.1f,",   data->em12_tx);
		sprintf(line+130, "EM12TY=%5.1f,",   data->em12_ty);
		sprintf(line+143, "EM1000TD=%5.1f,",   data->em1000_td);
		sprintf(line+158, "EM1000TX=%5.1f,",   data->em1000_tx);
		sprintf(line+173, "EM1000TY=%5.1f,",   data->em1000_ty);
		for (i=0;i<128;i++)
			line[188+i] = data->spare_parameter[i];
		sprintf(line+316, "SURVEY_LINE_%4.4d,", data->survey_line);
		sprintf(line+333, "COMMENT:");
		for (i=0;i<80;i++)
			line[341+i] = data->comment[i];
		line[EM_STOP_SIZE] = 0x03;
		line[EM_STOP_SIZE+1] = '\0';
		line[EM_STOP_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,EM_STOP_SIZE+3,mbfp);
		if (status != EM_STOP_SIZE+3)
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
int mbr_em12draw_wr_parameter(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em12draw_wr_parameter";
	int	status = MB_SUCCESS;
	struct mbf_em12draw_struct *data;
	char	line[EM_PARAMETER_SIZE+3];
	short int label;
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
	data = (struct mbf_em12draw_struct *) data_ptr;

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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->par_centisecond);
		fprintf(stderr,"dbg5       pos_type:         %d\n",data->pos_type);
		fprintf(stderr,"dbg5       pos_delay:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       roll_offset:      %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       pitch_offset:     %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       heading_offset:   %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em100_td:         %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em100_tx:         %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em100_ty:         %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em12_td:          %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em12_tx:          %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em12_ty:          %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em1000_td:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em1000_tx:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       em1000_ty:        %f\n",data->pos_delay);
		fprintf(stderr,"dbg5       survey_line:      %d\n",data->survey_line);
		fprintf(stderr,"dbg5       comment:          %s\n",data->comment);
		}

	/* write the record label */
	label = EM_PARAMETER;
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
		sprintf(line,"%2.2d%2.2d%2.2d,",
			data->par_day,data->par_month,data->par_year);
		sprintf(line+7,"%2.2d%2.2d%2.2d%2.2d,",
			data->par_hour,data->par_minute,
			data->par_second,data->par_centisecond);
		sprintf(line+16,  "PIS=%1d,",   data->pos_type);
		sprintf(line+22,  "PTD=%5.1f,",   data->pos_delay);
		sprintf(line+32,  "MSR=%5.2f,",   data->roll_offset);
		sprintf(line+42,  "MSP=%5.2f,",   data->pitch_offset);
		sprintf(line+52,  "MSG=%5.2f,",   data->heading_offset);
		sprintf(line+62,  "EM100TD=%5.1f,",   data->em100_td);
		sprintf(line+76,  "EM100TX=%5.1f,",   data->em100_tx);
		sprintf(line+90,  "EM100TY=%5.1f,",   data->em100_ty);
		sprintf(line+104, "EM12TD=%5.1f,",   data->em12_td);
		sprintf(line+117, "EM12TX=%5.1f,",   data->em12_tx);
		sprintf(line+130, "EM12TY=%5.1f,",   data->em12_ty);
		sprintf(line+143, "EM1000TD=%5.1f,",   data->em1000_td);
		sprintf(line+158, "EM1000TX=%5.1f,",   data->em1000_tx);
		sprintf(line+173, "EM1000TY=%5.1f,",   data->em1000_ty);
		for (i=0;i<128;i++)
			line[188+i] = data->spare_parameter[i];
		sprintf(line+316, "SURVEY_LINE_%4.4d,", data->survey_line);
		sprintf(line+333, "COMMENT:");
		for (i=0;i<80;i++)
			line[341+i] = data->comment[i];
		line[EM_PARAMETER_SIZE] = 0x03;
		line[EM_PARAMETER_SIZE+1] = '\0';
		line[EM_PARAMETER_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,EM_PARAMETER_SIZE+3,mbfp);
		if (status != EM_PARAMETER_SIZE+3)
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
int mbr_em12draw_wr_pos(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em12draw_wr_pos";
	int	status = MB_SUCCESS;
	struct mbf_em12draw_struct *data;
	char	line[EM_POS_SIZE+3];
	short int label;
	double	degree_dec;
	int	degree;
	double	minute;
	char	hemisphere;
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
	data = (struct mbf_em12draw_struct *) data_ptr;

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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->pos_centisecond);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->longitude);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->latitude);
		fprintf(stderr,"dbg5       utm_northing:     %f\n",data->utm_northing);
		fprintf(stderr,"dbg5       utm_easting:      %f\n",data->utm_easting);
		fprintf(stderr,"dbg5       utm_zone:         %d\n",data->utm_zone);
		fprintf(stderr,"dbg5       utm_zone_lon:     %f\n",data->utm_zone_lon);
		fprintf(stderr,"dbg5       utm_system:       %d\n",data->utm_system);
		fprintf(stderr,"dbg5       pos_quality:      %d\n",data->pos_quality);
		fprintf(stderr,"dbg5       speed:            %f\n",data->speed);
		fprintf(stderr,"dbg5       line_heading:     %f\n",data->line_heading);
		}

	/* write the record label */
	label = EM_POS;
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
		sprintf(line,"%2.2d%2.2d%2.2d,",
			data->pos_day,data->pos_month,data->pos_year);
		sprintf(line+7,"%2.2d%2.2d%2.2d%2.2d,",
			data->pos_hour,data->pos_minute,
			data->pos_second,data->pos_centisecond);
		if (data->latitude > 0.0)
			{
			hemisphere = 'N';
			degree_dec = data->latitude;
			}
		else
			{
			hemisphere = 'S';
			degree_dec = -data->latitude;
			}
		degree = (int) floor(degree_dec);
		minute = 60.0*(degree_dec - degree);
		sprintf(line+16,"%2.2d%7.4f%c,",degree,minute,hemisphere);
		if (data->longitude > 180.0)
			data->longitude = data->longitude - 360.0;
		if (data->longitude <= -180.0)
			data->longitude = data->longitude + 360.0;
		if (data->longitude > 0.0)
			{
			hemisphere = 'E';
			degree_dec = data->longitude;
			}
		else
			{
			hemisphere = 'W';
			degree_dec = -data->longitude;
			}
		degree = (int) floor(degree_dec);
		minute = 60.0*(degree_dec - degree);
		sprintf(line+27,"%3.3d%7.4f%c,",degree,minute,hemisphere);
		sprintf(line+39,"%11.1f,%9.1f,%2.2d,",
			data->utm_northing,data->utm_easting,data->utm_zone);
		if (data->utm_zone_lon > 180.0)
			data->utm_zone_lon = data->utm_zone_lon - 360.0;
		if (data->utm_zone_lon <= -180.0)
			data->utm_zone_lon = data->utm_zone_lon + 360.0;
		if (data->utm_zone_lon > 0.0)
			{
			hemisphere = 'E';
			degree_dec = data->utm_zone_lon;
			}
		else
			{
			hemisphere = 'W';
			degree_dec = -data->utm_zone_lon;
			}
		degree = (int) floor(degree_dec);
		minute = 60.0*(degree_dec - degree);
		sprintf(line+64,"%3.3d%7.4f%c,",degree,minute,hemisphere);
		sprintf(line+76,"%1.1d,%1.1d,%4.1f,%5.1f",
			data->utm_system,data->pos_quality,
			data->speed,data->line_heading);
		line[EM_POS_SIZE] = 0x03;
		line[EM_POS_SIZE+1] = '\0';
		line[EM_POS_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,EM_POS_SIZE+3,mbfp);
		if (status != EM_POS_SIZE+3)
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
int mbr_em12draw_wr_svp(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em12draw_wr_svp";
	int	status = MB_SUCCESS;
	struct mbf_em12draw_struct *data;
	char	line[EM_SVP_SIZE+3];
	short int label;
	short int *short_ptr;
	short int *short_ptr2;
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
	data = (struct mbf_em12draw_struct *) data_ptr;

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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->svp_centisecond);
		fprintf(stderr,"dbg5       svp_num:          %d\n",data->svp_num);
		for (i=0;i<data->svp_num;i++)
			fprintf(stderr,"dbg5       depth: %d     vel: %d\n",
				data->svp_depth[i],data->svp_vel[i]);
		}

	/* write the record label */
	label = EM_SVP;
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
		sprintf(line,"%2.2d%2.2d%2.2d",
			data->svp_day,data->svp_month,data->svp_year);
		sprintf(line+6,"%2.2d%2.2d%2.2d%2.2d",
			data->svp_hour,data->svp_minute,
			data->svp_second,data->svp_centisecond);
		short_ptr = (short int *) &line[14];
#ifdef BYTESWAPPED
		*short_ptr = data->svp_num;
#else
		*short_ptr = (short int) 
			mb_swap_short((short int)data->svp_num);
#endif
		for (i=0;i<data->svp_num;i++)
			{
			short_ptr = (short int *) &line[16+4*i];
			short_ptr2 = (short int *) &line[18+4*i];
#ifdef BYTESWAPPED
			*short_ptr = (short int) data->svp_depth[i];
			*short_ptr2 = (short int) data->svp_vel[i];
#else
			*short_ptr = (short int) 
				mb_swap_short((short int)data->svp_depth[i]);
			*short_ptr2 = (short int) 
				mb_swap_short((short int)data->svp_vel[i]);
#endif
			}
		for (i=data->svp_num;i<100;i++)
			{
			short_ptr = (short int *) &line[16+4*i];
			short_ptr2 = (short int *) &line[18+4*i];
			*short_ptr = 0;
			*short_ptr2 = 0;
			}
		line[EM_SVP_SIZE] = 0x03;
		line[EM_SVP_SIZE+1] = '\0';
		line[EM_SVP_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,EM_SVP_SIZE+3,mbfp);
		if (status != EM_SVP_SIZE+3)
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
int mbr_em12draw_wr_bath(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em12draw_wr_bath";
	int	status = MB_SUCCESS;
	struct mbf_em12draw_struct *data;
	char	line[EM_12DP_BATH_SIZE+3];
	char	beamarray[11];
	short int label;
	short int *short_ptr;
	char	*char_ptr;
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
	data = (struct mbf_em12draw_struct *) data_ptr;

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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",data->ping_number);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",data->beams_bath);
		fprintf(stderr,"dbg5       bath_mode:        %d\n",data->bath_mode);
		fprintf(stderr,"dbg5       bath_res:         %d\n",data->bath_res);
		fprintf(stderr,"dbg5       bath_quality:     %d\n",data->ping_number);
		fprintf(stderr,"dbg5       keel_depth:       %d\n",data->keel_depth);
		fprintf(stderr,"dbg5       heading:          %d\n",data->heading);
		fprintf(stderr,"dbg5       roll:             %d\n",data->roll);
		fprintf(stderr,"dbg5       pitch:            %d\n",data->pitch);
		fprintf(stderr,"dbg5       ping_heave:       %d\n",data->ping_heave);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       beam bath xtrack ltrack tt amp qual heave\n");
		for (i=0;i<data->beams_bath;i++)
			fprintf(stderr,"dbg5       beam:%d  bath:%d  xtrck:%d  ltrck:%d tt:%d  amp:%d  qual:%d  heave:%d\n",
				i,data->bath[i],data->bath_acrosstrack[i],
				data->bath_alongtrack[i],data->tt[i],
				data->amp[i],data->quality[i],data->heave[i]);
		}

	/* write the record label */
	if (data->swath_id == EM_SWATH_PORT)
		label = EM_12DP_BATH;
	else
		label = EM_12DS_BATH;
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
		sprintf(line,"%2.2d%2.2d%2.2d",
			data->day,data->month,data->year);
		sprintf(line+6,"%2.2d%2.2d%2.2d%2.2d",
			data->hour,data->minute,
			data->second,data->centisecond);
#ifdef BYTESWAPPED
		short_ptr = (short int *) &line[14];
		*short_ptr = (short int) data->ping_number;
		char_ptr = &line[16];
		*char_ptr = (char) data->bath_res;
		char_ptr = &line[17];
		*char_ptr = (char) data->bath_quality;
		short_ptr = (short int *) &line[18];
		*short_ptr = (short int) data->keel_depth;
		short_ptr = (short int *) &line[20];
		*short_ptr = (short int) data->heading;
		short_ptr = (short int *) &line[22];
		*short_ptr = (short int) data->roll;
		short_ptr = (short int *) &line[24];
		*short_ptr = (short int) data->pitch;
		short_ptr = (short int *) &line[26];
		*short_ptr = (short int) data->ping_heave;
		short_ptr = (short int *) &line[28];
		*short_ptr = (short int) data->sound_vel;
		char_ptr = &line[30];
		*char_ptr = (char) data->bath_mode;
		for (i=0;i<MBF_EM12DRAW_MAXBEAMS;i++)
			{
			short_ptr = (short int *) beamarray; 
			*short_ptr = (short int) data->bath[i];
			short_ptr = ((short int *) beamarray) + 1; 
			*short_ptr = (short int) data->bath_acrosstrack[i];
			short_ptr = ((short int *) beamarray) + 2; 
			*short_ptr = (short int) data->bath_alongtrack[i];
			short_ptr = ((short int *) beamarray) + 3; 
			*short_ptr = (short int) data->tt[i];
			char_ptr = beamarray + 8; 
			*char_ptr = (char) data->amp[i];
			char_ptr = beamarray + 9; 
			*char_ptr = (char) data->quality[i];
			char_ptr = beamarray + 10; 
			*char_ptr = (char) data->heave[i];
			for (j=0;j<11;j++)
				line[32+11*i+j] = beamarray[j];
			}
#else
		short_ptr = (short int *) &line[14];
		*short_ptr = (short int) mb_swap_short((short int)data->ping_number);
		char_ptr = &line[16];
		*char_ptr = (char) data->bath_res;
		char_ptr = &line[17];
		*char_ptr = (char) data->bath_quality;
		short_ptr = (short int *) &line[18];
		*short_ptr = (short int) mb_swap_short((short int)data->keel_depth);
		short_ptr = (short int *) &line[20];
		*short_ptr = (short int) mb_swap_short((short int)data->heading);
		short_ptr = (short int *) &line[22];
		*short_ptr = (short int) mb_swap_short((short int)data->roll);
		short_ptr = (short int *) &line[24];
		*short_ptr = (short int) mb_swap_short((short int)data->pitch);
		short_ptr = (short int *) &line[26];
		*short_ptr = (short int) mb_swap_short((short int)data->ping_heave);
		short_ptr = (short int *) &line[28];
		*short_ptr = (short int) mb_swap_short((short int)data->sound_vel);
		char_ptr = &line[30];
		*char_ptr = (char) data->bath_mode;
		for (i=0;i<MBF_EM12DRAW_MAXBEAMS;i++)
			{
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
			char_ptr = beamarray + 8; 
			*char_ptr = (char) data->amp[i];
			char_ptr = beamarray + 9; 
			*char_ptr = (char) data->quality[i];
			char_ptr = beamarray + 10; 
			*char_ptr = (char) data->heave[i];
			for (j=0;j<11;j++)
				line[32+11*i+j] = beamarray[j];
			}
#endif
		line[EM_12DP_BATH_SIZE] = 0x03;
		line[EM_12DP_BATH_SIZE+1] = '\0';
		line[EM_12DP_BATH_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,EM_12DP_BATH_SIZE+3,mbfp);
		if (status != EM_12DP_BATH_SIZE+3)
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
int mbr_em12draw_wr_ss(verbose,mbfp,data_ptr,error)
int	verbose;
FILE	*mbfp;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_em12draw_wr_ss";
	int	status = MB_SUCCESS;
	struct mbf_em12draw_struct *data;
	char	line[EM_12DP_SSP_SIZE+3];
	short int label;
	short int *short_ptr;
	char	*char_ptr;
	char	*char_ptr1, *char_ptr2;
	int	num_datagrams;
	int	datagram;
	int	num_beams;
	int	datagram_start[MBF_EM12DRAW_MAXBEAMS+1];
	int	datagram_end[MBF_EM12DRAW_MAXBEAMS+1];
	int	datagram_size[MBF_EM12DRAW_MAXBEAMS+1];
	int	new_datagram_size;
	mb_s_char *beam_ss;
	short int *beam_ssp;
	int	ioffset;
	int	odatagram, obeam;
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
	data = (struct mbf_em12draw_struct *) data_ptr;

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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",data->ping_number);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       ss_mode:          %d\n",data->ss_mode);
		fprintf(stderr,"dbg5       beam frequency samples center start\n");
		for (i=0;i<MBF_EM12DRAW_MAXBEAMS;i++)
			fprintf(stderr,"dbg5       beam:%d  frequency:%d  samples:%d  center:%d  start:%d\n",
				i,data->beam_frequency[i],
				data->beam_samples[i],
				data->beam_center_sample[i],
				data->beam_start_sample[i]);
		for (i=0;i<MBF_EM12DRAW_MAXBEAMS;i++)
			{
			beam_ss = &data->ss[data->beam_start_sample[i]];
			beam_ssp = &data->ssp[data->beam_start_sample[i]];
			for (j=0;j<data->beam_samples[i];j++)
				fprintf(stderr,"dbg5       beam:%d pixel:%d  amp:%d phase:%d\n",
					i,j,beam_ss[j],beam_ssp[j]);
			}
		}

	/* preprocess data to figure out how many 
		sidescan datagrams to output */
	num_datagrams = 0;
	datagram_size[0] = 22;
	datagram_start[0] = 0;
	datagram_end[0] = 0;
	for (i=0;i<MBF_EM12DRAW_MAXBEAMS;i++)
		{
		new_datagram_size = datagram_size[num_datagrams] + 6 + 3 * data->beam_samples[i];
		if (new_datagram_size > 1465 
			&& i == MBF_EM12DRAW_MAXBEAMS - 1)
			{
			datagram_end[num_datagrams] = i - 1;
			num_datagrams++;
			datagram_start[num_datagrams] = i;
			datagram_end[num_datagrams] = 
				MBF_EM12DRAW_MAXBEAMS - 1;
			datagram_size[num_datagrams] = 
				22 + 6 + 3 * data->beam_samples[i];
			num_datagrams++;
			}
		else if (new_datagram_size > 1465)
			{
			datagram_end[num_datagrams] = i - 1;
			num_datagrams++;
			datagram_start[num_datagrams] = i;
			datagram_end[num_datagrams] = 
				MBF_EM12DRAW_MAXBEAMS - 1;
			datagram_size[num_datagrams] = 
				22 + 6 + 3 * data->beam_samples[i];
			}
		else if (new_datagram_size == 1465)
			{
			datagram_end[num_datagrams] = i;
			datagram_size[num_datagrams] = new_datagram_size;
			num_datagrams++;
			datagram_start[num_datagrams] = i + 1;
			datagram_end[num_datagrams] = 
				MBF_EM12DRAW_MAXBEAMS - 1;
			datagram_size[num_datagrams] = 22;
			}
		else if (i == MBF_EM12DRAW_MAXBEAMS - 1)
			{
			datagram_end[num_datagrams] = i;
			datagram_size[num_datagrams] = new_datagram_size;
			num_datagrams++;
			}
		else
			datagram_size[num_datagrams] = new_datagram_size;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       num_datagrams:    %d\n",num_datagrams);
		for (datagram=0;datagram<num_datagrams;datagram++)
			{
			fprintf(stderr,"\ndbg5       datagram[%d]:  beam %d to beam %d\n",
				datagram,datagram_start[datagram],datagram_end[datagram]);
			for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
				fprintf(stderr,"dbg5       beam:%d  frequency:%d  samples:%d  center:%d\n",
					i,data->beam_frequency[i],
					data->beam_samples[i],
					data->beam_center_sample[i]);
			}
		}

	/* now loop over all of the sidescan datagrams to be written */
	for (datagram=0;datagram<num_datagrams;datagram++)
	{
	num_beams = datagram_end[datagram] 
		- datagram_start[datagram] + 1;

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
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",data->ping_number);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       ss_mode:          %d\n",data->ss_mode);
		fprintf(stderr,"dbg5       num_datagrams:    %d\n",num_datagrams);
		fprintf(stderr,"dbg5       datagram:         %d\n",datagram);
		fprintf(stderr,"dbg5       num_beams:        %d\n",num_beams);
		for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
			fprintf(stderr,"dbg5       beam:%d  frequency:%d  samples:%d  center:%d  start:%d\n",
				i,data->beam_frequency[i],
				data->beam_samples[i],
				data->beam_center_sample[i],
				data->beam_start_sample[i]);
		for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
			{
			beam_ss = &data->ss[data->beam_start_sample[i]];
			beam_ssp = &data->ssp[data->beam_start_sample[i]];
			for (j=0;j<data->beam_samples[i];j++)
				fprintf(stderr,"dbg5       beam:%d pixel:%d  amp:%d phase:%d\n",
					i,j,beam_ss[j],beam_ssp[j]);
			}
		}

	/* write the record label */
	if (data->swath_id == EM_SWATH_PORT)
		label = EM_12DP_SSP;
	else
		label = EM_12DS_SSP;
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
		sprintf(line,"%2.2d%2.2d%2.2d",
			data->day,data->month,data->year);
		sprintf(line+6,"%2.2d%2.2d%2.2d%2.2d",
			data->hour,data->minute,
			data->second,data->centisecond);
#ifdef BYTESWAPPED
		short_ptr = (short int *) &line[14];
		*short_ptr = (short int) data->ping_number;
		short_ptr = (short int *) &line[16];
		*short_ptr = (short int) data->sound_vel;
		char_ptr = &line[18];
		*char_ptr = (char) data->ss_mode;
		char_ptr = &line[19];
		*char_ptr = (char) num_datagrams;
		char_ptr = &line[20];
		odatagram = datagram + 1;
		*char_ptr = (char) odatagram;
		char_ptr = &line[21];
		*char_ptr = (char) num_beams;
		for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
			{
			char_ptr = line + 22 + 6*i;
			obeam = i + 1;
			*char_ptr = (char) obeam;
			char_ptr = line + 23 + 6*i;
			*char_ptr = (char) data->beam_frequency[i];
			short_ptr = ((short int *) line) + 1;
			*short_ptr = (short int) data->beam_samples[i];
			short_ptr = ((short int *) line) + 2;
			*short_ptr = (short int) data->beam_center_sample[i];
			}
		ioffset = 22 + 6*num_beams;
		for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
			{
			beam_ss = &data->ss[data->beam_start_sample[i]];
			beam_ssp = &data->ssp[data->beam_start_sample[i]];
			for (j=0;j<data->beam_samples[i];j++)
				{
				char_ptr = &line[ioffset];
				*char_ptr = (char) beam_ss[j];
				ioffset++;
				char_ptr1 = (char *) &beam_ssp[j];
				char_ptr2 = char_ptr1 + 1;
				char_ptr = &line[ioffset];
				*char_ptr = *char_ptr1;
				ioffset++;
				char_ptr = &line[ioffset];
				*char_ptr = *char_ptr2;
				ioffset++;
				}
			}
		for (i=ioffset;i<EM_12DP_SSP_SIZE;i++)
			line[i] = (char) 0;
#else
		short_ptr = (short int *) &line[14];
		*short_ptr = (short int) 
			mb_swap_short((short int) data->ping_number);
		short_ptr = (short int *) &line[16];
		*short_ptr = (short int) 
			mb_swap_short((short int) data->sound_vel);
		char_ptr = &line[18];
		*char_ptr = (char) data->ss_mode;
		char_ptr = &line[19];
		*char_ptr = (char) num_datagrams;
		char_ptr = &line[20];
		odatagram = datagram + 1;
		*char_ptr = (char) odatagram;
		char_ptr = &line[21];
		*char_ptr = (char) num_beams;
		j = 0;
		for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
			{
			char_ptr = line + 22 + 6*j;
			obeam = i + 1;
			*char_ptr = (char) obeam;
			char_ptr = line + 23 + 6*j;
			*char_ptr = (char) data->beam_frequency[i];
			short_ptr = ((short int *) line) + 12 + 3*j;
			*short_ptr = (short int) 
				mb_swap_short((short int) 
					data->beam_samples[i]);
			short_ptr = ((short int *) line) + 13 + 3*j;
			*short_ptr = (short int) 
				mb_swap_short((short int) 
					data->beam_center_sample[i]);
			j++;
			}
		ioffset = 22 + 6*num_beams;
		for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
			{
			beam_ss = &data->ss[data->beam_start_sample[i]];
			beam_ssp = &data->ssp[data->beam_start_sample[i]];
			for (j=0;j<data->beam_samples[i];j++)
				{
				char_ptr = &line[ioffset];
				*char_ptr = (char) beam_ss[j];
				ioffset++;
				char_ptr1 = (char *) &beam_ssp[j];
				char_ptr2 = char_ptr1 + 1;
				char_ptr = &line[ioffset];
				*char_ptr = *char_ptr2;
				ioffset++;
				char_ptr = &line[ioffset];
				*char_ptr = *char_ptr1;
				ioffset++;
				}
			}
		for (i=ioffset;i<EM_12DP_SSP_SIZE;i++)
			line[i] = (char) 0;
#endif
		line[EM_12DP_SSP_SIZE] = 0x03;
		line[EM_12DP_SSP_SIZE+1] = '\0';
		line[EM_12DP_SSP_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,EM_12DP_SSP_SIZE+3,mbfp);
		if (status != EM_12DP_SSP_SIZE+3)
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

	/* end loop over datagrams */
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
