/*--------------------------------------------------------------------
 *    The MB-system:    mbr_swplssxp.c  5/6/2013
 *  $Id$
 *
 *    Copyright (c) 2013-2014 by
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
 * mbr_swplssxp.c contains the functions for reading and writing
 * interferometric sonar data in the MBF_SWPLSSXP format.
 * These functions include:
 *   mbr_alm_swplssxp   - allocate read/write memory
 *   mbr_dem_swplssxp   - deallocate read/write memory
 *   mbr_rt_swplssxp    - read and translate data
 *   mbr_wt_swplssxp    - translate and write data
 *
 * Author:  D. P. Finlayson and D. W. Caress
 * Date:    May 6, 2013
 *
 * $Log: mbr_swplssxp.c,v $
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mbsys_swathplus.h"

/* include for byte swapping */
#include "mb_swap.h"

/* turn on debug statements here */
#define MBF_SWPLSSXP_DEBUG 1

#define OUT 0
#define IN 1

/* Private structs needed for doing coordinate transformations */
typedef struct swplssxp_vector_struct
	{
	double x;
	double y;
	double z;
	} swplssxp_vector;

typedef struct euler_angles_struct
	{
	double heading;
	double pitch;
	double roll;
	} swplssxp_angles;

typedef struct quaternion_struct
	{
	double w;
	double x;
	double y;
	double z;
	} swplssxp_quaternion;

typedef struct matrix_struct
	{
	double m11, m12, m13;
	double m21, m22, m23;
	double m31, m32, m33;
	double tx, ty, tz;
	} swplssxp_matrix;

