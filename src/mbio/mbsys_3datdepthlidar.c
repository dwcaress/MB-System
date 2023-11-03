/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_3datdepthlidar.c	3.00	5/7/2013
 *
 *    Copyright (c) 2013-2023 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * Notes on the MBSYS_3DATDEPTHLIDAR data structure:
 *
 * Vendor processed format from 3DatDepth, produced from raw LIDAR
 * time series files by proprietary 3DatDepth software
 *
 *--------------------------------------------------------------------------------
 * Processing Tool Data Output
 *
 * The 3D at Depth Processing Tool reads binary sensor data and
 * processes it to provide corresponding range angle data, saved
 * to a binary file. A timestamp is provided for each scan recorded
 * in the file. This timestamp is the time of the first laser pulse
 * for that scan. The μsec time value listed for each laser pulse
 * is the relative time between each successive pulse, offset from
 * the timestamp. The value for the first pulse should be subtracted
 * from all pulses in that scan (including itself), such that the
 * first pulse would be 0 μsec accordingly.
 *
 * ---------------------------------------------------------------------------------------
 * Range Angle Angle data format (binary)
 *              Item	                                Value	            Bytes
 * ---------------------------------------------------------------------------------------
 * File Header Record
 *           File header record id	                0x3D46	            2   (1 UINT16)
 *           File magic number	                        0x3D07	            2   (1 UINT16)
 *           File version	                        1	            2   (1 UINT16)
 *           File sub version	                        1	            2   (1 UINT16)
 * Scan Information
 *           Scan type (AZ raster, AZEL raster, bowtie)	2, 3, 4             2   (1 UINT16)
 *           Cross track angle start (deg)		                    4   (1 float32)
 *           Cross track angle end (deg)		                    4   (1 float32)
 *           Forward track angle start (deg)		                    4   (1 float32)
 *           Forward track angle end (deg)		                    4   (1 float32)
 *           Counts per Scan (AZ raster and bowtie)		            2   (1 UINT16)
 *           Counts per cross track (AZEL raster)		            2   (1 UINT16)
 *           Counts per forward track (AZEL raster)		            2   (1 UINT16)
 *           Scanner Efficiency		                                    2   (1 UINT16)
 *           Scans per File		                                    2   (1 UINT16)
 *           Scan count		                                            4   (1 UINT32)
 *
 * ---------------------------------------------------------------------------------------
 * Lidar Scan Record
 *           Lidar scan record id	                0x3D52	            2   (1 UINT16)
 * First Pulse Timestamp ( 1 to n Scans )
 *           Timestamp year		                                    2   (1 UINT16)
 *           Timestamp month		                                    1   (1 UINT8)
 *           Timestamp day		                                    1   (1 UINT8)
 *           Timestamp days since Jan 1		                            2   (1 UINT16)
 *           Timestamp hour		                                    2   (1 UINT16)
 *           Timestamp minutes		                                    1   (1 UINT8)
 *           Timestamp seconds		                                    1   (1 UINT8)
 *           Timestamp nano seconds		                            4   (1 UINT32)
 * Laser Pulse Data ( 1 to m pulses per scan )
 *           Range ( from glass front ) meters                              4   (1 float32)
 *           Amplitude / peak of signal	                         	    2   (1 short int)
 *           SNR of signal return		                            4   (1 float32)
 *           Cross track angle (deg)		                            4   (1 float32)
 *           Forward track angle (deg)		                            4   (1 float32)
 *           Cross track offset (m)		                            4   (1 float32)
 *           Forward track offset (m)		                            4   (1 float32)
 *           Pulse time offset (µsec)		                            4   (1 UINT32)
 *           Saturated (0/1)    		                            1   (1 UINT8)
 *
 * For each scan per file, a “First Pulse Timestamp” and “m” sets of
 * “Laser Pulse Data” will exist.  For example, for a Continuous Scan
 * file with 500 scans per file, and 200 pts per scan, the following
 * data would be present:
 * 	File Header
 * 	Scan Information
 *      (1) First Pulse Timestamp
 * 		200	 Laser Pulse Data sets
 *      (2) First Pulse Timestamp
 *              200	Laser Pulse Data sets
 * 		…
 *      (500) First Pulse Timestamp
 * 	        200	 Laser Pulse Data sets
 *
 * A Bowtie scan file would be the same as above.  A Full scan file
 * will only contain one scan of data but with “Counts per Cross track”
 * multiplied by “Counts per Forward track” sets of laser pulse data.
 *
 * The timestamp is the time of the first laser pulse for that scan.
 * The pulse time offset (µsec) value listed for each laser pulse is
 * the relative time between each successive pulse.  The value for the
 * first pulse should be subtracted from all pulses in that scan, such
 * that the first pulse would be 0 µsec accordingly.  The Pt Valid term
 * is provided to specify range validity.  If “valid” is given as 0, a
 * range could not be determined for that laser pulse, or it fell
 * outside of the processing limits.
 *
 * Note, if processing is interrupted by hitting the “Cancel Processing”
 * button, an indeterminate number of records may exist in the current
 * processing file.  All processed files will be written to the same
 * folder location of the raw file.  The same file name is used for
 * processed files, but with a “.csv” or “.bin” file extension.  If data
 * is reprocessed in the same folder location, previously processed data
 * files will be overwritten.
 *
 *--------------------------------------------------------------------------------
 * INS Data Format
 *
 * INU serial packets are archived to data files located at %RAW_DATA_PATH%/INU/.
 * %RAW_DATA_PATH% is set in the sensor configuration file. Data files are
 * named INU_MMDDYYYY_HHMMSS.bin. 3000 navigation messages are stored per
 * each archived data file, representing approximately 5 minutes data storage
 * if collecting messages at 10 Hz. The binary data file storage is provided
 * below, for the first record. Additional records follow to 3000 per file.
 * The latency from first character received to the timestamp is approximately
 * 100 msec. If an error occurs, or the first byte read from the serial port is
 * not equal to the INU magic character, the software will begin reading byte
 * by byte in an attempt to resync.
 *              Item                                Value                   Bytes
 *           Header Magic Number                    0xE32F                  2 (1 UINT16)
 *           Timestamp year                                                 2 (1 UINT16)
 *           Timestamp month                                                1 (1 UINT8)
 *           Timestamp day                                                  1 (1 UINT8)
 *           Timestamp days since Jan 1                                     2 (1 UINT16)
 *           Timestamp hour                                                 2 (1 UINT16)
 *           Timestamp minutes                                              1 (1 UINT8)
 *           Timestamp seconds                                              1 (1 UINT8)
 *           Timestamp nano seconds                                         4 (1 UINT32)
 *           Latency (μseconds) – latency from first                        4 (1 float32)
 *                                 character received to timestamp
 *           INU Nav message 1
 *                      complete message as received from INU               Len / packet ‐ reference the INU specification
 *                      1 magic, 1 ID, 1 data len, N NAV bytes
 *
 *--------------------------------------------------------------------------------
 * CTD Ethernet Interface
 *
 * The 3D at Depth laser sensor listens for CTD data packets available on port
 * 2003 of the static IP address for the sensor. The sensor reads and timestamps
 * these packets and saves the data to a binary file, located at %RAW_DATA_PATH%/CTD/.
 * %RAW_DATA_PATH% is set in the sensor configuration file.
 * Data files are named CTD_MMDDYYYY_HHMMSS.bin. The file format is as follows.
 * CTD Packets
 *              Item                                Value                   Bytes
 *           Header Magic Number                    0x3D07                  2 (1 UINT16)
 *           Timestamp year                                                 2 (1 UINT16)
 *           Timestamp month                                                1 (1 UINT8)
 *           Timestamp day                                                  1 (1 UINT8)
 *           Timestamp days since Jan 1                                     2 (1 UINT16)
 *           Timestamp hour                                                 2 (1 UINT16)
 *           Timestamp minutes                                              1 (1 UINT8)
 *           Timestamp seconds                                              1 (1 UINT8)
 *           Timestamp nano seconds                                         4 (1 UINT32)
 *           Header project code                    0x3D03                  2 (1 UINT16)
 *           Header version                         1                       1 (1 UINT8)
 *           Water Temperature C                                            8 (1 double)
 *           Water Salinity psu                                             8 (1 double)
 *           Water Pressure dbar                                            8 (1 double)
 *--------------------------------------------------------------------------------
 *
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mbsys_3datdepthlidar.h"

