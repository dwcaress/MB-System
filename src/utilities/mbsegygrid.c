/*--------------------------------------------------------------------
 *    The MB-system:	mbsegygrid.c	6/12/2004
 *    $Id: mbsegygrid.c,v 5.0 2004-06-18 04:06:05 caress Exp $
 *
 *    Copyright (c) 2004 by
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
 * $Log: not supported by cvs2svn $
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Includes for System 5 type operating system */
#if defined (IRIX) || defined (LYNX)
#include <stdlib.h>
#endif

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_segy.h"

/* GMT include files */
#include "gmt.h"

/* local options */
#define MBSEGYGRID_USESHOT	0
#define MBSEGYGRID_USECMP	1

/* NaN value */
double	NaN;

int write_cdfgrd(int verbose, char *outfile, float *grid,
		int nx, int ny, 
		double xmin, double xmax, double ymin, double ymax,
		double zmin, double zmax, double dx, double dy, 
		char *xlab, char *ylab, char *zlab, char *titl, 
		char *projection, int argc, char **argv, 
		int *error);
char	*ctime();
char	*getenv();

static char rcs_id[] = "$Id: mbsegygrid.c,v 5.0 2004-06-18 04:06:05 caress Exp $";

static char program_name[] = "MBsegygrid";
static char help_message[] =  "MBsegygrid grids trace data from segy data files.";
static char usage_message[] = "MBsegygrid -Ifile -Ogridfile [-Ddecimatex/decimatey -Smode[/start/end[/schan/echan]] -Tsweep[/delay] -H -V]";

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
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

	/* segy data */
	char	segyfile[MB_PATH_MAXLINE];
	void	*mbsegyioptr;
	struct mb_segyasciiheader_struct asciiheader;
	struct mb_segyfileheader_struct fileheader;
	struct mb_segytraceheader_struct traceheader;
	float	*trace = NULL;

	/* grid controls */
	char	gridfile[MB_PATH_MAXLINE];
	int	decimatex = 1;
	int	decimatey = 1;
	int	tracemode = MBSEGYGRID_USESHOT;
	int	tracestart = 0;
	int	traceend = 0;
	int	chanstart = 0;
	int	chanend = -1;
	double	timesweep = 0.0;
	double	timedelay = 0.0;
	double	sampleinterval = 0.0;
	int	nsamples;
	int	ntraces;
	int	ngridx = 0;
	int	ngridy = 0;
	int	ngridxy = 0;
	float	*grid = NULL;
	double	xmin;
	double	xmax;
	double	ymin;
	double	ymax;
	double	tracemin = 0.0;
	double	tracemax = 0.0;
	char	projection[MB_PATH_MAXLINE];
	char	xlabel[MB_PATH_MAXLINE];
	char	ylabel[MB_PATH_MAXLINE];
	char	zlabel[MB_PATH_MAXLINE];
	char	title[MB_PATH_MAXLINE];

	int	nread;
	int	tracecount, tracenum, channum, traceok;
	int	ix, iy, iys;
	double	factor, stime;
	int	i, j, k, n;

	/* set file to null */
	segyfile[0] = '\0';

	/* get NaN value */
#ifdef GMT3_0
	NaN = zero/zero;
#else
	GMT_make_dnan(NaN);
