/*--------------------------------------------------------------------
 *    The MB-system:	mbr_em300raw.c	10/16/98
 *	$Id: mbr_em300raw.c,v 5.4 2001-05-30 17:57:26 caress Exp $
 *
 *    Copyright (c) 1998, 2000 by
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
 * mbr_em300raw.c contains the functions for reading and writing
 * multibeam data in the EM300RAW format.  
 * These functions include:
 *   mbr_alm_em300raw	- allocate read/write memory
 *   mbr_dem_em300raw	- deallocate read/write memory
 *   mbr_rt_em300raw	- read and translate data
 *   mbr_wt_em300raw	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	October 16,  1998
 * $Log: not supported by cvs2svn $
 * Revision 5.3  2001/05/24  23:18:07  caress
 * Fixed handling of Revelle EM120 data (first cut).
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
 * Revision 4.10  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.9  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.8  2000/09/19  23:13:26  caress
 * Applied fixes from Gordon Keith at AGSO.
 *
 * Revision 4.7  2000/07/20  20:24:59  caress
 * First cut at supporting both EM120 and EM1002.
 *
 * Revision 4.6  2000/07/17  23:36:24  caress
 * Added support for EM120.
 *
 * Revision 4.5  2000/02/07  22:59:47  caress
 * Fixed problem with depth_offset_multiplier
 *
 * Revision 4.4  1999/04/21  05:45:32  caress
 * Fixed handling of bad sidescan data.
 *
 * Revision 4.3  1999/04/07  20:38:24  caress
 * Fixes related to building under Linux.
 *
 * Revision 4.3  1999/04/03 07:36:16  caress
 * Fix bugs in byteswapped code.
 *
 * Revision 4.2  1999/02/04 23:52:54  caress
 * MB-System version 4.6beta7
 *
 * Revision 4.1  1999/01/01  23:41:06  caress
 * MB-System version 4.6beta6
 *
 * Revision 4.0  1998/12/17  22:59:14  caress
 * MB-System version 4.6beta4
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
#include "../../include/mbsys_simrad2.h"

/* include for byte swapping */
#include "../../include/mb_swap.h"
	
/* turn on debug statements here */
/* #define MBR_EM300RAW_DEBUG 1 */

/* essential function prototypes */
int mbr_register_em300raw(int verbose, char *mbio_ptr, 
		int *error);
