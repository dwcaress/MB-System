/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*--------------------------------------------------------------------
 *    The MB-system:	mbextractsegy.c	4/18/2004
 *    $Id: mbextractsegy.c 1903 2011-07-31 22:19:30Z caress $
 *
 *    Copyright (c) 2004-2011 by
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
 * mbextractsegy extracts subbottom profiler, center beam reflection,
 * or seismic reflection data from data supported by MB-System and
 * rewrites it as a SEGY file in the form used by SIOSEIS. .
 *
 * Author:	D. W. Caress
 * Date:	April 18, 2004
 *
 * $Log: mbextractsegy.c,v $
 * Revision 5.20  2009/03/13 07:05:58  caress
 * Release 5.1.2beta02
 *
 * Revision 5.19  2009/03/02 18:54:40  caress
 * Fixed pixel size problems with mbmosaic, resurrected program mbfilter, and also updated copyright dates in several source files.
 *
 * Revision 5.18  2008/05/16 22:44:37  caress
 * Release 5.1.1beta18
 *
 * Revision 5.17  2007/11/16 17:53:03  caress
 * Fixes applied.
 *
 * Revision 5.16  2007/10/08 16:48:07  caress
 * State of the code on 8 October 2007.
 *
 * Revision 5.15  2007/03/02 18:22:54  caress
 * When extracting lines using a route file, now omits data between where a waypoint is crossed and the sonar comes onto the next line, thus eliminating data during turns.
 *
 * Revision 5.14  2006/12/15 21:42:49  caress
 * Incremental CVS update.
 *
 * Revision 5.13  2006/11/26 09:42:01  caress
 * Making distribution 5.1.0.
 *
 * Revision 5.12  2006/11/10 22:36:05  caress
 * Working towards release 5.1.0
 *
 * Revision 5.11  2006/08/09 22:41:27  caress
 * Fixed programs that read or write grids so that they do not use the GMT_begin() function; these programs will now work when GMT is built in the default fashion, when GMT is built in the default fashion, with "advisory file locking" enabled.
 *
 * Revision 5.10  2006/06/22 04:45:43  caress
 * Working towards 5.1.0
 *
 * Revision 5.9  2006/06/16 19:30:58  caress
 * Check in after the Santa Monica Basin Mapping AUV Expedition.
 *
 * Revision 5.8  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.7  2005/11/05 01:07:54  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.6  2005/06/04 06:07:02  caress
 * Fixed output of a single segy file deriving from a list
 * of input swath files.
 *
 * Revision 5.5  2004/10/06 19:10:52  caress
 * Release 5.0.5 update.
 *
 * Revision 5.4  2004/09/16 01:01:12  caress
 * Fixed many things.
 *
 * Revision 5.3  2004/07/27 19:48:35  caress
 * Working on handling subbottom data.
 *
 * Revision 5.2  2004/07/15 19:33:57  caress
 * Improvements to support for Reson 7k data.
 *
 * Revision 5.1  2004/06/18 05:20:05  caress
 * Working on adding support for segy i/o and for Reson 7k format 88.
 *
 * Revision 5.0  2004/05/21 23:50:44  caress
 * Progress supporting Reson 7k data, including support for extracing subbottom profiler data.
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

/* MBIO include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_segy.h"

/* defines */
#define MBES_ALLOC_NUM			128
#define MBES_ROUTE_WAYPOINT_NONE		0
#define MBES_ROUTE_WAYPOINT_SIMPLE	1
#define MBES_ROUTE_WAYPOINT_TRANSIT	2
#define MBES_ROUTE_WAYPOINT_STARTLINE	3
#define MBES_ROUTE_WAYPOINT_ENDLINE	4
#define MBES_ONLINE_THRESHOLD		15.0
#define MBES_ONLINE_COUNT		30

