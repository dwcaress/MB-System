/*--------------------------------------------------------------------
 *    The MB-system:	mbr_reson7kr.c	4/4/2004
 *	$Id: mbr_reson7kr.c,v 5.8 2005-04-07 04:24:33 caress Exp $
 *
 *    Copyright (c) 2004 by
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
 * mbr_reson7kr.c contains the functions for reading and writing
 * multibeam data in the RESON7KR format.  
 * These functions include:
 *   mbr_alm_reson7kr	- allocate read/write memory
 *   mbr_dem_reson7kr	- deallocate read/write memory
 *   mbr_rt_reson7kr	- read and translate data
 *   mbr_wt_reson7kr	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	April 4,2004
 * $Log: not supported by cvs2svn $
 * Revision 5.7  2005/03/25 04:33:30  caress
 * Improved handling of interpolated asynchronous data, particularly sonar depth.
 *
 * Revision 5.6  2004/12/02 06:33:31  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.5  2004/11/08 05:47:19  caress
 * Now gets sidescan from snippet data, maybe even properly...
 *
 * Revision 5.4  2004/11/06 03:55:16  caress
 * Working to support the Reson 7k format.
 *
 * Revision 5.3  2004/07/15 19:25:05  caress
 * Progress in supporting Reson 7k data.
 *
 * Revision 5.2  2004/06/18 05:22:32  caress
 * Working on adding support for segy i/o and for Reson 7k format 88.
 *
 * Revision 5.1  2004/05/21 23:44:49  caress
 * Progress supporting Reson 7k data, including support for extracing subbottom profiler data.
 *
 * Revision 5.0  2004/04/27 01:50:16  caress
 * Adding support for Reson 7k sonar data, including segy extensions.
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
#include "../../include/mbsys_reson7k.h"

/* include for byte swapping */
#include "../../include/mb_swap.h"
	
/* turn on debug statements here */
/* #define MBR_RESON7KR_DEBUG 1 */
	
