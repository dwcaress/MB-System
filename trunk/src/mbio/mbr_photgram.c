/*--------------------------------------------------------------------
 *    The MB-system:	mbr_photgram.c	1/27/2014
 *	$Id$
 *
 *    Copyright (c) 2014-2014 by
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
 * mbr_photgram.c contains the functions for reading and writing
 * multibeam data in the PHOTGRAM format.
 * These functions include:
 *   mbr_alm_photgram	- allocate read/write memory
 *   mbr_dem_photgram	- deallocate read/write memory
 *   mbr_rt_photgram	- read and translate data
 *   mbr_wt_photgram	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	November 23, 2014
 *
 */
/*
 * Notes on the mbsys_stereopair data structure and associated format
 * mbf_photgram:
 *   1. This is an MB-System i/o module to read and write topography
 *      calculated by photogrammetry from stereo pair photographs.
 *   2. The structure in mbsys_stereopair.h defines the internal
 *      representation of photogrammetric topography data.
 *   3. The functions in mbsys_stereopair.c allow for extracting
 *      data from or inserting data into this internal
 *      representation. These functions are called by the
 *      MBIO API functions found in mbio/mb_access.c.
 *   4. The functions in mbr_photgram.c actually read and
 *      write the mbf_photgram format.
 *   5. Prototypes for all of the functions in mbsys_stereopair.c
 *      are provided in mbsys_stereopair.h.
 *   6. This list of functions corresponds to the function pointers
 *      that are included in the structure mb_io_struct that is defined
 *      in the file mbsystem/src/mbio/mb_io.h
 *      Not all of these functions are required - some only make sense to
 *      define if the relevant data type is part of the format. For instance,
 *      do not define mbsys_stereopair_extract_segy() if there are no subbottom
 *      profiler data supported by this data system
 *   7. The data are structured as deriving from a series of stereo pairs.
 *      The position and attitude of the camera rig are included, as is the
 *      position (relative to the camera) of each sounding derived from the
 *      stereo pair.
 *   8. Files in format mbf_photgram begin with the characters:
 *          ##PHOTGRAM##V001
 *      Following the 16-byte file header, the individual data records follow
 *      in any order. The defined record types include survey (MB_DATA_DATA),
 *      comment (MB_DATA_COMMENT), and INS (MB_DATA_NAV) which includes
 *      navigation, sensor depth, heading, and attitude sampled more frequently
 *      than the stereo photography.
 *
 *      Survey data records are binary with the following form:
 *              Number of bytes in record           4U
 *              Data record identifier              4U      (0x44445047 = "DDPG" = 1145327687)
 *              Time stamp (MB-System time_d)       8F      Decimal seconds since 1970/1/1/ 00:00:00
 *              Longitude                           8F      Decimal degrees
 *              Lattitude                           8F      Decimal degrees
 *              Sensor depth                        8F      Meters
 *              Heading                             4F      Decimal degrees
 *              Roll                                4F      Decimal degrees
 *              Pitch                               4F      Decimal degrees
 *              Speed                               4F      Decimal degrees
 *              Altitude                            4F      Decimal degrees
 *              N (Number of soundings)             4U
 *              ------------------------------------------------------------
 *              Repeat N times:
 *              ------------------------------------------------------------
 *              acrosstrack                         8F      meters
 *              alongtrack                          8F      meters
 *              depth                               8F      meters
 *              beamflag                            1U      beamflag
 *              red                                 1U      0-255
 *              green                               1U      0-255
 *              blue                                1U      0-255
 *              ------------------------------------------------------------
 *              End identifier                      4U      (0x454E4421 = "END!" = 1162757153)
 *              Check sum of data record between    2U
 *              and including the data record and
 *              end identifiers
 *
 *      INS data records are binary with the following form:
 *              Number of bytes in record           4U
 *              Data record identifier              4U      (0x4444494E = "DDIN" = 1145325902)
 *              Time stamp (MB-System time_d)       8F      Decimal seconds since 1970/1/1/ 00:00:00
 *              Longitude                           8F      Decimal degrees
 *              Lattitude                           8F      Decimal degrees
 *              Sensor depth                        8F      Meters
 *              Heading                             4F      Decimal degrees
 *              Roll                                4F      Decimal degrees
 *              Pitch                               4F      Decimal degrees
 *              Speed                               4F      Decimal degrees
 *              Altitude                            4F      Decimal degrees
 *              End identifier                      4U      (0x454E4421 = "END!" = 1162757153)
 *              Check sum of data record between    2U
 *              and including the data record and
 *              end identifiers
 *
 *      Comment data records are binary with the following form:
 *              Number of bytes in record           4U
 *              Data record identifier              4U      (0x4444434D = "DDCM" = 1145324365)
 *              Number of characters in comment     4U      Includes at least one terminating
 *                                                          null character, multiple of 4.
 *              Comment                             NC      
 *              End identifier                      4U      (0x454E4421 = "END!" = 1162757153)
 *              Check sum of data record between    2U
 *              and including the data record and
 *              end identifiers
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
#include "mbsys_stereopair.h"

/* include for byte swapping */
#include "mb_swap.h"

