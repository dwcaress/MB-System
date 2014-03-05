/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_swathplus.c	3.00	5/7/2013
 *	$Id$
 *
 *    Copyright (c) 2013-2013 by
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
 * mbsys_swathplus.c contains the MBIO functions for handling data from
 * SEA SWATHplus interferometric formats:
 *      MBF_SWPLSSXI : MBIO ID 221 - SWATHplus intermediate format
 *      MBF_SWPLSSXP : MBIO ID 221 - SWATHplus processed format
 *
 * Author:	David Finlayson and D. W. Caress
 * Date:	Aug 29, 2013
 *
 * $Log: mbsys_swathplus.c,v $
 *
 *
 * Notes on the MBF_SWPLSSXP file format:
 *
 * 1. Processed data files are written with the file extension "SXP". They
 *
 *    contain the following data blocks:
 *    SWPLS_ID_SXP_HEADER_DATA
 *    SWPLS_ID_XYZA_PING2 (SBP_XYZA_PING2)
 *    SWPLS_ID_XYZA_PING  (SBP_XYZA_PING) - Obsolete as of Jan 2010
 *
 *    These items write out memory images of C++ classes, as created by
 *    Microsoft Visual Studio.  These memory images may have padding bytes
 *    between some objects, to align data objects to word boundaries, so
 *    caution may be needed when reading these objects with code created by
 *    other compilers, languages and operating systems.
 *
 * 2. The SWPLS_ID_XYZA_Ping2 format is used in the processed data files
 *    written by SWATHplus code distrubuted after January 2010. It contains
 *    all the processed data for a single ping.  It is similar to the
 *    formats used internally by the SEA SWATH software, but the structures
 *    are defined separately in order to keep control of file size.
 *    Following the block header, there are three kinds of element in the
 *    data block;
 *
 *    a. Ping data (class cXYZAPing)
 *    b. Transducer data header (class cXYZATxer)
 *    c. Bathymetric data samples (class cXYZAPoint)
 *
 *    Each block contains the data from one transducer. If the sonar is
 *    operating in "simultaneous" mode, with both transducers firing at the
 *    same time, then two separate "SBP_XYZA_PING2" blocks will be
 *    generated, with same time stamp.  Each block therefore contains one
 *    each of the data types in order: Ping data, Transducer data header,
 *    Bathy data array.  For MB System, the three structures have been
 *    reduced to a single large Ping struct.
 *
 *    The sign conventions are explained below. Height is the height below
 *    datum (measured positive down) and combines the heave and datum
 *    offset, which could come from GPS height or tide, for example.
 *
 *    Tide is measured with the usual marine convention, positive up (but
 *    remember it is already applied)
 *
 *    The number of samples (points) stored in the array is stored in
 *    nosampsfile.  The number of samples stored in the original array was
 *    nosampsorig, but some of these samples may have been rejected by
 *    filters and not written to the file.
 *
 *    The data array contains the three-dimensional position (XYZ) and
 *    amplitude derived from the sonar data.  The data points are not
 *    ordered in any pariticular way, but they will usually be stored in the
 *    order of the time in which the underlying phase data was collected.
 *    The status field in each sample gives information about the status of
 *    the data point.  A value of zero indicates that the point has been
 *    rejected by a filter.
 *
 * 3. The SWPLS_ID_XYZA_PING format was used in the processed data files
 *    written by SWATHplus code distributed before January 2010.  The ping
 *    data element is the same as in SWPLS_ID_XYZA_PING2.  Within the
 *    transducer data the position offset sub-element contains one fewer
 *    fields, and the individual data points do not contain a TPU estimate.
 *
 *    Because SWPLS_ID_XYZA_PING is a subset of SWPLS_ID_XYZA_PING2, MB
 *    System uses the newer format exclusively.  Older data is detected on
 *    read and converted into a PING2 style structure with the missing
 *    elements zeroed out.  On write, all data is written using
 *    SWPLS_ID_XYZA_PING2 format.
 *
 * 4. The position coordinates in an SXP file use a user-defined
 *    projected coordinate system (PCS). MB System assumes that the PCS is
 *    Cartesian, that both the transducers and points use the same PCS and
 *    datum, that the linear measure is in meters (no data with US measures
 *    have been tested).  Finally, each SXP file should be accompanied by a
 *    PRJ file identifying the correct PCS for automatic conversion of the
 *    PCS to geographic coordinates during import.
 *
 *    If these conditions are met, on read, MB System will properly convert
 *    the SXP data into the internal MB System formats and reverse the
 *    process on write.  Other configuratins of the PCS and/or linear
 *    measures are likely to lead to scrambled eggs.
 *
 *    Note: The ping.easting/ping.northing and ping.txer_e/ping.txer_n are
 *    identical and point to the position of the transducer.
 *
 * 5. The coordinates of the transducer (ping.txer_e, ping.txer_n,
 *    ping.height) and the points (points.x, points.y, points.z) are
 *    intended by BathySwath to be fully reduced, having all applicable
 *    lever arms, dynamic GPS heights, tide levels, svp ray-tracing, etc
 *    adjustments applied in the SWATH processor software prior to creating
 *    the SXP file.  It is not possible to reverse the applied processing
 *    using only the data stored in the SXP file itself.  For this reason,
 *    it is not safe to re-calculate the position of samples stored in the
 *    SXP format.  Instead, users should return to the raw data and export
 *    the data in the SXI or GSF formats.
 *
 *    Filtering, backscatter processing, and gridding of SXP data work well.
 *
 * 6. BathySwath sonars in the field typically contain 2 or 3 transducers,
 *    and these can be fired in one of 4 modes: passive (listen only),
 *    single-transducer, alternating-transducers, and simultaneous mode.
 *    Regardless of the mode, each transducer fired writes a complete ping
 *    data block.  In simultaneous mode, with all transducers fireing at the
 *    same time, each of these ping blocks will contain the same time stamp.
 *    This causes problems with some MB System filters which see the
 *    repeated time stamp as an error and filter out the 2nd and 3rd ping in
 *    the series.  The 2nd and 3rd transducers are sometimes also flaged by
 *    filteing programs (mbclean, mbedit) because the vessel hasn't moved
 *    between pings.  It is tempting to combine the port and stbd
 *    transducers into a psuedo-ping, but this doesn't work for systems with
 *    3 transducers, nor does it work for alternate ping mode since there
 *    has been vessel movement between the port and stbd pings in this case.
 *
 *    Rather than add a bunch of special cases to MBIO, I created
 *    mbsxppreprocess to break apart sxp files into 1 file per transducer.
 *    This way, each transducer is processed like a stand-alone multibeam
 *    and all of the above transducer configurations and firing modes can be
 *    accomidated in MBIO without any special cases.  Furthermore mbclean,
 *    mbedit and mbeditviz work transparently with the single-transducer
 *    files.
 *
 * 7. I have compared the results of data filtered and gridded by MB System
 *    to the same data processed with SEA's own software and the differences
 *    were negligible (on the order of a few cm across the grid).  More
 *    extensive testing will be done over the next couple of months, but I
 *    think this version of the code is ready for wider scrutiny.
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_swathplus.h"

#define MBF_SWPLSSXP_DEBUG 1

