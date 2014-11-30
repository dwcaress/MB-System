/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mbldeoih.c	2/2/93
 *	$Id$
 *
 *    Copyright (c) 1993-2014 by
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
 *           "cc" = 25443 : New comment - 36 byte header
 *           "dd" = 25700 : Version 1 survey data - 38 byte header
 *           "nn" = 28270 : Version 2 survey data - 44 byte header
 *           "DD" = 17476 : Version 3 survey data - 48 byte header
 *           "V4" = 22068 : Version 4 survey data - 90 byte header (13398 little-endian)
 *           "V5" = 22069 : Version 5 survey data - 98 byte header (13654 little-endian)
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
 *   4. The data headers changed for version 2, including beam angle
 *      widths to allow beam footprint calculation. Older data
 *      are read without complaint, and the beam widths are passed
 *      as zero.
 *   5. The data headers changed again for version 3. Previously the
 *      bathymetry values were absolute depths. For version 3 the
 *      stored bathymetry are in depths relative to the sonar, and the
 *      transducer depth must be added to calculate absolute depths.
 *      Older data are read without complaint, and converted to version
 *      3 on writing.
 *   6. The data consist of variable length binary records encoded
 *	entirely in 2-byte integers.
 *   7. All data arrays are centered.
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
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_ldeoih.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "mb_swap.h"
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

