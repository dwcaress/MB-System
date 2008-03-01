/*--------------------------------------------------------------------
 *    The MB-system:	mbr_em710raw.c	2/26/2008
 *	$Id: mbr_em710raw.c,v 5.0 2008-03-01 09:11:35 caress Exp $
 *
 *    Copyright (c) 2008 by
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
 * mbr_em710raw.c contains the functions for reading and writing
 * multibeam data in the EM710RAW format.  
 * These functions include:
 *   mbr_alm_em710raw	- allocate read/write memory
 *   mbr_dem_em710raw	- deallocate read/write memory
 *   mbr_rt_em710raw	- read and translate data
 *   mbr_wt_em710raw	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 26, 2008
 *
 * $Log: not supported by cvs2svn $
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
#include "../../include/mbsys_simrad3.h"

/* include for byte swapping */
#include "../../include/mb_swap.h"
	
/* turn on debug statements here */
/* #define MBR_EM710RAW_DEBUG 1 */
	
/* essential function prototypes */
int mbr_register_em710raw(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_em710raw(int verbose, 
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
int mbr_alm_em710raw(int verbose, void *mbio_ptr, int *error);
int mbr_dem_em710raw(int verbose, void *mbio_ptr, int *error);
int mbr_rt_em710raw(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_em710raw(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_em710raw_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_em710raw_chk_label(int verbose, void *mbio_ptr, char *label, 
		short *type, short *sonar);
int mbr_em710raw_rd_status(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short type, short sonar, int *goodend, int *error);
int mbr_em710raw_rd_start(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short type, short sonar, int *version, int *goodend, int *error);
int mbr_em710raw_rd_run_parameter(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error);
int mbr_em710raw_rd_clock(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error);
int mbr_em710raw_rd_tide(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error);
int mbr_em710raw_rd_height(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error);
int mbr_em710raw_rd_heading(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error);
int mbr_em710raw_rd_ssv(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error);
int mbr_em710raw_rd_tilt(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error);
int mbr_em710raw_rd_attitude(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error);
int mbr_em710raw_rd_pos(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error);
int mbr_em710raw_rd_svp(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error);
int mbr_em710raw_rd_svp2(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error);
int mbr_em710raw_rd_bath2(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		int *match, short sonar, int version, int *goodend, int *error);
int mbr_em710raw_rd_rawbeam4(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error);
int mbr_em710raw_rd_ss2(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int length, int *match, int *goodend, int *error);
int mbr_em710raw_rd_wc(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error);
int mbr_em710raw_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_em710raw_wr_status(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_start(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_run_parameter(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_clock(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_tide(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_height(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_heading(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_ssv(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_tilt(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_attitude(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_pos(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_svp(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_svp2(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_bath2(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_rawbeam4(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_ss2(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);
int mbr_em710raw_wr_wc(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error);

static char res_id[]="$Id: mbr_em710raw.c,v 5.0 2008-03-01 09:11:35 caress Exp $";

/*--------------------------------------------------------------------*/
int mbr_register_em710raw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_em710raw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set format info parameters */
	status = mbr_info_em710raw(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_em710raw;
	mb_io_ptr->mb_io_format_free = &mbr_dem_em710raw; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_simrad3_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_simrad3_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_em710raw; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_em710raw; 
	mb_io_ptr->mb_io_dimensions = &mbsys_simrad3_dimensions; 
	mb_io_ptr->mb_io_pingnumber = &mbsys_simrad3_pingnumber; 
	mb_io_ptr->mb_io_extract = &mbsys_simrad3_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_simrad3_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_simrad3_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_simrad3_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_simrad3_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = &mbsys_simrad3_extract_svp; 
	mb_io_ptr->mb_io_insert_svp = &mbsys_simrad3_insert_svp; 
	mb_io_ptr->mb_io_ttimes = &mbsys_simrad3_ttimes; 
	mb_io_ptr->mb_io_detects = &mbsys_simrad3_detects; 
	mb_io_ptr->mb_io_gains = &mbsys_simrad3_gains; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_simrad3_copy; 
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
		fprintf(stderr,"dbg2       detects:            %d\n",mb_io_ptr->mb_io_detects);
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
int mbr_info_em710raw(int verbose, 
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
	char	*function_name = "mbr_info_em710raw";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_SIMRAD3;
	*beams_bath_max = 400;
	*beams_amp_max = 400;
	*pixels_ss_max = 1024;
	strncpy(format_name, "EM710RAW", MB_NAME_LENGTH);
	strncpy(system_name, "SIMRAD3", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_EM710RAW\nInformal Description: Simrad current multibeam vendor format\nAttributes:           Simrad EM710,\n                      bathymetry, amplitude, and sidescan,\n                      up to 400 beams, variable pixels, binary, Simrad.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_NO;
	*nav_source = MB_DATA_NAV;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_ATTITUDE;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
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
int mbr_alm_em710raw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_em710raw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	*databyteswapped;
	double	*pixel_size;
	double	*swath_width;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	status = mbsys_simrad3_alloc(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);

	/* initialize saved values */
	databyteswapped = (int *) &mb_io_ptr->save10;
	pixel_size = &mb_io_ptr->saved1;
	swath_width = &mb_io_ptr->saved2;
	*databyteswapped = -1;
	*pixel_size = 0.0;
	*swath_width = 0.0;

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
int mbr_dem_em710raw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_em710raw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mbsys_simrad3_deall(
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
int mbr_rt_em710raw(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_em710raw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_attitude_struct *attitude;
	struct mbsys_simrad3_heading_struct *heading;
	struct mbsys_simrad3_ssv_struct *ssv;
	struct mbsys_simrad3_ping_struct *ping;
	int	time_i[7];
	double	ntime_d, ptime_d, atime_d;
	double	bath_time_d, ss_time_d;
	double	rawspeed, pheading;
	double	plon, plat, pspeed, roll, pitch, heave;
	double	soundspeed;
	double	receive_time_d, receive_roll, receive_pitch, receive_heave;
	double	transmit_time_d, transmit_roll, transmit_pitch, transmit_heave;
	double	transmit_alongtrack;
	double	alpha, beta, theta, phi;
	double	xx, yy, rr, zz;
	double	*pixel_size, *swath_width;
	mb_u_char detection_mask;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* read next data from file */
	status = mbr_em710raw_rd_data(verbose,mbio_ptr,store_ptr,error);

	/* get pointers to data structures */
	store = (struct mbsys_simrad3_struct *) store_ptr;
	attitude = (struct mbsys_simrad3_attitude_struct *) store->attitude;
	heading = (struct mbsys_simrad3_heading_struct *) store->heading;
	ssv = (struct mbsys_simrad3_ssv_struct *) store->ssv;
	ping = (struct mbsys_simrad3_ping_struct *) store->ping;
	pixel_size = (double *) &mb_io_ptr->saved1;
	swath_width = (double *) &mb_io_ptr->saved2;

	/* save fix if nav data */
	if (status == MB_SUCCESS
		&& store->kind == MB_DATA_NAV)
		{
		/* get nav time */
		time_i[0] = store->pos_date / 10000;
		time_i[1] = (store->pos_date % 10000) / 100;
		time_i[2] = store->pos_date % 100;
		time_i[3] = store->pos_msec / 3600000;
		time_i[4] = (store->pos_msec % 3600000) / 60000;
		time_i[5] = (store->pos_msec % 60000) / 1000;
		time_i[6] = (store->pos_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &ntime_d);
		
		/* add latest fix */
		if (store->pos_longitude != EM3_INVALID_INT
			&& store->pos_latitude != EM3_INVALID_INT)
			mb_navint_add(verbose, mbio_ptr, 
				ntime_d, 
				(double)(0.0000001 * store->pos_longitude), 
				(double)(0.00000005 * store->pos_latitude), 
				error);
		}

	/* save attitude if attitude data */
	if (status == MB_SUCCESS
		&& store->kind == MB_DATA_ATTITUDE)
		{
		/* get attitude time */
		time_i[0] = attitude->att_date / 10000;
		time_i[1] = (attitude->att_date % 10000) / 100;
		time_i[2] = attitude->att_date % 100;
		time_i[3] = attitude->att_msec / 3600000;
		time_i[4] = (attitude->att_msec % 3600000) / 60000;
		time_i[5] = (attitude->att_msec % 60000) / 1000;
		time_i[6] = (attitude->att_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &atime_d);
		
		/* add latest attitude samples */
		for (i=0;i<attitude->att_ndata;i++)
			{
			mb_attint_add(verbose, mbio_ptr,
				(double)(atime_d + 0.001 * attitude->att_time[i]),
				(double)(0.01 * attitude->att_heave[i]),
				(double)(0.01 * attitude->att_roll[i]),
				(double)(0.01 * attitude->att_pitch[i]),
				error);
			}
		}

	/* interpolate attitude data into navigation records */
	if (status == MB_SUCCESS
		&& (store->kind == MB_DATA_NAV
			|| store->kind == MB_DATA_NAV1
			|| store->kind == MB_DATA_NAV2
			|| store->kind == MB_DATA_NAV3))
		{
		/* get nav time */
		time_i[0] = store->pos_date / 10000;
		time_i[1] = (store->pos_date % 10000) / 100;
		time_i[2] = store->pos_date % 100;
		time_i[3] = store->pos_msec / 3600000;
		time_i[4] = (store->pos_msec % 3600000) / 60000;
		time_i[5] = (store->pos_msec % 60000) / 1000;
		time_i[6] = (store->pos_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &ntime_d);

		/* interpolate from saved attitude */
		mb_attint_interp(verbose, mbio_ptr, ntime_d,  
				    &heave, &roll, &pitch, error);
		store->pos_roll = (int) rint(roll / 0.01);
		store->pos_pitch = (int) rint(pitch / 0.01);
		store->pos_heave = (int) rint(heave / 0.01);
		}
	
	/* if no sidescan read then zero sidescan data */
	if (status == MB_SUCCESS 
		&& store->kind == MB_DATA_DATA
		&& ping->png_ss2_read == MB_NO)
		{
		status = mbsys_simrad3_zero_ss(verbose,store_ptr,error);
		}
	
	/* else check that bath and sidescan data record time stamps
	   match for survey data - we can have bath without
	   sidescan but not sidescan without bath */
	else if (status == MB_SUCCESS 
		&& store->kind == MB_DATA_DATA)
		{
		/* get times of bath and sidescan records */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &bath_time_d);
		time_i[0] = ping->png_ss_date / 10000;
		time_i[1] = (ping->png_ss_date % 10000) / 100;
		time_i[2] = ping->png_ss_date % 100;
		time_i[3] = ping->png_ss_msec / 3600000;
		time_i[4] = (ping->png_ss_msec % 3600000) / 60000;
		time_i[5] = (ping->png_ss_msec % 60000) / 1000;
		time_i[6] = (ping->png_ss_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &ss_time_d);
/* fprintf(stderr,"Check: png_count:%d png_raw_count:%d png_ss_count:%d    Beams:%d %d %d\n",
ping->png_count,ping->png_raw_count,ping->png_ss_count,ping->png_nbeams,ping->png_raw_nbeams,ping->png_nbeams_ss);*/
		
		/* check for time match - if bath newer than
		   sidescan then zero sidescan,  if sidescan
		   newer than bath then set error,  if ok then
		   check that beam ids are the same */
		if (ping->png_ss_date == 0
			|| ping->png_nbeams_ss == 0
			|| bath_time_d > ss_time_d)
		    {
		    status = mbsys_simrad3_zero_ss(verbose,store_ptr,error);
		    }
		else if (bath_time_d > ss_time_d)
		    {
		    if (verbose > 0)
		    	fprintf(stderr,"%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Sidescan zeroed, bathtime:%f >  sstime:%f\n",
				function_name, time_i[0], time_i[1], time_i[2], 
					time_i[3], time_i[4], time_i[5], time_i[6],
					bath_time_d, ss_time_d);
		    status = mbsys_simrad3_zero_ss(verbose,store_ptr,error);
		    }
		else if (bath_time_d < ss_time_d)
		    {
		    if (verbose > 0)
		    	fprintf(stderr,"%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Ping unintelligible bathtime:%f < sstime%f\n",
				function_name, time_i[0], time_i[1], time_i[2], 
					time_i[3], time_i[4], time_i[5], time_i[6],
					bath_time_d, ss_time_d);
		    *error = MB_ERROR_UNINTELLIGIBLE;
		    status = MB_FAILURE;
		    }
		else
		    {
		    /* check for some indicators of broken records */
		    if (ping->png_nbeams < ping->png_nbeams_ss
			|| ping->png_nbeams > ping->png_nbeams_ss + 1)
			{
		    	if (verbose > 1)
			    	fprintf(stderr,"%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Sidescan ignored: num bath beams != num ss beams: %d %d\n",
					function_name, time_i[0], time_i[1], time_i[2], 
					time_i[3], time_i[4], time_i[5], time_i[6],
					ping->png_nbeams, ping->png_nbeams_ss);
			}
		    }
		}

	/* add some calculated data to survey records */
	if (status == MB_SUCCESS
		&& store->kind == MB_DATA_DATA)
		{
/*fprintf(stderr, "mode:%d absorption:%d tran_pulse:%d tran_beam:%d tran_pow:%d rec_beam:%d rec_band:%d rec_gain:%d tvg_cross:%d\n", 
store->run_mode, store->run_absorption, 
store->run_tran_pulse, store->run_tran_pow, 
store->run_rec_beam, store->run_rec_band, 
store->run_rec_gain, store->run_tvg_cross);
fprintf(stderr, "max_range:%d r_zero:%d r_zero_corr:%d tvg_start:%d tvg_stop:%d bsn:%d bso:%d tx:%d tvg_crossover:%d\n", 
ping->png_max_range, ping->png_r_zero, 
ping->png_r_zero_corr, ping->png_tvg_start, 
ping->png_tvg_stop, ping->png_bsn, 
ping->png_bso, ping->png_tx, 
ping->png_tvg_crossover);
fprintf(stderr, "mode:%d depth:%11f max_range:%d r_zero:%d r_zero_corr:%d bsn:%d bso:%d\n", 
store->run_mode, 
0.01 * ping->png_depth_res * ping->png_depth[ping->png_nbeams/2], 
ping->png_max_range, ping->png_r_zero, 
ping->png_r_zero_corr, ping->png_bsn, 
ping->png_bso);*/

		/* get ping time */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &ptime_d);

		/* interpolate from saved nav */
		if (store->pos_speed == 0
		    || store->pos_speed == EM3_INVALID_SHORT)
			rawspeed = 0.0;
		else
			rawspeed =  0.036 * store->pos_speed;
		pheading = 0.01 * ping->png_heading;
		mb_navint_interp(verbose, mbio_ptr, ptime_d, pheading, rawspeed, 
				    &plon, &plat, &pspeed, error);
		if (plon == 0.0
		    && plat == 0.0)
		    {
		    ping->png_longitude = (int) EM3_INVALID_INT;
		    ping->png_latitude = (int) EM3_INVALID_INT;
		    }
		else
		    {
		    ping->png_longitude = (int) rint(10000000 * plon);
		    ping->png_latitude =  (int) rint(20000000 * plat);
		    }
		ping->png_speed = (int) rint(pspeed / 0.036);

		/* interpolate from saved attitude */
		mb_attint_interp(verbose, mbio_ptr, ptime_d,  
				    &heave, &roll, &pitch, error);
		ping->png_roll = (int) rint(roll / 0.01);
		ping->png_pitch = (int) rint(pitch / 0.01);
		ping->png_heave = (int) rint(heave / 0.01);
		
		/* calculate corrected ranges, angles, and bathymetry */
		for (i=0;i<ping->png_nbeams;i++)
			{
			/* get attitude and heave at ping and receive time */
			transmit_time_d = ptime_d + (double) ping->png_raw_txoffset[ping->png_raw_rxsector[i]];
			mb_attint_interp(verbose, mbio_ptr, transmit_time_d,  
				    		&transmit_heave, &transmit_roll, &transmit_pitch, error);
			receive_time_d = transmit_time_d + ping->png_raw_rxrange[i];
			mb_attint_interp(verbose, mbio_ptr, receive_time_d,  
				    		&receive_heave, &receive_roll, &receive_pitch, error);
			
			/* alongtrack offset distance */
			transmit_alongtrack = (100.0 * ((double)ping->png_speed)) 
						* ((double) ping->png_raw_txoffset[ping->png_raw_rxsector[i]]);
	
			/* get corrected range */
			if (ping->png_ssv <= 0)
				ping->png_ssv = 150;
			soundspeed = 0.1 * ((double)ping->png_ssv);
			ping->png_range[i] = ping->png_raw_rxrange[i] 
						- (receive_heave - transmit_heave) / soundspeed;
						
			

			/* calculate bathymetry */
			alpha = (0.01 * (double)ping->png_raw_txtiltangle[ping->png_raw_rxsector[i]]) + transmit_pitch;
			beta = 90.0 - (0.01 * (double)ping->png_raw_rxpointangle[i]) - receive_roll;
			mb_rollpitch_to_takeoff(
				verbose, 
				alpha, beta, 
				&theta, &phi, 
				error);
			/*rr = 0.5 * soundspeed * ping->png_range[i];
			xx = rr * sin(DTR * theta);
			zz = rr * cos(DTR * theta);
			ping->png_acrosstrack[i] = xx * cos(DTR * phi);
			ping->png_alongtrack[i] = xx * sin(DTR * phi) + transmit_alongtrack;
			ping->png_depth[i] = zz + ping->png_xducer_depth;*/
			ping->png_depression[i] = theta;
			ping->png_azimuth[i] = phi;
			
			/* calculate beamflag */
			detection_mask = (mb_u_char) ping->png_raw_rxdetection[i];
			if (((detection_mask & 128) == 128) && (((detection_mask & 32) == 32) || ((detection_mask & 24) == 24)))
				{
				ping->png_beamflag[i] = MB_FLAG_NULL;
				}
			else if ((detection_mask & 128) == 128)
				{
				ping->png_beamflag[i] = MB_FLAG_FLAG + MB_FLAG_SONAR;
				}
			else
				{
				ping->png_beamflag[i] = MB_FLAG_NONE;
				}
			}

		/* generate processed sidescan */
		ping->png_pixel_size = 0;
		ping->png_pixels_ss = 0;
		status = mbsys_simrad3_makess(verbose,
				mbio_ptr, store_ptr,
				MB_NO, pixel_size, 
				MB_NO, swath_width, 
				1, 
				error);
		}

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

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
int mbr_wt_em710raw(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_em710raw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_simrad3_struct *) store_ptr;

	/* write next data to file */
	status = mbr_em710raw_wr_data(verbose,mbio_ptr,store_ptr,error);

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
int mbr_em710raw_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_em710raw_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;
	FILE	*mbfp;
	int	swap = -1;
	int	done;
	int	*databyteswapped;
	int	record_size;
	int	*record_size_save;
	int	bytes_read;
	char	*label;
	int	*label_save_flag;
	char	*record_size_char;
	short	expect;
	short	type;
	short	sonar;
	int	*version;
	short	first_type;
	short	*expect_save;
	int	*expect_save_flag;
	short	*first_type_save;
	short	*typelast;
	short	*sonarlast;
	int	*nbadrec;
	int     *length;
	int	good_end_bytes;
	int	match;
	int	read_len;
	int	skip = 0;
	char	junk;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_simrad3_struct *) store_ptr;
	ping = (struct mbsys_simrad3_ping_struct *) store->ping;
	mbfp = mb_io_ptr->mbfp;
	
	/* get saved values */
	databyteswapped = (int *) &mb_io_ptr->save10;
	record_size_save = (int *) &mb_io_ptr->save5;
	label = (char *) mb_io_ptr->save_label;
	version = (int *) (&mb_io_ptr->save3);
	label_save_flag = (int *) &mb_io_ptr->save_label_flag;
	expect_save_flag = (int *) &mb_io_ptr->save_flag;
	expect_save = (short *) &mb_io_ptr->save1;
	first_type_save = (short *) &mb_io_ptr->save2;
	typelast = (short *) &mb_io_ptr->save6;
	sonarlast = (short *) &mb_io_ptr->save9;
	nbadrec = (int *) &mb_io_ptr->save7;
	length = (int *) &mb_io_ptr->save8;
	record_size_char = (char *) &record_size;
	if (*expect_save_flag == MB_YES)
		{
		expect = *expect_save;
		first_type = *first_type_save;
		*expect_save_flag = MB_NO;
		}
	else
		{
		expect = EM3_NONE;
		first_type = EM3_NONE;
		if (ping != NULL)
		    {
		    ping->png_raw4_read = MB_NO;
		    ping->png_ss2_read = MB_NO;
		    ping->png_raw_nbeams = 0;
		    ping->png_nbeams_ss = 0;
		    }
		}

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* set flag to swap bytes if necessary */
	swap =  *databyteswapped;

	/* loop over reading data until a record is ready for return */
	done = MB_NO;
	*error = MB_ERROR_NO_ERROR;
	while (done == MB_NO)
		{
		/* if no label saved get next record label */
		if (*label_save_flag == MB_NO)
			{
			/* read four byte record size */
			if ((read_len = fread(&record_size,
				1,4,mb_io_ptr->mbfp)) != 4)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
				
			/* read label */
			if ((read_len = fread(label,
				1,4,mb_io_ptr->mbfp)) != 4)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}

			/* check label - if not a good label read a byte 
				at a time until a good label is found */
			skip = 0;
			while (status == MB_SUCCESS
				&& mbr_em710raw_chk_label(verbose, 
					mbio_ptr, label, &type, &sonar) != MB_SUCCESS)
			    {
			    /* get next byte */
			    for (i=0;i<3;i++)
				record_size_char[i] = record_size_char[i+1];
			    record_size_char[3] = label[0];
			    for (i=0;i<3;i++)
				label[i] = label[i+1];
			    if ((read_len = fread(&label[3],
				    1,1,mb_io_ptr->mbfp)) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			    skip++;
			    }
			    
			/* report problem */
			if (skip > 0 && verbose > 0)
			    {
			    if (*nbadrec == 0)
			    	fprintf(stderr, 
"\nThe MBF_EM710RAW module skipped data between identified\n\
data records. Something is broken, most probably the data...\n\
However, the data may include a data record type that we\n\
haven't seen yet, or there could be an error in the code.\n\
If skipped data are reported multiple times, \n\
we recommend you send a data sample and problem \n\
description to the MB-System team \n\
(caress@mbari.org and dale@ldeo.columbia.edu)\n\
Have a nice day...\n");
				fprintf(stderr,
						"MBF_EM710RAW skipped %d bytes between records %4.4hX:%d and %4.4hX:%d\n",
						skip, *typelast, *typelast, type, type);
				(*nbadrec)++;
			    }
			*typelast = type;
			*sonarlast = sonar;

			/* set flag to swap MBR_EM710RAW_DEBUGbytes if necessary */
			swap = *databyteswapped;

			/* get record_size */
			if (*databyteswapped != mb_io_ptr->byteswapped)
				record_size = mb_swap_int(record_size);
			*record_size_save = record_size;
			}
		
		/* else use saved label */
		else
			{
			*label_save_flag = MB_NO;
			type = *typelast;
			sonar = *sonarlast;
			record_size = *record_size_save;
			}

#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"\nstart of mbr_em710raw_rd_data loop:\n");
	fprintf(stderr,"skip:%d expect:%x type:%x first_type:%x sonar:%d recsize:%u done:%d\n",
		skip, expect, type, first_type, sonar, *record_size_save, done);
#endif
		
		/* allocate secondary data structure for
			heading data if needed */
		if (status == MB_SUCCESS && 
			(type == EM3_HEADING)
			&& store->heading == NULL)
			{
			status = mbsys_simrad3_heading_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}
		
		/* allocate secondary data structure for
			attitude data if needed */
		if (status == MB_SUCCESS && 
			(type == EM3_ATTITUDE)
			&& store->attitude == NULL)
			{
			status = mbsys_simrad3_attitude_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}
		
		/* allocate secondary data structure for
			ssv data if needed */
		if (status == MB_SUCCESS && 
			(type == EM3_SSV)
			&& store->ssv == NULL)
			{
			status = mbsys_simrad3_ssv_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}
		
		/* allocate secondary data structure for
			tilt data if needed */
		if (status == MB_SUCCESS && 
			(type == EM3_TILT)
			&& store->tilt == NULL)
			{
			status = mbsys_simrad3_tilt_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}
		
		/* allocate secondary data structure for
			survey data if needed */
		if (status == MB_SUCCESS && 
			(type == EM3_BATH2
			|| type == EM3_RAWBEAM4
			|| type == EM3_SS2))
			{
			if (store->ping == NULL)
			    status = mbsys_simrad3_survey_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			ping = (struct mbsys_simrad3_ping_struct *) store->ping;
			}
		
		/* allocate secondary data structure for
			water column data if needed */
		if (status == MB_SUCCESS && 
			(type == EM3_WATERCOLUMN))
			{
			if (store->wc == NULL)
			    status = mbsys_simrad3_wc_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}

		/* read the appropriate data records */
		if (status == MB_FAILURE && expect == EM3_NONE)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call nothing, read failure, no expect\n");
#endif
			done = MB_YES;
			record_size = 0;
			*record_size_save = record_size;
			}
		else if (status == MB_FAILURE && expect != EM3_NONE)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call nothing, read failure, expect %x\n",expect);
#endif
			done = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else if (type !=  EM3_STOP2
			&& type != EM3_STATUS
			&& type != EM3_ON
			&& type != EM3_ATTITUDE
			&& type != EM3_CLOCK
			&& type != EM3_BATH
			&& type != EM3_SBDEPTH
			&& type != EM3_RAWBEAM
			&& type != EM3_SSV
			&& type != EM3_HEADING
			&& type != EM3_START
			&& type != EM3_TILT
			&& type != EM3_CBECHO
			&& type != EM3_RAWBEAM4
			&& type != EM3_POS
			&& type != EM3_RUN_PARAMETER
			&& type != EM3_SS
			&& type != EM3_TIDE
			&& type != EM3_SVP2
			&& type != EM3_SVP
			&& type != EM3_SSPINPUT
			&& type != EM3_BATH2
			&& type != EM3_SS2
			&& type != EM3_RAWBEAM2
			&& type != EM3_RAWBEAM3
			&& type != EM3_HEIGHT
			&& type != EM3_STOP
			&& type != EM3_WATERCOLUMN
			&& type != EM3_REMOTE
			&& type != EM3_SSP
			&& type != EM3_BATH_MBA
			&& type != EM3_SS_MBA
			&& type != EM3_BATH2_MBA
			&& type != EM3_SS2_MBA)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call nothing, try again\n");
#endif
			done = MB_NO;
			}
		else if (type == EM3_STATUS)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_status type %x\n",type);
#endif
			status = mbr_em710raw_rd_status(
				verbose,mbfp,swap,store,type,sonar,&good_end_bytes,error);
			if (status == MB_SUCCESS)
			    {
			    done = MB_YES;
			    if (expect != EM3_NONE)
				{
				*expect_save = expect;
				*expect_save_flag = MB_YES;
				*first_type_save = first_type;
				}
			    else
				*expect_save_flag = MB_NO;
			    }
			}
		else if (type == EM3_START
			|| type == EM3_STOP)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_start type %x\n",type);
#endif
			status = mbr_em710raw_rd_start(
				verbose,mbfp,swap,store,type,sonar,version,&good_end_bytes,error);
			if (status == MB_SUCCESS)
			    {
			    done = MB_YES;
			    if (expect != EM3_NONE)
				{
				*expect_save = expect;
				*expect_save_flag = MB_YES;
				*first_type_save = first_type;
				}
			    else
				*expect_save_flag = MB_NO;
			    }
			}
		else if (type == EM3_RUN_PARAMETER)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_run_parameter type %x\n",type);
#endif
			status = mbr_em710raw_rd_run_parameter(
				verbose,mbfp,swap,store,sonar,&good_end_bytes,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM3_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (type == EM3_CLOCK)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_clock type %x\n",type);
#endif
			status = mbr_em710raw_rd_clock(
				verbose,mbfp,swap,store,sonar,&good_end_bytes,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM3_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (type == EM3_TIDE)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_tide type %x\n",type);
#endif
			status = mbr_em710raw_rd_tide(
				verbose,mbfp,swap,store,sonar,&good_end_bytes,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM3_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (type == EM3_HEIGHT)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_height type %x\n",type);
#endif
			status = mbr_em710raw_rd_height(
				verbose,mbfp,swap,store,sonar,&good_end_bytes,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM3_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (type == EM3_HEADING)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_heading type %x\n",type);
#endif
			status = mbr_em710raw_rd_heading(
				verbose,mbfp,swap,store,sonar,&good_end_bytes,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM3_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (type == EM3_SSV)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_ssv type %x\n",type);
#endif
			status = mbr_em710raw_rd_ssv(
				verbose,mbfp,swap,store,sonar,&good_end_bytes,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM3_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (type == EM3_TILT)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_tilt type %x\n",type);
#endif
			status = mbr_em710raw_rd_tilt(
				verbose,mbfp,swap,store,sonar,&good_end_bytes,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM3_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (type == EM3_ATTITUDE)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_attitude type %x\n",type);
#endif
			status = mbr_em710raw_rd_attitude(
				verbose,mbfp,swap,store,sonar,&good_end_bytes,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM3_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}	
		else if (type == EM3_POS)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_pos type %x\n",type);
#endif
			status = mbr_em710raw_rd_pos(
				verbose,mbfp,swap,store,sonar,&good_end_bytes,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM3_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (type == EM3_SVP)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_svp type %x\n",type);
#endif
			status = mbr_em710raw_rd_svp(
				verbose,mbfp,swap,store,sonar,&good_end_bytes,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM3_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (type == EM3_SVP2)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_svp2 type %x\n",type);
#endif
			status = mbr_em710raw_rd_svp2(
				verbose,mbfp,swap,store,sonar,&good_end_bytes,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM3_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (type == EM3_BATH2 
			&& expect == EM3_SS2)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,type);
#endif
			done = MB_YES;
			expect = EM3_NONE;
			type = first_type;
			*label_save_flag = MB_YES;
			store->kind = MB_DATA_DATA;
			}
		else if (type == EM3_BATH2)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_bath2 type %x\n",type);
#endif
			status = mbr_em710raw_rd_bath2(
				verbose,mbfp,swap,store,&match,sonar,*version,&good_end_bytes,error);
			if (status == MB_SUCCESS)
				{
				if (first_type == EM3_NONE
					|| match == MB_NO)
					{
					done = MB_NO;
					first_type = EM3_BATH2;
					expect = EM3_SS2;
					}
				else
					{
					done = MB_YES;
					expect = EM3_NONE;
					}
				}
			}
		else if (type == EM3_RAWBEAM4)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_rawbeam4 type %x\n",type);
#endif
			status = mbr_em710raw_rd_rawbeam4(
				verbose,mbfp,swap,store,sonar,&good_end_bytes,error);
			if (status == MB_SUCCESS)
				ping->png_raw4_read = MB_YES;
			if (expect == EM3_SS2
				&& ping->png_nbeams == 0)
				{
				done = MB_YES;
				expect = EM3_NONE;
				}
			}
		else if (type == EM3_SS2 
			&& expect != EM3_NONE 
			&& expect != EM3_SS2)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,type);
#endif
			done = MB_YES;
			expect = EM3_NONE;
			type = first_type;
			*label_save_flag = MB_YES;
			store->kind = MB_DATA_DATA;
			}
		else if (type == EM3_SS2)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_ss2 type %x\n",type);
#endif
			status = mbr_em710raw_rd_ss2(
				verbose,mbfp,swap,store,sonar,*length,&match,&good_end_bytes,error);
			if (status == MB_SUCCESS)
			    {
			    ping->png_ss2_read = MB_YES;
			    if (first_type == EM3_NONE
				|| match == MB_NO)
				{
				done = MB_NO;
				first_type = EM3_SS2;
				expect = EM3_BATH2;
				}
			    else
				{
				done = MB_YES;
				expect = EM3_NONE;
				}
			    }

                        /* salvage bath even if sidescan is corrupt */
			else
			    {
			    if (first_type == EM3_BATH2 
				&& match == MB_YES)
				{
				status = MB_SUCCESS;
				done = MB_YES;
				expect = EM3_NONE;
				}
			    }
			}
		else if (type == EM3_WATERCOLUMN)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_rd_wc type %x\n",type);
#endif
			status = mbr_em710raw_rd_wc(
				verbose,mbfp,swap,store,sonar,&good_end_bytes,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM3_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"skip over %d bytes of unsupported datagram type %x\n",
			*record_size_save, type);
#endif
			for (i=0;i<*record_size_save-4;i++)
				{
				if ((read_len = fread(&junk,
					1,1,mb_io_ptr->mbfp)) != 1)
					{
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					expect = EM3_NONE;
					}
				}
			done = MB_NO;
			}

		/* bail out if there is an error */
		if (status == MB_FAILURE)
			done = MB_YES;
			
		/* if necessary read over unread but expected bytes */
		bytes_read = ftell(mbfp) - mb_io_ptr->file_bytes - 4;
		if (*label_save_flag == MB_NO && good_end_bytes == MB_NO 
			&& bytes_read < record_size)
			{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"skip over %d unread bytes of supported datagram type %x\n",
			record_size - bytes_read, type);
#endif
			for (i=0;i<record_size - bytes_read;i++)
				{
				if ((read_len = fread(&junk,
					1,1,mb_io_ptr->mbfp)) != 1)
					{
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					expect = EM3_NONE;
					}
				}
			}

#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"record_size:%d bytes read:%d file_pos old:%d new:%d\n", 
		record_size, ftell(mbfp) - mb_io_ptr->file_bytes, mb_io_ptr->file_bytes, ftell(mbfp));
	fprintf(stderr,"done:%d expect:%x status:%d error:%d\n", 
		done, expect, status, *error);
	fprintf(stderr,"end of mbr_em710raw_rd_data loop:\n\n");
#endif
		
		/* get file position */
		if (*label_save_flag == MB_YES)
			mb_io_ptr->file_bytes = ftell(mbfp) - 2;
		else
			mb_io_ptr->file_bytes = ftell(mbfp);
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
int mbr_em710raw_chk_label(int verbose, void *mbio_ptr, char *label, short *type, short *sonar)
{
	char	*function_name = "mbr_em710raw_chk_label";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	mb_u_char	startbyte;
	mb_u_char	typebyte;
	short	*sonar_save;
	short	sonarunswap;
	short	sonarswap;
	int	swap;
	int	*databyteswapped;
	int	typegood;
	int	sonargood;
	int	sonarswapgood;
	int	sonarunswapgood;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       label:      %x%x%x%x\n",label[0],label[1],label[2],label[3]);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	sonar_save = (short *) (&mb_io_ptr->save4);
	databyteswapped = (int *) &mb_io_ptr->save10;
	
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "Check label: %x|%x|%x|%x\n", label[0],label[1],label[2],label[3]);
#endif
		
	/* check for valid start byte and type */
	startbyte = label[0];
	typebyte = label[1];
	if (startbyte ==  EM3_START_BYTE && 
		(typebyte == EM3_ID_STOP2
		|| typebyte == EM3_ID_STATUS
		|| typebyte == EM3_ID_ON
		|| typebyte == EM3_ID_ATTITUDE
		|| typebyte == EM3_ID_CLOCK
		|| typebyte == EM3_ID_BATH
		|| typebyte == EM3_ID_SBDEPTH
		|| typebyte == EM3_ID_RAWBEAM
		|| typebyte == EM3_ID_SSV
		|| typebyte == EM3_ID_HEADING
		|| typebyte == EM3_ID_START
		|| typebyte == EM3_ID_TILT
		|| typebyte == EM3_ID_CBECHO
		|| typebyte == EM3_ID_RAWBEAM4
		|| typebyte == EM3_ID_POS
		|| typebyte == EM3_ID_RUN_PARAMETER
		|| typebyte == EM3_ID_SS
		|| typebyte == EM3_ID_TIDE
		|| typebyte == EM3_ID_SVP2
		|| typebyte == EM3_ID_SVP
		|| typebyte == EM3_ID_SSPINPUT
		|| typebyte == EM3_ID_BATH2
		|| typebyte == EM3_ID_SS2
		|| typebyte == EM3_ID_RAWBEAM2
		|| typebyte == EM3_ID_RAWBEAM3
		|| typebyte == EM3_ID_HEIGHT
		|| typebyte == EM3_ID_STOP
		|| typebyte == EM3_ID_WATERCOLUMN
		|| typebyte == EM3_ID_REMOTE
		|| typebyte == EM3_ID_SSP
		|| typebyte == EM3_ID_BATH_MBA
		|| typebyte == EM3_ID_SS_MBA
		|| typebyte == EM3_ID_BATH2_MBA
		|| typebyte == EM3_ID_SS2_MBA))
		{
		typegood = MB_YES;
		}
	else
		{
		typegood = MB_NO;
		}
		
	/* check for data byte swapping if necessary */
	if (typegood == MB_YES && *databyteswapped == -1)
		{
		sonarunswap = *((short *)&label[2]);
		sonarswap = mb_swap_short(sonarunswap);

		/* check for valid sonarunswap */
		if (sonarunswap == MBSYS_SIMRAD3_EM710)
			{
			sonarunswapgood = MB_YES;
			}
		else
			{
			sonarunswapgood = MB_NO;
			}

		/* check for valid sonarswap */
		if (sonarswap == MBSYS_SIMRAD3_EM710)
			{
			sonarswapgood = MB_YES;
			}
		else
			{
			sonarswapgood = MB_NO;
			}
			
		if (sonarunswapgood == MB_YES && sonarswapgood == MB_NO)
			{
			if (mb_io_ptr->byteswapped == MB_YES)
				*databyteswapped = MB_YES;
			else
				*databyteswapped = MB_NO;
			}
		else if (sonarunswapgood == MB_NO && sonarswapgood == MB_YES)
			{
			if (mb_io_ptr->byteswapped == MB_YES)
				*databyteswapped = MB_NO;
			else
				*databyteswapped = MB_YES;
			}
		
		}

	/* set flag to swap bytes if necessary */
	swap =  *databyteswapped;
		
	*type = *((short *)&label[0]);
	*sonar = *((short *)&label[2]);
	if (mb_io_ptr->byteswapped == MB_YES)
		*type = mb_swap_short(*type);
	if (*databyteswapped != mb_io_ptr->byteswapped)
		{
		*sonar = mb_swap_short(*sonar);
		}

#ifdef MBR_EM710RAW_DEBUG
fprintf(stderr,"typegood:%d mb_io_ptr->byteswapped:%d sonarswapgood:%d *databyteswapped:%d *type:%d *sonar:%d\n",typegood,mb_io_ptr->byteswapped,sonarswapgood,*databyteswapped,*type,*sonar);
#endif
		
	/* check for valid sonar */
	if (*sonar != MBSYS_SIMRAD3_EM710)
		{
		sonargood = MB_NO;
		}
	else
		{
		sonargood = MB_YES;
		}
	
	if (startbyte == EM3_START_BYTE && typegood == MB_NO && sonargood == MB_YES)
		{
		mb_notice_log_problem(verbose, mbio_ptr, 
			MB_PROBLEM_BAD_DATAGRAM);
		if (verbose >= 1)
		    fprintf(stderr, "Bad datagram type: %4.4hX %4.4hX | %d %d\n", *type, *sonar, *type, *sonar);
		}
	if (typegood != MB_YES || sonargood != MB_YES)
		{
		status = MB_FAILURE;
		}
			
	/* save sonar if successful */
	if (status == MB_SUCCESS)
	    *sonar_save = *sonar;
		
	/* allow exception found in some data */
	if (*type == EM3_SSV && *sonar == 0 && *sonar_save != 0)
		{
		status = MB_SUCCESS;
		*sonar = *sonar_save;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       type:       %d\n",*type);
		fprintf(stderr,"dbg2       sonar:      %d\n",*sonar);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_status(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short type, short sonar, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_status";
	int	status = MB_SUCCESS;
	char	line[EM3_STATUS_SIZE];
	short	short_val;
	int	read_len;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* set kind and type values */
	store->kind = MB_DATA_STATUS;
	store->type = EM3_STATUS;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM3_STATUS_SIZE-4,mbfp);
	if (read_len == EM3_STATUS_SIZE-4)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &store->sts_date); 
		    if (store->sts_date != 0) store->date = store->sts_date;
		mb_get_binary_int(swap, &line[4], &store->sts_msec); 
		    if (store->sts_date != 0) store->msec = store->sts_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    store->sts_status_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    store->sts_serial = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[12], &short_val); 
		    store->sts_pingrate = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[14], &short_val); 
		    store->sts_ping_count = (int) ((unsigned short) short_val);
		mb_get_binary_int(swap, &line[16], &store->sts_load); 
		mb_get_binary_int(swap, &line[20], &store->sts_udp_status); 
		mb_get_binary_int(swap, &line[24], &store->sts_serial1_status); 
		mb_get_binary_int(swap, &line[28], &store->sts_serial2_status); 
		mb_get_binary_int(swap, &line[32], &store->sts_serial3_status); 
		mb_get_binary_int(swap, &line[36], &store->sts_serial4_status); 
		store->sts_pps_status = (mb_u_char) line[40];
		store->sts_position_status = (mb_s_char) line[41];
		store->sts_attitude_status = (mb_s_char) line[42];
		store->sts_clock_status = (mb_s_char) line[43];
		store->sts_heading_status = (mb_s_char) line[44];
		store->sts_pu_status = (mb_u_char) line[45];
		mb_get_binary_short(swap, &line[46], &short_val); 
		    store->sts_last_heading = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[48], &short_val); 
		    store->sts_last_roll = (int) ((short) short_val);
		mb_get_binary_short(swap, &line[50], &short_val); 
		    store->sts_last_pitch = (int) ((short) short_val);
		mb_get_binary_short(swap, &line[52], &short_val); 
		    store->sts_last_heave = (int) ((short) short_val);
		mb_get_binary_short(swap, &line[54], &short_val); 
		    store->sts_last_ssv = (int) ((unsigned short) short_val);
		mb_get_binary_int(swap, &line[56], &store->sts_last_depth); 
		mb_get_binary_int(swap, &line[60], &store->sts_spare); 
		store->sts_bso = (mb_s_char) line[64];
		store->sts_bsn = (mb_s_char) line[65];
		store->sts_gain = (mb_s_char) line[66];
		store->sts_dno = (mb_u_char) line[67];
		mb_get_binary_short(swap, &line[68], &short_val); 
		    store->sts_rno = (int) ((unsigned short) short_val);
		store->sts_port = (mb_s_char) line[70];
		store->sts_stbd = (mb_u_char) line[71];
		mb_get_binary_short(swap, &line[72], &short_val); 
		    store->sts_ssp = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[74], &short_val); 
		    store->sts_yaw = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[76], &short_val); 
		    store->sts_port2 = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[78], &short_val); 
		    store->sts_stbd2 = (int) ((unsigned short) short_val);
		store->sts_spare2 = (mb_u_char) line[80];
		if (line[EM3_STATUS_SIZE-7] == EM3_END)
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[EM3_STATUS_SIZE-7], line[EM3_STATUS_SIZE-7], 
		line[EM3_STATUS_SIZE-6], line[EM3_STATUS_SIZE-6], 
		line[EM3_STATUS_SIZE-5], line[EM3_STATUS_SIZE-5]);
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:                %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:               %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:                %d\n",store->date);
		fprintf(stderr,"dbg5       msec:                %d\n",store->msec);
		fprintf(stderr,"dbg5       run_date:            %d\n",store->sts_date);
		fprintf(stderr,"dbg5       run_msec:            %d\n",store->sts_msec);
		fprintf(stderr,"dbg5       sts_status_count:    %d\n",store->sts_status_count);
		fprintf(stderr,"dbg5       run_serial:          %d\n",store->sts_serial);
		fprintf(stderr,"dbg5       sts_pingrate:        %d\n",store->sts_pingrate);
		fprintf(stderr,"dbg5       sts_ping_count:      %d\n",store->sts_ping_count);
		fprintf(stderr,"dbg5       sts_load:            %d\n",store->sts_load);
		fprintf(stderr,"dbg5       sts_udp_status:      %d\n",store->sts_udp_status);
		fprintf(stderr,"dbg5       sts_serial1_status:  %d\n",store->sts_serial1_status);
		fprintf(stderr,"dbg5       sts_serial2_status:  %d\n",store->sts_serial2_status);
		fprintf(stderr,"dbg5       sts_serial3_status:  %d\n",store->sts_serial3_status);
		fprintf(stderr,"dbg5       sts_serial4_status:  %d\n",store->sts_serial4_status);
		fprintf(stderr,"dbg5       sts_pps_status:      %d\n",store->sts_pps_status);
		fprintf(stderr,"dbg5       sts_position_status: %d\n",store->sts_position_status);
		fprintf(stderr,"dbg5       sts_attitude_status: %d\n",store->sts_attitude_status);
		fprintf(stderr,"dbg5       sts_clock_status:    %d\n",store->sts_clock_status);
		fprintf(stderr,"dbg5       sts_heading_status:  %d\n",store->sts_heading_status);
		fprintf(stderr,"dbg5       sts_pu_status:       %d\n",store->sts_pu_status);
		fprintf(stderr,"dbg5       sts_last_heading:    %d\n",store->sts_last_heading);
		fprintf(stderr,"dbg5       sts_last_roll:       %d\n",store->sts_last_roll);
		fprintf(stderr,"dbg5       sts_last_pitch:      %d\n",store->sts_last_pitch);
		fprintf(stderr,"dbg5       sts_last_heave:      %d\n",store->sts_last_heave);
		fprintf(stderr,"dbg5       sts_last_ssv:        %d\n",store->sts_last_ssv);
		fprintf(stderr,"dbg5       sts_last_heave:      %d\n",store->sts_last_heave);
		fprintf(stderr,"dbg5       sts_last_depth:      %d\n",store->sts_last_depth);
		fprintf(stderr,"dbg5       sts_spare:           %d\n",store->sts_spare);
		fprintf(stderr,"dbg5       sts_bso:             %d\n",store->sts_bso);
		fprintf(stderr,"dbg5       sts_bsn:             %d\n",store->sts_bsn);
		fprintf(stderr,"dbg5       sts_gain:            %d\n",store->sts_gain);
		fprintf(stderr,"dbg5       sts_dno:             %d\n",store->sts_dno);
		fprintf(stderr,"dbg5       sts_rno:             %d\n",store->sts_rno);
		fprintf(stderr,"dbg5       sts_port:            %d\n",store->sts_port);
		fprintf(stderr,"dbg5       sts_stbd:            %d\n",store->sts_stbd);
		fprintf(stderr,"dbg5       sts_ssp:             %d\n",store->sts_ssp);
		fprintf(stderr,"dbg5       sts_yaw:             %d\n",store->sts_yaw);
		fprintf(stderr,"dbg5       sts_port2:           %d\n",store->sts_port2);
		fprintf(stderr,"dbg5       sts_stbd2:           %d\n",store->sts_stbd2);
		fprintf(stderr,"dbg5       sts_spare2:          %d\n",store->sts_spare2);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_start(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short type, short sonar, int *version, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_start";
	int	status = MB_SUCCESS;
	char	line[MBSYS_SIMRAD3_BUFFER_SIZE];
	short	short_val;
	int	read_len, len;
	int	done;
	char	*comma_ptr;
	int	i1, i2, i3;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       type:       %d\n",type);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* make sure comment is initialized */
	store->par_com[0] = '\0';
	
	/* set type value */
	store->type = type;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM3_START_HEADER_SIZE,mbfp);
	if (read_len == EM3_START_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &store->par_date); 
		    store->date = store->par_date;
		mb_get_binary_int(swap, &line[4], &store->par_msec); 
		    store->msec = store->par_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    store->par_line_num = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    store->par_serial_1 = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[12], &short_val); 
		    store->par_serial_2 = (int) ((unsigned short) short_val);
		}
		
	/* now loop over reading individual characters to 
	    handle ASCII parameter values */
	done = MB_NO;
	len = 0;
	while (status == MB_SUCCESS && done == MB_NO)
		{
		read_len = fread(&line[len],1,1,mbfp);
		if (read_len == 1)
			{
			status = MB_SUCCESS;
			len++;
			}
		else
			{
			done = MB_YES;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		if (status == MB_SUCCESS 
			&& (((mb_u_char)(line[len-1])) < 32
			    || ((mb_u_char)(line[len-1])) > 127)
			&& ((mb_u_char)(line[len-1])) != '\r'
			&& ((mb_u_char)(line[len-1])) != '\n')
			{
			done = MB_YES;
			if (len > 1)
			    line[0] = line[len-1];
			}
		else if (status == MB_SUCCESS 
			&& line[len-1] == ','
			&& len > 5)
			{
			line[len] = 0;
			if (strncmp("WLZ=", line, 4) == 0)
			    mb_get_double(&(store->par_wlz), &line[4], len-5);
			else if (strncmp("SMH=", line, 4) == 0)
			    mb_get_int(&(store->par_smh), &line[4], len-5);
			else if (strncmp("S1Z=", line, 4) == 0)
			    mb_get_double(&(store->par_s1z), &line[4], len-5);
			else if (strncmp("S1X=", line, 4) == 0)
			    mb_get_double(&(store->par_s1x), &line[4], len-5);
			else if (strncmp("S1Y=", line, 4) == 0)
			    mb_get_double(&(store->par_s1y), &line[4], len-5);
			else if (strncmp("S1H=", line, 4) == 0)
			    mb_get_double(&(store->par_s1h), &line[4], len-5);
			else if (strncmp("S1R=", line, 4) == 0)
			    mb_get_double(&(store->par_s1r), &line[4], len-5);
			else if (strncmp("S1P=", line, 4) == 0)
			    mb_get_double(&(store->par_s1p), &line[4], len-5);
			else if (strncmp("S1N=", line, 4) == 0)
			    mb_get_int(&(store->par_s1n), &line[4], len-5);
			else if (strncmp("S2Z=", line, 4) == 0)
			    mb_get_double(&(store->par_s2z), &line[4], len-5);
			else if (strncmp("S2X=", line, 4) == 0)
			    mb_get_double(&(store->par_s2x), &line[4], len-5);
			else if (strncmp("S2Y=", line, 4) == 0)
			    mb_get_double(&(store->par_s2y), &line[4], len-5);
			else if (strncmp("S2H=", line, 4) == 0)
			    mb_get_double(&(store->par_s2h), &line[4], len-5);
			else if (strncmp("S2R=", line, 4) == 0)
			    mb_get_double(&(store->par_s2r), &line[4], len-5);
			else if (strncmp("S2P=", line, 4) == 0)
			    mb_get_double(&(store->par_s2p), &line[4], len-5);
			else if (strncmp("S2N=", line, 4) == 0)
			    mb_get_int(&(store->par_s2n), &line[4], len-5);
			else if (strncmp("GO1=", line, 4) == 0)
			    mb_get_double(&(store->par_go1), &line[4], len-5);
			else if (strncmp("GO2=", line, 4) == 0)
			    mb_get_double(&(store->par_go2), &line[4], len-5);
			else if (strncmp("TSV=", line, 4) == 0)
			    strncpy(store->par_tsv, &line[4], MIN(len-5, 15));
			else if (strncmp("RSV=", line, 4) == 0)
			    strncpy(store->par_rsv, &line[4], MIN(len-5, 15));
			else if (strncmp("BSV=", line, 4) == 0)
			    strncpy(store->par_bsv, &line[4], MIN(len-5, 15));
			else if (strncmp("PSV=", line, 4) == 0)
			    {
			    /* save the processor software version to use
			       in tracking changes to the data format */
			    strncpy(store->par_psv, &line[4], MIN(len-5, 15));
			    if (sscanf(store->par_psv, "%d.%d.%d", &i1, &i2, &i3) 
				== 3)
				*version = i3 + 100 * i2 + 10000 * i1;
			    }
			else if (strncmp("OSV=", line, 4) == 0)
			    strncpy(store->par_osv, &line[4], MIN(len-5, 15));
			else if (strncmp("DSD=", line, 4) == 0)
			    mb_get_double(&(store->par_dsd), &line[4], len-5);
			else if (strncmp("DSO=", line, 4) == 0)
			    mb_get_double(&(store->par_dso), &line[4], len-5);
			else if (strncmp("DSF=", line, 4) == 0)
			    mb_get_double(&(store->par_dsf), &line[4], len-5);
			else if (strncmp("DSH=", line, 4) == 0)
			    {
			    store->par_dsh[0] = line[4];
			    store->par_dsh[1] = line[5];
			    }
			else if (strncmp("APS=", line, 4) == 0)
			    mb_get_int(&(store->par_aps), &line[4], len-5);
			else if (strncmp("P1M=", line, 4) == 0)
			    mb_get_int(&(store->par_p1m), &line[4], len-5);
			else if (strncmp("P1T=", line, 4) == 0)
			    mb_get_int(&(store->par_p1t), &line[4], len-5);
			else if (strncmp("P1Z=", line, 4) == 0)
			    mb_get_double(&(store->par_p1z), &line[4], len-5);
			else if (strncmp("P1X=", line, 4) == 0)
			    mb_get_double(&(store->par_p1x), &line[4], len-5);
			else if (strncmp("P1Y=", line, 4) == 0)
			    mb_get_double(&(store->par_p1y), &line[4], len-5);
			else if (strncmp("P1D=", line, 4) == 0)
			    mb_get_double(&(store->par_p1d), &line[4], len-5);
			else if (strncmp("P1G=", line, 4) == 0)
			    strncpy(store->par_p1g, &line[4], MIN(len-5, 15));
			else if (strncmp("P2M=", line, 4) == 0)
			    mb_get_int(&(store->par_p2m), &line[4], len-5);
			else if (strncmp("P2T=", line, 4) == 0)
			    mb_get_int(&(store->par_p2t), &line[4], len-5);
			else if (strncmp("P2Z=", line, 4) == 0)
			    mb_get_double(&(store->par_p2z), &line[4], len-5);
			else if (strncmp("P2X=", line, 4) == 0)
			    mb_get_double(&(store->par_p2x), &line[4], len-5);
			else if (strncmp("P2Y=", line, 4) == 0)
			    mb_get_double(&(store->par_p2y), &line[4], len-5);
			else if (strncmp("P2D=", line, 4) == 0)
			    mb_get_double(&(store->par_p2d), &line[4], len-5);
			else if (strncmp("P2G=", line, 4) == 0)
			    strncpy(store->par_p2g, &line[4], MIN(len-5, 15));
			else if (strncmp("P3M=", line, 4) == 0)
			    mb_get_int(&(store->par_p3m), &line[4], len-5);
			else if (strncmp("P3T=", line, 4) == 0)
			    mb_get_int(&(store->par_p3t), &line[4], len-5);
			else if (strncmp("P3Z=", line, 4) == 0)
			    mb_get_double(&(store->par_p3z), &line[4], len-5);
			else if (strncmp("P3X=", line, 4) == 0)
			    mb_get_double(&(store->par_p3x), &line[4], len-5);
			else if (strncmp("P3Y=", line, 4) == 0)
			    mb_get_double(&(store->par_p3y), &line[4], len-5);
			else if (strncmp("P3D=", line, 4) == 0)
			    mb_get_double(&(store->par_p3d), &line[4], len-5);
			else if (strncmp("P3G=", line, 4) == 0)
			    strncpy(store->par_p3g, &line[4], MIN(len-5, 15));
			else if (strncmp("MSZ=", line, 4) == 0)
			    mb_get_double(&(store->par_msz), &line[4], len-5);
			else if (strncmp("MSX=", line, 4) == 0)
			    mb_get_double(&(store->par_msx), &line[4], len-5);
			else if (strncmp("MSY=", line, 4) == 0)
			    mb_get_double(&(store->par_msy), &line[4], len-5);
			else if (strncmp("MRP=", line, 4) == 0)
			    {
			    store->par_mrp[0] = line[4];
			    store->par_mrp[1] = line[5];
			    }
			else if (strncmp("MSD=", line, 4) == 0)
			    mb_get_double(&(store->par_msd), &line[4], len-5);
			else if (strncmp("MSR=", line, 4) == 0)
			    mb_get_double(&(store->par_msr), &line[4], len-5);
			else if (strncmp("MSP=", line, 4) == 0)
			    mb_get_double(&(store->par_msp), &line[4], len-5);
			else if (strncmp("MSG=", line, 4) == 0)
			    mb_get_double(&(store->par_msg), &line[4], len-5);
			else if (strncmp("GCG=", line, 4) == 0)
			    mb_get_double(&(store->par_gcg), &line[4], len-5);
			else if (strncmp("CPR=", line, 4) == 0)
			    strncpy(store->par_cpr, &line[4], MIN(len-5, 3));
			else if (strncmp("ROP=", line, 4) == 0)
			    strncpy(store->par_rop, &line[4], MIN(len-5, MBSYS_SIMRAD3_COMMENT_LENGTH-1));
			else if (strncmp("SID=", line, 4) == 0)
			    strncpy(store->par_sid, &line[4], MIN(len-5, MBSYS_SIMRAD3_COMMENT_LENGTH-1));
			else if (strncmp("PLL=", line, 4) == 0)
			    strncpy(store->par_pll, &line[4], MIN(len-5, MBSYS_SIMRAD3_COMMENT_LENGTH-1));
			else if (strncmp("COM=", line, 4) == 0)
			    {
			    strncpy(store->par_com, &line[4], MIN(len-5, MBSYS_SIMRAD3_COMMENT_LENGTH-1));
			    store->par_com[MIN(len-5, MBSYS_SIMRAD3_COMMENT_LENGTH-1)] = 0;
			    /* replace caret (^) values with commas (,) to circumvent
			       the format's inability to store commas in comments */
			    while ((comma_ptr = strchr(store->par_com, '^')) != NULL)
				{
				comma_ptr[0] = ',';
				}
			    }
			len = 0;
			}
		else if (status == MB_SUCCESS 
			&& line[len-1] == ','
			&& len <= 5)
			{
			len = 0;
			}
		}
		
	/* now set the data kind */
	if (status == MB_SUCCESS)
		{
		if (strlen(store->par_com) > 0)
		    store->kind = MB_DATA_COMMENT;
		else if (store->type == EM3_START)
		    store->kind = MB_DATA_START;
		else if (store->type == EM3_STOP)
		    store->kind = MB_DATA_STOP;
		else if (store->type == EM3_STOP2)
		    store->kind = MB_DATA_STOP;
		else if (store->type == EM3_STATUS)
		    store->kind = MB_DATA_STOP;
		else if (store->type == EM3_ON)
		    store->kind = MB_DATA_START;
		}
		
	/* read end of record and last two check sum bytes */
	if (status == MB_SUCCESS)
	    {
	    /* if EM3_END not yet found then the 
		next byte should be EM3_END */
	    if (line[0] != EM3_END)
		{
		read_len = fread(&line[0],1,1,mbfp);
		}
		
	    /* if EM3_END not yet found then the 
		next byte should be EM3_END */
	    if (line[0] != EM3_END)
		{
		read_len = fread(&line[0],1,1,mbfp);
		}
		
	    /* if we got the end byte then get check sum bytes */
	    if (line[0] == EM3_END)
		{
		if (line[0] == EM3_END)
			*goodend = MB_YES;
		read_len = fread(&line[1],2,1,mbfp);
	    /* don't check success of read
	        - return success here even if read fails
	        because all of the
		important information in this record has
		already been read - next attempt to read
		file will return error */
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[0], line[0], 
		line[1], line[1], 
		line[2], line[2]);
#endif
		}
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "\n");
#endif
	    }

	/* print debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       par_date:        %d\n",store->par_date);
		fprintf(stderr,"dbg5       par_msec:        %d\n",store->par_msec);
		fprintf(stderr,"dbg5       par_line_num:    %d\n",store->par_line_num);
		fprintf(stderr,"dbg5       par_serial_1:    %d\n",store->par_serial_1);
		fprintf(stderr,"dbg5       par_serial_2:    %d\n",store->par_serial_2);
		fprintf(stderr,"dbg5       par_wlz:         %f\n",store->par_wlz);
		fprintf(stderr,"dbg5       par_smh:         %d\n",store->par_smh);
		fprintf(stderr,"dbg5       par_s1z:         %f\n",store->par_s1z);
		fprintf(stderr,"dbg5       par_s1x:         %f\n",store->par_s1x);
		fprintf(stderr,"dbg5       par_s1y:         %f\n",store->par_s1y);
		fprintf(stderr,"dbg5       par_s1h:         %f\n",store->par_s1h);
		fprintf(stderr,"dbg5       par_s1r:         %f\n",store->par_s1r);
		fprintf(stderr,"dbg5       par_s1p:         %f\n",store->par_s1p);
		fprintf(stderr,"dbg5       par_s1n:         %d\n",store->par_s1n);
		fprintf(stderr,"dbg5       par_s2z:         %f\n",store->par_s2z);
		fprintf(stderr,"dbg5       par_s2x:         %f\n",store->par_s2x);
		fprintf(stderr,"dbg5       par_s2y:         %f\n",store->par_s2y);
		fprintf(stderr,"dbg5       par_s2h:         %f\n",store->par_s2h);
		fprintf(stderr,"dbg5       par_s2r:         %f\n",store->par_s2r);
		fprintf(stderr,"dbg5       par_s2p:         %f\n",store->par_s2p);
		fprintf(stderr,"dbg5       par_s2n:         %d\n",store->par_s2n);
		fprintf(stderr,"dbg5       par_go1:         %f\n",store->par_go1);
		fprintf(stderr,"dbg5       par_go2:         %f\n",store->par_go2);
		fprintf(stderr,"dbg5       par_tsv:         %s\n",store->par_tsv);
		fprintf(stderr,"dbg5       par_rsv:         %s\n",store->par_rsv);
		fprintf(stderr,"dbg5       par_bsv:         %s\n",store->par_bsv);
		fprintf(stderr,"dbg5       par_psv:         %s\n",store->par_psv);
		fprintf(stderr,"dbg5       par_osv:         %s\n",store->par_osv);
		fprintf(stderr,"dbg5       par_dsd:         %f\n",store->par_dsd);
		fprintf(stderr,"dbg5       par_dso:         %f\n",store->par_dso);
		fprintf(stderr,"dbg5       par_dsf:         %f\n",store->par_dsf);
		fprintf(stderr,"dbg5       par_dsh:         %c%c\n",
			store->par_dsh[0],store->par_dsh[1]);
		fprintf(stderr,"dbg5       par_aps:         %d\n",store->par_aps);
		fprintf(stderr,"dbg5       par_p1m:         %d\n",store->par_p1m);
		fprintf(stderr,"dbg5       par_p1t:         %d\n",store->par_p1t);
		fprintf(stderr,"dbg5       par_p1z:         %f\n",store->par_p1z);
		fprintf(stderr,"dbg5       par_p1x:         %f\n",store->par_p1x);
		fprintf(stderr,"dbg5       par_p1y:         %f\n",store->par_p1y);
		fprintf(stderr,"dbg5       par_p1d:         %f\n",store->par_p1d);
		fprintf(stderr,"dbg5       par_p1g:         %s\n",store->par_p1g);
		fprintf(stderr,"dbg5       par_p2m:         %d\n",store->par_p2m);
		fprintf(stderr,"dbg5       par_p2t:         %d\n",store->par_p2t);
		fprintf(stderr,"dbg5       par_p2z:         %f\n",store->par_p2z);
		fprintf(stderr,"dbg5       par_p2x:         %f\n",store->par_p2x);
		fprintf(stderr,"dbg5       par_p2y:         %f\n",store->par_p2y);
		fprintf(stderr,"dbg5       par_p2d:         %f\n",store->par_p2d);
		fprintf(stderr,"dbg5       par_p2g:         %s\n",store->par_p2g);
		fprintf(stderr,"dbg5       par_p3m:         %d\n",store->par_p3m);
		fprintf(stderr,"dbg5       par_p3t:         %d\n",store->par_p3t);
		fprintf(stderr,"dbg5       par_p3z:         %f\n",store->par_p3z);
		fprintf(stderr,"dbg5       par_p3x:         %f\n",store->par_p3x);
		fprintf(stderr,"dbg5       par_p3y:         %f\n",store->par_p3y);
		fprintf(stderr,"dbg5       par_p3d:         %f\n",store->par_p3d);
		fprintf(stderr,"dbg5       par_p3g:         %s\n",store->par_p3g);
		fprintf(stderr,"dbg5       par_msz:         %f\n",store->par_msz);
		fprintf(stderr,"dbg5       par_msx:         %f\n",store->par_msx);
		fprintf(stderr,"dbg5       par_msy:         %f\n",store->par_msy);
		fprintf(stderr,"dbg5       par_mrp:         %c%c\n",
			store->par_mrp[0],store->par_mrp[1]);
		fprintf(stderr,"dbg5       par_msd:         %f\n",store->par_msd);
		fprintf(stderr,"dbg5       par_msr:         %f\n",store->par_msr);
		fprintf(stderr,"dbg5       par_msp:         %f\n",store->par_msp);
		fprintf(stderr,"dbg5       par_msg:         %f\n",store->par_msg);
		fprintf(stderr,"dbg5       par_gcg:         %f\n",store->par_gcg);
		fprintf(stderr,"dbg5       par_cpr:         %s\n",store->par_cpr);
		fprintf(stderr,"dbg5       par_rop:         %s\n",store->par_rop);
		fprintf(stderr,"dbg5       par_sid:         %s\n",store->par_sid);
		fprintf(stderr,"dbg5       par_pll:         %s\n",store->par_pll);
		fprintf(stderr,"dbg5       par_com:         %s\n",store->par_com);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       version:    %d\n",*version);
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_run_parameter(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_run_parameter";
	int	status = MB_SUCCESS;
	char	line[EM3_RUN_PARAMETER_SIZE];
	short	short_val;
	int	read_len;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* set kind and type values */
	store->kind = MB_DATA_RUN_PARAMETER;
	store->type = EM3_RUN_PARAMETER;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM3_RUN_PARAMETER_SIZE-4,mbfp);
	if (read_len == EM3_RUN_PARAMETER_SIZE-4)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &store->run_date); 
		    if (store->run_date != 0) store->date = store->run_date;
		mb_get_binary_int(swap, &line[4], &store->run_msec); 
		    if (store->run_date != 0) store->msec = store->run_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    store->run_ping_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    store->run_serial = (int) ((unsigned short) short_val);
		mb_get_binary_int(swap, &line[12], &store->run_status); 
		store->run_mode = (mb_u_char) line[16];
		store->run_filter_id = (mb_u_char) line[17];
		mb_get_binary_short(swap, &line[18], &short_val); 
		    store->run_min_depth = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[20], &short_val); 
		    store->run_max_depth = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[22], &short_val); 
		    store->run_absorption = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[24], &short_val); 
		    store->run_tran_pulse = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[26], &short_val); 
		    store->run_tran_beam = (int) ((unsigned short) short_val);
		store->run_tran_pow = (mb_u_char) line[28];
		store->run_rec_beam = (mb_u_char) line[29];
		store->run_rec_band = (mb_u_char) line[30];
		store->run_rec_gain = (mb_u_char) line[31];
		store->run_tvg_cross = (mb_u_char) line[32];
		store->run_ssv_source = (mb_u_char) line[33];
		mb_get_binary_short(swap, &line[34], &short_val); 
		    store->run_max_swath = (int) ((unsigned short) short_val);
		store->run_beam_space = (mb_u_char) line[36];
		store->run_swath_angle = (mb_u_char) line[37];
		store->run_stab_mode = (mb_u_char) line[38];
		for (i=0;i<6;i++)
		    store->run_spare[i] = line[39+i];
		if (line[EM3_RUN_PARAMETER_SIZE-7] == EM3_END)
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[EM3_RUN_PARAMETER_SIZE-7], line[EM3_RUN_PARAMETER_SIZE-7], 
		line[EM3_RUN_PARAMETER_SIZE-6], line[EM3_RUN_PARAMETER_SIZE-6], 
		line[EM3_RUN_PARAMETER_SIZE-5], line[EM3_RUN_PARAMETER_SIZE-5]);
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       run_date:        %d\n",store->run_date);
		fprintf(stderr,"dbg5       run_msec:        %d\n",store->run_msec);
		fprintf(stderr,"dbg5       run_ping_count:  %d\n",store->run_ping_count);
		fprintf(stderr,"dbg5       run_serial:      %d\n",store->run_serial);
		fprintf(stderr,"dbg5       run_status:      %d\n",store->run_status);
		fprintf(stderr,"dbg5       run_mode:        %d\n",store->run_mode);
		fprintf(stderr,"dbg5       run_filter_id:   %d\n",store->run_filter_id);
		fprintf(stderr,"dbg5       run_min_depth:   %d\n",store->run_min_depth);
		fprintf(stderr,"dbg5       run_max_depth:   %d\n",store->run_max_depth);
		fprintf(stderr,"dbg5       run_absorption:  %d\n",store->run_absorption);
		fprintf(stderr,"dbg5       run_tran_pulse:  %d\n",store->run_tran_pulse);
		fprintf(stderr,"dbg5       run_tran_beam:   %d\n",store->run_tran_beam);
		fprintf(stderr,"dbg5       run_tran_pow:    %d\n",store->run_tran_pow);
		fprintf(stderr,"dbg5       run_rec_beam:    %d\n",store->run_rec_beam);
		fprintf(stderr,"dbg5       run_rec_band:    %d\n",store->run_rec_band);
		fprintf(stderr,"dbg5       run_rec_gain:    %d\n",store->run_rec_gain);
		fprintf(stderr,"dbg5       run_tvg_cross:   %d\n",store->run_tvg_cross);
		fprintf(stderr,"dbg5       run_ssv_source:  %d\n",store->run_ssv_source);
		fprintf(stderr,"dbg5       run_max_swath:   %d\n",store->run_max_swath);
		fprintf(stderr,"dbg5       run_beam_space:  %d\n",store->run_beam_space);
		fprintf(stderr,"dbg5       run_swath_angle: %d\n",store->run_swath_angle);
		fprintf(stderr,"dbg5       run_stab_mode:   %d\n",store->run_stab_mode);
		for (i=0;i<6;i++)
			fprintf(stderr,"dbg5       run_spare[%d]:    %d\n",i,store->run_spare[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_clock(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_clock";
	int	status = MB_SUCCESS;
	char	line[EM3_CLOCK_SIZE];
	short	short_val;
	int	read_len;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* set kind and type values */
	store->kind = MB_DATA_CLOCK;
	store->type = EM3_CLOCK;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM3_CLOCK_SIZE-4,mbfp);
	if (read_len == EM3_CLOCK_SIZE-4)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &store->clk_date); 
		    store->date = store->clk_date;
		mb_get_binary_int(swap, &line[4], &store->clk_msec); 
		    store->msec = store->clk_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    store->clk_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    store->clk_serial = (int) ((unsigned short) short_val);
		mb_get_binary_int(swap, &line[12], &store->clk_origin_date); 
		mb_get_binary_int(swap, &line[16], &store->clk_origin_msec); 
		store->clk_1_pps_use = (mb_u_char) line[20];
		if (line[EM3_CLOCK_SIZE-7] == EM3_END)
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[EM3_CLOCK_SIZE-7], line[EM3_CLOCK_SIZE-7], 
		line[EM3_CLOCK_SIZE-6], line[EM3_CLOCK_SIZE-6], 
		line[EM3_CLOCK_SIZE-5], line[EM3_CLOCK_SIZE-5]);
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       clk_date:        %d\n",store->clk_date);
		fprintf(stderr,"dbg5       clk_msec:        %d\n",store->clk_msec);
		fprintf(stderr,"dbg5       clk_count:       %d\n",store->clk_count);
		fprintf(stderr,"dbg5       clk_serial:      %d\n",store->clk_serial);
		fprintf(stderr,"dbg5       clk_origin_date: %d\n",store->clk_origin_date);
		fprintf(stderr,"dbg5       clk_origin_msec: %d\n",store->clk_origin_msec);
		fprintf(stderr,"dbg5       clk_1_pps_use:   %d\n",store->clk_1_pps_use);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_tide(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_tide";
	int	status = MB_SUCCESS;
	char	line[EM3_TIDE_SIZE];
	short	short_val;
	int	read_len;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* set kind and type values */
	store->kind = MB_DATA_TIDE;
	store->type = EM3_TIDE;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM3_TIDE_SIZE-4,mbfp);
	if (read_len == EM3_TIDE_SIZE-4)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &store->tid_date); 
		    store->date = store->tid_date;
		mb_get_binary_int(swap, &line[4], &store->tid_msec); 
		    store->msec = store->tid_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    store->tid_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    store->tid_serial = (int) ((unsigned short) short_val);
		mb_get_binary_int(swap, &line[12], &store->tid_origin_date); 
		mb_get_binary_int(swap, &line[16], &store->tid_origin_msec); 
		mb_get_binary_short(swap, &line[20], &short_val); 
		    store->tid_tide = (int) short_val;
		if (line[EM3_TIDE_SIZE-7] == 0x03)
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[EM3_TIDE_SIZE-7], line[EM3_TIDE_SIZE-7], 
		line[EM3_TIDE_SIZE-6], line[EM3_TIDE_SIZE-6], 
		line[EM3_TIDE_SIZE-5], line[EM3_TIDE_SIZE-5]);
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       tid_date:        %d\n",store->tid_date);
		fprintf(stderr,"dbg5       tid_msec:        %d\n",store->tid_msec);
		fprintf(stderr,"dbg5       tid_count:       %d\n",store->tid_count);
		fprintf(stderr,"dbg5       tid_serial:      %d\n",store->tid_serial);
		fprintf(stderr,"dbg5       tid_origin_date: %d\n",store->tid_origin_date);
		fprintf(stderr,"dbg5       tid_origin_msec: %d\n",store->tid_origin_msec);
		fprintf(stderr,"dbg5       tid_tide:        %d\n",store->tid_tide);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_height(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_height";
	int	status = MB_SUCCESS;
	char	line[EM3_HEIGHT_SIZE];
	short	short_val;
	int	read_len;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* set kind and type values */
	store->kind = MB_DATA_HEIGHT;
	store->type = EM3_HEIGHT;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM3_HEIGHT_SIZE-4,mbfp);
	if (read_len == EM3_HEIGHT_SIZE-4)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &store->hgt_date); 
		    store->date = store->hgt_date;
		mb_get_binary_int(swap, &line[4], &store->hgt_msec); 
		    store->msec = store->hgt_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    store->hgt_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    store->hgt_serial = (int) ((unsigned short) short_val);
		mb_get_binary_int(swap, &line[12], &store->hgt_height); 
		store->hgt_type = (mb_u_char) line[16];
		if (line[EM3_HEIGHT_SIZE-7] == EM3_END)
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[EM3_HEIGHT_SIZE-7], line[EM3_HEIGHT_SIZE-7], 
		line[EM3_HEIGHT_SIZE-6], line[EM3_HEIGHT_SIZE-6], 
		line[EM3_HEIGHT_SIZE-5], line[EM3_HEIGHT_SIZE-5]);
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       hgt_date:        %d\n",store->hgt_date);
		fprintf(stderr,"dbg5       hgt_msec:        %d\n",store->hgt_msec);
		fprintf(stderr,"dbg5       hgt_count:       %d\n",store->hgt_count);
		fprintf(stderr,"dbg5       hgt_serial:      %d\n",store->hgt_serial);
		fprintf(stderr,"dbg5       hgt_height:      %d\n",store->hgt_height);
		fprintf(stderr,"dbg5       hgt_type:        %d\n",store->hgt_type);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_heading(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_heading";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_heading_struct *heading;
	char	line[EM3_HEADING_HEADER_SIZE];
	short	short_val;
	int	read_len;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* get  storage structure */
	heading = (struct mbsys_simrad3_heading_struct *) store->heading;
		
	/* set kind and type values */
	store->kind = MB_DATA_HEADING;
	store->type = EM3_HEADING;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM3_HEADING_HEADER_SIZE,mbfp);
	if (read_len == EM3_HEADING_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &heading->hed_date); 
		    store->date = heading->hed_date;
		mb_get_binary_int(swap, &line[4], &heading->hed_msec); 
		    store->msec = heading->hed_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    heading->hed_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    heading->hed_serial = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[12], &short_val); 
		    heading->hed_ndata = (int) ((unsigned short) short_val);
		}

	/* read binary heading values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<heading->hed_ndata && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM3_HEADING_SLICE_SIZE,mbfp);
		if (read_len == EM3_HEADING_SLICE_SIZE 
			&& i < MBSYS_SIMRAD3_MAXHEADING)
			{
			status = MB_SUCCESS;
			mb_get_binary_short(swap, &line[0], &short_val); 
			    heading->hed_time[i] = (int) ((unsigned short) short_val);
			mb_get_binary_short(swap, &line[2], &short_val); 
			    heading->hed_heading[i] = (int) ((unsigned short) short_val);
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	    heading->hed_ndata = MIN(heading->hed_ndata, MBSYS_SIMRAD3_MAXHEADING);
	    }
		
	/* now get last bytes of record */
	if (status == MB_SUCCESS)
		{
		read_len = fread(&line[0],1,4,mbfp);
		if (read_len == 4)
			{
			status = MB_SUCCESS;
			heading->hed_heading_status = (mb_u_char) line[0];
			}
		else
			{
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		if (line[1] == EM3_END)
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[1], line[1], 
		line[2], line[2], 
		line[3], line[3]);
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       hed_date:        %d\n",heading->hed_date);
		fprintf(stderr,"dbg5       hed_msec:        %d\n",heading->hed_msec);
		fprintf(stderr,"dbg5       hed_count:       %d\n",heading->hed_count);
		fprintf(stderr,"dbg5       hed_serial:      %d\n",heading->hed_serial);
		fprintf(stderr,"dbg5       hed_ndata:       %d\n",heading->hed_ndata);
		fprintf(stderr,"dbg5       count    time (msec)    heading (0.01 deg)\n");
		fprintf(stderr,"dbg5       -----    -----------    ------------------\n");
		for (i=0;i<heading->hed_ndata;i++)
			fprintf(stderr,"dbg5        %4d      %7d          %7d\n",
				i, heading->hed_time[i], heading->hed_heading[i]);
		fprintf(stderr,"dbg5       hed_heading_status: %d\n",heading->hed_heading_status);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_ssv(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_ssv";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_ssv_struct *ssv;
	char	line[EM3_SSV_HEADER_SIZE];
	short	short_val;
	int	read_len;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* get  storage structure */
	ssv = (struct mbsys_simrad3_ssv_struct *) store->ssv;
	
	/* set kind and type values */
	store->kind = MB_DATA_SSV;
	store->type = EM3_SSV;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM3_SSV_HEADER_SIZE,mbfp);
	if (read_len == EM3_SSV_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &ssv->ssv_date); 
		    store->date = ssv->ssv_date;
		mb_get_binary_int(swap, &line[4], &ssv->ssv_msec); 
		    store->msec = ssv->ssv_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    ssv->ssv_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    ssv->ssv_serial = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[12], &short_val); 
		    ssv->ssv_ndata = (int) ((unsigned short) short_val);
		}

	/* read binary ssv values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<ssv->ssv_ndata && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM3_SSV_SLICE_SIZE,mbfp);
		if (read_len == EM3_SSV_SLICE_SIZE 
			&& i < MBSYS_SIMRAD3_MAXSSV)
			{
			status = MB_SUCCESS;
			mb_get_binary_short(swap, &line[0], &short_val); 
			    ssv->ssv_time[i] = (int) ((unsigned short) short_val);
			mb_get_binary_short(swap, &line[2], &short_val); 
			    ssv->ssv_ssv[i] = (int) ((unsigned short) short_val);
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	    ssv->ssv_ndata = MIN(ssv->ssv_ndata, MBSYS_SIMRAD3_MAXSSV);
	    }
		
	/* now get last bytes of record */
	if (status == MB_SUCCESS)
		{
		read_len = fread(&line[0],1,4,mbfp);
		if (read_len == 4)
			{
			status = MB_SUCCESS;
			}
		else
			{
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		if (line[1] == EM3_END)
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[1], line[1], 
		line[2], line[2], 
		line[3], line[3]);
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       ssv_date:        %d\n",ssv->ssv_date);
		fprintf(stderr,"dbg5       ssv_msec:        %d\n",ssv->ssv_msec);
		fprintf(stderr,"dbg5       ssv_count:       %d\n",ssv->ssv_count);
		fprintf(stderr,"dbg5       ssv_serial:      %d\n",ssv->ssv_serial);
		fprintf(stderr,"dbg5       ssv_ndata:       %d\n",ssv->ssv_ndata);
		fprintf(stderr,"dbg5       count    time (msec)    ssv (0.1 m/s)\n");
		fprintf(stderr,"dbg5       -----    -----------    ------------------\n");
		for (i=0;i<ssv->ssv_ndata;i++)
			fprintf(stderr,"dbg5        %4d      %7d          %7d\n",
				i, ssv->ssv_time[i], ssv->ssv_ssv[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_tilt(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_tilt";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_tilt_struct *tilt;
	char	line[EM3_TILT_HEADER_SIZE];
	short	short_val;
	int	read_len;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* get  storage structure */
	tilt = (struct mbsys_simrad3_tilt_struct *) store->tilt;
	
	/* set kind and type values */
	store->kind = MB_DATA_TILT;
	store->type = EM3_TILT;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM3_TILT_HEADER_SIZE,mbfp);
	if (read_len == EM3_TILT_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &tilt->tlt_date); 
		    store->date = tilt->tlt_date;
		mb_get_binary_int(swap, &line[4], &tilt->tlt_msec); 
		    store->msec = tilt->tlt_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    tilt->tlt_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    tilt->tlt_serial = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[12], &short_val); 
		    tilt->tlt_ndata = (int) ((unsigned short) short_val);
		}

	/* read binary tilt values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<tilt->tlt_ndata && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM3_TILT_SLICE_SIZE,mbfp);
		if (read_len == EM3_TILT_SLICE_SIZE 
			&& i < MBSYS_SIMRAD3_MAXTILT)
			{
			status = MB_SUCCESS;
			mb_get_binary_short(swap, &line[0], &short_val); 
			    tilt->tlt_time[i] = (int) ((unsigned short) short_val);
			mb_get_binary_short(swap, &line[2], &short_val); 
			    tilt->tlt_tilt[i] = (int) ((unsigned short) short_val);
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	    tilt->tlt_ndata = MIN(tilt->tlt_ndata, MBSYS_SIMRAD3_MAXTILT);
	    }
		
	/* now get last bytes of record */
	if (status == MB_SUCCESS)
		{
		read_len = fread(&line[0],1,4,mbfp);
		if (read_len == 4)
			{
			status = MB_SUCCESS;
			}
		else
			{
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		if (line[1] == EM3_END)
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[1], line[1], 
		line[2], line[2], 
		line[3], line[3]);
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       tlt_date:        %d\n",tilt->tlt_date);
		fprintf(stderr,"dbg5       tlt_msec:        %d\n",tilt->tlt_msec);
		fprintf(stderr,"dbg5       tlt_count:       %d\n",tilt->tlt_count);
		fprintf(stderr,"dbg5       tlt_serial:      %d\n",tilt->tlt_serial);
		fprintf(stderr,"dbg5       tlt_ndata:       %d\n",tilt->tlt_ndata);
		fprintf(stderr,"dbg5       count    time (msec)    tilt (0.01 deg)\n");
		fprintf(stderr,"dbg5       -----    -----------    ------------------\n");
		for (i=0;i<tilt->tlt_ndata;i++)
			fprintf(stderr,"dbg5        %4d      %7d          %7d\n",
				i, tilt->tlt_time[i], tilt->tlt_tilt[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_attitude(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_attitude";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_attitude_struct *attitude;
	char	line[EM3_ATTITUDE_HEADER_SIZE];
	short	short_val;
	int	read_len;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* get  storage structure */
	attitude = (struct mbsys_simrad3_attitude_struct *) store->attitude;
		
	/* set kind and type values */
	store->kind = MB_DATA_ATTITUDE;
	store->type = EM3_ATTITUDE;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM3_ATTITUDE_HEADER_SIZE,mbfp);
	if (read_len == EM3_ATTITUDE_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &attitude->att_date); 
		    store->date = attitude->att_date;
		mb_get_binary_int(swap, &line[4], &attitude->att_msec); 
		    store->msec = attitude->att_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    attitude->att_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    attitude->att_serial = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[12], &short_val); 
		    attitude->att_ndata = (int) ((unsigned short) short_val);
		}

	/* read binary attitude values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<attitude->att_ndata && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM3_ATTITUDE_SLICE_SIZE,mbfp);
		if (read_len == EM3_ATTITUDE_SLICE_SIZE 
			&& i < MBSYS_SIMRAD3_MAXATTITUDE)
			{
			status = MB_SUCCESS;
			mb_get_binary_short(swap, &line[0], &short_val); 
			    attitude->att_time[i] = (int) ((unsigned short) short_val);
			mb_get_binary_short(swap, &line[2], &short_val); 
			    attitude->att_sensor_status[i] = (int) ((unsigned short) short_val);
			mb_get_binary_short(swap, &line[4], &short_val); 
			    attitude->att_roll[i] = (int) short_val;
			mb_get_binary_short(swap, &line[6], &short_val); 
			    attitude->att_pitch[i] = (int) short_val;
			mb_get_binary_short(swap, &line[8], &short_val); 
			    attitude->att_heave[i] = (int) short_val;
			mb_get_binary_short(swap, &line[10], &short_val); 
			    attitude->att_heading[i] = (int) ((unsigned short) short_val);
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	    attitude->att_ndata = MIN(attitude->att_ndata, MBSYS_SIMRAD3_MAXATTITUDE);
	    }
		
	/* now get last bytes of record */
	if (status == MB_SUCCESS)
		{
		read_len = fread(&line[0],1,4,mbfp);
		if (read_len == 4)
			{
			status = MB_SUCCESS;
			attitude->att_heading_status = (mb_u_char) line[0];
			}
		else
			{
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		if (line[1] == EM3_END)
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[1], line[1], 
		line[2], line[2], 
		line[3], line[3]);
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       att_date:        %d\n",attitude->att_date);
		fprintf(stderr,"dbg5       att_msec:        %d\n",attitude->att_msec);
		fprintf(stderr,"dbg5       att_count:       %d\n",attitude->att_count);
		fprintf(stderr,"dbg5       att_serial:      %d\n",attitude->att_serial);
		fprintf(stderr,"dbg5       att_ndata:       %d\n",attitude->att_ndata);
		fprintf(stderr,"dbg5       cnt   time   roll pitch heave heading\n");
		fprintf(stderr,"dbg5       -------------------------------------\n");
		for (i=0;i<attitude->att_ndata;i++)
			fprintf(stderr,"dbg5        %3d  %d  %d %d %d %d\n",
				i, attitude->att_time[i], attitude->att_roll[i], 
				attitude->att_pitch[i], attitude->att_heave[i], 
				attitude->att_heading[i]);
		fprintf(stderr,"dbg5       att_heading_status: %d\n",attitude->att_heading_status);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_pos(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_pos";
	int	status = MB_SUCCESS;
	char	line[MBSYS_SIMRAD3_COMMENT_LENGTH];
	short	short_val;
	int	read_len;
	int	done;
	int	navchannel;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* set kind and type values */
	store->kind = MB_DATA_NAV;
	store->type = EM3_POS;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM3_POS_HEADER_SIZE,mbfp);
	if (read_len == EM3_POS_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &store->pos_date); 
		    store->date = store->pos_date;
		mb_get_binary_int(swap, &line[4], &store->pos_msec); 
		    store->msec = store->pos_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    store->pos_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    store->pos_serial = (int) ((unsigned short) short_val);
		mb_get_binary_int(swap, &line[12], &store->pos_latitude); 
		mb_get_binary_int(swap, &line[16], &store->pos_longitude); 
		mb_get_binary_short(swap, &line[20], &short_val); 
		    store->pos_quality = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[22], &short_val); 
		    store->pos_speed = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[24], &short_val); 
		    store->pos_course = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[26], &short_val); 
		    store->pos_heading = (int) ((unsigned short) short_val);
		store->pos_system = (mb_u_char) line[28];
		store->pos_input_size = (mb_u_char) line[29];
		}

	/* read input position string */
	if (status == MB_SUCCESS && store->pos_input_size < 256)
		{
		read_len = fread(store->pos_input,1,store->pos_input_size,mbfp);
		if (read_len == store->pos_input_size)
			{
			status = MB_SUCCESS;
			store->pos_input[store->pos_input_size] = '\0';
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
		
	/* now loop over reading individual characters to 
	    get last bytes of record */
	if (status == MB_SUCCESS)
	    {
	    done = MB_NO;
	    while (done == MB_NO)
		{
		read_len = fread(&line[0],1,1,mbfp);
		if (read_len == 1 && line[0] == EM3_END)
			{
			done = MB_YES;
			status = MB_SUCCESS;
			/* get last two check sum bytes */
			read_len = fread(&line[1],2,1,mbfp);
		if (line[0] == EM3_END)
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[0], line[0], 
		line[1], line[1], 
		line[2], line[2]);
#endif
			}
		else if (read_len == 1)
			{
			status = MB_SUCCESS;
			}
		else
			{
			done = MB_YES;
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		}
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "\n");
#endif
	    }
		
	/* check for navigation source */
	if (status == MB_SUCCESS)
		{
		/* "active" nav system has first bit set in store->pos_system */
		if (store->pos_system & 128)
		    {
		    store->kind = MB_DATA_NAV;
		    }

		/* otherwise its from a secondary nav system */
		else
		    {
		    navchannel = (store->pos_system & 0x03);
		    if (navchannel == 1)
			{
			store->kind = MB_DATA_NAV1;
			}
		    else if (navchannel == 2)
			{
			store->kind = MB_DATA_NAV2;
			}
		    else if (navchannel == 3)
			{
			store->kind = MB_DATA_NAV3;
			}
    
		    /* otherwise its an error */
		    else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		    }
		}	

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       pos_date:        %d\n",store->pos_date);
		fprintf(stderr,"dbg5       pos_msec:        %d\n",store->pos_msec);
		fprintf(stderr,"dbg5       pos_count:       %d\n",store->pos_count);
		fprintf(stderr,"dbg5       pos_serial:      %d\n",store->pos_serial);
		fprintf(stderr,"dbg5       pos_latitude:    %d\n",store->pos_latitude);
		fprintf(stderr,"dbg5       pos_longitude:   %d\n",store->pos_longitude);
		fprintf(stderr,"dbg5       pos_quality:     %d\n",store->pos_quality);
		fprintf(stderr,"dbg5       pos_speed:       %d\n",store->pos_speed);
		fprintf(stderr,"dbg5       pos_course:      %d\n",store->pos_course);
		fprintf(stderr,"dbg5       pos_heading:     %d\n",store->pos_heading);
		fprintf(stderr,"dbg5       pos_system:      %d\n",store->pos_system);
		fprintf(stderr,"dbg5       pos_input_size:  %d\n",store->pos_input_size);
		fprintf(stderr,"dbg5       pos_input:\ndbg5            %s\n",store->pos_input);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_svp(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_svp";
	int	status = MB_SUCCESS;
	char	line[EM3_SVP_HEADER_SIZE];
	short	short_val;
	int	read_len;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* set kind and type values */
	store->kind = MB_DATA_VELOCITY_PROFILE;
	store->type = EM3_SVP;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM3_SVP_HEADER_SIZE,mbfp);
	if (read_len == EM3_SVP_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &store->svp_use_date); 
		    store->date = store->svp_use_date;
		mb_get_binary_int(swap, &line[4], &store->svp_use_msec); 
		    store->msec = store->svp_use_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    store->svp_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    store->svp_serial = (int) ((unsigned short) short_val);
		mb_get_binary_int(swap, &line[12], &store->svp_origin_date); 
		mb_get_binary_int(swap, &line[16], &store->svp_origin_msec); 
		mb_get_binary_short(swap, &line[20], &short_val); 
		    store->svp_num = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[22], &short_val); 
		    store->svp_depth_res = (int) ((unsigned short) short_val);
		}

	/* read binary svp values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<store->svp_num && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM3_SVP_SLICE_SIZE,mbfp);
		if (read_len != EM3_SVP_SLICE_SIZE)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		else if (i < MBSYS_SIMRAD3_MAXSVP)
			{
			status = MB_SUCCESS;
			mb_get_binary_short(swap, &line[0], &short_val); 
			    store->svp_depth[i] = (int) ((unsigned short) short_val);
			mb_get_binary_short(swap, &line[2], &short_val); 
			    store->svp_vel[i] = (int) ((unsigned short) short_val);
			}
		}
	    store->svp_num = MIN(store->svp_num, MBSYS_SIMRAD3_MAXSVP);
	    }
		
	/* now get last bytes of record */
	if (status == MB_SUCCESS)
		{
		read_len = fread(&line[0],1,4,mbfp);
		if (read_len == 4)
			{
			status = MB_SUCCESS;
			}
		else
			{
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		if (line[1] == EM3_END)
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[1], line[1], 
		line[2], line[2], 
		line[3], line[3]);
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       svp_use_date:    %d\n",store->svp_use_date);
		fprintf(stderr,"dbg5       svp_use_msec:    %d\n",store->svp_use_msec);
		fprintf(stderr,"dbg5       svp_count:       %d\n",store->svp_count);
		fprintf(stderr,"dbg5       svp_serial:      %d\n",store->svp_serial);
		fprintf(stderr,"dbg5       svp_origin_date: %d\n",store->svp_origin_date);
		fprintf(stderr,"dbg5       svp_origin_msec: %d\n",store->svp_origin_msec);
		fprintf(stderr,"dbg5       svp_num:         %d\n",store->svp_num);
		fprintf(stderr,"dbg5       svp_depth_res:   %d\n",store->svp_depth_res);
		fprintf(stderr,"dbg5       count    depth    speed\n");
		fprintf(stderr,"dbg5       -----------------------\n");
		for (i=0;i<store->svp_num;i++)
			fprintf(stderr,"dbg5        %d   %d  %d\n",
				i, store->svp_depth[i], store->svp_vel[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_svp2(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_svp2";
	int	status = MB_SUCCESS;
	char	line[EM3_SVP2_HEADER_SIZE];
	short	short_val;
	int	read_len;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* set kind and type values */
	store->kind = MB_DATA_VELOCITY_PROFILE;
	store->type = EM3_SVP2;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM3_SVP_HEADER_SIZE,mbfp);
	if (read_len == EM3_SVP_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &store->svp_use_date); 
		    store->date = store->svp_use_date;
		mb_get_binary_int(swap, &line[4], &store->svp_use_msec); 
		    store->msec = store->svp_use_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    store->svp_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    store->svp_serial = (int) ((unsigned short) short_val);
		mb_get_binary_int(swap, &line[12], &store->svp_origin_date); 
		mb_get_binary_int(swap, &line[16], &store->svp_origin_msec); 
		mb_get_binary_short(swap, &line[20], &short_val); 
		    store->svp_num = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[22], &short_val); 
		    store->svp_depth_res = (int) ((unsigned short) short_val);
		}

	/* read binary svp values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<store->svp_num && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM3_SVP2_SLICE_SIZE,mbfp);
		if (read_len != EM3_SVP2_SLICE_SIZE)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		else if (i < MBSYS_SIMRAD3_MAXSVP)
			{
			status = MB_SUCCESS;
			mb_get_binary_int(swap, &line[0], &store->svp_depth[i]); 
			mb_get_binary_int(swap, &line[4], &store->svp_vel[i]); 
			}
		}
	    store->svp_num = MIN(store->svp_num, MBSYS_SIMRAD3_MAXSVP);
	    }
		
	/* now get last bytes of record */
	if (status == MB_SUCCESS)
		{
		read_len = fread(&line[0],1,4,mbfp);
		if (read_len == 4)
			{
			status = MB_SUCCESS;
			}
		else
			{
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		if (line[1] == EM3_END)
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[1], line[1], 
		line[2], line[2], 
		line[3], line[3]);
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       svp_use_date:    %d\n",store->svp_use_date);
		fprintf(stderr,"dbg5       svp_use_msec:    %d\n",store->svp_use_msec);
		fprintf(stderr,"dbg5       svp_count:       %d\n",store->svp_count);
		fprintf(stderr,"dbg5       svp_serial:      %d\n",store->svp_serial);
		fprintf(stderr,"dbg5       svp_origin_date: %d\n",store->svp_origin_date);
		fprintf(stderr,"dbg5       svp_origin_msec: %d\n",store->svp_origin_msec);
		fprintf(stderr,"dbg5       svp_num:         %d\n",store->svp_num);
		fprintf(stderr,"dbg5       svp_depth_res:   %d\n",store->svp_depth_res);
		fprintf(stderr,"dbg5       count    depth    speed\n");
		fprintf(stderr,"dbg5       -----------------------\n");
		for (i=0;i<store->svp_num;i++)
			fprintf(stderr,"dbg5        %d   %d  %d\n",
				i, store->svp_depth[i], store->svp_vel[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_bath2(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		int *match, short sonar, int version, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_bath2";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_ping_struct *ping;
	char	line[EM3_BATH2_HEADER_SIZE];
	short	short_val;
	float	float_val;
	int	int_val;
	int	read_len;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		fprintf(stderr,"dbg2       version:    %d\n",version);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* get  storage structure */
	ping = (struct mbsys_simrad3_ping_struct *) store->ping;
		
	/* set kind and type values */
	store->kind = MB_DATA_DATA;
	store->type = EM3_BATH;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM3_BATH2_HEADER_SIZE,mbfp);
	if (read_len == EM3_BATH2_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &ping->png_date); 
		    store->date = ping->png_date;
		mb_get_binary_int(swap, &line[4], &ping->png_msec); 
		    store->msec = ping->png_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    ping->png_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    ping->png_serial = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[12], &short_val); 
		    ping->png_heading = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[14], &short_val); 
		    ping->png_ssv = (int) ((unsigned short) short_val);
		mb_get_binary_float(swap, &line[16], &float_val); 
		    ping->png_xducer_depth = float_val;
		mb_get_binary_short(swap, &line[20], &short_val); 
		    ping->png_nbeams = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[22], &short_val); 
		    ping->png_nbeams_valid = (int) ((unsigned short) short_val);
		mb_get_binary_float(swap, &line[24], &float_val); 
		    ping->png_sample_rate = float_val;
		mb_get_binary_int(swap, &line[28], &int_val); 
		    ping->png_spare = int_val;
#ifdef MBR_EM710RAW_DEBUG
fprintf(stderr,"mbr_em710raw_rd_bath2:    ping->png_date:%d     ping->png_msec:%d     ping->png_count:%d     ping->png_nbeams:%d\n",
ping->png_date,ping->png_msec,ping->png_count,ping->png_nbeams);
#endif
		}
		
	/* check for some indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (ping->png_nbeams_valid > ping->png_nbeams
			|| ping->png_nbeams < 0
			|| ping->png_nbeams_valid < 0
			|| ping->png_nbeams > MBSYS_SIMRAD3_MAXBEAMS
			|| ping->png_nbeams_valid > MBSYS_SIMRAD3_MAXBEAMS)
			{
#ifdef MBR_EM710RAW_DEBUG
fprintf(stderr,"mbr_em710raw_rd_bath2: ERROR SET: ping->png_nbeams:%d ping->png_nbeams_valid:%d MBSYS_SIMRAD3_MAXBEAMS:%d\n",
ping->png_nbeams,ping->png_nbeams_valid,MBSYS_SIMRAD3_MAXBEAMS);
#endif
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read binary beam values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<ping->png_nbeams && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM3_BATH2_BEAM_SIZE,mbfp);
		if (read_len == EM3_BATH2_BEAM_SIZE 
			&& i < MBSYS_SIMRAD3_MAXBEAMS)
			{
			status = MB_SUCCESS;
			mb_get_binary_float(swap, &line[0], &float_val); 
			    ping->png_depth[i] = float_val;
			mb_get_binary_float(swap, &line[4], &float_val); 
			    ping->png_acrosstrack[i] = float_val;
			mb_get_binary_float(swap, &line[8], &float_val); 
			    ping->png_alongtrack[i] = float_val;
			mb_get_binary_short(swap, &line[12], &short_val); 
			    ping->png_window[i] = (int) ((unsigned short) short_val);
			ping->png_quality[i] = (int)((mb_u_char) line[14]);
			ping->png_iba[i] = (int)((mb_s_char) line[15]);
			ping->png_detection[i] = (int)((mb_u_char) line[16]);
			ping->png_clean[i] = (int)((mb_s_char) line[17]);
			mb_get_binary_short(swap, &line[18], &short_val); 
			    ping->png_amp[i] = (int) short_val;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	    }
		
	/* now get last bytes of record */
	if (status == MB_SUCCESS)
		{
		read_len = fread(&line[0],1,4,mbfp);
		if (read_len == 4)
			{
			status = MB_SUCCESS;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		if (line[1] == EM3_END)
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[1], line[1], 
		line[2], line[2], 
		line[3], line[3]);
#endif
		}
		
	/* check if bath and sidescan time tags agree 
	   - we cannot pair bath 
	   and sidescan records from different pings */
	if (status == MB_SUCCESS)
		{
		if (ping->png_date == ping->png_ss_date
		    && ping->png_msec == ping->png_ss_msec)
		    *match = MB_YES;
		else
		    *match = MB_NO;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:                  %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:                 %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:                  %d\n",store->date);
		fprintf(stderr,"dbg5       msec:                  %d\n",store->msec);
		fprintf(stderr,"dbg5       png_date:              %d\n",ping->png_date);
		fprintf(stderr,"dbg5       png_msec:              %d\n",ping->png_msec);
		fprintf(stderr,"dbg5       png_count:             %d\n",ping->png_count);
		fprintf(stderr,"dbg5       png_serial:            %d\n",ping->png_serial);
		fprintf(stderr,"dbg5       png_heading:           %d\n",ping->png_heading);
		fprintf(stderr,"dbg5       png_ssv:               %d\n",ping->png_ssv);
		fprintf(stderr,"dbg5       png_xducer_depth:      %f\n",ping->png_xducer_depth);
		fprintf(stderr,"dbg5       png_nbeams:            %d\n",ping->png_nbeams);
		fprintf(stderr,"dbg5       png_nbeams_valid:      %d\n",ping->png_nbeams_valid);
		fprintf(stderr,"dbg5       png_sample_rate:       %f\n",ping->png_sample_rate);
		fprintf(stderr,"dbg5       png_spare:             %d\n",ping->png_spare);
		fprintf(stderr,"dbg5       cnt  depth xtrack ltrack win  qual  iba det cln amp\n");
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		for (i=0;i<ping->png_nbeams;i++)
			fprintf(stderr,"dbg5       %3d %7.2f %7.2f %7.2f %5d %5d %5d %4d %3d %3d\n",
				i, ping->png_depth[i], ping->png_acrosstrack[i], 
				ping->png_alongtrack[i], ping->png_window[i], 
				ping->png_quality[i], ping->png_iba[i], 
				ping->png_detection[i], ping->png_clean[i], 
				ping->png_amp[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       match:      %d\n",*match);
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_rawbeam4(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_rawbeam4";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_ping_struct *ping;
	char	line[EM3_RAWBEAM4_HEADER_SIZE];
	short	short_val;
	int	int_val;
	float	float_val;
	int	read_len;
	int	spare;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* get  storage structure */
	ping = (struct mbsys_simrad3_ping_struct *) store->ping;
		
	/* read binary header values into char array */
	read_len = fread(line,1,EM3_RAWBEAM4_HEADER_SIZE,mbfp);
	if (read_len == EM3_RAWBEAM4_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &ping->png_raw_date); 
		    store->date = ping->png_raw_date;
		mb_get_binary_int(swap, &line[4], &ping->png_raw_msec); 
		    store->msec = ping->png_raw_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    ping->png_raw_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    ping->png_raw_serial = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[12], &short_val); 
		    ping->png_raw_ssv = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[14], &short_val); 
		    ping->png_raw_ntx = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[16], &short_val); 
		    ping->png_raw_nbeams = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[18], &short_val); 
		    ping->png_raw_detections = (int) ((unsigned short) short_val);
		mb_get_binary_float(swap, &line[20], &float_val); 
		    ping->png_raw_sample_rate = (int) (float_val);
		mb_get_binary_int(swap, &line[24], &int_val); 
		    ping->png_raw_spare = (int) (int_val);
