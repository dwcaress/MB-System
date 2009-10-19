/*--------------------------------------------------------------------
 *    The MB-system:	mbr_nvnetcdf.c	5/4/02
 *	$Id: mbr_nvnetcdf.c,v 5.6 2008/07/10 06:43:41 caress Exp $
 *
 *    Copyright (c) 2002-2008 by
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
 * mbr_nvnetcdf.c contains the functions for reading and writing
 * multibeam data in the MBF_MBNETCDF format.  
 * These functions include:
 *   mbr_alm_nvnetcdf	- allocate read/write memory
 *   mbr_dem_nvnetcdf	- deallocate read/write memory
 *   mbr_rt_nvnetcdf	- read and translate data
 *   mbr_wt_nvnetcdf	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	May 4, 2002
 * 
 * $Log: mbr_nvnetcdf.c,v $
 * Revision 5.6  2008/07/10 06:43:41  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.5  2008/05/16 22:56:24  caress
 * Release 5.1.1beta18.
 *
 * Revision 5.4  2005/11/05 00:48:04  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.3  2005/03/26 22:05:17  caress
 * Release 5.0.7.
 *
 * Revision 5.2  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.1  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.0  2002/05/29 23:39:23  caress
 * Release 5.0.beta18
 *
 *
 *
 */
