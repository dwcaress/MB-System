/*--------------------------------------------------------------------
 *    The MB-system:	mbedit.c	4/8/93
 *    $Id: mbedit_prog.c,v 4.16 1997-04-29 15:50:50 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 1995, 1997 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBEDIT is an interactive beam editor for multibeam bathymetry data.
 * It can work with any data format supported by the MBIO library.
 * This version uses the MOTIF toolkit and has been developed using
 * the Builder Xsessory package by ICS.  This file contains
 * the code that does not directly depend on the MOTIF interface - the 
 * companion file mbedit.c contains the user interface related 
 * code.
 *
 * Author:	D. W. Caress
 * Date:	April 8, 1993
 * Date:	March 28, 1997  GUI recast
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.15  1997/04/22  19:26:36  caress
 * Fixed startup mode.
 *
 * Revision 4.14  1997/04/21  16:56:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.14  1997/04/16  21:29:30  caress
 * Complete rewrite without uid file.
 *
 * Revision 4.13  1996/07/31  18:40:14  caress
 * The program now checks if the format being opened supports
 * sidescan - if it does support sidescan the maximum buffer
 * size is 1000 rather than 5000.
 *
 * Revision 4.12  1996/04/22  13:20:55  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.11  1996/04/17  23:11:09  caress
 * Fixed bug that caused display to reset to beginning of buffer
 * at inconvenient times.
 *
 * Revision 4.10  1996/04/05  15:25:11  caress
 * Fixed GUI mode so done means quit for real. Also changed done and
 * quit handling in browse mode so that the program doesn't read the
 * entire data file before closing it.
 *
 * Revision 4.9  1996/02/12  17:09:35  caress
 * Added autoscaling of acrosstrack distance when files
 * are first read and added -G argument to force done
 * events to be treated as quit events when mbedit is
 * started up by a GUI.
 *
 * Revision 4.8  1996/01/26  21:22:00  caress
 * Version 4.3 distribution.
 *
 * Revision 4.7  1995/09/28  18:03:05  caress
 * Improved handling of .mbxxx file suffix convention.
 *
 * Revision 4.6  1995/09/18  22:42:44  caress
 * I must have changed something!?
 *
 * Revision 4.5  1995/05/12  17:29:16  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.4  1995/03/15  14:12:23  caress
 * Added macro for zeroing data and made it possible to
 * hold down keyboard flagging keys.
 *
 * Revision 4.3  1995/03/06  19:40:49  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1995/02/14  19:16:04  caress
 * Improved widget handling, uses swath width rather than plot scale,
 * now handles default values properly.
 *
 * Revision 4.1  1994/11/24  01:52:07  caress
 * Now centers profiles based on bathymetry median value
 * rather than mean.
 *
 * Revision 4.0  1994/10/21  11:55:41  caress
 * Release V4.0
 *
 * Revision 1.1  1994/07/14  21:21:54  brockda
 * Initial revision
 *
 * Revision 4.2  1994/04/12  00:46:38  caress
 * Changed call to mb_buffer_close in accordance with change
 * in mb_buffer source code.  The parameter list now includes
 * mbio_ptr.
 *
 * Revision 4.1  1994/03/12  01:49:07  caress
 * Added declarations of ctime and/or getenv for compatability
 * with SGI compilers.
 *
 * Revision 5.0  1994/04/29  08:35 RCM
 * First cut at OPENLOOK to MOTIF conversion.
 *
 * Revision 4.0  1994/03/05  23:54:35  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:51:47  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/03/01  00:19:16  caress
 * First cut at new version.
 *
 * Revision 3.4  1993/11/03  19:40:34  caress
 * Changed scaling:
 *    x-scale has larger maximum.
 *    y-scale widget now sets vertical exageration instead
 *      of y-scale.
 *
 * Revision 3.3  1993/08/30  19:03:20  caress
 * Added Go To option.  Also added Erase and Restore edit modes.
 *
 * Revision 3.2  1993/08/17  00:28:52  caress
 * Version current as of 16 August 1993.
 *
 * Revision 3.1  1993/05/14  23:28:56  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/22  18:49:44  dale
 * Initial version
 *
 */

/*--------------------------------------------------------------------*/

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

/* MBIO include files */
#include "mb_format.h"
#include "mb_status.h"
#include "mb_define.h"
#include "mb_io.h"

/* output mode defines */
#define	MBEDIT_OUTPUT_OUTPUT 0
#define	MBEDIT_OUTPUT_BROWSE 1

/* edit action defines */
#define	MBEDIT_FLAG	1
#define	MBEDIT_UNFLAG	2
#define	MBEDIT_ZERO	3

/* edit outbounds defines */
#define	MBEDIT_OUTBOUNDS_NONE		0
#define	MBEDIT_OUTBOUNDS_FLAGGED	1
#define	MBEDIT_OUTBOUNDS_UNFLAGGED	2

/* ping structure definition */
struct mbedit_ping_struct 
	{
	int	id;
	int	record;
	int	outbounds;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	*bath;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*ssacrosstrack;
	double	*ssalongtrack;
	double	*amp;
	double	*ss;
	int	*bath_x;
	int	*bath_y;
	int	label_x;
	int	label_y;
	int	zap_x1;
	int	zap_x2;
	int	zap_y1;
	int	zap_y2;
	};

/* id variables */
static char rcs_id[] = "$Id: mbedit_prog.c,v 4.16 1997-04-29 15:50:50 caress Exp $";
static char program_name[] = "MBedit";
static char help_message[] =  
"MBedit is an interactive editor used to identify and flag\n\
artifacts in swath sonar bathymetry data. Once a file has\n\
been read in, MBedit displays the bathymetry profiles from\n\
several pings, allowing the user to identify and flag\n\
anomalous beams. Flagging is handled internally by setting\n\
depth values negative, so that no information is lost.";
static char usage_message[] = "mbedit [-Byr/mo/da/hr/mn/sc -D  -Eyr/mo/da/hr/mn/sc \n\t-Fformat -Ifile -Ooutfile -S -V -H]";

/* status variables */
int	error = MB_ERROR_NO_ERROR;
int	verbose = 0;
char	*message = NULL;

/* MBIO control parameters */
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
int	beams_bath;
int	beams_amp;
int	pixels_ss;
char	ifile[128];
char	ofile[128];
int	ofile_defined = MB_NO;
char	*imbio_ptr = NULL;
char	*ombio_ptr = NULL;
int	output_mode = MBEDIT_OUTPUT_OUTPUT;
int	gui_mode = MB_NO;
int	startup_save_mode = MB_NO;

/* mbio read and write values */
char	*store_ptr = NULL;
int	kind;
int	id;
int	time_i[7];
double	time_d;
double	navlon;
double	navlat;
double	speed;
double	heading;
double	distance;
int	nbath;
int	namp;
int	nss;
double	*bath = NULL;
double	*bathacrosstrack = NULL;
double	*bathalongtrack = NULL;
double	*amp = NULL;
double	*ss = NULL;
double	*ssacrosstrack = NULL;
double	*ssalongtrack = NULL;
int	idata = 0;
int	icomment = 0;
int	odata = 0;
int	ocomment = 0;
char	comment[256];

/* buffer control variables */
#define	MBEDIT_BUFFER_SIZE	MB_BUFFER_MAX
int	file_open = MB_NO;
char	*buff_ptr = NULL;
int	buff_size = MBEDIT_BUFFER_SIZE;
int	buff_size_max = MBEDIT_BUFFER_SIZE;
int	holdd_size = 100;
int	nload = 0;
int	ndump = 0;
int	nbuff = 0;
int	nlist = 0;
int	current = 0;
int	current_id = 0;
int	nload_total = 0;
int	ndump_total = 0;
char	last_ping[128];

/* save file control variables */
int	sifile_open = MB_NO;
int	sofile_open = MB_NO;
char	sifile[128];
char	sofile[128];
FILE	*sifp;
FILE	*sofp;

/* ping drawing control variables */
#define	MBEDIT_MAX_PINGS	100
#define	MBEDIT_PICK_DISTANCE	50
#define	MBEDIT_ERASE_DISTANCE	15
struct mbedit_ping_struct	ping[MBEDIT_MAX_PINGS];
int	list[MBEDIT_BUFFER_SIZE];
int	plot_size = 10;
int	nplot = 0;
int	mbedit_xgid;
int	borders[4];
int	margin;
int	xmin, xmax;
int	ymin, ymax;
int	exager = 100;
int	plot_width = 5000;
int	xscale;
int	yscale;
int	x_interval = 1000;
int	y_interval = 250;
int	show_flagged = MB_NO;
int	beam_save = MB_NO;
int	iping_save = 0;
int	jbeam_save = 0;
double	*bathlist;

/* color control values */
#define	WHITE	0	
#define	BLACK	1	
#define RED	2
#define GREEN	3
#define BLUE	4
#define CORAL	5
#define	XG_SOLIDLINE	0
#define	XG_DASHLINE	1
int	ncolors;
int	pixel_values[256];

/* system function declarations */
char	*ctime();
char	*getenv();
char	*strstr();

