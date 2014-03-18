/*--------------------------------------------------------------------
 *    The MB-system:	mbsegygrid.c	6/12/2004
 *    $Id$
 *
 *    Copyright (c) 2004-2014 by
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
 * mbsegygrid inserts trace data from segy data files into a grid in
 * which the y-axis is some measure of trace number, range, or distance
 * along a profile, and the y-axis is time..
 *
 * Author:	D. W. Caress
 * Date:	June 12, 2004
 *
 * $Log: mbsegygrid.c,v $
 * Revision 5.16  2008/11/16 21:51:18  caress
 * Updating all recent changes, including time lag analysis using mbeditviz and improvements to the mbgrid footprint gridding algorithm.
 *
 * Revision 5.15  2008/09/11 20:20:14  caress
 * Checking in updates made during cruise AT15-36.
 *
 * Revision 5.14  2007/10/08 16:48:07  caress
 * State of the code on 8 October 2007.
 *
 * Revision 5.13  2006/12/15 21:42:49  caress
 * Incremental CVS update.
 *
 * Revision 5.12  2006/08/09 22:41:27  caress
 * Fixed programs that read or write grids so that they do not use the GMT_begin() function; these programs will now work when GMT is built in the default fashion, when GMT is built in the default fashion, with "advisory file locking" enabled.
 *
 * Revision 5.11  2006/06/22 04:45:43  caress
 * Working towards 5.1.0
 *
 * Revision 5.10  2006/06/16 19:30:58  caress
 * Check in after the Santa Monica Basin Mapping AUV Expedition.
 *
 * Revision 5.9  2006/04/19 18:30:34  caress
 * Fixed application of gain below seafloor.
 *
 * Revision 5.8  2006/04/11 19:19:30  caress
 * Various fixes.
 *
 * Revision 5.7  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.6  2005/06/15 15:35:01  caress
 * Added capability to scale shot to distance and time to depth. Also added -MGQ100 to the mbm_grdplot arguments so that the seismic image is more nicely displayed by grdimage.
 *
 * Revision 5.5  2005/06/04 05:59:26  caress
 * Added a time-varying gain option.
 *
 * Revision 5.4  2004/11/08 05:49:17  caress
 * Fixed problem with decimation.
 *
 * Revision 5.3  2004/10/06 19:10:52  caress
 * Release 5.0.5 update.
 *
 * Revision 5.2  2004/09/16 01:01:12  caress
 * Fixed many things.
 *
 * Revision 5.1  2004/07/15 19:33:56  caress
 * Improvements to support for Reson 7k data.
 *
 * Revision 5.0  2004/06/18 04:06:05  caress
 * Adding support for segy i/o and working on support for Reson 7k format 88.
 *
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

/* GMT include files */
#include "gmt.h"

/* MBIO include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_segy.h"

/* local options */
#define MBSEGYGRID_USESHOT		0
#define MBSEGYGRID_USECMP		1
#define MBSEGYGRID_PLOTBYTRACENUMBER	0
#define MBSEGYGRID_PLOTBYDISTANCE	1
#define MBSEGYGRID_WINDOW_OFF		0
#define MBSEGYGRID_WINDOW_ON		1
#define MBSEGYGRID_WINDOW_SEAFLOOR	2
#define MBSEGYGRID_WINDOW_DEPTH		3
#define MBSEGYGRID_GAIN_OFF		0
#define MBSEGYGRID_GAIN_TZERO		1
#define MBSEGYGRID_GAIN_SEAFLOOR	2
#define MBSEGYGRID_GAIN_AGCSEAFLOOR	3
#define MBSEGYGRID_GEOMETRY_VERTICAL	0
#define MBSEGYGRID_GEOMETRY_REAL	1
#define MBSEGYGRID_FILTER_OFF		0
#define MBSEGYGRID_FILTER_COSINE	1

/* NaN value */
float	NaN;

int write_cdfgrd(int verbose, char *outfile, float *grid,
		int nx, int ny,
		double xmin, double xmax, double ymin, double ymax,
		double zmin, double zmax, double dx, double dy,
		char *xlab, char *ylab, char *zlab, char *titl,
		char *projection, int argc, char **argv,
		int *error);
int get_segy_limits(int verbose,
		char	*segyfile,
		int	*tracemode,
		int	*tracestart,
		int	*traceend,
		int	*chanstart,
		int	*chanend,
		double	*timesweep,
		double	*timedelay,
		double	*startlon,
		double	*startlat,
		double	*endlon,
		double	*endlat,
		int *error);
char	*ctime();
char	*getenv();
int fft(float *x, int *n, int *isign);
int four1(float *data, int *n, int *isign);

/* output stream for basic stuff (stdout if verbose <= 1,
	stderr if verbose > 1) */
FILE	*outfp;

