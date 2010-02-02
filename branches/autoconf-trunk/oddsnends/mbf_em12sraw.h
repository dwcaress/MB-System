/*--------------------------------------------------------------------
 *    The MB-system:	mbf_em12sraw.h	7/8/96
 *	$Id: mbf_em12sraw.h,v 5.2 2001-03-22 20:50:02 caress Exp $
 *
 *    Copyright (c) 1994, 2000 by
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
 * mbf_em12sraw.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_EM12SRAW format (MBIO id 51).  
 *
 * Author:	D. W. Caress
 * Date:	August 8, 1994
 * $Log: not supported by cvs2svn $
 * Revision 5.1  2000/12/10  20:26:50  caress
 * Version 5.0.alpha02
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.7  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.6  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.5  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.5  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.4  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.3  1996/08/26  19:03:38  caress
 * REALLY changed "signed char" to "char".
 *
 * Revision 4.3  1996/08/26  19:03:38  caress
 * REALLY changed "signed char" to "char".
 *
 * Revision 4.2  1996/08/26  18:33:50  caress
 * Changed "signed char" to "char" for SunOs 4.1 compiler compatibility.
 *
 * Revision 4.1  1996/08/05  15:25:43  caress
 * Just redid i/o for Simrad sonars, including adding EM12S and EM121 support.
 *
 * Revision 4.0  1996/07/26  21:07:59  caress
 * Initial version.
 *
 *
 */
/*
 * Notes on the MBF_EM12SRAW data format:
 *   1. Simrad multibeam systems output datagrams which are
 *      a combination of ascii and binary.
 *   2. Simrad EM-12S systems output both bathymetry
 *      and amplitude information for beams and sidescan information
 *      with a higher resolution than the bathymetry and amplitude.
 *   3. The system of interest:
 *         EM-12S:   Deep water system with up to 81 beams of
 *                   bathymetry and up to 523 sidescan samples per
 *                   bathymetry beam.
 *         EM-12D:   Deep water system with up to 81 beams of
 *                   bathymetry and up to 523 sidescan samples per
 *                   bathymetry beam in each of two swaths. The
 *                   port side and starboard side pings are reported
 *                   separately.
 *   4. Each datagram is preceded by a two byte start code and
 *      followed by a three byte end code consisting of 0x03
 *      followed by two bytes representing the checksum for
 *      the data bytes.  MB-System does not calculate checksums
 *      and puts 0's in the checksum bytes.
 *   5. The relevent datagram start codes, types, and sizes are:
 *         0x0285: Start                                  421 data bytes
 *         0x0286: Stop                                   421 data bytes
 *         0x0287: Parameter                              421 data bytes
 *         0x029A: Position                                90 data bytes
 *         0x029A: Sound velocity profile                 416 data bytes
 *         0x0296: EM-12S bathymetry                      923 data bytes
 *         0x02CD: EM-12S or EM-1000 sidescan + phase    1465 data bytes
 *         0x0294: EM-12D port bathymetry                 923 data bytes
 *         0x02CB: EM-12D port sidescan + phase          1465 data bytes
 *         0x0295: EM-12D starboard bathymetry            923 data bytes
 *         0x02CC: EM-12D starboard sidescan + phase     1465 data bytes
 *   6. Multiple sidescan datagrams are recorded for each ping because 
 *      there is too much information to fit in a single datagram.
 *   7. Simrad systems record navigation fixes using the position 
 *      datagram; no navigation is included in the per ping data.  Thus,
 *      it is necessary to extrapolate the navigation for each ping
 *      at read time from the last navigation fix.  The frequency of
 *      GPS fixes generally assures that this is not a problem, but
 *      we offer no guarentees that this will always be the case.
 *
 */

/* maximum number of beams and pixels */
#define	MBF_EM12SRAW_MAXBEAMS	81
#define	MBF_EM12SRAW_MAXRAWPIXELS	50*MBF_EM12SRAW_MAXBEAMS
#define	MBF_EM12SRAW_MAXPIXELS	1024

