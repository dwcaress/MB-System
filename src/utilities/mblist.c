/*--------------------------------------------------------------------
 *    The MB-system:	mblist.c	3.00	2/1/93
 *    $Id: mblist.c,v 3.5 1993-06-13 07:47:20 caress Exp $
 *
 *    Copyright (c) 1993 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBLIST prints the specified contents of a multibeam data  
 * file to stdout. The form of the output is quite flexible; 
 * MBLIST is tailored to produce ascii files in spreadsheet  
 * style with data columns separated by tabs.
 *
 * Author:	D. W. Caress
 * Date:	February 1, 1993
 *
 * Note:	This program is based on the program mblist created
 *		by A. Malinverno (currently at Schlumberger, formerly
 *		at L-DEO) in August 1991.  It also includes elements
 *		derived from the program mbdump created by D. Caress
 *		in 1990.
 *
 * $Log: not supported by cvs2svn $
 * Revision 3.4  1993/06/13  03:02:18  caress
 * Added new output option "A", which outputs the apparent
 * crosstrack seafloor slope in degrees from vertical. This
 * value is calculated by fitting a line to the bathymetry
 * data for each ping. Obtaining a time series of the apparent
 * seafloor slope is useful for detecting errors in the vertical
 * reference used by the multibeam sonar.
 *
 * Revision 3.3  1993/06/09  11:47:44  caress
 * Fixed problem with unix time value output. In unix time
 * months are counted 0-11 instead of 1-12.
 *
 * Revision 3.2  1993/05/17  16:37:18  caress
 * Fixed problem where program crashed if -F was used.
 *
 * Revision 3.0  1993/05/04  22:20:17  dale
 * Initial version
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

/* DTR define */
#define DTR	(M_PI/180.)