/* essential function prototypes */
int mbr_register_swplssxp(int verbose, void *mbio_ptr, int *error);
int mbr_info_swplssxp(int verbose,
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
int mbr_alm_swplssxp(int verbose, void *mbio_ptr, int *error);
int mbr_dem_swplssxp(int verbose, void *mbio_ptr, int *error);
int mbr_rt_swplssxp(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_swplssxp(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_swplssxp_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);

/* private helper functions */
static int mbr_swplssxp_rd_blockhead(int verbose,
	void *mb_io_ptr,
	int *recordid,
	int *size,
	int *error);
static int mbr_swplssxp_rd_header(int verbose, void *mb_io_ptr, swplssxp_header *header,
	int *error);
static int mbr_swplssxp_rd_ping(int verbose,
	void *mb_io_ptr,
	int type,
	swplssxp_ping *ping,
	int *error);
static int mbr_swplssxp_interp(int verbose,
	double x1,
	double y1,
	double x2,
	double y2,
	double x,
	double *y,
	int *error);
static int mbr_swplssxp_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
static int mbr_swplssxp_wr_header(int verbose, void *mbio_ptr, swplssxp_header *header, int *error);
static int mbr_swplssxp_wr_ping(int verbose, void *mbio_ptr, swplssxp_ping *ping, int *error);
static int mbr_swplssxp_get_beamwidth(int verbose,
	double frequency,
	double *bwxtrack,
	double *bwltrack,
	int *error);
static int mbr_swplssxp_transform(int verbose, swplssxp_matrix *m, swplssxp_vector *v, int *error);
static int mbr_swplssxp_setup_vessel_to_world_matrix(int verbose,
	swplssxp_vector *position,
	swplssxp_angles *orientation,
	swplssxp_matrix *m,
	int *error);
static void mbr_swplssxp_setup_world_to_vessel_matrix(
	int verbose,
	swplssxp_vector *position,
	swplssxp_angles *orientation,
	swplssxp_matrix *m,
	int *error);
static int mbr_swplssxp_angles_to_quaternion(
	int verbose, swplssxp_angles *orientation, swplssxp_quaternion *q, int *error);
static int mbr_swplssxp_quaternion_to_angles(
	int verbose, swplssxp_quaternion *q, swplssxp_angles *orientation, int *error);
static int mbr_swplssxp_slerp(
	int verbose,
	swplssxp_quaternion *q0,
	swplssxp_quaternion *q1,
	double t,
	swplssxp_quaternion *q,
	int *error);


static char rcs_id[]="$Id$";

/*----------------------------------------------------------------------
    mbr_register_swplssxp

    Register the SWATHplus-specific information parameters and the format
    specific function pointers into the MB System MBIO structureB.

    HISTORY:

    2013-08-09 - Defined function for SWATHplus - DPF
   ----------------------------------------------------------------------*/
int mbr_register_swplssxp
(
	int verbose,		/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,		/* in: see mb_io.h:0+/mb_io_struct/ */
	int *error			/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char    *function_name = "mbr_register_swplssxp";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	assert(mbio_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		}

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set format info parameters */
	status = mbr_info_swplssxp(verbose,
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_swplssxp;
	mb_io_ptr->mb_io_format_free = &mbr_dem_swplssxp;
	mb_io_ptr->mb_io_store_alloc = &mbsys_swathplus_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_swathplus_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_swplssxp;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_swplssxp;
	mb_io_ptr->mb_io_dimensions = &mbsys_swathplus_dimensions;
	mb_io_ptr->mb_io_pingnumber = &mbsys_swathplus_pingnumber;
	mb_io_ptr->mb_io_extract = &mbsys_swathplus_extract;
	mb_io_ptr->mb_io_insert = &mbsys_swathplus_insert;
	mb_io_ptr->mb_io_extract_nnav = &mbsys_swathplus_extract_nnav;
	mb_io_ptr->mb_io_extract_nav = &mbsys_swathplus_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_swathplus_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_swathplus_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = &mbsys_swathplus_extract_svp;
	mb_io_ptr->mb_io_insert_svp = &mbsys_swathplus_insert_svp;
	mb_io_ptr->mb_io_ttimes = &mbsys_swathplus_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_swathplus_detects;
	mb_io_ptr->mb_io_pulses = &mbsys_swathplus_pulses;
	mb_io_ptr->mb_io_gains = &mbsys_swathplus_gains;
	mb_io_ptr->mb_io_copyrecord = &mbsys_swathplus_copy;
	mb_io_ptr->mb_io_extract_rawss = NULL;
	mb_io_ptr->mb_io_insert_rawss = NULL;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       system:             %d\n", mb_io_ptr->system);
		fprintf(stderr, "dbg2       beams_bath_max:     %d\n", mb_io_ptr->beams_bath_max);
		fprintf(stderr, "dbg2       beams_amp_max:      %d\n", mb_io_ptr->beams_amp_max);
		fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", mb_io_ptr->pixels_ss_max);
		fprintf(stderr, "dbg2       format_name:        %s\n", mb_io_ptr->format_name);
		fprintf(stderr, "dbg2       system_name:        %s\n", mb_io_ptr->system_name);
		fprintf(stderr, "dbg2       format_description: %s\n", mb_io_ptr->format_description);
		fprintf(stderr, "dbg2       numfile:            %d\n", mb_io_ptr->numfile);
		fprintf(stderr, "dbg2       filetype:           %d\n", mb_io_ptr->filetype);
		fprintf(stderr, "dbg2       variable_beams:     %d\n", mb_io_ptr->variable_beams);
		fprintf(stderr, "dbg2       traveltime:         %d\n", mb_io_ptr->traveltime);
		fprintf(stderr, "dbg2       beam_flagging:      %d\n", mb_io_ptr->beam_flagging);
		fprintf(stderr, "dbg2       nav_source:         %d\n", mb_io_ptr->nav_source);
		fprintf(stderr, "dbg2       heading_source:     %d\n", mb_io_ptr->heading_source);
		fprintf(stderr, "dbg2       vru_source:         %d\n", mb_io_ptr->vru_source);
		fprintf(stderr, "dbg2       svp_source:         %d\n", mb_io_ptr->svp_source);
		fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", mb_io_ptr->beamwidth_xtrack);
		fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", mb_io_ptr->beamwidth_ltrack);
		fprintf(stderr, "dbg2       format_alloc:       %p\n", mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr, "dbg2       format_free:        %p\n", mb_io_ptr->mb_io_format_free);
		fprintf(stderr, "dbg2       store_alloc:        %p\n", mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr, "dbg2       store_free:         %p\n", mb_io_ptr->mb_io_store_free);
		fprintf(stderr, "dbg2       read_ping:          %p\n", mb_io_ptr->mb_io_read_ping);
		fprintf(stderr, "dbg2       write_ping:         %p\n", mb_io_ptr->mb_io_write_ping);
		fprintf(stderr, "dbg2       extract:            %p\n", mb_io_ptr->mb_io_extract);
		fprintf(stderr, "dbg2       insert:             %p\n", mb_io_ptr->mb_io_insert);
		fprintf(stderr, "dbg2       extract_nav:        %p\n", mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr, "dbg2       insert_nav:         %p\n", mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr, "dbg2       extract_altitude:   %p\n", mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr, "dbg2       insert_altitude:    %p\n", mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr, "dbg2       extract_svp:        %p\n", mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr, "dbg2       insert_svp:         %p\n", mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr, "dbg2       ttimes:             %p\n", mb_io_ptr->mb_io_ttimes);
		fprintf(stderr, "dbg2       detects:            %p\n", mb_io_ptr->mb_io_detects);
		fprintf(stderr, "dbg2       pulses:             %p\n", mb_io_ptr->mb_io_pulses);
		fprintf(stderr, "dbg2       extract_rawss:      %p\n", mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr, "dbg2       insert_rawss:       %p\n", mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr, "dbg2       copyrecord:         %p\n", mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr, "dbg2       error:              %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:         %d\n", status);
		}

	/* return status */
	return status;
}								/* mbr_register_swplssxp */
/*----------------------------------------------------------------------
    mbr_info_swplssxp

    Define the SWATHplus SXP file format and format-specific
    capabilities.

    HISTORY:

    2013-08-09 - Defined function for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbr_info_swplssxp
(
	int verbose,				/* in: verbosity level set at command line 0..N */
	int *system,				/* out: what is the MB System ID for this format? */
	int *beams_bath_max,		/* out: maximum number of bathymetry beams */
	int *beams_amp_max,			/* out: maximum number of amplitude beams (usually same as bathy) */
	int *pixels_ss_max,			/* out: maximum number of sidescan pixles */
	char *format_name,			/* out: file format string */
	char *system_name,			/* out: sonar system name */
	char *format_description,	/* out: format description */
	int *numfile,				/* out: number of files in the format */
	int *filetype,				/* out: file type code */
	int *variable_beams,		/* out: does the sonar have variable number of beams? */
	int *traveltime,			/* out: ?? TODO: what does this parameter set? */
	int *beam_flagging,			/* out: does the format handle beam flagging? */
	int *nav_source,			/* out: which packet kind has navigation? */
	int *heading_source,		/* out: which packet kind has heading? */
	int *vru_source,			/* out: which packet kind has attitude? */
	int *svp_source,			/* out: which packet kind has sound velocity profiles? */
	double *beamwidth_xtrack,	/* out: what is the across-track beam width? */
	double *beamwidth_ltrack,	/* out: what is the along-track beam width? */
	int *error					/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char    *function_name = "mbr_info_swplssxp";
	int status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_SWATHPLUS;
	*beams_bath_max = MBSYS_SWPLS_MAX_BEAMS;
	*beams_amp_max = MBSYS_SWPLS_MAX_BEAMS;
	*pixels_ss_max = MBSYS_SWPLS_MAX_PIXELS;
	strncpy(format_name, "SWPLSSXP", MB_NAME_LENGTH);
	strncpy(system_name, "SWATHPLUS", MB_NAME_LENGTH);
	strncpy(format_description,
		"Format name:          MBF_SWPLSSXP\nInformal Description: SEA interferometric sonar vendor processed data format\nAttributes:           SEA SWATHplus,\n                      bathymetry and amplitude,\n                      variable beams, binary, SEA.\n",
		MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_SINGLE;
	*variable_beams = MB_YES;
	*traveltime = MB_NO;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = SWPLS_TYPE_M_BEAM_WIDTH;
	*beamwidth_ltrack = SWPLS_TYPE_M_BEAM_WIDTH;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       system:             %d\n", *system);
		fprintf(stderr, "dbg2       beams_bath_max:     %d\n", *beams_bath_max);
		fprintf(stderr, "dbg2       beams_amp_max:      %d\n", *beams_amp_max);
		fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", *pixels_ss_max);
		fprintf(stderr, "dbg2       format_name:        %s\n", format_name);
		fprintf(stderr, "dbg2       system_name:        %s\n", system_name);
		fprintf(stderr, "dbg2       format_description: %s\n", format_description);
		fprintf(stderr, "dbg2       numfile:            %d\n", *numfile);
		fprintf(stderr, "dbg2       filetype:           %d\n", *filetype);
		fprintf(stderr, "dbg2       variable_beams:     %d\n", *variable_beams);
		fprintf(stderr, "dbg2       traveltime:         %d\n", *traveltime);
		fprintf(stderr, "dbg2       beam_flagging:      %d\n", *beam_flagging);
		fprintf(stderr, "dbg2       nav_source:         %d\n", *nav_source);
		fprintf(stderr, "dbg2       heading_source:     %d\n", *heading_source);
		fprintf(stderr, "dbg2       vru_source:         %d\n", *vru_source);
		fprintf(stderr, "dbg2       svp_source:         %d\n", *svp_source);
		fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", *beamwidth_xtrack);
		fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", *beamwidth_ltrack);
		fprintf(stderr, "dbg2       error:              %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:         %d\n", status);
		}

	/* return status */
	return status;
}					/* mbr_info_swplssxp */
/*--------------------------------------------------------------------
    mbr_alm_swplssxp

    Allocate and initialize memory for the for the internal data structure.
    This structure acts as a global variable pool used by many MB System functions.
    There are some open slots for storing system-specific state that will
    persist indefinately (the system-specific structure is re-initialized
    on each call to read new data, so you can't store it there), if you use
    them, storage for them needs to be allocated here. Remember to deallocate
    the memeory in mbr_dem_swplssxp

    History:

    2013-08-07 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbr_alm_swplssxp
(
	int verbose,	/* in: verbosity level set at command line 0..N */
	void *mbio_ptr,	/* in: see mb_io.h:mb_io_struct */
	int *error		/* out: see mb_status.h:MB_ERROR */
)
{
	char    *function_name = "mbr_alm_swplssxp";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	assert(mbio_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	status = mbsys_swathplus_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return status;
}					/* mbr_alm_swplssxp */
/*--------------------------------------------------------------------
    mbr_dem_swplssxp

    Deallocate memory from the MB System internal data store.

    History:

    2013-08-07 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbr_dem_swplssxp
(
	int verbose,	/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,	/* in: see mb_io.h:0+/mb_io_struct/ */
	int *error		/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char    *function_name = "mbr_dem_swplssxp";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	assert(mbio_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		}

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mbsys_swathplus_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:   %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return status;
}					/* mbr_dem_swplssxp */
/*--------------------------------------------------------------------
    mbr_rt_swplssxp

    Read and translate a single data block from the sonar-specific
    file and store the data in the appropriate MB System internal
    structure.

    SXP files are fully processed to ground coordinates. The information
    necessary to reverse this processing is not stored in the SXP file
    itself.  So, it is not safe to make any changes to the sample
    coordinates directly.  What I've set up here is a simple transformation
    from ground coordinates to vessel coordinates so that the data can be
    displayed, filtered and gridded in MB System tools.  Any other
    modifications to the data should be done up-stream of MB System.

    In most cases, the user would be better off processing SWATHplus data in
    SXI or GSF format.

    History:

    2013-08-07 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbr_rt_swplssxp
(
	int verbose,	/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,	/* in: see mb_io.h:0+/mb_io_struct/ */
	void *store_ptr,/* in: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int *error		/* out: see mb_status.h:MB_ERROR */
)
{
	char    *function_name = "mbr_rt_swplssxp";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	swplssxp_point *points;
	int i, nbath, namp, nss;
	swplssxp_vector ppos, zero;
	swplssxp_angles txatt;
	swplssxp_matrix wtov;
	double dx, dy, dt;

	int *warning_samps;
	int *spdtxer;
	double *speed;
	double *last_e;
	double *last_n;
	double *last_t;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		}

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *) store_ptr;
	if (verbose >= 4)
		mbsys_swathplus_print_store(verbose, store, error);

	/* get pointers to saved data */
	warning_samps = &(mb_io_ptr->save1);
	spdtxer = &(mb_io_ptr->save2);
	speed = &(mb_io_ptr->saved1);
	last_e = &(mb_io_ptr->saved2);
	last_n = &(mb_io_ptr->saved3);
	last_t = &(mb_io_ptr->saved4);
	*warning_samps = MB_NO;
	*spdtxer = -1;
	*speed = 0.0;
	*last_e = 0.0;
	*last_n = 0.0;
	*last_t = 0.0;

	/* check if projection has been set */
	if (store->projection_set == MB_NO && mb_io_ptr->projection_initialized == MB_YES)
		{
		store->projection_set = MB_YES;
		strcpy(store->projection_id, mb_io_ptr->projection_id);
		}

	/* read next data from file */
	status = mbr_swplssxp_rd_data(verbose, mbio_ptr, store_ptr, error);

	/* before processing, ensure that we have enough storage to hold all the points */
	if ((status == MB_SUCCESS) && (store->kind == MB_DATA_DATA))
		{
		if ((MBSYS_SWPLS_MAX_BEAMS <= store->ping.nosampsfile) && (*warning_samps == MB_NO))
			{
			fprintf(stderr, "\nWarning: MB System wasn't compiled with enough storage to hold\n");
			fprintf(stderr, "Warning: all the data stored in this ping!\n");
			fprintf(stderr, "Warning:\n");
			fprintf(stderr,
				"Warning: Please recompile with MBSYS_SWPLS_MAX_BEAMS >= %d!\n",
				store->ping.nosampsfile);
			fprintf(stderr,
				"Warning: MBSYS_SWPLS_MAX_BEAMS is currently set to %d samples.\n",
				MBSYS_SWPLS_MAX_BEAMS);
			fprintf(stderr,
				"Warning: Data will be truncated at sample number %d.\n",
				MBSYS_SWPLS_MAX_BEAMS);
			}
		*warning_samps = MB_YES;
		}

	/* Process ping data */
	if ((status == MB_SUCCESS) && (store->kind == MB_DATA_DATA))
		{
		/* translate time */

		/* Nudge time by a few msec for transducers > 1 so that simultaneous pings aren't
		   filtered out */
		store->time_d = store->ping.time_d + (store->ping.txno - 1) * 1e-5;
		mb_get_date(verbose, store->ping.time_d, store->time_i);

		/* translate transducer position */
		store->navlon = store->ping.txer_e;
		store->navlat = store->ping.txer_n;

		/* translate vessel speed */

		/* ... first txer detected will be the reference, following txers have duplicate times and
		   positions  */
		if (*spdtxer < 0)
			{
			*spdtxer = store->ping.txno;
			*last_e = store->ping.txer_e;
			*last_n = store->ping.txer_n;
			*last_t = store->ping.time_d;
			}

		/* ... only update speed when we see our reference transducer again */
		if (store->ping.txno == *spdtxer)
			{
			dx = store->ping.txer_e - *last_e;
			dy = store->ping.txer_n - *last_n;
			dt = store->ping.time_d - *last_t;
			*speed = sqrt(dx * dx + dy * dy) / dt;
			*last_e = store->ping.txer_e;
			*last_n = store->ping.txer_n;
			*last_t = store->ping.time_d;
			}

		store->speed = *speed;

		/* translate vessel attitude */
		store->heading = store->ping.heading;
		store->draft = 0.0;					/* sxp draft is included in ping.height */
		store->roll = -store->ping.roll;	/* sxp roll is positive stbd down (opposite) */
		store->pitch = store->ping.pitch;	/* sxp pitch is positive bow up (same) */
		store->heave = -store->ping.height;	/* sxp height is positive down (opposite) */

		/* translate environmental parameters */
		store->sos = store->ping.sos;

		/* translate beamwidth based on frequency of sonar */
		mbr_swplssxp_get_beamwidth(verbose, store->ping.frequency, &(store->beamwidth_xtrack),
			&(store->beamwidth_ltrack), error);

		/* ...translate SXP samples from real-world coordinates to vessel coordinates */
		zero.x = 0.0;
		zero.y = 0.0;
		zero.z = 0.0;

		txatt.heading = store->ping.heading * DTR;
		txatt.pitch = 0.0;
		txatt.roll = 0.0;

		points = store->ping.points;
		nbath = namp = nss = 0;
		for(i = 0; i < store->ping.nosampsfile; i++)
			{
			/* point position relative to transducer */
			ppos.x = points[i].x - store->ping.txer_e;
			ppos.y = -points[i].z;
			ppos.z = points[i].y - store->ping.txer_n;

			/* rotate point from world coordinates to vessel coordinates */
			mbr_swplssxp_setup_world_to_vessel_matrix(verbose, &zero, &txatt, &wtov, error);
			mbr_swplssxp_transform(verbose, &wtov, &ppos, error);

			if (points[i].status != SWPLS_POINT_REJECTED)
				store->beamflag[nbath] = MB_FLAG_NONE;
			else
				store->beamflag[nbath] = MB_FLAG_FLAG + MB_FLAG_FILTER;
			store->bath[nbath] = -ppos.y;
			store->bathacrosstrack[nbath] = ppos.x;
			store->bathalongtrack[nbath] = ppos.z;
			store->amp[nbath] = points[i].amp;
			nbath++;
			namp++;
			}

		store->nbath = nbath;
		store->namp = namp;
		store->nss = nss;
		}

	else if ((status == MB_SUCCESS) && (store->kind == MB_DATA_COMMENT))
		{
		status = MB_FAILURE;
		*error = MB_ERROR_COMMENT;
		}

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

	/* print output debug statements */
	if (verbose >= 4)
		mbsys_swathplus_print_store(verbose, store, error);
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return status;
}			/* mbr_rt_swplssxp */
/*--------------------------------------------------------------------
    mbr_wt_swplssxp

    Translate and write to the sonar-specific data store a single data block
    from the MB System internal structure to the appropriate sonar-specific
    data block.

    SXP files are fully processed to ground coordinates. The information
    necessary to reverse this processing is not stored in the SXP file
    itself.  So, it is not safe to make any changes to the sample
    coordinates directly.  The only modifications to the original data that
    will be persisted in this function are changes to the beam flags and
    amplitude data.

    In most cases, the user would be better off processing SWATHplus data in
    SXI or GSF format.

    History:

    2013-08-07 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbr_wt_swplssxp
(
	int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *error
)
{
	char    *function_name = "mbr_wt_swplssxp";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	swplssxp_point *points;
	swplssxp_angles txatt;
	swplssxp_vector zero, ppos;
	swplssxp_matrix vtow;
	int *projection_file_created;
	mb_path	projection_file;
	FILE *pfp;
	int i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *) store_ptr;
	if (verbose >= 4)
		mbsys_swathplus_print_store(verbose, store, error);

	/* get pointers to saved data */
	projection_file_created = &(mb_io_ptr->save3);

	/* write projection file if needed */
	if (*projection_file_created == MB_NO && store->projection_set == MB_YES)
		{
		sprintf(projection_file, "%s.prj", mb_io_ptr->file);
		if ((pfp = fopen(projection_file, "w")) != NULL)
			{
			fprintf(pfp, "%s\n",store->projection_id);
			*projection_file_created = MB_YES;
			}
		fclose(pfp);
		}

	if (store->kind == MB_DATA_DATA)
		{
		/* translate file name (strip directory) */
		for(i = strlen(mb_io_ptr->file); i >= 0; i--)
			if (mb_io_ptr->file[i] == '/')
				break;
		strncpy(store->ping.linename, &(mb_io_ptr->file[i+1]), SWPLS_MAX_LINENAME_LEN);
		store->ping.linename[SWPLS_MAX_LINENAME_LEN-1] = '\0';

		/* translate time (remove time nudge added in rt function) */
		store->ping.time_d = store->time_d - (store->ping.txno - 1) * 1e-5;

		/* translate position */
		store->ping.easting = store->navlon;
		store->ping.txer_e = store->navlon;
		store->ping.northing = store->navlat;
		store->ping.txer_n = store->navlat;

		/* translate attitude */
		store->ping.heading = store->heading;
		store->ping.roll = -store->roll;
		store->ping.pitch = store->ping.pitch;
		store->ping.height = -store->heave;

		/* translate environmental parameters */
		store->ping.sos = store->sos;

		/* translate bathy and amp from vessel coordinates to real-world coordinates */
		zero.x = 0.0;
		zero.y = 0.0;
		zero.z = 0.0;

		txatt.heading = store->heading * DTR;
		txatt.pitch = 0.0;
		txatt.roll = 0.0;

		store->ping.nosampsfile = store->nbath;

		points = store->ping.points;
		for(i = 0; i < store->ping.nosampsfile; i++)
			{
			ppos.x = store->bathacrosstrack[i];
			ppos.y = -store->bath[i];
			ppos.z = store->bathalongtrack[i];

			mbr_swplssxp_setup_vessel_to_world_matrix(verbose, &zero, &txatt, &vtow, error);
			mbr_swplssxp_transform(verbose, &vtow, &ppos, error);

			points[i].sampnum = i;
			points[i].x = ppos.x + store->ping.txer_e;
			points[i].y = ppos.z + store->ping.txer_n;
			points[i].z = -ppos.y;
			points[i].amp = (int)store->amp[i];
			points[i].procamp = (int)store->amp[i];

			if (store->beamflag[i] == MB_FLAG_NONE)
				points[i].status = SWPLS_POINT_ACCEPTED;
			else
				points[i].status = SWPLS_POINT_REJECTED;
			}
		}
	else if (store->kind == MB_DATA_COMMENT)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_COMMENT;
		}

	/* write next data to file */
	status = mbr_swplssxp_wr_data(verbose, mbio_ptr, store_ptr, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return status;
}					/* mbr_wt_swplssxp */
/*--------------------------------------------------------------------
    mbr_swplssxp_rd_data

    Reads the next block of data from the sonar file and stores it in
    the sonar-specific storage struct.

    If the datagram is a ping structure, it reads two consecutive pings from
    the same transducer into the store_ptr.  This is necessary to properly
    interpolate the attitude and position of the samples during translation
    of the ping into MB System storage format.

    However, the current implementation is very inefficient, it reads
    each ping

    History:

    2013-08-07 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbr_swplssxp_rd_data
(
	int verbose,	/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,	/* in: see mb_io.h:/^struct mb_io_struct/ */
	void *store_ptr,/* in: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int *error		/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char *function_name = "mbr_swplssxp_rd_data";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	FILE *mbfp;
	swplssxp_ping ping;
	swplssxp_header header;
	off_t mark;
	int done;
	int recordid, size;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		}

	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *) store_ptr;
	mbfp = mb_io_ptr->mbfp;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read the next datagram */
	done = MB_NO;
	mbsys_swathplus_ping_init(verbose, &ping, error);
	while (status == MB_SUCCESS && done == MB_NO)
		{
		/* Mark the file position at the start of the record in case we hit
		   a bad record and need to rewind */
		errno = 0;
		mark = ftello(mbfp); if (errno)
			{
			perror("mbr_swplssxp.c:mbr_swplssxp_rd_data(): mark ftello failed");
			status = MB_FAILURE;
			*error = MB_ERROR_OTHER;
			}

		status = mbr_swplssxp_rd_blockhead(verbose, mb_io_ptr, &recordid, &size, error);
		if ((status == MB_SUCCESS) && (recordid == SWPLS_ID_SXP_HEADER_DATA))
			{
			mbr_swplssxp_rd_header(verbose, mb_io_ptr, &header, error);
			store->kind = MB_DATA_HEADER;
			store->type = SWPLS_ID_SXP_HEADER_DATA;
			store->stored_header = MB_YES;
			store->header = header;
			done = MB_YES;
			}
		else if ((status == MB_SUCCESS) && (recordid == SWPLS_ID_XYZA_PING))
			{
			mbr_swplssxp_rd_ping(verbose, mb_io_ptr, SWPLS_ID_XYZA_PING, &ping, error);

			store->kind = MB_DATA_DATA;
			store->type = SWPLS_ID_XYZA_PING;
			store->stored_ping = MB_YES;
			store->ping = ping;
			done = MB_YES;
			}
		else if ((status == MB_SUCCESS) && (recordid == SWPLS_ID_XYZA_PING2))
			{
			mbr_swplssxp_rd_ping(verbose, mb_io_ptr, SWPLS_ID_XYZA_PING2, &ping, error);

			store->kind = MB_DATA_DATA;
			store->type = SWPLS_ID_XYZA_PING2;
			store->stored_ping = MB_YES;
			store->ping = ping;
			done = MB_YES;
			}
		else
			{
			/* bad or unknown record, rewind, skip ahead 1 byte and try again */
			fseeko(mbfp, ++mark, SEEK_SET);
			if (errno)
				{
				perror("mbr_swplssxp.c:mbr_swplssxp_rd_data(): fseeko failed");
				status = MB_FAILURE;
				*error = MB_ERROR_OTHER;
				}
			done = MB_NO;
			}
		}

	/* get file position */
	mb_io_ptr->file_bytes = ftell(mbfp);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return status;
}		/* mbr_swplssxp_rd_data */
/*--------------------------------------------------------------------
    mbr_swplssxp_rd_blockhead

    Read the blockid and blocksize integers that are at the head of all
    SEA binary datagrahms. This subroutine assumes that the file pointer
    is aligned at the beginning of the next datagrahm.

    History:

    2013-08-12 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbr_swplssxp_rd_blockhead
(
	int verbose,
	void *mb_io_ptr,
	int *recordid,
	int *size,
	int *error
)
{
	char *function_name = "mbr_swplssxp_rd_blockheader";
	int status = MB_SUCCESS;
	size_t read_len;
	static char buffer[SWPLS_SIZE_BLOCKHEADER];
	char *pb;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2      verbose:       %d\n", verbose);
		fprintf(stderr, "dbg2    mb_io_ptr:       %p\n", mb_io_ptr);
		}

	pb = &buffer[0];
	read_len = (size_t)SWPLS_SIZE_BLOCKHEADER;
	status = mb_fileio_get(verbose, mb_io_ptr, pb, &read_len, error);

	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(MB_YES, pb, recordid); pb += 4;
		mb_get_binary_int(MB_YES, pb, size);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		if (*recordid == SWPLS_ID_SXP_HEADER_DATA)
			fprintf(stderr, "dbg2    recordid:      SWPLS_ID_SXP_HEADER_DATA\n");
		else if (*recordid == SWPLS_ID_XYZA_PING)
			fprintf(stderr, "dbg2    recordid:      SWPLS_ID_XYZA_PING\n");
		else if (*recordid == SWPLS_ID_XYZA_PING2)
			fprintf(stderr, "dbg2    recordid:      SWPLS_ID_XYZA_PING2\n");
		else
			fprintf(stderr, "dbg2    recordid:      %d\n", *recordid);
		fprintf(stderr, "dbg2        size:      %d\n", *size);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return status;
}		/* mbr_swplssxp_rd_blockhead */
/*--------------------------------------------------------------------
    mbr_swplssxp_rd_header

    Read the SXP file header datagrahm. Assumes that
    mbr_swplssxp_rd_blockhead has already been called and that the file
    pointer has therefore been moved past the blockid and blocksize integers
    at the beginning of the datagrahm.

    History:

    2013-08-12 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
static int mbr_swplssxp_rd_header
(
	int verbose,
	void *mb_io_ptr,
	swplssxp_header *header,
	int *error
)
{
	char *function_name = "mbr_swplssxp_rd_header";
	int status = MB_SUCCESS;
	size_t read_len;
	static char buffer[SWPLS_SIZE_STARTER];
	char *pb;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2     mb_io_ptr:    %p\n", mb_io_ptr);
		fprintf(stderr, "dbg2        header:    %p\n", header);
		}

	pb = &buffer[0];
	read_len = (size_t)SWPLS_SIZE_STARTER;
	status = mb_fileio_get(verbose, mb_io_ptr, pb, &read_len, error);
	if (status == MB_SUCCESS)
		{
		mb_get_binary_int(MB_YES, pb, &(header->swver)); pb += 4;
		mb_get_binary_int(MB_YES, pb, &(header->fmtver));
		}

	/* print output debug statements */
	if (verbose >= 4)
		mbsys_swathplus_print_header(verbose, header, error);
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2      header:     %p\n", header);
		fprintf(stderr, "dbg2       error:     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	return status;
}		/* mbr_swplssxp_rd_header */
/*--------------------------------------------------------------------
    mbr_swplssxp_rd_ping

    Return an SXP processed ping record from the file. Set the type of SXP
    ping record as the third argument to the subroutine:

    SWPLS_ID_XYZA_PING  - processed ping data format prior to Jan 2010
    SWPLS_ID_XYZA_PING2 - procesed ping data format after Jan 2010

    All type 1 pings are converted to type 2 pings on read.

    History:

    2013-08-12 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbr_swplssxp_rd_ping
(
	int verbose,
	void *mb_io_ptr,
	int type,
	swplssxp_ping *ping,
	int *error
)
{
	char    *function_name = "mbr_swplssxp_rd_ping";
	int status = MB_SUCCESS;
	size_t read_len;
	static char buffer[SWPLS_MAX_RECORD_SIZE];
	char *pb;
	short short_val;
	int int_val;
	int i;

	assert(mb_io_ptr != NULL);
	assert(type == SWPLS_ID_XYZA_PING || type == SWPLS_ID_XYZA_PING2);	/* known type? */

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);;
		fprintf(stderr, "dbg2     mb_io_ptr:    %p\n", mb_io_ptr);
		if (type == SWPLS_ID_XYZA_PING)
			fprintf(stderr, "dbg2          type:    SWPLS_ID_XYZA_PING\n");
		else if (type == SWPLS_ID_XYZA_PING2)
			fprintf(stderr, "dbg2          type:    SWPLS_ID_XYZA_PING2\n");
		else
			fprintf(stderr, "dbg2          type:    %d\n", type);
		fprintf(stderr, "dbg2          ping:    %p\n", ping);
		}

	/* read the first part of the ping record from the file into the buffer */
	pb = &buffer[0];
	if (type == SWPLS_ID_XYZA_PING2)
		read_len = (size_t)SWPLS_SIZE_PING2;
	else if (type == SWPLS_ID_XYZA_PING)
		read_len = (size_t)SWPLS_SIZE_PING;
	status = mb_fileio_get(verbose, mb_io_ptr, pb, &read_len, error);

	/* extract the ping data from the buffer into the ping struct */
	if (status == MB_SUCCESS)
		{
		strncpy(pb, ping->linename, SWPLS_MAX_LINENAME_LEN); pb += SWPLS_MAX_LINENAME_LEN;
		ping->linename[SWPLS_MAX_LINENAME_LEN-1] = '\0';
		mb_get_binary_int(MB_YES, pb, &int_val); pb += 4;
		ping->pingnumber = (unsigned int)int_val;
		pb += 4;	/* padding bytes */
		mb_get_binary_double(MB_YES, pb, &(ping->time_d)); pb += 8;
		mb_get_binary_int(MB_YES, pb, &(ping->notxers)); pb += 4;
		pb += 4;	/* padding bytes */
		mb_get_binary_double(MB_YES, pb, &(ping->easting)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->northing)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->roll)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->pitch)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->heading)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->height)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->tide)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->sos)); pb += 8;
		ping->txno = *pb++;
		ping->txstat = *pb++;
		ping->txpower = *pb++;
		pb += 1;	/* padding byte */
		mb_get_binary_short(MB_YES, pb, &(ping->analoggain)); pb += 2;
		ping->nostaves = *pb++;
		for (i = 0; i != SWPLS_MAX_TX_INFO; ++i)
			ping->txinfo[i] = *pb++;
		ping->freq = *pb++;
		pb += 4;	/* padding bytes */
		mb_get_binary_double(MB_YES, pb, &(ping->frequency)); pb += 8;
		mb_get_binary_short(MB_YES, pb, &(ping->trnstime)); pb += 2;
		mb_get_binary_short(MB_YES, pb, &(ping->recvtime)); pb += 2;
		ping->samprate = *pb++;
		pb += 3;	/* padding bytes */
		mb_get_binary_int(MB_YES, pb, &(ping->nosampsorig)); pb += 4;
		mb_get_binary_int(MB_YES, pb, &(ping->nosampsfile)); pb += 4;
		mb_get_binary_int(MB_YES, pb, &(ping->nosampslots)); pb += 4;
		pb += 4;	/* padding bytes */
		mb_get_binary_double(MB_YES, pb, &(ping->txer_e)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->txer_n)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->txer_height)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->txer_forward)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->txer_starboard)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->txer_azimuth)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->txer_elevation)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->txer_skew)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->txer_time)); pb += 8;
		mb_get_binary_double(MB_YES, pb, &(ping->txer_waterdepth)); pb += 8;

		if (type == SWPLS_ID_XYZA_PING)
			{
			ping->txer_pitch = 0.0;
			}
		else if (type == SWPLS_ID_XYZA_PING2)
			{
			mb_get_binary_double(MB_YES, pb, &(ping->txer_pitch)); pb += 8;
			}
		}

	/* check that we have enough storage for the points stored in the file */
	if (MBSYS_SWPLS_MAX_BEAMS <= ping->nosampsfile)
		{
		fprintf(stderr,
			"error:%s: number of samples in file (%d) exceeds allocated storeage (%d)\n",
			function_name,
			ping->nosampsfile,
			MBSYS_SWPLS_MAX_BEAMS);
		fprintf(stderr, "Recompile MB System with MBSYS_SWPLS_MAX_BEAMS > %d\n", ping->nosampsfile);
		fprintf(stderr, "to process this ping. Skipping ping.\n");
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* read the point data from the file into the buffer */
	if (status == MB_SUCCESS)
		{
		if (type == SWPLS_ID_XYZA_PING2)
			read_len = (size_t)(ping->nosampsfile * SWPLS_SIZE_POINT2);
		else if (type == SWPLS_ID_XYZA_PING)
			read_len = (size_t)(ping->nosampsfile * SWPLS_SIZE_POINT);
		status = mb_fileio_get(verbose, mb_io_ptr, pb, &read_len, error);
		}

	/* extract the point data from the buffer into the ping struct */
	if (status == MB_SUCCESS)
		for (i = 0; i != ping->nosampsfile && i != MBSYS_SWPLS_MAX_BEAMS; ++i)
			{
			mb_get_binary_int(MB_YES, pb, &(ping->points[i].sampnum)); pb += 4;
			pb += 4;	/* padding bytes */
			mb_get_binary_double(MB_YES, pb, &(ping->points[i].y)); pb += 8;
			mb_get_binary_double(MB_YES, pb, &(ping->points[i].x)); pb += 8;
			mb_get_binary_float(MB_YES, pb, &(ping->points[i].z)); pb += 4;
			mb_get_binary_short(MB_YES, pb, &short_val); pb += 2;
			ping->points[i].amp = (unsigned short)short_val;
			mb_get_binary_short(MB_YES, pb, &short_val); pb += 2;
			ping->points[i].procamp = (unsigned short)short_val;
			ping->points[i].status = *pb++;
			pb += 7;	/* padding bytes */

			/* old-syle points don't have tpu parameter */
			if (type == SWPLS_ID_XYZA_PING)
				{
				ping->points[i].tpu = 0.0;
				}
			else if (type == SWPLS_ID_XYZA_PING2)
				{
				mb_get_binary_double(MB_YES, pb, &(ping->points[i].tpu)); pb += 8;
				}
			}

	/* print output debug statements */
	if (verbose >= 4)
		mbsys_swathplus_print_ping(verbose, ping, error);
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2        ping:      %p\n", ping);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	return status;
}	/* mbr_swplssxp_rd_ping */
/*--------------------------------------------------------------------
    mbr_swplssxp_wr_data

    Write data block to SXP file.

    History:

    2013-08-21 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/

int mbr_swplssxp_wr_data
(
	int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *error
)
{
	char *function_name = "mbr_swplssxp_wr_data";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	FILE *mbfp;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2 MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2      mbio_ptr:    %p\n", mbio_ptr);
		fprintf(stderr, "dbg2     store_ptr:    %p\n", mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_swathplus_struct *) store_ptr;
	if (verbose >= 4)
		mbsys_swathplus_print_store(verbose, store, error);

	mbfp = mb_io_ptr->mbfp;

	/* call appropriate writing routine for data */
	if ((status == MB_SUCCESS) && (store->kind == MB_DATA_DATA))
		{
		if ((store->type == SWPLS_ID_XYZA_PING2) || (store->type == SWPLS_ID_XYZA_PING))
			{
			status = mbr_swplssxp_wr_ping(verbose, mb_io_ptr, &(store->ping), error);
			}
		else
			{
			fprintf(stderr, "call nothing bad kind: %d type %x\n", store->kind, store->type);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_KIND;
			}
		}
	else if ((status == MB_SUCCESS) && (store->kind == MB_DATA_HEADER))
		{
		if (store->type == SWPLS_ID_SXP_HEADER_DATA)
			{
			status = mbr_swplssxp_wr_header(verbose, mb_io_ptr, &(store->header), error);
			}
		else
			{
			fprintf(stderr, "call nothing bad kind: %d type %x\n", store->kind, store->type);
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_KIND;
			}
		}
	else if ((status == MB_SUCCESS) && (store->kind == MB_DATA_COMMENT))
		{
		status = MB_FAILURE;
		*error = MB_ERROR_COMMENT;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:    %d\n", status);
		}

	/* return status */
	return status;
}		/* mbr_swplssxp_wr_data */
/*--------------------------------------------------------------------
    mbr_swplssxp_wr_header

    Write the file header block to disk.

    History:

    2013-08-22 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbr_swplssxp_wr_header
(
	int verbose,
	void *mb_io_ptr,
	swplssxp_header *header,
	int *error
)
{
	char    *function_name = "mbr_swplssxp_wr_header";
	int status = MB_SUCCESS;
	size_t write_len;
	static char buffer[SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_STARTER];
	char    *pb;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2     mb_io_ptr:    %p\n", mb_io_ptr);
		fprintf(stderr, "dbg2        header:    %p\n", header);
		}
	if (verbose >= 4)
		mbsys_swathplus_print_header(verbose, header, error);

	/* insert the block header */
	pb = &buffer[0];
	mb_put_binary_int(MB_YES, SWPLS_ID_SXP_HEADER_DATA, pb); pb += 4;
	mb_put_binary_int(MB_YES, SWPLS_SIZE_STARTER, pb); pb += 4;

	/* insert the file header data */
	mb_put_binary_int(MB_YES, header->swver, pb); pb += 4;
	mb_put_binary_int(MB_YES, header->fmtver, pb);

	/* write the data */
	write_len = (size_t)(SWPLS_SIZE_BLOCKHEADER + SWPLS_SIZE_STARTER);
	status = mb_fileio_put(verbose, mb_io_ptr, buffer, &write_len, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return status;
}						/* mbr_swplssxp_wr_header */
/*--------------------------------------------------------------------
    mbr_swplssxp_wr_ping

    Write the sxp ping record to disk.

    History:

    2013-08-22 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbr_swplssxp_wr_ping
(
	int verbose,		/* in: verbosity set at command line 0..N */
	void *mbio_ptr,		/* in:  */
	swplssxp_ping *ping,/* in: ?? */
	int *error			/* out: ?? */
)
{
	char    *function_name = "mbr_swplssxp_wr_ping";
	struct mb_io_struct *mb_io_ptr;
	int status = MB_SUCCESS;
	size_t write_len;
	static char buffer[SWPLS_MAX_RECORD_SIZE];
	char *pb;
	int i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2      mbio_ptr:    %p\n", mbio_ptr);
		fprintf(stderr, "dbg2          ping:    %p\n", ping);
		}
	if (verbose >= 4)
		mbsys_swathplus_print_ping(verbose, ping, error);

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* figure out the size of the output record */
	write_len = SWPLS_SIZE_BLOCKHEADER;
	write_len += SWPLS_SIZE_PING2;
	write_len += SWPLS_SIZE_POINT2 * ping->nosampsfile;

	/* insert the block header */
	pb = &buffer[0];
	mb_put_binary_int(MB_YES, SWPLS_ID_XYZA_PING2, pb); pb += 4;
	mb_put_binary_int(MB_YES, (write_len - SWPLS_SIZE_BLOCKHEADER), pb); pb += 4;

	/* get the name of the file without path */
	strncpy(pb, &(ping->linename[0]), SWPLS_MAX_LINENAME_LEN); pb += SWPLS_MAX_LINENAME_LEN;
	mb_put_binary_int(MB_YES, ping->pingnumber, pb); pb += 4;
	pb += 4;		/* padding bytes */
	mb_put_binary_double(MB_YES, ping->time_d, pb); pb += 8;
	mb_put_binary_int(MB_YES, ping->notxers, pb); pb += 4;
	pb += 4;		/* padding bytes */
	mb_put_binary_double(MB_YES, ping->easting, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->northing, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->roll, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->pitch, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->heading, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->height, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->tide, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->sos, pb); pb += 8;
	*pb = ping->txno; pb += 1;
	*pb = ping->txstat; pb += 1;
	*pb = ping->txpower; pb += 1;
	pb += 1;		/* padding byte */
	mb_put_binary_short(MB_YES, ping->analoggain, pb); pb += 2;
	*pb = ping->nostaves; pb += 1;
	for (i = 0; i != SWPLS_MAX_TX_INFO; ++i)
		{
		*pb = ping->txinfo[i]; pb += 1;
		}
	*pb = ping->freq; pb += 1;
	pb += 4;		/* padding bytes */
	mb_put_binary_double(MB_YES, ping->frequency, pb); pb += 8;
	mb_put_binary_short(MB_YES, ping->trnstime, pb); pb += 2;
	mb_put_binary_short(MB_YES, ping->recvtime, pb); pb += 2;
	*pb = ping->samprate; pb += 1;
	pb += 3;		/* padding bytes */
	mb_put_binary_int(MB_YES, ping->nosampsorig, pb); pb += 4;
	mb_put_binary_int(MB_YES, ping->nosampsfile, pb); pb += 4;
	mb_put_binary_int(MB_YES, ping->nosampslots, pb); pb += 4;
	pb += 4;		/* padding bytes */
	mb_put_binary_double(MB_YES, ping->txer_e, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->txer_n, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->txer_height, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->txer_forward, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->txer_starboard, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->txer_azimuth, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->txer_elevation, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->txer_skew, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->txer_time, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->txer_waterdepth, pb); pb += 8;
	mb_put_binary_double(MB_YES, ping->txer_pitch, pb); pb += 8;

	/* insert the xyza point data */
	for (i = 0; i != ping->nosampsfile; ++i)
		{
		mb_put_binary_int(MB_YES, ping->points[i].sampnum, pb); pb += 4;
		pb += 4;		/* padding bytes */
		mb_put_binary_double(MB_YES, ping->points[i].y, pb); pb += 8;
		mb_put_binary_double(MB_YES, ping->points[i].x, pb); pb += 8;
		mb_put_binary_float(MB_YES, ping->points[i].z, pb); pb += 4;
		mb_put_binary_short(MB_YES, ping->points[i].amp, pb); pb += 2;
		mb_put_binary_short(MB_YES, ping->points[i].procamp, pb); pb += 2;
		*pb = ping->points[i].status; pb += 1;
		pb += 7;		/* padding bytes */
		mb_put_binary_double(MB_YES, ping->points[i].tpu, pb); pb += 8;
		}

	/* write the data */
	status = mb_fileio_put(verbose, mb_io_ptr, buffer, &write_len, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return status;
}		/* mbr_swplssxp_wr_ping */
/*-------------------------------------------------------------------*/
int mbr_swplssxp_interp_point
(
	int verbose,
	swplssxp_point *p1,
	swplssxp_point *p2,
	swplssxp_point *p,
	int *error
)
{
	char *function_name = "mbr_swplssxp_interp_posn";
	int status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       p1:   %p\n", p1);
		fprintf(stderr, "dbg2       p2:   %p\n", p2);
		}

	if ((p->sampnum < p1->sampnum) || (p2->sampnum < p->sampnum))
		{
		*error = MB_ERROR_OUT_BOUNDS;
		status = MB_FAILURE;
		}

	if (status == MB_SUCCESS)
		status =
			mbr_swplssxp_interp(verbose,
			p1->sampnum,
			p2->sampnum,
			p1->x,
			p2->x,
			p->sampnum,
			&(p->x),
			error);
	if (status == MB_SUCCESS)
		status =
			mbr_swplssxp_interp(verbose,
			p1->sampnum,
			p2->sampnum,
			p1->y,
			p2->y,
			p->sampnum,
			&(p->y),
			error);
	if (status == MB_SUCCESS)
		{
		double z;
		status = mbr_swplssxp_interp(verbose,
			p1->sampnum,
			p2->sampnum,
			p1->z,
			p2->z,
			p->sampnum,
			&z,
			error);
		p->z = (float)z;
		}
	if (status == MB_SUCCESS)
		status = mbr_swplssxp_interp(verbose,
			p1->sampnum,
			p2->sampnum,
			p1->tpu,
			p2->tpu,
			p->sampnum,
			&(p->tpu),
			error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:   %d\n", *error);
		fprintf(stderr, "dbg2           p:   %p\n", p);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return status;
}		/* mbr_swplssxp_interp_point */
/* mbr_swplssxp_interp: linear intepolate the value x between (x1, y1) and (x2, y2) */
int mbr_swplssxp_interp
(
	int verbose,
	double x1,
	double y1,
	double x2,
	double y2,
	double x,
	double *y,
	int *error
)
{
	char    *function_name = "mbr_swplssxp_interp_posn";
	int status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       x1:   %f\n", x1);
		fprintf(stderr, "dbg2       y1:   %f\n", y1);
		fprintf(stderr, "dbg2       x2:   %f\n", x2);
		fprintf(stderr, "dbg2       y2:   %f\n", y2);
		fprintf(stderr, "dbg2        x:   %f\n", x);
		}

	if ((x < x1) || (x2 < x))
		{
		*error = MB_ERROR_OUT_BOUNDS;
		status = MB_FAILURE;
		}

	if (status == MB_SUCCESS)
		*y = y1 + (y2 - y1) / (x2 - x1) * (x - x1);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:   %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		fprintf(stderr, "dbg2            y:  %f\n", *y);
		}

	/* return status */
	return status;
}														/* mbr_swplssxp_interp */
/*--------------------------------------------------------------------
    mbr_swplssxp_get_beamwidth

    Returns an appropriate alongtrack and acrosstrack beam width angle based
    on the sonar frequency.

    History:

    2013-08-12 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbr_swplssxp_get_beamwidth
(
	int verbose,										/* in: verbosity level set at command line
														   0..N */
	double frequency,									/* in: SWATHplus sonar frequency (Hz) 0..N
														 */
	double *bwxtrack,									/* out: across-track beam width (deg) */
	double *bwltrack,									/* out: along-track beam width (deg) */
	int *error											/* out: see mb_status.h:MB_ERROR */
)
{
	char *function_name = "mbr_swplssxp_get_beamwidth";
	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	assert(100000 < frequency && frequency < 500000);	/* new sonar type? */

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:     %d\n", verbose);
		fprintf(stderr, "dbg2       frequency:   %f\n", frequency);
		}

	if (frequency < 234000)
		{
		*bwxtrack = SWPLS_TYPE_L_BEAM_WIDTH;
		*bwltrack = SWPLS_TYPE_L_BEAM_WIDTH;
		}
	else if (frequency < 468000)
		{
		*bwxtrack = SWPLS_TYPE_M_BEAM_WIDTH;
		*bwltrack = SWPLS_TYPE_M_BEAM_WIDTH;
		}
	else
		{
		*bwxtrack = SWPLS_TYPE_H_BEAM_WIDTH;
		*bwltrack = SWPLS_TYPE_H_BEAM_WIDTH;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:   %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		fprintf(stderr, "dbg2       bwxtrack:  %f\n", *bwxtrack);
		fprintf(stderr, "dbg2       bwltrack;  %f\n", *bwltrack);
		}

	/* return status */
	return status;
}				/* mbr_swplssxp_rt_beamwidth */
/*********************************************************************

   Following 3D Math algorithms are based on the book:

   Dunn, F. and Parberry, I. (2002) 3D Math Primer for Graphics and
   Game Development. Wordware, Sudbury, MA. 428 pp.

*********************************************************************/

