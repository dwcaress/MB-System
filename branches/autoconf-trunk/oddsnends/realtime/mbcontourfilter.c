/*--------------------------------------------------------------------
 *    The MB-system:	mbcontourfilter.c	8/13/93
 *    $Id: mbcontourfilter.c,v 4.1 1994-04-19 01:35:32 caress Exp $
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
 * MBCONTOURFILTER is a swath contouring utility designed for use with
 * an old fashioned pen plotter. The output consists of ascii pen plotting
 * calls in geographic (longitude and latitude) coordinates.  These
 * plot calls can be piped to a filter which translates them to calls 
 * which can be handled by a plotter.  The primary use of this utility
 * is as part of a shipboard realtime plotting package using four
 * color pen plotters. The contour levels and colors are controlled 
 * using contour and color change intervals. Contours can also be 
 * set to have ticks pointing downhill.
 *
 * This utility replaces the old mbcontfilter used from May 1991 to
 * August 1993 for realtime Hydrosweep plotting on the R/V Ewing.  
 * RIP to a workhorse bit of code that was pieced together in less
 * than 48 hours.
 *
 * Author:	D. W. Caress
 * Date:	August 13, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1994/03/01  20:50:45  caress
 * First cut at new version.
 *
 * Revision 3.0  1993/08/26  00:59:59  caress
 * Initial version.
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <time.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"

/* local define */
#define DTR (M_PI/180.)

/* global structure definitions */
#define MAXPINGS 1000
struct	ping
	{
	int	pings;
	int	kind;
	int	time_i[6];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	*bath;
	double	*bathlon;
	double	*bathlat;
	double	*amp;
	double	*ss;
	double	*sslon;
	double	*sslat;
	char	comment[256];
	};
struct swath
	{
	int	npings;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	struct ping data[MAXPINGS];
	};

/* global pen variables */
int	ncolor;
int	nlevel;
double	*level;
int	*label;
int	*tick;

/* global bold line width */
double	eps_inch = 0.005;
double	eps_geo;
#define IUP 3
#define IDN 2
#define IOR -3