/* #define MBNETCDF_DEBUG 1 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <netcdf.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"
#include "../../include/mbsys_navnetcdf.h"

/* essential function prototypes */
int mbr_register_nvnetcdf(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_nvnetcdf(int verbose, 
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
int mbr_alm_nvnetcdf(int verbose, void *mbio_ptr, int *error);
int mbr_dem_nvnetcdf(int verbose, void *mbio_ptr, int *error);
int mbr_rt_nvnetcdf(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_nvnetcdf(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char res_id[]="$Id: mbr_nvnetcdf.c,v 5.6 2008/07/10 06:43:41 caress Exp $";

/*--------------------------------------------------------------------*/
int mbr_register_nvnetcdf(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_nvnetcdf";
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
	status = mbr_info_nvnetcdf(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_nvnetcdf;
	mb_io_ptr->mb_io_format_free = &mbr_dem_nvnetcdf; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_navnetcdf_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_navnetcdf_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_nvnetcdf; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_nvnetcdf; 
	mb_io_ptr->mb_io_dimensions = &mbsys_navnetcdf_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_navnetcdf_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_navnetcdf_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_navnetcdf_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_navnetcdf_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_navnetcdf_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = &mbsys_navnetcdf_insert_altitude; 
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL; 
	mb_io_ptr->mb_io_ttimes = &mbsys_navnetcdf_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_navnetcdf_copy; 
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
int mbr_info_nvnetcdf(int verbose, 
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
	char	*function_name = "mbr_info_nvnetcdf";
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
	*system = MB_SYS_NAVNETCDF;
	*beams_bath_max = 0;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "NVNETCDF", MB_NAME_LENGTH);
	strncpy(system_name, "NAVNETCDF", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_NVNETCDF\nInformal Description: CARAIBES CDF navigation\nAttributes:           netCDF, IFREMER.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NETCDF;
	*variable_beams = MB_NO;
	*traveltime = MB_NO;
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
int mbr_alm_nvnetcdf(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_nvnetcdf";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;
	int	*dataread;
	int	*commentread;
	int	*recread;

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
	status = mbsys_navnetcdf_alloc(verbose,mbio_ptr,
		&mb_io_ptr->store_data,error);

	/* get pointer to raw data structure */
	store = (struct mbsys_navnetcdf_struct *) mb_io_ptr->store_data;

	/* initialize values in structure */
	dataread = (int *) &mb_io_ptr->save1;
	commentread = (int *) &mb_io_ptr->save2;
	recread = (int *) &mb_io_ptr->save4;
	*dataread = MB_NO;
	*commentread = 0;
	*recread = 0;

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
int mbr_dem_nvnetcdf(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_nvnetcdf";
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

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mbsys_navnetcdf_deall(verbose,mbio_ptr,
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
int mbr_rt_nvnetcdf(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_nvnetcdf";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;
	int	*dataread;
	int	*commentread;
	int	*recread;
	int	dim_id;
	size_t	index[2], count[2];
	int nc_status;
	int	i;
#ifdef MBNETCDF_DEBUG
	int	nc_verbose = 1;
#else
	int	nc_verbose = 0;
#endif

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

	/* get pointer to mbio descriptor and data structure */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_navnetcdf_struct *) store_ptr;
	dataread = (int *) &mb_io_ptr->save1;
	commentread = (int *) &mb_io_ptr->save2;
	recread = (int *) &mb_io_ptr->save4;
	
	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* if first read then set everything up */
	if (*dataread == MB_NO)
	    {
	    *dataread = MB_YES;

	    /* get dimensions */
	    nc_status = nc_inq_dimid((int)mb_io_ptr->mbfp, "mbHistoryRecNbr", &dim_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_dimid mbHistoryRecNbr error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_inq_dimlen((int)mb_io_ptr->mbfp, dim_id, &store->mbHistoryRecNbr);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_dimlen mbHistoryRecNbr error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_inq_dimid((int)mb_io_ptr->mbfp, "mbNameLength", &dim_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_dimid mbNameLength error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_inq_dimlen((int)mb_io_ptr->mbfp, dim_id, &store->mbNameLength);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_dimlen mbNameLength error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_inq_dimid((int)mb_io_ptr->mbfp, "mbCommentLength", &dim_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_dimid mbCommentLength error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_inq_dimlen((int)mb_io_ptr->mbfp, dim_id, &store->mbCommentLength);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_dimlen mbCommentLength error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_inq_dimid((int)mb_io_ptr->mbfp, "mbPositionNbr", &dim_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_dimid mbPositionNbr error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_inq_dimlen((int)mb_io_ptr->mbfp, dim_id, &store->mbPositionNbr);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_dimlen mbPositionNbr error: %s\n", nc_strerror(nc_status));
	    if (nc_status != NC_NOERR)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}		
		
	    /* print input debug statements */
	    if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  NetCDF array dimensions read in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Array and variable dimensions:\n");
		fprintf(stderr,"dbg2       status:                  %d\n", status);
		fprintf(stderr,"dbg2       error:                   %d\n", *error);
		fprintf(stderr,"dbg2       nc_status:               %d\n", nc_status);
		fprintf(stderr,"dbg2       mbHistoryRecNbr:         %d\n", store->mbHistoryRecNbr);
		fprintf(stderr,"dbg2       mbNameLength:            %d\n", store->mbNameLength);
		fprintf(stderr,"dbg2       mbCommentLength:         %d\n", store->mbCommentLength);
		fprintf(stderr,"dbg2       mbPositionNbr:              %d\n", store->mbPositionNbr);
		}
		
	    /* get global attributes */
	    if (status == MB_SUCCESS)
		{
		nc_status = nc_get_att_short((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbVersion", &store->mbVersion);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbVersion error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbName", store->mbName);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbName error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbClasse", store->mbClasse);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbClasse error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_short((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbLevel", &store->mbLevel);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbLevel error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_short((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbNbrHistoryRec", &store->mbNbrHistoryRec);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbNbrHistoryRec error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbTimeReference", store->mbTimeReference);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTimeReference error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbStartDate", &store->mbStartDate);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbStartDate error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbStartTime", &store->mbStartTime);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbStartTime error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbEndDate", &store->mbEndDate);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbEndDate error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbEndTime", &store->mbEndTime);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbEndTime error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbNorthLatitude", &store->mbNorthLatitude);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbNorthLatitude error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbSouthLatitude", &store->mbSouthLatitude);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSouthLatitude error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbEastLongitude", &store->mbEastLongitude);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbEastLongitude error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbWestLongitude", &store->mbWestLongitude);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbWestLongitude error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbMeridian180", store->mbMeridian180);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbMeridian180 error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbGeoDictionnary", store->mbGeoDictionnary);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbGeoDictionnary error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbGeoRepresentation", store->mbGeoRepresentation);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbGeoRepresentation error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbGeodesicSystem", store->mbGeodesicSystem);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbGeodesicSystem error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbEllipsoidName", store->mbEllipsoidName);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbEllipsoidName error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbEllipsoidA", &store->mbEllipsoidA);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbEllipsoidA error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbEllipsoidInvF", &store->mbEllipsoidInvF);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbEllipsoidInvF error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbEllipsoidE2", &store->mbEllipsoidE2);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbEllipsoidE2 error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_short((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbProjType", &store->mbProjType);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbProjType error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbProjParameterValue", &store->mbProjParameterValue[0]);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbProjParameterValue error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbProjParameterCode", store->mbProjParameterCode);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbProjParameterCode error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbShip", store->mbShip);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbShip error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbSurvey", store->mbSurvey);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSurvey error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbReference", store->mbReference);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbReference error: %s\n", nc_strerror(nc_status));
		nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbPointCounter", &store->mbPointCounter);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPointCounter error: %s\n", nc_strerror(nc_status));
		if (nc_status != NC_NOERR)
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }		
		
		/* print input debug statements */
		if (verbose >= 2)
		    {
		    fprintf(stderr,"\ndbg2  NetCDF global attributes read in function <%s>\n",
			    function_name);
		    fprintf(stderr,"dbg2  Global attributes:\n");
		    fprintf(stderr,"dbg2       status:                  %d\n", status);
		    fprintf(stderr,"dbg2       error:                   %d\n", *error);
		    fprintf(stderr,"dbg2       nc_status:             %d\n", nc_status);
		    fprintf(stderr,"dbg2       mbVersion:             %d\n", store->mbVersion);
		    fprintf(stderr,"dbg2       mbName:                %s\n", store->mbName);
		    fprintf(stderr,"dbg2       mbClasse:              %s\n", store->mbClasse);
		    fprintf(stderr,"dbg2       mbLevel:               %d\n", store->mbLevel);
		    fprintf(stderr,"dbg2       mbNbrHistoryRec:       %d\n", store->mbNbrHistoryRec);
		    fprintf(stderr,"dbg2       mbTimeReference:       %s\n", store->mbTimeReference);
		    fprintf(stderr,"dbg2       mbStartDate:           %d\n", store->mbStartDate);
		    fprintf(stderr,"dbg2       mbStartTime:           %d\n", store->mbStartTime);
		    fprintf(stderr,"dbg2       mbEndDate:             %d\n", store->mbEndDate);
		    fprintf(stderr,"dbg2       mbEndTime:             %d\n", store->mbEndTime);
		    fprintf(stderr,"dbg2       mbNorthLatitude:       %f\n", store->mbNorthLatitude);
		    fprintf(stderr,"dbg2       mbSouthLatitude:       %f\n", store->mbSouthLatitude);
		    fprintf(stderr,"dbg2       mbEastLongitude:       %f\n", store->mbEastLongitude);
		    fprintf(stderr,"dbg2       mbWestLongitude:       %f\n", store->mbWestLongitude);
		    fprintf(stderr,"dbg2       mbMeridian180:         %s\n", store->mbMeridian180);
		    fprintf(stderr,"dbg2       mbGeoDictionnary:      %s\n", store->mbGeoDictionnary);
		    fprintf(stderr,"dbg2       mbGeoRepresentation:   %s\n", store->mbGeoRepresentation);
		    fprintf(stderr,"dbg2       mbGeodesicSystem:      %s\n", store->mbGeodesicSystem);
		    fprintf(stderr,"dbg2       mbEllipsoidName:       %s\n", store->mbEllipsoidName);
		    fprintf(stderr,"dbg2       mbEllipsoidA:          %f\n", store->mbEllipsoidA);
		    fprintf(stderr,"dbg2       mbEllipsoidInvF:       %f\n", store->mbEllipsoidInvF);
		    fprintf(stderr,"dbg2       mbEllipsoidE2:         %f\n", store->mbEllipsoidE2);
		    fprintf(stderr,"dbg2       mbProjType:            %d\n", store->mbProjType);
		    for (i=0;i<10;i++)
			fprintf(stderr,"dbg2       mbProjParameterValue[%d]:%f\n", i, store->mbProjParameterValue[i]);
		    fprintf(stderr,"dbg2       mbProjParameterCode:   %s\n", store->mbProjParameterCode);
		    fprintf(stderr,"dbg2       mbShip:                %s\n", store->mbShip);
		    fprintf(stderr,"dbg2       mbSurvey:              %s\n", store->mbSurvey);
		    fprintf(stderr,"dbg2       mbReference:           %s\n", store->mbReference);
		    fprintf(stderr,"dbg2       mbPointCounter:        %d\n", store->mbPointCounter);
		    }
		}

	    /* get variable IDs */
	    if (status == MB_SUCCESS)
		{
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbHistDate", &store->mbHistDate_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbHistDate_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbHistTime", &store->mbHistTime_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbHistTime_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbHistCode", &store->mbHistCode_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbHistCode_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbHistAutor", &store->mbHistAutor_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbHistAutor_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbHistModule", &store->mbHistModule_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbHistModule_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbHistComment", &store->mbHistComment_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbHistComment_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbDate", &store->mbDate_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbDate_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbTime", &store->mbTime_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbTime_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbOrdinate", &store->mbOrdinate_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbOrdinate_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbAbscissa", &store->mbAbscissa_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbAbscissa_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbAltitude", &store->mbAltitude_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbAltitude_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbImmersion", &store->mbImmersion_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbImmersion_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbHeading", &store->mbHeading_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbHeading_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbSpeed", &store->mbSpeed_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbSpeed_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbPType", &store->mbPType_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbPType_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbPQuality", &store->mbPQuality_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbPQuality_id error: %s\n", nc_strerror(nc_status));
		nc_status = nc_inq_varid((int)mb_io_ptr->mbfp, "mbPFlag", &store->mbPFlag_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_inq_varid mbPFlag_id error: %s\n", nc_strerror(nc_status));
		if (nc_status != NC_NOERR)
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }		
		
		/* print input debug statements */
		if (verbose >= 2)
		    {
		    fprintf(stderr,"\ndbg2  NetCDF variable ids read in function <%s>\n",
			    function_name);
		    fprintf(stderr,"dbg2  Variable ids:\n");
		    fprintf(stderr,"dbg2       status:                  %d\n", status);
		    fprintf(stderr,"dbg2       error:                   %d\n", *error);
		    fprintf(stderr,"dbg2       nc_status:               %d\n", nc_status);
		    fprintf(stderr,"dbg2       mbHistDate_id:           %d\n", store->mbHistDate_id);
		    fprintf(stderr,"dbg2       mbHistTime_id:           %d\n", store->mbHistTime_id);
		    fprintf(stderr,"dbg2       mbHistCode_id:           %d\n", store->mbHistCode_id);
		    fprintf(stderr,"dbg2       mbHistAutor_id:          %d\n", store->mbHistAutor_id);
		    fprintf(stderr,"dbg2       mbHistModule_id:         %d\n", store->mbHistModule_id);
		    fprintf(stderr,"dbg2       mbHistComment_id:        %d\n", store->mbHistComment_id);
		    fprintf(stderr,"dbg2       mbDate_id:               %d\n", store->mbDate_id);
		    fprintf(stderr,"dbg2       mbTime_id:               %d\n", store->mbTime_id);
		    fprintf(stderr,"dbg2       mbOrdinate_id:           %d\n", store->mbOrdinate_id);
		    fprintf(stderr,"dbg2       mbAbscissa_id:           %d\n", store->mbAbscissa_id);
		    fprintf(stderr,"dbg2       mbAltitude_id:           %d\n", store->mbAltitude_id);
		    fprintf(stderr,"dbg2       mbImmersion_id:          %d\n", store->mbImmersion_id);
		    fprintf(stderr,"dbg2       mbHeading_id:            %d\n", store->mbHeading_id);
		    fprintf(stderr,"dbg2       mbSpeed_id:              %d\n", store->mbSpeed_id);
		    fprintf(stderr,"dbg2       mbPType_id:              %d\n", store->mbPType_id);
		    fprintf(stderr,"dbg2       mbPQuality_id:           %d\n", store->mbPQuality_id);
		    fprintf(stderr,"dbg2       mbPFlag_id:              %d\n", store->mbPFlag_id);
		    }
		}
		
	    /* allocate memory for variables */
	    if (status == MB_SUCCESS)
		{
		status = mb_mallocd(verbose,__FILE__,__LINE__, 
			    store->mbHistoryRecNbr * sizeof(int),
			    (void **)&store->mbHistDate,error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, 
			    store->mbHistoryRecNbr * sizeof(int),
			    (void **)&store->mbHistTime,error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, 
			    store->mbHistoryRecNbr * sizeof(char),
			    (void **)&store->mbHistCode,error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, 
			    store->mbHistoryRecNbr * store->mbNameLength * sizeof(char),
			    (void **)&store->mbHistAutor,error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, 
			    store->mbHistoryRecNbr * store->mbNameLength * sizeof(char),
			    (void **)&store->mbHistModule,error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, 
			    store->mbHistoryRecNbr * store->mbCommentLength * sizeof(char),
			    (void **)&store->mbHistComment,error);

		/* deal with a memory allocation failure */
		if (status == MB_FAILURE)
		    {
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistDate, error);
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistTime, error);
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistCode, error);
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistAutor, error);
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistModule, error);
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistComment, error);
		    status = MB_FAILURE;
		    *error = MB_ERROR_MEMORY_FAIL;
		    if (verbose >= 2)
			    {
			    fprintf(stderr,"\ndbg2  MBIO function <%s> terminated with error\n",
				    function_name);
			    fprintf(stderr,"dbg2  Return values:\n");
			    fprintf(stderr,"dbg2       error:      %d\n",*error);
			    fprintf(stderr,"dbg2  Return status:\n");
			    fprintf(stderr,"dbg2       status:  %d\n",status);
			    }
		    return(status);
		    }
		}

	    /* read variable attributes */
	    if (status == MB_SUCCESS)
		{
		if (store->mbHistDate_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistDate_id, "type", store->mbHistDate_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistDate_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistDate_id, "long_name", store->mbHistDate_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistDate_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistDate_id, "name_code", store->mbHistDate_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistDate_name_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistDate_id, "units", store->mbHistDate_units);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistDate_units error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistDate_id, "unit_code", store->mbHistDate_unit_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistDate_unit_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistDate_id, "add_offset", &store->mbHistDate_add_offset);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistDate_add_offset error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistDate_id, "scale_factor", &store->mbHistDate_scale_factor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistDate_scale_factor error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistDate_id, "minimum", &store->mbHistDate_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistDate_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistDate_id, "maximum", &store->mbHistDate_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistDate_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistDate_id, "valid_minimum", &store->mbHistDate_valid_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistDate_valid_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistDate_id, "valid_maximum", &store->mbHistDate_valid_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistDate_valid_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistDate_id, "missing_value", &store->mbHistDate_missing_value);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistDate_missing_value error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistDate_id, "format_C", store->mbHistDate_format_C);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistDate_format_C error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistDate_id, "orientation", store->mbHistDate_orientation);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistDate_orientation error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbHistTime_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistTime_id, "type", store->mbHistTime_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistTime_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistTime_id, "long_name", store->mbHistTime_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistTime_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistTime_id, "name_code", store->mbHistTime_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistTime_name_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistTime_id, "units", store->mbHistTime_units);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistTime_units error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistTime_id, "unit_code", store->mbHistTime_unit_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistTime_unit_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistTime_id, "add_offset", &store->mbHistTime_add_offset);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistTime_add_offset error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistTime_id, "scale_factor", &store->mbHistTime_scale_factor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistTime_scale_factor error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistTime_id, "minimum", &store->mbHistTime_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistTime_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistTime_id, "maximum", &store->mbHistTime_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistTime_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistTime_id, "valid_minimum", &store->mbHistTime_valid_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistTime_valid_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistTime_id, "valid_maximum", &store->mbHistTime_valid_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistTime_valid_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistTime_id, "missing_value", &store->mbHistTime_missing_value);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistTime_missing_value error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistTime_id, "format_C", store->mbHistTime_format_C);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistTime_format_C error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistTime_id, "orientation", store->mbHistTime_orientation);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistTime_orientation error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbHistCode_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistCode_id, "type", store->mbHistCode_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistCode_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistCode_id, "long_name", store->mbHistCode_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistCode_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistCode_id, "name_code", store->mbHistCode_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistCode_name_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistCode_id, "units", store->mbHistCode_units);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistCode_units error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistCode_id, "unit_code", store->mbHistCode_unit_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistCode_unit_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistCode_id, "add_offset", &store->mbHistCode_add_offset);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistCode_add_offset error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistCode_id, "scale_factor", &store->mbHistCode_scale_factor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistCode_scale_factor error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistCode_id, "minimum", &store->mbHistCode_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistCode_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistCode_id, "maximum", &store->mbHistCode_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistCode_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistCode_id, "valid_minimum", &store->mbHistCode_valid_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistCode_valid_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistCode_id, "valid_maximum", &store->mbHistCode_valid_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistCode_valid_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHistCode_id, "missing_value", &store->mbHistCode_missing_value);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistCode_missing_value error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistCode_id, "format_C", store->mbHistCode_format_C);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistCode_format_C error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistCode_id, "orientation", store->mbHistCode_orientation);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistCode_orientation error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbHistAutor_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistAutor_id, "type", store->mbHistAutor_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistAutor_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistAutor_id, "long_name", store->mbHistAutor_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistAutor_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistAutor_id, "name_code", store->mbHistAutor_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistAutor_name_code error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbHistModule_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistModule_id, "type", store->mbHistModule_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistModule_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistModule_id, "long_name", store->mbHistModule_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistModule_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistModule_id, "name_code", store->mbHistModule_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistModule_name_code error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbHistComment_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistComment_id, "type", store->mbHistComment_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistComment_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistComment_id, "long_name", store->mbHistComment_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistComment_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHistComment_id, "name_code", store->mbHistComment_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHistComment_name_code error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbDate_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbDate_id, "type", store->mbDate_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbDate_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbDate_id, "long_name", store->mbDate_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbDate_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbDate_id, "name_code", store->mbDate_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbDate_name_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbDate_id, "units", store->mbDate_units);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbDate_units error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbDate_id, "unit_code", store->mbDate_unit_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbDate_unit_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbDate_id, "add_offset", &store->mbDate_add_offset);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbDate_add_offset error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbDate_id, "scale_factor", &store->mbDate_scale_factor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbDate_scale_factor error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbDate_id, "minimum", &store->mbDate_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbDate_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbDate_id, "maximum", &store->mbDate_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbDate_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbDate_id, "valid_minimum", &store->mbDate_valid_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbDate_valid_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbDate_id, "valid_maximum", &store->mbDate_valid_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbDate_valid_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbDate_id, "missing_value", &store->mbDate_missing_value);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbDate_missing_value error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbDate_id, "format_C", store->mbDate_format_C);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbDate_format_C error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbDate_id, "orientation", store->mbDate_orientation);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbDate_orientation error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbTime_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbTime_id, "type", store->mbTime_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTime_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbTime_id, "long_name", store->mbTime_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTime_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbTime_id, "name_code", store->mbTime_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTime_name_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbTime_id, "units", store->mbTime_units);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTime_units error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbTime_id, "unit_code", store->mbTime_unit_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTime_unit_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbTime_id, "add_offset", &store->mbTime_add_offset);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTime_add_offset error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbTime_id, "scale_factor", &store->mbTime_scale_factor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTime_scale_factor error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbTime_id, "minimum", &store->mbTime_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTime_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbTime_id, "maximum", &store->mbTime_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTime_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbTime_id, "valid_minimum", &store->mbTime_valid_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTime_valid_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbTime_id, "valid_maximum", &store->mbTime_valid_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTime_valid_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbTime_id, "missing_value", &store->mbTime_missing_value);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTime_missing_value error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbTime_id, "format_C", store->mbTime_format_C);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTime_format_C error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbTime_id, "orientation", store->mbTime_orientation);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbTime_orientation error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbOrdinate_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbOrdinate_id, "type", store->mbOrdinate_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbOrdinate_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbOrdinate_id, "long_name", store->mbOrdinate_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbOrdinate_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbOrdinate_id, "name_code", store->mbOrdinate_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbOrdinate_name_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbOrdinate_id, "units", store->mbOrdinate_units);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbOrdinate_units error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbOrdinate_id, "unit_code", store->mbOrdinate_unit_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbOrdinate_unit_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, store->mbOrdinate_id, "add_offset", &store->mbOrdinate_add_offset);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbOrdinate_add_offset error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, store->mbOrdinate_id, "scale_factor", &store->mbOrdinate_scale_factor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbOrdinate_scale_factor error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbOrdinate_id, "minimum", &store->mbOrdinate_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbOrdinate_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbOrdinate_id, "maximum", &store->mbOrdinate_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbOrdinate_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbOrdinate_id, "valid_minimum", &store->mbOrdinate_valid_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbOrdinate_valid_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbOrdinate_id, "valid_maximum", &store->mbOrdinate_valid_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbOrdinate_valid_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbOrdinate_id, "missing_value", &store->mbOrdinate_missing_value);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbOrdinate_missing_value error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbOrdinate_id, "format_C", store->mbOrdinate_format_C);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbOrdinate_format_C error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbOrdinate_id, "orientation", store->mbOrdinate_orientation);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbOrdinate_orientation error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbAbscissa_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbAbscissa_id, "type", store->mbAbscissa_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAbscissa_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbAbscissa_id, "long_name", store->mbAbscissa_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAbscissa_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbAbscissa_id, "name_code", store->mbAbscissa_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAbscissa_name_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbAbscissa_id, "units", store->mbAbscissa_units);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAbscissa_units error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbAbscissa_id, "unit_code", store->mbAbscissa_unit_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAbscissa_unit_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, store->mbAbscissa_id, "add_offset", &store->mbAbscissa_add_offset);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAbscissa_add_offset error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, store->mbAbscissa_id, "scale_factor", &store->mbAbscissa_scale_factor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAbscissa_scale_factor error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbAbscissa_id, "minimum", &store->mbAbscissa_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAbscissa_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbAbscissa_id, "maximum", &store->mbAbscissa_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAbscissa_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbAbscissa_id, "valid_minimum", &store->mbAbscissa_valid_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAbscissa_valid_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbAbscissa_id, "valid_maximum", &store->mbAbscissa_valid_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAbscissa_valid_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbAbscissa_id, "missing_value", &store->mbAbscissa_missing_value);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAbscissa_missing_value error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbAbscissa_id, "format_C", store->mbAbscissa_format_C);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAbscissa_format_C error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbAbscissa_id, "orientation", store->mbAbscissa_orientation);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAbscissa_orientation error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbAltitude_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbAltitude_id, "type", store->mbAltitude_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAltitude_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbAltitude_id, "long_name", store->mbAltitude_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAltitude_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbAltitude_id, "name_code", store->mbAltitude_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAltitude_name_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbAltitude_id, "units", store->mbAltitude_units);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAltitude_units error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbAltitude_id, "unit_code", store->mbAltitude_unit_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAltitude_unit_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, store->mbAltitude_id, "add_offset", &store->mbAltitude_add_offset);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAltitude_add_offset error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, store->mbAltitude_id, "scale_factor", &store->mbAltitude_scale_factor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAltitude_scale_factor error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbAltitude_id, "minimum", &store->mbAltitude_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAltitude_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbAltitude_id, "maximum", &store->mbAltitude_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAltitude_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbAltitude_id, "valid_minimum", &store->mbAltitude_valid_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAltitude_valid_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbAltitude_id, "valid_maximum", &store->mbAltitude_valid_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAltitude_valid_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbAltitude_id, "missing_value", &store->mbAltitude_missing_value);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAltitude_missing_value error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbAltitude_id, "format_C", store->mbAltitude_format_C);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAltitude_format_C error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbAltitude_id, "orientation", store->mbAltitude_orientation);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbAltitude_orientation error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbImmersion_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbImmersion_id, "type", store->mbImmersion_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbImmersion_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbImmersion_id, "long_name", store->mbImmersion_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbImmersion_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbImmersion_id, "name_code", store->mbImmersion_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbImmersion_name_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbImmersion_id, "units", store->mbImmersion_units);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbImmersion_units error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbImmersion_id, "unit_code", store->mbImmersion_unit_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbImmersion_unit_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, store->mbImmersion_id, "add_offset", &store->mbImmersion_add_offset);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbImmersion_add_offset error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, store->mbImmersion_id, "scale_factor", &store->mbImmersion_scale_factor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbImmersion_scale_factor error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbImmersion_id, "minimum", &store->mbImmersion_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbImmersion_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbImmersion_id, "maximum", &store->mbImmersion_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbImmersion_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbImmersion_id, "valid_minimum", &store->mbImmersion_valid_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbImmersion_valid_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbImmersion_id, "valid_maximum", &store->mbImmersion_valid_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbImmersion_valid_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbImmersion_id, "missing_value", &store->mbImmersion_missing_value);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbImmersion_missing_value error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbImmersion_id, "format_C", store->mbImmersion_format_C);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbImmersion_format_C error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbImmersion_id, "orientation", store->mbImmersion_orientation);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbImmersion_orientation error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbHeading_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHeading_id, "type", store->mbHeading_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHeading_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHeading_id, "long_name", store->mbHeading_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHeading_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHeading_id, "name_code", store->mbHeading_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHeading_name_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHeading_id, "units", store->mbHeading_units);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHeading_units error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHeading_id, "unit_code", store->mbHeading_unit_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHeading_unit_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, store->mbHeading_id, "add_offset", &store->mbHeading_add_offset);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHeading_add_offset error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, store->mbHeading_id, "scale_factor", &store->mbHeading_scale_factor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHeading_scale_factor error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHeading_id, "minimum", &store->mbHeading_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHeading_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHeading_id, "maximum", &store->mbHeading_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHeading_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHeading_id, "valid_minimum", &store->mbHeading_valid_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHeading_valid_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHeading_id, "valid_maximum", &store->mbHeading_valid_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHeading_valid_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbHeading_id, "missing_value", &store->mbHeading_missing_value);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHeading_missing_value error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHeading_id, "format_C", store->mbHeading_format_C);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHeading_format_C error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbHeading_id, "orientation", store->mbHeading_orientation);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbHeading_orientation error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbSpeed_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbSpeed_id, "type", store->mbSpeed_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSpeed_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbSpeed_id, "long_name", store->mbSpeed_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSpeed_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbSpeed_id, "name_code", store->mbSpeed_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSpeed_name_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbSpeed_id, "units", store->mbSpeed_units);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSpeed_units error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbSpeed_id, "unit_code", store->mbSpeed_unit_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSpeed_unit_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, store->mbSpeed_id, "add_offset", &store->mbSpeed_add_offset);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSpeed_add_offset error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_double((int)mb_io_ptr->mbfp, store->mbSpeed_id, "scale_factor", &store->mbSpeed_scale_factor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSpeed_scale_factor error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbSpeed_id, "minimum", &store->mbSpeed_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSpeed_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbSpeed_id, "maximum", &store->mbSpeed_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSpeed_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbSpeed_id, "valid_minimum", &store->mbSpeed_valid_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSpeed_valid_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbSpeed_id, "valid_maximum", &store->mbSpeed_valid_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSpeed_valid_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbSpeed_id, "missing_value", &store->mbSpeed_missing_value);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSpeed_missing_value error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbSpeed_id, "format_C", store->mbSpeed_format_C);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSpeed_format_C error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbSpeed_id, "orientation", store->mbSpeed_orientation);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbSpeed_orientation error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbPType_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPType_id, "type", store->mbPType_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPType_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPType_id, "long_name", store->mbPType_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPType_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPType_id, "name_code", store->mbPType_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPType_name_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPType_id, "units", store->mbPType_units);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPType_units error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPType_id, "unit_code", store->mbPType_unit_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPType_unit_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPType_id, "add_offset", &store->mbPType_add_offset);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPType_add_offset error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPType_id, "scale_factor", &store->mbPType_scale_factor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPType_scale_factor error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPType_id, "minimum", &store->mbPType_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPType_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPType_id, "maximum", &store->mbPType_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPType_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPType_id, "valid_minimum", &store->mbPType_valid_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPType_valid_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPType_id, "valid_maximum", &store->mbPType_valid_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPType_valid_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPType_id, "missing_value", &store->mbPType_missing_value);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPType_missing_value error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPType_id, "format_C", store->mbPType_format_C);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPType_format_C error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPType_id, "orientation", store->mbPType_orientation);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPType_orientation error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbPQuality_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPQuality_id, "type", store->mbPQuality_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPQuality_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPQuality_id, "long_name", store->mbPQuality_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPQuality_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPQuality_id, "name_code", store->mbPQuality_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPQuality_name_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPQuality_id, "units", store->mbPQuality_units);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPQuality_units error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPQuality_id, "unit_code", store->mbPQuality_unit_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPQuality_unit_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPQuality_id, "add_offset", &store->mbPQuality_add_offset);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPQuality_add_offset error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPQuality_id, "scale_factor", &store->mbPQuality_scale_factor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPQuality_scale_factor error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPQuality_id, "minimum", &store->mbPQuality_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPQuality_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPQuality_id, "maximum", &store->mbPQuality_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPQuality_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPQuality_id, "valid_minimum", &store->mbPQuality_valid_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPQuality_valid_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPQuality_id, "valid_maximum", &store->mbPQuality_valid_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPQuality_valid_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPQuality_id, "missing_value", &store->mbPQuality_missing_value);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPQuality_missing_value error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPQuality_id, "format_C", store->mbPQuality_format_C);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPQuality_format_C error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPQuality_id, "orientation", store->mbPQuality_orientation);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPQuality_orientation error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbPFlag_id >= 0)
		    {
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPFlag_id, "type", store->mbPFlag_type);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPFlag_type error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPFlag_id, "long_name", store->mbPFlag_long_name);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPFlag_long_name error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPFlag_id, "name_code", store->mbPFlag_name_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPFlag_name_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPFlag_id, "units", store->mbPFlag_units);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPFlag_units error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPFlag_id, "unit_code", store->mbPFlag_unit_code);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPFlag_unit_code error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPFlag_id, "add_offset", &store->mbPFlag_add_offset);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPFlag_add_offset error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPFlag_id, "scale_factor", &store->mbPFlag_scale_factor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPFlag_scale_factor error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPFlag_id, "minimum", &store->mbPFlag_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPFlag_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPFlag_id, "maximum", &store->mbPFlag_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPFlag_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPFlag_id, "valid_minimum", &store->mbPFlag_valid_minimum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPFlag_valid_minimum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPFlag_id, "valid_maximum", &store->mbPFlag_valid_maximum);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPFlag_valid_maximum error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_int((int)mb_io_ptr->mbfp, store->mbPFlag_id, "missing_value", &store->mbPFlag_missing_value);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPFlag_missing_value error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPFlag_id, "format_C", store->mbPFlag_format_C);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPFlag_format_C error: %s\n", nc_strerror(nc_status));
		    nc_status = nc_get_att_text((int)mb_io_ptr->mbfp, store->mbPFlag_id, "orientation", store->mbPFlag_orientation);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_att mbPFlag_orientation error: %s\n", nc_strerror(nc_status));
		    }
		if (nc_status != NC_NOERR)
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }		
		
		/* print input debug statements */
		if (verbose >= 2)
		    {
		    fprintf(stderr,"\ndbg2  NetCDF variable attributes read in function <%s>\n",
			    function_name);
		    fprintf(stderr,"dbg2  Variable attributes:\n");
		    fprintf(stderr,"dbg2       status:				%d\n", status);
		    fprintf(stderr,"dbg2       error:				%d\n", *error);
		    fprintf(stderr,"dbg2       nc_status:			%d\n", nc_status);
		    fprintf(stderr,"dbg2       mbHistCode_long_name:		%s\n", store->mbHistCode_long_name);
		    fprintf(stderr,"dbg2       mbHistCode_name_code:		%s\n", store->mbHistCode_name_code);
		    fprintf(stderr,"dbg2       mbHistCode_units:		%s\n", store->mbHistCode_units);
		    fprintf(stderr,"dbg2       mbHistCode_unit_code:		%s\n", store->mbHistCode_unit_code);
		    fprintf(stderr,"dbg2       mbHistCode_add_offset:		%d\n", store->mbHistCode_add_offset);
		    fprintf(stderr,"dbg2       mbHistCode_scale_factor:		%d\n", store->mbHistCode_scale_factor);
		    fprintf(stderr,"dbg2       mbHistCode_minimum:		%d\n", store->mbHistCode_minimum);
		    fprintf(stderr,"dbg2       mbHistCode_maximum:		%d\n", store->mbHistCode_maximum);
		    fprintf(stderr,"dbg2       mbHistCode_valid_minimum:	%d\n", store->mbHistCode_valid_minimum);
		    fprintf(stderr,"dbg2       mbHistCode_valid_maximum:	%d\n", store->mbHistCode_valid_maximum);
		    fprintf(stderr,"dbg2       mbHistCode_missing_value:	%d\n", store->mbHistCode_missing_value);
		    fprintf(stderr,"dbg2       mbHistCode_format_C:		%s\n", store->mbHistCode_format_C);
		    fprintf(stderr,"dbg2       mbHistCode_orientation:		%s\n", store->mbHistCode_orientation);
		    fprintf(stderr,"dbg2       mbHistAutor_type:		%s\n", store->mbHistAutor_type);
		    fprintf(stderr,"dbg2       mbHistAutor_long_name:		%s\n", store->mbHistAutor_long_name);
		    fprintf(stderr,"dbg2       mbHistAutor_name_code:		%s\n", store->mbHistAutor_name_code);
		    fprintf(stderr,"dbg2       mbHistModule_type:		%s\n", store->mbHistModule_type);
		    fprintf(stderr,"dbg2       mbHistModule_long_name:		%s\n", store->mbHistModule_long_name);
		    fprintf(stderr,"dbg2       mbHistModule_name_code:		%s\n", store->mbHistModule_name_code);
		    fprintf(stderr,"dbg2       mbHistComment_type:		%s\n", store->mbHistComment_type);
		    fprintf(stderr,"dbg2       mbHistComment_long_name:		%s\n", store->mbHistComment_long_name);
		    fprintf(stderr,"dbg2       mbHistComment_name_code:		%s\n", store->mbHistComment_name_code);
		    fprintf(stderr,"dbg2       mbDate_type:			%s\n", store->mbDate_type);
		    fprintf(stderr,"dbg2       mbDate_long_name:		%s\n", store->mbDate_long_name);
		    fprintf(stderr,"dbg2       mbDate_name_code:		%s\n", store->mbDate_name_code);
		    fprintf(stderr,"dbg2       mbDate_units:			%s\n", store->mbDate_units);
		    fprintf(stderr,"dbg2       mbDate_unit_code:		%s\n", store->mbDate_unit_code);
		    fprintf(stderr,"dbg2       mbDate_add_offset:		%d\n", store->mbDate_add_offset);
		    fprintf(stderr,"dbg2       mbDate_scale_factor:		%d\n", store->mbDate_scale_factor);
		    fprintf(stderr,"dbg2       mbDate_minimum:			%d\n", store->mbDate_minimum);
		    fprintf(stderr,"dbg2       mbDate_maximum:			%d\n", store->mbDate_maximum);
		    fprintf(stderr,"dbg2       mbDate_valid_minimum:		%d\n", store->mbDate_valid_minimum);
		    fprintf(stderr,"dbg2       mbDate_valid_maximum:		%d\n", store->mbDate_valid_maximum);
		    fprintf(stderr,"dbg2       mbDate_missing_value:		%d\n", store->mbDate_missing_value);
		    fprintf(stderr,"dbg2       mbDate_format_C:			%s\n", store->mbDate_format_C);
		    fprintf(stderr,"dbg2       mbDate_orientation:		%s\n", store->mbDate_orientation);
		    fprintf(stderr,"dbg2       mbTime_type:			%s\n", store->mbTime_type);
		    fprintf(stderr,"dbg2       mbTime_long_name:		%s\n", store->mbTime_long_name);
		    fprintf(stderr,"dbg2       mbTime_name_code:		%s\n", store->mbTime_name_code);
		    fprintf(stderr,"dbg2       mbTime_units:			%s\n", store->mbTime_units);
		    fprintf(stderr,"dbg2       mbTime_unit_code:		%s\n", store->mbTime_unit_code);
		    fprintf(stderr,"dbg2       mbTime_add_offset:		%d\n", store->mbTime_add_offset);
		    fprintf(stderr,"dbg2       mbTime_scale_factor:		%d\n", store->mbTime_scale_factor);
		    fprintf(stderr,"dbg2       mbTime_minimum:			%d\n", store->mbTime_minimum);
		    fprintf(stderr,"dbg2       mbTime_maximum:			%d\n", store->mbTime_maximum);
		    fprintf(stderr,"dbg2       mbTime_valid_minimum:		%d\n", store->mbTime_valid_minimum);
		    fprintf(stderr,"dbg2       mbTime_valid_maximum:		%d\n", store->mbTime_valid_maximum);
		    fprintf(stderr,"dbg2       mbTime_missing_value:		%d\n", store->mbTime_missing_value);
		    fprintf(stderr,"dbg2       mbTime_format_C:			%s\n", store->mbTime_format_C);
		    fprintf(stderr,"dbg2       mbTime_orientation:		%s\n", store->mbTime_orientation);
		    fprintf(stderr,"dbg2       mbOrdinate_type:			%s\n", store->mbOrdinate_type);
		    fprintf(stderr,"dbg2       mbOrdinate_long_name:		%s\n", store->mbOrdinate_long_name);
		    fprintf(stderr,"dbg2       mbOrdinate_name_code:		%s\n", store->mbOrdinate_name_code);
		    fprintf(stderr,"dbg2       mbOrdinate_units:		%s\n", store->mbOrdinate_units);
		    fprintf(stderr,"dbg2       mbOrdinate_unit_code:		%s\n", store->mbOrdinate_unit_code);
		    fprintf(stderr,"dbg2       mbOrdinate_add_offset:		%f\n", store->mbOrdinate_add_offset);
		    fprintf(stderr,"dbg2       mbOrdinate_scale_factor:		%f\n", store->mbOrdinate_scale_factor);
		    fprintf(stderr,"dbg2       mbOrdinate_minimum:		%d\n", store->mbOrdinate_minimum);
		    fprintf(stderr,"dbg2       mbOrdinate_maximum:		%d\n", store->mbOrdinate_maximum);
		    fprintf(stderr,"dbg2       mbOrdinate_valid_minimum:	%d\n", store->mbOrdinate_valid_minimum);
		    fprintf(stderr,"dbg2       mbOrdinate_valid_maximum:	%d\n", store->mbOrdinate_valid_maximum);
		    fprintf(stderr,"dbg2       mbOrdinate_missing_value:	%d\n", store->mbOrdinate_missing_value);
		    fprintf(stderr,"dbg2       mbOrdinate_format_C:		%s\n", store->mbOrdinate_format_C);
		    fprintf(stderr,"dbg2       mbOrdinate_orientation:		%s\n", store->mbOrdinate_orientation);
		    fprintf(stderr,"dbg2       mbAbscissa_type:			%s\n", store->mbAbscissa_type);
		    fprintf(stderr,"dbg2       mbAbscissa_long_name:		%s\n", store->mbAbscissa_long_name);
		    fprintf(stderr,"dbg2       mbAbscissa_name_code:		%s\n", store->mbAbscissa_name_code);
		    fprintf(stderr,"dbg2       mbAbscissa_units:		%s\n", store->mbAbscissa_units);
		    fprintf(stderr,"dbg2       mbAbscissa_unit_code:		%s\n", store->mbAbscissa_unit_code);
		    fprintf(stderr,"dbg2       mbAbscissa_add_offset:		%f\n", store->mbAbscissa_add_offset);
		    fprintf(stderr,"dbg2       mbAbscissa_scale_factor:		%f\n", store->mbAbscissa_scale_factor);
		    fprintf(stderr,"dbg2       mbAbscissa_minimum:		%d\n", store->mbAbscissa_minimum);
		    fprintf(stderr,"dbg2       mbAbscissa_maximum:		%d\n", store->mbAbscissa_maximum);
		    fprintf(stderr,"dbg2       mbAbscissa_valid_minimum:	%d\n", store->mbAbscissa_valid_minimum);
		    fprintf(stderr,"dbg2       mbAbscissa_valid_maximum:	%d\n", store->mbAbscissa_valid_maximum);
		    fprintf(stderr,"dbg2       mbAbscissa_missing_value:	%d\n", store->mbAbscissa_missing_value);
		    fprintf(stderr,"dbg2       mbAbscissa_format_C:		%s\n", store->mbAbscissa_format_C);
		    fprintf(stderr,"dbg2       mbAbscissa_orientation:		%s\n", store->mbAbscissa_orientation);
		    fprintf(stderr,"dbg2       mbAltitude_type:			%s\n", store->mbAltitude_type);
		    fprintf(stderr,"dbg2       mbAltitude_long_name:		%s\n", store->mbAltitude_long_name);
		    fprintf(stderr,"dbg2       mbAltitude_name_code:		%s\n", store->mbAltitude_name_code);
		    fprintf(stderr,"dbg2       mbAltitude_units:		%s\n", store->mbAltitude_units);
		    fprintf(stderr,"dbg2       mbAltitude_unit_code:		%s\n", store->mbAltitude_unit_code);
		    fprintf(stderr,"dbg2       mbAltitude_add_offset:		%f\n", store->mbAltitude_add_offset);
		    fprintf(stderr,"dbg2       mbAltitude_scale_factor:		%f\n", store->mbAltitude_scale_factor);
		    fprintf(stderr,"dbg2       mbAltitude_minimum:		%d\n", store->mbAltitude_minimum);
		    fprintf(stderr,"dbg2       mbAltitude_maximum:		%d\n", store->mbAltitude_maximum);
		    fprintf(stderr,"dbg2       mbAltitude_valid_minimum:	%d\n", store->mbAltitude_valid_minimum);
		    fprintf(stderr,"dbg2       mbAltitude_valid_maximum:	%d\n", store->mbAltitude_valid_maximum);
		    fprintf(stderr,"dbg2       mbAltitude_missing_value:	%d\n", store->mbAltitude_missing_value);
		    fprintf(stderr,"dbg2       mbAltitude_format_C:		%s\n", store->mbAltitude_format_C);
		    fprintf(stderr,"dbg2       mbAltitude_orientation:		%s\n", store->mbAltitude_orientation);
		    fprintf(stderr,"dbg2       mbImmersion_type:		%s\n", store->mbImmersion_type);
		    fprintf(stderr,"dbg2       mbImmersion_long_name:		%s\n", store->mbImmersion_long_name);
		    fprintf(stderr,"dbg2       mbImmersion_name_code:		%s\n", store->mbImmersion_name_code);
		    fprintf(stderr,"dbg2       mbImmersion_units:		%s\n", store->mbImmersion_units);
		    fprintf(stderr,"dbg2       mbImmersion_unit_code:		%s\n", store->mbImmersion_unit_code);
		    fprintf(stderr,"dbg2       mbImmersion_add_offset:		%f\n", store->mbImmersion_add_offset);
		    fprintf(stderr,"dbg2       mbImmersion_scale_factor:	%f\n", store->mbImmersion_scale_factor);
		    fprintf(stderr,"dbg2       mbImmersion_minimum:		%d\n", store->mbImmersion_minimum);
		    fprintf(stderr,"dbg2       mbImmersion_maximum:		%d\n", store->mbImmersion_maximum);
		    fprintf(stderr,"dbg2       mbImmersion_valid_minimum:	%d\n", store->mbImmersion_valid_minimum);
		    fprintf(stderr,"dbg2       mbImmersion_valid_maximum:	%d\n", store->mbImmersion_valid_maximum);
		    fprintf(stderr,"dbg2       mbImmersion_missing_value:	%d\n", store->mbImmersion_missing_value);
		    fprintf(stderr,"dbg2       mbImmersion_format_C:		%s\n", store->mbImmersion_format_C);
		    fprintf(stderr,"dbg2       mbImmersion_orientation:		%s\n", store->mbImmersion_orientation);
		    fprintf(stderr,"dbg2       mbHeading_type:			%s\n", store->mbHeading_type);
		    fprintf(stderr,"dbg2       mbHeading_long_name:		%s\n", store->mbHeading_long_name);
		    fprintf(stderr,"dbg2       mbHeading_name_code:		%s\n", store->mbHeading_name_code);
		    fprintf(stderr,"dbg2       mbHeading_units:			%s\n", store->mbHeading_units);
		    fprintf(stderr,"dbg2       mbHeading_unit_code:		%s\n", store->mbHeading_unit_code);
		    fprintf(stderr,"dbg2       mbHeading_add_offset:		%f\n", store->mbHeading_add_offset);
		    fprintf(stderr,"dbg2       mbHeading_scale_factor:		%f\n", store->mbHeading_scale_factor);
		    fprintf(stderr,"dbg2       mbHeading_minimum:		%d\n", store->mbHeading_minimum);
		    fprintf(stderr,"dbg2       mbHeading_maximum:		%d\n", store->mbHeading_maximum);
		    fprintf(stderr,"dbg2       mbHeading_valid_minimum:		%d\n", store->mbHeading_valid_minimum);
		    fprintf(stderr,"dbg2       mbHeading_valid_maximum:		%d\n", store->mbHeading_valid_maximum);
		    fprintf(stderr,"dbg2       mbHeading_missing_value:		%d\n", store->mbHeading_missing_value);
		    fprintf(stderr,"dbg2       mbHeading_format_C:		%s\n", store->mbHeading_format_C);
		    fprintf(stderr,"dbg2       mbHeading_orientation:		%s\n", store->mbHeading_orientation);
		    fprintf(stderr,"dbg2       mbSpeed_type:			%s\n", store->mbSpeed_type);
		    fprintf(stderr,"dbg2       mbSpeed_long_name:		%s\n", store->mbSpeed_long_name);
		    fprintf(stderr,"dbg2       mbSpeed_name_code:		%s\n", store->mbSpeed_name_code);
		    fprintf(stderr,"dbg2       mbSpeed_units:			%s\n", store->mbSpeed_units);
		    fprintf(stderr,"dbg2       mbSpeed_unit_code:		%s\n", store->mbSpeed_unit_code);
		    fprintf(stderr,"dbg2       mbSpeed_add_offset:		%f\n", store->mbSpeed_add_offset);
		    fprintf(stderr,"dbg2       mbSpeed_scale_factor:		%f\n", store->mbSpeed_scale_factor);
		    fprintf(stderr,"dbg2       mbSpeed_minimum:			%d\n", store->mbSpeed_minimum);
		    fprintf(stderr,"dbg2       mbSpeed_maximum:			%d\n", store->mbSpeed_maximum);
		    fprintf(stderr,"dbg2       mbSpeed_valid_minimum:		%d\n", store->mbSpeed_valid_minimum);
		    fprintf(stderr,"dbg2       mbSpeed_valid_maximum:		%d\n", store->mbSpeed_valid_maximum);
		    fprintf(stderr,"dbg2       mbSpeed_missing_value:		%d\n", store->mbSpeed_missing_value);
		    fprintf(stderr,"dbg2       mbSpeed_format_C:		%s\n", store->mbSpeed_format_C);
		    fprintf(stderr,"dbg2       mbSpeed_orientation:		%s\n", store->mbSpeed_orientation);
		    fprintf(stderr,"dbg2       mbPType_type:			%s\n", store->mbPType_type);
		    fprintf(stderr,"dbg2       mbPType_long_name:		%s\n", store->mbPType_long_name);
		    fprintf(stderr,"dbg2       mbPType_name_code:		%s\n", store->mbPType_name_code);
		    fprintf(stderr,"dbg2       mbPType_units:			%s\n", store->mbPType_units);
		    fprintf(stderr,"dbg2       mbPType_unit_code:		%s\n", store->mbPType_unit_code);
		    fprintf(stderr,"dbg2       mbPType_add_offset:		%d\n", store->mbPType_add_offset);
		    fprintf(stderr,"dbg2       mbPType_scale_factor:		%d\n", store->mbPType_scale_factor);
		    fprintf(stderr,"dbg2       mbPType_minimum:			%d\n", store->mbPType_minimum);
		    fprintf(stderr,"dbg2       mbPType_maximum:			%d\n", store->mbPType_maximum);
		    fprintf(stderr,"dbg2       mbPType_valid_minimum:		%d\n", store->mbPType_valid_minimum);
		    fprintf(stderr,"dbg2       mbPType_valid_maximum:		%d\n", store->mbPType_valid_maximum);
		    fprintf(stderr,"dbg2       mbPType_missing_value:		%d\n", store->mbPType_missing_value);
		    fprintf(stderr,"dbg2       mbPType_format_C:		%s\n", store->mbPType_format_C);
		    fprintf(stderr,"dbg2       mbPType_orientation:		%s\n", store->mbPType_orientation);
		    fprintf(stderr,"dbg2       mbPQuality_type:			%s\n", store->mbPQuality_type);
		    fprintf(stderr,"dbg2       mbPQuality_long_name:		%s\n", store->mbPQuality_long_name);
		    fprintf(stderr,"dbg2       mbPQuality_name_code:		%s\n", store->mbPQuality_name_code);
		    fprintf(stderr,"dbg2       mbPQuality_units:		%s\n", store->mbPQuality_units);
		    fprintf(stderr,"dbg2       mbPQuality_unit_code:		%s\n", store->mbPQuality_unit_code);
		    fprintf(stderr,"dbg2       mbPQuality_add_offset:		%d\n", store->mbPQuality_add_offset);
		    fprintf(stderr,"dbg2       mbPQuality_scale_factor:		%d\n", store->mbPQuality_scale_factor);
		    fprintf(stderr,"dbg2       mbPQuality_minimum:		%d\n", store->mbPQuality_minimum);
		    fprintf(stderr,"dbg2       mbPQuality_maximum:		%d\n", store->mbPQuality_maximum);
		    fprintf(stderr,"dbg2       mbPQuality_valid_minimum:	%d\n", store->mbPQuality_valid_minimum);
		    fprintf(stderr,"dbg2       mbPQuality_valid_maximum:	%d\n", store->mbPQuality_valid_maximum);
		    fprintf(stderr,"dbg2       mbPQuality_missing_value:	%d\n", store->mbPQuality_missing_value);
		    fprintf(stderr,"dbg2       mbPQuality_format_C:		%s\n", store->mbPQuality_format_C);
		    fprintf(stderr,"dbg2       mbPQuality_orientation:		%s\n", store->mbPQuality_orientation);
		    fprintf(stderr,"dbg2       mbPFlag_type:			%s\n", store->mbPFlag_type);
		    fprintf(stderr,"dbg2       mbPFlag_long_name:		%s\n", store->mbPFlag_long_name);
		    fprintf(stderr,"dbg2       mbPFlag_name_code:		%s\n", store->mbPFlag_name_code);
		    fprintf(stderr,"dbg2       mbPFlag_units:			%s\n", store->mbPFlag_units);
		    fprintf(stderr,"dbg2       mbPFlag_unit_code:		%s\n", store->mbPFlag_unit_code);
		    fprintf(stderr,"dbg2       mbPFlag_add_offset:		%d\n", store->mbPFlag_add_offset);
		    fprintf(stderr,"dbg2       mbPFlag_scale_factor:		%d\n", store->mbPFlag_scale_factor);
		    fprintf(stderr,"dbg2       mbPFlag_minimum:			%d\n", store->mbPFlag_minimum);
		    fprintf(stderr,"dbg2       mbPFlag_maximum:			%d\n", store->mbPFlag_maximum);
		    fprintf(stderr,"dbg2       mbPFlag_valid_minimum:		%d\n", store->mbPFlag_valid_minimum);
		    fprintf(stderr,"dbg2       mbPFlag_valid_maximum:		%d\n", store->mbPFlag_valid_maximum);
		    fprintf(stderr,"dbg2       mbPFlag_missing_value:		%d\n", store->mbPFlag_missing_value);
		    fprintf(stderr,"dbg2       mbPFlag_format_C:		%s\n", store->mbPFlag_format_C);
		    fprintf(stderr,"dbg2       mbPFlag_orientation:		%s\n", store->mbPFlag_orientation);
		    }
		}

	    /* read global variables */
	    if (status == MB_SUCCESS)
		{
		if (store->mbHistDate_id >= 0)
		    {
		    index[0] = 0;
		    count[0] = store->mbNbrHistoryRec;
		    nc_status = nc_get_vara_int((int)mb_io_ptr->mbfp, store->mbHistDate_id, index, count, store->mbHistDate);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbHistDate error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbHistTime_id >= 0)
		    {
		    index[0] = 0;
		    count[0] = store->mbNbrHistoryRec;
		    nc_status = nc_get_vara_int((int)mb_io_ptr->mbfp, store->mbHistTime_id, index, count, store->mbHistTime);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbHistTime error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbHistCode_id >= 0)
		    {
		    index[0] = 0;
		    count[0] = store->mbNbrHistoryRec;
		    nc_status = nc_get_vara_text((int)mb_io_ptr->mbfp, store->mbHistCode_id, index, count, store->mbHistCode);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbHistCode error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbHistAutor_id >= 0)
		    {
		    index[0] = 0;
		    index[1] = 0;
		    count[0] = store->mbNbrHistoryRec;
		    count[1] = store->mbNameLength;
		    nc_status = nc_get_vara_text((int)mb_io_ptr->mbfp, store->mbHistAutor_id, index, count, store->mbHistAutor);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbHistAutor error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbHistModule_id >= 0)
		    {
		    index[0] = 0;
		    index[1] = 0;
		    count[0] = store->mbNbrHistoryRec;
		    count[1] = store->mbNameLength;
		    nc_status = nc_get_vara_text((int)mb_io_ptr->mbfp, store->mbHistModule_id, index, count, store->mbHistModule);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbHistModule error: %s\n", nc_strerror(nc_status));
		    }
		if (store->mbHistComment_id >= 0)
		    {
		    index[0] = 0;
		    index[1] = 0;
		    count[0] = store->mbNbrHistoryRec;
		    count[1] = store->mbCommentLength;
		    nc_status = nc_get_vara_text((int)mb_io_ptr->mbfp, store->mbHistComment_id, index, count, store->mbHistComment);
		    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbHistComment error: %s\n", nc_strerror(nc_status));
		    }
		if (nc_status != NC_NOERR)
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }		
		
		/* print input debug statements */
		if (verbose >= 2)
		    {
		    fprintf(stderr,"\ndbg2  NetCDF Global Variables read in function <%s>\n",
			    function_name);
		    fprintf(stderr,"dbg2  Global Variables:\n");
		    fprintf(stderr,"dbg2       status:                  %d\n", status);
		    fprintf(stderr,"dbg2       error:                   %d\n", *error);
		    fprintf(stderr,"dbg2       nc_status:               %d\n", nc_status);
		    fprintf(stderr,"dbg2       mbNbrHistoryRec:         %d\n", store->mbNbrHistoryRec);
		    for (i=0;i<store->mbNbrHistoryRec;i++)
			{
			fprintf(stderr,"dbg2       mbHistDate[%2d]:          %d\n", i, store->mbHistDate[i]);
			fprintf(stderr,"dbg2       mbHistTime[%2d]:          %d\n", i, store->mbHistTime[i]);
			fprintf(stderr,"dbg2       mbHistCode[%2d]:          %d\n", i, store->mbHistCode[i]);
			fprintf(stderr,"dbg2       mbHistAutor[%2d]:         %s\n", i, &(store->mbHistAutor[i*store->mbNameLength]));
			fprintf(stderr,"dbg2       mbHistModule[%2d]:        %s\n", i, &(store->mbHistModule[i*store->mbNameLength]));
			fprintf(stderr,"dbg2       mbHistComment[%2d]:       %s\n", i, &(store->mbHistComment[i*store->mbCommentLength]));
			}
		    }
		}
	    }

	/* read next data from file */
	/* first run through all comment records */
	if (status == MB_SUCCESS && store->mbNbrHistoryRec > *commentread)
	    {
	    store->kind = MB_DATA_COMMENT;
	    
	    /* get next comment */
	    strncpy(store->comment, 
		    &(store->mbHistComment[(*commentread) *store->mbCommentLength]), 
		    MBSYS_NAVNETCDF_COMMENTLEN);
	    
	    /* set counters */
	    (*commentread)++;
	    (*dataread)++;
	    
	    /* print input debug statements */
	    if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Comment read in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Comment:\n");
		fprintf(stderr,"dbg2       status:                  %d\n", status);
		fprintf(stderr,"dbg2       error:                   %d\n", *error);
		fprintf(stderr,"dbg2       comment:                 %s\n", store->comment);
		}
	    }
	
	/* next run through all survey records */
	else if (status == MB_SUCCESS && store->mbPositionNbr > *recread)
	    {
	    /* set kind */
	    store->kind = MB_DATA_DATA;

	    /* read the variables from next record */
	    if (store->mbDate_id >= 0)
		{
		index[0] = *recread;
		count[0] = 1;
		nc_status = nc_get_vara_int((int)mb_io_ptr->mbfp, store->mbDate_id, index, count, &store->mbDate);
		if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbDate error: %s\n", nc_strerror(nc_status));
		}
	    if (store->mbTime_id >= 0)
		{
		index[0] = *recread;
		count[0] = 1;
		nc_status = nc_get_vara_int((int)mb_io_ptr->mbfp, store->mbTime_id, index, count, &store->mbTime);
		if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbTime error: %s\n", nc_strerror(nc_status));
		}
	    if (store->mbOrdinate_id >= 0)
		{
		index[0] = *recread;
		count[0] = 1;
		nc_status = nc_get_vara_int((int)mb_io_ptr->mbfp, store->mbOrdinate_id, index, count, &store->mbOrdinate);
		if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbOrdinate error: %s\n", nc_strerror(nc_status));
		}
	    if (store->mbAbscissa_id >= 0)
		{
		index[0] = *recread;
		count[0] = 1;
		nc_status = nc_get_vara_int((int)mb_io_ptr->mbfp, store->mbAbscissa_id, index, count, &store->mbAbscissa);
		if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbAbscissa error: %s\n", nc_strerror(nc_status));
		}
	    if (store->mbAltitude_id >= 0)
		{
		index[0] = *recread;
		count[0] = 1;
		nc_status = nc_get_vara_short((int)mb_io_ptr->mbfp, store->mbAltitude_id, index, count, &store->mbAltitude);
		if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbAltitude error: %s\n", nc_strerror(nc_status));
		}
	    if (store->mbImmersion_id >= 0)
		{
		index[0] = *recread;
		count[0] = 1;
		nc_status = nc_get_vara_short((int)mb_io_ptr->mbfp, store->mbImmersion_id, index, count, &store->mbImmersion);
		if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbImmersion error: %s\n", nc_strerror(nc_status));
		}
	    if (store->mbHeading_id >= 0)
		{
		index[0] = *recread;
		count[0] = 1;
		nc_status = nc_get_vara_int((int)mb_io_ptr->mbfp, store->mbHeading_id, index, count, &store->mbHeading);
		if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbHeading error: %s\n", nc_strerror(nc_status));
		}
	    if (store->mbSpeed_id >= 0)
		{
		index[0] = *recread;
		count[0] = 1;
		nc_status = nc_get_vara_short((int)mb_io_ptr->mbfp, store->mbSpeed_id, index, count, &store->mbSpeed);
		if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbSpeed error: %s\n", nc_strerror(nc_status));
		}
	    if (store->mbPType_id >= 0)
		{
		index[0] = *recread;
		count[0] = 1;
		nc_status = nc_get_vara_text((int)mb_io_ptr->mbfp, store->mbPType_id, index, count, &store->mbPType);
		if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbPType error: %s\n", nc_strerror(nc_status));
		}
	    if (store->mbPQuality_id >= 0)
		{
		index[0] = *recread;
		count[0] = 1;
		nc_status = nc_get_vara_text((int)mb_io_ptr->mbfp, store->mbPQuality_id, index, count, &store->mbPQuality);
		if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbPQuality error: %s\n", nc_strerror(nc_status));
		}
	    if (store->mbPFlag_id >= 0)
		{
		index[0] = *recread;
		count[0] = 1;
		nc_status = nc_get_vara_text((int)mb_io_ptr->mbfp, store->mbPFlag_id, index, count, &store->mbPFlag);
		if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_get_vara mbPFlag error: %s\n", nc_strerror(nc_status));
		}
	    /* check status */
	    if (nc_status != NC_NOERR)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}		
	    
	    /* set counters */
	    (*recread)++;
	    (*dataread)++;
		
	    /* print input debug statements */
	    if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  NetCDF Survey Record read in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Global Variables:\n");
		fprintf(stderr,"dbg2       status:                  %d\n", status);
		fprintf(stderr,"dbg2       error:                   %d\n", *error);
		fprintf(stderr,"dbg2       nc_status:               %d\n", nc_status);
		fprintf(stderr,"dbg2       mbDate:                  %d\n", store->mbDate);
		fprintf(stderr,"dbg2       mbTime:                  %d\n", store->mbTime);
		fprintf(stderr,"dbg2       mbOrdinate:              %d\n", store->mbOrdinate);
		fprintf(stderr,"dbg2       mbAbscissa:              %d\n", store->mbAbscissa);
		fprintf(stderr,"dbg2       mbAltitude:              %d\n", store->mbAltitude);
		fprintf(stderr,"dbg2       mbImmersion:             %d\n", store->mbImmersion);
		fprintf(stderr,"dbg2       mbHeading:               %d\n", store->mbHeading);
		fprintf(stderr,"dbg2       mbSpeed:                 %d\n", store->mbSpeed);
		fprintf(stderr,"dbg2       mbPType:                 %d\n", store->mbPType);
		fprintf(stderr,"dbg2       mbPQuality:              %d\n", store->mbPQuality);
		fprintf(stderr,"dbg2       mbPFlag:                 %d\n", store->mbPFlag);
		}
	    }
	
	/* else end of file */
	else
	    {
	    /* set kind */
	    store->kind = MB_DATA_NONE;
	    
	    /* set flags */
	    *error = MB_ERROR_EOF;
	    status = MB_FAILURE;
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
int mbr_wt_nvnetcdf(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_nvnetcdf";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;
	struct mbsys_navnetcdf_struct *storelocal;
	int	*datawrite;
	int	*commentwrite;
	int	*recwrite;
	int 	nc_status;
	int	mbHistoryRecNbr_id;
	int	mbNameLength_id;
	int	mbCommentLength_id;
	int	mbPositionNbr_id;
	int	dims[1];
	size_t	index[2], count[2];
	char	*user_ptr;
	double	time_d;
	int	icomment;
	int	i;
#ifdef MBNETCDF_DEBUG
	int	nc_verbose = 1;
#else
	int	nc_verbose = 0;
#endif

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

	/* get pointer to mbio descriptor and data storage */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_navnetcdf_struct *) store_ptr;
	storelocal = (struct mbsys_navnetcdf_struct *) mb_io_ptr->store_data;
	datawrite = (int *) &mb_io_ptr->save1;
	commentwrite = (int *) &mb_io_ptr->save2;
	recwrite = (int *) &mb_io_ptr->save4;

	/* if comment and nothing written yet save it */
	if (store->kind == MB_DATA_COMMENT
	    && *recwrite == 0)
	    {
	    /* allocate arrays if needed */
	    if (storelocal->mbNbrHistoryRec >= storelocal->mbHistoryRecNbr)
	    	{
		/* allocate or reallocate history arrays */
		storelocal->mbHistoryRecNbr += 20;
		status = mb_reallocd(verbose, __FILE__, __LINE__,
			    storelocal->mbHistoryRecNbr * sizeof(int),
			    (void **)&storelocal->mbHistDate,error);
		status = mb_reallocd(verbose, __FILE__, __LINE__, 
			    storelocal->mbHistoryRecNbr * sizeof(int),
			    (void **)&storelocal->mbHistTime,error);
		status = mb_reallocd(verbose, __FILE__, __LINE__, 
			    storelocal->mbHistoryRecNbr * sizeof(char),
			    (void **)&storelocal->mbHistCode,error);
		status = mb_reallocd(verbose, __FILE__, __LINE__, 
			    storelocal->mbHistoryRecNbr * storelocal->mbNameLength * sizeof(char),
			    (void **)&storelocal->mbHistAutor,error);
		status = mb_reallocd(verbose, __FILE__, __LINE__, 
			    storelocal->mbHistoryRecNbr * storelocal->mbNameLength * sizeof(char),
			    (void **)&storelocal->mbHistModule,error);
		status = mb_reallocd(verbose, __FILE__, __LINE__, 
			    storelocal->mbHistoryRecNbr * storelocal->mbCommentLength * sizeof(char),
			    (void **)&storelocal->mbHistComment,error);
		for (i=storelocal->mbNbrHistoryRec;i<storelocal->mbHistoryRecNbr;i++)
			    {
			    storelocal->mbHistDate[i] = 0;
			    storelocal->mbHistTime[i] = 0;
			    storelocal->mbHistCode[i] = 0;
			    }
		}
		
	    /* save old comment (from pre-existing netcdf file) */
	    if (store != storelocal)
		{
		/* figure out which comment is being passed */
		icomment = -1;
		for (i=0;i<store->mbNbrHistoryRec;i++)
		    {
		    if (strncmp(store->comment, 
				&store->mbHistComment[i * store->mbCommentLength], 
				MBSYS_NAVNETCDF_COMMENTLEN) == 0)
			{
			icomment = i;
			}
		    }
		if (icomment > -1 && icomment < store->mbNbrHistoryRec)
		    {
		    strncpy(&(storelocal->mbHistAutor[(*commentwrite) * storelocal->mbNameLength]), 
			    &(store->mbHistAutor[icomment * store->mbNameLength]), 
			    MBSYS_NAVNETCDF_NAMELEN);
		    strncpy(&(storelocal->mbHistModule[(*commentwrite) * storelocal->mbNameLength]), 
			    &(store->mbHistModule[icomment * store->mbNameLength]), 
			    MBSYS_NAVNETCDF_NAMELEN);
		    strncpy(&(storelocal->mbHistComment[(*commentwrite) * storelocal->mbCommentLength]), 
			    &(store->mbHistComment[icomment * store->mbCommentLength]), 
			    MBSYS_NAVNETCDF_COMMENTLEN);
		    storelocal->mbHistDate[*commentwrite] = store->mbHistDate[icomment];
		    storelocal->mbHistTime[*commentwrite] = store->mbHistTime[icomment];
		    storelocal->mbHistCode[*commentwrite] = 1;
		    storelocal->mbNbrHistoryRec++;
		    }
		}
		
	    /* save new comment */
	    else
		{
		if ((user_ptr = getenv("USER")) == NULL)
			user_ptr = getenv("LOGNAME");
		if (user_ptr != NULL)
		    strncpy(&(storelocal->mbHistAutor[(*commentwrite) *storelocal->mbNameLength]), 
			user_ptr, MBSYS_NAVNETCDF_NAMELEN);
		else
		    strncpy(&(storelocal->mbHistAutor[(*commentwrite) *storelocal->mbNameLength]), 
			"Unknown", MBSYS_NAVNETCDF_NAMELEN);
		strncpy(&(storelocal->mbHistModule[(*commentwrite) *storelocal->mbNameLength]), 
			"MB-System", MBSYS_NAVNETCDF_NAMELEN);
		strncpy(&(storelocal->mbHistComment[(*commentwrite) *storelocal->mbCommentLength]), 
			store->comment, MBSYS_NAVNETCDF_COMMENTLEN);
		time_d = (double)time((time_t *)0);
		storelocal->mbHistDate[*commentwrite] = (int)(time_d / SECINDAY);
		storelocal->mbHistTime[*commentwrite] = (int)(1000 * (time_d - storelocal->mbHistDate[*commentwrite] * SECINDAY));
		storelocal->mbHistCode[*commentwrite] = 1;
		storelocal->mbNbrHistoryRec++;
		}
	    
	    /* set counters */
	    (*commentwrite)++;
	    (*datawrite)++;
	    
	    /* print input debug statements */
	    if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Comment saved in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Comment:\n");
		fprintf(stderr,"dbg2       status:                  %d\n", status);
		fprintf(stderr,"dbg2       error:                   %d\n", *error);
		fprintf(stderr,"dbg2       comment:                 %s\n", store->comment);
		}
	    }
	
	/* if data and nothing written yet start it off */
	if (store->kind == MB_DATA_DATA
	    && *recwrite == 0
	    && status == MB_SUCCESS)
	    {
	    /* copy over noncomment dimensions */
	    storelocal->mbPositionNbr = 0;
		
	    /* define the dimensions */
   	    nc_status = nc_def_dim((int)mb_io_ptr->mbfp, "mbHistoryRecNbr", storelocal->mbHistoryRecNbr, &mbHistoryRecNbr_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_dim mbHistoryRecNbr error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_def_dim((int)mb_io_ptr->mbfp, "mbNameLength", storelocal->mbNameLength, &mbNameLength_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_dim mbNameLength error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_def_dim((int)mb_io_ptr->mbfp, "mbCommentLength", storelocal->mbCommentLength, &mbCommentLength_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_dim mbCommentLength error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_def_dim((int)mb_io_ptr->mbfp, "mbPositionNbr", NC_UNLIMITED, &mbPositionNbr_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_dim mbPositionNbr error: %s\n", nc_strerror(nc_status));
		
	    /* print input debug statements */
	    if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  NetCDF array dimensions written in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Array and variable dimensions:\n");
		fprintf(stderr,"dbg2       status:                  %d\n", status);
		fprintf(stderr,"dbg2       error:                   %d\n", *error);
		fprintf(stderr,"dbg2       nc_status:               %d\n", nc_status);
		fprintf(stderr,"dbg2       mbHistoryRecNbr:         %d\n", storelocal->mbHistoryRecNbr);
		fprintf(stderr,"dbg2       mbNameLength:            %d\n", storelocal->mbNameLength);
		fprintf(stderr,"dbg2       mbCommentLength:         %d\n", storelocal->mbCommentLength);
		fprintf(stderr,"dbg2       mbPositionNbr:              %d\n", storelocal->mbPositionNbr);
		}
	    
	    /* define the variables */
	    dims[0] = mbHistoryRecNbr_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbHistDate", NC_INT, 1, dims, &storelocal->mbHistDate_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbHistDate_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbHistoryRecNbr_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbHistTime", NC_INT, 1, dims, &storelocal->mbHistTime_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbHistTime_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbHistoryRecNbr_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbHistCode", NC_CHAR, 1, dims, &storelocal->mbHistCode_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbHistCode_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbHistoryRecNbr_id;
	    dims[1] = mbNameLength_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbHistAutor", NC_CHAR, 2, dims, &storelocal->mbHistAutor_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbHistAutor_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbHistoryRecNbr_id;
	    dims[1] = mbNameLength_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbHistModule", NC_CHAR, 2, dims, &storelocal->mbHistModule_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbHistModule_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbHistoryRecNbr_id;
	    dims[1] = mbCommentLength_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbHistComment", NC_CHAR, 2, dims, &storelocal->mbHistComment_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbHistComment_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbPositionNbr_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbDate", NC_INT, 1, dims, &storelocal->mbDate_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbDate_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbPositionNbr_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbTime", NC_INT, 1, dims, &storelocal->mbTime_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbTime_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbPositionNbr_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbOrdinate", NC_INT, 1, dims, &storelocal->mbOrdinate_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbOrdinate_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbPositionNbr_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbAbscissa", NC_INT, 1, dims, &storelocal->mbAbscissa_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbAbscissa_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbPositionNbr_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbAltitude", NC_SHORT, 1, dims, &storelocal->mbAltitude_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbAltitude_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbPositionNbr_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbImmersion", NC_SHORT, 1, dims, &storelocal->mbImmersion_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbImmersion_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbPositionNbr_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbHeading", NC_INT, 1, dims, &storelocal->mbHeading_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbHeading_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbPositionNbr_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbSpeed", NC_SHORT, 1, dims, &storelocal->mbSpeed_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbSpeed_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbPositionNbr_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbPType", NC_CHAR, 1, dims, &storelocal->mbPType_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbPType_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbPositionNbr_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbPQuality", NC_CHAR, 1, dims, &storelocal->mbPQuality_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbPQuality_id error: %s\n", nc_strerror(nc_status));
	    dims[0] = mbPositionNbr_id;
	    nc_status = nc_def_var((int)mb_io_ptr->mbfp, "mbPFlag", NC_CHAR, 1, dims, &storelocal->mbPFlag_id);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_def_var mbPFlag_id error: %s\n", nc_strerror(nc_status));

	    /* print input debug statements */
	    if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  NetCDF variable ids written in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Variable ids:\n");
		fprintf(stderr,"dbg2       status:                  %d\n", status);
		fprintf(stderr,"dbg2       error:                   %d\n", *error);
		fprintf(stderr,"dbg2       nc_status:               %d\n", nc_status);
		fprintf(stderr,"dbg2       mbHistDate_id:           %d\n", storelocal->mbHistDate_id);
		fprintf(stderr,"dbg2       mbHistTime_id:           %d\n", storelocal->mbHistTime_id);
		fprintf(stderr,"dbg2       mbHistCode_id:           %d\n", storelocal->mbHistCode_id);
		fprintf(stderr,"dbg2       mbHistAutor_id:          %d\n", storelocal->mbHistAutor_id);
		fprintf(stderr,"dbg2       mbHistModule_id:         %d\n", storelocal->mbHistModule_id);
		fprintf(stderr,"dbg2       mbHistComment_id:        %d\n", storelocal->mbHistComment_id);
		fprintf(stderr,"dbg2       mbDate_id:               %d\n", storelocal->mbDate_id);
		fprintf(stderr,"dbg2       mbTime_id:               %d\n", storelocal->mbTime_id);
		fprintf(stderr,"dbg2       mbOrdinate_id:           %d\n", storelocal->mbOrdinate_id);
		fprintf(stderr,"dbg2       mbAbscissa_id:           %d\n", storelocal->mbAbscissa_id);
		fprintf(stderr,"dbg2       mbAltitude_id:           %d\n", storelocal->mbAltitude_id);
		fprintf(stderr,"dbg2       mbImmersion_id:          %d\n", storelocal->mbImmersion_id);
		fprintf(stderr,"dbg2       mbHeading_id:            %d\n", storelocal->mbHeading_id);
		fprintf(stderr,"dbg2       mbSpeed_id:              %d\n", storelocal->mbSpeed_id);
		fprintf(stderr,"dbg2       mbPType_id:              %d\n", storelocal->mbPType_id);
		fprintf(stderr,"dbg2       mbPQuality_id:           %d\n", storelocal->mbPQuality_id);
		fprintf(stderr,"dbg2       mbPFlag_id:              %d\n", storelocal->mbPFlag_id);
		}

	    /* save the global attributes */
	    nc_status = nc_put_att_short((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbVersion", NC_SHORT, 1, &store->mbVersion);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbVersion error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbName", MBSYS_NAVNETCDF_ATTRIBUTELEN, store->mbName);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbName error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbClasse", MBSYS_NAVNETCDF_ATTRIBUTELEN, store->mbClasse);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbClasse error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_short((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbLevel", NC_SHORT, 1, &store->mbLevel);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbLevel error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_short((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbNbrHistoryRec", NC_SHORT, 1, &storelocal->mbNbrHistoryRec);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbNbrHistoryRec error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbTimeReference", MBSYS_NAVNETCDF_ATTRIBUTELEN, store->mbTimeReference);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbTimeReference error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbStartDate", NC_INT, 1, &store->mbStartDate);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbStartDate error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbStartTime", NC_INT, 1, &store->mbStartTime);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbStartTime error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbEndDate", NC_INT, 1, &store->mbEndDate);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbEndDate error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbEndTime", NC_INT, 1, &store->mbEndTime);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbEndTime error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbNorthLatitude", NC_DOUBLE, 1, &store->mbNorthLatitude);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbNorthLatitude error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbSouthLatitude", NC_DOUBLE, 1, &store->mbSouthLatitude);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbSouthLatitude error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbEastLongitude", NC_DOUBLE, 1, &store->mbEastLongitude);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbEastLongitude error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbWestLongitude", NC_DOUBLE, 1, &store->mbWestLongitude);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbWestLongitude error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbMeridian180", MBSYS_NAVNETCDF_ATTRIBUTELEN, store->mbMeridian180);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbMeridian180 error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbGeoDictionnary", MBSYS_NAVNETCDF_ATTRIBUTELEN, store->mbGeoDictionnary);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbGeoDictionnary error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbGeoRepresentation", MBSYS_NAVNETCDF_ATTRIBUTELEN, store->mbGeoRepresentation);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbGeoRepresentation error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbGeodesicSystem", MBSYS_NAVNETCDF_ATTRIBUTELEN, store->mbGeodesicSystem);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbGeodesicSystem error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbEllipsoidName", MBSYS_NAVNETCDF_COMMENTLEN, store->mbEllipsoidName);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbEllipsoidName error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbEllipsoidA", NC_DOUBLE, 1, &store->mbEllipsoidA);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbEllipsoidA error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbEllipsoidInvF", NC_DOUBLE, 1, &store->mbEllipsoidInvF);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbEllipsoidInvF error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbEllipsoidE2", NC_DOUBLE, 1, &store->mbEllipsoidE2);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbEllipsoidE2 error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_short((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbProjType", NC_SHORT, 1, &store->mbProjType);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbProjType error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbProjParameterValue", NC_DOUBLE, 10, store->mbProjParameterValue);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbProjParameterValue error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbProjParameterCode", MBSYS_NAVNETCDF_COMMENTLEN, store->mbProjParameterCode);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbProjParameterCode error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbShip", MBSYS_NAVNETCDF_COMMENTLEN, store->mbShip);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbShip error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbSurvey", MBSYS_NAVNETCDF_COMMENTLEN, store->mbSurvey);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbSurvey error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbReference", MBSYS_NAVNETCDF_COMMENTLEN, store->mbReference);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbReference error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, NC_GLOBAL, "mbPointCounter", NC_INT, 1, &store->mbPointCounter);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbVersion error: %s\n", nc_strerror(nc_status));

	    /* print input debug statements */
	    if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  NetCDF global attributes written in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Global attributes:\n");
		fprintf(stderr,"dbg2       status:                  %d\n", status);
		fprintf(stderr,"dbg2       error:                   %d\n", *error);
		fprintf(stderr,"dbg2       nc_status:             %d\n", nc_status);
		fprintf(stderr,"dbg2       mbVersion:             %d\n", store->mbVersion);
		fprintf(stderr,"dbg2       mbName:                %s\n", store->mbName);
		fprintf(stderr,"dbg2       mbClasse:              %s\n", store->mbClasse);
		fprintf(stderr,"dbg2       mbLevel:               %d\n", store->mbLevel);
		fprintf(stderr,"dbg2       mbNbrHistoryRec:       %d\n", store->mbNbrHistoryRec);
		fprintf(stderr,"dbg2       mbTimeReference:       %s\n", store->mbTimeReference);
		fprintf(stderr,"dbg2       mbStartDate:           %d\n", store->mbStartDate);
		fprintf(stderr,"dbg2       mbStartTime:           %d\n", store->mbStartTime);
		fprintf(stderr,"dbg2       mbEndDate:             %d\n", store->mbEndDate);
		fprintf(stderr,"dbg2       mbEndTime:             %d\n", store->mbEndTime);
		fprintf(stderr,"dbg2       mbNorthLatitude:       %f\n", store->mbNorthLatitude);
		fprintf(stderr,"dbg2       mbSouthLatitude:       %f\n", store->mbSouthLatitude);
		fprintf(stderr,"dbg2       mbEastLongitude:       %f\n", store->mbEastLongitude);
		fprintf(stderr,"dbg2       mbWestLongitude:       %f\n", store->mbWestLongitude);
		fprintf(stderr,"dbg2       mbMeridian180:         %s\n", store->mbMeridian180);
		fprintf(stderr,"dbg2       mbGeoDictionnary:      %s\n", store->mbGeoDictionnary);
		fprintf(stderr,"dbg2       mbGeoRepresentation:   %s\n", store->mbGeoRepresentation);
		fprintf(stderr,"dbg2       mbGeodesicSystem:      %s\n", store->mbGeodesicSystem);
		fprintf(stderr,"dbg2       mbEllipsoidName:       %s\n", store->mbEllipsoidName);
		fprintf(stderr,"dbg2       mbEllipsoidA:          %f\n", store->mbEllipsoidA);
		fprintf(stderr,"dbg2       mbEllipsoidInvF:       %f\n", store->mbEllipsoidInvF);
		fprintf(stderr,"dbg2       mbEllipsoidE2:         %f\n", store->mbEllipsoidE2);
		fprintf(stderr,"dbg2       mbProjType:            %d\n", store->mbProjType);
		for (i=0;i<10;i++)
		    fprintf(stderr,"dbg2       mbProjParameterValue[%d]:%f\n", i, store->mbProjParameterValue[i]);
		fprintf(stderr,"dbg2       mbProjParameterCode:   %s\n", store->mbProjParameterCode);
		fprintf(stderr,"dbg2       mbShip:                %s\n", store->mbShip);
		fprintf(stderr,"dbg2       mbSurvey:              %s\n", store->mbSurvey);
		fprintf(stderr,"dbg2       mbReference:           %s\n", store->mbReference);
		fprintf(stderr,"dbg2       mbPointCounter:        %d\n", store->mbPointCounter);
		}
	    
	    /* save the variable attributes */
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistDate_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistDate_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistDate_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, "units", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistDate_units); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, "unit_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistDate_unit_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, "add_offset", NC_INT, 1, &storelocal->mbHistDate_add_offset);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistDate_add_offset error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, "scale_factor", NC_INT, 1, &storelocal->mbHistDate_scale_factor);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistDate_scale_factor error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, "minimum", NC_INT, 1, &storelocal->mbHistDate_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistDate_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, "maximum", NC_INT, 1, &storelocal->mbHistDate_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistDate_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, "valid_minimum", NC_INT, 1, &storelocal->mbHistDate_valid_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistDate_valid_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, "valid_maximum", NC_INT, 1, &storelocal->mbHistDate_valid_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistDate_valid_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, "missing_value", NC_INT, 1, &storelocal->mbHistDate_missing_value);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistDate_missing_value error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, "format_C", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistDate_format_C); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, "orientation", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistDate_orientation); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistTime_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistTime_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistTime_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, "units", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistTime_units); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, "unit_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistTime_unit_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, "add_offset", NC_INT, 1, &storelocal->mbHistTime_add_offset);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistTime_add_offset error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, "scale_factor", NC_INT, 1, &storelocal->mbHistTime_scale_factor);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistTime_scale_factor error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, "minimum", NC_INT, 1, &storelocal->mbHistTime_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistTime_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, "maximum", NC_INT, 1, &storelocal->mbHistTime_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistTime_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, "valid_minimum", NC_INT, 1, &storelocal->mbHistTime_valid_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistTime_valid_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, "valid_maximum", NC_INT, 1, &storelocal->mbHistTime_valid_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistTime_valid_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, "missing_value", NC_INT, 1, &storelocal->mbHistTime_missing_value);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistTime_missing_value error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, "format_C", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistTime_format_C); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, "orientation", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistTime_orientation); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistCode_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistCode_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistCode_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, "units", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistCode_units); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, "unit_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistCode_unit_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, "add_offset", NC_INT, 1, &storelocal->mbHistCode_add_offset);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistCode_add_offset error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, "scale_factor", NC_INT, 1, &storelocal->mbHistCode_scale_factor);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistCode_scale_factor error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, "minimum", NC_INT, 1, &storelocal->mbHistCode_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistCode_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, "maximum", NC_INT, 1, &storelocal->mbHistCode_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistCode_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, "valid_minimum", NC_INT, 1, &storelocal->mbHistCode_valid_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistCode_valid_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, "valid_maximum", NC_INT, 1, &storelocal->mbHistCode_valid_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistCode_valid_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, "missing_value", NC_INT, 1, &storelocal->mbHistCode_missing_value);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHistCode_missing_value error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, "format_C", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistCode_format_C); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, "orientation", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistCode_orientation); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistAutor_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistAutor_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistAutor_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistAutor_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistAutor_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistAutor_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistModule_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistModule_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistModule_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistModule_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistModule_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistModule_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistComment_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistComment_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistComment_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistComment_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHistComment_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHistComment_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbDate_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbDate_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbDate_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbDate_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbDate_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbDate_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbDate_id, "units", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbDate_units); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbDate_id, "unit_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbDate_unit_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbDate_id, "add_offset", NC_INT, 1, &storelocal->mbDate_add_offset);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbDate_add_offset error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbDate_id, "scale_factor", NC_INT, 1, &storelocal->mbDate_scale_factor);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbDate_scale_factor error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbDate_id, "minimum", NC_INT, 1, &storelocal->mbDate_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbDate_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbDate_id, "maximum", NC_INT, 1, &storelocal->mbDate_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbDate_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbDate_id, "valid_minimum", NC_INT, 1, &storelocal->mbDate_valid_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbDate_valid_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbDate_id, "valid_maximum", NC_INT, 1, &storelocal->mbDate_valid_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbDate_valid_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbDate_id, "missing_value", NC_INT, 1, &storelocal->mbDate_missing_value);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbDate_missing_value error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbDate_id, "format_C", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbDate_format_C); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbDate_id, "orientation", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbDate_orientation); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbTime_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbTime_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbTime_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbTime_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbTime_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbTime_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbTime_id, "units", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbTime_units); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbTime_id, "unit_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbTime_unit_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbTime_id, "add_offset", NC_INT, 1, &storelocal->mbTime_add_offset);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbTime_add_offset error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbTime_id, "scale_factor", NC_INT, 1, &storelocal->mbTime_scale_factor);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbTime_scale_factor error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbTime_id, "minimum", NC_INT, 1, &storelocal->mbTime_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbTime_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbTime_id, "maximum", NC_INT, 1, &storelocal->mbTime_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbTime_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbTime_id, "valid_minimum", NC_INT, 1, &storelocal->mbTime_valid_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbTime_valid_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbTime_id, "valid_maximum", NC_INT, 1, &storelocal->mbTime_valid_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbTime_valid_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbTime_id, "missing_value", NC_INT, 1, &storelocal->mbTime_missing_value);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbTime_missing_value error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbTime_id, "format_C", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbTime_format_C); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbTime_id, "orientation", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbTime_orientation); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbOrdinate_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbOrdinate_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbOrdinate_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, "units", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbOrdinate_units); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, "unit_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbOrdinate_unit_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, "add_offset", NC_DOUBLE, 1, &storelocal->mbOrdinate_add_offset);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbOrdinate_add_offset error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, "scale_factor", NC_DOUBLE, 1, &storelocal->mbOrdinate_scale_factor);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbOrdinate_scale_factor error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, "minimum", NC_INT, 1, &storelocal->mbOrdinate_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbOrdinate_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, "maximum", NC_INT, 1, &storelocal->mbOrdinate_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbOrdinate_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, "valid_minimum", NC_INT, 1, &storelocal->mbOrdinate_valid_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbOrdinate_valid_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, "valid_maximum", NC_INT, 1, &storelocal->mbOrdinate_valid_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbOrdinate_valid_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, "missing_value", NC_INT, 1, &storelocal->mbOrdinate_missing_value);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbOrdinate_missing_value error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, "format_C", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbOrdinate_format_C); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, "orientation", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbOrdinate_orientation); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbAbscissa_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbAbscissa_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbAbscissa_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, "units", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbAbscissa_units); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, "unit_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbAbscissa_unit_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, "add_offset", NC_DOUBLE, 1, &storelocal->mbAbscissa_add_offset);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbAbscissa_add_offset error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, "scale_factor", NC_DOUBLE, 1, &storelocal->mbAbscissa_scale_factor);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbAbscissa_scale_factor error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, "minimum", NC_INT, 1, &storelocal->mbAbscissa_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbAbscissa_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, "maximum", NC_INT, 1, &storelocal->mbAbscissa_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbAbscissa_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, "valid_minimum", NC_INT, 1, &storelocal->mbAbscissa_valid_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbAbscissa_valid_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, "valid_maximum", NC_INT, 1, &storelocal->mbAbscissa_valid_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbAbscissa_valid_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, "missing_value", NC_INT, 1, &storelocal->mbAbscissa_missing_value);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbAbscissa_missing_value error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, "format_C", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbAbscissa_format_C); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, "orientation", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbAbscissa_orientation); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbAltitude_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbAltitude_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbAltitude_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, "units", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbAltitude_units); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, "unit_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbAltitude_unit_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, "add_offset", NC_DOUBLE, 1, &storelocal->mbAltitude_add_offset);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbAltitude_add_offset error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, "scale_factor", NC_DOUBLE, 1, &storelocal->mbAltitude_scale_factor);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbAltitude_scale_factor error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, "minimum", NC_INT, 1, &storelocal->mbAltitude_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbAltitude_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, "maximum", NC_INT, 1, &storelocal->mbAltitude_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbAltitude_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, "valid_minimum", NC_INT, 1, &storelocal->mbAltitude_valid_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbAltitude_valid_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, "valid_maximum", NC_INT, 1, &storelocal->mbAltitude_valid_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbAltitude_valid_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, "missing_value", NC_INT, 1, &storelocal->mbAltitude_missing_value);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbAltitude_missing_value error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, "format_C", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbAltitude_format_C); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, "orientation", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbAltitude_orientation); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbImmersion_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbImmersion_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbImmersion_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, "units", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbImmersion_units); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, "unit_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbImmersion_unit_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, "add_offset", NC_DOUBLE, 1, &storelocal->mbImmersion_add_offset);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbImmersion_add_offset error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, "scale_factor", NC_DOUBLE, 1, &storelocal->mbImmersion_scale_factor);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbImmersion_scale_factor error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, "minimum", NC_INT, 1, &storelocal->mbImmersion_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbImmersion_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, "maximum", NC_INT, 1, &storelocal->mbImmersion_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbImmersion_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, "valid_minimum", NC_INT, 1, &storelocal->mbImmersion_valid_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbImmersion_valid_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, "valid_maximum", NC_INT, 1, &storelocal->mbImmersion_valid_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbImmersion_valid_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, "missing_value", NC_INT, 1, &storelocal->mbImmersion_missing_value);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbImmersion_missing_value error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, "format_C", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbImmersion_format_C); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, "orientation", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbImmersion_orientation); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHeading_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHeading_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHeading_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, "units", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHeading_units); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, "unit_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHeading_unit_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, "add_offset", NC_DOUBLE, 1, &storelocal->mbHeading_add_offset);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHeading_add_offset error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, "scale_factor", NC_DOUBLE, 1, &storelocal->mbHeading_scale_factor);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHeading_scale_factor error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, "minimum", NC_INT, 1, &storelocal->mbHeading_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHeading_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, "maximum", NC_INT, 1, &storelocal->mbHeading_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHeading_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, "valid_minimum", NC_INT, 1, &storelocal->mbHeading_valid_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHeading_valid_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, "valid_maximum", NC_INT, 1, &storelocal->mbHeading_valid_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHeading_valid_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, "missing_value", NC_INT, 1, &storelocal->mbHeading_missing_value);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbHeading_missing_value error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, "format_C", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHeading_format_C); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, "orientation", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbHeading_orientation); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbSpeed_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbSpeed_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbSpeed_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, "units", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbSpeed_units); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, "unit_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbSpeed_unit_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, "add_offset", NC_DOUBLE, 1, &storelocal->mbSpeed_add_offset);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbSpeed_add_offset error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_double((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, "scale_factor", NC_DOUBLE, 1, &storelocal->mbSpeed_scale_factor);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbSpeed_scale_factor error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, "minimum", NC_INT, 1, &storelocal->mbSpeed_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbSpeed_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, "maximum", NC_INT, 1, &storelocal->mbSpeed_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbSpeed_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, "valid_minimum", NC_INT, 1, &storelocal->mbSpeed_valid_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbSpeed_valid_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, "valid_maximum", NC_INT, 1, &storelocal->mbSpeed_valid_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbSpeed_valid_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, "missing_value", NC_INT, 1, &storelocal->mbSpeed_missing_value);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbSpeed_missing_value error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, "format_C", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbSpeed_format_C); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, "orientation", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbSpeed_orientation); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPType_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPType_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPType_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPType_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPType_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPType_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPType_id, "units", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPType_units); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPType_id, "unit_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPType_unit_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPType_id, "add_offset", NC_INT, 1, &storelocal->mbPType_add_offset);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPType_add_offset error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPType_id, "scale_factor", NC_INT, 1, &storelocal->mbPType_scale_factor);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPType_scale_factor error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPType_id, "minimum", NC_INT, 1, &storelocal->mbPType_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPType_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPType_id, "maximum", NC_INT, 1, &storelocal->mbPType_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPType_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPType_id, "valid_minimum", NC_INT, 1, &storelocal->mbPType_valid_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPType_valid_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPType_id, "valid_maximum", NC_INT, 1, &storelocal->mbPType_valid_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPType_valid_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPType_id, "missing_value", NC_INT, 1, &storelocal->mbPType_missing_value);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPType_missing_value error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPType_id, "format_C", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPType_format_C); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPType_id, "orientation", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPType_orientation); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPQuality_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPQuality_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPQuality_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, "units", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPQuality_units); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, "unit_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPQuality_unit_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, "add_offset", NC_INT, 1, &storelocal->mbPQuality_add_offset);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPQuality_add_offset error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, "scale_factor", NC_INT, 1, &storelocal->mbPQuality_scale_factor);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPQuality_scale_factor error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, "minimum", NC_INT, 1, &storelocal->mbPQuality_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPQuality_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, "maximum", NC_INT, 1, &storelocal->mbPQuality_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPQuality_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, "valid_minimum", NC_INT, 1, &storelocal->mbPQuality_valid_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPQuality_valid_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, "valid_maximum", NC_INT, 1, &storelocal->mbPQuality_valid_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPQuality_valid_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, "missing_value", NC_INT, 1, &storelocal->mbPQuality_missing_value);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPQuality_missing_value error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, "format_C", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPQuality_format_C); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, "orientation", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPQuality_orientation); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, "type", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPFlag_type); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, "long_name", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPFlag_long_name); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, "name_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPFlag_name_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, "units", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPFlag_units); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, "unit_code", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPFlag_unit_code); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, "add_offset", NC_INT, 1, &storelocal->mbPFlag_add_offset);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPFlag_add_offset error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, "scale_factor", NC_INT, 1, &storelocal->mbPFlag_scale_factor);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPFlag_scale_factor error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, "minimum", NC_INT, 1, &storelocal->mbPFlag_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPFlag_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, "maximum", NC_INT, 1, &storelocal->mbPFlag_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPFlag_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, "valid_minimum", NC_INT, 1, &storelocal->mbPFlag_valid_minimum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPFlag_valid_minimum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, "valid_maximum", NC_INT, 1, &storelocal->mbPFlag_valid_maximum);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPFlag_valid_maximum error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_int((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, "missing_value", NC_INT, 1, &storelocal->mbPFlag_missing_value);
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att mbPFlag_missing_value error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, "format_C", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPFlag_format_C); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
	    nc_status = nc_put_att_text((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, "orientation", MBSYS_NAVNETCDF_ATTRIBUTELEN, storelocal->mbPFlag_orientation); 
	    if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_att JJJJ error: %s\n", nc_strerror(nc_status));
		
	    /* print input debug statements */
	    if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  NetCDF variable attributes written in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Variable attributes:\n");
		fprintf(stderr,"dbg2       status:                  %d\n", status);
		fprintf(stderr,"dbg2       error:                   %d\n", *error);
		fprintf(stderr,"dbg2       nc_status:               %d\n", nc_status);
		fprintf(stderr,"dbg2       mbHistCode_long_name:		%s\n", storelocal->mbHistCode_long_name);
		fprintf(stderr,"dbg2       mbHistCode_name_code:		%s\n", storelocal->mbHistCode_name_code);
		fprintf(stderr,"dbg2       mbHistCode_units:	    %s\n", storelocal->mbHistCode_units);
		fprintf(stderr,"dbg2       mbHistCode_unit_code:		    %s\n", storelocal->mbHistCode_unit_code);
		fprintf(stderr,"dbg2       mbHistCode_add_offset:		    %d\n", storelocal->mbHistCode_add_offset);
		fprintf(stderr,"dbg2       mbHistCode_scale_factor:		%d\n", storelocal->mbHistCode_scale_factor);
		fprintf(stderr,"dbg2       mbHistCode_minimum:	    %d\n", storelocal->mbHistCode_minimum);
		fprintf(stderr,"dbg2       mbHistCode_maximum:	    %d\n", storelocal->mbHistCode_maximum);
		fprintf(stderr,"dbg2       mbHistCode_valid_minimum:	    %d\n", storelocal->mbHistCode_valid_minimum);
		fprintf(stderr,"dbg2       mbHistCode_valid_maximum:	%d\n", storelocal->mbHistCode_valid_maximum);
		fprintf(stderr,"dbg2       mbHistCode_missing_value:	%d\n", storelocal->mbHistCode_missing_value);
		fprintf(stderr,"dbg2       mbHistCode_format_C:	    %s\n", storelocal->mbHistCode_format_C);
		fprintf(stderr,"dbg2       mbHistCode_orientation:		%s\n", storelocal->mbHistCode_orientation);
		fprintf(stderr,"dbg2       mbHistAutor_type:	    %s\n", storelocal->mbHistAutor_type);
		fprintf(stderr,"dbg2       mbHistAutor_long_name:		%s\n", storelocal->mbHistAutor_long_name);
		fprintf(stderr,"dbg2       mbHistAutor_name_code:		%s\n", storelocal->mbHistAutor_name_code);
		fprintf(stderr,"dbg2       mbHistModule_type:	    %s\n", storelocal->mbHistModule_type);
		fprintf(stderr,"dbg2       mbHistModule_long_name:		%s\n", storelocal->mbHistModule_long_name);
		fprintf(stderr,"dbg2       mbHistModule_name_code:		%s\n", storelocal->mbHistModule_name_code);
		fprintf(stderr,"dbg2       mbHistComment_type:	    %s\n", storelocal->mbHistComment_type);
		fprintf(stderr,"dbg2       mbHistComment_long_name:		%s\n", storelocal->mbHistComment_long_name);
		fprintf(stderr,"dbg2       mbHistComment_name_code:		%s\n", storelocal->mbHistComment_name_code);
		fprintf(stderr,"dbg2       mbDate_type:		%s\n", storelocal->mbDate_type);
		fprintf(stderr,"dbg2       mbDate_long_name:		%s\n", storelocal->mbDate_long_name);
		fprintf(stderr,"dbg2       mbDate_name_code:		%s\n", storelocal->mbDate_name_code);
		fprintf(stderr,"dbg2       mbDate_units:		%s\n", storelocal->mbDate_units);
		fprintf(stderr,"dbg2       mbDate_unit_code:		%s\n", storelocal->mbDate_unit_code);
		fprintf(stderr,"dbg2       mbDate_add_offset:		%d\n", storelocal->mbDate_add_offset);
		fprintf(stderr,"dbg2       mbDate_scale_factor:		%d\n", storelocal->mbDate_scale_factor);
		fprintf(stderr,"dbg2       mbDate_minimum:		%d\n", storelocal->mbDate_minimum);
		fprintf(stderr,"dbg2       mbDate_maximum:		%d\n", storelocal->mbDate_maximum);
		fprintf(stderr,"dbg2       mbDate_valid_minimum:		%d\n", storelocal->mbDate_valid_minimum);
		fprintf(stderr,"dbg2       mbDate_valid_maximum:		%d\n", storelocal->mbDate_valid_maximum);
		fprintf(stderr,"dbg2       mbDate_missing_value:		%d\n", storelocal->mbDate_missing_value);
		fprintf(stderr,"dbg2       mbDate_format_C:		%s\n", storelocal->mbDate_format_C);
		fprintf(stderr,"dbg2       mbDate_orientation:		%s\n", storelocal->mbDate_orientation);
		fprintf(stderr,"dbg2       mbTime_type:		%s\n", storelocal->mbTime_type);
		fprintf(stderr,"dbg2       mbTime_long_name:		%s\n", storelocal->mbTime_long_name);
		fprintf(stderr,"dbg2       mbTime_name_code:		%s\n", storelocal->mbTime_name_code);
		fprintf(stderr,"dbg2       mbTime_units:		%s\n", storelocal->mbTime_units);
		fprintf(stderr,"dbg2       mbTime_unit_code:		%s\n", storelocal->mbTime_unit_code);
		fprintf(stderr,"dbg2       mbTime_add_offset:		%d\n", storelocal->mbTime_add_offset);
		fprintf(stderr,"dbg2       mbTime_scale_factor:		%d\n", storelocal->mbTime_scale_factor);
		fprintf(stderr,"dbg2       mbTime_minimum:		%d\n", storelocal->mbTime_minimum);
		fprintf(stderr,"dbg2       mbTime_maximum:		%d\n", storelocal->mbTime_maximum);
		fprintf(stderr,"dbg2       mbTime_valid_minimum:		%d\n", storelocal->mbTime_valid_minimum);
		fprintf(stderr,"dbg2       mbTime_valid_maximum:		%d\n", storelocal->mbTime_valid_maximum);
		fprintf(stderr,"dbg2       mbTime_missing_value:		%d\n", storelocal->mbTime_missing_value);
		fprintf(stderr,"dbg2       mbTime_format_C:		%s\n", storelocal->mbTime_format_C);
		fprintf(stderr,"dbg2       mbTime_orientation:		%s\n", storelocal->mbTime_orientation);
		fprintf(stderr,"dbg2       mbOrdinate_type:		%s\n", storelocal->mbOrdinate_type);
		fprintf(stderr,"dbg2       mbOrdinate_long_name:		%s\n", storelocal->mbOrdinate_long_name);
		fprintf(stderr,"dbg2       mbOrdinate_name_code:		%s\n", storelocal->mbOrdinate_name_code);
		fprintf(stderr,"dbg2       mbOrdinate_units:		%s\n", storelocal->mbOrdinate_units);
		fprintf(stderr,"dbg2       mbOrdinate_unit_code:		%s\n", storelocal->mbOrdinate_unit_code);
		fprintf(stderr,"dbg2       mbOrdinate_add_offset:		%f\n", storelocal->mbOrdinate_add_offset);
		fprintf(stderr,"dbg2       mbOrdinate_scale_factor:		%f\n", storelocal->mbOrdinate_scale_factor);
		fprintf(stderr,"dbg2       mbOrdinate_minimum:		%d\n", storelocal->mbOrdinate_minimum);
		fprintf(stderr,"dbg2       mbOrdinate_maximum:		%d\n", storelocal->mbOrdinate_maximum);
		fprintf(stderr,"dbg2       mbOrdinate_valid_minimum:		%d\n", storelocal->mbOrdinate_valid_minimum);
		fprintf(stderr,"dbg2       mbOrdinate_valid_maximum:		%d\n", storelocal->mbOrdinate_valid_maximum);
		fprintf(stderr,"dbg2       mbOrdinate_missing_value:		%d\n", storelocal->mbOrdinate_missing_value);
		fprintf(stderr,"dbg2       mbOrdinate_format_C:		%s\n", storelocal->mbOrdinate_format_C);
		fprintf(stderr,"dbg2       mbOrdinate_orientation:		%s\n", storelocal->mbOrdinate_orientation);
		fprintf(stderr,"dbg2       mbAbscissa_type:		%s\n", storelocal->mbAbscissa_type);
		fprintf(stderr,"dbg2       mbAbscissa_long_name:		%s\n", storelocal->mbAbscissa_long_name);
		fprintf(stderr,"dbg2       mbAbscissa_name_code:		%s\n", storelocal->mbAbscissa_name_code);
		fprintf(stderr,"dbg2       mbAbscissa_units:		%s\n", storelocal->mbAbscissa_units);
		fprintf(stderr,"dbg2       mbAbscissa_unit_code:		%s\n", storelocal->mbAbscissa_unit_code);
		fprintf(stderr,"dbg2       mbAbscissa_add_offset:		%f\n", storelocal->mbAbscissa_add_offset);
		fprintf(stderr,"dbg2       mbAbscissa_scale_factor:		%f\n", storelocal->mbAbscissa_scale_factor);
		fprintf(stderr,"dbg2       mbAbscissa_minimum:		%d\n", storelocal->mbAbscissa_minimum);
		fprintf(stderr,"dbg2       mbAbscissa_maximum:		%d\n", storelocal->mbAbscissa_maximum);
		fprintf(stderr,"dbg2       mbAbscissa_valid_minimum:		%d\n", storelocal->mbAbscissa_valid_minimum);
		fprintf(stderr,"dbg2       mbAbscissa_valid_maximum:		%d\n", storelocal->mbAbscissa_valid_maximum);
		fprintf(stderr,"dbg2       mbAbscissa_missing_value:		%d\n", storelocal->mbAbscissa_missing_value);
		fprintf(stderr,"dbg2       mbAbscissa_format_C:		%s\n", storelocal->mbAbscissa_format_C);
		fprintf(stderr,"dbg2       mbAbscissa_orientation:		%s\n", storelocal->mbAbscissa_orientation);
		fprintf(stderr,"dbg2       mbAltitude_type:		%s\n", storelocal->mbAltitude_type);
		fprintf(stderr,"dbg2       mbAltitude_long_name:		%s\n", storelocal->mbAltitude_long_name);
		fprintf(stderr,"dbg2       mbAltitude_name_code:		%s\n", storelocal->mbAltitude_name_code);
		fprintf(stderr,"dbg2       mbAltitude_units:		%s\n", storelocal->mbAltitude_units);
		fprintf(stderr,"dbg2       mbAltitude_unit_code:		%s\n", storelocal->mbAltitude_unit_code);
		fprintf(stderr,"dbg2       mbAltitude_add_offset:		%f\n", storelocal->mbAltitude_add_offset);
		fprintf(stderr,"dbg2       mbAltitude_scale_factor:		%f\n", storelocal->mbAltitude_scale_factor);
		fprintf(stderr,"dbg2       mbAltitude_minimum:		%d\n", storelocal->mbAltitude_minimum);
		fprintf(stderr,"dbg2       mbAltitude_maximum:		%d\n", storelocal->mbAltitude_maximum);
		fprintf(stderr,"dbg2       mbAltitude_valid_minimum:		%d\n", storelocal->mbAltitude_valid_minimum);
		fprintf(stderr,"dbg2       mbAltitude_valid_maximum:		%d\n", storelocal->mbAltitude_valid_maximum);
		fprintf(stderr,"dbg2       mbAltitude_missing_value:		%d\n", storelocal->mbAltitude_missing_value);
		fprintf(stderr,"dbg2       mbAltitude_format_C:		%s\n", storelocal->mbAltitude_format_C);
		fprintf(stderr,"dbg2       mbAltitude_orientation:		%s\n", storelocal->mbAltitude_orientation);
		fprintf(stderr,"dbg2       mbImmersion_type:		%s\n", storelocal->mbImmersion_type);
		fprintf(stderr,"dbg2       mbImmersion_long_name:		%s\n", storelocal->mbImmersion_long_name);
		fprintf(stderr,"dbg2       mbImmersion_name_code:		%s\n", storelocal->mbImmersion_name_code);
		fprintf(stderr,"dbg2       mbImmersion_units:		%s\n", storelocal->mbImmersion_units);
		fprintf(stderr,"dbg2       mbImmersion_unit_code:		%s\n", storelocal->mbImmersion_unit_code);
		fprintf(stderr,"dbg2       mbImmersion_add_offset:		%f\n", storelocal->mbImmersion_add_offset);
		fprintf(stderr,"dbg2       mbImmersion_scale_factor:		%f\n", storelocal->mbImmersion_scale_factor);
		fprintf(stderr,"dbg2       mbImmersion_minimum:		%d\n", storelocal->mbImmersion_minimum);
		fprintf(stderr,"dbg2       mbImmersion_maximum:		%d\n", storelocal->mbImmersion_maximum);
		fprintf(stderr,"dbg2       mbImmersion_valid_minimum:		%d\n", storelocal->mbImmersion_valid_minimum);
		fprintf(stderr,"dbg2       mbImmersion_valid_maximum:		%d\n", storelocal->mbImmersion_valid_maximum);
		fprintf(stderr,"dbg2       mbImmersion_missing_value:		%d\n", storelocal->mbImmersion_missing_value);
		fprintf(stderr,"dbg2       mbImmersion_format_C:		%s\n", storelocal->mbImmersion_format_C);
		fprintf(stderr,"dbg2       mbImmersion_orientation:		%s\n", storelocal->mbImmersion_orientation);
		fprintf(stderr,"dbg2       mbHeading_type:		%s\n", storelocal->mbHeading_type);
		fprintf(stderr,"dbg2       mbHeading_long_name:		%s\n", storelocal->mbHeading_long_name);
		fprintf(stderr,"dbg2       mbHeading_name_code:		%s\n", storelocal->mbHeading_name_code);
		fprintf(stderr,"dbg2       mbHeading_units:		%s\n", storelocal->mbHeading_units);
		fprintf(stderr,"dbg2       mbHeading_unit_code:		%s\n", storelocal->mbHeading_unit_code);
		fprintf(stderr,"dbg2       mbHeading_add_offset:		%f\n", storelocal->mbHeading_add_offset);
		fprintf(stderr,"dbg2       mbHeading_scale_factor:		%f\n", storelocal->mbHeading_scale_factor);
		fprintf(stderr,"dbg2       mbHeading_minimum:		%d\n", storelocal->mbHeading_minimum);
		fprintf(stderr,"dbg2       mbHeading_maximum:		%d\n", storelocal->mbHeading_maximum);
		fprintf(stderr,"dbg2       mbHeading_valid_minimum:		%d\n", storelocal->mbHeading_valid_minimum);
		fprintf(stderr,"dbg2       mbHeading_valid_maximum:		%d\n", storelocal->mbHeading_valid_maximum);
		fprintf(stderr,"dbg2       mbHeading_missing_value:		%d\n", storelocal->mbHeading_missing_value);
		fprintf(stderr,"dbg2       mbHeading_format_C:		%s\n", storelocal->mbHeading_format_C);
		fprintf(stderr,"dbg2       mbHeading_orientation:		%s\n", storelocal->mbHeading_orientation);
		fprintf(stderr,"dbg2       mbSpeed_type:		%s\n", storelocal->mbSpeed_type);
		fprintf(stderr,"dbg2       mbSpeed_long_name:		%s\n", storelocal->mbSpeed_long_name);
		fprintf(stderr,"dbg2       mbSpeed_name_code:		%s\n", storelocal->mbSpeed_name_code);
		fprintf(stderr,"dbg2       mbSpeed_units:		%s\n", storelocal->mbSpeed_units);
		fprintf(stderr,"dbg2       mbSpeed_unit_code:		%s\n", storelocal->mbSpeed_unit_code);
		fprintf(stderr,"dbg2       mbSpeed_add_offset:		%f\n", storelocal->mbSpeed_add_offset);
		fprintf(stderr,"dbg2       mbSpeed_scale_factor:		%f\n", storelocal->mbSpeed_scale_factor);
		fprintf(stderr,"dbg2       mbSpeed_minimum:		%d\n", storelocal->mbSpeed_minimum);
		fprintf(stderr,"dbg2       mbSpeed_maximum:		%d\n", storelocal->mbSpeed_maximum);
		fprintf(stderr,"dbg2       mbSpeed_valid_minimum:		%d\n", storelocal->mbSpeed_valid_minimum);
		fprintf(stderr,"dbg2       mbSpeed_valid_maximum:		%d\n", storelocal->mbSpeed_valid_maximum);
		fprintf(stderr,"dbg2       mbSpeed_missing_value:		%d\n", storelocal->mbSpeed_missing_value);
		fprintf(stderr,"dbg2       mbSpeed_format_C:		%s\n", storelocal->mbSpeed_format_C);
		fprintf(stderr,"dbg2       mbSpeed_orientation:		%s\n", storelocal->mbSpeed_orientation);
		fprintf(stderr,"dbg2       mbPType_type:		%s\n", storelocal->mbPType_type);
		fprintf(stderr,"dbg2       mbPType_long_name:		%s\n", storelocal->mbPType_long_name);
		fprintf(stderr,"dbg2       mbPType_name_code:		%s\n", storelocal->mbPType_name_code);
		fprintf(stderr,"dbg2       mbPType_units:		%s\n", storelocal->mbPType_units);
		fprintf(stderr,"dbg2       mbPType_unit_code:		%s\n", storelocal->mbPType_unit_code);
		fprintf(stderr,"dbg2       mbPType_add_offset:		%d\n", storelocal->mbPType_add_offset);
		fprintf(stderr,"dbg2       mbPType_scale_factor:		%d\n", storelocal->mbPType_scale_factor);
		fprintf(stderr,"dbg2       mbPType_minimum:		%d\n", storelocal->mbPType_minimum);
		fprintf(stderr,"dbg2       mbPType_maximum:		%d\n", storelocal->mbPType_maximum);
		fprintf(stderr,"dbg2       mbPType_valid_minimum:		%d\n", storelocal->mbPType_valid_minimum);
		fprintf(stderr,"dbg2       mbPType_valid_maximum:		%d\n", storelocal->mbPType_valid_maximum);
		fprintf(stderr,"dbg2       mbPType_missing_value:		%d\n", storelocal->mbPType_missing_value);
		fprintf(stderr,"dbg2       mbPType_format_C:		%s\n", storelocal->mbPType_format_C);
		fprintf(stderr,"dbg2       mbPType_orientation:		%s\n", storelocal->mbPType_orientation);
		fprintf(stderr,"dbg2       mbPQuality_type:		%s\n", storelocal->mbPQuality_type);
		fprintf(stderr,"dbg2       mbPQuality_long_name:		%s\n", storelocal->mbPQuality_long_name);
		fprintf(stderr,"dbg2       mbPQuality_name_code:		%s\n", storelocal->mbPQuality_name_code);
		fprintf(stderr,"dbg2       mbPQuality_units:		%s\n", storelocal->mbPQuality_units);
		fprintf(stderr,"dbg2       mbPQuality_unit_code:		%s\n", storelocal->mbPQuality_unit_code);
		fprintf(stderr,"dbg2       mbPQuality_add_offset:		%d\n", storelocal->mbPQuality_add_offset);
		fprintf(stderr,"dbg2       mbPQuality_scale_factor:		%d\n", storelocal->mbPQuality_scale_factor);
		fprintf(stderr,"dbg2       mbPQuality_minimum:		%d\n", storelocal->mbPQuality_minimum);
		fprintf(stderr,"dbg2       mbPQuality_maximum:		%d\n", storelocal->mbPQuality_maximum);
		fprintf(stderr,"dbg2       mbPQuality_valid_minimum:		%d\n", storelocal->mbPQuality_valid_minimum);
		fprintf(stderr,"dbg2       mbPQuality_valid_maximum:		%d\n", storelocal->mbPQuality_valid_maximum);
		fprintf(stderr,"dbg2       mbPQuality_missing_value:		%d\n", storelocal->mbPQuality_missing_value);
		fprintf(stderr,"dbg2       mbPQuality_format_C:		%s\n", storelocal->mbPQuality_format_C);
		fprintf(stderr,"dbg2       mbPQuality_orientation:		%s\n", storelocal->mbPQuality_orientation);
		fprintf(stderr,"dbg2       mbPFlag_type:		%s\n", storelocal->mbPFlag_type);
		fprintf(stderr,"dbg2       mbPFlag_long_name:		%s\n", storelocal->mbPFlag_long_name);
		fprintf(stderr,"dbg2       mbPFlag_name_code:		%s\n", storelocal->mbPFlag_name_code);
		fprintf(stderr,"dbg2       mbPFlag_units:		%s\n", storelocal->mbPFlag_units);
		fprintf(stderr,"dbg2       mbPFlag_unit_code:		%s\n", storelocal->mbPFlag_unit_code);
		fprintf(stderr,"dbg2       mbPFlag_add_offset:		%d\n", storelocal->mbPFlag_add_offset);
		fprintf(stderr,"dbg2       mbPFlag_scale_factor:		%d\n", storelocal->mbPFlag_scale_factor);
		fprintf(stderr,"dbg2       mbPFlag_minimum:		%d\n", storelocal->mbPFlag_minimum);
		fprintf(stderr,"dbg2       mbPFlag_maximum:		%d\n", storelocal->mbPFlag_maximum);
		fprintf(stderr,"dbg2       mbPFlag_valid_minimum:		%d\n", storelocal->mbPFlag_valid_minimum);
		fprintf(stderr,"dbg2       mbPFlag_valid_maximum:		%d\n", storelocal->mbPFlag_valid_maximum);
		fprintf(stderr,"dbg2       mbPFlag_missing_value:		%d\n", storelocal->mbPFlag_missing_value);
		fprintf(stderr,"dbg2       mbPFlag_format_C:		%s\n", storelocal->mbPFlag_format_C);
		fprintf(stderr,"dbg2       mbPFlag_orientation:		%s\n", storelocal->mbPFlag_orientation);
		}

	    /* end define mode */
	    nc_status = nc_enddef((int)mb_io_ptr->mbfp);

	    /* save the comments */
	    
	    /* save the beam arrays */
	    
	    /* save the soundvel arrays */

	    /* write global variables */
	    if (status == MB_SUCCESS)
		{
		index[0] = 0;
		count[0] = storelocal->mbHistoryRecNbr;
		nc_status = nc_put_vara_int((int)mb_io_ptr->mbfp, storelocal->mbHistDate_id, index, count, storelocal->mbHistDate);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbHistDate error: %s\n", nc_strerror(nc_status));

		index[0] = 0;
		count[0] = storelocal->mbHistoryRecNbr;
		nc_status = nc_put_vara_int((int)mb_io_ptr->mbfp, storelocal->mbHistTime_id, index, count, storelocal->mbHistTime);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbHistTime error: %s\n", nc_strerror(nc_status));
		
		index[0] = 0;
		count[0] = storelocal->mbHistoryRecNbr;
		nc_status = nc_put_vara_text((int)mb_io_ptr->mbfp, storelocal->mbHistCode_id, index, count, storelocal->mbHistCode);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbHistCode error: %s\n", nc_strerror(nc_status));
		
		index[0] = 0;
		index[1] = 0;
		count[0] = storelocal->mbHistoryRecNbr;
		count[1] = storelocal->mbNameLength;
		nc_status = nc_put_vara_text((int)mb_io_ptr->mbfp, storelocal->mbHistAutor_id, index, count, storelocal->mbHistAutor);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbHistAutor error: %s\n", nc_strerror(nc_status));
		
		index[0] = 0;
		index[1] = 0;
		count[0] = storelocal->mbHistoryRecNbr;
		count[1] = storelocal->mbNameLength;
		nc_status = nc_put_vara_text((int)mb_io_ptr->mbfp, storelocal->mbHistModule_id, index, count, storelocal->mbHistModule);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbHistModule error: %s\n", nc_strerror(nc_status));
		
		index[0] = 0;
		index[1] = 0;
		count[0] = storelocal->mbHistoryRecNbr;
		count[1] = storelocal->mbCommentLength;
		nc_status = nc_put_vara_text((int)mb_io_ptr->mbfp, storelocal->mbHistComment_id, index, count, storelocal->mbHistComment);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbHistComment error: %s\n", nc_strerror(nc_status));

		if (nc_status != NC_NOERR)
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }		
		}
	    }
	
	/* now write the data */
	if (store->kind == MB_DATA_DATA
	    && status == MB_SUCCESS)
	    {
	    /* write the variables from next record */
	    index[0] = *recwrite;
	    count[0] = 1;
	    nc_status = nc_put_vara_int((int)mb_io_ptr->mbfp, storelocal->mbDate_id, index, count, &store->mbDate);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbDate error: %s\n", nc_strerror(nc_status));
	    index[0] = *recwrite;
	    count[0] = 1;
	    nc_status = nc_put_vara_int((int)mb_io_ptr->mbfp, storelocal->mbTime_id, index, count, &store->mbTime);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbTime error: %s\n", nc_strerror(nc_status));
	    index[0] = *recwrite;
	    count[0] = 1;
	    nc_status = nc_put_vara_int((int)mb_io_ptr->mbfp, storelocal->mbOrdinate_id, index, count, &store->mbOrdinate);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbOrdinate error: %s\n", nc_strerror(nc_status));
	    index[0] = *recwrite;
	    count[0] = 1;
	    nc_status = nc_put_vara_int((int)mb_io_ptr->mbfp, storelocal->mbAbscissa_id, index, count, &store->mbAbscissa);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbAbscissa error: %s\n", nc_strerror(nc_status));
	    index[0] = *recwrite;
	    count[0] = 1;
	    nc_status = nc_put_vara_short((int)mb_io_ptr->mbfp, storelocal->mbAltitude_id, index, count, &store->mbAltitude);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbAltitude error: %s\n", nc_strerror(nc_status));
	    index[0] = *recwrite;
	    count[0] = 1;
	    nc_status = nc_put_vara_short((int)mb_io_ptr->mbfp, storelocal->mbImmersion_id, index, count, &store->mbImmersion);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbImmersion error: %s\n", nc_strerror(nc_status));
	    index[0] = *recwrite;
	    count[0] = 1;
	    nc_status = nc_put_vara_int((int)mb_io_ptr->mbfp, storelocal->mbHeading_id, index, count, &store->mbHeading);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbHeading error: %s\n", nc_strerror(nc_status));
	    index[0] = *recwrite;
	    count[0] = 1;
	    nc_status = nc_put_vara_short((int)mb_io_ptr->mbfp, storelocal->mbSpeed_id, index, count, &store->mbSpeed);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbSpeed error: %s\n", nc_strerror(nc_status));
	    index[0] = *recwrite;
	    count[0] = 1;
	    nc_status = nc_put_vara_text((int)mb_io_ptr->mbfp, storelocal->mbPType_id, index, count, &store->mbPType);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbPType error: %s\n", nc_strerror(nc_status));
	    index[0] = *recwrite;
	    count[0] = 1;
	    nc_status = nc_put_vara_text((int)mb_io_ptr->mbfp, storelocal->mbPQuality_id, index, count, &store->mbPQuality);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbPQuality error: %s\n", nc_strerror(nc_status));
	    index[0] = *recwrite;
	    count[0] = 1;
	    nc_status = nc_put_vara_text((int)mb_io_ptr->mbfp, storelocal->mbPFlag_id, index, count, &store->mbPFlag);
	    	if ((verbose >= 2 || nc_verbose >= 1) && nc_status != NC_NOERR) fprintf(stderr, "nc_put_vara mbPFlag error: %s\n", nc_strerror(nc_status));
	    
	    /* check status */
	    if (nc_status != NC_NOERR)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}		
		
	    /* print input debug statements */
	    if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  NetCDF Survey Record written in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Global Variables:\n");
		fprintf(stderr,"dbg2       status:                  %d\n", status);
		fprintf(stderr,"dbg2       error:                   %d\n", *error);
		fprintf(stderr,"dbg2       nc_status:               %d\n", nc_status);
		fprintf(stderr,"dbg2       mbDate:                  %d\n", store->mbDate);
		fprintf(stderr,"dbg2       mbTime:                  %d\n", store->mbTime);
		fprintf(stderr,"dbg2       mbOrdinate:              %d\n", store->mbOrdinate);
		fprintf(stderr,"dbg2       mbAbscissa:              %d\n", store->mbAbscissa);
		fprintf(stderr,"dbg2       mbAltitude:              %d\n", store->mbAltitude);
		fprintf(stderr,"dbg2       mbImmersion:             %d\n", store->mbImmersion);
		fprintf(stderr,"dbg2       mbHeading:               %d\n", store->mbHeading);
		fprintf(stderr,"dbg2       mbSpeed:                 %d\n", store->mbSpeed);
		fprintf(stderr,"dbg2       mbPType:                 %d\n", store->mbPType);
		fprintf(stderr,"dbg2       mbPQuality:              %d\n", store->mbPQuality);
		fprintf(stderr,"dbg2       mbPFlag:                 %d\n", store->mbPFlag);
		}
	    
	    /* set counters */
	    (*recwrite)++;
	    (*datawrite)++;
	    
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