#endif

	/* process argument list */
	while ((c = getopt(argc, argv, "D:d:I:i:O:o:S:s:T:t:VvWwHh")) != -1)
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
		case 'D':
		case 'd':
			n = sscanf (optarg,"%d/%d", &decimatex, &decimatey);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", segyfile);
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", gridfile);
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
		fprintf(stderr,"dbg2       segyfile:       %s\n",segyfile);
		fprintf(stderr,"dbg2       gridfile:       %s\n",gridfile);
		fprintf(stderr,"dbg2       decimatex:      %d\n",decimatex);
		fprintf(stderr,"dbg2       decimatey:      %d\n",decimatey);
		fprintf(stderr,"dbg2       tracemode:      %d\n",tracemode);
		fprintf(stderr,"dbg2       tracestart:     %d\n",tracestart);
		fprintf(stderr,"dbg2       traceend:       %d\n",traceend);
		fprintf(stderr,"dbg2       chanstart:      %d\n",chanstart);
		fprintf(stderr,"dbg2       chanend:        %d\n",chanend);
		fprintf(stderr,"dbg2       timesweep:      %f\n",timesweep);
		fprintf(stderr,"dbg2       timedelay:      %f\n",timedelay);
		fprintf(stderr,"dbg2       ngridx:         %d\n",ngridx);
		fprintf(stderr,"dbg2       ngridy:         %d\n",ngridy);
		fprintf(stderr,"dbg2       ngridxy:        %d\n",ngridxy);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}
		
	/* check specified parameters */
	if (traceend < 1 || traceend < tracestart)
		{
		fprintf(stderr,"\nBad trace numbers: %d %d specified...\n", tracestart, traceend);
		fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
		exit(error);
		}
	if (timesweep <= 0.0)
		{
		fprintf(stderr,"\nBad time sweep: %f specified...\n", timesweep);
		fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
		exit(error);
		}

	/* initialize reading the segy file */
	if (mb_segy_read_init(verbose, segyfile, 
		&mbsegyioptr, &asciiheader, &fileheader, &error) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_segy_read_init>:\n%s\n",message);
		fprintf(stderr,"\nSEGY File <%s> not initialized for reading\n",segyfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
		
	/* calculate implied grid parameters */
	if (chanend > chanstart)
		ntraces = (traceend - tracestart + 1) * (chanend - chanstart + 1);
	else
		ntraces = traceend - tracestart + 1;
	ngridx = ntraces / decimatex;
	sampleinterval = 0.000001 * (double) (fileheader.sample_interval);
	ngridy = timesweep / sampleinterval / decimatey + 1;
	ngridxy = ngridx * ngridy;
	xmin = (double) tracestart - 0.5;
	xmax = (double) traceend + 0.5;
	ymax = -(timedelay - 0.5 * sampleinterval / decimatey);
	ymin = ymax - ngridy * sampleinterval;
	/*ymax = timedelay + timesweep + 0.5 * sampleinterval / decimatey;*/
	
	/* allocate memory for grid array */
	status = mb_malloc(verbose, 2 * ngridxy * sizeof(float), &grid, &error);
	
	/* proceed if all ok */
	if (status == MB_SUCCESS)
		{
	
		/* fill grid with NaNs */
		for (i=0;i<ngridxy;i++)
			grid[i] = NaN;

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
				/* figure out where this trace is in the grid */
				if (tracemode == MBSEGYGRID_USESHOT)
					{
					tracenum = traceheader.shot_num;
					channum = traceheader.shot_tr;
					}
				else
					{
					tracenum = traceheader.rp_num;
					channum = traceheader.rp_tr;
					}
				if (chanend > chanstart)
					{
					tracecount = (tracenum - tracestart) * (chanend - chanstart + 1)
							+ (channum - chanstart);
					}
				else
					{
					tracecount = tracenum - tracestart;
					}
				ix = tracecount / decimatex;
				if (traceheader.elev_scalar < 0)
					factor = 1.0 / ((float) (-traceheader.elev_scalar));
				else
					factor = (float) traceheader.elev_scalar;
				stime = factor * traceheader.src_depth / 750.0 + 0.001 * traceheader.delay_mils;
				iys = (stime - timedelay) / sampleinterval;
				
				/* now check if this is a trace of interest */
				traceok = MB_YES;
				if (tracenum < tracestart 
					|| tracenum > traceend)
					traceok = MB_NO;
				else if (chanend > chanstart
						&& (channum < chanstart
							|| channum > chanend))
					traceok = MB_NO;
				else if (tracecount % decimatex != 0)
					traceok = MB_NO;
				if (verbose > 0)
					{
					if (traceok == MB_YES) 
						fprintf(stderr,"PROCESS ");
					else 
						fprintf(stderr,"IGNORE  ");
					if (tracemode == MBSEGYGRID_USESHOT) 
						fprintf(stderr,"read:%d position:%d shot:%d channel:%d ",
							nread,tracecount,tracenum,channum);
					else 
						fprintf(stderr,"read:%d position:%d rp:%d channel:%d ",
							nread,tracecount,tracenum,channum);
					fprintf(stderr,"%4.4d/%3.3d %2.2d:%2.2d:%2.2d.%3.3d samples:%d interval:%d usec\n",
					traceheader.year,traceheader.day_of_yr,
					traceheader.hour,traceheader.min,traceheader.sec,traceheader.mils,
					traceheader.nsamps,traceheader.si_micros);
					}

				/* now actually process traces of interest */
				if (traceok == MB_YES)
					{
					for (i=0;i<traceheader.nsamps;i+=decimatey)
						{
						iy = iys + i / decimatey;
						k = iy * ngridx + ix;
						if (iy >= 0 && iy < ngridy)
							{
							grid[k] = trace[i];
							tracemin = MIN(trace[i], tracemin);
							tracemax = MAX(trace[i], tracemax);
							}
						}
					}
				}

			/* now process the trace */
			if (status == MB_SUCCESS)
				nread++;
			}
		}

	/* write out the grid */
	error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;
	strcpy(projection, "SeismicProfile");
	strcpy(xlabel, "Trace Number");
	strcpy(ylabel, "Time (seconds)");
	strcpy(zlabel, "Trace Signal");
	sprintf(title, "SEGY File: %s", segyfile);
	status = write_cdfgrd(verbose, gridfile, grid,
		ngridx, ngridy, 
		xmin, xmax, ymin, ymax,
		tracemin, tracemax, (double) decimatex, sampleinterval, 
		xlabel, ylabel, zlabel, title, 
		projection, argc, argv, &error);
	

	/* close the swath file */
	status = mb_segy_close(verbose,&mbsegyioptr,&error);

	/* deallocate memory for grid array */
	status = mb_free(verbose, &grid, &error);

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
	time_t	right_now;
	char	date[MB_PATH_MAXLINE], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];
	char	*message;
	char	*ctime();
	char	*getenv();
	int	i;

	/* print input debug statements */
	if (verbose >= 0)
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
#ifdef GMT3_0
	grdio_init();
	grd_init (&grd, argc, argv, MB_NO);
#else
	GMT_grdio_init();
	GMT_grd_init (&grd, argc, argv, MB_NO);
#endif

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

	/* write grid to GMT netCDF grd file */
#ifdef GMT3_0
	write_grd(outfile, &grd, grid, w, e, s, n, pad, complex);
#else
	GMT_write_grd(outfile, &grd, grid, w, e, s, n, pad, complex);
#endif

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