/*fprintf(stderr,"ping->png_raw_date:%d ping->png_raw_msec:%d ping->png_raw_count:%d ping->png_raw_nbeams:%d\n",
ping->png_raw_date,ping->png_raw_msec,ping->png_raw_count,ping->png_raw_nbeams);*/
		}
		
	/* check for some indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (ping->png_raw_detections > ping->png_raw_nbeams
			|| ping->png_raw_nbeams < 0
			|| ping->png_raw_detections < 0
			|| ping->png_raw_nbeams > MBSYS_SIMRAD3_MAXBEAMS
			|| ping->png_raw_detections > MBSYS_SIMRAD3_MAXBEAMS
			|| ping->png_raw_ntx > MBSYS_SIMRAD3_MAXTX)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read binary tx values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<ping->png_raw_ntx && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM3_RAWBEAM4_TX_SIZE,mbfp);
		if (read_len == EM3_RAWBEAM4_TX_SIZE 
			&& i < MBSYS_SIMRAD3_MAXTX)
			{
			status = MB_SUCCESS;
			mb_get_binary_short(swap, &line[0], &short_val); 
			    ping->png_raw_txtiltangle[i] = (int) short_val;
			mb_get_binary_short(swap, &line[2], &short_val); 
			    ping->png_raw_txfocus[i] = (int) ((unsigned short) short_val);
			mb_get_binary_float(swap, &line[4], &float_val); 
			    ping->png_raw_txsignallength[i] = float_val;
			mb_get_binary_float(swap, &line[8], &float_val); 
			    ping->png_raw_txoffset[i] = float_val;
			mb_get_binary_float(swap, &line[12], &float_val); 
			    ping->png_raw_txcenter[i] = float_val;
			mb_get_binary_short(swap, &line[2], &short_val); 
			    ping->png_raw_txabsorption[i] = (int) ((unsigned short) short_val);
			ping->png_raw_txwaveform[i] = (int) line[18];
			ping->png_raw_txsector[i] = (int) line[19];
			mb_get_binary_float(swap, &line[20], &float_val); 
			    ping->png_raw_txbandwidth[i] = float_val;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	    }

	/* read binary beam values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<ping->png_raw_nbeams && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM3_RAWBEAM4_BEAM_SIZE,mbfp);
		if (read_len == EM3_RAWBEAM4_BEAM_SIZE 
			&& i < MBSYS_SIMRAD3_MAXBEAMS)
			{
			status = MB_SUCCESS;
			mb_get_binary_short(swap, &line[0], &short_val); 
			    ping->png_raw_rxpointangle[i] = (int) short_val;
			ping->png_raw_rxsector[i] = (mb_u_char) line[2];
			ping->png_raw_rxdetection[i] = (mb_u_char) line[3];
			mb_get_binary_short(swap, &line[4], &short_val); 
			    ping->png_raw_rxwindow[i] = (int) ((unsigned short) short_val);
			ping->png_raw_rxquality[i] = (mb_u_char) line[6];
			ping->png_raw_rxspare1[i] = (mb_s_char) line[7];
			mb_get_binary_float(swap, &line[8], &float_val); 
			    ping->png_raw_rxrange[i] = float_val;
			mb_get_binary_short(swap, &line[12], &short_val); 
			    ping->png_raw_rxamp[i] = (int) ((short) short_val);
			ping->png_raw_rxcleaning[i] = (mb_s_char) line[14];
			ping->png_raw_rxspare2[i] = (mb_u_char) line[15];
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	    }
		
	/* now get last bytes of record */
	if (status == MB_SUCCESS)
		{
		read_len = fread(&line[0],1,4,mbfp);
		if (read_len == 4)
			{
			status = MB_SUCCESS;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		if (line[1] == EM3_END)
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[1], line[1], 
		line[2], line[2], 
		line[3], line[3]);
#endif
		}
		
	/* check for some other indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (ping->png_raw_nbeams > 0
		    && ping->png_raw_detections > ping->png_raw_nbeams)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       png_raw4_read:               %d\n",ping->png_raw4_read);
		fprintf(stderr,"dbg5       png_raw_date:                %d\n",ping->png_raw_date);
		fprintf(stderr,"dbg5       png_raw_msec:                %d\n",ping->png_raw_msec);
		fprintf(stderr,"dbg5       png_raw_count:               %d\n",ping->png_raw_count);
		fprintf(stderr,"dbg5       png_raw_serial:              %d\n",ping->png_raw_serial);
		fprintf(stderr,"dbg5       png_raw_ssv:                 %d\n",ping->png_raw_ssv);
		fprintf(stderr,"dbg5       png_raw_ntx:                 %d\n",ping->png_raw_ntx);
		fprintf(stderr,"dbg5       png_raw_nbeams:              %d\n",ping->png_raw_nbeams);
		fprintf(stderr,"dbg5       png_raw_detections:          %d\n",ping->png_raw_detections);
		fprintf(stderr,"dbg5       png_raw_sample_rate:         %f\n",ping->png_raw_sample_rate);
		fprintf(stderr,"dbg5       png_raw_spare:               %d\n",ping->png_raw_spare);
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		fprintf(stderr,"dbg5       transmit pulse values:\n");
		fprintf(stderr,"dbg5       tiltangle focus length offset center bandwidth waveform sector\n");
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		for (i=0;i<ping->png_raw_ntx;i++)
			fprintf(stderr,"dbg5       %3d %5d %5d %f %f %f %4d %4d %4d %f\n",
				i, ping->png_raw_txtiltangle[i], 
				ping->png_raw_txfocus[i], ping->png_raw_txsignallength[i], 
				ping->png_raw_txoffset[i], ping->png_raw_txcenter[i], 
				ping->png_raw_txabsorption[i], ping->png_raw_txwaveform[i], 
				ping->png_raw_txsector[i], ping->png_raw_txbandwidth[i]);
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		fprintf(stderr,"dbg5       beam values:\n");
		fprintf(stderr,"dbg5       angle range sector amp quality window beam\n");
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		for (i=0;i<ping->png_raw_nbeams;i++)
			fprintf(stderr,"dbg5       %3d %5d %3d %3d %4d %3d %5d %f %5d %5d %5d\n",
				i, ping->png_raw_rxpointangle[i], ping->png_raw_rxsector[i], 
				ping->png_raw_rxdetection[i], ping->png_raw_rxwindow[i], 
				ping->png_raw_rxquality[i], ping->png_raw_rxspare1[i],  
				ping->png_raw_rxrange[i],ping->png_raw_rxamp[i], 
				ping->png_raw_rxcleaning[i],ping->png_raw_rxspare2[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_ss2(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int length, int *match, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_ss2";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_ping_struct *ping;
	char	line[EM3_SS2_HEADER_SIZE];
	short	short_val;
	int	int_val;
	float	float_val;
	int	read_len;
	int	done;
	int	junk_bytes;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		fprintf(stderr,"dbg2       length:     %d\n",length);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* get  storage structure */
	ping = (struct mbsys_simrad3_ping_struct *) store->ping;
		
	/* set kind and type values */
	store->kind = MB_DATA_DATA;
	store->type = EM3_SS2;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM3_SS2_HEADER_SIZE,mbfp);
	if (read_len == EM3_SS2_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &ping->png_ss_date); 
		    store->date = ping->png_ss_date;
		mb_get_binary_int(swap, &line[4], & ping->png_ss_msec); 
		    store->msec = ping->png_ss_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    ping->png_ss_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    ping->png_ss_serial = (int) ((unsigned short) short_val);
		mb_get_binary_float(swap, &line[12], &float_val); 
		    ping->png_ss_sample_rate = float_val;
		mb_get_binary_short(swap, &line[16], &short_val); 
		    ping->png_r_zero = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[18], &short_val); 
		    ping->png_bsn = (int) ((short) short_val);
		mb_get_binary_short(swap, &line[20], &short_val); 
		    ping->png_bso = (int) ((short) short_val);
		mb_get_binary_short(swap, &line[22], &short_val); 
		    ping->png_tx = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[24], &short_val); 
		    ping->png_tvg_crossover = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[26], &short_val); 
		    ping->png_nbeams_ss = (int) ((unsigned short) short_val);
