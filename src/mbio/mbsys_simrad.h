/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_simrad.h	8/5/94
 *
 *    Copyright (c) 1994-2024 by
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
 * mbsys_simrad.h defines the MBIO data structures for handling data from
 * old (pre-1997) Simrad multibeam sonars (e.g. EM950, EM1000, EM12S,
 * EM12D). The data formats associated with Simrad multibeams
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
 *
 *
 * Author:	D. W. Caress (L-DEO)
 * Date:	August 5, 1994
 *
 */
/*
 * Notes on the MBSYS_SIMRAD data structure:
 *   1. Simrad multibeam systems output datagrams which are
 *      a combination of ascii and binary.
 *   2. Simrad multibeam sonars output both bathymetry
 *      and amplitude information for beams and sidescan information
 *      with a higher resolution than the bathymetry and amplitude.
 *   3. There are five systems of interest:
 *         EM-1000:  Shallow water system with up to 60 beams of
 *                   bathymetry and up to 523 sidescan samples per
 *                   bathymetry beam.
 *         EM-12S:   Single array deep water system with up to 81
 *                   beams of bathymetry and up to 523 sidescan
 *                   samples per bathymetry beam.
 *         EM-12D:   Double array deep water system with up to
 *                   81 beams of bathymetry (port and starboard
 *                   calculated and recorded separately) and up
 *                   to 523 sidescan samples per bathymetry beam.
 *         EM-121:   Single array deep water system with up to 121
 *                   beams of bathymetry and up to 523 sidescan
 *                   samples per bathymetry beam.
 *   4. Each telegram is preceded by a two byte start code and
 *      followed by a three byte end code consisting of 0x03
 *      followed by two bytes representing the checksum for
 *      the data bytes.  MB-System does not calculate checksums
 *      and puts 0's in the checksum bytes.
 *   5. The relevant telegram start codes, types, and sizes are:
 *         0x0285: Start                                  421 data bytes
 *         0x0286: Stop                                   421 data bytes
 *         0x0287: Parameter                              421 data bytes
 *         0x0293: Position                                90 data bytes
 *         0x029A: Sound velocity profile                 416 data bytes
 *         0x0294: EM-12D starboard bathymetry            923 data bytes
 *         0x0295: EM-12D port bathymetry                 923 data bytes
 *         0x0296: EM-12S bathymetry                      923 data bytes
 *         0x0288: EM-121 bathymetry                      1375 data bytes
 *         0x0297: EM-1000 bathymetry                     692 data bytes
 *         0x02C8: EM-12D port sidescan                   551 data bytes
 *         0x02C9: EM-12D starboard sidescan              551 data bytes
 *         0x02CA: EM-12S or EM-1000 sidescan             551 data bytes
 *         0x02CB: EM-12D port sidescan + phase          1465 data bytes
 *         0x02CC: EM-12D starboard sidescan + phase     1465 data bytes
 *         0x02CD: EM-12S or EM-1000 sidescan + phase    1465 data bytes
 *   6. The EM-12D system records separate starboard and port datagrams
 *      for each ping.
 *   7. Multiple sidescan datagrams are recorded for each ping because
 *      there is too much information to fit in a single datagram.
 *   8. Simrad systems record navigation fixes using the position
 *      datagram; no navigation is included in the per ping data.  Thus,
 *      it is necessary to extrapolate the navigation for each ping
 *      at read time from the last navigation fix.  The frequency of
 *      GPS fixes generally assures that this is not a problem, but
 *      we offer no guarantees that this will always be the case.
 *
 */

#ifndef MBSYS_SIMRAD_H_
#define MBSYS_SIMRAD_H_

/* sonar types */
#define MBSYS_SIMRAD_UNKNOWN 0
#define MBSYS_SIMRAD_EM12S 1
#define MBSYS_SIMRAD_EM12D 2
#define MBSYS_SIMRAD_EM100 3
#define MBSYS_SIMRAD_EM1000 4
#define MBSYS_SIMRAD_EM121 5

