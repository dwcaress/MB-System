/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_simrad2.c	3.00	10/9/98
 *
 *    Copyright (c) 1998-2025 by
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
 * mbsys_simrad2.c contains the MBIO functions for handling data from
 * new (post-1997) Simrad multibeam sonars (e.g. EM120, EM300, EM3000).
 * The data formats associated with Simrad multibeams
 * (both old and new) include:
 *    MBSYS_SIMRAD formats (code in mbsys_simrad.c and mbsys_simrad.h):
 *      MBF_EMOLDRAW : MBIO ID 51 - Vendor EM1000, EM12S, EM12D, EM121
 *                   : MBIO ID 52 - aliased to 51
 *      MBF_EM12IFRM : MBIO ID 53 - IFREMER EM12S and EM12D
 *      MBF_EM12DARW : MBIO ID 54 - NERC EM12S
 *                   : MBIO ID 55 - aliased to 51
 *    MBSYS_SIMRAD2 formats (code in mbsys_simrad2.c and mbsys_simrad2.h):
 *      MBF_EM300RAW : MBIO ID 56 - Vendor EM3000, EM300, EM120
 *      MBF_EM300MBA : MBIO ID 57 - MBARI EM3000, EM300, EM120
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mbsys_simrad2.h"

/*--------------------------------------------------------------------*/
int mbsys_simrad2_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad2_struct), (void **)store_ptr, error);

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)*store_ptr;

	/* initialize everything */
	store->kind = MB_DATA_NONE;
	store->type = EM2_NONE;
	store->sonar = MBSYS_SIMRAD2_UNKNOWN;
	store->numberheads = 1;

	/* time stamp */
	store->date = 0;
	store->msec = 0;

	/* installation parameter values */
	store->par_date = 0; /* installation parameter date = year*10000 + month*100 + day
	             Feb 26, 1995 = 19950226 */
	store->par_msec = 0; /* installation parameter time since midnight in msec
	             08:12:51.234 = 29570234 */
	store->par_line_num = 0; /* survey line number */
	store->par_serial_1 = 0; /* system 1 serial number */
	store->par_serial_2 = 0; /* system 2 serial number */
	store->par_wlz = 0.0;    /* water line vertical location (m) */
	store->par_smh = 0;      /* system main head serial number */
	store->par_s1z = 0.0;    /* transducer 1 vertical location (m) */
	store->par_s1x = 0.0;    /* transducer 1 along location (m) */
	store->par_s1y = 0.0;    /* transducer 1 athwart location (m) */
	store->par_s1h = 0.0;    /* transducer 1 heading (deg) */
	store->par_s1r = 0.0;    /* transducer 1 roll (m) */
	store->par_s1p = 0.0;    /* transducer 1 pitch (m) */
	store->par_s1n = 0;      /* transducer 1 number of modules */
	store->par_s2z = 0.0;    /* transducer 2 vertical location (m) */
	store->par_s2x = 0.0;    /* transducer 2 along location (m) */
	store->par_s2y = 0.0;    /* transducer 2 athwart location (m) */
	store->par_s2h = 0.0;    /* transducer 2 heading (deg) */
	store->par_s2r = 0.0;    /* transducer 2 roll (m) */
	store->par_s2p = 0.0;    /* transducer 2 pitch (m) */
	store->par_s2n = 0;      /* transducer 2 number of modules */
	store->par_go1 = 0.0;    /* system (sonar head 1) gain offset */
	store->par_go2 = 0.0;    /* sonar head 2 gain offset */
	for (int i = 0; i < 16; i++) {
		store->par_tsv[i] = '\0'; /* transmitter (sonar head 1) software version */
		store->par_rsv[i] = '\0'; /* receiver (sonar head 2) software version */
		store->par_bsv[i] = '\0'; /* beamformer software version */
		store->par_psv[i] = '\0'; /* processing unit software version */
		store->par_osv[i] = '\0'; /* operator station software version */
	}
	store->par_dsd = 0.0;    /* depth sensor time delay (msec) */
	store->par_dso = 0.0;    /* depth sensor offset */
	store->par_dsf = 0.0;    /* depth sensor scale factor */
	store->par_dsh[0] = 'I'; /* depth sensor heave (IN or NI) */
	store->par_dsh[1] = 'N'; /* depth sensor heave (IN or NI) */
	store->par_aps = 0;      /* active position system number */
	store->par_p1m = 0;      /* position system 1 motion compensation (boolean) */
	store->par_p1t = 0;      /* position system 1 time stamp used
	                     (0=system time, 1=position input time) */
	store->par_p1z = 0.0;    /* position system 1 vertical location (m) */
	store->par_p1x = 0.0;    /* position system 1 along location (m) */
	store->par_p1y = 0.0;    /* position system 1 athwart location (m) */
	store->par_p1d = 0.0;    /* position system 1 time delay (sec) */
	for (int i = 0; i < 16; i++) {
		store->par_p1g[i] = '\0'; /* position system 1 geodetic datum */
	}
	strcpy(store->par_p1g, "WGS_84");
	store->par_p2m = 0;   /* position system 2 motion compensation (boolean) */
	store->par_p2t = 0;   /* position system 2 time stamp used
	                  (0=system time, 1=position input time) */
	store->par_p2z = 0.0; /* position system 2 vertical location (m) */
	store->par_p2x = 0.0; /* position system 2 along location (m) */
	store->par_p2y = 0.0; /* position system 2 athwart location (m) */
	store->par_p2d = 0.0; /* position system 2 time delay (sec) */
	for (int i = 0; i < 16; i++) {
		store->par_p2g[i] = '\0'; /* position system 2 geodetic datum */
	}
	store->par_p3m = 0;   /* position system 3 motion compensation (boolean) */
	store->par_p3t = 0;   /* position system 3 time stamp used
	                  (0=system time, 1=position input time) */
	store->par_p3z = 0.0; /* position system 3 vertical location (m) */
	store->par_p3x = 0.0; /* position system 3 along location (m) */
	store->par_p3y = 0.0; /* position system 3 athwart location (m) */
	store->par_p3d = 0.0; /* position system 3 time delay (sec) */
	for (int i = 0; i < 16; i++) {
		store->par_p3g[i] = '\0'; /* position system 3 geodetic datum */
	}
	store->par_msz = 0.0;    /* motion sensor vertical location (m) */
	store->par_msx = 0.0;    /* motion sensor along location (m) */
	store->par_msy = 0.0;    /* motion sensor athwart location (m) */
	store->par_mrp[0] = 'H'; /* motion sensor roll reference plane (HO or RP) */
	store->par_mrp[1] = 'O'; /* motion sensor roll reference plane (HO or RP) */
	store->par_msd = 0.0;    /* motion sensor time delay (sec) */
	store->par_msr = 0.0;    /* motion sensor roll offset (deg) */
	store->par_msp = 0.0;    /* motion sensor pitch offset (deg) */
	store->par_msg = 0.0;    /* motion sensor heading offset (deg) */
	store->par_gcg = 0.0;    /* gyro compass heading offset (deg) */
	for (int i = 0; i < 4; i++) {
		store->par_cpr[i] = '\0'; /* cartographic projection */
	}
	for (int i = 0; i < MBSYS_SIMRAD2_COMMENT_LENGTH; i++) {
		store->par_rop[i] = '\0'; /* responsible operator */
		store->par_sid[i] = '\0'; /* survey identifier */
		store->par_pll[i] = '\0'; /* survey line identifier (planned line number) */
		store->par_com[i] = '\0'; /* comment */
	}

	/* runtime parameter values */
	store->run_date = 0;       /* runtime parameter date = year*10000 + month*100 + day
	               Feb 26, 1995 = 19950226 */
	store->run_msec = 0;       /* runtime parameter time since midnight in msec
	               08:12:51.234 = 29570234 */
	store->run_ping_count = 0; /* ping counter */
	store->run_serial = 0;     /* system 1 or 2 serial number */
	store->run_status = 0;     /* system status */
	store->run_mode = 0;       /* system mode:
	               0 : nearfield (EM3000) or very shallow (EM300)
	               1 :	normal (EM3000) or shallow (EM300)
	               2 : medium (EM300)
	               3 : deep (EM300)
	               4 : very deep (EM300) */
	store->run_filter_id = 0;  /* filter identifier - the two lowest bits
	               indicate spike filter strength:
	               00 : off
	               01 : weak
	               10 : medium
	               11 : strong
	               bit 2 is set if the slope filter is on
	               bit 3 is set if the sidelobe filter is on
	               bit 4 is set if the range windows are expanded
	               bit 5 is set if the smoothing filter is on
	               bit	6 is set if the interference filter is on */
	store->run_min_depth = 0;  /* minimum depth (m) */
	store->run_max_depth = 0;  /* maximum depth (m) */
	store->run_absorption = 0; /* absorption coefficient (0.01 dB/km) */

	store->run_tran_pulse = 0;  /* transmit pulse length (usec) */
	store->run_tran_beam = 0;   /* transmit beamwidth (0.1 deg) */
	store->run_tran_pow = 0;    /* transmit power reduction (dB) */
	store->run_rec_beam = 0;    /* receiver beamwidth (0.1 deg) */
	store->run_rec_beam = 0;    /* receiver bandwidth (50 hz) */
	store->run_rec_gain = 0;    /* receiver fixed gain (dB) */
	store->run_tvg_cross = 0;   /* TVG law crossover angle (deg) */
	store->run_ssv_source = 0;  /* source of sound speed at transducer:
	                0 : from sensor
	                1 : manual
	                2 : from profile */
	store->run_max_swath = 0;   /* maximum swath width (m) */
	store->run_beam_space = 0;  /* beam spacing:
	                0 : determined by beamwidth (EM3000)
	                1 : equidistant
	                2 : equiangle */
	store->run_swath_angle = 0; /* coverage sector of swath (deg) */
	store->run_stab_mode = 0;   /* yaw and pitch stabilization mode:
	                The upper bit (bit 7) is set if pitch
	                stabilization is on.
	                The two lower bits are used to show yaw
	                stabilization mode as follows:
	                00 : none
	                01 : to survey line heading
	                10 : to mean vessel heading
	                11 : to manually entered heading */
	for (int i = 0; i < 4; i++) {
		store->run_spare[i] = '\0';
	}

	/* sound velocity profile */
	store->svp_use_date = 0; /* date at start of use
	             date = year*10000 + month*100 + day
	             Feb 26, 1995 = 19950226 */
	store->svp_use_msec = 0; /* time at start of use since midnight in msec
	             08:12:51.234 = 29570234 */
	store->svp_count = 0;    /* sequential counter or input identifier */
	store->svp_serial = 0;   /* system 1 serial number */
	store->svp_origin_date = 0; /* date at svp origin
	                date = year*10000 + month*100 + day
	                Feb 26, 1995 = 19950226 */
	store->svp_origin_msec = 0; /* time at svp origin since midnight in msec
	                08:12:51.234 = 29570234 */
	store->svp_num = 0;         /* number of svp entries */
	store->svp_depth_res = 0;   /* depth resolution (cm) */
	for (int i = 0; i < MBSYS_SIMRAD2_MAXSVP; i++) {
		store->svp_depth[i] = 0; /* depth of svp entries (according to svp_depth_res) */
		store->svp_vel[i] = 0;   /* sound speed of svp entries (0.1 m/sec) */
	}

	/* position */
	store->pos_date = 0;       /* position date = year*10000 + month*100 + day
	               Feb 26, 1995 = 19950226 */
	store->pos_msec = 0;       /* position time since midnight in msec
	               08:12:51.234 = 29570234 */
	store->pos_count = 0;      /* sequential counter */
	store->pos_serial = 0;     /* system 1 serial number */
	store->pos_latitude = 0;   /* latitude in decimal degrees * 20000000
	               (negative in southern hemisphere)
	               if valid, invalid = 0x7FFFFFFF */
	store->pos_longitude = 0;  /* longitude in decimal degrees * 10000000
	               (negative in western hemisphere)
	               if valid, invalid = 0x7FFFFFFF */
	store->pos_quality = 0;    /* measure of position fix quality (cm) */
	store->pos_speed = 0;      /* speed over ground (cm/sec) if valid,
	               invalid = 0xFFFF */
	store->pos_course = 0;     /* course over ground (0.01 deg) if valid,
	               invalid = 0xFFFF */
	store->pos_heading = 0;    /* heading (0.01 deg) if valid,
	                   invalid = 0xFFFF */
	store->pos_heave = 0;      /* heave from interpolation (0.01 m) */
	store->pos_roll = 0;       /* roll from interpolation (0.01 deg) */
	store->pos_pitch = 0;      /* pitch from interpolation (0.01 deg) */
	store->pos_system = 0;     /* position system number, type, and realtime use
	               - position system number given by two lowest bits
	               - fifth bit set means position must be derived
	               from input Simrad 90 datagram
	               - sixth bit set means valid time is that of
	               input datagram */
	store->pos_input_size = 0; /* number of bytes in input position datagram */
	for (int i = 0; i < 256; i++) {
		store->pos_input[i] = 0; /* position input datagram as received, minus
		         header and tail (such as NMEA 0183 $ and CRLF) */
	}

	/* height */
	store->hgt_date = 0;   /* height date = year*10000 + month*100 + day
	           Feb 26, 1995 = 19950226 */
	store->hgt_msec = 0;   /* height time since midnight in msec
	           08:12:51.234 = 29570234 */
	store->hgt_count = 0;  /* sequential counter */
	store->hgt_serial = 0; /* system 1 serial number */
	store->hgt_height = 0; /* height (0.01 m) */
	store->hgt_type = 0;   /* height type as given in input datagram or if
	           zero the height is derived from the GGK datagram
	           and is the height of the water level re the
	           vertical datum */

	/* tide */
	store->tid_date = 0;        /* tide date = year*10000 + month*100 + day
	                Feb 26, 1995 = 19950226 */
	store->tid_msec = 0;        /* tide time since midnight in msec
	                08:12:51.234 = 29570234 */
	store->tid_count = 0;       /* sequential counter */
	store->tid_serial = 0;      /* system 1 serial number */
	store->tid_origin_date = 0; /* tide input date = year*10000 + month*100 + day
	                Feb 26, 1995 = 19950226 */
	store->tid_origin_msec = 0; /* tide input time since midnight in msec
	                08:12:51.234 = 29570234 */
	store->tid_tide = 0;        /* tide offset (0.01 m) */

	/* clock */
	store->clk_date = 0;        /* system date = year*10000 + month*100 + day
	                Feb 26, 1995 = 19950226 */
	store->clk_msec = 0;        /* system time since midnight in msec
	                08:12:51.234 = 29570234 */
	store->clk_count = 0;       /* sequential counter */
	store->clk_serial = 0;      /* system 1 serial number */
	store->clk_origin_date = 0; /* external clock date = year*10000 + month*100 + day
	            Feb 26, 1995 = 19950226 */
	store->clk_origin_msec = 0; /* external clock time since midnight in msec
	                08:12:51.234 = 29570234 */
	store->clk_1_pps_use = 0; /* if 1 then the internal clock is synchronized
	              to an external 1 PPS signal, if 0 then not */

	/* pointer to extra parameters data structure */
	store->extraparameters = NULL;

	/* pointer to attitude data structure */
	store->attitude = NULL;

	/* pointer to heading data structure */
	store->heading = NULL;

	/* pointer to ssv data structure */
	store->ssv = NULL;

	/* pointer to tilt data structure */
	store->tilt = NULL;

	/* pointer to survey data structure */
	store->ping = NULL;
	store->ping2 = NULL;

	/* pointer to water column data structure */
	store->wc = NULL;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_simrad2_survey_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* allocate memory for data structure if needed */
	if (store->ping == NULL)
		status =
		    mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad2_ping_struct), (void **)&(store->ping), error);
	if (status == MB_SUCCESS && store->ping != NULL) {
		/* get data structure pointer */
		struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

		/* initialize everything */
		ping->png_date = 0;
		/* date = year*10000 + month*100 + day
		    Feb 26, 1995 = 19950226 */
		ping->png_msec = 0;
		/* time since midnight in msec
		    08:12:51.234 = 29570234 */
		ping->png_count = 0;
		/* sequential counter or input identifier */
		ping->png_serial = 0;
		/* system 1 or system 2 serial number */
		ping->png_latitude = EM2_INVALID_INT;
		/* latitude in decimal degrees * 20000000
		    (negative in southern hemisphere)
		    if valid, invalid = 0x7FFFFFFF */
		ping->png_longitude = EM2_INVALID_INT;
		/* longitude in decimal degrees * 10000000
		    (negative in western hemisphere)
		    if valid, invalid = 0x7FFFFFFF */
		ping->png_speed = 0;
		/* speed over ground (cm/sec) if valid,
		    invalid = 0xFFFF */
		ping->png_heading = 0;
		/* heading (0.01 deg) */
		ping->png_heave = 0;
		/* heave from interpolation (0.01 m) */
		ping->png_roll = 0;
		/* roll from interpolation (0.01 deg) */
		ping->png_pitch = 0;
		/* pitch from interpolation (0.01 deg) */
		ping->png_ssv = 0;
		/* sound speed at transducer (0.1 m/sec) */
		ping->png_xducer_depth = 0;
		/* transmit transducer depth (0.01 m)
		    - The transmit transducer depth plus the
		    depth offset multiplier times 65536 cm
		    should be added to the beam depths to
		    derive the depths re the water line.
		    The depth offset multiplier will usually
		    be zero, except when the EM3000 sonar
		    head is on an underwater vehicle at a
		    depth greater than about 650 m. Note that
		    the offset multiplier will be negative
		    (-1) if the actual heave is large enough
		    to bring the transmit transducer above
		    the water line. This may represent a valid
		    situation,  but may also be due to an
		    erroneously set installation depth of
		    the either transducer or the water line. */
		ping->png_offset_multiplier = 0;
		/* transmit transducer depth offset multiplier
		   - see note 7 above */

		/* beam data */
		ping->png_nbeams_max = 0;
		/* maximum number of beams possible */
		ping->png_nbeams = 0;
		/* number of valid beams */
		ping->png_depth_res = 0;
		/* depth resolution (0.01 m) */
		ping->png_distance_res = 0;
		/* x and y resolution (0.01 m) */
		ping->png_sample_rate = 0;
		/* sampling rate (Hz) OR depth difference between
		    sonar heads in EM3000D - see note 9 above */
		for (int i = 0; i < MBSYS_SIMRAD2_MAXBEAMS; i++) {
			ping->png_depth[i] = 0;
			/* depths in depth resolution units */
			ping->png_acrosstrack[i] = 0;
			/* acrosstrack distances in distance resolution units */
			ping->png_alongtrack[i] = 0;
			/* alongtrack distances in distance resolution units */
			ping->png_depression[i] = 0;
			/* Primary beam angles in one of two formats (see note 10 above)
			   1: Corrected format - gives beam depression angles
			    in 0.01 degree. These are the takeoff angles used
			    in raytracing calculations.
			   2: Uncorrected format - gives beam pointing angles
			    in 0.01 degree. These values are relative to
			    the transducer array and have not been corrected
			    for vessel motion. */
			ping->png_azimuth[i] = 0;
			/* Secondary beam angles in one of two formats (see note 10 above)
			   1: Corrected format - gives beam azimuth angles
			    in 0.01 degree. These values used to rotate sounding
			    position relative to the sonar after raytracing.
			   2: Uncorrected format - combines a flag indicating that
			    the angles are in the uncorrected format with
			    beam tilt angles. Values greater than
			    35999 indicate the uncorrected format is in use. The
			    beam tilt angles are given as (value - 54000) in
			    0.01 degree; the tilt angles give the tilt of the
			    transmitted ping due to compensation for vessel
			    motion. */
			ping->png_range[i] = 0;
			/* Ranges in one of two formats (see note 10 above):
			   1: Corrected format - the ranges are one way
			    travel times in time units defined as half
			    the inverse sampling rate.
			   2: Uncorrected format - the ranges are raw two
			    way travel times in time units defined as
			    half the inverse sampling rate. These values
			    have not been corrected for changes in the
			    heave during the ping cycle. */
			ping->png_quality[i] = 0;
			/* 0-254 */
			ping->png_window[i] = 0;
			/* samples/4 */
			ping->png_amp[i] = 0;
			/* 0.5 dB */
			ping->png_beam_num[i] = 0;
			/* beam 128 is first beam on
			    second head of EM3000D */
			ping->png_beamflag[i] = MB_FLAG_NULL;
			/* uses standard MB-System beamflags */
		}

		/* raw beam record */
		ping->png_raw1_read = false; /* flag indicating actual reading of old rawbeam record */
		ping->png_raw2_read = false; /* flag indicating actual reading of new rawbeam record */
		ping->png_raw_date = 0;
		/* date = year*10000 + month*100 + day
		    Feb 26, 1995 = 19950226 */
		ping->png_raw_msec = 0;
		/* time since midnight in msec
		    08:12:51.234 = 29570234 */
		ping->png_raw_count = 0;
		/* sequential counter or input identifier */
		ping->png_raw_serial = 0;
		/* system 1 or system 2 serial number */
		ping->png_raw_heading = 0;      /* heading (0.01 deg) */
		ping->png_raw_ssv = 0;          /* sound speed at transducer (0.1 m/sec) */
		ping->png_raw_xducer_depth = 0; /* transmit transducer depth (0.01 m) */
		ping->png_raw_nbeams_max = 0;   /* maximum number of beams possible */
		ping->png_raw_nbeams = 0;       /* number of valid beams */
		ping->png_raw_depth_res = 0;    /* depth resolution (0.01 m) */
		ping->png_raw_distance_res = 0; /* x and y resolution (0.01 m) */
		ping->png_raw_sample_rate = 0;  /* sampling rate (Hz) */
		ping->png_raw_status = 0;       /* status from PU/TRU */
		ping->png_raw_nbeams = 0; /* number of raw travel times and angles
		              - nonzero only if raw beam record read */
		ping->png_raw_rangenormal = 0;        /* normal incidence range (meters) */
		ping->png_raw_normalbackscatter = 0;  /* normal incidence backscatter (dB) (-60 to +9) */
		ping->png_raw_obliquebackscatter = 0; /* oblique incidence backscatter (dB) (-60 to +9) */
		ping->png_raw_fixedgain = 0;          /* fixed gain (dB) (0 to 30) */
		ping->png_raw_txpower = 0;            /* transmit power (dB) (0, -10, or -20) */
		ping->png_raw_mode = 0;               /* sonar mode:
		                              0 : very shallow
		                              1 : shallow
		                              2 : medium
		                              3 : deep
		                              4 : very deep
		                              5 : extra deep */
		ping->png_raw_coverage = 0;           /* swath width (degrees) (10 to 150 degrees) */
		ping->png_raw_yawstabheading = 0;     /* yaw stabilization heading (0.01 degrees) */
		ping->png_raw_ntx = 0;                /* number of TX pulses (1 to 9) */
		for (int i = 0; i < MBSYS_SIMRAD2_MAXTX; i++) {
			ping->png_raw_txlastbeam[i] = 0;  /* last beam number in this TX pulse */
			ping->png_raw_txtiltangle[i] = 0; /* tilt angle (0.01 deg) */
			ping->png_raw_txheading[i] = 0;   /* heading (0.01 deg) */
			ping->png_raw_txroll[i] = 0;      /* roll (0.01 deg) */
			ping->png_raw_txpitch[i] = 0;     /* pitch angle (0.01 deg) */
			ping->png_raw_txheave[i] = 0;     /* heave (0.01 m) */
		}
		for (int i = 0; i < MBSYS_SIMRAD2_MAXBEAMS; i++) {
			ping->png_raw_rxrange[i] = 0;
			/* Ranges as raw two way travel times in time
			    units defined as one-fourth the inverse
			    sampling rate. These values have not
			    been corrected for changes in the
			    heave during the ping cycle. */
			ping->png_raw_rxquality[i] = 0; /* beam quality flag */
			ping->png_raw_rxwindow[i] = 0;  /* length of detection window */
			ping->png_raw_rxamp[i] = 0;     /* 0.5 dB */
			ping->png_raw_rxbeam_num[i] = 0;
			/* beam 128 is first beam on
			    second head of EM3000D */
			ping->png_raw_rxpointangle[i] = 0;
			/* Raw beam pointing angles in 0.01 degree,
			    positive to port.
			    These values are relative to the transducer
			    array and have not been corrected
			    for vessel motion. */
			ping->png_raw_rxtiltangle[i] = 0;
			/* Raw transmit tilt angles in 0.01 degree,
			    positive forward.
			    These values are relative to the transducer
			    array and have not been corrected
			    for vessel motion. */
			ping->png_raw_rxheading[i] = 0; /* heading (0.01 deg) */
			ping->png_raw_rxroll[i] = 0;    /* roll (0.01 deg) */
			ping->png_raw_rxpitch[i] = 0;   /* pitch angle (0.01 deg) */
			ping->png_raw_rxheave[i] = 0;   /* heave (0.01 m) */
		}

		/* raw travel time and angle data version 3 */
		ping->png_raw3_read = 0;   /* flag indicating actual reading of newer rawbeam record */
		ping->png_raw3_date = 0;   /* date = year*10000 + month*100 + day
		           Feb 26, 1995 = 19950226 */
		ping->png_raw3_msec = 0;   /* time since midnight in msec
		           08:12:51.234 = 29570234 */
		ping->png_raw3_count = 0;  /* sequential counter or input identifier */
		ping->png_raw3_serial = 0; /* system 1 or system 2 serial number */
		ping->png_raw3_ntx = 0;    /* number of TX pulses (1 to 9) */
		ping->png_raw3_nbeams = 0; /* number of raw travel times and angles
		           - nonzero only if raw beam record read */
		ping->png_raw3_sample_rate = 0;  /* sampling rate (Hz or 0.01 Hz) */
		ping->png_raw3_xducer_depth = 0; /* transmit transducer depth (0.01 m) */
		ping->png_raw3_ssv = 0;          /* sound speed at transducer (0.1 m/sec) */
		ping->png_raw3_nbeams_max = 0;   /* maximum number of beams possible */
		for (int i = 0; i < MBSYS_SIMRAD2_MAXTX; i++) {
			ping->png_raw3_txtiltangle[i] = 0;    /* tilt angle (0.01 deg) */
			ping->png_raw3_txfocus[i] = 0;        /* focus range (0.1 m)
			                             0 = no focus */
			ping->png_raw3_txsignallength[i] = 0; /* signal length (usec) */
			ping->png_raw3_txoffset[i] = 0;       /* transmit time offset (usec) */
			ping->png_raw3_txcenter[i] = 0;       /* center frequency (Hz) */
			ping->png_raw3_txbandwidth[i] = 0;    /* bandwidth (10 Hz) */
			ping->png_raw3_txwaveform[i] = 0; /* signal waveform identifier
			                          0 = CW, 1 = FM */
			ping->png_raw3_txsector[i] = 0;   /* transmit sector number (0-19) */
		}
		for (int i = 0; i < MBSYS_SIMRAD2_MAXBEAMS; i++) {
			ping->png_raw3_rxpointangle[i] = 0;
			;
			/* Raw beam pointing angles in 0.01 degree,
			    positive to port.
			    These values are relative to the transducer
			    array and have not been corrected
			    for vessel motion. */
			ping->png_raw3_rxrange[i] = 0;
			; /* Ranges (0.25 samples) */
			ping->png_raw3_rxsector[i] = 0;
			; /* transmit sector identifier */
			ping->png_raw3_rxamp[i] = 0;
			; /* 0.5 dB */
			ping->png_raw3_rxquality[i] = 0;
			; /* beam quality flag */
			ping->png_raw3_rxwindow[i] = 0;
			; /* length of detection window */
			ping->png_raw3_rxbeam_num[i] = 0;
			;
			/* beam 128 is first beam on
			    second head of EM3000D */
			ping->png_raw3_rxspare[i] = 0;
			; /* spare */
		}

		/* sidescan */
		ping->png_ss_read = false;
		/* flag indicating actual reading of sidescan record */
		ping->png_ss_count = 0;
		/* sequential counter or input identifier */
		ping->png_ss_serial = 0;
		/* system 1 or system 2 serial number */
		ping->png_max_range = 0;
		/* max range of ping in number of samples */
		ping->png_r_zero = 0;
		/* range to normal incidence used in TVG
		    (R0 predicted) in samples */
		ping->png_r_zero_corr = 0;
		/* range to normal incidence used to correct
		    sample amplitudes in number of samples */
		ping->png_tvg_start = 0;
		/* start sample of TVG ramp if not enough
		    dynamic range (0 otherwise) */
		ping->png_tvg_stop = 0; /* stop sample of TVG ramp if not enough
		                            dynamic range (0 otherwise) */
		ping->png_bsn = 0;
		/* normal incidence backscatter (BSN) in dB */
		ping->png_bso = 0;
		/* oblique incidence backscatter (BSO) in dB */
		ping->png_tx = 0;
		/* Tx beamwidth in 0.1 degree */
		ping->png_tvg_crossover = 0;
		/* TVG law crossover angle in degrees */
		ping->png_nbeams_ss = 0;
		/* number of beams with sidescan */
		for (int i = 0; i < MBSYS_SIMRAD2_MAXBEAMS; i++) {
			ping->png_beam_index[i] = 0;
			/* beam index number */
			ping->png_sort_direction[i] = 0;
			/* sorting direction - first sample in beam has lowest
			    range if 1, highest if -1. */
			ping->png_beam_samples[i] = 0;
			/* number of sidescan samples derived from
			    each beam */
			ping->png_start_sample[i] = 0;
			/* start sample number */
			ping->png_center_sample[i] = 0;
			/* center sample number */
		}
		for (int i = 0; i < MBSYS_SIMRAD2_MAXRAWPIXELS; i++) {
			ping->png_ssraw[i] = EM2_INVALID_AMP;
			/* the raw sidescan ordered port to starboard */
		}
		ping->png_pixel_size = 0;
		ping->png_pixels_ss = 0;
		for (int i = 0; i < MBSYS_SIMRAD2_MAXPIXELS; i++) {
			ping->png_ss[i] = EM2_INVALID_AMP;
			/* the processed sidescan ordered port to starboard */
			ping->png_ssalongtrack[i] = EM2_INVALID_AMP;
			/* the processed sidescan alongtrack distances
			    in distance resolution units */
		}
	}

	/* allocate memory for second data structure if needed */
	if (store->ping2 == NULL && store->sonar == MBSYS_SIMRAD2_EM3002) {
		if ((status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad2_ping_struct), (void **)&(store->ping2),
		                         error)) == MB_SUCCESS) {

			/* get data structure pointer */
			struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping2;

			/* initialize everything */
			ping->png_date = 0;
			/* date = year*10000 + month*100 + day
			    Feb 26, 1995 = 19950226 */
			ping->png_msec = 0;
			/* time since midnight in msec
			    08:12:51.234 = 29570234 */
			ping->png_count = 0;
			/* sequential counter or input identifier */
			ping->png_serial = 0;
			/* system 1 or system 2 serial number */
			ping->png_latitude = EM2_INVALID_INT;
			/* latitude in decimal degrees * 20000000
			    (negative in southern hemisphere)
			    if valid, invalid = 0x7FFFFFFF */
			ping->png_longitude = EM2_INVALID_INT;
			/* longitude in decimal degrees * 10000000
			    (negative in western hemisphere)
			    if valid, invalid = 0x7FFFFFFF */
			ping->png_speed = 0;
			/* speed over ground (cm/sec) if valid,
			    invalid = 0xFFFF */
			ping->png_heading = 0;
			/* heading (0.01 deg) */
			ping->png_heave = 0;
			/* heave from interpolation (0.01 m) */
			ping->png_roll = 0;
			/* roll from interpolation (0.01 deg) */
			ping->png_pitch = 0;
			/* pitch from interpolation (0.01 deg) */
			ping->png_ssv = 0;
			/* sound speed at transducer (0.1 m/sec) */
			ping->png_xducer_depth = 0;
			/* transmit transducer depth (0.01 m)
			    - The transmit transducer depth plus the
			    depth offset multiplier times 65536 cm
			    should be added to the beam depths to
			    derive the depths re the water line.
			    The depth offset multiplier will usually
			    be zero, except when the EM3000 sonar
			    head is on an underwater vehicle at a
			    depth greater than about 650 m. Note that
			    the offset multiplier will be negative
			    (-1) if the actual heave is large enough
			    to bring the transmit transducer above
			    the water line. This may represent a valid
			    situation,  but may also be due to an
			    erroneously set installation depth of
			    the either transducer or the water line. */
			ping->png_offset_multiplier = 0;
			/* transmit transducer depth offset multiplier
			   - see note 7 above */

			/* beam data */
			ping->png_nbeams_max = 0;
			/* maximum number of beams possible */
			ping->png_nbeams = 0;
			/* number of valid beams */
			ping->png_depth_res = 0;
			/* depth resolution (0.01 m) */
			ping->png_distance_res = 0;
			/* x and y resolution (0.01 m) */
			ping->png_sample_rate = 0;
			/* sampling rate (Hz) OR depth difference between
			    sonar heads in EM3000D - see note 9 above */
			for (int i = 0; i < MBSYS_SIMRAD2_MAXBEAMS; i++) {
				ping->png_depth[i] = 0;
				/* depths in depth resolution units */
				ping->png_acrosstrack[i] = 0;
				/* acrosstrack distances in distance resolution units */
				ping->png_alongtrack[i] = 0;
				/* alongtrack distances in distance resolution units */
				ping->png_depression[i] = 0;
				/* Primary beam angles in one of two formats (see note 10 above)
				   1: Corrected format - gives beam depression angles
				        in 0.01 degree. These are the takeoff angles used
				    in raytracing calculations.
				   2: Uncorrected format - gives beam pointing angles
				        in 0.01 degree. These values are relative to
				    the transducer array and have not been corrected
				    for vessel motion. */
				ping->png_azimuth[i] = 0;
				/* Secondary beam angles in one of two formats (see note 10 above)
				   1: Corrected format - gives beam azimuth angles
				        in 0.01 degree. These values used to rotate sounding
				    position relative to the sonar after raytracing.
				   2: Uncorrected format - combines a flag indicating that
				        the angles are in the uncorrected format with
				    beam tilt angles. Values greater than
				    35999 indicate the uncorrected format is in use. The
				    beam tilt angles are given as (value - 54000) in
				    0.01 degree; the tilt angles give the tilt of the
				    transmitted ping due to compensation for vessel
				    motion. */
				ping->png_range[i] = 0;
				/* Ranges in one of two formats (see note 10 above):
				   1: Corrected format - the ranges are one way
				        travel times in time units defined as half
				    the inverse sampling rate.
				   2: Uncorrected format - the ranges are raw two
				        way travel times in time units defined as
				    half the inverse sampling rate. These values
				    have not been corrected for changes in the
				    heave during the ping cycle. */
				ping->png_quality[i] = 0;
				/* 0-254 */
				ping->png_window[i] = 0;
				/* samples/4 */
				ping->png_amp[i] = 0;
				/* 0.5 dB */
				ping->png_beam_num[i] = 0;
				/* beam 128 is first beam on
				    second head of EM3000D */
				ping->png_beamflag[i] = MB_FLAG_NULL;
				/* uses standard MB-System beamflags */
			}

			/* raw beam record */
			ping->png_raw1_read = false; /* flag indicating actual reading of old rawbeam record */
			ping->png_raw2_read = false; /* flag indicating actual reading of new rawbeam record */
			ping->png_raw_date = 0;
			/* date = year*10000 + month*100 + day
			    Feb 26, 1995 = 19950226 */
			ping->png_raw_msec = 0;
			/* time since midnight in msec
			    08:12:51.234 = 29570234 */
			ping->png_raw_count = 0;
			/* sequential counter or input identifier */
			ping->png_raw_serial = 0;
			/* system 1 or system 2 serial number */
			ping->png_raw_heading = 0;      /* heading (0.01 deg) */
			ping->png_raw_ssv = 0;          /* sound speed at transducer (0.1 m/sec) */
			ping->png_raw_xducer_depth = 0; /* transmit transducer depth (0.01 m) */
			ping->png_raw_nbeams_max = 0;   /* maximum number of beams possible */
			ping->png_raw_nbeams = 0;       /* number of valid beams */
			ping->png_raw_depth_res = 0;    /* depth resolution (0.01 m) */
			ping->png_raw_distance_res = 0; /* x and y resolution (0.01 m) */
			ping->png_raw_sample_rate = 0;  /* sampling rate (Hz) */
			ping->png_raw_status = 0;       /* status from PU/TRU */
			ping->png_raw_nbeams = 0; /* number of raw travel times and angles
			              - nonzero only if raw beam record read */
			ping->png_raw_rangenormal = 0;        /* normal incidence range (meters) */
			ping->png_raw_normalbackscatter = 0;  /* normal incidence backscatter (dB) (-60 to +9) */
			ping->png_raw_obliquebackscatter = 0; /* oblique incidence backscatter (dB) (-60 to +9) */
			ping->png_raw_fixedgain = 0;          /* fixed gain (dB) (0 to 30) */
			ping->png_raw_txpower = 0;            /* transmit power (dB) (0, -10, or -20) */
			ping->png_raw_mode = 0;               /* sonar mode:
			                              0 : very shallow
			                              1 : shallow
			                              2 : medium
			                              3 : deep
			                              4 : very deep
			                              5 : extra deep */
			ping->png_raw_coverage = 0;           /* swath width (degrees) (10 to 150 degrees) */
			ping->png_raw_yawstabheading = 0;     /* yaw stabilization heading (0.01 degrees) */
			ping->png_raw_ntx = 0;                /* number of TX pulses (1 to 9) */
			for (int i = 0; i < MBSYS_SIMRAD2_MAXTX; i++) {
				ping->png_raw_txlastbeam[i] = 0;  /* last beam number in this TX pulse */
				ping->png_raw_txtiltangle[i] = 0; /* tilt angle (0.01 deg) */
				ping->png_raw_txheading[i] = 0;   /* heading (0.01 deg) */
				ping->png_raw_txroll[i] = 0;      /* roll (0.01 deg) */
				ping->png_raw_txpitch[i] = 0;     /* pitch angle (0.01 deg) */
				ping->png_raw_txheave[i] = 0;     /* heave (0.01 m) */
			}
			for (int i = 0; i < MBSYS_SIMRAD2_MAXBEAMS; i++) {
				ping->png_raw_rxrange[i] = 0;
				/* Ranges as raw two way travel times in time
				    units defined as one-fourth the inverse
				    sampling rate. These values have not
				    been corrected for changes in the
				    heave during the ping cycle. */
				ping->png_raw_rxquality[i] = 0; /* beam quality flag */
				ping->png_raw_rxwindow[i] = 0;  /* length of detection window */
				ping->png_raw_rxamp[i] = 0;     /* 0.5 dB */
				ping->png_raw_rxbeam_num[i] = 0;
				/* beam 128 is first beam on
				    second head of EM3000D */
				ping->png_raw_rxpointangle[i] = 0;
				/* Raw beam pointing angles in 0.01 degree,
				    positive to port.
				    These values are relative to the transducer
				    array and have not been corrected
				    for vessel motion. */
				ping->png_raw_rxtiltangle[i] = 0;
				/* Raw transmit tilt angles in 0.01 degree,
				    positive forward.
				    These values are relative to the transducer
				    array and have not been corrected
				    for vessel motion. */
				ping->png_raw_rxheading[i] = 0; /* heading (0.01 deg) */
				ping->png_raw_rxroll[i] = 0;    /* roll (0.01 deg) */
				ping->png_raw_rxpitch[i] = 0;   /* pitch angle (0.01 deg) */
				ping->png_raw_rxheave[i] = 0;   /* heave (0.01 m) */
			}

			/* raw travel time and angle data version 3 */
			ping->png_raw3_read = 0;   /* flag indicating actual reading of newer rawbeam record */
			ping->png_raw3_date = 0;   /* date = year*10000 + month*100 + day
			           Feb 26, 1995 = 19950226 */
			ping->png_raw3_msec = 0;   /* time since midnight in msec
			           08:12:51.234 = 29570234 */
			ping->png_raw3_count = 0;  /* sequential counter or input identifier */
			ping->png_raw3_serial = 0; /* system 1 or system 2 serial number */
			ping->png_raw3_ntx = 0;    /* number of TX pulses (1 to 9) */
			ping->png_raw3_nbeams = 0; /* number of raw travel times and angles
			           - nonzero only if raw beam record read */
			ping->png_raw3_sample_rate = 0;  /* sampling rate (Hz or 0.01 Hz) */
			ping->png_raw3_xducer_depth = 0; /* transmit transducer depth (0.01 m) */
			ping->png_raw3_ssv = 0;          /* sound speed at transducer (0.1 m/sec) */
			ping->png_raw3_nbeams_max = 0;   /* maximum number of beams possible */
			for (int i = 0; i < MBSYS_SIMRAD2_MAXTX; i++) {
				ping->png_raw3_txtiltangle[i] = 0;    /* tilt angle (0.01 deg) */
				ping->png_raw3_txfocus[i] = 0;        /* focus range (0.1 m)
				                             0 = no focus */
				ping->png_raw3_txsignallength[i] = 0; /* signal length (usec) */
				ping->png_raw3_txoffset[i] = 0;       /* transmit time offset (usec) */
				ping->png_raw3_txcenter[i] = 0;       /* center frequency (Hz) */
				ping->png_raw3_txbandwidth[i] = 0;    /* bandwidth (10 Hz) */
				ping->png_raw3_txwaveform[i] = 0; /* signal waveform identifier
				                          0 = CW, 1 = FM */
				ping->png_raw3_txsector[i] = 0; /* transmit sector number (0-19) */
			}
			for (int i = 0; i < MBSYS_SIMRAD2_MAXBEAMS; i++) {
				ping->png_raw3_rxpointangle[i] = 0;
				;
				/* Raw beam pointing angles in 0.01 degree,
				    positive to port.
				    These values are relative to the transducer
				    array and have not been corrected
				    for vessel motion. */
				ping->png_raw3_rxrange[i] = 0;
				; /* Ranges (0.25 samples) */
				ping->png_raw3_rxsector[i] = 0;
				; /* transmit sector identifier */
				ping->png_raw3_rxamp[i] = 0;
				; /* 0.5 dB */
				ping->png_raw3_rxquality[i] = 0;
				; /* beam quality flag */
				ping->png_raw3_rxwindow[i] = 0;
				; /* length of detection window */
				ping->png_raw3_rxbeam_num[i] = 0;
				;
				/* beam 128 is first beam on
				    second head of EM3000D */
				ping->png_raw3_rxspare[i] = 0;
				; /* spare */
			}

			/* sidescan */
			ping->png_ss_read = false;
			/* flag indicating actual reading of sidescan record */
			ping->png_ss_count = 0;
			/* sequential counter or input identifier */
			ping->png_ss_serial = 0;
			/* system 1 or system 2 serial number */
			ping->png_max_range = 0;
			/* max range of ping in number of samples */
			ping->png_r_zero = 0;
			/* range to normal incidence used in TVG
			    (R0 predicted) in samples */
			ping->png_r_zero_corr = 0;
			/* range to normal incidence used to correct
			    sample amplitudes in number of samples */
			ping->png_tvg_start = 0;
			/* start sample of TVG ramp if not enough
			    dynamic range (0 otherwise) */
			ping->png_tvg_stop = 0; /* stop sample of TVG ramp if not enough
			                            dynamic range (0 otherwise) */
			ping->png_bsn = 0;
			/* normal incidence backscatter (BSN) in dB */
			ping->png_bso = 0;
			/* oblique incidence backscatter (BSO) in dB */
			ping->png_tx = 0;
			/* Tx beamwidth in 0.1 degree */
			ping->png_tvg_crossover = 0;
			/* TVG law crossover angle in degrees */
			ping->png_nbeams_ss = 0;
			/* number of beams with sidescan */
			for (int i = 0; i < MBSYS_SIMRAD2_MAXBEAMS; i++) {
				ping->png_beam_index[i] = 0;
				/* beam index number */
				ping->png_sort_direction[i] = 0;
				/* sorting direction - first sample in beam has lowest
				    range if 1, highest if -1. */
				ping->png_beam_samples[i] = 0;
				/* number of sidescan samples derived from
				    each beam */
				ping->png_start_sample[i] = 0;
				/* start sample number */
				ping->png_center_sample[i] = 0;
				/* center sample number */
			}
			for (int i = 0; i < MBSYS_SIMRAD2_MAXRAWPIXELS; i++) {
				ping->png_ssraw[i] = EM2_INVALID_AMP;
				/* the raw sidescan ordered port to starboard */
			}
			ping->png_pixel_size = 0;
			ping->png_pixels_ss = 0;
			for (int i = 0; i < MBSYS_SIMRAD2_MAXPIXELS; i++) {
				ping->png_ss[i] = EM2_INVALID_AMP;
				/* the processed sidescan ordered port to starboard */
				ping->png_ssalongtrack[i] = EM2_INVALID_AMP;
				/* the processed sidescan alongtrack distances
				    in distance resolution units */
			}
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_simrad2_wc_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* allocate memory for data structure if needed */
	if (store->wc == NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad2_watercolumn_struct), (void **)&(store->wc),
		                    error);

	if (status == MB_SUCCESS) {
		/* get data structure pointer */
		struct mbsys_simrad2_watercolumn_struct *wc = (struct mbsys_simrad2_watercolumn_struct *)store->wc;

		/* initialize everything */
		wc->wtc_date = 0;       /* date = year*10000 + month*100 + day
		                Feb 26, 1995 = 19950226 */
		wc->wtc_msec = 0;       /* time since midnight in msec
		                08:12:51.234 = 29570234 */
		wc->wtc_count = 0;      /* sequential counter or input identifier */
		wc->wtc_serial = 0;     /* system 1 or system 2 serial number */
		wc->wtc_ndatagrams = 0; /* number of datagrams used to represent
		                the water column for this ping */
		wc->wtc_datagram = 0;   /* number this datagram */
		wc->wtc_ntx = 0;        /* number of transmit sectors */
		wc->wtc_nrx = 0;        /* number of receive beams */
		wc->wtc_nbeam = 0;      /* number of beams in this datagram */
		wc->wtc_ssv = 0;        /* sound speed at transducer (0.1 m/sec) */
		wc->wtc_sfreq = 0;      /* sampling frequency (0.01 Hz) */
		wc->wtc_heave = 0;      /* tx time heave at transducer (0.01 m) */
		wc->wtc_spare1 = 0;     /* spare */
		wc->wtc_spare2 = 0;     /* spare */
		wc->wtc_spare3 = 0;     /* spare */
		for (int i = 0; i < MBSYS_SIMRAD2_MAXTX; i++) {
			wc->wtc_txtiltangle[i] = 0; /* tilt angle (0.01 deg) */
			wc->wtc_txcenter[i] = 0;    /* center frequency (Hz) */
			wc->wtc_txsector[i] = 0;    /* transmit sector number (0-19) */
		}
		for (int i = 0; i < MBSYS_SIMRAD2_MAXBEAMS; i++) {
			wc->beam[i].wtc_rxpointangle = 0; /* Beam pointing angles in 0.01 degree,
			                      positive to port. These values are roll stabilized. */
			wc->beam[i].wtc_start_sample = 0; /* start sample number */
			wc->beam[i].wtc_beam_samples = 0; /* number of water column samples derived from
			                      each beam */
			wc->beam[i].wtc_sector = 0; /* transmit sector identifier */
			wc->beam[i].wtc_beam = 0;   /* beam 128 is first beam on
			                    second head of EM3000D */
			for (int j = 0; j < MBSYS_SIMRAD2_MAXRAWPIXELS; j++)
				wc->beam[i].wtc_amp[j] = 0; /* water column amplitude (dB) */
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_simrad2_extraparameters_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* allocate memory for data structure if needed */
	if (store->extraparameters == NULL) {
		status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad2_extraparameters_struct),
		                    (void **)&(store->extraparameters), error);
		if (status == MB_SUCCESS)
			memset(store->extraparameters, 0, sizeof(struct mbsys_simrad2_extraparameters_struct));
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_simrad2_attitude_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* allocate memory for data structure if needed */
	if (store->attitude == NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad2_attitude_struct),
		                    (void **)&(store->attitude), error);

	if (status == MB_SUCCESS) {

		/* get data structure pointer */
		struct mbsys_simrad2_attitude_struct *attitude = (struct mbsys_simrad2_attitude_struct *)store->attitude;

		/* initialize everything */
		attitude->att_date = 0;
		/* date = year*10000 + month*100 + day
		    Feb 26, 1995 = 19950226 */
		attitude->att_msec = 0;
		/* time since midnight in msec
		    08:12:51.234 = 29570234 */
		attitude->att_count = 0;
		/* sequential counter or input identifier */
		attitude->att_serial = 0;
		/* system 1 or system 2 serial number */
		attitude->att_ndata = 0;
		/* number of attitude data */
		for (int i = 0; i < MBSYS_SIMRAD2_MAXATTITUDE; i++) {
			attitude->att_time[i] = 0;
			/* time since record start (msec) */
			attitude->att_sensor_status[i] = 0;
			/* see note 12 above */
			attitude->att_roll[i] = 0;
			/* roll (0.01 degree) */
			attitude->att_pitch[i] = 0;
			/* pitch (0.01 degree) */
			attitude->att_heave[i] = 0;
			/* heave (cm) */
			attitude->att_heading[i] = 0;
			/* heading (0.01 degree) */
		}
		attitude->att_heading_status = 0;
		/* heading status (0=inactive) */
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_heading_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* allocate memory for data structure if needed */
	if (store->heading == NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad2_heading_struct), (void **)&(store->heading),
		                    error);

	if (status == MB_SUCCESS) {
		/* get data structure pointer */
		struct mbsys_simrad2_heading_struct *heading = (struct mbsys_simrad2_heading_struct *)store->heading;

		/* initialize everything */
		heading->hed_date = 0;
		/* date = year*10000 + month*100 + day
		    Feb 26, 1995 = 19950226 */
		heading->hed_msec = 0;
		/* time since midnight in msec
		    08:12:51.234 = 29570234 */
		heading->hed_count = 0;
		/* sequential counter or input identifier */
		heading->hed_serial = 0;
		/* system 1 or system 2 serial number */
		heading->hed_ndata = 0;
		/* number of heading data */
		for (int i = 0; i < MBSYS_SIMRAD2_MAXHEADING; i++) {
			heading->hed_time[i] = 0;
			/* time since record start (msec) */
			heading->hed_heading[i] = 0;
			/* heading (0.01 degree) */
		}
		heading->hed_heading_status = 0;
		/* heading status (0=inactive) */
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_ssv_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* allocate memory for data structure if needed */
	if (store->ssv == NULL)
		status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad2_ssv_struct), (void **)&(store->ssv), error);

	if (status == MB_SUCCESS) {
		/* get data structure pointer */
		struct mbsys_simrad2_ssv_struct *ssv = (struct mbsys_simrad2_ssv_struct *)store->ssv;

		/* initialize everything */
		ssv->ssv_date = 0;
		/* date = year*10000 + month*100 + day
		    Feb 26, 1995 = 19950226 */
		ssv->ssv_msec = 0;
		/* time since midnight in msec
		    08:12:51.234 = 29570234 */
		ssv->ssv_count = 0;
		/* sequential counter or input identifier */
		ssv->ssv_serial = 0;
		/* system 1 or system 2 serial number */
		ssv->ssv_ndata = 0;
		/* number of ssv data */
		for (int i = 0; i < MBSYS_SIMRAD2_MAXTILT; i++) {
			ssv->ssv_time[i] = 0;
			/* time since record start (msec) */
			ssv->ssv_ssv[i] = 0;
			/* ssv (0.1 m/s) */
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_tilt_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* allocate memory for data structure if needed */
	if (store->tilt == NULL)
		status =
		    mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_simrad2_tilt_struct), (void **)&(store->tilt), error);

	if (status == MB_SUCCESS) {
		/* get data structure pointer */
		struct mbsys_simrad2_tilt_struct *tilt = (struct mbsys_simrad2_tilt_struct *)store->tilt;

		/* initialize everything */
		tilt->tlt_date = 0;
		/* date = year*10000 + month*100 + day
		    Feb 26, 1995 = 19950226 */
		tilt->tlt_msec = 0;
		/* time since midnight in msec
		    08:12:51.234 = 29570234 */
		tilt->tlt_count = 0;
		/* sequential counter or input identifier */
		tilt->tlt_serial = 0;
		/* system 1 or system 2 serial number */
		tilt->tlt_ndata = 0;
		/* number of tilt data */
		for (int i = 0; i < MBSYS_SIMRAD2_MAXTILT; i++) {
			tilt->tlt_time[i] = 0;
			/* time since record start (msec) */
			tilt->tlt_tilt[i] = 0;
			/* tilt + forward (0.01 deg) */
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
	}

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)*store_ptr;

	int status = MB_SUCCESS;

	/* deallocate memory for survey data structure */
	if (store->ping != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->ping), error);

	/* deallocate memory for survey data structure */
	if (store->ping2 != NULL)
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->ping2), error);

	/* deallocate memory for extraparameters data structure */
	if (store->extraparameters != NULL) {
		struct mbsys_simrad2_extraparameters_struct *extraparameters =
                    (struct mbsys_simrad2_extraparameters_struct *)store->extraparameters;
		if (extraparameters->xtr_data != NULL)
			status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(extraparameters->xtr_data), error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->extraparameters), error);
	}

	/* deallocate memory for water column data structure */
	if (store->wc != NULL)
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->wc), error);

	/* deallocate memory for attitude data structure */
	if (store->attitude != NULL)
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->attitude), error);

	/* deallocate memory for heading data structure */
	if (store->heading != NULL)
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->heading), error);

	/* deallocate memory for ssv data structure */
	if (store->ssv != NULL)
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->ssv), error);

	/* deallocate memory for tilt data structure */
	if (store->tilt != NULL)
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->tilt), error);

	/* deallocate memory for data structure */
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_zero_ss(int verbose, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to data descriptor */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;
	struct mbsys_simrad2_ping_struct *ping;
	if (store != NULL)
		ping = (struct mbsys_simrad2_ping_struct *)store->ping;

	/* initialize all sidescan stuff to zeros */
	if (store->ping != NULL) {
		ping->png_ss_date = 0;
		/* date = year*10000 + month*100 + day
		    Feb 26, 1995 = 19950226 */
		ping->png_ss_msec = 0;
		/* time since midnight in msec
		    08:12:51.234 = 29570234 */
		ping->png_ss_count = 0;
		/* sequential counter or input identifier */
		ping->png_ss_serial = 0;
		/* system 1 or system 2 serial number */
		ping->png_max_range = 0;
		/* max range of ping in number of samples */
		ping->png_r_zero = 0;
		/* range to normal incidence used in TVG
		    (R0 predicted) in samples */
		ping->png_r_zero_corr = 0;
		/* range to normal incidence used to correct
		    sample amplitudes in number of samples */
		ping->png_tvg_start = 0;
		/* start sample of TVG ramp if not enough
		    dynamic range (0 otherwise) */
		ping->png_tvg_stop = 0; /* stop sample of TVG ramp if not enough
		                            dynamic range (0 otherwise) */
		ping->png_bsn = 0;
		/* normal incidence backscatter (BSN) in dB */
		ping->png_bso = 0;
		/* oblique incidence backscatter (BSO) in dB */
		ping->png_tx = 0;
		/* Tx beamwidth in 0.1 degree */
		ping->png_tvg_crossover = 0;
		/* TVG law crossover angle in degrees */
		ping->png_nbeams_ss = 0;
		/* number of beams with sidescan */
		ping->png_npixels = 0;
		/* number of pixels of sidescan */
		for (int i = 0; i < MBSYS_SIMRAD2_MAXBEAMS; i++) {
			ping->png_beam_index[i] = 0;
			/* beam index number */
			ping->png_sort_direction[i] = 0;
			/* sorting direction - first sample in beam has lowest
			    range if 1, highest if -1. */
			ping->png_beam_samples[i] = 0;
			/* number of sidescan samples derived from
			    each beam */
			ping->png_start_sample[i] = 0;
			/* start sample number */
			ping->png_center_sample[i] = 0;
			/* center sample number */
		}
		for (int i = 0; i < MBSYS_SIMRAD2_MAXRAWPIXELS; i++) {
			ping->png_ssraw[i] = EM2_INVALID_AMP;
			/* the sidescan ordered port to starboard */
		}
		for (int i = 0; i < MBSYS_SIMRAD2_MAXPIXELS; i++) {
			ping->png_ss[i] = EM2_INVALID_AMP;
			/* the sidescan ordered port to starboard */
			ping->png_ssalongtrack[i] = EM2_INVALID_AMP;
			/* the sidescan ordered port to starboard */
		}
	}

	/* assume success */
	const int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                             int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get beam and pixel numbers */
		struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;
		*nbath = ping->png_nbeams_max;
		*namp = *nbath;
		*nss = MBSYS_SIMRAD2_MAXPIXELS;

		/* double it for the EM3002 */
		if (store->sonar == MBSYS_SIMRAD2_EM3002) {
			ping = (struct mbsys_simrad2_ping_struct *)store->ping2;
			*nbath += ping->png_nbeams_max;
			*namp = *nbath;
			*nss += MBSYS_SIMRAD2_MAXPIXELS;
		}
	}
	else {
		/* get beam and pixel numbers */
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

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)mb_io_ptr->store_data;

	/* extract data from structure */
	struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;
	*pingnumber = ping->png_count;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       pingnumber: %u\n", *pingnumber);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	// struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* get sonar type */
	*sonartype = MB_TOPOGRAPHY_TYPE_MULTIBEAM;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       sonartype:  %d\n", *sonartype);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_sidescantype(int verbose, void *mbio_ptr, void *store_ptr, int *ss_type, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	// struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* get sidescan type */
	*ss_type = MB_SIDESCAN_LOGARITHMIC;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ss_type:    %d\n", *ss_type);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_preprocess(int verbose,     /* in: verbosity level set on command line 0..N */
                             void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                             void *store_ptr, /* in: see mbsys_reson7k.h:/^struct mbsys_reson7k_struct/ */
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

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get preprocessing parameters */
  struct mb_preprocess_struct *pars = (struct mb_preprocess_struct *)preprocess_pars_ptr;

  /* get data structure pointers */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;
  // struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

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

	int time_i[7];
	double time_d;
	double navlon, navlat, sensordepth, speed, heading, roll, pitch, heave, altitude;

	/* depth sensor offsets - used in place of heave for underwater platforms */
	// int depthsensor_mode = MBSYS_SIMRAD2_ZMODE_UNKNOWN;

	int interp_error = MB_ERROR_NO_ERROR;
	int jnav, jsensordepth, jheading, jaltitude, jattitude;

  /* if called with store_ptr == NULL then called after mb_read_init() but before
      any data are read - for some formats this allows kluge options to set special
      reading conditions/behaviors */
  if (store_ptr == NULL) {

  }

	/* deal with a survey record */
	else if (store->kind == MB_DATA_DATA) {
  	struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

		/*--------------------------------------------------------------*/
		/* get depth sensor mode from the start record
		    NI => Use heave
		    IN => Depth sensor */
		/*--------------------------------------------------------------*/
		// if (store->par_dsh[0] == 'I')
		// 	depthsensor_mode = MBSYS_SIMRAD2_ZMODE_USE_SENSORDEPTH_ONLY;
		// else if (store->par_dsh[0] == 'N')
		// 	depthsensor_mode = MBSYS_SIMRAD2_ZMODE_USE_SENSORDEPTH_AND_HEAVE;
		// else
		// 	depthsensor_mode = MBSYS_SIMRAD2_ZMODE_USE_HEAVE_ONLY;

		/*--------------------------------------------------------------*/
		/* change timestamp if indicated */
		/*--------------------------------------------------------------*/
		if (pars->timestamp_changed) {
			time_d = pars->time_d;
			mb_get_date(verbose, time_d, time_i);

			/* set time */
			ping->png_date = 10000 * time_i[0] + 100 * time_i[1] + time_i[2];
			ping->png_msec = 3600000 * time_i[3] + 60000 * time_i[4] + 1000 * time_i[5] + 0.001 * time_i[6];
			store->date = ping->png_date;
			store->msec = ping->png_msec;
			fprintf(stderr,
			        "Timestamp changed in function %s: "
			        "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d "
			        "| ping_number:%d\n",
			        __func__, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], ping->png_count);
		}

		/*--------------------------------------------------------------*/
		/* interpolate ancillary values from arrays provided by mbpreprocess  */
		/*--------------------------------------------------------------*/

		/* get time */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &time_d);

		mb_linear_interp_longitude(verbose, pars->nav_time_d - 1, pars->nav_lon - 1, pars->n_nav, time_d, &navlon,
		                                           &jnav, &interp_error);
		if (navlon < -180.0)
			navlon += 360.0;
		else if (navlon > 180.0)
			navlon -= 360.0;
		 mb_linear_interp_latitude(verbose, pars->nav_time_d - 1, pars->nav_lat - 1, pars->n_nav, time_d, &navlat,
		                                          &jnav, &interp_error);
		 mb_linear_interp(verbose, pars->nav_time_d - 1, pars->nav_speed - 1, pars->n_nav, time_d, &speed, &jnav,
		                                 &interp_error);

		/* interpolate sensordepth */
		 mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
		                                 pars->n_sensordepth, time_d, &sensordepth, &jsensordepth, &interp_error);

		/* interpolate heading */
		 mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1, pars->n_heading,
		                                         time_d, &heading, &jheading, &interp_error);
		if (heading < 0.0)
			heading += 360.0;
		else if (heading >= 360.0)
			heading -= 360.0;

		/* interpolate altitude */
		 mb_linear_interp(verbose, pars->altitude_time_d - 1, pars->altitude_altitude - 1, pars->n_altitude,
		                                 time_d, &altitude, &jaltitude, &interp_error);

		/* interpolate attitude */
		 mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude, time_d,
		                                 &roll, &jattitude, &interp_error);
		 mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude, time_d,
		                                 &pitch, &jattitude, &interp_error);
		 mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1, pars->n_attitude, time_d,
		                                 &heave, &jattitude, &interp_error);

		/* insert navigation */
		ping->png_longitude = 10000000 * navlon;
		ping->png_latitude = 20000000 * navlat;

		/* insert heading */
		if (heading < 0.0)
			heading += 360.0;
		else if (heading > 360.0)
			heading -= 360.0;
		ping->png_heading = (int)rint(heading * 100);

		/* insert roll pitch and heave */
		ping->png_roll = (int)rint(roll / 0.01);
		ping->png_pitch = (int)rint(pitch / 0.01);
		ping->png_heave = (int)rint(heave / 0.01);

		/* generate processed sidescan */
		double *pixel_size = (double *)&mb_io_ptr->saved1;
		double *swath_width = (double *)&mb_io_ptr->saved2;
		ping->png_pixel_size = 0;
		ping->png_pixels_ss = 0;
		mbsys_simrad2_makess(verbose, mbio_ptr, store_ptr, false, pixel_size, false, swath_width, 1, error);
	}

  const int status = MB_SUCCESS;

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
int mbsys_simrad2_extract_platform(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void **platform_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:         %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:      %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       platform_ptr:   %p\n", (void *)platform_ptr);
		fprintf(stderr, "dbg2       *platform_ptr:  %p\n", (void *)*platform_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* if needed allocate a new platform structure */
	if (*platform_ptr == NULL) {
		status = mb_platform_init(verbose, (void **)platform_ptr, error);
	}

	/* extract sensor offsets from installation record */
	if (*platform_ptr != NULL) {
		/* get pointer to platform structure */
		struct mb_platform_struct *platform = (struct mb_platform_struct *)(*platform_ptr);

		/* look for multibeam sensor, add it if necessary */
		int sensor_multibeam = -1;
		for (int isensor = 0; isensor < platform->num_sensors && sensor_multibeam < 0; isensor++) {
			if (platform->sensors[isensor].type == MB_SENSOR_TYPE_SONAR_MULTIBEAM &&
			    platform->sensors[isensor].num_offsets == 2) {
				sensor_multibeam = isensor;
			}
		}
		int capability1;
		int capability2;
		int num_offsets;
		int num_time_latency;
		int par_stc = 0;
		if (sensor_multibeam < 0) {
			/* set sensor 0 (multibeam) */
			int multibeam_offsets;
			// int multibeam_type;
			if (store->sonar == MBSYS_SIMRAD2_EM3000D_1 || store->sonar == MBSYS_SIMRAD2_EM3000D_2 ||
			    store->sonar == MBSYS_SIMRAD2_EM3000D_3 || store->sonar == MBSYS_SIMRAD2_EM3000D_4 ||
			    store->sonar == MBSYS_SIMRAD2_EM3000D_5 || store->sonar == MBSYS_SIMRAD2_EM3000D_6 ||
			    store->sonar == MBSYS_SIMRAD2_EM3000D_7 || store->sonar == MBSYS_SIMRAD2_EM3000D_8 ||
			    store->sonar == MBSYS_SIMRAD2_EM3002 || store->sonar == MBSYS_SIMRAD2_EM12D) {
				// multibeam_type = MB_SENSOR_TYPE_SONAR_MULTIBEAM_TWOHEAD;
				multibeam_offsets = 4;
				par_stc = 2;
			}
			else if (store->sonar == MBSYS_SIMRAD2_EM1002 || store->sonar == MBSYS_SIMRAD2_EM2000 ||
			         store->sonar == MBSYS_SIMRAD2_EM3000 || store->sonar == MBSYS_SIMRAD2_EM100 ||
			         store->sonar == MBSYS_SIMRAD2_EM1000) {
				// multibeam_type = MB_SENSOR_TYPE_SONAR_MULTIBEAM;
				multibeam_offsets = 2;
				par_stc = 1;
			}
			else {
				// multibeam_type = MB_SENSOR_TYPE_SONAR_MULTIBEAM;
				multibeam_offsets = 2;
				par_stc = 0;
			}
			mb_path multibeam_model;
			switch (store->sonar) {
			case MBSYS_SIMRAD2_EM120:
				strcpy(multibeam_model, "EM120");
				break;
			case MBSYS_SIMRAD2_EM300:
				strcpy(multibeam_model, "EM300");
				break;
			case MBSYS_SIMRAD2_EM1002:
				strcpy(multibeam_model, "EM1002");
				break;
			case MBSYS_SIMRAD2_EM2000:
				strcpy(multibeam_model, "EM2000");
				break;
			case MBSYS_SIMRAD2_EM3000:
				strcpy(multibeam_model, "EM3000");
				break;
			case MBSYS_SIMRAD2_EM3000D_1:
				strcpy(multibeam_model, "EM3000D_1");
				break;
			case MBSYS_SIMRAD2_EM3000D_2:
				strcpy(multibeam_model, "EM3000D_2");
				break;
			case MBSYS_SIMRAD2_EM3000D_3:
				strcpy(multibeam_model, "EM3000D_3");
				break;
			case MBSYS_SIMRAD2_EM3000D_4:
				strcpy(multibeam_model, "EM3000D_4");
				break;
			case MBSYS_SIMRAD2_EM3000D_5:
				strcpy(multibeam_model, "EM3000D_5");
				break;
			case MBSYS_SIMRAD2_EM3000D_6:
				strcpy(multibeam_model, "EM3000D_6");
				break;
			case MBSYS_SIMRAD2_EM3000D_7:
				strcpy(multibeam_model, "EM3000D_7");
				break;
			case MBSYS_SIMRAD2_EM3000D_8:
				strcpy(multibeam_model, "EM3000D_8");
				break;
			case MBSYS_SIMRAD2_EM3002:
				strcpy(multibeam_model, "EM3002");
				break;
			case MBSYS_SIMRAD2_EM710:
				strcpy(multibeam_model, "EM710");
				break;
			case MBSYS_SIMRAD2_EM12S:
				strcpy(multibeam_model, "EM12S");
				break;
			case MBSYS_SIMRAD2_EM12D:
				strcpy(multibeam_model, "EM12D");
				break;
			case MBSYS_SIMRAD2_EM121:
				strcpy(multibeam_model, "EM121");
				break;
			case MBSYS_SIMRAD2_EM100:
				strcpy(multibeam_model, "EM100");
				break;
			case MBSYS_SIMRAD2_EM1000:
				strcpy(multibeam_model, "EM1000");
				break;
			default:
				sprintf(multibeam_model, "Unknown sonar model %d", store->sonar);
			}
			mb_path multibeam_serial;
			sprintf(multibeam_serial, "%d", store->par_serial_1);
			capability1 = MB_SENSOR_CAPABILITY1_NONE;
			capability2 = MB_SENSOR_CAPABILITY2_TOPOGRAPHY_MULTIBEAM + MB_SENSOR_CAPABILITY2_BACKSCATTER_MULTIBEAM;
			num_offsets = multibeam_offsets;
			num_time_latency = 0;
			status =
			    mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_SONAR_MULTIBEAM, multibeam_model, "Kongsberg",
			                           multibeam_serial, capability1, capability2, num_offsets, num_time_latency, error);
			if (status == MB_SUCCESS) {
				sensor_multibeam = platform->num_sensors - 1;
			}
		}
		if (sensor_multibeam >= 0) {
			if (status == MB_SUCCESS) {
				platform->source_bathymetry = sensor_multibeam;
				platform->source_backscatter = sensor_multibeam;
			}
			/* par_stc:
			 * System transducer configuration
			 *      0 = Single TX + single RX
			 *              EM120, EM300, EM710
			 *      1 = Single head
			 *              EM3000, EM2000, EM1002
			 *      2 = Dual Head
			 *              EM3002D, EM3002
			 *  If present, the STC parameter can be used in
			 *  decoding of the transducer installation parameters:
			 *      STC  S1X/Y/Z/R/P/H  S2X/Y/Z/R/P/H
			 *      ---  -------------  -------------
			 *       0         TX             RX
			 *       1        Head           ----
			 *       2       Head 1         Head 2
			 */
			if (par_stc == 0) {
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 0, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s1y,
					    (double)store->par_s1x, (double)-store->par_s1z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s1h,
					    (double)store->par_s1r, (double)store->par_s1p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 1, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s2y,
					    (double)store->par_s2x, (double)-store->par_s2z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s2h,
					    (double)store->par_s2r, (double)store->par_s2p, error);
			}
			else if (par_stc == 1) {
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 0, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s1y,
					    (double)store->par_s1x, (double)-store->par_s1z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s1h,
					    (double)store->par_s1r, (double)store->par_s1p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 1, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s1y,
					    (double)store->par_s1x, (double)-store->par_s1z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s1h,
					    (double)store->par_s1r, (double)store->par_s1p, error);
			}
			else if (par_stc == 2) {
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 0, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s1y,
					    (double)store->par_s1x, (double)-store->par_s1z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s1h,
					    (double)store->par_s1r, (double)store->par_s1p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 1, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s1y,
					    (double)store->par_s1x, (double)-store->par_s1z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s1h,
					    (double)store->par_s1r, (double)store->par_s1p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 2, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s2y,
					    (double)store->par_s2x, (double)-store->par_s2z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s2h,
					    (double)store->par_s2r, (double)store->par_s2p, error);
				if (status == MB_SUCCESS)
					status = mb_platform_set_sensor_offset(
					    verbose, (void *)platform, sensor_multibeam, 3, MB_SENSOR_POSITION_OFFSET_STATIC, (double)store->par_s2y,
					    (double)store->par_s2x, (double)-store->par_s2z, MB_SENSOR_ATTITUDE_OFFSET_STATIC, (double)store->par_s2h,
					    (double)store->par_s2r, (double)store->par_s2p, error);
			}
		}

		/* set position sensor 1, add it if necessary
		   - note that sometimes the start datagrams may have the active position
		     sensor set == 0 (which means position system 1 is active) while
		     having the position system 1 quality flag set to off  - in this case
		     force the position system 1 quality flag to on so that the sensor
		     structure is created */
		if (store->par_aps == 0 && platform->source_position1 < 0) {
			/* set sensor 1 (position) */
			capability1 = MB_SENSOR_CAPABILITY1_POSITION + MB_SENSOR_CAPABILITY1_HEADING;
			capability2 = MB_SENSOR_CAPABILITY2_NONE;
			capability1 = 0;
			capability2 = 0;
			num_offsets = 1;
			num_time_latency = 0;
			status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_POSITION, NULL, NULL, NULL, capability1,
			                                capability2, num_offsets, num_time_latency, error);
			if (status == MB_SUCCESS) {
				platform->source_position1 = platform->num_sensors - 1;
			}
		}

		int position_offset_mode;
		double position_offset_x;
		double position_offset_y;
		double position_offset_z;
		int attitude_offset_mode;
		double attitude_offset_heading;
		double attitude_offset_roll;
                double attitude_offset_pitch;

		/* set offsets for position sensor 1 */
		if (platform->source_position1 >= 0 && platform->sensors[platform->source_position1].num_offsets == 1) {
			/* set offsets based on whether position data are already motion compensated */
			if (store->par_p1m) {
				position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
				position_offset_x = 0.0;
				position_offset_y = 0.0;
				position_offset_z = 0.0;
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_NONE;
				attitude_offset_heading = 0.0;
				attitude_offset_roll = 0.0;
				attitude_offset_pitch = 0.0;
			}
			else {
				position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
				position_offset_x = store->par_p1y;
				position_offset_y = store->par_p1x;
				position_offset_z = -store->par_p1z;
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_STATIC;
				attitude_offset_heading = store->par_gcg;
				attitude_offset_roll = 0.0;
				attitude_offset_pitch = 0.0;
			}

			/* now set the offsets for position sensor 1 */
			status = mb_platform_set_sensor_offset(verbose, (void *)platform, platform->source_position1, 0, position_offset_mode,
			                                       position_offset_x, position_offset_y, position_offset_z, attitude_offset_mode,
			                                       attitude_offset_heading, attitude_offset_roll, attitude_offset_pitch, error);

			/* set time latency for position sensor 1 */
			if (status == MB_SUCCESS && store->par_p1d != 0.0) {
				status = mb_platform_set_sensor_timelatency(verbose, (void *)platform, platform->source_position1,
				                                            MB_SENSOR_TIME_LATENCY_STATIC, (double)store->par_p1d, 0, NULL, NULL,
				                                            error);
			}
		}

		/* set position sensor 2, add it if necessary
		   - note that sometimes the start datagrams may have the active position
		     sensor set == 1 (which means position system 2 is active) while
		     having the position system 2 quality flag set to off  - in this case
		     force the position system 2 quality flag to on so that the sensor
		     structure is created */
		if (store->par_aps == 1 && platform->source_position2 < 0) {
			/* set sensor 2 (position) */
			capability1 = MB_SENSOR_CAPABILITY1_POSITION + MB_SENSOR_CAPABILITY1_HEADING;
			capability2 = MB_SENSOR_CAPABILITY2_NONE;
			capability1 = 0;
			capability2 = 0;
			num_offsets = 1;
			num_time_latency = 0;
			status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_POSITION, NULL, NULL, NULL, capability1,
			                                capability2, num_offsets, num_time_latency, error);
			if (status == MB_SUCCESS) {
				platform->source_position2 = platform->num_sensors - 1;
			}
		}

		/* set offsets for position sensor 2 */
		if (platform->source_position2 >= 0 && platform->sensors[platform->source_position2].num_offsets == 1) {
			/* set offsets based on whether position data are already motion compensated */
			if (store->par_p2m) {
				position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
				position_offset_x = 0.0;
				position_offset_y = 0.0;
				position_offset_z = 0.0;
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_NONE;
				attitude_offset_heading = 0.0;
				attitude_offset_roll = 0.0;
				attitude_offset_pitch = 0.0;
			}
			else {
				position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
				position_offset_x = store->par_p2y;
				position_offset_y = store->par_p2x;
				position_offset_z = -store->par_p2z;
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_STATIC;
				attitude_offset_heading = store->par_gcg;
				attitude_offset_roll = 0.0;
				attitude_offset_pitch = 0.0;
			}

			/* now set the offsets for position sensor 2 */
			status = mb_platform_set_sensor_offset(verbose, (void *)platform, platform->source_position2, 0, position_offset_mode,
			                                       position_offset_x, position_offset_y, position_offset_z, attitude_offset_mode,
			                                       attitude_offset_heading, attitude_offset_roll, attitude_offset_pitch, error);

			/* set time latency for position sensor 2 */
			if (status == MB_SUCCESS && store->par_p2d != 0.0) {
				status = mb_platform_set_sensor_timelatency(verbose, (void *)platform, platform->source_position2,
				                                            MB_SENSOR_TIME_LATENCY_STATIC, (double)store->par_p2d, 0, NULL, NULL,
				                                            error);
			}
		}

		/* set position sensor 3, add it if necessary
		   - note that sometimes the start datagrams may have the active position
		     sensor set == 2 (which means position system 3 is active) while
		     having the position system 3 quality flag set to off  - in this case
		     force the position system 3 quality flag to on so that the sensor
		     structure is created */
		if (store->par_aps == 2 && platform->source_position3 < 0) {
			/* set sensor 3 (position) */
			capability1 = MB_SENSOR_CAPABILITY1_POSITION + MB_SENSOR_CAPABILITY1_HEADING;
			capability2 = MB_SENSOR_CAPABILITY2_NONE;
			capability1 = 0;
			capability2 = 0;
			num_offsets = 1;
			num_time_latency = 0;
			status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_POSITION, NULL, NULL, NULL, capability1,
			                                capability2, num_offsets, num_time_latency, error);
			if (status == MB_SUCCESS) {
				platform->source_position3 = platform->num_sensors - 1;
			}
		}

		/* set offsets for position sensor 3 */
		if (platform->source_position3 >= 0 && platform->sensors[platform->source_position3].num_offsets == 1) {
			/* set offsets based on whether position data are already motion compensated */
			if (store->par_p3m) {
				position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
				position_offset_x = 0.0;
				position_offset_y = 0.0;
				position_offset_z = 0.0;
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_NONE;
				attitude_offset_heading = 0.0;
				attitude_offset_roll = 0.0;
				attitude_offset_pitch = 0.0;
			}
			else {
				position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
				position_offset_x = store->par_p3y;
				position_offset_y = store->par_p3x;
				position_offset_z = -store->par_p3z;
				attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_STATIC;
				attitude_offset_heading = store->par_gcg;
				attitude_offset_roll = 0.0;
				attitude_offset_pitch = 0.0;
			}

			/* now set the offsets for position sensor 3 */
			status = mb_platform_set_sensor_offset(verbose, (void *)platform, platform->source_position3, 0, position_offset_mode,
			                                       position_offset_x, position_offset_y, position_offset_z, attitude_offset_mode,
			                                       attitude_offset_heading, attitude_offset_roll, attitude_offset_pitch, error);

			/* set time latency for position sensor 3 */
			if (status == MB_SUCCESS && store->par_p3d != 0.0) {
				status = mb_platform_set_sensor_timelatency(verbose, (void *)platform, platform->source_position3,
				                                            MB_SENSOR_TIME_LATENCY_STATIC, (double)store->par_p3d, 0, NULL, NULL,
				                                            error);
			}
		}

		/* add depth sensor if needed */
		if (platform->source_depth1 < 0 && store->par_dsh[0] == 'I' && store->par_dsh[1] == 'N') {
			capability1 = MB_SENSOR_CAPABILITY1_DEPTH;
			capability2 = MB_SENSOR_CAPABILITY2_NONE;
			capability1 = 0;
			capability2 = 0;
			num_offsets = 1;
			num_time_latency = 0;
			status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_PRESSURE, NULL, NULL, NULL, capability1,
			                                capability2, num_offsets, num_time_latency, error);
			if (status == MB_SUCCESS) {
				platform->source_depth1 = platform->num_sensors - 1;
			}
		}
		if (platform->source_depth1 >= 0 && platform->sensors[platform->source_depth1].num_offsets == 1) {
			position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
			position_offset_x = 0.0;
			position_offset_y = 0.0;
			position_offset_z = 0.0;
			attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_NONE;
			attitude_offset_heading = 0.0;
			attitude_offset_roll = 0.0;
			attitude_offset_pitch = 0.0;
			status = mb_platform_set_sensor_offset(verbose, (void *)platform, platform->source_depth1, 0, position_offset_mode,
			                                       position_offset_x, position_offset_y, position_offset_z, attitude_offset_mode,
			                                       attitude_offset_heading, attitude_offset_roll, attitude_offset_pitch, error);
			if (status == MB_SUCCESS && store->par_dsd != 0.0) {
				status = mb_platform_set_sensor_timelatency(verbose, (void *)platform, platform->source_depth1,
				                                            MB_SENSOR_TIME_LATENCY_STATIC, (double)store->par_dsd, 0, NULL, NULL,
				                                            error);
			}
		}

		/* set motion sensor 1, add it if necessary */
		if (platform->source_rollpitch1 < 0) {
			/* set sensor 1 (position) */
			capability1 = MB_SENSOR_CAPABILITY1_ROLLPITCH + MB_SENSOR_CAPABILITY1_HEADING + MB_SENSOR_CAPABILITY1_HEAVE;
			capability2 = MB_SENSOR_CAPABILITY2_NONE;
			capability1 = 0;
			capability2 = 0;
			num_offsets = 1;
			num_time_latency = 0;
			status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_VRU, NULL, NULL, NULL, capability1,
			                                capability2, num_offsets, num_time_latency, error);
			if (status == MB_SUCCESS) {
				platform->source_rollpitch1 = platform->num_sensors - 1;
			}
		}

		/* set motion sensor 1 offsets */
		if (platform->source_rollpitch1 >= 0 && platform->sensors[platform->source_rollpitch1].num_offsets == 1) {
			/* set offsets */
			position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
			position_offset_x = store->par_msy;
			position_offset_y = store->par_msx;
			position_offset_z = -store->par_msz;
			attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_STATIC;
			attitude_offset_heading = store->par_msg;
			attitude_offset_roll = store->par_msr;
			attitude_offset_pitch = store->par_msp;
			status =
			    mb_platform_set_sensor_offset(verbose, (void *)platform, platform->source_rollpitch1, 0, position_offset_mode,
			                                  position_offset_x, position_offset_y, position_offset_z, attitude_offset_mode,
			                                  attitude_offset_heading, attitude_offset_roll, attitude_offset_pitch, error);

			/* set time latency */
			if (status == MB_SUCCESS && store->par_msd != 0.0) {
				status = mb_platform_set_sensor_timelatency(verbose, (void *)platform, platform->source_rollpitch1,
				                                            MB_SENSOR_TIME_LATENCY_STATIC, (double)store->par_msd, 0, NULL, NULL,
				                                            error);
			}
		}

		/* now set primary data sources */
		if (store->par_aps == 0)
			platform->source_position = platform->source_position1;
		else if (store->par_aps == 1)
			platform->source_position = platform->source_position2;
		else if (store->par_aps == 2)
			platform->source_position = platform->source_position3;
		else
			platform->source_position = platform->source_position1;
		platform->source_rollpitch = platform->source_rollpitch1;
		if (store->par_dsh[0] == 'I' && store->par_dsh[1] == 'N')
			platform->source_depth = platform->source_depth1;
		platform->source_heave = platform->source_rollpitch1;
		if (store->par_aps == 0)
			platform->source_heading = platform->source_position1;
		else if (store->par_aps == 1)
			platform->source_heading = platform->source_position2;
		else if (store->par_aps == 2)
			platform->source_heading = platform->source_position3;

		/* print platform */
		if (verbose >= 2) {
			status = mb_platform_print(verbose, (void *)platform, error);
		}
	}
	else {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		fprintf(stderr, "\nUnable to initialize platform offset structure\n");
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:           %d\n", *kind);
		fprintf(stderr, "dbg2       platform_ptr:   %p\n", (void *)platform_ptr);
		fprintf(stderr, "dbg2       *platform_ptr:  %p\n", (void *)*platform_ptr);
		fprintf(stderr, "dbg2       error:		   %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:		   %d\n", status);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:         %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                          double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                          double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                          double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

		/* get time */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		if (ping->png_longitude != EM2_INVALID_INT)
			*navlon = 0.0000001 * ping->png_longitude;
		else
			*navlon = 0.0;
		if (ping->png_latitude != EM2_INVALID_INT)
			*navlat = 0.00000005 * ping->png_latitude;
		else
			*navlat = 0.0;

		/* get heading */
		*heading = 0.01 * ping->png_heading;

		/* get speed  */
		if (ping->png_speed != EM2_INVALID_SHORT)
			*speed = 0.036 * ping->png_speed;
		else
			*speed = 0.0;

		/* set beamwidths in mb_io structure */
		if (store->run_tran_beam > 0 && store->run_tran_beam < 30)
			mb_io_ptr->beamwidth_ltrack = 0.1 * store->run_tran_beam;
		else if (ping->png_tx > 0) {
			mb_io_ptr->beamwidth_ltrack = 0.1 * ping->png_tx;
		}
		else if (store->sonar == MBSYS_SIMRAD2_EM120)
			mb_io_ptr->beamwidth_ltrack = 2.0;
		else if (store->sonar == MBSYS_SIMRAD2_EM300)
			mb_io_ptr->beamwidth_ltrack = 2.0;
		else if (store->sonar == MBSYS_SIMRAD2_EM1002)
			mb_io_ptr->beamwidth_ltrack = 2.0;
		else if (store->sonar == MBSYS_SIMRAD2_EM2000)
			mb_io_ptr->beamwidth_ltrack = 1.5;
		else if (store->sonar == MBSYS_SIMRAD2_EM3000 || store->sonar == MBSYS_SIMRAD2_EM3000D_1 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_2 || store->sonar == MBSYS_SIMRAD2_EM3000D_3 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_4 || store->sonar == MBSYS_SIMRAD2_EM3000D_5 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_6 || store->sonar == MBSYS_SIMRAD2_EM3000D_7 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_8 || store->sonar == MBSYS_SIMRAD2_EM3002)
			mb_io_ptr->beamwidth_ltrack = 1.5;
		else if (store->sonar == MBSYS_SIMRAD2_EM1000)
			mb_io_ptr->beamwidth_ltrack = 3.3;
		else if (store->sonar == MBSYS_SIMRAD2_EM12S || store->sonar == MBSYS_SIMRAD2_EM12D)
			mb_io_ptr->beamwidth_ltrack = 1.7;
		else if (store->sonar == MBSYS_SIMRAD2_EM121) {
			mb_io_ptr->beamwidth_ltrack = 1.0;
		}
		if (store->run_rec_beam > 0 && store->run_rec_beam < 30) {
			mb_io_ptr->beamwidth_xtrack = 0.1 * store->run_rec_beam;
		}
		else if (store->sonar == MBSYS_SIMRAD2_EM120)
			mb_io_ptr->beamwidth_xtrack = 2.0;
		else if (store->sonar == MBSYS_SIMRAD2_EM300)
			mb_io_ptr->beamwidth_xtrack = 2.0;
		else if (store->sonar == MBSYS_SIMRAD2_EM1002)
			mb_io_ptr->beamwidth_xtrack = 2.0;
		else if (store->sonar == MBSYS_SIMRAD2_EM2000)
			mb_io_ptr->beamwidth_xtrack = 1.5;
		else if (store->sonar == MBSYS_SIMRAD2_EM3000 || store->sonar == MBSYS_SIMRAD2_EM3000D_1 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_2 || store->sonar == MBSYS_SIMRAD2_EM3000D_3 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_4 || store->sonar == MBSYS_SIMRAD2_EM3000D_5 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_6 || store->sonar == MBSYS_SIMRAD2_EM3000D_7 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_8 || store->sonar == MBSYS_SIMRAD2_EM3002)
			mb_io_ptr->beamwidth_xtrack = 1.5;
		else if (store->sonar == MBSYS_SIMRAD2_EM1000)
			mb_io_ptr->beamwidth_xtrack = 3.3;
		else if (store->sonar == MBSYS_SIMRAD2_EM12S || store->sonar == MBSYS_SIMRAD2_EM12D)
			mb_io_ptr->beamwidth_xtrack = 3.5;
		else if (store->sonar == MBSYS_SIMRAD2_EM121)
			mb_io_ptr->beamwidth_xtrack = mb_io_ptr->beamwidth_ltrack;

		/* read distance and depth values into storage arrays */
		double depthscale = 0.01 * ping->png_depth_res;
		double depthoffset = 0.01 * ping->png_xducer_depth + 655.36 * ping->png_offset_multiplier;

		double dacrscale = 0.01 * ping->png_distance_res;
		double daloscale = dacrscale;  // 0.01 * ping->png_distance_res;
		double reflscale = 0.5;
		*nbath = 0;
		for (int j = 0; j < MBSYS_SIMRAD2_MAXBEAMS; j++) {
			bath[j] = 0.0;
			beamflag[j] = MB_FLAG_NULL;
			amp[j] = 0.0;
			bathacrosstrack[j] = 0.0;
			bathalongtrack[j] = 0.0;
		}
		for (int i = 0; i < ping->png_nbeams; i++) {
			const int j = ping->png_beam_num[i] - 1;
			bath[j] = depthscale * ping->png_depth[i] + depthoffset;
			beamflag[j] = ping->png_beamflag[i];
			bathacrosstrack[j] = dacrscale * ping->png_acrosstrack[i];
			bathalongtrack[j] = daloscale * ping->png_alongtrack[i];
			amp[j] = reflscale * ping->png_amp[i];
		}
		*nbath = ping->png_nbeams_max;
		*namp = *nbath;
		*nss = MBSYS_SIMRAD2_MAXPIXELS;
		double pixel_size = 0.01 * ping->png_pixel_size;
		for (int i = 0; i < MBSYS_SIMRAD2_MAXPIXELS; i++) {
			if (ping->png_ss[i] != EM2_INVALID_AMP) {
				ss[i] = 0.01 * ping->png_ss[i];
				ssacrosstrack[i] = pixel_size * (i - MBSYS_SIMRAD2_MAXPIXELS / 2);
				ssalongtrack[i] = daloscale * ping->png_ssalongtrack[i];
			}
			else {
				ss[i] = MB_SIDESCAN_NULL;
				ssacrosstrack[i] = pixel_size * (i - MBSYS_SIMRAD2_MAXPIXELS / 2);
				ssalongtrack[i] = 0.0;
			}
		}

		/* deal with second head in case of EM3002 */
		if (store->sonar == MBSYS_SIMRAD2_EM3002 && store->numberheads == 2 && store->ping2 != NULL &&
		    store->ping2->png_count == ping->png_count) {
			/* get survey data structure */
			ping = (struct mbsys_simrad2_ping_struct *)store->ping2;

			/* read distance and depth values into storage arrays */
			depthscale = 0.01 * ping->png_depth_res;
			depthoffset = 0.01 * ping->png_xducer_depth + 655.36 * ping->png_offset_multiplier;

			dacrscale = 0.01 * ping->png_distance_res;
			daloscale = 0.01 * ping->png_distance_res;
			reflscale = 0.5;
			for (int j = *nbath; j < 2 * MBSYS_SIMRAD2_MAXBEAMS; j++) {
				bath[j] = 0.0;
				beamflag[j] = MB_FLAG_NULL;
				amp[j] = 0.0;
				bathacrosstrack[j] = 0.0;
				bathalongtrack[j] = 0.0;
			}
			for (int i = 0; i < ping->png_nbeams; i++) {
				const int j = *nbath + ping->png_beam_num[i] - 1;
				bath[j] = depthscale * ping->png_depth[i] + depthoffset;
				beamflag[j] = ping->png_beamflag[i];
				bathacrosstrack[j] = dacrscale * ping->png_acrosstrack[i];
				bathalongtrack[j] = daloscale * ping->png_alongtrack[i];
				amp[j] = reflscale * ping->png_amp[i];
			}
			*nbath += ping->png_nbeams_max;
			*namp = *nbath;
			pixel_size = 0.01 * ping->png_pixel_size;
			for (int i = 0; i < MBSYS_SIMRAD2_MAXPIXELS; i++) {
				const int j = *nss + i;
				if (ping->png_ss[i] != EM2_INVALID_AMP) {
					ss[j] = 0.01 * ping->png_ss[i];
					ssacrosstrack[j] = pixel_size * (i - MBSYS_SIMRAD2_MAXPIXELS / 2);
					ssalongtrack[j] = daloscale * ping->png_ssalongtrack[i];
				}
				else {
					ss[j] = MB_SIDESCAN_NULL;
					ssacrosstrack[j] = pixel_size * (i - MBSYS_SIMRAD2_MAXPIXELS / 2);
					ssalongtrack[j] = 0.0;
				}
			}
			*nss += MBSYS_SIMRAD2_MAXPIXELS;
		}

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Extracted values:\n");
			fprintf(stderr, "dbg4       kind:       %d\n", *kind);
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
			fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
			fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
			fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
			fprintf(stderr, "dbg4       speed:      %f\n", *speed);
			fprintf(stderr, "dbg4       heading:    %f\n", *heading);
			fprintf(stderr, "dbg4       nbath:      %d\n", *nbath);
			for (int i = 0; i < *nbath; i++)
				fprintf(stderr, "dbg4       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
				        bathacrosstrack[i], bathalongtrack[i]);
			fprintf(stderr, "dbg4        namp:     %d\n", *namp);
			for (int i = 0; i < *namp; i++)
				fprintf(stderr, "dbg4        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
				        bathalongtrack[i]);
			fprintf(stderr, "dbg4        nss:      %d\n", *nss);
			for (int i = 0; i < *nss; i++)
				fprintf(stderr, "dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
				        ssalongtrack[i]);
		}

		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV || *kind == MB_DATA_NAV1 || *kind == MB_DATA_NAV2 || *kind == MB_DATA_NAV3) {
		/* get time */
		time_i[0] = store->pos_date / 10000;
		time_i[1] = (store->pos_date % 10000) / 100;
		time_i[2] = store->pos_date % 100;
		time_i[3] = store->pos_msec / 3600000;
		time_i[4] = (store->pos_msec % 3600000) / 60000;
		time_i[5] = (store->pos_msec % 60000) / 1000;
		time_i[6] = (store->pos_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		if (store->pos_longitude != EM2_INVALID_INT)
			*navlon = 0.0000001 * store->pos_longitude;
		else
			*navlon = 0.0;
		if (store->pos_latitude != EM2_INVALID_INT)
			*navlat = 0.00000005 * store->pos_latitude;
		else
			*navlat = 0.0;

		/* get heading */
		*heading = 0.01 * store->pos_heading;

		/* get speed  */
		if (store->pos_speed != EM2_INVALID_SHORT)
			*speed = 0.036 * store->pos_speed;
		else
			*speed = 0.0;

		*nbath = 0;
		*namp = 0;
		*nss = 0;

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Extracted values:\n");
			fprintf(stderr, "dbg4       kind:       %d\n", *kind);
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
			fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
			fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
			fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
			fprintf(stderr, "dbg4       speed:      %f\n", *speed);
			fprintf(stderr, "dbg4       heading:    %f\n", *heading);
		}

		/* done translating values */
	}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT) {
		/* copy comment */
    memset((void *)comment, 0, MB_COMMENT_MAXLINE);
    strncpy(comment, store->par_com, MIN(MB_COMMENT_MAXLINE, MBSYS_SIMRAD2_COMMENT_LENGTH) - 1);

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  New ping read by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  New ping values:\n");
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       comment:    %s\n", comment);
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}
	else if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind != MB_DATA_COMMENT) {
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
	}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_DATA) {
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		for (int i = 0; i < *nbath; i++)
			fprintf(stderr, "dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
			        bathacrosstrack[i], bathalongtrack[i]);
		fprintf(stderr, "dbg2        namp:     %d\n", *namp);
		for (int i = 0; i < *namp; i++)
			fprintf(stderr, "dbg2       beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
			        bathalongtrack[i]);
		fprintf(stderr, "dbg2        nss:      %d\n", *nss);
		for (int i = 0; i < *nss; i++)
			fprintf(stderr, "dbg2        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
			        ssalongtrack[i]);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                         double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                         double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                         double *ssalongtrack, char *comment, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       kind:       %d\n", kind);
	}
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV)) {
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
	}
	if (verbose >= 2 && kind == MB_DATA_DATA) {
		fprintf(stderr, "dbg2       nbath:      %d\n", nbath);
		if (verbose >= 3)
			for (int i = 0; i < nbath; i++)
				fprintf(stderr, "dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
				        bathacrosstrack[i], bathalongtrack[i]);
		fprintf(stderr, "dbg2       namp:       %d\n", namp);
		if (verbose >= 3)
			for (int i = 0; i < namp; i++)
				fprintf(stderr, "dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
				        bathalongtrack[i]);
		fprintf(stderr, "dbg2        nss:       %d\n", nss);
		if (verbose >= 3)
			for (int i = 0; i < nss; i++)
				fprintf(stderr, "dbg3        beam:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
				        ssalongtrack[i]);
	}
	if (verbose >= 2 && kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* set data kind */
	store->kind = kind;

	int status = MB_SUCCESS;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get guess at sonar if needed  */
		if (store->sonar == MBSYS_SIMRAD2_UNKNOWN) {
			if (nbath <= 87) {
				store->sonar = MBSYS_SIMRAD2_EM2000;
			}
			else if (nbath <= 111) {
				store->sonar = MBSYS_SIMRAD2_EM1002;
			}
			else if (nbath <= 127) {
				store->sonar = MBSYS_SIMRAD2_EM3000;
			}
			else if (nbath <= 135) {
				store->sonar = MBSYS_SIMRAD2_EM300;
			}
			else if (nbath <= 191) {
				store->sonar = MBSYS_SIMRAD2_EM120;
			}
			else if (nbath <= 254) {
				store->sonar = MBSYS_SIMRAD2_EM3000D_2;
			}
			else if (nbath <= 508) {
				store->sonar = MBSYS_SIMRAD2_EM3002;
			}
		}

		/* allocate secondary data structure for
		    survey data if needed */
		if (store->ping == NULL) {
			status = mbsys_simrad2_survey_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* get survey data structure */
		struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

		/* get time */
		ping->png_date = 10000 * time_i[0] + 100 * time_i[1] + time_i[2];
		ping->png_msec = 3600000 * time_i[3] + 60000 * time_i[4] + 1000 * time_i[5] + 0.001 * time_i[6];
		store->date = ping->png_date;
		store->msec = ping->png_msec;

		/* get navigation */
		if (navlon < -180.0)
			navlon += 360.0;
		else if (navlon > 180.0)
			navlon -= 360.0;
		ping->png_longitude = 10000000 * navlon;
		ping->png_latitude = 20000000 * navlat;

		/* get heading */
		ping->png_heading = (int)rint(heading * 100);

		/* get speed  */
		ping->png_speed = (int)rint(speed / 0.036);

		/* get resolutions if needed  */
		if (ping->png_depth_res == 0 || ping->png_distance_res == 0) {
			if (store->sonar == MBSYS_SIMRAD2_EM300 || store->sonar == MBSYS_SIMRAD2_EM120) {
				ping->png_depth_res = 10;    /* kluge */
				ping->png_distance_res = 10; /* kluge */
			}
			else {
				ping->png_depth_res = 1;    /* kluge */
				ping->png_distance_res = 1; /* kluge */
			}
		}

		/* set initial values for resolutions */
		double depthscale = 0.01 * ping->png_depth_res;
		const double depthoffset = 0.01 * ping->png_xducer_depth + 655.36 * ping->png_offset_multiplier;
		double dacrscale = 0.01 * ping->png_distance_res;
		double daloscale = dacrscale;
		const double reflscale = 0.5;

		/* Figure out depth and distance scaling on
		 * the fly. Using the existing scaling got us
		 * into trouble with Revelle data in August-September 2001.
		 * Use calculated values only if needed to fit
		 * new depths into short int's.
		 */
		if (status == MB_SUCCESS) {
			/* get max depth and distance values */
			double depthmax = 0.0;
			double distancemax = 0.0;
			for (int i = 0; i < nbath; i++) {
				if (beamflag[i] != MB_FLAG_NULL) {
					depthmax = MAX(depthmax, fabs(bath[i] - depthoffset));
					distancemax = MAX(distancemax, fabs(bathacrosstrack[i]));
				}
			}

			/* figure out best scaling */
			const int png_depth_res =
				store->sonar == MBSYS_SIMRAD2_EM120 || store->sonar == MBSYS_SIMRAD2_EM300
				? (int)(depthmax / 655.36) + 1
				: (int)(depthmax / 327.68) + 1;
			const int png_distance_res = (int)(distancemax / 327.68) + 1;

			/* Change scaling if needed */
			if (png_depth_res > ping->png_depth_res) {
				ping->png_depth_res = png_depth_res;
				depthscale = 0.01 * ping->png_depth_res;
			}
			if (png_distance_res > ping->png_distance_res) {
				ping->png_distance_res = png_distance_res;
				dacrscale = 0.01 * ping->png_distance_res;
				daloscale = 0.01 * ping->png_distance_res;
			}
		}

		/* deal with data from the dual head EM3002 */
		if (status == MB_SUCCESS && store->sonar == MBSYS_SIMRAD2_EM3002) {
			struct mbsys_simrad2_ping_struct *ping2 = (struct mbsys_simrad2_ping_struct *)store->ping2;

			ping2->png_date = ping->png_date;
			ping2->png_msec = ping->png_msec;
			ping2->png_longitude = ping->png_longitude;
			ping2->png_latitude = ping->png_latitude;
			ping2->png_heading = ping->png_heading;
			ping2->png_speed = ping->png_speed;
			ping2->png_depth_res = ping->png_depth_res;
			ping2->png_distance_res = ping->png_distance_res;

			if (ping->png_nbeams == 0) {
				for (int i = 0; i < nbath / 2; i++) {
					if (beamflag[i] != MB_FLAG_NULL) {
						const int j = ping->png_nbeams;
						ping->png_beam_num[j] = i + 1;
						ping->png_depth[j] = (int)rint((bath[i] - depthoffset) / depthscale);
						ping->png_beamflag[j] = beamflag[i];
						ping->png_acrosstrack[j] = (int)rint(bathacrosstrack[i] / dacrscale);
						ping->png_alongtrack[j] = (int)rint(bathalongtrack[i] / daloscale);
						ping->png_amp[j] = (int)rint(amp[i] / reflscale);
						ping->png_nbeams++;
					}
				}
				ping->png_nbeams_max = nbath;
				ping2->png_nbeams = 0;
				for (int i = nbath / 2; i < nbath; i++) {
					if (beamflag[i] != MB_FLAG_NULL) {
						const int j = ping2->png_nbeams;
						ping2->png_beam_num[j] = i + 1;
						ping2->png_depth[j] = (int)rint((bath[i] - depthoffset) / depthscale);
						ping2->png_beamflag[j] = beamflag[i];
						ping2->png_acrosstrack[j] = (int)rint(bathacrosstrack[i] / dacrscale);
						ping2->png_alongtrack[j] = (int)rint(bathalongtrack[i] / daloscale);
						ping2->png_amp[j] = (int)rint(amp[i] / reflscale);
						ping2->png_nbeams++;
					}
				}
				ping2->png_nbeams_max = nbath;
			}
			else {
				for (int j = 0; j < ping->png_nbeams; j++) {
					const int i = ping->png_beam_num[j] - 1;
					ping->png_depth[j] = (int)rint((bath[i] - depthoffset) / depthscale);
					ping->png_beamflag[j] = beamflag[i];
					ping->png_acrosstrack[j] = (int)rint(bathacrosstrack[i] / dacrscale);
					ping->png_alongtrack[j] = (int)rint(bathalongtrack[i] / daloscale);
					ping->png_amp[j] = (int)rint(amp[i] / reflscale);
				}
				for (int j = 0; j < ping2->png_nbeams; j++) {
					const int i = ping->png_beam_num[ping->png_nbeams - 1] + ping2->png_beam_num[j] - 1;
					ping2->png_depth[j] = (int)rint((bath[i] - depthoffset) / depthscale);
					ping2->png_beamflag[j] = beamflag[i];
					ping2->png_acrosstrack[j] = (int)rint(bathacrosstrack[i] / dacrscale);
					ping2->png_alongtrack[j] = (int)rint(bathalongtrack[i] / daloscale);
					ping2->png_amp[j] = (int)rint(amp[i] / reflscale);
				}
			}

			/* handle sidescan */
			if (ping->png_pixels_ss + ping2->png_pixels_ss != nss) {
				ping->png_pixels_ss = nss / 2;
				ping2->png_pixels_ss = nss / 2;
			}
			if (ping->png_pixel_size == 0) {
				int i0 = nss;
				int i1 = 0;
				double x0;
				double x1;
				for (int i = 0; i < nss / 2; i++) {
					if (ss[i] > MB_SIDESCAN_NULL) {
						if (i < i0) {
							i0 = i;
							x0 = ssacrosstrack[i];
						}
						i1 = i;
						x1 = ssacrosstrack[i];
					}
				}
				if (i1 - i0 > 1) {
					ping->png_pixel_size = (int)(100.0 * (x1 - x0) / (i1 - 10 - 1));
				}
			}
			if (ping2->png_pixel_size == 0) {
				int i0 = nss;
				int i1 = 0;
				double x0;
				double x1;
				for (int i = nss / 2; i < nss; i++) {
					if (ss[i] > MB_SIDESCAN_NULL) {
						if (i < i0) {
							i0 = i;
							x0 = ssacrosstrack[i];
						}
						i1 = i;
						x1 = ssacrosstrack[i];
					}
				}
				if (i1 - i0 > 1) {
					ping2->png_pixel_size = (int)(100.0 * (x1 - x0) / (i1 - 10 - 1));
				}
			}
			for (int j = 0; j < nss / 2; j++) {
				if (ss[j] > MB_SIDESCAN_NULL) {
					ping->png_ss[j] = (int)rint(100 * ss[j]);
					ping->png_ssalongtrack[j] = (int)rint(ssalongtrack[j] / daloscale);
				}
				else {
					ping->png_ss[j] = EM2_INVALID_AMP;
					ping->png_ssalongtrack[j] = EM2_INVALID_AMP;
				}
			}
			for (int j = 0; j < nss / 2; j++) {
				const int i = nss / 2 + j;
				if (ss[i] > MB_SIDESCAN_NULL) {
					ping2->png_ss[j] = (int)rint(100 * ss[i]);
					ping2->png_ssalongtrack[j] = (int)rint(ssalongtrack[i] / daloscale);
				}
				else {
					ping2->png_ss[j] = EM2_INVALID_AMP;
					ping2->png_ssalongtrack[j] = EM2_INVALID_AMP;
				}
			}
		}

		/* else deal with data from all the single head sonars */
		else if (status == MB_SUCCESS) {
			if (ping->png_nbeams == 0) {
				for (int i = 0; i < nbath; i++)
					if (beamflag[i] != MB_FLAG_NULL) {
						const int j = ping->png_nbeams;
						ping->png_beam_num[j] = i + 1;
						ping->png_depth[j] = (int)rint((bath[i] - depthoffset) / depthscale);
						ping->png_beamflag[j] = beamflag[i];
						ping->png_acrosstrack[j] = (int)rint(bathacrosstrack[i] / dacrscale);
						ping->png_alongtrack[j] = (int)rint(bathalongtrack[i] / daloscale);
						ping->png_amp[j] = (int)rint(amp[i] / reflscale);
						ping->png_nbeams++;
					}
				ping->png_nbeams_max = nbath;
			}
			else {
				for (int j = 0; j < ping->png_nbeams; j++) {
					const int i = ping->png_beam_num[j] - 1;
					ping->png_depth[j] = (int)rint((bath[i] - depthoffset) / depthscale);
					ping->png_beamflag[j] = beamflag[i];
					ping->png_acrosstrack[j] = (int)rint(bathacrosstrack[i] / dacrscale);
					ping->png_alongtrack[j] = (int)rint(bathalongtrack[i] / daloscale);
					ping->png_amp[j] = (int)rint(amp[i] / reflscale);
				}
			}
			for (int i = 0; i < nss; i++) {
				if (ss[i] > MB_SIDESCAN_NULL) {
					ping->png_ss[i] = (int)rint(100 * ss[i]);
					ping->png_ssalongtrack[i] = (int)rint(ssalongtrack[i] / daloscale);
				}
				else {
					ping->png_ss[i] = EM2_INVALID_AMP;
					ping->png_ssalongtrack[i] = EM2_INVALID_AMP;
				}
			}
		}
	}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV || store->kind == MB_DATA_NAV1 || store->kind == MB_DATA_NAV2 ||
	         store->kind == MB_DATA_NAV3) {

		/* get time */
		store->pos_date = 10000 * time_i[0] + 100 * time_i[1] + time_i[2];
		store->pos_msec = 3600000 * time_i[3] + 60000 * time_i[4] + 1000 * time_i[5] + 0.001 * time_i[6];
		store->msec = store->pos_msec;
		store->date = store->pos_date;

		/* get navigation */
		if (navlon < -180.0)
			navlon += 360.0;
		else if (navlon > 180.0)
			navlon -= 360.0;
		store->pos_longitude = 10000000 * navlon;
		store->pos_latitude = 20000000 * navlat;

		/* get heading */
		store->pos_heading = (int)rint(heading * 100);

		/* get speed  */
		store->pos_speed = (int)rint(speed / 0.036);

		/* get roll pitch and heave */

		/* set "active" flag if needed */
		if (store->kind == MB_DATA_NAV) {
			store->pos_system = store->pos_system | 128;
		}

		/* set secondary nav flag if needed */
		else if (store->kind == MB_DATA_NAV1) {
			store->pos_system = store->pos_system | 1;
		}
		else if (store->kind == MB_DATA_NAV2) {
			store->pos_system = store->pos_system | 2;
		}
		else if (store->kind == MB_DATA_NAV3) {
			store->pos_system = store->pos_system | 3;
		}
	}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT) {
    memset((void *)store->par_com, 0, MBSYS_SIMRAD2_COMMENT_LENGTH);
    strncpy(store->par_com, comment, MIN(MBSYS_SIMRAD2_COMMENT_LENGTH, MB_COMMENT_MAXLINE) - 1);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                         double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                         double *ssv, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       ttimes:     %p\n", (void *)ttimes);
		fprintf(stderr, "dbg2       angles_xtrk:%p\n", (void *)angles);
		fprintf(stderr, "dbg2       angles_ltrk:%p\n", (void *)angles_forward);
		fprintf(stderr, "dbg2       angles_null:%p\n", (void *)angles_null);
		fprintf(stderr, "dbg2       heave:      %p\n", (void *)heave);
		fprintf(stderr, "dbg2       ltrk_off:   %p\n", (void *)alongtrack_offset);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

		/* get depth offset (heave + heave offset) */
		const double png_heave = 0.01 * ping->png_heave;
		*ssv = 0.1 * ping->png_ssv;
		*draft = 0.01 * ping->png_xducer_depth + 655.36 * ping->png_offset_multiplier - png_heave;

		/* get travel times, angles */
		double ttscale;
		if (store->sonar == MBSYS_SIMRAD2_EM120 || store->sonar == MBSYS_SIMRAD2_EM300 || store->sonar == MBSYS_SIMRAD2_EM1002 ||
		    store->sonar == MBSYS_SIMRAD2_EM2000 || store->sonar == MBSYS_SIMRAD2_EM3000 || store->sonar == MBSYS_SIMRAD2_EM710) {
			ttscale = 0.5 / ping->png_sample_rate;
		} else if (store->sonar == MBSYS_SIMRAD2_EM3000D_1 || store->sonar == MBSYS_SIMRAD2_EM3000D_2 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_3 || store->sonar == MBSYS_SIMRAD2_EM3000D_4 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_5 || store->sonar == MBSYS_SIMRAD2_EM3000D_6 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_7 || store->sonar == MBSYS_SIMRAD2_EM3000D_8 ||
		         store->sonar == MBSYS_SIMRAD2_EM3002) {
			ttscale = 0.5 / 14000;
		} else if (store->sonar == MBSYS_SIMRAD2_EM12S || store->sonar == MBSYS_SIMRAD2_EM12D ||
		         store->sonar == MBSYS_SIMRAD2_EM121 || store->sonar == MBSYS_SIMRAD2_EM1000) {
			ttscale = 1.0 / ping->png_sample_rate;
		} else {
			assert(false);
		}

		/* deal with data from the dual head EM3002 */
		if (status == MB_SUCCESS && store->sonar == MBSYS_SIMRAD2_EM3002) {
			struct mbsys_simrad2_ping_struct *ping2 = (struct mbsys_simrad2_ping_struct *)store->ping2;

			*nbeams = ping->png_nbeams_max + ping2->png_nbeams_max;
			for (int j = 0; j < *nbeams; j++) {
				ttimes[j] = 0.0;
				angles[j] = 0.0;
				angles_forward[j] = 0.0;
				angles_null[j] = 0.0;
				heave[j] = 0.0;
				alongtrack_offset[j] = 0.0;
			}
			for (int i = 0; i < ping->png_nbeams; i++) {
				const int j = ping->png_beam_num[i] - 1;
				ttimes[j] = ttscale * ping->png_range[i];
				angles[j] = 90.0 - 0.01 * ping->png_depression[i];
				angles_forward[j] = 90 - 0.01 * ping->png_azimuth[i];
				if (angles_forward[j] < 0.0)
					angles_forward[j] += 360.0;
				angles_null[i] = 0.0;
				heave[j] = png_heave;
				alongtrack_offset[j] = 0.0;
			}
			for (int i = 0; i < ping2->png_nbeams; i++) {
				const int j = ping->png_beam_num[ping->png_nbeams - 1] + ping2->png_beam_num[i] - 1;
				ttimes[j] = ttscale * ping2->png_range[i];
				angles[j] = 90.0 - 0.01 * ping2->png_depression[i];
				angles_forward[j] = 90 - 0.01 * ping2->png_azimuth[i];
				if (angles_forward[j] < 0.0)
					angles_forward[j] += 360.0;
				angles_null[i] = 0.0;
				heave[j] = png_heave;
				alongtrack_offset[j] = 0.0;
			}
		}

		/* else deal with data from single head sonars */
		else if (status == MB_SUCCESS) {
			*nbeams = ping->png_nbeams_max;
			for (int j = 0; j < ping->png_nbeams_max; j++) {
				ttimes[j] = 0.0;
				angles[j] = 0.0;
				angles_forward[j] = 0.0;
				angles_null[j] = 0.0;
				heave[j] = 0.0;
				alongtrack_offset[j] = 0.0;
			}
			for (int i = 0; i < ping->png_nbeams; i++) {
				const int j = ping->png_beam_num[i] - 1;
				ttimes[j] = ttscale * ping->png_range[i];
				angles[j] = 90.0 - 0.01 * ping->png_depression[i];
				angles_forward[j] = 90 - 0.01 * ping->png_azimuth[i];
				if (angles_forward[j] < 0.0)
					angles_forward[j] += 360.0;
				if (store->sonar == MBSYS_SIMRAD2_EM120 || store->sonar == MBSYS_SIMRAD2_EM300 ||
				    store->sonar == MBSYS_SIMRAD2_EM2000 || store->sonar == MBSYS_SIMRAD2_EM3000 ||
				    store->sonar == MBSYS_SIMRAD2_EM3000D_1 || store->sonar == MBSYS_SIMRAD2_EM3000D_2 ||
				    store->sonar == MBSYS_SIMRAD2_EM3000D_3 || store->sonar == MBSYS_SIMRAD2_EM3000D_4 ||
				    store->sonar == MBSYS_SIMRAD2_EM3000D_5 || store->sonar == MBSYS_SIMRAD2_EM3000D_6 ||
				    store->sonar == MBSYS_SIMRAD2_EM3000D_7 || store->sonar == MBSYS_SIMRAD2_EM3000D_8 ||
				    store->sonar == MBSYS_SIMRAD2_EM3002 || store->sonar == MBSYS_SIMRAD2_EM710)
					angles_null[i] = 0.0;
				else if (store->sonar == MBSYS_SIMRAD2_EM1000 || store->sonar == MBSYS_SIMRAD2_EM1002)
					angles_null[i] = angles[i];
				else if (store->sonar == MBSYS_SIMRAD2_EM12S || store->sonar == MBSYS_SIMRAD2_EM12D ||
				         store->sonar == MBSYS_SIMRAD2_EM121)
					angles_null[i] = 0.0;
				heave[j] = png_heave;
				alongtrack_offset[j] = 0.0;
			}

			/* reset null angles for EM1000 outer beams */
			if (store->sonar == MBSYS_SIMRAD2_EM1000 && *nbeams == 60) {
				for (int i = 0; i < 6; i++)
					angles_null[i] = angles_null[6];
				for (int i = 55; i <= 60; i++)
					angles_null[i] = angles_null[54];
			}
		}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
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
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       draft:      %f\n", *draft);
		fprintf(stderr, "dbg2       ssv:        %f\n", *ssv);
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (int i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n",
			        i, ttimes[i], angles[i], angles_forward[i], angles_null[i], heave[i], alongtrack_offset[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       detects:    %p\n", (void *)detects);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

		/* deal with data from the dual head EM3002 */
		if (store->sonar == MBSYS_SIMRAD2_EM3002) {
			struct mbsys_simrad2_ping_struct *ping2 = (struct mbsys_simrad2_ping_struct *)store->ping2;

			*nbeams = ping->png_nbeams_max + ping2->png_nbeams_max;
			for (int j = 0; j < *nbeams; j++) {
				detects[j] = MB_DETECT_UNKNOWN;
			}
			for (int i = 0; i < ping->png_nbeams; i++) {
				const int j = ping->png_beam_num[i] - 1;
				if (ping->png_quality[i] & 128)
					detects[j] = MB_DETECT_PHASE;
				else
					detects[j] = MB_DETECT_AMPLITUDE;
			}
			for (int i = 0; i < ping2->png_nbeams; i++) {
				const int j = ping2->png_beam_num[ping->png_nbeams - 1] + ping2->png_beam_num[i] - 1;
				if (ping2->png_quality[i] & 128)
					detects[j] = MB_DETECT_PHASE;
				else
					detects[j] = MB_DETECT_AMPLITUDE;
			}
		}

		/* else deal with data from single head sonars */
		else {
			*nbeams = ping->png_nbeams_max;
			for (int j = 0; j < ping->png_nbeams_max; j++) {
				detects[j] = MB_DETECT_UNKNOWN;
			}
			for (int i = 0; i < ping->png_nbeams; i++) {
				const int j = ping->png_beam_num[i] - 1;
				if (ping->png_quality[i] & 128)
					detects[j] = MB_DETECT_PHASE;
				else
					detects[j] = MB_DETECT_AMPLITUDE;
			}
		}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
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
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (int i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: detects:%d\n", i, detects[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_pulses(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *pulses, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       pulses:    %p\n", (void *)pulses);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

		/* deal with data from the dual head EM3002 */
		if (store->sonar == MBSYS_SIMRAD2_EM3002) {
			struct mbsys_simrad2_ping_struct *ping2 = (struct mbsys_simrad2_ping_struct *)store->ping2;

			*nbeams = ping->png_nbeams_max + ping2->png_nbeams_max;
			for (int j = 0; j < *nbeams; j++) {
				pulses[j] = MB_PULSE_UNKNOWN;
			}
			for (int i = 0; i < ping->png_nbeams; i++) {
				const int j = ping->png_beam_num[i] - 1;
				pulses[j] = MB_PULSE_CW;
			}
			for (int i = 0; i < ping2->png_nbeams; i++) {
				const int j = ping2->png_beam_num[ping->png_nbeams - 1] + ping2->png_beam_num[i] - 1;
				pulses[j] = MB_PULSE_CW;
			}
		}

		/* else deal with data from single head sonars */
		else {
			*nbeams = ping->png_nbeams_max;
			for (int j = 0; j < ping->png_nbeams_max; j++) {
				pulses[j] = MB_PULSE_UNKNOWN;
			}
			for (int i = 0; i < ping->png_nbeams; i++) {
				const int j = ping->png_beam_num[i] - 1;
				pulses[j] = MB_PULSE_CW;
			}
		}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
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
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (int i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: pulses:%d\n", i, pulses[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_gains(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain, double *pulse_length,
                        double *receive_gain, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		// struct mbsys_simrad2_survey_struct *ping = (struct mbsys_simrad2_survey_struct *)store->ping;

		/* get transmit_gain (dB) */
		*transmit_gain = (double)store->run_tran_pow;

		/* get pulse_length (sec) */
		*pulse_length = 0.000001 * (double)store->run_tran_pulse;

		/* get receive_gain (dB) */
		*receive_gain = (double)store->run_rec_gain;

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
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
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       transmit_gain: %f\n", *transmit_gain);
		fprintf(stderr, "dbg2       pulse_length:  %f\n", *pulse_length);
		fprintf(stderr, "dbg2       receive_gain:  %f\n", *receive_gain);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                   double *altitude, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

		/* get transducer depth and altitude */
		*transducer_depth = 0.01 * ping->png_xducer_depth + 655.36 * ping->png_offset_multiplier;
		const double depthscale = 0.01 * ping->png_depth_res;
		const double dacrscale = 0.01 * ping->png_distance_res;
		bool found = false;
		double altitude_best = 0.0;
		double xtrack_min = 99999999.9;
		for (int i = 0; i < ping->png_nbeams; i++) {
			if (mb_beam_ok(ping->png_beamflag[i]) && fabs(dacrscale * ping->png_acrosstrack[i]) < xtrack_min) {
				xtrack_min = fabs(dacrscale * ping->png_acrosstrack[i]);
				altitude_best = depthscale * ping->png_depth[i];
				found = true;
			}
		}
		if (!found) {
			xtrack_min = 99999999.9;
			for (int i = 0; i < ping->png_nbeams; i++) {
				if (ping->png_quality[i] > 0 && fabs(dacrscale * ping->png_acrosstrack[i]) < xtrack_min) {
					xtrack_min = fabs(dacrscale * ping->png_acrosstrack[i]);
					altitude_best = depthscale * ping->png_depth[i];
					found = true;
				}
			}
		}
		if (found) {
			*altitude = altitude_best;
		}
		else
			*altitude = 0.0;

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
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
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       transducer_depth:  %f\n", *transducer_depth);
		fprintf(stderr, "dbg2       altitude:          %f\n", *altitude);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr, int nmax, int *kind, int *n, int *time_i,
                               double *time_d, double *navlon, double *navlat, double *speed, double *heading, double *draft,
                               double *roll, double *pitch, double *heave, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       nmax:       %d\n", nmax);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	struct mbsys_simrad2_ping_struct *ping = NULL;
	struct mbsys_simrad2_attitude_struct *attitude = NULL;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		ping = (struct mbsys_simrad2_ping_struct *)store->ping;

		/* just one navigation value */
		*n = 1;

		/* get time */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		if (ping->png_longitude != EM2_INVALID_INT)
			*navlon = 0.0000001 * ping->png_longitude;
		else
			*navlon = 0.0;
		if (ping->png_latitude != EM2_INVALID_INT)
			*navlat = 0.00000005 * ping->png_latitude;
		else
			*navlat = 0.0;

		/* get heading */
		*heading = 0.01 * ping->png_heading;

		/* get speed  */
		if (ping->png_speed != EM2_INVALID_SHORT)
			*speed = 0.036 * ping->png_speed;
		else
			*speed = 0.0;

		/* get draft  */
		*draft = 0.01 * ping->png_xducer_depth + 655.36 * ping->png_offset_multiplier;

		/* get roll pitch and heave */
		*roll = 0.01 * ping->png_roll;
		*pitch = 0.01 * ping->png_pitch;
		*heave = 0.01 * ping->png_heave;

		/* done translating values */
	}

	/* extract data from nav structure */
	else if (*kind == MB_DATA_NAV || *kind == MB_DATA_NAV1 || *kind == MB_DATA_NAV2 || *kind == MB_DATA_NAV3) {
		/* get survey data structure */
		if (store->ping != NULL)
			ping = (struct mbsys_simrad2_ping_struct *)store->ping;

		/* just one navigation value */
		*n = 1;

		/* get time */
		time_i[0] = store->pos_date / 10000;
		time_i[1] = (store->pos_date % 10000) / 100;
		time_i[2] = store->pos_date % 100;
		time_i[3] = store->pos_msec / 3600000;
		time_i[4] = (store->pos_msec % 3600000) / 60000;
		time_i[5] = (store->pos_msec % 60000) / 1000;
		time_i[6] = (store->pos_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		if (store->pos_longitude != EM2_INVALID_INT)
			*navlon = 0.0000001 * store->pos_longitude;
		else
			*navlon = 0.0;
		if (store->pos_latitude != EM2_INVALID_INT)
			*navlat = 0.00000005 * store->pos_latitude;
		else
			*navlat = 0.0;

		/* get heading */
		if (store->pos_heading != EM2_INVALID_SHORT)
			*heading = 0.01 * store->pos_heading;
		else
			*heading = 0.0;

		/* get speed  */
		if (store->pos_speed != EM2_INVALID_SHORT)
			*speed = 0.036 * store->pos_speed;
		else
			*speed = 0.0;

		/* get draft  */
		if (ping != NULL)
			*draft = 0.01 * ping->png_xducer_depth + 655.36 * ping->png_offset_multiplier - 0.01 * ping->png_heave;
		else
			*draft = 0.0;

		/* get roll pitch and heave */
		*roll = 0.01 * store->pos_roll;
		*pitch = 0.01 * store->pos_pitch;
		*heave = 0.01 * store->pos_heave;

		/* done translating values */
	}

	/* extract data from attitude structure */
	else if (store->type == EM2_ATTITUDE && store->attitude != NULL) {
		/* get attitude data structure */
		attitude = (struct mbsys_simrad2_attitude_struct *)store->attitude;

		/* get n */
		*n = MIN(attitude->att_ndata, MB_ASYNCH_SAVE_MAX);

		/* get attitude time */
		int atime_i[7];
		atime_i[0] = attitude->att_date / 10000;
		atime_i[1] = (attitude->att_date % 10000) / 100;
		atime_i[2] = attitude->att_date % 100;
		atime_i[3] = attitude->att_msec / 3600000;
		atime_i[4] = (attitude->att_msec % 3600000) / 60000;
		atime_i[5] = (attitude->att_msec % 60000) / 1000;
		atime_i[6] = (attitude->att_msec % 1000) * 1000;
		double atime_d;
		mb_get_time(verbose, atime_i, &atime_d);

		/* loop over the data */
		for (int i = 0; i < *n; i++) {
			/* get time from the data record */
			time_d[i] = (double)(atime_d + 0.001 * attitude->att_time[i]);
			mb_get_date(verbose, time_d[i], &(time_i[7 * i]));

			/* get attitude from the data record */
			heave[i] = (double)(0.01 * attitude->att_heave[i]);
			roll[i] = (double)(0.01 * attitude->att_roll[i]);
			pitch[i] = (double)(0.01 * attitude->att_pitch[i]);

			/* interpolate the heading */
			int interp_error = MB_ERROR_NO_ERROR;
			mb_hedint_interp(verbose, mbio_ptr, time_d[i], &heading[i], &interp_error);

			/* interpolate the navigation */
			mb_navint_interp(verbose, mbio_ptr, time_d[i], heading[i], 0.0, &navlon[i], &navlat[i], &speed[i], &interp_error);

			/* interpolate the sonar depth */
			mb_depint_interp(verbose, mbio_ptr, time_d[i], &draft[i], &interp_error);
		}

		/* done translating values */
	}

	/* deal with comment */
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

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                              double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                              double *pitch, double *heave, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	struct mbsys_simrad2_ping_struct *ping = NULL;

	/* extract data from ping structure */
	if (*kind == MB_DATA_DATA) {
		/* get survey data structure */
		ping = (struct mbsys_simrad2_ping_struct *)store->ping;

		/* get time */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		if (ping->png_longitude != EM2_INVALID_INT)
			*navlon = 0.0000001 * ping->png_longitude;
		else
			*navlon = 0.0;
		if (ping->png_latitude != EM2_INVALID_INT)
			*navlat = 0.00000005 * ping->png_latitude;
		else
			*navlat = 0.0;

		/* get heading */
		*heading = 0.01 * ping->png_heading;

		/* get speed  */
		if (ping->png_speed != EM2_INVALID_SHORT)
			*speed = 0.036 * ping->png_speed;
		else
			*speed = 0.0;

		/* get draft  */
		*draft = 0.01 * ping->png_xducer_depth + 655.36 * ping->png_offset_multiplier - 0.01 * ping->png_heave;

		/* get roll pitch and heave */
		*roll = 0.01 * ping->png_roll;
		*pitch = 0.01 * ping->png_pitch;
		*heave = 0.01 * ping->png_heave;

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Extracted values:\n");
			fprintf(stderr, "dbg4       kind:       %d\n", *kind);
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
			fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
			fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
			fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
			fprintf(stderr, "dbg4       speed:      %f\n", *speed);
			fprintf(stderr, "dbg4       heading:    %f\n", *heading);
			fprintf(stderr, "dbg4       draft:      %f\n", *draft);
			fprintf(stderr, "dbg4       roll:       %f\n", *roll);
			fprintf(stderr, "dbg4       pitch:      %f\n", *pitch);
			fprintf(stderr, "dbg4       heave:      %f\n", *heave);
		}

		/* done translating values */
	}

	/* extract data from nav structure */
	else if (*kind == MB_DATA_NAV || *kind == MB_DATA_NAV1 || *kind == MB_DATA_NAV2 || *kind == MB_DATA_NAV3) {
		/* get survey data structure */
		if (store->ping != NULL)
			ping = (struct mbsys_simrad2_ping_struct *)store->ping;

		/* get time */
		time_i[0] = store->pos_date / 10000;
		time_i[1] = (store->pos_date % 10000) / 100;
		time_i[2] = store->pos_date % 100;
		time_i[3] = store->pos_msec / 3600000;
		time_i[4] = (store->pos_msec % 3600000) / 60000;
		time_i[5] = (store->pos_msec % 60000) / 1000;
		time_i[6] = (store->pos_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		if (store->pos_longitude != EM2_INVALID_INT)
			*navlon = 0.0000001 * store->pos_longitude;
		else
			*navlon = 0.0;
		if (store->pos_latitude != EM2_INVALID_INT)
			*navlat = 0.00000005 * store->pos_latitude;
		else
			*navlat = 0.0;

		/* get heading */
		if (store->pos_heading != EM2_INVALID_SHORT)
			*heading = 0.01 * store->pos_heading;
		else
			*heading = 0.0;

		/* get speed  */
		if (store->pos_speed != EM2_INVALID_SHORT)
			*speed = 0.036 * store->pos_speed;
		else
			*speed = 0.0;

		/* get draft  */
		if (ping != NULL)
			*draft = 0.01 * ping->png_xducer_depth + 655.36 * ping->png_offset_multiplier - 0.01 * ping->png_heave;
		else
			*draft = 0.0;

		/* get roll pitch and heave */
		*roll = 0.01 * store->pos_roll;
		*pitch = 0.01 * store->pos_pitch;
		*heave = 0.01 * store->pos_heave;

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Extracted values:\n");
			fprintf(stderr, "dbg4       kind:       %d\n", *kind);
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
			fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
			fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
			fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
			fprintf(stderr, "dbg4       speed:      %f\n", *speed);
			fprintf(stderr, "dbg4       heading:    %f\n", *heading);
			fprintf(stderr, "dbg4       draft:      %f\n", *draft);
			fprintf(stderr, "dbg4       roll:       %f\n", *roll);
			fprintf(stderr, "dbg4       pitch:      %f\n", *pitch);
			fprintf(stderr, "dbg4       heave:      %f\n", *heave);
		}

		/* done translating values */
	}

	/* deal with comment */
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
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_DATA) {
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

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                             double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                             int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
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
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* insert data in ping structure */
	if (store->kind == MB_DATA_DATA) {
		/* allocate secondary data structure for
		    survey data if needed */
		if (store->ping == NULL) {
			status = mbsys_simrad2_survey_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* get survey data structure */
		struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

		/* get time */
		ping->png_date = 10000 * time_i[0] + 100 * time_i[1] + time_i[2];
		ping->png_msec = 3600000 * time_i[3] + 60000 * time_i[4] + 1000 * time_i[5] + 0.001 * time_i[6];
		store->msec = ping->png_msec;
		store->date = ping->png_date;

		/* get navigation */
		if (navlon < -180.0)
			navlon += 360.0;
		else if (navlon > 180.0)
			navlon -= 360.0;
		ping->png_longitude = 10000000 * navlon;
		ping->png_latitude = 20000000 * navlat;

		/* get heading */
		ping->png_heading = (int)rint(heading * 100);

		/* get speed  */
		ping->png_speed = (int)rint(speed / 0.036);

		/* get draft  */
		ping->png_offset_multiplier = (int)floor(draft / 655.36);
		ping->png_xducer_depth = 100 * (draft + heave - 655.36 * ping->png_offset_multiplier);

		/* get roll pitch and heave */
		ping->png_roll = (int)rint(roll / 0.01);
		ping->png_pitch = (int)rint(pitch / 0.01);
		ping->png_heave = (int)rint(heave / 0.01);
	}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV || store->kind == MB_DATA_NAV1 || store->kind == MB_DATA_NAV2 ||
	         store->kind == MB_DATA_NAV3) {

		/* get time */
		store->pos_date = 10000 * time_i[0] + 100 * time_i[1] + time_i[2];
		store->pos_msec = 3600000 * time_i[3] + 60000 * time_i[4] + 1000 * time_i[5] + 0.001 * time_i[6];
		store->msec = store->pos_msec;
		store->date = store->pos_date;

		/* get navigation */
		if (navlon < -180.0)
			navlon += 360.0;
		else if (navlon > 180.0)
			navlon -= 360.0;
		store->pos_longitude = 10000000 * navlon;
		store->pos_latitude = 20000000 * navlat;

		/* get heading */
		store->pos_heading = (int)rint(heading * 100);

		/* get speed  */
		store->pos_speed = (int)rint(speed / 0.036);

		/* get roll pitch and heave */
		store->pos_roll = (int)rint(roll / 0.01);
		store->pos_pitch = (int)rint(pitch / 0.01);
		store->pos_heave = (int)rint(heave / 0.01);

		/* set "active" flag if needed */
		if (store->kind == MB_DATA_NAV) {
			store->pos_system = store->pos_system | 128;
		}

		/* set secondary nav flag if needed */
		else if (store->kind == MB_DATA_NAV1) {
			store->pos_system = store->pos_system | 1;
		}
		else if (store->kind == MB_DATA_NAV2) {
			store->pos_system = store->pos_system | 2;
		}
		else if (store->kind == MB_DATA_NAV3) {
			store->pos_system = store->pos_system | 3;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                              int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_VELOCITY_PROFILE) {
		/* get number of depth-velocity pairs */
		*nsvp = store->svp_num;

		/* get profile */
		for (int i = 0; i < *nsvp; i++) {
			depth[i] = 0.01 * store->svp_depth_res * store->svp_depth[i];
			velocity[i] = 0.1 * store->svp_vel[i];
		}

		/* done translating values */
	}

	/* deal with comment */
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
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       nsvp:              %d\n", *nsvp);
		for (int i = 0; i < *nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity,
                             int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       nsvp:       %d\n", nsvp);
		for (int i = 0; i < nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE) {
		/* get number of depth-velocity pairs */
		store->svp_num = MIN(nsvp, MBSYS_SIMRAD2_MAXSVP);
		store->svp_depth_res = 1;

		/* get profile */
		for (int i = 0; i < store->svp_num; i++) {
			store->svp_depth[i] = (int)(100 * depth[i] / store->svp_depth_res);
			store->svp_vel[i] = (int)(10 * velocity[i]);
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointers */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;
	struct mbsys_simrad2_struct *copy = (struct mbsys_simrad2_struct *)copy_ptr;

	int status = MB_SUCCESS;

	char *ping_save = NULL;

	/* check if survey data needs to be copied */
	if (store->kind == MB_DATA_DATA && store->ping != NULL) {
		/* make sure a survey data structure exists to
		    be copied into */
		if (copy->ping == NULL) {
			status = mbsys_simrad2_survey_alloc(verbose, mbio_ptr, copy_ptr, error);
		}

		/* save pointer value */
		ping_save = (char *)copy->ping;
	}

	char *attitude_save = NULL;

	/* check if attitude data needs to be copied */
	if (store->attitude != NULL) {
		/* make sure a attitude data structure exists to
		    be copied into */
		if (copy->attitude == NULL) {
			status = mbsys_simrad2_attitude_alloc(verbose, mbio_ptr, copy_ptr, error);
		}

		/* save pointer value */
		attitude_save = (char *)copy->attitude;
	}

	char *heading_save = NULL;

	/* check if heading data needs to be copied */
	if (store->heading != NULL) {
		/* make sure a heading data structure exists to
		    be copied into */
		if (copy->heading == NULL) {
			status = mbsys_simrad2_heading_alloc(verbose, mbio_ptr, copy_ptr, error);
		}

		/* save pointer value */
		heading_save = (char *)copy->heading;
	}

	char *ssv_save = NULL;

	/* check if ssv data needs to be copied */
	if (store->ssv != NULL) {
		/* make sure a ssv data structure exists to
		    be copied into */
		if (copy->ssv == NULL) {
			status = mbsys_simrad2_ssv_alloc(verbose, mbio_ptr, copy_ptr, error);
		}

		/* save pointer value */
		ssv_save = (char *)copy->ssv;
	}

	char *tilt_save = NULL;

	/* check if tilt data needs to be copied */
	if (store->tilt != NULL) {
		/* make sure a tilt data structure exists to
		    be copied into */
		if (copy->tilt == NULL) {
			status = mbsys_simrad2_tilt_alloc(verbose, mbio_ptr, copy_ptr, error);
		}

		/* save pointer value */
		tilt_save = (char *)copy->tilt;
	}

	/* copy the main structure */
	*copy = *store;

	/* if needed copy the survey data structure */
	if (store->kind == MB_DATA_DATA && store->ping != NULL && status == MB_SUCCESS) {
		copy->ping = (struct mbsys_simrad2_ping_struct *)ping_save;
		struct mbsys_simrad2_ping_struct *ping_store = (struct mbsys_simrad2_ping_struct *)store->ping;
		struct mbsys_simrad2_ping_struct *ping_copy = (struct mbsys_simrad2_ping_struct *)copy->ping;
		*ping_copy = *ping_store;
	} else {
		copy->ping = NULL;
	}

	/* if needed copy the attitude data structure */
	if (store->attitude != NULL && status == MB_SUCCESS) {
		copy->attitude = (struct mbsys_simrad2_attitude_struct *)attitude_save;
		struct mbsys_simrad2_attitude_struct *attitude_store = (struct mbsys_simrad2_attitude_struct *)store->attitude;
		struct mbsys_simrad2_attitude_struct *attitude_copy = (struct mbsys_simrad2_attitude_struct *)copy->attitude;
		*attitude_copy = *attitude_store;
	}

	/* if needed copy the heading data structure */
	if (store->heading != NULL && status == MB_SUCCESS) {
		copy->heading = (struct mbsys_simrad2_heading_struct *)heading_save;
		struct mbsys_simrad2_heading_struct *heading_store = (struct mbsys_simrad2_heading_struct *)store->heading;
		struct mbsys_simrad2_heading_struct *heading_copy = (struct mbsys_simrad2_heading_struct *)copy->heading;
		*heading_copy = *heading_store;
	}

	/* if needed copy the ssv data structure */
	if (store->ssv != NULL && status == MB_SUCCESS) {
		copy->ssv = (struct mbsys_simrad2_ssv_struct *)ssv_save;
		struct mbsys_simrad2_ssv_struct *ssv_store = (struct mbsys_simrad2_ssv_struct *)store->ssv;
		struct mbsys_simrad2_ssv_struct *ssv_copy = (struct mbsys_simrad2_ssv_struct *)copy->ssv;
		*ssv_copy = *ssv_store;
	}


	/* if needed copy the tilt data structure */
	if (store->tilt != NULL && status == MB_SUCCESS) {
		copy->tilt = (struct mbsys_simrad2_tilt_struct *)tilt_save;
		struct mbsys_simrad2_tilt_struct *tilt_store = (struct mbsys_simrad2_tilt_struct *)store->tilt;
		struct mbsys_simrad2_tilt_struct *tilt_copy = (struct mbsys_simrad2_tilt_struct *)copy->tilt;
		*tilt_copy = *tilt_store;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_simrad2_makess(int verbose, void *mbio_ptr, void *store_ptr, int pixel_size_set, double *pixel_size,
                         int swath_width_set, double *swath_width, int pixel_int, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:       %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       pixel_size_set:  %d\n", pixel_size_set);
		fprintf(stderr, "dbg2       pixel_size:      %f\n", *pixel_size);
		fprintf(stderr, "dbg2       swath_width_set: %d\n", swath_width_set);
		fprintf(stderr, "dbg2       swath_width:     %f\n", *swath_width);
		fprintf(stderr, "dbg2       pixel_int:       %d\n", pixel_int);
	}

	double ss[MBSYS_SIMRAD2_MAXPIXELS];
	int ss_cnt[MBSYS_SIMRAD2_MAXPIXELS];
	double ssacrosstrack[MBSYS_SIMRAD2_MAXPIXELS];
	double ssalongtrack[MBSYS_SIMRAD2_MAXPIXELS];
	mb_s_char *beam_ss;
	int nbathsort;
	double bathsort[MBSYS_SIMRAD2_MAXBEAMS];
	double depthscale, depthoffset;
	double dacrscale, daloscale;
	double reflscale;
	double pixel_size_calc;
	double ss_spacing, ss_spacing_use;
	int pixel_int_use;
	double angle, depth, xtrack, xtrackss;
	double range, beam_foot, beamwidth, sint;
	int time_i[7];
	double bath_time_d, ss_time_d;
	bool ss_ok;
	int first, last, k1, k2;

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	struct mbsys_simrad2_ping_struct *ping = NULL;

	/* construct sidescan data for first sonar head (all data) */
	if (store->kind == MB_DATA_DATA) {
		/* get pointer to raw data structure */
		ping = (struct mbsys_simrad2_ping_struct *)store->ping;

		/* zero the sidescan */
		for (int i = 0; i < MBSYS_SIMRAD2_MAXPIXELS; i++) {
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
			ss_cnt[i] = 0;
		}

		/* set scaling parameters */
		depthscale = 0.01 * ping->png_depth_res;
		depthoffset = 0.01 * ping->png_xducer_depth + 655.36 * ping->png_offset_multiplier;
		dacrscale = 0.01 * ping->png_distance_res;
		daloscale = 0.01 * ping->png_distance_res;
		reflscale = 0.5;
		// double ssoffset = 64.0;
		// if (store->sonar == MBSYS_SIMRAD2_EM300 && store->run_mode == 4) {
		//	if (depthscale * ping->png_depth[ping->png_nbeams / 2] > 3500.0 && ping->png_max_range > 19000 &&
		//	    ping->png_bsn + ping->png_bso < -60) {
		//		ssoffset = 64.0 - 0.6 * (ping->png_bsn + ping->png_bso + 60);
		//	}
		// }

		/* get raw pixel size */
		if (store->sonar == MBSYS_SIMRAD2_EM120 || store->sonar == MBSYS_SIMRAD2_EM300 || store->sonar == MBSYS_SIMRAD2_EM1002 ||
		    store->sonar == MBSYS_SIMRAD2_EM2000 || store->sonar == MBSYS_SIMRAD2_EM3000 || store->sonar == MBSYS_SIMRAD2_EM710)
			ss_spacing = 750.0 / ping->png_sample_rate;
		else if (store->sonar == MBSYS_SIMRAD2_EM3000D_1 || store->sonar == MBSYS_SIMRAD2_EM3000D_2 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_3 || store->sonar == MBSYS_SIMRAD2_EM3000D_4 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_5 || store->sonar == MBSYS_SIMRAD2_EM3000D_6 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_7 || store->sonar == MBSYS_SIMRAD2_EM3000D_8 ||
		         store->sonar == MBSYS_SIMRAD2_EM3002)
			ss_spacing = 750.0 / 14000;
		else if (store->sonar == MBSYS_SIMRAD2_EM12S || store->sonar == MBSYS_SIMRAD2_EM12D ||
		         store->sonar == MBSYS_SIMRAD2_EM121 || store->sonar == MBSYS_SIMRAD2_EM1000) {
			ss_spacing = 0.01 * ping->png_max_range;
		}

		/* get beam angle size */
		if (store->sonar == MBSYS_SIMRAD2_EM1000) {
			beamwidth = 2.5;
		}
		else {
			beamwidth = 0.1 * ((double)ping->png_tx);
		}

		/* get median depth */
		nbathsort = 0;
		for (int i = 0; i < ping->png_nbeams; i++) {
			if (mb_beam_ok(ping->png_beamflag[i])) {
				bathsort[nbathsort] = depthscale * ping->png_depth[i] + depthoffset;
				nbathsort++;
			}
		}

		/* get sidescan pixel size */
		if (!swath_width_set && nbathsort > 0) {
			(*swath_width) =
			    2.5 + MAX(90.0 - 0.01 * ping->png_depression[0], 90.0 - 0.01 * ping->png_depression[ping->png_nbeams - 1]);
			(*swath_width) = MAX((*swath_width), 60.0);
		}
		if (!pixel_size_set && nbathsort > 0) {
			qsort((char *)bathsort, nbathsort, sizeof(double), (void *)mb_double_compare);
			pixel_size_calc = 2 * tan(DTR * (*swath_width)) * bathsort[nbathsort / 2] / MBSYS_SIMRAD2_MAXPIXELS;
			pixel_size_calc = MAX(pixel_size_calc, bathsort[nbathsort / 2] * sin(DTR * 0.1));
			if ((*pixel_size) <= 0.0)
				(*pixel_size) = pixel_size_calc;
			else if (0.95 * (*pixel_size) > pixel_size_calc)
				(*pixel_size) = 0.95 * (*pixel_size);
			else if (1.05 * (*pixel_size) < pixel_size_calc)
				(*pixel_size) = 1.05 * (*pixel_size);
			else
				(*pixel_size) = pixel_size_calc;
		}

		/* get pixel interpolation */
		pixel_int_use = pixel_int + 1;

		/* check that sidescan can be used */
		/* get times of bath and sidescan records */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &bath_time_d);
		time_i[0] = ping->png_ss_date / 10000;
		time_i[1] = (ping->png_ss_date % 10000) / 100;
		time_i[2] = ping->png_ss_date % 100;
		time_i[3] = ping->png_ss_msec / 3600000;
		time_i[4] = (ping->png_ss_msec % 3600000) / 60000;
		time_i[5] = (ping->png_ss_msec % 60000) / 1000;
		time_i[6] = (ping->png_ss_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &ss_time_d);
		ss_ok = true;
		if (ping->png_nbeams < ping->png_nbeams_ss || ping->png_nbeams > ping->png_nbeams_ss + 1) {
			ss_ok = false;
			if (verbose >= 2)
				fprintf(stderr,
				        "%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Sidescan ignored: num bath beams != num ss beams: %d %d\n",
				        __func__, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
				        ping->png_nbeams, ping->png_nbeams_ss);
		}
		else if (ping->png_nbeams == ping->png_nbeams_ss) {
			for (int i = 0; i < ping->png_nbeams; i++) {
				if (ping->png_beam_num[i] != ping->png_beam_index[i] + 1 &&
				    ping->png_beam_num[i] != ping->png_beam_index[i] - 1) {
					ss_ok = false;
					if (verbose > 0)
						fprintf(stderr,
						        "%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Sidescan ignored: bath and ss beam indexes don't "
						        "match: : %d %d %d\n",
						        __func__, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], i,
						        ping->png_beam_num[i], ping->png_beam_index[i]);
				}
			}
		}

		/* loop over raw sidescan, putting each raw pixel into
		    the binning arrays */
		if (ss_ok)
			for (int i = 0; i < ping->png_nbeams_ss; i++) {
				beam_ss = &ping->png_ssraw[ping->png_start_sample[i]];
				if (mb_beam_ok(ping->png_beamflag[i])) {
					if (ping->png_beam_samples[i] > 0) {
						depth = depthscale * ping->png_depth[i];
						xtrack = dacrscale * ping->png_acrosstrack[i];
						range = sqrt(depth * depth + xtrack * xtrack);
						angle = 90.0 - 0.01 * ping->png_depression[i];
						beam_foot = range * sin(DTR * beamwidth) / cos(DTR * angle);
						sint = fabs(sin(DTR * angle));
						if (sint < ping->png_beam_samples[i] * ss_spacing / beam_foot)
							ss_spacing_use = beam_foot / ping->png_beam_samples[i];
						else
							ss_spacing_use = ss_spacing / sint;
					}
					for (int k = 0; k < ping->png_beam_samples[i]; k++) {
						if (beam_ss[k] != EM2_INVALID_AMP) {
							/* locate based on range */
							if (k == ping->png_center_sample[i]) {
								xtrackss = xtrack;
							}
							else if (i == ping->png_nbeams_ss - 1 || (k <= ping->png_center_sample[i] && i != 0)) {
								if (ping->png_range[i] != ping->png_range[i - 1]) {
									xtrackss = dacrscale * ping->png_acrosstrack[i] +
									           (dacrscale * ping->png_acrosstrack[i] - dacrscale * ping->png_acrosstrack[i - 1]) *
									               2 * ((double)(k - ping->png_center_sample[i])) /
									               fabs((double)(ping->png_range[i] - ping->png_range[i - 1]));
								}
								else {
									xtrackss = xtrack + ss_spacing_use * (k - ping->png_center_sample[i]);
								}
							}
							else {
								if (ping->png_range[i] != ping->png_range[i + 1]) {
									xtrackss = dacrscale * ping->png_acrosstrack[i] +
									           (dacrscale * ping->png_acrosstrack[i + 1] - dacrscale * ping->png_acrosstrack[i]) *
									               2 * ((double)(k - ping->png_center_sample[i])) /
									               fabs((double)(ping->png_range[i + 1] - ping->png_range[i]));
								}
								else {
									xtrackss = xtrack + ss_spacing_use * (k - ping->png_center_sample[i]);
								}
							}
							xtrackss = xtrack + ss_spacing_use * (k - ping->png_center_sample[i]);
							const int kk = MBSYS_SIMRAD2_MAXPIXELS / 2 + (int)(xtrackss / (*pixel_size));
							if (kk > 0 && kk < MBSYS_SIMRAD2_MAXPIXELS) {
								ss[kk] += reflscale * ((double)beam_ss[k]);
								ssalongtrack[kk] += daloscale * ping->png_alongtrack[i];
								ss_cnt[kk]++;
							}
						}
					}
				}
			}

		/* average the sidescan */
		first = MBSYS_SIMRAD2_MAXPIXELS;
		last = -1;
		for (int k = 0; k < MBSYS_SIMRAD2_MAXPIXELS; k++) {
			if (ss_cnt[k] > 0) {
				ss[k] /= ss_cnt[k];
				ssalongtrack[k] /= ss_cnt[k];
				ssacrosstrack[k] = (k - MBSYS_SIMRAD2_MAXPIXELS / 2) * (*pixel_size);
				first = MIN(first, k);
				last = k;
			}
			else
				ss[k] = MB_SIDESCAN_NULL;
		}

		/* interpolate the sidescan */
		k1 = first;
		k2 = first;
		for (int k = first + 1; k < last; k++) {
			if (ss_cnt[k] <= 0) {
				if (k2 <= k) {
					k2 = k + 1;
					while (ss_cnt[k2] <= 0 && k2 < last)
						k2++;
				}
				if (k2 - k1 <= pixel_int_use) {
					ss[k] = ss[k1] + (ss[k2] - ss[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
					ssacrosstrack[k] = (k - MBSYS_SIMRAD2_MAXPIXELS / 2) * (*pixel_size);
					ssalongtrack[k] =
					    ssalongtrack[k1] + (ssalongtrack[k2] - ssalongtrack[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
				}
			}
			else {
				k1 = k;
			}
		}

		/* insert the new sidescan into store */
		ping->png_pixel_size = (int)(100 * (*pixel_size));
		if (last > first)
			ping->png_pixels_ss = MBSYS_SIMRAD2_MAXPIXELS;
		else
			ping->png_pixels_ss = 0;
		for (int i = 0; i < MBSYS_SIMRAD2_MAXPIXELS; i++) {
			if (ss[i] > MB_SIDESCAN_NULL) {
				ping->png_ss[i] = (short)(100 * ss[i]);
				ping->png_ssalongtrack[i] = (short)(ssalongtrack[i] / daloscale);
			}
			else {
				ping->png_ss[i] = EM2_INVALID_AMP;
				ping->png_ssalongtrack[i] = EM2_INVALID_AMP;
			}
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Sidescan regenerated in <%s>\n", __func__);
			fprintf(stderr, "dbg2       png_nbeams_ss: %d\n", ping->png_nbeams_ss);
			for (int i = 0; i < ping->png_nbeams_ss; i++)
				fprintf(stderr, "dbg2       beam:%d  flag:%3d  bath:%d  amp:%d  acrosstrack:%d  alongtrack:%d\n",
				        ping->png_beam_num[i], ping->png_beamflag[i], ping->png_depth[i], ping->png_amp[i],
				        ping->png_acrosstrack[i], ping->png_alongtrack[i]);
			fprintf(stderr, "dbg2       pixels_ss:  %d\n", MBSYS_SIMRAD2_MAXPIXELS);
			for (int i = 0; i < MBSYS_SIMRAD2_MAXPIXELS; i++)
				fprintf(stderr, "dbg2       pixel:%4d  cnt:%3d  ss:%10f  xtrack:%10f  ltrack:%10f\n", i, ss_cnt[i], ss[i],
				        ssacrosstrack[i], ssalongtrack[i]);
			fprintf(stderr, "dbg2       pixels_ss:  %d\n", ping->png_pixels_ss);
			for (int i = 0; i < MBSYS_SIMRAD2_MAXPIXELS; i++)
				fprintf(stderr, "dbg2       pixel:%4d  ss:%8d  ltrack:%8d\n", i, ping->png_ss[i], ping->png_ssalongtrack[i]);
		}
	}

	/* construct sidescan data for second sonar head (EM3002 data) */
	if (store->kind == MB_DATA_DATA && store->sonar == MBSYS_SIMRAD2_EM3002) {
		/* get pointer to raw data structure */
		ping = (struct mbsys_simrad2_ping_struct *)store->ping2;

		/* zero the sidescan */
		for (int i = 0; i < MBSYS_SIMRAD2_MAXPIXELS; i++) {
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
			ss_cnt[i] = 0;
		}

		/* set scaling parameters */
		depthscale = 0.01 * ping->png_depth_res;
		depthoffset = 0.01 * ping->png_xducer_depth + 655.36 * ping->png_offset_multiplier;
		dacrscale = 0.01 * ping->png_distance_res;
		daloscale = 0.01 * ping->png_distance_res;
		reflscale = 0.5;
		// double ssoffset = 64.0;
		// if (store->sonar == MBSYS_SIMRAD2_EM300 && store->run_mode == 4) {
		//	if (depthscale * ping->png_depth[ping->png_nbeams / 2] > 3500.0 && ping->png_max_range > 19000 &&
		//	    ping->png_bsn + ping->png_bso < -60) {
				// ssoffset = 64.0 - 0.6 * (ping->png_bsn + ping->png_bso + 60);
		//	}
		//}

		/* get raw pixel size */
		if (store->sonar == MBSYS_SIMRAD2_EM120 || store->sonar == MBSYS_SIMRAD2_EM300 || store->sonar == MBSYS_SIMRAD2_EM1002 ||
		    store->sonar == MBSYS_SIMRAD2_EM2000 || store->sonar == MBSYS_SIMRAD2_EM3000 || store->sonar == MBSYS_SIMRAD2_EM710)
			ss_spacing = 750.0 / ping->png_sample_rate;
		else if (store->sonar == MBSYS_SIMRAD2_EM3000D_1 || store->sonar == MBSYS_SIMRAD2_EM3000D_2 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_3 || store->sonar == MBSYS_SIMRAD2_EM3000D_4 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_5 || store->sonar == MBSYS_SIMRAD2_EM3000D_6 ||
		         store->sonar == MBSYS_SIMRAD2_EM3000D_7 || store->sonar == MBSYS_SIMRAD2_EM3000D_8 ||
		         store->sonar == MBSYS_SIMRAD2_EM3002)
			ss_spacing = 750.0 / 14000;
		else if (store->sonar == MBSYS_SIMRAD2_EM12S || store->sonar == MBSYS_SIMRAD2_EM12D ||
		         store->sonar == MBSYS_SIMRAD2_EM121 || store->sonar == MBSYS_SIMRAD2_EM1000) {
			ss_spacing = 0.01 * ping->png_max_range;
		}

		/* get beam angle size */
		if (store->sonar == MBSYS_SIMRAD2_EM1000) {
			beamwidth = 2.5;
		}
		else {
			beamwidth = 0.1 * ((double)ping->png_tx);
		}

		/* get median depth */
		nbathsort = 0;
		for (int i = 0; i < ping->png_nbeams; i++) {
			if (mb_beam_ok(ping->png_beamflag[i])) {
				bathsort[nbathsort] = depthscale * ping->png_depth[i] + depthoffset;
				nbathsort++;
			}
		}

		/* get sidescan pixel size */
		if (!swath_width_set && nbathsort > 0) {
			(*swath_width) =
			    2.5 + MAX(90.0 - 0.01 * ping->png_depression[0], 90.0 - 0.01 * ping->png_depression[ping->png_nbeams - 1]);
			(*swath_width) = MAX((*swath_width), 60.0);
		}
		if (!pixel_size_set && nbathsort > 0) {
			qsort((char *)bathsort, nbathsort, sizeof(double), (void *)mb_double_compare);
			pixel_size_calc = 2 * tan(DTR * (*swath_width)) * bathsort[nbathsort / 2] / MBSYS_SIMRAD2_MAXPIXELS;
			pixel_size_calc = MAX(pixel_size_calc, bathsort[nbathsort / 2] * sin(DTR * 0.1));
			if ((*pixel_size) <= 0.0)
				(*pixel_size) = pixel_size_calc;
			else if (0.95 * (*pixel_size) > pixel_size_calc)
				(*pixel_size) = 0.95 * (*pixel_size);
			else if (1.05 * (*pixel_size) < pixel_size_calc)
				(*pixel_size) = 1.05 * (*pixel_size);
			else
				(*pixel_size) = pixel_size_calc;
		}

		/* get pixel interpolation */
		pixel_int_use = pixel_int + 1;

		/* check that sidescan can be used */
		/* get times of bath and sidescan records */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &bath_time_d);
		time_i[0] = ping->png_ss_date / 10000;
		time_i[1] = (ping->png_ss_date % 10000) / 100;
		time_i[2] = ping->png_ss_date % 100;
		time_i[3] = ping->png_ss_msec / 3600000;
		time_i[4] = (ping->png_ss_msec % 3600000) / 60000;
		time_i[5] = (ping->png_ss_msec % 60000) / 1000;
		time_i[6] = (ping->png_ss_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &ss_time_d);
		ss_ok = true;
		if (ping->png_nbeams < ping->png_nbeams_ss || ping->png_nbeams > ping->png_nbeams_ss + 1) {
			ss_ok = false;
			if (verbose > 0)
				fprintf(stderr,
				        "%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Sidescan ignored: num bath beams != num ss beams: %d %d\n",
				        __func__, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
				        ping->png_nbeams, ping->png_nbeams_ss);
		}
		else if (ping->png_nbeams == ping->png_nbeams_ss) {
			for (int i = 0; i < ping->png_nbeams; i++) {
				if (ping->png_beam_num[i] != ping->png_beam_index[i] + 1 &&
				    ping->png_beam_num[i] != ping->png_beam_index[i] - 1) {
					ss_ok = false;
					if (verbose > 0)
						fprintf(stderr,
						        "%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Sidescan ignored: bath and ss beam indexes don't "
						        "match: : %d %d %d\n",
						        __func__, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], i,
						        ping->png_beam_num[i], ping->png_beam_index[i]);
				}
			}
		}

		/* loop over raw sidescan, putting each raw pixel into
		    the binning arrays */
		if (ss_ok)
			for (int i = 0; i < ping->png_nbeams_ss; i++) {
				beam_ss = &ping->png_ssraw[ping->png_start_sample[i]];
				if (mb_beam_ok(ping->png_beamflag[i])) {
					if (ping->png_beam_samples[i] > 0) {
						depth = depthscale * ping->png_depth[i];
						xtrack = dacrscale * ping->png_acrosstrack[i];
						range = sqrt(depth * depth + xtrack * xtrack);
						angle = 90.0 - 0.01 * ping->png_depression[i];
						beam_foot = range * sin(DTR * beamwidth) / cos(DTR * angle);
						sint = fabs(sin(DTR * angle));
						if (sint < ping->png_beam_samples[i] * ss_spacing / beam_foot)
							ss_spacing_use = beam_foot / ping->png_beam_samples[i];
						else
							ss_spacing_use = ss_spacing / sint;
					}
					for (int k = 0; k < ping->png_beam_samples[i]; k++) {
						if (beam_ss[k] != EM2_INVALID_AMP) {
							/* locate based on range */
							if (k == ping->png_center_sample[i]) {
								xtrackss = xtrack;
							}
							else if (i == ping->png_nbeams_ss - 1 || (k <= ping->png_center_sample[i] && i != 0)) {
								if (ping->png_range[i] != ping->png_range[i - 1]) {
									xtrackss = dacrscale * ping->png_acrosstrack[i] +
									           (dacrscale * ping->png_acrosstrack[i] - dacrscale * ping->png_acrosstrack[i - 1]) *
									               2 * ((double)(k - ping->png_center_sample[i])) /
									               fabs((double)(ping->png_range[i] - ping->png_range[i - 1]));
								}
								else {
									xtrackss = xtrack + ss_spacing_use * (k - ping->png_center_sample[i]);
								}
							}
							else {
								if (ping->png_range[i] != ping->png_range[i + 1]) {
									xtrackss = dacrscale * ping->png_acrosstrack[i] +
									           (dacrscale * ping->png_acrosstrack[i + 1] - dacrscale * ping->png_acrosstrack[i]) *
									               2 * ((double)(k - ping->png_center_sample[i])) /
									               fabs((double)(ping->png_range[i + 1] - ping->png_range[i]));
								}
								else {
									xtrackss = xtrack + ss_spacing_use * (k - ping->png_center_sample[i]);
								}
							}
							// TODO(schwehr): BUG.  This xtrackss overwrites all the previous ones.
							xtrackss = xtrack + ss_spacing_use * (k - ping->png_center_sample[i]);
							const int kk = MBSYS_SIMRAD2_MAXPIXELS / 2 + (int)(xtrackss / (*pixel_size));
							if (kk > 0 && kk < MBSYS_SIMRAD2_MAXPIXELS) {
								ss[kk] += reflscale * ((double)beam_ss[k]);
								ssalongtrack[kk] += daloscale * ping->png_alongtrack[i];
								ss_cnt[kk]++;
							}
						}
					}
				}
			}

		/* average the sidescan */
		first = MBSYS_SIMRAD2_MAXPIXELS;
		last = -1;
		for (int k = 0; k < MBSYS_SIMRAD2_MAXPIXELS; k++) {
			if (ss_cnt[k] > 0) {
				ss[k] /= ss_cnt[k];
				ssalongtrack[k] /= ss_cnt[k];
				ssacrosstrack[k] = (k - MBSYS_SIMRAD2_MAXPIXELS / 2) * (*pixel_size);
				first = MIN(first, k);
				last = k;
			}
			else
				ss[k] = MB_SIDESCAN_NULL;
		}

		/* interpolate the sidescan */
		k1 = first;
		k2 = first;
		for (int k = first + 1; k < last; k++) {
			if (ss_cnt[k] <= 0) {
				if (k2 <= k) {
					k2 = k + 1;
					while (ss_cnt[k2] <= 0 && k2 < last)
						k2++;
				}
				if (k2 - k1 <= pixel_int_use) {
					ss[k] = ss[k1] + (ss[k2] - ss[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
					ssacrosstrack[k] = (k - MBSYS_SIMRAD2_MAXPIXELS / 2) * (*pixel_size);
					ssalongtrack[k] =
					    ssalongtrack[k1] + (ssalongtrack[k2] - ssalongtrack[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
				}
			}
			else {
				k1 = k;
			}
		}

		/* insert the new sidescan into store */
		ping->png_pixel_size = (int)(100 * (*pixel_size));
		if (last > first)
			ping->png_pixels_ss = MBSYS_SIMRAD2_MAXPIXELS;
		else
			ping->png_pixels_ss = 0;
		for (int i = 0; i < MBSYS_SIMRAD2_MAXPIXELS; i++) {
			if (ss[i] > MB_SIDESCAN_NULL) {
				ping->png_ss[i] = (short)(100 * ss[i]);
				ping->png_ssalongtrack[i] = (short)(ssalongtrack[i] / daloscale);
			}
			else {
				ping->png_ss[i] = EM2_INVALID_AMP;
				ping->png_ssalongtrack[i] = EM2_INVALID_AMP;
			}
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Sidescan regenerated in <%s>\n", __func__);
			fprintf(stderr, "dbg2       png_nbeams_ss: %d\n", ping->png_nbeams_ss);
			for (int i = 0; i < ping->png_nbeams_ss; i++)
				fprintf(stderr, "dbg2       beam:%d  flag:%3d  bath:%d  amp:%d  acrosstrack:%d  alongtrack:%d\n",
				        ping->png_beam_num[i], ping->png_beamflag[i], ping->png_depth[i], ping->png_amp[i],
				        ping->png_acrosstrack[i], ping->png_alongtrack[i]);
			fprintf(stderr, "dbg2       pixels_ss:  %d\n", MBSYS_SIMRAD2_MAXPIXELS);
			for (int i = 0; i < MBSYS_SIMRAD2_MAXPIXELS; i++)
				fprintf(stderr, "dbg2       pixel:%4d  cnt:%3d  ss:%10f  xtrack:%10f  ltrack:%10f\n", i, ss_cnt[i], ss[i],
				        ssacrosstrack[i], ssalongtrack[i]);
			fprintf(stderr, "dbg2       pixels_ss:  %d\n", ping->png_pixels_ss);
			for (int i = 0; i < MBSYS_SIMRAD2_MAXPIXELS; i++)
				fprintf(stderr, "dbg2       pixel:%4d  ss:%8d  ltrack:%8d\n", i, ping->png_ss[i], ping->png_ssalongtrack[i]);
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       pixel_size:      %f\n", *pixel_size);
		fprintf(stderr, "dbg2       swath_width:     %f\n", *swath_width);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