int mbr_info_em300raw(int verbose, 
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
int mbr_alm_em300raw(int verbose, char *mbio_ptr, int *error);
int mbr_dem_em300raw(int verbose, char *mbio_ptr, int *error);
int mbr_rt_em300raw(int verbose, char *mbio_ptr, char *store_ptr, int *error);
int mbr_wt_em300raw(int verbose, char *mbio_ptr, char *store_ptr, int *error);
int mbr_em300raw_rd_data(int verbose, char *mbio_ptr, char *store_ptr, int *error);
int mbr_em300raw_chk_label(int verbose, char *mbio_ptr, short type, short sonar);
int mbr_em300raw_rd_start(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short type, short sonar, int *version, int *error);
int mbr_em300raw_rd_run_parameter(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error);
int mbr_em300raw_rd_clock(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error);
int mbr_em300raw_rd_tide(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error);
int mbr_em300raw_rd_height(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error);
int mbr_em300raw_rd_heading(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error);
int mbr_em300raw_rd_ssv(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error);
int mbr_em300raw_rd_attitude(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error);
int mbr_em300raw_rd_pos(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error);
int mbr_em300raw_rd_svp(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error);
int mbr_em300raw_rd_svp2(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error);
int mbr_em300raw_rd_bath(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		int *match, short sonar, int version, int *error);
int mbr_em300raw_rd_rawbeam(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error);
int mbr_em300raw_rd_ss(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *match, int *error);
int mbr_em300raw_wr_data(int verbose, char *mbio_ptr, char *store_ptr, int *error);
int mbr_em300raw_wr_start(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error);
int mbr_em300raw_wr_run_parameter(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error);
int mbr_em300raw_wr_clock(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error);
int mbr_em300raw_wr_tide(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error);
int mbr_em300raw_wr_height(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error);
int mbr_em300raw_wr_heading(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error);
int mbr_em300raw_wr_ssv(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error);
int mbr_em300raw_wr_attitude(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error);
int mbr_em300raw_wr_pos(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error);
int mbr_em300raw_wr_svp(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error);
int mbr_em300raw_wr_bath(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error);
int mbr_em300raw_wr_rawbeam(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error);
int mbr_em300raw_wr_ss(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error);

/*--------------------------------------------------------------------*/
int mbr_register_em300raw(int verbose, char *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_em300raw.c,v 5.4 2001-05-30 17:57:26 caress Exp $";
	char	*function_name = "mbr_register_em300raw";
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
	status = mbr_info_em300raw(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_em300raw;
	mb_io_ptr->mb_io_format_free = &mbr_dem_em300raw; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_simrad2_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_simrad2_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_em300raw; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_em300raw; 
	mb_io_ptr->mb_io_extract = &mbsys_simrad2_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_simrad2_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_simrad2_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_simrad2_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_simrad2_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = &mbsys_simrad2_extract_svp; 
	mb_io_ptr->mb_io_insert_svp = &mbsys_simrad2_insert_svp; 
	mb_io_ptr->mb_io_ttimes = &mbsys_simrad2_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_simrad2_copy; 
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
int mbr_info_em300raw(int verbose, 
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
	static char res_id[]="$Id: mbr_em300raw.c,v 5.4 2001-05-30 17:57:26 caress Exp $";
	char	*function_name = "mbr_info_em300raw";
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
	*system = MB_SYS_SIMRAD2;
	*beams_bath_max = 254;
	*beams_amp_max = 254;
	*pixels_ss_max = 1024;
	strncpy(format_name, "EM300RAW", MB_NAME_LENGTH);
	strncpy(system_name, "SIMRAD2", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_EM300RAW\nInformal Description: Simrad current multibeam vendor format\nAttributes:           Simrad EM120, EM300, EM1002, EM3000, \n                      bathymetry, amplitude, and sidescan,\n                      up to 254 beams, variable pixels, ascii + binary, Simrad.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_NO;
	*nav_source = MB_DATA_NAV;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_ATTITUDE;
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
int mbr_alm_em300raw(int verbose, char *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_em300raw.c,v 5.4 2001-05-30 17:57:26 caress Exp $";
	char	*function_name = "mbr_alm_em300raw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	*wrapper;
	double	*pixel_size;
	double	*swath_width;

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
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	status = mbsys_simrad2_alloc(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);

	/* initialize everything to zeros */
	wrapper = (int *) &mb_io_ptr->save5;
	pixel_size = &mb_io_ptr->saved1;
	swath_width = &mb_io_ptr->saved2;
	*wrapper = -1;
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
int mbr_dem_em300raw(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_em300raw";
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
	status = mbsys_simrad2_deall(
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
int mbr_zero_ss_em300raw(int verbose, char *store_ptr, int *error)
{
	char	*function_name = "mbr_zero_ss_em300raw";
	int	status = MB_SUCCESS;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_ping_struct *ping;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to data descriptor */
	store = (struct mbsys_simrad2_struct *) store_ptr;
	if (store != NULL)
	    ping = (struct mbsys_simrad2_ping_struct *) store->ping;

	/* initialize all sidescan stuff to zeros */
	if (store->ping != NULL)
		{
		ping->png_ss_date = 0;	
				/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
		ping->png_ss_msec = 0;	
				/* time since midnight in msec
				    08:12:51.234 = 29570234 */
		ping->png_max_range = 0;  
				/* max range of ping in number of samples */
		ping->png_r_zero = 0;	
				/* range to normal incidence used in TVG
				    (R0 predicted) in samples */
		ping->png_r_zero_corr = 0;
				/* range to normal incidence used to correct
				    sample amplitudes in number of samples */
		ping->png_tvg_start = 0;	
				/* start sample of TVG ramp if not enough 
				    dynamic range (0 otherwise) */
		ping->png_tvg_stop = 0;	\
				/* stop sample of TVG ramp if not enough 
				    dynamic range (0 otherwise) */
		ping->png_bsn = 0;	
				/* normal incidence backscatter (BSN) in dB */
		ping->png_bso = 0;	
				/* oblique incidence backscatter (BSO) in dB */
		ping->png_tx = 0;	
				/* Tx beamwidth in 0.1 degree */
		ping->png_tvg_crossover = 0;	
				/* TVG law crossover angle in degrees */
		ping->png_nbeams_ss = 0;	
				/* number of beams with sidescan */
		ping->png_npixels = 0;	
				/* number of pixels of sidescan */
		for (i=0;i<MBSYS_SIMRAD2_MAXBEAMS;i++)
		    {
		    ping->png_beam_index[i] = 0;	
				/* beam index number */
		    ping->png_sort_direction[i] = 0;	
				/* sorting direction - first sample in beam has lowest
				    range if 1, highest if -1. */
		    ping->png_beam_samples[i] = 0;	
				/* number of sidescan samples derived from
					each beam */
		    ping->png_start_sample[i] = 0;	
				/* start sample number */
		    ping->png_center_sample[i] = 0;	
				/* center sample number */
		    }
		for (i=0;i<MBSYS_SIMRAD2_MAXRAWPIXELS;i++)
		    {
		    ping->png_ssraw[i] = EM2_INVALID_AMP;
				/* the sidescan ordered port to starboard */
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
int mbr_rt_em300raw(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_em300raw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_attitude_struct *attitude;
	struct mbsys_simrad2_heading_struct *heading;
	struct mbsys_simrad2_ssv_struct *ssv;
	struct mbsys_simrad2_ping_struct *ping;
	int	time_i[7];
	double	bath_time_d, ss_time_d;
	double	dd, dt, dx, dy;
	double	plon, plat, pspeed;
	double	mtodeglon, mtodeglat;
	double	headingx, headingy;
	double	*pixel_size, *swath_width;
	int	ifix;
	int	first, last;
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

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* read next data from file */
	status = mbr_em300raw_rd_data(verbose,mbio_ptr,store_ptr,error);

	/* get pointers to data structures */
	store = (struct mbsys_simrad2_struct *) store_ptr;
	attitude = (struct mbsys_simrad2_attitude_struct *) store->attitude;
	heading = (struct mbsys_simrad2_heading_struct *) store->heading;
	ssv = (struct mbsys_simrad2_ssv_struct *) store->ssv;
	ping = (struct mbsys_simrad2_ping_struct *) store->ping;
	pixel_size = (double *) &mb_io_ptr->saved1;
	swath_width = (double *) &mb_io_ptr->saved2;
	
	/* check that bath and sidescan data record time stamps
	   match for survey data - we can have bath without
	   sidescan but not sidescan without bath */
	if (status == MB_SUCCESS 
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
		
		/* check for time match - if bath newer than
		   sidescan then zero sidescan,  if sidescan
		   newer than bath then set error,  if ok then
		   check that beam ids are the same */
		if (ping->png_ss_date == 0
			|| ping->png_nbeams_ss == 0
			|| bath_time_d > ss_time_d)
		    {
		    status = mbr_zero_ss_em300raw(verbose,store_ptr,error);
		    }
		else if (bath_time_d < ss_time_d)
		    {
		    *error = MB_ERROR_UNINTELLIGIBLE;
		    status = MB_FAILURE;
		    }
		else
		    {
		    /* check for some indicators of broken records */
		    if (ping->png_nbeams != ping->png_nbeams_ss)
			    {
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    status = MB_FAILURE;
			    }
		    else
			    {
			    for (i=0;i<ping->png_nbeams;i++)
				{
				if (ping->png_beam_num[i] != 
					ping->png_beam_index[i] + 1
				    && ping->png_beam_num[i] != 
					ping->png_beam_index[i] - 1)
				    {
				    *error = MB_ERROR_UNINTELLIGIBLE;
				    status = MB_FAILURE;
				    }
				}
			    }
		    }
		}

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

	/* translate  values to temporary arrays 
		in mbio descriptor structure 
		to facillitate interpolating navigation
		and sidescan calculation */
	if (status == MB_SUCCESS)
		{
		/* get time */
		if (store->kind == MB_DATA_DATA)
			{
			mb_io_ptr->new_time_i[0] = ping->png_date / 10000;
			mb_io_ptr->new_time_i[1] = (ping->png_date % 10000) / 100;
			mb_io_ptr->new_time_i[2] = ping->png_date % 100;
			mb_io_ptr->new_time_i[3] = ping->png_msec / 3600000;
			mb_io_ptr->new_time_i[4] = (ping->png_msec % 3600000) / 60000;
			mb_io_ptr->new_time_i[5] = (ping->png_msec % 60000) / 1000;
			mb_io_ptr->new_time_i[6] = (ping->png_msec % 1000) * 1000;
			}
		else if (store->kind == MB_DATA_COMMENT
			|| store->kind == MB_DATA_START
			|| store->kind == MB_DATA_STOP)
			{
			mb_io_ptr->new_time_i[0] = store->par_date / 10000;
			mb_io_ptr->new_time_i[1] = (store->par_date % 10000) / 100;
			mb_io_ptr->new_time_i[2] = store->par_date % 100;
			mb_io_ptr->new_time_i[3] = store->par_msec / 3600000;
			mb_io_ptr->new_time_i[4] = (store->par_msec % 3600000) / 60000;
			mb_io_ptr->new_time_i[5] = (store->par_msec % 60000) / 1000;
			mb_io_ptr->new_time_i[6] = (store->par_msec % 1000) * 1000;
			}
		else if (store->kind == MB_DATA_VELOCITY_PROFILE)
			{
			mb_io_ptr->new_time_i[0] = store->svp_use_date / 10000;
			mb_io_ptr->new_time_i[1] = (store->svp_use_date % 10000) / 100;
			mb_io_ptr->new_time_i[2] = store->svp_use_date % 100;
			mb_io_ptr->new_time_i[3] = store->svp_use_msec / 3600000;
			mb_io_ptr->new_time_i[4] = (store->svp_use_msec % 3600000) / 60000;
			mb_io_ptr->new_time_i[5] = (store->svp_use_msec % 60000) / 1000;
			mb_io_ptr->new_time_i[6] = (store->svp_use_msec % 1000) * 1000;
			}
		else if (store->kind == MB_DATA_NAV)
			{
			mb_io_ptr->new_time_i[0] = store->pos_date / 10000;
			mb_io_ptr->new_time_i[1] = (store->pos_date % 10000) / 100;
			mb_io_ptr->new_time_i[2] = store->pos_date % 100;
			mb_io_ptr->new_time_i[3] = store->pos_msec / 3600000;
			mb_io_ptr->new_time_i[4] = (store->pos_msec % 3600000) / 60000;
			mb_io_ptr->new_time_i[5] = (store->pos_msec % 60000) / 1000;
			mb_io_ptr->new_time_i[6] = (store->pos_msec % 1000) * 1000;
			}
		else if (store->kind == MB_DATA_ATTITUDE)
			{
			mb_io_ptr->new_time_i[0] = attitude->att_date / 10000;
			mb_io_ptr->new_time_i[1] = (attitude->att_date % 10000) / 100;
			mb_io_ptr->new_time_i[2] = attitude->att_date % 100;
			mb_io_ptr->new_time_i[3] = attitude->att_msec / 3600000;
			mb_io_ptr->new_time_i[4] = (attitude->att_msec % 3600000) / 60000;
			mb_io_ptr->new_time_i[5] = (attitude->att_msec % 60000) / 1000;
			mb_io_ptr->new_time_i[6] = (attitude->att_msec % 1000) * 1000;
			}
		else if (store->kind == MB_DATA_SSV)
			{
			mb_io_ptr->new_time_i[0] = ssv->ssv_date / 10000;
			mb_io_ptr->new_time_i[1] = (ssv->ssv_date % 10000) / 100;
			mb_io_ptr->new_time_i[2] = ssv->ssv_date % 100;
			mb_io_ptr->new_time_i[3] = ssv->ssv_msec / 3600000;
			mb_io_ptr->new_time_i[4] = (ssv->ssv_msec % 3600000) / 60000;
			mb_io_ptr->new_time_i[5] = (ssv->ssv_msec % 60000) / 1000;
			mb_io_ptr->new_time_i[6] = (ssv->ssv_msec % 1000) * 1000;
			}
		else if (store->kind == MB_DATA_RUN_PARAMETER)
			{
			if (store->run_date != 0)
			    {
			    mb_io_ptr->new_time_i[0] = store->run_date / 10000;
			    mb_io_ptr->new_time_i[1] = (store->run_date % 10000) / 100;
			    mb_io_ptr->new_time_i[2] = store->run_date % 100;
			    mb_io_ptr->new_time_i[3] = store->run_msec / 3600000;
			    mb_io_ptr->new_time_i[4] = (store->run_msec % 3600000) / 60000;
			    mb_io_ptr->new_time_i[5] = (store->run_msec % 60000) / 1000;
			    mb_io_ptr->new_time_i[6] = (store->run_msec % 1000) * 1000;
			    }
			else
			    {
			    mb_io_ptr->new_time_i[0] = store->date / 10000;
			    mb_io_ptr->new_time_i[1] = (store->date % 10000) / 100;
			    mb_io_ptr->new_time_i[2] = store->date % 100;
			    mb_io_ptr->new_time_i[3] = store->msec / 3600000;
			    mb_io_ptr->new_time_i[4] = (store->msec % 3600000) / 60000;
			    mb_io_ptr->new_time_i[5] = (store->msec % 60000) / 1000;
			    mb_io_ptr->new_time_i[6] = (store->msec % 1000) * 1000;
			    }
			}
		if (mb_io_ptr->new_time_i[0] < 1970)
			mb_io_ptr->new_time_d = 0.0;
		else
			mb_get_time(verbose,mb_io_ptr->new_time_i,
				&mb_io_ptr->new_time_d);
				
		/* save fix if nav data */
		if (store->kind == MB_DATA_NAV
			&& store->pos_longitude != EM2_INVALID_INT
			&& store->pos_latitude != EM2_INVALID_INT)
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
				= 0.0000001 * store->pos_longitude;
			mb_io_ptr->fix_lat[mb_io_ptr->nfix] 
				= 0.00000005 * store->pos_latitude;
			mb_io_ptr->nfix++;
			}

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Nav fix added to list by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				mb_io_ptr->new_error);
			fprintf(stderr,"dbg4       kind:       %d\n",
				mb_io_ptr->new_kind);
			fprintf(stderr,"dbg4       nfix:       %d\n",
				mb_io_ptr->nfix);
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
				mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1]);
			fprintf(stderr,"dbg4       fix_lon:    %f\n",
				mb_io_ptr->fix_lon[mb_io_ptr->nfix-1]);
			fprintf(stderr,"dbg4       fix_lat:    %f\n",
				mb_io_ptr->fix_lat[mb_io_ptr->nfix-1]);
			}
		}

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

		/* interpolate from saved nav if possible */
		if (mb_io_ptr->nfix > 1)
			{
			/* get speed if necessary */
			if (store->pos_speed == 0
			    || store->pos_speed == EM2_INVALID_SHORT)
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
			    }
			else
			    {
			    pspeed = 3.6 * store->pos_speed;
			    }
			if (pspeed > 100.0)
			    pspeed = 0.0;

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
			    plon = mb_io_ptr->fix_lon[ifix]
				+ (mb_io_ptr->fix_lon[ifix+1] 
				    - mb_io_ptr->fix_lon[ifix])
				* (mb_io_ptr->new_time_d
				    - mb_io_ptr->fix_time_d[ifix])
				/ (mb_io_ptr->fix_time_d[ifix+1]
				    - mb_io_ptr->fix_time_d[ifix]);
			    plat = mb_io_ptr->fix_lat[ifix]
				+ (mb_io_ptr->fix_lat[ifix+1] 
				    - mb_io_ptr->fix_lat[ifix])
				* (mb_io_ptr->new_time_d
				    - mb_io_ptr->fix_time_d[ifix])
				/ (mb_io_ptr->fix_time_d[ifix+1]
				    - mb_io_ptr->fix_time_d[ifix]);
			    }
			
			/* extrapolate from first fix */
			else if (mb_io_ptr->new_time_d 
				< mb_io_ptr->fix_time_d[0]
				&& pspeed > 0.0)
			    {
			    dd = (mb_io_ptr->new_time_d 
				- mb_io_ptr->fix_time_d[0])
				* pspeed / 3.6;
			    mb_coor_scale(verbose,mb_io_ptr->fix_lat[0],
				&mtodeglon,&mtodeglat);
			    headingx = sin(DTR*(0.01 * ping->png_heading));
			    headingy = cos(DTR*(0.01 * ping->png_heading));
			    plon = mb_io_ptr->fix_lon[0] 
				+ headingx*mtodeglon*dd;
			    plat = mb_io_ptr->fix_lat[0] 
				+ headingy*mtodeglat*dd;
			    }
			
			/* extrapolate from last fix */
			else if (mb_io_ptr->new_time_d 
				> mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1]
				&& pspeed > 0.0)
			    {
			    dd = (mb_io_ptr->new_time_d 
				- mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
				* pspeed / 3.6;
			    mb_coor_scale(verbose,mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
				&mtodeglon,&mtodeglat);
			    headingx = sin(DTR*(0.01 * ping->png_heading));
			    headingy = cos(DTR*(0.01 * ping->png_heading));
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
			&& store->pos_speed > 0
			&& store->pos_speed != EM2_INVALID_SHORT)
			{
			dd = (mb_io_ptr->new_time_d - mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
				* 0.01 * store->pos_speed;
			mb_coor_scale(verbose,mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
				&mtodeglon,&mtodeglat);
			headingx = sin(DTR*(0.01 * ping->png_heading));
			headingy = cos(DTR*(0.01 * ping->png_heading));
			plon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1] 
				+ headingx*mtodeglon*dd;
			plat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1] 
				+ headingy*mtodeglat*dd;
			pspeed = 3.6 * store->pos_speed;
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

		if (plon == 0.0
		    && plat == 0.0)
		    {
		    ping->png_longitude = (int) EM2_INVALID_INT;
		    ping->png_latitude = (int) EM2_INVALID_INT;
		    }
		else
		    {
		    ping->png_longitude = (int) (10000000 * plon);
		    ping->png_latitude = (int) (20000000 * plat);
		    }
		ping->png_speed = (int) (pspeed / 0.036);

		/* generate processed sidescan */
		ping->png_pixel_size = 0;
		ping->png_pixels_ss = 0;
		status = mbsys_simrad2_makess(verbose,
				mbio_ptr, store_ptr,
				MB_NO, pixel_size, 
				MB_NO, swath_width, 
				0, 
				error);
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
int mbr_wt_em300raw(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_em300raw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
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
	store = (struct mbsys_simrad2_struct *) store_ptr;

	/* write next data to file */
	status = mbr_em300raw_wr_data(verbose,mbio_ptr,store_ptr,error);

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
int mbr_em300raw_rd_data(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_em300raw_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_heading_struct *heading;
	struct mbsys_simrad2_attitude_struct *attitude;
	struct mbsys_simrad2_ssv_struct *ssv;
	struct mbsys_simrad2_ping_struct *ping;
	FILE	*mbfp;
	int	done;
	int	*wrapper;
	int	*record_size;
	int	record_size_save;
	char	*label;
	int	*label_save_flag;
	short	expect;
	short	*type;
	short	*sonar;
	int	*version;
	short	first_type;
	short	*expect_save;
	int	*expect_save_flag;
	short	*first_type_save;
	short	*typelast;
	int	match;
	int	read_len;
	int	skip = 0;
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
	store = (struct mbsys_simrad2_struct *) store_ptr;
	ping = (struct mbsys_simrad2_ping_struct *) store->ping;
	mbfp = mb_io_ptr->mbfp;
	
	/* get saved values */
	wrapper = (int *) &mb_io_ptr->save5;
	label = (char *) mb_io_ptr->save_label;
	type = (short *) mb_io_ptr->save_label;
	sonar = (short *) (&mb_io_ptr->save_label[2]);
	record_size = (int *) mb_io_ptr->save_label;
	version = (int *) (&mb_io_ptr->save3);
	label_save_flag = (int *) &mb_io_ptr->save_label_flag;
	expect_save_flag = (int *) &mb_io_ptr->save_flag;
	expect_save = (short *) &mb_io_ptr->save1;
	first_type_save = (short *) &mb_io_ptr->save2;
	typelast = (short *) &mb_io_ptr->save6;
	if (*expect_save_flag == MB_YES)
		{
		expect = *expect_save;
		first_type = *first_type_save;
		*expect_save_flag = MB_NO;
		}
	else
		{
		expect = EM2_NONE;
		first_type = EM2_NONE;
		if (ping != NULL)
		    {
		    ping->png_raw_read = MB_NO;
		    ping->png_ss_read = MB_NO;
		    ping->png_nrawbeams = 0;
		    ping->png_nbeams_ss = 0;
		    }
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
			/* read four byte wrapper if data stream is known
				to have wrappers */
			if (*wrapper == MB_YES)
				{
				if ((read_len = fread(label,
					1,4,mb_io_ptr->mbfp)) != 4)
					{
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					}
				record_size_save = *record_size;
				}
				
			/* look for label */
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
				&& mbr_em300raw_chk_label(verbose, 
					mbio_ptr, *type, *sonar) != MB_SUCCESS)
			    {
			    /* get next byte */
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
			if (skip > 0 && !(skip == 4 || *wrapper < 0))
			    {
			    fprintf(stderr, 
"\nThe MBSYS_SIMRAD2 module skipped %d bytes between\n\
identified data records %d:%x and %d:%x \n\
Something is broken...\n\
We recommend you send a data sample and problem \n\
description to the MB-System team \n\
(caress@mbari.org and dale@ldeo.columbia.edu)\n\
Have a nice day....\n", 
skip, *typelast, *typelast, *type, *type);
			    }
			*typelast = *type;

			/* set wrapper status if needed */
			if (*wrapper < 0)
			    {
			    if (skip == 0) 
					*wrapper = MB_NO;
			    else if (skip == 4)
					*wrapper = MB_YES;
			    }
			}
		
		/* else use saved label */
		else
			*label_save_flag = MB_NO;

		/* swap bytes if necessary */
#ifdef BYTESWAPPED
		*type = (short) mb_swap_short(*type);
		*sonar = (short) mb_swap_short(*sonar);
#endif

#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"\nstart of mbr_em300raw_rd_data loop:\n");
	fprintf(stderr,"skip:%d expect:%x type:%x first_type:%x sonar:%d recsize:%d done:%d\n",
		skip, expect, *type, first_type, *sonar, record_size_save, done);
#endif
		
		/* allocate secondary data structure for
			heading data if needed */
		if (status == MB_SUCCESS && 
			(*type == EM2_HEADING)
			&& store->heading == NULL)
			{
			status = mbsys_simrad2_heading_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}
		
		/* allocate secondary data structure for
			attitude data if needed */
		if (status == MB_SUCCESS && 
			(*type == EM2_ATTITUDE)
			&& store->attitude == NULL)
			{
			status = mbsys_simrad2_attitude_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}
		
		/* allocate secondary data structure for
			ssv data if needed */
		if (status == MB_SUCCESS && 
			(*type == EM2_SSV)
			&& store->ssv == NULL)
			{
			status = mbsys_simrad2_ssv_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}
		
		/* allocate secondary data structure for
			survey data if needed */
		if (status == MB_SUCCESS && 
			(*type == EM2_BATH
			|| *type == EM2_RAWBEAM
			|| *type == EM2_SS))
			{
			if (store->ping == NULL)
			    status = mbsys_simrad2_survey_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			ping = (struct mbsys_simrad2_ping_struct *) store->ping;
			}

		/* read the appropriate data records */
		if (status == MB_FAILURE && expect == EM2_NONE)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call nothing, read failure, no expect\n");
#endif
			done = MB_YES;
			}
		else if (status == MB_FAILURE && expect != EM2_NONE)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call nothing, read failure, expect %x\n",expect);
#endif
			done = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else if (*type != EM2_START
			&& *type != EM2_STOP
			&& *type != EM2_STOP2
			&& *type != EM2_OFF
			&& *type != EM2_ON
			&& *type != EM2_RUN_PARAMETER
			&& *type != EM2_CLOCK
			&& *type != EM2_TIDE
			&& *type != EM2_HEIGHT
			&& *type != EM2_HEADING
			&& *type != EM2_SSV
			&& *type != EM2_ATTITUDE
			&& *type != EM2_POS
			&& *type != EM2_SVP2
			&& *type != EM2_SVP
			&& *type != EM2_BATH
			&& *type != EM2_RAWBEAM
			&& *type != EM2_SS)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call nothing, try again\n");
#endif
			done = MB_NO;
			}
		else if (*type == EM2_START
			|| *type == EM2_STOP
			|| *type == EM2_STOP2
			|| *type == EM2_OFF
			|| *type == EM2_ON)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_rd_start type %x\n",*type);
#endif
			status = mbr_em300raw_rd_start(
				verbose,mbfp,store,*type,*sonar,version,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_RUN_PARAMETER)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_rd_run_parameter type %x\n",*type);
#endif
			status = mbr_em300raw_rd_run_parameter(
				verbose,mbfp,store,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_CLOCK)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_rd_clock type %x\n",*type);
#endif
			status = mbr_em300raw_rd_clock(
				verbose,mbfp,store,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_TIDE)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_rd_tide type %x\n",*type);
#endif
			status = mbr_em300raw_rd_tide(
				verbose,mbfp,store,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_HEIGHT)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_rd_height type %x\n",*type);
#endif
			status = mbr_em300raw_rd_height(
				verbose,mbfp,store,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_HEADING)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_rd_heading type %x\n",*type);
#endif
			status = mbr_em300raw_rd_heading(
				verbose,mbfp,store,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_SSV)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_rd_ssv type %x\n",*type);
#endif
			status = mbr_em300raw_rd_ssv(
				verbose,mbfp,store,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_ATTITUDE)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_rd_attitude type %x\n",*type);
#endif
			status = mbr_em300raw_rd_attitude(
				verbose,mbfp,store,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}	
		else if (*type == EM2_POS)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_rd_pos type %x\n",*type);
#endif
			status = mbr_em300raw_rd_pos(
				verbose,mbfp,store,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_SVP)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_rd_svp type %x\n",*type);
#endif
			status = mbr_em300raw_rd_svp(
				verbose,mbfp,store,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_SVP2)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_rd_svp2 type %x\n",*type);
#endif
			status = mbr_em300raw_rd_svp2(
				verbose,mbfp,store,*sonar,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				if (expect != EM2_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM2_BATH 
			&& expect == EM2_SS)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			done = MB_YES;
			expect = EM2_NONE;
			*type = first_type;
			*label_save_flag = MB_YES;
			store->kind = MB_DATA_DATA;
			}
		else if (*type == EM2_BATH)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_rd_bath type %x\n",*type);
#endif
			status = mbr_em300raw_rd_bath(
				verbose,mbfp,store,&match,*sonar,*version,error);
			if (status == MB_SUCCESS)
				{
				if (first_type == EM2_NONE
					|| match == MB_NO)
					{
					done = MB_NO;
					first_type = EM2_BATH;
					expect = EM2_SS;
					}
				else
					{
					done = MB_YES;
					expect = EM2_NONE;
					}
				}
			}
		else if (*type == EM2_RAWBEAM)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_rd_rawbeam type %x\n",*type);
#endif
			status = mbr_em300raw_rd_rawbeam(
				verbose,mbfp,store,*sonar,error);
			if (status == MB_SUCCESS)
				ping->png_raw_read = MB_YES;
			}
		else if (*type == EM2_SS 
			&& expect != EM2_NONE 
			&& expect != EM2_SS)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			done = MB_YES;
			expect = EM2_NONE;
			*type = first_type;
			*label_save_flag = MB_YES;
			store->kind = MB_DATA_DATA;
			}
		else if (*type == EM2_SS)
			{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_rd_ss type %x\n",*type);
#endif
			status = mbr_em300raw_rd_ss(
				verbose,mbfp,store,*sonar,&match,error);
			if (status == MB_SUCCESS)
				{
				ping->png_ss_read = MB_YES;
				if (first_type == EM2_NONE
					|| match == MB_NO)
					{
					done = MB_NO;
					first_type = EM2_SS;
					expect = EM2_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM2_NONE;
					}
				}
			}

		/* bail out if there is an error */
		if (status == MB_FAILURE)
			done = MB_YES;

#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"done:%d expect:%x status:%d error:%d\n", 
		done, expect, status, *error);
	fprintf(stderr,"end of mbr_em300raw_rd_data loop:\n");
#endif
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
int mbr_em300raw_chk_label(int verbose, char *mbio_ptr, short type, short sonar)
{
	char	*function_name = "mbr_em300raw_chk_label";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	char	*startid;
	short	*sonar_save;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       type:       %d\n",type);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	sonar_save = (short *) (&mb_io_ptr->save4);

		/* swap bytes if necessary */
#ifdef BYTESWAPPED
		type = (short) mb_swap_short(type);
		sonar = (short) mb_swap_short(sonar);
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2  Input values byte swapped:\n");
		fprintf(stderr,"dbg2       type:       %d\n",type);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
#endif

	/* check for valid label */
	if (type != EM2_START
		&& type != EM2_STOP
		&& type != EM2_STOP2
		&& type != EM2_OFF
		&& type != EM2_ON
		&& type != EM2_RUN_PARAMETER
		&& type != EM2_CLOCK
		&& type != EM2_TIDE
		&& type != EM2_HEIGHT
		&& type != EM2_HEADING
		&& type != EM2_SSV
		&& type != EM2_ATTITUDE
		&& type != EM2_POS
		&& type != EM2_SVP
		&& type != EM2_SVP2
		&& type != EM2_BATH
		&& type != EM2_RAWBEAM
		&& type != EM2_SS)
		{
		status = MB_FAILURE;
		startid = (char *) &type;
		if ((verbose >= 1 && *startid == 2)
		    && (sonar == EM2_EM120
			|| sonar == EM2_EM300
			|| sonar == EM2_EM1002
			|| sonar == EM2_EM2000
			|| sonar == EM2_EM3000
			|| sonar == EM2_EM3000D_1
			|| sonar == EM2_EM3000D_2
			|| sonar == EM2_EM3000D_3
			|| sonar == EM2_EM3000D_4
			|| sonar == EM2_EM3000D_5
			|| sonar == EM2_EM3000D_6
			|| sonar == EM2_EM3000D_7))
			{
			fprintf(stderr, "Bad datagram type: %d %x   %d %x\n", type, type, sonar, sonar);
			}
		}
		
	/* check for valid sonar model */
	if (sonar != EM2_EM120
		&& sonar != EM2_EM300
		&& sonar != EM2_EM1002
		&& sonar != EM2_EM2000
		&& sonar != EM2_EM3000
		&& sonar != EM2_EM3000D_1
		&& sonar != EM2_EM3000D_2
		&& sonar != EM2_EM3000D_3
		&& sonar != EM2_EM3000D_4
		&& sonar != EM2_EM3000D_5
		&& sonar != EM2_EM3000D_6
		&& sonar != EM2_EM3000D_7)
		{
		status = MB_FAILURE;
		}
			
	/* save sonar if successful */
	if (status == MB_SUCCESS)
	    *sonar_save = sonar;
		
	/* allow exception found in some EM3000 data */
	if (type == EM2_SVP && sonar == 0 && *sonar_save == EM2_EM3000)
		{
		status = MB_SUCCESS;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_start(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short type, short sonar, int *version, int *error)
{
	char	*function_name = "mbr_em300raw_rd_start";
	int	status = MB_SUCCESS;
	char	line[MBSYS_SIMRAD2_BUFFER_SIZE];
	short	*short_ptr;
	int	*int_ptr;
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
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       type:       %d\n",type);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* make sure comment is initialized */
	store->par_com[0] = '\0';
	
	/* set type value */
	store->type = type;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM2_START_HEADER_SIZE,mbfp);
	if (read_len == EM2_START_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		store->par_date = (int) mb_swap_int(*int_ptr);
		store->date = store->par_date;
		int_ptr = (int *) &line[4];
		store->par_msec = (int) mb_swap_int(*int_ptr);
		store->msec = store->par_msec;
		short_ptr = (short *) &line[8];
		store->par_line_num = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		store->par_serial_1 = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[12];
		store->par_serial_2 = (unsigned short) mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		store->par_date = (int) *int_ptr;
		store->date = store->par_date;
		int_ptr = (int *) &line[4];
		store->par_msec = (int) *int_ptr;
		store->msec = store->par_msec;
		short_ptr = (short *) &line[8];
		store->par_line_num = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		store->par_serial_1 = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[12];
		store->par_serial_2 = (unsigned short) *short_ptr;
#endif
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
			&& (line[len-1] < 32
			    || line[len-1] > 127)
			&& line[len-1] != '\r'
			&& line[len-1] != '\n')
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
			    strncpy(store->par_rop, &line[4], MIN(len-5, MBSYS_SIMRAD2_COMMENT_LENGTH-1));
			else if (strncmp("SID=", line, 4) == 0)
			    strncpy(store->par_sid, &line[4], MIN(len-5, MBSYS_SIMRAD2_COMMENT_LENGTH-1));
			else if (strncmp("PLL=", line, 4) == 0)
			    strncpy(store->par_pll, &line[4], MIN(len-5, MBSYS_SIMRAD2_COMMENT_LENGTH-1));
			else if (strncmp("COM=", line, 4) == 0)
			    {
			    strncpy(store->par_com, &line[4], MIN(len-5, MBSYS_SIMRAD2_COMMENT_LENGTH-1));
			    store->par_com[MIN(len-5, MBSYS_SIMRAD2_COMMENT_LENGTH-1)] = 0;
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
		else if (store->type == EM2_START)
		    store->kind = MB_DATA_START;
		else if (store->type == EM2_STOP)
		    store->kind = MB_DATA_STOP;
		else if (store->type == EM2_STOP2)
		    store->kind = MB_DATA_STOP;
		else if (store->type == EM2_OFF)
		    store->kind = MB_DATA_STOP;
		else if (store->type == EM2_ON)
		    store->kind = MB_DATA_START;
		}
		
	/* read end of record and last two check sum bytes */
	if (status == MB_SUCCESS)
	    {
	    /* if EM2_END not yet found then the 
		next byte should be EM2_END */
/*fprintf(stderr, "endbyte1: %d:%x:%c\n", line[0], line[0], line[0]);*/
	    if (line[0] != EM2_END)
		{
		read_len = fread(&line[0],1,1,mbfp);
/*fprintf(stderr, "endbyte2: %d:%x:%c\n", line[0], line[0], line[0]);*/
		}
		
	    /* if EM2_END not yet found then the 
		next byte should be EM2_END */
	    if (line[0] != EM2_END)
		{
		read_len = fread(&line[0],1,1,mbfp);
/*fprintf(stderr, "endbyte3: %d:%x:%c\n", line[0], line[0], line[0]);*/
		}
		
	    /* if we got the end byte then get check sum bytes */
	    if (line[0] == EM2_END)
		read_len = fread(&line[0],2,1,mbfp);
	    /* don't check success of read
	        - return success here even if read fails
	        because all of the
		important information in this record has
		already been read - next attempt to read
		file will return error */
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
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_run_parameter(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error)
{
	char	*function_name = "mbr_em300raw_rd_run_parameter";
	int	status = MB_SUCCESS;
	char	line[EM2_RUN_PARAMETER_SIZE];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	store->kind = MB_DATA_RUN_PARAMETER;
	store->type = EM2_RUN_PARAMETER;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM2_RUN_PARAMETER_SIZE-4,mbfp);
	if (read_len == EM2_RUN_PARAMETER_SIZE-4)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		store->run_date = (int) mb_swap_int(*int_ptr);
		if (store->run_date != 0) store->date = store->run_date;
		int_ptr = (int *) &line[4];
		store->run_msec = (int) mb_swap_int(*int_ptr);
		if (store->run_date != 0) store->msec = store->run_msec;
		short_ptr = (short *) &line[8];
		store->run_ping_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		store->run_serial = (unsigned short) mb_swap_short(*short_ptr);
		int_ptr = (int *) &line[12];
		store->run_status = (int) mb_swap_int(*int_ptr);
		store->run_mode = (mb_u_char) line[16];
		store->run_filter_id = (mb_u_char) line[17];
		short_ptr = (short *) &line[18];
		store->run_min_depth = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[20];
		store->run_max_depth = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[22];
		store->run_absorption = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[24];
		store->run_tran_pulse = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[26];
		store->run_tran_beam = (unsigned short) mb_swap_short(*short_ptr);
		store->run_tran_pow = (mb_u_char) line[28];
		store->run_rec_beam = (mb_u_char) line[29];
		store->run_rec_band = (mb_u_char) line[30];
		store->run_rec_gain = (mb_u_char) line[31];
		store->run_tvg_cross = (mb_u_char) line[32];
		store->run_ssv_source = (mb_u_char) line[33];
		short_ptr = (short *) &line[34];
		store->run_max_swath = (unsigned short) mb_swap_short(*short_ptr);
		store->run_beam_space = (mb_u_char) line[36];
		store->run_swath_angle = (mb_u_char) line[37];
		store->run_stab_mode = (mb_u_char) line[38];
		for (i=0;i<6;i++)
		    store->run_spare[i] = line[39+i];
#else
		int_ptr = (int *) &line[0];
		store->run_date = (int) *int_ptr;
		if (store->run_date != 0) store->date = store->run_date;
		int_ptr = (int *) &line[4];
		store->run_msec = (int) *int_ptr;
		if (store->run_date != 0) store->msec = store->run_msec;
		short_ptr = (short *) &line[8];
		store->run_ping_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		store->run_serial = (unsigned short) *short_ptr;
		int_ptr = (int *) &line[12];
		store->run_status = (int) *int_ptr;
		store->run_mode = (mb_u_char) line[16];
		store->run_filter_id = (mb_u_char) line[17];
		short_ptr = (short *) &line[18];
		store->run_min_depth = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[20];
		store->run_max_depth = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[22];
		store->run_absorption = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[24];
		store->run_tran_pulse = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[26];
		store->run_tran_beam = (unsigned short) *short_ptr;
		store->run_tran_pow = (mb_u_char) line[28];
		store->run_rec_beam = (mb_u_char) line[29];
		store->run_rec_band = (mb_u_char) line[30];
		store->run_rec_gain = (mb_u_char) line[31];
		store->run_tvg_cross = (mb_u_char) line[32];
		store->run_ssv_source = (mb_u_char) line[33];
		short_ptr = (short *) &line[34];
		store->run_max_swath = (unsigned short) *short_ptr;
		store->run_beam_space = (mb_u_char) line[36];
		store->run_swath_angle = (mb_u_char) line[37];
		store->run_stab_mode = (mb_u_char) line[38];
		for (i=0;i<6;i++)
		    store->run_spare[i] = line[39+i];
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
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_clock(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error)
{
	char	*function_name = "mbr_em300raw_rd_clock";
	int	status = MB_SUCCESS;
	char	line[EM2_CLOCK_SIZE];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	store->kind = MB_DATA_CLOCK;
	store->type = EM2_CLOCK;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM2_CLOCK_SIZE-4,mbfp);
	if (read_len == EM2_CLOCK_SIZE-4)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		store->clk_date = (int) mb_swap_int(*int_ptr);
		store->date = store->clk_date;
		int_ptr = (int *) &line[4];
		store->clk_msec = (int) mb_swap_int(*int_ptr);
		store->msec = store->clk_msec;
		short_ptr = (short *) &line[8];
		store->clk_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		store->clk_serial = (unsigned short) mb_swap_short(*short_ptr);
		int_ptr = (int *) &line[12];
		store->clk_origin_date = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[16];
		store->clk_origin_msec = (int) mb_swap_int(*int_ptr);
		store->clk_1_pps_use = (mb_u_char) line[20];
#else
		int_ptr = (int *) &line[0];
		store->clk_date = (int) *int_ptr;
		store->date = store->clk_date;
		int_ptr = (int *) &line[4];
		store->clk_msec = (int) *int_ptr;
		store->msec = store->clk_msec;
		short_ptr = (short *) &line[8];
		store->clk_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		store->clk_serial = (unsigned short) *short_ptr;
		int_ptr = (int *) &line[12];
		store->clk_origin_date = (int) *int_ptr;
		int_ptr = (int *) &line[16];
		store->clk_origin_msec = (int) *int_ptr;
		store->clk_1_pps_use = (mb_u_char) line[20];
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
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_tide(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error)
{
	char	*function_name = "mbr_em300raw_rd_tide";
	int	status = MB_SUCCESS;
	char	line[EM2_TIDE_SIZE];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	store->kind = MB_DATA_TIDE;
	store->type = EM2_TIDE;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM2_TIDE_SIZE-4,mbfp);
	if (read_len == EM2_TIDE_SIZE-4)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		store->tid_date = (int) mb_swap_int(*int_ptr);
		store->date = store->tid_date;
		int_ptr = (int *) &line[4];
		store->tid_msec = (int) mb_swap_int(*int_ptr);
		store->msec = store->tid_msec;
		short_ptr = (short *) &line[8];
		store->tid_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		store->tid_serial = (unsigned short) mb_swap_short(*short_ptr);
		int_ptr = (int *) &line[12];
		store->tid_origin_date = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[16];
		store->tid_origin_msec = (int) mb_swap_int(*int_ptr);
		short_ptr = (short *) &line[20];
		store->tid_tide = mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		store->tid_date = (int) *int_ptr;
		store->date = store->tid_date;
		int_ptr = (int *) &line[4];
		store->tid_msec = (int) *int_ptr;
		store->msec = store->tid_msec;
		short_ptr = (short *) &line[8];
		store->tid_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		store->tid_serial = (unsigned short) *short_ptr;
		int_ptr = (int *) &line[12];
		store->tid_origin_date = (int) *int_ptr;
		int_ptr = (int *) &line[16];
		store->tid_origin_msec = (int) *int_ptr;
		short_ptr = (short *) &line[20];
		store->tid_tide = *short_ptr;
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
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_height(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error)
{
	char	*function_name = "mbr_em300raw_rd_height";
	int	status = MB_SUCCESS;
	char	line[EM2_HEIGHT_SIZE];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	store->kind = MB_DATA_HEIGHT;
	store->type = EM2_HEIGHT;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = fread(line,1,EM2_HEIGHT_SIZE-4,mbfp);
	if (read_len == EM2_HEIGHT_SIZE-4)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		store->hgt_date = (int) mb_swap_int(*int_ptr);
		store->date = store->hgt_date;
		int_ptr = (int *) &line[4];
		store->hgt_msec = (int) mb_swap_int(*int_ptr);
		store->msec = store->hgt_msec;
		short_ptr = (short *) &line[8];
		store->hgt_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		store->hgt_serial = (unsigned short) mb_swap_short(*short_ptr);
		int_ptr = (int *) &line[12];
		store->hgt_height = (int) mb_swap_int(*int_ptr);
		store->hgt_type = (mb_u_char) line[16];
#else
		int_ptr = (int *) &line[0];
		store->hgt_date = (int) *int_ptr;
		store->date = store->hgt_date;
		int_ptr = (int *) &line[4];
		store->hgt_msec = (int) *int_ptr;
		store->msec = store->hgt_msec;
		short_ptr = (short *) &line[8];
		store->hgt_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		store->hgt_serial = (unsigned short) *short_ptr;
		int_ptr = (int *) &line[12];
		store->hgt_height = (int) *int_ptr;
		store->hgt_type = (mb_u_char) line[16];
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
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_heading(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error)
{
	char	*function_name = "mbr_em300raw_rd_heading";
	int	status = MB_SUCCESS;
	struct mbsys_simrad2_heading_struct *heading;
	char	line[16];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* get  storage structure */
	heading = (struct mbsys_simrad2_heading_struct *) store->heading;
		
	/* set kind and type values */
	store->kind = MB_DATA_HEADING;
	store->type = EM2_HEADING;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM2_HEADING_HEADER_SIZE,mbfp);
	if (read_len == EM2_HEADING_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		heading->hed_date = (int) mb_swap_int(*int_ptr);
		store->date = heading->hed_date;
		int_ptr = (int *) &line[4];
		heading->hed_msec = (int) mb_swap_int(*int_ptr);
		store->msec = heading->hed_msec;
		short_ptr = (short *) &line[8];
		heading->hed_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		heading->hed_serial = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[12];
		heading->hed_ndata = (int) mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		heading->hed_date = (int) *int_ptr;
		store->date = heading->hed_date;
		int_ptr = (int *) &line[4];
		heading->hed_msec = (int) *int_ptr;
		store->msec = heading->hed_msec;
		short_ptr = (short *) &line[8];
		heading->hed_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		heading->hed_serial = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[12];
		heading->hed_ndata = (int) *short_ptr;
#endif
		}

	/* read binary heading values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<heading->hed_ndata && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM2_HEADING_SLICE_SIZE,mbfp);
		if (read_len == EM2_HEADING_SLICE_SIZE 
			&& i < MBSYS_SIMRAD2_MAXHEADING)
			{
			status = MB_SUCCESS;
#ifdef BYTESWAPPED
			short_ptr = (short *) &line[0];
			heading->hed_time[i] = (unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[2];
			heading->hed_heading[i] = (unsigned short) mb_swap_short(*short_ptr);
#else
			short_ptr = (short *) &line[0];
			heading->hed_time[i] = (unsigned short) *short_ptr;
			short_ptr = (short *) &line[2];
			heading->hed_heading[i] = (unsigned short) *short_ptr;
#endif
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	    heading->hed_ndata = MIN(heading->hed_ndata, MBSYS_SIMRAD2_MAXHEADING);
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
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_ssv(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error)
{
	char	*function_name = "mbr_em300raw_rd_ssv";
	int	status = MB_SUCCESS;
	struct mbsys_simrad2_ssv_struct *ssv;
	char	line[16];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* get  storage structure */
	ssv = (struct mbsys_simrad2_ssv_struct *) store->ssv;
	
	/* set kind and type values */
	store->kind = MB_DATA_SSV;
	store->type = EM2_SSV;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM2_SSV_HEADER_SIZE,mbfp);
	if (read_len == EM2_SSV_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		ssv->ssv_date = (int) mb_swap_int(*int_ptr);
		store->date = ssv->ssv_date;
		int_ptr = (int *) &line[4];
		ssv->ssv_msec = (int) mb_swap_int(*int_ptr);
		store->msec = ssv->ssv_msec;
		short_ptr = (short *) &line[8];
		ssv->ssv_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		ssv->ssv_serial = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[12];
		ssv->ssv_ndata = (int) mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		ssv->ssv_date = (int) *int_ptr;
		store->date = ssv->ssv_date;
		int_ptr = (int *) &line[4];
		ssv->ssv_msec = (int) *int_ptr;
		store->msec = ssv->ssv_msec;
		short_ptr = (short *) &line[8];
		ssv->ssv_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		ssv->ssv_serial = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[12];
		ssv->ssv_ndata = (int) *short_ptr;
#endif
		}

	/* read binary heading values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<ssv->ssv_ndata && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM2_SSV_SLICE_SIZE,mbfp);
		if (read_len == EM2_SSV_SLICE_SIZE 
			&& i < MBSYS_SIMRAD2_MAXSSV)
			{
			status = MB_SUCCESS;
#ifdef BYTESWAPPED
			short_ptr = (short *) &line[0];
			ssv->ssv_time[i] = (unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[2];
			ssv->ssv_ssv[i] = (unsigned short) mb_swap_short(*short_ptr);
#else
			short_ptr = (short *) &line[0];
			ssv->ssv_time[i] = (unsigned short) *short_ptr;
			short_ptr = (short *) &line[2];
			ssv->ssv_ssv[i] = (unsigned short) *short_ptr;
#endif
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	    ssv->ssv_ndata = MIN(ssv->ssv_ndata, MBSYS_SIMRAD2_MAXSSV);
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
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_attitude(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error)
{
	char	*function_name = "mbr_em300raw_rd_attitude";
	int	status = MB_SUCCESS;
	struct mbsys_simrad2_attitude_struct *attitude;
	char	line[16];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* get  storage structure */
	attitude = (struct mbsys_simrad2_attitude_struct *) store->attitude;
		
	/* set kind and type values */
	store->kind = MB_DATA_ATTITUDE;
	store->type = EM2_ATTITUDE;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM2_ATTITUDE_HEADER_SIZE,mbfp);
	if (read_len == EM2_ATTITUDE_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		attitude->att_date = (int) mb_swap_int(*int_ptr);
		store->date = attitude->att_date;
		int_ptr = (int *) &line[4];
		attitude->att_msec = (int) mb_swap_int(*int_ptr);
		store->msec = attitude->att_msec;
		short_ptr = (short *) &line[8];
		attitude->att_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		attitude->att_serial = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[12];
		attitude->att_ndata = (int) mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		attitude->att_date = (int) *int_ptr;
		store->date = attitude->att_date;
		int_ptr = (int *) &line[4];
		attitude->att_msec = (int) *int_ptr;
		store->msec = attitude->att_msec;
		short_ptr = (short *) &line[8];
		attitude->att_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		attitude->att_serial = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[12];
		attitude->att_ndata = (int) *short_ptr;
#endif
		}

	/* read binary attitude values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<attitude->att_ndata && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM2_ATTITUDE_SLICE_SIZE,mbfp);
		if (read_len == EM2_ATTITUDE_SLICE_SIZE 
			&& i < MBSYS_SIMRAD2_MAXATTITUDE)
			{
			status = MB_SUCCESS;
#ifdef BYTESWAPPED
			short_ptr = (short *) &line[0];
			attitude->att_time[i] = (unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[2];
			attitude->att_sensor_status[i] = (unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[4];
			attitude->att_roll[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[6];
			attitude->att_pitch[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[8];
			attitude->att_heave[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[10];
			attitude->att_heading[i] = (unsigned short) mb_swap_short(*short_ptr);
#else
			short_ptr = (short *) &line[0];
			attitude->att_time[i] = (unsigned short) *short_ptr;
			short_ptr = (short *) &line[2];
			attitude->att_sensor_status[i] = (unsigned short) *short_ptr;
			short_ptr = (short *) &line[4];
			attitude->att_roll[i] = (short) *short_ptr;
			short_ptr = (short *) &line[6];
			attitude->att_pitch[i] = (short) *short_ptr;
			short_ptr = (short *) &line[8];
			attitude->att_heave[i] = (short) *short_ptr;
			short_ptr = (short *) &line[10];
			attitude->att_heading[i] = (unsigned short) *short_ptr;
#endif
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	    attitude->att_ndata = MIN(attitude->att_ndata, MBSYS_SIMRAD2_MAXATTITUDE);
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
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_pos(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error)
{
	char	*function_name = "mbr_em300raw_rd_pos";
	int	status = MB_SUCCESS;
	char	line[256];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	store->kind = MB_DATA_NAV;
	store->type = EM2_POS;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM2_POS_HEADER_SIZE,mbfp);
	if (read_len == EM2_POS_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		store->pos_date = (int) mb_swap_int(*int_ptr);
		store->date = store->pos_date;
		int_ptr = (int *) &line[4];
		store->pos_msec = (int) mb_swap_int(*int_ptr);
		store->msec = store->pos_msec;
		short_ptr = (short *) &line[8];
		store->pos_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		store->pos_serial = (unsigned short) mb_swap_short(*short_ptr);
		int_ptr = (int *) &line[12];
		store->pos_latitude = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[16];
		store->pos_longitude = (int) mb_swap_int(*int_ptr);
		short_ptr = (short *) &line[20];
		store->pos_quality = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[22];
		store->pos_speed = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[24];
		store->pos_course = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[26];
		store->pos_heading = (unsigned short) mb_swap_short(*short_ptr);
		store->pos_system = (mb_u_char) line[28];
		store->pos_input_size = (mb_u_char) line[29];
#else
		int_ptr = (int *) &line[0];
		store->pos_date = (int) *int_ptr;
		store->date = store->pos_date;
		int_ptr = (int *) &line[4];
		store->pos_msec = (int) *int_ptr;
		store->msec = store->pos_msec;
		short_ptr = (short *) &line[8];
		store->pos_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		store->pos_serial = (unsigned short) *short_ptr;
		int_ptr = (int *) &line[12];
		store->pos_latitude = (int) *int_ptr;
		int_ptr = (int *) &line[16];
		store->pos_longitude = (int) *int_ptr;
		short_ptr = (short *) &line[20];
		store->pos_quality = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[22];
		store->pos_speed = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[24];
		store->pos_course = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[26];
		store->pos_heading = (unsigned short) *short_ptr;
		store->pos_system = (mb_u_char) line[28];
		store->pos_input_size = (mb_u_char) line[29];
#endif
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
		if (read_len == 1 && line[0] == EM2_END)
			{
			done = MB_YES;
			status = MB_SUCCESS;
			/* get last two check sum bytes */
			read_len = fread(&line[0],2,1,mbfp);
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
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_svp(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error)
{
	char	*function_name = "mbr_em300raw_rd_svp";
	int	status = MB_SUCCESS;
	char	line[256];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	store->kind = MB_DATA_VELOCITY_PROFILE;
	store->type = EM2_SVP;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM2_SVP_HEADER_SIZE,mbfp);
	if (read_len == EM2_SVP_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		store->svp_use_date = (int) mb_swap_int(*int_ptr);
		store->date = store->svp_use_date;
		int_ptr = (int *) &line[4];
		store->svp_use_msec = (int) mb_swap_int(*int_ptr);
		store->msec = store->svp_use_msec;
		short_ptr = (short *) &line[8];
		store->svp_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		store->svp_serial = (unsigned short) mb_swap_short(*short_ptr);
		int_ptr = (int *) &line[12];
		store->svp_origin_date = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[16];
		store->svp_origin_msec = (int) mb_swap_int(*int_ptr);
		short_ptr = (short *) &line[20];
		store->svp_num = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[22];
		store->svp_depth_res = (unsigned short) mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		store->svp_use_date = (int) *int_ptr;
		store->date = store->svp_use_date;
		int_ptr = (int *) &line[4];
		store->svp_use_msec = (int) *int_ptr;
		store->msec = store->svp_use_msec;
		short_ptr = (short *) &line[8];
		store->svp_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		store->svp_serial = (unsigned short) *short_ptr;
		int_ptr = (int *) &line[12];
		store->svp_origin_date = (int) *int_ptr;
		int_ptr = (int *) &line[16];
		store->svp_origin_msec = (int) *int_ptr;
		short_ptr = (short *) &line[20];
		store->svp_num = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[22];
		store->svp_depth_res = (unsigned short) *short_ptr;
#endif
		}

	/* read binary svp values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<store->svp_num && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM2_SVP_SLICE_SIZE,mbfp);
		if (read_len != EM2_SVP_SLICE_SIZE)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		else if (i < MBSYS_SIMRAD2_MAXSVP)
			{
			status = MB_SUCCESS;
#ifdef BYTESWAPPED
			short_ptr = (short *) &line[0];
			store->svp_depth[i] = (unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[2];
			store->svp_vel[i] = (unsigned short) mb_swap_short(*short_ptr);
#else
			short_ptr = (short *) &line[0];
			store->svp_depth[i] = (unsigned short) *short_ptr;
			short_ptr = (short *) &line[2];
			store->svp_vel[i] = (unsigned short) *short_ptr;
#endif
			}
		}
	    store->svp_num = MIN(store->svp_num, MBSYS_SIMRAD2_MAXSVP);
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
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_svp2(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error)
{
	char	*function_name = "mbr_em300raw_rd_svp2";
	int	status = MB_SUCCESS;
	char	line[256];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* set kind and type values */
	store->kind = MB_DATA_VELOCITY_PROFILE;
	store->type = EM2_SVP2;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM2_SVP_HEADER_SIZE,mbfp);
	if (read_len == EM2_SVP_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		store->svp_use_date = (int) mb_swap_int(*int_ptr);
		store->date = store->svp_use_date;
		int_ptr = (int *) &line[4];
		store->svp_use_msec = (int) mb_swap_int(*int_ptr);
		store->msec = store->svp_use_msec;
		short_ptr = (short *) &line[8];
		store->svp_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		store->svp_serial = (unsigned short) mb_swap_short(*short_ptr);
		int_ptr = (int *) &line[12];
		store->svp_origin_date = (int) mb_swap_int(*int_ptr);
		int_ptr = (int *) &line[16];
		store->svp_origin_msec = (int) mb_swap_int(*int_ptr);
		short_ptr = (short *) &line[20];
		store->svp_num = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[22];
		store->svp_depth_res = (unsigned short) mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		store->svp_use_date = (int) *int_ptr;
		store->date = store->svp_use_date;
		int_ptr = (int *) &line[4];
		store->svp_use_msec = (int) *int_ptr;
		store->msec = store->svp_use_msec;
		short_ptr = (short *) &line[8];
		store->svp_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		store->svp_serial = (unsigned short) *short_ptr;
		int_ptr = (int *) &line[12];
		store->svp_origin_date = (int) *int_ptr;
		int_ptr = (int *) &line[16];
		store->svp_origin_msec = (int) *int_ptr;
		short_ptr = (short *) &line[20];
		store->svp_num = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[22];
		store->svp_depth_res = (unsigned short) *short_ptr;
#endif
		}

	/* read binary svp values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<store->svp_num && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM2_SVP2_SLICE_SIZE,mbfp);
		if (read_len != EM2_SVP2_SLICE_SIZE)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		else if (i < MBSYS_SIMRAD2_MAXSVP)
			{
			status = MB_SUCCESS;
#ifdef BYTESWAPPED
			int_ptr = (int *) &line[0];
			store->svp_depth[i] = (int) mb_swap_int(*int_ptr);
			int_ptr = (int *) &line[4];
			store->svp_vel[i] = (int) mb_swap_int(*int_ptr);
#else
			int_ptr = (int *) &line[0];
			store->svp_depth[i] = (int) *int_ptr;
			int_ptr = (int *) &line[4];
			store->svp_vel[i] = (int) *int_ptr;
#endif
			}
		}
	    store->svp_num = MIN(store->svp_num, MBSYS_SIMRAD2_MAXSVP);
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
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_bath(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		int *match, short sonar, int version, int *error)
{
	char	*function_name = "mbr_em300raw_rd_bath";
	int	status = MB_SUCCESS;
	struct mbsys_simrad2_ping_struct *ping;
	char	line[EM2_BATH_HEADER_SIZE];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		fprintf(stderr,"dbg2       version:    %d\n",version);
		}
		
	/* get  storage structure */
	ping = (struct mbsys_simrad2_ping_struct *) store->ping;
		
	/* set kind and type values */
	store->kind = MB_DATA_DATA;
	store->type = EM2_BATH;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM2_BATH_HEADER_SIZE,mbfp);
	if (read_len == EM2_BATH_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		ping->png_date = (int) mb_swap_int(*int_ptr);
		store->date = ping->png_date;
		int_ptr = (int *) &line[4];
		ping->png_msec = (int) mb_swap_int(*int_ptr);
		store->msec = ping->png_msec;
		short_ptr = (short *) &line[8];
		ping->png_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		ping->png_serial = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[12];
		ping->png_heading = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[14];
		ping->png_ssv = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[16];
		ping->png_xducer_depth = (unsigned short) mb_swap_short(*short_ptr);
		ping->png_nbeams_max = (mb_u_char) line[18];
		ping->png_nbeams = (mb_u_char) line[19];
		ping->png_depth_res = (mb_u_char) line[20];
		ping->png_distance_res = (mb_u_char) line[21];
		short_ptr = (short *) &line[22];
		ping->png_sample_rate = (unsigned short) mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		ping->png_date = (int) *int_ptr;
		store->date = ping->png_date;
		int_ptr = (int *) &line[4];
		ping->png_msec = (int) *int_ptr;
		store->msec = ping->png_msec;
		short_ptr = (short *) &line[8];
		ping->png_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		ping->png_serial = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[12];
		ping->png_heading = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[14];
		ping->png_ssv = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[16];
		ping->png_xducer_depth = (unsigned short) *short_ptr;
		ping->png_nbeams_max = (mb_u_char) line[18];
		ping->png_nbeams = (mb_u_char) line[19];
		ping->png_depth_res = (mb_u_char) line[20];
		ping->png_distance_res = (mb_u_char) line[21];
		short_ptr = (short *) &line[22];
		ping->png_sample_rate = (unsigned short) *short_ptr;
#endif
		}
		
	/* check for some indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (ping->png_nbeams > ping->png_nbeams_max
			|| ping->png_nbeams < 0
			|| ping->png_nbeams_max < 0
			|| ping->png_nbeams > MBSYS_SIMRAD2_MAXBEAMS
			|| ping->png_nbeams_max > MBSYS_SIMRAD2_MAXBEAMS)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read binary beam values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<ping->png_nbeams && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM2_BATH_BEAM_SIZE,mbfp);
		if (read_len == EM2_BATH_BEAM_SIZE 
			&& i < MBSYS_SIMRAD2_MAXBEAMS)
			{
			status = MB_SUCCESS;
#ifdef BYTESWAPPED
			short_ptr = (short *) &line[0];
			if (store->sonar == EM2_EM120
				|| store->sonar == EM2_EM300)
			    ping->png_depth[i] = (unsigned short) mb_swap_short(*short_ptr);
			else
			    ping->png_depth[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[2];
			ping->png_acrosstrack[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[4];
			ping->png_alongtrack[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[6];
			ping->png_depression[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[8];
			ping->png_azimuth[i] = (unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[10];
			ping->png_range[i] = (unsigned short) mb_swap_short(*short_ptr);
			ping->png_quality[i] = (mb_u_char) line[12];
			ping->png_window[i] = (mb_u_char) line[13];
			ping->png_amp[i] = (mb_s_char) line[14];
			ping->png_beam_num[i] = (mb_u_char) line[15];
			ping->png_beamflag[i] = MB_FLAG_NONE;
#else
			short_ptr = (short *) &line[0];
			if (store->sonar == EM2_EM120
				|| store->sonar == EM2_EM300)
			    ping->png_depth[i] = (unsigned short) *short_ptr;
			else
			    ping->png_depth[i] = (short) *short_ptr;
			short_ptr = (short *) &line[2];
			ping->png_acrosstrack[i] = (short) *short_ptr;
			short_ptr = (short *) &line[4];
			ping->png_alongtrack[i] = (short) *short_ptr;
			short_ptr = (short *) &line[6];
			ping->png_depression[i] = (short) *short_ptr;
			short_ptr = (short *) &line[8];
			ping->png_azimuth[i] = (unsigned short) *short_ptr;
			short_ptr = (short *) &line[10];
			ping->png_range[i] = (unsigned short) *short_ptr;
			ping->png_quality[i] = (mb_u_char) line[12];
			ping->png_window[i] = (mb_u_char) line[13];
			ping->png_amp[i] = (mb_s_char) line[14];
			ping->png_beam_num[i] = (mb_u_char) line[15];
			ping->png_beamflag[i] = MB_FLAG_NONE;
#endif
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
			ping->png_offset_multiplier = (mb_s_char) line[0];
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
		
	/* check sonar version and adjust data as necessary */
	if (status == MB_SUCCESS 
		&& sonar >= MBSYS_SIMRAD2_EM3000
		&& version != 0
		&& version < 20000 )
		{
		ping->png_offset_multiplier = 0;
		}
		
	/* check for some other indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (ping->png_nbeams > 0
		    && ping->png_beam_num[0] > ping->png_nbeams_max)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		for (i=1;i<ping->png_nbeams;i++)
			{
			if (ping->png_beam_num[i] < ping->png_beam_num[i-1]
				|| ping->png_beam_num[i] > ping->png_nbeams_max)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				}
			}
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
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       png_date:        %d\n",ping->png_date);
		fprintf(stderr,"dbg5       png_msec:        %d\n",ping->png_msec);
		fprintf(stderr,"dbg5       png_count:       %d\n",ping->png_count);
		fprintf(stderr,"dbg5       png_serial:      %d\n",ping->png_serial);
		fprintf(stderr,"dbg5       png_heading:     %d\n",ping->png_heading);
		fprintf(stderr,"dbg5       png_ssv:         %d\n",ping->png_ssv);
		fprintf(stderr,"dbg5       png_xducer_depth:      %d\n",ping->png_xducer_depth);
		fprintf(stderr,"dbg5       png_offset_multiplier: %d\n",ping->png_offset_multiplier);
		fprintf(stderr,"dbg5       png_nbeams_max:        %d\n",ping->png_nbeams_max);
		fprintf(stderr,"dbg5       png_nbeams:            %d\n",ping->png_nbeams);
		fprintf(stderr,"dbg5       png_depth_res:         %d\n",ping->png_depth_res);
		fprintf(stderr,"dbg5       png_distance_res:      %d\n",ping->png_distance_res);
		fprintf(stderr,"dbg5       png_sample_rate:       %d\n",ping->png_sample_rate);
		fprintf(stderr,"dbg5       cnt  depth xtrack ltrack dprsn   azi   rng  qual wnd amp num\n");
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		for (i=0;i<ping->png_nbeams;i++)
			fprintf(stderr,"dbg5       %3d %6d %6d %6d %5d %5d %5d %4d %3d %3d %3d\n",
				i, ping->png_depth[i], ping->png_acrosstrack[i], 
				ping->png_alongtrack[i], ping->png_depression[i], 
				ping->png_azimuth[i], ping->png_range[i], 
				ping->png_quality[i], ping->png_window[i], 
				ping->png_amp[i], ping->png_beam_num[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       match:      %d\n",*match);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_rawbeam(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *error)
{
	char	*function_name = "mbr_em300raw_rd_rawbeam";
	int	status = MB_SUCCESS;
	struct mbsys_simrad2_ping_struct *ping;
	char	line[EM2_BATH_HEADER_SIZE];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* get  storage structure */
	ping = (struct mbsys_simrad2_ping_struct *) store->ping;
		
	/* read binary header values into char array */
	read_len = fread(line,1,EM2_RAWBEAM_HEADER_SIZE,mbfp);
	if (read_len == EM2_RAWBEAM_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		ping->png_date = (int) mb_swap_int(*int_ptr);
		store->date = ping->png_date;
		int_ptr = (int *) &line[4];
		ping->png_msec = (int) mb_swap_int(*int_ptr);
		store->msec = ping->png_msec;
		short_ptr = (short *) &line[8];
		ping->png_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		ping->png_serial = (unsigned short) mb_swap_short(*short_ptr);
		ping->png_nbeams_max = (mb_u_char) line[12];
		ping->png_nrawbeams = (mb_u_char) line[13];
		short_ptr = (short *) &line[14];
		ping->png_ssv = (unsigned short) mb_swap_short(*short_ptr);
#else
		int_ptr = (int *) &line[0];
		ping->png_date = (int) *int_ptr;
		store->date = ping->png_date;
		int_ptr = (int *) &line[4];
		ping->png_msec = (int) *int_ptr;
		store->msec = ping->png_msec;
		short_ptr = (short *) &line[8];
		ping->png_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		ping->png_serial = (unsigned short) *short_ptr;
		ping->png_nbeams_max = (mb_u_char) line[12];
		ping->png_nrawbeams = (mb_u_char) line[13];
		short_ptr = (short *) &line[14];
		ping->png_ssv = (unsigned short) *short_ptr;
#endif
		}
		
	/* check for some indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (ping->png_nrawbeams > ping->png_nbeams_max
			|| ping->png_nrawbeams < 0
			|| ping->png_nbeams_max < 0
			|| ping->png_nrawbeams > MBSYS_SIMRAD2_MAXBEAMS
			|| ping->png_nbeams_max > MBSYS_SIMRAD2_MAXBEAMS)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

	/* read binary beam values */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<ping->png_nrawbeams && status == MB_SUCCESS;i++)
		{
		read_len = fread(line,1,EM2_RAWBEAM_BEAM_SIZE,mbfp);
		if (read_len == EM2_RAWBEAM_BEAM_SIZE 
			&& i < MBSYS_SIMRAD2_MAXBEAMS)
			{
			status = MB_SUCCESS;
#ifdef BYTESWAPPED
			short_ptr = (short *) &line[0];
			ping->png_rawpointangle[i] = (short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[2];
			ping->png_rawtiltangle[i] = (unsigned short) mb_swap_short(*short_ptr);
			short_ptr = (short *) &line[4];
			ping->png_rawrange[i] = (unsigned short) mb_swap_short(*short_ptr);
			ping->png_rawamp[i] = (mb_s_char) line[6];
			ping->png_rawbeam_num[i] = (mb_u_char) line[7];
#else
			short_ptr = (short *) &line[0];
			ping->png_rawpointangle[i] = (short) *short_ptr;
			short_ptr = (short *) &line[2];
			ping->png_rawtiltangle[i] = (unsigned short) *short_ptr;
			short_ptr = (short *) &line[4];
			ping->png_rawrange[i] = (unsigned short) *short_ptr;
			ping->png_rawamp[i] = (mb_s_char) line[6];
			ping->png_rawbeam_num[i] = (mb_u_char) line[7];
#endif
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
		}
		
	/* check for some other indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (ping->png_nbeams > 0
		    && ping->png_rawbeam_num[0] > ping->png_nbeams_max)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		for (i=1;i<ping->png_nrawbeams;i++)
			{
			if (ping->png_rawbeam_num[i] < ping->png_rawbeam_num[i-1]
				|| ping->png_rawbeam_num[i] > ping->png_nbeams_max)
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
		fprintf(stderr,"dbg5       png_date:        %d\n",ping->png_date);
		fprintf(stderr,"dbg5       png_msec:        %d\n",ping->png_msec);
		fprintf(stderr,"dbg5       png_count:       %d\n",ping->png_count);
		fprintf(stderr,"dbg5       png_serial:      %d\n",ping->png_serial);
		fprintf(stderr,"dbg5       png_nbeams_max:  %d\n",ping->png_nbeams_max);
		fprintf(stderr,"dbg5       png_nrawbeams:   %d\n",ping->png_nrawbeams);
		fprintf(stderr,"dbg5       png_ssv:         %d\n",ping->png_ssv);
		fprintf(stderr,"dbg5       cnt  point   tilt   rng  amp num\n");
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		for (i=0;i<ping->png_nrawbeams;i++)
			fprintf(stderr,"dbg5       %3d %5d %5d %5d %3d %3d\n",
				i, ping->png_rawpointangle[i], ping->png_rawtiltangle[i], 
				ping->png_rawrange[i], ping->png_rawamp[i], 
				ping->png_rawbeam_num[i]);
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
int mbr_em300raw_rd_ss(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, 
		short sonar, int *match, int *error)
{
	char	*function_name = "mbr_em300raw_rd_ss";
	int	status = MB_SUCCESS;
	struct mbsys_simrad2_ping_struct *ping;
	char	line[30];
	short	*short_ptr;
	int	*int_ptr;
	int	read_len, len;
	int	done;
	int	junk_bytes;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       sonar:      %d\n",sonar);
		}
		
	/* get  storage structure */
	ping = (struct mbsys_simrad2_ping_struct *) store->ping;
		
	/* set kind and type values */
	store->kind = MB_DATA_DATA;
	store->type = EM2_SS;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = fread(line,1,EM2_SS_HEADER_SIZE,mbfp);
	if (read_len == EM2_SS_HEADER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get binary header data */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		ping->png_ss_date = (int) mb_swap_int(*int_ptr);
		store->date = ping->png_ss_date;
		int_ptr = (int *) &line[4];
		ping->png_ss_msec = (int) mb_swap_int(*int_ptr);
		store->msec = ping->png_ss_msec;
		short_ptr = (short *) &line[8];
		ping->png_count = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[10];
		ping->png_serial = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[12];
		ping->png_max_range = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[14];
		ping->png_r_zero = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[16];
		ping->png_r_zero_corr = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[18];
		ping->png_tvg_start = (unsigned short) mb_swap_short(*short_ptr);
		short_ptr = (short *) &line[20];
		ping->png_tvg_stop = (unsigned short) mb_swap_short(*short_ptr);
		ping->png_bsn = (mb_s_char) line[22];
		ping->png_bso = (mb_s_char) line[23];
		short_ptr = (short *) &line[24];
		ping->png_tx = (unsigned short) mb_swap_short(*short_ptr);
		ping->png_tvg_crossover = (mb_u_char) line[26];
		ping->png_nbeams_ss = (mb_u_char) line[27];
#else
		int_ptr = (int *) &line[0];
		ping->png_ss_date = (int) *int_ptr;
		store->date = ping->png_ss_date;
		int_ptr = (int *) &line[4];
		ping->png_ss_msec = (int) *int_ptr;
		store->msec = ping->png_ss_msec;
		short_ptr = (short *) &line[8];
		ping->png_count = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[10];
		ping->png_serial = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[12];
		ping->png_max_range = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[14];
		ping->png_r_zero = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[16];
		ping->png_r_zero_corr = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[18];
		ping->png_tvg_start = (unsigned short) *short_ptr;
		short_ptr = (short *) &line[20];
		ping->png_tvg_stop = (unsigned short) *short_ptr;
		ping->png_bsn = (mb_s_char) line[22];
		ping->png_bso = (mb_s_char) line[23];
		short_ptr = (short *) &line[24];
		ping->png_tx = (unsigned short) *short_ptr;
		ping->png_tvg_crossover = (mb_u_char) line[26];
		ping->png_nbeams_ss = (mb_u_char) line[27];
#endif
		}
		
	/* check for some indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (ping->png_nbeams_ss < 0
			|| ping->png_nbeams_ss > MBSYS_SIMRAD2_MAXBEAMS)
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
		read_len = fread(line,1,EM2_SS_BEAM_SIZE,mbfp);
		if (read_len == EM2_SS_BEAM_SIZE 
			&& i < MBSYS_SIMRAD2_MAXBEAMS)
			{
			status = MB_SUCCESS;
#ifdef BYTESWAPPED
			ping->png_beam_index[i] = (mb_u_char) line[0];
			ping->png_sort_direction[i] = (mb_s_char) line[1];
			short_ptr = (short *) &line[2];
			ping->png_beam_samples[i] = (unsigned short) mb_swap_short(*short_ptr);
			ping->png_start_sample[i] = ping->png_npixels;
			short_ptr = (short *) &line[4];
			ping->png_center_sample[i] = (unsigned short) mb_swap_short(*short_ptr);
#else
			ping->png_beam_index[i] = (mb_u_char) line[0];
			ping->png_sort_direction[i] = (mb_s_char) line[1];
			short_ptr = (short *) &line[2];
			ping->png_beam_samples[i] = (unsigned short) *short_ptr;
			ping->png_start_sample[i] = ping->png_npixels;
			short_ptr = (short *) &line[4];
			ping->png_center_sample[i] = (unsigned short) *short_ptr;
#endif
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		ping->png_npixels += ping->png_beam_samples[i];
		if (ping->png_npixels > MBSYS_SIMRAD2_MAXRAWPIXELS)
			{
			ping->png_beam_samples[i] 
				-= (ping->png_npixels 
					- MBSYS_SIMRAD2_MAXRAWPIXELS);
			if (ping->png_beam_samples[i] < 0)
				ping->png_beam_samples[i] = 0;
			}
		}
	    if (ping->png_npixels > MBSYS_SIMRAD2_MAXRAWPIXELS)
		{
		if (verbose > 0)
		    fprintf(stderr, "WARNING: EM300/3000 sidescan pixels %d exceed maximum %d!\n", 
			    ping->png_npixels, MBSYS_SIMRAD2_MAXRAWPIXELS);
		junk_bytes = ping->png_npixels - MBSYS_SIMRAD2_MAXRAWPIXELS;
		ping->png_npixels = MBSYS_SIMRAD2_MAXRAWPIXELS;
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}
	    else
		junk_bytes = 0;
	    }
		
	/* check for some other indicators of a broken record 
	    - these do happen!!!! */
	if (status == MB_SUCCESS)
		{
		if (ping->png_nbeams_ss > 0
		    && ping->png_beam_index[0] > MBSYS_SIMRAD2_MAXBEAMS)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		for (i=1;i<ping->png_nbeams_ss;i++)
			{
			if (ping->png_beam_index[i] < ping->png_beam_index[i-1]
				|| ping->png_beam_index[0] > MBSYS_SIMRAD2_MAXBEAMS)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				}
			}
		}

	/* read binary sidescan values */
	if (status == MB_SUCCESS)
		{
		read_len = fread(ping->png_ssraw,1,ping->png_npixels,mbfp);
		if (read_len == ping->png_npixels )
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
		if (read_len == 1 && line[0] == EM2_END)
			{
			done = MB_YES;
			status = MB_SUCCESS;
			/* get last two check sum bytes */
			read_len = fread(&line[0],2,1,mbfp);
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
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       png_date:        %d\n",ping->png_date);
		fprintf(stderr,"dbg5       png_msec:        %d\n",ping->png_msec);
		fprintf(stderr,"dbg5       png_ss_date:     %d\n",ping->png_ss_date);
		fprintf(stderr,"dbg5       png_ss_msec:     %d\n",ping->png_ss_msec);
		fprintf(stderr,"dbg5       png_count:       %d\n",ping->png_count);
		fprintf(stderr,"dbg5       png_serial:      %d\n",ping->png_serial);

		fprintf(stderr,"dbg5       png_heading:     %d\n",ping->png_heading);
		fprintf(stderr,"dbg5       png_ssv:         %d\n",ping->png_ssv);
		fprintf(stderr,"dbg5       png_xducer_depth:      %d\n",ping->png_xducer_depth);
		fprintf(stderr,"dbg5       png_offset_multiplier: %d\n",ping->png_offset_multiplier);
		fprintf(stderr,"dbg5       png_nbeams_max:        %d\n",ping->png_nbeams_max);
		fprintf(stderr,"dbg5       png_nbeams:            %d\n",ping->png_nbeams);
		fprintf(stderr,"dbg5       png_depth_res:         %d\n",ping->png_depth_res);
		fprintf(stderr,"dbg5       png_distance_res:      %d\n",ping->png_distance_res);
		fprintf(stderr,"dbg5       png_sample_rate:       %d\n",ping->png_sample_rate);
		fprintf(stderr,"dbg5       cnt  depth xtrack ltrack dprsn   azi   rng  qual wnd amp num\n");
		fprintf(stderr,"dbg5       ----------------------------------------------------------------\n");
		for (i=0;i<ping->png_nbeams;i++)
			fprintf(stderr,"dbg5       %3d %6d %6d %6d %5d %5d %5d %4d %3d %3d %3d\n",
				i, ping->png_depth[i], ping->png_acrosstrack[i], 
				ping->png_alongtrack[i], ping->png_depression[i], 
				ping->png_azimuth[i], ping->png_range[i], 
				ping->png_quality[i], ping->png_window[i], 
				ping->png_amp[i], ping->png_beam_num[i]);
		fprintf(stderr,"dbg5       png_max_range:   %d\n",ping->png_max_range);
		fprintf(stderr,"dbg5       png_r_zero:      %d\n",ping->png_r_zero);
		fprintf(stderr,"dbg5       png_r_zero_corr: %d\n",ping->png_r_zero_corr);
		fprintf(stderr,"dbg5       png_tvg_start:   %d\n",ping->png_tvg_start);
		fprintf(stderr,"dbg5       png_tvg_stop:    %d\n",ping->png_tvg_stop);
		fprintf(stderr,"dbg5       png_bsn:         %d\n",ping->png_bsn);
		fprintf(stderr,"dbg5       png_bso:         %d\n",ping->png_bso);
		fprintf(stderr,"dbg5       png_tx:          %d\n",ping->png_tx);
		fprintf(stderr,"dbg5       png_tvg_crossover: %d\n",ping->png_tvg_crossover);
		fprintf(stderr,"dbg5       png_nbeams_ss:     %d\n",ping->png_nbeams_ss);
		fprintf(stderr,"dbg5       png_npixels:       %d\n",ping->png_npixels);
		fprintf(stderr,"dbg5       cnt  index sort samples start center\n");
		fprintf(stderr,"dbg5       --------------------------------------------------\n");
		for (i=0;i<ping->png_nbeams_ss;i++)
			fprintf(stderr,"dbg5        %4d %3d %2d %4d %4d %4d\n",
				i, ping->png_beam_index[i], ping->png_sort_direction[i], 
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
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_wr_data(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_em300raw_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store;
	struct mbsys_simrad2_ping_struct *ping;
	FILE	*mbfp;

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
	store = (struct mbsys_simrad2_struct *) store_ptr;
	ping = (struct mbsys_simrad2_ping_struct *) store->ping;
	mbfp = mb_io_ptr->mbfp;

#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"\nstart of mbr_em300raw_wr_data:\n");
	fprintf(stderr,"kind:%d %d type:%x\n", store->kind, mb_io_ptr->new_kind, store->type);
#endif

	if (store->kind == MB_DATA_COMMENT
		|| store->kind == MB_DATA_START
		|| store->kind == MB_DATA_STOP)
		{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_wr_start kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em300raw_wr_start(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_RUN_PARAMETER)
		{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_wr_run_parameter kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em300raw_wr_run_parameter(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_CLOCK)
		{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_wr_clock kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em300raw_wr_clock(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_TIDE)
		{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_wr_tide kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em300raw_wr_tide(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_HEIGHT)
		{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_wr_height kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em300raw_wr_height(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_HEADING)
		{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_wr_heading kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em300raw_wr_heading(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_SSV)
		{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_wr_ssv kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em300raw_wr_ssv(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_ATTITUDE)
		{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_wr_attitude kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em300raw_wr_attitude(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_NAV)
		{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_wr_pos kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em300raw_wr_pos(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_VELOCITY_PROFILE)
		{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_wr_svp kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em300raw_wr_svp(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_DATA)
		{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_wr_bath kind:%d type %x\n",store->kind,store->type);
#endif
		status = mbr_em300raw_wr_bath(verbose,mbfp,store,error);
		if (ping->png_raw_read == MB_YES)
		    {
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_wr_rawbeam kind:%d type %x\n",store->kind,store->type);
#endif
		    status = mbr_em300raw_wr_rawbeam(verbose,mbfp,store,error);
		    }
#ifdef MBR_EM300RAW_DEBUG
	else fprintf(stderr,"NOT call mbr_em300raw_wr_rawbeam kind:%d type %x\n",store->kind,store->type);
#endif
		if (ping->png_ss_read == MB_YES)
		    {
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call mbr_em300raw_wr_ss kind:%d type %x\n",store->kind,store->type);
#endif
		    status = mbr_em300raw_wr_ss(verbose,mbfp,store,error);
		    }
#ifdef MBR_EM300RAW_DEBUG
	else fprintf(stderr,"NOT call mbr_em300raw_wr_ss kind:%d type %x\n",store->kind,store->type);
#endif
		}
	else
		{
#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"call nothing bad kind: %d type %x\n", store->kind, store->type);
#endif
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
		}

#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr,"status:%d error:%d\n", status, *error);
	fprintf(stderr,"end of mbr_em300raw_wr_data:\n");
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
int mbr_em300raw_wr_start(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error)
{
	char	*function_name = "mbr_em300raw_wr_start";
	int	status = MB_SUCCESS;
	char	line[MBSYS_SIMRAD2_BUFFER_SIZE], *buff;
	int	buff_len, write_len;
	short	*label;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	char	*comma_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
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
	if (store->type == EM2_NONE)
	    store->type = EM2_START;
	    
	/* if sonar not set use EM300 */
	if (store->sonar == 0)
	    store->sonar = EM2_EM300;
		
	/* set up start of output buffer - we handle this
	   record differently because of the ascii data */
	memset(line, 0, MBSYS_SIMRAD2_BUFFER_SIZE);
	label = (short *) &line[4];
	*label = store->type;
#ifdef BYTESWAPPED
	*label = (short) mb_swap_short(*label);
#endif

	/* put binary header data into buffer */
	if (status == MB_SUCCESS)
		{
#ifdef BYTESWAPPED
		short_ptr = (short *) &line[6];
		*short_ptr = (unsigned short) mb_swap_short(store->sonar);
		int_ptr = (int *) &line[8];
		*int_ptr = (int) mb_swap_int(store->par_date);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(store->par_msec);
		short_ptr = (short *) &line[16];
		*short_ptr = (unsigned short) mb_swap_short(store->par_line_num);
		short_ptr = (short *) &line[18];
		*short_ptr = (unsigned short) mb_swap_short(store->par_serial_1);
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) mb_swap_short(store->par_serial_2);
#else
		short_ptr = (short *) &line[6];
		*short_ptr = (unsigned short) store->sonar;
		int_ptr = (int *) &line[8];
		*int_ptr = (int) store->par_date;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) store->par_msec;
		short_ptr = (short *) &line[16];
		*short_ptr = (unsigned short) store->par_line_num;
		short_ptr = (short *) &line[18];
		*short_ptr = (unsigned short) store->par_serial_1;
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) store->par_serial_2;
#endif
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
	line[buff_len + 22] = EM2_END;
		
	/* get size of record */
	write_size = 25 + buff_len;
	int_ptr = (int *) &line[0];
	*int_ptr = write_size - 4;
#ifdef BYTESWAPPED
	*int_ptr = (int) mb_swap_int(*int_ptr);
#endif
		
	/* compute checksum */
	uchar_ptr = (mb_u_char *) line;
	for (j=5;j<write_size-3;j++)
	    checksum += uchar_ptr[j];
    
	/* set checksum */
	short_ptr = (short *) &line[buff_len + 23];
#ifdef BYTESWAPPED
	*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
	*short_ptr = (unsigned short) checksum;
#endif

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
int mbr_em300raw_wr_run_parameter(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error)
{
	char	*function_name = "mbr_em300raw_wr_run_parameter";
	int	status = MB_SUCCESS;
	char	line[EM2_RUN_PARAMETER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
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
	write_size = EM2_RUN_PARAMETER_SIZE;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
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
		label = EM2_RUN_PARAMETER;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
		label = store->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(store->run_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(store->run_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(store->run_ping_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(store->run_serial);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(store->run_status);
		line[16] = store->run_mode;
		line[17] = store->run_filter_id;
		short_ptr = (short *) &line[18];
		*short_ptr = (unsigned short) mb_swap_short(store->run_min_depth);
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) mb_swap_short(store->run_max_depth);
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) mb_swap_short(store->run_absorption);
		short_ptr = (short *) &line[24];
		*short_ptr = (unsigned short) mb_swap_short(store->run_tran_pulse);
		short_ptr = (short *) &line[26];
		*short_ptr = (unsigned short) mb_swap_short(store->run_tran_beam);
		line[28] = store->run_tran_pow;
		line[29] = store->run_rec_beam;
		line[30] = store->run_rec_band;
		line[31] = store->run_rec_gain;
		line[32] = store->run_tvg_cross;
		line[33] = store->run_ssv_source;
		short_ptr = (short *) &line[34];
		*short_ptr = (unsigned short) mb_swap_short(store->run_max_swath);
		line[36] = store->run_beam_space;
		line[37] = store->run_swath_angle;
		line[38] = store->run_stab_mode;
		for (i=0;i<6;i++)
		    line[39+i] = store->run_spare[i];
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) store->run_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) store->run_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) store->run_ping_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) store->run_serial;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) store->run_status;
		line[16] = store->run_mode;
		line[17] = store->run_filter_id;
		short_ptr = (short *) &line[18];
		*short_ptr = (unsigned short) store->run_min_depth;
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) store->run_max_depth;
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) store->run_absorption;
		short_ptr = (short *) &line[24];
		*short_ptr = (unsigned short) store->run_tran_pulse;
		short_ptr = (short *) &line[26];
		*short_ptr = (unsigned short) store->run_tran_beam;
		line[28] = store->run_tran_pow;
		line[29] = store->run_rec_beam;
		line[30] = store->run_rec_band;
		line[31] = store->run_rec_gain;
		line[32] = store->run_tvg_cross;
		line[33] = store->run_ssv_source;
		short_ptr = (short *) &line[34];
		*short_ptr = (unsigned short) store->run_max_swath;
		line[36] = store->run_beam_space;
		line[37] = store->run_swath_angle;
		line[38] = store->run_stab_mode;
		for (i=0;i<6;i++)
		    line[39+i] = store->run_spare[i];
#endif
		line[EM2_RUN_PARAMETER_SIZE-7] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_RUN_PARAMETER_SIZE-7;j++)
		    checksum += uchar_ptr[j];
	    
		/* set checksum */
		short_ptr = (short *) &line[EM2_RUN_PARAMETER_SIZE-6];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

		/* write out data */
		write_len = fwrite(line,1,EM2_RUN_PARAMETER_SIZE-4,mbfp);
		if (write_len != EM2_RUN_PARAMETER_SIZE-4)
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
int mbr_em300raw_wr_clock(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error)
{
	char	*function_name = "mbr_em300raw_wr_clock";
	int	status = MB_SUCCESS;
	char	line[EM2_CLOCK_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
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
	write_size = EM2_CLOCK_SIZE;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
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
		label = EM2_CLOCK;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
		label = store->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(store->clk_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(store->clk_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(store->clk_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(store->clk_serial);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(store->clk_origin_date);
		int_ptr = (int *) &line[16];
		*int_ptr = (int) mb_swap_int(store->clk_origin_msec);
		line[20] = store->clk_1_pps_use;
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) store->clk_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) store->clk_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) store->clk_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) store->clk_serial;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) store->clk_origin_date;
		int_ptr = (int *) &line[16];
		*int_ptr = (int) store->clk_origin_msec;
		line[20] = store->clk_1_pps_use;
#endif
		line[EM2_CLOCK_SIZE-7] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_CLOCK_SIZE-7;j++)
		    checksum += uchar_ptr[j];
	    
		/* set checksum */
		short_ptr = (short *) &line[EM2_CLOCK_SIZE-6];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

		/* write out data */
		write_len = fwrite(line,1,EM2_CLOCK_SIZE-4,mbfp);
		if (write_len != EM2_CLOCK_SIZE-4)
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
int mbr_em300raw_wr_tide(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error)
{
	char	*function_name = "mbr_em300raw_wr_tide";
	int	status = MB_SUCCESS;
	char	line[EM2_TIDE_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
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
	write_size = EM2_TIDE_SIZE;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
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
		label = EM2_TIDE;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
		label = store->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(store->tid_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(store->tid_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(store->tid_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(store->tid_serial);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(store->tid_origin_date);
		int_ptr = (int *) &line[16];
		*int_ptr = (int) mb_swap_int(store->tid_origin_msec);
		short_ptr = (short *) &line[20];
		*short_ptr = (short) mb_swap_short(store->tid_tide);
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) store->tid_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) store->tid_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) store->tid_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) store->tid_serial;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) store->tid_origin_date;
		int_ptr = (int *) &line[16];
		*int_ptr = (int) store->tid_origin_msec;
		short_ptr = (short *) &line[20];
		*short_ptr = (short) store->tid_tide;
#endif
		line[EM2_TIDE_SIZE-8] = '\0';
		line[EM2_TIDE_SIZE-7] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_TIDE_SIZE-7;j++)
		    checksum += uchar_ptr[j];
	    
		/* set checksum */
		short_ptr = (short *) &line[EM2_TIDE_SIZE-6];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

		/* write out data */
		write_len = fwrite(line,1,EM2_TIDE_SIZE-4,mbfp);
		if (write_len != EM2_TIDE_SIZE-4)
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
int mbr_em300raw_wr_height(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error)
{
	char	*function_name = "mbr_em300raw_wr_height";
	int	status = MB_SUCCESS;
	char	line[EM2_HEIGHT_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
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
	write_size = EM2_HEIGHT_SIZE;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
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
		label = EM2_HEIGHT;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
		label = store->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(store->hgt_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(store->hgt_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(store->hgt_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(store->hgt_serial);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(store->hgt_height);
		line[16] = (mb_u_char) store->hgt_type;
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) store->hgt_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) store->hgt_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) store->hgt_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) store->hgt_serial;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) store->hgt_height;
		line[16] = (mb_u_char) store->hgt_type;
#endif
		line[EM2_HEIGHT_SIZE-7] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_HEIGHT_SIZE-7;j++)
		    checksum += uchar_ptr[j];
	    
		/* set checksum */
		short_ptr = (short *) &line[EM2_HEIGHT_SIZE-6];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

		/* write out data */
		write_len = fwrite(line,1,EM2_HEIGHT_SIZE-4,mbfp);
		if (write_len != EM2_HEIGHT_SIZE-4)
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
int mbr_em300raw_wr_heading(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error)
{
	char	*function_name = "mbr_em300raw_wr_heading";
	int	status = MB_SUCCESS;
	struct mbsys_simrad2_heading_struct *heading;
	char	line[EM2_HEADING_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}
		
	/* get storage structure */
	heading = (struct mbsys_simrad2_heading_struct *) store->heading;

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
	write_size = EM2_HEADING_HEADER_SIZE 
			+ EM2_HEADING_SLICE_SIZE * heading->hed_ndata + 8;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
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
		label = EM2_HEADING;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
		label = store->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(heading->hed_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(heading->hed_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(heading->hed_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(heading->hed_serial);
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) mb_swap_short(heading->hed_ndata);
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) heading->hed_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) heading->hed_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) heading->hed_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) heading->hed_serial;
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) heading->hed_ndata;
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_HEADING_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_HEADING_HEADER_SIZE,mbfp);
		if (write_len != EM2_HEADING_HEADER_SIZE)
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
#ifdef BYTESWAPPED
		short_ptr = (short *) &line[0];
		*short_ptr = (unsigned short) mb_swap_short(heading->hed_time[i]);
		short_ptr = (short *) &line[2];
		*short_ptr = (unsigned short) mb_swap_short(heading->hed_heading[i]);
#else
		short_ptr = (short *) &line[0];
		*short_ptr = (unsigned short) heading->hed_time[i];
		short_ptr = (short *) &line[2];
		*short_ptr = (unsigned short) heading->hed_heading[i];
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_HEADING_SLICE_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_HEADING_SLICE_SIZE,mbfp);
		if (write_len != EM2_HEADING_SLICE_SIZE)
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
		short_ptr = (short *) &line[2];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

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
int mbr_em300raw_wr_ssv(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error)
{
	char	*function_name = "mbr_em300raw_wr_ssv";
	int	status = MB_SUCCESS;
	struct mbsys_simrad2_ssv_struct *ssv;
	char	line[EM2_SSV_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}
		
	/* get storage structure */
	ssv = (struct mbsys_simrad2_ssv_struct *) store->ssv;

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
	write_size = EM2_SSV_HEADER_SIZE 
			+ EM2_SSV_SLICE_SIZE * ssv->ssv_ndata + 8;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
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
		label = EM2_SSV;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
		label = store->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(ssv->ssv_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(ssv->ssv_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(ssv->ssv_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(ssv->ssv_serial);
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) mb_swap_short(ssv->ssv_ndata);
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) ssv->ssv_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) ssv->ssv_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) ssv->ssv_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) ssv->ssv_serial;
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) ssv->ssv_ndata;
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_SSV_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_SSV_HEADER_SIZE,mbfp);
		if (write_len != EM2_SSV_HEADER_SIZE)
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
#ifdef BYTESWAPPED
		short_ptr = (short *) &line[0];
		*short_ptr = (unsigned short) mb_swap_short(ssv->ssv_time[i]);
		short_ptr = (short *) &line[2];
		*short_ptr = (unsigned short) mb_swap_short(ssv->ssv_ssv[i]);
