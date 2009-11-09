/*--------------------------------------------------------------------
 *    The MB-system:	mbsegypsd.c	11/2/2009
 *    $Id: mbsegypsd.c 1770 2009-10-19 17:16:39Z caress $
 *
 *    Copyright (c) 2009-2009 by
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
 * mbsegypsd calculates the power spectral densisty function of each trace in a 
 * segy files, outputing the PSD as a GMT grid file with trace number along
 * the x axis and frequeny along the y axis.
 *
 * Author:	D. W. Caress
 * Date:	November 2, 2009
 *
 * $Log: mbsegypsd.c,v $
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

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_segy.h"

/* GMT include files */
#include "gmt.h"

/* FFTW include files */
#include "fftw3.h"

/* local options */
#define MBSEGYPSD_USESHOT		0
#define MBSEGYPSD_USECMP		1
#define MBSEGYPSD_WINDOW_OFF		0
#define MBSEGYPSD_WINDOW_ON		1
#define MBSEGYPSD_WINDOW_SEAFLOOR	2
#define MBSEGYPSD_WINDOW_DEPTH		3

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
		int *error);
char	*ctime();
char	*getenv();

/* output stream for basic stuff (stdout if verbose <= 1,
	stderr if verbose > 1) */
FILE	*outfp;

static char rcs_id[] = "$Id: mbsegypsd.c 1770 2009-10-19 17:16:39Z caress $";
char program_name[] = "mbsegypsd";
char help_message[] =  "mbsegypsd calculates the power spectral density function of each trace in a segy data file, \noutputting the results as a GMT grid file.";
char usage_message[] = "mbsegypsd -Ifile -Oroot [-Ashotscale/frequencyscale \n\
          -Ddecimatex -R \n\
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

	/* segy data */
	char	segyfile[MB_PATH_MAXLINE];
	void	*mbsegyioptr;
	struct mb_segyasciiheader_struct asciiheader;
	struct mb_segyfileheader_struct fileheader;
	struct mb_segytraceheader_struct traceheader;
	float	*trace = NULL;
	float	*worktrace = NULL;
	double	*spsd = NULL;
	double	*wpsd = NULL;
	double	*spsdtot = NULL;
	double	*wpsdtot = NULL;
	
	/* fft controls */
	int		nfft = 1024;
	fftw_plan 	plan;
	fftw_complex	*fftw_in = NULL;
	fftw_complex	*fftw_out = NULL;
	int		nsection;

	/* grid controls */
	char	fileroot[MB_PATH_MAXLINE];
	char	gridfile[MB_PATH_MAXLINE];
	char	psdfile[MB_PATH_MAXLINE];
	int	decimatex = 1;
	int	tracemode = MBSEGYPSD_USESHOT;
	int	tracestart = 0;
	int	traceend = 0;
	int	chanstart = 0;
	int	chanend = -1;
	double	timesweep = 0.0;
	double	timedelay = 0.0;
	double	sampleinterval = 0.0;
	int	windowmode = MBSEGYPSD_WINDOW_OFF;
	double	windowstart, windowend;
	int	ntraces;
	int	ngridx = 0;
	int	ngridy = 0;
	int	ngridxy = 0;
	float	*grid = NULL;
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
	double	frequencyscale = 1.0;
	int	logscale = MB_NO;

	int	sinftracemode = MBSEGYPSD_USESHOT;
	int	sinftracestart = 0;
	int	sinftraceend = 0;
	int	sinfchanstart = 0;
	int	sinfchanend = -1;
	double	sinftimesweep = 0.0;
	double	sinftimedelay = 0.0;
	
	double	soundpressurelevel;
	
	double	sint, taper;
	double	norm, normraw, normtaper, normfft;

	FILE	*fp;
	int	nread;
	int	tracecount, tracenum, channum, traceok;
	double	tracemin, tracemax;
	double	xwidth, ywidth;
	int	ix, iy, iys;
	int	itstart, itend;
	double	factor, btime, stime, dtime;
	double	btimesave = 0.0;
	double	stimesave = 0.0;
	double	dtimesave = 0.0;
	int	plot_status;
	int	kstart, kend;
	int	i, j, k, n;

	/* set file to null */
	segyfile[0] = '\0';

	/* get NaN value */
	GMT_make_fnan(NaN);

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:D:d:I:i:LlN:n:O:o:PpS:s:T:t:VvW:w:Hh")) != -1)
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
			n = sscanf (optarg,"%lf/%lf", &shotscale, &frequencyscale);
			if (n == 2)
				scale2distance = MB_YES;
			flag++;
			break;
		case 'D':
		case 'd':
			n = sscanf (optarg,"%d", &decimatex);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", segyfile);
			flag++;
			break;
		case 'L':
		case 'l':
			logscale = MB_YES;
			flag++;
			break;
		case 'N':
		case 'n':
			n = sscanf (optarg,"%d", &nfft);
			flag++;
			break;
		case 'G':
		case 'O':
		case 'o':
			sscanf (optarg,"%s", fileroot);
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
				tracemode = MBSEGYPSD_USESHOT;
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
	if (verbose >= 2)
		{
		fprintf(outfp,"\ndbg2  Program <%s>\n",program_name);
		fprintf(outfp,"dbg2  Version %s\n",rcs_id);
		fprintf(outfp,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(outfp,"dbg2  Control Parameters:\n");
		fprintf(outfp,"dbg2       verbose:        %d\n",verbose);
		fprintf(outfp,"dbg2       help:           %d\n",help);
		fprintf(outfp,"dbg2       segyfile:       %s\n",segyfile);
		fprintf(outfp,"dbg2       fileroot:       %s\n",fileroot);
		fprintf(outfp,"dbg2       nfft:           %d\n",nfft);
		fprintf(outfp,"dbg2       decimatex:      %d\n",decimatex);
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
		fprintf(outfp,"dbg2       scale2distance: %d\n",scale2distance);
		fprintf(outfp,"dbg2       shotscale:      %f\n",shotscale);
		fprintf(outfp,"dbg2       frequencyscale: %f\n",frequencyscale);
		fprintf(outfp,"dbg2       logscale:       %d\n",logscale);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(outfp,"\n%s\n",help_message);
		fprintf(outfp,"\nusage: %s\n", usage_message);
		exit(error);
		}
		
	/* get segy limits if required */
	if (traceend < 1 || traceend < tracestart || timesweep <= 0.0)
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
	strcpy(psdfile,fileroot);
	strcat(psdfile,"_psd.txt");
	if (chanend >= chanstart)
		ntraces = (traceend - tracestart + 1) * (chanend - chanstart + 1);
	else
		ntraces = traceend - tracestart + 1;
	ngridx = ntraces / decimatex;
	sampleinterval = 0.000001 * (double) (fileheader.sample_interval);
	ngridy = nfft / 2 + 1;
	ngridxy = ngridx * ngridy;
	dx = decimatex;
	xmin = (double) tracestart - 0.5;
	xmax = (double) traceend + 0.5;
	dy = 1.0 / (2.0 * sampleinterval * ngridy);
	ymin = -0.5 * dy;
	ymax = (ngridy - 0.5) * dy;
	
	/* get start and end samples */
	if (windowmode == MBSEGYPSD_WINDOW_OFF)
		{
		itstart = 0;
		itend = ngridy - 1;
		}
	else if (windowmode == MBSEGYPSD_WINDOW_ON)
		{
		itstart = MAX((windowstart) / sampleinterval, 0);
		itend = MIN((windowend) / sampleinterval, ngridy - 1);
		}		
	
	/* allocate memory for grid array */
	status = mb_mallocd(verbose,__FILE__,__LINE__, 2 * ngridxy * sizeof(float), (void **)&grid, &error);
	status = mb_mallocd(verbose,__FILE__,__LINE__, ngridy * sizeof(double), (void **)&spsd, &error);
	status = mb_mallocd(verbose,__FILE__,__LINE__, ngridy * sizeof(double), (void **)&wpsd, &error);
	status = mb_mallocd(verbose,__FILE__,__LINE__, ngridy * sizeof(double), (void **)&spsdtot, &error);
	status = mb_mallocd(verbose,__FILE__,__LINE__, ngridy * sizeof(double), (void **)&wpsdtot, &error);

	/* zero working psd array */
	for (iy=0;iy<ngridy;iy++)
		{
		spsdtot[iy] = 0.0;
		wpsdtot[iy] = 0.0;
		}

	/* output info */
	if (verbose >= 0)
		{
		fprintf(outfp,"\nMBsegypsd Parameters:\n");
		fprintf(outfp,"Input segy file:         %s\n",segyfile);
		fprintf(outfp,"Output fileroot:         %s\n",fileroot);
		fprintf(outfp,"Input Parameters:\n");
		fprintf(outfp,"     trace mode:         %d\n",tracemode);
		fprintf(outfp,"     trace start:        %d\n",tracestart);
		fprintf(outfp,"     trace end:          %d\n",traceend);
		fprintf(outfp,"     channel start:      %d\n",chanstart);
		fprintf(outfp,"     channel end:        %d\n",chanend);
		fprintf(outfp,"     trace decimation:   %d\n",decimatex);
		fprintf(outfp,"     time sweep:         %f seconds\n",timesweep);
		fprintf(outfp,"     time delay:         %f seconds\n",timedelay);
		fprintf(outfp,"     sample interval:    %f seconds\n",sampleinterval);
		fprintf(outfp,"     window mode:        %d\n",windowmode);
		fprintf(outfp,"     window start:       %f seconds\n",windowstart);
		fprintf(outfp,"     window end:         %f seconds\n",windowend);
		fprintf(outfp,"Output Parameters:\n");
		fprintf(outfp,"     grid filename:      %s\n",gridfile);
		fprintf(outfp,"     psd filename:       %s\n",psdfile);
		fprintf(outfp,"     x grid dimension:   %d\n",ngridx);
		fprintf(outfp,"     y grid dimension:   %d\n",ngridy);
		fprintf(outfp,"     grid xmin:          %f\n",xmin);
		fprintf(outfp,"     grid xmax:          %f\n",xmax);
		fprintf(outfp,"     grid ymin:          %f\n",ymin);
		fprintf(outfp,"     grid ymax:          %f\n",ymax);
		fprintf(outfp,"     NaN values used to flag regions with no data\n");
		fprintf(outfp,"     shotscale:          %f\n",shotscale);
		fprintf(outfp,"     frequencyscale:     %f\n",frequencyscale);
		if (scale2distance == MB_YES)
			{
			fprintf(outfp,"     trace numbers scaled to distance in meters\n");
			fprintf(outfp,"     scaled grid xmin    %f\n",0.0);
			fprintf(outfp,"     scaled grid xmax:   %f\n",shotscale * (xmax - xmin));
			}
		}
	if (verbose > 0)
		fprintf(outfp,"\n");
	
	/* proceed if all ok */
	if (status == MB_SUCCESS)
		{
	
		/* fill grid with NaNs */
		for (i=0;i<ngridxy;i++)
			grid[i] = NaN;
			
		/* generate the fftw plan */
		fftw_in = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft);
		fftw_out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft);
		plan = fftw_plan_dft_1d(nfft, fftw_in, fftw_out, FFTW_FORWARD, FFTW_MEASURE);

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
				if (tracemode == MBSEGYPSD_USESHOT)
					{
					tracenum = traceheader.shot_num;
					channum = traceheader.shot_tr;
					}
				else
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
					if (tracemode == MBSEGYPSD_USESHOT) 
						fprintf(outfp,"read:%d position:%d shot:%d channel:%d ",
							nread,tracecount,tracenum,channum);
					else 
						fprintf(outfp,"read:%d position:%d rp:%d channel:%d ",
							nread,tracecount,tracenum,channum);
					fprintf(outfp,"%4.4d/%3.3d %2.2d:%2.2d:%2.2d.%3.3d samples:%d interval:%d usec minmax: %f %f\n",
					traceheader.year,traceheader.day_of_yr,
					traceheader.hour,traceheader.min,traceheader.sec,traceheader.mils,
					traceheader.nsamps,traceheader.si_micros,
					tracemin, tracemax);
					}

				/* now actually process traces of interest */
				if (traceok == MB_YES)
					{
					/* zero working psd array */
					for (iy=0;iy<ngridy;iy++)
						{
						spsd[iy] = 0.0;
						wpsd[iy] = 0.0;
						}

					/* get bounds of trace in depth window mode */
					if (windowmode == MBSEGYPSD_WINDOW_DEPTH)
						{
						itstart = (int)((dtime + windowstart - timedelay) / sampleinterval);
						itstart = MAX(itstart, 0);
						itend = (int)((dtime + windowend - timedelay) / sampleinterval);
						itend = MIN(itend, ngridy - 1);
						}
					else if (windowmode == MBSEGYPSD_WINDOW_SEAFLOOR)
						{
						itstart = MAX((stime + windowstart - timedelay) / sampleinterval, 0);
						itend = MIN((stime + windowend - timedelay) / sampleinterval, ngridy - 1);
						}
						
					/* loop over the data calculating fft in nfft long sections */
					nsection = (itend - itstart + 1) / nfft;
					if (((itend - itstart + 1) % nfft) > 0)
						nsection++;
					for (j=0;j<nsection;j++)
						{
						/* initialize normalization factors */
						normraw = 0.0;
						normtaper = 0.0;
						normfft = 0.0;
						
						/* extract data section to be fft'd with taper */
						kstart = itstart + j * nfft;
						kend = MIN(kstart + nfft, itend);
						for (i=0;i<nfft;i++)
							{
							k = itstart + j * nfft + i;
							if (k <= kend)
								{
								sint = sin(M_PI * ((double)(k - kstart)) / ((double)(kend - kstart)));
								taper = sint * sint;
								fftw_in[i][0] = taper * trace[k];
								normraw += trace[k] * trace[k];
								normtaper += fftw_in[i][0] * fftw_in[i][0];
								}
							else
								fftw_in[i][0] = 0.0;
/*if (ix < 500)
fftw_in[i][0] = sin(2.0 * M_PI * 1000.0 * i * sampleinterval) 
			+ sin(2.0 * M_PI * 3000.0 * i * sampleinterval) 
			+ sin(2.0 * M_PI * 6000.0 * i * sampleinterval);*/
							fftw_in[i][1] = 0.0;
							}
						soundpressurelevel = 20.0 * log10(normraw / nfft);
fprintf(stderr,"Sound Pressure Level: %f dB re 1 uPa\n",soundpressurelevel);
							
						/* execute the fft */
						fftw_execute(plan);
						
						/* get normalization factor - require variance of transform to equal variance of input */
						for (i=1;i<nfft;i++)
							{
							normfft += fftw_out[i][0] * fftw_out[i][0] + fftw_out[i][1] * fftw_out[i][1];
							}
						norm = normraw / normfft;
						
						/* apply normalization factor */
						for (i=1;i<nfft;i++)
							{
							fftw_out[i][0] = norm * fftw_out[i][0];
							fftw_out[i][1] = norm * fftw_out[i][1];
							}
							
						/* calculate psd from result of transform */
						spsd[0] += fftw_out[0][0] * fftw_out[0][0] + fftw_out[0][1] * fftw_out[0][1];
						wpsd[0] += 1.0;
/* fprintf(stderr,"FFT result: i:%d  %f %f  %f\n",
0,fftw_out[0][0],fftw_out[0][1],fftw_out[0][0] * fftw_out[0][0] + fftw_out[0][1] * fftw_out[0][1]);*/
						for (i=1;i<nfft/2;i++)
							{
							spsd[i] += 2.0 * (fftw_out[i][0] * fftw_out[i][0] + fftw_out[i][1] * fftw_out[i][1]);
							wpsd[i] += 1.0;
/* fprintf(stderr,"FFT result: i:%d  %f %f  %f\n",
i,fftw_out[i][0],fftw_out[i][1],2.0 * fftw_out[i][0] * fftw_out[i][0] + fftw_out[i][1] * fftw_out[i][1]);*/
							}
						if (nfft % 2 == 0)
							{
							spsd[i] += fftw_out[nfft/2][0] * fftw_out[nfft/2][0] + fftw_out[nfft/2][1] * fftw_out[nfft/2][1];
							wpsd[i] += 1.0;
/* fprintf(stderr,"FFT result: i:%d  %f %f  %f\n",
nfft/2,fftw_out[nfft/2][0],fftw_out[nfft/2][1],fftw_out[nfft/2][0] * fftw_out[nfft/2][0] + fftw_out[nfft/2][1] * fftw_out[nfft/2][1]); */
							}
						}
						
					/* output psd for this trace to the grid */
fprintf(stderr,"N:%d Normalization: %f %f %f    ratios: %f %f     %f %f\n",
nfft,normraw,normtaper,normfft,normraw/normfft,normfft/normraw,normtaper/normfft,normfft/normtaper);
					for (iy=0;iy<ngridy;iy++)
						{
						k = (ngridy - 1 - iy) * ngridx + ix;
						if (wpsd[iy] > 0.0)
							{
							spsdtot[iy] += spsd[iy];
							wpsdtot[iy] += wpsd[iy];
							if (logscale == MB_NO)
								grid[k] = spsd[iy] / wpsd[iy];
							else
								grid[k] = 20.0 * log10(spsd[iy] / wpsd[iy]);
/*fprintf(stderr,"ix:%d iy:%d k:%d spsd:%f wpsd:%f     f:%f p:%f\n",
ix,iy,k,spsd[iy],wpsd[iy],ymax * iy / ngridy,grid[k]);*/
							gridmintot = MIN(grid[k], gridmintot);
							gridmaxtot = MAX(grid[k], gridmaxtot);
							}
						}
					}
				}

			/* now process the trace */
			if (status == MB_SUCCESS)
				nread++;
			}
			
		/* deallocate fftw arrays and plan */
		fftw_destroy_plan(plan);
		fftw_free(fftw_in);
		fftw_free(fftw_out);
		}

	/* write out the grid */
	error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;
	strcpy(projection, "GenericLinear");
	if (scale2distance == MB_YES)
		{
		strcpy(xlabel, "Distance (m)");
		strcpy(ylabel, "Frequency (Hz)");
		xmax *= shotscale;
		xmin *= shotscale;
		dx *= shotscale;
		}
	else
		{
		strcpy(xlabel, "Trace Number");
		strcpy(ylabel, "Frequency (Hz)");
		dx = (double) decimatex;
		}
	strcpy(zlabel, "dB/Hz");
	sprintf(title, "Power Spectral Density Grid from %s", segyfile);
	status = write_cdfgrd(verbose, gridfile, grid,
		ngridx, ngridy, 
		xmin, xmax, ymin, ymax,
		gridmintot, gridmaxtot, dx, dy, 
		xlabel, ylabel, zlabel, title, 
		projection, argc, argv, &error);

	/* output average power spectra */
	if ((fp = fopen(psdfile, "w")) != NULL)
		{
		for (iy=0;iy<ngridy;iy++)
			{
			if (wpsd[iy] > 0.0)
				{
				spsdtot[iy] = spsd[iy] / wpsd[iy];
				}
			fprintf(fp, "%f %f\n", dy * iy, spsdtot[iy]);
			}
		fclose(fp);
		}

	/* close the segy file */
	status = mb_segy_close(verbose,&mbsegyioptr,&error);

	/* deallocate memory for grid array */
	if (worktrace != NULL)
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&worktrace, &error);	
	status = mb_freed(verbose,__FILE__,__LINE__,(void **)&grid, &error);
	status = mb_freed(verbose,__FILE__,__LINE__,(void **)&spsd, &error);
	status = mb_freed(verbose,__FILE__,__LINE__,(void **)&wpsd, &error);
	status = mb_freed(verbose,__FILE__,__LINE__,(void **)&spsdtot, &error);
	status = mb_freed(verbose,__FILE__,__LINE__,(void **)&wpsdtot, &error);
	
	/* run mbm_grdplot */
	xwidth = MIN(0.01 * (double) ngridx, 55.0);
	ywidth = MIN(0.01 * (double) ngridy, 28.0);
	sprintf(plot_cmd, "mbm_grdplot -I%s -JX%f/%f -G1 -S -V -L\"File %s - %s:%s\"", 
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
	
	/* run mbm_xyplot */
	xwidth = 9.0;
	ywidth = 7.0;
	sprintf(plot_cmd, "mbm_xyplot -I%s -JX%f/%f -V -L\"File %s - %s:%s\"", 
			psdfile, xwidth, ywidth, psdfile, title, zlabel);
	if (verbose)
		{
		fprintf(outfp, "\nexecuting mbm_xyplot...\n%s\n", 
			plot_cmd);
		}
	plot_status = system(plot_cmd);
	if (plot_status == -1)
		{
		fprintf(outfp, "\nError executing mbm_xyplot on psd file %s\n", psdfile);
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
	double	delay0, delay1, delaydel;
	int	shot0, shot1, shotdel;
	int	shottrace0, shottrace1, shottracedel;
	int	rp0, rp1, rpdel;
	int	rptrace0, rptrace1, rptracedel;
	int	nscan;

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
		system(command);
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
		    }
		fclose(sfp);
		}
		
	/* set the trace mode */
	if (rpdel > 1)
		{
		*tracemode = MBSEGYPSD_USECMP;
		*tracestart = rp0;
		*traceend = rp1;
		*chanstart = rptrace0;
		*chanend = rptrace1;
		}
	else
		{
		*tracemode = MBSEGYPSD_USESHOT;
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
	GMT_LONG	pad[4];
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
		fprintf(outfp,"dbg2       grid:       %d\n",(int)grid);
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
		fprintf(outfp,"dbg2       *argv:      %ld\n",(long)*argv);
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
