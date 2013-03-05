/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mgd77dat.c	5/18/99
 *	$Id: mbr_mgd77dat.c 1917 2012-01-10 19:25:33Z caress $
 *
 *    Copyright (c) 1999-2012 by
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
 * mbr_mgd77dat.c contains the functions for reading and writing
 * multibeam data in the MGD77DAT format.
 * These functions include:
 *   mbr_alm_mgd77dat	- allocate read/write memory
 *   mbr_dem_mgd77dat	- deallocate read/write memory
 *   mbr_rt_mgd77dat	- read and translate data
 *   mbr_wt_mgd77dat	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	May 18, 1999
 *
 * $Log: mbr_mgd77dat.c,v $
 * Revision 5.11  2008/01/14 17:52:34  caress
 * Fixed problem with comments that prevented proper processing.
 *
 * Revision 5.10  2005/11/05 00:48:03  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.9  2004/09/24 20:44:44  caress
 * Implemented code fixes provided by Bob Covill.
 *
 * Revision 5.8  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.7  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.6  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.5  2002/08/21 00:55:46  caress
 * Release 5.0.beta22
 *
 * Revision 5.4  2002/02/22 09:03:43  caress
 * Release 5.0.beta13
 *
 * Revision 5.3  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
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
 * Revision 4.3  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.2  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.1  1999/07/16  19:29:09  caress
 * First revision.
 *
 * Revision 1.1  1999/07/16  19:24:15  caress
 * Initial revision
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_singlebeam.h"
#include "mbf_mgd77dat.h"