#else
		short_ptr = (short *) &line[0];
		*short_ptr = (unsigned short) ssv->ssv_time[i];
		short_ptr = (short *) &line[2];
		*short_ptr = (unsigned short) ssv->ssv_ssv[i];
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_SSV_SLICE_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_SSV_SLICE_SIZE,mbfp);
		if (write_len != EM2_SSV_SLICE_SIZE)
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
		short_ptr = (short *) &line[2];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

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
int mbr_em300raw_wr_attitude(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error)
{
	char	*function_name = "mbr_em300raw_wr_attitude";
	int	status = MB_SUCCESS;
	struct mbsys_simrad2_attitude_struct *attitude;
	char	line[EM2_ATTITUDE_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}
		
	/* get storage structure */
	attitude = (struct mbsys_simrad2_attitude_struct *) store->attitude;

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
	write_size = EM2_ATTITUDE_HEADER_SIZE 
			+ EM2_ATTITUDE_SLICE_SIZE * attitude->att_ndata + 8;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
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
		label = EM2_ATTITUDE;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
		label = store->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(attitude->att_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(attitude->att_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(attitude->att_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(attitude->att_serial);
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) mb_swap_short(attitude->att_ndata);
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) attitude->att_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) attitude->att_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) attitude->att_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) attitude->att_serial;
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) attitude->att_ndata;
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_ATTITUDE_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_ATTITUDE_HEADER_SIZE,mbfp);
		if (write_len != EM2_ATTITUDE_HEADER_SIZE)
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
#ifdef BYTESWAPPED
		short_ptr = (short *) &line[0];
		*short_ptr = (unsigned short) mb_swap_short(attitude->att_time[i]);
		short_ptr = (short *) &line[2];
		*short_ptr = (unsigned short) mb_swap_short(attitude->att_sensor_status[i]);
		short_ptr = (short *) &line[4];
		*short_ptr = (short) mb_swap_short(attitude->att_roll[i]);
		short_ptr = (short *) &line[6];
		*short_ptr = (short) mb_swap_short(attitude->att_pitch[i]);
		short_ptr = (short *) &line[8];
		*short_ptr = (short) mb_swap_short(attitude->att_heave[i]);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(attitude->att_heading[i]);