/* turn on debug statements here */
/* #define MBR_PHOTGRAM_DEBUG 1 */

/* essential function prototypes */
int mbr_register_photgram(int verbose, void *mbio_ptr,
		int *error);
int mbr_info_photgram(int verbose,
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
int mbr_alm_photgram(int verbose, void *mbio_ptr, int *error);
int mbr_dem_photgram(int verbose, void *mbio_ptr, int *error);
int mbr_rt_photgram(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_photgram(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_photgram_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_photgram_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_photgram(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_photgram";
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
	status = mbr_info_photgram(verbose,
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_photgram;
	mb_io_ptr->mb_io_format_free = &mbr_dem_photgram;
	mb_io_ptr->mb_io_store_alloc = &mbsys_stereopair_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_stereopair_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_photgram;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_photgram;
	mb_io_ptr->mb_io_dimensions = &mbsys_stereopair_dimensions;
	mb_io_ptr->mb_io_pingnumber = &mbsys_stereopair_pingnumber;
	mb_io_ptr->mb_io_sonartype = &mbsys_stereopair_sonartype;
	mb_io_ptr->mb_io_sidescantype = &mbsys_stereopair_sidescantype;
	mb_io_ptr->mb_io_extract = &mbsys_stereopair_extract;
	mb_io_ptr->mb_io_insert = &mbsys_stereopair_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_stereopair_extract_nav;
	mb_io_ptr->mb_io_extract_nnav = &mbsys_stereopair_extract_nnav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_stereopair_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_stereopair_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_stereopair_ttimes;
	mb_io_ptr->mb_io_detects = NULL;
	mb_io_ptr->mb_io_gains = NULL;
	mb_io_ptr->mb_io_copyrecord = &mbsys_stereopair_copy;
	mb_io_ptr->mb_io_extract_rawss = NULL;
	mb_io_ptr->mb_io_insert_rawss = NULL;
	mb_io_ptr->mb_io_extract_segytraceheader = NULL;
	mb_io_ptr->mb_io_extract_segy = NULL;
	mb_io_ptr->mb_io_insert_segy = NULL;
	mb_io_ptr->mb_io_ctd = NULL;
	mb_io_ptr->mb_io_ancilliarysensor = NULL;

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
		fprintf(stderr,"dbg2       extract_segytraceheader: %p\n",(void *)mb_io_ptr->mb_io_extract_segytraceheader);
		fprintf(stderr,"dbg2       extract_segy:       %p\n",(void *)mb_io_ptr->mb_io_extract_segy);
		fprintf(stderr,"dbg2       insert_segy:        %p\n",(void *)mb_io_ptr->mb_io_insert_segy);
		fprintf(stderr,"dbg2       copyrecord:         %p\n",(void *)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_photgram(int verbose,
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
	char	*function_name = "mbr_info_photgram";
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
	*system = MB_SYS_STEREOPAIR;
	*beams_bath_max = 0;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "PHOTGRAM", MB_NAME_LENGTH);
	strncpy(system_name, "STEREOPAIR", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_PHOTGRAM\nInformal Description: Example format\nAttributes:           Name the relevant sensor(s), \n                      what data types are supported\n                      how many beams and pixels, file type (ascii, binary, netCDF), Organization that defined this format.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_SINGLE;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
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
int mbr_alm_photgram(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_photgram";
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
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	status = mbsys_stereopair_alloc(
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
int mbr_dem_photgram(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_photgram";
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

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mbsys_stereopair_deall(
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
int mbr_rt_photgram(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_photgram";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_stereopair_struct *store;

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

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

#ifdef MBR_PHOTGRAM_DEBUG
	fprintf(stderr,"About to call mbr_photgram_rd_data...\n");
#endif

	/* read next data from file */
	status = mbr_photgram_rd_data(verbose,mbio_ptr,store_ptr,error);

	/* get pointers to data structures */
	store = (struct mbsys_stereopair_struct *) store_ptr;

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

#ifdef MBR_PHOTGRAM_DEBUG
	fprintf(stderr,"Done with mbr_photgram_rd_data: status:%d error:%d record kind:%d\n", status, *error, store->kind);
#endif

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
int mbr_wt_photgram(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_photgram";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_stereopair_struct *store;

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

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_stereopair_struct *) store_ptr;

#ifdef MBR_PHOTGRAM_DEBUG
	fprintf(stderr,"About to call mbr_photgram_wr_data record kind:%d\n", store->kind);
#endif

	/* write next data to file */
	status = mbr_photgram_wr_data(verbose,mbio_ptr,store_ptr,error);

#ifdef MBR_PHOTGRAM_DEBUG
	fprintf(stderr,"Done with mbr_photgram_wr_data: status:%d error:%d\n", status, *error);
#endif

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
int mbr_photgram_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_photgram_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_stereopair_struct *store;
	struct mbsys_stereopair_sounding_struct *sounding;
	char	buffer[MB_COMMENT_MAXLINE+8];
	size_t	read_len;
	int	recordsize;
	short	checksum;
	int	*fileheader_initialized;
	int	*formatversion;
	int	swap = MB_YES;
	int	index;
	int	skip;
	int	i, n;

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

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_stereopair_struct *) store_ptr;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
	
	/* get saved values */
	fileheader_initialized = (int *) &mb_io_ptr->save1;
	formatversion = (int *) &mb_io_ptr->save2;
	
	/* read file header if necessary */
	if (*fileheader_initialized == MB_NO)
		{
		read_len = 16;
		buffer[read_len] = '\0';
		status = mb_fileio_get(verbose, mbio_ptr, (char *)buffer, &read_len, error);
		if (strncmp(buffer, "##PHOTGRAM##V", 13) == 0)
			{
			n = sscanf(buffer, "##PHOTGRAM##V%d", formatversion);
			*fileheader_initialized = MB_YES;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_FORMAT;
			}
		}

	/* read the next record header */
	read_len = 8;
	status = mb_fileio_get(verbose, mbio_ptr, (char *)buffer, &read_len, error);

	/* check for valid record, loop over reading bytes until a valid
		record label is found or the read fails */
	skip = 0;
	while (status == MB_SUCCESS && strncmp(&buffer[4], "DD", 2) != 0)
		{
		for (i=0;i<7;i++)
			buffer[i] = buffer[i+1];
		read_len = 1;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)&buffer[7], &read_len, error);
		skip++;
		}
		
	/* if a valid record label has been found then read and parse it */
	if (status == MB_SUCCESS)
		{
		/* get the record size */
		mb_get_binary_int(swap, &buffer[0], &recordsize);
		
		/* read a survey record */
		if (strncmp(&buffer[4], "DDPG", 4) == 0)
			{
			store->kind = MB_DATA_DATA;
			
			/* read the record header */
			read_len = MBSYS_STEREOPAIR_HEADER_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)buffer, &read_len, error);
			index = 0;
			
			/* parse the record header */
			mb_get_binary_double(swap, &buffer[index], &store->time_d); index += 8;
			mb_get_date(verbose, store->time_d, store->time_i);
			mb_get_binary_double(swap, &buffer[index], &store->longitude); index += 8;
			mb_get_binary_double(swap, &buffer[index], &store->latitude); index += 8;
			mb_get_binary_double(swap, &buffer[index], &store->sensordepth); index += 8;
			mb_get_binary_float(swap, &buffer[index], &store->heading); index += 4;
			mb_get_binary_float(swap, &buffer[index], &store->roll); index += 4;
			mb_get_binary_float(swap, &buffer[index], &store->pitch); index += 4;
			mb_get_binary_float(swap, &buffer[index], &store->speed); index += 4;
			mb_get_binary_float(swap, &buffer[index], &store->altitude); index += 4;
			mb_get_binary_int(swap, &buffer[index], &store->num_soundings); index += 4;

			/* output debug information */
			if (verbose >= 4)
				{
				fprintf(stderr,"dbg4   DDPG Survey Record just read:\n");
				fprintf(stderr,"dbg4     recordsize:                 %d\n",recordsize);
				fprintf(stderr,"dbg4     kind:                       %d\n",store->kind);
				fprintf(stderr,"dbg4     time_d:                     %f\n",store->time_d);
				fprintf(stderr,"dbg4     longitude:                  %f\n",store->longitude);
				fprintf(stderr,"dbg4     latitude:                   %f\n",store->latitude);
				fprintf(stderr,"dbg4     sensordepth:                %f\n",store->sensordepth);
				fprintf(stderr,"dbg4     heading:                    %f\n",store->heading);
				fprintf(stderr,"dbg4     roll:                       %f\n",store->roll);
				fprintf(stderr,"dbg4     pitch:                      %f\n",store->pitch);
				fprintf(stderr,"dbg4     speed:                      %f\n",store->speed);
				fprintf(stderr,"dbg4     altitude:                   %f\n",store->altitude);
				fprintf(stderr,"dbg4     num_soundings:              %d\n",store->num_soundings);
				fprintf(stderr,"dbg4     num_soundings_alloc:        %d\n",store->num_soundings_alloc);
				}
			
			/* allocated memory to hold the soundings if necessary */
			if (store->num_soundings_alloc < store->num_soundings)
				{
				/* allocate memory for data structure */
				status = mb_reallocd(verbose, __FILE__, __LINE__,
							store->num_soundings * sizeof(struct mbsys_stereopair_sounding_struct),
							(void **)(&store->soundings), error);
				if (status == MB_SUCCESS)
					store->num_soundings_alloc = store->num_soundings;
				else
					store->num_soundings_alloc = 0;
				}

			/* output debug information */
			if (verbose >= 4)
				{
				fprintf(stderr,"dbg4     num_soundings_alloc:        %d\n",store->num_soundings_alloc);
				}
				
			/* read in the soundings */
			for (i=0; i< store->num_soundings; i++)
				{
				sounding = &store->soundings[i];
				
				/* read the sounding */
				read_len = MBSYS_STEREOPAIR_SOUNDING_SIZE;
				status = mb_fileio_get(verbose, mbio_ptr, (char *)buffer, &read_len, error);
				
				/* parse the sounding */
				index = 0;
				mb_get_binary_double(swap, &buffer[index], &sounding->acrosstrack); index += 8;
				mb_get_binary_double(swap, &buffer[index], &sounding->alongtrack); index += 8;
				mb_get_binary_double(swap, &buffer[index], &sounding->depth); index += 8;
				sounding->beamflag = buffer[index]; index++;
				sounding->red = buffer[index]; index++;
				sounding->green = buffer[index]; index++;
				sounding->blue = buffer[index]; index++;

				/* output debug information */
				if (verbose >= 4)
					{
					fprintf(stderr,"dbg4     %10d  %10g  %10g  %10g %x   %3d %3d %3d\n",
						i,sounding->acrosstrack,sounding->alongtrack,sounding->depth,sounding->beamflag,
						sounding->red,sounding->green,sounding->blue);
					}
				}
				
			/* read the end identifier and checksum */
			read_len = 6;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)&buffer, &read_len, error);
				
			/* parse the end identifier and checksum */
			if (strncmp(buffer, "END!", 4) != 0)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				}
			else
				{
				mb_get_binary_short(swap, &buffer[4], &checksum);
				}

			/* output debug information */
			if (verbose >= 4)
				{
				fprintf(stderr,"dbg4     end identifier:             %c%c%c%c\n",buffer[0],buffer[1],buffer[2],buffer[3]);
				fprintf(stderr,"dbg4     checksum:                   %d\n",checksum);
				}
			}
		
		/* read an ins record */
		else if (strncmp(&buffer[4], "DDIN", 4) == 0)
			{
			store->kind = MB_DATA_NAV;
			
			/* read the record header */
			read_len = MBSYS_STEREOPAIR_INS_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)buffer, &read_len, error);
			index = 0;
			
			/* parse the record header */
			mb_get_binary_double(swap, &buffer[index], &store->time_d); index += 8;
			mb_get_date(verbose, store->time_d, store->time_i);
			mb_get_binary_double(swap, &buffer[index], &store->longitude); index += 8;
			mb_get_binary_double(swap, &buffer[index], &store->latitude); index += 8;
			mb_get_binary_double(swap, &buffer[index], &store->sensordepth); index += 8;
			mb_get_binary_float(swap, &buffer[index], &store->heading); index += 4;
			mb_get_binary_float(swap, &buffer[index], &store->roll); index += 4;
			mb_get_binary_float(swap, &buffer[index], &store->pitch); index += 4;
			mb_get_binary_float(swap, &buffer[index], &store->speed); index += 4;
			mb_get_binary_float(swap, &buffer[index], &store->altitude); index += 4;
			
			/* read the end identifier and checksum */
			read_len = 6;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)&buffer, &read_len, error);
				
			/* parse the end identifier and checksum */
			if (strncmp(buffer, "END!", 4) != 0)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				}
			else
				{
				mb_get_binary_short(swap, &buffer[4], &checksum);
				}

			/* output debug information */
			if (verbose >= 4)
				{
				fprintf(stderr,"dbg4   DDIN Survey Record just read:\n");
				fprintf(stderr,"dbg4     recordsize:                 %d\n",recordsize);
				fprintf(stderr,"dbg4     kind:                       %d\n",store->kind);
				fprintf(stderr,"dbg4     time_d:                     %f\n",store->time_d);
				fprintf(stderr,"dbg4     longitude:                  %f\n",store->longitude);
				fprintf(stderr,"dbg4     latitude:                   %f\n",store->latitude);
				fprintf(stderr,"dbg4     sensordepth:                %f\n",store->sensordepth);
				fprintf(stderr,"dbg4     heading:                    %f\n",store->heading);
				fprintf(stderr,"dbg4     roll:                       %f\n",store->roll);
				fprintf(stderr,"dbg4     pitch:                      %f\n",store->pitch);
				fprintf(stderr,"dbg4     speed:                      %f\n",store->speed);
				fprintf(stderr,"dbg4     altitude:                   %f\n",store->altitude);
				fprintf(stderr,"dbg4     end identifier:             %c%c%c%c\n",buffer[0],buffer[1],buffer[2],buffer[3]);
				fprintf(stderr,"dbg4     checksum:                   %d\n",checksum);
				}
			}
		
		/* read a comment record */
		else if (strncmp(&buffer[4], "DDCM", 4) == 0)
			{
			store->kind = MB_DATA_COMMENT;
			
			/* read the comment length */
			read_len = 4;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)&store->comment_len, &read_len, error);
			
			/* read the comment */
			read_len = store->comment_len;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)store->comment, &read_len, error);
			
			/* read the end identifier and checksum */
			read_len = 6;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)&buffer, &read_len, error);
				
			/* parse the end identifier and checksum */
			if (strncmp(buffer, "END!", 4) != 0)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				}
			else
				{
				mb_get_binary_short(swap, &buffer[4], &checksum);
				}

			/* output debug information */
			if (verbose >= 4)
				{
				fprintf(stderr,"dbg4   DDCM Survey Record just read:\n");
				fprintf(stderr,"dbg4     recordsize:                 %d\n",recordsize);
				fprintf(stderr,"dbg4     kind:                       %d\n",store->kind);
				fprintf(stderr,"dbg4     comment_len:                %d\n",store->comment_len);
				fprintf(stderr,"dbg4     comment:                    %s\n",store->comment);
				fprintf(stderr,"dbg4     end identifier:             %c%c%c%c\n",buffer[0],buffer[1],buffer[2],buffer[3]);
				fprintf(stderr,"dbg4     checksum:                   %d\n",checksum);
				}
			}
		}

	/* get file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

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
int mbr_photgram_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_photgram_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_stereopair_struct *store;
	struct mbsys_stereopair_sounding_struct *sounding;
	char	buffer[MB_COMMENT_MAXLINE+8];
	size_t	write_len;
	int	checksum = 0;
	int	*fileheader_initialized;
	int	*formatversion;
	int	swap = MB_YES;
	int	index;
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

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_stereopair_struct *) store_ptr;
	
	/* get saved values */
	fileheader_initialized = (int *) &mb_io_ptr->save1;
	formatversion = (int *) &mb_io_ptr->save2;
	
	/* write file header if necessary */
	if (*fileheader_initialized == MB_NO)
		{
		sprintf(buffer, "##PHOTGRAM##V001");
		write_len = 16;
		buffer[write_len] = '\0';
		status = mb_fileio_put(verbose, mbio_ptr, (char *)buffer, &write_len, error);
		if (status == MB_SUCCESS)
			{
			*fileheader_initialized = MB_YES;
			}
		}
		
	/* now write the data record */
	if (status == MB_SUCCESS)
		{
		/* write a survey record */
		if (store->kind == MB_DATA_DATA)
			{
			/* calculate full write length */
			write_len = 8 + MBSYS_STEREOPAIR_HEADER_SIZE
					+ store->num_soundings * MBSYS_STEREOPAIR_SOUNDING_SIZE
					+ 6;
			
			/* output debug information */
			if (verbose >= 4)
				{
				fprintf(stderr,"dbg4   DDPG Survey Record to be written:\n");
				fprintf(stderr,"dbg4     write_len:                  %zu\n",write_len);
				fprintf(stderr,"dbg4     kind:                       %d\n",store->kind);
				fprintf(stderr,"dbg4     time_d:                     %f\n",store->time_d);
				fprintf(stderr,"dbg4     longitude:                  %f\n",store->longitude);
				fprintf(stderr,"dbg4     latitude:                   %f\n",store->latitude);
				fprintf(stderr,"dbg4     sensordepth:                %f\n",store->sensordepth);
				fprintf(stderr,"dbg4     heading:                    %f\n",store->heading);
				fprintf(stderr,"dbg4     roll:                       %f\n",store->roll);
				fprintf(stderr,"dbg4     pitch:                      %f\n",store->pitch);
				fprintf(stderr,"dbg4     speed:                      %f\n",store->speed);
				fprintf(stderr,"dbg4     altitude:                   %f\n",store->altitude);
				fprintf(stderr,"dbg4     num_soundings:              %d\n",store->num_soundings);
				fprintf(stderr,"dbg4     num_soundings_alloc:        %d\n",store->num_soundings_alloc);
				fprintf(stderr,"dbg4     num_soundings_alloc:        %d\n",store->num_soundings_alloc);
				//for (i=0; i< store->num_soundings; i++)
				for (i=0; i< 10; i++)
					{
					sounding = &store->soundings[i];
					fprintf(stderr,"dbg4     %10d  %10g  %10g  %10g %x   %3d %3d %3d\n",
						i,sounding->acrosstrack,sounding->alongtrack,sounding->depth,sounding->beamflag,
						sounding->red,sounding->green,sounding->blue);
					}
				}

			/* insert and write the header values */
			index = 0;
			mb_put_binary_int(swap, write_len, (void *)&buffer[index]); index += 4;
			strncpy(&buffer[index], "DDPG", 4); index += 4;
			mb_put_binary_double(swap, store->time_d, (void *)&buffer[index]); index += 8;
			mb_put_binary_double(swap, store->longitude, (void *)&buffer[index]); index += 8;
			mb_put_binary_double(swap, store->latitude, (void *)&buffer[index]); index += 8;
			mb_put_binary_double(swap, store->sensordepth, (void *)&buffer[index]); index += 8;
			mb_put_binary_float(swap, store->heading, (void *)&buffer[index]); index += 4;
			mb_put_binary_float(swap, store->roll, (void *)&buffer[index]); index += 4;
			mb_put_binary_float(swap, store->pitch, (void *)&buffer[index]); index += 4;
			mb_put_binary_float(swap, store->speed, (void *)&buffer[index]); index += 4;
			mb_put_binary_float(swap, store->altitude, (void *)&buffer[index]); index += 4;
			mb_put_binary_int(swap, store->num_soundings, (void *)&buffer[index]); index += 4;
			write_len = index;
			status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
				
			/* write out the soundings */
			for (i=0; i< store->num_soundings; i++)
				{
				/* insert the sounding */
				sounding = &store->soundings[i];
				index = 0;
				mb_put_binary_double(swap, sounding->acrosstrack, (void *)&buffer[index]); index += 8;
				mb_put_binary_double(swap, sounding->alongtrack, (void *)&buffer[index]); index += 8;
				mb_put_binary_double(swap, sounding->depth, (void *)&buffer[index]); index += 8;
				buffer[index] = sounding->beamflag; index++;
				buffer[index] = sounding->red; index++;
				buffer[index] = sounding->green; index++;
				buffer[index] = sounding->blue; index++;
				write_len = index;
				status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
				}

			/* write the end identifier and checksum */
			index = 0;
			strncpy(&buffer[index], "END!", 4); index += 4;
			mb_put_binary_short(swap, checksum, (void *)&buffer[index]); index += 2;
			write_len = index;
			status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
			}
			
		/* write an ins record */
		else if (store->kind == MB_DATA_NAV)
			{
			/* calculate full write length */
			write_len = 8 + MBSYS_STEREOPAIR_INS_SIZE + 6;
			
			/* output debug information */
			if (verbose >= 4)
				{
				fprintf(stderr,"dbg4   DDIN Survey Record to be written:\n");
				fprintf(stderr,"dbg4     write_len:                  %zu\n",write_len);
				fprintf(stderr,"dbg4     kind:                       %d\n",store->kind);
				fprintf(stderr,"dbg4     time_d:                     %f\n",store->time_d);
				fprintf(stderr,"dbg4     longitude:                  %f\n",store->longitude);
				fprintf(stderr,"dbg4     latitude:                   %f\n",store->latitude);
				fprintf(stderr,"dbg4     sensordepth:                %f\n",store->sensordepth);
				fprintf(stderr,"dbg4     heading:                    %f\n",store->heading);
				fprintf(stderr,"dbg4     roll:                       %f\n",store->roll);
				fprintf(stderr,"dbg4     pitch:                      %f\n",store->pitch);
				fprintf(stderr,"dbg4     speed:                      %f\n",store->speed);
				fprintf(stderr,"dbg4     altitude:                   %f\n",store->altitude);
				}

			/* insert and write the header values */
			index = 0;
			mb_put_binary_int(swap, write_len, (void *)&buffer[index]); index += 4;
			strncpy(&buffer[index], "DDIN", 4); index += 4;
			mb_put_binary_double(swap, store->time_d, (void *)&buffer[index]); index += 8;
			mb_put_binary_double(swap, store->longitude, (void *)&buffer[index]); index += 8;
			mb_put_binary_double(swap, store->latitude, (void *)&buffer[index]); index += 8;
			mb_put_binary_double(swap, store->sensordepth, (void *)&buffer[index]); index += 8;
			mb_put_binary_float(swap, store->heading, (void *)&buffer[index]); index += 4;
			mb_put_binary_float(swap, store->roll, (void *)&buffer[index]); index += 4;
			mb_put_binary_float(swap, store->pitch, (void *)&buffer[index]); index += 4;
			mb_put_binary_float(swap, store->speed, (void *)&buffer[index]); index += 4;
			mb_put_binary_float(swap, store->altitude, (void *)&buffer[index]); index += 4;
			strncpy(&buffer[index], "END!", 4); index += 4;
			mb_put_binary_short(swap, checksum, (void *)&buffer[index]); index += 2;
			write_len = index;
			status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
			}
			
		/* write a comment record */
		else if (store->kind == MB_DATA_COMMENT)
			{
			/* calculate full write length */
			write_len = 12 + store->comment_len + 6;
			
			/* output debug information */
			if (verbose >= 4)
				{
				fprintf(stderr,"dbg4   DDCM Survey Record to be written:\n");
				fprintf(stderr,"dbg4     write_len:                  %zu\n",write_len);
				fprintf(stderr,"dbg4     kind:                       %d\n",store->kind);
				fprintf(stderr,"dbg4     comment_len:                %d\n",store->comment_len);
				fprintf(stderr,"dbg4     comment:                    %s\n",store->comment);
				}

			/* insert and write the header values */
			index = 0;
			mb_put_binary_int(swap, write_len, (void *)&buffer[index]); index += 4;
			strncpy(&buffer[index], "DDCM", 4); index += 4;
			mb_put_binary_int(swap, store->comment_len, (void *)&buffer[index]); index += 4;
			strncpy(&buffer[index], store->comment, store->comment_len); index += store->comment_len;
			strncpy(&buffer[index], "END!", 4); index += 4;
			mb_put_binary_short(swap, checksum, (void *)&buffer[index]); index += 2;
			write_len = index;
			status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
			}
		}

#ifdef MBR_PHOTGRAM_DEBUG
	fprintf(stderr,"PHOTGRAM DATA WRITTEN: type:%d status:%d error:%d\n\n",
	store->kind, status, *error);
#endif

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
