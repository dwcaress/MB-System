/*--------------------------------------------------------------------
 *    The MB-system:	mbf_hsldeoih.h	3/11/93
 *	$Id: mbf_hsldeoih.h,v 5.0 2000-12-01 22:48:41 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000 by
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
 * mbf_hsldeoih.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_HSLDEOIH format (MBIO id 24).  
 *
 * Author:	D. W. Caress
 * Date:	March 11, 1993
 * $Log: not supported by cvs2svn $
 * Revision 4.4  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.3  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.2  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.2  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.1  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.1  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.2  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.1  1994/02/17  21:19:08  caress
 * Updated associated MBIO format id in comments.
 *
 * Revision 4.0  1994/02/17  20:59:54  caress
 * First cut at new version. No changes.
 *
 * Revision 3.0  1993/05/14  22:51:50  sohara
 * initial version
 *
 */
/*
 * Notes on the MBF_HSLDEOIH data format:
 *   1. Hydrosweep DS multibeam systems output raw data in an
 *      ascii format.  The data consists of a number of different
 *      multi-line ascii records.
 *   2. The DS systems output 59 beams of bathymetry and 59 beams
 *      of backscatter measurements, along with a plethora of other
 *      information.
 *   3. The records all include navigation and time stamp information.
 *      The record types are:
 *        ERGNHYDI:  mean and keel water velocity values
 *        ERGNPARA:  navigation when system in standby
 *        ERGNPOSI:  navigation source
 *        ERGNMESS:  across-track "survey" bathymetry
 *        ERGNEICH:  along-track "calibration" bathymetry
 *        ERGNLSZT:  travel times associated with 
 *                   ERGNMESS or ERGNEICH records
 *        ERGNCTDS:  water sound velocity profile
 *        ERGNAMPL:  amplitudes associated with 
 *                   ERGNMESS or ERGNEICH records
 *        LDEOCOMM:  comment records (an L-DEO extension)
 *   4. A single ping usually results in the following series of
 *      of records:
 *        1. ERGNMESS or ERGNEICH
 *        2. ERGNSLZT
 *        3. ERGNAMPL
 *      The ERGNHYDI, ERGNPARA, ERGNPOSI and ERGNCTDS records occur
 *      at system startup and when the associated operational
 *      parameters of the Hydrosweep are changed.
 *   5. The kind value in the mbf_hsldeoih_struct indicates whether the
 *      mbf_hsldeoih_data_struct structure holds data from a ping or
 *      data from some other record:
 *        kind = 1 : data from a survey ping 
 *                   (ERGNMESS + ERGNSLZT + ERGNAMPL)
 *        kind = 2 : comment (LDEOCOMM)
 *        kind = 4 : data from a calibrate ping 
 *                   (ERGNEICH + ERGNSLZT + ERGNAMPL)
 *        kind = 5 : mean and keel velocity (ERGNHYDI)
 *        kind = 6 : water velocity profile (ERGNCTDS)
 *        kind = 7 : standby navigation (ERGNPARA)
 *        kind = 8 : navigation source (ERGNPOSI)
 *   6. The data structure defined below includes all of the values
 *      which are passed in Hydrosweep records.
 *   7. The data structure defined below also includes backscatter values
 *      obtained by processing the amplitude information.
 *   8. The first four bytes of every data record consist of the characters
 *      "data" which has a four byte integer equivalent value of 1684108385.
 *   9. Following the "data" flag is a two byte integer value containing
 *      the kind of data contained in the record.  The length of the
 *      record will depend on the kind.  The data structures associated
 *      with the different data records are defined below.
 *   10. This format is envisioned as the L-DEO in-house archive format 
 *      for processed Hydrosweep DS data.
 *   11. The kind values have changed. In older versions the definitions
 *      where:
 *        kind = 1 : data from a survey ping 
 *                   (ERGNMESS + ERGNSLZT + ERGNAMPL)
 *        kind = 2 : comment (LDEOCOMM)
 *        kind = 3 : data from a calibrate ping 
 *                   (ERGNEICH + ERGNSLZT + ERGNAMPL)
 *        kind = 4 : mean and keel velocity (ERGNHYDI)
 *        kind = 5 : water velocity profile (ERGNCTDS)
 *        kind = 6 : standby navigation (ERGNPARA)
 *        kind = 7 : navigation source (ERGNPOSI)
 *      The code checks for and fixes older data files on read, 
 *      using the record size values to check for bad kind values.
 */

/* maximum number of depth-velocity pairs */
#define MBF_HSLDEOIH_MAXVEL 30

/* maximum line length in characters */
#define MBF_HSLDEOIH_MAXLINE 200

/* number of beams for hydrosweep */
#define MBF_HSLDEOIH_BEAMS 59