#else
		short_ptr = (short *) &line[0];
		*short_ptr = (unsigned short) attitude->att_time[i];
		short_ptr = (short *) &line[2];
		*short_ptr = (unsigned short) attitude->att_sensor_status[i];
		short_ptr = (short *) &line[4];
		*short_ptr = (short) attitude->att_roll[i];
		short_ptr = (short *) &line[6];
		*short_ptr = (short) attitude->att_pitch[i];
		short_ptr = (short *) &line[8];
		*short_ptr = (short) attitude->att_heave[i];
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) attitude->att_heading[i];
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_ATTITUDE_SLICE_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_ATTITUDE_SLICE_SIZE,mbfp);
		if (write_len != EM2_ATTITUDE_SLICE_SIZE)
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
		short_ptr = (short *) &line[2];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

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
int mbr_em300raw_wr_pos(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error)
{
	char	*function_name = "mbr_em300raw_wr_pos";
	int	status = MB_SUCCESS;
	char	line[EM2_POS_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
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
	write_size = EM2_POS_HEADER_SIZE 
			+ store->pos_input_size 
			- (store->pos_input_size % 2) + 8;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
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
		label = EM2_POS;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
		label = store->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(store->pos_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(store->pos_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(store->pos_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(store->pos_serial);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(store->pos_latitude);
		int_ptr = (int *) &line[16];
		*int_ptr = (int) mb_swap_int(store->pos_longitude);
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) mb_swap_short(store->pos_quality);
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) mb_swap_short(store->pos_speed);
		short_ptr = (short *) &line[24];
		*short_ptr = (unsigned short) mb_swap_short(store->pos_course);
		short_ptr = (short *) &line[26];
		*short_ptr = (unsigned short) mb_swap_short(store->pos_heading);
		line[28] = (mb_u_char) store->pos_system;
		line[29] = (mb_u_char) store->pos_input_size;
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) store->pos_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) store->pos_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) store->pos_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) store->pos_serial;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) store->pos_latitude;
		int_ptr = (int *) &line[16];
		*int_ptr = (int) store->pos_longitude;
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) store->pos_quality;
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) store->pos_speed;
		short_ptr = (short *) &line[24];
		*short_ptr = (unsigned short) store->pos_course;
		short_ptr = (short *) &line[26];
		*short_ptr = (unsigned short) store->pos_heading;
		line[28] = (mb_u_char) store->pos_system;
		line[29] = (mb_u_char) store->pos_input_size;
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_POS_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_POS_HEADER_SIZE,mbfp);
		if (write_len != EM2_POS_HEADER_SIZE)
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

	/* output original ascii heading data */
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
		short_ptr = (short *) &line[2];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

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
int mbr_em300raw_wr_svp(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error)
{
	char	*function_name = "mbr_em300raw_wr_svp";
	int	status = MB_SUCCESS;
	char	line[EM2_SVP2_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
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
	write_size = EM2_SVP2_HEADER_SIZE 
			+ EM2_SVP2_SLICE_SIZE * store->svp_num + 8;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
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
		label = EM2_SVP2;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
		label = store->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(store->svp_use_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(store->svp_use_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(store->svp_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(store->svp_serial);
		int_ptr = (int *) &line[12];
		*int_ptr = (int) mb_swap_int(store->svp_origin_date);
		int_ptr = (int *) &line[16];
		*int_ptr = (int) mb_swap_int(store->svp_origin_msec);
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) mb_swap_short(store->svp_num);
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) mb_swap_short(store->svp_depth_res);
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) store->svp_use_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) store->svp_use_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) store->svp_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) store->svp_serial;
		int_ptr = (int *) &line[12];
		*int_ptr = (int) store->svp_origin_date;
		int_ptr = (int *) &line[16];
		*int_ptr = (int) store->svp_origin_msec;
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) store->svp_num;
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) store->svp_depth_res;
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_SVP2_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_SVP2_HEADER_SIZE,mbfp);
		if (write_len != EM2_SVP2_HEADER_SIZE)
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
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(store->svp_depth[i]);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(store->svp_vel[i]);
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) store->svp_depth[i];
		int_ptr = (int *) &line[4];
		*int_ptr = (int) store->svp_vel[i];
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_SVP2_SLICE_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_SVP2_SLICE_SIZE,mbfp);
		if (write_len != EM2_SVP2_SLICE_SIZE)
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
		short_ptr = (short *) &line[2];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

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
int mbr_em300raw_wr_bath(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error)
{
	char	*function_name = "mbr_em300raw_wr_bath";
	int	status = MB_SUCCESS;
	struct mbsys_simrad2_ping_struct *ping;
	char	line[EM2_BATH_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}
		
	/* get storage structure */
	ping = (struct mbsys_simrad2_ping_struct *) store->ping;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       png_date:        %d\n",ping->png_date);
		fprintf(stderr,"dbg5       png_msec:        %d\n",ping->png_msec);
		fprintf(stderr,"dbg5       png_count:       %d\n",ping->png_count);
		fprintf(stderr,"dbg5       png_serial:      %d\n",ping->png_serial);
		fprintf(stderr,"dbg5       png_heading:     %d\n",ping->png_heading);
		fprintf(stderr,"dbg5       png_ssv:         %d\n",ping->png_ssv);
		fprintf(stderr,"dbg5       png_xducer_depth:      %d\n",ping->png_xducer_depth);
		fprintf(stderr,"dbg5       png_offset_multiplier: %d\n",ping->png_offset_multiplier);
		fprintf(stderr,"dbg5       png_nbeams_max:        %d\n",ping->png_nbeams_max);
		fprintf(stderr,"dbg5       png_nbeams:            %d\n",ping->png_nbeams);
		fprintf(stderr,"dbg5       png_depth_res:         %d\n",ping->png_depth_res);
		fprintf(stderr,"dbg5       png_distance_res:      %d\n",ping->png_distance_res);
		fprintf(stderr,"dbg5       png_sample_rate:       %d\n",ping->png_sample_rate);
		fprintf(stderr,"dbg5       cnt  depth xtrack ltrack dprsn   azi   rng  qual wnd amp num\n");
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		for (i=0;i<ping->png_nbeams;i++)
			fprintf(stderr,"dbg5       %3d %6d %6d %6d %5d %5d %5d %4d %3d %3d %3d\n",
				i, ping->png_depth[i], ping->png_acrosstrack[i], 
				ping->png_alongtrack[i], ping->png_depression[i], 
				ping->png_azimuth[i], ping->png_range[i], 
				ping->png_quality[i], ping->png_window[i], 
				ping->png_amp[i], ping->png_beam_num[i]);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	write_size = EM2_BATH_HEADER_SIZE 
			+ EM2_BATH_BEAM_SIZE * ping->png_nbeams + 8;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
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
		label = EM2_BATH;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
		label = store->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(ping->png_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(ping->png_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_serial);
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_heading);
		short_ptr = (short *) &line[14];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_ssv);
		short_ptr = (short *) &line[16];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_xducer_depth);
		line[18] = (mb_u_char) ping->png_nbeams_max;
		line[19] = (mb_u_char) ping->png_nbeams;
		line[20] = (mb_u_char) ping->png_depth_res;
		line[21] = (mb_u_char) ping->png_distance_res;
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_sample_rate);
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) ping->png_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) ping->png_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) ping->png_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) ping->png_serial;
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) ping->png_heading;
		short_ptr = (short *) &line[14];
		*short_ptr = (unsigned short) ping->png_ssv;
		short_ptr = (short *) &line[16];
		*short_ptr = (unsigned short) ping->png_xducer_depth;
		line[18] = (mb_u_char) ping->png_nbeams_max;
		line[19] = (mb_u_char) ping->png_nbeams;
		line[20] = (mb_u_char) ping->png_depth_res;
		line[21] = (mb_u_char) ping->png_distance_res;
		short_ptr = (short *) &line[22];
		*short_ptr = (unsigned short) ping->png_sample_rate;
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_BATH_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_BATH_HEADER_SIZE,mbfp);
		if (write_len != EM2_BATH_HEADER_SIZE)
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
#ifdef BYTESWAPPED
		short_ptr = (short *) &line[0];
		if (store->sonar == EM2_EM120
			|| store->sonar == EM2_EM300)
		    *short_ptr = (unsigned short) mb_swap_short(ping->png_depth[i]);
		else
		    *short_ptr = (short) mb_swap_short(ping->png_depth[i]);
		short_ptr = (short *) &line[2];
		*short_ptr = (short) mb_swap_short(ping->png_acrosstrack[i]);
		short_ptr = (short *) &line[4];
		*short_ptr = (short) mb_swap_short(ping->png_alongtrack[i]);
		short_ptr = (short *) &line[6];
		*short_ptr = (short) mb_swap_short(ping->png_depression[i]);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_azimuth[i]);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_range[i]);
		line[12] = (mb_u_char) ping->png_quality[i];
		line[13] = (mb_u_char) ping->png_window[i];
		line[14] = (mb_s_char) ping->png_amp[i];
		line[15] = (mb_u_char) ping->png_beam_num[i];
