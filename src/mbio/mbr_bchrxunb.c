/*--------------------------------------------------------------------
 *    The MB-system:	mbr_bchrxunb.c	8/29/97
 *	$Id: mbr_bchrxunb.c,v 5.8 2005-11-05 00:48:03 caress Exp $
 *
 *    Copyright (c) 1997, 2000, 2002, 2003 by
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
 * mbr_bchrxunb.c contains the functions for reading and writing
 * multibeam data in the BCHRXUNB format.  
 * These functions include:
 *   mbr_alm_bchrxunb	- allocate read/write memory
 *   mbr_dem_bchrxunb	- deallocate read/write memory
 *   mbr_rt_bchrxunb	- read and translate data
 *   mbr_wt_bchrxunb	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	August 29, 1997
 * $Log: not supported by cvs2svn $
 * Revision 5.7  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.6  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.5  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.4  2001/07/20 00:31:11  caress
 * Release 5.0.beta03
 *
 * Revision 5.3  2001/06/08  21:44:01  caress
 * Version 5.0.beta01
 *
 * Revision 5.2  2001/03/22  20:45:56  caress
 * Trying to make 5.0.beta0...
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.4  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.3  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.2  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
 *
 * Revision 4.1  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.0  1997/09/15  19:09:17  caress
 * Real Version 4.5
 *
 * Revision 1.1  1997/09/15  19:06:40  caress
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
#include "../../include/mbsys_elac.h"
#include "../../include/mbf_bchrxunb.h"

/* include for byte swapping */
#include "../../include/mb_swap.h"