/* define header sizes */
#define	MBF_MBLDEOIH_V1HEADERSIZE	38
#define	MBF_MBLDEOIH_V2HEADERSIZE	44
#define	MBF_MBLDEOIH_V3HEADERSIZE	48
#define	MBF_MBLDEOIH_V4HEADERSIZE	90
#define	MBF_MBLDEOIH_V5HEADERSIZE	98
#define	MBF_MBLDEOIH_ID_COMMENT1	8995	/* ## */
#define	MBF_MBLDEOIH_ID_COMMENT2	25443	/* cc */
#define	MBF_MBLDEOIH_ID_DATA1		25700	/* dd */
#define	MBF_MBLDEOIH_ID_DATA2		28270	/* nn */
#define	MBF_MBLDEOIH_ID_DATA3		17476	/* DD */
#define	MBF_MBLDEOIH_ID_DATA4		22068	/* V4 big endian, 13398 little endian*/
#define	MBF_MBLDEOIH_ID_DATA5		22069	/* V5 bin endian, 13654 little endian */

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_mbldeoih(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_mbldeoih";
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
	mb_io_ptr->mb_io_sonartype = &mbsys_ldeoih_sonartype;
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
	mb_io_ptr->mb_io_detects = &mbsys_ldeoih_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_ldeoih_copy;
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
		fprintf(stderr,"dbg2       format_alloc:       %p\n",(void *)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %p\n",(void *)mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %p\n",(void *)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %p\n",(void *)mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %p\n",(void *)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %p\n",(void *)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       dimensions:         %p\n",(void *)mb_io_ptr->mb_io_dimensions);
		fprintf(stderr,"dbg2       sidescantype:       %p\n",(void *)mb_io_ptr->mb_io_sidescantype);
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
		fprintf(stderr,"dbg2       copyrecord:         %p\n",(void *)mb_io_ptr->mb_io_copyrecord);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_LDEOIH;
	*beams_bath_max = 0;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
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
int mbr_alm_mbldeoih(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_mbldeoih";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

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
	status = mbsys_ldeoih_alloc(verbose,mbio_ptr,
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
int mbr_dem_mbldeoih(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_mbldeoih";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

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

	/* deallocate memory for data descriptor */
	status = mbsys_ldeoih_deall(verbose,mbio_ptr,
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
int mbr_rt_mbldeoih(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_mbldeoih";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	struct mbsys_ldeoih_old_struct oldstore;
	int	read_size;
	short	*flag;
	short	short_transducer_depth;
	short	short_altitude;
	short	short_beams_bath, short_beams_amp, short_pixels_ss, short_spare1;
	int	header_length;
	char	buffer[MBF_MBLDEOIH_V4HEADERSIZE];
	int	index;
	double	newdepthscale;
	double	depthmax;
	int	time_i[7], time_j[6];
	int	version;
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
#ifdef BYTESWAPPED
		*flag = mb_swap_short(*flag);
#endif
		if (*flag == MBF_MBLDEOIH_ID_COMMENT1)
			{
			store->kind = MB_DATA_COMMENT;
			header_length = MBF_MBLDEOIH_V1HEADERSIZE;
			}
		else if (*flag == MBF_MBLDEOIH_ID_COMMENT2)
			{
			store->kind = MB_DATA_COMMENT;
			header_length = 2;
			}
		else if (*flag == MBF_MBLDEOIH_ID_DATA5)
			{
			store->kind = MB_DATA_DATA;
			header_length = MBF_MBLDEOIH_V5HEADERSIZE;
			version = 5;
			}
		else if (*flag == MBF_MBLDEOIH_ID_DATA4)
			{
			store->kind = MB_DATA_DATA;
			header_length = MBF_MBLDEOIH_V4HEADERSIZE;
			version = 4;
			}
		else if (*flag == MBF_MBLDEOIH_ID_DATA3)
			{
			store->kind = MB_DATA_DATA;
			header_length = MBF_MBLDEOIH_V3HEADERSIZE;
			version = 3;
			}
		else if (*flag == MBF_MBLDEOIH_ID_DATA2)
			{
			store->kind = MB_DATA_DATA;
			header_length = MBF_MBLDEOIH_V2HEADERSIZE;
			version = 2;
			}
		else if (*flag == MBF_MBLDEOIH_ID_DATA1)
			{
			store->kind = MB_DATA_DATA;
			header_length = MBF_MBLDEOIH_V1HEADERSIZE;
			version = 1;
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
		if (version == 5)
			{
			index = 2;
			mb_get_binary_double(MB_NO, (void *)  &buffer[index], &store->time_d); index +=8;
			mb_get_binary_double(MB_NO, (void *)  &buffer[index], &store->longitude); index +=8;
			mb_get_binary_double(MB_NO, (void *)  &buffer[index], &store->latitude); index +=8;
			mb_get_binary_double(MB_NO, (void *)  &buffer[index], &store->sonardepth); index +=8;
			mb_get_binary_double(MB_NO, (void *)  &buffer[index], &store->altitude); index +=8;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->heading); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->speed); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->roll); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->pitch); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->heave); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->beam_xwidth); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->beam_lwidth); index +=4;
			mb_get_binary_int(MB_NO, (void *)  &buffer[index], &store->beams_bath); index +=4;
			mb_get_binary_int(MB_NO, (void *)  &buffer[index], &store->beams_amp); index +=4;
			mb_get_binary_int(MB_NO, (void *)  &buffer[index], &store->pixels_ss); index +=4;
			mb_get_binary_int(MB_NO, (void *)  &buffer[index], &store->spare1); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->depth_scale); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->distance_scale); index +=4;
			store->ss_scalepower = buffer[index]; index++;
			store->ss_type = buffer[index]; index++;
			store->imagery_type = buffer[index]; index++;
			store->topo_type = buffer[index]; index++;
			}
		else if (version == 4)
			{
			index = 2;
			mb_get_binary_double(MB_NO, (void *)  &buffer[index], &store->time_d); index +=8;
			mb_get_binary_double(MB_NO, (void *)  &buffer[index], &store->longitude); index +=8;
			mb_get_binary_double(MB_NO, (void *)  &buffer[index], &store->latitude); index +=8;
			mb_get_binary_double(MB_NO, (void *)  &buffer[index], &store->sonardepth); index +=8;
			mb_get_binary_double(MB_NO, (void *)  &buffer[index], &store->altitude); index +=8;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->heading); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->speed); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->roll); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->pitch); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->heave); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->beam_xwidth); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->beam_lwidth); index +=4;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &short_beams_bath); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &short_beams_amp); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &short_pixels_ss); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &short_spare1); index +=2;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->depth_scale); index +=4;
			mb_get_binary_float(MB_NO, (void *)  &buffer[index], &store->distance_scale); index +=4;
			store->ss_scalepower = buffer[index]; index++;
			store->ss_type = buffer[index]; index++;
			store->imagery_type = buffer[index]; index++;
			store->topo_type = buffer[index]; index++;
			store->beams_bath = short_beams_bath;
			store->beams_amp = short_beams_amp;
			store->pixels_ss = short_pixels_ss;
			store->spare1 = short_spare1;
			}
		else
			{
			index = 2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.year); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.day); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.min); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.sec); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.msec); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.lon2u); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.lon2b); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.lat2u); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.lat2b); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.heading); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.speed); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.beams_bath); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.beams_amp); index +=2;
			mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.pixels_ss); index +=2;
			if (version == 1)
				{
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.depth_scale); index +=2;
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.distance_scale); index +=2;
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &short_transducer_depth); index +=2;
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &short_altitude); index +=2;
				oldstore.transducer_depth = (int) (oldstore.depth_scale * short_transducer_depth);
				oldstore.altitude = (int) (oldstore.depth_scale * short_altitude);
				}
			else if (version == 2)
				{
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.depth_scale); index +=2;
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.distance_scale); index +=2;
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &short_transducer_depth); index +=2;
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &short_altitude); index +=2;
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.beam_xwidth); index +=2;
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.beam_lwidth); index +=2;
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.ss_type); index +=2;
				oldstore.transducer_depth = (int) (oldstore.depth_scale * short_transducer_depth);
				oldstore.altitude = (int) (oldstore.depth_scale * short_altitude);
				}
			else if (version == 3)
				{
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.depth_scale); index +=2;
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.distance_scale); index +=2;
				mb_get_binary_int(MB_NO, (void *)  &buffer[index], &oldstore.transducer_depth); index +=4;
				mb_get_binary_int(MB_NO, (void *)  &buffer[index], &oldstore.altitude); index +=4;
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.beam_xwidth); index +=2;
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.beam_lwidth); index +=2;
				mb_get_binary_short(MB_NO, (void *)  &buffer[index], &oldstore.ss_type); index +=2;
				}

			/* translate old data to current */

			/* get time */
			time_j[0] = oldstore.year;
			time_j[1] = oldstore.day;
			time_j[2] = oldstore.min;
			time_j[3] = oldstore.sec;
			time_j[4] = 1000 * oldstore.msec;
			mb_get_itime(verbose,time_j,time_i);
			mb_get_time(verbose,time_i,&store->time_d);

			/* get navigation */
			store->longitude = ((double) oldstore.lon2u)/60.
						+ ((double) oldstore.lon2b)/600000.;
			store->latitude = ((double) oldstore.lat2u)/60.
						+ ((double) oldstore.lat2b)/600000. - 90.;

			/* get sonardepth and altitude */
			store->sonardepth = 0.001 * oldstore.transducer_depth;
		    	store->altitude = 0.001 * oldstore.altitude;

			/* get heading (360 degrees = 65536) and speed */
			store->heading = (float) (0.0054932 * oldstore.heading);
			store->speed = (float)(0.01 * oldstore.speed);

			/* set roll and pitch to zero */
			store->roll = 0.0;
			store->pitch = 0.0;

			/* set beamwidths in mb_io structure */
			if (oldstore.beam_xwidth > 0)
			    store->beam_xwidth = 0.01 * oldstore.beam_xwidth;
			else
			    store->beam_xwidth = 2.0;
			if (oldstore.beam_lwidth > 0)
			    store->beam_lwidth = 0.01 * oldstore.beam_lwidth;
			else
			    store->beam_lwidth = 2.0;

			/* get beams_bath, beams_amp, pixels_ss */
			store->beams_bath = oldstore.beams_bath;
			store->beams_amp = oldstore.beams_amp;
			store->pixels_ss = oldstore.pixels_ss;
			store->spare1 = 0;

			/* get scaling */
			store->depth_scale = 0.001 * oldstore.depth_scale;
			store->distance_scale = 0.001 * oldstore.distance_scale;

			/* get sidescan type */
			store->ss_scalepower = 0;
			store->ss_type = oldstore.ss_type;
			store->imagery_type = 0;
			store->topo_type = MB_TOPOGRAPHY_TYPE_UNKNOWN;
			}
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = store->kind;
	mb_io_ptr->new_error = *error;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New header read in function <%s>\n",function_name);
		fprintf(stderr,"dbg5       flag:       %d\n",*flag);
		}
	if (verbose >= 5 && store->kind == MB_DATA_DATA && version < 4)
		{
		fprintf(stderr,"\ndbg5  Old version header read in function <%s>\n",function_name);
		fprintf(stderr,"dbg5       version:          %d\n",version);
		fprintf(stderr,"dbg5       year:             %d\n",oldstore.year);
		fprintf(stderr,"dbg5       day:              %d\n",oldstore.day);
		fprintf(stderr,"dbg5       minute:           %d\n",oldstore.min);
		fprintf(stderr,"dbg5       second:           %d\n",oldstore.sec);
		fprintf(stderr,"dbg5       msec:             %d\n",oldstore.msec);
		fprintf(stderr,"dbg5       lonu:             %d\n",oldstore.lon2u);
		fprintf(stderr,"dbg5       lonb:             %d\n",oldstore.lon2b);
		fprintf(stderr,"dbg5       latu:             %d\n",oldstore.lat2u);
		fprintf(stderr,"dbg5       latb:             %d\n",oldstore.lat2b);
		fprintf(stderr,"dbg5       heading:          %d\n",oldstore.heading);
		fprintf(stderr,"dbg5       speed:            %d\n",oldstore.speed);
		fprintf(stderr,"dbg5       beams bath:       %d\n",oldstore.beams_bath);
		fprintf(stderr,"dbg5       beams amp:        %d\n",oldstore.beams_amp);
		fprintf(stderr,"dbg5       pixels ss:        %d\n",oldstore.pixels_ss);
		fprintf(stderr,"dbg5       depth scale:      %d\n",oldstore.depth_scale);
		fprintf(stderr,"dbg5       dist scale:       %d\n",oldstore.distance_scale);
		fprintf(stderr,"dbg5       transducer_depth: %d\n",oldstore.transducer_depth);
		fprintf(stderr,"dbg5       altitude:         %d\n",oldstore.altitude);
		fprintf(stderr,"dbg5       beam_xwidth:      %d\n",oldstore.beam_xwidth);
		fprintf(stderr,"dbg5       beam_lwidth:      %d\n",oldstore.beam_lwidth);
		fprintf(stderr,"dbg5       ss_type:          %d\n",oldstore.ss_type);
		fprintf(stderr,"dbg5       status:           %d\n",status);
		fprintf(stderr,"dbg5       error:            %d\n",*error);
		}
	if (verbose >= 5 && store->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  Current version header values in function <%s>\n",function_name);
		fprintf(stderr,"dbg5       time_d:           %f\n",store->time_d);
		fprintf(stderr,"dbg5       longitude:        %f\n",store->longitude);
		fprintf(stderr,"dbg5       latitude:         %f\n",store->latitude);
		fprintf(stderr,"dbg5       sonardepth:       %f\n",store->sonardepth);
		fprintf(stderr,"dbg5       altitude:         %f\n",store->altitude);
		fprintf(stderr,"dbg5       heading:          %f\n",store->heading);
		fprintf(stderr,"dbg5       speed:            %f\n",store->speed);
		fprintf(stderr,"dbg5       roll:             %f\n",store->roll);
		fprintf(stderr,"dbg5       pitch:            %f\n",store->pitch);
		fprintf(stderr,"dbg5       heave:            %f\n",store->heave);
		fprintf(stderr,"dbg5       beam_xwidth:      %f\n",store->beam_xwidth);
		fprintf(stderr,"dbg5       beam_lwidth:      %f\n",store->beam_lwidth);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",store->beams_bath);
		fprintf(stderr,"dbg5       beams_amp:        %d\n",store->beams_amp);
		fprintf(stderr,"dbg5       pixels_ss:        %d\n",store->pixels_ss);
		fprintf(stderr,"dbg5       spare1:           %d\n",store->spare1);
		fprintf(stderr,"dbg5       depth_scale:      %f\n",store->depth_scale);
		fprintf(stderr,"dbg5       distance_scale:   %f\n",store->distance_scale);
		fprintf(stderr,"dbg5       ss_scalepower:    %d\n",store->ss_scalepower);
		fprintf(stderr,"dbg5       ss_type:          %d\n",store->ss_type);
		fprintf(stderr,"dbg5       spare3:           %d\n",store->imagery_type);
		fprintf(stderr,"dbg5       sonartype:        %d\n",store->topo_type);
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
			fprintf(stderr,"\ndbg5  New header comment in function <%s>\n",function_name);
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
				fprintf(stderr,"\ndbg2  MBIO function <%s> terminated with error\n",function_name);
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
				fprintf(stderr,"\ndbg2  MBIO function <%s> terminated with error\n",function_name);
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
				fprintf(stderr,"\ndbg2  MBIO function <%s> terminated with error\n",function_name);
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

		/* subtract the transducer depth from the bathymetry if version
			1 or 2 data has been read */
		if (version < 3)
			{
			depthmax = 0.0;
			for (i=0;i<store->beams_bath;i++)
				{
				depthmax = MAX(depthmax, (store->depth_scale * store->bath[i] - store->sonardepth));
				}
			if (depthmax > 0.0)
				newdepthscale = 0.001 * (double)(MAX((int) (1 + depthmax / 30.0), 1));
			for (i=0;i<store->beams_bath;i++)
				{
				store->bath[i] = (short)((store->depth_scale * store->bath[i] - store->sonardepth) / newdepthscale);
				}
			store->depth_scale = newdepthscale;
			}

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
			
		/* update maximum numbers of beams and pixels */
		mb_io_ptr->beams_bath_max = MAX(mb_io_ptr->beams_bath_max, store->beams_bath);
		mb_io_ptr->beams_amp_max = MAX(mb_io_ptr->beams_amp_max, store->beams_amp);
		mb_io_ptr->pixels_ss_max = MAX(mb_io_ptr->pixels_ss_max, store->pixels_ss);

		/* print debug messages */
		if (verbose >= 5 && status == MB_SUCCESS)
			{
			fprintf(stderr,"\ndbg5  New data read in function <%s>\n",function_name);
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
int mbr_wt_mbldeoih(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_mbldeoih";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	struct mbsys_ldeoih_old_struct oldstore;
	int	write_size;
	short	*flag;
	int	header_length;
	char	buffer[MBF_MBLDEOIH_V4HEADERSIZE];
	int	index;
	double	depthscale, newdepthscale;
	double	depthmax, transducer_depth;
	double	navlon, navlat;
	short	short_transducer_depth;
	short	short_altitude;
	int	*version;
	int	time_j[5], time_i[7];
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

	/* get pointer to mbio descriptor and data storage */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* set data flag pointer */
	flag = (short *) buffer;

	/* set version pointer */
	version = &(mb_io_ptr->save1);

	/* if set, write old format - this should only happen for writing fbt files
		when the user has set fbtversion = old in the .mbio_defaults file
		using mbdefaults */
	if (store->kind == MB_DATA_DATA && *version == 2)
		{
		*flag = MBF_MBLDEOIH_ID_DATA2;
		header_length = MBF_MBLDEOIH_V2HEADERSIZE;

		/* translate data to old format */

		/* get time */
		mb_get_date(verbose, store->time_d, time_i);
		mb_get_jtime(verbose,time_i,time_j);
		oldstore.year = time_j[0];
		oldstore.day = time_j[1];
		oldstore.min = time_j[2];
		oldstore.sec = time_j[3];
		oldstore.msec = (short int)(((double)time_j[4] / 1000.0) + 0.5);

		/* get navigation */
		navlon = store->longitude;
		if (navlon < 0.0)
			navlon = navlon + 360.0;
		oldstore.lon2u = (short int) 60.0 * navlon;
		oldstore.lon2b = (short int) (600000.0 *
					(navlon - ((double) oldstore.lon2u)/60.0));
		navlat = store->latitude + 90.0;
		oldstore.lat2u = (short int) 60.0 * navlat;
		oldstore.lat2b = (short int) (600000.0 * (navlat - ((double) oldstore.lat2u)/60.0));

		/* get heading (360 degrees = 65536) */
		oldstore.heading = 182.044444 * store->heading;

		oldstore.speed = 0.01 * store->speed;

		/* get beams_bath, beams_amp, pixels_ss */
		oldstore.beams_bath = store->beams_bath;
		oldstore.beams_amp = store->beams_amp;
		oldstore.pixels_ss = store->pixels_ss;

		/* set beamwidths */
		oldstore.beam_xwidth = 100 *store->beam_xwidth;
		oldstore.beam_lwidth = 100 *store->beam_lwidth;

		/* get scaling */
		oldstore.depth_scale = 1000 * store->depth_scale;
		oldstore.distance_scale = 1000 * store->distance_scale;
		if (oldstore.depth_scale == 0)
			oldstore.depth_scale = 10;
		if (oldstore.distance_scale == 0)
			oldstore.distance_scale = 10;

		/* set scaled transducer_depth and altitude */
		oldstore.transducer_depth = 1000 * store->sonardepth;
		oldstore.altitude = 1000 * store->altitude;

		/* get sidescan type */
		oldstore.ss_type = store->ss_type;
		}

	/* otherwise write curent version data, which is version 4
		if the number of beams and pixels is <= 32768 and version 5 if
		it is greater than 32768 */
	else if (store->kind == MB_DATA_DATA
		&& store->beams_bath <= 32768
		&& store->pixels_ss <= 32768)
		{
		*flag = MBF_MBLDEOIH_ID_DATA4;
		header_length = MBF_MBLDEOIH_V4HEADERSIZE;
		}
	else if (store->kind == MB_DATA_DATA)
		{
		*flag = MBF_MBLDEOIH_ID_DATA5;
		header_length = MBF_MBLDEOIH_V5HEADERSIZE;
		}

	/* otherwise write comment */
	else
		{
		*flag = MBF_MBLDEOIH_ID_COMMENT2;
		header_length = 2;
		}
#ifdef BYTESWAPPED
	*flag = mb_swap_short(*flag);
#endif

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New header set in function <%s>\n",function_name);
		fprintf(stderr,"dbg5       flag:       %d\n",*flag);
		}
	if (verbose >= 5 && store->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  Current version header values in function <%s>\n",function_name);
		fprintf(stderr,"dbg5       time_d:           %f\n",store->time_d);
		fprintf(stderr,"dbg5       longitude:        %f\n",store->longitude);
		fprintf(stderr,"dbg5       latitude:         %f\n",store->latitude);
		fprintf(stderr,"dbg5       sonardepth:       %f\n",store->sonardepth);
		fprintf(stderr,"dbg5       altitude:         %f\n",store->altitude);
		fprintf(stderr,"dbg5       heading:          %f\n",store->heading);
		fprintf(stderr,"dbg5       speed:            %f\n",store->speed);
		fprintf(stderr,"dbg5       roll:             %f\n",store->roll);
		fprintf(stderr,"dbg5       pitch:            %f\n",store->pitch);
		fprintf(stderr,"dbg5       heave:            %f\n",store->heave);
		fprintf(stderr,"dbg5       beam_xwidth:      %f\n",store->beam_xwidth);
		fprintf(stderr,"dbg5       beam_lwidth:      %f\n",store->beam_lwidth);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",store->beams_bath);
		fprintf(stderr,"dbg5       beams_amp:        %d\n",store->beams_amp);
		fprintf(stderr,"dbg5       pixels_ss:        %d\n",store->pixels_ss);
		fprintf(stderr,"dbg5       spare1:           %d\n",store->spare1);
		fprintf(stderr,"dbg5       depth_scale:      %f\n",store->depth_scale);
		fprintf(stderr,"dbg5       distance_scale:   %f\n",store->distance_scale);
		fprintf(stderr,"dbg5       ss_scalepower:    %d\n",store->ss_scalepower);
		fprintf(stderr,"dbg5       ss_type:          %d\n",store->ss_type);
		fprintf(stderr,"dbg5       spare3:           %d\n",store->imagery_type);
		fprintf(stderr,"dbg5       sonartype:        %d\n",store->topo_type);
		fprintf(stderr,"dbg5       status:           %d\n",status);
		fprintf(stderr,"dbg5       error:            %d\n",*error);
		}
	if (verbose >= 5 && store->kind == MB_DATA_DATA && *version == 2)
		{
		fprintf(stderr,"\ndbg5  Old version header values in function <%s>\n",function_name);
		fprintf(stderr,"dbg5       year:             %d\n",oldstore.year);
		fprintf(stderr,"dbg5       day:              %d\n",oldstore.day);
		fprintf(stderr,"dbg5       minute:           %d\n",oldstore.min);
		fprintf(stderr,"dbg5       second:           %d\n",oldstore.sec);
		fprintf(stderr,"dbg5       msec:             %d\n",oldstore.msec);
		fprintf(stderr,"dbg5       lonu:             %d\n",oldstore.lon2u);
		fprintf(stderr,"dbg5       lonb:             %d\n",oldstore.lon2b);
		fprintf(stderr,"dbg5       latu:             %d\n",oldstore.lat2u);
		fprintf(stderr,"dbg5       latb:             %d\n",oldstore.lat2b);
		fprintf(stderr,"dbg5       heading:          %d\n",oldstore.heading);
		fprintf(stderr,"dbg5       speed:            %d\n",oldstore.speed);
		fprintf(stderr,"dbg5       beams bath:       %d\n",oldstore.beams_bath);
		fprintf(stderr,"dbg5       beams amp:        %d\n",oldstore.beams_amp);
		fprintf(stderr,"dbg5       pixels ss:        %d\n",oldstore.pixels_ss);
		fprintf(stderr,"dbg5       depth scale:      %d\n",oldstore.depth_scale);
		fprintf(stderr,"dbg5       dist scale:       %d\n",oldstore.distance_scale);
		fprintf(stderr,"dbg5       transducer_depth: %d\n",oldstore.transducer_depth);
		fprintf(stderr,"dbg5       altitude:         %d\n",oldstore.altitude);
		fprintf(stderr,"dbg5       beam_xwidth:      %d\n",oldstore.beam_xwidth);
		fprintf(stderr,"dbg5       beam_lwidth:      %d\n",oldstore.beam_lwidth);
		fprintf(stderr,"dbg5       ss_type:          %d\n",oldstore.ss_type);
		fprintf(stderr,"dbg5       status:           %d\n",status);
		fprintf(stderr,"dbg5       error:            %d\n",*error);
		}

	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA)
		{
		/* if set, write old format - this should only happen for writing fbt files
			when the user has set fbtversion = old in the .mbio_defaults file
			using mbdefaults */
		if (mb_io_ptr->save1 == 2)
			{
			/* recalculate depth scaling so that it encompasses full bathymetry values, not
				just bathymetry relative to the sonar
				- to convert to old format add transducer depth to the bathymetry
				and reset the scaling */
			depthscale = 0.001 * oldstore.depth_scale;
			transducer_depth = 0.001 * oldstore.transducer_depth;
			depthmax = 0.0;
			for (i=0;i<oldstore.beams_bath;i++)
				{
				depthmax = MAX(depthmax, (depthscale * store->bath[i] + transducer_depth));
				}
			if (depthmax > 0.0)
				oldstore.depth_scale = MAX((int) (1 + depthmax / 30.0), 1);
			newdepthscale = 0.001 * oldstore.depth_scale;
			for (i=0;i<oldstore.beams_bath;i++)
				{
				store->bath[i] = (short int)((store->depth_scale * store->bath[i] + transducer_depth) / newdepthscale);
				}
			short_transducer_depth = (short)(oldstore.transducer_depth / oldstore.depth_scale);
			short_altitude = (short)(oldstore.altitude / oldstore.depth_scale);

			/* write old version header */
			index = 2;
			mb_put_binary_short(MB_NO, oldstore.year, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.day, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.min, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.sec, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.msec, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.lon2u, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.lon2b, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.lat2u, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.lat2b, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.heading, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.speed, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.beams_bath, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.beams_amp, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.pixels_ss, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.depth_scale, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.distance_scale, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, short_transducer_depth, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, short_altitude, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.beam_xwidth, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.beam_lwidth, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, oldstore.ss_type, (void *)  &buffer[index]); index +=2;
			}

		/* otherwise if reasonable number of beams then write version 4 record */
		else if (*flag == MBF_MBLDEOIH_ID_DATA4)
			{
			/* write current version header */
			index = 2;
			mb_put_binary_double(MB_NO, store->time_d, (void *)  &buffer[index]); index +=8;
			mb_put_binary_double(MB_NO, store->longitude, (void *)  &buffer[index]); index +=8;
			mb_put_binary_double(MB_NO, store->latitude, (void *)  &buffer[index]); index +=8;
			mb_put_binary_double(MB_NO, store->sonardepth, (void *)  &buffer[index]); index +=8;
			mb_put_binary_double(MB_NO, store->altitude, (void *)  &buffer[index]); index +=8;
			mb_put_binary_float(MB_NO, store->heading, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->speed, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->roll, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->pitch, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->heave, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->beam_xwidth, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->beam_lwidth, (void *)  &buffer[index]); index +=4;
			mb_put_binary_short(MB_NO, (short)store->beams_bath, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, (short)store->beams_amp, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, (short)store->pixels_ss, (void *)  &buffer[index]); index +=2;
			mb_put_binary_short(MB_NO, (short)store->spare1, (void *)  &buffer[index]); index +=2;
			mb_put_binary_float(MB_NO, store->depth_scale, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->distance_scale, (void *)  &buffer[index]); index +=4;
			buffer[index] = store->ss_scalepower; index++;
			buffer[index] = store->ss_type; index++;
			buffer[index] = store->imagery_type; index++;
			buffer[index] = store->topo_type; index++;
			}

		/* otherwise if unreasonable number of beams then write version 5 record */
		else
			{
			/* write current version header */
			index = 2;
			mb_put_binary_double(MB_NO, store->time_d, (void *)  &buffer[index]); index +=8;
			mb_put_binary_double(MB_NO, store->longitude, (void *)  &buffer[index]); index +=8;
			mb_put_binary_double(MB_NO, store->latitude, (void *)  &buffer[index]); index +=8;
			mb_put_binary_double(MB_NO, store->sonardepth, (void *)  &buffer[index]); index +=8;
			mb_put_binary_double(MB_NO, store->altitude, (void *)  &buffer[index]); index +=8;
			mb_put_binary_float(MB_NO, store->heading, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->speed, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->roll, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->pitch, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->heave, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->beam_xwidth, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->beam_lwidth, (void *)  &buffer[index]); index +=4;
			mb_put_binary_int(MB_NO, store->beams_bath, (void *)  &buffer[index]); index +=4;
			mb_put_binary_int(MB_NO, store->beams_amp, (void *)  &buffer[index]); index +=4;
			mb_put_binary_int(MB_NO, store->pixels_ss, (void *)  &buffer[index]); index +=4;
			mb_put_binary_int(MB_NO, store->spare1, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->depth_scale, (void *)  &buffer[index]); index +=4;
			mb_put_binary_float(MB_NO, store->distance_scale, (void *)  &buffer[index]); index +=4;
			buffer[index] = store->ss_scalepower; index++;
			buffer[index] = store->ss_type; index++;
			buffer[index] = store->imagery_type; index++;
			buffer[index] = store->topo_type; index++;
			}
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
		fprintf(stderr,"\ndbg5  Going to write data in function <%s>\n",function_name);
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