#else
		short_ptr = (short *) &line[0];
		if (store->sonar == EM2_EM120
			|| store->sonar == EM2_EM300)
		    *short_ptr = (unsigned short) ping->png_depth[i];
		else
		    *short_ptr = (short) ping->png_depth[i];
		short_ptr = (short *) &line[2];
		*short_ptr = (short) ping->png_acrosstrack[i];
		short_ptr = (short *) &line[4];
		*short_ptr = (short) ping->png_alongtrack[i];
		short_ptr = (short *) &line[6];
		*short_ptr = (short) ping->png_depression[i];
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) ping->png_azimuth[i];
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) ping->png_range[i];
		line[12] = (mb_u_char) ping->png_quality[i];
		line[13] = (mb_u_char) ping->png_window[i];
		line[14] = (mb_s_char) ping->png_amp[i];
		line[15] = (mb_u_char) ping->png_beam_num[i];
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_BATH_BEAM_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_BATH_BEAM_SIZE,mbfp);
		if (write_len != EM2_BATH_BEAM_SIZE)
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
		line[0] = (mb_s_char) ping->png_offset_multiplier;
		line[1] = 0x03;
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum += uchar_ptr[0];
	    
		/* set checksum */
		short_ptr = (short *) &line[2];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

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
int mbr_em300raw_wr_rawbeam(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error)
{
	char	*function_name = "mbr_em300raw_wr_rawbeam";
	int	status = MB_SUCCESS;
	struct mbsys_simrad2_ping_struct *ping;
	char	line[EM2_BATH_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}
		
	/* get storage structure */
	ping = (struct mbsys_simrad2_ping_struct *) store->ping;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       png_date:        %d\n",ping->png_date);
		fprintf(stderr,"dbg5       png_msec:        %d\n",ping->png_msec);
		fprintf(stderr,"dbg5       png_count:       %d\n",ping->png_count);
		fprintf(stderr,"dbg5       png_serial:      %d\n",ping->png_serial);
		fprintf(stderr,"dbg5       png_nbeams_max:  %d\n",ping->png_nbeams_max);
		fprintf(stderr,"dbg5       png_nrawbeams:   %d\n",ping->png_nrawbeams);
		fprintf(stderr,"dbg5       png_ssv:         %d\n",ping->png_ssv);
		fprintf(stderr,"dbg5       cnt  point   tilt   rng  amp num\n");
		fprintf(stderr,"dbg5       ------------------------------------------------------------\n");
		for (i=0;i<ping->png_nrawbeams;i++)
			fprintf(stderr,"dbg5       %3d %5d %5d %5d %3d %3d\n",
				i, ping->png_rawpointangle[i], ping->png_rawtiltangle[i], 
				ping->png_rawrange[i], ping->png_rawamp[i], 
				ping->png_rawbeam_num[i]);
		}
		
	/* zero checksum */
	checksum = 0;

	/* write the record size */
	write_size = EM2_RAWBEAM_HEADER_SIZE 
			+ EM2_RAWBEAM_BEAM_SIZE * ping->png_nrawbeams + 8;
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
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
		label = EM2_RAWBEAM;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
		label = store->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(ping->png_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(ping->png_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_serial);
		line[12] = (mb_u_char) ping->png_nbeams_max;
		line[13] = (mb_u_char) ping->png_nrawbeams;
		short_ptr = (short *) &line[14];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_ssv);
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) ping->png_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) ping->png_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) ping->png_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) ping->png_serial;
		line[12] = (mb_u_char) ping->png_nbeams_max;
		line[13] = (mb_u_char) ping->png_nrawbeams;
		short_ptr = (short *) &line[14];
		*short_ptr = (unsigned short) ping->png_ssv;
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_RAWBEAM_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_RAWBEAM_HEADER_SIZE,mbfp);
		if (write_len != EM2_RAWBEAM_HEADER_SIZE)
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
	    for (i=0;i<ping->png_nrawbeams;i++)
		{
#ifdef BYTESWAPPED
		short_ptr = (short *) &line[0];
		*short_ptr = (short) mb_swap_short(ping->png_rawpointangle[i]);
		short_ptr = (short *) &line[2];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_rawtiltangle[i]);
		short_ptr = (short *) &line[4];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_rawrange[i]);
		line[6] = (mb_s_char) ping->png_rawamp[i];
		line[7] = (mb_u_char) ping->png_rawbeam_num[i];
