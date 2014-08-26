/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hsds2raw.c	6/20/01
 *	$Id$
 *
 *    Copyright (c) 2001-2014 by
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
 * mbr_hsds2raw.c contains the functions for reading
 * multibeam data in the HSDS2RAW format.
 * These functions include:
 *   mbr_alm_hsds2raw	- allocate read/write memory
 *   mbr_dem_hsds2raw	- deallocate read/write memory
 *   mbr_rt_hsds2raw	- read and translate data
 *   mbr_wt_hsds2raw	- translate and write data
 *
 * Authors:	D. W. Caress
 * 		D. N. Chayes
 * Date:	June 20, 2001
 * $Log: mbr_hsds2raw.c,v $
 * Revision 5.12  2007/10/08 15:59:34  caress
 * MBIO changes as of 8 October 2007.
 *
 * Revision 5.11  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.10  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.9  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.8  2003/02/27 04:33:33  caress
 * Fixed handling of SURF format data.
 *
 * Revision 5.7  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.6  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.5  2002/04/06 02:43:39  caress
 * Release 5.0.beta16
 *
 * Revision 5.4  2001/12/18 04:27:45  caress
 * Release 5.0.beta11.
 *
 * Revision 5.3  2001/08/10 22:41:19  dcaress
 * Release 5.0.beta07
 *
 * Revision 5.2  2001-07-25 20:40:56-07  caress
 * Fixed handling of sidescan.
 *
 * Revision 5.1  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.0  2001/06/29  22:49:07  caress
 * Added support for HSDS2RAW
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_define.h"
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mbsys_atlas.h"

/* turn on debug statements here */
/* #define MBR_HSDS2RAW_DEBUG 1 */

/* essential function prototypes */
int mbr_register_hsds2raw(int verbose, void *mbio_ptr,
		int *error);
int mbr_info_hsds2raw(int verbose,
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
int mbr_alm_hsds2raw(int verbose, void *mbio_ptr, int *error);
int mbr_dem_hsds2raw(int verbose, void *mbio_ptr, int *error);
int mbr_rt_hsds2raw(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_hsds2raw(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_hsds2raw_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_hsds2raw_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_hsds2raw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_hsds2raw";
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
	status = mbr_info_hsds2raw(verbose,
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_hsds2raw;
	mb_io_ptr->mb_io_format_free = &mbr_dem_hsds2raw;
	mb_io_ptr->mb_io_store_alloc = &mbsys_atlas_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_atlas_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_hsds2raw;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_hsds2raw;
	mb_io_ptr->mb_io_dimensions = &mbsys_atlas_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_atlas_extract;
	mb_io_ptr->mb_io_insert = &mbsys_atlas_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_atlas_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_atlas_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_atlas_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_atlas_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_atlas_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_atlas_copy;
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
		fprintf(stderr,"dbg2       format_alloc:       %p\n",(void *)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %p\n",(void *)mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %p\n",(void *)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %p\n",(void *)mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %p\n",(void *)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %p\n",(void *)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %p\n",(void *)mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %p\n",(void *)mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %p\n",(void *)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %p\n",(void *)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %p\n",(void *)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %p\n",(void *)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %p\n",(void *)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %p\n",(void *)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %p\n",(void *)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       detects:            %p\n",(void *)mb_io_ptr->mb_io_detects);
		fprintf(stderr,"dbg2       extract_rawss:      %p\n",(void *)mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr,"dbg2       insert_rawss:       %p\n",(void *)mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr,"dbg2       copyrecord:         %p\n",(void *)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_hsds2raw(int verbose,
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
	char	*function_name = "mbr_info_hsds2raw";
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
	*system = MB_SYS_ATLAS;
	*beams_bath_max = MBSYS_ATLAS_MAXBEAMS;
	*beams_amp_max = MBSYS_ATLAS_MAXBEAMS;
	*pixels_ss_max = MBSYS_ATLAS_MAXPIXELS;
	strncpy(format_name, "HSDS2RAW", MB_NAME_LENGTH);
	strncpy(system_name, "ATLAS", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_HSDS2RAW\nInformal Description: STN Atlas raw multibeam format\nAttributes:           STN Atlas multibeam sonars, \n                      Hydrosweep DS2, Hydrosweep MD, \n                      Fansweep 10, Fansweep 20, \n                      bathymetry, amplitude, and sidescan,\n                      up to 1440 beams and 4096 pixels,\n                      XDR binary, STN Atlas.\n", MB_DESCRIPTION_LENGTH);
	*numfile = -3;
	*filetype = MB_FILETYPE_XDR;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
	*beamwidth_xtrack = 2.67;
	*beamwidth_ltrack = 2.67;

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
int mbr_alm_hsds2raw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_hsds2raw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	status = mbsys_atlas_alloc(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);

	/* set name for navigation and angle files */
	if (status == MB_SUCCESS)
		{
		if (strlen(mb_io_ptr->file) >= 5
		    && strncmp(&(mb_io_ptr->file[strlen(mb_io_ptr->file)-4]),
				".fsw", 4) == 0)
			{
			strcpy(mb_io_ptr->file2,mb_io_ptr->file);
			strcpy(&(mb_io_ptr->file2[strlen(mb_io_ptr->file)-4]),
				".nav");
			}
		if (strlen(mb_io_ptr->file) >= 5
		    && strncmp(&(mb_io_ptr->file[strlen(mb_io_ptr->file)-4]),
				".fsw", 4) == 0)
			{
			strcpy(mb_io_ptr->file3,mb_io_ptr->file);
			strcpy(&(mb_io_ptr->file3[strlen(mb_io_ptr->file)-4]),
				".ang");
			}
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
int mbr_dem_hsds2raw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_hsds2raw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		}

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mbsys_atlas_deall(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);

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
int mbr_rt_hsds2raw(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_hsds2raw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* read next data from file */
	status = mbr_hsds2raw_rd_data(verbose,mbio_ptr,store_ptr,error);

	/* get pointers to data structures */
	store = (struct mbsys_atlas_struct *) store_ptr;

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

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
int mbr_wt_hsds2raw(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_hsds2raw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_atlas_struct *) store_ptr;

	/* write next data to file */
	/* status = mbr_hsds2raw_wr_data(verbose,mbio_ptr,store_ptr,error); */

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
int mbr_hsds2raw_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_hsds2raw_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;
	int	xdr_status;
	int	read_status;
	int	nskip;
	int	done;
	int	length;
	int	telegram_id;
	int	telegram_cnt;
	char	telegram_send[16];
	char	telegram_recv[16];
	double	telegram_utc_time_d;
	double	telegram_loc_time_d;
	int	telegram_block_no;
	int	telegram_block_cnt;
	int	telegram_max_no;
	int	telegram_act_no;
	int	telegram_data_status;
	int	telegram_sensor_status;
	int	tt_max_lead_cnt;
	int	tt_act_lead_cnt;
	int	ss_act_side_cnt;

	char	carrier_name[8];
	char	task_name[16];
	char	operator_name[32];
	char	gauge_name[32];
	char	comment[32];
	char	profile_name[32];
	int	profile_version;
	double	sys_pos_lat;
	double	sys_pos_lon;
	char	sys_pos_sensor[8];
	double	sys_pos_lat_tpe;
	double	sys_pos_lon_tpe;
	double	sys_pos_time;
	int	sys_pos_data_status;
	int	sys_pos_status;
	double	sys_height;
	char	sys_height_sensor[8];
	double	sys_height_time;
	int	sys_height_data_status;
	int	sys_height_status;
	double	sys_speed_wlong;
	double	sys_speed_wcross;
	char	sys_tw_sensor[8];
	double	sys_tw_time;
	int	sys_tw_data_status;
	int	sys_tw_status;
	double	sys_cog;
	char	sys_cog_sensor[8];
	double	sys_cog_time;
	int	sys_cog_data_status;
	int	sys_cog_status;
	double	sys_sog;
	char	sys_sog_sensor[8];
	double	sys_sog_time;
	int	sys_sog_data_status;
	int	sys_sog_status;
	double	sys_set;
	double	sys_drift;
	char	sys_set_drift_sensor[8];
	double	sys_set_drift_time;
	int	sys_set_drift_data_status;
	int	sys_set_drift_status;
	double	sys_heading;
	char	sys_heading_sensor[8];
	double	sys_heading_time;
	int	sys_heading_data_status;
	int	sys_heading_status;
	double	sys_depth;
	char	sys_depth_sensor[8];
	double	sys_depth_water_level;
	double	sys_depth_time;
	int	sys_depth_data_status;
	int	sys_depth_status;
	double	sys_wspeed_abs;
	double	sys_wdir_abs;
	char	sys_wind_sensor[8];
	double	sys_wind_time;
	int	sys_wind_data_status;
	int	sys_wind_status;

	double	rr;
	double  pspeed;
	double	*angle_table;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_atlas_struct *) store_ptr;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read the next record (start telegram + travel time telegrams
	    + sidescan telegrams +  */
	*error = MB_ERROR_NO_ERROR;

	/* get start telegram */
	xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_id);

	/* telegram id ok - just read send and receive strings */
	if (xdr_status == MB_YES && telegram_id == MBSYS_ATLAS_TELEGRAM_START)
	    {
	    if (xdr_status == MB_YES)
		xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_cnt);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)telegram_send, 16);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)telegram_recv, 16);
	    }

	/* expected telegram id wrong - try to resync on recv string */
	else if (xdr_status == MB_YES)
	    {
	    memset(telegram_recv, 0, 16);
	    read_status = 1;
	    nskip = 0;
	    while (strncmp(telegram_recv, "BROADCAST", 9) != 0
		&& read_status == 1)
		{
		for (i=0;i<15;i++)
		    telegram_recv[i] = telegram_recv[i+1];
		if ((read_status = fread(&(telegram_recv[15]), 1, 1, mb_io_ptr->mbfp)) == 1)
		    nskip++;
		}
	    if (read_status == MB_YES)
		{
		fprintf(stderr, "Resync on START telegram: %d missing bytes\n",
			(44 - nskip));
		}
	    else
		{
		xdr_status = MB_NO;
		}
	    }

	/* hopefully we are synced - read the telegram */
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &telegram_utc_time_d);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &telegram_loc_time_d);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_block_no);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_block_cnt);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_u_int((XDR *)mb_io_ptr->xdrs, &store->start_ping_no);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->start_transmit_time_d);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *) store->start_opmode, 32);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->start_heave);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->start_roll);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->start_pitch);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->start_heading);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->start_ckeel);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->start_cmean);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->start_depth_min);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->start_depth_max);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_data_status);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_sensor_status);

	/* print debug statements */