#define MBF_3DDEPTHP_DEBUG 1


/*-------------------------------------------------------------------- */
int mbsys_3datdepthlidar_alloc(int verbose,      /* in: verbosity level set on command line 0..N */
                               void *mbio_ptr,   /* in: see mb_io.h:/^struct mb_io_struct/ */
                               void **store_ptr, /* in: see mbsys_3datdepthlidar.h:/^struct mbsys_3datdepthlidar_struct/ */
                               int *error        /* out: see mb_status.h:/MB_ERROR/ */
                               ) {
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
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_3datdepthlidar_struct), (void **)store_ptr, error);
	mb_io_ptr->structure_size = 0;

	/* get data structure pointer */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)*store_ptr;

	/* initialize everything */
	store->kind = MB_DATA_NONE; /* MB-System record ID */

	/* File Header */
	store->file_version = 1; /* 1 */
	store->sub_version = 0;  /* 0 */

	/* Scan Information */
	store->scan_type = 0x0101;              /* continuous: 0x0101, bowtie: 0x0102, full: 0x0103 */
	store->cross_track_angle_start = 0.0;   /* degrees */
	store->cross_track_angle_end = 0.0;     /* degrees */
	store->forward_track_angle_start = 0.0; /* degrees */
	store->forward_track_angle_end = 0.0;   /* degrees */
	store->counts_per_scan = 0;             /* AZ raster and bowtie*/
	store->counts_per_cross_track = 0;      /* AZEL raster */
	store->counts_per_forward_track = 0;    /* AZEL raster */
	store->scanner_efficiency = 0;          /* */
	store->scans_per_file = 0;              /* */
	store->scan_count = 0;

	/* Id of most recently read record */
	store->record_id = 0;

	/* Laser Scan Data (1 to scans_per_file Scans) */
	store->current_scan = 0;

	/* First Pulse Timestamp (1 to n Scans) */
	store->year = 0;
	store->month = 0;
	store->day = 0;
	store->days_since_jan_1 = 0;
	store->hour = 0;
	store->minutes = 0;
	store->seconds = 0;
	store->nanoseconds = 0;

	/* position and attitude per first pulse */
	store->time_d = 0.0;
	store->navlon = 0.0;
	store->navlat = 0.0;
	store->sensordepth = 0.0;
	store->heading = 0.0;
	store->roll = 0.0;
	store->pitch = 0.0;
	store->speed = 0.0;

	/* Laser Scan Data (1 to m pulses per scan) */
	store->num_pulses = 0;
	store->num_pulses_alloc = 0;
	store->pulses = NULL;

	/* comment */
	store->comment_len = 0;
	store->comment[0] = 0; /* comment string */

	/* position data */
	store->pos_time_d = 0.0;
	store->pos_longitude = 0.0;
	store->pos_latitude = 0.0;

	/* attitude data */
	store->att_time_d = 0.0;
	store->att_roll = 0.0;
	store->att_pitch = 0.0;
	store->att_heave = 0.0;

	/* heading data */
	store->hdg_time_d = 0.0;
	store->hdg_heading = 0.0;

	/* sensordepth data */
	store->sdp_time_d = 0.0;
	store->sdp_sensordepth = 0.0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       store_ptr:  %p\n", *store_ptr);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
} /* mbsys_3datdepthlidar_alloc */
/*----------------------------------------------------------------------*/
int mbsys_3datdepthlidar_deall(int verbose,      /* in: verbosity level set on command line 0..N */
                               void *mbio_ptr,   /* in: see mb_io.h:/^struct mb_io_struct/ */
                               void **store_ptr, /* in: see mbsys_3datdepthlidar.h:/^struct mbsys_3datdepthlidar_struct/ */
                               int *error        /* out: see mb_status.h:/error values/ */
                               ) {
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
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)*store_ptr;

	int status = MB_SUCCESS;

	/* deallocate pulses */
	if (store->pulses != NULL) {
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&store->pulses), error);
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
} /* mbsys_3datdepthlidar_deall */
/*----------------------------------------------------------------------*/
int mbsys_3datdepthlidar_dimensions(int verbose, void *mbio_ptr, /* in: verbosity level set on command line 0..N */
                                    void *store_ptr, /* in: see mbsys_3datdepthlidar.h:/^struct mbsys_3datdepthlidar_struct/ */
                                    int *kind,       /* in: see mb_status.h:0+/MBIO data type/ */
                                    int *nbath,      /* out: number of bathymetric samples 0..MBSYS_SWPLS_MAX_BEAMS */
                                    int *namp,       /* out: number of amplitude samples 0..MBSYS_SWPLS_MAX_BEAMS */
                                    int *nss,        /* out: number of sidescan samples 0..MBSYS_SWPLS_MAX_BEAMS */
                                    int *error       /* out: see mb_status.h:/error values/ */
                                    ) {
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract beam and pixel numbers from structure */
	if (*kind == MB_DATA_DATA) {
		if (store->counts_per_scan > 0) {
			*nbath = store->counts_per_scan;
		}
		else {
			*nbath = store->counts_per_cross_track * store->counts_per_forward_track;
		}
		*namp = *nbath;
		*nss = 0;
	}
	else {
		*nbath = 0;
		*namp = 0;
		*nss = 0;
	}

	const int status = MB_SUCCESS;

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
} /* mbsys_3datdepthlidar_dimensions */
/*--------------------------------------------------------------------*/
int mbsys_3datdepthlidar_pingnumber(int verbose,     /* in: verbosity level set on command line 0..N */
                                    void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                    unsigned int *pingnumber, /* out: swathplus ping number */
                                    int *error       /* out: see mb_status.h:/MB_ERROR/ */
                                    ) {
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
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)mb_io_ptr->store_data;

	/* extract ping number from structure */
	*pingnumber = store->current_scan;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       pingnumber: %u\n", *pingnumber);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
} /* mbsys_3datdepthlidar_pingnumber */
/*--------------------------------------------------------------------*/
int mbsys_3datdepthlidar_preprocess(int verbose,     /* in: verbosity level set on command line 0..N */
                                    void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                    void *store_ptr, /* in: see mbsys_3datdepthlidar.h:/^struct mbsys_3datdepthlidar_struct/ */
                                    void *platform_ptr, void *preprocess_pars_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:                   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:                  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       platform_ptr:               %p\n", (void *)platform_ptr);
		fprintf(stderr, "dbg2       preprocess_pars_ptr:        %p\n", (void *)preprocess_pars_ptr);
	}

	*error = MB_ERROR_NO_ERROR;

	/* check for non-null data */
	assert(mbio_ptr != NULL);
	assert(preprocess_pars_ptr != NULL);

	/* get data structure pointers */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;
	// struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;
	struct mb_preprocess_struct *pars = (struct mb_preprocess_struct *)preprocess_pars_ptr;

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

	double time_d;
	int time_i[7], time_j[5];
	double heading; /* heading (degrees) */
	double roll;    /* roll (degrees) */
	double pitch;   /* pitch (degrees) */
	double speed;   /* speed (degrees) */
	int interp_error = MB_ERROR_NO_ERROR;
	int jnav = 0;
	int jsensordepth = 0;
	int jheading = 0;
	// int	jaltitude = 0;
	int jattitude = 0;
	int status = MB_SUCCESS;

  /* if called with store_ptr == NULL then called after mb_read_init() but before
      any data are read - for some formats this allows kluge options to set special
      reading conditions/behaviors */
  if (store_ptr == NULL) {

  }

	/* deal with a survey record */
	else if (store->kind == MB_DATA_DATA) {

  	/* change timestamp if indicated */
  	if (pars->timestamp_changed) {
  		store->time_d = pars->time_d;
  		mb_get_date(verbose, pars->time_d, time_i);
  		mb_get_jtime(verbose, time_i, time_j);
  		store->year = time_i[0];
  		store->month = time_i[1];
  		store->day = time_i[2];
  		store->days_since_jan_1 = time_j[1];
  		store->hour = time_i[3];
  		store->minutes = time_i[4];
  		store->seconds = time_i[5];
  		store->nanoseconds = 1000 * ((unsigned int)time_i[6]);
  	}

  	/* interpolate navigation and attitude */
  	time_d = store->time_d;
  	mb_get_date(verbose, time_d, time_i);

  	/* get nav sensordepth heading attitude values for record timestamp */
  	// int interp_status = MB_SUCCESS;
  	if (pars->n_nav > 0) {
  		/* interp_status = */ mb_linear_interp_longitude(verbose, pars->nav_time_d - 1, pars->nav_lon - 1, pars->n_nav, time_d,
  		                                           &store->navlon, &jnav, &interp_error);
  		/* interp_status = */ mb_linear_interp_latitude(verbose, pars->nav_time_d - 1, pars->nav_lat - 1, pars->n_nav, time_d,
  		                                          &store->navlat, &jnav, &interp_error);
  		/* interp_status = */ mb_linear_interp(verbose, pars->nav_time_d - 1, pars->nav_speed - 1, pars->n_nav, time_d, &speed, &jnav,
  		                                 &interp_error);
  		store->speed = (float)speed;
  	}
  	if (pars->n_sensordepth > 0) {
  		/* interp_status = */ mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
  		                                 pars->n_sensordepth, time_d, &store->sensordepth, &jsensordepth, &interp_error);
  	}
  	if (pars->n_heading > 0) {
  		/* interp_status = */ mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1, pars->n_heading,
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
  		/* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude, time_d,
  		                                 &roll, &jattitude, &interp_error);
  		roll = -roll;
  		store->roll = (float)roll;
  		/* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude, time_d,
  		                                 &pitch, &jattitude, &interp_error);
  		store->pitch = (float)pitch;
  	}

  	/* do lever arm correction */
  	if (platform_ptr != NULL) {

  		/* calculate sonar position position */
  		status =
  		    mb_platform_position(verbose, platform_ptr, pars->target_sensor, 0, store->navlon, store->navlat, store->sensordepth,
  		                         heading, roll, pitch, &store->navlon, &store->navlat, &store->sensordepth, error);

  		/* calculate sonar attitude */
  		status = mb_platform_orientation_target(verbose, platform_ptr, pars->target_sensor, 0, heading, roll, pitch, &heading,
  		                                        &roll, &pitch, error);
  		store->heading = (float)heading;
  		store->roll = (float)roll;
  		store->pitch = (float)pitch;
  	}

  	/* loop over all pulses */
  	for (int i = 0; i < store->num_pulses; i++) {
  		/* get pulse */
  		struct mbsys_3datdepthlidar_pulse_struct *pulse = (struct mbsys_3datdepthlidar_pulse_struct *)&store->pulses[i];

  		/* set time */
  		pulse->time_d = store->time_d + 0.000001 * pulse->pulse_time_offset;

  		/* get nav sensordepth heading attitude values for record timestamp */
  		if (pars->n_nav > 0) {
  			/* interp_status = */ mb_linear_interp_longitude(verbose, pars->nav_time_d - 1, pars->nav_lon - 1, pars->n_nav,
  			                                           pulse->time_d, &pulse->navlon, &jnav, &interp_error);
  			/* interp_status = */ mb_linear_interp_latitude(verbose, pars->nav_time_d - 1, pars->nav_lat - 1, pars->n_nav,
  			                                          pulse->time_d, &pulse->navlat, &jnav, &interp_error);
  			// interp_status = mb_linear_interp(verbose,
  			//			pars->nav_time_d-1, pars->nav_speed-1, pars->n_nav,
  			//			pulse->time_d, &pulse->speed, &jnav,
  			//			&interp_error);
  		}
  		if (pars->n_sensordepth > 0) {
  			/* interp_status = */
  			    mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1, pars->n_sensordepth,
  			                     pulse->time_d, &pulse->sensordepth, &jsensordepth, &interp_error);
  		}
  		if (pars->n_heading > 0) {
  			/* interp_status = */ mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
  			                                         pars->n_heading, pulse->time_d, &heading, &jheading, &interp_error);
  			pulse->heading = (float)heading;
  		}
  		// if (pars->n_altitude > 0)
  		//	{
  		//	interp_status = mb_linear_interp(verbose,
  		//				pars->altitude_time_d-1, pars->altitude_altitude-1, pars->n_altitude,
  		//				pulse->time_d, &pulse->altitude, &jaltitude,
  		//				&interp_error);
  		//	}
  		if (pars->n_attitude > 0) {
  			/* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
  			                                 pulse->time_d, &roll, &jattitude, &interp_error);
  			roll = -roll;
  			pulse->roll = (float)roll;
  			/* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
  			                                 pulse->time_d, &pitch, &jattitude, &interp_error);
  			pulse->pitch = (float)pitch;
  			// interp_status = mb_linear_interp(verbose,
  			//			pars->attitude_time_d-1, pars->attitude_heave-1, pars->n_attitude,
  			//			pulse->time_d, &store->heave, &jattitude,
  			//			&interp_error);
  		}

  		/* do lever arm correction */
  		if (platform_ptr != NULL) {
  			/* calculate sensor position position */
  			status = mb_platform_position(verbose, platform_ptr, pars->target_sensor, 0, pulse->navlon, pulse->navlat,
  			                              pulse->sensordepth, heading, roll, pitch, &pulse->navlon, &pulse->navlat,
  			                              &pulse->sensordepth, error);

  			/* calculate sensor attitude */
  			status = mb_platform_orientation_target(verbose, platform_ptr, pars->target_sensor, 0, heading, roll, pitch, &heading,
  			                                        &roll, &pitch, error);
  			pulse->heading = (float)heading;
  			pulse->roll = (float)roll;
  			pulse->pitch = (float)pitch;
  		}
  	}

  	/* calculate the bathymetry using the newly inserted values */
  	status = mbsys_3datdepthlidar_calculatebathymetry(verbose, mbio_ptr, store_ptr, error);
  }

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
int mbsys_3datdepthlidar_extract(int verbose,     /* in: verbosity level set on command line 0..N */
                                 void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                 void *store_ptr, /* in: see mbsys_3datdepthlidar.h:/^struct mbsys_3datdepthlidar_struct/ */
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
	(void)ss;  //  Unused arg
	(void)ssacrosstrack;  //  Unused arg
	(void)ssalongtrack;  //  Unused arg


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
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

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

		/* get the number of soundings according to mode */
		if (store->counts_per_scan > 0) {
			*nbath = store->counts_per_scan;
		}
		else {
			*nbath = store->counts_per_cross_track * store->counts_per_forward_track;
		}
		*namp = *nbath;
		*nss = 0;

		/* we are poking into the mb_io_ptr to change the beamwidth here
		    350 microradians for the LIDAR laser */
		mb_io_ptr->beamwidth_xtrack = 0.02;
		mb_io_ptr->beamwidth_ltrack = 0.02;

		/* get the bathymetry */
		for (int i = 0; i < *nbath; i++) {
			struct mbsys_3datdepthlidar_pulse_struct *pulse = &store->pulses[i];
			beamflag[i] = pulse->beamflag;
			bath[i] = pulse->depth + pulse->sensordepth;
			amp[i] = pulse->amplitude;
			bathacrosstrack[i] = pulse->acrosstrack;
			bathalongtrack[i] = pulse->alongtrack;
		}

		/* always successful */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}

	else if (*kind == MB_DATA_COMMENT) {
    memset((void *)comment, 0, sizeof(comment));
		strncpy(comment, store->comment, MIN(sizeof(comment), sizeof(store->comment)) - 1);
  }

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
} /* mbsys_3datdepthlidar_extract */
/*--------------------------------------------------------------------*/
int mbsys_3datdepthlidar_insert(int verbose,     /* in: verbosity level set on command line 0..N */
                                void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                void *store_ptr, /* in: see mbsys_3datdepthlidar.h:/^struct mbsys_3datdepthlidar_struct/ */
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
	(void)ss;  //  Unused arg
	(void)ssacrosstrack;  //  Unused arg
	(void)ssalongtrack;  //  Unused arg

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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	/* get data kind */
	store->kind = kind;

	int status = MB_SUCCESS;

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
		mb_get_time(verbose, time_i, &time_d);

		/* set the navigation */
		store->navlon = navlon;
		store->navlat = navlat;
		store->speed = speed;
		store->heading = heading;

		const double dlon = navlon - store->navlon;
		const double dlat = navlat - store->navlat;
		const double dheading = heading - store->heading;

		/* set the bathymetry */
		for (int i = 0; i < nbath; i++) {
			struct mbsys_3datdepthlidar_pulse_struct *pulse = &store->pulses[i];
			pulse->beamflag = beamflag[i];
			pulse->navlon += dlon;
			pulse->navlat += dlat;
			pulse->heading += dheading;
			if (pulse->heading < 0.0)
				pulse->heading += 360.0;
			if (pulse->heading > 360.0)
				pulse->heading -= 360.0;
			pulse->depth = bath[i] - pulse->sensordepth;
			pulse->amplitude = amp[i];
			pulse->acrosstrack = bathacrosstrack[i];
			pulse->alongtrack = bathalongtrack[i];
		}

		/* insert the sidescan pixel data */
	}

	/* deal with comments */
	else if (store->kind == MB_DATA_COMMENT) {
		store->time_d = time_d;
		store->comment_len = strlen(comment) + 1;
    memset((void *)store->comment, 0, sizeof(store->comment));
		strncpy(store->comment, comment, MIN(sizeof(store->comment), sizeof(comment)) - 1);
	}

	/* deal with other records types  */
	else {
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	if (verbose >= 4)
		mbsys_3datdepthlidar_print_store(verbose, store, error);
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return status;
} /* mbsys_3datdepthlidar_insert */
/*--------------------------------------------------------------------*/
int mbsys_3datdepthlidar_ttimes(int verbose,            /* in: verbosity level set on command line 0..N */
                                void *mbio_ptr,         /* in: see mb_io.h:/^struct mb_io_struct/ */
                                void *store_ptr,        /* in: see mbsys_3datdepthlidar.h:/^struct mbsys_3datdepthlidar_struct/ */
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structre pointer */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract travel time data */
	if (*kind == MB_DATA_DATA) {
		/* get the number of soundings according to mode */
		if (store->counts_per_scan > 0) {
			*nbeams = store->counts_per_scan;
		}
		else {
			*nbeams = store->counts_per_cross_track * store->counts_per_forward_track;
		}

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
} /* mbsys_3datdepthlidar_ttimes */
/*--------------------------------------------------------------------*/
int mbsys_3datdepthlidar_detects(int verbose,     /* in: verbosity level set on command line 0..N */
                                 void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                 void *store_ptr, /* in: see mbsys_3datdepthlidar.h:/^struct mbsys_3datdepthlidar_struct/ */
                                 int *kind,       /* out: MBIO data type; see mb_status.h:0+/MBIO data type/ */
                                 int *nbeams,     /* out: number of beams (samples) in this ping */
                                 int *detects, /* out: array[nbeams] detection flag;
                                                   see mb_status.h:/Bottom detect flags/ */
                                 int *error /* out: see mb_status.h:/MB_ERROR/ */
                                 ) {
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get the number of soundings according to mode */
		if (store->counts_per_scan > 0) {
			*nbeams = store->counts_per_scan;
		}
		else {
			*nbeams = store->counts_per_cross_track * store->counts_per_forward_track;
		}

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
} /* mbsys_3datdepthlidar_detects */
/*--------------------------------------------------------------------*/
int mbsys_3datdepthlidar_pulses(int verbose,     /* in: verbosity level set on command line 0..N */
                                void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                void *store_ptr, /* in: see mbsys_3datdepthlidar.h:/^struct mbsys_3datdepthlidar_struct/ */
                                int *kind,       /* out: MBIO data type; see mb_status.h:0+/MBIO data type/ */
                                int *nbeams,     /* out: number of beams (samples) in this ping */
                                int *pulses,     /* out: array[nbeams] pulse type; see mb_status.h:/Source pulse/ */
                                int *error       /* out: see mb_status.h:/MB_ERROR/ */
                                ) {
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get the number of soundings according to mode */
		if (store->counts_per_scan > 0) {
			*nbeams = store->counts_per_scan;
		}
		else {
			*nbeams = store->counts_per_cross_track * store->counts_per_forward_track;
		}

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
} /* mbsys_3datdepthlidar_pulses */
/*--------------------------------------------------------------------*/
int mbsys_3datdepthlidar_gains(int verbose,           /* in: verbosity level set on command line 0..N */
                               void *mbio_ptr,        /* in: see mb_io.h:/^struct mb_io_struct/ */
                               void *store_ptr,       /* in: see mbsys_3datdepthlidar.h:/^struct mbsys_3datdepthlidar_struct/ */
                               int *kind,             /* in: MBIO data type; see mb_status.h:0+/MBIO data type/ */
                               double *transmit_gain, /* out: transmit gain (dB) */
                               double *pulse_length,  /* out: pulse width (usec) */
                               double *receive_gain,  /* out: receive gain (dB) */
                               int *error             /* out: see mb_status.h:/MB_ERROR/ */
                               ) {
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* get transmit_gain (dB) [I don't know how to convert this to db]*/
		*transmit_gain = 0.0;

		/* get pulse_length */
		*pulse_length = 0.0;

		/* get receive_gain (dB) [I don't know how to convert to db] */
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
} /* mbsys_3datdepthlidar_gains */
/*--------------------------------------------------------------------*/
int mbsys_3datdepthlidar_extract_altitude(
    int verbose,              /* in: verbosity level set on command line 0..N */
    void *mbio_ptr,           /* in: see mb_io.h:/^struct mb_io_struct/ */
    void *store_ptr,          /* in: see mbsys_3datdepthlidar.h:/^struct mbsys_3datdepthlidar_struct/ */
    int *kind,                /* in: MBIO data type; see mb_status.h:0+/MBIO data type/ */
    double *transducer_depth, /* out: transducer depth below water line (m) */
    double *altitude,         /* out: transducer altitude above seafloor (m) */
    int *error                /* out: see mb_status.h:/MB_ERROR/ */
    ) {
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get sonar depth */
		*transducer_depth = store->sensordepth;

		/* loop over all soundings looking for most nadir */
		double rmin = 9999999.9;
		for (int i = 0; i < store->num_pulses; i++) {
			struct mbsys_3datdepthlidar_pulse_struct *pulse = &store->pulses[i];
			const double r = sqrt(pulse->acrosstrack * pulse->acrosstrack + pulse->alongtrack * pulse->alongtrack);
			if (r < rmin) {
				rmin = r;
				*altitude = pulse->depth;
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
} /* mbsys_3datdepthlidar_extract_altitude */
/*--------------------------------------------------------------------*/
int mbsys_3datdepthlidar_extract_nnav(int verbose,     /* in: verbosity level set on command line 0..N */
                                      void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                                      void *store_ptr, /* in: see mbsys_3datdepthlidar.h:/^struct mbsys_3datdepthlidar_struct/ */
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

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
		for (int inav = 0; inav < *n; inav++) {
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
} /* mbsys_3datdepthlidar_extract_nnav */
/*--------------------------------------------------------------------*/
int mbsys_3datdepthlidar_extract_nav(int verbose, void *mbio_ptr, /* in: verbosity level set on command line 0..N */
                                     void *store_ptr,             /* in: see mb_io.h:/^struct mb_io_struct/ */
                                     int *kind,       /* out: see mbsys_3datdepthlidar.h:/^struct mbsys_3datdepthlidar_struct/ */
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	/* extract data from structure */
	*kind = store->kind;

	int status = MB_SUCCESS;

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
} /* mbsys_3datdepthlidar_extract_nav */
/*----------------------------------------------------------------------*/
int mbsys_3datdepthlidar_insert_nav(int verbose, void *mbio_ptr, /* in: verbosity level set on command line */
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* insert data in swathplus data structure */
	if (store->kind == MB_DATA_DATA) {
		const double dlon = navlon - store->navlon;
		const double dlat = navlat - store->navlat;
		const double dheading = heading - store->heading;
		const double dsensordepth = draft - heave - store->sensordepth;
		const double droll = roll - store->roll;
		const double dpitch = pitch - store->pitch;

		store->time_d = time_d;
		store->navlon = navlon;
		store->navlat = navlat;
		store->speed = speed;
		store->heading = heading;
		store->sensordepth = draft - heave;
		store->roll = roll;
		store->pitch = pitch;

		/* need to apply nav values to all pulses */
		for (int i = 0; i < store->num_pulses; i++) {
			struct mbsys_3datdepthlidar_pulse_struct *pulse = &store->pulses[i];
			pulse->navlon += dlon;
			pulse->navlat += dlat;
			pulse->sensordepth += dsensordepth;
			pulse->heading += dheading;
			if (pulse->heading < 0.0)
				pulse->heading += 360.0;
			if (pulse->heading > 360.0)
				pulse->heading -= 360.0;
			pulse->roll += droll;
			pulse->pitch += dpitch;
		}

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
} /* mbsys_3datdepthlidar_insert_nav */
/*----------------------------------------------------------------------*/
int mbsys_3datdepthlidar_extract_svp(int verbose,      /* in: verbosity level set on command line 0..N */
                                     void *mbio_ptr,   /* in: see mb_io.h:mb_io_struct */
                                     void *store_ptr,  /* in: see mbsys_3datdepthlidar.h:mbsys_3datdepthlidar_struct */
                                     int *kind,        /* out: see mb_status.h:MBIO data type */
                                     int *nsvp,        /* out: number of svp measurements */
                                     double *depth,    /* out: array[nsvp] depths (m) */
                                     double *velocity, /* out: array[nsvp] velocity (m) */
                                     int *error        /* out: see: mb_status.h:MB_ERROR */
                                     ) {
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

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
} /* mbsys_3datdepthlidar_extract_svp */
/*----------------------------------------------------------------------*/
int mbsys_3datdepthlidar_insert_svp(int verbose,      /* in: verbosity level set on command line 0..N */
                                    void *mbio_ptr,   /* in: mbio.h:mb_io_struct */
                                    void *store_ptr,  /* in: mbsys_3datdepthlidar_struct */
                                    int nsvp,         /* in: number of svp records to insert */
                                    double *depth,    /* in: array[nsvp] depth records (m) */
                                    double *velocity, /* in: array[nsvp] sound velocity records (m/s) */
                                    int *error        /* out: see mb_status.h:MB_ERROR */
                                    ) {
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	int status = MB_SUCCESS;

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
} /* mbsys_3datdepthlidar_insert_svp */
/*----------------------------------------------------------------------*/
int mbsys_3datdepthlidar_copy(int verbose,     /* in: verbosity level set on command line */
                              void *mbio_ptr,  /* in: see mb_io.h:mb_io_struct */
                              void *store_ptr, /* in: see mbsys_3datdepthlidar.h:mbsys_3datdepthlidar_struct */
                              void *copy_ptr,  /* out: see mbsys_3datdepthlidar.h:mbsys_3datdepthlidar_struct */
                              int *error       /* out: see mb_status.h:MB_ERROR */
                              ) {
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
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointers */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;
	struct mbsys_3datdepthlidar_struct *copy = (struct mbsys_3datdepthlidar_struct *)copy_ptr;

	/* copy structure */
	copy = store;

	/* allocate array of pulse structures */
	copy->pulses = NULL;

	/* allocate memory for data structure */
	int npulses;
	if (store->counts_per_scan > 0) {
		npulses = store->counts_per_scan;
	}
	else {
		npulses = store->counts_per_cross_track * store->counts_per_forward_track;
	}
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, npulses * sizeof(struct mbsys_3datdepthlidar_pulse_struct),
	                    (void **)store_ptr, error);

	/* copy pulses */
	memcpy((void *)copy->pulses, (void *)store->pulses, npulses * sizeof(struct mbsys_3datdepthlidar_pulse_struct));

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
} /* mbsys_3datdepthlidar_copy */
/*----------------------------------------------------------------------*/
int mbsys_3datdepthlidar_print_store(int verbose,     /* in: verbosity level set on command line 0..N */
                                     void *store_ptr, /* in: see mbsys_3datdepthlidar.h:mbsys_3datdepthlidar_struct */
                                     int *error       /* out: see mb_status.h:MB_ERROR */
                                     ) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2         store:    %p\n", store_ptr);
	}

	/* check for non-null data */
	assert(store_ptr != NULL);

	/* always successful */
	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* get data structure pointers */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	/* print 3datdepthlidar store structure contents */
	const char debug_str[] = "dbg2  ";
	const char nodebug_str[] = "  ";
	const char *first;
	if (verbose >= 2)
		first = debug_str;
	else {
		first = nodebug_str;
		fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
	}
	fprintf(stderr, "%s struct mbsys_3datdepthlidar contents:\n", first);
	fprintf(stderr, "%s     kind:                          %d\n", first, store->kind);
	fprintf(stderr, "%s     file_version:                  %u\n", first, store->file_version);
	fprintf(stderr, "%s     sub_version:                   %u\n", first, store->sub_version);
	fprintf(stderr, "%s     scan_type:                     %u\n", first, store->scan_type);
	fprintf(stderr, "%s     cross_track_angle_start:       %f\n", first, store->cross_track_angle_start);
	fprintf(stderr, "%s     cross_track_angle_end:         %f\n", first, store->cross_track_angle_end);
	fprintf(stderr, "%s     forward_track_angle_start:     %f\n", first, store->forward_track_angle_start);
	fprintf(stderr, "%s     forward_track_angle_end:       %f\n", first, store->forward_track_angle_end);
	fprintf(stderr, "%s     counts_per_scan:               %u\n", first, store->counts_per_scan);
	fprintf(stderr, "%s     counts_per_cross_track:        %u\n", first, store->counts_per_cross_track);
	fprintf(stderr, "%s     counts_per_forward_track:      %u\n", first, store->counts_per_forward_track);
	fprintf(stderr, "%s     scanner_efficiency:            %u\n", first, store->scanner_efficiency);
	fprintf(stderr, "%s     scans_per_file:                %u\n", first, store->scans_per_file);
	fprintf(stderr, "%s     scan_count:                    %u\n", first, store->scan_count);
	fprintf(stderr, "%s     record_id:                     %u\n", first, store->record_id);
	if (store->kind == MB_DATA_DATA) {
		fprintf(stderr, "%s     current_scan:                  %d\n", first, store->current_scan);
		fprintf(stderr, "%s     year:                          %u\n", first, store->year);
		fprintf(stderr, "%s     month:                         %u\n", first, store->month);
		fprintf(stderr, "%s     day:                           %u\n", first, store->day);
		fprintf(stderr, "%s     days_since_jan_1:              %u\n", first, store->days_since_jan_1);
		fprintf(stderr, "%s     hour:                          %u\n", first, store->hour);
		fprintf(stderr, "%s     minutes:                       %u\n", first, store->minutes);
		fprintf(stderr, "%s     seconds:                       %u\n", first, store->seconds);
		fprintf(stderr, "%s     nanoseconds:                   %u\n", first, store->nanoseconds);
		fprintf(stderr, "%s     time_d:                        %f\n", first, store->time_d);
		fprintf(stderr, "%s     navlon:                        %f\n", first, store->navlon);
		fprintf(stderr, "%s     navlat:                        %f\n", first, store->navlat);
		fprintf(stderr, "%s     sensordepth:                    %f\n", first, store->sensordepth);
		fprintf(stderr, "%s     heading:                       %f\n", first, store->heading);
		fprintf(stderr, "%s     roll:                          %f\n", first, store->roll);
		fprintf(stderr, "%s     pitch:                         %f\n", first, store->pitch);
		fprintf(stderr, "%s     speed:                         %f\n", first, store->speed);
		fprintf(stderr, "%s     bathymetry_calculated:         %d\n", first, store->bathymetry_calculated);
		fprintf(stderr, "%s     num_pulses:                    %d\n", first, store->num_pulses);
		fprintf(stderr, "%s     num_pulses_alloc:              %d\n", first, store->num_pulses_alloc);
		// const int npulses =
		//	store->counts_per_scan > 0
		//	? store->counts_per_scan
                //	: store->counts_per_cross_track * store->counts_per_forward_track;

		for (int i = 0; i < store->num_pulses; i++) {
			struct mbsys_3datdepthlidar_pulse_struct *pulse = &(store->pulses[i]);
			fprintf(stderr, "%s------------------------------------------\n", first);
			fprintf(stderr, "%s     pulse:                         %d\n", first, i);
			fprintf(stderr, "%s     range:                         %f\n", first, pulse->range);
			fprintf(stderr, "%s     amplitude:                     %d\n", first, pulse->amplitude);
			fprintf(stderr, "%s     snr:                           %f\n", first, pulse->snr);
			fprintf(stderr, "%s     cross_track_angle:             %f\n", first, pulse->cross_track_angle);
			fprintf(stderr, "%s     forward_track_angle:           %f\n", first, pulse->forward_track_angle);
			fprintf(stderr, "%s     cross_track_offset:            %f\n", first, pulse->cross_track_offset);
			fprintf(stderr, "%s     forward_track_offset:          %f\n", first, pulse->forward_track_offset);
			fprintf(stderr, "%s     pulse_time_offset:             %d\n", first, pulse->pulse_time_offset);
			fprintf(stderr, "%s     saturated:                     %u\n", first, pulse->saturated);
			fprintf(stderr, "%s     time_d:                        %f\n", first, pulse->time_d);
			fprintf(stderr, "%s     beamflag:                      %u\n", first, pulse->beamflag);
			fprintf(stderr, "%s     acrosstrack:                   %f\n", first, pulse->acrosstrack);
			fprintf(stderr, "%s     alongtrack:                    %f\n", first, pulse->alongtrack);
			fprintf(stderr, "%s     depth:                         %f\n", first, pulse->depth);
			fprintf(stderr, "%s     navlon:                        %f\n", first, pulse->navlon);
			fprintf(stderr, "%s     navlat:                        %f\n", first, pulse->navlat);
			fprintf(stderr, "%s     sensordepth:                    %f\n", first, pulse->sensordepth);
			fprintf(stderr, "%s     heading:                       %f\n", first, pulse->heading);
			fprintf(stderr, "%s     roll:                          %f\n", first, pulse->roll);
			fprintf(stderr, "%s     pitch:                         %f\n", first, pulse->pitch);
		}
	}
	else if (store->kind == MB_DATA_COMMENT) {
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
} /* mbsys_3datdepthlidar_print_store */
/*--------------------------------------------------------------------*/
int mbsys_3datdepthlidar_calculatebathymetry(int verbose,     /* in: verbosity level set on command line 0..N */
                                             void *mbio_ptr,  /* in: see mb_io.h:mb_io_struct */
                                             void *store_ptr, /* in: see mbsys_3datdepthlidar.h:mbsys_3datdepthlidar_struct */
                                             int *error       /* out: see mb_status.h:MB_ERROR */
                                             ) {
	(void)mbio_ptr;  // Unused arg
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2         store:    %p\n", store_ptr);
	}

	/* check for non-null data */
	assert(store_ptr != NULL);

	/* always successful */
	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* get data structure pointers */
	struct mbsys_3datdepthlidar_struct *store = (struct mbsys_3datdepthlidar_struct *)store_ptr;

	/* recalculate bathymetry from LIDAR data */
	if (store->kind == MB_DATA_DATA) {
		/* get time_d timestamp */
		int time_i[7];
		time_i[0] = store->year;
		time_i[1] = store->month;
		time_i[2] = store->day;
		time_i[3] = store->hour;
		time_i[4] = store->minutes;
		time_i[5] = store->seconds;
		time_i[6] = (int)(0.001 * store->nanoseconds);
		mb_get_time(verbose, time_i, &store->time_d);

		/* get scaling */
		double mtodeglon;
		double mtodeglat;
		mb_coor_scale(verbose, store->navlat, &mtodeglon, &mtodeglat);

		/* loop over all pulses */
		for (int i = 0; i < store->num_pulses; i++) {
			/* get pulse */
			struct mbsys_3datdepthlidar_pulse_struct *pulse = (struct mbsys_3datdepthlidar_pulse_struct *)&store->pulses[i];

			/* valid pulses have nonzero ranges */
			if (pulse->range > 0.001) {
				/* set beamflag */
				pulse->beamflag = MB_FLAG_NONE;

				/* apply pitch and roll */
				const double alpha = pulse->forward_track_angle + pulse->pitch;
				const double beta = 90.0 - pulse->cross_track_angle + pulse->roll;

				/* translate to takeoff coordinates */
				double theta;
				double phi;
				mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);

				/* get lateral and vertical components of range */
				const double xx = pulse->range * sin(DTR * theta);
				pulse->depth = pulse->range * cos(DTR * theta);
				pulse->acrosstrack = xx * cos(DTR * phi) + pulse->cross_track_offset;
				pulse->alongtrack =
				    xx * sin(DTR * phi) + pulse->forward_track_offset + 0.0000002777777 * pulse->pulse_time_offset * store->speed;
			}
			else {
				/* null everything */
				pulse->beamflag = MB_FLAG_NULL;
				pulse->depth = 0.0;
				pulse->acrosstrack = 0.0;
				pulse->alongtrack = 0.0;
			}
		}

		/* set the bathymetry_calculated flag */
		store->bathymetry_calculated = true;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
} /* mbsys_3datdepthlidar_print_store */
/*--------------------------------------------------------------------*/
