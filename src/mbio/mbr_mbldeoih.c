/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mbldeoih.c	2/2/93
 *	$Id: mbr_mbldeoih.c,v 5.2 2001-01-22 07:43:34 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000 by
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
 * mbr_mbldeoih.c contains the functions for reading and writing
 * multibeam data in the MBF_MBLDEOIH format.  
 * These functions include:
 *   mbr_alm_mbldeoih	- allocate read/write memory
 *   mbr_dem_mbldeoih	- deallocate read/write memory
 *   mbr_rt_mbldeoih	- read and translate data
 *   mbr_wt_mbldeoih	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
 * $Log: not supported by cvs2svn $
 * Revision 5.1  2000/12/10  20:26:50  caress
 * Version 5.0.alpha02
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
 * Revision 4.10  2000/07/19  03:51:38  caress
 * Fixed some things.
 *
 * Revision 4.9  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
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
 * Revision 4.1  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/21  03:59:50  caress
 * First cut at new version. Altered to be consistent
 * with passing of three types of data: bathymetry,
 * amplitude, and sidescan.
 *
 * Revision 3.0  1993/05/14  22:56:57  sohara
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
#include "../../include/mbf_mbldeoih.h"
#include "../../include/mbsys_ldeoih.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* essential function prototypes */
int mbr_info_mbldeoih(int verbose, 
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
			int (**format_alloc)(), 
			int (**format_free)(), 
			int (**store_alloc)(), 
			int (**store_free)(), 
			int (**read_ping)(), 
			int (**write_ping)(), 
			int (**extract)(), 
			int (**insert)(), 
			int (**extract_nav)(), 
			int (**insert_nav)(), 
			int (**extract_altitude)(), 
			int (**insert_altitude)(), 
			int (**extract_svp)(), 
			int (**insert_svp)(), 
			int (**ttimes)(), 
			int (**copyrecord)(), 
			int *error);
int mbr_alm_mbldeoih(int verbose, char *mbio_ptr, int *error);
int mbr_dem_mbldeoih(int verbose, char *mbio_ptr, int *error);
int mbr_rt_mbldeoih(int verbose, char *mbio_ptr, char *store_ptr, int *error);
int mbr_wt_mbldeoih(int verbose, char *mbio_ptr, char *store_ptr, int *error);