/* maximum number of beams and pixels */
#define MBSYS_SIMRAD_MAXBEAMS 121
#define MBSYS_EM1000_MAXBEAMS 60
#define MBSYS_EM12_MAXBEAMS 81
#define MBSYS_EM121_MAXBEAMS 121
#define MBSYS_SIMRAD_MAXPIXELS 1024
#define MBSYS_SIMRAD_MAXRAWPIXELS 32000
#define MBSYS_SIMRAD_MAXSVP 100
#define MBSYS_SIMRAD_COMMENT_LENGTH 80

/* datagram types */
#define EM_NONE 0
#define EM_START 0x0285
#define EM_STOP 0x0286
#define EM_PARAMETER 0x0287
#define EM_POS 0x0293
#define EM_SVP 0x029A
#define EM_12DS_BATH 0x0294
#define EM_12DP_BATH 0x0295
#define EM_12S_BATH 0x0296
#define EM_121_BATH 0x0288
#define EM_1000_BATH 0x0297
#define EM_12DP_SS 0x02C8
#define EM_12DS_SS 0x02C9
#define EM_12S_SS 0x02CA
#define EM_12DP_SSP 0x02CB
#define EM_12DS_SSP 0x02CC
#define EM_12S_SSP 0x02CD

/* datagram sizes */
#define EM_START_SIZE 424
#define EM_STOP_SIZE 424
#define EM_PARAMETER_SIZE 424
#define EM_POS_SIZE 93
#define EM_SVP_SIZE 419
#define EM_12DS_BATH_SIZE 926
#define EM_12DP_BATH_SIZE 926
#define EM_12S_BATH_SIZE 926
#define EM_121_BATH_SIZE 1378
#define EM_1000_BATH_SIZE 695
#define EM_SS_SIZE 554
#define EM_SSP_SIZE 1468
#define EM_12DP_SS_SIZE 554
#define EM_12DS_SS_SIZE 554
#define EM_12S_SS_SIZE 554
#define EM_12DP_SSP_SIZE 1468
#define EM_12DS_SSP_SIZE 1468
#define EM_12S_SSP_SIZE 1468

/* swath id */
#define EM_SWATH_CENTER 0
#define EM_SWATH_PORT -1
#define EM_SWATH_STARBOARD 1

/* sidescan status */
#define EM_SS_NONE 0
#define EM_SS_AMPONLY 1
#define EM_SS_AMPPHASE 2

/* internal data structure for survey data */
struct mbsys_simrad_survey_struct {
	/* swath id */
	int swath_id; /* EM_SWATH_CENTER:	0
	             EM_SWATH_PORT:	-1  (EM12D only)
	             EM_SWATH_STARBOARD:	1   (EM12D only) */
	/* interpolated nav */
	double longitude;
	double latitude;