/* essential function prototypes */
int mbr_register_bchrxunb(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_bchrxunb(int verbose, 
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
			int *svp_source,
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error);
int mbr_alm_bchrxunb(int verbose, void *mbio_ptr, int *error);
int mbr_dem_bchrxunb(int verbose, void *mbio_ptr, int *error);
int mbr_rt_bchrxunb(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_bchrxunb(int verbose, void *mbio_ptr, void *store_ptr, int *error);

/*--------------------------------------------------------------------*/
int mbr_register_bchrxunb(int verbose, void *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_bchrxunb.c,v 5.8 2005-11-05 00:48:03 caress Exp $";
	char	*function_name = "mbr_register_bchrxunb";
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
	status = mbr_info_bchrxunb(verbose, 
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
			&mb_io_ptr->svp_source, 
			&mb_io_ptr->beamwidth_xtrack, 
			&mb_io_ptr->beamwidth_ltrack, 
			error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_bchrxunb;
	mb_io_ptr->mb_io_format_free = &mbr_dem_bchrxunb; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_elac_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_elac_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_bchrxunb; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_bchrxunb; 
	mb_io_ptr->mb_io_dimensions = &mbsys_elac_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_elac_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_elac_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_elac_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_elac_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_elac_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = &mbsys_elac_extract_svp; 
	mb_io_ptr->mb_io_insert_svp = &mbsys_elac_insert_svp; 
	mb_io_ptr->mb_io_ttimes = &mbsys_elac_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_elac_copy; 
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
		fprintf(stderr,"dbg2       svp_source:         %d\n",mb_io_ptr->svp_source);
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
int mbr_info_bchrxunb(int verbose, 
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
			int *svp_source, 
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error)
{
	static char res_id[]="$Id: mbr_bchrxunb.c,v 5.8 2005-11-05 00:48:03 caress Exp $";
	char	*function_name = "mbr_info_bchrxunb";
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
	*system = MB_SYS_ELAC;
	*beams_bath_max = 56;
	*beams_amp_max = 56;
	*pixels_ss_max = 0;
	strncpy(format_name, "BCHRXUNB", MB_NAME_LENGTH);
	strncpy(system_name, "ELAC", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_BCHRXUNB\nInformal Description: Elac BottomChart shallow water multibeam\nAttributes:           56 beam bathymetry and amplitude,\n                      binary, University of New Brunswick.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_NAV;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
	*beamwidth_xtrack = 3.0;
	*beamwidth_ltrack = 6.0;

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
		fprintf(stderr,"dbg2       svp_source:         %d\n",*svp_source);
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
int mbr_alm_bchrxunb(int verbose, void *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_bchrxunb.c,v 5.8 2005-11-05 00:48:03 caress Exp $";
	char	*function_name = "mbr_alm_bchrxunb";
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
	mb_io_ptr->structure_size = sizeof(struct mbf_bchrxunb_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_elac_struct),
				&mb_io_ptr->store_data,error);

	/* initialize everything to zeros */
	mbr_zero_bchrxunb(verbose,mb_io_ptr->raw_data,error);

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
int mbr_dem_bchrxunb(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_bchrxunb";
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
int mbr_zero_bchrxunb(int verbose, char *data_ptr, int *error)
{
	char	*function_name = "mbr_zero_bchrxunb";
	int	status = MB_SUCCESS;
	struct mbf_bchrxunb_struct *data;
	int	i, j;

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
	data = (struct mbf_bchrxunb_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		data->kind = MB_DATA_NONE;
		data->sonar = MBSYS_ELAC_BOTTOMCHART;
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
		data->transducer_port_height = 0;
		data->transducer_starboard_height = 0;
		data->transducer_port_depth = 0;
		data->transducer_starboard_depth = 0;
		data->transducer_port_x = 0;
		data->transducer_starboard_x = 0;
		data->transducer_port_y = 0;
		data->transducer_starboard_y = 0;
		data->transducer_port_error = 0;
		data->transducer_starboard_error = 0;
		data->antenna_height = 0;
		data->antenna_x = 0;
		data->antenna_y = 0;
		data->vru_height = 0;
		data->vru_x = 0;
		data->vru_y = 0;
		data->heave_offset = 0;
		data->line_number = 0;
		data->start_or_stop = 0;
		data->transducer_serial_number = 0;
		for (i=0;i<MBF_BCHRXUNB_COMMENT_LENGTH;i++)
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

		/* depth telegram */
		data->ping_num = 0;
		data->sound_vel = 0;
		data->mode = 0;
		data->pulse_length = 0;
		data->source_power = 0;
		data->receiver_gain = 0;
		data->profile_num = 0;
		data->beams_bath = 0;
		for (i=0;i<7;i++)
			{
			data->profile[i].year = 0;
			data->profile[i].month = 0;
			data->profile[i].day = 0;
			data->profile[i].hour = 0;
			data->profile[i].minute = 0;
			data->profile[i].second = 0;
			data->profile[i].hundredth_sec = 0;
			data->profile[i].thousandth_sec = 0;
			data->profile[i].longitude = 0;
			data->profile[i].latitude = 0;
			data->profile[i].roll = 0;
			data->profile[i].pitch = 0;
			data->profile[i].heading = 0;
			data->profile[i].heave = 0;
			for (j=0;j<8;j++)
				{
				data->profile[i].bath[j] = 0;
				data->profile[i].bath_acrosstrack[j] = 0;
				data->profile[i].bath_alongtrack[j] = 0;
				data->profile[i].tt[j] = 0;
				data->profile[i].angle[j] = 0;
				data->profile[i].quality[j] = 0;
				data->profile[i].amp[j] = 0;
				}
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
int mbr_rt_bchrxunb(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_bchrxunb";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_bchrxunb_struct *data;
	struct mbsys_elac_struct *store;
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
	data = (struct mbf_bchrxunb_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_elac_struct *) store_ptr;

	/* read next data from file */
	status = mbr_bchrxunb_rd_data(verbose,mbio_ptr,error);

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
		&& data->profile[0].longitude == 0 
		&& data->profile[0].latitude == 0
		&& mb_io_ptr->nfix >= 1)
		{
		mb_fix_y2k(verbose, data->profile[0].year,&time_i[0]);
		time_i[1] = data->profile[0].month;
		time_i[2] = data->profile[0].day;
		time_i[3] = data->profile[0].hour;
		time_i[4] = data->profile[0].minute;
		time_i[5] = data->profile[0].second;
		time_i[6] = 10000*data->profile[0].hundredth_sec
			+ 100*data->profile[0].thousandth_sec;
		mb_get_time(verbose,time_i, &time_d);
		heading = 0.01 * data->profile[0].heading;
		mb_navint_interp(verbose, mbio_ptr, time_d, heading, 0.0, 
				    &lon, &lat, &speed, error);
		data->profile[0].longitude = (int) (lon / 0.00000009);
		data->profile[0].latitude = (int) (lat / 0.00000009);
		}

	/* translate values to elac data storage structure */
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
		store->transducer_port_height = data->transducer_port_height;
		store->transducer_starboard_height 
			= data->transducer_starboard_height;
		store->transducer_port_depth = data->transducer_port_depth;
		store->transducer_starboard_depth 
			= data->transducer_starboard_depth;
		store->transducer_port_x = data->transducer_port_x;
		store->transducer_starboard_x = data->transducer_starboard_x;
		store->transducer_port_y = data->transducer_port_y;
		store->transducer_starboard_y = data->transducer_starboard_y;
		store->transducer_port_error = data->transducer_port_error;
		store->transducer_starboard_error 
			= data->transducer_starboard_error;
		store->antenna_height = data->antenna_height;
		store->antenna_x = data->antenna_x;
		store->antenna_y = data->antenna_y;
		store->vru_height = data->vru_height;
		store->vru_x = data->vru_x;
		store->vru_y = data->vru_y;
		store->heave_offset = data->heave_offset;
		store->line_number = data->line_number;
		store->start_or_stop = data->start_or_stop;
		store->transducer_serial_number 
			= data->transducer_serial_number;
		for (i=0;i<MBF_BCHRXUNB_COMMENT_LENGTH;i++)
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

		/* depth telegram */
		store->ping_num = data->ping_num;
		store->sound_vel = data->sound_vel;
		store->mode = data->mode;
		store->pulse_length = data->pulse_length;
		store->source_power = data->source_power;
		store->receiver_gain = data->receiver_gain;
		store->profile_num = data->profile_num;
		store->beams_bath = data->beams_bath;
		for (i=0;i<7;i++)
			{
			store->profile[i].year = data->profile[i].year;
			store->profile[i].month = data->profile[i].month;
			store->profile[i].day = data->profile[i].day;
			store->profile[i].hour = data->profile[i].hour;
			store->profile[i].minute = data->profile[i].minute;
			store->profile[i].second = data->profile[i].second;
			store->profile[i].hundredth_sec 
				= data->profile[i].hundredth_sec;
			store->profile[i].thousandth_sec 
				= data->profile[i].thousandth_sec;
			store->profile[i].longitude = data->profile[i].longitude;
			store->profile[i].latitude = data->profile[i].latitude;
			store->profile[i].roll = data->profile[i].roll;
			store->profile[i].pitch = data->profile[i].pitch;
			store->profile[i].heading = data->profile[i].heading;
			store->profile[i].heave = data->profile[i].heave;
			for (j=0;j<8;j++)
				{
				store->profile[i].bath[j] 
					= data->profile[i].bath[j];
				store->profile[i].bath_acrosstrack[j] 
					= data->profile[i].bath_acrosstrack[j];
				store->profile[i].bath_alongtrack[j] 
					= data->profile[i].bath_alongtrack[j];
				store->profile[i].tt[j] 
					= data->profile[i].tt[j];
				store->profile[i].angle[j] 
					= data->profile[i].angle[j];
				store->profile[i].quality[j] 
					= data->profile[i].quality[j];
				store->profile[i].amp[j] 
					= data->profile[i].amp[j];
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
int mbr_wt_bchrxunb(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_bchrxunb";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_bchrxunb_struct *data;
	char	*data_ptr;
	struct mbsys_elac_struct *store;
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
	data = (struct mbf_bchrxunb_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	store = (struct mbsys_elac_struct *) store_ptr;

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
		data->transducer_port_height = store->transducer_port_height;
		data->transducer_starboard_height 
			= store->transducer_starboard_height;
		data->transducer_port_depth = store->transducer_port_depth;
		data->transducer_starboard_depth 
			= store->transducer_starboard_depth;
		data->transducer_port_x = store->transducer_port_x;
		data->transducer_starboard_x = store->transducer_starboard_x;
		data->transducer_port_y = store->transducer_port_y;
		data->transducer_starboard_y = store->transducer_starboard_y;
		data->transducer_port_error = store->transducer_port_error;
		data->transducer_starboard_error 
			= store->transducer_starboard_error;
		data->antenna_height = store->antenna_height;
		data->antenna_x = store->antenna_x;
		data->antenna_y = store->antenna_y;
		data->vru_height = store->vru_height;
		data->vru_x = store->vru_x;
		data->vru_y = store->vru_y;
		data->heave_offset = store->heave_offset;
		data->line_number = store->line_number;
		data->start_or_stop = store->start_or_stop;
		data->transducer_serial_number 
			= store->transducer_serial_number;
		for (i=0;i<MBF_BCHRXUNB_COMMENT_LENGTH;i++)
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

		/* depth telegram */
		data->ping_num = store->ping_num;
		data->sound_vel = store->sound_vel;
		data->mode = store->mode;
		data->pulse_length = store->pulse_length;
		data->source_power = store->source_power;
		data->receiver_gain = store->receiver_gain;
		data->profile_num = store->profile_num;
		data->beams_bath = store->beams_bath;
		for (i=0;i<7;i++)
			{
			data->profile[i].year = store->profile[i].year;
			data->profile[i].month = store->profile[i].month;
			data->profile[i].day = store->profile[i].day;
			data->profile[i].hour = store->profile[i].hour;
			data->profile[i].minute = store->profile[i].minute;
			data->profile[i].second = store->profile[i].second;
			data->profile[i].hundredth_sec 
				= store->profile[i].hundredth_sec;
			data->profile[i].thousandth_sec 
				= store->profile[i].thousandth_sec;
			data->profile[i].longitude = store->profile[i].longitude;
			data->profile[i].latitude = store->profile[i].latitude;
			data->profile[i].roll = store->profile[i].roll;
			data->profile[i].pitch = store->profile[i].pitch;
			data->profile[i].heading = store->profile[i].heading;
			data->profile[i].heave = store->profile[i].heave;
			for (j=0;j<8;j++)
				{
				data->profile[i].bath[j] 
					= store->profile[i].bath[j];
				data->profile[i].bath_acrosstrack[j] 
					= store->profile[i].bath_acrosstrack[j];
				data->profile[i].bath_alongtrack[j] 
					= store->profile[i].bath_alongtrack[j];
				data->profile[i].tt[j] 
					= store->profile[i].tt[j];
				data->profile[i].angle[j] 
					= store->profile[i].angle[j];
				data->profile[i].quality[j] 
					= store->profile[i].quality[j];
				data->profile[i].amp[j] 
					= store->profile[i].amp[j];
				}
			}
		}

	/* write next data to file */
	status = mbr_bchrxunb_wr_data(verbose,mbio_ptr,data_ptr,error);

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
int mbr_bchrxunb_rd_data(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_bchrxunb_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_bchrxunb_struct *data;
	char	*data_ptr;
	FILE	*mbfp;
	int	done;
	short int *type;
	static char label[2];

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
	data = (struct mbf_bchrxunb_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	mbfp = mb_io_ptr->mbfp;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	done = MB_NO;
	type = (short int *) label;
	*error = MB_ERROR_NO_ERROR;
	while (done == MB_NO)
		{
		/* get next record label */
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

		/* swap bytes if necessary */
#ifdef BYTESWAPPED
		*type = (short int) mb_swap_short(*type);
#endif

		/* read the appropriate data records */
		if (status == MB_FAILURE)
			{
			done = MB_YES;
			}
		else if (*type == ELAC_COMMENT)
			{
			status = mbr_bchrxunb_rd_comment(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_COMMENT;
				}
			}
		else if (*type == ELAC_PARAMETER)
			{
			status = mbr_bchrxunb_rd_parameter(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_PARAMETER;
				}
			}
		else if (*type == ELAC_POS)
			{
			status = mbr_bchrxunb_rd_pos(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_NAV;
				}
			}
		else if (*type == ELAC_SVP)
			{
			status = mbr_bchrxunb_rd_svp(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_VELOCITY_PROFILE;
				}
			}
		else if (*type == ELAC_XBATH56)
			{
			status = mbr_bchrxunb_rd_bath56(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_DATA;
				}
			}
		else if (*type == ELAC_XBATH40)
			{
			status = mbr_bchrxunb_rd_bath40(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_DATA;
				}
			}
		else if (*type == ELAC_XBATH32)
			{
			status = mbr_bchrxunb_rd_bath32(
				verbose,mbfp,data,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_DATA;
				}
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}

		/* bail out if there is an error */
		if (status == MB_FAILURE)
			done = MB_YES;

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
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_bchrxunb_rd_comment(int verbose, FILE *mbfp, 
		struct mbf_bchrxunb_struct *data, int *error)
{
	char	*function_name = "mbr_bchrxunb_rd_comment";
	int	status = MB_SUCCESS;
	char	line[ELAC_COMMENT_SIZE+3];
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
	status = fread(line,1,ELAC_COMMENT_SIZE+3,mbfp);
	if (status == ELAC_COMMENT_SIZE+3)
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
		strncpy(data->comment,line,MBF_BCHRXUNB_COMMENT_LENGTH-1);
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
int mbr_bchrxunb_rd_parameter(int verbose, FILE *mbfp, 
		struct mbf_bchrxunb_struct *data, int *error)
{
	char	*function_name = "mbr_bchrxunb_rd_parameter";
	int	status = MB_SUCCESS;
	char	line[ELAC_XPARAMETER_SIZE+3];
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
	status = fread(line,1,ELAC_XPARAMETER_SIZE+3,mbfp);
	if (status == ELAC_XPARAMETER_SIZE+3)
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
		data->transducer_port_height = *short_ptr;
		short_ptr = (short int *) &line[18];
		data->transducer_starboard_height = *short_ptr;
		short_ptr = (short int *) &line[20];
		data->transducer_port_depth = *short_ptr;
		short_ptr = (short int *) &line[22];
		data->transducer_starboard_depth = *short_ptr;
		short_ptr = (short int *) &line[24];
		data->transducer_port_x = *short_ptr;
		short_ptr = (short int *) &line[26];
		data->transducer_starboard_x = *short_ptr;
		short_ptr = (short int *) &line[28];
		data->transducer_port_y = *short_ptr;
		short_ptr = (short int *) &line[30];
		data->transducer_starboard_y = *short_ptr;
		short_ptr = (short int *) &line[32];
		data->transducer_port_error = *short_ptr;
		short_ptr = (short int *) &line[34];
		data->transducer_starboard_error = *short_ptr;
		short_ptr = (short int *) &line[36];
		data->antenna_height = *short_ptr;
		short_ptr = (short int *) &line[38];
		data->antenna_x = *short_ptr;
		short_ptr = (short int *) &line[40];
		data->antenna_y = *short_ptr;
		short_ptr = (short int *) &line[42];
		data->vru_height = *short_ptr;
		short_ptr = (short int *) &line[44];
		data->vru_x = *short_ptr;
		short_ptr = (short int *) &line[46];
		data->vru_y = *short_ptr;
		short_ptr = (short int *) &line[48];
		data->line_number = *short_ptr;
		short_ptr = (short int *) &line[50];
		data->start_or_stop = *short_ptr;
		short_ptr = (short int *) &line[52];
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
		data->transducer_port_height 
			= (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[18];
		data->transducer_starboard_height 
			= (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[20];
		data->transducer_port_depth 
			= (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[22];
		data->transducer_starboard_depth 
			= (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[24];
		data->transducer_port_x 
			= (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[26];
		data->transducer_starboard_x 
			= (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[28];
		data->transducer_port_y 
			= (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[30];
		data->transducer_starboard_y 
			= (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[32];
		data->transducer_port_error 
			= (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[34];
		data->transducer_starboard_error 
			= (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[36];
		data->antenna_height = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[38];
		data->antenna_x = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[40];
		data->antenna_y = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[42];
		data->vru_height = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[44];
		data->vru_x = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[46];
		data->vru_y = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[48];
		data->line_number = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[50];
		data->start_or_stop = (short int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[52];
		data->transducer_serial_number 
			= (short int) mb_swap_short(*short_ptr);
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
		fprintf(stderr,"dbg5       transducer_port_height: %d\n",
			data->transducer_port_height);
		fprintf(stderr,"dbg5       transducer_starboard_height:%d\n",
			data->transducer_starboard_height);
		fprintf(stderr,"dbg5       transducer_port_depth:     %d\n",
			data->transducer_port_depth);
		fprintf(stderr,"dbg5       transducer_starboard_depth:     %d\n",
			data->transducer_starboard_depth);
		fprintf(stderr,"dbg5       transducer_port_x:        %d\n",
			data->transducer_port_x);
		fprintf(stderr,"dbg5       transducer_starboard_x:        %d\n",
			data->transducer_starboard_x);
		fprintf(stderr,"dbg5       transducer_port_y:        %d\n",
			data->transducer_port_y);
		fprintf(stderr,"dbg5       transducer_starboard_y:  %d\n",
			data->transducer_starboard_y);
		fprintf(stderr,"dbg5       transducer_port_error:  %d\n",
			data->transducer_port_error);
		fprintf(stderr,"dbg5       transducer_starboard_error:  %d\n",
			data->transducer_starboard_error);
		fprintf(stderr,"dbg5       antenna_height:            %d\n",data->antenna_height);
		fprintf(stderr,"dbg5       antenna_x:      %d\n",data->antenna_x);
		fprintf(stderr,"dbg5       antenna_y:    %d\n",data->antenna_y);
		fprintf(stderr,"dbg5       vru_height:%d\n",data->vru_height);
		fprintf(stderr,"dbg5       vru_x:%d\n",data->vru_x);
		fprintf(stderr,"dbg5       vru_y:%d\n",data->vru_y);
		fprintf(stderr,"dbg5       line_number:%d\n",data->line_number);
		fprintf(stderr,"dbg5       start_or_stop:%d\n",data->start_or_stop);
		fprintf(stderr,"dbg5       transducer_serial_number:%d\n",
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
int mbr_bchrxunb_rd_pos(int verbose, FILE *mbfp, 
		struct mbf_bchrxunb_struct *data, int *error)
{
	char	*function_name = "mbr_bchrxunb_rd_pos";
	int	status = MB_SUCCESS;
	char	line[ELAC_POS_SIZE+3];
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
	status = fread(line,1,ELAC_POS_SIZE+3,mbfp);
	if (status == ELAC_POS_SIZE+3)
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
		data->pos_latitude = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[12];
		data->pos_longitude = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[16];
		data->utm_northing = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[20];
		data->utm_easting = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[24];
		data->utm_zone_lon = (int) mb_swap_int(*int_ptr);
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
		
	/* KLUGE for 1996 UNB TRAINING COURSE - FLIP LONGITUDE */
	if (data->pos_year == 96 
	    && data->pos_month >= 6 
	    && data->pos_month <= 8)
		data->pos_longitude = -data->pos_longitude;

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
int mbr_bchrxunb_rd_svp(int verbose, FILE *mbfp, 
		struct mbf_bchrxunb_struct *data, int *error)
{
	char	*function_name = "mbr_bchrxunb_rd_svp";
	int	status = MB_SUCCESS;
	char	line[ELAC_SVP_SIZE+3];
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
	status = fread(line,1,ELAC_SVP_SIZE+3,mbfp);
	if (status == ELAC_SVP_SIZE+3)
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
		data->svp_latitude = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[12];
		data->svp_latitude = (int) mb_swap_int(*int_ptr);
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
int mbr_bchrxunb_rd_bath56(int verbose, FILE *mbfp, 
		struct mbf_bchrxunb_struct *data, int *error)
{
	char	*function_name = "mbr_bchrxunb_rd_bath56";
	int	status = MB_SUCCESS;
	char	line[ELAC_XBATH56_SIZE+3];
	char	*profile;
	char	*beam;
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
	status = fread(line,1,ELAC_XBATH56_SIZE+3,mbfp);
	if (status == ELAC_XBATH56_SIZE+3)
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
#ifndef BYTESWAPPED
		short_ptr = (short int *) &line[0];
		data->ping_num = (int) *short_ptr;
		short_ptr = (short int *) &line[2];
		data->sound_vel = (int) *short_ptr;
#else
		short_ptr = (short int *) &line[0];
		data->ping_num = (int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[2];
		data->sound_vel = (int) mb_swap_short(*short_ptr);
#endif
		data->mode = (int) line[4];
		data->pulse_length = (int) line[5];
		data->source_power = (int) line[6];
		data->receiver_gain = (int) line[7];
		data->profile_num = 7;
		data->beams_bath = 56;
		for (i=0;i<data->profile_num;i++)
			{
			profile = &line[8+i*152];
			data->profile[i].day =            (int) profile[0];
			data->profile[i].month =          (int) profile[1];
			data->profile[i].year =           (int) profile[2];
			data->profile[i].hour =           (int) profile[3];
			data->profile[i].minute =         (int) profile[4];
			data->profile[i].second =         (int) profile[5];
			data->profile[i].hundredth_sec =  (int) profile[6];
			data->profile[i].thousandth_sec = (int) profile[7];
#ifndef BYTESWAPPED
			int_ptr = (int *) &profile[8];
			data->profile[i].latitude = (int) *int_ptr;
			int_ptr = (int *) &profile[12];
			data->profile[i].longitude = (int) *int_ptr;
			short_ptr = (short int *) &profile[16];
			data->profile[i].roll = (int) *short_ptr;
			short_ptr = (short int *) &profile[18];
			data->profile[i].pitch = (int) *short_ptr;
			short_ptr = (short int *) &profile[20];
			data->profile[i].heading = (int)(unsigned short) *short_ptr;
			short_ptr = (short int *) &profile[22];
			data->profile[i].heave = (int) *short_ptr;
			for (j=0;j<8;j++)
				{
				beam = &profile[24 + 16*j];
				int_ptr = (int *) &beam[0];
				data->profile[i].bath[j] = (int) *int_ptr;
				int_ptr = (int *) &beam[4];
				data->profile[i].bath_acrosstrack[j] 
					= (int) *int_ptr;
				short_ptr = (short *) &beam[8];
				data->profile[i].bath_alongtrack[j] 
					= (short int) *short_ptr;
				short_ptr = (short *) &beam[10];
				data->profile[i].tt[j] = (short int) *short_ptr;
				short_ptr = (short *) &beam[12];
				data->profile[i].angle[j] = (short int) *short_ptr;
				data->profile[i].quality[j] = (short int) beam[14];
				data->profile[i].amp[j] = (short int) beam[15];
				}
#else
			int_ptr = (int *) &profile[8];
			data->profile[i].latitude 
				= (int) mb_swap_int(*int_ptr);
			int_ptr = (int *) &profile[12];
			data->profile[i].longitude 
				= (int) mb_swap_int(*int_ptr);
			short_ptr = (short int *) &profile[16];
			data->profile[i].roll 
				= (int) mb_swap_short(*short_ptr);
			short_ptr = (short int *) &profile[18];
			data->profile[i].pitch 
				= (int) mb_swap_short(*short_ptr);
			short_ptr = (short int *) &profile[20];
			data->profile[i].heading 
				= (int)(unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short int *) &profile[22];
			data->profile[i].heave 
				= (int) mb_swap_short(*short_ptr);
			for (j=0;j<8;j++)
				{
				beam = &profile[24 + 16*j];
				int_ptr = (int *) &beam[0];
				data->profile[i].bath[j] 
					= (int) mb_swap_int(*int_ptr);
				int_ptr = (int *) &beam[4];
				data->profile[i].bath_acrosstrack[j] 
					= (int) mb_swap_int(*int_ptr);
				short_ptr = (short *) &beam[8];
				data->profile[i].bath_alongtrack[j] 
					= (short int) mb_swap_short(*short_ptr);
				short_ptr = (short *) &beam[10];
				data->profile[i].tt[j] 
					= (short int) mb_swap_short(*short_ptr);
				short_ptr = (short *) &beam[12];
				data->profile[i].angle[j] 
					= (short int) mb_swap_short(*short_ptr);
				data->profile[i].quality[j] = (short int) beam[14];
				data->profile[i].amp[j] = (short int) beam[15];
				}
#endif
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       ping_num:         %d\n",data->ping_num);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       mode:             %d\n",data->mode);
		fprintf(stderr,"dbg5       pulse_length:     %d\n",
			data->pulse_length);
		fprintf(stderr,"dbg5       source_power:     %d\n",
			data->source_power);
		fprintf(stderr,"dbg5       receiver_gain:    %d\n",
			data->receiver_gain);
		fprintf(stderr,"dbg5       profile_num:      %d\n",
			data->profile_num);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",
			data->beams_bath);
		for (i=0;i<data->profile_num;i++)
			{
			fprintf(stderr,"dbg5       profile:          %d\n",i);
			fprintf(stderr,"dbg5       year:             %d\n",
				data->profile[i].year);
			fprintf(stderr,"dbg5       month:            %d\n",
				data->profile[i].month);
			fprintf(stderr,"dbg5       day:              %d\n",
				data->profile[i].day);
			fprintf(stderr,"dbg5       hour:             %d\n",
				data->profile[i].hour);
			fprintf(stderr,"dbg5       minute:           %d\n",
				data->profile[i].minute);
			fprintf(stderr,"dbg5       sec:              %d\n",
				data->profile[i].second);
			fprintf(stderr,"dbg5       hundredth_sec:    %d\n",
				data->profile[i].hundredth_sec);
			fprintf(stderr,"dbg5       thousandth_sec:   %d\n",
				data->profile[i].thousandth_sec);
			fprintf(stderr,"dbg5       latitude:         %d\n",
				data->profile[i].latitude);
			fprintf(stderr,"dbg5       longitude:        %d\n",
				data->profile[i].longitude);
			fprintf(stderr,"dbg5       roll:             %d\n",
				data->profile[i].roll);
			fprintf(stderr,"dbg5       pitch:            %d\n",
				data->profile[i].pitch);
			fprintf(stderr,"dbg5       heading:          %d\n",
				data->profile[i].heading);
			fprintf(stderr,"dbg5       heave:            %d\n",
				data->profile[i].heave);
			for (j=0;j<8;j++)
				{
				fprintf(stderr,"dbg5       bath[%2d][%d]:             %d\n",
					i,j,data->profile[i].bath[j]);
				fprintf(stderr,"dbg5       bath_acrosstrack[%2d][%d]: %d\n",
					i,j,data->profile[i].bath_acrosstrack[j]);
				fprintf(stderr,"dbg5       bath_alongtrack[%2d][%d]:  %d\n",
					i,j,data->profile[i].bath_alongtrack[j]);
				fprintf(stderr,"dbg5       tt[%2d][%d]:               %d\n",
					i,j,data->profile[i].tt[j]);
				fprintf(stderr,"dbg5       angle[%2d][%d]:            %d\n",
					i,j,data->profile[i].angle[j]);
				fprintf(stderr,"dbg5       quality[%2d][%d]:          %d\n",
					i,j,data->profile[i].quality[j]);
				fprintf(stderr,"dbg5       amp[%2d][%d]:              %d\n",
					i,j,data->profile[i].amp[j]);
				}
			fprintf(stderr,"dbg5       \n");
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
int mbr_bchrxunb_rd_bath40(int verbose, FILE *mbfp, 
		struct mbf_bchrxunb_struct *data, int *error)
{
	char	*function_name = "mbr_bchrxunb_rd_bath40";
	int	status = MB_SUCCESS;
	char	line[ELAC_XBATH40_SIZE+3];
	char	*profile;
	char	*beam;
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
	status = fread(line,1,ELAC_XBATH40_SIZE+3,mbfp);
	if (status == ELAC_XBATH40_SIZE+3)
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
#ifndef BYTESWAPPED
		short_ptr = (short int *) &line[0];
		data->ping_num = (int) *short_ptr;
		short_ptr = (short int *) &line[2];
		data->sound_vel = (int) *short_ptr;
#else
		short_ptr = (short int *) &line[0];
		data->ping_num = (int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[2];
		data->sound_vel = (int) mb_swap_short(*short_ptr);
#endif
		data->mode = (int) line[4];
		data->pulse_length = (int) line[5];
		data->source_power = (int) line[6];
		data->receiver_gain = (int) line[7];
		data->profile_num = 5;
		data->beams_bath = 40;
		for (i=0;i<data->profile_num;i++)
			{
			profile = &line[8+i*152];
			data->profile[i].day =            (int) profile[0];
			data->profile[i].month =          (int) profile[1];
			data->profile[i].year =           (int) profile[2];
			data->profile[i].hour =           (int) profile[3];
			data->profile[i].minute =         (int) profile[4];
			data->profile[i].second =         (int) profile[5];
			data->profile[i].hundredth_sec =  (int) profile[6];
			data->profile[i].thousandth_sec = (int) profile[7];
#ifndef BYTESWAPPED
			int_ptr = (int *) &profile[8];
			data->profile[i].latitude = (int) *int_ptr;
			int_ptr = (int *) &profile[12];
			data->profile[i].longitude = (int) *int_ptr;
			short_ptr = (short int *) &profile[16];
			data->profile[i].roll = (int) *short_ptr;
			short_ptr = (short int *) &profile[18];
			data->profile[i].pitch = (int) *short_ptr;
			short_ptr = (short int *) &profile[20];
			data->profile[i].heading = (int)(unsigned short) *short_ptr;
			short_ptr = (short int *) &profile[22];
			data->profile[i].heave = (int) *short_ptr;
			for (j=0;j<8;j++)
				{
				beam = &profile[24 + 16*j];
				int_ptr = (int *) &beam[0];
				data->profile[i].bath[j] = (int) *int_ptr;
				int_ptr = (int *) &beam[4];
				data->profile[i].bath_acrosstrack[j] 
					= (int) *int_ptr;
				short_ptr = (short *) &beam[8];
				data->profile[i].bath_alongtrack[j] 
					= (short int) *short_ptr;
				short_ptr = (short *) &beam[10];
				data->profile[i].tt[j] = (short int) *short_ptr;
				short_ptr = (short *) &beam[12];
				data->profile[i].angle[j] = (short int) *short_ptr;
				data->profile[i].quality[j] = (short int) beam[14];
				data->profile[i].amp[j] = (short int) beam[15];
				}
#else
			int_ptr = (int *) &profile[8];
			data->profile[i].latitude 
				= (int) mb_swap_int(*int_ptr);
			int_ptr = (int *) &profile[12];
			data->profile[i].longitude 
				= (int) mb_swap_int(*int_ptr);
			short_ptr = (short int *) &profile[16];
			data->profile[i].roll 
				= (int) mb_swap_short(*short_ptr);
			short_ptr = (short int *) &profile[18];
			data->profile[i].pitch 
				= (int) mb_swap_short(*short_ptr);
			short_ptr = (short int *) &profile[20];
			data->profile[i].heading 
				= (int)(unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short int *) &profile[22];
			data->profile[i].heave 
				= (int) mb_swap_short(*short_ptr);
			for (j=0;j<8;j++)
				{
				beam = &profile[24 + 16*j];
				int_ptr = (int *) &beam[0];
				data->profile[i].bath[j] 
					= (int) mb_swap_int(*int_ptr);
				int_ptr = (int *) &beam[4];
				data->profile[i].bath_acrosstrack[j] 
					= (int) mb_swap_int(*int_ptr);
				short_ptr = (short *) &beam[8];
				data->profile[i].bath_alongtrack[j] 
					= (short int) mb_swap_short(*short_ptr);
				short_ptr = (short *) &beam[10];
				data->profile[i].tt[j] 
					= (short int) mb_swap_short(*short_ptr);
				short_ptr = (short *) &beam[12];
				data->profile[i].angle[j] 
					= (short int) mb_swap_short(*short_ptr);
				data->profile[i].quality[j] = (short int) beam[14];
				data->profile[i].amp[j] = (short int) beam[15];
				}
#endif
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       ping_num:         %d\n",data->ping_num);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       mode:             %d\n",data->mode);
		fprintf(stderr,"dbg5       pulse_length:     %d\n",
			data->pulse_length);
		fprintf(stderr,"dbg5       source_power:     %d\n",
			data->source_power);
		fprintf(stderr,"dbg5       receiver_gain:    %d\n",
			data->receiver_gain);
		fprintf(stderr,"dbg5       profile_num:      %d\n",
			data->profile_num);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",
			data->beams_bath);
		for (i=0;i<data->profile_num;i++)
			{
			fprintf(stderr,"dbg5       profile:          %d\n",i);
			fprintf(stderr,"dbg5       year:             %d\n",
				data->profile[i].year);
			fprintf(stderr,"dbg5       month:            %d\n",
				data->profile[i].month);
			fprintf(stderr,"dbg5       day:              %d\n",
				data->profile[i].day);
			fprintf(stderr,"dbg5       hour:             %d\n",
				data->profile[i].hour);
			fprintf(stderr,"dbg5       minute:           %d\n",
				data->profile[i].minute);
			fprintf(stderr,"dbg5       sec:              %d\n",
				data->profile[i].second);
			fprintf(stderr,"dbg5       hundredth_sec:    %d\n",
				data->profile[i].hundredth_sec);
			fprintf(stderr,"dbg5       thousandth_sec:   %d\n",
				data->profile[i].thousandth_sec);
			fprintf(stderr,"dbg5       latitude:         %d\n",
				data->profile[i].latitude);
			fprintf(stderr,"dbg5       longitude:        %d\n",
				data->profile[i].longitude);
			fprintf(stderr,"dbg5       roll:             %d\n",
				data->profile[i].roll);
			fprintf(stderr,"dbg5       pitch:            %d\n",
				data->profile[i].pitch);
			fprintf(stderr,"dbg5       heading:          %d\n",
				data->profile[i].heading);
			fprintf(stderr,"dbg5       heave:            %d\n",
				data->profile[i].heave);
			for (j=0;j<8;j++)
				{
				fprintf(stderr,"dbg5       bath:             %d\n",
					data->profile[i].bath[j]);
				fprintf(stderr,"dbg5       bath_acrosstrack: %d\n",
					data->profile[i].bath_acrosstrack[j]);
				fprintf(stderr,"dbg5       bath_alongtrack:  %d\n",
					data->profile[i].bath_alongtrack[j]);
				fprintf(stderr,"dbg5       tt:               %d\n",
					data->profile[i].tt[j]);
				fprintf(stderr,"dbg5       angle:            %d\n",
					data->profile[i].angle[j]);
				fprintf(stderr,"dbg5       quality:          %d\n",
					data->profile[i].quality[j]);
				fprintf(stderr,"dbg5       amp:              %d\n",
					data->profile[i].amp[j]);
				}
			fprintf(stderr,"dbg5       \n");
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
int mbr_bchrxunb_rd_bath32(int verbose, FILE *mbfp, 
		struct mbf_bchrxunb_struct *data, int *error)
{
	char	*function_name = "mbr_bchrxunb_rd_bath32";
	int	status = MB_SUCCESS;
	char	line[ELAC_XBATH32_SIZE+3];
	char	*profile;
	char	*beam;
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
	status = fread(line,1,ELAC_XBATH32_SIZE+3,mbfp);
	if (status == ELAC_XBATH32_SIZE+3)
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
#ifndef BYTESWAPPED
		short_ptr = (short int *) &line[0];
		data->ping_num = (int) *short_ptr;
		short_ptr = (short int *) &line[2];
		data->sound_vel = (int) *short_ptr;
#else
		short_ptr = (short int *) &line[0];
		data->ping_num = (int) mb_swap_short(*short_ptr);
		short_ptr = (short int *) &line[2];
		data->sound_vel = (int) mb_swap_short(*short_ptr);
#endif
		data->mode = (int) line[4];
		data->pulse_length = (int) line[5];
		data->source_power = (int) line[6];
		data->receiver_gain = (int) line[7];
		data->profile_num = 4;
		data->beams_bath = 32;
		for (i=0;i<data->profile_num;i++)
			{
			profile = &line[8+i*152];
			data->profile[i].day =            (int) profile[0];
			data->profile[i].month =          (int) profile[1];
			data->profile[i].year =           (int) profile[2];
			data->profile[i].hour =           (int) profile[3];
			data->profile[i].minute =         (int) profile[4];
			data->profile[i].second =         (int) profile[5];
			data->profile[i].hundredth_sec =  (int) profile[6];
			data->profile[i].thousandth_sec = (int) profile[7];
#ifndef BYTESWAPPED
			int_ptr = (int *) &profile[8];
			data->profile[i].latitude = (int) *int_ptr;
			int_ptr = (int *) &profile[12];
			data->profile[i].longitude = (int) *int_ptr;
			short_ptr = (short int *) &profile[16];
			data->profile[i].roll = (int) *short_ptr;
			short_ptr = (short int *) &profile[18];
			data->profile[i].pitch = (int) *short_ptr;
			short_ptr = (short int *) &profile[20];
			data->profile[i].heading = (int)(unsigned short) *short_ptr;
			short_ptr = (short int *) &profile[22];
			data->profile[i].heave = (int) *short_ptr;
			for (j=0;j<8;j++)
				{
				beam = &profile[24 + 16*j];
				int_ptr = (int *) &beam[0];
				data->profile[i].bath[j] = (int) *int_ptr;
				int_ptr = (int *) &beam[4];
				data->profile[i].bath_acrosstrack[j] 
					= (int) *int_ptr;
				short_ptr = (short *) &beam[8];
				data->profile[i].bath_alongtrack[j] 
					= (short int) *short_ptr;
				short_ptr = (short *) &beam[10];
				data->profile[i].tt[j] = (short int) *short_ptr;
				short_ptr = (short *) &beam[12];
				data->profile[i].angle[j] = (short int) *short_ptr;
				data->profile[i].quality[j] = (short int) beam[14];
				data->profile[i].amp[j] = (short int) beam[15];
				}
#else
			int_ptr = (int *) &profile[8];
			data->profile[i].latitude 
				= (int) mb_swap_int(*int_ptr);
			int_ptr = (int *) &profile[12];
			data->profile[i].longitude 
				= (int) mb_swap_int(*int_ptr);
			short_ptr = (short int *) &profile[16];
			data->profile[i].roll 
				= (int) mb_swap_short(*short_ptr);
			short_ptr = (short int *) &profile[18];
			data->profile[i].pitch 
				= (int) mb_swap_short(*short_ptr);
			short_ptr = (short int *) &profile[20];
			data->profile[i].heading 
				= (int)(unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short int *) &profile[22];
			data->profile[i].heave 
				= (int) mb_swap_short(*short_ptr);
			for (j=0;j<8;j++)
				{
				beam = &profile[24 + 16*j];
				int_ptr = (int *) &beam[0];
				data->profile[i].bath[j] 
					= (int) mb_swap_int(*int_ptr);
				int_ptr = (int *) &beam[4];
				data->profile[i].bath_acrosstrack[j] 
					= (int) mb_swap_int(*int_ptr);
				short_ptr = (short *) &beam[8];
				data->profile[i].bath_alongtrack[j] 
					= (short int) mb_swap_short(*short_ptr);
				short_ptr = (short *) &beam[10];
				data->profile[i].tt[j] 
					= (short int) mb_swap_short(*short_ptr);
				short_ptr = (short *) &beam[12];
				data->profile[i].angle[j] 
					= (short int) mb_swap_short(*short_ptr);
				data->profile[i].quality[j] = (short int) beam[14];
				data->profile[i].amp[j] = (short int) beam[15];
				}
#endif
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       ping_num:         %d\n",data->ping_num);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       mode:             %d\n",data->mode);
		fprintf(stderr,"dbg5       pulse_length:     %d\n",
			data->pulse_length);
		fprintf(stderr,"dbg5       source_power:     %d\n",
			data->source_power);
		fprintf(stderr,"dbg5       receiver_gain:    %d\n",
			data->receiver_gain);
		fprintf(stderr,"dbg5       profile_num:      %d\n",
			data->profile_num);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",
			data->beams_bath);
		for (i=0;i<data->profile_num;i++)
			{
			fprintf(stderr,"dbg5       profile:          %d\n",i);
			fprintf(stderr,"dbg5       year:             %d\n",
				data->profile[i].year);
			fprintf(stderr,"dbg5       month:            %d\n",
				data->profile[i].month);
			fprintf(stderr,"dbg5       day:              %d\n",
				data->profile[i].day);
			fprintf(stderr,"dbg5       hour:             %d\n",
				data->profile[i].hour);
			fprintf(stderr,"dbg5       minute:           %d\n",
				data->profile[i].minute);
			fprintf(stderr,"dbg5       sec:              %d\n",
				data->profile[i].second);
			fprintf(stderr,"dbg5       hundredth_sec:    %d\n",
				data->profile[i].hundredth_sec);
			fprintf(stderr,"dbg5       thousandth_sec:   %d\n",
				data->profile[i].thousandth_sec);
			fprintf(stderr,"dbg5       latitude:         %d\n",
				data->profile[i].latitude);
			fprintf(stderr,"dbg5       longitude:        %d\n",
				data->profile[i].longitude);
			fprintf(stderr,"dbg5       roll:             %d\n",
				data->profile[i].roll);
			fprintf(stderr,"dbg5       pitch:            %d\n",
				data->profile[i].pitch);
			fprintf(stderr,"dbg5       heading:          %d\n",
				data->profile[i].heading);
			fprintf(stderr,"dbg5       heave:            %d\n",
				data->profile[i].heave);
			for (j=0;j<8;j++)
				{
				fprintf(stderr,"dbg5       bath:             %d\n",
					data->profile[i].bath[j]);
				fprintf(stderr,"dbg5       bath_acrosstrack: %d\n",
					data->profile[i].bath_acrosstrack[j]);
				fprintf(stderr,"dbg5       bath_alongtrack:  %d\n",
					data->profile[i].bath_alongtrack[j]);
				fprintf(stderr,"dbg5       tt:               %d\n",
					data->profile[i].tt[j]);
				fprintf(stderr,"dbg5       angle:            %d\n",
					data->profile[i].angle[j]);
				fprintf(stderr,"dbg5       quality:          %d\n",
					data->profile[i].quality[j]);
				fprintf(stderr,"dbg5       amp:              %d\n",
					data->profile[i].amp[j]);
				}
			fprintf(stderr,"dbg5       \n");
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
int mbr_bchrxunb_wr_data(int verbose, void *mbio_ptr, char *data_ptr, int *error)
{
	char	*function_name = "mbr_bchrxunb_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_bchrxunb_struct *data;
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
	data = (struct mbf_bchrxunb_struct *) data_ptr;
	mbfp = mb_io_ptr->mbfp;

	if (data->kind == MB_DATA_COMMENT)
		{
		status = mbr_bchrxunb_wr_comment(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_PARAMETER)
		{
		status = mbr_bchrxunb_wr_parameter(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_NAV)
		{
		status = mbr_bchrxunb_wr_pos(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_VELOCITY_PROFILE)
		{
		status = mbr_bchrxunb_wr_svp(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_DATA && data->profile_num == 7)
		{
		status = mbr_bchrxunb_wr_bath56(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_DATA && data->profile_num == 5)
		{
		status = mbr_bchrxunb_wr_bath40(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_DATA && data->profile_num == 4)
		{
		status = mbr_bchrxunb_wr_bath32(verbose,mbfp,data,error);
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
int mbr_bchrxunb_wr_comment(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_bchrxunb_wr_comment";
	int	status = MB_SUCCESS;
	struct mbf_bchrxunb_struct *data;
	char	line[ELAC_COMMENT_SIZE+3];
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
	data = (struct mbf_bchrxunb_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",data->comment);
		}

	/* write the record label */
	label = ELAC_COMMENT;
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
		if (len > MBSYS_ELAC_COMMENT_LENGTH)
			len = MBSYS_ELAC_COMMENT_LENGTH;
		for (i=0;i<len;i++)
			line[i] = data->comment[i];
		for (i=len;i<MBSYS_ELAC_COMMENT_LENGTH;i++)
			line[i] = '\0';
		line[ELAC_COMMENT_SIZE] = 0x03;
		line[ELAC_COMMENT_SIZE+1] = '\0';
		line[ELAC_COMMENT_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,ELAC_COMMENT_SIZE+3,mbfp);
		if (status != ELAC_COMMENT_SIZE+3)
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
int mbr_bchrxunb_wr_parameter(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_bchrxunb_wr_parameter";
	int	status = MB_SUCCESS;
	struct mbf_bchrxunb_struct *data;
	char	line[ELAC_XPARAMETER_SIZE+3];
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
	data = (struct mbf_bchrxunb_struct *) data_ptr;

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
		fprintf(stderr,"dbg5       transducer_port_height: %d\n",
			data->transducer_port_height);
		fprintf(stderr,"dbg5       transducer_starboard_height:%d\n",
			data->transducer_starboard_height);
		fprintf(stderr,"dbg5       transducer_port_depth:     %d\n",
			data->transducer_port_depth);
		fprintf(stderr,"dbg5       transducer_starboard_depth:     %d\n",
			data->transducer_starboard_depth);
		fprintf(stderr,"dbg5       transducer_port_x:        %d\n",
			data->transducer_port_x);
		fprintf(stderr,"dbg5       transducer_starboard_x:        %d\n",
			data->transducer_starboard_x);
		fprintf(stderr,"dbg5       transducer_port_y:        %d\n",
			data->transducer_port_y);
		fprintf(stderr,"dbg5       transducer_starboard_y:  %d\n",
			data->transducer_starboard_y);
		fprintf(stderr,"dbg5       transducer_port_error:  %d\n",
			data->transducer_port_error);
		fprintf(stderr,"dbg5       transducer_starboard_error:  %d\n",
			data->transducer_starboard_error);
		fprintf(stderr,"dbg5       antenna_height:            %d\n",data->antenna_height);
		fprintf(stderr,"dbg5       antenna_x:      %d\n",data->antenna_x);
		fprintf(stderr,"dbg5       antenna_y:    %d\n",data->antenna_y);
		fprintf(stderr,"dbg5       vru_height:%d\n",data->vru_height);
		fprintf(stderr,"dbg5       vru_x:%d\n",data->vru_x);
		fprintf(stderr,"dbg5       vru_y:%d\n",data->vru_y);
		fprintf(stderr,"dbg5       line_number:%d\n",data->line_number);
		fprintf(stderr,"dbg5       start_or_stop:%d\n",data->start_or_stop);
		fprintf(stderr,"dbg5       transducer_serial_number:%d\n",
			data->transducer_serial_number);
		}

	/* write the record label */
	label = ELAC_PARAMETER;
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
		*short_ptr = data->transducer_port_height;
		short_ptr = (short int *) &line[18];
		*short_ptr = data->transducer_starboard_height;
		short_ptr = (short int *) &line[20];
		*short_ptr = data->transducer_port_depth;
		short_ptr = (short int *) &line[22];
		*short_ptr = data->transducer_starboard_depth;
		short_ptr = (short int *) &line[24];
		*short_ptr = data->transducer_port_x;
		short_ptr = (short int *) &line[26];
		*short_ptr = data->transducer_starboard_x;
		short_ptr = (short int *) &line[28];
		*short_ptr = data->transducer_port_y;
		short_ptr = (short int *) &line[30];
		*short_ptr = data->transducer_starboard_y;
		short_ptr = (short int *) &line[32];
		*short_ptr = data->transducer_port_error;
		short_ptr = (short int *) &line[34];
		*short_ptr = data->transducer_starboard_error;
		short_ptr = (short int *) &line[36];
		*short_ptr = data->antenna_height;
		short_ptr = (short int *) &line[38];
		*short_ptr = data->antenna_x;
		short_ptr = (short int *) &line[40];
		*short_ptr = data->antenna_y;
		short_ptr = (short int *) &line[42];
		*short_ptr = data->vru_height;
		short_ptr = (short int *) &line[44];
		*short_ptr = data->vru_x;
		short_ptr = (short int *) &line[46];
		*short_ptr = data->vru_y;
		short_ptr = (short int *) &line[48];
		*short_ptr = data->line_number;
		short_ptr = (short int *) &line[50];
		*short_ptr = data->start_or_stop;
		short_ptr = (short int *) &line[52];
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
		*short_ptr = (short int) 
			mb_swap_short(data->transducer_port_height);
		short_ptr = (short int *) &line[18];
		*short_ptr = (short int) 
			mb_swap_short(data->transducer_starboard_height);
		short_ptr = (short int *) &line[20];
		*short_ptr = (short int) mb_swap_short(data->transducer_port_depth);
		short_ptr = (short int *) &line[22];
		*short_ptr = (short int) 
			mb_swap_short(data->transducer_starboard_depth);
		short_ptr = (short int *) &line[24];
		*short_ptr = (short int) mb_swap_short(data->transducer_port_x);
		short_ptr = (short int *) &line[26];
		*short_ptr = (short int) 
			mb_swap_short(data->transducer_starboard_x);
		short_ptr = (short int *) &line[28];
		*short_ptr = (short int) mb_swap_short(data->transducer_port_y);
		short_ptr = (short int *) &line[30];
		*short_ptr = (short int) 
			mb_swap_short(data->transducer_starboard_y);
		short_ptr = (short int *) &line[32];
		*short_ptr = (short int) mb_swap_short(data->transducer_port_error);
		short_ptr = (short int *) &line[34];
		*short_ptr = (short int) 
			mb_swap_short(data->transducer_starboard_error);
		short_ptr = (short int *) &line[36];
		*short_ptr = (short int) mb_swap_short(data->antenna_height);
		short_ptr = (short int *) &line[38];
		*short_ptr = (short int) mb_swap_short(data->antenna_x);
		short_ptr = (short int *) &line[40];
		*short_ptr = (short int) mb_swap_short(data->antenna_y);
		short_ptr = (short int *) &line[42];
		*short_ptr = (short int) mb_swap_short(data->vru_height);
		short_ptr = (short int *) &line[44];
		*short_ptr = (short int) mb_swap_short(data->vru_x);
		short_ptr = (short int *) &line[46];
		*short_ptr = (short int) mb_swap_short(data->vru_y);
		short_ptr = (short int *) &line[48];
		*short_ptr = (short int) mb_swap_short(data->line_number);
		short_ptr = (short int *) &line[50];
		*short_ptr = (short int) mb_swap_short(data->start_or_stop);
		short_ptr = (short int *) &line[52];
		*short_ptr = (short int) 
			mb_swap_short(data->transducer_serial_number);
#endif
		line[ELAC_XPARAMETER_SIZE] = 0x03;
		line[ELAC_XPARAMETER_SIZE+1] = '\0';
		line[ELAC_XPARAMETER_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,ELAC_XPARAMETER_SIZE+3,mbfp);
		if (status != ELAC_XPARAMETER_SIZE+3)
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
int mbr_bchrxunb_wr_pos(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_bchrxunb_wr_pos";
	int	status = MB_SUCCESS;
	struct mbf_bchrxunb_struct *data;
	char	line[ELAC_POS_SIZE+3];
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
	data = (struct mbf_bchrxunb_struct *) data_ptr;

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
	label = ELAC_POS;
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
		*int_ptr = (int) mb_swap_int(data->pos_latitude);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(data->pos_longitude);
		int_ptr = (int *) &line[16];
		*int_ptr = (int) mb_swap_int(data->utm_northing);
		int_ptr = (int *) &line[20];
		*int_ptr = (int) mb_swap_int(data->utm_easting);
		int_ptr = (int *) &line[24];
		*int_ptr = (int) mb_swap_int(data->utm_zone_lon);
		line[28] = data->utm_zone;
		line[29] = data->hemisphere;
		line[30] = data->ellipsoid;
		line[31] = data->pos_spare;
		short_ptr = (short int *) &line[32];
		*short_ptr = (int) mb_swap_short(data->semi_major_axis);
		short_ptr = (short int *) &line[34];
		*short_ptr = (int) mb_swap_short(data->other_quality);
#endif
		line[ELAC_POS_SIZE] = 0x03;
		line[ELAC_POS_SIZE+1] = '\0';
		line[ELAC_POS_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,ELAC_POS_SIZE+3,mbfp);
		if (status != ELAC_POS_SIZE+3)
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
int mbr_bchrxunb_wr_svp(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_bchrxunb_wr_svp";
	int	status = MB_SUCCESS;
	struct mbf_bchrxunb_struct *data;
	char	line[ELAC_SVP_SIZE+3];
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
	data = (struct mbf_bchrxunb_struct *) data_ptr;

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
			fprintf(stderr,"dbg5       depth: %d     vel: %d\n",
				data->svp_depth[i],data->svp_vel[i]);
		}

	/* write the record label */
	label = ELAC_SVP;
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
		*int_ptr = (int) mb_swap_int(data->svp_latitude);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(data->svp_longitude);
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
		line[ELAC_SVP_SIZE] = 0x03;
		line[ELAC_SVP_SIZE+1] = '\0';
		line[ELAC_SVP_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,ELAC_SVP_SIZE+3,mbfp);
		if (status != ELAC_SVP_SIZE+3)
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
int mbr_bchrxunb_wr_bath56(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_bchrxunb_wr_bath56";
	int	status = MB_SUCCESS;
	struct mbf_bchrxunb_struct *data;
	char	line[ELAC_XBATH56_SIZE+3];
	char	*profile;
	char	*beam;
	short int label;
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
	data = (struct mbf_bchrxunb_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       ping_num:         %d\n",data->ping_num);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       mode:             %d\n",data->mode);
		fprintf(stderr,"dbg5       pulse_length:     %d\n",
			data->pulse_length);
		fprintf(stderr,"dbg5       source_power:     %d\n",
			data->source_power);
		fprintf(stderr,"dbg5       receiver_gain:    %d\n",
			data->receiver_gain);
		fprintf(stderr,"dbg5       profile_num:      %d\n",
			data->profile_num);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",
			data->beams_bath);
		for (i=0;i<data->profile_num;i++)
			{
			fprintf(stderr,"dbg5       profile:          %d\n",i);
			fprintf(stderr,"dbg5       year:             %d\n",
				data->profile[i].year);
			fprintf(stderr,"dbg5       month:            %d\n",
				data->profile[i].month);
			fprintf(stderr,"dbg5       day:              %d\n",
				data->profile[i].day);
			fprintf(stderr,"dbg5       hour:             %d\n",
				data->profile[i].hour);
			fprintf(stderr,"dbg5       minute:           %d\n",
				data->profile[i].minute);
			fprintf(stderr,"dbg5       sec:              %d\n",
				data->profile[i].second);
			fprintf(stderr,"dbg5       hundredth_sec:    %d\n",
				data->profile[i].hundredth_sec);
			fprintf(stderr,"dbg5       thousandth_sec:   %d\n",
				data->profile[i].thousandth_sec);
			fprintf(stderr,"dbg5       latitude:         %d\n",
				data->profile[i].latitude);
			fprintf(stderr,"dbg5       longitude:        %d\n",
				data->profile[i].longitude);
			fprintf(stderr,"dbg5       roll:             %d\n",
				data->profile[i].roll);
			fprintf(stderr,"dbg5       pitch:            %d\n",
				data->profile[i].pitch);
			fprintf(stderr,"dbg5       heading:          %d\n",
				data->profile[i].heading);
			fprintf(stderr,"dbg5       heave:            %d\n",
				data->profile[i].heave);
			for (j=0;j<8;j++)
				{
				fprintf(stderr,"dbg5       bath:             %d\n",
					data->profile[i].bath[j]);
				fprintf(stderr,"dbg5       bath_acrosstrack: %d\n",
					data->profile[i].bath_acrosstrack[j]);
				fprintf(stderr,"dbg5       bath_alongtrack:  %d\n",
					data->profile[i].bath_alongtrack[j]);
				fprintf(stderr,"dbg5       tt:               %d\n",
					data->profile[i].tt[j]);
				fprintf(stderr,"dbg5       angle:            %d\n",
					data->profile[i].angle[j]);
				fprintf(stderr,"dbg5       quality:          %d\n",
					data->profile[i].quality[j]);
				fprintf(stderr,"dbg5       amp:              %d\n",
					data->profile[i].amp[j]);
				}
			fprintf(stderr,"dbg5       \n");
			}
		}

	/* write the record label */
	label = ELAC_XBATH56;
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
#ifndef BYTESWAPPED
		short_ptr = (short int *) &line[0];
		*short_ptr = (short int) data->ping_num;
		short_ptr = (short int *) &line[2];
		*short_ptr = (short int) data->sound_vel;
#else
		short_ptr = (short int *) &line[0];
		*short_ptr = (short int) 
			mb_swap_short((short int) data->ping_num);
		short_ptr = (short int *) &line[2];
		*short_ptr = (short int) 
			mb_swap_short((short int) data->sound_vel);
#endif
		line[4] = (char) data->mode;
		line[5] = (char) data->pulse_length;
		line[6] = (char) data->source_power;
		line[7] = (char) data->receiver_gain;
		for (i=0;i<data->profile_num;i++)
			{
			profile = &line[8+i*152];
			profile[0] = (char) data->profile[i].day;
			profile[1] = (char) data->profile[i].month;
			profile[2] = (char) data->profile[i].year;
			profile[3] = (char) data->profile[i].hour;
			profile[4] = (char) data->profile[i].minute;
			profile[5] = (char) data->profile[i].second;
			profile[6] = (char) data->profile[i].hundredth_sec;
			profile[7] = (char) data->profile[i].thousandth_sec;
#ifndef BYTESWAPPED
			int_ptr = (int *) &profile[8];
			*int_ptr = (int) data->profile[i].latitude;
			int_ptr = (int *) &profile[12];
			*int_ptr = (int) data->profile[i].longitude;
			short_ptr = (short int *) &profile[16];
			*short_ptr = (short int) data->profile[i].roll;
			short_ptr = (short int *) &profile[18];
			*short_ptr = (short int) data->profile[i].pitch;
			short_ptr = (short int *) &profile[20];
			*short_ptr = (short int)(unsigned short) data->profile[i].heading;
			short_ptr = (short int *) &profile[22];
			*short_ptr = (short int) data->profile[i].heave;
			for (j=0;j<8;j++)
				{
				beam = &profile[24 + 16*j];
				int_ptr = (int *) &beam[0];
				*int_ptr = (int) data->profile[i].bath[j];
				int_ptr = (int *) &beam[4];
				*int_ptr = (int) 
					data->profile[i].bath_acrosstrack[j];
				short_ptr = (short *) &beam[8];
				*short_ptr = (short int) 
					data->profile[i].bath_alongtrack[j];
				short_ptr = (short *) &beam[10];
				*short_ptr = (short int) data->profile[i].tt[j];
				short_ptr = (short *) &beam[12];
				*short_ptr = (short int) data->profile[i].angle[j];
				beam[14] = (char) data->profile[i].quality[j];
				beam[15] = (char) data->profile[i].amp[j];
				}
#else
			int_ptr = (int *) &profile[8];
			*int_ptr 
				= (int) mb_swap_int(data->profile[i].latitude);
			int_ptr = (int *) &profile[12];
			*int_ptr 
				= (int) mb_swap_int(data->profile[i].longitude);
			short_ptr = (short int *) &profile[16];
			*short_ptr = (short int) 
				mb_swap_short((short int) data->profile[i].roll);
			short_ptr = (short int *) &profile[18];
			*short_ptr = (short int) 
				mb_swap_short((short int) data->profile[i].pitch);
			short_ptr = (short int *) &profile[20];
			*short_ptr = (short int) 
				mb_swap_short((short int)(unsigned short) data->profile[i].heading);
			short_ptr = (short int *) &profile[22];
			*short_ptr = (short int) 
				mb_swap_short((short int) data->profile[i].heave);
			for (j=0;j<8;j++)
				{
				beam = &profile[24 + 16*j];
				int_ptr = (int *) &beam[0];
				*int_ptr = (int) 
					mb_swap_int(data->profile[i].bath[j]);
				int_ptr = (int *) &beam[4];
				*int_ptr = (int) 
					mb_swap_int(data->profile[i].bath_acrosstrack[j]);
				short_ptr = (short *) &beam[8];
				*short_ptr = (short int) 
					mb_swap_short(data->profile[i].bath_alongtrack[j]);
				short_ptr = (short *) &beam[10];
				*short_ptr = (short int) 
					mb_swap_short(data->profile[i].tt[j]);
				short_ptr = (short *) &beam[12];
				*short_ptr = (short int) 
					mb_swap_short(data->profile[i].angle[j]);
				beam[14] = (char) data->profile[i].quality[j];
				beam[15] = (char) data->profile[i].amp[j];
				}
#endif
			}
		line[ELAC_XBATH56_SIZE] = 0x03;
		line[ELAC_XBATH56_SIZE+1] = '\0';
		line[ELAC_XBATH56_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,ELAC_XBATH56_SIZE+3,mbfp);
		if (status != ELAC_XBATH56_SIZE+3)
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
int mbr_bchrxunb_wr_bath40(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_bchrxunb_wr_bath40";
	int	status = MB_SUCCESS;
	struct mbf_bchrxunb_struct *data;
	char	line[ELAC_XBATH40_SIZE+3];
	char	*profile;
	char	*beam;
	short int label;
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
	data = (struct mbf_bchrxunb_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       ping_num:         %d\n",data->ping_num);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       mode:             %d\n",data->mode);
		fprintf(stderr,"dbg5       pulse_length:     %d\n",
			data->pulse_length);
		fprintf(stderr,"dbg5       source_power:     %d\n",
			data->source_power);
		fprintf(stderr,"dbg5       receiver_gain:    %d\n",
			data->receiver_gain);
		fprintf(stderr,"dbg5       profile_num:      %d\n",
			data->profile_num);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",
			data->beams_bath);
		for (i=0;i<data->profile_num;i++)
			{
			fprintf(stderr,"dbg5       profile:          %d\n",i);
			fprintf(stderr,"dbg5       year:             %d\n",
				data->profile[i].year);
			fprintf(stderr,"dbg5       month:            %d\n",
				data->profile[i].month);
			fprintf(stderr,"dbg5       day:              %d\n",
				data->profile[i].day);
			fprintf(stderr,"dbg5       hour:             %d\n",
				data->profile[i].hour);
			fprintf(stderr,"dbg5       minute:           %d\n",
				data->profile[i].minute);
			fprintf(stderr,"dbg5       sec:              %d\n",
				data->profile[i].second);
			fprintf(stderr,"dbg5       hundredth_sec:    %d\n",
				data->profile[i].hundredth_sec);
			fprintf(stderr,"dbg5       thousandth_sec:   %d\n",
				data->profile[i].thousandth_sec);
			fprintf(stderr,"dbg5       latitude:         %d\n",
				data->profile[i].latitude);
			fprintf(stderr,"dbg5       longitude:        %d\n",
				data->profile[i].longitude);
			fprintf(stderr,"dbg5       roll:             %d\n",
				data->profile[i].roll);
			fprintf(stderr,"dbg5       pitch:            %d\n",
				data->profile[i].pitch);
			fprintf(stderr,"dbg5       heading:          %d\n",
				data->profile[i].heading);
			fprintf(stderr,"dbg5       heave:            %d\n",
				data->profile[i].heave);
			for (j=0;j<8;j++)
				{
				fprintf(stderr,"dbg5       bath:             %d\n",
					data->profile[i].bath[j]);
				fprintf(stderr,"dbg5       bath_acrosstrack: %d\n",
					data->profile[i].bath_acrosstrack[j]);
				fprintf(stderr,"dbg5       bath_alongtrack:  %d\n",
					data->profile[i].bath_alongtrack[j]);
				fprintf(stderr,"dbg5       tt:               %d\n",
					data->profile[i].tt[j]);
				fprintf(stderr,"dbg5       angle:            %d\n",
					data->profile[i].angle[j]);
				fprintf(stderr,"dbg5       quality:          %d\n",
					data->profile[i].quality[j]);
				fprintf(stderr,"dbg5       amp:              %d\n",
					data->profile[i].amp[j]);
				}
			fprintf(stderr,"dbg5       \n");
			}
		}

	/* write the record label */
	label = ELAC_XBATH40;
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
#ifndef BYTESWAPPED
		short_ptr = (short int *) &line[0];
		*short_ptr = (short int) data->ping_num;
		short_ptr = (short int *) &line[2];
		*short_ptr = (short int) data->sound_vel;
#else
		short_ptr = (short int *) &line[0];
		*short_ptr = (short int) 
			mb_swap_short((short int) data->ping_num);
		short_ptr = (short int *) &line[2];
		*short_ptr = (short int) 
			mb_swap_short((short int) data->sound_vel);
#endif
		line[4] = (char) data->mode;
		line[5] = (char) data->pulse_length;
		line[6] = (char) data->source_power;
		line[7] = (char) data->receiver_gain;
		for (i=0;i<data->profile_num;i++)
			{
			profile = &line[8+i*152];
			profile[0] = (char) data->profile[i].day;
			profile[1] = (char) data->profile[i].month;
			profile[2] = (char) data->profile[i].year;
			profile[3] = (char) data->profile[i].hour;
			profile[4] = (char) data->profile[i].minute;
			profile[5] = (char) data->profile[i].second;
			profile[6] = (char) data->profile[i].hundredth_sec;
			profile[7] = (char) data->profile[i].thousandth_sec;
#ifndef BYTESWAPPED
			int_ptr = (int *) &profile[8];
			*int_ptr = (int) data->profile[i].latitude;
			int_ptr = (int *) &profile[12];
			*int_ptr = (int) data->profile[i].longitude;
			short_ptr = (short int *) &profile[16];
			*short_ptr = (short int) data->profile[i].roll;
			short_ptr = (short int *) &profile[18];
			*short_ptr = (short int) data->profile[i].pitch;
			short_ptr = (short int *) &profile[20];
			*short_ptr = (short int)(unsigned short) data->profile[i].heading;
			short_ptr = (short int *) &profile[22];
			*short_ptr = (short int) data->profile[i].heave;
			for (j=0;j<8;j++)
				{
				beam = &profile[24 + 16*j];
				int_ptr = (int *) &beam[0];
				*int_ptr = (int) data->profile[i].bath[j];
				int_ptr = (int *) &beam[4];
				*int_ptr = (int) 
					data->profile[i].bath_acrosstrack[j];
				short_ptr = (short *) &beam[8];
				*short_ptr = (short int) 
					data->profile[i].bath_alongtrack[j];
				short_ptr = (short *) &beam[10];
				*short_ptr = (short int) data->profile[i].tt[j];
				short_ptr = (short *) &beam[12];
				*short_ptr = (short int) data->profile[i].angle[j];
				beam[14] = (char) data->profile[i].quality[j];
				beam[15] = (char) data->profile[i].amp[j];
				}
#else
			int_ptr = (int *) &profile[8];
			*int_ptr 
				= (int) mb_swap_int(data->profile[i].latitude);
			int_ptr = (int *) &profile[12];
			*int_ptr 
				= (int) mb_swap_int(data->profile[i].longitude);
			short_ptr = (short int *) &profile[16];
			*short_ptr = (short int) 
				mb_swap_short((short int) data->profile[i].roll);
			short_ptr = (short int *) &profile[18];
			*short_ptr = (short int) 
				mb_swap_short((short int) data->profile[i].pitch);
			short_ptr = (short int *) &profile[20];
			*short_ptr = (short int) 
				mb_swap_short((short int)(unsigned short) data->profile[i].heading);
			short_ptr = (short int *) &profile[22];
			*short_ptr = (short int) 
				mb_swap_short((short int) data->profile[i].heave);
			for (j=0;j<8;j++)
				{
				beam = &profile[24 + 16*j];
				int_ptr = (int *) &beam[0];
				*int_ptr = (int) 
					mb_swap_int(data->profile[i].bath[j]);
				int_ptr = (int *) &beam[4];
				*int_ptr = (int) 
					mb_swap_int(data->profile[i].bath_acrosstrack[j]);
				short_ptr = (short *) &beam[8];
				*short_ptr = (short int) 
					mb_swap_short(data->profile[i].bath_alongtrack[j]);
				short_ptr = (short *) &beam[10];
				*short_ptr = (short int) 
					mb_swap_short(data->profile[i].tt[j]);
				short_ptr = (short *) &beam[12];
				*short_ptr = (short int) 
					mb_swap_short(data->profile[i].angle[j]);
				beam[14] = (char) data->profile[i].quality[j];
				beam[15] = (char) data->profile[i].amp[j];
				}
#endif
			}
		line[ELAC_XBATH40_SIZE] = 0x03;
		line[ELAC_XBATH40_SIZE+1] = '\0';
		line[ELAC_XBATH40_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,ELAC_XBATH40_SIZE+3,mbfp);
		if (status != ELAC_XBATH40_SIZE+3)
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
int mbr_bchrxunb_wr_bath32(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_bchrxunb_wr_bath32";
	int	status = MB_SUCCESS;
	struct mbf_bchrxunb_struct *data;
	char	line[ELAC_XBATH32_SIZE+3];
	char	*profile;
	char	*beam;
	short int label;
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
	data = (struct mbf_bchrxunb_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       ping_num:         %d\n",data->ping_num);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       mode:             %d\n",data->mode);
		fprintf(stderr,"dbg5       pulse_length:     %d\n",
			data->pulse_length);
		fprintf(stderr,"dbg5       source_power:     %d\n",
			data->source_power);
		fprintf(stderr,"dbg5       receiver_gain:    %d\n",
			data->receiver_gain);
		fprintf(stderr,"dbg5       profile_num:      %d\n",
			data->profile_num);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",
			data->beams_bath);
		for (i=0;i<data->profile_num;i++)
			{
			fprintf(stderr,"dbg5       profile:          %d\n",i);
			fprintf(stderr,"dbg5       year:             %d\n",
				data->profile[i].year);
			fprintf(stderr,"dbg5       month:            %d\n",
				data->profile[i].month);
			fprintf(stderr,"dbg5       day:              %d\n",
				data->profile[i].day);
			fprintf(stderr,"dbg5       hour:             %d\n",
				data->profile[i].hour);
			fprintf(stderr,"dbg5       minute:           %d\n",
				data->profile[i].minute);
			fprintf(stderr,"dbg5       sec:              %d\n",
				data->profile[i].second);
			fprintf(stderr,"dbg5       hundredth_sec:    %d\n",
				data->profile[i].hundredth_sec);
			fprintf(stderr,"dbg5       thousandth_sec:   %d\n",
				data->profile[i].thousandth_sec);
			fprintf(stderr,"dbg5       latitude:         %d\n",
				data->profile[i].latitude);
			fprintf(stderr,"dbg5       longitude:        %d\n",
				data->profile[i].longitude);
			fprintf(stderr,"dbg5       roll:             %d\n",
				data->profile[i].roll);
			fprintf(stderr,"dbg5       pitch:            %d\n",
				data->profile[i].pitch);
			fprintf(stderr,"dbg5       heading:          %d\n",
				data->profile[i].heading);
			fprintf(stderr,"dbg5       heave:            %d\n",
				data->profile[i].heave);
			for (j=0;j<8;j++)
				{
				fprintf(stderr,"dbg5       bath:             %d\n",
					data->profile[i].bath[j]);
				fprintf(stderr,"dbg5       bath_acrosstrack: %d\n",
					data->profile[i].bath_acrosstrack[j]);
				fprintf(stderr,"dbg5       bath_alongtrack:  %d\n",
					data->profile[i].bath_alongtrack[j]);
				fprintf(stderr,"dbg5       tt:               %d\n",
					data->profile[i].tt[j]);
				fprintf(stderr,"dbg5       angle:            %d\n",
					data->profile[i].angle[j]);
				fprintf(stderr,"dbg5       quality:          %d\n",
					data->profile[i].quality[j]);
				fprintf(stderr,"dbg5       amp:              %d\n",
					data->profile[i].amp[j]);
				}
			fprintf(stderr,"dbg5       \n");
			}
		}

	/* write the record label */
	label = ELAC_XBATH32;
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
#ifndef BYTESWAPPED
		short_ptr = (short int *) &line[0];
		*short_ptr = (short int) data->ping_num;
		short_ptr = (short int *) &line[2];
		*short_ptr = (short int) data->sound_vel;
#else
		short_ptr = (short int *) &line[0];
		*short_ptr = (short int) 
			mb_swap_short((short int) data->ping_num);
		short_ptr = (short int *) &line[2];
		*short_ptr = (short int) 
			mb_swap_short((short int) data->sound_vel);
#endif
		line[4] = (char) data->mode;
		line[5] = (char) data->pulse_length;
		line[6] = (char) data->source_power;
		line[7] = (char) data->receiver_gain;
		for (i=0;i<data->profile_num;i++)
			{
			profile = &line[8+i*152];
			profile[0] = (char) data->profile[i].day;
			profile[1] = (char) data->profile[i].month;
			profile[2] = (char) data->profile[i].year;
			profile[3] = (char) data->profile[i].hour;
			profile[4] = (char) data->profile[i].minute;
			profile[5] = (char) data->profile[i].second;
			profile[6] = (char) data->profile[i].hundredth_sec;
			profile[7] = (char) data->profile[i].thousandth_sec;
#ifndef BYTESWAPPED
			int_ptr = (int *) &profile[8];
			*int_ptr = (int) data->profile[i].latitude;
			int_ptr = (int *) &profile[12];
			*int_ptr = (int) data->profile[i].longitude;
			short_ptr = (short int *) &profile[16];
			*short_ptr = (short int) data->profile[i].roll;
			short_ptr = (short int *) &profile[18];
			*short_ptr = (short int) data->profile[i].pitch;
			short_ptr = (short int *) &profile[20];
			*short_ptr = (short int)(unsigned short) data->profile[i].heading;
			short_ptr = (short int *) &profile[22];
			*short_ptr = (short int) data->profile[i].heave;
			for (j=0;j<8;j++)
				{
				beam = &profile[24 + 16*j];
				int_ptr = (int *) &beam[0];
				*int_ptr = (int) data->profile[i].bath[j];
				int_ptr = (int *) &beam[4];
				*int_ptr = (int) 
					data->profile[i].bath_acrosstrack[j];
				short_ptr = (short *) &beam[8];
				*short_ptr = (short int) 
					data->profile[i].bath_alongtrack[j];
				short_ptr = (short *) &beam[10];
				*short_ptr = (short int) data->profile[i].tt[j];
				short_ptr = (short *) &beam[12];
				*short_ptr = (short int) data->profile[i].angle[j];
				beam[14] = (char) data->profile[i].quality[j];
				beam[15] = (char) data->profile[i].amp[j];
				}
#else
			int_ptr = (int *) &profile[8];
			*int_ptr 
				= (int) mb_swap_int(data->profile[i].latitude);
			int_ptr = (int *) &profile[12];
			*int_ptr 
				= (int) mb_swap_int(data->profile[i].longitude);
			short_ptr = (short int *) &profile[16];
			*short_ptr = (short int) 
				mb_swap_short((short int) data->profile[i].roll);
			short_ptr = (short int *) &profile[18];
			*short_ptr = (short int) 
				mb_swap_short((short int) data->profile[i].pitch);
			short_ptr = (short int *) &profile[20];
			*short_ptr = (short int) 
				mb_swap_short((short int)(unsigned short) data->profile[i].heading);
			short_ptr = (short int *) &profile[22];
			*short_ptr = (short int) 
				mb_swap_short((short int) data->profile[i].heave);
			for (j=0;j<8;j++)
				{
				beam = &profile[24 + 16*j];
				int_ptr = (int *) &beam[0];
				*int_ptr = (int) 
					mb_swap_int(data->profile[i].bath[j]);
				int_ptr = (int *) &beam[4];
				*int_ptr = (int) 
					mb_swap_int(data->profile[i].bath_acrosstrack[j]);
				short_ptr = (short *) &beam[8];
				*short_ptr = (short int) 
					mb_swap_short(data->profile[i].bath_alongtrack[j]);
				short_ptr = (short *) &beam[10];
				*short_ptr = (short int) 
					mb_swap_short(data->profile[i].tt[j]);
				short_ptr = (short *) &beam[12];
				*short_ptr = (short int) 
					mb_swap_short(data->profile[i].angle[j]);
				beam[14] = (char) data->profile[i].quality[j];
				beam[15] = (char) data->profile[i].amp[j];
				}
#endif
			}
		line[ELAC_XBATH32_SIZE] = 0x03;
		line[ELAC_XBATH32_SIZE+1] = '\0';
		line[ELAC_XBATH32_SIZE+2] = '\0';

		/* write out data */
		status = fwrite(line,1,ELAC_XBATH32_SIZE+3,mbfp);
		if (status != ELAC_XBATH32_SIZE+3)
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
