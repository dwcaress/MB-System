/*--------------------------------------------------------------------
 *    The MB-system:	mbclean.c	2/26/93
 *    $Id: mbclean.c,v 4.18 1998-10-05 19:19:24 caress Exp $
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
 * MBCLEAN identifies and flags artifacts in swath sonar bathymetry data
 * Bad beams  are  indentified  based  on  one simple criterion only: 
 * excessive bathymetric slopes.   The default input and output streams 
 * are stdin and stdout.
 *
 * Author:	D. W. Caress
 * Date:	February 26, 1993
 *
 * Acknowledgments:
 * This program is based to a large extent on the program mbcleanx
 * by Alberto Malinverno (formerly at L-DEO, now at Schlumberger),
 * which was in turn based on the original program mbclean (v. 1.0)
 * by David Caress.
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.17  1997/10/03  18:44:36  caress
 * Fixed problem with sort call.
 *
 * Revision 4.16  1997/09/15  19:11:06  caress
 * Real Version 4.5
 *
 * Revision 4.15  1997/07/25  14:28:10  caress
 * Version 4.5beta2
 *
 * Revision 4.14  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.14  1997/04/17  15:14:38  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.13  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.12  1996/03/01  22:37:24  caress
 * Added -U option to flag half-pings containing less than
 * a specified number of good bathymetry values.
 *
 * Revision 4.11  1996/01/26  21:25:58  caress
 * Version 4.3 distribution
 *
 * Revision 4.10  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.9  1995/03/13  19:22:12  caress
 * Added fractional depth range checking (-G option).
 *
 * Revision 4.8  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.7  1995/03/02  13:49:21  caress
 * Fixed bug related to error messages.
 *
 * Revision 4.6  1994/12/02  16:02:51  caress
 * Fixed (?) bug where mbclean went into infinite loop
 * with MR1 (61) data.
 *
 * Revision 4.5  1994/10/21  13:02:31  caress
 * Release V4.0
 *
 * Revision 4.4  1994/07/29  19:02:56  caress
 * Changes associated with supporting byte swapped Lynx OS and
 * using unix second time base.
 *
 * Revision 4.3  1994/04/12  00:42:00  caress
 * Changed call to mb_buffer_close in accordance with change
 * in mb_buffer source code.  The parameter list now includes
 * mbio_ptr.
 *
 * Revision 4.2  1994/03/25  14:01:31  caress
 * Added ability to check that depth values are within a specified
 * acceptable range.
 *
 * Revision 4.1  1994/03/12  01:44:37  caress
 * Added declarations of ctime and/or getenv for compatability
 * with SGI compilers.
 *
 * Revision 4.0  1994/03/06  00:13:22  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 3.2  1993/08/26  12:46:36  caress
 * Added checking for "rails" at Dan Bissell's suggestion.
 *
 * Revision 3.1  1993/05/18  00:01:15  caress
 * Changed buffer size (nwant) to 500 and buffer holdover (nhold) to 50.
 *
 * Revision 3.0  1993/05/04  22:21:32  dale
 * Initial version.
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_io.h"

/* local defines */
#define	MBCLEAN_FLAG_ONE	1
#define	MBCLEAN_FLAG_BOTH	2
#define	MBCLEAN_ZERO_ONE	3
#define	MBCLEAN_ZERO_BOTH	4

/* MBIO buffer size default */
#define	MBCLEAN_BUFFER_DEFAULT	500

/* ping structure definition */
struct mbclean_ping_struct 
	{
	int	id;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	char	*beamflag;
	double	*bath;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*amp;
	double	*ss;
	double	*ssacrosstrack;
	double	*ssalongtrack;
	double	*bathx;
	double	*bathy;
	};