	/* bathymetry */
	int ping_number;
	int beams_bath; /* EM-1000:  60
	               EM12S:    81
	               EM121:   121
	               EM12D:   81 */
	int bath_mode;  /* EM-1000: 1=deep; 2=medium; 3=shallow
	               EM-12S:  1=shallow equiangle spacing
	                        2=deep equiangle spacing
	                        3=shallow equidistant spacing
	                        4=deep 120 degree equidistant
	                        5=deep 105 degree equidistant
	                        6=deep 90 degree equidistant
	               EM-12D:  1=shallow equiangle spacing
	                        2=deep equiangle spacing
	                        3=shallow equidistant spacing
	                        4=deep 150 degree equidistant
	                        5=deep 140 degree equidistant
	                        6=deep 128 degree equidistant
	                        7=deep 114 degree equidistant
	                        8=deep 98 degree equidistant */
	int bath_res;   /* EM-12 only: 1=high res; 2=low res */
	int bath_quality; /* number of good beams,
	              negative if ping rejected */
	int bath_num;     /* number of beams, EM-121 only, 61 or 121 */
	int pulse_length; /* pulse length in ms, EM-121 only */
	int beam_width;   /* beam width in degree, 1, 2 or 4, EM-121 only */
	int power_level;  /* power level, 0-5, EM-121 only */
	int tx_status;    /* 0-58, EM-121 only */
	int rx_status;    /* 0-144, EM-121 only */
	int along_res;    /* alongtrack resolution, 0.01 m, EM-121 only */
	int across_res;   /* acrosstrack resolution, 0.01 m, EM-121 only */
	int depth_res;    /* depth resolution, 0.01 m, EM-121 only */
	int range_res;    /* range resolution, 0.1 ms, EM-121 only */
	int keel_depth;   /* depth of most vertical beam:
	                  EM-1000:        0.02 meters
	                  EM-12 high res: 0.10 meters
	                  EM-12 low res:  0.20 meters
	                  EM-121          depth_res meters */
	int heading;      /* heading:
	                  EM-1000:        0.1 degrees
	                  EM-12:          0.1 degrees
	                  EM-121:		0.01 degrees */
	int roll;         /* 0.01 degrees */
	int pitch;        /* 0.01 degrees */
	int xducer_pitch; /* 0.01 degrees */
	int ping_heave;   /* 0.01 meters */
	int sound_vel;    /* 0.1 meters/sec */
	unsigned short int bath[MBSYS_SIMRAD_MAXBEAMS];
	/* depths:
	    EM-1000:        0.02 meters
	    EM-12 high res: 0.10 meters
	    EM-12 low res:  0.20 meters
	    EM-121:         depth_res meters */
	short int bath_acrosstrack[MBSYS_SIMRAD_MAXBEAMS];
	/* acrosstrack distances:
	    EM-1000:         0.1 meters
	    EM-12 high res:  0.2 meters
	    EM-12 low res:   0.5 meters
	    EM-121:          across_res meters */
	short int bath_alongtrack[MBSYS_SIMRAD_MAXBEAMS];
	/* alongtrack distances:
	    EM-1000:         0.1 meters
	    EM-12 high res:  0.2 meters
	    EM-12 low res:   0.5 meters
	    EM-121:          along_res meters */
	short int tt[MBSYS_SIMRAD_MAXBEAMS];       /* meters */
	                                           /* travel times:
	                                               EM-1000:         0.05 msec
	                                               EM-12 high res:  0.20 msec
	                                               EM-12 low res:   0.80 msec
	                                               EM-121:          0.1 * range_res msec */
	mb_s_char amp[MBSYS_SIMRAD_MAXBEAMS];      /* 0.5 dB */
	mb_u_char quality[MBSYS_SIMRAD_MAXBEAMS];  /* meters */
	mb_s_char heave[MBSYS_SIMRAD_MAXBEAMS];    /* 0.1 meters */
	mb_s_char beamflag[MBSYS_SIMRAD_MAXBEAMS]; /* MB-System beamflags */

	/* sidescan */
	int ss_status;
	int pixels_ssraw; /* total number of samples for this ping */
	int ss_mode;      /* 1 = EM-12 shallow:   0.6 m/sample
	                 2 = EM-12 deep:      2.4 m/sample
	                 3 = EM-1000 deep:    0.3 m/sample
	                 4 = EM-1000 medium:  0.3 m/sample
	                 5 = EM-1000 shallow: 0.15 m/sample */
	short int beam_frequency[MBSYS_SIMRAD_MAXBEAMS];
	/*	0 = 12.67 kHz
	    1 = 13.00 kHz
	    2 = 13.33 kHz
	    3 = 95.00 kHz */
	short int beam_samples[MBSYS_SIMRAD_MAXBEAMS];
	/* number of sidescan samples derived from
	    each beam */
	short int beam_center_sample[MBSYS_SIMRAD_MAXBEAMS];
	/* center beam sample number among samples
	    from one beam */
	short int beam_start_sample[MBSYS_SIMRAD_MAXBEAMS];
	/* start beam sample number among samples
	    from entire ping */
	mb_s_char ssraw[MBSYS_SIMRAD_MAXRAWPIXELS];
	short int ssp[MBSYS_SIMRAD_MAXRAWPIXELS];

	int pixel_size; /* processed sidescan pixel size in cm */
	int pixels_ss;  /* number of processed sidescan pixels stored */
	short ss[MBSYS_SIMRAD_MAXPIXELS];
	/* the processed sidescan ordered port to starboard */
	short ssalongtrack[MBSYS_SIMRAD_MAXPIXELS];
	/* the processed sidescan alongtrack distances
	    in distance resolution units */
};

