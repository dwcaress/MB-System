/*--------------------------------------------------------------------
 *    The MB-system:	mblist.c	2/1/93
 *    $Id: mblist.c,v 5.5 2001-09-17 23:21:14 caress Exp $
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
 * MBLIST prints the specified contents of a swath sonar data  
 * file to stdout. The form of the output is quite flexible; 
 * MBLIST is tailored to produce ascii files in spreadsheet  
 * style with data columns separated by tabs.
 *
 * Author:	D. W. Caress
 * Date:	February 1, 1993
 *
 * Note:	This program is based on the program mblist created
 *		by A. Malinverno (currently at Schlumberger, formerly
 *		at L-DEO) in August 1991.  It also includes elements
 *		derived from the program mbdump created by D. Caress
 *		in 1990.
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.4  2001/07/20  00:34:38  caress
 * Release 5.0.beta03
 *
 * Revision 5.3  2001/06/29  22:50:23  caress
 * Atlas Hydrosweep DS2 raw data and SURF data formats.
 *
 * Revision 5.2  2001/06/08  21:45:46  caress
 * Version 5.0.beta01
 *
 * Revision 5.1  2001/03/22  21:15:49  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.26  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.25  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.24  2000/09/11  20:10:02  caress
 * Linked to new datalist parsing functions. Now supports recursive datalists
 * and comments in datalists.
 *
 * Revision 4.23  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.22  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.21  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.21  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.20  1995/11/28  21:03:36  caress
 * Fixed scaling for meters to feet.
 *
 * Revision 4.19  1995/11/22  22:21:36  caress
 * Now handles bathymetry in feet with -W option.
 *
 * Revision 4.18  1995/09/19  14:56:59  caress
 * Added output for ping interval.
 *
 * Revision 4.17  1995/08/17  15:04:52  caress
 * Revision for release 4.3.
 *
 * Revision 4.16  1995/08/10  15:39:36  caress
 * mblist now works with datalist files.
 *
 * Revision 4.15  1995/07/13  20:13:36  caress
 * Added output options x and y for longitude and latitude in
 * integer degrees + decimal minutes + EW/NS
 *
 * Revision 4.14  1995/06/06  13:31:48  caress
 * Fixed warnings under Solaris by explicit casting of strlen result.
 *
 * Revision 4.13  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.12  1995/03/22  18:33:38  caress
 * Fixed output formats for latitude and depth values.
 *
 * Revision 4.11  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.10  1995/03/02  13:49:21  caress
 * Fixed bug related to error messages.
 *
 * Revision 4.9  1995/01/18  20:20:46  caress
 * Fixed bug messing up sidescan output.
 *
 * Revision 4.8  1994/12/21  20:22:30  caress
 * Fixed bug in printing sidescan values.
 *
 * Revision 4.7  1994/10/21  13:02:31  caress
 * Release V4.0
 *
 * Revision 4.6  1994/07/29  19:02:56  caress
 * Changes associated with supporting byte swapped Lynx OS and
 * using unix second time base.
 *
 * Revision 4.5  1994/06/21  22:53:08  caress
 * Changes to support PCs running Lynx OS.
 *
 * Revision 4.4  1994/06/05  22:31:22  caress
 * Major revision changing the manner in which complete
 * dumps occur.  Options added to control the range of beams
 * or pixels which are output.
 *
 * Revision 4.3  1994/06/01  21:00:55  caress
 * Added new dump modes with acrosstrack and alongtrack
 * values. Added topography dumps.
 *
 * Revision 4.2  1994/04/29  18:01:20  caress
 * Added output option "j" for time string in form: year jday daymin sec
 *
 * Revision 4.1  1994/04/12  18:50:33  caress
 * Added #ifdef IRIX statements for compatibility with
 * SGI machines.  The system routine timegm does not exist
 * on SGI's; mktime must be used instead.
 *
 * Revision 4.0  1994/03/06  00:13:22  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 3.6  1993/09/11  15:44:30  caress
 * Fixed bug so that mblist no longer checks for a valid depth
 * even if it is not outputting depth.
 *
 * Revision 3.5  1993/06/13  07:47:20  caress
 * Added new time output options "m" and "u" and changed
 * old options "U" to "M" and "u" to "U
 * Now time options are:
 *   T  for a time string (yyyy/mm/dd/hh/mm/ss)
 *   t  for a time string (yyyy mm dd hh mm ss)
 *   J  for a time string (yyyy jd hh mm ss)
 *   M  for mbio time in minutes since 1/1/81 00:00:00
 *   m  for time in minutes since first record
 *   U  for unix time in seconds since 1/1/70 00:00:00
 *   u  for time in seconds since first record
 *
 * Revision 3.4  1993/06/13  03:02:18  caress
 * Added new output option "A", which outputs the apparent
 * crosstrack seafloor slope in degrees from vertical. This
 * value is calculated by fitting a line to the bathymetry
 * data for each ping. Obtaining a time series of the apparent
 * seafloor slope is useful for detecting errors in the vertical
 * reference used by the multibeam sonar.
 *
 * Revision 3.3  1993/06/09  11:47:44  caress
 * Fixed problem with unix time value output. In unix time
 * months are counted 0-11 instead of 1-12.
 *
 * Revision 3.2  1993/05/17  16:37:18  caress
 * Fixed problem where program crashed if -F was used.
 *
 * Revision 3.0  1993/05/04  22:20:17  dale
 * Initial version
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"

/* GMT include files */
#include "gmt_nan.h"

/* local options */
#define	MAX_OPTIONS	25
#define	DUMP_MODE_LIST	1
#define	DUMP_MODE_BATH	2
#define	DUMP_MODE_TOPO	3
#define	DUMP_MODE_AMP	4
#define	DUMP_MODE_SS	5
#define	MBLIST_CHECK_ON			0
#define	MBLIST_CHECK_ON_NULL		1
#define	MBLIST_CHECK_OFF_RAW		2
#define	MBLIST_CHECK_OFF_NAN		3
#define	MBLIST_CHECK_OFF_FLAGNAN	4
#define	MBLIST_SET_OFF	0
#define	MBLIST_SET_ON	1
#define	MBLIST_SET_ALL	2

/* function prototypes */
int set_output(	int	verbose, 
		int	beams_bath, 
		int	beams_amp, 
		int	pixels_ss, 
		int	use_bath, 
		int	use_amp, 
		int	use_ss, 
		int	dump_mode, 
		int	beam_set, 
		int	pixel_set, 
		int	beam_vertical, 
		int	pixel_vertical, 
		int	*beam_start, 
		int	*beam_end, 
		int	*pixel_start, 
		int	*pixel_end, 
		int	*n_list, 
		char	*list, 
		int	*error);
int set_bathyslope(int verbose,
	int nbath, char *beamflag, double *bath, double *bathacrosstrack,
	int *ndepths, double *depths, double *depthacrosstrack, 
	int *nslopes, double *slopes, double *slopeacrosstrack, 
	int *error);
int get_bathyslope(int verbose,
	int ndepths, double *depths, double *depthacrosstrack, 
	int nslopes, double *slopes, double *slopeacrosstrack, 
	double acrosstrack, double *depth,  double *slope, 
	int *error);
int printsimplevalue(int verbose, 
	double value, int width, int precision, 
	int ascii, int *invert, int *flipsign, int *error);
int printNaN(int verbose, int ascii, int *invert, int *flipsign, int *error);

