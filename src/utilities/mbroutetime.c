/*--------------------------------------------------------------------
 *    The MB-system:	mbroutetime.c	5/4/2009
 *    $Id$
 *
 *    Copyright (c) 2009-2014 by
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
 * mbroutetime outputs a list of the times when a survey hit the waypoints
 * of a planned survey route. This (lon lat time_d) list can then be used by mbextractsegy
 * or mb7k2ss to extract subbottom (or sidescan) data into files corresponding
 * to the lines between waypoints. The input route files are in the MBgrdviz
 * route file format. The times are in decimal epoch seconds (seconds since 1/1/1970).
 *
 * Author:	D. W. Caress
 * Date:	May 5, 2009
 * Location:	R/V Thompson, at the dock in Apia, Samoa
 *
 * $Log: mbroutetime.c,v $
 *
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

/* defines */
#define MBES_ALLOC_NUM			128
#define MBES_ROUTE_WAYPOINT_NONE		0
#define MBES_ROUTE_WAYPOINT_SIMPLE	1
#define MBES_ROUTE_WAYPOINT_TRANSIT	2
#define MBES_ROUTE_WAYPOINT_STARTLINE	3
#define MBES_ROUTE_WAYPOINT_ENDLINE	4
#define MBES_ONLINE_THRESHOLD		15.0
#define MBES_ONLINE_COUNT		30

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "MBroutetime";
	char help_message[] =  "MBroutetime outputs a list of the times when a survey hit the waypoints\nof a planned survey route. This (lon lat time_d) list can then be used by mbextractsegy\nor mb7k2ss to extract subbottom (or sidescan) data into files corresponding\nto the lines between waypoints.";
	char usage_message[] = "mbroutetime  -Rroutefile [-Fformat -Ifile -Owaypointtimefile -Urangethreshold -H -V]";
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
	char	read_file[MB_PATH_MAXLINE];
	char	output_file[MB_PATH_MAXLINE];
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
	char	file[MB_PATH_MAXLINE];
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* MBIO read values */
	void	*mbio_ptr = NULL;
	void	*store_ptr = NULL;
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

	/* route and auto-line data */
	char	route_file[MB_PATH_MAXLINE];
	int	rawroutefile = MB_NO;
	int	nroutepoint = 0;
	int	nroutepointfound = 0;
	int	nroutepointalloc = 0;
	double	lon;
	double	lat;
	double	topo;
	int	waypoint;
	double	*routelon = NULL;
	double	*routelat = NULL;
	double	*routeheading = NULL;
	int	*routewaypoint = NULL;
	double	*routetime_d = NULL;
	double	range;
	double	rangethreshold = 25.0;
	double	rangelast;
	int	activewaypoint = 0;

	double	mtodeglon, mtodeglat;
	double	lastlon;
	double	lastlat;
	double	lastheading;
	double	lasttime_d;
	double	dx, dy;
	FILE	*fp = NULL;
	char	*result;
	int	nget;
	int	point_ok;
	int	read_data;
	int	nread;
	int	i;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to datalist.mb-1 */
	strcpy (read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt(argc, argv, "F:f:I:i:O:o:R:r:U:u:VvHh")) != -1)
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
		case 'I':
		case 'i':
			sscanf (optarg,"%s", read_file);
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
			flag++;
			break;
		case 'U':
		case 'u':
			sscanf (optarg,"%lf", &rangethreshold);
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
		fprintf(stderr,"dbg2       read_file:         %s\n",read_file);
		fprintf(stderr,"dbg2       route_file:        %s\n",route_file);
		fprintf(stderr,"dbg2       output_file_set:   %d\n",output_file_set);
		fprintf(stderr,"dbg2       output_file:       %s\n",output_file);
		fprintf(stderr,"dbg2       rangethreshold:    %f\n",rangethreshold);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* read route file */
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
				|| (rawroutefile == MB_NO && nget >= 3 && waypoint > MBES_ROUTE_WAYPOINT_TRANSIT))
				point_ok = MB_YES;
			else
				point_ok = MB_NO;

			/* if good data check for need to allocate more space */
			if (point_ok == MB_YES
				&& nroutepoint + 2 > nroutepointalloc)
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
				status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double),
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
			if (point_ok == MB_YES && nroutepointalloc > nroutepoint)
				{
				routelon[nroutepoint] = lon;
				routelat[nroutepoint] = lat;
				routeheading[nroutepoint] = heading;
				routewaypoint[nroutepoint] = waypoint;
				routetime_d[nroutepoint] = 0.0;
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

	/* output status */
	if (verbose > 0)
		{
		/* output info on file output */
		fprintf(stderr,"Read %d waypoints from route file: %s\n",
			nroutepoint, route_file);
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

	/* loop over all files to be read */
	while (read_data == MB_YES)
		{
		/* read fnv file if possible */
		mb_get_fnv(verbose, file, &format, &error);

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

		/* read and use data */
		nread = 0;
		while (error <= MB_ERROR_NO_ERROR && activewaypoint < nroutepoint)
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
				/* increment counter */
				nread++;

				/* save last nav and heading */
				if (navlon != 0.0)
					lastlon = navlon;
				if (navlat != 0.0)
					lastlat = navlat;
				if (heading != 0.0)
					lastheading = heading;
				if (time_d != 0.0)
					lasttime_d = time_d;


				/* check survey data position against waypoints */
				if (navlon != 0.0 && navlat != 0.0)
					{
					dx = (navlon - routelon[activewaypoint]) / mtodeglon;
					dy = (navlat - routelat[activewaypoint]) / mtodeglat;
					range = sqrt(dx * dx + dy * dy);
					if (verbose > 0)
						fprintf(stderr,"> activewaypoint:%d time_d:%f range:%f   lon: %f %f   lat: %f %f\n",
							activewaypoint, time_d, range, navlon,
							routelon[activewaypoint], navlat, routelat[activewaypoint]);

					if (range < rangethreshold
						&& (activewaypoint == 0 || range > rangelast)
						&& activewaypoint < nroutepoint)
						{
						fprintf(stderr,"Waypoint %d of %d found with range %f m\n",
								activewaypoint, nroutepoint, range);
						routetime_d[activewaypoint] = time_d;
						activewaypoint++;
						nroutepointfound++;
						mb_coor_scale(verbose,routelat[activewaypoint], &mtodeglon, &mtodeglat);
						rangelast = 1000 * rangethreshold;
						}
					else
						rangelast = range;
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
			}

		/* close the swath file */
		status = mb_close(verbose,&mbio_ptr,&error);

		/* output read statistics */
		fprintf(stderr,"%d records read from %s\n", nread, file);

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

		/* end loop over files in list */
		}
	if (read_datalist == MB_YES)
		mb_datalist_close(verbose,&datalist,&error);

	/* if the last route point was not reached, add one last waypoint */
	if (nroutepointfound < nroutepoint)
		{
		fprintf(stderr,"Waypoint %d of %d set at end of data with range %f m to next specified waypoint\n",
				activewaypoint, nroutepoint, range);
		routelon[nroutepointfound] = lastlon;
		routelat[nroutepointfound] = lastlat;
		routeheading[nroutepointfound] = lastheading;
		routetime_d[nroutepointfound] = lasttime_d;
		routewaypoint[nroutepointfound] = MBES_ROUTE_WAYPOINT_ENDLINE;
		nroutepointfound++;
		}

	/* output time list for the route */
	if (output_file_set == MB_NO)
		{
		sprintf(output_file, "%s_wpttime_d.txt", read_file);
		}
	if ((fp = fopen(output_file, "w")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		fprintf(stderr,"\nUnable to open output waypoint time list file <%s> for writing\n",output_file);
		exit(status);
		}
	for (i=0;i<nroutepointfound;i++)
		{
		fprintf(fp,"%3d %3d %11.6f %10.6f %10.6f %.6f\n", i, routewaypoint[i], routelon[i], routelat[i], routeheading[i], routetime_d[i]);
		if (verbose > 0)
			fprintf(stderr,"%3d %3d %11.6f %10.6f %10.6f %.6f\n", i, routewaypoint[i], routelon[i], routelat[i], routeheading[i], routetime_d[i]);
		}
	fclose(fp);

	/* deallocate route arrays */
	status = mb_freed(verbose,__FILE__,__LINE__, (void **)&routelon, &error);
	status = mb_freed(verbose,__FILE__,__LINE__, (void **)&routelat, &error);
	status = mb_freed(verbose,__FILE__,__LINE__, (void **)&routeheading, &error);
	status = mb_freed(verbose,__FILE__,__LINE__, (void **)&routewaypoint, &error);
	status = mb_freed(verbose,__FILE__,__LINE__, (void **)&routetime_d, &error);

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