int mbsys_swathplus_ping_init(int verbose, swplssxp_ping *ping, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------
   mbsys_swathplus_alloc

   Allocate and initialize memory for the sonar-specific data structure.
   This function is called automatically (once? on each block read?) by MBIO before
   reading a sonar data file.

   See: :/mbsys_swathplus_deall/

   HISTORY:

   2013-08-29 - Function defined for SWATHplus by DPF

   ---------------------------------------------------------------------*/
int mbsys_swathplus_alloc
(
	int verbose,		/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,		/* in: see mb_io.h:/^struct mb_io_struct/ */
	void **store_ptr,	/* in: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int *error			/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char *function_name = "mbsys_swathplus_alloc";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	int i;

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
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* allocate memory for data structure */
	status = mb_mallocd(verbose,
		__FILE__,
		__LINE__,
		sizeof(struct mbsys_swathplus_struct),
		(void **)store_ptr,
		error);

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) *store_ptr;


	/* initialize everything */
	store->kind = MB_DATA_NONE;
	store->type = SWPLS_ID_NONE;

	store->projection_set = MB_NO;
	store->projection_id[0] = '\0';

	store->stored_header = MB_NO;
	store->stored_ping = MB_NO;
	store->stored_comment = MB_NO;

	/* initialize file header datagram */
	store->header.swver = 0;
	store->header.fmtver = 0;

	/* initialize ping datagram */
	mbsys_swathplus_ping_init(verbose, &(store->ping), error);

	/* initialize comment */
	memset(store->comment, '\0', MB_COMMENT_MAXLINE);

	/* initialize storage for translated data */
	for (i = 0; i < 7; i++)
		store->time_i[i] = 0;
	store->time_d = 0.0;
	store->navlon = 0.0;
	store->navlat = 0.0;
	store->heading = 0.0;
	store->speed = 0.0;
	store->draft = 0.0;
	store->sos = 0.0;
	store->beamwidth_xtrack = 0.0;
	store->beamwidth_ltrack = 0.0;
	store->nbath = 0;
	store->namp = 0;
	store->nss = 0;
	for (i = 0; i < MBSYS_SWPLS_MAX_BEAMS; i++)
		{
		store->beamflag[i] = MB_FLAG_NONE;
		store->bath[i] = 0.0;
		store->amp[i] = 0;
		store->bathacrosstrack[i] = 0.0;
		store->bathalongtrack[i] = 0.0;
		store->ss[i] = 0;
		store->ssacrosstrack[i] = 0.0;
		store->ssalongtrack[i] = 0.0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       store_ptr:  %p\n", *store_ptr);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return status;
}					/* mbsys_swathplus_alloc */
/*----------------------------------------------------------------------
    mbsys_swathplus_survey_alloc

    TODO: What does this function do and when is it called?

   ----------------------------------------------------------------------*/
int mbsys_swathplus_survey_alloc
(
	int verbose,	/* in: verbosity level set at command line */
	void *mbio_ptr,	/* in: see mb_io.h:/^struct mb_io_struct/ */
	void *store_ptr,/* in: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int *error		/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char    *function_name = "mbsys_swathplus_survey_alloc";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;

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

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) store_ptr;

	/* print output debug statements */
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
}						/* mbsys_swathplus_survey_alloc */
/*----------------------------------------------------------------------
    mbsys_swathplus_deall

    This function is called automatically by MBIO to free all memory
    allocated by mbsys_swathplus_alloc (that is, all of the dynamically
    allocated memory in the sonar-specific struct).

    TODO: When and how often is this function called?

    See: :0+/mbsys_swathplus_alloc/

   ---------------------------------------------------------------------*/
int mbsys_swathplus_deall
(
	int verbose,		/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,		/* in: see mb_io.h:/^struct mb_io_struct/ */
	void **store_ptr,	/* in: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int *error			/* out: see mb_status.h:/error values/ */
)
{
	char *function_name = "mbsys_swathplus_deall";
	int status = MB_SUCCESS;
	struct mbsys_swathplus_struct *store;

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
		fprintf(stderr, "dbg2       store_ptr:  %p\n", *store_ptr);
		}

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) *store_ptr;

	/* deallocate memory for data structure */
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

	/* print output debug statements */
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
}					/* mbsys_swathplus_deall */
/*----------------------------------------------------------------------
    mbsys_swathplus_dimensions

    Returns the maximum numbers of beams and pixels associated with the
    SXP data format.

    HISTORY:

    2013-08-29 - Function defined for SXP by DPF
   ----------------------------------------------------------------------*/