/*--------------------------------------------------------------------*/
int mbedit_init(argc,argv,startup_file)
int	argc;
char	**argv;
int	*startup_file;
{
	/* local variables */
	char	*function_name = "mbedit_init";
	int	status = MB_SUCCESS;
	int	fileflag = 0;
	int	i;

	/* parsing variables */
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* set default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);
	pings = 1;
	lonflip = 0;
	bounds[0] = -360.;
	bounds[1] = 360.;
	bounds[2] = -90.;
	bounds[3] = 90.;
	btime_i[0] = 1962;
	btime_i[1] = 2;
	btime_i[2] = 21;
	btime_i[3] = 10;
	btime_i[4] = 30;
	btime_i[5] = 0;
	btime_i[6] = 0;
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	etime_i[6] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;
	strcpy(ifile,"\0");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhB:b:DdE:e:F:f:GgI:i:O:o:S:s:")) != -1)
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
		case 'D':
		case 'd':
			output_mode = MBEDIT_OUTPUT_BROWSE;
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
		case 'G':
		case 'g':
			gui_mode = MB_YES;
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", ifile);
			flag++;
			fileflag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", ofile);
			ofile_defined = MB_YES;
			flag++;
			break;
		case 'S':
		case 's':
			startup_save_mode = MB_YES;
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
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       help:            %d\n",help);
		fprintf(stderr,"dbg2       format:          %d\n",format);
		fprintf(stderr,"dbg2       input file:      %s\n",ifile);
		fprintf(stderr,"dbg2       save mode:       %d\n",startup_save_mode);
		fprintf(stderr,"dbg2       output mode:     %d\n",output_mode);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       argc:      %d\n",argc);
		for (i=0;i<argc;i++)
			fprintf(stderr,"dbg2       argv[%d]:    %s\n",
				i,argv[i]);
		}

	/* if file specified then use it */
	if (fileflag > 0)
		*startup_file = MB_YES;
	else
		*startup_file = MB_NO;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       startup_file: %d\n",*startup_file);
		fprintf(stderr,"dbg2       error:        %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbedit_startup_file()
{
	/* local variables */
	char	*function_name = "mbedit_startup_file";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* open startup file */
	status = mbedit_action_open(ifile,format,
			startup_save_mode, output_mode, 
			plot_width, exager,
			x_interval, y_interval, plot_size,
			show_flagged, 
			&buff_size, &buff_size_max,
			&holdd_size, 
			&ndump, &nload, &nbuff,
			&nlist, &current_id, &nplot);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbedit_set_graphics(xgid,brdr,ncol,pixels)
int	xgid;
int	*brdr;
int	ncol;
int	*pixels;
{
	/* local variables */
	char	*function_name = "mbedit_set_graphics";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       xgid:         %d\n",xgid);
		for (i=0;i<4;i++)
			fprintf(stderr,"dbg2       brdr[%d]:     %d\n",
				i,brdr[i]);
		fprintf(stderr,"dbg2       ncolors:      %d\n",ncol);
		for (i=0;i<ncol;i++)
			fprintf(stderr,"dbg2       pixel[%d]:     %d\n",
				pixels[i]);
		}

	/* set graphics id */
	mbedit_xgid = xgid;

	/* set graphics bounds */
	for (i=0;i<4;i++)
		borders[i] = brdr[i];

	/* set colors */
	ncolors = ncol;
	for (i=0;i<ncolors;i++)
		pixel_values[i] = pixels[i];

	/* set scaling */
	margin = (borders[1] - borders[0])/16;
	xmin = 5*margin;
	xmax = borders[1] - margin;
	ymin = margin;
	ymax = borders[3] - margin/2;
	xscale = 100*plot_width/(xmax - xmin);
	yscale = (xscale*exager)/100;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_get_defaults(plt_size_max,plt_size,sh_flggd,
	buffer_size_max,buffer_size,hold_size,form,
	plwd,exgr,xntrvl,yntrvl,ttime_i,outmode)
int	*plt_size_max;
int	*plt_size;
int	*sh_flggd;
int	*buffer_size_max;
int	*buffer_size;
int	*hold_size;
int	*form;
int	*plwd;
int	*exgr;
int	*xntrvl;
int	*yntrvl;
int	*ttime_i;
int	*outmode;
{
	/* local variables */
	char	*function_name = "mbedit_get_defaults";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* get maximum number of pings to plot */
	*plt_size_max = MBEDIT_MAX_PINGS;
	*plt_size = plot_size;
	
	/* get show flagged flag */
	*sh_flggd = show_flagged;

	/* get maximum and starting buffer sizes */
	*buffer_size_max = buff_size_max;
	*buffer_size = buff_size;

	/* get starting hold size */
	*hold_size = holdd_size;

	/* get format */
	*form = format;

	/* get scaling */
	*plwd = plot_width;
	*exgr = exager;

	/* get tick intervals */
	*xntrvl = x_interval;
	*yntrvl = y_interval;

	/* get time of first data */
	if (file_open == MB_YES && nlist > 0)
		{
		status = mb_buffer_get_next_data(verbose,
			buff_ptr,imbio_ptr,list[0],&id,
			time_i,&time_d,
			&navlon,&navlat,
			&speed,&heading,
			&beams_bath,&beams_amp,&pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			&error);
		for (i=0;i<7;i++)
			ttime_i[i] = time_i[i];
		}
	else
		for (i=0;i<7;i++)
			ttime_i[i] = btime_i[i];

	/* get output mode */
	*outmode = output_mode;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       plot max:    %d\n",*plt_size_max);
		fprintf(stderr,"dbg2       plot_size:   %d\n",*plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",*sh_flggd);
		fprintf(stderr,"dbg2       buffer max:  %d\n",*buffer_size_max);
		fprintf(stderr,"dbg2       buffer_size: %d\n",*buffer_size);
		fprintf(stderr,"dbg2       hold_size:   %d\n",*hold_size);
		fprintf(stderr,"dbg2       format:      %d\n",*form);
		fprintf(stderr,"dbg2       plot_width:  %d\n",*plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",*exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",*xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",*yntrvl);
		for (i=0;i<7;i++)
			fprintf(stderr,"dbg2       ttime[%d]:    %d\n",
				ttime_i[i]);
		fprintf(stderr,"dbg2       outmode:     %d\n",*outmode);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_open(file,form,savemode,outmode,
	plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
	buffer_size,buffer_size_max,hold_size,
	ndumped,nloaded,nbuffer,ngood,icurrent,nplt)
char	*file;
int	form;
int	savemode;
int	outmode;
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*buffer_size;
int	*buffer_size_max;
int	*hold_size;
int	*ndumped;
int	*nloaded;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_action_open";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       file:            %s\n",file);
		fprintf(stderr,"dbg2       format:          %d\n",form);
		fprintf(stderr,"dbg2       savemode:        %d\n",savemode);
		fprintf(stderr,"dbg2       outmode:         %d\n",outmode);
		fprintf(stderr,"dbg2       plot_width:      %d\n",plwd);
		fprintf(stderr,"dbg2       exager:          %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:      %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:      %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:       %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:    %d\n",sh_flggd);
		fprintf(stderr,"dbg2       buffer_size:     %d\n",*buffer_size);
		fprintf(stderr,"dbg2       buffer_size_max: %d\n",*buffer_size_max);
		fprintf(stderr,"dbg2       hold_size:       %d\n",*hold_size);
		}

	/* set the output mode */
	output_mode = outmode;

	/* clear the screen */
	status = mbedit_clear_screen();

	/* open the file */
	status = mbedit_open_file(file, form, savemode);
	
	/* check buffer size */
	if (status == MB_SUCCESS)
		{
		mbedit_check_buffer_size(form,
			buffer_size,buffer_size_max);
		if (*hold_size > *buffer_size)
			*hold_size = *buffer_size / 2;
		buff_size = *buffer_size;
		buff_size_max = *buffer_size_max;
		holdd_size = *hold_size;
		}

	/* load the buffer */
	if (status == MB_SUCCESS)
		{
		status = mbedit_load_data(*buffer_size,nloaded,nbuffer,
			ngood,icurrent);

		/* keep going until good data or end of file found */
		while (*nloaded > 0 && *ngood == 0)
			{
			/* dump the buffer */
			status = mbedit_dump_data(*hold_size,ndumped,nbuffer);
	
			/* load the buffer */
			status = mbedit_load_data(*buffer_size,nloaded,nbuffer,
				ngood,icurrent);
			}
			
		if (*ngood <= 0)
			do_error_dialog("No data were read from the input", 
					"file. You may have specified an", 
					"incorrect MB-System format id!");
		}

	/* set up plotting */
	if (*ngood > 0)
		{		
		/* turn file button off */
		do_filebutton_off();

		/* now plot it */
		status = mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,
			    plt_size,sh_flggd,nplt,MB_YES);
		}

	/* reset beam_save */
	beam_save = MB_NO;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       buffer_size:     %d\n",*buffer_size);
		fprintf(stderr,"dbg2       buffer_size_max: %d\n",*buffer_size_max);
		fprintf(stderr,"dbg2       hold_size:       %d\n",*hold_size);
		fprintf(stderr,"dbg2       ndumped:         %d\n",*ndumped);
		fprintf(stderr,"dbg2       nloaded:         %d\n",*nloaded);
		fprintf(stderr,"dbg2       nbuffer:         %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:           %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:        %d\n",*icurrent);
		fprintf(stderr,"dbg2       nplot:           %d\n",*nplt);
		fprintf(stderr,"dbg2       error:           %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_next_buffer(hold_size,buffer_size,
	plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,ndumped,
	nloaded,nbuffer,ngood,icurrent,nplt,quit)
int	hold_size;
int	buffer_size;
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*ndumped;
int	*nloaded;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
int	*quit;
{
	/* local variables */
	char	*function_name = "mbedit_action_next_buffer";
	int	status = MB_SUCCESS;
	int	save_dumped = 0;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       hold_size:   %d\n",hold_size);
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		}

	/* clear the screen */
	status = mbedit_clear_screen();
	
	/* set quit off */
	*quit = MB_NO;

	/* check if a file has been opened */
	if (file_open == MB_YES)
		{

		/* keep going until good data or end of file found */
		do
			{
			/* dump the buffer */
			status = mbedit_dump_data(hold_size,ndumped,nbuffer);

			/* load the buffer */
			status = mbedit_load_data(buffer_size,nloaded,nbuffer,
				ngood,icurrent);
			}
		while (*nloaded > 0 && *ngood == 0);

		/* if end of file reached then 
			dump last buffer and close file */
		if (*nloaded <= 0)
			{
			save_dumped = *ndumped;
			status = mbedit_dump_data(0,ndumped,nbuffer);
			status = mbedit_close_file();
			*ndumped = *ndumped + save_dumped;
			*nplt = 0;
				
			/* if in normal mode last next_buffer 
				does not mean quit,
				if in gui mode it does mean quit */
			if (gui_mode == MB_YES)
				*quit = MB_YES;
			else
				*quit = MB_NO;
		
			/* if quitting let the world know... */
			if (*quit == MB_YES && verbose >= 1)
				fprintf(stderr,"\nQuitting MBedit\nBye Bye...\n");
			}

		/* else set up plotting */
		else
			{
			status = mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,
				plt_size,sh_flggd,nplt,MB_YES);
			}
		}

	/* if no file open set failure status */
	else
		{
		status = MB_FAILURE;
		*ndumped = 0;
		*nloaded = 0;
		*nbuffer = nbuff;
		*ngood = nlist;
		current_id = 0;
		*icurrent = current_id;
		current = 0;
		*nplt = 0;
		}

	/* reset beam_save */
	beam_save = MB_NO;


	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndumped:     %d\n",*ndumped);
		fprintf(stderr,"dbg2       nloaded:     %d\n",*nloaded);
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       nplot:       %d\n",*nplt);
		fprintf(stderr,"dbg2       quit:        %d\n",*quit);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_close(buffer_size,ndumped,nloaded,nbuffer,ngood,icurrent)
int	buffer_size;
int	*ndumped;
int	*nloaded;
int	*nbuffer;
int	*ngood;
int	*icurrent;
{
	/* local variables */
	char	*function_name = "mbedit_action_close";
	int	status = MB_SUCCESS;
	int	save_nloaded = 0;
	int	save_ndumped = 0;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		}

	/* clear the screen */
	status = mbedit_clear_screen();

	/* if file has been opened and browse mode 
		just dump the current buffer and close the file */
	if (file_open == MB_YES 
		&& output_mode == MBEDIT_OUTPUT_BROWSE)
		{

		/* dump the buffer */
		status = mbedit_dump_data(0,ndumped,nbuffer);
		save_ndumped = save_ndumped + *ndumped;
		*ndumped = save_ndumped;
		*nloaded = save_nloaded;

		/* now close the file */
		status = mbedit_close_file();
		}

	/* if file has been opened deal with all of the data */
	else if (file_open == MB_YES)
		{

		/* dump and load until the end of the file is reached */
		do
			{
			/* dump the buffer */
			status = mbedit_dump_data(0,ndumped,nbuffer);
			save_ndumped = save_ndumped + *ndumped;

			/* load the buffer */
			status = mbedit_load_data(buffer_size,nloaded,nbuffer,ngood,icurrent);
			save_nloaded = save_nloaded + *nloaded;
			}
		while (*nloaded > 0);
		*ndumped = save_ndumped;
		*nloaded = save_nloaded;

		/* now close the file */
		status = mbedit_close_file();
		}

	else
		{
		*ndumped = 0;
		*nloaded = 0;
		*nbuffer = 0;
		*ngood = 0;
		*icurrent = 0;
		status = MB_FAILURE;
		}

	/* reset beam_save */
	beam_save = MB_NO;

	/* let the world know... */
	if (verbose >= 1)
		{
		fprintf(stderr,"\nLast ping viewed: %s\n",last_ping);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndumped:     %d\n",*ndumped);
		fprintf(stderr,"dbg2       nloaded:     %d\n",*nloaded);
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_done(buffer_size,ndumped,nloaded,nbuffer,
	ngood,icurrent, quit)
int	buffer_size;
int	*ndumped;
int	*nloaded;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*quit;
{
	/* local variables */
	char	*function_name = "mbedit_action_done";
	int	status = MB_SUCCESS;
	int	save_nloaded = 0;
	int	save_ndumped = 0;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		}
		
	/* if in normal mode done does not mean quit,
		if in gui mode done does mean quit */
	if (gui_mode == MB_YES)
		*quit = MB_YES;
	else
		*quit = MB_NO;

	/* if quitting let the world know... */
	if (*quit == MB_YES && verbose >= 1)
		fprintf(stderr,"\nShutting MBedit down without further ado...\n");

	/* call routine to deal with saving the current file, if any */
	if (file_open == MB_YES)
		status = mbedit_action_close(buffer_size,ndumped,nloaded,
			nbuffer,ngood,icurrent);

	/* if quitting let the world know... */
	if (*quit == MB_YES && verbose >= 1)
		fprintf(stderr,"\nQuitting MBedit\nBye Bye...\n");

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndumped:     %d\n",*ndumped);
		fprintf(stderr,"dbg2       nloaded:     %d\n",*nloaded);
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       quit:        %d\n",*quit);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_quit(buffer_size,ndumped,nloaded,nbuffer,ngood,icurrent)
int	buffer_size;
int	*ndumped;
int	*nloaded;
int	*nbuffer;
int	*ngood;
int	*icurrent;
{
	/* local variables */
	char	*function_name = "mbedit_action_quit";
	int	status = MB_SUCCESS;
	int	save_nloaded = 0;
	int	save_ndumped = 0;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		}

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\nShutting MBedit down without further ado...\n");

	/* call routine to deal with saving the current file, if any */
	if (file_open == MB_YES)
		status = mbedit_action_close(buffer_size,ndumped,nloaded,
			nbuffer,ngood,icurrent);

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\nQuitting MBedit\nBye Bye...\n");

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndumped:     %d\n",*ndumped);
		fprintf(stderr,"dbg2       nloaded:     %d\n",*nloaded);
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_step(step,plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
	nbuffer,ngood,icurrent,nplt)
int	step;
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_action_step";
	int	status = MB_SUCCESS;
	int	old_id, new_id;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       step:        %d\n",step);
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		}

	/* check if a file has been opened */
	if (file_open == MB_YES)
		{

		/* figure out if stepping is possible */
		old_id = current_id;
		new_id = current_id + step;
		if (new_id < 0)
			new_id = 0;
		if (new_id >= nlist)
			new_id = nlist - 1;

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nlist;
		current_id = new_id;
		*icurrent = current_id;
		current = list[current_id];

		/* set the plotting list */
		if (*ngood > 0)
			{
			status = mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,
				    plt_size,sh_flggd,nplt,MB_NO);
			}

		/* set failure flag if no step was made */
		if (new_id == old_id)
			status = MB_FAILURE;
		}

	/* if no file open set failure status */
	else
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*ngood = nlist;
		current_id = 0;
		*icurrent = current_id;
		current = 0;
		}

	/* reset beam_save */
	beam_save = MB_NO;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       nplt:        %d\n",*nplt);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_plot(plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
	nbuffer,ngood,icurrent,nplt)
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_action_plot";
	int	status = MB_SUCCESS;
	int	new_id;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		}
		
	/* clear the screen */
	mbedit_clear_screen();

	/* check if a file has been opened */
	if (file_open == MB_YES)
		{

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nlist;
		*icurrent = current_id;
		current = list[current_id];

		/* set the plotting list */
		if (*ngood > 0)
			{
			status = mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,
					plt_size,sh_flggd,nplt,MB_NO);
			}
		}

	/* if no file open set failure status */
	else
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nlist;
		current_id = 0;
		*icurrent = current_id;
		current = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       nplt:        %d\n",*nplt);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_mouse_pick(x_loc,y_loc,
	plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
	nbuffer,ngood,icurrent,nplt)
