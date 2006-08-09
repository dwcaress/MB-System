/*--------------------------------------------------------------------
 *    The MB-system:	mbbackangle.c	1/6/95
 *    $Id: mbbackangle.c,v 5.18 2006-08-09 22:41:27 caress Exp $
 *
 *    Copyright (c) 1995, 2000, 2002, 2003, 2004 by
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
 * MBBACKANGLE reads a swath sonar data file and generates a table
 * of the average amplitude or sidescan values as a function of
 * the grazing angle with the seafloor. If bathymetry is
 * not available,  the seafloor is assumed to be flat. The takeoff
 * angle for each beam or pixel arrival is projected to the seafloor;
 * no raytracing is done.
 * The results are dumped to stdout.
 *
 * Author:	D. W. Caress
 * Date:	January 6, 1995
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.17  2006/04/26 22:05:26  caress
 * Changes to handle MBARI Mapping AUV data better.
 *
 * Revision 5.16  2006/04/19 18:29:04  caress
 * Added output of global correction table.
 *
 * Revision 5.15  2006/03/14 02:35:45  caress
 * Altered mbbackangle so that it outputs at least one table, even if there are no survey pings in a file.
 *
 * Revision 5.14  2006/03/14 01:59:24  caress
 * A minor change to mbbackangle to output the status of slope correction (-Q)
 * in the coments in the output file.
 *
 * Revision 5.13  2006/02/01 07:31:06  caress
 * Modifications suggested by Gordon Keith
 *
 * Revision 5.12  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.11  2005/11/05 01:07:54  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.10  2005/08/17 17:28:54  caress
 * Improved how the best altitude value is obtained for sidescan and amplitude data correction.
 *
 * Revision 5.9  2005/03/25 04:43:00  caress
 * Standardized the string lengths used for filenames and comment data.
 *
 * Revision 5.8  2004/10/06 19:10:52  caress
 * Release 5.0.5 update.
 *
 * Revision 5.7  2003/04/17 21:17:10  caress
 * Release 5.0.beta30
 *
 * Revision 5.6  2002/09/19 00:28:12  caress
 * Release 5.0.beta23
 *
 * Revision 5.5  2002/09/07 04:49:23  caress
 * Added slope mode option to mb_process.
 *
 * Revision 5.4  2002/07/25 19:07:17  caress
 * Release 5.0.beta21
 *
 * Revision 5.3  2002/07/20 20:56:55  caress
 * Release 5.0.beta20
 *
 * Revision 5.2  2001/07/20 00:34:38  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/03/22 21:14:16  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.12  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.11  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.10  2000/09/11  20:10:02  caress
 * Linked to new datalist parsing functions. Now supports recursive datalists
 * and comments in datalists.
 *
 * Revision 4.9  2000/06/06  20:32:46  caress
 * Now handles amplitude flagging using beamflags.
 *
 * Revision 4.8  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.7  1997/07/25  14:28:10  caress
 * Version 4.5beta2
 *
 * Revision 4.6  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.5  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.5  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.4  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.3  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1995/03/02  13:49:21  caress
 * Fixed bug related to error messages.
 *
 * Revision 4.1  1995/02/27  14:43:18  caress
 * Fixed bug regarding closing a text input file.
 *
 * Revision 4.0  1995/02/14  21:17:15  caress
 * Version 4.2
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_format.h"
#include "../../include/mb_process.h"

/* GMT include files */
#include "gmt.h"

/* mode defines */
#define	MBBACKANGLE_AMP	1
#define	MBBACKANGLE_SS	2
#define	MBBACKANGLE_INNERSWATHLIMIT 15.0
#define	MBBACKANGLE_BEAMPATTERN_EMPIRICAL		0
#define	MBBACKANGLE_BEAMPATTERN_SIDESCAN		1
int output_table(int verbose, FILE *tfp, int ntable, int nping, double time_d,
	int nangles, double angle_max, double dangle, int symmetry,
	int *nmean, double *mean, double *sigma, 
	int *error);
int output_model(int verbose, FILE *tfp, 
	double ssbeamwidth, double ssdepression, double ref_angle,
	int ntable, int nping, double time_d, double altitude,
	int nangles, double angle_max, double dangle, int symmetry,
	int *nmean, double *mean, double *sigma, 
	int *error);
						
