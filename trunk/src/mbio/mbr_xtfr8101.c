/*--------------------------------------------------------------------
 *    The MB-system:	mbr_xtfr8101.c	8/8/94
 *	$Id$
 *
 *    Copyright (c) 2001-2009 by
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
 * mbr_xtfr8101.c contains the functions for reading and writing
 * multibeam data in the XTFR8101 format.  
 * These functions include:
 *   mbr_alm_xtfr8101	- allocate read/write memory
 *   mbr_dem_xtfr8101	- deallocate read/write memory
 *   mbr_rt_xtfr8101	- read and translate data
 *   mbr_wt_xtfr8101	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	August 26, 2001
 *
 * $Log: mbr_xtfr8101.c,v $
 * Revision 5.11  2008/09/13 06:08:09  caress
 * Updates to apply suggested patches to segy handling. Also fixes to remove compiler warnings.
 *
 * Revision 5.10  2006/03/06 21:47:48  caress
 * Implemented changes suggested by Bob Courtney of the Geological Survey of Canada to support translating Reson data to GSF.
 *
 * Revision 5.9  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.8  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.7  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.6  2003/01/15 20:51:48  caress
 * Release 5.0.beta28
 *
 * Revision 5.5  2002/09/25 20:41:04  caress
 * Fixed some problems.
 *
 * Revision 5.4  2002/09/19 01:12:39  caress
 * Release 5.0.beta23
 *
 * Revision 5.3  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.2  2001/11/16 01:32:31  caress
 * Working on it...
 *
 * Revision 5.1  2001/10/12  21:08:37  caress
 * Added interpolation of attitude data.
 *
 * Revision 5.0  2001/09/17  23:24:10  caress
 * Added XTF format.
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
#include "../../include/mbsys_reson8k.h"
#include "../../include/mbf_xtfr8101.h"
	
/* turn on debug statements here */
/* #define MBR_XTFR8101_DEBUG 1 */