#else
		short_ptr = (short *) &line[0];
		*short_ptr = (short) ping->png_rawpointangle[i];
		short_ptr = (short *) &line[2];
		*short_ptr = (unsigned short) ping->png_rawtiltangle[i];
		short_ptr = (short *) &line[4];
		*short_ptr = (unsigned short) ping->png_rawrange[i];
		line[6] = (mb_s_char) ping->png_rawamp[i];
		line[7] = (mb_u_char) ping->png_rawbeam_num[i];
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_RAWBEAM_BEAM_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_RAWBEAM_BEAM_SIZE,mbfp);
		if (write_len != EM2_RAWBEAM_BEAM_SIZE)
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
		short_ptr = (short *) &line[2];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

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
int mbr_em300raw_wr_ss(int verbose, FILE *mbfp, 
		struct mbsys_simrad2_struct *store, int *error)
{
	char	*function_name = "mbr_em300raw_wr_ss";
	int	status = MB_SUCCESS;
	struct mbsys_simrad2_ping_struct *ping;
	char	line[EM2_SS_HEADER_SIZE];
	short	label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   *uchar_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}
		
	/* get storage structure */
	ping = (struct mbsys_simrad2_ping_struct *) store->ping;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       type:            %d\n",store->type);
		fprintf(stderr,"dbg5       sonar:           %d\n",store->sonar);
		fprintf(stderr,"dbg5       date:            %d\n",store->date);
		fprintf(stderr,"dbg5       msec:            %d\n",store->msec);
		fprintf(stderr,"dbg5       png_ss_date:     %d\n",ping->png_ss_date);
		fprintf(stderr,"dbg5       png_ss_msec:     %d\n",ping->png_ss_msec);
		fprintf(stderr,"dbg5       png_count:       %d\n",ping->png_count);
		fprintf(stderr,"dbg5       png_serial:      %d\n",ping->png_serial);
		fprintf(stderr,"dbg5       png_max_range:   %d\n",ping->png_max_range);
		fprintf(stderr,"dbg5       png_r_zero:      %d\n",ping->png_r_zero);
		fprintf(stderr,"dbg5       png_r_zero_corr: %d\n",ping->png_r_zero_corr);
		fprintf(stderr,"dbg5       png_tvg_start:   %d\n",ping->png_tvg_start);
		fprintf(stderr,"dbg5       png_tvg_stop:    %d\n",ping->png_tvg_stop);
		fprintf(stderr,"dbg5       png_bsn:         %d\n",ping->png_bsn);
		fprintf(stderr,"dbg5       png_bso:         %d\n",ping->png_bso);
		fprintf(stderr,"dbg5       png_tx:          %d\n",ping->png_tx);
		fprintf(stderr,"dbg5       png_tvg_crossover: %d\n",ping->png_tvg_crossover);
		fprintf(stderr,"dbg5       png_nbeams_ss:     %d\n",ping->png_nbeams_ss);
		fprintf(stderr,"dbg5       png_npixels:       %d\n",ping->png_npixels);
		fprintf(stderr,"dbg5       cnt  index sort samples start center\n");
		fprintf(stderr,"dbg5       --------------------------------------------------\n");
		for (i=0;i<ping->png_nbeams_ss;i++)
			fprintf(stderr,"dbg5        %4d %3d %2d %4d %4d %4d\n",
				i, ping->png_beam_index[i], ping->png_sort_direction[i], 
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
	write_size = EM2_SS_HEADER_SIZE 
			+ EM2_SS_BEAM_SIZE * ping->png_nbeams_ss 
			+ ping->png_npixels - (ping->png_npixels % 2) + 8;
fprintf(stderr, "\nwrite ss em300raw: npixels:%d write_size:%d\n", 
ping->png_npixels, write_size);
#ifdef BYTESWAPPED
	write_size = (int) mb_swap_int(write_size);
#endif
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
		label = EM2_SS;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
		label = store->sonar;
#ifdef BYTESWAPPED
		label = (short) mb_swap_short(label);
#endif
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
#ifdef BYTESWAPPED
		int_ptr = (int *) &line[0];
		*int_ptr = (int) mb_swap_int(ping->png_ss_date);
		int_ptr = (int *) &line[4];
		*int_ptr = (int) mb_swap_int(ping->png_ss_msec);
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_count);
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_serial);
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_max_range);
		short_ptr = (short *) &line[14];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_r_zero);
		short_ptr = (short *) &line[16];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_r_zero_corr);
		short_ptr = (short *) &line[18];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_tvg_start);
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_tvg_stop);
		line[22] = (mb_s_char) ping->png_bsn;
		line[23] = (mb_s_char) ping->png_bso;
		short_ptr = (short *) &line[24];
		*short_ptr = (unsigned short) mb_swap_short(ping->png_tx);
		line[26] = (mb_u_char) ping->png_tvg_crossover;
		line[27] = (mb_u_char) ping->png_nbeams_ss;
