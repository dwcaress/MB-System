/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_reson8k.c	3.00	8/20/94
 *
 *    Copyright (c) 2001-2023 by
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
 * mbsys_reson.h defines the data structures used by MBIO functions
 * to store data from Reson SeaBat 8101 and other 8K series
 * multibeam sonar systems.
 * The data formats which are commonly used to store Reson 8K
 * data in files include
 *      MBF_xtfr8101 : MBIO ID 84
 *
 * Author:	D. W. Caress
 * Date:	September 3, 2001
 *
 *
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_reson8k.h"

/*--------------------------------------------------------------------*/
int mbsys_reson8k_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* allocate memory for data structure */
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_reson8k_struct), store_ptr, error);

	/* get data structure pointer */
	struct mbsys_reson8k_struct *store = (struct mbsys_reson8k_struct *)*store_ptr;

	/* initialize everything */

	/* type of data record */
	store->kind = MB_DATA_NONE; /* Data kind */

	/* type of sonar */
	store->sonar = MBSYS_RESON8K_UNKNOWN; /* Type of Reson sonar */

	/* parameter info */
	store->MBOffsetX = 0.0;
	store->MBOffsetY = 0.0;
	store->MBOffsetZ = 0.0;
	store->NavLatency = 0.0;     /* GPS_time_received - GPS_time_sent (sec) */
	store->NavOffsetY = 0.0;     /* Nav offset (m) */
	store->NavOffsetX = 0.0;     /* Nav offset (m) */
	store->NavOffsetZ = 0.0;     /* Nav z offset (m) */
	store->NavOffsetYaw = 0.0;   /* Heading offset (m) */
	store->MRUOffsetY = 0.0;     /* Multibeam MRU y offset (m) */
	store->MRUOffsetX = 0.0;     /* Multibeam MRU x offset (m) */
	store->MRUOffsetZ = 0.0;     /* Multibeam MRU z offset (m) */
	store->MRUOffsetPitch = 0.0; /* Multibeam MRU pitch offset (degrees) */
	store->MRUOffsetRoll = 0.0;  /* Multibeam MRU roll offset (degrees) */

	/* nav data */
	store->nav_time_d = 0.0;
	store->nav_longitude = 0.0;
	store->nav_latitude = 0.0;
	store->nav_heading = 0.0;

	/* attitude data */
	store->att_timetag = 0.0;
	store->att_heading = 0.0;
	store->att_heave = 0.0;
	store->att_roll = 0.0;
	store->att_pitch = 0.0;

	/* comment */
	store->comment[0] = '\0';

	/* sound velocity profile */
	store->svp_time_d = 0.0;
	store->svp_num = 0;
	for (int i = 0; i < store->svp_num; i++) {
		store->svp_depth[0] = 0.0; /* meters */
		store->svp_vel[0] = 0.0;   /* meters/sec */
	}

	/* survey data */
	store->png_time_d = 0.0;
	store->png_latency = 0.0;
	store->png_latitude = 0.0;
	store->png_longitude = 0.0;
	store->png_roll = 0.0;
	store->png_pitch = 0.0;
	store->png_heading = 0.0;
	store->png_heave = 0.0;

	store->packet_type = 0;    /* identifier for packet type  */
	store->packet_subtype = 0; /* identifier for packet subtype */
	                           /* for dual head system, most significant bit (bit 7) */
	                           /* indicates which sonar head to associate with packet */
	                           /* 	head 1 - bit 7 set to 0 */
	                           /* 	head 2 -	bit 7 set to 1 		 */
	store->latency = 0;        /* time from ping to output (milliseconds) */
	store->Seconds = 0;        /* seconds since 00:00:00, 1 January 1970 */
	store->Millisecs = 0;      /* milliseconds, LSB = 1 ms */
	store->ping_number = 0;    /* sequential ping number from sonar startup/reset */
	store->sonar_id = 0;       /* least significant four bytes of Ethernet address */
	store->sonar_model = 0;    /* coded model number of sonar */
	store->frequency = 0;      /* sonar frequency in KHz */
	store->velocity = 0;       /* programmed sound velocity (LSB = 1 m/sec) */
	store->sample_rate = 0;    /* A/D sample rate (samples per second) */
	store->ping_rate = 0;      /* Ping rate (pings per second * 1000) */
	store->range_set = 0;      /* range setting for SeaBat (meters ) */
	store->power = 0;          /* power setting for SeaBat  	 */
	                           /* bits	0-4 -	power (0 - 8) */
	store->gain = 0;           /* gain setting for SeaBat */
	/* bits	0-6 -	gain (1 - 45) */
	/* bit 	14	(0 = fixed, 1 = tvg) */
	/* bit	15	(0 = manual, 1 = auto) */
	store->pulse_width = 0;          /* transmit pulse width (microseconds) */
	store->tvg_spread = 0;           /* spreading coefficient for tvg * 4  */
	                                 /* valid values = 0 to 240 (0.0 to 60.0 in 0.25 steps) */
	store->tvg_absorp = 0;           /* absorption coefficient for tvg */
	store->projector_type = 0;       /* bits 0-4 = projector type */
	                                 /* 0 = stick projector */
	                                 /* 1 = array face */
	                                 /* 2 = ER projector */
	                                 /* bit 7 - pitch steering (1=enabled, 0=disabled) */
	store->projector_beam_width = 0; /* along track transmit beam width (degrees * 10) */
	store->beam_width_num = 0;       /* cross track receive beam width numerator */
	store->beam_width_denom = 0;     /* cross track receive beam width denominator */
	                                 /* beam width degrees = numerator / denominator */
	store->projector_angle = 0;      /* projector pitch steering angle (degrees * 100) */
	store->min_range = 0;            /* sonar filter settings */
	store->max_range = 0;
	store->min_depth = 0;
	store->max_depth = 0;
	store->filters_active = 0; /* range/depth filters active  */
	                           /* bit 0 - range filter (0 = off, 1 = active) */
	                           /* bit 1 - depth filter (0 = off, 1 = active) */
	store->temperature = 0;    /* temperature at sonar head (deg C * 10) */
	store->beam_count = 0;     /* number of sets of beam data in packet */
	for (int i = 0; i < MBSYS_RESON8K_MAXBEAMS; i++)
		store->range[i] = 0; /* range for beam where n = Beam Count */
	                         /* range units = sample cells * 4 */
	for (int i = 0; i < MBSYS_RESON8K_MAXBEAMS / 2 + 1; i++)
		store->quality[i] = 0; /* packed quality array (two 4 bit values/char) */
	                           /* cnt = n/2 if beam count even, n/2+1 if odd */
	                           /* cnt then rounded up to next even number */
	                           /* e.g. if beam count=101, cnt=52  */
	                           /* unused trailing quality values set to zero */
	                           /* bit 0 - brightness test (0=failed, 1=passed) */
	                           /* bit 1 - colinearity test (0=failed, 1=passed) */
	                           /* bit 2 - amplitude bottom detect used */
	                           /* bit 3 - phase bottom detect used */
	                           /* bottom detect can be amplitude, phase or both */
	for (int i = 0; i < MBSYS_RESON8K_MAXBEAMS; i++)
		store->intensity[i] = 0;    /* intensities at bottom detect  */
	store->ssrawtimedelay = 0.0;    /* raw sidescan delay (sec) */
	store->ssrawtimeduration = 0.0; /* raw sidescan duration (sec) */
	store->ssrawbottompick = 0.0;   /* bottom pick time (sec) */
	store->ssrawportsamples = 0;    /* number of port raw sidescan samples */
	store->ssrawstbdsamples = 0;    /* number of stbd raw sidescan samples */
	for (int i = 0; i < MBSYS_RESON8K_MAXRAWPIXELS; i++) {
		store->ssrawport[i] = 0; /* raw port sidescan */
		store->ssrawstbd[i] = 0; /* raw starboard sidescan */
	}

	store->beams_bath = 0;
	store->beams_amp = 0;
	store->pixels_ss = 0;
	store->pixel_size = 0.0;
	for (int i = 0; i < MBSYS_RESON8K_MAXBEAMS; i++) {
		store->beamflag[i] = MB_FLAG_NULL; /* beamflags */
		store->bath[i] = 0.0;              /* bathymetry (m) */
		store->amp[i] = 0.0;               /* amplitude */
		store->bath_acrosstrack[i] = 0.0;  /* acrosstrack distance (m) */
		store->bath_alongtrack[i] = 0.0;   /* alongtrack distance (m) */
	}
	for (int i = 0; i < MBSYS_RESON8K_MAXPIXELS; i++) {
		store->ss[i] = 0.0;            /* sidescan */
		store->ss_alongtrack[i] = 0.0; /* alongtrack distance (m) */
	}

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
int mbsys_reson8k_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
	}

	/* deallocate memory for data structure */
	const int status = mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

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
int mbsys_reson8k_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                             int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	struct mbsys_reson8k_struct *store = (struct mbsys_reson8k_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get beam and pixel numbers */
		*nbath = store->beams_bath;
		*namp = store->beams_amp;
		*nss = store->pixels_ss;
		;
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
int mbsys_reson8k_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
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
	struct mbsys_reson8k_struct *store = (struct mbsys_reson8k_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		*time_d = store->png_time_d;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		*navlon = store->png_longitude;
		*navlat = store->png_latitude;

		/* get heading */
		*heading = store->png_heading;

		/* get speed  */
		*speed = store->png_speed;

		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_ltrack = 1.5;
		mb_io_ptr->beamwidth_xtrack = 1.5;

		/* read distance and depth values into storage arrays */
		*nbath = store->beams_bath;
		*namp = store->beams_amp;
		*nss = store->pixels_ss;
		;
		for (int i = 0; i < *nbath; i++) {
			beamflag[i] = store->beamflag[i];
			bath[i] = store->bath[i];
			bathacrosstrack[i] = store->bath_acrosstrack[i];
			bathalongtrack[i] = store->bath_alongtrack[i];
		}
		for (int i = 0; i < *namp; i++) {
			amp[i] = store->intensity[i];
		}
		for (int i = 0; i < *nss; i++) {
			ss[i] = store->ss[i];
			ssacrosstrack[i] = store->pixel_size * (i - store->pixels_ss / 2);
			ssalongtrack[i] = store->ss_alongtrack[i];
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
		}

		/* done translating values */
	}

	/* extract nav from structure */
	else if (*kind == MB_DATA_NAV) {
		/* get time */
		*time_d = store->nav_time_d;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		*navlon = store->nav_longitude;
		*navlat = store->nav_latitude;

		/* get heading */
		*heading = store->nav_heading;

		/* get speed  */
		*speed = 0.0;

		/* read distance and depth values into storage arrays */
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
    strncpy(comment, store->comment, MIN(MB_COMMENT_MAXLINE, MBSYS_RESON8K_COMMENT_LENGTH) - 1);

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
int mbsys_reson8k_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
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
	}
	if (verbose >= 2 && kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}

	struct mbsys_reson8k_struct *store = (struct mbsys_reson8k_struct *)store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		store->png_time_d = time_d;

		/*get navigation */
		store->png_longitude = navlon;
		store->png_latitude = navlat;

		/* get heading */
		store->png_heading = heading;

		/* get speed */
		store->png_speed = speed;

		/* insert distance and depth values into storage arrays */
		store->beams_bath = nbath;
		store->beams_amp = namp;
		store->pixels_ss = nss;
		if (store->pixels_ss > 0)
			store->pixel_size = (ssacrosstrack[store->pixels_ss - 1] - ssacrosstrack[0]) / store->pixels_ss;
		for (int i = 0; i < nbath; i++) {
			store->beamflag[i] = beamflag[i];
			store->bath[i] = bath[i];
			store->bath_acrosstrack[i] = bathacrosstrack[i];
			store->bath_alongtrack[i] = bathalongtrack[i];
		}
		for (int i = 0; i < namp; i++) {
			store->intensity[i] = (unsigned short)amp[i];
		}
		for (int i = 0; i < nss; i++) {
			store->ss[i] = ss[i];
			store->ss_alongtrack[i] = ssalongtrack[i];
		}
	}

	/* insert nav in structure */
	else if (store->kind == MB_DATA_NAV) {
		/* get time */
		store->nav_time_d = time_d;

		/*get navigation */
		store->nav_longitude = navlon;
		store->nav_latitude = navlat;

		/* get heading */
		store->nav_heading = heading;

		/* get speed */
	}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT) {
    memset((void *)store->comment, 0, MBSYS_RESON8K_COMMENT_LENGTH);
    strncpy(store->comment, comment, MIN(MBSYS_RESON8K_COMMENT_LENGTH, MB_COMMENT_MAXLINE) - 1);
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
int mbsys_reson8k_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
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

	struct mbsys_reson8k_struct *store = (struct mbsys_reson8k_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get nbeams */
		*nbeams = store->beams_bath;

		/* get depth offset (heave + transducer_depth) */
		/* const double heave_use = store->png_heave; */
		*draft = store->MBOffsetZ;
		*ssv = (double)store->velocity;

		/* get travel times, angles */
		const double ttscale = 0.25 / store->sample_rate;
		const int icenter = store->beams_bath / 2;
		const double angscale = ((double)store->beam_width_num) / ((double)store->beam_width_denom);
		for (int i = 0; i < *nbeams; i++) {
			ttimes[i] = ttscale * store->range[i];
			const double angle = 90.0 + (icenter - i) * angscale + store->png_roll;
			const double pitch = store->png_pitch;
			mb_rollpitch_to_takeoff(verbose, pitch, angle, &angles[i], &angles_forward[i], error);
			angles_null[i] = angles[i];
			heave[i] = store->png_heave;
			alongtrack_offset[i] = 0.0;
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
			fprintf(stderr, "dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  heave:%f  ltrk_off:%f\n", i,
			        ttimes[i], angles[i], angles_forward[i], angles_null[i], heave[i], alongtrack_offset[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson8k_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       detects:    %p\n", (void *)detects);
	}

	struct mbsys_reson8k_struct *store = (struct mbsys_reson8k_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get nbeams */
		*nbeams = store->beams_bath;

		/* get detects */
		for (int i = 0; i < *nbeams; i++) {
			detects[i] = MB_DETECT_AMPLITUDE;
		}
		for (int i = 0; i < *nbeams; i++) {
			/* get beamflag */
			int detect;
			if (i % 2 == 0)
				detect = ((store->quality[i / 2]) & 15) & 12;
			else
				detect = ((store->quality[i / 2] >> 4) & 15) & 12;

			if (detect & 4)
				detects[i] = MB_DETECT_AMPLITUDE;
			else if (detect & 8)
				detects[i] = MB_DETECT_PHASE;
			else
				detects[i] = MB_DETECT_UNKNOWN;
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
int mbsys_reson8k_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                   double *altitude, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	struct mbsys_reson8k_struct *store = (struct mbsys_reson8k_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		double bath_best = 0.0;
		if (mb_beam_ok(store->beamflag[store->beams_bath / 2]))
			bath_best = store->bath[store->beams_bath / 2];
		else {
			double xtrack_min = 99999999.9;
			for (int i = 0; i < store->beams_bath; i++) {
				if (mb_beam_ok(store->beamflag[i]) && fabs(store->bath_acrosstrack[i]) < xtrack_min) {
					xtrack_min = fabs(store->bath_acrosstrack[i]);
					bath_best = store->bath[i];
				}
			}
		}
		if (bath_best == 0.0) {
			double xtrack_min = 99999999.9;
			for (int i = 0; i < store->beams_bath; i++) {
				if (store->beamflag[i] != MB_FLAG_NULL && fabs(store->bath_acrosstrack[i]) < xtrack_min) {
					xtrack_min = fabs(store->bath_acrosstrack[i]);
					bath_best = store->bath[i];
				}
			}
		}
		*transducer_depth = store->MBOffsetZ + store->png_heave;
		*altitude = bath_best - *transducer_depth;

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
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson8k_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                              double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                              double *pitch, double *heave, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	struct mbsys_reson8k_struct *store = (struct mbsys_reson8k_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		*time_d = store->png_time_d;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		*navlon = store->png_longitude;
		*navlat = store->png_latitude;

		/* get heading */
		*heading = store->png_heading;

		/* get speed  */
		*speed = store->png_speed;

		/* get draft  */
		*draft = store->MBOffsetZ;

		/* get roll pitch and heave */
		*roll = store->png_roll;
		*pitch = store->png_pitch;
		*heave = store->png_heave;

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

	/* extract nav from structure */
	else if (*kind == MB_DATA_NAV) {
		/* get time */
		*time_d = store->nav_time_d;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		*navlon = store->nav_longitude;
		*navlat = store->nav_latitude;

		/* get heading */
		*heading = store->nav_heading;

		/* get speed  */
		*speed = 0.0;
		/* get time */

		/* get draft  */
		*draft = store->MBOffsetZ;

		/* get roll pitch and heave */
		*roll = store->png_roll;
		*pitch = store->png_pitch;
		*heave = store->png_heave;

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
int mbsys_reson8k_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
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

	struct mbsys_reson8k_struct *store = (struct mbsys_reson8k_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		store->png_time_d = time_d;

		/*get navigation */
		store->png_longitude = navlon;
		store->png_latitude = navlat;

		/* get heading */
		store->png_heading = heading;

		/* get speed */
		store->png_speed = speed;

		/* get draft  */
		store->MBOffsetZ = draft;

		/* get roll pitch and heave */
		store->png_roll = roll;
		store->png_pitch = pitch;
		store->png_heave = heave;
	}

	/* insert nav in structure */
	else if (store->kind == MB_DATA_NAV) {
		/* get time */
		store->nav_time_d = time_d;

		/*get navigation */
		store->nav_longitude = navlon;
		store->nav_latitude = navlat;

		/* get heading */
		store->nav_heading = heading;

		/* get speed */

		/* get draft  */
		store->MBOffsetZ = draft;
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
int mbsys_reson8k_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                              int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	struct mbsys_reson8k_struct *store = (struct mbsys_reson8k_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_VELOCITY_PROFILE) {
		/* get number of depth-velocity pairs */
		*nsvp = store->svp_num;

		/* get profile */
		for (int i = 0; i < *nsvp; i++) {
			depth[i] = 0.1 * store->svp_depth[i];
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
int mbsys_reson8k_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity,
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

	struct mbsys_reson8k_struct *store = (struct mbsys_reson8k_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE) {
		/* get number of depth-velocity pairs */
		store->svp_num = MIN(nsvp, MBSYS_RESON8K_MAXSVP);

		/* get profile */
		for (int i = 0; i < store->svp_num; i++) {
			store->svp_depth[i] = (int)(10 * depth[i]);
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
int mbsys_reson8k_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
	}

	struct mbsys_reson8k_struct *store = (struct mbsys_reson8k_struct *)store_ptr;
	struct mbsys_reson8k_struct *copy = (struct mbsys_reson8k_struct *)copy_ptr;

	/* copy the data */
	*copy = *store;

	const int status = MB_SUCCESS;

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
int mbsys_reson8k_makess(int verbose, void *mbio_ptr, void *store_ptr, int pixel_size_set, double *pixel_size,
                         int swath_width_set, double *swath_width, int *error) {
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
	}

	/* get data structure pointer */
	struct mbsys_reson8k_struct *store = (struct mbsys_reson8k_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA && store->ssrawstbdsamples > 0 && store->ssrawportsamples > 0) {
		double ss[MBSYS_RESON8K_MAXPIXELS];
		int ss_cnt[MBSYS_RESON8K_MAXPIXELS];
		double ssacrosstrack[MBSYS_RESON8K_MAXPIXELS];
		double ssalongtrack[MBSYS_RESON8K_MAXPIXELS];

		/* zero the sidescan */
		for (int i = 0; i < MBSYS_RESON8K_MAXPIXELS; i++) {
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
			ss_cnt[i] = 0;
		}

		/* get raw pixel size */
		const double ss_spacing = store->ssrawtimeduration / (store->ssrawportsamples - 1);

		/* get median depth */
		int nbathsort = 0;
		int istart = store->beams_bath;
		int iend = -1;
		double bathsort[MBSYS_RESON8K_MAXBEAMS];
		for (int i = 0; i < store->beams_bath; i++) {
			if (mb_beam_ok(store->beamflag[i])) {
				bathsort[nbathsort] = store->bath[i];
				nbathsort++;
			}
			if (store->beamflag[i] != MB_FLAG_NULL) {
				istart = MIN(istart, i);
				iend = MAX(iend, i);
			}
		}

		/* get sidescan pixel size */
		const double angscale = ((double)store->beam_width_num) / ((double)store->beam_width_denom);
		const double ttscale = 0.25 / store->sample_rate;
		const int icenter = store->beams_bath / 2;
		if (!swath_width_set && nbathsort > 0) {
			double anglestart = fabs((icenter - istart) * angscale + store->png_roll);
			(*swath_width) = anglestart;
			const double angleend = fabs((icenter - iend) * angscale + store->png_roll);
			(*swath_width) = MAX(anglestart, angleend);
			(*swath_width) = MAX((*swath_width), 60.0);
		}
		if (!pixel_size_set && nbathsort > 0) {
			qsort((char *)bathsort, nbathsort, sizeof(double), (void *)mb_double_compare);
			double pixel_size_calc = 2 * tan(DTR * (*swath_width)) * bathsort[nbathsort / 2] / MBSYS_RESON8K_MAXPIXELS;
			pixel_size_calc = MAX(pixel_size_calc, bathsort[nbathsort / 2] * sin(DTR * 0.1));
			pixel_size_calc = MIN(pixel_size_calc, ((double)(2.0 * store->range_set) / MBSYS_RESON8K_MAXPIXELS));
			if ((*pixel_size) <= 0.0)
				(*pixel_size) = pixel_size_calc;
			else if (0.95 * (*pixel_size) > pixel_size_calc)
				(*pixel_size) = 0.95 * (*pixel_size);
			else if (1.05 * (*pixel_size) < pixel_size_calc)
				(*pixel_size) = 1.05 * (*pixel_size);
			else
				(*pixel_size) = pixel_size_calc;
		}

		/* loop over the port beams, figuring out
		    acrosstrack distance for each raw sidescan sample */
		int goodbeam1 = -1;
		int goodbeam2 = -1;
		for (int i = store->beams_bath / 2; i >= 0; i--) {
			if (mb_beam_ok(store->beamflag[i])) {
				goodbeam1 = goodbeam2;
				goodbeam2 = i;
				if (goodbeam2 >= 0 && goodbeam1 >= 0) {
					const int pixel1 = (ttscale * store->range[goodbeam1] - store->ssrawtimedelay) / ss_spacing;
					const int pixel2 = (ttscale * store->range[goodbeam2] - store->ssrawtimedelay) / ss_spacing;
					for (int ipixel = pixel1; ipixel < pixel2; ipixel++) {
						const double xtrackss = store->bath_acrosstrack[goodbeam1] +
						           ((double)(ipixel - pixel1)) / ((double)(pixel2 - pixel1)) *
						               (store->bath_acrosstrack[goodbeam2] - store->bath_acrosstrack[goodbeam1]);
						const double ltrackss = store->bath_alongtrack[goodbeam1] +
						           ((double)(ipixel - pixel1)) / ((double)(pixel2 - pixel1)) *
						               (store->bath_alongtrack[goodbeam2] - store->bath_alongtrack[goodbeam1]);
						const int kk = MBSYS_RESON8K_MAXPIXELS / 2 + (int)(xtrackss / (*pixel_size));
						if (kk > 0 && kk < MBSYS_RESON8K_MAXPIXELS) {
							ss[kk] += store->ssrawport[ipixel];
							ssalongtrack[kk] += ltrackss;
							ss_cnt[kk]++;
						}
					}
				}
			}
		}

		/* loop over the starboard beams, figuring out
		    acrosstrack distance for each raw sidescan sample */
		goodbeam1 = -1;
		goodbeam2 = -1;
		for (int i = store->beams_bath / 2; i < store->beams_bath; i++) {
			if (mb_beam_ok(store->beamflag[i])) {
				goodbeam1 = goodbeam2;
				goodbeam2 = i;
				if (goodbeam2 >= 0 && goodbeam1 >= 0) {
					const int pixel1 = (ttscale * store->range[goodbeam1] - store->ssrawtimedelay) / ss_spacing;
					const int pixel2 = (ttscale * store->range[goodbeam2] - store->ssrawtimedelay) / ss_spacing;
					for (int ipixel = pixel1; ipixel < pixel2; ipixel++) {
						const double xtrackss = store->bath_acrosstrack[goodbeam1] +
						           ((double)(ipixel - pixel1)) / ((double)(pixel2 - pixel1)) *
						               (store->bath_acrosstrack[goodbeam2] - store->bath_acrosstrack[goodbeam1]);
						const double ltrackss = store->bath_alongtrack[goodbeam1] +
						           ((double)(ipixel - pixel1)) / ((double)(pixel2 - pixel1)) *
						               (store->bath_alongtrack[goodbeam2] - store->bath_alongtrack[goodbeam1]);
						const int kk = MBSYS_RESON8K_MAXPIXELS / 2 + (int)(xtrackss / (*pixel_size));
						if (kk > 0 && kk < MBSYS_RESON8K_MAXPIXELS) {
							ss[kk] += store->ssrawstbd[ipixel];
							ssalongtrack[kk] += ltrackss;
							ss_cnt[kk]++;
						}
					}
				}
			}
		}

		/* average the sidescan */
		int first = MBSYS_RESON8K_MAXPIXELS;
		int last = -1;
		for (int k = 0; k < MBSYS_RESON8K_MAXPIXELS; k++) {
			if (ss_cnt[k] > 0) {
				ss[k] /= ss_cnt[k];
				ssalongtrack[k] /= ss_cnt[k];
				ssacrosstrack[k] = (k - MBSYS_RESON8K_MAXPIXELS / 2) * (*pixel_size);
				first = MIN(first, k);
				last = k;
			}
			else
				ss[k] = MB_SIDESCAN_NULL;
		}

		/* interpolate the sidescan */
		int k1 = first;
		int k2 = first;
		for (int k = first + 1; k < last; k++) {
			if (ss_cnt[k] <= 0) {
				if (k2 <= k) {
					k2 = k + 1;
					while (ss_cnt[k2] <= 0 && k2 < last)
						k2++;
				}
				ss[k] = ss[k1] + (ss[k2] - ss[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
				ssacrosstrack[k] = (k - MBSYS_RESON8K_MAXPIXELS / 2) * (*pixel_size);
				ssalongtrack[k] =
				    ssalongtrack[k1] + (ssalongtrack[k2] - ssalongtrack[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
			}
			else {
				k1 = k;
			}
		}

		/* insert the new sidescan into store */
		store->pixel_size = (*pixel_size);
		if (last > first)
			store->pixels_ss = MBSYS_RESON8K_MAXPIXELS;
		else
			store->pixels_ss = 0;
		for (int i = 0; i < MBSYS_RESON8K_MAXPIXELS; i++) {
			store->ss[i] = ss[i];
			store->ss_alongtrack[i] = ssalongtrack[i];
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Sidescan regenerated in <%s>\n", __func__);
			fprintf(stderr, "dbg2       beams_bath:    %d\n", store->beams_bath);
			for (int i = 0; i < store->beams_bath; i++)
				fprintf(stderr, "dbg2       beam:%d  flag:%3d  bath:%10f  amp:%10f  acrosstrack:%10f  alongtrack:%10f\n", i,
				        store->beamflag[i], store->bath[i], store->amp[i], store->bath_acrosstrack[i], store->bath_alongtrack[i]);
			fprintf(stderr, "dbg2       pixels_ss:  %d\n", MBSYS_RESON8K_MAXPIXELS);
			for (int i = 0; i < MBSYS_RESON8K_MAXPIXELS; i++)
				fprintf(stderr, "dbg2       pixel:%4d  cnt:%3d  ss:%10f  xtrack:%10f  ltrack:%10f\n", i, ss_cnt[i], ss[i],
				        ssacrosstrack[i], ssalongtrack[i]);
			fprintf(stderr, "dbg2       pixels_ss:  %d\n", store->pixels_ss);
			for (int i = 0; i < MBSYS_RESON8K_MAXPIXELS; i++)
				fprintf(stderr, "dbg2       pixel:%4d  ss:%10f  xtrack:%10f  ltrack:%10f\n", i, store->ss[i],
				        store->ss_acrosstrack[i], store->ss_alongtrack[i]);
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
