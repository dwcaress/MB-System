/*--------------------------------------------------------------------
 *    The MB-system:	mbr_3ddepthp.c	2/11/93
 *	$Id$
 *
 *    Copyright (c) 1993-2014 by
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
 * mbr_3ddepthp.c contains the functions for reading and writing
 * multibeam data in the MBF_3DDEPTHP format.
 * These functions include:
 *   mbr_alm_3ddepthp	- allocate read/write memory
 *   mbr_dem_3ddepthp	- deallocate read/write memory
 *   mbr_rt_3ddepthp	- read and translate data
 *   mbr_wt_3ddepthp	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	December 27, 2013
 * 
 * $Log: mbr_3ddepthp.c,v $
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_3datdepthlidar.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "mb_swap.h"
#endif

/* local defines */
#define ZERO_ALL    0
#define ZERO_SOME   1
#define	MBF_3DDEPTHP_BUFFER_SIZE	MB_COMMENT_MAXLINE

/* essential function prototypes */
int mbr_register_3ddepthp(int verbose, void *mbio_ptr,
		int *error);
int mbr_info_3ddepthp(int verbose,
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
int mbr_alm_3ddepthp(int verbose, void *mbio_ptr, int *error);
int mbr_dem_3ddepthp(int verbose, void *mbio_ptr, int *error);
int mbr_rt_3ddepthp(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_3ddepthp(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_3ddepthp_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_3ddepthp_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_3ddepthp(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_3ddepthp";
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

	/* check for non-null structure pointers */
	assert(mbio_ptr != NULL);

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set format info parameters */
	status = mbr_info_3ddepthp(verbose,
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_3ddepthp;
	mb_io_ptr->mb_io_format_free = &mbr_dem_3ddepthp;
	mb_io_ptr->mb_io_store_alloc = &mbsys_3datdepthlidar_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_3datdepthlidar_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_3ddepthp;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_3ddepthp;
	mb_io_ptr->mb_io_dimensions = &mbsys_3datdepthlidar_dimensions;
	mb_io_ptr->mb_io_preprocess = &mbsys_3datdepthlidar_preprocess;
	mb_io_ptr->mb_io_extract = &mbsys_3datdepthlidar_extract;
	mb_io_ptr->mb_io_insert = &mbsys_3datdepthlidar_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_3datdepthlidar_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_3datdepthlidar_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_3datdepthlidar_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = &mbsys_3datdepthlidar_extract_svp;
	mb_io_ptr->mb_io_insert_svp = &mbsys_3datdepthlidar_insert_svp;
	mb_io_ptr->mb_io_ttimes = &mbsys_3datdepthlidar_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_3datdepthlidar_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_3datdepthlidar_copy;
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
		fprintf(stderr,"dbg2       preprocess:         %p\n",(void *)mb_io_ptr->mb_io_preprocess);
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
int mbr_info_3ddepthp(int verbose,
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
	char	*function_name = "mbr_info_3ddepthp";
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
	*system = MB_SYS_3DATDEPTHLIDAR;
	*beams_bath_max = 0;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "3DDEPTHP", MB_NAME_LENGTH);
	strncpy(system_name, "3DATDEPTHLIDAR", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_3DDEPTHP\nInformal Description: 3DatDepth prototype binary swath mapping LIDAR format\nAttributes:           3DatDepth LIDAR, variable pulses, bathymetry and amplitude, \n                      binary, 3DatDepth.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_YES;
	*traveltime = MB_NO;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 0.02;
	*beamwidth_ltrack = 0.02;

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
int mbr_alm_3ddepthp(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_3ddepthp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	*file_header_readwritten;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		}

	/* check for non-null pointers */
	assert(mbio_ptr != NULL);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	status = mbsys_3datdepthlidar_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	
	/* set file header read flag */
	file_header_readwritten = (int *) &mb_io_ptr->save1;
	*file_header_readwritten = MB_NO;
	
	/* set saved bytes flag */
	mb_io_ptr->save2 = MB_NO;

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
int mbr_dem_3ddepthp(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_3ddepthp";
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

	/* check for non-null pointers */
	assert(mbio_ptr != NULL);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory  */
	status = mbsys_3datdepthlidar_deall(verbose,mbio_ptr,&mb_io_ptr->store_data,error);

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
int mbr_rt_3ddepthp(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_3ddepthp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3datdepthlidar_struct *store;

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

	/* check for non-null pointers */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* get pointers to mbio descriptor and data structure */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_3datdepthlidar_struct *) store_ptr;

	/* read next data from file */
	status = mbr_3ddepthp_rd_data(verbose,mbio_ptr,store_ptr,error);
	
	/* if needed calculate bathymetry */
	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA
	    && store->bathymetry_calculated == MB_NO)
		{
		mbsys_3datdepthlidar_calculatebathymetry(verbose, mbio_ptr, store_ptr, error);
		}
		
	/* print out status info */
	if (verbose > 1)
		mbsys_3datdepthlidar_print_store(verbose, store_ptr, error);

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
int mbr_wt_3ddepthp(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_3ddepthp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3datdepthlidar_struct *store;

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

	/* check for non-null pointers */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* get pointers to mbio descriptor and data structure */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_3datdepthlidar_struct *) store_ptr;

	/* write next data to file */
	status = mbr_3ddepthp_wr_data(verbose,mbio_ptr,store_ptr,error);

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
int mbr_3ddepthp_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_3ddepthp_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3datdepthlidar_struct *store;
	struct mbsys_3datdepthlidar_pulse_struct *pulse;
	int	*file_header_readwritten;
	char	buffer[MBF_3DDEPTHP_BUFFER_SIZE];
	size_t	read_len;
	size_t	index;
	unsigned short magic_number;
	unsigned short unused;
	unsigned int *newscancheck, newscancheckvalue;
	int	done;
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

	/* check for non-null pointers */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_3datdepthlidar_struct *) store_ptr;

	/* get saved values */
	file_header_readwritten = (int *) &mb_io_ptr->save1;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
	
	/* set status */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	done = MB_NO;

	/* if first read then read 2 byte magic number at start of file */
	if (*file_header_readwritten == MB_NO)
		{
		/* read and check the first two bytes */
		read_len = (size_t)2;
		status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
		if (status == MB_SUCCESS)
			{
			index = 0;
			mb_get_binary_short(MB_YES, (void *) &buffer[index], &(magic_number)); index += 2;
			}
		
		/* if ok and magic_number == 0x3D46 then this is version 1.1 */
		if (status == MB_SUCCESS && magic_number == MBF_3DDEPTHP_MAGICNUMBER)
			{
			store->file_version = 1;
			store->sub_version = 1;
			*file_header_readwritten = MB_YES;
			}
		
		/* if ok and magic_number == 0x3D07 then this is the obsolete version 1.0
			- this file always starts with a parameter record */
		else if (status == MB_SUCCESS && magic_number == MBF_3DDEPTHP_RECORD_PARAMETER)
			{
			/* read file header into buffer */
			read_len = (size_t)MBF_3DDEPTHP_VERSION_1_0_PARAMETER_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
			
			/* if read ok then get values */
			if (status == MB_SUCCESS)
				{
				index = 0;
				store->record_id = MBF_3DDEPTHP_RECORD_PARAMETER;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->file_version)); index += 2;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->sub_version)); index += 2;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->scan_type)); index += 2;
				mb_get_binary_float(MB_YES, (void *) &buffer[index], &(store->cross_track_angle_start)); index += 4;
				mb_get_binary_float(MB_YES, (void *) &buffer[index], &(store->cross_track_angle_end)); index += 4;
				mb_get_binary_float(MB_YES, (void *) &buffer[index], &(store->forward_track_angle_start)); index += 4;
				mb_get_binary_float(MB_YES, (void *) &buffer[index], &(store->forward_track_angle_end)); index += 4;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->counts_per_scan)); index += 2;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->counts_per_cross_track)); index += 2;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->counts_per_forward_track)); index += 2;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->scanner_efficiency)); index += 2;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->scans_per_file)); index += 2;
				mb_get_binary_int(MB_YES, (void *) &buffer[index], &(store->scan_count)); index += 4;
				store->current_scan = -1;

				/* calculate number of pulses per scan according to mode */
				if (store->counts_per_scan <= 0)
					{
					if (store->counts_per_forward_track == 0)
						store->counts_per_scan = store->counts_per_cross_track;
					else if (store->counts_per_cross_track == 0)
						store->counts_per_scan = store->counts_per_forward_track;
					else
						store->counts_per_scan = store->counts_per_cross_track * store->counts_per_forward_track;
					}
				store->num_pulses_alloc = store->counts_per_scan;
					
				/* allocate memory for pulses */
				status = mb_mallocd(verbose, __FILE__, __LINE__,
							store->num_pulses_alloc * sizeof(struct mbsys_3datdepthlidar_pulse_struct),
							(void **)&store->pulses, error);
				if (status != MB_SUCCESS)
					store->num_pulses_alloc = 0;
				store->num_pulses = 0;
				
				/* success */
				if (status == MB_SUCCESS)
					{
					*file_header_readwritten = MB_YES;
					store->kind = MB_DATA_PARAMETER;
					done = MB_YES;
					}
				}	
			}
			
		/* else this isn't a known way for the file to start */
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			done = MB_YES;
			}
		}
		
	/* read next record in format version 1.1 */
	if (status == MB_SUCCESS && done == MB_NO
		&& store->file_version == 1 && store->sub_version == 1)
		{
//fprintf(stderr,"READ NEXT RECORD verions %d %d\n",store->file_version,store->sub_version);

		/* read the next record header */
		read_len = (size_t)sizeof(short);
		status = mb_fileio_get(verbose, mbio_ptr, (void *) &(store->record_id), &read_len, error);
//fprintf(stderr,"RECORD ID: %x %d\n",store->record_id,store->record_id);
		
		/* read parameter record */
		if (store->record_id == MBF_3DDEPTHP_RECORD_PARAMETER)
			{
			/* read file header into buffer */
			read_len = (size_t)MBF_3DDEPTHP_VERSION_1_1_PARAMETER_SIZE - 2;
			status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
			
			/* if read ok then get values */
			if (status == MB_SUCCESS)
				{
				index = 0;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->file_version)); index += 2;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->sub_version)); index += 2;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->scan_type)); index += 2;
				mb_get_binary_float(MB_YES, (void *) &buffer[index], &(store->cross_track_angle_start)); index += 4;
				mb_get_binary_float(MB_YES, (void *) &buffer[index], &(store->cross_track_angle_end)); index += 4;
				mb_get_binary_float(MB_YES, (void *) &buffer[index], &(store->forward_track_angle_start)); index += 4;
				mb_get_binary_float(MB_YES, (void *) &buffer[index], &(store->forward_track_angle_end)); index += 4;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->counts_per_scan)); index += 2;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->counts_per_cross_track)); index += 2;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->counts_per_forward_track)); index += 2;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->scanner_efficiency)); index += 2;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->scans_per_file)); index += 2;
				mb_get_binary_int(MB_YES, (void *) &buffer[index], &(store->scan_count)); index += 4;
				store->current_scan = -1;

				/* calculate number of pulses per scan according to mode */
				if (store->counts_per_scan <= 0)
					{
					if (store->counts_per_forward_track == 0)
						store->counts_per_scan = store->counts_per_cross_track;
					else if (store->counts_per_cross_track == 0)
						store->counts_per_scan = store->counts_per_forward_track;
					else
						store->counts_per_scan = store->counts_per_cross_track * store->counts_per_forward_track;
					}
				store->num_pulses_alloc = store->counts_per_scan;
					
				/* allocate memory for pulses */
				status = mb_mallocd(verbose, __FILE__, __LINE__,
							store->num_pulses_alloc * sizeof(struct mbsys_3datdepthlidar_pulse_struct),
							(void **)&store->pulses, error);
				if (status != MB_SUCCESS)
					store->num_pulses_alloc = 0;
				store->num_pulses = 0;
				
				/* success */
				if (status == MB_SUCCESS)
					{
					*file_header_readwritten = MB_YES;
					store->kind = MB_DATA_PARAMETER;
					}
				}
			}
		
		/* read comment record */
		else if (store->record_id == MBF_3DDEPTHP_RECORD_COMMENT)
			{
			/* read the comment data */
			read_len = (size_t) sizeof(short);
			status = mb_fileio_get(verbose, mbio_ptr, (void *) &store->comment_len, &read_len, error);
			if (status == MB_SUCCESS)
				{
				read_len = (size_t) store->comment_len;
				status = mb_fileio_get(verbose, mbio_ptr, (void *) &store->comment, &read_len, error);
			
				store->kind = MB_DATA_COMMENT;
				}
			}
		
		/* read position record */
		else if (store->record_id == MBF_3DDEPTHP_RECORD_POSITION)
			{
			/* read the position data */
			read_len = (size_t) (3 * sizeof(double));
			status = mb_fileio_get(verbose, mbio_ptr, (void *) buffer, &read_len, error);
			
			/* decode the data */
			if (status == MB_SUCCESS)
				{
				index = 0;
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->pos_time_d)); index += sizeof(double);
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->pos_longitude)); index += sizeof(double);
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->pos_latitude)); index += sizeof(double);
			
				store->kind = MB_DATA_NAV;
				}
			}

		/* read attitude record */
		else if (store->record_id == MBF_3DDEPTHP_RECORD_ATTITUDE)
			{
			/* read the attitude data */
			read_len = (size_t) (4 * sizeof(double));
			status = mb_fileio_get(verbose, mbio_ptr, (void *) buffer, &read_len, error);
			
			/* decode the data */
			if (status == MB_SUCCESS)
				{
				index = 0;
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->att_time_d)); index += sizeof(double);
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->att_roll)); index += sizeof(double);
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->att_pitch)); index += sizeof(double);
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->att_heave)); index += sizeof(double);
			
				store->kind = MB_DATA_ATTITUDE;
				}
			}

		/* read heading record */
		else if (store->record_id == MBF_3DDEPTHP_RECORD_HEADING)
			{
			/* read the heading data */
			read_len = (size_t) (2 * sizeof(double));
			status = mb_fileio_get(verbose, mbio_ptr, (void *) buffer, &read_len, error);
			
			/* decode the data */
			if (status == MB_SUCCESS)
				{
				index = 0;
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->hdg_time_d)); index += sizeof(double);
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->hdg_heading)); index += sizeof(double);
			
				store->kind = MB_DATA_HEADING;
				}
			}

		/* read sensordepth record */
		else if (store->record_id == MBF_3DDEPTHP_RECORD_SENSORDEPTH)
			{
			/* read the sensordepth data */
			read_len = (size_t) (2 * sizeof(double));
			status = mb_fileio_get(verbose, mbio_ptr, (void *) buffer, &read_len, error);
			
			/* decode the data */
			if (status == MB_SUCCESS)
				{
				index = 0;
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->sdp_time_d)); index += sizeof(double);
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->sdp_sensordepth)); index += sizeof(double);
			
				store->kind = MB_DATA_SONARDEPTH;
				}
			}

			
		/* read raw LIDAR scan record */
		else if (store->record_id == MBF_3DDEPTHP_RECORD_RAWLIDAR)
			{
			/* read the next scan header */
			read_len = (size_t)MBF_3DDEPTHP_VERSION_1_1_RAWSCANHEADER_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (void *) buffer, &read_len, error);
		
			/* decode the data */
			if (status == MB_SUCCESS)
				{
				store->current_scan++;
				index = 0;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->year)); index += 2;
				store->month = (mb_u_char) buffer[index]; index++;
				store->day = (mb_u_char) buffer[index]; index++;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->days_since_jan_1)); index += 2;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->hour)); index += 2;
				store->minutes = (mb_u_char) buffer[index]; index++;
				store->seconds = (mb_u_char) buffer[index]; index++;
				mb_get_binary_int(MB_YES, (void *) &buffer[index], &(store->nanoseconds)); index += 4;
				mb_get_binary_int(MB_YES, (void *) &buffer[index], &(store->num_pulses)); index += 4;
				store->bathymetry_calculated = MB_NO;
