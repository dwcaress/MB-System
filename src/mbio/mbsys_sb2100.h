/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_sb2100.h	2/4/94
 *	$Id: mbsys_sb2100.h,v 4.3 1994-10-21 12:20:01 caress Exp $
 *
 *    Copyright (c) 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbsys_sb2100.h defines the data structures used by MBIO functions
 * to store data from SeaBeam 2100 and 1000 multibeam sonar systems.
 * The data formats which are commonly used to store SeaBeam 1000/2100
 * data in files include
 *      MBF_SB2100RW : MBIO ID 41
 *      MBF_SB2100BN : MBIO ID 42
 *
 * Author:	D. W. Caress
 * Date:	February 4, 1994
 * $Log: not supported by cvs2svn $
 * Revision 4.2  1994/06/21  22:54:21  caress
 * Added #ifdef statements to handle byte swapping.
 *
 * Revision 4.1  1994/04/09  15:49:21  caress
 * Altered to fit latest iteration of SeaBeam 2100 vendor format.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/05  02:12:07  caress
 * First cut for SeaBeam 2100 i/o.
 *
 *
 */
/*
 * Notes on the MBSYS_SB2100 data structure:
 *   1. SeaBeam 1000/2100 multibeam systems output raw data in an
 *      ascii format.  The data consists of a number of different
 *      multi-line ascii records.
 *   2. The 2100/2100 systems output 151 beams of bathymetry and 2000 pixels
 *      of sidescan measurements, along with a plethora of other
 *      information.
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

/* maximum number of depth-velocity pairs */
#define MBSYS_SB2100_MAXVEL 30

/* maximum line length in characters */
#define MBSYS_SB2100_MAXLINE 1944

/* maximum number of formed beams for SeaBeam 1000/2100 */
#define MBSYS_SB2100_BEAMS 151

/* maximum number of sidescan pixels for SeaBeam 1000/2100 */
#define MBSYS_SB2100_PIXELS 2000

/* center beam for SeaBeam 1000/2100 */
#define MBSYS_SB2100_CENTER_BEAM 75

/* center pixel for SeaBeam 1000/2100 */
#define MBSYS_SB2100_CENTER_PIXEL 1000

struct mbsys_sb2100_struct
	{
	/* type of data record */
	int	kind;

	/* time stamp (all records ) */
	int	year;
	int	jday;
	int	hour;
	int	minute;
	long int	msec;			/* msec */

	/* sonar parameters (PR) */
	int	roll_bias_port;			/* 0.01 deg */
	int	roll_bias_starboard;		/* 0.01 deg */
	int	pitch_bias;			/* 0.01 deg */
	int	num_svp;
	int	vdepth[MBSYS_SB2100_MAXVEL];	/* 0.01 m */
	int	velocity[MBSYS_SB2100_MAXVEL];	/* 0.01 m/sec */

	/* DR and SS header info */
	double	longitude;
	double	latitude;
	int	speed;			/* 0.001 m/sec */
	int	surface_sound_velocity;	/* 0.01 m/sec */
	char	ssv_source;		/* V=Velocimeter, M=Manual, 
						T=Temperature */
	char	depth_gate_mode;	/* A=Auto, M=Manual */

	/* DR header info */
	int	num_beams;		/* number of formed beams recorded */
	char	svp_corr_beams;		/* 0=None; A=True Xtrack 
						and Apparent Depth;
						T=True Xtrack and True Depth */
	char	spare[8];
	char	range_scale; 		/* D = meters; S = cm */
	int	num_algorithms;		/* If 1 then only "best" algorithm 
						recorded, else multiple 
						algorithm results recorded */
	char	algorithm_order[4];	/* blank if num_algorithms=1; 
						W=WMT and B=BDI */

	/* SS header info */
	int	num_pixels;		/* number of sidescan pixels recorded */
	char	svp_corr_ss;		/* 0=off; 1=on */
	int	ss_data_length;		/* number of bytes of sidescan data */
	char	pixel_algorithm;	/* pixel intensity algorithm
						D = logarithm, L = linear */
	int	num_pixels_12khz;
	int	pixel_size_12khz;	/* meters */
	int	num_pixels_36khz;
	int	pixel_size_36khz;	/* meters */

	/* transmit parameters and navigation (DR and SS) */
	char	frequency[2];		/* LL=12kHz; HH=36kHz; number=36kHz
						until this angle 
						in degrees then 12kHz */
	int	ping_gain_12khz;			/* dB */
	int	ping_pulse_width_12khz;			/* msec */
	int	transmitter_attenuation_12khz;		/* dB */
	int	pitch_12khz;				/* 0.01 deg */
	int	roll_12khz;				/* 0.01 deg */
	int	heading_12khz;				/* 0.01 deg */
	int	ping_gain_36khz;				/* dB */
	int	ping_pulse_width_36khz;			/* msec */
	int	transmitter_attenuation_36khz;		/* dB */
	int	pitch_36khz;					/* 0.01 deg */
	int	roll_36khz;					/* 0.01 deg */
	int	heading_36khz;				/* 0.01 deg */

	/* formed beam data (DR) */
	char		source[MBSYS_SB2100_BEAMS];	/* B=BDI, W=WMT */
	int	travel_time[MBSYS_SB2100_BEAMS];		/*  msec */
	int	angle_across[MBSYS_SB2100_BEAMS];	/* 0.01 deg */
	int	angle_forward[MBSYS_SB2100_BEAMS];	/* 0.01 deg */
	int	depth[MBSYS_SB2100_BEAMS];		/* m or cm */
	int	acrosstrack_beam[MBSYS_SB2100_BEAMS];	/* m or cm */
	int	alongtrack_beam[MBSYS_SB2100_BEAMS];	/* m or cm */
	int	amplitude_beam[MBSYS_SB2100_BEAMS];	/* 0.25 dB */
	int	signal_to_noise[MBSYS_SB2100_BEAMS];	/* dB */
	int	echo_length[MBSYS_SB2100_BEAMS];	/* samples */
	char		quality[MBSYS_SB2100_BEAMS];	/* 0=no data, 
							Q=poor quality, 
							blank otherwise */

	/* sidescan data (SS) */
	int	amplitude_ss[MBSYS_SB2100_PIXELS];	/* range 0-65535 */
	int	alongtrack_ss[MBSYS_SB2100_PIXELS];	/* m or cm */

	/* comment (TR) */
	char	comment[MBSYS_SB2100_MAXLINE ];
};

