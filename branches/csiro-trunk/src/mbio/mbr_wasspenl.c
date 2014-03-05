/*--------------------------------------------------------------------
 *    The MB-system:	mbr_wasspenl.c	1/27/2014
 *	$Id$
 *
 *    Copyright (c) 2014-2014 by
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
 * mbr_wasspenl.c contains the functions for reading and writing
 * multibeam data in the WASSPENL format.
 * These functions include:
 *   mbr_alm_wasspenl	- allocate read/write memory
 *   mbr_dem_wasspenl	- deallocate read/write memory
 *   mbr_rt_wasspenl	- read and translate data
 *   mbr_wt_wasspenl	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	January 27, 2014
 * $Log: mbr_wasspenl.c,v $
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
#include "mbsys_wassp.h"

/* include for byte swapping */
#include "mb_swap.h"

/* turn on debug statements here */
/* #define MBR_WASSPENLDEBUG 1 */

/* essential function prototypes */
int mbr_register_wasspenl(int verbose, void *mbio_ptr,
		int *error);
int mbr_info_wasspenl(int verbose,
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
int mbr_alm_wasspenl(int verbose, void *mbio_ptr, int *error);
int mbr_dem_wasspenl(int verbose, void *mbio_ptr, int *error);
int mbr_rt_wasspenl(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_wasspenl(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wasspenl_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wasspenl_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wasspenl_rd_genbathy(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_wasspenl_rd_corbathy(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_wasspenl_rd_rawsonar(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_wasspenl_rd_gen_sens(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_wasspenl_rd_nvupdate(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_wasspenl_rd_wcd_navi(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_wasspenl_rd_sys_cfg1(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_wasspenl_rd_mcomment(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_wasspenl_rd_unknown1(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_wasspenl_wr_genbathy(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_wasspenl_wr_corbathy(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_wasspenl_wr_rawsonar(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_wasspenl_wr_gen_sens(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_wasspenl_wr_nvupdate(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_wasspenl_wr_wcd_navi(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_wasspenl_wr_sys_cfg1(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_wasspenl_wr_mcomment(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_wasspenl_wr_unknown1(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);

int mbr_reson7kr_wr_reference(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_wasspenl(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_wasspenl";
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
	status = mbr_info_wasspenl(verbose,
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_wasspenl;
	mb_io_ptr->mb_io_format_free = &mbr_dem_wasspenl;
	mb_io_ptr->mb_io_store_alloc = &mbsys_wassp_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_wassp_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_wasspenl;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_wasspenl;
	mb_io_ptr->mb_io_dimensions = &mbsys_wassp_dimensions;
	mb_io_ptr->mb_io_pingnumber = &mbsys_wassp_pingnumber;
	mb_io_ptr->mb_io_sonartype = &mbsys_wassp_sonartype;
	mb_io_ptr->mb_io_sidescantype = NULL;
	mb_io_ptr->mb_io_extract = &mbsys_wassp_extract;
	mb_io_ptr->mb_io_insert = &mbsys_wassp_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_wassp_extract_nav;
	mb_io_ptr->mb_io_extract_nnav = NULL;
	mb_io_ptr->mb_io_insert_nav = &mbsys_wassp_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_wassp_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_wassp_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_wassp_detects;
	mb_io_ptr->mb_io_gains = &mbsys_wassp_gains;
	mb_io_ptr->mb_io_copyrecord = &mbsys_wassp_copy;
	mb_io_ptr->mb_io_extract_rawss = NULL;
	mb_io_ptr->mb_io_insert_rawss = NULL;
	mb_io_ptr->mb_io_extract_segytraceheader = NULL;
	mb_io_ptr->mb_io_extract_segy = NULL;
	mb_io_ptr->mb_io_insert_segy = NULL;
	mb_io_ptr->mb_io_ctd = NULL;
	mb_io_ptr->mb_io_ancilliarysensor = NULL;

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
		fprintf(stderr,"dbg2       extract_segytraceheader: %p\n",(void *)mb_io_ptr->mb_io_extract_segytraceheader);
		fprintf(stderr,"dbg2       extract_segy:       %p\n",(void *)mb_io_ptr->mb_io_extract_segy);
		fprintf(stderr,"dbg2       insert_segy:        %p\n",(void *)mb_io_ptr->mb_io_insert_segy);
		fprintf(stderr,"dbg2       copyrecord:         %p\n",(void *)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_wasspenl(int verbose,
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
	char	*function_name = "mbr_info_wasspenl";
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
	*system = MB_SYS_WASSP;
	*beams_bath_max = MBSYS_WASSP_MAX_BEAMS;
	*beams_amp_max = MBSYS_WASSP_MAX_BEAMS;
	*pixels_ss_max = MBSYS_WASSP_MAX_PIXELS;
	strncpy(format_name, "WASSPENL", MB_NAME_LENGTH);
	strncpy(system_name, "WASSP", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_WASSPENL\nInformal Description: WASSP Multibeam Vendor Format\nAttributes:           WASSP multibeams, \n                      bathymetry and amplitude,\n		      122 or 244 beams, binary, Electronic Navigation Ltd.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_SINGLE;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 4.0;
	*beamwidth_ltrack = 4.0;

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
int mbr_alm_wasspenl(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_wasspenl";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	char	**bufferptr;
	char	*buffer;
	int	*bufferalloc;

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
	status = mbsys_wassp_alloc(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);
	
	/* allocate starting memory for data record buffer */
	bufferptr = (char **) &mb_io_ptr->saveptr1;
	buffer = (char *) *bufferptr;
	bufferalloc = (int *) &mb_io_ptr->save6;
	*bufferptr = NULL;
	*bufferalloc = 0;
	if (status == MB_SUCCESS)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, MBSYS_WASSP_BUFFER_STARTSIZE,
					(void **)bufferptr, error);
		if (status == MB_SUCCESS)
			*bufferalloc = MBSYS_WASSP_BUFFER_STARTSIZE;
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
int mbr_dem_wasspenl(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_wasspenl";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	char	**bufferptr;
	char	*buffer;
	int	*bufferalloc;

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

	/* deallocate memory for reading/writing buffer */
	bufferptr = (char **) &mb_io_ptr->saveptr1;
	buffer = (char *) *bufferptr;
	bufferalloc = (int *) &mb_io_ptr->save6;
	status = mb_freed(verbose,__FILE__,__LINE__,(void **)bufferptr,error);
	*bufferalloc = 0;

	/* deallocate memory for data descriptor */
	status = mbsys_wassp_deall(
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
int mbr_rt_wasspenl(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_wasspenl";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_wassp_struct *store;

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

#ifdef MBR_WASSPENLDEBUG
	fprintf(stderr,"About to call mbr_wasspenl_rd_data...\n");
#endif

	/* read next data from file */
	status = mbr_wasspenl_rd_data(verbose,mbio_ptr,store_ptr,error);

	/* get pointers to data structures */
	store = (struct mbsys_wassp_struct *) store_ptr;

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

#ifdef MBR_WASSPENLDEBUG
	fprintf(stderr,"Done with mbr_wasspenl_rd_data: status:%d error:%d record kind:%d\n", status, *error, store->kind);
#endif

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
int mbr_wt_wasspenl(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_wasspenl";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_wassp_struct *store;

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
	store = (struct mbsys_wassp_struct *) store_ptr;

#ifdef MBR_WASSPENLDEBUG
	fprintf(stderr,"About to call mbr_wasspenl_wr_data record kind:%d\n", store->kind);
#endif

	/* write next data to file */
	status = mbr_wasspenl_wr_data(verbose,mbio_ptr,store_ptr,error);

#ifdef MBR_WASSPENLDEBUG
	fprintf(stderr,"Done with mbr_wasspenl_wr_data: status:%d error:%d\n", status, *error);
#endif

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
int mbr_wasspenl_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wasspenl_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_genbathy_struct *genbathy;
        struct mbsys_wassp_corbathy_struct *corbathy;
        struct mbsys_wassp_rawsonar_struct *rawsonar;
        struct mbsys_wassp_gen_sens_struct *gen_sens;
        struct mbsys_wassp_nvupdate_struct *nvupdate;
        struct mbsys_wassp_wcd_navi_struct *wcd_navi;
	struct mbsys_wassp_sys_cfg1_struct *sys_cfg1;
	struct mbsys_wassp_mcomment_struct *mcomment;
	char	**bufferptr;
	char	*buffer;
	int	*bufferalloc;
	unsigned int *synctest;
	char	recordid[12];
	size_t	read_len;
	int	skip;
	unsigned int *record_size;
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

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
        genbathy = (struct mbsys_wassp_genbathy_struct *) &(store->genbathy);
        corbathy = (struct mbsys_wassp_corbathy_struct *) &(store->corbathy);
        rawsonar = (struct mbsys_wassp_rawsonar_struct *) &(store->rawsonar);
        gen_sens = (struct mbsys_wassp_gen_sens_struct *) &(store->gen_sens);
        nvupdate = (struct mbsys_wassp_nvupdate_struct *) &(store->nvupdate);
        wcd_navi = (struct mbsys_wassp_wcd_navi_struct *) &(store->wcd_navi);
        sys_cfg1 = (struct mbsys_wassp_sys_cfg1_struct *) &(store->sys_cfg1);
        mcomment = (struct mbsys_wassp_mcomment_struct *) &(store->mcomment);

	bufferptr = (char **) &mb_io_ptr->saveptr1;
	buffer = (char *) *bufferptr;
	bufferalloc = (int *) &mb_io_ptr->save6;
	synctest = (unsigned int *) buffer;
	record_size = (unsigned int *)&buffer[4];

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* loop over reading data until a record is ready for return */
	done = MB_NO;
	*error = MB_ERROR_NO_ERROR;
	while (done == MB_NO)
		{
		/* read next record header into buffer */
		read_len = (size_t)16;
		status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);

		/* check header - if not a good header read a byte
			at a time until a good header is found */
		skip = 0;
		while (status == MB_SUCCESS
			&& *synctest != MBSYS_WASSP_SYNC)
			{
			/* get next byte */
			for (i=0;i<15;i++)
			    buffer[i] = buffer[i+1];
			read_len = (size_t)1;
			status = mb_fileio_get(verbose, mbio_ptr, &buffer[15],
					   &read_len, error);
			skip++;
			}
		    
		/* get record id string */
		memcpy((void *)recordid, (void *)&buffer[8], (size_t)8);
		recordid[9] = '\0';
#ifdef MBR_WASSPENLDEBUG
	fprintf(stderr,"Found sync - skip:%d record:%s\n", skip,recordid);
#endif

		/* report problem */
		if (skip > 0 && verbose >= 0)
		    	{
			fprintf(stderr,
				"\nThe MBF_WASSPENL module skipped data between identified\n"
				"data records. Something is broken, most probably the data...\n"
				"However, the data may include a data record type that we\n"
				"haven't seen yet, or there could be an error in the code.\n"
				"If skipped data are reported multiple times, \n"
				"we recommend you send a data sample and problem \n"
				"description to the MB-System team \n"
				"(caress@mbari.org and dale@ldeo.columbia.edu)\n"
				"Have a nice day...\n");
			fprintf(stderr, "MBF_WASSPENL skipped %d bytes before record %s\n",
					skip, recordid);
		    }

		/* allocate memory to read rest of record if necessary */
		if (*bufferalloc < *record_size)
			{
			status = mb_reallocd(verbose, __FILE__, __LINE__, *record_size,
						(void **)bufferptr, error);
			if (status != MB_SUCCESS)
				{
				*bufferalloc = 0;
				done = MB_YES;
				}
			else
				{
				*bufferalloc = *record_size;
				buffer = (char *) *bufferptr;
				}
			}

		/* read the rest of the record */
		if (status == MB_SUCCESS)
			{
			read_len = (size_t)(*record_size - 16);
			status = mb_fileio_get(verbose, mbio_ptr, &buffer[16],
				       &read_len, error);
			}
		
		/* if valid parse the record */
		if (status == MB_SUCCESS)
			{
			/* read GENBATHY record */
			if (strncmp(recordid, "GENBATHY", 8) == 0)
				{
				status = mbr_wasspenl_rd_genbathy(verbose, buffer, store_ptr, error);
				}

			/* read CORBATHY record */
			else if (strncmp(recordid, "CORBATHY", 8) == 0)
				{
				status = mbr_wasspenl_rd_corbathy(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					{
					if (genbathy->ping_number == corbathy->ping_number)
						done = MB_YES;
					else
						{
						status = MB_FAILURE;
						*error = MB_ERROR_UNINTELLIGIBLE;
						done = MB_YES;
						}
					}
				}

			/* read RAWSONAR record */
			else if (strncmp(recordid, "RAWSONAR", 8) == 0)
				{
				status = mbr_wasspenl_rd_rawsonar(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = MB_YES;
				}

			/* read GEN_SENS record */
			else if (strncmp(recordid, "GEN_SENS", 8) == 0)
				{
				status = mbr_wasspenl_rd_gen_sens(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = MB_YES;
				}

			/* read NVUPDATE record */
			else if (strncmp(recordid, "NVUPDATE", 8) == 0)
				{
				status = mbr_wasspenl_rd_nvupdate(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = MB_YES;
				}

			/* read WCD_NAVI record */
			else if (strncmp(recordid, "WCD_NAVI", 8) == 0)
				{
				status = mbr_wasspenl_rd_wcd_navi(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = MB_YES;
				}

			/* read SYS_CFG1 record */
			else if (strncmp(recordid, "SYS_CFG1", 8) == 0)
				{
				status = mbr_wasspenl_rd_sys_cfg1(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = MB_YES;
				}

			/* read MCOMMENT_ record */
			else if (strncmp(recordid, "MCOMMENT", 8) == 0)
				{
				status = mbr_wasspenl_rd_mcomment(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = MB_YES;
				}

			/* read an unknown1 record */
			else
				{
				status = mbr_wasspenl_rd_unknown1(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = MB_YES;
				}
			}

		/* set done if read failure */
		else
			{
			done = MB_YES;
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
int mbr_wasspenl_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wasspenl_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_wassp_struct *store;
	char	**bufferptr;
	char	*buffer;
	int	*bufferalloc;
	int	size;
	size_t	write_len;

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
	store = (struct mbsys_wassp_struct *) store_ptr;

	/* get saved values */
	bufferptr = (char **) &mb_io_ptr->saveptr1;
	buffer = (char *) *bufferptr;
	bufferalloc = (int *) &mb_io_ptr->save6;

	/* write the current data record */
	
	/* write GENBATHY record */
	if (store->kind == MB_DATA_DATA)
		{
		status = mbr_wasspenl_wr_genbathy(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *) *bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);

		status = mbr_wasspenl_wr_corbathy(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *) *bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}

	/* write RAWSONAR record */
	else if (store->kind == MB_DATA_WATER_COLUMN)
		{
		status = mbr_wasspenl_wr_rawsonar(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *) *bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}

	/* write GEN_SENS record */
	else if (store->kind == MB_DATA_GEN_SENS)
		{
		status = mbr_wasspenl_wr_gen_sens(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *) *bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}

	/* write NVUPDATE record */
	else if (store->kind == MB_DATA_NAV)
		{
		status = mbr_wasspenl_wr_nvupdate(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *) *bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}

	/* write WCD_NAVI record */
	else if (store->kind == MB_DATA_WC_PICKS)
		{
		status = mbr_wasspenl_wr_wcd_navi(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *) *bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}

	/* write SYS_CFG1 record */
	else if (store->kind == MB_DATA_PARAMETER)
		{
		status = mbr_wasspenl_wr_sys_cfg1(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *) *bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}

	/* write MCOMMENT_ record */
	else if (store->kind == MB_DATA_COMMENT)
		{
		status = mbr_wasspenl_wr_mcomment(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *) *bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}

	/* write unknown1 record */
	else if (store->kind == MB_DATA_RAW_LINE)
		{
		status = mbr_wasspenl_wr_sys_cfg1(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *) *bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}

#ifdef MBR_WASSPENLDEBUG
	fprintf(stderr,"WASSPENL DATA WRITTEN: type:%d status:%d error:%d\n\n",
	store->kind, status, *error);
#endif

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
int mbr_wasspenl_rd_genbathy(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wasspenl_rd_genbathy";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_genbathy_struct *genbathy;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %p\n",(void *)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	genbathy = &(store->genbathy);

	/* extract the data */
	index = 16;
	mb_get_binary_int(MB_YES, &buffer[index], &(genbathy->version)); index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &(genbathy->msec)); index += 8;
	genbathy->day = buffer[index]; index++;
	genbathy->month = buffer[index]; index++;
	mb_get_binary_short(MB_YES, &buffer[index], &(genbathy->year)); index += 2;
	mb_get_binary_int(MB_YES, &buffer[index], &(genbathy->ping_number)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(genbathy->sonar_model)); index += 4;
	mb_get_binary_long(MB_YES, &buffer[index], &(genbathy->transducer_serial_number)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(genbathy->number_beams)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(genbathy->modeflags)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(genbathy->sampling_frequency)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(genbathy->acoustic_frequency)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(genbathy->tx_power)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(genbathy->pulse_width)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(genbathy->absorption_loss)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(genbathy->spreading_loss)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(genbathy->sample_type)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(genbathy->sound_velocity)); index += 4;
	for (i=0;i<genbathy->number_beams;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(genbathy->detection_point[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(genbathy->rx_angle[i])); index += 4;
		mb_get_binary_int(MB_YES, &buffer[index], &(genbathy->flags[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(genbathy->backscatter[i])); index += 4;
		}
	mb_get_binary_int(MB_YES, &buffer[index], &(genbathy->checksum)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_DATA;

		/* get the time */
		store->time_i[0] = genbathy->year;
		store->time_i[1] = genbathy->month;
		store->time_i[2] = genbathy->day;
		store->time_i[3] = (int)floor(genbathy->msec / 3600000.0);
		store->time_i[4] = (int)floor((genbathy->msec - 3600000.0 * store->time_i[3]) / 60000.0);
		store->time_i[5] = (int)floor((genbathy->msec
					       - 3600000.0 * store->time_i[3]
					       - 60000.0 * store->time_i[4]) / 1000.0);;
		store->time_i[6] = (int)((genbathy->msec
					       - 3600000.0 * store->time_i[3]
					       - 60000.0 * store->time_i[4]
					       - 1000.0 * store->time_i[5]) * 1000.0);;
		mb_get_time(verbose, store->time_i, &(store->time_d));
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       genbathy->version:                    %u\n",genbathy->version);
		fprintf(stderr,"dbg5       genbathy->msec:                       %f\n",genbathy->msec);
		fprintf(stderr,"dbg5       genbathy->day:                        %u\n",genbathy->day);
		fprintf(stderr,"dbg5       genbathy->month:                      %u\n",genbathy->month);
		fprintf(stderr,"dbg5       genbathy->year:                       %u\n",genbathy->year);
		fprintf(stderr,"dbg5       genbathy->ping_number:                %u\n",genbathy->ping_number);
		fprintf(stderr,"dbg5       genbathy->sonar_model:                %u\n",genbathy->sonar_model);
		fprintf(stderr,"dbg5       genbathy->transducer_serial_number:   %lu\n",genbathy->transducer_serial_number);
		fprintf(stderr,"dbg5       genbathy->number_beams:               %u\n",genbathy->number_beams);
		fprintf(stderr,"dbg5       genbathy->modeflags:                  %u\n",genbathy->modeflags);
		fprintf(stderr,"dbg5       genbathy->sampling_frequency:         %f\n",genbathy->sampling_frequency);
		fprintf(stderr,"dbg5       genbathy->acoustic_frequency:         %f\n",genbathy->acoustic_frequency);
		fprintf(stderr,"dbg5       genbathy->tx_power:                   %f\n",genbathy->tx_power);
		fprintf(stderr,"dbg5       genbathy->pulse_width:                %f\n",genbathy->pulse_width);
		fprintf(stderr,"dbg5       genbathy->absorption_loss:            %f\n",genbathy->absorption_loss);
		fprintf(stderr,"dbg5       genbathy->spreading_loss:             %f\n",genbathy->spreading_loss);
		fprintf(stderr,"dbg5       genbathy->sample_type:                %u\n",genbathy->sample_type);
		fprintf(stderr,"dbg5       genbathy->sound_velocity:             %f\n",genbathy->sound_velocity);
		for (i=0;i<genbathy->number_beams;i++)
			{
			fprintf(stderr,"dbg5       genbathy->detection_point[%3d]:       %f\n",i,genbathy->detection_point[i]);
			fprintf(stderr,"dbg5       genbathy->rx_angle[%3d]:              %f\n",i,genbathy->rx_angle[i]);	
			fprintf(stderr,"dbg5       genbathy->flags[%3d]:                 %u\n",i,genbathy->flags[i]);	
			fprintf(stderr,"dbg5       genbathy->backscatter[%3d]:           %f\n",i,genbathy->backscatter[i]);
			}
		fprintf(stderr,"dbg5       genbathy->checksum:                   %u\n",genbathy->checksum);	
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
int mbr_wasspenl_rd_corbathy(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wasspenl_rd_corbathy";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_corbathy_struct *corbathy;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %p\n",(void *)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	corbathy = &(store->corbathy);

	/* extract the data */
	index = 16;
	mb_get_binary_int(MB_YES, &buffer[index], &(corbathy->version)); index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &(corbathy->msec)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(corbathy->num_beams)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(corbathy->ping_number)); index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &(corbathy->latitude)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(corbathy->longitude)); index += 8;
	mb_get_binary_float(MB_YES, &buffer[index], &(corbathy->bearing)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(corbathy->roll)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(corbathy->pitch)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(corbathy->heave)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(corbathy->sample_type)); index += 4;
	for (i=0;i<6;i++)
		{
		mb_get_binary_int(MB_YES, &buffer[index], &(corbathy->spare[i])); index += 4;
		}
	for (i=0;i<corbathy->num_beams;i++)
		{
		mb_get_binary_int(MB_YES, &buffer[index], &(corbathy->beam_index[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(corbathy->x[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(corbathy->y[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(corbathy->z[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(corbathy->beam_angle[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(corbathy->backscatter[i])); index += 4;
		corbathy->quality[i] = buffer[index]; index++;
		corbathy->fish[i] = buffer[index]; index++;
		corbathy->roughness[i] = buffer[index]; index++;
		corbathy->empty[i] = buffer[index]; index++;
		mb_get_binary_int(MB_YES, &buffer[index], &(corbathy->pad[i])); index += 4;
		}
	mb_get_binary_int(MB_YES, &buffer[index], &(corbathy->checksum)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_DATA;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       corbathy->version:                    %u\n",corbathy->version);
		fprintf(stderr,"dbg5       corbathy->msec:                       %f\n",corbathy->msec);
		fprintf(stderr,"dbg5       corbathy->num_beams:                  %u\n",corbathy->num_beams);
		fprintf(stderr,"dbg5       corbathy->ping_number:                %u\n",corbathy->ping_number);
		fprintf(stderr,"dbg5       corbathy->latitude:                   %f\n",corbathy->latitude);
		fprintf(stderr,"dbg5       corbathy->longitude:                  %f\n",corbathy->longitude);
		fprintf(stderr,"dbg5       corbathy->bearing:                    %f\n",corbathy->bearing);
		fprintf(stderr,"dbg5       corbathy->roll:                       %f\n",corbathy->roll);
		fprintf(stderr,"dbg5       corbathy->pitch:                      %f\n",corbathy->pitch);
		fprintf(stderr,"dbg5       corbathy->heave:                      %f\n",corbathy->heave);
		fprintf(stderr,"dbg5       corbathy->sample_type:                %u\n",corbathy->sample_type);
		for (i=0;i<6;i++)
			{
			fprintf(stderr,"dbg5       corbathy->spare[%3d]:                 %u\n",i,corbathy->spare[i]);	
			}
		for (i=0;i<corbathy->num_beams;i++)
			{
			fprintf(stderr,"dbg5       corbathy->beam_index[%3d]:            %u\n",i,corbathy->beam_index[i]);	
			fprintf(stderr,"dbg5       corbathy->x[%3d]:                     %f\n",i,corbathy->x[i]);
			fprintf(stderr,"dbg5       corbathy->y[%3d]:                     %f\n",i,corbathy->y[i]);
			fprintf(stderr,"dbg5       corbathy->z[%3d]:                     %f\n",i,corbathy->z[i]);
			fprintf(stderr,"dbg5       corbathy->beam_angle[%3d]:            %f\n",i,corbathy->beam_angle[i]);	
			fprintf(stderr,"dbg5       corbathy->backscatter[%3d]:           %f\n",i,corbathy->backscatter[i]);	
			fprintf(stderr,"dbg5       corbathy->quality[%3d]:               %u\n",i,corbathy->quality[i]);	
			fprintf(stderr,"dbg5       corbathy->fish[%3d]:                  %u\n",i,corbathy->fish[i]);	
			fprintf(stderr,"dbg5       corbathy->roughness[%3d]:             %u\n",i,corbathy->roughness[i]);	
			fprintf(stderr,"dbg5       corbathy->empty[%3d]:                 %u\n",i,corbathy->empty[i]);	
			fprintf(stderr,"dbg5       corbathy->pad[%3d]:                   %u\n",i,corbathy->pad[i]);	
			}
		fprintf(stderr,"dbg5       corbathy->checksum:                   %u\n",corbathy->checksum);	
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
int mbr_wasspenl_rd_rawsonar(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wasspenl_rd_rawsonar";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_rawsonar_struct *rawsonar;
	int	index;
	size_t	rawdata_len;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %p\n",(void *)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	rawsonar = &(store->rawsonar);

	/* extract the data */
	index = 16;
	mb_get_binary_int(MB_YES, &buffer[index], &(rawsonar->version)); index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &(rawsonar->msec)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(rawsonar->ping_number)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(rawsonar->sample_rate)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(rawsonar->n)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(rawsonar->m)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(rawsonar->tx_power)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(rawsonar->pulse_width)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(rawsonar->sample_type)); index += 4;
	for (i=0;i<rawsonar->n;i++)
		{
		mb_get_binary_int(MB_YES, &buffer[index], &(rawsonar->spare[i])); index += 4;
		}
	for (i=0;i<rawsonar->n;i++)
		{
		mb_get_binary_int(MB_YES, &buffer[index], &(rawsonar->beam_index[i])); index += 4;
		}
	for (i=0;i<rawsonar->n;i++)
		{
		mb_get_binary_int(MB_YES, &buffer[index], &(rawsonar->detection_point[i])); index += 4;
		}
	for (i=0;i<rawsonar->n;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(rawsonar->beam_angle[i])); index += 4;
		}
	rawdata_len = (size_t)(rawsonar->n * rawsonar->m);
	if (rawsonar->rawdata_alloc < rawdata_len)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, rawdata_len,
						(void **)&(rawsonar->rawdata), error);
		if (status != MB_SUCCESS)
			rawsonar->rawdata_alloc = 0;
		else
			rawsonar->rawdata_alloc = rawdata_len;
		}
	memcpy(rawsonar->rawdata, &buffer[index], rawdata_len); index += rawdata_len;
	mb_get_binary_int(MB_YES, &buffer[index], &(rawsonar->checksum)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_WATER_COLUMN;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       rawsonar->version:                    %u\n",rawsonar->version);
		fprintf(stderr,"dbg5       rawsonar->msec:                       %f\n",rawsonar->msec);
		fprintf(stderr,"dbg5       rawsonar->ping_number:                %u\n",rawsonar->ping_number);
		fprintf(stderr,"dbg5       rawsonar->sample_rate:                %f\n",rawsonar->sample_rate);
		fprintf(stderr,"dbg5       rawsonar->n:                          %u\n",rawsonar->n);
		fprintf(stderr,"dbg5       rawsonar->m:                          %u\n",rawsonar->m);
		fprintf(stderr,"dbg5       rawsonar->tx_power:                   %f\n",rawsonar->tx_power);
		fprintf(stderr,"dbg5       rawsonar->pulse_width:                %f\n",rawsonar->pulse_width);
		fprintf(stderr,"dbg5       rawsonar->sample_type:                %u\n",rawsonar->sample_type);
		for (i=0;i<rawsonar->n;i++)
			{
			fprintf(stderr,"dbg5       rawsonar->spare[%3d]:                 %u\n",i,rawsonar->spare[i]);	
			fprintf(stderr,"dbg5       rawsonar->beam_index[%3d]:            %u\n",i,rawsonar->beam_index[i]);	
			fprintf(stderr,"dbg5       rawsonar->detection_point[%3d]:       %u\n",i,rawsonar->detection_point[i]);	
			fprintf(stderr,"dbg5       rawsonar->beam_angle[%3d]:            %f\n",i,rawsonar->beam_angle[i]);	
			}
		fprintf(stderr,"dbg5       rawsonar->rawdata_alloc:              %zu\n",rawsonar->rawdata_alloc);
		for (i=0;i<rawsonar->m;i++)
		for (j=0;j<rawsonar->n;j++)
			{
			k = i * rawsonar->n + j;
			fprintf(stderr,"dbg5       rawsonar->rawdata[%4d][%4d]:          %u\n",i,j,rawsonar->rawdata[k]);	
			}
		fprintf(stderr,"dbg5       rawsonar->checksum:                   %u\n",rawsonar->checksum);	
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
int mbr_wasspenl_rd_gen_sens(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wasspenl_rd_gen_sens";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_gen_sens_struct *gen_sens;
	int	index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %p\n",(void *)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	gen_sens = &(store->gen_sens);

	/* extract the data */
	index = 16;
	mb_get_binary_int(MB_YES, &buffer[index], &(gen_sens->version)); index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &(gen_sens->msec)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(gen_sens->port_number)); index += 4;
	gen_sens->message_length = buffer[index]; index++;
	memcpy(gen_sens->message, &buffer[index], (size_t)gen_sens->message_length); index += gen_sens->message_length;
	mb_get_binary_int(MB_YES, &buffer[index], &(gen_sens->checksum)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_GEN_SENS;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       gen_sens->version:                    %u\n",gen_sens->version);
		fprintf(stderr,"dbg5       gen_sens->msec:                       %f\n",gen_sens->msec);
		fprintf(stderr,"dbg5       gen_sens->port_number:                %u\n",gen_sens->port_number);
		fprintf(stderr,"dbg5       gen_sens->message_length:             %u\n",gen_sens->message_length);
		fprintf(stderr,"dbg5       gen_sens->message:                    %s\n",gen_sens->message);
		fprintf(stderr,"dbg5       gen_sens->checksum:                   %u\n",gen_sens->checksum);	
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

int mbr_wasspenl_rd_nvupdate(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wasspenl_rd_nvupdate";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_nvupdate_struct *nvupdate;
	int	index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %p\n",(void *)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	nvupdate = &(store->nvupdate);

	/* extract the data */
	index = 16;
	mb_get_binary_int(MB_YES, &buffer[index], &(nvupdate->version)); index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &(nvupdate->latitude)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(nvupdate->longitude)); index += 8;
	mb_get_binary_float(MB_YES, &buffer[index], &(nvupdate->sog)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(nvupdate->cog)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(nvupdate->heading)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(nvupdate->roll)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(nvupdate->pitch)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(nvupdate->heave)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(nvupdate->nadir_depth)); index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(nvupdate->checksum)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_NAV;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       nvupdate->version:                    %u\n",nvupdate->version);
		fprintf(stderr,"dbg5       nvupdate->latitude:                   %f\n",nvupdate->latitude);
		fprintf(stderr,"dbg5       nvupdate->longitude:                  %f\n",nvupdate->longitude);
		fprintf(stderr,"dbg5       nvupdate->sog:                        %f\n",nvupdate->sog);
		fprintf(stderr,"dbg5       nvupdate->cog:                        %f\n",nvupdate->cog);
		fprintf(stderr,"dbg5       nvupdate->heading:                    %f\n",nvupdate->heading);
		fprintf(stderr,"dbg5       nvupdate->roll:                       %f\n",nvupdate->roll);
		fprintf(stderr,"dbg5       nvupdate->pitch:                      %f\n",nvupdate->pitch);
		fprintf(stderr,"dbg5       nvupdate->heave:                      %f\n",nvupdate->heave);
		fprintf(stderr,"dbg5       nvupdate->nadir_depth:                %f\n",nvupdate->nadir_depth);
		fprintf(stderr,"dbg5       nvupdate->checksum:                   %u\n",nvupdate->checksum);	
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

int mbr_wasspenl_rd_wcd_navi(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wasspenl_rd_wcd_navi";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_wcd_navi_struct *wcd_navi;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %p\n",(void *)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	wcd_navi = &(store->wcd_navi);

	/* extract the data */
	index = 16;
	mb_get_binary_int(MB_YES, &buffer[index], &(wcd_navi->version)); index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &(wcd_navi->latitude)); index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(wcd_navi->longitude)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(wcd_navi->num_points)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(wcd_navi->bearing)); index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &(wcd_navi->msec)); index += 8;
	mb_get_binary_int(MB_YES, &buffer[index], &(wcd_navi->ping_number)); index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(wcd_navi->sample_rate)); index += 4;
	if (wcd_navi->wcdata_alloc < wcd_navi->num_points)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, wcd_navi->num_points * sizeof(float),
						(void **)&(wcd_navi->wcdata_x), error);
		status = mb_reallocd(verbose, __FILE__, __LINE__, wcd_navi->num_points * sizeof(float),
						(void **)&(wcd_navi->wcdata_y), error);
		status = mb_reallocd(verbose, __FILE__, __LINE__, wcd_navi->num_points * sizeof(float),
						(void **)&(wcd_navi->wcdata_mag), error);
		if (status != MB_SUCCESS)
			wcd_navi->wcdata_alloc = 0;
		else
			wcd_navi->wcdata_alloc = wcd_navi->num_points;
		}
	for (i=0;i<wcd_navi->num_points;i++)
		{
		mb_get_binary_float(MB_YES, &buffer[index], &(wcd_navi->wcdata_x[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(wcd_navi->wcdata_y[i])); index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(wcd_navi->wcdata_mag[i])); index += 4;
		}
	mb_get_binary_int(MB_YES, &buffer[index], &(wcd_navi->checksum)); index += 4;

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_WC_PICKS;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       wcd_navi->version:                    %u\n",wcd_navi->version);
		fprintf(stderr,"dbg5       wcd_navi->latitude:                   %f\n",wcd_navi->latitude);
		fprintf(stderr,"dbg5       wcd_navi->longitude:                  %f\n",wcd_navi->longitude);
		fprintf(stderr,"dbg5       wcd_navi->num_points:                 %u\n",wcd_navi->num_points);
		fprintf(stderr,"dbg5       wcd_navi->bearing:                    %f\n",wcd_navi->bearing);
		fprintf(stderr,"dbg5       wcd_navi->msec:                       %f\n",wcd_navi->msec);
		fprintf(stderr,"dbg5       wcd_navi->ping_number:                %u\n",wcd_navi->ping_number);
		fprintf(stderr,"dbg5       wcd_navi->sample_type:                %f\n",wcd_navi->sample_rate);
		for (i=0;i<wcd_navi->num_points;i++)
			{
			fprintf(stderr,"dbg5       wcd_navi->wcdata_x[%3d]:              %f\n",i,wcd_navi->wcdata_x[i]);	
			fprintf(stderr,"dbg5       wcd_navi->wcdata_y[%3d]:              %f\n",i,wcd_navi->wcdata_y[i]);	
			fprintf(stderr,"dbg5       wcd_navi->wcdata_mag[%3d]:            %f\n",i,wcd_navi->wcdata_mag[i]);	
			}
		fprintf(stderr,"dbg5       wcd_navi->checksum:                   %u\n",wcd_navi->checksum);	
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

int mbr_wasspenl_rd_sys_cfg1(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wasspenl_rd_sys_cfg1";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_sys_cfg1_struct *sys_cfg1;
	int	index;
	unsigned int size;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %p\n",(void *)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	sys_cfg1 = &(store->sys_cfg1);
	
	/* get the size */
	index = 4;
	mb_get_binary_int(MB_YES, &buffer[index], &size);
	sys_cfg1->sys_cfg1_len = size;

	/* extract the data */
	if (sys_cfg1->sys_cfg1_data_alloc < size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, (size_t)size,
						(void **)&(sys_cfg1->sys_cfg1_data), error);
		if (status != MB_SUCCESS)
			sys_cfg1->sys_cfg1_data_alloc = 0;
		else
			sys_cfg1->sys_cfg1_data_alloc = size;
		}
	memcpy(sys_cfg1->sys_cfg1_data, buffer, (size_t)sys_cfg1->sys_cfg1_len);

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_PARAMETER;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		for (i=0;i<sys_cfg1->sys_cfg1_len;i++)
			{
			fprintf(stderr,"dbg5       sys_cfg1->sys_cfg1_data[%3d]:           %u\n",i,sys_cfg1->sys_cfg1_data[i]);	
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

int mbr_wasspenl_rd_mcomment(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wasspenl_rd_mcomment";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_mcomment_struct *mcomment;
	int	index;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %p\n",(void *)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	mcomment = &(store->mcomment);

	/* extract the data */
	index = 16;
	mb_get_binary_int(MB_YES, &buffer[index], &(mcomment->comment_length)); index += 4;
	memcpy(mcomment->comment_message, &buffer[index], (size_t)mcomment->comment_length);

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_COMMENT;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       mcomment->comment_length:             %u\n",mcomment->comment_length);
		fprintf(stderr,"dbg5       mcomment->comment_message:            %s\n",mcomment->comment_message);
		fprintf(stderr,"dbg5       mcomment->checksum:                   %u\n",mcomment->checksum);	
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

int mbr_wasspenl_rd_unknown1(int verbose, char *buffer, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wasspenl_rd_unknown1";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_unknown1_struct *unknown1;
	int	index;
	unsigned int size;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer:     %p\n",(void *)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	unknown1 = &(store->unknown1);
	
	/* get the size */
	index = 4;
	mb_get_binary_int(MB_YES, &buffer[index], &size);
	unknown1->unknown1_len = size;

	/* extract the data */
	if (unknown1->unknown1_data_alloc < size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, (size_t)size,
						(void **)&(unknown1->unknown1_data), error);
		if (status != MB_SUCCESS)
			unknown1->unknown1_data_alloc = 0;
		else
			unknown1->unknown1_data_alloc = size;
		}
	memcpy(unknown1->unknown1_data, buffer, (size_t)unknown1->unknown1_len);

	/* set kind */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_RAW_LINE;
		}
	else
		{
		store->kind = MB_DATA_NONE;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		for (i=0;i<unknown1->unknown1_len;i++)
			{
			fprintf(stderr,"dbg5       unknown1->unknown1_data[%3d]:           %u\n",i,unknown1->unknown1_data[i]);	
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
int mbr_wasspenl_wr_genbathy(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_wasspenl_wr_genbathy";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_genbathy_struct *genbathy;
	char	*buffer;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %p\n",(void *)bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	genbathy = &(store->genbathy);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       genbathy->version:                    %u\n",genbathy->version);
		fprintf(stderr,"dbg5       genbathy->msec:                       %f\n",genbathy->msec);
		fprintf(stderr,"dbg5       genbathy->day:                        %u\n",genbathy->day);
		fprintf(stderr,"dbg5       genbathy->month:                      %u\n",genbathy->month);
		fprintf(stderr,"dbg5       genbathy->year:                       %u\n",genbathy->year);
		fprintf(stderr,"dbg5       genbathy->ping_number:                %u\n",genbathy->ping_number);
		fprintf(stderr,"dbg5       genbathy->sonar_model:                %u\n",genbathy->sonar_model);
		fprintf(stderr,"dbg5       genbathy->transducer_serial_number:   %lu\n",genbathy->transducer_serial_number);
		fprintf(stderr,"dbg5       genbathy->number_beams:               %u\n",genbathy->number_beams);
		fprintf(stderr,"dbg5       genbathy->modeflags:                  %u\n",genbathy->modeflags);
		fprintf(stderr,"dbg5       genbathy->sampling_frequency:         %f\n",genbathy->sampling_frequency);
		fprintf(stderr,"dbg5       genbathy->acoustic_frequency:         %f\n",genbathy->acoustic_frequency);
		fprintf(stderr,"dbg5       genbathy->tx_power:                   %f\n",genbathy->tx_power);
		fprintf(stderr,"dbg5       genbathy->pulse_width:                %f\n",genbathy->pulse_width);
		fprintf(stderr,"dbg5       genbathy->absorption_loss:            %f\n",genbathy->absorption_loss);
		fprintf(stderr,"dbg5       genbathy->spreading_loss:             %f\n",genbathy->spreading_loss);
		fprintf(stderr,"dbg5       genbathy->sample_type:                %u\n",genbathy->sample_type);
		fprintf(stderr,"dbg5       genbathy->sound_velocity:             %f\n",genbathy->sound_velocity);
		for (i=0;i<genbathy->number_beams;i++)
			{
			fprintf(stderr,"dbg5       genbathy->detection_point[%3d]:       %f\n",i,genbathy->detection_point[i]);
			fprintf(stderr,"dbg5       genbathy->rx_angle[%3d]:              %f\n",i,genbathy->rx_angle[i]);	
			fprintf(stderr,"dbg5       genbathy->flags[%3d]:                 %u\n",i,genbathy->flags[i]);	
			fprintf(stderr,"dbg5       genbathy->backscatter[%3d]:           %f\n",i,genbathy->backscatter[i]);
			}
		fprintf(stderr,"dbg5       genbathy->checksum:                   %u\n",genbathy->checksum);	
		}

	/* figure out size of output record */
	*size = 92 + 16 * genbathy->number_beams;

	/* allocate memory to write rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
					(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			*bufferalloc = 0;
		else
			*bufferalloc = *size;
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the data */
		index = 0;
		mb_put_binary_int(MB_YES, MBSYS_WASSP_SYNC, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, *size, &buffer[index]); index += 4;
		strncpy(&buffer[index], "GENBATHY", 8); index += 8;
		mb_put_binary_int(MB_YES, genbathy->version, &buffer[index]); index += 4;
		mb_put_binary_double(MB_YES, genbathy->msec, &buffer[index]); index += 8;
		buffer[index] = genbathy->day; index++;
		buffer[index] = genbathy->month; index++;
		mb_put_binary_short(MB_YES, genbathy->year, &buffer[index]); index += 2;
		mb_put_binary_int(MB_YES, genbathy->ping_number, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, genbathy->sonar_model, &buffer[index]); index += 4;
		mb_put_binary_long(MB_YES, genbathy->transducer_serial_number, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, genbathy->number_beams, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, genbathy->modeflags, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, genbathy->sampling_frequency, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, genbathy->acoustic_frequency, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, genbathy->tx_power, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, genbathy->pulse_width, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, genbathy->absorption_loss, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, genbathy->spreading_loss, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, genbathy->sample_type, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, genbathy->sound_velocity, &buffer[index]); index += 4;
		for (i=0;i<genbathy->number_beams;i++)
			{
			mb_put_binary_float(MB_YES, genbathy->detection_point[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, genbathy->rx_angle[i], &buffer[index]); index += 4;
			mb_put_binary_int(MB_YES, genbathy->flags[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, genbathy->backscatter[i], &buffer[index]); index += 4;
			}

		/* now add the checksum */
		genbathy->checksum = 0;
		for (i=0;i<index;i++)
			genbathy->checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, genbathy->checksum, &buffer[index]); index += 4;
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
int mbr_wasspenl_wr_corbathy(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_wasspenl_wr_corbathy";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_corbathy_struct *corbathy;
	char	*buffer;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %p\n",(void *)bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	corbathy = &(store->corbathy);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       corbathy->version:                    %u\n",corbathy->version);
		fprintf(stderr,"dbg5       corbathy->msec:                       %f\n",corbathy->msec);
		fprintf(stderr,"dbg5       corbathy->num_beams:                  %u\n",corbathy->num_beams);
		fprintf(stderr,"dbg5       corbathy->ping_number:                %u\n",corbathy->ping_number);
		fprintf(stderr,"dbg5       corbathy->latitude:                   %f\n",corbathy->latitude);
		fprintf(stderr,"dbg5       corbathy->longitude:                  %f\n",corbathy->longitude);
		fprintf(stderr,"dbg5       corbathy->bearing:                    %f\n",corbathy->bearing);
		fprintf(stderr,"dbg5       corbathy->roll:                       %f\n",corbathy->roll);
		fprintf(stderr,"dbg5       corbathy->pitch:                      %f\n",corbathy->pitch);
		fprintf(stderr,"dbg5       corbathy->heave:                      %f\n",corbathy->heave);
		fprintf(stderr,"dbg5       corbathy->sample_type:                %u\n",corbathy->sample_type);
		for (i=0;i<6;i++)
			{
			fprintf(stderr,"dbg5       corbathy->spare[%3d]:                 %u\n",i,corbathy->spare[i]);	
			}
		for (i=0;i<corbathy->num_beams;i++)
			{
			fprintf(stderr,"dbg5       corbathy->beam_index[%3d]:            %u\n",i,corbathy->beam_index[i]);	
			fprintf(stderr,"dbg5       corbathy->x[%3d]:                     %f\n",i,corbathy->x[i]);
			fprintf(stderr,"dbg5       corbathy->y[%3d]:                     %f\n",i,corbathy->y[i]);
			fprintf(stderr,"dbg5       corbathy->z[%3d]:                     %f\n",i,corbathy->z[i]);
			fprintf(stderr,"dbg5       corbathy->beam_angle[%3d]:            %f\n",i,corbathy->beam_angle[i]);	
			fprintf(stderr,"dbg5       corbathy->backscatter[%3d]:           %f\n",i,corbathy->backscatter[i]);	
			fprintf(stderr,"dbg5       corbathy->quality[%3d]:               %u\n",i,corbathy->quality[i]);	
			fprintf(stderr,"dbg5       corbathy->fish[%3d]:                  %u\n",i,corbathy->fish[i]);	
			fprintf(stderr,"dbg5       corbathy->roughness[%3d]:             %u\n",i,corbathy->roughness[i]);	
			fprintf(stderr,"dbg5       corbathy->empty[%3d]:                 %u\n",i,corbathy->empty[i]);	
			fprintf(stderr,"dbg5       corbathy->pad[%3d]:                   %u\n",i,corbathy->pad[i]);	
			}
		fprintf(stderr,"dbg5       corbathy->checksum:                   %u\n",corbathy->checksum);	
		}

	/* figure out size of output record */
	*size = 100 + 32 * corbathy->num_beams;

	/* allocate memory to write rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
					(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			*bufferalloc = 0;
		else
			*bufferalloc = *size;
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the data */
		index = 0;
		mb_put_binary_int(MB_YES, MBSYS_WASSP_SYNC, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, *size, &buffer[index]); index += 4;
		strncpy(&buffer[index], "CORBATHY", 8); index += 8;
		mb_put_binary_int(MB_YES, corbathy->version, &buffer[index]); index += 4;
		mb_put_binary_double(MB_YES, corbathy->msec, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, corbathy->num_beams, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, corbathy->ping_number, &buffer[index]); index += 4;
		mb_put_binary_double(MB_YES, corbathy->latitude, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, corbathy->longitude, &buffer[index]); index += 8;
		mb_put_binary_float(MB_YES, corbathy->bearing, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, corbathy->roll, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, corbathy->pitch, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, corbathy->heave, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, corbathy->sample_type, &buffer[index]); index += 4;
		for (i=0;i<6;i++)
			{
			mb_put_binary_int(MB_YES, corbathy->spare[i], &buffer[index]); index += 4;
			}
		for (i=0;i<corbathy->num_beams;i++)
			{
			mb_put_binary_int(MB_YES, corbathy->beam_index[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, corbathy->x[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, corbathy->y[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, corbathy->z[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, corbathy->beam_angle[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, corbathy->backscatter[i], &buffer[index]); index += 4;
			buffer[index] = corbathy->quality[i]; index++;
			buffer[index] = corbathy->fish[i]; index++;
			buffer[index] = corbathy->roughness[i]; index++;
			buffer[index] = corbathy->empty[i]; index++;
			mb_put_binary_int(MB_YES, corbathy->pad[i], &buffer[index]); index += 4;
			}

		/* now add the checksum */
		corbathy->checksum = 0;
		for (i=0;i<index;i++)
			corbathy->checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, corbathy->checksum, &buffer[index]); index += 4;
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
int mbr_wasspenl_wr_rawsonar(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_wasspenl_wr_rawsonar";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_rawsonar_struct *rawsonar;
	char	*buffer;
	int	index;
	size_t	rawdata_len;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %p\n",(void *)bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	rawsonar = &(store->rawsonar);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       rawsonar->version:                    %u\n",rawsonar->version);
		fprintf(stderr,"dbg5       rawsonar->msec:                       %f\n",rawsonar->msec);
		fprintf(stderr,"dbg5       rawsonar->ping_number:                %u\n",rawsonar->ping_number);
		fprintf(stderr,"dbg5       rawsonar->sample_rate:                %f\n",rawsonar->sample_rate);
		fprintf(stderr,"dbg5       rawsonar->n:                          %u\n",rawsonar->n);
		fprintf(stderr,"dbg5       rawsonar->m:                          %u\n",rawsonar->m);
		fprintf(stderr,"dbg5       rawsonar->tx_power:                   %f\n",rawsonar->tx_power);
		fprintf(stderr,"dbg5       rawsonar->pulse_width:                %f\n",rawsonar->pulse_width);
		fprintf(stderr,"dbg5       rawsonar->sample_type:                %u\n",rawsonar->sample_type);
		for (i=0;i<rawsonar->n;i++)
			{
			fprintf(stderr,"dbg5       rawsonar->spare[%3d]:                 %u\n",i,rawsonar->spare[i]);	
			fprintf(stderr,"dbg5       rawsonar->beam_index[%3d]:            %u\n",i,rawsonar->beam_index[i]);	
			fprintf(stderr,"dbg5       rawsonar->detection_point[%3d]:       %u\n",i,rawsonar->detection_point[i]);	
			fprintf(stderr,"dbg5       rawsonar->beam_angle[%3d]:            %f\n",i,rawsonar->beam_angle[i]);	
			}
		fprintf(stderr,"dbg5       rawsonar->rawdata_alloc:              %zu\n",rawsonar->rawdata_alloc);
		for (i=0;i<rawsonar->m;i++)
		for (j=0;j<rawsonar->n;j++)
			{
			k = i * rawsonar->n + j;
			fprintf(stderr,"dbg5       rawsonar->rawdata[%4d][%4d]:          %u\n",i,j,rawsonar->rawdata[k]);	
			}
		fprintf(stderr,"dbg5       rawsonar->checksum:                   %u\n",rawsonar->checksum);	
		}

	/* figure out size of output record */
	*size = 60 + 12 * rawsonar->n + 2 * rawsonar->m * rawsonar->n;

	/* allocate memory to write rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
					(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			*bufferalloc = 0;
		else
			*bufferalloc = *size;
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the data */
		index = 0;
		mb_put_binary_int(MB_YES, MBSYS_WASSP_SYNC, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, *size, &buffer[index]); index += 4;
		strncpy(&buffer[index], "RAWSONAR", 8); index += 8;
		mb_put_binary_int(MB_YES, rawsonar->version, &buffer[index]); index += 4;
		mb_put_binary_double(MB_YES, rawsonar->msec, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, rawsonar->ping_number, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, rawsonar->sample_rate, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, rawsonar->n, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, rawsonar->m, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, rawsonar->tx_power, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, rawsonar->pulse_width, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, rawsonar->sample_type, &buffer[index]); index += 4;
		for (i=0;i<rawsonar->n;i++)
			{
			mb_put_binary_int(MB_YES, rawsonar->spare[i], &buffer[index]); index += 4;
			}
		for (i=0;i<rawsonar->n;i++)
			{
			mb_put_binary_int(MB_YES, rawsonar->beam_index[i], &buffer[index]); index += 4;
			}
		for (i=0;i<rawsonar->n;i++)
			{
			mb_put_binary_int(MB_YES, rawsonar->detection_point[i], &buffer[index]); index += 4;
			}
		for (i=0;i<rawsonar->n;i++)
			{
			mb_put_binary_float(MB_YES, rawsonar->beam_angle[i], &buffer[index]); index += 4;
			}
		rawdata_len = (size_t)(rawsonar->n * rawsonar->m);
		memcpy(&buffer[index], rawsonar->rawdata, rawdata_len); index += rawdata_len;

		/* now add the checksum */
		rawsonar->checksum = 0;
		for (i=0;i<index;i++)
			rawsonar->checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, rawsonar->checksum, &buffer[index]); index += 4;
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
int mbr_wasspenl_wr_gen_sens(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_wasspenl_wr_gen_sens";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_gen_sens_struct *gen_sens;
	char	*buffer;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %p\n",(void *)bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	gen_sens = &(store->gen_sens);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       gen_sens->version:                    %u\n",gen_sens->version);
		fprintf(stderr,"dbg5       gen_sens->msec:                       %f\n",gen_sens->msec);
		fprintf(stderr,"dbg5       gen_sens->port_number:                %u\n",gen_sens->port_number);
		fprintf(stderr,"dbg5       gen_sens->message_length:             %u\n",gen_sens->message_length);
		fprintf(stderr,"dbg5       gen_sens->message:                    %s\n",gen_sens->message);
		fprintf(stderr,"dbg5       gen_sens->checksum:                   %u\n",gen_sens->checksum);	
		}

	/* figure out size of output record */
	*size = 33 + gen_sens->message_length;

	/* allocate memory to write rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
					(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			*bufferalloc = 0;
		else
			*bufferalloc = *size;
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the data */
		index = 0;
		mb_put_binary_int(MB_YES, MBSYS_WASSP_SYNC, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, *size, &buffer[index]); index += 4;
		strncpy(&buffer[index], "GEN_SENS", 8); index += 8;
		mb_put_binary_int(MB_YES, gen_sens->version, &buffer[index]); index += 4;
		mb_put_binary_double(MB_YES, gen_sens->msec, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, gen_sens->port_number, &buffer[index]); index += 4;
		buffer[index] = gen_sens->message_length; index++;
		memcpy(&buffer[index], gen_sens->message, (size_t)gen_sens->message_length); index += gen_sens->message_length;

		/* now add the checksum */
		gen_sens->checksum = 0;
		for (i=0;i<index;i++)
			gen_sens->checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, gen_sens->checksum, &buffer[index]); index += 4;
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

int mbr_wasspenl_wr_nvupdate(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_wasspenl_wr_nvupdate";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_nvupdate_struct *nvupdate;
	char	*buffer;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %p\n",(void *)bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	nvupdate = &(store->nvupdate);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       nvupdate->version:                    %u\n",nvupdate->version);
		fprintf(stderr,"dbg5       nvupdate->latitude:                   %f\n",nvupdate->latitude);
		fprintf(stderr,"dbg5       nvupdate->longitude:                  %f\n",nvupdate->longitude);
		fprintf(stderr,"dbg5       nvupdate->sog:                        %f\n",nvupdate->sog);
		fprintf(stderr,"dbg5       nvupdate->cog:                        %f\n",nvupdate->cog);
		fprintf(stderr,"dbg5       nvupdate->heading:                    %f\n",nvupdate->heading);
		fprintf(stderr,"dbg5       nvupdate->roll:                       %f\n",nvupdate->roll);
		fprintf(stderr,"dbg5       nvupdate->pitch:                      %f\n",nvupdate->pitch);
		fprintf(stderr,"dbg5       nvupdate->heave:                      %f\n",nvupdate->heave);
		fprintf(stderr,"dbg5       nvupdate->nadir_depth:                %f\n",nvupdate->nadir_depth);
		fprintf(stderr,"dbg5       nvupdate->checksum:                   %u\n",nvupdate->checksum);	
		}

	/* figure out size of output record */
	*size = 68;

	/* allocate memory to write rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
					(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			*bufferalloc = 0;
		else
			*bufferalloc = *size;
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the data */
		index = 0;
		mb_put_binary_int(MB_YES, MBSYS_WASSP_SYNC, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, *size, &buffer[index]); index += 4;
		strncpy(&buffer[index], "NVUPDATE", 8); index += 8;
		mb_put_binary_int(MB_YES, nvupdate->version, &buffer[index]); index += 4;
		mb_put_binary_double(MB_YES, nvupdate->latitude, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, nvupdate->longitude, &buffer[index]); index += 8;
		mb_put_binary_float(MB_YES, nvupdate->sog, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, nvupdate->cog, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, nvupdate->heading, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, nvupdate->roll, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, nvupdate->pitch, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, nvupdate->heave, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, nvupdate->nadir_depth, &buffer[index]); index += 4;

		/* now add the checksum */
		nvupdate->checksum = 0;
		for (i=0;i<index;i++)
			nvupdate->checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, nvupdate->checksum, &buffer[index]); index += 4;
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

int mbr_wasspenl_wr_wcd_navi(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_wasspenl_wr_wcd_navi";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_wcd_navi_struct *wcd_navi;
	char	*buffer;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %p\n",(void *)bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	wcd_navi = &(store->wcd_navi);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       wcd_navi->version:                    %u\n",wcd_navi->version);
		fprintf(stderr,"dbg5       wcd_navi->latitude:                   %f\n",wcd_navi->latitude);
		fprintf(stderr,"dbg5       wcd_navi->longitude:                  %f\n",wcd_navi->longitude);
		fprintf(stderr,"dbg5       wcd_navi->num_points:                 %u\n",wcd_navi->num_points);
		fprintf(stderr,"dbg5       wcd_navi->bearing:                    %f\n",wcd_navi->bearing);
		fprintf(stderr,"dbg5       wcd_navi->msec:                       %f\n",wcd_navi->msec);
		fprintf(stderr,"dbg5       wcd_navi->ping_number:                %u\n",wcd_navi->ping_number);
		fprintf(stderr,"dbg5       wcd_navi->sample_type:                %f\n",wcd_navi->sample_rate);
		for (i=0;i<wcd_navi->num_points;i++)
			{
			fprintf(stderr,"dbg5       wcd_navi->wcdata_x[%3d]:              %f\n",i,wcd_navi->wcdata_x[i]);	
			fprintf(stderr,"dbg5       wcd_navi->wcdata_y[%3d]:              %f\n",i,wcd_navi->wcdata_y[i]);	
			fprintf(stderr,"dbg5       wcd_navi->wcdata_mag[%3d]:            %f\n",i,wcd_navi->wcdata_mag[i]);	
			}
		fprintf(stderr,"dbg5       wcd_navi->checksum:                   %u\n",wcd_navi->checksum);	
		}

	/* figure out size of output record */
	*size = 64 + 12 * wcd_navi->num_points;

	/* allocate memory to write rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
					(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			*bufferalloc = 0;
		else
			*bufferalloc = *size;
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the data */
		index = 0;
		mb_put_binary_int(MB_YES, MBSYS_WASSP_SYNC, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, *size, &buffer[index]); index += 4;
		strncpy(&buffer[index], "WCD_NAVI", 8); index += 8;
		mb_put_binary_int(MB_YES, wcd_navi->version, &buffer[index]); index += 4;
		mb_put_binary_double(MB_YES, wcd_navi->latitude, &buffer[index]); index += 8;
		mb_put_binary_double(MB_YES, wcd_navi->longitude, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, wcd_navi->num_points, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, wcd_navi->bearing, &buffer[index]); index += 4;
		mb_put_binary_double(MB_YES, wcd_navi->msec, &buffer[index]); index += 8;
		mb_put_binary_int(MB_YES, wcd_navi->ping_number, &buffer[index]); index += 4;
		mb_put_binary_float(MB_YES, wcd_navi->sample_rate, &buffer[index]); index += 4;
		for (i=0;i<wcd_navi->num_points;i++)
			{
			mb_put_binary_float(MB_YES, wcd_navi->wcdata_x[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, wcd_navi->wcdata_y[i], &buffer[index]); index += 4;
			mb_put_binary_float(MB_YES, wcd_navi->wcdata_mag[i], &buffer[index]); index += 4;
			}

		/* now add the checksum */
		wcd_navi->checksum = 0;
		for (i=0;i<index;i++)
			wcd_navi->checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, wcd_navi->checksum, &buffer[index]); index += 4;
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

int mbr_wasspenl_wr_sys_cfg1(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_wasspenl_wr_sys_cfg1";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_sys_cfg1_struct *sys_cfg1;
	char	*buffer;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %p\n",(void *)bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	sys_cfg1 = &(store->sys_cfg1);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		for (i=0;i<sys_cfg1->sys_cfg1_len;i++)
			{
			fprintf(stderr,"dbg5       sys_cfg1->sys_cfg1_data[%3d]:           %u\n",i,sys_cfg1->sys_cfg1_data[i]);	
			}
		}

	/* figure out size of output record */
	*size = sys_cfg1->sys_cfg1_len;

	/* allocate memory to write rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
					(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			*bufferalloc = 0;
		else
			*bufferalloc = *size;
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the data */
		memcpy(buffer, sys_cfg1->sys_cfg1_data, (size_t)sys_cfg1->sys_cfg1_len);
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

int mbr_wasspenl_wr_mcomment(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_wasspenl_wr_mcomment";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_mcomment_struct *mcomment;
	char	*buffer;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %p\n",(void *)bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	mcomment = &(store->mcomment);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       mcomment->comment_length:             %u\n",mcomment->comment_length);
		fprintf(stderr,"dbg5       mcomment->comment_message:            %s\n",mcomment->comment_message);
		fprintf(stderr,"dbg5       mcomment->checksum:                   %u\n",mcomment->checksum);	
		}

	/* figure out size of output record */
	*size = 24 + mcomment->comment_length;

	/* allocate memory to write rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
					(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			*bufferalloc = 0;
		else
			*bufferalloc = *size;
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the data */
		index = 0;
		mb_put_binary_int(MB_YES, MBSYS_WASSP_SYNC, &buffer[index]); index += 4;
		mb_put_binary_int(MB_YES, *size, &buffer[index]); index += 4;
		strncpy(&buffer[index], "MCOMMENT", 8); index += 8;
		mb_put_binary_int(MB_YES, mcomment->comment_length, &buffer[index]); index += 4;
		memcpy(&buffer[index], mcomment->comment_message, (size_t)mcomment->comment_length); index += mcomment->comment_length;

		/* now add the checksum */
		mcomment->checksum = 0;
		for (i=0;i<index;i++)
			mcomment->checksum += (unsigned char) buffer[i];
		mb_put_binary_int(MB_YES, mcomment->checksum, &buffer[index]); index += 4;
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

int mbr_wasspenl_wr_unknown1(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error)
{
	char	*function_name = "mbr_wasspenl_wr_unknown1";
	int	status = MB_SUCCESS;
	struct mbsys_wassp_struct *store;
        struct mbsys_wassp_unknown1_struct *unknown1;
	char	*buffer;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       bufferalloc:%d\n",*bufferalloc);
		fprintf(stderr,"dbg2       bufferptr:  %p\n",(void *)bufferptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to raw data structure */
	store = (struct mbsys_wassp_struct *) store_ptr;
	unknown1 = &(store->unknown1);

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		for (i=0;i<unknown1->unknown1_len;i++)
			{
			fprintf(stderr,"dbg5       unknown1->unknown1_data[%3d]:           %u\n",i,unknown1->unknown1_data[i]);	
			}
		}

	/* figure out size of output record */
	*size = unknown1->unknown1_len;

	/* allocate memory to write rest of record if necessary */
	if (*bufferalloc < *size)
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, *size,
					(void **)bufferptr, error);
		if (status != MB_SUCCESS)
			*bufferalloc = 0;
		else
			*bufferalloc = *size;
		}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS)
		{
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the data */
		memcpy(buffer, unknown1->unknown1_data, (size_t)unknown1->unknown1_len);
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
