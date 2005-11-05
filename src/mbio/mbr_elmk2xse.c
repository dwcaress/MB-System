/*--------------------------------------------------------------------
 *    The MB-system:	mbr_elmk2xse.c	3/27/99
 *	$Id: mbr_elmk2xse.c,v 5.4 2005-11-05 00:48:04 caress Exp $
 *
 *    Copyright (c) 1999, 2000 by
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
 * mbr_elmk2xse.c contains the functions for reading and writing
 * multibeam data in the ELMK2XSE format.  
 * These functions include:
 *   mbr_alm_elmk2xse	- allocate read/write memory
 *   mbr_dem_elmk2xse	- deallocate read/write memory
 *   mbr_rt_elmk2xse	- read and translate data
 *   mbr_wt_elmk2xse	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 27, 1999
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.3  2004/04/27 01:46:12  caress
 * Various updates of April 26, 2004.
 *
 * Revision 5.2  2001/03/22 20:45:56  caress
 * Trying to make 5.0.beta0...
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.2  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.1  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.0  1999/08/08  04:14:35  caress
 * Initial revision.
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
#include "../../include/mbsys_xse.h"
#include "../../include/mbf_elmk2xse.h"

/* include for byte swapping */
#include "../../include/mb_swap.h"

/* set up byte swapping scenario */
#ifdef BYTESWAPPED

#ifndef DATAINPCBYTEORDER
#define SWAPFLAG    MB_YES
#else
#define SWAPFLAG    MB_NO
#endif

#else

#ifdef DATAINPCBYTEORDER
#define SWAPFLAG    MB_YES
#else
#define SWAPFLAG    MB_NO
#endif

#endif

/* essential function prototypes */
int mbr_register_elmk2xse(int verbose, char *mbio_ptr, 
		int *error);
