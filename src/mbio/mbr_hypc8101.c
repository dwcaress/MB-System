/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hypc8101.c	8/8/94
 *	$Id: mbr_hypc8101.c,v 5.5 2002-09-18 23:32:59 caress Exp $
 *
 *    Copyright (c) 1998, 2000, 2002 by
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
 * mbr_hypc8101.c contains the functions for reading and writing
 * multibeam data in the HYPC8101 format.  
 * These functions include:
 *   mbr_alm_hypc8101	- allocate read/write memory
 *   mbr_dem_hypc8101	- deallocate read/write memory
 *   mbr_rt_hypc8101	- read and translate data
 *   mbr_wt_hypc8101	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	December 10, 1998
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.4  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.3  2001/06/08  21:44:01  caress
 * Version 5.0.beta01
 *
 * Revision 5.2  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.4  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.3  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.2  1999/08/16  23:14:41  caress
 * Added ability to handle Mesotech SM2000 data
 *
 * Revision 4.1  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
 *
 * Revision 4.0  1999/01/01  23:38:01  caress
 * MB-System version 4.6beta6
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
#include "../../include/mbsys_reson.h"
#include "../../include/mbf_hypc8101.h"

/* essential function prototypes */
int mbr_register_hypc8101(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_hypc8101(int verbose, 
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
			int *error);
int mbr_alm_hypc8101(int verbose, void *mbio_ptr, int *error);
int mbr_dem_hypc8101(int verbose, void *mbio_ptr, int *error);
int mbr_rt_hypc8101(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_hypc8101(int verbose, void *mbio_ptr, void *store_ptr, int *error);

/*--------------------------------------------------------------------*/
int mbr_register_hypc8101(int verbose, void *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_hypc8101.c,v 5.5 2002-09-18 23:32:59 caress Exp $";
	char	*function_name = "mbr_register_hypc8101";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set format info parameters */
	status = mbr_info_hypc8101(verbose, 
			&mb_io_ptr->system, 
			&mb_io_ptr->beams_bath_max, 
			&mb_io_ptr->beams_amp_max, 
			&mb_io_ptr->pixels_ss_max, 
			mb_io_ptr->format_name, 
			mb_io_ptr->system_name, 
			mb_io_ptr->format_description, 
			&mb_io_ptr->numfile, 
			&mb_io_ptr->filetype, 
			&mb_io_ptr->variable_beams, 
			&mb_io_ptr->traveltime, 
			&mb_io_ptr->beam_flagging, 
			&mb_io_ptr->nav_source, 
			&mb_io_ptr->heading_source, 
			&mb_io_ptr->vru_source, 
			&mb_io_ptr->beamwidth_xtrack, 
			&mb_io_ptr->beamwidth_ltrack, 
			error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_hypc8101;
	mb_io_ptr->mb_io_format_free = &mbr_dem_hypc8101; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_reson_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_reson_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_hypc8101; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_hypc8101; 
	mb_io_ptr->mb_io_extract = &mbsys_reson_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_reson_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_reson_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_reson_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_reson_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = &mbsys_reson_extract_svp; 
	mb_io_ptr->mb_io_insert_svp = &mbsys_reson_insert_svp; 
	mb_io_ptr->mb_io_ttimes = &mbsys_reson_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_reson_copy; 
	mb_io_ptr->mb_io_extract_rawss = NULL; 
	mb_io_ptr->mb_io_insert_rawss = NULL; 

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");	
		fprintf(stderr,"dbg2       system:             %d\n",mb_io_ptr->system);
		fprintf(stderr,"dbg2       beams_bath_max:     %d\n",mb_io_ptr->beams_bath_max);
		fprintf(stderr,"dbg2       beams_amp_max:      %d\n",mb_io_ptr->beams_amp_max);
		fprintf(stderr,"dbg2       pixels_ss_max:      %d\n",mb_io_ptr->pixels_ss_max);
		fprintf(stderr,"dbg2       format_name:        %s\n",mb_io_ptr->format_name);
		fprintf(stderr,"dbg2       system_name:        %s\n",mb_io_ptr->system_name);
		fprintf(stderr,"dbg2       format_description: %s\n",mb_io_ptr->format_description);
		fprintf(stderr,"dbg2       numfile:            %d\n",mb_io_ptr->numfile);
		fprintf(stderr,"dbg2       filetype:           %d\n",mb_io_ptr->filetype);
		fprintf(stderr,"dbg2       variable_beams:     %d\n",mb_io_ptr->variable_beams);
		fprintf(stderr,"dbg2       traveltime:         %d\n",mb_io_ptr->traveltime);
		fprintf(stderr,"dbg2       beam_flagging:      %d\n",mb_io_ptr->beam_flagging);
		fprintf(stderr,"dbg2       nav_source:         %d\n",mb_io_ptr->nav_source);
		fprintf(stderr,"dbg2       heading_source:     %d\n",mb_io_ptr->heading_source);
		fprintf(stderr,"dbg2       vru_source:         %d\n",mb_io_ptr->vru_source);
		fprintf(stderr,"dbg2       heading_source:     %d\n",mb_io_ptr->heading_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",mb_io_ptr->beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",mb_io_ptr->beamwidth_ltrack);
		fprintf(stderr,"dbg2       format_alloc:       %d\n",mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %d\n",mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %d\n",mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %d\n",mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %d\n",mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %d\n",mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %d\n",mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %d\n",mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %d\n",mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %d\n",mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %d\n",mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %d\n",mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %d\n",mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %d\n",mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %d\n",mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       extract_rawss:      %d\n",mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr,"dbg2       insert_rawss:       %d\n",mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr,"dbg2       copyrecord:         %d\n",mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_hypc8101(int verbose, 
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
			int *error)
{
	static char res_id[]="$Id: mbr_hypc8101.c,v 5.5 2002-09-18 23:32:59 caress Exp $";
	char	*function_name = "mbr_info_hypc8101";
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
	*system = MB_SYS_RESON;
	*beams_bath_max = 101;
	*beams_amp_max = 101;
	*pixels_ss_max = 0;
	strncpy(format_name, "HYPC8101", MB_NAME_LENGTH);
	strncpy(system_name, "RESON", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_HYPC8101\nInformal Description: Reson SeaBat 8101 shallow water multibeam\nAttributes:           101 beam bathymetry,\n                      ASCII, read-only, Coastal Oceanographics.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_NAV;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*beamwidth_xtrack = 2.0;
	*beamwidth_ltrack = 2.0;

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
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_alm_hypc8101(int verbose, void *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_hypc8101.c,v 5.5 2002-09-18 23:32:59 caress Exp $";
	char	*function_name = "mbr_alm_hypc8101";
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
	mb_io_ptr->structure_size = sizeof(struct mbf_hypc8101_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_reson_struct),
				&mb_io_ptr->store_data,error);

	/* initialize everything to zeros */
	mbr_zero_hypc8101(verbose,mb_io_ptr->raw_data,error);

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
int mbr_dem_hypc8101(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_hypc8101";
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
int mbr_zero_hypc8101(int verbose, char *data_ptr, int *error)
{
	char	*function_name = "mbr_zero_hypc8101";
	int	status = MB_SUCCESS;
	struct mbf_hypc8101_struct *data;
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
	data = (struct mbf_hypc8101_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		data->kind = MB_DATA_NONE;
		data->sonar = MBSYS_RESON_UNKNOWN;
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
		for (i=0;i<MBF_HYPC8101_COMMENT_LENGTH;i++)
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
		data->longitude = 0;
		data->latitude = 0;
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
		data->beams_bath = 0;
		for (i=0;i<MBF_HYPC8101_MAXBEAMS;i++)
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
int mbr_rt_hypc8101(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_hypc8101";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hypc8101_struct *data;
	struct mbsys_reson_struct *store;
	int	time_i[7];
	double	time_d;
	double	lon, lat, heading, speed;
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
	data = (struct mbf_hypc8101_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_reson_struct *) store_ptr;

	/* read next data from file */
	status = mbr_hypc8101_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;
	
	/* add nav records to list for interpolation */
	if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_NAV)
		{
		mb_fix_y2k(verbose, data->pos_year,&time_i[0]);
		time_i[1] = data->pos_month;
		time_i[2] = data->pos_day;
		time_i[3] = data->pos_hour;
		time_i[4] = data->pos_minute;
		time_i[5] = data->pos_second;
		time_i[6] = 10000*data->pos_hundredth_sec
			+ 100*data->pos_thousandth_sec;
		mb_get_time(verbose,time_i, &time_d);
		lon = data->pos_longitude*0.00000009;
		lat = data->pos_latitude*0.00000009;
		mb_navint_add(verbose, mbio_ptr, time_d, lon, lat, error);
		}

	/* interpolate navigation for survey pings if needed */
	if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_DATA
		&& data->longitude == 0 
		&& data->latitude == 0
		&& mb_io_ptr->nfix >= 1)
		{
		mb_fix_y2k(verbose, data->year,&time_i[0]);
		time_i[1] = data->month;
		time_i[2] = data->day;
		time_i[3] = data->hour;
		time_i[4] = data->minute;
		time_i[5] = data->second;
		time_i[6] = 10000*data->hundredth_sec
			+ 100*data->thousandth_sec;
		mb_get_time(verbose,time_i, &time_d);
		heading = 0.01 * data->heading;
		mb_navint_interp(verbose, mbio_ptr, time_d, heading, 0.0, 
				    &lon, &lat, &speed, error);
		data->longitude = (int) (lon / 0.00000009);
		data->latitude = (int) (lat / 0.00000009);
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
		store->longitude = data->longitude;
		store->latitude = data->latitude;
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
int mbr_wt_hypc8101(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_hypc8101";
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
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
		
	/* set error as this is a read only format */
	status = MB_FAILURE;
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
int mbr_hypc8101_rd_data(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_hypc8101_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hypc8101_struct *data;
	char	*result;
	char	line[MBF_HYPC8101_MAXLINE];
	int	done;
	int	nscan;
	int	ndevice, idummy;
	double	raw_clock;
	double	raw_lat, raw_lon;
	double	hcp_clock, hcp_heave, hcp_roll, hcp_pitch;
	double	gyr_clock, gyr_gyro;
	double	pos_clock, pos_easting, pos_northing;
	double	time_d;
	int	time_i[7];
	double	angle0, angle_inc;
	int	device_type, device_nav, device_hcp;
	int	device_gyro, device_sb2;
	char	device_name[32];
	double	sb2_clock, sb2_ssv;
	int	sb2_nvalues;
	int	sb2_nbeams, sb2_nbeams_read;
	int	sb2_nquality, sb2_nquality_read, iquality;
	double	sb2_quality, sb2_range;
	int	syr, smon, sday, shour, smin, ssec;
	double	off1, off2, off3, off4, off5, off6, off7;
	double	heave, roll, pitch, gyro, dgyro;
	double	angle, theta, phi;
	double	lon, lat, factor;
	double	rr, xx, zz;
	double	ddummy1, ddummy2;
	char	sdummy[20];
	char	*token;
	int	i, it;

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
	data = (struct mbf_hypc8101_struct *) mb_io_ptr->raw_data;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
		
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	done = MB_NO;
	while (done == MB_NO)
		{
		/* read the next line */
		result = fgets(line,MBF_HYPC8101_MAXLINE,mb_io_ptr->mbfp);
		if (result == line 
			&& strlen(line) < MBF_HYPC8101_MAXLINE)
		    {
		    status = MB_SUCCESS;
		    *error = MB_ERROR_NO_ERROR;
		    
		    if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Raw line read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4       line: %s\n",line);
			}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    done = MB_YES;
		    }
			
		/* now make sense of the line */
		if (status == MB_SUCCESS)
		    {
		    /* deal with vru data */
		    if (strncmp(line, "HCP", 3) == 0)
			{
			nscan = sscanf(line+6, "%lf %lf %lf %lf", 
					&hcp_clock, 
					&hcp_heave, 
					&hcp_roll, 
					&hcp_pitch);
			if (nscan == 4)
			    {
			    if (data->hcp_num >= MBF_HYPC8101_NHCP_MAX)
				{
				for (i=0;i<MBF_HYPC8101_NHCP_MAX-1;i++)
				    {
				    data->hcp_clock[i] = data->hcp_clock[i+1];
				    data->hcp_heave[i] = data->hcp_heave[i+1];
				    data->hcp_roll[i] = data->hcp_roll[i+1];
				    data->hcp_pitch[i] = data->hcp_pitch[i+1];
				    }
				data->hcp_num = MBF_HYPC8101_NHCP_MAX - 1;
				}
			    data->hcp_clock[data->hcp_num] = hcp_clock;
			    data->hcp_heave[data->hcp_num] = hcp_heave;
			    data->hcp_roll[data->hcp_num] = hcp_roll;
			    data->hcp_pitch[data->hcp_num] = hcp_pitch;
			    data->hcp_num++;
			    
			    /* get time tag */
			    time_d = data->start_time_d + hcp_clock;
			    mb_get_date(verbose, time_d, time_i);
			    mb_unfix_y2k(verbose, time_i[0], &data->year);
			    data->month = time_i[1];
			    data->day = time_i[2];
			    data->hour = time_i[3];
			    data->minute = time_i[4];
			    data->second = time_i[5];
			    data->hundredth_sec = time_i[6]/10000;
			    data->thousandth_sec = 
				(time_i[6] - 10000 * data->hundredth_sec)/100;
				
			    /* get attitude data */
			    data->heave = 1000 * hcp_heave;
			    data->roll = 200 * hcp_roll;
			    data->pitch = 200 * hcp_pitch;
			    
			    /* set done and kind */
			    done = MB_YES;
			    data->kind = MB_DATA_ATTITUDE;

			    /* print debug statements */
			    if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  New data read by MBIO function <%s>\n",
					function_name);
				fprintf(stderr,"dbg4  New attitude values:\n");
				fprintf(stderr,"dbg4       kind:       %d\n",
					data->kind);
				fprintf(stderr,"dbg4       year:       %d\n",
					data->year);
				fprintf(stderr,"dbg4       month:      %d\n",
					data->month);
				fprintf(stderr,"dbg4       day:        %d\n",
					data->day);
				fprintf(stderr,"dbg4       hour:       %d\n",
					data->hour);
				fprintf(stderr,"dbg4       minute:     %d\n",
					data->minute);
				fprintf(stderr,"dbg4       second:     %d\n",
					data->second);
				fprintf(stderr,"dbg4       0.01 sec:   %d\n",
					data->hundredth_sec);
				fprintf(stderr,"dbg4       0.0001 sec: %d\n",
					data->thousandth_sec);
				fprintf(stderr,"dbg4       clock:      %f\n",
					hcp_clock);
				fprintf(stderr,"dbg4       heave:      %f\n",
					hcp_heave);
				fprintf(stderr,"dbg4       roll:       %f\n",
					hcp_roll);
				fprintf(stderr,"dbg4       pitch:      %f\n",
					hcp_pitch);
				fprintf(stderr,"dbg4       hcp_num:    %d\n",
					data->hcp_num);
				fprintf(stderr,"dbg4       cnt clock heave roll pitch\n");
				for (i=0;i<data->hcp_num;i++)
				fprintf(stderr,"dbg4       %d  %f %f %f %f\n",
					i, data->hcp_clock[i], 
					data->hcp_heave[i], 
					data->hcp_roll[i], 
					data->hcp_pitch[i]);
				}
			    }
			}
			
		    /* deal with gyro data */
		    else if (strncmp(line, "GYR", 3) == 0)
			{
			nscan = sscanf(line+6, "%lf %lf", 
					&gyr_clock, 
					&gyr_gyro);
			if (nscan == 2)
			    {
			    if (data->gyr_num >= MBF_HYPC8101_NGYR_MAX)
				{
				for (i=0;i<MBF_HYPC8101_NGYR_MAX-1;i++)
				    {
				    data->gyr_clock[i] = data->gyr_clock[i+1];
				    data->gyr_gyro[i] = data->gyr_gyro[i+1];
				    }
				data->gyr_num = MBF_HYPC8101_NGYR_MAX - 1;
				}
			    data->gyr_clock[data->gyr_num] = gyr_clock;
			    data->gyr_gyro[data->gyr_num] = gyr_gyro;
			    data->gyr_num++;
			    
			    /* get time tag */
			    time_d = data->start_time_d + gyr_clock;
			    mb_get_date(verbose, time_d, time_i);
			    mb_unfix_y2k(verbose, time_i[0], &data->year);
			    data->month = time_i[1];
			    data->day = time_i[2];
			    data->hour = time_i[3];
			    data->minute = time_i[4];
			    data->second = time_i[5];
			    data->hundredth_sec = time_i[6]/10000;
			    data->thousandth_sec = 
				(time_i[6] - 10000 * data->hundredth_sec)/100;
				
			    /* get gyro data */
			    data->heading = 100 * gyr_gyro;
			    
			    /* set done and kind */
			    done = MB_YES;
			    data->kind = MB_DATA_HEADING;

			    /* print debug statements */
			    if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  New data read by MBIO function <%s>\n",
					function_name);
				fprintf(stderr,"dbg4  New heading values:\n");
				fprintf(stderr,"dbg4       kind:       %d\n",
					data->kind);
				fprintf(stderr,"dbg4       year:       %d\n",
					data->year);
				fprintf(stderr,"dbg4       month:      %d\n",
					data->month);
				fprintf(stderr,"dbg4       day:        %d\n",
					data->day);
				fprintf(stderr,"dbg4       hour:       %d\n",
					data->hour);
				fprintf(stderr,"dbg4       minute:     %d\n",
					data->minute);
				fprintf(stderr,"dbg4       second:     %d\n",
					data->second);
				fprintf(stderr,"dbg4       0.01 sec:   %d\n",
					data->hundredth_sec);
				fprintf(stderr,"dbg4       0.0001 sec: %d\n",
					data->thousandth_sec);
				fprintf(stderr,"dbg4       clock:      %f\n",
					gyr_clock);
				fprintf(stderr,"dbg4       heading:    %f\n",
					gyr_gyro);
				fprintf(stderr,"dbg4       gyr_num:    %d\n",
					data->gyr_num);
				fprintf(stderr,"dbg4       cnt clock heading\n");
				for (i=0;i<data->gyr_num;i++)
				fprintf(stderr,"dbg4       %d  %f %f\n",
					i, data->gyr_clock[i], 
					data->gyr_gyro[i]);
				}
			    }
			}
			
		    /* deal with nav easting northing data */
		    else if (strncmp(line, "POS", 3) == 0)
			{
			nscan = sscanf(line+6, "%lf %lf %lf", 
					&pos_clock, 
					&pos_easting, 
					&pos_northing);
			if (nscan == 3)
			    {
			    if (data->pos_num >= MBF_HYPC8101_NPOS_MAX)
				{
				for (i=0;i<MBF_HYPC8101_NPOS_MAX-1;i++)
				    {
				    data->pos_clock[i] = data->pos_clock[i+1];
				    data->pos_easting[i] = data->pos_easting[i+1];
				    data->pos_northing[i] = data->pos_northing[i+1];
				    }
				data->pos_num = MBF_HYPC8101_NPOS_MAX - 1;
				}
			    data->pos_clock[data->pos_num] = pos_clock;
			    data->pos_easting[data->pos_num] = pos_easting;
			    data->pos_northing[data->pos_num] = pos_northing;
			    data->pos_num++;
			    
			    /* get time tag */
			    time_d = data->start_time_d + pos_clock;
			    mb_get_date(verbose, time_d, time_i);
			    mb_unfix_y2k(verbose, time_i[0], &data->year);
			    data->month = time_i[1];
			    data->day = time_i[2];
			    data->hour = time_i[3];
			    data->minute = time_i[4];
			    data->second = time_i[5];
			    data->hundredth_sec = time_i[6]/10000;
			    data->thousandth_sec = 
				(time_i[6] - 10000 * data->hundredth_sec)/100;
				
			    /* get position data */
			    data->utm_northing = 100 * pos_northing;
			    data->utm_easting = 100 * pos_easting;
			    
			    /* set done and kind */
			    done = MB_YES;
			    data->kind = MB_DATA_NAV;

			    /* print debug statements */
			    if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  New data read by MBIO function <%s>\n",
					function_name);
				fprintf(stderr,"dbg4  New position values:\n");
				fprintf(stderr,"dbg4       kind:       %d\n",
					data->kind);
				fprintf(stderr,"dbg4       year:       %d\n",
					data->year);
				fprintf(stderr,"dbg4       month:      %d\n",
					data->month);
				fprintf(stderr,"dbg4       day:        %d\n",
					data->day);
				fprintf(stderr,"dbg4       hour:       %d\n",
					data->hour);
				fprintf(stderr,"dbg4       minute:     %d\n",
					data->minute);
				fprintf(stderr,"dbg4       second:     %d\n",
					data->second);
				fprintf(stderr,"dbg4       0.01 sec:   %d\n",
					data->hundredth_sec);
				fprintf(stderr,"dbg4       0.0001 sec: %d\n",
					data->thousandth_sec);
				fprintf(stderr,"dbg4       clock:      %f\n",
					pos_clock);
				fprintf(stderr,"dbg4       easting:    %f\n",
					pos_easting);
				fprintf(stderr,"dbg4       northing:   %f\n",
					pos_northing);
				fprintf(stderr,"dbg4       pos_num:    %d\n",
					data->pos_num);
				fprintf(stderr,"dbg4       cnt clock easting northing\n");
				for (i=0;i<data->pos_num;i++)
				fprintf(stderr,"dbg4       %d  %f %f %f\n",
					i, data->pos_clock[i], 
					data->pos_easting[i], 
					data->pos_northing[i]);
				}
			    }
			}
			
		    /* deal with nav lon lat data - always followed
		       by projected position data - return MB_DATA_NAV
		       after both RAW and POS lines */
		    else if (strncmp(line, "RAW", 3) == 0)
			{
			nscan = sscanf(line+6, "%lf %d %lf %lf %lf %lf", 
					&raw_clock, 
					&idummy, 
					&raw_lat, 
					&raw_lon, 
					&ddummy1, 
					&ddummy2);
			if (nscan == 6)
			    {
			    raw_lat = 0.0001 * raw_lat;
			    raw_lon = 0.0001 * raw_lon;
			    if (data->raw_num >= MBF_HYPC8101_NRAW_MAX)
				{
				for (i=0;i<MBF_HYPC8101_NRAW_MAX-1;i++)
				    {
				    data->raw_clock[i] = data->raw_clock[i+1];
				    data->raw_lat[i] = data->raw_lat[i+1];
				    data->raw_lon[i] = data->raw_lon[i+1];
				    }
				data->raw_num = MBF_HYPC8101_NRAW_MAX - 1;
				}
			    data->raw_clock[data->raw_num] = raw_clock;
			    data->raw_lat[data->raw_num] = raw_lat;
			    data->raw_lon[data->raw_num] = raw_lon;
			    data->raw_num++;
			    
			    /* get time tag */
			    time_d = data->start_time_d + raw_clock;
			    mb_get_date(verbose, time_d, time_i);
			    mb_unfix_y2k(verbose, time_i[0], &data->pos_year);
			    data->pos_month = time_i[1];
			    data->pos_day = time_i[2];
			    data->pos_hour = time_i[3];
			    data->pos_minute = time_i[4];
			    data->pos_second = time_i[5];
			    data->pos_hundredth_sec = time_i[6]/10000;
			    data->pos_thousandth_sec = 
				(time_i[6] - 10000 * data->pos_hundredth_sec)/100;
			    
			    /* get position */
			    data->pos_latitude = raw_lat / 0.00000009;
			    data->pos_longitude = raw_lon / 0.00000009;

			    /* print debug statements */
			    if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  New data read by MBIO function <%s>\n",
					function_name);
				fprintf(stderr,"dbg4  New navigation values:\n");
				fprintf(stderr,"dbg4       kind:       %d\n",
					data->kind);
				fprintf(stderr,"dbg4       year:       %d\n",
					data->pos_year);
				fprintf(stderr,"dbg4       month:      %d\n",
					data->pos_month);
				fprintf(stderr,"dbg4       day:        %d\n",
					data->pos_day);
				fprintf(stderr,"dbg4       hour:       %d\n",
					data->pos_hour);
				fprintf(stderr,"dbg4       minute:     %d\n",
					data->pos_minute);
				fprintf(stderr,"dbg4       second:     %d\n",
					data->pos_second);
				fprintf(stderr,"dbg4       0.01 sec:   %d\n",
					data->pos_hundredth_sec);
				fprintf(stderr,"dbg4       0.0001 sec: %d\n",
					data->pos_thousandth_sec);
				fprintf(stderr,"dbg4       pos_lon:    %d\n",
					data->pos_longitude);
				fprintf(stderr,"dbg4       pos_lat:    %d\n",
					data->pos_latitude);
				fprintf(stderr,"dbg4       clock:      %f\n",
					raw_clock);
				fprintf(stderr,"dbg4       longitude:   %f\n",
					raw_lon);
				fprintf(stderr,"dbg4       latitude:    %f\n",
					raw_lat);
				fprintf(stderr,"dbg4       raw_num:    %d\n",
					data->raw_num);
				fprintf(stderr,"dbg4       cnt clock lon lat\n");
				for (i=0;i<data->raw_num;i++)
				fprintf(stderr,"dbg4       %d  %f %f %f\n",
					i, data->raw_clock[i], 
					data->raw_lon[i], 
					data->raw_lat[i]);
				}
			    }
			}
			
		    /* deal with multibeam data */
		    else if (strncmp(line, "SB2", 3) == 0)
			{
			/* start strtok and get sb2_clock */
			if (status == MB_SUCCESS
			    && (token = strtok(line+6, " ")) != NULL)
			    nscan = sscanf(token, "%lf",&sb2_clock);
			else
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    }

			/* get sb2_nbeams */
			if (status == MB_SUCCESS
			    && (token = strtok(NULL, " ")) != NULL)
			    nscan = sscanf(token, "%d",&sb2_nvalues);
			else
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    }

			/* get sb2_ssv */
			if (status == MB_SUCCESS
			    && (token = strtok(NULL, " ")) != NULL)
			    nscan = sscanf(token, "%lf",&sb2_ssv);
			else
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    }

			/* copy values and get beam data */
			if (status == MB_SUCCESS)
			    {
			    data->sound_vel = 10 * sb2_ssv;
			    sb2_nbeams_read = 0;
			    sb2_nbeams = floor(0.8 * (sb2_nvalues - 1));
			    sb2_nquality = sb2_nvalues - sb2_nbeams - 1;
			    for (i=0;i<sb2_nbeams;i++)
				{
				if (status == MB_SUCCESS
				    && (token = strtok(NULL, " ")) != NULL)
				    {
				    if ((nscan = sscanf(token, "%lf",&sb2_range)) == 1);
					{
					data->tt[sb2_nbeams_read] = 100 * sb2_range;
					sb2_nbeams_read++;
					}
				    }
				else
				    {
				    status = MB_FAILURE;
				    *error = MB_ERROR_UNINTELLIGIBLE;
				    }
				}
			    }

			/* copy values and get quality data */
			if (status == MB_SUCCESS)
			    {
			    sb2_nquality_read = 0;
			    for (i=0;i<sb2_nquality;i++)
				{
				if (status == MB_SUCCESS
				    && (token = strtok(NULL, " ")) != NULL)
				    {
				    if ((nscan = sscanf(token, "%lf",&sb2_quality)) == 1);
					{
					iquality = sb2_quality;
					data->quality[4 * sb2_nquality_read] 
						= (iquality >> 6) & 3;
					data->quality[4 * sb2_nquality_read + 1] 
						= (iquality >> 4) & 3;
					data->quality[4 * sb2_nquality_read + 2] 
						= (iquality >> 2) & 3;
					data->quality[4 * sb2_nquality_read + 3] 
						= iquality & 3;
					sb2_nquality_read++;
					}
				    }
				else
				    {
				    status = MB_FAILURE;
				    *error = MB_ERROR_UNINTELLIGIBLE;
				    }
				}
			    }

			/* calculate the rest of the data */
			if (status == MB_SUCCESS)
			    {
			    /* get roll, pitch, and heave values */
			    if (data->hcp_num > 1)
				{
				it = 0;
				for (i=0;i<data->hcp_num-1;i++)
				    {
				    if (sb2_clock > data->hcp_clock[i])
					it = i;
				    }
				factor = (sb2_clock - data->hcp_clock[it])
					/ (data->hcp_clock[it+1] - data->hcp_clock[it]);
				heave = data->hcp_heave[it]
				    + factor * (data->hcp_heave[it+1] 
						- data->hcp_heave[it]);
				roll = data->hcp_roll[it]
				    + factor * (data->hcp_roll[it+1] 
						- data->hcp_roll[it]);
				pitch = data->hcp_pitch[it]
				    + factor * (data->hcp_pitch[it+1] 
						- data->hcp_pitch[it]);
				}
			    else if (data->hcp_num == 1)
				{
				heave = data->hcp_heave[0];
				roll = data->hcp_roll[0];
				pitch = data->hcp_pitch[0];
				}
			    else
				{
				heave = 0.0;
				roll = 0.0;
				pitch = 0.0;
				}
			    data->heave = 1000 * heave;
			    data->roll = 200 * roll;
			    data->pitch = 200 * pitch;

			    /* get gyro value */
			    if (data->gyr_num > 1)
				{
				it = 0;
				for (i=0;i<data->gyr_num-1;i++)
				    {
				    if (sb2_clock > data->gyr_clock[i])
					it = i;
				    }
				dgyro = data->gyr_gyro[it+1] 
						- data->gyr_gyro[it];
				if (dgyro > 180.0)
				    dgyro = dgyro - 360.0;
				else if (dgyro < -180.0)
				    dgyro = dgyro + 360.0;
				factor = (sb2_clock - data->gyr_clock[it])
					/ (data->gyr_clock[it+1] 
					    - data->gyr_clock[it]);
				gyro = data->gyr_gyro[it]
					    + factor * dgyro;
				}
			    else if (data->gyr_num == 1)
				{
				gyro = data->gyr_gyro[0];
				}
			    else
				{
				gyro = 0.0;
				}
			    if (gyro >= 360.0)
				gyro -= 360.0;
			    else if (gyro < 0.0)
				gyro += 360.0;
			    data->heading = 100 * gyro;

			    /* get longitude and latitude values */
			    if (data->raw_num > 1)
				{
				it = 0;
				for (i=0;i<data->raw_num-1;i++)
				    {
				    if (sb2_clock > data->raw_clock[i])
					it = i;
				    }
				factor = (sb2_clock - data->raw_clock[it])
					/ (data->raw_clock[it+1] - data->raw_clock[it]);
				lon = data->raw_lon[it]
				    + factor * (data->raw_lon[it+1] 
						- data->raw_lon[it]);
				lat = data->raw_lat[it]
				    + factor * (data->raw_lat[it+1] 
						- data->raw_lat[it]);
				}
			    else if (data->raw_num == 1)
				{
				lon = data->raw_lon[0];
				lat = data->raw_lat[0];
				}
			    else
				{
				lon = 0.0;
				lat = 0.0;
				}
			    data->latitude = lat / 0.00000009;
			    data->longitude = lon / 0.00000009;
			    
			    /* calculate bathymetry */
			    for (i=0;i<data->beams_bath;i++)
				{
				angle = data->angle0 
					+ data->angle_inc * (i - 0) - roll;
				data->angle[i] = 200 * angle;
				angle = 90.0 - angle;
				mb_rollpitch_to_takeoff(
					verbose, 
					pitch, angle, 
					&theta, &phi, 
					error);
				rr = 0.0000005 * data->sound_vel * data->tt[i];
				xx = rr * sin(DTR * theta);
				zz = rr * cos(DTR * theta);
				data->bath_acrosstrack[i] 
					= 100 * xx * cos(DTR * phi);
				data->bath_alongtrack[i] 
					= 100 * xx * sin(DTR * phi);
				data->bath[i] = 100 * (zz + heave) 
						+ data->transducer_depth;
/*fprintf(stderr, "i:%d tt:%d angle:%f roll:%f pitch:%f heave:%f\n", 
i, data->tt[i], angle, roll, pitch, heave);
fprintf(stderr, "theta:%f phi:%f\n", theta, phi);
fprintf(stderr, "rr:%f xx:%f zz:%f\n", rr, xx, zz);
fprintf(stderr, "bath: %d %d %d\n\n", 
data->bath[i], data->bath_acrosstrack[i], data->bath_alongtrack[i]);*/

				/* deal with Mesotech SM2000 quality values */
				if (data->sonar == MBSYS_RESON_MESOTECHSM2000)
				    {
				    if (data->quality[i] != 0)
					data->quality[i] = 3;
				    }
				}
			    
			    /* get time tag */
			    time_d = data->start_time_d + sb2_clock;
			    mb_get_date(verbose, time_d, time_i);
			    mb_unfix_y2k(verbose, time_i[0], &data->year);
			    data->month = time_i[1];
			    data->day = time_i[2];
			    data->hour = time_i[3];
			    data->minute = time_i[4];
			    data->second = time_i[5];
			    data->hundredth_sec = time_i[6]/10000;
			    data->thousandth_sec = 
				(time_i[6] - 10000 * data->hundredth_sec)/100;

			    /* set kind and done */
			    done = MB_YES;
			    data->kind = MB_DATA_DATA;

			    /* print debug statements */
			    if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  New data read by MBIO function <%s>\n",
					function_name);
				fprintf(stderr,"dbg4  New ping values:\n");
				fprintf(stderr,"dbg4       kind:       %d\n",
					data->kind);
				fprintf(stderr,"dbg4       year:       %d\n",
					data->year);
				fprintf(stderr,"dbg4       month:      %d\n",
					data->month);
				fprintf(stderr,"dbg4       day:        %d\n",
					data->day);
				fprintf(stderr,"dbg4       hour:       %d\n",
					data->hour);
				fprintf(stderr,"dbg4       minute:     %d\n",
					data->minute);
				fprintf(stderr,"dbg4       second:     %d\n",
					data->second);
				fprintf(stderr,"dbg4       0.01 sec:   %d\n",
					data->hundredth_sec);
				fprintf(stderr,"dbg4       0.0001 sec: %d\n",
					data->thousandth_sec);
				fprintf(stderr,"dbg4       longitude:  %d\n",
					data->longitude);
				fprintf(stderr,"dbg4       latitude:   %d\n",
					data->latitude);
				fprintf(stderr,"dbg4       roll:       %d\n",
					data->roll);
				fprintf(stderr,"dbg4       pitch:      %d\n",
					data->pitch);
				fprintf(stderr,"dbg4       heading:    %d\n",
					data->heading);
				fprintf(stderr,"dbg4       heave:      %d\n",
					data->heave);
				fprintf(stderr,"dbg4       beams_bath: %d\n",
					data->beams_bath);
				fprintf(stderr,"dbg4       cnt bath xtrk ltrk tt ang qual\n");
				for (i=0;i<data->beams_bath;i++)
				fprintf(stderr,"dbg4       %3d  %6d %6d %6d %6d %6d %d\n",
					i, data->bath[i], 
					data->bath_acrosstrack[i], 
					data->bath_alongtrack[i], 
					data->tt[i], 
					data->angle[i], 
					data->quality[i]);
				}
			    }
			}
			
		    /* deal with time data */
		    else if (strncmp(line, "TND", 3) == 0)
			{
			nscan = sscanf(line+4, "%d:%d:%d %d/%d/%d", 
					&shour, &smin, &ssec, 
					&smon, &sday, &syr);
			if (nscan == 6)
			    {
			    mb_fix_y2k(verbose, syr, &time_i[0]);
			    time_i[1] = smon;
			    time_i[2] = sday;
			    time_i[3] = shour;
			    time_i[4] = smin;
			    time_i[5] = ssec;
			    time_i[6] = 0;
			    mb_get_time(verbose, time_i, 
					&data->start_time_d);
			    data->par_year = syr;
			    data->par_month = time_i[1];
			    data->par_day = time_i[2];
			    data->par_hour = time_i[3];
			    data->par_minute = time_i[4];
			    data->par_second = time_i[5];
			    data->par_hundredth_sec = 0;
			    data->par_thousandth_sec = 0;
			    }
			}
			
		    /* deal with device data */
		    else if (strncmp(line, "DEV", 3) == 0)
			{
			nscan = sscanf(line+4, "%d %d %s", 
					&ndevice, &device_type, device_name);
			if (nscan == 3)
			    {
			    if (device_type == 4)
				device_nav = ndevice;
			    else if (device_type == 32)
				device_gyro = ndevice;
			    else if (device_type == 512)
				device_hcp = ndevice;
			    else if (device_type == 32784)
				device_sb2 = ndevice;
			    }
			}
			
		    /* deal with device offset data */
		    else if (strncmp(line, "OFF", 3) == 0)
			{
			nscan = sscanf(line+4, "%d %lf %lf %lf %lf %lf %lf %lf", 
					&ndevice, &off1, &off2, &off3, 
					&off4, &off5, &off6, &off7);
			if (nscan == 8)
			    {
			    if (ndevice == device_nav)
				{
				data->antenna_x = 100 * off1;
				data->antenna_y = 100 * off2;
				data->antenna_z = 100 * off3;
				data->time_delay = 1000 * off7;
				}
			    else if (ndevice == device_hcp)
				{
				data->motion_sensor_x = 100 * off1;
				data->motion_sensor_y = 100 * off2;
				data->motion_sensor_z = 100 * off3;
				}
			    else if (ndevice == device_sb2)
				{
				data->motion_sensor_x = 100 * off1;
				data->motion_sensor_y = 100 * off2;
				data->motion_sensor_z = 100 * off3;
				}
			    }
			}
			
		    /* deal with private device data */
		    else if (strncmp(line, "PRD", 3) == 0)
			{
			nscan = sscanf(line+4, "%d %s %lf %lf %d", 
					&ndevice, device_name, 
					&angle0, &angle_inc, 
					&sb2_nbeams);
			if (nscan == 5)
			    {
			    if (ndevice == device_sb2)
				{
				data->beams_bath = sb2_nbeams;
				data->angle0 = angle0;
				data->angle_inc = angle_inc;
				if (strcmp(device_name, "SEA8101") == 0)
				    data->sonar = MBSYS_RESON_SEABAT8101;
				else if (strcmp(device_name, "SM2000") == 0)
				    data->sonar = MBSYS_RESON_MESOTECHSM2000;
				}
			    }
			}
			
		    /* deal with end of header */
		    else if (strncmp(line, "EOH", 3) == 0)
			{
			/* set done and kind */
			done = MB_YES;
			data->kind = MB_DATA_PARAMETER;

			/* print debug statements */
			if (verbose >= 4)
			    {
			    fprintf(stderr,"\ndbg4  New data read by MBIO function <%s>\n",
				    function_name);
			    fprintf(stderr,"dbg4  New parameter values:\n");
			    fprintf(stderr,"dbg4       kind:       %d\n",
				    data->kind);
			    fprintf(stderr,"dbg4       year:       %d\n",
				    data->par_year);
			    fprintf(stderr,"dbg4       month:      %d\n",
				    data->par_month);
			    fprintf(stderr,"dbg4       day:        %d\n",
				    data->par_day);
			    fprintf(stderr,"dbg4       hour:       %d\n",
				    data->par_hour);
			    fprintf(stderr,"dbg4       minute:     %d\n",
				    data->par_minute);
			    fprintf(stderr,"dbg4       second:     %d\n",
				    data->par_second);
			    fprintf(stderr,"dbg4       0.01 sec:   %d\n",
				    data->par_hundredth_sec);
			    fprintf(stderr,"dbg4       0.0001 sec: %d\n",
				    data->par_thousandth_sec);
			    fprintf(stderr,"dbg4       start_time_d:      %f\n",
				    data->start_time_d);
			    fprintf(stderr,"dbg4       angle0:            %f\n",
				    data->angle0);
			    fprintf(stderr,"dbg4       angle_inc:         %f\n",
				    data->angle_inc);
			    fprintf(stderr,"dbg4       beams_bath:        %d\n",
				    data->beams_bath);
			    fprintf(stderr,"dbg4       roll_offset:       %d\n",
				    data->roll_offset);
			    fprintf(stderr,"dbg4       pitch_offset:      %d\n",
				    data->pitch_offset);
			    fprintf(stderr,"dbg4       heading_offset:    %d\n",
				    data->heading_offset);
			    fprintf(stderr,"dbg4       time_delay:        %d\n",
				    data->time_delay);
			    fprintf(stderr,"dbg4       transducer_depth:  %d\n",
				    data->transducer_depth);
			    fprintf(stderr,"dbg4       transducer_height: %d\n",
				    data->transducer_height);
			    fprintf(stderr,"dbg4       transducer_x:      %d\n",
				    data->transducer_x);
			    fprintf(stderr,"dbg4       transducer_y:      %d\n",
				    data->transducer_y);
			    fprintf(stderr,"dbg4       antenna_x:         %d\n",
				    data->antenna_x);
			    fprintf(stderr,"dbg4       antenna_y:         %d\n",
				    data->antenna_y);
			    fprintf(stderr,"dbg4       antenna_z:         %d\n",
				    data->antenna_z);
			    fprintf(stderr,"dbg4       motion_sensor_x:   %d\n",
				    data->motion_sensor_x);
			    fprintf(stderr,"dbg4       motion_sensor_y:   %d\n",
				    data->motion_sensor_y);
			    fprintf(stderr,"dbg4       motion_sensor_z:   %d\n",
				    data->motion_sensor_z);
			    }
			}
		    }
		}
		
	/* get file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

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