//fprintf(stderr,"   %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%9.9d\n",
//store->year,store->month,store->day,store->hour,store->minutes,store->seconds,store->nanoseconds);

				store->time_d = 0.0;
				store->navlon = 0.0;
				store->navlat = 0.0;
				store->sensordepth = 0.0;
				store->heading = 0.0;
				store->roll = 0.0;
				store->pitch = 0.0;
				store->speed = 0.0;
				
				/* read all of the pulses */
				for (i=0;i<store->num_pulses;i++)
					{
					/* read the next pulse */
					read_len = (size_t)MBF_3DDEPTHP_VERSION_1_1_RAWPULSE_SIZE;
					status = mb_fileio_get(verbose, mbio_ptr, (void *) buffer, &read_len, error);
				
					/* if read ok then get values */
					if (status == MB_SUCCESS)
						{
						pulse = (struct mbsys_3datdepthlidar_pulse_struct *)&store->pulses[i];
						index = 0;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->range)); index += 4;
						mb_get_binary_short(MB_YES, (void *) &buffer[index], &(pulse->amplitude)); index += 2;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->snr)); index += 4;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->cross_track_angle)); index += 4;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->forward_track_angle)); index += 4;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->cross_track_offset)); index += 4;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->forward_track_offset)); index += 4;
						mb_get_binary_int(MB_YES, (void *) &buffer[index], &(pulse->pulse_time_offset)); index += 4;
						pulse->saturated = buffer[index]; index++;
						
						pulse->time_d = 0.0;
						pulse->beamflag = MB_FLAG_NULL;
						pulse->acrosstrack = 0.0;
						pulse->alongtrack = 0.0;
						pulse->depth = 0.0;
						pulse->navlon = 0.0;
						pulse->navlat = 0.0;
						pulse->sensordepth = 0.0;
						pulse->heading = 0.0;
						pulse->roll = 0.0;
						pulse->pitch = 0.0;
						}
					}
				for (i=store->num_pulses;i<store->counts_per_scan;i++)
					{
					pulse = (struct mbsys_3datdepthlidar_pulse_struct *)&store->pulses[i];
					pulse->range = 0.0;
					pulse->amplitude = 0;
					pulse->snr = 0.0;
					pulse->cross_track_angle = 0.0;
					pulse->forward_track_angle = 0.0;
					pulse->cross_track_offset = 0.0;
					pulse->forward_track_offset = 0.0;
					pulse->pulse_time_offset = 0;
					pulse->saturated = 0;
					pulse->time_d = 0.0;
					pulse->beamflag = MB_FLAG_NULL;
					pulse->acrosstrack = 0.0;
					pulse->alongtrack = 0.0;
					pulse->depth = 0.0;
					pulse->navlon = 0.0;
					pulse->navlat = 0.0;
					pulse->sensordepth = 0.0;
					pulse->heading = 0.0;
					pulse->roll = 0.0;
					pulse->pitch = 0.0;
					}
				}
			
			store->kind = MB_DATA_DATA;
			}
			
		/* read processed LIDAR scan record */
		else if (store->record_id == MBF_3DDEPTHP_RECORD_LIDAR)
			{			
			/* read the next scan header */
			read_len = (size_t)MBF_3DDEPTHP_VERSION_1_1_SCANHEADER_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (void *) buffer, &read_len, error);
		
			/* decode the data */
			if (status == MB_SUCCESS)
				{
				store->current_scan++;
				index = 0;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->year)); index += 2;
				store->month = (mb_u_char) buffer[index]; index++;
				store->day = (mb_u_char) buffer[index]; index++;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->days_since_jan_1)); index += 2;
				mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->hour)); index += 2;
				store->minutes = (mb_u_char) buffer[index]; index++;
				store->seconds = (mb_u_char) buffer[index]; index++;
				mb_get_binary_int(MB_YES, (void *) &buffer[index], &(store->nanoseconds)); index += 4;
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->time_d)); index += 8;
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->navlon)); index += 8;
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->navlat)); index += 8;
				mb_get_binary_double(MB_YES, (void *) &buffer[index], &(store->sensordepth)); index += 8;
				mb_get_binary_float(MB_YES, (void *) &buffer[index], &(store->heading)); index += 4;
				mb_get_binary_float(MB_YES, (void *) &buffer[index], &(store->roll)); index += 4;
				mb_get_binary_float(MB_YES, (void *) &buffer[index], &(store->pitch)); index += 4;
				mb_get_binary_float(MB_YES, (void *) &buffer[index], &(store->speed)); index += 4;
				mb_get_binary_int(MB_YES, (void *) &buffer[index], &(store->num_pulses)); index += 4;
				store->bathymetry_calculated = MB_YES;