#ifndef MBR_HSDS2RAW_DEBUG
	if (verbose >= 5)
#endif
		{
		fprintf(stderr,"\ndbg5  Start telegram read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       xdr_status:              %d\n",xdr_status);
		fprintf(stderr,"dbg5       telegram_id:             %d\n",telegram_id);
		fprintf(stderr,"dbg5       telegram_cnt:            %d\n",telegram_cnt);
		fprintf(stderr,"dbg5       telegram_send:           %s\n",telegram_send);
		fprintf(stderr,"dbg5       telegram_recv:           %s\n",telegram_recv);
		fprintf(stderr,"dbg5       telegram_utc_time_d:     %f\n",telegram_utc_time_d);
		fprintf(stderr,"dbg5       telegram_loc_time_d:     %f\n",telegram_utc_time_d);
		fprintf(stderr,"dbg5       telegram_block_no:       %d\n",telegram_block_no);
		fprintf(stderr,"dbg5       telegram_block_cnt:      %d\n",telegram_block_cnt);
		fprintf(stderr,"dbg5       start_ping_no:           %d\n",store->start_ping_no);
		fprintf(stderr,"dbg5       start_transmit_time_d:   %f\n",store->start_transmit_time_d);
		fprintf(stderr,"dbg5       start_opmode:            ");
		for (i=0;i<32;i++)
		    fprintf(stderr," %d",store->start_opmode[i]);
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       start_heave:             %f\n",store->start_heave);
		fprintf(stderr,"dbg5       start_roll:              %f\n",store->start_roll);
		fprintf(stderr,"dbg5       start_pitch:             %f\n",store->start_pitch);
		fprintf(stderr,"dbg5       start_heading:           %f\n",store->start_heading);
		fprintf(stderr,"dbg5       start_ckeel:             %f\n",store->start_ckeel);
		fprintf(stderr,"dbg5       start_cmean:             %f\n",store->start_cmean);
		fprintf(stderr,"dbg5       start_depth_min:         %f\n",store->start_depth_min);
		fprintf(stderr,"dbg5       start_depth_max:         %f\n",store->start_depth_max);
		}

	/* get travel times telegrams */
	done = MB_NO;
	store->tt_beam_cnt = 0;
	while (xdr_status == MB_YES && done == MB_NO)
		{
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_id);

		/* telegram id ok - just read send and receive strings */
		if (xdr_status == MB_YES && telegram_id == MBSYS_ATLAS_TELEGRAM_TRAVELTIMES)
		    {
		    if (xdr_status == MB_YES)
			xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_cnt);
		    if (xdr_status == MB_YES)
			xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
		    if (xdr_status == MB_YES)
			xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)telegram_send, 16);
		    if (xdr_status == MB_YES)
			xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
		    if (xdr_status == MB_YES)
			xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)telegram_recv, 16);
		    }

		/* expected telegram id wrong - try to resync on recv string */
		else if (xdr_status == MB_YES)
		    {
		    memset(telegram_recv, 0, 16);
		    read_status = 1;
		    nskip = 0;
		    while (strncmp(telegram_recv, "BROADCAST", 9) != 0
			&& read_status == 1)
			{
			for (i=0;i<15;i++)
			    telegram_recv[i] = telegram_recv[i+1];
			if ((read_status = fread(&(telegram_recv[15]), 1, 1, mb_io_ptr->mbfp)) == 1)
			    nskip++;
			}
		    if (read_status == MB_YES)
			{
			fprintf(stderr, "Resync on TRAVELTIMES telegram: %d missing bytes\n",
				(44 - nskip));
			}
		    else
			{
			xdr_status = MB_NO;
			}
		    }

		/* hopefully we are synced - read the telegram */
		if (xdr_status == MB_YES)
		    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &telegram_utc_time_d);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &telegram_loc_time_d);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_block_no);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_block_cnt);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_max_no);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_act_no);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_data_status);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_sensor_status);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_u_int((XDR *)mb_io_ptr->xdrs, &store->tt_ping_no);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->tt_transmit_time_d);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &store->tt_beam_table_index);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &tt_max_lead_cnt);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &tt_act_lead_cnt);
		store->tt_beam_cnt += tt_act_lead_cnt;
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &store->tt_long1);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &store->tt_long2);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &store->tt_long3);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &store->tt_xdraught);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->tt_double1);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->tt_double2);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->tt_sensdraught);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->tt_draught);
		for (i=0;i<MBSYS_ATLAS_MAXBEAMTELEGRAM;i++)
		    {
		    if (xdr_status == MB_YES)
			xdr_status = xdr_float((XDR *)mb_io_ptr->xdrs, &(store->tt_lruntime[i]));
		    }
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)store->tt_lamplitude,
						MBSYS_ATLAS_MAXBEAMTELEGRAM);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)store->tt_lstatus,
						MBSYS_ATLAS_MAXBEAMTELEGRAM);

		/* set done if done */
		if (xdr_status != MB_YES
		    || telegram_act_no == telegram_max_no)
		    done = MB_YES;

		/* print debug statements */
