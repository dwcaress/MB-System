/*--------------------------------------------------------------------
 *    The MB-system:	MBF_SB2100RW.h	3/3/94
 *
 *    Copyright (c) 1994-2025 by
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
 * mbf_sb2100rw.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_SB2100RW format (MBIO id 41).
 *
 * Author:	D. W. Caress
 * Date:	March 3, 1994
 *
 */
/*
 * Notes on the MBF_SB2100RW data format:
 *   1. SeaBeam 1000/2100 multibeam systems output raw data in an
 *      ascii format.  The data consists of a number of different
 *      multi-line ascii records.
 *   2. The 2100/2100 systems output up to 151 beams of bathymetry
 *      and 2000 pixels of sidescan measurements, along with a plethora
 *      of other information.
 *   3. The records all include navigation and time stamp information.
 *      The record types are:
 *        PR:  sonar parameter record (roll bias, pitch bias, sound velocity profile)
 *        TR:  sonar text record (comments)
 *        SB:  sub-bottom data record (undefined as yet)
 *        DR:  bathymetry data record (bathymetry and per-beam amplitudes)
 *        SS:  side scan data record
 *   4. A single ping usually results in both DR and SS records.  The PR record
 *      occurs every 30 minutes or when the sound velocity profile is changed.
 *   5. The kind value in the mbsys_sb2k_struct indicates whether the
 *      mbsys_sb2k_data_struct structure holds data from a ping or
 *      data from some other record:
 *        kind = 1 : data from a ping
 *                   (DR + SS)
 *        kind = 2 : comment (TR)
 *        kind = 8 : sonar parameter (PR)
 *   6. The data structure defined below includes all of the values
 *      which are passed in SeaBeam 1000/2100 records.
 */

#ifndef MBF_SB2100RW_H_
#define MBF_SB2100RW_H_

/* maximum number of depth-velocity pairs */
#define MBF_SB2100RW_MAXVEL 30

/* maximum line length in characters */
#define MBF_SB2100RW_MAXLINE 1944

/* maximum number of formed beams for SeaBeam 1000/2100 */
#define MBF_SB2100RW_BEAMS 151

/* maximum number of sidescan pixels for SeaBeam 1000/2100 */
#define MBF_SB2100RW_PIXELS 2000

/* maximum value of beam amplitudes for SeaBeam 1000/2100 */
#define MBF_SB2100RW_AMP_MAX 999

/* maximum value of sidescan pixels for SeaBeam 1000/2100 */
#define MBF_SB2100RW_SS_MAX 65535

/* center beam for SeaBeam 1000/2100 */
#define MBF_SB2100RW_CENTER_BEAM 75

/* center pixel for SeaBeam 1000/2100 */
#define MBF_SB2100RW_CENTER_PIXEL 1000

/* define id's for the different types of raw Hydrosweep records */
#define MBF_SB2100RW_RECORDS 6
#define MBF_SB2100RW_NONE 0
#define MBF_SB2100RW_RAW_LINE 1
#define MBF_SB2100RW_PR 2
#define MBF_SB2100RW_TR 3
#define MBF_SB2100RW_DR 4
#define MBF_SB2100RW_SS 5
char *mbf_sb2100rw_labels[] = {"NONE    ", "RAW_LINE", "SB2100PR", "SB2100TR", "SB2100DR", "SB2100SS"};

struct mbf_sb2100rw_struct {
	/* type of data record */
	int kind;

	/* time stamp (all records ) */
	int year;
	int jday;
	int hour;
	int minute;
	int msec; /* msec */

	/* sonar parameters (PR) */
	int roll_bias_port;      /* 0.01 deg */
	int roll_bias_starboard; /* 0.01 deg */
	int pitch_bias;          /* 0.01 deg */
	int ship_draft;          /* 0.01 m */
	int num_svp;
	int vdepth[MBF_SB2100RW_MAXVEL];   /* 0.01 m */
	int velocity[MBF_SB2100RW_MAXVEL]; /* 0.01 m/sec */

