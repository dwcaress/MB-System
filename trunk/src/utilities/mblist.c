/*--------------------------------------------------------------------
 *    The MB-system:	mblist.c	2/1/93
 *    $Id: mblist.c,v 4.3 1994-06-01 21:00:55 caress Exp $
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
 * Revision 4.2  1994/04/29  18:01:20  caress
 * Added output option "j" for time string in form: year jday daymin sec
 *
 * Revision 4.1  1994/04/12  18:50:33  caress
 * Added #ifdef IRIX statements for compatibility with
 * SGI machines.  The system routine timegm does not exist
 * on SGI's; mktime must be used instead.
 *
 * Revision 4.0  1994/03/06  00:13:22  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 3.6  1993/09/11  15:44:30  caress
 * Fixed bug so that mblist no longer checks for a valid depth
 * even if it is not outputting depth.
 *
 * Revision 3.5  1993/06/13  07:47:20  caress
 * Added new time output options "m" and "u" and changed
 * old options "U" to "M" and "u" to "U
 * Now time options are:
 *   T  for a time string (yyyy/mm/dd/hh/mm/ss)
 *   t  for a time string (yyyy mm dd hh mm ss)
 *   J  for a time string (yyyy jd hh mm ss)
 *   M  for mbio time in minutes since 1/1/81 00:00:00
 *   m  for time in minutes since first record
 *   U  for unix time in seconds since 1/1/70 00:00:00
 *   u  for time in seconds since first record
 *
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
#define	MBLIST_MODE_DUMP_LL_BATHYMETRY	2
#define	MBLIST_MODE_DUMP_LL_TOPOGRAPHY	3
#define	MBLIST_MODE_DUMP_LL_AMPLITUDE	4
#define	MBLIST_MODE_DUMP_LL_SIDESCAN	5
#define	MBLIST_MODE_DUMP_CT_BATHYMETRY	-2
#define	MBLIST_MODE_DUMP_CT_TOPOGRAPHY	-3
#define	MBLIST_MODE_DUMP_CT_AMPLITUDE	-4
#define	MBLIST_MODE_DUMP_CT_SIDESCAN	-5

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mblist.c,v 4.3 1994-06-01 21:00:55 caress Exp $";
	static char program_name[] = "MBLIST";
	static char help_message[] =  "MBLIST prints the specified contents of a multibeam data \nfile to stdout. The form of the output is quite flexible; \nMBLIST is tailored to produce ascii files in spreadsheet \nstyle with data columns separated by tabs.";
	static char usage_message[] = "mblist [-Fformat -Rw/e/s/n -Ppings -Sspeed -Llonflip\n	-Byr/mo/da/hr/mn/sc -Eyr/mo/da/hr/mn/sc -V -H -Ifile\n	-Lbath_beam -Mamp_beam -Nss_pixel -Ooptions -Ddumpmode]";
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

	/* output format list controls */
	int	dumpmode;
	char	list[MAX_OPTIONS];
	int	n_list;
	int	ibeam_list_bath = -1;
	int	ibeam_list_amp = -1;
	int	ipixel_list_ss = -1;
	int	beam_list_bath = -1;
	int	beam_list_amp = -1;
	int	pixel_list_ss = -1;
	double	distance_total;
	int	nread;
	int	beam_status = MB_SUCCESS;
	int	time_j[4];
	int	bathcheck = 0;
	int	ampcheck = 0;
	int	sscheck = 0;

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
	int	*bathacrosstrack;
	int	*bathalongtrack;
	int	*amp;
	int	*ss;
	int	*ssacrosstrack;
	int	*ssalongtrack;
	double	*dbath;
	double	*dbathlon;
	double	*dbathlat;
	double	*damp;
	double	*dss;
	double	*dsslon;
	double	*dsslat;
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
		case 'B':
		case 'b':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%d", &dumpmode);
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
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
		case 'M':
		case 'm':
			sscanf (optarg,"%d", &ibeam_list_bath);
			flag++;
			break;
		case 'N':
			sscanf (optarg,"%d", &ibeam_list_amp);
			flag++;
			break;
		case 'n':
			sscanf (optarg,"%d", &ipixel_list_ss);
			flag++;
			break;
		case 'O':
		case 'o':
			for(j=0,n_list=0;j<strlen(optarg);j++,n_list++)
				if (n_list<MAX_OPTIONS)
					list[n_list] = optarg[j];
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf (optarg,"%d", &pings);
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
		fprintf(stderr,"dbg2       beam_list_amp:  %d\n",
						ibeam_list_amp);
		fprintf(stderr,"dbg2       pixel_list_ss:   %d\n",
						ipixel_list_ss);
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
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* check which data types need to be checked */
	for (i=0; i<n_list; i++) 
		{
		if (list[i] == 'Z' || list[i] == 'z')
			bathcheck = 1;
		if (list[i] == 'B')
			ampcheck = 1;
		if (list[i] == 'b')
			sscheck = 1;
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
	if (ibeam_list_amp < 0 && beams_amp > 0)
		ibeam_list_amp = beams_amp/2;
	else if (ibeam_list_amp >= beams_amp)
		{
		fprintf(stderr,"\nRequested amplitude beam %d does not exist \n- there are only %d amplitude beams available in format %d)\n",
			ibeam_list_amp,beams_amp,format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_FAILURE);
		}
	else if (ibeam_list_amp >= 0)
		beam_list_amp = ibeam_list_amp;
	if (ipixel_list_ss < 0 && pixels_ss > 0)
		ipixel_list_ss = pixels_ss/2;
	else if (ipixel_list_ss >= pixels_ss)
		{
		fprintf(stderr,"\nRequested sidescan beam %d does not exist \n- there are only %d sidescan beams available in format %d)\n",
			ipixel_list_ss,pixels_ss,format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_FAILURE);
		}
	else if (ipixel_list_ss >= 0)
		pixel_list_ss = ipixel_list_ss;

	/* print debug statements */
	if (verbose >= 2 && dumpmode == MBLIST_MODE_LIST)
		{
		fprintf(stderr,"\ndbg2  Beams set for output in <%s>\n",
			program_name);
		fprintf(stderr,"dbg2       status:         %d\n",status);
		fprintf(stderr,"dbg2       error:          %d\n",error);
		fprintf(stderr,"dbg2       beam_list_bath: %d\n",
					beam_list_bath);
		fprintf(stderr,"dbg2       beam_list_amp:  %d\n",
					beam_list_amp);
		fprintf(stderr,"dbg2       pixel_list_ss:   %d\n",
					pixel_list_ss);
		}

	/* print debug statements */
	if (verbose >= 2 && dumpmode == MBLIST_MODE_DUMP_LL_BATHYMETRY)
		{
		fprintf(stderr,"\ndbg2  All nonzero bathymetry beams will be output as \n(lon lat depth) triples in <%s>\n",
			program_name);
		}
	/* print debug statements */
	if (verbose >= 2 && dumpmode == MBLIST_MODE_DUMP_LL_TOPOGRAPHY)
		{
		fprintf(stderr,"\ndbg2  All nonzero bathymetry beams will be output as \n(lon lat topography) triples in <%s>\n",
			program_name);
		}

	/* print debug statements */
	if (verbose >= 2 && dumpmode == MBLIST_MODE_DUMP_LL_AMPLITUDE)
		{
		fprintf(stderr,"\ndbg2  All nonzero amplitude beams will be output as \n(lon lat amplitude) triples in <%s>\n",
			program_name);
		}

	/* print debug statements */
	if (verbose >= 2 && dumpmode == MBLIST_MODE_DUMP_LL_SIDESCAN)
		{
		fprintf(stderr,"\ndbg2  All nonzero sidescan pixels will be output as \n(lon lat sidescan) triples in <%s>\n",
			program_name);
		}

	/* print debug statements */
	if (verbose >= 2 && dumpmode == MBLIST_MODE_DUMP_CT_BATHYMETRY)
		{
		fprintf(stderr,"\ndbg2  All nonzero bathymetry beams will be output as \n(acrosstrack alongtrackdepth) triples in <%s>\n",
			program_name);
		}
	/* print debug statements */
	if (verbose >= 2 && dumpmode == MBLIST_MODE_DUMP_CT_TOPOGRAPHY)
		{
		fprintf(stderr,"\ndbg2  All nonzero bathymetry beams will be output as \n(acrosstrack alongtrack topography) triples in <%s>\n",
			program_name);
		}

	/* print debug statements */
	if (verbose >= 2 && dumpmode == MBLIST_MODE_DUMP_CT_AMPLITUDE)
		{
		fprintf(stderr,"\ndbg2  All nonzero amplitude beams will be output as \n(acrosstrack alongtrack amplitude) triples in <%s>\n",
			program_name);
		}

	/* print debug statements */
	if (verbose >= 2 && dumpmode == MBLIST_MODE_DUMP_CT_SIDESCAN)
		{
		fprintf(stderr,"\ndbg2  All nonzero sidescan pixels will be output as \n(acrosstrack alongtrack sidescan) triples in <%s>\n",
			program_name);
		}

	/* allocate memory for data arrays */
	if (dumpmode <= MBLIST_MODE_LIST)
		{
		status = mb_malloc(verbose,beams_bath*sizeof(int),&bath,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(int),
				&bathacrosstrack,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(int),
				&bathalongtrack,&error);
		status = mb_malloc(verbose,beams_amp*sizeof(int),&amp,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(int),&ss,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(int),&ssacrosstrack,
				&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(int),&ssalongtrack,
				&error);
		}
	else
		{
		status = mb_malloc(verbose,beams_bath*sizeof(double),
				&dbath,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
				&dbathlon,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
				&dbathlat,&error);
		status = mb_malloc(verbose,beams_amp*sizeof(double),
				&damp,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
				&dss,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
				&dsslon,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
				&dsslat,&error);
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
			&beams_bath,&beams_amp,&pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
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
		if (ibeam_list_amp < 0 
			&& variable_beams_table[format] == MB_YES)
			beam_list_amp = beams_amp/2;
		if (ipixel_list_ss < 0 
			&& variable_beams_table[format] == MB_YES)
			pixel_list_ss = pixels_ss/2;

		/* zero pings are no good if bathymetry, amplitude, or
			sidescan is being output */
		beam_status = status;
		if (bathcheck && beams_bath > 0 && beam_list_bath >= 0)
			{
			if (bath[beam_list_bath] <= 0)
				beam_status = MB_FAILURE;
			}
		if (ampcheck && beams_amp > 0 && beam_list_amp >= 0)
			{
			if (amp[beam_list_amp] <= 0)
				beam_status = MB_FAILURE;
			}
		if (sscheck && pixels_ss > 0 && pixel_list_ss >= 0)
			{
			if (ss[pixel_list_ss] <= 0)
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
			fprintf(stderr,"dbg2       beams_amp:      %d\n",
					beams_amp);
			if (beams_amp > 0)
				{
				fprintf(stderr,"dbg2       beam_list_amp:  %d\n",
						beam_list_amp);
				fprintf(stderr,"dbg2       amp:            %d\n",
						amp[beam_list_amp]);
				}
			fprintf(stderr,"dbg2       pixels_ss:      %d\n",
					pixels_ss);
			if (pixels_ss > 0)
				{
				fprintf(stderr,"dbg2       pixel_list_ss:   %d\n",
						pixel_list_ss);
				fprintf(stderr,"dbg2       ss:             %d\n",
						ss[pixel_list_ss]);
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
				case 'A': /* Seafloor crosstrack slope */
				case 'a':
					ns = 0;
					sx = 0.0;
					sy = 0.0;
					sxx = 0.0;
					sxy = 0.0;
					for (j=0;j<beams_bath;j++)
					  if (bath[j] > 0)
					    {
					    sx += bathacrosstrack[j];
					    sy += bath[j];
					    sxx += bathacrosstrack[j]
						*bathacrosstrack[j];
					    sxy += bathacrosstrack[j]*bath[j];
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
				case 'B': /* amplitude */
					printf("%6d",amp[beam_list_amp]);
					break;
				case 'b': /* sidescan */
					printf("%6d",ss[pixel_list_ss]);
					break;
				case 'D': /* bathymetry acrosstrack dist. */
					printf("%5d",
					bathacrosstrack[beam_list_bath]);
					break;
				case 'd': /* sidescan acrosstrack dist. */
					printf("%5d",
					ssacrosstrack[pixel_list_ss]);
					break;
				case 'E': /* bathymetry alongtrack dist. */
					printf("%5d",
					bathacrosstrack[beam_list_bath]);
					break;
				case 'e': /* sidescan alongtrack dist. */
					printf("%5d",
					ssacrosstrack[pixel_list_ss]);
					break;
				case 'H': /* heading */
					printf("%5.1f",heading);
					break;
				case 'J': /* time string */
					mb_get_jtime(verbose,time_i,time_j);
					printf(
					"%.4d %.3d %.2d %.2d %.2d",
					time_j[0],time_j[1],
					time_i[3],time_i[4],time_i[5]);
					break;
				case 'j': /* time string */
					mb_get_jtime(verbose,time_i,time_j);
					printf(
					"%.4d %.3d %.4d %.2d",
					time_j[0],time_j[1],
					time_j[2],time_j[3]);
					break;
				case 'L': /* along-track dist. */
					printf("%7.3f",distance_total);
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
				case 'N': /* ping counter */
					printf("%6d",nread);
					break;
				case 'S': /* speed */
					printf("%5.2f",speed);
					break;
				case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
					printf(
					"%.4d/%.2d/%.2d/%.2d/%.2d/%.2d",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5]);
					break;
				case 't': /* yyyy mm dd hh mm ss time string */
					printf(
					"%.4d %.2d %.2d %.2d %.2d %.2d",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5]);
					break;
				case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
					time_tm.tm_year = time_i[0] - 1900;
					time_tm.tm_mon = time_i[1] - 1;
					time_tm.tm_mday = time_i[2];
					time_tm.tm_hour = time_i[3];
					time_tm.tm_min = time_i[4];
					time_tm.tm_sec = time_i[5];
#ifdef IRIX
					time_u = mktime(&time_tm);
#else
					time_u = timegm(time_tm);
#endif
					printf("%d",time_u);
					break;
				case 'u': /* time in seconds since first record */
					time_tm.tm_year = time_i[0] - 1900;
					time_tm.tm_mon = time_i[1] - 1;
					time_tm.tm_mday = time_i[2];
					time_tm.tm_hour = time_i[3];
					time_tm.tm_min = time_i[4];
					time_tm.tm_sec = time_i[5];
#ifdef IRIX
					time_u = mktime(&time_tm);
#else
					time_u = timegm(time_tm);
#endif
					if (first_u == MB_YES)
						{
						time_u_ref = time_u;
						first_u = MB_NO;
						}
					printf("%d",time_u - time_u_ref);
					break;
				case 'X': /* longitude */
					printf("%11.6f",navlon);
					break;
				case 'Y': /* latitude */
					printf("%10.6f",navlat);	
					break;
				case 'Z': /* depth */
					printf("%6d",-bath[beam_list_bath]);
					break;
				case 'z': /* depth */
					printf("%6d",bath[beam_list_bath]);
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
	else if (dumpmode > MBLIST_MODE_LIST)
	{
	distance_total = 0.0;
	nread = 0;
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read a ping of data */
		status = mb_read(verbose,mbio_ptr,&kind,&pings,time_i,&time_d,
			&navlon,&navlat,&speed,&heading,&distance,
			&beams_bath,&beams_amp,&pixels_ss,
			dbath,damp,dbathlon,dbathlat,
			dss,dsslon,dsslat,
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
			fprintf(stderr,"dbg2       beams_amp:      %d\n",
					beams_amp);
			fprintf(stderr,"dbg2       pixels_ss:      %d\n",
					pixels_ss);
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
			&& dumpmode == MBLIST_MODE_DUMP_LL_BATHYMETRY)
			{
			for (i=0;i<beams_bath;i++)
				if (dbath[i] > 0.0)
					printf("%f\t%f\t%f\n",
						dbathlon[i],
						dbathlat[i],
						dbath[i]);
			}
		if (error == MB_ERROR_NO_ERROR 
			&& dumpmode == MBLIST_MODE_DUMP_LL_TOPOGRAPHY)
			{
			for (i=0;i<beams_bath;i++)
				if (dbath[i] > 0.0)
					printf("%f\t%f\t%f\n",
						dbathlon[i],
						dbathlat[i],
						-dbath[i]);
			}
		if (error == MB_ERROR_NO_ERROR 
			&& dumpmode == MBLIST_MODE_DUMP_LL_AMPLITUDE)
			{
			for (i=0;i<beams_amp;i++)
				if (damp[i] > 0.0)
					printf("%f\t%f\t%f\n",
						dbathlon[i],
						dbathlat[i],
						damp[i]);
			}
		if (error == MB_ERROR_NO_ERROR 
			&& dumpmode == MBLIST_MODE_DUMP_LL_SIDESCAN)
			{
			for (i=0;i<pixels_ss;i++)
				if (dss[i] > 0.0)
					printf("%f\t%f\t%f\n",
						dsslon[i],
						dsslat[i],
						dss[i]);
			}
		}
	}

	/* read and print data in dump acrosstrack alongtrack depth mode */
	else
	{
	distance_total = 0.0;
	nread = 0;
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read a ping of data */
		status = mb_get(verbose,mbio_ptr,&kind,&pings,time_i,&time_d,
			&navlon,&navlat,&speed,&heading,&distance,
			&beams_bath,&beams_amp,&pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
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
			fprintf(stderr,"dbg2       beams_amp:      %d\n",
					beams_amp);
			fprintf(stderr,"dbg2       pixels_ss:      %d\n",
					pixels_ss);
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
			&& dumpmode == MBLIST_MODE_DUMP_CT_BATHYMETRY)
			{
			for (i=0;i<beams_bath;i++)
				if (bath[i] > 0)
					printf("%d\t%d\t%d\n",
						bathacrosstrack[i],
						bathalongtrack[i],
						bath[i]);
			}
		if (error == MB_ERROR_NO_ERROR 
			&& dumpmode == MBLIST_MODE_DUMP_CT_TOPOGRAPHY)
			{
			for (i=0;i<beams_bath;i++)
				if (bath[i] > 0)
					printf("%d\t%d\t%d\n",
						bathacrosstrack[i],
						bathalongtrack[i],
						-bath[i]);
			}
		if (error == MB_ERROR_NO_ERROR 
			&& dumpmode == MBLIST_MODE_DUMP_CT_AMPLITUDE)
			{
			for (i=0;i<beams_amp;i++)
				if (amp[i] > 0)
					printf("%d\t%d\t%d\n",
						bathacrosstrack[i],
						bathalongtrack[i],
						amp[i]);
			}
		if (error == MB_ERROR_NO_ERROR 
			&& dumpmode == MBLIST_MODE_DUMP_CT_SIDESCAN)
			{
			for (i=0;i<pixels_ss;i++)
				if (ss[i] > 0)
					printf("%d\t%d\t%d\n",
						ssacrosstrack[i],
						ssalongtrack[i],
						ss[i]);
			}
		}
	}

	/* close the multibeam file */
	status = mb_close(verbose,mbio_ptr,&error);

	/* deallocate memory used for data arrays */
	mb_free(verbose,bath,&error); 
	mb_free(verbose,bathacrosstrack,&error); 
	mb_free(verbose,bathalongtrack,&error); 
	mb_free(verbose,amp,&error); 
	mb_free(verbose,ss,&error); 
	mb_free(verbose,ssacrosstrack,&error); 
	mb_free(verbose,ssalongtrack,&error); 
	mb_free(verbose,dbath,&error); 
	mb_free(verbose,dbathlon,&error); 
	mb_free(verbose,dbathlat,&error); 
	mb_free(verbose,damp,&error); 
	mb_free(verbose,dss,&error); 
	mb_free(verbose,dsslon,&error); 
	mb_free(verbose,dsslat,&error); 

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