/* record label value (integer equivalent to "data" */
#define MBF_HSLDEOIH_LABEL 1684108385

/* data record kind values */
#define MBF_HSLDEOIH_KIND_DATA			1
#define MBF_HSLDEOIH_KIND_COMMENT		2
#define MBF_HSLDEOIH_KIND_CALIBRATE		4
#define MBF_HSLDEOIH_KIND_MEAN_VELOCITY		5
#define MBF_HSLDEOIH_KIND_VELOCITY_PROFILE	6
#define MBF_HSLDEOIH_KIND_STANDBY		7
#define MBF_HSLDEOIH_KIND_NAV_SOURCE		8
#define MBF_HSLDEOIH_OLDKIND_CALIBRATE		3
#define MBF_HSLDEOIH_OLDKIND_MEAN_VELOCITY	4
#define MBF_HSLDEOIH_OLDKIND_VELOCITY_PROFILE	5
#define MBF_HSLDEOIH_OLDKIND_STANDBY		6
#define MBF_HSLDEOIH_OLDKIND_NAV_SOURCE		7

/* complete data structure containing everything */
struct mbf_hsldeoih_struct
	{
	/* type of data record */
	int	kind;

	/* position (all records ) */
	double	lon;
	double	lat;

	/* time stamp (all records ) */
	int	year;
	int	month;
	int	day;
	int	hour;
	int	minute;
	int	second;
	int	alt_minute;
	int	alt_second;

	/* additional navigation and depths (ERGNMESS and ERGNEICH) */
	double	course_true;
	double	speed_transverse;
	double	speed;
	char	speed_reference[2];
	double	pitch;
	int	track;
	double	depth_center;
	double	depth_scale;
	int	spare;
	int	distance[MBF_HSLDEOIH_BEAMS];
	int	depth[MBF_HSLDEOIH_BEAMS];

	/* travel time data (ERGNSLZT) */
	double	course_ground;
	double	speed_ground;
	double	heave;
	double	roll;
	double	time_center;
	double	time_scale;
	int	time[MBF_HSLDEOIH_BEAMS];
	double	gyro[11];

	/* amplitude data (ERGNAMPL) */
	char	mode[2];
	int	trans_strbd;
	int	trans_vert;
	int	trans_port;
	int	pulse_len_strbd;
	int	pulse_len_vert;
	int	pulse_len_port;
	int	gain_start;
	int	r_compensation_factor;
	int	compensation_start;
	int	increase_start;
	int	tvc_near;
	int	tvc_far;
	int	increase_int_near;
	int	increase_int_far;
	int	gain_center;
	double	filter_gain;
	int	amplitude_center;
	int	echo_duration_center;
	int	echo_scale_center;
	int	gain[16];
	int	amplitude[MBF_HSLDEOIH_BEAMS];
	int	echo_scale[16];
	int	echo_duration[MBF_HSLDEOIH_BEAMS];

	/* mean velocity (ERGNHYDI) */
	double	draught;
	double	vel_mean;
	double	vel_keel;
	double	tide;

	/* water velocity profile (HS_ERGNCTDS) */
	int	num_vel;
	double	vdepth[MBF_HSLDEOIH_MAXVEL];
	double	velocity[MBF_HSLDEOIH_MAXVEL];

	/* navigation source (ERGNPOSI) */
	double	pos_corr_x;
	double	pos_corr_y;
	char	sensors[10];

	/* comment (LDEOCMNT) */
	char	comment[MBF_HSLDEOIH_MAXLINE];

	/* processed backscatter data */
	double	back_scale;
	int	back[MBF_HSLDEOIH_BEAMS];
	};

/* data structure for navigation source records */
struct mbf_hsldeoih_nav_source_struct
	{
	/* position */
	float		lon;
	float		lat;

	/* time stamp */
	short int	year;
	short int	month;
	short int	day;
	short int	hour;
	short int	minute;
	short int	second;
	short int	alt_minute;
	short int	alt_second;

	/* navigation source */
	float	pos_corr_x;
	float	pos_corr_y;
	char	sensors[10];
	};

/* data structure for mean velocity records */
struct mbf_hsldeoih_mean_velocity_struct
	{
	/* position */
	float		lon;
	float		lat;

	/* time stamp */
	short int	year;
	short int	month;
	short int	day;
	short int	hour;
	short int	minute;
	short int	second;
	short int	alt_minute;
	short int	alt_second;

	/* mean velocity */
	float		draught;
	float		vel_mean;
	float		vel_keel;
	float		tide;
	};

