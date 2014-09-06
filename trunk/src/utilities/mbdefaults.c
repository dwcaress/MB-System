/*--------------------------------------------------------------------
 *    The MB-system:	mbdefaults.c	1/23/93
 *	$Id$
 *
 *    Copyright (c) 1993-2014 by
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
 * MBDEFAULTS sets and retrieves the default MBIO control parameters
 * stored in the file ~/.mbio_defaults.  Only the parameters specified
 * by command line arguments will be changed; if no ~/.mbio_defaults
 * file exists one will be created.
 *
 * Author:	D. W. Caress
 * Date:	January 23, 1993
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_define.h"

/* colortable view mode defines */
#define	MBV_COLORTABLE_HAXBY		0
#define	MBV_COLORTABLE_BRIGHT		1
#define	MBV_COLORTABLE_MUTED		2
#define	MBV_COLORTABLE_GRAY		3
#define	MBV_COLORTABLE_FLAT		4
#define	MBV_COLORTABLE_SEALEVEL1	5
#define	MBV_COLORTABLE_SEALEVEL2	6

/* colortable view mode defines */
#define	MBV_COLORTABLE_NORMAL		0
#define	MBV_COLORTABLE_REVERSED		1

/* shade view mode defines */
#define	MBV_SHADE_VIEW_NONE		0
#define	MBV_SHADE_VIEW_ILLUMINATION	1
#define	MBV_SHADE_VIEW_SLOPE		2
#define	MBV_SHADE_VIEW_OVERLAY		3

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "MBDEFAULTS";
	char help_message[] = "MBDEFAULTS sets and retrieves the /default MBIO control \nparameters stored in the file ~/.mbio_defaults. \nOnly the parameters specified by command line \narguments will be changed; if no ~/.mbio_defaults \nfile exists one will be created.";
	char usage_message[] = "mbdefaults [-Bfileiobuffer -Dpsdisplay -Ffbtversion -Iimagedisplay -Llonflip\n\t-Mmbviewsettings\n\t-Ttimegap -Wproject -V -H]";
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	status;
	int	error = MB_ERROR_NO_ERROR;
	int	verbose = 0;
	int	help = 0;
	int	flag = 0;
	FILE	*fp;
	char	file[MB_PATH_MAXLINE];
	char	psdisplay[MB_PATH_MAXLINE];
	char	imgdisplay[MB_PATH_MAXLINE];
	char	mbproject[MB_PATH_MAXLINE];
	char	argstring[MB_PATH_MAXLINE];
	int	fbtversion = 3;
	int	uselockfiles = 1;
	int	fileiobuffer = 0;
	char	*HOME = "HOME";
	int	primary_colortable;
	int	primary_colortable_mode;
	int	primary_shade_mode;
	int	slope_colortable;
	int	slope_colortable_mode;
	int	secondary_colortable;
	int	secondary_colortable_mode;
	double	illuminate_magnitude;
	double	illuminate_elevation;
	double	illuminate_azimuth;
	double	slope_magnitude;
	char	*getenv();

	/* MBIO control parameters */
	int format;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;
	
	int	n;

	/* get current default mbio values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* now get current mb environment values */
	status = mb_env(verbose,psdisplay,imgdisplay,mbproject);

	/* now get current mbview default display values */
	status = mb_mbview_defaults(verbose,
					&primary_colortable,
					&primary_colortable_mode,
					&primary_shade_mode,
					&slope_colortable,
					&slope_colortable_mode,
					&secondary_colortable,
					&secondary_colortable_mode,
					&illuminate_magnitude,
					&illuminate_elevation,
					&illuminate_azimuth,
					&slope_magnitude);

	/* now get current fbtversion value */
	status = mb_fbtversion(verbose,&fbtversion);

	/* now get current uselockfiles value */
	status = mb_uselockfiles(verbose,&uselockfiles);

	/* now get current fileio buffering values */
	status = mb_fileiobuffer(verbose,&fileiobuffer);

	/* process argument list */
	while ((c = getopt(argc, argv, "B:b:D:d:F:f:HhI:i:L:l:M:m:T:t:U:u:VvW:w:")) != -1)
	  switch (c)
		{
		case 'B':
		case 'b':
			sscanf (optarg,"%d",&fileiobuffer);
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%s",psdisplay);
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%s",argstring);
			if (strncmp(argstring,"new",3) == 0 || strncmp(argstring,"NEW",3) == 0)
				fbtversion = 3;
			else if (strncmp(argstring,"old",2) == 0 || strncmp(argstring,"OLD",2) == 0)
				fbtversion = 2;
			else if (strncmp(argstring,"2",1) == 0)
				fbtversion = 2;
			else if (strncmp(argstring,"3",1) == 0)
				fbtversion = 3;
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", imgdisplay);
			flag++;
			break;
		case 'H':
		case 'h':
			help++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'M':
		case 'm':
			/* default primary colortable and modes */
			if (optarg[0] == 'P' || optarg[0] == 'p')
				n = sscanf (&optarg[1],"%d/%d/%d", &primary_colortable, &primary_colortable_mode, &primary_shade_mode);
				
			/* default slope colortable and mode */
			else if (optarg[0] == 'G' ||optarg[0] == 'g')
				n = sscanf (&optarg[1],"%d/%d", &slope_colortable, &slope_colortable_mode);
				
			/* default overlay colortable and mode */
			else if (optarg[0] == 'O' ||optarg[0] == 'o')
				n = sscanf (&optarg[1],"%d/%d", &secondary_colortable, &secondary_colortable_mode);
				
			/* default illumination parameters */
			else if (optarg[0] == 'I' ||optarg[0] == 'i')
				n = sscanf (&optarg[1],"%lf/%lf/%lf", &illuminate_magnitude, &illuminate_elevation, &illuminate_azimuth);
				
			/* default slope shading magnitude */
			else if (optarg[0] == 'S' ||optarg[0] == 'S')
				n = sscanf (&optarg[1],"%lf", &slope_magnitude);

			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf", &timegap);
			flag++;
			break;
		case 'U':
		case 'u':
			sscanf (optarg,"%s",argstring);
			if (strncmp(argstring,"yes",3) == 0 || strncmp(argstring,"YES",3) == 0)
				uselockfiles = 1;
			else if (strncmp(argstring,"no",2) == 0 || strncmp(argstring,"NO",2) == 0)
				uselockfiles = 0;
			else if (strncmp(argstring,"1",1) == 0)
				uselockfiles = 1;
			else if (strncmp(argstring,"0",1) == 0)
				uselockfiles = 0;
			flag++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'W':
		case 'w':
			sscanf (optarg,"%s", mbproject);
			flag++;
			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
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
		fprintf(stderr,"dbg2       verbose:                    %d\n",verbose);
		fprintf(stderr,"dbg2       help:                       %d\n",help);
		fprintf(stderr,"dbg2       format:                     %d\n",format);
		fprintf(stderr,"dbg2       pings:                      %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:                    %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:                  %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:                  %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:                  %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:                  %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:                 %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:                 %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:                 %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:                 %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:                 %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:                 %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:                 %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:                 %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:                 %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:                 %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:                 %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:                 %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:                 %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:                 %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:                   %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:                    %f\n",timegap);
		fprintf(stderr,"dbg2       psdisplay:                  %s\n",psdisplay);
		fprintf(stderr,"dbg2       imgdisplay:                 %s\n",imgdisplay);
		fprintf(stderr,"dbg2       mbproject:                  %s\n",mbproject);
		fprintf(stderr,"dbg2       fbtversion:                 %d\n",fbtversion);
		fprintf(stderr,"dbg2       uselockfiles:               %d\n",uselockfiles);
		fprintf(stderr,"dbg2       fileiobuffer:               %d\n",fileiobuffer);
		fprintf(stderr,"dbg2       primary_colortable:         %d\n",primary_colortable);
		fprintf(stderr,"dbg2       primary_colortable_mode:    %d\n",primary_colortable_mode);
		fprintf(stderr,"dbg2       primary_shade_mode:         %d\n",primary_shade_mode);
		fprintf(stderr,"dbg2       slope_colortable:           %d\n",slope_colortable);
		fprintf(stderr,"dbg2       slope_colortable_mode:      %d\n",slope_colortable_mode);
		fprintf(stderr,"dbg2       secondary_colortable:       %d\n",secondary_colortable);
		fprintf(stderr,"dbg2       secondary_colortable_mode:  %d\n",secondary_colortable_mode);
		fprintf(stderr,"dbg2       illuminate_magnitude:       %f\n",illuminate_magnitude);
		fprintf(stderr,"dbg2       illuminate_elevation:       %f\n",illuminate_elevation);
		fprintf(stderr,"dbg2       illuminate_azimuth:         %f\n",illuminate_azimuth);
		fprintf(stderr,"dbg2       slope_magnitude:            %f\n",slope_magnitude);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* write out new ~/.mbio_defaults file if needed */
	if (flag)
		{
		strcpy(file,getenv(HOME));
		strcat(file,"/.mbio_defaults");
		if ((fp = fopen(file, "w")) == NULL)
			{
			fprintf (stderr, "Could not open file %s\n", file);
			error = MB_ERROR_OPEN_FAIL;
			exit(error);
			}
		fprintf(fp,"MBIO Default Control Parameters\n");
		fprintf(fp,"lonflip:    %d\n",lonflip);
		fprintf(fp,"timegap:    %f\n",timegap);
		fprintf(fp,"ps viewer:  %s\n",psdisplay);
		fprintf(fp,"img viewer: %s\n",imgdisplay);
		fprintf(fp,"project:    %s\n",mbproject);
		fprintf(fp,"fbtversion: %d\n",fbtversion);
		fprintf(fp,"uselockfiles:%d\n",uselockfiles);
		fprintf(fp,"fileiobuffer:%d\n",fileiobuffer);
		fprintf(fp,"mbview_primary_colortable:        %d\n",primary_colortable);
		fprintf(fp,"mbview_primary_colortable_mode:   %d\n",primary_colortable_mode);
		fprintf(fp,"mbview_primary_shade_mode:        %d\n",primary_shade_mode);
		fprintf(fp,"mbview_slope_colortable:          %d\n",slope_colortable);
		fprintf(fp,"mbview_slope_colortable_mode:     %d\n",slope_colortable_mode);
		fprintf(fp,"mbview_secondary_colortable:      %d\n",secondary_colortable);
		fprintf(fp,"mbview_secondary_colortable_mode: %d\n",secondary_colortable_mode);
		fprintf(fp,"mbview_illuminate_magnitude:      %f\n",illuminate_magnitude);
		fprintf(fp,"mbview_illuminate_elevation:      %f\n",illuminate_elevation);
		fprintf(fp,"mbview_illuminate_azimuth:        %f\n",illuminate_azimuth);
		fprintf(fp,"mbview_slope_magnitude:           %f\n",slope_magnitude);
		fclose(fp);

		printf("\nNew MBIO Default Control Parameters:\n");
		printf("lonflip:    %d\n",lonflip);
		printf("timegap:    %f\n",timegap);
		printf("ps viewer:  %s\n",psdisplay);
		printf("img viewer: %s\n",imgdisplay);
		printf("project:    %s\n",mbproject);
		if (fbtversion == 2)
			printf("fbtversion: 2 (old)\n");
		else if (fbtversion == 3)
			printf("fbtversion: 3 (new)\n");
		else
			printf("fbtversion: %d\n",fbtversion);
		printf("uselockfiles: %d\n",uselockfiles);
		if (fileiobuffer == 0)
			printf("fileiobuffer: %d (use standard fread() & fwrite() buffering)\n",fileiobuffer);
		else if (fileiobuffer > 0)
			printf("fileiobuffer: %d (use %d kB buffer for fread() & fwrite())\n",fileiobuffer,fileiobuffer);
		else
			printf("fileiobuffer: %d (use mmap for file i/o)\n",fileiobuffer);
		if (primary_colortable == MBV_COLORTABLE_HAXBY)
			printf("mbview primary colortable:    %d  (Haxby)\n",primary_colortable);
		else if (primary_colortable == MBV_COLORTABLE_BRIGHT)
			printf("mbview primary colortable:    %d  (Bright)\n",primary_colortable);
		else if (primary_colortable == MBV_COLORTABLE_MUTED)
			printf("mbview primary colortable:    %d  (Muted)\n",primary_colortable);
		else if (primary_colortable == MBV_COLORTABLE_GRAY)
			printf("mbview primary colortable:    %d  (Grayscale)\n",primary_colortable);
		else if (primary_colortable == MBV_COLORTABLE_FLAT)
			printf("mbview primary colortable:    %d  (Flat  gray)\n",primary_colortable);
		else if (primary_colortable == MBV_COLORTABLE_SEALEVEL1)
			printf("mbview primary colortable:    %d  (Sealevel 1)\n",primary_colortable);
		else if (primary_colortable == MBV_COLORTABLE_SEALEVEL2)
			printf("mbview primary colortable:    %d  (Sealevel 2)\n",primary_colortable);
		if (primary_colortable_mode == MBV_COLORTABLE_NORMAL)
			printf("mbview primary colortable mode:    %d  (Normal: Cold to Hot)\n",primary_colortable_mode);
		else
			printf("mbview primary colortable mode:    %d  (Reversed: Hot to Cold)\n",primary_colortable_mode);
		if (primary_shade_mode == MBV_SHADE_VIEW_NONE)
			printf("mbview primary shade mode:    %d  (No shading)\n",primary_shade_mode);
		else if (primary_shade_mode == MBV_SHADE_VIEW_ILLUMINATION)
			printf("mbview primary shade mode:    %d  (Shading by illumination)\n",primary_shade_mode);
		else if (primary_shade_mode == MBV_SHADE_VIEW_SLOPE)
			printf("mbview primary shade mode:    %d  (Shading by slope magnitude)\n",primary_shade_mode);
		else if (primary_shade_mode == MBV_SHADE_VIEW_OVERLAY)
			printf("mbview primary shade mode:    %d  (Shading by overlay)\n",primary_shade_mode);

		if (slope_colortable == MBV_COLORTABLE_HAXBY)
			printf("mbview slope colortable:    %d  (Haxby)\n",slope_colortable);
		else if (slope_colortable == MBV_COLORTABLE_BRIGHT)
			printf("mbview slope colortable:    %d  (Bright)\n",slope_colortable);
		else if (slope_colortable == MBV_COLORTABLE_MUTED)
			printf("mbview slope colortable:    %d  (Muted)\n",slope_colortable);
		else if (slope_colortable == MBV_COLORTABLE_GRAY)
			printf("mbview slope colortable:    %d  (Grayscale)\n",slope_colortable);
		else if (slope_colortable == MBV_COLORTABLE_FLAT)
			printf("mbview slope colortable:    %d  (Flat  gray)\n",slope_colortable);
		else if (slope_colortable == MBV_COLORTABLE_SEALEVEL1)
			printf("mbview slope colortable:    %d  (Sealevel 1)\n",slope_colortable);
		else if (slope_colortable == MBV_COLORTABLE_SEALEVEL2)
			printf("mbview slope colortable:    %d  (Sealevel 2)\n",slope_colortable);
		if (slope_colortable_mode == MBV_COLORTABLE_NORMAL)
			printf("mbview slope colortable mode:    %d  (Normal: Cold to Hot)\n",slope_colortable_mode);
		else
			printf("mbview slope colortable mode:    %d  (Reversed: Hot to Cold)\n",slope_colortable_mode);

		if (secondary_colortable == MBV_COLORTABLE_HAXBY)
			printf("mbview overlay colortable:    %d  (Haxby)\n",secondary_colortable);
		else if (secondary_colortable == MBV_COLORTABLE_BRIGHT)
			printf("mbview overlay colortable:    %d  (Bright)\n",secondary_colortable);
		else if (secondary_colortable == MBV_COLORTABLE_MUTED)
			printf("mbview overlay colortable:    %d  (Muted)\n",secondary_colortable);
		else if (secondary_colortable == MBV_COLORTABLE_GRAY)
			printf("mbview overlay colortable:    %d  (Grayscale)\n",secondary_colortable);
		else if (secondary_colortable == MBV_COLORTABLE_FLAT)
			printf("mbview overlay colortable:    %d  (Flat  gray)\n",secondary_colortable);
		else if (secondary_colortable == MBV_COLORTABLE_SEALEVEL1)
			printf("mbview overlay colortable:    %d  (Sealevel 1)\n",secondary_colortable);
		else if (secondary_colortable == MBV_COLORTABLE_SEALEVEL2)
			printf("mbview overlay colortable:    %d  (Sealevel 2)\n",secondary_colortable);
		if (secondary_colortable_mode == MBV_COLORTABLE_NORMAL)
			printf("mbview overlay colortable mode:    %d  (Normal: Cold to Hot)\n",secondary_colortable_mode);
		else
			printf("mbview overlay colortable mode:    %d  (Reversed: Hot to Cold)\n",secondary_colortable_mode);
		printf("mbview illumination magnitude:    %f\n",illuminate_magnitude);
		printf("mbview illumination elevation:    %f degrees\n",illuminate_elevation);
		printf("mbview illumination azimuth:      %f degrees\n",illuminate_azimuth);
		printf("mbview slope magnitude:           %f\n",slope_magnitude);
		}

	/* else just list the current defaults */
	else
		{
		printf("\nCurrent MBIO Default Control Parameters:\n");
		printf("lonflip:    %d\n",lonflip);
		printf("timegap:    %f\n",timegap);
		printf("ps viewer:  %s\n",psdisplay);
		printf("img viewer: %s\n",imgdisplay);
		printf("project:    %s\n",mbproject);
		if (fbtversion == 2)
			printf("fbtversion: 2 (old)\n");
		else if (fbtversion == 3)
			printf("fbtversion: 3 (new)\n");
		else
			printf("fbtversion: %d\n",fbtversion);
		printf("uselockfiles: %d\n",uselockfiles);
		if (fileiobuffer == 0)
			printf("fileiobuffer: %d (use standard fread() & fwrite() buffering)\n",fileiobuffer);
		else if (fileiobuffer > 0)
			printf("fileiobuffer: %d (use %d kB buffer for fread() & fwrite())\n",fileiobuffer,fileiobuffer);
		else
			printf("fileiobuffer: %d (use mmap for file i/o)\n",fileiobuffer);
		if (primary_colortable == MBV_COLORTABLE_HAXBY)
			printf("mbview primary colortable:         %d  (Haxby)\n",primary_colortable);
		else if (primary_colortable == MBV_COLORTABLE_BRIGHT)
			printf("mbview primary colortable:         %d  (Bright)\n",primary_colortable);
		else if (primary_colortable == MBV_COLORTABLE_MUTED)
			printf("mbview primary colortable:         %d  (Muted)\n",primary_colortable);
		else if (primary_colortable == MBV_COLORTABLE_GRAY)
			printf("mbview primary colortable:         %d  (Grayscale)\n",primary_colortable);
		else if (primary_colortable == MBV_COLORTABLE_FLAT)
			printf("mbview primary colortable:         %d  (Flat  gray)\n",primary_colortable);
		else if (primary_colortable == MBV_COLORTABLE_SEALEVEL1)
			printf("mbview primary colortable:         %d  (Sealevel 1)\n",primary_colortable);
		else if (primary_colortable == MBV_COLORTABLE_SEALEVEL2)
			printf("mbview primary colortable:         %d  (Sealevel 2)\n",primary_colortable);
		if (primary_colortable_mode == MBV_COLORTABLE_NORMAL)
			printf("mbview primary colortable mode:    %d  (Normal: Cold to Hot)\n",primary_colortable_mode);
		else
			printf("mbview primary colortable mode:    %d  (Reversed: Hot to Cold)\n",primary_colortable_mode);
		if (primary_shade_mode == MBV_SHADE_VIEW_NONE)
			printf("mbview primary shade mode:         %d  (No shading)\n",primary_shade_mode);
		else if (primary_shade_mode == MBV_SHADE_VIEW_ILLUMINATION)
			printf("mbview primary shade mode:         %d  (Shading by illumination)\n",primary_shade_mode);
		else if (primary_shade_mode == MBV_SHADE_VIEW_SLOPE)
			printf("mbview primary shade mode:         %d  (Shading by slope magnitude)\n",primary_shade_mode);
		else if (primary_shade_mode == MBV_SHADE_VIEW_OVERLAY)
			printf("mbview primary shade mode:         %d  (Shading by overlay)\n",primary_shade_mode);

		if (slope_colortable == MBV_COLORTABLE_HAXBY)
			printf("mbview slope colortable:           %d  (Haxby)\n",slope_colortable);
		else if (slope_colortable == MBV_COLORTABLE_BRIGHT)
			printf("mbview slope colortable:           %d  (Bright)\n",slope_colortable);
		else if (slope_colortable == MBV_COLORTABLE_MUTED)
			printf("mbview slope colortable:           %d  (Muted)\n",slope_colortable);
		else if (slope_colortable == MBV_COLORTABLE_GRAY)
			printf("mbview slope colortable:           %d  (Grayscale)\n",slope_colortable);
		else if (slope_colortable == MBV_COLORTABLE_FLAT)
			printf("mbview slope colortable:           %d  (Flat  gray)\n",slope_colortable);
		else if (slope_colortable == MBV_COLORTABLE_SEALEVEL1)
			printf("mbview slope colortable:           %d  (Sealevel 1)\n",slope_colortable);
		else if (slope_colortable == MBV_COLORTABLE_SEALEVEL2)
			printf("mbview slope colortable:           %d  (Sealevel 2)\n",slope_colortable);
		if (slope_colortable_mode == MBV_COLORTABLE_NORMAL)
			printf("mbview slope colortable mode:      %d  (Normal: Cold to Hot)\n",slope_colortable_mode);
		else
			printf("mbview slope colortable mode:      %d  (Reversed: Hot to Cold)\n",slope_colortable_mode);

		if (secondary_colortable == MBV_COLORTABLE_HAXBY)
			printf("mbview overlay colortable:         %d  (Haxby)\n",secondary_colortable);
		else if (secondary_colortable == MBV_COLORTABLE_BRIGHT)
			printf("mbview overlay colortable:         %d  (Bright)\n",secondary_colortable);
		else if (secondary_colortable == MBV_COLORTABLE_MUTED)
			printf("mbview overlay colortable:         %d  (Muted)\n",secondary_colortable);
		else if (secondary_colortable == MBV_COLORTABLE_GRAY)
			printf("mbview overlay colortable:         %d  (Grayscale)\n",secondary_colortable);
		else if (secondary_colortable == MBV_COLORTABLE_FLAT)
			printf("mbview overlay colortable:         %d  (Flat  gray)\n",secondary_colortable);
		else if (secondary_colortable == MBV_COLORTABLE_SEALEVEL1)
			printf("mbview overlay colortable:         %d  (Sealevel 1)\n",secondary_colortable);
		else if (secondary_colortable == MBV_COLORTABLE_SEALEVEL2)
			printf("mbview overlay colortable:         %d  (Sealevel 2)\n",secondary_colortable);
		if (secondary_colortable_mode == MBV_COLORTABLE_NORMAL)
			printf("mbview overlay colortable mode:    %d  (Normal: Cold to Hot)\n",secondary_colortable_mode);
		else
			printf("mbview overlay colortable mode:    %d  (Reversed: Hot to Cold)\n",secondary_colortable_mode);
		printf("mbview illumination magnitude:     %f\n",illuminate_magnitude);
		printf("mbview illumination elevation:     %f degrees\n",illuminate_elevation);
		printf("mbview illumination azimuth:       %f degrees\n",illuminate_azimuth);
		printf("mbview slope magnitude:            %f\n",slope_magnitude);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s> completed\n",program_name);
		fprintf(stderr,"dbg2  Ending status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