main (argc, argv) 
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mbcontourfilter.c,v 4.1 1994-04-19 01:35:32 caress Exp $";
	static char program_name[] = "MBCONTOURFILTER";
	static char help_message[] =  "MBCONTOURFILTER is a swath contouring utility designed for use with \nan old fashioned pen plotter. The output consists of ascii pen plotting \ncalls in geographic (longitude and latitude) coordinates.  These plot \ncalls can be piped to a filter which translates them to calls which \ncan be handled by a plotter.  The primary use of this utility is as \npart of a shipboard realtime plotting package using four color pen \nplotters. The contour levels and colors are controlled using contour \nand color change intervals. Contours can also be set to have ticks \npointing downhill.";
	static char usage_message[] = "mbcontourfilter.c -Fformat -Iinfile -Rwest/east/south/north \n\t[-Atime_tick/time_annot/date_annot/time_tick_len -Byr/mo/da/hr/mn/sc \n\t-Ccontour_int/color_int/tick_int/label_int/tick_len/label_hgt \n\t-Eyr/mo/da/hr/mn/sc -Jscale -Llonflip -Nnplot \n\t-Ppings -Sspeed -Ttimegap -V -H]";

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
	int	format;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[6];
	int	etime_i[6];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[128];
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	*mbio_ptr;

	/* mbio read values */
	struct swath *swath_plot;
	struct ping *pingcur;
	double	*bath;
	double	*amp;
	double	*bathlon;
	double	*bathlat;
	double	*ss;
	double	*sslon;
	double	*sslat;

	/* gmt control variables */
	int	doplot;
	int	done;
	int	flush;
	int	save_new;
	int	first;
	int	*npings;
	int	nping_read;
	int	nping_plot;
	int	nplot;
	double	contour_int;
	double	color_int;
	double	tick_int;
	double	label_int;
	double	tick_len;
	double	label_hgt;
	double	tick_len_l;
	double	label_hgt_l;
	double	scale;
	double	time_tick_int;
	double	time_annot_int;
	double	date_annot_int;
	double	time_tick_len;
	double	time_tick_len_l;

	/* other variables */
	double	x1, y1, x2, y2, xx1, yy1, xx2, yy2;
	char	line[128];
	char	labelstr[128], tickstr[128];
	int	count;
	int	i;

	/* get current mb default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* initialize some values */
	strcpy (file, "stdin");
	nplot = 2;
	contour_int = 25.;
	color_int = 100.;
	tick_int = 100.;
	label_int = 100.;
	label_hgt = 0.1;
	tick_len = 0.05;
	scale = 1.0;
	time_tick_int = 0.25;
	time_annot_int = 1.0;
	date_annot_int = 4.0;
	time_tick_len = 0.1;
	ncolor = 4;
	nlevel = 0;

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
		fprintf(stderr,"GMT option error\n");
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_FAILURE);
		}

	/* deal with mb options */
	while ((c = getopt(argc, argv, "VvHhF:f:P:p:L:l:B:b:E:e:J:j:R:r:S:s:T:t:I:i:A:a:C:c:N:n:")) != -1)
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
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf (optarg,"%d", &pings);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf (optarg, "%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			break;
		case 'E':
		case 'e':
			sscanf (optarg, "%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
			break;
		case 'R':
		case 'r':
			sscanf (optarg, "%lf/%lf/%lf/%lf",
				&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
			break;
		case 'S':
		case 's':
			sscanf (optarg, "%lf", &speedmin);
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf", &timegap);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", file);
			flag++;
			break;
		case 'J':
		case 'j':
			sscanf (optarg,"%lf", &scale);
			flag++;
			break;
		case 'A':
		case 'a':
			sscanf (optarg, "%lf/%lf/%lf/%lf",
				&time_tick_int,&time_annot_int,
				&date_annot_int,&time_tick_len);
			break;
		case 'C':
		case 'c':
			sscanf (optarg, "%lf/%lf/%lf/%lf/%lf/%lf",
				&contour_int,&color_int,
				&tick_int,&label_int,
				&tick_len,&label_hgt);
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%d", &nplot);
			if (nplot < 3) nplot = 3;
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
		exit(MB_FAILURE);
		}

	/* print starting message */
	if (verbose == 1)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"\ncontour interval:  %f\n",contour_int);
		fprintf(stderr,"color interval:     %f\n",color_int);
		fprintf(stderr,"tick interval:      %f\n",tick_int);
		fprintf(stderr,"tick length:        %f\n",tick_len);
		fprintf(stderr,"label interval:     %f\n",label_int);
		fprintf(stderr,"label height:       %f\n",label_hgt);
		fprintf(stderr,"pings averaged:     %d\n",pings);
		fprintf(stderr,"pings contoured:    %d\n",nplot);
		fprintf(stderr,"time tick interval: %f\n",
			time_tick_int);
		fprintf(stderr,"time interval:      %f\n",
			time_annot_int);
		fprintf(stderr,"date interval:      %f\n",
			date_annot_int);
		fprintf(stderr,"time tick length:   %f\n\n",
			time_tick_len);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:            %d\n",verbose);
		fprintf(stderr,"dbg2       help:               %d\n",help);
		fprintf(stderr,"dbg2       pings:              %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:            %d\n",lonflip);
		fprintf(stderr,"dbg2       btime_i[0]:         %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:         %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:         %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:         %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:         %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:         %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       etime_i[0]:         %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:         %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:         %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:         %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:         %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:         %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       speedmin:           %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:            %f\n",timegap);
		fprintf(stderr,"dbg2       input file:         %s\n",file);
		fprintf(stderr,"dbg2       bounds[0]:          %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:          %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:          %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:          %f\n",bounds[3]);
		fprintf(stderr,"dbg2       contour interval:   %f\n",contour_int);
		fprintf(stderr,"dbg2       color interval:     %f\n",color_int);
		fprintf(stderr,"dbg2       tick interval:      %f\n",tick_int);
		fprintf(stderr,"dbg2       label interval:     %f\n",label_int);
		fprintf(stderr,"dbg2       tick length:        %f\n",tick_len);
		fprintf(stderr,"dbg2       label height:       %f\n",label_hgt);
		fprintf(stderr,"dbg2       plot scale:         %f\n",scale);
		fprintf(stderr,"dbg2       number contoured:   %d\n",nplot);
		fprintf(stderr,"dbg2       time tick interval: %f\n",
			time_tick_int);
		fprintf(stderr,"dbg2       time interval:      %f\n",
			time_annot_int);
		fprintf(stderr,"dbg2       date interval:      %f\n",
			date_annot_int);
		fprintf(stderr,"dbg2       time tick length:   %f\n",
			time_tick_len);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(MB_ERROR_NO_ERROR);
		}

	/* if bounds not specified then quit */
	if (bounds[0] >= bounds[1] || bounds[2] >= bounds[3]
		|| bounds[2] < -90.0 || bounds[3] > 90.0)
		{
		fprintf(stderr,"\nRegion bounds not properly specified:\n\t%f %f %f %f\n",bounds[0],bounds[1],bounds[2],bounds[3]);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_ERROR_BAD_PARAMETER);
		}

	/* set default colors and use contour intervals */
	nlevel = 0;
	ncolor = 4;
	tick_len_l = tick_len/scale;
	label_hgt_l = label_hgt/scale;
	time_tick_len_l = time_tick_len/scale;
	eps_geo = eps_inch/scale;

	/* initialize reading the multibeam file */
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

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,sizeof(struct swath),
			&swath_plot,&error);
	npings = &swath_plot->npings;
	for (i=0;i<MAXPINGS;i++)
		{
		pingcur = &(swath_plot->data[i]);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&(pingcur->bath),&error);
		status = mb_malloc(verbose,beams_amp*sizeof(double),
			&(pingcur->amp),&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&(pingcur->bathlon),&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&(pingcur->bathlat),&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&(pingcur->ss),&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&(pingcur->sslon),&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&(pingcur->sslat),&error);
		}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* print message */
	if (verbose >= 2) 
		fprintf(stderr,"\n");
	if (verbose >= 1)
		fprintf(stderr,"processing data in %s...\n",file);

	/* initialize plotting */
	init_graphics();

	/* plot boundary marks */
	plot (bounds[0],bounds[2],IUP);
	plot (bounds[0],bounds[3],IDN);
	plot (bounds[1],bounds[3],IDN);
	plot (bounds[1],bounds[2],IDN);
	plot (bounds[0],bounds[2],IDN);
	plot(bounds[0]-2.*time_tick_len_l,bounds[2],IUP);
	plot(bounds[0]+2.*time_tick_len_l,bounds[2],IDN);
	plot(bounds[0],bounds[2]-2.*time_tick_len_l,IUP);
	plot(bounds[0],bounds[2]+2.*time_tick_len_l,IDN);
	plot(bounds[0]-2.*time_tick_len_l,bounds[3],IUP);
	plot(bounds[0]+2.*time_tick_len_l,bounds[3],IDN);
	plot(bounds[0],bounds[3]-2.*time_tick_len_l,IUP);
	plot(bounds[0],bounds[3]+2.*time_tick_len_l,IDN);
	plot(bounds[1]-2.*time_tick_len_l,bounds[2],IUP);
	plot(bounds[1]+2.*time_tick_len_l,bounds[2],IDN);
	plot(bounds[1],bounds[2]-2.*time_tick_len_l,IUP);
	plot(bounds[1],bounds[2]+2.*time_tick_len_l,IDN);
	plot(bounds[1]-2.*time_tick_len_l,bounds[3],IUP);
	plot(bounds[1]+2.*time_tick_len_l,bounds[3],IDN);
	plot(bounds[1],bounds[3]-2.*time_tick_len_l,IUP);
	plot(bounds[1],bounds[3]+2.*time_tick_len_l,IDN);

	/* loop over reading */
	*npings = 0;
	nping_read = 0;
	nping_plot = 0;
	swath_plot->beams_bath = beams_bath;
	swath_plot->beams_amp = beams_amp;
	swath_plot->pixels_ss = pixels_ss;
	done = MB_NO;
	while (done == MB_NO)
		{
		pingcur = &swath_plot->data[*npings];
		bath = pingcur->bath;
		amp = pingcur->amp;
		bathlon = pingcur->bathlon;
		bathlat = pingcur->bathlat;
		ss = pingcur->ss;
		sslon = pingcur->sslon;
		sslat = pingcur->sslat;
		status = mb_read(verbose,mbio_ptr,&(pingcur->kind),
			&(pingcur->pings),pingcur->time_i,&(pingcur->time_d),
			&(pingcur->navlon),&(pingcur->navlat),&(pingcur->speed),
			&(pingcur->heading),&(pingcur->distance),
			&beams_bath,&beams_amp,&pixels_ss,
			bath,amp,bathlon,bathlat,
			ss,sslon,sslat,
			pingcur->comment,&error);

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",
				pingcur->kind);
			fprintf(stderr,"dbg2       beams_bath:     %d\n",
				beams_bath);
			fprintf(stderr,"dbg2       beams_amp:      %d\n",
				beams_amp);
			fprintf(stderr,"dbg2       pixels_ss:      %d\n",
				pixels_ss);
			fprintf(stderr,"dbg2       error:          %d\n",
				error);
			fprintf(stderr,"dbg2       status:         %d\n",
				status);
			}

		/* update bookkeeping */
		if (error == MB_ERROR_NO_ERROR)
			{
			nping_read += pingcur->pings;
			(*npings)++;
			}

		/* decide whether to plot, whether to 
			save the new ping, and if done */
		doplot = MB_NO; 
		flush = MB_NO;
		if (*npings >= nplot)
			doplot = MB_YES;
		if (*npings > 0 
			&& (error > MB_ERROR_NO_ERROR
			|| error == MB_ERROR_TIME_GAP
			|| error == MB_ERROR_OUT_BOUNDS
			|| error == MB_ERROR_OUT_TIME
			|| error == MB_ERROR_SPEED_TOO_SMALL))
			{
			doplot = MB_YES;
			flush = MB_YES;
			}
		save_new = MB_NO;
		if (error == MB_ERROR_TIME_GAP)
			save_new = MB_YES;
		if (error > MB_ERROR_NO_ERROR)
			done = MB_YES;
		if (verbose == MB_YES && error == MB_ERROR_NO_ERROR)
			{
			fprintf(stderr,"read status: %d  pings in buffer: %d  total pings read: %d\n",
				status,*npings,nping_read);
			fprintf(stderr,"nav:  %f %f\n",
				pingcur->navlon,pingcur->navlat);
			fprintf(stderr,"time: %d %d %d %d %d %d\n\n",
				pingcur->time_i[0],pingcur->time_i[1],
				pingcur->time_i[2],pingcur->time_i[3],
				pingcur->time_i[4],pingcur->time_i[5]);
			}

		/* if enough pings read in, plot them */
		if (doplot == MB_YES)
			{

			/* print debug statements */
			if (verbose >= 2)
				{
				fprintf(stderr,"\ndbg2  Plotting %d pings in program <%s>\n",
					*npings,program_name);
				for (i=0;i<*npings;i++)
					{
					pingcur = &swath_plot->data[i];
					fprintf(stderr,"dbg2       %4d  %4d %2d %2d %2d %2d %2d\n",
						i,pingcur->time_i[0],
						pingcur->time_i[1],
						pingcur->time_i[2],
						pingcur->time_i[3],
						pingcur->time_i[4],
						pingcur->time_i[5]);
					}
				}

			/* plot data */
			mb_contour(verbose,swath_plot,contour_int,color_int,
				tick_int,label_int,
				tick_len_l,label_hgt_l,
				ncolor,nlevel,level,label,tick,&error);
			flush_graphics();
			nping_plot++;
			if (verbose)
				fprintf(stderr,"pings plotted: %d  total plot calls: %d\n\n",
					*npings,nping_plot);

			/* plot shiptrack */
			mb_track(verbose,swath_plot,time_tick_int,
				time_annot_int,date_annot_int,
				time_tick_len_l,
				&error);
			flush_graphics();

			/* reorganize data */
			if (flush == MB_YES && save_new == MB_YES)
				{
				status = ping_copy(verbose,0,*npings,
					swath_plot,&error);
				*npings = 1;
				}
			else if (flush == MB_YES)
				{
				*npings = 0;
				}
			else if (*npings > 1)
				{
				status = ping_copy(verbose,0,*npings-1,
						swath_plot,&error);
				*npings = 1;
				}

			}
		}
	status = mb_close(verbose,mbio_ptr,&error);
	end_graphics();

	/* deallocate memory for data arrays */
	for (i=0;i<*npings;i++)
		{
		pingcur = &swath_plot->data[i];
		mb_free(verbose,pingcur->bath,&error);
		mb_free(verbose,pingcur->amp,&error);
		mb_free(verbose,pingcur->bathlon,&error);
		mb_free(verbose,pingcur->bathlat,&error);
		mb_free(verbose,pingcur->ss,&error);
		mb_free(verbose,pingcur->sslon,&error);
		mb_free(verbose,pingcur->sslat,&error);
		}
	mb_free(verbose,swath_plot,&error);

	/* deallocate memory for data arrays */
	mb_free(verbose,level,&error);
	mb_free(verbose,label,&error);
	mb_free(verbose,tick,&error);

	/* print ending info */
	if (verbose >= 1)
		fprintf(stderr,"\n%d pings read and plotted\n",
			nping_read);

	/* check memory */
	if (verbose >= 2)
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
	exit(0);
}
/*--------------------------------------------------------------------*/
int ping_copy(verbose,one,two,swath,error)
int	verbose;
int	one;
int	two;
struct swath *swath;
int	*error;
{
	char	*function_name = "ping_copy";
	int	status = MB_SUCCESS;

	struct ping	*ping1;
	struct ping	*ping2;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       one:        %d\n",one);
		fprintf(stderr,"dbg2       two:        %d\n",two);
		fprintf(stderr,"dbg2       swath:      %d\n",swath);
		fprintf(stderr,"dbg2       pings:      %d\n",swath->npings);
		}

	/* copy things */
	ping1 = &swath->data[one];
	ping2 = &swath->data[two];
	ping1->pings = ping2->pings;
	ping1->kind = ping2->kind;
	for (i=0;i<6;i++)
		ping1->time_i[i] = ping2->time_i[i];
	ping1->time_d = ping2->time_d;
	ping1->navlon = ping2->navlon;
	ping1->navlat = ping2->navlat;
	ping1->speed = ping2->speed;
	ping1->heading = ping2->heading;
	ping1->distance = ping2->distance;
	strcpy(ping1->comment,ping2->comment);
	for (i=0;i<swath->beams_bath;i++)
		{
		ping1->bath[i] = ping2->bath[i];
		ping1->bathlon[i] = ping2->bathlon[i];
		ping1->bathlat[i] = ping2->bathlat[i];
		}
	for (i=0;i<swath->beams_amp;i++)
		{
		ping1->amp[i] = ping2->amp[i];
		}
	for (i=0;i<swath->pixels_ss;i++)
		{
		ping1->ss[i] = ping2->ss[i];
		ping1->sslon[i] = ping2->sslon[i];
		ping1->sslat[i] = ping2->sslat[i];
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> completed\n",
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
int plot(x,y,ipen)
double x,y;
int ipen;
{
	printf("plot %f %f %d c\n",x,y,ipen);
	return;
}
/*--------------------------------------------------------------------*/
int plot_(x,y,ipen)
float *x,*y;
int *ipen;
{
	printf("plot %f %f %d f\n",((double) *x),((double) *y),*ipen);
	return;
}
/*--------------------------------------------------------------------*/
int newpen(ipen)
int ipen;
{
	printf("newp %d\n",ipen);
	return;
}
/*--------------------------------------------------------------------*/
int boldline(x1,y1,x2,y2)
double x1,y1,x2,y2;
{
	double	dx, dy, mag;

	dx = x2 - x1;
	dy = y2 - y1;
	mag = sqrt(dx*dx + dy*dy);
	if (mag > 0.0)
		{
		dx = eps_geo*dx/mag;
		dy = eps_geo*dy/mag;
		plot(x1,y1,IUP);
		plot(x2,y2,IDN);
		plot(x2+dy,y2-dx,IDN);
		plot(x1+dy,y1-dx,IDN);
		plot(x1-dy,y1+dx,IDN);
		plot(x2-dy,y2+dx,IDN);
		plot(x1-dy,y1+dx,IDN);
		plot(x1+dy,y1-dx,IDN);
		plot(x2+dy,y2-dx,IDN);
		plot(x2,y2,IDN);
		plot(x1,y1,IDN);
		}
	return;
}
/*--------------------------------------------------------------------*/
int justify_string(height,string,s)
double	height;
char	*string;
double	*s;
{
	float	hgtf;
	float	ss[4];
	int	len;
	int	i;

	len = strlen(string);
	for (i=0;i<len;i++)
		if (string[i] == ' ')
			string[i] = '_';
	hgtf = height;
	justify_(ss,&hgtf,string,&len);
	s[0] = ss[0];
	s[1] = ss[1];
	s[2] = ss[2];
	s[3] = ss[3];

	return;
}
/*--------------------------------------------------------------------*/
int init_graphics()
{
	printf("init\n");
	return;
}
/*--------------------------------------------------------------------*/
int end_graphics()
{
	printf("stop\n");
	return;
}
/*--------------------------------------------------------------------*/
int flush_graphics()
{
	printf("flus\n");
	fflush(stdout);
	fflush(stderr);
	return;
}
/*--------------------------------------------------------------------*/
int plot_string(x,y,hgt,angle,label)
double	x;
double	y;
double	hgt;
double	angle;
char	*label;
{
	float	xlab, ylab, hght, ang;
	int	len;
	int	i;

	xlab = x;
	ylab = y;
	hght = hgt;
	ang = angle;
	len = strlen(label);
	for (i=0;i<len;i++)
		if (label[i] == ' ')
			label[i] = '_';
	label_(&xlab,&ylab,&hght,&ang,label,&len);

	return;
}
/*--------------------------------------------------------------------*/