static char rcs_id[] = "$Id$";
char program_name[] = "MBsegygrid";
char help_message[] =  "MBsegygrid grids trace data from segy data files.";
char usage_message[] = "MBsegygrid -Ifile -Oroot [-Ashotscale/timescale \n\
          -Ddecimatex/decimatey -Gmode/gain[/window] -Rdistancebin[]/startlon/startlat/endlon/endlat]\n\
          -Smode[/start/end[/schan/echan]] -Tsweep[/delay] \n\
          -Wmode/start/end -H -V]";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	extern char *optarg;
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
	int	format;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	speedmin;
	double	timegap;

	/* segy data */
	char	segyfile[MB_PATH_MAXLINE];
	void	*mbsegyioptr;
	struct mb_segyasciiheader_struct asciiheader;
	struct mb_segyfileheader_struct fileheader;
	struct mb_segytraceheader_struct traceheader;
	float	*trace = NULL;
	float	*worktrace = NULL;
	float	*filtertrace = NULL;

	/* grid controls */
	char	fileroot[MB_PATH_MAXLINE];
	char	gridfile[MB_PATH_MAXLINE];
	int	decimatex = 1;
	int	decimatey = 1;
	int	plotmode = MBSEGYGRID_PLOTBYTRACENUMBER;
	double	distancebin = 1.0;
	double	startlon = 0.0;
	double	startlat = 0.0;
	double	endlon = 0.0;
	double	endlat = 0.0;
	int	tracemode = MBSEGYGRID_USESHOT;
	int	tracestart = 0;
	int	traceend = 0;
	int	chanstart = 0;
	int	chanend = -1;
	double	timesweep = 0.0;
	double	timedelay = 0.0;
	double	sampleinterval = 0.0;
	int	windowmode = MBSEGYGRID_WINDOW_OFF;
	double	windowstart, windowend;
	int	gainmode = MBSEGYGRID_GAIN_OFF;
	double	gain = 0.0;
	double	gainwindow = 0.0;
	double	gaindelay = 0.0;
	int	agcmode = MB_NO;
	double	agcwindow = 0.0;
	double	agcmaxvalue = 0.0;
	int	filtermode = MBSEGYGRID_FILTER_OFF;
	double	filterwindow = 0.0;
	int	geometrymode = MBSEGYGRID_GEOMETRY_VERTICAL;
	int	ntraces;
	int	ngridx = 0;
	int	ngridy = 0;
	int	ngridxy = 0;
	float	*grid = NULL;
	float	*gridweight = NULL;
	double	xmin;
	double	xmax;
	double	ymin;
	double	ymax;
	double	dx;
	double	dy;
	double	gridmintot = 0.0;
	double	gridmaxtot = 0.0;
	char	projection[MB_PATH_MAXLINE];
	char	xlabel[MB_PATH_MAXLINE];
	char	ylabel[MB_PATH_MAXLINE];
	char	zlabel[MB_PATH_MAXLINE];
	char	title[MB_PATH_MAXLINE];
	char	plot_cmd[MB_PATH_MAXLINE];
	int	scale2distance = MB_NO;
	double	shotscale = 1.0;
	double	timescale = 1.0;

	int	sinftracemode = MBSEGYGRID_USESHOT;
	int	sinftracestart = 0;
	int	sinftraceend = 0;
	int	sinfchanstart = 0;
	int	sinfchanend = -1;
	double	sinftimesweep = 0.0;
	double	sinftimedelay = 0.0;
	double	sinfstartlon = 0.0;
	double	sinfstartlat = 0.0;
	double	sinfendlon = 0.0;
	double	sinfendlat = 0.0;

	int	nread;
	int	tracecount, tracenum, channum, traceok;
	double	tracemin, tracemax;
	double	xwidth, ywidth;
	int	ix, iy, iys, igainstart, igainend;
	int	iystart, iyend;
	double	factor, gtime, btime, stime, dtime, ttime, tmax;
	double	cosfactor, sinfactor, rangefactor, range;
	double	filtersum;
	double	btimesave = 0.0;
	double	stimesave = 0.0;
	double	dtimesave = 0.0;
	double	mtodeglon, mtodeglat;
	double	navlon, navlat;
	double	line_distance, line_dx, line_dy, trace_x;
	double	cos_arg;
	int	plot_status;
	int	worktrace_alloc;
	int	filtertrace_alloc;
	int	nfilter;
	int	iagchalfwindow;
	int	ixc, iyc;
	int	jstart, jend;
	int	i, ii, j, k, n;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set file to null */
	segyfile[0] = '\0';

	/* get NaN value */
	GMT_make_fnan(NaN);

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:C:c:D:d:F:f:G:g:I:i:O:o:R:r:S:s:T:t:VvW:w:Hh")) != -1)
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
			n = sscanf (optarg,"%lf/%lf", &shotscale, &timescale);
			if (n == 2)
				scale2distance = MB_YES;
			flag++;
			break;
		case 'B':
		case 'b':
			n = sscanf (optarg,"%lf/%lf", &agcmaxvalue, &agcwindow);
			if (n < 2)
				agcwindow = 0.0;
			agcmode = MB_YES;
			flag++;
			break;
		case 'C':
		case 'c':
			n = sscanf (optarg,"%d", &geometrymode);
			if (n < 1)
				geometrymode = MBSEGYGRID_GEOMETRY_VERTICAL;
			flag++;
			break;
		case 'D':
		case 'd':
			n = sscanf (optarg,"%d/%d", &decimatex, &decimatey);
			flag++;
			break;
		case 'F':
		case 'f':
			n = sscanf (optarg,"%d/%lf", &filtermode, &filterwindow);
			flag++;
			break;
		case 'G':
		case 'g':
			n = sscanf (optarg,"%d/%lf/%lf/%lf", &gainmode, &gain, &gainwindow, &gaindelay);
			if (n < 4)
				gaindelay = 0.0;
			if (n < 3)
				gainwindow = 0.0;
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", segyfile);
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", fileroot);
			flag++;
			break;
		case 'R':
		case 'r':
			n = sscanf (optarg,"%lf/%lf/%lf/%lf/%lf", &distancebin, &startlon, &endlon, &startlat, &endlat);
			plotmode = MBSEGYGRID_PLOTBYDISTANCE;
			if (n < 1)
				{
				distancebin = 1.0;
				}
			if (n < 25)
				{
				startlon = 0.0;
				startlat = 0.0;
				endlon = 0.0;
				endlat = 0.0;
				}
			flag++;
			break;
		case 'S':
		case 's':
			n = sscanf (optarg,"%d/%d/%d/%d/%d", &tracemode, &tracestart, &traceend, &chanstart, &chanend);
			if (n < 5)
				{
				chanstart = 0;
				chanend = -1;
				}
			if (n < 3)
				{
				tracestart = 0;
				traceend = 0;
				}
			if (n < 1)
				{
				tracemode = MBSEGYGRID_USESHOT;
				}
			flag++;
			break;
		case 'T':
		case 't':
			n = sscanf (optarg,"%lf/%lf", &timesweep, &timedelay);
			if (n < 2)
				timedelay = 0.0;
			flag++;
			break;
		case 'W':
		case 'w':
			n = sscanf (optarg,"%d/%lf/%lf", &windowmode, &windowstart, &windowend);
			flag++;
			break;
		case '?':
			errflg++;
		}

	/* set output stream to stdout or stderr */
	if (verbose >= 2)
	    outfp = stderr;
	else
	    outfp = stdout;

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(outfp,"usage: %s\n", usage_message);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(outfp,"\nProgram %s\n",program_name);
		fprintf(outfp,"Version %s\n",rcs_id);
		fprintf(outfp,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 0)
		{
		fprintf(outfp,"\ndbg2  Program <%s>\n",program_name);
		fprintf(outfp,"dbg2  Version %s\n",rcs_id);
		fprintf(outfp,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(outfp,"dbg2  Control Parameters:\n");
		fprintf(outfp,"dbg2       verbose:        %d\n",verbose);
		fprintf(outfp,"dbg2       help:           %d\n",help);
		fprintf(outfp,"dbg2       segyfile:       %s\n",segyfile);
		fprintf(outfp,"dbg2       fileroot:       %s\n",fileroot);
		fprintf(outfp,"dbg2       decimatex:      %d\n",decimatex);
		fprintf(outfp,"dbg2       decimatey:      %d\n",decimatey);
		fprintf(outfp,"dbg2       plotmode:       %d\n",plotmode);
		fprintf(outfp,"dbg2       distancebin:    %f\n",distancebin);
		fprintf(outfp,"dbg2       startlon:       %f\n",startlon);
		fprintf(outfp,"dbg2       startlat:       %f\n",startlat);
		fprintf(outfp,"dbg2       endlon:         %f\n",endlon);
		fprintf(outfp,"dbg2       endlat:         %f\n",endlat);
		fprintf(outfp,"dbg2       tracemode:      %d\n",tracemode);
		fprintf(outfp,"dbg2       tracestart:     %d\n",tracestart);
		fprintf(outfp,"dbg2       traceend:       %d\n",traceend);
		fprintf(outfp,"dbg2       chanstart:      %d\n",chanstart);
		fprintf(outfp,"dbg2       chanend:        %d\n",chanend);
		fprintf(outfp,"dbg2       timesweep:      %f\n",timesweep);
		fprintf(outfp,"dbg2       timedelay:      %f\n",timedelay);
		fprintf(outfp,"dbg2       ngridx:         %d\n",ngridx);
		fprintf(outfp,"dbg2       ngridy:         %d\n",ngridy);
		fprintf(outfp,"dbg2       ngridxy:        %d\n",ngridxy);
		fprintf(outfp,"dbg2       windowmode:     %d\n",windowmode);
		fprintf(outfp,"dbg2       windowstart:    %f\n",windowstart);
		fprintf(outfp,"dbg2       windowend:      %f\n",windowend);
		fprintf(outfp,"dbg2       agcmode:        %d\n",agcmode);
		fprintf(outfp,"dbg2       agcmaxvalue:    %f\n",agcmaxvalue);
		fprintf(outfp,"dbg2       agcwindow:      %f\n",agcwindow);
		fprintf(outfp,"dbg2       gainmode:       %d\n",gainmode);
		fprintf(outfp,"dbg2       gain:           %f\n",gain);
		fprintf(outfp,"dbg2       gainwindow:     %f\n",gainwindow);
		fprintf(outfp,"dbg2       gaindelay:      %f\n",gaindelay);
		fprintf(outfp,"dbg2       filtermode:     %d\n",filtermode);
		fprintf(outfp,"dbg2       filterwindow:   %f\n",filterwindow);
		fprintf(outfp,"dbg2       geometrymode:   %d\n",geometrymode);
		fprintf(outfp,"dbg2       scale2distance: %d\n",scale2distance);
		fprintf(outfp,"dbg2       shotscale:      %f\n",shotscale);
		fprintf(outfp,"dbg2       timescale:      %f\n",timescale);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(outfp,"\n%s\n",help_message);
		fprintf(outfp,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* get segy limits if required */
	if (traceend < 1 || traceend < tracestart || timesweep <= 0.0
		|| (plotmode == MBSEGYGRID_PLOTBYDISTANCE && startlon == 0.0))
		{
		get_segy_limits(verbose,
				segyfile,
				&sinftracemode,
				&sinftracestart,
				&sinftraceend,
				&sinfchanstart,
				&sinfchanend,
				&sinftimesweep,
				&sinftimedelay,
				&sinfstartlon,
				&sinfstartlat,
				&sinfendlon,
				&sinfendlat,
				&error);
		if (traceend < 1 || traceend < tracestart)
			{
			tracemode = sinftracemode;
			tracestart = sinftracestart;
			traceend = sinftraceend;
			}
		if (chanend < 1 || chanend < chanstart)
			{
			chanstart = sinfchanstart;
			chanend = sinfchanend;
			}
		if (timesweep <= 0.0)
			{
			timesweep = sinftimesweep;
			timedelay = sinftimedelay;
			}
		if (sinfstartlon != sinfendlon && sinfstartlat != sinfendlat)
			{
			startlon = sinfstartlon;
			startlat = sinfstartlat;
			endlon = sinfendlon;
			endlat = sinfendlat;
			}
		}

	/* check specified parameters */
	if (traceend < 1 || traceend < tracestart)
		{
		fprintf(outfp,"\nBad trace numbers: %d %d specified...\n", tracestart, traceend);
		fprintf(outfp,"\nProgram <%s> Terminated\n", program_name);
		exit(error);
		}
	if (timesweep <= 0.0)
		{
		fprintf(outfp,"\nBad time sweep: %f specified...\n", timesweep);
		fprintf(outfp,"\nProgram <%s> Terminated\n", program_name);
		exit(error);
		}

	/* initialize reading the segy file */
	if (mb_segy_read_init(verbose, segyfile,
		&mbsegyioptr, &asciiheader, &fileheader, &error) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(outfp,"\nMBIO Error returned from function <mb_segy_read_init>:\n%s\n",message);
		fprintf(outfp,"\nSEGY File <%s> not initialized for reading\n",segyfile);
		fprintf(outfp,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* calculate implied grid parameters */
	strcpy(gridfile,fileroot);
	strcat(gridfile,".grd");
	if (chanend >= chanstart)
		ntraces = (traceend - tracestart + 1) * (chanend - chanstart + 1);
	else
		ntraces = traceend - tracestart + 1;

	/* set up plotting trace by trace */
	if (plotmode == MBSEGYGRID_PLOTBYTRACENUMBER)
		{
		ngridx = ntraces / decimatex;
		sampleinterval = 0.000001 * (double) (fileheader.sample_interval);
		ngridy = timesweep / sampleinterval / decimatey + 1;
		ngridxy = ngridx * ngridy;
		xmin = (double) tracestart - 0.5;
		xmax = (double) traceend + 0.5;
		ymax = -(timedelay - 0.5 * sampleinterval / decimatey);
		ymin = ymax - ngridy * sampleinterval * decimatey;
		/*ymax = timedelay + timesweep + 0.5 * sampleinterval / decimatey;*/
		}

	/* set up plotting trace by distance along a line */
	else if (plotmode == MBSEGYGRID_PLOTBYDISTANCE)
		{
		/* get distance scaling */
		mb_coor_scale(verbose,0.5*(startlat + endlat),&mtodeglon,&mtodeglat);
		dx = (endlon - startlon) / mtodeglon;
		dy = (endlat - startlat) / mtodeglat;
		line_distance = sqrt(dx * dx + dy * dy);
		line_dx = dx / line_distance;
		line_dy = dy / line_distance;

		ngridx = (int)(line_distance / distancebin / decimatex);
		sampleinterval = 0.000001 * (double) (fileheader.sample_interval);
		ngridy = timesweep / sampleinterval / decimatey + 1;
		ngridxy = ngridx * ngridy;
		xmin = -0.5 * distancebin;
		xmax = line_distance + 0.5 * distancebin;
		ymax = -(timedelay - 0.5 * sampleinterval / decimatey);
		ymin = ymax - ngridy * sampleinterval * decimatey;
		/*ymax = timedelay + timesweep + 0.5 * sampleinterval / decimatey;*/
		}

	/* get start and end samples */
	if (windowmode == MBSEGYGRID_WINDOW_OFF)
		{
		iystart = 0;
		iyend = ngridy - 1;
		}
	else if (windowmode == MBSEGYGRID_WINDOW_ON)
		{
		iystart = MAX((windowstart) / sampleinterval, 0);
		iyend = MIN((windowend) / sampleinterval, ngridy - 1);
		}

	/* allocate memory for grid array */
	status = mb_mallocd(verbose,__FILE__,__LINE__, ngridxy * sizeof(float), (void **)&grid, &error);
	status = mb_mallocd(verbose,__FILE__,__LINE__, ngridxy * sizeof(float), (void **)&gridweight, &error);

	/* output info */
	if (verbose >= 0)
		{
		fprintf(outfp,"\nMBsegygrid Parameters:\n");
		fprintf(outfp,"Input segy file:         %s\n",segyfile);
		fprintf(outfp,"Output fileroot:         %s\n",fileroot);
		fprintf(outfp,"Input Parameters:\n");
		fprintf(outfp,"     plot mode:          %d\n",plotmode);
		fprintf(outfp,"     trace mode:         %d\n",tracemode);
		fprintf(outfp,"     trace start:        %d\n",tracestart);
		fprintf(outfp,"     trace end:          %d\n",traceend);
		fprintf(outfp,"     channel start:      %d\n",chanstart);
		fprintf(outfp,"     channel end:        %d\n",chanend);
		fprintf(outfp,"     start longitude:    %f\n",startlon);
		fprintf(outfp,"     start latitude:     %f\n",startlat);
		fprintf(outfp,"     end longitude:      %f\n",endlon);
		fprintf(outfp,"     end latitude:       %f\n",endlat);
		fprintf(outfp,"     trace decimation:   %d\n",decimatex);
		fprintf(outfp,"     time sweep:         %f seconds\n",timesweep);
		fprintf(outfp,"     time delay:         %f seconds\n",timedelay);
		fprintf(outfp,"     sample interval:    %f seconds\n",sampleinterval);
		fprintf(outfp,"     sample decimation:  %d\n",decimatey);
		fprintf(outfp,"     window mode:        %d\n",windowmode);
		fprintf(outfp,"     window start:       %f seconds\n",windowstart);
		fprintf(outfp,"     window end:         %f seconds\n",windowend);
		fprintf(outfp,"     agcmode:            %d\n",agcmode);
		fprintf(outfp,"     gain mode:          %d\n",gainmode);
		fprintf(outfp,"     gain:               %f\n",gain);
		fprintf(outfp,"     gainwindow:         %f\n",gainwindow);
		fprintf(outfp,"     gaindelay:          %f\n",gaindelay);
		fprintf(outfp,"Output Parameters:\n");
		fprintf(outfp,"     grid filename:      %s\n",gridfile);
		fprintf(outfp,"     x grid dimension:   %d\n",ngridx);
		fprintf(outfp,"     y grid dimension:   %d\n",ngridy);
		fprintf(outfp,"     grid xmin:          %f\n",xmin);
		fprintf(outfp,"     grid xmax:          %f\n",xmax);
		fprintf(outfp,"     grid ymin:          %f\n",ymin);
		fprintf(outfp,"     grid ymax:          %f\n",ymax);
		fprintf(outfp,"     NaN values used to flag regions with no data\n");
		if (scale2distance == MB_YES)
			{
			fprintf(outfp,"     shot and time scaled to distance in meters\n");
			fprintf(outfp,"     shotscale:          %f\n",shotscale);
			fprintf(outfp,"     timescale:          %f\n",timescale);
			fprintf(outfp,"     scaled grid xmin    %f\n",0.0);
			fprintf(outfp,"     scaled grid xmax:   %f\n",shotscale * (xmax - xmin));
			fprintf(outfp,"     scaled grid ymin:   %f\n",0.0);
			fprintf(outfp,"     scaled grid ymax:   %f\n",timescale * (ymax - ymin));
			}
		}
	if (verbose > 0)
		fprintf(outfp,"\n");

	/* proceed if all ok */
	if (status == MB_SUCCESS)
		{

		/* initialize grid and weight arrays */
		for (k=0;k<ngridxy;k++)
			{
			grid[k] = 0.0;
			gridweight[k] = 0.0;
			}

		/* read and print data */
		nread = 0;
		while (error <= MB_ERROR_NO_ERROR)
			{
			/* reset error */
			error = MB_ERROR_NO_ERROR;

			/* read a trace */
			status = mb_segy_read_trace(verbose, mbsegyioptr,
					&traceheader, &trace, &error);

			/* now process the trace */
			if (status == MB_SUCCESS)
				{
				/* figure out where this trace is in the grid laterally */
				if (plotmode == MBSEGYGRID_PLOTBYTRACENUMBER)
					{
					if (tracemode == MBSEGYGRID_USESHOT)
						{
						tracenum = traceheader.shot_num;
						channum = traceheader.shot_tr;
						}
					else if (tracemode == MBSEGYGRID_USECMP)
						{
						tracenum = traceheader.rp_num;
						channum = traceheader.rp_tr;
						}
					if (chanend >= chanstart)
						{
						tracecount = (tracenum - tracestart) * (chanend - chanstart + 1)
								+ (channum - chanstart);
						}
					else
						{
						tracecount = tracenum - tracestart;
						}
					ix = tracecount / decimatex;

					/* now check if this is a trace of interest */
					traceok = MB_YES;
					if (tracenum < tracestart
						|| tracenum > traceend)
						traceok = MB_NO;
					else if (chanend >= chanstart
							&& (channum < chanstart
								|| channum > chanend))
						traceok = MB_NO;
					else if (tracecount % decimatex != 0)
						traceok = MB_NO;
					}
				else if (plotmode == MBSEGYGRID_PLOTBYDISTANCE)
					{
					if (traceheader.coord_scalar < 0)
						factor = 1.0 / ((float) (-traceheader.coord_scalar)) / 3600.0;
					else
						factor = (float) traceheader.coord_scalar / 3600.0;
					if (traceheader.src_long != 0)
						navlon  = factor * ((float)traceheader.src_long);
					else
						navlon  = factor * ((float)traceheader.grp_long);
					if (traceheader.src_lat != 0)
						navlat  = factor * ((float)traceheader.src_lat);
					else
						navlat  = factor * ((float)traceheader.grp_lat);
					if (lonflip < 0)
						{
						if (navlon > 0.)
							navlon = navlon - 360.;
						else if (navlon < -360.)
							navlon = navlon + 360.;
						}
					else if (lonflip == 0)
						{
						if (navlon > 180.)
							navlon = navlon - 360.;
						else if (navlon < -180.)
							navlon = navlon + 360.;
						}
					else
						{
						if (navlon > 360.)
							navlon = navlon - 360.;
						else if (navlon < 0.)
							navlon = navlon + 360.;
						}
					dx = (navlon - startlon) / mtodeglon;
					dy = (navlat - startlat) / mtodeglat;
					trace_x = dx * line_dx + dy * line_dy;
					ix = ((int)((trace_x - 0.5 * distancebin) / distancebin)) / decimatex;
					if (ix >= 0 && ix < ngridx)
						traceok = MB_YES;
					else
						traceok = MB_NO;
					}

				/* figure out where this trace is in the grid vertically */
				if (traceheader.elev_scalar < 0)
					factor = 1.0 / ((float) (-traceheader.elev_scalar));
				else
					factor = (float) traceheader.elev_scalar;
				if (traceheader.src_depth > 0)
					{
					btime = factor * traceheader.src_depth / 750.0 + 0.001 * traceheader.delay_mils;
					dtime = factor * traceheader.src_depth / 750.0;
					btimesave = btime;
					dtimesave = dtime;
					}
				else if (traceheader.src_elev > 0)
					{
					btime = -factor * traceheader.src_elev / 750.0 + 0.001 * traceheader.delay_mils;
					dtime = -factor * traceheader.src_elev / 750.0;
					btimesave = btime;
					dtimesave = dtime;
					}
				else
					{
					btime = btimesave;
					dtime = dtimesave;
					}
				if (traceheader.src_wbd > 0)
					{
					stime = factor * traceheader.src_wbd / 750.0;
					stimesave = stime;
					}
				else
					{
					stime = stimesave;
					}
				iys = (btime - timedelay) / sampleinterval;

				/* get trace min and max */
				tracemin = trace[0];
				tracemax = trace[0];
				for (i=0;i<traceheader.nsamps;i++)
					{
					tracemin = MIN(tracemin, trace[i]);
					tracemax = MAX(tracemin, trace[i]);
					}

				if ((verbose == 0 && nread % 250 == 0) || (nread % 25 == 0))
					{
					if (traceok == MB_YES)
						fprintf(outfp,"PROCESS ");
					else
						fprintf(outfp,"IGNORE  ");
					if (tracemode == MBSEGYGRID_USESHOT)
						fprintf(outfp,"read:%d position:%d shot:%d channel:%d ",
							nread,tracecount,tracenum,channum);
					else
						fprintf(outfp,"read:%d position:%d rp:%d channel:%d ",
							nread,tracecount,tracenum,channum);
					if (plotmode == MBSEGYGRID_PLOTBYDISTANCE)
						fprintf(outfp,"distance:%.3f ", trace_x);
					fprintf(outfp,"%4.4d/%3.3d %2.2d:%2.2d:%2.2d.%3.3d samples:%d interval:%d usec minmax: %f %f\n",
					traceheader.year,traceheader.day_of_yr,
					traceheader.hour,traceheader.min,traceheader.sec,traceheader.mils,
					traceheader.nsamps,traceheader.si_micros,
					tracemin, tracemax);
					}

				/* now actually process traces of interest */
				if (traceok == MB_YES)
					{
					/* get bounds of trace in depth window mode */
					if (windowmode == MBSEGYGRID_WINDOW_DEPTH)
						{
						iystart = (int)((dtime + windowstart - timedelay) / sampleinterval);
						iystart = MAX(iystart, 0);
						iyend = (int)((dtime + windowend - timedelay) / sampleinterval);
						iyend = MIN(iyend, ngridy - 1);
						}
					else if (windowmode == MBSEGYGRID_WINDOW_SEAFLOOR)
						{
						iystart = MAX((stime + windowstart - timedelay) / sampleinterval, 0);
						iyend = MIN((stime + windowend - timedelay) / sampleinterval, ngridy - 1);
						}

					/* apply gain if desired */
					if (gainmode == MBSEGYGRID_GAIN_TZERO
						|| gainmode == MBSEGYGRID_GAIN_SEAFLOOR)
						{
						if (gainmode == MBSEGYGRID_GAIN_TZERO)
							igainstart = (dtime - btime + gaindelay) / sampleinterval;
						else if (gainmode == MBSEGYGRID_GAIN_SEAFLOOR)
							igainstart = (stime - btime + gaindelay) / sampleinterval;
						igainstart = MAX(0, igainstart);
						if (gainwindow <= 0.0)
							{
							igainend = traceheader.nsamps - 1;
							}
						else
							{
							igainend = igainstart + gainwindow / sampleinterval;
							igainend = MIN(traceheader.nsamps - 1, igainend);
							}
/*fprintf(stderr,"gainmode:%d btime:%f stime:%f igainstart:%d igainend:%d\n",
gainmode,btime,stime,igainstart,igainend);*/
						for (i=0;i<=igainstart;i++)
							{
							trace[i] = 0.0;
							}
						for (i=igainstart;i<=igainend;i++)
							{
							gtime = (i - igainstart) * sampleinterval;
							factor = 1.0 + gain * gtime;
/*fprintf(stderr,"i:%d iy:%d factor:%f trace[%d]: %f",
i,iy,factor,i,trace[i]);*/
							trace[i] = trace[i] * factor;
/*fprintf(stderr," %f\n",trace[i]);*/
							}
						for (i=igainend+1;i<=traceheader.nsamps;i++)
							{
							trace[i] = 0.0;
							}
						}
					else if (gainmode == MBSEGYGRID_GAIN_AGCSEAFLOOR)
						{
						igainstart = (stime - btime - 0.5 * gainwindow) / sampleinterval;
						igainstart = MAX(0, igainstart);
						igainend = (stime - btime + 0.5 * gainwindow) / sampleinterval;
						igainend = MIN(traceheader.nsamps - 1, igainend);
						tmax = fabs(trace[igainstart]);
						for (i=igainstart;i<=igainend;i++)
							{
							tmax = MAX(tmax, fabs(trace[i]));
							}
						if (tmax > 0.0)
							factor = gain / tmax;
						else
							factor = 1.0;
						for (i=0;i<=traceheader.nsamps;i++)
							{
							trace[i] *= factor;
							}
/*fprintf(stderr,"igainstart:%d igainend:%d tmax:%f factor:%f\n",
igainstart,igainend,tmax,factor);*/
						}

					/* apply filtering if desired */
					if (filtermode != MBSEGYGRID_FILTER_OFF)
						{
						if (worktrace == NULL || traceheader.nsamps > worktrace_alloc)
							{
							status = mb_reallocd(verbose,__FILE__,__LINE__,
										traceheader.nsamps * sizeof(float),
										(void **)&worktrace, &error);
							worktrace_alloc = traceheader.nsamps;
							}
						nfilter = 2 * ((int)(0.5 * filterwindow / sampleinterval)) + 1;
						if (filtertrace == NULL || nfilter > filtertrace_alloc)
							{
							status = mb_reallocd(verbose,__FILE__,__LINE__,
										nfilter * sizeof(float),
										(void **)&filtertrace, &error);
							filtertrace_alloc = nfilter;
							}
						filtersum = 0.0;
						for (j=0;j<nfilter;j++)
							{
							cos_arg = (0.5 * M_PI * (j - nfilter / 2)) / (0.5 * nfilter);
							filtertrace[j] = cos(cos_arg);
							filtersum += filtertrace[j];
/*fprintf(stderr,"FILTER: j:%d nfilter:%d cos_arg:%f cos:%f filtertrace:%f sum:%f\n",j,nfilter,cos_arg,cos(cos_arg),filtertrace[j],filtersum);*/
							}
						for (i=0;i<=traceheader.nsamps;i++)
							{
							worktrace[i] = 0.0;
							filtersum = 0.0;
							jstart = MAX(nfilter / 2 - i, 0);
							jend = MIN(nfilter - 1, nfilter - 1 + (traceheader.nsamps - 1 - nfilter / 2 - i));
							for (j=jstart;j<=jend;j++)
								{
								ii = i - nfilter/2 + j;
								worktrace[i] += filtertrace[j] * trace[ii];
								filtersum += filtertrace[j];
/* fprintf(stderr,"      i:%d j:%d ii:%d trace:%f sum:%f\n",i,j,ii,worktrace[i],filtersum); */
								}
							worktrace[i] /= filtersum;
/* fprintf(stderr,"i:%d jstart:%d jend:%d trace: %f %f\n",i,jstart,jend,trace[i], worktrace[i]);*/
							}
						for (i=0;i<=traceheader.nsamps;i++)
							{
							trace[i] = worktrace[i];
							}
						}

					/* apply agc if desired */
					if (agcmode == MB_YES && agcwindow > 0.0)
						{
						if (worktrace == NULL || traceheader.nsamps > worktrace_alloc)
							{
							status = mb_reallocd(verbose,__FILE__,__LINE__,
										traceheader.nsamps * sizeof(float),
										(void **)&worktrace, &error);
							worktrace_alloc = traceheader.nsamps;
							}
						iagchalfwindow = 0.5 * agcwindow / sampleinterval;
						for (i=0;i<=traceheader.nsamps;i++)
							{
							igainstart = i - iagchalfwindow;
							igainstart = MAX(0, igainstart);
							igainend = i + iagchalfwindow;
							igainend = MIN(traceheader.nsamps - 1, igainend);
							tmax = 0.0;
							for (j=igainstart;j<=igainend;j++)
								{
								tmax = MAX(tmax, fabs(trace[j]));
								}
							if (tmax > 0.0)
								worktrace[i] = trace[i] * agcmaxvalue / tmax;
							else
								worktrace[i] = trace[i];
							}
						for (i=0;i<=traceheader.nsamps;i++)
							{
							trace[i] = worktrace[i];
							}
						}
					else if (agcmode == MB_YES)
						{
						tmax = 0.0;
						for (i=0;i<=traceheader.nsamps;i++)
							{
							tmax = MAX(tmax, fabs(trace[i]));
							}
						if (tmax > 0.0)
							factor = agcmaxvalue / tmax;
						else
							factor = 1.0;
						for (i=0;i<=traceheader.nsamps;i++)
							{
							trace[i] *= factor;
							}
						}

					/* process trace for simple vertical geometry */
					if (geometrymode == MBSEGYGRID_GEOMETRY_VERTICAL)
						{
						for (i=0;i<traceheader.nsamps;i++)
							{
							iy = iys + i / decimatey;
							k = iy * ngridx + ix;
							if (iy >= iystart && iy <= iyend)
								{
								grid[k] += trace[i];
								gridweight[k] += 1.0;
								}
							}
						}

					/* process trace for real geometry using pitch */
					else /* if (geometrymode == MBSEGYGRID_GEOMETRY_REAL) */
						{
						cosfactor = cos(DTR * traceheader.pitch);
						sinfactor = sin(DTR * traceheader.pitch);
						rangefactor = 0.5 * traceheader.soundspeed;

						for (i=0;i<traceheader.nsamps;i++)
							{
							/* get range of sample in meters using sound speed */
							ttime = i * sampleinterval + timedelay;
							range = rangefactor * ttime;

							/* get corrected x and y location of this sample
							  in the section grid using the pitch angle */
							iyc = iys + ((int)((ttime * cosfactor - timedelay) / sampleinterval)) / decimatey;
							if (traceheader.distance > 0.0)
								ixc = ix + ((int)(range * sinfactor / traceheader.distance)) / decimatex;
							else
								ixc = ix;

							/* get the index of the sample location */
							if (iyc >= iystart && iyc <= iyend
								&& ixc >= 0 && ixc < ngridx)
								{
								k = iyc * ngridx + ixc;
								grid[k] += trace[i];
								gridweight[k] += 1.0;
								}
							}
						}
					}
				}

			/* now process the trace */
			if (status == MB_SUCCESS)
				nread++;
			}

		/* calculate the grid */
		gridmintot = 0.0;
		gridmaxtot = 0.0;
		for (k=0;k<ngridxy;k++)
			{
			if (gridweight[k] > 0.0)
				{
				grid[k] = grid[k] / gridweight[k];
				gridmintot = MIN(grid[k], gridmintot);
				gridmaxtot = MAX(grid[k], gridmaxtot);
				}
			else
				{
				grid[k] = NaN;
				}
			}
		}

	/* write out the grid */
	error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;
	strcpy(projection, "SeismicProfile");
	if (scale2distance == MB_YES)
		{
		strcpy(xlabel, "Distance (m)");
		strcpy(ylabel, "Depth (m)");
		xmax = shotscale * (xmax - xmin);
		xmin = 0.0;
		ymin = timescale * ymin;
		ymax = timescale * ymax;
		dx = shotscale * decimatex;
		dy = timescale * sampleinterval / decimatey;
		}
	else
		{
		strcpy(xlabel, "Trace Number");
		strcpy(ylabel, "Travel Time (seconds)");
		dx = (double) decimatex;
		dy = sampleinterval / decimatey;
		}
	strcpy(zlabel, "Trace Signal");
	sprintf(title, "Seismic Grid from %s", segyfile);
	status = write_cdfgrd(verbose, gridfile, grid,
		ngridx, ngridy,
		xmin, xmax, ymin, ymax,
		gridmintot, gridmaxtot, dx, dy,
		xlabel, ylabel, zlabel, title,
		projection, argc, argv, &error);

	/* close the swath file */
	status = mb_segy_close(verbose,&mbsegyioptr,&error);

	/* deallocate memory for grid array */
	if (worktrace != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&worktrace, &error);
	if (filtertrace != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&filtertrace, &error);
	status = mb_freed(verbose,__FILE__,__LINE__,(void **)&grid, &error);
	status = mb_freed(verbose,__FILE__,__LINE__,(void **)&gridweight, &error);

	/* run mbm_grdplot */
	xwidth = MIN(0.01 * (double) ngridx, 55.0);
	ywidth = MIN(0.01 * (double) ngridy, 28.0);
	sprintf(plot_cmd, "mbm_grdplot -I%s -JX%f/%f -G1 -V -L\"File %s - %s:%s\"",
			gridfile, xwidth, ywidth, gridfile, title, zlabel);
	if (verbose)
		{
		fprintf(outfp, "\nexecuting mbm_grdplot...\n%s\n",
			plot_cmd);
		}
	plot_status = system(plot_cmd);
	if (plot_status == -1)
		{
		fprintf(outfp, "\nError executing mbm_grdplot on grid file %s\n", gridfile);
		}

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(outfp,"dbg2  Ending status:\n");
		fprintf(outfp,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
/*
 * function get_segy_limits gets info for default segy gridding
 */
int get_segy_limits(int verbose,
		char	*segyfile,
		int	*tracemode,
		int	*tracestart,
		int	*traceend,
		int	*chanstart,
		int	*chanend,
		double	*timesweep,
		double	*timedelay,
		double	*startlon,
		double	*startlat,
		double	*endlon,
		double	*endlat,
		int *error)
{
	char	*function_name = "get_segy_limits";
	int	status = MB_SUCCESS;
	char	sinffile[MB_PATH_MAXLINE];
	char	command[MB_PATH_MAXLINE];
	char	line[MB_PATH_MAXLINE];
	FILE	*sfp;
	int	datmodtime = 0;
	int	sinfmodtime = 0;
	struct stat file_status;
	int	fstat;
	double	delay0 = 0.0;
	double	delay1 = 0.0;
	double	delaydel = 0.0;
	int	shot0, shot1, shotdel;
	int	shottrace0, shottrace1, shottracedel;
	int	rp0, rp1;
	int	rpdel = 0;
	int	rptrace0, rptrace1, rptracedel;
	int	nscan;
	int	shellstatus;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(outfp,"dbg2  Input arguments:\n");
		fprintf(outfp,"dbg2       verbose:    %d\n",verbose);
		fprintf(outfp,"dbg2       segyfile:   %s\n",segyfile);
		}

	/* set sinf filename */
	sprintf(sinffile, "%s.sinf", segyfile);

	/* check status of segy and sinf file */
	datmodtime = 0;
	sinfmodtime = 0;
	if ((fstat = stat(segyfile, &file_status)) == 0
		&& (file_status.st_mode & S_IFMT) != S_IFDIR)
		{
		datmodtime = file_status.st_mtime;
		}
	if ((fstat = stat(sinffile, &file_status)) == 0
		&& (file_status.st_mode & S_IFMT) != S_IFDIR)
		{
		sinfmodtime = file_status.st_mtime;
		}

	/* if sinf file is missing or out of date, make it */
	if (datmodtime > 0 && datmodtime > sinfmodtime)
		{
		if (verbose >= 1)
			fprintf(stderr,"\nGenerating sinf file for %s\n",segyfile);
		sprintf(command, "mbsegyinfo -I %s -O", segyfile);
		shellstatus = system(command);
		}


	/* read sinf file if possible */
	sprintf(sinffile, "%s.sinf", segyfile);
	if ((sfp = fopen(sinffile, "r")) != NULL)
		{
		/* read the sinf file */
		while (fgets(line, MB_PATH_MAXLINE, sfp) != NULL)
		    {
		    if (strncmp(line, "  Trace length (sec):", 21) == 0)
			{
			nscan = sscanf(line, "  Trace length (sec):%lf", timesweep);
			}
		    else if (strncmp(line, "    Delay (sec):", 16) == 0)
			{
			nscan = sscanf(line, "    Delay (sec): %lf %lf %lf", &delay0, &delay1, &delaydel);
			}
		    else if (strncmp(line, "    Shot number:", 16) == 0)
			{
			nscan = sscanf(line, "    Shot number: %d %d %d", &shot0, &shot1, &shotdel);
			}
		    else if (strncmp(line, "    Shot trace:", 15) == 0)
			{
			nscan = sscanf(line, "    Shot trace: %d %d %d", &shottrace0, &shottrace1, &shottracedel);
			}
		    else if (strncmp(line, "    RP number:", 14) == 0)
			{
			nscan = sscanf(line, "    RP number: %d %d %d", &rp0, &rp1, &rpdel);
			}
		    else if (strncmp(line, "    RP trace:", 13) == 0)
			{
			nscan = sscanf(line, "    RP trace: %d %d %d", &rptrace0, &rptrace1, &rptracedel);
			}
		    else if (strncmp(line, "    Start Position:", 19) == 0)
			{
			nscan = sscanf(line, "    Start Position: Lon: %lf     Lat:   %lf", startlon, startlat);
			}
		    else if (strncmp(line, "    End Position:", 17) == 0)
			{
			nscan = sscanf(line, "    End Position:   Lon: %lf     Lat:   %lf", endlon, endlat);
			}
		    }
		fclose(sfp);
		}

	/* set the trace mode */
	if (rpdel > 1)
		{
		*tracemode = MBSEGYGRID_USECMP;
		*tracestart = rp0;
		*traceend = rp1;
		*chanstart = rptrace0;
		*chanend = rptrace1;
		}
	else
		{
		*tracemode = MBSEGYGRID_USESHOT;
		*tracestart = shot0;
		*traceend = shot1;
		*chanstart = shottrace0;
		*chanend = shottrace1;
		}

	/* set the sweep and delay */
	if (delaydel > 0.0)
		{
		*timesweep += delaydel;
		}
	*timedelay = delay0;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(outfp,"dbg2  Return values:\n");
		fprintf(outfp,"dbg2       tracemode:  %d\n",*tracemode);
		fprintf(outfp,"dbg2       tracestart: %d\n",*tracestart);
		fprintf(outfp,"dbg2       traceend:   %d\n",*traceend);
		fprintf(outfp,"dbg2       chanstart:  %d\n",*chanstart);
		fprintf(outfp,"dbg2       chanend:    %d\n",*chanend);
		fprintf(outfp,"dbg2       timesweep:  %f\n",*timesweep);
		fprintf(outfp,"dbg2       timedelay:  %f\n",*timedelay);
		fprintf(outfp,"dbg2       startlon:   %f\n",*startlon);
		fprintf(outfp,"dbg2       startlat:   %f\n",*startlat);
		fprintf(outfp,"dbg2       endlon:     %f\n",*endlon);
		fprintf(outfp,"dbg2       endlat:     %f\n",*endlat);
		fprintf(outfp,"dbg2       error:      %d\n",*error);
		fprintf(outfp,"dbg2  Return status:\n");
		fprintf(outfp,"dbg2       status:     %d\n",status);
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
#ifdef GMT_MINOR_VERSION
	GMT_LONG	pad[4];
#else
	int	pad[4];
#endif
	time_t	right_now;
	char	date[MB_PATH_MAXLINE], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];
	char	remark[MB_PATH_MAXLINE];
	char	*ctime();
	char	*getenv();
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(outfp,"dbg2  Input arguments:\n");
		fprintf(outfp,"dbg2       verbose:    %d\n",verbose);
		fprintf(outfp,"dbg2       outfile:    %s\n",outfile);
		fprintf(outfp,"dbg2       grid:       %p\n",(void *)grid);
		fprintf(outfp,"dbg2       nx:         %d\n",nx);
		fprintf(outfp,"dbg2       ny:         %d\n",ny);
		fprintf(outfp,"dbg2       xmin:       %f\n",xmin);
		fprintf(outfp,"dbg2       xmax:       %f\n",xmax);
		fprintf(outfp,"dbg2       ymin:       %f\n",ymin);
		fprintf(outfp,"dbg2       ymax:       %f\n",ymax);
		fprintf(outfp,"dbg2       zmin:       %f\n",zmin);
		fprintf(outfp,"dbg2       zmax:       %f\n",zmax);
		fprintf(outfp,"dbg2       dx:         %f\n",dx);
		fprintf(outfp,"dbg2       dy:         %f\n",dy);
		fprintf(outfp,"dbg2       xlab:       %s\n",xlab);
		fprintf(outfp,"dbg2       ylab:       %s\n",ylab);
		fprintf(outfp,"dbg2       zlab:       %s\n",zlab);
		fprintf(outfp,"dbg2       titl:       %s\n",titl);
		fprintf(outfp,"dbg2       argc:       %d\n",(int)argc);
		fprintf(outfp,"dbg2       *argv:      %p\n",(void *)*argv);
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
	grd.node_offset = 1; /* pixel registration */
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
	strncpy(date,ctime(&right_now),24);date[24]='\0';
	if ((user_ptr = getenv("USER")) == NULL)
		user_ptr = getenv("LOGNAME");
	if (user_ptr != NULL)
		strcpy(user,user_ptr);
	else
		strcpy(user, "unknown");
	gethostname(host,MB_PATH_MAXLINE);
	sprintf(remark,"\n\tProjection: %s\n\tGrid created by %s\n\tMB-system Version %s\n\tRun by <%s> on <%s> at <%s>",
		projection,program_name,MB_VERSION,user,host,date);
	strncpy(grd.remark, remark, 159);

	/* set extract wesn,pad */
	w = 0.0;
	e = 0.0;
	s = 0.0;
	n = 0.0;
	for (i=0;i<4;i++)
		pad[i] = 0;

	/* write grid to GMT netCDF grd file */
/*for (i=0;i<nx;i++)
for (j=0;j<ny;j++)
{
k = j * nx + i;
fprintf(outfp,"%d %d %d %f\n",i,j,k,grid[k]);
}*/
	GMT_write_grd(outfile, &grd, grid, w, e, s, n, pad, FALSE);

	/* free GMT memory */
	GMT_free ((void *)GMT_io.skip_if_NaN);
	GMT_free ((void *)GMT_io.in_col_type);
	GMT_free ((void *)GMT_io.out_col_type);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(outfp,"dbg2  Return values:\n");
		fprintf(outfp,"dbg2       error:      %d\n",*error);
		fprintf(outfp,"dbg2  Return status:\n");
		fprintf(outfp,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
