/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sbsioswb.c	9/18/93
 *	$Id: mbr_sbsioswb.c,v 5.5 2002-02-26 07:50:41 caress Exp $
 *
 *    Copyright (c) 1994, 2000 by
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
 * mbr_sbsioswb.c contains the functions for reading and writing
 * multibeam data in the SBSIOSWB format.  
 * These functions include:
 *   mbr_alm_sbsioswb	- allocate read/write memory
 *   mbr_dem_sbsioswb	- deallocate read/write memory
 *   mbr_rt_sbsioswb	- read and translate data
 *   mbr_wt_sbsioswb	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
 * $Log: not supported by cvs2svn $
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
 * Revision 4.12  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.11  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.10  1999/08/06  00:47:44  caress
 * Added code to handle case where fewer beams than listed are actually
 * in the data.
 *
 * Revision 4.9  1999/02/04  23:52:54  caress
 * MB-System version 4.6beta7
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
 * Revision 4.4  1995/03/22  19:44:26  caress
 * Added explicit casts to shorts divided by doubles for
 * ansi C compliance.
 *
 * Revision 4.3  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/11/01  16:00:08  caress
 * Fixed heading output handling.
 *
 * Revision 4.1  1994/10/21  15:42:42  caress
 * Release V4.0
 *
 * Revision 4.0  1994/10/21  12:34:58  caress
 * Release V4.0
 *
 * Revision 1.1  1994/10/21  12:20:01  caress
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
#include "../../include/mbf_sbsioswb.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* macro for rounding values to nearest integer */
#define	round(X)	X < 0.0 ? ceil(X - 0.5) : floor(X + 0.5)

