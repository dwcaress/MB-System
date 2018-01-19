/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_3ddwissl.c	3.00	12/26/2017
 *	$Id$
 *
 *    Copyright (c) 2017-2017 by
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
 * mbsys_3ddwissl.h defines the MBIO data structures for handling data from
 * the 3DatDepth WiSSL (wide swath lidar) submarine lidar:
 *      MBF_3DDWISSL : MBIO ID 232 - 3DatDepth WiSSL vendor format
 *
 * Author:	David W. Caress
 * Date:	December 19, 2017
 *
 *
 */
/*
 * Notes on the MBSYS_3DDWISSL data structure:
 *
 * Vendor format from 3D at Depth for the WiSSL (wide swath lidar) submarine
 * lidar system delivered to MBARI in December 2017.
 *
 * Initial coding done using the WiSSL Wide Swath Subsea LiDAR Software User
 * Manual version 1.2 from December 2017.
 *
 *--------------------------------------------------------------------------------
 * Range Angle Angle data format (binary)
 *              Item	                                Value	            Bytes
 * ---------------------------------------------------------------------------------------
 * File Header
 *           Record ID – WiSSL                             0x3D47   2 (1 UINT16)
 *           File Magic Number                             0x3D08   2 (1 UINT16)
 *           File version                                  1        2 (1 UINT16)
 *           File sub version                              1        2 (1 UINT16)
 *           
 * Scan Information
 *           AZ, Cross track angle start, typical (deg)             4 (1 float32)
 *           AZ, Cross track angle end, typical (deg)               4 (1 float32)
 *           Pulses per cross track, scan line                      2 (1 UINT16)
 *           Number pulses per LOS                                  1 (1 UINT8)
 *           Scan lines per this File, Head A                       2 (1 UINT16)
 *           Scan lines per this File, Head B                       2 (1 UINT16)
 *           
 * Calibration Information
 *           Calibration Structure, Head A                          Size of calibration structure
 *           Calibration Structure, Head B                          Size of calibration structure
 * 
 * Pulse ID and Timestamp ( 1 to n Scans )
 *           Record ID – Head A or B              0x3D53, 0x3D54    2 (1 UINT16)
 *           Timestamp year (true year)                             2 (1 UINT16)
 *           Timestamp month (1-12)                                 1 (1 UINT8)
 *           Timestamp day                                          1 (1 UINT8)
 *           Timestamp days since Jan 1                             2 (1 UINT16)
 *           Timestamp hour                                         2 (1 UINT16)
 *           Timestamp minutes                                      1 (1 UINT8)
 *           Timestamp seconds                                      1 (1 UINT8)
 *           Timestamp nano seconds                                 4 (1 UINT32)
 *           Gain (laser power)                                     1 (UINT8)
 *           Digitizer temperature C                                4 (float)
 *           CTD temperature C                                      4 (float)
 *           CTD salinity psu                                       4 (float)
 *           CTD pressure dbar                                      4 (float)
 *           Index                                                  4 (float)
 *           Start processing m                                     4 (float)
 *           End processing m                                       4 (float)
 *           Pulse Count this scan line                             4 (1 UINT32)
 *
 * Laser Pulse Data ( 1 to m pulses per scan )
 *           AZ, Cross track angle (deg)                            4 (1 float32)
 *           EL, Forward track angle (deg)                          4 (1 float32)
 *           AZ, Cross track offset (m)                             4 (1 float32)
 *           EL, Forward track offset (m)                           4 (1 float32)
 *           Pulse time offset (sec)                                4 (1 float32)
 *           LOS Range 1 ( from glass front ) meters                4 (1 float32)
 *           ...
 *           LOS Range n ( from glass front ) meters                4 (1 float32) 
 *           Amplitude LOS 1 / peak of signal                       2 (1 UINT16)
 *           ...
 *           Amplitude LOS n / peak of signal                       2 (1 UINT16)
 *
 *
 * Each RAA file begins with a File Header, followed by a “Scan Information”
 * block and a “Calibration Information” block of data. Then, the file contains
 * scan line data. The data for each scan line contains: a Record ID (head
 * designator), a full timestamp, and a” Laser Pulse Data” collection of data.
 * Note, Head A and B scanlines are interleaved in the RAA file per their
 * specific time stamps.
 *
 * For example, if the sensor was configured for 250 pulses per scan line and
 * 3 LOS range measurements per pulse, the following data would be present in
 * the RAA file:
 *      File Header
 *      Scan Information
 *      Calibration Information Head A
 *      Calibration Information Head B
 *          (1) Record ID (A or B)
 *              Pulse Timestamp
 *              Pulse count this scan line
 *                  (1) Laser Pulse Data:
 *                      AZ angle
 *                      EL angle
 *                      AZ offset
 *                      EL offset
 *                      Pulse time offset
 *                      Range Data:
 *                          LOS Range 1
 *                          LOS Range 2
 *                          LOS Range 3
 *                      Intensity Data:
 *                          Intensity 1
 *                          Intensity 2
 *                          Intensity 3
 *                      ...
 *                  (250) Laser Pulse Data:
 *                      AZ angle
 *                      EL angle
 *                      AZ offset
 *                      EL offset
 *                      Pulse time offset
 *                      Range Data:
 *                          LOS Range 1
 *                          LOS Range 2
 *                          LOS Range 3
 *                      Intensity Data:
 *                          Intensity 1
 *                          Intensity 2
 *                          Intensity 3
 *                      ...
 *
 * Note: based on laser head performance, differing counts of data sets may
 * exist for Head A and B. The “.raa” file extension is used for the binary file.
 *
 *--------------------------------------------------------------------------------
 *
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
#include "mb_process.h"
#include "mb_aux.h"
#include "mbsys_3ddwissl.h"

#define MBF_3DDEPTHP_DEBUG 1

static char rcs_id[] = "$Id$";

/*-------------------------------------------------------------------- */
int mbsys_3ddwissl_alloc(int verbose,      /* in: verbosity level set on command line 0..N */
                               void *mbio_ptr,   /* in: see mb_io.h:/^struct mb_io_struct/ */
                               void **store_ptr, /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
                               int *error        /* out: see mb_status.h:/MB_ERROR/ */
                               ) {
	char *function_name = "mbsys_3ddwissl_alloc";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_3ddwissl_struct), (void **)store_ptr, error);
	//mb_io_ptr->structure_size = 0;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)*store_ptr;

	/* initialize everything */
	
	/* Type of data record */
	store->kind = MB_DATA_NONE; /* MB-System record ID */

	/* File Header */
	store->parameter_id = 0x3D47; /* 0x3D47 */
	store->magic_number = 0x3D08; /* 0x3D08 */
	store->file_version = 1; /* 1 */
	store->sub_version = 1; /* 1 = initial version from 3DatDepth, extended for MB-System */
    
    /* Scan Information */
    store->cross_track_angle_start = 0.0; /* AZ, Cross track angle start, typical (deg) */
    store->cross_track_angle_end = 0.0; /* AZ, Cross track angle end, typical (deg) */
    store->pulses_per_scan = 0; /* Pulses per cross track, scan line */
    store->soundings_per_pulse = 0; /* soundings per pulse (line of sight, or LOS) */
	store->heada_scans_per_file = 0; /* number of heada scans in this file */
	store->headb_scans_per_file = 0; /* number of headb scans in this file */
    
    /* head A calibration */
	memset((void *)&store->calibration_a, 0, sizeof(struct mbsys_3ddwissl_calibration_struct));

	memset((void *)&store->calibration_b, 0, sizeof(struct mbsys_3ddwissl_calibration_struct));
   
	/* Scan Information */
    store->record_id = MB_DATA_NONE;       /* head A (0x3D53 or 0x3D73) or head B (0x3D54 or 0x3D74) */
    store->year = 0;
    store->day = 0;
    store->jday = 0;
    store->hour = 0;
    store->minutes = 0;
    store->seconds = 0;
    store->nanoseconds = 0;

    store->gain = 0;                 /* laser power setting */
    store->digitizer_temperature = 0.0;    /* digitizer temperature degrees C */
    store->ctd_temperature = 0.0;          /* ctd temperature degrees C */
    store->ctd_salinity = 0.0;             /* ctd salinity psu */
    store->ctd_pressure = 0.0;             /* ctd pressure dbar */
    store->index = 0.0;
    store->range_start = 0.0;              /* range start processing meters */
    store->range_end = 0.0;                /* range end processing meters */
	store->pulse_count = 0;      	 	   /* pulse count for this scan */

    store->time_d = 0.0;      /* epoch time - not in data file, calculated following reading */
    store->navlon = 0.0;      /* absolute position longitude (degrees) */
    store->navlat = 0.0;      /* absolute position latitude (degrees) */
    store->sensordepth = 0.0; /* absolute position depth below sea surface (meters), includes any tide correction */
    store->heading = 0.0;      /* lidar heading (degrees) */
    store->roll = 0.0;         /* lidar roll (degrees) */
    store->pitch = 0.0;        /* lidar pitch (degrees) */
    
	store->scan_count = 0; /* global scan count */
    store->size_pulse_record_raw = 0;         /* for original logged records
                                                 * - calculated from file header values */
    store->size_pulse_record_processed = 0;   /* for extended processed records
                                                 * -  calculated from file header values */
    store->bathymetry_calculated = 0;         /* flag regarding calculation of bathymetry */
	store->num_pulses_alloc = 0;      /* array allocated for this number of pulses */
    store->pulses = NULL;

	/* comment */
	store->comment_len = 0;       /* comment length in bytes */
	memset(store->comment, 0, MB_COMMENT_MAXLINE); /* comment string */

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       store_ptr:  %p\n", *store_ptr);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return status;
} /* mbsys_3ddwissl_alloc */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl_deall(int verbose,      /* in: verbosity level set on command line 0..N */
                               void *mbio_ptr,   /* in: see mb_io.h:/^struct mb_io_struct/ */
                               void **store_ptr, /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
                               int *error        /* out: see mb_status.h:/error values/ */
                               ) {
	char *function_name = "mbsys_3ddwissl_deall";
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", *store_ptr);
	}

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)*store_ptr;

	/* deallocate pulses */
	if (store->pulses != NULL) {
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&store->pulses), error);
		store->pulses = NULL;
	}

	/* deallocate memory for data structure */
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return status;
} /* mbsys_3ddwissl_deall */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl_dimensions(int verbose, void *mbio_ptr, /* in: verbosity level set on command line 0..N */
                                    void *store_ptr, /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
                                    int *kind,       /* in: see mb_status.h:0+/MBIO data type/ */
                                    int *nbath,      /* out: number of bathymetric samples 0..MBSYS_SWPLS_MAX_BEAMS */
                                    int *namp,       /* out: number of amplitude samples 0..MBSYS_SWPLS_MAX_BEAMS */
                                    int *nss,        /* out: number of sidescan samples 0..MBSYS_SWPLS_MAX_BEAMS */
                                    int *error       /* out: see mb_status.h:/error values/ */
                                    ) {
	char *function_name = "mbsys_3ddwissl_dimensions";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract beam and pixel numbers from structure */
	if (*kind == MB_DATA_DATA) {
		*nbath = store->pulses_per_scan * store->soundings_per_pulse;
		*namp = *nbath;
		*nss = 0;
	}
	else {
		*nbath = 0;
		*namp = 0;
		*nss = 0;
	}

	/* print output debug statements */
	if (verbose >= 2) {
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
} /* mbsys_3ddwissl_dimensions */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_pingnumber(int verbose,     /* in: verbosity level set on command line 0..N */
                                    void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                    int *pingnumber, /* out: swathplus ping number */
                                    int *error       /* out: see mb_status.h:/MB_ERROR/ */
                                    ) {
	char *function_name = "mbsys_3ddwissl_pingnumber";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)mb_io_ptr->store_data;

	/* extract ping number from structure */
	*pingnumber = store->scan_count;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       pingnumber: %d\n", *pingnumber);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
} /* mbsys_3ddwissl_pingnumber */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_preprocess(int verbose,     /* in: verbosity level set on command line 0..N */
                                    void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                    void *store_ptr, /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
                                    void *platform_ptr, void *preprocess_pars_ptr, int *error) {
	char *function_name = "mbsys_3ddwissl_preprocess";
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_pulse_struct *pulse;
	struct mb_platform_struct *platform;
	struct mb_preprocess_struct *pars;
	int status = MB_SUCCESS;
	double time_d;
	int time_i[7], time_j[5];
	double navlon;
	double navlat;
	double heading; /* heading (degrees) */
	double sensordepth;
	double roll;    /* roll (degrees) */
	double pitch;   /* pitch (degrees) */
	double speed;   /* speed (degrees) */
	double mtodeglon, mtodeglat;
	double dlonm, dlatm;
	double headingx, headingy;
	int interp_status = MB_SUCCESS;
	int interp_error = MB_ERROR_NO_ERROR;
	int i, ipulse, isounding;
	int jnav = 0;
	int jsensordepth = 0;
	int jheading = 0;
	// int	jaltitude = 0;
	int jattitude = 0;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:                   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:                  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       platform_ptr:               %p\n", (void *)platform_ptr);
		fprintf(stderr, "dbg2       preprocess_pars_ptr:        %p\n", (void *)preprocess_pars_ptr);
	}

	/* always successful */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(preprocess_pars_ptr != NULL);

	/* get data structure pointers */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;
	platform = (struct mb_platform_struct *)platform_ptr;
	pars = (struct mb_preprocess_struct *)preprocess_pars_ptr;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       target_sensor:              %d\n", pars->target_sensor);
		fprintf(stderr, "dbg2       timestamp_changed:          %d\n", pars->timestamp_changed);
		fprintf(stderr, "dbg2       time_d:                     %f\n", pars->time_d);
		fprintf(stderr, "dbg2       n_nav:                      %d\n", pars->n_nav);
		fprintf(stderr, "dbg2       nav_time_d:                 %p\n", pars->nav_time_d);
		fprintf(stderr, "dbg2       nav_lon:                    %p\n", pars->nav_lon);
		fprintf(stderr, "dbg2       nav_lat:                    %p\n", pars->nav_lat);
		fprintf(stderr, "dbg2       nav_speed:                  %p\n", pars->nav_speed);
		fprintf(stderr, "dbg2       n_sensordepth:              %d\n", pars->n_sensordepth);
		fprintf(stderr, "dbg2       sensordepth_time_d:         %p\n", pars->sensordepth_time_d);
		fprintf(stderr, "dbg2       sensordepth_sensordepth:    %p\n", pars->sensordepth_sensordepth);
		fprintf(stderr, "dbg2       n_heading:                  %d\n", pars->n_heading);
		fprintf(stderr, "dbg2       heading_time_d:             %p\n", pars->heading_time_d);
		fprintf(stderr, "dbg2       heading_heading:            %p\n", pars->heading_heading);
		fprintf(stderr, "dbg2       n_altitude:                 %d\n", pars->n_altitude);
		fprintf(stderr, "dbg2       altitude_time_d:            %p\n", pars->altitude_time_d);
		fprintf(stderr, "dbg2       altitude_altitude:          %p\n", pars->altitude_altitude);
		fprintf(stderr, "dbg2       n_attitude:                 %d\n", pars->n_attitude);
		fprintf(stderr, "dbg2       attitude_time_d:            %p\n", pars->attitude_time_d);
		fprintf(stderr, "dbg2       attitude_roll:              %p\n", pars->attitude_roll);
		fprintf(stderr, "dbg2       attitude_pitch:             %p\n", pars->attitude_pitch);
		fprintf(stderr, "dbg2       attitude_heave:             %p\n", pars->attitude_heave);
		fprintf(stderr, "dbg2       n_kluge:                    %d\n", pars->n_kluge);
		for (i = 0; i < pars->n_kluge; i++)
			fprintf(stderr, "dbg2       kluge_id[%d]:                    %d\n", i, pars->kluge_id[i]);
	}

	/* change timestamp if indicated */
	if (pars->timestamp_changed == MB_YES) {
		store->time_d = pars->time_d;
		mb_get_date(verbose, pars->time_d, time_i);
		mb_get_jtime(verbose, time_i, time_j);
		store->year = time_i[0];
		store->month = time_i[1];
		store->day = time_i[2];
		store->jday = time_j[1];
		store->hour = time_i[3];
		store->minutes = time_i[4];
		store->seconds = time_i[5];
		store->nanoseconds = 1000 * ((unsigned int)time_i[6]);
	}

	/* interpolate navigation and attitude */
	time_d = store->time_d;
	mb_get_date(verbose, time_d, time_i);

	/* get nav sensordepth heading attitude values for record timestamp
	   - this will generally conform to the first pulse of the scan */
	if (pars->n_nav > 0) {
		interp_status = mb_linear_interp_longitude(verbose, pars->nav_time_d - 1, pars->nav_lon - 1, pars->n_nav, time_d,
		                                           &store->navlon, &jnav, &interp_error);
		interp_status = mb_linear_interp_latitude(verbose, pars->nav_time_d - 1, pars->nav_lat - 1, pars->n_nav, time_d,
		                                          &store->navlat, &jnav, &interp_error);
		interp_status = mb_linear_interp(verbose, pars->nav_time_d - 1, pars->nav_speed - 1, pars->n_nav, time_d, &speed, &jnav,
		                                 &interp_error);
		store->speed = (float)speed;
		// fprintf(stderr," 2: lon:%.12f lat:%.12f ", store->navlon, store->navlat);
	}
	if (pars->n_sensordepth > 0) {
		interp_status = mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
		                                 pars->n_sensordepth, time_d, &store->sensordepth, &jsensordepth, &interp_error);
	}
	if (pars->n_heading > 0) {
		interp_status = mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1, pars->n_heading,
		                                         time_d, &heading, &jheading, &interp_error);
		store->heading = (float)heading;
	}
	// if (pars->n_altitude > 0)
	//	{
	//	interp_status = mb_linear_interp(verbose,
	//				pars->altitude_time_d-1, pars->altitude_altitude-1, pars->n_altitude,
	//				time_d, &store->altitude, &jaltitude,
	//				&interp_error);
	//	}
	if (pars->n_attitude > 0) {
		interp_status = mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude, time_d,
		                                 &roll, &jattitude, &interp_error);
		store->roll = (float)roll;
		interp_status = mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude, time_d,
		                                 &pitch, &jattitude, &interp_error);
		store->pitch = (float)pitch;
		// interp_status = mb_linear_interp(verbose,
		//			pars->attitude_time_d-1, pars->attitude_heave-1, pars->n_attitude,
		//			time_d, &store->heave, &jattitude,
		//			&interp_error);
	}

	/* do lever arm correction */
	if (platform_ptr != NULL) {
		// fprintf(stderr,"Before: lon:%f lat:%f sensordepth:%f heading:%f roll:%f pitch:%f   ",
		// store->navlon,store->navlat,store->sensordepth,heading,roll,pitch);

		/* calculate sonar position position */
		status =
		    mb_platform_position(verbose, platform_ptr, pars->target_sensor, 0, store->navlon, store->navlat, store->sensordepth,
		                         heading, roll, pitch, &store->navlon, &store->navlat, &store->sensordepth, error);
		// printf(stderr,"   3: lon:%.12f lat:%.12f \n", store->navlon, store->navlat);

		/* calculate sonar attitude */
		status = mb_platform_orientation_target(verbose, platform_ptr, pars->target_sensor, 0, heading, roll, pitch, &heading,
		                                        &roll, &pitch, error);
		store->heading = (float)heading;
		store->roll = (float)roll;
		store->pitch = (float)pitch;
		// fprintf(stderr,"After: lon:%f lat:%f sensordepth:%f heading:%f roll:%f pitch:%f\n",
		// store->navlon,store->navlat,store->sensordepth,store->heading,store->roll,store->pitch);
	}

	/* get scaling */
	mb_coor_scale(verbose, store->navlat, &mtodeglon, &mtodeglat);
	headingx = sin(store->heading * DTR);
	headingy = cos(store->heading * DTR);

	/* loop over all pulses */
	for (ipulse = 0; ipulse < store->pulses_per_scan; ipulse++) {
		/* get pulse */
		pulse = (struct mbsys_3ddwissl_pulse_struct *)&store->pulses[ipulse];

		/* set time */
		pulse->time_d = store->time_d + (double)pulse->time_offset;
		
		/* initialize values */
		navlon = store->navlon;
		navlat = store->navlat;
		sensordepth = store->sensordepth;
		heading = store->heading;
		roll = store->roll;
		pitch = store->pitch;
		pulse->acrosstrack_offset = 0.0;
		pulse->alongtrack_offset = 0.0;
		pulse->sensordepth_offset = 0.0;
		pulse->heading_offset = 0.0;
		pulse->roll_offset = 0.0;
		pulse->pitch_offset = 0.0;

		/* get nav sensordepth heading attitude values for record timestamp */
		if (pars->n_nav > 0) {
			interp_status = mb_linear_interp_longitude(verbose, pars->nav_time_d - 1, pars->nav_lon - 1, pars->n_nav,
			                                           pulse->time_d, &navlon, &jnav, &interp_error);
			interp_status = mb_linear_interp_latitude(verbose, pars->nav_time_d - 1, pars->nav_lat - 1, pars->n_nav,
			                                          pulse->time_d, &navlat, &jnav, &interp_error);
			dlonm = (navlon - store->navlon) / mtodeglon;
			dlatm = (navlat - store->navlat) / mtodeglat;
			pulse->acrosstrack_offset = dlonm * headingx + dlatm * headingy;
			pulse->alongtrack_offset = dlonm * headingy - dlatm * headingx;
		}
		if (pars->n_sensordepth > 0) {
			interp_status =
			    mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1, pars->n_sensordepth,
			                     pulse->time_d, &sensordepth, &jsensordepth, &interp_error);
			pulse->sensordepth_offset = (float)(sensordepth - store->sensordepth);
		}
		if (pars->n_heading > 0) {
			interp_status = mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
			                                         pars->n_heading, pulse->time_d, &heading, &jheading, &interp_error);
			pulse->heading_offset = (float)(heading - store->heading);
		}
		if (pars->n_attitude > 0) {
			interp_status = mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
			                                 pulse->time_d, &roll, &jattitude, &interp_error);
			pulse->roll_offset = (float)(roll - store->roll);
			
			interp_status = mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
			                                 pulse->time_d, &pitch, &jattitude, &interp_error);
			pulse->pitch_offset = (float)(pitch - store->pitch);
		}

		/* do lever arm correction */
		if (platform_ptr != NULL) {
			/* calculate sensor position position */
			status = mb_platform_position(verbose, platform_ptr, pars->target_sensor, 0, navlon, navlat,
			                              sensordepth, heading, roll, pitch, &navlon, &navlat,
			                              &sensordepth, error);
			dlonm = (navlon - store->navlon) / mtodeglon;
			dlatm = (navlat - store->navlat) / mtodeglat;
			pulse->acrosstrack_offset = dlonm * headingx + dlatm * headingy;
			pulse->alongtrack_offset = dlonm * headingy - dlatm * headingx;
			pulse->sensordepth_offset = (float)(sensordepth - store->sensordepth);

			/* calculate sensor attitude */
			status = mb_platform_orientation_target(verbose, platform_ptr, pars->target_sensor, 0, heading, roll, pitch, &heading,
			                                        &roll, &pitch, error);
			pulse->heading_offset = (float)(heading - store->heading);
			pulse->roll_offset = (float)(roll - store->roll);
			pulse->pitch_offset = (float)(pitch - store->pitch);
		}
	}

	/* calculate the bathymetry using the newly inserted values */
	status = mbsys_3ddwissl_calculatebathymetry(verbose, mbio_ptr, store_ptr, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:         %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:        %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_extract(int verbose,     /* in: verbosity level set on command line 0..N */
                                 void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                 void *store_ptr, /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
                                 int *kind,       /* out: MBIO data type; see mb_status.h:0+/MBIO data type/ */
                                 int time_i[7],   /* out: MBIO time array; see mb_time.c:0+/mb_get_time/ */
                                 double *time_d,  /* out: MBIO time (seconds since 1,1,1970) */
                                 double *navlon,  /* out: transducer longitude -180.0..+180.0 */
                                 double *navlat,  /* out: transducer latitude -180.0..+180.0 */
                                 double *speed,   /* out: vessel speed (km/hr) */
                                 double *heading, /* out: vessel heading -180.0..+180.0 */
                                 int *nbath,      /* out: number of bathymetry samples (beams) */
                                 int *namp,       /* out: number of amplitude samples, usually namp = nbath */
                                 int *nss,        /* out: number of side scan pixels */
                                 char *beamflag,  /* out: array[nbath] of beam flags; see mb_status.h:/FLAG category/ */
                                 double *bath,    /* out: array[nbath] of depth values (m) positive down */
                                 double *amp,     /* out: array[namp] of amplitude values */
                                 double *bathacrosstrack, /* out: array[nbath] bathy across-track offsets from transducer (m) */
                                 double *bathalongtrack,  /* out: array[nbath] bathy along-track offsets from transducer (m) */
                                 double *ss,              /* out: array[nss] sidescan pixel values */
                                 double *ssacrosstrack,   /* out: array[nss] sidescan across-track offsets from transducer (m) */
                                 double *ssalongtrack,    /* out: array[nss] sidescan along-track offsets from transducer (m) */
                                 char *comment,           /* out: comment string (not supported by SWATHplus SXP) */
                                 int *error               /* out: see mb_status.h:/MB_ERROR/ */
                                 ) {
	char *function_name = "mbsys_3ddwissl_extract";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_pulse_struct *pulse;
	struct mbsys_3ddwissl_sounding_struct *sounding;
	int ipulse, isounding, ibath;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from store and copy into mb-system slots */
	if (*kind == MB_DATA_DATA) {
		/* get the timestamp */
		time_i[0] = store->year;
		time_i[1] = store->month;
		time_i[2] = store->day;
		time_i[3] = store->hour;
		time_i[4] = store->minutes;
		time_i[5] = store->seconds;
		time_i[6] = (int)(0.001 * store->nanoseconds);
		mb_get_time(verbose, time_i, time_d);

		/* get the navigation */
		*navlon = store->navlon;
		*navlat = store->navlat;
		*speed = store->speed;
		*heading = store->heading;

		/* get the number of soundings */
		*nbath = store->pulses_per_scan * store->soundings_per_pulse;
		*namp = *nbath;
		*nss = 0;

		/* we are poking into the mb_io_ptr to change the beamwidth here
		    350 microradians for the LIDAR laser */
		mb_io_ptr->beamwidth_xtrack = 0.02;
		mb_io_ptr->beamwidth_ltrack = 0.02;

		/* get the bathymetry */
		for (ipulse = 0; ipulse < store->pulses_per_scan; ipulse++) {
			pulse = &store->pulses[ipulse];
			for (isounding = 0; isounding < store->soundings_per_pulse; isounding++) {
				ibath = store->soundings_per_pulse * ipulse + isounding;
				sounding = &pulse->soundings[isounding];
				beamflag[ibath] = sounding->beamflag;
				bath[ibath] = sounding->depth + store->sensordepth;
				amp[ibath] = (double) sounding->amplitude;
				bathacrosstrack[ibath] = sounding->acrosstrack;
				bathalongtrack[ibath] = sounding->alongtrack;
//fprintf(stderr,"%s:%s():%d Extracting sounding ipulse:%d isounding:%d ibath:%d beamflag:%d\n",
//__FILE__, __FUNCTION__, __LINE__, ipulse, isounding, ibath, beamflag[ibath]);
			}
		}

		/* always successful */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}

	else if (*kind == MB_DATA_COMMENT)
		strncpy(comment, store->comment, MB_COMMENT_MAXLINE);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
} /* mbsys_3ddwissl_extract */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_insert(int verbose,     /* in: verbosity level set on command line 0..N */
                                void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                void *store_ptr, /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
                                int kind,        /* in: see mb_status.h:0+/MBIO data type/ */
                                int time_i[7],   /* in: see mb_time.c:0+/mb_get_time/ */
                                double time_d,   /* in: time in seconds since 1,1,1970) */
                                double navlon,   /* in: transducer longitude -180.0..+180.0 */
                                double navlat,   /* in: transducer latitude -180.0..+180.0 */
                                double speed,    /* in: vessel speed (km/hr) */
                                double heading,  /* in: vessel heading -180.0..+180.0 */
                                int nbath,       /* in: number of bathymetry samples/beams */
                                int namp,        /* in: number of amplitude samples, usually namp == nbath */
                                int nss,         /* in: number of sidescan pixels */
                                char *beamflag,  /* in: array[nbath] of beam flags; see mb_status.h:/FLAG category/ */
                                double *bath,    /* in: array[nbath] of depth values (m) positive down */
                                double *amp,     /* in: array[namp] of amplitude values */
                                double *bathacrosstrack, /* in: array[nbath] bathy across-track offsets from transducer (m) */
                                double *bathalongtrack,  /* in: array[nbath] bathy along-track offsets from transducer (m) */
                                double *ss,              /* in: array[nss] sidescan pixel values */
                                double *ssacrosstrack,   /* in: array[nss] sidescan across-track offsets from transducer (m) */
                                double *ssalongtrack,    /* in: array[nss] sidescan along-track offsets from transducer (m) */
                                char *comment,           /* in: comment string (not supported by SWATHplus SXP) */
                                int *error               /* out: see mb_status.h:/MB_ERROR/ */
                                ) {
	char *function_name = "mbsys_3ddwissl_insert";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_pulse_struct *pulse;
	struct mbsys_3ddwissl_sounding_struct *sounding;
	double dlon, dlat, dheading;
	int ipulse, isounding;
	int ibath;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(time_i != NULL);
	assert(0 <= nbath);
	assert(0 <= namp);
	assert(namp == nbath);
	assert(0 <= nss);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       kind:       %d\n", kind);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* set the timestamp */
		store->year = time_i[0];
		store->month = time_i[1];
		store->day = time_i[2];
		store->hour = time_i[3];
		store->minutes = time_i[4];
		store->seconds = time_i[5];
		store->nanoseconds = 1000 * ((unsigned int)time_i[6]);
		store->time_d = time_d;

		/* calculate change in navigation */
		dlon = navlon - store->navlon;
		dlat = navlat - store->navlat;
		dheading = heading - store->heading;

		/* set the navigation */
		store->navlon = navlon;
		store->navlat = navlat;
		store->speed = speed;
		store->heading = heading;
		
		/* check for allocation of space */
		if (store->soundings_per_pulse <= 0)
			store->soundings_per_pulse = 1;
		if (store->pulses_per_scan != nbath / store->soundings_per_pulse)
			store->pulses_per_scan = nbath / store->soundings_per_pulse;
		if (store->num_pulses_alloc < store->pulses_per_scan) {
			status = mb_reallocd(verbose, __FILE__, __LINE__,
								 (size_t) (store->pulses_per_scan * sizeof(struct mbsys_3ddwissl_pulse_struct)),
								 (void **) &(store->pulses),
								 error);
			if (status == MB_SUCCESS) {
				memset((void *) &(store->pulses[store->num_pulses_alloc]), 0,
					   (store->pulses_per_scan - store->num_pulses_alloc)
						* sizeof(struct mbsys_3ddwissl_pulse_struct));
				store->num_pulses_alloc = store->pulses_per_scan;
			}
		}

		/* set the bathymetry */
		for (ipulse = 0; ipulse < store->pulses_per_scan; ipulse++) {
			pulse = &store->pulses[ipulse];
			for (isounding = 0; isounding < store->soundings_per_pulse; isounding++) {
				ibath = store->soundings_per_pulse * ipulse + isounding;
				sounding = &pulse->soundings[isounding];
				sounding->beamflag = beamflag[ibath];
				sounding->depth = bath[ibath] - store->sensordepth;
				sounding->amplitude = amp[ibath];
				sounding->acrosstrack = bathacrosstrack[ibath];
				sounding->alongtrack = bathalongtrack[ibath];
//fprintf(stderr,"%s:%s():%d Inserting sounding ipulse:%d isounding:%d ibath:%d beamflag:%d\n",
//__FILE__, __FUNCTION__, __LINE__, ipulse, isounding, ibath, beamflag[ibath]);
			}
		}

		/* insert the sidescan pixel data */
	}

	/* deal with comments */
	else if (store->kind == MB_DATA_COMMENT) {
		store->time_d = time_d;
		store->comment_len = MIN(strlen(comment), MB_COMMENT_MAXLINE-1);
		strncpy(store->comment, comment, MB_COMMENT_MAXLINE);
	}

	/* deal with other records types  */
	else {
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 4)
		mbsys_3ddwissl_print_store(verbose, store, error);
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return status;
} /* mbsys_3ddwissl_insert */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_ttimes(int verbose,            /* in: verbosity level set on command line 0..N */
                                void *mbio_ptr,         /* in: see mb_io.h:/^struct mb_io_struct/ */
                                void *store_ptr,        /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
                                int *kind,              /* out: MBIO data type; see mb_status.h:0+/MBIO data type/ */
                                int *nbeams,            /* out: number of beams (samples) in this ping */
                                double *ttimes,         /* out: array[nbeams] travel time of beam (secs) */
                                double *angles,         /* out: array[nbeams] across-track angle of beam (deg) */
                                double *angles_forward, /* out: array[nbeams] along-track angle of beam (deg) */
                                double *angles_null,    /* out: array[nbeams] ?? */
                                double *heave,          /* out: array[nbeams] heave for each beam ?? */
                                double *alongtrack_offset, /* out: array[nbeams] ?? */
                                double *draft,             /* out: draft of transducer below waterline ?? (m) */
                                double *ssv,               /* out: sound velocity at head (m/s) */
                                int *error                 /* out: see mb_status.h:/MB_ERROR/ */
                                ) {
	char *function_name = "mbsys_3ddwissl_ttimes";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
	int i;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(ttimes != NULL);
	assert(angles != NULL);
	assert(angles_forward != NULL);
	assert(angles_null != NULL);
	assert(heave != NULL);
	assert(alongtrack_offset != NULL);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structre pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract travel time data */
	if (*kind == MB_DATA_DATA) {
		/* get the number of soundings */
		*nbeams = store->pulses_per_scan * store->soundings_per_pulse;

		/* get travel times, angles */
		for (i = 0; i < *nbeams; i++) {
			ttimes[i] = 0.0;
			angles[i] = 0.0;
			angles_forward[i] = 0.0;
			angles_null[i] = 0.0;
			heave[i] = 0.0;
			alongtrack_offset[i] = 0.0;
		}

		/* get ssv */
		*ssv = 0.0;
		*draft = 0.0;

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}
	/* deal with comment record type */
	else if (*kind == MB_DATA_COMMENT) {
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}
	/* deal with other record types */
	else {
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	/* print output debu statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
} /* mbsys_3ddwissl_ttimes */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_detects(int verbose,     /* in: verbosity level set on command line 0..N */
                                 void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                 void *store_ptr, /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
                                 int *kind,       /* out: MBIO data type; see mb_status.h:0+/MBIO data type/ */
                                 int *nbeams,     /* out: number of beams (samples) in this ping */
                                 int *detects, /* out: array[nbeams] detection flag;
                                                   see mb_status.h:/Bottom detect flags/ */
                                 int *error /* out: see mb_status.h:/MB_ERROR/ */
                                 ) {
	char *function_name = "mbsys_3ddwissl_detects";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
	int i;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       detects:    %p\n", detects);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get the number of soundings */
		*nbeams = store->pulses_per_scan * store->soundings_per_pulse;

		/* LIDAR detects */
		for (i = 0; i < *nbeams; i++)
			detects[i] = MB_DETECT_LIDAR;

		/* always successful */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if ((verbose >= 2) && (*error == MB_ERROR_NO_ERROR)) {
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: detects:%d\n", i, detects[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return status;
} /* mbsys_3ddwissl_detects */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_pulses(int verbose,     /* in: verbosity level set on command line 0..N */
                                void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                void *store_ptr, /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
                                int *kind,       /* out: MBIO data type; see mb_status.h:0+/MBIO data type/ */
                                int *nbeams,     /* out: number of beams (samples) in this ping */
                                int *pulses,     /* out: array[nbeams] pulse type; see mb_status.h:/Source pulse/ */
                                int *error       /* out: see mb_status.h:/MB_ERROR/ */
                                ) {
	char *function_name = "mbsys_3ddwissl_pulses";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
	int i;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       pulses:     %p\n", pulses);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get the number of soundings */
		*nbeams = store->pulses_per_scan * store->soundings_per_pulse;

		/* get pulse type */
		for (i = 0; i < *nbeams; i++) {
			pulses[i] = MB_PULSE_LIDAR;
		}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}

	/* deal with comments */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if ((verbose >= 2) && (*error == MB_ERROR_NO_ERROR)) {
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: pulses:%d\n", i, pulses[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
} /* mbsys_3ddwissl_pulses */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_gains(int verbose,           /* in: verbosity level set on command line 0..N */
                               void *mbio_ptr,        /* in: see mb_io.h:/^struct mb_io_struct/ */
                               void *store_ptr,       /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
                               int *kind,             /* in: MBIO data type; see mb_status.h:0+/MBIO data type/ */
                               double *transmit_gain, /* out: transmit gain (dB) */
                               double *pulse_length,  /* out: pulse width (usec) */
                               double *receive_gain,  /* out: receive gain (dB) */
                               int *error             /* out: see mb_status.h:/MB_ERROR/ */
                               ) {
	char *function_name = "mbsys_3ddwissl_gains";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* get transmit_gain (dB) */
		*transmit_gain = store->gain;

		/* get pulse_length */
		*pulse_length = 0.0;

		/* get receive_gain (dB) */
		*receive_gain = 0.0;
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record types */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if ((verbose >= 2) && (*error == MB_ERROR_NO_ERROR)) {
		fprintf(stderr, "dbg2       transmit_gain: %f\n", *transmit_gain);
		fprintf(stderr, "dbg2       pulse_length:  %f\n", *pulse_length);
		fprintf(stderr, "dbg2       receive_gain:  %f\n", *receive_gain);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return status;
} /* mbsys_3ddwissl_gains */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_extract_altitude(
    int verbose,              /* in: verbosity level set on command line 0..N */
    void *mbio_ptr,           /* in: see mb_io.h:/^struct mb_io_struct/ */
    void *store_ptr,          /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
    int *kind,                /* in: MBIO data type; see mb_status.h:0+/MBIO data type/ */
    double *transducer_depth, /* out: transducer depth below water line (m) */
    double *altitude,         /* out: transducer altitude above seafloor (m) */
    int *error                /* out: see mb_status.h:/MB_ERROR/ */
    ) {
	char *function_name = "mbsys_3ddwissl_extract_altitude";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_pulse_struct *pulse;
	struct mbsys_3ddwissl_sounding_struct *sounding;
	int i, ipulse, isounding;
	double rmin, r;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get sonar depth */
		*transducer_depth = store->sensordepth;

		/* loop over all soundings looking for most nadir */
		rmin = 9999999.9;
		for (ipulse = 0; ipulse < store->pulses_per_scan; ipulse++) {
			pulse = &store->pulses[ipulse];
			for (isounding=0; isounding < store->soundings_per_pulse; isounding++) {
				sounding = &pulse->soundings[isounding];
				if (mb_beam_ok(sounding->beamflag)) {
					r = sqrt(sounding->acrosstrack * sounding->acrosstrack + sounding->alongtrack * sounding->alongtrack);
					if (r < rmin) {
						rmin = r;
						*altitude = sounding->depth;
					}
				}
			}
		}

		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
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
} /* mbsys_3ddwissl_extract_altitude */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_extract_nnav(int verbose,     /* in: verbosity level set on command line 0..N */
                                      void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                      void *store_ptr, /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
                                      int nmax,        /* in: maximum size available to n; e.g., n < nmax */
                                      int *kind,       /* out: MBIO data type; see mb_status.h:0+/MBIO data type/ */
                                      int *n,          /* out: number of navigation values extracted */
                                      int *time_i,     /* out: array[n] time_i[7] values; see mb_time.c:0+/mb_get_time/ */
                                      double *time_d,  /* out: array[n] time_d values; seconds since 1,1,1970 */
                                      double *navlon,  /* out: array[n] longitude (degrees); -180.0..+180.0 */
                                      double *navlat,  /* out: array[n] latitude (degree); -90..+90 */
                                      double *speed,   /* out: array[n] speed (m/s) */
                                      double *heading, /* out: array[n] heading (degree): 0..360 */
                                      double *draft,   /* out: array[n] txer depth below datum (m) */
                                      double *roll,    /* out: array[n] roll (degrees) */
                                      double *pitch,   /* out: array[n] pitch (degrees) */
                                      double *heave,   /* out: array[n] heave (m) */
                                      int *error       /* out: see mb_status.h:/MB_ERROR/ */
                                      ) {
	char *function_name = "mbsys_3ddwissl_extract_nnav";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
	int inav;
	int i;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(nmax > 0);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       nmax:       %d\n", nmax);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA) {
		/* just one navigation value */
		*n = 1;

		/* get time */
		time_d[0] = store->time_d;
		mb_get_date(verbose, store->time_d, time_i);

		/* get navigation and heading */
		navlon[0] = store->navlon;
		navlat[0] = store->navlat;
		speed[0] = store->speed;
		heading[0] = store->heading;

		/* get draft */
		draft[0] = store->sensordepth;

		/* get roll pitch and heave. In SXP heave is included in height. */
		roll[0] = store->roll;
		pitch[0] = store->pitch;
		heave[0] = 0.0;

		/* done translating values */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}
	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		*n = 0;
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}
	/* deal with other record type */
	else {
		*n = 0;
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       n:          %d\n", *n);
		for (inav = 0; inav < *n; inav++) {
			for (i = 0; i < 7; i++)
				fprintf(stderr, "dbg2       %d time_i[%d]:     %d\n", inav, i, time_i[inav * 7 + i]);
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
} /* mbsys_3ddwissl_extract_nnav */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_extract_nav(int verbose, void *mbio_ptr, /* in: verbosity level set on command line 0..N */
                                     void *store_ptr,             /* in: see mb_io.h:/^struct mb_io_struct/ */
                                     int *kind,       /* out: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
                                     int time_i[7],   /* out: time_i[7] values; see mb_time.c */
                                     double *time_d,  /* out: time in seconds since 1,1,1970 */
                                     double *navlon,  /* out: longitude (degrees) -180..+180.0 */
                                     double *navlat,  /* out: latittude (degrees) -90..+90 */
                                     double *speed,   /* out: speed (km/s) */
                                     double *heading, /* out: heading (degrees) 0..360 */
                                     double *draft,   /* out: draft (m) */
                                     double *roll,    /* out: roll (degrees) */
                                     double *pitch,   /* out: pitch (degrees) */
                                     double *heave,   /* out: heave (degrees) */
                                     int *error       /* out: see mb_status.h:MB_ERROR */
                                     ) {
	char *function_name = "mbsys_3ddwissl_extract_nav";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* extract data from structure */
	*kind = store->kind;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA) {
		mb_get_date(verbose, store->time_d, time_i);
		*time_d = store->time_d;
		*navlon = store->navlon;
		*navlat = store->navlat;
		*speed = store->speed;
		*heading = store->heading;
		*draft = store->sensordepth;
		*roll = store->roll;
		*pitch = store->pitch;
		*heave = 0.0;
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}
	/* deal with other record type */
	else {
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if ((verbose >= 2) && (*error <= MB_ERROR_NO_ERROR) && (*kind == MB_DATA_DATA)) {
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
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
} /* mbsys_3ddwissl_extract_nav */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl_insert_nav(int verbose, void *mbio_ptr, /* in: verbosity level set on command line */
                                    void *store_ptr,             /* in: see mb_io.h:mb_io_struct */
                                    int time_i[7],               /* in: time_i struct; see mb_time.c */
                                    double time_d,               /* in: time in seconds since 1,1,1970 */
                                    double navlon,               /* in: longitude in degrees -180..+180 */
                                    double navlat,               /* in: latitude in degrees -90..+90 */
                                    double speed,                /* in: speed (m/s) */
                                    double heading,              /* in: heading (degrees) */
                                    double draft,                /* in: draft (m) */
                                    double roll,                 /* in: roll (degrees) */
                                    double pitch,                /* in: pitch (degreees) */
                                    double heave,                /* in: heave (m) */
                                    int *error                   /* out: see mb_status.h:MB_ERROR */
                                    ) {
	char *function_name = "mbsys_3ddwissl_insert_nav";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_pulse_struct *pulse;
	double dlon, dlat, dheading, dsensordepth, droll, dpitch;
	int ipulse, isounding;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(time_i != NULL);

	/* print input debug statements */
	if (verbose >= 2) {
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
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* insert data in swathplus data structure */
	if (store->kind == MB_DATA_DATA) {
		dlon = navlon - store->navlon;
		dlat = navlat - store->navlat;
		dheading = heading - store->heading;
		dsensordepth = draft - heave - store->sensordepth;
		droll = roll - store->roll;
		dpitch = pitch - store->pitch;

		store->time_d = time_d;
		store->navlon = navlon;
		store->navlat = navlat;
		store->speed = speed;
		store->heading = heading;
		store->sensordepth = draft - heave;
		store->roll = roll;
		store->pitch = pitch;

		/* done translating values */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return status;
} /* mbsys_3ddwissl_insert_nav */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl_extract_svp(int verbose,      /* in: verbosity level set on command line 0..N */
                                     void *mbio_ptr,   /* in: see mb_io.h:mb_io_struct */
                                     void *store_ptr,  /* in: see mbsys_3ddwissl.h:mbsys_3ddwissl_struct */
                                     int *kind,        /* out: see mb_status.h:MBIO data type */
                                     int *nsvp,        /* out: number of svp measurements */
                                     double *depth,    /* out: array[nsvp] depths (m) */
                                     double *velocity, /* out: array[nsvp] velocity (m) */
                                     int *error        /* out: see: mb_status.h:MB_ERROR */
                                     ) {
	char *function_name = "mbsys_3ddwissl_extract_svp";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
	int i;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       nsvp:              %d\n", *nsvp);
		for (i = 0; i < *nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	/* return status */
	return status;
} /* mbsys_3ddwissl_extract_svp */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl_insert_svp(int verbose,      /* in: verbosity level set on command line 0..N */
                                    void *mbio_ptr,   /* in: mbio.h:mb_io_struct */
                                    void *store_ptr,  /* in: mbsys_3ddwissl_struct */
                                    int nsvp,         /* in: number of svp records to insert */
                                    double *depth,    /* in: array[nsvp] depth records (m) */
                                    double *velocity, /* in: array[nsvp] sound velocity records (m/s) */
                                    int *error        /* out: see mb_status.h:MB_ERROR */
                                    ) {
	char *function_name = "mbsys_3ddwissl_insert_svp";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
	int i;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(nsvp > 0);

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       nsvp:       %d\n", nsvp);
		for (i = 0; i < nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_COMMENT) {
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* handle other types */
	else {
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return status;
} /* mbsys_3ddwissl_insert_svp */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl_copy(int verbose,     /* in: verbosity level set on command line */
                              void *mbio_ptr,  /* in: see mb_io.h:mb_io_struct */
                              void *store_ptr, /* in: see mbsys_3ddwissl.h:mbsys_3ddwissl_struct */
                              void *copy_ptr,  /* out: see mbsys_3ddwissl.h:mbsys_3ddwissl_struct */
                              int *error       /* out: see mb_status.h:MB_ERROR */
                              ) {
	char *function_name = "mbsys_3ddwissl_copy";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_struct *copy;
	int num_pulses_alloc;
	struct mbsys_3ddwissl_pulse_struct *pulse_ptr;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(copy_ptr != NULL);
	assert(store_ptr != copy_ptr);

	/* print input debug statements */
	if (verbose >= 2) {
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
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;
	copy = (struct mbsys_3ddwissl_struct *)copy_ptr;

	/* copy structure */
	num_pulses_alloc = copy->num_pulses_alloc;
	pulse_ptr = copy->pulses;
	memcpy(copy_ptr, store_ptr, sizeof(struct mbsys_3ddwissl_struct));
	copy->num_pulses_alloc = num_pulses_alloc;
	copy->pulses = pulse_ptr;

	/* allocate memory for data structure */
	if (copy->pulses_per_scan > copy->num_pulses_alloc || copy->pulses == NULL) {
		status = mb_reallocd(verbose, __FILE__, __LINE__,
							 copy->pulses_per_scan * sizeof(struct mbsys_3ddwissl_pulse_struct),
							(void **)&copy->pulses,
							error);
		if (status == MB_SUCCESS)
			copy->num_pulses_alloc = copy->pulses_per_scan;
	}

	/* copy pulses */
	memcpy((void *)copy->pulses, (void *)store->pulses,
		   copy->pulses_per_scan * sizeof(struct mbsys_3ddwissl_pulse_struct));

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
} /* mbsys_3ddwissl_copy */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl_print_store(int verbose,     /* in: verbosity level set on command line 0..N */
                                     void *store_ptr, /* in: see mbsys_3ddwissl.h:mbsys_3ddwissl_struct */
                                     int *error       /* out: see mb_status.h:MB_ERROR */
                                     ) {
	char *function_name = "mbsys_3ddwissl_print_store";
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_pulse_struct *pulse;
	struct mbsys_3ddwissl_sounding_struct *sounding;
	int status;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int npulses;
	int i, ipulse, isounding;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2         store:    %p\n", store_ptr);
	}

	/* check for non-null data */
	assert(store_ptr != NULL);

	/* always successful */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* get data structure pointers */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* print 3DDWISSL store structure contents */
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, function_name);
	}
	fprintf(stderr, "%s struct mbsys_3ddwissl contents:\n", first);
	fprintf(stderr, "%s     kind:                          %d\n", first, store->kind);
	fprintf(stderr, "%s     magic_number:                  %u\n", first, store->magic_number);
	fprintf(stderr, "%s     file_version:                  %u\n", first, store->file_version);
	fprintf(stderr, "%s     sub_version:                   %u\n", first, store->sub_version);
	fprintf(stderr, "%s     cross_track_angle_start:       %f\n", first, store->cross_track_angle_start);
	fprintf(stderr, "%s     cross_track_angle_end:         %f\n", first, store->cross_track_angle_end);
	fprintf(stderr, "%s     pulses_per_scan:               %u\n", first, store->pulses_per_scan);
	fprintf(stderr, "%s     soundings_per_pulse:           %u\n", first, store->soundings_per_pulse);
	fprintf(stderr, "%s     heada_scans_per_file:          %u\n", first, store->heada_scans_per_file);
	fprintf(stderr, "%s     headb_scans_per_file:          %u\n", first, store->headb_scans_per_file);
	if (store->kind == MB_DATA_PARAMETER) {
		fprintf(stderr, "%s     calibration A: cfg_path:                      %s\n", first, store->calibration_a.cfg_path);
		fprintf(stderr, "%s     calibration A: laser_head_no:                 %d\n", first, store->calibration_a.laser_head_no);
		fprintf(stderr, "%s     calibration A: process_for_air:               %d\n", first, store->calibration_a.process_for_air);
		fprintf(stderr, "%s     calibration A: temperature_compensation:      %d\n", first, store->calibration_a.temperature_compensation);
		fprintf(stderr, "%s     calibration A: emergency_shutdown:            %d\n", first, store->calibration_a.emergency_shutdown);
		fprintf(stderr, "%s     calibration A: ocb_temperature_limit_c:       %f\n", first, store->calibration_a.ocb_temperature_limit_c);
		fprintf(stderr, "%s     calibration A: ocb_temperature_limit_c:       %f\n", first, store->calibration_a.ocb_temperature_limit_c);
		fprintf(stderr, "%s     calibration A: ocb_humidity_limit:            %f\n", first, store->calibration_a.ocb_humidity_limit);
		fprintf(stderr, "%s     calibration A: pb_temperature_limit_1_c:      %f\n", first, store->calibration_a.pb_temperature_limit_1_c);
		fprintf(stderr, "%s     calibration A: pb_temperature_limit_2_c:      %f\n", first, store->calibration_a.pb_temperature_limit_2_c);
		fprintf(stderr, "%s     calibration A: pb_humidity_limit:             %f\n", first, store->calibration_a.pb_humidity_limit);
		fprintf(stderr, "%s     calibration A: dig_temperature_limit_c:       %f\n", first, store->calibration_a.dig_temperature_limit_c);
		fprintf(stderr, "%s     calibration A: l_d_cable_set:                 %s\n", first, store->calibration_a.l_d_cable_set);
		fprintf(stderr, "%s     calibration A: ocb_comm_port:                 %s\n", first, store->calibration_a.ocb_comm_port);
		fprintf(stderr, "%s     calibration A: ocb_comm_cfg:                  %s\n", first, store->calibration_a.ocb_comm_cfg);
		fprintf(stderr, "%s     calibration A: az_ao_deg_to_volt:             %f\n", first, store->calibration_a.az_ao_deg_to_volt);
		fprintf(stderr, "%s     calibration A: az_ai_neg_v_to_deg:            %f\n", first, store->calibration_a.az_ai_neg_v_to_deg);
		fprintf(stderr, "%s     calibration A: az_ai_pos_v_to_deg:            %f\n", first, store->calibration_a.az_ai_pos_v_to_deg);
		fprintf(stderr, "%s     calibration A: t1_air:                        %f\n", first, store->calibration_a.t1_air);
		fprintf(stderr, "%s     calibration A: ff_air:                        %f\n", first, store->calibration_a.ff_air);
		fprintf(stderr, "%s     calibration A: t1_water_g4000:                %f\n", first, store->calibration_a.t1_water_g4000);
		fprintf(stderr, "%s     calibration A: ff_water_g4000:                %f\n", first, store->calibration_a.ff_water_g4000);
		fprintf(stderr, "%s     calibration A: t1_water_g3000:                %f\n", first, store->calibration_a.t1_water_g3000);
		fprintf(stderr, "%s     calibration A: ff_water_g3000:                %f\n", first, store->calibration_a.ff_water_g3000);
		fprintf(stderr, "%s     calibration A: t1_water_g2000:                %f\n", first, store->calibration_a.t1_water_g2000);
		fprintf(stderr, "%s     calibration A: ff_water_g2000:                %f\n", first, store->calibration_a.ff_water_g2000);
		fprintf(stderr, "%s     calibration A: t1_water_g1000:                %f\n", first, store->calibration_a.t1_water_g1000);
		fprintf(stderr, "%s     calibration A: ff_water_g1000:                %f\n", first, store->calibration_a.ff_water_g1000);
		fprintf(stderr, "%s     calibration A: t1_water_g400:                 %f\n", first, store->calibration_a.t1_water_g400);
		fprintf(stderr, "%s     calibration A: ff_water_g400:                 %f\n", first, store->calibration_a.ff_water_g400);
		fprintf(stderr, "%s     calibration A: t1_water_g300:                 %f\n", first, store->calibration_a.t1_water_g300);
		fprintf(stderr, "%s     calibration A: ff_water_g300:                 %f\n", first, store->calibration_a.ff_water_g300);
		fprintf(stderr, "%s     calibration A: t1_water_secondary_g4000:      %f\n", first, store->calibration_a.t1_water_secondary_g4000);
		fprintf(stderr, "%s     calibration A: ff_water_secondary_g4000:      %f\n", first, store->calibration_a.ff_water_secondary_g4000);
		fprintf(stderr, "%s     calibration A: t1_water_secondary_g3000:      %f\n", first, store->calibration_a.t1_water_secondary_g3000);
		fprintf(stderr, "%s     calibration A: ff_water_secondary_g3000:      %f\n", first, store->calibration_a.ff_water_secondary_g3000);
		fprintf(stderr, "%s     calibration A: t1_water_secondary_g2000:      %f\n", first, store->calibration_a.t1_water_secondary_g2000);
		fprintf(stderr, "%s     calibration A: ff_water_secondary_g2000:      %f\n", first, store->calibration_a.ff_water_secondary_g2000);
		fprintf(stderr, "%s     calibration A: t1_water_secondary_g1000:      %f\n", first, store->calibration_a.t1_water_secondary_g1000);
		fprintf(stderr, "%s     calibration A: ff_water_secondary_g1000:      %f\n", first, store->calibration_a.ff_water_secondary_g1000);
		fprintf(stderr, "%s     calibration A: t1_water_secondary_g400:       %f\n", first, store->calibration_a.t1_water_secondary_g400);
		fprintf(stderr, "%s     calibration A: ff_water_secondary_g400:       %f\n", first, store->calibration_a.ff_water_secondary_g400);
		fprintf(stderr, "%s     calibration A: t1_water_secondary_g300:       %f\n", first, store->calibration_a.t1_water_secondary_g300);
		fprintf(stderr, "%s     calibration A: ff_water_secondary_g300:       %f\n", first, store->calibration_a.ff_water_secondary_g300);
		fprintf(stderr, "%s     calibration A: temp_comp_poly2:               %f\n", first, store->calibration_a.temp_comp_poly2);
		fprintf(stderr, "%s     calibration A: temp_comp_poly1:               %f\n", first, store->calibration_a.temp_comp_poly1);
		fprintf(stderr, "%s     calibration A: temp_comp_poly:                %f\n", first, store->calibration_a.temp_comp_poly);
		fprintf(stderr, "%s     calibration A: laser_start_time_sec:          %f\n", first, store->calibration_a.laser_start_time_sec);
		fprintf(stderr, "%s     calibration A: scanner_shift_cts:             %f\n", first, store->calibration_a.scanner_shift_cts);
		fprintf(stderr, "%s     calibration A: factory_scanner_lrg_deg:       %f\n", first, store->calibration_a.factory_scanner_lrg_deg);
		fprintf(stderr, "%s     calibration A: factory_scanner_med_deg:       %f\n", first, store->calibration_a.factory_scanner_med_deg);
		fprintf(stderr, "%s     calibration A: factory_scanner_sml_deg:       %f\n", first, store->calibration_a.factory_scanner_sml_deg);
		fprintf(stderr, "%s     calibration A: el_angle_fixed_deg:            %f\n", first, store->calibration_a.el_angle_fixed_deg);
		fprintf(stderr, "%s     calibration B: cfg_path:                      %s\n", first, store->calibration_b.cfg_path);
		fprintf(stderr, "%s     calibration B: laser_head_no:                 %d\n", first, store->calibration_b.laser_head_no);
		fprintf(stderr, "%s     calibration B: process_for_air:               %d\n", first, store->calibration_b.process_for_air);
		fprintf(stderr, "%s     calibration B: temperature_compensation:      %d\n", first, store->calibration_b.temperature_compensation);
		fprintf(stderr, "%s     calibration B: emergency_shutdown:            %d\n", first, store->calibration_b.emergency_shutdown);
		fprintf(stderr, "%s     calibration B: ocb_temperature_limit_c:       %f\n", first, store->calibration_b.ocb_temperature_limit_c);
		fprintf(stderr, "%s     calibration B: ocb_temperature_limit_c:       %f\n", first, store->calibration_b.ocb_temperature_limit_c);
		fprintf(stderr, "%s     calibration B: ocb_humidity_limit:            %f\n", first, store->calibration_b.ocb_humidity_limit);
		fprintf(stderr, "%s     calibration B: pb_temperature_limit_1_c:      %f\n", first, store->calibration_b.pb_temperature_limit_1_c);
		fprintf(stderr, "%s     calibration B: pb_temperature_limit_2_c:      %f\n", first, store->calibration_b.pb_temperature_limit_2_c);
		fprintf(stderr, "%s     calibration B: pb_humidity_limit:             %f\n", first, store->calibration_b.pb_humidity_limit);
		fprintf(stderr, "%s     calibration B: dig_temperature_limit_c:       %f\n", first, store->calibration_b.dig_temperature_limit_c);
		fprintf(stderr, "%s     calibration B: l_d_cable_set:                 %s\n", first, store->calibration_b.l_d_cable_set);
		fprintf(stderr, "%s     calibration B: ocb_comm_port:                 %s\n", first, store->calibration_b.ocb_comm_port);
		fprintf(stderr, "%s     calibration B: ocb_comm_cfg:                  %s\n", first, store->calibration_b.ocb_comm_cfg);
		fprintf(stderr, "%s     calibration B: az_ao_deg_to_volt:             %f\n", first, store->calibration_b.az_ao_deg_to_volt);
		fprintf(stderr, "%s     calibration B: az_ai_neg_v_to_deg:            %f\n", first, store->calibration_b.az_ai_neg_v_to_deg);
		fprintf(stderr, "%s     calibration B: az_ai_pos_v_to_deg:            %f\n", first, store->calibration_b.az_ai_pos_v_to_deg);
		fprintf(stderr, "%s     calibration B: t1_air:                        %f\n", first, store->calibration_b.t1_air);
		fprintf(stderr, "%s     calibration B: ff_air:                        %f\n", first, store->calibration_b.ff_air);
		fprintf(stderr, "%s     calibration B: t1_water_g4000:                %f\n", first, store->calibration_b.t1_water_g4000);
		fprintf(stderr, "%s     calibration B: ff_water_g4000:                %f\n", first, store->calibration_b.ff_water_g4000);
		fprintf(stderr, "%s     calibration B: t1_water_g3000:                %f\n", first, store->calibration_b.t1_water_g3000);
		fprintf(stderr, "%s     calibration B: ff_water_g3000:                %f\n", first, store->calibration_b.ff_water_g3000);
		fprintf(stderr, "%s     calibration B: t1_water_g2000:                %f\n", first, store->calibration_b.t1_water_g2000);
		fprintf(stderr, "%s     calibration B: ff_water_g2000:                %f\n", first, store->calibration_b.ff_water_g2000);
		fprintf(stderr, "%s     calibration B: t1_water_g1000:                %f\n", first, store->calibration_b.t1_water_g1000);
		fprintf(stderr, "%s     calibration B: ff_water_g1000:                %f\n", first, store->calibration_b.ff_water_g1000);
		fprintf(stderr, "%s     calibration B: t1_water_g400:                 %f\n", first, store->calibration_b.t1_water_g400);
		fprintf(stderr, "%s     calibration B: ff_water_g400:                 %f\n", first, store->calibration_b.ff_water_g400);
		fprintf(stderr, "%s     calibration B: t1_water_g300:                 %f\n", first, store->calibration_b.t1_water_g300);
		fprintf(stderr, "%s     calibration B: ff_water_g300:                 %f\n", first, store->calibration_b.ff_water_g300);
		fprintf(stderr, "%s     calibration B: t1_water_secondary_g4000:      %f\n", first, store->calibration_b.t1_water_secondary_g4000);
		fprintf(stderr, "%s     calibration B: ff_water_secondary_g4000:      %f\n", first, store->calibration_b.ff_water_secondary_g4000);
		fprintf(stderr, "%s     calibration B: t1_water_secondary_g3000:      %f\n", first, store->calibration_b.t1_water_secondary_g3000);
		fprintf(stderr, "%s     calibration B: ff_water_secondary_g3000:      %f\n", first, store->calibration_b.ff_water_secondary_g3000);
		fprintf(stderr, "%s     calibration B: t1_water_secondary_g2000:      %f\n", first, store->calibration_b.t1_water_secondary_g2000);
		fprintf(stderr, "%s     calibration B: ff_water_secondary_g2000:      %f\n", first, store->calibration_b.ff_water_secondary_g2000);
		fprintf(stderr, "%s     calibration B: t1_water_secondary_g1000:      %f\n", first, store->calibration_b.t1_water_secondary_g1000);
		fprintf(stderr, "%s     calibration B: ff_water_secondary_g1000:      %f\n", first, store->calibration_b.ff_water_secondary_g1000);
		fprintf(stderr, "%s     calibration B: t1_water_secondary_g400:       %f\n", first, store->calibration_b.t1_water_secondary_g400);
		fprintf(stderr, "%s     calibration B: ff_water_secondary_g400:       %f\n", first, store->calibration_b.ff_water_secondary_g400);
		fprintf(stderr, "%s     calibration B: t1_water_secondary_g300:       %f\n", first, store->calibration_b.t1_water_secondary_g300);
		fprintf(stderr, "%s     calibration B: ff_water_secondary_g300:       %f\n", first, store->calibration_b.ff_water_secondary_g300);
		fprintf(stderr, "%s     calibration B: temp_comp_poly2:               %f\n", first, store->calibration_b.temp_comp_poly2);
		fprintf(stderr, "%s     calibration B: temp_comp_poly1:               %f\n", first, store->calibration_b.temp_comp_poly1);
		fprintf(stderr, "%s     calibration B: temp_comp_poly:                %f\n", first, store->calibration_b.temp_comp_poly);
		fprintf(stderr, "%s     calibration B: laser_start_time_sec:          %f\n", first, store->calibration_b.laser_start_time_sec);
		fprintf(stderr, "%s     calibration B: scanner_shift_cts:             %f\n", first, store->calibration_b.scanner_shift_cts);
		fprintf(stderr, "%s     calibration B: factory_scanner_lrg_deg:       %f\n", first, store->calibration_b.factory_scanner_lrg_deg);
		fprintf(stderr, "%s     calibration B: factory_scanner_med_deg:       %f\n", first, store->calibration_b.factory_scanner_med_deg);
		fprintf(stderr, "%s     calibration B: factory_scanner_sml_deg:       %f\n", first, store->calibration_b.factory_scanner_sml_deg);
		fprintf(stderr, "%s     calibration B: el_angle_fixed_deg:            %f\n", first, store->calibration_b.el_angle_fixed_deg);
	}
	if (store->kind == MB_DATA_DATA) {
	    fprintf(stderr, "%s     record_id:                     %x\n", first, store->record_id);
		fprintf(stderr, "%s     year:                          %u\n", first, store->year);
		fprintf(stderr, "%s     month:                         %u\n", first, store->month);
		fprintf(stderr, "%s     day:                           %u\n", first, store->day);
		fprintf(stderr, "%s     days_since_jan_1:              %u\n", first, store->jday);
		fprintf(stderr, "%s     hour:                          %u\n", first, store->hour);
		fprintf(stderr, "%s     minutes:                       %u\n", first, store->minutes);
		fprintf(stderr, "%s     seconds:                       %u\n", first, store->seconds);
		fprintf(stderr, "%s     nanoseconds:                   %u\n", first, store->nanoseconds);
		
		fprintf(stderr, "%s     gain:                          %u\n", first, store->gain);
		fprintf(stderr, "%s     digitizer_temperature:         %f\n", first, store->digitizer_temperature);
		fprintf(stderr, "%s     ctd_temperature:               %f\n", first, store->ctd_temperature);
		fprintf(stderr, "%s     ctd_salinity:                  %f\n", first, store->ctd_salinity);
		fprintf(stderr, "%s     ctd_pressure:                  %f\n", first, store->ctd_pressure);
		fprintf(stderr, "%s     index:                         %f\n", first, store->index);
		fprintf(stderr, "%s     range_start:                   %f\n", first, store->range_start);
		fprintf(stderr, "%s     range_end:                     %f\n", first, store->range_end);
		fprintf(stderr, "%s     pulse_count:                   %d\n", first, store->pulse_count);
		fprintf(stderr, "%s     time_d:                        %f\n", first, store->time_d);
		fprintf(stderr, "%s     navlon:                        %f\n", first, store->navlon);
		fprintf(stderr, "%s     navlat:                        %f\n", first, store->navlat);
		fprintf(stderr, "%s     sonardepth:                    %f\n", first, store->sensordepth);
		fprintf(stderr, "%s     speed:                         %f\n", first, store->speed);
		fprintf(stderr, "%s     heading:                       %f\n", first, store->heading);
		fprintf(stderr, "%s     roll:                          %f\n", first, store->roll);
		fprintf(stderr, "%s     pitch:                         %f\n", first, store->pitch);
		fprintf(stderr, "%s     scan_count:                    %u\n", first, store->scan_count);
		fprintf(stderr, "%s     size_pulse_record_raw:         %u\n", first, store->size_pulse_record_raw);
		fprintf(stderr, "%s     size_pulse_record_processed:   %u\n", first, store->size_pulse_record_processed);
		fprintf(stderr, "%s     bathymetry_calculated:         %u\n", first, store->bathymetry_calculated);
		
		fprintf(stderr, "%s     num_pulses_alloc:              %d\n", first, store->num_pulses_alloc);
		for (ipulse = 0; ipulse < store->pulses_per_scan; ipulse++) {
			pulse = &(store->pulses[ipulse]);
			fprintf(stderr, "%s------------------------------------------\n", first);
			fprintf(stderr, "%s     ipulse:                        %d\n", first, ipulse);
			fprintf(stderr, "%s     angle_az:                      %f\n", first, pulse->angle_az);
			fprintf(stderr, "%s     angle_el:                      %f\n", first, pulse->angle_el);
			fprintf(stderr, "%s     offset_az:                     %f\n", first, pulse->offset_az);
			fprintf(stderr, "%s     offset_el:                     %f\n", first, pulse->offset_el);
			fprintf(stderr, "%s     time_offset:                   %f\n", first, pulse->time_offset);
			fprintf(stderr, "%s     time_d:                        %f\n", first, pulse->time_d);
			fprintf(stderr, "%s     acrosstrack_offset:            %f\n", first, pulse->acrosstrack_offset);
			fprintf(stderr, "%s     alongtrack_offset:             %f\n", first, pulse->alongtrack_offset);
			fprintf(stderr, "%s     sensordepth_offset:            %f\n", first, pulse->sensordepth_offset);
			fprintf(stderr, "%s     heading_offset:                %f\n", first, pulse->heading_offset);
			fprintf(stderr, "%s     roll_offset:                   %f\n", first, pulse->roll_offset);
			fprintf(stderr, "%s     pitch_offset:                  %f\n", first, pulse->pitch_offset);
			for (isounding = 0; isounding < store->soundings_per_pulse; isounding++) {
				sounding = &(pulse->soundings[isounding]);
				fprintf(stderr, "%s     --------\n", first);
				fprintf(stderr, "%s     isounding:                     %d\n", first, isounding);			
				fprintf(stderr, "%s     range:                         %f\n", first, sounding->range);
			    fprintf(stderr, "%s     amplitude:                     %d\n", first, sounding->amplitude);
				fprintf(stderr, "%s     beamflag:                      %u\n", first, sounding->beamflag);
				fprintf(stderr, "%s     acrosstrack:                   %f\n", first, sounding->acrosstrack);
				fprintf(stderr, "%s     alongtrack:                    %f\n", first, sounding->alongtrack);
				fprintf(stderr, "%s     depth:                         %f\n", first, sounding->depth);
			}
			fprintf(stderr, "%s     --------\n", first);
		}
		fprintf(stderr, "%s------------------------------------------\n", first);
	}
	else if (store->kind == MB_DATA_COMMENT) {
	    fprintf(stderr, "%s     record_id:                     %x\n", first, store->record_id);
		fprintf(stderr, "%s     comment_len:                   %d\n", first, store->comment_len);
		fprintf(stderr, "%s     comment:                       %s\n", first, store->comment);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return status;
} /* mbsys_3ddwissl_print_store */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_calculatebathymetry(int verbose,     /* in: verbosity level set on command line 0..N */
                                             void *mbio_ptr,  /* in: see mb_io.h:mb_io_struct */
                                             void *store_ptr, /* in: see mbsys_3ddwissl.h:mbsys_3ddwissl_struct */
                                             int *error       /* out: see mb_status.h:MB_ERROR */
                                             ) {
	char *function_name = "mbsys_3ddwissl_calculatebathymetry";
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_pulse_struct *pulse;
	struct mbsys_3ddwissl_sounding_struct *sounding;
	int status;
	int time_i[7];
	double alpha, beta, theta, phi;
	double angle_az_sign, angle_el_sign;
	double mtodeglon, mtodeglat;
	double xx;
	int ipulse, isounding;
    double head_offset_x_m;
    double head_offset_y_m;
    double head_offset_z_m;
    double head_offset_heading_deg;
    double head_offset_roll_deg;
    double head_offset_pitch_deg;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2         store:    %p\n", store_ptr);
	}

	/* check for non-null data */
	assert(store_ptr != NULL);

	/* always successful */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* get data structure pointers */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* recalculate bathymetry from LIDAR data */
	if (store->kind == MB_DATA_DATA) {
		/* get time_d timestamp */
		time_i[0] = store->year;
		time_i[1] = store->month;
		time_i[2] = store->day;
		time_i[3] = store->hour;
		time_i[4] = store->minutes;
		time_i[5] = store->seconds;
		time_i[6] = (int)(0.001 * store->nanoseconds);
		mb_get_time(verbose, time_i, &store->time_d);

		/* get scaling */
		mb_coor_scale(verbose, store->navlat, &mtodeglon, &mtodeglat);
		
		/* set offsets according to which optical head these soundings come from */
		if (store->record_id == MBSYS_3DDWISSL_RECORD_RAWHEADA || store->record_id == MBSYS_3DDWISSL_RECORD_PROHEADA) {
			/* optical head A */
			angle_az_sign = -1.0;
			angle_el_sign = 1.0;
			head_offset_x_m = store->heada_offset_x_m;
			head_offset_y_m = store->heada_offset_y_m;
			head_offset_z_m = store->heada_offset_z_m;
			head_offset_heading_deg = store->heada_offset_heading_deg;
			head_offset_roll_deg = store->heada_offset_roll_deg;
			head_offset_pitch_deg = store->heada_offset_pitch_deg;
		}
		else {
			/* optical head B */
			angle_az_sign = 1.0;
			angle_el_sign = 1.0;
			head_offset_x_m = store->headb_offset_x_m;
			head_offset_y_m = store->headb_offset_y_m;
			head_offset_z_m = store->headb_offset_z_m;
			head_offset_heading_deg = store->headb_offset_heading_deg;
			head_offset_roll_deg = store->headb_offset_roll_deg;
			head_offset_pitch_deg = store->headb_offset_pitch_deg;
		}

		/* loop over all pulses and soundings */
		for (ipulse = 0; ipulse < store->pulses_per_scan; ipulse++) {
			pulse = (struct mbsys_3ddwissl_pulse_struct *)&store->pulses[ipulse];
			for (isounding=0; isounding<store->soundings_per_pulse; isounding++) {
				sounding = &pulse->soundings[isounding];
				/* valid pulses have nonzero ranges */
				if (sounding->range > 0.001) {
	
					/* set beamflag */
					sounding->beamflag = MB_FLAG_NONE;
	
					/* apply pitch and roll */
					alpha = angle_el_sign * pulse->angle_el
										+ store->pitch
										+ head_offset_pitch_deg
										+ pulse->pitch_offset;
					beta = 90.0 - (angle_az_sign * pulse->angle_az)
										+ store->roll
										+ head_offset_roll_deg
										+ pulse->roll_offset;
	
					/* translate to takeoff coordinates */
					mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);
					phi += head_offset_heading_deg + pulse->heading_offset;
	
					/* get lateral and vertical components of range */
					xx = sounding->range * sin(DTR * theta);
					sounding->depth = sounding->range * cos(DTR * theta)
										+ head_offset_z_m
										+ pulse->sensordepth_offset;
					sounding->acrosstrack = xx * cos(DTR * phi)
										+ head_offset_x_m
										+ pulse->acrosstrack_offset;
					sounding->alongtrack = xx * sin(DTR * phi)
										+ head_offset_y_m
										+ pulse->alongtrack_offset;
				}
				else {
					/* null everything */
					sounding->beamflag = MB_FLAG_NULL;
					sounding->depth = 0.0;
					sounding->acrosstrack = 0.0;
					sounding->alongtrack = 0.0;
				}
			}
		}

		/* set flag indicating that bathymetry has been calculated */
		store->bathymetry_calculated = MB_YES;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return status;
} /* mbsys_3ddwissl_print_store */
/*--------------------------------------------------------------------*/