int mbr_info_elmk2xse(int verbose, 
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
int mbr_alm_elmk2xse(int verbose, char *mbio_ptr, int *error);
int mbr_dem_elmk2xse(int verbose, char *mbio_ptr, int *error);
int mbr_rt_elmk2xse(int verbose, char *mbio_ptr, char *store_ptr, int *error);
int mbr_wt_elmk2xse(int verbose, char *mbio_ptr, char *store_ptr, int *error);

/*--------------------------------------------------------------------*/
int mbr_register_elmk2xse(int verbose, char *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_elmk2xse.c,v 5.4 2005-11-05 00:48:04 caress Exp $";
	char	*function_name = "mbr_register_elmk2xse";
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
	status = mbr_info_elmk2xse(verbose, 
			&mb_io_ptr->system, 
			&mb_io_ptr->beams_bath_max, 
			&mb_io_ptr->beams_amp_max, 
			&mb_io_ptr->pixels_ss_max, 
			&mb_io_ptr->format_name, 
			&mb_io_ptr->system_name, 
			&mb_io_ptr->format_description, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_elmk2xse;
	mb_io_ptr->mb_io_format_free = &mbr_dem_elmk2xse; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_xse_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_xse_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_elmk2xse; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_elmk2xse; 
	mb_io_ptr->mb_io_dimensions = &mbsys_xse_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_xse_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_xse_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_xse_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_xse_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_xse_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = &mbsys_xse_extract_svp; 
	mb_io_ptr->mb_io_insert_svp = &mbsys_xse_insert_svp; 
	mb_io_ptr->mb_io_ttimes = &mbsys_xse_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_xse_copy; 
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
int mbr_info_elmk2xse(int verbose, 
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
	static char res_id[]="$Id: mbr_elmk2xse.c,v 5.4 2005-11-05 00:48:04 caress Exp $";
	char	*function_name = "mbr_info_elmk2xse";
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
	*system = MB_SYS_XSE;
	*beams_bath_max = 126;
	*beams_amp_max = 126;
	*pixels_ss_max = 2000;
	strncpy(format_name, "ELMK2XSE", MB_NAME_LENGTH);
	strncpy(system_name, "XSE", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_ELMK2XSE\nInformal Description: ELAC and SeaBeam multibeam\nAttributes:           151 beam bathymetry and amplitude,\n                      2000 pixels sidescan, \n                      binary, L-3 Communications ELAC Nautik.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_NAV;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*beamwidth_xtrack = 3.0;
	*beamwidth_ltrack = 3.0;

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
int mbr_alm_elmk2xse(int verbose, char *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_elmk2xse.c,v 5.4 2005-11-05 00:48:04 caress Exp $";
	char	*function_name = "mbr_alm_elmk2xse";
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
	mb_io_ptr->structure_size = sizeof(struct mbf_elmk2xse_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,MBF_ELMK2XSE_BUFFER_SIZE,
				&mb_io_ptr->hdr_comment,error);
	mbsys_xse_alloc(verbose,mbio_ptr,
				&mb_io_ptr->store_data,error);

	/* initialize everything to zeros */
	mbr_zero_elmk2xse(verbose,mb_io_ptr->raw_data,error);

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
int mbr_dem_elmk2xse(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_elmk2xse";
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
	status = mb_free(verbose,&mb_io_ptr->hdr_comment,error);

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
int mbr_zero_elmk2xse(int verbose, char *data_ptr, int *error)
{
	char	*function_name = "mbr_zero_elmk2xse";
	int	status = MB_SUCCESS;
	struct mbf_elmk2xse_struct *data;
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
	data = (struct mbf_elmk2xse_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		/* type of data record */
		data->kind = MB_DATA_NONE;  /* Survey, nav, Comment */
	
		/* parameter (ship frames) */
		data->par_source = 0;		/* sensor id */
		data->par_sec = 0;		/* sec since 1/1/1901 00:00 */
		data->par_usec = 0;		/* microseconds */
		data->par_roll_bias = 0.0;		/* radians */
		data->par_pitch_bias = 0.0;		/* radians */
		data->par_heading_bias = 0.0;	/* radians */
		data->par_time_delay = 0.0;		/* nav time lag, seconds */
		data->par_trans_x_port = 0.0;	/* port transducer x position, meters */
		data->par_trans_y_port = 0.0;	/* port transducer y position, meters */
		data->par_trans_z_port = 0.0;	/* port transducer z position, meters */
		data->par_trans_x_stbd = 0.0;	/* starboard transducer x position, meters */
		data->par_trans_y_stbd = 0.0;	/* starboard transducer y position, meters */
		data->par_trans_z_stbd = 0.0;	/* starboard transducer z position, meters */
		data->par_trans_err_port = 0.0;	/* port transducer rotation in roll direction, radians */
		data->par_trans_err_stbd = 0.0;	/* starboard transducer rotation in roll direction, radians */
		data->par_nav_x = 0.0;		/* navigation antenna x position, meters */
		data->par_nav_y = 0.0;		/* navigation antenna y position, meters */
		data->par_nav_z = 0.0;		/* navigation antenna z position, meters */
		data->par_hrp_x = 0.0;		/* motion sensor x position, meters */
		data->par_hrp_y = 0.0;		/* motion sensor y position, meters */
		data->par_hrp_z = 0.0;		/* motion sensor z position, meters */
	
		/* svp (sound velocity frames) */
		data->svp_source = 0;		/* sensor id */
		data->svp_sec = 0;		/* sec since 1/1/1901 00:00 */
		data->svp_usec = 0;		/* microseconds */
		data->svp_nsvp = 0;		/* number of depth values */
		data->svp_nctd = 0;		/* number of ctd values */
		data->svp_ssv = 0.0;				/* m/s */
		for (i=0;i<MBF_ELMK2XSE_MAXSVP;i++)
		    {
		    data->svp_depth[i] = 0.0;		/* m */
		    data->svp_velocity[i] = 0.0;	/* m/s */
		    data->svp_conductivity[i] = 0.0;	/* mmho/cm */
		    data->svp_salinity[i] = 0.0;	/* o/oo */
		    data->svp_temperature[i] = 0.0;	/* degree celcius */
		    data->svp_pressure[i] = 0.0;	/* bar */
		    }

		/* position (navigation frames) */
		data->nav_source = 0;		/* sensor id */
		data->nav_sec = 0;		/* sec since 1/1/1901 00:00 */
		data->nav_usec = 0;		/* microseconds */
		data->nav_quality = 0;
		data->nav_status = 0;
		data->nav_description_len = 0;
		for (i=0;i<MBF_ELMK2XSE_DESCRIPTION_LENGTH;i++)
		    data->nav_description[i] = 0;
		data->nav_x = 0.0;			/* eastings (m) or 
						    longitude (radians) */
		data->nav_y = 0.0;			/* northings (m) or 
						    latitude (radians) */
		data->nav_z = 0.0;			/* height (m) or 
						    ellipsoidal height (m) */
		data->nav_speed_ground = 0.0;	/* m/s */
		data->nav_course_ground = 0.0;	/* radians */
		data->nav_speed_water = 0.0;	/* m/s */
		data->nav_course_water = 0.0;	/* radians */
		
		/* survey depth (multibeam frames) */
		data->mul_frame = MB_NO;	/* boolean flag - mutlibeam frame read */
		data->mul_group_beam = MB_NO;	/* boolean flag - beam group read */
		data->mul_group_tt = MB_NO;	/* boolean flag - tt group read */
		data->mul_group_quality = MB_NO;/* boolean flag - quality group read */
		data->mul_group_amp = MB_NO;	/* boolean flag - amp group read */
		data->mul_group_delay = MB_NO;	/* boolean flag - delay group read */
		data->mul_group_lateral = MB_NO;/* boolean flag - lateral group read */
		data->mul_group_along = MB_NO;	/* boolean flag - along group read */
		data->mul_group_depth = MB_NO;	/* boolean flag - depth group read */
		data->mul_group_angle = MB_NO;	/* boolean flag - angle group read */
		data->mul_group_heave = MB_NO;	/* boolean flag - heave group read */
		data->mul_group_roll = MB_NO;	/* boolean flag - roll group read */
		data->mul_group_pitch = MB_NO;	/* boolean flag - pitch group read */
		data->mul_source = 0;		/* sensor id */
		data->mul_sec = 0;		/* sec since 1/1/1901 00:00 */
		data->mul_usec = 0;		/* microseconds */
		data->mul_ping = 0;		/* ping number */
		data->mul_frequency = 0.0;	/* transducer frequency (Hz) */
		data->mul_pulse = 0.0;		/* transmit pulse length (sec) */
		data->mul_power = 0.0;		/* transmit power (dB) */
		data->mul_bandwidth = 0.0;	/* receive bandwidth (Hz) */
		data->mul_sample = 0.0;		/* receive sample interval (sec) */
		data->mul_swath = 0.0;		/* swath width (radians) */
		data->mul_num_beams = 0;	/* number of beams */
		for (i=0;i<MBF_ELMK2XSE_MAXBEAMS;i++)
		    {
		    data->beams[i].tt = 0.0;
		    data->beams[i].delay = 0.0;
		    data->beams[i].lateral = 0.0;
		    data->beams[i].along = 0.0;
		    data->beams[i].depth = 0.0;
		    data->beams[i].angle = 0.0;
		    data->beams[i].heave = 0.0;
		    data->beams[i].roll = 0.0;
		    data->beams[i].pitch = 0.0;
		    data->beams[i].beam = i + 1;
		    data->beams[i].quality = 0;
		    data->beams[i].amplitude = 0;		    
		    }
		
		/* survey sidescan (sidescan frames) */
		data->sid_frame = MB_NO;	/* boolean flag - sidescan frame read */
		data->sid_source = 0;		/* sensor id */
		data->sid_sec = 0;		/* sec since 1/1/1901 00:00 */
		data->sid_usec = 0;		/* microseconds */
		data->sid_ping = 0;		/* ping number */
		data->sid_frequency = 0.0;		/* transducer frequency (Hz) */
		data->sid_pulse = 0.0;		/* transmit pulse length (sec) */
		data->sid_power = 0.0;		/* transmit power (dB) */
		data->sid_bandwidth = 0.0;		/* receive bandwidth (Hz) */
		data->sid_sample = 0.0;		/* receive sample interval (sec) */
		data->sid_bin_size = 0;		/* bin size in mm */
		data->sid_offset = 0;		/* lateral offset in mm */
		data->sid_num_pixels = 0;		/* number of pixels */
		for (i=0;i<MBF_ELMK2XSE_MAXPIXELS;i++)
		    data->ss[i] = 0; /* sidescan amplitude in dB */
	
		/* comment */
		for (i=0;i<MBF_ELMK2XSE_COMMENT_LENGTH;i++)
		    data->comment[i] = 0;
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
int mbr_rt_elmk2xse(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_elmk2xse";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_elmk2xse_struct *data;
	struct mbsys_xse_struct *store;
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
	data = (struct mbf_elmk2xse_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_xse_struct *) store_ptr;

	/* read next data from file */
	status = mbr_elmk2xse_rd_data(verbose,mbio_ptr,error);
/*fprintf(stderr, "read kind:%d\n", data->kind);
fprintf(stderr, "data->mul_frame:%d data->sid_frame:%d\n\n", 
data->mul_frame, data->sid_frame);*/

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;
	
	/* add nav records to list for interpolation */
	if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_NAV)
		{
		time_d = data->nav_sec 
			    - MBSYS_XSE_TIME_OFFSET
			    + 0.000001 * data->nav_usec;
		lon = RTD * data->nav_x;
		lat = RTD * data->nav_y;
		mb_navint_add(verbose, mbio_ptr, time_d, lon, lat, error);
		}

	/* interpolate navigation for survey pings if needed */
	if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_DATA
		&& mb_io_ptr->nfix >= 1)
		{
		time_d = data->nav_sec 
			    - MBSYS_XSE_TIME_OFFSET
			    + 0.000001 * data->nav_usec;
		heading = RTD * data->nav_course_ground;
		mb_navint_interp(verbose, mbio_ptr, time_d, heading, 0.0,
				    &lon, &lat, &speed, error);
		data->mul_x = lon;
		data->mul_y = lat;
		}

	/* translate values to hydrostar data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		store->kind = data->kind;

		/* ship frame */
		if (store->kind == MB_DATA_PARAMETER)
		    {
		    store->par_source = data->par_source;		/* sensor id */
		    store->par_sec = data->par_sec;		/* sec since 1/1/1901 00:00 */
		    store->par_usec = data->par_usec;		/* microseconds */
		    store->par_roll_bias = data->par_roll_bias;		/* radians */
		    store->par_pitch_bias = data->par_pitch_bias;		/* radians */
		    store->par_heading_bias = data->par_heading_bias;	/* radians */
		    store->par_time_delay = data->par_time_delay;		/* nav time lag, seconds */
		    store->par_trans_x_port = data->par_trans_x_port;	/* port transducer x position, meters */
		    store->par_trans_y_port = data->par_trans_y_port;	/* port transducer y position, meters */
		    store->par_trans_z_port = data->par_trans_z_port;	/* port transducer z position, meters */
		    store->par_trans_x_stbd = data->par_trans_x_stbd;	/* starboard transducer x position, meters */
		    store->par_trans_y_stbd = data->par_trans_y_stbd;	/* starboard transducer y position, meters */
		    store->par_trans_z_stbd = data->par_trans_z_stbd;	/* starboard transducer z position, meters */
		    store->par_trans_err_port = data->par_trans_err_port;	/* port transducer rotation in roll direction, radians */
		    store->par_trans_err_stbd = data->par_trans_err_stbd;	/* starboard transducer rotation in roll direction, radians */
		    store->par_nav_x = data->par_nav_x;		/* navigation antenna x position, meters */
		    store->par_nav_y = data->par_nav_y;		/* navigation antenna y position, meters */
		    store->par_nav_z = data->par_nav_z;		/* navigation antenna z position, meters */
		    store->par_hrp_x = data->par_hrp_x;		/* motion sensor x position, meters */
		    store->par_hrp_y = data->par_hrp_y;		/* motion sensor y position, meters */
		    store->par_hrp_z = data->par_hrp_z;		/* motion sensor z position, meters */
		    }

		/* position frame */
		if (store->kind == MB_DATA_NAV)
		    {		
		    store->nav_source = data->nav_source;		/* sensor id */
		    store->nav_sec = data->nav_sec;		/* sec since 1/1/1901 00:00 */
		    store->nav_usec = data->nav_usec;		/* microseconds */
		    store->nav_quality = data->nav_quality;
		    store->nav_status = data->nav_status;
		    store->nav_description_len = data->nav_description_len;
		    for (i=0;i<MBF_ELMK2XSE_DESCRIPTION_LENGTH;i++)
			store->nav_description[i] = data->nav_description[i];
		    store->nav_x = data->nav_x;			/* eastings (m) or 
							longitude (radians) */
		    store->nav_y = data->nav_y;			/* northings (m) or 
							latitude (radians) */
		    store->nav_z = data->nav_z;			/* height (m) or 
							ellipsoidal height (m) */
		    store->nav_speed_ground = data->nav_speed_ground;	/* m/s */
		    store->nav_course_ground = data->nav_course_ground;	/* radians */
		    store->nav_speed_water = data->nav_speed_water;	/* m/s */
		    store->nav_course_water = data->nav_course_water;	/* radians */
		    }

		/* svp frame */
		if (store->kind == MB_DATA_VELOCITY_PROFILE)
		    {		
		    store->svp_source = data->svp_source;		/* sensor id */
		    store->svp_sec = data->svp_sec;		/* sec since 1/1/1901 00:00 */
		    store->svp_usec = data->svp_usec;		/* microseconds */
		    store->svp_nsvp = data->svp_nsvp;		/* number of depth values */
		    store->svp_nctd = data->svp_nctd;		/* number of ctd values */
		    store->svp_ssv = data->svp_ssv;				/* m/s */
		    for (i=0;i<MBF_ELMK2XSE_MAXSVP;i++)
			{
			store->svp_depth[i] = data->svp_depth[i];		/* m */
			store->svp_velocity[i] = data->svp_velocity[i];	/* m/s */
			store->svp_conductivity[i] = data->svp_conductivity[i];	/* mmho/cm */
			store->svp_salinity[i] = data->svp_salinity[i];	/* o/oo */
			store->svp_temperature[i] = data->svp_temperature[i];	/* degree celcius */
			store->svp_pressure[i] = data->svp_pressure[i];	/* bar */
			}
		    }

		/* multibeam and sidescan frames */
		if (store->kind == MB_DATA_DATA)
		    {		
		    store->mul_frame = data->mul_frame;			/* boolean flag - multibeam frame read */
		    store->mul_group_beam = data->mul_group_beam;	/* boolean flag - beam group read */
		    store->mul_group_tt = data->mul_group_tt;		/* boolean flag - tt group read */
		    store->mul_group_quality = data->mul_group_quality;	/* boolean flag - quality group read */
		    store->mul_group_amp = data->mul_group_amp;		/* boolean flag - amp group read */
		    store->mul_group_delay = data->mul_group_delay;	/* boolean flag - delay group read */
		    store->mul_group_lateral = data->mul_group_lateral;	/* boolean flag - lateral group read */
		    store->mul_group_along = data->mul_group_along;	/* boolean flag - along group read */
		    store->mul_group_depth = data->mul_group_depth;	/* boolean flag - depth group read */
		    store->mul_group_angle = data->mul_group_angle;	/* boolean flag - angle group read */
		    store->mul_group_heave = data->mul_group_heave;	/* boolean flag - heave group read */
		    store->mul_group_roll = data->mul_group_roll;		/* boolean flag - roll group read */
		    store->mul_group_pitch = data->mul_group_pitch;	/* boolean flag - pitch group read */
		    store->mul_source = data->mul_source;		/* sensor id */
		    store->mul_sec = data->mul_sec;		/* sec since 1/1/1901 00:00 */
		    store->mul_usec = data->mul_usec;		/* microseconds */
		    store->mul_x = mb_io_ptr->new_lon;		/* interpolated longitude degrees */
		    store->mul_y = mb_io_ptr->new_lat;		/* interpolated latitude degrees */
		    store->mul_ping = data->mul_ping;		/* ping number */
		    store->mul_frequency = data->mul_frequency;	/* transducer frequency (Hz) */
		    store->mul_pulse = data->mul_pulse;		/* transmit pulse length (sec) */
		    store->mul_power = data->mul_power;		/* transmit power (dB) */
		    store->mul_bandwidth = data->mul_bandwidth;	/* receive bandwidth (Hz) */
		    store->mul_sample = data->mul_sample;		/* receive sample interval (sec) */
		    store->mul_swath = data->mul_swath;		/* swath width (radians) */
		    store->mul_num_beams = data->mul_num_beams;	/* number of beams */
		    for (i=0;i<MBF_ELMK2XSE_MAXBEAMS;i++)
			{
			store->beams[i].tt = data->beams[i].tt;
			store->beams[i].delay = data->beams[i].delay;
			store->beams[i].lateral = data->beams[i].lateral;
			store->beams[i].along = data->beams[i].along;
			store->beams[i].depth = data->beams[i].depth;
			store->beams[i].angle = data->beams[i].angle;
			store->beams[i].heave = data->beams[i].heave;
			store->beams[i].roll = data->beams[i].roll;
			store->beams[i].pitch = data->beams[i].pitch;
			store->beams[i].beam = data->beams[i].beam;
			store->beams[i].quality = data->beams[i].quality;
			store->beams[i].amplitude = data->beams[i].amplitude;		    
			}
		    store->sid_frame = data->sid_frame;			/* boolean flag - sidescan frame read */
		    store->sid_source = data->sid_source;		/* sensor id */
		    store->sid_sec = data->sid_sec;		/* sec since 1/1/1901 00:00 */
		    store->sid_usec = data->sid_usec;		/* microseconds */
		    store->sid_ping = data->sid_ping;		/* ping number */
		    store->sid_frequency = data->sid_frequency;		/* transducer frequency (Hz) */
		    store->sid_pulse = data->sid_pulse;		/* transmit pulse length (sec) */
		    store->sid_power = data->sid_power;		/* transmit power (dB) */
		    store->sid_bandwidth = data->sid_bandwidth;		/* receive bandwidth (Hz) */
		    store->sid_sample = data->sid_sample;		/* receive sample interval (sec) */
		    store->sid_bin_size = data->sid_bin_size;		/* bin size in mm */
		    store->sid_offset = data->sid_offset;		/* lateral offset in mm */
		    store->sid_num_pixels = data->sid_num_pixels;		/* number of pixels */
		    for (i=0;i<MBF_ELMK2XSE_MAXPIXELS;i++)
			store->ss[i] = data->ss[i]; /* sidescan amplitude in dB */
		    }
		    
		/* comment */
		if (store->kind == MB_DATA_COMMENT)
		    {
		    for (i=0;i<MBF_ELMK2XSE_COMMENT_LENGTH;i++)
			    store->comment[i] = data->comment[i];
		    }
		    
		/* unsupported frame */
		if (store->kind == MB_DATA_RAW_LINE)
		    {
		    store->rawsize = data->rawsize;
		    for (i=0;i<data->rawsize;i++)
			store->raw[i] = data->raw[i];

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
int mbr_wt_elmk2xse(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_elmk2xse";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_elmk2xse_struct *data;
	char	*data_ptr;
	struct mbsys_xse_struct *store;
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
	data = (struct mbf_elmk2xse_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	store = (struct mbsys_xse_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		data->kind = store->kind;

		/* ship frame */
		if (data->kind == MB_DATA_PARAMETER)
		    {
		    data->par_source = store->par_source;		/* sensor id */
		    data->par_sec = store->par_sec;		/* sec since 1/1/1901 00:00 */
		    data->par_usec = store->par_usec;		/* microseconds */
		    data->par_roll_bias = store->par_roll_bias;		/* radians */
		    data->par_pitch_bias = store->par_pitch_bias;		/* radians */
		    data->par_heading_bias = store->par_heading_bias;	/* radians */
		    data->par_time_delay = store->par_time_delay;		/* nav time lag, seconds */
		    data->par_trans_x_port = store->par_trans_x_port;	/* port transducer x position, meters */
		    data->par_trans_y_port = store->par_trans_y_port;	/* port transducer y position, meters */
		    data->par_trans_z_port = store->par_trans_z_port;	/* port transducer z position, meters */
		    data->par_trans_x_stbd = store->par_trans_x_stbd;	/* starboard transducer x position, meters */
		    data->par_trans_y_stbd = store->par_trans_y_stbd;	/* starboard transducer y position, meters */
		    data->par_trans_z_stbd = store->par_trans_z_stbd;	/* starboard transducer z position, meters */
		    data->par_trans_err_port = store->par_trans_err_port;	/* port transducer rotation in roll direction, radians */
		    data->par_trans_err_stbd = store->par_trans_err_stbd;	/* starboard transducer rotation in roll direction, radians */
		    data->par_nav_x = store->par_nav_x;		/* navigation antenna x position, meters */
		    data->par_nav_y = store->par_nav_y;		/* navigation antenna y position, meters */
		    data->par_nav_z = store->par_nav_z;		/* navigation antenna z position, meters */
		    data->par_hrp_x = store->par_hrp_x;		/* motion sensor x position, meters */
		    data->par_hrp_y = store->par_hrp_y;		/* motion sensor y position, meters */
		    data->par_hrp_z = store->par_hrp_z;		/* motion sensor z position, meters */
		    }

		/* position frame */
		if (data->kind == MB_DATA_NAV)
		    {		
		    data->nav_source = store->nav_source;		/* sensor id */
		    data->nav_sec = store->nav_sec;		/* sec since 1/1/1901 00:00 */
		    data->nav_usec = store->nav_usec;		/* microseconds */
		    data->nav_quality = store->nav_quality;
		    data->nav_status = store->nav_status;
		    data->nav_description_len = store->nav_description_len;
		    for (i=0;i<MBF_ELMK2XSE_DESCRIPTION_LENGTH;i++)
			data->nav_description[i] = store->nav_description[i];
		    data->nav_x = store->nav_x;			/* eastings (m) or 
							longitude (radians) */
		    data->nav_y = store->nav_y;			/* northings (m) or 
							latitude (radians) */
		    data->nav_z = store->nav_z;			/* height (m) or 
							ellipsoidal height (m) */
		    data->nav_speed_ground = store->nav_speed_ground;	/* m/s */
		    data->nav_course_ground = store->nav_course_ground;	/* radians */
		    data->nav_speed_water = store->nav_speed_water;	/* m/s */
		    data->nav_course_water = store->nav_course_water;	/* radians */
		    }

		/* svp frame */
		if (data->kind == MB_DATA_VELOCITY_PROFILE)
		    {		
		    data->svp_source = store->svp_source;		/* sensor id */
		    data->svp_sec = store->svp_sec;		/* sec since 1/1/1901 00:00 */
		    data->svp_usec = store->svp_usec;		/* microseconds */
		    data->svp_nsvp = store->svp_nsvp;		/* number of depth values */
		    data->svp_nctd = store->svp_nctd;		/* number of ctd values */
		    data->svp_ssv = store->svp_ssv;				/* m/s */
		    for (i=0;i<MBF_ELMK2XSE_MAXSVP;i++)
			{
			data->svp_depth[i] = store->svp_depth[i];		/* m */
			data->svp_velocity[i] = store->svp_velocity[i];	/* m/s */
			data->svp_conductivity[i] = store->svp_conductivity[i];	/* mmho/cm */
			data->svp_salinity[i] = store->svp_salinity[i];	/* o/oo */
			data->svp_temperature[i] = store->svp_temperature[i];	/* degree celcius */
			data->svp_pressure[i] = store->svp_pressure[i];	/* bar */
			}
		    }

		/* multibeam and sidescan frames */
		if (data->kind == MB_DATA_DATA)
		    {		
		    data->mul_frame = store->mul_frame;			/* boolean flag - multibeam frame read */
		    data->mul_group_beam = store->mul_group_beam;	/* boolean flag - beam group read */
		    data->mul_group_tt = store->mul_group_tt;		/* boolean flag - tt group read */
		    data->mul_group_quality = store->mul_group_quality;	/* boolean flag - quality group read */
		    data->mul_group_amp = store->mul_group_amp;		/* boolean flag - amp group read */
		    data->mul_group_delay = store->mul_group_delay;	/* boolean flag - delay group read */
		    data->mul_group_lateral = store->mul_group_lateral;	/* boolean flag - lateral group read */
		    data->mul_group_along = store->mul_group_along;	/* boolean flag - along group read */
		    data->mul_group_depth = store->mul_group_depth;	/* boolean flag - depth group read */
		    data->mul_group_angle = store->mul_group_angle;	/* boolean flag - angle group read */
		    data->mul_group_heave = store->mul_group_heave;	/* boolean flag - heave group read */
		    data->mul_group_roll = store->mul_group_roll;		/* boolean flag - roll group read */
		    data->mul_group_pitch = store->mul_group_pitch;	/* boolean flag - pitch group read */
		    data->mul_source = store->mul_source;		/* sensor id */
		    data->mul_sec = store->mul_sec;		/* sec since 1/1/1901 00:00 */
		    data->mul_usec = store->mul_usec;		/* microseconds */
		    data->mul_ping = store->mul_ping;		/* ping number */
		    data->mul_frequency = store->mul_frequency;	/* transducer frequency (Hz) */
		    data->mul_pulse = store->mul_pulse;		/* transmit pulse length (sec) */
		    data->mul_power = store->mul_power;		/* transmit power (dB) */
		    data->mul_bandwidth = store->mul_bandwidth;	/* receive bandwidth (Hz) */
		    data->mul_sample = store->mul_sample;		/* receive sample interval (sec) */
		    data->mul_swath = store->mul_swath;		/* swath width (radians) */
		    data->mul_num_beams = store->mul_num_beams;	/* number of beams */
		    for (i=0;i<MBF_ELMK2XSE_MAXBEAMS;i++)
			{
			data->beams[i].tt = store->beams[i].tt;
			data->beams[i].delay = store->beams[i].delay;
			data->beams[i].lateral = store->beams[i].lateral;
			data->beams[i].along = store->beams[i].along;
			data->beams[i].depth = store->beams[i].depth;
			data->beams[i].angle = store->beams[i].angle;
			data->beams[i].heave = store->beams[i].heave;
			data->beams[i].roll = store->beams[i].roll;
			data->beams[i].pitch = store->beams[i].pitch;
			data->beams[i].beam = store->beams[i].beam;
			data->beams[i].quality = store->beams[i].quality;
			data->beams[i].amplitude = store->beams[i].amplitude;		    
			}
		    data->sid_frame = store->sid_frame;			/* boolean flag - sidescan frame read */
		    data->sid_source = store->sid_source;		/* sensor id */
		    data->sid_sec = store->sid_sec;		/* sec since 1/1/1901 00:00 */
		    data->sid_usec = store->sid_usec;		/* microseconds */
		    data->sid_ping = store->sid_ping;		/* ping number */
		    data->sid_frequency = store->sid_frequency;		/* transducer frequency (Hz) */
		    data->sid_pulse = store->sid_pulse;		/* transmit pulse length (sec) */
		    data->sid_power = store->sid_power;		/* transmit power (dB) */
		    data->sid_bandwidth = store->sid_bandwidth;		/* receive bandwidth (Hz) */
		    data->sid_sample = store->sid_sample;		/* receive sample interval (sec) */
		    data->sid_bin_size = store->sid_bin_size;		/* bin size in mm */
		    data->sid_offset = store->sid_offset;		/* lateral offset in mm */
		    data->sid_num_pixels = store->sid_num_pixels;		/* number of pixels */
		    for (i=0;i<MBF_ELMK2XSE_MAXPIXELS;i++)
			data->ss[i] = store->ss[i]; /* sidescan amplitude in dB */
		    }
		    
		/* comment */
		if (data->kind == MB_DATA_COMMENT)
		    {
		    for (i=0;i<MBF_ELMK2XSE_COMMENT_LENGTH;i++)
			    data->comment[i] = store->comment[i];
		    }
		    
		/* unsupported frame */
		if (data->kind == MB_DATA_RAW_LINE)
		    {
		    data->rawsize = store->rawsize;
		    for (i=0;i<store->rawsize;i++)
			data->raw[i] = store->raw[i];
		    }
		}

	/* write next data to file */
	status = mbr_elmk2xse_wr_data(verbose,mbio_ptr,data_ptr,error);

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
int mbr_elmk2xse_rd_data(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_elmk2xse_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_elmk2xse_struct *data;
	char	*data_ptr;
	FILE	*mbfp;
	static char label[4];
	int	done;
	int	frame_id;
	int	frame_source;
	unsigned int	frame_sec;
	unsigned int	frame_usec;
	int	buffer_size;
	int	*frame_save;
	int	*frame_expect;
	int	*frame_id_save;
	int	*frame_source_save;
	unsigned int	*frame_sec_save;
	unsigned int	*frame_usec_save;
	int	*buffer_size_save;
	char	*buffer;
	int	index;
	int	read_len;
	int	i, j;

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
	data = (struct mbf_elmk2xse_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	mbfp = mb_io_ptr->mbfp;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
	
	/* read until done */
	frame_expect = &mb_io_ptr->save1;
	frame_save = &mb_io_ptr->save2;
	frame_id_save = &mb_io_ptr->save3;
	frame_source_save = &mb_io_ptr->save4;
	frame_sec_save = (unsigned int *) &mb_io_ptr->save5;
	frame_usec_save = (unsigned int *) &mb_io_ptr->save6;
	buffer_size_save = &mb_io_ptr->save7;
	buffer = mb_io_ptr->hdr_comment;
	done = MB_NO;
	if (*frame_save == MB_YES)
	    {
	    data->mul_frame = MB_NO;
	    data->sid_frame = MB_NO;
	    }
	while (done == MB_NO)
	    {
	    /* use saved frame if available */
	    if (*frame_save == MB_YES)
		{
		frame_id = *frame_id_save;
		frame_source = *frame_source_save;
		frame_sec = *frame_sec_save;
		frame_usec = *frame_usec_save;
		buffer_size = *buffer_size_save;
		*frame_save = MB_NO;
		}
		
	    /* else read from file */
	    else
		{
		
		/* look for the next frame start */
		if ((read_len = fread(&label[0],1,4,mb_io_ptr->mbfp)) != 4)
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }
#ifdef DATAINPCBYTEORDER
		while (status == MB_SUCCESS
		    && strncmp(label, "FSH$", 4) != 0)
#else
		while (status == MB_SUCCESS
		    && strncmp(label, "$HSF", 4) != 0)
#endif
		    {
		    /* get next byte */
		    for (i=0;i<3;i++)
			label[i] = label[i+1];
		    if ((read_len = fread(&label[3],1,1,mb_io_ptr->mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		    }
	
		/* get byte count */
		if (status == MB_SUCCESS)
		    {
		    if ((read_len = fread(buffer,1,4,mb_io_ptr->mbfp)) != 4)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		    else
			{
			mb_get_binary_int(SWAPFLAG, buffer, &buffer_size);
			if (buffer_size > 0
			    && buffer_size <= MBF_ELMK2XSE_BUFFER_SIZE - 4)
			    buffer_size += 4;
			else
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    }
			}
		    }
	
		/* read entire data record into buffer */
		if (status == MB_SUCCESS)
		    {
		    if ((read_len = fread(buffer,1,
			buffer_size,mb_io_ptr->mbfp)) != buffer_size)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		    }

		/* parse header values */
		if (status == MB_SUCCESS)
		    {
		    /* get frame id, source, and time */
		    index = 0;
		    mb_get_binary_int(SWAPFLAG, &buffer[index], &frame_id); index += sizeof(int);
		    mb_get_binary_int(SWAPFLAG, &buffer[index], &frame_source); index += sizeof(int);
		    mb_get_binary_int(SWAPFLAG, &buffer[index], &frame_sec); index += sizeof(int);
		    mb_get_binary_int(SWAPFLAG, &buffer[index], &frame_usec); index += sizeof(int);
		    }
		}
		
	    /* parse data if possible */
	    if (status == MB_SUCCESS)
		{
/*fprintf(stderr, "frame_expect:%d frame_id:%d frame_source:%d frame_sec:%u frame_usec:%u\n", 
*frame_expect, frame_id, frame_source, frame_sec, frame_usec);*/
		if (frame_id == MBF_ELMK2XSE_NAV_FRAME)
		    {
/*fprintf(stderr, "READ NAV\n");*/
		    /* read extra 32 bytes to handle bug in Hydrostar software */
		    if (buffer_size < 124)
			{
			if ((read_len = fread(&buffer[buffer_size],1,
			    32,mb_io_ptr->mbfp)) == 32)
			    {
			    buffer_size += 32;
			    }
			}
			
		    data->kind = MB_DATA_NAV;
		    status = mbr_elmk2xse_rd_nav(
				    verbose,buffer_size,
				    buffer,data,error);
		    done = MB_YES;
		    }
		else if (frame_id == MBF_ELMK2XSE_SVP_FRAME)
		    {
/*fprintf(stderr, "READ SVP\n");*/
		    data->kind = MB_DATA_VELOCITY_PROFILE;
		    status = mbr_elmk2xse_rd_svp(
				    verbose,buffer_size,
				    buffer,data,error);
		    done = MB_YES;
		    }
		else if (frame_id == MBF_ELMK2XSE_SHP_FRAME)
		    {
/*fprintf(stderr, "READ PARAMETER\n");*/
		    data->kind = MB_DATA_PARAMETER;
		    status = mbr_elmk2xse_rd_ship(
				    verbose,buffer_size,
				    buffer,data,error);
		    done = MB_YES;
		    }
		else if (frame_id == MBF_ELMK2XSE_COM_FRAME)
		    {
/*fprintf(stderr, "READ COMMENT\n");*/
		    data->kind = MB_DATA_COMMENT;
		    status = mbr_elmk2xse_rd_comment(
				    verbose,buffer_size,
				    buffer,data,error);
		    done = MB_YES;
		    }
		else if (*frame_expect != MBF_ELMK2XSE_NONE_FRAME
		    && frame_id != *frame_expect)
		    {
/*fprintf(stderr, "READ NOTHING - SAVE HEADER\n");*/
		    data->kind = MB_DATA_DATA;
		    *frame_save = MB_YES;
		    *frame_id_save = frame_id;
		    *frame_source_save = frame_source;
		    *frame_sec_save = frame_sec;
		    *frame_usec_save = frame_usec;
		    *buffer_size_save = buffer_size;
		    *frame_expect = MBF_ELMK2XSE_NONE_FRAME;
		    done = MB_YES;
		    }
		else if (frame_id == MBF_ELMK2XSE_SSN_FRAME)
		    {
/*fprintf(stderr, "READ SIDESCAN\n");*/
		    data->kind = MB_DATA_DATA;
		    status = mbr_elmk2xse_rd_sidescan(
				    verbose,buffer_size,
				    buffer,data,error);
		    data->sid_frame = MB_YES;
		    *frame_expect = MBF_ELMK2XSE_MBM_FRAME;
		    done = MB_NO;
		    }
		else if (frame_id == MBF_ELMK2XSE_MBM_FRAME)
		    {
/*fprintf(stderr, "READ MULTIBEAM\n");*/
		    data->kind = MB_DATA_DATA;
		    status = mbr_elmk2xse_rd_multibeam(
				    verbose,buffer_size,
				    buffer,data,error);
		    data->mul_frame = MB_YES;
		    if (frame_id == *frame_expect)
			*frame_expect = MBF_ELMK2XSE_NONE_FRAME;
		    done = MB_YES;
		    }
		else if (frame_id == MBF_ELMK2XSE_MBM_FRAME)
		    {
/*fprintf(stderr, "READ OTHER\n");*/
		    data->kind = MB_DATA_RAW_LINE;
		    data->rawsize = buffer_size;
		    for (i=0;i<buffer_size;i++)
			data->raw[i] = buffer[i];
		    done = MB_YES;
		    }
		}
		
	    /* check for status */
	    if (status == MB_FAILURE)
		{
		done = MB_YES;
		*frame_save = MB_NO;
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
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_elmk2xse_rd_nav(int verbose, int buffer_size, char *buffer, 
		struct mbf_elmk2xse_struct *data, int *error)
{
	char	*function_name = "mbr_elmk2xse_rd_nav";
	int	status = MB_SUCCESS;
	int	byte_count;
	int	group_id;
	int	done;
	int	index;
	int	nchar;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		}
		
	/* get source and time */
	index = 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->nav_source); index += sizeof(int);
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->nav_sec); index += sizeof(int);
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->nav_usec); index += sizeof(int);
		
	/* loop over groups */
	done = MB_NO;
	while (index <= buffer_size 
		&& status == MB_SUCCESS
		&& done == MB_NO)
	    {
	    /* look for group start or frame end */
#ifdef DATAINPCBYTEORDER
	    while (index < buffer_size - 4
		&& strncmp(&buffer[index], "GSH$", 4) != 0
		&& strncmp(&buffer[index], "FSH#", 4) != 0)
		index++;
	    if (index >= buffer_size - 4
		|| strncmp(&buffer[index], "FSH#", 4) == 0)
		done = MB_YES;
	    else
		index += 4;
#else
	    while (index < buffer_size
		&& strncmp(&buffer[index], "$HSG", 4) != 0
		&& strncmp(&buffer[index], "#HSF", 4) != 0)
		index++;
	    if (index >= buffer_size
		|| strncmp(&buffer[index], "#HSF", 4) == 0)
		done = MB_YES;
	    else
		index += 4;
#endif
	    
	    /* deal with group */
	    if (done == MB_NO)
		{
		/* get group size and id */
		mb_get_binary_int(SWAPFLAG, &buffer[index], &byte_count); index += sizeof(int);
		mb_get_binary_int(SWAPFLAG, &buffer[index], &group_id); index += sizeof(int);

		/* print debug statements */
		if (verbose >= 5)
		    {
		    fprintf(stderr,"\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n",
			    group_id, byte_count, function_name);
		    }
		}
	    
	    /* handle general group */
	    if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_NAV_GROUP_GEN)
		{
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->nav_source); index += sizeof(int);
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->nav_quality); index += sizeof(int);
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->nav_status); index += sizeof(int);
		}
	    
	    /* handle point group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_NAV_GROUP_POS)
		{
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->nav_description_len); index += sizeof(int);
		for (i=0;i<data->nav_description_len;i++)
		    {
		    data->nav_description[i] = buffer[index];
		    index++;
		    }
		data->nav_description[data->nav_description_len] = 0;
		mb_get_binary_double(SWAPFLAG, &buffer[index], &data->nav_x); index += sizeof(double);
		mb_get_binary_double(SWAPFLAG, &buffer[index], &data->nav_y); index += sizeof(double);
		mb_get_binary_double(SWAPFLAG, &buffer[index], &data->nav_z); index += sizeof(double);
		}
	    
	    /* handle motion ground truth group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_NAV_GROUP_MOTIONGT)
		{
		mb_get_binary_double(SWAPFLAG, &buffer[index], &data->nav_speed_ground); index += sizeof(double);
		mb_get_binary_double(SWAPFLAG, &buffer[index], &data->nav_course_ground); index += sizeof(double);
		}
	    
	    /* handle motion through water group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_NAV_GROUP_MOTIONTW)
		{
		mb_get_binary_double(SWAPFLAG, &buffer[index], &data->nav_speed_water); index += sizeof(double);
		mb_get_binary_double(SWAPFLAG, &buffer[index], &data->nav_course_water); index += sizeof(double);
		}
	    
	    /* handle motion through water group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_NAV_GROUP_TRACK)
		{
		index += byte_count - 4;
		}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       nav_source:          %d\n",data->nav_source);
	    fprintf(stderr,"dbg5       nav_sec:             %u\n",data->nav_sec);
	    fprintf(stderr,"dbg5       nav_usec:            %u\n",data->nav_usec);
	    fprintf(stderr,"dbg5       nav_quality:         %d\n",data->nav_quality);
	    fprintf(stderr,"dbg5       nav_status:          %d\n",data->nav_status);
	    fprintf(stderr,"dbg5       nav_description_len: %d\n",data->nav_description_len);
	    fprintf(stderr,"dbg5       nav_description:     %s\n",data->nav_description);
	    fprintf(stderr,"dbg5       nav_x:               %f\n",data->nav_x);
	    fprintf(stderr,"dbg5       nav_y:               %f\n",data->nav_y);
	    fprintf(stderr,"dbg5       nav_z:               %f\n",data->nav_z);
	    fprintf(stderr,"dbg5       nav_speed_ground:    %f\n",data->nav_speed_ground);
	    fprintf(stderr,"dbg5       nav_course_ground:   %f\n",data->nav_course_ground);
	    fprintf(stderr,"dbg5       nav_speed_water:     %f\n",data->nav_speed_water);
	    fprintf(stderr,"dbg5       nav_course_water:    %f\n",data->nav_course_water);
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
int mbr_elmk2xse_rd_svp(int verbose, int buffer_size, char *buffer, 
		struct mbf_elmk2xse_struct *data, int *error)
{
	char	*function_name = "mbr_elmk2xse_rd_svp";
	int	status = MB_SUCCESS;
	int	byte_count;
	int	group_id;
	int	done;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		}
		
	/* get source and time */
	index = 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->svp_source); index += sizeof(int);
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->svp_sec); index += sizeof(int);
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->svp_usec); index += sizeof(int);
		
	/* loop over groups */
	done = MB_NO;
	while (index <= buffer_size 
		&& status == MB_SUCCESS
		&& done == MB_NO)
	    {
	    /* look for group start or frame end */
#ifdef DATAINPCBYTEORDER
	    while (index < buffer_size - 4
		&& strncmp(&buffer[index], "GSH$", 4) != 0
		&& strncmp(&buffer[index], "FSH#", 4) != 0)
		index++;
	    if (index >= buffer_size - 4
		|| strncmp(&buffer[index], "FSH#", 4) == 0)
		done = MB_YES;
	    else
		index += 4;
#else
	    while (index < buffer_size
		&& strncmp(&buffer[index], "$HSG", 4) != 0
		&& strncmp(&buffer[index], "#HSF", 4) != 0)
		index++;
	    if (index >= buffer_size
		|| strncmp(&buffer[index], "#HSF", 4) == 0)
		done = MB_YES;
	    else
		index += 4;
#endif
	    
	    /* deal with group */
	    if (done == MB_NO)
		{
		/* get group size and id */
		mb_get_binary_int(SWAPFLAG, &buffer[index], &byte_count); index += sizeof(int);
		mb_get_binary_int(SWAPFLAG, &buffer[index], &group_id); index += sizeof(int);

		/* print debug statements */
		if (verbose >= 5)
		    {
		    fprintf(stderr,"\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n",
			    group_id, byte_count, function_name);
		    }
		}
	    
	    /* handle depth group */
	    if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_SVP_GROUP_DEPTH)
		{
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->svp_nsvp); index += sizeof(int);
		for (i=0;i<data->svp_nsvp;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXSVP)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->svp_depth[i]);
		    index += sizeof(double);
		    }
		}
	    
	    /* handle velocity group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_SVP_GROUP_VELOCITY)
		{
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->svp_nsvp); index += sizeof(int);
		for (i=0;i<data->svp_nsvp;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXSVP)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->svp_velocity[i]);
		    index += sizeof(double);
		    }
		}
	    
	    /* handle conductivity group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_SVP_GROUP_CONDUCTIVITY)
		{
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->svp_nctd); index += sizeof(int);
		for (i=0;i<data->svp_nctd;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXSVP)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->svp_conductivity[i]);
		    index += sizeof(double);
		    }
		}
	    
	    /* handle salinity group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_SVP_GROUP_SALINITY)
		{
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->svp_nctd); index += sizeof(int);
		for (i=0;i<data->svp_nctd;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXSVP)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->svp_salinity[i]);
		    index += sizeof(double);
		    }
		}
	    
	    /* handle temperature group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_SVP_GROUP_TEMP)
		{
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->svp_nctd); index += sizeof(int);
		for (i=0;i<data->svp_nctd;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXSVP)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->svp_temperature[i]);
		    index += sizeof(double);
		    }
		}
	    
	    /* handle pressure group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_SVP_GROUP_PRESSURE)
		{
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->svp_nctd); index += sizeof(int);
		for (i=0;i<data->svp_nctd;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXSVP)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->svp_pressure[i]);
		    index += sizeof(double);
		    }
		}
	    
	    /* handle ssv group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_SVP_GROUP_SSV)
		{
		mb_get_binary_double(SWAPFLAG, &buffer[index], &data->svp_ssv);
		}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       svp_source:          %d\n",data->svp_source);
	    fprintf(stderr,"dbg5       svp_sec:             %u\n",data->svp_sec);
	    fprintf(stderr,"dbg5       svp_usec:            %u\n",data->svp_usec);
	    fprintf(stderr,"dbg5       svp_nsvp:            %d\n",data->svp_nsvp);
	    fprintf(stderr,"dbg5       svp_nctd:            %d\n",data->svp_nctd);
	    fprintf(stderr,"dbg5       svp_ssv:             %f\n",data->svp_ssv);
	    for (i=0;i<data->svp_nsvp;i++)
		fprintf(stderr,"dbg5       svp[%d]:	        %f %f\n",
		    i, data->svp_depth[i], data->svp_velocity[i]);
	    for (i=0;i<data->svp_nctd;i++)
		fprintf(stderr,"dbg5       cstd[%d]:        %f %f %f %f\n",
		    i, data->svp_conductivity[i], data->svp_salinity[i], 
		    data->svp_temperature[i], data->svp_pressure[i]);
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
int mbr_elmk2xse_rd_ship(int verbose, int buffer_size, char *buffer, 
		struct mbf_elmk2xse_struct *data, int *error)
{
	char	*function_name = "mbr_elmk2xse_rd_ship";
	int	status = MB_SUCCESS;
	int	byte_count;
	int	group_id;
	int	done;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		}
		
	/* get source and time */
	index = 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->par_source); index += sizeof(int);
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->par_sec); index += sizeof(int);
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->par_usec); index += sizeof(int);
		
	/* loop over groups */
	done = MB_NO;
	while (index <= buffer_size 
		&& status == MB_SUCCESS
		&& done == MB_NO)
	    {
	    /* look for group start or frame end */
#ifdef DATAINPCBYTEORDER
	    while (index < buffer_size - 4
		&& strncmp(&buffer[index], "GSH$", 4) != 0
		&& strncmp(&buffer[index], "FSH#", 4) != 0)
		index++;
	    if (index >= buffer_size - 4
		|| strncmp(&buffer[index], "FSH#", 4) == 0)
		done = MB_YES;
	    else
		index += 4;
#else
	    while (index < buffer_size
		&& strncmp(&buffer[index], "$HSG", 4) != 0
		&& strncmp(&buffer[index], "#HSF", 4) != 0)
		index++;
	    if (index >= buffer_size
		|| strncmp(&buffer[index], "#HSF", 4) == 0)
		done = MB_YES;
	    else
		index += 4;
#endif
	    
	    /* deal with group */
	    if (done == MB_NO)
		{
		/* get group size and id */
		mb_get_binary_int(SWAPFLAG, &buffer[index], &byte_count); index += sizeof(int);
		mb_get_binary_int(SWAPFLAG, &buffer[index], &group_id); index += sizeof(int);

		/* print debug statements */
		if (verbose >= 5)
		    {
		    fprintf(stderr,"\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n",
			    group_id, byte_count, function_name);
		    }
		}
	    
	    /* handle parameter group */
	    if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_SHP_GROUP_PARAMETER)
		{
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_roll_bias); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_pitch_bias); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_heading_bias); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_time_delay); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_trans_x_port); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_trans_y_port); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_trans_z_port); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_trans_x_stbd); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_trans_y_stbd); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_trans_z_stbd); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_trans_err_port); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_trans_err_stbd); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_nav_x); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_nav_y); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_nav_z); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_hrp_x); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_hrp_y); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->par_hrp_z); index += sizeof(float);
		}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       par_source:          %d\n",data->par_source);
	    fprintf(stderr,"dbg5       par_sec:             %u\n",data->par_sec);
	    fprintf(stderr,"dbg5       par_usec:            %u\n",data->par_usec);
	    fprintf(stderr,"dbg5       par_roll_bias:       %f\n",data->par_roll_bias);
	    fprintf(stderr,"dbg5       par_pitch_bias:      %f\n",data->par_pitch_bias);
	    fprintf(stderr,"dbg5       par_heading_bias:    %f\n",data->par_heading_bias);
	    fprintf(stderr,"dbg5       par_time_delay:      %f\n",data->par_time_delay);
	    fprintf(stderr,"dbg5       par_trans_x_port:    %f\n",data->par_trans_x_port);
	    fprintf(stderr,"dbg5       par_trans_y_port:    %f\n",data->par_trans_y_port);
	    fprintf(stderr,"dbg5       par_trans_z_port:    %f\n",data->par_trans_z_port);
	    fprintf(stderr,"dbg5       par_trans_x_stbd:    %f\n",data->par_trans_x_stbd);
	    fprintf(stderr,"dbg5       par_trans_y_stbd:    %f\n",data->par_trans_y_stbd);
	    fprintf(stderr,"dbg5       par_trans_z_stbd:    %f\n",data->par_trans_z_stbd);
	    fprintf(stderr,"dbg5       par_trans_err_port:  %f\n",data->par_trans_err_port);
	    fprintf(stderr,"dbg5       par_trans_err_stbd:  %f\n",data->par_trans_err_stbd);
	    fprintf(stderr,"dbg5       par_nav_x:           %f\n",data->par_nav_x);
	    fprintf(stderr,"dbg5       par_nav_y:           %f\n",data->par_nav_y);
	    fprintf(stderr,"dbg5       par_nav_z:           %f\n",data->par_nav_z);
	    fprintf(stderr,"dbg5       par_hrp_x:           %f\n",data->par_hrp_x);
	    fprintf(stderr,"dbg5       par_hrp_y:           %f\n",data->par_hrp_y);
	    fprintf(stderr,"dbg5       par_hrp_z:           %f\n",data->par_hrp_z);
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
int mbr_elmk2xse_rd_sidescan(int verbose, int buffer_size, char *buffer, 
		struct mbf_elmk2xse_struct *data, int *error)
{
	char	*function_name = "mbr_elmk2xse_rd_sidescan";
	int	status = MB_SUCCESS;
	int	byte_count;
	int	group_id;
	int	done;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		}

	/* get source and time */
	index = 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->sid_source); index += sizeof(int);
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->sid_sec); index += sizeof(int);
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->sid_usec); index += sizeof(int);
		
	/* loop over groups */
	done = MB_NO;
	while (index <= buffer_size 
		&& status == MB_SUCCESS
		&& done == MB_NO)
	    {
	    /* look for group start or frame end */
#ifdef DATAINPCBYTEORDER
	    while (index < buffer_size - 4
		&& strncmp(&buffer[index], "GSH$", 4) != 0
		&& strncmp(&buffer[index], "FSH#", 4) != 0)
		index++;
	    if (index >= buffer_size - 4
		|| strncmp(&buffer[index], "FSH#", 4) == 0)
		done = MB_YES;
	    else
		index += 4;
#else
	    while (index < buffer_size
		&& strncmp(&buffer[index], "$HSG", 4) != 0
		&& strncmp(&buffer[index], "#HSF", 4) != 0)
		index++;
	    if (index >= buffer_size
		|| strncmp(&buffer[index], "#HSF", 4) == 0)
		done = MB_YES;
	    else
		index += 4;
#endif
	    
	    /* deal with group */
	    if (done == MB_NO)
		{
		/* get group size and id */
		mb_get_binary_int(SWAPFLAG, &buffer[index], &byte_count); index += sizeof(int);
		mb_get_binary_int(SWAPFLAG, &buffer[index], &group_id); index += sizeof(int);

		/* print debug statements */
		if (verbose >= 5)
		    {
		    fprintf(stderr,"\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n",
			    group_id, byte_count, function_name);
		    }
		}
	    
	    /* handle general group */
	    if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_SSN_GROUP_GEN)
		{
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->sid_ping); index += sizeof(int);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->sid_frequency); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->sid_pulse); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->sid_power); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->sid_bandwidth); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->sid_sample); index += sizeof(float);
		}
	    
	    /* handle amplitude vs lateral group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_SSN_GROUP_AMPVSLAT)
		{
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->sid_bin_size); index += sizeof(int);
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->sid_offset); index += sizeof(int);
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->sid_num_pixels); index += sizeof(int);
		for (i=0;i<data->sid_num_pixels;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXPIXELS)
			{
			mb_get_binary_short(SWAPFLAG, &buffer[index], &data->ss[i]);
			}
		    index += sizeof(short);
		    }
		}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       sid_source:          %d\n",data->sid_source);
	    fprintf(stderr,"dbg5       sid_sec:             %u\n",data->sid_sec);
	    fprintf(stderr,"dbg5       sid_usec:            %u\n",data->sid_usec);
	    fprintf(stderr,"dbg5       sid_ping:            %d\n",data->sid_ping);
	    fprintf(stderr,"dbg5       sid_frequency:       %f\n",data->sid_frequency);
	    fprintf(stderr,"dbg5       sid_pulse:           %f\n",data->sid_pulse);
	    fprintf(stderr,"dbg5       sid_power:           %f\n",data->sid_power);
	    fprintf(stderr,"dbg5       sid_bandwidth:       %f\n",data->sid_bandwidth);
	    fprintf(stderr,"dbg5       sid_sample:          %f\n",data->sid_sample);
	    fprintf(stderr,"dbg5       sid_bin_size:        %d\n",data->sid_bin_size);
	    fprintf(stderr,"dbg5       sid_offset:          %d\n",data->sid_offset);
	    fprintf(stderr,"dbg5       sid_num_pixels:      %d\n",data->sid_num_pixels);
	    for (i=0;i<data->sid_num_pixels;i++)
		fprintf(stderr,"dbg5       pixel[%d]: %5d\n",
		    i, data->ss[i]);
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
int mbr_elmk2xse_rd_multibeam(int verbose, int buffer_size, char *buffer, 
		struct mbf_elmk2xse_struct *data, int *error)
{
	char	*function_name = "mbr_elmk2xse_rd_multibeam";
	int	status = MB_SUCCESS;
	int	byte_count;
	int	group_id;
	int	done;
	int	index;
	double	alpha, beta, theta, phi;
	double	rr, xx, zz;
	double	xmin, xmax, binsize;
	int	ngoodss;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		}

	/* set group flags off */
	data->mul_group_beam = MB_NO;	/* boolean flag - beam group read */
	data->mul_group_tt = MB_NO;	/* boolean flag - tt group read */
	data->mul_group_quality = MB_NO;/* boolean flag - quality group read */
	data->mul_group_amp = MB_NO;	/* boolean flag - amp group read */
	data->mul_group_delay = MB_NO;	/* boolean flag - delay group read */
	data->mul_group_lateral = MB_NO;/* boolean flag - lateral group read */
	data->mul_group_along = MB_NO;	/* boolean flag - along group read */
	data->mul_group_depth = MB_NO;	/* boolean flag - depth group read */
	data->mul_group_angle = MB_NO;	/* boolean flag - angle group read */
	data->mul_group_heave = MB_NO;	/* boolean flag - heave group read */
	data->mul_group_roll = MB_NO;	/* boolean flag - roll group read */
	data->mul_group_pitch = MB_NO;	/* boolean flag - pitch group read */
		
	/* get source and time */
	index = 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_source); index += sizeof(int);
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_sec); index += sizeof(int);
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_usec); index += sizeof(int);
		
	/* loop over groups */
	done = MB_NO;
	while (index <= buffer_size 
		&& status == MB_SUCCESS
		&& done == MB_NO)
	    {
	    /* look for group start or frame end */
#ifdef DATAINPCBYTEORDER
	    while (index < buffer_size - 4
		&& strncmp(&buffer[index], "GSH$", 4) != 0
		&& strncmp(&buffer[index], "FSH#", 4) != 0)
		index++;
	    if (index >= buffer_size - 4
		|| strncmp(&buffer[index], "FSH#", 4) == 0)
		done = MB_YES;
	    else
		index += 4;
#else
	    while (index < buffer_size
		&& strncmp(&buffer[index], "$HSG", 4) != 0
		&& strncmp(&buffer[index], "#HSF", 4) != 0)
		index++;
	    if (index >= buffer_size
		|| strncmp(&buffer[index], "#HSF", 4) == 0)
		done = MB_YES;
	    else
		index += 4;
#endif
	    
	    /* deal with group */
	    if (done == MB_NO)
		{
		/* get group size and id */
		mb_get_binary_int(SWAPFLAG, &buffer[index], &byte_count); index += sizeof(int);
		mb_get_binary_int(SWAPFLAG, &buffer[index], &group_id); index += sizeof(int);

		/* print debug statements */
		if (verbose >= 5)
		    {
		    fprintf(stderr,"\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n",
			    group_id, byte_count, function_name);
		    }
		}
	    
	    /* handle general group */
	    if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_MBM_GROUP_GEN)
		{
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_ping); index += sizeof(int);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->mul_frequency); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->mul_pulse); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->mul_power); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->mul_bandwidth); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->mul_sample); index += sizeof(float);
		mb_get_binary_float(SWAPFLAG, &buffer[index], &data->mul_swath); index += sizeof(float);
		}
	    
	    /* handle beam group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_MBM_GROUP_BEAM)
		{
		data->mul_group_beam = MB_YES;
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_num_beams); index += sizeof(int);
		for (i=0;i<data->mul_num_beams;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXBEAMS)
			mb_get_binary_short(SWAPFLAG, &buffer[index], &data->beams[i].beam);
		    index += sizeof(short);
		    }
		}
	    
	    /* handle traveltime group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_MBM_GROUP_TT)
		{
		data->mul_group_tt = MB_YES;
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_num_beams); index += sizeof(int);
		for (i=0;i<data->mul_num_beams;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXBEAMS)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->beams[i].tt);
		    index += sizeof(double);
		    }
		}
	    
	    /* handle quality group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_MBM_GROUP_QUALITY)
		{
		data->mul_group_quality = MB_YES;
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_num_beams); index += sizeof(int);
		for (i=0;i<data->mul_num_beams;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXBEAMS)
			data->beams[i].quality = buffer[index];
		    index++;
		    }
		}
	    
	    /* handle amplitude group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_MBM_GROUP_AMP)
		{
		data->mul_group_amp = MB_YES;
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_num_beams); index += sizeof(int);
		for (i=0;i<data->mul_num_beams;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXBEAMS)
			mb_get_binary_short(SWAPFLAG, &buffer[index], &data->beams[i].amplitude);
		    index += sizeof(short);
		    }
		}
	    
	    /* handle delay group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_MBM_GROUP_DELAY)
		{
		data->mul_group_delay = MB_YES;
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_num_beams); index += sizeof(int);
		for (i=0;i<data->mul_num_beams;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXBEAMS)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->beams[i].delay);
		    index += sizeof(double);
		    }
		}
	    
	    /* handle lateral group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_MBM_GROUP_LATERAL)
		{
		data->mul_group_lateral = MB_YES;
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_num_beams); index += sizeof(int);
		for (i=0;i<data->mul_num_beams;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXBEAMS)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->beams[i].lateral);
		    index += sizeof(double);
		    }
		}
	    
	    /* handle along group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_MBM_GROUP_ALONG)
		{
		data->mul_group_along = MB_YES;
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_num_beams); index += sizeof(int);
		for (i=0;i<data->mul_num_beams;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXBEAMS)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->beams[i].along);
		    index += sizeof(double);
		    }
		}
	    
	    /* handle depth group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_MBM_GROUP_DEPTH)
		{
		data->mul_group_depth = MB_YES;
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_num_beams); index += sizeof(int);
		for (i=0;i<data->mul_num_beams;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXBEAMS)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->beams[i].depth);
		    index += sizeof(double);
		    }
		}
	    
	    /* handle angle group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_MBM_GROUP_ANGLE)
		{
		data->mul_group_angle = MB_YES;
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_num_beams); index += sizeof(int);
		for (i=0;i<data->mul_num_beams;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXBEAMS)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->beams[i].angle);
		    index += sizeof(double);
		    }
		}
	    
	    /* handle heave group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_MBM_GROUP_HEAVE)
		{
		data->mul_group_heave = MB_YES;
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_num_beams); index += sizeof(int);
		for (i=0;i<data->mul_num_beams;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXBEAMS)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->beams[i].heave);
		    index += sizeof(double);
		    }
		}
	    
	    /* handle roll group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_MBM_GROUP_ROLL)
		{
		data->mul_group_roll = MB_YES;
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_num_beams); index += sizeof(int);
		for (i=0;i<data->mul_num_beams;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXBEAMS)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->beams[i].roll);
		    index += sizeof(double);
		    }
		}
	    
	    /* handle pitch group */
	    else if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_MBM_GROUP_PITCH)
		{
		data->mul_group_pitch = MB_YES;
		mb_get_binary_int(SWAPFLAG, &buffer[index], &data->mul_num_beams); index += sizeof(int);
		for (i=0;i<data->mul_num_beams;i++)
		    {
		    if (i < MBF_ELMK2XSE_MAXBEAMS)
			mb_get_binary_double(SWAPFLAG, &buffer[index], &data->beams[i].pitch);
		    index += sizeof(double);
		    }
		}
	    }
	    
	/* now if tt and angles read but bathymetry not read
	   calculate bathymetry assuming 1500 m/s velocity */
	if (status == MB_SUCCESS
	    && data->mul_group_tt == MB_YES
	    && data->mul_group_angle == MB_YES
	    && data->mul_group_heave == MB_YES
	    && data->mul_group_roll == MB_YES
	    && data->mul_group_pitch == MB_YES
	    && data->mul_group_depth == MB_NO)
	    {
	    data->mul_group_lateral = MB_YES;
	    data->mul_group_along = MB_YES;
	    data->mul_group_depth = MB_YES;
	    for (i=0;i<data->mul_num_beams;i++)
		{
		beta = 90.0 - RTD * data->beams[i].angle;
		alpha = RTD * data->beams[i].pitch;
		mb_rollpitch_to_takeoff(verbose, 
		    alpha, beta, &theta, 
		    &phi, error);
		rr = 1500.0 * data->beams[i].tt;
		xx = rr * sin(DTR * theta);
		zz = rr * cos(DTR * theta);
		data->beams[i].lateral 
			= xx * cos(DTR * phi);
		data->beams[i].along 
			= xx * sin(DTR * phi) 
			    + 0.5 * data->nav_speed_ground 
				* data->beams[i].delay;
		data->beams[i].depth = zz;
		}
	    }
	    
	/* now if sidescan already read but bin size lacking then
	   calculate bin size from bathymetry */
	if (data->mul_num_beams > 1
	    && data->sid_frame == MB_YES
	    && data->sid_num_pixels > 1
	    && data->sid_bin_size <= 0)
	    {
	    /* get width of bathymetry swath size */
	    xmin = 9999999.9;
	    xmax = -9999999.9;
	    for (i=0;i<data->mul_num_beams;i++)
		{
		xmin = MIN(xmin, data->beams[i].lateral);
		xmax = MAX(xmax, data->beams[i].lateral);
		}
	    
	    /* get number of nonzero pixels */
	    ngoodss = 0;
	    for (i=0;i<data->sid_num_pixels;i++)
		if (data->ss[i] != 0) ngoodss++;
		
	    /* get bin size */
	    if (xmax > xmin && ngoodss > 1)
		{
		binsize = (xmax - xmin) / (ngoodss - 1);
		data->sid_bin_size = 1000 * binsize;
		}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       mul_source:          %d\n",data->mul_source);
	    fprintf(stderr,"dbg5       mul_sec:             %u\n",data->mul_sec);
	    fprintf(stderr,"dbg5       mul_usec:            %u\n",data->mul_usec);
	    fprintf(stderr,"dbg5       mul_ping:            %d\n",data->mul_ping);
	    fprintf(stderr,"dbg5       mul_frequency:       %f\n",data->mul_frequency);
	    fprintf(stderr,"dbg5       mul_pulse:           %f\n",data->mul_pulse);
	    fprintf(stderr,"dbg5       mul_power:           %f\n",data->mul_power);
	    fprintf(stderr,"dbg5       mul_bandwidth:       %f\n",data->mul_bandwidth);
	    fprintf(stderr,"dbg5       mul_sample:          %f\n",data->mul_sample);
	    fprintf(stderr,"dbg5       mul_swath:           %f\n",data->mul_swath);
	    fprintf(stderr,"dbg5       mul_group_beam:      %d\n",data->mul_group_beam);
	    fprintf(stderr,"dbg5       mul_group_tt:        %d\n",data->mul_group_tt);
	    fprintf(stderr,"dbg5       mul_group_quality:   %d\n",data->mul_group_quality);
	    fprintf(stderr,"dbg5       mul_group_amp:       %d\n",data->mul_group_amp);
	    fprintf(stderr,"dbg5       mul_group_delay:     %d\n",data->mul_group_delay);
	    fprintf(stderr,"dbg5       mul_group_lateral:   %d\n",data->mul_group_lateral);
	    fprintf(stderr,"dbg5       mul_group_along:     %d\n",data->mul_group_along);
	    fprintf(stderr,"dbg5       mul_group_depth:     %d\n",data->mul_group_depth);
	    fprintf(stderr,"dbg5       mul_group_angle:     %d\n",data->mul_group_angle);
	    fprintf(stderr,"dbg5       mul_group_heave:     %d\n",data->mul_group_heave);
	    fprintf(stderr,"dbg5       mul_group_roll:      %d\n",data->mul_group_roll);
	    fprintf(stderr,"dbg5       mul_group_pitch:     %d\n",data->mul_group_pitch);
	    fprintf(stderr,"dbg5       mul_num_beams:       %d\n",data->mul_num_beams);
	    for (i=0;i<data->mul_num_beams;i++)
		fprintf(stderr,"dbg5       beam[%d]: %3d %7.2f %7.2f %7.2f %3d %3d %6.3f %6.2f %5.3f %5.2f %6.2f %6.2f\n",
		    i, data->beams[i].beam, 
		    data->beams[i].lateral, 
		    data->beams[i].along, 
		    data->beams[i].depth, 
		    data->beams[i].amplitude, 
		    data->beams[i].quality, 
		    data->beams[i].tt, 
		    data->beams[i].angle, 
		    data->beams[i].delay, 
		    data->beams[i].heave, 
		    data->beams[i].roll, 
		    data->beams[i].pitch);
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
int mbr_elmk2xse_rd_comment(int verbose, int buffer_size, char *buffer, 
		struct mbf_elmk2xse_struct *data, int *error)
{
	char	*function_name = "mbr_elmk2xse_rd_comment";
	int	status = MB_SUCCESS;
	int	byte_count;
	int	group_id;
	int	done;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		}
		
	/* get source and time */
	index = 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->com_source); index += sizeof(int);
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->com_sec); index += sizeof(int);
	mb_get_binary_int(SWAPFLAG, &buffer[index], &data->com_usec); index += sizeof(int);
		
	/* loop over groups */
	done = MB_NO;
	while (index <= buffer_size 
		&& status == MB_SUCCESS
		&& done == MB_NO)
	    {
	    /* look for group start or frame end */
#ifdef DATAINPCBYTEORDER
	    while (index < buffer_size - 4
		&& strncmp(&buffer[index], "GSH$", 4) != 0
		&& strncmp(&buffer[index], "FSH#", 4) != 0)
		index++;
	    if (index >= buffer_size - 4
		|| strncmp(&buffer[index], "FSH#", 4) == 0)
		done = MB_YES;
	    else
		index += 4;
#else
	    while (index < buffer_size
		&& strncmp(&buffer[index], "$HSG", 4) != 0
		&& strncmp(&buffer[index], "#HSF", 4) != 0)
		index++;
	    if (index >= buffer_size
		|| strncmp(&buffer[index], "#HSF", 4) == 0)
		done = MB_YES;
	    else
		index += 4;
#endif
	    
	    /* deal with group */
	    if (done == MB_NO)
		{
		/* get group size and id */
		mb_get_binary_int(SWAPFLAG, &buffer[index], &byte_count); index += sizeof(int);
		mb_get_binary_int(SWAPFLAG, &buffer[index], &group_id); index += sizeof(int);

		/* print debug statements */
		if (verbose >= 5)
		    {
		    fprintf(stderr,"\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n",
			    group_id, byte_count, function_name);
		    }
		}
	    
	    /* handle general group */
	    if (done == MB_NO
		&& group_id == MBF_ELMK2XSE_COM_GROUP_GEN)
		{
		for (i=0;i<byte_count-4;i++)
		    {
		    if (i<MBF_ELMK2XSE_COMMENT_LENGTH-1)
			data->comment[i] = buffer[index];
		    index++;
		    }
		data->comment[MIN(byte_count-4, MBF_ELMK2XSE_COMMENT_LENGTH-1)] = 0;
		}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       comment:             %s\n",data->comment);
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
int mbr_elmk2xse_wr_data(int verbose, char *mbio_ptr, char *data_ptr, int *error)
{
	char	*function_name = "mbr_elmk2xse_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_elmk2xse_struct *data;
	FILE	*mbfp;
	int	buffer_size;
	int	write_size;
	char	*buffer;

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
	data = (struct mbf_elmk2xse_struct *) data_ptr;
	mbfp = mb_io_ptr->mbfp;
	buffer = mb_io_ptr->hdr_comment;

	if (data->kind == MB_DATA_COMMENT)
		{
		status = mbr_elmk2xse_wr_comment(verbose,&buffer_size,buffer,data,error);
		if ((write_size = fwrite(buffer,1,buffer_size,mbfp)) != buffer_size)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		}
	else if (data->kind == MB_DATA_NAV)
		{
		status = mbr_elmk2xse_wr_nav(verbose,&buffer_size,buffer,data,error);
		if ((write_size = fwrite(buffer,1,buffer_size,mbfp)) != buffer_size)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		}
	else if (data->kind == MB_DATA_VELOCITY_PROFILE)
		{
		status = mbr_elmk2xse_wr_svp(verbose,&buffer_size,buffer,data,error);
		if ((write_size = fwrite(buffer,1,buffer_size,mbfp)) != buffer_size)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		}
	else if (data->kind == MB_DATA_PARAMETER)
		{
		status = mbr_elmk2xse_wr_ship(verbose,&buffer_size,buffer,data,error);
		if ((write_size = fwrite(buffer,1,buffer_size,mbfp)) != buffer_size)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		}
	else if (data->kind == MB_DATA_DATA)
		{
		if (data->sid_frame == MB_YES)
		    {
		    status = mbr_elmk2xse_wr_sidescan(verbose,&buffer_size,buffer,data,error);
		    if ((write_size = fwrite(buffer,1,buffer_size,mbfp)) != buffer_size)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		    }
		if (data->mul_frame == MB_YES)
		    {
		    status = mbr_elmk2xse_wr_multibeam(verbose,&buffer_size,buffer,data,error);
		    if ((write_size = fwrite(buffer,1,buffer_size,mbfp)) != buffer_size)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		    }
		}
	else if (data->kind == MB_DATA_RAW_LINE)
		{
		if (data->rawsize > 0)
		    {
		    if ((write_size = fwrite(data->raw,1,data->rawsize,mbfp)) != data->rawsize)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		    }
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
int mbr_elmk2xse_wr_nav(int verbose, int *buffer_size, char *buffer, 
		char *data_ptr, int *error)
{
	char	*function_name = "mbr_elmk2xse_wr_nav";
	int	status = MB_SUCCESS;
	struct mbf_elmk2xse_struct *data;
	int	index;
	int	size;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
	    fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
	    }

	/* get pointer to raw data structure */
	data = (struct mbf_elmk2xse_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       nav_source:          %d\n",data->nav_source);
	    fprintf(stderr,"dbg5       nav_sec:             %u\n",data->nav_sec);
	    fprintf(stderr,"dbg5       nav_usec:            %u\n",data->nav_usec);
	    fprintf(stderr,"dbg5       nav_quality:         %d\n",data->nav_quality);
	    fprintf(stderr,"dbg5       nav_status:          %d\n",data->nav_status);
	    fprintf(stderr,"dbg5       nav_description_len: %d\n",data->nav_description_len);
	    fprintf(stderr,"dbg5       nav_description:     %s\n",data->nav_description);
	    fprintf(stderr,"dbg5       nav_x:               %f\n",data->nav_x);
	    fprintf(stderr,"dbg5       nav_y:               %f\n",data->nav_y);
	    fprintf(stderr,"dbg5       nav_z:               %f\n",data->nav_z);
	    fprintf(stderr,"dbg5       nav_speed_ground:    %f\n",data->nav_speed_ground);
	    fprintf(stderr,"dbg5       nav_course_ground:   %f\n",data->nav_course_ground);
	    fprintf(stderr,"dbg5       nav_speed_water:     %f\n",data->nav_speed_water);
	    fprintf(stderr,"dbg5       nav_course_water:    %f\n",data->nav_course_water);
	    }

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* get frame size */
	size = 16 + 44 + data->nav_description_len + 32 + 32;
	mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	
	/* get frame time */
	mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_NAV_FRAME, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->nav_source, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->nav_sec, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->nav_usec, &buffer[index]); index += sizeof(int);

	/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* get group size */
	size = 32 + data->nav_description_len;
	mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	
	/* get pos group */
	mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_NAV_GROUP_POS, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->nav_description_len, &buffer[index]); index += sizeof(int);
	for (i=0;i<data->nav_description_len;i++)
	    {
	    buffer[index] = data->nav_description[i]; index++;
	    }
	mb_put_binary_double(SWAPFLAG, data->nav_x, &buffer[index]); index += sizeof(double);
	mb_put_binary_double(SWAPFLAG, data->nav_y, &buffer[index]); index += sizeof(double);
	mb_put_binary_double(SWAPFLAG, data->nav_z, &buffer[index]); index += sizeof(double);

	/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;

	/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* get group size */
	size = 20;
	mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	
	/* get motion ground truth group */
	mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_NAV_GROUP_MOTIONGT, &buffer[index]); index += sizeof(int);
	mb_put_binary_double(SWAPFLAG, data->nav_speed_ground, &buffer[index]); index += sizeof(double);
	mb_put_binary_double(SWAPFLAG, data->nav_course_ground, &buffer[index]); index += sizeof(double);

	/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;

	/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* get group size */
	size = 20;
	mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	
	/* get motion through water group */
	mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_NAV_GROUP_MOTIONTW, &buffer[index]); index += sizeof(int);
	mb_put_binary_double(SWAPFLAG, data->nav_speed_water, &buffer[index]); index += sizeof(double);
	mb_put_binary_double(SWAPFLAG, data->nav_course_water, &buffer[index]); index += sizeof(double);

	/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;

	/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;	
	
	*buffer_size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       buffer_size:%d\n",*buffer_size);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_elmk2xse_wr_svp(int verbose, int *buffer_size, char *buffer, 
		char *data_ptr, int *error)
{
	char	*function_name = "mbr_elmk2xse_wr_svp";
	int	status = MB_SUCCESS;
	struct mbf_elmk2xse_struct *data;
	int	index;
	int	size;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
	    fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
	    }

	/* get pointer to raw data structure */
	data = (struct mbf_elmk2xse_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       svp_source:          %d\n",data->svp_source);
	    fprintf(stderr,"dbg5       svp_sec:             %u\n",data->svp_sec);
	    fprintf(stderr,"dbg5       svp_usec:            %u\n",data->svp_usec);
	    fprintf(stderr,"dbg5       svp_nsvp:            %d\n",data->svp_nsvp);
	    fprintf(stderr,"dbg5       svp_nctd:            %d\n",data->svp_nctd);
	    fprintf(stderr,"dbg5       svp_ssv:             %f\n",data->svp_ssv);
	    for (i=0;i<data->svp_nsvp;i++)
		fprintf(stderr,"dbg5       svp[%d]:	        %f %f\n",
		    i, data->svp_depth[i], data->svp_velocity[i]);
	    for (i=0;i<data->svp_nctd;i++)
		fprintf(stderr,"dbg5       cstd[%d]:        %f %f %f %f\n",
		    i, data->svp_conductivity[i], data->svp_salinity[i], 
		    data->svp_temperature[i], data->svp_pressure[i]);
	    }

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* get frame size */
	size = 16;
	if (data->svp_nsvp > 0)
	    size += 2 * (20 + 8 * data->svp_nsvp);
	if (data->svp_nctd > 0)
	    size += 4 * (20 + 8 * data->svp_nctd);
	if (data->svp_ssv > 0.0)
	    size += 24;
	mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	
	/* get frame time */
	mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_SVP_FRAME, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->svp_source, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->svp_sec, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->svp_usec, &buffer[index]); index += sizeof(int);

	/* get depth and velocity groups */
	if (data->svp_nsvp > 0)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + 8 * data->svp_nsvp;
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get depth array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_SVP_GROUP_DEPTH, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->svp_nsvp, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->svp_nsvp;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->svp_depth[i], &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;

	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + 8 * data->svp_nsvp;
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get depth array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_SVP_GROUP_VELOCITY, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->svp_nsvp, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->svp_nsvp;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->svp_velocity[i], &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    }

	/* get ctd groups */
	if (data->svp_nctd > 0)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + 8 * data->svp_nctd;
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get conductivity array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_SVP_GROUP_CONDUCTIVITY, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->svp_nctd, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->svp_nctd;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->svp_conductivity[i], &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;

	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + 8 * data->svp_nctd;
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get salinity array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_SVP_GROUP_SALINITY, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->svp_nctd, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->svp_nctd;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->svp_salinity[i], &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;

	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + 8 * data->svp_nctd;
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get temperature array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_SVP_GROUP_TEMP, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->svp_nctd, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->svp_nctd;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->svp_temperature[i], &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;

	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + 8 * data->svp_nctd;
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get pressure array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_SVP_GROUP_PRESSURE, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->svp_nctd, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->svp_nctd;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->svp_pressure[i], &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    }
	    
	if (data->svp_ssv > 0.0)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 12;
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get ssv */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_SVP_GROUP_SSV, &buffer[index]); index += sizeof(int);
	    mb_put_binary_double(SWAPFLAG, data->svp_ssv, &buffer[index]); index += sizeof(double);

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    }

	/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;	
	
	*buffer_size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       buffer_size:%d\n",*buffer_size);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_elmk2xse_wr_ship(int verbose, int *buffer_size, char *buffer, 
		char *data_ptr, int *error)
{
	char	*function_name = "mbr_elmk2xse_wr_ship";
	int	status = MB_SUCCESS;
	struct mbf_elmk2xse_struct *data;
	int	index;
	int	size;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
	    fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
	    }

	/* get pointer to raw data structure */
	data = (struct mbf_elmk2xse_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       par_source:          %d\n",data->par_source);
	    fprintf(stderr,"dbg5       par_sec:             %u\n",data->par_sec);
	    fprintf(stderr,"dbg5       par_usec:            %u\n",data->par_usec);
	    fprintf(stderr,"dbg5       par_roll_bias:       %f\n",data->par_roll_bias);
	    fprintf(stderr,"dbg5       par_pitch_bias:      %f\n",data->par_pitch_bias);
	    fprintf(stderr,"dbg5       par_heading_bias:    %f\n",data->par_heading_bias);
	    fprintf(stderr,"dbg5       par_time_delay:      %f\n",data->par_time_delay);
	    fprintf(stderr,"dbg5       par_trans_x_port:    %f\n",data->par_trans_x_port);
	    fprintf(stderr,"dbg5       par_trans_y_port:    %f\n",data->par_trans_y_port);
	    fprintf(stderr,"dbg5       par_trans_z_port:    %f\n",data->par_trans_z_port);
	    fprintf(stderr,"dbg5       par_trans_x_stbd:    %f\n",data->par_trans_x_stbd);
	    fprintf(stderr,"dbg5       par_trans_y_stbd:    %f\n",data->par_trans_y_stbd);
	    fprintf(stderr,"dbg5       par_trans_z_stbd:    %f\n",data->par_trans_z_stbd);
	    fprintf(stderr,"dbg5       par_trans_err_port:  %f\n",data->par_trans_err_port);
	    fprintf(stderr,"dbg5       par_trans_err_stbd:  %f\n",data->par_trans_err_stbd);
	    fprintf(stderr,"dbg5       par_nav_x:           %f\n",data->par_nav_x);
	    fprintf(stderr,"dbg5       par_nav_y:           %f\n",data->par_nav_y);
	    fprintf(stderr,"dbg5       par_nav_z:           %f\n",data->par_nav_z);
	    fprintf(stderr,"dbg5       par_hrp_x:           %f\n",data->par_hrp_x);
	    fprintf(stderr,"dbg5       par_hrp_y:           %f\n",data->par_hrp_y);
	    fprintf(stderr,"dbg5       par_hrp_z:           %f\n",data->par_hrp_z);
	    }

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* get frame size */
	mb_put_binary_int(SWAPFLAG, 104, &buffer[index]); index += sizeof(int);
	
	/* get frame time */
	mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_SHP_FRAME, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->par_source, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->par_sec, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->par_usec, &buffer[index]); index += sizeof(int);

	/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* get group size */
	size = 76;
	mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	
	/* get parameter group */
	mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_SHP_GROUP_PARAMETER, &buffer[index]); index += sizeof(int);
	mb_put_binary_float(SWAPFLAG, data->par_roll_bias, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_pitch_bias, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_heading_bias, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_time_delay, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_trans_x_port, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_trans_y_port, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_trans_z_port, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_trans_x_stbd, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_trans_y_stbd, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_trans_z_stbd, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_trans_err_port, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_trans_err_stbd, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_nav_x, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_nav_y, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_nav_z, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_hrp_x, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_hrp_y, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->par_hrp_z, &buffer[index]); index += sizeof(float);

	/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;

	/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;	
	
	*buffer_size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       buffer_size:%d\n",*buffer_size);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_elmk2xse_wr_multibeam(int verbose, int *buffer_size, char *buffer, 
		char *data_ptr, int *error)
{
	char	*function_name = "mbr_elmk2xse_wr_multibeam";
	int	status = MB_SUCCESS;
	struct mbf_elmk2xse_struct *data;
	int	index;
	int	size;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
	    fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
	    }

	/* get pointer to raw data structure */
	data = (struct mbf_elmk2xse_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       mul_source:          %d\n",data->mul_source);
	    fprintf(stderr,"dbg5       mul_sec:             %u\n",data->mul_sec);
	    fprintf(stderr,"dbg5       mul_usec:            %u\n",data->mul_usec);
	    fprintf(stderr,"dbg5       mul_ping:            %d\n",data->mul_ping);
	    fprintf(stderr,"dbg5       mul_frequency:       %f\n",data->mul_frequency);
	    fprintf(stderr,"dbg5       mul_pulse:           %f\n",data->mul_pulse);
	    fprintf(stderr,"dbg5       mul_power:           %f\n",data->mul_power);
	    fprintf(stderr,"dbg5       mul_bandwidth:       %f\n",data->mul_bandwidth);
	    fprintf(stderr,"dbg5       mul_sample:          %f\n",data->mul_sample);
	    fprintf(stderr,"dbg5       mul_swath:           %f\n",data->mul_swath);
	    fprintf(stderr,"dbg5       mul_group_beam:      %d\n",data->mul_group_beam);
	    fprintf(stderr,"dbg5       mul_group_tt:        %d\n",data->mul_group_tt);
	    fprintf(stderr,"dbg5       mul_group_quality:   %d\n",data->mul_group_quality);
	    fprintf(stderr,"dbg5       mul_group_amp:       %d\n",data->mul_group_amp);
	    fprintf(stderr,"dbg5       mul_group_delay:     %d\n",data->mul_group_delay);
	    fprintf(stderr,"dbg5       mul_group_lateral:   %d\n",data->mul_group_lateral);
	    fprintf(stderr,"dbg5       mul_group_along:     %d\n",data->mul_group_along);
	    fprintf(stderr,"dbg5       mul_group_depth:     %d\n",data->mul_group_depth);
	    fprintf(stderr,"dbg5       mul_group_angle:     %d\n",data->mul_group_angle);
	    fprintf(stderr,"dbg5       mul_group_heave:     %d\n",data->mul_group_heave);
	    fprintf(stderr,"dbg5       mul_group_roll:      %d\n",data->mul_group_roll);
	    fprintf(stderr,"dbg5       mul_group_pitch:     %d\n",data->mul_group_pitch);
	    fprintf(stderr,"dbg5       mul_num_beams:       %d\n",data->mul_num_beams);
	    for (i=0;i<data->mul_num_beams;i++)
		fprintf(stderr,"dbg5       beam[%d]: %3d %7.2f %7.2f %7.2f %3d %3d %6.3f %6.2f %5.3f %5.2f %6.2f %6.2f\n",
		    i, data->beams[i].beam, 
		    data->beams[i].lateral, 
		    data->beams[i].along, 
		    data->beams[i].depth, 
		    data->beams[i].amplitude, 
		    data->beams[i].quality, 
		    data->beams[i].tt, 
		    data->beams[i].angle, 
		    data->beams[i].delay, 
		    data->beams[i].heave, 
		    data->beams[i].roll, 
		    data->beams[i].pitch);
	    }

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* get frame size */
	size = 16;
	size += 44;
	if (data->mul_group_beam == MB_YES)
	    size += 20 + data->mul_num_beams * sizeof(short);
	if (data->mul_group_tt == MB_YES)
	    size += 20 + data->mul_num_beams * sizeof(double);
	if (data->mul_group_quality == MB_YES)
	    size += 20 + data->mul_num_beams * sizeof(char);
	if (data->mul_group_amp == MB_YES)
	    size += 20 + data->mul_num_beams * sizeof(short);
	if (data->mul_group_delay == MB_YES)
	    size += 20 + data->mul_num_beams * sizeof(double);
	if (data->mul_group_lateral == MB_YES)
	    size += 20 + data->mul_num_beams * sizeof(double);
	if (data->mul_group_along == MB_YES)
	    size += 20 + data->mul_num_beams * sizeof(double);
	if (data->mul_group_depth == MB_YES)
	    size += 20 + data->mul_num_beams * sizeof(double);
	if (data->mul_group_angle == MB_YES)
	    size += 20 + data->mul_num_beams * sizeof(double);
	if (data->mul_group_heave == MB_YES)
	    size += 20 + data->mul_num_beams * sizeof(double);
	if (data->mul_group_roll == MB_YES)
	    size += 20 + data->mul_num_beams * sizeof(double);
	if (data->mul_group_pitch == MB_YES)
	    size += 20 + data->mul_num_beams * sizeof(double);
	mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	
	/* get frame time */
	mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_MBM_FRAME, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->mul_source, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->mul_sec, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->mul_usec, &buffer[index]); index += sizeof(int);

	/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* get group size */
	size = 32;
	mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	
	/* get general group */
	mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_MBM_GROUP_GEN, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->mul_ping, &buffer[index]); index += sizeof(int);
	mb_put_binary_float(SWAPFLAG, data->mul_frequency, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->mul_pulse, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->mul_power, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->mul_bandwidth, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->mul_sample, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->mul_swath, &buffer[index]); index += sizeof(float);

	/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;

	/* get beam groups */
	if (data->mul_group_beam == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + data->mul_num_beams * sizeof(short);
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get beam array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_MBM_GROUP_BEAM, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->mul_num_beams, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->mul_num_beams;i++)
		{
		mb_put_binary_short(SWAPFLAG, data->beams[i].beam, &buffer[index]); index += sizeof(short);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    }

	/* get tt groups */
	if (data->mul_group_tt == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + data->mul_num_beams * sizeof(double);
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get tt array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_MBM_GROUP_TT, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->mul_num_beams, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->mul_num_beams;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->beams[i].tt, &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    }

	/* get quality groups */
	if (data->mul_group_quality == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + data->mul_num_beams * sizeof(char);
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get quality array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_MBM_GROUP_QUALITY, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->mul_num_beams, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->mul_num_beams;i++)
		{
		buffer[index] = data->beams[i].quality; index += sizeof(char);		    
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    }

	/* get amplitude groups */
	if (data->mul_group_amp == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + data->mul_num_beams * sizeof(short);
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get amplitude array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_MBM_GROUP_AMP, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->mul_num_beams, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->mul_num_beams;i++)
		{
		mb_put_binary_short(SWAPFLAG, data->beams[i].amplitude, &buffer[index]); index += sizeof(short);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    }

	/* get delay groups */
	if (data->mul_group_delay == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + data->mul_num_beams * sizeof(double);
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get delay array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_MBM_GROUP_DELAY, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->mul_num_beams, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->mul_num_beams;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->beams[i].delay, &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    }

	/* get lateral groups */
	if (data->mul_group_lateral == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + data->mul_num_beams * sizeof(double);
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get lateral array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_MBM_GROUP_LATERAL, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->mul_num_beams, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->mul_num_beams;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->beams[i].lateral, &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    }

	/* get along groups */
	if (data->mul_group_along == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + data->mul_num_beams * sizeof(double);
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get along array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_MBM_GROUP_ALONG, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->mul_num_beams, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->mul_num_beams;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->beams[i].along, &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    }

	/* get depth groups */
	if (data->mul_group_depth == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + data->mul_num_beams * sizeof(double);
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get depth array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_MBM_GROUP_DEPTH, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->mul_num_beams, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->mul_num_beams;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->beams[i].depth, &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    }

	/* get angle groups */
	if (data->mul_group_angle == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + data->mul_num_beams * sizeof(double);
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get angle array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_MBM_GROUP_ANGLE, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->mul_num_beams, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->mul_num_beams;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->beams[i].angle, &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    }

	/* get heave groups */
	if (data->mul_group_heave == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + data->mul_num_beams * sizeof(double);
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get heave array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_MBM_GROUP_HEAVE, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->mul_num_beams, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->mul_num_beams;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->beams[i].heave, &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    }

	/* get roll groups */
	if (data->mul_group_roll == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + data->mul_num_beams * sizeof(double);
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get roll array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_MBM_GROUP_ROLL, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->mul_num_beams, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->mul_num_beams;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->beams[i].roll, &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    }

	/* get pitch groups */
	if (data->mul_group_pitch == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* get group size */
	    size = 8 + data->mul_num_beams * sizeof(double);
	    mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	    
	    /* get pitch array */
	    mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_MBM_GROUP_PITCH, &buffer[index]); index += sizeof(int);
	    mb_put_binary_int(SWAPFLAG, data->mul_num_beams, &buffer[index]); index += sizeof(int);
	    for (i=0;i<data->mul_num_beams;i++)
		{
		mb_put_binary_double(SWAPFLAG, data->beams[i].pitch, &buffer[index]); index += sizeof(double);
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    }

	/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;	
	
	*buffer_size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       buffer_size:%d\n",*buffer_size);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_elmk2xse_wr_sidescan(int verbose, int *buffer_size, char *buffer, 
		char *data_ptr, int *error)
{
	char	*function_name = "mbr_elmk2xse_wr_sidescan";
	int	status = MB_SUCCESS;
	struct mbf_elmk2xse_struct *data;
	int	index;
	int	size;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
	    fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
	    }

	/* get pointer to raw data structure */
	data = (struct mbf_elmk2xse_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       sid_source:          %d\n",data->sid_source);
	    fprintf(stderr,"dbg5       sid_sec:             %u\n",data->sid_sec);
	    fprintf(stderr,"dbg5       sid_usec:            %u\n",data->sid_usec);
	    fprintf(stderr,"dbg5       sid_ping:            %d\n",data->sid_ping);
	    fprintf(stderr,"dbg5       sid_frequency:       %f\n",data->sid_frequency);
	    fprintf(stderr,"dbg5       sid_pulse:           %f\n",data->sid_pulse);
	    fprintf(stderr,"dbg5       sid_power:           %f\n",data->sid_power);
	    fprintf(stderr,"dbg5       sid_bandwidth:       %f\n",data->sid_bandwidth);
	    fprintf(stderr,"dbg5       sid_sample:          %f\n",data->sid_sample);
	    fprintf(stderr,"dbg5       sid_bin_size:        %d\n",data->sid_bin_size);
	    fprintf(stderr,"dbg5       sid_offset:          %d\n",data->sid_offset);
	    fprintf(stderr,"dbg5       sid_num_pixels:      %d\n",data->sid_num_pixels);
	    for (i=0;i<data->sid_num_pixels;i++)
		fprintf(stderr,"dbg5       pixel[%d]: %5d\n",
		    i, data->ss[i]);
	    }

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* get frame size */
	size = 16;
	size += 40;
	size += 28 + data->sid_num_pixels * sizeof(short);
	mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	
	/* get frame time */
	mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_SSN_FRAME, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->sid_source, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->sid_sec, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->sid_usec, &buffer[index]); index += sizeof(int);

	/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* get group size */
	size = 28;
	mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	
	/* get general group */
	mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_SSN_GROUP_GEN, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->sid_ping, &buffer[index]); index += sizeof(int);
	mb_put_binary_float(SWAPFLAG, data->sid_frequency, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->sid_pulse, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->sid_power, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->sid_bandwidth, &buffer[index]); index += sizeof(float);
	mb_put_binary_float(SWAPFLAG, data->sid_sample, &buffer[index]); index += sizeof(float);

	/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;

	/* get amplitude vs lateral groups */
	/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* get group size */
	size = 16 + data->sid_num_pixels * sizeof(short);
	mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	
	/* get amplitude array */
	mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_SSN_GROUP_AMPVSLAT, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->sid_bin_size, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->sid_offset, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->sid_num_pixels, &buffer[index]); index += sizeof(int);
	for (i=0;i<data->sid_num_pixels;i++)
	    {
	    mb_put_binary_short(SWAPFLAG, data->ss[i], &buffer[index]); index += sizeof(short);
	    }

	/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += sizeof(int);

	/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;	
	
	*buffer_size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       buffer_size:%d\n",*buffer_size);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_elmk2xse_wr_comment(int verbose, int *buffer_size, char *buffer, 
		char *data_ptr, int *error)
{
	char	*function_name = "mbr_elmk2xse_wr_comment";
	int	status = MB_SUCCESS;
	struct mbf_elmk2xse_struct *data;
	int	index;
	int	size;
	int	len;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
	    fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
	    }

	/* get pointer to raw data structure */
	data = (struct mbf_elmk2xse_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       comment:             %s\n",data->comment);
	    }

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* get frame size */
	len = strlen(data->comment) + 4;
	if (len % 4 > 0)
	    len += 4 - (len % 4);
	size = len + 32;
	mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += sizeof(int);
	
	/* get frame time */
	mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_COM_FRAME, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->com_source, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->com_sec, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, data->com_usec, &buffer[index]); index += sizeof(int);

	/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* get group size and id */
	mb_put_binary_int(SWAPFLAG, len, &buffer[index]); index += sizeof(int);
	mb_put_binary_int(SWAPFLAG, MBF_ELMK2XSE_COM_GROUP_GEN, &buffer[index]); index += sizeof(int);
	strncpy(&buffer[index], data->comment, len); index += len;

	/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;

	/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;
	
	*buffer_size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       buffer_size:%d\n",*buffer_size);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
