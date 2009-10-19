/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mbldeoih.c	2/2/93
 *	$Id: mbr_mbldeoih.c,v 5.14 2008/07/10 06:43:40 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2002, 2003 by
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
 * $Log: mbr_mbldeoih.c,v $
 * Revision 5.14  2008/07/10 06:43:40  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.13  2007/10/08 15:59:34  caress
 * MBIO changes as of 8 October 2007.
 *
 * Revision 5.12  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.11  2005/03/25 04:21:33  caress
 * Corrected problem with debug output of sidescan data.
 *
 * Revision 5.10  2004/12/02 06:33:31  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.9  2004/09/16 18:59:42  caress
 * Comment updates.
 *
 * Revision 5.8  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.7  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.6  2002/09/25 20:41:04  caress
 * Fixed old DSL120 format.
 *
 * Revision 5.5  2002/04/06 02:43:39  caress
 * Release 5.0.beta16
 *
 * Revision 5.4  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.3  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
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
/*
 * Notes on the MBF_MBLDEOIH data format:
 *   1. This data format is used to store swath bathymetry
 *      and/or backscatter data with arbitrary numbers of beams
 *      and pixels. This format was created by the 
 *      Lamont-Doherty Earth Observatory and the Monterey Bay  
 *      Aquarium Research Institute to serve as general  
 *      purpose archive formats for processed swath data.
 *   2. The format stores bathymetry, amplitude, and sidescan data.
 *   3. Each data record has a header section and a data section.
 *      The beginning of each header is a two byte identifier.
 *      The size of the header depends on the identifier:
 *           "##" =  8995 : Old comment - 30 byte header
 *           "dd" = 25700 : Old data - 30 byte header
 *           "cc" = 25443 : New comment - 36 byte header
 *           "nn" = 28270 : New data - 2 byte header
 *      In the case of data records, the header contains the time stamp,
 *      navigation, and the numbers of depth, beam amplitude, and
 *      sidescan values.  The data section contains the depth and
 *      backscatter values.  The number of depth and beam amplitude
 *      values is generally different from the number of sidescan
 *      values, so the length of the data section must be calculated
 *      from the numbers of beams and pixels. In the case of comment
 *      records, the header contains no information other than the
 *      identifier whether it is old (30 byte) or new (2 byte). The
 *      data section of the comment record is always 128 bytes. 
 *   4. The data headers have changed and now include beam angle
 *      widths to allow beam footprint calculation. Older data 
 *      is read without complaint, and the beam widths are passed
 *      as zero.
 *   5. The data consist of variable length binary records encoded
 *	entirely in 2-byte integers.
 *   6. All data arrays are centered.
 *
 * The kind value in the mbsys_ldeoih_struct indicates whether the
 * structure holds data (kind = 1) or an
 * ascii comment record (kind = 0).
 *
 * The structures used to represent the binary data in the MBF_MBLDEOIH format
 * are documented in the mbsys_ldeoih.h file.
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
int mbr_register_mbldeoih(int verbose, void *mbio_ptr, 
		int *error);
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
			int *svp_source,
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error);
int mbr_alm_mbldeoih(int verbose, void *mbio_ptr, int *error);
int mbr_dem_mbldeoih(int verbose, void *mbio_ptr, int *error);
int mbr_rt_mbldeoih(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_mbldeoih(int verbose, void *mbio_ptr, void *store_ptr, int *error);

/* define maximum number of beams */
#define	MBF_MBLDEOIH_MAX_BEAMS	250
#define	MBF_MBLDEOIH_MAX_PIXELS	10000

/* define header sizes */
#define	MBF_MBLDEOIH_OLDHEADERSIZE	38
#define	MBF_MBLDEOIH_NEWHEADERSIZE	44

static char res_id[]="$Id: mbr_mbldeoih.c,v 5.14 2008/07/10 06:43:40 caress Exp $";