/* data structure for velocity profile records */
struct mbf_hsldeoih_velocity_profile_struct
	{
	/* position */
	float		lon;
	float		lat;

	/* time stamp */
	short int	year;
	short int	month;
	short int	day;
	short int	hour;
	short int	minute;
	short int	second;

	/* water velocity profile */
	int	num_vel;
	float	vdepth[MBF_HSLDEOIH_MAXVEL];
	float	velocity[MBF_HSLDEOIH_MAXVEL];
	};

/* data structure for standby records */
struct mbf_hsldeoih_standby_struct
	{
	/* position */
	float		lon;
	float		lat;

	/* time stamp */
	short int	year;
	short int	month;
	short int	day;
	short int	hour;
	short int	minute;
	short int	second;
	short int	alt_minute;
	short int	alt_second;

	/* additional navigation */
	float		course_true;
	float		speed_transverse;
	float		speed;
	char		speed_reference[2];
	float		pitch;
	short int	track;
	float		depth_center;
	};

/* data structure for survey data records */
struct mbf_hsldeoih_survey_struct
	{
	/* position */
	float		lon;
	float		lat;

	/* time stamp */
	short int	year;
	short int	month;
	short int	day;
	short int	hour;
	short int	minute;
	short int	second;
	short int	alt_minute;
	short int	alt_second;

	/* additional navigation and depths */
	float		course_true;
	float		speed_transverse;
	float		speed;
	char		speed_reference[2];
	float		pitch;
	short int	track;
	float		depth_center;
	float		depth_scale;
	short int	spare;
	short int	distance[MBF_HSLDEOIH_BEAMS];
	short int	depth[MBF_HSLDEOIH_BEAMS];

	/* travel time data */
	float		course_ground;
	float		speed_ground;
	float		heave;
	float		roll;
	float		time_center;
	float		time_scale;
	short int	time[MBF_HSLDEOIH_BEAMS];
	float		gyro[11];

	/* amplitude data */
	char		mode[2];
	short int	trans_strbd;
	short int	trans_vert;
	short int	trans_port;
	short int	pulse_len_strbd;
	short int	pulse_len_vert;
	short int	pulse_len_port;
	short int	gain_start;
	short int	r_compensation_factor;
	short int	compensation_start;
	short int	increase_start;
	short int	tvc_near;
	short int	tvc_far;
	short int	increase_int_near;
	short int	increase_int_far;
	short int	gain_center;
	float		filter_gain;
	short int	amplitude_center;
	short int	echo_duration_center;
	short int	echo_scale_center;
	short int	gain[16];
	short int	amplitude[MBF_HSLDEOIH_BEAMS];
	short int	echo_scale[16];
	short int	echo_duration[MBF_HSLDEOIH_BEAMS];

	/* processed backscatter data */
	float		back_scale;
	short int	back[MBF_HSLDEOIH_BEAMS];
	};

/* data structure for calibrate data records */
struct mbf_hsldeoih_calibrate_struct
	{
	/* position */
	float		lon;
	float		lat;

	/* time stamp */
	short int	year;
	short int	month;
	short int	day;
	short int	hour;
	short int	minute;
	short int	second;
	short int	alt_minute;
	short int	alt_second;

	/* additional navigation and depths */
	float		course_true;
	float		speed_transverse;
	float		speed;
	char		speed_reference[2];
	float		pitch;
	short int	track;
	float		depth_center;
	float		depth_scale;
	short int	spare;
	short int	distance[MBF_HSLDEOIH_BEAMS];
	short int	depth[MBF_HSLDEOIH_BEAMS];

	/* travel time data */
	float		course_ground;
	float		speed_ground;
	float		heave;
	float		roll;
	float		time_center;
	float		time_scale;
	short int	time[MBF_HSLDEOIH_BEAMS];
	float		gyro[11];

	/* amplitude data */
	char		mode[2];
	short int	trans_strbd;
	short int	trans_vert;
	short int	trans_port;
	short int	pulse_len_strbd;
	short int	pulse_len_vert;
	short int	pulse_len_port;
	short int	gain_start;
	short int	r_compensation_factor;
	short int	compensation_start;
	short int	increase_start;
	short int	tvc_near;
	short int	tvc_far;
	short int	increase_int_near;
	short int	increase_int_far;
	short int	gain_center;
	float		filter_gain;
	short int	amplitude_center;
	short int	echo_duration_center;
	short int	echo_scale_center;
	short int	gain[16];
	short int	amplitude[MBF_HSLDEOIH_BEAMS];
	short int	echo_scale[16];
	short int	echo_duration[MBF_HSLDEOIH_BEAMS];

	/* processed backscatter data */
	float		back_scale;
	short int	back[MBF_HSLDEOIH_BEAMS];
	};

/* data structure for comment records */
struct mbf_hsldeoih_comment_struct
	{
	/* comment */
	char	comment[MBF_HSLDEOIH_MAXLINE];
	};