/*--------------------------------------------------------------------
    mbr_swplssxp_transform

    Transform the vector coordinates, v using the transformation
    matrix m.

    See: Dunn and Parberry, 2002, 3D Math Primer for Graphics and Game
    Development , Wordware, Sudbury, MA.  pg.  235

    History:

    2013-08-15 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
static int mbr_swplssxp_transform
(
	int verbose,
	swplssxp_matrix *m,
	swplssxp_vector *v,
	int *error
)
{
	char  *function_name = "mbsys_swathplus_transform";
	double vx, vy, vz;
	int status;

	/* print input debug values */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       m:            %p\n", m);
		fprintf(stderr, "dbg2       v:            %p\n", v);
		}
	if (verbose >= 4)
		{
		fprintf(stderr, "\ndbg4  v.x = %f\n", v->x);
		fprintf(stderr, "dbg4  v.y = %f\n", v->y);
		fprintf(stderr, "dbg4  v.z = %f\n", v->z);
		}

	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	vx = v->x;
	vy = v->y;
	vz = v->z;
	v->x = vx*m->m11 + vy*m->m21 + vz*m->m31 + m->tx;
	v->y = vx*m->m12 + vy*m->m22 + vz*m->m32 + m->ty;
	v->z = vx*m->m13 + vy*m->m23 + vz*m->m33 + m->tz;

	/* print output debug values */
	if (verbose >= 4)
		{
		fprintf(stderr, "\ndbg4  v.x = %f\n", v->x);
		fprintf(stderr, "dbg4  v.y = %f\n", v->y);
		fprintf(stderr, "dbg4  v.z = %f\n", v->z);
		}
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return status;
}									/* mbsys_swathplus_transform */
/*--------------------------------------------------------------------
    mbr_swplssxp_angles_to_quaternion

    Setup the quaternion to perform a vessel->world rotation, given
    the orientation in Euler angle format.

    See: Dunn and Parberry, 2002, 3D Math Primer for Graphics and Game
    Development , Wordware, Sudbury, MA.  pg.  209

    History:

    2013-08-15 - Function defined for SWATHplus - DPF

   --------------------------------------------------------------------*/
