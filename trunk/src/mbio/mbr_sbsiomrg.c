/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sbsiomrg.c	2/2/93
 *	$Id: mbr_sbsiomrg.c,v 5.6 2002-10-15 18:34:58 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2002 by
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
 * mbr_sbsiomrg.c contains the functions for reading and writing
 * multibeam data in the SBSIOMRG format.  
 * These functions include:
 *   mbr_alm_sbsiomrg	- allocate read/write memory
 *   mbr_dem_sbsiomrg	- deallocate read/write memory
 *   mbr_rt_sbsiomrg	- read and translate data
 *   mbr_wt_sbsiomrg	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
 * $Log: not supported by cvs2svn $
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
 * Revision 4.12  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.11  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.10  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.9  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.8  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.7  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.7  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.6  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.5  1995/02/14  15:10:38  caress
 * Fixed typo that set all acrosstrack distances to zero!!!
 *
 * Revision 4.4  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.3  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.2  1994/05/21  02:23:29  caress
 * Made sure that mb_io_ptr->new_bath_alongtrack is set to zero on reading.
 *
 * Revision 4.1  1994/05/11  21:23:01  caress
 * Added initialization of bathalongtrack array.
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
 * Revision 3.1  1993/07/03  02:28:20  caress
 * Fixed bug where ping with single nonzero beam which is off-center
 * caused a divide by zero.  Such a beam will not be passed on by
 * the reading subroutines because the center beam is unknown.
 *
 * Revision 3.0  1993/05/14  22:58:04  sohara
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
#include "../../include/mbsys_sb.h"
#include "../../include/mbf_sbsiomrg.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* essential function prototypes */
int mbr_register_sbsiomrg(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_sbsiomrg(int verbose, 
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
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error);
int mbr_alm_sbsiomrg(int verbose, void *mbio_ptr, int *error);
int mbr_dem_sbsiomrg(int verbose, void *mbio_ptr, int *error);
int mbr_rt_sbsiomrg(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_sbsiomrg(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char res_id[]="$Id: mbr_sbsiomrg.c,v 5.6 2002-10-15 18:34:58 caress Exp $";

/*--------------------------------------------------------------------*/
int mbr_register_sbsiomrg(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_sbsiomrg";
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
	status = mbr_info_sbsiomrg(verbose, 
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
			&mb_io_ptr->beamwidth_xtrack, 
			&mb_io_ptr->beamwidth_ltrack, 
			error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_sbsiomrg;
	mb_io_ptr->mb_io_format_free = &mbr_dem_sbsiomrg; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_sb_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_sb_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_sbsiomrg; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_sbsiomrg; 
	mb_io_ptr->mb_io_extract = &mbsys_sb_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_sb_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_sb_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_sb_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_sb_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL; 
	mb_io_ptr->mb_io_ttimes = &mbsys_sb_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_sb_copy; 
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
		fprintf(stderr,"dbg2       heading_source:     %d\n",mb_io_ptr->heading_source);
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
int mbr_info_sbsiomrg(int verbose, 
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
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error)
{
	char	*function_name = "mbr_info_sbsiomrg";
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
	*system = MB_SYS_SB;
	*beams_bath_max = 19;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "SBSIOMRG", MB_NAME_LENGTH);
	strncpy(system_name, "SB", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_SBSIOMRG\nInformal Description: SIO merge Sea Beam\nAttributes:           Sea Beam, bathymetry, 16 beams, binary, uncentered,\n                      SIO.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_NO;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*beamwidth_xtrack = 2.67;
	*beamwidth_ltrack = 2.67;

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
		fprintf(stderr,"dbg2       heading_source:     %d\n",*heading_source);
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
int mbr_alm_sbsiomrg(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_sbsiomrg";
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

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_sbsiomrg_struct);
	mb_io_ptr->data_structure_size = 
		sizeof(struct mbf_sbsiomrg_data_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_sb_struct),
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
int mbr_dem_sbsiomrg(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_sbsiomrg";
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
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mb_free(verbose,&mb_io_ptr->store_data,error);

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
int mbr_rt_sbsiomrg(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_sbsiomrg";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sbsiomrg_struct *dataplus;
	struct mbf_sbsiomrg_data_struct *data;
	struct mbsys_sb_struct *store;
	char	*datacomment;
	int	i, j, k, l;
	int	icenter;
	int	jpos, jneg;
	int	ipos, ineg;
	int	apos, aneg;
	int	id;

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

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	dataplus = (struct mbf_sbsiomrg_struct *) mb_io_ptr->raw_data;
	data = &(dataplus->data);
	datacomment = (char *) data;
	dataplus->kind = MB_DATA_DATA;
	store = (struct mbsys_sb_struct *) store_ptr;

	/* read next record from file */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
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

	/* check for comment or unintelligible records */
	if (status == MB_SUCCESS)
		{
		if (datacomment[0] == '#')
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

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS 
		&& dataplus->kind == MB_DATA_DATA)
		{
		data->year = mb_swap_short(data->year);
		data->day = mb_swap_short(data->day);
		data->min = mb_swap_short(data->min);
		data->sec = mb_swap_short(data->sec);
		data->lat2u = mb_swap_short(data->lat2u);
		data->lat2b = mb_swap_short(data->lat2b);
		data->lon2u = mb_swap_short(data->lon2u);
		data->lon2b = mb_swap_short(data->lon2b);
		for (i=0;i<3;i++)
			data->spare1[i] = mb_swap_short(data->spare1[i]);
		data->sbtim = mb_swap_short(data->sbtim);
		data->sbhdg = mb_swap_short(data->sbhdg);
		for (i=0;i<MB_BEAMS_RAW_SBSIOMRG;i++)
			{
			data->dist[i] = mb_swap_short(data->dist[i]);
			data->deph[i] = mb_swap_short(data->deph[i]);
			}
		for (i=0;i<5;i++)
			data->spare2[i] = mb_swap_short(data->spare2[i]);
		}
#endif

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = dataplus->kind;
	mb_io_ptr->new_error = *error;

	/* print debug statements */
	if (verbose >= 5 && status == MB_SUCCESS 
		&& dataplus->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  New ping read by MBIO function <%s>\n",
				function_name);
		fprintf(stderr,"dbg5  Raw ping values:\n");
		fprintf(stderr,"dbg5       year:       %d\n",data->year);
		fprintf(stderr,"dbg5       day:        %d\n",data->day);
		fprintf(stderr,"dbg5       min:        %d\n",data->min);
		fprintf(stderr,"dbg5       sec:        %d\n",data->sec);
		fprintf(stderr,"dbg5       lon2u:      %d\n",data->lon2u);
		fprintf(stderr,"dbg5       lon2b:      %d\n",data->lon2b);
		fprintf(stderr,"dbg5       lat2u:      %d\n",data->lat2u);
		fprintf(stderr,"dbg5       lat2b:      %d\n",data->lat2b);
		fprintf(stderr,"dbg5       spare1[0]:  %d\n",data->spare1[0]);
		fprintf(stderr,"dbg5       spare1[1]:  %d\n",data->spare1[1]);
		fprintf(stderr,"dbg5       spare1[2]:  %d\n",data->spare1[2]);
		fprintf(stderr,"dbg5       sbtim:      %d\n",data->sbtim);
		fprintf(stderr,"dbg5       sbhdg:      %d\n",data->sbhdg);
		fprintf(stderr,"dbg5       spare2[0]:  %d\n",data->spare2[0]);
		fprintf(stderr,"dbg5       spare2[1]:  %d\n",data->spare2[1]);
		fprintf(stderr,"dbg5       spare2[2]:  %d\n",data->spare2[2]);
		fprintf(stderr,"dbg5       spare2[3]:  %d\n",data->spare2[3]);
		fprintf(stderr,"dbg5       spare2[4]:  %d\n",data->spare2[4]);
		for (i=0;i<MB_BEAMS_RAW_SBSIOMRG;i++)
			fprintf(stderr,"dbg5       deph[%d]: %d  dist[%d]: %d\n",
				i,data->deph[i],
				i,data->dist[i]);
		}

	/* translate values to seabeam data storage structure */
	if (status == MB_SUCCESS)
		{
		/* type of data record */
		store->kind = dataplus->kind;
		
		if (store->kind == MB_DATA_DATA)
			{
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

			/* zero arrays */
			for (i=0;i<MBSYS_SB_BEAMS;i++)
				{
				store->deph[i] = 0;
				store->dist[i] = 0;
				}

			/* find center beam */
			icenter = -1;
			for (i=0;i<MB_BEAMS_RAW_SBSIOMRG;i++)
				{
				if (data->dist[i] == 0 
					&& data->deph[i] != 0) 
					icenter = i;
				if (icenter < 0 && data->dist[i] == 0 
					&& data->dist[i-1] < 0 
					&& data->dist[i+1] > 0) 
					icenter = i;
				}

			/* get center beam from closest distances if still needed */
			if (icenter < 0)
				{
				jpos = 0;
				jneg = 0;
				for (i=0;i<MB_BEAMS_RAW_SBSIOMRG;i++)
					{
					if (data->dist[i] > 0
					&& (data->dist[i] < jpos 
						|| jpos == 0))
						{
						jpos = data->dist[i];
						ipos = i;
						}
					if (data->dist[i] < 0 
						&& (data->dist[i] > jneg 
						|| jneg == 0))
						{
						jneg = data->dist[i];
						ineg = i;
						}
					}
				if (jpos > 0 && jneg < 0)
					{
					apos = jpos;
					aneg = jneg;
					icenter = ineg + (int)((ipos-ineg)*
						((0 - aneg)/(apos-aneg)) + 0.5);
					}
				if (icenter < 0 || icenter >= MB_BEAMS_RAW_SBSIOMRG)
					icenter = -1;
				}

			/* get center beam from any distances if still needed */
			if (icenter < 0)
				{
				jneg = 0;
				jpos = 0;
				for (i=0;i<MB_BEAMS_RAW_SBSIOMRG;i++)
					{
					if (data->dist[i] != 0)
						{
						if (jneg == 0 && jpos == 0)
							{
							jneg = data->dist[i];
							ineg = i;
							jpos = data->dist[i];
							ipos = i;
							}
						else if (data->dist[i] < jneg)
							{
							jneg = data->dist[i];
							ineg = i;
							}
						else if (data->dist[i] > jpos)
							{
							jpos = data->dist[i];
							ipos = i;
							}
						}
					}
				if (jpos != 0 && jneg != 0 && jpos != jneg)
					{
					apos = jpos;
					aneg = jneg;
					icenter = ineg + (int)((ipos-ineg)*
						((0 - aneg)/(apos-aneg)) + 0.5);
					}
				if (icenter < 0 || icenter >= MB_BEAMS_RAW_SBSIOMRG)
					icenter = -1;
				}

			/* center the data in the global arrays */
			if (icenter >= 0)
				{
				id = MB_BEAMS_PROC_SBSIOMRG/2 - icenter;
				j = 0;
				k = MB_BEAMS_RAW_SBSIOMRG;
				if (id < 0) j = -id;
				if (id > (MB_BEAMS_PROC_SBSIOMRG 
					- MB_BEAMS_RAW_SBSIOMRG)) 
					k = MB_BEAMS_PROC_SBSIOMRG - id;
				for (i=j;i<k;i++)
					{
					l = MBSYS_SB_BEAMS - 1 - id - i;
					store->deph[l] = data->deph[i];
					store->dist[l] = data->dist[i];
					}
				}

			/* additional values */
			store->sbtim = data->sbtim;
			store->sbhdg = data->sbhdg;
			store->axis = 0;
			store->major = 0;
			store->minor = 0;
			}

		else if (store->kind == MB_DATA_COMMENT)
			{
			/* comment */
			strncpy(store->comment,&datacomment[2],
				MBSYS_SB_MAXLINE);
			}
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
int mbr_wt_sbsiomrg(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_sbsiomrg";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sbsiomrg_struct *dataplus;
	struct mbf_sbsiomrg_data_struct *data;
	struct mbsys_sb_struct *store;
	char	*datacomment;
	int	i, j;
	int	offset, iend, id;

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

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	dataplus = (struct mbf_sbsiomrg_struct *) mb_io_ptr->raw_data;
	data = &(dataplus->data);
	datacomment = (char *) data;
	store = (struct mbsys_sb_struct *) store_ptr;

	/* translate values from seabeam data storage structure */
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

		/* put distance and depth values 
			into sbsiomrg data structure */

		/* initialize depth and distance in 
			output structure */
		for (i=0;i<MB_BEAMS_RAW_SBSIOMRG;i++)
			{
			data->deph[i] = 0;
			data->dist[i] = 0;
			}

		/* find first nonzero beam */
		id = MB_BEAMS_PROC_SBSIOMRG - 1;
		offset = -1;
		for (i=0;i<MB_BEAMS_PROC_SBSIOMRG;i++)
			if (store->deph[id-i] != 0 
				&& offset == -1) 
				offset = i;
		if (offset == -1) offset = 0;
		iend = MB_BEAMS_RAW_SBSIOMRG;
		if (iend + offset > MB_BEAMS_PROC_SBSIOMRG) 
			iend = MB_BEAMS_PROC_SBSIOMRG - offset;

		/* read depth and distance values into 
			output structure */
		for (i=0;i<iend;i++)
			{
			j = id - i - offset;
			data->deph[i] = store->deph[j];
			data->dist[i] = store->dist[j];
			}

		/* additional values */
		data->sbtim = store->sbtim;
		data->sbhdg = store->sbhdg;
		}

	/* comment */
	else if (store->kind == MB_DATA_COMMENT)
		{
		strcpy(datacomment,"##");
		strncat(datacomment,store->comment,
			mb_io_ptr->data_structure_size-3);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Ready to write data in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",
			dataplus->kind);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (dataplus->kind == MB_DATA_DATA)
		{
		data->year = mb_swap_short(data->year);
		data->day = mb_swap_short(data->day);
		data->min = mb_swap_short(data->min);
		data->sec = mb_swap_short(data->sec);
		data->lat2u = mb_swap_short(data->lat2u);
		data->lat2b = mb_swap_short(data->lat2b);
		data->lon2u = mb_swap_short(data->lon2u);
		data->lon2b = mb_swap_short(data->lon2b);
		for (i=0;i<3;i++)
			data->spare1[i] = mb_swap_short(data->spare1[i]);
		data->sbtim = mb_swap_short(data->sbtim);
		data->sbhdg = mb_swap_short(data->sbhdg);
		for (i=0;i<MB_BEAMS_RAW_SBSIOMRG;i++)
			{
			data->dist[i] = mb_swap_short(data->dist[i]);
			data->deph[i] = mb_swap_short(data->deph[i]);
			}
		for (i=0;i<5;i++)
			data->spare2[i] = mb_swap_short(data->spare2[i]);
		}
#endif

	/* write next record to file */
	if (dataplus->kind == MB_DATA_DATA
		|| dataplus->kind == MB_DATA_COMMENT)
		{
		if ((status = fwrite(data,1,mb_io_ptr->data_structure_size,
			mb_io_ptr->mbfp)) == mb_io_ptr->data_structure_size) 
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
			fprintf(stderr,"\ndbg5  No data written in MBIO function <%s>\n",
				function_name);
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