static char rcs_id[] = "$Id: mbbackangle.c,v 5.18 2006-08-09 22:41:27 caress Exp $";
static char program_name[] = "mbbackangle";

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char help_message[] =  
"MBbackangle reads a swath sonar data file and generates \n\t\
a set of tables containing the average sidescan values\n\t\
as a function of the angle of interaction (grazing angle) \n\t\
with the seafloor. Each table represents the symmetrical \n\t\
average function for a user defined number of pings. The \n\t\
tables are output to an \".aga\" file which can be applied \n\t\
by MBprocess.";
	static char usage_message[] = "mbbackangle -Ifile \
[-Akind -Bmode[/beamwidth/depression] -Fformat -Ggridmode/angle/max/nx/ny \
-Nnangles/angle_max -Ppings -Q -Rrefangle -Zaltitude -V -H]";
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
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	swathfile[MB_PATH_MAXLINE];
	char	amptablefile[MB_PATH_MAXLINE];
	char	sstablefile[MB_PATH_MAXLINE];
	char	ampalltablefile[MB_PATH_MAXLINE];
	char	ssalltablefile[MB_PATH_MAXLINE];
	FILE	*atfp;
	FILE	*stfp;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* MBIO read values */
	void	*mbio_ptr = NULL;
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
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[MB_COMMENT_MAXLINE];

	/* slope calculation variables */
	int	nsmooth = 5;
	int	ndepths;
	double	*depths;
	double	*depthsmooth;
	double	*depthacrosstrack;
	int	nslopes;
	double	*slopes;
	double	*slopeacrosstrack;

	/* angle function variables */
	int	amplitude_on = MB_NO;
	int	sidescan_on = MB_NO;
	int	dump = MB_NO;
	int	symmetry = MB_NO;
	int	useslope = MB_NO;
	int	nangles = 81;
	double	angle_max = 80.0;
	double	dangle;
	double	angle_start;
	int	pings_avg = 50;
	int	navg = 0;
	int	ntotavg = 0;
	int	*nmeanamp = NULL;
	double	*meanamp = NULL;
	double	*sigmaamp = NULL;
	int	*nmeanss = NULL;
	double	*meanss = NULL;
	double	*sigmass = NULL;
	int	*nmeantotamp = NULL;
	double	*meantotamp = NULL;
	double	*sigmatotamp = NULL;
	int	*nmeantotss = NULL;
	double	*meantotss = NULL;
	double	*sigmatotss = NULL;
	double	altitude_default = 0.0;
	double	time_d_avg;
	double	altitude_avg;
	double	time_d_totavg;
	double	altitude_totavg;
	int	beammode = MBBACKANGLE_BEAMPATTERN_EMPIRICAL;
	double	ssbeamwidth = 50.0;
	double	ssdepression = 20.0;
	int	amp_corr_type;
	int	amp_corr_symmetry = MBP_SSCORR_SYMMETRIC;
	int	amp_corr_slope = MBP_AMPCORR_IGNORESLOPE;
	int	ss_corr_type;
	int	ss_corr_symmetry = MBP_AMPCORR_SYMMETRIC;
	int	ss_corr_slope = MBP_SSCORR_IGNORESLOPE;
	double	ref_angle;
	double	ref_angle_default = 30.0;
	
	/* amp vs angle grid variables */
	int	gridamp = MB_NO;
	double	gridampangle = 0.0;
	double	gridampmax = 0.0;
	int	gridampnx = 0;
	int	gridampny = 0;
	double	gridampdx = 0.0;
	double	gridampdy = 0.0;
	float	*gridamphist = NULL;
	int	gridss = MB_NO;
	double	gridssangle = 0.0;
	double	gridssmax = 0.0;
	int	gridssnx = 0;
	int	gridssny = 0;
	double	gridssdx = 0.0;
	double	gridssdy = 0.0;
	float	*gridsshist = NULL;
	char	gridfile[MB_PATH_MAXLINE];
	char	*xlabel = "Grazing Angle (degrees)";
	char	*ylabel = "Amplitude";
	char	zlabel[MB_PATH_MAXLINE];
	char	title[MB_PATH_MAXLINE];
	char	plot_cmd[MB_PATH_MAXLINE];
	char	*projection = "GenericLinear";

	int	ampkind;
	int	read_data;
	double	bathy;
	double	altitude_use;
	double	slope;
	double	angle;
	double	ampmax;
	double	norm;
	int	nrec, namp, nss, ntable;
	int	nrectot = 0;
	int	namptot = 0;
	int	nsstot = 0;
	int	ntabletot = 0;
	int	mode;
	int	plot_status;

	/* time, user, host variables */
	time_t	right_now;
	char	date[25], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];

	double	d1, d2;
	int	i, j;
	int	ix, jy, k, n;
	
	char	*ctime();
	char	*getenv();

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* reset pings and timegap */
	pings = 1;
	timegap = 10000000.0;

	/* set default input to stdin */
	strcpy (read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:CcDdF:f:G:g:HhI:i:N:n:P:p:QqR:r:VvZ:z:")) != -1)
	  switch (c) 
		{
		case 'A':
		case 'a':
			sscanf (optarg,"%d", &ampkind);
			if (ampkind == MBBACKANGLE_SS)
				sidescan_on = MB_YES;
			if (ampkind == MBBACKANGLE_AMP)
				amplitude_on = MB_YES;
			flag++;
			break;
		case 'B':
		case 'b':
			n = sscanf (optarg,"%d/%lf/%lf", &beammode, &d1, &d2);
			if (beammode == MBBACKANGLE_BEAMPATTERN_SIDESCAN)
				{
				if (n >= 2)
					ssbeamwidth = d1;
				if (n >= 3)
					ssdepression = d2;
				}
			flag++;
			break;
		case 'C':
		case 'c':
			symmetry = MB_YES;
			flag++;
			break;
		case 'D':
		case 'd':
			dump = MB_YES;
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'G':
		case 'g':
			n = sscanf (optarg,"%d/%lf/%lf/%d/%d", &mode,&angle, &ampmax, &i, &j);
			if (mode == MBBACKANGLE_AMP && n == 5)
				{
				gridamp = MB_YES;
				gridampangle = angle;
				gridampmax = ampmax;
				gridampnx = i;
				gridampny = j;
				gridampdx = 2.0 * gridampangle / (gridampnx - 1);
				gridampdy = gridampmax / (gridampny - 1);
				}
			else if (mode == MBBACKANGLE_SS && n == 5)
				{
				gridss = MB_YES;
				gridssangle = angle;
				gridssmax = ampmax;
				gridssnx = i;
				gridssny = j;
				gridssdx = 2.0 * gridssangle / (gridssnx - 1);
				gridssdy = gridssmax / (gridssny - 1);
				}
			flag++;
			break;
		case 'H':
		case 'h':
			help++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", read_file);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%d/%lf", &nangles, &angle_max);
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf (optarg,"%d", &pings_avg);
			flag++;
			break;
		case 'Q':
		case 'q':
			useslope = MB_YES;
			amp_corr_slope = MBP_AMPCORR_USESLOPE;
			ss_corr_slope = MBP_SSCORR_USESLOPE;
			flag++;
			break;
		case 'R':
		case 'r':
			sscanf (optarg,"%lf", &ref_angle_default);
			flag++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'Z':
		case 'z':
			sscanf (optarg,"%lf", &altitude_default);
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
		
	/* set mode if necessary */
	if (amplitude_on != MB_YES 
		&& sidescan_on != MB_YES)
		{
		amplitude_on = MB_YES;
		sidescan_on = MB_YES;
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:      %d\n",verbose);
		fprintf(stderr,"dbg2       help:         %d\n",help);
		fprintf(stderr,"dbg2       format:       %d\n",format);
		fprintf(stderr,"dbg2       pings:        %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:      %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:    %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:    %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:    %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:    %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:   %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:   %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:   %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:   %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:   %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:   %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:   %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:   %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:   %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:   %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:   %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:   %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:   %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:   %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:     %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:      %f\n",timegap);
		fprintf(stderr,"dbg2       read_file:    %s\n",read_file);
		fprintf(stderr,"dbg2       dump:         %d\n",dump);
		fprintf(stderr,"dbg2       symmetry:     %d\n",symmetry);
		fprintf(stderr,"dbg2       amplitude_on: %d\n",amplitude_on);
		fprintf(stderr,"dbg2       sidescan_on:  %d\n",sidescan_on);
		fprintf(stderr,"dbg2       useslope:     %d\n",useslope);
		fprintf(stderr,"dbg2       nangles:      %d\n",nangles);
		fprintf(stderr,"dbg2       angle_max:    %f\n",angle_max);
		fprintf(stderr,"dbg2       ref_angle:    %f\n",ref_angle_default);
		fprintf(stderr,"dbg2       beammode:     %d\n",beammode);
		fprintf(stderr,"dbg2       ssbeamwidth:  %f\n",ssbeamwidth);
		fprintf(stderr,"dbg2       ssdepression: %f\n",ssdepression);
		fprintf(stderr,"dbg2       ref_angle:    %f\n",ref_angle_default);
		fprintf(stderr,"dbg2       pings_avg:    %d\n",pings_avg);
		fprintf(stderr,"dbg2       angle_max:    %f\n",angle_max);
		fprintf(stderr,"dbg2       altitude:     %f\n",altitude_default);
		fprintf(stderr,"dbg2       gridamp:      %d\n",gridamp);
		fprintf(stderr,"dbg2       gridampangle: %f\n",gridampangle);
		fprintf(stderr,"dbg2       gridampmax:   %f\n",gridampmax);
		fprintf(stderr,"dbg2       gridampnx:    %d\n",gridampnx);
		fprintf(stderr,"dbg2       gridampny:    %d\n",gridampny);
		fprintf(stderr,"dbg2       gridampdx:    %d\n",gridampdx);
		fprintf(stderr,"dbg2       gridampdy:    %d\n",gridampdy);
		fprintf(stderr,"dbg2       gridss:       %d\n",gridss);
		fprintf(stderr,"dbg2       gridssangle:  %f\n",gridssangle);
		fprintf(stderr,"dbg2       gridssmax:    %f\n",gridssmax);
		fprintf(stderr,"dbg2       gridssnx:     %d\n",gridssnx);
		fprintf(stderr,"dbg2       gridssny:     %d\n",gridssny);
		fprintf(stderr,"dbg2       gridssdx:     %d\n",gridssdx);
		fprintf(stderr,"dbg2       gridssdy:     %d\n",gridssdy);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* allocate memory for angle arrays */
	if (amplitude_on == MB_YES)
		{
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(int),
				&nmeanamp,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(double),
				&meanamp,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(double),
				&sigmaamp,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(int),
				&nmeantotamp,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(double),
				&meantotamp,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(double),
				&sigmatotamp,&error);
		}
	if (sidescan_on == MB_YES)
		{
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(int),
				&nmeanss,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(double),
				&meanss,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(double),
				&sigmass,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(int),
				&nmeantotss,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(double),
				&meantotss,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(double),
				&sigmatotss,&error);
		}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating angle arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* check grid modes */
	if (gridamp == MB_YES && amplitude_on == MB_NO)
		gridamp = MB_NO;
	if (gridss == MB_YES && sidescan_on == MB_NO)
		gridss = MB_NO;

	/* output some information */
	if (verbose > 0)
		{
		fprintf(stderr, "\nPings to average:    %d\n", pings_avg);
		fprintf(stderr, "Number of angle bins: %d\n", nangles);
		fprintf(stderr, "Maximum angle:         %f\n", angle_max);
		fprintf(stderr, "Default altitude:      %f\n", altitude_default);
		if (amplitude_on == MB_YES)
			fprintf(stderr, "Working on beam amplitude data...\n");
		if (sidescan_on == MB_YES)
			fprintf(stderr, "Working on sidescan data...\n");
		if (beammode == MBBACKANGLE_BEAMPATTERN_EMPIRICAL)
			fprintf(stderr, "Generating empirical correction tables...\n");
		else if (beammode == MBBACKANGLE_BEAMPATTERN_SIDESCAN)
			fprintf(stderr, "Generating sidescan model correction tables...\n");
		if (useslope == MB_YES)
			fprintf(stderr, "Using seafloor slope in calculating correction tables...\n");
		else
			fprintf(stderr, "Using flat bottom assumption in calculating correction tables...\n");
		if (gridamp == MB_YES)
			fprintf(stderr, "Outputting gridded histograms of beam amplitude vs grazing angle...\n");
		if (gridss == MB_YES)
			fprintf(stderr, "Outputting gridded histograms of sidescan amplitude vs grazing angle...\n");
		}

	/* get size of bins */
	dangle = 2 * angle_max / (nangles-1);
	angle_start = -angle_max - 0.5*dangle;

	/* initialize histogram */
	if (amplitude_on == MB_YES)
	for (i=0;i<nangles;i++)
		{
		nmeanamp[i] = 0;
		meanamp[i] = 0.0;
		sigmaamp[i] = 0.0;
		nmeantotamp[i] = 0;
		meantotamp[i] = 0.0;
		sigmatotamp[i] = 0.0;
		}
	if (sidescan_on == MB_YES)
	for (i=0;i<nangles;i++)
		{
		nmeanss[i] = 0;
		meanss[i] = 0.0;
		sigmass[i] = 0.0;
		nmeantotss[i] = 0;
		meantotss[i] = 0.0;
		sigmatotss[i] = 0.0;
		}

	/* initialize counting variables */
	ntotavg = 0;
	time_d_totavg = 0.0;
	altitude_totavg = 0.0;

	/* initialize grids */
	if (gridamp == MB_YES)
		{
		/* allocate memory for output grids */
		status = mb_malloc(verbose, gridampnx * gridampny * sizeof(float), &gridamphist, &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
				message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}
		}
	if (gridss == MB_YES)
		{
		/* allocate memory for output grids */
		status = mb_malloc(verbose, gridssnx * gridssny * sizeof(float), &gridsshist, &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
				message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}
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

	/* obtain format array location - format id will 
		be aliased to current id if old format id given */
	status = mb_format(verbose,&format,&error);

	/* initialize reading the swath sonar file */
	if ((status = mb_read_init(
		verbose,swathfile,format,1,lonflip,bounds,
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
		
	/* set correction modes according to format */
	if (format == MBF_SB2100RW
	    || format == MBF_SB2100B1
	    || format == MBF_SB2100B2
	    || format == MBF_EDGJSTAR
	    || format == MBF_EDGJSTR2)
	    ss_corr_type = MBP_SSCORR_DIVISION;
	else
	    ss_corr_type = MBP_SSCORR_SUBTRACTION;
	amp_corr_type = MBP_AMPCORR_SUBTRACTION;
	ref_angle = ref_angle_default;
	
	/* allocate memory for data arrays */
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,beams_bath*sizeof(char),
					&beamflag,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,beams_bath*sizeof(double),
					&bath,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,beams_amp*sizeof(double),
					&amp,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,beams_bath*sizeof(double),
					&bathacrosstrack,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,beams_bath*sizeof(double),
					&bathalongtrack,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
					&ss,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
					&ssacrosstrack,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
					&ssalongtrack,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,beams_bath*sizeof(double),
					&depths,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,beams_bath*sizeof(double),
					&depthsmooth,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,beams_bath*sizeof(double),
					&depthacrosstrack,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,(beams_bath+1)*sizeof(double),
					&slopes,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,(beams_bath+1)*sizeof(double),
					&slopeacrosstrack,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(char), (void **)&beamflag, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bath, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
						sizeof(double), (void **)&amp, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ss, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ssacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ssalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&depths, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&depthsmooth, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&depthacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&slopes, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						2 * sizeof(double), (void **)&slopeacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						2 * sizeof(double), (void **)&bathalongtrack, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* output information */
	if (error == MB_ERROR_NO_ERROR && verbose > 0)
	    {
	    fprintf(stderr, "\nprocessing swath file: %s %d\n", swathfile, format);
	    }

	/* initialize grid arrays */
	if (error == MB_ERROR_NO_ERROR)
	    {
	    if (gridamp == MB_YES)
		{
		/* initialize the memory */
		for (i=0;i<gridampnx*gridampny;i++)
			{
			gridamphist[i] = 0.0;
			}
		}			
	    if (gridss == MB_YES)
		{
		/* initialize the memory */
		for (i=0;i<gridssnx*gridssny;i++)
			{
			gridsshist[i] = 0.0;
			}
		}
	    }

	/* open output files */
	if (error == MB_ERROR_NO_ERROR
		&& dump == MB_YES)
	    {
	    atfp = stdout;
	    stfp = stdout;
	    }
	else if (error == MB_ERROR_NO_ERROR)
	    {
	    if (amplitude_on == MB_YES)
	    	{
	    	strcpy(amptablefile,swathfile);
 	    	strcat(amptablefile,".aga");
	    	if ((atfp = fopen(amptablefile,"w")) == NULL)
		    {
		    error = MB_ERROR_OPEN_FAIL;
		    mb_error(verbose,error,&message);
		    fprintf(stderr, "\nUnable to open output table file %s\n", amptablefile);
		    fprintf(stderr, "Program %s aborted!\n", program_name);
		    exit(error);
		    }
		}
	    if (sidescan_on == MB_YES)
	    	{
	    	strcpy(sstablefile,swathfile);
 	    	strcat(sstablefile,".sga");
	    	if ((stfp = fopen(sstablefile,"w")) == NULL)
		    {
		    error = MB_ERROR_OPEN_FAIL;
		    mb_error(verbose,error,&message);
		    fprintf(stderr, "\nUnable to open output table file %s\n", sstablefile);
		    fprintf(stderr, "Program %s aborted!\n", program_name);
		    exit(error);
		    }
		}
	    }

	/* write to output file */
	if (error == MB_ERROR_NO_ERROR)
	    {
	    /* set comments in table files */
	    if (amplitude_on == MB_YES)
	    	{
		fprintf(atfp,"## Amplitude correction table files generated by program %s\n",program_name);
	    	fprintf(atfp,"## Version %s\n",rcs_id);
	    	fprintf(atfp,"## MB-system Version %s\n",MB_VERSION);
	    	fprintf(atfp,"## Table file format: 1.0.0\n");
	    	right_now = time((time_t *)0);
	    	strncpy(date,ctime(&right_now),24);
	    	if ((user_ptr = getenv("USER")) == NULL)
			user_ptr = getenv("LOGNAME");
	    	if (user_ptr != NULL)
			strcpy(user,user_ptr);
	    	else
			strcpy(user, "unknown");
	    	gethostname(host,MB_PATH_MAXLINE);
	    	fprintf(atfp,"## Run by user <%s> on cpu <%s> at <%s>\n",
			user,host,date);
	    	fprintf(atfp, "## Input swath file:      %s\n", swathfile);
	    	fprintf(atfp, "## Input swath format:    %d\n", format);
	    	fprintf(atfp, "## Output table file:     %s\n", amptablefile);
	    	fprintf(atfp, "## Pings to average:      %d\n", pings_avg);
	    	fprintf(atfp, "## Number of angle bins:  %d\n", nangles);
	    	fprintf(atfp, "## Maximum angle:         %f\n", angle_max);
	   	fprintf(atfp, "## Default altitude:      %f\n", altitude_default);
		fprintf(atfp, "## Slope correction:      %d\n", amp_corr_slope);
	   	fprintf(atfp, "## Data type:             beam amplitude\n");
		}

	    if (sidescan_on == MB_YES)
	    	{
		fprintf(stfp,"## Sidescan correction table files generated by program %s\n",program_name);
	    	fprintf(stfp,"## Version %s\n",rcs_id);
	    	fprintf(stfp,"## MB-system Version %s\n",MB_VERSION);
	    	fprintf(stfp,"## Table file format: 1.0.0\n");
	    	right_now = time((time_t *)0);
	    	strncpy(date,ctime(&right_now),24);
	    	if ((user_ptr = getenv("USER")) == NULL)
			user_ptr = getenv("LOGNAME");
	    	if (user_ptr != NULL)
			strcpy(user,user_ptr);
	    	else
			strcpy(user, "unknown");
	    	gethostname(host,MB_PATH_MAXLINE);
	    	fprintf(stfp,"## Run by user <%s> on cpu <%s> at <%s>\n",
			user,host,date);
	    	fprintf(stfp, "## Input swath file:      %s\n", swathfile);
	    	fprintf(stfp, "## Input swath format:    %d\n", format);
	    	fprintf(stfp, "## Output table file:     %s\n", sstablefile);
	    	fprintf(stfp, "## Pings to average:      %d\n", pings_avg);
	    	fprintf(stfp, "## Number of angle bins:  %d\n", nangles);
	    	fprintf(stfp, "## Maximum angle:         %f\n", angle_max);
	   	fprintf(stfp, "## Default altitude:      %f\n", altitude_default);
		fprintf(stfp, "## Slope Correction:      %d\n", ss_corr_slope);
	   	fprintf(stfp, "## Data type:             sidescan\n");
		}
	    }

	/* initialize counting variables */
	nrec = 0;
	namp = 0;
	nss = 0;
	navg = 0;
	ntable = 0;

	/* read and process data */
	while (error <= MB_ERROR_NO_ERROR)
		{

		/* read a ping of data */
		status = mb_get(verbose,mbio_ptr,&kind,&pings,
				time_i,&time_d,
				&navlon,&navlat,
				&speed,&heading,
				&distance,&altitude,&sonardepth,
				&beams_bath,&beams_amp,&pixels_ss,
				beamflag,bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,&error);

		if ((navg > 0
			&& (error == MB_ERROR_TIME_GAP
				|| error == MB_ERROR_EOF))
			|| (navg >= pings_avg)
			|| (navg == 0 && error == MB_ERROR_EOF))
			{
			/* write out tables */
			time_d_avg /= navg;
			altitude_avg /= navg;
			if (beammode == MBBACKANGLE_BEAMPATTERN_EMPIRICAL)
				{
				if (amplitude_on == MB_YES)
				output_table(verbose, atfp, ntable, navg, time_d_avg, 
						nangles, angle_max, dangle, symmetry,
						nmeanamp, meanamp, sigmaamp, &error);
				if (sidescan_on == MB_YES)
				output_table(verbose, stfp, ntable, navg, time_d_avg, 
						nangles, angle_max, dangle, symmetry,
						nmeanss, meanss, sigmass, &error);
				}
			else if (beammode == MBBACKANGLE_BEAMPATTERN_SIDESCAN)
				{
				if (amplitude_on == MB_YES)
				output_model(verbose, atfp, ssbeamwidth, ssdepression, ref_angle,
						ntable, navg, time_d_avg, altitude_avg,
						nangles, angle_max, dangle, symmetry,
						nmeanamp, meanamp, sigmaamp, &error);
				if (sidescan_on == MB_YES)
				output_model(verbose, stfp, ssbeamwidth, ssdepression, ref_angle,
						ntable, navg, time_d_avg, altitude_avg,
						nangles, angle_max, dangle, symmetry,
						nmeanss, meanss, sigmass, &error);
				}
			ntable++;
					
			/* reinitialize arrays */
			navg = 0;
			time_d_avg = 0.0;
			altitude_avg = 0.0;
			if (amplitude_on == MB_YES)
			for (i=0;i<nangles;i++)
				{
				nmeanamp[i] = 0;
				meanamp[i] = 0.0;
				sigmaamp[i] = 0.0;
				}
			if (sidescan_on == MB_YES)
			for (i=0;i<nangles;i++)
				{
				nmeanss[i] = 0;
				meanss[i] = 0.0;
				sigmass[i] = 0.0;
				}
			}


		/* process the pings */
		if (error == MB_ERROR_NO_ERROR 
		    || error == MB_ERROR_TIME_GAP)
		    {
		    /* increment record counter */
		    nrec++;
		    navg++;
		    ntotavg++;
		    
		    /* increment time */
		    time_d_avg += time_d;
		    altitude_avg += altitude;
		    time_d_totavg += time_d;
		    altitude_totavg += altitude;

		    /* get the seafloor slopes */
		    if (beams_bath > 0)
			mb_pr_set_bathyslope(verbose,nsmooth,
				beams_bath,beamflag,bath,bathacrosstrack,
				&ndepths,depths,depthacrosstrack,
				&nslopes,slopes,slopeacrosstrack,
				depthsmooth,
				&error);
		    /* do the amplitude */
		    if (amplitude_on == MB_YES)
		    for (i=0;i<beams_amp;i++)
			{
			if (mb_beam_ok(beamflag[i]))
			    {
			    namp++;
			    if (beams_bath != beams_amp)
				{
				if (altitude > 0.0)
				    bathy = altitude + sonardepth;
				else
				    bathy = altitude_default + sonardepth;
				slope = 0.0;
				}
			    else
				{
				status = mb_pr_get_bathyslope(verbose,
				    ndepths,depths,depthacrosstrack,
				    nslopes,slopes,slopeacrosstrack,
				    bathacrosstrack[i],
				    &bathy,&slope,&error);
				if (status != MB_SUCCESS)
				    {
				    if (altitude > 0.0)
				    	bathy = altitude + sonardepth;
				    else
				    	bathy = altitude_default + sonardepth;
				    slope = 0.0;
				    status = MB_SUCCESS;
				    error = MB_ERROR_NO_ERROR;
				    }
				}
			    if (bathy > 0.0)
				{
				altitude_use = bathy - sonardepth;
				angle = RTD * atan(bathacrosstrack[i] / altitude_use);
				if (useslope == MB_YES)
					angle += RTD * atan(slope);
					
				/* load amplitude into table */
				j = (angle - angle_start)/dangle;
				if (j >= 0 && j < nangles)
				    {
				    meanamp[j] += amp[i];
				    sigmaamp[j] += amp[i]*amp[i];
				    nmeanamp[j]++;
				    meantotamp[j] += amp[i];
				    sigmatotamp[j] += amp[i]*amp[i];
				    nmeantotamp[j]++;
				    }
					
				/* load amplitude into grid */
				if (gridamp == MB_YES)
				    {
				    ix = (angle + gridampangle) / gridampdx;
				    jy = amp[i] / gridampdy;
				    if (ix >= 0 && ix < gridampnx && jy >= 0 && jy < gridampny)
					{
					k = ix * gridampny + jy;
					gridamphist[k] += 1.0;
					}
				    }
				}

			    /* print debug statements */
			    if (verbose >= 5)
				{
				fprintf(stderr,"dbg5       %d %d: slope:%f altitude:%f xtrack:%f ang:%f j:%d\n",
				    nrec, i, slope, altitude_use, bathacrosstrack[i], angle, j);
				}
			    }
			}

		    /* do the sidescan */
		    if (sidescan_on == MB_YES)
		    for (i=0;i<pixels_ss;i++)
			{
			if (ss[i] > 0.0)
			    {
			    nss++;
			    if (beams_bath > 0)
				{
				status = mb_pr_get_bathyslope(verbose,
				    ndepths,depths,depthacrosstrack,
				    nslopes,slopes,slopeacrosstrack,
				    ssacrosstrack[i],
				    &bathy,&slope,&error);
				if (status != MB_SUCCESS
					|| bathy <= 0.0)
				    {
				    if (altitude > 0.0)
				    	bathy = altitude + sonardepth;
				    else
				    	bathy = altitude_default;
				    slope = 0.0;
				    status = MB_SUCCESS;
				    error = MB_ERROR_NO_ERROR;
				    }
				}
			    else
				{
				if (altitude > 0.0)
				    	bathy = altitude + sonardepth;
				else
				    	bathy = altitude_default;
				slope = 0.0;
				}
			    if (bathy > 0.0)
				{
				altitude_use = bathy - sonardepth;
				angle = RTD * atan(ssacrosstrack[i] / altitude_use);
/*if (altitude > 0.0) fprintf(stderr,"time_d:%f i:%d xtrack:%f altitude:%f angle:%f\n",
time_d, i, ssacrosstrack[i], altitude_use, angle);*/
				if (useslope == MB_YES)
					{
/*fprintf(stderr,"SLOPECALC: time_d:%f i:%d angle:%f ",time_d,i,angle);*/
					angle += RTD * atan(slope);
/*fprintf(stderr,"slope:%f slopeangle:%f angle:%f\n",slope,RTD * atan(slope),angle);*/
					}
					
				/* load amplitude into table */
				j = (angle - angle_start)/dangle;
				if (j >= 0 && j < nangles)
				    {
				    meanss[j] += ss[i];
				    sigmass[j] += ss[i]*ss[i];
				    nmeanss[j]++;
				    meantotss[j] += ss[i];
				    sigmatotss[j] += ss[i]*ss[i];
				    nmeantotss[j]++;
				    }
					
				/* load amplitude into grid */
				if (gridss == MB_YES)
				    {
				    ix = (angle + gridssangle) / gridssdx;
				    jy = ss[i] / gridssdy;
				    if (ix >= 0 && ix < gridssnx && jy >= 0 && jy < gridssny)
					{
					k = ix * gridssny + jy;
					gridsshist[k] += 1.0;
					}
				    }
				}

			    /* print debug statements */
			    if (verbose >= 5)
				{
				fprintf(stderr,"dbg5kkk       %d %d: slope:%f altitude:%f xtrack:%f ang:%f j:%d\n",
				    nrec, i, slope, altitude_use, ssacrosstrack[i], angle, j);
				}
			    }
			}

		    }
		}

	/* close the swath sonar file */
	status = mb_close(verbose,&mbio_ptr,&error);
	if (dump == MB_NO && amplitude_on == MB_YES)
		fclose(atfp);
	if (dump == MB_NO && sidescan_on == MB_YES)
		fclose(stfp);
	ntabletot += ntable;
	nrectot += nrec;
	namptot += namp;
	nsstot += nss;
		
	/* output grids */
	if (gridamp == MB_YES)
		{
		/* normalize the grid */
		ampmax = 0.0;
		for (ix=0;ix<gridampnx;ix++)
			{
			norm = 0.0;
			for (jy=0;jy<gridampny;jy++)
				{
				k = ix * gridampny + jy;
				norm += gridamphist[k];
				}
			if (norm > 0.0)
				{
				norm *= 0.001;
				for (jy=0;jy<gridampny;jy++)
					{
					k = ix * gridampny + jy;
					gridamphist[k] /= norm;
					ampmax = MAX(ampmax, gridamphist[k]);
					}
				}
			}
		
		/* set the strings */
		strcpy(gridfile, swathfile);
 	    	strcat(gridfile,"_aga.grd");
		strcpy(zlabel, "Beam Amplitude PDF (X1000)");
		strcpy(title, "Beam Amplitude vs. Grazing Angle PDF");
		
		/* output the grid */
		write_cdfgrd(verbose, gridfile, gridamphist,
				gridampnx, gridampny, 
				(double)(-gridampangle), gridampangle, 
				(double)0.0, gridampmax,
				(double)0.0, ampmax,
				gridampdx, gridampdy,
				xlabel, ylabel, zlabel, title, projection,
				argc, argv, &error);
	
		/* run mbm_grdplot */
		sprintf(plot_cmd, "mbm_grdplot -I%s -JX9/5 -G1 -S -MGQ100 -MXM# -MXI%s -V -L\"File %s - %s:%s\"", 
				gridfile, amptablefile, gridfile, title, zlabel);
		if (verbose)
			{
			fprintf(stderr, "\nexecuting mbm_grdplot...\n%s\n", 
				plot_cmd);
			}
		plot_status = system(plot_cmd);
		if (plot_status == -1)
			{
			fprintf(stderr, "\nError executing mbm_grdplot on grid file %s\n", gridfile);
			}
		}
	if (gridss == MB_YES)
		{
		/* normalize the grid */
		ampmax = 0.0;
		for (ix=0;ix<gridssnx;ix++)
			{
			norm = 0.0;
			for (jy=0;jy<gridssny;jy++)
				{
				k = ix * gridssny + jy;
				norm += gridsshist[k];
				}
			if (norm > 0.0)
				{
				norm *= 0.001;
				for (jy=0;jy<gridssny;jy++)
					{
					k = ix * gridssny + jy;
					gridsshist[k] /= norm;
					ampmax = MAX(ampmax, gridsshist[k]);
					}
				}
			}
		
		/* set the strings */
		strcpy(gridfile, swathfile);
 	    	strcat(gridfile,"_sga.grd");
		strcpy(zlabel, "Sidescan Amplitude PDF (X1000)");
		strcpy(title, "Sidescan Amplitude vs. Grazing Angle PDF");
		
		/* output the grid */
		write_cdfgrd(verbose, gridfile, gridsshist,
				gridssnx, gridssny, 
				(double)(-gridssangle), gridssangle, 
				(double)0.0, gridssmax,
				(double)0.0, ampmax,
				gridssdx, gridssdy,
				xlabel, ylabel, zlabel, title, projection,
				argc, argv, &error);
	
		/* run mbm_grdplot */
		sprintf(plot_cmd, "mbm_grdplot -I%s -JX9/5 -G1 -S -MGQ100 -MXM# -MXI%s -V -L\"File %s - %s:%s\"", 
				gridfile, sstablefile, gridfile, title, zlabel);
		if (verbose)
			{
			fprintf(stderr, "\nexecuting mbm_grdplot...\n%s\n", 
				plot_cmd);
			}
		plot_status = system(plot_cmd);
		if (plot_status == -1)
			{
			fprintf(stderr, "\nError executing mbm_grdplot on grid file %s\n", gridfile);
			}
		}

	/* set amplitude correction in parameter file */
	if (amplitude_on == MB_YES)
		status = mb_pr_update_ampcorr(verbose, swathfile, 
			MB_YES, amptablefile, 
			amp_corr_type, amp_corr_symmetry, ref_angle, amp_corr_slope, 
			&error);

	/* set sidescan correction in parameter file */
	if (sidescan_on == MB_YES)
		status = mb_pr_update_sscorr(verbose, swathfile, 
			MB_YES, sstablefile, 
			ss_corr_type, ss_corr_symmetry, ref_angle, ss_corr_slope, 
			&error);

	/* output information */
	if (error == MB_ERROR_NO_ERROR && verbose > 0)
	    {
	    fprintf(stderr, "%d records processed\n", nrec);
	    if (amplitude_on == MB_YES)
		{
		fprintf(stderr, "%d amplitude data processed\n", namp);
	    	fprintf(stderr, "%d tables written to %s\n", ntable, amptablefile);
		}
	    if (sidescan_on == MB_YES)
		{
		fprintf(stderr, "%d sidescan data processed\n", nss);
	    	fprintf(stderr, "%d tables written to %s\n", ntable, sstablefile);
		}
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

	/* write out total tables */
	time_d_totavg /= ntotavg;
	altitude_totavg /= ntotavg;
	if (dump == MB_NO && amplitude_on == MB_YES)
	    {
	    strcpy(amptablefile,read_file);
 	    strcat(amptablefile,"_tot.aga");
	    if ((atfp = fopen(amptablefile,"w")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		mb_error(verbose,error,&message);
		fprintf(stderr, "\nUnable to open output table file %s\n", amptablefile);
		fprintf(stderr, "Program %s aborted!\n", program_name);
		exit(error);
		}
	    fprintf(atfp,"## Amplitude correction table files generated by program %s\n",program_name);
	    fprintf(atfp,"## Version %s\n",rcs_id);
	    fprintf(atfp,"## MB-system Version %s\n",MB_VERSION);
	    fprintf(atfp,"## Table file format: 1.0.0\n");
	    right_now = time((time_t *)0);
	    strncpy(date,ctime(&right_now),24);
	    if ((user_ptr = getenv("USER")) == NULL)
		    user_ptr = getenv("LOGNAME");
	    if (user_ptr != NULL)
		    strcpy(user,user_ptr);
	    else
		    strcpy(user, "unknown");
	    gethostname(host,MB_PATH_MAXLINE);
	    fprintf(atfp,"## Run by user <%s> on cpu <%s> at <%s>\n",
		    user,host,date);
	    fprintf(atfp, "## Input file:            %s\n", read_file);
	    fprintf(atfp, "## Output table file:     %s\n", amptablefile);
	    fprintf(atfp, "## Pings to average:      %d\n", pings_avg);
	    fprintf(atfp, "## Number of angle bins:  %d\n", nangles);
	    fprintf(atfp, "## Maximum angle:         %f\n", angle_max);
	    fprintf(atfp, "## Default altitude:      %f\n", altitude_default);
	    fprintf(atfp, "## Slope correction:      %d\n", amp_corr_slope);
	    fprintf(atfp, "## Data type:             beam amplitude\n");
	    if (beammode == MBBACKANGLE_BEAMPATTERN_EMPIRICAL)
		    output_table(verbose, atfp, 0, ntotavg, time_d_totavg, 
				    nangles, angle_max, dangle, symmetry,
				    nmeanamp, meanamp, sigmaamp, &error);
	    else if (beammode == MBBACKANGLE_BEAMPATTERN_SIDESCAN)
		    output_model(verbose, atfp, ssbeamwidth, ssdepression, ref_angle,
				    0, ntotavg, time_d_totavg, altitude_totavg,
				    nangles, angle_max, dangle, symmetry,
				    nmeantotamp, meantotamp, sigmatotamp, &error);
	    fclose(atfp);
	    }
	if (dump == MB_NO && sidescan_on == MB_YES)
	    {
	    strcpy(sstablefile,read_file);
 	    strcat(sstablefile,"_tot.sga");
	    if ((stfp = fopen(sstablefile,"w")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		mb_error(verbose,error,&message);
		fprintf(stderr, "\nUnable to open output table file %s\n", sstablefile);
		fprintf(stderr, "Program %s aborted!\n", program_name);
		exit(error);
		}
	    fprintf(stfp,"## Sidescan correction table files generated by program %s\n",program_name);
	    fprintf(stfp,"## Version %s\n",rcs_id);
	    fprintf(stfp,"## MB-system Version %s\n",MB_VERSION);
	    fprintf(stfp,"## Table file format: 1.0.0\n");
	    right_now = time((time_t *)0);
	    strncpy(date,ctime(&right_now),24);
	    if ((user_ptr = getenv("USER")) == NULL)
		    user_ptr = getenv("LOGNAME");
	    if (user_ptr != NULL)
		    strcpy(user,user_ptr);
	    else
		    strcpy(user, "unknown");
	    gethostname(host,MB_PATH_MAXLINE);
	    fprintf(stfp,"## Run by user <%s> on cpu <%s> at <%s>\n",
		    user,host,date);
	    fprintf(stfp, "## Input file:            %s\n", read_file);
	    fprintf(stfp, "## Output table file:     %s\n", sstablefile);
	    fprintf(stfp, "## Pings to average:      %d\n", pings_avg);
	    fprintf(stfp, "## Number of angle bins:  %d\n", nangles);
	    fprintf(stfp, "## Maximum angle:         %f\n", angle_max);
	    fprintf(stfp, "## Default altitude:      %f\n", altitude_default);
	    fprintf(stfp, "## Slope Correction:      %d\n", ss_corr_slope);
	    fprintf(stfp, "## Data type:             sidescan\n");
	    if (beammode == MBBACKANGLE_BEAMPATTERN_EMPIRICAL)
		    output_table(verbose, stfp, 0, ntotavg, time_d_totavg, 
				    nangles, angle_max, dangle, symmetry,
				    nmeantotss, meantotss, sigmatotss, &error);
	    else if (beammode == MBBACKANGLE_BEAMPATTERN_SIDESCAN)
		    output_model(verbose, stfp, ssbeamwidth, ssdepression, ref_angle,
				    0, ntotavg, time_d_totavg, altitude_totavg,
				    nangles, angle_max, dangle, symmetry,
				    nmeantotss, meantotss, sigmatotss, &error);
	    fclose(stfp);
	    }

	/* output information */
	if (error == MB_ERROR_NO_ERROR && verbose > 0)
	    {
	    fprintf(stderr, "\n%d total records processed\n", nrectot);
	    if (amplitude_on == MB_YES)
		{
		fprintf(stderr, "%d total amplitude data processed\n", namptot);
	    	fprintf(stderr, "%d total aga tables written\n", ntabletot);
		}
	    if (sidescan_on == MB_YES)
		{
		fprintf(stderr, "%d total sidescan data processed\n", nsstot);
	    	fprintf(stderr, "%d total sga tables written\n", ntabletot);
		}
	    }

	/* deallocate memory used for data arrays */
	if (amplitude_on == MB_YES)
		{
		mb_free(verbose,&nmeanamp,&error);
		mb_free(verbose,&meanamp,&error);
		mb_free(verbose,&sigmaamp,&error);
		mb_free(verbose,&nmeantotamp,&error);
		mb_free(verbose,&meantotamp,&error);
		mb_free(verbose,&sigmatotamp,&error);
		if (gridamp == MB_YES)
			{
			mb_free(verbose,&gridamphist,&error);
			}
		}
	if (sidescan_on == MB_YES)
		{
		mb_free(verbose,&nmeanss,&error);
		mb_free(verbose,&meanss,&error);
		mb_free(verbose,&sigmass,&error);
		mb_free(verbose,&nmeantotss,&error);
		mb_free(verbose,&meantotss,&error);
		mb_free(verbose,&sigmatotss,&error);
		if (gridss == MB_YES)
			{
			mb_free(verbose,&gridsshist,&error);
			}
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
	if (verbose > 0)
		fprintf(stderr,"\n");
	exit(error);
}
/*--------------------------------------------------------------------*/
int output_table(int verbose, FILE *tfp, int ntable, int nping, double time_d,
	int nangles, double angle_max, double dangle, int symmetry, 
	int *nmean, double *mean, double *sigma, 
	int *error)
{
	char	*function_name = "output_table";
	int	status = MB_SUCCESS;
	double	angle, amean, asigma, sum, sumsq, sumn;
	int	time_i[7];
	int	ii, jj, i0, i1;
	int	i, j;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBACKANGLE function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n", verbose);
		fprintf(stderr,"dbg2       tfp:             %d\n", tfp);
		fprintf(stderr,"dbg2       ntable:          %d\n", ntable);
		fprintf(stderr,"dbg2       nping:           %d\n", nping);
		fprintf(stderr,"dbg2       time_d:          %f\n", time_d);
		fprintf(stderr,"dbg2       nangles:         %d\n", nangles);
		fprintf(stderr,"dbg2       angle_max:       %f\n", angle_max);
		fprintf(stderr,"dbg2       dangle:          %f\n", dangle);
		fprintf(stderr,"dbg2       symmetry:        %d\n", symmetry);
		fprintf(stderr,"dbg2       mean and sigma:\n");
		for (i=0;i<nangles;i++)
			fprintf(stderr,"dbg2         %d %f %d %f %f\n", 
				i, (i * dangle), nmean[i], mean[i], sigma[i]);
		}

	/* process sums and print out results */
	mb_get_date(verbose, time_d, time_i, error);
	fprintf(tfp,"# table: %d\n", ntable);
	fprintf(tfp,"# nping: %d\n", nping);
	fprintf(tfp,"# time:  %4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d    %16.6f\n",
		time_i[0],time_i[1],time_i[2],
		time_i[3],time_i[4],time_i[5],
		time_i[6],time_d);
	fprintf(tfp,"# nangles: %d\n", nangles);
	for (i=0;i<nangles;i++)
		{
		amean = 0.0;
		asigma = 0.0;
		sum = 0.0;
		sumsq = 0.0;
		sumn = 0.0;
		angle = -angle_max + i * dangle;
		if (fabs(angle) > MBBACKANGLE_INNERSWATHLIMIT)
			{
			i0 = MAX(i - 1, 0);
			i1 = MIN(i + 1, nangles - 1);
			}
		else
			{
			i0 = i;
			i1 = i;
			}
		for (ii=i0;ii<=i1;ii++)
			{
			sum += mean[ii];
			sumsq += sigma[ii];
			sumn += nmean[ii];
			if (symmetry == MB_YES)
				{
				jj = nangles - ii - 1;
				sum += mean[jj];
				sumsq += sigma[jj];
				sumn += nmean[jj];
				}
			}
		if (sumn > 0.0)
			{
			amean = sum /  sumn;
			asigma = sqrt((sumsq / sumn) - amean * amean);
			}
		fprintf(tfp,"%7.4f %12.4f %12.4f\n", angle, amean, asigma);
		}
	fprintf(tfp,"#\n");
	fprintf(tfp,"#\n");

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBACKANGLE function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int output_model(int verbose, FILE *tfp, 
	double beamwidth, double depression, double ref_angle,
	int ntable, int nping, double time_d, double altitude,
	int nangles, double angle_max, double dangle, int symmetry,
	int *nmean, double *mean, double *sigma, 
	int *error)
{
	char	*function_name = "output_model";
	int	status = MB_SUCCESS;
	double	ref_amp, range, del, factor, aa;
	double	angle, amean, asigma, sum, sumsq, sumn;
	int	time_i[7];
	int	ii, jj, i0, i1, iref;
	int	i, j;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBACKANGLE function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n", verbose);
		fprintf(stderr,"dbg2       tfp:             %d\n", tfp);
		fprintf(stderr,"dbg2       beamwidth:       %f\n", beamwidth);
		fprintf(stderr,"dbg2       depression:      %f\n", depression);
		fprintf(stderr,"dbg2       ref_angle:       %f\n", ref_angle);
		fprintf(stderr,"dbg2       ntable:          %d\n", ntable);
		fprintf(stderr,"dbg2       nping:           %d\n", nping);
		fprintf(stderr,"dbg2       time_d:          %f\n", time_d);
		fprintf(stderr,"dbg2       altitude:        %f\n", altitude);
		fprintf(stderr,"dbg2       nangles:         %d\n", nangles);
		fprintf(stderr,"dbg2       angle_max:       %f\n", angle_max);
		fprintf(stderr,"dbg2       dangle:          %f\n", dangle);
		fprintf(stderr,"dbg2       symmetry:        %d\n", symmetry);
		fprintf(stderr,"dbg2       mean and sigma:\n");
		for (i=0;i<nangles;i++)
			fprintf(stderr,"dbg2         %d %f %d %f %f\n", 
				i, (i * dangle), nmean[i], mean[i], sigma[i]);
		}
		
	/* get average amplitude at reference angle */
	iref = (angle_max - ref_angle) / dangle;
	i0 = MAX(iref - 1, 0);
	i1 = MIN(iref + 1, nangles - 1);
	ref_amp = 0.0;
	sum = 0.0;
	sumsq = 0.0;
	sumn = 0.0;
	for (ii=i0;ii<=i1;ii++)
		{
		sum += mean[ii];
		sumsq += sigma[ii];
		sumn += nmean[ii];
		jj = nangles - ii - 1;
		sum += mean[jj];
		sumsq += sigma[jj];
		sumn += nmean[jj];
		}
	if (sumn > 0.0)
		{
		ref_amp = sum /  sumn;
		asigma = sqrt((sumsq / sumn) - amean * amean);
		}
		
	/* get model that combines gaussian with 1/r
		- gaussian must drop to 0.7 max at 0.5 * beamwidth
		- model must equal ref_amp at ref_angle */
	del = (90.0 - depression) - 0.5 * beamwidth;
	aa = -log(0.1) / (del * del);
	del = 90.0 - depression - ref_angle;
	range = altitude / cos (DTR * ref_angle);
	factor = ref_amp * range * range / exp(-aa * del * del);

	/* process sums and print out results */
	mb_get_date(verbose, time_d, time_i, error);
	fprintf(tfp,"# table: %d\n", ntable);
	fprintf(tfp,"# nping: %d\n", nping);
	fprintf(tfp,"# time:  %4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d    %16.6f\n",
		time_i[0],time_i[1],time_i[2],
		time_i[3],time_i[4],time_i[5],
		time_i[6],time_d);
	fprintf(tfp,"# nangles: %d\n", nangles);
	for (i=0;i<nangles;i++)
		{
		angle = -angle_max + i * dangle;
		del = fabs(angle) - (90 - depression);
		range = altitude / cos (DTR * fabs(angle));
		amean = factor * exp(-aa * del * del) / (range * range);
		fprintf(tfp,"%7.4f %12.4f %12.4f\n", angle, amean, asigma);
		}
	fprintf(tfp,"#\n");
	fprintf(tfp,"#\n");

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBACKANGLE function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
/*
 * function write_cdfgrd writes output grid to a 
 * GMT version 2 netCDF grd file 
 */
int write_cdfgrd(int verbose, char *outfile, float *grid,
		int nx, int ny, 
		double xmin, double xmax, double ymin, double ymax,
		double zmin, double zmax, double dx, double dy, 
		char *xlab, char *ylab, char *zlab, char *titl, 
		char *projection, int argc, char **argv, 
		int *error)
{
	char	*function_name = "write_cdfgrd";
	int	status = MB_SUCCESS;
	struct GRD_HEADER grd;
	double	w, e, s, n;
	int	pad[4];
	int	complex;
	float	*a;
	time_t	right_now;
	char	date[MB_PATH_MAXLINE], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];
	int	i, j, kg, ka;
	char	*message;
	char	*ctime();
	char	*getenv();

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       outfile:    %s\n",outfile);
		fprintf(stderr,"dbg2       grid:       %d\n",grid);
		fprintf(stderr,"dbg2       nx:         %d\n",nx);
		fprintf(stderr,"dbg2       ny:         %d\n",ny);
		fprintf(stderr,"dbg2       xmin:       %f\n",xmin);
		fprintf(stderr,"dbg2       xmax:       %f\n",xmax);
		fprintf(stderr,"dbg2       ymin:       %f\n",ymin);
		fprintf(stderr,"dbg2       ymax:       %f\n",ymax);
		fprintf(stderr,"dbg2       zmin:       %f\n",zmin);
		fprintf(stderr,"dbg2       zmax:       %f\n",zmax);
		fprintf(stderr,"dbg2       dx:         %f\n",dx);
		fprintf(stderr,"dbg2       dy:         %f\n",dy);
		fprintf(stderr,"dbg2       xlab:       %s\n",xlab);
		fprintf(stderr,"dbg2       ylab:       %s\n",ylab);
		fprintf(stderr,"dbg2       zlab:       %s\n",zlab);
		fprintf(stderr,"dbg2       titl:       %s\n",titl);
		fprintf(stderr,"dbg2       argc:       %d\n",argc);
		fprintf(stderr,"dbg2       *argv:      %d\n",*argv);
		}

	/* inititialize grd header */
	GMT_program = program_name;
	GMT_grd_init (&grd, 1, argv, FALSE);
	GMT_io_init ();
	GMT_grdio_init ();
	GMT_make_fnan (GMT_f_NaN);
	GMT_make_dnan (GMT_d_NaN);

	/* copy values to grd header */
	grd.nx = nx;
	grd.ny = ny;
	grd.node_offset = 0;
	grd.x_min = xmin;
	grd.x_max = xmax;
	grd.y_min = ymin;
	grd.y_max = ymax;
	grd.z_min = zmin;
	grd.z_max = zmax;
	grd.x_inc = dx;
	grd.y_inc = dy;
	grd.z_scale_factor = 1.0;
	grd.z_add_offset = 0.0;
	strcpy(grd.x_units,xlab);
	strcpy(grd.y_units,ylab);
	strcpy(grd.z_units,zlab);
	strcpy(grd.title,titl);
	strcpy(grd.command,"\0");
	strncpy(date,"\0",MB_PATH_MAXLINE);
	right_now = time((time_t *)0);
	strncpy(date,ctime(&right_now),24);
	if ((user_ptr = getenv("USER")) == NULL)
		user_ptr = getenv("LOGNAME");
	if (user_ptr != NULL)
		strcpy(user,user_ptr);
	else
		strcpy(user, "unknown");
	gethostname(host,MB_PATH_MAXLINE);
	sprintf(grd.remark,"\n\tProjection: %s\n\tGrid created by %s\n\tMB-system Version %s\n\tRun by <%s> on <%s> at <%s>",
		projection,program_name,MB_VERSION,user,host,date);

	/* set extract wesn,pad and complex */
	w = 0.0;
	e = 0.0;
	s = 0.0;
	n = 0.0;
	for (i=0;i<4;i++)
		pad[i] = 0;
	complex = 0;

	/* allocate memory for output array */
	status = mb_malloc(verbose,grd.nx*grd.ny*sizeof(float),&a,error);
	if (*error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,MB_ERROR_MEMORY_FAIL,&message);
		fprintf(stderr,"\nMBIO Error allocating output arrays.\n",
			message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		mb_memory_clear(verbose, &error);
		exit(status);
		}

	/* copy grid to new array and write it to GMT netCDF grd file */
	if (status == MB_SUCCESS)
		{
		/* copy grid to new array */
		for (i=0;i<grd.nx;i++)
			for (j=0;j<grd.ny;j++)
				{
				kg = i*grd.ny+j;
				ka = (grd.ny-1-j)*grd.nx+i;
				a[ka] = grid[kg];
				}

		/* write the GMT netCDF grd file */
		GMT_write_grd(outfile, &grd, a, w, e, s, n, pad, complex);

		/* free memory for output array */
		mb_free(verbose, &a, error);
		}
	    
	/* free GMT memory */
	GMT_free ((void *)GMT_io.skip_if_NaN);
	GMT_free ((void *)GMT_io.in_col_type);
	GMT_free ((void *)GMT_io.out_col_type);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/