/* essential function prototypes */
int mbr_register_reson7kr(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_reson7kr(int verbose, 
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
int mbr_alm_reson7kr(int verbose, void *mbio_ptr, int *error);
int mbr_dem_reson7kr(int verbose, void *mbio_ptr, int *error);
int mbr_rt_reson7kr(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_reson7kr(int verbose, void *mbio_ptr, void *store_ptr, int *error);

int mbr_reson7kr_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_reson7kr_chk_label(int verbose, void *mbio_ptr, short type);
int mbr_reson7kr_chk_pingnumber(int verbose, int recordid, char *buffer, 
				int *ping_number);
int mbr_reson7kr_rd_header(int verbose, char *buffer, int *index, 
		s7k_header *header, int *error);

int mbr_reson7kr_rd_reference(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_sensoruncal(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_sensorcal(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_position(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_customattitude(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_tide(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_altitude(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_motion(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_depth(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_svp(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_ctd(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_geodesy(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_rollpitchheave(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_heading(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_attitude(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_fsdwchannel(int verbose, int data_format, char *buffer, int *index, s7k_fsdwchannel *fsdwchannel, int *error);
int mbr_reson7kr_rd_fsdwssheader(int verbose, char *buffer, int *index, s7k_fsdwssheader *fsdwssheader, int *error);
int mbr_reson7kr_rd_fsdwsegyheader(int verbose, char *buffer, int *index, s7k_fsdwsegyheader *fsdwsegyheader, int *error);
int mbr_reson7kr_rd_fsdwsslo(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_fsdwsshi(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_fsdwsb(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_bluefin(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_volatilesonarsettings(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_configuration(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_beamgeometry(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_calibration(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_bathymetry(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_backscatter(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_beam(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_image(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_systemeventmessage(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_fileheader(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_remotecontrolsettings(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_roll(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_pitch(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_soundvelocity(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_absorptionloss(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_spreadingloss(int verbose, char *buffer, void *store_ptr, int *error);

int mbr_reson7kr_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_reson7kr_wr_header(int verbose, char *buffer, int *index, s7k_header *header, int *error);

int mbr_reson7kr_wr_reference(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_sensoruncal(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_sensorcal(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_position(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_customattitude(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_tide(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_altitude(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_motion(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_depth(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_svp(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_ctd(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_geodesy(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_rollpitchheave(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_heading(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_attitude(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_fsdwchannel(int verbose, int data_format, char *buffer, int *index, s7k_fsdwchannel *fsdwchannel, int *error);
int mbr_reson7kr_wr_fsdwssheader(int verbose, char *buffer, int *index, s7k_fsdwssheader *fsdwssheader, int *error);
int mbr_reson7kr_wr_fsdwsegyheader(int verbose, char *buffer, int *index, s7k_fsdwsegyheader *fsdwsegyheader, int *error);
int mbr_reson7kr_wr_fsdwsslo(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_fsdwsshi(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_fsdwsb(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_bluefin(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_volatilesonarsettings(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_configuration(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_beamgeometry(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_calibration(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_bathymetry(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_backscatter(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_beam(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_image(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_systemeventmessage(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_fileheader(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_remotecontrolsettings(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_roll(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_pitch(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_soundvelocity(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_absorptionloss(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_spreadingloss(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);

static char res_id[]="$Id: mbr_reson7kr.c,v 5.8 2005-04-07 04:24:33 caress Exp $";

/*--------------------------------------------------------------------*/
int mbr_register_reson7kr(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_reson7kr";
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
	status = mbr_info_reson7kr(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_reson7kr;
	mb_io_ptr->mb_io_format_free = &mbr_dem_reson7kr; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_reson7k_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_reson7k_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_reson7kr; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_reson7kr; 
	mb_io_ptr->mb_io_extract = &mbsys_reson7k_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_reson7k_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_reson7k_extract_nav; 
	mb_io_ptr->mb_io_extract_nnav = &mbsys_reson7k_extract_nnav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_reson7k_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_reson7k_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = &mbsys_reson7k_extract_svp; 
	mb_io_ptr->mb_io_insert_svp = &mbsys_reson7k_insert_svp; 
	mb_io_ptr->mb_io_ttimes = &mbsys_reson7k_ttimes; 
	mb_io_ptr->mb_io_detects = &mbsys_reson7k_detects; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_reson7k_copy; 
	mb_io_ptr->mb_io_extract_rawss = NULL; 
	mb_io_ptr->mb_io_insert_rawss = NULL; 
	mb_io_ptr->mb_io_extract_segytraceheader = &mbsys_reson7k_extract_segytraceheader; 
	mb_io_ptr->mb_io_extract_segy = &mbsys_reson7k_extract_segy; 
	mb_io_ptr->mb_io_insert_segy = &mbsys_reson7k_insert_segy; 

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
		fprintf(stderr,"dbg2       extract_segytraceheader: %d\n",mb_io_ptr->mb_io_extract_segytraceheader);
		fprintf(stderr,"dbg2       extract_segy:       %d\n",mb_io_ptr->mb_io_extract_segy);
		fprintf(stderr,"dbg2       insert_segy:        %d\n",mb_io_ptr->mb_io_insert_segy);
		fprintf(stderr,"dbg2       copyrecord:         %d\n",mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_reson7kr(int verbose, 
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
	char	*function_name = "mbr_info_reson7kr";
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
	*system = MB_SYS_RESON7K;
	*beams_bath_max = 254;
	*beams_amp_max = 254;
	*pixels_ss_max = 4096;
	strncpy(format_name, "RESON7KR", MB_NAME_LENGTH);
	strncpy(system_name, "RESON7K", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_RESON7KR\nInformal Description: Reson 7K multibeam vendor format\nAttributes:           Reson 7K series multibeam sonars, \n                      bathymetry, amplitude, three channels sidescan, and subbottom\n                      up to 254 beams, variable pixels, binary, Reson.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_NAV;
	*heading_source = MB_DATA_NAV;
	*vru_source = MB_DATA_NAV;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
	*beamwidth_xtrack = 1.0;
	*beamwidth_ltrack = 1.0;

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
int mbr_alm_reson7kr(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_reson7kr";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	*current_ping;
	int	*last_ping;
	int	*save_flag;
	int	*recordid;
	int	*recordidlast;
	char	**bufferptr;
	char	*buffer;
	int	*bufferalloc;
	char	**buffersaveptr;
	char	*buffersave;
	int	*size;
	int	*nbadrec;
	int	*deviceid;
	unsigned short	*enumerator;
	int	*fileheaders;
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
	status = mbsys_reson7k_alloc(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);
	save_flag = (int *) &mb_io_ptr->save_flag;
	current_ping = (int *) &mb_io_ptr->save14;
	last_ping = (int *) &mb_io_ptr->save1;
	recordid = (int *) &mb_io_ptr->save3;
	recordidlast = (int *) &mb_io_ptr->save4;
	bufferptr = (char **) &mb_io_ptr->save5;
	buffer = (char *) *bufferptr;
	bufferalloc = (int *) &mb_io_ptr->save6;
	buffersaveptr = (char **) &mb_io_ptr->save7;
	buffersave = (char *) *buffersaveptr;
	size = (int *) &mb_io_ptr->save8;
	nbadrec = (int *) &mb_io_ptr->save9;
	deviceid = (int *) &mb_io_ptr->save10;
	enumerator = (unsigned short *) &mb_io_ptr->save11;
	fileheaders = (int *) &mb_io_ptr->save12;
	pixel_size = (double *) &mb_io_ptr->saved1;
	swath_width = (double *) &mb_io_ptr->saved2;
	*current_ping = -1;
	*last_ping = -1;
	*save_flag = MB_NO;
	*recordid = R7KRECID_None;
	*recordidlast = R7KRECID_None;
	*bufferptr = NULL;
	*bufferalloc = 0;
	*size = 0;
	*nbadrec = 0;
	*deviceid = 0;
	*enumerator = 0;
	*fileheaders = 0;
	*pixel_size = 0.0;
	*swath_width = 0.0;
	
	/* allocate memory if necessary */
	if (status == MB_SUCCESS)
		{
		status = mb_realloc(verbose, MBSYS_RESON7K_BUFFER_STARTSIZE,
					bufferptr, error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, MBSYS_RESON7K_BUFFER_STARTSIZE,
					buffersaveptr, error);
		if (status == MB_SUCCESS)
			*bufferalloc = MBSYS_RESON7K_BUFFER_STARTSIZE;
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
int mbr_dem_reson7kr(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_reson7kr";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	char	**bufferptr;
	char	*buffer;
	int	*bufferalloc;
	char	**buffersaveptr;
	char	*buffersave;

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
	status = mbsys_reson7k_deall(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);

	/* deallocate memory for reading/writing buffer */
	bufferptr = (char **) &mb_io_ptr->save5;
	buffer = (char *) *bufferptr;
	bufferalloc = (int *) &mb_io_ptr->save6;
	buffersaveptr = (char **) &mb_io_ptr->save7;
	buffersave = (char *) *buffersaveptr;
	status = mb_free(verbose,bufferptr,error);
	status = mb_free(verbose,buffersaveptr,error);
	*bufferalloc = 0;

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
int mbr_rt_reson7kr(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_reson7kr";
	int	status = MB_SUCCESS;
	int	interp_status;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_position 		*position;
	s7kr_attitude 		*attitude;
	s7kr_volatilesettings	*volatilesettings;
	s7kr_beamgeometry	*beamgeometry;
	s7kr_bathymetry		*bathymetry;
	s7kr_bluefin		*bluefin;
	int	*current_ping;
	double	speed, heading, longitude, latitude;
	double	roll, pitch, heave;
	double	sonar_depth, sonar_altitude;
	double	soundspeed, alpha, beta, theta, phi;
	double	rr, xx, zz;
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
	status = mbr_reson7kr_rd_data(verbose,mbio_ptr,store_ptr,error);

	/* get pointers to data structures */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	position = &store->position;
	attitude = &store->attitude;
	volatilesettings = &store->volatilesettings;
	beamgeometry = &store->beamgeometry;
	bathymetry = &store->bathymetry;
	bluefin = &store->bluefin;
	current_ping = (int *) &mb_io_ptr->save14;

	/* save fix if nav data */
	if (status == MB_SUCCESS
		&& store->kind == MB_DATA_NAV)
		{
		/* add latest fix */
		position = &(store->position);
		mb_navint_add(verbose, mbio_ptr, 
				store->time_d, 
				position->longitude, 
				position->latitude, 
				error);
		}

	/* save nav and attitude if bluefin data */
	if (status == MB_SUCCESS
		&& store->kind == MB_DATA_NAV1)
		{
		/* add latest fix */
		bluefin = &(store->bluefin);
		for (i=0;i<bluefin->number_frames;i++)
			{
			mb_navint_add(verbose, mbio_ptr, 
					(double)(bluefin->nav[i].position_time), 
					(double)(RTD * bluefin->nav[i].longitude), 
					(double)(RTD * bluefin->nav[i].latitude), 
					error);
			mb_attint_add(verbose, mbio_ptr,
					(double)(bluefin->nav[i].position_time), 
					(double)(0.0), 
					(double)(RTD * bluefin->nav[i].roll), 
					(double)(RTD * bluefin->nav[i].pitch), 
					error);
			mb_hedint_add(verbose, mbio_ptr,
					(double)(bluefin->nav[i].position_time), 
					(double)(RTD * bluefin->nav[i].yaw), 
					error);
			if (mb_io_ptr->nsonardepth == 0
				|| (bluefin->nav[i].depth 
					!= mb_io_ptr->sonardepth_sonardepth[mb_io_ptr->nsonardepth-1]))
			mb_depint_add(verbose, mbio_ptr,
					(double)(bluefin->nav[i].position_time), 
					(double)(bluefin->nav[i].depth), 
					error);
			mb_altint_add(verbose, mbio_ptr,
					(double)(bluefin->nav[i].altitude_time), 
					(double)(bluefin->nav[i].altitude), 
					error);
			}
		}

	/* save attitude if attitude data */
	if (status == MB_SUCCESS
		&& store->kind == MB_DATA_ATTITUDE)
		{
		/* get attitude structure */
		attitude = &(store->attitude);
		
		/* add latest attitude samples */
		for (i=0;i<attitude->n;i++)
			{
			mb_attint_add(verbose, mbio_ptr,
				(double)(store->time_d + 0.001 * ((double)attitude->delta_time[i])),
				(double)(attitude->heave[i]),
				(double)(attitude->roll[i]),
				(double)(attitude->pitch[i]),
				error);
			mb_hedint_add(verbose, mbio_ptr,
				(double)(store->time_d + 0.001 * ((double)attitude->delta_time[i])),
				(double)(attitude->heading[i]),
				error);
			}
		}
#ifdef MBR_RESON7KR_DEBUG
if (verbose > 0)
fprintf(stderr,"Record returned: type:%d status:%d error:%d\n\n",store->kind, status, *error);
#endif

	/* kluge to reset quality flags */
	if (status == MB_SUCCESS
		&& store->kind == MB_DATA_DATA)
		{
		for (i=0;i<bathymetry->number_beams;i++)
			{
			if ((bathymetry->quality[i] & 15) < 2)
				{
				if (bathymetry->range[i] > 0.007)
					{
					bathymetry->quality[i] = (bathymetry->quality[i] & 240) + 15;
					}
				else
					{
					bathymetry->quality[i] = (bathymetry->quality[i] & 240) + 3;
					}
				}
			}
		}

	/* get optional values in bathymetry record if needed */
	if (status == MB_SUCCESS
		&& store->kind == MB_DATA_DATA
		&& bathymetry->optionaldata == MB_NO)
		{
		/* get navigation, etc */
		speed = 0.0;
		interp_status = mb_hedint_interp(verbose, mbio_ptr, store->time_d,  
				    &heading, error);
		if (interp_status == MB_SUCCESS)
		interp_status = mb_navint_interp(verbose, mbio_ptr, store->time_d, heading, speed, 
				    &longitude, &latitude, &speed, error);
		if (interp_status == MB_SUCCESS)
		interp_status = mb_depint_interp(verbose, mbio_ptr, store->time_d,  
				    &sonar_depth, error);
		if (interp_status == MB_SUCCESS)
		interp_status = mb_altint_interp(verbose, mbio_ptr, store->time_d,  
				    &sonar_altitude, error);
		if (interp_status == MB_SUCCESS)
		interp_status = mb_attint_interp(verbose, mbio_ptr, store->time_d,  
				    &heave, &roll, &pitch, error);
				    
		/* if the optional data are not all available, this ping
			is not useful, and is discarded by setting
			*error to MB_ERROR_UNINTELLIGIBLE */
		if (interp_status == MB_FAILURE)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
			
		/* if the optional data are available, then proceed */
		else
			{
			bathymetry->longitude = DTR * longitude;
			bathymetry->latitude = DTR * latitude;
			bathymetry->heading = DTR * heading;
			bathymetry->height_source = 1;
			bathymetry->tide = 0.0;
			bathymetry->roll = DTR * roll;
			bathymetry->pitch = DTR * pitch;
			bathymetry->heave = heave;
			bathymetry->vehicle_height = -sonar_depth;

			/* get bathymetry */
			if (volatilesettings->sound_velocity > 0.0)
				soundspeed = volatilesettings->sound_velocity;
			else if (bluefin->environmental[0].sound_speed > 0.0)
				soundspeed = bluefin->environmental[0].sound_speed;
			else
				soundspeed = 1500.0;
			for (i=0;i<bathymetry->number_beams;i++)
				{
				if ((bathymetry->quality[i] & 15) > 0)
					{
					alpha = RTD * beamgeometry->angle_alongtrack[i] + bathymetry->pitch;
					beta = 90.0 - RTD * beamgeometry->angle_acrosstrack[i] + bathymetry->roll;
					mb_rollpitch_to_takeoff(
						verbose, 
						alpha, beta, 
						&theta, &phi, 
						error);
					rr = 0.5 * soundspeed * bathymetry->range[i];
					xx = rr * sin(DTR * theta);
					zz = rr * cos(DTR * theta);
					bathymetry->acrosstrack[i] = xx * cos(DTR * phi);
					bathymetry->alongtrack[i] = xx * sin(DTR * phi);
					bathymetry->depth[i] = zz + sonar_depth;
					bathymetry->pointing_angle[i] = DTR * theta;
					bathymetry->azimuth_angle[i] = DTR * phi;
					}
				else
					{
					bathymetry->depth[i] = 0.0;
					bathymetry->acrosstrack[i] = 0.0;
					bathymetry->alongtrack[i] = 0.0;
					bathymetry->pointing_angle[i] = 0.0;
					bathymetry->azimuth_angle[i] = 0.0;
					}
				}

			/* set flag */
			bathymetry->optionaldata = MB_YES;
			bathymetry->header.OffsetToOptionalData 
					= MBSYS_RESON7K_RECORDHEADER_SIZE 
						+ R7KHDRSIZE_7kBathymetricData
						+ bathymetry->number_beams * 9;
			}
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
int mbr_wt_reson7kr(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_reson7kr";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;

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
	store = (struct mbsys_reson7k_struct *) store_ptr;

	/* write next data to file */
	status = mbr_reson7kr_wr_data(verbose,mbio_ptr,store_ptr,error);

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
int mbr_reson7kr_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_bathymetry *bathymetry;
	FILE	*mbfp;
	int	done;
	int	*current_ping;
	int	*last_ping;
	int	*new_ping;
	int	*save_flag;
	int	*recordid;
	int	*recordidlast;
	int	*deviceid;
	unsigned short	*enumerator;
	int	*fileheaders;
	char	**bufferptr;
	char	*buffer;
	int	*bufferalloc;
	char	**buffersaveptr;
	char	*buffersave;
	int	*size;
	int	*nbadrec;
	int	read_len;
	int	skip;
	int	ping_record;
double klugelon, klugelat;
	int	time_j[5];
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
	store = (struct mbsys_reson7k_struct *) store_ptr;
	mbfp = mb_io_ptr->mbfp;
	
	/* get saved values */
	save_flag = (int *) &mb_io_ptr->save_flag;
	current_ping = (int *) &mb_io_ptr->save14;
	last_ping = (int *) &mb_io_ptr->save1;
	new_ping = (int *) &mb_io_ptr->save2;
	recordid = (int *) &mb_io_ptr->save3;
	recordidlast = (int *) &mb_io_ptr->save4;
	bufferptr = (char **) &mb_io_ptr->save5;
	buffer = (char *) *bufferptr;
	bufferalloc = (int *) &mb_io_ptr->save6;
	buffersaveptr = (char **) &mb_io_ptr->save7;
	buffersave = (char *) *buffersaveptr;
	size = (int *) &mb_io_ptr->save8;
	nbadrec = (int *) &mb_io_ptr->save9;
	deviceid = (int *) &mb_io_ptr->save10;
	enumerator = (unsigned short *) &mb_io_ptr->save11;
	fileheaders = (int *) &mb_io_ptr->save12;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* loop over reading data until a record is ready for return */
	done = MB_NO;
	*error = MB_ERROR_NO_ERROR;
	while (done == MB_NO)
		{
		/* if no header saved get next record label */
		if (*save_flag == MB_NO)
			{
			/* read next record header into buffer */
			if ((read_len = fread(buffer,
						1,MBSYS_RESON7K_VERSIONSYNCSIZE,
						mb_io_ptr->mbfp)) 
					!= MBSYS_RESON7K_VERSIONSYNCSIZE)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			
			/* check header - if not a good header read a byte 
				at a time until a good header is found */
			skip = 0;
			while (status == MB_SUCCESS
				&& mbr_reson7kr_chk_header(verbose, mbio_ptr, buffer, 
				recordid, deviceid, enumerator, size) != MB_SUCCESS)
			    {
			    /* get next byte */
			    for (i=0;i<MBSYS_RESON7K_VERSIONSYNCSIZE-1;i++)
				buffer[i] = buffer[i+1];
			    if ((read_len = fread(&buffer[MBSYS_RESON7K_VERSIONSYNCSIZE-1],
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
"\nThe MBF_RESON7KR module skipped data between identified\n\
data records. Something is broken, most probably the data...\n\
However, the data may include a data record type that we\n\
haven't seen yet, or there could be an error in the code.\n\
If skipped data are reported multiple times, \n\
we recommend you send a data sample and problem \n\
description to the MB-System team \n\
(caress@mbari.org and dale@ldeo.columbia.edu)\n\
Have a nice day...\n");
				fprintf(stderr,
						"MBF_RESON7KR skipped %d bytes between records %4.4hX:%d and %4.4hX:%d\n",
						skip, *recordidlast, *recordidlast, *recordid, *recordid);
				(*nbadrec)++;
			    }
			*recordidlast = *recordid;
			store->type = *recordid;
			
			/* allocate memory to read rest of record if necessary */
			if (*bufferalloc < *size)
				{
				status = mb_realloc(verbose, *size,
							bufferptr, error);
				if (status == MB_SUCCESS)
				status = mb_realloc(verbose, *size,
							buffersaveptr, error);
				if (status != MB_SUCCESS)
					{
					*bufferalloc = 0;
					done = MB_YES;
					}
				else
					{
					*bufferalloc = *size;
					buffer = (char *) *bufferptr;
					buffersave = (char *) *buffersaveptr;
					}
				}
			
			/* read the rest of the record */
			if (status == MB_SUCCESS
				&& (read_len = fread(&(buffer[MBSYS_RESON7K_VERSIONSYNCSIZE]),
						1, (*size - MBSYS_RESON7K_VERSIONSYNCSIZE),
						mb_io_ptr->mbfp)) 
					!= (*size - MBSYS_RESON7K_VERSIONSYNCSIZE))
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			
#ifndef MBR_RESON7KR_DEBUG
if (skip > 0)
fprintf(stderr,"RESON7KR record:skip:%d recordid:%x %d deviceid:%x %d enumerator:%x %d size:%d done:%d\n",
skip, *recordid, *recordid, 
*deviceid, *deviceid, 
*enumerator, *enumerator, 
*size, done);
#endif
			}
		
		/* else use saved record */
		else
			{
			*save_flag = MB_NO;
			mbr_reson7kr_chk_header(verbose, mbio_ptr, buffersave, 
						recordid, deviceid, enumerator, size);
			for (i=0;i<*size;i++)
				buffer[i] = buffersave[i];
			}


		/* check for ping record and ping number */
		ping_record = MB_NO;
		if (status == MB_SUCCESS)
			{
			if (*recordid == R7KRECID_7kVolatileSonarSettings 
				|| *recordid == R7KRECID_7kMatchFilter 
				|| *recordid == R7KRECID_7kBeamGeometry 
				|| *recordid == R7KRECID_7kBathymetricData 
				|| *recordid == R7KRECID_7kBackscatterImageData 
				|| *recordid == R7KRECID_7kBeamData 
				|| *recordid == R7KRECID_7kVerticalDepth 
				|| *recordid == R7KRECID_7kImageData)
				{
				/* check for ping number */
				ping_record = MB_YES;
				mbr_reson7kr_chk_pingnumber(verbose, *recordid, buffer, new_ping);
				
				/* fix lack of ping number for backscatter and beam geometry records */
				if (*recordid == R7KRECID_7kBackscatterImageData
					&& *new_ping <= 0)
					*new_ping = *last_ping;
				else if (*recordid == R7KRECID_7kBeamGeometry)
					*new_ping = *last_ping;
					
				/* determine if record is continuation of the last ping
					or a new ping - if new ping and last ping not yet
					output then save the new record and output the
					last ping as fully read */
				if (*last_ping >= 0
					&& *new_ping >= 0
					&& *last_ping != *new_ping)
					{
					done = MB_YES;
					store->kind = MB_DATA_DATA;
					*save_flag = MB_YES;
					*current_ping = *last_ping;
					*last_ping = -1;
					for (i=0;i<*size;i++)
						buffersave[i] = buffer[i];

					/* get the time */
					bathymetry = &(store->bathymetry);
					header = &(bathymetry->header);
					time_j[0] = header->s7kTime.Year;
					time_j[1] = header->s7kTime.Day;
					time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
					time_j[3] = (int) header->s7kTime.Seconds;
					time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
					mb_get_itime(verbose, time_j, store->time_i);
					mb_get_time(verbose, store->time_i, &(store->time_d));
					
					/* not a complete record unless there is bathymetry */
					if (store->read_bathymetry == MB_NO)
						{
						status = MB_FAILURE;
						*error = MB_ERROR_UNINTELLIGIBLE;
						}

					}
				else if (*last_ping >= 0
					&& *new_ping >= 0
					&& *last_ping == *new_ping)
					{
					done = MB_NO;
					}
				else if (*last_ping == -1
					&& *new_ping >= 0)
					{
					done = MB_NO;
					*current_ping = -1;
					*last_ping = *new_ping;
					store->read_volatilesettings = MB_NO;
					store->read_matchfilter = MB_NO;
					store->read_beamgeometry = MB_NO;
					store->read_bathymetry = MB_NO;
					store->read_backscatter = MB_NO;
					store->read_beam = MB_NO;
					store->read_verticaldepth = MB_NO;
					store->read_image = MB_NO;
					}
				}
			}
			
		/* check for ping data already read in read error case */
		if (status == MB_FAILURE && *last_ping >= 0)
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			done = MB_YES;
			*save_flag = MB_NO;
			*last_ping = -1;
			store->kind = MB_DATA_DATA;
			}

#ifdef MBR_RESON7KR_DEBUG
if (status == MB_SUCCESS && *save_flag == MB_NO)
{
fprintf(stderr, "Reading record id: %4.4hX  %4.4d | %4.4hX  %4.4d | %4.4hX  %4.4d |", 
*recordid, *recordid, *deviceid, *deviceid, *enumerator, *enumerator);
if (*recordid == R7KRECID_ReferencePoint) fprintf(stderr," R7KRECID_ReferencePoint\n");
if (*recordid == R7KRECID_UncalibratedSensorOffset) fprintf(stderr," R7KRECID_UncalibratedSensorOffset\n");
if (*recordid == R7KRECID_CalibratedSensorOffset) fprintf(stderr," R7KRECID_CalibratedSensorOffset\n");
if (*recordid == R7KRECID_Position) fprintf(stderr," R7KRECID_Position\n");
if (*recordid == R7KRECID_Attitude) fprintf(stderr," R7KRECID_Attitude\n");
if (*recordid == R7KRECID_Tide) fprintf(stderr," R7KRECID_Tide\n");
if (*recordid == R7KRECID_Altitude) fprintf(stderr," R7KRECID_Altitude\n");
if (*recordid == R7KRECID_MotionOverGround) fprintf(stderr," R7KRECID_MotionOverGround\n");
if (*recordid == R7KRECID_Depth) fprintf(stderr," R7KRECID_Depth\n");
if (*recordid == R7KRECID_SoundVelocityProfile) fprintf(stderr," R7KRECID_SoundVelocityProfile\n");
if (*recordid == R7KRECID_CTD) fprintf(stderr," R7KRECID_CTD\n");
if (*recordid == R7KRECID_Geodesy) fprintf(stderr," R7KRECID_Geodesy\n");
if (*recordid == R7KRECID_FSDWsidescan) fprintf(stderr," R7KRECID_FSDWsidescan\n");
if (*recordid == R7KRECID_FSDWsubbottom) fprintf(stderr," R7KRECID_FSDWsubbottom\n");
if (*recordid == R7KRECID_Bluefin) fprintf(stderr," R7KRECID_Bluefin\n");
if (*recordid == R7KRECID_7kVolatileSonarSettings) fprintf(stderr," R7KRECID_7kVolatileSonarSettings\n");
if (*recordid == R7KRECID_7kConfiguration) fprintf(stderr," R7KRECID_7kConfiguration\n");
if (*recordid == R7KRECID_7kMatchFilter) fprintf(stderr," R7KRECID_7kMatchFilter\n");
if (*recordid == R7KRECID_7kBeamGeometry) fprintf(stderr," R7KRECID_7kBeamGeometry\n");
if (*recordid == R7KRECID_7kCalibrationData) fprintf(stderr," R7KRECID_7kCalibrationData\n");
if (*recordid == R7KRECID_7kBathymetricData) fprintf(stderr," R7KRECID_7kBathymetricData\n");
if (*recordid == R7KRECID_7kBackscatterImageData) fprintf(stderr," R7KRECID_7kBackscatterImageData\n");
if (*recordid == R7KRECID_7kBeamData) fprintf(stderr," R7KRECID_7kBeamData\n");
if (*recordid == R7KRECID_7kVerticalDepth) fprintf(stderr," R7KRECID_7kVerticalDepth\n");
if (*recordid == R7KRECID_7kImageData) fprintf(stderr," R7KRECID_7kImageData\n");
if (*recordid == R7KRECID_7kInstallationParameters) fprintf(stderr," R7KRECID_7kInstallationParameters\n");
if (*recordid == R7KRECID_7kSystemEventMessage) fprintf(stderr,"R7KRECID_7kSystemEventMessage\n");
if (*recordid == R7KRECID_7kDataStorageStatus) fprintf(stderr," R7KRECID_7kDataStorageStatus\n");
if (*recordid == R7KRECID_7kFileHeader) fprintf(stderr," R7KRECID_7kFileHeader\n");
if (*recordid == R7KRECID_7kTrigger) fprintf(stderr," R7KRECID_7kTrigger\n");
if (*recordid == R7KRECID_7kTriggerSequenceSetup) fprintf(stderr," R7KRECID_7kTriggerSequenceSetup\n");
if (*recordid == R7KRECID_7kTriggerSequenceDone) fprintf(stderr," R7KRECID_7kTriggerSequenceDone\n");
if (*recordid == R7KRECID_7kTimeMessage) fprintf(stderr," R7KRECID_7kTimeMessage\n");
if (*recordid == R7KRECID_7kRemoteControl) fprintf(stderr," R7KRECID_7kRemoteControl\n");
if (*recordid == R7KRECID_7kRemoteControlAcknowledge) fprintf(stderr," R7KRECID_7kRemoteControlAcknowledge\n");
if (*recordid == R7KRECID_7kRemoteControlNotAcknowledge) fprintf(stderr," R7KRECID_7kRemoteControlNotAcknowledge\n");
if (*recordid == R7KRECID_7kRemoteControlSonarSettings) fprintf(stderr," R7KRECID_7kRemoteControlSonarSettings\n");
if (*recordid == R7KRECID_7kRoll) fprintf(stderr," R7KRECID_7kRoll\n");
if (*recordid == R7KRECID_7kPitch) fprintf(stderr," R7KRECID_7kPitch\n");
if (*recordid == R7KRECID_7kSoundVelocity) fprintf(stderr," R7KRECID_7kSoundVelocity\n");
if (*recordid == R7KRECID_7kAbsorptionLoss) fprintf(stderr," R7KRECID_7kAbsorptionLoss\n");
if (*recordid == R7KRECID_7kSpreadingLoss) fprintf(stderr," R7KRECID_7kSpreadingLoss\n");
if (*recordid == R7KRECID_8100SonarData) fprintf(stderr," R7KRECID_8100SonarData\n");
}
#endif

		/* set done if read failure */
		if (status == MB_FAILURE)
			{
#ifdef MBR_RESON7KR_DEBUG
	fprintf(stderr,"call nothing, read failure\n");
#endif
			done = MB_YES;
			}

		/* if needed parse the data record */
		if (status == MB_SUCCESS && done == MB_NO)
			{
			if (*recordid == R7KRECID_7kFileHeader)
				{
				status = mbr_reson7kr_rd_fileheader(verbose, buffer, store_ptr, error);
				(*fileheaders)++;
				done = MB_YES;

/* kluge to set bogus background navigation */
/*klugelon = -121.0;
klugelat = 36.0;		
mb_navint_add(verbose, mbio_ptr, 
		store->time_d, 
		klugelon, 
		klugelat, 
		error);
klugelon = -121.0;
klugelat = 37.168;
mb_navint_add(verbose, mbio_ptr, 
		store->time_d + 86400.0, 
		klugelon, 
		klugelat, 
		error);*/
				}
			else if (*recordid == R7KRECID_ReferencePoint)
				{
				status = mbr_reson7kr_rd_reference(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_UncalibratedSensorOffset)
				{
				status = mbr_reson7kr_rd_sensoruncal(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_CalibratedSensorOffset)
				{
				status = mbr_reson7kr_rd_sensorcal(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_Position)
				{
				status = mbr_reson7kr_rd_position(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_CustomAttitude)
				{
				status = mbr_reson7kr_rd_customattitude(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_Tide)
				{
				status = mbr_reson7kr_rd_tide(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_Altitude)
				{
				status = mbr_reson7kr_rd_altitude(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_MotionOverGround)
				{
				status = mbr_reson7kr_rd_motion(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_Depth)
				{
				status = mbr_reson7kr_rd_depth(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_SoundVelocityProfile)
				{
				status = mbr_reson7kr_rd_svp(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_CTD)
				{
				status = mbr_reson7kr_rd_ctd(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_Geodesy)
				{
				status = mbr_reson7kr_rd_geodesy(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_RollPitchHeave)
				{
				status = mbr_reson7kr_rd_rollpitchheave(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_Heading)
				{
				status = mbr_reson7kr_rd_heading(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_Attitude)
				{
				status = mbr_reson7kr_rd_attitude(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_FSDWsidescan
				&& *deviceid == R7KDEVID_EdgetechFSDW
				&& *enumerator == 20)
				{
				status = mbr_reson7kr_rd_fsdwsslo(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_FSDWsidescan
				&& *deviceid == R7KDEVID_EdgetechFSDWSSLF)
				{
				status = mbr_reson7kr_rd_fsdwsslo(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_FSDWsidescan
				&& *deviceid == R7KDEVID_EdgetechFSDW
				&& *enumerator == 21)
				{
				status = mbr_reson7kr_rd_fsdwsshi(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_FSDWsidescan
				&& *deviceid == R7KDEVID_EdgetechFSDWSSHF)
				{
				status = mbr_reson7kr_rd_fsdwsshi(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_FSDWsubbottom)
				{
				status = mbr_reson7kr_rd_fsdwsb(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_Bluefin)
				{
				status = mbr_reson7kr_rd_bluefin(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_7kVolatileSonarSettings)
				{
				status = mbr_reson7kr_rd_volatilesonarsettings(verbose, buffer, store_ptr, error);
				store->read_volatilesettings = MB_YES;
				}
			else if (*recordid == R7KRECID_7kConfiguration)
				{
				status = mbr_reson7kr_rd_configuration(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_7kMatchFilter)
				{
				status = mbr_reson7kr_rd_matchfilter(verbose, buffer, store_ptr, error);
				store->read_matchfilter = MB_YES;
				}
			else if (*recordid == R7KRECID_7kBeamGeometry)
				{
				status = mbr_reson7kr_rd_beamgeometry(verbose, buffer, store_ptr, error);
				store->read_beamgeometry = MB_YES;
				done = MB_NO;
				}
			else if (*recordid == R7KRECID_7kCalibrationData)
				{
				status = mbr_reson7kr_rd_calibration(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_7kBathymetricData)
				{
				status = mbr_reson7kr_rd_bathymetry(verbose, buffer, store_ptr, error);
				store->read_bathymetry = MB_YES;
				}
			else if (*recordid == R7KRECID_7kBackscatterImageData)
				{
				status = mbr_reson7kr_rd_backscatter(verbose, buffer, store_ptr, error);
				store->read_backscatter = MB_YES;
				}
			else if (*recordid == R7KRECID_7kBeamData)
				{
				status = mbr_reson7kr_rd_beam(verbose, buffer, store_ptr, error);
				store->read_beam = MB_YES;
				}
			else if (*recordid == R7KRECID_7kVerticalDepth)
				{
				status = mbr_reson7kr_rd_verticaldepth(verbose, buffer, store_ptr, error);
				store->read_verticaldepth = MB_YES;
				}
			else if (*recordid == R7KRECID_7kImageData)
				{
				status = mbr_reson7kr_rd_image(verbose, buffer, store_ptr, error);
				store->read_image = MB_YES;
				}
			else if (*recordid == R7KRECID_7kInstallationParameters)
				{
				status = mbr_reson7kr_rd_installation(verbose, buffer, store_ptr, error);
				store->read_image = MB_YES;
				}
			else if (*recordid == R7KRECID_7kSystemEventMessage)
				{
				status = mbr_reson7kr_rd_systemeventmessage(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_7kRemoteControlSonarSettings)
				{
				status = mbr_reson7kr_rd_remotecontrolsettings(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_7kRoll)
				{
				status = mbr_reson7kr_rd_roll(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_7kPitch)
				{
				status = mbr_reson7kr_rd_pitch(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_7kSoundVelocity)
				{
				status = mbr_reson7kr_rd_soundvelocity(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_7kAbsorptionLoss)
				{
				status = mbr_reson7kr_rd_absorptionloss(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_7kSpreadingLoss)
				{
				status = mbr_reson7kr_rd_spreadingloss(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else
				{
				
#ifdef MBR_RESON7KR_DEBUG
				fprintf(stderr,"Record type %d - recognized but not supported.\n");
#endif
				done = MB_NO;
				}
				
			/* check if ping record is known to be done */
			if (status == MB_SUCCESS 
				&& ping_record == MB_YES
				&& store->read_volatilesettings == MB_YES
				&& store->read_matchfilter == MB_YES
				&& store->read_beamgeometry == MB_YES
				&& store->read_bathymetry == MB_YES
				&& store->read_backscatter == MB_YES
				&& store->read_beam == MB_YES
				&& store->read_verticaldepth == MB_YES
				&& store->read_image == MB_YES)
				{
				done = MB_YES;
				*current_ping = *last_ping;
				*last_ping = -1;
				}
			}

		/* bail out if there is a parsing error */
		if (status == MB_FAILURE)
			done = MB_YES;
#ifdef MBR_RESON7KR_DEBUG
if (verbose > 0)
{
fprintf(stderr,"done:%d kind:%d recordid:%x size:%d status:%d error:%d\n", 
	done, store->kind, *recordid, *size, status, *error);
fprintf(stderr,"end of mbr_reson7kr_rd_data loop:\n\n");
}
/*if (status == MB_SUCCESS)
{
if (*save_flag == MB_YES)
fprintf(stderr,"RECORD SAVED\n");
else
fprintf(stderr,"status:%d recordid:%d ping_record:%d current:%d last:%d new:%d  done:%d recs:%d %d %d %d %d %d %d %d\n",
status,*recordid,ping_record,*current_ping,*last_ping,*new_ping,done,
store->read_volatilesettings,store->read_matchfilter,
store->read_beamgeometry,store->read_bathymetry,store->read_backscatter,
store->read_beam,store->read_verticaldepth,store->read_image);
}*/
#endif
		}
#ifdef MBR_RESON7KR_DEBUG
	if (status == MB_SUCCESS)
		fprintf(stderr,"RESON7KR DATA READ: type:%d status:%d error:%d\n\n", 
			store->kind, status, *error);
#endif
		
	/* get file position */
	if (*save_flag == MB_YES)
		mb_io_ptr->file_bytes = ftell(mbfp) - *size;
	else
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
int mbr_reson7kr_chk_header(int verbose, void *mbio_ptr, char *buffer, 
				int *recordid,  int *deviceid, unsigned short *enumerator, int *size)
{
	char	*function_name = "mbr_reson7kr_chk_label";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	*recordidptr;
	int	*deviceidptr;
	int	*sizeptr;
	unsigned short	version;
	unsigned short	offset;
	unsigned int	sync;
	unsigned short	reserved;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:        %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:      %d\n",mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	
	/* get values to check */
	mb_get_binary_short(MB_YES, &buffer[0], &version); 
	mb_get_binary_short(MB_YES, &buffer[2], &offset); 
	mb_get_binary_int(MB_YES, &buffer[4], &sync); 
	mb_get_binary_int(MB_YES, &buffer[8], size); 
	mb_get_binary_int(MB_YES, &buffer[32], recordid); 
	mb_get_binary_int(MB_YES, &buffer[36], deviceid); 
	mb_get_binary_short(MB_YES, &buffer[40], &reserved); 
	mb_get_binary_short(MB_YES, &buffer[42], enumerator); 
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
		{
		fprintf(stderr, "\nChecking header in mbr_reson7kr_chk_header:\n");
		fprintf(stderr, "Version:      %4.4hX | %d\n", version, version);
		fprintf(stderr, "Offset:       %4.4hX | %d\n", offset, offset);
		fprintf(stderr, "Sync:         %4.4hX | %d\n", sync, sync);
		fprintf(stderr, "Size:         %4.4hX | %d\n", *size, *size);
		fprintf(stderr, "Record id:    %4.4hX | %d\n", *recordid, *recordid);
		fprintf(stderr, "Device id:    %4.4hX | %d\n", *deviceid, *deviceid);
		fprintf(stderr, "Reserved:     %4.4hX | %d\n", reserved, reserved);
		fprintf(stderr, "Enumerator:   %4.4hX | %d\n", *enumerator, *enumerator);
		}
#endif

	/* reset enumerator if version 2 */
	if (version == 2)
		*enumerator = reserved;
	
	/* check sync */
	if (sync != 0x0000FFFF)
		{
		status = MB_FAILURE;
		}
		
	/* check recordid */
	else if (*recordid != R7KRECID_ReferencePoint
			&& *recordid != R7KRECID_UncalibratedSensorOffset
			&& *recordid != R7KRECID_CalibratedSensorOffset
			&& *recordid != R7KRECID_Position
			&& *recordid != R7KRECID_CustomAttitude
			&& *recordid != R7KRECID_Tide
			&& *recordid != R7KRECID_Altitude
			&& *recordid != R7KRECID_MotionOverGround
			&& *recordid != R7KRECID_Depth
			&& *recordid != R7KRECID_SoundVelocityProfile
			&& *recordid != R7KRECID_CTD
			&& *recordid != R7KRECID_Geodesy
			&& *recordid != R7KRECID_RollPitchHeave
			&& *recordid != R7KRECID_Heading
			&& *recordid != R7KRECID_Attitude
			&& *recordid != R7KRECID_FSDWsidescan
			&& *recordid != R7KRECID_FSDWsubbottom
			&& *recordid != R7KRECID_Bluefin
			&& *recordid != R7KRECID_7kVolatileSonarSettings
			&& *recordid != R7KRECID_7kConfiguration
			&& *recordid != R7KRECID_7kMatchFilter
			&& *recordid != R7KRECID_7kBeamGeometry
			&& *recordid != R7KRECID_7kCalibrationData
			&& *recordid != R7KRECID_7kBathymetricData
			&& *recordid != R7KRECID_7kBackscatterImageData
			&& *recordid != R7KRECID_7kBeamData
			&& *recordid != R7KRECID_7kVerticalDepth
			&& *recordid != R7KRECID_7kImageData
			&& *recordid != R7KRECID_7kInstallationParameters
			&& *recordid != R7KRECID_7kSystemEventMessage
			&& *recordid != R7KRECID_7kDataStorageStatus
			&& *recordid != R7KRECID_7kFileHeader
			&& *recordid != R7KRECID_7kTrigger
			&& *recordid != R7KRECID_7kTriggerSequenceSetup
			&& *recordid != R7KRECID_7kTriggerSequenceDone
			&& *recordid != R7KRECID_7kTimeMessage
			&& *recordid != R7KRECID_7kRemoteControl
			&& *recordid != R7KRECID_7kRemoteControlAcknowledge
			&& *recordid != R7KRECID_7kRemoteControlNotAcknowledge
			&& *recordid != R7KRECID_7kRemoteControlSonarSettings
			&& *recordid != R7KRECID_7kRoll	
			&& *recordid != R7KRECID_7kPitch
			&& *recordid != R7KRECID_7kSoundVelocity
			&& *recordid != R7KRECID_7kAbsorptionLoss
			&& *recordid != R7KRECID_7kSpreadingLoss
			&& *recordid != R7KRECID_8100SonarData)
		{
		status = MB_FAILURE;
		}
	else
		{
		status = MB_SUCCESS;

#ifdef MBR_RESON7KR_DEBUG
		if (verbose > 0)
			{
			fprintf(stderr, "Good record id: %4.4hX | %d", *recordid, *recordid);
			if (*recordid == R7KRECID_ReferencePoint) fprintf(stderr," R7KRECID_ReferencePoint\n");
			if (*recordid == R7KRECID_UncalibratedSensorOffset) fprintf(stderr," R7KRECID_UncalibratedSensorOffset\n");
			if (*recordid == R7KRECID_CalibratedSensorOffset) fprintf(stderr," R7KRECID_CalibratedSensorOffset\n");
			if (*recordid == R7KRECID_Position) fprintf(stderr," R7KRECID_Position\n");
			if (*recordid == R7KRECID_CustomAttitude) fprintf(stderr," R7KRECID_CustomAttitude\n");
			if (*recordid == R7KRECID_Tide) fprintf(stderr," R7KRECID_Tide\n");
			if (*recordid == R7KRECID_Altitude) fprintf(stderr," R7KRECID_Altitude\n");
			if (*recordid == R7KRECID_MotionOverGround) fprintf(stderr," R7KRECID_MotionOverGround\n");
			if (*recordid == R7KRECID_Depth) fprintf(stderr," R7KRECID_Depth\n");
			if (*recordid == R7KRECID_SoundVelocityProfile) fprintf(stderr," R7KRECID_SoundVelocityProfile\n");
			if (*recordid == R7KRECID_CTD) fprintf(stderr," R7KRECID_CTD\n");
			if (*recordid == R7KRECID_Geodesy) fprintf(stderr," R7KRECID_Geodesy\n");
			if (*recordid == R7KRECID_RollPitchHeave) fprintf(stderr," R7KRECID_RollPitchHeave\n");
			if (*recordid == R7KRECID_Heading) fprintf(stderr," R7KRECID_Heading\n");
			if (*recordid == R7KRECID_Attitude) fprintf(stderr," R7KRECID_Attitude\n");
			if (*recordid == R7KRECID_FSDWsidescan) fprintf(stderr," R7KRECID_FSDWsidescan\n");
			if (*recordid == R7KRECID_FSDWsubbottom) fprintf(stderr," R7KRECID_FSDWsubbottom\n");
			if (*recordid == R7KRECID_Bluefin) fprintf(stderr," R7KRECID_Bluefin\n");
			if (*recordid == R7KRECID_7kVolatileSonarSettings) fprintf(stderr," R7KRECID_7kVolatileSonarSettings\n");
			if (*recordid == R7KRECID_7kConfiguration) fprintf(stderr," R7KRECID_7kConfiguration\n");
			if (*recordid == R7KRECID_7kMatchFilter) fprintf(stderr," R7KRECID_7kMatchFilter\n");
			if (*recordid == R7KRECID_7kBeamGeometry) fprintf(stderr," R7KRECID_7kBeamGeometry\n");
			if (*recordid == R7KRECID_7kCalibrationData) fprintf(stderr," R7KRECID_7kCalibrationData\n");
			if (*recordid == R7KRECID_7kBathymetricData) fprintf(stderr," R7KRECID_7kBathymetricData\n");
			if (*recordid == R7KRECID_7kBackscatterImageData) fprintf(stderr," R7KRECID_7kBackscatterImageData\n");
			if (*recordid == R7KRECID_7kBeamData) fprintf(stderr," R7KRECID_7kBeamData\n");
			if (*recordid == R7KRECID_7kVerticalDepth) fprintf(stderr," R7KRECID_7kVerticalDepth\n");
			if (*recordid == R7KRECID_7kImageData) fprintf(stderr," R7KRECID_7kImageData\n");
			if (*recordid == R7KRECID_7kInstallationParameters) fprintf(stderr," R7KRECID_7kInstallationParameters\n");
			if (*recordid == R7KRECID_7kSystemEventMessage) fprintf(stderr,"R7KRECID_7kSystemEventMessage\n");
			if (*recordid == R7KRECID_7kDataStorageStatus) fprintf(stderr," R7KRECID_7kDataStorageStatus\n");
			if (*recordid == R7KRECID_7kFileHeader) fprintf(stderr," R7KRECID_7kFileHeader\n");
			if (*recordid == R7KRECID_7kTrigger) fprintf(stderr," R7KRECID_7kTrigger\n");
			if (*recordid == R7KRECID_7kTriggerSequenceSetup) fprintf(stderr," R7KRECID_7kTriggerSequenceSetup\n");
			if (*recordid == R7KRECID_7kTriggerSequenceDone) fprintf(stderr," R7KRECID_7kTriggerSequenceDone\n");
			if (*recordid == R7KRECID_7kTimeMessage) fprintf(stderr," R7KRECID_7kTimeMessage\n");
			if (*recordid == R7KRECID_7kRemoteControl) fprintf(stderr," R7KRECID_7kRemoteControl\n");
			if (*recordid == R7KRECID_7kRemoteControlAcknowledge) fprintf(stderr," R7KRECID_7kRemoteControlAcknowledge\n");
			if (*recordid == R7KRECID_7kRemoteControlNotAcknowledge) fprintf(stderr," R7KRECID_7kRemoteControlNotAcknowledge\n");
			if (*recordid == R7KRECID_7kRemoteControlSonarSettings) fprintf(stderr," R7KRECID_7kRemoteControlSonarSettings\n");
			if (*recordid == R7KRECID_7kRoll) fprintf(stderr," R7KRECID_7kRoll\n");
			if (*recordid == R7KRECID_7kPitch) fprintf(stderr," R7KRECID_7kPitch\n");
			if (*recordid == R7KRECID_7kSoundVelocity) fprintf(stderr," R7KRECID_7kSoundVelocity\n");
			if (*recordid == R7KRECID_7kAbsorptionLoss) fprintf(stderr," R7KRECID_7kAbsorptionLoss\n");
			if (*recordid == R7KRECID_7kSpreadingLoss) fprintf(stderr," R7KRECID_7kSpreadingLoss\n");
			if (*recordid == R7KRECID_8100SonarData) fprintf(stderr," R7KRECID_8100SonarData\n");
			}
#endif
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Output arguments:\n");
		fprintf(stderr,"dbg2       recordid:      %d\n",*recordid);
		fprintf(stderr,"dbg2       deviceid:      %d\n",*deviceid);
		fprintf(stderr,"dbg2       enumerator:    %d\n",*enumerator);
		fprintf(stderr,"dbg2       size:          %d\n",*size);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:        %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_chk_pingnumber(int verbose, int recordid, char *buffer, 
				int *ping_number)
{
	char	*function_name = "mbr_reson7kr_chk_pingnumber";
	int	status = MB_SUCCESS;
	unsigned short	offset;
	int	index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:        %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       recordid:      %d\n",recordid);
		fprintf(stderr,"dbg2       buffer:        %d\n",buffer);
		}

	/* get offset to data section */
	mb_get_binary_short(MB_YES, &buffer[2], &offset);

	/* check ping number if one of the ping records */
	if (recordid == R7KRECID_7kVolatileSonarSettings)
		{
		index = offset + 12;
		mb_get_binary_int(MB_YES, &buffer[index], ping_number);
		status = MB_SUCCESS;
		}
	else if (recordid == R7KRECID_7kMatchFilter)
		{
		index = offset + 12;
		mb_get_binary_int(MB_YES, &buffer[index], ping_number);
		status = MB_SUCCESS;
		}
	else if (recordid == R7KRECID_7kBathymetricData)
		{
		index = offset + 12;
		mb_get_binary_int(MB_YES, &buffer[index], ping_number);
		status = MB_SUCCESS;
		}
	else if (recordid == R7KRECID_7kBackscatterImageData)
		{
		index = offset + 12;
		mb_get_binary_int(MB_YES, &buffer[index], ping_number);
		status = MB_SUCCESS;
		}
	else if (recordid == R7KRECID_7kBeamData)
		{
		index = offset + 12;
		mb_get_binary_int(MB_YES, &buffer[index], ping_number);
		status = MB_SUCCESS;
		}
	else if (recordid == R7KRECID_7kVerticalDepth)
		{
		index = offset + 8;
		mb_get_binary_int(MB_YES, &buffer[index], ping_number);
		status = MB_SUCCESS;
		}
	else if (recordid == R7KRECID_7kImageData)
		{
		index = offset + 4;
		mb_get_binary_int(MB_YES, &buffer[index], ping_number);
		status = MB_SUCCESS;
		}
	else
		{
		status = MB_FAILURE;
		*ping_number = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Output arguments:\n");
		fprintf(stderr,"dbg2       ping_number:   %d\n",*ping_number);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:        %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_header(int verbose, char *buffer, int *index, 
		s7k_header *header, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_header";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       index:      %d\n",*index);
		fprintf(stderr,"dbg2       header:     %d\n",header);
		}
	
	/* extract the header */
	mb_get_binary_short(MB_YES, &buffer[*index], &(header->Version)); *index +=2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(header->Offset)); *index +=2;
	mb_get_binary_int(MB_YES, &buffer[*index], &(header->SyncPattern)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(header->Size)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(header->OffsetToOptionalData)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(header->OptionalDataIdentifier)); *index += 4;
	mb_get_binary_short(MB_YES, &buffer[*index], &(header->s7kTime.Year)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(header->s7kTime.Day)); *index += 2;
	mb_get_binary_float(MB_YES, &buffer[*index], &(header->s7kTime.Seconds)); *index += 4;
	header->s7kTime.Hours = (mb_u_char) buffer[*index]; (*index)++;
	header->s7kTime.Minutes = (mb_u_char) buffer[*index]; (*index)++;
	mb_get_binary_short(MB_YES, &buffer[*index], &(header->Reserved)); *index += 2;
	mb_get_binary_int(MB_YES, &buffer[*index], &(header->RecordType)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(header->DeviceId)); *index += 4;
	mb_get_binary_short(MB_YES, &buffer[*index], &(header->Reserved2)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(header->SystemEnumerator)); *index += 2;
	if (header->Version == 2)
		header->SystemEnumerator = header->Reserved2;
	if (header->Version == 2)
		mb_get_binary_int(MB_YES, &buffer[*index], &(header->DataSetNumber)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(header->RecordNumber)); *index += 4;
	if (header->Version == 2)
		{
		for (i=0;i<8;i++)
			{
			header->PreviousRecord[i] = buffer[*index]; (*index)++;
			}
		for (i=0;i<8;i++)
			{
			header->NextRecord[i] = buffer[*index]; (*index)++;
			}
		}
	mb_get_binary_short(MB_YES, &buffer[*index], &(header->Flags)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(header->Reserved3)); *index += 2;
	if (header->Version == 2)
		{
		mb_get_binary_int(MB_YES, &buffer[*index], &(header->Reserved4)); *index += 4;
		mb_get_binary_int(MB_YES, &buffer[*index], &(header->FragmentedTotal)); *index += 4;
		mb_get_binary_int(MB_YES, &buffer[*index], &(header->FragmentNumber)); *index += 4;
		}
	
	/* print out the results */
	/* mbsys_reson7k_print_header(verbose, header, error); */

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       index:      %d\n",*index);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_reference(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_reference";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_reference *reference;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	reference = &(store->reference);
	header = &(reference->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(reference->offset_x)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(reference->offset_y)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(reference->offset_z)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(reference->water_z)); index += 4;
		
	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_PARAMETER;
		store->type = R7KRECID_ReferencePoint;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_ReferencePoint:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_reference(verbose, reference, error);	

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
int mbr_reson7kr_rd_sensoruncal(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_sensoruncal";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_sensoruncal *sensoruncal;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	sensoruncal = &(store->sensoruncal);
	header = &(sensoruncal->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sensoruncal->offset_x)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sensoruncal->offset_y)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sensoruncal->offset_z)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sensoruncal->offset_roll)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sensoruncal->offset_pitch)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sensoruncal->offset_yaw)); index += 4;
		
	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_PARAMETER;
		store->type = R7KRECID_UncalibratedSensorOffset;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_UncalibratedSensorOffset: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_sensoruncal(verbose, sensoruncal, error);

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
int mbr_reson7kr_rd_sensorcal(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_sensorcal";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_sensorcal *sensorcal;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	sensorcal = &(store->sensorcal);
	header = &(sensorcal->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sensorcal->offset_x)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sensorcal->offset_y)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sensorcal->offset_z)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sensorcal->offset_roll)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sensorcal->offset_pitch)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sensorcal->offset_yaw)); index += 4;
		
	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_PARAMETER;
		store->type = R7KRECID_CalibratedSensorOffset;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_CalibratedSensorOffset:   7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_sensorcal(verbose, sensorcal, error);

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
int mbr_reson7kr_rd_position(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_position";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_position *position;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	position = &(store->position);
	header = &(position->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(position->datum)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(position->latency)); index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &(position->latitude)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(position->longitude)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(position->height)); index += 8;
	position->type = buffer[index]; index++;
	position->utm_zone = buffer[index]; index++;
	position->quality = buffer[index]; index++;
	position->method = buffer[index]; index++;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_NAV;
		store->type = R7KRECID_Position;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_Position:                 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_position(verbose, position, error);

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
int mbr_reson7kr_rd_customattitude(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_customattitude";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_customattitude *customattitude;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	customattitude = &(store->customattitude);
	header = &(customattitude->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	customattitude->bitfield = (mb_u_char) buffer[index]; index++;
	customattitude->reserved = (mb_u_char) buffer[index]; index++;
	mb_get_binary_short(MB_YES, &buffer[index], &(customattitude->n)); index += 2;
	mb_get_binary_float(MB_YES, &buffer[index], &(customattitude->frequency)); index += 4;
		
	/* make sure enough memory is allocated for channel data */
	if (customattitude->nalloc < customattitude->n)
		{
		data_size = customattitude->n * sizeof(float);
		status = mb_realloc(verbose, data_size, &(customattitude->pitch), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(customattitude->roll), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(customattitude->heading), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(customattitude->heave), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(customattitude->pitchrate), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(customattitude->rollrate), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(customattitude->headingrate), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(customattitude->heaverate), error);
		if (status == MB_SUCCESS)
			{
			customattitude->nalloc = customattitude->n;
			}
		else
			{
			customattitude->nalloc = 0;
			customattitude->n = 0;
			}
		}

	if (customattitude->bitfield & 1)
	for (i=0;i<customattitude->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(customattitude->pitch[i])); index += 4;
		}
	if (customattitude->bitfield & 2)
	for (i=0;i<customattitude->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(customattitude->roll[i])); index += 4;
		}
	if (customattitude->bitfield & 4)
	for (i=0;i<customattitude->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(customattitude->heading[i])); index += 4;
		}
	if (customattitude->bitfield & 8)
	for (i=0;i<customattitude->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(customattitude->heave[i])); index += 4;
		}
	if (customattitude->bitfield & 16)
	for (i=0;i<customattitude->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(customattitude->pitchrate[i])); index += 4;
		}
	if (customattitude->bitfield & 32)
	for (i=0;i<customattitude->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(customattitude->rollrate[i])); index += 4;
		}
	if (customattitude->bitfield & 64)
	for (i=0;i<customattitude->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(customattitude->headingrate[i])); index += 4;
		}
	if (customattitude->bitfield & 128)
	for (i=0;i<customattitude->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(customattitude->heaverate[i])); index += 4;
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_ATTITUDE;
		store->type = R7KRECID_CustomAttitude;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_CustomAttitude:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_customattitude(verbose, customattitude, error);

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
int mbr_reson7kr_rd_tide(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_tide";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_tide *tide;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	tide = &(store->tide);
	header = &(tide->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(tide->tide)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(tide->source)); index += 2;
	tide->flags = buffer[index]; index++;
	mb_get_binary_short(MB_YES, &buffer[index], &(tide->gauge)); index += 2;
	mb_get_binary_int(MB_YES, &buffer[index], &(tide->datum)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(tide->latency)); index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &(tide->latitude)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(tide->longitude)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(tide->height)); index += 8;
	tide->type = buffer[index]; index++;
	tide->utm_zone = buffer[index]; index++;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_TIDE;
		store->type = R7KRECID_Tide;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_Tide:                     7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_tide(verbose, tide, error);

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
int mbr_reson7kr_rd_altitude(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_altitude";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_altitude *altitude;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	altitude = &(store->altitude);
	header = &(altitude->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(altitude->altitude)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_HEIGHT;
		store->type = R7KRECID_Altitude;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_Altitude:                 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_altitude(verbose, altitude, error);

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
int mbr_reson7kr_rd_motion(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_motion";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_motion *motion;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	motion = &(store->motion);
	header = &(motion->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	motion->bitfield = (mb_u_char) buffer[index]; index++;
	motion->reserved = (mb_u_char) buffer[index]; index++;
	mb_get_binary_short(MB_YES, &buffer[index], &(motion->n)); index += 2;
	mb_get_binary_float(MB_YES, &buffer[index], &(motion->frequency)); index += 4;
		
	/* make sure enough memory is allocated for channel data */
	if (motion->nalloc < motion->n)
		{
		data_size = motion->n * sizeof(float);
		status = mb_realloc(verbose, data_size, &(motion->x), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(motion->y), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(motion->z), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(motion->xa), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(motion->ya), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(motion->za), error);
		if (status == MB_SUCCESS)
			{
			motion->nalloc = motion->n;
			}
		else
			{
			motion->nalloc = 0;
			motion->n = 0;
			}
		}

	if (motion->bitfield & 1)
		{
		for (i=0;i<motion->n;i++)
			{
			mb_get_binary_float(MB_YES, &buffer[index], &(motion->x[i])); index += 4;
			}
		for (i=0;i<motion->n;i++)
			{
			mb_get_binary_float(MB_YES, &buffer[index], &(motion->y[i])); index += 4;
			}
		for (i=0;i<motion->n;i++)
			{
			mb_get_binary_float(MB_YES, &buffer[index], &(motion->z[i])); index += 4;
			}
		}
	if (motion->bitfield & 2)
		{
		for (i=0;i<motion->n;i++)
			{
			mb_get_binary_float(MB_YES, &buffer[index], &(motion->xa[i])); index += 4;
			}
		for (i=0;i<motion->n;i++)
			{
			mb_get_binary_float(MB_YES, &buffer[index], &(motion->ya[i])); index += 4;
			}
		for (i=0;i<motion->n;i++)
			{
			mb_get_binary_float(MB_YES, &buffer[index], &(motion->za[i])); index += 4;
			}
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_MOTION;
		store->type = R7KRECID_MotionOverGround;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_MotionOverGround:         7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_motion(verbose, motion, error);

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
int mbr_reson7kr_rd_depth(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_depth";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_depth *depth;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	depth = &(store->depth);
	header = &(depth->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	depth->descriptor = (mb_u_char) buffer[index]; index++;
	depth->correction = (mb_u_char) buffer[index]; index++;
	mb_get_binary_short(MB_YES, &buffer[index], &(depth->reserved)); index += 2;
	mb_get_binary_float(MB_YES, &buffer[index], &(depth->depth)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_HEIGHT;
		store->type = R7KRECID_Depth;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_Depth:                    7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_depth(verbose, depth, error);

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
int mbr_reson7kr_rd_svp(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_svp";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_svp *svp;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	svp = &(store->svp);
	header = &(svp->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	svp->position_flag = (mb_u_char) buffer[index]; index++;
	svp->reserved1 = (mb_u_char) buffer[index]; index++;
	mb_get_binary_short(MB_YES, &buffer[index], &(svp->reserved2)); index += 2;
	mb_get_binary_double(MB_YES, &buffer[index], &(svp->latitude)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(svp->longitude)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(svp->n)); index += 4;
		
	/* make sure enough memory is allocated for channel data */
	if (svp->nalloc < svp->n)
		{
		data_size = svp->n * sizeof(float);
		status = mb_realloc(verbose, data_size, &(svp->depth), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(svp->sound_velocity), error);
		if (status == MB_SUCCESS)
			{
			svp->nalloc = svp->n;
			}
		else
			{
			svp->nalloc = 0;
			svp->n = 0;
			}
		}

	for (i=0;i<svp->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(svp->depth[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(svp->sound_velocity[i])); index += 4;
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_VELOCITY_PROFILE;
		store->type = R7KRECID_SoundVelocityProfile;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_SoundVelocityProfile:     7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_svp(verbose, svp, error);

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
int mbr_reson7kr_rd_ctd(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_ctd";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_ctd *ctd;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	ctd = &(store->ctd);
	header = &(ctd->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(ctd->frequency)); index += 4;
	ctd->velocity_source_flag = (mb_u_char) buffer[index]; index++;
	ctd->velocity_algorithm = (mb_u_char) buffer[index]; index++;
	ctd->conductivity_flag = (mb_u_char) buffer[index]; index++;
	ctd->pressure_flag = (mb_u_char) buffer[index]; index++;
	ctd->position_flag = (mb_u_char) buffer[index]; index++;
	ctd->validity = (mb_u_char) buffer[index]; index++;
	mb_get_binary_short(MB_YES, &buffer[index], &(ctd->reserved)); index += 2;
	mb_get_binary_double(MB_YES, &buffer[index], &(ctd->latitude)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ctd->longitude)); index += 8;
	mb_get_binary_float(MB_YES, &buffer[index], &(ctd->sample_rate)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(ctd->n)); index += 4;
		
	/* make sure enough memory is allocated for channel data */
	if (ctd->nalloc < ctd->n)
		{
		data_size = ctd->n * sizeof(float);
		status = mb_realloc(verbose, data_size, &(ctd->conductivity_salinity), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(ctd->temperature), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(ctd->pressure_depth), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(ctd->sound_velocity), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(ctd->absorption), error);
		if (status == MB_SUCCESS)
			{
			ctd->nalloc = ctd->n;
			}
		else
			{
			ctd->nalloc = 0;
			ctd->n = 0;
			}
		}

	for (i=0;i<ctd->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(ctd->conductivity_salinity[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(ctd->temperature[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(ctd->pressure_depth[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(ctd->sound_velocity[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(ctd->absorption[i])); index += 4;
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_CTD;
		store->type = R7KRECID_CTD;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_CTD:                      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_ctd(verbose, ctd, error);

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
int mbr_reson7kr_rd_geodesy(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_geodesy";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_geodesy *geodesy;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	geodesy = &(store->geodesy);
	header = &(geodesy->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	for (i=0;i<32;i++)
		{
		geodesy->spheroid[i] = (mb_u_char) buffer[index]; index++;
		}
	mb_get_binary_double(MB_YES, &buffer[index], &(geodesy->semimajoraxis)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(geodesy->flattening)); index += 8;
	for (i=0;i<16;i++)
		{
		geodesy->reserved1[i] = (mb_u_char) buffer[index]; index++;
		}
	for (i=0;i<32;i++)
		{
		geodesy->datum[i] = (mb_u_char) buffer[index]; index++;
		}
	mb_get_binary_int(MB_YES, &buffer[index], &(geodesy->calculation_method)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(geodesy->number_parameters)); index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &(geodesy->dx)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(geodesy->dy)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(geodesy->dz)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(geodesy->rx)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(geodesy->ry)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(geodesy->rz)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(geodesy->scale)); index += 8;
	for (i=0;i<35;i++)
		{
		geodesy->reserved2[i] = (mb_u_char) buffer[index]; index++;
		}
	for (i=0;i<32;i++)
		{
		geodesy->grid_name[i] = (mb_u_char) buffer[index]; index++;
		}
	geodesy->distance_units = (mb_u_char) buffer[index]; index++;
	geodesy->angular_units = (mb_u_char) buffer[index]; index++;
	mb_get_binary_double(MB_YES, &buffer[index], &(geodesy->latitude_origin)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(geodesy->central_meriidan)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(geodesy->false_easting)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(geodesy->false_northing)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(geodesy->central_scale_factor)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(geodesy->custum_identifier)); index += 4;
	for (i=0;i<50;i++)
		{
		geodesy->reserved3[i] = (mb_u_char) buffer[index]; index++;
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_PARAMETER;
		store->type = R7KRECID_Geodesy;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_Geodesy:                  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_geodesy(verbose, geodesy, error);

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
int mbr_reson7kr_rd_rollpitchheave(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_rollpitchheave";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_rollpitchheave *rollpitchheave;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	rollpitchheave = &(store->rollpitchheave);
	header = &(rollpitchheave->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(rollpitchheave->roll)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(rollpitchheave->pitch)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(rollpitchheave->heave)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_ATTITUDE;
		store->type = R7KRECID_RollPitchHeave;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_RollPitchHeave:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_rollpitchheave(verbose, rollpitchheave, error);

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
int mbr_reson7kr_rd_heading(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_heading";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_heading *heading;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	heading = &(store->heading);
	header = &(heading->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(heading->heading)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_HEADING;
		store->type = R7KRECID_Heading;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_Heading:                  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_heading(verbose, heading, error);

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
int mbr_reson7kr_rd_attitude(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_attitude";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_attitude *attitude;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	attitude = &(store->attitude);
	header = &(attitude->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	attitude->n = (mb_u_char) buffer[index]; index++;
		
	/* make sure enough memory is allocated for channel data */
	if (attitude->nalloc < attitude->n)
		{
		data_size = attitude->n * sizeof(unsigned short);
		status = mb_realloc(verbose, data_size, &(attitude->delta_time), error);
		data_size = attitude->n * sizeof(float);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(attitude->roll), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(attitude->pitch), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(attitude->heave), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(attitude->heading), error);
		if (status == MB_SUCCESS)
			{
			attitude->nalloc = attitude->n;
			}
		else
			{
			attitude->nalloc = 0;
			attitude->n = 0;
			}
		}

	for (i=0;i<attitude->n;i++)
		{
		mb_get_binary_short(MB_YES, &buffer[index], &(attitude->delta_time[i])); index += 2;
		mb_get_binary_float(MB_YES, &buffer[index], &(attitude->roll[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(attitude->pitch[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(attitude->heave[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(attitude->heading[i])); index += 4;
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_ATTITUDE;
		store->type = R7KRECID_Attitude;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_Attitude:                 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_attitude(verbose, attitude, error);

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
int mbr_reson7kr_rd_fsdwchannel(int verbose, int data_format, char *buffer, int *index, 
		s7k_fsdwchannel *fsdwchannel, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_fsdwchannel";
	int	status = MB_SUCCESS;
	int	data_size;
	short	*shortptr;
	unsigned short	*ushortptr;
	int	i;
short *srptr,*ssptr;
unsigned short *urptr,*usptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       data_format:%d\n",data_format);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       index:      %d\n",*index);
		fprintf(stderr,"dbg2       fsdwchannel:%d\n",fsdwchannel);
		}
	
	/* extract the channel header */
	fsdwchannel->number = (mb_u_char) buffer[*index]; (*index)++;
	fsdwchannel->type = (mb_u_char) buffer[*index]; (*index)++;
	fsdwchannel->data_type = (mb_u_char) buffer[*index]; (*index)++;
	fsdwchannel->polarity = (mb_u_char) buffer[*index]; (*index)++;
	fsdwchannel->bytespersample = (mb_u_char) buffer[*index]; (*index)++;
	for (i=0;i<3;i++)
		{
		fsdwchannel->reserved1[i] = buffer[*index]; (*index)++;
		}
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwchannel->number_samples)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwchannel->start_time)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwchannel->sample_interval)); *index += 4;
	mb_get_binary_float(MB_YES, &buffer[*index], &(fsdwchannel->range)); *index += 4;
	mb_get_binary_float(MB_YES, &buffer[*index], &(fsdwchannel->voltage)); *index += 4;
	for (i=0;i<16;i++)
		{
		fsdwchannel->name[i] = buffer[*index]; (*index)++;
		}
	for (i=0;i<20;i++)
		{
		fsdwchannel->reserved2[i] = buffer[*index]; (*index)++;
		}
		
	/* make sure enough memory is allocated for channel data */
	data_size = fsdwchannel->bytespersample * fsdwchannel->number_samples;
	if (fsdwchannel->data_alloc < data_size)
		{
		status = mb_realloc(verbose, data_size, &(fsdwchannel->data), error);
		if (status != MB_SUCCESS)
			fsdwchannel->data_alloc = 0;
		else
			fsdwchannel->data_alloc = data_size;
		}
			
	/* copy over the data */
	if (status == MB_SUCCESS)
		{
		if (fsdwchannel->bytespersample == 1)
			{
			for (i=0;i<fsdwchannel->number_samples;i++)
				{
				fsdwchannel->data[i] = buffer[*index]; (*index)++;
				}
			}
		else if (fsdwchannel->bytespersample == 2)
			{
			shortptr = (short *) fsdwchannel->data;
			for (i=0;i<fsdwchannel->number_samples;i++)
				{
/*srptr = (short *) &(buffer[*index]);
urptr = (unsigned short *) &(buffer[*index]);*/
				mb_get_binary_short(MB_YES, &(buffer[*index]), &(shortptr[i])); *index += 2;
/*ssptr = (short *) &(shortptr[i]);
usptr = (unsigned short *) &(shortptr[i]);
fprintf(stderr,"sample:%5d   raw:%6d %6d   swapped:%6d %6d\n",
i,*srptr,*urptr,*ssptr,*usptr);*/
				}
			}
		else if (fsdwchannel->bytespersample == 4)
			{
			shortptr = (short *) fsdwchannel->data;
			for (i=0;i<fsdwchannel->number_samples;i++)
				{
/*srptr = (short *) &(buffer[*index]);
urptr = (unsigned short *) &(buffer[*index]);*/
				mb_get_binary_short(MB_YES, &(buffer[*index]), &(shortptr[2*i])); *index += 2;
/*ssptr = (short *) &(shortptr[2*i]);
usptr = (unsigned short *) &(shortptr[2*i]);
fprintf(stderr,"sample:%5d   IMAGINARY: raw:%6d %6d   swapped:%6d %6d",
i,*srptr,*urptr,*ssptr,*usptr);
srptr = (short *) &(buffer[*index]);
urptr = (unsigned short *) &(buffer[*index]);*/
				mb_get_binary_short(MB_YES, &(buffer[*index]), &(shortptr[2*i+1])); *index += 2;
/*ssptr = (short *) &(shortptr[2*i+1]);
usptr = (unsigned short *) &(shortptr[2*i+1]);
fprintf(stderr,"    REAL: raw:%6d %6d   swapped:%6d %6d\n",
*srptr,*urptr,*ssptr,*usptr);*/
				}
			}
		}
	
	/* print out the results */
	/*mbsys_reson7k_print_fsdwchannel(verbose, data_format, fsdwchannel, error);*/

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       index:      %d\n",*index);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_fsdwssheader(int verbose, char *buffer, int *index, 
		s7k_fsdwssheader *fsdwssheader, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_fsdwssheader";
	int	status = MB_SUCCESS;
	int	data_size;
	short	*shortptr;
	int	*intptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:         %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:         %d\n",buffer);
		fprintf(stderr,"dbg2       index:          %d\n",*index);
		fprintf(stderr,"dbg2       fsdwssheader:   %d\n",fsdwssheader);
		}
	
	/* extract the Edgetech sidescan header */
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->subsystem)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->channelNum)); *index += 2;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwssheader->pingNum)); *index += 4;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->packetNum)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->trigSource)); *index += 2;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwssheader->samples)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwssheader->sampleInterval)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwssheader->startDepth)); *index += 4;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->weightingFactor)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->ADCGain)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->ADCMax)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->rangeSetting)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->pulseID)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->markNumber)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->dataFormat)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->reserved)); *index += 2;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwssheader->millisecondsToday)); *index += 4;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->year)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->day)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->hour)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->minute)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->second)); *index += 2;   
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->heading)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->pitch)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->roll)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->heave)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->yaw)); *index += 2;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwssheader->depth)); *index += 4;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwssheader->temperature)); *index += 2;
	for (i=0;i<10;i++)
		{
		fsdwssheader->reserved2[i] = buffer[*index]; (*index)++;
		}
	
	/* print out the results */
	/*mbsys_reson7k_print_fsdwssheader(verbose, fsdwssheader, error);*/

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       index:      %d\n",*index);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_fsdwsegyheader(int verbose, char *buffer, int *index, 
		s7k_fsdwsegyheader *fsdwsegyheader, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_fsdwsegyheader";
	int	status = MB_SUCCESS;
	int	data_size;
	short	*shortptr;
	int	*intptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:         %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:         %d\n",buffer);
		fprintf(stderr,"dbg2       index:          %d\n",*index);
		fprintf(stderr,"dbg2       fsdwsegyheader: %d\n",fsdwsegyheader);
		}
	
	/* extract the Edgetech segy header */
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwsegyheader->sequenceNumber)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwsegyheader->startDepth)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwsegyheader->pingNum)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwsegyheader->channelNum)); *index += 4;
	for (i=0;i<6;i++)
		{
		mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->unused1[i])); *index += 2;
		}
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->traceIDCode)); *index += 2;
	for (i=0;i<2;i++)
		{
		mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->unused2[i])); *index += 2;
		}
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->dataFormat)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->NMEAantennaeR)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->NMEAantennaeO)); *index += 2;
	for (i=0;i<32;i++)
		{
		fsdwsegyheader->RS232[i] = buffer[*index]; (*index)++;
		}
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwsegyheader->sourceCoordX)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwsegyheader->sourceCoordY)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwsegyheader->groupCoordX)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwsegyheader->groupCoordY)); *index += 4;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->coordUnits)); *index += 2;
	for (i=0;i<24;i++)
		{
		fsdwsegyheader->annotation[i] = buffer[*index]; (*index)++;
		}
 	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->samples)); *index += 2;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwsegyheader->sampleInterval)); *index += 4;
 	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->ADCGain)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->pulsePower)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->correlated)); *index += 2;
 	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->startFreq)); *index += 2;
 	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->endFreq)); *index += 2;
 	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->sweepLength)); *index += 2;
	for (i=0;i<4;i++)
		{
		mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->unused7[i])); *index += 2;
		}
 	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->aliasFreq)); *index += 2;
 	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->pulseID)); *index += 2;
	for (i=0;i<6;i++)
		{
		mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->unused8[i])); *index += 2;
		}
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->year)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->day)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->hour)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->minute)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->second)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->timeBasis)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->weightingFactor)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->unused9)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->heading)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->pitch)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->roll)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->temperature)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->heaveCompensation)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->trigSource)); *index += 2;
 	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->markNumber)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->NMEAHour)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->NMEAMinutes)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->NMEASeconds)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->NMEACourse)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->NMEASpeed)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->NMEADay)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->NMEAYear)); *index += 2;
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwsegyheader->millisecondsToday)); *index += 4;
 	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->ADCMax)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->calConst)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->vehicleID)); *index += 2;
	for (i=0;i<6;i++)
		{
		fsdwsegyheader->softwareVersion[i] = buffer[*index]; (*index)++;
		}
	mb_get_binary_int(MB_YES, &buffer[*index], &(fsdwsegyheader->sphericalCorrection)); *index += 4;
 	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->packetNum)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->ADCDecimation)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->decimation)); *index += 2;
	for (i=0;i<7;i++)
		{
		mb_get_binary_short(MB_YES, &buffer[*index], &(fsdwsegyheader->unuseda[i])); *index += 2;
		}
	
	/* print out the results */
	/*mbsys_reson7k_print_fsdwsegyheader(verbose, fsdwsegyheader, error);*/

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       index:      %d\n",*index);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_fsdwsslo(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_fsdwsslo";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_fsdwss *fsdwsslo;
	s7k_fsdwchannel *fsdwchannel;
	s7k_fsdwssheader *fsdwssheader;
	s7kr_bathymetry *bathymetry;
	s7kr_bluefin *bluefin;
	double	bin[MBSYS_RESON7K_MAX_PIXELS];
	int	nbin[MBSYS_RESON7K_MAX_PIXELS];
	int	index;
	int	time_j[5];
	int 	bottompick;
	double	range, sign, sound_speed, half_sound_speed;
	double	tracemax;
	double	xtrack;
	double	pixelsize;
	double	pickvalue, value;
	short	*shortptr;
	unsigned short	*ushortptr;
	int	jstart, jend;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	fsdwsslo = &(store->fsdwsslo);
	header = &(fsdwsslo->header);
	bathymetry = &(store->bathymetry);
	bluefin = &(store->bluefin);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
	
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsslo->msec_timestamp)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsslo->ping_number)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsslo->number_channels)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsslo->total_bytes)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsslo->data_format));index += 4;
	index += 12;
	for (i=0;i<2;i++)
		{
		fsdwchannel = &(fsdwsslo->channel[i]);
		mbr_reson7kr_rd_fsdwchannel(verbose, fsdwsslo->data_format, buffer, &index, fsdwchannel, error);
		}
/*fprintf(stderr,"In mbr_reson7kr_rd_fsdwsslo: index:%d OffsetToOptionalData:%d\n", 
index, header->OffsetToOptionalData);
	index = header->OffsetToOptionalData;*/
	for (i=0;i<2;i++)
		{
		fsdwssheader = &(fsdwsslo->ssheader[i]);
		mbr_reson7kr_rd_fsdwssheader(verbose, buffer, &index, fsdwssheader, error);
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_SIDESCAN3;
		store->type = R7KRECID_FSDWsidescan;
		store->sstype = R7KRECID_FSDWsidescanLo;
		
		/* use Edgetech time for early MBARI SBP missions with
			bad time synching, otherwise use 7K timestamp */
		if (header->s7kTime.Year == 2004)
			{
			/* get the time from the original Edgetech header */
			time_j[0] = fsdwssheader->year;
			time_j[1] = fsdwssheader->day;
			time_j[2] = 60 * fsdwssheader->hour + fsdwssheader->minute;
			time_j[3] = fsdwssheader->second;
			time_j[4] = 1000 * (fsdwssheader->millisecondsToday 
						- 1000 * ((int)(0.001 * fsdwssheader->millisecondsToday)));
			}

		else
			{
			/* get the time from the 6046 datalogger header */
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			}

		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_fsdwss(verbose, fsdwsslo, error);
#ifdef MBR_RESON7KR_DEBUG
for (i=0;i<fsdwsslo->number_channels;i++)
{
fsdwchannel = &(fsdwsslo->channel[i]);
fsdwssheader = &(fsdwsslo->ssheader[i]);
fprintf(stderr,"R7KRECID_FSDWsidescanLo:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d %d chan:%d %d sampint:%d %d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
fsdwssheader->year,fsdwssheader->day,fsdwssheader->hour,fsdwssheader->minute,fsdwssheader->second,
fsdwssheader->millisecondsToday - 1000 * (int)(0.001 * fsdwssheader->millisecondsToday),
fsdwsslo->ping_number,fsdwssheader->pingNum,
fsdwchannel->number,fsdwssheader->channelNum,
fsdwchannel->sample_interval,fsdwssheader->sampleInterval);
}
#endif

	/* get sonar_depth and altitude if possible */
/*
	if (bluefin->nav[0].depth > 0.0)
		survey->sonar_depth = bluefin->nav[0].depth;
	if (bluefin->nav[0].altitude > 0.0)
		survey->sonar_altitude = bluefin->nav[0].altitude;
	if (bluefin->environmental[0].sound_speed > 0.0)
		sound_speed = bluefin->environmental[0].sound_speed;
	else
		sound_speed = 1500.0;
	half_sound_speed = 0.5 * sound_speed;
*/

	/* if necessary get first arrival and treat as altitude */
/*
	if (survey->sonar_altitude == 0.0)
		{
		bottompick = fsdwchannel->number_samples;
		for (i=0;i<2;i++)
			{
			fsdwchannel = &(fsdwsslo->channel[i]);
			shortptr = (short *) fsdwchannel->data;
			ushortptr = (unsigned short *) fsdwchannel->data;
			if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_ENVELOPE)
				pickvalue = (double) ushortptr[0];
			else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_ANALYTIC)
				pickvalue = sqrt((double) (shortptr[0] * shortptr[0] + shortptr[1] * shortptr[1]));
			else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_RAW)
				pickvalue = fabs((double) ushortptr[0]);
			else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_REALANALYTIC)
				pickvalue = fabs((double) ushortptr[0]);
			else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_PIXEL)
				pickvalue = (double) ushortptr[0];
			pickvalue = MAX(40 * pickvalue, 80.0);
			for (j=0;j<fsdwchannel->number_samples;j++)
				{
				if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_ENVELOPE)
					value = (double) ushortptr[j];
				else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_ANALYTIC)
					value = sqrt((double) (shortptr[2*j] * shortptr[2*j] + shortptr[2*j+1] * shortptr[2*j+1]));
				else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_RAW)
					value = (double) ushortptr[j];
				else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_REALANALYTIC)
					value = (double) ushortptr[j];
				else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_PIXEL)
					value = (double) ushortptr[j];
				if (bottompick > j && fabs(value) > pickvalue)
					{
					bottompick = j;
					survey->sonar_altitude = half_sound_speed * bottompick * 0.000001 * fsdwchannel->sample_interval;
					}
				}
			}
		}
	else
		{
		bottompick = survey->sonar_altitude / (half_sound_speed * 0.000001 * fsdwchannel->sample_interval);
		}
*/

	/* insert sslo data into survey structure */
/*
	tracemax = 0.0;
	pixelsize = (fsdwsslo->channel[0].range + fsdwsslo->channel[1].range) / 1024;
	for (j=0;j<1024;j++)
		{
		bin[j] = 0.0;
		nbin[j] = 0;
		}
	for (i=0;i<2;i++)
		{
		fsdwchannel = &(fsdwsslo->channel[i]);
		shortptr = (short *) fsdwchannel->data;
		ushortptr = (short *) fsdwchannel->data;
		survey->number_sslow_pixels = 1024;
		for (j=bottompick;j<fsdwchannel->number_samples;j++)
			{
			if (i == 0)
				{
				sign = -1.0;
				k = fsdwchannel->number_samples - (j - bottompick);
				}
			else
				{
				sign = 1.0;
				k = fsdwchannel->number_samples + (j - bottompick);
				}
			if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_ENVELOPE)
				value = (double) ushortptr[j];
			else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_ANALYTIC)
				value = sqrt((double) (shortptr[2*j] * shortptr[2*j] + shortptr[2*j+1] * shortptr[2*j+1]));
			else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_RAW)
				value = (double) ushortptr[j];
			else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_REALANALYTIC)
				value = (double) ushortptr[j];
			else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_PIXEL)
				value = (double) ushortptr[j];
			tracemax = MAX(tracemax, value);
			range = half_sound_speed * j * 0.000001 * fsdwchannel->sample_interval;
			xtrack = sign * sqrt(fabs(range * range 
						- survey->sonar_altitude * survey->sonar_altitude));
			k = (xtrack / pixelsize) + 512;
			if (k < 0) k = 0;
			if (k > 1023) k = 1023;
			bin[k] += value * value;
			nbin[k]++;
			}
		}
	for (j=0;j<1024;j++)
		{
		if (nbin[j] > 0)
			survey->sslow[j] = sqrt(bin[j]) / nbin[j];
		else
			survey->sslow[j] = 0.0;
		survey->sslow_acrosstrack[j] = pixelsize * (j - 512);;
		survey->sslow_alongtrack[j] = 0.0;
		}
*/

/*fprintf(stderr,"altitude:%f depth:%f sv:%f heading:%f bottompick:%d weight:%d ADCGain:%d ADCMax:%d max:%f\n",
survey->sonar_altitude,survey->sonar_depth,sound_speed,survey->heading,bottompick,
fsdwssheader->weightingFactor,fsdwssheader->ADCGain,fsdwssheader->ADCMax,tracemax);
fprintf(stderr,"\n\nbottompick:%d altitude:%f pixelsize:%f\n",bottompick,survey->sonar_altitude,pixelsize);
for (j=0;j<fsdwchannel->number_samples;j++)
{
fsdwchannel = &(fsdwsslo->channel[0]);
ushortptr = (unsigned short *) fsdwchannel->data;
value = (double) ushortptr[j];
fprintf(stderr,"SSL[%5d]: %10.0f",j,value);
fsdwchannel = &(fsdwsslo->channel[1]);
ushortptr = (unsigned short *) fsdwchannel->data;
value = (double) ushortptr[j];
fprintf(stderr," %10.0f",value);
if (j == bottompick) fprintf(stderr," BOTTOMPICK");
fprintf(stderr,"\n");
}*/
	
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
int mbr_reson7kr_rd_fsdwsshi(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_fsdwsshi";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_fsdwss *fsdwsshi;
	s7k_fsdwchannel *fsdwchannel;
	s7k_fsdwssheader *fsdwssheader;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	fsdwsshi = &(store->fsdwsshi);
	header = &(fsdwsshi->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
	
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsshi->msec_timestamp)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsshi->ping_number)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsshi->number_channels)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsshi->total_bytes)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsshi->data_format));index += 4;
	index += 12;
	for (i=0;i<2;i++)
		{
		fsdwchannel = &(fsdwsshi->channel[i]);
		mbr_reson7kr_rd_fsdwchannel(verbose, fsdwsshi->data_format, buffer, &index, fsdwchannel, error);
		}
/*fprintf(stderr,"In mbr_reson7kr_rd_fsdwsshi: index:%d OffsetToOptionalData:%d\n", 
index, header->OffsetToOptionalData);
	index = header->OffsetToOptionalData;*/
	for (i=0;i<2;i++)
		{
		fsdwssheader = &(fsdwsshi->ssheader[i]);
		mbr_reson7kr_rd_fsdwssheader(verbose, buffer, &index, fsdwssheader, error);
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_SIDESCAN2;
		store->type = R7KRECID_FSDWsidescan;
		store->sstype = R7KRECID_FSDWsidescanHi;
		
		/* use Edgetech time for early MBARI SBP missions with
			bad time synching, otherwise use 7K timestamp */
		if (header->s7kTime.Year == 2004)
			{
			/* get the time from the original Edgetech header */
			time_j[0] = fsdwssheader->year;
			time_j[1] = fsdwssheader->day;
			time_j[2] = 60 * fsdwssheader->hour + fsdwssheader->minute;
			time_j[3] = fsdwssheader->second;
			time_j[4] = 1000 * (fsdwssheader->millisecondsToday 
						- 1000 * ((int)(0.001 * fsdwssheader->millisecondsToday)));
			}

		else
			{
			/* get the time from the 6046 datalogger header */
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			}

		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_fsdwss(verbose, fsdwsshi, error);
#ifdef MBR_RESON7KR_DEBUG
for (i=0;i<fsdwsshi->number_channels;i++)
{
fsdwchannel = &(fsdwsshi->channel[i]);
fsdwssheader = &(fsdwsshi->ssheader[i]);
fprintf(stderr,"R7KRECID_FSDWsidescanHi:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d %d chan:%d %d sampint:%d %d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
fsdwssheader->year,fsdwssheader->day,fsdwssheader->hour,fsdwssheader->minute,fsdwssheader->second,
fsdwssheader->millisecondsToday - 1000 * (int)(0.001 * fsdwssheader->millisecondsToday),
fsdwsshi->ping_number,fsdwssheader->pingNum,
fsdwchannel->number,fsdwssheader->channelNum,
fsdwchannel->sample_interval,fsdwssheader->sampleInterval);
}
#endif

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
int mbr_reson7kr_rd_fsdwsb(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_fsdwsb";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_fsdwsb *fsdwsb;
	s7k_fsdwchannel *fsdwchannel;
	s7k_fsdwsegyheader *fsdwsegyheader;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	fsdwsb = &(store->fsdwsb);
	header = &(fsdwsb->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsb->msec_timestamp)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsb->ping_number)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsb->number_channels)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsb->total_bytes)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fsdwsb->data_format));index += 4;
	index += 12;
	fsdwchannel = &(fsdwsb->channel);
	mbr_reson7kr_rd_fsdwchannel(verbose, fsdwsb->data_format, buffer, &index, fsdwchannel, error);
