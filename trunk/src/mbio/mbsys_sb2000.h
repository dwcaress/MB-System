/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_sb2000.h	10/4/94
 *	$Id: mbsys_sb2000.h,v 4.3 1997-04-21 17:02:07 caress Exp $
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
 * mbsys_sb2000.h defines the data structures used by MBIO functions
 * to store data from the SeaBeam 2000 multibeam sonar systems.
 * The data formats which are commonly used to store SeaBeam
 * data in files include
 *      MBF_SB2000RW : MBIO ID 31
 *      MBF_SB2000SB : MBIO ID 32
 *
 * Author:	D. W. Caress
 * Date:	October 4, 1994
 * $Log: not supported by cvs2svn $
 * Revision 4.2  1995/02/14  22:01:39  caress
 * Version 4.2
 *
 * Revision 4.2  1995/02/14  22:01:39  caress
 * Version 4.2
 *
 * Revision 4.1  1994/12/21  20:21:09  caress
 * Changes to support high resolution SeaBeam 2000 sidescan files
 * from R/V Melville data.
 *
 * Revision 4.0  1994/10/21  12:35:09  caress
 * Release V4.0
 *
 * Revision 1.1  1994/10/21  12:20:01  caress
 * Initial revision
 *
 *
 */
/*
 * Notes on the MBSYS_SB2000 data structure:
 *   1. SeaBeam 2000 multibeam systems output raw data in 121 beams.
 *   5. The kind value in the mbsys_sb2000_struct indicates whether the
 *      mbsys_sb2000_data_struct structure holds data from a ping or
 *      data from a comment:
 *        kind = 1 : data from a ping 
 *        kind = 2 : comment 
 *   6. The data structure defined below includes all of the values
 *      which are passed in SeaBeam 2000 records.
 */

/* number of bathymetry beams for SeaBeam 2000 */
#define MBSYS_SB2000_BEAMS 121

/* number of sidescan pixels for SeaBeam 2000 */
#define MBSYS_SB2000_PIXELS 2000

/* maximum length of comments in data */
#define	MBSYS_SB2000_COMMENT_LENGTH	250

struct mbsys_sb2000_struct
	{
	/* type of data record */
	int	kind;

	/* time stamp */
	short	year;		/* year (4 digits) */
	short	day;		/* julian day (1-366) */
	short	min;		/* minutes from beginning of day (0-1439) */
	short	sec;		/* seconds from beginning of minute (0-59) */

	/* position */
	int	lat;		/* 1e-7 degrees from equator */
	int	lon;		/* 1e-7 degrees from prime meridian */

	/* other values */
	short	heading;	/* heading in 0.1 degrees */
	short	course;		/* course in 0.1 degrees */
	short	speed;		/* fore-aft speed in 0.1 knots */
	short	speed_ps;	/* port-starboard speed in 0.1 knots */
	short	quality;	/* quality value, 0 good, bigger bad */
	short	sensor_size;	/* size of sensor specific record in bytes */
	short	data_size;	/* size of data record in bytes */
	char	speed_ref[2];	/* speed reference */
	char	sensor_type[2];	/* sensor type */
	char	data_type[2];	/* type of data recorded */
	short	pitch;	/* 0.01 degrees */
	short	roll;	/* 0.01 degrees */
	short	gain;	/* ping gain, receiver gain */
	short	correction;	/* sonar correction */
	short	surface_vel;	/* sea surface sound velocity */
	short	pulse_width;	/* transmitter pulse width */
	short	attenuation;	/* transmitter attenuation */
	short	spare1;
	short	spare2;
	char	mode[2];	/* operation mode */
	char	data_correction[2];	/* data correction */
	char	ssv_source[2];	/* surface sound velocity source */
	
	/* sound velocity record */
	int	svp_mean;
	short	svp_number;
	short	svp_spare;
	short	svp_depth[30];
	short	svp_vel[30];
	short	vru1;
	short	vru1_port;
	short	vru1_forward;
	short	vru1_vert;
	short	vru2;
	short	vru2_port;
	short	vru2_forward;
	short	vru2_vert;
	short	pitch_bias;
	short	roll_bias;
	char	vru[8];

	/* bathymetry data */
	short	beams_bath;	/* number of bathymetry beams */
	short	scale_factor;	/* scale factor */
	short	bath[MBSYS_SB2000_BEAMS];
	short	bath_acrosstrack[MBSYS_SB2000_BEAMS];

	/* comment */
	char	comment[MBSYS_SB2000_COMMENT_LENGTH];

	/* sidescan data */
	int	ping_number;
	short	ping_length;
	short	pixel_size;	/* meters per pixel */
	short	ss_min;	/* dB gray level minimum */
	short	ss_max;	/* dB gray level maximum */
	short	sample_rate;	/* hydrophone sampling rate 0.1 usec */
	short	start_time;	/* first time slice */
        short	tot_slice;	/* total time slices */
	short	pixels_ss;	/* number of pixels */
	char	spare_ss[12];	/* spare */
	char	ss_type;	/* sidescan type: G=grayscale, R=raw sidescan */
	char	ss_dummy;
	char	ss[2*MBSYS_SB2000_PIXELS];
	
	};
