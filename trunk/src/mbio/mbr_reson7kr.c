/*--------------------------------------------------------------------
 *    The MB-system:	mbr_reson7kr.c	4/4/2004
 *	$Id: mbr_reson7kr.c,v 5.3 2004-07-15 19:25:05 caress Exp $
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

int mbr_reson7kr_rd_header(int verbose, char *buffer, int *index, 
		s7k_header *header, int *error);

int mbr_reson7kr_rd_reference(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_sensoruncal(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_sensorcal(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_position(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_attitude(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_tide(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_altitude(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_motion(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_depth(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_svp(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_ctd(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_geodesy(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_survey(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_fsdwchannel(int verbose, int data_format, char *buffer, int *index, s7k_fsdwchannel *fsdwchannel, int *error);
int mbr_reson7kr_rd_fsdwssheader(int verbose, char *buffer, int *index, s7k_fsdwssheader *fsdwssheader, int *error);
int mbr_reson7kr_rd_fsdwsegyheader(int verbose, char *buffer, int *index, s7k_fsdwsegyheader *fsdwsegyheader, int *error);
int mbr_reson7kr_rd_fsdwsslo(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_fsdwsshi(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_fsdwsb(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_volatilesettings(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_configuration(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_beamgeometry(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_calibration(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_bathymetry(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_backscatter(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_systemevent(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_reson7kr_rd_fileheader(int verbose, char *buffer, void *store_ptr, int *error);

int mbr_reson7kr_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_reson7kr_wr_reference(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_sensoruncal(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_sensorcal(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_position(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_attitude(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_tide(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_altitude(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_motion(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_depth(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_svp(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_ctd(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_geodesy(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_survey(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_fsdwsslo(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_fsdwsshi(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_fsdwsb(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_bluefin(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_volatilesettings(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_configuration(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_beamgeometry(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_calibration(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_bathymetry(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_backscatter(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_systemevent(int verbose, char *buffer, void *store_ptr, int *size, int *error);
int mbr_reson7kr_wr_fileheader(int verbose, char *buffer, void *store_ptr, int *size, int *error);

static char res_id[]="$Id: mbr_reson7kr.c,v 5.3 2004-07-15 19:25:05 caress Exp $";

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
	*pixels_ss_max = 2048;
	strncpy(format_name, "RESON7KR", MB_NAME_LENGTH);
	strncpy(system_name, "RESON7K", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_RESON7KR\nInformal Description: Reson 7K multibeam vendor format\nAttributes:           Reson 7K series multibeam sonars, \n                      bathymetry, amplitude, three channels sidescan, and subbottom\n                      up to 254 beams, variable pixels, binary, Reson.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_NO;
	*nav_source = MB_DATA_NAV;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_ATTITUDE;
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
	int	*record_save_flag;
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
	int	*subsystemid;
	int	*fileheaders;

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
	current_ping = (int *) &mb_io_ptr->save1;
	record_save_flag = (int *) &mb_io_ptr->save2;
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
	subsystemid = (int *) &mb_io_ptr->save11;
	fileheaders = (int *) &mb_io_ptr->save12;
	*current_ping = -1;
	*record_save_flag = MB_NO;
	*recordid = R7KRECID_None;
	*recordidlast = R7KRECID_None;
	*bufferptr = NULL;
	*bufferalloc = 0;
	*size = 0;
	*nbadrec = 0;
	*deviceid = 0;
	*subsystemid = 0;
	*fileheaders = 0;
	
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
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson7k_struct *store;
	s7kr_position 	*position;
	s7kr_attitude 	*attitude;
	s7kr_survey	*survey;
	s7kr_bluefin	*bluefin;
	double		speed, heading;
	double		sonar_depth, sonar_altitude;
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
	survey = &store->survey;
	bluefin = &store->bluefin;

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
					(double)(bluefin->nav[i].position_time - 32.0), 
					(double)(RTD * bluefin->nav[i].longitude), 
					(double)(RTD * bluefin->nav[i].latitude), 
					error);
			mb_attint_add(verbose, mbio_ptr,
					(double)(bluefin->nav[i].position_time - 32.0), 
					(double)(0.0), 
					(double)(RTD * bluefin->nav[i].roll), 
					(double)(RTD * bluefin->nav[i].pitch), 
					error);
			mb_hedint_add(verbose, mbio_ptr,
					(double)(bluefin->nav[i].position_time - 32.0), 
					(double)(RTD * bluefin->nav[i].yaw), 
					error);
			mb_depint_add(verbose, mbio_ptr,
					(double)(bluefin->nav[i].position_time - 32.0), 
					(double)(bluefin->nav[i].depth), 
					error);
			mb_altint_add(verbose, mbio_ptr,
					(double)(bluefin->nav[i].altitude_time - 32.0), 
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
				(double)(store->time_d + ((double)i) / attitude->frequency - 32.0),
				(double)(attitude->heave[i]),
				(double)(attitude->roll[i]),
				(double)(attitude->pitch[i]),
				error);
			mb_hedint_add(verbose, mbio_ptr,
				(double)(store->time_d + ((double)i) / attitude->frequency - 32.0),
				(double)(attitude->heading[i]),
				error);
			}
		}
#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"Record returned: type:%d status:%d error:%d\n\n",store->kind, status, *error);
#endif

	if (status == MB_SUCCESS
		&& store->kind == MB_DATA_DATA)
		{
		speed = survey->speed;
		mb_navint_interp(verbose, mbio_ptr, store->time_d, survey->heading, speed, 
				    &survey->longitude, &survey->latitude, &speed, error);
		survey->speed = speed;
		mb_hedint_interp(verbose, mbio_ptr, store->time_d,  
				    &heading, error);
		survey->heading = heading;
		mb_depint_interp(verbose, mbio_ptr, store->time_d,  
				    &sonar_depth, error);
		survey->sonar_depth = sonar_depth;
		mb_altint_interp(verbose, mbio_ptr, store->time_d,  
				    &sonar_altitude, error);
		survey->sonar_altitude = sonar_altitude;
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
	FILE	*mbfp;
	int	done;
	int	*current_ping;
	int	*record_save_flag;
	int	*recordid;
	int	*recordidlast;
	int	*deviceid;
	int	*subsystemid;
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
double klugelon, klugelat;
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
	current_ping = (int *) &mb_io_ptr->save1;
	record_save_flag = (int *) &mb_io_ptr->save2;
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
	subsystemid = (int *) &mb_io_ptr->save11;
	fileheaders = (int *) &mb_io_ptr->save12;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* loop over reading data until a record is ready for return */
	done = MB_NO;
	*error = MB_ERROR_NO_ERROR;
	while (done == MB_NO)
		{
		/* if no header saved get next record label */
		if (*record_save_flag == MB_NO)
			{
			/* read next record header into buffer */
			if ((read_len = fread(buffer,
						1,MBSYS_RESON7K_RECORDHEADER_SIZE,
						mb_io_ptr->mbfp)) 
					!= MBSYS_RESON7K_RECORDHEADER_SIZE)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			
			/* check header - if not a good header read a byte 
				at a time until a good header is found */
			skip = 0;
			while (status == MB_SUCCESS
				&& mbr_reson7kr_chk_header(verbose, mbio_ptr, buffer, 
				recordid, deviceid, subsystemid, size) != MB_SUCCESS)
			    {
			    /* get next byte */
			    for (i=0;i<MBSYS_RESON7K_RECORDHEADER_SIZE-1;i++)
				buffer[i] = buffer[i+1];
			    if ((read_len = fread(&buffer[MBSYS_RESON7K_RECORDHEADER_SIZE-1],
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
					}
				}
			
			/* read the rest of the record */
			if (status == MB_SUCCESS
				&& (read_len = fread(&(buffer[MBSYS_RESON7K_RECORDHEADER_SIZE]),
						1, (*size - MBSYS_RESON7K_RECORDHEADER_SIZE),
						mb_io_ptr->mbfp)) 
					!= (*size - MBSYS_RESON7K_RECORDHEADER_SIZE))
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			
			}
		
		/* else use saved record */
		else
			{
			*record_save_flag = MB_NO;
			for (i=0;i<*size;i++)
				buffer[i] = buffersave[i];
			}

#ifdef MBR_RESON7KR_DEBUG
fprintf(stderr,"RESON7KR record:skip:%d recordid:%x %d deviceid:%x %d subsystemid:%x %d size:%d done:%d\n",
skip, *recordid, *recordid, 
*deviceid, *deviceid, 
*subsystemid, *subsystemid, 
*size, done);
#endif

		/* set done if read failure */
		if (status == MB_FAILURE)
			{
#ifdef MBR_RESON7KR_DEBUG
	fprintf(stderr,"call nothing, read failure\n");
#endif
			done = MB_YES;
			}

		/* parse the data record */
		else
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
			else if (*recordid == R7KRECID_Attitude)
				{
				status = mbr_reson7kr_rd_attitude(verbose, buffer, store_ptr, error);
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
			else if (*recordid == R7KRECID_FSDWsidescan
				&& *deviceid == R7KDEVID_EdgetechFSDW
				&& *subsystemid == 20)
				{
				status = mbr_reson7kr_rd_fsdwsslo(verbose, buffer, store_ptr, error);
				/*if (*current_ping >= 0
					&& store->fsdwsslo.ping_number == *current_ping)*/
				done = MB_YES;
				}
			else if (*recordid == R7KRECID_FSDWsidescan
				&& *deviceid == R7KDEVID_EdgetechFSDW
				&& *subsystemid == 21)
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
			else if (*recordid == R7KRECID_7kSystemEvent)
				{
				status = mbr_reson7kr_rd_systemevent(verbose, buffer, store_ptr, error);
				done = MB_YES;
				}
			else
				{
				
#ifdef MBR_RESON7KR_DEBUG
				fprintf(stderr,"would parse if the code existed\n");
#endif
				done = MB_NO;
				}
			}

		/* bail out if there is a parsing error */
		if (status == MB_FAILURE)
			done = MB_YES;
#ifdef MBR_RESON7KR_DEBUG
	fprintf(stderr,"done:%d recordid:%x size:%d status:%d error:%d\n", 
		done, *recordid, *size, status, *error);
	fprintf(stderr,"end of mbr_reson7kr_rd_data loop:\n\n");
#endif
		}
		
	/* get file position */
	if (*record_save_flag == MB_YES)
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
				int *recordid,  int *deviceid, int *subsystemid,int *size)
{
	char	*function_name = "mbr_reson7kr_chk_label";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	*recordidptr;
	int	*deviceidptr;
	int	*subsystemidptr;
	int	*sizeptr;
	int	sync;

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
	mb_get_binary_int(MB_YES, &buffer[4], &sync); 
	mb_get_binary_int(MB_YES, &buffer[8], size); 
	mb_get_binary_int(MB_YES, &buffer[32], recordid); 
	mb_get_binary_int(MB_YES, &buffer[36], deviceid); 
	mb_get_binary_int(MB_YES, &buffer[40], subsystemid); 
#ifdef MBR_RESON7KR_DEBUG
	fprintf(stderr, "Record id:    %4.4hX | %d\n", *recordid, *recordid);
	fprintf(stderr, "Device id:    %4.4hX | %d\n", *deviceid, *deviceid);
	fprintf(stderr, "Subsystem id: %4.4hX | %d\n", *subsystemid, *subsystemid);
	fprintf(stderr, "Size: %d\n", *size);
	fprintf(stderr, "Sync:  %4.4hX | %d\n", sync, sync);
#endif
	
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
			&& *recordid != R7KRECID_Attitude
			&& *recordid != R7KRECID_Tide
			&& *recordid != R7KRECID_Altitude
			&& *recordid != R7KRECID_MotionOverGround
			&& *recordid != R7KRECID_Depth
			&& *recordid != R7KRECID_SoundVelocityProfile
			&& *recordid != R7KRECID_CTD
			&& *recordid != R7KRECID_Geodesy
			&& *recordid != R7KRECID_Survey
			&& *recordid != R7KRECID_FSDWsidescan
			&& *recordid != R7KRECID_FSDWsubbottom
			&& *recordid != R7KRECID_Bluefin
			&& *recordid != R7KRECID_7kVolatileSonarSettings
			&& *recordid != R7KRECID_7kConfigurationSettings
			&& *recordid != R7KRECID_7kMatchFilter
			&& *recordid != R7KRECID_7kBeamGeometry
			&& *recordid != R7KRECID_7kCalibrationData
			&& *recordid != R7KRECID_7kBathymetricData
			&& *recordid != R7KRECID_7kBackscatterImageData
			&& *recordid != R7KRECID_7kBeamData
			&& *recordid != R7KRECID_7kSystemEvent
			&& *recordid != R7KRECID_7kDataStorageStatus
			&& *recordid != R7KRECID_7kFileHeader)
		{
		status = MB_SUCCESS;

#ifdef MBR_RESON7KR_DEBUG
		fprintf(stderr, "Good record id: %4.4hX | %d", *recordid, *recordid);
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
		if (*recordid == R7KRECID_Survey) fprintf(stderr," R7KRECID_Survey\n");
		if (*recordid == R7KRECID_FSDWsidescan) fprintf(stderr," R7KRECID_FSDWsidescan\n");
		if (*recordid == R7KRECID_FSDWsubbottom) fprintf(stderr," R7KRECID_FSDWsubbottom\n");
		if (*recordid == R7KRECID_Bluefin) fprintf(stderr," R7KRECID_Bluefin\n");
		if (*recordid == R7KRECID_7kVolatileSonarSettings) fprintf(stderr," R7KRECID_7kVolatileSonarSettings\n");
		if (*recordid == R7KRECID_7kConfigurationSettings) fprintf(stderr," R7KRECID_7kConfigurationSettings\n");
		if (*recordid == R7KRECID_7kMatchFilter) fprintf(stderr," R7KRECID_7kMatchFilter\n");
		if (*recordid == R7KRECID_7kBeamGeometry) fprintf(stderr," R7KRECID_7kBeamGeometry\n");
		if (*recordid == R7KRECID_7kCalibrationData) fprintf(stderr," R7KRECID_7kCalibrationData\n");
		if (*recordid == R7KRECID_7kBathymetricData) fprintf(stderr," R7KRECID_7kBathymetricData\n");
		if (*recordid == R7KRECID_7kBackscatterImageData) fprintf(stderr," R7KRECID_7kBackscatterImageData\n");
		if (*recordid == R7KRECID_7kBeamData) fprintf(stderr," R7KRECID_7kBeamData\n");
		if (*recordid == R7KRECID_7kSystemEvent) fprintf(stderr," R7KRECID_7kSystemEvent\n");
		if (*recordid == R7KRECID_7kDataStorageStatus) fprintf(stderr," R7KRECID_7kDataStorageStatus\n");
		if (*recordid == R7KRECID_7kFileHeader) fprintf(stderr," R7KRECID_7kFileHeader\n");
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
		fprintf(stderr,"dbg2       subsystemid:   %d\n",*subsystemid);
		fprintf(stderr,"dbg2       size:          %d\n",*size);
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
	mb_get_binary_int(MB_YES, &buffer[*index], &(header->SubsystemId)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(header->DataSetNumber)); *index += 4;
	mb_get_binary_int(MB_YES, &buffer[*index], &(header->RecordNumber)); *index += 4;
	for (i=0;i<8;i++)
		{
		header->PreviousRecord[i] = buffer[*index]; (*index)++;
		}
	for (i=0;i<8;i++)
		{
		header->NextRecord[i] = buffer[*index]; (*index)++;
		}
	mb_get_binary_short(MB_YES, &buffer[*index], &(header->Flags)); *index += 2;
	mb_get_binary_short(MB_YES, &buffer[*index], &(header->Reserved2)); *index += 2;
	
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
#ifndef MBR_RESON7KR_DEBUG
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
#ifndef MBR_RESON7KR_DEBUG
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
#ifndef MBR_RESON7KR_DEBUG
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
	mb_get_binary_int(MB_YES, &buffer[index], &(position->datum)); index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &(position->latitude)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(position->longitude)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(position->height)); index += 8;

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
#ifndef MBR_RESON7KR_DEBUG
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
	attitude->bitfield = (mb_u_char) buffer[index]; index++;
	attitude->reserved = (mb_u_char) buffer[index]; index++;
	mb_get_binary_short(MB_YES, &buffer[index], &(attitude->n)); index += 2;
	mb_get_binary_float(MB_YES, &buffer[index], &(attitude->frequency)); index += 4;
		
	/* make sure enough memory is allocated for channel data */
	if (attitude->nalloc < attitude->n)
		{
		data_size = attitude->n * sizeof(float);
		status = mb_realloc(verbose, data_size, &(attitude->pitch), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(attitude->roll), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(attitude->heading), error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose, data_size, &(attitude->heave), error);
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
		mb_get_binary_float(MB_YES, &buffer[index], &(attitude->pitch[i])); index += 4;
		}
	for (i=0;i<attitude->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(attitude->roll[i])); index += 4;
		}
	for (i=0;i<attitude->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(attitude->heading[i])); index += 4;
		}
	for (i=0;i<attitude->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(attitude->heave[i])); index += 4;
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
#ifndef MBR_RESON7KR_DEBUG
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
	mb_get_binary_float(MB_YES, &buffer[index], &(tide->tide)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(tide->source)); index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(tide->reserved)); index += 2;

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
#ifndef MBR_RESON7KR_DEBUG
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
#ifndef MBR_RESON7KR_DEBUG
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
#ifndef MBR_RESON7KR_DEBUG
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
#ifndef MBR_RESON7KR_DEBUG
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
		}
	for (i=0;i<svp->n;i++)
		{
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
#ifndef MBR_RESON7KR_DEBUG
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
	ctd->velocity_source_flag = (mb_u_char) buffer[index]; index++;
	ctd->velocity_algorithm = (mb_u_char) buffer[index]; index++;
	ctd->conductivity_flag = (mb_u_char) buffer[index]; index++;
	ctd->pressure_flag = (mb_u_char) buffer[index]; index++;
	ctd->position_flag = (mb_u_char) buffer[index]; index++;
	ctd->reserved1 = (mb_u_char) buffer[index]; index++;
	mb_get_binary_short(MB_YES, &buffer[index], &(ctd->reserved2)); index += 2;
	mb_get_binary_double(MB_YES, &buffer[index], &(ctd->latitude)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(ctd->longitude)); index += 8;
	mb_get_binary_float(MB_YES, &buffer[index], &(ctd->frequency)); index += 4;
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
		}
	for (i=0;i<ctd->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(ctd->temperature[i])); index += 4;
		}
	for (i=0;i<ctd->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(ctd->pressure_depth[i])); index += 4;
		}
	for (i=0;i<ctd->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(ctd->sound_velocity[i])); index += 4;
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
#ifndef MBR_RESON7KR_DEBUG
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
#ifndef MBR_RESON7KR_DEBUG
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
	s7kr_survey *survey;
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
	survey = &(store->survey);
	bluefin = &(store->bluefin);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
	
	/* extract the data */
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
		store->kind = MB_DATA_DATA;
		store->type = R7KRECID_FSDWsidescan;
		store->sstype = R7KRECID_FSDWsidescanLo;
		
		/* get the time from the 6046 datalogger header */