#ifndef MBR_HSDS2RAW_DEBUG
		if (verbose >= 5)
#endif
			{
			fprintf(stderr,"\ndbg5  Travel time telegrams read in MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg5       xdr_status:              %d\n",xdr_status);
			fprintf(stderr,"dbg5       telegram_id:             %d\n",telegram_id);
			fprintf(stderr,"dbg5       telegram_cnt:            %d\n",telegram_cnt);
			fprintf(stderr,"dbg5       telegram_send:           %s\n",telegram_send);
			fprintf(stderr,"dbg5       telegram_recv:           %s\n",telegram_recv);
			fprintf(stderr,"dbg5       telegram_utc_time_d:     %f\n",telegram_utc_time_d);
			fprintf(stderr,"dbg5       telegram_loc_time_d:     %f\n",telegram_utc_time_d);
			fprintf(stderr,"dbg5       telegram_block_no:       %d\n",telegram_block_no);
			fprintf(stderr,"dbg5       telegram_block_cnt:      %d\n",telegram_block_cnt);
			fprintf(stderr,"dbg5       telegram_max_no:         %d\n",telegram_max_no);
			fprintf(stderr,"dbg5       telegram_act_no:         %d\n",telegram_act_no);
			fprintf(stderr,"dbg5       telegram_data_status:    %d\n",telegram_data_status);
			fprintf(stderr,"dbg5       telegram_sensor_status:  %d\n",telegram_sensor_status);
			}
		}

	/* print debug statements */
#ifndef MBR_HSDS2RAW_DEBUG
	if (verbose >= 5)
