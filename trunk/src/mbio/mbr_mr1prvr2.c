/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mr1prvr2.c	3/6/2003
 *	$Id$
 *
 *    Copyright (c) 2003-2013 by
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
 * mbr_mr1prvr2.c contains the functions for reading and writing
 * multibeam data in the MR1PRVR2 format.
 * These functions include:
 *   mbr_alm_mr1prvr2	- allocate read/write memory
 *   mbr_dem_mr1prvr2	- deallocate read/write memory
 *   mbr_rt_mr1prvr2	- read and translate data
 *   mbr_wt_mr1prvr2	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 6, 2003
 *
 * $Log: mbr_mr1prvr2.c,v $
 * Revision 5.4  2008/07/10 06:43:41  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.3  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.2  2004/04/27 01:01:46  caress
 * Fixed typo.
 *
 * Revision 5.1  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.0  2003/03/10 20:03:59  caress
 * Initial version.
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "../mr1pr/mr1pr.h"
#include "mbsys_mr1v2001.h"

/* essential function prototypes */
int mbr_register_mr1prvr2(int verbose, void *mbio_ptr,
		int *error);
int mbr_info_mr1prvr2(int verbose,
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
int mbr_alm_mr1prvr2(int verbose, void *mbio_ptr, int *error);
int mbr_dem_mr1prvr2(int verbose, void *mbio_ptr, int *error);
int mbr_rt_mr1prvr2(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_mr1prvr2(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_mr1prvr2_rd_data(int verbose, void *mbio_ptr, int *error);
int mbr_mr1prvr2_wr_data(int verbose, void *mbio_ptr, char *store_ptr, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_mr1prvr2(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_mr1prvr2";
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
	status = mbr_info_mr1prvr2(verbose,
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_mr1prvr2;
	mb_io_ptr->mb_io_format_free = &mbr_dem_mr1prvr2;
	mb_io_ptr->mb_io_store_alloc = &mbsys_mr1v2001_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_mr1v2001_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_mr1prvr2;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_mr1prvr2;
	mb_io_ptr->mb_io_dimensions = &mbsys_mr1v2001_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_mr1v2001_extract;
	mb_io_ptr->mb_io_insert = &mbsys_mr1v2001_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_mr1v2001_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_mr1v2001_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_mr1v2001_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_mr1v2001_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_mr1v2001_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_mr1v2001_copy;
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
int mbr_info_mr1prvr2(int verbose,
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
	char	*function_name = "mbr_info_mr1prvr2";
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
	*system = MB_SYS_MR1;
	*beams_bath_max = MBSYS_MR1V2001_BEAMS;
	*beams_amp_max = 0;
	*pixels_ss_max = MBSYS_MR1V2001_PIXELS;
	strncpy(format_name, "MR1PRVR2", MB_NAME_LENGTH);
	strncpy(system_name, "MR1", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_MR1PRVR2\nInformal Description: SOEST MR1 post processed format\nAttributes:           SOEST MR1, bathymetry and sidescan,\n                      variable beams and pixels, xdr binary, \n                      SOEST, University of Hawaii.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_XDR;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 2.0;

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
int mbr_alm_mr1prvr2(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_mr1prvr2";
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

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	status = mbsys_mr1v2001_alloc(verbose, mbio_ptr,
			(void **)&mb_io_ptr->store_data,
			error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* initialize everything to zeros */
	mb_io_ptr->fileheader = MB_NO;
	mb_io_ptr->hdr_comment_size = 0;
	mb_io_ptr->hdr_comment = NULL;

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
int mbr_dem_mr1prvr2(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_mr1prvr2";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_mr1v2001_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_mr1v2001_struct *) mb_io_ptr->store_data;

	/* deallocate memory for data descriptor */
	if (store->mr1buffersize > 0
		&& store->mr1buffer != NULL)
		free(store->mr1buffer);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&mb_io_ptr->store_data,error);

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
int mbr_rt_mr1prvr2(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_mr1prvr2";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_mr1v2001_struct *store;

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
	store = (struct mbsys_mr1v2001_struct *) store_ptr;

	/* read next data from file */
	status = mbr_mr1prvr2_rd_data(verbose,mbio_ptr,error);

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
int mbr_wt_mr1prvr2(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_mr1prvr2";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_mr1v2001_struct *store;

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
	store = (struct mbsys_mr1v2001_struct *) store_ptr;

	/* write next data to file */
	status = mbr_mr1prvr2_wr_data(verbose,mbio_ptr,store_ptr,error);

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
int mbr_mr1prvr2_rd_data(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_mr1prvr2_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_mr1v2001_struct *store;
	char	*store_ptr;
	char	*xdrs;
	int	read_size;
	int	mr1pr_status = MR1_SUCCESS;
	char	*eol;
	int	i;

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

	/* get pointer to data */
	store = (struct mbsys_mr1v2001_struct *) mb_io_ptr->store_data;
	store_ptr = (char *) store;
	xdrs = mb_io_ptr->xdrs;

	/* if first time through read file header */
	if (mb_io_ptr->fileheader == MB_NO)
		{
		/* read the header into memory */
		mr1pr_status = mr1_rdmrfhdr(&(store->header), (XDR *)xdrs);
		if (mr1pr_status == MR1_SUCCESS)
			{
			mb_io_ptr->fileheader = MB_YES;
			status = MB_SUCCESS;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* extract the comments string */
		mb_io_ptr->hdr_comment_size = 0;
		if (mb_io_ptr->hdr_comment != NULL)
			mb_freed(verbose,__FILE__, __LINE__, (void **)&mb_io_ptr->hdr_comment,error);
		if (status == MB_SUCCESS
			&& store->header.mf_count > 0)
			{
			status = mb_mallocd(verbose,__FILE__,__LINE__,strlen(store->header.mf_log)+1,
					(void **)&mb_io_ptr->hdr_comment,error);
			}
		if (status == MB_SUCCESS)
			{
			strcpy(mb_io_ptr->hdr_comment,store->header.mf_log);
			if (mb_io_ptr->hdr_comment == NULL)
				mb_io_ptr->hdr_comment_size = 0;
			else
				mb_io_ptr->hdr_comment_size
					= strlen(mb_io_ptr->hdr_comment);
			mb_io_ptr->hdr_comment_loc = 0;
			}

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg5       mf_version:       %d\n",store->header.mf_version);
			fprintf(stderr,"dbg5       mf_count:         %d\n",store->header.mf_count);
			fprintf(stderr,"dbg5       mf_log:         \n%s\n",store->header.mf_log);
			}
		}

	/* if comments are still held in mb_io_ptr->hdr_comment then
		extract comment and return */
	if (mb_io_ptr->hdr_comment_size > mb_io_ptr->hdr_comment_loc)
		{
		eol = strchr(&mb_io_ptr->hdr_comment[mb_io_ptr->hdr_comment_loc], '\n');
		if (eol == NULL)
			read_size = strlen(&mb_io_ptr->hdr_comment[mb_io_ptr->hdr_comment_loc]);
		else
			read_size = strlen(&mb_io_ptr->hdr_comment[mb_io_ptr->hdr_comment_loc]) - strlen(eol);
		if (read_size > MBSYS_MR1V2001_MAXLINE - 1)
			{
			read_size = MBSYS_MR1V2001_MAXLINE - 1;
			eol = NULL;
			}
		strncpy(store->comment,
			&mb_io_ptr->hdr_comment[mb_io_ptr->hdr_comment_loc],
			read_size);
		store->comment[read_size] = '\0';
		mb_io_ptr->hdr_comment_loc += read_size;
		if (eol != NULL)
			mb_io_ptr->hdr_comment_loc++;
		store->kind = MB_DATA_COMMENT;
		}

	/* else read data */
	else
		{
		if ((mr1pr_status = mr1_rdpnghdr(&(store->ping), (XDR *)xdrs,
						store->header.mf_version))
						!= MR1_SUCCESS)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		if (status == MB_SUCCESS
			&& (mr1pr_status = mr1_pngrealloc(&(store->ping),
						&(store->mr1buffer), &(store->mr1buffersize)))
						!= MR1_SUCCESS)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
			}
		if (status == MB_SUCCESS
			&& (mr1pr_status = mr1_rdpngdata(&(store->ping),
						(float *)store->mr1buffer, (XDR *)xdrs))
						!= MR1_SUCCESS)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		if (status == MB_SUCCESS
			&& (mr1pr_status = mr1_getpngdataptrs(&(store->ping),
						(float *)store->mr1buffer,
						&(store->compass), &(store->depth),
						&(store->pitch), &(store->roll),
						&(store->pbty), &(store->pss),
						&(store->sbty), &(store->sss))) != MR1_SUCCESS)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		if (status == MB_SUCCESS)
			{
			store->kind = MB_DATA_DATA;
			}

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg5       sec:              %d\n",store->ping.png_tm.tv_sec);
			fprintf(stderr,"dbg5       usec:             %d\n",store->ping.png_tm.tv_usec);
			fprintf(stderr,"dbg5       period:           %f\n",store->ping.png_period);
			fprintf(stderr,"dbg5       ship longitude:   %f\n",store->ping.png_slon);
			fprintf(stderr,"dbg5       ship latitude:    %f\n",store->ping.png_slat);
			fprintf(stderr,"dbg5       ship course:      %f\n",store->ping.png_scourse);
			fprintf(stderr,"dbg5       layback range:    %f\n",store->ping.png_laybackrng);
			fprintf(stderr,"dbg5       layback bearing:  %f\n",store->ping.png_laybackbrg);
			fprintf(stderr,"dbg5       towfish longitude:%f\n",store->ping.png_tlon);
			fprintf(stderr,"dbg5       towfish latitude: %f\n",store->ping.png_tlat);
			fprintf(stderr,"dbg5       towfish course:   %f\n",store->ping.png_tcourse);
			fprintf(stderr,"dbg5       compass ptr:      %lu\n",(size_t)store->compass);
			fprintf(stderr,"dbg5       towfish compass interval:  %f\n",store->ping.png_compass.sns_int);
			fprintf(stderr,"dbg5       towfish compass samples:   %d\n",store->ping.png_compass.sns_nsamps);
			fprintf(stderr,"dbg5       towfish compass value:     %f\n",store->ping.png_compass.sns_repval);
			fprintf(stderr,"dbg5       towfish compass  heading:\n");
			for (i=0;i<store->ping.png_compass.sns_nsamps;i++)
			  {
			  fprintf(stderr,"dbg5         %3d     %12.4g\n",i,store->compass[i]);
			  }
			fprintf(stderr,"dbg5       depth ptr:        %lu\n",(size_t)store->depth);
			fprintf(stderr,"dbg5       towfish depth interval:    %f\n",store->ping.png_depth.sns_int);
			fprintf(stderr,"dbg5       towfish depth samples:     %d\n",store->ping.png_depth.sns_nsamps);
			fprintf(stderr,"dbg5       towfish depth value:       %f\n",store->ping.png_depth.sns_repval);
			fprintf(stderr,"dbg5       towfish depth:\n");
			for (i=0;i<store->ping.png_depth.sns_nsamps;i++)
			  {
			  fprintf(stderr,"dbg5         %3d     %12.4g\n",i,store->depth[i]);
			  }
			fprintf(stderr,"dbg5       pitch ptr:        %lu\n",(size_t)store->pitch);
			fprintf(stderr,"dbg5       towfish pitch interval:    %f\n",store->ping.png_pitch.sns_int);
			fprintf(stderr,"dbg5       towfish pitch samples:     %d\n",store->ping.png_pitch.sns_nsamps);
			fprintf(stderr,"dbg5       towfish pitch value:       %f\n",store->ping.png_pitch.sns_repval);
			fprintf(stderr,"dbg5       towfish pitch:\n");
			for (i=0;i<store->ping.png_pitch.sns_nsamps;i++)
			  {
			  fprintf(stderr,"dbg5         %3d     %12.4g\n",i,store->pitch[i]);
			  }
			fprintf(stderr,"dbg5       roll ptr:         %lu\n",(size_t)store->roll);
			fprintf(stderr,"dbg5       towfish roll interval:     %f\n",store->ping.png_roll.sns_int);
			fprintf(stderr,"dbg5       towfish roll samples:      %d\n",store->ping.png_roll.sns_nsamps);
			fprintf(stderr,"dbg5       towfish roll value:        %f\n",store->ping.png_roll.sns_repval);
			fprintf(stderr,"dbg5       towfish roll:\n");
			for (i=0;i<store->ping.png_roll.sns_nsamps;i++)
			  {
			  fprintf(stderr,"dbg5         %3d     %12.4g\n",i,store->roll[i]);
			  }
			fprintf(stderr,"dbg5       png_snspad:       %d\n",store->ping.png_snspad);
			fprintf(stderr,"dbg5       png_temp:         %f\n",store->ping.png_temp);
			fprintf(stderr,"dbg5       png_atssincr:     %f\n",store->ping.png_atssincr);
			fprintf(stderr,"dbg5       png_alt:          %f\n",store->ping.png_alt);
			fprintf(stderr,"dbg5       png_magcorr:      %f\n",store->ping.png_magcorr);
			fprintf(stderr,"dbg5       png_sndvel:       %f\n",store->ping.png_sndvel);
			fprintf(stderr,"dbg5       port ps_xmitpwr:  %f\n",store->ping.png_sides[ACP_PORT].ps_xmitpwr);
			fprintf(stderr,"dbg5       port ps_gain:     %f\n",store->ping.png_sides[ACP_PORT].ps_gain);
			fprintf(stderr,"dbg5       port ps_pulse:    %f\n",store->ping.png_sides[ACP_PORT].ps_pulse);
			fprintf(stderr,"dbg5       port ps_bdrange:  %f\n",store->ping.png_sides[ACP_PORT].ps_bdrange);
			fprintf(stderr,"dbg5       port ps_btycount: %d\n",store->ping.png_sides[ACP_PORT].ps_btycount);
			fprintf(stderr,"dbg5       port ps_btypad:   %d\n",store->ping.png_sides[ACP_PORT].ps_btypad);
			fprintf(stderr,"dbg5       port bty ptr:     %lu\n",(size_t)store->pbty);
			fprintf(stderr,"dbg5       port xtrack bathymetry:\n");
			for (i=0;i<store->ping.png_sides[ACP_PORT].ps_btycount;i++)
			  {
			  fprintf(stderr,"dbg5         %3d     %12.4g %12.4g\n",i,store->pbty[2*i],store->pbty[2*i+1]);
			  }
			fprintf(stderr,"dbg5       port ps_ssoffset: %f\n",store->ping.png_sides[ACP_PORT].ps_ssoffset);
			fprintf(stderr,"dbg5       port ps_sscount:  %d\n",store->ping.png_sides[ACP_PORT].ps_sscount);
			fprintf(stderr,"dbg5       port ps_sspad:    %d\n",store->ping.png_sides[ACP_PORT].ps_sspad);
			fprintf(stderr,"dbg5       port ss ptr:      %lu\n",(size_t)store->pss);
			fprintf(stderr,"dbg5       port sidescan:\n");
			for (i=0;i<store->ping.png_sides[ACP_PORT].ps_sscount;i++)
			  {
			  fprintf(stderr,"dbg5         %3d     %12.4g\n",i,store->pss[i]);
			  }

			fprintf(stderr,"dbg5       stbd ps_xmitpwr:  %f\n",store->ping.png_sides[ACP_STBD].ps_xmitpwr);
			fprintf(stderr,"dbg5       stbd ps_gain:     %f\n",store->ping.png_sides[ACP_STBD].ps_gain);
			fprintf(stderr,"dbg5       stbd ps_pulse:    %f\n",store->ping.png_sides[ACP_STBD].ps_pulse);
			fprintf(stderr,"dbg5       stbd ps_bdrange:  %f\n",store->ping.png_sides[ACP_STBD].ps_bdrange);
			fprintf(stderr,"dbg5       stbd ps_btycount: %d\n",store->ping.png_sides[ACP_STBD].ps_btycount);
			fprintf(stderr,"dbg5       stbd ps_btypad:   %d\n",store->ping.png_sides[ACP_STBD].ps_btypad);
			fprintf(stderr,"dbg5       stbd bty ptr:     %lu\n",(size_t)store->sbty);
			fprintf(stderr,"dbg5       stbd xtrack bathymetry:\n");
			for (i=0;i<store->ping.png_sides[ACP_STBD].ps_btycount;i++)
			  {
			  fprintf(stderr,"dbg5         %3d     %12.4g %12.4g\n",i,store->sbty[2*i],store->sbty[2*i+1]);
			  }
			fprintf(stderr,"dbg5       stbd ps_ssoffset: %f\n",store->ping.png_sides[ACP_STBD].ps_ssoffset);
			fprintf(stderr,"dbg5       stbd ps_sscount:  %d\n",store->ping.png_sides[ACP_STBD].ps_sscount);
			fprintf(stderr,"dbg5       stbd ps_sspad:    %d\n",store->ping.png_sides[ACP_STBD].ps_sspad);
			fprintf(stderr,"dbg5       stbd ss ptr:      %lu\n",(size_t)store->sss);
			fprintf(stderr,"dbg5       stbd sidescan:\n");
			for (i=0;i<store->ping.png_sides[ACP_STBD].ps_sscount;i++)
			  {
			  fprintf(stderr,"dbg5         %3d     %12.4g\n",i,store->sss[i]);
			  }
			fprintf(stderr,"\n");
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
int mbr_mr1prvr2_wr_data(int verbose, void *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_mr1prvr2_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_mr1v2001_struct *store;
	char	*xdrs;
	int	mr1pr_status = MR1_SUCCESS;
	int	i;

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

	/* get pointer to data */
	store = (struct mbsys_mr1v2001_struct *) store_ptr;
	xdrs = mb_io_ptr->xdrs;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       sec:              %d\n",store->ping.png_tm.tv_sec);
		fprintf(stderr,"dbg5       usec:             %d\n",store->ping.png_tm.tv_usec);
		fprintf(stderr,"dbg5       period:           %f\n",store->ping.png_period);
		fprintf(stderr,"dbg5       ship longitude:   %f\n",store->ping.png_slon);
		fprintf(stderr,"dbg5       ship latitude:    %f\n",store->ping.png_slat);
		fprintf(stderr,"dbg5       ship course:      %f\n",store->ping.png_scourse);
		fprintf(stderr,"dbg5       layback range:    %f\n",store->ping.png_laybackrng);
		fprintf(stderr,"dbg5       layback bearing:  %f\n",store->ping.png_laybackbrg);
		fprintf(stderr,"dbg5       towfish longitude:%f\n",store->ping.png_tlon);
		fprintf(stderr,"dbg5       towfish latitude: %f\n",store->ping.png_tlat);
		fprintf(stderr,"dbg5       towfish course:   %f\n",store->ping.png_tcourse);
		fprintf(stderr,"dbg5       compass ptr:      %lu\n",(size_t)store->compass);
		fprintf(stderr,"dbg5       towfish compass interval:  %f\n",store->ping.png_compass.sns_int);
		fprintf(stderr,"dbg5       towfish compass samples:   %d\n",store->ping.png_compass.sns_nsamps);
		fprintf(stderr,"dbg5       towfish compass value:     %f\n",store->ping.png_compass.sns_repval);
		fprintf(stderr,"dbg5       towfish compass  heading:\n");
		for (i=0;i<store->ping.png_compass.sns_nsamps;i++)
		  {
		  fprintf(stderr,"dbg5         %3d     %12.4g\n",i,store->compass[i]);
		  }
		fprintf(stderr,"dbg5       depth ptr:        %lu\n",(size_t)store->depth);
		fprintf(stderr,"dbg5       towfish depth interval:    %f\n",store->ping.png_depth.sns_int);
		fprintf(stderr,"dbg5       towfish depth samples:     %d\n",store->ping.png_depth.sns_nsamps);
		fprintf(stderr,"dbg5       towfish depth value:       %f\n",store->ping.png_depth.sns_repval);
		fprintf(stderr,"dbg5       towfish depth:\n");
		for (i=0;i<store->ping.png_depth.sns_nsamps;i++)
		  {
		  fprintf(stderr,"dbg5         %3d     %12.4g\n",i,store->depth[i]);
		  }
		fprintf(stderr,"dbg5       pitch ptr:        %lu\n",(size_t)store->pitch);
		fprintf(stderr,"dbg5       towfish pitch interval:    %f\n",store->ping.png_pitch.sns_int);
		fprintf(stderr,"dbg5       towfish pitch samples:     %d\n",store->ping.png_pitch.sns_nsamps);
		fprintf(stderr,"dbg5       towfish pitch value:       %f\n",store->ping.png_pitch.sns_repval);
		fprintf(stderr,"dbg5       towfish pitch:\n");
		for (i=0;i<store->ping.png_pitch.sns_nsamps;i++)
		  {
		  fprintf(stderr,"dbg5         %3d     %12.4g\n",i,store->pitch[i]);
		  }
		fprintf(stderr,"dbg5       roll ptr:         %lu\n",(size_t)store->roll);
		fprintf(stderr,"dbg5       towfish roll interval:     %f\n",store->ping.png_roll.sns_int);
		fprintf(stderr,"dbg5       towfish roll samples:      %d\n",store->ping.png_roll.sns_nsamps);
		fprintf(stderr,"dbg5       towfish roll value:        %f\n",store->ping.png_roll.sns_repval);
		fprintf(stderr,"dbg5       towfish roll:\n");
		for (i=0;i<store->ping.png_roll.sns_nsamps;i++)
		  {
		  fprintf(stderr,"dbg5         %3d     %12.4g\n",i,store->roll[i]);
		  }
		fprintf(stderr,"dbg5       png_snspad:       %d\n",store->ping.png_snspad);
		fprintf(stderr,"dbg5       png_temp:         %f\n",store->ping.png_temp);
		fprintf(stderr,"dbg5       png_atssincr:     %f\n",store->ping.png_atssincr);
		fprintf(stderr,"dbg5       png_alt:          %f\n",store->ping.png_alt);
		fprintf(stderr,"dbg5       png_magcorr:      %f\n",store->ping.png_magcorr);
		fprintf(stderr,"dbg5       png_sndvel:       %f\n",store->ping.png_sndvel);
		fprintf(stderr,"dbg5       port ps_xmitpwr:  %f\n",store->ping.png_sides[ACP_PORT].ps_xmitpwr);
		fprintf(stderr,"dbg5       port ps_gain:     %f\n",store->ping.png_sides[ACP_PORT].ps_gain);
		fprintf(stderr,"dbg5       port ps_pulse:    %f\n",store->ping.png_sides[ACP_PORT].ps_pulse);
		fprintf(stderr,"dbg5       port ps_bdrange:  %f\n",store->ping.png_sides[ACP_PORT].ps_bdrange);
		fprintf(stderr,"dbg5       port ps_btycount: %d\n",store->ping.png_sides[ACP_PORT].ps_btycount);
		fprintf(stderr,"dbg5       port ps_btypad:   %d\n",store->ping.png_sides[ACP_PORT].ps_btypad);
		fprintf(stderr,"dbg5       port bty ptr:     %lu\n",(size_t)store->pbty);
		fprintf(stderr,"dbg5       port bathymetry xtrack:\n");
		for (i=0;i<store->ping.png_sides[ACP_PORT].ps_btycount;i++)
		  {
		  fprintf(stderr,"dbg5         %3d     %12.4g %12.4g\n",i,store->pbty[2*i],store->pbty[2*i+1]);
		  }
		fprintf(stderr,"dbg5       port ps_ssoffset: %f\n",store->ping.png_sides[ACP_PORT].ps_ssoffset);
		fprintf(stderr,"dbg5       port ps_sscount:  %d\n",store->ping.png_sides[ACP_PORT].ps_sscount);
		fprintf(stderr,"dbg5       port ps_sspad:    %d\n",store->ping.png_sides[ACP_PORT].ps_sspad);
		fprintf(stderr,"dbg5       port ss ptr:      %lu\n",(size_t)store->pss);
		fprintf(stderr,"dbg5       port sidescan:\n");
		for (i=0;i<store->ping.png_sides[ACP_PORT].ps_sscount;i++)
		  {
		  fprintf(stderr,"dbg5         %3d     %12.4g\n",i,store->pss[i]);
		  }

		fprintf(stderr,"dbg5       stbd ps_xmitpwr:  %f\n",store->ping.png_sides[ACP_STBD].ps_xmitpwr);
		fprintf(stderr,"dbg5       stbd ps_gain:     %f\n",store->ping.png_sides[ACP_STBD].ps_gain);
		fprintf(stderr,"dbg5       stbd ps_pulse:    %f\n",store->ping.png_sides[ACP_STBD].ps_pulse);
		fprintf(stderr,"dbg5       stbd ps_bdrange:  %f\n",store->ping.png_sides[ACP_STBD].ps_bdrange);
		fprintf(stderr,"dbg5       stbd ps_btycount: %d\n",store->ping.png_sides[ACP_STBD].ps_btycount);
		fprintf(stderr,"dbg5       stbd ps_btypad:   %d\n",store->ping.png_sides[ACP_STBD].ps_btypad);
		fprintf(stderr,"dbg5       stbd bty ptr:     %lu\n",(size_t)store->pbty);
		fprintf(stderr,"dbg5       stbd bathymetry xtrack:\n");
		for (i=0;i<store->ping.png_sides[ACP_STBD].ps_btycount;i++)
		  {
		  fprintf(stderr,"dbg5         %3d     %12.4g %12.4g\n",i,store->sbty[2*i],store->sbty[2*i+1]);
		  }
		fprintf(stderr,"dbg5       stbd ps_ssoffset: %f\n",store->ping.png_sides[ACP_STBD].ps_ssoffset);
		fprintf(stderr,"dbg5       stbd ps_sscount:  %d\n",store->ping.png_sides[ACP_STBD].ps_sscount);
		fprintf(stderr,"dbg5       stbd ps_sspad:    %d\n",store->ping.png_sides[ACP_STBD].ps_sspad);
		fprintf(stderr,"dbg5       stbd ss ptr:      %lu\n",(size_t)store->sss);
		fprintf(stderr,"dbg5       stbd sidescan:\n");
		for (i=0;i<store->ping.png_sides[ACP_STBD].ps_sscount;i++)
		  {
		  fprintf(stderr,"dbg5         %3d     %12.4g\n",i,store->sss[i]);
		  }
		fprintf(stderr,"\n");
		}

	/* if comment and file header not written */
	if (mb_io_ptr->fileheader == MB_NO && store->kind == MB_DATA_COMMENT)
		{
		/* add comment to string mb_io_ptr->hdr_comment
			to be be written in file header */
		mb_io_ptr->hdr_comment_size += strlen(store->comment) + 2;
		status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->hdr_comment_size,
					(void **)&(mb_io_ptr->hdr_comment),
					error);
		strcat(mb_io_ptr->hdr_comment,store->comment);
		strcat(mb_io_ptr->hdr_comment,"\n");
		}

	/* if data and file header not written */
	else if (mb_io_ptr->fileheader == MB_NO
		&& store->kind != MB_DATA_COMMENT)
		{
		/* insert new comments into file header */
		mr1_replacestr(&(store->header.mf_log), mb_io_ptr->hdr_comment);

		/* write file header */
		if ((mr1pr_status = mr1_wrmrfhdr(&(store->header),
				(XDR *)xdrs))
				!= MR1_SUCCESS)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			mb_io_ptr->fileheader = MB_YES;
		}

	/* if data and file header written */
	if (mb_io_ptr->fileheader == MB_YES
		&& store->kind == MB_DATA_DATA)
		{
		/* write data */
		if ((mr1pr_status = mr1_wrpnghdr(&(store->ping),
				(XDR *)xdrs))
				!= MR1_SUCCESS)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		if ((mr1pr_status = mr1_wrpngdata(&(store->ping),
					(float *)store->mr1buffer,
					(XDR *)xdrs))
					!= MR1_SUCCESS)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		}

	/* if not data and file header written */
	else if (store->kind != MB_DATA_COMMENT
		&& store->kind != MB_DATA_DATA)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
		}

	/* print output debug statements */
	if (verbose >= 5)
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