/*fprintf(stderr," ping->png_ss_date:%d  ping->png_ss_msec:%d  ping->png_ss_count:%d  ping->png_nbeams_ss:%d\n",
ping->png_ss_date,ping->png_ss_msec,ping->png_ss_count,ping->png_nbeams_ss);*/
		}
		
	/* check for some indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (ping->png_nbeams_ss < 0
			|| ping->png_nbeams_ss > MBSYS_SIMRAD3_MAXBEAMS)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read binary beam values */
	if (status == MB_SUCCESS)
	    {
	    ping->png_npixels = 0;
	    for (i=0;i<ping->png_nbeams_ss && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM3_SS2_BEAM_SIZE,mbfp);
		if (read_len == EM3_SS2_BEAM_SIZE 
			&& i < MBSYS_SIMRAD3_MAXBEAMS)
			{
			status = MB_SUCCESS;
			ping->png_sort_direction[i] = (mb_s_char) line[0];
			ping->png_ssdetection[i] = (mb_u_char) line[1];
			mb_get_binary_short(swap, &line[2], &short_val); 
			    ping->png_beam_samples[i] = (int) ((unsigned short) short_val);
			mb_get_binary_short(swap, &line[4], &short_val); 
			    ping->png_center_sample[i] = (int) ((unsigned short) short_val);

			ping->png_start_sample[i] = ping->png_npixels;
			ping->png_npixels += ping->png_beam_samples[i];
			if (ping->png_npixels > MBSYS_SIMRAD3_MAXRAWPIXELS)
				{
				ping->png_beam_samples[i] 
					-= (ping->png_npixels 
						- MBSYS_SIMRAD3_MAXRAWPIXELS);
				if (ping->png_beam_samples[i] < 0)
					ping->png_beam_samples[i] = 0;
				}
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}

	    /* check for no pixel data - frequently occurs with EM1002 */
	    if (length == EM3_SS2_HEADER_SIZE + ping->png_nbeams_ss * EM3_SS2_BEAM_SIZE + 8)
		{
		if (verbose > 0)
		    fprintf(stderr, "WARNING: No Simrad multibeam sidescan pixels in data record!\n");
		junk_bytes = 0;
		ping->png_npixels = 0;
		}

	    /* check for too much pixel data */
	    if (ping->png_npixels > MBSYS_SIMRAD3_MAXRAWPIXELS)
		{
		if (verbose > 0)
		    fprintf(stderr, "WARNING: Simrad multibeam sidescan pixels %d exceed maximum %d!\n", 
			    ping->png_npixels, MBSYS_SIMRAD3_MAXRAWPIXELS);
		junk_bytes = ping->png_npixels - MBSYS_SIMRAD3_MAXRAWPIXELS;
		ping->png_npixels = MBSYS_SIMRAD3_MAXRAWPIXELS;
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}
	    else
		junk_bytes = 0;
	    }

	/* read binary sidescan values */
	if (status == MB_SUCCESS)
		{
		read_len = fread(ping->png_ssraw,1,2 * ping->png_npixels,mbfp);
		if (read_len == 2 * ping->png_npixels )
			{
			status = MB_SUCCESS;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}

	/* read any leftover binary sidescan values */
	if (status == MB_SUCCESS)
		{
		for (i=0;i<junk_bytes;i++)
		    read_len = fread(&line[0],1,1,mbfp);
		}
		
	/* now loop over reading individual characters to 
	    get last bytes of record */
	if (status == MB_SUCCESS)
	    {
	    done = MB_NO;
	    while (done == MB_NO)
		{
		read_len = fread(&line[0],1,1,mbfp);
		if (read_len == 1 && line[0] == EM3_END)
			{
			done = MB_YES;
			status = MB_SUCCESS;
			/* get last two check sum bytes */
			read_len = fread(&line[1],2,1,mbfp);
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[0], line[0], 
		line[1], line[1], 
		line[2], line[2]);
#endif
			}
		else if (read_len == 1)
			{
			status = MB_SUCCESS;
			}
		else
			{
			done = MB_YES;
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		}
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "\n");
#endif
	    }
		
	/* check if bath and sidescan time tags agree 
	   - we cannot pair bath 
	   and sidescan records from different pings */
	if (status == MB_SUCCESS)
		{
		if (ping->png_date == ping->png_ss_date
		    && ping->png_msec == ping->png_ss_msec)
		    *match = MB_YES;
		else
		    *match = MB_NO;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:               %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:              %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:               %d\n",store->date);
		fprintf(stderr,"dbg5       msec:               %d\n",store->msec);
		fprintf(stderr,"dbg5       png_date:           %d\n",ping->png_date);
		fprintf(stderr,"dbg5       png_msec:           %d\n",ping->png_msec);
		
		fprintf(stderr,"dbg5       png_date:              %d\n",ping->png_date);
		fprintf(stderr,"dbg5       png_msec:              %d\n",ping->png_msec);
		fprintf(stderr,"dbg5       png_count:             %d\n",ping->png_count);
		fprintf(stderr,"dbg5       png_serial:            %d\n",ping->png_serial);
		fprintf(stderr,"dbg5       png_heading:           %d\n",ping->png_heading);
		fprintf(stderr,"dbg5       png_ssv:               %d\n",ping->png_ssv);
		fprintf(stderr,"dbg5       png_xducer_depth:      %f\n",ping->png_xducer_depth);
		fprintf(stderr,"dbg5       png_nbeams:            %d\n",ping->png_nbeams);
		fprintf(stderr,"dbg5       png_nbeams_valid:      %d\n",ping->png_nbeams_valid);
		fprintf(stderr,"dbg5       png_sample_rate:       %f\n",ping->png_sample_rate);
		fprintf(stderr,"dbg5       png_spare:             %d\n",ping->png_spare);
		fprintf(stderr,"dbg5       cnt  depth xtrack ltrack dprsn   azi   rng  qual wnd amp num\n");
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		for (i=0;i<ping->png_nbeams;i++)
			fprintf(stderr,"dbg5       %3d %7.2f %7.2f %7.2f %5d %5d %5d %4d %3d %3d\n",
				i, ping->png_depth[i], ping->png_acrosstrack[i], 
				ping->png_alongtrack[i], ping->png_window[i], 
				ping->png_quality[i], ping->png_iba[i], 
				ping->png_detection[i], ping->png_clean[i], 
				ping->png_amp[i]);

		fprintf(stderr,"dbg5       png_ss_date:        %d\n",ping->png_ss_date);
		fprintf(stderr,"dbg5       png_ss_msec:        %d\n",ping->png_ss_msec);
		fprintf(stderr,"dbg5       png_ss_count:       %d\n",ping->png_ss_count);
		fprintf(stderr,"dbg5       png_ss_serial:      %d\n",ping->png_ss_serial);
		fprintf(stderr,"dbg5       png_ss_sample_rate: %f\n",ping->png_ss_sample_rate);
		fprintf(stderr,"dbg5       png_r_zero:         %d\n",ping->png_r_zero);
		fprintf(stderr,"dbg5       png_bsn:            %d\n",ping->png_bsn);
		fprintf(stderr,"dbg5       png_bso:            %d\n",ping->png_bso);
		fprintf(stderr,"dbg5       png_tx:             %d\n",ping->png_tx);
		fprintf(stderr,"dbg5       png_tvg_crossover:  %d\n",ping->png_tvg_crossover);
		fprintf(stderr,"dbg5       png_nbeams_ss:      %d\n",ping->png_nbeams_ss);
		fprintf(stderr,"dbg5       png_npixels:        %d\n",ping->png_npixels);
		fprintf(stderr,"dbg5       cnt  index sort samples start center\n");
		fprintf(stderr,"dbg5       --------------------------------------------------\n");
		for (i=0;i<ping->png_nbeams_ss;i++)
			fprintf(stderr,"dbg5        %4d %2d %4d %4d %4d %4d\n",
				i, ping->png_sort_direction[i], ping->png_ssdetection[i], 
				ping->png_beam_samples[i], ping->png_start_sample[i], 
				ping->png_center_sample[i]);
		fprintf(stderr,"dbg5       cnt  ss\n");
		fprintf(stderr,"dbg5       --------------------------------------------------\n");
		for (i=0;i<ping->png_npixels;i++)
			fprintf(stderr,"dbg5        %d %d\n",
				i, ping->png_ssraw[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       match:      %d\n",*match);
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_wc(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, 
		short sonar, int *goodend, int *error)
{
	char	*function_name = "mbr_em710raw_rd_wc";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_watercolumn_struct *wc;
	char	line[EM3_WC_HEADER_SIZE];
	short	short_val;
	int	read_len;
	int	done;
	int	file_bytes;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set goodend false until a good end is found */
	*goodend = MB_NO;
		
	/* get  storage structure */
	wc = (struct mbsys_simrad3_watercolumn_struct *) store->wc;
		
	/* set kind and type values */
	store->kind = MB_DATA_WATER_COLUMN;
	store->type = EM3_WATERCOLUMN;
	store->sonar = sonar;
	file_bytes = ftell(mbfp);

	/* read binary header values into char array */
	read_len = fread(line,1,EM3_WC_HEADER_SIZE,mbfp);
	if (read_len == EM3_WC_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(swap, &line[0], &wc->wtc_date); 
		    store->date = wc->wtc_date;
		mb_get_binary_int(swap, &line[4], & wc->wtc_msec); 
		    store->msec = wc->wtc_msec;
		mb_get_binary_short(swap, &line[8], &short_val); 
		    wc->wtc_count = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[10], &short_val); 
		    wc->wtc_serial = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[12], &short_val); 
		    wc->wtc_ndatagrams = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[14], &short_val); 
		    wc->wtc_datagram = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[16], &short_val); 
		    wc->wtc_ntx = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[18], &short_val); 
		    wc->wtc_nrx = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[20], &short_val); 
		    wc->wtc_nbeam = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[22], &short_val); 
		    wc->wtc_ssv = (int) ((unsigned short) short_val);
		mb_get_binary_int(swap, &line[24], &(wc->wtc_sfreq)); 
		mb_get_binary_short(swap, &line[28], &short_val); 
		    wc->wtc_heave = (int) ((short) short_val);
		mb_get_binary_short(swap, &line[30], &short_val); 
		    wc->wtc_spare1 = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[32], &short_val); 
		    wc->wtc_spare2 = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &line[34], &short_val); 
		    wc->wtc_spare3 = (int) ((unsigned short) short_val);
		}
		
	/* check for some indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (wc->wtc_nbeam < 0
			|| wc->wtc_nbeam > MBSYS_SIMRAD3_MAXBEAMS
			|| wc->wtc_ntx < 0
			|| wc->wtc_ntx > MBSYS_SIMRAD3_MAXTX)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read binary beam values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<wc->wtc_ntx && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM3_WC_TX_SIZE,mbfp);
		if (read_len == EM3_WC_TX_SIZE 
			&& i < MBSYS_SIMRAD3_MAXTX)
			{
			mb_get_binary_short(swap, &line[0], &short_val); 
			    wc->wtc_txtiltangle[i] = (int) (short_val);
			mb_get_binary_short(swap, &line[2], &short_val); 
			    wc->wtc_txcenter[i] = (int) (short_val);
			wc->wtc_txsector[i] = (int) ((mb_u_char) line[4]);
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	    for (i=0;i<wc->wtc_nbeam && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM3_WC_BEAM_SIZE,mbfp);
		if (read_len == EM3_WC_BEAM_SIZE 
			&& i < MBSYS_SIMRAD3_MAXBEAMS)
			{
			mb_get_binary_short(swap, &line[0], &short_val); 
			    wc->beam[i].wtc_rxpointangle = (int) (short_val);
			mb_get_binary_short(swap, &line[2], &short_val); 
			    wc->beam[i].wtc_start_sample = (int) (short_val);
			mb_get_binary_short(swap, &line[4], &short_val); 
			    wc->beam[i].wtc_beam_samples = (int) (unsigned short)(short_val);
			mb_get_binary_short(swap, &line[6], &short_val); 
			    wc->beam[i].wtc_beam_spare = (int) (unsigned short)(short_val);
			wc->beam[i].wtc_sector = (int) (mb_u_char) (line[8]);
			wc->beam[i].wtc_beam = (int) (mb_u_char) (line[9]);
			}
		read_len = fread(wc->beam[i].wtc_amp,1,wc->beam[i].wtc_beam_samples,mbfp);
		}
	    }
		
	/* now loop over reading individual characters to 
	    get last bytes of record */
	if (status == MB_SUCCESS)
	    {
	    done = MB_NO;
	    while (done == MB_NO)
		{
		read_len = fread(&line[0],1,1,mbfp);
		if (read_len == 1 && line[0] == EM3_END)
			{
			done = MB_YES;
			status = MB_SUCCESS;
			/* get last two check sum bytes */
			read_len = fread(&line[1],2,1,mbfp);
			*goodend = MB_YES;
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "End Bytes: %2.2hX %d | %2.2hX %d | %2.2hX %d\n", 
		line[0], line[0], 
		line[1], line[1], 
		line[2], line[2]);
#endif
			}
		else if (read_len == 1)
			{
			status = MB_SUCCESS;
			}
		else
			{
			done = MB_YES;
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			}
		}
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "\n");
#endif
	    }

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       wtc_date:        %d\n",wc->wtc_date);
		fprintf(stderr,"dbg5       wtc_msec:        %d\n",wc->wtc_msec);
		fprintf(stderr,"dbg5       wtc_count:       %d\n",wc->wtc_count);
		fprintf(stderr,"dbg5       wtc_serial:      %d\n",wc->wtc_serial);
		fprintf(stderr,"dbg5       wtc_ndatagrams:  %d\n",wc->wtc_ndatagrams);
		fprintf(stderr,"dbg5       wtc_datagram:    %d\n",wc->wtc_datagram);
		fprintf(stderr,"dbg5       wtc_ntx:         %d\n",wc->wtc_ntx);
		fprintf(stderr,"dbg5       wtc_nrx:         %d\n",wc->wtc_nrx);
		fprintf(stderr,"dbg5       wtc_nbeam:       %d\n",wc->wtc_nbeam);
		fprintf(stderr,"dbg5       wtc_ssv:         %d\n",wc->wtc_ssv);
		fprintf(stderr,"dbg5       wtc_sfreq:       %d\n",wc->wtc_sfreq);
		fprintf(stderr,"dbg5       wtc_heave:       %d\n",wc->wtc_heave);
		fprintf(stderr,"dbg5       wtc_spare1:      %d\n",wc->wtc_spare1);
		fprintf(stderr,"dbg5       wtc_spare2:      %d\n",wc->wtc_spare2);
		fprintf(stderr,"dbg5       wtc_spare3:      %d\n",wc->wtc_spare3);
		fprintf(stderr,"dbg5       ---------------------------\n");
		fprintf(stderr,"dbg5       cnt  tilt center sector\n");
		fprintf(stderr,"dbg5       ---------------------------\n");
		for (i=0;i<wc->wtc_ntx;i++)
			fprintf(stderr,"dbg5       %3d %6d %6d %6d\n",
				i, wc->wtc_txtiltangle[i], wc->wtc_txcenter[i], 
				wc->wtc_txsector[i]);
		for (i=0;i<wc->wtc_nbeam;i++)
			{
			fprintf(stderr,"dbg5       --------------------------------------------------\n");
			fprintf(stderr,"dbg5       cnt  angle start samples unknown sector beam\n");
			fprintf(stderr,"dbg5       --------------------------------------------------\n");
			fprintf(stderr,"dbg5        %4d %3d %2d %4d %4d %4d %4d\n",
				i, wc->beam[i].wtc_rxpointangle, 
				wc->beam[i].wtc_start_sample, 
				wc->beam[i].wtc_beam_samples, 
				wc->beam[i].wtc_beam_spare, 
				wc->beam[i].wtc_sector, 
				wc->beam[i].wtc_beam);
/*			fprintf(stderr,"dbg5       --------------------------------------------------\n");
			fprintf(stderr,"dbg5       beam[%d]: sample amplitude\n",i);
			fprintf(stderr,"dbg5       --------------------------------------------------\n");
			for (j=0;j<wc->beam[i].wtc_beam_samples;j++)
				fprintf(stderr,"dbg5        %4d %4d\n",
					j, wc->beam[i].wtc_amp[j]);*/
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       goodend:    %d\n",*goodend);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_em710raw_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store;
	struct mbsys_simrad3_ping_struct *ping;
	FILE	*mbfp;
	int	swap;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_simrad3_struct *) store_ptr;
	ping = (struct mbsys_simrad3_ping_struct *) store->ping;
	mbfp = mb_io_ptr->mbfp;