/* local options */
#define	MAX_OPTIONS			25
#define	MBLIST_MODE_LIST		1
#define	MBLIST_MODE_DUMP_BATHYMETRY	2
#define	MBLIST_MODE_DUMP_BACKSCATTER	3

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mblist.c,v 3.5 1993-06-13 07:47:20 caress Exp $";
	static char program_name[] = "MBLIST";
	static char help_message[] =  "MBLIST prints the specified contents of a multibeam data \nfile to stdout. The form of the output is quite flexible; \nMBLIST is tailored to produce ascii files in spreadsheet \nstyle with data columns separated by tabs.";
	static char usage_message[] = "mblist [-Fformat -Rw/e/s/n -Ppings -Sspeed -Llonflip\n	-Byr/mo/da/hr/mn/sc -Eyr/mo/da/hr/mn/sc -V -H -Ifile\n	-Mbath_beam -Nback_beam -Ooptions -Ddumpmode]";
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
	int	beams_back;

	/* output format list controls */
	int	dumpmode;
	char	list[MAX_OPTIONS];
	int	n_list;
	int	ibeam_list_bath = -1;
	int	ibeam_list_back = -1;
	int	beam_list_bath = -1;
	int	beam_list_back = -1;
	double	distance_total;
	int	nread;
	int	beam_status = MB_SUCCESS;
	int	time_j[4];

	/* MBIO read values */
	char	*mbio_ptr;
	int	kind;
	int	time_i[6];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	int	*bath;
	int	*bathdist;
	int	*back;
	int	*backdist;
	double	*depth;
	double	*depthlon;
	double	*depthlat;
	double	*scatter;
	double	*scatterlon;
	double	*scatterlat;
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
	double	slope;
	double	sx, sy, sxx, sxy;
	int	ns;
	double	delta, a, b, theta;

	int	i, j;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (file, "stdin");

	/* set up the default list controls 
		(lon, lat, along-track distance, center beam depth) */
	list[0]='X';
	list[1]='Y';
	list[2]='L';
	list[3]='Z';
	n_list = 4;

	/* set dump mode flag to MBLIST_MODE_LIST */
	dumpmode = MBLIST_MODE_LIST;

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:P:p:L:l:R:r:B:b:E:e:S:s:T:t:I:i:O:o:M:m:N:n:D:d:")) != -1)
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
		case 'R':
		case 'r':
			sscanf (optarg,"%lf/%lf/%lf/%lf", 
				&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
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
		case 'I':
		case 'i':
			sscanf (optarg,"%s", file);
			flag++;
			break;
		case 'O':
		case 'o':
			for(j=0,n_list=0;j<strlen(optarg);j++,n_list++)
				if (n_list<MAX_OPTIONS)
					list[n_list] = optarg[j];
			flag++;
			break;
		case 'M':
		case 'm':
			sscanf (optarg,"%d", &ibeam_list_bath);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%d", &ibeam_list_back);
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%d", &dumpmode);
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
		exit(MB_FAILURE);
		}

	/* print starting message */
	if (verbose == 1)
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
		fprintf(stderr,"dbg2       etime_i[0]:     %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:     %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:     %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:     %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:     %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:     %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       speedmin:       %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:        %f\n",timegap);
		fprintf(stderr,"dbg2       file:           %s\n",file);
		fprintf(stderr,"dbg2       mode:           %d\n",dumpmode);
		fprintf(stderr,"dbg2       beam_list_bath: %d\n",
						ibeam_list_bath);
		fprintf(stderr,"dbg2       beam_list_back: %d\n",
						ibeam_list_back);
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
		exit(MB_ERROR_NO_ERROR);
		}

	/* initialize reading the multibeam file */
	if ((status = mb_read_init(
		verbose,file,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_back,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* set the beam to be output (default = centerbeam) */
	if (ibeam_list_bath < 0 && beams_bath > 0)
		beam_list_bath = beams_bath/2;
	else if (ibeam_list_bath >= beams_bath)
		{
		fprintf(stderr,"\nRequested bathymetry beam %d does not exist \n- there are only %d bathymetry beams available in format %d)\n",
			ibeam_list_bath,beams_bath,format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_FAILURE);
		}
	else if (ibeam_list_bath >= 0)
		beam_list_bath = ibeam_list_bath;
	if (ibeam_list_back < 0 && beams_back > 0)
		ibeam_list_back = beams_back/2;
	else if (ibeam_list_back >= beams_back)
		{
		fprintf(stderr,"\nRequested backscatter beam %d does not exist \n- there are only %d backscatter beams available in format %d)\n",
			ibeam_list_back,beams_back,format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_FAILURE);
		}
	else if (ibeam_list_back >= 0)
		beam_list_back = ibeam_list_back;

	/* print debug statements */
	if (verbose >= 2 && dumpmode == MBLIST_MODE_LIST)
		{
		fprintf(stderr,"\ndbg2  Beams set for output in <%s>\n",
			program_name);
		fprintf(stderr,"dbg2       status:         %d\n",status);
		fprintf(stderr,"dbg2       error:          %d\n",error);
		fprintf(stderr,"dbg2       beam_list_bath: %d\n",
					beam_list_bath);
		fprintf(stderr,"dbg2       beam_list_back: %d\n",
					beam_list_back);
		}

	/* print debug statements */
	if (verbose >= 2 && dumpmode == MBLIST_MODE_DUMP_BATHYMETRY)
		{
		fprintf(stderr,"\ndbg2  All nonzero bathymetry beams will be output as \n(lon lat depth) triples in <%s>\n",
			program_name);
		}

	/* print debug statements */
	if (verbose >= 2 && dumpmode == MBLIST_MODE_DUMP_BACKSCATTER)
		{
		fprintf(stderr,"\ndbg2  All nonzero backscatter beams will be output as \n(lon lat backscatter) triples in <%s>\n",
			program_name);
		}

	/* allocate memory for data arrays */
	if (dumpmode == MBLIST_MODE_LIST)
		{
		if ((bath = (int *) 
			calloc(beams_bath,sizeof(int))) == NULL) 
			error = MB_ERROR_MEMORY_FAIL;
		if ((bathdist = (int *) 
			calloc(beams_bath,sizeof(int))) == NULL)
			error = MB_ERROR_MEMORY_FAIL;
		if ((back = (int *) 
			calloc(beams_back,sizeof(int))) == NULL) 
			error = MB_ERROR_MEMORY_FAIL;
		if ((backdist = (int *) 
			calloc(beams_back,sizeof(int))) == NULL)
			error = MB_ERROR_MEMORY_FAIL;
		}
	else
		{
		if ((depth = (double *) 
			calloc(beams_bath,sizeof(double))) == NULL) 
			error = MB_ERROR_MEMORY_FAIL;
		if ((depthlon = (double *) 
			calloc(beams_bath,sizeof(double))) == NULL)
			error = MB_ERROR_MEMORY_FAIL;
		if ((depthlat = (double *) 
			calloc(beams_bath,sizeof(double))) == NULL)
			error = MB_ERROR_MEMORY_FAIL;
		if ((scatter = (double *) 
			calloc(beams_bath,sizeof(double))) == NULL) 
			error = MB_ERROR_MEMORY_FAIL;
		if ((scatterlon = (double *) 
			calloc(beams_bath,sizeof(double))) == NULL)
			error = MB_ERROR_MEMORY_FAIL;
		if ((scatterlat = (double *) 
			calloc(beams_bath,sizeof(double))) == NULL)
			error = MB_ERROR_MEMORY_FAIL;
		}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
			message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* read and print data in standard mode */
	if (dumpmode == MBLIST_MODE_LIST)
	{
	distance_total = 0.0;
	nread = 0;
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read a ping of data */
		status = mb_get(verbose,mbio_ptr,&kind,&pings,time_i,&time_d,
			&navlon,&navlat,&speed,&heading,&distance,
			&beams_bath,bath,bathdist,
			&beams_back,back,backdist,
			comment,&error);

		/* time gaps are not a problem here */
		if (error == MB_ERROR_TIME_GAP)
			{
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}

		/* increment counter and set cumulative distance */
		if (error <= MB_ERROR_NO_ERROR 
			&& error != MB_ERROR_COMMENT)
			{
			nread++;
			distance_total += distance;
			}

		/* reset beams to be listed if beam numbers
			are variable and center beam specified */
		if (ibeam_list_bath < 0 
			&& variable_beams_table[format] == MB_YES)
			beam_list_bath = beams_bath/2;
		if (ibeam_list_back < 0 
			&& variable_beams_table[format] == MB_YES)
			beam_list_back = beams_back/2;

		/* zero pings are no good */
		beam_status = status;
		if (beams_bath > 0 && beam_list_bath >= 0)
			{
			if (bath[beam_list_bath] <= 0)
				beam_status = MB_FAILURE;
			}
		if (beams_back > 0 && beam_list_back >= 0)
			{
			if (back[beam_list_back] <= 0)
				beam_status = MB_FAILURE;
			}

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",kind);
			fprintf(stderr,"dbg2       beams_bath:     %d\n",
					beams_bath);
			if (beams_bath > 0)
				{
				fprintf(stderr,"dbg2       beam_list_bath: %d\n",
						beam_list_bath);
				fprintf(stderr,"dbg2       bath:           %d\n",
						bath[beam_list_bath]);
				}
			fprintf(stderr,"dbg2       beams_back:     %d\n",
					beams_back);
			if (beams_back > 0)
				{
				fprintf(stderr,"dbg2       beam_list_back: %d\n",
						beam_list_back);
				fprintf(stderr,"dbg2       back:           %d\n",
						back[beam_list_back]);
				}
			fprintf(stderr,"dbg2       kind:           %d\n",kind);
			fprintf(stderr,"dbg2       error:          %d\n",error);
			fprintf(stderr,"dbg2       beam status:    %d\n",						beam_status);
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

		/* print out good pings */
		if (error == MB_ERROR_NO_ERROR 
			&& beam_status == MB_SUCCESS)
		{
		for (i=0; i<n_list; i++) 
			{
			switch (list[i]) 
				{
				case 'X': /* longitude */
					printf("%11.6f",navlon);
					break;
				case 'Y': /* latitude */
					printf("%10.6f",navlat);	
					break;
				case 'L': /* along-track dist. */
					printf("%7.3f",distance_total);
					break;
				case 'Z': /* depth */
					printf("%6d",-bath[beam_list_bath]);
					break;
				case 'z': /* depth */
					printf("%6d",bath[beam_list_bath]);
					break;
				case 'B': /* backscatter */
				case 'b':
					printf("%6d",back[beam_list_back]);
					break;
				case 'D': /* bathymetry across-track dist. */
					printf("%5d",bathdist[beam_list_bath]);
					break;
				case 'd': /* backscatter across-track dist. */
					printf("%5d",backdist[beam_list_back]);
					break;
				case 'H': /* heading */
					printf("%5.1f",heading);
					break;
				case 'S': /* speed */
					printf("%5.2f",speed);
					break;
				case 'T': /* time string */
					printf(
					"%.4d/%.2d/%.2d/%.2d/%.2d/%.2d",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5]);
					break;
				case 't': /* time string */
					printf(
					"%.4d %.2d %.2d %.2d %.2d %.2d",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5]);
					break;
				case 'M': /* MBIO time in minutes since 1/1/81 00:00:00 */
					printf("%.3f",time_d);
					break;
				case 'm': /* time in minutes since first record */
					if (first_m == MB_YES)
						{
						time_d_ref = time_d;
						first_m = MB_NO;
						}
					printf("%.3f",time_d - time_d_ref);
					break;
				case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
					time_tm.tm_year = time_i[0] - 1900;
					time_tm.tm_mon = time_i[1] - 1;
					time_tm.tm_mday = time_i[2];
					time_tm.tm_hour = time_i[3];
					time_tm.tm_min = time_i[4];
					time_tm.tm_sec = time_i[5];
					time_u = timegm(time_tm);
					printf("%d",time_u);
					break;
				case 'u': /* time in seconds since first record */
					time_tm.tm_year = time_i[0] - 1900;
					time_tm.tm_mon = time_i[1] - 1;
					time_tm.tm_mday = time_i[2];
					time_tm.tm_hour = time_i[3];
					time_tm.tm_min = time_i[4];
					time_tm.tm_sec = time_i[5];
					time_u = timegm(time_tm);
					if (first_u == MB_YES)
						{
						time_u_ref = time_u;
						first_u = MB_NO;
						}
					printf("%d",time_u - time_u_ref);
					break;
				case 'J': /* time string */
					mb_get_jtime(verbose,time_i,time_j);
					printf(
					"%.4d %.3d %.2d %.2d %.2d",
					time_j[0],time_j[1],
					time_i[3],time_i[4],time_i[5]);
					break;
				case 'N': /* ping counter */
					printf("%6d",nread);
					break;
				case 'A': /* Seafloor crosstrack slope */
					ns = 0;
					sx = 0.0;
					sy = 0.0;
					sxx = 0.0;
					sxy = 0.0;
					for (j=0;j<beams_bath;j++)
					  if (bath[j] > 0)
					    {
					    sx += bathdist[j];
					    sy += bath[j];
					    sxx += bathdist[j]*bathdist[j];
					    sxy += bathdist[j]*bath[j];
					    ns++;
					    }
					if (ns > 0)
					  {
					  delta = ns*sxx - sx*sx;
					  a = (sxx*sy - sx*sxy)/delta;
					  b = (ns*sxy - sx*sy)/delta;
					  slope = atan(b)/DTR;
					  }
					else
					  slope = 0.0;
					printf("%.4f",slope);
					break;
				default:
					printf("<Invalid Option: %c>",
						list[i]);
					break;
				}
				if (i<(n_list-1)) printf ("\t");
				else printf ("\n");
			}
		}
		}
	}

	/* read and print data in dump lon lat depth mode */
	else
	{
	distance_total = 0.0;
	nread = 0;
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read a ping of data */
		status = mb_read(verbose,mbio_ptr,&kind,&pings,time_i,&time_d,
			&navlon,&navlat,&speed,&heading,&distance,
			&beams_bath,depth,depthlon,depthlat,
			&beams_back,scatter,scatterlon,scatterlat,
			comment,&error);

		/* time gaps are not a problem here */
		if (error == MB_ERROR_TIME_GAP)
			{
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}

		/* increment counter and set cumulative distance */
		if (error <= MB_ERROR_NO_ERROR 
			&& error != MB_ERROR_COMMENT)
			{
			nread++;
			distance_total += distance;
			}

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",kind);
			fprintf(stderr,"dbg2       beams_bath:     %d\n",
					beams_bath);
			fprintf(stderr,"dbg2       beams_back:     %d\n",
					beams_back);
			fprintf(stderr,"dbg2       kind:           %d\n",kind);
			fprintf(stderr,"dbg2       error:          %d\n",error);
			fprintf(stderr,"dbg2       beam status:    %d\n",						beam_status);
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

		/* print out good pings */
		if (error == MB_ERROR_NO_ERROR 
			&& dumpmode == MBLIST_MODE_DUMP_BATHYMETRY)
			{
			for (i=0;i<beams_bath;i++)
				if (depth[i] > 0.0)
					printf("%f\t%f\t%f\n",
						depthlon[i],
						depthlat[i],
						depth[i]);
			}
		if (error == MB_ERROR_NO_ERROR 
			&& dumpmode == MBLIST_MODE_DUMP_BACKSCATTER)
			{
			for (i=0;i<beams_back;i++)
				if (scatter[i] > 0.0)
					printf("%f\t%f\t%f\n",
						scatterlon[i],
						scatterlat[i],
						scatter[i]);
			}
		}
	}

	/* close the multibeam file */
	status = mb_close(verbose,mbio_ptr,&error);

	/* deallocate memory used for data arrays */
	free(bath);
	free(bathdist);
	free(back);
	free(backdist);
	free(depth);
	free(depthlon);
	free(depthlat);
	free(scatter);
	free(scatterlon);
	free(scatterlat);

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
	exit(status);
}

