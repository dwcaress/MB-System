/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sburivax.c	2/2/93
 *	$Id$
 *
 *    Copyright (c) 1993-2011 by
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
 * mbr_sburivax.c contains the functions for reading and writing
 * multibeam data in the SBURIVAX format. The only difference between
 * the SBURIVAX format and the SBURICEN format is that the data files
 * are stored in VAX byte order ("little endian"). This is why the
 * format structure definitions are taken from the include file 
 * mbf_sburicen.h.
 * These functions include:
 *   mbr_alm_sburivax	- allocate read/write memory
 *   mbr_dem_sburivax	- deallocate read/write memory
 *   mbr_rt_sburivax	- read and translate data
 *   mbr_wt_sburivax	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
 * $Log: mbr_sburivax.c,v $
 * Revision 5.8  2005/11/05 00:48:03  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.7  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.6  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.5  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
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
 * Revision 4.9  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.8  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.7  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.6  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.5  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.4  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.4  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.3  1995/03/22  18:58:09  caress
 * Added handling of extra 2 bytes which occurs every 5 records in
 * data derived from VAX VMS.
 *
 * Revision 4.2  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.1  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.0  1994/07/29  18:59:33  caress
 * Initial Revision.
 *
 * Revision 1.1  1994/07/29  18:46:51  caress
 * Initial revision
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
#include "../../include/mbsys_sb.h"
#include "../../include/mbf_sburicen.h"