/*		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));*/
		
		/* get the time from the original Edgetech header */
		time_j[0] = fsdwssheader->year;
		time_j[1] = fsdwssheader->day;
		time_j[2] = 60 * fsdwssheader->hour + fsdwssheader->minute;
		time_j[3] = fsdwssheader->second;
		time_j[4] = 1000 * (fsdwssheader->millisecondsToday 
					- 1000 * ((int)(0.001 * fsdwssheader->millisecondsToday)));

		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifndef MBR_RESON7KR_DEBUG
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

	/* get sonar_depth and altitude if possible */
	if (bluefin->nav[0].depth > 0.0)
		survey->sonar_depth = bluefin->nav[0].depth;
	if (bluefin->nav[0].altitude > 0.0)
		survey->sonar_altitude = bluefin->nav[0].altitude;
	if (bluefin->environmental[0].sound_speed > 0.0)
		sound_speed = bluefin->environmental[0].sound_speed;
	else
		sound_speed = 1500.0;
	half_sound_speed = 0.5 * sound_speed;

	/* if necessary get first arrival and treat as altitude */
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

	/* insert sslo data into survey structure */
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
		
		/* get the time from the 6046 datalogger header */
/*		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));*/
		
		/* get the time from the original Edgetech header */
		time_j[0] = fsdwssheader->year;
		time_j[1] = fsdwssheader->day;
		time_j[2] = 60 * fsdwssheader->hour + fsdwssheader->minute;
		time_j[3] = fsdwssheader->second;
		time_j[4] = 1000 * (fsdwssheader->millisecondsToday 
					- 1000 * ((int)(0.001 * fsdwssheader->millisecondsToday)));

		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifndef MBR_RESON7KR_DEBUG
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
		
		/* get the time from the 6046 datalogger header */