/* essential function prototypes */
int mbr_register_xtfr8101(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_xtfr8101(int verbose, 
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
int mbr_alm_xtfr8101(int verbose, void *mbio_ptr, int *error);
int mbr_dem_xtfr8101(int verbose, void *mbio_ptr, int *error);
int mbr_zero_xtfr8101(int verbose, char *data_ptr, int *error);
int mbr_rt_xtfr8101(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_xtfr8101(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_xtfr8101_rd_data(int verbose, void *mbio_ptr, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_xtfr8101(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_xtfr8101";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set format info parameters */
	status = mbr_info_xtfr8101(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_xtfr8101;
	mb_io_ptr->mb_io_format_free = &mbr_dem_xtfr8101; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_reson8k_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_reson8k_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_xtfr8101; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_xtfr8101; 
	mb_io_ptr->mb_io_dimensions = &mbsys_reson8k_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_reson8k_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_reson8k_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_reson8k_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_reson8k_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_reson8k_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = &mbsys_reson8k_extract_svp; 
	mb_io_ptr->mb_io_insert_svp = &mbsys_reson8k_insert_svp; 
	mb_io_ptr->mb_io_ttimes = &mbsys_reson8k_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_reson8k_copy; 
	mb_io_ptr->mb_io_extract_rawss = NULL; 
	mb_io_ptr->mb_io_insert_rawss = NULL; 

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
		fprintf(stderr,"dbg2       format_alloc:       %ld\n",(size_t)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %ld\n",(size_t)mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %ld\n",(size_t)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %ld\n",(size_t)mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %ld\n",(size_t)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %ld\n",(size_t)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %ld\n",(size_t)mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %ld\n",(size_t)mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %ld\n",(size_t)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %ld\n",(size_t)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %ld\n",(size_t)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %ld\n",(size_t)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %ld\n",(size_t)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %ld\n",(size_t)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %ld\n",(size_t)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       extract_rawss:      %ld\n",(size_t)mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr,"dbg2       insert_rawss:       %ld\n",(size_t)mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr,"dbg2       copyrecord:         %ld\n",(size_t)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_xtfr8101(int verbose, 
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
	char	*function_name = "mbr_info_xtfr8101";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_RESON8K;
	*beams_bath_max = MBSYS_RESON8K_MAXBEAMS;
	*beams_amp_max = MBSYS_RESON8K_MAXBEAMS;
	*pixels_ss_max = MBSYS_RESON8K_MAXPIXELS;
	strncpy(format_name, "XTFR8101", MB_NAME_LENGTH);
	strncpy(system_name, "RESON8K", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_XTFR8101\nInformal Description: XTF format Reson SeaBat 81XX\nAttributes:           240 beam bathymetry and amplitude,\n		      1024 pixel sidescan\n                      binary, read-only,\n                      Triton-Elics.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
	*beamwidth_xtrack = 1.5;
	*beamwidth_ltrack = 1.5;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbr_alm_xtfr8101(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_xtfr8101";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	*fileheaderread;
	double	*pixel_size;
	double	*swath_width;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_xtfr8101_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_reson8k_struct),
				&mb_io_ptr->store_data,error);
				
	/* set saved flags */
	fileheaderread = (int *) &(mb_io_ptr->save1);
	pixel_size = &mb_io_ptr->saved1;
	swath_width = &mb_io_ptr->saved2;
	*fileheaderread = MB_NO;
	*pixel_size = 0.0;
	*swath_width = 0.0;

	/* initialize everything to zeros */
	mbr_zero_xtfr8101(verbose,mb_io_ptr->raw_data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_dem_xtfr8101(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_xtfr8101";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		}

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mb_free(verbose,&mb_io_ptr->store_data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_zero_xtfr8101(int verbose, char *data_ptr, int *error)
{
	char	*function_name = "mbr_zero_xtfr8101";
	int	status = MB_SUCCESS;
	struct mbf_xtfr8101_struct *data;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       data_ptr:   %ld\n",(size_t)data_ptr);
		}

	/* get pointer to data descriptor */
	data = (struct mbf_xtfr8101_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		data->kind = MB_DATA_NONE;
		data->sonar = MBSYS_RESON8K_UNKNOWN;
		}

	/* assume success */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_xtfr8101(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_xtfr8101";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_xtfr8101_struct *data;
	struct mbsys_reson8k_struct *store;
	int	nchan;
	int	time_i[7];
	double	time_d, ntime_d, dtime, timetag;
	double	ttscale, angscale;
	int	icenter, quality;
	int	intensity_max;
	double	angle, theta, phi;
	double	rr, xx, zz;
	double	*pixel_size, *swath_width;
	double	lever_x, lever_y, lever_z;
	int	badtime;
	double	gain_correction;
	double	lon, lat;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %ld\n",(size_t)store_ptr);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_xtfr8101_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_reson8k_struct *) store_ptr;
	pixel_size = (double *) &mb_io_ptr->saved1;
	swath_width = (double *) &mb_io_ptr->saved2;

	/* read next data from file */
	status = mbr_xtfr8101_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* handle navigation fix delay */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA)
		{
		/* get ping time */
		time_i[0] = data->bathheader.Year;
		time_i[1] = data->bathheader.Month;
		time_i[2] = data->bathheader.Day;
		time_i[3] = data->bathheader.Hour;
		time_i[4] = data->bathheader.Minute;
		time_i[5] = data->bathheader.Second;
		time_i[6] = 10000 * data->bathheader.HSeconds;
		mb_get_time(verbose, time_i, &time_d);

		/* do check on time here - we sometimes get a bad fix */
		badtime = MB_NO;
		if (time_i[0] < 1970 && time_i[0] > 2100 ) badtime = MB_YES;
		if (time_i[1] < 0 && time_i[1] > 12) badtime = MB_YES;
		if (time_i[2] < 0 && time_i[2] > 31 ) badtime = MB_YES;
		if (badtime == MB_YES) 
			{
			if (verbose > 0)
				fprintf(stderr," Bad time from XTF in bathy header\n");
			data->kind = MB_DATA_NONE;
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		
		/* get nav time */
		dtime = 3600.0 * (data->bathheader.FixTimeHour - data->bathheader.Hour)
		    + 60.0 * (data->bathheader.FixTimeMinute - data->bathheader.Minute)
		    + data->bathheader.FixTimeSecond - data->bathheader.Second
		    - 0.01 * data->bathheader.HSeconds;
		if (data->bathheader.FixTimeHour - data->bathheader.Hour > 1)
		    dtime -= 3600.0 * 24;
		ntime_d = time_d + dtime;
		
		/* check for use of projected coordinates
			XTF allows projected coordinates like UTM but the format spec
			lists the projection specification values as unused!
			Assume UTM zone 1N as we have to assume something */
		if (mb_io_ptr->projection_initialized == MB_YES)
			{
			mb_proj_inverse(verbose, mb_io_ptr->pjptr,
							data->bathheader.SensorXcoordinate, 
							data->bathheader.SensorYcoordinate,
							&lon, &lat,
							error);
			}
		else
			{
			lon = data->bathheader.SensorXcoordinate;
			lat = data->bathheader.SensorYcoordinate;
			}
		    
		/* add latest fix to list */
		mb_navint_add(verbose, mbio_ptr, ntime_d, lon, lat, error);
		}

	/* translate values to reson data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* type of data record */
		store->kind = data->kind;

		/* type of sonar */
		store->sonar = data->sonar;			/* Type of Reson sonar */
	
		/* parameter info */
		nchan = data->fileheader.NumberOfSonarChannels
			+ data->fileheader.NumberOfBathymetryChannels;
		for (i=0;i<nchan;i++)
			{
			if (data->fileheader.chaninfo[i].TypeOfChannel == 3)
			    {
			    store->MBOffsetX = data->fileheader.chaninfo[i].OffsetX;
			    store->MBOffsetY = data->fileheader.chaninfo[i].OffsetY;
			    store->MBOffsetZ = data->fileheader.chaninfo[i].OffsetZ;
			    }
			}
		store->NavLatency = data->fileheader.NavigationLatency;		/* GPS_time_received - GPS_time_sent (sec) */
		store->NavOffsetY = data->fileheader.NavOffsetY;		/* Nav offset (m) */
		store->NavOffsetX = data->fileheader.NavOffsetX;		/* Nav offset (m) */
		store->NavOffsetZ = data->fileheader.NavOffsetZ; 		/* Nav z offset (m) */
		store->NavOffsetYaw = data->fileheader.NavOffsetZ;		/* Heading offset (m) */
		store->MRUOffsetY = data->fileheader.MRUOffsetY;		/* Multibeam MRU y offset (m) */
		store->MRUOffsetX = data->fileheader.MRUOffsetX;		/* Multibeam MRU x offset (m) */
		store->MRUOffsetZ = data->fileheader.MRUOffsetZ; 		/* Multibeam MRU z offset (m) */
		store->MRUOffsetPitch = data->fileheader.MRUOffsetPitch; 		/* Multibeam MRU pitch offset (degrees) */
		store->MRUOffsetRoll = data->fileheader.MRUOffsetRoll;		/* Multibeam MRU roll offset (degrees) */
	
		/* attitude data */
		store->att_timetag = data->attitudeheader.TimeTag;
		store->att_heading = data->attitudeheader.Heading;
		store->att_heave = data->attitudeheader.Heave;
		store->att_roll = data->attitudeheader.Roll;
		store->att_pitch = data->attitudeheader.Pitch;

		/* comment */
		for (i=0;i<MBSYS_RESON8K_COMMENT_LENGTH;i++)
			store->comment[i] = data->comment[i];

		/* survey data */
		store->png_latency = 0.001 * data->reson8100rit.latency;
		time_i[0] = data->bathheader.Year;
		time_i[1] = data->bathheader.Month;
		time_i[2] = data->bathheader.Day;
		time_i[3] = data->bathheader.Hour;
		time_i[4] = data->bathheader.Minute;
		time_i[5] = data->bathheader.Second;
		time_i[6] = 10000 * data->bathheader.HSeconds;
		mb_get_time(verbose, time_i,  &(store->png_time_d));
		store->png_time_d -= store->png_latency;
		store->png_longitude = data->bathheader.SensorXcoordinate;
		store->png_latitude = data->bathheader.SensorYcoordinate;
		store->png_speed = 0.0;
		
		/* interpolate attitude if possible */
		if (mb_io_ptr->nattitude > 1)
		    {
                    /* time tag is on receive;  average reception is closer 
		    	to the midpoint of the two way travel time
		        but will vary on beam angle and water depth 
		        set the receive time delay to the average 
			( 0 to 60 deg)  two way travel time for a seabed 
		        located at 80% of the maximum range 
		        Old code:
		    timetag = 0.001 * data->bathheader.AttitudeTimeTag 
				    - store->png_latency 
				    + 2.0 * ((double)data->reson8100rit.range_set) 
					    / ((double)data->reson8100rit.velocity); */
		    timetag = 0.001 * data->bathheader.AttitudeTimeTag 
				    - store->png_latency 
				    + 1.4*((double)data->reson8100rit.range_set) 
					    / ((double)data->reson8100rit.velocity);		    
		    mb_attint_interp(verbose, mbio_ptr, timetag,  
			    &(store->png_heave), &(store->png_roll), 
			    &(store->png_pitch), error);
		    mb_hedint_interp(verbose, mbio_ptr, timetag,  
			    &(store->png_heading), error);
#ifdef MBR_XTFR8101_DEBUG
fprintf(stderr, "roll: %d %f %f %f %f   latency:%f time:%f %f roll:%f\n", 
mb_io_ptr->nattitude, 
mb_io_ptr->attitude_time_d[0], 
mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude-1], 
mb_io_ptr->attitude_roll[0], 
mb_io_ptr->attitude_roll[mb_io_ptr->nattitude-1], 
store->png_latency, (double)(0.001 * data->bathheader.AttitudeTimeTag), 
timetag, store->png_roll);
#endif
		    }
		else
		    {
		    store->png_roll = data->bathheader.SensorRoll;
		    store->png_pitch = data->bathheader.SensorPitch;
		    store->png_heading = data->bathheader.SensorHeading;
		    store->png_heave = data->bathheader.Heave;
		    }
		
		/* interpolate nav if possible */
		if (mb_io_ptr->nfix > 0)
			{
			mb_navint_interp(verbose, mbio_ptr, store->png_time_d, store->png_heading, 0.0, 
			    &(store->png_longitude), &(store->png_latitude), &(store->png_speed), error);
		    
			/* now deal with odd case where original nav is in eastings and northings
				- since the projection is initialized, it will be applied when data
				are extracted using mb_extract(), mb_extract_nav(), etc., so we have
				to reproject the lon lat values to eastings northings for now */
			if (mb_io_ptr->projection_initialized == MB_YES)
				{
				mb_proj_forward(verbose, mb_io_ptr->pjptr,
							store->png_longitude, store->png_latitude,
							&(store->png_longitude), &(store->png_latitude),
							error);
				}
			}

		/* get lever arm correction for heave */
		mb_lever(verbose, 
			    (double) store->MBOffsetX,
			    (double) store->MBOffsetY,
			    (double) store->MBOffsetZ,
			    (double) store->NavOffsetX,
			    (double) store->NavOffsetY,
			    (double) store->NavOffsetZ,
			    (double) store->MRUOffsetX,
			    (double) store->MRUOffsetY,
			    (double) store->MRUOffsetZ,
			    (double) (store->png_roll - store->MRUOffsetRoll),
			    (double) (store->MRUOffsetPitch - store->png_pitch),
			    &lever_x,
			    &lever_y,
			    &lever_z,
			    error);
		store->png_heave -= lever_z;
#ifdef MBR_XTFR8101_DEBUG
fprintf(stderr,"offsets: %f %f %f   roll:%f pitch:%f    dz:%f\n", 
store->MBOffsetX - store->MRUOffsetX,
store->MBOffsetY - store->MRUOffsetY,
store->MBOffsetZ - store->MRUOffsetZ,
(double) (store->png_roll - store->MRUOffsetRoll),
(double) (store->MRUOffsetPitch - store->png_pitch),
lever_z);
#endif

		store->packet_type = data->reson8100rit.packet_type;      		/* identifier for packet type  */
		store->packet_subtype = data->reson8100rit.packet_subtype;   		/* identifier for packet subtype */
											/* for dual head system, most significant bit (bit 7) */
											/* indicates which sonar head to associate with packet */
											/* 	head 1 - bit 7 set to 0 */
											/* 	head 2 -	bit 7 set to 1 		 */
		store->latency = data->reson8100rit.latency;          			/* time from ping to output (milliseconds) */
		store->Seconds = data->reson8100rit.Seconds;				/* seconds since 00:00:00, 1 January 1970 */
		store->Millisecs = data->reson8100rit.Millisecs;			/* milliseconds, LSB = 1 ms */
		store->ping_number = data->reson8100rit.ping_number;			/* sequential ping number from sonar startup/reset */
		store->sonar_id = data->reson8100rit.sonar_id;				/* least significant four bytes of Ethernet address */
		store->sonar_model = data->reson8100rit.sonar_model;			/* coded model number of sonar */
		store->frequency = data->reson8100rit.frequency;			/* sonar frequency in KHz */
		store->velocity = data->reson8100rit.velocity;         			/* programmed sound velocity (LSB = 1 m/sec) */
		store->sample_rate = data->reson8100rit.sample_rate;      		/* A/D sample rate (samples per second) */
		store->ping_rate = data->reson8100rit.ping_rate;        		/* Ping rate (pings per second * 1000) */
		store->range_set = data->reson8100rit.range_set;        		/* range setting for SeaBat (meters ) */
		store->power = data->reson8100rit.power;            			/* power setting for SeaBat  	 */
											/* bits	0-4 -	power (0 - 8) */
		store->gain = data->reson8100rit.gain;             			/* gain setting for SeaBat */
											/* bits	0-6 -	gain (1 - 45) */
											/* bit 	14	(0 = fixed, 1 = tvg) */
											/* bit	15	(0 = manual, 1 = auto) */
		store->pulse_width = data->reson8100rit.pulse_width;      		/* transmit pulse width (microseconds) */
		store->tvg_spread = data->reson8100rit.tvg_spread;			/* spreading coefficient for tvg * 4  */
											/* valid values = 0 to 240 (0.0 to 60.0 in 0.25 steps) */
		store->tvg_absorp = data->reson8100rit.tvg_absorp;			/* absorption coefficient for tvg */
		store->projector_type = data->reson8100rit.projector_type;      	/* bits 0-4 = projector type */
							/* 0 = stick projector */
							/* 1 = array face */
							/* 2 = ER projector */
							/* bit 7 - pitch steering (1=enabled, 0=disabled) */
		store->projector_beam_width = data->reson8100rit.projector_beam_width;	/* along track transmit beam width (degrees * 10) */
		store->beam_width_num = data->reson8100rit.beam_width_num;   	/* cross track receive beam width numerator */
		store->beam_width_denom = data->reson8100rit.beam_width_denom; 	/* cross track receive beam width denominator */
							/* beam width degrees = numerator / denominator */
		store->projector_angle = data->reson8100rit.projector_angle;		/* projector pitch steering angle (degrees * 100) */
		store->min_range = data->reson8100rit.min_range;		/* sonar filter settings */
		store->max_range = data->reson8100rit.max_range;
		store->min_depth = data->reson8100rit.min_depth;
		store->max_depth = data->reson8100rit.max_depth;
		store->filters_active = data->reson8100rit.filters_active;		/* range/depth filters active  */
							/* bit 0 - range filter (0 = off, 1 = active) */
							/* bit 1 - depth filter (0 = off, 1 = active) */
		store->temperature = data->reson8100rit.temperature;		/* temperature at sonar head (deg C * 10) */
		store->beam_count = data->reson8100rit.beam_count;       		/* number of sets of beam data in packet */
		for (i=0;i<store->beam_count;i++)
			store->range[i] = data->reson8100rit.range[i]; 		/* range for beam where n = Beam Count */
							/* range units = sample cells * 4 */
		for (i=0;i<store->beam_count/2+1;i++)
			store->quality[i] = data->reson8100rit.quality[i];   		/* packed quality array (two 4 bit values/char) */
							/* cnt = n/2 if beam count even, n/2+1 if odd */
							/* cnt then rounded up to next even number */
							/* e.g. if beam count=101, cnt=52  */
							/* unused trailing quality values set to zero */
							/* bit 0 - brightness test (0=failed, 1=passed) */
							/* bit 1 - colinearity test (0=failed, 1=passed) */
							/* bit 2 - amplitude bottom detect used */
							/* bit 3 - phase bottom detect used */
							/* bottom detect can be amplitude, phase or both */
		intensity_max = 0;
		for (i=0;i<store->beam_count;i++)
			{
			store->intensity[i] = data->reson8100rit.intensity[i];   		/* intensities at bottom detect  */
			intensity_max = MAX(intensity_max, (int)store->intensity[i]);
			}

		store->beams_bath = data->reson8100rit.beam_count;
		if (intensity_max > 0)
			store->beams_amp = store->beams_bath;
		else
			store->beams_amp = 0;
		
		/* ttscale in seconds per range count ( 4 counts per time interval) */
		ttscale = 0.25 / store->sample_rate;
		icenter = store->beams_bath / 2;
		angscale = ((double)store->beam_width_num) 
			/ ((double)store->beam_width_denom);
		for (i=0;i<store->beams_bath;i++)
			{
			/* get beamflag */
			if (i % 2 == 0)
				quality = ((store->quality[i/2]) & 15) & 3;
			else
				quality = ((store->quality[i/2] >> 4) & 15) & 3;
			if (quality == 0)
				store->beamflag[i] = MB_FLAG_NULL;
			else if (quality < 3)
				store->beamflag[i] = MB_FLAG_FLAG + MB_FLAG_SONAR;
			else
				store->beamflag[i] = MB_FLAG_NONE;

			if (store->beamflag[i] == MB_FLAG_NULL)
				{
				store->bath[i] = 0.0;		/* bathymetry (m) */	
				store->bath_acrosstrack[i] = 0.0;/* acrosstrack distance (m) */
				store->bath_alongtrack[i] = 0.0;	/* alongtrack distance (m) */
				}
			else
				{
				angle = 90.0 + (icenter - i) * angscale + store->png_roll; 
				mb_rollpitch_to_takeoff(
					verbose, 
					store->png_pitch, angle, 
					&theta, &phi, 
					error);
				rr = 0.5 * store->velocity * ttscale * store->range[i];
				xx = rr * sin(DTR * theta);
				zz = rr * cos(DTR * theta);
				store->bath_acrosstrack[i] 
					= xx * cos(DTR * phi);
				store->bath_alongtrack[i] 
					= xx * sin(DTR * phi);
				store->bath[i] = zz - store->png_heave
						+ store->MBOffsetZ;
/*if (i==store->beams_bath/2 && timetag > 1.0)
fprintf(stderr,"%f %f %f %f %f\n",timetag,zz,store->png_heave,lever_z,store->bath[i]);*/
				}
			}
		gain_correction = 2.2 * (store->gain & 63) + 6 * store->power;
		for (i=0;i<store->beams_amp;i++)
			{
			store->amp[i] = (double)(40.0 * log10(store->intensity[i])- gain_correction);
			}
		store->ssrawtimedelay = data->pingchanportheader.TimeDelay;
		store->ssrawtimeduration = data->pingchanportheader.TimeDuration;
		store->ssrawbottompick = data->sidescanheader.SensorPrimaryAltitude 
					    / data->sidescanheader.SoundVelocity;
		store->ssrawportsamples = data->pingchanportheader.NumSamples;
		store->ssrawstbdsamples = data->pingchanstbdheader.NumSamples;
		for (i=0;i<store->ssrawportsamples;i++)
		    store->ssrawport[i] = data->ssrawport[store->ssrawportsamples - i - 1];
		for (i=0;i<store->ssrawstbdsamples;i++)
		    store->ssrawstbd[i] = data->ssrawstbd[i];
				
		/* generate processed sidescan */
		store->pixel_size = 0.0;
		store->pixels_ss = 0;
		status = mbsys_reson8k_makess(verbose,
				mbio_ptr, store_ptr,
				MB_NO, pixel_size, 
				MB_NO, swath_width, 
				error);
		}
	
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_wt_xtfr8101(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_xtfr8101";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %ld\n",(size_t)store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
		
	/* set error as this is a read only format */
	status = MB_FAILURE;
	*error = MB_ERROR_WRITE_FAIL;	

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_xtfr8101_rd_data(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_xtfr8101_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_xtfr8101_struct *data;
	char	line[MBF_XTFR8101_MAXLINE];
	int	*fileheaderread;
	struct mbf_xtfr8101_xtffileheader *fileheader;
	struct mbf_xtfpacketheader packetheader;
	struct mbf_xtfattitudeheader *attitudeheader;
	struct mbf_xtfbathheader *bathheader;
	struct RESON8100_RIT *reson8100rit;
	struct mbf_xtfbathheader *sidescanheader;
	struct mbf_xtfpingchanheader *pingchanportheader;
	struct mbf_xtfpingchanheader *pingchanstbdheader;
	int	index;
	int	ichan;
	int	done, found;
	int	synch;
	int	read_len, read_bytes;
	int	skip;
	int	quality;
	mb_u_char *mb_u_char_ptr;
	double	timetag, heave, roll, pitch, heading;
	int	utm_zone;
	char	projection[MB_NAME_LENGTH];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
				
	/* set saved flags */
	fileheaderread = (int *) &(mb_io_ptr->save1);

	/* get pointer to raw data structure */
	data = (struct mbf_xtfr8101_struct *) mb_io_ptr->raw_data;
	fileheader = (struct mbf_xtfr8101_xtffileheader *) &(data->fileheader);
	attitudeheader = (struct mbf_xtfattitudeheader *) &(data->attitudeheader);
	bathheader = (struct mbf_xtfbathheader *) &(data->bathheader);
	reson8100rit = (struct RESON8100_RIT *) &(data->reson8100rit);
	sidescanheader = (struct mbf_xtfbathheader *) &(data->sidescanheader);
	pingchanportheader = (struct mbf_xtfpingchanheader *) &(data->pingchanportheader);
	pingchanstbdheader = (struct mbf_xtfpingchanheader *) &(data->pingchanstbdheader);

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
		
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	
	/* read file header if required */
	if (*fileheaderread == MB_NO)
	    {
	    read_len = fread(line,1,MBF_XTFR8101_FILEHEADERLEN,mb_io_ptr->mbfp);
	    if (read_len == MBF_XTFR8101_FILEHEADERLEN)
		{
		/* extract data from buffer */
		*fileheaderread = MB_YES;
		status = MB_SUCCESS;
		index = 0;
		fileheader->FileFormat = line[index]; 
		index++;
		fileheader->SystemType = line[index]; 
		index++;
		for (i=0;i<8;i++)
			fileheader->RecordingProgramName[i] = line[index+i]; 
		index += 8;
		for (i=0;i<8;i++)
			fileheader->RecordingProgramVersion[i] = line[index+i];
		index += 8;
		for (i=0;i<16;i++)
			fileheader->SonarName[i] = line[index+i];
		index += 16;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->SonarType)); 
		index += 2;
		for (i=0;i<64;i++)
			fileheader->NoteString[i] = line[index+i];
		index += 64;
		for (i=0;i<64;i++)
			fileheader->ThisFileName[i] = line[index+i];
		index += 64;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->NavUnits));
		index += 2; 
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->NumberOfSonarChannels)); 
		index += 2; 
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->NumberOfBathymetryChannels)); 
		index += 2; 
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->Reserved1)); 
		index += 2; 
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->Reserved2)); 
		index += 2; 
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->Reserved3)); 
		index += 2; 
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->Reserved4)); 
		index += 2; 
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->Reserved5)); 
		index += 2; 
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->Reserved6)); 
		index += 2; 
		for (i=0;i<12;i++)
			fileheader->ProjectionType[i] = line[index+i];
		index += 12;
		for (i=0;i<10;i++)
			fileheader->SpheroidType[i] = line[index+i];
		index += 10;
		mb_get_binary_int(MB_YES, &line[index], (int *)&(fileheader->NavigationLatency)); 
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->OriginY)); 
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->OriginX)); 
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->NavOffsetY)); 
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->NavOffsetX)); 
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->NavOffsetZ)); 
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->NavOffsetYaw)); 
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->MRUOffsetY)); 
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->MRUOffsetX)); 
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->MRUOffsetZ)); 
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->MRUOffsetYaw)); 
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->MRUOffsetPitch)); 
		index += 4;
		mb_get_binary_float(MB_YES, &line[index], &(fileheader->MRUOffsetRoll)); 
		index += 4;
		for (ichan=0;ichan<6;ichan++)
			{
			fileheader->chaninfo[ichan].TypeOfChannel = line[index];
			index++;
			fileheader->chaninfo[ichan].SubChannelNumber = line[index];
			index++;
			mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->chaninfo[ichan].CorrectionFlags)); 
			index += 2;
			mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->chaninfo[ichan].UniPolar)); 
			index += 2;
			mb_get_binary_short(MB_YES, &line[index], (short int *)&(fileheader->chaninfo[ichan].BytesPerSample)); 
			index += 2;
			mb_get_binary_int(MB_YES, &line[index], (int *)&(fileheader->chaninfo[ichan].SamplesPerChannel)); 
			index += 4;
			for (i=0;i<16;i++)
				fileheader->chaninfo[ichan].ChannelName[i] = line[index+i];
			index += 16;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].VoltScale)); 
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].Frequency)); 
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].HorizBeamAngle)); 
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].TiltAngle)); 
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].BeamWidth)); 
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].OffsetX)); 
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].OffsetY)); 
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].OffsetZ)); 
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].OffsetYaw)); 
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].OffsetPitch)); 
			index += 4;
			mb_get_binary_float(MB_YES, &line[index], &(fileheader->chaninfo[ichan].OffsetRoll)); 
			index += 4;
			for (i=0;i<56;i++)
				fileheader->chaninfo[ichan].ReservedArea[i] = line[index+i];
			index += 56;
			}

		/* if NavUnits indicates use of projected coordinates (the format spec
			indicates the projection parameters are unused!) assume UTM zone 1N 
			and set up the projection */
		if (fileheader->NavUnits == 0 && mb_io_ptr->projection_initialized == MB_NO)
			{
			/* initialize UTM projection */
			utm_zone = (int)(((RTD * 0.0 + 183.0)
					/ 6.0) + 0.5);
			sprintf(projection,"UTM%2.2dN", utm_zone);
			mb_proj_init(verbose, projection, &(mb_io_ptr->pjptr), error);
			mb_io_ptr->projection_initialized = MB_YES;
			}

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg5       FileFormat:                 %d\n",fileheader->FileFormat);
			fprintf(stderr,"dbg5       SystemType:                 %d\n",fileheader->SystemType);
			fprintf(stderr,"dbg5       RecordingProgramName:       %s\n",fileheader->RecordingProgramName);
			fprintf(stderr,"dbg5       RecordingProgramVersion:    %s\n",fileheader->RecordingProgramVersion);
			fprintf(stderr,"dbg5       SonarName:                  %s\n",fileheader->SonarName);
			fprintf(stderr,"dbg5       SonarType:                  %d\n",fileheader->SonarType);
			fprintf(stderr,"dbg5       NoteString:                 %s\n",fileheader->NoteString);
			fprintf(stderr,"dbg5       ThisFileName:               %s\n",fileheader->ThisFileName);
			fprintf(stderr,"dbg5       NavUnits:                   %d\n",fileheader->NavUnits);
			fprintf(stderr,"dbg5       NumberOfSonarChannels:      %d\n",fileheader->NumberOfSonarChannels);
			fprintf(stderr,"dbg5       NumberOfBathymetryChannels: %d\n",fileheader->NumberOfBathymetryChannels);
			fprintf(stderr,"dbg5       Reserved1:                  %d\n",fileheader->Reserved1);
			fprintf(stderr,"dbg5       Reserved2:                  %d\n",fileheader->Reserved2);
			fprintf(stderr,"dbg5       Reserved3:                  %d\n",fileheader->Reserved3);
			fprintf(stderr,"dbg5       Reserved4:                  %d\n",fileheader->Reserved4);
			fprintf(stderr,"dbg5       Reserved5:                  %d\n",fileheader->Reserved5);
			fprintf(stderr,"dbg5       Reserved6:                  %d\n",fileheader->Reserved6);
			fprintf(stderr,"dbg5       ProjectionType:             %s\n",fileheader->ProjectionType);
			fprintf(stderr,"dbg5       SpheroidType:               %s\n",fileheader->SpheroidType);
			fprintf(stderr,"dbg5       NavigationLatency:          %d\n",fileheader->NavigationLatency);
			fprintf(stderr,"dbg5       OriginY:                    %f\n",fileheader->OriginY);
			fprintf(stderr,"dbg5       OriginX:                    %f\n",fileheader->OriginX);
			fprintf(stderr,"dbg5       NavOffsetY:                 %f\n",fileheader->NavOffsetY);
			fprintf(stderr,"dbg5       NavOffsetX:                 %f\n",fileheader->NavOffsetX);
			fprintf(stderr,"dbg5       NavOffsetZ:                 %f\n",fileheader->NavOffsetZ);
			fprintf(stderr,"dbg5       NavOffsetYaw:               %f\n",fileheader->NavOffsetYaw);
			fprintf(stderr,"dbg5       MRUOffsetY:                 %f\n",fileheader->MRUOffsetY);
			fprintf(stderr,"dbg5       MRUOffsetX:                 %f\n",fileheader->MRUOffsetX);
			fprintf(stderr,"dbg5       MRUOffsetZ:                 %f\n",fileheader->MRUOffsetZ);
			fprintf(stderr,"dbg5       MRUOffsetYaw:               %f\n",fileheader->MRUOffsetYaw);
			fprintf(stderr,"dbg5       MRUOffsetPitch:             %f\n",fileheader->MRUOffsetPitch);
			fprintf(stderr,"dbg5       MRUOffsetRoll:              %f\n",fileheader->MRUOffsetRoll);
			for (i=0;i<fileheader->NumberOfSonarChannels 
				+ fileheader->NumberOfBathymetryChannels;i++)
			    {
			    fprintf(stderr,"dbg5       TypeOfChannel:              %d\n",fileheader->chaninfo[i].TypeOfChannel);
			    fprintf(stderr,"dbg5       SubChannelNumber:           %d\n",fileheader->chaninfo[i].SubChannelNumber);
			    fprintf(stderr,"dbg5       CorrectionFlags:            %d\n",fileheader->chaninfo[i].CorrectionFlags);
			    fprintf(stderr,"dbg5       UniPolar:                   %d\n",fileheader->chaninfo[i].UniPolar);
			    fprintf(stderr,"dbg5       BytesPerSample:             %d\n",fileheader->chaninfo[i].BytesPerSample);
			    fprintf(stderr,"dbg5       SamplesPerChannel:          %d\n",fileheader->chaninfo[i].SamplesPerChannel);
			    fprintf(stderr,"dbg5       ChannelName:                %s\n",fileheader->chaninfo[i].ChannelName);
			    fprintf(stderr,"dbg5       VoltScale:                  %f\n",fileheader->chaninfo[i].VoltScale);
			    fprintf(stderr,"dbg5       Frequency:                  %f\n",fileheader->chaninfo[i].Frequency);
			    fprintf(stderr,"dbg5       HorizBeamAngle:             %f\n",fileheader->chaninfo[i].HorizBeamAngle);
			    fprintf(stderr,"dbg5       TiltAngle:                  %f\n",fileheader->chaninfo[i].TiltAngle);
			    fprintf(stderr,"dbg5       BeamWidth:                  %f\n",fileheader->chaninfo[i].BeamWidth);
			    fprintf(stderr,"dbg5       OffsetX:                    %f\n",fileheader->chaninfo[i].OffsetX);
			    fprintf(stderr,"dbg5       OffsetY:                    %f\n",fileheader->chaninfo[i].OffsetY);
			    fprintf(stderr,"dbg5       OffsetZ:                    %f\n",fileheader->chaninfo[i].OffsetZ);
			    fprintf(stderr,"dbg5       OffsetYaw:                  %f\n",fileheader->chaninfo[i].OffsetYaw);
			    fprintf(stderr,"dbg5       OffsetPitch:                %f\n",fileheader->chaninfo[i].OffsetPitch);
			    fprintf(stderr,"dbg5       OffsetRoll:                 %f\n",fileheader->chaninfo[i].OffsetRoll);
			    fprintf(stderr,"dbg5       ReservedArea:               %s\n",fileheader->chaninfo[i].ReservedArea);
			    }
			}
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	    }
		
	/* look for next recognizable record */
	done = MB_NO;
	while (status == MB_SUCCESS && done == MB_NO)
	    {
	    /* find the next packet beginning */
	    found = MB_NO;
	    skip = 0;
	    read_len = fread(line,1,2,mb_io_ptr->mbfp);
	    if (read_len != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	    else if (((mb_u_char)line[0]) == 0xce && ((mb_u_char)line[1] == 0xfa))
		found = MB_YES;
	    while (status == MB_SUCCESS 
		&& found == MB_NO)
		{
		line[0] = line[1];
		read_len = fread(&(line[1]),1,1,mb_io_ptr->mbfp);
		skip++;
		if (read_len != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		else if (((mb_u_char)line[0]) == 0xce && ((mb_u_char)line[1] == 0xfa))
			found = MB_YES;
		}

	    /* read the next packet header */
	    read_len = fread(&(line[2]),1,12,mb_io_ptr->mbfp);
	    if (read_len == 12)
		{
		/* extract data from buffer */
		index = 0;
		packetheader.MagicNumber[0] = line[index]; 
		index++;
		packetheader.MagicNumber[1] = line[index]; 
		index++;
		packetheader.HeaderType = line[index]; 
		index++;
		packetheader.SubChannelNumber = line[index]; 
		index++;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(packetheader.NumChansToFollow)); 
		index += 2;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(packetheader.Reserved1[0])); 
		index += 2;
		mb_get_binary_short(MB_YES, &line[index], (short int *)&(packetheader.Reserved1[1])); 
		index += 2;
		mb_get_binary_int(MB_YES, &line[index], (int *)&(packetheader.NumBytesThisRecord)); 
		index += 4;

		/* check packet header details */
		if( packetheader.NumChansToFollow > 20) 
			{
			if (verbose > 0)
				fprintf(stderr,"Bad packet header in xtf - skip this record\n");
			packetheader.NumBytesThisRecord = 0;
			packetheader.HeaderType = 99;
			}
		
		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg5       Bytes Skipped:              %d\n",skip);
			fprintf(stderr,"dbg5       MagicNumber:                %d %d %x%x\n",
					packetheader.MagicNumber[0],packetheader.MagicNumber[1],
					packetheader.MagicNumber[0],packetheader.MagicNumber[1]);
			fprintf(stderr,"dbg5       HeaderType:                 %d\n",packetheader.HeaderType);
			fprintf(stderr,"dbg5       SubChannelNumber:           %d\n",packetheader.SubChannelNumber);
			fprintf(stderr,"dbg5       NumChansToFollow:           %d\n",packetheader.NumChansToFollow);
			fprintf(stderr,"dbg5       Reserved1:                  %d %d\n",packetheader.Reserved1[0],packetheader.Reserved1[1]);
			fprintf(stderr,"dbg5       NumBytesThisRecord:         %d\n",packetheader.NumBytesThisRecord);
			}
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		done = MB_YES;
		}
		
	    /* read rest of attitude packet */
	    if (status == MB_SUCCESS
		&& packetheader.HeaderType == XTF_DATA_ATTITUDE
		&& packetheader.NumBytesThisRecord == 64)
		{
#ifdef MBR_XTFR8101_DEBUG
fprintf(stderr,"Reading attitude packet type:%d bytes:%d\n",
packetheader.HeaderType,packetheader.NumBytesThisRecord);
#endif
		attitudeheader->packetheader = packetheader;
		read_len = fread(line,1,50,mb_io_ptr->mbfp);
		if (read_len == 50)
		    {
		    /* parse the rest of the attitude record */
		    index = 0;
		    for (i=0;i<4;i++)
			{
			mb_get_binary_int(MB_YES, &line[index], (int *)&(attitudeheader->Reserved2[i]));
			index += 4;
			}
		    mb_get_binary_float(MB_YES, &line[index], &(attitudeheader->Pitch)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(attitudeheader->Roll)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(attitudeheader->Heave)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(attitudeheader->Yaw)); 
		    index += 4;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(attitudeheader->TimeTag)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(attitudeheader->Heading)); 
		    index += 4;
		    for (i=0;i<10;i++)
			{
			attitudeheader->Reserved3[i] = line[index];
			index++;
			}
			
		    /* add attitude to list for interpolation */
		    timetag = 0.001 *  attitudeheader->TimeTag;
		    heave = attitudeheader->Heave;
		    roll = attitudeheader->Roll;
		    pitch = attitudeheader->Pitch;
		    heading = attitudeheader->Heading;

		    /* add latest attitude to list */
		    mb_attint_add(verbose, mbio_ptr, 
				    timetag, heave, roll, pitch, 
				    error);
		    mb_hedint_add(verbose, mbio_ptr, 
				    timetag, heading, error);
	
		    /* print debug statements */
		    if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg5       MagicNumber:                %d %d %x%x\n",
					attitudeheader->packetheader.MagicNumber[0],attitudeheader->packetheader.MagicNumber[1],
					attitudeheader->packetheader.MagicNumber[0],attitudeheader->packetheader.MagicNumber[1]);
			fprintf(stderr,"dbg5       HeaderType:                 %d\n",attitudeheader->packetheader.HeaderType);
			fprintf(stderr,"dbg5       SubChannelNumber:           %d\n",attitudeheader->packetheader.SubChannelNumber);
			fprintf(stderr,"dbg5       NumChansToFollow:           %d\n",attitudeheader->packetheader.NumChansToFollow);
			fprintf(stderr,"dbg5       Reserved1:                  %d %d\n",attitudeheader->packetheader.Reserved1[0],attitudeheader->packetheader.Reserved1[1]);
			fprintf(stderr,"dbg5       NumBytesThisRecord:         %d\n",attitudeheader->packetheader.NumBytesThisRecord);
			fprintf(stderr,"dbg5       Reserved2[0]:               %d\n",attitudeheader->Reserved2[0]);
			fprintf(stderr,"dbg5       Reserved2[1]:               %d\n",attitudeheader->Reserved2[1]);
			fprintf(stderr,"dbg5       Reserved2[2]:               %d\n",attitudeheader->Reserved2[2]);
			fprintf(stderr,"dbg5       Reserved2[3]:               %d\n",attitudeheader->Reserved2[3]);
			fprintf(stderr,"dbg5       Pitch:                      %f\n",attitudeheader->Pitch);
			fprintf(stderr,"dbg5       Roll:                       %f\n",attitudeheader->Roll);
			fprintf(stderr,"dbg5       Heave:                      %f\n",attitudeheader->Heave);
			fprintf(stderr,"dbg5       Yaw:                        %f\n",attitudeheader->Yaw);
			fprintf(stderr,"dbg5       TimeTag:                    %d\n",attitudeheader->TimeTag);
			fprintf(stderr,"dbg5       Heading:                    %f\n",attitudeheader->Heading);
			}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    done = MB_YES;
		    }
		}
		
	    /* read rest of sidescan packet */
	    else if (status == MB_SUCCESS
		&& packetheader.HeaderType == XTF_DATA_SIDESCAN)
		{
#ifdef MBR_XTFR8101_DEBUG
fprintf(stderr,"Reading sidescan packet type:%d bytes:%d\n",
packetheader.HeaderType,packetheader.NumBytesThisRecord);
#endif
		/* read and parse the the sidescan header */
		sidescanheader->packetheader = packetheader;
		read_len = fread(line,1,242,mb_io_ptr->mbfp);
		if (read_len == 242)
		    {
		    index = 0;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(sidescanheader->Year)); 
		    index += 2;
		    sidescanheader->Month = line[index];
		    index++;
		    sidescanheader->Day = line[index];
		    index++;
		    sidescanheader->Hour = line[index];
		    index++;
		    sidescanheader->Minute = line[index];
		    index++;
		    sidescanheader->Second = line[index];
		    index++;
		    sidescanheader->HSeconds = line[index];
		    index++;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(sidescanheader->JulianDay)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(sidescanheader->CurrentLineID)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(sidescanheader->EventNumber)); 
		    index += 2;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(sidescanheader->PingNumber)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->SoundVelocity)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->OceanTide)); 
		    index += 4;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(sidescanheader->Reserved2)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->ConductivityFreq)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->TemperatureFreq)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->PressureFreq)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->PressureTemp)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->Conductivity)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->WaterTemperature)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->Pressure)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->ComputedSoundVelocity)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->MagX)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->MagY)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->MagZ)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->AuxVal1)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->AuxVal2)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->AuxVal3)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->AuxVal4)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->AuxVal5)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->AuxVal6)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->SpeedLog)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->Turbidity)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->ShipSpeed)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->ShipGyro)); 
		    index += 4;
		    mb_get_binary_double(MB_YES, &line[index], &(sidescanheader->ShipYcoordinate)); 
		    index += 8;
		    mb_get_binary_double(MB_YES, &line[index], &(sidescanheader->ShipXcoordinate)); 
		    index += 8;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(sidescanheader->ShipAltitude)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(sidescanheader->ShipDepth)); 
		    index += 2;
		    sidescanheader->FixTimeHour = line[index];
		    index++;
		    sidescanheader->FixTimeMinute = line[index];
		    index++;
		    sidescanheader->FixTimeSecond = line[index];
		    index++;
		    sidescanheader->Reserved4 = line[index];
		    index++;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->SensorSpeed)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->KP)); 
		    index += 4;
		    mb_get_binary_double(MB_YES, &line[index], &(sidescanheader->SensorYcoordinate)); 
		    index += 8;
		    mb_get_binary_double(MB_YES, &line[index], &(sidescanheader->SensorXcoordinate)); 
		    index += 8;
		    mb_get_binary_short(MB_YES, &line[index], &(sidescanheader->Reserved6)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], &(sidescanheader->RangeToSensor)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], &(sidescanheader->BearingToSensor)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], &(sidescanheader->CableOut)); 
		    index += 2;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->Layback)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->CableTension)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->SensorDepth)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->SensorPrimaryAltitude)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->SensorAuxAltitude)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->SensorPitch)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->SensorRoll)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->SensorHeading)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->Heave)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->Yaw)); 
		    index += 4;
		    mb_get_binary_int(MB_YES, &line[index], &(sidescanheader->AttitudeTimeTag)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(sidescanheader->DOT)); 
		    index += 4;
		    for (i=0;i<20;i++)
			{
			sidescanheader->ReservedSpace[i] = line[index];
			index++;
			}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    done = MB_YES;
		    }
			
		/* read and parse the port sidescan channel header */
		if (status == MB_SUCCESS)
		    read_len = fread(line,1,64,mb_io_ptr->mbfp);
		if (status == MB_SUCCESS && read_len == 64)
		    {
		    index = 0;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->ChannelNumber)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->DownsampleMethod)); 
		    index += 2;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanportheader->SlantRange)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanportheader->GroundRange)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanportheader->TimeDelay)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanportheader->TimeDuration)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanportheader->SecondsPerPing)); 
		    index += 4;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->ProcessingFlags)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->Frequency)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->InitialGainCode)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->GainCode)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->BandWidth)); 
		    index += 2;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(pingchanportheader->ContactNumber)); 
		    index += 4;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->ContactClassification)); 
		    index += 2;
		    pingchanportheader->ContactSubNumber = (mb_u_char) line[index];
		    index++;
		    pingchanportheader->ContactType = (mb_u_char) line[index];
		    index++;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(pingchanportheader->NumSamples)); 
		    index += 4;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanportheader->Reserved)); 
		    index += 2;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanportheader->ContactTimeOffTrack)); 
		    index += 4;
		    pingchanportheader->ContactCloseNumber = (mb_u_char) line[index];
		    index++;
		    pingchanportheader->Reserved2 = (mb_u_char) line[index];
		    index++;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanportheader->FixedVSOP)); 
		    index += 4;
		    for (i=0;i<6;i++)
			{
			pingchanportheader->ReservedSpace[i] = (mb_u_char) line[index];
			index++;
			}
		    
		    /* fix up on time duration if needed */
		    if ( pingchanportheader->TimeDuration == 0.0) 
		    	{
		    	pingchanportheader->TimeDuration 
				= pingchanportheader->SlantRange 
					/ sidescanheader->SoundVelocity;
		    	}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    done = MB_YES;
		    }
		    
		/* check for corrupted record */
		if (pingchanportheader->ChannelNumber 
			> (fileheader->NumberOfSonarChannels 
				+ fileheader->NumberOfBathymetryChannels - 1))
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		else if (pingchanportheader->NumSamples 
			> fileheader->chaninfo[pingchanportheader->ChannelNumber].SamplesPerChannel)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}

		/* read port sidescan data */
		if (status == MB_SUCCESS)
		    {
		    read_bytes = pingchanportheader->NumSamples 
				    * fileheader->chaninfo[pingchanportheader->ChannelNumber].BytesPerSample;
		    read_len = fread(line,1,read_bytes,mb_io_ptr->mbfp);
		    }
		if (status == MB_SUCCESS && read_len == read_bytes)
		    {
		    if (fileheader->chaninfo[pingchanportheader->ChannelNumber].BytesPerSample == 1)
			{
			for (i=0;i<pingchanportheader->NumSamples;i++)
			    {
			    mb_u_char_ptr = (mb_u_char *) &line[i];
			    data->ssrawport[i] = (unsigned short) (*mb_u_char_ptr);
			    }
			}
		    else if (fileheader->chaninfo[pingchanportheader->ChannelNumber].BytesPerSample == 2)
			{
			index = 0;
			for (i=0;i<pingchanportheader->NumSamples;i++)
			    {
			    mb_get_binary_short(MB_YES, &line[index], (short *)&(data->ssrawport[i])); 
			    index += 2;
			    }
			}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    done = MB_YES;
		    }
			
		/* read and parse the starboard sidescan channel header */
		if (status == MB_SUCCESS)
		    read_len = fread(line,1,64,mb_io_ptr->mbfp);
		if (status == MB_SUCCESS && read_len == 64)
		    {
		    index = 0;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->ChannelNumber)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->DownsampleMethod)); 
		    index += 2;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanstbdheader->SlantRange)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanstbdheader->GroundRange)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanstbdheader->TimeDelay)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanstbdheader->TimeDuration)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanstbdheader->SecondsPerPing)); 
		    index += 4;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->ProcessingFlags)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->Frequency)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->InitialGainCode)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->GainCode)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->BandWidth)); 
		    index += 2;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(pingchanstbdheader->ContactNumber)); 
		    index += 4;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->ContactClassification)); 
		    index += 2;
		    pingchanstbdheader->ContactSubNumber = (mb_u_char) line[index];
		    index++;
		    pingchanstbdheader->ContactType = (mb_u_char) line[index];
		    index++;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(pingchanstbdheader->NumSamples)); 
		    index += 4;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(pingchanstbdheader->Reserved)); 
		    index += 2;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanstbdheader->ContactTimeOffTrack)); 
		    index += 4;
		    pingchanstbdheader->ContactCloseNumber = (mb_u_char) line[index];
		    index++;
		    pingchanstbdheader->Reserved2 = (mb_u_char) line[index];
		    index++;
		    mb_get_binary_float(MB_YES, &line[index], &(pingchanstbdheader->FixedVSOP)); 
		    index += 4;
		    for (i=0;i<6;i++)
			{
			pingchanstbdheader->ReservedSpace[i] = (mb_u_char) line[index];
			index++;
			}
		    
		    /* fix up on time duration if needed */
		    if ( pingchanstbdheader->TimeDuration == 0.0) 
		    	{
		    	pingchanstbdheader->TimeDuration 
				= pingchanstbdheader->SlantRange 
					/ sidescanheader->SoundVelocity;
		    	}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    done = MB_YES;
		    }
		    
		/* check for corrupted record */
		if (pingchanstbdheader->ChannelNumber 
			> (fileheader->NumberOfSonarChannels 
				+ fileheader->NumberOfBathymetryChannels - 1))
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		else if (pingchanstbdheader->NumSamples 
			> fileheader->chaninfo[pingchanstbdheader->ChannelNumber].SamplesPerChannel)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}

		/* read starboard sidescan data */
		if (status == MB_SUCCESS)
		    {
		    read_bytes = pingchanstbdheader->NumSamples 
				    * fileheader->chaninfo[pingchanstbdheader->ChannelNumber].BytesPerSample;
		    read_len = fread(line,1,read_bytes,mb_io_ptr->mbfp);
		    }
		if (status == MB_SUCCESS && read_len == read_bytes)
		    {
		    if (fileheader->chaninfo[pingchanstbdheader->ChannelNumber].BytesPerSample == 1)
			{
			for (i=0;i<pingchanstbdheader->NumSamples;i++)
			    {
			    mb_u_char_ptr = (mb_u_char *) &line[i];
			    data->ssrawstbd[i] = (unsigned short) (*mb_u_char_ptr);
			    }
			}
		    else if (fileheader->chaninfo[pingchanstbdheader->ChannelNumber].BytesPerSample == 2)
			{
			index = 0;
			for (i=0;i<pingchanstbdheader->NumSamples;i++)
			    {
			    mb_get_binary_short(MB_YES, &line[index], (short *)&(data->ssrawstbd[i])); 
			    index += 2;
			    }
			}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    done = MB_YES;
		    }

		/* print debug statements */
		if (verbose >= 5)
		    {
		    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		    fprintf(stderr,"dbg5       MagicNumber:                %d %d %x%x\n",
				    sidescanheader->packetheader.MagicNumber[0],sidescanheader->packetheader.MagicNumber[1],
				    sidescanheader->packetheader.MagicNumber[0],sidescanheader->packetheader.MagicNumber[1]);
		    fprintf(stderr,"dbg5       HeaderType:                 %d\n",sidescanheader->packetheader.HeaderType);
		    fprintf(stderr,"dbg5       SubChannelNumber:           %d\n",sidescanheader->packetheader.SubChannelNumber);
		    fprintf(stderr,"dbg5       NumChansToFollow:           %d\n",sidescanheader->packetheader.NumChansToFollow);
		    fprintf(stderr,"dbg5       Reserved1:                  %d %d\n",sidescanheader->packetheader.Reserved1[0],sidescanheader->packetheader.Reserved1[1]);
		    fprintf(stderr,"dbg5       NumBytesThisRecord:         %d\n",sidescanheader->packetheader.NumBytesThisRecord);
		    fprintf(stderr,"dbg5       Year:                       %d\n",sidescanheader->Year);
		    fprintf(stderr,"dbg5       Month:                      %d\n",sidescanheader->Month);
		    fprintf(stderr,"dbg5       Day:                        %d\n",sidescanheader->Day);
		    fprintf(stderr,"dbg5       Hour:                       %d\n",sidescanheader->Hour);
		    fprintf(stderr,"dbg5       Minute:                     %d\n",sidescanheader->Minute);
		    fprintf(stderr,"dbg5       Second:                     %d\n",sidescanheader->Second);
		    fprintf(stderr,"dbg5       HSeconds:                   %d\n",sidescanheader->HSeconds);
		    fprintf(stderr,"dbg5       JulianDay:                  %d\n",sidescanheader->JulianDay);
		    fprintf(stderr,"dbg5       CurrentLineID:              %d\n",sidescanheader->CurrentLineID);
		    fprintf(stderr,"dbg5       EventNumber:                %d\n",sidescanheader->EventNumber);
		    fprintf(stderr,"dbg5       PingNumber:                 %d\n",sidescanheader->PingNumber);
		    fprintf(stderr,"dbg5       SoundVelocity:              %f\n",sidescanheader->SoundVelocity);
		    fprintf(stderr,"dbg5       OceanTide:                  %f\n",sidescanheader->OceanTide);
		    fprintf(stderr,"dbg5       Reserved2:                  %d\n",sidescanheader->Reserved2);
		    fprintf(stderr,"dbg5       ConductivityFreq:           %f\n",sidescanheader->ConductivityFreq);
		    fprintf(stderr,"dbg5       TemperatureFreq:            %f\n",sidescanheader->TemperatureFreq);
		    fprintf(stderr,"dbg5       PressureFreq:               %f\n",sidescanheader->PressureFreq);
		    fprintf(stderr,"dbg5       PressureTemp:               %f\n",sidescanheader->PressureTemp);
		    fprintf(stderr,"dbg5       Conductivity:               %f\n",sidescanheader->Conductivity);
		    fprintf(stderr,"dbg5       WaterTemperature:           %f\n",sidescanheader->WaterTemperature);
		    fprintf(stderr,"dbg5       Pressure:                   %f\n",sidescanheader->Pressure);
		    fprintf(stderr,"dbg5       ComputedSoundVelocity:      %f\n",sidescanheader->ComputedSoundVelocity);
		    fprintf(stderr,"dbg5       MagX:                       %f\n",sidescanheader->MagX);
		    fprintf(stderr,"dbg5       MagY:                       %f\n",sidescanheader->MagY);
		    fprintf(stderr,"dbg5       MagZ:                       %f\n",sidescanheader->MagZ);
		    fprintf(stderr,"dbg5       AuxVal1:                    %f\n",sidescanheader->AuxVal1);
		    fprintf(stderr,"dbg5       AuxVal2:                    %f\n",sidescanheader->AuxVal2);
		    fprintf(stderr,"dbg5       AuxVal3:                    %f\n",sidescanheader->AuxVal3);
		    fprintf(stderr,"dbg5       AuxVal4:                    %f\n",sidescanheader->AuxVal4);
		    fprintf(stderr,"dbg5       AuxVal5:                    %f\n",sidescanheader->AuxVal5);
		    fprintf(stderr,"dbg5       AuxVal6:                    %f\n",sidescanheader->AuxVal6);
		    fprintf(stderr,"dbg5       SpeedLog:                   %f\n",sidescanheader->SpeedLog);
		    fprintf(stderr,"dbg5       Turbidity:                  %f\n",sidescanheader->Turbidity);
		    fprintf(stderr,"dbg5       ShipSpeed:                  %f\n",sidescanheader->ShipSpeed);
		    fprintf(stderr,"dbg5       ShipGyro:                   %f\n",sidescanheader->ShipGyro);
		    fprintf(stderr,"dbg5       ShipYcoordinate:            %f\n",sidescanheader->ShipYcoordinate);
		    fprintf(stderr,"dbg5       ShipXcoordinate:            %f\n",sidescanheader->ShipXcoordinate);
		    fprintf(stderr,"dbg5       ShipAltitude:               %d\n",sidescanheader->ShipAltitude);
		    fprintf(stderr,"dbg5       ShipDepth:                  %d\n",sidescanheader->ShipDepth);
		    fprintf(stderr,"dbg5       FixTimeHour:                %d\n",sidescanheader->FixTimeHour);
		    fprintf(stderr,"dbg5       FixTimeMinute:              %d\n",sidescanheader->FixTimeMinute);
		    fprintf(stderr,"dbg5       FixTimeSecond:              %d\n",sidescanheader->FixTimeSecond);
		    fprintf(stderr,"dbg5       Reserved4:                  %d\n",sidescanheader->Reserved4);
		    fprintf(stderr,"dbg5       SensorSpeed:                %f\n",sidescanheader->SensorSpeed);
		    fprintf(stderr,"dbg5       KP:                         %f\n",sidescanheader->KP);
		    fprintf(stderr,"dbg5       SensorYcoordinate:          %f\n",sidescanheader->SensorYcoordinate);
		    fprintf(stderr,"dbg5       SensorXcoordinate:          %f\n",sidescanheader->SensorXcoordinate);
		    fprintf(stderr,"dbg5       Reserved6:                  %d\n",sidescanheader->Reserved6);
		    fprintf(stderr,"dbg5       RangeToSensor:              %d\n",sidescanheader->RangeToSensor);
		    fprintf(stderr,"dbg5       BearingToSensor:            %d\n",sidescanheader->BearingToSensor);
		    fprintf(stderr,"dbg5       CableOut:                   %d\n",sidescanheader->CableOut);
		    fprintf(stderr,"dbg5       Layback:                    %f\n",sidescanheader->Layback);
		    fprintf(stderr,"dbg5       CableTension:               %f\n",sidescanheader->CableTension);
		    fprintf(stderr,"dbg5       SensorDepth:                %f\n",sidescanheader->SensorDepth);
		    fprintf(stderr,"dbg5       SensorPrimaryAltitude:      %f\n",sidescanheader->SensorPrimaryAltitude);
		    fprintf(stderr,"dbg5       SensorAuxAltitude:          %f\n",sidescanheader->SensorAuxAltitude);
		    fprintf(stderr,"dbg5       SensorPitch:                %f\n",sidescanheader->SensorPitch);
		    fprintf(stderr,"dbg5       SensorRoll:                 %f\n",sidescanheader->SensorRoll);
		    fprintf(stderr,"dbg5       SensorHeading:              %f\n",sidescanheader->SensorHeading);
		    fprintf(stderr,"dbg5       Heave:                      %f\n",sidescanheader->Heave);
		    fprintf(stderr,"dbg5       Yaw:                        %f\n",sidescanheader->Yaw);
		    fprintf(stderr,"dbg5       AttitudeTimeTag:            %d\n",sidescanheader->AttitudeTimeTag);
		    fprintf(stderr,"dbg5       DOT:                        %f\n",sidescanheader->DOT);
		    for (i=0;i<20;i++)
			fprintf(stderr,"dbg5       ReservedSpace[%2.2d]:          %d\n",i,sidescanheader->ReservedSpace[i]);
		    fprintf(stderr,"dbg5       ChannelNumber:              %d\n",pingchanportheader->ChannelNumber);
		    fprintf(stderr,"dbg5       DownsampleMethod:           %d\n",pingchanportheader->DownsampleMethod);
		    fprintf(stderr,"dbg5       SlantRange:                 %f\n",pingchanportheader->SlantRange);
		    fprintf(stderr,"dbg5       GroundRange:                %f\n",pingchanportheader->GroundRange);
		    fprintf(stderr,"dbg5       TimeDelay:                  %f\n",pingchanportheader->TimeDelay);
		    fprintf(stderr,"dbg5       TimeDuration:               %f\n",pingchanportheader->TimeDuration);
		    fprintf(stderr,"dbg5       SecondsPerPing:             %f\n",pingchanportheader->SecondsPerPing);
		    fprintf(stderr,"dbg5       ProcessingFlags:            %d\n",pingchanportheader->ProcessingFlags);
		    fprintf(stderr,"dbg5       Frequency:                  %d\n",pingchanportheader->Frequency);
		    fprintf(stderr,"dbg5       InitialGainCode:            %d\n",pingchanportheader->InitialGainCode);
		    fprintf(stderr,"dbg5       GainCode:                   %d\n",pingchanportheader->GainCode);
		    fprintf(stderr,"dbg5       BandWidth:                  %d\n",pingchanportheader->BandWidth);
		    fprintf(stderr,"dbg5       ContactNumber:              %d\n",pingchanportheader->ContactNumber);
		    fprintf(stderr,"dbg5       ContactClassification:      %d\n",pingchanportheader->ContactClassification);
		    fprintf(stderr,"dbg5       ContactSubNumber:           %d\n",pingchanportheader->ContactSubNumber);
		    fprintf(stderr,"dbg5       ContactType:                %d\n",pingchanportheader->ContactType);
		    fprintf(stderr,"dbg5       NumSamples:                 %d\n",pingchanportheader->NumSamples);
		    fprintf(stderr,"dbg5       Reserved:                   %d\n",pingchanportheader->Reserved);
		    fprintf(stderr,"dbg5       ContactTimeOffTrack:        %f\n",pingchanportheader->ContactTimeOffTrack);
		    fprintf(stderr,"dbg5       ContactCloseNumber:         %d\n",pingchanportheader->ContactCloseNumber);
		    fprintf(stderr,"dbg5       Reserved2:                  %d\n",pingchanportheader->Reserved2);
		    fprintf(stderr,"dbg5       FixedVSOP:                  %f\n",pingchanportheader->FixedVSOP);
		    for (i=0;i<6;i++)
			fprintf(stderr,"dbg5       ReservedSpace[%2.2d]:          %d\n",i,pingchanportheader->ReservedSpace[i]);
		    fprintf(stderr,"dbg5       ChannelNumber:              %d\n",pingchanstbdheader->ChannelNumber);
		    fprintf(stderr,"dbg5       DownsampleMethod:           %d\n",pingchanstbdheader->DownsampleMethod);
		    fprintf(stderr,"dbg5       SlantRange:                 %f\n",pingchanstbdheader->SlantRange);
		    fprintf(stderr,"dbg5       GroundRange:                %f\n",pingchanstbdheader->GroundRange);
		    fprintf(stderr,"dbg5       TimeDelay:                  %f\n",pingchanstbdheader->TimeDelay);
		    fprintf(stderr,"dbg5       TimeDuration:               %f\n",pingchanstbdheader->TimeDuration);
		    fprintf(stderr,"dbg5       SecondsPerPing:             %f\n",pingchanstbdheader->SecondsPerPing);
		    fprintf(stderr,"dbg5       ProcessingFlags:            %d\n",pingchanstbdheader->ProcessingFlags);
		    fprintf(stderr,"dbg5       Frequency:                  %d\n",pingchanstbdheader->Frequency);
		    fprintf(stderr,"dbg5       InitialGainCode:            %d\n",pingchanstbdheader->InitialGainCode);
		    fprintf(stderr,"dbg5       GainCode:                   %d\n",pingchanstbdheader->GainCode);
		    fprintf(stderr,"dbg5       BandWidth:                  %d\n",pingchanstbdheader->BandWidth);
		    fprintf(stderr,"dbg5       ContactNumber:              %d\n",pingchanstbdheader->ContactNumber);
		    fprintf(stderr,"dbg5       ContactClassification:      %d\n",pingchanstbdheader->ContactClassification);
		    fprintf(stderr,"dbg5       ContactSubNumber:           %d\n",pingchanstbdheader->ContactSubNumber);
		    fprintf(stderr,"dbg5       ContactType:                %d\n",pingchanstbdheader->ContactType);
		    fprintf(stderr,"dbg5       NumSamples:                 %d\n",pingchanstbdheader->NumSamples);
		    fprintf(stderr,"dbg5       Reserved:                   %d\n",pingchanstbdheader->Reserved);
		    fprintf(stderr,"dbg5       ContactTimeOffTrack:        %f\n",pingchanstbdheader->ContactTimeOffTrack);
		    fprintf(stderr,"dbg5       ContactCloseNumber:         %d\n",pingchanstbdheader->ContactCloseNumber);
		    fprintf(stderr,"dbg5       Reserved2:                  %d\n",pingchanstbdheader->Reserved2);
		    fprintf(stderr,"dbg5       FixedVSOP:                  %f\n",pingchanstbdheader->FixedVSOP);
		    for (i=0;i<6;i++)
			fprintf(stderr,"dbg5       ReservedSpace[%2.2d]:          %d\n",i,pingchanstbdheader->ReservedSpace[i]);
		    for (i=0;i<MAX(pingchanportheader->NumSamples,pingchanstbdheader->NumSamples);i++)
			fprintf(stderr,"dbg5       sidescan[%4.4d]: %d %d\n",i,data->ssrawport[i], data->ssrawstbd[i]);
		    }
		}
		
	    /* read rest of bathymetry packet */
	    else if (status == MB_SUCCESS
		&& packetheader.HeaderType == XTF_DATA_BATHYMETRY)
		{
#ifdef MBR_XTFR8101_DEBUG
fprintf(stderr,"Reading bathymetry packet type:%d bytes:%d\n",
packetheader.HeaderType,packetheader.NumBytesThisRecord);
#endif
		data->kind = MB_DATA_DATA;
		bathheader->packetheader = packetheader;
		read_len = fread(line,1,242,mb_io_ptr->mbfp);
		if (read_len == 242)
		    {
		    /* parse the rest of the bathymetry header */
		    index = 0;

		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(bathheader->Year)); 
		    index += 2;
		    bathheader->Month = line[index];
		    index++;
		    bathheader->Day = line[index];
		    index++;
		    bathheader->Hour = line[index];
		    index++;
		    bathheader->Minute = line[index];
		    index++;
		    bathheader->Second = line[index];
		    index++;
		    bathheader->HSeconds = line[index];
		    index++;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(bathheader->JulianDay)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(bathheader->CurrentLineID)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(bathheader->EventNumber)); 
		    index += 2;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(bathheader->PingNumber)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->SoundVelocity)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->OceanTide)); 
		    index += 4;
		    mb_get_binary_int(MB_YES, &line[index], (int *)&(bathheader->Reserved2)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->ConductivityFreq)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->TemperatureFreq)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->PressureFreq)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->PressureTemp)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->Conductivity)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->WaterTemperature)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->Pressure)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->ComputedSoundVelocity)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->MagX)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->MagY)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->MagZ)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->AuxVal1)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->AuxVal2)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->AuxVal3)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->AuxVal4)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->AuxVal5)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->AuxVal6)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->SpeedLog)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->Turbidity)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->ShipSpeed)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->ShipGyro)); 
		    index += 4;
		    mb_get_binary_double(MB_YES, &line[index], &(bathheader->ShipYcoordinate)); 
		    index += 8;
		    mb_get_binary_double(MB_YES, &line[index], &(bathheader->ShipXcoordinate)); 
		    index += 8;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(bathheader->ShipAltitude)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], (short int *)&(bathheader->ShipDepth)); 
		    index += 2;
		    bathheader->FixTimeHour = line[index];
		    index++;
		    bathheader->FixTimeMinute = line[index];
		    index++;
		    bathheader->FixTimeSecond = line[index];
		    index++;
		    bathheader->Reserved4 = line[index];
		    index++;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->SensorSpeed)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->KP)); 
		    index += 4;
		    mb_get_binary_double(MB_YES, &line[index], &(bathheader->SensorYcoordinate)); 
		    index += 8;
		    mb_get_binary_double(MB_YES, &line[index], &(bathheader->SensorXcoordinate)); 
		    index += 8;
		    mb_get_binary_short(MB_YES, &line[index], &(bathheader->Reserved6)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], &(bathheader->RangeToSensor)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], &(bathheader->BearingToSensor)); 
		    index += 2;
		    mb_get_binary_short(MB_YES, &line[index], &(bathheader->CableOut)); 
		    index += 2;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->Layback)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->CableTension)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->SensorDepth)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->SensorPrimaryAltitude)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->SensorAuxAltitude)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->SensorPitch)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->SensorRoll)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->SensorHeading)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->Heave)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->Yaw)); 
		    index += 4;
		    mb_get_binary_int(MB_YES, &line[index], &(bathheader->AttitudeTimeTag)); 
		    index += 4;
		    mb_get_binary_float(MB_YES, &line[index], &(bathheader->DOT)); 
		    index += 4;
		    for (i=0;i<20;i++)
			{
			bathheader->ReservedSpace[i] = line[index];
			index++;
			}

		    /* read rest of record */
		    read_len = fread(line,1,bathheader->packetheader.NumBytesThisRecord-242-14,mb_io_ptr->mbfp);
		    if (read_len == bathheader->packetheader.NumBytesThisRecord-242-14)
			{
			/* check synch value */
			mb_get_binary_int(MB_YES, &line[0], (int *)&(synch));
			if (synch != 65535)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    done = MB_YES;			    
			    }

			/* handle RESON_PACKETID_RT_VERY_OLD */
			else if (line[4] == 0x13)
			    {
			    index = 0;
			    for (i=0;i<4;i++)
				{
				reson8100rit->synch_header[i] = line[index];
				index++;
				}
			    reson8100rit->packet_type = line[index];
			    index++;
			    reson8100rit->packet_subtype = line[index];
			    index++;
			    mb_get_binary_int(MB_NO, &line[index], (int *)&(reson8100rit->Seconds)); 
			    index += 4;
			    mb_get_binary_int(MB_NO, &line[index], (int *)&(reson8100rit->Millisecs)); 
			    index += 4;
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->latency)); 
			    index += 2;
			    reson8100rit->ping_number = 0;
			    reson8100rit->sonar_id = 0;
			    reson8100rit->sonar_model = 0;
			    reson8100rit->frequency = 0;
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->velocity)); 
			    index += 2;
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->sample_rate)); 
			    index += 2;
			    reson8100rit->pulse_width = (unsigned short) (mb_u_char) line[index];
			    index++;
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->ping_rate)); 
			    index += 2;
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->range_set)); 
			    index += 2;
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->power)); 
			    index += 2;
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->gain)); 
			    index += 2;
			    index += 2; /* skip projector value */
			    reson8100rit->tvg_spread = (mb_u_char) line[index];
			    index++;
			    reson8100rit->tvg_absorp = (mb_u_char) line[index];
			    index++;
			    reson8100rit->projector_beam_width = 0;
			    reson8100rit->beam_width_num = (mb_u_char) line[index];
			    reson8100rit->beam_width_denom = 10;
			    index++;
			    reson8100rit->projector_angle = 0;
			    reson8100rit->min_range = 0;
			    reson8100rit->max_range = 0;
			    reson8100rit->min_depth = 0;
			    reson8100rit->max_depth = 0;
			    reson8100rit->filters_active = 0;
			    reson8100rit->spare[0] = 0;
			    reson8100rit->spare[1] = 0;
			    reson8100rit->spare[2] = 0;
			    reson8100rit->temperature = 0;
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->beam_count)); 
			    index += 2;
			    for (i=0;i<reson8100rit->beam_count;i++)
				{
				mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->range[i])); 
				reson8100rit->intensity[i] = 0;
				index += 2;
				}
			    for (i=0;i<reson8100rit->beam_count/2+reson8100rit->beam_count%2;i++)
				{
				reson8100rit->quality[i] = (mb_u_char) line[index]; 
				index++;
				}
			    
			    }

			/* handle RESON_PACKETID_RIT */
			else if (line[4] == 0x18)
			    {
			    index = 0;
			    for (i=0;i<4;i++)
				{
				reson8100rit->synch_header[i] = line[index];
				index++;
				}
			    reson8100rit->packet_type = line[index];
			    index++;
			    reson8100rit->packet_subtype = line[index];
			    index++;
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->latency)); 
			    index += 2;
			    mb_get_binary_int(MB_NO, &line[index], (int *)&(reson8100rit->Seconds)); 
			    index += 4;
			    mb_get_binary_int(MB_NO, &line[index], (int *)&(reson8100rit->Millisecs)); 
			    index += 4;    
			    
			    mb_get_binary_int(MB_NO, &line[index], (int *)&(reson8100rit->ping_number)); 
			    index += 4;

			    mb_get_binary_int(MB_NO, &line[index], (int *)&(reson8100rit->sonar_id)); 
			    index += 4;
			    
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->sonar_model)); 
			    index += 2;
			    
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->frequency)); 
			    index += 2;
			    		    			   
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->velocity)); 
			    index += 2;
			    
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->sample_rate)); 
			    index += 2;
			    
			    
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->ping_rate)); 
			    index += 2;
			  		    
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->range_set)); 
			    index += 2;
			    
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->power)); 
			    index += 2;
			    
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->gain)); 
			    index += 2;			    
			    
			     mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->pulse_width)); 
			    index += 2;
			    			    
			    reson8100rit->tvg_spread = (mb_u_char) line[index];
			    index++;	
			    	    
			    reson8100rit->tvg_absorp = (mb_u_char) line[index];
			    index++;	
			    				
			    reson8100rit->projector_type = (mb_u_char) line[index];
			    index++;				    
			        
			    reson8100rit->projector_beam_width = (mb_u_char) line[index];
			    index++;

			     mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->beam_width_num)); 
			    index += 2;

			     mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->beam_width_denom)); 
			    index += 2;
			    
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->projector_angle)); 
			    index += 2;		
			    
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->min_range)); 
			    index += 2;
			    
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->max_range)); 
			    index += 2;
			    
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->min_depth)); 
			    index += 2;
			    
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->max_depth)); 
			    index += 2;
			    
			    reson8100rit->filters_active = (mb_u_char) line[index];
			    index++;
			    
			    reson8100rit->spare[0] = (mb_u_char) line[index];
			    index++;
			    
			    reson8100rit->spare[1] = (mb_u_char) line[index];
			    index++;
			    
			    reson8100rit->spare[2] = (mb_u_char) line[index];
			    index++;
			    
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->temperature)); 
			    index += 2;
			    
			    mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->beam_count)); 
			    index += 2;			    	    

			    for (i=0;i<reson8100rit->beam_count;i++)
				{
				mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->range[i])); 
				index += 2;
				}
			    for (i=0;i<reson8100rit->beam_count/2+reson8100rit->beam_count%2;i++)
				{
				reson8100rit->quality[i] = (mb_u_char) line[index]; 
				index++;
				}
			    for (i=0;i<reson8100rit->beam_count;i++)
				{
				mb_get_binary_short(MB_NO, &line[index], (short int *)&(reson8100rit->intensity[i])); 
				index += 2;
			        }
			    
			    }
			else
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    done = MB_YES;			    
			    }
			}
		    else
		    	{
		    	status = MB_FAILURE;
		    	*error = MB_ERROR_EOF;
		    	done = MB_YES;
		    	}

		    /* print debug statements */
		    if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg5       MagicNumber:                %d %d %x%x\n",
					bathheader->packetheader.MagicNumber[0],bathheader->packetheader.MagicNumber[1],
					bathheader->packetheader.MagicNumber[0],bathheader->packetheader.MagicNumber[1]);
			fprintf(stderr,"dbg5       HeaderType:                 %d\n",bathheader->packetheader.HeaderType);
			fprintf(stderr,"dbg5       SubChannelNumber:           %d\n",bathheader->packetheader.SubChannelNumber);
			fprintf(stderr,"dbg5       NumChansToFollow:           %d\n",bathheader->packetheader.NumChansToFollow);
			fprintf(stderr,"dbg5       Reserved1:                  %d %d\n",bathheader->packetheader.Reserved1[0],bathheader->packetheader.Reserved1[1]);
			fprintf(stderr,"dbg5       NumBytesThisRecord:         %d\n",bathheader->packetheader.NumBytesThisRecord);
			fprintf(stderr,"dbg5       Year:                       %d\n",bathheader->Year);
			fprintf(stderr,"dbg5       Month:                      %d\n",bathheader->Month);
			fprintf(stderr,"dbg5       Day:                        %d\n",bathheader->Day);
			fprintf(stderr,"dbg5       Hour:                       %d\n",bathheader->Hour);
			fprintf(stderr,"dbg5       Minute:                     %d\n",bathheader->Minute);
			fprintf(stderr,"dbg5       Second:                     %d\n",bathheader->Second);
			fprintf(stderr,"dbg5       HSeconds:                   %d\n",bathheader->HSeconds);
			fprintf(stderr,"dbg5       JulianDay:                  %d\n",bathheader->JulianDay);
			fprintf(stderr,"dbg5       CurrentLineID:              %d\n",bathheader->CurrentLineID);
			fprintf(stderr,"dbg5       EventNumber:                %d\n",bathheader->EventNumber);
			fprintf(stderr,"dbg5       PingNumber:                 %d\n",bathheader->PingNumber);
			fprintf(stderr,"dbg5       SoundVelocity:              %f\n",bathheader->SoundVelocity);
			fprintf(stderr,"dbg5       OceanTide:                  %f\n",bathheader->OceanTide);
			fprintf(stderr,"dbg5       Reserved2:                  %d\n",bathheader->Reserved2);
			fprintf(stderr,"dbg5       ConductivityFreq:           %f\n",bathheader->ConductivityFreq);
			fprintf(stderr,"dbg5       TemperatureFreq:            %f\n",bathheader->TemperatureFreq);
			fprintf(stderr,"dbg5       PressureFreq:               %f\n",bathheader->PressureFreq);
			fprintf(stderr,"dbg5       PressureTemp:               %f\n",bathheader->PressureTemp);
			fprintf(stderr,"dbg5       Conductivity:               %f\n",bathheader->Conductivity);
			fprintf(stderr,"dbg5       WaterTemperature:           %f\n",bathheader->WaterTemperature);
			fprintf(stderr,"dbg5       Pressure:                   %f\n",bathheader->Pressure);
			fprintf(stderr,"dbg5       ComputedSoundVelocity:      %f\n",bathheader->ComputedSoundVelocity);
			fprintf(stderr,"dbg5       MagX:                       %f\n",bathheader->MagX);
			fprintf(stderr,"dbg5       MagY:                       %f\n",bathheader->MagY);
			fprintf(stderr,"dbg5       MagZ:                       %f\n",bathheader->MagZ);
			fprintf(stderr,"dbg5       AuxVal1:                    %f\n",bathheader->AuxVal1);
			fprintf(stderr,"dbg5       AuxVal2:                    %f\n",bathheader->AuxVal2);
			fprintf(stderr,"dbg5       AuxVal3:                    %f\n",bathheader->AuxVal3);
			fprintf(stderr,"dbg5       AuxVal4:                    %f\n",bathheader->AuxVal4);
			fprintf(stderr,"dbg5       AuxVal5:                    %f\n",bathheader->AuxVal5);
			fprintf(stderr,"dbg5       AuxVal6:                    %f\n",bathheader->AuxVal6);
			fprintf(stderr,"dbg5       SpeedLog:                   %f\n",bathheader->SpeedLog);
			fprintf(stderr,"dbg5       Turbidity:                  %f\n",bathheader->Turbidity);
			fprintf(stderr,"dbg5       ShipSpeed:                  %f\n",bathheader->ShipSpeed);
			fprintf(stderr,"dbg5       ShipGyro:                   %f\n",bathheader->ShipGyro);
			fprintf(stderr,"dbg5       ShipYcoordinate:            %f\n",bathheader->ShipYcoordinate);
			fprintf(stderr,"dbg5       ShipXcoordinate:            %f\n",bathheader->ShipXcoordinate);
			fprintf(stderr,"dbg5       ShipAltitude:               %d\n",bathheader->ShipAltitude);
			fprintf(stderr,"dbg5       ShipDepth:                  %d\n",bathheader->ShipDepth);
			fprintf(stderr,"dbg5       FixTimeHour:                %d\n",bathheader->FixTimeHour);
			fprintf(stderr,"dbg5       FixTimeMinute:              %d\n",bathheader->FixTimeMinute);
			fprintf(stderr,"dbg5       FixTimeSecond:              %d\n",bathheader->FixTimeSecond);
			fprintf(stderr,"dbg5       Reserved4:                  %d\n",bathheader->Reserved4);
			fprintf(stderr,"dbg5       SensorSpeed:                %f\n",bathheader->SensorSpeed);
			fprintf(stderr,"dbg5       KP:                         %f\n",bathheader->KP);
			fprintf(stderr,"dbg5       SensorYcoordinate:          %f\n",bathheader->SensorYcoordinate);
			fprintf(stderr,"dbg5       SensorXcoordinate:          %f\n",bathheader->SensorXcoordinate);
			fprintf(stderr,"dbg5       Reserved6:                  %d\n",bathheader->Reserved6);
			fprintf(stderr,"dbg5       RangeToSensor:              %d\n",bathheader->RangeToSensor);
			fprintf(stderr,"dbg5       BearingToSensor:            %d\n",bathheader->BearingToSensor);
			fprintf(stderr,"dbg5       CableOut:                   %d\n",bathheader->CableOut);
			fprintf(stderr,"dbg5       Layback:                    %f\n",bathheader->Layback);
			fprintf(stderr,"dbg5       CableTension:               %f\n",bathheader->CableTension);
			fprintf(stderr,"dbg5       SensorDepth:                %f\n",bathheader->SensorDepth);
			fprintf(stderr,"dbg5       SensorPrimaryAltitude:      %f\n",bathheader->SensorPrimaryAltitude);
			fprintf(stderr,"dbg5       SensorAuxAltitude:          %f\n",bathheader->SensorAuxAltitude);
			fprintf(stderr,"dbg5       SensorPitch:                %f\n",bathheader->SensorPitch);
			fprintf(stderr,"dbg5       SensorRoll:                 %f\n",bathheader->SensorRoll);
			fprintf(stderr,"dbg5       SensorHeading:              %f\n",bathheader->SensorHeading);
			fprintf(stderr,"dbg5       Heave:                      %f\n",bathheader->Heave);
			fprintf(stderr,"dbg5       Yaw:                        %f\n",bathheader->Yaw);
			fprintf(stderr,"dbg5       AttitudeTimeTag:            %d\n",bathheader->AttitudeTimeTag);
			fprintf(stderr,"dbg5       DOT:                        %f\n",bathheader->DOT);
			for (i=0;i<20;i++)
			    fprintf(stderr,"dbg5       ReservedSpace[%2.2d]:          %d\n",i,bathheader->ReservedSpace[i]);
			fprintf(stderr,"dbg5       synch_header:               %x %x %x %x \n",
				    reson8100rit->synch_header[0], reson8100rit->synch_header[1], 
				    reson8100rit->synch_header[2], reson8100rit->synch_header[3]);
			fprintf(stderr,"dbg5       packet_type:                %d\n",reson8100rit->packet_type);
			fprintf(stderr,"dbg5       packet_subtype:             %d\n",reson8100rit->packet_subtype);
			fprintf(stderr,"dbg5       latency:                    %d\n",reson8100rit->latency);
			fprintf(stderr,"dbg5       Seconds:                    %d\n",reson8100rit->Seconds);
			fprintf(stderr,"dbg5       Millisecs:                  %d\n",reson8100rit->Millisecs);
			fprintf(stderr,"dbg5       ping_number:                %d\n",reson8100rit->ping_number);
			fprintf(stderr,"dbg5       sonar_id:                   %d\n",reson8100rit->sonar_id);
			fprintf(stderr,"dbg5       sonar_model:                %d\n",reson8100rit->sonar_model);
			fprintf(stderr,"dbg5       frequency:                  %d\n",reson8100rit->frequency);
			fprintf(stderr,"dbg5       velocity:                   %d\n",reson8100rit->velocity);
			fprintf(stderr,"dbg5       sample_rate:                %d\n",reson8100rit->sample_rate);
			fprintf(stderr,"dbg5       ping_rate:                  %d\n",reson8100rit->ping_rate);
			fprintf(stderr,"dbg5       range_set:                  %d\n",reson8100rit->range_set);
			fprintf(stderr,"dbg5       power:                      %d\n",reson8100rit->power);
			fprintf(stderr,"dbg5       gain:                       %d\n",reson8100rit->gain);
			fprintf(stderr,"dbg5       tvg_spread:                 %d\n",reson8100rit->tvg_spread);
			fprintf(stderr,"dbg5       tvg_absorp:                 %d\n",reson8100rit->tvg_absorp);
			fprintf(stderr,"dbg5       projector_type:             %d\n",reson8100rit->projector_type);
			fprintf(stderr,"dbg5       projector_beam_width:       %d\n",reson8100rit->projector_beam_width);
			fprintf(stderr,"dbg5       beam_width_num:             %d\n",reson8100rit->beam_width_num);
			fprintf(stderr,"dbg5       beam_width_denom:           %d\n",reson8100rit->beam_width_denom);
			fprintf(stderr,"dbg5       projector_angle:            %d\n",reson8100rit->projector_angle);
			fprintf(stderr,"dbg5       min_range:                  %d\n",reson8100rit->min_range);
			fprintf(stderr,"dbg5       max_range:                  %d\n",reson8100rit->max_range);
			fprintf(stderr,"dbg5       min_depth:                  %d\n",reson8100rit->min_depth);
			fprintf(stderr,"dbg5       max_depth:                  %d\n",reson8100rit->max_depth);
			fprintf(stderr,"dbg5       filters_active:             %d\n",reson8100rit->filters_active);
			fprintf(stderr,"dbg5       spare:                      %d\n",reson8100rit->spare[0]);
			fprintf(stderr,"dbg5       spare:                      %d\n",reson8100rit->spare[1]);
			fprintf(stderr,"dbg5       spare:                      %d\n",reson8100rit->spare[2]);
			fprintf(stderr,"dbg5       temperature:                %d\n",reson8100rit->temperature);
			fprintf(stderr,"dbg5       beam_count:                 %d\n",reson8100rit->beam_count);
			for (i=0;i<reson8100rit->beam_count;i++)
			    {
			    fprintf(stderr,"dbg5       beam[%3.3d]   range:%5.5d",
				    i,reson8100rit->range[i]);
			    if (i % 2 == 0)
				    quality = ((reson8100rit->quality[i/2]) & 15);
			    else
				    quality = ((reson8100rit->quality[i/2] >> 4) & 15);
			    fprintf(stderr,"  quality:%3.3d %d%d%d%d\n",quality,quality & 1, quality >>1 & 1,
						    quality >> 2 & 1, quality >> 3 & 1);
			    }
			}
			
		    /* set success */
		    status = MB_SUCCESS;
		    *error = MB_ERROR_NO_ERROR;
		    done = MB_YES;
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    done = MB_YES;
		    }
		}

	    /* else read rest of unknown packet */
	    else if (status == MB_SUCCESS)
		{
		if (((int)packetheader.NumBytesThisRecord) > 14)
			{
			for (i=0;i<((int)packetheader.NumBytesThisRecord)-14;i++)
				{
				read_len = fread(line,1,1,mb_io_ptr->mbfp);
				}
			if (read_len != 1)
		    		{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				done = MB_YES;
				}
			}
#ifdef MBR_XTFR8101_DEBUG
fprintf(stderr,"Reading unknown packet type:%d bytes:%d\n",
packetheader.HeaderType,packetheader.NumBytesThisRecord);
#endif
		}
	    }
		
	/* get file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