int mbsys_swathplus_dimensions
(
	int verbose,
	void *mbio_ptr,	/* in: verbosity level set on command line 0..N */
	void *store_ptr,/* in: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int *kind,		/* in: see mb_status.h:0+/MBIO data type/ */
	int *nbath,		/* out: number of bathymetric samples 0..MBSYS_SWPLS_MAX_BEAMS */
	int *namp,		/* out: number of amplitude samples 0..MBSYS_SWPLS_MAX_BEAMS */
	int *nss,		/* out: number of sidescan samples 0..MBSYS_SWPLS_MAX_BEAMS */
	int *error		/* out: see mb_status.h:/error values/ */
)
{
	char    *function_name = "mbsys_swathplus_dimensions";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract beam and pixel numbers from structure */
	if (*kind == MB_DATA_DATA)
		{
		*nbath = store->nbath;
		*namp = store->namp;
		*nss = store->nss;
		}
	else
		{
		*nbath = 0;
		*namp = 0;
		*nss = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		fprintf(stderr, "dbg2        namp:      %d\n", *namp);
		fprintf(stderr, "dbg2        nss:       %d\n", *nss);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* print return status */
	return status;
}					/* mbsys_swathplus_dimensions */
/*--------------------------------------------------------------------
    mbsys_swathplus_pingnumber

    Return the ping number of the current record.

    History:

    2013-08-01 - Function re-defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbsys_swathplus_pingnumber
(
	int verbose,	/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,	/* in: see mb_io.h:/^struct mb_io_struct/ */
	int *pingnumber,/* out: swathplus ping number */
	int *error		/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char    *function_name = "mbsys_swathplus_pingnumber";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;

	assert(mbio_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) mb_io_ptr->store_data;

	/* extract ping number from structure */
	*pingnumber = store->ping.pingnumber;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       pingnumber: %d\n", *pingnumber);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return status;
}							/* mbsys_swathplus_pingnumber */
/*--------------------------------------------------------------------
    mbsys_swathplus_extract

    Extract sonar data from the structure pointed to by store_ptr according
    to the MBIO descriptor pointed to by mbio_ptr.  The verbose value
    controls the standard error output verbosity of the function.  The kind
    value indicates which type of record is stored in *store_ptr.
    Additional data is returned if the data record is survey data
    (navigation, bathymetry, amplitude, and sidescan), navigation data
    (navigation only), or comment data (comment only).

    See: :/mbsys_swathplus_insert/

    History:

    2013-08-01 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbsys_swathplus_extract
(
	int verbose,			/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,			/* in: see mb_io.h:/^struct mb_io_struct/ */
	void *store_ptr,		/* in: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int *kind,				/* out: MBIO data type; see mb_status.h:0+/MBIO data type/ */
	int time_i[7],			/* out: MBIO time array; see mb_time.c:0+/mb_get_time/ */
	double *time_d,			/* out: MBIO time (seconds since 1,1,1970) */
	double *navlon,			/* out: transducer longitude -180.0..+180.0 */
	double *navlat,			/* out: transducer latitude -180.0..+180.0 */
	double *speed,			/* out: vessel speed (km/s) */
	double *heading,		/* out: vessel heading -180.0..+180.0 */
	int *nbath,				/* out: number of bathymetry samples (beams) */
	int *namp,				/* out: number of amplitude samples, usually namp = nbath */
	int *nss,				/* out: number of side scan pixels */
	char *beamflag,			/* out: array[nbath] of beam flags; see mb_status.h:/FLAG category/ */
	double *bath,			/* out: array[nbath] of depth values (m) positive down */
	double *amp,			/* out: array[namp] of amplitude values */
	double *bathacrosstrack,/* out: array[nbath] bathy across-track offsets from transducer (m) */
	double *bathalongtrack,	/* out: array[nbath] bathy along-track offsets from transducer (m) */
	double *ss,				/* out: array[nss] sidescan pixel values */
	double *ssacrosstrack,	/* out: array[nss] sidescan across-track offsets from transducer (m) */
	double *ssalongtrack,	/* out: array[nss] sidescan along-track offsets from transducer (m) */
	char *comment,			/* out: comment string (not supported by SWATHplus SXP) */
	int *error				/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char    *function_name = "mbsys_swathplus_extract";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(time_i != NULL);
	assert(beamflag != NULL);
	assert(bath != NULL);
	assert(amp != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from store and copy into mb-system slots */
	if (*kind == MB_DATA_DATA)
		{
		/* copy the scalar ping data */
		memcpy(time_i, &(store->time_i), 7*sizeof(time_i[0]));
		mb_get_time(verbose, time_i, time_d);
		*navlon = store->navlon;
		*navlat = store->navlat;
		*speed = store->speed;
		*heading = store->heading;
		*nbath = store->nbath;
		*namp = *nbath;
		*nss = store->nss;

		/* we are poking into the mb_io_ptr to change the beamwidth here */
		mb_io_ptr->beamwidth_xtrack = store->beamwidth_xtrack;
		mb_io_ptr->beamwidth_ltrack = store->beamwidth_ltrack;

		/* copy the bathymetry-related arrays */
		if ((*nbath < 0) || (MBSYS_SWPLS_MAX_BEAMS <= *nbath))
			{
			fprintf(stderr, "%s: %d: nbath is out of bounds (%d)\n", function_name, __LINE__,
				*nbath);
			*error = MB_ERROR_BAD_DATA;
			status = MB_FAILURE;
			}
		else
			{
			memcpy(beamflag, &(store->beamflag), *nbath*sizeof(beamflag[0]));
			memcpy(bath, &(store->bath), *nbath*sizeof(bath[0]));
			memcpy(amp, &(store->amp), *nbath*sizeof(amp[0]));
			memcpy(bathacrosstrack, &(store->bathacrosstrack), *nbath*sizeof(bathacrosstrack[0]));
			memcpy(bathalongtrack, &(store->bathalongtrack), *nbath*sizeof(bathalongtrack[0]));
			}

		/* copy the sidescan-related arrays */
		if ((*nss < 0) || (MBSYS_SWPLS_MAX_BEAMS <= *nss))
			{
			fprintf(stderr, "%s: %d: nss is out of bounds (%d)\n", function_name, __LINE__, *nbath);
			*error = MB_ERROR_BAD_DATA;
			status = MB_FAILURE;
			}
		else
			{
			memcpy(ss, &(store->ss), *nss*sizeof(ss[0]));
			memcpy(ssacrosstrack, &(store->ssacrosstrack), *nss*sizeof(ssacrosstrack));
			memcpy(ssalongtrack, &(store->ssalongtrack), *nss*sizeof(ssalongtrack));
			}
		}

	if (*kind == MB_DATA_COMMENT)
		strncpy(comment, store->comment, MB_COMMENT_MAXLINE);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return status;
}							/* mbsys_swathplus_extract */
/*--------------------------------------------------------------------
    mbsys_swathplus_insert

    This function inserts sonar data into the structure pointed to by
* store_ptr according to the MBIO descriptor pointed to by mbio_ptr. The verbose
    value controls the standard error output verbosity of the function. The
    data will be inserted only if th edata record is survey data
    (navigation, bathymetry, amplitude, and sidescan), navigation data
    (navigation only), or comment data (comment only).

    See: :0+/mbsys_swathplus_extract/

    History:

    2013-08-01 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbsys_swathplus_insert
(
	int verbose,			/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,			/* in: see mb_io.h:/^struct mb_io_struct/ */
	void *store_ptr,		/* in: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int kind,				/* in: see mb_status.h:0+/MBIO data type/ */
	int time_i[7],			/* in: see mb_time.c:0+/mb_get_time/ */
	double time_d,			/* in: time in seconds since 1,1,1970) */
	double navlon,			/* in: transducer longitude -180.0..+180.0 */
	double navlat,			/* in: transducer latitude -180.0..+180.0 */
	double speed,			/* in: vessel speed (m/s) */
	double heading,			/* in: vessel heading -180.0..+180.0 */
	int nbath,				/* in: number of bathymetry samples/beams */
	int namp,				/* in: number of amplitude samples, usually namp == nbath */
	int nss,				/* in: number of sidescan pixels */
	char *beamflag,			/* in: array[nbath] of beam flags; see mb_status.h:/FLAG category/ */
	double *bath,			/* in: array[nbath] of depth values (m) positive down */
	double *amp,			/* in: array[namp] of amplitude values */
	double *bathacrosstrack,/* in: array[nbath] bathy across-track offsets from transducer (m) */
	double *bathalongtrack,	/* in: array[nbath] bathy along-track offsets from transducer (m) */
	double *ss,				/* in: array[nss] sidescan pixel values */
	double *ssacrosstrack,	/* in: array[nss] sidescan across-track offsets from transducer (m) */
	double *ssalongtrack,	/* in: array[nss] sidescan along-track offsets from transducer (m) */
	char *comment,			/* in: comment string (not supported by SWATHplus SXP) */
	int *error				/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char    *function_name = "mbsys_swathplus_insert";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(time_i != NULL);
	assert(0 <= nbath && nbath < MBSYS_SWPLS_MAX_BEAMS);
	assert(0 <= namp && namp < MBSYS_SWPLS_MAX_BEAMS);
	assert(namp == nbath);
	assert(0 <= nss && nss < MBSYS_SWPLS_MAX_BEAMS);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       kind:       %d\n", kind);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) store_ptr;

	/* get data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		memcpy(&(store->time_i[0]), time_i, 7*sizeof(time_i[0]));
		store->time_d = time_d;
		store->navlon = navlon;
		store->navlat = navlat;
		store->speed = speed;
		store->heading = heading;
		store->nbath = nbath;
		store->namp = namp;
		store->nss = nss;

		/* insert the bathymetry data */
		if ((nbath < 0) || (MBSYS_SWPLS_MAX_BEAMS <= nbath))
			{
			fprintf(stderr, "%s: %d: nbath is out of bounds (%d)\n", function_name, __LINE__,
				nbath);
			*error = MB_ERROR_BAD_DATA;
			status = MB_FAILURE;
			}
		else
			{
			memcpy(store->beamflag, beamflag, nbath*sizeof(beamflag[0]));
			memcpy(store->bath, bath, nbath*sizeof(bath[0]));
			memcpy(store->amp, amp, nbath*sizeof(amp[0]));
			memcpy(store->bathacrosstrack, bathacrosstrack, nbath*sizeof(bathacrosstrack[0]));
			memcpy(store->bathalongtrack, bathalongtrack, nbath*sizeof(bathalongtrack[0]));
			}

		/* insert the sidescan pixel data */
		if ((nss < 0) || (MBSYS_SWPLS_MAX_BEAMS <= nss))
			{
			fprintf(stderr, "%s: %d: nss is out of bounds (%d)\n", function_name, __LINE__, nbath);
			*error = MB_ERROR_BAD_DATA;
			status = MB_FAILURE;
			}
		else
			{
			memcpy(store->ss, ss, nss*sizeof(ss[0]));
			memcpy(store->ssacrosstrack, ssacrosstrack, nss*sizeof(ssacrosstrack[0]));
			memcpy(store->ssalongtrack, ssalongtrack, nss*sizeof(ssalongtrack[0]));
			}
		}

	/* deal with comments */
	else if (store->kind == MB_DATA_COMMENT)
		{
		store->time_d = time_d;
		store->stored_comment = MB_YES;
		strncpy(store->comment, comment, MB_COMMENT_MAXLINE);
		}

	/* deal with other records types  */
	else
		{
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 4)
		mbsys_swathplus_print_store(verbose, store, error);
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return status;
}							/* mbsys_swathplus_insert */
/*--------------------------------------------------------------------
    mbsys_swathplus_ttimes

    Retrieve sample raw travel times and projection angles for low-level
    processing.

    Note from Dave:

    We set *traveltime = MB_NO;
    for formats that do not include travel time data. This disables
    bathymetry recalculation by mbprocess.

    The mbsys_*_ttimes () function should be defined even if there are no
    travel times. The function can return zero travel times for the beams.This
    will change for MB6 - access functions will be left null for data that
    do not exist .

    See: mbr_swplssxp.c:/int mbr_rt_swplssxp/

    History:

    2013-08-01 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbsys_swathplus_ttimes
(
	int verbose,			/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,			/* in: see mb_io.h:/^struct mb_io_struct/ */
	void *store_ptr,		/* in: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int *kind,				/* out: MBIO data type; see mb_status.h:0+/MBIO data type/ */
	int *nbeams,			/* out: number of beams (samples) in this ping */
	double *ttimes,			/* out: array[nbeams] travel time of beam (secs) */
	double *angles,			/* out: array[nbeams] across-track angle of beam (deg) */
	double *angles_forward,	/* out: array[nbeams] along-track angle of beam (deg) */
	double *angles_null,	/* out: array[nbeams] ?? */
	double *heave,			/* out: array[nbeams] heave for each beam ?? */
	double *alongtrack_offset,	/* out: array[nbeams] ?? */
	double *draft,			/* out: draft of transducer below waterline ?? (m) */
	double *ssv,			/* out: sound velocity at head (m/s) */
	int *error				/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char    *function_name = "mbsys_swathplus_ttimes";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	int i;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(ttimes != NULL);
	assert(angles != NULL);
	assert(angles_forward != NULL);
	assert(angles_null != NULL);
	assert(heave != NULL);
	assert(alongtrack_offset != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		}

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structre pointer */
	store = (struct mbsys_swathplus_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract travel time data */
	if (*kind == MB_DATA_DATA)
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		*nbeams = store->ping.nosampsfile;
		if ((*nbeams < 0) || (MBSYS_SWPLS_MAX_BEAMS <= *nbeams))
			{
			fprintf(stderr,
				"%s:%d: nbeams is out of bounds (%d)\n",
				function_name,
				__LINE__,
				*nbeams);
			*error = MB_ERROR_OTHER;
			status = MB_FAILURE;
			}
		else
			{
			for (i = 0; i < *nbeams; i++)
				{
				ttimes[i] = 0.0;
				angles[i] = 0.0;
				angles_forward[i] = 0.0;
				angles_null[i] = 0.0;
				heave[i] = 0.0;
				alongtrack_offset[i] = 0.0;
				}
			*draft = store->draft;
			*ssv = store->sos;
			}
		}
	/* deal with comment record type */
	else if (*kind == MB_DATA_COMMENT)
		{
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}
	/* deal with other record types */
	else
		{
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debu statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return status;
}					/* mbsys_swathplus_ttimes */
/*--------------------------------------------------------------------
    mbsys_swathplus_detects

    Set the bottom detection algorithm used by the sonar for each beam

    History:

    2013-08-01 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbsys_swathplus_detects
(
	int verbose,	/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,	/* in: see mb_io.h:/^struct mb_io_struct/ */
	void *store_ptr,/* in: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int *kind,		/* out: MBIO data type; see mb_status.h:0+/MBIO data type/ */
	int *nbeams,	/* out: number of beams (samples) in this ping */
	int *detects,	/* out: array[nbeams] detection flag;
					    see mb_status.h:/Bottom detect flags/ */
	int *error		/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char    *function_name = "mbsys_swathplus_detects";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	int i;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       detects:    %p\n", detects);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		*nbeams = store->ping.nosampsfile;
		if ((*nbeams < 0) || (MBSYS_SWPLS_MAX_BEAMS <= *nbeams))
			{
			fprintf(stderr, "%s:%d: nbeams is out of bound (%d)\n", function_name, __LINE__,
				*nbeams);
			*error = MB_ERROR_OTHER;
			status = MB_FAILURE;
			}
		else
			{
			for (i = 0; i < *nbeams; i++)
				detects[i] = MB_DETECT_AMPLITUDE;
			}
		}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
		{
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}

	/* deal with other record type */
	else
		{
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		}
	if ((verbose >= 2) && (*error == MB_ERROR_NO_ERROR))
		{
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (i=0; i<*nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: detects:%d\n", i, detects[i]);
		}
	if (verbose >= 2)
		{
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return status;
}					/* mbsys_swathplus_detects */
/*--------------------------------------------------------------------
    mbsys_swathplus_pulses

    Return source pulse type for each beam.

    See:	mbsys_simrad.c:/mbsys_simrad_pulses/
            mb_status.h:/Source pulse type/

    TODO: Maybe this function should be left blank for the SWATHplus?

    History:

    2013-08-01 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbsys_swathplus_pulses
(
	int verbose,	/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,	/* in: see mb_io.h:/^struct mb_io_struct/ */
	void *store_ptr,/* in: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int *kind,		/* out: MBIO data type; see mb_status.h:0+/MBIO data type/ */
	int *nbeams,	/* out: number of beams (samples) in this ping */
	int *pulses,	/* out: array[nbeams] pulse type; see mb_status.h:/Source pulse/ */
	int *error		/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char    *function_name = "mbsys_swathplus_pulses";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	int i;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       pulses:     %p\n", pulses);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		*nbeams = store->ping.nosampsfile;
		if ((*nbeams < 0) || (MBSYS_SWPLS_MAX_BEAMS <= *nbeams))
			{
			fprintf(stderr, "%s:%d: nbeams is out of bound (%d)\n", function_name, __LINE__,
				*nbeams);
			*error = MB_ERROR_OTHER;
			status = MB_FAILURE;
			}
		else
			{
			for (i = 0; i < *nbeams; i++)
				pulses[i] = MB_PULSE_UNKNOWN;
			}
		}

	/* deal with comments */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}

	/* deal with other record type */
	else
		{
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		}
	if (( verbose >= 2) && ( *error == MB_ERROR_NO_ERROR) )
		{
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (i=0; i<*nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: pulses:%d\n", i, pulses[i]);
		}
	if (verbose >= 2)
		{
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return status;
}							/* mbsys_swathplus_pulses */
/*--------------------------------------------------------------------
    mbsys_swathplus_gains

    Return the pulse length, transmit- and receive-gains of the system.

    NOTE:

    I need to contact SEA (Bathyswath) to get a better idea of what
    these values stored in the SXP file actually mean. For now, they
    are just digital numbers. /DPF

    History:

    2013-08-01 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbsys_swathplus_gains
(
	int verbose,			/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,			/* in: see mb_io.h:/^struct mb_io_struct/ */
	void *store_ptr,		/* in: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int *kind,				/* in: MBIO data type; see mb_status.h:0+/MBIO data type/ */
	double *transmit_gain,	/* out: transmit gain (dB) */
	double *pulse_length,	/* out: pulse width (usec) */
	double *receive_gain,	/* out: receive gain (dB) */
	int *error				/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char    *function_name = "mbsys_swathplus_gains";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* get transmit_gain (dB) [I don't know how to convert this to db]*/
		*transmit_gain = (double)store->ping.txpower;

		/* get pulse_length (usec) */
		*pulse_length = (double)store->ping.trnstime/store->ping.frequency*1e6;

		/* get receive_gain (dB) [I don't know how to convert to db] */
		*receive_gain = (double)store->ping.analoggain;
		}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
		{
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}

	/* deal with other record types */
	else
		{
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		}
	if ((verbose >= 2) && (*error == MB_ERROR_NO_ERROR))
		{
		fprintf(stderr, "dbg2       transmit_gain: %f\n", *transmit_gain);
		fprintf(stderr, "dbg2       pulse_length:  %f\n", *pulse_length);
		fprintf(stderr, "dbg2       receive_gain:  %f\n", *receive_gain);
		}
	if (verbose >= 2)
		{
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	/* return status */
	return status;
}								/* mbsys_swathplus_gains */
/*--------------------------------------------------------------------
    mbsys_swathplus_extract_altitude

    Extract the sonar transducer depth (transducer_depth) below the sea
    surface and the sonar transducer alititude above the seafloor according
    to the MBIO descriptor pointed to by mbio_ptr.  These data are returned
    only if the data record is survey data.  These values are useful for
    sidescan processing applications.  Both transducer depths and altitudes
    are reported in meters.

    History:

    2013-08-02 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbsys_swathplus_extract_altitude
(
	int verbose,				/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,				/* in: see mb_io.h:/^struct mb_io_struct/ */
	void *store_ptr,			/* in: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int *kind,					/* in: MBIO data type; see mb_status.h:0+/MBIO data type/ */
	double *transducer_depth,	/* out: transducer depth below water line (m) */
	double *altitude,			/* out: transducer altitude above seafloor (m) */
	int *error					/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char    *function_name = "mbsys_swathplus_extract_altitude";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	int i, n;
	double sum, ave;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		i = n = 0;
		sum = 0.0;
		while(i < store->ping.nosampsfile && n < 100)
			{
			if (store->ping.points[i].status != SWPLS_POINT_REJECTED)
				{
				sum += store->ping.points[i].z;
				n++;
				}
			i++;
			}

		/* return the mean depth value or zero */
		if (n > 0)
			{
			ave = sum / n;
			*altitude = ave - store->ping.height;
			}
		else
			{
			*altitude = 0.0;
			}

		/* sxp transducer height is depth below datum, can't assume the datum
		   is the waterline. */

		if (store->ping.txer_waterdepth > 0.0)
			/* if defined, use the static transducer water depth (surface vessel) */
			*transducer_depth = store->ping.txer_waterdepth;
		else
			/* if not defined, hope ping.height is transducer depth */
			*transducer_depth = store->ping.height;

		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
		{
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}

	/* deal with other record type */
	else
		{
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       transducer_depth:  %f\n", *transducer_depth);
		fprintf(stderr, "dbg2       altitude:          %f\n", *altitude);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
		}

	/* return status */
	return status;
}					/* mbsys_swathplus_extract_altitude */
/*--------------------------------------------------------------------
    mbsys_swathplus_extract_nnav

    Extract multiple navigation values from the data file.

    Most values are self-explainitory. However, draft is complicated
    because the SXP format stores a single height value for the
    transducer which is the fully reduced

    TODO: What happends if nmax < n?

    History:

    2013-08-03 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbsys_swathplus_extract_nnav
(
	int verbose,	/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,	/* in: see mb_io.h:/^struct mb_io_struct/ */
	void *store_ptr,/* in: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int nmax,		/* in: maximum size available to n; e.g., n < nmax */
	int *kind,		/* out: MBIO data type; see mb_status.h:0+/MBIO data type/ */
	int *n,			/* out: number of navigation values extracted */
	int *time_i,	/* out: array[n] time_i[7] values; see mb_time.c:0+/mb_get_time/ */
	double *time_d,	/* out: array[n] time_d values; seconds since 1,1,1970 */
	double *navlon,	/* out: array[n] longitude (degrees); -180.0..+180.0 */
	double *navlat,	/* out: array[n] latitude (degree); -90..+90 */
	double *speed,	/* out: array[n] speed (m/s) */
	double *heading,/* out: array[n] heading (degree): 0..360 */
	double *draft,	/* out: array[n] txer depth below datum (m) */
	double *roll,	/* out: array[n] roll (degrees) */
	double *pitch,	/* out: array[n] pitch (degrees) */
	double *heave,	/* out: array[n] heave (m) */
	int *error		/* out: see mb_status.h:/MB_ERROR/ */
)
{
	char    *function_name = "mbsys_swathplus_extract_nnav";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	int inav;
	int i;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(nmax > 0);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       nmax:       %d\n", nmax);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA)
		{
		/* just one navigation value */
		*n = 1;

		/* get time */
		for(i=0; i<7; i++)
			time_i[i] = store->time_i[i];
		time_d[0] = store->time_d;

		/* get navigation and heading */
		navlon[0] = store->navlon;
		navlat[0] = store->navlat;
		speed[0] = store->speed;
		heading[0] = store->heading;

		/* get draft */
		draft[0] = store->draft;

		/* get roll pitch and heave. In SXP heave is included in height. */
		roll[0] = store->roll;
		pitch[0] = store->pitch;
		heave[0] = store->heave;

		/* done translating values */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}
	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
		{
		*n = 0;
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}
	/* deal with other record type */
	else
		{
		*n = 0;
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       n:          %d\n", *n);
		for (inav=0; inav<*n; inav++)
			{
			for (i=0; i<7; i++)
				fprintf(stderr, "dbg2       %d time_i[%d]:     %d\n", inav, i,
					time_i[inav * 7 + i]);
			fprintf(stderr, "dbg2       %d time_d:        %f\n", inav, time_d[inav]);
			fprintf(stderr, "dbg2       %d longitude:     %f\n", inav, navlon[inav]);
			fprintf(stderr, "dbg2       %d latitude:      %f\n", inav, navlat[inav]);
			fprintf(stderr, "dbg2       %d speed:         %f\n", inav, speed[inav]);
			fprintf(stderr, "dbg2       %d heading:       %f\n", inav, heading[inav]);
			fprintf(stderr, "dbg2       %d draft:         %f\n", inav, draft[inav]);
			fprintf(stderr, "dbg2       %d roll:          %f\n", inav, roll[inav]);
			fprintf(stderr, "dbg2       %d pitch:         %f\n", inav, pitch[inav]);
			fprintf(stderr, "dbg2       %d heave:         %f\n", inav, heave[inav]);
			}
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return status;
}					/* mbsys_swathplus_extract_nnav */
/*--------------------------------------------------------------------
    mbsys_swathplus_extract_nav

    Extract navigation data from the structure pointed to by *store_ptr
    according to the MBIO descriptor pointed to by mbio_ptr.  The verbose
    value controls the standard error output verbosity of the function.
    Navigation data is returned only if the data record is survey data.

    History:

    2013-08-03 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbsys_swathplus_extract_nav
(
	int verbose,
	void *mbio_ptr,	/* in: verbosity level set on command line 0..N */
	void *store_ptr,/* in: see mb_io.h:/^struct mb_io_struct/ */
	int *kind,		/* out: see mbsys_swathplus.h:/^struct mbsys_swathplus_struct/ */
	int time_i[7],	/* out: time_i[7] values; see mb_time.c */
	double *time_d,	/* out: time in seconds since 1,1,1970 */
	double *navlon,	/* out: longitude (degrees) -180..+180.0 */
	double *navlat,	/* out: latittude (degrees) -90..+90 */
	double *speed,	/* out: speed (km/s) */
	double *heading,/* out: heading (degrees) 0..360 */
	double *draft,	/* out: draft (m) */
	double *roll,	/* out: roll (degrees) */
	double *pitch,	/* out: pitch (degrees) */
	double *heave,	/* out: heave (degrees) */
	int *error		/* out: see mb_status.h:MB_ERROR */
)
{
	char    *function_name = "mbsys_swathplus_extract_nav";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	int i;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure */
	store = (struct mbsys_swathplus_struct *) store_ptr;

	/* extract data from structure */
	*kind = store->kind;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA)
		{
		for(i=0; i<7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;
		*navlon = store->navlon;
		*navlat = store->navlat;
		*speed = store->speed;
		*heading = store->heading;
		*draft = store->draft;
		*roll = store->roll;
		*pitch = store->pitch;
		*heave = store->heave;
		}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
		{
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}
	/* deal with other record type */
	else
		{
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		}
	if ((verbose >= 2) && (*error <= MB_ERROR_NO_ERROR) && (*kind == MB_DATA_DATA))
		{
		fprintf(stderr, "dbg2       time_i[0]:     %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:     %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:     %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:     %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:     %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:     %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:     %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:        %f\n", *time_d);
		fprintf(stderr, "dbg2       longitude:     %f\n", *navlon);
		fprintf(stderr, "dbg2       latitude:      %f\n", *navlat);
		fprintf(stderr, "dbg2       speed:         %f\n", *speed);
		fprintf(stderr, "dbg2       heading:       %f\n", *heading);
		fprintf(stderr, "dbg2       draft:         %f\n", *draft);
		fprintf(stderr, "dbg2       roll:          %f\n", *roll);
		fprintf(stderr, "dbg2       pitch:         %f\n", *pitch);
		fprintf(stderr, "dbg2       heave:         %f\n", *heave);
		}
	if (verbose >= 2)
		{
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return status;
}					/* mbsys_swathplus_extract_nav */
/*----------------------------------------------------------------------
    mbsys_swathplus_insert_nav

    Insert new/modified navigation data from MB-System internal format
    into the sonar-specific struct.

    HISTORY:

    2013-08-05 - Function defined for SWATHplus - DPF
   ----------------------------------------------------------------------*/
int mbsys_swathplus_insert_nav
(
	int verbose,
	void *mbio_ptr,	/* in: verbosity level set on command line */
	void *store_ptr,/* in: see mb_io.h:mb_io_struct */
	int time_i[7],	/* in: time_i struct; see mb_time.c */
	double time_d,	/* in: time in seconds since 1,1,1970 */
	double navlon,	/* in: longitude in degrees -180..+180 */
	double navlat,	/* in: latitude in degrees -90..+90 */
	double speed,	/* in: speed (m/s) */
	double heading,	/* in: heading (degrees) */
	double draft,	/* in: draft (m) */
	double roll,	/* in: roll (degrees) */
	double pitch,	/* in: pitch (degreees) */
	double heave,	/* in: heave (m) */
	int *error		/* out: see mb_status.h:MB_ERROR */
)
{
	char    *function_name = "mbsys_swathplus_insert_nav";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	int i;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(time_i != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		fprintf(stderr, "dbg2       navlon:     %f\n", navlon);
		fprintf(stderr, "dbg2       navlat:     %f\n", navlat);
		fprintf(stderr, "dbg2       speed:      %f\n", speed);
		fprintf(stderr, "dbg2       heading:    %f\n", heading);
		fprintf(stderr, "dbg2       draft:      %f\n", draft);
		fprintf(stderr, "dbg2       roll:       %f\n", roll);
		fprintf(stderr, "dbg2       pitch:      %f\n", pitch);
		fprintf(stderr, "dbg2       heave:      %f\n", heave);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) store_ptr;

	/* insert data in swathplus data structure */
	if (store->kind == MB_DATA_DATA)
		{
		for(i=0; i<7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;
		store->navlon = navlon;
		store->navlat = navlat;
		store->speed = speed;
		store->heading = heading;
		store->draft = draft;
		store->roll = roll;
		store->pitch = pitch;
		store->heave = heave;

		/* done translating values */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	return status;
}						/* mbsys_swathplus_insert_nav */
/*----------------------------------------------------------------------
    mbsys_swathplus_extract_svp

    Extract sound velocity profile data.

    NOTE: SXP format doesn't store sound velocity profile data.

    HISTORY:

    2013-08-05 - Function defined for SWATHplus - DPF
   ----------------------------------------------------------------------*/
int mbsys_swathplus_extract_svp
(
	int verbose,		/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,		/* in: see mb_io.h:mb_io_struct */
	void *store_ptr,	/* in: see mbsys_swathplus.h:mbsys_swathplus_struct */
	int *kind,			/* out: see mb_status.h:MBIO data type */
	int *nsvp,			/* out: number of svp measurements */
	double *depth,		/* out: array[nsvp] depths (m) */
	double *velocity,	/* out: array[nsvp] velocity (m) */
	int *error			/* out: see: mb_status.h:MB_ERROR */
)
{
	char    *function_name = "mbsys_swathplus_extract_svp";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	int i;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_VELOCITY_PROFILE)
		{
		*nsvp = 0;
		/* done translating values */
		}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}

	/* deal with other record type */
	else
		{
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       nsvp:              %d\n", *nsvp);
		for (i=0; i<*nsvp; i++)
			fprintf(stderr,
				"dbg2       depth[%d]: %f   velocity[%d]: %f\n",
				i,
				depth[i],
				i,
				velocity[i]);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
		}

	/* return status */
	return status;
}						/* mbsys_swathplus_extract_svp */
/*----------------------------------------------------------------------
    mbsys_swathplus_insert_svp

    Insert a modified SVP profile into the data structure. SXP files
    do not support SVP profiles.

    HISTORY:

    2013-08-09 - Function defined for SXP format /DPF
   ----------------------------------------------------------------------*/
int mbsys_swathplus_insert_svp
(
	int verbose,		/* in: verbosity level set on command line 0..N */
	void *mbio_ptr,		/* in: mbio.h:mb_io_struct */
	void *store_ptr,	/* in: mbsys_swathplus_struct */
	int nsvp,			/* in: number of svp records to insert */
	double *depth,		/* in: array[nsvp] depth records (m) */
	double *velocity,	/* in: array[nsvp] sound velocity records (m/s) */
	int *error			/* out: see mb_status.h:MB_ERROR */
)
{
	char    *function_name = "mbsys_swathplus_insert_svp";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	int i;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(nsvp > 0);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       nsvp:       %d\n", nsvp);
		for (i=0; i<nsvp; i++)
			fprintf(stderr,
				"dbg2       depth[%d]: %f   velocity[%d]: %f\n",
				i,
				depth[i],
				i,
				velocity[i]);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_swathplus_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* NOOP SXP files do not store svp data */
		fprintf(stderr, "%s:%d: SXP files cannot store SVP data!\n", function_name, __LINE__);
		*error = MB_ERROR_DATA_NOT_INSERTED;
		status = MB_FAILURE;
		}
	/* handle comment */
	else if (store->kind == MB_DATA_COMMENT)
		{
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
		}
	/* handle other types */
	else
		{
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* return status */
	return status;
}					/* mbsys_swathplus_insert_svp */
/*----------------------------------------------------------------------
    mbsys_swathplus_copy

    Deep copy the system-specific data array

    HISTORY:

    2013-08-05 - Function defined for SWATHplus - dpf
   ----------------------------------------------------------------------*/
int mbsys_swathplus_copy
(
	int verbose,	/* in: verbosity level set on command line */
	void *mbio_ptr,	/* in: see mb_io.h:mb_io_struct */
	void *store_ptr,/* in: see mbsys_swathplus.h:mbsys_swathplus_struct */
	void *copy_ptr,	/* out: see mbsys_swathplus.h:mbsys_swathplus_struct */
	int *error		/* out: see mb_status.h:MB_ERROR */
)
{
	char    *function_name = "mbsys_swathplus_copy";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_swathplus_struct *store;
	struct mbsys_swathplus_struct *copy;

	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(copy_ptr != NULL);
	assert(store_ptr != copy_ptr);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       copy_ptr:   %p\n", copy_ptr);
		}

	/* set error status */
	*error = MB_ERROR_NO_ERROR;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_swathplus_struct *) store_ptr;
	copy = (struct mbsys_swathplus_struct *) copy_ptr;

	/* mbsys_swathplus_struct is statically allocated, shallow copy is OK */
	copy = store;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return status;
}						/* mbsys_swathplus_copy */
/*----------------------------------------------------------------------
    mbsys_swathplus_ping_init

    Initialize the swplssxp_ping data structure to default values.

    HISORY:

    2013-08-05 - definined function. DPF
   ----------------------------------------------------------------------*/