//fprintf(stderr,"   %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%9.9d\n",
//store->year,store->month,store->day,store->hour,store->minutes,store->seconds,store->nanoseconds);
				}
				
			/* read all of the pulses */
			if (status == MB_SUCCESS)
				{
				for (i=0;i<store->num_pulses;i++)
					{
					/* read the next pulse */
					read_len = (size_t)MBF_3DDEPTHP_VERSION_1_1_PULSE_SIZE;
					status = mb_fileio_get(verbose, mbio_ptr, (void *) buffer, &read_len, error);
				
					/* if read ok then get values */
					if (status == MB_SUCCESS)
						{
						pulse = (struct mbsys_3datdepthlidar_pulse_struct *)&store->pulses[i];
						index = 0;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->range)); index += 4;
						mb_get_binary_short(MB_YES, (void *) &buffer[index], &(pulse->amplitude)); index += 2;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->snr)); index += 4;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->cross_track_angle)); index += 4;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->forward_track_angle)); index += 4;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->cross_track_offset)); index += 4;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->forward_track_offset)); index += 4;
						mb_get_binary_int(MB_YES, (void *) &buffer[index], &(pulse->pulse_time_offset)); index += 4;
						pulse->saturated = buffer[index]; index++;
						mb_get_binary_double(MB_YES, (void *) &buffer[index], &(pulse->time_d)); index += 8;
						pulse->beamflag = buffer[index]; index++;
						mb_get_binary_double(MB_YES, (void *) &buffer[index], &(pulse->acrosstrack)); index += 8;
						mb_get_binary_double(MB_YES, (void *) &buffer[index], &(pulse->alongtrack)); index += 8;
						mb_get_binary_double(MB_YES, (void *) &buffer[index], &(pulse->depth)); index += 8;
						mb_get_binary_double(MB_YES, (void *) &buffer[index], &(pulse->navlon)); index += 8;
						mb_get_binary_double(MB_YES, (void *) &buffer[index], &(pulse->navlat)); index += 8;
						mb_get_binary_double(MB_YES, (void *) &buffer[index], &(pulse->sensordepth)); index += 8;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->heading)); index += 4;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->roll)); index += 4;
						mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->pitch)); index += 4;
						}
					}
				for (i=store->num_pulses;i<store->counts_per_scan;i++)
					{
					pulse = (struct mbsys_3datdepthlidar_pulse_struct *)&store->pulses[i];
					pulse->range = 0.0;
					pulse->amplitude = 0;
					pulse->snr = 0.0;
					pulse->cross_track_angle = 0.0;
					pulse->forward_track_angle = 0.0;
					pulse->cross_track_offset = 0.0;
					pulse->forward_track_offset = 0.0;
					pulse->pulse_time_offset = 0;
					pulse->saturated = 0;
					pulse->time_d = 0.0;
					pulse->beamflag = MB_FLAG_NULL;
					pulse->acrosstrack = 0.0;
					pulse->alongtrack = 0.0;
					pulse->depth = 0.0;
					pulse->navlon = 0.0;
					pulse->navlat = 0.0;
					pulse->sensordepth = 0.0;
					pulse->heading = 0.0;
					pulse->roll = 0.0;
					pulse->pitch = 0.0;
					}
				}
			
			store->kind = MB_DATA_DATA;
			}
		}
		
	/* else read next record in the obsolete format version 1.0
			- LIDAR scans only with no record id's */
	else if (status == MB_SUCCESS && done == MB_NO
		&& store->file_version == 1 && store->sub_version == 0)
		{
//fprintf(stderr,"READ NEXT RECORD version %d %d\n",store->file_version,store->sub_version);
			
		/* read the next scan header */
		if (mb_io_ptr->save2 == MB_NO)
			{
//fprintf(stderr,"No save, read full scan\n");
			read_len = (size_t)MBF_3DDEPTHP_VERSION_1_0_SCANHEADER_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (void *) buffer, &read_len, error);
			}
		else
			{
//fprintf(stderr,"Bytes saved, read rest of scan\n");
			for (i=0;i<4;i++)
				buffer[i] = mb_io_ptr->save_label[i];
			read_len = (size_t)(MBF_3DDEPTHP_VERSION_1_0_SCANHEADER_SIZE - 4);
			status = mb_fileio_get(verbose, mbio_ptr, (void *) &buffer[4], &read_len, error);
			mb_io_ptr->save2 = MB_NO;
			}
	
		/* if read ok then get values */
		if (status == MB_SUCCESS)
			{
			newscancheck = (unsigned int *) buffer;
			newscancheckvalue = *newscancheck;
			
			store->current_scan++;
			index = 0;
			store->record_id = MBF_3DDEPTHP_RECORD_RAWLIDAR;
			mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->year)); index += 2;
			store->month = (mb_u_char) buffer[index]; index++;
			store->day = (mb_u_char) buffer[index]; index++;
			mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->days_since_jan_1)); index += 2;
			mb_get_binary_short(MB_YES, (void *) &buffer[index], &(store->hour)); index += 2;
			store->minutes = (mb_u_char) buffer[index]; index++;
			store->seconds = (mb_u_char) buffer[index]; index++;
			mb_get_binary_int(MB_YES, (void *) &buffer[index], &(store->nanoseconds)); index += 4;