/*		time_j[0] = header->s7kTime.Year;
		time_j[1] = header->s7kTime.Day;
		time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
		time_j[3] = (int) header->s7kTime.Seconds;
		time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));*/
		
		/* get the time from the original Edgetech header */
		time_j[0] = fsdwsegyheader->year;
		time_j[1] = fsdwsegyheader->day;
		time_j[2] = 60 * fsdwsegyheader->hour + fsdwsegyheader->minute;
		time_j[3] = fsdwsegyheader->second;
		time_j[4] = 1000 * (fsdwsegyheader->millisecondsToday 
					- 1000 * ((int)(0.001 * fsdwsegyheader->millisecondsToday)));

		mb_get_itime(verbose, time_j, store->time_i);
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print out the results */
#ifndef MBR_RESON7KR_DEBUG
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
#ifndef MBR_RESON7KR_DEBUG
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
		mb_get_binary_short(MB_YES, &buffer[index], &(subsystem->subsystem_identifier)); index += 2;
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
#ifndef MBR_RESON7KR_DEBUG
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
int mbr_reson7kr_rd_systemevent(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_reson7kr_rd_systemevent";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_systemevent *systemevent;
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
	systemevent = &(store->systemevent);
	header = &(systemevent->header);
	
	/* extract the header */
	index = 0;
	status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);
		
	/* extract the data */
	mb_get_binary_int(MB_YES, &buffer[index], &(systemevent->serial_number)); index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(systemevent->event_id)); index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(systemevent->message_length)); index += 2;
		
	/* make sure enough memory is allocated for channel data */
	if (systemevent->message_alloc < systemevent->message_length)
		{
		data_size = systemevent->message_length + 1;
		status = mb_realloc(verbose, data_size, &(systemevent->message), error);
		if (status == MB_SUCCESS)
			{
			systemevent->message_alloc = systemevent->message_length;
			}
		else
			{
			systemevent->message_alloc = 0;
			systemevent->message_length = 0;
			}
		}

	/* extract the data */
	for (i=0;i<systemevent->message_length;i++)
		{
		systemevent->message[i] = buffer[index]; index++;
		}

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_COMMENT;
		store->type = R7KRECID_7kSystemEvent;
		
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
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_systemevent(verbose, systemevent, error);

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