int	x_loc;
int	y_loc;
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_action_mouse_pick";
	int	status = MB_SUCCESS;
	int	zap_box, zap_ping;
	int	ix, iy, range, range_min;
	int	found;
	int	iping, jbeam;
	char	comment[128];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       x_loc:       %d\n",x_loc);
		fprintf(stderr,"dbg2       y_loc:       %d\n",y_loc);
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
		{
		/* check if a zap box has been picked */
		zap_box = MB_NO;
		for (i=0;i<nplot;i++)
			{
			if (ping[i].outbounds == MBEDIT_OUTBOUNDS_UNFLAGGED)
			    {
			    if (x_loc >= ping[i].zap_x1
				&& x_loc <= ping[i].zap_x2
				&& y_loc >= ping[i].zap_y1
				&& y_loc <= ping[i].zap_y2)
				{
				zap_box = MB_YES;
				zap_ping = i;
				}
			    }
			}
			
		/* if a zap box has been picked call zap routine */
		if (zap_box == MB_YES)
			status = mbedit_action_zap_outbounds(zap_ping,
				plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
				nbuffer,ngood,icurrent,nplt);
		}

	/* do not look for beam pick unless file has been opened 
		and no zap box was picked */
	if (file_open == MB_YES && zap_box == MB_NO)
		{
		/* check if a beam has been picked */
		iping = 0;
		jbeam = 0;
		range_min = 100000;
		for (i=0;i<nplot;i++)
			{
			for (j=0;j<beams_bath;j++)
				{
				if (ping[i].bath[j] != 0.0)
					{
					ix = x_loc - ping[i].bath_x[j];
					iy = y_loc - ping[i].bath_y[j];
					range = (int) 
						sqrt((double) (ix*ix + iy*iy));
					if (range < range_min)
						{
						range_min = range;
						iping = i;
						jbeam = j;
						}
					}
				}
			}

		/* check to see if closest beam is 
			close enough to be picked */
		if (range_min <= MBEDIT_PICK_DISTANCE)
			found = MB_YES;
		else
			found = MB_NO;

		/* unplot the affected beam and ping */
		if (found && *ngood > 0)
			{
			status = mbedit_unplot_ping(iping);
			status = mbedit_unplot_beam(iping,jbeam);
			}

		/* reset picked beam */
		if (found == MB_YES)
			{
			/* write edit to save file */
			if (sofile_open == MB_YES)
			    {
			    if (ping[iping].bath[jbeam] > 0.0)
				mbedit_save_edit(
				    ping[iping].time_d, 
				    jbeam, 
				    MBEDIT_FLAG);
			    else if (ping[iping].bath[jbeam] < 0.0)
				mbedit_save_edit(
				    ping[iping].time_d, 
				    jbeam, 
				    MBEDIT_UNFLAG);
			    }
			
			/* apply edit */
			ping[iping].bath[jbeam] = 
				-ping[iping].bath[jbeam];
			status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[iping].id,
				ping[iping].time_i,ping[iping].time_d,
				ping[iping].navlon,ping[iping].navlat,
				ping[iping].speed,ping[iping].heading,
				beams_bath,beams_amp,pixels_ss,
				ping[iping].bath,
				ping[iping].amp,
				ping[iping].bathacrosstrack,
				ping[iping].bathalongtrack,
				ping[iping].ss,
				ping[iping].ssacrosstrack,
				ping[iping].ssalongtrack,
				comment,
				&error);
			if (verbose >= 1)
				{
				fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
					iping,jbeam,ping[iping].bath[jbeam]);
				fprintf(stderr," flagged\n");
				}
			beam_save = MB_YES;
			iping_save = iping;
			jbeam_save = jbeam;
			}

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nlist;
		*icurrent = current_id;
		current = list[current_id];

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping);
		status = mbedit_plot_beam(iping,jbeam-1);
		status = mbedit_plot_beam(iping,jbeam);
		status = mbedit_plot_beam(iping,jbeam+1);
		
		/* if beam out of bounds replot label */
		if (ping[iping].bath_x[jbeam] < xmin
		    || ping[iping].bath_x[jbeam] > xmax
		    || ping[iping].bath_y[jbeam] < ymin
		    || ping[iping].bath_y[jbeam] > ymax)
		    status = mbedit_plot_ping_label(iping, MB_NO);
		}
		
	/* if no file open set failure status */
	else if (file_open == MB_NO)
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nlist;
		current_id = 0;
		*icurrent = current_id;
		current = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       nplt:        %d\n",*nplt);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_mouse_erase(x_loc,y_loc,
	plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
	nbuffer,ngood,icurrent,nplt)
int	x_loc;
int	y_loc;
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_action_mouse_erase";
	int	status = MB_SUCCESS;
	int	zap_box, zap_ping;
	int	ix, iy, range;
	int	found;
	int	replot_label;
	char	comment[128];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       x_loc:       %d\n",x_loc);
		fprintf(stderr,"dbg2       y_loc:       %d\n",y_loc);
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
	    {
	    /* check if a zap box has been picked */
	    zap_box = MB_NO;
	    for (i=0;i<nplot;i++)
		{
		if (ping[i].outbounds == MBEDIT_OUTBOUNDS_UNFLAGGED)
		    {
		    if (x_loc >= ping[i].zap_x1
			&& x_loc <= ping[i].zap_x2
			&& y_loc >= ping[i].zap_y1
			&& y_loc <= ping[i].zap_y2)
			{
			zap_box = MB_YES;
			zap_ping = i;
			}
		    }
		}
		    
	    /* if a zap box has been picked call zap routine */
	    if (zap_box == MB_YES)
		status = mbedit_action_zap_outbounds(zap_ping,
			plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
			nbuffer,ngood,icurrent,nplt);
	    }

	/* do not look for beam erase unless file has been opened 
		and no zap box was picked */
	if (file_open == MB_YES && zap_box == MB_NO)
	  {

	  /* look for beams to be erased */
	  for (i=0;i<nplot;i++)
	    {
	    found = MB_NO;
	    replot_label = MB_NO;
	    for (j=0;j<beams_bath;j++)
	      {
	      if (ping[i].bath[j] > 0.0)
		{
		ix = x_loc - ping[i].bath_x[j];
		iy = y_loc - ping[i].bath_y[j];
		range = (int) sqrt((double) (ix*ix + iy*iy));
		if (range < MBEDIT_ERASE_DISTANCE && *ngood > 0)
			{
			/* write edit to save file */
			if (sofile_open == MB_YES)
			    {
			    mbedit_save_edit(
				    ping[i].time_d, 
				    j, MBEDIT_FLAG);
			    }
			
	          	/* unplot the affected beam and ping */
			status = mbedit_unplot_ping(i);
			status = mbedit_unplot_beam(i,j);

			/* reset the beam value */
			if (ping[i].bath[j] > 0.0)
			ping[i].bath[j] = 
				-ping[i].bath[j];
			status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[i].id,
				ping[i].time_i,ping[i].time_d,
				ping[i].navlon,ping[i].navlat,
				ping[i].speed,ping[i].heading,
				beams_bath,beams_amp,pixels_ss,
				ping[i].bath,
				ping[i].amp,
				ping[i].bathacrosstrack,
				ping[i].bathalongtrack,
				ping[i].ss,
				ping[i].ssacrosstrack,
				ping[i].ssalongtrack,
				comment,
				&error);
			if (verbose >= 1)
				{
				fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
					i,j,ping[i].bath[j]);
				fprintf(stderr," flagged\n");
				}

			/* replot the affected beams */
		 	found = MB_YES;
			beam_save = MB_YES;
			iping_save = i;
			jbeam_save = j;
			status = mbedit_plot_beam(i,j-1);
			status = mbedit_plot_beam(i,j);
			status = mbedit_plot_beam(i,j+1);
		
			/* if beam out of bounds replot label */
			if (ping[i].bath_x[j] < xmin
			    || ping[i].bath_x[j] > xmax
			    || ping[i].bath_y[j] < ymin
			    || ping[i].bath_y[j] > ymax)
			    replot_label = MB_YES;
			}
		}
	      }

	    /* replot affected ping */
	    if (found == MB_YES && *ngood > 0)
			status = mbedit_plot_ping(i);
	    if (replot_label == MB_YES)
		    status = mbedit_plot_ping_label(i, MB_NO);
	    }

	  /* set some return values */
	  *nbuffer = nbuff;
	  *ngood = nlist;
	  *icurrent = current_id;
	  current = list[current_id];
	  }
		
	/* if no file open set failure status */
	else if (file_open == MB_NO)
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nlist;
		current_id = 0;
		*icurrent = current_id;
		current = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       nplt:        %d\n",*nplt);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_mouse_restore(x_loc,y_loc,
	plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
	nbuffer,ngood,icurrent,nplt)