static int mbr_swplssxp_angles_to_quaternion
(
	int verbose,					/* in: verbosity; 0..N */
	swplssxp_angles *orientation,	/* in: orientation Euler angles */
	swplssxp_quaternion *q,			/* out: orientation converted to a quaternion */
	int *error						/* out: see mb_status.h:MB_ERROR */
)
{
	char *function_name = "mbr_swplssxp_angles_to_quaternion";
	int status;
	double sp, sr, sh, cp, cr, ch;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2   orientation:      %p\n", orientation);
		}
	if (verbose >= 4)
		{
		fprintf(stderr, "\ndgb4  orientation.heading = %f\n", orientation->heading);
		fprintf(stderr, "dbg4  orientation.pitch   = %f\n", orientation->pitch);
		fprintf(stderr, "dbg4  orienation.roll     = %f\n", orientation->roll);
		}

	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* Compute sine and cosine of the half angles */
	sp = sin(orientation->pitch * 0.5);
	cp = cos(orientation->pitch * 0.5);
	sr = sin(orientation->roll * 0.5);
	cr = cos(orientation->roll * 0.5);
	sh = sin(orientation->heading * 0.5);
	ch = cos(orientation->heading * 0.5);

	/* Compute values */
	q->w = ch*cp*cr + sh*sp*sr;
	q->x = ch*sp*cr + sh*cp*sr;
	q->y = -ch*sp*sr + sh*cp*cr;
	q->z = -sh*sp*cr + ch*cp*sr;

	/* print output debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr, "\ndbg4  q.w = %f\n", q->w);
		fprintf(stderr, "dbg4  q.x = %f\n", q->x);
		fprintf(stderr, "dbg4  q.y = %f\n", q->y);
		fprintf(stderr, "dbg4  q.z = %f\n", q->z);
		}
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2           q:      %p\n", q);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return status;
}									/* mbr_swplssxp_angles_to_quaternion */
/*--------------------------------------------------------------------
    mbr_swplssxp_quaternion_to_angles

    Setup the Euler angles, given an vessel->world rotation quaternion.

    See: Dunn and Parberry, 2002, 3D Math Primer for Graphics and Game
    Development , Wordware, Sudbury, MA.  pg.  201-202

    History:

    2013-08-15 - Function defined for SWATHplus - DPF

   --------------------------------------------------------------------*/