#else
		int_ptr = (int *) &line[0];
		*int_ptr = (int) ping->png_ss_date;
		int_ptr = (int *) &line[4];
		*int_ptr = (int) ping->png_ss_msec;
		short_ptr = (short *) &line[8];
		*short_ptr = (unsigned short) ping->png_count;
		short_ptr = (short *) &line[10];
		*short_ptr = (unsigned short) ping->png_serial;
		short_ptr = (short *) &line[12];
		*short_ptr = (unsigned short) ping->png_max_range;
		short_ptr = (short *) &line[14];
		*short_ptr = (unsigned short) ping->png_r_zero;
		short_ptr = (short *) &line[16];
		*short_ptr = (unsigned short) ping->png_r_zero_corr;
		short_ptr = (short *) &line[18];
		*short_ptr = (unsigned short) ping->png_tvg_start;
		short_ptr = (short *) &line[20];
		*short_ptr = (unsigned short) ping->png_tvg_stop;
		line[22] = (mb_s_char) ping->png_bsn;
		line[23] = (mb_s_char) ping->png_bso;
		short_ptr = (short *) &line[24];
		*short_ptr = (unsigned short) ping->png_tx;
		line[26] = (mb_u_char) ping->png_tvg_crossover;
		line[27] = (mb_u_char) ping->png_nbeams_ss;
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_SS_HEADER_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_SS_HEADER_SIZE,mbfp);
		if (write_len != EM2_SS_HEADER_SIZE)
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
#ifdef BYTESWAPPED
		line[0] = (mb_u_char) ping->png_beam_index[i];
		line[1] = (mb_s_char) ping->png_sort_direction[i];
		short_ptr = (short *) &line[2];
		*short_ptr = (short) mb_swap_short(ping->png_beam_samples[i]);
		short_ptr = (short *) &line[4];
		*short_ptr = (short) mb_swap_short(ping->png_center_sample[i]);
#else
		line[0] = (mb_u_char) ping->png_beam_index[i];
		line[1] = (mb_s_char) ping->png_sort_direction[i];
		short_ptr = (short *) &line[2];
		*short_ptr = (short) ping->png_beam_samples[i];
		short_ptr = (short *) &line[4];
		*short_ptr = (short) ping->png_center_sample[i];
#endif
		
		/* compute checksum */
		uchar_ptr = (mb_u_char *) line;
		for (j=0;j<EM2_SS_BEAM_SIZE;j++)
		    checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line,1,EM2_SS_BEAM_SIZE,mbfp);
		if (write_len != EM2_SS_BEAM_SIZE)
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
		write_size = ping->png_npixels + 1 - (ping->png_npixels % 2);
		if (ping->png_npixels % 2 == 0)
		    ping->png_ssraw[ping->png_npixels] = 0;
fprintf(stderr, "                              write_size:%d\n", write_size);

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
		short_ptr = (short *) &line[2];
#ifdef BYTESWAPPED
		*short_ptr = (unsigned short) mb_swap_short(checksum);
#else
		*short_ptr = (unsigned short) checksum;
#endif

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