#endif
		{
		fprintf(stderr,"\ndbg5  Travel time telegrams read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       xdr_status:              %d\n",xdr_status);
		fprintf(stderr,"dbg5       tt_ping_no:              %d\n",store->tt_ping_no);
		fprintf(stderr,"dbg5       tt_transmit_time_d:      %f\n",store->tt_transmit_time_d);
		fprintf(stderr,"dbg5       tt_beam_table_index:     %d\n",store->tt_beam_table_index);
		fprintf(stderr,"dbg5       tt_beam_cnt:             %d\n",store->tt_beam_cnt);
		fprintf(stderr,"dbg5       tt_long1:                %d\n",store->tt_long1);
		fprintf(stderr,"dbg5       tt_long2:                %d\n",store->tt_long2);
		fprintf(stderr,"dbg5       tt_long3:                %d\n",store->tt_long3);
		fprintf(stderr,"dbg5       tt_xdraught:             %d\n",store->tt_xdraught);
		fprintf(stderr,"dbg5       tt_double1:              %f\n",store->tt_double1);
		fprintf(stderr,"dbg5       tt_double2:              %f\n",store->tt_double2);
		fprintf(stderr,"dbg5       tt_sensdraught:          %f\n",store->tt_sensdraught);
		fprintf(stderr,"dbg5       tt_draught:              %f\n",store->tt_draught);
		if (verbose > 0)
		for (i=0;i<MBSYS_ATLAS_MAXBEAMS;i++)
			fprintf(stderr,"dbg5       beam[%d] tt amp stat:    %12f %3d %3d\n",
				i, store->tt_lruntime[i], store->tt_lamplitude[i], store->tt_lstatus[i]);
		}

	/* get sidescan telegrams */
	done = MB_NO;
	while (xdr_status == MB_YES && done == MB_NO)
		{
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_id);

		/* telegram id ok - just read send and receive strings */
		if (xdr_status == MB_YES && telegram_id == MBSYS_ATLAS_TELEGRAM_SIDESCAN)
		    {
		    if (xdr_status == MB_YES)
			xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_cnt);
		    if (xdr_status == MB_YES)
			xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
		    if (xdr_status == MB_YES)
			xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)telegram_send, 16);
		    if (xdr_status == MB_YES)
			xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
		    if (xdr_status == MB_YES)
			xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)telegram_recv, 16);
		    }

		/* expected telegram id wrong - try to resync on recv string */
		else if (xdr_status == MB_YES)
		    {
		    memset(telegram_recv, 0, 16);
		    read_status = 1;
		    nskip = 0;
		    while (strncmp(telegram_recv, "BROADCAST", 9) != 0
			&& read_status == 1)
			{
			for (i=0;i<15;i++)
			    telegram_recv[i] = telegram_recv[i+1];
			if ((read_status = fread(&(telegram_recv[15]), 1, 1, mb_io_ptr->mbfp)) == 1)
			    nskip++;
			}
		    if (read_status == MB_YES)
			{
			fprintf(stderr, "Resync on SIDESCAN telegram: %d missing bytes\n",
				(44 - nskip));
			}
		    else
			{
			xdr_status = MB_NO;
			}
		    }

		/* hopefully we are synced - read the telegram */
		if (xdr_status == MB_YES)
		    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &telegram_utc_time_d);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &telegram_loc_time_d);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_block_no);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_block_cnt);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_max_no);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_act_no);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_data_status);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_sensor_status);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_u_int((XDR *)mb_io_ptr->xdrs, &store->ss_ping_no);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->ss_transmit_time_d);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->ss_timedelay);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->ss_timespacing);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &store->ss_max_side_bb_cnt);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &store->ss_max_side_sb_cnt);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &ss_act_side_cnt);
		if (xdr_status == MB_YES)
		    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
		if (xdr_status == MB_YES)
			{
			if (telegram_act_no * MBSYS_ATLAS_MAXPIXELTELEGRAM <= MBSYS_ATLAS_MAXPIXELS)
			    i = (telegram_act_no - 1) * MBSYS_ATLAS_MAXPIXELTELEGRAM;
			else
			    i = 0;
			xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)&(store->ss_sidescan[i]),
						MBSYS_ATLAS_MAXPIXELTELEGRAM);
			}

		/* set done if done */
		if (xdr_status != MB_YES
		    || telegram_act_no == telegram_max_no)
		    done = MB_YES;

		/* print debug statements */
#ifndef MBR_HSDS2RAW_DEBUG
		if (verbose >= 5)
#endif
			{
			fprintf(stderr,"\ndbg5  Sidescan telegram read in MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg5       xdr_status:              %d\n",xdr_status);
			fprintf(stderr,"dbg5       telegram_id:             %d\n",telegram_id);
			fprintf(stderr,"dbg5       telegram_cnt:            %d\n",telegram_cnt);
			fprintf(stderr,"dbg5       telegram_send:           %s\n",telegram_send);
			fprintf(stderr,"dbg5       telegram_recv:           %s\n",telegram_recv);
			fprintf(stderr,"dbg5       telegram_utc_time_d:     %f\n",telegram_utc_time_d);
			fprintf(stderr,"dbg5       telegram_loc_time_d:     %f\n",telegram_utc_time_d);
			fprintf(stderr,"dbg5       telegram_block_no:       %d\n",telegram_block_no);
			fprintf(stderr,"dbg5       telegram_block_cnt:      %d\n",telegram_block_cnt);
			fprintf(stderr,"dbg5       telegram_max_no:         %d\n",telegram_max_no);
			fprintf(stderr,"dbg5       telegram_act_no:         %d\n",telegram_act_no);
			fprintf(stderr,"dbg5       telegram_data_status:    %d\n",telegram_data_status);
			fprintf(stderr,"dbg5       telegram_sensor_status:  %d\n",telegram_sensor_status);
			fprintf(stderr,"dbg5       ss_ping_no:              %d\n",store->ss_ping_no);
			fprintf(stderr,"dbg5       ss_transmit_time_d:      %f\n",store->ss_transmit_time_d);
			}
		}

	/* print debug statements */
#ifndef MBR_HSDS2RAW_DEBUG
	if (verbose >= 5)
#endif
		{
		fprintf(stderr,"\ndbg5  Sidescan telegrams read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       xdr_status:              %d\n",xdr_status);
		fprintf(stderr,"dbg5       ss_ping_no:              %d\n",store->ss_ping_no);
		fprintf(stderr,"dbg5       ss_transmit_time_d:      %f\n",store->ss_transmit_time_d);
		fprintf(stderr,"dbg5       ss_timedelay:            %f\n",store->ss_timedelay);
		fprintf(stderr,"dbg5       ss_timespacing:          %f\n",store->ss_timespacing);
		fprintf(stderr,"dbg5       ss_max_side_bb_cnt:      %d\n",store->ss_max_side_bb_cnt);
		fprintf(stderr,"dbg5       ss_max_side_sb_cnt:      %d\n",store->ss_max_side_sb_cnt);
		if (verbose > 0)
		for (i=0;i<MBSYS_ATLAS_MAXPIXELS;i++)
			fprintf(stderr,"dbg5       pixel[%d] ss:            %d\n",i, store->ss_sidescan[i]);
		}

	/* get tracking window telegram */
	xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_id);

	/* telegram id ok - just read send and receive strings */
	if (xdr_status == MB_YES && telegram_id == MBSYS_ATLAS_TELEGRAM_TRACKINGWINDOWS)
	    {
	    if (xdr_status == MB_YES)
		xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_cnt);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)telegram_send, 16);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)telegram_recv, 16);
	    }

	/* expected telegram id wrong - try to resync on recv string */
	else if (xdr_status == MB_YES)
	    {
	    memset(telegram_recv, 0, 16);
	    read_status = 1;
	    nskip = 0;
	    while (strncmp(telegram_recv, "BROADCAST", 9) != 0
		&& read_status == 1)
		{
		for (i=0;i<15;i++)
		    telegram_recv[i] = telegram_recv[i+1];
		if ((read_status = fread(&(telegram_recv[15]), 1, 1, mb_io_ptr->mbfp)) == 1)
		    nskip++;
		}
	    if (read_status == MB_YES)
		{
		fprintf(stderr, "Resync on TRACKINGWINDOWS telegram: %d missing bytes\n",
			(44 - nskip));
		}
	    else
		{
		xdr_status = MB_NO;
		}
	    }

	/* hopefully we are synced - read the telegram */
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &telegram_utc_time_d);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &telegram_loc_time_d);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_block_no);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_block_cnt);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_data_status);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_sensor_status);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->tr_transmit_time_d);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_u_int((XDR *)mb_io_ptr->xdrs, &store->tr_ping_no);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &store->tr_window_mode);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &store->tr_no_of_win_groups);
	for (i=0;i<100;i++)
	    {
	    if (xdr_status == MB_YES)
		xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &store->tr_repeat_count[i]);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_float((XDR *)mb_io_ptr->xdrs, &store->tr_start[i]);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_float((XDR *)mb_io_ptr->xdrs, &store->tr_stop[i]);
	    }

	/* print debug statements */
