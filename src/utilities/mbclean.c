/*--------------------------------------------------------------------
 *    The MB-system:	mbclean.c	2/26/93
 *    $Id: mbclean.c,v 5.7 2003-04-17 21:17:10 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2001, 2003 by
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
 * mbclean identifies and flags artifacts in swath sonar bathymetry data.  
 * The edit events are output to an edit save file which can be applied
 * to the data by the program mbprocess.  
 * Several algorithms are available for identifying artifacts; multiple  
 * algorithmscan be applied in a single pass.  The most commonly used  
 * approach is to identify artifacts  based  on  excessive  bathymetric 
 * slopes.  If desired, mbclean will also flag beams associated with 
 * "rails" where outer beams have  smaller  acrosstrack distances than 
 * more inner beams (-Q option).  Low and high bounds on acceptable depth
 * values can be set; depth values outside  the  acceptable  range  will  be
 * flagged.  The acceptable depth ranges can either be absolute (-B option), rela­
 * tive to the local median depth (-A option) or defined by low and high fractions
 * of the local median depth (-G option).  A set number of outer beams can also be
 * flagged.

 * The order in which the flagging algorithms are applied is as follows:
 *      1. Flag specified number of outer beams (-X option).
 *      2. Flag soundings outside specified acceptable
 *         depth range (-B option).
 *      3. Flag soundings outside acceptable depth range using
 *         fractions of local median depth (-G option).
 *      4. Flag soundings outside acceptable depth range using
 *         deviation from local median depth (-A option).
 *      5. Flag soundings associated with excessive slopes
 *         (-C option or default).
 *      6. Zap "rails" (-Q option).
 *      7. Flag all soundings in pings with too few
 *         good soundings (-U option).
 * 
 * 
 * Author:	D. W. Caress
 * Date:	February 26, 1993 (buffered i/o version)
 * Date:	January 19, 2001 (edit save file version)
 *
 * Acknowledgments:
 * This program is based to a large extent on the program mbcleanx
 * by Alberto Malinverno (formerly at L-DEO, now at Schlumberger),
 * which was in turn based on the original program mbclean (v. 1.0)
 * by David Caress.
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.6  2002/10/02 23:56:06  caress
 * Release 5.0.beta24
 *
 * Revision 5.5  2002/08/21 00:57:11  caress
 * Release 5.0.beta22
 *
 * Revision 5.4  2001/12/30 20:41:03  caress
 * Fixed sorting messages.
 *
 * Revision 5.3  2001/07/20 00:34:38  caress
 * Release 5.0.beta03
 *
 * Revision 5.2  2001/03/22 21:14:16  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.1  2001/01/23  01:16:25  caress
 * Working esf version.
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.21  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.20  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.19  1998/12/17  22:50:20  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.18  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
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
 * in mb_buffer source cp hs91_283.d01.esf.BAK hs91_283.d01.esf
 code.  The parameter list now includes
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
#include <sys/stat.h>
#include <time.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_io.h"
#include "../../include/mb_swap.h"
#include "../../include/mb_process.h"

/* local defines */
#define	MBCLEAN_FLAG_ONE	1
#define	MBCLEAN_FLAG_BOTH	2
#define	MBCLEAN_ZERO_ONE	3
#define	MBCLEAN_ZERO_BOTH	4

/* MBIO buffer size default */
#define	MBCLEAN_BUFFER_DEFAULT	500

/* edit action defines */
#define	MBCLEAN_NOACTION	0

