/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_simrad.h	8/5/94
 *	$Id: mbsys_simrad.h,v 4.1 1996-07-26 21:06:00 caress Exp $
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
 * mbsys_simrad.h defines the data structures used by MBIO functions
 * to store data from Simrad EM-1000 and EM-12 multibeam sonar systems.
 * The data formats which are commonly used to store Simrad 
 * data in files include
 *      MBF_EM1000RW : MBIO ID 51
 *      MBF_EM12SRAW : MBIO ID 52
 *      MBF_EM12DRAW : MBIO ID 53 - not supported yet
 *      MBF_EM12DARW : MBIO ID 54
 *      MBF_EM121RAW : MBIO ID 55
 *      MBF_EM3000RW : MBIO ID 56 - not supported yet
 *
 *
 * Author:	D. W. Caress (L-DEO)
 * Date:	August 5, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1994/10/21  12:35:09  caress
 * Release V4.0
 *
 * Revision 1.1  1994/10/21  12:20:01  caress
 * Initial revision
 *
 *
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
 *                   162 beams of bathymetry (port and starboard
 *                   calculated and recorded separately) and up
 *                   to 523 sidescan samples per bathymetry beam.
 *                   *** At present, the EM-12D is not supported! **
 *         EM-121:   Single array deep water system with up to 121
 *                   beams of bathymetry and up to 523 sidescan
 *                   samples per bathymetry beam.
 *         EM-3000:  Single array shallow water system with up to 81
 *                   beams of bathymetry and up to 523 sidescan
 *                   samples per bathymetry beam.
 *   4. Each telegram is preceded by a two byte start code and
 *      followed by a three byte end code consisting of 0x03
 *      followed by two bytes representing the checksum for
 *      the data bytes.  MB-System does not calculate checksums
 *      and puts 0's in the checksum bytes.
 *   5. The relevent telegram start codes, types, and sizes are:
 *         0x0285: Start                                  421 data bytes
 *         0x0286: Stop                                   421 data bytes
 *         0x0287: Parameter                              421 data bytes
 *         0x029A: Position                                90 data bytes
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
 *      we offer no guarentees that this will always be the case.
 *
 */

/* sonar types */
#define	MBSYS_SIMRAD_UNKNOWN	0
#define	MBSYS_SIMRAD_EM12S	1
#define	MBSYS_SIMRAD_EM12D	2
#define	MBSYS_SIMRAD_EM100	3
#define	MBSYS_SIMRAD_EM1000	4
#define	MBSYS_SIMRAD_EM121	5
#define	MBSYS_SIMRAD_EM3000	6

/* maximum number of beams and pixels */
#define	MBSYS_SIMRAD_MAXBEAMS	162
#define	MBSYS_SIMRAD_MAXPIXELS	100*MBSYS_SIMRAD_MAXBEAMS
#define	MBSYS_SIMRAD_COMMENT_LENGTH	80

/* datagram types */
#define	EM_NONE			0
#define	EM_START		0x0285
#define	EM_STOP			0x0286
#define	EM_PARAMETER		0x0287
#define	EM_POS			0x0293
#define	EM_SVP			0x029A
#define	EM_12DS_BATH		0x0294
#define	EM_12DP_BATH		0x0295
#define	EM_12S_BATH		0x0296
#define	EM_121_BATH		0x0288
#define	EM_1000_BATH		0x0297
#define	EM_12DP_SS		0x02C8
#define	EM_12DS_SS		0x02C9
#define	EM_12S_SS		0x02CA
#define	EM_12DP_SSP		0x02CB
#define	EM_12DS_SSP		0x02CC
#define	EM_12S_SSP		0x02CD

/* datagram sizes */
#define	EM_START_SIZE		421
#define	EM_STOP_SIZE		421
#define	EM_PARAMETER_SIZE	421
#define	EM_POS_SIZE		90
#define	EM_SVP_SIZE		416
#define	EM_12DS_BATH_SIZE	923
#define	EM_12DP_BATH_SIZE	923
#define	EM_12S_BATH_SIZE	923
#define	EM_121_BATH_SIZE	1375
#define	EM_1000_BATH_SIZE	692
#define	EM_12DP_SS_SIZE		551
#define	EM_12DS_SS_SIZE		551
#define	EM_12S_SS_SIZE		551
#define	EM_12DP_SSP_SIZE	1465
#define	EM_12DS_SSP_SIZE	1465
#define	EM_12S_SSP_SIZE		1465

