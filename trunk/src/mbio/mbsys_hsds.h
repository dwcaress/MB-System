/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_hsds.h	2/16/93
 *	$Id$
 *
 *    Copyright (c) 1993-2015 by
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
 * mbsys_hsds.h defines the data structures used by MBIO functions
 * to store data from Hydrosweep DS multibeam sonar systems.
 * The data formats which are commonly used to store Hydrosweep DS
 * data in files include
 *      MBF_HSATLRAW : MBIO ID 21
 *      MBF_HSLDEDMB : MBIO ID 22
 *      MBF_HSURICEN : MBIO ID 23
 *      MBF_HSLDEOIH : MBIO ID 24
 *
 * Author:	D. W. Caress
 * Date:	February 16, 1993
 * $Log: mbsys_hsds.h,v $
 * Revision 5.5  2005/11/05 00:48:03  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.4  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.3  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.2  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.5  2000/09/30  06:31:19  caress
 * Snapshot for Dale.
 *
 * Revision 4.4  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 1.1  1998/10/05  17:46:15  caress
 * Initial revision
 *
 * Revision 4.3  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.3  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.2  1995/02/22  21:56:08  caress
 * Put array of per beam gain values in this include file.
 *
 * Revision 4.2  1995/02/22  21:56:08  caress
 * Put array of per beam gain values in this include file.
 *
 * Revision 4.1  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/17  20:36:40  caress
 * First cut at new version.  Comment changes only.
 *
 * Revision 3.1  1993/06/09  08:21:11  caress
 * Added definition of beam angle spacing.  The actual spacing
 * seems to be 1.510 degrees instead of the 1.525 degrees listed
 * in the Atlas manuals (based on Hydrosweep DS data collected
 * on the R/V Ewing on June 9, 1993.
 *
 * Revision 3.0  1993/05/14  22:59:28  sohara
 * initial version
 *
 */
/*
 * Notes on the MBSYS_HSDS data structure:
 *   1. Hydrosweep DS multibeam systems output raw data in an
 *      ascii format.  The data consists of a number of different
 *      multi-line ascii records.
 *   2. The DS systems output 59 beams of bathymetry and 59 beams
 *      of amplitude measurements, along with a plethora of other
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
 *   5. The kind value in the mbsys_hsds_struct indicates whether the
 *      mbsys_hsds_data_struct structure holds data from a ping or
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
#define MBSYS_HSDS_MAXVEL 30

/* maximum line length in characters */
#define MBSYS_HSDS_MAXLINE 200

/* number of beams for hydrosweep */
#define MBSYS_HSDS_BEAMS 59

/* angular beam spacing for Hydrosweep DS
	- this is supposed to be 1.525 degrees but seems
	to really be 1.510 degrees */
#define MBSYS_HSDS_BEAM_SPACING 1.510

struct mbsys_hsds_struct
	{
	/* type of data record */
	int	kind;

	/* position (all records but comment ) */
	double	lon;
	double	lat;

	/* time stamp (all records but comment ) */
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
	int	distance[MBSYS_HSDS_BEAMS];
	int	depth[MBSYS_HSDS_BEAMS];

	/* travel time data (ERGNSLZT) */
	double	course_ground;
	double	speed_ground;
	double	heave;
	double	roll;
	double	time_center;
	double	time_scale;
	int	time[MBSYS_HSDS_BEAMS];
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
	int	amplitude[MBSYS_HSDS_BEAMS];
	int	echo_scale[16];
	int	echo_duration[MBSYS_HSDS_BEAMS];

	/* mean velocity (ERGNHYDI) */
	double	draught;
	double	vel_mean;
	double	vel_keel;
	double	tide;

	/* water velocity profile (HS_ERGNCTDS) */
	int	num_vel;
	double	vdepth[MBSYS_HSDS_MAXVEL];
	double	velocity[MBSYS_HSDS_MAXVEL];

	/* navigation source (ERGNPOSI) */
	double	pos_corr_x;
	double	pos_corr_y;
	char	sensors[10];

	/* comment (LDEOCMNT) */
	char	comment[MBSYS_HSDS_MAXLINE];

	/* processed amplitude data */
	double	back_scale;
	int	back[MBSYS_HSDS_BEAMS];
	};

/* system specific function prototypes */
int mbsys_hsds_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_hsds_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_hsds_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_hsds_extract(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_hsds_insert(int verbose, void *mbio_ptr, void *store_ptr,
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_hsds_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles,
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset,
			double *draft, double *ssv, int *error);
int mbsys_hsds_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_hsds_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude,
			int *error);
int mbsys_hsds_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
int mbsys_hsds_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft,
			double roll, double pitch, double heave,
			int *error);
int mbsys_hsds_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind,
			int *nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_hsds_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_hsds_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error);