	/* DR and SS header info */
	double longitude;
	double latitude;
	int speed;                  /* 0.001 m/sec */
	int heave;                  /* 0.001 m; + up */
	char range_scale;           /* D = m; I = 0.1 m; S = 0.01 m */
	int surface_sound_velocity; /* 0.01 m/sec */
	char ssv_source;            /* V=Velocimeter, M=Manual,
	                        T=Temperature */
	char depth_gate_mode;       /* A=Auto, M=Manual */

	/* DR header info */
	int num_beams;       /* number of formed beams recorded */
	char svp_corr_beams; /* 0=None; A=True Xtrack
	             and Apparent Depth;
	             T=True Xtrack and True Depth */
	char spare_dr[2];
	int num_algorithms; /* If 1 then only "best" algorithm
	                recorded, else multiple
	                algorithm results recorded */
	char algorithm_order[4]; /* blank if num_algorithms=1;
	                 W=WMT and B=BDI */

	/* SS header info */
	int num_pixels;       /* number of sidescan pixels recorded */
	char svp_corr_ss;     /* 0=off; 1=on */
	int ss_data_length;   /* number of bytes of sidescan data */
	char pixel_algorithm; /* pixel intensity algorithm
	              D = logarithm, L = linear */
	int num_pixels_12khz;
	double pixel_size_12khz; /* meters */
	int num_pixels_36khz;
	double pixel_size_36khz; /* meters */
	char spare_ss;
	char pixel_size_scale;

	/* transmit parameters and navigation (DR and SS) */
	char frequency[2];   /* LL=12kHz; HH=36kHz; number=36kHz
	             until this angle
	             in degrees then 12kHz */
	int ping_gain_12khz; /* dB */
	int ping_pulse_width_12khz;        /* msec */
	int transmitter_attenuation_12khz; /* dB */
	int pitch_12khz;                   /* 0.001 deg */
	int roll_12khz;                    /* 0.001 deg */
	int heading_12khz;                 /* 0.001 deg */
	int ping_gain_36khz;               /* dB */
	int ping_pulse_width_36khz;        /* msec */
	int transmitter_attenuation_36khz; /* dB */
	int pitch_36khz;                   /* 0.001 deg */
	int roll_36khz;                    /* 0.001 deg */
	int heading_36khz;                 /* 0.001 deg */

	/* formed beam data (DR) */
	char source[MBF_SB2100RW_BEAMS];          /* B=BDI, W=WMT */
	int travel_time[MBF_SB2100RW_BEAMS];      /*  msec */
	int angle_across[MBF_SB2100RW_BEAMS];     /* 0.001 deg */
	int angle_forward[MBF_SB2100RW_BEAMS];    /* 0.01 deg */
	int depth[MBF_SB2100RW_BEAMS];            /* m or cm */
	int acrosstrack_beam[MBF_SB2100RW_BEAMS]; /* m or cm */
	int alongtrack_beam[MBF_SB2100RW_BEAMS];  /* m or cm */
	int amplitude_beam[MBF_SB2100RW_BEAMS];   /* 0.25 dB */
	int signal_to_noise[MBF_SB2100RW_BEAMS];  /* dB */
	int echo_length[MBF_SB2100RW_BEAMS];      /* samples */
	char quality[MBF_SB2100RW_BEAMS];         /* 0=no data,
	                      Q=poor quality,
	                      blank otherwise */

	/* sidescan data (SS) */
	int amplitude_ss[MBF_SB2100RW_PIXELS];  /* range 0-65535 */
	int alongtrack_ss[MBF_SB2100RW_PIXELS]; /* m or cm */

	/* comment (TR) */
	char comment[MBF_SB2100RW_MAXLINE];
};

#endif  /* MBF_SB2100RW_H_ */
