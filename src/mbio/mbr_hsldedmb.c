/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hsldedmb.c	2/2/93
 *	$Id$
 *
 *    Copyright (c) 1993-2012 by
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
 * mbr_hsldedmb.c contains the functions for reading and writing
 * multibeam data in the HSLDEDMB format.  
 * These functions include:
 *   mbr_alm_hsldedmb	- allocate read/write memory
 *   mbr_dem_hsldedmb	- deallocate read/write memory
 *   mbr_rt_hsldedmb	- read and translate data
 *   mbr_wt_hsldedmb	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
 * $Log: mbr_hsldedmb.c,v $
 * Revision 5.10  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.9  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.8  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.7  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.6  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.5  2002/03/26 07:43:25  caress
 * Release 5.0.beta15
 *
 * Revision 5.4  2002/02/26 07:50:41  caress
 * Release 5.0.beta14
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
 * Revision 4.10  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.9  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.8  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.7  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.6  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.5  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.5  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.4  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.3  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.2  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.1  1994/05/21  02:23:29  caress
 * Made sure that mb_io_ptr->new_bath_alongtrack is set to zero on reading.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/21  03:59:50  caress
 * First cut at new version. Altered to be consistent
 * with passing of three types of data: bathymetry,
 * amplitude, and sidescan.
 *
 * Revision 3.0  1993/05/14  22:55:40  sohara
 * initial version
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
#include "../../include/mbsys_hsds.h"
#include "../../include/mbf_hsldedmb.h"

