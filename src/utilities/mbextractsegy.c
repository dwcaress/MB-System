/*--------------------------------------------------------------------
 *    The MB-system:	mbextractsegy.c	4/18/2004
 *    $Id: mbextractsegy.c,v 5.11 2006-08-09 22:41:27 caress Exp $
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
 * mbextractsegy extracts subbottom profiler, center beam reflection,
 * or seismic reflection data from data supported by MB-System and
 * rewrites it as a SEGY file in the form used by SIOSEIS. .
 *
 * Author:	D. W. Caress
 * Date:	April 18, 2004
 *
 * $Log: not supported by cvs2svn $
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
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_segy.h"

/* defines */
#define MBES_ALLOC_NUM			128
#define MBES_ROUTE_WAYPOINT_NONE		0
#define MBES_ROUTE_WAYPOINT_SIMPLE	1
#define MBES_ROUTE_WAYPOINT_TRANSIT	2
#define MBES_ROUTE_WAYPOINT_STARTLINE	3
#define MBES_ROUTE_WAYPOINT_ENDLINE	4

static char rcs_id[] = "$Id: mbextractsegy.c,v 5.11 2006-08-09 22:41:27 caress Exp $";

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char program_name[] = "MBextractsegy";
	static char help_message[] =  "MBextractsegy extracts subbottom profiler, center beam reflection,\nor seismic reflection data from data supported by MB-System and\nrewrites it as a SEGY file in the form used by SIOSEIS.";
	static char usage_message[] = "mbextractsegy [-Byr/mo/dy/hr/mn/sc/us -Eyr/mo/dy/hr/mn/sc/us -Fformat -Ifile -H -Osegyfile -Ssampleformat -V]";
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
	char	output_file[MB_PATH_MAXLINE];
	int	output_file_set = MB_NO;
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	int	format;
	int	pings;
	int	pings_read;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[MB_PATH_MAXLINE];
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
	double	draft;
	double	roll;
	double	pitch;
	double	heave;
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
	int	sampleformat = MB_SEGY_SAMPLEFORMAT_NONE;
	int	samplesize = 0;
	struct mb_segyasciiheader_struct segyasciiheader;
	struct mb_segyfileheader_struct segyfileheader;
	struct mb_segytraceheader_struct segytraceheader;
	int	segydata_alloc = 0;
	float	*segydata = NULL;
	int	buffer_alloc = 0;
	char	*buffer = NULL;
	
	/* route and auto-line data */
	char	route_file[MB_PATH_MAXLINE];
	int	route_file_set = MB_NO;
	int	rawroutefile = MB_NO;
	char	lineroot[MB_PATH_MAXLINE];
	int	nroutepoint = 0;
	int	nroutepointalloc = 0;
	double	lon;
	double	lat;
	double	topo;
	int	waypoint;
	double	*routelon = NULL;
	double	*routelat = NULL;
	int	*routewaypoint = NULL;
	double	range;
	double	rangethreshold = 10.0;
	double	rangelast;
	int	activewaypoint = 0;
	int	startline = 1;
	int	linenumber;
	
	/* auto plotting */
	FILE	*sfp = NULL;
	char	scriptfile[MB_PATH_MAXLINE];
	double	seafloordepthmin;
	double	seafloordepthmax;
	double	sweep;
	double	delay;
	double	startlon;
	double	startlat;
	double	endlon;
	double	endlat;
	double	linedistance;
	double	linebearing;

	char	command[MB_PATH_MAXLINE];
	char	scale[MB_PATH_MAXLINE];
	double	mtodeglon, mtodeglat;
	double	dx, dy;
	FILE	*fp = NULL;
	char	*result;
	int	nget;
	int	point_ok;
	int	read_data;
	double	distmin;
	int	found;
	int	nread;
	int	nwrite;
	int	first;
	int	index;
	double	tracemin, tracemax;
	int	closefile;
	int	i, j, k, n;

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
	while ((c = getopt(argc, argv, "B:b:D:d:E:e:F:f:I:i:L:l:O:o:R:r:S:s:T:t:VvHh")) != -1)
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
		case 'L':
		case 'l':
			sscanf (optarg,"%d/%s", &startline, lineroot);
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", output_file);
			output_file_set = MB_YES;
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
		fprintf(stderr,"dbg2       format:         %d\n",format);
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
		fprintf(stderr,"dbg2       sampleformat:   %d\n",sampleformat);
		fprintf(stderr,"dbg2       timeshift:      %f\n",timeshift);
		fprintf(stderr,"dbg2       file:           %s\n",file);
		fprintf(stderr,"dbg2       route_file_set: %d\n",route_file_set);
		fprintf(stderr,"dbg2       route_file:     %s\n",route_file);
		fprintf(stderr,"dbg2       output_file_set:%d\n",output_file_set);
		fprintf(stderr,"dbg2       output_file:    %s\n",output_file);
		fprintf(stderr,"dbg2       lineroot:       %s\n",lineroot);
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

	/* if specified read route file */
	if (route_file_set == MB_YES)
		{	    
		/* open the input file */
		if ((fp = fopen(route_file, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			status == MB_FAILURE;
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
				nget = sscanf(comment,"%lf %lf %lf %d",
				    &lon, &lat, &topo, &waypoint);
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
					status = mb_realloc(verbose, nroutepointalloc * sizeof(double),
								(char **)&routelon, &error);
					status = mb_realloc(verbose, nroutepointalloc * sizeof(double),
								(char **)&routelat, &error);
					status = mb_realloc(verbose, nroutepointalloc * sizeof(int),
								(char **)&routewaypoint, &error);
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
					routewaypoint[nroutepoint] = waypoint;
					nroutepoint++;
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

		/* output status */
		if (verbose > 0)
			{
			/* output info on file output */
			fprintf(stderr,"Read %d waypoints from route file: %s\n",
				nroutepoint, output_file);
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
			    file,&format,&file_weight,&error)
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
	if (route_file_set == MB_YES && nroutepoint > 1)
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
		status == MB_FAILURE;
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
		    
		/* check survey data position against waypoints */
		if (nroutepoint > 1 && navlon != 0.0 && navlat != 0.0)
			{
			dx = (navlon - routelon[activewaypoint]) / mtodeglon;
			dy = (navlat - routelat[activewaypoint]) / mtodeglat;
			range = sqrt(dx * dx + dy * dy);
			if (range < rangethreshold 
				&& (activewaypoint == 0 || range > rangelast) 
				&& activewaypoint < nroutepoint - 1)
				{
				/* close current output file */
				if (fp != NULL)
				    {
				    fclose(fp);
				    fp = NULL;

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
		   			strcpy(scale, "-Jx0.01/75");
				    else
		   			strcpy(scale, "-Jx-0.01/75");
				    
				    /* output commands to first cut plotting script file */
				    sweep = (seafloordepthmax - seafloordepthmin) / 750.0 + 0.1;
				    sweep = (1 + (int)(sweep / 0.05)) * 0.05; 
				    delay = seafloordepthmin / 750.0;
				    delay = ((int)(delay / 0.05)) * 0.05; 
				    fprintf(sfp, "# Generate section plot of segy file: %s\n", output_file);
				    fprintf(sfp, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
				    fprintf(sfp, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
				    fprintf(sfp, "#   Section length: %f km\n", linedistance);
				    fprintf(sfp, "#   Section bearing: %f degrees\n", linebearing);
				    fprintf(sfp, "mbsegygrid -I %s \\\n\t-S0 -T%.2f/%.2f -W3/-0.01/%.2f \\\n\t-G2/50.0/0.1 \\\n\t-O %s_%4.4d_section\n", 
						    output_file, sweep, delay, sweep, lineroot, linenumber);
				    fprintf(sfp, "mbm_grdplot -I %s_%4.4d_section.grd \\\n\t%s -Z0/400/1 \\\n\t-Ba250/a0.05g0.05 -G1 -W1/4 -D -V \\\n\t-O %s_%4.4d_sectionplot \\\n\t-L\"%s Line %d\"\n",
						    lineroot, linenumber, scale, lineroot, linenumber, lineroot, linenumber);
				    fprintf(sfp, "%s_%4.4d_sectionplot.cmd\n\n",
						    lineroot, linenumber);
				    fflush(sfp);
				    fprintf(stderr, "# Generate section plot of segy file: %s\n", output_file);
				    fprintf(stderr, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
				    fprintf(stderr, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
				    fprintf(stderr, "#   Section length: %f km\n", linedistance);
				    fprintf(stderr, "#   Section bearing: %f degrees\n", linebearing);
				    fprintf(stderr, "mbsegygrid -I %s \\\n\t-S0 -T%.2f/%.2f -W3/-0.01/%.2f \\\n\t-G2/50.0/0.1 \\\n\t-O %s_%4.4d_section\n", 
						    output_file, sweep, delay, sweep, lineroot, linenumber);
				    fprintf(stderr, "mbm_grdplot -I %s_%4.4d_section.grd \\\n\t%s -Z0/400/1 \\\n\t-Ba250/a0.05g0.05 -G1 -W1/4 -D -V \\\n\t-O %s_%4.4d_sectionplot \\\n\t-L\"%s Line %d\"\n",
						    lineroot, linenumber, scale, lineroot, linenumber, lineroot, linenumber);
				    fprintf(stderr, "%s_%4.4d_sectionplot.cmd\n\n",
						    lineroot, linenumber);
				
				    /* increment line number */
				    linenumber++;
				    }

				/* increment active waypoint */
				activewaypoint++;
				mb_coor_scale(verbose,routelat[activewaypoint], &mtodeglon, &mtodeglat);
				rangelast = 1000 * rangethreshold;
				seafloordepthmin = -1.0;
				seafloordepthmax = -1.0;
				}
			else
				rangelast = range;
			}

		/* if desired extract subbottom data */
		if (error == MB_ERROR_NO_ERROR
			&& (kind  == MB_DATA_SUBBOTTOM_MCS
				|| kind == MB_DATA_SUBBOTTOM_CNTRBEAM
				|| kind == MB_DATA_SUBBOTTOM_SUBBOTTOM))
		    {
		    /* open output segy file if needed */
		    if (fp == NULL)
			{
			/* set up output filename */
			if (output_file_set == MB_NO)
			    {
			    if (route_file_set == MB_YES && nroutepoint > 1)
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
			}

		    /* extract the header */
		    status = mb_extract_segytraceheader(verbose,mbio_ptr,store_ptr,&kind,
				    (void *)&segytraceheader,&error);

		    /* allocate the required memory */
		    if (status == MB_SUCCESS 
		    	&& segytraceheader.nsamps > segydata_alloc)
			{
			status = mb_malloc(verbose, segytraceheader.nsamps * samplesize,
						(char **)&segydata, &error);
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
			status = mb_malloc(verbose, buffer_alloc, (char **)&buffer, &error);
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
				    
		    /* write fileheader if needed */
		    if (status == MB_SUCCESS && nwrite == 0)
		    	{
			segyfileheader.format = 5;
			segyfileheader.channels = 1;
			segyfileheader.aux_channels = 0;
			segyfileheader.sample_interval = segytraceheader.si_micros;
			segyfileheader.sample_interval_org = segytraceheader.si_micros;
			segyfileheader.number_samples = segytraceheader.nsamps;
			segyfileheader.number_samples_org = segytraceheader.nsamps;
			if (fwrite(&segyasciiheader, 1, MB_SEGY_ASCIIHEADER_LENGTH, fp) 
						!= MB_SEGY_ASCIIHEADER_LENGTH)
				{
				status = MB_FAILURE;
				error = MB_ERROR_WRITE_FAIL;
				}
			else if (fwrite(&segyfileheader, 1, MB_SEGY_FILEHEADER_LENGTH, fp) 
						!= MB_SEGY_FILEHEADER_LENGTH)
				{
				status = MB_FAILURE;
				error = MB_ERROR_WRITE_FAIL;
				}
			}
				    
		    /* note good status */
		    if (status == MB_SUCCESS)
		    	{
			/* get trace min and max */
			tracemin = segydata[0];
			tracemax = segydata[0];
			for (i=0;i<segytraceheader.nsamps;i++)
				{
				tracemin = MIN(tracemin, segydata[i]);
				tracemax = MAX(tracemax, segydata[i]);
				}
				
			/* get starting and ending positions */
			if (nwrite == 0)
				{
				startlon = ((double)segytraceheader.src_long) / 360000.0;
				startlat = ((double)segytraceheader.src_lat) / 360000.0;
				}
			else
				{
				endlon = ((double)segytraceheader.src_long) / 360000.0;
				endlat = ((double)segytraceheader.src_lat) / 360000.0;
				}
				
			/* get seafloor depth min and max */
			if (segytraceheader.src_wbd > 0)
				{
				if (seafloordepthmin < 0.0)
					{
					seafloordepthmin = 0.01 * segytraceheader.src_wbd;
					seafloordepthmax = 0.01 * segytraceheader.src_wbd;
					}
				else
					{
					seafloordepthmin = MIN(seafloordepthmin, 0.01 * segytraceheader.src_wbd);
					seafloordepthmax = MAX(seafloordepthmax, 0.01 * segytraceheader.src_wbd);
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
        		mb_put_binary_float(MB_NO, segytraceheader.dummy9, (void *) &buffer[index]); index += 4;

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

	/* deallocate memory used for segy data arrays */
	mb_free(verbose,(char **)&segydata,&error); 
	segydata_alloc = 0;
	mb_free(verbose,(char **)&buffer,&error); 
	buffer_alloc = 0;

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if (status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error)
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
		|| (output_file_set == MB_NO && nroutepoint < 2))
		{			
		/* close current output file */
		if (fp != NULL)
		    {
		    fclose(fp);
		    fp = NULL;

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
		    if (linebearing < 0.0)
		    	linebearing += 360.0;
		    if (linebearing >= 45.0 && linebearing <= 225.0)
		   	strcpy(scale, "-Jx0.01/75");
		    else
		   	strcpy(scale, "-Jx-0.01/75");
				    
		    /* output commands to first cut plotting script file */
		    sweep = (seafloordepthmax - seafloordepthmin) / 750.0 + 0.1;
		    sweep = (1 + (int)(sweep / 0.05)) * 0.05; 
		    delay = seafloordepthmin / 750.0;
		    delay = ((int)(delay / 0.05)) * 0.05; 
		    fprintf(sfp, "# Generate section plot of segy file: %s\n", output_file);
		    fprintf(sfp, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
		    fprintf(sfp, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
		    fprintf(sfp, "#   Section length: %f km\n", linedistance);
		    fprintf(sfp, "#   Section bearing: %f degrees\n", linebearing);
		    fprintf(sfp, "mbsegygrid -I %s \\\n\t-S0 -T%.2f/%.2f -W3/-0.01/%.2f \\\n\t-G2/50.0/0.1 \\\n\t-O %s_%4.4d_section\n", 
				    output_file, sweep, delay, sweep, lineroot, linenumber);
		    fprintf(sfp, "mbm_grdplot -I %s_%4.4d_section.grd \\\n\t%s -Z0/400/1 \\\n\t-Ba250/a0.05g0.05 -G1 -W1/4 -D -V \\\n\t-O %s_%4.4d_sectionplot \\\n\t-L\"%s Line %d\"\n",
				    lineroot, linenumber, scale, lineroot, linenumber, lineroot, linenumber);
		    fprintf(sfp, "%s_%4.4d_sectionplot.cmd\n\n",
				    lineroot, linenumber);
		    fflush(sfp);
		    fprintf(stderr, "# Generate section plot of segy file: %s\n", output_file);
		    fprintf(stderr, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
		    fprintf(stderr, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
		    fprintf(stderr, "#   Section length: %f km\n", linedistance);
		    fprintf(stderr, "#   Section bearing: %f degrees\n", linebearing);
		    fprintf(stderr, "mbsegygrid -I %s \\\n\t-S0 -T%.2f/%.2f -W3/-0.01/%.2f \\\n\t-G2/50.0/0.1 \\\n\t-O %s_%4.4d_section\n", 
				    output_file, sweep, delay, sweep, lineroot, linenumber);
		    fprintf(stderr, "mbm_grdplot -I %s_%4.4d_section.grd \\\n\t%s -Z0/400/1 \\\n\t-Ba250/a0.05g0.05 -G1 -W1/4 -D -V \\\n\t-O %s_%4.4d_sectionplot \\\n\t-L\"%s Line %d\"\n",
				    lineroot, linenumber, scale, lineroot, linenumber, lineroot, linenumber);
		    fprintf(stderr, "%s_%4.4d_sectionplot.cmd\n\n",
				    lineroot, linenumber);
				    
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
		status = mb_free(verbose, (char **)&routelon, &error);
		status = mb_free(verbose, (char **)&routelat, &error);
		status = mb_free(verbose, (char **)&routewaypoint, &error);
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