#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"\nstart of mbr_em710raw_wr_data:\n");
	fprintf(stderr,"kind:%d %d type:%x\n", store->kind, mb_io_ptr->new_kind, store->type);
#endif

	/* set swap flag */
	swap = MB_NO;

	if (store->kind == MB_DATA_COMMENT
		|| store->kind == MB_DATA_START
		|| store->kind == MB_DATA_STOP)
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_start kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em710raw_wr_start(verbose,mbfp,swap,store,error);
		}
	else if (store->kind == MB_DATA_STATUS)
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_status kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em710raw_wr_status(verbose,mbfp,swap,store,error);
		}
	else if (store->kind == MB_DATA_RUN_PARAMETER)
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_run_parameter kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em710raw_wr_run_parameter(verbose,mbfp,swap,store,error);
		}
	else if (store->kind == MB_DATA_CLOCK)
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_clock kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em710raw_wr_clock(verbose,mbfp,swap,store,error);
		}
	else if (store->kind == MB_DATA_TIDE)
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_tide kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em710raw_wr_tide(verbose,mbfp,swap,store,error);
		}
	else if (store->kind == MB_DATA_HEIGHT)
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_height kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em710raw_wr_height(verbose,mbfp,swap,store,error);
		}
	else if (store->kind == MB_DATA_HEADING)
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_heading kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em710raw_wr_heading(verbose,mbfp,swap,store,error);
		}
	else if (store->kind == MB_DATA_SSV)
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_ssv kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em710raw_wr_ssv(verbose,mbfp,swap,store,error);
		}
	else if (store->kind == MB_DATA_TILT)
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_tilt kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em710raw_wr_tilt(verbose,mbfp,swap,store,error);
		}
	else if (store->kind == MB_DATA_ATTITUDE)
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_attitude kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em710raw_wr_attitude(verbose,mbfp,swap,store,error);
		}
	else if (store->kind == MB_DATA_NAV
		|| store->kind == MB_DATA_NAV1
		|| store->kind == MB_DATA_NAV2
		|| store->kind == MB_DATA_NAV3)
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_pos kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em710raw_wr_pos(verbose,mbfp,swap,store,error);
		}
	else if (store->kind == MB_DATA_VELOCITY_PROFILE)
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_svp kind:%d type %x\n",store->kind,store->type);
#endif
	        if (store->type == EM3_SVP)
		  status = mbr_em710raw_wr_svp(verbose,mbfp,swap,store,error);
		else
		  status = mbr_em710raw_wr_svp2(verbose,mbfp,swap,store,error); 
		}
	else if (store->kind == MB_DATA_DATA)
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_bath2 kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em710raw_wr_bath2(verbose,mbfp,swap,store,error);
		if (ping->png_raw4_read == MB_YES)
		    {
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_rawbeam4 kind:%d type %x\n",store->kind,store->type);
#endif
		    status = mbr_em710raw_wr_rawbeam4(verbose,mbfp,swap,store,error);
		    }