/*
err_exit(message)
char message[];
{
	fprintf(stderr,"%s: %s\n",progname,message);
	fprintf(stderr,"Usage (defaults in parentheses):\n");
	fprintf(stderr,"-Iinput_file (stdin)\n");
	fprintf(stderr,"-Finput_file_format (7)\n");
	fprintf(stderr,"-Sspeed_min, in km/hr (0.0)\n");
	fprintf(stderr,"-Ttime_gap, in minutes (1.0)\n");
	fprintf(stderr,"-Ppings, pings to be averaged (1)\n");
	fprintf(stderr,"-Rregion as W/E/S/N (-360/360/-90/90)\n");
	fprintf(stderr,"-Bbegin_time (1970/01/01/00/00/00)\n");
	fprintf(stderr,"-Eend_time   (1999/01/01/00/00/00)\n");
	fprintf(stderr,"-Llon_flip (-1)\n");
	fprintf(stderr,"-#beam_to_be_listed (centerbeam)\n");
	fprintf(stderr,"-Ooutput_format (YXLZ)\n");
	fprintf(stderr,"output_format is a string composed of :\n");
	fprintf(stderr,"\tY for latitude\n");
	fprintf(stderr,"\tX for longitude\n");
	fprintf(stderr,"\tL for cumulative along-track distance\n");
	fprintf(stderr,"\tD for across-track distance\n");
	fprintf(stderr,"\tT for a time string yyyy/mm/dd/hh/mm/ss\n");
	fprintf(stderr,"\tt for a time string yyyy mm dd hh mm ss\n");
	fprintf(stderr,"\tJ for a time string yyyy jjj hh mm ss\n");
	fprintf(stderr,"\tH for heading\n");
	fprintf(stderr,"\tS for speed\n");
	fprintf(stderr,"\tZ for depth\n");
	exit (-1);
}
*/
