/*--------------------------------------------------------------------
 *    The MB-system:	mbr_em121raw.c	7/8/96
 *	$Id: mbr_em121raw.c,v 5.0 2000-12-01 22:48:41 caress Exp $
 *
 *    Copyright (c) 1994, 2000 by
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
 * mbr_em121raw.c contains the functions for reading and writing
 * multibeam data in the EM121RAW format.  
 * These functions include:
 *   mbr_alm_em121raw	- allocate read/write memory
 *   mbr_dem_em121raw	- deallocate read/write memory
 *   mbr_rt_em121raw	- read and translate data
 *   mbr_wt_em121raw	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	August 8, 1994
 * $Log: not supported by cvs2svn $
 * Revision 4.12  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.11  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.10  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
 *
 * Revision 4.9  1999/02/04  23:52:54  caress
 * MB-System version 4.6beta7
 *
 * Revision 4.8  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.7  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.6  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.5  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.5  1997/04/17  15:07:36  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.4  1996/09/19  20:22:47  caress
 * Enabled the writing of new position records with mb_put_all
 * calls.
 *
 * Revision 4.3  1996/08/26  19:03:38  caress
 * REALLY changed "signed char" to "char".
 *
 * Revision 4.2  1996/08/26  18:33:50  caress
 * Changed "signed char" to "char" for SunOs 4.1 compiler compatibility.
 *
 * Revision 4.1  1996/08/05  15:21:58  caress
 * Just redid i/o for Simrad sonars, including adding EM12S and EM121 support.
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
#include "../../include/mbf_em121raw.h"

/* include for byte swapping */
#include "../../include/mb_swap.h"

/* essential function prototypes */
int mbr_info_em121raw(int verbose, 
			int *system, 
			int *beams_bath_max, 
			int *beams_amp_max, 
			int *pixels_ss_max, 
			char *format_name, 
			char *system_name, 
			char *format_description, 
			int *numfile, 
			int *filetype, 
			int *variable_beams, 
			int *traveltime, 
			int *beam_flagging, 
			int *nav_source, 
			int *heading_source, 
			int *vru_source, 
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int (**format_alloc)(), 
			int (**format_free)(), 
			int (**store_alloc)(), 
			int (**store_free)(), 
			int (**read_ping)(), 
			int (**write_ping)(), 
			int (**extract)(), 
			int (**insert)(), 
			int (**extract_nav)(), 
			int (**insert_nav)(), 
			int (**altitude)(), 
			int (**insert_altitude)(), 
			int (**ttimes)(), 
			int (**copyrecord)(), 
			int *error);
int mbr_alm_em121raw(int verbose, char *mbio_ptr, int *error);
int mbr_dem_em121raw(int verbose, char *mbio_ptr, int *error);
int mbr_rt_em121raw(int verbose, char *mbio_ptr, char *store_ptr, int *error);
int mbr_wt_em121raw(int verbose, char *mbio_ptr, char *store_ptr, int *error);

