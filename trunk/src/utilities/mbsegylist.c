/*--------------------------------------------------------------------
 *    The MB-system:	mbsegylist.c	5/29/2004
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
 * MBsegylist prints the specified contents of a segy data
 * file to stdout. The form of the output is quite flexible;
 * MBsegylist is tailored to produce ascii files in spreadsheet
 * style with data columns separated by tabs.
 *
 * Author:	D. W. Caress
 * Date:	May 29, 2004
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

/* MBIO include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_segy.h"

/* local options */
#define	MAX_OPTIONS	25
#define	MBLIST_CHECK_ON			0
#define	MBLIST_CHECK_ON_NULL		1
#define	MBLIST_CHECK_OFF_RAW		2
#define	MBLIST_CHECK_OFF_NAN		3
#define	MBLIST_CHECK_OFF_FLAGNAN	4
#define	MBLIST_SET_OFF	0
#define	MBLIST_SET_ON	1
#define	MBLIST_SET_ALL	2

/* NaN value */
double	NaN;

/* function prototypes */
int printsimplevalue(int verbose,
	double value, int width, int precision,
	int ascii, int *invert, int *flipsign, int *error);
int printNaN(int verbose, int ascii, int *invert, int *flipsign, int *error);

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "MBsegylist";
	char help_message[] =  "MBsegylist lists table data from a segy data file.";
	char usage_message[] = "MBsegylist -Ifile [-A -Ddecimate -Gdelimiter -Llonflip -Olist -H -V]";
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
	char	file[MB_PATH_MAXLINE];
	int	pings;
	int	decimate;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	speedmin;
	double	timegap;

	/* segy data */
	void	*mbsegyioptr;
	struct mb_segyasciiheader_struct asciiheader;
	struct mb_segyfileheader_struct fileheader;
	struct mb_segytraceheader_struct traceheader;
	float	*trace;

	/* output format list controls */
	char	list[MAX_OPTIONS];
	int	n_list;
	int	nread;
	int	invert_next_value = MB_NO;
	int	signflip_next_value = MB_NO;
	int	first = MB_YES;
	int	ascii = MB_YES;
	int	segment = MB_NO;
	char	segment_tag[MB_PATH_MAXLINE];
	char	delimiter[MB_PATH_MAXLINE];

	/* additional time variables */
	int	first_m = MB_YES;
	double	time_d_ref;
	int	first_u = MB_YES;
	time_t	time_u;
	time_t	time_u_ref;
	double	time_interval;
	double	minutes;
	int	degrees;
	char	hemi;

	int	time_i[7], time_j[5];
	double	time_d, time_d_old;
	double	navlon, navlat;
	double	factor, sonardepth, waterdepth;
	double	delay, interval;

	double	b;
	int	format;
	int	i, j;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set file to null */
	file[0] = '\0';

	/* set up the default list controls: TiXYSsCcDINL
		(time, time interval, lon, lat, shot, shot trace #, cmp, cmp trace #,
			delay, sample length, number samples, trace length) */
	n_list = 0;
	list[n_list]='T'; n_list++;
	list[n_list]='i'; n_list++;
	list[n_list]='X'; n_list++;
	list[n_list]='Y'; n_list++;
	list[n_list]='S'; n_list++;
	list[n_list]='s'; n_list++;
	list[n_list]='C'; n_list++;
	list[n_list]='c'; n_list++;
	list[n_list]='D'; n_list++;
	list[n_list]='I'; n_list++;
	list[n_list]='N'; n_list++;
	list[n_list]='L'; n_list++;
	sprintf(delimiter, "\t");
	decimate = 1;

	/* get NaN value */
	MB_MAKE_DNAN(NaN);

	/* process argument list */
	while ((c = getopt(argc, argv, "AaD:d:G:g:I:i:L:l:O:o:VvWwZ:z:Hh")) != -1)
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
		case 'D':
		case 'd':
			sscanf (optarg,"%d", &decimate);
			flag++;
			break;
		case 'G':
		case 'g':
			sscanf (optarg,"%s", delimiter);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", file);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'O':
		case 'o':
			for(j=0,n_list=0;j<(int)strlen(optarg);j++,n_list++)
				if (n_list<MAX_OPTIONS)
					list[n_list] = optarg[j];
			flag++;
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
		fprintf(stderr,"dbg2       lonflip:        %d\n",lonflip);
		fprintf(stderr,"dbg2       decimate:       %d\n",decimate);
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
		fprintf(stderr,"dbg2       delimiter:      %s\n",delimiter);
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

	/* initialize reading the segy file */
	if (mb_segy_read_init(verbose, file,
		&mbsegyioptr, &asciiheader, &fileheader, &error) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_segy_read_init>:\n%s\n",message);
		fprintf(stderr,"\nSEGY File <%s> not initialized for reading\n",file);
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
	nread = 0;
	first = MB_YES;
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* reset error */
		error = MB_ERROR_NO_ERROR;

		/* read a trace */
		status = mb_segy_read_trace(verbose, mbsegyioptr,
				&traceheader, &trace, &error);
/*fprintf(stderr,"file:%s record:%d shot:%d  %4.4d/%3.3d %2.2d:%2.2d:%2.2d.%3.3d samples:%d interval:%d\n",
	file,nread,traceheader.shot_num,
	traceheader.year,traceheader.day_of_yr,
	traceheader.hour,traceheader.min,traceheader.sec,traceheader.mils,
	traceheader.nsamps,traceheader.si_micros);*/

		/* get needed values */
		if (status == MB_SUCCESS)
			{
			nread++;
			time_j[0] = traceheader.year;
			time_j[1] = traceheader.day_of_yr;
			time_j[2] = traceheader.min + 60 * traceheader.hour;
			time_j[3] = traceheader.sec;
			time_j[4] = 1000 * traceheader.mils;
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (first == MB_YES)
				{
				time_d_old = time_d;
				}
			if (traceheader.elev_scalar < 0)
				factor = 1.0 / ((float) (-traceheader.elev_scalar));
			else
				factor = (float) traceheader.elev_scalar;
			if (traceheader.grp_elev != 0)
				sonardepth = -factor * traceheader.grp_elev;
			else if (traceheader.src_elev != 0)
				sonardepth = -factor * traceheader.src_elev;
			else if (traceheader.src_depth != 0)
				sonardepth = factor * traceheader.src_depth;
			else
				sonardepth = 0.0;
			if (traceheader.src_wbd != 0)
				waterdepth = -traceheader.grp_elev;
			else if (traceheader.grp_wbd != 0)
				waterdepth = -traceheader.src_elev;
			else
				waterdepth = 0;
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
			}

		/* print out info */
		if (status == MB_SUCCESS && (nread - 1) % decimate == 0)
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
					case 'C': /* CDP number or CMP number or RP number */
						if (ascii == MB_YES)
						    printf("%6d",traceheader.rp_num);
						else
						    {
						    b = traceheader.rp_num;
						    fwrite(&b, sizeof(double), 1, stdout);
						    }
						break;
					case 'c': /* CDP trace or CMP trace or RP trace */
						if (ascii == MB_YES)
						    printf("%6d",traceheader.rp_tr);
						else
						    {
						    b = traceheader.rp_tr;
						    fwrite(&b, sizeof(double), 1, stdout);
						    }
						break;
					case 'D': /* trace start delay */
						delay = 0.001 * traceheader.delay_mils;
						printsimplevalue(verbose, delay, 0, 3, ascii,
								    &invert_next_value,
								    &signflip_next_value, &error);
						break;
					case 'I': /* sample interval in seconds */
						interval = 0.000001 * traceheader.si_micros;
						printsimplevalue(verbose, interval, 0, 6, ascii,
								    &invert_next_value,
								    &signflip_next_value, &error);
						break;
					case 'i': /* time interval since last trace */
						interval = time_d - time_d_old;
						printsimplevalue(verbose, interval, 0, 3, ascii,
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
					case 'L': /* Trace length in seconds */
						interval = 0.000001 * traceheader.si_micros
								* traceheader.nsamps;
						printsimplevalue(verbose, interval, 0, 6, ascii,
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
					case 'N': /* number of samples in trace */
						if (ascii == MB_YES)
						    printf("%6d",traceheader.nsamps);
						else
						    {
						    b = traceheader.nsamps;
						    fwrite(&b, sizeof(double), 1, stdout);
						    }
						break;
					case 'n': /* trace counter */
						if (ascii == MB_YES)
						    printf("%6d",nread);
						else
						    {
						    b = nread;
						    fwrite(&b, sizeof(double), 1, stdout);
						    }
						break;
					case 'R': /* range */
						if (ascii == MB_YES)
						    printf("%6d",traceheader.range);
						else
						    {
						    b = traceheader.range;
						    fwrite(&b, sizeof(double), 1, stdout);
						    }
						break;
					case 'S': /* shot number */
						if (ascii == MB_YES)
						    printf("%6d",traceheader.shot_num);
						else
						    {
						    b = traceheader.shot_num;
						    fwrite(&b, sizeof(double), 1, stdout);
						    }
						break;
					case 's': /* shot trace */
						if (ascii == MB_YES)
						    printf("%6d",traceheader.shot_tr);
						else
						    {
						    b = traceheader.shot_tr;
						    fwrite(&b, sizeof(double), 1, stdout);
						    }
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
						    printf("%ld",time_u);
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
						    printf("%ld",time_u - time_u_ref);
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
						printsimplevalue(verbose, navlon, 11, 6, ascii,
								    &invert_next_value,
								    &signflip_next_value, &error);
						break;
					case 'x': /* longitude degress + decimal minutes */
						if (navlon < 0.0)
							{
							hemi = 'W';
							navlon = -navlon;
							}
						else
							hemi = 'E';
						degrees = (int) navlon;
						minutes = 60.0*(navlon - degrees);
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
						printsimplevalue(verbose, navlat, 11, 6, ascii,
								    &invert_next_value,
								    &signflip_next_value, &error);
						break;
					case 'y': /* latitude degrees + decimal minutes */
						if (navlat < 0.0)
							{
							hemi = 'S';
							navlat = -navlat;
							}
						else
							hemi = 'N';
						degrees = (int) navlat;
						minutes = 60.0*(navlat - degrees);
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
					case 'Z': /* sonar depth (m) */
						if (traceheader.elev_scalar < 0)
							factor = 1.0 / ((float) (-traceheader.elev_scalar));
						else
							factor = (float) traceheader.elev_scalar;
						if (traceheader.grp_elev != 0)
							sonardepth = -factor * traceheader.grp_elev;
						else if (traceheader.src_elev != 0)
							sonardepth = -factor * traceheader.src_elev;
						else if (traceheader.src_depth != 0)
							sonardepth = factor * traceheader.src_depth;
						else
							sonardepth = 0.0;
						printsimplevalue(verbose, sonardepth, 11, 6, ascii,
								    &invert_next_value,
								    &signflip_next_value, &error);
						break;
					case 'z': /* water depth (m) */
						if (traceheader.elev_scalar < 0)
							factor = 1.0 / ((float) (-traceheader.elev_scalar));
						else
							factor = (float) traceheader.elev_scalar;
						if (traceheader.src_wbd != 0)
							waterdepth = -factor * traceheader.src_wbd;
						else if (traceheader.grp_wbd != 0)
							waterdepth = -factor * traceheader.grp_wbd;
						else
							waterdepth = 0.0;
						printsimplevalue(verbose, waterdepth, 11, 6, ascii,
								    &invert_next_value,
								    &signflip_next_value, &error);
						break;
					default:
						if (ascii == MB_YES)
						    printf("<Invalid Option: %c>",
							list[i]);
						break;
					}
				if (ascii == MB_YES)
				    {
				    if (i<(n_list-1)) printf ("%s",delimiter);
				    else printf ("\n");
				    }
				}

			}

		/* reset first flag */
		if (error == MB_ERROR_NO_ERROR && first == MB_YES)
			{
			first = MB_NO;
			}

		/* save old values */
		if (error == MB_ERROR_NO_ERROR)
			{
			time_d_old = time_d;
			}
		}

	/* close the swath file */
	status = mb_segy_close(verbose,&mbsegyioptr,&error);

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
int printsimplevalue(int verbose,
	double value, int width, int precision,
	int ascii, int *invert, int *flipsign, int *error)
{
	char	*function_name = "printsimplevalue";
	int	status = MB_SUCCESS;
	char	format[24];

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
