/*--------------------------------------------------------------------
 *    The MB-system:	mb_io.h	1/19/93
 *    $Id: mb_io.h,v 4.0 1994-03-06 00:01:56 caress Exp $
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

/* MBIO input/output control structure */
struct mb_io_struct
	{
	/* file descriptor, file name, and usage flag */
	FILE	*mbfp;		/* file descriptor */
	char	file[256];	/* file name */

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
	int	btime_i[6];	/* beginning time of acceptable data */
	int	etime_i[6];	/* ending time of acceptable data */
	double	btime_d;	/* beginning time of acceptable data 
					in "_d" format */
	double	etime_d;	/* ending time of acceptable data 
					in "_d" format */
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
	int	*bath;
	int	*amp;
	int	*bath_acrosstrack;
	int	*bath_alongtrack;
	int	*bath_num;
	int	*amp_num;
	int	*ss;
	int	*ss_acrosstrack;
	int	*ss_alongtrack;
	int	*ss_num;

	/* current ping variables */
	int	need_new_ping;
	int	new_kind;
	int	new_error;
	char	new_comment[MB_COMMENT_MAXLINE];
	int	new_time_i[6];
	double	new_time_d;
	double	new_lon;
	double	new_lat;
	double	new_speed;
	double	new_heading;
	int	*new_bath;
	int	*new_amp;
	int	*new_bath_acrosstrack;
	int	*new_bath_alongtrack;
	int	*new_ss;
	int	*new_ss_acrosstrack;
	int	*new_ss_alongtrack;

	};

/* MBIO buffer control structure */
struct mb_buffer_struct
	{
	char *buffer[MB_BUFFER_MAX];
	int buffer_kind[MB_BUFFER_MAX];
	int nbuffer;
	};