static int mbr_swplssxp_quaternion_to_angles
(
	int verbose,					/* in: verbosity set at command line; 0..N */
	swplssxp_quaternion *q,			/* in: orientation quaternion */
	swplssxp_angles *orientation,	/* out: orientation converted to Euler angles */
	int *error						/* out: see mb_status.h:MB_ERROR */
)
{
	char *function_name = "mbr_swplssxp_quaternion_to_angles";
	int status;
	double sp;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       q:            %p\n", q);
		}
	if (verbose >= 4)
		{
		fprintf(stderr, "\ndbg4  q.w = %f\n", q->w);
		fprintf(stderr, "dbg4  q.x = %f\n", q->x);
		fprintf(stderr, "dbg4  q.y = %f\n", q->y);
		fprintf(stderr, "dbg4  q.z = %f\n", q->z);
		}

	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* extract sin(pitch) */
	sp = -2.0 * (q->y*q->z - q->w*q->x);

	if (fabs(sp) > 0.9999)
		{
		/* looking straight up or down */
		orientation->pitch = M_PI/2.0 * sp;
		orientation->heading = atan2(-q->x*q->z + q->w*q->y, 0.5 - q->y*q->y - q->z*q->z);
		orientation->roll = 0.0;
		}
	else
		{
		orientation->pitch = asin(sp);
		orientation->heading = atan2(q->x*q->z + q->w*q->y, 0.5 - q->x*q->x - q->y*q->y);
		orientation->roll = atan2(q->x*q->y + q->w*q->z, 0.5 - q->x*q->x - q->z*q->z);
		}

	/* print output debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr, "\ndgb4  orientation.heading = %f\n", orientation->heading);
		fprintf(stderr, "dbg4  orientation.pitch   = %f\n", orientation->pitch);
		fprintf(stderr, "dbg4  orienation.roll     = %f\n", orientation->roll);
		}
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2 orientation:      %p\n", orientation);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return status;
}							/* mbr_swplssxp_quaternion_to_angles */
/*--------------------------------------------------------------------
    mbr_swplssxp_slerp

    Sphereical Linear intERPolation (SLERP) between two orientations.

    q0 and q1 are the starting and ending orientations expressed as
    quaternions.  t is the fraction between q0 and q1 for the interpolation
    and should be a number between 0 and 1.  q is a quaternion in which to
    store the result.

    SLERP avoids all the normal problems interpolating Euler Angles. It is
    smooth, it doesn't alias at 360 degrees, it picks the acute angle even
    when interpolating across cyclic boundaries (-170 to +170 degrees, or
  +350 to +10 degrees, for example), and it won't gimble lock.

    if t < 0 then q0 is returned, if 1 < t then q1 is returned.

    See: Dunn and Parberry, 2002, 3D Math Primer for Graphics and Game
    Development , Wordware, Sudbury, MA.  pg. 212-214.

    History:

    2013-08-15 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
static int mbr_swplssxp_slerp
(
	int verbose,			/* in: verbosity level set at command line 0..N */
	swplssxp_quaternion *q0,/* in: starting orientation */
	swplssxp_quaternion *q1,/* in: ending orientation */
	double t,				/* in: fraction between q0 and q1; range 0..1 */
	swplssxp_quaternion *q,	/* out: result of interpolation */
	int *error				/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char *function_name = "mbr_swplssxp_slerp";
	int status;
	double q1w, q1x, q1y, q1z;
	double k0, k1;
	double omega, cosOmega, sinOmega, oneOverSinOmega;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2           verbose:  %d\n", verbose);
		fprintf(stderr, "dbg2                q0:  %p\n", q0);
		fprintf(stderr, "dbg2                q1:  %p\n", q1);
		fprintf(stderr, "dbg2                 t:  %f\n", t);
		}
	if (verbose >= 4)
		{
		fprintf(stderr, "\ndbg4  q0.w = %f\n", q0->w);
		fprintf(stderr, "dbg4  q0.x = %f\n", q0->x);
		fprintf(stderr, "dbg4  q0.y = %f\n", q0->y);
		fprintf(stderr, "dbg4  q0.z = %f\n", q0->z);
		fprintf(stderr, "\ndbg4  q1.w = %f\n", q1->w);
		fprintf(stderr, "dbg4  q1.x = %f\n", q1->x);
		fprintf(stderr, "dbg4  q1.y = %f\n", q1->y);
		fprintf(stderr, "dbg4  q1.z = %f\n", q1->z);
		}

	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* Check for out of range parameter and return edge points if so */
	if (t <= 0.0)
		{
		*q = *q0;
		}
	else if (t >= 1.0)
		{
		*q = *q1;
		}
	else
		{
		/* Compute "cosine of angle between quaternions" using dot product */
		cosOmega = q0->w*q1->w + q0->x*q1->x + q0->y*q1->y + q0->z*q1->z;

		/* Chose q or -q to rotate using the acute angle */
		q1w = q1->w;
		q1x = q1->x;
		q1y = q1->y;
		q1z = q1->z;
		if (cosOmega < 0.0)
			{
			q1w = -q1w;
			q1x = -q1x;
			q1y = -q1y;
			q1z = -q1z;
			cosOmega = -cosOmega;
			}

		/* We should have two unit quaternions, so dot should be <= 1.0 */
		assert(cosOmega < 1.1);

		/* Compute interpolation fraction */
		if (cosOmega > 0.9999)
			{
			/* very close - just use linear interpolation */
			k0 = 1.0 - t;
			k1 = t;
			}
		else
			{
			sinOmega = sqrt(1.0 - cosOmega*cosOmega);
			omega = atan2(sinOmega, cosOmega);
			oneOverSinOmega = 1.0 / sinOmega;
			k0 = sin((1.0 - t) * omega) * oneOverSinOmega;
			k1 = sin(t * omega) * oneOverSinOmega;
			}

		/* Interpolate */
		q->x = k0*q0->x + k1*q1x;
		q->y = k0*q0->y + k1*q1y;
		q->z = k0*q0->z + k1*q1z;
		q->w = k0*q0->w + k1*q1w;
		}

	/* print output debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr, "\ndbg4  q.x = %f\n", q->x);
		fprintf(stderr, "dbg4  q.y = %f\n", q->y);
		fprintf(stderr, "dbg4  q.z = %f\n", q->z);
		fprintf(stderr, "dbg4  q.w = %f\n", q->w);
		}
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2            q:      %p\n", q);
		fprintf(stderr, "dbg2        error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
		}

	/* return status */
	return status;
}	/* mbr_swplssxp_slerp */
/*--------------------------------------------------------------------
    mbr_swplssxp_setup_vessel_to_world_matrix

    set up the transformation matrix from the vessel coordinate system to
    the world coordinate system given the position of the vessel in world
    coordinates and the attitude orientation of the vessel. Uses the heading
    > pitch > bank (Eurler 3-2-1) convention as used by SWATHplus. Rotation
    is done before translation.

    See: Dunn and Parberry, 2002, 3D Math Primer for Graphics and Game
    Development , Wordware, Sudbury, MA.  pg. 224-238.

    History:

    2013-08-15 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
static int mbr_swplssxp_setup_vessel_to_world_matrix
(
	int verbose,
	swplssxp_vector *position,
	swplssxp_angles *orientation,
	swplssxp_matrix *m,
	int *error
)
{
	char  *function_name = "mbr_swplssxp_setup_vessel_to_world_matrix";
	int status = MB_SUCCESS;
	double sh, ch, sp, cp, sr, cr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2           verbose:  %d\n", verbose);
		fprintf(stderr, "dbg2          position:  %p\n", position);
		fprintf(stderr, "dbg2       orientation:  %p\n", orientation);
		fprintf(stderr, "dbg2                 m:  %p\n", m);
		}

	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* Precalculate trig values */
	sh = sin(orientation->heading);
	ch = cos(orientation->heading);
	sp = sin(orientation->pitch);
	cp = cos(orientation->pitch);
	sr = sin(orientation->roll);
	cr = cos(orientation->roll);

	/* Setup the rotation matrix */
	m->m11 = ch * cr + sh * sp * sr;
	m->m12 = sr * cp;
	m->m13 = -sh * cr + ch * sp * sr;

	m->m21 = -ch * sr + sh * sp * cr;
	m->m22 = cr * cp;
	m->m23 = sr * sh + ch * sp * cr;

	m->m31 = sh * cp;
	m->m32 = -sp;
	m->m33 = ch * cp;

	/* Setup the translation portion. */
	m->tx = position->x;
	m->ty = position->y;
	m->tz = position->z;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2           m:      %p\n", m);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return status;
}		/* mbsys_swathplus_setup_vessel_to_world_matrix */
/*--------------------------------------------------------------------
    mbsys_swathplus_setup_world_to_vessel_matrix

    Setup the transformation matrix from the world coordinate system to the
    vessel coordinate system given the position of the vessel in world
    coordinates and the vessel attitude orientation.Uses the heading > pitch
    > bank (Euler 3-2-1) conventions used by SWATHplus.  Rotations are done
    before translations.

    See: Dunn and Parberry, 2002, 3D Math Primer for Graphics and Game
    Development , Wordware, Sudbury, MA.  pg. 224-238.

    History:

    2013-08-15 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
static void mbr_swplssxp_setup_world_to_vessel_matrix
(
	int verbose,
	swplssxp_vector *position,
	swplssxp_angles *orientation,
	swplssxp_matrix *m,
	int *error
)
{
	char  *function_name = "mbsys_swathplus_setup_world_to_vessel_matrix";
	int status = MB_SUCCESS;
	double sh, ch, sp, cp, sr, cr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       m:            %p\n", m);
		fprintf(stderr, "dbg2       position:     %p\n", position);
		fprintf(stderr, "dbg2       orientation:  %p\n", orientation);
		}

	/* Precalculate trig values */
	sh = sin(orientation->heading);
	ch = cos(orientation->heading);
	sp = sin(orientation->pitch);
	cp = cos(orientation->pitch);
	sr = sin(orientation->roll);
	cr = cos(orientation->roll);

	/* Setup the rotation matrix */
	m->m11 = ch * cr + sh * sp * sr;
	m->m12 = -ch * sr + sh * sp * cr;
	m->m13 = sh * cp;

	m->m21 = sr * cp;
	m->m22 = cr * cp;
	m->m23 = -sp;

	m->m31 = -sh * cr + ch * sp * sr;
	m->m32 = sr * sh + ch * sp * cr;
	m->m33 = ch * cp;

	/* Setup the translation portion, the rotation occurs first,
	 * so the translation is rotated before applying. */
	m->tx = -(position->x*m->m11 + position->y*m->m21 + position->z*m->m31);
	m->ty = -(position->x*m->m12 + position->y*m->m22 + position->z*m->m32);
	m->tz = -(position->x*m->m13 + position->y*m->m23 + position->z*m->m33);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}
}		/* mbsys_swathplus_setup_world_to_vessel_matrix */
/* --------------------------------------------------------------------*/

