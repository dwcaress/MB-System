/*--------------------------------------------------------------------
 *    The MB-system:	mb_io.h	1/19/93
 *    $Id: mb_io.h,v 4.8 1998-10-05 17:46:15 caress Exp $
 *
 *    Copyright (c) 1993, 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
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
 
/* declare buffer maximum */
#define	MB_BUFFER_MAX	5000

/* maximum comment length in characters */
#define MB_COMMENT_MAXLINE 1944

/* maximum number of navigation points saved */
#define MB_NAV_SAVE_MAX 20

/* MBIO input/output control structure */
struct mb_io_struct
	{
	/* file descriptor, file name, and usage flag */
	FILE	*mbfp;		/* file descriptor */
	char	file[256];	/* file name */
	long	file_pos;	/* file position at start of
				    last record read */
	long	file_bytes;	/* number of bytes read from file */
	FILE	*mbfp2;		/* file descriptor #2 */
	char	file2[256];	/* file name #2 */
	long	file2_pos;	/* file position #2 at start of
				    last record read */
	long	file2_bytes;	/* number of bytes read from file */
	FILE	*mbfp3;		/* file descriptor #3 */
	char	file3[256];	/* file name #3 */
	long	file3_pos;	/* file position #3 at start of
				    last record read */
	long	file3_bytes;	/* number of bytes read from file */
	char	*xdrs;		/* XDR stream handle */

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
	char	*raw_data;
	char	*store_data;

	/* control parameters - see mbio manual pages for explanation */
	int	format;		/* data format id */
	int	format_num;	/* data format array id */
	int	beams_bath;	/* maximum number of bathymetry beams */
	int	beams_amp;	/* maximum number of amplitude beams
					- either 0 or = beams_bath */
	int	pixels_ss;	/* maximum number of sidescan pixels */
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

	/* working variables */
	int	ping_count;	/* number of pings read or written so far */
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
	char	*new_beamflag;
	double	*new_bath;
	double	*new_amp;
	double	*new_bath_acrosstrack;
	double	*new_bath_alongtrack;
	double	*new_ss;
	double	*new_ss_acrosstrack;
	double	*new_ss_alongtrack;

	/* variables for interpolating/extrapolating navigation 
		for formats containing nav as asynchronous
		position records separate from ping data */
	int nfix;
	double fix_time_d[MB_NAV_SAVE_MAX];
	double fix_lon[MB_NAV_SAVE_MAX];
	double fix_lat[MB_NAV_SAVE_MAX];
	
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
	double	saved1;
	};

/* MBIO buffer control structure */
struct mb_buffer_struct
	{
	char *buffer[MB_BUFFER_MAX];
	int buffer_kind[MB_BUFFER_MAX];
	int nbuffer;
	};