#ifndef MBR_HSDS2RAW_DEBUG
	if (verbose >= 5)
#endif
		{
		fprintf(stderr,"\ndbg5  Tracking windows telegram read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       xdr_status:              %d\n",xdr_status);
		fprintf(stderr,"dbg5       telegram_id:             %d\n",telegram_id);
		fprintf(stderr,"dbg5       telegram_cnt:            %d\n",telegram_cnt);
		fprintf(stderr,"dbg5       telegram_send:           %s\n",telegram_send);
		fprintf(stderr,"dbg5       telegram_recv:           %s\n",telegram_recv);
		fprintf(stderr,"dbg5       telegram_utc_time_d:     %f\n",telegram_utc_time_d);
		fprintf(stderr,"dbg5       telegram_loc_time_d:     %f\n",telegram_utc_time_d);
		fprintf(stderr,"dbg5       telegram_block_no:       %d\n",telegram_block_no);
		fprintf(stderr,"dbg5       telegram_block_cnt:      %d\n",telegram_block_cnt);
		fprintf(stderr,"dbg5       telegram_max_no:         %d\n",telegram_max_no);
		fprintf(stderr,"dbg5       telegram_act_no:         %d\n",telegram_act_no);
		fprintf(stderr,"dbg5       telegram_data_status:    %d\n",telegram_data_status);
		fprintf(stderr,"dbg5       telegram_sensor_status:  %d\n",telegram_sensor_status);
		fprintf(stderr,"dbg5       tr_ping_no:              %d\n",store->tr_ping_no);
		fprintf(stderr,"dbg5       tr_transmit_time_d:      %f\n",store->tr_transmit_time_d);
		fprintf(stderr,"dbg5       tr_window_mode:          %d\n",store->tr_window_mode);
		fprintf(stderr,"dbg5       tr_no_of_win_groups:     %d\n",store->tr_no_of_win_groups);
		if (verbose > 0)
		for (i=0;i<MBSYS_ATLAS_MAXWINDOWS;i++)
			{
			fprintf(stderr,"dbg5       window[%d]:cnt start stop: %d %f %f\n",
				i, store->tr_repeat_count[i], store->tr_start[i], store->tr_stop[i]);
			}
		}

	/* get backscatter telegram */
	xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_id);

	/* telegram id ok - just read send and receive strings */
	if (xdr_status == MB_YES && telegram_id == MBSYS_ATLAS_TELEGRAM_BACKSCATTER)
	    {
	    if (xdr_status == MB_YES)
		xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_cnt);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)telegram_send, 16);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)telegram_recv, 16);
	    }

	/* expected telegram id wrong - try to resync on recv string */
	else if (xdr_status == MB_YES)
	    {
	    memset(telegram_recv, 0, 16);
	    read_status = 1;
	    nskip = 0;
	    while (strncmp(telegram_recv, "BROADCAST", 9) != 0
		&& read_status == 1)
		{
		for (i=0;i<15;i++)
		    telegram_recv[i] = telegram_recv[i+1];
		if ((read_status = fread(&(telegram_recv[15]), 1, 1, mb_io_ptr->mbfp)) == 1)
		    nskip++;
		}
	    if (read_status == MB_YES)
		{
		fprintf(stderr, "Resync on BACKSCATTER telegram: %d missing bytes\n",
			(44 - nskip));
		}
	    else
		{
		xdr_status = MB_NO;
		}
	    }

	/* hopefully we are synced - read the telegram */
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &telegram_utc_time_d);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &telegram_loc_time_d);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_block_no);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_block_cnt);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_data_status);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &telegram_sensor_status);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs, &store->bs_transmit_time_d);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &store->bs_ping_no);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_u_short((XDR *)mb_io_ptr->xdrs, &store->bs_nrActualGainSets);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_float((XDR *)mb_io_ptr->xdrs, &store->bs_rxGup);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_float((XDR *)mb_io_ptr->xdrs, &store->bs_rxGain);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_float((XDR *)mb_io_ptr->xdrs, &store->bs_ar);
	for (i=0;i<MBSYS_ATLAS_HSDS2_RX_PAR;i++)
	    {
	    if (xdr_status == MB_YES)
		xdr_status = xdr_float((XDR *)mb_io_ptr->xdrs, &store->bs_TvgRx_time[i]);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_float((XDR *)mb_io_ptr->xdrs, &store->bs_TvgRx_gain[i]);
	    }
	if (xdr_status == MB_YES)
	    xdr_status = xdr_u_short((XDR *)mb_io_ptr->xdrs, &store->bs_nrTxSets);
	for (i=0;i<MBSYS_ATLAS_HSDS2_TX_PAR;i++)
	    {
	    if (xdr_status == MB_YES)
		xdr_status = xdr_u_int((XDR *)mb_io_ptr->xdrs, &store->bs_txBeamIndex[i]);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_float((XDR *)mb_io_ptr->xdrs, &store->bs_txLevel[i]);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_float((XDR *)mb_io_ptr->xdrs, &store->bs_txBeamAngle[i]);
	    if (xdr_status == MB_YES)
		xdr_status = xdr_float((XDR *)mb_io_ptr->xdrs, &store->bs_pulseLength[i]);
	    }
	if (xdr_status == MB_YES)
	    xdr_status = xdr_u_short((XDR *)mb_io_ptr->xdrs, &store->bs_nrBsSets);
	for (i=0;i<MBSYS_ATLAS_HSDS2_PFB_NUM;i++)
	    {
	    if (xdr_status == MB_YES)
		xdr_status = xdr_float((XDR *)mb_io_ptr->xdrs, &store->bs_m_tau[i]);
	    }
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)store->bs_eff_ampli, MBSYS_ATLAS_HSDS2_PFB_NUM);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs, &length);
	if (xdr_status == MB_YES)
	    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs, (char *)store->bs_nis, MBSYS_ATLAS_HSDS2_PFB_NUM);

	/* print debug statements */