int	x_loc;
int	y_loc;
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_action_mouse_restore";
	int	status = MB_SUCCESS;
	int	zap_box, zap_ping;
	int	ix, iy, range;
	int	found;
	int	replot_label;
	char	comment[128];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       x_loc:       %d\n",x_loc);
		fprintf(stderr,"dbg2       y_loc:       %d\n",y_loc);
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
	    {
	    /* check if a zap box has been picked */
	    zap_box = MB_NO;
	    for (i=0;i<nplot;i++)
		{
		if (ping[i].outbounds == MBEDIT_OUTBOUNDS_UNFLAGGED)
		    {
		    if (x_loc >= ping[i].zap_x1
			&& x_loc <= ping[i].zap_x2
			&& y_loc >= ping[i].zap_y1
			&& y_loc <= ping[i].zap_y2)
			{
			zap_box = MB_YES;
			zap_ping = i;
			}
		    }
		}
		    
	    /* if a zap box has been picked call zap routine */
	    if (zap_box == MB_YES)
		status = mbedit_action_zap_outbounds(zap_ping,
			plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
			nbuffer,ngood,icurrent,nplt);
	    }

	/* do not look for beam restore unless file has been opened 
		and no zap box was picked */
	if (file_open == MB_YES && zap_box == MB_NO)
	  {

	  /* look for beams to be erased */
	  for (i=0;i<nplot;i++)
	    {
	    found = MB_NO;
	    replot_label = MB_NO;
	    for (j=0;j<beams_bath;j++)
	      {
	      if (ping[i].bath[j] < 0.0)
		{
		ix = x_loc - ping[i].bath_x[j];
		iy = y_loc - ping[i].bath_y[j];
		range = (int) sqrt((double) (ix*ix + iy*iy));
		if (range < MBEDIT_ERASE_DISTANCE && *ngood > 0)
			{
			/* write edit to save file */
			if (sofile_open == MB_YES)
			    {
			    mbedit_save_edit(
				    ping[i].time_d, 
				    j, MBEDIT_UNFLAG);
			    }
			
	          	/* unplot the affected beam and ping */
			if (found == MB_NO)
				status = mbedit_unplot_ping(i);
			status = mbedit_unplot_beam(i,j);

			/* reset the beam value */
			if (ping[i].bath[j] < 0.0)
			ping[i].bath[j] = 
				-ping[i].bath[j];
			status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[i].id,
				ping[i].time_i,ping[i].time_d,
				ping[i].navlon,ping[i].navlat,
				ping[i].speed,ping[i].heading,
				beams_bath,beams_amp,pixels_ss,
				ping[i].bath,
				ping[i].amp,
				ping[i].bathacrosstrack,
				ping[i].bathalongtrack,
				ping[i].ss,
				ping[i].ssacrosstrack,
				ping[i].ssalongtrack,
				comment,
				&error);
			if (verbose >= 1)
				{
				fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
					i,j,ping[i].bath[j]);
				fprintf(stderr," flagged\n");
				}

			/* replot the affected beams */
		 	found = MB_YES;
			beam_save = MB_YES;
			iping_save = i;
			jbeam_save = j;
			status = mbedit_plot_beam(i,j-1);
			status = mbedit_plot_beam(i,j);
			status = mbedit_plot_beam(i,j+1);
		
			/* if beam out of bounds replot label */
			if (ping[i].bath_x[j] < xmin
			    || ping[i].bath_x[j] > xmax
			    || ping[i].bath_y[j] < ymin
			    || ping[i].bath_y[j] > ymax)
			    replot_label = MB_YES;
			}
		}
	      }

	    /* replot affected ping */
	    if (found == MB_YES && *ngood > 0)
		    status = mbedit_plot_ping(i);
	    if (replot_label == MB_YES)
		    status = mbedit_plot_ping_label(i, MB_NO);
	    }

	  /* set some return values */
	  *nbuffer = nbuff;
	  *ngood = nlist;
	  *icurrent = current_id;
	  current = list[current_id];
	  }
		
	/* if no file open set failure status */
	else if (file_open == MB_NO)
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nlist;
		current_id = 0;
		*icurrent = current_id;
		current = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       nplt:        %d\n",*nplt);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_zap_outbounds(iping,
	plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
	nbuffer,ngood,icurrent,nplt)