//fprintf(stderr,"DATE: %d %d %d  %d  %d %d %d  %d\n",store->year,store->month,store->day,store->days_since_jan_1,store->hour,store->minutes,store->seconds,store->nanoseconds);
			store->bathymetry_calculated = MB_NO;
			
			/* fix timestamp problem with the original data files */
			if (store->year < 2000)
				{
				store->year += 1900;
				store->month++;
				}
			}
			
		/* read all of the pulses */
		if (status == MB_SUCCESS)
			{
			store->num_pulses = 0;
			done = MB_NO;
			while (done == MB_NO)
				{
				/* read the next four bytes */
				read_len = (size_t)4;
				status = mb_fileio_get(verbose, mbio_ptr, (void *) buffer, &read_len, error);
				
				/* if end of file reached handle it gracefully
				   else check to see if the new pulse is valid or is really the start of a new scan
				   by checking the first four bytes */
				newscancheck = (unsigned int *) buffer;
//fprintf(stderr,"Pulse %d header:%x  %f\n",store->num_pulses,*newscancheck, *((float *)newscancheck));
				if (status == MB_FAILURE)
					{
					done = MB_YES;
					status = MB_SUCCESS;
					*error = MB_ERROR_NO_ERROR;
					}
				else if (*newscancheck == newscancheckvalue)
					{
					done = MB_YES;
					mb_io_ptr->save2 = MB_YES;
					for (i=0;i<4;i++)
						mb_io_ptr->save_label[i] = buffer[i];
					}
				else
					{
					read_len = (size_t)(MBF_3DDEPTHP_VERSION_1_0_PULSE_SIZE - 4);
					status = mb_fileio_get(verbose, mbio_ptr, (void *) &buffer[4], &read_len, error);							
					if (status == MB_FAILURE)
						done = MB_YES;
					}
			
				/* if read ok and consistent with new pulse then get values */
				if (status == MB_SUCCESS && done == MB_NO)
					{
					pulse = (struct mbsys_3datdepthlidar_pulse_struct *)&store->pulses[store->num_pulses];
					index = 0;
					mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->range)); index += 4;
					mb_get_binary_short(MB_YES, (void *) &buffer[index], &(pulse->amplitude)); index += 2;
					mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->snr)); index += 4;
					mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->cross_track_angle)); index += 4;
					mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->forward_track_angle)); index += 4;
					mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->cross_track_offset)); index += 4;
					mb_get_binary_float(MB_YES, (void *) &buffer[index], &(pulse->forward_track_offset)); index += 4;
					mb_get_binary_int(MB_YES, (void *) &buffer[index], &(pulse->pulse_time_offset)); index += 4;
					pulse->saturated = buffer[index]; index++;

					pulse->time_d = 0.0;
					pulse->beamflag = MB_FLAG_NULL;
					pulse->acrosstrack = 0.0;
					pulse->alongtrack = 0.0;
					pulse->depth = 0.0;
					pulse->navlon = 0.0;
					pulse->navlat = 0.0;
					pulse->sensordepth = 0.0;
					pulse->heading = 0.0;
					pulse->roll = 0.0;
					pulse->pitch = 0.0;
					
					store->num_pulses++;
					if (store->num_pulses >= store->counts_per_scan)
						done = MB_YES;
					}
				}
			for (i=store->num_pulses;i<store->counts_per_scan;i++)
				{
				pulse = (struct mbsys_3datdepthlidar_pulse_struct *)&store->pulses[i];
				pulse->range = 0.0;
				pulse->amplitude = 0;
				pulse->snr = 0.0;
				pulse->cross_track_angle = 0.0;
				pulse->forward_track_angle = 0.0;
				pulse->cross_track_offset = 0.0;
				pulse->forward_track_offset = 0.0;
				pulse->pulse_time_offset = 0;
				pulse->saturated = 0;
				pulse->time_d = 0.0;
				pulse->beamflag = MB_FLAG_NULL;
				pulse->acrosstrack = 0.0;
				pulse->alongtrack = 0.0;
				pulse->depth = 0.0;
				pulse->navlon = 0.0;
				pulse->navlat = 0.0;
				pulse->sensordepth = 0.0;
				pulse->heading = 0.0;
				pulse->roll = 0.0;
				pulse->pitch = 0.0;
				}
			
			store->kind = MB_DATA_DATA;