/*--------------------------------------------------------------------*/
int mbr_info_em121raw(int verbose, 
			int *system, 
			int *beams_bath_max, 
			int *beams_amp_max, 
			int *pixels_ss_max, 
			char *format_name, 
			char *system_name, 
			char *format_description, 
			int *numfile, 
			int *filetype, 
			int *variable_beams, 
			int *traveltime, 
			int *beam_flagging, 
			int *nav_source, 
			int *heading_source, 
			int *vru_source, 
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int (**format_alloc)(), 
			int (**format_free)(), 
			int (**store_alloc)(), 
			int (**store_free)(), 
			int (**read_ping)(), 
			int (**write_ping)(), 
			int (**extract)(), 
			int (**insert)(), 
			int (**extract_nav)(), 
			int (**insert_nav)(), 
			int (**altitude)(), 
			int (**insert_altitude)(), 
			int (**ttimes)(), 
			int (**copyrecord)(), 
			int *error)
{
	static char res_id[]="$Id: mbr_em121raw.c,v 5.0 2000-12-01 22:48:41 caress Exp $";
	char	*function_name = "mbr_info_em121raw";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_SIMRAD;
	*beams_bath_max = 121;
	*beams_amp_max = 121;
	*pixels_ss_max = 6050;
	strncpy(format_name, "EM12SRAW", MB_NAME_LENGTH);
	strncpy(system_name, "SIMRAD", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_EM12SRAW\nInformal Description: Simrad EM12S vendor format\nAttributes:           Simrad EM12S, bathymetry, amplitude, and sidescan,\n                      81 beams, variable pixels, ascii + binary, Simrad.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_NO;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_NAV;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*beamwidth_xtrack = 1.0;
	*beamwidth_ltrack = 1.0;

	/* set format and system specific function pointers */
	*format_alloc = &mbr_alm_em121raw;
	*format_free = &mbr_dem_em121raw; 
	*store_alloc = &mbsys_simrad_alloc; 
	*store_free = &mbsys_simrad_deall; 
	*read_ping = &mbr_rt_em121raw; 
	*write_ping = &mbr_wt_em121raw; 
	*extract = &mbsys_simrad_extract; 
	*insert = &mbsys_simrad_insert; 
	*extract_nav = &mbsys_simrad_extract_nav; 
	*insert_nav = &mbsys_simrad_insert_nav; 
	*altitude = &mbsys_simrad_altitude; 
	*insert_altitude = NULL;
	*ttimes = &mbsys_simrad_ttimes; 
	*copyrecord = &mbsys_simrad_copy; 

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");	
		fprintf(stderr,"dbg2       system:             %d\n",*system);
		fprintf(stderr,"dbg2       beams_bath_max:     %d\n",*beams_bath_max);
		fprintf(stderr,"dbg2       beams_amp_max:      %d\n",*beams_amp_max);
		fprintf(stderr,"dbg2       pixels_ss_max:      %d\n",*pixels_ss_max);
		fprintf(stderr,"dbg2       format_name:        %s\n",format_name);
		fprintf(stderr,"dbg2       system_name:        %s\n",system_name);
		fprintf(stderr,"dbg2       format_description: %s\n",format_description);
		fprintf(stderr,"dbg2       numfile:            %d\n",*numfile);
		fprintf(stderr,"dbg2       filetype:           %d\n",*filetype);
		fprintf(stderr,"dbg2       variable_beams:     %d\n",*variable_beams);
		fprintf(stderr,"dbg2       traveltime:         %d\n",*traveltime);
		fprintf(stderr,"dbg2       beam_flagging:      %d\n",*beam_flagging);
		fprintf(stderr,"dbg2       nav_source:         %d\n",*nav_source);
		fprintf(stderr,"dbg2       heading_source:     %d\n",*heading_source);
		fprintf(stderr,"dbg2       vru_source:         %d\n",*vru_source);
		fprintf(stderr,"dbg2       heading_source:     %d\n",*heading_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",*beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",*beamwidth_ltrack);
		fprintf(stderr,"dbg2       format_alloc:       %d\n",*format_alloc);
		fprintf(stderr,"dbg2       format_free:        %d\n",*format_free);
		fprintf(stderr,"dbg2       store_alloc:        %d\n",*store_alloc);
		fprintf(stderr,"dbg2       store_free:         %d\n",*store_free);
		fprintf(stderr,"dbg2       read_ping:          %d\n",*read_ping);
		fprintf(stderr,"dbg2       write_ping:         %d\n",*write_ping);
		fprintf(stderr,"dbg2       extract:            %d\n",*extract);
		fprintf(stderr,"dbg2       insert:             %d\n",*insert);
		fprintf(stderr,"dbg2       extract_nav:        %d\n",*extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %d\n",*insert_nav);
		fprintf(stderr,"dbg2       altitude:           %d\n",*altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %d\n",*insert_altitude);
		fprintf(stderr,"dbg2       ttimes:             %d\n",*ttimes);
		fprintf(stderr,"dbg2       copyrecord:         %d\n",*copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}


/*--------------------------------------------------------------------*/
int mbr_alm_em121raw(int verbose, char *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_em121raw.c,v 5.0 2000-12-01 22:48:41 caress Exp $";
	char	*function_name = "mbr_alm_em121raw";
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
	mb_io_ptr->structure_size = sizeof(struct mbf_em121raw_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mbsys_simrad_alloc(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);

	/* initialize everything to zeros */
	mbr_zero_em121raw(verbose,mb_io_ptr->raw_data,error);

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
int mbr_dem_em121raw(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_em121raw";
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
int mbr_zero_em121raw(int verbose, char *data_ptr, int *error)
{
	char	*function_name = "mbr_zero_em121raw";
	int	status = MB_SUCCESS;
	struct mbf_em121raw_struct *data;
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
	data = (struct mbf_em121raw_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		data->kind = MB_DATA_NONE;
		data->sonar = MBSYS_SIMRAD_EM121;

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
		data->ping_number = 0;
		data->beams_bath = MBF_EM121RAW_MAXBEAMS;
		data->bath_mode = 0;
		data->bath_res = 0;
		data->bath_num = 0;
		data->pulse_length = 0;
		data->beam_width = 0;
		data->power_level = 0;
		data->tx_status = 0;
		data->rx_status = 0;
		data->along_res = 0;
		data->across_res = 0;
		data->depth_res = 0;
		data->range_res = 0;
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
		for (i=0;i<MBF_EM121RAW_MAXBEAMS;i++)
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
		for (i=0;i<MBF_EM121RAW_MAXPIXELS;i++)
			{
			data->ss[i] = 0;
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
int mbr_rt_em121raw(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_em121raw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em121raw_struct *data;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	mb_s_char *data_ss, *store_ss;
	double	ss_spacing;
	int	ntime_i[7];
	double	ntime_d;
	int	ptime_i[7];
	double	ptime_d;
	double	plon, plat, pspeed;
	double	dd, dt, dx, dy;
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
	data = (struct mbf_em121raw_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_simrad_struct *) store_ptr;

	/* read next data from file */
	status = mbr_em121raw_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* save fix if nav data */
	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_NAV)
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
			
		/* get nav time */
		mb_fix_y2k(verbose, data->pos_year, 
			    &ntime_i[0]);
		ntime_i[1] = data->pos_month;
		ntime_i[2] = data->pos_day;
		ntime_i[3] = data->pos_hour;
		ntime_i[4] = data->pos_minute;
		ntime_i[5] = data->pos_second;
		ntime_i[6] = 10000 * data->pos_centisecond;
		mb_get_time(verbose, ntime_i, &ntime_d);
		
		/* add latest fix */
		mb_io_ptr->fix_time_d[mb_io_ptr->nfix] 
			= ntime_d;
		mb_io_ptr->fix_lon[mb_io_ptr->nfix] 
			= data->longitude;
		mb_io_ptr->fix_lat[mb_io_ptr->nfix] 
			= data->latitude;
		mb_io_ptr->nfix++;
		}

	/* handle navigation interpolation */
	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_DATA)
		{			
		/* get ping time */
		mb_fix_y2k(verbose, data->year, 
			    &ptime_i[0]);
		ptime_i[1] = data->month;
		ptime_i[2] = data->day;
		ptime_i[3] = data->hour;
		ptime_i[4] = data->minute;
		ptime_i[5] = data->second;
		ptime_i[6] = 10000*data->centisecond;
		mb_get_time(verbose, ptime_i, &ptime_d);

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
                            pspeed = 3.6 * sqrt(dx*dx + dy*dy)/dt; /* km/hr */
			    data->speed = pspeed / 3.6;
			    }
			else
			    {
			    pspeed = 3.6 * data->speed;
			    }
			if (pspeed > 100.0)
			    pspeed = 0.0;

			/* interpolation possible */
			if (ptime_d >= mb_io_ptr->fix_time_d[0]
			    && ptime_d <= mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
			    {
			    ifix = 0;
			    while (ptime_d > mb_io_ptr->fix_time_d[ifix+1])
				ifix++;
			    plon = mb_io_ptr->fix_lon[ifix]
				+ (mb_io_ptr->fix_lon[ifix+1] 
				    - mb_io_ptr->fix_lon[ifix])
				* (ptime_d
				    - mb_io_ptr->fix_time_d[ifix])
				/ (mb_io_ptr->fix_time_d[ifix+1]
				    - mb_io_ptr->fix_time_d[ifix]);
			    plat = mb_io_ptr->fix_lat[ifix]
				+ (mb_io_ptr->fix_lat[ifix+1] 
				    - mb_io_ptr->fix_lat[ifix])
				* (ptime_d
				    - mb_io_ptr->fix_time_d[ifix])
				/ (mb_io_ptr->fix_time_d[ifix+1]
				    - mb_io_ptr->fix_time_d[ifix]);
			    }
			
			/* extrapolate from first fix */
			else if (ptime_d 
				< mb_io_ptr->fix_time_d[0]
				&& pspeed > 0.0)
			    {
			    dd = (ptime_d 
				- mb_io_ptr->fix_time_d[0])
				* pspeed / 3.6;
			    mb_coor_scale(verbose,mb_io_ptr->fix_lat[0],
				&mtodeglon,&mtodeglat);
			    headingx = sin(DTR*(data->line_heading));
			    headingy = cos(DTR*(data->line_heading));
			    plon = mb_io_ptr->fix_lon[0] 
				+ headingx*mtodeglon*dd;
			    plat = mb_io_ptr->fix_lat[0] 
				+ headingy*mtodeglat*dd;
			    }
			
			/* extrapolate from last fix */
			else if (ptime_d 
				> mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1]
				&& pspeed > 0.0)
			    {
			    dd = (ptime_d 
				- mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
				* pspeed / 3.6;
			    mb_coor_scale(verbose,mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
				&mtodeglon,&mtodeglat);
			    headingx = sin(DTR*(data->line_heading));
			    headingy = cos(DTR*(data->line_heading));
			    plon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1] 
				+ headingx*mtodeglon*dd;
			    plat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1] 
				+ headingy*mtodeglat*dd;
			    }
			
			/* use last fix */
			else
			    {
			    plon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1];
			    plat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1];
			    }
			}
			
		/* else extrapolate from only fix */
		else if (mb_io_ptr->nfix == 1
			&& data->speed > 0.0)
			{
			pspeed = 3.6 * data->speed;
			dd = (ptime_d - mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
				* pspeed / 3.6;
			mb_coor_scale(verbose,mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
				&mtodeglon,&mtodeglat);
			headingx = sin(DTR*(data->line_heading));
			headingy = cos(DTR*(data->line_heading));
			plon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1] 
				+ headingx*mtodeglon*dd;
			plat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1] 
				+ headingy*mtodeglat*dd;
			}

		/* else just take last position */
		else if (mb_io_ptr->nfix == 1)
			{
			plon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1];
			plat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1];
			pspeed = 0.0;
			}
		else
			{
			plon = 0.0;
			plat = 0.0;
			pspeed = 0.0;
			}
		if (mb_io_ptr->lonflip < 0)
			{
			if (plon > 0.) 
				plon = plon - 360.;
			else if (plon < -360.)
				plon = plon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (plon > 180.) 
				plon = plon - 360.;
			else if (plon < -180.)
				plon = plon + 360.;
			}
		else
			{
			if (plon > 360.) 
				plon = plon - 360.;
			else if (plon < 0.)
				plon = plon + 360.;
			}

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"dbg4       Interpolated Navigation:\n",
				plon);
			fprintf(stderr,"dbg4       longitude:  %f\n",
				plon);
			fprintf(stderr,"dbg4       latitude:   %f\n",
				plat);
			fprintf(stderr,"dbg4       speed:      %f\n",
				pspeed);
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
		store->pos_latitude = data->latitude;
		store->pos_longitude = data->longitude;
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
			ping->longitude = plon;
			ping->latitude = plat;
			ping->ping_number = data->ping_number;
			ping->beams_bath = data->beams_bath;
			ping->bath_mode = data->bath_mode;
			ping->bath_res = data->bath_res;
			ping->bath_quality = data->bath_quality;		
			ping->bath_num = data->bath_num;
			ping->pulse_length = data->pulse_length;
			ping->beam_width = data->beam_width;
			ping->power_level = data->power_level;
			ping->tx_status = data->tx_status;
			ping->rx_status = data->rx_status;
			ping->along_res = data->along_res;
			ping->across_res = data->across_res;
			ping->depth_res = data->depth_res;
			ping->range_res = data->range_res;
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
					for (j=0;j<ping->beam_samples[i];j++)
						store_ss[j] = data_ss[j];
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
int mbr_wt_em121raw(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_em121raw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em121raw_struct *data;
	char	*data_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	double	scalefactor;
	int	time_j[5];
	double	depthscale, dacrscale, daloscale, ttscale, reflscale;
	int	iss;
	mb_s_char *data_ss;
	mb_s_char *store_ss;
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
	data = (struct mbf_em121raw_struct *) mb_io_ptr->raw_data;
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
		data->latitude = store->pos_latitude;
		data->longitude = store->pos_longitude;
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
			data->ping_number = ping->ping_number;
			data->beams_bath = ping->beams_bath;
			data->bath_mode = ping->bath_mode;
			data->bath_res = ping->bath_res;
			data->bath_quality = ping->bath_quality;
			data->bath_num = ping->bath_num;
			data->pulse_length = ping->pulse_length;
			data->beam_width = ping->beam_width;
			data->power_level = ping->power_level;
			data->tx_status = ping->tx_status;
			data->rx_status = ping->rx_status;
			data->along_res = ping->along_res;
			data->across_res = ping->across_res;
			data->depth_res = ping->depth_res;
			data->range_res = ping->range_res;
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
					for (j=0;j<data->beam_samples[i];j++)
						data_ss[j] = store_ss[j];
					}
				}
			}
		}

	/* write next data to file */
	status = mbr_em121raw_wr_data(verbose,mbio_ptr,data_ptr,error);

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
int mbr_em121raw_rd_data(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_em121raw_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em121raw_struct *data;
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
	data = (struct mbf_em121raw_struct *) mb_io_ptr->raw_data;
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

		/*fprintf(stderr,"\nstart of mbr_em121raw_rd_data loop:\n");
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
			&& *type != EM_121_BATH
			&& *type != EM_12S_SS)
			{
	/*fprintf(stderr,"call nothing, try again\n");*/
			done = MB_NO;
			}
		else if (*type == EM_START)
			{
	/*fprintf(stderr,"call mbr_em121raw_rd_start type %x\n",*type);*/
			status = mbr_em121raw_rd_start(
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
	/*fprintf(stderr,"call mbr_em121raw_rd_stop type %x\n",*type);*/
			status = mbr_em121raw_rd_stop(
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
	/*fprintf(stderr,"call mbr_em121raw_rd_parameter type %x\n",*type);*/
			status = mbr_em121raw_rd_parameter(
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
	/*fprintf(stderr,"call mbr_em121raw_rd_pos type %x\n",*type);*/
			status = mbr_em121raw_rd_pos(
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
	/*fprintf(stderr,"call mbr_em121raw_rd_svp type %x\n",*type);*/
			status = mbr_em121raw_rd_svp(
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
		else if (*type == EM_121_BATH 
			&& expect != EM_NONE 
			&& expect != EM_121_BATH)
			{
	/*fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);*/
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_121_BATH)
			{
	/*fprintf(stderr,"call mbr_em121raw_rd_bath type %x\n",*type);*/
			status = mbr_em121raw_rd_bath(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				data->kind = MB_DATA_DATA;
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_121_BATH;
					expect = EM_12S_SS;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				}
			}
		else if (*type == EM_12S_SS 
			&& expect != EM_NONE 
			&& expect != EM_12S_SS)
			{
	/*fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);*/
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_12S_SS)
			{
	/*fprintf(stderr,"call mbr_em121raw_rd_ss type %x\n",*type);*/
			status = mbr_em121raw_rd_ss(
				verbose,mbfp,data,first_ss,&more_ss,error);
			if (status == MB_SUCCESS 
				&& more_ss == MB_NO)
				{
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12S_SS;
					expect = EM_121_BATH;
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
				expect = EM_12S_SS;
				first_ss = MB_NO;
				}
			else if (status == MB_FAILURE)
				{
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12S_SS;
					expect = EM_121_BATH;
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

		/*fprintf(stderr,"end of mbr_em121raw_rd_data loop:\n");
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
int mbr_em121raw_rd_start(int verbose, FILE *mbfp, 
		struct mbf_em121raw_struct *data, int *error)
{
	char	*function_name = "mbr_em121raw_rd_start";
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
int mbr_em121raw_rd_stop(int verbose, FILE *mbfp, 
		struct mbf_em121raw_struct *data, int *error)
{
	char	*function_name = "mbr_em121raw_rd_stop";
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
int mbr_em121raw_rd_parameter(int verbose, FILE *mbfp, 
		struct mbf_em121raw_struct *data, int *error)
{
	char	*function_name = "mbr_em121raw_rd_parameter";
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
int mbr_em121raw_rd_pos(int verbose, FILE *mbfp, 
		struct mbf_em121raw_struct *data, int *error)
{
	char	*function_name = "mbr_em121raw_rd_pos";
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
int mbr_em121raw_rd_svp(int verbose, FILE *mbfp, 
		struct mbf_em121raw_struct *data, int *error)
{
	char	*function_name = "mbr_em121raw_rd_svp";
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
int mbr_em121raw_rd_bath(int verbose, FILE *mbfp, 
		struct mbf_em121raw_struct *data, int *error)
{
	char	*function_name = "mbr_em121raw_rd_bath";
	int	status = MB_SUCCESS;
	char	line[EM_121_BATH_SIZE+3];
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
		}

	/* read record into char array */
	status = fread(line,1,EM_121_BATH_SIZE+3,mbfp);
	if (status == EM_121_BATH_SIZE+3)
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

		/* get binary stuff */
#ifdef BYTESWAPPED
		short_ptr = (short int *) &line[14]; data->ping_number = *short_ptr;
		char_ptr = &line[16]; data->bath_mode = (int) *char_ptr;
		data->bath_res = 0;
		char_ptr = &line[17]; data->bath_quality = (int) *char_ptr;
		char_ptr = &line[18]; data->bath_num = (int) *char_ptr;
		data->beams_bath = data->bath_num;
		char_ptr = &line[19]; data->pulse_length = (int) *char_ptr;
		char_ptr = &line[20]; data->beam_width = (int) *char_ptr;
		char_ptr = &line[21]; data->power_level = (int) *char_ptr;
		char_ptr = &line[22]; data->tx_status = (int) *char_ptr;
		char_ptr = &line[23]; data->rx_status = (int) *char_ptr;
		short_ptr = (short int *) &line[24]; data->keel_depth = *short_ptr;
		short_ptr = (short int *) &line[26]; data->heading = (unsigned short int) *short_ptr;
		short_ptr = (short int *) &line[28]; data->roll = *short_ptr;
		short_ptr = (short int *) &line[30]; data->pitch = *short_ptr;
		short_ptr = (short int *) &line[32]; data->ping_heave = *short_ptr;
		short_ptr = (short int *) &line[34]; data->sound_vel = *short_ptr;
		char_ptr = &line[36]; data->along_res = (int) *char_ptr;
		char_ptr = &line[37]; data->across_res = (int) *char_ptr;
		char_ptr = &line[38]; data->depth_res = (int) *char_ptr;
		char_ptr = &line[39]; data->range_res = (int) *char_ptr;
		for (i=0;i<data->beams_bath;i++)
			{
			for (j=0;j<11;j++)
				beamarray[j] = line[44+11*i+j];
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
		char_ptr = &line[16]; data->bath_mode = (int) *char_ptr;
		data->bath_res = 0;
		char_ptr = &line[17]; data->bath_quality = (int) *char_ptr;
		char_ptr = &line[18]; data->bath_num = (int) *char_ptr;
		data->beams_bath = data->bath_num;
		char_ptr = &line[19]; data->pulse_length = (int) *char_ptr;
		char_ptr = &line[20]; data->beam_width = (int) *char_ptr;
		char_ptr = &line[21]; data->power_level = (int) *char_ptr;
		char_ptr = &line[22]; data->tx_status = (int) *char_ptr;
		char_ptr = &line[23]; data->rx_status = (int) *char_ptr;
		short_ptr = (short int *) &line[24]; 
		data->keel_depth = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[26]; 
		data->heading = (unsigned short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[28]; 
		data->roll = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[30]; 
		data->pitch = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[32]; 
		data->ping_heave = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[34]; 
		data->sound_vel = (short int) mb_swap_short(*short_ptr);
		char_ptr = &line[36]; data->along_res = (int) *char_ptr;
		char_ptr = &line[37]; data->across_res = (int) *char_ptr;
		char_ptr = &line[38]; data->depth_res = (int) *char_ptr;
		char_ptr = &line[39]; data->range_res = (int) *char_ptr;
		for (i=0;i<data->beams_bath;i++)
			{
			for (j=0;j<11;j++)
				beamarray[j] = line[44+11*i+j];
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
		fprintf(stderr,"dbg5       bath_quality:     %d\n",data->bath_quality);
		fprintf(stderr,"dbg5       bath_num:         %d\n",data->bath_num);
		fprintf(stderr,"dbg5       pulse_length:     %d\n",data->pulse_length);
		fprintf(stderr,"dbg5       beam_width:       %d\n",data->beam_width);
		fprintf(stderr,"dbg5       power_level:      %d\n",data->power_level);
		fprintf(stderr,"dbg5       tx_status:        %d\n",data->tx_status);
		fprintf(stderr,"dbg5       rx_status:        %d\n",data->rx_status);
		fprintf(stderr,"dbg5       along_res:        %d\n",data->along_res);
		fprintf(stderr,"dbg5       across_res:       %d\n",data->across_res);
		fprintf(stderr,"dbg5       depth_res:        %d\n",data->depth_res);
		fprintf(stderr,"dbg5       range_res:        %d\n",data->range_res);
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
int mbr_em121raw_rd_ss(int verbose, FILE *mbfp, 
		struct mbf_em121raw_struct *data, 
		int first, int *more, int *error)
{
	char	*function_name = "mbr_em121raw_rd_ss";
	int	status = MB_SUCCESS;
	char	line[EM_12S_SS_SIZE+3];
	short int *short_ptr;
	char	*char_ptr;
	int	num_datagrams;
	int	datagram;
	int	num_beams;
	int	beamlist[MBF_EM121RAW_MAXBEAMS];
	mb_s_char *beam_ss;
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
	status = fread(line,1,EM_12S_SS_SIZE+3,mbfp);
	if (status == EM_12S_SS_SIZE+3)
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

		/* get binary stuff */
#ifdef BYTESWAPPED
		short_ptr = (short int *) &line[14]; data->ping_number = *short_ptr;
		short_ptr = (short int *) &line[16]; data->sound_vel = *short_ptr;
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
				> MBF_EM121RAW_MAXPIXELS)
				data->beam_samples[beamlist[i]] = 0;
			
			/* get the sidescan */
			data->beam_start_sample[beamlist[i]] = data->pixels_ss;
			for (j=0;j<data->beam_samples[beamlist[i]];j++)
				{
				data->ss[data->pixels_ss] = (mb_s_char) line[ioffset];
				data->pixels_ss++;
				ioffset++;
				}
			}
#else
		short_ptr = (short int *) &line[14]; 
		data->ping_number = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[16]; 
		data->sound_vel = (short int) mb_swap_short(*short_ptr);
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
				> MBF_EM121RAW_MAXPIXELS)
				data->beam_samples[beamlist[i]] = 0;
			
			/* get the sidescan */
			data->beam_start_sample[beamlist[i]] = data->pixels_ss;
			for (j=0;j<data->beam_samples[beamlist[i]];j++)
				{
				data->ss[data->pixels_ss] = (mb_s_char) line[ioffset];
				data->pixels_ss++;
				ioffset++;
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
			for (j=0;j<data->beam_samples[beamlist[i]];j++)
				fprintf(stderr,"dbg5       beam:%d pixel:%d  amp:%d\n",
					beamlist[i],j,beam_ss[j]);
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
int mbr_em121raw_wr_data(int verbose, char *mbio_ptr, char *data_ptr, int *error)
{
	char	*function_name = "mbr_em121raw_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em121raw_struct *data;
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
	data = (struct mbf_em121raw_struct *) data_ptr;
	mbfp = mb_io_ptr->mbfp;

	if (data->kind == MB_DATA_COMMENT)
		{
		status = mbr_em121raw_wr_parameter(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_START)
		{
		status = mbr_em121raw_wr_start(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_STOP)
		{
		status = mbr_em121raw_wr_stop(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_NAV)
		{
		status = mbr_em121raw_wr_pos(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_VELOCITY_PROFILE)
		{
		status = mbr_em121raw_wr_svp(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_DATA)
		{
		status = mbr_em121raw_wr_ss(verbose,mbfp,data,error);
		status = mbr_em121raw_wr_bath(verbose,mbfp,data,error);
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
int mbr_em121raw_wr_start(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_em121raw_wr_start";
	int	status = MB_SUCCESS;
	struct mbf_em121raw_struct *data;
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
	data = (struct mbf_em121raw_struct *) data_ptr;

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
int mbr_em121raw_wr_stop(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_em121raw_wr_stop";
	int	status = MB_SUCCESS;
	struct mbf_em121raw_struct *data;
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
	data = (struct mbf_em121raw_struct *) data_ptr;

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
int mbr_em121raw_wr_parameter(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_em121raw_wr_parameter";
	int	status = MB_SUCCESS;
	struct mbf_em121raw_struct *data;
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
	data = (struct mbf_em121raw_struct *) data_ptr;

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
int mbr_em121raw_wr_pos(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_em121raw_wr_pos";
	int	status = MB_SUCCESS;
	struct mbf_em121raw_struct *data;
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
	data = (struct mbf_em121raw_struct *) data_ptr;

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
int mbr_em121raw_wr_svp(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_em121raw_wr_svp";
	int	status = MB_SUCCESS;
	struct mbf_em121raw_struct *data;
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
	data = (struct mbf_em121raw_struct *) data_ptr;

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
int mbr_em121raw_wr_bath(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_em121raw_wr_bath";
	int	status = MB_SUCCESS;
	struct mbf_em121raw_struct *data;
	char	line[EM_121_BATH_SIZE+3];
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
	data = (struct mbf_em121raw_struct *) data_ptr;

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
		fprintf(stderr,"dbg5       bath_quality:     %d\n",data->bath_quality);
		fprintf(stderr,"dbg5       bath_num:         %d\n",data->bath_num);
		fprintf(stderr,"dbg5       pulse_length:     %d\n",data->pulse_length);
		fprintf(stderr,"dbg5       beam_width:       %d\n",data->beam_width);
		fprintf(stderr,"dbg5       power_level:      %d\n",data->power_level);
		fprintf(stderr,"dbg5       tx_status:        %d\n",data->tx_status);
		fprintf(stderr,"dbg5       rx_status:        %d\n",data->rx_status);
		fprintf(stderr,"dbg5       along_res:        %d\n",data->along_res);
		fprintf(stderr,"dbg5       across_res:       %d\n",data->across_res);
		fprintf(stderr,"dbg5       depth_res:        %d\n",data->depth_res);
		fprintf(stderr,"dbg5       range_res:        %d\n",data->range_res);
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
	label = EM_121_BATH;
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
		*char_ptr = (char) data->bath_mode;
		char_ptr = &line[17];
		*char_ptr = (char) data->bath_quality;
		char_ptr = &line[18];
		*char_ptr = (char) data->bath_num;
		char_ptr = &line[19];
		*char_ptr = (char) data->pulse_length;
		char_ptr = &line[20];
		*char_ptr = (char) data->beam_width;
		char_ptr = &line[21];
		*char_ptr = (char) data->power_level;
		char_ptr = &line[22];
		*char_ptr = (char) data->tx_status;
		char_ptr = &line[23];
		*char_ptr = (char) data->rx_status;
		short_ptr = (short int *) &line[24];
		*short_ptr = (short int) data->keel_depth;
		short_ptr = (short int *) &line[26];
		*short_ptr = (unsigned short int) data->heading;
		short_ptr = (short int *) &line[28];
		*short_ptr = (short int) data->roll;
		short_ptr = (short int *) &line[30];
		*short_ptr = (short int) data->pitch;
		short_ptr = (short int *) &line[32];
		*short_ptr = (short int) data->ping_heave;
		short_ptr = (short int *) &line[34];
		*short_ptr = (short int) data->sound_vel;
		char_ptr = &line[36];
		*char_ptr = (char) data->along_res;
		char_ptr = &line[37];
		*char_ptr = (char) data->across_res;
		char_ptr = &line[38];
		*char_ptr = (char) data->depth_res;
		char_ptr = &line[39];
		*char_ptr = (char) data->range_res;
		for (i=0;i<MBF_EM121RAW_MAXBEAMS;i++)
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
				line[44+11*i+j] = beamarray[j];
			}
#else
		short_ptr = (short int *) &line[14];
		*short_ptr = (short int) mb_swap_short((short int)data->ping_number);
		char_ptr = &line[16];
		*char_ptr = (char) data->bath_mode;
		char_ptr = &line[17];
		*char_ptr = (char) data->bath_quality;
		char_ptr = &line[18];
		*char_ptr = (char) data->bath_num;
		char_ptr = &line[19];
		*char_ptr = (char) data->pulse_length;
		char_ptr = &line[20];
		*char_ptr = (char) data->beam_width;
		char_ptr = &line[21];
		*char_ptr = (char) data->power_level;
		char_ptr = &line[22];
		*char_ptr = (char) data->tx_status;
		char_ptr = &line[23];
		*char_ptr = (char) data->rx_status;
		
		short_ptr = (short int *) &line[24];
		*short_ptr = (short int) mb_swap_short((short int)data->keel_depth);
		short_ptr = (short int *) &line[26];
		*short_ptr = (unsigned short int) mb_swap_short((unsigned short int)data->heading);
		short_ptr = (short int *) &line[28];
		*short_ptr = (short int) mb_swap_short((short int)data->roll);
		short_ptr = (short int *) &line[30];
		*short_ptr = (short int) mb_swap_short((short int)data->pitch);
		short_ptr = (short int *) &line[32];
		*short_ptr = (short int) mb_swap_short((short int)data->ping_heave);
		short_ptr = (short int *) &line[34];
		*short_ptr = (short int) mb_swap_short((short int)data->sound_vel);
		char_ptr = &line[36];
		*char_ptr = (char) data->along_res;
		char_ptr = &line[37];
		*char_ptr = (char) data->across_res;
		char_ptr = &line[38];
		*char_ptr = (char) data->depth_res;
		char_ptr = &line[39];
		*char_ptr = (char) data->range_res;
		for (i=0;i<MBF_EM121RAW_MAXBEAMS;i++)
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
				line[44+11*i+j] = beamarray[j];
			}
#endif
		line[EM_121_BATH_SIZE] = 0x03;
		line[EM_121_BATH_SIZE+1] = '\0';
		line[EM_121_BATH_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,EM_121_BATH_SIZE+3,mbfp);
		if (status != EM_121_BATH_SIZE+3)
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
int mbr_em121raw_wr_ss(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_em121raw_wr_ss";
	int	status = MB_SUCCESS;
	struct mbf_em121raw_struct *data;
	char	line[EM_12S_SS_SIZE+3];
	short int label;
	short int *short_ptr;
	char	*char_ptr;
	int	num_datagrams;
	int	datagram;
	int	num_beams;
	int	datagram_start[MBF_EM121RAW_MAXBEAMS+1];
	int	datagram_end[MBF_EM121RAW_MAXBEAMS+1];
	int	datagram_size[MBF_EM121RAW_MAXBEAMS+1];
	int	new_datagram_size;
	mb_s_char *beam_ss;
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
	data = (struct mbf_em121raw_struct *) data_ptr;

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
		for (i=0;i<MBF_EM121RAW_MAXBEAMS;i++)
			fprintf(stderr,"dbg5       beam:%d  frequency:%d  samples:%d  center:%d  start:%d\n",
				i,data->beam_frequency[i],
				data->beam_samples[i],
				data->beam_center_sample[i],
				data->beam_start_sample[i]);
		for (i=0;i<MBF_EM121RAW_MAXBEAMS;i++)
			{
			beam_ss = &data->ss[data->beam_start_sample[i]];
			for (j=0;j<data->beam_samples[i];j++)
				fprintf(stderr,"dbg5       beam:%d pixel:%d  amp:%d\n",
					i,j,beam_ss[j]);
			}
		}

	/* preprocess data to figure out how many 
		sidescan datagrams to output */
	num_datagrams = 0;
	datagram_size[0] = 22;
	datagram_start[0] = 0;
	datagram_end[0] = 0;
	for (i=0;i<MBF_EM121RAW_MAXBEAMS;i++)
		{
		new_datagram_size = datagram_size[num_datagrams] + 6 + data->beam_samples[i];
		if (new_datagram_size > 551 
			&& i == MBF_EM121RAW_MAXBEAMS - 1)
			{
			datagram_end[num_datagrams] = i - 1;
			num_datagrams++;
			datagram_start[num_datagrams] = i;
			datagram_end[num_datagrams] = 
				MBF_EM121RAW_MAXBEAMS - 1;
			datagram_size[num_datagrams] = 
				22 + 6 + data->beam_samples[i];
			num_datagrams++;
			}
		else if (new_datagram_size > 551)
			{
			datagram_end[num_datagrams] = i - 1;
			num_datagrams++;
			datagram_start[num_datagrams] = i;
			datagram_end[num_datagrams] = 
				MBF_EM121RAW_MAXBEAMS - 1;
			datagram_size[num_datagrams] = 
				22 + 6 + data->beam_samples[i];
			}
		else if (new_datagram_size == 551)
			{
			datagram_end[num_datagrams] = i;
			datagram_size[num_datagrams] = new_datagram_size;
			num_datagrams++;
			datagram_start[num_datagrams] = i + 1;
			datagram_end[num_datagrams] = 
				MBF_EM121RAW_MAXBEAMS - 1;
			datagram_size[num_datagrams] = 22;
			}
		else if (i == MBF_EM121RAW_MAXBEAMS - 1)
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
			for (j=0;j<data->beam_samples[i];j++)
				fprintf(stderr,"dbg5       beam:%d pixel:%d  amp:%d\n",
					i,j,beam_ss[j]);
			}
		}

	/* write the record label */
	label = EM_12S_SS;
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
			for (j=0;j<data->beam_samples[i];j++)
				{
				char_ptr = &line[ioffset+j];
				*char_ptr = (char) beam_ss[j];
				}
			ioffset = ioffset + data->beam_samples[i];
			}
		for (i=ioffset;i<EM_12S_SS_SIZE;i++)
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
			for (j=0;j<data->beam_samples[i];j++)
				{
				char_ptr = line + ioffset + j;
				*char_ptr = (char) beam_ss[j];
				}
			ioffset = ioffset + data->beam_samples[i];
			}
		for (i=ioffset;i<EM_12S_SS_SIZE;i++)
			line[i] = (char) 0;
#endif
		line[EM_12S_SS_SIZE] = 0x03;
		line[EM_12S_SS_SIZE+1] = '\0';
		line[EM_12S_SS_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,EM_12S_SS_SIZE+3,mbfp);
		if (status != EM_12S_SS_SIZE+3)
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