/* time conversion constants */
#define MININYEAR 525600.0
#define MININDAY 1440.0

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* essential function prototypes */
int mbr_register_hsldedmb(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_hsldedmb(int verbose, 
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
int mbr_alm_hsldedmb(int verbose, void *mbio_ptr, int *error);
int mbr_dem_hsldedmb(int verbose, void *mbio_ptr, int *error);
int mbr_rt_hsldedmb(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_hsldedmb(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_hsldedmb(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_hsldedmb";
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
	status = mbr_info_hsldedmb(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_hsldedmb;
	mb_io_ptr->mb_io_format_free = &mbr_dem_hsldedmb; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_hsds_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_hsds_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_hsldedmb; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_hsldedmb; 
	mb_io_ptr->mb_io_dimensions = &mbsys_hsds_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_hsds_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_hsds_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_hsds_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_hsds_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_hsds_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL; 
	mb_io_ptr->mb_io_ttimes = &mbsys_hsds_ttimes; 
	mb_io_ptr->mb_io_detects = &mbsys_hsds_detects; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_hsds_copy; 
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
int mbr_info_hsldedmb(int verbose, 
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
	char	*function_name = "mbr_info_hsldedmb";
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
	*system = MB_SYS_HSDS;
	*beams_bath_max = 59;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "HSLDEDMB", MB_NAME_LENGTH);
	strncpy(system_name, "HSDS", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_HSLDEDMB\nInformal Description: EDMB Hydrosweep\nAttributes:           Hydrosweep DS, bathymetry, 59 beams, binary, NRL.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_NO;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 2.0;
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
int mbr_alm_hsldedmb(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_hsldedmb";
	int	status;
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
	mb_io_ptr->structure_size = sizeof(struct mbf_hsldedmb_struct);
	mb_io_ptr->data_structure_size = 
		sizeof(struct mbf_hsldedmb_data_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_hsds_struct),
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
int mbr_dem_hsldedmb(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_hsldedmb";
	int	status;
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
int mbr_rt_hsldedmb(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_hsldedmb";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsldedmb_struct *dataplus;
	struct mbf_hsldedmb_data_struct *data;
	struct mbsys_hsds_struct *store;
	char	*datacomment;
	int	i;
	int	id;

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
	dataplus = (struct mbf_hsldedmb_struct *) mb_io_ptr->raw_data;
	data = &(dataplus->data);
	datacomment = (char *) data;
	dataplus->kind = MB_DATA_DATA;
	store = (struct mbsys_hsds_struct *) store_ptr;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next record from file */
	if ((status = fread(data,1,mb_io_ptr->data_structure_size,
			mb_io_ptr->mbfp)) == mb_io_ptr->data_structure_size) 
		{
		mb_io_ptr->file_bytes += status;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		mb_io_ptr->file_bytes += status;
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS && data->seconds != 2054847098)
		{
		data->seconds = mb_swap_int(data->seconds);
		data->microseconds = mb_swap_int(data->microseconds);
		data->alt_seconds = mb_swap_int(data->alt_seconds);
		data->alt_microseconds = mb_swap_int(data->alt_microseconds);
		data->lat = mb_swap_int(data->lat);
		data->lon = mb_swap_int(data->lon);
		data->heading = mb_swap_short(data->heading);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->pitch = mb_swap_short(data->pitch);
		data->scale = mb_swap_short(data->scale);
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			data->depth[i] = mb_swap_short(data->depth[i]);
			data->range[i] = mb_swap_short(data->range[i]);
			}
		for (i=0;i<4;i++)
			data->flag[i] = mb_swap_int(data->flag[i]);

		}
#endif

	/* check for comment or unintelligible records */
	if (status == MB_SUCCESS)
		{
		if (data->seconds == 2054847098)
			{
			dataplus->kind = MB_DATA_COMMENT;
			}
		else if (data->seconds == 0)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			dataplus->kind = MB_DATA_NONE;
			}
		else
			{
			dataplus->kind = MB_DATA_DATA;
			}
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = dataplus->kind;
	mb_io_ptr->new_error = *error;

	/* translate values to hydrosweep data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* type of data record */
		store->kind = dataplus->kind;

		/* time stamp (all records ) */
		mb_io_ptr->new_time_d = data->seconds 
			+ 0.000001*data->microseconds;
		mb_get_date(verbose,mb_io_ptr->new_time_d,
			mb_io_ptr->new_time_i);
		store->year = mb_io_ptr->new_time_i[0];
		store->month = mb_io_ptr->new_time_i[1];
		store->day = mb_io_ptr->new_time_i[2];
		store->hour = mb_io_ptr->new_time_i[3];
		store->minute = mb_io_ptr->new_time_i[4];
		store->second = mb_io_ptr->new_time_i[5];
		store->alt_minute = 0;
		store->alt_second = 0;

		/* position (all records ) */
		store->lon = 0.0000001*data->lon;
		store->lat = 0.0000001*data->lat;
		if (store->lon > 180.) 
			store->lon = store->lon - 360.;
		else if (store->lon < -180.)
			store->lon = store->lon + 360.;

		/* additional navigation and depths (ERGNMESS and ERGNEICH) */
		store->course_true = 0.1*data->heading;
		store->speed_transverse = 0.0;
		store->speed = 0.1*data->speed;
		store->speed_reference[0] = data->speed_ref;
		store->pitch = 0.1*data->pitch;
		store->track = 0;
		store->depth_center 
			= data->depth[MBSYS_HSDS_BEAMS/2];
		/*store->depth_scale = 0.01*data->scale;*/
		store->depth_scale = 1.0;
		store->spare = 1;
		id = MBSYS_HSDS_BEAMS - 1;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			store->distance[id-i] = data->range[i];
			store->depth[id-i] = data->depth[i];
			}

		/* travel time data (ERGNSLZT) */
		store->course_ground = 0.1*data->course;
		store->speed_ground = 0.0;
		store->heave = 0.0;
		store->roll = 0.0;
		store->time_center = 0.0;
		store->time_scale = 0.0;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			store->time[i] = 0;
		for (i=0;i<11;i++)
			store->gyro[i] = 0.0;

		/* amplitude data (ERGNAMPL) */
		store->mode[0] = '\0';
		store->trans_strbd = 0;
		store->trans_vert = 0;
		store->trans_port = 0;
		store->pulse_len_strbd = 0;
		store->pulse_len_vert = 0;
		store->pulse_len_port = 0;
		store->gain_start = 0;
		store->r_compensation_factor = 0;
		store->compensation_start = 0;
		store->increase_start = 0;
		store->tvc_near = 0;
		store->tvc_far = 0;
		store->increase_int_near = 0;
		store->increase_int_far = 0;
		store->gain_center = 0;
		store->filter_gain = 0.0;
		store->amplitude_center = 0;
		store->echo_duration_center = 0;
		store->echo_scale_center = 0;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			store->amplitude[i] = 0;
			store->echo_duration[i] = 0;
			}
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			store->gain[i] = 0;
			store->echo_scale[i] = 0;
			}

		/* mean velocity (ERGNHYDI) */
		store->draught = 0.0;
		store->vel_mean = 0.0;
		store->vel_keel = 0.0;
		store->tide = 0.0;

		/* water velocity profile (HS_ERGNCTDS) */
		store->num_vel = 0.0;

		/* navigation source (ERGNPOSI) */
		store->pos_corr_x = 0.0;
		store->pos_corr_y = 0.0;
		strncpy(store->sensors,"\0",8);

		/* comment (LDEOCMNT) */
		strncpy(store->comment,&datacomment[4],
			MBSYS_HSDS_MAXLINE);

		/* processed backscatter */
		store->back_scale = 0.0;
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			store->back[i] = 0;
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
int mbr_wt_hsldedmb(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_hsldedmb";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_hsldedmb_struct *dataplus;
	struct mbf_hsldedmb_data_struct *data;
	struct mbsys_hsds_struct *store;
	char	*datacomment;
	int	time_i[7];
	double	time_d;
	int	i;
	int	id;

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
	dataplus = (struct mbf_hsldedmb_struct *) mb_io_ptr->raw_data;
	data = &(dataplus->data);
	datacomment = (char *) data;
	store = (struct mbsys_hsds_struct *) store_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Status at beginning of MBIO function <%s>\n",function_name);
		if (store != NULL)
			fprintf(stderr,"dbg5       store->kind:    %d\n",
				store->kind);
		fprintf(stderr,"dbg5       new_kind:       %d\n",
			mb_io_ptr->new_kind);
		fprintf(stderr,"dbg5       new_error:      %d\n",
			mb_io_ptr->new_error);
		fprintf(stderr,"dbg5       error:          %d\n",*error);
		fprintf(stderr,"dbg5       status:         %d\n",status);
		}

	/* first set some plausible amounts for some of the 
		variables in the HSLDEDMB record */
	data->course = 0;
	data->pitch = 0;
	data->scale = 100;	/* this is a unit scale factor */
	data->speed_ref = 'B';	/* assume speed is over the ground */
	data->quality = '\0';
	for (i=0;i<4;i++)
		data->flag[i] = 0.0;

	/* second translate values from hydrosweep data storage structure */
	if (store != NULL)
		{
		dataplus->kind = store->kind;
		if (store->kind == MB_DATA_DATA)
			{
			/* position */
			if (store->lon < -180.0)
				store->lon = store->lon + 360.0;
			if (store->lon > 180.0)
				store->lon = store->lon - 360.0;
			data->lon = (int)(0.5 + 10000000.0*store->lon);
			data->lat = (int)(0.5 + 10000000.0*store->lat);

			/* time stamp */
			time_i[0] = store->year;
			time_i[1] = store->month;
			time_i[2] = store->day;
			time_i[3] = store->hour;
			time_i[4] = store->minute;
			time_i[5] = store->second;
			time_i[6] = 0;
			mb_get_time(verbose,time_i,&time_d);
			data->seconds = (int) time_d;

			/* additional navigation and depths */
			data->heading = 10.0*store->course_true;
			data->course = 10.0*store->course_ground;
			data->speed = 10*store->speed;
			data->speed_ref = store->speed_reference[0];
			data->pitch = 10.0*store->pitch;
			data->scale = 100*store->depth_scale;
			id = MBSYS_HSDS_BEAMS - 1;
			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				{
				data->range[i] = store->distance[id-i];
				data->depth[i] = store->depth[id-i];
				}
			}

		/* comment */
		else if (store->kind == MB_DATA_COMMENT)
			{
			strcpy(datacomment,"zzzz");
			strncat(datacomment,store->comment,MBSYS_HSDS_MAXLINE);
			}
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (dataplus->kind == MB_DATA_DATA)
		{
		data->seconds = mb_swap_int(data->seconds);
		data->microseconds = mb_swap_int(data->microseconds);
		data->alt_seconds = mb_swap_int(data->alt_seconds);
		data->alt_microseconds = mb_swap_int(data->alt_microseconds);
		data->lat = mb_swap_int(data->lat);
		data->lon = mb_swap_int(data->lon);
		data->heading = mb_swap_short(data->heading);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->pitch = mb_swap_short(data->pitch);
		data->scale = mb_swap_short(data->scale);
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			data->depth[i] = mb_swap_short(data->depth[i]);
			data->range[i] = mb_swap_short(data->range[i]);
			}
		for (i=0;i<4;i++)
			data->flag[i] = mb_swap_int(data->flag[i]);

		}
#endif

	/* write next record to file */
	if (dataplus->kind == MB_DATA_DATA
		|| dataplus->kind == MB_DATA_COMMENT)
		{
		if ((status = fwrite(data,1,mb_io_ptr->data_structure_size,
			mb_io_ptr->mbfp)) 
			== mb_io_ptr->data_structure_size) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		}
	else
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		if (verbose >= 5)
			fprintf(stderr,"\ndbg5  No data written in MBIO function <%s>\n",function_name);
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