static char rcs_id[] = "$Id: mbextractsegy.c 1903 2011-07-31 22:19:30Z caress $";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "MBextractsegy";
	char help_message[] =  "MBextractsegy extracts subbottom profiler, center beam reflection,\nor seismic reflection data from data supported by MB-System and\nrewrites it as a SEGY file in the form used by SIOSEIS.";
	char usage_message[] = "mbextractsegy [-Byr/mo/dy/hr/mn/sc/us -Eyr/mo/dy/hr/mn/sc/us -Fformat \n\t-Ifile -Jxscale/yscale -Lstartline/lineroot \n\t-Osegyfile -Qtimelistfile -Rroutefile \n\t-Ssampleformat -Zplotmax -H -V]";
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
	int	read_datalist = MB_NO;
	mb_path	read_file;
	mb_path	output_file;
	mb_path	output_list_file;
	int	output_file_set = MB_NO;
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
	mb_path	file;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	double	timeshift = 0.0;

	/* MBIO read values */
	void	*mbio_ptr = NULL;
	void	*store_ptr = NULL;
	int	kind;
	int	time_i[7];
	int	time_j[5];
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
	int	icomment = 0;
	
	/* segy data */
	int	sampleformat = MB_SEGY_SAMPLEFORMAT_ENVELOPE;
	int	samplesize = 0;
	struct mb_segyasciiheader_struct segyasciiheader;
	struct mb_segyfileheader_struct segyfileheader;
	struct mb_segytraceheader_struct segytraceheader;
	int	segydata_alloc = 0;
	float	*segydata = NULL;
	int	buffer_alloc = 0;
	char	*buffer = NULL;
	
	/* route and auto-line data */
	mb_path	timelist_file;
	int	timelist_file_set = MB_NO;
	int	ntimepoint = 0;
	int	ntimepointalloc = 0;
	double	*routetime_d = NULL;	
	mb_path	route_file;
	int	route_file_set = MB_NO;
	int	checkroutebearing = MB_NO;
	int	rawroutefile = MB_NO;
	mb_path	lineroot;
	int	nroutepoint = 0;
	int	nroutepointalloc = 0;
	double	lon;
	double	lat;
	double	topo;
	int	waypoint;
	double	*routelon = NULL;
	double	*routelat = NULL;
	double	*routeheading = NULL;
	int	*routewaypoint = NULL;
	double	range;
	double	rangethreshold = 25.0;
	double	rangelast;
	int	activewaypoint = 0;
	int	startline = 1;
	int	linenumber;
	
	/* auto plotting */
	FILE	*sfp = NULL;
	mb_path	scriptfile;
	double	seafloordepthmin = -1.0;
	double	seafloordepthmax = -1.0;
	double	sweep;
	double	delay;
	double	startlon;
	double	startlat;
	int	startshot;
	double	endlon;
	double	endlat;
	int	endshot;
	double	linedistance;
	double	linebearing;
	int	nshot;
	int	nshotmax;
	int	nplot;
	double	xscale = 0.01;
	double	yscale = 50.0;
	double	maxwidth = 30.0;
	mb_path	zbounds;
	double	zmax = 50;

	mb_path	command;
	mb_path	scale;
	double	mtodeglon, mtodeglat;
	double	lastlon;
	double	lastlat;
	double	lastheading;
	double	headingdiff;
	int	rangeok;
	int	oktowrite;
	int	linechange;
	double	dx, dy;
	FILE	*fp = NULL;
	FILE	*cfp = NULL;
	char	*result;
	int	nget;
	int	point_ok;
	int	read_data;
	int	nread;
	int	nwrite;
	int	first;
	int	index;
	double	tracemin, tracemax, tracerms;
	double	linetracemin, linetracemax;
	double	draft, roll, pitch, heave;
	int	i, j;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to datalist.mb-1 */
	strcpy (read_file, "datalist.mb-1");

	/* set default lineroot to sbp */
	strcpy (lineroot, "sbp");
	
	/* initialize output segy structures */
	for (j=0;j<40;j++)
		for (i=0;i<80;i++)
			segyasciiheader.line[j][i] = 0;
	segyfileheader.jobid = 0;
	segyfileheader.line = 0;
	segyfileheader.reel = 0;
	segyfileheader.channels = 0;
	segyfileheader.aux_channels = 0;
	segyfileheader.sample_interval = 0;
	segyfileheader.sample_interval_org = 0;
	segyfileheader.number_samples = 0;
	segyfileheader.number_samples_org = 0;
	segyfileheader.format = 5;
	segyfileheader.cdp_fold = 0;
	segyfileheader.trace_sort = 0;
	segyfileheader.vertical_sum = 0;
	segyfileheader.sweep_start = 0;
	segyfileheader.sweep_end = 0;
	segyfileheader.sweep_length = 0;
	segyfileheader.sweep_type = 0;
	segyfileheader.sweep_trace = 0;
	segyfileheader.sweep_taper_start = 0;
	segyfileheader.sweep_taper_end = 0;
	segyfileheader.sweep_taper = 0;
	segyfileheader.correlated = 0;
	segyfileheader.binary_gain = 0;
	segyfileheader.amplitude = 0;
	segyfileheader.units = 0;
	segyfileheader.impulse_polarity = 0;
	segyfileheader.domain = 0;
	for (i=0;i<338;i++)
		segyfileheader.extra[i] = 0;

	/* process argument list */
	while ((c = getopt(argc, argv, "B:b:D:d:E:e:F:f:I:i:J:j:L:l:MmO:o:Q:q:R:r:S:s:T:t:U:u:Z:z:VvHh")) != -1)
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
		case 'B':
		case 'b':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			btime_i[6] = 0;
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
		case 'J':
		case 'j':
			sscanf (optarg,"%lf/%lf/%lf", &xscale, &yscale, &maxwidth);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d/%s", &startline, lineroot);
			flag++;
			break;
		case 'M':
		case 'm':
			checkroutebearing = MB_YES;
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", output_file);
			output_file_set = MB_YES;
			flag++;
			break;
		case 'Q':
		case 'q':
			sscanf (optarg,"%s", timelist_file);
			timelist_file_set = MB_YES;
			flag++;
			break;
		case 'R':
		case 'r':
			sscanf (optarg,"%s", route_file);
			route_file_set = MB_YES;
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%d", &sampleformat);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf", &timeshift);
			flag++;
			break;
		case 'U':
		case 'u':
			sscanf (optarg,"%lf", &rangethreshold);
			flag++;
			break;
		case 'Z':
		case 'z':
			sscanf (optarg,"%lf", &zmax);
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
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       help:              %d\n",help);
		fprintf(stderr,"dbg2       format:            %d\n",format);
		fprintf(stderr,"dbg2       pings:             %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:           %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:         %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:         %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:         %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:         %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:        %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:        %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:        %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:        %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:        %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:        %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:        %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:        %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:        %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:        %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:        %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:        %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:        %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:        %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:          %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:           %f\n",timegap);
		fprintf(stderr,"dbg2       sampleformat:      %d\n",sampleformat);
		fprintf(stderr,"dbg2       timeshift:         %f\n",timeshift);
		fprintf(stderr,"dbg2       file:              %s\n",file);
		fprintf(stderr,"dbg2       timelist_file_set: %d\n",timelist_file_set);
		fprintf(stderr,"dbg2       timelist_file:     %s\n",timelist_file);
		fprintf(stderr,"dbg2       route_file_set:    %d\n",route_file_set);
		fprintf(stderr,"dbg2       route_file:        %s\n",route_file);
		fprintf(stderr,"dbg2       checkroutebearing: %d\n",checkroutebearing);
		fprintf(stderr,"dbg2       output_file_set:   %d\n",output_file_set);
		fprintf(stderr,"dbg2       output_file:       %s\n",output_file);
		fprintf(stderr,"dbg2       lineroot:          %s\n",lineroot);
		fprintf(stderr,"dbg2       xscale:            %f\n",xscale);
		fprintf(stderr,"dbg2       yscale:            %f\n",yscale);
		fprintf(stderr,"dbg2       maxwidth:          %f\n",maxwidth);
		fprintf(stderr,"dbg2       rangethreshold:    %f\n",rangethreshold);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}
		
	/* set starting line number */
	linenumber = startline;
	
	/* set maximum number of shots per plot */
	nshotmax = (int)(maxwidth / xscale);

	/* if specified read route time list file */
	if (timelist_file_set == MB_YES)
		{	    
		/* open the input file */
		if ((fp = fopen(timelist_file, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			fprintf(stderr,"\nUnable to open time list file <%s> for reading\n",timelist_file);
			exit(status);
			}
		rawroutefile = MB_NO;
		while ((result = fgets(comment,MB_PATH_MAXLINE,fp)) == comment)
		    	{
			if (comment[0] != '#')
				{
				nget = sscanf(comment,"%d %d %lf %lf %lf %lf",
				    &i, &waypoint, &lon, &lat, &heading, &time_d);
			    
				/* if good data check for need to allocate more space */
				if (ntimepoint + 1 > ntimepointalloc)
				    	{
				    	ntimepointalloc += MBES_ALLOC_NUM;
					status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double),
								(void **)&routelon, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double),
								(void **)&routelat, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double),
								(void **)&routeheading, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(int),
								(void **)&routewaypoint, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double),
								(void **)&routetime_d, &error);
				    	if (status != MB_SUCCESS)
					    	{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
							message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
							program_name);
						exit(error);
					    	}
				    	}

				/* add good point to route */
				if (ntimepointalloc > ntimepoint)
					{
					routewaypoint[ntimepoint] = waypoint;
					routelon[ntimepoint] = lon;
					routelat[ntimepoint] = lat;
					routeheading[ntimepoint] = heading;
					routetime_d[ntimepoint] = time_d;
					ntimepoint++;
					}
				}
			}

		/* close the file */
		fclose(fp);
		fp = NULL;
		
		/* set starting values */
		activewaypoint = 0;
		mb_coor_scale(verbose,routelat[activewaypoint], &mtodeglon, &mtodeglat);
		rangelast = 1000 * rangethreshold;
		seafloordepthmin = -1.0;
		seafloordepthmax = -1.0;
		oktowrite = 0;
		rangeok = MB_NO;

		/* output status */
		if (verbose > 0)
			{
			/* output info on file output */
			fprintf(stderr,"Read %d waypoints from time list file: %s\n",
				ntimepoint, timelist_file);
			}
		}

	/* if specified read route file */
	else if (route_file_set == MB_YES)
		{	    
		/* open the input file */
		if ((fp = fopen(route_file, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			fprintf(stderr,"\nUnable to open route file <%s> for reading\n",route_file);
			exit(status);
			}
		rawroutefile = MB_NO;
		while ((result = fgets(comment,MB_PATH_MAXLINE,fp)) == comment)
		    	{
			if (comment[0] == '#')
				{
				if (strncmp(comment,"## Route File Version", 21) == 0)
					{
					rawroutefile = MB_NO;
					}
				}
			else
				{
				nget = sscanf(comment,"%lf %lf %lf %d %lf",
				    &lon, &lat, &topo, &waypoint, &heading);
				if (comment[0] == '#')
					{
					fprintf(stderr,"buffer:%s",comment);
					if (strncmp(comment,"## Route File Version", 21) == 0)
						{
						rawroutefile = MB_NO;
						}
					}
		    		if ((rawroutefile == MB_YES && nget >= 2)
					|| (rawroutefile == MB_NO && nget >= 3 && waypoint > MBES_ROUTE_WAYPOINT_NONE))
					point_ok = MB_YES;
				else
					point_ok = MB_NO;
			    
				/* if good data check for need to allocate more space */
				if (point_ok == MB_YES
					&& nroutepoint + 1 > nroutepointalloc)
				    	{
				    	nroutepointalloc += MBES_ALLOC_NUM;
					status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double),
								(void **)&routelon, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double),
								(void **)&routelat, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double),
								(void **)&routeheading, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(int),
								(void **)&routewaypoint, &error);
				    	if (status != MB_SUCCESS)
					    	{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
							message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
							program_name);
						exit(error);
					    	}
				    	}

				/* add good point to route */
				if (point_ok == MB_YES && nroutepointalloc > nroutepoint + 1)
					{
					routelon[nroutepoint] = lon;
					routelat[nroutepoint] = lat;
					routeheading[nroutepoint] = heading;
					routewaypoint[nroutepoint] = waypoint;
					nroutepoint++;
					}
				}
			}

		/* close the file */
		fclose(fp);
		fp = NULL;
		
		/* set starting values */
		activewaypoint = 1;
		mb_coor_scale(verbose,routelat[activewaypoint], &mtodeglon, &mtodeglat);
		rangelast = 1000 * rangethreshold;
		seafloordepthmin = -1.0;
		seafloordepthmax = -1.0;
		oktowrite = 0;
		rangeok = MB_NO;

		/* output status */
		if (verbose > 0)
			{
			/* output info on file output */
			fprintf(stderr,"Read %d waypoints from route file: %s\n",
				nroutepoint, route_file);
			}
		}
		
	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,read_file,NULL,&format,&error);
		
	/* get sample size from sampleformat */
	if (sampleformat == MB_SEGY_SAMPLEFORMAT_ANALYTIC)
		samplesize = 2 * sizeof(float);
	else
		samplesize = sizeof(float);
		
	/* get plot zbounds from sampleformat */
	if (sampleformat == MB_SEGY_SAMPLEFORMAT_ENVELOPE)
		sprintf(zbounds, "0/%f/1", zmax);
	else
		sprintf(zbounds, "-%f/%f", zmax, zmax);

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
	    if ((status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error))
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
	    
	/* set up plotting script file */
	if ((route_file_set == MB_YES && nroutepoint > 1) ||
		(timelist_file_set == MB_YES && ntimepoint > 1))
		{
		sprintf(scriptfile, "%s_section.cmd", lineroot);
		}
	else if (output_file_set == MB_NO || read_datalist == MB_YES)
		{
		sprintf(scriptfile, "%s_section.cmd", read_file);
		}
	else
		{
		sprintf(scriptfile, "%s_section.cmd", file);
		}
	if ((sfp = fopen(scriptfile, "w")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		fprintf(stderr,"\nUnable to open plotting script file <%s> \n",scriptfile);
		exit(status);
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

	/* allocate memory for data arrays */
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

	/* read and print data */
	nread = 0;
	first = MB_YES;
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* reset error */
		error = MB_ERROR_NO_ERROR;
		
		/* read next data record */
		status = mb_get_all(verbose,mbio_ptr,&store_ptr,&kind,
				    time_i,&time_d,&navlon,&navlat,
				    &speed,&heading,
				    &distance,&altitude,&sonardepth,
				    &beams_bath,&beams_amp,&pixels_ss,
				    beamflag,bath,amp,bathacrosstrack,bathalongtrack,
				    ss,ssacrosstrack,ssalongtrack,
				    comment,&error);
		    
		/* deal with nav and time from survey data only - not nav, sidescan, or subbottom */
		if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA)
			{
			/* reset output flag */
			linechange = MB_NO;
			
			/* get nav data */
			status = mb_extract_nav(verbose,mbio_ptr,store_ptr,&kind,
					time_i,&time_d,&navlon,&navlat,
					&speed,&heading,&draft,&roll,&pitch,&heave,&error);
		   
			/* save last nav and heading */
			if (navlon != 0.0)
				lastlon = navlon;
			if (navlat != 0.0)
				lastlat = navlat;
			if (heading != 0.0)
				lastheading = heading;
				
			/* to set lines check survey data time against time list */
			if (ntimepoint > 1)
				{
				dx = (navlon - routelon[activewaypoint]) / mtodeglon;
				dy = (navlat - routelat[activewaypoint]) / mtodeglat;
				range = sqrt(dx * dx + dy * dy);
				if (time_d >= routetime_d[activewaypoint]
					&& activewaypoint < ntimepoint)
					{
					linechange = MB_YES;
/* fprintf(stderr,"LINECHANGE 1!! dx:%f dy:%f range:%f activewaypoint:%d time_d: %f %f\n",
dx,dy,range,activewaypoint,time_d,routetime_d[activewaypoint]); */
					}
				}

			/* else to set lines check survey data position against waypoints */
			else if (nroutepoint > 1 && navlon != 0.0 && navlat != 0.0)
				{
				dx = (navlon - routelon[activewaypoint]) / mtodeglon;
				dy = (navlat - routelat[activewaypoint]) / mtodeglat;
				range = sqrt(dx * dx + dy * dy);
				if (range < rangethreshold)
					rangeok = MB_YES;
				if (rangeok == MB_YES 
					&& (activewaypoint == 0 || range > rangelast) 
					&& activewaypoint < nroutepoint - 1)
					{
					linechange = MB_YES;
/* fprintf(stderr,"LINECHANGE 2!! dx:%f dy:%f range:%f %f activewaypoint:%d\n",dx,dy,range,rangethreshold,activewaypoint); */
					}
				}

			/* apply line change */
			if (linechange == MB_YES)
				{
				/* close current output file */
				if (fp != NULL)
				    {
				    fclose(fp);
				    fp = NULL;
				    fclose(cfp);
				    cfp = NULL;

				    /* output count of segy records */
				    fprintf(stderr,"%d records output to segy file %s\n",
							nwrite, output_file);
				    if (verbose > 0)
					fprintf(stderr,"\n");

				    /* use mbsegyinfo to generate a sinf file */
				    sprintf(command, "mbsegyinfo -I %s -O", output_file);
		   		    fprintf(stderr, "Executing: %s\n", command);
				    system(command);

				    /* get bearing and plot scale */
				    dx = (endlon - startlon) / mtodeglon;
				    dy = (endlat - startlat) / mtodeglat;
				    linedistance = sqrt(dx * dx + dy * dy);
				    linebearing = RTD * atan2(dx, dy);
				    if (linebearing < 0.0)
		    			linebearing += 360.0;
				    if (linebearing >= 45.0 && linebearing <= 225.0)
				        sprintf(scale, "-Jx%f/%f", xscale, yscale);
				    else
				        sprintf(scale, "-Jx-%f/%f", xscale, yscale);

				    /* output commands to first cut plotting script file */
				    /* The maximum useful plot length is about nshotmax shots, so
		    			we break longer files up into multiple plots */
				    nshot = endshot - startshot + 1;
				    nplot = nshot / nshotmax;
				    if (nwrite % nshotmax > 0)
		    			nplot++;
				    sweep = (seafloordepthmax - seafloordepthmin) / 750.0 + 0.1;
				    sweep = (1 + (int)(sweep / 0.05)) * 0.05; 
				    delay = seafloordepthmin / 750.0;
				    delay = ((int)(delay / 0.05)) * 0.05; 
				    fprintf(sfp, "# Generate %d section plot(s) of segy file: %s\n", nplot, output_file);
				    fprintf(sfp, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
				    fprintf(sfp, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
				    fprintf(sfp, "#   Section length: %f km\n", linedistance);
				    fprintf(sfp, "#   Section bearing: %f degrees\n", linebearing);
				    fprintf(stderr, "# Generate %d section plot(s) of segy file: %s\n", nplot, output_file);
				    fprintf(stderr, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
				    fprintf(stderr, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
				    fprintf(stderr, "#   Section length: %f km\n", linedistance);
				    fprintf(stderr, "#   Section bearing: %f degrees\n", linebearing);
				    for (i=0;i<nplot;i++)
		    			{
		        		sprintf(command, "#   Section plot %d of %d\n", i + 1, nplot);
					fprintf(stderr, "%s", command);
					fprintf(sfp, "%s", command);

					sprintf(command, "mbsegygrid -I %s \\\n\t-S0/%d/%d -T%.2f/%.2f \\\n\t-O %s_%4.4d_%2.2d_section\n", 
							output_file, (startshot + i * nshotmax),
							MIN((startshot  + (i + 1) * nshotmax - 1), endshot),
							sweep, delay, lineroot, linenumber, i + 1);
					fprintf(stderr, "%s", command);
					fprintf(sfp, "%s", command);

					sprintf(command, "mbm_grdplot -I %s_%4.4d_%2.2d_section.grd \\\n\t%s -Z%s \\\n\t-Ba250/a0.05g0.05 -G1 -W1/4 -D -V \\\n\t-O %s_%4.4d_%2.2d_sectionplot \\\n\t-L\"%s Line %d Plot %d of %d\"\n",
							lineroot, linenumber, i + 1, scale, zbounds,
							lineroot, linenumber, i + 1, lineroot, linenumber,
							i + 1, nplot);
					fprintf(stderr, "%s", command);
					fprintf(sfp, "%s", command);

					sprintf(command, "%s_%4.4d_%2.2d_sectionplot.cmd\n\n",
							lineroot, linenumber, i + 1);
					fprintf(stderr, "%s", command);
					fprintf(sfp, "%s", command);
					fflush(sfp);
					}
				    }
				    
				/* increment line number */
				if (activewaypoint > 0)
					linenumber++;

				/* increment active waypoint */
				activewaypoint++;
				}

			if (linechange == MB_YES)
				{
				mb_coor_scale(verbose,routelat[activewaypoint], &mtodeglon, &mtodeglat);
				rangelast = 1000 * rangethreshold;
				seafloordepthmin = -1.0;
				seafloordepthmax = -1.0;
				oktowrite = 0;
				rangeok = MB_NO;
				}
			else
				rangelast = range;
			if (verbose > 0)
				fprintf(stderr,"> activewaypoint:%d linenumber:%d time_d:%f range:%f   lon: %f %f   lat: %f %f oktowrite:%d rangeok:%d kind:%d\n", 
					activewaypoint, linenumber, time_d, range, navlon, 
					routelon[activewaypoint], navlat, routelat[activewaypoint], 
					oktowrite, rangeok, kind);
			}

		/* if desired extract subbottom data */
		if (error == MB_ERROR_NO_ERROR
			&& (kind  == MB_DATA_SUBBOTTOM_MCS
				|| kind == MB_DATA_SUBBOTTOM_CNTRBEAM
				|| kind == MB_DATA_SUBBOTTOM_SUBBOTTOM))
		    {
		    /* extract the header */
		    status = mb_extract_segytraceheader(verbose,mbio_ptr,store_ptr,&kind,
				    (void *)&segytraceheader,&error);

		    /* allocate the required memory */
		    if (status == MB_SUCCESS 
		    	&& segytraceheader.nsamps > segydata_alloc)
			{
			status = mb_mallocd(verbose, __FILE__, __LINE__, segytraceheader.nsamps * samplesize,
						(void **)&segydata, &error);
			if (status == MB_SUCCESS)
				segydata_alloc = segytraceheader.nsamps;
			else
				segydata_alloc = 0;
			}
		    if (status == MB_SUCCESS 
		    	&& (buffer_alloc < MB_SEGY_TRACEHEADER_LENGTH
				|| buffer_alloc < segytraceheader.nsamps * samplesize))
			{
			buffer_alloc = MAX(MB_SEGY_TRACEHEADER_LENGTH, segytraceheader.nsamps * samplesize);
			status = mb_mallocd(verbose, __FILE__, __LINE__, buffer_alloc, (void **)&buffer, &error);
			if (status != MB_SUCCESS)
				buffer_alloc = 0;
			}

		    /* extract the data */
		    if (status == MB_SUCCESS)
			status = mb_extract_segy(verbose,mbio_ptr,store_ptr,&sampleformat,&kind,
				    (void *)&segytraceheader,segydata,&error);
				    
		    /* apply time shift if needed */
		    if (status == MB_SUCCESS && timeshift != 0.0)
		    	{
			time_j[0] = segytraceheader.year;
			time_j[1] = segytraceheader.day_of_yr;
			time_j[2] = 60 * segytraceheader.hour + segytraceheader.min;
			time_j[3] = segytraceheader.sec;
			time_j[4] = 1000 * segytraceheader.mils;
			mb_get_itime(verbose,time_j,time_i);
			mb_get_time(verbose,time_i,&time_d);
			time_d += timeshift;
			mb_get_date(verbose,time_d,time_i);
			mb_get_jtime(verbose,time_i,time_j);
			segytraceheader.year = time_i[0];
			segytraceheader.day_of_yr = time_j[1];
			segytraceheader.hour = time_i[3];
			segytraceheader.min = time_i[4];
			segytraceheader.sec = time_i[5];
			}
		    
		    /* set nav and heading using most recent survey data */
		    segytraceheader.src_long = (int)(lastlon * 360000.0);
		    segytraceheader.src_lat = (int)(lastlat * 360000.0);
		    segytraceheader.heading = lastheading;
			
		    /* if following a route check that the vehicle has come on line 
		    	(within MBES_ONLINE_THRESHOLD degrees)
		    	before writing any data */
		    if (activewaypoint > 0 && checkroutebearing == MB_YES 
		    	&& nroutepoint > 1)
		    	{
			headingdiff = fabs(routeheading[activewaypoint-1] - segytraceheader.heading);
			if (headingdiff > 180.0)
				headingdiff = 360.0 - headingdiff;
			if (headingdiff < MBES_ONLINE_THRESHOLD)
				oktowrite++;
			else
				oktowrite = 0;
/* fprintf(stderr,"heading: %f %f %f oktowrite:%d\n", 
routeheading[activewaypoint-1],segytraceheader.heading,headingdiff,oktowrite);*/
			}
		    else if (activewaypoint > 0)
		    	oktowrite = MBES_ONLINE_COUNT;
		    else if (nroutepoint == 0 && ntimepoint == 0)
		    	oktowrite = MBES_ONLINE_COUNT;
/*if (status == MB_SUCCESS)
fprintf(stderr,"activewaypoint:%d linenumber:%d range:%f   lon: %f %f   lat: %f %f oktowrite:%d\n", 
activewaypoint,linenumber,range, navlon, 
routelon[activewaypoint], navlat, routelat[activewaypoint], oktowrite);*/

		    /* open output segy file if needed */
		    if (fp == NULL && oktowrite > 0)
			{
			/* set up output filename */
			if (output_file_set == MB_NO)
			    {
			    if (nroutepoint > 1 || ntimepoint > 1)
				    {
				    sprintf(output_file, "%s_%4.4d.segy", lineroot, linenumber);
				    }
			    else
				    {
				    strcpy(output_file, file);
				    strcat(output_file,".segy");
				    }
			    }
			    
			/* open the new file */
			nwrite = 0;
			if ((fp = fopen(output_file, "w")) == NULL) 
				{
				status = MB_FAILURE;
				error = MB_ERROR_WRITE_FAIL;
				fprintf(stderr,"\nError opening output segy file:\n%s\n",
					output_file);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
			else if (verbose > 0)
				{
				/* output info on file output */
				fprintf(stderr,"Outputting subbottom data to segy file %s\n",
					output_file);
				}
			sprintf(output_list_file,"%s.txt",output_file);
			if ((cfp = fopen(output_list_file, "w")) == NULL) 
				{
				status = MB_FAILURE;
				error = MB_ERROR_WRITE_FAIL;
				fprintf(stderr,"\nError opening output segy list file:\n%s\n",
					output_list_file);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
			}
				    
		    /* note good status */
		    if (status == MB_SUCCESS)
		    	{
			/* get trace min and max */
			tracemin = segydata[0];
			tracemax = segydata[0];
			tracerms = 0.0;
			for (i=0;i<segytraceheader.nsamps;i++)
				{
				tracemin = MIN(tracemin, segydata[i]);
				tracemax = MAX(tracemax, segydata[i]);
				tracerms += segydata[i] * segydata[i];
				}
			tracerms = sqrt(tracerms / segytraceheader.nsamps);
				
			/* get starting and ending positions */
			if (nwrite == 0)
				{
				startlon = ((double)segytraceheader.src_long) / 360000.0;
				startlat = ((double)segytraceheader.src_lat) / 360000.0;
				startshot = segytraceheader.shot_num;
				linetracemin = tracemin;
				linetracemax = tracemax;
				}
			else
				{
				endlon = ((double)segytraceheader.src_long) / 360000.0;
				endlat = ((double)segytraceheader.src_lat) / 360000.0;
				endshot = segytraceheader.shot_num;
				linetracemin = MIN(tracemin,linetracemin);
				linetracemax = MAX(tracemax,linetracemax);
				}
				
			/* get seafloor depth min and max */
			if (segytraceheader.src_wbd > 0)
				{
				if (seafloordepthmin < 0.0)
					{
					seafloordepthmin = 0.01 * ((double) segytraceheader.src_wbd);
					seafloordepthmax = 0.01 * ((double) segytraceheader.src_wbd);
					}
				else
					{
					seafloordepthmin = MIN(seafloordepthmin, 0.01 * ((double) segytraceheader.src_wbd));
					seafloordepthmax = MAX(seafloordepthmax, 0.01 * ((double) segytraceheader.src_wbd));
					}
				}
			/* output info */
			nread++;
			if (nread % 10 == 0 && verbose > 0)
			fprintf(stderr,"file:%s record:%d shot:%d  %4.4d/%3.3d %2.2d:%2.2d:%2.2d.%3.3d samples:%d interval:%d usec  minmax: %f %f\n",
				file,nread,segytraceheader.shot_num,
				segytraceheader.year,segytraceheader.day_of_yr,
				segytraceheader.hour,segytraceheader.min,segytraceheader.sec,segytraceheader.mils,
				segytraceheader.nsamps,segytraceheader.si_micros,tracemin,tracemax);

			/* only write data if ok */
			if (oktowrite >= MBES_ONLINE_COUNT)
				{	
				/* write characteristics file */
				fprintf(stderr,"%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d  %d %d %d   %f %f %f  %f %f %f %f\n",
					time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
					segytraceheader.shot_num,segytraceheader.nsamps,segytraceheader.si_micros,
					tracemin,tracemax,tracerms,
					sonardepth,altitude,roll,pitch);
				    
				/* write fileheader if needed */
				if (status == MB_SUCCESS && nwrite == 0)
		    		    {
				    segyfileheader.line = linenumber;
				    segyfileheader.format = 5;
				    segyfileheader.channels = 1;
				    segyfileheader.aux_channels = 0;
				    segyfileheader.sample_interval = segytraceheader.si_micros;
				    segyfileheader.sample_interval_org = segytraceheader.si_micros;
				    segyfileheader.number_samples = segytraceheader.nsamps;

				    /* insert file header data into output buffer */
				    index = 0;
				    mb_put_binary_int(MB_NO, segyfileheader.jobid, (void *) &(buffer[index])); index += 4;
				    mb_put_binary_int(MB_NO, segyfileheader.line, (void *) &(buffer[index])); index += 4;
				    mb_put_binary_int(MB_NO, segyfileheader.reel, (void *) &(buffer[index])); index += 4;
				    mb_put_binary_short(MB_NO, segyfileheader.channels, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.aux_channels, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.sample_interval, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.sample_interval_org, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.number_samples, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.number_samples_org, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.format, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.cdp_fold, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.trace_sort, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.vertical_sum, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.sweep_start, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.sweep_end, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.sweep_length, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.sweep_type, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.sweep_trace, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.sweep_taper_start, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.sweep_taper_end, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.sweep_taper, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.correlated, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.binary_gain, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.amplitude, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.units, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.impulse_polarity, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.vibrate_polarity, (void *) &(buffer[index])); index += 2;
				    mb_put_binary_short(MB_NO, segyfileheader.domain, (void *) &(buffer[index])); index += 2;
				    for (i=0;i<338;i++)
					    {
					    buffer[index] = segyfileheader.extra[i]; index++;
					    }

				    segyfileheader.number_samples_org = segytraceheader.nsamps;
				    if (fwrite(&segyasciiheader, 1, MB_SEGY_ASCIIHEADER_LENGTH, fp) 
							    != MB_SEGY_ASCIIHEADER_LENGTH)
					    {
					    status = MB_FAILURE;
					    error = MB_ERROR_WRITE_FAIL;
					    }
				    else if (fwrite(buffer, 1, MB_SEGY_FILEHEADER_LENGTH, fp) 
							    != MB_SEGY_FILEHEADER_LENGTH)
					    {
					    status = MB_FAILURE;
					    error = MB_ERROR_WRITE_FAIL;
					    }
				    }

				/* insert segy header data into output buffer */
				index = 0;
				mb_put_binary_int(MB_NO, segytraceheader.seq_num, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.seq_reel, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.shot_num, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.shot_tr, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.espn, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.rp_num, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.rp_tr, (void *) &buffer[index]); index += 4;
				mb_put_binary_short(MB_NO, segytraceheader.trc_id, (void *) &buffer[index]); index += 2;
				mb_put_binary_short(MB_NO, segytraceheader.num_vstk, (void *) &buffer[index]); index += 2;
				mb_put_binary_short(MB_NO, segytraceheader.cdp_fold, (void *) &buffer[index]); index += 2;
				mb_put_binary_short(MB_NO, segytraceheader.use, (void *) &buffer[index]); index += 2;
				mb_put_binary_int(MB_NO, segytraceheader.range, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.grp_elev, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.src_elev, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.src_depth, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.grp_datum, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.src_datum, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.src_wbd, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.grp_wbd, (void *) &buffer[index]); index += 4;
        			mb_put_binary_short(MB_NO, segytraceheader.elev_scalar, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.coord_scalar, (void *) &buffer[index]); index += 2;
				mb_put_binary_int(MB_NO, segytraceheader.src_long, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.src_lat, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.grp_long, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.grp_lat, (void *) &buffer[index]); index += 4;
        			mb_put_binary_short(MB_NO, segytraceheader.coord_units, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.wvel, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.sbvel, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.src_up_vel, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.grp_up_vel, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.src_static, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.grp_static, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.tot_static, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.laga, (void *) &buffer[index]); index += 2;
				mb_put_binary_int(MB_NO, segytraceheader.delay_mils, (void *) &buffer[index]); index += 4;
        			mb_put_binary_short(MB_NO, segytraceheader.smute_mils, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.emute_mils, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.nsamps, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.si_micros, (void *) &buffer[index]); index += 2;
				for (i=0;i<19;i++)
					{
        				mb_put_binary_short(MB_NO, segytraceheader.other_1[i], (void *) &buffer[index]); index += 2;
					}
        			mb_put_binary_short(MB_NO, segytraceheader.year, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.day_of_yr, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.hour, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.min, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.sec, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.mils, (void *) &buffer[index]); index += 2;
        			mb_put_binary_short(MB_NO, segytraceheader.tr_weight, (void *) &buffer[index]); index += 2;
				for (i=0;i<5;i++)
					{
        				mb_put_binary_short(MB_NO, segytraceheader.other_2[i], (void *) &buffer[index]); index += 2;
					}
				mb_put_binary_float(MB_NO, segytraceheader.delay, (void *) &buffer[index]); index += 4;
        			mb_put_binary_float(MB_NO, segytraceheader.smute_sec, (void *) &buffer[index]); index += 4;
        			mb_put_binary_float(MB_NO, segytraceheader.emute_sec, (void *) &buffer[index]); index += 4;
        			mb_put_binary_float(MB_NO, segytraceheader.si_secs, (void *) &buffer[index]); index += 4;
        			mb_put_binary_float(MB_NO, segytraceheader.wbt_secs, (void *) &buffer[index]); index += 4;
				mb_put_binary_int(MB_NO, segytraceheader.end_of_rp, (void *) &buffer[index]); index += 4;
        			mb_put_binary_float(MB_NO, segytraceheader.dummy1, (void *) &buffer[index]); index += 4;
        			mb_put_binary_float(MB_NO, segytraceheader.dummy2, (void *) &buffer[index]); index += 4;
        			mb_put_binary_float(MB_NO, segytraceheader.dummy3, (void *) &buffer[index]); index += 4;
        			mb_put_binary_float(MB_NO, segytraceheader.dummy4, (void *) &buffer[index]); index += 4;
        			mb_put_binary_float(MB_NO, segytraceheader.dummy5, (void *) &buffer[index]); index += 4;
        			mb_put_binary_float(MB_NO, segytraceheader.dummy6, (void *) &buffer[index]); index += 4;
        			mb_put_binary_float(MB_NO, segytraceheader.dummy7, (void *) &buffer[index]); index += 4;
        			mb_put_binary_float(MB_NO, segytraceheader.dummy8, (void *) &buffer[index]); index += 4;
        			mb_put_binary_float(MB_NO, segytraceheader.heading, (void *) &buffer[index]); index += 4;

				/* write out segy header */
				if (fwrite(buffer,1,MB_SEGY_TRACEHEADER_LENGTH,fp) 
							!= MB_SEGY_TRACEHEADER_LENGTH)
					{
					status = MB_FAILURE;
					error = MB_ERROR_WRITE_FAIL;
					}

				/* insert segy data into output buffer */
				index = 0;
				for (i=0;i<segytraceheader.nsamps;i++)
					{
        				mb_put_binary_float(MB_NO, segydata[i], (void *) &buffer[index]); index += 4;
					}

				/* write out data */
				nwrite++;
				if (status == MB_SUCCESS
					&& fwrite(buffer, 1, segytraceheader.nsamps * samplesize, fp) 
							!= segytraceheader.nsamps * samplesize)
					{
					status = MB_FAILURE;
					error = MB_ERROR_WRITE_FAIL;
					}
				}
			
			}
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
		}

	/* close the swath file */
	status = mb_close(verbose,&mbio_ptr,&error);
	
	/* output read statistics */
	fprintf(stderr,"%d records read from %s\n", nread, file);

	/* deallocate memory used for segy data arrays */
	mb_freed(verbose,__FILE__,__LINE__,(void **)&segydata,&error); 
	segydata_alloc = 0;
	mb_freed(verbose,__FILE__,__LINE__,(void **)&buffer,&error); 
	buffer_alloc = 0;

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if ((status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error))
			    == MB_SUCCESS)
                        read_data = MB_YES;
                else
                        read_data = MB_NO;
                }
        else
                {
                read_data = MB_NO;
                }

	/* close output file if conditions warrent */
	if (read_data == MB_NO
		|| (output_file_set == MB_NO && nroutepoint < 2 && ntimepoint < 2))
		{			
		/* close current output file */
		if (fp != NULL)
		    {
		    fclose(fp);
		    fp = NULL;

		    /* output count of segy records */
		    fprintf(stderr,"\n%d records output to segy file %s\n",
					nwrite, output_file);
		    if (verbose > 0)
			fprintf(stderr,"\n");
				    
		    /* use mbsegyinfo to generate a sinf file */
		    sprintf(command, "mbsegyinfo -I %s -O", output_file);
		    fprintf(stderr, "Executing: %s\n", command);
		    system(command);
		    
		    /* get bearing and plot scale */
		    dx = (endlon - startlon) / mtodeglon;
		    dy = (endlat - startlat) / mtodeglat;
		    linedistance = sqrt(dx * dx + dy * dy);
		    if (linebearing < 0.0)
		    	linebearing += 360.0;
		    if (linebearing >= 45.0 && linebearing <= 225.0)
			sprintf(scale, "-Jx%f/%f", xscale, yscale);
		    else
			sprintf(scale, "-Jx-%f/%f", xscale, yscale);
				    
		    /* output commands to first cut plotting script file */
		    /* The maximum useful plot length is about nshotmax shots, so
		    	we break longer files up into multiple plots */
		    nshot = endshot - startshot + 1;
		    nplot = nshot / nshotmax;
		    if (nwrite % nshotmax > 0)
		    	nplot++;
/*fprintf(stderr,"seafloordepthmin:%f seafloordepthmax:%f\n", seafloordepthmin, seafloordepthmax);*/
		    sweep = (seafloordepthmax - seafloordepthmin) / 750.0 + 0.1;
		    sweep = (1 + (int)(sweep / 0.05)) * 0.05; 
		    delay = seafloordepthmin / 750.0;
		    delay = ((int)(delay / 0.05)) * 0.05; 
		    fprintf(sfp, "# Generate %d section plot(s) of segy file: %s\n", nplot, output_file);
		    fprintf(sfp, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
		    fprintf(sfp, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
		    fprintf(sfp, "#   Section length: %f km\n", linedistance);
		    fprintf(sfp, "#   Section bearing: %f degrees\n", linebearing);
		    fprintf(stderr, "# Generate %d section plot(s) of segy file: %s\n", nplot, output_file);
		    fprintf(stderr, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
		    fprintf(stderr, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
		    fprintf(stderr, "#   Section length: %f km\n", linedistance);
		    fprintf(stderr, "#   Section bearing: %f degrees\n", linebearing);
		    for (i=0;i<nplot;i++)
		    	{
		        sprintf(command, "#   Section plot %d of %d\n", i + 1, nplot);
			fprintf(stderr, "%s", command);
			fprintf(sfp, "%s", command);

			sprintf(command, "mbsegygrid -I %s \\\n\t-S0/%d/%d -T%.2f/%.2f \\\n\t-O %s_%4.4d_%2.2d_section\n", 
					output_file, (startshot + i * nshotmax),
					MIN((startshot  + (i + 1) * nshotmax - 1), endshot),
					sweep, delay, lineroot, linenumber, i + 1);
			fprintf(stderr, "%s", command);
			fprintf(sfp, "%s", command);

			sprintf(command, "mbm_grdplot -I %s_%4.4d_%2.2d_section.grd \\\n\t%s -Z%s \\\n\t-Ba250/a0.05g0.05 -G1 -W1/4 -D -V \\\n\t-O %s_%4.4d_%2.2d_sectionplot \\\n\t-L\"%s Line %d Plot %d of %d\"\n",
					lineroot, linenumber, i + 1, scale, zbounds,
					lineroot, linenumber, i + 1, lineroot, linenumber,
					i + 1, nplot);
			fprintf(stderr, "%s", command);
			fprintf(sfp, "%s", command);

			sprintf(command, "%s_%4.4d_%2.2d_sectionplot.cmd\n\n",
					lineroot, linenumber, i + 1);
			fprintf(stderr, "%s", command);
			fprintf(sfp, "%s", command);
			fflush(sfp);
			}
				    
		    /* increment line number */
		    linenumber++;
		    }
		}

	/* end loop over files in list */
	}
	if (read_datalist == MB_YES)
		mb_datalist_close(verbose,&datalist,&error);
	    
	/* close plotting script file */
	fclose(sfp);
	sprintf(command, "chmod +x %s", scriptfile);
	system(command);
		
	/* deallocate route arrays */
	if (route_file_set == MB_YES)
		{	    
		status = mb_freed(verbose,__FILE__,__LINE__, (void **)&routelon, &error);
		status = mb_freed(verbose,__FILE__,__LINE__, (void **)&routelat, &error);
		status = mb_freed(verbose,__FILE__,__LINE__, (void **)&routeheading, &error);
		status = mb_freed(verbose,__FILE__,__LINE__, (void **)&routewaypoint, &error);
		}

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