#ifdef MBR_EM710RAW_DEBUG
	if (ping->png_raw4_read == MB_NO) 
	fprintf(stderr,"NOT call mbr_em710raw_wr_rawbeam4 kind:%d type %x\n",store->kind,store->type);
#endif
		if (ping->png_ss2_read == MB_YES)
		    {
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_ss2 kind:%d type %x\n",store->kind,store->type);
#endif
		    status = mbr_em710raw_wr_ss2(verbose,mbfp,swap,store,error);
		    }
#ifdef MBR_EM710RAW_DEBUG
	else fprintf(stderr,"NOT call mbr_em710raw_wr_ss2 kind:%d type %x\n",store->kind,store->type);
#endif
		}
	else if (store->kind == MB_DATA_WATER_COLUMN)
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call mbr_em710raw_wr_wc kind:%d type %x\n",store->kind,store->type);
#endif
	        status = mbr_em710raw_wr_wc(verbose,mbfp,swap,store,error);
		}
	else
		{
#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"call nothing bad kind: %d type %x\n", store->kind, store->type);
#endif
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
		}

#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,"status:%d error:%d\n", status, *error);
	fprintf(stderr,"end of mbr_em710raw_wr_data:\n");
#endif

	/* print output debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data record kind in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",store->kind);
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
int mbr_em710raw_wr_start(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_start";
	int	status = MB_SUCCESS;
	char	line[MBSYS_SIMRAD3_BUFFER_SIZE], *buff;
	int	buff_len, write_len;
	int	write_size;
	unsigned short checksum;
	char	*comma_ptr;
	mb_u_char   *uchar_ptr;
	int	j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       par_date:        %d\n",store->par_date);
		fprintf(stderr,"dbg5       par_msec:        %d\n",store->par_msec);
		fprintf(stderr,"dbg5       par_line_num:    %d\n",store->par_line_num);
		fprintf(stderr,"dbg5       par_serial_1:    %d\n",store->par_serial_1);
		fprintf(stderr,"dbg5       par_serial_2:    %d\n",store->par_serial_2);
		fprintf(stderr,"dbg5       par_wlz:         %f\n",store->par_wlz);
		fprintf(stderr,"dbg5       par_smh:         %d\n",store->par_smh);
		fprintf(stderr,"dbg5       par_s1z:         %f\n",store->par_s1z);
		fprintf(stderr,"dbg5       par_s1x:         %f\n",store->par_s1x);
		fprintf(stderr,"dbg5       par_s1y:         %f\n",store->par_s1y);
		fprintf(stderr,"dbg5       par_s1h:         %f\n",store->par_s1h);
		fprintf(stderr,"dbg5       par_s1r:         %f\n",store->par_s1r);
		fprintf(stderr,"dbg5       par_s1p:         %f\n",store->par_s1p);
		fprintf(stderr,"dbg5       par_s1n:         %d\n",store->par_s1n);
		fprintf(stderr,"dbg5       par_s2z:         %f\n",store->par_s2z);
		fprintf(stderr,"dbg5       par_s2x:         %f\n",store->par_s2x);
		fprintf(stderr,"dbg5       par_s2y:         %f\n",store->par_s2y);
		fprintf(stderr,"dbg5       par_s2h:         %f\n",store->par_s2h);
		fprintf(stderr,"dbg5       par_s2r:         %f\n",store->par_s2r);
		fprintf(stderr,"dbg5       par_s2p:         %f\n",store->par_s2p);
		fprintf(stderr,"dbg5       par_s2n:         %d\n",store->par_s2n);
		fprintf(stderr,"dbg5       par_go1:         %f\n",store->par_go1);
		fprintf(stderr,"dbg5       par_go2:         %f\n",store->par_go2);
		fprintf(stderr,"dbg5       par_tsv:         %s\n",store->par_tsv);
		fprintf(stderr,"dbg5       par_rsv:         %s\n",store->par_rsv);
		fprintf(stderr,"dbg5       par_bsv:         %s\n",store->par_bsv);
		fprintf(stderr,"dbg5       par_psv:         %s\n",store->par_psv);
		fprintf(stderr,"dbg5       par_osv:         %s\n",store->par_osv);
		fprintf(stderr,"dbg5       par_dsd:         %f\n",store->par_dsd);
		fprintf(stderr,"dbg5       par_dso:         %f\n",store->par_dso);
		fprintf(stderr,"dbg5       par_dsf:         %f\n",store->par_dsf);
		fprintf(stderr,"dbg5       par_dsh:         %c%c\n",
			store->par_dsh[0],store->par_dsh[1]);
		fprintf(stderr,"dbg5       par_aps:         %d\n",store->par_aps);
		fprintf(stderr,"dbg5       par_p1m:         %d\n",store->par_p1m);
		fprintf(stderr,"dbg5       par_p1t:         %d\n",store->par_p1t);
		fprintf(stderr,"dbg5       par_p1z:         %f\n",store->par_p1z);
		fprintf(stderr,"dbg5       par_p1x:         %f\n",store->par_p1x);
		fprintf(stderr,"dbg5       par_p1y:         %f\n",store->par_p1y);
		fprintf(stderr,"dbg5       par_p1d:         %f\n",store->par_p1d);
		fprintf(stderr,"dbg5       par_p1g:         %s\n",store->par_p1g);
		fprintf(stderr,"dbg5       par_p2m:         %d\n",store->par_p2m);
		fprintf(stderr,"dbg5       par_p2t:         %d\n",store->par_p2t);
		fprintf(stderr,"dbg5       par_p2z:         %f\n",store->par_p2z);
		fprintf(stderr,"dbg5       par_p2x:         %f\n",store->par_p2x);
		fprintf(stderr,"dbg5       par_p2y:         %f\n",store->par_p2y);
		fprintf(stderr,"dbg5       par_p2d:         %f\n",store->par_p2d);
		fprintf(stderr,"dbg5       par_p2g:         %s\n",store->par_p2g);
		fprintf(stderr,"dbg5       par_p3m:         %d\n",store->par_p3m);
		fprintf(stderr,"dbg5       par_p3t:         %d\n",store->par_p3t);
		fprintf(stderr,"dbg5       par_p3z:         %f\n",store->par_p3z);
		fprintf(stderr,"dbg5       par_p3x:         %f\n",store->par_p3x);
		fprintf(stderr,"dbg5       par_p3y:         %f\n",store->par_p3y);
		fprintf(stderr,"dbg5       par_p3d:         %f\n",store->par_p3d);
		fprintf(stderr,"dbg5       par_p3g:         %s\n",store->par_p3g);
		fprintf(stderr,"dbg5       par_msz:         %f\n",store->par_msz);
		fprintf(stderr,"dbg5       par_msx:         %f\n",store->par_msx);
		fprintf(stderr,"dbg5       par_msy:         %f\n",store->par_msy);
		fprintf(stderr,"dbg5       par_mrp:         %c%c\n",
			store->par_mrp[0],store->par_mrp[1]);
		fprintf(stderr,"dbg5       par_msd:         %f\n",store->par_msd);
		fprintf(stderr,"dbg5       par_msr:         %f\n",store->par_msr);
		fprintf(stderr,"dbg5       par_msp:         %f\n",store->par_msp);
		fprintf(stderr,"dbg5       par_msg:         %f\n",store->par_msg);
		fprintf(stderr,"dbg5       par_gcg:         %f\n",store->par_gcg);
		fprintf(stderr,"dbg5       par_cpr:         %s\n",store->par_cpr);
		fprintf(stderr,"dbg5       par_rop:         %s\n",store->par_rop);
		fprintf(stderr,"dbg5       par_sid:         %s\n",store->par_sid);
		fprintf(stderr,"dbg5       par_pll:         %s\n",store->par_pll);
		fprintf(stderr,"dbg5       par_com:         %s\n",store->par_com);
		}
		
	/* zero checksum */
	checksum = 0;
		
	/* if data type not set - use start */
	if (store->type == EM3_NONE)
	    store->type = EM3_START;
	    
	/* if sonar not set use EM710 */
	if (store->sonar == 0)
	    store->sonar = MBSYS_SIMRAD3_EM710;
		
	/* set up start of output buffer - we handle this
	   record differently because of the ascii data */
	memset(line, 0, MBSYS_SIMRAD3_BUFFER_SIZE);

	/* put binary header data into buffer */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) store->type, (void *) &line[4]);
		mb_put_binary_short(swap, (unsigned short) store->sonar, (void *) &line[6]);
		mb_put_binary_int(swap, (int) store->par_date, (void *) &line[8]); 
		mb_put_binary_int(swap, (int) store->par_msec, (void *) &line[12]); 
		mb_put_binary_short(swap, (unsigned short) store->par_line_num, (void *) &line[16]);
		mb_put_binary_short(swap, (unsigned short) store->par_serial_1, (void *) &line[18]);
		mb_put_binary_short(swap, (unsigned short) store->par_serial_2, (void *) &line[20]);
		}
		
	/* construct ASCII parameter buffer */
	buff = &line[22];
	sprintf(&buff[0], "WLZ=%.2f,", store->par_wlz);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "SMH=%d,", store->par_smh);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1Z=%.2f,", store->par_s1z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1X=%.2f,", store->par_s1x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1Y=%.2f,", store->par_s1y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1H=%.2f,", store->par_s1h);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1R=%.2f,", store->par_s1r);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1P=%.2f,", store->par_s1p);
	buff_len = strlen(buff);
	if (store->par_s1n > 0)
	    {
	    sprintf(&buff[buff_len], "S1N=%d,", store->par_s1n);
	    buff_len = strlen(buff);
	    }
	sprintf(&buff[buff_len], "S2Z=%.2f,", store->par_s2z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2X=%.2f,", store->par_s2x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2Y=%.2f,", store->par_s2y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2H=%.2f,", store->par_s2h);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2R=%.2f,", store->par_s2r);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2P=%.2f,", store->par_s2p);
	buff_len = strlen(buff);
	if (store->par_s2n > 0)
	    {
	    sprintf(&buff[buff_len], "S2N=%d,", store->par_s2n);
	    buff_len = strlen(buff);
	    }
	if (store->par_go1 != 0.0)
	    {
	    sprintf(&buff[buff_len], "GO1=%.2f,", store->par_go1);
	    buff_len = strlen(buff);
	    }
	if (store->par_go2 != 0.0)
	    {
	    sprintf(&buff[buff_len], "GO2=%.2f,", store->par_go2);
	    buff_len = strlen(buff);
	    }
	sprintf(&buff[buff_len], "TSV=%s,", store->par_tsv);
	buff_len = strlen(buff);
	if (strlen(store->par_rsv) > 0)
	    {
	    sprintf(&buff[buff_len], "RSV=%s,", store->par_rsv);
	    buff_len = strlen(buff);
	    }
	sprintf(&buff[buff_len], "BSV=%s,", store->par_bsv);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "PSV=%s,", store->par_tsv);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "OSV=%s,", store->par_osv);
	buff_len = strlen(buff);
	if (store->par_dsd != 0.0)
	    {
	    sprintf(&buff[buff_len], "DSD=%.1f,", store->par_dsd);
	    buff_len = strlen(buff);
	    }
	else
	    {
	    sprintf(&buff[buff_len], "DSD=,");
	    buff_len = strlen(buff);
	    }
	sprintf(&buff[buff_len], "DSO=%.6f,", store->par_dso);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "DSF=%.6f,", store->par_dsf);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "DSH=%c%c,", 
		store->par_dsh[0], store->par_dsh[1]);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "APS=%d,",store->par_aps);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1M=%d,",store->par_p1m);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1T=%d,",store->par_p1t);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1Z=%.2f,", store->par_p1z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1X=%.2f,", store->par_p1x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1Y=%.2f,", store->par_p1y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1D=%.1f,", store->par_p1d);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1G=%s,", store->par_p1g);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2M=%d,",store->par_p2m);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2T=%d,",store->par_p2t);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2Z=%.2f,", store->par_p2z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2X=%.2f,", store->par_p2x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2Y=%.2f,", store->par_p2y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2D=%.1f,", store->par_p2d);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2G=%s,", store->par_p2g);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3M=%d,",store->par_p3m);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3T=%d,",store->par_p3t);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3Z=%.2f,", store->par_p3z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3X=%.2f,", store->par_p3x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3Y=%.2f,", store->par_p3y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3D=%.1f,", store->par_p3d);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3G=%s,", store->par_p3g);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSZ=%.2f,", store->par_msz);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSX=%.2f,", store->par_msx);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSY=%.2f,", store->par_msy);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MRP=%c%c,", 
		store->par_mrp[0], store->par_mrp[1]);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSD=%.2f,", store->par_msd);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSR=%.2f,", store->par_msr);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSP=%.2f,", store->par_msp);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSG=%.2f,", store->par_msg);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "GCG=%.2f,", store->par_gcg);
	buff_len = strlen(buff);
	if (strlen(store->par_cpr) > 0)
	    {
	    sprintf(&buff[buff_len], "CPR=%s,", store->par_cpr);
	    buff_len = strlen(buff);
	    }
	if (strlen(store->par_rop) > 0)
	    {
	    sprintf(&buff[buff_len], "ROP=%s,", store->par_rop);
	    buff_len = strlen(buff);
	    }
	if (strlen(store->par_sid) > 0)
	    {
	    sprintf(&buff[buff_len], "SID=%s,", store->par_sid);
	    buff_len = strlen(buff);
	    }
	if (strlen(store->par_pll) > 0)
	    {
	    sprintf(&buff[buff_len], "PLL=%s,", store->par_pll);
	    buff_len = strlen(buff);
	    }
	if (strlen(store->par_com) > 0)
	    {
	    /* replace commas (,) with caret (^) values to circumvent
	       the format's inability to store commas in comments */
	    while ((comma_ptr = strchr(store->par_com, ',')) != NULL)
		{
		comma_ptr[0] = '^';
		}
	    sprintf(&buff[buff_len], "COM=%s,", store->par_com);
	    buff_len = strlen(buff);
	    }
	buff[buff_len] = ',';
	buff_len++;
	if (buff_len % 2 == 0)
	    buff_len++;
	    
	/* put end of record in buffer */
	line[buff_len + 22] = EM3_END;
		
	/* get size of record */
	write_size = 25 + buff_len;
	mb_put_binary_int(swap, (int) (write_size - 4), (void *) &line[0]); 
		
	/* compute checksum */
	uchar_ptr = (mb_u_char *) line;
	for (j=5;j<write_size-3;j++)
	    checksum += uchar_ptr[j];
    
	/* set checksum */
	mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[buff_len + 23]);

	/* finally write out the data */
	write_len = fwrite(&line,1,write_size,mbfp);
	if (write_len != write_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

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
int mbr_em710raw_wr_status(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_status";
	int	status = MB_SUCCESS;
	char	line[EM3_STATUS_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:                %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:               %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:                %d\n",store->date);
		fprintf(stderr,"dbg5       msec:                %d\n",store->msec);
		fprintf(stderr,"dbg5       run_date:            %d\n",store->sts_date);
		fprintf(stderr,"dbg5       run_msec:            %d\n",store->sts_msec);
		fprintf(stderr,"dbg5       sts_status_count:    %d\n",store->sts_status_count);
		fprintf(stderr,"dbg5       run_serial:          %d\n",store->sts_serial);
		fprintf(stderr,"dbg5       sts_pingrate:        %d\n",store->sts_pingrate);
		fprintf(stderr,"dbg5       sts_ping_count:      %d\n",store->sts_ping_count);
		fprintf(stderr,"dbg5       sts_load:            %d\n",store->sts_load);
		fprintf(stderr,"dbg5       sts_udp_status:      %d\n",store->sts_udp_status);
		fprintf(stderr,"dbg5       sts_serial1_status:  %d\n",store->sts_serial1_status);
		fprintf(stderr,"dbg5       sts_serial2_status:  %d\n",store->sts_serial2_status);
		fprintf(stderr,"dbg5       sts_serial3_status:  %d\n",store->sts_serial3_status);
		fprintf(stderr,"dbg5       sts_serial4_status:  %d\n",store->sts_serial4_status);
		fprintf(stderr,"dbg5       sts_pps_status:      %d\n",store->sts_pps_status);
		fprintf(stderr,"dbg5       sts_position_status: %d\n",store->sts_position_status);
		fprintf(stderr,"dbg5       sts_attitude_status: %d\n",store->sts_attitude_status);
		fprintf(stderr,"dbg5       sts_clock_status:    %d\n",store->sts_clock_status);
		fprintf(stderr,"dbg5       sts_heading_status:  %d\n",store->sts_heading_status);
		fprintf(stderr,"dbg5       sts_pu_status:       %d\n",store->sts_pu_status);
		fprintf(stderr,"dbg5       sts_last_heading:    %d\n",store->sts_last_heading);
		fprintf(stderr,"dbg5       sts_last_roll:       %d\n",store->sts_last_roll);
		fprintf(stderr,"dbg5       sts_last_pitch:      %d\n",store->sts_last_pitch);
		fprintf(stderr,"dbg5       sts_last_heave:      %d\n",store->sts_last_heave);
		fprintf(stderr,"dbg5       sts_last_ssv:        %d\n",store->sts_last_ssv);
		fprintf(stderr,"dbg5       sts_last_heave:      %d\n",store->sts_last_heave);
		fprintf(stderr,"dbg5       sts_last_depth:      %d\n",store->sts_last_depth);
		fprintf(stderr,"dbg5       sts_spare:           %d\n",store->sts_spare);
		fprintf(stderr,"dbg5       sts_bso:             %d\n",store->sts_bso);
		fprintf(stderr,"dbg5       sts_bsn:             %d\n",store->sts_bsn);
		fprintf(stderr,"dbg5       sts_gain:            %d\n",store->sts_gain);
		fprintf(stderr,"dbg5       sts_dno:             %d\n",store->sts_dno);
		fprintf(stderr,"dbg5       sts_rno:             %d\n",store->sts_rno);
		fprintf(stderr,"dbg5       sts_port:            %d\n",store->sts_port);
		fprintf(stderr,"dbg5       sts_stbd:            %d\n",store->sts_stbd);
		fprintf(stderr,"dbg5       sts_ssp:             %d\n",store->sts_ssp);
		fprintf(stderr,"dbg5       sts_yaw:             %d\n",store->sts_yaw);
		fprintf(stderr,"dbg5       sts_port2:           %d\n",store->sts_port2);
		fprintf(stderr,"dbg5       sts_stbd2:           %d\n",store->sts_stbd2);
		fprintf(stderr,"dbg5       sts_spare2:          %d\n",store->sts_spare2);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_STATUS_SIZE), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_STATUS), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* construct binary data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) store->sts_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) store->sts_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) store->sts_status_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) store->run_serial, (void *) &line[10]);
		mb_put_binary_short(swap, (unsigned short) store->sts_pingrate, (void *) &line[12]);
		mb_put_binary_short(swap, (unsigned short) store->sts_ping_count, (void *) &line[14]);
		mb_put_binary_int(swap, (int) store->sts_load, (void *) &line[16]); 
		mb_put_binary_int(swap, (int) store->sts_udp_status, (void *) &line[20]); 
		mb_put_binary_int(swap, (int) store->sts_serial1_status, (void *) &line[24]); 
		mb_put_binary_int(swap, (int) store->sts_serial2_status, (void *) &line[28]); 
		mb_put_binary_int(swap, (int) store->sts_serial3_status, (void *) &line[32]); 
		mb_put_binary_int(swap, (int) store->sts_serial3_status, (void *) &line[36]); 
		line[40] = store->sts_pps_status;
		line[41] = store->sts_position_status;
		line[42] = store->sts_attitude_status;
		line[43] = store->sts_clock_status;
		line[44] = store->sts_heading_status;
		line[45] = store->sts_pu_status;
		mb_put_binary_short(swap, (unsigned short) store->sts_last_heading, (void *) &line[46]);
		mb_put_binary_short(swap, (short) store->sts_last_roll, (void *) &line[48]);
		mb_put_binary_short(swap, (short) store->sts_last_pitch, (void *) &line[50]);
		mb_put_binary_short(swap, (short) store->sts_last_heave, (void *) &line[52]);
		mb_put_binary_short(swap, (unsigned short) store->sts_last_ssv, (void *) &line[54]);
		mb_put_binary_int(swap, (int) store->sts_last_depth, (void *) &line[56]); 
		mb_put_binary_int(swap, (int) store->sts_spare, (void *) &line[60]); 
		line[64] = store->sts_bso;
		line[65] = store->sts_bsn;
		line[66] = store->sts_gain;
		line[67] = store->sts_dno;
		mb_put_binary_short(swap, (unsigned short) store->sts_rno, (void *) &line[68]);
		line[70] = store->sts_port;
		line[71] = store->sts_stbd;
		mb_put_binary_short(swap, (unsigned short) store->sts_ssp, (void *) &line[72]);
		mb_put_binary_short(swap, (unsigned short) store->sts_yaw, (void *) &line[74]);
		mb_put_binary_short(swap, (unsigned short) store->sts_port2, (void *) &line[76]);
		mb_put_binary_short(swap, (unsigned short) store->sts_stbd2, (void *) &line[78]);
		line[80] = store->sts_spare2;
		line[EM3_STATUS_SIZE-7] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_STATUS_SIZE-7;j++)
		    checksum += uchar_ptr[j];
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[EM3_STATUS_SIZE-6]);

		/* write out data */
		write_len = fwrite(line,1,EM3_STATUS_SIZE-4,mbfp);
		if (write_len != EM3_STATUS_SIZE-4)
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
int mbr_em710raw_wr_run_parameter(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_run_parameter";
	int	status = MB_SUCCESS;
	char	line[EM3_RUN_PARAMETER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       run_date:        %d\n",store->run_date);
		fprintf(stderr,"dbg5       run_msec:        %d\n",store->run_msec);
		fprintf(stderr,"dbg5       run_ping_count:  %d\n",store->run_ping_count);
		fprintf(stderr,"dbg5       run_serial:      %d\n",store->run_serial);
		fprintf(stderr,"dbg5       run_status:      %d\n",store->run_status);
		fprintf(stderr,"dbg5       run_mode:        %d\n",store->run_mode);
		fprintf(stderr,"dbg5       run_filter_id:   %d\n",store->run_filter_id);
		fprintf(stderr,"dbg5       run_min_depth:   %d\n",store->run_min_depth);
		fprintf(stderr,"dbg5       run_max_depth:   %d\n",store->run_max_depth);
		fprintf(stderr,"dbg5       run_absorption:  %d\n",store->run_absorption);
		fprintf(stderr,"dbg5       run_tran_pulse:  %d\n",store->run_tran_pulse);
		fprintf(stderr,"dbg5       run_tran_beam:   %d\n",store->run_tran_beam);
		fprintf(stderr,"dbg5       run_tran_pow:    %d\n",store->run_tran_pow);
		fprintf(stderr,"dbg5       run_rec_beam:    %d\n",store->run_rec_beam);
		fprintf(stderr,"dbg5       run_rec_band:    %d\n",store->run_rec_band);
		fprintf(stderr,"dbg5       run_rec_gain:    %d\n",store->run_rec_gain);
		fprintf(stderr,"dbg5       run_tvg_cross:   %d\n",store->run_tvg_cross);
		fprintf(stderr,"dbg5       run_ssv_source:  %d\n",store->run_ssv_source);
		fprintf(stderr,"dbg5       run_max_swath:   %d\n",store->run_max_swath);
		fprintf(stderr,"dbg5       run_beam_space:  %d\n",store->run_beam_space);
		fprintf(stderr,"dbg5       run_swath_angle: %d\n",store->run_swath_angle);
		fprintf(stderr,"dbg5       run_stab_mode:   %d\n",store->run_stab_mode);
		for (i=0;i<6;i++)
			fprintf(stderr,"dbg5       run_spare[%d]:    %d\n",i,store->run_spare[i]);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_RUN_PARAMETER_SIZE), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_RUN_PARAMETER), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* construct binary data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) store->run_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) store->run_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) store->run_ping_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) store->run_serial, (void *) &line[10]);
		mb_put_binary_int(swap, (int) store->run_status, (void *) &line[12]); 
		line[16] = store->run_mode;
		line[17] = store->run_filter_id;
		mb_put_binary_short(swap, (unsigned short) store->run_min_depth, (void *) &line[18]);
		mb_put_binary_short(swap, (unsigned short) store->run_max_depth, (void *) &line[20]);
		mb_put_binary_short(swap, (unsigned short) store->run_absorption, (void *) &line[22]);
		mb_put_binary_short(swap, (unsigned short) store->run_tran_pulse, (void *) &line[24]);
		mb_put_binary_short(swap, (unsigned short) store->run_tran_beam, (void *) &line[26]);
		line[28] = store->run_tran_pow;
		line[29] = store->run_rec_beam;
		line[30] = store->run_rec_band;
		line[31] = store->run_rec_gain;
		line[32] = store->run_tvg_cross;
		line[33] = store->run_ssv_source;
		mb_put_binary_short(swap, (unsigned short) store->run_max_swath, (void *) &line[34]);
		line[36] = store->run_beam_space;
		line[37] = store->run_swath_angle;
		line[38] = store->run_stab_mode;
		for (i=0;i<6;i++)
		    line[39+i] = store->run_spare[i];
		line[EM3_RUN_PARAMETER_SIZE-7] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_RUN_PARAMETER_SIZE-7;j++)
		    checksum += uchar_ptr[j];
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[EM3_RUN_PARAMETER_SIZE-6]);

		/* write out data */
		write_len = fwrite(line,1,EM3_RUN_PARAMETER_SIZE-4,mbfp);
		if (write_len != EM3_RUN_PARAMETER_SIZE-4)
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
int mbr_em710raw_wr_clock(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_clock";
	int	status = MB_SUCCESS;
	char	line[EM3_CLOCK_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       clk_date:        %d\n",store->clk_date);
		fprintf(stderr,"dbg5       clk_msec:        %d\n",store->clk_msec);
		fprintf(stderr,"dbg5       clk_count:       %d\n",store->clk_count);
		fprintf(stderr,"dbg5       clk_serial:      %d\n",store->clk_serial);
		fprintf(stderr,"dbg5       clk_origin_date: %d\n",store->clk_origin_date);
		fprintf(stderr,"dbg5       clk_origin_msec: %d\n",store->clk_origin_msec);
		fprintf(stderr,"dbg5       clk_1_pps_use:   %d\n",store->clk_1_pps_use);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_CLOCK_SIZE), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_CLOCK), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* construct binary data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) store->clk_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) store->clk_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) store->clk_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) store->clk_serial, (void *) &line[10]);
		mb_put_binary_int(swap, (int) store->clk_origin_date, (void *) &line[12]); 
		mb_put_binary_int(swap, (int) store->clk_origin_msec, (void *) &line[16]); 
		line[20] = store->clk_1_pps_use;
		line[EM3_CLOCK_SIZE-7] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_CLOCK_SIZE-7;j++)
		    checksum += uchar_ptr[j];
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[EM3_CLOCK_SIZE-6]);

		/* write out data */
		write_len = fwrite(line,1,EM3_CLOCK_SIZE-4,mbfp);
		if (write_len != EM3_CLOCK_SIZE-4)
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
int mbr_em710raw_wr_tide(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_tide";
	int	status = MB_SUCCESS;
	char	line[EM3_TIDE_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       tid_date:        %d\n",store->tid_date);
		fprintf(stderr,"dbg5       tid_msec:        %d\n",store->tid_msec);
		fprintf(stderr,"dbg5       tid_count:       %d\n",store->tid_count);
		fprintf(stderr,"dbg5       tid_serial:      %d\n",store->tid_serial);
		fprintf(stderr,"dbg5       tid_origin_date: %d\n",store->tid_origin_date);
		fprintf(stderr,"dbg5       tid_origin_msec: %d\n",store->tid_origin_msec);
		fprintf(stderr,"dbg5       tid_tide:        %d\n",store->tid_tide);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_TIDE_SIZE), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_TIDE), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* construct binary data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) store->tid_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) store->tid_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) store->tid_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) store->tid_serial, (void *) &line[10]);
		mb_put_binary_int(swap, (int) store->tid_origin_date, (void *) &line[12]); 
		mb_put_binary_int(swap, (int) store->tid_origin_msec, (void *) &line[16]); 
		mb_put_binary_short(swap, (short) store->tid_tide, (void *) &line[20]);
		line[EM3_TIDE_SIZE-8] = '\0';
		line[EM3_TIDE_SIZE-7] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_TIDE_SIZE-7;j++)
		    checksum += uchar_ptr[j];
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[EM3_TIDE_SIZE-6]);

		/* write out data */
		write_len = fwrite(line,1,EM3_TIDE_SIZE-4,mbfp);
		if (write_len != EM3_TIDE_SIZE-4)
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
int mbr_em710raw_wr_height(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_height";
	int	status = MB_SUCCESS;
	char	line[EM3_HEIGHT_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       hgt_date:        %d\n",store->hgt_date);
		fprintf(stderr,"dbg5       hgt_msec:        %d\n",store->hgt_msec);
		fprintf(stderr,"dbg5       hgt_count:       %d\n",store->hgt_count);
		fprintf(stderr,"dbg5       hgt_serial:      %d\n",store->hgt_serial);
		fprintf(stderr,"dbg5       hgt_height:      %d\n",store->hgt_height);
		fprintf(stderr,"dbg5       hgt_type:        %d\n",store->hgt_type);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_HEIGHT_SIZE), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_HEIGHT), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* construct binary data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) store->hgt_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) store->hgt_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) store->hgt_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) store->hgt_serial, (void *) &line[10]);
		mb_put_binary_int(swap, (int) store->hgt_height, (void *) &line[12]); 
		line[16] = (mb_u_char) store->hgt_type;
		line[EM3_HEIGHT_SIZE-7] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_HEIGHT_SIZE-7;j++)
		    checksum += uchar_ptr[j];
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[EM3_HEIGHT_SIZE-6]);

		/* write out data */
		write_len = fwrite(line,1,EM3_HEIGHT_SIZE-4,mbfp);
		if (write_len != EM3_HEIGHT_SIZE-4)
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
int mbr_em710raw_wr_heading(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_heading";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_heading_struct *heading;
	char	line[EM3_HEADING_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}
		
	/* get storage structure */
	heading = (struct mbsys_simrad3_heading_struct *) store->heading;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       hed_date:        %d\n",heading->hed_date);
		fprintf(stderr,"dbg5       hed_msec:        %d\n",heading->hed_msec);
		fprintf(stderr,"dbg5       hed_count:       %d\n",heading->hed_count);
		fprintf(stderr,"dbg5       hed_serial:      %d\n",heading->hed_serial);
		fprintf(stderr,"dbg5       hed_ndata:       %d\n",heading->hed_ndata);
		fprintf(stderr,"dbg5       count    time (msec)    heading (0.01 deg)\n");
		fprintf(stderr,"dbg5       -----    -----------    ------------------\n");
		for (i=0;i<heading->hed_ndata;i++)
			fprintf(stderr,"dbg5        %4d      %7d          %7d\n",
				i, heading->hed_time[i], heading->hed_heading[i]);
		fprintf(stderr,"dbg5       hed_heading_status: %d\n",heading->hed_heading_status);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_HEADING_HEADER_SIZE 
			+ EM3_HEADING_SLICE_SIZE * heading->hed_ndata + 8), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_HEADING), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) heading->hed_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) heading->hed_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) heading->hed_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) heading->hed_serial, (void *) &line[10]);
		mb_put_binary_short(swap, (unsigned short) heading->hed_ndata, (void *) &line[12]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_HEADING_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_HEADING_HEADER_SIZE,mbfp);
		if (write_len != EM3_HEADING_HEADER_SIZE)
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

	/* output binary heading data */
	if (status == MB_SUCCESS)
	    for (i=0;i<heading->hed_ndata;i++)
		{
		mb_put_binary_short(swap, (unsigned short) heading->hed_time[i], (void *) &line[0]);
		mb_put_binary_short(swap, (unsigned short) heading->hed_heading[i], (void *) &line[2]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_HEADING_SLICE_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_HEADING_SLICE_SIZE,mbfp);
		if (write_len != EM3_HEADING_SLICE_SIZE)
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

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[0] = (mb_u_char) heading->hed_heading_status;
		line[1] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum += uchar_ptr[0];
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[2]);

		/* write out data */
		write_len = fwrite(line,1,4,mbfp);
		if (write_len != 4)
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
int mbr_em710raw_wr_ssv(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_ssv";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_ssv_struct *ssv;
	char	line[EM3_SSV_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}
		
	/* get storage structure */
	ssv = (struct mbsys_simrad3_ssv_struct *) store->ssv;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       ssv_date:        %d\n",ssv->ssv_date);
		fprintf(stderr,"dbg5       ssv_msec:        %d\n",ssv->ssv_msec);
		fprintf(stderr,"dbg5       ssv_count:       %d\n",ssv->ssv_count);
		fprintf(stderr,"dbg5       ssv_serial:      %d\n",ssv->ssv_serial);
		fprintf(stderr,"dbg5       ssv_ndata:       %d\n",ssv->ssv_ndata);
		fprintf(stderr,"dbg5       count    time (msec)    ssv (0.1 m/s)\n");
		fprintf(stderr,"dbg5       -----    -----------    ------------------\n");
		for (i=0;i<ssv->ssv_ndata;i++)
			fprintf(stderr,"dbg5        %4d      %7d          %7d\n",
				i, ssv->ssv_time[i], ssv->ssv_ssv[i]);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_SSV_HEADER_SIZE 
			+ EM3_SSV_SLICE_SIZE * ssv->ssv_ndata + 8), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_SSV), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) ssv->ssv_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) ssv->ssv_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) ssv->ssv_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) ssv->ssv_serial, (void *) &line[10]);
		mb_put_binary_short(swap, (unsigned short) ssv->ssv_ndata, (void *) &line[12]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_SSV_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_SSV_HEADER_SIZE,mbfp);
		if (write_len != EM3_SSV_HEADER_SIZE)
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

	/* output binary ssv data */
	if (status == MB_SUCCESS)
	    for (i=0;i<ssv->ssv_ndata;i++)
		{
		mb_put_binary_short(swap, (unsigned short) ssv->ssv_time[i], (void *) &line[0]);
		mb_put_binary_short(swap, (unsigned short) ssv->ssv_ssv[i], (void *) &line[2]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_SSV_SLICE_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_SSV_SLICE_SIZE,mbfp);
		if (write_len != EM3_SSV_SLICE_SIZE)
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

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[0] = 0;
		line[1] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum += uchar_ptr[0];
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[2]);

		/* write out data */
		write_len = fwrite(line,1,4,mbfp);
		if (write_len != 4)
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
int mbr_em710raw_wr_tilt(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_tilt";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_tilt_struct *tilt;
	char	line[EM3_TILT_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}
		
	/* get storage structure */
	tilt = (struct mbsys_simrad3_tilt_struct *) store->tilt;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       tlt_date:        %d\n",tilt->tlt_date);
		fprintf(stderr,"dbg5       tlt_msec:        %d\n",tilt->tlt_msec);
		fprintf(stderr,"dbg5       tlt_count:       %d\n",tilt->tlt_count);
		fprintf(stderr,"dbg5       tlt_serial:      %d\n",tilt->tlt_serial);
		fprintf(stderr,"dbg5       tlt_ndata:       %d\n",tilt->tlt_ndata);
		fprintf(stderr,"dbg5       count    time (msec)    tilt (0.01 deg)\n");
		fprintf(stderr,"dbg5       -----    -----------    ------------------\n");
		for (i=0;i<tilt->tlt_ndata;i++)
			fprintf(stderr,"dbg5        %4d      %7d          %7d\n",
				i, tilt->tlt_time[i], tilt->tlt_tilt[i]);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_TILT_HEADER_SIZE 
			+ EM3_TILT_SLICE_SIZE * tilt->tlt_ndata + 8), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_TILT), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) tilt->tlt_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) tilt->tlt_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) tilt->tlt_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) tilt->tlt_serial, (void *) &line[10]);
		mb_put_binary_short(swap, (unsigned short) tilt->tlt_ndata, (void *) &line[12]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_TILT_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_TILT_HEADER_SIZE,mbfp);
		if (write_len != EM3_TILT_HEADER_SIZE)
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

	/* output binary tilt data */
	if (status == MB_SUCCESS)
	    for (i=0;i<tilt->tlt_ndata;i++)
		{
		mb_put_binary_short(swap, (unsigned short) tilt->tlt_time[i], (void *) &line[0]);
		mb_put_binary_short(swap, (unsigned short) tilt->tlt_tilt[i], (void *) &line[2]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_TILT_SLICE_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_TILT_SLICE_SIZE,mbfp);
		if (write_len != EM3_TILT_SLICE_SIZE)
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

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[0] = 0;
		line[1] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum += uchar_ptr[0];
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[2]);

		/* write out data */
		write_len = fwrite(line,1,4,mbfp);
		if (write_len != 4)
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
int mbr_em710raw_wr_attitude(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_attitude";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_attitude_struct *attitude;
	char	line[EM3_ATTITUDE_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}
		
	/* get storage structure */
	attitude = (struct mbsys_simrad3_attitude_struct *) store->attitude;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       att_date:        %d\n",attitude->att_date);
		fprintf(stderr,"dbg5       att_msec:        %d\n",attitude->att_msec);
		fprintf(stderr,"dbg5       att_count:       %d\n",attitude->att_count);
		fprintf(stderr,"dbg5       att_serial:      %d\n",attitude->att_serial);
		fprintf(stderr,"dbg5       att_ndata:       %d\n",attitude->att_ndata);
		fprintf(stderr,"dbg5       cnt   time   roll pitch heave heading\n");
		fprintf(stderr,"dbg5       -------------------------------------\n");
		for (i=0;i<attitude->att_ndata;i++)
			fprintf(stderr,"dbg5        %3d  %d  %d %d %d %d\n",
				i, attitude->att_time[i], attitude->att_roll[i], 
				attitude->att_pitch[i], attitude->att_heave[i], 
				attitude->att_heading[i]);
		fprintf(stderr,"dbg5       att_heading_status: %d\n",attitude->att_heading_status);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_ATTITUDE_HEADER_SIZE 
			+ EM3_ATTITUDE_SLICE_SIZE * attitude->att_ndata + 8), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_ATTITUDE), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) attitude->att_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) attitude->att_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) attitude->att_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) attitude->att_serial, (void *) &line[10]);
		mb_put_binary_short(swap, (unsigned short) attitude->att_ndata, (void *) &line[12]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_ATTITUDE_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_ATTITUDE_HEADER_SIZE,mbfp);
		if (write_len != EM3_ATTITUDE_HEADER_SIZE)
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

	/* output binary heading data */
	if (status == MB_SUCCESS)
	    for (i=0;i<attitude->att_ndata;i++)
		{
		mb_put_binary_short(swap, (unsigned short) attitude->att_time[i], (void *) &line[0]);
		mb_put_binary_short(swap, (unsigned short) attitude->att_sensor_status[i], (void *) &line[2]);
		mb_put_binary_short(swap, (short) attitude->att_roll[i], (void *) &line[4]);
		mb_put_binary_short(swap, (short) attitude->att_pitch[i], (void *) &line[6]);
		mb_put_binary_short(swap, (short) attitude->att_heave[i], (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) attitude->att_heading[i], (void *) &line[10]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_ATTITUDE_SLICE_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_ATTITUDE_SLICE_SIZE,mbfp);
		if (write_len != EM3_ATTITUDE_SLICE_SIZE)
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

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[0] = (mb_u_char) attitude->att_heading_status;
		line[1] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum += uchar_ptr[0];
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[2]);

		/* write out data */
		write_len = fwrite(line,1,4,mbfp);
		if (write_len != 4)
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
int mbr_em710raw_wr_pos(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_pos";
	int	status = MB_SUCCESS;
	char	line[EM3_POS_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       pos_date:        %d\n",store->pos_date);
		fprintf(stderr,"dbg5       pos_msec:        %d\n",store->pos_msec);
		fprintf(stderr,"dbg5       pos_count:       %d\n",store->pos_count);
		fprintf(stderr,"dbg5       pos_serial:      %d\n",store->pos_serial);
		fprintf(stderr,"dbg5       pos_latitude:    %d\n",store->pos_latitude);
		fprintf(stderr,"dbg5       pos_longitude:   %d\n",store->pos_longitude);
		fprintf(stderr,"dbg5       pos_quality:     %d\n",store->pos_quality);
		fprintf(stderr,"dbg5       pos_speed:       %d\n",store->pos_speed);
		fprintf(stderr,"dbg5       pos_course:      %d\n",store->pos_course);
		fprintf(stderr,"dbg5       pos_heading:     %d\n",store->pos_heading);
		fprintf(stderr,"dbg5       pos_system:      %d\n",store->pos_system);
		fprintf(stderr,"dbg5       pos_input_size:  %d\n",store->pos_input_size);
		fprintf(stderr,"dbg5       pos_input:\ndbg5            %s\n",store->pos_input);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_POS_HEADER_SIZE 
			+ store->pos_input_size 
			- (store->pos_input_size % 2) + 8), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_POS), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) store->pos_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) store->pos_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) store->pos_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) store->pos_serial, (void *) &line[10]);
		mb_put_binary_int(swap, (int) store->pos_latitude, (void *) &line[12]); 
		mb_put_binary_int(swap, (int) store->pos_longitude, (void *) &line[16]); 
		mb_put_binary_short(swap, (unsigned short) store->pos_quality, (void *) &line[20]);
		mb_put_binary_short(swap, (unsigned short) store->pos_speed, (void *) &line[22]);
		mb_put_binary_short(swap, (unsigned short) store->pos_course, (void *) &line[24]);
		mb_put_binary_short(swap, (unsigned short) store->pos_heading, (void *) &line[26]);
		line[28] = (mb_u_char) store->pos_system;
		line[29] = (mb_u_char) store->pos_input_size;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_POS_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_POS_HEADER_SIZE,mbfp);
		if (write_len != EM3_POS_HEADER_SIZE)
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

	/* output original ascii nav data */
	if (status == MB_SUCCESS)
		{
		write_size = store->pos_input_size 
				- (store->pos_input_size % 2) + 1;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) store->pos_input;
		for (j=0;j<write_size;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(store->pos_input,1,write_size,mbfp);
		if (write_len != write_size)
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

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[1] = 0x03;
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[2]);

		/* write out data */
		write_len = fwrite(&line[1],1,3,mbfp);
		if (write_len != 3)
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
int mbr_em710raw_wr_svp(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_svp";
	int	status = MB_SUCCESS;
	char	line[EM3_SVP_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       svp_use_date:    %d\n",store->svp_use_date);
		fprintf(stderr,"dbg5       svp_use_msec:    %d\n",store->svp_use_msec);
		fprintf(stderr,"dbg5       svp_count:       %d\n",store->svp_count);
		fprintf(stderr,"dbg5       svp_serial:      %d\n",store->svp_serial);
		fprintf(stderr,"dbg5       svp_origin_date: %d\n",store->svp_origin_date);
		fprintf(stderr,"dbg5       svp_origin_msec: %d\n",store->svp_origin_msec);
		fprintf(stderr,"dbg5       svp_num:         %d\n",store->svp_num);
		fprintf(stderr,"dbg5       svp_depth_res:   %d\n",store->svp_depth_res);
		fprintf(stderr,"dbg5       count    depth    speed\n");
		fprintf(stderr,"dbg5       -----------------------\n");
		for (i=0;i<store->svp_num;i++)
			fprintf(stderr,"dbg5        %d   %d  %d\n",
				i, store->svp_depth[i], store->svp_vel[i]);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_SVP_HEADER_SIZE 
			+ EM3_SVP_SLICE_SIZE * store->svp_num + 8), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_SVP), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) store->svp_use_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) store->svp_use_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) store->svp_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) store->svp_serial, (void *) &line[10]);
		mb_put_binary_int(swap, (int) store->svp_origin_date, (void *) &line[12]); 
		mb_put_binary_int(swap, (int) store->svp_origin_msec, (void *) &line[16]); 
		mb_put_binary_short(swap, (unsigned short) store->svp_num, (void *) &line[20]);
		mb_put_binary_short(swap, (unsigned short) store->svp_depth_res, (void *) &line[22]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_SVP_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_SVP_HEADER_SIZE,mbfp);
		if (write_len != EM3_SVP_HEADER_SIZE)
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

	/* output binary svp data */
	if (status == MB_SUCCESS)
	    for (i=0;i<store->svp_num;i++)
		{
		mb_put_binary_short(swap, (unsigned short) store->svp_depth[i], (void *) &line[0]); 
		mb_put_binary_short(swap, (unsigned short) store->svp_vel[i], (void *) &line[4]); 
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_SVP_SLICE_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_SVP_SLICE_SIZE,mbfp);
		if (write_len != EM3_SVP_SLICE_SIZE)
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

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[0] = '\0';
		line[1] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum += uchar_ptr[0];
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[2]);

		/* write out data */
		write_len = fwrite(line,1,4,mbfp);
		if (write_len != 4)
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
int mbr_em710raw_wr_svp2(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_svp2";
	int	status = MB_SUCCESS;
	char	line[EM3_SVP2_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       svp_use_date:    %d\n",store->svp_use_date);
		fprintf(stderr,"dbg5       svp_use_msec:    %d\n",store->svp_use_msec);
		fprintf(stderr,"dbg5       svp_count:       %d\n",store->svp_count);
		fprintf(stderr,"dbg5       svp_serial:      %d\n",store->svp_serial);
		fprintf(stderr,"dbg5       svp_origin_date: %d\n",store->svp_origin_date);
		fprintf(stderr,"dbg5       svp_origin_msec: %d\n",store->svp_origin_msec);
		fprintf(stderr,"dbg5       svp_num:         %d\n",store->svp_num);
		fprintf(stderr,"dbg5       svp_depth_res:   %d\n",store->svp_depth_res);
		fprintf(stderr,"dbg5       count    depth    speed\n");
		fprintf(stderr,"dbg5       -----------------------\n");
		for (i=0;i<store->svp_num;i++)
			fprintf(stderr,"dbg5        %d   %d  %d\n",
				i, store->svp_depth[i], store->svp_vel[i]);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_SVP2_HEADER_SIZE 
			+ EM3_SVP2_SLICE_SIZE * store->svp_num + 8), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_SVP2), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) store->svp_use_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) store->svp_use_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) store->svp_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) store->svp_serial, (void *) &line[10]);
		mb_put_binary_int(swap, (int) store->svp_origin_date, (void *) &line[12]); 
		mb_put_binary_int(swap, (int) store->svp_origin_msec, (void *) &line[16]); 
		mb_put_binary_short(swap, (unsigned short) store->svp_num, (void *) &line[20]);
		mb_put_binary_short(swap, (unsigned short) store->svp_depth_res, (void *) &line[22]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_SVP2_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_SVP2_HEADER_SIZE,mbfp);
		if (write_len != EM3_SVP2_HEADER_SIZE)
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

	/* output binary svp data */
	if (status == MB_SUCCESS)
	    for (i=0;i<store->svp_num;i++)
		{
		mb_put_binary_int(swap, (int) store->svp_depth[i], (void *) &line[0]); 
		mb_put_binary_int(swap, (int) store->svp_vel[i], (void *) &line[4]); 
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_SVP2_SLICE_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_SVP2_SLICE_SIZE,mbfp);
		if (write_len != EM3_SVP2_SLICE_SIZE)
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

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[0] = '\0';
		line[1] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum += uchar_ptr[0];
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[2]);

		/* write out data */
		write_len = fwrite(line,1,4,mbfp);
		if (write_len != 4)
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
int mbr_em710raw_wr_bath2(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_bath2";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_ping_struct *ping;
	char	line[EM3_BATH2_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}
		
	/* get storage structure */
	ping = (struct mbsys_simrad3_ping_struct *) store->ping;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:                  %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:                 %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:                  %d\n",store->date);
		fprintf(stderr,"dbg5       msec:                  %d\n",store->msec);
		fprintf(stderr,"dbg5       png_date:              %d\n",ping->png_date);
		fprintf(stderr,"dbg5       png_msec:              %d\n",ping->png_msec);
		fprintf(stderr,"dbg5       png_count:             %d\n",ping->png_count);
		fprintf(stderr,"dbg5       png_serial:            %d\n",ping->png_serial);
		fprintf(stderr,"dbg5       png_heading:           %d\n",ping->png_heading);
		fprintf(stderr,"dbg5       png_ssv:               %d\n",ping->png_ssv);
		fprintf(stderr,"dbg5       png_xducer_depth:      %f\n",ping->png_xducer_depth);
		fprintf(stderr,"dbg5       png_nbeams:            %d\n",ping->png_nbeams);
		fprintf(stderr,"dbg5       png_nbeams_valid:      %d\n",ping->png_nbeams_valid);
		fprintf(stderr,"dbg5       png_sample_rate:       %f\n",ping->png_sample_rate);
		fprintf(stderr,"dbg5       png_spare:             %d\n",ping->png_spare);
		fprintf(stderr,"dbg5       cnt  depth xtrack ltrack dprsn   azi   rng  qual wnd amp num\n");
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		for (i=0;i<ping->png_nbeams;i++)
			fprintf(stderr,"dbg5       %3d %7.2f %7.2f %7.2f %5d %5d %5d %4d %3d %3d\n",
				i, ping->png_depth[i], ping->png_acrosstrack[i], 
				ping->png_alongtrack[i], ping->png_window[i], 
				ping->png_quality[i], ping->png_iba[i], 
				ping->png_detection[i], ping->png_clean[i], 
				ping->png_amp[i]);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_BATH2_HEADER_SIZE 
			+ EM3_BATH2_BEAM_SIZE * ping->png_nbeams + 8), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_BATH2), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) ping->png_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) ping->png_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) ping->png_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) ping->png_serial, (void *) &line[10]);
		mb_put_binary_short(swap, (unsigned short) ping->png_heading, (void *) &line[12]);
		mb_put_binary_short(swap, (unsigned short) ping->png_ssv, (void *) &line[14]);
		mb_put_binary_float(swap, (float) ping->png_xducer_depth, (void *) &line[16]);
		mb_put_binary_short(swap, (unsigned short) ping->png_nbeams, (void *) &line[20]);
		mb_put_binary_short(swap, (unsigned short) ping->png_nbeams_valid, (void *) &line[22]);
		mb_put_binary_float(swap, (float) ping->png_sample_rate, (void *) &line[24]);
		mb_put_binary_short(swap, (unsigned short) ping->png_spare, (void *) &line[28]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_BATH2_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_BATH2_HEADER_SIZE,mbfp);
		if (write_len != EM3_BATH2_HEADER_SIZE)
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

	/* output binary beam data */
	if (status == MB_SUCCESS)
	    for (i=0;i<ping->png_nbeams;i++)
		{
		mb_put_binary_float(swap, ping->png_depth[i], (void *) &line[0]);
		mb_put_binary_float(swap, ping->png_acrosstrack[i], (void *) &line[4]);
		mb_put_binary_float(swap, ping->png_alongtrack[i], (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) ping->png_window[i], (void *) &line[12]);
		line[14] = (mb_u_char) ping->png_quality[i];
		line[15] = (mb_s_char) ping->png_iba[i];
		line[16] = (mb_u_char) ping->png_detection[i];
		line[17] = (mb_s_char) ping->png_clean[i];
		mb_put_binary_short(swap, (short) ping->png_amp[i], (void *) &line[18]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_BATH2_BEAM_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_BATH2_BEAM_SIZE,mbfp);
		if (write_len != EM3_BATH2_BEAM_SIZE)
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

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[0] = 0;
		line[1] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum += uchar_ptr[0];
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[2]);

		/* write out data */
		write_len = fwrite(line,1,4,mbfp);
		if (write_len != 4)
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
int mbr_em710raw_wr_rawbeam4(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_rawbeam4";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_ping_struct *ping;
	char	line[EM3_RAWBEAM4_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}
		
	/* get storage structure */
	ping = (struct mbsys_simrad3_ping_struct *) store->ping;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       png_raw4_read:               %d\n",ping->png_raw4_read);
		fprintf(stderr,"dbg5       png_raw_date:                %d\n",ping->png_raw_date);
		fprintf(stderr,"dbg5       png_raw_msec:                %d\n",ping->png_raw_msec);
		fprintf(stderr,"dbg5       png_raw_count:               %d\n",ping->png_raw_count);
		fprintf(stderr,"dbg5       png_raw_serial:              %d\n",ping->png_raw_serial);
		fprintf(stderr,"dbg5       png_raw_ssv:                 %d\n",ping->png_raw_ssv);
		fprintf(stderr,"dbg5       png_raw_ntx:                 %d\n",ping->png_raw_ntx);
		fprintf(stderr,"dbg5       png_raw_nbeams:              %d\n",ping->png_raw_nbeams);
		fprintf(stderr,"dbg5       png_raw_detections:          %d\n",ping->png_raw_detections);
		fprintf(stderr,"dbg5       png_raw_sample_rate:         %f\n",ping->png_raw_sample_rate);
		fprintf(stderr,"dbg5       png_raw_spare:               %d\n",ping->png_raw_spare);
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		fprintf(stderr,"dbg5       transmit pulse values:\n");
		fprintf(stderr,"dbg5       tiltangle focus length offset center bandwidth waveform sector\n");
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		for (i=0;i<ping->png_raw_ntx;i++)
			fprintf(stderr,"dbg5       %3d %5d %5d %f %f %f %4d %4d %4d %f\n",
				i, ping->png_raw_txtiltangle[i], 
				ping->png_raw_txfocus[i], ping->png_raw_txsignallength[i], 
				ping->png_raw_txoffset[i], ping->png_raw_txcenter[i], 
				ping->png_raw_txabsorption[i], ping->png_raw_txwaveform[i], 
				ping->png_raw_txsector[i], ping->png_raw_txbandwidth[i]);
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		fprintf(stderr,"dbg5       beam values:\n");
		fprintf(stderr,"dbg5       angle range sector amp quality window beam\n");
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		for (i=0;i<ping->png_raw_nbeams;i++)
			fprintf(stderr,"dbg5       %3d %5d %3d %3d %4d %3d %5d %f %5d %5d %5d\n",
				i, ping->png_raw_rxpointangle[i], ping->png_raw_rxsector[i], 
				ping->png_raw_rxdetection[i], ping->png_raw_rxwindow[i], 
				ping->png_raw_rxquality[i], ping->png_raw_rxspare1[i],  
				ping->png_raw_rxrange[i],ping->png_raw_rxamp[i], 
				ping->png_raw_rxcleaning[i],ping->png_raw_rxspare2[i]);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_RAWBEAM4_HEADER_SIZE 
			+ EM3_RAWBEAM4_TX_SIZE * ping->png_raw_ntx
			+ EM3_RAWBEAM4_BEAM_SIZE * ping->png_raw_nbeams + 8), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_RAWBEAM4), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	/* write the sonar id */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) ping->png_raw_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) ping->png_raw_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) ping->png_raw_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) ping->png_raw_serial, (void *) &line[10]);
		mb_put_binary_short(swap, (unsigned short) ping->png_raw_ssv, (void *) &line[12]);
		mb_put_binary_short(swap, (unsigned short) ping->png_raw_ntx, (void *) &line[14]);
		mb_put_binary_short(swap, (unsigned short) ping->png_raw_nbeams, (void *) &line[16]);
		mb_put_binary_short(swap, (unsigned short) ping->png_raw_detections, (void *) &line[18]);
		mb_put_binary_float(swap, ping->png_raw_sample_rate, (void *) &line[20]); 
		mb_put_binary_int(swap, (int) ping->png_raw_spare, (void *) &line[24]); 
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_RAWBEAM4_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_RAWBEAM4_HEADER_SIZE,mbfp);
		if (write_len != EM3_RAWBEAM4_HEADER_SIZE)
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

	/* output binary tx data */
	if (status == MB_SUCCESS)
	    for (i=0;i<ping->png_raw_ntx;i++)
		{
		mb_put_binary_short(swap, (short) ping->png_raw_txtiltangle[i], (void *) &line[0]);
		mb_put_binary_short(swap, (unsigned short) ping->png_raw_txfocus[i], (void *) &line[2]);
		mb_put_binary_float(swap, ping->png_raw_txsignallength[i], (void *) &line[4]); 
		mb_put_binary_float(swap, ping->png_raw_txoffset[i], (void *) &line[8]); 
		mb_put_binary_float(swap, ping->png_raw_txcenter[i], (void *) &line[12]); 
		mb_put_binary_short(swap, (unsigned short) ping->png_raw_txabsorption[i], (void *) &line[16]);
		line[18] = (mb_u_char) ping->png_raw_txwaveform[i];
		line[19] = (mb_u_char) ping->png_raw_txsector[i];
		mb_put_binary_float(swap, ping->png_raw_txbandwidth[i], (void *) &line[20]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_RAWBEAM4_TX_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_RAWBEAM4_TX_SIZE,mbfp);
		if (write_len != EM3_RAWBEAM4_TX_SIZE)
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

	/* output binary beam data */
	if (status == MB_SUCCESS)
	    for (i=0;i<ping->png_raw_nbeams;i++)
		{
		mb_put_binary_short(swap, (short) ping->png_raw_rxpointangle[i], (void *) &line[0]);
		line[2] = (mb_u_char) ping->png_raw_rxsector[i];
		line[3] = (mb_u_char) ping->png_raw_rxdetection[i];
		mb_put_binary_short(swap, (short) ping->png_raw_rxwindow[i], (void *) &line[4]);
		line[6] = (mb_u_char) ping->png_raw_rxquality[i];
		line[7] = (mb_u_char) ping->png_raw_rxspare1[i];
		mb_put_binary_float(swap, ping->png_raw_rxrange[i], (void *) &line[8]);
		mb_put_binary_short(swap, (short) ping->png_raw_rxamp[i], (void *) &line[12]);
		line[14] = (mb_u_char) ping->png_raw_rxcleaning[i];
		line[15] = (mb_u_char) ping->png_raw_rxspare2[i];
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_RAWBEAM4_BEAM_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_RAWBEAM4_BEAM_SIZE,mbfp);
		if (write_len != EM3_RAWBEAM4_BEAM_SIZE)
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

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[0] = 0;
		line[1] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum += uchar_ptr[0];
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[2]);

		/* write out data */
		write_len = fwrite(line,1,4,mbfp);
		if (write_len != 4)
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
int mbr_em710raw_wr_ss2(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_ss2";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_ping_struct *ping;
	char	line[EM3_SS2_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}
		
	/* get storage structure */
	ping = (struct mbsys_simrad3_ping_struct *) store->ping;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:               %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:              %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:               %d\n",store->date);
		fprintf(stderr,"dbg5       msec:               %d\n",store->msec);
		fprintf(stderr,"dbg5       png_date:           %d\n",ping->png_date);
		fprintf(stderr,"dbg5       png_msec:           %d\n",ping->png_msec);
		
		fprintf(stderr,"dbg5       png_date:              %d\n",ping->png_date);
		fprintf(stderr,"dbg5       png_msec:              %d\n",ping->png_msec);
		fprintf(stderr,"dbg5       png_count:             %d\n",ping->png_count);
		fprintf(stderr,"dbg5       png_serial:            %d\n",ping->png_serial);
		fprintf(stderr,"dbg5       png_heading:           %d\n",ping->png_heading);
		fprintf(stderr,"dbg5       png_ssv:               %d\n",ping->png_ssv);
		fprintf(stderr,"dbg5       png_xducer_depth:      %f\n",ping->png_xducer_depth);
		fprintf(stderr,"dbg5       png_nbeams:            %d\n",ping->png_nbeams);
		fprintf(stderr,"dbg5       png_nbeams_valid:      %d\n",ping->png_nbeams_valid);
		fprintf(stderr,"dbg5       png_sample_rate:       %f\n",ping->png_sample_rate);
		fprintf(stderr,"dbg5       png_spare:             %d\n",ping->png_spare);
		fprintf(stderr,"dbg5       cnt  depth xtrack ltrack dprsn   azi   rng  qual wnd amp num\n");
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		for (i=0;i<ping->png_nbeams;i++)
			fprintf(stderr,"dbg5       %3d %7.2f %7.2f %7.2f %5d %5d %5d %4d %3d %3d\n",
				i, ping->png_depth[i], ping->png_acrosstrack[i], 
				ping->png_alongtrack[i], ping->png_window[i], 
				ping->png_quality[i], ping->png_iba[i], 
				ping->png_detection[i], ping->png_clean[i], 
				ping->png_amp[i]);

		fprintf(stderr,"dbg5       png_ss_date:        %d\n",ping->png_ss_date);
		fprintf(stderr,"dbg5       png_ss_msec:        %d\n",ping->png_ss_msec);
		fprintf(stderr,"dbg5       png_ss_count:       %d\n",ping->png_ss_count);
		fprintf(stderr,"dbg5       png_ss_serial:      %d\n",ping->png_ss_serial);
		fprintf(stderr,"dbg5       png_ss_sample_rate: %f\n",ping->png_ss_sample_rate);
		fprintf(stderr,"dbg5       png_r_zero:         %d\n",ping->png_r_zero);
		fprintf(stderr,"dbg5       png_bsn:            %d\n",ping->png_bsn);
		fprintf(stderr,"dbg5       png_bso:            %d\n",ping->png_bso);
		fprintf(stderr,"dbg5       png_tx:             %d\n",ping->png_tx);
		fprintf(stderr,"dbg5       png_tvg_crossover:  %d\n",ping->png_tvg_crossover);
		fprintf(stderr,"dbg5       png_nbeams_ss:      %d\n",ping->png_nbeams_ss);
		fprintf(stderr,"dbg5       png_npixels:        %d\n",ping->png_npixels);
		fprintf(stderr,"dbg5       cnt  index sort samples start center\n");
		fprintf(stderr,"dbg5       --------------------------------------------------\n");
		for (i=0;i<ping->png_nbeams_ss;i++)
			fprintf(stderr,"dbg5        %4d %2d %4d %4d %4d %4d\n",
				i, ping->png_sort_direction[i], ping->png_ssdetection[i], 
				ping->png_beam_samples[i], ping->png_start_sample[i], 
				ping->png_center_sample[i]);
		fprintf(stderr,"dbg5       cnt  ss\n");
		fprintf(stderr,"dbg5       --------------------------------------------------\n");
		for (i=0;i<ping->png_npixels;i++)
			fprintf(stderr,"dbg5        %d %d\n",
				i, ping->png_ssraw[i]);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int) (EM3_SS2_HEADER_SIZE 
			+ EM3_SS2_BEAM_SIZE * ping->png_nbeams_ss 
			+ ping->png_npixels - (ping->png_npixels % 2) + 8), (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_SS2), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
			
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) ping->png_ss_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) ping->png_ss_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) ping->png_ss_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) ping->png_ss_serial, (void *) &line[10]);
		mb_put_binary_float(swap, ping->png_ss_sample_rate, (void *) &line[12]);
		mb_put_binary_short(swap, (unsigned short) ping->png_r_zero, (void *) &line[16]);
		mb_put_binary_short(swap, (short) ping->png_bsn, (void *) &line[18]);
		mb_put_binary_short(swap, (short) ping->png_bso, (void *) &line[20]);
		mb_put_binary_short(swap, (unsigned short) ping->png_tx, (void *) &line[22]);
		mb_put_binary_short(swap, (unsigned short) ping->png_tvg_crossover, (void *) &line[24]);
		mb_put_binary_short(swap, (unsigned short) ping->png_nbeams_ss, (void *) &line[26]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_SS2_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_SS2_HEADER_SIZE,mbfp);
		if (write_len != EM3_SS2_HEADER_SIZE)
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

	/* output binary beam data */
	if (status == MB_SUCCESS)
	    for (i=0;i<ping->png_nbeams_ss;i++)
		{
		line[0] = (mb_s_char) ping->png_sort_direction[i];
		line[1] = (mb_u_char) ping->png_ssdetection[i];
		mb_put_binary_short(swap, (unsigned short) ping->png_beam_samples[i], (void *) &line[2]);
		mb_put_binary_short(swap, (unsigned short) ping->png_center_sample[i], (void *) &line[4]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_SS2_BEAM_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_SS2_BEAM_SIZE,mbfp);
		if (write_len != EM3_SS2_BEAM_SIZE)
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

	/* output sidescan data */
	if (status == MB_SUCCESS)
		{
		write_size = 2 * ping->png_npixels;

		/* compute checksum */
		uchar_ptr = (mb_u_char *) ping->png_ssraw;
		for (j=0;j<write_size;j++)
		    checksum += uchar_ptr[j];

		write_len = fwrite(ping->png_ssraw,1,write_size,mbfp);
		if (write_len != write_size)
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

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		line[1] = 0x03;
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[2]);

		/* write out data */
		write_len = fwrite(&line[1],1,3,mbfp);
		if (write_len != 3)
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
int mbr_em710raw_wr_wc(int verbose, FILE *mbfp, int swap, 
		struct mbsys_simrad3_struct *store, int *error)
{
	char	*function_name = "mbr_em710raw_wr_wc";
	int	status = MB_SUCCESS;
	struct mbsys_simrad3_watercolumn_struct *wc;
	char	line[EM3_WC_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	mb_u_char   *uchar_ptr;
	int	record_size;
	int	pad;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       swap:       %d\n",swap);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}
		
	/* get storage structure */
	wc = (struct mbsys_simrad3_watercolumn_struct *) store->wc;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       wtc_date:        %d\n",wc->wtc_date);
		fprintf(stderr,"dbg5       wtc_msec:        %d\n",wc->wtc_msec);
		fprintf(stderr,"dbg5       wtc_count:       %d\n",wc->wtc_count);
		fprintf(stderr,"dbg5       wtc_serial:      %d\n",wc->wtc_serial);
		fprintf(stderr,"dbg5       wtc_ndatagrams:  %d\n",wc->wtc_ndatagrams);
		fprintf(stderr,"dbg5       wtc_datagram:    %d\n",wc->wtc_datagram);
		fprintf(stderr,"dbg5       wtc_ntx:         %d\n",wc->wtc_ntx);
		fprintf(stderr,"dbg5       wtc_nrx:         %d\n",wc->wtc_nrx);
		fprintf(stderr,"dbg5       wtc_nbeam:       %d\n",wc->wtc_nbeam);
		fprintf(stderr,"dbg5       wtc_ssv:         %d\n",wc->wtc_ssv);
		fprintf(stderr,"dbg5       wtc_sfreq:       %d\n",wc->wtc_sfreq);
		fprintf(stderr,"dbg5       wtc_heave:       %d\n",wc->wtc_heave);
		fprintf(stderr,"dbg5       wtc_spare1:      %d\n",wc->wtc_spare1);
		fprintf(stderr,"dbg5       wtc_spare2:      %d\n",wc->wtc_spare2);
		fprintf(stderr,"dbg5       wtc_spare3:      %d\n",wc->wtc_spare3);
		fprintf(stderr,"dbg5       ---------------------------\n");
		fprintf(stderr,"dbg5       cnt  tilt center sector\n");
		fprintf(stderr,"dbg5       ---------------------------\n");
		for (i=0;i<wc->wtc_ntx;i++)
			fprintf(stderr,"dbg5       %3d %6d %6d %6d\n",
				i, wc->wtc_txtiltangle[i], wc->wtc_txcenter[i], 
				wc->wtc_txsector[i]);
		for (i=0;i<wc->wtc_nbeam;i++)
			{
			fprintf(stderr,"dbg5       --------------------------------------------------\n");
			fprintf(stderr,"dbg5       cnt  angle start samples unknown sector beam\n");
			fprintf(stderr,"dbg5       --------------------------------------------------\n");
			fprintf(stderr,"dbg5        %4d %3d %2d %4d %4d %4d %4d\n",
				i, wc->beam[i].wtc_rxpointangle, 
				wc->beam[i].wtc_start_sample, 
				wc->beam[i].wtc_beam_samples, 
				wc->beam[i].wtc_beam_spare, 
				wc->beam[i].wtc_sector, 
				wc->beam[i].wtc_beam);
/*			fprintf(stderr,"dbg5       --------------------------------------------------\n");
			fprintf(stderr,"dbg5       beam[%d]: sample amplitude\n",i);
			fprintf(stderr,"dbg5       --------------------------------------------------\n");
			for (j=0;j<wc->beam[i].wtc_beam_samples;j++)
				fprintf(stderr,"dbg5        %4d %4d\n",
					j, wc->beam[i].wtc_amp[j]);*/
			}
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	record_size = EM3_WC_HEADER_SIZE 
			+ EM3_WC_BEAM_SIZE * wc->wtc_nbeam 
			+ EM3_WC_TX_SIZE * wc->wtc_ntx + 8;
	for (i=0;i<wc->wtc_nbeam;i++)
		{
		record_size += wc->beam[i].wtc_beam_samples;
		}
	pad = (record_size % 2);
	record_size += pad;
	mb_put_binary_int(swap, record_size, (void *) &write_size); 
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (EM3_WATERCOLUMN), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[1];
		}

	if (status == MB_SUCCESS)
		{
		mb_put_binary_short(swap, (short) (store->sonar), (void *) &label); 
		write_len = fwrite(&label,1,2,mbfp);
		if (write_len != 2)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			status = MB_SUCCESS;
			
		/* compute checksum */
		uchar_ptr = (mb_u_char *) &label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
		}

	/* output binary header data */
	if (status == MB_SUCCESS)
		{
		mb_put_binary_int(swap, (int) wc->wtc_date, (void *) &line[0]); 
		mb_put_binary_int(swap, (int) wc->wtc_msec, (void *) &line[4]); 
		mb_put_binary_short(swap, (unsigned short) wc->wtc_count, (void *) &line[8]);
		mb_put_binary_short(swap, (unsigned short) wc->wtc_serial, (void *) &line[10]);
		mb_put_binary_short(swap, (unsigned short) wc->wtc_ndatagrams, (void *) &line[12]);
		mb_put_binary_short(swap, (unsigned short) wc->wtc_datagram, (void *) &line[14]);
		mb_put_binary_short(swap, (unsigned short) wc->wtc_ntx, (void *) &line[16]);
		mb_put_binary_short(swap, (unsigned short) wc->wtc_nrx, (void *) &line[18]);
		mb_put_binary_short(swap, (unsigned short) wc->wtc_nbeam, (void *) &line[20]);
		mb_put_binary_short(swap, (unsigned short) wc->wtc_ssv, (void *) &line[22]);
		mb_put_binary_int(swap, (int) wc->wtc_sfreq, (void *) &line[24]); 
		mb_put_binary_short(swap, (short) wc->wtc_heave, (void *) &line[28]);
		mb_put_binary_short(swap, (unsigned short) wc->wtc_spare1, (void *) &line[30]);
		mb_put_binary_short(swap, (unsigned short) wc->wtc_spare2, (void *) &line[32]);
		mb_put_binary_short(swap, (unsigned short) wc->wtc_spare3, (void *) &line[34]);
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_WC_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_WC_HEADER_SIZE,mbfp);
		if (write_len != EM3_WC_HEADER_SIZE)
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

	/* output binary beam data */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<wc->wtc_ntx;i++)
		{
		mb_put_binary_short(swap, (short) wc->wtc_txtiltangle[i], (void *) &line[0]);
		mb_put_binary_short(swap, (unsigned short) wc->wtc_txcenter[i], (void *) &line[2]);
		line[4] = (mb_u_char) wc->wtc_txsector[i];
		line[5] = (mb_u_char) 0;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_WC_TX_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_WC_TX_SIZE,mbfp);
		if (write_len != EM3_WC_TX_SIZE)
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
	    for (i=0;i<wc->wtc_nbeam;i++)
		{
		mb_put_binary_short(swap, (short) wc->beam[i].wtc_rxpointangle, (void *) &line[0]);
		mb_put_binary_short(swap, (unsigned short) wc->beam[i].wtc_start_sample, (void *) &line[2]);
		mb_put_binary_short(swap, (unsigned short) wc->beam[i].wtc_beam_samples, (void *) &line[4]);
		mb_put_binary_short(swap, (unsigned short) wc->beam[i].wtc_beam_spare, (void *) &line[6]);
		line[8] = (mb_u_char) wc->beam[i].wtc_sector;
		line[9] = (mb_u_char) wc->beam[i].wtc_beam;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM3_WC_BEAM_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM3_WC_BEAM_SIZE,mbfp);
		if (write_len != EM3_WC_BEAM_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) wc->beam[i].wtc_amp;
		for (j=0;j<wc->beam[i].wtc_beam_samples;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(wc->beam[i].wtc_amp,1,wc->beam[i].wtc_beam_samples,mbfp);
		if (write_len != wc->beam[i].wtc_beam_samples)
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
	    }

	/* output end of record */
	if (status == MB_SUCCESS)
		{
		if (pad == 1)
			{
			line[0] = 0;
			checksum += line[0];
			}
		line[1] = 0x03;
	    
		/* set checksum */
		mb_put_binary_short(swap, (unsigned short) checksum, (void *) &line[2]);

		/* write out data */
		write_len = fwrite(&line[!pad],1,3+pad,mbfp);
		if (write_len != 3+pad)
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
