/*--------------------------------------------------------------------
 *    The MB-system:	mb_io.h	1/19/93
 *    $Id: mb_io.h,v 5.14 2003-04-17 21:05:23 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2002, 2003 by
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
 * mb_io.h defines data structures used by MBIO "mb_" functions 
 * to store parameters relating to reading data from or writing
 * data to a single multibeam data file.
 *
 * Author:	D. W. Caress
 * Date:	January 19, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.13  2002/10/15 18:34:58  caress
 * Release 5.0.beta25
 *
 * Revision 5.12  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.11  2002/07/25 19:09:04  caress
 * Release 5.0.beta21
 *
 * Revision 5.10  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.9  2002/05/29 23:40:48  caress
 * Release 5.0.beta18
 *
 * Revision 5.8  2002/05/02 04:00:41  caress
 * Release 5.0.beta17
 *
 * Revision 5.7  2001/10/12 21:10:41  caress
 * Added interpolation of attitude data.
 *
 * Revision 5.6  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.5  2001/06/30  17:40:14  caress
 * Release 5.0.beta02
 *
 * Revision 5.4  2001/06/08  21:44:01  caress
 * Version 5.0.beta01
 *
 * Revision 5.3  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.2  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.1  2000/12/10  20:26:50  caress
 * Version 5.0.alpha02
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.12  2000/09/30  06:29:44  caress
 * Snapshot for Dale.
 *
 * Revision 4.11  2000/04/19  21:06:32  caress
 * Added MB_PATH_MAX define
 *
 * Revision 4.10  2000/04/19  20:55:07  caress
 * Added datalist parsing structure for use in code in mb_format.c
 *
 * Revision 4.9  1998/12/17  23:01:15  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.8  1998/10/05 17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.7  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.6  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.7  1997/04/17  15:07:36  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.6  1997/03/14  16:07:24  caress
 * Made save_label string 8 bytes.
 *
 * Revision 4.5  1996/08/05  15:25:43  caress
 * Just redid i/o for Simrad sonars, including adding EM12S and EM121 support.
 *
 * Revision 4.4  1996/04/22  10:59:25  caress
 * Put DTR defines in this include file.
 *
 * Revision 4.3  1995/03/22  18:59:33  caress
 * Added record counting ints for use with sburivax format.
 *
 * Revision 4.2  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.1  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.5  1994/03/05  02:14:41  caress
 * Altered to accomodate MBF_SB2100RW format.
 *
 * Revision 4.4  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.3  1994/02/18  21:21:15  caress
 * Added amp_num array.
 *
 * Revision 4.2  1994/02/18  18:42:25  caress
 * Added *new_ss pointer - this should have already been here.
 *
 * Revision 4.1  1994/02/18  18:23:26  caress
 * Added parameter format_num to mb_io_struct; this
 * stores the data format array id in addition to the external
 * format id.
 *
 * Revision 4.0  1994/02/17  20:16:10  caress
 * First cut at new version.  Resetting data passed to
 * include three types instead of two - now supposed to
 * handle bathymetry and beam amplitude and sidescan, all
 * at once.  Added alongtrack distances to beam and pixel
 * values.  Renamed some fundamental arrays - what was
 * *back (backscatter) is now *amp (beam amplitude) and/or
 * *ss (sidescan). More changes will undoubtably follow.
 *
 * Revision 3.0  1993/04/23  16:01:21  dale
 * Initial version
 *
 */

/* include this code only once */
#ifndef MB_IO_DEF
#define MB_IO_DEF

#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

#ifndef MB_STATUS_DEF
#include "mb_status.h"
#endif