struct mbf_em12sraw_struct
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
	char	comment[80];

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

	/* swath id */
	int	swath_id;	/* EM_SWATH_CENTER:	0
				   EM_SWATH_PORT:	-1  (EM12D only)
				   EM_SWATH_STARBOARD:	1   (EM12D only) */

	/* bathymetry */
	int	ping_number;
	int	beams_bath;	/* EM-1000:  60
				   EM12S:    81
				   EM12D:    81 */
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
	int	keel_depth;	/* depth of most vertical beam:
					EM-1000:        0.02 meters 
					EM-12 high res: 0.10 meters 
					EM-12 low res:  0.20 meters */
	int	heading;	/* 0.1 degrees */
	int	roll;		/* 0.01 degrees */
	int	pitch;		/* 0.01 degrees */
	int	xducer_pitch;	/* 0.01 degrees */
	int	ping_heave;	/* 0.01 meters */
	int	sound_vel;	/* 0.1 meters/sec */
	short int bath[MBF_EM12SRAW_MAXBEAMS];	
				/* depths:
					EM-1000:        0.02 meters 
					EM-12 high res: 0.10 meters 
					EM-12 low res:  0.20 meters */
	short int bath_acrosstrack[MBF_EM12SRAW_MAXBEAMS];
				/* acrosstrack distances:
					EM-1000:         0.1 meters 
					EM-12 high res:  0.2 meters 
					EM-12 low res:   0.5 meters */
	short int bath_alongtrack[MBF_EM12SRAW_MAXBEAMS];
				/* alongtrack distances:
					EM-1000:         0.1 meters 
					EM-12 high res:  0.2 meters 
					EM-12 low res:   0.5 meters */
	short int tt[MBF_EM12SRAW_MAXBEAMS];	/* meters */
				/* travel times:
					EM-1000:         0.05 msec 
					EM-12 high res:  0.20 msec 
					EM-12 low res:   0.80 msec */
	mb_s_char	amp[MBF_EM12SRAW_MAXBEAMS];	    /* 0.5 dB */
	mb_u_char	quality[MBF_EM12SRAW_MAXBEAMS];	    /* meters */
	mb_s_char	heave[MBF_EM12SRAW_MAXBEAMS];	    /* 0.1 meters */
	
	/* sidescan */
	int	pixels_ssraw;	/* total number of samples for this ping */
	int	ss_mode;	/* 1 = EM-12 shallow:   0.6 m/sample
				   2 = EM-12 deep:      2.4 m/sample
				   3 = EM-1000 deep:    0.3 m/sample
				   4 = EM-1000 medium:  0.3 m/sample
				   5 = EM-1000 shallow: 0.15 m/sample */
	short int beam_frequency[MBF_EM12SRAW_MAXBEAMS]; 
				/*	0 = 12.67 kHz
					1 = 13.00 kHz
					2 = 13.33 kHz
					3 = 95.00 kHz */
	short int beam_samples[MBF_EM12SRAW_MAXBEAMS];	
				/* number of sidescan samples derived from
					each beam */
	short int beam_center_sample[MBF_EM12SRAW_MAXBEAMS];
				/* center beam sample number among samples
					from one beam */	
	short int beam_start_sample[MBF_EM12SRAW_MAXBEAMS];
				/* start beam sample number among samples
					from entire ping */
	mb_s_char ssraw[MBF_EM12SRAW_MAXRAWPIXELS];
	short int ssp[MBF_EM12SRAW_MAXRAWPIXELS];
	int	pixel_size;	/* processed sidescan pixel size in cm */
	int	pixels_ss;	/* number of processed sidescan pixels stored */
	short	ss[MBF_EM12SRAW_MAXPIXELS];
				/* the processed sidescan ordered port to starboard */
	short	ssalongtrack[MBF_EM12SRAW_MAXPIXELS];
				/* the processed sidescan alongtrack distances 
					in distance resolution units */

};