#ifndef MBR_HSDS2RAW_DEBUG
	if (verbose >= 5)
#endif
		{
		fprintf(stderr,"\ndbg5  Backscatter telegram read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       xdr_status:              %d\n",xdr_status);
		fprintf(stderr,"dbg5       telegram_id:             %d\n",telegram_id);
		fprintf(stderr,"dbg5       telegram_cnt:            %d\n",telegram_cnt);
		fprintf(stderr,"dbg5       telegram_send:           %s\n",telegram_send);
		fprintf(stderr,"dbg5       telegram_recv:           %s\n",telegram_recv);
		fprintf(stderr,"dbg5       telegram_utc_time_d:     %f\n",telegram_utc_time_d);
		fprintf(stderr,"dbg5       telegram_loc_time_d:     %f\n",telegram_utc_time_d);
		fprintf(stderr,"dbg5       telegram_block_no:       %d\n",telegram_block_no);
		fprintf(stderr,"dbg5       telegram_block_cnt:      %d\n",telegram_block_cnt);
		fprintf(stderr,"dbg5       telegram_max_no:         %d\n",telegram_max_no);
		fprintf(stderr,"dbg5       telegram_act_no:         %d\n",telegram_act_no);
		fprintf(stderr,"dbg5       telegram_data_status:    %d\n",telegram_data_status);
		fprintf(stderr,"dbg5       telegram_sensor_status:  %d\n",telegram_sensor_status);
		fprintf(stderr,"dbg5       bs_ping_no:              %d\n",store->bs_ping_no);
		fprintf(stderr,"dbg5       bs_transmit_time_d:      %f\n",store->bs_transmit_time_d);
		fprintf(stderr,"dbg5       bs_nrActualGainSets:     %d\n",store->bs_nrActualGainSets);
		fprintf(stderr,"dbg5       bs_rxGup:                %f\n",store->bs_rxGup);
		fprintf(stderr,"dbg5       bs_rxGain:               %f\n",store->bs_rxGain);
		fprintf(stderr,"dbg5       bs_ar:                   %f\n",store->bs_ar);
		if (verbose > 0)
		for (i=0;i<MBSYS_ATLAS_HSDS2_RX_PAR;i++)
			{
			fprintf(stderr,"dbg5       tvgrx[%d]: time gain: %f %f\n",
				i, store->bs_TvgRx_time[i], store->bs_TvgRx_gain[i]);
			}
		fprintf(stderr,"dbg5       bs_nrTxSets:             %d\n",store->bs_nrTxSets);
		if (verbose > 0)
		for (i=0;i<MBSYS_ATLAS_HSDS2_TX_PAR;i++)
			{
			fprintf(stderr,"dbg5       tx[%d]: # gain ang len:    %d %f %f %f\n",
				i, store->bs_txBeamIndex[i], store->bs_txLevel[i],
				store->bs_txBeamAngle[i], store->bs_pulseLength[i]);
			}
		fprintf(stderr,"dbg5       bs_nrBsSets:             %d\n",store->bs_nrBsSets);
		if (verbose > 0)
		for (i=0;i<MBSYS_ATLAS_HSDS2_TX_PAR;i++)
			{
			fprintf(stderr,"dbg5       bs[%d]: # tau amp nis:   %f %d %d\n",
				i, store->bs_m_tau[i], store->bs_eff_ampli[i], store->bs_nis[i]);
			}
		}

	/* set error if required */
	if (xdr_status == MB_NO)
		{
		*error = MB_ERROR_EOF;
		status = MB_FAILURE;
		}

	/* check for broken records - these do happen!!! */
	if (status == MB_SUCCESS
	    && (store->tt_beam_cnt > MBSYS_ATLAS_MAXBEAMS
		|| store->ss_max_side_bb_cnt > MBSYS_ATLAS_MAXPIXELS
		|| store->ss_max_side_sb_cnt > MBSYS_ATLAS_MAXPIXELS
		|| store->start_opmode[0] != 1))
		{
		*error = MB_ERROR_UNINTELLIGIBLE;
		status = MB_FAILURE;
		}

	/* check again for broken records - these do happen!!! */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<store->tt_beam_cnt;i++)
		{
		if (store->tt_lruntime[i] > 20.0)
		    {
		    *error = MB_ERROR_UNINTELLIGIBLE;
		    status = MB_FAILURE;
		    }
		}
	    }

	/* check again for broken records - these do happen!!! */
	if (status == MB_SUCCESS)
	    {
	    for (i=0;i<store->tt_beam_cnt;i++)
		{
		if (store->tt_lruntime[i] > 20.0)
		    {
		    *error = MB_ERROR_UNINTELLIGIBLE;
		    status = MB_FAILURE;
		    }
		}
	    }

	/* calculate first cut bathymetry */
	if (status == MB_SUCCESS)
		{
		/* get angle_table for 90 degree coverage */
		if (store->start_opmode[3] == 0)
			{
			if (store->tt_beam_cnt == 140)
		    		angle_table = (double *) ds2_ang_90d_140b;
			else if (store->tt_beam_cnt == 59)
		    		angle_table = (double *) ds2_ang_90d_59b;
			}

		/* get angle_table for 120 degree coverage */
		else if (store->start_opmode[3] == 1)
			{
			if (store->tt_beam_cnt == 140)
		    		angle_table = (double *) ds2_ang_120d_140b;
			else if (store->tt_beam_cnt == 59)
		    		angle_table = (double *) ds2_ang_120d_59b;
			}

		/* calculate bathymetry */
		for (i=0;i<store->tt_beam_cnt;i++)
			{
			if (store->tt_lruntime[i] > 0.0)
				{
				rr = store->start_cmean * store->tt_lruntime[i] / 2.0;
				store->pr_bath[i] = rr * cos(angle_table[i])
									+ store->start_heave + store->tt_draught;
				store->pr_bathacrosstrack[i] = rr * sin(angle_table[i]);
 				store->pr_bathalongtrack[i] = 0.0;
				store->pr_beamflag[i] = MB_FLAG_NONE;
				}
			else
				{
				store->pr_bath[i] = 0.0;
				store->pr_bathacrosstrack[i] = 0.0;
				store->pr_bathalongtrack[i] = 0.0;
				store->pr_beamflag[i] = MB_FLAG_NULL;
				}
			}
		}

	/* look for navigation if needed */
	if ((XDR *)mb_io_ptr->xdrs2 != NULL
		&& ((mb_io_ptr->nfix > 0
				&& mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1] < store->tt_transmit_time_d)
			|| mb_io_ptr->nfix <= 0))
		{
		done = MB_NO;
		while (done == MB_NO)
			{
			/* get system telegram */
			xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &telegram_id);

			/* telegram id ok - just read send and receive strings */
			if (xdr_status == MB_YES && telegram_id == MBSYS_ATLAS_TELEGRAM_SYSTEM)
			    {
			    if (xdr_status == MB_YES)
				xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &telegram_cnt);
			    if (xdr_status == MB_YES)
				xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			    if (xdr_status == MB_YES)
				xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, telegram_send, 16);
			    if (xdr_status == MB_YES)
				xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			    if (xdr_status == MB_YES)
				xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, telegram_recv, 16);
			    }

			/* expected telegram id wrong - try to resync on recv string */
			else if (xdr_status == MB_YES)
			    {
			    memset(telegram_recv, 0, 16);
			    read_status = 1;
			    nskip = 0;
			    while (strncmp(telegram_recv, "BROADCAST", 9) != 0
				&& read_status == 1)
				{
				for (i=0;i<15;i++)
				    telegram_recv[i] = telegram_recv[i+1];
				if ((read_status = fread(&(telegram_recv[15]), 1, 1, mb_io_ptr->mbfp2)) == 1)
				    nskip++;
				}
			    if (read_status == MB_YES)
				{
				fprintf(stderr, "Resync on SYSTEM telegram: %d missing bytes\n",
					(44 - nskip));
				}
			    else
				{
				xdr_status = MB_NO;
				}
			    }

			/* hopefully we are synced - read the telegram */
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &telegram_utc_time_d);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &telegram_loc_time_d);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &telegram_block_no);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &telegram_block_cnt);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, carrier_name, 8);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, task_name, 16);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, operator_name, 32);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, gauge_name, 32);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, comment, 32);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, profile_name, 32);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &profile_version);

			/* print debug statements */