#ifdef MBR_RESON7KR_DEBUG
	fprintf(stderr,"\nstart of mbr_reson7kr_wr_data:\n");
	fprintf(stderr,"kind:%d %d type:%x ", store->kind, mb_io_ptr->new_kind, store->type);
#endif

	/* get size of output record */
	size = 0;
	size += MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
	if (store->type == R7KRECID_7kFileHeader || *fileheaders == 0)
		{
		size += R7KHDRSIZE_7kFileHeader + R7KRDTSIZE_7kFileHeader;
		}
	if (store->type == R7KRECID_ReferencePoint)
		{
		size += R7KHDRSIZE_ReferencePoint;
		}
	else if (store->type == R7KRECID_UncalibratedSensorOffset)
		{
		size += R7KHDRSIZE_UncalibratedSensorOffset;
		}
	else if (store->type == R7KRECID_CalibratedSensorOffset)
		{
		size += R7KHDRSIZE_CalibratedSensorOffset;
		}
	else if (store->type == R7KRECID_Position)
		{
		size += R7KHDRSIZE_Position;
		}
	else if (store->type == R7KRECID_Attitude)
		{
		size += R7KHDRSIZE_Attitude;
		nvalue = store->attitude.bitfield | 1
			 + store->attitude.bitfield | 2
			 + store->attitude.bitfield | 4
			 + store->attitude.bitfield | 8;
		size += nvalue * 4 + store->attitude.n;
		}
	else if (store->type == R7KRECID_Tide)
		{
		size += R7KHDRSIZE_Tide;
		}
	else if (store->type == R7KRECID_Altitude)
		{
		size += R7KHDRSIZE_Altitude;
		}
	else if (store->type == R7KRECID_MotionOverGround)
		{
		size += R7KHDRSIZE_MotionOverGround;
		nvalue = store->motion.bitfield | 1
			 + store->motion.bitfield | 2;
		size += nvalue * 12 + store->motion.n;
		}
	else if (store->type == R7KRECID_Depth)
		{
		size += R7KHDRSIZE_Depth;
		}
	else if (store->type == R7KRECID_SoundVelocityProfile)
		{
		size += R7KHDRSIZE_SoundVelocityProfile;
		size += 8 + store->svp.n;
		}
	else if (store->type == R7KRECID_CTD)
		{
		size += R7KHDRSIZE_CTD;
		size += 16 + store->ctd.n;
		}
	else if (store->type == R7KRECID_Geodesy)
		{
		size += R7KHDRSIZE_Geodesy;
		}
	else if (store->type == R7KRECID_Survey)
		{
		size += R7KHDRSIZE_Survey;
		}
	else if (store->type == R7KRECID_FSDWsidescan
		&& store->sstype == R7KRECID_FSDWsidescanLo)
		{
		size += R7KHDRSIZE_FSDWsidescan;
		size += store->fsdwsslo.number_channels * R7KHDRSIZE_FSDWchannelinfo
			 + store->fsdwsslo.number_channels * R7KHDRSIZE_FSDWssheader;
		for (i=0;i<store->fsdwsslo.number_channels;i++)
			{
			size += store->fsdwsslo.channel[i].number_samples * store->fsdwsslo.channel[i].bytespersample;
			}
		}
	else if (store->type == R7KRECID_FSDWsidescan
		&& store->sstype == R7KRECID_FSDWsidescanHi)
		{
		size += R7KHDRSIZE_FSDWsidescan;
		size += store->fsdwsshi.number_channels * R7KHDRSIZE_FSDWchannelinfo
			 + store->fsdwsshi.number_channels * R7KHDRSIZE_FSDWssheader;
		for (i=0;i<store->fsdwsshi.number_channels;i++)
			{
			size += store->fsdwsshi.channel[i].number_samples * store->fsdwsshi.channel[i].bytespersample;
			}
		}
	else if (store->type == R7KRECID_FSDWsubbottom)
		{
		size += R7KHDRSIZE_FSDWsubbottom;
		size += R7KHDRSIZE_FSDWchannelinfo * R7KHDRSIZE_FSDWsbheader;
		size += store->fsdwsb.channel.number_samples * store->fsdwsb.channel.bytespersample;
		}
	else
		{
		size += R7KHDRSIZE_None;
		}
			
	/* allocate memory to write record if necessary */
	if (*bufferalloc < size)
		{
		status = mb_realloc(verbose, size,
					bufferptr, error);
		if (status != MB_SUCCESS)
			{
			*bufferalloc = 0;
			}
		else
			{
			*bufferalloc = size;
			buffer = (char *) *bufferptr;
			}
		}
			
	/* write fileheader if needed */
	if (status == MB_SUCCESS 
		&& (store->type == R7KRECID_7kFileHeader || *fileheaders == 0))
		{
fprintf(stderr,"Writing FileHeader:  kind:%d %d type:%x %d size:%d\n", store->kind, mb_io_ptr->new_kind, store->type, store->type, size);
		status = mbr_reson7kr_wr_fileheader(verbose, buffer, store_ptr, &size, error);
fprintf(stderr,"Fileheader data inserted...\n");
		(*fileheaders)++;
			
		/* write the record to the output file */
		write_len = fwrite(buffer,1,size,mb_io_ptr->mbfp);
		if (write_len != size)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
fprintf(stderr,"Done writing FileHeader: %d %d status:%d\n", write_len, size, status);
		}
			
	/* call appropriate writing routine */
	if (status == MB_SUCCESS && store->type != R7KRECID_7kFileHeader)
		{
fprintf(stderr,"Writing Record:  kind:%d %d type:%x \n", store->kind, mb_io_ptr->new_kind, store->type);
		if (store->type == R7KRECID_ReferencePoint)
			{
			status = mbr_reson7kr_wr_reference(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_UncalibratedSensorOffset)
			{
			status = mbr_reson7kr_wr_sensoruncal(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_CalibratedSensorOffset)
			{
			status = mbr_reson7kr_wr_sensorcal(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Position)
			{
			status = mbr_reson7kr_wr_position(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Attitude)
			{
			status = mbr_reson7kr_wr_attitude(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Tide)
			{
			status = mbr_reson7kr_wr_tide(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Altitude)
			{
			status = mbr_reson7kr_wr_altitude(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_MotionOverGround)
			{
			status = mbr_reson7kr_wr_motion(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Depth)
			{
			status = mbr_reson7kr_wr_depth(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_SoundVelocityProfile)
			{
			status = mbr_reson7kr_wr_svp(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_CTD)
			{
			status = mbr_reson7kr_wr_ctd(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Geodesy)
			{
			status = mbr_reson7kr_wr_geodesy(verbose, buffer, store_ptr, &size, error);
			}
		/*else if (store->type == R7KRECID_Survey)
			{
			status = mbr_reson7kr_wr_survey(verbose, buffer, store_ptr, &size, error);
			}*/
		else if (store->type == R7KRECID_FSDWsidescan)
			{
			status = mbr_reson7kr_wr_fsdwsslo(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_FSDWsidescan)
			{
			status = mbr_reson7kr_wr_fsdwsshi(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_FSDWsubbottom)
			{
			status = mbr_reson7kr_wr_fsdwsb(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_Bluefin)
			{
			status = mbr_reson7kr_wr_bluefin(verbose, buffer, store_ptr, &size, error);
			}
		else if (store->type == R7KRECID_7kSystemEvent)
			{
			status = mbr_reson7kr_wr_systemevent(verbose, buffer, store_ptr, &size, error);
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
			write_len = fwrite(buffer,1,size,mb_io_ptr->mbfp);
			if (write_len != size)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
				}
			}
		}

#ifdef MBR_RESON7KR_DEBUG
	fprintf(stderr,"status:%d error:%d\n", status, *error);
	fprintf(stderr,"end of mbr_reson7kr_wr_data:\n");
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
	header->Version = 2;
	header->Offset = 68;
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
#ifndef MBR_RESON7KR_DEBUG
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
	mb_put_binary_int(MB_YES, header->SubsystemId, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, header->DataSetNumber, &buffer[*index]); *index += 4;
	mb_put_binary_int(MB_YES, header->RecordNumber, &buffer[*index]); *index += 4;
	for (i=0;i<8;i++)
		{
		buffer[*index] = header->PreviousRecord[i]; (*index)++;
		}
	for (i=0;i<8;i++)
		{
		buffer[*index] = header->NextRecord[i]; (*index)++;
		}
	mb_put_binary_short(MB_YES, header->Flags, &buffer[*index]); *index += 2;
	mb_put_binary_short(MB_YES, header->Reserved2, &buffer[*index]); *index += 2;

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
int mbr_reson7kr_wr_reference(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_reference";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_reference *reference;
	unsigned int checksum;
	int	index;
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_reference(verbose, reference, error);
	
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
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_sensoruncal(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_sensoruncal";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_sensoruncal *sensoruncal;
	unsigned int checksum;
	int	index;
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_sensoruncal(verbose, sensoruncal, error);
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
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
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_sensorcal(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_sensorcal";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_sensorcal *sensorcal;
	unsigned int checksum;
	int	index;
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_sensorcal(verbose, sensorcal, error);
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
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
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_position(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_position";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_position *position;
	unsigned int checksum;
	int	index;
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_position(verbose, position, error);
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
	mb_put_binary_int(MB_YES, position->datum, &buffer[index]); index += 4;
	mb_put_binary_double(MB_YES, position->latitude, &buffer[index]); index += 8;
	mb_put_binary_double(MB_YES, position->longitude, &buffer[index]); index += 8;
	mb_put_binary_double(MB_YES, position->height, &buffer[index]); index += 8;

	/* reset the header size value */
	mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);
		
	/* now add the checksum */
	checksum = 0;
	for (i=0;i<index;i++)
		checksum += (unsigned char) buffer[i];
	mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_attitude(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_attitude";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_attitude *attitude;
	int	data_size;
	unsigned int checksum;
	int	index;
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_attitude(verbose, attitude, error);
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
	buffer[index] = attitude->bitfield; index++;
	buffer[index] = attitude->reserved; index++;
	mb_put_binary_short(MB_YES, attitude->n, &buffer[index]); index += 2;
	mb_put_binary_float(MB_YES, attitude->frequency, &buffer[index]); index += 4;

	for (i=0;i<attitude->n;i++)
		{
		mb_put_binary_float(MB_YES, attitude->pitch[i], &buffer[index]); index += 4;
		}
	for (i=0;i<attitude->n;i++)
		{
		mb_put_binary_float(MB_YES, attitude->roll[i], &buffer[index]); index += 4;
		}
	for (i=0;i<attitude->n;i++)
		{
		mb_put_binary_float(MB_YES, attitude->heading[i], &buffer[index]); index += 4;
		}
	for (i=0;i<attitude->n;i++)
		{
		mb_put_binary_float(MB_YES, attitude->heave[i], &buffer[index]); index += 4;
		}

	/* reset the header size value */
	mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);
		
	/* now add the checksum */
	checksum = 0;
	for (i=0;i<index;i++)
		checksum += (unsigned char) buffer[i];
	mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_tide(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_tide";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_tide *tide;
	unsigned int checksum;
	int	index;
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_tide(verbose, tide, error);
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
	mb_put_binary_float(MB_YES, tide->tide, &buffer[index]); index += 4;
	mb_put_binary_short(MB_YES, tide->source, &buffer[index]); index += 2;
	mb_put_binary_short(MB_YES, tide->reserved, &buffer[index]); index += 2;

	/* reset the header size value */
	mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);
		
	/* now add the checksum */
	checksum = 0;
	for (i=0;i<index;i++)
		checksum += (unsigned char) buffer[i];
	mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_altitude(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_altitude";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_altitude *altitude;
	unsigned int checksum;
	int	index;
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_altitude(verbose, altitude, error);
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
	mb_put_binary_float(MB_YES, altitude->altitude, &buffer[index]); index += 4;

	/* reset the header size value */
	mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);
		
	/* now add the checksum */
	checksum = 0;
	for (i=0;i<index;i++)
		checksum += (unsigned char) buffer[i];
	mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_motion(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_motion";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_motion *motion;
	int	data_size;
	unsigned int checksum;
	int	index;
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_motion(verbose, motion, error);
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
	buffer[index] = motion->bitfield; index++;
	buffer[index] = motion->reserved; index++;
	mb_put_binary_short(MB_YES, motion->n, &buffer[index]); index += 2;
	mb_put_binary_float(MB_YES, motion->frequency, &buffer[index]); index += 4;

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

	/* reset the header size value */
	mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);
		
	/* now add the checksum */
	checksum = 0;
	for (i=0;i<index;i++)
		checksum += (unsigned char) buffer[i];
	mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_depth(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_depth";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_depth *depth;
	unsigned int checksum;
	int	index;
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_depth(verbose, depth, error);
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
	buffer[index] = depth->descriptor; index++;
	buffer[index] = depth->correction; index++;
	mb_put_binary_short(MB_YES, depth->reserved, &buffer[index]); index += 2;
	mb_put_binary_float(MB_YES, depth->depth, &buffer[index]); index += 4;

	/* reset the header size value */
	mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);
		
	/* now add the checksum */
	checksum = 0;
	for (i=0;i<index;i++)
		checksum += (unsigned char) buffer[i];
	mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_svp(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_svp";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_svp *svp;
	int	data_size;
	unsigned int checksum;
	int	index;
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_svp(verbose, svp, error);
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
	buffer[index] = svp->position_flag; index++;
	buffer[index] = svp->reserved1; index++;
	mb_put_binary_short(MB_YES, svp->reserved2, &buffer[index]); index += 2;
	mb_put_binary_double(MB_YES, svp->latitude, &buffer[index]); index += 8;
	mb_put_binary_double(MB_YES, svp->longitude, &buffer[index]); index += 8;
	mb_put_binary_int(MB_YES, svp->n, &buffer[index]); index += 4;

	for (i=0;i<svp->n;i++)
		{
		mb_put_binary_float(MB_YES, svp->depth[i], &buffer[index]); index += 4;
		}
	for (i=0;i<svp->n;i++)
		{
		mb_put_binary_float(MB_YES, svp->sound_velocity[i], &buffer[index]); index += 4;
		}

	/* reset the header size value */
	mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);
		
	/* now add the checksum */
	checksum = 0;
	for (i=0;i<index;i++)
		checksum += (unsigned char) buffer[i];
	mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_ctd(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_ctd";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_ctd *ctd;
	int	data_size;
	unsigned int checksum;
	int	index;
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_ctd(verbose, ctd, error);
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
	buffer[index] = ctd->velocity_source_flag; index++;
	buffer[index] = ctd->velocity_algorithm; index++;
	buffer[index] = ctd->conductivity_flag; index++;
	buffer[index] = ctd->pressure_flag; index++;
	buffer[index] = ctd->position_flag; index++;
	buffer[index] = ctd->reserved1; index++;
	mb_put_binary_short(MB_YES, ctd->reserved2, &buffer[index]); index += 2;
	mb_put_binary_double(MB_YES, ctd->latitude, &buffer[index]); index += 8;
	mb_put_binary_double(MB_YES, ctd->longitude, &buffer[index]); index += 8;
	mb_put_binary_float(MB_YES, ctd->frequency, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, ctd->n, &buffer[index]); index += 4;

	for (i=0;i<ctd->n;i++)
		{
		mb_put_binary_float(MB_YES, ctd->conductivity_salinity[i], &buffer[index]); index += 4;
		}
	for (i=0;i<ctd->n;i++)
		{
		mb_put_binary_float(MB_YES, ctd->temperature[i], &buffer[index]); index += 4;
		}
	for (i=0;i<ctd->n;i++)
		{
		mb_put_binary_float(MB_YES, ctd->pressure_depth[i], &buffer[index]); index += 4;
		}
	for (i=0;i<ctd->n;i++)
		{
		mb_put_binary_float(MB_YES, ctd->sound_velocity[i], &buffer[index]); index += 4;
		}

	/* reset the header size value */
	mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);
		
	/* now add the checksum */
	checksum = 0;
	for (i=0;i<index;i++)
		checksum += (unsigned char) buffer[i];
	mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_geodesy(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_geodesy";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_geodesy *geodesy;
	int	data_size;
	unsigned int checksum;
	int	index;
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_geodesy(verbose, geodesy, error);
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
	for (i=0;i<32;i++)
		{
		buffer[index] = geodesy->spheroid[i]; index++;
		}
	mb_put_binary_double(MB_YES, geodesy->semimajoraxis, &buffer[index]); index += 8;
	mb_put_binary_double(MB_YES, geodesy->flattening, &buffer[index]); index += 8;
	for (i=0;i<16;i++)
		{
		buffer[index] = geodesy->reserved1[i]; index++;
		}
	for (i=0;i<32;i++)
		{
		buffer[index] = geodesy->datum[i]; index++;
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
		buffer[index] = geodesy->reserved2[i]; index++;
		}
	for (i=0;i<32;i++)
		{
		buffer[index] = geodesy->grid_name[i]; index++;
		}
	buffer[index] = geodesy->distance_units; index++;
	buffer[index] = geodesy->angular_units; index++;
	mb_put_binary_double(MB_YES, geodesy->latitude_origin, &buffer[index]); index += 8;
	mb_put_binary_double(MB_YES, geodesy->central_meriidan, &buffer[index]); index += 8;
	mb_put_binary_double(MB_YES, geodesy->false_easting, &buffer[index]); index += 8;
	mb_put_binary_double(MB_YES, geodesy->false_northing, &buffer[index]); index += 8;
	mb_put_binary_double(MB_YES, geodesy->central_scale_factor, &buffer[index]); index += 8;
	mb_put_binary_int(MB_YES, geodesy->custum_identifier, &buffer[index]); index += 4;
	for (i=0;i<50;i++)
		{
		buffer[index] = geodesy->reserved3[i]; index++;
		}

	/* reset the header size value */
	mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);
		
	/* now add the checksum */
	checksum = 0;
	for (i=0;i<index;i++)
		checksum += (unsigned char) buffer[i];
	mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
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
		fprintf(stderr,"dbg2       buffer:     %d\n",buffer);
		fprintf(stderr,"dbg2       index:      %d\n",*index);
		fprintf(stderr,"dbg2       fsdwchannel:%d\n",fsdwchannel);
		}
	
	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_fsdwchannel(verbose, data_format, fsdwchannel, error);
	
	/* insert the channel header */
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
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_fsdwssheader(verbose, fsdwssheader, error);
	
	/* insert the Edgetech sidescan header */
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
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_fsdwsegyheader(verbose, fsdwsegyheader, error);
	
	/* insert the Edgetech segy header */
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
int mbr_reson7kr_wr_fsdwsslo(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_fsdwsslo";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_fsdwss *fsdwsslo;
	s7k_fsdwchannel *fsdwchannel;
	s7k_fsdwssheader *fsdwssheader;
	s7kr_survey *survey;
	double	bin[MBSYS_RESON7K_MAX_PIXELS];
	int	nbin[MBSYS_RESON7K_MAX_PIXELS];
	unsigned int checksum;
	int	index;
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
	survey = &(store->survey);

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
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
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
	
	/* insert the data */
	mb_put_binary_int(MB_YES, fsdwsslo->msec_timestamp, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, fsdwsslo->ping_number, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, fsdwsslo->number_channels, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, fsdwsslo->total_bytes, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, fsdwsslo->data_format, &buffer[index]); index += 4;
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
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fsdwsshi(int verbose, char *buffer, void *store_ptr, int *size, int *error)
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
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
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
	
	/* insert the data */
	mb_put_binary_int(MB_YES, fsdwsshi->msec_timestamp, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, fsdwsshi->ping_number, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, fsdwsshi->number_channels, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, fsdwsshi->total_bytes, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, fsdwsshi->data_format, &buffer[index]); index += 4;
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
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fsdwsb(int verbose, char *buffer, void *store_ptr, int *size, int *error)
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
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
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
	mb_put_binary_int(MB_YES, fsdwsb->msec_timestamp, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, fsdwsb->ping_number, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, fsdwsb->number_channels, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, fsdwsb->total_bytes, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, fsdwsb->data_format, &buffer[index]); index += 4;
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
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_bluefin(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_bluefin";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_bluefin *bluefin;
	unsigned int checksum;
	int	index;
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_bluefin(verbose, bluefin, error);
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
	mb_put_binary_int(MB_YES, bluefin->msec_timestamp, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, bluefin->number_frames, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, bluefin->frame_size, &buffer[index]); index += 4;
	mb_put_binary_int(MB_YES, bluefin->data_format, &buffer[index]); index += 4;
	for (i=0;i<16;i++)
		{
		buffer[index] = bluefin->reserved[i] = buffer[index]; index++;
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
			buffer[index] = bluefin->nav[i].s7kTime.Hours; (index)++;
			buffer[index] = bluefin->nav[i].s7kTime.Minutes; (index)++;
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
			buffer[index] = bluefin->environmental[i].s7kTime.Hours; (index)++;
			buffer[index] = bluefin->environmental[i].s7kTime.Minutes; (index)++;
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
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fileheader(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_fileheader";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_fileheader *fileheader;
	s7kr_subsystem *subsystem;
	unsigned int checksum;
	int	index;
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

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_fileheader(verbose, fileheader, error);
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
	for (i=0;i<16;i++)
		{
		fileheader->file_identifier[i]; index++;
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
		mb_put_binary_short(MB_YES, subsystem->subsystem_identifier, &buffer[index]); index += 2;
		mb_put_binary_short(MB_YES, subsystem->system_enumerator, &buffer[index]); index += 2;
		}
		
	/* reset the header size value */
	mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);
		
	/* now add the checksum */
	checksum = 0;
	for (i=0;i<index;i++)
		checksum += (unsigned char) buffer[i];
	mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_systemevent(int verbose, char *buffer, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_reson7kr_wr_systemevent";
	int	status = MB_SUCCESS;
	struct mbsys_reson7k_struct *store;
	s7k_header *header;
	s7kr_systemevent *systemevent;
	int	data_size;
	unsigned int checksum;
	int	index;
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
	systemevent = &(store->systemevent);
	header = &(systemevent->header);

	/* print out the data to be output */
#ifndef MBR_RESON7KR_DEBUG
	if (verbose >= 2)
#endif
	mbsys_reson7k_print_systemevent(verbose, systemevent, error);
	
	/* insert the header */
	index = 0;
	status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);
		
	/* insert the data */
	mb_put_binary_int(MB_YES, systemevent->serial_number, &buffer[index]); index += 4;
	mb_put_binary_short(MB_YES, systemevent->event_id, &buffer[index]); index += 2;
	mb_put_binary_short(MB_YES, systemevent->message_length, &buffer[index]); index += 2;
	for (i=0;i<systemevent->message_length;i++)
		{
		buffer[index] = systemevent->message[i]; index++;
		}

	/* reset the header size value */
	mb_put_binary_int(MB_YES, ((unsigned int)(index + 4)), &buffer[8]);
		
	/* now add the checksum */
	checksum = 0;
	for (i=0;i<index;i++)
		checksum += (unsigned char) buffer[i];
	mb_put_binary_int(MB_YES, checksum, &buffer[index]); index += 4;
	*size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %d\n",*size);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