int mbsys_swathplus_ping_init
(
	int verbose,		/* in: verbosity level set on command line 0..N */
	swplssxp_ping *ping,/* in: swplssxp_ping struct to initialize */
	int *error			/* out: see mb_status.h:MB_ERROR */
)
{
	char *function_name = "mbsys_swathplus_init_ping";
	int status = MB_SUCCESS;
	int i;

	assert(ping != NULL);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2          ping:    %p\n", ping);
		}

	/* set error status */
	*error = MB_ERROR_NO_ERROR;

	/* initialize the ping struct */
	memset(ping->linename, '\0', SWPLS_MAX_LINENAME_LEN);
	ping->pingnumber = 0;
	ping->time_d = 0;
	ping->notxers = 0;
	ping->easting = 0.0;
	ping->northing = 0.0;
	ping->roll = 0.0;
	ping->pitch = 0.0;
	ping->heading = 0.0;
	ping->height = 0.0;
	ping->tide = 0.0;
	ping->sos = 0.0;
	ping->txno = 0;
	ping->txstat = 0;
	ping->txpower = 0;
	ping->analoggain = 0;
	ping->nostaves = 0;
	for (i = 0; i < SWPLS_MAX_TX_INFO; i++)
		ping->txinfo[i] = 0;
	ping->freq = 0;
	ping->frequency = 0.0;
	ping->trnstime = 0.0;
	ping->recvtime = 0.0;
	ping->samprate = 0;
	ping->nosampsorig = 0;
	ping->nosampsfile = 0;
	ping->nosampslots = 0;
	ping->txer_e = 0.0;
	ping->txer_n = 0.0;
	ping->txer_height = 0.0;
	ping->txer_forward = 0.0;
	ping->txer_starboard = 0.0;
	ping->txer_azimuth = 0.0;
	ping->txer_elevation = 0.0;
	ping->txer_skew = 0.0;
	ping->txer_time = 0.0;
	ping->txer_waterdepth = 0.0;
	ping->txer_pitch = 0.0;
	for (i = 0; i < MBSYS_SWPLS_MAX_BEAMS; i++)
		{
		ping->points[i].sampnum = 0;
		ping->points[i].y = 0.0;
		ping->points[i].x = 0.0;
		ping->points[i].z = 0.0;
		ping->points[i].amp = 0;
		ping->points[i].procamp = 0;
		ping->points[i].status = SWPLS_POINT_REJECTED;
		ping->points[i].tpu = 0.0;
		}

	/* print output debug statements */
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
}											/* mbsys_swathplus_ping_init */
/*--------------------------------------------------------------------
    mbsys_swathplus_print_store

    Print MB System storage structure to stdout

    History:

    2013-08-16 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbsys_swathplus_print_store
(
	int verbose,							/* in: verbosity level set on command line 0..N */
	struct mbsys_swathplus_struct *store,	/* in: see mbsys_swathplus.h:/mbsys_swathplus_struct/ */
	int *error								/* out: see mb_status.h:MB_ERROR */
)
{
	char    *function_name = "mbsys_swathplus_print_store";
	int status;
	int i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2         store:    %p\n", store);
		}

	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* print SWATHplus ping record information */
	fprintf(stderr, "  Structure Contents:\n");

	if (store->kind == MB_DATA_DATA)
		fprintf(stderr, "        kind: %d (MB_DATA_DATA)\n", store->kind);
	else if (store->kind == MB_DATA_HEADER)
		fprintf(stderr, "        kind: %d (MB_DATA_HEADER)\n", store->kind);
	else if (store->kind == MB_DATA_COMMENT)
		fprintf(stderr, "        kind: %d (MB_DATA_COMMENT)\n", store->kind);
	else
		fprintf(stderr, "        kind: %d (not used)\n", store->kind);

	if (store->type == SWPLS_ID_SXP_HEADER_DATA)
		fprintf(stderr, "        type: %o (SWPLS_ID_SXP_HEADER_DATA)\n", store->type);
	else if (store->type == SWPLS_ID_XYZA_PING2)
		fprintf(stderr, "        type: %o (SWPLS_ID_XYZA_PING2)\n", store->type);
	else if (store->type == SWPLS_ID_XYZA_PING)
		fprintf(stderr, "        type: %o (SWPLS_ID_XYZA_PING)\n", store->type);
	else
		fprintf(stderr, "        type: %o (not used)\n", store->type);

	fprintf(stderr, "  Projection\n");
	if (store->projection_set == MB_YES)
		fprintf(stderr, "        projection_id: %s\n", store->projection_id);
	else
		fprintf(stderr, "        projection not set\n");

	fprintf(stderr, "  Structure values\n");
	if (store->stored_header == MB_YES)
		mbsys_swathplus_print_header(0, &(store->header), error);
	if (store->stored_ping == MB_YES)
		mbsys_swathplus_print_ping(0, &(store->ping), error);
	if (store->stored_comment == MB_YES)
		fprintf(stderr, "        comment: %s\n", store->comment);

	fprintf(stderr, "  MB System translated values\n");
	for(i=0; i<7; i++)
		fprintf(stderr, "        time_i[%d]:        %d\n", i, store->time_i[i]);
	fprintf(stderr, "        time_d:           %f\n", store->time_d);
	fprintf(stderr, "        navlon:           %f\n", store->navlon);
	fprintf(stderr, "        navlat:           %f\n", store->navlat);
	fprintf(stderr, "        speed:            %f\n", store->speed);
	fprintf(stderr, "        heading:          %f\n", store->heading);
	fprintf(stderr, "        draft:            %f\n", store->draft);
	fprintf(stderr, "        roll:             %f\n", store->roll);
	fprintf(stderr, "        pitch:            %f\n", store->pitch);
	fprintf(stderr, "        heave:            %f\n", store->heave);
	fprintf(stderr, "        sos:              %f\n", store->sos);
	fprintf(stderr, "        beamwidth_xtrack: %f\n", store->beamwidth_xtrack);
	fprintf(stderr, "        beamwidth_ltrack: %f\n", store->beamwidth_ltrack);
	fprintf(stderr, "        nbath:            %d\n", store->nbath);
	fprintf(stderr, "        namp:             %d\n", store->namp);
	fprintf(stderr, "        nss:              %d\n", store->nss);
	fprintf(stderr, "\tNBATH\tFLAG\tBATH\tAMP\tXTRACK\tLTRACK\n");
	for(i=0; i<store->nbath; i++)
		fprintf(stderr,
			"\t%5d\t%5d\t%6.2f\t%.0f\t%6.2f\t%6.2f\n",
			i,
			store->beamflag[i],
			store->bath[i],
			store->amp[i],
			store->bathacrosstrack[i],
			store->bathalongtrack[i]);
	fprintf(stderr, "\tNSS\tSS\tXTRACK\tLTRACK\n");
	for(i=0; i<store->nss; i++)
		fprintf(stderr,
			"\t%d\t%f\t%f\t[A%f\n",
			i,
			store->ss[i],
			store->ssacrosstrack[i],
			store->ssalongtrack[i]);

	/* print output debug statements */
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
}							/* mbsys_swathplus_print_store */
/*--------------------------------------------------------------------
    mbsys_swathplus_print_header

    Print SWATHplus header structure to stdout

    History:

    2013-08-06 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbsys_swathplus_print_header
(
	int verbose,			/* in: verbosity level set on command line 0..N */
	swplssxp_header *header,/* in: see mbsys_swathplus.h:/mbsys_swplssxp_file_header_struct/ */
	int *error				/* out: see mb_status.h:MB_ERROR */
)
{
	char    *function_name = "mbsys_swathplus_print_header";
	char    *debug_str = "dbg2  ";
	char    *nodebug_str = "  ";
	char    *first;
	int status;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       header:     %p\n", header);
		}

	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* print SWATHplus ping record information */
	first = (verbose > 2) ? debug_str : nodebug_str;
	fprintf(stderr, "%sSWPLS_ID_SXP_HEADER_DATA Contents:\n", first);
	fprintf(stderr, "%s      swver:   %d\n", first, header->swver);
	fprintf(stderr, "%s      fmtver:  %d\n", first, header->fmtver);

	/* print output debug statements */
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
}						/* mbsys_swathplus_print_header */
/*--------------------------------------------------------------------
    mbsys_swathplus_print_ping

    Print SWATHplus structure to stdout

    History:

    2013-08-06 - Function defined for SWATHplus - DPF
   --------------------------------------------------------------------*/