/* essential function prototypes */
int mbr_register_sbsioswb(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_sbsioswb(int verbose, 
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
int mbr_alm_sbsioswb(int verbose, void *mbio_ptr, int *error);
int mbr_dem_sbsioswb(int verbose, void *mbio_ptr, int *error);
int mbr_rt_sbsioswb(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_sbsioswb(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char res_id[]="$Id: mbr_sbsioswb.c,v 5.5 2002-02-26 07:50:41 caress Exp $";

/*--------------------------------------------------------------------*/
int mbr_register_sbsioswb(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_sbsioswb";
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
	status = mbr_info_sbsioswb(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_sbsioswb;
	mb_io_ptr->mb_io_format_free = &mbr_dem_sbsioswb; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_sb_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_sb_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_sbsioswb; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_sbsioswb; 
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
int mbr_info_sbsioswb(int verbose, 
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
	char	*function_name = "mbr_info_sbsioswb";
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
	strncpy(format_name, "SBSIOSWB", MB_NAME_LENGTH);
	strncpy(system_name, "SB", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_SBSIOSWB\nInformal Description: SIO Swath-bathy SeaBeam\nAttributes:           Sea Beam, bathymetry, 19 beams, binary, centered,\n                      SIO.\n", MB_DESCRIPTION_LENGTH);
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
int mbr_alm_sbsioswb(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_sbsioswb";
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
	mb_io_ptr->structure_size = sizeof(struct mbf_sbsioswb_struct);
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
int mbr_dem_sbsioswb(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_sbsioswb";
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
int mbr_rt_sbsioswb(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_sbsioswb";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sbsioswb_struct *data;
	struct mbsys_sb_struct *store;
	char	*headerptr;
	char	*sensorptr;
	char	*datarecptr;
	char	*commentptr;
	int	read_status;
	char	dummy[2];
	double	lon, lat;
	int	id;
	int	skip;
	int	i, k;

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
	data = (struct mbf_sbsioswb_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_sb_struct *) store_ptr;

	/* get pointers to records */
	headerptr = (char *) &data->year;
	sensorptr = (char *) &data->eclipse_time;
	datarecptr = (char *) &data->beams_bath;
	commentptr = (char *) &data->comment[0];
	skip = 0;

	/* read next header record from file */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
	if ((status = fread(headerptr,1,MB_SBSIOSWB_HEADER_SIZE,
			mb_io_ptr->mbfp)) == MB_SBSIOSWB_HEADER_SIZE) 
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
	if (status == MB_SUCCESS)
		{
		data->year = mb_swap_short(data->year);
		data->day = mb_swap_short(data->day);
		data->min = mb_swap_short(data->min);
		data->sec = mb_swap_short(data->sec);
		data->lat = mb_swap_int(data->lat);
		data->lon = mb_swap_int(data->lon);
		data->heading = mb_swap_short(data->heading);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->speed_ps = mb_swap_short(data->speed_ps);
		data->quality = mb_swap_short(data->quality);
		data->sensor_size = mb_swap_short(data->sensor_size);
		data->data_size = mb_swap_short(data->data_size);
		}
#endif

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New header record read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  New header values:\n");
		fprintf(stderr,"dbg5       year:       %d\n",data->year);
		fprintf(stderr,"dbg5       day:        %d\n",data->day);
		fprintf(stderr,"dbg5       min:        %d\n",data->min);
		fprintf(stderr,"dbg5       sec:        %d\n",data->sec);
		fprintf(stderr,"dbg5       lat:        %d\n",data->lat);
		fprintf(stderr,"dbg5       lon:        %d\n",data->lon);
		fprintf(stderr,"dbg5       heading:    %d\n",data->heading);
		fprintf(stderr,"dbg5       course:     %d\n",data->course);
		fprintf(stderr,"dbg5       speed:      %d\n",data->speed);
		fprintf(stderr,"dbg5       speed_ps:   %d\n",data->speed_ps);
		fprintf(stderr,"dbg5       quality:    %d\n",data->quality);
		fprintf(stderr,"dbg5       sensor size:%d\n",data->sensor_size);
		fprintf(stderr,"dbg5       data size:  %d\n",data->data_size);
		fprintf(stderr,"dbg5       speed_ref:  %c%c\n",
			data->speed_ref[0],data->speed_ref[1]);
		fprintf(stderr,"dbg5       sensor_type:%c%c\n",
			data->sensor_type[0],data->sensor_type[1]);
		fprintf(stderr,"dbg5       data_type:  %c%c\n",
			data->data_type[0],data->data_type[1]);
		}

	/* if not a good header search through file to find one */
	while (status == MB_SUCCESS && 
		(strncmp(data->data_type,"SR",2) != 0
		&& strncmp(data->data_type,"RS",2) != 0
		&& strncmp(data->data_type,"SP",2) != 0
		&& strncmp(data->data_type,"TR",2) != 0
		&& strncmp(data->data_type,"IR",2) != 0
		&& strncmp(data->data_type,"AT",2) != 0
		&& strncmp(data->data_type,"SC",2) != 0))
		{
		/* unswap data if necessary */
#ifdef BYTESWAPPED
		data->year = mb_swap_short(data->year);
		data->day = mb_swap_short(data->day);
		data->min = mb_swap_short(data->min);
		data->sec = mb_swap_short(data->sec);
		data->lat = mb_swap_int(data->lat);
		data->lon = mb_swap_int(data->lon);
		data->heading = mb_swap_short(data->heading);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->speed_ps = mb_swap_short(data->speed_ps);
		data->quality = mb_swap_short(data->quality);
		data->sensor_size = mb_swap_short(data->sensor_size);
		data->data_size = mb_swap_short(data->data_size);
#endif

		/* shift bytes by one */
		for (i=0;i<MB_SBSIOSWB_HEADER_SIZE-1;i++)
			headerptr[i] = headerptr[i+1];
		mb_io_ptr->file_pos += 1;

		/* read next byte */
		if ((status = fread(&headerptr[MB_SBSIOSWB_HEADER_SIZE-1],
			1,1,mb_io_ptr->mbfp)) == 1) 
			{
			mb_io_ptr->file_bytes += status;
			skip++;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			mb_io_ptr->file_bytes += status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* swap data if necessary */
#ifdef BYTESWAPPED
		data->year = mb_swap_short(data->year);
		data->day = mb_swap_short(data->day);
		data->min = mb_swap_short(data->min);
		data->sec = mb_swap_short(data->sec);
		data->lat = mb_swap_int(data->lat);
		data->lon = mb_swap_int(data->lon);
		data->heading = mb_swap_short(data->heading);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->speed_ps = mb_swap_short(data->speed_ps);
		data->quality = mb_swap_short(data->quality);
		data->sensor_size = mb_swap_short(data->sensor_size);
		data->data_size = mb_swap_short(data->data_size);
#endif

		/* print debug statements */
		if (status == MB_SUCCESS && verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Header record after byte shift in MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg5  New header values:\n");
			fprintf(stderr,"dbg5       skip:       %d\n",skip);
			fprintf(stderr,"dbg5       year:       %d\n",data->year);
			fprintf(stderr,"dbg5       day:        %d\n",data->day);
			fprintf(stderr,"dbg5       min:        %d\n",data->min);
			fprintf(stderr,"dbg5       sec:        %d\n",data->sec);
			fprintf(stderr,"dbg5       lat:        %d\n",data->lat);
			fprintf(stderr,"dbg5       lon:        %d\n",data->lon);
			fprintf(stderr,"dbg5       heading:    %d\n",data->heading);
			fprintf(stderr,"dbg5       course:     %d\n",data->course);
			fprintf(stderr,"dbg5       speed:      %d\n",data->speed);
			fprintf(stderr,"dbg5       speed_ps:   %d\n",data->speed_ps);
			fprintf(stderr,"dbg5       quality:    %d\n",data->quality);
			fprintf(stderr,"dbg5       sensor size:%d\n",data->sensor_size);
			fprintf(stderr,"dbg5       data size:  %d\n",data->data_size);
			fprintf(stderr,"dbg5       speed_ref:  %c%c\n",
				data->speed_ref[0],data->speed_ref[1]);
			fprintf(stderr,"dbg5       sensor_type:%c%c\n",
				data->sensor_type[0],data->sensor_type[1]);
			fprintf(stderr,"dbg5       data_type:  %c%c\n",
				data->data_type[0],data->data_type[1]);
			}
		}

	/* check for unintelligible records */
	if (status == MB_SUCCESS)
		{
		if ((strncmp(data->sensor_type,"SB",2) != 0 || 
			strncmp(data->data_type,"SR",2) != 0)
			&& strncmp(data->data_type,"TR",2) != 0)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			data->kind = MB_DATA_NONE;

			/* read rest of record into dummy */
			for (i=0;i<data->sensor_size;i++)
				{
				if ((read_status = fread(dummy,1,1,
					mb_io_ptr->mbfp)) != 1) 
					{
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					}
				mb_io_ptr->file_bytes 
						+= read_status;
				}
			for (i=0;i<data->data_size;i++)
				{
				if ((read_status = fread(dummy,1,1,
					mb_io_ptr->mbfp)) != 1) 
					{
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					}
				mb_io_ptr->file_bytes 
						+= read_status;
				}

			}
		else if (strncmp(data->data_type,"SR",2) == 0)
			{
			data->kind = MB_DATA_DATA;
			}
		else
			{
			data->kind = MB_DATA_COMMENT;
			}
		}

	/* read sensor record from file */
	if (status == MB_SUCCESS)
		{
		if ((status = fread(sensorptr,1,data->sensor_size,
			mb_io_ptr->mbfp)) == data->sensor_size) 
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
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		data->eclipse_time = mb_swap_short(data->eclipse_time);
		data->eclipse_heading = mb_swap_short(data->eclipse_heading);
		}
#endif

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New sensor record read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  New sensor values:\n");
		fprintf(stderr,"dbg5       eclipse_time:    %d\n",
			data->eclipse_time);
		fprintf(stderr,"dbg5       eclipse_heading: %d\n",
			data->eclipse_heading);
		}

	/* read data record from file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA)
		{
		if ((status = fread(datarecptr,1,data->data_size,
			mb_io_ptr->mbfp)) == data->data_size) 
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
		}
		
	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA)
		{
		data->beams_bath = mb_swap_short(data->beams_bath);
		data->scale_factor = mb_swap_short(data->scale_factor);
		}
#endif

	/* check for unintelligible records */
	if (status == MB_SUCCESS)
		{
		if (data->beams_bath < 0
			|| data->beams_bath > MB_BEAMS_SBSIOSWB)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			data->kind = MB_DATA_NONE;
			}
		}
		
	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA)
		{
		for (i=0;i<data->beams_bath;i++)
			{
			data->bath_struct[i].bath = 
				mb_swap_short(data->bath_struct[i].bath);
			data->bath_struct[i].bath_acrosstrack = 
				mb_swap_short(data->bath_struct[i].bath_acrosstrack);
			}
		}
#endif
		
	/* check for fewer than expected beams */
	if (status == MB_SUCCESS
		&& (data->data_size / 4) - 1 < data->beams_bath)
		{
		k = (data->data_size / 4) - 2;
		for (i=k;i<data->beams_bath;i++)
		    {
		    data->bath_struct[i].bath = 0;
		    data->bath_struct[i].bath_acrosstrack = 0;
		    }
		}
		
	/* zero ridiculous soundings */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA)
		{
		for (i=0;i<data->beams_bath;i++)
		    {
		    if (data->bath_struct[i].bath > 11000
			|| data->bath_struct[i].bath_acrosstrack > 11000
			    || data->bath_struct[i].bath_acrosstrack < -11000)
			{
			data->bath_struct[i].bath = 0;
			data->bath_struct[i].bath_acrosstrack = 0;
			}
		    }
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& data->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  New data record read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  New data values:\n");
		fprintf(stderr,"dbg5       beams_bath:   %d\n",
			data->beams_bath);
		fprintf(stderr,"dbg5       scale_factor: %d\n",
			data->scale_factor);
		for (i=0;i<data->beams_bath;i++)
			fprintf(stderr,"dbg5       beam: %d  bath: %d  across_track: %d\n",
				i,data->bath_struct[i].bath,
				data->bath_struct[i].bath_acrosstrack);
		}

	/* read comment record from file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_COMMENT)
		{
		if ((status = fread(commentptr,1,data->data_size,
			mb_io_ptr->mbfp)) == data->data_size) 
			{
			mb_io_ptr->file_bytes += status;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			for (i=data->data_size;i<MB_SBSIOSWB_COMMENT_LENGTH;i++)
				commentptr[i] = '\0';
			}
		else
			{
			mb_io_ptr->file_bytes += status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& data->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"\ndbg5  New comment record read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  New comment:\n");
		fprintf(stderr,"dbg5       comment:   %s\n",
			data->comment);
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = data->kind;
	mb_io_ptr->new_error = *error;

	/* translate values to seabeam data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* type of data record */
		store->kind = data->kind;

		/* position */
		lon = 0.0000001*data->lon;
		if (lon < 0.0) lon = lon + 360.0;
		store->lon2u = (short) 60.0*lon;
		store->lon2b = (short) round(600000.0*
			(lon - store->lon2u/60.0));
		lat = 0.0000001*data->lat + 90.0;
		store->lat2u = (short) 60.0*lat;
		store->lat2b = (short) round(600000.0*
			(lat - store->lat2u/60.0));

		/* time stamp */
		store->year = data->year;
		store->day = data->day;
		store->min = data->min;
		store->sec = 0.01*data->sec;
		
		/* heading */
		store->sbhdg = (data->heading < (short) 0) 
		    ? (unsigned short) round(((int)data->heading + 3600)*18.204444444)
		    : (unsigned short) round(data->heading*18.204444444);

		/* depths and distances */
		id = data->beams_bath - 1;
		for (i=0;i<data->beams_bath;i++)
			{
			store->deph[id-i] = data->bath_struct[i].bath;
			store->dist[id-i] = 
				data->bath_struct[i].bath_acrosstrack;
			}

		/* additional values */
		store->sbtim = data->eclipse_time;
		store->axis = 0;
		store->major = 0;
		store->minor = 0;

		/* comment */
		strncpy(store->comment,data->comment,
			MBSYS_SB_MAXLINE);
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
int mbr_wt_sbsioswb(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_sbsioswb";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sbsioswb_struct *data;
	struct mbsys_sb_struct *store;
	char	*headerptr;
	char	*sensorptr;
	char	*datarecptr;
	char	*commentptr;
	double	lon, lat;
	int	id;
	int	i;

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
	data = (struct mbf_sbsioswb_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_sb_struct *) store_ptr;

	/* get pointers to records */
	headerptr = (char *) &data->year;
	sensorptr = (char *) &data->eclipse_time;
	datarecptr = (char *) &data->beams_bath;
	commentptr = (char *) &data->comment[0];

	/* first set some plausible amounts for some of the 
		variables in the SBSIOSWB record */
	data->year = 0;
	data->day = 0;
	data->min = 0;
	data->sec = 0;
	data->lat = 0;
	data->lon = 0;
	data->heading = 0;
	data->course = 0;
	data->speed = 0;
	data->speed_ps = 0;
	data->quality = 0;
	data->sensor_size = 0;
	data->data_size = 0;
	data->speed_ref[0] = 0;
	data->speed_ref[1] = 0;
	if (mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		data->sensor_type[0] = 'S';
		data->sensor_type[1] = 'B';
		data->data_type[0] = 'S';
		data->data_type[1] = 'R';
		}
	else
		{
		data->sensor_type[0] = 0;
		data->sensor_type[1] = 0;
		data->data_type[0] = 'T';
		data->data_type[1] = 'R';
		}
	data->eclipse_time = 0;
	data->eclipse_heading = 0;
	data->beams_bath = MB_BEAMS_SBSIOSWB;
	data->sensor_size = 4;
	data->data_size = 4 + 4*data->beams_bath;
	data->scale_factor = 100;
	for (i=0;i<MB_BEAMS_SBSIOSWB;i++)
		{
		data->bath_struct[i].bath = 0;
		data->bath_struct[i].bath_acrosstrack = 0;
		}

	/* second translate values from seabeam data storage structure */
	if (store != NULL)
		{
		data->kind = store->kind;
		if (store->kind == MB_DATA_DATA)
			{
			data->sensor_type[0] = 'S';
			data->sensor_type[1] = 'B';
			data->data_type[0] = 'S';
			data->data_type[1] = 'R';
			}
		else
			{
			data->sensor_type[0] = 0;
			data->sensor_type[1] = 0;
			data->data_type[0] = 'T';
			data->data_type[1] = 'R';
			}

		/* position */
		lon = 10000000*(store->lon2u/60. 
			+ store->lon2b/600000.);
		if (lon > 1800000000.)
			lon = lon - 3600000000.;
		lat = 10000000*(store->lat2u/60. 
			+ store->lat2b/600000. - 90.);
		data->lon = lon;
		data->lat = lat;

		/* time stamp */
		data->year = store->year;
		data->day = store->day;
		data->min = store->min;
		data->sec = 100*store->sec;

		/* heading */
		data->heading =  
		    (short) round(((int)store->sbhdg)*0.054931641625);

		/* additional values */
		data->eclipse_time = store->sbtim;
		data->eclipse_heading = store->sbhdg;

		if (store->kind == MB_DATA_DATA)
			{
			/* put distance and depth values 
				into sbsioswb data structure */
			id = data->beams_bath - 1;
			for (i=0;i<MB_BEAMS_SBSIOSWB;i++)
				{
				data->bath_struct[id-i].bath = store->deph[i];;
				data->bath_struct[id-i].bath_acrosstrack = 
					store->dist[i];;
				}
			}

		/* comment */
		else if (store->kind == MB_DATA_COMMENT)
			{
			strncpy(commentptr,store->comment,
				MB_SBSIOSWB_COMMENT_LENGTH-1);
			data->data_size = strlen(commentptr);
			data->sensor_size = 0;
			}
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (data->kind == MB_DATA_DATA)
		{
		data->year = mb_swap_short(data->year);
		data->day = mb_swap_short(data->day);
		data->min = mb_swap_short(data->min);
		data->sec = mb_swap_short(data->sec);
		data->lat = mb_swap_int(data->lat);
		data->lon = mb_swap_int(data->lon);
		data->heading = mb_swap_short(data->heading);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->speed_ps = mb_swap_short(data->speed_ps);
		data->quality = mb_swap_short(data->quality);
		data->sensor_size = mb_swap_short(data->sensor_size);
		data->data_size = mb_swap_short(data->data_size);
		data->eclipse_time = mb_swap_short(data->eclipse_time);
		data->eclipse_heading = mb_swap_short(data->eclipse_heading);
		data->beams_bath = mb_swap_short(data->beams_bath);
		data->scale_factor = mb_swap_short(data->scale_factor);
		for (i=0;i<data->beams_bath;i++)
			{
			data->bath_struct[i].bath = 
				mb_swap_short(data->bath_struct[i].bath);
			data->bath_struct[i].bath_acrosstrack = 
				mb_swap_short(data->bath_struct[i].bath_acrosstrack);
			}
		}
#endif

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Ready to write data in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",
			data->kind);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Header record to be written by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  Header values:\n");
		fprintf(stderr,"dbg5       year:       %d\n",data->year);
		fprintf(stderr,"dbg5       day:        %d\n",data->day);
		fprintf(stderr,"dbg5       min:        %d\n",data->min);
		fprintf(stderr,"dbg5       sec:        %d\n",data->sec);
		fprintf(stderr,"dbg5       lat:        %d\n",data->lat);
		fprintf(stderr,"dbg5       lon:        %d\n",data->lon);
		fprintf(stderr,"dbg5       heading:    %d\n",data->heading);
		fprintf(stderr,"dbg5       course:     %d\n",data->course);
		fprintf(stderr,"dbg5       speed:      %d\n",data->speed);
		fprintf(stderr,"dbg5       speed_ps:   %d\n",data->speed_ps);
		fprintf(stderr,"dbg5       quality:    %d\n",data->quality);
		fprintf(stderr,"dbg5       sensor size:%d\n",data->sensor_size);
		fprintf(stderr,"dbg5       data size:  %d\n",data->data_size);
		fprintf(stderr,"dbg5       speed_ref:  %c%c\n",
			data->speed_ref[0],data->speed_ref[1]);
		fprintf(stderr,"dbg5       sensor_type:%c%c\n",
			data->sensor_type[0],data->sensor_type[1]);
		fprintf(stderr,"dbg5       data_type:  %c%c\n",
			data->data_type[0],data->data_type[1]);
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& data->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  Sensor record to be written by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  Sensor values:\n");
		fprintf(stderr,"dbg5       eclipse_time:    %d\n",
			data->eclipse_time);
		fprintf(stderr,"dbg5       eclipse_heading: %d\n",
			data->eclipse_heading);
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& data->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  Data record to be written by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  Data values:\n");
		fprintf(stderr,"dbg5       beams_bath:   %d\n",
			data->beams_bath);
		fprintf(stderr,"dbg5       scale_factor: %d\n",
			data->scale_factor);
		for (i=0;i<data->beams_bath;i++)
			fprintf(stderr,"dbg5       beam: %d  bath: %d  across_track: %d\n",
				i,data->bath_struct[i].bath,
				data->bath_struct[i].bath_acrosstrack);
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& data->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"\ndbg5  Comment record to be written by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  Comment:\n");
		fprintf(stderr,"dbg5       comment:   %s\n",
			data->comment);
		}

	/* write header record to file */
	if (status == MB_SUCCESS)
		{
		if ((status = fwrite(headerptr,1,MB_SBSIOSWB_HEADER_SIZE,
			mb_io_ptr->mbfp)) == MB_SBSIOSWB_HEADER_SIZE) 
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

	/* write sensor record to file */
	if (status == MB_SUCCESS)
		{
		if ((status = fwrite(sensorptr,1,data->sensor_size,
			mb_io_ptr->mbfp)) == data->sensor_size) 
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

	/* write data record to file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA)
		{
		if ((status = fwrite(datarecptr,1,data->data_size,
			mb_io_ptr->mbfp)) == data->data_size) 
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

	/* write comment record to file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_COMMENT)
		{
		if ((status = fwrite(commentptr,1,strlen(data->comment),
			mb_io_ptr->mbfp)) == strlen(data->comment)) 
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