#ifndef MBR_HSDS2RAW_DEBUG
			if (verbose >= 5)
#endif
				{
				fprintf(stderr,"\ndbg5  System telegram read in MBIO function <%s>\n",function_name);
				fprintf(stderr,"dbg5       xdr_status:              %d\n",xdr_status);
				fprintf(stderr,"dbg5       telegram_id:             %d\n",telegram_id);
				fprintf(stderr,"dbg5       telegram_cnt:            %d\n",telegram_cnt);
				fprintf(stderr,"dbg5       telegram_send:           %s\n",telegram_send);
				fprintf(stderr,"dbg5       telegram_recv:           %s\n",telegram_recv);
				fprintf(stderr,"dbg5       telegram_utc_time_d:     %f\n",telegram_utc_time_d);
				fprintf(stderr,"dbg5       telegram_loc_time_d:     %f\n",telegram_utc_time_d);
				fprintf(stderr,"dbg5       telegram_block_no:       %d\n",telegram_block_no);
				fprintf(stderr,"dbg5       telegram_block_cnt:      %d\n",telegram_block_cnt);
				fprintf(stderr,"dbg5       carrier_name:            %s\n",carrier_name);
				fprintf(stderr,"dbg5       task_name:               %s\n",task_name);
				fprintf(stderr,"dbg5       operator_name:           %s\n",operator_name);
				fprintf(stderr,"dbg5       gauge_name:              %s\n",gauge_name);
				fprintf(stderr,"dbg5       comment:                 %s\n",comment);
				fprintf(stderr,"dbg5       profile_name:            %s\n",profile_name);
				fprintf(stderr,"dbg5       profile_version:         %d\n",profile_version);
				}

			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &telegram_block_no);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &telegram_block_cnt);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_pos_lat);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_pos_lon);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, sys_pos_sensor, 8);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_pos_lat_tpe);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_pos_lon_tpe);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_pos_time);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_pos_data_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_pos_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_height);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, sys_height_sensor, 8);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_height_time);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_height_data_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_height_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_speed_wlong);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_speed_wcross);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, sys_tw_sensor, 8);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_tw_time);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_tw_data_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_tw_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_cog);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, sys_cog_sensor, 8);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_cog_time);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_cog_data_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_cog_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_sog);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, sys_sog_sensor, 8);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_sog_time);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_sog_data_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_sog_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_set);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_drift);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, sys_set_drift_sensor, 8);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_set_drift_time);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_set_drift_data_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_set_drift_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_heading);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, sys_heading_sensor, 8);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_heading_time);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_heading_data_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_heading_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_depth);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, sys_depth_sensor, 8);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_depth_water_level);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_depth_time);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_depth_data_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_depth_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_wspeed_abs);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_wdir_abs);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &length);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_opaque((XDR *)mb_io_ptr->xdrs2, sys_wind_sensor, 8);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_double((XDR *)mb_io_ptr->xdrs2, &sys_wind_time);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_wind_data_status);
			if (xdr_status == MB_YES)
			    xdr_status = xdr_int((XDR *)mb_io_ptr->xdrs2, &sys_wind_status);

			/* print debug statements */
#ifndef MBR_HSDS2RAW_DEBUG
			if (verbose >= 5)