/* fprintf(stderr,"index:%d offset:%d\n",index,header->OffsetToOptionalData);*/
	fsdwsegyheader = &(fsdwsb->segyheader);
	mbr_reson7kr_rd_fsdwsegyheader(verbose, buffer, &index, fsdwsegyheader, error);

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_SUBBOTTOM_SUBBOTTOM;
		store->type = R7KRECID_FSDWsubbottom;
		
		/* use Edgetech time for early MBARI SBP missions with
			bad time synching, otherwise use 7K timestamp */
		if (header->s7kTime.Year == 2004)
			{
			/* get the time from the original Edgetech header */
			time_j[0] = fsdwsegyheader->year;
			time_j[1] = fsdwsegyheader->day;
			time_j[2] = 60 * fsdwsegyheader->hour + fsdwsegyheader->minute;
			time_j[3] = fsdwsegyheader->second;
			time_j[4] = 1000 * (fsdwsegyheader->millisecondsToday 
						- 1000 * ((int)(0.001 * fsdwsegyheader->millisecondsToday)));
			}

		else
			{
			/* get the time from the 6046 datalogger header */
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			}

		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_fsdwsb(verbose, fsdwsb, error);
#ifdef MBR_RESON7KR_DEBUG
for (i=0;i<fsdwsb->number_channels;i++)
{
fsdwchannel = &(fsdwsb->channel);
fsdwsegyheader = &(fsdwsb->segyheader);
fprintf(stderr,"R7KRECID_FSDWsubbottom:            7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d %d chan:%d %d sampint:%d %d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
fsdwsegyheader->year,fsdwsegyheader->day,fsdwsegyheader->hour,fsdwsegyheader->minute,fsdwsegyheader->second,
fsdwsegyheader->millisecondsToday - 1000 * (int)(0.001 * fsdwsegyheader->millisecondsToday),
fsdwsb->ping_number,fsdwsegyheader->pingNum,
fsdwchannel->number,fsdwsegyheader->channelNum,
fsdwchannel->sample_interval,fsdwsegyheader->sampleInterval);
}
#endif

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
int mbr_reson7kr_rd_bluefin(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_bluefin";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_bluefin *bluefin;
	int	index;
	int	time_j[5];
	int	time_i[7];
	double	time_d;
	int	timeproblem;
	int	timechange;
	double	dtime;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	bluefin = &(store->bluefin);
	header = &(bluefin->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(bluefin->msec_timestamp)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(bluefin->number_frames)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(bluefin->frame_size)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(bluefin->data_format)); index += 4;
	for (i=0;i<16;i++)
		{
		bluefin->reserved[i] = buffer[index]; index++;
		}
	if (bluefin->data_format == R7KRECID_BluefinNav)
		{
		for (i=0;i<bluefin->number_frames;i++)
			{
			mb_get_binary_int(MB_YES, &buffer[index], &(bluefin->nav[i].packet_size)); index += 4;
			mb_get_binary_short(MB_YES, &buffer[index], &(bluefin->nav[i].version)); index += 2;
			mb_get_binary_short(MB_YES, &buffer[index], &(bluefin->nav[i].offset)); index += 2;
			mb_get_binary_int(MB_YES, &buffer[index], &(bluefin->nav[i].data_type)); index += 4;
			mb_get_binary_int(MB_YES, &buffer[index], &(bluefin->nav[i].data_size)); index += 4;
			mb_get_binary_short(MB_YES, &buffer[index], &(bluefin->nav[i].s7kTime.Year)); index += 2;
			mb_get_binary_short(MB_YES, &buffer[index], &(bluefin->nav[i].s7kTime.Day)); index += 2;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->nav[i].s7kTime.Seconds)); index += 4;
			bluefin->nav[i].s7kTime.Hours = (mb_u_char) buffer[index]; (index)++;
			bluefin->nav[i].s7kTime.Minutes = (mb_u_char) buffer[index]; (index)++;
			mb_get_binary_int(MB_YES, &buffer[index], &(bluefin->nav[i].checksum)); index += 4;
			mb_get_binary_short(MB_YES, &buffer[index], &(bluefin->nav[i].reserved)); index += 2;
			mb_get_binary_int(MB_YES, &buffer[index], &(bluefin->nav[i].quality)); index += 4;
			mb_get_binary_double(MB_YES, &buffer[index], &(bluefin->nav[i].latitude)); index += 8;
			mb_get_binary_double(MB_YES, &buffer[index], &(bluefin->nav[i].longitude)); index += 8;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->nav[i].speed)); index += 4;
			mb_get_binary_double(MB_YES, &buffer[index], &(bluefin->nav[i].depth)); index += 8;
			mb_get_binary_double(MB_YES, &buffer[index], &(bluefin->nav[i].altitude)); index += 8;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->nav[i].roll)); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->nav[i].pitch)); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->nav[i].yaw)); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->nav[i].northing_rate)); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->nav[i].easting_rate)); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->nav[i].depth_rate)); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->nav[i].altitude_rate)); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->nav[i].roll_rate)); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->nav[i].pitch_rate)); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->nav[i].yaw_rate)); index += 4;
			mb_get_binary_double(MB_YES, &buffer[index], &(bluefin->nav[i].position_time)); index += 8;
			mb_get_binary_double(MB_YES, &buffer[index], &(bluefin->nav[i].altitude_time)); index += 8;
			}
			
		/* The Reson 6046 datalogger has been placing the same time tag in each of the frames
			- check if this is the case, if it is kluge new time tags spread over a one second interval */
		if (bluefin->number_frames > 1)
			{
			/* figure out if there is a time problem */
			timeproblem = MB_NO;
			for (i=1;i<bluefin->number_frames;i++)
				{
				if (bluefin->nav[i].position_time == bluefin->nav[i-1].position_time)
					timeproblem = MB_YES;
				}
			
			/* figure out if the time changes anywhere */
			if (timeproblem == MB_YES)
				{
				timechange = 0;
				dtime = 0.2;
				for (i=1;i<bluefin->number_frames;i++)
					{
					if (bluefin->nav[i].position_time != bluefin->nav[i-1].position_time)
						timechange = i;
	/*fprintf(stderr,"Time: %f timechange:%d\n",bluefin->nav[i].s7kTime.Seconds,timechange);*/
					}

				/* change times assuming a 5 Hz data rate */
				dtime = 0.2;
				for (i=0;i<bluefin->number_frames;i++)
					{
					/* get the time  */
					time_d = bluefin->nav[timechange].position_time + (i - timechange) * dtime;
	/*fprintf(stderr,"CHANGE TIMESTAMP: %d %2.2d:%2.2d:%6.3f %12f",
	i,bluefin->nav[i].s7kTime.Hours,bluefin->nav[i].s7kTime.Minutes,bluefin->nav[i].s7kTime.Seconds,time_d);*/
					bluefin->nav[i].position_time = time_d;
					bluefin->nav[i].altitude_time = time_d;
					mb_get_date(verbose, time_d, time_i);
					mb_get_jtime(verbose, time_i, time_j);
					bluefin->nav[i].s7kTime.Seconds = 0.000001 * time_j[4] + time_j[3];
					bluefin->nav[i].s7kTime.Hours = (int)(time_j[2] / 60.0);
					bluefin->nav[i].s7kTime.Minutes = (int)(time_j[2] - 60.0 * bluefin->nav[i].s7kTime.Hours);
					bluefin->nav[i].s7kTime.Day = time_j[1];
					bluefin->nav[i].s7kTime.Year = time_j[0];
	/*fprintf(stderr,"    %12f  %2.2d:%2.2d:%6.3f\n",
	time_d, bluefin->nav[i].s7kTime.Hours,bluefin->nav[i].s7kTime.Minutes,bluefin->nav[i].s7kTime.Seconds);*/
					}
				}
			}