/* NaN value */
double	NaN;

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char rcs_id[] = "$Id: mblist.c,v 5.5 2001-09-17 23:21:14 caress Exp $";
	static char program_name[] = "MBLIST";
	static char help_message[] =  "MBLIST prints the specified contents of a swath data \nfile to stdout. The form of the output is quite flexible; \nMBLIST is tailored to produce ascii files in spreadsheet \nstyle with data columns separated by tabs.";
	static char usage_message[] = "mblist [-Byr/mo/da/hr/mn/sc -Ddump_mode -Eyr/mo/da/hr/mn/sc \n-Fformat -H -Ifile -Llonflip -Mbeam_start/beam_end -Npixel_start/pixel_end \n-Ooptions -Ppings -Rw/e/s/n -Sspeed -Ttimegap -Ucheck -V -W -Zsegment]";
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message;

	/* MBIO read control parameters */
	int	read_datalist = MB_NO;
	char	read_file[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	int	format;
	int	pings;
	int	pings_read;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[MB_PATH_MAXLINE];
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* output format list controls */
	char	list[MAX_OPTIONS];
	int	n_list;
	int	beam_set = MBLIST_SET_OFF;
	int	beam_start;
	int	beam_end;
	int	beam_vertical;
	int	pixel_set = MBLIST_SET_OFF;
	int	pixel_start;
	int	pixel_end;
	int	pixel_vertical;
	int	dump_mode = 1;
	double	distance_total;
	int	nread;
	int	beam_status = MB_SUCCESS;
	int	pixel_status = MB_SUCCESS;
	int	time_j[5];
	int	use_bath = MB_NO;
	int	use_amp = MB_NO;
	int	use_ss = MB_NO;
	int	use_slope = MB_NO;
	int	check_values = MBLIST_CHECK_ON;
	int	check_bath = MB_NO;
	int	check_amp = MB_NO;
	int	check_ss = MB_NO;
	int	invert_next_value = MB_NO;
	int	signflip_next_value = MB_NO;
	int	first = MB_YES;
	int	ascii = MB_YES;
	int	segment = MB_NO;
	char	segment_tag[MB_PATH_MAXLINE];

	/* MBIO read values */
	void	*mbio_ptr = NULL;
	void	*store_ptr = NULL;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sonardepth;
	double	draft;
	double	roll;
	double	pitch;
	double	heave;
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[MB_COMMENT_MAXLINE];
	int	icomment = 0;

	/* additional time variables */
	int	first_m = MB_YES;
	double	time_d_ref;
	struct tm	time_tm;
	int	first_u = MB_YES;
	time_t	time_u;
	time_t	time_u_ref;

	/* crosstrack slope values */
	double	avgslope;
	double	sx, sy, sxx, sxy;
	int	ns;
	double	angle, depth, slope;
	int	ndepths;
	double	*depths;
	double	*depthacrosstrack;
	int	nslopes;
	double	*slopes;
	double	*slopeacrosstrack;

	/* course calculation variables */
	int	use_course = MB_NO;
	int	use_time_interval = MB_NO;
	double	course, course_old;
	double	time_d_old, dt;
	double	time_interval;
	double	speed_made_good, speed_made_good_old;
	double	navlon_old, navlat_old;
	double	dx, dy, dist;
	double	delta, a, b, theta;
	double	dlon, dlat, minutes;
	int	degrees;
	char	hemi;
	double	headingx, headingy, mtodeglon, mtodeglat;

	/* bathymetry feet flag */
	int	bathy_in_feet = MB_NO;
	double	bathy_scale;

	int	read_data;
	double	distmin;
	int	found;
	int	i, j, k;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (read_file, "stdin");

	/* set up the default list controls 
		(Time, lon, lat, heading, speed, along-track distance, center beam depth) */
	list[0]='T';
	list[0]='X';
	list[1]='Y';
	list[2]='H';
	list[2]='S';
	list[2]='L';
	list[3]='Z';
	n_list = 7;

	/* set dump mode flag to DUMP_MODE_LIST */
	dump_mode = DUMP_MODE_LIST;

	/* get NaN value */
#ifdef GMT3_0
	NaN = zero/zero;
#else
	GMT_make_dnan(NaN);
#endif

	/* process argument list */
	while ((c = getopt(argc, argv, "AaB:b:D:d:E:e:F:f:I:i:L:l:M:m:N:n:O:o:P:p:QqR:r:S:s:T:t:U:u:Z:z:VvWwHh")) != -1)
	  switch (c) 
		{
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'A':
		case 'a':
			ascii = MB_NO;
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			btime_i[6] = 0;
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%d", &dump_mode);
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
			etime_i[6] = 0;
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", read_file);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'M':
		case 'm':
			if (optarg[0] == 'a' || optarg[0] == 'A')
			    {
			    beam_set = MBLIST_SET_ALL;
			    }
			else
			    {
			    sscanf (optarg,"%d/%d", &beam_start,&beam_end);
			    beam_set = MBLIST_SET_ON;
			    }
			flag++;
			break;
		case 'N':
		case 'n':
			if (optarg[0] == 'a' || optarg[0] == 'A')
			    {
			    pixel_set = MBLIST_SET_ALL;
			    }
			else
			    {
			    sscanf (optarg,"%d/%d", &pixel_start,&pixel_end);
			    pixel_set = MBLIST_SET_ON;
			    }
			flag++;
			break;
		case 'O':
		case 'o':
			for(j=0,n_list=0;j<(int)strlen(optarg);j++,n_list++)
				if (n_list<MAX_OPTIONS)
					list[n_list] = optarg[j];
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf (optarg,"%d", &pings);
			flag++;
			break;
		case 'Q':
		case 'q':
			check_values = MBLIST_CHECK_OFF_RAW;
			flag++;
			break;
		case 'R':
		case 'r':
			mb_get_bounds(optarg, bounds);
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%lf", &speedmin);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf", &timegap);
			flag++;
			break;
		case 'U':
		case 'u':
			sscanf (optarg,"%d", &check_values);
			if (check_values < MBLIST_CHECK_ON 
			    || check_values > MBLIST_CHECK_OFF_FLAGNAN)
			    check_values = MBLIST_CHECK_ON;
			flag++;
			break;
		case 'W':
		case 'w':
			bathy_in_feet = MB_YES;
			break;
		case 'Z':
		case 'z':
			segment = MB_YES;
			sscanf (optarg,"%s", segment_tag);
			flag++;
			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       help:           %d\n",help);
		fprintf(stderr,"dbg2       format:         %d\n",format);
		fprintf(stderr,"dbg2       pings:          %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:        %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:      %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:      %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:      %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:      %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:     %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:     %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:     %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:     %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:     %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:     %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:     %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:     %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:     %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:     %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:     %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:     %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:     %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:     %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:       %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:        %f\n",timegap);
		fprintf(stderr,"dbg2       file:           %s\n",file);
		fprintf(stderr,"dbg2       ascii:          %d\n",ascii);
		fprintf(stderr,"dbg2       segment:        %d\n",segment);
		fprintf(stderr,"dbg2       segment_tag:    %s\n",segment_tag);
		fprintf(stderr,"dbg2       beam_set:       %d\n",beam_set);
		fprintf(stderr,"dbg2       beam_start:     %d\n",beam_start);
		fprintf(stderr,"dbg2       beam_end:       %d\n",beam_end);
		fprintf(stderr,"dbg2       pixel_set:      %d\n",pixel_set);
		fprintf(stderr,"dbg2       pixel_start:    %d\n",pixel_start);
		fprintf(stderr,"dbg2       pixel_end:      %d\n",pixel_end);
		fprintf(stderr,"dbg2       dump_mode:      %d\n",dump_mode);
		fprintf(stderr,"dbg2       check_values:   %d\n",check_values);
		fprintf(stderr,"dbg2       n_list:         %d\n",n_list);
		for (i=0;i<n_list;i++)
			fprintf(stderr,"dbg2         list[%d]:      %c\n",
						i,list[i]);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,read_file,NULL,&format,&error);

	/* set bathymetry scaling */
	if (bathy_in_feet == MB_YES)
		bathy_scale = 1.0 / 0.3048;
	else
		bathy_scale = 1.0;

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* open file list */
	if (read_datalist == MB_YES)
	    {
	    if ((status = mb_datalist_open(verbose,&datalist,
					    read_file,look_processed,&error)) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open data list file: %s\n",
			read_file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    if (status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error)
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(file, read_file);
	    read_data = MB_YES;
	    }

	/* loop over all files to be read */
	while (read_data == MB_YES)
	{

	/* initialize reading the swath file */
	if ((status = mb_read_init(
		verbose,file,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* figure out whether bath, amp, or ss will be used */
	if (dump_mode == DUMP_MODE_BATH || dump_mode == DUMP_MODE_TOPO)
		use_bath = MB_YES;
	else if (dump_mode == DUMP_MODE_AMP)
		use_amp = MB_YES;
	else if (dump_mode == DUMP_MODE_SS)
		use_ss = MB_YES;
	else
		for (i=0; i<n_list; i++) 
			{
			if (list[i] == 'Z' || list[i] == 'z'
				|| list[i] == 'A' || list[i] == 'a')
				use_bath = MB_YES;
			if (list[i] == 'B')
				use_amp = MB_YES;
			if (list[i] == 'b')
				use_ss = MB_YES;
			if (list[i] == 'h')
				use_course = MB_YES;
			if (list[i] == 's')
				use_course = MB_YES;
			if (list[i] == 'V' || list[i] == 'v')
				use_time_interval = MB_YES;
			if (list[i] == 'A' || list[i] == 'a'
				|| list[i] == 'G' || list[i] == 'g')
				use_slope = MB_YES;
			}
	if (check_values == MBLIST_CHECK_ON
		|| check_values == MBLIST_CHECK_ON_NULL)
		{
		if (use_bath == MB_YES) check_bath = MB_YES;
		if (use_amp == MB_YES) check_amp = MB_YES;
		if (use_ss == MB_YES) check_ss = MB_YES;
		}

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,beams_bath*sizeof(char),&beamflag,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&bath,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathacrosstrack,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathalongtrack,&error);
	status = mb_malloc(verbose,beams_amp*sizeof(double),&amp,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ssacrosstrack,
			&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ssalongtrack,
			&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
					&depths,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
					&depthacrosstrack,&error);
	status = mb_malloc(verbose,(beams_bath+1)*sizeof(double),
					&slopes,&error);
	status = mb_malloc(verbose,(beams_bath+1)*sizeof(double),
					&slopeacrosstrack,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
			message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
		
	/* output separator for GMT style segment file output */
	if (segment == MB_YES && ascii == MB_YES)
		{
		printf("%s\n", segment_tag);
		}

	/* read and print data */
	distance_total = 0.0;
	nread = 0;
	first = MB_YES;
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read a ping of data */
		if (pings == 1)
		    {
		    status = mb_get_all(verbose,mbio_ptr,&store_ptr,&kind,
			time_i,&time_d,&navlon,&navlat,
			&speed,&heading,
			&distance,&altitude,&sonardepth,
			&beams_bath,&beams_amp,&pixels_ss,
			beamflag,bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
		    if (error == MB_ERROR_NO_ERROR
			&& kind == MB_DATA_DATA)
			status = mb_extract_nav(verbose,mbio_ptr,store_ptr,&kind,
					time_i,&time_d,&navlon,&navlat,
					&speed,&heading,&draft,&roll,&pitch,&heave,&error);
		    }
		else
		    {
		    status = mb_get(verbose,mbio_ptr,&kind,&pings_read,
			time_i,&time_d,
			&navlon,&navlat,
			&speed,&heading,
			&distance,&altitude,&sonardepth,
			&beams_bath,&beams_amp,&pixels_ss,
			beamflag,bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
		    }
		    
		/* make sure non survey data records are ignored */
		if (error == MB_ERROR_NO_ERROR
			&& kind != MB_DATA_DATA)
			error = MB_ERROR_OTHER;

		/* time gaps are not a problem here */
		if (error == MB_ERROR_TIME_GAP)
			{
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}

		/* increment counter and set cumulative distance */
		if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_DATA)
			{
			nread++;
			distance_total += distance;
			}

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",kind);
			fprintf(stderr,"dbg2       error:          %d\n",error);
			fprintf(stderr,"dbg2       status:         %d\n",status);
			}

		/* print comments */
		if (verbose >= 1 && kind == MB_DATA_COMMENT)
			{
			if (icomment == 0)
				{
				fprintf(stderr,"\nComments:\n");
				icomment++;
				}
			fprintf(stderr,"%s\n",comment);
			}

		/* set output beams and pixels */
		if (error == MB_ERROR_NO_ERROR)
			{
			/* find vertical-most non-null beam */
			if (beams_bath > 0)
			    {
			    found = MB_NO;
			    beam_vertical = beams_bath / 2;			
			    for (i=0;i<beams_bath;i++)
				{
				if (beamflag[i] != MB_FLAG_NULL)
				    {
				    if (found == MB_NO)
					{
					distmin = fabs(bathacrosstrack[i]);
					beam_vertical = i;
					found = MB_YES;
					}
				    else if (fabs(bathacrosstrack[i]) < distmin)
					{
					distmin = fabs(bathacrosstrack[i]);
					beam_vertical = i;
					}
				    }
				}
			    }

			/* find vertical-most pixel */
			if (pixels_ss > 0)
			    {
			    found = MB_NO;
			    pixel_vertical = pixels_ss / 2;			
			    for (i=0;i<pixels_ss;i++)
				{
				if (ss[i] != 0.0)
				    {
				    if (found == MB_NO)
					{
					distmin = fabs(ssacrosstrack[i]);
					pixel_vertical = i;
					found = MB_YES;
					}
				    else if (fabs(ssacrosstrack[i]) < distmin)
					{
					distmin = fabs(ssacrosstrack[i]);
					pixel_vertical = i;
					}
				    }
				}
			    }

			/* set and/or check beams and pixels to be output */
			status = set_output(verbose,
				beams_bath,beams_amp,pixels_ss,
				use_bath,use_amp,use_ss,
				dump_mode,beam_set,pixel_set,
				beam_vertical, pixel_vertical, 
				&beam_start,&beam_end,
				&pixel_start,&pixel_end,
				&n_list,list,&error);

			if (status == MB_FAILURE)
				{
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}

			/* print debug statements */
			if (verbose >= 2)
				{
				fprintf(stderr,"\ndbg2  Beams set for output in <%s>\n",
					program_name);
				fprintf(stderr,"dbg2       status:       %d\n",
					status);
				fprintf(stderr,"dbg2       error:        %d\n",
					error);
				fprintf(stderr,"dbg2       use_bath:     %d\n",
					use_bath);
				fprintf(stderr,"dbg2       use_amp:      %d\n",
					use_amp);
				fprintf(stderr,"dbg2       use_ss:       %d\n",
					use_ss);
				fprintf(stderr,"dbg2       beam_start:   %d\n",
					beam_start);
				fprintf(stderr,"dbg2       beam_end:     %d\n",
					beam_end);
				fprintf(stderr,"dbg2       pixel_start:  %d\n",
					pixel_start);
				fprintf(stderr,"dbg2       pixel_end:    %d\n",
					pixel_end);
				fprintf(stderr,"dbg2       check_values: %d\n",
					check_values);
				fprintf(stderr,"dbg2       check_bath:   %d\n",
					check_bath);
				fprintf(stderr,"dbg2       check_amp:    %d\n",
					check_amp);
				fprintf(stderr,"dbg2       check_ss:     %d\n",
					check_ss);
				fprintf(stderr,"dbg2       n_list:       %d\n",
					n_list);
				for (i=0;i<n_list;i++)
					fprintf(stderr,"dbg2       list[%d]:      %c\n",
						i,list[i]);
				}
			}

		/* get factors for lon lat calculations */
		if (error == MB_ERROR_NO_ERROR)
			{
			mb_coor_scale(verbose,navlat,&mtodeglon,&mtodeglat);
			headingx = sin(DTR*heading);
			headingy = cos(DTR*heading);
			}
		
		/* get time interval since last ping */
		if (error == MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_DATA
			&& first == MB_YES)
			{
			time_interval = 0.0;
			}
		else if (error == MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_DATA)
			{
			time_interval = time_d - time_d_old;
			}

		/* calculate course made good */
		if (error == MB_ERROR_NO_ERROR 
			&& use_course == MB_YES)
			{
			if (first == MB_YES)
				{
				course = heading;
				speed_made_good = speed;
				course_old = heading;
				speed_made_good_old = speed;
				}
			else
				{
				dx = (navlon - navlon_old)/mtodeglon;
				dy = (navlat - navlat_old)/mtodeglat;
				dist = sqrt(dx*dx + dy*dy);
				if (dist > 0.0)
					course = RTD*atan2(dx/dist,dy/dist);
				else
					course = course_old;
				if (course < 0.0)
					course = course + 360.0;
				dt = (time_d - time_d_old);
				if (dt > 0.0)
					speed_made_good = 3.6*dist/dt;
				else
					speed_made_good 
						= speed_made_good_old;
				}
			}
			
		/* calculate slopes if required */
		if (error == MB_ERROR_NO_ERROR 
			&& use_slope == MB_YES)
			{
			/* get average slope */
			ns = 0;
			sx = 0.0;
			sy = 0.0;
			sxx = 0.0;
			sxy = 0.0;
			for (k=0;k<beams_bath;k++)
			  if (mb_beam_ok(beamflag[k]))
			    {
			    sx += bathacrosstrack[k];
			    sy += bath[k];
			    sxx += bathacrosstrack[k]
				*bathacrosstrack[k];
			    sxy += bathacrosstrack[k]*bath[k];
			    ns++;
			    }
			if (ns > 0)
			  {
			  delta = ns*sxx - sx*sx;
			  a = (sxx*sy - sx*sxy)/delta;
			  b = (ns*sxy - sx*sy)/delta;
			  avgslope = RTD * atan(b);
			  }
			else
			  avgslope = 0.0;
			  
			/* get per beam slope */
			set_bathyslope(verbose, 
				beams_bath,beamflag,bath,bathacrosstrack,
				&ndepths,depths,depthacrosstrack,
				&nslopes,slopes,slopeacrosstrack,
				&error);
			}

		/* reset old values */
		if (error == MB_ERROR_NO_ERROR)
			{
			navlon_old = navlon;
			navlat_old = navlat;
			course_old = course;
			speed_made_good_old = speed_made_good;
			time_d_old = time_d;
			}

		/* now loop over beams */
		if (error == MB_ERROR_NO_ERROR)
		for (j=beam_start;j<=beam_end;j++)
		  {
		  /* check beam status */
		  beam_status = MB_SUCCESS;
		  if (check_bath == MB_YES 
		    && check_values == MBLIST_CHECK_ON 
		    && !mb_beam_ok(beamflag[j]))
			beam_status = MB_FAILURE;
		  else if (check_bath == MB_YES 
		    && check_values == MBLIST_CHECK_ON_NULL 
		    && beamflag[j] == MB_FLAG_NULL)
			beam_status = MB_FAILURE;
		  if (check_amp == MB_YES 
		    && check_values == MBLIST_CHECK_ON 
		    && !mb_beam_ok(beamflag[j]))
			beam_status = MB_FAILURE;
		  else if (check_amp == MB_YES 
		    && check_values == MBLIST_CHECK_ON_NULL 
		    && beamflag[j] == MB_FLAG_NULL)
			beam_status = MB_FAILURE;
		  if (check_ss == MB_YES && j != beam_vertical)
			beam_status = MB_FAILURE;
		  else if (check_ss == MB_YES && j == beam_vertical)
			if (ss[pixel_vertical] <= 0)
				beam_status = MB_FAILURE;
		  if (use_time_interval == MB_YES && first == MB_YES)
			beam_status = MB_FAILURE;

		  /* print out good pings */
		  if (beam_status == MB_SUCCESS)
		    {
		    for (i=0; i<n_list; i++) 
			{
			switch (list[i]) 
				{
				case '/': /* Inverts next simple value */
					invert_next_value = MB_YES;
					break;
				case '-': /* Flip sign on next simple value */
					signflip_next_value = MB_YES;
					break;
				case 'A': /* Average seafloor crosstrack slope */
					printsimplevalue(verbose, avgslope, 0, 4, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'a': /* Per-beam seafloor crosstrack slope */
					if (beamflag[j] == MB_FLAG_NULL
					    && (check_values == MBLIST_CHECK_OFF_NAN
						|| check_values == MBLIST_CHECK_OFF_FLAGNAN))
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else if (!mb_beam_ok(beamflag[j])
					    && check_values == MBLIST_CHECK_OFF_FLAGNAN)
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else
					    {
					    status = get_bathyslope(verbose,
						ndepths,depths,depthacrosstrack,
						nslopes,slopes,slopeacrosstrack,
						bathacrosstrack[j],
						&depth,&slope,&error);
					    printsimplevalue(verbose, slope, 0, 4, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					break;
				case 'B': /* amplitude */
					if (beamflag[j] == MB_FLAG_NULL
					    && (check_values == MBLIST_CHECK_OFF_NAN
						|| check_values == MBLIST_CHECK_OFF_FLAGNAN))
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else if (!mb_beam_ok(beamflag[j])
					    && check_values == MBLIST_CHECK_OFF_FLAGNAN)
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else
					    {
					    printsimplevalue(verbose, amp[j], 0, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					break;
				case 'b': /* sidescan */
					printsimplevalue(verbose, ss[pixel_vertical], 0, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'C': /* Sonar altitude (m) */
					printsimplevalue(verbose, altitude, 0, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'c': /* Sonar transducer depth (m) */
					printsimplevalue(verbose, sonardepth, 0, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'D': /* acrosstrack dist. */
				case 'd':
					if (beamflag[j] == MB_FLAG_NULL
					    && (check_values == MBLIST_CHECK_OFF_NAN
						|| check_values == MBLIST_CHECK_OFF_FLAGNAN))
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else if (!mb_beam_ok(beamflag[j])
					    && check_values == MBLIST_CHECK_OFF_FLAGNAN)
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else
					    {
					    b = bathy_scale * bathacrosstrack[j];
					    printsimplevalue(verbose, b, 0, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					break;
				case 'E': /* alongtrack dist. */
				case 'e':
					if (beamflag[j] == MB_FLAG_NULL
					    && (check_values == MBLIST_CHECK_OFF_NAN
						|| check_values == MBLIST_CHECK_OFF_FLAGNAN))
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else if (!mb_beam_ok(beamflag[j])
					    && check_values == MBLIST_CHECK_OFF_FLAGNAN)
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else
					    {
					    b = bathy_scale * bathalongtrack[j];
					    printsimplevalue(verbose, b, 0, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					break;
				case 'G': /* flat bottom grazing angle */
					if (beamflag[j] == MB_FLAG_NULL
					    && (check_values == MBLIST_CHECK_OFF_NAN
						|| check_values == MBLIST_CHECK_OFF_FLAGNAN))
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else if (!mb_beam_ok(beamflag[j])
					    && check_values == MBLIST_CHECK_OFF_FLAGNAN)
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else
					    {
					    angle = RTD*(atan(bathacrosstrack[j] / bath[j]));
					    printsimplevalue(verbose, angle, 0, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					break;
				case 'g': /* grazing angle using slope */
					if (beamflag[j] == MB_FLAG_NULL
					    && (check_values == MBLIST_CHECK_OFF_NAN
						|| check_values == MBLIST_CHECK_OFF_FLAGNAN))
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else if (!mb_beam_ok(beamflag[j])
					    && check_values == MBLIST_CHECK_OFF_FLAGNAN)
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else
					    {
					    status = get_bathyslope(verbose,
						ndepths,depths,depthacrosstrack,
						nslopes,slopes,slopeacrosstrack,
						bathacrosstrack[j],
						&depth,&slope,&error);
					    angle = RTD * (atan(bathacrosstrack[j] / bath[j]))
						+ slope;
					    printsimplevalue(verbose, angle, 0, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					break;
				case 'H': /* heading */
					printsimplevalue(verbose, heading, 6, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'h': /* course */
					printsimplevalue(verbose, course, 6, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'J': /* time string */
					mb_get_jtime(verbose,time_i,time_j);
					if (ascii == MB_YES)
					    {
					    printf("%.4d %.3d %.2d %.2d %.2d.%6.6d",
						time_j[0],time_j[1],
						time_i[3],time_i[4],
						time_i[5],time_i[6]);
					    }
					else
					    {
					    b = time_j[0];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_j[1];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[3];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[4];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[5];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[6];
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'j': /* time string */
					mb_get_jtime(verbose,time_i,time_j);
					if (ascii == MB_YES)
					    {
					    printf("%.4d %.3d %.4d %.2d.%6.6d",
						time_j[0],time_j[1],
						time_j[2],time_j[3],time_j[4]);
					    }
					else
					    {
					    b = time_j[0];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_j[1];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_j[2];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_j[3];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_j[4];
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'L': /* along-track distance (km) */
					printsimplevalue(verbose, distance_total, 7, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'l': /* along-track distance (m) */
					printsimplevalue(verbose, 1000.0 * distance_total, 7, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'M': /* Decimal unix seconds since 
						1/1/70 00:00:00 */
					printsimplevalue(verbose, time_d, 0, 6, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'm': /* time in decimal seconds since 
						first record */
					if (first_m == MB_YES)
						{
						time_d_ref = time_d;
						first_m = MB_NO;
						}
					b = time_d - time_d_ref;
					printsimplevalue(verbose, b, 0, 6, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'N': /* ping counter */
					if (ascii == MB_YES)
					    printf("%6d",nread);
					else
					    {
					    b = nread;
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'P': /* pitch */
					printsimplevalue(verbose, pitch, 5, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'p': /* draft */
					printsimplevalue(verbose, draft, 5, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'R': /* roll */
					printsimplevalue(verbose, roll, 5, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'r': /* heave */
					printsimplevalue(verbose, heave, 5, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'S': /* speed */
					printsimplevalue(verbose, speed, 5, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 's': /* speed made good */
					printsimplevalue(verbose, speed_made_good, 5, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
					if (ascii == MB_YES)
					    printf("%.4d/%.2d/%.2d/%.2d/%.2d/%.2d.%.6d",
						time_i[0],time_i[1],time_i[2],
						time_i[3],time_i[4],time_i[5],
						time_i[6]);
					else
					    {
					    b = time_i[0];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[1];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[2];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[3];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[4];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[5] + 1e-6 * time_i[6];
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 't': /* yyyy mm dd hh mm ss time string */
					if (ascii == MB_YES)
					    printf("%.4d %.2d %.2d %.2d %.2d %.2d.%.6d",
						time_i[0],time_i[1],time_i[2],
						time_i[3],time_i[4],time_i[5],
						time_i[6]);
					else
					    {
					    b = time_i[0];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[1];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[2];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[3];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[4];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[5] + 1e-6 * time_i[6];
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
					time_u = (int) time_d;
					if (ascii == MB_YES)
					    printf("%d",time_u);
					else
					    {
					    b = time_u;
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'u': /* time in seconds since first record */
					time_u = (int) time_d;
					if (first_u == MB_YES)
						{
						time_u_ref = time_u;
						first_u = MB_NO;
						}
					if (ascii == MB_YES)
					    printf("%d",time_u - time_u_ref);
					else
					    {
					    b = time_u - time_u_ref;
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'V': /* time in seconds since last ping */
				case 'v': 
					if (ascii == MB_YES)
					    {
					    if ( fabs(time_interval) > 100. )
						printf("%g",time_interval); 
					    else
						printf("%7.3f",time_interval);
					    }
					else
					    {
					    fwrite(&time_interval, sizeof(double), 1, stdout);
					    }
					break;
				case 'X': /* longitude decimal degrees */
					dlon = navlon;
					if (beam_set != MBLIST_SET_OFF)
					    dlon += headingy*mtodeglon
							*bathacrosstrack[j]
						    + headingx*mtodeglon
							*bathalongtrack[j];
					printsimplevalue(verbose, dlon, 11, 6, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'x': /* longitude degress + decimal minutes */
					dlon = navlon;
					if (beam_set != MBLIST_SET_OFF)
					    dlon += headingy*mtodeglon
							*bathacrosstrack[j]
						    + headingx*mtodeglon
							*bathalongtrack[j];
					if (dlon < 0.0)
						{
						hemi = 'W';
						dlon = -dlon;
						}
					else
						hemi = 'E';
					degrees = (int) dlon;
					minutes = 60.0*(dlon - degrees);
					if (ascii == MB_YES)
					    {
					    printf("%3d %8.5f%c",
						degrees, minutes, hemi);
					    }
					else
					    {
					    b = degrees;
					    if (hemi == 'W') b = -b;
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = minutes;
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'Y': /* latitude decimal degrees */
					dlat = navlat;
					if (beam_set != MBLIST_SET_OFF)
					    dlat += -headingx*mtodeglat
							*bathacrosstrack[j]
						    + headingy*mtodeglat
							*bathalongtrack[j];
					printsimplevalue(verbose, dlat, 11, 6, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'y': /* latitude degrees + decimal minutes */
					dlat = navlat;
					if (beam_set != MBLIST_SET_OFF)
					    dlat += -headingx*mtodeglat
							*bathacrosstrack[j]
						    + headingy*mtodeglat
							*bathalongtrack[j];
					if (dlat < 0.0)
						{
						hemi = 'S';
						dlat = -dlat;
						}
					else
						hemi = 'N';
					degrees = (int) dlat;
					minutes = 60.0*(dlat - degrees);
					if (ascii == MB_YES)
					    {
					    printf("%3d %8.5f%c",
						degrees, minutes, hemi);
					    }
					else
					    {
					    b = degrees;
					    if (hemi == 'S') b = -b;
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = minutes;
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'Z': /* topography */
					if (beamflag[j] == MB_FLAG_NULL
					    && (check_values == MBLIST_CHECK_OFF_NAN
						|| check_values == MBLIST_CHECK_OFF_FLAGNAN))
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else if (!mb_beam_ok(beamflag[j])
					    && check_values == MBLIST_CHECK_OFF_FLAGNAN)
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else
					    {
					    b = -bathy_scale * bath[j];
					    printsimplevalue(verbose, b, 0, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					break;
				case 'z': /* depth */
					if (beamflag[j] == MB_FLAG_NULL
					    && (check_values == MBLIST_CHECK_OFF_NAN
						|| check_values == MBLIST_CHECK_OFF_FLAGNAN))
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else if (!mb_beam_ok(beamflag[j])
					    && check_values == MBLIST_CHECK_OFF_FLAGNAN)
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else
					    {
					    b = bathy_scale * bath[j];
					    printsimplevalue(verbose, b, 0, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					break;
				case '#': /* beam number */
					if (ascii == MB_YES)
					    printf("%6d",j);
					else
					    {
					    b = j;
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				default:
					if (ascii == MB_YES)
					    printf("<Invalid Option: %c>",
						list[i]);
					break;
				}
			if (ascii == MB_YES)
			    {
			    if (i<(n_list-1)) printf ("\t");
			    else printf ("\n");
			    }
			}
		    }
		  }

		/* now loop over pixels */
		if (error == MB_ERROR_NO_ERROR)
		for (j=pixel_start;j<=pixel_end;j++)
		  {
		  /* check pixel status */
		  pixel_status = MB_SUCCESS;
		  if (check_bath == MB_YES && j != pixel_vertical)
			pixel_status = MB_FAILURE;
		  else if (check_bath == MB_YES && j == pixel_vertical)
			{
			if (check_values == MBLIST_CHECK_ON
			    && !mb_beam_ok(beamflag[beam_vertical]))
				pixel_status = MB_FAILURE;
			else if (check_values == MBLIST_CHECK_ON_NULL
			    && beamflag[beam_vertical] == MB_FLAG_NULL)
				pixel_status = MB_FAILURE;
			}
		  if (check_amp == MB_YES && j != pixel_vertical)
			pixel_status = MB_FAILURE;
		  else if (check_amp == MB_YES && j == pixel_vertical)
			{
			if (check_values == MBLIST_CHECK_ON
			    && !mb_beam_ok(beamflag[beam_vertical]))
				pixel_status = MB_FAILURE;
			else if (check_values == MBLIST_CHECK_ON_NULL
			    && beamflag[beam_vertical] == MB_FLAG_NULL)
				pixel_status = MB_FAILURE;
			}
		  if (check_ss == MB_YES && ss[j] <= 0)
			pixel_status = MB_FAILURE;
		  if (use_time_interval == MB_YES && first == MB_YES)
			pixel_status = MB_FAILURE;

		  /* print out good pings */
		  if (pixel_status == MB_SUCCESS)
		    {
		    for (i=0; i<n_list; i++) 
			{
			switch (list[i]) 
				{
				case '/': /* Inverts next simple value */
					invert_next_value = MB_YES;
					break;
				case '-': /* Flip sign on next simple value */
					signflip_next_value = MB_YES;
					break;
				case 'A': /* Average seafloor crosstrack slope */
					printsimplevalue(verbose, avgslope, 0, 4, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'a': /* Per-pixel seafloor crosstrack slope */
					status = get_bathyslope(verbose,
						ndepths,depths,depthacrosstrack,
						nslopes,slopes,slopeacrosstrack,
						ssacrosstrack[j],
						&depth,&slope,&error);
					printsimplevalue(verbose, slope, 0, 4, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'B': /* amplitude */
					printsimplevalue(verbose, amp[beam_vertical], 0, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'b': /* sidescan */
					printsimplevalue(verbose, ss[j], 0, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'C': /* Sonar altitude (m) */
					printsimplevalue(verbose, altitude, 0, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'c': /* Sonar transducer depth (m) */
					printsimplevalue(verbose, sonardepth, 0, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'D': /* acrosstrack dist. */
				case 'd':
					b = bathy_scale * ssacrosstrack[j];
					printsimplevalue(verbose, b, 0, 3, ascii, 
							&invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'E': /* alongtrack dist. */
				case 'e':
					b = bathy_scale * ssalongtrack[j];
					printsimplevalue(verbose, b, 0, 3, ascii, 
							&invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'G': /* flat bottom grazing angle */
					status = get_bathyslope(verbose,
					    ndepths,depths,depthacrosstrack,
					    nslopes,slopes,slopeacrosstrack,
					    ssacrosstrack[j],
					    &depth,&slope,&error);
					angle = RTD*(atan(ssacrosstrack[j] / depth));
					printsimplevalue(verbose, angle, 0, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'g': /* grazing angle using slope */
					status = get_bathyslope(verbose,
					    ndepths,depths,depthacrosstrack,
					    nslopes,slopes,slopeacrosstrack,
					    ssacrosstrack[j],
					    &depth,&slope,&error);
					angle = RTD * (atan(bathacrosstrack[j] / depth))
					    + slope;
					printsimplevalue(verbose, angle, 0, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'H': /* heading */
					printsimplevalue(verbose, heading, 6, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'h': /* course */
					printsimplevalue(verbose, course, 6, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'J': /* time string */
					mb_get_jtime(verbose,time_i,time_j);
					if (ascii == MB_YES)
					    {
					    printf("%.4d %.3d %.2d %.2d %.2d.%6.6d",
						time_j[0],time_j[1],
						time_i[3],time_i[4],
						time_i[5],time_i[6]);
					    }
					else
					    {
					    b = time_j[0];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_j[1];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[3];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[4];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[5];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[6];
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'j': /* time string */
					mb_get_jtime(verbose,time_i,time_j);
					if (ascii == MB_YES)
					    {
					    printf("%.4d %.3d %.4d %.2d.%6.6d",
						time_j[0],time_j[1],
						time_j[2],time_j[3],time_j[4]);
					    }
					else
					    {
					    b = time_j[0];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_j[1];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_j[2];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_j[3];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_j[4];
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'L': /* along-track distance (km) */
					printsimplevalue(verbose, distance_total, 7, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'l': /* along-track distance (m) */
					printsimplevalue(verbose, 1000.0 * distance_total, 7, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'M': /* Decimal unix seconds since 
						1/1/70 00:00:00 */
					printsimplevalue(verbose, time_d, 0, 6, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'm': /* time in decimal seconds since 
						first record */
					if (first_m == MB_YES)
						{
						time_d_ref = time_d;
						first_m = MB_NO;
						}
					b = time_d - time_d_ref;
					printsimplevalue(verbose, b, 0, 6, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'N': /* ping counter */
					if (ascii == MB_YES)
					    printf("%6d",nread);
					else
					    {
					    b = nread;
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'P': /* pitch */
					printsimplevalue(verbose, pitch, 5, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'p': /* draft */
					printsimplevalue(verbose, draft, 5, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'R': /* roll */
					printsimplevalue(verbose, roll, 5, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'r': /* heave */
					printsimplevalue(verbose, heave, 5, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'S': /* speed */
					printsimplevalue(verbose, speed, 5, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 's': /* speed made good */
					printsimplevalue(verbose, speed_made_good, 5, 2, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
					if (ascii == MB_YES)
					    printf("%.4d/%.2d/%.2d/%.2d/%.2d/%.2d.%.6d",
						time_i[0],time_i[1],time_i[2],
						time_i[3],time_i[4],time_i[5],
						time_i[6]);
					else
					    {
					    b = time_i[0];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[1];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[2];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[3];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[4];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[5] + 1e-6 * time_i[6];
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 't': /* yyyy mm dd hh mm ss time string */
					if (ascii == MB_YES)
					    printf("%.4d %.2d %.2d %.2d %.2d %.2d.%.6d",
						time_i[0],time_i[1],time_i[2],
						time_i[3],time_i[4],time_i[5],
						time_i[6]);
					else
					    {
					    b = time_i[0];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[1];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[2];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[3];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[4];
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = time_i[5] + 1e-6 * time_i[6];
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
					time_u = (int) time_d;
					if (ascii == MB_YES)
					    printf("%d",time_u);
					else
					    {
					    b = time_u;
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'u': /* time in seconds since first record */
					time_u = (int) time_d;
					if (first_u == MB_YES)
						{
						time_u_ref = time_u;
						first_u = MB_NO;
						}
					if (ascii == MB_YES)
					    printf("%d",time_u - time_u_ref);
					else
					    {
					    b = time_u - time_u_ref;
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'V': /* time in seconds since last ping */
				case 'v': 
					if (ascii == MB_YES)
					    {
					    if ( fabs(time_interval) > 100. )
						printf("%g",time_interval); 
					    else
						printf("%7.3f",time_interval);
					    }
					else
					    {
					    fwrite(&time_interval, sizeof(double), 1, stdout);
					    }
					break;
				case 'X': /* longitude decimal degrees */
					dlon = navlon;
					if (pixel_set != MBLIST_SET_OFF)
					    dlon += headingy*mtodeglon
							*ssacrosstrack[j]
						    + headingx*mtodeglon
							*ssalongtrack[j];
					printsimplevalue(verbose, dlon, 11, 6, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'x': /* longitude degress + decimal minutes */
					dlon = navlon;
					if (pixel_set != MBLIST_SET_OFF)
					    dlon += headingy*mtodeglon
							*ssacrosstrack[j]
						    + headingx*mtodeglon
							*ssalongtrack[j];
					if (dlon < 0.0)
						{
						hemi = 'W';
						dlon = -dlon;
						}
					else
						hemi = 'E';
					degrees = (int) dlon;
					minutes = 60.0*(dlon - degrees);
					if (ascii == MB_YES)
					    {
					    printf("%3d %8.5f%c",
						degrees, minutes, hemi);
					    }
					else
					    {
					    b = degrees;
					    if (hemi == 'W') b = -b;
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = minutes;
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'Y': /* latitude decimal degrees */
					dlat = navlat;
					if (pixel_set != MBLIST_SET_OFF)
					    dlat += -headingx*mtodeglat
							*ssacrosstrack[j]
						    + headingy*mtodeglat
							*ssalongtrack[j];
					printsimplevalue(verbose, dlat, 11, 6, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					break;
				case 'y': /* latitude degrees + decimal minutes */
					dlat = navlat;
					if (pixel_set != MBLIST_SET_OFF)
					    dlat += -headingx*mtodeglat
							*ssacrosstrack[j]
						    + headingy*mtodeglat
							*ssalongtrack[j];
					if (dlat < 0.0)
						{
						hemi = 'S';
						dlat = -dlat;
						}
					else
						hemi = 'N';
					degrees = (int) dlat;
					minutes = 60.0*(dlat - degrees);
					if (ascii == MB_YES)
					    {
					    printf("%3d %8.5f%c",
						degrees, minutes, hemi);
					    }
					else
					    {
					    b = degrees;
					    if (hemi == 'S') b = -b;
					    fwrite(&b, sizeof(double), 1, stdout);
					    b = minutes;
					    fwrite(&b, sizeof(double), 1, stdout);
					    }
					break;
				case 'Z': /* topography */
					if (beamflag[beam_vertical] == MB_FLAG_NULL
					    && (check_values == MBLIST_CHECK_OFF_NAN
						|| check_values == MBLIST_CHECK_OFF_FLAGNAN))
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else if (!mb_beam_ok(beamflag[beam_vertical])
					    && check_values == MBLIST_CHECK_OFF_FLAGNAN)
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else
					    {
					    b = -bathy_scale * bath[beam_vertical];
					    printsimplevalue(verbose, b, 0, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					break;
				case 'z': /* depth */
					if (beamflag[beam_vertical] == MB_FLAG_NULL
					    && (check_values == MBLIST_CHECK_OFF_NAN
						|| check_values == MBLIST_CHECK_OFF_FLAGNAN))
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else if (!mb_beam_ok(beamflag[beam_vertical])
					    && check_values == MBLIST_CHECK_OFF_FLAGNAN)
					    {
					    printNaN(verbose, ascii, &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					else
					    {
					    b = bathy_scale * bath[beam_vertical];
					    printsimplevalue(verbose, b, 0, 3, ascii, 
							    &invert_next_value, 
							    &signflip_next_value, &error);
					    }
					break;
				case '#': /* pixel number */
					printf("%6d",j);
					break;
				default:
					printf("<Invalid Option: %c>",
						list[i]);
					break;
				}
				if (i<(n_list-1)) printf ("\t");
				else printf ("\n");
			}
		    }
		  }

		/* reset first flag */
		if (error == MB_ERROR_NO_ERROR && first == MB_YES)
			{
			first = MB_NO;
			}

		}

	/* close the swath file */
	status = mb_close(verbose,&mbio_ptr,&error);

	/* deallocate memory used for data arrays */
	mb_free(verbose,&beamflag,&error); 
	mb_free(verbose,&bath,&error); 
	mb_free(verbose,&bathacrosstrack,&error); 
	mb_free(verbose,&bathalongtrack,&error); 
	mb_free(verbose,&amp,&error); 
	mb_free(verbose,&ss,&error); 
	mb_free(verbose,&ssacrosstrack,&error); 
	mb_free(verbose,&ssalongtrack,&error); 
	mb_free(verbose,&depths,&error);
	mb_free(verbose,&depthacrosstrack,&error);
	mb_free(verbose,&slopes,&error);
	mb_free(verbose,&slopeacrosstrack,&error);

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if (status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error)
			    == MB_SUCCESS)
                        read_data = MB_YES;
                else
                        read_data = MB_NO;
                }
        else
                {
                read_data = MB_NO;
                }

	/* end loop over files in list */
	}
	if (read_datalist == MB_YES)
		mb_datalist_close(verbose,&datalist,&error);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(stderr,"dbg2  Ending status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
int set_output(	int	verbose, 
		int	beams_bath, 
		int	beams_amp, 
		int	pixels_ss, 
		int	use_bath, 
		int	use_amp, 
		int	use_ss, 
		int	dump_mode, 
		int	beam_set, 
		int	pixel_set, 
		int	beam_vertical, 
		int	pixel_vertical, 
		int	*beam_start, 
		int	*beam_end, 
		int	*pixel_start, 
		int	*pixel_end, 
		int	*n_list, 
		char	*list, 
		int	*error)
{
	char	*function_name = "set_output";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBLIST function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       beams_bath:      %d\n",beams_bath);
		fprintf(stderr,"dbg2       beams_amp:       %d\n",beams_amp);
		fprintf(stderr,"dbg2       pixels_ss:       %d\n",pixels_ss);
		fprintf(stderr,"dbg2       use_bath:        %d\n",use_bath);
		fprintf(stderr,"dbg2       use_amp:         %d\n",use_amp);
		fprintf(stderr,"dbg2       use_ss:          %d\n",use_ss);
		fprintf(stderr,"dbg2       dump_mode:       %d\n",dump_mode);
		fprintf(stderr,"dbg2       beam_set:        %d\n",beam_set);
		fprintf(stderr,"dbg2       pixel_set:       %d\n",pixel_set);
		fprintf(stderr,"dbg2       beam_vertical:   %d\n",beam_vertical);
		fprintf(stderr,"dbg2       pixel_vertical:  %d\n",pixel_vertical);
		fprintf(stderr,"dbg2       beam_start:      %d\n",*beam_start);
		fprintf(stderr,"dbg2       beam_end:        %d\n",*beam_end);
		fprintf(stderr,"dbg2       pixel_start:     %d\n",*pixel_start);
		fprintf(stderr,"dbg2       pixel_end:       %d\n",*pixel_end);
		fprintf(stderr,"dbg2       n_list:          %d\n",*n_list);
		for (i=0;i<*n_list;i++)
		    fprintf(stderr,"dbg2       list[%2d]:        %c\n",
						i,list[i]);
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	if (beam_set == MBLIST_SET_OFF && pixel_set != MBLIST_SET_OFF)
		{
		*beam_start = 0;
		*beam_end = -1;
		}
	else if (beam_set == MBLIST_SET_OFF && beams_bath <= 0)
		{
		*beam_start = 0;
		*beam_end = -1;
		*pixel_start = pixel_vertical;
		*pixel_end = pixel_vertical;
		}
	else if (beam_set == MBLIST_SET_OFF)
		{
		*beam_start = beam_vertical;
		*beam_end = beam_vertical;			
		}
	else if (beam_set == MBLIST_SET_ALL)
		{
		*beam_start = 0;
		*beam_end = beams_bath - 1;			
		}
	if (pixel_set == MBLIST_SET_OFF && beams_bath > 0)
		{
		*pixel_start = 0;
		*pixel_end = -1;
		}
	else if (pixel_set == MBLIST_SET_ALL)
		{
		*pixel_start = 0;
		*pixel_end = pixels_ss - 1;
		}

	/* deal with dump_mode if set */
	if (dump_mode == DUMP_MODE_BATH)
		{
		*beam_start = 0;
		*beam_end = beams_bath - 1;
		*pixel_start = 0;
		*pixel_end = -1;
		strcpy(list,"XYz");
		*n_list = 3;
		}
	else if (dump_mode == DUMP_MODE_TOPO)
		{
		*beam_start = 0;
		*beam_end = beams_bath - 1;
		*pixel_start = 0;
		*pixel_end = -1;
		strcpy(list,"XYZ");
		*n_list = 3;
		}
	else if (dump_mode == DUMP_MODE_AMP)
		{
		*beam_start = 0;
		*beam_end = beams_bath - 1;
		*pixel_start = 0;
		*pixel_end = -1;
		strcpy(list,"XYB");
		*n_list = 3;
		}
	else if (dump_mode == DUMP_MODE_SS)
		{
		*beam_start = 0;
		*beam_end = -1;
		*pixel_start = 0;
		*pixel_end = pixels_ss - 1;
		strcpy(list,"XYb");
		*n_list = 3;
		}

	/* check if beam and pixel range is ok */
	if ((use_bath == MB_YES || *beam_end >= *beam_start)
		&& beams_bath <= 0)
		{
		fprintf(stderr,"\nBathymetry data not available\n");
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
		}
	else if (*beam_end >= *beam_start && 
		(*beam_start < 0 || *beam_end >= beams_bath))
		{
		fprintf(stderr,"\nBeam range %d to %d exceeds available beams 0 to %d\n",
			*beam_start,*beam_end,beams_bath-1);
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
		}
	if (*error == MB_ERROR_NO_ERROR 
		&& use_amp == MB_YES && beams_amp <= 0)
		{
		fprintf(stderr,"\nAmplitude data not available\n");
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
		}
	else if (*error == MB_ERROR_NO_ERROR 
		&& *beam_end >= *beam_start && use_amp == MB_YES &&
		(*beam_start < 0 || *beam_end >= beams_amp))
		{
		fprintf(stderr,"\nAmplitude beam range %d to %d exceeds available beams 0 to %d\n",
			*beam_start,*beam_end,beams_amp-1);
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
		}
	if (*error == MB_ERROR_NO_ERROR 
		&& (use_ss == MB_YES || *pixel_end >= *pixel_start)
		&& pixels_ss <= 0)
		{
		fprintf(stderr,"\nSidescan data not available\n");
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
		}
	else if (*error == MB_ERROR_NO_ERROR 
		&& *pixel_end >= *pixel_start && 
		(*pixel_start < 0 || *pixel_end >= pixels_ss))
		{
		fprintf(stderr,"\nPixels range %d to %d exceeds available pixels 0 to %d\n",
			*pixel_start,*pixel_end,pixels_ss-1);
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBCOPY function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       beam_start:    %d\n",*beam_start);
		fprintf(stderr,"dbg2       beam_end:      %d\n",*beam_end);
		fprintf(stderr,"dbg2       pixel_start:   %d\n",*pixel_start);
		fprintf(stderr,"dbg2       pixel_end:     %d\n",*pixel_end);
		fprintf(stderr,"dbg2       n_list:        %d\n",*n_list);
		for (i=0;i<*n_list;i++)
			fprintf(stderr,"dbg2       list[%2d]:      %c\n",
						i,list[i]);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int set_bathyslope(int verbose,
	int nbath, char *beamflag, double *bath, double *bathacrosstrack,
	int *ndepths, double *depths, double *depthacrosstrack, 
	int *nslopes, double *slopes, double *slopeacrosstrack, 
	int *error)
{
	char	*function_name = "set_bathyslope";
	int	status = MB_SUCCESS;
	int	i;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       nbath:           %d\n",nbath);
		fprintf(stderr,"dbg2       bath:            %d\n",bath);
		fprintf(stderr,"dbg2       bathacrosstrack: %d\n",
			bathacrosstrack);
		fprintf(stderr,"dbg2       bath:\n");
		for (i=0;i<nbath;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, bath[i], bathacrosstrack[i]);
		}

	/* first find all depths */
	*ndepths = 0;
	for (i=0;i<nbath;i++)
		{
		if (mb_beam_ok(beamflag[i]))
			{
			depths[*ndepths] = bath[i];
			depthacrosstrack[*ndepths] = bathacrosstrack[i];
			(*ndepths)++;
			}
		}

	/* now calculate slopes */
	*nslopes = *ndepths + 1;
	for (i=0;i<*ndepths-1;i++)
		{
		slopes[i+1] = (depths[i+1] - depths[i])
			/(depthacrosstrack[i+1] - depthacrosstrack[i]);
		slopeacrosstrack[i+1] = 0.5*(depthacrosstrack[i+1] 
			+ depthacrosstrack[i]);
		}
	if (*ndepths > 1)
		{
		slopes[0] = 0.0;
		slopeacrosstrack[0] = depthacrosstrack[0];
		slopes[*ndepths] = 0.0;
		slopeacrosstrack[*ndepths] = 
			depthacrosstrack[*ndepths-1];
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndepths:         %d\n",
			*ndepths);
		fprintf(stderr,"dbg2       depths:\n");
		for (i=0;i<*ndepths;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, depths[i], depthacrosstrack[i]);
		fprintf(stderr,"dbg2       nslopes:         %d\n",
			*nslopes);
		fprintf(stderr,"dbg2       slopes:\n");
		for (i=0;i<*nslopes;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int get_bathyslope(int verbose,
	int ndepths, double *depths, double *depthacrosstrack, 
	int nslopes, double *slopes, double *slopeacrosstrack, 
	double acrosstrack, double *depth,  double *slope, 
	int *error)
{
	char	*function_name = "get_bathyslope";
	int	status = MB_SUCCESS;
	int	found_depth, found_slope;
	int	idepth, islope;
	int	i;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       ndepths:         %d\n",
			ndepths);
		fprintf(stderr,"dbg2       depths:\n");
		for (i=0;i<ndepths;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, depths[i], depthacrosstrack[i]);
		fprintf(stderr,"dbg2       nslopes:         %d\n",
			nslopes);
		fprintf(stderr,"dbg2       slopes:\n");
		for (i=0;i<nslopes;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr,"dbg2       acrosstrack:     %f\n",acrosstrack);
		}

	/* check if acrosstrack is in defined interval */
	found_depth = MB_NO;
	found_slope = MB_NO;
	if (ndepths > 1)
	if (acrosstrack >= depthacrosstrack[0]
		&& acrosstrack <= depthacrosstrack[ndepths-1])
	    {

	    /* look for depth */
	    idepth = -1;
	    while (found_depth == MB_NO && idepth < ndepths - 2)
		{
		idepth++;
		if (acrosstrack >= depthacrosstrack[idepth]
		    && acrosstrack <= depthacrosstrack[idepth+1])
		    {
		    *depth = depths[idepth] 
			    + (acrosstrack - depthacrosstrack[idepth])
			    /(depthacrosstrack[idepth+1] 
			    - depthacrosstrack[idepth])
			    *(depths[idepth+1] - depths[idepth]);
		    found_depth = MB_YES;
		    *error = MB_ERROR_NO_ERROR;
		    }
		}

	    /* look for slope */
	    islope = -1;
	    while (found_slope == MB_NO && islope < nslopes - 2)
		{
		islope++;
		if (acrosstrack >= slopeacrosstrack[islope]
		    && acrosstrack <= slopeacrosstrack[islope+1])
		    {
		    *slope = slopes[islope] 
			    + (acrosstrack - slopeacrosstrack[islope])
			    /(slopeacrosstrack[islope+1] 
			    - slopeacrosstrack[islope])
			    *(slopes[islope+1] - slopes[islope]);
		    found_slope = MB_YES;
		    *error = MB_ERROR_NO_ERROR;
		    }
		}
	    }

	/* translate slope to degrees */
	if (found_slope == MB_YES)
	    *slope = RTD * atan(*slope);

	/* check for failure */
	if (found_depth != MB_YES || found_slope != MB_YES)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_OTHER;
		*depth = 0.0;
		*slope = 0.0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       depth:           %f\n",*depth);
		fprintf(stderr,"dbg2       slope:           %f\n",*slope);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int printsimplevalue(int verbose, 
	double value, int width, int precision, 
	int ascii, int *invert, int *flipsign, int *error)
{
	char	*function_name = "printsimplevalue";
	int	status = MB_SUCCESS;
	char	format[24];
	int	i;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       value:           %f\n",value);
		fprintf(stderr,"dbg2       width:           %d\n",width);
		fprintf(stderr,"dbg2       precision:       %d\n",precision);
		fprintf(stderr,"dbg2       ascii:           %d\n",ascii);
		fprintf(stderr,"dbg2       invert:          %d\n",*invert);
		fprintf(stderr,"dbg2       flipsign:        %d\n",*flipsign);
		}
		
	/* make print format */
	format[0] = '%';
	if (*invert == MB_YES)
	    strcpy(format, "%g");
	else if (width > 0)
	    sprintf(&format[1], "%d.%df", width, precision);
	else
	    sprintf(&format[1], ".%df", precision);
	
	/* invert value if desired */
	if (*invert == MB_YES)
	    {
	    *invert = MB_NO;
	    if (value != 0.0)
		value = 1.0 / value;
	    }
	
	/* flip sign value if desired */
	if (*flipsign == MB_YES)
	    {
	    *flipsign = MB_NO;
	    value = -value;
	    }
	    
	/* print value */
	if (ascii == MB_YES)
	    printf(format, value);
	else
	    fwrite(&value, sizeof(double), 1, stdout);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       invert:          %d\n",*invert);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int printNaN(int verbose, int ascii, int *invert, int *flipsign, int *error)
{
	char	*function_name = "printNaN";
	int	status = MB_SUCCESS;
	int	i;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       ascii:           %d\n",ascii);
		fprintf(stderr,"dbg2       invert:          %d\n",*invert);
		fprintf(stderr,"dbg2       flipsign:        %d\n",*flipsign);
		}
		
	/* reset invert flag */
	if (*invert == MB_YES)
	    *invert = MB_NO;
		
	/* reset flipsign flag */
	if (*flipsign == MB_YES)
	    *flipsign = MB_NO;
	    
	/* print value */
	if (ascii == MB_YES)
	    printf("NaN");
	else
	    fwrite(&NaN, sizeof(double), 1, stdout);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       invert:          %d\n",*invert);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