/*--------------------------------------------------------------------*/
int mbr_register_mbldeoih(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_mbldeoih";
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
	status = mbr_info_mbldeoih(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_mbldeoih;
	mb_io_ptr->mb_io_format_free = &mbr_dem_mbldeoih; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_ldeoih_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_ldeoih_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_mbldeoih; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_mbldeoih; 
	mb_io_ptr->mb_io_dimensions = &mbsys_ldeoih_dimensions; 
	mb_io_ptr->mb_io_sidescantype = &mbsys_ldeoih_sidescantype; 
	mb_io_ptr->mb_io_extract = &mbsys_ldeoih_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_ldeoih_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_ldeoih_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_ldeoih_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_ldeoih_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = &mbsys_ldeoih_insert_altitude; 
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL; 
	mb_io_ptr->mb_io_ttimes = &mbsys_ldeoih_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_ldeoih_copy; 
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
		fprintf(stderr,"dbg2       dimensions:         %d\n",mb_io_ptr->mb_io_dimensions);
		fprintf(stderr,"dbg2       sidescantype:       %d\n",mb_io_ptr->mb_io_sidescantype);
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
			int *svp_source, 
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error)
{
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
	*beams_bath_max = 3003;
	*beams_amp_max = 1440;
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
int mbr_alm_mbldeoih(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_mbldeoih";
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
	status = mbsys_ldeoih_alloc(verbose,mbio_ptr,
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
int mbr_dem_mbldeoih(int verbose, void *mbio_ptr, int *error)
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
int mbr_rt_mbldeoih(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_mbldeoih";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	int	read_size;
	short	*flag;
	int	header_length;
	char	buffer[MBF_MBLDEOIH_NEWHEADERSIZE];
	int	index;
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
	
	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next header id from file */
	if ((status = fread(buffer,1,2,mb_io_ptr->mbfp)) == 2) 
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

	/* read rest of header from file */
	if (status == MB_SUCCESS)
		{
		flag = (short *) buffer;
		if (*flag == 8995)
			{
			store->kind = MB_DATA_COMMENT;
			header_length = MBF_MBLDEOIH_OLDHEADERSIZE;
			}
		else if (*flag == 25443)
			{
			store->kind = MB_DATA_COMMENT;
			header_length = 2;
			}
		else if (*flag == 28270)
			{
			store->kind = MB_DATA_DATA;
			header_length = MBF_MBLDEOIH_NEWHEADERSIZE;
			}
		else if (*flag == 25700)
			{
			store->kind = MB_DATA_DATA;
			header_length = MBF_MBLDEOIH_OLDHEADERSIZE;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			store->kind = MB_DATA_NONE;
			}
		}
	if (status == MB_SUCCESS
	    && header_length == 2)
		{
		/* only 2 byte header for new style comment */
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else if (status == MB_SUCCESS
	    && (status = fread(&buffer[2],1,header_length-2,
			mb_io_ptr->mbfp)) == header_length-2)
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

	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA)
		{
		index = 2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->year); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->day); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->min); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->sec); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->msec); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->lon2u); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->lon2b); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->lat2u); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->lat2b); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->heading); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->speed); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->beams_bath); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->beams_amp); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->pixels_ss); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->depth_scale); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->distance_scale); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->transducer_depth); index +=2;
		mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->altitude); index +=2;
		if (header_length == MBF_MBLDEOIH_NEWHEADERSIZE)
			{
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->beam_xwidth); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->beam_lwidth); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &store->ss_type); index +=2;
			}
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = store->kind;
	mb_io_ptr->new_error = *error;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New header read in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       flag:       %d\n",*flag);
		}
	if (verbose >= 5 && store->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       minute:           %d\n",store->min);
		fprintf(stderr,"dbg5       second:           %d\n",store->sec);
		fprintf(stderr,"dbg5       msec:             %d\n",store->msec);
		fprintf(stderr,"dbg5       lonu:             %d\n",store->lon2u);
		fprintf(stderr,"dbg5       lonb:             %d\n",store->lon2b);
		fprintf(stderr,"dbg5       latu:             %d\n",store->lat2u);
		fprintf(stderr,"dbg5       latb:             %d\n",store->lat2b);
		fprintf(stderr,"dbg5       heading:          %d\n",store->heading);
		fprintf(stderr,"dbg5       speed:            %d\n",store->speed);
		fprintf(stderr,"dbg5       beams bath:       %d\n",
			store->beams_bath);
		fprintf(stderr,"dbg5       beams amp:        %d\n",
			store->beams_amp);
		fprintf(stderr,"dbg5       pixels ss:        %d\n",
			store->pixels_ss);
		fprintf(stderr,"dbg5       depth scale:      %d\n",store->depth_scale);
		fprintf(stderr,"dbg5       dist scale:       %d\n",store->distance_scale);
		fprintf(stderr,"dbg5       transducer_depth: %d\n",store->transducer_depth);
		fprintf(stderr,"dbg5       altitude:         %d\n",store->altitude);
		fprintf(stderr,"dbg5       beam_xwidth:      %d\n",store->beam_xwidth);
		fprintf(stderr,"dbg5       beam_lwidth:      %d\n",store->beam_lwidth);
		fprintf(stderr,"dbg5       ss_type:          %d\n",store->ss_type);
		fprintf(stderr,"dbg5       status:           %d\n",status);
		fprintf(stderr,"dbg5       error:            %d\n",*error);
		}

	/* read next chunk of the data */
	if (status == MB_SUCCESS 
		&& store->kind == MB_DATA_COMMENT)
		{
		read_size = 128;
		if ((status = fread(store->comment,1,read_size,mb_io_ptr->mbfp))
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
			fprintf(stderr,"dbg5       comment: %s\n",store->comment);
			}
		}
	else if (status == MB_SUCCESS 
		&& store->kind == MB_DATA_DATA)
		{
		/* if needed reset numbers of beams and allocate 
		   memory for store arrays */
		if (store->beams_bath > store->beams_bath_alloc)
		    {
		    store->beams_bath_alloc = store->beams_bath;
		    if (store->beamflag != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->beamflag, error);
		    if (store->bath != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->bath, error);
		    if (store->bath_acrosstrack != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->bath_acrosstrack, error);
		    if (store->bath_alongtrack != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->bath_alongtrack, error);
		    status = mb_mallocd(verbose,__FILE__,__LINE__, 
				store->beams_bath_alloc * sizeof(char),
				(void **)&store->beamflag,error);
		    status = mb_mallocd(verbose,__FILE__,__LINE__, 
				store->beams_bath_alloc * sizeof(short),
				(void **)&store->bath,error);
		    status = mb_mallocd(verbose,__FILE__,__LINE__, 
				store->beams_bath_alloc * sizeof(short),
				(void **)&store->bath_acrosstrack,error);
		    status = mb_mallocd(verbose,__FILE__,__LINE__, 
				store->beams_bath_alloc * sizeof(short),
				(void **)&store->bath_alongtrack,error);

		    /* deal with a memory allocation failure */
		    if (status == MB_FAILURE)
			{
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->beamflag, error);
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->bath, error);
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->bath_acrosstrack, error);
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->bath_alongtrack, error);
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
		if (store->beams_amp > store->beams_amp_alloc)
		    {
		    store->beams_amp_alloc = store->beams_amp;
		    if (store->amp != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->amp, error);
		    status = mb_mallocd(verbose,__FILE__,__LINE__, 
				store->beams_amp_alloc * sizeof(short),
				(void **)&store->amp,error);

		    /* deal with a memory allocation failure */
		    if (status == MB_FAILURE)
			{
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->amp, error);
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
		if (store->pixels_ss > store->pixels_ss_alloc)
		    {
		    store->pixels_ss_alloc = store->pixels_ss;
		    if (store->ss != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->ss, error);
		    if (store->ss_acrosstrack != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->ss_acrosstrack, error);
		    if (store->ss_alongtrack != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->ss_alongtrack, error);
		    status = mb_mallocd(verbose,__FILE__,__LINE__, 
				store->pixels_ss_alloc * sizeof(short),
				(void **)&store->ss,error);
		    status = mb_mallocd(verbose,__FILE__,__LINE__, 
				store->pixels_ss_alloc * sizeof(short),
				(void **)&store->ss_acrosstrack,error);
		    status = mb_mallocd(verbose,__FILE__,__LINE__, 
				store->pixels_ss_alloc * sizeof(short),
				(void **)&store->ss_alongtrack,error);

		    /* deal with a memory allocation failure */
		    if (status == MB_FAILURE)
			{
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->ss, error);
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->ss_acrosstrack, error);
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->ss_alongtrack, error);
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

		/* read bathymetry */
		read_size = sizeof(char)*store->beams_bath;
		status = fread(store->beamflag,1,read_size,mb_io_ptr->mbfp);
		read_size = sizeof(short int)*store->beams_bath;
		status = fread(store->bath,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;
		status = fread(store->bath_acrosstrack,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;
		status = fread(store->bath_alongtrack,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;

		/* read amplitudes */
		read_size = sizeof(short int)*store->beams_amp;
		status = fread(store->amp,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;

		/* read sidescan */
		read_size = sizeof(short int)*store->pixels_ss;
		status = fread(store->ss,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;
		status = fread(store->ss_acrosstrack,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;
		status = fread(store->ss_alongtrack,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;

		/* byte swap the data if necessary */
#ifdef BYTESWAPPED
		for (i=0;i<store->beams_bath;i++)
			{
			store->bath[i] = mb_swap_short(store->bath[i]);
			store->bath_acrosstrack[i] 
				= mb_swap_short(store->bath_acrosstrack[i]);
			store->bath_alongtrack[i] 
				= mb_swap_short(store->bath_alongtrack[i]);
			}
		for (i=0;i<store->beams_amp;i++)
			{
			store->amp[i] = mb_swap_short(store->amp[i]);
			}
		for (i=0;i<store->pixels_ss;i++)
			{
			store->ss[i] = mb_swap_short(store->ss[i]);
			store->ss_acrosstrack[i] 
				= mb_swap_short(store->ss_acrosstrack[i]);
			store->ss_alongtrack[i] 
				= mb_swap_short(store->ss_alongtrack[i]);
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
				store->beams_bath);
			for (i=0;i<store->beams_bath;i++)
			  fprintf(stderr,"dbg5       beam:%d  flag:%d  bath:%d  acrosstrack:%d  alongtrack:%d\n",
				i,store->beamflag[i],store->bath[i],
				store->bath_acrosstrack[i],store->bath_alongtrack[i]);
			fprintf(stderr,"dbg5       beams_amp:  %d\n",
				store->beams_amp);
			for (i=0;i<store->beams_amp;i++)
			  fprintf(stderr,"dbg5       beam:%d  flag:%d  amp:%d  acrosstrack:%d  alongtrack:%d\n",
				i,store->beamflag[i],store->amp[i],
				store->bath_acrosstrack[i],store->bath_alongtrack[i]);
			fprintf(stderr,"dbg5       pixels_ss:  %d\n",
				store->pixels_ss);
			for (i=0;i<store->pixels_ss;i++)
			  fprintf(stderr,"dbg5       pixel:%d  ss:%d acrosstrack:%d  alongtrack:%d\n",
				i,store->ss[i],
				store->ss_acrosstrack[i],store->ss_alongtrack[i]);
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
int mbr_wt_mbldeoih(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_mbldeoih";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	int	write_size;
	short	*flag;
	int	header_length;
	char	buffer[MBF_MBLDEOIH_NEWHEADERSIZE];
	int	index;
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

	/* set data flag 
		(data: flag='nn'=28270 or comment:flag='cc'=25443) */
	flag = (short *) buffer;
	if (store->kind == MB_DATA_DATA)
		{
		*flag = 28270;
		header_length = MBF_MBLDEOIH_NEWHEADERSIZE;
		}
	else
		{
		*flag = 25443;
		header_length = 2;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New header set in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       flag:       %d\n",*flag);
		}
	if (verbose >= 5 && store->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       minute:           %d\n",store->min);
		fprintf(stderr,"dbg5       second:           %d\n",store->sec);
		fprintf(stderr,"dbg5       msec:             %d\n",store->msec);
		fprintf(stderr,"dbg5       lonu:             %d\n",store->lon2u);
		fprintf(stderr,"dbg5       lonb:             %d\n",store->lon2b);
		fprintf(stderr,"dbg5       latu:             %d\n",store->lat2u);
		fprintf(stderr,"dbg5       latb:             %d\n",store->lat2b);
		fprintf(stderr,"dbg5       heading:          %d\n",store->heading);
		fprintf(stderr,"dbg5       speed:            %d\n",store->speed);
		fprintf(stderr,"dbg5       beams bath:       %d\n",store->beams_bath);
		fprintf(stderr,"dbg5       beams amp:        %d\n",store->beams_amp);
		fprintf(stderr,"dbg5       pixels ss:        %d\n",store->pixels_ss);
		fprintf(stderr,"dbg5       depth scale:      %d\n",store->depth_scale);
		fprintf(stderr,"dbg5       dist scale:       %d\n",store->distance_scale);
		fprintf(stderr,"dbg5       transducer_depth: %d\n",store->transducer_depth);
		fprintf(stderr,"dbg5       altitude:         %d\n",store->altitude);
		fprintf(stderr,"dbg5       beam_xwidth:      %d\n",store->beam_xwidth);
		fprintf(stderr,"dbg5       beam_lwidth:      %d\n",store->beam_lwidth);
		fprintf(stderr,"dbg5       ss_type:          %d\n",store->ss_type);
		fprintf(stderr,"dbg5       status:           %d\n",status);
		fprintf(stderr,"dbg5       error:            %d\n",*error);
		}

	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA)
		{
		index = 2;
		mb_put_binary_short(MB_NO, store->year, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->day, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->min, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->sec, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->msec, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->lon2u, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->lon2b, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->lat2u, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->lat2b, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->heading, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->speed, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->beams_bath, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->beams_amp, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->pixels_ss, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->depth_scale, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->distance_scale, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->transducer_depth, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->altitude, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->beam_xwidth, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->beam_lwidth, (void *)  &buffer[index]); index +=2;
		mb_put_binary_short(MB_NO, store->ss_type, (void *)  &buffer[index]); index +=2;
		}

	/* write next header to file */
	if ((status = fwrite(buffer,1,header_length,
			mb_io_ptr->mbfp)) == header_length) 
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
		fprintf(stderr,"dbg5       kind:       %d\n",store->kind);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		}
	if (verbose >= 5 && store->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg5       comment:    %s\n",store->comment);
		}
	if (verbose >= 5 && store->kind == MB_DATA_DATA)
		{
			fprintf(stderr,"dbg5       beams_bath: %d\n",
				store->beams_bath);
			for (i=0;i<store->beams_bath;i++)
			  fprintf(stderr,"dbg5       beam:%d  flag:%d  bath:%d  acrosstrack:%d  alongtrack:%d\n",
				i,store->beamflag[i],store->bath[i],
				store->bath_acrosstrack[i],store->bath_alongtrack[i]);
			fprintf(stderr,"dbg5       beams_amp:  %d\n",
				store->beams_amp);
			for (i=0;i<store->beams_amp;i++)
			  fprintf(stderr,"dbg5       beam:%d  flag:%d  amp:%d  acrosstrack:%d  alongtrack:%d\n",
				i,store->beamflag[i],store->amp[i],
				store->bath_acrosstrack[i],store->bath_alongtrack[i]);
			fprintf(stderr,"dbg5       pixels_ss:  %d\n",
				store->pixels_ss);
			for (i=0;i<store->pixels_ss;i++)
			  fprintf(stderr,"dbg5       pixel:%d  ss:%d acrosstrack:%d  alongtrack:%d\n",
				i,store->ss[i],
				store->ss_acrosstrack[i],store->ss_alongtrack[i]);
		}

	/* write next chunk of the data */
	if (store->kind == MB_DATA_COMMENT)
		{
		write_size = 128;
		if ((status = fwrite(store->comment,1,write_size,mb_io_ptr->mbfp))
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
	else if (store->kind == MB_DATA_DATA)
		{

		/* byte swap the data if necessary */
#ifdef BYTESWAPPED
		for (i=0;i<store->beams_bath;i++)
			{
			store->bath[i] = mb_swap_short(store->bath[i]);
			store->bath_acrosstrack[i] 
				= mb_swap_short(store->bath_acrosstrack[i]);
			store->bath_alongtrack[i] 
				= mb_swap_short(store->bath_alongtrack[i]);
			}
		for (i=0;i<store->beams_amp;i++)
			{
			store->amp[i] = mb_swap_short(store->amp[i]);
			}
		for (i=0;i<store->pixels_ss;i++)
			{
			store->ss[i] = mb_swap_short(store->ss[i]);
			store->ss_acrosstrack[i] 
				= mb_swap_short(store->ss_acrosstrack[i]);
			store->ss_alongtrack[i] 
				= mb_swap_short(store->ss_alongtrack[i]);
			}
#endif

		/* write bathymetry */
		write_size = sizeof(char) * store->beams_bath;
		status = fwrite(store->beamflag,1,write_size,mb_io_ptr->mbfp);
		write_size = sizeof(short int) * store->beams_bath;
		status = fwrite(store->bath,1,write_size,mb_io_ptr->mbfp);
		status = fwrite(store->bath_acrosstrack,1,write_size,mb_io_ptr->mbfp);
		status = fwrite(store->bath_alongtrack,1,write_size,mb_io_ptr->mbfp);

		/* write amplitude */
		write_size = sizeof(short int) * store->beams_amp;
		status = fwrite(store->amp,1,write_size,mb_io_ptr->mbfp);

		/* write sidescan */
		write_size = sizeof(short int) * store->pixels_ss;
		status = fwrite(store->ss,1,write_size,mb_io_ptr->mbfp);
		status = fwrite(store->ss_acrosstrack,1,write_size,mb_io_ptr->mbfp);
		status = fwrite(store->ss_alongtrack,1,write_size,mb_io_ptr->mbfp);

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