/* internal data structure */
struct mbsys_simrad_struct {
	/* type of data record */
	int kind; /* Data vs Comment */

	/* type of sonar */
	int sonar; /* Type of Simrad sonar */

	/* parameter info (start, stop and parameter datagrams) */
	int par_year;
	int par_month;
	int par_day;
	int par_hour;
	int par_minute;
	int par_second;
	int par_centisecond;
	int pos_type;          /* positioning system type */
	double pos_delay;      /* positioning system delay (sec) */
	double roll_offset;    /* roll offset (degrees) */
	double pitch_offset;   /* pitch offset (degrees) */
	double heading_offset; /* heading offset (degrees) */
	double em100_td;       /* EM-100 transducer depth (meters) */
	double em100_tx;       /* EM-100 transducer fore-aft
	                   offset (meters) */
	double em100_ty;       /* EM-100 transducer athwartships
	                   offset (meters) */
	double em12_td;        /* EM-12 transducer depth (meters) */
	double em12_tx;        /* EM-12 transducer fore-aft
	                   offset (meters) */
	double em12_ty;        /* EM-12 transducer athwartships
	                   offset (meters) */
	double em1000_td;      /* EM-1000 transducer depth (meters) */
	double em1000_tx;      /* EM-1000 transducer fore-aft
	                   offset (meters) */
	double em1000_ty;      /* EM-1000 transducer athwartships
	                   offset (meters) */
	char spare_parameter[128];
	int survey_line;
	char comment[MBSYS_SIMRAD_COMMENT_LENGTH];

	/* position (position datagrams) */
	int pos_year;
	int pos_month;
	int pos_day;
	int pos_hour;
	int pos_minute;
	int pos_second;
	int pos_centisecond;
	double pos_latitude;
	double pos_longitude;
	double utm_northing;
	double utm_easting;
	int utm_zone;
	double utm_zone_lon;
	int utm_system;
	int pos_quality;
	double speed;        /* meters/second */
	double line_heading; /* degrees */

	/* sound velocity profile */
	int svp_year;
	int svp_month;
	int svp_day;
	int svp_hour;
	int svp_minute;
	int svp_second;
	int svp_centisecond;
	int svp_num;
	int svp_depth[MBSYS_SIMRAD_MAXSVP]; /* meters */
	int svp_vel[MBSYS_SIMRAD_MAXSVP];   /* 0.1 meters/sec */

	/* time stamp */
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int centisecond;

	/* pointer to survey data structure */
	struct mbsys_simrad_survey_struct *ping;
};

/* system specific function prototypes */
int mbsys_simrad_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_simrad_survey_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbsys_simrad_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_simrad_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_simrad_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                         double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                         double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                         double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_simrad_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                        double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                        double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                        double *ssalongtrack, char *comment, int *error);
                        int mbsys_simrad_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                        double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                        double *ssv, int *error);
int mbsys_simrad_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error);
int mbsys_simrad_pulses(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *pulses, int *error);
int mbsys_simrad_gains(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain, double *pulse_length,
                       double *receive_gain, int *error);
int mbsys_simrad_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                  double *altitude, int *error);
int mbsys_simrad_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                             double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                             double *pitch, double *heave, int *error);
int mbsys_simrad_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                            double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                            int *error);
int mbsys_simrad_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                             int *error);
int mbsys_simrad_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity, int *error);
int mbsys_simrad_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);
int mbsys_simrad_makess(int verbose, void *mbio_ptr, void *store_ptr, int pixel_size_set, double *pixel_size, int swath_width_set,
                        double *swath_width, int pixel_int, int *error);

// function called from C++ program mbcopy
#ifdef __cplusplus
extern "C" {
#endif
int mbsys_simrad_beamangles(int verbose, void *store_ptr, bool *interleave,
                        int *nbeams, double **angles_simrad, int *error);
#ifdef __cplusplus
}
#endif

#endif  /* MBSYS_SIMRAD_H_ */
