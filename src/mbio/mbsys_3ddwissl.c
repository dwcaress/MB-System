/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_3ddwissl.c	3.00	12/26/2017
 *
 *    Copyright (c) 2017-2019 by
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


#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mbsys_3ddwissl.h"

#define MBF_3DDEPTHP_DEBUG 1

/*-------------------------------------------------------------------- */
int mbsys_3ddwissl_alloc(int verbose,      /* in: verbosity level set on command line 0..N */
                               void *mbio_ptr,   /* in: see mb_io.h:/^struct mb_io_struct/ */
                               void **store_ptr, /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
                               int *error        /* out: see mb_status.h:/MB_ERROR/ */
                               ) {
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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
	memset((void *)&store->calibration_v1s1_a, 0, sizeof(struct mbsys_3ddwissl_calibration_v1s1_struct));
	memset((void *)&store->calibration_v1s2_a, 0, sizeof(struct mbsys_3ddwissl_calibration_v1s2_struct));

	memset((void *)&store->calibration_v1s1_b, 0, sizeof(struct mbsys_3ddwissl_calibration_v1s1_struct));
	memset((void *)&store->calibration_v1s2_b, 0, sizeof(struct mbsys_3ddwissl_calibration_v1s2_struct));

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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       store_ptr:  %p\n", *store_ptr);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
} /* mbsys_3ddwissl_alloc */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl_deall(int verbose,      /* in: verbosity level set on command line 0..N */
                               void *mbio_ptr,   /* in: see mb_io.h:/^struct mb_io_struct/ */
                               void **store_ptr, /* in: see mbsys_3ddwissl.h:/^struct mbsys_3ddwissl_struct/ */
                               int *error        /* out: see mb_status.h:/error values/ */
                               ) {
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

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
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		fprintf(stderr, "dbg2        namp:      %d\n", *namp);
		fprintf(stderr, "dbg2        nss:       %d\n", *nss);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
} /* mbsys_3ddwissl_dimensions */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_pingnumber(int verbose,     /* in: verbosity level set on command line 0..N */
                                    void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                    unsigned int *pingnumber, /* out: ping number */
                                    int *error       /* out: see mb_status.h:/MB_ERROR/ */
                                    ) {
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)mb_io_ptr->store_data;

	/* extract ping number from structure */
	*pingnumber = store->scan_count;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       pingnumber: %u\n", *pingnumber);
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
	int ipulse;
	int jnav = 0;
	int jsensordepth = 0;
	int jheading = 0;
	// int	jaltitude = 0;
	int jattitude = 0;
	double amplitude_threshold;
	double target_altitude;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
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
		for (int i = 0; i < pars->n_kluge; i++)
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
	if (pars->sounding_amplitude_filter == MB_YES) {
		amplitude_threshold = pars->sounding_amplitude_threshold;
	} else {
		amplitude_threshold = MBSYS_3DDWISSL_DEFAULT_AMPLITUDE_THRESHOLD;
	}
	if (pars->sounding_altitude_filter == MB_YES) {
		target_altitude = pars->sounding_target_altitude;
	} else {
		target_altitude = MBSYS_3DDWISSL_DEFAULT_TARGET_ALTITUDE;
	}
  if (pars->head1_offsets == MB_YES) {
    store->heada_offset_x_m = pars->head1_offsets_x;
    store->heada_offset_y_m = pars->head1_offsets_y;
    store->heada_offset_z_m = pars->head1_offsets_z;
    store->heada_offset_heading_deg = pars->head1_offsets_heading;
    store->heada_offset_roll_deg = pars->head1_offsets_roll;
    store->heada_offset_pitch_deg = pars->head1_offsets_pitch;
  }
  if (pars->head2_offsets == MB_YES) {
    store->headb_offset_x_m = pars->head2_offsets_x;
    store->headb_offset_y_m = pars->head2_offsets_y;
    store->headb_offset_z_m = pars->head2_offsets_z;
    store->headb_offset_heading_deg = pars->head2_offsets_heading;
    store->headb_offset_roll_deg = pars->head2_offsets_roll;
    store->headb_offset_pitch_deg = pars->head2_offsets_pitch;
  }
	status = mbsys_3ddwissl_calculatebathymetry(verbose, mbio_ptr, store_ptr,
					amplitude_threshold, target_altitude, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:         %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:        %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_sensorhead(int verbose, void *mbio_ptr, void *store_ptr,
							  int *sensorhead, int *error) {
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* if survey data extract which lidar head used for this scan */
	if (store->kind == MB_DATA_DATA) {
		if (store->record_id == MBSYS_3DDWISSL_RECORD_RAWHEADA
			|| store->record_id == MBSYS_3DDWISSL_RECORD_PROHEADA) {
			*sensorhead = 1;
		}
		else if (store->record_id == MBSYS_3DDWISSL_RECORD_RAWHEADB
			|| store->record_id == MBSYS_3DDWISSL_RECORD_PROHEADB) {
			*sensorhead = 0;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       sensorhead: %d\n", *sensorhead);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

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
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_pulse_struct *pulse;
	struct mbsys_3ddwissl_sounding_struct *sounding;
	int ipulse, isounding, ibath;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
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
	int status = MB_SUCCESS;
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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       kind:       %d\n", kind);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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

	if (verbose >= 4)
		mbsys_3ddwissl_print_store(verbose, store, error);
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

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
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(ttimes != NULL);
	assert(angles != NULL);
	assert(angles_forward != NULL);
	assert(angles_null != NULL);
	assert(heave != NULL);
	assert(alongtrack_offset != NULL);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structre pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract travel time data */
	if (*kind == MB_DATA_DATA) {
		/* get the number of soundings */
		*nbeams = store->pulses_per_scan * store->soundings_per_pulse;

		/* get travel times, angles */
		for (int i = 0; i < *nbeams; i++) {
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
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
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
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       detects:    %p\n", detects);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get the number of soundings */
		*nbeams = store->pulses_per_scan * store->soundings_per_pulse;

		/* LIDAR detects */
		for (int i = 0; i < *nbeams; i++)
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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if ((verbose >= 2) && (*error == MB_ERROR_NO_ERROR)) {
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (int i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: detects:%d\n", i, detects[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

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
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       pulses:     %p\n", pulses);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get the number of soundings */
		*nbeams = store->pulses_per_scan * store->soundings_per_pulse;

		/* get pulse type */
		for (int i = 0; i < *nbeams; i++) {
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
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if ((verbose >= 2) && (*error == MB_ERROR_NO_ERROR)) {
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (int i = 0; i < *nbeams; i++)
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
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
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
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_pulse_struct *pulse;
	struct mbsys_3ddwissl_sounding_struct *sounding;
	int ipulse, isounding;
	double rmin, r;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       transducer_depth:  %f\n", *transducer_depth);
		fprintf(stderr, "dbg2       altitude:          %f\n", *altitude);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

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
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;
	int inav;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(nmax > 0);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       nmax:       %d\n", nmax);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       n:          %d\n", *n);
		for (inav = 0; inav < *n; inav++) {
			for (int i = 0; i < 7; i++)
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
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
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
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;
	double dlon, dlat, dheading, dsensordepth, droll, dpitch;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(time_i != NULL);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
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
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
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
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       nsvp:              %d\n", *nsvp);
		for (int i = 0; i < *nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

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
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(nsvp > 0);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       nsvp:       %d\n", nsvp);
		for (int i = 0; i < nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return status;
} /* mbsys_3ddwissl_insert_svp */
/*----------------------------------------------------------------------*/
int mbsys_3ddwissl_copy(int verbose,     /* in: verbosity level set on command line */
                              void *mbio_ptr,  /* in: see mb_io.h:mb_io_struct */
                              void *store_ptr, /* in: see mbsys_3ddwissl.h:mbsys_3ddwissl_struct */
                              void *copy_ptr,  /* out: see mbsys_3ddwissl.h:mbsys_3ddwissl_struct */
                              int *error       /* out: see mb_status.h:MB_ERROR */
                              ) {
	int status = MB_SUCCESS;
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_struct *copy;
	int num_pulses_alloc;
	struct mbsys_3ddwissl_pulse_struct *pulse_ptr;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);
	assert(copy_ptr != NULL);
	assert(store_ptr != copy_ptr);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
		fprintf(stderr, "dbg2       copy_ptr:   %p\n", copy_ptr);
	}

	/* set error status */
	*error = MB_ERROR_NO_ERROR;

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
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
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_pulse_struct *pulse;
	struct mbsys_3ddwissl_sounding_struct *sounding;
	int status;
	char *debug_str = "dbg2  ";
	char *nodebug_str = "  ";
	char *first;
	int ipulse, isounding;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
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
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
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
	if (store->kind == MB_DATA_PARAMETER && store->file_version == 1 && store->sub_version == 1) {
		fprintf(stderr, "%s     calibration A: cfg_path:                      %s\n", first, store->calibration_v1s1_a.cfg_path);
		fprintf(stderr, "%s     calibration A: laser_head_no:                 %d\n", first, store->calibration_v1s1_a.laser_head_no);
		fprintf(stderr, "%s     calibration A: process_for_air:               %d\n", first, store->calibration_v1s1_a.process_for_air);
		fprintf(stderr, "%s     calibration A: temperature_compensation:      %d\n", first, store->calibration_v1s1_a.temperature_compensation);
		fprintf(stderr, "%s     calibration A: emergency_shutdown:            %d\n", first, store->calibration_v1s1_a.emergency_shutdown);
		fprintf(stderr, "%s     calibration A: ocb_temperature_limit_c:       %f\n", first, store->calibration_v1s1_a.ocb_temperature_limit_c);
		fprintf(stderr, "%s     calibration A: ocb_temperature_limit_c:       %f\n", first, store->calibration_v1s1_a.ocb_temperature_limit_c);
		fprintf(stderr, "%s     calibration A: ocb_humidity_limit:            %f\n", first, store->calibration_v1s1_a.ocb_humidity_limit);
		fprintf(stderr, "%s     calibration A: pb_temperature_limit_1_c:      %f\n", first, store->calibration_v1s1_a.pb_temperature_limit_1_c);
		fprintf(stderr, "%s     calibration A: pb_temperature_limit_2_c:      %f\n", first, store->calibration_v1s1_a.pb_temperature_limit_2_c);
		fprintf(stderr, "%s     calibration A: pb_humidity_limit:             %f\n", first, store->calibration_v1s1_a.pb_humidity_limit);
		fprintf(stderr, "%s     calibration A: dig_temperature_limit_c:       %f\n", first, store->calibration_v1s1_a.dig_temperature_limit_c);
		fprintf(stderr, "%s     calibration A: l_d_cable_set:                 %s\n", first, store->calibration_v1s1_a.l_d_cable_set);
		fprintf(stderr, "%s     calibration A: ocb_comm_port:                 %s\n", first, store->calibration_v1s1_a.ocb_comm_port);
		fprintf(stderr, "%s     calibration A: ocb_comm_cfg:                  %s\n", first, store->calibration_v1s1_a.ocb_comm_cfg);
		fprintf(stderr, "%s     calibration A: az_ao_deg_to_volt:             %f\n", first, store->calibration_v1s1_a.az_ao_deg_to_volt);
		fprintf(stderr, "%s     calibration A: az_ai_neg_v_to_deg:            %f\n", first, store->calibration_v1s1_a.az_ai_neg_v_to_deg);
		fprintf(stderr, "%s     calibration A: az_ai_pos_v_to_deg:            %f\n", first, store->calibration_v1s1_a.az_ai_pos_v_to_deg);
		fprintf(stderr, "%s     calibration A: t1_air:                        %f\n", first, store->calibration_v1s1_a.t1_air);
		fprintf(stderr, "%s     calibration A: ff_air:                        %f\n", first, store->calibration_v1s1_a.ff_air);
		fprintf(stderr, "%s     calibration A: t1_water_g4000:                %f\n", first, store->calibration_v1s1_a.t1_water_g4000);
		fprintf(stderr, "%s     calibration A: ff_water_g4000:                %f\n", first, store->calibration_v1s1_a.ff_water_g4000);
		fprintf(stderr, "%s     calibration A: t1_water_g3000:                %f\n", first, store->calibration_v1s1_a.t1_water_g3000);
		fprintf(stderr, "%s     calibration A: ff_water_g3000:                %f\n", first, store->calibration_v1s1_a.ff_water_g3000);
		fprintf(stderr, "%s     calibration A: t1_water_g2000:                %f\n", first, store->calibration_v1s1_a.t1_water_g2000);
		fprintf(stderr, "%s     calibration A: ff_water_g2000:                %f\n", first, store->calibration_v1s1_a.ff_water_g2000);
		fprintf(stderr, "%s     calibration A: t1_water_g1000:                %f\n", first, store->calibration_v1s1_a.t1_water_g1000);
		fprintf(stderr, "%s     calibration A: ff_water_g1000:                %f\n", first, store->calibration_v1s1_a.ff_water_g1000);
		fprintf(stderr, "%s     calibration A: t1_water_g400:                 %f\n", first, store->calibration_v1s1_a.t1_water_g400);
		fprintf(stderr, "%s     calibration A: ff_water_g400:                 %f\n", first, store->calibration_v1s1_a.ff_water_g400);
		fprintf(stderr, "%s     calibration A: t1_water_g300:                 %f\n", first, store->calibration_v1s1_a.t1_water_g300);
		fprintf(stderr, "%s     calibration A: ff_water_g300:                 %f\n", first, store->calibration_v1s1_a.ff_water_g300);
		fprintf(stderr, "%s     calibration A: t1_water_secondary_g4000:      %f\n", first, store->calibration_v1s1_a.t1_water_secondary_g4000);
		fprintf(stderr, "%s     calibration A: ff_water_secondary_g4000:      %f\n", first, store->calibration_v1s1_a.ff_water_secondary_g4000);
		fprintf(stderr, "%s     calibration A: t1_water_secondary_g3000:      %f\n", first, store->calibration_v1s1_a.t1_water_secondary_g3000);
		fprintf(stderr, "%s     calibration A: ff_water_secondary_g3000:      %f\n", first, store->calibration_v1s1_a.ff_water_secondary_g3000);
		fprintf(stderr, "%s     calibration A: t1_water_secondary_g2000:      %f\n", first, store->calibration_v1s1_a.t1_water_secondary_g2000);
		fprintf(stderr, "%s     calibration A: ff_water_secondary_g2000:      %f\n", first, store->calibration_v1s1_a.ff_water_secondary_g2000);
		fprintf(stderr, "%s     calibration A: t1_water_secondary_g1000:      %f\n", first, store->calibration_v1s1_a.t1_water_secondary_g1000);
		fprintf(stderr, "%s     calibration A: ff_water_secondary_g1000:      %f\n", first, store->calibration_v1s1_a.ff_water_secondary_g1000);
		fprintf(stderr, "%s     calibration A: t1_water_secondary_g400:       %f\n", first, store->calibration_v1s1_a.t1_water_secondary_g400);
		fprintf(stderr, "%s     calibration A: ff_water_secondary_g400:       %f\n", first, store->calibration_v1s1_a.ff_water_secondary_g400);
		fprintf(stderr, "%s     calibration A: t1_water_secondary_g300:       %f\n", first, store->calibration_v1s1_a.t1_water_secondary_g300);
		fprintf(stderr, "%s     calibration A: ff_water_secondary_g300:       %f\n", first, store->calibration_v1s1_a.ff_water_secondary_g300);
		fprintf(stderr, "%s     calibration A: temp_comp_poly2:               %f\n", first, store->calibration_v1s1_a.temp_comp_poly2);
		fprintf(stderr, "%s     calibration A: temp_comp_poly1:               %f\n", first, store->calibration_v1s1_a.temp_comp_poly1);
		fprintf(stderr, "%s     calibration A: temp_comp_poly:                %f\n", first, store->calibration_v1s1_a.temp_comp_poly);
		fprintf(stderr, "%s     calibration A: laser_start_time_sec:          %f\n", first, store->calibration_v1s1_a.laser_start_time_sec);
		fprintf(stderr, "%s     calibration A: scanner_shift_cts:             %f\n", first, store->calibration_v1s1_a.scanner_shift_cts);
		fprintf(stderr, "%s     calibration A: factory_scanner_lrg_deg:       %f\n", first, store->calibration_v1s1_a.factory_scanner_lrg_deg);
		fprintf(stderr, "%s     calibration A: factory_scanner_med_deg:       %f\n", first, store->calibration_v1s1_a.factory_scanner_med_deg);
		fprintf(stderr, "%s     calibration A: factory_scanner_sml_deg:       %f\n", first, store->calibration_v1s1_a.factory_scanner_sml_deg);
		fprintf(stderr, "%s     calibration A: el_angle_fixed_deg:            %f\n", first, store->calibration_v1s1_a.el_angle_fixed_deg);
		fprintf(stderr, "%s     calibration B: cfg_path:                      %s\n", first, store->calibration_v1s1_b.cfg_path);
		fprintf(stderr, "%s     calibration B: laser_head_no:                 %d\n", first, store->calibration_v1s1_b.laser_head_no);
		fprintf(stderr, "%s     calibration B: process_for_air:               %d\n", first, store->calibration_v1s1_b.process_for_air);
		fprintf(stderr, "%s     calibration B: temperature_compensation:      %d\n", first, store->calibration_v1s1_b.temperature_compensation);
		fprintf(stderr, "%s     calibration B: emergency_shutdown:            %d\n", first, store->calibration_v1s1_b.emergency_shutdown);
		fprintf(stderr, "%s     calibration B: ocb_temperature_limit_c:       %f\n", first, store->calibration_v1s1_b.ocb_temperature_limit_c);
		fprintf(stderr, "%s     calibration B: ocb_temperature_limit_c:       %f\n", first, store->calibration_v1s1_b.ocb_temperature_limit_c);
		fprintf(stderr, "%s     calibration B: ocb_humidity_limit:            %f\n", first, store->calibration_v1s1_b.ocb_humidity_limit);
		fprintf(stderr, "%s     calibration B: pb_temperature_limit_1_c:      %f\n", first, store->calibration_v1s1_b.pb_temperature_limit_1_c);
		fprintf(stderr, "%s     calibration B: pb_temperature_limit_2_c:      %f\n", first, store->calibration_v1s1_b.pb_temperature_limit_2_c);
		fprintf(stderr, "%s     calibration B: pb_humidity_limit:             %f\n", first, store->calibration_v1s1_b.pb_humidity_limit);
		fprintf(stderr, "%s     calibration B: dig_temperature_limit_c:       %f\n", first, store->calibration_v1s1_b.dig_temperature_limit_c);
		fprintf(stderr, "%s     calibration B: l_d_cable_set:                 %s\n", first, store->calibration_v1s1_b.l_d_cable_set);
		fprintf(stderr, "%s     calibration B: ocb_comm_port:                 %s\n", first, store->calibration_v1s1_b.ocb_comm_port);
		fprintf(stderr, "%s     calibration B: ocb_comm_cfg:                  %s\n", first, store->calibration_v1s1_b.ocb_comm_cfg);
		fprintf(stderr, "%s     calibration B: az_ao_deg_to_volt:             %f\n", first, store->calibration_v1s1_b.az_ao_deg_to_volt);
		fprintf(stderr, "%s     calibration B: az_ai_neg_v_to_deg:            %f\n", first, store->calibration_v1s1_b.az_ai_neg_v_to_deg);
		fprintf(stderr, "%s     calibration B: az_ai_pos_v_to_deg:            %f\n", first, store->calibration_v1s1_b.az_ai_pos_v_to_deg);
		fprintf(stderr, "%s     calibration B: t1_air:                        %f\n", first, store->calibration_v1s1_b.t1_air);
		fprintf(stderr, "%s     calibration B: ff_air:                        %f\n", first, store->calibration_v1s1_b.ff_air);
		fprintf(stderr, "%s     calibration B: t1_water_g4000:                %f\n", first, store->calibration_v1s1_b.t1_water_g4000);
		fprintf(stderr, "%s     calibration B: ff_water_g4000:                %f\n", first, store->calibration_v1s1_b.ff_water_g4000);
		fprintf(stderr, "%s     calibration B: t1_water_g3000:                %f\n", first, store->calibration_v1s1_b.t1_water_g3000);
		fprintf(stderr, "%s     calibration B: ff_water_g3000:                %f\n", first, store->calibration_v1s1_b.ff_water_g3000);
		fprintf(stderr, "%s     calibration B: t1_water_g2000:                %f\n", first, store->calibration_v1s1_b.t1_water_g2000);
		fprintf(stderr, "%s     calibration B: ff_water_g2000:                %f\n", first, store->calibration_v1s1_b.ff_water_g2000);
		fprintf(stderr, "%s     calibration B: t1_water_g1000:                %f\n", first, store->calibration_v1s1_b.t1_water_g1000);
		fprintf(stderr, "%s     calibration B: ff_water_g1000:                %f\n", first, store->calibration_v1s1_b.ff_water_g1000);
		fprintf(stderr, "%s     calibration B: t1_water_g400:                 %f\n", first, store->calibration_v1s1_b.t1_water_g400);
		fprintf(stderr, "%s     calibration B: ff_water_g400:                 %f\n", first, store->calibration_v1s1_b.ff_water_g400);
		fprintf(stderr, "%s     calibration B: t1_water_g300:                 %f\n", first, store->calibration_v1s1_b.t1_water_g300);
		fprintf(stderr, "%s     calibration B: ff_water_g300:                 %f\n", first, store->calibration_v1s1_b.ff_water_g300);
		fprintf(stderr, "%s     calibration B: t1_water_secondary_g4000:      %f\n", first, store->calibration_v1s1_b.t1_water_secondary_g4000);
		fprintf(stderr, "%s     calibration B: ff_water_secondary_g4000:      %f\n", first, store->calibration_v1s1_b.ff_water_secondary_g4000);
		fprintf(stderr, "%s     calibration B: t1_water_secondary_g3000:      %f\n", first, store->calibration_v1s1_b.t1_water_secondary_g3000);
		fprintf(stderr, "%s     calibration B: ff_water_secondary_g3000:      %f\n", first, store->calibration_v1s1_b.ff_water_secondary_g3000);
		fprintf(stderr, "%s     calibration B: t1_water_secondary_g2000:      %f\n", first, store->calibration_v1s1_b.t1_water_secondary_g2000);
		fprintf(stderr, "%s     calibration B: ff_water_secondary_g2000:      %f\n", first, store->calibration_v1s1_b.ff_water_secondary_g2000);
		fprintf(stderr, "%s     calibration B: t1_water_secondary_g1000:      %f\n", first, store->calibration_v1s1_b.t1_water_secondary_g1000);
		fprintf(stderr, "%s     calibration B: ff_water_secondary_g1000:      %f\n", first, store->calibration_v1s1_b.ff_water_secondary_g1000);
		fprintf(stderr, "%s     calibration B: t1_water_secondary_g400:       %f\n", first, store->calibration_v1s1_b.t1_water_secondary_g400);
		fprintf(stderr, "%s     calibration B: ff_water_secondary_g400:       %f\n", first, store->calibration_v1s1_b.ff_water_secondary_g400);
		fprintf(stderr, "%s     calibration B: t1_water_secondary_g300:       %f\n", first, store->calibration_v1s1_b.t1_water_secondary_g300);
		fprintf(stderr, "%s     calibration B: ff_water_secondary_g300:       %f\n", first, store->calibration_v1s1_b.ff_water_secondary_g300);
		fprintf(stderr, "%s     calibration B: temp_comp_poly2:               %f\n", first, store->calibration_v1s1_b.temp_comp_poly2);
		fprintf(stderr, "%s     calibration B: temp_comp_poly1:               %f\n", first, store->calibration_v1s1_b.temp_comp_poly1);
		fprintf(stderr, "%s     calibration B: temp_comp_poly:                %f\n", first, store->calibration_v1s1_b.temp_comp_poly);
		fprintf(stderr, "%s     calibration B: laser_start_time_sec:          %f\n", first, store->calibration_v1s1_b.laser_start_time_sec);
		fprintf(stderr, "%s     calibration B: scanner_shift_cts:             %f\n", first, store->calibration_v1s1_b.scanner_shift_cts);
		fprintf(stderr, "%s     calibration B: factory_scanner_lrg_deg:       %f\n", first, store->calibration_v1s1_b.factory_scanner_lrg_deg);
		fprintf(stderr, "%s     calibration B: factory_scanner_med_deg:       %f\n", first, store->calibration_v1s1_b.factory_scanner_med_deg);
		fprintf(stderr, "%s     calibration B: factory_scanner_sml_deg:       %f\n", first, store->calibration_v1s1_b.factory_scanner_sml_deg);
		fprintf(stderr, "%s     calibration B: el_angle_fixed_deg:            %f\n", first, store->calibration_v1s1_b.el_angle_fixed_deg);
	} else if (store->kind == MB_DATA_PARAMETER && store->file_version == 1 && store->sub_version == 2) {
		fprintf(stderr, "%s     calibration A: cfg_path:                      %s\n", first, store->calibration_v1s2_a.cfg_path);
		fprintf(stderr, "%s     calibration A: laser_head_no:                 %d\n", first, store->calibration_v1s2_a.laser_head_no);
		fprintf(stderr, "%s     calibration A: process_for_air:               %d\n", first, store->calibration_v1s2_a.process_for_air);
		fprintf(stderr, "%s     calibration A: temperature_compensation:      %d\n", first, store->calibration_v1s2_a.temperature_compensation);
		fprintf(stderr, "%s     calibration A: emergency_shutdown:            %d\n", first, store->calibration_v1s2_a.emergency_shutdown);
		fprintf(stderr, "%s     calibration A: ocb_temperature_limit_c:       %f\n", first, store->calibration_v1s2_a.ocb_temperature_limit_c);
		fprintf(stderr, "%s     calibration A: ocb_temperature_limit_c:       %f\n", first, store->calibration_v1s2_a.ocb_temperature_limit_c);
		fprintf(stderr, "%s     calibration A: ocb_humidity_limit:            %f\n", first, store->calibration_v1s2_a.ocb_humidity_limit);
		fprintf(stderr, "%s     calibration A: pb_temperature_limit_1_c:      %f\n", first, store->calibration_v1s2_a.pb_temperature_limit_1_c);
		fprintf(stderr, "%s     calibration A: pb_temperature_limit_2_c:      %f\n", first, store->calibration_v1s2_a.pb_temperature_limit_2_c);
		fprintf(stderr, "%s     calibration A: pb_humidity_limit:             %f\n", first, store->calibration_v1s2_a.pb_humidity_limit);
		fprintf(stderr, "%s     calibration A: dig_temperature_limit_c:       %f\n", first, store->calibration_v1s2_a.dig_temperature_limit_c);
		fprintf(stderr, "%s     calibration A: ocb_comm_port:                 %s\n", first, store->calibration_v1s2_a.ocb_comm_port);
		fprintf(stderr, "%s     calibration A: ocb_comm_cfg:                  %s\n", first, store->calibration_v1s2_a.ocb_comm_cfg);
		fprintf(stderr, "%s     calibration A: az_ao_deg_to_volt:             %f\n", first, store->calibration_v1s2_a.az_ao_deg_to_volt);
		fprintf(stderr, "%s     calibration A: az_ai_neg_v_to_deg:            %f\n", first, store->calibration_v1s2_a.az_ai_neg_v_to_deg);
		fprintf(stderr, "%s     calibration A: az_ai_pos_v_to_deg:            %f\n", first, store->calibration_v1s2_a.az_ai_pos_v_to_deg);
		fprintf(stderr, "%s     calibration A: t1_air:                        %f\n", first, store->calibration_v1s2_a.t1_air);
		fprintf(stderr, "%s     calibration A: ff_air:                        %f\n", first, store->calibration_v1s2_a.ff_air);
		fprintf(stderr, "%s     calibration A: t1_water_g4000:                %f\n", first, store->calibration_v1s2_a.t1_water_g4000);
		fprintf(stderr, "%s     calibration A: ff_water_g4000:                %f\n", first, store->calibration_v1s2_a.ff_water_g4000);
		fprintf(stderr, "%s     calibration A: t1_water_g3000:                %f\n", first, store->calibration_v1s2_a.t1_water_g3000);
		fprintf(stderr, "%s     calibration A: ff_water_g3000:                %f\n", first, store->calibration_v1s2_a.ff_water_g3000);
		fprintf(stderr, "%s     calibration A: t1_water_g2000:                %f\n", first, store->calibration_v1s2_a.t1_water_g2000);
		fprintf(stderr, "%s     calibration A: ff_water_g2000:                %f\n", first, store->calibration_v1s2_a.ff_water_g2000);
		fprintf(stderr, "%s     calibration A: t1_water_g1000:                %f\n", first, store->calibration_v1s2_a.t1_water_g1000);
		fprintf(stderr, "%s     calibration A: ff_water_g1000:                %f\n", first, store->calibration_v1s2_a.ff_water_g1000);
		fprintf(stderr, "%s     calibration A: t1_water_g400:                 %f\n", first, store->calibration_v1s2_a.t1_water_g400);
		fprintf(stderr, "%s     calibration A: ff_water_g400:                 %f\n", first, store->calibration_v1s2_a.ff_water_g400);
		fprintf(stderr, "%s     calibration A: t1_water_g300:                 %f\n", first, store->calibration_v1s2_a.t1_water_g300);
		fprintf(stderr, "%s     calibration A: ff_water_g300:                 %f\n", first, store->calibration_v1s2_a.ff_water_g300);
		fprintf(stderr, "%s     calibration A: temp_comp_poly2:               %f\n", first, store->calibration_v1s2_a.temp_comp_poly2);
		fprintf(stderr, "%s     calibration A: temp_comp_poly1:               %f\n", first, store->calibration_v1s2_a.temp_comp_poly1);
		fprintf(stderr, "%s     calibration A: temp_comp_poly:                %f\n", first, store->calibration_v1s2_a.temp_comp_poly);
		fprintf(stderr, "%s     calibration A: laser_start_time_sec:          %f\n", first, store->calibration_v1s2_a.laser_start_time_sec);
		fprintf(stderr, "%s     calibration A: scanner_shift_cts:             %f\n", first, store->calibration_v1s2_a.scanner_shift_cts);
		fprintf(stderr, "%s     calibration A: factory_scanner_lrg_deg:       %f\n", first, store->calibration_v1s2_a.factory_scanner_lrg_deg);
		fprintf(stderr, "%s     calibration A: factory_scanner_med_deg:       %f\n", first, store->calibration_v1s2_a.factory_scanner_med_deg);
		fprintf(stderr, "%s     calibration A: factory_scanner_sml_deg:       %f\n", first, store->calibration_v1s2_a.factory_scanner_sml_deg);
		fprintf(stderr, "%s     calibration A: el_angle_fixed_deg:            %f\n", first, store->calibration_v1s2_a.el_angle_fixed_deg);
		fprintf(stderr, "%s     calibration A: zda_to_pps_max_msec            %d\n", first, store->calibration_v1s2_a.zda_to_pps_max_msec);
		fprintf(stderr, "%s     calibration A: zda_udp_port                   %d\n", first, store->calibration_v1s2_a.zda_udp_port);
		fprintf(stderr, "%s     calibration A: show_time_sync_errors          %d\n", first, store->calibration_v1s2_a.show_time_sync_errors);
		fprintf(stderr, "%s     calibration A: min_time_diff_update_msec      %d\n", first, store->calibration_v1s2_a.min_time_diff_update_msec);
		fprintf(stderr, "%s     calibration A:  ctd_tcp_port                  %d\n", first, store->calibration_v1s2_a.ctd_tcp_port);
		fprintf(stderr, "%s     calibration A: trigger_level_volt             %f\n", first, store->calibration_v1s2_a.trigger_level_volt);
		fprintf(stderr, "%s     calibration A: mf_t0_position                 %d\n", first, store->calibration_v1s2_a.mf_t0_position);
		fprintf(stderr, "%s     calibration A: mf_start_proc                  %d\n", first, store->calibration_v1s2_a.mf_start_proc);
		fprintf(stderr, "%s     calibration A: dig_ref_pos_t0_cnts            %d\n", first, store->calibration_v1s2_a.dig_ref_pos_t0_cnts);
		fprintf(stderr, "%s     calibration A: dummy                          %d\n", first, store->calibration_v1s2_a.dummy);
		fprintf(stderr, "%s     calibration A:  t0_min_height_raw_cts         %d\n", first, store->calibration_v1s2_a.t0_min_height_raw_cts);
		fprintf(stderr, "%s     calibration A: scanner_neg_polynom_0          %f\n", first, store->calibration_v1s2_a.scanner_neg_polynom_0);
		fprintf(stderr, "%s     calibration A: scanner_neg_polynom_1          %f\n", first, store->calibration_v1s2_a.scanner_neg_polynom_1);
		fprintf(stderr, "%s     calibration A: scanner_neg_polynom_2          %f\n", first, store->calibration_v1s2_a.scanner_neg_polynom_2);
		fprintf(stderr, "%s     calibration A: scanner_neg_polynom_3          %f\n", first, store->calibration_v1s2_a.scanner_neg_polynom_3);
		fprintf(stderr, "%s     calibration A: scanner_neg_polynom_4          %f\n", first, store->calibration_v1s2_a.scanner_neg_polynom_4);
		fprintf(stderr, "%s     calibration A: scanner_neg_polynom_5          %f\n", first, store->calibration_v1s2_a.scanner_neg_polynom_5);
		fprintf(stderr, "%s     calibration A: scanner_pos_polynom_0          %f\n", first, store->calibration_v1s2_a.scanner_pos_polynom_0);
		fprintf(stderr, "%s     calibration A: scanner_pos_polynom_1          %f\n", first, store->calibration_v1s2_a.scanner_pos_polynom_1);
		fprintf(stderr, "%s     calibration A: scanner_pos_polynom_2          %f\n", first, store->calibration_v1s2_a.scanner_pos_polynom_2);
		fprintf(stderr, "%s     calibration A: scanner_pos_polynom_3          %f\n", first, store->calibration_v1s2_a.scanner_pos_polynom_3);
		fprintf(stderr, "%s     calibration A: scanner_pos_polynom_4          %f\n", first, store->calibration_v1s2_a.scanner_pos_polynom_4);
		fprintf(stderr, "%s     calibration A: scanner_pos_polynom_5          %f\n", first, store->calibration_v1s2_a.scanner_pos_polynom_5);

		fprintf(stderr, "%s     calibration B: cfg_path:                      %s\n", first, store->calibration_v1s2_b.cfg_path);
		fprintf(stderr, "%s     calibration B: laser_head_no:                 %d\n", first, store->calibration_v1s2_b.laser_head_no);
		fprintf(stderr, "%s     calibration B: process_for_air:               %d\n", first, store->calibration_v1s2_b.process_for_air);
		fprintf(stderr, "%s     calibration B: temperature_compensation:      %d\n", first, store->calibration_v1s2_b.temperature_compensation);
		fprintf(stderr, "%s     calibration B: emergency_shutdown:            %d\n", first, store->calibration_v1s2_b.emergency_shutdown);
		fprintf(stderr, "%s     calibration B: ocb_temperature_limit_c:       %f\n", first, store->calibration_v1s2_b.ocb_temperature_limit_c);
		fprintf(stderr, "%s     calibration B: ocb_temperature_limit_c:       %f\n", first, store->calibration_v1s2_b.ocb_temperature_limit_c);
		fprintf(stderr, "%s     calibration B: ocb_humidity_limit:            %f\n", first, store->calibration_v1s2_b.ocb_humidity_limit);
		fprintf(stderr, "%s     calibration B: pb_temperature_limit_1_c:      %f\n", first, store->calibration_v1s2_b.pb_temperature_limit_1_c);
		fprintf(stderr, "%s     calibration B: pb_temperature_limit_2_c:      %f\n", first, store->calibration_v1s2_b.pb_temperature_limit_2_c);
		fprintf(stderr, "%s     calibration B: pb_humidity_limit:             %f\n", first, store->calibration_v1s2_b.pb_humidity_limit);
		fprintf(stderr, "%s     calibration B: dig_temperature_limit_c:       %f\n", first, store->calibration_v1s2_b.dig_temperature_limit_c);
		fprintf(stderr, "%s     calibration B: ocb_comm_port:                 %s\n", first, store->calibration_v1s2_b.ocb_comm_port);
		fprintf(stderr, "%s     calibration B: ocb_comm_cfg:                  %s\n", first, store->calibration_v1s2_b.ocb_comm_cfg);
		fprintf(stderr, "%s     calibration B: az_ao_deg_to_volt:             %f\n", first, store->calibration_v1s2_b.az_ao_deg_to_volt);
		fprintf(stderr, "%s     calibration B: az_ai_neg_v_to_deg:            %f\n", first, store->calibration_v1s2_b.az_ai_neg_v_to_deg);
		fprintf(stderr, "%s     calibration B: az_ai_pos_v_to_deg:            %f\n", first, store->calibration_v1s2_b.az_ai_pos_v_to_deg);
		fprintf(stderr, "%s     calibration B: t1_air:                        %f\n", first, store->calibration_v1s2_b.t1_air);
		fprintf(stderr, "%s     calibration B: ff_air:                        %f\n", first, store->calibration_v1s2_b.ff_air);
		fprintf(stderr, "%s     calibration B: t1_water_g4000:                %f\n", first, store->calibration_v1s2_b.t1_water_g4000);
		fprintf(stderr, "%s     calibration B: ff_water_g4000:                %f\n", first, store->calibration_v1s2_b.ff_water_g4000);
		fprintf(stderr, "%s     calibration B: t1_water_g3000:                %f\n", first, store->calibration_v1s2_b.t1_water_g3000);
		fprintf(stderr, "%s     calibration B: ff_water_g3000:                %f\n", first, store->calibration_v1s2_b.ff_water_g3000);
		fprintf(stderr, "%s     calibration B: t1_water_g2000:                %f\n", first, store->calibration_v1s2_b.t1_water_g2000);
		fprintf(stderr, "%s     calibration B: ff_water_g2000:                %f\n", first, store->calibration_v1s2_b.ff_water_g2000);
		fprintf(stderr, "%s     calibration B: t1_water_g1000:                %f\n", first, store->calibration_v1s2_b.t1_water_g1000);
		fprintf(stderr, "%s     calibration B: ff_water_g1000:                %f\n", first, store->calibration_v1s2_b.ff_water_g1000);
		fprintf(stderr, "%s     calibration B: t1_water_g400:                 %f\n", first, store->calibration_v1s2_b.t1_water_g400);
		fprintf(stderr, "%s     calibration B: ff_water_g400:                 %f\n", first, store->calibration_v1s2_b.ff_water_g400);
		fprintf(stderr, "%s     calibration B: t1_water_g300:                 %f\n", first, store->calibration_v1s2_b.t1_water_g300);
		fprintf(stderr, "%s     calibration B: ff_water_g300:                 %f\n", first, store->calibration_v1s2_b.ff_water_g300);
		fprintf(stderr, "%s     calibration B: temp_comp_poly2:               %f\n", first, store->calibration_v1s2_b.temp_comp_poly2);
		fprintf(stderr, "%s     calibration B: temp_comp_poly1:               %f\n", first, store->calibration_v1s2_b.temp_comp_poly1);
		fprintf(stderr, "%s     calibration B: temp_comp_poly:                %f\n", first, store->calibration_v1s2_b.temp_comp_poly);
		fprintf(stderr, "%s     calibration B: laser_start_time_sec:          %f\n", first, store->calibration_v1s2_b.laser_start_time_sec);
		fprintf(stderr, "%s     calibration B: scanner_shift_cts:             %f\n", first, store->calibration_v1s2_b.scanner_shift_cts);
		fprintf(stderr, "%s     calibration B: factory_scanner_lrg_deg:       %f\n", first, store->calibration_v1s2_b.factory_scanner_lrg_deg);
		fprintf(stderr, "%s     calibration B: factory_scanner_med_deg:       %f\n", first, store->calibration_v1s2_b.factory_scanner_med_deg);
		fprintf(stderr, "%s     calibration B: factory_scanner_sml_deg:       %f\n", first, store->calibration_v1s2_b.factory_scanner_sml_deg);
		fprintf(stderr, "%s     calibration B: el_angle_fixed_deg:            %f\n", first, store->calibration_v1s2_b.el_angle_fixed_deg);
		fprintf(stderr, "%s     calibration B: zda_to_pps_max_msec            %d\n", first, store->calibration_v1s2_b.zda_to_pps_max_msec);
		fprintf(stderr, "%s     calibration B: zda_udp_port                   %d\n", first, store->calibration_v1s2_b.zda_udp_port);
		fprintf(stderr, "%s     calibration B: show_time_sync_errors          %d\n", first, store->calibration_v1s2_b.show_time_sync_errors);
		fprintf(stderr, "%s     calibration B: min_time_diff_update_msec      %d\n", first, store->calibration_v1s2_b.min_time_diff_update_msec);
		fprintf(stderr, "%s     calibration B:  ctd_tcp_port                  %d\n", first, store->calibration_v1s2_b.ctd_tcp_port);
		fprintf(stderr, "%s     calibration B: trigger_level_volt             %f\n", first, store->calibration_v1s2_b.trigger_level_volt);
		fprintf(stderr, "%s     calibration B: mf_t0_position                 %d\n", first, store->calibration_v1s2_b.mf_t0_position);
		fprintf(stderr, "%s     calibration B: mf_start_proc                  %d\n", first, store->calibration_v1s2_b.mf_start_proc);
		fprintf(stderr, "%s     calibration B: dig_ref_pos_t0_cnts            %d\n", first, store->calibration_v1s2_b.dig_ref_pos_t0_cnts);
		fprintf(stderr, "%s     calibration B: dummy                          %d\n", first, store->calibration_v1s2_b.dummy);
		fprintf(stderr, "%s     calibration B:  t0_min_height_raw_cts         %d\n", first, store->calibration_v1s2_b.t0_min_height_raw_cts);
		fprintf(stderr, "%s     calibration B: scanner_neg_polynom_0          %f\n", first, store->calibration_v1s2_b.scanner_neg_polynom_0);
		fprintf(stderr, "%s     calibration B: scanner_neg_polynom_1          %f\n", first, store->calibration_v1s2_b.scanner_neg_polynom_1);
		fprintf(stderr, "%s     calibration B: scanner_neg_polynom_2          %f\n", first, store->calibration_v1s2_b.scanner_neg_polynom_2);
		fprintf(stderr, "%s     calibration B: scanner_neg_polynom_3          %f\n", first, store->calibration_v1s2_b.scanner_neg_polynom_3);
		fprintf(stderr, "%s     calibration B: scanner_neg_polynom_4          %f\n", first, store->calibration_v1s2_b.scanner_neg_polynom_4);
		fprintf(stderr, "%s     calibration B: scanner_neg_polynom_5          %f\n", first, store->calibration_v1s2_b.scanner_neg_polynom_5);
		fprintf(stderr, "%s     calibration B: scanner_pos_polynom_0          %f\n", first, store->calibration_v1s2_b.scanner_pos_polynom_0);
		fprintf(stderr, "%s     calibration B: scanner_pos_polynom_1          %f\n", first, store->calibration_v1s2_b.scanner_pos_polynom_1);
		fprintf(stderr, "%s     calibration B: scanner_pos_polynom_2          %f\n", first, store->calibration_v1s2_b.scanner_pos_polynom_2);
		fprintf(stderr, "%s     calibration B: scanner_pos_polynom_3          %f\n", first, store->calibration_v1s2_b.scanner_pos_polynom_3);
		fprintf(stderr, "%s     calibration B: scanner_pos_polynom_4          %f\n", first, store->calibration_v1s2_b.scanner_pos_polynom_4);
		fprintf(stderr, "%s     calibration B: scanner_pos_polynom_5          %f\n", first, store->calibration_v1s2_b.scanner_pos_polynom_5);
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
		fprintf(stderr, "%s     validpulse_count:              %d\n", first, store->validpulse_count);
		fprintf(stderr, "%s     validsounding_count:           %d\n", first, store->validsounding_count);
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

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
} /* mbsys_3ddwissl_print_store */
/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_calculatebathymetry(int verbose,     /* in: verbosity level set on command line 0..N */
                                             void *mbio_ptr,  /* in: see mb_io.h:mb_io_struct */
                                             void *store_ptr, /* in: see mbsys_3ddwissl.h:mbsys_3ddwissl_struct */
											 double amplitude_threshold, /* used to determine valid soundings */
 											 double target_altitude, /* used to prioritize picks close to a desired standoff */
                                             int *error       /* out: see mb_status.h:MB_ERROR */
                                             ) {
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_pulse_struct *pulse;
	struct mbsys_3ddwissl_sounding_struct *sounding;
	int status;
	int time_i[7];
	double alpha, beta, theta, phi;
	double angle_az_sign, angle_el_sign;
	double mtodeglon, mtodeglat;
	double xx;
	int ipulse, isounding, isounding_largest;
	short amplitude_largest, amplitude_max;
    double head_offset_x_m;
    double head_offset_y_m;
    double head_offset_z_m;
    double head_offset_heading_deg;
    double head_offset_roll_deg;
    double head_offset_pitch_deg;
	double amplitude_factor;
	double target_range;
	double scaled_range_diff;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:               %d\n", verbose);
		fprintf(stderr, "dbg2         store:               %p\n", store_ptr);
		fprintf(stderr, "dbg2         amplitude_threshold: %f\n", amplitude_threshold);
		fprintf(stderr, "dbg2         target_altitude:     %f\n", target_altitude);
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
			angle_el_sign = -1.0;
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

		/* figure out valid amplitude threshold */
		for (ipulse = 0; ipulse < store->pulses_per_scan; ipulse++) {
			pulse = (struct mbsys_3ddwissl_pulse_struct *)&store->pulses[ipulse];
			amplitude_max = 0;
			for (isounding=0; isounding<store->soundings_per_pulse; isounding++) {
				sounding = &pulse->soundings[isounding];
				/* valid pulses have nonzero ranges */
				if (sounding->range > 0.001 && sounding->amplitude > amplitude_max) {
					amplitude_max = sounding->amplitude;
				}
			}
		}
//fprintf(stderr,"\namplitude_max:%d amplitude_threshold:%f\n", amplitude_max, amplitude_threshold);

		/* loop over all pulses and soundings */
		for (ipulse = 0; ipulse < store->pulses_per_scan; ipulse++) {
			pulse = (struct mbsys_3ddwissl_pulse_struct *)&store->pulses[ipulse];
			isounding_largest = -1;
			amplitude_largest = 0;
			for (isounding=0; isounding<store->soundings_per_pulse; isounding++) {
				sounding = &pulse->soundings[isounding];

				/* valid pulses have nonzero ranges */
				if (sounding->range > 0.001) {

					/* apply pitch and roll */
					alpha = angle_el_sign * pulse->angle_el
										+ store->pitch
										+ head_offset_pitch_deg
										+ pulse->pitch_offset;
					beta = 90.0 - (angle_az_sign * pulse->angle_az)
										+ store->roll
										+ head_offset_roll_deg
										+ pulse->roll_offset;

					/* calculate amplitude range factor */
					if (target_altitude > 0.0) {
						target_range = target_altitude
											/ cos(DTR * (angle_az_sign * pulse->angle_az
													- head_offset_roll_deg
													- pulse->roll_offset));
						scaled_range_diff = (sounding->range - target_range) / target_range;
						amplitude_factor = exp(-4.0 * scaled_range_diff * scaled_range_diff);
					} else {
						amplitude_factor = 1.0;
					}
//fprintf(stderr,"amplitude_max:%d amplitude_threshold:%f amplitude_factor:%f\n",
//amplitude_max, amplitude_threshold, amplitude_factor);

					/* set beamflag */
					if (sounding->amplitude * amplitude_factor >= amplitude_threshold)
						sounding->beamflag = MB_FLAG_FLAG + MB_FLAG_SONAR;
					else
						sounding->beamflag = MB_FLAG_NULL;

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


					/* check for largest amplitude */
					if (sounding->amplitude > amplitude_largest) {
						amplitude_largest = sounding->amplitude;
						isounding_largest = isounding;
					}
				}
				else {
					/* null everything */
					sounding->beamflag = MB_FLAG_NULL;
					sounding->depth = 0.0;
					sounding->acrosstrack = 0.0;
					sounding->alongtrack = 0.0;
				}
			}

			/* reset beam flags */
			if (isounding_largest >= 0) {
				sounding = &pulse->soundings[isounding_largest];
				if (sounding->beamflag != MB_FLAG_NULL) {
					sounding->beamflag = MB_FLAG_NONE;
//fprintf(stderr," ipulse:%d isounding:%d angle_az_sign:%f xyz: %f %f %f\n",
//ipulse,isounding,angle_az_sign,sounding->acrosstrack,sounding->alongtrack,sounding->depth);
				}
			}
		}

		/* set flag indicating that bathymetry has been calculated */
		store->bathymetry_calculated = MB_YES;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;

} /* mbsys_3ddwissl_calculatebathymetry */

/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_wissl_indextable_compare1(const void *a, const void *b) {
    struct mb_io_indextable_struct *aa;
    struct mb_io_indextable_struct *bb;
    int result = 0;

    aa = (struct mb_io_indextable_struct*) a;
    bb = (struct mb_io_indextable_struct*) b;

    if (aa->subsensor < bb->subsensor) {
        result = -1;
    }
    else if (aa->subsensor > bb->subsensor) {
        result = 1;
    }
    else if (aa->file_index < bb->file_index) {
        result = -1;
    }
    else if (aa->file_index > bb->file_index) {
        result = 1;
    }
    else if (aa->subsensor_index < bb->subsensor_index) {
        result = -1;
    }
    else if (aa->subsensor_index > bb->subsensor_index) {
        result = 1;
    }

    return(result);
}

/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_wissl_indextable_compare2(const void *a, const void *b) {
    struct mb_io_indextable_struct *aa;
    struct mb_io_indextable_struct *bb;
    int result = 0;

    aa = (struct mb_io_indextable_struct*) a;
    bb = (struct mb_io_indextable_struct*) b;
    if (aa->time_d_corrected < bb->time_d_corrected)
        result = -1;
    else if (aa->time_d_corrected > bb->time_d_corrected)
        result = 1;
    return(result);
}

/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_indextablefix(int verbose, void *mbio_ptr, int num_indextable,
                                 void *indextable_ptr, int *error) {
	int status = MB_SUCCESS;
    struct mb_io_indextable_struct *indextable;
	struct mbsys_3ddwissl_struct *store;
    double dt, dt_threshold, nearest_minute_time_d;
    int head_a_start, head_a_end, head_b_start, head_b_end;
    int first_good_timestamp, next_good_timestamp, last_good_timestamp;
    int num_good_timestamps;
    int done;
    int i;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:               %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:              %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       num_indextable:        %d\n", num_indextable);
		fprintf(stderr, "dbg2       indextable_ptr:        %p\n", indextable_ptr);
	}

	/* always successful */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_3ddwissl_struct *)mb_io_ptr->store_data;

	/* get index table structure pointer */
	indextable = (struct mb_io_indextable_struct *)indextable_ptr;

    /* resort the total index table so that the data records are sorted by
     * left/right head, then file, then original order */
/*fprintf(stderr,"\nOriginal Index Table:\n");
for (i=0;i<num_indextable;i++) {
double nearest_minute_time_d = 60.0 * round(indextable[i].time_d_org / 60.0);
fprintf(stderr,"%4.4d %4.4d %4.4d %4.4d %d %4.4d %15.6f %15.6f %15.6f %5ld %lu %d %d\n",
i, indextable[i].file_index, indextable[i].total_index_org, indextable[i].total_index_sorted,
indextable[i].subsensor, indextable[i].subsensor_index,
indextable[i].time_d_org, indextable[i].time_d_corrected, nearest_minute_time_d,
indextable[i].offset, indextable[i].size, indextable[i].kind,
indextable[i].read);
}*/
    qsort((void *)indextable, num_indextable,
              sizeof(struct mb_io_indextable_struct),
              (void *)mbsys_3ddwissl_wissl_indextable_compare1);
    for (i=0;i<num_indextable;i++) {
        indextable[i].total_index_sorted = i;
    }
/*
fprintf(stderr,"\nSorted Index Table:\n");
dt = 0.0;
for (i=0;i<num_indextable;i++) {
if (i > 0 && indextable[i].kind == MB_DATA_DATA && indextable[i].subsensor == indextable[i-1].subsensor) {
dt = indextable[i].time_d_org - indextable[i-1].time_d_org;
}
else {
dt = 0.0;
}
double nearest_minute_time_d = 60.0 * round(indextable[i].time_d_org / 60.0);
fprintf(stderr,"%4.4d %4.4d %4.4d %4.4d %d %4.4d %15.6f %15.6f %15.6f %5ld %lu %d %d   %10.6f",
i, indextable[i].file_index, indextable[i].total_index_org, indextable[i].total_index_sorted,
indextable[i].subsensor, indextable[i].subsensor_index,
indextable[i].time_d_org, indextable[i].time_d_corrected, nearest_minute_time_d,
indextable[i].offset, indextable[i].size, indextable[i].kind,
indextable[i].read, dt);
if (fabs(dt) > 0.05) fprintf(stderr, " *****");
if (indextable[i].time_d_org - nearest_minute_time_d >= 0.0
     && indextable[i].time_d_org - nearest_minute_time_d < 0.01)
    fprintf(stderr, " $$$$$");
fprintf(stderr, "\n");
}
*/

    /* Correct the WiSSL timestamps produced by the first version of the sensor.
     * The timestamps need correcting because the clock drifts between being reset t
     * to a correct value at each even minute. Therefore look for large time offsets
     * immediately after the minute mark, and treat the post shift timestamp as correct.
     * Interpolate between the post-minute timestamps to correct all the other timestamps.
     * Note that large positive clock drifts lead to two timestamps at the minute mark,
     * one incorrect and the second correct, while negative clock drifts lead to a
     * gap in time prior to the minute mark. Also, some datasets have little, if any,
     * clock drift even though the problem exists.
     */

    /* calculate the approximate expected time between scan timestamps */
    dt_threshold = 2.30 * store->pulses_per_scan / MBSYS_3DDWISSL_LASERPULSERATE;

    /* find the index bounds of sorted data from the two WiSSL optical heads */
    head_a_start = num_indextable;
    head_a_end = -1;
    head_b_start = num_indextable;
    head_b_end = -1;
    for (int i = 0; i < num_indextable; i++) {
        if (indextable[i].subsensor == MBSYS_3DDWISSL_HEADA) {
            if (i < head_a_start)
                head_a_start = i;
            if (i > head_a_end)
                head_a_end = i;
        }
        else if (indextable[i].subsensor == MBSYS_3DDWISSL_HEADB) {
            if (i < head_b_start)
                head_b_start = i;
            if (i > head_b_end)
                head_b_end = i;
        }
    }
//fprintf(stderr,"head_a_start:%d head_a_end:%d\n", head_a_start, head_a_end);
//fprintf(stderr,"head_b_start:%d head_b_end:%d\n\n", head_b_start, head_b_end);

    /* deal with head A data - start by identifying all good timestamps */
    dt = 0.0;
    num_good_timestamps = 0;
    first_good_timestamp = head_a_end;
    last_good_timestamp = head_a_start;
    for (int i = head_a_start; i <= head_a_end; i++) {
        if (i > head_a_start)
            dt = indextable[i].time_d_org - indextable[i-1].time_d_org;
        nearest_minute_time_d = 60.0 * round(indextable[i].time_d_org / 60.0);
        if (indextable[i].time_d_org - nearest_minute_time_d >= 0.0
            && indextable[i].time_d_org - nearest_minute_time_d < dt_threshold
            && fabs(dt) > dt_threshold) {
//fprintf(stderr,"Good timestamp A: %d %.6f\n",i,indextable[i].time_d_org);
            indextable[i].time_d_corrected = indextable[i].time_d_org;
            num_good_timestamps++;
            if (first_good_timestamp > i)
                first_good_timestamp = i;
            last_good_timestamp = i;
        }
        else
            indextable[i].time_d_corrected = 0.0;
    }
//fprintf(stderr,"\nHead A: dt_threshold:%f num_good_timestamps:%d first_good_timestamp:%d last_good_timestamp:%d\n\n",
//dt_threshold, num_good_timestamps,first_good_timestamp,last_good_timestamp);

    /* if good timestamps found then extrapolate and interpolate the other timestamps */
    if (last_good_timestamp > first_good_timestamp) {
        dt = (indextable[last_good_timestamp].time_d_corrected - indextable[first_good_timestamp].time_d_corrected)
            / ((double)(last_good_timestamp - first_good_timestamp));
//fprintf(stderr,"dt:%.6f\n",dt);
        for (int i = head_a_start; i < first_good_timestamp; i++) {
            indextable[i].time_d_corrected = indextable[first_good_timestamp].time_d_corrected
                                                + dt * (i - first_good_timestamp);
//fprintf(stderr,"New time_d A-0: %5d %.6f\n", i, indextable[i].time_d_corrected);
        }
        for (int i = last_good_timestamp + 1; i <= head_a_end; i++) {
            indextable[i].time_d_corrected = indextable[last_good_timestamp].time_d_corrected
                                                + dt * (i - last_good_timestamp);
//fprintf(stderr,"New time_d A-2: %5d %.6f\n", i, indextable[i].time_d_corrected);
        }
        done = MB_NO;
        next_good_timestamp = first_good_timestamp;
        while (done == MB_NO) {
            for (int i = first_good_timestamp + 1;
                 i <= last_good_timestamp && next_good_timestamp == first_good_timestamp;
                 i++) {
                if (indextable[i].time_d_corrected > 0.0) {
                    next_good_timestamp = i;
                }
            }
            dt = (indextable[next_good_timestamp].time_d_corrected - indextable[first_good_timestamp].time_d_corrected)
            / ((double)(next_good_timestamp - first_good_timestamp));
            for (int i = first_good_timestamp + 1; i < next_good_timestamp; i++) {
                indextable[i].time_d_corrected = indextable[first_good_timestamp].time_d_corrected
                                                + dt * (i - first_good_timestamp);
//fprintf(stderr,"New time_d A-1: %5d %.6f\n", i, indextable[i].time_d_corrected);
            }
            first_good_timestamp = next_good_timestamp;
            if (next_good_timestamp == last_good_timestamp)
                done = MB_YES;
        }
    }

    /* if no good timestamps identified assume all are good and reset all
        timestamps to the original values */
    else {
        for (int i = head_a_start; i <= head_a_end; i++) {
            indextable[i].time_d_corrected = indextable[i].time_d_org;
        }
    }

    /* deal with head B data - start by identifying all good timestamps */
    dt = 0.0;
    num_good_timestamps = 0;
    first_good_timestamp = head_b_end;
    last_good_timestamp = head_b_start;
    for (int i = head_b_start; i <= head_b_end; i++) {
        if (i > head_b_start)
            dt = indextable[i].time_d_org - indextable[i-1].time_d_org;
        nearest_minute_time_d = 60.0 * round(indextable[i].time_d_org / 60.0);
        if (indextable[i].time_d_org - nearest_minute_time_d >= 0.0
            && indextable[i].time_d_org - nearest_minute_time_d < dt_threshold
            && fabs(dt) > dt_threshold) {
//fprintf(stderr,"Good timestamp B: %d %.6f\n",i,indextable[i].time_d_org);
            indextable[i].time_d_corrected = indextable[i].time_d_org;
            num_good_timestamps++;
            if (first_good_timestamp > i)
                first_good_timestamp = i;
            last_good_timestamp = i;
        }
        else
            indextable[i].time_d_corrected = 0.0;
    }
//fprintf(stderr,"\nHead B: dt_threshold:%f num_good_timestamps:%d first_good_timestamp:%d last_good_timestamp:%d\n",
//dt_threshold, num_good_timestamps,first_good_timestamp,last_good_timestamp);

    /* if good timestamps found then extrapolate and interpolate the other timestamps */
    if (last_good_timestamp > first_good_timestamp) {
        dt = (indextable[last_good_timestamp].time_d_corrected - indextable[first_good_timestamp].time_d_corrected)
            / ((double)(last_good_timestamp - first_good_timestamp));
        for (int i = head_b_start; i < first_good_timestamp; i++) {
            indextable[i].time_d_corrected = indextable[first_good_timestamp].time_d_corrected
                                                + dt * (i - first_good_timestamp);
//fprintf(stderr,"New time_d B-0: %5d %.6f\n", i, indextable[i].time_d_corrected);
        }
        for (int i = last_good_timestamp + 1; i <= head_b_end; i++) {
            indextable[i].time_d_corrected = indextable[last_good_timestamp].time_d_corrected
                                                + dt * (i - last_good_timestamp);
//fprintf(stderr,"New time_d B-2: %5d %.6f\n", i, indextable[i].time_d_corrected);
        }
        done = MB_NO;
        next_good_timestamp = first_good_timestamp;
        while (done == MB_NO) {
            for (int i = first_good_timestamp + 1;
                 i <= last_good_timestamp && next_good_timestamp == first_good_timestamp;
                 i++) {
                if (indextable[i].time_d_corrected > 0.0) {
                    next_good_timestamp = i;
                }
            }
            dt = (indextable[next_good_timestamp].time_d_corrected - indextable[first_good_timestamp].time_d_corrected)
            / ((double)(next_good_timestamp - first_good_timestamp));
            for (int i = first_good_timestamp + 1; i < next_good_timestamp; i++) {
                indextable[i].time_d_corrected = indextable[first_good_timestamp].time_d_corrected
                                                + dt * (i - first_good_timestamp);
//fprintf(stderr,"New time_d B-1: %5d %.6f\n", i, indextable[i].time_d_corrected);
            }
            first_good_timestamp = next_good_timestamp;
            if (next_good_timestamp == last_good_timestamp)
                done = MB_YES;
        }
    }

    /* if no good timestamps identified assume all are good and reset all
        timestamps to the original values */
    else {
        for (int i = head_b_start; i <= head_b_end; i++) {
            indextable[i].time_d_corrected = indextable[i].time_d_org;
        }
    }

/*fprintf(stderr,"\nCorrected Timestamp Index Table:\n");
dt = 0.0;
for (i=0;i<num_indextable;i++) {
if (i > 0 && indextable[i].kind == MB_DATA_DATA && indextable[i].subsensor == indextable[i-1].subsensor) {
dt = indextable[i].time_d_corrected - indextable[i-1].time_d_corrected;
}
else {
dt = 0.0;
}
double nearest_minute_time_d = 60.0 * round(indextable[i].time_d_org / 60.0);
fprintf(stderr,"FIX: %4.4d %4.4d %4.4d %4.4d %d %4.4d %15.6f %15.6f %15.6f %5ld %lu %d %d   %10.6f",
i, indextable[i].file_index, indextable[i].total_index_org, indextable[i].total_index_sorted,
indextable[i].subsensor, indextable[i].subsensor_index,
indextable[i].time_d_org, indextable[i].time_d_corrected, nearest_minute_time_d,
indextable[i].offset, indextable[i].size, indextable[i].kind,
indextable[i].read, dt);
if (fabs(dt) > 0.05) fprintf(stderr, " *****");
if (indextable[i].time_d_corrected - nearest_minute_time_d >= 0.0
     && indextable[i].time_d_corrected - nearest_minute_time_d < dt_threshold)
    fprintf(stderr, " $$$$$");
fprintf(stderr, "\n");
}*/

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;

} /* mbsys_3ddwissl_fixwissltimestamps */

/*--------------------------------------------------------------------*/
int mbsys_3ddwissl_indextableapply(int verbose, void *mbio_ptr, int num_indextable,
                                   void *indextable_ptr, int n_file, int *error) {
	int status = MB_SUCCESS;
        struct mb_io_indextable_struct *indextable;
        int giindex, iindex;
        int giindex_a_begin, giindex_a_end;
        int giindex_b_begin, giindex_b_end;
        int i;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:               %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:              %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       num_indextable:        %d\n", num_indextable);
		fprintf(stderr, "dbg2       indextable_ptr:        %p\n", indextable_ptr);
		fprintf(stderr, "dbg2       n_file:                %d\n", n_file);
	}

	/* check for non-null data */
	assert(mbio_ptr != NULL);

	/* always successful */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get index table structure pointer */
	indextable = (struct mb_io_indextable_struct *)indextable_ptr;

/*fprintf(stderr,"\nExternal Index Table:\n");
for (i=0;i<num_indextable;i++) {
nearest_minute_time_d = 60.0 * round(indextable[i].time_d_org / 60.0);
fprintf(stderr,"EXT %4.4d %4.4d %4.4d %4.4d %d %4.4d %15.6f %15.6f %15.6f %5ld %lu %d %d\n",
i, indextable[i].file_index, indextable[i].total_index_org, indextable[i].total_index_sorted,
indextable[i].subsensor, indextable[i].subsensor_index,
indextable[i].time_d_org, indextable[i].time_d_corrected, nearest_minute_time_d,
indextable[i].offset, indextable[i].size, indextable[i].kind,
mb_io_ptr->indextable[i].read);
}*//*

fprintf(stderr,"\nOriginal internal Index Table:\n");
for (i=0;i<mb_io_ptr->num_indextable;i++) {
nearest_minute_time_d = 60.0 * round(mb_io_ptr->indextable[i].time_d_org / 60.0);
fprintf(stderr,"INT %4.4d %4.4d %4.4d %4.4d %d %4.4d %15.6f %15.6f %15.6f %5ld %lu %d %d\n",
i, mb_io_ptr->indextable[i].file_index, mb_io_ptr->indextable[i].total_index_org, mb_io_ptr->indextable[i].total_index_sorted,
mb_io_ptr->indextable[i].subsensor, mb_io_ptr->indextable[i].subsensor_index,
mb_io_ptr->indextable[i].time_d_org, mb_io_ptr->indextable[i].time_d_corrected, nearest_minute_time_d,
mb_io_ptr->indextable[i].offset, mb_io_ptr->indextable[i].size, mb_io_ptr->indextable[i].kind,
mb_io_ptr->indextable[i].read);
}*/

    /* correct timestamps in the file's internal index table using information
     * supplied in the external index table */

    /* find the start and end of relevant entries in the global index table */
    giindex_a_begin = -1;
    giindex_a_end = -1;
    giindex_b_begin = -1;
    giindex_b_end = -1;
    for (giindex = 0; giindex < num_indextable; giindex++) {
        if (indextable[giindex].file_index == n_file) {
            if (indextable[giindex].subsensor == MBSYS_3DDWISSL_HEADA) {
                if (giindex_a_begin < 0)
                    giindex_a_begin = giindex;
                giindex_a_end = giindex;
            }
            else if (indextable[giindex].subsensor == MBSYS_3DDWISSL_HEADB) {
                if (giindex_b_begin < 0)
                    giindex_b_begin = giindex;
                giindex_b_end = giindex;
            }
        }
    }

    /* replace timestamps with corrected values from the global index table */
    for (iindex = 0; iindex < mb_io_ptr->num_indextable; iindex++) {
        if (mb_io_ptr->indextable[iindex].subsensor == MBSYS_3DDWISSL_HEADA) {
            for (giindex = giindex_a_begin; giindex <= giindex_a_end; giindex++) {
                if (mb_io_ptr->indextable[iindex].subsensor_index == indextable[giindex].subsensor_index) {
/*fprintf(stderr,"A Replacing iindex:%d giindex:%d  file: %d %d subsensor: %d:%d %d:%d  time_d: %.6f %.6f %.6f\n",
iindex, giindex, n_file, indextable[giindex].file_index,
mb_io_ptr->indextable[iindex].subsensor, mb_io_ptr->indextable[iindex].subsensor_index,
indextable[giindex].subsensor, indextable[giindex].subsensor_index,
mb_io_ptr->indextable[iindex].time_d_org,
indextable[giindex].time_d_org,
indextable[giindex].time_d_corrected);*/
                    mb_io_ptr->indextable[iindex].time_d_corrected = indextable[giindex].time_d_corrected;
                }

            }
        }
        else if (mb_io_ptr->indextable[iindex].subsensor == MBSYS_3DDWISSL_HEADB) {
            for (giindex = giindex_b_begin; giindex <= giindex_b_end; giindex++) {
                if (mb_io_ptr->indextable[iindex].subsensor_index == indextable[giindex].subsensor_index) {
/*fprintf(stderr,"B Replacing iindex:%d giindex:%d  file: %d %d subsensor: %d:%d %d:%d  time_d: %.6f %.6f %.6f\n",
iindex, giindex, n_file, indextable[giindex].file_index,
mb_io_ptr->indextable[iindex].subsensor, mb_io_ptr->indextable[iindex].subsensor_index,
indextable[giindex].subsensor, indextable[giindex].subsensor_index,
mb_io_ptr->indextable[iindex].time_d_org,
indextable[giindex].time_d_org,
indextable[giindex].time_d_corrected);*/
                    mb_io_ptr->indextable[iindex].time_d_corrected = indextable[giindex].time_d_corrected;
                }

            }
        }
    }

    /* resort the file's index table using the new timestamps */
    qsort((void *)mb_io_ptr->indextable, mb_io_ptr->num_indextable,
              sizeof(struct mb_io_indextable_struct),
              (void *)mbsys_3ddwissl_wissl_indextable_compare2);
    for (i=0;i<mb_io_ptr->num_indextable;i++) {
        mb_io_ptr->indextable[i].total_index_sorted = i;
    }
/*fprintf(stderr,"\nCorrected and resorted Index Table:\n");
for (i=0;i<mb_io_ptr->num_indextable;i++) {
nearest_minute_time_d = 60.0 * round(mb_io_ptr->indextable[i].time_d_org / 60.0);
fprintf(stderr,"%4.4d %4.4d %4.4d %4.4d %d %4.4d %15.6f %15.6f %15.6f %5ld %lu %d %d\n",
i, mb_io_ptr->indextable[i].file_index, mb_io_ptr->indextable[i].total_index_org, mb_io_ptr->indextable[i].total_index_sorted,
mb_io_ptr->indextable[i].subsensor, mb_io_ptr->indextable[i].subsensor_index,
mb_io_ptr->indextable[i].time_d_org, mb_io_ptr->indextable[i].time_d_corrected, nearest_minute_time_d,
mb_io_ptr->indextable[i].offset, mb_io_ptr->indextable[i].size, mb_io_ptr->indextable[i].kind,
mb_io_ptr->indextable[i].read);
}*/

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;

} /* mbsys_3ddwissl_applywissltimestamps */

/*--------------------------------------------------------------------*/