/* essential function prototypes */
int mbr_register_mgd77dat(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_mgd77dat(int verbose, 
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
int mbr_alm_mgd77dat(int verbose, void *mbio_ptr, int *error);
int mbr_dem_mgd77dat(int verbose, void *mbio_ptr, int *error);
int mbr_zero_mgd77dat(int verbose, char *data_ptr, int *error);
int mbr_rt_mgd77dat(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_mgd77dat(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_mgd77dat_rd_data(int verbose, void *mbio_ptr, int *error);
int mbr_mgd77dat_wr_data(int verbose, void *mbio_ptr, void *data_ptr, int *error);

static char rcs_id[]="$Id: mbr_mgd77dat.c 1917 2012-01-10 19:25:33Z caress $";

/*--------------------------------------------------------------------*/
int mbr_register_mgd77dat(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_mgd77dat";
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
	status = mbr_info_mgd77dat(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_mgd77dat;
	mb_io_ptr->mb_io_format_free = &mbr_dem_mgd77dat; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_singlebeam_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_singlebeam_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_mgd77dat; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_mgd77dat; 
	mb_io_ptr->mb_io_dimensions = &mbsys_singlebeam_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_singlebeam_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_singlebeam_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_singlebeam_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_singlebeam_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_singlebeam_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_singlebeam_ttimes; 
	mb_io_ptr->mb_io_detects = &mbsys_singlebeam_detects; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_singlebeam_copy; 
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
		fprintf(stderr,"dbg2       format_alloc:       %lu\n",(size_t)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %lu\n",(size_t)mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %lu\n",(size_t)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %lu\n",(size_t)mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %lu\n",(size_t)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %lu\n",(size_t)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %lu\n",(size_t)mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %lu\n",(size_t)mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %lu\n",(size_t)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %lu\n",(size_t)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %lu\n",(size_t)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %lu\n",(size_t)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %lu\n",(size_t)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %lu\n",(size_t)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %lu\n",(size_t)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       detects:            %lu\n",(size_t)mb_io_ptr->mb_io_detects);
		fprintf(stderr,"dbg2       extract_rawss:      %lu\n",(size_t)mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr,"dbg2       insert_rawss:       %lu\n",(size_t)mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr,"dbg2       copyrecord:         %lu\n",(size_t)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_mgd77dat(int verbose, 
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
	char	*function_name = "mbr_info_mgd77dat";
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
	*system = MB_SYS_SINGLEBEAM;
	*beams_bath_max = 1;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "MGD77DAT", MB_NAME_LENGTH);
	strncpy(system_name, "SINGLEBEAM", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_MGD77DAT\nInformal Description: NGDC MGD77 underway geophysics format\nAttributes:           single beam bathymetry, nav, magnetics,\n                      gravity, ascii, NOAA NGDC\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_YES;
	*beam_flagging = MB_NO;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 0.0;

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
int mbr_alm_mgd77dat(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_mgd77dat";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77dat_struct *data;
	char	*data_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_mgd77dat_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_singlebeam_struct),
				&mb_io_ptr->store_data,error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_mgd77dat_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	
	/* set number of header records read to zero */
	mb_io_ptr->save1 = 0;

	/* initialize everything to zeros */
	mbr_zero_mgd77dat(verbose,data_ptr,error);

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
int mbr_dem_mgd77dat(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_mgd77dat";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
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
int mbr_zero_mgd77dat(int verbose, char *data_ptr, int *error)
{
	char	*function_name = "mbr_zero_mgd77dat";
	int	status = MB_SUCCESS;
	struct mbf_mgd77dat_struct *data;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       data_ptr:   %lu\n",(size_t)data_ptr);
		}

	/* get pointer to data descriptor */
	data = (struct mbf_mgd77dat_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		data->kind = MB_DATA_NONE;
		for (i=0;i<8;i++)
		    data->survey_id[i] = 0;
		data->time_d = 0.0;
		for (i=0;i<7;i++)
		    data->time_i[i] = 0;
		data->timezone = 0;
		data->longitude = 0.0;
		data->latitude = 0.0;
		data->heading = 0.0;
		data->speed = 0.0;
		data->nav_type = 9;
		data->nav_quality = 9;
		data->roll = 0.0;
		data->pitch = 0.0;
		data->heave = 0.0;
		data->tt = 0.0;
		data->flag = MB_FLAG_NULL;
		data->bath = 0.0;
		data->bath_corr = 99;
		data->bath_type = 9;
		data->mag_tot_1 = 0.0;
		data->mag_tot_2 = 0.0;
		data->mag_res = 0.0;
		data->mag_res_sensor = 9;
		data->mag_diurnal = 0.0;
		data->mag_altitude = 0.0;
		data->gravity = 0.0;
		data->eotvos = 0.0;
		data->free_air = 0.0;
		data->seismic_line = 0;
		data->seismic_shot = 0;
		for (i=0;i<MBF_MGD77DAT_DATA_LEN;i++)
		    data->comment[i] = 0;
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
int mbr_rt_mgd77dat(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_mgd77dat";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77dat_struct *data;
	struct mbsys_singlebeam_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_mgd77dat_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_singlebeam_struct *) store_ptr;

	/* read next data from file */
	status = mbr_mgd77dat_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate values to data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		store->kind = data->kind;
		for (i=0;i<8;i++)
		    store->survey_id[i] = data->survey_id[i];
		store->time_d = data->time_d;
		for (i=0;i<7;i++)
		    store->time_i[i] = data->time_i[i];
		store->timezone = data->timezone;
		store->longitude = data->longitude;
		store->latitude = data->latitude;
		store->heading = data->heading;
		store->speed = data->speed;
		store->nav_type = data->nav_type;
		store->nav_quality = data->nav_quality;
		store->roll = data->roll;
		store->pitch = data->pitch;
		store->heave = data->heave;
		store->flag = data->flag;
		store->tt = data->tt;
		store->bath = data->bath;
		store->bath_corr = data->bath_corr;
		store->bath_type = data->bath_type;
		store->mag_tot_1 = data->mag_tot_1;
		store->mag_tot_2 = data->mag_tot_2;
		store->mag_res = data->mag_res;
		store->mag_res_sensor = data->mag_res_sensor;
		store->mag_diurnal = data->mag_diurnal;
		store->mag_altitude = data->mag_altitude;
		store->gravity = data->gravity;
		store->eotvos = data->eotvos;
		store->free_air = data->free_air;
		store->seismic_line = data->seismic_line;
		store->seismic_shot = data->seismic_shot;
		for (i=0;i<MBSYS_SINGLEBEAM_MAXLINE;i++)
		    store->comment[i] = data->comment[i];
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
int mbr_wt_mgd77dat(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_mgd77dat";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77dat_struct *data;
	struct mbsys_singlebeam_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_mgd77dat_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_singlebeam_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		data->kind = store->kind;
		for (i=0;i<8;i++)
		    data->survey_id[i] = store->survey_id[i];
		data->time_d = store->time_d;
		for (i=0;i<7;i++)
		    data->time_i[i] = store->time_i[i];
		data->timezone = store->timezone;
		data->longitude = store->longitude;
		data->latitude = store->latitude;
		data->heading = store->heading;
		data->speed = store->speed;
		data->nav_type = store->nav_type;
		data->nav_quality = store->nav_quality;
		data->roll = store->roll;
		data->pitch = store->pitch;
		data->heave = store->heave;
		data->flag = store->flag;
		data->tt = store->tt;
		data->bath = store->bath;
		data->bath_corr = store->bath_corr;
		data->bath_type = store->bath_type;
		data->mag_tot_1 = store->mag_tot_1;
		data->mag_tot_2 = store->mag_tot_2;
		data->mag_res = store->mag_res;
		data->mag_res_sensor = store->mag_res_sensor;
		data->mag_diurnal = store->mag_diurnal;
		data->mag_altitude = store->mag_altitude;
		data->gravity = store->gravity;
		data->eotvos = store->eotvos;
		data->free_air = store->free_air;
		data->seismic_line = store->seismic_line;
		data->seismic_shot = store->seismic_shot;
		for (i=0;i<MBSYS_SINGLEBEAM_MAXLINE;i++)
		    data->comment[i] = store->comment[i];
		}

	/* write next data to file */
	status = mbr_mgd77dat_wr_data(verbose,mbio_ptr,(void *)data,error);

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
int mbr_mgd77dat_rd_data(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_mgd77dat_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77dat_struct *data;
	int	*header_read;
	char	line[MBF_MGD77DAT_DATA_LEN];
	int	read_len;
	int	shift;
	int 	neg_unit;
	int	itmp;
	double	dtmp;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_mgd77dat_struct *) mb_io_ptr->raw_data;
	header_read = (int *) &mb_io_ptr->save1;

	/* initialize everything to zeros */
	mbr_zero_mgd77dat(verbose,mb_io_ptr->raw_data,error);

	/* set file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next record */
	if ((read_len = fread(line,1,MBF_MGD77DAT_DATA_LEN,
			mb_io_ptr->mbfp)) == MBF_MGD77DAT_DATA_LEN) 
		{
		mb_io_ptr->file_bytes += read_len;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		mb_io_ptr->file_bytes += read_len;
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
/*fprintf(stderr,"_RAWLINE:");
for (i=0;i<MBF_MGD77DAT_DATA_LEN;i++)
fprintf(stderr,"%c",line[i]);
fprintf(stderr,"\n");*/
		
	/* handle "pseudo-mgd77" in which each record is
	 * followed by a cr or lf */
	for (i=0;i<MBF_MGD77DAT_DATA_LEN;i++)
	    {
	    if (line[i] == '\r' || line[i] == '\n')
	    	{
		for (j=i;j<MBF_MGD77DAT_DATA_LEN-1;j++)
			{
			line[j] = line[j+1];
			}
	        if ((read_len = fread(&line[MBF_MGD77DAT_DATA_LEN-1],1,1,
			    mb_io_ptr->mbfp)) == 1) 
		    {
		    mb_io_ptr->file_bytes += read_len;
		    status = MB_SUCCESS;
		    *error = MB_ERROR_NO_ERROR;
		    }
	        else
		    {
		    mb_io_ptr->file_bytes += read_len;
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }
		}
	    }
/*fprintf(stderr,"+FIXLINE:");
for (i=0;i<MBF_MGD77DAT_DATA_LEN;i++)
fprintf(stderr,"%c",line[i]);
fprintf(stderr,"\n");*/
	
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);	

	/* handle the data */
	if (status == MB_SUCCESS
	    && *header_read > 0
	    && *header_read < MBF_MGD77DAT_HEADER_NUM)
	    {
	    data->kind = MB_DATA_HEADER;
	    (*header_read)++;
	    for (i=0;i<MBF_MGD77DAT_DATA_LEN;i++)
		data->comment[i] = line[i];
	    }
	else if (status == MB_SUCCESS
	    && (line[0] == '1' || line[0] == '4'))
	    {
	    data->kind = MB_DATA_HEADER;
	    (*header_read) = 1;
	    for (i=0;i<MBF_MGD77DAT_DATA_LEN;i++)
		data->comment[i] = line[i];
	    }
	else if (status == MB_SUCCESS
	    && line[0] == '#')
	    {
	    data->kind = MB_DATA_COMMENT;
            strncpy(data->comment,&line[1],MBF_MGD77DAT_DATA_LEN-1);
	    }
	else if (status == MB_SUCCESS
	    && line[0] == '3')
	    {
	    data->kind = MB_DATA_DATA;
	    
	    /* get survey id */
	    shift = 1;
	    for (i=0;i<8;i++)
		data->survey_id[i] = line[i+shift];
		
	    /* get time */
	    shift += 8;
	    mb_get_int(&data->timezone, &line[shift], 5); shift += 5;
	    data->timezone = data->timezone / 100;
	    mb_get_int(&itmp, &line[shift], 2); shift += 2;
	    mb_fix_y2k(verbose, itmp, &data->time_i[0]);
	    mb_get_int(&data->time_i[1], &line[shift], 2); shift += 2;
	    mb_get_int(&data->time_i[2], &line[shift], 2); shift += 2;
	    mb_get_int(&data->time_i[3], &line[shift], 2); shift += 2;
	    mb_get_int(&itmp, &line[shift], 5); shift += 5;
	    data->time_i[4] = 0.001 * itmp;
	    dtmp = (itmp - 1000 * data->time_i[4]) * 0.06;
	    data->time_i[5] = (int) dtmp;
	    data->time_i[6] = 1000000 * (dtmp - data->time_i[5]);
	    mb_get_time(verbose,data->time_i,&data->time_d);
	    
	    /* get nav */
	    neg_unit = 8;
	    if (line[shift] == '-') {
		    shift += 1;
		    neg_unit = 7;
	    }
	    mb_get_int(&itmp, &line[shift], neg_unit); shift += neg_unit;
	    data->latitude = 0.00001 * itmp;
	    if (neg_unit == 7)
		    data->latitude = -data->latitude;

	    neg_unit = 9;
	    if (line[shift] == '-') {
		    shift += 1;
		    neg_unit = 8;
	    }
	    mb_get_int(&itmp, &line[shift], neg_unit); shift += neg_unit;
	    data->longitude = 0.00001 * itmp;
	    if (neg_unit == 8)
		    data->longitude = -data->longitude;
	    mb_get_int(&data->nav_type, &line[shift], 1); shift += 1;
	    
	    /* get bath */
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->tt = 0.0001 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->bath = 0.1 * itmp;
	    mb_get_int(&data->bath_corr, &line[shift], 2); shift += 2;
	    mb_get_int(&data->bath_type, &line[shift], 1); shift += 1;
	    if (data->bath > 0.0 && data->bath < 99999.9)
		{
		data->flag = MB_FLAG_NONE;
		}
	    else
		{
		data->flag = MB_FLAG_NULL;
		}
	    
	    /* get magnetics */
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_tot_1 = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_tot_2 = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_res = 0.1 * itmp;
	    mb_get_int(&data->mag_res_sensor, &line[shift], 1); shift += 1;
	    mb_get_int(&itmp, &line[shift], 5); shift += 5;
	    data->mag_diurnal = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_altitude = itmp;
	    
	    /* get gravity */
	    mb_get_int(&itmp, &line[shift], 7); shift += 7;
	    data->gravity = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->eotvos = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 5); shift += 5;
	    data->free_air = 0.1 * itmp;
	    mb_get_int(&data->seismic_line, &line[shift], 5); shift += 5;
	    mb_get_int(&data->seismic_shot, &line[shift], 6); shift += 6;
	    
	    /* get nav quality */
	    mb_get_int(&data->nav_quality, &line[shift], 1); shift += 1;
	    }
	else if (status == MB_SUCCESS
	    && line[0] == '5')
	    {
	    data->kind = MB_DATA_DATA;
	    
	    /* get survey id */
	    shift = 1;
	    for (i=0;i<8;i++)
		data->survey_id[i] = line[i+shift];
		
	    /* get time */
	    shift += 8;
	    mb_get_int(&data->timezone, &line[shift], 3); shift += 3;
	    mb_get_int(&data->time_i[0], &line[shift], 4); shift += 4;
	    mb_get_int(&data->time_i[1], &line[shift], 2); shift += 2;
	    mb_get_int(&data->time_i[2], &line[shift], 2); shift += 2;
	    mb_get_int(&data->time_i[3], &line[shift], 2); shift += 2;
	    mb_get_int(&itmp, &line[shift], 5); shift += 5;
	    data->time_i[4] = 0.001 * itmp;
	    dtmp = (itmp - 1000 * data->time_i[4]) * 0.06;
	    data->time_i[5] = (int) dtmp;
	    data->time_i[6] = 1000000 * (dtmp - data->time_i[5]);

	    mb_get_time(verbose,data->time_i,&data->time_d);
	    
	    /* get nav */
	    neg_unit = 8;
	    if (line[shift] == '-') {
		    shift += 1;
		    neg_unit = 7;
	    }
	    mb_get_int(&itmp, &line[shift], neg_unit); shift += neg_unit;
	    data->latitude = 0.00001 * itmp;
	    if (neg_unit == 7)
		    data->latitude = -data->latitude;

	    neg_unit = 9;
	    if (line[shift] == '-') {
		    shift += 1;
		    neg_unit = 8;
	    }
	    mb_get_int(&itmp, &line[shift], neg_unit); shift += neg_unit;
	    data->longitude = 0.00001 * itmp;
	    if (neg_unit == 8)
		    data->longitude = -data->longitude;

	    mb_get_int(&data->nav_type, &line[shift], 1); shift += 1;
	    
	    /* get bath */
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->tt = 0.0001 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->bath = 0.1 * itmp;
	    mb_get_int(&data->bath_corr, &line[shift], 2); shift += 2;
	    mb_get_int(&data->bath_type, &line[shift], 1); shift += 1;
	    if (data->bath > 0.0 && data->bath < 99999.9)
		{
		data->flag = MB_FLAG_NONE;
		}
	    else
		{
		data->flag = MB_FLAG_NULL;
		}
	    
	    /* get magnetics */
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_tot_1 = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_tot_2 = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_res = 0.1 * itmp;
	    mb_get_int(&data->mag_res_sensor, &line[shift], 1); shift += 1;
	    mb_get_int(&itmp, &line[shift], 5); shift += 5;
	    data->mag_diurnal = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_altitude = itmp;
	    
	    /* get gravity */
	    mb_get_int(&itmp, &line[shift], 7); shift += 7;
	    data->gravity = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->eotvos = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 5); shift += 5;
	    data->free_air = 0.1 * itmp;
	    mb_get_int(&data->seismic_line, &line[shift], 5); shift += 5;
	    mb_get_int(&data->seismic_shot, &line[shift], 6); shift += 6;
	    
	    /* get nav quality */
	    mb_get_int(&data->nav_quality, &line[shift], 1); shift += 1;
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
int mbr_mgd77dat_wr_data(int verbose, void *mbio_ptr, void *data_ptr, int *error)
{
	char	*function_name = "mbr_mgd77dat_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77dat_struct *data;
	char	line[MBF_MGD77DAT_DATA_LEN+1];
	int	itmp;
	int	write_len;
	int	shift;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       data_ptr:   %lu\n",(size_t)data_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_mgd77dat_struct *) data_ptr;

	/* handle the data */
	if (data->kind == MB_DATA_HEADER)
	    {
	    strcpy(line, data->comment);
	    for (i=strlen(line);i<MBF_MGD77DAT_DATA_LEN;i++)
		line[i] = ' ';
	    }
	else if (data->kind == MB_DATA_COMMENT)
	    {
	    line[0] = '#';
            strncpy(&line[1],data->comment,MBF_MGD77DAT_DATA_LEN-1);
	    for (i=strlen(line);i<MBF_MGD77DAT_DATA_LEN;i++)
		line[i] = ' ';
	    }
	else if (data->kind == MB_DATA_DATA)
	    {
	    /* set data record id */
	    shift = 0;
	    line[0] = '5'; shift += 1;
	    
	    /* get survey id */
	    for (i=0;i<8;i++)
		line[i+shift] = data->survey_id[i]; 
	    shift += 8;
		
	    /* get time */
	    sprintf(&line[shift], "%3.3d", data->timezone); shift += 3;
	    sprintf(&line[shift], "%4.4d", data->time_i[0]); shift += 4;
	    sprintf(&line[shift], "%2.2d", data->time_i[1]); shift += 2;
	    sprintf(&line[shift], "%2.2d", data->time_i[2]); shift += 2;
	    sprintf(&line[shift], "%2.2d", data->time_i[3]); shift += 2;
	    itmp = (1000.0 * data->time_i[4])
		    + (1000.0 * (data->time_i[5]/60.0))
			+ (1000.0 * ((data->time_i[6]/1000000.0)/60.0));
	    sprintf(&line[shift], "%5.5d", itmp); shift += 5;

	    /* get nav */
	    itmp = 100000 * data->latitude;
	    if (itmp < 0) {
		    sprintf(&line[shift], "-"); shift += 1;
		    sprintf(&line[shift], "%7.7d", itmp*-1); shift += 7;
	    } else {
		    sprintf(&line[shift], "%8.8d", itmp); shift += 8;
	    }
	    itmp = 100000 * data->longitude;
	    if (itmp < 0) {
		    sprintf(&line[shift], "-"); shift += 1;
		    sprintf(&line[shift], "%8.8d", itmp*-1); shift += 8;
	    } else {
		    sprintf(&line[shift], "%9.9d", itmp); shift += 9;
	    }
	    sprintf(&line[shift], "%1.1d", data->nav_type); shift += 1;
	    
	    /* get bath */
	    if (data->flag == MB_FLAG_NONE)
		{
		itmp = 10000 * data->tt;
		sprintf(&line[shift], "%6.6d", itmp); shift += 6;
		itmp = 10 * data->bath;
		sprintf(&line[shift], "%6.6d", itmp); shift += 6;
		}
	    else
		{
		itmp = 999999;
		sprintf(&line[shift], "%6.6d", itmp); shift += 6;
		sprintf(&line[shift], "%6.6d", itmp); shift += 6;
		}
	    sprintf(&line[shift], "%2.2d", data->bath_corr); shift += 2;
	    sprintf(&line[shift], "%1.1d", data->bath_type); shift += 1;

	    /* get magnetics */
	    itmp = 10 * data->mag_tot_1;
	    sprintf(&line[shift], "%6.6d", itmp); shift += 6;
	    itmp = 10 * data->mag_tot_2;
	    sprintf(&line[shift], "%6.6d", itmp); shift += 6;
	    itmp = 10 * data->mag_res;
	    sprintf(&line[shift], "%6.6d", itmp); shift += 6;
	    sprintf(&line[shift], "%1.1d", data->mag_res_sensor); shift += 1;
	    itmp = 10 * data->mag_diurnal;
	    sprintf(&line[shift], "%5.5d", itmp); shift += 5;
	    itmp = data->mag_altitude;
	    sprintf(&line[shift], "%6.6d", itmp); shift += 6;
	
	    /* get gravity */
	    itmp = 10 * data->gravity;
	    sprintf(&line[shift], "%7.7d", itmp); shift += 7;
	    itmp = 10 * data->eotvos;
	    sprintf(&line[shift], "%6.6d", itmp); shift += 6;
	    itmp = 10 * data->free_air;
	    sprintf(&line[shift], "%5.5d", itmp); shift += 5;
	    sprintf(&line[shift], "%5.5d", data->seismic_line); shift += 5;
	    sprintf(&line[shift], "%6.6d", data->seismic_shot); shift += 6;
	
	    /* get nav quality */
	    sprintf(&line[shift], "%1.1d", data->nav_quality); shift += 1;
	    }

	write_len = fwrite(line,1,MBF_MGD77DAT_DATA_LEN,mb_io_ptr->mbfp);
	if (write_len != MBF_MGD77DAT_DATA_LEN)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}


	/* print output debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data record kind in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",data->kind);
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