/* ping structure definition */
struct mbclean_ping_struct 
	{
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	int	beams_bath;
	char	*beamflag;
	double	*bath;
	double	*bathacrosstrack;
	double	*bathalongtrack;
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

/* edit output function */
int mbclean_save_edit(int verbose, FILE *sofp, double time_d, int beam, 
			int action, int *error);

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char rcs_id[] = "$Id: mbclean.c,v 5.7 2003-04-17 21:17:10 caress Exp $";
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
	int	read_datalist = MB_NO;
	char	read_file[MB_PATH_MAXLINE];
	char	swathfile[MB_PATH_MAXLINE];
	char	swathfileread[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	int	format;
	int	formatread;
	int	variable_beams;
	int	traveltime;
	int	beam_flagging; 
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	double	distance;
	double	altitude;
	double	sonardepth;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	double	*amp;
	double	*ss;
	double	*ssacrosstrack;
	double	*ssalongtrack;

	/* mbio read and write values */
	void	*mbio_ptr = NULL;
	void	*store_ptr = NULL;
	int	kind;
	struct mbclean_ping_struct ping[3];
	int	nrec, irec;
	int	pingsread;
	struct bad_struct bad[2];
	int	find_bad;
	int	nfiletot = 0;
	int	ndatatot = 0;
	int	nrangetot = 0;
	int	nfractiontot = 0;
	int	ndeviationtot = 0;
	int	noutertot = 0;
	int	nrailtot = 0;
	int	nmintot = 0;
	int	nbadtot = 0;
	int	nflagtot = 0;
	int	nunflagtot = 0;
	int	nzerotot = 0;
	int	nflagesftot = 0;
	int	nunflagesftot = 0;
	int	nzeroesftot = 0;
	int	ndata = 0;
	int	nrange = 0;
	int	nfraction = 0;
	int	ndeviation = 0;
	int	nouter = 0;
	int	nrail = 0;
	int	nmin = 0;
	int	nbad = 0;
	int	nflag = 0;
	int	nunflag = 0;
	int	nzero = 0;
	int	nflagesf = 0;
	int	nunflagesf = 0;
	int	nzeroesf = 0;
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

	/* save file control variables */
	int	esffile_open = MB_NO;
	char	esffile[MB_PATH_MAXLINE];
	struct mb_esf_struct esf;

	/* processing variables */
	int	read_data;
	int	start, done;
	int	i, j, k, l, m, p, b;

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
	strcpy(read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhA:a:B:b:C:c:D:d:G:g:F:f:L:l:I:i:M:m:QqU:u:X:x:")) != -1)
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
			sscanf (optarg,"%d", &mode);
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
		fprintf(stderr,"dbg2       input file:     %s\n",read_file);
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

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,read_file,NULL,&format,&error);

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
			    swathfile,&format,&file_weight,&error)
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(swathfile, read_file);
	    read_data = MB_YES;
	    }

	/* loop over all files to be read */
	while (read_data == MB_YES)
	{

	/* check format and get format flags */
	if ((status = mb_format_flags(verbose,&format,
			&variable_beams, &traveltime, &beam_flagging, 
			&error)) 
		!= MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_format_flags> regarding input format %d:\n%s\n",format,message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* check that clean mode is allowed 
		for the specified data format */
	if (beam_flagging == MB_NO && mode <= 2)
		{
		fprintf(stderr,"\nMBIO format %d does not allow flagging of bad data \nas negative numbers (specified by cleaning mode %d).\n",format,mode);
		fprintf(stderr,"\nCopy the data to another format or set the cleaning mode to zero \nbad data values (-M3 or -M4).\n");
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	/* check for "fast bathymetry" or "fbt" file */
	strcpy(swathfileread, swathfile);
	formatread = format;
	mb_get_fbt(verbose, swathfileread, &formatread, &error);

	/* initialize reading the input swath sonar file */
	if ((status = mb_read_init(
		verbose,swathfileread,formatread,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",swathfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
		
	/* initialize and increment counting variables */
	ndata = 0;
	nrange = 0;
	nfraction = 0;
	ndeviation = 0;
	nouter = 0;
	nrail = 0;
	nmin = 0;
	nbad = 0;
	nflag = 0;
	nunflag = 0;
	nzero = 0;
	nflagesf = 0;
	nunflagesf = 0;
	nzeroesf = 0;

	/* give the statistics */
	if (verbose >= 0)
		{
		fprintf(stderr,"\nProcessing %s\n",swathfileread);
		}

	/* allocate memory for data arrays */
	for (i=0;i<3;i++)
		{
		ping[i].beamflag = NULL;
		ping[i].bath = NULL;
		ping[i].bathacrosstrack = NULL;
		ping[i].bathalongtrack = NULL;
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
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bathx,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bathy,&error);
		}
	amp = NULL;
	ss = NULL;
	ssacrosstrack = NULL;
	ssalongtrack = NULL;
	status = mb_malloc(verbose,beams_amp*sizeof(double),
			&amp,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ssacrosstrack,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ssalongtrack,&error);
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
		
	/* now deal with old edit save file */
	if (status == MB_SUCCESS)
	    {
	    /* reset message */
	    fprintf(stderr, "Sorting old edits...\n");

	    /* handle esf edits */
	    status = mb_esf_load(verbose, swathfile, 
			    MB_YES, MB_YES, esffile, &esf, &error);
	    if (status == MB_SUCCESS
		    && esf.esffp != NULL)
		    esffile_open = MB_YES;
	    if (status == MB_FAILURE 
		    && error == MB_ERROR_OPEN_FAIL)
		    {
		    esffile_open = MB_NO;
		    fprintf(stderr, "\nUnable to open new edit save file %s\n", 
			esf.esffile);
		    }
	    else if (status == MB_FAILURE 
		    && error == MB_ERROR_MEMORY_FAIL)
		    {
		    esffile_open = MB_NO;
		    fprintf(stderr, "\nUnable to allocate memory for edits in esf file %s\n", esf.esffile);
		    }
	    }
 
	/* read */
	done = MB_NO;
	start = 0;
	nrec = 0;
	while (done == MB_NO)
	    {
	    if (verbose > 1) fprintf(stderr,"\n");

	    /* read next record */
	    error = MB_ERROR_NO_ERROR;
	    status = mb_get(verbose,
			    mbio_ptr,&kind,&pingsread,
			    ping[nrec].time_i,&ping[nrec].time_d,
			    &ping[nrec].navlon,&ping[nrec].navlat,
			    &ping[nrec].speed,&ping[nrec].heading,
			    &distance,&altitude,&sonardepth,
			    &ping[nrec].beams_bath,&beams_amp,&pixels_ss,
			    ping[nrec].beamflag,ping[nrec].bath,amp,
			    ping[nrec].bathacrosstrack,ping[nrec].bathalongtrack,
			    ss,ssacrosstrack,ssalongtrack,
			    comment,
			    &error);
	    if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  current data status:\n");
		fprintf(stderr,"dbg2    kind:       %d\n",kind);
		fprintf(stderr,"dbg2    status:     %d\n",status);
		fprintf(stderr,"dbg2    ndata:      %d\n",ndata);
		fprintf(stderr,"dbg2    nrec:       %d\n",nrec);
		fprintf(stderr,"dbg2    nflagesf:   %d\n",nflagesf);
		fprintf(stderr,"dbg2    nunflagesf: %d\n",nunflagesf);
		fprintf(stderr,"dbg2    nzeroesf:   %d\n",nzeroesf);
		fprintf(stderr,"dbg2    nouter:     %d\n",nouter);
		fprintf(stderr,"dbg2    nmin:       %d\n",nmin);
		fprintf(stderr,"dbg2    nrange:     %d\n",nrange);
		fprintf(stderr,"dbg2    nfraction:  %d\n",nfraction);
		fprintf(stderr,"dbg2    ndeviation: %d\n",ndeviation);
		fprintf(stderr,"dbg2    nrail:      %d\n",nrail);
		fprintf(stderr,"dbg2    nbad:       %d\n",nbad);
		fprintf(stderr,"dbg2    nflag:      %d\n",nflag);
		fprintf(stderr,"dbg2    nunflag:    %d\n",nunflag);
		fprintf(stderr,"dbg2    nzero:      %d\n",nzero);
		}
	    if (status == MB_SUCCESS && kind == MB_DATA_DATA)
		{
		/* get locations of data points in local coordinates */
		mb_coor_scale(verbose,ping[nrec].navlat,
				    &mtodeglon,&mtodeglat);
		headingx = sin(ping[nrec].heading*DTR);
		headingy = cos(ping[nrec].heading*DTR);
		for (i=0;i<ping[nrec].beams_bath;i++)
		    {
		    ping[nrec].bathx[i] = (ping[nrec].navlon 
					- ping[nrec].navlon) / mtodeglon 
				    + headingy * ping[nrec].bathacrosstrack[i];
		    ping[nrec].bathy[i] = (ping[nrec].navlat 
					- ping[nrec].navlat) / mtodeglat 
				    - headingx * ping[nrec].bathacrosstrack[i];
		    }
		if (verbose >= 2)
		    {
		    fprintf(stderr,"\ndbg2  center beam locations:\n");
		    for (j=0;j<nrec;j++)
			{
			fprintf(stderr,"dbg2    ping[%d] x:%f    y:%f\n",
					j,ping[j].bathx[ping[j].beams_bath/2],
					ping[j].bathy[ping[j].beams_bath/2]);
			}
		    }
		    
		/* apply saved edits */
		status = mb_esf_apply(verbose, &esf, 
		    		ping[nrec].time_d, ping[nrec].beams_bath, 
				ping[nrec].beamflag, &error);

		/* update counters */
		ndata++;
		nrec++;		    
		}
	    else if (error > MB_ERROR_NO_ERROR)
		{
		done = MB_YES;
		}
		    
	    /* process a record */
	    if (nrec > 0)
		{
		/* get record to process */
		if (nrec >= 2)
		    irec = 1;
		else if (nrec == 1)
		    irec = 0;
		    
		/* get center beam */
		center = ping[irec].beams_bath / 2;

		/* zap outer beams if requested */
		if (zap_beams > 0)
		    {
		    for (i=0;i<MIN(zap_beams, center);i++)
			{
			if (mb_beam_ok(ping[irec].beamflag[i]))
			    {
			    find_bad = MB_YES;
			    if (verbose >= 1)
			    fprintf(stderr,"z: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n",
					    ping[irec].time_i[0],
					    ping[irec].time_i[1],
					    ping[irec].time_i[2],
					    ping[irec].time_i[3],
					    ping[irec].time_i[4],
					    ping[irec].time_i[5],
					    ping[irec].time_i[6],
					    i,ping[irec].bath[i]);
			    if (mode <= 2)
				{
				ping[irec].beamflag[i] 
					    = MB_FLAG_FLAG + MB_FLAG_FILTER;
				nouter++;
				nflag++;
				mb_esf_save(verbose, &esf, ping[irec].time_d, i, MBP_EDIT_FILTER, &error);
				}
			    else
				{
				ping[irec].beamflag[i] = MB_FLAG_NULL;
				nouter++;
				nzero++;
				mb_esf_save(verbose, &esf, ping[irec].time_d, i, MBP_EDIT_ZERO, &error);
				}
			    }
			j = ping[irec].beams_bath - i - 1;
			if (mb_beam_ok(ping[irec].beamflag[j]))
			    {
			    find_bad = MB_YES;
			    if (verbose >= 1)
			    fprintf(stderr,"z: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n",
					    ping[irec].time_i[0],
					    ping[irec].time_i[1],
					    ping[irec].time_i[2],
					    ping[irec].time_i[3],
					    ping[irec].time_i[4],
					    ping[irec].time_i[5],
					    ping[irec].time_i[6],
					    j,ping[irec].bath[j]);
			    if (mode <= 2)
				{
				ping[irec].beamflag[j] 
				    = MB_FLAG_FLAG + MB_FLAG_FILTER;
				nouter++;
				nflag++;
				mb_esf_save(verbose, &esf, ping[irec].time_d, j, MBP_EDIT_FILTER, &error);
				}
			    else
				{
				ping[irec].beamflag[j] = MB_FLAG_NULL;
				nouter++;
				nzero++;
				mb_esf_save(verbose, &esf, ping[irec].time_d, j, MBP_EDIT_ZERO, &error);
				}
			    }
			}
		    }
    
		/* check depths for acceptable range if requested */
		if (check_range == MB_YES)
		    {
		    for (i=0;i<ping[irec].beams_bath;i++)
			{
			if (mb_beam_ok(ping[irec].beamflag[i])
				&& (ping[irec].bath[i] < depth_low
				|| ping[irec].bath[i] > depth_high))
			    {
			    if (verbose >= 1)
			    fprintf(stderr,"d: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n",
				    ping[irec].time_i[0],
				    ping[irec].time_i[1],
				    ping[irec].time_i[2],
				    ping[irec].time_i[3],
				    ping[irec].time_i[4],
				    ping[irec].time_i[5],
				    ping[irec].time_i[6],
				    i,ping[irec].bath[i]);
			    find_bad = MB_YES;
			    if (mode <= 2)
				{
				ping[irec].beamflag[i] 
					    = MB_FLAG_FLAG + MB_FLAG_FILTER;
				nrange++;
				nflag++;
				mb_esf_save(verbose, &esf, ping[irec].time_d, i, MBP_EDIT_FILTER, &error);
				}
			    else
				{
				ping[irec].beamflag[i] = MB_FLAG_NULL;
				nrange++;
				nzero++;
				mb_esf_save(verbose, &esf, ping[irec].time_d, i, MBP_EDIT_ZERO, &error);
				}
			    }
			}
		    }

		/* zap rails if requested */
		if (zap_rails == MB_YES)
		    {
		    /* find limits of good data */
		    lowok = MB_YES;
		    highok = MB_YES;
		    lowbeam = center;
		    highbeam = center;
		    lowdist = 0;
		    highdist = 0;
		    for (j=center+1;j<ping[irec].beams_bath;j++)
			{
			k = center - (j - center);
			if (highok == MB_YES && mb_beam_ok(ping[irec].beamflag[j]))
			    {
			    if (ping[irec].bathacrosstrack[j] <= highdist)
				    {
				    highok = MB_NO;
				    highbeam = j;
				    }
			    else
				    highdist = ping[irec].bathacrosstrack[j];
			    }
			if (lowok == MB_YES && mb_beam_ok(ping[irec].beamflag[k]))
			    {
			    if (ping[irec].bathacrosstrack[k] >= lowdist)
				    {
				    lowok = MB_NO;
				    lowbeam = k;
				    }
			    else
				    lowdist = ping[irec].bathacrosstrack[k];
			    }
			}


		    /* get rid of bad data */
		    if (highok == MB_NO)
			{
			find_bad = MB_YES;
			for (j=highbeam;j<ping[irec].beams_bath;j++)
			    {
			    if (verbose >= 1)
			    fprintf(stderr,"r: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n",
					    ping[irec].time_i[0],
					    ping[irec].time_i[1],
					    ping[irec].time_i[2],
					    ping[irec].time_i[3],
					    ping[irec].time_i[4],
					    ping[irec].time_i[5],
					    ping[irec].time_i[6],
					    j,ping[irec].bath[j]);
			    if (mode <= 2)
				{
				if (mb_beam_ok(ping[irec].beamflag[j]))
				    {
				    ping[irec].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
				    nrail++;
				    nflag++;
fprintf(stderr,"EDIT SAVED: %f %d %d\n",ping[irec].time_d, i, MBP_EDIT_FILTER);
				    mb_esf_save(verbose, &esf, ping[irec].time_d, j, MBP_EDIT_FILTER, &error);
				    }
				}
			    else
				{
				if (mb_beam_ok(ping[irec].beamflag[j]))
				    {
				    ping[irec].beamflag[j] = MB_FLAG_NULL;
				    nrail++;
				    nzero++;
				    mb_esf_save(verbose, &esf, ping[irec].time_d, j, MBP_EDIT_ZERO, &error);
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
					    ping[irec].time_i[0],
					    ping[irec].time_i[1],
					    ping[irec].time_i[2],
					    ping[irec].time_i[3],
					    ping[irec].time_i[4],
					    ping[irec].time_i[5],
					    ping[irec].time_i[6],
					    k,ping[irec].bath[k]);
			    if (mode <= 2)
				{
				if (mb_beam_ok(ping[irec].beamflag[k]))
				    {
				    ping[irec].beamflag[k] = MB_FLAG_FLAG + MB_FLAG_FILTER;
				    nrail++;
				    nflag++;
				    mb_esf_save(verbose, &esf, ping[irec].time_d, k, MBP_EDIT_FILTER, &error);
				    }
				}
			    else
				{
				if (mb_beam_ok(ping[irec].beamflag[k]))
				    {
				    ping[irec].beamflag[k] = MB_FLAG_NULL;
				    nrail++;
				    nzero++;
				    mb_esf_save(verbose, &esf, ping[irec].time_d, k, MBP_EDIT_FILTER, &error);
				    }
				}
			    }
			}
		    }

		/* do tests that require looping over all available beams */
		if (check_fraction == MB_YES 
		    || check_deviation == MB_YES
		    || check_slope == MB_YES)
		    {
		    for (i=0;i<ping[irec].beams_bath;i++)
			{
			if (mb_beam_ok(ping[irec].beamflag[i]))
			    {
			    /* get local median value from all available records */
			    if (median <= 0.0)
				median = ping[irec].bath[i];
			    nlist = 0;
			    for (j=0;j<nrec;j++)
				{
				for (k=0;k<ping[j].beams_bath;k++)
				    {
				    if (mb_beam_ok(ping[j].beamflag[k]))
					{
					dd = sqrt((ping[j].bathx[k] 
							- ping[irec].bathx[i])
						    * (ping[j].bathx[k] 
							- ping[irec].bathx[i])
						+ (ping[j].bathy[k] 
							- ping[irec].bathy[i])
						    * (ping[j].bathy[k] 
							- ping[irec].bathy[i]));
					if (dd <= distancemax * median)
					    {
					    list[nlist] = ping[j].bath[k];
					    nlist++;
					    }
					}
				    }
				}
			    qsort((char *)list,nlist,sizeof(double),mb_double_compare);
			    median = list[nlist / 2];
			    if (verbose >= 2)
				{
				fprintf(stderr,"\ndbg2  depth statistics:\n");
				fprintf(stderr,"dbg2    number:        %d\n",nlist);
				fprintf(stderr,"dbg2    minimum depth: %f\n",list[0]);
				fprintf(stderr,"dbg2    median depth:  %f\n",median);
				fprintf(stderr,"dbg2    maximum depth: %f\n",list[nlist-1]);
				}

			    /* check fractional deviation from median if desired */
			    if (check_fraction == MB_YES 
				&& median > 0.0)
				{
				if (ping[irec].bath[i]/median < fraction_low
				    || ping[irec].bath[i]/median > fraction_high)
				    {
				    if (verbose >= 1)
					fprintf(stderr,"f: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %8.2f\n",
					    ping[irec].time_i[0],
					    ping[irec].time_i[1],
					    ping[irec].time_i[2],
					    ping[irec].time_i[3],
					    ping[irec].time_i[4],
					    ping[irec].time_i[5],
					    ping[irec].time_i[6],
					    i,ping[irec].bath[i],median);
				    find_bad = MB_YES;
				    if (mode <= 2)
					{
					ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
					nfraction++;
					nflag++;
					mb_esf_save(verbose, &esf, ping[irec].time_d, i, MBP_EDIT_FILTER, &error);
					}
				    else
					{
					ping[irec].beamflag[i] = MB_FLAG_NULL;
					nfraction++;
					nzero++;
					mb_esf_save(verbose, &esf, ping[irec].time_d, i, MBP_EDIT_ZERO, &error);
					}
				    }
				}
	    
			    /* check absolute deviation from median if desired */
			    if (check_deviation == MB_YES 
				&& median > 0.0)
				{
				if (fabs(ping[irec].bath[i] - median) > deviation_max)
				    {
				    if (verbose >= 1)
				    fprintf(stderr,"a: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %8.2f\n",
					ping[irec].time_i[0],
					ping[irec].time_i[1],
					ping[irec].time_i[2],
					ping[irec].time_i[3],
					ping[irec].time_i[4],
					ping[irec].time_i[5],
					ping[irec].time_i[6],
					i,ping[irec].bath[i],median);
				    find_bad = MB_YES;
				    if (mode <= 2)
					{
					ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
					ndeviation++;
					nflag++;
					mb_esf_save(verbose, &esf, ping[irec].time_d, i, MBP_EDIT_FILTER, &error);
					}
				    else
					{
					ping[irec].beamflag[i] = MB_FLAG_NULL;
					ndeviation++;
					nzero++;
					mb_esf_save(verbose, &esf, ping[irec].time_d, i, MBP_EDIT_ZERO, &error);
					}
				    }
				}

			    /* check slopes - loop over each of the beams in the current ping */
			    if (check_slope == MB_YES
				&& nrec == 3 
				&& median > 0.0)
			    for (j=0;j<nrec;j++)
				{
				for (k=0;k<ping[j].beams_bath;k++)
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
					if (dd > 0.0 && dd <= distancemax * median)
					    slope = fabs((ping[j].bath[k] 
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
						mb_esf_save(verbose, &esf, ping[j].time_d, k, MBP_EDIT_FILTER, &error);
						mb_esf_save(verbose, &esf, ping[1].time_d, i, MBP_EDIT_FILTER, &error);
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
						    mb_esf_save(verbose, &esf, ping[j].time_d, k, MBP_EDIT_FILTER, &error);
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
						    mb_esf_save(verbose, &esf, ping[1].time_d, i, MBP_EDIT_FILTER, &error);
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
						mb_esf_save(verbose, &esf, ping[j].time_d, k, MBP_EDIT_ZERO, &error);
						mb_esf_save(verbose, &esf, ping[1].time_d, i, MBP_EDIT_ZERO, &error);
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
						    mb_esf_save(verbose, &esf, ping[j].time_d, k, MBP_EDIT_ZERO, &error);
						    }
						else
						    {
						    bad[0].flag = MB_YES;
						    bad[0].ping = 1;
						    bad[0].beam = i;
						    bad[0].bath = ping[1].bath[i];
						    bad[1].flag = MB_NO;
    
						    ping[1].beamflag[i] = MB_FLAG_NULL;
						    mb_esf_save(verbose, &esf, ping[1].time_d, i, MBP_EDIT_ZERO, &error);
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
			    }
			}
		    }

		/* check for minimum number of good depths
			on each side of swath */
		if (check_num_good_min == MB_YES 
		    && num_good_min > 0)
		    {
		    /* do port */
		    num_good = 0;
		    for (i=0;i<center;i++)
			{
			if (mb_beam_ok(ping[irec].beamflag[i]))
				num_good++;
			}
		    if (num_good < num_good_min)
			{
			find_bad = MB_YES;
			for (i=0;i<center;i++)
			    {
			    if (mb_beam_ok(ping[irec].beamflag[i]) && mode <= 2)
				{
				if (verbose >= 1)
				fprintf(stderr,"n: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %3d %3d\n",
				    ping[irec].time_i[0],
				    ping[irec].time_i[1],
				    ping[irec].time_i[2],
				    ping[irec].time_i[3],
				    ping[irec].time_i[4],
				    ping[irec].time_i[5],
				    ping[irec].time_i[6],
				    i,ping[irec].bath[i],
				    num_good, num_good_min);
				ping[irec].beamflag[i] 
				    = MB_FLAG_FLAG + MB_FLAG_FILTER;
				nmin++;
				nflag++;
				mb_esf_save(verbose, &esf, ping[irec].time_d, i, MBP_EDIT_FILTER, &error);
				}
			    else if (mb_beam_ok(ping[irec].beamflag[i]))
				{
				if (verbose >= 1)
				fprintf(stderr,"n: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %3d %3d\n",
				    ping[irec].time_i[0],
				    ping[irec].time_i[1],
				    ping[irec].time_i[2],
				    ping[irec].time_i[3],
				    ping[irec].time_i[4],
				    ping[irec].time_i[5],
				    ping[irec].time_i[6],
				    i,ping[irec].bath[i],
				    num_good, num_good_min);
				ping[irec].beamflag[i] = MB_FLAG_NULL;
				nmin++;
				nzero++;
				mb_esf_save(verbose, &esf, ping[irec].time_d, i, MBP_EDIT_ZERO, &error);
				}
			    }
			}
			    
		    /* do starboard */
		    num_good = 0;
		    for (i=center+1;i<ping[irec].beams_bath;i++)
			{
			if (mb_beam_ok(ping[irec].beamflag[i]))
				num_good++;
			}
		    if (num_good < num_good_min)
			{
			find_bad = MB_YES;
			for (i=center+1;i<ping[irec].beams_bath;i++)
			    {
			    if (mb_beam_ok(ping[irec].beamflag[i]) && mode <= 2)
				{
				if (verbose >= 1)
				fprintf(stderr,"n: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %3d %3d\n",
				    ping[irec].time_i[0],
				    ping[irec].time_i[1],
				    ping[irec].time_i[2],
				    ping[irec].time_i[3],
				    ping[irec].time_i[4],
				    ping[irec].time_i[5],
				    ping[irec].time_i[6],
				    i,ping[irec].bath[i],
				    num_good, num_good_min);
				ping[irec].beamflag[i] 
				    = MB_FLAG_FLAG + MB_FLAG_FILTER;
				nmin++;
				nflag++;
				mb_esf_save(verbose, &esf, ping[irec].time_d, i, MBP_EDIT_FILTER, &error);
				}
			    else if (mb_beam_ok(ping[irec].beamflag[i]))
				{
				if (verbose >= 1)
				fprintf(stderr,"n: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %3d %3d\n",
				    ping[irec].time_i[0],
				    ping[irec].time_i[1],
				    ping[irec].time_i[2],
				    ping[irec].time_i[3],
				    ping[irec].time_i[4],
				    ping[irec].time_i[5],
				    ping[irec].time_i[6],
				    i,ping[irec].bath[i],
				    num_good, num_good_min);
				ping[irec].beamflag[i] = MB_FLAG_NULL;
				nmin++;
				nzero++;
				mb_esf_save(verbose, &esf, ping[irec].time_d, i, MBP_EDIT_ZERO, &error);
				}
			    }
			}
		    }
		}

	    /* reset counters and data */
	    if (status == MB_SUCCESS
		&& nrec == 3)
		{
		nrec = 2;
		for (j=0;j<2;j++)
		    {
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
		    }
		}
	    }

	/* close the files */
	status = mb_close(verbose,&mbio_ptr,&error);

/*for (i=0;i<esf.nedit;i++)
{
if (esf.edit_use[i] == 1000)
fprintf(stderr,"BEAM FLAG TIED TO NULL BEAM: i:%d edit: %f %d %d   %d\n",
i,esf.edit_time_d[i],esf.edit_beam[i],esf.edit_action[i],esf.edit_use[i]);
else if (esf.edit_use[i] == 100)
fprintf(stderr,"DUPLICATE BEAM FLAG: i:%d edit: %f %d %d   %d\n",
i,esf.edit_time_d[i],esf.edit_beam[i],esf.edit_action[i],esf.edit_use[i]);
else if (esf.edit_use[i] == 1)
fprintf(stderr,"BEAM FLAG USED:      i:%d edit: %f %d %d   %d\n",
i,esf.edit_time_d[i],esf.edit_beam[i],esf.edit_action[i],esf.edit_use[i]);
else if (esf.edit_use[i] != 1)
fprintf(stderr,"BEAM FLAG NOT USED:  i:%d edit: %f %d %d   %d\n",
i,esf.edit_time_d[i],esf.edit_beam[i],esf.edit_action[i],esf.edit_use[i]);
}*/

	/* close edit save file */
	status = mb_esf_close(verbose, &esf, &error);
	
	/* update mbprocess parameter file */
	if (esffile_open == MB_YES)
	    {
	    /* update mbprocess parameter file */
	    status = mb_pr_update_format(verbose, swathfile, 
			MB_YES, format, 
			&error);
	    status = mb_pr_update_edit(verbose, swathfile, 
			MBP_EDIT_ON, esffile, 
			&error);
	    }

	/* free the memory */
	for (j=0;j<3;j++)
		{
		mb_free(verbose,&ping[j].beamflag,&error); 
		mb_free(verbose,&ping[j].bath,&error); 
		mb_free(verbose,&ping[j].bathacrosstrack,&error); 
		mb_free(verbose,&ping[j].bathalongtrack,&error); 
		mb_free(verbose,&ping[j].bathx,&error); 
		mb_free(verbose,&ping[j].bathy,&error); 
		}
	mb_free(verbose,&amp,&error); 
	mb_free(verbose,&ss,&error); 
	mb_free(verbose,&ssacrosstrack,&error); 
	mb_free(verbose,&ssalongtrack,&error); 
	mb_free(verbose,&list,&error); 

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* increment the total counting variables */
	nfiletot++;
	ndatatot += ndata;
	nflagesftot += nflagesf;
	nunflagesftot += nunflagesf;
	nzeroesftot += nzeroesf;
	nrangetot += nrange;
	nfractiontot += nfraction;
	ndeviationtot += ndeviation;
	noutertot += nouter;
	nrailtot += nrail;
	nmintot += nmin;
	nbadtot += nbad;
	nflagtot += nflag;
	nunflagtot += nunflag;
	nzerotot += nzero;

	/* give the statistics */
	if (verbose >= 0)
		{
		fprintf(stderr,"%d bathymetry data records processed\n",ndata);
		if (esf.nedit > 0)
			{
			fprintf(stderr,"%d beams flagged in old esf file\n",nflagesf);
			fprintf(stderr,"%d beams unflagged in old esf file\n",nunflagesf);
			fprintf(stderr,"%d beams zeroed in old esf file\n",nzeroesf);
			}
		fprintf(stderr,"%d outer beams zapped\n",nouter);
		fprintf(stderr,"%d beams zapped for too few good beams in ping\n",nmin);
		fprintf(stderr,"%d beams out of acceptable depth range\n",nrange);
		fprintf(stderr,"%d beams out of acceptable fractional depth range\n",nfraction);
		fprintf(stderr,"%d beams exceed acceptable deviation from median depth\n",ndeviation);
		fprintf(stderr,"%d bad rail beams identified\n",nrail);
		fprintf(stderr,"%d excessive slopes identified\n",nbad);
		fprintf(stderr,"%d beams flagged\n",nflag);
		fprintf(stderr,"%d beams unflagged\n",nunflag);
		fprintf(stderr,"%d beams zeroed\n",nzero);
		}

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if (status = mb_datalist_read(verbose,datalist,
			    swathfile,&format,&file_weight,&error)
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

	/* give the total statistics */
	if (verbose >= 0)
		{
		fprintf(stderr,"\nMBclean Processing Totals:\n");
		fprintf(stderr,"-------------------------\n");
		fprintf(stderr,"%d total swath data files processed\n",nfiletot);
		fprintf(stderr,"%d total bathymetry data records processed\n",ndatatot);
		fprintf(stderr,"%d total beams flagged in old esf files\n",nflagesftot);
		fprintf(stderr,"%d total beams unflagged in old esf files\n",nunflagesftot);
		fprintf(stderr,"%d total beams zeroed in old esf files\n",nzeroesftot);
		fprintf(stderr,"%d total outer beams zapped\n",noutertot);
		fprintf(stderr,"%d total beams zapped for too few good beams in ping\n",nmintot);
		fprintf(stderr,"%d total beams out of acceptable depth range\n",nrangetot);
		fprintf(stderr,"%d total beams out of acceptable fractional depth range\n",nfractiontot);
		fprintf(stderr,"%d total beams exceed acceptable deviation from median depth\n",ndeviationtot);
		fprintf(stderr,"%d total bad rail beams identified\n",nrailtot);
		fprintf(stderr,"%d total excessive slopes identified\n",nbadtot);
		fprintf(stderr,"%d total beams flagged\n",nflagtot);
		fprintf(stderr,"%d total beams unflagged\n",nunflagtot);
		fprintf(stderr,"%d total beams zeroed\n",nzerotot);
		}

	/* set program status */
	status = MB_SUCCESS;

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
int mbclean_save_edit(int verbose, FILE *sofp, double time_d, int beam, int action, int *error)
{
	/* local variables */
	char	*function_name = "mbclean_save_edit";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
	
		fprintf(stderr,"dbg2       sofp:            %d\n",sofp);
		fprintf(stderr,"dbg2       time_d:          %f\n",time_d);
		fprintf(stderr,"dbg2       beam:            %d\n",beam);
		fprintf(stderr,"dbg2       action:          %d\n",action);
		}
	/* write out the edit */
fprintf(stderr,"OUTPUT EDIT: %f %d %d\n",time_d,beam,action);
	if (sofp != NULL)
	    {		
#ifdef BYTESWAPPED
	    mb_swap_double(&time_d);
	    beam = mb_swap_int(beam);
	    action = mb_swap_int(action);
#endif
	    if (fwrite(&time_d, sizeof(double), 1, sofp) != 1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	    if (status == MB_SUCCESS
		&& fwrite(&beam, sizeof(int), 1, sofp) != 1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	    if (status == MB_SUCCESS
		&& fwrite(&action, sizeof(int), 1, sofp) != 1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/