/* internal data structure */
struct mbsys_simrad_struct
	{
	/* type of data record */
	int	kind;			/* Data vs Comment */

	/* type of sonar */
	int	sonar;			/* Type of Simrad sonar */

	/* parameter info (start, stop and parameter datagrams) */
	int	par_year;
	int	par_month;
	int	par_day;
	int	par_hour;
	int	par_minute;
	int	par_second;
	int	par_centisecond;
	int	pos_type;	/* positioning system type */
	double	pos_delay;	/* positioning system delay (sec) */
	double	roll_offset;	/* roll offset (degrees) */
	double	pitch_offset;	/* pitch offset (degrees) */
	double	heading_offset;	/* heading offset (degrees) */
	double	em100_td;	/* EM-100 tranducer depth (meters) */
	double	em100_tx;	/* EM-100 tranducer fore-aft 
					offset (meters) */
	double	em100_ty;	/* EM-100 tranducer athwartships 
					offset (meters) */
	double	em12_td;	/* EM-12 tranducer depth (meters) */
	double	em12_tx;	/* EM-12 tranducer fore-aft 
					offset (meters) */
	double	em12_ty;	/* EM-12 tranducer athwartships 
					offset (meters) */
	double	em1000_td;	/* EM-1000 tranducer depth (meters) */
	double	em1000_tx;	/* EM-1000 tranducer fore-aft 
					offset (meters) */
	double	em1000_ty;	/* EM-1000 tranducer athwartships 
					offset (meters) */
	char	spare_parameter[128];
	int	survey_line;
	char	comment[MBSYS_SIMRAD_COMMENT_LENGTH];

	/* position (position datagrams) */
	int	pos_year;
	int	pos_month;
	int	pos_day;
	int	pos_hour;
	int	pos_minute;
	int	pos_second;
	int	pos_centisecond;
	double	latitude;
	double	longitude;
	double	utm_northing;
	double	utm_easting;
	int	utm_zone;
	double	utm_zone_lon;
	int	utm_system;
	int	pos_quality;
	double	speed;			/* meters/second */
	double	line_heading;		/* degrees */

	/* sound velocity profile */
	int	svp_year;
	int	svp_month;
	int	svp_day;
	int	svp_hour;
	int	svp_minute;
	int	svp_second;
	int	svp_centisecond;
	int	svp_num;
	int	svp_depth[100]; /* meters */
	int	svp_vel[100];	/* 0.1 meters/sec */

	/* time stamp */
	int	year;
	int	month;
	int	day;
	int	hour;
	int	minute;
	int	second;
	int	centisecond;
	int	ping_number;

	/* bathymetry */
	int	beams_bath;	/* EM-1000:  60
				   EM12S:    81
				   EM121:   121 
				   EM12D:   162 */
	int	bath_mode;	/* EM-1000: 1=deep; 2=medium; 3=shallow 
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
	int	bath_res;	/* EM-12 only: 1=high res; 2=low res */
	int	bath_quality;	/* number of good beams, 
					negative if ping rejected */
	int	bath_num;	/* number of beams, EM-121 only, 61 or 121 */
	int	pulse_length;	/* pulse length in ms, EM-121 only */
	int	beam_width;	/* beam width in degree, 1, 2 or 4, EM-121 only */
	int	power_level;	/* power level, 0-5, EM-121 only */
	int	tx_status;	/* 0-58, EM-121 only */
	int	rx_status;	/* 0-144, EM-121 only */
	int	along_res;	/* alongtrack resolution, 0.01 m, EM-121 only */
	int	across_res;	/* acrosstrack resolution, 0.01 m, EM-121 only */
	int	depth_res;	/* depth resolution, 0.01 m, EM-121 only */
	int	range_res;	/* range resolution, 0.1 ms, EM-121 only */
	int	keel_depth;	/* depth of most vertical beam:
					EM-1000:        0.02 meters 
					EM-12 high res: 0.10 meters 
					EM-12 low res:  0.20 meters
					EM-121          depth_res meters */
	int	heading;	/* heading:
					EM-1000:        0.1 degrees
					EM-12:          0.1 degrees
					EM-121:		0.01 degrees */
	int	roll;		/* 0.01 degrees */
	int	pitch;		/* 0.01 degrees */
	int	xducer_pitch;	/* 0.01 degrees */
	int	ping_heave;	/* 0.01 meters */
	int	sound_vel;	/* 0.1 meters/sec */
	short int bath[MBSYS_SIMRAD_MAXBEAMS];	
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
	short int tt[MBSYS_SIMRAD_MAXBEAMS];	/* meters */
				/* travel times:
					EM-1000:         0.05 msec 
					EM-12 high res:  0.20 msec 
					EM-12 low res:   0.80 msec
					EM-121:          range_res meters */
	signed char	amp[MBSYS_SIMRAD_MAXBEAMS];	/* 0.5 dB */
	unsigned char	quality[MBSYS_SIMRAD_MAXBEAMS];	/* meters */
	signed char	heave[MBSYS_SIMRAD_MAXBEAMS];	/* 0.1 meters */
	
	/* sidescan */
	int	pixels_ss;	/* total number of samples for this ping */
	int	ss_mode;	/* 1 = EM-12 shallow:   0.6 m/sample
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
	signed char ss[MBSYS_SIMRAD_MAXPIXELS];
	short int ssp[MBSYS_SIMRAD_MAXPIXELS];

	};
