/*--------------------------------------------------------------------
 *    The MB-system:	mbf_hsatlraw.h	1/20/93
 *	$Id: mbf_hsatlraw.h,v 4.4 2000-09-30 06:34:20 caress Exp $
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
 * mbf_hsatlraw.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_HSATLRAW format (MBIO id 21).  
 *
 * Author:	D. W. Caress
 * Date:	January 20, 1993
 * $Log: not supported by cvs2svn $
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
 * Revision 3.0  1993/05/14  22:50:34  sohara
 * initial version
 *
 */
/*
 * Notes on the MBF_HSATLRAW data format:
 *   1. Hydrosweep DS multibeam systems output raw data in this
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
 *   5. The kind value in the mbf_hsatlraw_struct indicates whether the
 *      mbf_hsatlraw_data_struct structure holds data from a ping or
 *      data from some other record:
 *        kind = 1 : data from a survey ping 
 *                   (ERGNMESS + ERGNSLZT + ERGNAMPL)
 *        kind = 2 : comment (LDEOCOMM)
 *        kind = 3 : data from a calibrate ping 
 *                   (ERGNEICH + ERGNSLZT + ERGNAMPL)
 *        kind = 4 : mean and keel velocity (ERGNHYDI)
 *        kind = 5 : water velocity profile (ERGNCTDS)
 *        kind = 6 : standby navigation (ERGNPARA)
 *        kind = 7 : navigation source (ERGNPOSI)
 *   6. The data structure defined below includes all of the values
 *      which are passed in Hydrosweep records.
 */

/* maximum number of depth-velocity pairs */
#define MBF_HSATLRAW_MAXVEL 30

/* maximum line length in characters */
#define MBF_HSATLRAW_MAXLINE 200

/* number of beams for hydrosweep */
#define MBF_HSATLRAW_BEAMS 59

/* define id's for the different types of raw Hydrosweep records */
#define MBF_HSATLRAW_RECORDS	11
#define	MBF_HSATLRAW_NONE	0
#define	MBF_HSATLRAW_RAW_LINE	1
#define	MBF_HSATLRAW_ERGNHYDI	2
#define	MBF_HSATLRAW_ERGNPARA	3
#define	MBF_HSATLRAW_ERGNPOSI	4
#define	MBF_HSATLRAW_ERGNEICH	5
#define	MBF_HSATLRAW_ERGNMESS	6
#define	MBF_HSATLRAW_ERGNSLZT	7
#define	MBF_HSATLRAW_ERGNCTDS	8
#define	MBF_HSATLRAW_ERGNAMPL	9
#define	MBF_HSATLRAW_LDEOCMNT	10
char *mbf_hsatlraw_labels[] = {
	"NONE    ", "RAW_LINE", "ERGNHYDI", "ERGNPARA", "ERGNPOSI", 
	"ERGNEICH", "ERGNMESS", "ERGNSLZT", "ERGNCTDS", "ERGNAMPL",
	"LDEOCMNT"};

struct mbf_hsatlraw_struct
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
	int	distance[MBF_HSATLRAW_BEAMS];
	int	depth[MBF_HSATLRAW_BEAMS];

	/* travel time data (ERGNSLZT) */
	double	course_ground;
	double	speed_ground;
	double	heave;
	double	roll;
	double	time_center;
	double	time_scale;
	int	time[MBF_HSATLRAW_BEAMS];
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
	int	amplitude[MBF_HSATLRAW_BEAMS];
	int	echo_scale[16];
	int	echo_duration[MBF_HSATLRAW_BEAMS];

	/* mean velocity (ERGNHYDI) */
	double	draught;
	double	vel_mean;
	double	vel_keel;
	double	tide;

	/* water velocity profile (HS_ERGNCTDS) */
	int	num_vel;
	double	vdepth[MBF_HSATLRAW_MAXVEL];
	double	velocity[MBF_HSATLRAW_MAXVEL];

	/* navigation source (ERGNPOSI) */
	double	pos_corr_x;
	double	pos_corr_y;
	char	sensors[10];

	/* comment (LDEOCMNT) */
	char	comment[MBF_HSATLRAW_MAXLINE];
	};	