for (i=0;i<bluefin->number_frames;i++)
{
/*bluefin->nav[i].depth = 0.0;
bluefin->nav[i].roll = 0.0;
bluefin->nav[i].pitch = 0.0;*/
}
		}
	else if (bluefin->data_format == R7KRECID_BluefinEnvironmental)
		{
		for (i=0;i<bluefin->number_frames;i++)
			{
			mb_get_binary_int(MB_YES, &buffer[index], &(bluefin->environmental[i].packet_size)); index += 4;
			mb_get_binary_short(MB_YES, &buffer[index], &(bluefin->environmental[i].version)); index += 2;
			mb_get_binary_short(MB_YES, &buffer[index], &(bluefin->environmental[i].offset)); index += 2;
			mb_get_binary_int(MB_YES, &buffer[index], &(bluefin->environmental[i].data_type)); index += 4;
			mb_get_binary_int(MB_YES, &buffer[index], &(bluefin->environmental[i].data_size)); index += 4;
			mb_get_binary_short(MB_YES, &buffer[index], &(bluefin->environmental[i].s7kTime.Year)); index += 2;
			mb_get_binary_short(MB_YES, &buffer[index], &(bluefin->environmental[i].s7kTime.Day)); index += 2;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->environmental[i].s7kTime.Seconds)); index += 4;
			bluefin->environmental[i].s7kTime.Hours = (mb_u_char) buffer[index]; (index)++;
			bluefin->environmental[i].s7kTime.Minutes = (mb_u_char) buffer[index]; (index)++;
			mb_get_binary_int(MB_YES, &buffer[index], &(bluefin->environmental[i].checksum)); index += 4;
			mb_get_binary_short(MB_YES, &buffer[index], &(bluefin->environmental[i].reserved1)); index += 2;
			mb_get_binary_int(MB_YES, &buffer[index], &(bluefin->environmental[i].quality)); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->environmental[i].sound_speed)); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->environmental[i].conductivity)); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->environmental[i].temperature)); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->environmental[i].pressure)); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bluefin->environmental[i].salinity)); index += 4;
			mb_get_binary_double(MB_YES, &buffer[index], &(bluefin->environmental[i].ctd_time)); index += 8;
			mb_get_binary_double(MB_YES, &buffer[index], &(bluefin->environmental[i].temperature_time)); index += 8;
			for (j=0;j<56;j++)
				{
				bluefin->environmental[i].reserved2[j] = buffer[index]; index++;
				}
			}
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		if (bluefin->data_format == R7KRECID_BluefinNav)
			{
			store->kind = MB_DATA_NAV1;
			store->type = R7KRECID_Bluefin;
		
			/* get the time from the 6046 datalogger header */
			time_j[0] = bluefin->nav[0].s7kTime.Year;
			time_j[1] = bluefin->nav[0].s7kTime.Day;
			time_j[2] = 60 * bluefin->nav[0].s7kTime.Hours + bluefin->nav[0].s7kTime.Minutes;
			time_j[3] = (int) bluefin->nav[0].s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (bluefin->nav[0].s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, store->time_i);
			mb_get_time(verbose, store->time_i, &(store->time_d));
			}
		else if (bluefin->data_format == R7KRECID_BluefinEnvironmental)
			{
			store->kind = MB_DATA_SSV;
			store->type = R7KRECID_Bluefin;
		
			/* get the time from the 6046 datalogger header */
			time_j[0] = bluefin->environmental[0].s7kTime.Year;
			time_j[1] = bluefin->environmental[0].s7kTime.Day;
			time_j[2] = 60 * bluefin->environmental[0].s7kTime.Hours + bluefin->environmental[0].s7kTime.Minutes;
			time_j[3] = (int) bluefin->environmental[0].s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (bluefin->environmental[0].s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, store->time_i);
			mb_get_time(verbose, store->time_i, &(store->time_d));
			}
		else
			{
			store->kind = MB_DATA_NONE;
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
if (bluefin->data_format == R7KRECID_BluefinNav)
fprintf(stderr,"R7KRECID_BluefinNav:               7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
else
fprintf(stderr,"R7KRECID_BluefinEnvironmental:     7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_bluefin(verbose, bluefin, error);

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
int mbr_reson7kr_rd_volatilesonarsettings(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_volatilesonarsettings";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_volatilesettings *volatilesettings;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	volatilesettings = &(store->volatilesettings);
	header = &(volatilesettings->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_long(MB_YES, &buffer[index], &(volatilesettings->serial_number)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(volatilesettings->ping_number)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(volatilesettings->multi_ping)); index += 2;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->frequency)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->sample_rate)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->receiver_bandwidth)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->pulse_width)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(volatilesettings->pulse_type)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(volatilesettings->pulse_envelope)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->pulse_envelope_par)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(volatilesettings->pulse_reserved)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->max_ping_rate)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->ping_period)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->range_selection)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->power_selection)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->gain_selection)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(volatilesettings->control_flags)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(volatilesettings->projector_magic_no)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->steering_vertical)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->steering_horizontal)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->beamwidth_vertical)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->beamwidth_horizontal)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->focal_point)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(volatilesettings->projector_weighting)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->projector_weighting_par)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(volatilesettings->transmit_flags)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(volatilesettings->hydrophone_magic_no)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(volatilesettings->receive_weighting)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->receive_weighting_par)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(volatilesettings->receive_flags)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->receive_width)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->range_minimum)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->range_maximum)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->depth_minimum)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->depth_maximum)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->absorption)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->sound_velocity)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(volatilesettings->spreading)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(volatilesettings->reserved)); index += 2;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_PARAMETER;
		store->type = R7KRECID_7kVolatileSonarSettings;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_7kVolatileSonarSettings:  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_volatilesettings(verbose, volatilesettings, error);

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
int mbr_reson7kr_rd_configuration(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_configuration";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_configuration *configuration;
	s7k_device *device;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	configuration = &(store->configuration);
	header = &(configuration->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_long(MB_YES, &buffer[index], &(configuration->serial_number)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(configuration->number_devices)); index += 4;
	
	/* extract the data for each device */
	for (i=0;i<configuration->number_devices;i++)
		{
		device = &(configuration->device[i]);
		mb_get_binary_int(MB_YES, &buffer[index], &(device->magic_number)); index += 4;
		for (j=0;j<64;j++)
			{
			device->description[j] = buffer[index]; index++;
			}
		mb_get_binary_long(MB_YES, &buffer[index], &(device->serial_number)); index += 8;
		mb_get_binary_int(MB_YES, &buffer[index], &(device->info_length)); index += 4;

		/* make sure enough memory is allocated for info data */
		if (device->info_alloc < device->info_length)
			{
			data_size = device->info_length + 1;
			status = mb_realloc(verbose, data_size, &(device->info), error);
			if (status == MB_SUCCESS)
				{
				device->info_alloc = device->info_length;
				}
			else
				{
				device->info_alloc = 0;
				device->info_length = 0;
				}
			}
			
		for (j=0;j<device->info_length;j++)
			{
			device->info[j] = buffer[index]; index++;
			}
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_PARAMETER;
		store->type = R7KRECID_7kConfiguration;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_7kConfiguration:          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_configuration(verbose, configuration, error);

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
int mbr_reson7kr_rd_matchfilter(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_matchfilter";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_matchfilter *matchfilter;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	matchfilter = &(store->matchfilter);
	header = &(matchfilter->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_long(MB_YES, &buffer[index], &(matchfilter->serial_number)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(matchfilter->ping_number)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(matchfilter->operation)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(matchfilter->start_frequency)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(matchfilter->end_frequency)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_DATA;
		store->type = R7KRECID_7kMatchFilter;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_7kMatchFilter:            7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_matchfilter(verbose, matchfilter, error);

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
int mbr_reson7kr_rd_beamgeometry(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_beamgeometry";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_beamgeometry *beamgeometry;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	beamgeometry = &(store->beamgeometry);
	header = &(beamgeometry->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_long(MB_YES, &buffer[index], &(beamgeometry->serial_number)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(beamgeometry->number_beams)); index += 4;
	
	/* extract the data */
	for (i=0;i<beamgeometry->number_beams;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(beamgeometry->angle_alongtrack[i])); index += 4;
		}
	for (i=0;i<beamgeometry->number_beams;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(beamgeometry->angle_acrosstrack[i])); index += 4;
		}
	for (i=0;i<beamgeometry->number_beams;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(beamgeometry->beamwidth_alongtrack[i])); index += 4;
		}
	for (i=0;i<beamgeometry->number_beams;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(beamgeometry->beamwidth_acrosstrack[i])); index += 4;
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_DATA;
		store->type = R7KRECID_7kBeamGeometry;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_7kBeamGeometry:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_beamgeometry(verbose, beamgeometry, error);

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
int mbr_reson7kr_rd_calibration(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_calibration";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_calibration *calibration;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	calibration = &(store->calibration);
	header = &(calibration->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_long(MB_YES, &buffer[index], &(calibration->serial_number)); index += 8;
	mb_get_binary_short(MB_YES, &buffer[index], &(calibration->number_channels)); index += 2;
	
	/* extract the data */
	for (i=0;i<calibration->number_channels;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(calibration->gain[i])); index += 4;
		}
	for (i=0;i<calibration->number_channels;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(calibration->phase[i])); index += 4;
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_DATA;
		store->type = R7KRECID_7kCalibrationData;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_7kCalibrationData:        7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_calibration(verbose, calibration, error);

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
int mbr_reson7kr_rd_bathymetry(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_bathymetry";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_bathymetry *bathymetry;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	bathymetry = &(store->bathymetry);
	header = &(bathymetry->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_long(MB_YES, &buffer[index], &(bathymetry->serial_number)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(bathymetry->ping_number)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(bathymetry->multi_ping)); index += 2;
	mb_get_binary_int(MB_YES, &buffer[index], &(bathymetry->number_beams)); index += 4;
	
	/* extract the data */
	for (i=0;i<bathymetry->number_beams;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(bathymetry->range[i])); index += 4;
		}
	for (i=0;i<bathymetry->number_beams;i++)
		{
		bathymetry->quality[i] = buffer[index]; index++;
		}
	for (i=0;i<bathymetry->number_beams;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(bathymetry->intensity[i])); index += 4;
		}
		
	/* extract the optional data */
	if (header->OffsetToOptionalData > 0)
		{
		index = header->OffsetToOptionalData;
		bathymetry->optionaldata = MB_YES;
		mb_get_binary_float(MB_YES, &buffer[index], &(bathymetry->frequency)); index += 4;
		mb_get_binary_double(MB_YES, &buffer[index], &(bathymetry->latitude)); index += 8;
		mb_get_binary_double(MB_YES, &buffer[index], &(bathymetry->longitude)); index += 8;
		mb_get_binary_float(MB_YES, &buffer[index], &(bathymetry->heading)); index += 4;
		bathymetry->height_source = buffer[index]; index++;
		mb_get_binary_float(MB_YES, &buffer[index], &(bathymetry->tide)); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(bathymetry->roll)); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(bathymetry->pitch)); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(bathymetry->heave)); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(bathymetry->vehicle_height)); index += 4;
		for (i=0;i<bathymetry->number_beams;i++)
			{
			mb_get_binary_float(MB_YES, &buffer[index], &(bathymetry->depth[i])); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bathymetry->acrosstrack[i])); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bathymetry->alongtrack[i])); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bathymetry->pointing_angle[i])); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(bathymetry->azimuth_angle[i])); index += 4;
			}
		}
	else
		{
		bathymetry->optionaldata = MB_NO;
		bathymetry->frequency = 0.0;
		bathymetry->latitude = 0.0;
		bathymetry->longitude = 0.0;
		bathymetry->heading = 0.0;
		bathymetry->height_source = 0;
		bathymetry->tide = 0.0;
		bathymetry->roll = 0.0;
		bathymetry->pitch = 0.0;
		bathymetry->heave = 0.0;
		bathymetry->vehicle_height = 0.0;
		for (i=0;i<MBSYS_RESON7K_MAX_BEAMS;i++)
			{
			bathymetry->depth[i] = 0.0;
			bathymetry->acrosstrack[i] = 0.0;
			bathymetry->alongtrack[i] = 0.0;
			bathymetry->pointing_angle[i] = 0.0;
			bathymetry->azimuth_angle[i] = 0.0;
			}
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_DATA;
		store->type = R7KRECID_7kBathymetricData;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_7kBathymetricData:        7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber,bathymetry->ping_number);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_bathymetry(verbose, bathymetry, error);

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
int mbr_reson7kr_rd_backscatter(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_backscatter";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_backscatter *backscatter;
	int	data_size;
	int	index;
	int	time_j[5];
	short	*short_ptr;
	int	*int_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	backscatter = &(store->backscatter);
	header = &(backscatter->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_long(MB_YES, &buffer[index], &(backscatter->serial_number)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(backscatter->ping_number)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(backscatter->multi_ping)); index += 2;
	mb_get_binary_float(MB_YES, &buffer[index], &(backscatter->beam_position)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(backscatter->control_flags)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(backscatter->number_samples)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(backscatter->port_beamwidth_x)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(backscatter->port_beamwidth_y)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(backscatter->stbd_beamwidth_x)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(backscatter->stbd_beamwidth_y)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(backscatter->port_steering_x)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(backscatter->port_steering_y)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(backscatter->stbd_steering_x)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(backscatter->stbd_steering_y)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(backscatter->number_beams)); index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(backscatter->current_beam)); index += 2;
	backscatter->sample_size = buffer[index]; index++;
	backscatter->data_type = buffer[index]; index++;
	
	/* allocate memory if required */
	data_size = backscatter->number_samples * backscatter->sample_size;
	if (backscatter->nalloc < data_size)
		{
		status = mb_realloc(verbose, data_size, &(backscatter->port_data), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, data_size, &(backscatter->stbd_data), error);
		if (status == MB_SUCCESS)
			{
			backscatter->nalloc = data_size;
			}
		else
			{
			backscatter->nalloc = 0;
			backscatter->number_samples = 0;
			}
		}
	
	/* extract backscatter data */
	if (backscatter->sample_size == 1)
		{
		for (i=0;i<backscatter->number_samples;i++)
			{
			backscatter->port_data[i] = buffer[index]; index++;
			}
		for (i=0;i<backscatter->number_samples;i++)
			{
			backscatter->stbd_data[i] = buffer[index]; index++;
			}
		}
	else if (backscatter->sample_size == 2)
		{
		short_ptr = (short *) backscatter->port_data;
		for (i=0;i<backscatter->number_samples;i++)
			{
			mb_get_binary_short(MB_YES, &buffer[index], &(short_ptr[i])); index += 2;
			}
		short_ptr = (short *) backscatter->stbd_data;
		for (i=0;i<backscatter->number_samples;i++)
			{
			mb_get_binary_short(MB_YES, &buffer[index], &(short_ptr[i])); index += 2;
			}
		}
	else if (backscatter->sample_size == 4)
		{
		int_ptr = (int *) backscatter->port_data;
		for (i=0;i<backscatter->number_samples;i++)
			{
			mb_get_binary_int(MB_YES, &buffer[index], &(int_ptr[i])); index += 4;
			}
		int_ptr = (int *) backscatter->stbd_data;
		for (i=0;i<backscatter->number_samples;i++)
			{
			mb_get_binary_int(MB_YES, &buffer[index], &(int_ptr[i])); index += 4;
			}
		}
		
	/* extract the optional data */
	if (header->OffsetToOptionalData > 0)
		{
		index = header->OffsetToOptionalData;
		backscatter->optionaldata = MB_YES;
		mb_get_binary_float(MB_YES, &buffer[index], &(backscatter->frequency)); index += 4;
		mb_get_binary_double(MB_YES, &buffer[index], &(backscatter->latitude)); index += 8;
		mb_get_binary_double(MB_YES, &buffer[index], &(backscatter->longitude)); index += 8;
		mb_get_binary_float(MB_YES, &buffer[index], &(backscatter->heading)); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(backscatter->altitude)); index += 4;
		}
	else
		{
		backscatter->optionaldata = MB_NO;
		backscatter->frequency = 0.0;
		backscatter->latitude = 0.0;
		backscatter->longitude = 0.0;
		backscatter->heading = 0.0;
		backscatter->altitude = 0.0;
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_DATA;
		store->type = R7KRECID_7kBackscatterImageData;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_7kBackscatterImageData:   7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber,backscatter->ping_number);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_backscatter(verbose, backscatter, error);

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
int mbr_reson7kr_rd_beam(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_beam";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_beam *beam;
	s7kr_snippet *snippet;
	int	data_size;
	int	index;
	int	time_j[5];
	short	*short_ptr;
	int	*int_ptr;
	int	nalloc;
	int	nsamples;
	int	sample_type_amp;
	int	sample_type_phase;
	int	sample_type_iandq;
	char	*charptr;
	unsigned short 	*ushortptr;
	unsigned int	*uintptr;
	short	*shortptramp;
	short	*shortptrphase;
	int	*intptramp;
	int	*intptrphase;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	beam = &(store->beam);
	header = &(beam->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_long(MB_YES, &buffer[index], &(beam->serial_number)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(beam->ping_number)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(beam->multi_ping)); index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(beam->number_beams)); index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(beam->reserved)); index += 2;
	mb_get_binary_int(MB_YES, &buffer[index], &(beam->number_samples)); index += 4;
	beam->record_subset_flag = buffer[index]; index++;
	beam->row_column_flag = buffer[index]; index++;
	mb_get_binary_short(MB_YES, &buffer[index], &(beam->sample_header_id)); index += 2;
	mb_get_binary_int(MB_YES, &buffer[index], &(beam->sample_type)); index += 4;
	sample_type_amp = beam->sample_type & 15;
	sample_type_phase = (beam->sample_type << 4) & 15;
	sample_type_iandq = (beam->sample_type << 8) & 15;
	for (i=0;i<beam->number_beams;i++)
		{
		snippet = &beam->snippets[i];
		mb_get_binary_short(MB_YES, &buffer[index], &(snippet->beam_number)); index += 2;
		mb_get_binary_int(MB_YES, &buffer[index], &(snippet->begin_sample)); index += 4;
		mb_get_binary_int(MB_YES, &buffer[index], &(snippet->end_sample)); index += 4;
		}
	
	for (i=0;i<beam->number_beams;i++)
		{
		/* allocate memory for snippet if needed */
		snippet = &beam->snippets[i];
		nalloc = 0;
		if (sample_type_amp == 1)
			nalloc += 1;
		else if (sample_type_amp == 2)
			nalloc += 2;
		else if (sample_type_amp == 3)
			nalloc += 4;
		if (sample_type_phase == 1)
			nalloc += 1;
		else if (sample_type_phase == 2)
			nalloc += 2;
		else if (sample_type_phase == 3)
			nalloc += 4;
		if (sample_type_iandq == 1)
			nalloc += 4;
		else if (sample_type_iandq == 2)
			nalloc += 8;
		nalloc *= snippet->end_sample - snippet->begin_sample + 1;
		if (status == MB_SUCCESS
			&& snippet->nalloc < nalloc)
			{
			snippet->nalloc = nalloc;
			if (status == MB_SUCCESS)
			status = mb_realloc(verbose, snippet->nalloc,
						(char **) &(snippet->amplitude), error);
			if (status == MB_SUCCESS)
			status = mb_realloc(verbose, snippet->nalloc,
						(char **) &(snippet->phase), error);
			if (status != MB_SUCCESS)
				{
				snippet->nalloc = 0;
				}
			}

		/* extract snippet data */
		if (status == MB_SUCCESS)
			{
			nsamples = snippet->end_sample - snippet->begin_sample + 1;
			if (sample_type_amp == 1)
				{
				charptr = (char *)snippet->amplitude;
				for (j=0;j<nsamples;j++)
					{
					charptr[j] = buffer[index]; index++;
					}
				}
			else if (sample_type_amp == 2)
				{
				ushortptr = (unsigned short *)snippet->amplitude;
				for (j=0;j<nsamples;j++)
					{
					mb_get_binary_short(MB_YES, &buffer[index], &(ushortptr[j])); index += 2;
					}
				}
			else if (sample_type_amp == 3)
				{
				uintptr = (unsigned int *)snippet->amplitude;
				for (j=0;j<nsamples;j++)
					{
					mb_get_binary_int(MB_YES, &buffer[index], &(uintptr[j])); index += 4;
					}
				}
			if (sample_type_phase == 1)
				{
				charptr = (char *)snippet->phase;
				for (j=0;j<nsamples;j++)
					{
					charptr[j] = buffer[index]; index++;
					}
				}
			else if (sample_type_phase == 2)
				{
				ushortptr = (unsigned short *)snippet->phase;
				for (j=0;j<nsamples;j++)
					{
					mb_get_binary_short(MB_YES, &buffer[index], &(ushortptr[j])); index += 2;
					}
				}
			else if (sample_type_phase == 3)
				{
				uintptr = (unsigned int *)snippet->phase;
				for (j=0;j<nsamples;j++)
					{
					mb_get_binary_int(MB_YES, &buffer[index], &(uintptr[j])); index += 4;
					}
				}
			else if (sample_type_iandq == 1)
				{
				shortptramp = (short *)snippet->amplitude;
				shortptrphase = (short *)snippet->phase;
				for (j=0;j<nsamples;j++)
					{
					mb_get_binary_short(MB_YES, &buffer[index], &(shortptramp[j])); index += 2;
					mb_get_binary_short(MB_YES, &buffer[index], &(shortptrphase[j])); index += 2;
					}
				}
			else if (sample_type_iandq == 2)
				{
				intptramp = (int *)snippet->amplitude;
				intptrphase = (int *)snippet->phase;
				for (j=0;j<nsamples;j++)
					{
					mb_get_binary_int(MB_YES, &buffer[index], &(intptramp[j])); index += 4;
					mb_get_binary_int(MB_YES, &buffer[index], &(intptrphase[j])); index += 4;
					}
				}
			}
		}
		
	/* extract the optional data */
	if (header->OffsetToOptionalData > 0)
		{
		index = header->OffsetToOptionalData;
		beam->optionaldata = MB_YES;
		mb_get_binary_float(MB_YES, &buffer[index], &(beam->frequency)); index += 4;
		mb_get_binary_double(MB_YES, &buffer[index], &(beam->latitude)); index += 8;
		mb_get_binary_double(MB_YES, &buffer[index], &(beam->longitude)); index += 8;
		mb_get_binary_float(MB_YES, &buffer[index], &(beam->heading)); index += 4;
		for (i=0;i<beam->number_beams;i++)
			{
			mb_get_binary_float(MB_YES, &buffer[index], &(beam->alongtrack[i])); index += 4;
			mb_get_binary_float(MB_YES, &buffer[index], &(beam->acrosstrack[i])); index += 4;
			mb_get_binary_int(MB_YES, &buffer[index], &(beam->center_sample[i])); index += 4;
			}
		}
	else
		{
		beam->optionaldata = MB_NO;
		beam->frequency = 0.0;
		beam->latitude = 0.0;
		beam->longitude = 0.0;
		beam->heading = 0.0;
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_DATA;
		store->type = R7KHDRSIZE_7kBeamData;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KHDRSIZE_7kBeamData: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber,beam->ping_number);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_beam(verbose, beam, error);

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
int mbr_reson7kr_rd_verticaldepth(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_verticaldepth";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_verticaldepth *verticaldepth;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	verticaldepth = &(store->verticaldepth);
	header = &(verticaldepth->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(verticaldepth->frequency)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(verticaldepth->ping_number)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(verticaldepth->multi_ping)); index += 2;
	mb_get_binary_double(MB_YES, &buffer[index], &(verticaldepth->latitude)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(verticaldepth->longitude)); index += 8;
	mb_get_binary_float(MB_YES, &buffer[index], &(verticaldepth->heading)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(verticaldepth->alongtrack)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(verticaldepth->acrosstrack)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(verticaldepth->vertical_depth)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_DATA;
		store->type = R7KRECID_7kVerticalDepth;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_7kVerticalDepth:          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber,verticaldepth->ping_number);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_verticaldepth(verbose, verticaldepth, error);

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
int mbr_reson7kr_rd_image(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_image";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_image *image;
	s7kr_snippet *snippet;
	int	data_size;
	int	index;
	int	time_j[5];
	short	*short_ptr;
	int	*int_ptr;
	int	nalloc;
	int	nsamples;
	char	*charptr;
	unsigned short 	*ushortptr;
	unsigned int	*uintptr;
	short	*shortptramp;
	short	*shortptrphase;
	int	*intptramp;
	int	*intptrphase;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	image = &(store->image);
	header = &(image->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(image->ping_number)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(image->multi_ping)); index += 2;
	mb_get_binary_int(MB_YES, &buffer[index], &(image->width)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(image->height)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(image->color_depth)); index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(image->width_height_flag)); index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(image->compression)); index += 2;
	
	/* allocate memory for image if needed */
	nalloc = image->width * image->height * image->color_depth;
	if (status == MB_SUCCESS
		&& image->nalloc < nalloc)
		{
		image->nalloc = nalloc;
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, image->nalloc,
					(char **) &(image->image), error);
		if (status != MB_SUCCESS)
			{
			image->nalloc = 0;
			image->width = 0;
			image->height = 0;
			}
		}
	
	/* extract image data */
	if (image->color_depth == 1)
		{
		charptr = (char *)image->image;
		for (i=0;i<image->width * image->height;i++)
			{
			charptr[i] = buffer[index]; index++;
			}
		}
	else if (image->color_depth == 2)
		{
		ushortptr = (unsigned short *)image->image;
		for (i=0;i<image->width * image->height;i++)
			{
			mb_get_binary_short(MB_YES, &buffer[index], &(ushortptr[i])); index += 2;
			}
		}
	else if (image->color_depth == 4)
		{
		uintptr = (unsigned int *)image->image;
		for (i=0;i<image->width * image->height;i++)
			{
			mb_get_binary_int(MB_YES, &buffer[index], &(uintptr[i])); index += 4;
			}
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_DATA;
		store->type = R7KRECID_7kImageData;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_7kImageData:              7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber,image->ping_number);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_image(verbose, image, error);

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
int mbr_reson7kr_rd_installation(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_installation";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_installation *installation;
	s7kr_snippet *snippet;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	installation = &(store->installation);
	header = &(installation->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->frequency)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(installation->firmware_version_len)); index += 2;
	for (i=0;i<128;i++)
		{
		installation->firmware_version[i] = buffer[index]; index++;
		}
	mb_get_binary_short(MB_YES, &buffer[index], &(installation->software_version_len)); index += 2;
	for (i=0;i<128;i++)
		{
		installation->software_version[i] = buffer[index]; index++;
		}
	mb_get_binary_short(MB_YES, &buffer[index], &(installation->s7k_version_len)); index += 2;
	for (i=0;i<128;i++)
		{
		installation->s7k_version[i] = buffer[index]; index++;
		}
	mb_get_binary_short(MB_YES, &buffer[index], &(installation->protocal_version_len)); index += 2;
	for (i=0;i<128;i++)
		{
		installation->protocal_version[i] = buffer[index]; index++;
		}
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->transmit_x)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->transmit_y)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->transmit_z)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->transmit_roll)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->transmit_pitch)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->transmit_heading)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->receive_x)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->receive_y)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->receive_z)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->receive_roll)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->receive_pitch)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->receive_heading)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->motion_x)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->motion_y)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->motion_z)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->motion_roll)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->motion_pitch)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->motion_heading)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(installation->motion_time_delay)); index += 2;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->position_x)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->position_y)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->position_z)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(installation->position_time_delay)); index += 2;
	mb_get_binary_float(MB_YES, &buffer[index], &(installation->waterline_z)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_INSTALLATION;
		store->type = R7KRECID_7kInstallationParameters;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_7kInstallationParameters: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_installation(verbose, installation, error);

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
int mbr_reson7kr_rd_fileheader(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_fileheader";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_fileheader *fileheader;
	s7kr_subsystem *subsystem;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	fileheader = &(store->fileheader);
	header = &(fileheader->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	for (i=0;i<16;i++)
		{
		fileheader->file_identifier[i] = buffer[index]; index++;
		}
	mb_get_binary_short(MB_YES, &buffer[index], &(fileheader->version)); index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(fileheader->reserved)); index += 2;
	for (i=0;i<16;i++)
		{
		fileheader->session_identifier[i] = buffer[index]; index++;
		}
	mb_get_binary_int(MB_YES, &buffer[index], &(fileheader->record_data_size)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(fileheader->number_subsystems)); index += 4;
	for (i=0;i<64;i++)
		{
		fileheader->recording_name[i] = buffer[index]; index++;
		}
	for (i=0;i<16;i++)
		{
		fileheader->recording_version[i] = buffer[index]; index++;
		}
	for (i=0;i<64;i++)
		{
		fileheader->user_defined_name[i] = buffer[index]; index++;
		}
	for (i=0;i<128;i++)
		{
		fileheader->notes[i] = buffer[index]; index++;
		}
	for (i=0;i<fileheader->number_subsystems;i++)
		{
		subsystem = &(fileheader->subsystem[i]);
		mb_get_binary_int(MB_YES, &buffer[index], &(subsystem->device_identifier)); index += 4;
		if (header->Version == 2)
			mb_get_binary_short(MB_YES, &buffer[index], &(subsystem->system_enumerator)); index += 2;
		mb_get_binary_short(MB_YES, &buffer[index], &(subsystem->system_enumerator)); index += 2;
		}
		
	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_HEADER;
		store->type = R7KRECID_7kFileHeader;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"R7KRECID_7kFileHeader:             7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
header->RecordNumber);
#endif
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_fileheader(verbose, fileheader, error);

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
int mbr_reson7kr_rd_systemeventmessage(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_systemeventmessage";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_systemeventmessage *systemeventmessage;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	systemeventmessage = &(store->systemeventmessage);
	header = &(systemeventmessage->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_long(MB_YES, &buffer[index], &(systemeventmessage->serial_number)); index += 8;
	mb_get_binary_short(MB_YES, &buffer[index], &(systemeventmessage->event_id)); index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(systemeventmessage->message_length)); index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(systemeventmessage->event_identifier)); index += 2;
		
	/* make sure enough memory is allocated for channel data */
	if (systemeventmessage->message_alloc < systemeventmessage->message_length)
		{
		data_size = systemeventmessage->message_length + 1;
		status = mb_realloc(verbose, data_size, &(systemeventmessage->message), error);
		if (status == MB_SUCCESS)
			{
			systemeventmessage->message_alloc = systemeventmessage->message_length;
			}
		else
			{
			systemeventmessage->message_alloc = 0;
			systemeventmessage->message_length = 0;
			}
		}

	/* extract the data */
	for (i=0;i<systemeventmessage->message_length;i++)
		{
		systemeventmessage->message[i] = buffer[index]; index++;
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_COMMENT;
		store->type = R7KRECID_7kSystemEventMessage;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_systemeventmessage(verbose, systemeventmessage, error);

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
int mbr_reson7kr_rd_remotecontrolsettings(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_remotecontrolsettings";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_remotecontrolsettings *remotecontrolsettings;
	int	data_size;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	remotecontrolsettings = &(store->remotecontrolsettings);
	header = &(remotecontrolsettings->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_long(MB_YES, &buffer[index], &(remotecontrolsettings->serial_number)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(remotecontrolsettings->ping_number)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->frequency)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->sample_rate)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->receiver_bandwidth)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->pulse_width)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(remotecontrolsettings->pulse_type)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(remotecontrolsettings->pulse_envelope)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->pulse_envelope_par)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(remotecontrolsettings->pulse_reserved)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->max_ping_rate)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->ping_period)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->range_selection)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->power_selection)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->gain_selection)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(remotecontrolsettings->control_flags)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(remotecontrolsettings->projector_magic_no)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->steering_vertical)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->steering_horizontal)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->beamwidth_vertical)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->beamwidth_horizontal)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->focal_point)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(remotecontrolsettings->projector_weighting)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->projector_weighting_par)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(remotecontrolsettings->transmit_flags)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(remotecontrolsettings->hydrophone_magic_no)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(remotecontrolsettings->receive_weighting)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->receive_weighting_par)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(remotecontrolsettings->receive_flags)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->range_minimum)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->range_maximum)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->depth_minimum)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->depth_maximum)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->absorption)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->sound_velocity)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(remotecontrolsettings->spreading)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(remotecontrolsettings->reserved)); index += 2;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_PARAMETER;
		store->type = R7KRECID_7kRemoteControlSonarSettings;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose >= 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_remotecontrolsettings(verbose, remotecontrolsettings, error);

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
int mbr_reson7kr_rd_roll(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_roll";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_roll *roll;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	roll = &(store->roll);
	header = &(roll->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(roll->roll)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_ROLL;
		store->type = R7KRECID_7kRoll;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_roll(verbose, roll, error);

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
int mbr_reson7kr_rd_pitch(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_pitch";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_pitch *pitch;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	pitch = &(store->pitch);
	header = &(pitch->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(pitch->pitch)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_PITCH;
		store->type = R7KRECID_7kPitch;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_pitch(verbose, pitch, error);

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
int mbr_reson7kr_rd_soundvelocity(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_soundvelocity";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_soundvelocity *soundvelocity;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	soundvelocity = &(store->soundvelocity);
	header = &(soundvelocity->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(soundvelocity->soundvelocity)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_SSV;
		store->type = R7KRECID_7kSoundVelocity;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_soundvelocity(verbose, soundvelocity, error);

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
int mbr_reson7kr_rd_absorptionloss(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_absorptionloss";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_absorptionloss *absorptionloss;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	absorptionloss = &(store->absorptionloss);
	header = &(absorptionloss->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(absorptionloss->absorptionloss)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_ABSORPTIONLOSS;
		store->type = R7KRECID_7kAbsorptionLoss;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_absorptionloss(verbose, absorptionloss, error);

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
int mbr_reson7kr_rd_spreadingloss(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_spreadingloss";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_spreadingloss *spreadingloss;
	int	index;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	spreadingloss = &(store->spreadingloss);
	header = &(spreadingloss->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	index = header->Offset + 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(spreadingloss->spreadingloss)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_SPREADINGLOSS;
		store->type = R7KRECID_7kSpreadingLoss;
		
		/* get the time */
		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_spreadingloss(verbose, spreadingloss, error);

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
int mbr_reson7kr_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	FILE	*mbfp;
	char	**bufferptr;
	char	*buffer;
	int	*bufferalloc;
	int	*fileheaders;
	int	size, write_len;
	int	nvalue;
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
	store = (struct mbsys_reson7k_struct *) store_ptr;
	mbfp = mb_io_ptr->mbfp;
	
	/* get saved values */
	bufferptr = (char **) &mb_io_ptr->save5;
	buffer = (char *) *bufferptr;
	bufferalloc = (int *) &mb_io_ptr->save6;
	fileheaders = (int *) &mb_io_ptr->save12;

	/* write fileheader if needed */
	if (status == MB_SUCCESS 
		&& (store->type == R7KRECID_7kFileHeader || *fileheaders == 0))
		{
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr, "Writing record id: %4.4hX | %d", R7KRECID_7kFileHeader, R7KRECID_7kFileHeader);
fprintf(stderr," R7KRECID_7kFileHeader\n");
#endif
		status = mbr_reson7kr_wr_fileheader(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		(*fileheaders)++;
			
		/* write the record to the output file */
		if (status == MB_SUCCESS)
			{
			buffer = (char *) *bufferptr;
			write_len = fwrite(buffer,1,size,mb_io_ptr->mbfp);
			if (write_len != size)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
				}
			}
		}
			
	/* call appropriate writing routines for ping data */
	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA)
		{
		/* Reson 7k volatile sonar settings (record 7000) */
		if (status == MB_SUCCESS && store->read_volatilesettings == MB_YES)
			{
			store->type = R7KRECID_7kVolatileSonarSettings;
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr, "Writing record id: %4.4hX | %d", R7KRECID_7kVolatileSonarSettings, R7KRECID_7kVolatileSonarSettings);
fprintf(stderr," R7KRECID_7kVolatileSonarSettings\n");
#endif
			status = mbr_reson7kr_wr_volatilesonarsettings(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			buffer = (char *) *bufferptr;
			write_len = fwrite(buffer,1,size,mb_io_ptr->mbfp);
			if (write_len != size)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
				}
			}
		/* Reson 7k match filter (record 7002) */
		if (status == MB_SUCCESS && store->read_matchfilter == MB_YES)
			{
			store->type = R7KRECID_7kMatchFilter;
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr, "Writing record id: %4.4hX | %d", R7KRECID_7kMatchFilter, R7KRECID_7kMatchFilter);
fprintf(stderr," R7KRECID_7kMatchFilter\n");
#endif
			status = mbr_reson7kr_wr_matchfilter(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			buffer = (char *) *bufferptr;
			write_len = fwrite(buffer,1,size,mb_io_ptr->mbfp);
			if (write_len != size)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
				}
			}
			
		/* Reson 7k beam geometry (record 7004) */
		if (status == MB_SUCCESS && store->read_beamgeometry == MB_YES)
			{
			store->type = R7KRECID_7kBeamGeometry;
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr, "Writing record id: %4.4hX | %d", R7KRECID_7kBeamGeometry, R7KRECID_7kBeamGeometry);
fprintf(stderr," R7KRECID_7kBeamGeometry\n");
#endif
			status = mbr_reson7kr_wr_beamgeometry(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			buffer = (char *) *bufferptr;
			write_len = fwrite(buffer,1,size,mb_io_ptr->mbfp);
			if (write_len != size)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
				}
			}
			
		/* Reson 7k bathymetry (record 7006) */
		if (status == MB_SUCCESS && store->read_bathymetry == MB_YES)
			{
				store->type = R7KRECID_7kBathymetricData;
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr, "Writing record id: %4.4hX | %d", R7KRECID_7kBathymetricData, R7KRECID_7kBathymetricData);
fprintf(stderr," R7KRECID_7kBathymetricData\n");
#endif
				status = mbr_reson7kr_wr_bathymetry(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
				buffer = (char *) *bufferptr;
			write_len = fwrite(buffer,1,size,mb_io_ptr->mbfp);
				if (write_len != size)
					{
					status = MB_FAILURE;
					*error = MB_ERROR_WRITE_FAIL;
					}
			}
			
		/* Reson 7k backscatter imagery data (record 7007) */
		if (status == MB_SUCCESS && store->read_backscatter == MB_YES)
			{
			store->type = R7KRECID_7kBackscatterImageData;
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr, "Writing record id: %4.4hX | %d", R7KRECID_7kBackscatterImageData, R7KRECID_7kBackscatterImageData);
fprintf(stderr," R7KRECID_7kBackscatterImageData\n");
#endif
			status = mbr_reson7kr_wr_backscatter(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			buffer = (char *) *bufferptr;
			write_len = fwrite(buffer,1,size,mb_io_ptr->mbfp);
			if (write_len != size)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
				}
			}
			
		/* Reson 7k beam data (record 7008) */
		if (status == MB_SUCCESS && store->read_beam == MB_YES)
			{
			store->type = R7KRECID_7kBeamData;
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr, "Writing record id: %4.4hX | %d", R7KRECID_7kBeamData, R7KRECID_7kBeamData);
fprintf(stderr," R7KRECID_7kBeamData\n");
#endif
			status = mbr_reson7kr_wr_beam(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			buffer = (char *) *bufferptr;
			write_len = fwrite(buffer,1,size,mb_io_ptr->mbfp);
			if (write_len != size)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
				}
			}
			
		/* Reson 7k vertical depth (record 7009) */
		if (status == MB_SUCCESS && store->read_verticaldepth == MB_YES)
			{
			store->type = R7KRECID_7kVerticalDepth;
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr, "Writing record id: %4.4hX | %d", R7KRECID_7kVerticalDepth, R7KRECID_7kVerticalDepth);
fprintf(stderr," R7KRECID_7kVerticalDepth\n");
#endif
			status = mbr_reson7kr_wr_verticaldepth(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			buffer = (char *) *bufferptr;
			write_len = fwrite(buffer,1,size,mb_io_ptr->mbfp);
			if (write_len != size)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
				}
			}
			
		/* Reson 7k image data (record 7011) */
		if (status == MB_SUCCESS && store->read_image == MB_YES)
			{
			store->type = R7KRECID_7kImageData;
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr, "Writing record id: %4.4hX | %d", R7KRECID_7kImageData, R7KRECID_7kImageData);
fprintf(stderr," R7KRECID_7kImageData\n");
#endif
			status = mbr_reson7kr_wr_image(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			buffer = (char *) *bufferptr;
			write_len = fwrite(buffer,1,size,mb_io_ptr->mbfp);
			if (write_len != size)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
				}
			}
		}
			
	/* call appropriate writing routine for other records */
	else if (status == MB_SUCCESS && store->type != R7KRECID_7kFileHeader)
		{
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr, "Writing record id: %4.4hX | %d", store->type, store->type);
if (store->type == R7KRECID_ReferencePoint) fprintf(stderr," R7KRECID_ReferencePoint\n");
if (store->type == R7KRECID_UncalibratedSensorOffset) fprintf(stderr," R7KRECID_UncalibratedSensorOffset\n");
if (store->type == R7KRECID_CalibratedSensorOffset) fprintf(stderr," R7KRECID_CalibratedSensorOffset\n");
if (store->type == R7KRECID_Position) fprintf(stderr," R7KRECID_Position\n");
if (store->type == R7KRECID_Attitude) fprintf(stderr," R7KRECID_Attitude\n");
if (store->type == R7KRECID_Tide) fprintf(stderr," R7KRECID_Tide\n");
if (store->type == R7KRECID_Altitude) fprintf(stderr," R7KRECID_Altitude\n");
if (store->type == R7KRECID_MotionOverGround) fprintf(stderr," R7KRECID_MotionOverGround\n");
if (store->type == R7KRECID_Depth) fprintf(stderr," R7KRECID_Depth\n");
if (store->type == R7KRECID_SoundVelocityProfile) fprintf(stderr," R7KRECID_SoundVelocityProfile\n");
if (store->type == R7KRECID_CTD) fprintf(stderr," R7KRECID_CTD\n");
if (store->type == R7KRECID_Geodesy) fprintf(stderr," R7KRECID_Geodesy\n");
if (store->type == R7KRECID_FSDWsidescan)
	{
	if (store->sstype == R7KRECID_FSDWsidescanLo) fprintf(stderr," R7KRECID_FSDWsidescan R7KRECID_FSDWsidescanLo\n");
	if (store->sstype == R7KRECID_FSDWsidescanHi) fprintf(stderr," R7KRECID_FSDWsidescan R7KRECID_FSDWsidescanHi\n");
	}
if (store->type == R7KRECID_FSDWsubbottom) fprintf(stderr," R7KRECID_FSDWsubbottom\n");
if (store->type == R7KRECID_Bluefin) fprintf(stderr," R7KRECID_Bluefin\n");
if (store->type == R7KRECID_7kVolatileSonarSettings) fprintf(stderr," R7KRECID_7kVolatileSonarSettings\n");
if (store->type == R7KRECID_7kConfiguration) fprintf(stderr," R7KRECID_7kConfiguration\n");
if (store->type == R7KRECID_7kMatchFilter) fprintf(stderr," R7KRECID_7kMatchFilter\n");
if (store->type == R7KRECID_7kBeamGeometry) fprintf(stderr," R7KRECID_7kBeamGeometry\n");
if (store->type == R7KRECID_7kCalibrationData) fprintf(stderr," R7KRECID_7kCalibrationData\n");
if (store->type == R7KRECID_7kBathymetricData) fprintf(stderr," R7KRECID_7kBathymetricData\n");
if (store->type == R7KRECID_7kBackscatterImageData) fprintf(stderr," R7KRECID_7kBackscatterImageData\n");
if (store->type == R7KRECID_7kBeamData) fprintf(stderr," R7KRECID_7kBeamData\n");
if (store->type == R7KRECID_7kVerticalDepth) fprintf(stderr," R7KRECID_7kVerticalDepth\n");
if (store->type == R7KRECID_7kImageData) fprintf(stderr," R7KRECID_7kImageData\n");
if (store->type == R7KRECID_7kInstallationParameters) fprintf(stderr," R7KRECID_7kInstallationParameters\n");
if (store->type == R7KRECID_7kSystemEventMessage) fprintf(stderr,"R7KRECID_7kSystemEventMessage\n");
if (store->type == R7KRECID_7kDataStorageStatus) fprintf(stderr," R7KRECID_7kDataStorageStatus\n");
if (store->type == R7KRECID_7kFileHeader) fprintf(stderr," R7KRECID_7kFileHeader\n");
if (store->type == R7KRECID_7kTrigger) fprintf(stderr," R7KRECID_7kTrigger\n");
if (store->type == R7KRECID_7kTriggerSequenceSetup) fprintf(stderr," R7KRECID_7kTriggerSequenceSetup\n");
if (store->type == R7KRECID_7kTriggerSequenceDone) fprintf(stderr," R7KRECID_7kTriggerSequenceDone\n");
if (store->type == R7KRECID_7kTimeMessage) fprintf(stderr," R7KRECID_7kTimeMessage\n");
if (store->type == R7KRECID_7kRemoteControl) fprintf(stderr," R7KRECID_7kRemoteControl\n");
if (store->type == R7KRECID_7kRemoteControlAcknowledge) fprintf(stderr," R7KRECID_7kRemoteControlAcknowledge\n");
if (store->type == R7KRECID_7kRemoteControlNotAcknowledge) fprintf(stderr," R7KRECID_7kRemoteControlNotAcknowledge\n");
if (store->type == R7KRECID_7kRemoteControlSonarSettings) fprintf(stderr," R7KRECID_7kRemoteControlSonarSettings\n");
if (store->type == R7KRECID_7kRoll) fprintf(stderr," R7KRECID_7kRoll\n");
if (store->type == R7KRECID_7kPitch) fprintf(stderr," R7KRECID_7kPitch\n");
if (store->type == R7KRECID_7kSoundVelocity) fprintf(stderr," R7KRECID_7kSoundVelocity\n");
if (store->type == R7KRECID_7kAbsorptionLoss) fprintf(stderr," R7KRECID_7kAbsorptionLoss\n");
if (store->type == R7KRECID_7kSpreadingLoss) fprintf(stderr," R7KRECID_7kSpreadingLoss\n");
if (store->type == R7KRECID_8100SonarData) fprintf(stderr," R7KRECID_8100SonarData\n");
#endif
		if (store->type == R7KRECID_ReferencePoint)
			{
			status = mbr_reson7kr_wr_reference(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_UncalibratedSensorOffset)
			{
			status = mbr_reson7kr_wr_sensoruncal(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_CalibratedSensorOffset)
			{
			status = mbr_reson7kr_wr_sensorcal(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Position)
			{
			status = mbr_reson7kr_wr_position(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_CustomAttitude)
			{
			status = mbr_reson7kr_wr_customattitude(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Tide)
			{
			status = mbr_reson7kr_wr_tide(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Altitude)
			{
			status = mbr_reson7kr_wr_altitude(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_MotionOverGround)
			{
			status = mbr_reson7kr_wr_motion(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Depth)
			{
			status = mbr_reson7kr_wr_depth(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_SoundVelocityProfile)
			{
			status = mbr_reson7kr_wr_svp(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_CTD)
			{
			status = mbr_reson7kr_wr_ctd(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Geodesy)
			{
			status = mbr_reson7kr_wr_geodesy(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_RollPitchHeave)
			{
			status = mbr_reson7kr_wr_rollpitchheave(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Heading)
			{
			status = mbr_reson7kr_wr_heading(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Attitude)
			{
			status = mbr_reson7kr_wr_attitude(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_FSDWsidescan
				&& store->sstype == R7KRECID_FSDWsidescanLo)
			{
			status = mbr_reson7kr_wr_fsdwsslo(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_FSDWsidescan
				&& store->sstype == R7KRECID_FSDWsidescanHi)
			{
			status = mbr_reson7kr_wr_fsdwsshi(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_FSDWsubbottom)
			{
			status = mbr_reson7kr_wr_fsdwsb(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Bluefin)
			{
			status = mbr_reson7kr_wr_bluefin(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_7kVolatileSonarSettings)
			{
			status = mbr_reson7kr_wr_volatilesonarsettings(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_7kConfiguration)
			{
			status = mbr_reson7kr_wr_configuration(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_7kCalibrationData)
			{
			status = mbr_reson7kr_wr_calibration(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_7kInstallationParameters)
			{
			status = mbr_reson7kr_wr_installation(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_7kSystemEventMessage)
			{
			status = mbr_reson7kr_wr_systemeventmessage(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_7kRemoteControlSonarSettings)
			{
			status = mbr_reson7kr_wr_remotecontrolsettings(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_7kRoll)
			{
			status = mbr_reson7kr_wr_roll(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_7kPitch)
			{
			status = mbr_reson7kr_wr_pitch(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_7kSoundVelocity)
			{
			status = mbr_reson7kr_wr_soundvelocity(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_7kAbsorptionLoss)
			{
			status = mbr_reson7kr_wr_absorptionloss(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_7kSpreadingLoss)
			{
			status = mbr_reson7kr_wr_spreadingloss(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
			}
		else
			{
			fprintf(stderr,"call nothing bad kind: %d type %x\n", store->kind, store->type);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_KIND;
			}
			
		/* finally write the record to the output file */
		if (status == MB_SUCCESS)
			{
			buffer = (char *) *bufferptr;
			write_len = fwrite(buffer,1,size,mb_io_ptr->mbfp);
			if (write_len != size)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
				}
			}
		}

#ifdef MBR_RESON7KR_DEBUG
	fprintf(stderr,"RESON7KR DATA WRITTEN: type:%d status:%d error:%d\n\n", 
	store->kind, status, *error);
#endif

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
int mbr_reson7kr_wr_header(int verbose, char *buffer, int *index, 
		s7k_header *header, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_header";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       index:      %d\n",*index);
		fprintf(stderr,"dbg2       header:     %d\n",header);
		}
	
	/* set some important values */
	header->Version = 4;
	header->Offset = 60;
	header->SyncPattern = 0x0000ffff;
	header->Reserved = 0;
	for (i=0;i<8;i++)
		{
		header->PreviousRecord[i] = -1;
		header->NextRecord[i] = -1;
		}
	header->Flags = 0;
	header->Reserved2 = 0;
	
	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_header(verbose, header, error);
  
	/* insert the header */
	mb_put_binary_short(MB_YES, header->Version, &buffer[*index]); *index +=2;
	mb_put_binary_short(MB_YES, header->Offset, &buffer[*index]); *index +=2;
	mb_put_binary_int(MB_YES, header->SyncPattern, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, header->Size, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, header->OffsetToOptionalData, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, header->OptionalDataIdentifier, &buffer[*index]); *index += 4;
	mb_put_binary_short(MB_YES, header->s7kTime.Year, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, header->s7kTime.Day, &buffer[*index]); *index += 2;
	mb_put_binary_float(MB_YES, header->s7kTime.Seconds, &buffer[*index]); *index += 4;
	buffer[*index] = header->s7kTime.Hours; (*index)++;
	buffer[*index] = header->s7kTime.Minutes; (*index)++;
	mb_put_binary_short(MB_YES, header->Reserved, &buffer[*index]); *index += 2;
	mb_put_binary_int(MB_YES, header->RecordType, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, header->DeviceId, &buffer[*index]); *index += 4;
	mb_put_binary_short(MB_YES, header->Reserved2, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, header->SystemEnumerator, &buffer[*index]); *index += 2;
	mb_put_binary_int(MB_YES, header->RecordNumber, &buffer[*index]); *index += 4;
	mb_put_binary_short(MB_YES, header->Flags, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, header->Reserved3, &buffer[*index]); *index += 2;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       index:      %d\n",*index);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_reference(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_reference";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_reference *reference;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	reference = &(store->reference);
	header = &(reference->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_reference(verbose, reference, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_ReferencePoint;

	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		mb_put_binary_float(MB_YES, reference->offset_x, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, reference->offset_y, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, reference->offset_z, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, reference->water_z, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_sensoruncal(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_sensoruncal";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_sensoruncal *sensoruncal;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	sensoruncal = &(store->sensoruncal);
	header = &(sensoruncal->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_sensoruncal(verbose, sensoruncal, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_UncalibratedSensorOffset;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_float(MB_YES, sensoruncal->offset_x, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, sensoruncal->offset_y, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, sensoruncal->offset_z, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, sensoruncal->offset_roll, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, sensoruncal->offset_pitch, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, sensoruncal->offset_yaw, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_sensorcal(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_sensorcal";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_sensorcal *sensorcal;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	sensorcal = &(store->sensorcal);
	header = &(sensorcal->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_sensorcal(verbose, sensorcal, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_CalibratedSensorOffset;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_float(MB_YES, sensorcal->offset_x, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, sensorcal->offset_y, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, sensorcal->offset_z, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, sensorcal->offset_roll, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, sensorcal->offset_pitch, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, sensorcal->offset_yaw, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_position(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_position";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_position *position;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	position = &(store->position);
	header = &(position->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_position(verbose, position, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_Position;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_int(MB_YES, position->datum, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, position->latency, &buffer[index]); index += 4;
		mb_put_binary_double(MB_YES, position->latitude, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, position->longitude, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, position->height, &buffer[index]); index += 8;
		buffer[index] = position->type; index++;
		buffer[index] = position->utm_zone; index++;
		buffer[index] = position->quality; index++;
		buffer[index] = position->method; index++;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_customattitude(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_customattitude";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_customattitude *customattitude;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	customattitude = &(store->customattitude);
	header = &(customattitude->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_customattitude(verbose, customattitude, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_CustomAttitude;
	if (customattitude->bitfield & 1)
		*size += customattitude->n * sizeof(float);
	if (customattitude->bitfield & 2)
		*size += customattitude->n * sizeof(float);
	if (customattitude->bitfield & 4)
		*size += customattitude->n * sizeof(float);
	if (customattitude->bitfield & 8)
		*size += customattitude->n * sizeof(float);
	if (customattitude->bitfield & 16)
		*size += customattitude->n * sizeof(float);
	if (customattitude->bitfield & 32)
		*size += customattitude->n * sizeof(float);
	if (customattitude->bitfield & 64)
		*size += customattitude->n * sizeof(float);
	if (customattitude->bitfield & 128)
		*size += customattitude->n * sizeof(float);
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		customattitude->bitfield = (mb_u_char) buffer[index]; index++;
		customattitude->reserved = (mb_u_char) buffer[index]; index++;
		mb_put_binary_short(MB_YES, customattitude->n, &buffer[index]); index += 2;
		mb_put_binary_float(MB_YES, customattitude->frequency, &buffer[index]); index += 4;

		if (customattitude->bitfield & 1)
		for (i=0;i<customattitude->n;i++)
			{
			mb_put_binary_float(MB_YES, customattitude->pitch[i], &buffer[index]); index += 4;
			}
		if (customattitude->bitfield & 2)
		for (i=0;i<customattitude->n;i++)
			{
			mb_put_binary_float(MB_YES, customattitude->roll[i], &buffer[index]); index += 4;
			}
		if (customattitude->bitfield & 4)
		for (i=0;i<customattitude->n;i++)
			{
			mb_put_binary_float(MB_YES, customattitude->heading[i], &buffer[index]); index += 4;
			}
		if (customattitude->bitfield & 8)
		for (i=0;i<customattitude->n;i++)
			{
			mb_put_binary_float(MB_YES, customattitude->heave[i], &buffer[index]); index += 4;
			}
		if (customattitude->bitfield & 16)
		for (i=0;i<customattitude->n;i++)
			{
			mb_put_binary_float(MB_YES, customattitude->pitchrate[i], &buffer[index]); index += 4;
			}
		if (customattitude->bitfield & 32)
		for (i=0;i<customattitude->n;i++)
			{
			mb_put_binary_float(MB_YES, customattitude->rollrate[i], &buffer[index]); index += 4;
			}
		if (customattitude->bitfield & 64)
		for (i=0;i<customattitude->n;i++)
			{
			mb_put_binary_float(MB_YES, customattitude->headingrate[i], &buffer[index]); index += 4;
			}
		if (customattitude->bitfield & 128)
		for (i=0;i<customattitude->n;i++)
			{
			mb_put_binary_float(MB_YES, customattitude->heaverate[i], &buffer[index]); index += 4;
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_tide(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_tide";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_tide *tide;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	tide = &(store->tide);
	header = &(tide->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_tide(verbose, tide, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_Tide;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_float(MB_YES, tide->tide, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, tide->source, &buffer[index]); index += 2;
		buffer[index] = tide->flags; index++;
		mb_get_binary_short(MB_YES, &buffer[index], &(tide->gauge)); index += 2;
		mb_get_binary_int(MB_YES, &buffer[index], &(tide->datum)); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(tide->latency)); index += 4;
		mb_get_binary_double(MB_YES, &buffer[index], &(tide->latitude)); index += 8;
		mb_get_binary_double(MB_YES, &buffer[index], &(tide->longitude)); index += 8;
		mb_get_binary_double(MB_YES, &buffer[index], &(tide->height)); index += 8;
		buffer[index] = tide->type; index++;
		buffer[index] = tide->utm_zone; index++;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_altitude(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_altitude";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_altitude *altitude;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	altitude = &(store->altitude);
	header = &(altitude->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_altitude(verbose, altitude, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_Altitude;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_float(MB_YES, altitude->altitude, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_motion(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_motion";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_motion *motion;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	motion = &(store->motion);
	header = &(motion->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_motion(verbose, motion, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_MotionOverGround;
	if (motion->bitfield & 1)
		*size += 3 * motion->n * sizeof(float);
	if (motion->bitfield & 2)
		*size += 3 * motion->n * sizeof(float);
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		motion->bitfield = (mb_u_char) buffer[index]; index++;
		motion->reserved = (mb_u_char) buffer[index]; index++;
		mb_put_binary_short(MB_YES, motion->n, &buffer[index]); index += 2;
		mb_put_binary_float(MB_YES, motion->frequency, &buffer[index]); index += 4;

		if (motion->bitfield & 1)
			{
			for (i=0;i<motion->n;i++)
				{
				mb_put_binary_float(MB_YES, motion->x[i], &buffer[index]); index += 4;
				}
			for (i=0;i<motion->n;i++)
				{
				mb_put_binary_float(MB_YES, motion->y[i], &buffer[index]); index += 4;
				}
			for (i=0;i<motion->n;i++)
				{
				mb_put_binary_float(MB_YES, motion->z[i], &buffer[index]); index += 4;
				}
			}
		if (motion->bitfield & 2)
			{
			for (i=0;i<motion->n;i++)
				{
				mb_put_binary_float(MB_YES, motion->xa[i], &buffer[index]); index += 4;
				}
			for (i=0;i<motion->n;i++)
				{
				mb_put_binary_float(MB_YES, motion->ya[i], &buffer[index]); index += 4;
				}
			for (i=0;i<motion->n;i++)
				{
				mb_put_binary_float(MB_YES, motion->za[i], &buffer[index]); index += 4;
				}
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_depth(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_depth";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_depth *depth;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	depth = &(store->depth);
	header = &(depth->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_depth(verbose, depth, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_Depth;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		depth->descriptor = (mb_u_char) buffer[index]; index++;
		depth->correction = (mb_u_char) buffer[index]; index++;
		mb_put_binary_short(MB_YES, depth->reserved, &buffer[index]); index += 2;
		mb_put_binary_float(MB_YES, depth->depth, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_svp(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_svp";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_svp *svp;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	svp = &(store->svp);
	header = &(svp->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_svp(verbose, svp, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_SoundVelocityProfile;
	*size += R7KRDTSIZE_SoundVelocityProfile * svp->n;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		svp->position_flag = (mb_u_char) buffer[index]; index++;
		svp->reserved1 = (mb_u_char) buffer[index]; index++;
		mb_put_binary_short(MB_YES, svp->reserved2, &buffer[index]); index += 2;
		mb_put_binary_double(MB_YES, svp->latitude, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, svp->longitude, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, svp->n, &buffer[index]); index += 4;

		for (i=0;i<svp->n;i++)
			{
			mb_put_binary_float(MB_YES, svp->depth[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, svp->sound_velocity[i], &buffer[index]); index += 4;
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_ctd(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_ctd";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_ctd *ctd;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	ctd = &(store->ctd);
	header = &(ctd->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_ctd(verbose, ctd, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_CTD;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_float(MB_YES, ctd->frequency, &buffer[index]); index += 4;
		buffer[index] = ctd->velocity_source_flag; index++;
		buffer[index] = ctd->velocity_algorithm; index++;
		buffer[index] = ctd->conductivity_flag; index++;
		buffer[index] = ctd->pressure_flag; index++;
		buffer[index] = ctd->position_flag; index++;
		buffer[index] = ctd->validity; index++;
		mb_put_binary_short(MB_YES, ctd->reserved, &buffer[index]); index += 2;
		mb_put_binary_double(MB_YES, ctd->latitude, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, ctd->longitude, &buffer[index]); index += 8;
		mb_put_binary_float(MB_YES, ctd->sample_rate, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, ctd->n, &buffer[index]); index += 4;

		for (i=0;i<ctd->n;i++)
			{
			mb_put_binary_float(MB_YES, ctd->conductivity_salinity[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, ctd->temperature[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, ctd->pressure_depth[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, ctd->sound_velocity[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, ctd->absorption[i], &buffer[index]); index += 4;
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_geodesy(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_geodesy";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_geodesy *geodesy;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	geodesy = &(store->geodesy);
	header = &(geodesy->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_geodesy(verbose, geodesy, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_Geodesy;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		for (i=0;i<32;i++)
			{
			geodesy->spheroid[i] = (mb_u_char) buffer[index]; index++;
			}
		mb_put_binary_double(MB_YES, geodesy->semimajoraxis, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, geodesy->flattening, &buffer[index]); index += 8;
		for (i=0;i<16;i++)
			{
			geodesy->reserved1[i] = (mb_u_char) buffer[index]; index++;
			}
		for (i=0;i<32;i++)
			{
			geodesy->datum[i] = (mb_u_char) buffer[index]; index++;
			}
		mb_put_binary_int(MB_YES, geodesy->calculation_method, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, geodesy->number_parameters, &buffer[index]); index += 4;
		mb_put_binary_double(MB_YES, geodesy->dx, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, geodesy->dy, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, geodesy->dz, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, geodesy->rx, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, geodesy->ry, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, geodesy->rz, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, geodesy->scale, &buffer[index]); index += 8;
		for (i=0;i<35;i++)
			{
			geodesy->reserved2[i] = (mb_u_char) buffer[index]; index++;
			}
		for (i=0;i<32;i++)
			{
			geodesy->grid_name[i] = (mb_u_char) buffer[index]; index++;
			}
		geodesy->distance_units = (mb_u_char) buffer[index]; index++;
		geodesy->angular_units = (mb_u_char) buffer[index]; index++;
		mb_put_binary_double(MB_YES, geodesy->latitude_origin, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, geodesy->central_meriidan, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, geodesy->false_easting, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, geodesy->false_northing, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, geodesy->central_scale_factor, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, geodesy->custum_identifier, &buffer[index]); index += 4;
		for (i=0;i<50;i++)
			{
			geodesy->reserved3[i] = (mb_u_char) buffer[index]; index++;
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
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
int mbr_reson7kr_wr_rollpitchheave(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_rollpitchheave";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_rollpitchheave *rollpitchheave;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	rollpitchheave = &(store->rollpitchheave);
	header = &(rollpitchheave->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_rollpitchheave(verbose, rollpitchheave, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_RollPitchHeave;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_float(MB_YES, rollpitchheave->roll, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, rollpitchheave->pitch, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, rollpitchheave->heave, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
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
int mbr_reson7kr_wr_heading(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_heading";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_heading *heading;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	heading = &(store->heading);
	header = &(heading->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_heading(verbose, heading, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_Heading;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_float(MB_YES, heading->heading, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
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
int mbr_reson7kr_wr_attitude(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_attitude";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_attitude *attitude;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	attitude = &(store->attitude);
	header = &(attitude->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_attitude(verbose, attitude, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_Attitude;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		buffer[index] = attitude->n; index++;
		for (i=0;i<attitude->n;i++)
			{
			mb_put_binary_short(MB_YES, attitude->delta_time[i], &buffer[index]); index += 2;
			mb_put_binary_float(MB_YES, attitude->roll[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, attitude->pitch[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, attitude->heave[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, attitude->heading[i], &buffer[index]); index += 4;
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
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
int mbr_reson7kr_wr_fsdwchannel(int verbose, int data_format, char *buffer, int *index, 
		s7k_fsdwchannel *fsdwchannel, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_fsdwchannel";
	int	status = MB_SUCCESS;
	int	data_size;
	short	*shortptr;
	unsigned short	*ushortptr;
	int	i;
short *srptr,*ssptr;
unsigned short *urptr,*usptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       data_format:%d\n",data_format);
		fprintf(stderr,"dbg2       index:      %d\n",*index);
		fprintf(stderr,"dbg2       fsdwchannel:%d\n",fsdwchannel);
		}
	
	/* print out the data to be output */
	/*mbsys_reson7k_print_fsdwchannel(verbose, data_format, fsdwchannel, error);*/
	
	/* extract the channel header */
	buffer[*index] = fsdwchannel->number; (*index)++;
	buffer[*index] = fsdwchannel->type; (*index)++;
	buffer[*index] = fsdwchannel->data_type; (*index)++;
	buffer[*index] = fsdwchannel->polarity; (*index)++;
	buffer[*index] = fsdwchannel->bytespersample; (*index)++;
	for (i=0;i<3;i++)
		{
		buffer[*index] = fsdwchannel->reserved1[i]; (*index)++;
		}
	mb_put_binary_int(MB_YES, fsdwchannel->number_samples, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, fsdwchannel->start_time, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, fsdwchannel->sample_interval, &buffer[*index]); *index += 4;
	mb_put_binary_float(MB_YES, fsdwchannel->range, &buffer[*index]); *index += 4;
	mb_put_binary_float(MB_YES, fsdwchannel->voltage, &buffer[*index]); *index += 4;
	for (i=0;i<16;i++)
		{
		buffer[*index] = fsdwchannel->name[i]; (*index)++;
		}
	for (i=0;i<20;i++)
		{
		buffer[*index] = fsdwchannel->reserved2[i]; (*index)++;
		}
			
	/* copy over the data */
	if (status == MB_SUCCESS)
		{
		if (fsdwchannel->bytespersample == 1)
			{
			for (i=0;i<fsdwchannel->number_samples;i++)
				{
				buffer[*index] = fsdwchannel->data[i]; (*index)++;
				}
			}
		else if (fsdwchannel->bytespersample == 2)
			{
			shortptr = (short *) fsdwchannel->data;
			for (i=0;i<fsdwchannel->number_samples;i++)
				{
/*srptr = (short *) &(buffer[*index]);
urptr = (unsigned short *) &(buffer[*index]);*/
				mb_put_binary_short(MB_YES, shortptr[i], &buffer[*index]); *index += 2;
/*ssptr = (short *) &(shortptr[i]);
usptr = (unsigned short *) &(shortptr[i]);
fprintf(stderr,"sample:%5d   raw:%6d %6d   swapped:%6d %6d\n",
i,*srptr,*urptr,*ssptr,*usptr);*/
				}
			}
		else if (fsdwchannel->bytespersample == 4)
			{
			shortptr = (short *) fsdwchannel->data;
			for (i=0;i<fsdwchannel->number_samples;i++)
				{
/*srptr = (short *) &(buffer[*index]);
urptr = (unsigned short *) &(buffer[*index]);*/
				mb_put_binary_short(MB_YES, shortptr[2*i], &buffer[*index]); *index += 2;
/*ssptr = (short *) &(shortptr[2*i]);
usptr = (unsigned short *) &(shortptr[2*i]);
fprintf(stderr,"sample:%5d   IMAGINARY: raw:%6d %6d   swapped:%6d %6d",
i,*srptr,*urptr,*ssptr,*usptr);
srptr = (short *) &(buffer[*index]);
urptr = (unsigned short *) &(buffer[*index]);*/
				mb_put_binary_short(MB_YES, shortptr[2*i+1], &buffer[*index]); *index += 2;
/*ssptr = (short *) &(shortptr[2*i+1]);
usptr = (unsigned short *) &(shortptr[2*i+1]);
fprintf(stderr,"    REAL: raw:%6d %6d   swapped:%6d %6d\n",
*srptr,*urptr,*ssptr,*usptr);*/
				}
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       index:      %d\n",*index);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fsdwssheader(int verbose, char *buffer, int *index, 
		s7k_fsdwssheader *fsdwssheader, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_fsdwssheader";
	int	status = MB_SUCCESS;
	int	data_size;
	short	*shortptr;
	int	*intptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:         %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:         %d\n",buffer);
		fprintf(stderr,"dbg2       index:          %d\n",*index);
		fprintf(stderr,"dbg2       fsdwssheader:   %d\n",fsdwssheader);
		}
	
	/* print out the data to be output */
	/*mbsys_reson7k_print_fsdwssheader(verbose, fsdwssheader, error);*/
	
	/* extract the Edgetech sidescan header */
	mb_put_binary_short(MB_YES, fsdwssheader->subsystem, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->channelNum, &buffer[*index]); *index += 2;
	mb_put_binary_int(MB_YES, fsdwssheader->pingNum, &buffer[*index]); *index += 4;
	mb_put_binary_short(MB_YES, fsdwssheader->packetNum, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->trigSource, &buffer[*index]); *index += 2;
	mb_put_binary_int(MB_YES, fsdwssheader->samples, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, fsdwssheader->sampleInterval, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, fsdwssheader->startDepth, &buffer[*index]); *index += 4;
	mb_put_binary_short(MB_YES, fsdwssheader->weightingFactor, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->ADCGain, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->ADCMax, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->rangeSetting, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->pulseID, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->markNumber, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->dataFormat, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->reserved, &buffer[*index]); *index += 2;
	mb_put_binary_int(MB_YES, fsdwssheader->millisecondsToday, &buffer[*index]); *index += 4;
	mb_put_binary_short(MB_YES, fsdwssheader->year, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->day, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->hour, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->minute, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->second, &buffer[*index]); *index += 2;   
	mb_put_binary_short(MB_YES, fsdwssheader->heading, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->pitch, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->roll, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->heave, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwssheader->yaw, &buffer[*index]); *index += 2;
	mb_put_binary_int(MB_YES, fsdwssheader->depth, &buffer[*index]); *index += 4;
	mb_put_binary_short(MB_YES, fsdwssheader->temperature, &buffer[*index]); *index += 2;
	for (i=0;i<10;i++)
		{
		buffer[*index] = fsdwssheader->reserved2[i]; (*index)++;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       index:      %d\n",*index);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fsdwsegyheader(int verbose, char *buffer, int *index, 
		s7k_fsdwsegyheader *fsdwsegyheader, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_fsdwsegyheader";
	int	status = MB_SUCCESS;
	int	data_size;
	short	*shortptr;
	int	*intptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:         %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:         %d\n",buffer);
		fprintf(stderr,"dbg2       index:          %d\n",*index);
		fprintf(stderr,"dbg2       fsdwsegyheader: %d\n",fsdwsegyheader);
		}
	
	/* print out the data to be output */
	/*mbsys_reson7k_print_fsdwsegyheader(verbose, fsdwsegyheader, error);*/
	
	/* extract the Edgetech segy header */
	mb_put_binary_int(MB_YES, fsdwsegyheader->sequenceNumber, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, fsdwsegyheader->startDepth, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, fsdwsegyheader->pingNum, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, fsdwsegyheader->channelNum, &buffer[*index]); *index += 4;
	for (i=0;i<6;i++)
		{
		mb_put_binary_short(MB_YES, fsdwsegyheader->unused1[i], &buffer[*index]); *index += 2;
		}
	mb_put_binary_short(MB_YES, fsdwsegyheader->traceIDCode, &buffer[*index]); *index += 2;
	for (i=0;i<2;i++)
		{
		mb_put_binary_short(MB_YES, fsdwsegyheader->unused2[i], &buffer[*index]); *index += 2;
		}
	mb_put_binary_short(MB_YES, fsdwsegyheader->dataFormat, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->NMEAantennaeR, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->NMEAantennaeO, &buffer[*index]); *index += 2;
	for (i=0;i<32;i++)
		{
		buffer[*index] = fsdwsegyheader->RS232[i]; (*index)++;
		}
	mb_put_binary_int(MB_YES, fsdwsegyheader->sourceCoordX, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, fsdwsegyheader->sourceCoordY, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, fsdwsegyheader->groupCoordX, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, fsdwsegyheader->groupCoordY, &buffer[*index]); *index += 4;
	mb_put_binary_short(MB_YES, fsdwsegyheader->coordUnits, &buffer[*index]); *index += 2;
	for (i=0;i<24;i++)
		{
		buffer[*index] = fsdwsegyheader->annotation[i]; (*index)++;
		}
 	mb_put_binary_short(MB_YES, fsdwsegyheader->samples, &buffer[*index]); *index += 2;
	mb_put_binary_int(MB_YES, fsdwsegyheader->sampleInterval, &buffer[*index]); *index += 4;
 	mb_put_binary_short(MB_YES, fsdwsegyheader->ADCGain, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->pulsePower, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->correlated, &buffer[*index]); *index += 2;
 	mb_put_binary_short(MB_YES, fsdwsegyheader->startFreq, &buffer[*index]); *index += 2;
 	mb_put_binary_short(MB_YES, fsdwsegyheader->endFreq, &buffer[*index]); *index += 2;
 	mb_put_binary_short(MB_YES, fsdwsegyheader->sweepLength, &buffer[*index]); *index += 2;
	for (i=0;i<4;i++)
		{
		mb_put_binary_short(MB_YES, fsdwsegyheader->unused7[i], &buffer[*index]); *index += 2;
		}
 	mb_put_binary_short(MB_YES, fsdwsegyheader->aliasFreq, &buffer[*index]); *index += 2;
 	mb_put_binary_short(MB_YES, fsdwsegyheader->pulseID, &buffer[*index]); *index += 2;
	for (i=0;i<6;i++)
		{
		mb_put_binary_short(MB_YES, fsdwsegyheader->unused8[i], &buffer[*index]); *index += 2;
		}
	mb_put_binary_short(MB_YES, fsdwsegyheader->year, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->day, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->hour, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->minute, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->second, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->timeBasis, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->weightingFactor, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->unused9, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->heading, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->pitch, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->roll, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->temperature, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->heaveCompensation, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->trigSource, &buffer[*index]); *index += 2;
 	mb_put_binary_short(MB_YES, fsdwsegyheader->markNumber, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->NMEAHour, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->NMEAMinutes, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->NMEASeconds, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->NMEACourse, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->NMEASpeed, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->NMEADay, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->NMEAYear, &buffer[*index]); *index += 2;
	mb_put_binary_int(MB_YES, fsdwsegyheader->millisecondsToday, &buffer[*index]); *index += 4;
 	mb_put_binary_short(MB_YES, fsdwsegyheader->ADCMax, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->calConst, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->vehicleID, &buffer[*index]); *index += 2;
	for (i=0;i<6;i++)
		{
		buffer[*index] = fsdwsegyheader->softwareVersion[i]; (*index)++;
		}
	mb_put_binary_int(MB_YES, fsdwsegyheader->sphericalCorrection, &buffer[*index]); *index += 4;
 	mb_put_binary_short(MB_YES, fsdwsegyheader->packetNum, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->ADCDecimation, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, fsdwsegyheader->decimation, &buffer[*index]); *index += 2;
	for (i=0;i<7;i++)
		{
		mb_put_binary_short(MB_YES, fsdwsegyheader->unuseda[i], &buffer[*index]); *index += 2;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       index:      %d\n",*index);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fsdwsslo(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_fsdwsslo";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_fsdwss *fsdwsslo;
	s7k_fsdwchannel *fsdwchannel;
	s7k_fsdwssheader *fsdwssheader;
	s7kr_bathymetry *bathymetry;
	s7kr_bluefin *bluefin;
	double	bin[MBSYS_RESON7K_MAX_PIXELS];
	int	nbin[MBSYS_RESON7K_MAX_PIXELS];
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int 	bottompick;
	double	range, sign, sound_speed, half_sound_speed;
	double	tracemax;
	double	xtrack;
	double	pixelsize;
	double	pickvalue, value;
	short	*shortptr;
	unsigned short	*ushortptr;
	int	jstart, jend;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	fsdwsslo = &(store->fsdwsslo);
	header = &(fsdwsslo->header);
	bathymetry = &(store->bathymetry);
	bluefin = &(store->bluefin);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_fsdwss(verbose, fsdwsslo, error);
#ifdef MBR_RESON7KR_DEBUG
for (i=0;i<fsdwsslo->number_channels;i++)
{
fsdwchannel = &(fsdwsslo->channel[i]);
fsdwssheader = &(fsdwsslo->ssheader[i]);
fprintf(stderr,"SSLO: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d %d chan:%d %d sampint:%d %d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
fsdwssheader->year,fsdwssheader->day,fsdwssheader->hour,fsdwssheader->minute,fsdwssheader->second,
fsdwssheader->millisecondsToday - 1000 * (int)(0.001 * fsdwssheader->millisecondsToday),
fsdwsslo->ping_number,fsdwssheader->pingNum,
fsdwchannel->number,fsdwssheader->channelNum,
fsdwchannel->sample_interval,fsdwssheader->sampleInterval);
}
#endif
		
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_FSDWsidescan;
	for (i=0;i<fsdwsslo->number_channels;i++)
		{
		*size += R7KHDRSIZE_FSDWchannelinfo;
		*size += R7KHDRSIZE_FSDWssheader;
		fsdwchannel = &(fsdwsslo->channel[i]);
		*size += fsdwchannel->bytespersample * fsdwchannel->number_samples;
		}
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_int(MB_YES, fsdwsslo->msec_timestamp, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, fsdwsslo->ping_number, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, fsdwsslo->number_channels, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, fsdwsslo->total_bytes, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, fsdwsslo->data_format, &buffer[index]);index += 4;
		index += 12;
		for (i=0;i<2;i++)
			{
			fsdwchannel = &(fsdwsslo->channel[i]);
			mbr_reson7kr_wr_fsdwchannel(verbose, fsdwsslo->data_format, buffer, &index, fsdwchannel, error);
			}
	/*fprintf(stderr,"In mbr_reson7kr_wr_fsdwsslo: index:%d OffsetToOptionalData:%d\n", 
	index, header->OffsetToOptionalData);
		index = header->OffsetToOptionalData;*/
		for (i=0;i<2;i++)
			{
			fsdwssheader = &(fsdwsslo->ssheader[i]);
			mbr_reson7kr_wr_fsdwssheader(verbose, buffer, &index, fsdwssheader, error);
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}
	
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fsdwsshi(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_fsdwsshi";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_fsdwss *fsdwsshi;
	s7k_fsdwchannel *fsdwchannel;
	s7k_fsdwssheader *fsdwssheader;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	fsdwsshi = &(store->fsdwsshi);
	header = &(fsdwsshi->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_fsdwss(verbose, fsdwsshi, error);
#ifdef MBR_RESON7KR_DEBUG
for (i=0;i<fsdwsshi->number_channels;i++)
{
fsdwchannel = &(fsdwsshi->channel[i]);
fsdwssheader = &(fsdwsshi->ssheader[i]);
fprintf(stderr,"SSHI: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d %d chan:%d %d sampint:%d %d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
fsdwssheader->year,fsdwssheader->day,fsdwssheader->hour,fsdwssheader->minute,fsdwssheader->second,
fsdwssheader->millisecondsToday - 1000 * (int)(0.001 * fsdwssheader->millisecondsToday),
fsdwsshi->ping_number,fsdwssheader->pingNum,
fsdwchannel->number,fsdwssheader->channelNum,
fsdwchannel->sample_interval,fsdwssheader->sampleInterval);
}
#endif
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_FSDWsidescan;
	for (i=0;i<fsdwsshi->number_channels;i++)
		{
		*size += R7KHDRSIZE_FSDWchannelinfo;
		*size += R7KHDRSIZE_FSDWssheader;
		fsdwchannel = &(fsdwsshi->channel[i]);
		*size += fsdwchannel->bytespersample * fsdwchannel->number_samples;
		}
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_int(MB_YES, fsdwsshi->msec_timestamp, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, fsdwsshi->ping_number, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, fsdwsshi->number_channels, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, fsdwsshi->total_bytes, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, fsdwsshi->data_format, &buffer[index]);index += 4;
		index += 12;
		for (i=0;i<2;i++)
			{
			fsdwchannel = &(fsdwsshi->channel[i]);
			mbr_reson7kr_wr_fsdwchannel(verbose, fsdwsshi->data_format, buffer, &index, fsdwchannel, error);
			}
	/*fprintf(stderr,"In mbr_reson7kr_wr_fsdwsshi: index:%d OffsetToOptionalData:%d\n", 
	index, header->OffsetToOptionalData);
		index = header->OffsetToOptionalData;*/
		for (i=0;i<2;i++)
			{
			fsdwssheader = &(fsdwsshi->ssheader[i]);
			mbr_reson7kr_wr_fsdwssheader(verbose, buffer, &index, fsdwssheader, error);
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fsdwsb(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_fsdwsb";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_fsdwsb *fsdwsb;
	s7k_fsdwchannel *fsdwchannel;
	s7k_fsdwsegyheader *fsdwsegyheader;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	fsdwsb = &(store->fsdwsb);
	header = &(fsdwsb->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_fsdwsb(verbose, fsdwsb, error);
#ifdef MBR_RESON7KR_DEBUG
for (i=0;i<fsdwsb->number_channels;i++)
{
fsdwchannel = &(fsdwsb->channel);
fsdwsegyheader = &(fsdwsb->segyheader);
fprintf(stderr,"SBP:  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d %d chan:%d %d sampint:%d %d\n",
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],store->time_i[5],store->time_i[6],
fsdwsegyheader->year,fsdwsegyheader->day,fsdwsegyheader->hour,fsdwsegyheader->minute,fsdwsegyheader->second,
fsdwsegyheader->millisecondsToday - 1000 * (int)(0.001 * fsdwsegyheader->millisecondsToday),
fsdwsb->ping_number,fsdwsegyheader->pingNum,
fsdwchannel->number,fsdwsegyheader->channelNum,
fsdwchannel->sample_interval,fsdwsegyheader->sampleInterval);
}
#endif
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_FSDWsubbottom;
		
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_FSDWsubbottom;
	for (i=0;i<fsdwsb->number_channels;i++)
		{
		*size += R7KHDRSIZE_FSDWchannelinfo;
		*size += R7KHDRSIZE_FSDWsbheader;
		fsdwchannel = &(fsdwsb->channel);
		*size += fsdwchannel->bytespersample * fsdwchannel->number_samples;
		}
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_int(MB_YES, fsdwsb->msec_timestamp, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, fsdwsb->ping_number, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, fsdwsb->number_channels, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, fsdwsb->total_bytes, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, fsdwsb->data_format, &buffer[index]);index += 4;
		index += 12;
		fsdwchannel = &(fsdwsb->channel);
		mbr_reson7kr_wr_fsdwchannel(verbose, fsdwsb->data_format, buffer, &index, fsdwchannel, error);
/* fprintf(stderr,"index:%d offset:%d\n",index,header->OffsetToOptionalData);*/
		fsdwsegyheader = &(fsdwsb->segyheader);
		mbr_reson7kr_wr_fsdwsegyheader(verbose, buffer, &index, fsdwsegyheader, error);

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_bluefin(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_bluefin";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_bluefin *bluefin;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	bluefin = &(store->bluefin);
	header = &(bluefin->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_bluefin(verbose, bluefin, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_BluefinDataFrame;
	*size += bluefin->number_frames * bluefin->frame_size;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_int(MB_YES, bluefin->msec_timestamp, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, bluefin->number_frames, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, bluefin->frame_size, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, bluefin->data_format, &buffer[index]); index += 4;
		for (i=0;i<16;i++)
			{
			buffer[index] = bluefin->reserved[i]; index++;
			}
		if (bluefin->data_format == R7KRECID_BluefinNav)
			{
			for (i=0;i<bluefin->number_frames;i++)
				{
				mb_put_binary_int(MB_YES, bluefin->nav[i].packet_size, &buffer[index]); index += 4;
				mb_put_binary_short(MB_YES, bluefin->nav[i].version, &buffer[index]); index += 2;
				mb_put_binary_short(MB_YES, bluefin->nav[i].offset, &buffer[index]); index += 2;
				mb_put_binary_int(MB_YES, bluefin->nav[i].data_type, &buffer[index]); index += 4;
				mb_put_binary_int(MB_YES, bluefin->nav[i].data_size, &buffer[index]); index += 4;
				mb_put_binary_short(MB_YES, bluefin->nav[i].s7kTime.Year, &buffer[index]); index += 2;
				mb_put_binary_short(MB_YES, bluefin->nav[i].s7kTime.Day, &buffer[index]); index += 2;
				mb_put_binary_float(MB_YES, bluefin->nav[i].s7kTime.Seconds, &buffer[index]); index += 4;
				bluefin->nav[i].s7kTime.Hours = (mb_u_char) buffer[index]; (index)++;
				bluefin->nav[i].s7kTime.Minutes = (mb_u_char) buffer[index]; (index)++;
				mb_put_binary_int(MB_YES, bluefin->nav[i].checksum, &buffer[index]); index += 4;
				mb_put_binary_short(MB_YES, bluefin->nav[i].reserved, &buffer[index]); index += 2;
				mb_put_binary_int(MB_YES, bluefin->nav[i].quality, &buffer[index]); index += 4;
				mb_put_binary_double(MB_YES, bluefin->nav[i].latitude, &buffer[index]); index += 8;
				mb_put_binary_double(MB_YES, bluefin->nav[i].longitude, &buffer[index]); index += 8;
				mb_put_binary_float(MB_YES, bluefin->nav[i].speed, &buffer[index]); index += 4;
				mb_put_binary_double(MB_YES, bluefin->nav[i].depth, &buffer[index]); index += 8;
				mb_put_binary_double(MB_YES, bluefin->nav[i].altitude, &buffer[index]); index += 8;
				mb_put_binary_float(MB_YES, bluefin->nav[i].roll, &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bluefin->nav[i].pitch, &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bluefin->nav[i].yaw, &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bluefin->nav[i].northing_rate, &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bluefin->nav[i].easting_rate, &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bluefin->nav[i].depth_rate, &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bluefin->nav[i].altitude_rate, &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bluefin->nav[i].roll_rate, &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bluefin->nav[i].pitch_rate, &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bluefin->nav[i].yaw_rate, &buffer[index]); index += 4;
				mb_put_binary_double(MB_YES, bluefin->nav[i].position_time, &buffer[index]); index += 8;
				mb_put_binary_double(MB_YES, bluefin->nav[i].altitude_time, &buffer[index]); index += 8;
				}
			}
		else if (bluefin->data_format == R7KRECID_BluefinEnvironmental)
			{
			for (i=0;i<bluefin->number_frames;i++)
				{
				mb_put_binary_int(MB_YES, bluefin->environmental[i].packet_size, &buffer[index]); index += 4;
				mb_put_binary_short(MB_YES, bluefin->environmental[i].version, &buffer[index]); index += 2;
				mb_put_binary_short(MB_YES, bluefin->environmental[i].offset, &buffer[index]); index += 2;
				mb_put_binary_int(MB_YES, bluefin->environmental[i].data_type, &buffer[index]); index += 4;
				mb_put_binary_int(MB_YES, bluefin->environmental[i].data_size, &buffer[index]); index += 4;
				mb_put_binary_short(MB_YES, bluefin->environmental[i].s7kTime.Year, &buffer[index]); index += 2;
				mb_put_binary_short(MB_YES, bluefin->environmental[i].s7kTime.Day, &buffer[index]); index += 2;
				mb_put_binary_float(MB_YES, bluefin->environmental[i].s7kTime.Seconds, &buffer[index]); index += 4;
				bluefin->environmental[i].s7kTime.Hours = (mb_u_char) buffer[index]; (index)++;
				bluefin->environmental[i].s7kTime.Minutes = (mb_u_char) buffer[index]; (index)++;
				mb_put_binary_int(MB_YES, bluefin->environmental[i].checksum, &buffer[index]); index += 4;
				mb_put_binary_short(MB_YES, bluefin->environmental[i].reserved1, &buffer[index]); index += 2;
				mb_put_binary_int(MB_YES, bluefin->environmental[i].quality, &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bluefin->environmental[i].sound_speed, &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bluefin->environmental[i].conductivity, &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bluefin->environmental[i].temperature, &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bluefin->environmental[i].pressure, &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bluefin->environmental[i].salinity, &buffer[index]); index += 4;
				mb_put_binary_double(MB_YES, bluefin->environmental[i].ctd_time, &buffer[index]); index += 8;
				mb_put_binary_double(MB_YES, bluefin->environmental[i].temperature_time, &buffer[index]); index += 8;
				for (j=0;j<56;j++)
					{
					buffer[index] = bluefin->environmental[i].reserved2[j]; index++;
					}
				}
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_volatilesonarsettings(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_volatilesonarsettings";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_volatilesettings *volatilesettings;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	volatilesettings = &(store->volatilesettings);
	header = &(volatilesettings->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_volatilesettings(verbose, volatilesettings, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kVolatileSonarSettings;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_long(MB_YES, volatilesettings->serial_number, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, volatilesettings->ping_number, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, volatilesettings->multi_ping, &buffer[index]); index += 2;
		mb_put_binary_float(MB_YES, volatilesettings->frequency, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->sample_rate, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->receiver_bandwidth, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->pulse_width, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, volatilesettings->pulse_type, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, volatilesettings->pulse_envelope, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->pulse_envelope_par, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, volatilesettings->pulse_reserved, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->max_ping_rate, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->ping_period, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->range_selection, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->power_selection, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->gain_selection, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, volatilesettings->control_flags, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, volatilesettings->projector_magic_no, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->steering_vertical, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->steering_horizontal, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->beamwidth_vertical, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->beamwidth_horizontal, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->focal_point, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, volatilesettings->projector_weighting, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->projector_weighting_par, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, volatilesettings->transmit_flags, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, volatilesettings->hydrophone_magic_no, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, volatilesettings->receive_weighting, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->receive_weighting_par, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, volatilesettings->receive_flags, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->receive_width, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->range_minimum, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->range_maximum, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->depth_minimum, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->depth_maximum, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->absorption, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->sound_velocity, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, volatilesettings->spreading, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, volatilesettings->reserved, &buffer[index]); index += 2;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_configuration(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_configuration";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_configuration *configuration;
	s7k_device *device;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	configuration = &(store->configuration);
	header = &(configuration->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_configuration(verbose, configuration, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kConfiguration;
	for (i=0;i<configuration->number_devices;i++)
		{
		*size += 80;
		device = &(configuration->device[i]);
		*size += device->info_length;
		}
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_long(MB_YES, configuration->serial_number, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, configuration->number_devices, &buffer[index]); index += 4;

		/* extract the data for each device */
		for (i=0;i<configuration->number_devices;i++)
			{
			device = &(configuration->device[i]);
			mb_put_binary_int(MB_YES, device->magic_number, &buffer[index]); index += 4;
			for (j=0;j<64;j++)
				{
				buffer[index] = device->description[j]; index++;
				}
			mb_put_binary_long(MB_YES, device->serial_number, &buffer[index]); index += 8;
			mb_put_binary_int(MB_YES, device->info_length, &buffer[index]); index += 4;

			for (j=0;j<device->info_length;j++)
				{
				buffer[index] = device->info[j]; index++;
				}
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_matchfilter(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_matchfilter";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_matchfilter *matchfilter;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	matchfilter = &(store->matchfilter);
	header = &(matchfilter->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_matchfilter(verbose, matchfilter, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kMatchFilter;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_long(MB_YES, matchfilter->serial_number, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, matchfilter->ping_number, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, matchfilter->operation, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, matchfilter->start_frequency, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, matchfilter->end_frequency, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_beamgeometry(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_beamgeometry";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_beamgeometry *beamgeometry;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	beamgeometry = &(store->beamgeometry);
	header = &(beamgeometry->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_beamgeometry(verbose, beamgeometry, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kBeamGeometry;
	*size += beamgeometry->number_beams * 16;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_long(MB_YES, beamgeometry->serial_number, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, beamgeometry->number_beams, &buffer[index]); index += 4;

		/* insert the data */
		for (i=0;i<beamgeometry->number_beams;i++)
			{
			mb_put_binary_float(MB_YES, beamgeometry->angle_alongtrack[i], &buffer[index]); index += 4;
			}
		for (i=0;i<beamgeometry->number_beams;i++)
			{
			mb_put_binary_float(MB_YES, beamgeometry->angle_acrosstrack[i], &buffer[index]); index += 4;
			}
		for (i=0;i<beamgeometry->number_beams;i++)
			{
			mb_put_binary_float(MB_YES, beamgeometry->beamwidth_alongtrack[i], &buffer[index]); index += 4;
			}
		for (i=0;i<beamgeometry->number_beams;i++)
			{
			mb_put_binary_float(MB_YES, beamgeometry->beamwidth_acrosstrack[i], &buffer[index]); index += 4;
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_calibration(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_calibration";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_calibration *calibration;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	calibration = &(store->calibration);
	header = &(calibration->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_calibration(verbose, calibration, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kCalibrationData;
	*size += calibration->number_channels * 8;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_long(MB_YES, calibration->serial_number, &buffer[index]); index += 8;
		mb_put_binary_short(MB_YES, calibration->number_channels, &buffer[index]); index += 2;

		/* insert the data */
		for (i=0;i<calibration->number_channels;i++)
			{
			mb_put_binary_float(MB_YES, calibration->gain[i], &buffer[index]); index += 4;
			}
		for (i=0;i<calibration->number_channels;i++)
			{
			mb_put_binary_float(MB_YES, calibration->phase[i], &buffer[index]); index += 4;
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_bathymetry(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_bathymetry";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_bathymetry *bathymetry;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	bathymetry = &(store->bathymetry);
	header = &(bathymetry->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_bathymetry(verbose, bathymetry, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kBathymetricData;
	*size += bathymetry->number_beams * 9;
	if (bathymetry->optionaldata == MB_YES)
		{
		*size += 45 + bathymetry->number_beams * 20;
		}
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;
		
		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_long(MB_YES, bathymetry->serial_number, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, bathymetry->ping_number, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, bathymetry->multi_ping, &buffer[index]); index += 2;
		mb_put_binary_int(MB_YES, bathymetry->number_beams, &buffer[index]); index += 4;

		/* insert the data */
		for (i=0;i<bathymetry->number_beams;i++)
			{
			mb_put_binary_float(MB_YES, bathymetry->range[i], &buffer[index]); index += 4;
			}
		for (i=0;i<bathymetry->number_beams;i++)
			{
			buffer[index] = bathymetry->quality[i]; index++;
			}
		for (i=0;i<bathymetry->number_beams;i++)
			{
			mb_put_binary_float(MB_YES, bathymetry->intensity[i], &buffer[index]); index += 4;
			}

		/* insert the optional data */
		if (bathymetry->optionaldata == MB_YES)
			{
			mb_put_binary_float(MB_YES, bathymetry->frequency, &buffer[index]); index += 4;
			mb_put_binary_double(MB_YES, bathymetry->latitude, &buffer[index]); index += 8;
			mb_put_binary_double(MB_YES, bathymetry->longitude, &buffer[index]); index += 8;
			mb_put_binary_float(MB_YES, bathymetry->heading, &buffer[index]); index += 4;
			buffer[index] = bathymetry->height_source; index++;
			mb_put_binary_float(MB_YES, bathymetry->tide, &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, bathymetry->roll, &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, bathymetry->pitch, &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, bathymetry->heave, &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, bathymetry->vehicle_height, &buffer[index]); index += 4;
			for (i=0;i<bathymetry->number_beams;i++)
				{
				mb_put_binary_float(MB_YES, bathymetry->depth[i], &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bathymetry->acrosstrack[i], &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bathymetry->alongtrack[i], &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bathymetry->pointing_angle[i], &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, bathymetry->azimuth_angle[i], &buffer[index]); index += 4;
				}
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_backscatter(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_backscatter";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_backscatter *backscatter;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	short	*short_ptr;
	int	*int_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	backscatter = &(store->backscatter);
	header = &(backscatter->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_backscatter(verbose, backscatter, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kBackscatterImageData;
	*size += 2 * backscatter->number_samples * backscatter->sample_size;
	if (header->OffsetToOptionalData > 0)
		{
		*size += 28;
		}
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_long(MB_YES, backscatter->serial_number, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, backscatter->ping_number, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, backscatter->multi_ping, &buffer[index]); index += 2;
		mb_put_binary_float(MB_YES, backscatter->beam_position, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, backscatter->control_flags, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, backscatter->number_samples, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, backscatter->port_beamwidth_x, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, backscatter->port_beamwidth_y, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, backscatter->stbd_beamwidth_x, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, backscatter->stbd_beamwidth_y, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, backscatter->port_steering_x, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, backscatter->port_steering_y, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, backscatter->stbd_steering_x, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, backscatter->stbd_steering_y, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, backscatter->number_beams, &buffer[index]); index += 2;
		mb_put_binary_short(MB_YES, backscatter->current_beam, &buffer[index]); index += 2;
		buffer[index] = backscatter->sample_size; index++;
		buffer[index] = backscatter->data_type; index++;

		/* allocate memory if required */
		data_size = backscatter->number_samples * backscatter->sample_size;
		if (backscatter->nalloc < data_size)
			{
			status = mb_realloc(verbose, data_size, &(backscatter->port_data), error);
			if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(backscatter->stbd_data), error);
			if (status == MB_SUCCESS)
				{
				backscatter->nalloc = data_size;
				}
			else
				{
				backscatter->nalloc = 0;
				backscatter->number_samples = 0;
				}
			}

		/* extract backscatter data */
		if (backscatter->sample_size == 1)
			{
			for (i=0;i<backscatter->number_samples;i++)
				{
				buffer[index] = backscatter->port_data[i]; index++;
				}
			for (i=0;i<backscatter->number_samples;i++)
				{
				buffer[index] = backscatter->stbd_data[i]; index++;
				}
			}
		else if (backscatter->sample_size == 2)
			{
			short_ptr = (short *) backscatter->port_data;
			for (i=0;i<backscatter->number_samples;i++)
				{
				mb_put_binary_short(MB_YES, short_ptr[i], &buffer[index]); index += 2;
				}
			short_ptr = (short *) backscatter->stbd_data;
			for (i=0;i<backscatter->number_samples;i++)
				{
				mb_put_binary_short(MB_YES, short_ptr[i], &buffer[index]); index += 2;
				}
			}
		else if (backscatter->sample_size == 4)
			{
			int_ptr = (int *) backscatter->port_data;
			for (i=0;i<backscatter->number_samples;i++)
				{
				mb_put_binary_int(MB_YES, int_ptr[i], &buffer[index]); index += 4;
				}
			int_ptr = (int *) backscatter->stbd_data;
			for (i=0;i<backscatter->number_samples;i++)
				{
				mb_put_binary_int(MB_YES, int_ptr[i], &buffer[index]); index += 4;
				}
			}

		/* extract the optional data */
		if (header->OffsetToOptionalData > 0)
			{
			index = header->OffsetToOptionalData;
			backscatter->optionaldata = MB_YES;
			mb_put_binary_float(MB_YES, backscatter->frequency, &buffer[index]); index += 4;
			mb_put_binary_double(MB_YES, backscatter->latitude, &buffer[index]); index += 8;
			mb_put_binary_double(MB_YES, backscatter->longitude, &buffer[index]); index += 8;
			mb_put_binary_float(MB_YES, backscatter->heading, &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, backscatter->altitude, &buffer[index]); index += 4;
			}
		else
			{
			backscatter->optionaldata = MB_NO;
			backscatter->frequency = 0.0;
			backscatter->latitude = 0.0;
			backscatter->longitude = 0.0;
			backscatter->heading = 0.0;
			backscatter->altitude = 0.0;
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_beam(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_beam";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_beam *beam;
	s7kr_snippet *snippet;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	short	*short_ptr;
	int	*int_ptr;
	int	nalloc;
	int	nsamples;
	int	sample_type_amp;
	int	sample_type_phase;
	int	sample_type_iandq;
	int	sample_size;
	char	*charptr;
	unsigned short 	*ushortptr;
	unsigned int	*uintptr;
	short	*shortptramp;
	short	*shortptrphase;
	int	*intptramp;
	int	*intptrphase;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	beam = &(store->beam);
	header = &(beam->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_beam(verbose, beam, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kBeamData;
	sample_type_amp = beam->sample_type & 15;
	sample_type_phase = (beam->sample_type << 4) & 15;
	sample_type_iandq = (beam->sample_type << 8) & 15;
	sample_size = 0;
	if (sample_type_amp == 1)
		sample_size += 1;
	else if (sample_type_amp == 2)
		sample_size += 2;
	else if (sample_type_amp == 3)
		sample_size += 4;
	if (sample_type_phase == 1)
		sample_size += 1;
	else if (sample_type_phase == 2)
		sample_size += 2;
	else if (sample_type_phase == 3)
		sample_size += 4;
	if (sample_type_iandq == 1)
		sample_size += 4;
	else if (sample_type_iandq == 2)
		sample_size += 8;
	for (i=0;i<beam->number_beams;i++)
		{
		snippet = &beam->snippets[i];
		*size += 10 + sample_size * (snippet->end_sample - snippet->begin_sample + 1);
		}
	if (header->OffsetToOptionalData > 0)
		{
		*size += 24 + beam->number_beams * 12;
		}
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_long(MB_YES, beam->serial_number, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, beam->ping_number, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, beam->multi_ping, &buffer[index]); index += 2;
		mb_put_binary_short(MB_YES, beam->number_beams, &buffer[index]); index += 2;
		mb_put_binary_short(MB_YES, beam->reserved, &buffer[index]); index += 2;
		mb_put_binary_int(MB_YES, beam->number_samples, &buffer[index]); index += 4;
		buffer[index] = beam->record_subset_flag; index++;
		buffer[index] = beam->row_column_flag; index++;
		mb_put_binary_short(MB_YES, beam->sample_header_id, &buffer[index]); index += 2;
		mb_put_binary_int(MB_YES, beam->sample_type, &buffer[index]); index += 4;
		for (i=0;i<beam->number_beams;i++)
			{
			snippet = &beam->snippets[i];
			mb_put_binary_short(MB_YES, snippet->beam_number, &buffer[index]); index += 2;
			mb_put_binary_int(MB_YES, snippet->begin_sample, &buffer[index]); index += 4;
			mb_put_binary_int(MB_YES, snippet->end_sample, &buffer[index]); index += 4;
			}

		for (i=0;i<beam->number_beams;i++)
			{
			/* allocate memory for snippet if needed */
			snippet = &beam->snippets[i];
			nalloc = sample_size * (snippet->end_sample - snippet->begin_sample + 1);
			if (status == MB_SUCCESS
				&& snippet->nalloc < nalloc)
				{
				snippet->nalloc = nalloc;
				if (status == MB_SUCCESS)
				status = mb_realloc(verbose, snippet->nalloc,
							(char **) &(snippet->amplitude), error);
				if (status == MB_SUCCESS)
				status = mb_realloc(verbose, snippet->nalloc,
							(char **) &(snippet->phase), error);
				if (status != MB_SUCCESS)
					{
					snippet->nalloc = 0;
					}
				}

			/* extract snippet data */
			if (status == MB_SUCCESS)
				{
				nsamples = snippet->end_sample - snippet->begin_sample + 1;
				if (sample_type_amp == 1)
					{
					charptr = (char *)snippet->amplitude;
					for (j=0;j<nsamples;j++)
						{
						buffer[index] = charptr[j]; index++;
						}
					}
				else if (sample_type_amp == 2)
					{
					ushortptr = (unsigned short *)snippet->amplitude;
					for (j=0;j<nsamples;j++)
						{
						mb_put_binary_short(MB_YES, ushortptr[j], &buffer[index]); index += 2;
						}
					}
				else if (sample_type_amp == 3)
					{
					uintptr = (unsigned int *)snippet->amplitude;
					for (j=0;j<nsamples;j++)
						{
						mb_put_binary_int(MB_YES, uintptr[j], &buffer[index]); index += 4;
						}
					}
				if (sample_type_phase == 1)
					{
					charptr = (char *)snippet->phase;
					for (j=0;j<nsamples;j++)
						{
						buffer[index] = charptr[j]; index++;
						}
					}
				else if (sample_type_phase == 2)
					{
					ushortptr = (unsigned short *)snippet->phase;
					for (j=0;j<nsamples;j++)
						{
						mb_put_binary_short(MB_YES, ushortptr[j], &buffer[index]); index += 2;
						}
					}
				else if (sample_type_phase == 3)
					{
					uintptr = (unsigned int *)snippet->phase;
					for (j=0;j<nsamples;j++)
						{
						mb_put_binary_int(MB_YES, uintptr[j], &buffer[index]); index += 4;
						}
					}
				else if (sample_type_iandq == 1)
					{
					shortptramp = (short *)snippet->amplitude;
					shortptrphase = (short *)snippet->phase;
					for (j=0;j<nsamples;j++)
						{
						mb_put_binary_short(MB_YES, shortptramp[j], &buffer[index]); index += 2;
						mb_put_binary_short(MB_YES, shortptrphase[j], &buffer[index]); index += 2;
						}
					}
				else if (sample_type_iandq == 2)
					{
					intptramp = (int *)snippet->amplitude;
					intptrphase = (int *)snippet->phase;
					for (j=0;j<nsamples;j++)
						{
						mb_put_binary_int(MB_YES, intptramp[j], &buffer[index]); index += 4;
						mb_put_binary_int(MB_YES, intptrphase[j], &buffer[index]); index += 4;
						}
					}
				}
			}

		/* extract the optional data */
		if (header->OffsetToOptionalData > 0)
			{
			index = header->OffsetToOptionalData;
			beam->optionaldata = MB_YES;
			mb_put_binary_float(MB_YES, beam->frequency, &buffer[index]); index += 4;
			mb_put_binary_double(MB_YES, beam->latitude, &buffer[index]); index += 8;
			mb_put_binary_double(MB_YES, beam->longitude, &buffer[index]); index += 8;
			mb_put_binary_float(MB_YES, beam->heading, &buffer[index]); index += 4;
			for (i=0;i<beam->number_beams;i++)
				{
				mb_put_binary_float(MB_YES, beam->alongtrack[i], &buffer[index]); index += 4;
				mb_put_binary_float(MB_YES, beam->acrosstrack[i], &buffer[index]); index += 4;
				mb_put_binary_int(MB_YES, beam->center_sample[i], &buffer[index]); index += 4;
				}
			}
		else
			{
			beam->optionaldata = MB_NO;
			beam->frequency = 0.0;
			beam->latitude = 0.0;
			beam->longitude = 0.0;
			beam->heading = 0.0;
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_verticaldepth(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_verticaldepth";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_verticaldepth *verticaldepth;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	verticaldepth = &(store->verticaldepth);
	header = &(verticaldepth->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_verticaldepth(verbose, verticaldepth, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kVerticalDepth;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_float(MB_YES, verticaldepth->frequency, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, verticaldepth->ping_number, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, verticaldepth->multi_ping, &buffer[index]); index += 2;
		mb_put_binary_double(MB_YES, verticaldepth->latitude, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, verticaldepth->longitude, &buffer[index]); index += 8;
		mb_put_binary_float(MB_YES, verticaldepth->heading, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, verticaldepth->alongtrack, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, verticaldepth->acrosstrack, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, verticaldepth->vertical_depth, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_image(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_image";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_image *image;
	s7kr_snippet *snippet;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	short	*short_ptr;
	int	*int_ptr;
	int	nalloc;
	int	nsamples;
	char	*charptr;
	unsigned short 	*ushortptr;
	unsigned int	*uintptr;
	short	*shortptramp;
	short	*shortptrphase;
	int	*intptramp;
	int	*intptrphase;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	image = &(store->image);
	header = &(image->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_image(verbose, image, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kImageData;
	*size += image->width * image->height * image->color_depth;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_int(MB_YES, image->ping_number, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, image->multi_ping, &buffer[index]); index += 2;
		mb_put_binary_int(MB_YES, image->width, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, image->height, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, image->color_depth, &buffer[index]); index += 2;
		mb_put_binary_short(MB_YES, image->width_height_flag, &buffer[index]); index += 2;
		mb_put_binary_short(MB_YES, image->compression, &buffer[index]); index += 2;

		/* allocate memory for image if needed */
		nalloc = image->width * image->height * image->color_depth;
		if (status == MB_SUCCESS
			&& image->nalloc < nalloc)
			{
			image->nalloc = nalloc;
			if (status == MB_SUCCESS)
			status = mb_realloc(verbose, image->nalloc,
						(char **) &(image->image), error);
			if (status != MB_SUCCESS)
				{
				image->nalloc = 0;
				image->width = 0;
				image->height = 0;
				}
			}

		/* extract image data */
		if (image->color_depth == 1)
			{
			charptr = (char *)image->image;
			for (i=0;i<image->width * image->height;i++)
				{
				buffer[index] = charptr[i]; index++;
				}
			}
		else if (image->color_depth == 2)
			{
			ushortptr = (unsigned short *)image->image;
			for (i=0;i<image->width * image->height;i++)
				{
				mb_put_binary_short(MB_YES, ushortptr[i], &buffer[index]); index += 2;
				}
			}
		else if (image->color_depth == 4)
			{
			uintptr = (unsigned int *)image->image;
			for (i=0;i<image->width * image->height;i++)
				{
				mb_put_binary_int(MB_YES, uintptr[i], &buffer[index]); index += 4;
				}
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_installation(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_installation";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_installation *installation;
	s7kr_snippet *snippet;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	installation = &(store->installation);
	header = &(installation->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_installation(verbose, installation, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kInstallationParameters;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_float(MB_YES, installation->frequency, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, installation->firmware_version_len, &buffer[index]); index += 2;
		for (i=0;i<128;i++)
			{
			buffer[index] = installation->firmware_version[i]; index++;
			}
		mb_put_binary_short(MB_YES, installation->software_version_len, &buffer[index]); index += 2;
		for (i=0;i<128;i++)
			{
			buffer[index] = installation->software_version[i]; index++;
			}
		mb_put_binary_short(MB_YES, installation->s7k_version_len, &buffer[index]); index += 2;
		for (i=0;i<128;i++)
			{
			buffer[index] = installation->s7k_version[i]; index++;
			}
		mb_put_binary_short(MB_YES, installation->protocal_version_len, &buffer[index]); index += 2;
		for (i=0;i<128;i++)
			{
			buffer[index] = installation->protocal_version[i]; index++;
			}
		mb_put_binary_float(MB_YES, installation->transmit_x, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->transmit_y, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->transmit_z, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->transmit_roll, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->transmit_pitch, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->transmit_heading, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->receive_x, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->receive_y, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->receive_z, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->receive_roll, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->receive_pitch, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->receive_heading, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->motion_x, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->motion_y, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->motion_z, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->motion_roll, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->motion_pitch, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->motion_heading, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, installation->motion_time_delay, &buffer[index]); index += 2;
		mb_put_binary_float(MB_YES, installation->position_x, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->position_y, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, installation->position_z, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, installation->position_time_delay, &buffer[index]); index += 2;
		mb_put_binary_float(MB_YES, installation->waterline_z, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fileheader(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_fileheader";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_fileheader *fileheader;
	s7kr_subsystem *subsystem;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	fileheader = &(store->fileheader);
	header = &(fileheader->header);
		
	/* make sure data are defined properly */
	if (status == MB_SUCCESS && header->RecordType != R7KRECID_7kFileHeader)
		{

		/* Reson 7k data record header information */
		header->Version = 4;
		header->Offset = 60;
		header->SyncPattern = 0x0000ffff;
		header->OffsetToOptionalData = 0;
		header->OptionalDataIdentifier = 0;
		header->s7kTime.Year = 0;
		header->s7kTime.Day = 0;
		header->s7kTime.Seconds = 0.0;
		header->s7kTime.Hours = 0;
		header->s7kTime.Minutes = 0;
		header->Reserved = 0;
		header->RecordType = R7KRECID_7kFileHeader;
		header->DeviceId = 0;
		header->Reserved2 = 0;
		header->SystemEnumerator = 0;
		header->DataSetNumber = 0;
		header->RecordNumber = 0;
		for (i=0;i<8;i++)
			{
			header->PreviousRecord[i] = -1;
			header->NextRecord[i] = -1;
			}
		header->Flags = 0;
		header->Reserved3 = 0;
		header->Reserved4 = 0;
		header->FragmentedTotal = 0;
		header->FragmentNumber = 0;
		}

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_fileheader(verbose, fileheader, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kFileHeader + R7KRDTSIZE_7kFileHeader;
	for (i=0;i<fileheader->number_subsystems;i++)
		*size += 6;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		for (i=0;i<16;i++)
			{
			buffer[index] = fileheader->file_identifier[i]; index++;
			}
		mb_put_binary_short(MB_YES, fileheader->version, &buffer[index]); index += 2;
		mb_put_binary_short(MB_YES, fileheader->reserved, &buffer[index]); index += 2;
		for (i=0;i<16;i++)
			{
			buffer[index] = fileheader->session_identifier[i]; index++;
			}
		mb_put_binary_int(MB_YES, fileheader->record_data_size, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, fileheader->number_subsystems, &buffer[index]); index += 4;
		for (i=0;i<64;i++)
			{
			buffer[index] = fileheader->recording_name[i]; index++;
			}
		for (i=0;i<16;i++)
			{
			buffer[index] = fileheader->recording_version[i]; index++;
			}
		for (i=0;i<64;i++)
			{
			buffer[index] = fileheader->user_defined_name[i]; index++;
			}
		for (i=0;i<128;i++)
			{
			buffer[index] = fileheader->notes[i]; index++;
			}
		for (i=0;i<fileheader->number_subsystems;i++)
			{
			subsystem = &(fileheader->subsystem[i]);
			mb_put_binary_int(MB_YES, subsystem->device_identifier, &buffer[index]); index += 4;
			mb_put_binary_short(MB_YES, subsystem->system_enumerator, &buffer[index]); index += 2;
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_systemeventmessage(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_systemeventmessage";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_systemeventmessage *systemeventmessage;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	systemeventmessage = &(store->systemeventmessage);
	header = &(systemeventmessage->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_systemeventmessage(verbose, systemeventmessage, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kSystemEventMessage;
	*size += systemeventmessage->message_length;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_long(MB_YES, systemeventmessage->serial_number, &buffer[index]); index += 8;
		mb_put_binary_short(MB_YES, systemeventmessage->event_id, &buffer[index]); index += 2;
		mb_put_binary_short(MB_YES, systemeventmessage->message_length, &buffer[index]); index += 2;
		mb_put_binary_short(MB_YES, systemeventmessage->event_identifier, &buffer[index]); index += 2;

		/* insert the data */
		for (i=0;i<systemeventmessage->message_length;i++)
			{
			buffer[index] = systemeventmessage->message[i]; index++;
			}

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_remotecontrolsettings(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_remotecontrolsettings";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_remotecontrolsettings *remotecontrolsettings;
	int	data_size;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	remotecontrolsettings = &(store->remotecontrolsettings);
	header = &(remotecontrolsettings->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_remotecontrolsettings(verbose, remotecontrolsettings, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kRemoteControlSonarSettings;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_long(MB_YES, remotecontrolsettings->serial_number, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, remotecontrolsettings->ping_number, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->frequency, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->sample_rate, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->receiver_bandwidth, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->pulse_width, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, remotecontrolsettings->pulse_type, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, remotecontrolsettings->pulse_envelope, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->pulse_envelope_par, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, remotecontrolsettings->pulse_reserved, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->max_ping_rate, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->ping_period, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->range_selection, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->power_selection, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->gain_selection, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, remotecontrolsettings->control_flags, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, remotecontrolsettings->projector_magic_no, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->steering_vertical, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->steering_horizontal, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->beamwidth_vertical, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->beamwidth_horizontal, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->focal_point, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, remotecontrolsettings->projector_weighting, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->projector_weighting_par, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, remotecontrolsettings->transmit_flags, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, remotecontrolsettings->hydrophone_magic_no, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, remotecontrolsettings->receive_weighting, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->receive_weighting_par, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, remotecontrolsettings->receive_flags, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->range_minimum, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->range_maximum, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->depth_minimum, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->depth_maximum, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->absorption, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->sound_velocity, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, remotecontrolsettings->spreading, &buffer[index]); index += 4;
		mb_put_binary_short(MB_YES, remotecontrolsettings->reserved, &buffer[index]); index += 2;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_roll(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_roll";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_roll *roll;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	roll = &(store->roll);
	header = &(roll->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_roll(verbose, roll, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kRoll;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_float(MB_YES, roll->roll, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_pitch(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_pitch";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_pitch *pitch;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	pitch = &(store->pitch);
	header = &(pitch->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_pitch(verbose, pitch, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kPitch;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_float(MB_YES, pitch->pitch, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_soundvelocity(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_soundvelocity";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_soundvelocity *soundvelocity;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	soundvelocity = &(store->soundvelocity);
	header = &(soundvelocity->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_soundvelocity(verbose, soundvelocity, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kSoundVelocity;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_float(MB_YES, soundvelocity->soundvelocity, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_absorptionloss(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_absorptionloss";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_absorptionloss *absorptionloss;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	absorptionloss = &(store->absorptionloss);
	header = &(absorptionloss->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_absorptionloss(verbose, absorptionloss, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kAbsorptionLoss;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_float(MB_YES, absorptionloss->absorptionloss, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_spreadingloss(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_spreadingloss";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_spreadingloss *spreadingloss;
	unsigned int checksum;
	int	index;
	char	*buffer;
	int	time_j[5];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       res_id:     %s\n",res_id);
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %d\n",bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_reson7k_struct *) store_ptr;
	spreadingloss = &(store->spreadingloss);
	header = &(spreadingloss->header);

	/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG
	if (verbose > 0)
#else
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_spreadingloss(verbose, spreadingloss, error);
	
	/* figure out size of output record */
	*size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	*size += R7KHDRSIZE_7kSpreadingLoss;
	
	/* allocate memory to read rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_realloc(verbose, *size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = *size;
			}
		}
		
	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the header */
		index = 0;
		status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

		/* insert the data */
		index = header->Offset + 4;
		mb_put_binary_float(MB_YES, spreadingloss->spreadingloss, &buffer[index]); index += 4;

		/* reset the header size value */
		mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);

		/* now add the checksum */
		checksum = 0;
		for (i=0;i<index;i++)
			checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;

		/* check size */
		if (*size != index)
			{
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
fprintf(stderr,"Bad size comparison: size:%d index:%d\n",*size,index);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
			*size = 0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