//fprintf(stderr,"READ %d pulses, save status:%d current_scan:%d\n",store->num_pulses,mb_io_ptr->save2,store->current_scan);
			}
		}

	/* print out status info */
	if (verbose >= 3 && status == MB_SUCCESS)
		mbsys_3datdepthlidar_print_store(verbose, store_ptr, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_3ddepthp_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_3ddepthp_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3datdepthlidar_struct *store;
	struct mbsys_3datdepthlidar_pulse_struct *pulse;
	int	*file_header_readwritten;
	char	buffer[MBF_3DDEPTHP_BUFFER_SIZE];
	size_t	write_len;
	size_t	index;
	unsigned short record_id;
	unsigned short magic_number;
	unsigned short file_version;
	unsigned short sub_version;
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


	/* check for non-null pointers */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_3datdepthlidar_struct *) store_ptr;

	/* get saved values */
	file_header_readwritten = (int *) &mb_io_ptr->save1;

	/* print output debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Data record kind in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg4       kind:       %d\n",store->kind);
		}
	
	/* set status */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* if first write then write the magic number file header */
	if (*file_header_readwritten == MB_NO)
		{
		/* write magic_number */
		magic_number = MBF_3DDEPTHP_MAGICNUMBER;
		
		/* encode the header data */
		index = 0;
		mb_put_binary_short(MB_YES, magic_number, &buffer[index]); index += 2;

		/* write file header from buffer */
		write_len = (size_t)2;
		status = mb_fileio_put(verbose, mbio_ptr, (void *) buffer, &write_len, error);
		
		/* set that header has been written */
		*file_header_readwritten = MB_YES;
		}

	/* write next record */
	if (status == MB_SUCCESS)
		{
		/* write comment record */
		if (store->kind == MB_DATA_PARAMETER)
			{
			/* encode the data */
			index = 0;
			record_id = MBF_3DDEPTHP_RECORD_PARAMETER;
			file_version = 1;
			sub_version = 1;
			mb_put_binary_short(MB_YES, record_id, &buffer[index]); index += 2;
			mb_put_binary_short(MB_YES, file_version, &buffer[index]); index += 2;
			mb_put_binary_short(MB_YES, sub_version, &buffer[index]); index += 2;
			mb_put_binary_short(MB_YES, store->scan_type, &buffer[index]); index += 2;
			mb_put_binary_float(MB_YES, store->cross_track_angle_start, &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, store->cross_track_angle_end, &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, store->forward_track_angle_start, &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, store->forward_track_angle_end, &buffer[index]); index += 4;
			mb_put_binary_short(MB_YES, store->counts_per_scan, &buffer[index]); index += 2;
			mb_put_binary_short(MB_YES, store->counts_per_cross_track, &buffer[index]); index += 2;
			mb_put_binary_short(MB_YES, store->counts_per_forward_track, &buffer[index]); index += 2;
			mb_put_binary_short(MB_YES, store->scanner_efficiency, &buffer[index]); index += 2;
			mb_put_binary_short(MB_YES, store->scans_per_file, &buffer[index]); index += 2;
			mb_put_binary_int(MB_YES, store->scan_count, &buffer[index]); index += 4;
	
			/* write file header from buffer */
			write_len = (size_t)MBF_3DDEPTHP_VERSION_1_1_PARAMETER_SIZE;
			status = mb_fileio_put(verbose, mbio_ptr, (void *) buffer, &write_len, error);
			}

		/* write comment record */
		else if (store->kind == MB_DATA_COMMENT)
			{
			/* encode the data */
			index = 0;
			store->record_id = MBF_3DDEPTHP_RECORD_COMMENT;
			mb_put_binary_short(MB_YES, store->record_id, &buffer[index]); index += 2;
			mb_put_binary_short(MB_YES, store->comment_len, &buffer[index]); index += 2;
			
			/* write comment record */
			write_len = (size_t) index;
			status = mb_fileio_put(verbose, mbio_ptr, (void *) buffer, &write_len, error);
			write_len = (size_t) store->comment_len;
			status = mb_fileio_put(verbose, mbio_ptr, (void *) &store->comment, &write_len, error);
			}

		/* write position record */
		else if (store->kind == MB_DATA_NAV)
			{
			/* encode the data */
			index = 0;
			store->record_id = MBF_3DDEPTHP_RECORD_POSITION;
			mb_put_binary_short(MB_YES, store->record_id, &buffer[index]); index += 2;
			mb_put_binary_double(MB_YES, store->pos_time_d, &buffer[index]); index += sizeof(double);
			mb_put_binary_double(MB_YES, store->pos_longitude, &buffer[index]); index += sizeof(double);
			mb_put_binary_double(MB_YES, store->pos_latitude, &buffer[index]); index += sizeof(double);
			
			/* write position record */
			write_len = (size_t) index;
			status = mb_fileio_put(verbose, mbio_ptr, (void *) buffer, &write_len, error);
			}

		/* write attitude record */
		else if (store->kind == MB_DATA_ATTITUDE)
			{
			/* encode the data */
			index = 0;
			store->record_id = MBF_3DDEPTHP_RECORD_ATTITUDE;
			mb_put_binary_short(MB_YES, store->record_id, &buffer[index]); index += 2;
			mb_put_binary_double(MB_YES, store->att_time_d, &buffer[index]); index += sizeof(double);
			mb_put_binary_double(MB_YES, store->att_roll, &buffer[index]); index += sizeof(double);
			mb_put_binary_double(MB_YES, store->att_pitch, &buffer[index]); index += sizeof(double);
			mb_put_binary_double(MB_YES, store->att_heave, &buffer[index]); index += sizeof(double);
			
			/* write attitude record */
			write_len = (size_t) index;
			status = mb_fileio_put(verbose, mbio_ptr, (void *) buffer, &write_len, error);
			}

		/* write heading record */
		else if (store->kind == MB_DATA_HEADING)
			{
			/* encode the data */
			index = 0;
			store->record_id = MBF_3DDEPTHP_RECORD_HEADING;
			mb_put_binary_short(MB_YES, store->record_id, &buffer[index]); index += 2;
			mb_put_binary_double(MB_YES, store->hdg_time_d, &buffer[index]); index += sizeof(double);
			mb_put_binary_double(MB_YES, store->hdg_heading, &buffer[index]); index += sizeof(double);
			
			/* write heading record */
			write_len = (size_t) index;
			status = mb_fileio_put(verbose, mbio_ptr, (void *) buffer, &write_len, error);
			}

		/* write sensordepth record */
		else if (store->kind == MB_DATA_SONARDEPTH)
			{
			/* encode the data */
			index = 0;
			store->record_id = MBF_3DDEPTHP_RECORD_SENSORDEPTH;
			mb_put_binary_short(MB_YES, store->record_id, &buffer[index]); index += 2;
			mb_put_binary_double(MB_YES, store->sdp_time_d, &buffer[index]); index += sizeof(double);
			mb_put_binary_double(MB_YES, store->sdp_sensordepth, &buffer[index]); index += sizeof(double);
			
			/* write sensordepth record */
			write_len = (size_t) index;
			status = mb_fileio_put(verbose, mbio_ptr, (void *) buffer, &write_len, error);
			}

		/* write LIDAR scan record */
		else if (store->kind == MB_DATA_DATA)
			{
			/* encode the data */
			index = 0;
			store->record_id = MBF_3DDEPTHP_RECORD_LIDAR;
			mb_put_binary_short(MB_YES, store->record_id, &buffer[index]); index += 2;
			mb_put_binary_short(MB_YES, store->year, &buffer[index]); index += 2;
			buffer[index] = (mb_u_char) store->month; index++;
			buffer[index] = (mb_u_char) store->day; index++;
			mb_put_binary_short(MB_YES, store->days_since_jan_1, &buffer[index]); index += 2;
			mb_put_binary_short(MB_YES, store->hour, &buffer[index]); index += 2;
			buffer[index] = (mb_u_char) store->minutes; index++;
			buffer[index] = (mb_u_char) store->seconds; index++;
			mb_put_binary_int(MB_YES, store->nanoseconds, &buffer[index]); index += 4;
			mb_put_binary_double(MB_YES, store->time_d, &buffer[index]); index += 8;
			mb_put_binary_double(MB_YES, store->navlon, &buffer[index]); index += 8;
			mb_put_binary_double(MB_YES, store->navlat, &buffer[index]); index += 8;
			mb_put_binary_double(MB_YES, store->sensordepth, &buffer[index]); index += 8;
			mb_put_binary_float(MB_YES, store->heading, &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, store->roll, &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, store->pitch, &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, store->speed, &buffer[index]); index += 4;
			mb_put_binary_int(MB_YES, store->num_pulses, &buffer[index]); index += 4;
			
			/* write LIDAR scan record header */
			write_len = (size_t) index;
			status = mb_fileio_put(verbose, mbio_ptr, (void *) buffer, &write_len, error);
			
			/* write LIDAR scan pulses */
			if (status == MB_SUCCESS)
				{
				for (i=0;i<store->num_pulses;i++)
					{
					/* encode the data */
					pulse = (struct mbsys_3datdepthlidar_pulse_struct *)&store->pulses[i];
					index = 0;
					mb_put_binary_float(MB_YES, pulse->range, &buffer[index]); index += 4;
					mb_put_binary_short(MB_YES, pulse->amplitude, &buffer[index]); index += 2;
					mb_put_binary_float(MB_YES, pulse->snr, &buffer[index]); index += 4;
					mb_put_binary_float(MB_YES, pulse->cross_track_angle, &buffer[index]); index += 4;
					mb_put_binary_float(MB_YES, pulse->forward_track_angle, &buffer[index]); index += 4;
					mb_put_binary_float(MB_YES, pulse->cross_track_offset, &buffer[index]); index += 4;
					mb_put_binary_float(MB_YES, pulse->forward_track_offset, &buffer[index]); index += 4;
					mb_put_binary_int(MB_YES, pulse->pulse_time_offset, &buffer[index]); index += 4;
					buffer[index] = pulse->saturated; index++;
					mb_put_binary_double(MB_YES, pulse->time_d, &buffer[index]); index += 8;
					buffer[index] = pulse->beamflag; index++;
					mb_put_binary_double(MB_YES, pulse->acrosstrack, &buffer[index]); index += 8;
					mb_put_binary_double(MB_YES, pulse->alongtrack, &buffer[index]); index += 8;
					mb_put_binary_double(MB_YES, pulse->depth, &buffer[index]); index += 8;
					mb_put_binary_double(MB_YES, pulse->navlon, &buffer[index]); index += 8;
					mb_put_binary_double(MB_YES, pulse->navlat, &buffer[index]); index += 8;
					mb_put_binary_double(MB_YES, pulse->sensordepth, &buffer[index]); index += 8;
					mb_put_binary_float(MB_YES, pulse->heading, &buffer[index]); index += 4;
					mb_put_binary_float(MB_YES, pulse->roll, &buffer[index]); index += 4;
					mb_put_binary_float(MB_YES, pulse->pitch, &buffer[index]); index += 4;
			
					/* write LIDAR scan pulse record */
					write_len = (size_t) index;
					status = mb_fileio_put(verbose, mbio_ptr, (void *) buffer, &write_len, error);
					}
				}
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
