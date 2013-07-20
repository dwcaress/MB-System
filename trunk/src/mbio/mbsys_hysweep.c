/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_hysweep.c	3.00	12/23/2011
 *	$Id$
 *
 *    Copyright (c) 2011-2013 by
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
 * mbsys_hysweep.c contains the MBIO functions for handling data logged
 * in the HYSWEEP format using HYSWEEP from HYPACK Inc.
 * The data format associated with this representation is:
 *      MBF_HYSWEEP1 : MBIO ID 201
 *
 * Author:	D. W. Caress
 * Date:	December 23, 2011
 *
 * $Log: mbsys_hysweep.c,v $
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_hysweep.h"

/* turn on debug statements here */
/* #define MSYS_HYSWEEP_DEBUG 1 */

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbsys_hysweep_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error)
{
	char	*function_name = "mbsys_hysweep_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;
	struct mbsys_hysweep_device_struct *device;
	struct mbsys_hysweep_device_offset_struct *offset;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* allocate memory for data structure */
	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_hysweep_struct),
				(void **)store_ptr, error);

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) *store_ptr;

	/* initialize everything */

	/* Type of data record */
	store->kind = MB_DATA_NONE;
	store->type = MBSYS_HYSWEEP_RECORDTYPE_NONE;

	/* HYSWEEP file header */
	for (i=0;i<MB_NAME_LENGTH;i++)
		store->FTP_record[i] = '\0';	/* FTP NEW 2: HYSWEEP file type */
	store->HSX_record = 0;			/* HSX: HYSWEEP format version number */
	for (i=0;i<MB_NAME_LENGTH;i++)
		store->VER_version[i] = '\0';	/* VER: HYSWEEP distribution version */
	for (i=0;i<7;i++)
		store->TND_survey_time_i[i] = 0;/* TND: Survey time and date (system startup)
							hh:mm:ss mm/dd/yy */
	store->TND_survey_time_d = 0.0;
	for (i=0;i<MB_NAME_LENGTH;i++)
		store->INF_surveyor[i] = '\0';	/* INF: surveyor name */
	for (i=0;i<MB_NAME_LENGTH;i++)
		store->INF_boat[i] = '\0';	/* INF: boat name */
	for (i=0;i<MB_NAME_LENGTH;i++)
		store->INF_project[i] = '\0';	/* INF: project name */
	for (i=0;i<MB_NAME_LENGTH;i++)
		store->INF_area[i] = '\0';	/* INF: area name */
	store->INF_tide_correction = 0.0;	/* INF: initial tide correction */
	store->INF_draft_correction = 0.0;	/* INF: initial draft correction */
	store->INF_sound_velocity = 0.0;	/* INF: initial sound velocity */
	store->HSP_minimum_depth = 0.0;	/* HSP: minimum depth in work units */
	store->HSP_maximum_depth = 0.0;	/* HSP: maximum depth in work units */
	store->HSP_port_offset_limit = 0.0;	/* HSP: port side offset limit in work units */
	store->HSP_stbd_offset_limit = 0.0;	/* HSP: starboard side offset limit in work units */
	store->HSP_port_angle_limit = 0.0;	/* HSP: port side angle limit in degrees */
	store->HSP_stbd_angle_limit = 0.0;	/* HSP: starboard side angle limit in degrees */
	store->HSP_high_beam_quality = 0;	/* HSP: high beam quality; codes >= this are good */
	store->HSP_low_beam_quality = 0;	/* HSP: low beam quality; codes < this are bad */
	store->HSP_sonar_range = 0.0;	/* HSP: sonar range setting in work units */
	store->HSP_towfish_layback = 0.0;	/* HSP: towfish layback in work units */
	store->HSP_units = 0;		/* HSP: work units:
						0: = meters
						1 = US foot
						2 = international foot */
	store->HSP_sonar_id = 0;		/* HSP: sonar id for advanced processing (see defines above) */
	for (i=0;i<MB_NAME_LENGTH;i++)
		store->PRJ_proj4_command[i] = '\0';	/* PRJ: projection in use defined as either
								a PROJ4 command string or as an
								EPSG identifier */

	/* HYSWEEP devices */
	store->num_devices = 0;		/* number of devices defined */
	for (i=0;i<MBSYS_HYSWEEP_DEVICE_NUM_MAX;i++)
		{
		device = (struct mbsys_hysweep_device_struct *) &(store->devices[i]);

		/* DEV: first line of device declaration
			DEV dn dc "name"
			DEV 0 544 "IXSEA OCTANS Serial" */
		device->DEV_device_number = 0;
		device->DEV_device_capability = 0;	/* Hypack device capabilities (bit code)
							1 - device provides range/range position
							2 - device provides range/azimuth position
							4 - device provides lat/long (e.g. GPS)
							8 - device provides grid positions XY
							16 - device provides echo soundings
							32 - device provides heading
							64 - device provides ship speed
							128 - Hypack clock is synched to device clock
							256 - device provides tide
							512 - device provides heave, pitch, and roll
							1024 - device is a ROV
							2048 - device is a left/right indicator
							4096 - device accepts annotations strings
							8192 - device accepts output from Hypack
							16384 - xxx
							32768 - device has extended capability */
		for (j=0;j<MB_PATH_MAXLINE;j++)
			device->DEV_device_name[j] = '\0';		/* e.g. "GPS" */

		/* DV2: second line of device declaration
			DV2 dn dc tf en
			DV2 0 220 0 1 */
		device->DV2_device_capability = 0;	/* Hysweep device capabilities (bit coded hexadecimal)
							0001 - multibeam sonar
							0002 - multiple transducer sonar
							0004 - GPS (boat position)
							0008 - sidescan sonar
							0010 - single beam echosounder
							0020 - gyro (boat heading)
							0040 - tide
							0200 - MRU (heave, pitch, and roll compensation) */
		device->DV2_towfish = 0;		/* 1 if device is mountedc on a tow fish */
		device->DV2_enabled = 0;		/* 1 if device is enabled */

		/* OF2: Hysweep device offsets  */
		device->num_offsets = 0;		/* number of offsets identified for this device */
		for (j=0;j<MBSYS_HYSWEEP_OFFSET_NUM_MAX;j++)
			{
			offset = (struct mbsys_hysweep_device_offset_struct *) &(device->offsets[j]);

			/* OF2: Hysweep device offsets
				OF2 dn on n1 n2 n3 n4 n5 n6 n7 */
			offset->OF2_device_number = 0;	/* device number */
			offset->OF2_offset_type = 0;	/* offset type
								0 - position antenna offsets
								1 - gyro heading offset
								2 - MRU device offsets
								3 - sonar head 1 / transducer 1 offsets
								4 - sonar head 2 / transducer 2 offsets
								........
								131 - transducer 128 offsets */
			offset->OF2_offset_starboard = 0.0;	/* starboard/port mounting offset, positive to starboard */
			offset->OF2_offset_forward = 0.0;	/* forward/aft mounting offset, positive to forward */
			offset->OF2_offset_vertical = 0.0;	/* vertical mounting offset, positive downward from waterline */
			offset->OF2_offset_yaw = 0.0;		/* yaw rotation angle, positive for clockwise rotation */
			offset->OF2_offset_roll = 0.0;	/* roll rotation angle, port side up is positive */
			offset->OF2_offset_pitch = 0.0;	/* pitch rotation angle, bow up is positive */
			offset->OF2_offset_time = 0.0;	/* time latency in seconds */
			}

		/* PRI: device set as primary navigational device */
		device->PRI_primary_nav_device = 0;	/* 1 if device is primary navigational device */

		/* MBI: multibeam / multiple transducer device information
			MBI dn st sf db n1 n2 fa ai */
		device->MBI_sonar_id = 0;	/* sonar id from table, not part of MBI record
							but instead inferred from device name in the
							corresponding DEV record */
		device->MBI_sonar_receive_shape = 0;	/* sonar receive head shape, not part of MBI record
								but instead inferred from device name in the
								corresponding DEV record
									0 - flat
									1 - circular */
		device->MBI_sonar_type = 0;	/* sonar type code:
							0 - invalid
							1 - fixed beam roll angles (e.g. Reson Seabat )
							2 - variable beam roll angles (e.g. Seabeam SB1185)
							3 - beam info in spherical coordinates (e.g. Simrad EM3000)
							4 - multiple transducer (e.g. Odom Miniscan) */
		device->MBI_sonar_flags = 0;	/* sonar flags (bit coded hexadecimal)
							0x0001 - roll corrected by sonar
							0x0002 - pitch corrected by sonar
							0x0004 - dual head
							0x0008 - heading corrected by sonar (ver 1)
							0x0010 - medium depth: slant ranges recorded to 1 dm
									resolution (version 2)
							0x0020 - deep water: slant ranges divided by 1 m
									resolution (ver 2)
							0x0040 - SVP corrected by sonar (ver 5)
							0x0080 - topographic device, upgoing beams accepted (ver 6) */
		device->MBI_beam_data_available = 0;/* beam data (bit coded hexadecimal)
							0x0001 - beam ranges are available (survey units)
							0x0002 - sounding point casting available (survey units)
							0x0004 - point northing available (survey units)
							0x0008 - point corrected depth available (survey units)
							0x0010 - along track distance available (survey units)
							0x0020 - across track distance available (survey units)
							0x0040 - beam pitch angles available (degrees, TSS convention)
							0x0080 - beam roll angles available (degrees, TSS convention)
							0x0100 - beam takeoff angles available (degrees from vertical)
							0x0200 - beam direction angles available (degrees from forward)
							0x0400 - ping delay times included (milliseconds)
							0x0800 - beam intensity data available
							0x1000 - beam quality codes (from sonar unit) available
							0x2000 - sounding flags included
							0x4000 - spare
							0x8000 - spare */
		device->MBI_num_beams_1 = 0;	/* number of beams, head 1 (multibeam) or number of transducers
							(multitransducer) */
		device->MBI_num_beams_2 = 0;	/* number of beams, head 2 (multibeam) */
		device->MBI_first_beam_angle = 0.0;	/* first beam angle is for sonar type = fixed angle
							(degrees, TSS convention) */
		device->MBI_angle_increment = 0.0;	/* angle increment is for sonare type = fixed angle
							(degrees, TSS convention) */

		/* SSI: sidescan device information
			SSI dn sf np ns */
		device->SSI_sonar_flags = 0;	/* sonar flags (bit coded hexadecimal)
							0x0100 - amplitude is bit-shifted into byte storage */
		device->SSI_port_num_samples = 0;	/* number of samples per ping, port transducer */
		device->SSI_starboard_num_samples = 0;	/* number of samples per ping, starboard transducer */
		}
	store->primary_nav_device = 0;	/* device number of primary navigational device */

	/* HYSWEEP HVF - Hysweep view filters - always first record after end of file header
		HVF dn tt p1 p2 p3 p4 p5 p6
			dn: dummy device number, always = 99
			tt: time tag this filter set became active (in seconds past midnight)
			p1: minimum depth in work units
			p2: maximum depth in work units
			p3: port side offset limit in work units
			p4: starboard side offset limit in work units
			p5: minimum beam angle limit, 0 to -90 degrees
			p6: maximum beam angle limit, 0 to 90 degrees
		Example:
		HVF 99 80491.965 0.0 95.0 35.6 35.6 -75.0 75.0 */
	store->HVF_time_after_midnight = 0.0;	/* time this filter set became active
							seconds after midnight */
	store->HVF_minimum_depth = 0.0;	/* minimum depth in work units */
	store->HVF_maximum_depth = 0.0;	/* maximum depth in work units */
	store->HVF_port_offset_limit = 0.0;	/* port side offset limit in work units */
	store->HVF_starboard_offset_limit = 0.0;	/* starboard side offset limit in work units */
	store->HVF_minimum_angle_limit = 0.0;	/* minimum beam angle limit, 0 to -90 degrees */
	store->HVF_maximum_angle_limit = 0.0;	/* maximum beam angle limit, 0 to 90 degrees */

	/* HYSWEEP FIX - fix (event) mark - always second record after end of file header
		FIX dn tt en  */
	store->FIX_device_number = 0;	/* device number */
	store->FIX_time_after_midnight = 0.0;	/* time in seconds after midnight */
	store->FIX_event_number = 0;		/* FIX event number */

	/* HYSWEEP RMB - raw multibeam data
		RMB dn t st sf bd n sv pn psa
			dn: device number 
			t: time tag (seconds past midnight) 
			st: sonar type code (see MBI above) 
			sf: sonar flags (see MBI above) 
			bd: available beam data (see MBI above) 
			n: number of beams to follow 
			sv: sound velocity in m/sec 
			pn: ping number (or 0 if not tracked)

		Immediately following the RMB record is a record containing slant ranges (multibeam) 
		or raw depths (multiple transducer). Following the ranges are 0 to n additional records 
		depending on the bd (beam data) field.

		Example (Seabat 9001 storing slant ranges and quality codes): 
			RMB 1 27244.135 1 0 1001 1500.00 0 60 
			19.50 19.31 18.60 1.66 18.47 ... (60 slant ranges in work units) 
			3 3 3 0 3 ... (60 quality codes)

		Example (multiple transducer storing 8 raw depths): 
			RMB 1 27244.135 4 0 1 1500.00 0 60 
			31.44 33.01 32.83 32.80 ... (8 raw depths in work units)

		Example (Dual-head Seabeam SB1185 storing range, beam pitch and roll angles, 
			ping delay times, beam quality code and sounding flags): 
			RMB 1 27244.135 2 5 1481 1500.00 0 
			108 93.18 88.30 84.74 80.46 ... (108 slant ranges in working units)
			-69.72 -68.53 -67.36 -66.15 ... (108 beam roll angles in degrees) 
			0 0 0 67 ... (108 ping delay times in msecs) 
			7 7 7 7 ... (108 beam quality codes) */
	store->RMB_device_number = 0;	/* device number */
	store->RMB_time = 0.0;		/* time tag (seconds past midnight) */
	store->RMB_sonar_type = 0;		/* sonar type code:
						0 - invalid
						1 - fixed beam roll angles (e.g. Reson Seabat )
						2 - variable beam roll angles (e.g. Seabeam SB1185)
						3 - beam info in spherical coordinates (e.g. Simrad EM3000)
						4 - multiple transducer (e.g. Odom Miniscan) */
	store->RMB_sonar_flags = 0;	/* sonar flags (bit coded hexadecimal)
						0x0001 - roll corrected by sonar
						0x0002 - pitch corrected by sonar
						0x0004 - dual head
						0x0008 - heading corrected by sonar (ver 1)
						0x0010 - medium depth: slant ranges recorded to 1 dm
								resolution (version 2)
						0x0020 - deep water: slant ranges divided by 1 m
								resolution (ver 2)
						0x0040 - SVP corrected by sonar (ver 5)
						0x0080 - topographic device, upgoing beams accepted (ver 6) */
	store->RMB_beam_data_available = 0;/* beam data (bit coded hexadecimal)
						0x0001 - beam ranges are available (survey units)
						0x0002 - sounding point casting available (survey units)
						0x0004 - point northing available (survey units)
						0x0008 - point corrected depth available (survey units)
						0x0010 - along track distance available (survey units)
						0x0020 - across track distance available (survey units)
						0x0040 - beam pitch angles available (degrees, TSS convention)
						0x0080 - beam roll angles available (degrees, TSS convention)
						0x0100 - beam takeoff angles available (degrees from vertical)
						0x0200 - beam direction angles available (degrees from forward)
						0x0400 - ping delay times included (milliseconds)
						0x0800 - beam intensity data available
						0x1000 - beam quality codes (from sonar unit) available
						0x2000 - sounding flags included
						0x4000 - spare
						0x8000 - spare */
	store->RMB_num_beams = 0;		/* number of beams to follow */
	store->RMB_num_beams_alloc = 0;		/* number of beams allocated to non-null arrays */
	store->RMB_sound_velocity = 0.0;	/* sound velocity in m/sec */
	store->RMB_ping_number = 0;	/* ping number (or 0 if not tracked) */
	store->RMB_beam_ranges = NULL;	/* beam ranges (survey units) */
	store->RMB_multi_ranges = NULL;	/* sounding point casting (survey units) (multi-transducer rangers) */
	store->RMB_sounding_eastings = NULL;	/* easting positions of soundings */
	store->RMB_sounding_northings = NULL;/* northing positions of soundings */
	store->RMB_sounding_depths = NULL;	/* corrected depths of soundings */
	store->RMB_sounding_across = NULL;	/* acrosstrack positions of soundings */
	store->RMB_sounding_along = NULL;	/* alongtrack positions of soundings */
	store->RMB_sounding_pitchangles = NULL;	/* beam pitch angles of soundings (degrees, TSS convention) */
	store->RMB_sounding_rollangles = NULL;	/* beam roll angles of soundings (degrees, TSS convention) */
	store->RMB_sounding_takeoffangles = NULL;	/* beam takeoff angles of soundings (degrees from vertical) */
	store->RMB_sounding_azimuthalangles = NULL;	/* beam azimuthal angles of soundings (degrees from forward) */
	store->RMB_sounding_timedelays = NULL;	/* beam delay times (milliseconds) */
	store->RMB_sounding_intensities = NULL;	/* beam intensities */
	store->RMB_sounding_quality = NULL;		/* beam quality codes (from sonar unit) */
	store->RMB_sounding_flags = NULL;		/* beam edit flags */

	/* RSS - Raw Sidescan
		RSS dn t sf np ns sv pn alt sr amin amax bs freq
			dn: device number
			t: time tag (seconds past midnight)
			sf: sonar flags (bit coded hexadecimal)
				0100 - amplitude is bit-shifted into byte storage
			np: number of samples, port transducer (down-sampled to 4096 max)
			ns: number of samples, starboard transducer (down-sampled to 4096 max)
			sv: sound velocity in m/sec
			pn: ping number (or 0 if not tracked)
			alt: altitude in work units
			sr: sample rate (samples per second after down-sample)
			amin: amplitude minimum
			amax: amplitude maximum
			bs: bit shift for byte recording
			freq: frequency 0 or 1 for simultaneous dual frequency operation

		Immediately following the RSS record are two records containing port and
		starboard amplitude samples.

		Example:
		RSS 3 61323.082 100 341 341 1460.00 0 10.75 4983.47 0 4096 4 0
		109 97 84 95 120 111 ... (341 port samples)
		106 93 163 106 114 127 ... (341 starboard samples) */
	store->RSS_device_number = 0;	/* device number */
	store->RSS_time = 0.0;		/* time tag (seconds past midnight) */
	store->RSS_sonar_flags = 0;	/* sonar flags:
						0100 - amplitude is bit-shifted into byte storage */
	store->RSS_port_num_samples = 0; 	/* number of samples, port transducer (down-sampled to 4096 max) */
	store->RSS_starboard_num_samples = 0; 	/* number of samples, starboard transducer
						(down-sampled to 4096 max) */
	store->RSS_port_num_samples_alloc = 0;
	store->RSS_starboard_num_samples_alloc = 0;
	store->RSS_sound_velocity = 0.0;	/* sound velocity in m/sec  */
	store->RSS_ping_number = 0.0;	/* ping number (or 0 if not tracked) */
	store->RSS_altitude = 0.0;		/* altitude in work units  */
	store->RSS_sample_rate = 0.0;	/* sample rate (samples per second after down-sample)  */
	store->RSS_minimum_amplitude = 0;	/* amplitude minimum  */
	store->RSS_maximum_amplitude = 0;	/* amplitude maximum  */
	store->RSS_bit_shift = 0;		/* bit shift for byte recording  */
	store->RSS_frequency = 0;		/* frequency 0 or 1 for simultaneous dual frequency operation */
	store->RSS_port = NULL;		/* port sidescan amplitude samples */
	store->RSS_starboard = NULL;		/* starboard sidescan amplitude samples */

        /* MSS - MB-System Sidescan
                MSS dn t pn n ps
			dn: device number
			t: time tag (seconds past midnight)
			pn: ping number (or 0 if not tracked)
			n: number of pixels
			ps: pixel size in meters

		Immediately following the MSS record is one record containing
		the amplitude samples. Zero samples are null values
		indicating no data.

		Example:
		MSS 3 61323.082 1500.00 1024 0.340 27535
		0.00 0.00 109.25 97.13 .... 95.34 120.76 111.26 0.00 (1024 samples) */
	store->MSS_device_number = 0;	/* device number */
	store->MSS_time = 0.0;		/* time tag (seconds past midnight) */
	store->MSS_sound_velocity = 0.0;	/* sound velocity in m/sec  */
	store->MSS_num_pixels = 0;	        /* number of pixels (typically 1024) */
	store->MSS_pixel_size = 0.0;	        /* pixel size (meters) */
	store->MSS_ping_number = 0;	/* ping number (or 0 if not tracked) */
	for (i=0;i<MBSYS_HYSWEEP_MSS_NUM_PIXELS;i++)
		{
		store->MSS_ss[i] = 0.0;
		store->MSS_ss_across[i] = 0.0;
		store->MSS_ss_along[i] = 0.0;
		}
        store->MSS_table_num_alloc = 0;          /* sidescan working array allocated dimensions */
        store->MSS_table_altitude_sort = NULL;   /* sidescan working array - altitude = depth - draft + heave */
        store->MSS_table_range = NULL;           /* sidescan working array - range */
        store->MSS_table_acrosstrack = NULL;     /* sidescan working array - acrosstrack */
        store->MSS_table_alongtrack = NULL;      /* sidescan working array - alongtrack */

	/* SNR - dynamic sonar settings
		up to 12 fields depending on sonar type
		SNR dn t pn sonar ns s0 --- s11
			dn: device number 
			t: time tag (seconds past midnight) 
			pn: ping number (or 0 if not tracked)
			sonar: sonar ID code (see defines above) 
			ns: number of settings to follow
			s: up to 12 settings

		Up to 12 fields are included in SNR records, providing sonar runtime settings.
		Not available for all systems. Defined differently depending on sonar model
		and manufacturer.

		For Seabat 81XX Serial and 81XX Network Drivers:
			Sonar id: 1, 23, 24, 25, 39
			P0: Sonar range setting in meters.
			P1: power setting, 0 - 8
			P2: gain setting, 1 Ð 45
			P3: gain modes: bit 0 = TVG on/off, bit 1 = auto gain on/off.

		For Seabat 7K drivers (7125, 7101, 7150, 7111)
			Sonar id: 22, 53, 60, 62
			P0: Sonar range selection in meters.
			P1: Transmit power selection in dBs relative to 1 uPa.
			P2: Receiver gain selection in 0.1 dBs.
			P3: Transmitter frequency in KHz.
			P4: Transmit pulse width in microseconds.

		For EdgeTech 4200 Driver
			Sonar id: 7-10
			P0: Pulse power setting, 0 to 100 percent.
			P1: ADC Gain factor.
			P2: Start Frequency in 10 * Hz.
			P3: End Frequency in 10 * Hz.
			P4: Sweep length in milliseconds. */
	store->SNR_device_number = 0;	/* device number */
	store->SNR_time = 0.0;		/* time tag (seconds past midnight) */
	store->SNR_ping_number = 0.0;	/* ping number (or 0 if not tracked) */
	store->SNR_sonar_id = 0.0;		/* sonar ID (see defines above) */
	store->SNR_num_settings = 0.0;	/* number of settings to follow */
	for (i=0;i<12;i++)
		store->SNR_settings[i] = 0;	/* sonar settings */

	/* PSA - pitch stabilization angle
		PSA dn t pn a0 a1
			dn: device number 
			t: time tag (seconds past midnight) 
			pn: ping number (or 0 if not tracked)
			a0: projector (head 0) pitch angle
			a1: projector (head 1) pitch angle

		Note: PSA records are recorded only when pitch stabilization
		is active. They immediately proceed corresponding RMB records. */
	store->PSA_device_number = 0;	/* device number */
	store->PSA_time = 0.0;		/* time tag (seconds past midnight) */
	store->PSA_ping_number = 0.0;	/* ping number (or 0 if not tracked) */
	store->PSA_a0 = 0.0;			/* projector (head 0) pitch angle */
	store->PSA_a1 = 0.0;			/* projector (head 1) pitch angle */

	/* HCP - heave compensation
		HCP dn t h r p
			dn: device number
			t: time tag (seconds past midnight)
			h: heave in meters
			r: roll in degrees (+ port side up)
			p: pitch in degrees (+ bow up)
		Example:
		HCP 0 80492.021 -0.65 -0.51 -1.73 */
	store->HCP_device_number = 0;	/* device number */
	store->HCP_time = 0.0;		/* time tag (seconds past midnight) */
	store->HCP_heave = 0.0;		/* heave (meters) */
	store->HCP_roll = 0.0;		/* roll (+ port side up) */
	store->HCP_pitch = 0.0;		/* pitch (+ bow up) */

	/* EC1 - echo sounding (single frequency)
		EC1 dn t rd
			dn: device number
			t: time tag (seconds past midnight)
			rd: raw depth
		Example:
		EC1 1 80491.897 99.02 */
	store->EC1_device_number = 0;	/* device number */
	store->EC1_time = 0.0;		/* time tag (seconds past midnight) */
	store->EC1_rawdepth = 0.0;		/* raw depth */

	/* GPS - GPS measurements
		GPS dn t cog sog hdeop mode nsats
			dn: device number
			t: time tag (seconds past midnight)
			cog: course over ground (degrees)
			sog: speed over ground (knots)
			hdop: GPS hdop
			mode: GPS mode
				0 - unknown
				1 - stand alone
				2 - differential
				3 - rtk
			nsats: number of satellites
		Example:
		GPS 1 80491.897 178.16 0.23 1.2 2 8 */
	store->GPS_device_number = 0;	/* device number */
	store->GPS_time = 0.0;		/* time tag (seconds past midnight) */
	store->GPS_cog = 0.0;		/* course over ground (degrees) */
	store->GPS_sog = 0.0;		/* speed over ground (knots) */
	store->GPS_hdop = 0.0;		/* GPS hdop */
	store->GPS_mode = 0;		/* GPS mode
						0 - unknown
						1 - stand alone
						2 - differential
						3 - rtk */
	store->GPS_nsats = 0;		/* number of satellites */

	/* GYR - gyro data (heading)
		GYR dn t h
			dn: device number
			t: time tag (seconds past midnight)
			h: heading (degrees)
		Example:
		GYR 1 80491.897 178.16 */
	store->GYR_device_number = 0;	/* device number */
	store->GYR_time = 0.0;		/* time tag (seconds past midnight) */
	store->GYR_heading = 0.0;		/* heading (degrees) */

	/* POS - position
		POS dn t x y
			dn: device number
			t: time tag (seconds past midnight)
			x: easting
			y: northing
		Example:
		POS 1 80491.897 308214.82 1414714.97 */
	store->POS_device_number = 0;	/* device number */
	store->POS_time = 0.0;		/* time tag (seconds past midnight) */
	store->POS_x = 0.0;			/* easting */
	store->POS_y = 0.0;			/* northing */

	/* DFT - dynamic draft (squat) correction
		DFT dn t dc
			dn: device number
			t: time tag (seconds past midnight)
			dc: draft correction
		Example:
		DFT 1 80491.897 1453.44 */
	store->DFT_device_number = 0;	/* device number */
	store->DFT_time = 0.0;		/* time tag (seconds past midnight) */
	store->DFT_draft = 0.0;		/* draft correction */

	/* TID - tide correction
		TID dn t tc
			dn: device number
			t: time tag (seconds past midnight)
			tc: tide correction
		Example:
		TID 1 80491.897 0.00 */
	store->TID_device_number = 0;	/* device number */
	store->TID_time = 0.0;		/* time tag (seconds past midnight) */
	store->TID_tide = 0.0;		/* tide correction */

	/* COM - comment record - MB-System extension
		COM c
			c: comment string
		Example:
		COM everything after the first four characters is part of the comment */
	for (i=0;i<MB_COMMENT_MAXLINE;i++)
		store->COM_comment[i] = '\0';		/* comment string */

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)*store_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hysweep_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error)
{
	char	*function_name = "mbsys_hysweep_deall";
	int	status = MB_SUCCESS;
	struct mbsys_hysweep_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)*store_ptr);
		}

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) *store_ptr;

	/* deallocate arrays */
	if (store->RMB_beam_ranges != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_beam_ranges), error);	/* beam ranges (survey units) */
	if (store->RMB_multi_ranges != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_multi_ranges), error);	/* sounding point casting (survey units) (multi-transducer rangers) */
	if (store->RMB_sounding_eastings != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_sounding_eastings), error);	/* easting positions of soundings */
	if (store->RMB_sounding_northings != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_sounding_northings), error);/* northing positions of soundings */
	if (store->RMB_sounding_depths != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_sounding_depths), error);	/* corrected depths of soundings */
	if (store->RMB_sounding_across != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_sounding_across), error);	/* acrosstrack positions of soundings */
	if (store->RMB_sounding_along != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_sounding_along), error);	/* alongtrack positions of soundings */
	if (store->RMB_sounding_pitchangles != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_sounding_pitchangles), error);	/* beam pitch angles of soundings (degrees, TSS convention) */
	if (store->RMB_sounding_rollangles != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_sounding_rollangles), error);	/* beam roll angles of soundings (degrees, TSS convention) */
	if (store->RMB_sounding_takeoffangles != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_sounding_takeoffangles), error);	/* beam takeoff angles of soundings (degrees from vertical) */
	if (store->RMB_sounding_azimuthalangles != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_sounding_azimuthalangles), error);	/* beam azimuthal angles of soundings (degrees from forward) */
	if (store->RMB_sounding_timedelays != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_sounding_timedelays), error);	/* beam delay times (milliseconds) */
	if (store->RMB_sounding_intensities != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_sounding_intensities), error);	/* beam intensities */
	if (store->RMB_sounding_quality != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_sounding_quality), error);		/* beam quality codes (from sonar unit) */
	if (store->RMB_sounding_flags != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RMB_sounding_flags), error);		/* beam edit flags */
	if (store->RSS_port != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RSS_port), error);		/* port sidescan amplitude samples */
	if (store->RSS_starboard != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->RSS_starboard), error);		/* starboard sidescan amplitude samples */
        if (store->MSS_table_altitude_sort != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->MSS_table_altitude_sort), error);   /* sidescan working array - altitude = depth - draft + heave */
        if (store->MSS_table_range != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->MSS_table_range), error);           /* sidescan working array - range */
        if (store->MSS_table_acrosstrack != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->MSS_table_acrosstrack), error);     /* sidescan working array - acrosstrack */
        if (store->MSS_table_alongtrack != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)(&store->MSS_table_alongtrack), error);      /* sidescan working array - alongtrack */

	/* deallocate memory for data structure */
	status = mb_freed(verbose,__FILE__,__LINE__,(void **)store_ptr,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbsys_hysweep_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nbath, int *namp, int *nss, int *error)
{
	char	*function_name = "mbsys_hysweep_dimensions";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get beam and pixel numbers */
		*nbath = store->RMB_num_beams;
		if (store->RMB_beam_data_available | 0x0800)
			*namp = store->RMB_num_beams;
		else
			*namp = 0;
		*nss = 0;
		}
	else
		{
		/* get beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		fprintf(stderr,"dbg2       nbath:      %d\n",*nbath);
		fprintf(stderr,"dbg2        namp:      %d\n",*namp);
		fprintf(stderr,"dbg2        nss:       %d\n",*nss);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hysweep_pingnumber(int verbose, void *mbio_ptr,
		int *pingnumber, int *error)
{
	char	*function_name = "mbsys_hysweep_pingnumber";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) mb_io_ptr->store_data;

	/* extract data from structure */
	*pingnumber = store->RMB_ping_number;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       pingnumber: %d\n",*pingnumber);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hysweep_sonartype(int verbose, void *mbio_ptr, void *store_ptr,
		int *sonartype, int *error)
{
	char	*function_name = "mbsys_hysweep_sonartype";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) store_ptr;

	/* get sonar type */
	if (store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_RESON_SEABAT8101_150		/* Reson Seabat 8101 - 150 Deg - 1 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_RESON_SEABAT7125		/* Reson Seabat 7125 - 22 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_RESON_SEABAT8111		/* Reson Seabat 8111 - 23 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_RESON_SEABAT8124		/* Reson Seabat 8124 - 24 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_RESON_SEABAT8125		/* Reson Seabat 8125 - 25 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_RESON_SEABAT9001		/* Reson Seabat 9001 - 26 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_RESON_SEABAT9003		/* Reson Seabat 9003 - 27 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SEABEAM_2100		/* Seabeam 2100 - 29 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SEABEAM_1185		/* Seabeam SB1185 - 30 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SIMRAD_EA400		/* Simrad EA400 - 31 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SIMRAD_EM102		/* Simrad EM102 - 32 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SIMRAD_EM1002		/* Simrad EM1002 - 33 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SIMRAD_EM2000		/* Simrad EM2000 - 34 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SIMRAD_EM3000		/* Simrad EM3000 - 35 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SIMRAD_EM3000D		/* Simrad EM3000D - 36 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SIMRAD_EM3002		/* Simrad EM3002 - 37 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SIMRAD_EM3002D		/* Simrad EM3002D - 38 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_RESON_SEABAT8101_210	/* Reson Seabat 8101 - 210 Deg - 39 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_IMAGENEX_DELTAT		/* Imagenex Delta T - 40 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_ATLAS_HYDROSWEEPMD2	/* Atlas Hydrosweep MD2 - 41 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SIMRAD_SM2000		/* Simrad SM2000 - 42 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SIMRAD_EM710		/* Simrad EM710 - 43 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SIMRAD_EM302		/* Simrad EM302 - 44 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_BLUEVIEW_MB1350_45	/* Blueview MB1350-45 - 45 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_BLUEVIEW_MB2250_45	/* Blueview MB2250-45 - 46 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_BLUEVIEW_MB1350_90	/* Blueview MB1350-90 - 47 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_BLUEVIEW_MB2250_90	/* Blueview MB2250-90 - 48 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_RESON_7101		/* Reson Seabat 7101 - 53 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_FURUNO_HS300F		/* Furuno HS-300F - 54 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_FURUNO_HS600		/* Furuno HS-600 - 55 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_FURUNO_HS600F		/* Furuno HS-600F - 56 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_RESON_8150		/* Reson 8150 - 58 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_RESON_8160		/* Reson 8160 - 59 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_RESON_7150		/* Reson 7150 - 60 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_RESON_7111		/* Reson Seabat 7111 - 62 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_R2SONIC_SONIC2024		/* R2Sonic SONIC 2024 - 63 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_WASSP_MULTIBEAM		/* WASSP Multibeam - 65 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_ATLAS_HYDROSWEEPMD50	/* Atlas Hydrosweep  MD/50 - 66 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_ATLAS_HYDROSWEEPMD30	/* Atlas Hydrosweep  MD/30 - 67 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_ATLAS_HYDROSWEEPDS	/* Atlas Hydrosweep  DS - 68 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SEABEAM_3012		/* SeaBeam 3012 - 70 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SEABEAM_3020		/* SeaBeam 3020 - 71 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SEABEAM_3050		/* SeaBeam 3050 - 72 */)
		{
		*sonartype = MB_SONARTYPE_MULTIBEAM;
		}
	else if (store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_ATLAS_FANSWEEP20		/* Atlas Fansweep 20 - 2 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_BENTHOS_C3D		/* Benthos C3D - 3 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_GEOACOUSTICS_GEOSWATH	/* GeoAcoustics GeoSwath - 11 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_KLEIN_5000		/* Klein 5000 - 18 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SEA_SWATHPLUS		/* SEA Swathplus - 28 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_GEOACOUSTICS_DSS		/* Geoacoustics digital sidescan - 49 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_EDGETECH_4600		/* Edgetech 4600 - 61 */)
		{
		*sonartype = MB_SONARTYPE_INTERFEROMETRIC;
		}
	else if (store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_EDGETECH_272		/* EdgeTech 272 - 5 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_EDGETECH_4100		/* EdgeTech 4100 - 6 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_EDGETECH_4125		/* EdgeTech 4125 - 7 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_EDGETECH_4150		/* EdgeTech 4150 - 8 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_EDGETECH_4200		/* EdgeTech 4200 - 9 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_EDGETECH_4300		/* EdgeTech 4300 - 10 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_KLEIN_595			/* Klein 595 - 14 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_KLEIN_2000		/* Klein 2000 - 15 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_KLEIN_3000		/* Klein 3000 - 16 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_KLEIN_3900		/* Klein 3900 - 17 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_BENTHOS_1624		/* Benthos 1624 - 50 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_BENTHOS_1625		/* Benthos 1625 - 51 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_MARINESONIC_SEASCAN	/* Marine Sonic Sea Scan - 52 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_TRITECH_STARFISH		/* Tritech Starfish - 57 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_INNOMAR_SES		/* Innomar SES - 69 */)
		{
		*sonartype = MB_SONARTYPE_SIDESCAN;
		}
	else if (store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_CMAX_CM2			/* CMAX CM-2 - 4 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_IMAGENEX_SPORTSCAN	/* Imagenex Sportscan - 12 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_IMAGENEX_YELLOWFIN	/* Imagenex Yellowfin - 13 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_ODOM_CV3			/* Odom CV3 - 19 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_ODOM_ECHOSCAN2		/* Odom Echoscan 2 - 20 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_ODOM_ES3			/* Odom ES3 - 21 */
		|| store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_MDL_DYNASCAN		/* MDL Dynascan - 64 */)
		{
		*sonartype = MB_SONARTYPE_ECHOSOUNDER;
		}
	else
		{
		*sonartype = MB_SONARTYPE_UNKNOWN;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       sonartype:  %d\n",*sonartype);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hysweep_sidescantype(int verbose, void *mbio_ptr, void *store_ptr,
		int *ss_type, int *error)
{
	char	*function_name = "mbsys_hysweep_sidescantype";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) store_ptr;

	/* get sidescan type */
	if (store->HSP_sonar_id == MBSYS_HYSWEEP_SONAR_SEABEAM_2100		/* Seabeam 2100 - 29 */)
		{
		*ss_type = MB_SIDESCAN_LINEAR;
		}
	else
		{
		*ss_type = MB_SIDESCAN_LOGARITHMIC;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ss_type:    %d\n",*ss_type);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hysweep_extract(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_hysweep_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		for (i=0;i<7;i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get interpolated nav heading and speed  */
		*speed = 0.0;
		if (mb_io_ptr->nfix > 0)
			mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed,
				    navlon, navlat, speed, error);

		/* get heading */
		*heading = store->RMBint_heading;

		/* get navigation - actually return the easting northing because
			the projection is set and so the position will be inverse
			projected */
		*navlon = store->RMBint_x;
		*navlat = store->RMBint_y;

		/* read distance and depth values into storage arrays */
		*nbath = store->RMB_num_beams;
		if (store->RMB_beam_data_available | 0x0800)
			*namp = store->RMB_num_beams;
		else
			*namp = 0;
		for (i=0;i<*nbath;i++)
			{
			if (store->RMB_sounding_depths != NULL)
				bath[i] = store->RMB_sounding_depths[i];
			else
				bath[i] = 0.0;
			if (store->RMB_sounding_depths != NULL)
				bathacrosstrack[i] = store->RMB_sounding_across[i];
			else
				bathacrosstrack[i] = 0.0;
			if (store->RMB_sounding_depths != NULL)
				bathalongtrack[i] = store->RMB_sounding_along[i];
			else
				bathalongtrack[i] = 0.0;
			if (store->RMB_sounding_depths != NULL)
				amp[i] = store->RMB_sounding_intensities[i];
			else
				amp[i] = 0.0;
			if (store->RMB_sounding_flags != NULL)
				{
				beamflag[i] = store->RMB_sounding_flags[i];
				}
			else
				{
				if (store->RMB_sounding_quality[i] >= store->HSP_high_beam_quality)
					{
					beamflag[i] = MB_FLAG_NONE;
					}
				else
					{
					beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
					}
				}
			}

		/* initialize sidescan */
		if (store->MSS_num_pixels > 0
			&& (store->MSS_ping_number == store->RMB_ping_number
				|| 10 * store->MSS_ping_number == store->RMB_ping_number))
			{
			*nss = store->MSS_num_pixels;
			for (i=0;i<*nss;i++)
				{
				ss[i] =store->MSS_ss[i];
				ssacrosstrack[i] =store->MSS_ss_across[i];
				ssalongtrack[i] = store->MSS_ss_along[i];
				}
			}
		else
			*nss = 0;

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg4  Data extracted by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  Extracted values:\n");
			fprintf(stderr,"dbg4       kind:       %d\n",
				*kind);
			fprintf(stderr,"dbg4       error:      %d\n",
				*error);
			fprintf(stderr,"dbg4       time_i[0]:  %d\n",
				time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:  %d\n",
				time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:  %d\n",
				time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:  %d\n",
				time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:  %d\n",
				time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:  %d\n",
				time_i[5]);
			fprintf(stderr,"dbg4       time_i[6]:  %d\n",
				time_i[6]);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				*time_d);
			fprintf(stderr,"dbg4       longitude:  %f\n",
				*navlon);
			fprintf(stderr,"dbg4       latitude:   %f\n",
				*navlat);
			fprintf(stderr,"dbg4       speed:      %f\n",
				*speed);
			fprintf(stderr,"dbg4       heading:    %f\n",
				*heading);
			fprintf(stderr,"dbg4       nbath:      %d\n",
				*nbath);
			for (i=0;i<*nbath;i++)
			  fprintf(stderr,"dbg4       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
				i,beamflag[i],bath[i],
				bathacrosstrack[i],bathalongtrack[i]);
			fprintf(stderr,"dbg4        namp:     %d\n",
				*namp);
			for (i=0;i<*namp;i++)
			  fprintf(stderr,"dbg4        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
				i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
			fprintf(stderr,"dbg4        nss:      %d\n",
				*nss);
			for (i=0;i<*nss;i++)
			  fprintf(stderr,"dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
				i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
			}

		/* done translating values */

		}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV
		|| *kind == MB_DATA_NAV1
		|| *kind == MB_DATA_NAV2)
		{
		/* get time */
		for (i=0;i<7;i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get heading */
		if (mb_io_ptr->nheading > 0)
			mb_hedint_interp(verbose, mbio_ptr, store->time_d,
				    heading, error);

		/* get nav and speed */
		*speed = 0.0;
		if (mb_io_ptr->nfix > 0)
			mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed,
				    navlon, navlat, speed, error);

		/* get navigation - actually return the easting northing because
			the projection is set and so the position will be inverse
			projected */
		*navlon = store->POS_x;
		*navlat = store->POS_y;

		/* set beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg4  Data extracted by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  Extracted values:\n");
			fprintf(stderr,"dbg4       kind:       %d\n",
				*kind);
			fprintf(stderr,"dbg4       error:      %d\n",
				*error);
			fprintf(stderr,"dbg4       time_i[0]:  %d\n",
				time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:  %d\n",
				time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:  %d\n",
				time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:  %d\n",
				time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:  %d\n",
				time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:  %d\n",
				time_i[5]);
			fprintf(stderr,"dbg4       time_i[6]:  %d\n",
				time_i[6]);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				*time_d);
			fprintf(stderr,"dbg4       longitude:  %f\n",
				*navlon);
			fprintf(stderr,"dbg4       latitude:   %f\n",
				*navlat);
			fprintf(stderr,"dbg4       speed:      %f\n",
				*speed);
			fprintf(stderr,"dbg4       heading:    %f\n",
				*heading);
			}

		/* done translating values */

		}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* get time */
		for (i=0;i<7;i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* copy comment */
		if (strlen(store->COM_comment) > 0)
			strncpy(comment, store->COM_comment, MB_COMMENT_MAXLINE);
		else
			comment[0] = '\0';

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Comment extracted by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       kind:       %d\n",
				*kind);
			fprintf(stderr,"dbg4       error:      %d\n",
				*error);
			fprintf(stderr,"dbg4       time_i[0]:  %d\n",
				time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:  %d\n",
				time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:  %d\n",
				time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:  %d\n",
				time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:  %d\n",
				time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:  %d\n",
				time_i[5]);
			fprintf(stderr,"dbg4       time_i[6]:  %d\n",
				time_i[6]);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				*time_d);
			fprintf(stderr,"dbg4       comment:    %s\n",
				comment);
			}
		}

	/* set time for other data records */
	else
		{
		/* get time */
		for (i=0;i<7;i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Data extracted by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  Extracted values:\n");
			fprintf(stderr,"dbg4       kind:       %d\n",
				*kind);
			fprintf(stderr,"dbg4       error:      %d\n",
				*error);
			fprintf(stderr,"dbg4       time_i[0]:  %d\n",
				time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:  %d\n",
				time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:  %d\n",
				time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:  %d\n",
				time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:  %d\n",
				time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:  %d\n",
				time_i[5]);
			fprintf(stderr,"dbg4       time_i[6]:  %d\n",
				time_i[6]);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				*time_d);
			fprintf(stderr,"dbg4       comment:    %s\n",
				comment);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR
		&& *kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
			comment);
		}
	else if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR
		&& *kind != MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:     %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:        %f\n",*time_d);
		fprintf(stderr,"dbg2       longitude:     %f\n",*navlon);
		fprintf(stderr,"dbg2       latitude:      %f\n",*navlat);
		fprintf(stderr,"dbg2       speed:         %f\n",*speed);
		fprintf(stderr,"dbg2       heading:       %f\n",*heading);
		}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR
		&& *kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg2       nbath:      %d\n",
			*nbath);
		for (i=0;i<*nbath;i++)
		  fprintf(stderr,"dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,beamflag[i],bath[i],
			bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2        namp:     %d\n",
			*namp);
		for (i=0;i<*namp;i++)
		  fprintf(stderr,"dbg2       beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2        nss:      %d\n",
			*nss);
		for (i=0;i<*nss;i++)
		  fprintf(stderr,"dbg2        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
			i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
		}
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hysweep_insert(int verbose, void *mbio_ptr, void *store_ptr,
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_hysweep_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       kind:       %d\n",kind);
		}
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV1 || kind == MB_DATA_NAV2))
		{
		fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:  %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		fprintf(stderr,"dbg2       navlon:     %f\n",navlon);
		fprintf(stderr,"dbg2       navlat:     %f\n",navlat);
		fprintf(stderr,"dbg2       speed:      %f\n",speed);
		fprintf(stderr,"dbg2       heading:    %f\n",heading);
		}
	if (verbose >= 2 && kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg2       nbath:      %d\n",nbath);
		if (verbose >= 3)
		 for (i=0;i<nbath;i++)
		  fprintf(stderr,"dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,beamflag[i],bath[i],
			bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2       namp:       %d\n",namp);
		if (verbose >= 3)
		 for (i=0;i<namp;i++)
		  fprintf(stderr,"dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2        nss:       %d\n",nss);
		if (verbose >= 3)
		 for (i=0;i<nss;i++)
		  fprintf(stderr,"dbg3        beam:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
			i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
		}
	if (verbose >= 2 && kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
			comment);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		for (i=0;i<7;i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation - actually easting northings show up here because
			the projection is set and so the position has been
			projected */
		/* get navigation */
		store->RMBint_x = navlon;
		store->RMBint_y = navlat;
		mb_proj_inverse(verbose, mb_io_ptr->pjptr, navlon, navlat,
							&(store->RMBint_lon), &(store->RMBint_lat),
							error);

		/* get heading */
		store->RMBint_heading = heading;

		/* get speed  */

		/* only insert data if the number of beams matchs */
		if (nbath ==  store->RMB_num_beams)
			{
			/* allocate storage arrays if needed */
			if (store->RMB_sounding_depths == NULL)
				status = mb_reallocd(verbose, __FILE__, __LINE__, nbath * sizeof(double),
							(void **)&(store->RMB_sounding_depths), error);
			if (store->RMB_sounding_across == NULL)
				status = mb_reallocd(verbose, __FILE__, __LINE__, nbath * sizeof(double),
							(void **)&(store->RMB_sounding_across), error);
			if (store->RMB_sounding_along == NULL)
				status = mb_reallocd(verbose, __FILE__, __LINE__, nbath * sizeof(double),
							(void **)&(store->RMB_sounding_along), error);

			/* insert the depth and distance values into the storage arrays */
			for (i=0;i<store->RMB_num_beams;i++)
				{
				store->RMB_sounding_depths[i] = bath[i];
				store->RMB_sounding_across[i] = bathacrosstrack[i];
				store->RMB_sounding_along[i] = bathalongtrack[i];
				store->RMB_sounding_flags[i] = beamflag[i];
				}
			}
		if (namp ==  store->RMB_num_beams)
			{
			/* allocate storage arrays if needed */
			if (store->RMB_sounding_intensities == NULL)
				status = mb_reallocd(verbose, __FILE__, __LINE__, namp * sizeof(double),
							(void **)&(store->RMB_sounding_intensities), error);

			/* insert the amplitude values into the storage arrays */
			for (i=0;i<store->RMB_num_beams;i++)
				{
				store->RMB_sounding_intensities[i] = amp[i];
				}
			}

		/* only insert sidescan if the number of pixels matches */

		/* initialize sidescan */
		if (nss == store->MSS_num_pixels
			&& (store->MSS_ping_number == store->RMB_ping_number
				|| 10 * store->MSS_ping_number == store->RMB_ping_number))
			{
			for (i=0;i<nss;i++)
				{
				store->MSS_ss[i] = ss[i];
				store->MSS_ss_across[i] = ssacrosstrack[i];
				store->MSS_ss_along[i] = ssalongtrack[i];
				}
			}
		}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV
		|| store->kind == MB_DATA_NAV1
		|| store->kind == MB_DATA_NAV2)
		{
		/* get time */
		for (i=0;i<7;i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation - actually easting northings show up here because
			the projection is set and so the position has been
			projected */
		store->POS_x = navlon;
		store->POS_y = navlat;
fprintf(stderr, "INSERT1b %.3f %.2f %.2f\r\n",
	store->POS_time,
	store->POS_x,
	store->POS_y);

		/* get heading */

		/* get speed  */
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		strncpy(store->COM_comment, comment, MB_COMMENT_MAXLINE);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hysweep_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles,
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset,
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_hysweep_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;
	struct mbsys_hysweep_device_struct *device;
	double	alpha, beta, theta, phi;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       ttimes:     %lu\n",(size_t)ttimes);
		fprintf(stderr,"dbg2       angles_xtrk:%lu\n",(size_t)angles);
		fprintf(stderr,"dbg2       angles_ltrk:%lu\n",(size_t)angles_forward);
		fprintf(stderr,"dbg2       angles_null:%lu\n",(size_t)angles_null);
		fprintf(stderr,"dbg2       heave:      %lu\n",(size_t)heave);
		fprintf(stderr,"dbg2       ltrk_off:   %lu\n",(size_t)alongtrack_offset);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get sound velocity */
		*ssv = store->RMB_sound_velocity;

		/* get draft */
		*draft = store->RMBint_draft;

		/* get travel times, angles */
		*nbeams = store->RMB_num_beams;
		device = (struct mbsys_hysweep_device_struct *) &(store->devices[store->RMB_device_number]);
		for (i=0;i<store->RMB_num_beams;i++)
			{
			ttimes[i] = 2.0 * store->RMB_beam_ranges[i] / (*ssv);
			if (store->RMB_sounding_takeoffangles != NULL && store->RMB_sounding_azimuthalangles != NULL)
				{
				angles[i] = store->RMB_sounding_takeoffangles[i];
				angles_forward[i] = store->RMB_sounding_azimuthalangles[i] + 90.0;
				}
			else if (store->RMB_sounding_pitchangles != NULL && store->RMB_sounding_rollangles != NULL)
				{
				alpha = store->RMB_sounding_pitchangles[i];
				beta = 90.0 + store->RMB_sounding_rollangles[i];

				/* correct alpha for pitch if necessary */
				if (!(device->MBI_sonar_flags & 0x0002))
					alpha += store->RMBint_pitch;

				/* correct beta for roll if necessary */
				if (!(device->MBI_sonar_flags & 0x0001))
					beta -= store->RMBint_roll;

				mb_rollpitch_to_takeoff(
					verbose,
					alpha, beta,
					&theta, &phi,
					error);
				angles[i] = theta;
				angles_forward[i] = phi;
				}
			if (device->MBI_sonar_receive_shape == 1)
				angles_null[i] = angles[i];
			else
				angles_null[i] = 0.0;
			heave[i] = store->RMBint_heave;
			if (store->RMB_beam_data_available & 0x0400)
				alongtrack_offset[i] = store->GPS_sog * 0.0005144 * store->RMB_sounding_timedelays[i];
			else
				alongtrack_offset[i] = 0.0;
			}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       draft:      %f\n",*draft);
		fprintf(stderr,"dbg2       ssv:        %f\n",*ssv);
		fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
		for (i=0;i<*nbeams;i++)
			fprintf(stderr,"dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n",
				i,ttimes[i],angles[i],
				angles_forward[i],angles_null[i],
				heave[i],alongtrack_offset[i]);
		}
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hysweep_detects(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams, int *detects, int *error)
{
	char	*function_name = "mbsys_hysweep_detects";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       detects:    %lu\n",(size_t)detects);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* read distance and depth values into storage arrays */
		*nbeams = store->RMB_num_beams;
		for (i=0;i<*nbeams;i++)
			{
			detects[i] = MB_DETECT_UNKNOWN;
			}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
		for (i=0;i<*nbeams;i++)
			fprintf(stderr,"dbg2       beam %d: detects:%d\n",
				i,detects[i]);
		}
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hysweep_gains(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transmit_gain, double *pulse_length,
			double *receive_gain, int *error)
{
	char	*function_name = "mbsys_hysweep_gains";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get transmit_gain (dB) */
		*transmit_gain = (double)0.0;

		/* get pulse_length (usec) */
		*pulse_length = (double)0.0;

		/* get receive_gain (dB) */
		*receive_gain = (double)0.0;

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       transmit_gain: %f\n",*transmit_gain);
		fprintf(stderr,"dbg2       pulse_length:  %f\n",*pulse_length);
		fprintf(stderr,"dbg2       receive_gain:  %f\n",*receive_gain);
		}
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hysweep_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitudev,
	int *error)
{
	char	*function_name = "mbsys_hysweep_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;
	double	xtrackmin;
	int	altitude_found;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get transducer depth */
		*transducer_depth = store->RMBint_draft - store->RMBint_heave;

		/* get altitude */
		altitude_found = MB_NO;
		if (mb_io_ptr->naltitude > 0)
			{
			mb_altint_interp(verbose, mbio_ptr, store->time_d,
				    altitudev, error);
			altitude_found = MB_YES;
			}
		if (altitude_found == MB_NO)
			{
			/* get depth closest to nadir */
			xtrackmin = 999999.9;
			for (i=0;i<store->RMB_num_beams;i++)
				{
				if ((store->RMB_sounding_flags[i] == MB_FLAG_NONE)
					&& fabs((double)store->RMB_sounding_across[i]) < xtrackmin)
					{
					*altitudev = store->RMB_sounding_depths[i] - *transducer_depth;
					altitude_found = MB_YES;
					xtrackmin = fabs((double)store->RMB_sounding_across[i]);
					}
				}
			}
		if (altitude_found == MB_NO)
			{
			*altitudev = 0.0;
			}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:              %d\n",*kind);
		fprintf(stderr,"dbg2       transducer_depth:  %f\n",*transducer_depth);
		fprintf(stderr,"dbg2       altitude:          %f\n",*altitudev);
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hysweep_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft,
		double *roll, double *pitch, double *heave,
		int *error)
{
	char	*function_name = "mbsys_hysweep_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		for (i=0;i<7;i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get heading */
		*heading = store->RMBint_heading;

		/* get interpolated nav heading and speed  */
		*speed = 0.0;
		if (mb_io_ptr->nfix > 0)
			mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed,
				    navlon, navlat, speed, error);


		/* get navigation - actually return the easting northing because
			the projection is set and so the position will be inverse
			projected */
		*navlon = store->RMBint_x;
		*navlat = store->RMBint_y;

		/* get draft  */
		*draft = store->RMBint_draft;

		/* get attitude  */
		*roll = store->RMBint_roll;
		*pitch = store->RMBint_pitch;
		*heave = store->RMBint_heave;

		/* done translating values */

		}

	/* extract data from nav structure */
	else if (*kind == MB_DATA_NAV
		|| *kind == MB_DATA_NAV1
		|| *kind == MB_DATA_NAV2)
		{
		/* get time */
		for (i=0;i<7;i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get heading */
		if (mb_io_ptr->nheading > 0)
			mb_hedint_interp(verbose, mbio_ptr, store->time_d,
				    heading, error);

		/* get speed */
		*speed = 0.0;
		if (mb_io_ptr->nfix > 0)
			mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed,
				    navlon, navlat, speed, error);


		/* get navigation - actually return the easting northing because
			the projection is set and so the position will be inverse
			projected */
		*navlon = store->POS_x;
		*navlat = store->POS_y;

		/* get roll pitch and heave */
		if (mb_io_ptr->nattitude > 0)
			{
			mb_attint_interp(verbose, mbio_ptr, *time_d,
				    heave, roll, pitch, error);
			}

		/* get draft  */
		if (mb_io_ptr->nsonardepth > 0)
			{
			if (mb_io_ptr->nsonardepth > 0)
				mb_depint_interp(verbose, mbio_ptr, store->time_d,
				    draft, error);
			*heave = 0.0;
			}

		/* done translating values */

		}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;

		/* get time */
		for (i=0;i<7;i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;
		}

	/* deal with other record type */
	else
		{
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;

		/* get time */
		for (i=0;i<7;i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:          %d\n",*kind);
		fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:     %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:        %f\n",*time_d);
		fprintf(stderr,"dbg2       longitude:     %f\n",*navlon);
		fprintf(stderr,"dbg2       latitude:      %f\n",*navlat);
		fprintf(stderr,"dbg2       speed:         %f\n",*speed);
		fprintf(stderr,"dbg2       heading:       %f\n",*heading);
		fprintf(stderr,"dbg2       draft:         %f\n",*draft);
		fprintf(stderr,"dbg2       roll:          %f\n",*roll);
		fprintf(stderr,"dbg2       pitch:         %f\n",*pitch);
		fprintf(stderr,"dbg2       heave:         %f\n",*heave);
		fprintf(stderr,"dbg2       error:         %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:        %d\n",status);
		}
/*if (status == MB_SUCCESS)
fprintf(stderr,"%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %f %f %f %f %f %f %f %f\n",
time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
*navlon,*navlat,*speed,*heading,*draft,*roll,*pitch,*heave);*/

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hysweep_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft,
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_hysweep_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:  %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		fprintf(stderr,"dbg2       navlon:     %f\n",navlon);
		fprintf(stderr,"dbg2       navlat:     %f\n",navlat);
		fprintf(stderr,"dbg2       speed:      %f\n",speed);
		fprintf(stderr,"dbg2       heading:    %f\n",heading);
		fprintf(stderr,"dbg2       draft:      %f\n",draft);
		fprintf(stderr,"dbg2       roll:       %f\n",roll);
		fprintf(stderr,"dbg2       pitch:      %f\n",pitch);
		fprintf(stderr,"dbg2       heave:      %f\n",heave);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) store_ptr;

	/* insert data in ping structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		for (i=0;i<7;i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation - actually easting northings show up here because
			the projection is set and so the position has been
			projected */
		/* get navigation */
		store->RMBint_x = navlon;
		store->RMBint_y = navlat;
		mb_proj_inverse(verbose, mb_io_ptr->pjptr, navlon, navlat,
							&(store->RMBint_lon), &(store->RMBint_lat),
							error);

		/* get heading */
		store->RMBint_heading = heading;

		/* get speed  */

		/* get draft  */
		store->RMBint_draft = draft;

		/* get roll pitch and heave */
		store->RMBint_heave = heave;
		store->RMBint_pitch = pitch;
		store->RMBint_roll = roll;
		}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV
		|| store->kind == MB_DATA_NAV1
		|| store->kind == MB_DATA_NAV2)
		{
		/* get time */
		for (i=0;i<7;i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation - actually easting northings show up here because
			the projection is set and so the position has been
			projected */
		store->POS_x = navlon;
		store->POS_y = navlat;

		/* get heading */

		/* get speed  */

		/* get draft  */

		/* get roll pitch and heave */
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hysweep_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_hysweep_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;
	struct mbsys_hysweep_struct *copy;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       copy_ptr:   %lu\n",(size_t)copy_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_hysweep_struct *) store_ptr;
	copy = (struct mbsys_hysweep_struct *) copy_ptr;

	/* copy over structures, allocating memory where necessary */
	(*copy) = (*store);
	copy->RMB_beam_ranges = NULL;	/* beam ranges (survey units) */
	copy->RMB_multi_ranges = NULL;	/* sounding point casting (survey units) (multi-transducer rangers) */
	copy->RMB_sounding_eastings = NULL;	/* easting positions of soundings */
	copy->RMB_sounding_northings = NULL;/* northing positions of soundings */
	copy->RMB_sounding_depths = NULL;	/* corrected depths of soundings */
	copy->RMB_sounding_across = NULL;	/* acrosstrack positions of soundings */
	copy->RMB_sounding_along = NULL;	/* alongtrack positions of soundings */
	copy->RMB_sounding_pitchangles = NULL;	/* beam pitch angles of soundings (degrees, TSS convention) */
	copy->RMB_sounding_rollangles = NULL;	/* beam roll angles of soundings (degrees, TSS convention) */
	copy->RMB_sounding_takeoffangles = NULL;	/* beam takeoff angles of soundings (degrees from vertical) */
	copy->RMB_sounding_azimuthalangles = NULL;	/* beam azimuthal angles of soundings (degrees from forward) */
	copy->RMB_sounding_timedelays = NULL;	/* beam delay times (milliseconds) */
	copy->RMB_sounding_intensities = NULL;	/* beam intensities */
	copy->RMB_sounding_quality = NULL;		/* beam quality codes (from sonar unit) */
	copy->RMB_sounding_flags = NULL;		/* beam edit flags */
	if (copy->RMB_num_beams > 0)
		{
		if (store->RMB_beam_ranges != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(double),
					(void **)&(copy->RMB_beam_ranges), error);
		if (store->RMB_multi_ranges != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(double),
					(void **)&(copy->RMB_multi_ranges), error);
		if (store->RMB_sounding_eastings != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(double),
					(void **)&(copy->RMB_sounding_eastings), error);
		if (store->RMB_sounding_northings != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(double),
					(void **)&(copy->RMB_sounding_northings), error);
		if (store->RMB_sounding_depths != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(double),
					(void **)&(copy->RMB_sounding_depths), error);
		if (store->RMB_sounding_across != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(double),
					(void **)&(copy->RMB_sounding_across), error);
		if (store->RMB_sounding_along != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(double),
					(void **)&(copy->RMB_sounding_along), error);
		if (store->RMB_sounding_pitchangles != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(double),
					(void **)&(copy->RMB_sounding_pitchangles), error);
		if (store->RMB_sounding_rollangles != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(double),
					(void **)&(copy->RMB_sounding_rollangles), error);
		if (store->RMB_sounding_takeoffangles != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(double),
					(void **)&(copy->RMB_sounding_takeoffangles), error);
		if (store->RMB_sounding_azimuthalangles != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(double),
					(void **)&(copy->RMB_sounding_azimuthalangles), error);
		if (store->RMB_sounding_timedelays != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(int),
					(void **)&(copy->RMB_sounding_timedelays), error);
		if (store->RMB_sounding_intensities != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(int),
					(void **)&(copy->RMB_sounding_intensities), error);
		if (store->RMB_sounding_quality != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(int),
					(void **)&(copy->RMB_sounding_quality), error);
		if (store->RMB_sounding_flags != NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, copy->RMB_num_beams * sizeof(int),
					(void **)&(copy->RMB_sounding_flags), error);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hysweep_makess(int verbose, void *mbio_ptr, void *store_ptr,
		int pixel_size_set, double *pixel_size,
		int swath_width_set, double *swath_width,
		int pixel_int, int *error)
{
	char	*function_name = "mbsys_hysweep_makess";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hysweep_struct *store;
	int	nbathsort;
	double  pixel_size_calc;
	double	ss_spacing;
	int	iminxtrack;
	double	minxtrack;
	double	maxxtrack;
	int	nrangetable;
	double	acrosstracktablemin;
	int	irangenadir, irange;
	int	found;
	int	pixel_int_use;
	int	sample_start, sample_end;
	double	xtrackss, ltrackss, factor;
	double	range;
	int	first, last, k1, k2;
	int	i, j, k, kk;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:        %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:       %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       pixel_size_set:  %d\n",pixel_size_set);
		fprintf(stderr,"dbg2       pixel_size:      %f\n",*pixel_size);
		fprintf(stderr,"dbg2       swath_width_set: %d\n",swath_width_set);
		fprintf(stderr,"dbg2       swath_width:     %f\n",*swath_width);
		fprintf(stderr,"dbg2       pixel_int:       %d\n",pixel_int);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hysweep_struct *) store_ptr;

	/* generate processed sidescan if multibeam and sidescan records both exist */
	if (store->RMB_ping_number > 0
		&& (store->RMB_ping_number == store->RSS_ping_number
			|| store->RMB_ping_number == 10 * store->RSS_ping_number))
		{
		/* allocate working arrays as needed */
		if (store->MSS_table_num_alloc < store->RMB_num_beams)
			{
			status = mb_reallocd(verbose, __FILE__, __LINE__, store->RMB_num_beams * sizeof(double),
						(void **)&(store->MSS_table_altitude_sort), error);
			status = mb_reallocd(verbose, __FILE__, __LINE__, store->RMB_num_beams * sizeof(double),
						(void **)&(store->MSS_table_range), error);
			status = mb_reallocd(verbose, __FILE__, __LINE__, store->RMB_num_beams * sizeof(double),
						(void **)&(store->MSS_table_acrosstrack), error);
			status = mb_reallocd(verbose, __FILE__, __LINE__, store->RMB_num_beams * sizeof(double),
						(void **)&(store->MSS_table_alongtrack), error);
			if (status == MB_SUCCESS)
				store->MSS_table_num_alloc = store->RMB_num_beams;
			}

		/* set basic parameters */
		store->MSS_device_number = store->RSS_device_number;
		store->MSS_time = store->RSS_time;
		store->MSS_sound_velocity = store->RSS_sound_velocity;
		store->MSS_ping_number = store->RSS_ping_number;

		/* get raw pixel size */
		ss_spacing = 0.5 * store->RSS_sound_velocity / store->RSS_sample_rate;

		/* get median depth relative to the sonar and check for min max xtrack */
		nbathsort = 0;
		minxtrack = 0.0;
		maxxtrack = 0.0;
		iminxtrack = store->RMB_num_beams / 2;
		found = MB_NO;
		for (i=0;i<store->RMB_num_beams;i++)
			{
			if (mb_beam_ok(store->RMB_sounding_flags[i]))
				{
				store->MSS_table_altitude_sort[nbathsort] = store->RMB_sounding_depths[i]
										- store->RMBint_draft
										+ store->RMBint_heave;
				nbathsort++;

				if (found == MB_NO || fabs(store->RMB_sounding_across[i] < minxtrack))
					{
					minxtrack = fabs(store->RMB_sounding_across[i]);
					iminxtrack = i;
					found = MB_YES;
					}

				maxxtrack = MAX(fabs(store->RMB_sounding_across[i]), maxxtrack);
				}
			}
		if (nbathsort > 0)
			qsort((char *)store->MSS_table_altitude_sort, nbathsort, sizeof(double),(void *)mb_double_compare);

		/* set number of pixels */
		store->MSS_num_pixels = MBSYS_HYSWEEP_MSS_NUM_PIXELS;

		/* get sidescan swath width and pixel size */
		if (swath_width_set == MB_NO && store->RMB_num_beams > 0)
			{
			(*swath_width) = MAX(fabs(store->RMB_sounding_rollangles[0]),
					fabs(store->RMB_sounding_rollangles[store->RMB_num_beams-1]));
			}
		if (pixel_size_set == MB_NO
		    && nbathsort > 0)
			{
			/* calculate pixel size implied using swath width and nadir altitude */
			pixel_size_calc = 2.1 * tan(DTR * (*swath_width)) * store->MSS_table_altitude_sort[nbathsort/2]
										/ store->MSS_num_pixels;

			/* use pixel size based on actual swath width if that is larger than the first value */
			pixel_size_calc = MAX(pixel_size_calc, 2.1 * maxxtrack / store->MSS_num_pixels);

			/* make sure the pixel size is at least equivalent to a 0.1 degree nadir beamwidth */
			pixel_size_calc = MAX(pixel_size_calc, store->MSS_table_altitude_sort[nbathsort/2] * sin(DTR * 0.1));

			/* if the pixel size appears to be changing in size, moderate the change */
			if ((*pixel_size) <= 0.0)
				(*pixel_size) = pixel_size_calc;
			else if (0.95 * (*pixel_size) > pixel_size_calc)
				(*pixel_size) = 0.95 * (*pixel_size);
			else if (1.05 * (*pixel_size) < pixel_size_calc)
				(*pixel_size) = 1.05 * (*pixel_size);
			else
				(*pixel_size) = pixel_size_calc;
			}
		store->MSS_pixel_size = *pixel_size;

		/* get pixel interpolation */
		pixel_int_use = pixel_int + 1;

		/* zero the sidescan */
		for (i=0;i<store->MSS_num_pixels;i++)
			{
			store->MSS_ss[i] = 0.0;
			store->MSS_ss_across[i] = 0.0;
			store->MSS_ss_along[i] = 0.0;
			store->MSS_ss_cnt[i] = 0;
			}
		for (i=0;i<store->MSS_num_pixels;i++)
			{
			store->MSS_ss_across[i] = (*pixel_size) * (double)(i - (store->MSS_num_pixels / 2));;
			}

/* fprintf(stderr,"pixel_size:%f swath_width:%f altitude:%f\n",*pixel_size,*swath_width,
store->MSS_table_altitude_sort[nbathsort/2]); */

		/* loop over raw sidescan putting each raw sample into the binning arrays */
		/* get acrosstrack distance versus range table from bathymetry */
		nrangetable = 0;
		acrosstracktablemin = 99999.99;
		for (i=0;i<store->RMB_num_beams;i++)
			{
			if (mb_beam_ok(store->RMB_sounding_flags[i]))
				{
				store->MSS_table_range[nrangetable] = 2.0 * store->RMB_beam_ranges[i]
									/ store->RSS_sound_velocity;
				store->MSS_table_acrosstrack[nrangetable] = store->RMB_sounding_across[i];
				store->MSS_table_alongtrack[nrangetable] = store->RMB_sounding_along[i];
				if (nrangetable == 0
					|| fabs(store->MSS_table_acrosstrack[nrangetable])
						< acrosstracktablemin)
					{
					irangenadir = nrangetable;
					acrosstracktablemin = fabs(store->MSS_table_acrosstrack[nrangetable]);
					}
				nrangetable++;
				}
			}

		/* lay out port side */
		sample_start = store->MSS_table_range[irangenadir] * store->RSS_sample_rate;
		sample_end = MIN(store->MSS_table_range[0] * store->RSS_sample_rate,
					store->RSS_port_num_samples - 1);
		irange = irangenadir;
/* fprintf(stderr,"Port: irangenadir:%d sample_start:%d sample_end:%d\n",irangenadir,sample_start, sample_end); */
		for (i=sample_start;i<sample_end;i++)
			{
			range = ((double)i) / ((double)store->RSS_sample_rate);
			found = MB_NO;
			for (j=irange;j>0 && found == MB_NO;j--)
				{
				if (range >= store->MSS_table_range[j] && range < store->MSS_table_range[j-1])
					{
					irange = j;
					found = MB_YES;
					}
				}
			factor = (range - store->MSS_table_range[irange])
						/ (store->MSS_table_range[irange-1] - store->MSS_table_range[irange]);
			xtrackss = store->MSS_table_acrosstrack[irange]
					+ factor * (store->MSS_table_acrosstrack[irange-1] - store->MSS_table_acrosstrack[irange]);
			ltrackss = store->MSS_table_alongtrack[irange]
					+ factor * (store->MSS_table_alongtrack[irange-1] - store->MSS_table_alongtrack[irange]);
			kk = store->MSS_num_pixels / 2 + (int)(xtrackss / (*pixel_size));
			if (kk >= 0 && kk < store->MSS_num_pixels)
				{
				store->MSS_ss[kk]  += (double) store->RSS_port[i];
				store->MSS_ss_along[kk] += ltrackss;
				store->MSS_ss_cnt[kk]++;
				}
			}

		/* lay out starboard side */
		sample_start = store->MSS_table_range[irangenadir] * store->RSS_sample_rate;
		sample_end = MIN(store->MSS_table_range[nrangetable-1] * store->RSS_sample_rate,
					store->RSS_starboard_num_samples - 1);
		irange = irangenadir;
/* fprintf(stderr,"Starboard: irangenadir:%d sample_start:%d sample_end:%d\n",irangenadir,sample_start, sample_end); */
		for (i=sample_start;i<sample_end;i++)
			{
			range = ((double)i) / ((double)store->RSS_sample_rate);
			found = MB_NO;
			for (j=irange;j<nrangetable-1 && found == MB_NO;j++)
				{
				if (range >= store->MSS_table_range[j] && range < store->MSS_table_range[j+1])
					{
					irange = j;
					found = MB_YES;
					}
				}
			factor = (range - store->MSS_table_range[irange])
						/ (store->MSS_table_range[irange+1] - store->MSS_table_range[irange]);
			xtrackss = store->MSS_table_acrosstrack[irange]
					+ factor * (store->MSS_table_acrosstrack[irange+1] - store->MSS_table_acrosstrack[irange]);
			ltrackss = store->MSS_table_alongtrack[irange]
					+ factor * (store->MSS_table_alongtrack[irange+1] - store->MSS_table_alongtrack[irange]);
			kk = store->MSS_num_pixels / 2 + (int)(xtrackss / (*pixel_size));
			if (kk >= 0 && kk < store->MSS_num_pixels)
				{
				store->MSS_ss[kk]  += (double) store->RSS_starboard[i];
				store->MSS_ss_along[kk] += ltrackss;
				store->MSS_ss_cnt[kk]++;
				}
			}

		/* average the sidescan */
		first = store->MSS_num_pixels;
		last = -1;
		for (k=0;k<store->MSS_num_pixels;k++)
			{
			if (store->MSS_ss_cnt[k] > 0)
				{
				store->MSS_ss[k] /= store->MSS_ss_cnt[k];
				store->MSS_ss_along[k] /= store->MSS_ss_cnt[k];
				first = MIN(first, k);
				last = k;
				}
			else
				store->MSS_ss[k] = MB_SIDESCAN_NULL;
			}

		/* interpolate the sidescan */
		k1 = first;
		k2 = first;
		for (k=first+1;k<last;k++)
			{
			if (store->MSS_ss_cnt[k] <= 0)
				{
				if (k2 <= k)
					{
					k2 = k+1;
					while (store->MSS_ss_cnt[k2] <= 0 && k2 < last)
					    k2++;
					}
				if (k2 - k1 <= pixel_int_use)
					{
					store->MSS_ss[k] = store->MSS_ss[k1] + (store->MSS_ss[k2] - store->MSS_ss[k1])
						* ((double)(k - k1)) / ((double)(k2 - k1));
					store->MSS_ss_across[k] = (k - store->MSS_num_pixels / 2) * (*pixel_size);
					store->MSS_ss_along[k] = store->MSS_ss_along[k1]
								+ (store->MSS_ss_along[k2] - store->MSS_ss_along[k1])
									* ((double)(k - k1)) / ((double)(k2 - k1));
					}
				}
			else
				{
				k1 = k;
				 }
			}

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Sidescan regenerated in <%s>\n",
				function_name);
			fprintf(stderr,"dbg2       pixels_ss:  %d\n", store->MSS_num_pixels);
			for (i=0;i<store->MSS_num_pixels;i++)
			  fprintf(stderr,"dbg2       pixel:%4d  cnt:%3d  ss:%10f  xtrack:%10f  ltrack:%10f\n",
				i,store->MSS_ss_cnt[i],store->MSS_ss[i],
				store->MSS_ss_across[i],
				store->MSS_ss_along[i]);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       pixel_size:      %f\n",*pixel_size);
		fprintf(stderr,"dbg2       swath_width:     %f\n",*swath_width);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