/* bad beam identifier structure definition */
struct bad_struct
	{
	int	flag;
	int	ping;
	int	beam;
	double	bath;
	};

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mbclean.c,v 4.18 1998-10-05 19:19:24 caress Exp $";
	static char program_name[] = "MBCLEAN";
	static char help_message[] =  "MBCLEAN identifies and flags artifacts in swath sonar bathymetry data\nBad beams  are  indentified  based  on  one simple criterion only: \nexcessive bathymetric slopes.   The default input and output streams \nare stdin and stdout.";
	static char usage_message[] = "mbclean [-Amax -Blow/high -Cslope -Dmin/max \n\t-Fformat -Gfraction_low/fraction_high \n\t-Iinfile -Llonflip -Mmode -Nbuffersize -Ooutfile -Q -Xzap_beams \n\t-V -H]";
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message = NULL;

	/* MBIO read control parameters */
	int	format;
	int	format_num;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	ifile[128];
	char	*imbio_ptr = NULL;

	/* MBIO write control parameters */
	char	ofile[128];
	char	*ombio_ptr = NULL;

	/* MBIO buffer structure pointer */
	char	*buff_ptr = NULL;
	int	n_buffer_max = MBCLEAN_BUFFER_DEFAULT;
	int	nwant = MBCLEAN_BUFFER_DEFAULT;
	int	nhold = 1;
	int	nbuff;
	int	nload;
	int	ndump;
	int	done;
	int	finished;
	double	save_time_d = 0.0;

	/* mbio read and write values */
	char	*store_ptr = NULL;
	int	kind;
	struct mbclean_ping_struct ping[3];
	struct bad_struct bad[2];
	int	find_bad;
	int	ndata = 0;
	int	nrange = 0;
	int	nfraction = 0;
	int	ndeviation = 0;
	int	nouter = 0;
	int	nrail = 0;
	int	nmin = 0;
	int	nbad = 0;
	int	nflag = 0;
	int	nzero = 0;
	char	comment[256];
	int	check_slope = MB_NO;
	double	slopemax = 1.0;
	double	distancemin = 0.01;
	double	distancemax = 0.25;
	int	mode = MBCLEAN_FLAG_ONE;
	int	zap_beams = 0;
	int	zap_rails = MB_NO;
	int	check_range = MB_NO;
	double	depth_low;
	double	depth_high;
	int	check_fraction = MB_NO;
	double	fraction_low;
	double	fraction_high;
	int	check_deviation = MB_NO;
	double	deviation_max;
	int	check_num_good_min = MB_NO;
	int	num_good_min;
	int	num_good;

	/* rail processing variables */
	int	center;
	int	lowok;
	int	highok;
	int	lowbeam;
	int	highbeam;
	int	lowdist;
	int	highdist;

	/* slope processing variables */
	double	mtodeglon;
	double	mtodeglat;
	double	headingx;
	double	headingy;
	double	*bathx1 = NULL;
	double	*bathy1 = NULL;
	double	*bathx2 = NULL;
	double	*bathy2 = NULL;
	double	*bathx3 = NULL;
	double	*bathy3 = NULL;
	int	nlist;
	double	*list = NULL;
	double	median = 0.0;
	double	dd;
	double	slope;

	/* time, user, host variables */
	time_t	right_now;
	char	date[25], user[128], *user_ptr, host[128];

	/* processing variables */
	int	start;
	int	i, j, k, l, m, p, b;

	char	*ctime();
	char	*getenv();

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* reset all defaults but the format and lonflip */
	pings = 1;
	bounds[0] = -360.;
	bounds[1] = 360.;
	bounds[2] = -90.;
	bounds[3] = 90.;
	btime_i[0] = 1962;
	btime_i[1] = 2;
	btime_i[2] = 21;
	btime_i[3] = 10;
	btime_i[4] = 30;
	btime_i[5] = 0;
	btime_i[6] = 0;
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	etime_i[6] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;

	/* set default input and output */
	strcpy (ifile, "stdin");
	strcpy (ofile, "stdout");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhA:a:B:b:C:c:D:d:G:g:F:f:L:l:I:i:M:m:N:n:O:o:QqU:u:X:x:")) != -1)
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
			sscanf (optarg,"%lf", &deviation_max);
			check_deviation = MB_YES;
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf (optarg,"%lf/%lf", &depth_low,&depth_high);
			check_range = MB_YES;
			flag++;
			break;
		case 'C':
		case 'c':
			sscanf (optarg,"%lf", &slopemax);
			check_slope = MB_YES;
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%lf/%lf", &distancemin, &distancemax);
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'G':
		case 'g':
			sscanf (optarg,"%lf/%lf", &fraction_low,&fraction_high);
			check_fraction = MB_YES;
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", ifile);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'M':
		case 'm':
			sscanf (optarg,"%d", &mode);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%d", &n_buffer_max);
			if (n_buffer_max > MB_BUFFER_MAX
			    || n_buffer_max < 50)
			    n_buffer_max = MBCLEAN_BUFFER_DEFAULT;
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", ofile);
			flag++;
			break;
		case 'Q':
		case 'q':
			zap_rails = MB_YES;
			flag++;
			break;
		case 'U':
		case 'u':
			sscanf (optarg,"%d", &num_good_min);
			check_num_good_min = MB_YES;
			flag++;
			break;
		case 'X':
		case 'x':
			sscanf (optarg,"%d", &zap_beams);
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

	/* turn on slope checking if nothing else is to be used */
	if (check_slope == MB_NO
		&& zap_beams == 0
		&& zap_rails == MB_NO
		&& check_range == MB_NO
		&& check_fraction == MB_NO
		&& check_deviation == MB_NO
		&& check_num_good_min == MB_NO)
		check_slope = MB_YES;

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
		fprintf(stderr,"dbg2       data format:    %d\n",format);
		fprintf(stderr,"dbg2       input file:     %s\n",ifile);
		fprintf(stderr,"dbg2       output file:    %s\n",ofile);
		fprintf(stderr,"dbg2       mode:           %d\n",mode);
		fprintf(stderr,"dbg2       zap_beams:      %d\n",zap_beams);
		fprintf(stderr,"dbg2       zap_rails:      %d\n",zap_rails);
		fprintf(stderr,"dbg2       check_slope:    %d\n",check_slope);
		fprintf(stderr,"dbg2       maximum slope:  %f\n",slopemax);
		fprintf(stderr,"dbg2       minimum dist:   %f\n",distancemin);
		fprintf(stderr,"dbg2       minimum dist:   %f\n",distancemax);
		fprintf(stderr,"dbg2       check_range:    %d\n",check_range);
		fprintf(stderr,"dbg2       depth_low:      %f\n",depth_low);
		fprintf(stderr,"dbg2       depth_high:     %f\n",depth_high);
		fprintf(stderr,"dbg2       check_fraction: %d\n",check_fraction);
		fprintf(stderr,"dbg2       fraction_low:   %f\n",fraction_low);
		fprintf(stderr,"dbg2       fraction_high:  %f\n",fraction_high);
		fprintf(stderr,"dbg2       check_deviation:%d\n",check_deviation);
		fprintf(stderr,"dbg2       check_num_good_min:%d\n",check_num_good_min);
		fprintf(stderr,"dbg2       num_good_min:   %d\n",num_good_min);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* obtain format array location - format id will 
		be aliased to current id if old format id given */
	if ((status = mb_format(verbose,&format,&format_num,&error)) 
		!= MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_format> regarding input format %d:\n%s\n",format,message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* check that clean mode is allowed 
		for the specified data format */
	if (mb_no_flag_table[format_num] == MB_YES && mode <= 2)
		{
		fprintf(stderr,"\nMBIO format %d does not allow flagging of bad data \nas negative numbers (specified by cleaning mode %d).\n",format,mode);
		fprintf(stderr,"\nCopy the data to another format or set the cleaning mode to zero \nbad data values (-M3 or -M4).\n");
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize reading the input swath sonar file */
	if ((status = mb_read_init(
		verbose,ifile,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",ifile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	center = beams_bath/2;

	/* initialize writing the output swath sonar file */
	if ((status = mb_write_init(
		verbose,ofile,format,&ombio_ptr,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",ofile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* allocate memory for data arrays */
	for (i=0;i<3;i++)
		{
		ping[i].beamflag = NULL;
		ping[i].bath = NULL;
		ping[i].bathacrosstrack = NULL;
		ping[i].bathalongtrack = NULL;
		ping[i].amp = NULL;
		ping[i].ss = NULL;
		ping[i].ssacrosstrack = NULL;
		ping[i].ssalongtrack = NULL;
		ping[i].bathx = NULL;
		ping[i].bathy = NULL;
		status = mb_malloc(verbose,beams_bath*sizeof(char),
			&ping[i].beamflag,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bath,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bathacrosstrack,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bathalongtrack,&error);
		status = mb_malloc(verbose,beams_amp*sizeof(double),
			&ping[i].amp,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ping[i].ss,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ping[i].ssacrosstrack,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ping[i].ssalongtrack,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bathx,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bathy,&error);
		}
	status = mb_malloc(verbose,4*beams_bath*sizeof(double),
			&list,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* write comments to beginning of output file */
	kind = MB_DATA_COMMENT;
	sprintf(comment,"This bathymetry data automatically edited by program %s version %s",
		program_name,rcs_id);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"MB-system Version %s",MB_VERSION);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	strncpy(date,"\0",25);
	right_now = time((time_t *)0);
	strncpy(date,ctime(&right_now),24);
	if ((user_ptr = getenv("USER")) == NULL)
		user_ptr = getenv("LOGNAME");
	if (user_ptr != NULL)
		strcpy(user,user_ptr);
	else
		strcpy(user, "unknown");
	gethostname(host,128);
	sprintf(comment,"Run by user <%s> on cpu <%s> at <%s>",
		user,host,date);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"Control Parameters:");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  MBIO data format:   %d",format);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Input file:         %s",ifile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Output file:        %s",ofile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Longitude flip:     %d",lonflip);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Cleaning mode:      %d",mode);
	if (mode == MBCLEAN_FLAG_ONE)
		strcat(comment," (flag one beam of each outlier slope)");
	if (mode == MBCLEAN_FLAG_BOTH)
		strcat(comment," (flag both beams of each outlier slope)");
	if (mode == MBCLEAN_ZERO_ONE)
		strcat(comment," (zero one beam of each outlier slope)");
	if (mode == MBCLEAN_ZERO_BOTH)
		strcat(comment," (zero both beams of each outlier slope)");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Maximum slope:      %f",slopemax);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Minimum distance:   %f",distancemin);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Maximum distance:   %f",distancemax);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Outer beams zapped: %d",zap_beams);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (check_range == MB_YES)
		{
		sprintf(comment,"  Depth range checking on:");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		sprintf(comment,"    Minimum acceptable depth: %f",depth_low);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		sprintf(comment,"    Maximum acceptable depth: %f",depth_high);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else
		{
		sprintf(comment,"  Depth range checking off");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	if (check_fraction == MB_YES)
		{
		sprintf(comment,"  Depth fractional range checking on:");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		sprintf(comment,"    Minimum acceptable depth fraction: %f",fraction_low);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		sprintf(comment,"    Maximum acceptable depth fraction: %f",fraction_high);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else
		{
		sprintf(comment,"  Depth fractional range checking off");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	if (check_deviation == MB_YES)
		{
		sprintf(comment,"  Depth deviation from median checking on:");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		sprintf(comment,"    Maximum acceptable depth deviation: %f",deviation_max);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else
		{
		sprintf(comment,"  Depth deviation from median checking off");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	sprintf(comment," ");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);

	/* initialize the buffer */
	status = mb_buffer_init(verbose,&buff_ptr,&error);

	/* read and write */
	done = MB_NO;
	nwant = n_buffer_max;
	nhold = 1;
	start = 0;
	ping[0].id = -1;
	ping[1].id = -1;
	ping[2].id = -1;
	if (verbose == 1) fprintf(stderr,"\n");
	while (!done)
		{
		/* load some data into the buffer */
		error = MB_ERROR_NO_ERROR;
		status = mb_buffer_load(verbose,buff_ptr,imbio_ptr,nwant,
				&nload,&nbuff,&error);

		/* give the statistics */
		if (verbose > 1) fprintf(stderr,"\n");
		if (verbose >= 1)
			{
			fprintf(stderr,"%d records loaded into buffer\n\n",nload);
			}

		/* check for done */
		if (nbuff < nwant)
			{
			done = MB_YES;
			nhold = 0;
			}

		/* find first data */
		if (ping[1].id < 0)
			{
			status = mb_buffer_get_next_data(verbose,
				buff_ptr,imbio_ptr,start,&ping[1].id,
				ping[1].time_i,&ping[1].time_d,
				&ping[1].navlon,&ping[1].navlat,
				&ping[1].speed,&ping[1].heading,
				&beams_bath,&beams_amp,&pixels_ss,
				ping[1].beamflag,ping[1].bath,ping[1].amp,
				ping[1].bathacrosstrack,ping[1].bathalongtrack,
				ping[1].ss,
				ping[1].ssacrosstrack,ping[1].ssalongtrack,
				&error);
			if (status == MB_SUCCESS)
				ndata++;
			}

		/* find next data */
		finished = MB_NO;
		while (finished == MB_NO)
			{
			find_bad = MB_NO;
			status = mb_buffer_get_next_data(verbose,
				buff_ptr,imbio_ptr,ping[1].id+1,&ping[2].id,
				ping[2].time_i,&ping[2].time_d,
				&ping[2].navlon,&ping[2].navlat,
				&ping[2].speed,&ping[2].heading,
				&beams_bath,&beams_amp,&pixels_ss,
				ping[2].beamflag,ping[2].bath,ping[2].amp,
				ping[2].bathacrosstrack,ping[2].bathalongtrack,
				ping[2].ss,
				ping[2].ssacrosstrack,ping[2].ssalongtrack,
				&error);
			if (status == MB_SUCCESS)
				ndata++;
			else
				finished = MB_YES;

			if (verbose >= 2)
				{
				fprintf(stderr,"\ndbg2  current data status:\n");
				fprintf(stderr,"dbg2    status:   %d\n",status);
				fprintf(stderr,"dbg2    last:     %d\n",ping[0].id);
				fprintf(stderr,"dbg2    current:  %d\n",ping[1].id);
				fprintf(stderr,"dbg2    next:     %d\n",ping[2].id);
				}

			/* zap outer beams if requested */
			if (zap_beams > 0 && ping[1].id >= 0)
				{
				for (i=0;i<zap_beams;i++)
					{
					if (mb_beam_ok(ping[1].beamflag[i]))
					{
					find_bad = MB_YES;
					if (mode <= 2)
						{
						ping[1].beamflag[i] 
							    = MB_FLAG_FLAG + MB_FLAG_FILTER;
						nouter++;
						nflag++;
						}
					else
						{
						ping[1].beamflag[i] = MB_FLAG_NULL;
						nouter++;
						nzero++;
						}
					}
					j = beams_bath - i - 1;
					if (mb_beam_ok(ping[1].beamflag[j]))
					{
					find_bad = MB_YES;
					if (mode <= 2)
						{
						ping[1].beamflag[j] 
						    = MB_FLAG_FLAG + MB_FLAG_FILTER;
						nouter++;
						nflag++;
						}
					else
						{
						ping[1].beamflag[j] = MB_FLAG_NULL;
						nouter++;
						nzero++;
						}
					}
					}
				}

			/* check depths for acceptable range if requested */
			if (check_range == MB_YES && ping[1].id >= 0)
				{
				for (i=0;i<beams_bath;i++)
					{
					if (mb_beam_ok(ping[1].beamflag[i])
					&& (ping[1].bath[i] < depth_low
					|| ping[1].bath[i] > depth_high))
					{
					if (verbose >= 1)
					fprintf(stderr,"d: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n",
						ping[1].time_i[0],
						ping[1].time_i[1],
						ping[1].time_i[2],
						ping[1].time_i[3],
						ping[1].time_i[4],
						ping[1].time_i[5],
						ping[1].time_i[6],
						i,ping[1].bath[i]);
					find_bad = MB_YES;
					if (mode <= 2)
						{
						ping[1].beamflag[i] 
							    = MB_FLAG_FLAG + MB_FLAG_FILTER;
						nrange++;
						nflag++;
						}
					else
						{
						ping[1].beamflag[i] = MB_FLAG_NULL;
						nrange++;
						nzero++;
						}
					}
					}
				}

			/* get locations of data points in local coordinates */
			if (ping[1].id >= 0)
				mb_coor_scale(verbose,ping[1].navlat,
					&mtodeglon,&mtodeglat);
			for (j=0;j<3;j++)
			{
			if (ping[j].id >= 0)
				{
				headingx = sin(ping[j].heading*DTR);
				headingy = cos(ping[j].heading*DTR);
				for (i=0;i<beams_bath;i++)
					{
					ping[j].bathx[i] = (ping[j].navlon 
						- ping[1].navlon)/mtodeglon 
						+ headingy*ping[j].bathacrosstrack[i];
					ping[j].bathy[i] = (ping[j].navlat 
						- ping[1].navlat)/mtodeglat 
						- headingx*ping[j].bathacrosstrack[i];
					}
				}
			}
			if (verbose >= 2)
				{
				fprintf(stderr,"\ndbg2  center beam locations:\n");
				for (j=0;j<3;j++)
				{
				if (ping[j].id >= 0)
					{
					fprintf(stderr,"dbg2    ping[%d] x:    %f\n",
						j,ping[j].bathx[beams_bath/2]);
					fprintf(stderr,"dbg2    ping[%d] y:    %f\n",
						j,ping[j].bathy[beams_bath/2]);
					}
				}
				}

			/* loop over each of the beams in the current ping */
			if (ping[1].id >= 0)
			{
			for (i=0;i<beams_bath;i++)
			if (mb_beam_ok(ping[1].beamflag[i]))
			{
			
			/* get local median value */
			if (median <= 0.0)
			    median = ping[1].bath[i];
			nlist = 0;
			for (j=0;j<3;j++)
			{
			if (ping[j].id >= 0)
			    for (k=0;k<beams_bath;k++)
				{
				if (mb_beam_ok(ping[j].beamflag[k]))
				    {
				    dd = sqrt((ping[j].bathx[k] 
					- ping[1].bathx[i])
					*(ping[j].bathx[k] 
					- ping[1].bathx[i])
					+ (ping[j].bathy[k] 
					- ping[1].bathy[i])
					*(ping[j].bathy[k] 
					- ping[1].bathy[i]));
				    if (dd <= distancemax * median)
					{
					list[nlist] = ping[j].bath[k];
					nlist++;
					}
				    }
				}
			}
			sort(nlist,list-1);
			median = list[nlist/2];
			if (verbose >= 2)
				{
				fprintf(stderr,"\ndbg2  depth statistics:\n");
				fprintf(stderr,"dbg2    number:        %d\n",nlist);
				fprintf(stderr,"dbg2    minimum depth: %f\n",list[0]);
				fprintf(stderr,"dbg2    median depth:  %f\n",median);
				fprintf(stderr,"dbg2    maximum depth: %f\n",
					list[nlist-1]);
				}

			/* check fractional deviation from median if desired */
			if (check_fraction == MB_YES 
			    && median > 0.0)
			    {
			    if (ping[1].bath[i]/median < fraction_low
				|| ping[1].bath[i]/median > fraction_high)
				{
				if (verbose >= 1)
				    fprintf(stderr,"f: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %8.2f\n",
					ping[1].time_i[0],
					ping[1].time_i[1],
					ping[1].time_i[2],
					ping[1].time_i[3],
					ping[1].time_i[4],
					ping[1].time_i[5],
					ping[1].time_i[6],
					i,ping[1].bath[i],median);
				find_bad = MB_YES;
				if (mode <= 2)
				    {
				    ping[1].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
				    nfraction++;
				    nflag++;
				    }
				else
				    {
				    ping[1].beamflag[i] = MB_FLAG_NULL;
				    nfraction++;
				    nzero++;
				    }
				}
			    }

			/* check absolute deviation from median if desired */
			if (check_deviation == MB_YES 
			    && median > 0.0)
			    {
			    if (fabs(ping[1].bath[i] - median) > deviation_max)
				{
				if (verbose >= 1)
				fprintf(stderr,"a: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %8.2f\n",
				    ping[1].time_i[0],
				    ping[1].time_i[1],
				    ping[1].time_i[2],
				    ping[1].time_i[3],
				    ping[1].time_i[4],
				    ping[1].time_i[5],
				    ping[1].time_i[6],
				    i,ping[1].bath[i],median);
				find_bad = MB_YES;
				if (mode <= 2)
				    {
				    ping[1].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
				    ndeviation++;
				    nflag++;
				    }
				else
				    {
				    ping[1].beamflag[i] = MB_FLAG_NULL;
				    ndeviation++;
				    nzero++;
				    }
				}
			    }

			/* check slopes - loop over each of the beams in the current ping */
			if (check_slope == MB_YES)
			for (j=0;j<3;j++)
				{
				if (ping[j].id >= 0)
				{
				for (k=0;k<beams_bath;k++)
				if (mb_beam_ok(ping[j].beamflag[k])
					&& mb_beam_ok(ping[1].beamflag[i]))
					{
					dd = sqrt((ping[j].bathx[k] 
						- ping[1].bathx[i])
						*(ping[j].bathx[k] 
						- ping[1].bathx[i])
						+ (ping[j].bathy[k] 
						- ping[1].bathy[i])
						*(ping[j].bathy[k] 
						- ping[1].bathy[i]));
					if (dd > 0.0 && dd <= distancemax * median)
						slope = fabs(
						(ping[j].bath[k] 
						- ping[1].bath[i])/dd);
					else
						slope = 0.0;
					if (slope > slopemax 
						&& dd > distancemin * median)
						{
						find_bad = MB_YES;
						if (mode == MBCLEAN_FLAG_BOTH)
							{
							bad[0].flag = MB_YES;
							bad[0].ping = j;
							bad[0].beam = k;
							bad[0].bath = 
								ping[j].bath[k];
							bad[1].flag = MB_YES;
							bad[1].ping = 1;
							bad[1].beam = i;
							bad[1].bath = 
								ping[1].bath[i];
							ping[j].beamflag[k] = 
								MB_FLAG_FLAG + MB_FLAG_FILTER;
							ping[1].beamflag[i] = 
								MB_FLAG_FLAG + MB_FLAG_FILTER;
							nbad++;
							nflag = nflag + 2;
							}
						else if (mode == MBCLEAN_FLAG_ONE)
							{
							if (fabs((double)ping[j].bath[k]-median) 
							> fabs((double)ping[1].bath[i]-median))
								{
								bad[0].flag = MB_YES;
								bad[0].ping = j;
								bad[0].beam = k;
								bad[0].bath = ping[j].bath[k];
								bad[1].flag = MB_NO;
								ping[j].beamflag[k] 
								    = MB_FLAG_FLAG + MB_FLAG_FILTER;
								}
							else
								{
								bad[0].flag = MB_YES;
								bad[0].ping = 1;
								bad[0].beam = i;
								bad[0].bath = ping[1].bath[i];
								bad[1].flag = MB_NO;
								ping[1].beamflag[i] 
								    = MB_FLAG_FLAG + MB_FLAG_FILTER;
								}
							nbad++;
							nflag++;
							}
						else if (mode == MBCLEAN_ZERO_BOTH)
							{
							bad[0].flag = MB_YES;
							bad[0].ping = j;
							bad[0].beam = k;
							bad[0].bath = ping[j].bath[k];
							bad[1].flag = MB_YES;
							bad[1].ping = 1;
							bad[1].beam = i;
							bad[1].bath = ping[1].bath[i];
							ping[j].beamflag[k] = MB_FLAG_NULL;
							ping[1].beamflag[i] = MB_FLAG_NULL;
							nbad++;
							nzero = nzero + 2;
							}
						else if (mode == MBCLEAN_ZERO_ONE)
							{
							if (fabs((double)ping[j].bath[k]-median) 
							> fabs((double)ping[1].bath[i]-median))
								{
								bad[0].flag = MB_YES;
								bad[0].ping = j;
								bad[0].beam = k;
								bad[0].bath = ping[j].bath[k];
								bad[1].flag = MB_NO;
								ping[j].beamflag[k] = MB_FLAG_NULL;
								}
							else
								{
								bad[0].flag = MB_YES;
								bad[0].ping = 1;
								bad[0].beam = i;
								bad[0].bath = ping[1].bath[i];
								bad[1].flag = MB_NO;

								ping[1].beamflag[i] = MB_FLAG_NULL;
								}
							nbad++;
							nzero++;
							}
						}
					if (verbose >= 1 && slope > slopemax 
						&& dd > distancemin * median
						&& bad[0].flag == MB_YES)
						{
						p = bad[0].ping;
						b = bad[0].beam;
						if (verbose >= 2)
							fprintf(stderr,"\n");
						fprintf(stderr,"s: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %8.2f %6.2f %6.2f\n",
						ping[p].time_i[0],
						ping[p].time_i[1],
						ping[p].time_i[2],
						ping[p].time_i[3],
						ping[p].time_i[4],
						ping[p].time_i[5],
						ping[p].time_i[6],
						b,bad[0].bath,median,slope,dd);
						}
					if (verbose >= 1 && slope > slopemax
						&& dd > distancemin * median
						&& bad[1].flag == MB_YES)
						{
						p = bad[1].ping;
						b = bad[1].beam;
						if (verbose >= 2)
							fprintf(stderr,"\n");
						fprintf(stderr,"s: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %8.2f %6.2f %6.2f\n",
						ping[p].time_i[0],
						ping[p].time_i[1],
						ping[p].time_i[2],
						ping[p].time_i[3],
						ping[p].time_i[4],
						ping[p].time_i[5],
						ping[p].time_i[6],
						b,bad[1].bath,median,slope,dd);
						}
					}
					
				}
				}

			/* zap rails if requested */
			if (zap_rails == MB_YES && ping[1].id >= 0)
			  {

			  /* find limits of good data */
			  lowok = MB_YES;
			  highok = MB_YES;
			  lowbeam = center;
			  highbeam = center;
			  lowdist = 0;
			  highdist = 0;
			  for (j=center+1;j<beams_bath;j++)
			    {
			    k = center - (j - center);
			    if (highok == MB_YES && mb_beam_ok(ping[1].beamflag[j]))
				{
				if (ping[1].bathacrosstrack[j] <= highdist)
					{
					highok = MB_NO;
					highbeam = j;
					}
				else
					highdist = ping[1].bathacrosstrack[j];
				}
			    if (lowok == MB_YES && mb_beam_ok(ping[1].beamflag[k]))
				{
				if (ping[1].bathacrosstrack[k] >= lowdist)
					{
					lowok = MB_NO;
					lowbeam = k;
					}
				else
					lowdist = ping[1].bathacrosstrack[k];
				}
			    }


			  /* get rid of bad data */
			  if (highok == MB_NO)
			    {
			    find_bad = MB_YES;
			    for (j=highbeam;j<beams_bath;j++)
			  	{
				if (verbose >= 1)
				fprintf(stderr,"r: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n",
						ping[1].time_i[0],
						ping[1].time_i[1],
						ping[1].time_i[2],
						ping[1].time_i[3],
						ping[1].time_i[4],
						ping[1].time_i[5],
						ping[1].time_i[6],
						j,ping[1].bath[j]);
				if (mode <= 2)
				    {
				    if (mb_beam_ok(ping[1].beamflag[j]))
					{
					ping[1].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
					nrail++;
					nflag++;
					}
				    }
				else
				    {
				    if (mb_beam_ok(ping[1].beamflag[j]))
					{
					ping[1].beamflag[j] = MB_FLAG_NULL;
					nrail++;
					nzero++;
					}
				    }
			  	}
			    }
			  if (lowok == MB_NO)
			    {
			    find_bad = MB_YES;
			    for (k=0;k<=lowbeam;k++)
			  	{
				if (verbose >= 1)
				fprintf(stderr,"r: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n",
						ping[1].time_i[0],
						ping[1].time_i[1],
						ping[1].time_i[2],
						ping[1].time_i[3],
						ping[1].time_i[4],
						ping[1].time_i[5],
						ping[1].time_i[6],
						k,ping[1].bath[k]);
				if (mode <= 2)
				    {
				    if (mb_beam_ok(ping[1].beamflag[k]))
					{
					ping[1].beamflag[k] = MB_FLAG_FLAG + MB_FLAG_FILTER;
					nrail++;
					nflag++;
					}
				    }
				else
				    {
				    if (mb_beam_ok(ping[1].beamflag[k]))
					{
					ping[1].beamflag[k] = MB_FLAG_NULL;
					nrail++;
					nzero++;
					}
				    }
			  	}
			    }
			  }

			/* check for minimum number of good depths
				on each side of swath */
			if (check_num_good_min == MB_YES 
				&& num_good_min > 0
				&& ping[1].id >= 0)
				{
				/* do port */
				num_good = 0;
				for (i=0;i<center;i++)
					{
					if (mb_beam_ok(ping[1].beamflag[i]))
						num_good++;
					}
				if (num_good < num_good_min)
					{
					find_bad = MB_YES;
					for (i=0;i<center;i++)
						{
						if (mb_beam_ok(ping[1].beamflag[i]) && mode <= 2)
							{
							if (verbose >= 1)
							fprintf(stderr,"n: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %3d %3d\n",
							    ping[1].time_i[0],
							    ping[1].time_i[1],
							    ping[1].time_i[2],
							    ping[1].time_i[3],
							    ping[1].time_i[4],
							    ping[1].time_i[5],
							    ping[1].time_i[6],
							    i,ping[1].bath[i],
							    num_good, num_good_min);
							ping[1].beamflag[i] 
							    = MB_FLAG_FLAG + MB_FLAG_FILTER;
							nmin++;
							nflag++;
							}
						else if (mb_beam_ok(ping[1].beamflag[i]))
							{
							if (verbose >= 1)
							fprintf(stderr,"n: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %3d %3d\n",
							    ping[1].time_i[0],
							    ping[1].time_i[1],
							    ping[1].time_i[2],
							    ping[1].time_i[3],
							    ping[1].time_i[4],
							    ping[1].time_i[5],
							    ping[1].time_i[6],
							    i,ping[1].bath[i],
							    num_good, num_good_min);
							ping[1].beamflag[i] = MB_FLAG_NULL;
							nmin++;
							nzero++;
							}
						}
					}
					
				/* do starboard */
				num_good = 0;
				for (i=center+1;i<beams_bath;i++)
					{
					if (mb_beam_ok(ping[1].beamflag[i]))
						num_good++;
					}
				if (num_good < num_good_min)
					{
					find_bad = MB_YES;
					for (i=center+1;i<beams_bath;i++)
						{
						if (mb_beam_ok(ping[1].beamflag[i]) && mode <= 2)
							{
							if (verbose >= 1)
							fprintf(stderr,"n: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %3d %3d\n",
							    ping[1].time_i[0],
							    ping[1].time_i[1],
							    ping[1].time_i[2],
							    ping[1].time_i[3],
							    ping[1].time_i[4],
							    ping[1].time_i[5],
							    ping[1].time_i[6],
							    i,ping[1].bath[i],
							    num_good, num_good_min);
							ping[1].beamflag[i] 
							    = MB_FLAG_FLAG + MB_FLAG_FILTER;
							nmin++;
							nflag++;
							}
						else if (mb_beam_ok(ping[1].beamflag[i]))
							{
							if (verbose >= 1)
							fprintf(stderr,"n: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %3d %3d\n",
							    ping[1].time_i[0],
							    ping[1].time_i[1],
							    ping[1].time_i[2],
							    ping[1].time_i[3],
							    ping[1].time_i[4],
							    ping[1].time_i[5],
							    ping[1].time_i[6],
							    i,ping[1].bath[i],
							    num_good, num_good_min);
							ping[1].beamflag[i] = MB_FLAG_NULL;
							nmin++;
							nzero++;
							}
						}
					}
				}
			}
			}

			/* if a bad ping was found reset pings in buffer */
			if (find_bad == MB_YES)
			for (j=0;j<3;j++)
				{
				if (ping[j].id >= 0)
				status = mb_buffer_insert(verbose,
					buff_ptr,imbio_ptr,ping[j].id,
					ping[j].time_i,ping[j].time_d,
					ping[j].navlon,ping[j].navlat,
					ping[j].speed,ping[j].heading,
					beams_bath,beams_amp,pixels_ss,
					ping[j].beamflag,ping[j].bath,ping[j].amp,
					ping[j].bathacrosstrack,
					ping[j].bathalongtrack,
					ping[j].ss,ping[j].ssacrosstrack,
					ping[j].ssalongtrack,
					comment,&error);
				}

			/* reset counters and data */
			if (status == MB_SUCCESS)
				{
				for (j=0;j<2;j++)
					{
					ping[j].id = ping[j+1].id;
					for (i=0;i<7;i++)
						ping[j].time_i[i] = 
							ping[j+1].time_i[i];
					ping[j].time_d = ping[j+1].time_d;
					ping[j].navlon = ping[j+1].navlon;
					ping[j].navlat = ping[j+1].navlat;
					ping[j].speed = ping[j+1].speed;
					ping[j].heading = ping[j+1].heading;
					for (i=0;i<beams_bath;i++)
						{
						ping[j].beamflag[i] = 
							ping[j+1].beamflag[i];
						ping[j].bath[i] = 
							ping[j+1].bath[i];
						ping[j].bathacrosstrack[i] = 
							ping[j+1].bathacrosstrack[i];
						ping[j].bathalongtrack[i] = 
							ping[j+1].bathalongtrack[i];
						}
					for (i=0;i<beams_amp;i++)
						{
						ping[j].amp[i] = 
							ping[j+1].amp[i];
						}
					for (i=0;i<pixels_ss;i++)
						{
						ping[j].ss[i] = 
							ping[j+1].ss[i];
						ping[j].ssacrosstrack[i] = 
							ping[j+1].ssacrosstrack[i];
						ping[j].ssalongtrack[i] = 
							ping[j+1].ssalongtrack[i];
						}
					}
				}
			else
				finished = MB_YES;
			}

		/* dump data from the buffer */
		ndump = 0;
		if (nbuff > 0)
			{
			error = MB_ERROR_NO_ERROR;
			status = mb_buffer_dump(verbose,buff_ptr,ombio_ptr,
				nhold,&ndump,&nbuff,&error);
			ping[1].id = ping[1].id - ndump;
			ping[0].id = ping[0].id - ndump;
			}

		/* give the statistics */
		if (verbose >= 1)
			{
			fprintf(stderr,"\n%d records dumped from buffer\n",ndump);
			}
		}

	/* close the files */
	status = mb_buffer_close(verbose,&buff_ptr,imbio_ptr,&error);
	status = mb_close(verbose,&imbio_ptr,&error);
	status = mb_close(verbose,&ombio_ptr,&error);

	/* free the memory */
	for (j=0;j<3;j++)
		{
		mb_free(verbose,&ping[j].beamflag,&error); 
		mb_free(verbose,&ping[j].bath,&error); 
		mb_free(verbose,&ping[j].bathacrosstrack,&error); 
		mb_free(verbose,&ping[j].bathalongtrack,&error); 
		mb_free(verbose,&ping[j].amp,&error); 
		mb_free(verbose,&ping[j].ss,&error); 
		mb_free(verbose,&ping[j].ssacrosstrack,&error); 
		mb_free(verbose,&ping[j].ssalongtrack,&error); 
		mb_free(verbose,&ping[j].bathx,&error); 
		mb_free(verbose,&ping[j].bathy,&error); 
		}
	mb_free(verbose,&list,&error); 

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d bathymetry data records processed\n",ndata);
		fprintf(stderr,"%d outer beams zapped\n",nouter);
		fprintf(stderr,"%d beams zapped for too few good beams in ping\n",nmin);
		fprintf(stderr,"%d beams out of acceptable depth range\n",nrange);
		fprintf(stderr,"%d beams out of acceptable fractional depth range\n",nfraction);
		fprintf(stderr,"%d beams exceed acceptable deviation from median depth\n",ndeviation);
		fprintf(stderr,"%d bad rail beams identified\n",nrail);
		fprintf(stderr,"%d excessive slopes identified\n",nbad);
		fprintf(stderr,"%d beams flagged\n",nflag);
		fprintf(stderr,"%d beams zeroed\n",nzero);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
/* 	function sort sorts the data.  
 *	Uses the shell method from Numerical Recipes.
 */
#define ALN2I 1.442695022
#define TINY 1.0e-5
int sort(n,r)
int	n;
double *r;
{
	int	status = MB_SUCCESS;
	int	nn, m, j, i, lognb2;
	double	tr;

	if (n <= 0) 
		{
		status = MB_FAILURE;
		return(status);
		}

	lognb2 = (log((double) n)*ALN2I + TINY);
	m = n;
	for (nn=1;nn<=lognb2;nn++)
		{
		m >>= 1;
		for (j=m+1;j<=n;j++)
			{
			i = j - m;
			tr = r[j];
			while (i >= 1 && r[i] > tr)
				{
				r[i+m] = r[i];
				i -= m;
				}
			r[i+m] = tr;
			}
		}

	return(status);
}
/*--------------------------------------------------------------------*/
