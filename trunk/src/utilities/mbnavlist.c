/*--------------------------------------------------------------------
 *    The MB-system:	mbnavlist.c	2/1/93
 *    $Id: mbnavlist.c,v 4.1 2000-03-08 00:03:45 caress Exp $
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
 * mbnavlist prints the specified contents of navigation records
 * in a swath sonar data file to stdout. The form of the 
 * output is quite flexible; mbnavlist is tailored to produce 
 * ascii files in spreadsheet style with data columns separated by tabs.
 *
 * Author:	D. W. Caress
 * Date:	November 11, 1999
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1999/12/29  00:58:18  caress
 * Release 4.6.8
 *
 * Revision 1.1  1999/12/29  00:35:11  caress
 * Initial revision
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"

/* local options */
#define	MAX_OPTIONS	25

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mbnavlist.c,v 4.1 2000-03-08 00:03:45 caress Exp $";
	static char program_name[] = "mbnavlist";
	static char help_message[] =  "mbnavlist prints the specified contents of navigation records\nin a swath sonar data file to stdout. The form of the \noutput is quite flexible; mbnavlist is tailored to produce \nascii files in spreadsheet style with data columns separated by tabs.";
	static char usage_message[] = "mbnavlist [-Byr/mo/da/hr/mn/sc -Eyr/mo/da/hr/mn/sc \n-Fformat -H -Ifile -Llonflip \n-Ooptions -Rw/e/s/n -Sspeed -Ttimegap -V]";
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
	char	read_file[128];
	FILE	*fp;
	int	format;
	int	format_num;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[128];
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* output format list controls */
	char	list[MAX_OPTIONS];
	int	n_list;
	double	distance_total;
	int	nread;
	int	time_j[5];
	int	first = MB_YES;

	/* MBIO read values */
	char	*mbio_ptr = NULL;
	char	*store_ptr;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[256];
	int	icomment = 0;

	/* additional time variables */
	int	first_m = MB_YES;
	double	time_d_ref;
	struct tm	time_tm;
	int	first_u = MB_YES;
	time_t	time_u;
	time_t	time_u_ref;

	/* crosstrack slope values */
	double	dlon, dlat, minutes;
	int	degrees;
	char	hemi;
	double	headingx, headingy, mtodeglon, mtodeglat;

	/* course calculation variables */
	double	course, course_old;
	double	time_d_old;
	double	time_interval;
	double	speed_made_good, speed_made_good_old;
	double	navlon_old, navlat_old;
	double	dx, dy;

	int	read_data;
	char	line[128];
	int	i, j, k;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (read_file, "stdin");

	/* set up the default list controls 
		(lon, lat, along-track distance, center beam depth) */
	list[0]='t';
	list[1]='M';
	list[2]='X';
	list[3]='Y';
	list[4]='H';
	list[5]='s';
	n_list = 6;

	/* process argument list */
	while ((c = getopt(argc, argv, "B:b:E:e:F:f:I:i:L:l:O:o:R:r:S:s:T:t:VvHh")) != -1)
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
		case 'R':
		case 'r':
			sscanf (optarg,"%lf/%lf/%lf/%lf", 
				&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%lf", &speedmin);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf", &timegap);
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
		fprintf(stderr,"dbg2       file:           %s\n",file);
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

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* open file list */
	if (read_datalist == MB_YES)
	    {
	    if ((fp = fopen(read_file,"r")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open data list file: %s\n",
			read_file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    if (fgets(line,128,fp) != NULL
		&& sscanf(line,"%s %d",file,&format) == 2)
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
	/* check format */
	status = mb_format(verbose,&format,&format_num,&error);

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
	status = mb_malloc(verbose,beams_bath*sizeof(char),&beamflag,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&bath,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathacrosstrack,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathalongtrack,&error);
	status = mb_malloc(verbose,beams_amp*sizeof(double),&amp,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ssacrosstrack,
			&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ssalongtrack,
			&error);

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
	distance_total = 0.0;
	nread = 0;
	first = MB_YES;
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read a ping of data */
		status = mb_get_all(verbose,mbio_ptr,&store_ptr,&kind,
			time_i,&time_d,&navlon,&navlat,&speed,
			&heading,&distance,
			&beams_bath,&beams_amp,&pixels_ss,
			beamflag,bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);

		/* time gaps are not a problem here */
		if (error == MB_ERROR_TIME_GAP)
			{
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
			
		/* check for appropriate navigation record */
		if (error <= MB_ERROR_NO_ERROR
			&& kind != mb_nav_source[format_num])
			{
			error = MB_ERROR_IGNORE;
			status = MB_FAILURE;
			}

		/* increment counter */
		if (error == MB_ERROR_NO_ERROR)
			nread++;

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",kind);
			fprintf(stderr,"dbg2       error:          %d\n",error);
			fprintf(stderr,"dbg2       status:         %d\n",status);
			}
/*fprintf(stdout, "kind:%d error:%d time:%4d/%2d/%2d %2.2d:%2.2d:%2.2d.%6.6d\n", 
kind, error, time_i[0],  time_i[1],  time_i[2],  
time_i[3],  time_i[4],  time_i[5],   time_i[6]);*/
		
		/* calculate course made good and distance */
		if (error == MB_ERROR_NO_ERROR)
			{
			mb_coor_scale(verbose,navlat,&mtodeglon,&mtodeglat);
			headingx = sin(DTR*heading);
			headingy = cos(DTR*heading);
			if (first == MB_YES)
				{
				time_interval = 0.0;
				course = heading;
				speed_made_good = 0.0;
				course_old = heading;
				speed_made_good_old = speed;
				distance = 0.0;
				}
			else
				{
				time_interval = time_d - time_d_old;
				dx = (navlon - navlon_old)/mtodeglon;
				dy = (navlat - navlat_old)/mtodeglat;
				distance = sqrt(dx*dx + dy*dy);
				if (distance > 0.0)
					course = RTD*atan2(dx/distance,dy/distance);
				else
					course = course_old;
				if (course < 0.0)
					course = course + 360.0;
				if (time_interval > 0.0)
					speed_made_good = 3.6*distance/time_interval;
				else
					speed_made_good 
						= speed_made_good_old;
				}
			distance_total += distance;
			}

		/* reset old values */
		if (error == MB_ERROR_NO_ERROR)
			{
			navlon_old = navlon;
			navlat_old = navlat;
			course_old = course;
			speed_made_good_old = speed_made_good;
			time_d_old = time_d;
			}

		/* now loop over list of output parameters */
		if (error == MB_ERROR_NO_ERROR)
		    {
		    for (i=0; i<n_list; i++) 
			{
			switch (list[i]) 
				{
				case 'H': /* heading */
					printf("%5.1f",heading);
					break;
				case 'h': /* course */
					printf("%5.1f",course);
					break;
				case 'J': /* time string */
					mb_get_jtime(verbose,time_i,time_j);
					printf(
					"%.4d %.3d %.2d %.2d %.2d.%6.6d",
					time_j[0],time_j[1],
					time_i[3],time_i[4],
					time_i[5],time_i[6]);
					break;
				case 'j': /* time string */
					mb_get_jtime(verbose,time_i,time_j);
					printf(
					"%.4d %.3d %.4d %.2d.%6.6d",
					time_j[0],time_j[1],
					time_j[2],time_j[3],time_j[4]);
					break;
				case 'L': /* along-track dist. */
					printf("%7.3f",distance_total);
					break;
				case 'M': /* Decimal unix seconds since 
						1/1/70 00:00:00 */
					printf("%.6f",time_d);
					break;
				case 'm': /* time in decimal seconds since 
						first record */
					if (first_m == MB_YES)
						{
						time_d_ref = time_d;
						first_m = MB_NO;
						}
					printf("%.6f",time_d - time_d_ref);
					break;
				case 'S': /* speed */
					printf("%5.2f",speed);
					break;
				case 's': /* speed made good */
					printf("%5.2f",speed_made_good);
					break;
				case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
					printf(
					"%.4d/%.2d/%.2d/%.2d/%.2d/%.2d.%.6d",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],
					time_i[6]);
					break;
				case 't': /* yyyy mm dd hh mm ss time string */
					printf(
					"%.4d %.2d %.2d %.2d %.2d %.2d.%.6d",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],
					time_i[6]);
					break;
				case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
					time_u = (int) time_d;
					printf("%d",time_u);
					break;
				case 'u': /* time in seconds since first record */
					time_u = (int) time_d;
					if (first_u == MB_YES)
						{
						time_u_ref = time_u;
						first_u = MB_NO;
						}
					printf("%d",time_u - time_u_ref);
					break;
				case 'V': /* time in seconds since last ping */
				case 'v': 
					if ( fabs(time_interval) > 100. )
						printf("%g",time_interval); 
					else
						printf("%7.3f",time_interval);
					break;
				case 'X': /* longitude decimal degrees */
					dlon = navlon;
					printf("%11.6f",dlon);
					break;
				case 'x': /* longitude degress + decimal minutes */
					dlon = navlon;
					if (dlon < 0.0)
						{
						hemi = 'W';
						dlon = -dlon;
						}
					else
						hemi = 'E';
					degrees = (int) dlon;
					minutes = 60.0*(dlon - degrees);
					printf("%3d %8.5f%c",
						degrees, minutes, hemi);
					break;
				case 'Y': /* latitude decimal degrees */
					dlat = navlat;
					printf("%11.6f",dlat);
					break;
				case 'y': /* latitude degrees + decimal minutes */
					dlat = navlat;
					if (dlat < 0.0)
						{
						hemi = 'S';
						dlat = -dlat;
						}
					else
						hemi = 'N';
					degrees = (int) dlat;
					minutes = 60.0*(dlat - degrees);
					printf("%3d %8.5f%c",
						degrees, minutes, hemi);
					break;
				default:
					printf("<Invalid Option: %c>",
						list[i]);
					break;
				}
				if (i<(n_list-1)) printf ("\t");
				else printf ("\n");
			}
		    first = MB_NO;
		    }
		}

	/* close the swath file */
	status = mb_close(verbose,&mbio_ptr,&error);

	/* deallocate memory used for data arrays */
	mb_free(verbose,&beamflag,&error); 
	mb_free(verbose,&bath,&error); 
	mb_free(verbose,&bathacrosstrack,&error); 
	mb_free(verbose,&bathalongtrack,&error); 
	mb_free(verbose,&amp,&error); 
	mb_free(verbose,&ss,&error); 
	mb_free(verbose,&ssacrosstrack,&error); 
	mb_free(verbose,&ssalongtrack,&error); 

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
                if (fgets(line,128,fp) != NULL
                        && sscanf(line,"%s %d",file,&format) == 2)
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
		fclose (fp);

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