/* MBIO input/output control structure */
struct mb_io_struct
	{
	/* format parameters */
	int	format;		/* data format id */
	int	system;		/* sonar system id */
	int	beams_bath_max;	/* maximum number of bathymetry beams */
	int	beams_amp_max;	/* maximum number of amplitude beams
					- either 0 or = beams_bath */
	int	pixels_ss_max;	/* maximum number of sidescan pixels */
	char	format_name[MB_NAME_LENGTH];
	char	system_name[MB_NAME_LENGTH];
	char	format_description[MB_DESCRIPTION_LENGTH];
	int	numfile;	/* the number of parallel files required for i/o */
	int	filetype;	/* type of files used (normal, xdr, or gsf) */
	int	filemode;	/* file mode (read or write) */
	int	variable_beams; /* if true then number of beams variable */
	int	traveltime;	/* if true then traveltime and angle data supported */
	int	beam_flagging;	/* if true then beam flagging supported */
	int	nav_source;	/* data record types containing the primary navigation */
	int	heading_source;	/* data record types containing the primary heading */
	int	vru_source;	/* data record types containing the primary vru */
	double	beamwidth_xtrack;   /* nominal acrosstrack beamwidth */
	double	beamwidth_ltrack;   /* nominal alongtrack beamwidth */

	/* control parameters - see mbio manual pages for explanation */
	int	pings;		/* controls ping averaging */
	int	lonflip;	/* controls longitude range */
	double	bounds[4];	/* locations bounds of acceptable data */
	int	btime_i[7];	/* beginning time of acceptable data */
	int	etime_i[7];	/* ending time of acceptable data */
	double	btime_d;	/* beginning time of acceptable data 
					in "_d" format (unix seconds) */
	double	etime_d;	/* ending time of acceptable data 
					in "_d" format (unix seconds) */
	double	speedmin;	/* minimum ship speed of acceptable data
					in km/hr */
	double	timegap;	/* maximum time between pings without
					a data gap */

	/* file descriptor, file name, and usage flag */
	FILE	*mbfp;		/* file descriptor */
	char	file[MB_PATH_MAXLINE];	/* file name */
	long	file_pos;	/* file position at start of
				    last record read */
	long	file_bytes;	/* number of bytes read from file */
	FILE	*mbfp2;		/* file descriptor #2 */
	char	file2[MB_PATH_MAXLINE];	/* file name #2 */
	long	file2_pos;	/* file position #2 at start of
				    last record read */
	long	file2_bytes;	/* number of bytes read from file */
	FILE	*mbfp3;		/* file descriptor #3 */
	char	file3[MB_PATH_MAXLINE];	/* file name #3 */
	long	file3_pos;	/* file position #3 at start of
				    last record read */
	long	file3_bytes;	/* number of bytes read from file */
	void	*xdrs;		/* XDR stream handle */
	void	*xdrs2;		/* XDR stream handle #2 */

	/* read or write history */
	int	fileheader;	/* indicates whether file header has
					been read or written */
	int	hdr_comment_size;/* number of characters in
					header_comment string */
	int	hdr_comment_loc;/* number of characters already extracted
					from header_comment string */
	char	*hdr_comment;	/* placeholder for long comment strings
					for formats using a single
					comment string in a file header */
	int	irecord_count;	/* counting variable used for VMS derived
					data formats to remove extra 
					bytes (e.g. sburivax format) */
	int	orecord_count;	/* counting variable used for VMS derived
					data formats to insert extra 
					bytes (e.g. sburivax format) */

	/* pointer to structure containing raw data (could be any format) */
	int	structure_size;
	int	data_structure_size;
	int	header_structure_size;
	void	*raw_data;
	void	*store_data;

	/* working variables */
	int	ping_count;	/* number of pings read or written so far */
	int	nav_count;	/* number of nav records read or written so far */
	int	comment_count;	/* number of comments read or written so far */
	int	pings_avg;	/* number of pings currently averaged */
	int	pings_read;	/* number of pings read this binning cycle */
	int	error_save;	/* saves time gap error to end of binning */
	double	last_time_d;
	double	last_lon;
	double	last_lat;
	double	old_time_d;
	double	old_lon;
	double	old_lat;
	double	old_ntime_d;
	double	old_nlon;
	double	old_nlat;

	/* data binning variables */
	int	pings_binned;
	double	time_d;
	double	lon;
	double	lat;
	double	speed;
	double	heading;
	char	*beamflag;
	double	*bath;
	double	*amp;
	double	*bath_acrosstrack;
	double	*bath_alongtrack;
	int	*bath_num;
	int	*amp_num;
	double	*ss;
	double	*ss_acrosstrack;
	double	*ss_alongtrack;
	int	*ss_num;

	/* current ping variables */
	int	need_new_ping;
	int	new_kind;
	int	new_error;
	char	new_comment[MB_COMMENT_MAXLINE];
	int	new_time_i[7];
	double	new_time_d;
	double	new_lon;
	double	new_lat;
	double	new_speed;
	double	new_heading;
	int	new_beams_bath;	/* number of bathymetry beams */
	int	new_beams_amp;	/* number of amplitude beams
					- either 0 or = beams_bath */
	int	new_pixels_ss;	/* number of sidescan pixels */
	char	*new_beamflag;
	double	*new_bath;
	double	*new_amp;
	double	*new_bath_acrosstrack;
	double	*new_bath_alongtrack;
	double	*new_ss;
	double	*new_ss_acrosstrack;
	double	*new_ss_alongtrack;
	
	/* variables for projections to and from projected coordinates */
	int	projection_initialized;
	void 	*pjptr;

	/* variables for interpolating/extrapolating navigation 
		for formats containing nav as asynchronous
		position records separate from ping data */
	int nfix;
	double fix_time_d[MB_ASYNCH_SAVE_MAX];
	double fix_lon[MB_ASYNCH_SAVE_MAX];
	double fix_lat[MB_ASYNCH_SAVE_MAX];

	/* variables for interpolating/extrapolating attitude
		for formats containing attitude as asynchronous
		data records separate from ping data */
	int nattitude;
	double attitude_time_d[MB_ASYNCH_SAVE_MAX];
	double attitude_heave[MB_ASYNCH_SAVE_MAX];
	double attitude_roll[MB_ASYNCH_SAVE_MAX];
	double attitude_pitch[MB_ASYNCH_SAVE_MAX];

	/* variables for interpolating/extrapolating heading
		for formats containing heading as asynchronous
		data records separate from ping data */
	int nheading;
	double heading_time_d[MB_ASYNCH_SAVE_MAX];
	double heading_heading[MB_ASYNCH_SAVE_MAX];
	
	/* variables for accumulating MBIO notices */
	int	notice_list[MB_NOTICE_MAX];
	
	/* variables for saving information */
	char	save_label[12];
	int	save_label_flag;
	int	save_flag;
	int	save1;
	int	save2;
	int	save3;
	int	save4;
	int	save5;
	int	save6;
	int	save7;
	int	save8;
	int	save9;
	int	save10;
	int	save11;
	int	save12;
	int	save13;
	int	save14;
	double	saved1;
	double	saved2;

	/* function pointers for allocating and deallocating format
		specific structures */
	int (*mb_io_format_alloc)(int verbose, void *mbio_ptr, int *error);
	int (*mb_io_format_free)(int verbose, void *mbio_ptr, int *error);
	int (*mb_io_store_alloc)(int verbose, void *mbio_ptr,
		void **store_ptr, int *error);
	int (*mb_io_store_free)(int verbose, void *mbio_ptr,
		void **store_ptr, int *error);
	
	/* function pointers for reading and writing records */
	int (*mb_io_read_ping)(int verbose, void *mbio_ptr, 
		void *store_ptr, int *error);
	int (*mb_io_write_ping)(int verbose, void *mbio_ptr, 
		void *store_ptr, int *error);
		
	/* function pointers for extracting and inserting data */
	int (*mb_io_extract)(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error);
	int (*mb_io_insert)(int verbose, void *mbio_ptr, void *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error);
	int (*mb_io_extract_nav)(int verbose, void *mbio_ptr, void *store_ptr, int *kind,
		int time_i[7], double *time_d, 
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error);
	int (*mb_io_insert_nav)(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d, 
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave, 
		int *error);
	int (*mb_io_extract_altitude)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		double *transducer_depth, double *altitude,
		int *error);
	int (*mb_io_insert_altitude)(int verbose, void *mbio_ptr, void *store_ptr,
		double transducer_depth, double altitude,
		int *error);
	int (*mb_io_extract_svp)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		int *nsvp,
		double *depth, double *velocity,
		int *error);
	int (*mb_io_insert_svp)(int verbose, void *mbio_ptr, void *store_ptr,
		int nsvp,
		double *depth, double *velocity,
		int *error);
	int (*mb_io_ttimes)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nbeams,
		double *ttimes, double	*angles, 
		double *angles_forward, double *angles_null,
		double *heave, double *alongtrack_offset, 
		double *draft, double *ssv, int *error);
	int (*mb_io_detects)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nbeams, int *detects, int *error);
	int (*mb_io_extract_rawss)(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nrawss,
		double *rawss, 
		double *rawssacrosstrack, 
		double *rawssalongtrack, 
		int *error);
	int (*mb_io_insert_rawss)(int verbose, void *mbio_ptr, void *store_ptr,
		int nrawss,
		double *rawss, 
		double *rawssacrosstrack, 
		double *rawssalongtrack, 
		int *error);
	int (*mb_io_copyrecord)(int verbose, void *mbio_ptr,
		void *store_ptr, void *copy_ptr, int *error);

	};

/* MBIO buffer control structure */
struct mb_buffer_struct
	{
	void *buffer[MB_BUFFER_MAX];
	int buffer_kind[MB_BUFFER_MAX];
	int nbuffer;
	};

/* MBIO datalist control structure */
#define MB_DATALIST_RECURSION_MAX 	25
struct mb_datalist_struct {
	int	open;
	int	recursion;
	int	look_processed;
	int	local_weight;
	int	weight_set;
	double	weight;
	FILE	*fp;
	char	path[MB_PATH_MAXLINE];
	struct mb_datalist_struct *datalist;
	};

/* end conditional include */
#endif
