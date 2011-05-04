/*--------------------------------------------------------------------
 *    The MB-system:	mbr_em12darw.c	2/2/93
 *	$Id$
 *
 *    Copyright (c) 1994-2011 by
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
 * mbr_em12darw.c contains the functions for reading and writing
 * multibeam data in the EM12DARW format.  
 * These functions include:
 *   mbr_alm_em12darw	- allocate read/write memory
 *   mbr_dem_em12darw	- deallocate read/write memory
 *   mbr_rt_em12darw	- read and translate data
 *   mbr_wt_em12darw	- translate and write data
 *
 * Author:	R. B. Owens
 * Date:	January 24, 1994
 * $Log: mbr_em12darw.c,v $
 * Revision 5.12  2008/03/01 09:14:02  caress
 * Some housekeeping changes.
 *
 * Revision 5.11  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.10  2003/12/04 23:10:22  caress
 * Fixed problems with format 54 EM12DARW due to old code assuming how internal structure was packed. Also changed handling of beamflags for formats that don't support beamflags. Now flagged beams will always be nulled in such cases.
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
 * Revision 5.5  2002/02/26 07:50:41  caress
 * Release 5.0.beta14
 *
 * Revision 5.4  2001/07/20 00:31:11  caress
 * Release 5.0.beta03
 *
 * Revision 5.3  2001/03/22  20:45:56  caress
 * Trying to make 5.0.beta0...
 *
 * Revision 5.2  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.1  2000/12/10  20:26:50  caress
 * Version 5.0.alpha02
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.14  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.13  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.12  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
 *
 * Revision 4.11  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.10  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.9  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.8  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.7  1996/08/26  20:05:02  caress
 * Changed "signed char" to "char".
 *
 * Revision 4.7  1996/08/26  20:05:02  caress
 * Changed "signed char" to "char".
 *
 * Revision 4.6  1996/08/05  15:21:58  caress
 * Just redid i/o for Simrad sonars, including adding EM12S and EM121 support.
 *
 * Revision 4.5  1996/07/26  21:09:33  caress
 * Version after first cut of handling em12s and em121 data.
 *
 * Revision 4.4  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.3  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.1  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/05  22:54:09  caress
 * First cut.
 *
 * Revision 3.0  1993/05/14  22:56:29  sohara
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
#include "../../include/mbf_em12darw.h"
#include "../../include/mbsys_simrad.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* essential function prototypes */
int mbr_register_em12darw(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_em12darw(int verbose, 
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
int mbr_alm_em12darw(int verbose, void *mbio_ptr, int *error);
int mbr_dem_em12darw(int verbose, void *mbio_ptr, int *error);
int mbr_zero_em12darw(int verbose, char *data_ptr, int *error);
int mbr_rt_em12darw(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_em12darw(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_em12darw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_em12darw";
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
	status = mbr_info_em12darw(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_em12darw;
	mb_io_ptr->mb_io_format_free = &mbr_dem_em12darw; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_simrad_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_simrad_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_em12darw; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_em12darw; 
	mb_io_ptr->mb_io_dimensions = &mbsys_simrad_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_simrad_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_simrad_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_simrad_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_simrad_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_simrad_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL; 
	mb_io_ptr->mb_io_ttimes = &mbsys_simrad_ttimes; 
	mb_io_ptr->mb_io_detects = &mbsys_simrad_detects; 
	mb_io_ptr->mb_io_gains = &mbsys_simrad_gains; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_simrad_copy; 
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
int mbr_info_em12darw(int verbose, 
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
	char	*function_name = "mbr_info_em12darw";
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
	*system = MB_SYS_SIMRAD;
	*beams_bath_max = MBF_EM12DARW_BEAMS;
	*beams_amp_max = MBF_EM12DARW_BEAMS;
	*pixels_ss_max = 0;
	strncpy(format_name, "EM12DARW", MB_NAME_LENGTH);
	strncpy(system_name, "SIMRAD", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_EM12DARW\nInformal Description: Simrad EM12S RRS Darwin processed format\nAttributes:           Simrad EM12S, bathymetry and amplitude,\n                      81 beams, binary, Oxford University.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_YES;
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
int mbr_alm_em12darw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_em12darw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em12darw_struct *data;
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
	mb_io_ptr->structure_size = sizeof(struct mbf_em12darw_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mbsys_simrad_alloc(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_em12darw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;

	/* initialize everything to zeros */
	mbr_zero_em12darw(verbose,data_ptr,error);

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
int mbr_dem_em12darw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_em12darw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;

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
	store = (struct mbsys_simrad_struct *) mb_io_ptr->store_data;

	/* deallocate memory for data descriptor */
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mbsys_simrad_deall(
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
int mbr_zero_em12darw(int verbose, char *data_ptr, int *error)
{
	char	*function_name = "mbr_zero_em12darw";
	int	status = MB_SUCCESS;
	struct mbf_em12darw_struct *data;
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
	data = (struct mbf_em12darw_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		/* record type */
		data->func = 150;

		/* time */
		data->year = 0;
		data->jday = 0;
		data->minute = 0;
		data->secs = 0;

		/* navigation */
		data->latitude = 0.0;
		data->longitude = 0.0;
		data->speed = 0.0;
		data->gyro = 0.0;
		data->roll = 0.0;
		data->pitch = 0.0;
		data->heave = 0.0;

		/* other parameters */
		data->corflag = 0;
		data->utm_merd = 0.0;
		data->utm_zone = 0;
		data->posq = 0;
		data->pingno = 0;
		data->mode = 0;
		data->depthl = 0.0;
		data->sndval = 0.0;

		/* beam values */
		for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			{
			data->depth[i] = 0;
			data->distacr[i] = 0;
			data->distalo[i] = 0;
			data->range[i] = 0;
			data->refl[i] = 0;
			data->beamq[i] = 0;
			}
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
int mbr_rt_em12darw(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_em12darw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em12darw_struct *data;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	char	line[MBF_EM12DARW_RECORD_LENGTH];
	int	index;
	char	*datacomment;
	int	time_j[5];
	int	time_i[7];
	int	kind;
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
	data = (struct mbf_em12darw_struct *) mb_io_ptr->raw_data;
	datacomment = (char *) &line[80];
	store = (struct mbsys_simrad_struct *) store_ptr;	

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next record from file */
	if ((status = fread(line,1,MBF_EM12DARW_RECORD_LENGTH,
			mb_io_ptr->mbfp)) == MBF_EM12DARW_RECORD_LENGTH) 
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

	/* get data type */
	if (status == MB_SUCCESS)
		{
		mb_get_binary_short(MB_NO, &line[0], &(data->func)); 
		}
		
	/* deal with comment */
	if (status == MB_SUCCESS && data->func == 100)
		{
		kind = MB_DATA_COMMENT;

		strncpy(store->comment,datacomment,
			MBSYS_SIMRAD_COMMENT_LENGTH);
		}
		
	/* deal with data */
	else if (status == MB_SUCCESS && data->func == 150)
		{
		kind = MB_DATA_DATA;

		index = 2;
		mb_get_binary_short(MB_NO, &line[index], &(data->year)); index += 2;
		mb_get_binary_short(MB_NO, &line[index], &(data->jday)); index += 2; 
		mb_get_binary_short(MB_NO, &line[index], &(data->minute)); index += 2; 
		mb_get_binary_short(MB_NO, &line[index], &(data->secs)); index += 8; 
		mb_get_binary_double(MB_NO, &line[index], &(data->latitude)); index += 8; 
		mb_get_binary_double(MB_NO, &line[index], &(data->longitude)); index += 8; 
		mb_get_binary_short(MB_NO, &line[index], &(data->corflag)); index += 4; 
		mb_get_binary_float(MB_NO, &line[index], &(data->utm_merd)); index += 4; 
		mb_get_binary_short(MB_NO, &line[index], &(data->utm_zone)); index += 2; 
		mb_get_binary_short(MB_NO, &line[index], &(data->posq)); index += 2; 
		mb_get_binary_int(MB_NO, &line[index], &(data->pingno)); index += 4; 
		mb_get_binary_short(MB_NO, &line[index], &(data->mode)); index += 4; 
		mb_get_binary_float(MB_NO, &line[index], &(data->depthl)); index += 4; 
		mb_get_binary_float(MB_NO, &line[index], &(data->speed)); index += 4; 
		mb_get_binary_float(MB_NO, &line[index], &(data->gyro)); index += 4; 
		mb_get_binary_float(MB_NO, &line[index], &(data->roll)); index += 4; 
		mb_get_binary_float(MB_NO, &line[index], &(data->pitch)); index += 4; 
		mb_get_binary_float(MB_NO, &line[index], &(data->heave)); index += 4; 
		mb_get_binary_float(MB_NO, &line[index], &(data->sndval)); index += 4; 
		for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			{
			mb_get_binary_short(MB_NO, &line[index], &(data->depth[i])); index += 2; 
			}
		for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			{
			mb_get_binary_short(MB_NO, &line[index], &(data->distacr[i])); index += 2; 
			}
		for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			{
			mb_get_binary_short(MB_NO, &line[index], &(data->distalo[i])); index += 2; 
			}
		for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			{
			mb_get_binary_short(MB_NO, &line[index], &(data->range[i])); index += 2; 
			}
		for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			{
			mb_get_binary_short(MB_NO, &line[index], &(data->refl[i])); index += 2; 
			}
		for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			{
			mb_get_binary_short(MB_NO, &line[index], &(data->beamq[i])); index += 2; 
			}
			

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Data read by MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg4  Read values:\n");
			fprintf(stderr,"dbg4       kind:       %d\n", kind);
			fprintf(stderr,"dbg4       error:      %d\n", *error);
			fprintf(stderr,"dbg4       year:       %d\n", data->year);
			fprintf(stderr,"dbg4       jday:       %d\n", data->jday);
			fprintf(stderr,"dbg4       minute:     %d\n", data->minute);
			fprintf(stderr,"dbg4       secs:       %d\n", data->secs);
			fprintf(stderr,"dbg4       latitude:   %f\n", data->latitude);
			fprintf(stderr,"dbg4       longitude:  %f\n", data->longitude);
			fprintf(stderr,"dbg4       corflag:    %d\n", data->corflag);
			fprintf(stderr,"dbg4       utm_merd:   %f\n", data->utm_merd);
			fprintf(stderr,"dbg4       utm_zone:   %d\n", data->utm_zone);
			fprintf(stderr,"dbg4       posq:       %d\n", data->posq);
			fprintf(stderr,"dbg4       pingno:     %d\n", data->pingno);
			fprintf(stderr,"dbg4       mode:       %d\n", data->mode);
			fprintf(stderr,"dbg4       depthl:     %f\n", data->depthl);
			fprintf(stderr,"dbg4       speed:      %f\n", data->speed);
			fprintf(stderr,"dbg4       gyro:       %f\n", data->gyro);
			fprintf(stderr,"dbg4       roll:       %f\n", data->roll);
			fprintf(stderr,"dbg4       pitch:      %f\n", data->pitch);
			fprintf(stderr,"dbg4       heave:      %f\n", data->heave);
			fprintf(stderr,"dbg4       sndval:     %f\n", data->sndval);
			for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			  fprintf(stderr,"dbg4       beam:%d  depth:%d  distacr:%d  distalo:%d  range:%d refl:%d beamq:%d\n",
				i,data->depth[i],data->distacr[i],
				data->distalo[i],data->range[i],
				data->refl[i],data->beamq[i]);
			}
		
		}

	/* else unintelligible */
	else if (status == MB_SUCCESS)
		{
		kind = MB_DATA_NONE;
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = kind;
	mb_io_ptr->new_error = *error;

	/* translate values to em12 data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* type of data record */
		store->kind = kind;
		store->sonar = MBSYS_SIMRAD_EM12S;

		/* time */
		mb_fix_y2k(verbose, (int)data->year, 
				&time_j[0]);
		time_j[1] = data->jday;
		time_j[2] = data->minute;
		time_j[3] = data->secs/100;
		time_j[4] = 0.0001*(100*time_j[3] - data->secs);
		mb_get_itime(verbose,time_j,time_i);
		store->year = data->year;
		store->month = time_i[1];
		store->day = time_i[2];
		store->hour = time_i[3];
		store->minute = time_i[4];
		store->second = time_i[5];
		store->centisecond = 0.0001*time_i[6];
		store->pos_year = store->year;
		store->pos_month = store->month;
		store->pos_day = store->day;
		store->pos_hour = store->hour;
		store->pos_minute = store->minute;
		store->pos_second = store->second;
		store->pos_centisecond = store->centisecond;

		/* navigation */
		if (data->corflag == 0)
			{
			store->pos_latitude = data->latitude;
			store->pos_longitude = data->longitude;
			store->utm_northing = 0.0;
			store->utm_easting = 0.0;
			}
		else
			{
			store->pos_latitude = 0.0;
			store->pos_longitude = 0.0;
			store->utm_northing = data->latitude;
			store->utm_easting = data->longitude;
			}
		store->utm_zone = data->utm_zone;
		store->utm_zone_lon = data->utm_merd;
		store->utm_system = data->corflag;
		store->pos_quality = data->posq;
		store->speed = data->speed;
		store->line_heading = 10*data->gyro;
		
		/* allocate secondary data structure for
			survey data if needed */
		if (kind == MB_DATA_DATA
			&& store->ping == NULL)
			{
			status = mbsys_simrad_survey_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}
		
		/* deal with putting survey data into
		secondary data structure */
		if (status == MB_SUCCESS 
			&& kind == MB_DATA_DATA)
			{
			/* get data structure pointer */
			ping = (struct mbsys_simrad_survey_struct *) 
				store->ping;

			/* copy data */
			ping->longitude = data->longitude;
			ping->latitude = data->latitude;
			ping->swath_id = EM_SWATH_CENTER;
			ping->ping_number = data->pingno;
			ping->beams_bath = MBF_EM12DARW_BEAMS;
			ping->bath_mode = 0;
			ping->bath_res = data->mode;
			ping->bath_quality = 0;
			ping->keel_depth = data->depthl;
			ping->heading = (int) 10*data->gyro;
			ping->roll = (int) 100*data->roll;
			ping->pitch = (int) 100*data->pitch;
			ping->xducer_pitch = (int) 100*data->pitch;
			ping->ping_heave = (int) 100*data->heave;
			ping->sound_vel = (int) 10*data->sndval;
			ping->pixels_ss = 0;
			ping->ss_mode = 0;
			for (i=0;i<ping->beams_bath;i++)
				{
				if (data->depth[i] > 0)
					{
					ping->bath[i] = data->depth[i];
					ping->beamflag[i] = MB_FLAG_NONE;
					}
				else if (data->depth[i] < 0)
					{
					ping->bath[i] = -data->depth[i];
					ping->beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
					}
				else
					{
					ping->bath[i] = data->depth[i];
					ping->beamflag[i] = MB_FLAG_NULL;
					}
				ping->bath_acrosstrack[i] = data->distacr[i];
				ping->bath_alongtrack[i] = data->distalo[i];
				ping->tt[i] = data->range[i];
				ping->amp[i] = (mb_s_char) data->refl[i];
				ping->quality[i] = (mb_u_char) data->beamq[i];
				ping->heave[i] = (mb_s_char) 0;
				ping->beam_frequency[i] = 0;
				ping->beam_samples[i] = 0;
				ping->beam_center_sample[i] = 0;
				}
			}

		else if (status == MB_SUCCESS 
			&& kind == MB_DATA_COMMENT)
			{
			/* comment */
			strncpy(store->comment,datacomment,
				MBSYS_SIMRAD_COMMENT_LENGTH);
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
int mbr_wt_em12darw(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_em12darw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em12darw_struct *data;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	char	line[MBF_EM12DARW_RECORD_LENGTH];
	int	index;
	char	*datacomment;
	int	time_i[7];
	int	time_j[5];
	int	year;
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
	data = (struct mbf_em12darw_struct *) mb_io_ptr->raw_data;
	datacomment = (char *) &line[80];
	store = (struct mbsys_simrad_struct *) store_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Status at beginning of MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       store->kind:    %d\n",store->kind);
		fprintf(stderr,"dbg5       error:          %d\n",*error);
		fprintf(stderr,"dbg5       status:         %d\n",status);
		}


	/*  translate values from em12 data storage structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* record type */
		data->func = 150;

		/* time */
		time_i[0] = store->year;
		time_i[1] = store->month;
		time_i[2] = store->day;
		time_i[3] = store->hour;
		time_i[4] = store->minute;
		time_i[5] = store->second;
		time_i[6] = store->centisecond;
		mb_get_jtime(verbose,time_i,time_j);
		mb_unfix_y2k(verbose, time_j[0], &year);
		data->year = (short)year;
		data->jday = time_j[1];
		data->minute = time_j[2];
		data->secs = 100*time_j[3] + 0.0001*time_j[4];

		/* navigation */
		data->utm_zone = store->utm_zone;
		data->utm_merd = store->utm_zone_lon;
		data->corflag = store->utm_system;
		data->posq = store->pos_quality;
		data->speed = store->speed;
		if (data->corflag == 0)
			{
			data->latitude = store->pos_latitude;
			data->longitude = store->pos_longitude;
			}
		else
			{
			data->latitude = store->utm_northing;
			data->longitude = store->utm_easting;
			}


		/* deal with survey data 
			in secondary data structure */
		if (store->ping != NULL)
			{
			/* get data structure pointer */
			ping = (struct mbsys_simrad_survey_struct *) 
				store->ping;

			/* copy survey data */
			data->latitude = ping->latitude;
			data->longitude = ping->longitude;
			data->pingno = ping->ping_number;
			data->mode = ping->bath_res;
			data->depthl = ping->keel_depth;
			data->gyro = 0.1*ping->heading;
			data->roll = 0.01*ping->roll;
			data->pitch = 0.01*ping->pitch;
			data->heave = 0.01*ping->ping_heave;
			data->sndval = 0.1*ping->sound_vel;
			for (i=0;i<ping->beams_bath;i++)
				{
				if (ping->beamflag[i] == MB_FLAG_NULL)
					data->depth[i] = 0;
				else if (!mb_beam_ok(ping->beamflag[i]))
					data->depth[i] = -ping->bath[i];
				else
					data->depth[i] = ping->bath[i];
				data->distacr[i] 
					= ping->bath_acrosstrack[i];
				data->distalo[i] 
					= ping->bath_alongtrack[i];
				data->range[i] 
					= ping->tt[i];
				data->refl[i] 
					= (short int) ping->amp[i];
				data->beamq[i] 
					= (short int) ping->quality[i];
				}
			}
		}

	/* comment */
	else if (store->kind == MB_DATA_COMMENT)
		{
		data->func = 100;
		strncpy(datacomment,store->comment,
			MBSYS_SIMRAD_COMMENT_LENGTH);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Ready to write data in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       store->kind:       %d\n",store->kind);
		fprintf(stderr,"dbg5       error:             %d\n",*error);
		fprintf(stderr,"dbg5       status:            %d\n",status);
		}

	/* print debug statements */
	if (verbose >= 4 && status == MB_SUCCESS)
		{
		fprintf(stderr,"\ndbg4  Data to be written by MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg4  Status values:\n");
		fprintf(stderr,"dbg4       store->kind:%d\n", store->kind);
		fprintf(stderr,"dbg4       error:      %d\n", *error);
		fprintf(stderr,"dbg4       status:     %d\n",status);
		if (store->kind == MB_DATA_DATA)
			{
			fprintf(stderr,"dbg4  Survey values:\n");
			fprintf(stderr,"dbg4       year:       %d\n", data->year);
			fprintf(stderr,"dbg4       jday:       %d\n", data->jday);
			fprintf(stderr,"dbg4       minute:     %d\n", data->minute);
			fprintf(stderr,"dbg4       secs:       %d\n", data->secs);
			fprintf(stderr,"dbg4       latitude:   %f\n", data->latitude);
			fprintf(stderr,"dbg4       longitude:  %f\n", data->longitude);
			fprintf(stderr,"dbg4       corflag:    %d\n", data->corflag);
			fprintf(stderr,"dbg4       utm_merd:   %f\n", data->utm_merd);
			fprintf(stderr,"dbg4       utm_zone:   %d\n", data->utm_zone);
			fprintf(stderr,"dbg4       posq:       %d\n", data->posq);
			fprintf(stderr,"dbg4       pingno:     %d\n", data->pingno);
			fprintf(stderr,"dbg4       mode:       %d\n", data->mode);
			fprintf(stderr,"dbg4       depthl:     %f\n", data->depthl);
			fprintf(stderr,"dbg4       speed:      %f\n", data->speed);
			fprintf(stderr,"dbg4       gyro:       %f\n", data->gyro);
			fprintf(stderr,"dbg4       roll:       %f\n", data->roll);
			fprintf(stderr,"dbg4       pitch:      %f\n", data->pitch);
			fprintf(stderr,"dbg4       heave:      %f\n", data->heave);
			fprintf(stderr,"dbg4       sndval:     %f\n", data->sndval);
			for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			  fprintf(stderr,"dbg4       beam:%d  depth:%d  distacr:%d  distalo:%d  range:%d refl:%d beamq:%d\n",
				i,data->depth[i],data->distacr[i],
				data->distalo[i],data->range[i],
				data->refl[i],data->beamq[i]);
			}
		else if (store->kind == MB_DATA_COMMENT)
			{
			fprintf(stderr,"dbg4  Comment:\n");
			fprintf(stderr,"dbg4       comment:    %s\n", datacomment);
			}
		}
		
	/* deal with comment */
	if (status == MB_SUCCESS && store->kind == MB_DATA_COMMENT)
		{
		index = 0;
		for (i=0;i<MBF_EM12DARW_RECORD_LENGTH;i++)
			line[i] = 0;
		mb_put_binary_short(MB_NO, data->func, &line[0]); index+= 2;
		strncpy(datacomment, store->comment,
			MBSYS_SIMRAD_COMMENT_LENGTH);
		}
		
	/* deal with data */
	else if (status == MB_SUCCESS && store->kind == MB_DATA_DATA)
		{
		index = 0;
		mb_put_binary_short(MB_NO, data->func, &line[0]); index+= 2;
		mb_put_binary_short(MB_NO, data->year, &line[index]); index+= 2;
		mb_put_binary_short(MB_NO, data->jday, &line[index]); index+= 2;
		mb_put_binary_short(MB_NO, data->minute, &line[index]); index+= 2;
		mb_put_binary_short(MB_NO, data->secs, &line[index]); index+= 8;
		mb_put_binary_double(MB_NO, data->latitude, &line[index]); index+= 8;
		mb_put_binary_double(MB_NO, data->longitude, &line[index]); index+= 8;
		mb_put_binary_short(MB_NO, data->corflag, &line[index]); index+= 4;
		mb_put_binary_float(MB_NO, data->utm_merd, &line[index]); index+= 4;
		mb_put_binary_short(MB_NO, data->utm_zone, &line[index]); index+= 2;
		mb_put_binary_short(MB_NO, data->posq, &line[index]); index+= 2;
		mb_put_binary_int(MB_NO, data->pingno, &line[index]); index+= 4;
		mb_put_binary_short(MB_NO, data->mode, &line[index]); index+= 4;
		mb_put_binary_float(MB_NO, data->depthl, &line[index]); index+= 4;
		mb_put_binary_float(MB_NO, data->speed, &line[index]); index+= 4;
		mb_put_binary_float(MB_NO, data->gyro, &line[index]); index+= 4;
		mb_put_binary_float(MB_NO, data->roll, &line[index]); index+= 4;
		mb_put_binary_float(MB_NO, data->pitch, &line[index]); index+= 4;
		mb_put_binary_float(MB_NO, data->heave, &line[index]); index+= 4;
		mb_put_binary_float(MB_NO, data->sndval, &line[index]); index+= 4;
		for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			{
			mb_put_binary_short(MB_NO, data->depth[i], &line[index]); index+= 2;
			}
		for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			{
			mb_put_binary_short(MB_NO, data->distacr[i], &line[index]); index+= 2;
			}
		for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			{
			mb_put_binary_short(MB_NO, data->distalo[i], &line[index]); index+= 2;
			}
		for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			{
			mb_put_binary_short(MB_NO, data->range[i], &line[index]); index+= 2;
			}
		for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			{
			mb_put_binary_short(MB_NO, data->refl[i], &line[index]); index+= 2;
			}
		for (i=0;i<MBF_EM12DARW_BEAMS;i++)
			{
			mb_put_binary_short(MB_NO, data->beamq[i], &line[index]); index+= 2;
			}		
		}

	/* write next record to file */
	if (store->kind == MB_DATA_DATA || store->kind == MB_DATA_COMMENT)
		{
		if ((status = fwrite(line,1,MBF_EM12DARW_RECORD_LENGTH,
				mb_io_ptr->mbfp)) 
				== MBF_EM12DARW_RECORD_LENGTH) 
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