/*--------------------------------------------------------------------*/
int mbr_info_mbldeoih(int verbose, 
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
			int (**format_alloc)(), 
			int (**format_free)(), 
			int (**store_alloc)(), 
			int (**store_free)(), 
			int (**read_ping)(), 
			int (**write_ping)(), 
			int (**extract)(), 
			int (**insert)(), 
			int (**extract_nav)(), 
			int (**insert_nav)(), 
			int (**extract_altitude)(), 
			int (**insert_altitude)(), 
			int (**extract_svp)(), 
			int (**insert_svp)(), 
			int (**ttimes)(), 
			int (**copyrecord)(), 
			int *error)
{
	static char res_id[]="$Id: mbr_mbldeoih.c,v 5.2 2001-01-22 07:43:34 caress Exp $";
	char	*function_name = "mbr_info_mbldeoih";
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
	*system = MB_SYS_LDEOIH;
	*beams_bath_max = 250;
	*beams_amp_max = 250;
	*pixels_ss_max = 10000;
	strncpy(format_name, "MBLDEOIH", MB_NAME_LENGTH);
	strncpy(system_name, "LDEOIH", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_MBLDEOIH\nInformal Description: L-DEO in-house generic multibeam\nAttributes:           Data from all sonar systems, bathymetry, \n                      amplitude and sidescan, variable beams and pixels, \n                      binary, centered, L-DEO.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_YES;
	*traveltime = MB_NO;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 0.0;

	/* set format and system specific function pointers */
	*format_alloc = &mbr_alm_mbldeoih;
	*format_free = &mbr_dem_mbldeoih; 
	*store_alloc = &mbsys_ldeoih_alloc; 
	*store_free = &mbsys_ldeoih_deall; 
	*read_ping = &mbr_rt_mbldeoih; 
	*write_ping = &mbr_wt_mbldeoih; 
	*extract = &mbsys_ldeoih_extract; 
	*insert = &mbsys_ldeoih_insert; 
	*extract_nav = &mbsys_ldeoih_extract_nav; 
	*insert_nav = &mbsys_ldeoih_insert_nav; 
	*extract_altitude = &mbsys_ldeoih_extract_altitude; 
	*insert_altitude = &mbsys_ldeoih_insert_altitude; 
	*extract_svp = NULL; 
	*insert_svp = NULL; 
	*ttimes = &mbsys_ldeoih_ttimes; 
	*copyrecord = &mbsys_ldeoih_copy; 

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
		fprintf(stderr,"dbg2       format_alloc:       %d\n",*format_alloc);
		fprintf(stderr,"dbg2       format_free:        %d\n",*format_free);
		fprintf(stderr,"dbg2       store_alloc:        %d\n",*store_alloc);
		fprintf(stderr,"dbg2       store_free:         %d\n",*store_free);
		fprintf(stderr,"dbg2       read_ping:          %d\n",*read_ping);
		fprintf(stderr,"dbg2       write_ping:         %d\n",*write_ping);
		fprintf(stderr,"dbg2       extract:            %d\n",*extract);
		fprintf(stderr,"dbg2       insert:             %d\n",*insert);
		fprintf(stderr,"dbg2       extract_nav:        %d\n",*extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %d\n",*insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %d\n",*extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %d\n",*insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %d\n",*extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %d\n",*insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %d\n",*ttimes);
		fprintf(stderr,"dbg2       copyrecord:         %d\n",*copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}


/*--------------------------------------------------------------------*/
int mbr_alm_mbldeoih(int verbose, char *mbio_ptr, int *error)
{
 static char res_id[]="$Id: mbr_mbldeoih.c,v 5.2 2001-01-22 07:43:34 caress Exp $";
	char	*function_name = "mbr_alm_mbldeoih";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mbldeoih_struct *dataplus;
	struct mbf_mbldeoih_header_struct *header;
	struct mbf_mbldeoih_data_struct *data;
	int	i;

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
	mb_io_ptr->structure_size = sizeof(struct mbf_mbldeoih_struct);
	mb_io_ptr->header_structure_size = 
		sizeof(struct mbf_mbldeoih_header_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mbsys_ldeoih_alloc(verbose,mbio_ptr,
		&mb_io_ptr->store_data,error);

	/* get pointer to raw data structure */
	dataplus = (struct mbf_mbldeoih_struct *) mb_io_ptr->raw_data;
	header = &(dataplus->header);
	data = &(dataplus->data);

	/* initialize values in structure */
	dataplus->kind = MB_DATA_NONE;
	header->lon2u = 0;
	header->lon2b = 0;
	header->lat2u = 0;
	header->lat2b = 0;
	header->year = 0;
	header->day = 0;
	header->min = 0;
	header->sec = 0;
	header->msec = 0;
	header->heading = 0;
	header->speed = 0;
	header->beams_amp = 0;
	header->pixels_ss = 0;
	header->depth_scale = 0;
	header->distance_scale = 0;
	header->transducer_depth = 0;
	header->altitude = 0;
	for (i=0;i<MBF_MBLDEOIH_MAX_BEAMS;i++)
	    {
	    data->beamflag[i] = 0;
	    data->bath[i] = 0;
	    data->bath_acrosstrack[i] = 0;
	    data->bath_alongtrack[i] = 0;
	    data->amp[i] = 0;
	    }
	for (i=0;i<MBF_MBLDEOIH_MAX_PIXELS;i++)
	    {
	    data->ss[i] = 0;
	    data->ss_acrosstrack[i] = 0;
	    data->ss_alongtrack[i] = 0;
	    }
	memset(dataplus->comment, 0, 128);

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
int mbr_dem_mbldeoih(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_mbldeoih";
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
	status = mbsys_ldeoih_deall(verbose,mbio_ptr,
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
int mbr_rt_mbldeoih(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_mbldeoih";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mbldeoih_struct *dataplus;
	struct mbf_mbldeoih_header_struct *header;
	struct mbf_mbldeoih_data_struct *data;
	struct mbsys_ldeoih_struct *store;
	char	*comment;
	unsigned char	*beamflag;
	short int	*bath;
	short int	*amp;
	short int	*bath_acrosstrack;
	short int	*bath_alongtrack;
	short int	*ss;
	short int	*ss_acrosstrack;
	short int	*ss_alongtrack;
	int	read_size;
	int	time_j[5];
	double	depthscale;
	double	distscale;
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

	/* get pointer to mbio descriptor and data structure */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get pointer to raw data structure */
	dataplus = (struct mbf_mbldeoih_struct *) mb_io_ptr->raw_data;
	header = &(dataplus->header);
	data = &(dataplus->data);
	comment = dataplus->comment;
	beamflag = data->beamflag;
	bath = data->bath;
	amp = data->amp;
	bath_acrosstrack = data->bath_acrosstrack;
	bath_alongtrack = data->bath_alongtrack;
	ss = data->ss;
	ss_acrosstrack = data->ss_acrosstrack;
	ss_alongtrack = data->ss_alongtrack;
	
	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next header from file */
	if ((status = fread(header,1,mb_io_ptr->header_structure_size,
			mb_io_ptr->mbfp)) == mb_io_ptr->header_structure_size) 
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
		header->flag = mb_swap_short(header->flag);
		header->year = mb_swap_short(header->year);
		header->day = mb_swap_short(header->day);
		header->min = mb_swap_short(header->min);
		header->sec = mb_swap_short(header->sec);
		header->msec = mb_swap_short(header->msec);
		header->lon2u = mb_swap_short(header->lon2u);
		header->lon2b = mb_swap_short(header->lon2b);
		header->lat2u = mb_swap_short(header->lat2u);
		header->lat2b = mb_swap_short(header->lat2b);
		header->heading = mb_swap_short(header->heading);
		header->speed = mb_swap_short(header->speed);
		header->beams_bath = mb_swap_short(header->beams_bath);
		header->beams_amp = mb_swap_short(header->beams_amp);
		header->pixels_ss = mb_swap_short(header->pixels_ss);
		header->depth_scale = mb_swap_short(header->depth_scale);
		header->distance_scale = mb_swap_short(header->distance_scale);
		}
#endif

	/* check for comment or unintelligible records */
	if (status == MB_SUCCESS)
		{
		if (header->flag == 8995)
			{
			dataplus->kind = MB_DATA_COMMENT;
			}
		else if (header->flag == 25700)
			{
			dataplus->kind = MB_DATA_DATA;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			dataplus->kind = MB_DATA_NONE;
			}
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = dataplus->kind;
	mb_io_ptr->new_error = *error;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New header read in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       flag:       %d\n",header->flag);
		fprintf(stderr,"dbg5       year:       %d\n",header->year);
		fprintf(stderr,"dbg5       day:        %d\n",header->day);
		fprintf(stderr,"dbg5       minute:     %d\n",header->min);
		fprintf(stderr,"dbg5       second:     %d\n",header->sec);
		fprintf(stderr,"dbg5       msec:       %d\n",header->msec);
		fprintf(stderr,"dbg5       lonu:       %d\n",header->lon2u);
		fprintf(stderr,"dbg5       lonb:       %d\n",header->lon2b);
		fprintf(stderr,"dbg5       latu:       %d\n",header->lat2u);
		fprintf(stderr,"dbg5       latb:       %d\n",header->lat2b);
		fprintf(stderr,"dbg5       heading:    %d\n",header->heading);
		fprintf(stderr,"dbg5       speed:      %d\n",header->speed);
		fprintf(stderr,"dbg5       beams bath: %d\n",
			header->beams_bath);
		fprintf(stderr,"dbg5       beams amp:  %d\n",
			header->beams_amp);
		fprintf(stderr,"dbg5       pixels ss:  %d\n",
			header->pixels_ss);
		fprintf(stderr,"dbg5       depth scale:%d\n",header->depth_scale);
		fprintf(stderr,"dbg5       dist scale: %d\n",header->distance_scale);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		}

	/* read next chunk of the data */
	if (status == MB_SUCCESS 
		&& dataplus->kind == MB_DATA_COMMENT)
		{
		read_size = 128;
		if ((status = fread(comment,1,read_size,mb_io_ptr->mbfp))
			== read_size) 
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

		/* print debug messages */
		if (verbose >= 5 && status == MB_SUCCESS)
			{
			fprintf(stderr,"\ndbg5  New comment read in function <%s>\n",
				function_name);
			fprintf(stderr,"dbg5       comment: %s\n",comment);
			}
		}
	else if (status == MB_SUCCESS 
		&& store != NULL
		&& dataplus->kind == MB_DATA_DATA)
		{
		/* if needed reset numbers of beams and allocate 
		   memory for store arrays */
		if (header->beams_bath > store->beams_bath_alloc)
		    {
		    store->beams_bath_alloc = header->beams_bath;
		    if (store->beamflag != NULL)
			status = mb_free(verbose, &store->beamflag, error);
		    if (store->bath != NULL)
			status = mb_free(verbose, &store->bath, error);
		    if (store->bath_acrosstrack != NULL)
			status = mb_free(verbose, &store->bath_acrosstrack, error);
		    if (store->bath_alongtrack != NULL)
			status = mb_free(verbose, &store->bath_alongtrack, error);
		    status = mb_malloc(verbose, 
				store->beams_bath_alloc * sizeof(char),
				&store->beamflag,error);
		    status = mb_malloc(verbose, 
				store->beams_bath_alloc * sizeof(short),
				&store->bath,error);
		    status = mb_malloc(verbose, 
				store->beams_bath_alloc * sizeof(short),
				&store->bath_acrosstrack,error);
		    status = mb_malloc(verbose, 
				store->beams_bath_alloc * sizeof(short),
				&store->bath_alongtrack,error);

		    /* deal with a memory allocation failure */
		    if (status == MB_FAILURE)
			{
			status = mb_free(verbose, &store->beamflag, error);
			status = mb_free(verbose, &store->bath, error);
			status = mb_free(verbose, &store->bath_acrosstrack, error);
			status = mb_free(verbose, &store->bath_alongtrack, error);
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
		if (header->beams_amp > store->beams_amp_alloc)
		    {
		    store->beams_amp_alloc = header->beams_amp;
		    if (store != NULL)
			{
			if (store->amp != NULL)
			    status = mb_free(verbose, &store->amp, error);
			status = mb_malloc(verbose, 
				    store->beams_amp_alloc * sizeof(short),
				    &store->amp,error);

			/* deal with a memory allocation failure */
			if (status == MB_FAILURE)
			    {
			    status = mb_free(verbose, &store->amp, error);
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
		    }
		if (header->pixels_ss > store->pixels_ss_alloc)
		    {
		    store->pixels_ss_alloc = header->pixels_ss;
		    if (store != NULL)
			{
			if (store->ss != NULL)
			    status = mb_free(verbose, &store->ss, error);
			if (store->ss_acrosstrack != NULL)
			    status = mb_free(verbose, &store->ss_acrosstrack, error);
			if (store->ss_alongtrack != NULL)
			    status = mb_free(verbose, &store->ss_alongtrack, error);
			status = mb_malloc(verbose, 
				    store->pixels_ss_alloc * sizeof(short),
				    &store->ss,error);
			status = mb_malloc(verbose, 
				    store->pixels_ss_alloc * sizeof(short),
				    &store->ss_acrosstrack,error);
			status = mb_malloc(verbose, 
				    store->pixels_ss_alloc * sizeof(short),
				    &store->ss_alongtrack,error);

			/* deal with a memory allocation failure */
			if (status == MB_FAILURE)
			    {
			    status = mb_free(verbose, &store->ss, error);
			    status = mb_free(verbose, &store->ss_acrosstrack, error);
			    status = mb_free(verbose, &store->ss_alongtrack, error);
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
		    }

		/* read bathymetry */
		read_size = sizeof(char)*header->beams_bath;
		status = fread(beamflag,1,read_size,mb_io_ptr->mbfp);
		read_size = sizeof(short int)*header->beams_bath;
		status = fread(bath,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;
		status = fread(bath_acrosstrack,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;
		status = fread(bath_alongtrack,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;

		/* read amplitudes */
		read_size = sizeof(short int)*header->beams_amp;
		status = fread(amp,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;

		/* read sidescan */
		read_size = sizeof(short int)*header->pixels_ss;
		status = fread(ss,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;
		status = fread(ss_acrosstrack,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;
		status = fread(ss_alongtrack,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;

		/* byte swap the data if necessary */
#ifdef BYTESWAPPED
		for (i=0;i<header->beams_bath;i++)
			{
			data->bath[i] = mb_swap_short(data->bath[i]);
			data->bath_acrosstrack[i] 
				= mb_swap_short(data->bath_acrosstrack[i]);
			data->bath_alongtrack[i] 
				= mb_swap_short(data->bath_alongtrack[i]);
			}
		for (i=0;i<header->beams_amp;i++)
			{
			data->amp[i] = mb_swap_short(data->amp[i]);
			}
		for (i=0;i<header->pixels_ss;i++)
			{
			data->ss[i] = mb_swap_short(data->ss[i]);
			data->ss_acrosstrack[i] 
				= mb_swap_short(data->ss_acrosstrack[i]);
			data->ss_alongtrack[i] 
				= mb_swap_short(data->ss_alongtrack[i]);
			}
#endif

		/* check for end of file */
		if (status == read_size) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* print debug messages */
		if (verbose >= 5 && status == MB_SUCCESS)
			{
			fprintf(stderr,"\ndbg5  New data read in function <%s>\n",
				function_name);
			fprintf(stderr,"dbg5       beams_bath: %d\n",
				header->beams_bath);
			fprintf(stderr,"dbg5       beams_amp:  %d\n",
				header->beams_amp);
			for (i=0;i<header->beams_bath;i++)
			  fprintf(stderr,"dbg5       beam:%d  flag:%d  bath:%d  amp:%d  acrosstrack:%d  alongtrack:%d\n",
				i,beamflag[i],bath[i],amp[i],
				bath_acrosstrack[i],bath_alongtrack[i]);
			fprintf(stderr,"dbg5       pixels_ss:  %d\n",
				header->pixels_ss);
			  fprintf(stderr,"dbg5       pixel:%d  ss:%d acrosstrack:%d  alongtrack:%d\n",
				i,ss[i],
				ss_acrosstrack[i],ss_alongtrack[i]);
			}
		}

	/* translate data values to data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL
		&& dataplus->kind == MB_DATA_DATA)
		{
		/* type of data record */
		store->kind = dataplus->kind;

		/* position */
		store->lon2u = header->lon2u;
		store->lon2b = header->lon2b;
		store->lat2u = header->lat2u;
		store->lat2b = header->lat2b;

		/* time stamp */
		store->year = header->year;
		store->day = header->day;
		store->min = header->min;
		store->sec = header->sec;
		store->msec = header->msec;

		/* heading and speed */
		store->heading = header->heading;
		store->speed = header->speed;

		/* numbers of beams and scaling */
		store->beams_bath = header->beams_bath;
		store->beams_amp = header->beams_amp;
		store->pixels_ss = header->pixels_ss;
		store->depth_scale = header->depth_scale;
		store->distance_scale = header->distance_scale;

		/* get altitude and transducer depth */
		store->altitude = header->altitude;
		store->transducer_depth = header->transducer_depth;

		/* depths and backscatter */
		for (i=0;i<store->beams_bath;i++)
			{
			store->beamflag[i] = data->beamflag[i];
			store->bath[i] = data->bath[i];
			store->bath_acrosstrack[i] = data->bath_acrosstrack[i];
			store->bath_alongtrack[i] = data->bath_alongtrack[i];
			}
		for (i=0;i<store->beams_amp;i++)
			{
			store->amp[i] = data->amp[i];
			}
		for (i=0;i<store->pixels_ss;i++)
			{
			store->ss[i] = data->ss[i];
			store->ss_acrosstrack[i] = data->ss_acrosstrack[i];
			store->ss_alongtrack[i] = data->ss_alongtrack[i];
			}
		}

	/* translate comment to data storage structure */
	else if (status == MB_SUCCESS
		&& store != NULL
		&& dataplus->kind == MB_DATA_COMMENT)
		{
		/* type of data record */
		store->kind = dataplus->kind;

		/* comment */
		strcpy(store->comment,comment);
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
int mbr_wt_mbldeoih(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_mbldeoih";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mbldeoih_struct *dataplus;
	struct mbf_mbldeoih_header_struct *header;
	struct mbf_mbldeoih_data_struct *data;
	struct mbsys_ldeoih_struct *store;
	char	*comment;
	unsigned char	*beamflag;
	short int	*bath;
	short int	*amp;
	short int	*bath_acrosstrack;
	short int	*bath_alongtrack;
	short int	*ss;
	short int	*ss_acrosstrack;
	short int	*ss_alongtrack;
	int	write_size;
	int	time_j[5];
	double	lon;
	double	lat;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	double	depthmax;
	double	distmax;
	double	depthscale;
	double	distscale;
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

	/* get pointer to mbio descriptor and data storage */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get pointer to raw data structure */
	dataplus = (struct mbf_mbldeoih_struct *) mb_io_ptr->raw_data;
	header = &(dataplus->header);
	data = &(dataplus->data);
	comment = dataplus->comment;
	beamflag = data->beamflag;
	bath = data->bath;
	amp = data->amp;
	bath_acrosstrack = data->bath_acrosstrack;
	bath_alongtrack = data->bath_alongtrack;
	ss = data->ss;
	ss_acrosstrack = data->ss_acrosstrack;
	ss_alongtrack = data->ss_alongtrack;
	header->depth_scale = 0;
	header->distance_scale = 0;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		/* save beam and pixel numbers */
		beams_bath = store->beams_bath;
		beams_amp = store->beams_amp;
		pixels_ss = store->pixels_ss;

		/* type of data record */
		dataplus->kind = store->kind;

		/* position */
		header->lon2u = store->lon2u;
		header->lon2b = store->lon2b;
		header->lat2u = store->lat2u;
		header->lat2b = store->lat2b;

		/* time stamp */
		header->year = store->year;
		header->day = store->day;
		header->min = store->min;
		header->sec = store->sec;
		header->msec = store->msec;

		/* heading and speed */
		header->heading = store->heading;
		header->speed = store->speed;

		/* numbers of beams and scaling */
		header->beams_bath = store->beams_bath;
		header->beams_amp = store->beams_amp;
		header->pixels_ss = store->pixels_ss;
		header->depth_scale = store->depth_scale;
		header->distance_scale = store->distance_scale;

		/* get altitude and transducer depth */
		header->altitude = store->altitude;
		header->transducer_depth = store->transducer_depth;

		/* depths amplitude and sidescan */
		for (i=0;i<header->beams_bath;i++)
			{
			beamflag[i] = store->beamflag[i];
			bath[i] = store->bath[i];
			bath_acrosstrack[i] = store->bath_acrosstrack[i];
			bath_alongtrack[i] = store->bath_alongtrack[i];
			}
		for (i=0;i<header->beams_amp;i++)
			{
			amp[i] = store->amp[i];
			}
		for (i=0;i<header->pixels_ss;i++)
			{
			ss[i] = store->ss[i];
			ss_acrosstrack[i] = store->ss_acrosstrack[i];
			ss_alongtrack[i] = store->ss_alongtrack[i];
			}

		/* comment */
		strcpy(comment,store->comment);
		}

	/* set data flag 
		(data: flag='dd'=25700 or comment:flag='##'=8995) */
	if (dataplus->kind == MB_DATA_DATA)
		header->flag = 25700;
	else
		header->flag = 8995;


	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New header set in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       flag:       %d\n",header->flag);
		fprintf(stderr,"dbg5       year:       %d\n",header->year);
		fprintf(stderr,"dbg5       day:        %d\n",header->day);
		fprintf(stderr,"dbg5       minute:     %d\n",header->min);
		fprintf(stderr,"dbg5       second:     %d\n",header->sec);
		fprintf(stderr,"dbg5       msec:       %d\n",header->msec);
		fprintf(stderr,"dbg5       lonu:       %d\n",header->lon2u);
		fprintf(stderr,"dbg5       lonb:       %d\n",header->lon2b);
		fprintf(stderr,"dbg5       latu:       %d\n",header->lat2u);
		fprintf(stderr,"dbg5       latb:       %d\n",header->lat2b);
		fprintf(stderr,"dbg5       heading:    %d\n",header->heading);
		fprintf(stderr,"dbg5       speed:      %d\n",header->speed);
		fprintf(stderr,"dbg5       beams bath: %d\n",
			header->beams_bath);
		fprintf(stderr,"dbg5       beams amp:  %d\n",
			header->beams_amp);
		fprintf(stderr,"dbg5       pixels ss:  %d\n",
			header->pixels_ss);
		fprintf(stderr,"dbg5       depth scale: %d\n",header->depth_scale);
		fprintf(stderr,"dbg5       dist scale:  %d\n",header->distance_scale);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		header->flag = mb_swap_short(header->flag);
		header->year = mb_swap_short(header->year);
		header->day = mb_swap_short(header->day);
		header->min = mb_swap_short(header->min);
		header->sec = mb_swap_short(header->sec);
		header->msec = mb_swap_short(header->msec);
		header->lon2u = mb_swap_short(header->lon2u);
		header->lon2b = mb_swap_short(header->lon2b);
		header->lat2u = mb_swap_short(header->lat2u);
		header->lat2b = mb_swap_short(header->lat2b);
		header->heading = mb_swap_short(header->heading);
		header->speed = mb_swap_short(header->speed);
		header->beams_bath = mb_swap_short(header->beams_bath);
		header->beams_amp = mb_swap_short(header->beams_amp);
		header->pixels_ss = mb_swap_short(header->pixels_ss);
		header->depth_scale = mb_swap_short(header->depth_scale);
		header->distance_scale = mb_swap_short(header->distance_scale);
		}
#endif

	/* write next header to file */
	if ((status = fwrite(header,1,mb_io_ptr->header_structure_size,
			mb_io_ptr->mbfp)) == mb_io_ptr->header_structure_size) 
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Going to write data in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",dataplus->kind);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		}
	if (verbose >=5 && dataplus->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg5       comment:    %s\n",comment);
		}
	if (verbose >=5 && dataplus->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg5       beams_bath: %d\n",beams_bath);
		fprintf(stderr,"dbg5       beams_amp:  %d\n",beams_amp);
		for (i=0;i<beams_bath;i++)
			fprintf(stderr,"dbg5       beam:%d  flag:%d  bath:%d  amp:%d  acrosstrack:%d  alongtrack:%d\n",
				i,beamflag[i],bath[i],amp[i],bath_acrosstrack[i],bath_alongtrack[i]);
		fprintf(stderr,"dbg5       pixels_ss:  %d\n",pixels_ss);
		for (i=0;i<pixels_ss;i++)
			fprintf(stderr,"dbg5       beam:%d  ss:%d  acrosstrack:%d  alongtrack:%d\n",
				i,ss[i],ss_acrosstrack[i],ss_alongtrack[i]);
		}

	/* write next chunk of the data */
	if (dataplus->kind == MB_DATA_COMMENT)
		{
		write_size = 128;
		if ((status = fwrite(comment,1,write_size,mb_io_ptr->mbfp))
			== write_size) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	else if (dataplus->kind == MB_DATA_DATA)
		{

		/* byte swap the data if necessary */
#ifdef BYTESWAPPED
		for (i=0;i<beams_bath;i++)
			{
			data->bath[i] = mb_swap_short(data->bath[i]);
			data->bath_acrosstrack[i] 
				= mb_swap_short(data->bath_acrosstrack[i]);
			data->bath_alongtrack[i] 
				= mb_swap_short(data->bath_alongtrack[i]);
			}
		for (i=0;i<beams_amp;i++)
			{
			data->amp[i] = mb_swap_short(data->amp[i]);
			}
		for (i=0;i<pixels_ss;i++)
			{
			data->ss[i] = mb_swap_short(data->ss[i]);
			data->ss_acrosstrack[i] 
				= mb_swap_short(data->ss_acrosstrack[i]);
			data->ss_alongtrack[i] 
				= mb_swap_short(data->ss_alongtrack[i]);
			}
#endif

		/* write bathymetry */
		write_size = sizeof(char)*beams_bath;
		status = fwrite(beamflag,1,write_size,mb_io_ptr->mbfp);
		write_size = sizeof(short int)*beams_bath;
		status = fwrite(bath,1,write_size,mb_io_ptr->mbfp);
		status = fwrite(bath_acrosstrack,1,write_size,mb_io_ptr->mbfp);
		status = fwrite(bath_alongtrack,1,write_size,mb_io_ptr->mbfp);

		/* write amplitude */
		write_size = sizeof(short int)*beams_amp;
		status = fwrite(amp,1,write_size,mb_io_ptr->mbfp);

		/* write sidescan */
		write_size = sizeof(short int)*pixels_ss;
		status = fwrite(ss,1,write_size,mb_io_ptr->mbfp);
		status = fwrite(ss_acrosstrack,1,write_size,mb_io_ptr->mbfp);
		status = fwrite(ss_alongtrack,1,write_size,mb_io_ptr->mbfp);

		/* check for error */
		if (status == write_size) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
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