int mbsys_swathplus_print_ping
(
	int verbose,		/* in: verbosity level set on command line 0..N */
	swplssxp_ping *ping,/* in: see mbsys_swathplus.h:/mbsys_swplssxp_file_header_struct/ */
	int *error			/* out: see mb_status.h:MB_ERROR */
)
{
	char *function_name = "mbsys_swathplus_print_ping";
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int status;
	int i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2     verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2     ping:       %p\n", ping);
		}

	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* print SWATHplus ping record information */
	first = (verbose > 2) ? debug_str : nodebug_str;
	fprintf(stderr, "%sSWPLS_ID_XYZA_PING2 Contents:\n", first);
	fprintf(stderr, "%s   linename: %s\n", first, ping->linename);
	fprintf(stderr, "%s   pingnumber:       %u\n", first, ping->pingnumber);
	fprintf(stderr, "%s   time_d:           %f\n", first, ping->time_d);
	fprintf(stderr, "%s   notxers:          %d\n", first, ping->notxers);
	fprintf(stderr, "%s   easting:          %f\n", first, ping->easting);
	fprintf(stderr, "%s   northing:         %f\n", first, ping->northing);
	fprintf(stderr, "%s   roll:             %f\n", first, ping->roll);
	fprintf(stderr, "%s   pitch:            %f\n", first, ping->pitch);
	fprintf(stderr, "%s   heading:          %f\n", first, ping->heading);
	fprintf(stderr, "%s   height:           %f\n", first, ping->height);
	fprintf(stderr, "%s   tide:             %f\n", first, ping->tide);
	fprintf(stderr, "%s   sos:              %f\n", first, ping->sos);
	fprintf(stderr, "%s   txno:             %u\n", first, (unsigned int)ping->txno);
	fprintf(stderr, "%s   txstat:           %u\n", first, ping->txstat);
	fprintf(stderr, "%s   txpower:          %u\n", first, ping->txpower);
	fprintf(stderr, "%s   analoggain:       %d\n", first, ping->analoggain);
	fprintf(stderr, "%s   nostaves:         %u\n", first, ping->nostaves);
	for(i=0; i<SWPLS_MAX_TX_INFO; i++)
		fprintf(stderr, "%s   txinfo[%d]:        %u\n", first,  i, ping->txinfo[i]);
	fprintf(stderr, "%s   freq:             %u\n", first, ping->freq);
	fprintf(stderr, "%s   frequency:        %f\n", first, ping->frequency);
	fprintf(stderr, "%s   trnstime:         %d\n", first, ping->trnstime);
	fprintf(stderr, "%s   recvtime:         %d\n", first, ping->recvtime);
	fprintf(stderr, "%s   samprate:         %u\n", first, ping->samprate);
	fprintf(stderr, "%s   nosampsorig:      %d\n", first, ping->nosampsorig);
	fprintf(stderr, "%s   nosampsfile:      %d\n", first, ping->nosampsfile);
	fprintf(stderr, "%s   nosampslots:      %d\n", first, ping->nosampslots);
	fprintf(stderr, "%s   txer_e:           %f\n", first, ping->txer_e);
	fprintf(stderr, "%s   txer_n:           %f\n", first, ping->txer_n);
	fprintf(stderr, "%s   offsetheight:     %f\n", first, ping->txer_height);
	fprintf(stderr, "%s   offsetforward:    %f\n", first, ping->txer_forward);
	fprintf(stderr, "%s   offsetstarboard:  %f\n", first, ping->txer_starboard);
	fprintf(stderr, "%s   offsetazimuth:    %f\n", first, ping->txer_azimuth);
	fprintf(stderr, "%s   offsetelevation:  %f\n", first, ping->txer_elevation);
	fprintf(stderr, "%s   offsetkew:        %f\n", first, ping->txer_skew);
	fprintf(stderr, "%s   offsettime:       %f\n", first, ping->txer_time);
	fprintf(stderr, "%s   offsetwaterdepth: %f\n", first, ping->txer_waterdepth);
	fprintf(stderr, "%s   offsetpitch:      %f\n", first, ping->txer_pitch);
	fprintf(stderr, "%s\tSAMPLE\tNORTHING\tEASTING\t\tDEPTH\tAMP\tPAMP\tSTAT\tTPU\n", first);
	for(i = 0; i<ping->nosampsfile && i < MBSYS_SWPLS_MAX_BEAMS; i++)
		fprintf(stderr,
			"%s\t%d\t%.2f\t%.2f\t%.2f\t%u\t%u\t%u\t%.2f\n",
			first,
			ping->points[i].sampnum,
			ping->points[i].y,
			ping->points[i].x,
			ping->points[i].z,
			ping->points[i].amp,
			ping->points[i].procamp,
			ping->points[i].status,
			ping->points[i].tpu);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       None\n");
		}

	/* return status */
	return status;
}	/* mbsys_swathplus_print_ping */