int	iping;
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_action_zap_outbounds";
	int	status = MB_SUCCESS;
	int	found;
	char	comment[128];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iping:       %d\n",iping);
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
	    {

	    /* look for beams to be erased */
	    found = MB_NO;
	    for (j=0;j<beams_bath;j++)
	      {
	      if (ping[iping].bath[j] > 0.0
		    && (ping[iping].bath_x[j] < xmin
			|| ping[iping].bath_x[j] > xmax
			|| ping[iping].bath_y[j] < ymin
			|| ping[iping].bath_y[j] > ymax))
		    {
		    /* write edit to save file */
		    if (sofile_open == MB_YES)
			{
			mbedit_save_edit(
				ping[iping].time_d, 
				j, MBEDIT_FLAG);
			}
		    
		    /* unplot the affected beam and ping */
		    status = mbedit_unplot_ping(iping);
		    status = mbedit_unplot_beam(iping,j);

		    /* reset the beam value */
		    if (ping[iping].bath[j] > 0.0)
		    ping[iping].bath[j] = 
			    -ping[iping].bath[j];
		    status = mb_buffer_insert(verbose,
			    buff_ptr,imbio_ptr,ping[iping].id,
			    ping[iping].time_i,ping[iping].time_d,
			    ping[iping].navlon,ping[iping].navlat,
			    ping[iping].speed,ping[iping].heading,
			    beams_bath,beams_amp,pixels_ss,
			    ping[iping].bath,
			    ping[iping].amp,
			    ping[iping].bathacrosstrack,
			    ping[iping].bathalongtrack,
			    ping[iping].ss,
			    ping[iping].ssacrosstrack,
			    ping[iping].ssalongtrack,
			    comment,
			    &error);
		    if (verbose >= 1)
			    {
			    fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
				    i,j,ping[iping].bath[j]);
			    fprintf(stderr," flagged\n");
			    }

		    /* replot the affected beams */
		    found = MB_YES;
		    beam_save = MB_YES;
		    iping_save = iping;
		    jbeam_save = j;
		    status = mbedit_plot_beam(iping,j-1);
		    status = mbedit_plot_beam(iping,j);
		    status = mbedit_plot_beam(iping,j+1);
		    }
	      }

	   /* replot affected ping */
	   if (found == MB_YES && *ngood > 0)
		    {
		    status = mbedit_plot_ping(iping);
		    status = mbedit_plot_ping_label(iping, MB_NO);
		    }

	   /* set some return values */
	   *nbuffer = nbuff;
	   *ngood = nlist;
	   *icurrent = current_id;
	   current = list[current_id];
	   }
		
	/* if no file open set failure status */
	else if (file_open == MB_NO)
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nlist;
		current_id = 0;
		*icurrent = current_id;
		current = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       nplt:        %d\n",*nplt);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_bad_ping(
	plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
	nbuffer,ngood,icurrent,nplt)
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_action_bad_ping";
	int	status = MB_SUCCESS;
	char	comment[128];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		}

	/* check if a file has been opened 
		and a beam has been picked and saved */
	if (file_open == MB_YES && beam_save == MB_YES)
		{
		/* write edits to save file */
		if (sofile_open == MB_YES)
		    {
		    for (j=0;j<beams_bath;j++)
			if (ping[iping_save].bath[j] > 0.0)
			    mbedit_save_edit(
				ping[iping_save].time_d, 
				j, MBEDIT_FLAG);
		    }

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (j=0;j<beams_bath;j++)
			status = mbedit_unplot_beam(iping_save,j);

		/* flag beams in bad ping */
		for (j=0;j<beams_bath;j++)
			if (ping[iping_save].bath[j] > 0.0)
				ping[iping_save].bath[j] = 
					-ping[iping_save].bath[j];
		status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[iping_save].id,
				ping[iping_save].time_i,ping[iping_save].time_d,
				ping[iping_save].navlon,ping[iping_save].navlat,
				ping[iping_save].speed,ping[iping_save].heading,
				beams_bath,beams_amp,pixels_ss,
				ping[iping_save].bath,
				ping[iping_save].amp,
				ping[iping_save].bathacrosstrack,
				ping[iping_save].bathalongtrack,
				ping[iping_save].ss,
				ping[iping_save].ssacrosstrack,
				ping[iping_save].ssalongtrack,
				comment,
				&error);
		if (verbose >= 1)
			fprintf(stderr,"\nbeams in ping: %d flagged\n",
				iping_save);

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nlist;
		*icurrent = current_id;
		current = list[current_id];

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (j=0;j<beams_bath;j++)
			status = mbedit_plot_beam(iping_save,j);
			
		/* if ping has outbounds flag replot label */
		if (ping[iping_save].outbounds != MBEDIT_OUTBOUNDS_NONE)
			status = mbedit_plot_ping_label(iping_save, MB_NO);
		}

	/* if no file open or beam saved set failure status */
	else
		{
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       nplt:        %d\n",*nplt);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_good_ping(
	plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
	nbuffer,ngood,icurrent,nplt)
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_action_good_ping";
	int	status = MB_SUCCESS;
	char	comment[128];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		}

	/* check if a file has been opened 
		and a beam has been picked and saved */
	if (file_open == MB_YES && beam_save == MB_YES)
		{
		/* write edits to save file */
		if (sofile_open == MB_YES)
		    {
		    for (j=0;j<beams_bath;j++)
			if (ping[iping_save].bath[j] < 0.0)
			    mbedit_save_edit(
				ping[iping_save].time_d, 
				j, MBEDIT_UNFLAG);
		    }

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (j=0;j<beams_bath;j++)
			status = mbedit_unplot_beam(iping_save,j);

		/* flag beams in good ping */
		for (j=0;j<beams_bath;j++)
			if (ping[iping_save].bath[j] < 0.0)
				ping[iping_save].bath[j] = 
					-ping[iping_save].bath[j];
		status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[iping_save].id,
				ping[iping_save].time_i,ping[iping_save].time_d,
				ping[iping_save].navlon,ping[iping_save].navlat,
				ping[iping_save].speed,ping[iping_save].heading,
				beams_bath,beams_amp,pixels_ss,
				ping[iping_save].bath,
				ping[iping_save].amp,
				ping[iping_save].bathacrosstrack,
				ping[iping_save].bathalongtrack,
				ping[iping_save].ss,
				ping[iping_save].ssacrosstrack,
				ping[iping_save].ssalongtrack,
				comment,
				&error);
		if (verbose >= 1)
			fprintf(stderr,"\nbeams in ping: %d unflagged\n",
				iping_save);

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nlist;
		*icurrent = current_id;
		current = list[current_id];

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (j=0;j<beams_bath;j++)
			status = mbedit_plot_beam(iping_save,j);
			
		/* if ping has outbounds flag replot label */
		if (ping[iping_save].outbounds != MBEDIT_OUTBOUNDS_NONE)
			status = mbedit_plot_ping_label(iping_save, MB_NO);
		}

	/* if no file open or beam saved set failure status */
	else
		{
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       nplt:        %d\n",*nplt);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_left_ping(
	plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
	nbuffer,ngood,icurrent,nplt)
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_action_left_ping";
	int	status = MB_SUCCESS;
	char	comment[128];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		}

	/* check if a file has been opened 
		and a beam has been picked and saved */
	if (file_open == MB_YES && beam_save == MB_YES)
		{
		/* write edits to save file */
		if (sofile_open == MB_YES)
		    {
		    for (j=0;j<=jbeam_save;j++)
			if (ping[iping_save].bath[j] > 0.0)
			    mbedit_save_edit(
				ping[iping_save].time_d, 
				j, MBEDIT_FLAG);
		    }

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (j=0;j<beams_bath;j++)
			status = mbedit_unplot_beam(iping_save,j);

		/* flag beams to left of picked beam */
		for (j=0;j<=jbeam_save;j++)
			if (ping[iping_save].bath[j] > 0.0)
				ping[iping_save].bath[j] = 
					-ping[iping_save].bath[j];
		status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[iping_save].id,
				ping[iping_save].time_i,ping[iping_save].time_d,
				ping[iping_save].navlon,ping[iping_save].navlat,
				ping[iping_save].speed,ping[iping_save].heading,
				beams_bath,beams_amp,pixels_ss,
				ping[iping_save].bath,
				ping[iping_save].amp,
				ping[iping_save].bathacrosstrack,
				ping[iping_save].bathalongtrack,
				ping[iping_save].ss,
				ping[iping_save].ssacrosstrack,
				ping[iping_save].ssalongtrack,
				comment,
				&error);
		if (verbose >= 1)
			fprintf(stderr,"\nbeams in ping: %d left of beam: %d flagged\n",
				iping_save,jbeam_save);

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nlist;
		*icurrent = current_id;
		current = list[current_id];

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (j=0;j<beams_bath;j++)
			status = mbedit_plot_beam(iping_save,j);
			
		/* if ping has outbounds flag replot label */
		if (ping[iping_save].outbounds != MBEDIT_OUTBOUNDS_NONE)
			status = mbedit_plot_ping_label(iping_save, MB_NO);
		}

	/* if no file open or beam saved set failure status */
	else
		{
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       nplt:        %d\n",*nplt);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_right_ping(
	plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
	nbuffer,ngood,icurrent,nplt)
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_action_right_ping";
	int	status = MB_SUCCESS;
	char	comment[128];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		}

	/* check if a file has been opened 
		and a beam has been picked and saved */
	if (file_open == MB_YES && beam_save == MB_YES)
		{
		/* write edits to save file */
		if (sofile_open == MB_YES)
		    {
		    for (j=jbeam_save;j<beams_bath;j++)
			if (ping[iping_save].bath[j] > 0.0)
			    mbedit_save_edit(
				ping[iping_save].time_d, 
				j, MBEDIT_FLAG);
		    }

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (j=0;j<beams_bath;j++)
			status = mbedit_unplot_beam(iping_save,j);

		/* flag beams to right of picked beam */
		for (j=jbeam_save;j<beams_bath;j++)
			if (ping[iping_save].bath[j] > 0.0)
				ping[iping_save].bath[j] = 
					-ping[iping_save].bath[j];
		status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[iping_save].id,
				ping[iping_save].time_i,ping[iping_save].time_d,
				ping[iping_save].navlon,ping[iping_save].navlat,
				ping[iping_save].speed,ping[iping_save].heading,
				beams_bath,beams_amp,pixels_ss,
				ping[iping_save].bath,
				ping[iping_save].amp,
				ping[iping_save].bathacrosstrack,
				ping[iping_save].bathalongtrack,
				ping[iping_save].ss,
				ping[iping_save].ssacrosstrack,
				ping[iping_save].ssalongtrack,
				comment,
				&error);
		if (verbose >= 1)
			fprintf(stderr,"\nbeams in ping: %d right of beam: %d flagged\n",
				iping_save,jbeam_save);

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nlist;
		*icurrent = current_id;
		current = list[current_id];

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (j=0;j<beams_bath;j++)
			status = mbedit_plot_beam(iping_save,j);
			
		/* if ping has outbounds flag replot label */
		if (ping[iping_save].outbounds != MBEDIT_OUTBOUNDS_NONE)
			status = mbedit_plot_ping_label(iping_save, MB_NO);
		}

	/* if no file open or beam saved set failure status */
	else
		{
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       nplt:        %d\n",*nplt);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_zero_ping(
	plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
	nbuffer,ngood,icurrent,nplt)
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_action_zero_ping";
	int	status = MB_SUCCESS;
	char	comment[128];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		}

	/* check if a file has been opened 
		and a beam has been picked and saved */
	if (file_open == MB_YES && beam_save == MB_YES)
		{
		/* write edits to save file */
		if (sofile_open == MB_YES)
		    {
		    for (j=0;j<beams_bath;j++)
			if (ping[iping_save].bath[j] != 0.0)
			    mbedit_save_edit(
				ping[iping_save].time_d, 
				j, MBEDIT_ZERO);
		    }

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (j=0;j<beams_bath;j++)
			status = mbedit_unplot_beam(iping_save,j);

		/* zero beams in bad ping */
		for (j=0;j<beams_bath;j++)
			{
			ping[iping_save].bath[j] = 0.0;
			ping[iping_save].bathacrosstrack[j] = 0.0;
			ping[iping_save].bathalongtrack[j] = 0.0;
			}
		for (j=0;j<beams_amp;j++)
			ping[iping_save].amp[j] = 0.0;
		status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[iping_save].id,
				ping[iping_save].time_i,ping[iping_save].time_d,
				ping[iping_save].navlon,ping[iping_save].navlat,
				ping[iping_save].speed,ping[iping_save].heading,
				beams_bath,beams_amp,pixels_ss,
				ping[iping_save].bath,
				ping[iping_save].amp,
				ping[iping_save].bathacrosstrack,
				ping[iping_save].bathalongtrack,
				ping[iping_save].ss,
				ping[iping_save].ssacrosstrack,
				ping[iping_save].ssalongtrack,
				comment,
				&error);
		if (verbose >= 1)
			fprintf(stderr,"\nbeams in ping: %d zeroed\n",
				iping_save);

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nlist;
		*icurrent = current_id;
		current = list[current_id];

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (j=0;j<beams_bath;j++)
			status = mbedit_plot_beam(iping_save,j);
			
		/* if ping has outbounds flag replot label */
		if (ping[iping_save].outbounds != MBEDIT_OUTBOUNDS_NONE)
			status = mbedit_plot_ping_label(iping_save, MB_NO);
		}

	/* if no file open or beam saved set failure status */
	else
		{
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       nplt:        %d\n",*nplt);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_set_output_file(output_file)
char	*output_file;
{
	/* local variables */
	char	*function_name = "mbedit_set_output_file";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       output file: %s\n",output_file);
		}

	/* copy output file name */
	if (output_file != NULL)
		{
		strcpy(ofile,output_file);
		ofile_defined = MB_YES;
		}
	else
		{
		ofile_defined = MB_NO;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_open_file(file,form, savemode)
char	*file;
int	form;
int	savemode;
{
	/* local variables */
	char	*function_name = "mbedit_open_file";
	int	status = MB_SUCCESS;
	char	*mb_suffix;
	char	*sb_suffix;
	int	mb_len;
	int	sb_len;
	char	*start;
	struct stat file_status;
	int	fstat;
	char	command[256];
	int	i;

	/* time, user, host variables */
	long int	right_now;
	char	date[25], user[128], *user_ptr, host[128];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       file:        %s\n",file);
		fprintf(stderr,"dbg2       format:      %d\n",form);
		fprintf(stderr,"dbg2       savemode:    %d\n",savemode);
		}

	/* get filenames */
	strcpy(ifile,file);
	if (ofile_defined == MB_NO 
		&& output_mode == MBEDIT_OUTPUT_OUTPUT)
		{
		/* look for MB suffix convention */
		if ((mb_suffix = strstr(ifile,".mb")) != NULL)
			mb_len = strlen(mb_suffix);

		/* look for SeaBeam suffix convention */
		if ((sb_suffix = strstr(ifile,".rec")) != NULL)
			sb_len = strlen(sb_suffix);

		/* if MB suffix convention used keep it */
		if (mb_len >= 4 && mb_len <= 6)
			{
			/* get the output filename */
			strncpy(ofile,"\0",128);
			strncpy(ofile,ifile,
				strlen(ifile)-mb_len);
			if (strstr(ofile, "_") != NULL)
				strcat(ofile, "e");
			else
				strcat(ofile,"_e");
			strcat(ofile,mb_suffix);
			}
			
		/* else look for ".rec" format 41 file */
		else if (sb_len == 4 && form == 41)
			{
			/* get the output filename */
			strncpy(ofile,"\0",128);
			strncpy(ofile,ifile,
				strlen(ifile)-sb_len);
			strcat(ofile,"_e.mb41");
			}

		/* else just at ".ed" to file name */
		else
			{
			strcpy(ofile,ifile);
			strcat(ofile,".ed");
			}
		}
	format = form;

	/* initialize reading the input multibeam file */
	if ((status = mb_read_init(
		verbose,ifile,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",ifile);
		status = MB_FAILURE;
		do_error_dialog("Unable to open input file.", 
				"You may not have read", 
				"permission in this directory!");
		return(status);
		}

	/* initialize writing the output multibeam file */
	if (output_mode == MBEDIT_OUTPUT_OUTPUT)
		{
		if ((status = mb_write_init(
			verbose,ofile,format,&ombio_ptr,
			&beams_bath,&beams_amp,
			&pixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
			fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",ofile);
			status = MB_FAILURE;
			do_error_dialog("Unable to open output file.", 
					"You may not have write", 
					"permission in this directory!");
			return(status);
			}
		}
	else
		ombio_ptr = NULL;

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,beams_bath*sizeof(double),&bath,&error);
	status = mb_malloc(verbose,beams_amp*sizeof(double),&amp,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathacrosstrack,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathalongtrack,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ssacrosstrack,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ssalongtrack,&error);
	for (i=0;i<MBEDIT_MAX_PINGS;i++)
		{
		ping[i].bath = NULL;
		ping[i].amp = NULL;
		ping[i].bathacrosstrack = NULL;
		ping[i].bathalongtrack = NULL;
		ping[i].ss = NULL;
		ping[i].ssacrosstrack = NULL;
		ping[i].ssalongtrack = NULL;
		ping[i].bath_x = NULL;
		ping[i].bath_y = NULL;
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bath,&error);
		status = mb_malloc(verbose,beams_amp*sizeof(double),
			&ping[i].amp,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bathacrosstrack,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bathalongtrack,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ping[i].ss,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ping[i].ssacrosstrack,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ping[i].ssalongtrack,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(int),
			&ping[i].bath_x,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(int),
			&ping[i].bath_y,&error);
		}
	status = mb_malloc(verbose,beams_bath*MBEDIT_MAX_PINGS*sizeof(double),
			&bathlist,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize the buffer */
	status = mb_buffer_init(verbose,&buff_ptr,&error);
	nbuff = 0;

	/* write comments to beginning of output file */
	if (output_mode == MBEDIT_OUTPUT_OUTPUT)
		{
		kind = MB_DATA_COMMENT;
		strncpy(comment,"\0",256);
		sprintf(comment,"Bathymetry data edited interactively using program %s version %s",
			program_name,rcs_id);
		status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"MB-system Version %s",MB_VERSION);
		status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		right_now = time((long *)0);
		strncpy(date,"\0",25);
		right_now = time((long *)0);
		strncpy(date,ctime(&right_now),24);
		if ((user_ptr = getenv("USER")) == NULL)
			user_ptr = getenv("LOGNAME");
		if (user_ptr != NULL)
			strcpy(user,user_ptr);
		else
			strcpy(user, "unknown");
		gethostname(host,128);
		strncpy(comment,"\0",256);
		sprintf(comment,"Run by user <%s> on cpu <%s> at <%s>",
			user,host,date);
		status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"Control Parameters:");
		status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  MBIO data format:   %d",format);
		status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  Input file:         %s",ifile);
		status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  Output file:        %s",ofile);
		status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment," ");
		status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
		
	/* now deal with old edit save file */
	sifile_open = MB_NO;
	if (status == MB_SUCCESS && savemode == MB_YES)
		{
		/* check if old edit save file exists */
		sprintf(sofile, "%s.mbesf", ifile);
		fstat = stat(sofile, &file_status);
		if (fstat == 0 
		    && (file_status.st_mode & S_IFMT) != S_IFDIR)
		    {
		    /* get temporary file name */
		    strcpy(sifile, sofile);
		    if ((start = (char *) strrchr(sifile, '/'))
			!= NULL)
			start++;
		    else
			start = sifile;
		    strcpy(start, "mbedit_tmp.mbesf");

		    /* copy old edit save file to tmp file */
		    sprintf(command, "cp %s %s\n", 
			sofile, sifile);
		    system(command);
		    
		    /* open the old edit file */
		    if ((sifp = fopen(sifile,"r")) != NULL)
			sifile_open = MB_YES;
		    else
			{
			sifile_open = MB_NO;
			fprintf(stderr, "\nUnable to open old edit save file %s\n", 
			    sifile);
			do_error_dialog("Unable to copy and open old edit", 
					"save file. You may not have write", 
					"or read permission in this directory!");
			}
		    }
		}
		
	/* now deal with new edit save file */
	sofile_open = MB_NO;
	if (status == MB_SUCCESS 
		&& output_mode == MBEDIT_OUTPUT_OUTPUT)
		{
		/* get edit save file exists */
		sprintf(sofile, "%s.mbesf", ifile);
		    
		/* open the edit save file */
		if ((sofp = fopen(sofile,"w")) != NULL)
		    sofile_open = MB_YES;
		else
		    {
		    sofile_open = MB_NO;
		    fprintf(stderr, "\nUnable to open new edit save file %s\n", 
			sofile);
		    do_error_dialog("Unable to open new edit save file.", 
				    "You may not have write", 
				    "permission in this directory!");
		    }
		}

	/* if we got here we must have succeeded */
	if (verbose >= 0)
		{
		fprintf(stderr,"\nMultibeam File <%s> initialized for reading\n",ifile);
		if (output_mode == MBEDIT_OUTPUT_OUTPUT)
			fprintf(stderr,"Multibeam File <%s> initialized for writing\n",ofile);
		fprintf(stderr,"Multibeam Data Format ID: %d\n",format);
		}
	file_open = MB_YES;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_close_file()
{
	/* local variables */
	char	*function_name = "mbedit_close_file";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* close the files */
	status = mb_buffer_close(verbose,&buff_ptr,imbio_ptr,&error);
	status = mb_close(verbose,&imbio_ptr,&error);
	if (ombio_ptr != NULL)
		status = mb_close(verbose,&ombio_ptr,&error);
	ofile_defined = MB_NO;
	if (sifile_open == MB_YES)
	    {
	    fclose(sifp);
	    sifile_open = MB_NO;
	    }
	if (sofile_open == MB_YES)
	    {
	    fclose(sofp);
	    sofile_open = MB_NO;
	    }

	/* deallocate memory for data arrays */
	mb_free(verbose,&bath,&error);
	mb_free(verbose,&amp,&error);
	mb_free(verbose,&bathacrosstrack,&error);
	mb_free(verbose,&bathalongtrack,&error);
	mb_free(verbose,&ss,&error);
	mb_free(verbose,&ssacrosstrack,&error);
	mb_free(verbose,&ssalongtrack,&error);
	for (i=0;i<MBEDIT_MAX_PINGS;i++)
		{
		mb_free(verbose,&ping[i].bath,&error);
		mb_free(verbose,&ping[i].amp,&error);
		mb_free(verbose,&ping[i].bathacrosstrack,&error);
		mb_free(verbose,&ping[i].bathalongtrack,&error);
		mb_free(verbose,&ping[i].ss,&error);
		mb_free(verbose,&ping[i].ssacrosstrack,&error);
		mb_free(verbose,&ping[i].ssalongtrack,&error);
		mb_free(verbose,&ping[i].bath_x,&error);
		mb_free(verbose,&ping[i].bath_y,&error);
		}
	mb_free(verbose, &bathlist, &error);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* if we got here we must have succeeded */
	if (verbose >= 0)
		{
		fprintf(stderr,"\nMultibeam Input File <%s> closed\n",ifile);
		if (output_mode == MBEDIT_OUTPUT_OUTPUT)
			fprintf(stderr,"Multibeam Output File <%s> closed\n",ofile);
		fprintf(stderr,"%d data records loaded\n",nload_total);
		fprintf(stderr,"%d data records dumped\n",ndump_total);
		
		}
	file_open = MB_NO;
	nload_total = 0;
	ndump_total = 0;
	
	/* turn file button on */
	do_filebutton_on();

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_dump_data(hold_size,ndumped,nbuffer)
int	hold_size;
int	*ndumped;
int	*nbuffer;
{
	/* local variables */
	char	*function_name = "mbedit_dump_data";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       hold_size:   %d\n",hold_size);
		}

	/* dump or clear data from the buffer */
	ndump = 0;
	if (nbuff > 0)
		{
		if (output_mode == MBEDIT_OUTPUT_OUTPUT)
			{
			/* turn message on */
			do_message_on("MBedit is dumping data...");

			status = mb_buffer_dump(verbose,
					buff_ptr,ombio_ptr,
					hold_size,&ndump,&nbuff,
					&error);
			}
		else
			{
			/* turn message on */
			do_message_on("MBedit is clearing data...");

			status = mb_buffer_clear(verbose,
					buff_ptr,imbio_ptr,
					hold_size,&ndump,&nbuff,
					&error);
			}

		/* turn message off */
		do_message_off();
		}
	*ndumped = ndump;
	ndump_total += ndump;

	/* reset current data pointer */
	if (ndump > 0)
		current = current - ndump;
	if (current < 0)
		current == 0;
	if (current > nbuff - 1)
		current == nbuff - 1;
	*nbuffer = nbuff;

	/* flag lack of indexing */
	nlist = 0;

	/* print out information */
	if (verbose >= 0)
		{
		if (output_mode == MBEDIT_OUTPUT_OUTPUT)
			fprintf(stderr,"\n%d data records dumped to output file <%s>\n",
				*ndumped,ofile);
		else
			fprintf(stderr,"\n%d data records dumped from buffer\n",
				*ndumped);
		fprintf(stderr,"%d data records remain in buffer\n",*nbuffer);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndumped:    %d\n",*ndumped);
		fprintf(stderr,"dbg2       nbuffer:    %d\n",*nbuffer);
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_load_data(buffer_size,nloaded,nbuffer,ngood,icurrent)
int	buffer_size;
int	*nloaded;
int	*nbuffer;
int	*ngood;
int	*icurrent;
{
	/* local variables */
	char	*function_name = "mbedit_load_data";
	int	status = MB_SUCCESS;
	int	found;
	double	stime_d;
	int	sbeam;
	int	saction;
	int	i;
	int	start;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		}
		
	/* turn message on */
	do_message_on("MBedit is loading data...");

	/* load data into buffer */
	status = mb_buffer_load(verbose,buff_ptr,imbio_ptr,buffer_size,
			&nload,&nbuff,&error);
	*nbuffer = nbuff;
	*nloaded = nload;
	nload_total += nload;

	/* set up index of bathymetry pings */
	nlist = 0;
	start = 0;
	list[0] = 0;
	if (status == MB_SUCCESS) 
	do
		{
		status = mb_buffer_get_next_data(verbose,
			buff_ptr,imbio_ptr,start,&id,
			time_i,&time_d,
			&navlon,&navlat,
			&speed,&heading,
			&beams_bath,&beams_amp,&pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			&error);
		if (status == MB_SUCCESS)
			{
			start = id + 1;
			list[nlist] = id;
			nlist++;

			/* print output debug statements */
			if (verbose >= 5)
				{
				fprintf(stderr,"\ndbg5  Next good data found in function <%s>:\n",
					function_name);
				fprintf(stderr,"dbg5       list[%d]: %d %d\n",
					nlist-1,list[nlist-1],
					list[nlist-1] + ndump_total);
				}
			}
		}
	while (status == MB_SUCCESS);
	*ngood = nlist;

	/* define success */
	if (nlist > 0)
		{
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		}

	/* find index of current ping */
	current_id = 0;
	for (i=0;i<nlist;i++)
		{
		if (list[i] <= current)
			current_id = i;
		}
	*icurrent = current_id;
	current = list[current_id];
	
	/* if desired apply saved edits */
	if (sifile_open == MB_YES)
		{
		/* reset message */
		do_message_on("MBedit is applying saved edits...");
		
		/* rewinds saved edit file */
		rewind(sifp);
		
		/* loop over reading saved edits */
		while (mbedit_retrieve_edit(&stime_d, &sbeam, &saction) 
		    == MB_SUCCESS)
		    {
		    /* find ping of interest if possible */
		    found = MB_NO;
		    for (i = 0; i < nlist && found == MB_NO ; i++)
			{
			status = mb_buffer_get_next_data(verbose,
				buff_ptr,imbio_ptr,list[i],&id,
				time_i,&time_d,
				&navlon,&navlat,
				&speed,&heading,
				&beams_bath,&beams_amp,&pixels_ss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				&error);
			if (time_d == stime_d && sbeam >= 0 
			    && sbeam < beams_bath)
			    {
			    /* apply edit */
			    found = MB_YES;
			    if (saction == MBEDIT_FLAG)
				bath[sbeam] = -fabs(bath[sbeam]);
			    else if (saction == MBEDIT_UNFLAG)
				bath[sbeam] = fabs(bath[sbeam]);
			    else if (saction == MBEDIT_ZERO)
				bath[sbeam] = 0.0;
				
			    /* write saved edit to current edit save file */
			    if (sofile_open == MB_YES)
				mbedit_save_edit(stime_d, sbeam, saction);
				
			    /* reinsert data */
			    status = mb_buffer_insert(verbose,
				    buff_ptr, imbio_ptr, id,
				    time_i, time_d,
				    navlon, navlat, speed, heading,
				    beams_bath, beams_amp, pixels_ss,
				    bath, amp, bathacrosstrack, bathalongtrack,
				    ss, ssacrosstrack, ssalongtrack,
				    comment,
				    &error);
			    }
			}
		    }
		}
		
	/* turn message off */
	do_message_off();

	/* print out information */
	if (verbose >= 0)
		{
		fprintf(stderr,"\n%d data records loaded from input file <%s>\n",
			*nloaded,ifile);
		fprintf(stderr,"%d data records now in buffer\n",*nbuffer);
		fprintf(stderr,"%d editable survey data records now in buffer\n",*ngood);
		fprintf(stderr,"Current data record index:  %d\n",
			current_id);
		fprintf(stderr,"Current data record:        %d\n",
			list[current_id]);
		fprintf(stderr,"Current global data record: %d\n",
			list[current_id] + ndump_total);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nloaded:    %d\n",*nloaded);
		fprintf(stderr,"dbg2       nbuffer:    %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:      %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:   %d\n",*icurrent);
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_clear_screen()
{
	/* local variables */
	char	*function_name = "mbedit_clear_screen";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* clear screen */
	xg_fillrectangle(mbedit_xgid,borders[0],borders[2],
		borders[1]-borders[0],borders[3]-borders[2],
		pixel_values[WHITE],XG_SOLIDLINE);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,
		    nplt,autoscale)
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*nplt;
int	autoscale;
{
	/* local variables */
	char	*function_name = "mbedit_plot_all";
	int	status = MB_SUCCESS;
	int	i, j, k, ii;
	int	jbeam_cen;
	int	nbathsum,  nbathlist;
	double	bathsum, bathmean, bathmedian;
	double	xtrack_max;
	int	ndec, maxx;
	double	dxscale, dyscale;
	double	dx_width, dy_height;
	int	nx_int, ny_int;
	int	x_int, y_int;
	int	xx, vx, yy, vy;
	int	swidth, sascent, sdescent;
	int	xcen;
	int	y, dy, first, xold, yold;
	char	string[128];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       nplt:        %d\n",nplt);
		fprintf(stderr,"dbg2       autoscale:   %d\n",autoscale);
		}

	/* set scales and tick intervals */
	plot_width = plwd;
	exager = exgr;
	x_interval = xntrvl;
	y_interval = yntrvl;
	show_flagged = sh_flggd;

	/* figure out which pings to plot */
	plot_size = plt_size;
	if (current_id + plot_size > nlist)
		nplot = nlist - current_id;
	else
		nplot = plot_size;
	*nplt = nplot;

	/* get data into ping arrays and find median depth value */
	bathsum = 0.0;
	nbathsum = 0;
	nbathlist = 0;
	xtrack_max = 0.0;
	ii = current;
	for (i=0;i<nplot;i++)
		{
		status = mb_buffer_get_next_data(verbose,
			buff_ptr,imbio_ptr,ii,&ping[i].id,
			ping[i].time_i,&ping[i].time_d,
			&ping[i].navlon,&ping[i].navlat,
			&ping[i].speed,&ping[i].heading,
			&beams_bath,&beams_amp,&pixels_ss,
			ping[i].bath,ping[i].amp,
			ping[i].bathacrosstrack,ping[i].bathalongtrack,
			ping[i].ss,ping[i].ssacrosstrack,ping[i].ssalongtrack,
			&error);
		if (status == MB_SUCCESS)
			{
			ping[i].record = ping[i].id + ndump_total;
			ping[i].outbounds = MBEDIT_OUTBOUNDS_NONE;
			for (j=0;j<beams_bath;j++)
				{
				if (ping[i].bath[j] > 0.0)
					{
					bathsum += ping[i].bath[j];
					nbathsum++;
					bathlist[nbathlist] = ping[i].bath[j];
					nbathlist++;
					xtrack_max = MAX(xtrack_max, 
						fabs(ping[i].bathacrosstrack[j]));
					}
				}
			}
		ii = ping[i].id + 1;
		}
		
	/* if not enough information in unflagged bathymetry look
	    into the flagged bathymetry */
	if (nbathlist <= 0 || xtrack_max <= 0.0)
		{
		for (i=0;i<nplot;i++)
			{
			for (j=0;j<beams_bath;j++)
				{
				if (ping[i].bath[j] < 0.0)
					{
					bathsum += fabs(ping[i].bath[j]);
					nbathsum++;
					bathlist[nbathlist] = fabs(ping[i].bath[j]);
					nbathlist++;
					xtrack_max = MAX(xtrack_max, 
						fabs(ping[i].bathacrosstrack[j]));
					}
				}
			}
		}
	if (nbathsum > 0)
		bathmean = bathsum/nbathsum;
	if (nbathlist > 0)
		{
		sort(nbathlist, bathlist);
		bathmedian = bathlist[nbathlist/2];
		}
		
	/* if autoscale on reset plot width */
	if (autoscale == MB_YES && xtrack_max > 0.0)
		{
		plot_width = 2.4 * xtrack_max;
		ndec = MAX(1, (int) log10((double) plot_width));
		maxx = 1;
		for (i=0;i<ndec;i++)
			maxx = maxx * 10;
		maxx = (plot_width / maxx + 1) * maxx;
		do_reset_scale_x(plot_width, maxx);
		}

	/* print out information */
	jbeam_cen = beams_bath/2;    
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2       %d data records set for plotting (%d desired)\n",
			nplot,plot_size);
		for (i=0;i<nplot;i++)
			{
			fprintf(stderr,"dbg2       %4d %4d %4d  %d/%d/%d %2.2d:%2.2d:%2.2d.%6.6d  %10.3f\n",
				i,ping[i].id,ping[i].record,
				ping[i].time_i[1],ping[i].time_i[2],
				ping[i].time_i[0],ping[i].time_i[3],
				ping[i].time_i[4],ping[i].time_i[5],
				ping[i].time_i[6],
				ping[i].bath[jbeam_cen]);
			}
		}

	/* clear screen */
	xg_fillrectangle(mbedit_xgid,borders[0],borders[2],
		borders[1]-borders[0],borders[3]-borders[2],
		pixel_values[WHITE],XG_SOLIDLINE);

	/* set scaling */
	xcen = xmin + (xmax - xmin)/2;
	dy = (ymax - ymin)/plot_size;
	xscale = 100*plot_width/(xmax - xmin);
	yscale = (xscale*100)/exager;
	dxscale = 100.0/xscale;
	dyscale = 100.0/yscale;

	/* plot top label */
	sprintf(string,"Vertical Exageration: %4.2f",(exager/100.));
	xg_justify(mbedit_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(mbedit_xgid,xcen-swidth/2,
		ymin-margin/2+sascent,string,
		pixel_values[BLACK],XG_SOLIDLINE);
	sprintf(string,"Acrosstrack Distances and Depths in Meters");
	xg_justify(mbedit_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(mbedit_xgid,xcen-swidth/2,
		ymin-margin/2+2*(sascent+sdescent),string,
		pixel_values[BLACK],XG_SOLIDLINE);

	/* plot filename */
	sprintf(string,"Current Data File:");
	xg_justify(mbedit_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(mbedit_xgid,50,
		ymin-margin/2-3*sascent/2,string,pixel_values[BLACK],XG_SOLIDLINE);
	xg_drawstring(mbedit_xgid,50+swidth,
		ymin-margin/2-3*sascent/2,ifile,pixel_values[BLACK],XG_SOLIDLINE);

	/* plot scale bars */
	dx_width = (xmax - xmin)/dxscale;
	nx_int = 0.5*dx_width/x_interval + 1;
	x_int = x_interval*dxscale;
	xg_drawline(mbedit_xgid,xmin,ymax,xmax,ymax,
		pixel_values[BLACK],XG_SOLIDLINE);
	xg_drawline(mbedit_xgid,xmin,ymin,xmax,ymin,
		pixel_values[BLACK],XG_SOLIDLINE);
	for (i=0;i<nx_int;i++)
		{
		xx = i*x_int;
		vx = i*x_interval;
		xg_drawline(mbedit_xgid,xcen-xx,ymin,xcen-xx,ymax,
			pixel_values[BLACK],XG_DASHLINE);
		xg_drawline(mbedit_xgid,xcen+xx,ymin,xcen+xx,ymax,
			pixel_values[BLACK],XG_DASHLINE);
		sprintf(string,"%1d",vx);
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,xcen+xx-swidth/2,
			ymax+sascent+5,string,
			pixel_values[BLACK],XG_SOLIDLINE);
		xg_drawstring(mbedit_xgid,xcen-xx-swidth/2,
			ymax+sascent+5,string,
			pixel_values[BLACK],XG_SOLIDLINE);
		}
	dy_height = (ymax - ymin)/dyscale;
	ny_int = dy_height/y_interval + 1;
	y_int = y_interval*dyscale;
	xg_drawline(mbedit_xgid,xmin,ymin,xmin,ymax,
		pixel_values[BLACK],XG_SOLIDLINE);
	xg_drawline(mbedit_xgid,xmax,ymin,xmax,ymax,
		pixel_values[BLACK],XG_SOLIDLINE);
	for (i=0;i<ny_int;i++)
		{
		yy = i*y_int;
		vy = i*y_interval;
		xg_drawline(mbedit_xgid,xmin,ymax-yy,xmax,ymax-yy,
			pixel_values[BLACK],XG_DASHLINE);
		sprintf(string,"%1d",vy);
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,xmax+5,
			ymax-yy+sascent/2,string,
			pixel_values[BLACK],XG_SOLIDLINE);
		}

	/* plot pings */
	for (i=0;i<nplot;i++)
		{
		/* set beam plotting locations */
		y = ymax - dy / 2 - i * dy;
		ping[i].label_x = 5 * margin - 5;
		ping[i].label_y = y;
		for (j=0;j<beams_bath;j++)
			{
			if (ping[i].bath[j] != 0.0)
				{
				ping[i].bath_x[j] = xcen 
					+ dxscale*ping[i].bathacrosstrack[j];
				ping[i].bath_y[j] = y + dyscale*
					(fabs((double)ping[i].bath[j]) 
					- bathmedian);
				}
			else
				{
				ping[i].bath_x[j] = 0;
				ping[i].bath_y[j] = 0;
				}
			}

		/* plot the beams */
		for (j=0;j<beams_bath;j++)
			status = mbedit_plot_beam(i,j);

		/* plot the ping profile */
		status = mbedit_plot_ping(i);

		/* set and draw info string */
		mbedit_plot_ping_label(i, MB_YES);
		}

	/* set status */
	if (nplot > 0)
		status = MB_SUCCESS;
	else
		status = MB_FAILURE;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nplot:       %d\n",*nplt);
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_plot_beam(iping,jbeam)
int	iping;
int	jbeam;
{
	/* local variables */
	char	*function_name = "mbedit_plot_beam";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iping:       %d\n",iping);
		fprintf(stderr,"dbg2       jbeam:       %d\n",jbeam);
		}

	/* plot the beam */
	if (jbeam >= 0 && jbeam < beams_bath)
		{
		if (ping[iping].bath[jbeam] > 0)
			xg_fillrectangle(mbedit_xgid, 
				ping[iping].bath_x[jbeam]-2, 
				ping[iping].bath_y[jbeam]-2, 4, 4, 
				pixel_values[BLACK],XG_SOLIDLINE);
		else if (ping[iping].bath[jbeam] < 0)
			xg_drawrectangle(mbedit_xgid, 
				ping[iping].bath_x[jbeam]-2, 
				ping[iping].bath_y[jbeam]-2, 4, 4, 
				pixel_values[RED],XG_SOLIDLINE);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_plot_ping(iping)
int	iping;
{
	/* local variables */
	char	*function_name = "mbedit_plot_ping";
	int	status = MB_SUCCESS;
	int	j;
	int	first, last_flagged;
	int	xold, yold;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iping:       %d\n",iping);
		}

	/* plot the ping profile */
	first = MB_YES;
	last_flagged = MB_NO;
	for (j=0;j<beams_bath;j++)
		{
		if (show_flagged == MB_YES 
			&& ping[iping].bath[j] < 0.0 && first == MB_YES)
			{
			first = MB_NO;
			last_flagged = MB_YES;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
			}
		else if (ping[iping].bath[j] > 0.0 && first == MB_YES)
			{
			first = MB_NO;
			last_flagged = MB_NO;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
			}
		else if (last_flagged == MB_NO 
			&& ping[iping].bath[j] > 0.0)
			{
			xg_drawline(mbedit_xgid,xold,yold,
					ping[iping].bath_x[j],
					ping[iping].bath_y[j],
					pixel_values[BLACK],XG_SOLIDLINE);
			last_flagged = MB_NO;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
			}
		else if (ping[iping].bath[j] > 0.0)
			{
			xg_drawline(mbedit_xgid,xold,yold,
					ping[iping].bath_x[j],
					ping[iping].bath_y[j],
					pixel_values[RED],XG_SOLIDLINE);
			last_flagged = MB_NO;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
			}
		else if (show_flagged == MB_YES 
			&& ping[iping].bath[j] < 0.0)
			{
			xg_drawline(mbedit_xgid,xold,yold,
					ping[iping].bath_x[j],
					ping[iping].bath_y[j],
					pixel_values[RED],XG_SOLIDLINE);
			last_flagged = MB_YES;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_plot_ping_label(iping, save)
int	iping;
int	save;
{
	/* local variables */
	char	*function_name = "mbedit_plot_ping_label";
	int	status = MB_SUCCESS;
	int	sascent, sdescent, swidth;
	char	string[50];
	int	j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iping:       %d\n",iping);
		fprintf(stderr,"dbg2       save:        %d\n",save);
		}

	/* get the ping outbounds value */
	ping[iping].outbounds = MBEDIT_OUTBOUNDS_NONE;
	for (j=0;j<beams_bath;j++)
		{
		if (ping[iping].bath[j] != 0.0
		    && (ping[iping].bath_x[j] < xmin
		    || ping[iping].bath_x[j] > xmax
		    || ping[iping].bath_y[j] < ymin
		    || ping[iping].bath_y[j] > ymax))
		    {
		    if (ping[iping].bath[j] > 0.0)
			ping[iping].outbounds 
			    = MBEDIT_OUTBOUNDS_UNFLAGGED;
		    else if (ping[iping].bath[j] < 0.0
			&& ping[iping].outbounds != MBEDIT_OUTBOUNDS_UNFLAGGED)
			ping[iping].outbounds 
			    = MBEDIT_OUTBOUNDS_FLAGGED;
		    }
		}

	/* set info string */
	sprintf(string,"%5d %2d/%2d/%4d %2.2d:%2.2d:%2.2d.%3.3d %10.3f",
		ping[iping].record,
		ping[iping].time_i[1],ping[iping].time_i[2],
		ping[iping].time_i[0],ping[iping].time_i[3],
		ping[iping].time_i[4],ping[iping].time_i[5],
		(int)(0.001 * ping[iping].time_i[6]),
		ping[iping].bath[beams_bath/2]);
	xg_justify(mbedit_xgid,string,&swidth,&sascent,&sdescent);

	/* save string to show last ping seen at end of program */
	if (save == MB_YES)
		strcpy(last_ping,string);

	/* unplot the ping label */
	xg_fillrectangle(mbedit_xgid,
		ping[iping].label_x - swidth - 21, 
		ping[iping].label_y - sascent - 1, 
		swidth + 22, 
		sascent + sdescent + 2, 
		pixel_values[WHITE],XG_SOLIDLINE);
		
	/* plot the ping label */
	if (ping[iping].outbounds == MBEDIT_OUTBOUNDS_UNFLAGGED)
	    {
	    xg_fillrectangle(mbedit_xgid,
		    ping[iping].label_x - swidth, 
		    ping[iping].label_y - sascent, 
		    swidth, 
		    sascent + sdescent, 
		    pixel_values[RED],XG_SOLIDLINE);
	    ping[iping].zap_x1 = ping[iping].label_x - swidth - 20;
	    ping[iping].zap_x2 =  ping[iping].zap_x1 + 10;
	    ping[iping].zap_y1 = ping[iping].label_y - sascent;
	    ping[iping].zap_y2 =  ping[iping].zap_y1 + sascent + sdescent;
	    xg_drawrectangle(mbedit_xgid,
		    ping[iping].zap_x1, 
		    ping[iping].zap_y1, 
		    10, 
		    sascent + sdescent, 
		    pixel_values[BLACK],XG_SOLIDLINE);
	    }
	else if (ping[iping].outbounds == MBEDIT_OUTBOUNDS_FLAGGED)
	    xg_fillrectangle(mbedit_xgid,
		    ping[iping].label_x - swidth, 
		    ping[iping].label_y - sascent, 
		    swidth, 
		    sascent + sdescent, 
		    pixel_values[GREEN],XG_SOLIDLINE);
	xg_drawstring(mbedit_xgid,
		    ping[iping].label_x - swidth,
		    ping[iping].label_y,
		    string,
		    pixel_values[BLACK],XG_SOLIDLINE);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_unplot_beam(iping,jbeam)
int	iping;
int	jbeam;
{
	/* local variables */
	char	*function_name = "mbedit_unplot_beam";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iping:       %d\n",iping);
		fprintf(stderr,"dbg2       jbeam:       %d\n",jbeam);
		}

	/* unplot the beam */
	if (jbeam >= 0 && jbeam < beams_bath)
		{
		if (ping[iping].bath[jbeam] > 0.0)
			xg_fillrectangle(mbedit_xgid, 
				ping[iping].bath_x[jbeam]-2, 
				ping[iping].bath_y[jbeam]-2, 4, 4, 
				pixel_values[WHITE],XG_SOLIDLINE);
		else if (ping[iping].bath[jbeam] < 0.0)
			xg_drawrectangle(mbedit_xgid, 
				ping[iping].bath_x[jbeam]-2, 
				ping[iping].bath_y[jbeam]-2, 4, 4, 
				pixel_values[WHITE],XG_SOLIDLINE);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_unplot_ping(iping)
int	iping;
{
	/* local variables */
	char	*function_name = "mbedit_unplot_ping";
	int	status = MB_SUCCESS;
	int	j;
	int	first, xold, yold;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iping:       %d\n",iping);
		}

	/* unplot the ping profile */
	first = MB_YES;
	for (j=0;j<beams_bath;j++)
		{
		if (ping[iping].bath[j] > 0.0 && first == MB_YES)
			{
			first = MB_NO;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
			}
		else if (ping[iping].bath[j] > 0.0)
			{
			xg_drawline(mbedit_xgid,xold,yold,
					ping[iping].bath_x[j],
					ping[iping].bath_y[j],
					pixel_values[WHITE],XG_SOLIDLINE);
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_goto(ttime_i,hold_size,buffer_size,
	plwd,exgr,xntrvl,yntrvl,plt_size,sh_flggd,ndumped,
	nloaded,nbuffer,ngood,icurrent,nplt)
int	ttime_i[7];
int	hold_size;
int	buffer_size;
int	plwd;
int	exgr;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	sh_flggd;
int	*ndumped;
int	*nloaded;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_action_goto";
	int	status = MB_SUCCESS;
	double	ttime_d;
	int	found;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       time_i[0]:   %d\n",ttime_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:   %d\n",ttime_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:   %d\n",ttime_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:   %d\n",ttime_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:   %d\n",ttime_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:   %d\n",ttime_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:   %d\n",ttime_i[6]);
		fprintf(stderr,"dbg2       hold_size:   %d\n",hold_size);
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		}

	/* let the world know... */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n>> Looking for time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n\n",
			ttime_i[0],ttime_i[1],ttime_i[2],ttime_i[3],
			ttime_i[4],ttime_i[5],ttime_i[6]);
		}

	/* set found flag */
	found = MB_NO;

	/* get time_d value */
	mb_get_time(verbose,ttime_i,&ttime_d);

	/* check if a file has been opened */
	if (file_open == MB_NO)
		{
		status = MB_FAILURE;
		*ndumped = 0;
		*nloaded = 0;
		*nbuffer = nbuff;
		*ngood = nlist;
		current_id = 0;
		*icurrent = current_id;
		current = 0;
		*nplt = 0;
		if (verbose >= 1)
			fprintf(stderr,"\n>> No data file has been opened...\n");
		}

	/* check if the present buffer is already 
		later than the target time */
	else if (nlist > 0)
		{
		status = mb_buffer_get_next_data(verbose,
			buff_ptr,imbio_ptr,list[0],&id,
			time_i,&time_d,
			&navlon,&navlat,
			&speed,&heading,
			&beams_bath,&beams_amp,&pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			&error);
		if (time_d > ttime_d)
			{
			status = MB_FAILURE;
			*ndumped = 0;
			*nloaded = 0;
			*nbuffer = nbuff;
			*ngood = nlist;
			*icurrent = current_id;
			*nplt = 0;
			if (verbose >= 1)
				fprintf(stderr,"\n>> Beginning of present buffer is later than target time...\n");
			}
		}

	/* loop through buffers until the target time is found
		or the file ends */
	while (found == MB_NO && status == MB_SUCCESS)
		{
		/* check out current buffer */
		for (i=0;i<nlist;i++)
			{
			status = mb_buffer_get_next_data(verbose,
				buff_ptr,imbio_ptr,list[i],&id,
				time_i,&time_d,
				&navlon,&navlat,
				&speed,&heading,
				&beams_bath,&beams_amp,&pixels_ss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				&error);
			if (time_d > ttime_d && found == MB_NO)
				{
				found = MB_YES;
				current_id = i;
				current = id;
				}
			}

		/* load new buffer if needed */
		if (found == MB_NO)
			{
			/* dump the buffer */
			status = mbedit_dump_data(hold_size,ndumped,nbuffer);

			/* load the buffer */
			status = mbedit_load_data(buffer_size,nloaded,
				nbuffer,ngood,icurrent);

			/* if end of file close it */
			if (status == MB_FAILURE)
				{
				status = mbedit_dump_data(0,ndumped,nbuffer);
				mbedit_close_file();
				status = MB_FAILURE;
				}
			}
		}

	/* clear the screen */
	status = mbedit_clear_screen();

	/* set up plotting */
	if (*ngood > 0)
		{
		status = mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,
				plt_size,sh_flggd,nplt, MB_NO);
		}

	/* let the world know... */
	if (verbose >= 0 && found == MB_YES)
		{
		fprintf(stderr,"\n>> Target time %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d found\n",
			ttime_i[0],ttime_i[1],ttime_i[2],
			ttime_i[3],ttime_i[4],ttime_i[5],ttime_i[6]);
		fprintf(stderr,">> Found time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
			ping[0].time_i[0],ping[0].time_i[1],
			ping[0].time_i[2],ping[0].time_i[3],
			ping[0].time_i[4],ping[0].time_i[5],
			ping[0].time_i[6]);
		fprintf(stderr,"Current data record index:  %d\n",
			current_id);
		fprintf(stderr,"Current data record:        %d\n",
			list[current_id]);
		fprintf(stderr,"Current global data record: %d\n",
			list[current_id] + ndump_total);
		}
	else if (verbose >= 0)
		{
		fprintf(stderr,"\n>> Target time %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d found\n",
			ttime_i[0],ttime_i[1],ttime_i[2],
			ttime_i[3],ttime_i[4],ttime_i[5],ttime_i[6]);
		fprintf(stderr,"\n>> Unable to go to target time...\n");
		}

	/* reset beam_save */
	beam_save = MB_NO;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndumped:     %d\n",*ndumped);
		fprintf(stderr,"dbg2       nloaded:     %d\n",*nloaded);
		fprintf(stderr,"dbg2       nbuffer:     %d\n",*nbuffer);
		fprintf(stderr,"dbg2       ngood:       %d\n",*ngood);
		fprintf(stderr,"dbg2       icurrent:    %d\n",*icurrent);
		fprintf(stderr,"dbg2       nplot:        %d\n",*nplt);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_check_buffer_size(form,buffer_size,buffer_size_max)
int	form;
int	*buffer_size;
int	*buffer_size_max;
{
	/* local variables */
	char	*function_name = "mbedit_check_buffer_size";
	int	status = MB_SUCCESS;
	int	format_num;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       form:            %d\n",form);
		fprintf(stderr,"dbg2       buffer_size:     %d\n",*buffer_size);
		fprintf(stderr,"dbg2       buffer_size_max: %d\n",*buffer_size_max);
		}
		
	/* get format_num */
	status = mb_format(verbose,&form,&format_num,&error);
	
	/* set buffer size lower if format supports sidescan */
	if (pixels_ss_table[format_num] > 0)
		*buffer_size_max = MBEDIT_BUFFER_SIZE / 5;
	else
		*buffer_size_max = MBEDIT_BUFFER_SIZE;
	if (*buffer_size > *buffer_size_max)
		*buffer_size = *buffer_size_max;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       buffer_size:     %d\n",*buffer_size);
		fprintf(stderr,"dbg2       buffer_size_max: %d\n",*buffer_size_max);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_save_edit(time_d, beam, action)
double	time_d;
int	beam;
int	action;
{
	/* local variables */
	char	*function_name = "mbedit_save_edit";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       time_d:          %f\n",time_d);
		fprintf(stderr,"dbg2       beam:            %d\n",beam);
		fprintf(stderr,"dbg2       action:          %d\n",action);
		}
		
	/* write out the edit */
	if (sofile_open == MB_YES)
	    {
	    if (fwrite(&time_d, sizeof(double), 1, sofp) != 1)
		{
		status = MB_FAILURE;
		error = MB_ERROR_WRITE_FAIL;
		}
	    if (status == MB_SUCCESS
		&& fwrite(&beam, sizeof(int), 1, sofp) != 1)
		{
		status = MB_FAILURE;
		error = MB_ERROR_WRITE_FAIL;
		}
	    if (status == MB_SUCCESS
		&& fwrite(&action, sizeof(int), 1, sofp) != 1)
		{
		status = MB_FAILURE;
		error = MB_ERROR_WRITE_FAIL;
		}
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_retrieve_edit(time_d, beam, action)
double	*time_d;
int	*beam;
int	*action;
{
	/* local variables */
	char	*function_name = "mbedit_save_edit";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}
		
	/* write out the edit */
	if (sifile_open == MB_YES)
	    {
	    if (fread(time_d, sizeof(double), 1, sifp) != 1)
		{
		status = MB_FAILURE;
		error = MB_ERROR_EOF;
		}
	    if (status == MB_SUCCESS
		&& fread(beam, sizeof(int), 1, sifp) != 1)
		{
		status = MB_FAILURE;
		error = MB_ERROR_EOF;
		}
	    if (status == MB_SUCCESS
		&& fread(action, sizeof(int), 1, sifp) != 1)
		{
		status = MB_FAILURE;
		error = MB_ERROR_EOF;
		}
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       time_d:          %f\n",*time_d);
		fprintf(stderr,"dbg2       beam:            %d\n",*beam);
		fprintf(stderr,"dbg2       action:          %d\n",*action);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function sort sorts the data.  
 *	Uses the shell method from Numerical Recipes.
 */
#define ALN2I 1.442695022
#define TINY 1.0e-5
int sort(n,r)
int	n;
double *r;
{
	int	status = MB_SUCCESS;
	int	nn, m, j, i, lognb2;
	double	tr;

	if (n <= 0) 
		{
		status = MB_FAILURE;
		return(status);
		}

	lognb2 = (log((double) n)*ALN2I + TINY);
	m = n;
	for (nn=1;nn<=lognb2;nn++)
		{
		m >>= 1;
		for (j=m+1;j<=n;j++)
			{
			i = j - m;
			tr = r[j];
			while (i >= 1 && r[i] > tr)
				{
				r[i+m] = r[i];
				i -= m;
				}
			r[i+m] = tr;
			}
		}

	return(status);
}
/*--------------------------------------------------------------------*/

