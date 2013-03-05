/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*--------------------------------------------------------------------
 *    The MB-system:	mbsegyinfo.c	6/2/2004
 *    $Id: mbsegyinfo.c 1945 2012-05-02 19:11:42Z caress $
 *
 *    Copyright (c) 2004-2012 by
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
 * MBsegyinfo reads a segy data file and outputs some basic statistics.
 *
 * Author:	D. W. Caress
 * Date:	June 2, 2004
 *
 * $Log: mbsegyinfo.c,v $
 * Revision 5.5  2008/09/13 06:08:09  caress
 * Updates to apply suggested patches to segy handling. Also fixes to remove compiler warnings.
 *
 * Revision 5.4  2008/05/24 19:40:42  caress
 * Applied a Gordon Keith fix.
 *
 * Revision 5.3  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.2  2004/10/06 19:10:53  caress
 * Release 5.0.5 update.
 *
 * Revision 5.1  2004/07/27 19:48:35  caress
 * Working on handling subbottom data.
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

/* MBIO include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_segy.h"

/* GMT include files */
#include "gmt_nan.h"

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

static char rcs_id[] = "$Id: mbsegyinfo.c 1945 2012-05-02 19:11:42Z caress $";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "MBsegyinfo";
	char help_message[] =  "MBsegyinfo lists table data from a segy data file.";
	char usage_message[] = "MBsegyinfo -Ifile [-Llonflip -O -H -V]";
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
	char	read_file[MB_PATH_MAXLINE];
	int	pings;
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
	int	nread = 0;
	int	first = MB_YES;

	/* limit variables */
	int	shotmin = 0;
	int	shotmax = 0;
	int	shottracemin = 0;
	int	shottracemax = 0;
	int	rpmin = 0;
	int	rpmax = 0;
	int	rptracemin = 0;
	int	rptracemax = 0;
	double	rangemin = 0.0;
	double	rangemax = 0.0;
	double	receiverelevationmin = 0.0;
	double	receiverelevationmax = 0.0;
	double	sourceelevationmin = 0.0;
	double	sourceelevationmax = 0.0;
	double	sourcedepthmin = 0.0;
	double	sourcedepthmax = 0.0;
	double	sourcewaterdepthmin = 0.0;
	double	sourcewaterdepthmax = 0.0;
	double	receiverwaterdepthmin = 0.0;
	double	receiverwaterdepthmax = 0.0;
	double	delaymin = 0.0;
	double	delaymax = 0.0;
	double	lonmin = 0.0;
	double	lonmax = 0.0;
	double	latmin = 0.0;
	double	latmax = 0.0;
	double	lonbeg = 0.0;
	double	latbeg = 0.0;
	double	lonend = 0.0;
	double	latend = 0.0;
	double	timbeg = 0.0;
	double	timend = 0.0;
	int	timbeg_i[7];
	int	timend_i[7];
	int	timbeg_j[5];
	int	timend_j[5];

	int	time_i[7], time_j[5];
	double	time_d;
	double	navlon, navlat;
	double	factor, sonardepth, waterdepth;
	double	tracelength, delay;
	double	range;
	double	receiverelevation;
	double	sourceelevation;
	double	sourcedepth;
	double	sourcewaterdepth;
	double	receiverwaterdepth;

	/* output stream for basic stuff (stdout if verbose <= 1,
		output if verbose > 1) */
	FILE	*stream = NULL;
	FILE	*output = NULL;
	int	output_usefile = MB_NO;
	char	output_file[MB_PATH_MAXLINE];

	int	format;
	int	i;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set read_file to null */
	read_file[0] = '\0';

	/* process argument list */
	while ((c = getopt(argc, argv, "I:i:L:l:OoVvWwHh")) != -1)
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
		case 'O':
		case 'o':
			output_usefile = MB_YES;
			flag++;
			break;
		case '?':
			errflg++;
		}

	/* set output stream */
	if (verbose <= 1)
		stream = stdout;
	else
		stream = stderr;

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
		fprintf(stderr,"dbg2       read_file:      %s\n",read_file);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* initialize reading the segy file */
	if (mb_segy_read_init(verbose, read_file,
		&mbsegyioptr, &asciiheader, &fileheader, &error) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_segy_read_init>:\n%s\n",message);
		fprintf(stderr,"\nSEGY File <%s> not initialized for reading\n",read_file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* Open output file if requested */
	if (output_usefile == MB_YES)
	    {
	    strcpy(output_file, read_file);
	    strcat(output_file, ".sinf");
	    if ((output = fopen(output_file, "w")) == NULL)
		output = stream;
	    }
	else
	    {
	    output = stream;
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
/*fprintf(stderr,"read_file:%s record:%d shot:%d  %4.4d/%3.3d %2.2d:%2.2d:%2.2d.%3.3d samples:%d interval:%d\n",
	read_file,nread,traceheader.shot_num,
	traceheader.year,traceheader.day_of_yr,
	traceheader.hour,traceheader.min,traceheader.sec,traceheader.mils,
	traceheader.nsamps,traceheader.si_micros);*/

		/* deal with success */
		if (status == MB_SUCCESS)
			{
			nread++;

			/* get needed values */
			time_j[0] = traceheader.year;
			time_j[1] = traceheader.day_of_yr;
			time_j[2] = traceheader.min + 60 * traceheader.hour;
			time_j[3] = traceheader.sec;
			time_j[4] = 1000 * traceheader.mils;
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
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
			if (traceheader.elev_scalar < 0)
				factor = 1.0 / ((float) (-traceheader.elev_scalar));
			else
				factor = (float) traceheader.elev_scalar;
			range = (double) traceheader.range;
			receiverelevation = factor * ((double)traceheader.grp_elev);
			sourceelevation = factor * ((double)traceheader.src_elev);
			sourcedepth = factor * ((double)traceheader.src_depth);
			sourcewaterdepth = factor * ((double)traceheader.src_wbd);
			receiverwaterdepth = factor * ((double)traceheader.grp_wbd);
			delay = 0.001 * ((double)traceheader.delay_mils);

			/* get initial values */
			if (first == MB_YES)
				{
				shotmin = traceheader.shot_num;
				shotmax = traceheader.shot_num;
				shottracemin = traceheader.shot_tr;
				shottracemax = traceheader.shot_tr;
				rpmin = traceheader.rp_num;
				rpmax = traceheader.rp_num;
				rptracemin = traceheader.rp_tr;
				rptracemax = traceheader.rp_tr;
				delaymin = delay;
				delaymax = delay;
				lonmin = navlon;
				lonmax = navlon;
				latmin = navlat;
				latmax = navlat;
				rangemin = range;
				rangemax = range;
				receiverelevationmin = receiverelevation;
				receiverelevationmax = receiverelevation;
				sourceelevationmin = sourceelevation;
				sourceelevationmax = sourceelevation;
				sourcedepthmin = sourcedepth;
				sourcedepthmax = sourcedepth;
				sourcewaterdepthmin = sourcewaterdepth;
				sourcewaterdepthmax = sourcewaterdepth;
				receiverwaterdepthmin = receiverwaterdepth;
				receiverwaterdepthmax = receiverwaterdepth;

				lonbeg = navlon;
				latbeg = navlat;
				lonend = navlon;
				latend = navlat;
				timbeg = time_d;
				timend = time_d;
				for (i=0;i<7;i++)
					{
					timbeg_i[i] = time_i[i];
					timend_i[i] = time_i[i];
					}
				for (i=0;i<5;i++)
					{
					timbeg_j[i] = time_j[i];
					timend_j[i] = time_j[i];
					}
				first = MB_NO;
				}

			/* get min max values */
			else
				{
				shotmin = MIN(shotmin, traceheader.shot_num);
				shotmax = MAX(shotmax, traceheader.shot_num);
				shotmin = MIN(shotmin, traceheader.shot_num);
				shotmax = MAX(shotmax, traceheader.shot_num);
				rpmin = MIN(rpmin, traceheader.rp_num);
				rpmax = MAX(rpmax, traceheader.rp_num);
				rptracemin = MIN(rptracemin, traceheader.rp_tr);
				rptracemax = MAX(rptracemax, traceheader.rp_tr);
				delaymin = MIN(delaymin, delay);
				delaymax = MAX(delaymax, delay);
				if (navlon != 0.0 && navlat != 0.0)
					{
					lonmin = MIN(lonmin, navlon);
					lonmax = MAX(lonmax, navlon);
					latmin = MIN(latmin, navlat);
					latmax = MAX(latmax, navlat);
					}
				lonend = navlon;
				latend = navlat;
				timend = time_d;
				for (i=0;i<7;i++)
					{
					timend_i[i] = time_i[i];
					}
				for (i=0;i<5;i++)
					{
					timend_j[i] = time_j[i];
					}
				rangemin = MIN(rangemin, range);
				rangemax = MAX(rangemax, range);
				receiverelevationmin = MIN(receiverelevationmin, receiverelevation);
				receiverelevationmax = MAX(receiverelevationmax, receiverelevation);
				sourceelevationmin = MIN(sourceelevationmin, sourceelevation);
				sourceelevationmax = MAX(sourceelevationmax, sourceelevation);
				sourcedepthmin = MIN(sourcedepthmin, sourcedepth);
				sourcedepthmax = MAX(sourcedepthmax, sourcedepth);
				sourcewaterdepthmin = MIN(sourcewaterdepthmin, sourcewaterdepth);
				sourcewaterdepthmax = MAX(sourcewaterdepthmax, sourcewaterdepth);
				receiverwaterdepthmin = MIN(receiverwaterdepthmin, receiverwaterdepth);
				receiverwaterdepthmax = MAX(receiverwaterdepthmax, receiverwaterdepth);
				}

			}

		/* reset first flag */
		if (error == MB_ERROR_NO_ERROR && first == MB_YES)
			{
			first = MB_NO;
			}

		}

	/* close the swath file */
	status = mb_segy_close(verbose,&mbsegyioptr,&error);

	/* output the information */
	tracelength = 0.000001 * (double)(fileheader.sample_interval * fileheader.number_samples);
	fprintf(output,"\nSEGY Data File:      %s\n",read_file);
	fprintf(output,"\nFile Header Info:\n");
	fprintf(output,"  Channels:                   %8d\n",fileheader.channels);
	fprintf(output,"  Auxilliary Channels:        %8d\n",fileheader.aux_channels);
	fprintf(output,"  Sample Interval (usec):     %8d\n",fileheader.sample_interval);
	fprintf(output,"  Number of Samples in Trace: %8d\n",fileheader.number_samples);
	fprintf(output,"  Trace length (sec):         %8f\n",tracelength);
	if (fileheader.format == 1)
		fprintf(output,"  Data Format:                IBM 32 bit floating point\n");
	else if (fileheader.format == 2)
		fprintf(output,"  Data Format:                32 bit integer\n");
	else if (fileheader.format == 3)
		fprintf(output,"  Data Format:                16 bit integer\n");
	else if (fileheader.format == 5)
		fprintf(output,"  Data Format:                IEEE 32 bit integer\n");
	else if (fileheader.format == 6)
		fprintf(output,"  Data Format:                IEEE 32 bit integer\n");
	else if (fileheader.format == 8)
		fprintf(output,"  Data Format:                8 bit integer\n");
	else if (fileheader.format == 11)
		fprintf(output,"  Data Format:                Little-endian IEEE 32 bit floating point\n");
	else
		fprintf(output,"  Data Format:                Unknown\n");
	fprintf(output,"  CDP Fold:                   %8d\n",fileheader.cdp_fold);
	fprintf(output,"\nData Totals:\n");
	fprintf(output,"  Number of Traces:           %8d\n",nread);
	fprintf(output,"  Min Max Delta:\n");
	fprintf(output,"    Shot number:              %8d %8d %8d\n",
		shotmin, shotmax, shotmax - shotmin + 1);
	fprintf(output,"    Shot trace:               %8d %8d %8d\n",
		shottracemin, shottracemax, shottracemax - shottracemin + 1);
	fprintf(output,"    RP number:                %8d %8d %8d\n",
		rpmin, rpmax, rpmax - rpmin + 1);
	fprintf(output,"    RP trace:                 %8d %8d %8d\n",
		rptracemin, rptracemax, rptracemax - rptracemin + 1);
	fprintf(output,"    Delay (sec):              %8f %8f %8f\n",
		delaymin, delaymax, delaymax - delaymin);
	fprintf(output,"    Range (m):                %8f %8f %8f\n",
		rangemin, rangemax, rangemax - rangemin);
	fprintf(output,"    Receiver Elevation (m):   %8f %8f %8f\n",
		receiverelevationmin, receiverelevationmax, receiverelevationmax - receiverelevationmin);
	fprintf(output,"    Source Elevation (m):     %8f %8f %8f\n",
		sourceelevationmin, sourceelevationmax, sourceelevationmax - sourceelevationmin);
	fprintf(output,"    Source Depth (m):         %8f %8f %8f\n",
		sourcedepthmin, sourcedepthmax, sourcedepthmax - sourcedepthmin);
	fprintf(output,"    Receiver Water Depth (m): %8f %8f %8f\n",
		receiverwaterdepthmin, receiverwaterdepthmax, receiverwaterdepthmax - receiverwaterdepthmin);
	fprintf(output,"    Source Water Depth (m):   %8f %8f %8f\n",
		sourcewaterdepthmin, sourcewaterdepthmax, sourcewaterdepthmax - sourcewaterdepthmin);
	fprintf(output,"\nNavigation Totals:\n");
	fprintf(output,"\n  Start of Data:\n");
	fprintf(output,"    Start Time:  %2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d\n",
		timbeg_i[1],timbeg_i[2],timbeg_i[0],timbeg_i[3],
		timbeg_i[4],timbeg_i[5],timbeg_i[6],timbeg_j[1]);
	fprintf(output,"    Start Position: Lon: %14.9f     Lat: %14.9f\n", lonbeg,latbeg);
	fprintf(output,"\n  End of Data:\n");
	fprintf(output,"    End Time:    %2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d\n",
		timend_i[1],timend_i[2],timend_i[0],timend_i[3],
		timend_i[4],timend_i[5],timend_i[6],timend_j[1]);
	fprintf(output,"    End Position:   Lon: %14.9f     Lat: %14.9f \n", lonend,latend);
	fprintf(output,"\nLimits:\n");
	fprintf(output,"  Minimum Longitude:   %14.9f   Maximum Longitude:   %14.9f\n",lonmin,lonmax);
	fprintf(output,"  Minimum Latitude:    %14.9f   Maximum Latitude:    %14.9f\n",latmin,latmax);

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