/* include for byte swapping on little-endian machines */
#ifndef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* essential function prototypes */
int mbr_register_sburivax(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_sburivax(int verbose, 
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
int mbr_alm_sburivax(int verbose, void *mbio_ptr, int *error);
int mbr_dem_sburivax(int verbose, void *mbio_ptr, int *error);
int mbr_rt_sburivax(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_sburivax(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_sburivax(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_sburivax";
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
	status = mbr_info_sburivax(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_sburivax;
	mb_io_ptr->mb_io_format_free = &mbr_dem_sburivax; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_sb_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_sb_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_sburivax; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_sburivax; 
	mb_io_ptr->mb_io_dimensions = &mbsys_sb_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_sb_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_sb_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_sb_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_sb_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_sb_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL; 
	mb_io_ptr->mb_io_ttimes = &mbsys_sb_ttimes; 
	mb_io_ptr->mb_io_detects = &mbsys_sb_detects; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_sb_copy; 
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
int mbr_info_sburivax(int verbose, 
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
	char	*function_name = "mbr_info_sburivax";
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
	*system = MB_SYS_SB;
	*beams_bath_max = 19;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "SBURIVAX", MB_NAME_LENGTH);
	strncpy(system_name, "SB", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_SBURIVAX\nInformal Description: URI Sea Beam from VAX\nAttributes:           Sea Beam, bathymetry, 19 beams, binary, centered,\n                      VAX byte order, URI.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_NO;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
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
int mbr_alm_sburivax(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_sburivax";
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
	mb_io_ptr->structure_size = sizeof(struct mbf_sburicen_struct);
	mb_io_ptr->data_structure_size = 
		sizeof(struct mbf_sburicen_data_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_sb_struct),
				&mb_io_ptr->store_data,error);

	/* set record counters to zero */
	mb_io_ptr->irecord_count = 0;
	mb_io_ptr->orecord_count = 0;

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
int mbr_dem_sburivax(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_sburivax";
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
int mbr_rt_sburivax(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_sburivax";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sburicen_struct *dataplus;
	struct mbf_sburicen_data_struct *data;
	struct mbsys_sb_struct *store;
	char	*datacomment;
	short	dummy;
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
	dataplus = (struct mbf_sburicen_struct *) mb_io_ptr->raw_data;
	data = &(dataplus->data);
	datacomment = (char *) data;
	dataplus->kind = MB_DATA_DATA;
	store = (struct mbsys_sb_struct *) store_ptr;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next record from file */
	if ((status = fread(data,1,mb_io_ptr->data_structure_size,
			mb_io_ptr->mbfp)) == mb_io_ptr->data_structure_size) 
		{
		mb_io_ptr->file_bytes += status;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		mb_io_ptr->irecord_count++;
		}
	else
		{
		mb_io_ptr->file_bytes += status;
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* if needed read the extra two bytes that occur
		every five records */
	if (status == MB_SUCCESS && mb_io_ptr->irecord_count >= 5)
		{
		mb_io_ptr->irecord_count = 0;
		fread(&dummy, 1, sizeof(short), mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += sizeof(short);
		}

	/* byte swap the data if necessary */
#ifndef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		for (i=0;i<MBSYS_SB_BEAMS;i++)
			{
			data->dist[i] = mb_swap_short(data->dist[i]);
			data->deph[i] = mb_swap_short(data->deph[i]);
			}
		data->axis = mb_swap_short(data->axis);
		data->major = mb_swap_short(data->major);
		data->minor = mb_swap_short(data->minor);
		data->sbhdg = mb_swap_short(data->sbhdg);
		data->lat2b = mb_swap_short(data->lat2b);
		data->lat2u = mb_swap_short(data->lat2u);
		data->lon2b = mb_swap_short(data->lon2b);
		data->lon2u = mb_swap_short(data->lon2u);
		data->sec = mb_swap_short(data->sec);
		data->min = mb_swap_short(data->min);
		data->day = mb_swap_short(data->day);
		data->year = mb_swap_short(data->year);
		data->sbtim = mb_swap_short(data->sbtim);
		}
#endif

	/* check for comment or unintelligible records */
	if (status == MB_SUCCESS)
		{
		if (data->deph[0] > 15000)
			{
			dataplus->kind = MB_DATA_COMMENT;
			}
		else if (data->year == 0)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		else
			{
			dataplus->kind = MB_DATA_DATA;
			}
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = dataplus->kind;
	mb_io_ptr->new_error = *error;

	/* translate values to seabeam data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* type of data record */
		store->kind = dataplus->kind;

		/* position */
		store->lon2u = data->lon2u;
		store->lon2b = data->lon2b;
		store->lat2u = data->lat2u;
		store->lat2b = data->lat2b;

		/* time stamp */
		store->year = data->year;
		store->day = data->day;
		store->min = data->min;
		store->sec = data->sec;

		/* depths and distances */
		/* switch order of data as it is read into the global arrays */
		id = MBSYS_SB_BEAMS - 1;
		for (i=0;i<MBSYS_SB_BEAMS;i++)
			{
			store->dist[id-i] = data->dist[i];
			store->deph[id-i] = data->deph[i];
			}

		/* additional values */
		store->sbtim = data->sbtim;
		store->sbhdg = data->sbhdg;
		store->axis = data->axis;
		store->major = data->major;
		store->minor = data->minor;

		/* comment */
		strncpy(store->comment,&datacomment[2],
			MBSYS_SB_MAXLINE);
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
int mbr_wt_sburivax(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_sburivax";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sburicen_struct *dataplus;
	struct mbf_sburicen_data_struct *data;
	struct mbsys_sb_struct *store;
	char	*datacomment;
	short	dummy = 0;
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
	dataplus = (struct mbf_sburicen_struct *) mb_io_ptr->raw_data;
	data = &(dataplus->data);
	datacomment = (char *) data;
	store = (struct mbsys_sb_struct *) store_ptr;

	/* first set some plausible amounts for some of the 
		variables in the MBURICEN record */
	data->sbtim = 0;
	data->axis = 0;
	data->major = 0;
	data->minor = 0;

	/* second translate values from seabeam data storage structure */
	if (store != NULL)
		{
		dataplus->kind = store->kind;
		if (store->kind == MB_DATA_DATA)
			{
			/* position */
			data->lon2u = store->lon2u;
			data->lon2b = store->lon2b;
			data->lat2u = store->lat2u;
			data->lat2b = store->lat2b;

			/* time stamp */
			data->year = store->year;
			data->day = store->day;
			data->min = store->min;
			data->sec = store->sec;

			/* depths and distances */
			/* switch order of data as it is read 
				into the output arrays */
			id = MBSYS_SB_BEAMS - 1;
			for (i=0;i<MBSYS_SB_BEAMS;i++)
				{
				data->dist[i] = store->dist[id-i];
				data->deph[i] = store->deph[id-i];
				}

			/* additional values */
			data->sbtim = store->sbtim;
			data->sbhdg = store->sbhdg;
			data->axis = store->axis;
			data->major = store->major;
			data->minor = store->minor;
			}

		/* comment */
		else if (store->kind == MB_DATA_COMMENT)
			{
			strcpy(datacomment,"cc");
			strncat(datacomment,store->comment,MBSYS_SB_MAXLINE);
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Ready to write data in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",
			dataplus->kind);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		}

	/* byte swap the data if necessary */
#ifndef BYTESWAPPED
	if (dataplus->kind == MB_DATA_DATA
		|| dataplus->kind == MB_DATA_COMMENT)
		{
		for (i=0;i<MBSYS_SB_BEAMS;i++)
			{
			data->dist[i] = mb_swap_short(data->dist[i]);
			data->deph[i] = mb_swap_short(data->deph[i]);
			}
		data->axis = mb_swap_short(data->axis);
		data->major = mb_swap_short(data->major);
		data->minor = mb_swap_short(data->minor);
		data->sbhdg = mb_swap_short(data->sbhdg);
		data->lat2b = mb_swap_short(data->lat2b);
		data->lat2u = mb_swap_short(data->lat2u);
		data->lon2b = mb_swap_short(data->lon2b);
		data->lon2u = mb_swap_short(data->lon2u);
		data->sec = mb_swap_short(data->sec);
		data->min = mb_swap_short(data->min);
		data->day = mb_swap_short(data->day);
		data->year = mb_swap_short(data->year);
		data->sbtim = mb_swap_short(data->sbtim);
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
			mb_io_ptr->orecord_count++;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}

		/* if needed write the extra two bytes that occur
			every five records */
		if (status == MB_SUCCESS && mb_io_ptr->orecord_count >= 5)
			{
			mb_io_ptr->orecord_count = 0;
			fwrite(&dummy, 1, sizeof(short), mb_io_ptr->mbfp);
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
