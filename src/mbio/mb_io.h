/*--------------------------------------------------------------------
 *    The MB-system:	mb_io.h	3.00	1/19/93
 *    $Id: mb_io.h,v 3.0 1993-04-23 16:01:21 dale Exp $
 *
 *    Copyright (c) 1993 by 
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
 */

/* define file i/o usage flags */
#define	MB_READ_ONLY	1
#define	MB_WRITE_ONLY	2
#define	MBR_READ_ONLY	3
#define	MBR_WRITE_ONLY	4

/* declare buffer maximum */
#define	MB_BUFFER_MAX	5000

/* MBIO input/output control structure */
struct mb_io_struct
	{
	/* file descriptor, file name, and usage flag */
	FILE	*mbfp;		/* file descriptor */
	char	file[256];	/* file name */
	int	usage;		/* file usage flag - only one sort of
				   i/o is allowed for each file
				   usage = MB_READ_ONLY   : mb_read 
				                            or mb_get
					   MB_WRITE_ONLY  : mb_write 
				                            or mb_put 
					   MBR_READ_ONLY  : mbr_read 
					   MBR_WRITE_ONLY : mbr_write  */

	/* pointer to structure containing raw data (could be any format) */
	int	structure_size;
	int	data_structure_size;
	int	header_structure_size;
	char	*raw_data;
	char	*store_data;

	/* control parameters - see mbio manual pages for explanation */
	int	format;		/* data format id */
	int	beams_bath;	/* maximum number of bathymetry beams */
	int	beams_back;	/* maximum number of backscatter beams */
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
	int	*bathdist;
	int	*bathnum;
	int	*back;
	int	*backdist;
	int	*backnum;

	/* current ping variables */
	int	need_new_ping;
	int	new_kind;
	int	new_error;
	char	new_comment[256];
	int	new_time_i[6];
	double	new_time_d;
	double	new_lon;
	double	new_lat;
	double	new_speed;
	double	new_heading;
	int	*new_bath;
	int	*new_bathdist;
	int	*new_back;
	int	*new_backdist;

	};

/* MBIO buffer control structure */
struct mb_buffer_struct
	{
	char *buffer[MB_BUFFER_MAX];
	int buffer_kind[MB_BUFFER_MAX];
	int nbuffer;
	};