#endif
				{
				fprintf(stderr,"dbg5       xdr_status:              %d\n",xdr_status);
				fprintf(stderr,"dbg5       telegram_block_no:       %d\n",telegram_block_no);
				fprintf(stderr,"dbg5       telegram_block_cnt:      %d\n",telegram_block_cnt);
				fprintf(stderr,"dbg5       sys_pos_lat:             %f\n",sys_pos_lat);
				fprintf(stderr,"dbg5       sys_pos_lon:             %f\n",sys_pos_lon);
				fprintf(stderr,"dbg5       sys_pos_sensor:          %s\n",sys_pos_sensor);
				fprintf(stderr,"dbg5       sys_pos_lat_tpe:         %f\n",sys_pos_lat_tpe);
				fprintf(stderr,"dbg5       sys_pos_lon_tpe:         %f\n",sys_pos_lon_tpe);
				fprintf(stderr,"dbg5       sys_pos_time:            %f\n",sys_pos_time);
				fprintf(stderr,"dbg5       sys_pos_data_status:     %d\n",sys_pos_data_status);
				fprintf(stderr,"dbg5       sys_pos_status:          %d\n",sys_pos_status);
				fprintf(stderr,"dbg5       sys_height:              %f\n",sys_height);
				fprintf(stderr,"dbg5       sys_height_sensor:       %s\n",sys_height_sensor);
				fprintf(stderr,"dbg5       sys_height_time:         %f\n",sys_height_time);
				fprintf(stderr,"dbg5       sys_height_data_status:  %d\n",sys_height_data_status);
				fprintf(stderr,"dbg5       sys_height_status:       %d\n",sys_height_status);
				fprintf(stderr,"dbg5       sys_speed_wlong:         %f\n",sys_speed_wlong);
				fprintf(stderr,"dbg5       sys_speed_wcross:        %f\n",sys_speed_wcross);
				fprintf(stderr,"dbg5       sys_tw_sensor:           %s\n",sys_tw_sensor);
				fprintf(stderr,"dbg5       sys_tw_time:             %f\n",sys_tw_time);
				fprintf(stderr,"dbg5       sys_tw_data_status:      %d\n",sys_tw_data_status);
				fprintf(stderr,"dbg5       sys_tw_status:           %d\n",sys_tw_status);
				fprintf(stderr,"dbg5       sys_cog:                 %f\n",sys_cog);
				fprintf(stderr,"dbg5       sys_cog_sensor:          %s\n",sys_cog_sensor);
				fprintf(stderr,"dbg5       sys_cog_time:            %f\n",sys_cog_time);
				fprintf(stderr,"dbg5       sys_cog_data_status:     %d\n",sys_cog_data_status);
				fprintf(stderr,"dbg5       sys_cog_status:          %d\n",sys_cog_status);
				fprintf(stderr,"dbg5       sys_sog:                 %f\n",sys_sog);
				fprintf(stderr,"dbg5       sys_sog_sensor:          %s\n",sys_sog_sensor);
				fprintf(stderr,"dbg5       sys_sog_time:            %f\n",sys_sog_time);
				fprintf(stderr,"dbg5       sys_sog_data_status:     %d\n",sys_sog_data_status);
				fprintf(stderr,"dbg5       sys_sog_status:          %d\n",sys_sog_status);
				fprintf(stderr,"dbg5       sys_set:                 %f\n",sys_set);
				fprintf(stderr,"dbg5       sys_drift:               %f\n",sys_drift);
				fprintf(stderr,"dbg5       sys_set_drift_sensor:    %s\n",sys_set_drift_sensor);
				fprintf(stderr,"dbg5       sys_set_drift_time:      %f\n",sys_set_drift_time);
				fprintf(stderr,"dbg5       sys_set_drift_data_status: %d\n",sys_set_drift_data_status);
				fprintf(stderr,"dbg5       sys_set_drift_status:      %d\n",sys_set_drift_status);
				fprintf(stderr,"dbg5       sys_heading:             %f\n",sys_heading);
				fprintf(stderr,"dbg5       sys_heading_sensor:      %s\n",sys_heading_sensor);
				fprintf(stderr,"dbg5       sys_heading_time:        %f\n",sys_heading_time);
				fprintf(stderr,"dbg5       sys_heading_data_status: %d\n",sys_heading_data_status);
				fprintf(stderr,"dbg5       sys_heading_status:      %d\n",sys_heading_status);
				fprintf(stderr,"dbg5       sys_depth:               %f\n",sys_depth);
				fprintf(stderr,"dbg5       sys_depth_sensor:        %s\n",sys_depth_sensor);
				fprintf(stderr,"dbg5       sys_depth_water_level:   %f\n",sys_depth_water_level);
				fprintf(stderr,"dbg5       sys_depth_time:          %f\n",sys_depth_time);
				fprintf(stderr,"dbg5       sys_depth_data_status:   %d\n",sys_depth_data_status);
				fprintf(stderr,"dbg5       sys_depth_status:        %d\n",sys_depth_status);
				fprintf(stderr,"dbg5       sys_wspeed_abs:          %f\n",sys_wspeed_abs);
				fprintf(stderr,"dbg5       sys_wdir_abs:            %f\n",sys_wdir_abs);
				fprintf(stderr,"dbg5       sys_wind_sensor:         %s\n",sys_wind_sensor);
				fprintf(stderr,"dbg5       sys_wind_time:           %f\n",sys_wind_time);
				fprintf(stderr,"dbg5       sys_wind_data_status:    %d\n",sys_wind_data_status);
				fprintf(stderr,"dbg5       sys_wind_status:         %d\n",sys_wind_status);
				}

			/* add to nav fix list */
			if (xdr_status == MB_YES)
				{
				mb_navint_add(verbose, mbio_ptr,
						sys_pos_time, (RTD * sys_pos_lon), (RTD * sys_pos_lat),
						error);
				if (mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1]
					>= store->tt_transmit_time_d)
					done = MB_YES;
				}
			else
				done = MB_YES;
			}
		}

	/* now interpolate navigation if available */
	if (mb_io_ptr->nfix > 0)
		{
		mb_navint_interp(verbose, mbio_ptr,
				store->tt_transmit_time_d, store->start_heading, 0.0,
				&(store->pr_navlon), &(store->pr_navlat), &pspeed,
				error);
		store->pr_speed = pspeed / 3.6;
		}

	/* set kind */
	if (store->start_opmode[12] == 0)
		store->kind = MB_DATA_DATA;
	else
		store->kind = MB_DATA_CALIBRATE;

	/* get file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);
	if (mb_io_ptr->mbfp2 != NULL)
		mb_io_ptr->file2_bytes = ftell(mb_io_ptr->mbfp2);

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
int mbr_hsds2raw_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_hsds2raw_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_atlas_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_atlas_struct *) store_ptr;

	/* print output debug statements */
#ifndef MBR_HSDS2RAW_DEBUG
	if (verbose >= 5)
#endif
		{
		fprintf(stderr,"\ndbg5  Data record kind in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",store->kind);
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
