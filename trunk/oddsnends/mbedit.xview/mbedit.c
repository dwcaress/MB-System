/*--------------------------------------------------------------------
 *    The MB-system:	mbedit.c	3.00	4/8/93
 *    $Id: mbedit.c,v 3.2 1993-08-17 00:28:52 caress Exp $
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
 * MBEDIT is an interactive beam editor for multibeam bathymetry data.
 * It can work with any data format supported by the MBIO library.
 * This version uses the XVIEW toolkit and has been developed using
 * the DEVGUIDE package.  A future version will employ the MOTIF
 * toolkit for greater portability.  This file contains
 * the code that does not directly depend on the XVIEW interface - the 
 * companion file mbedit_stubs.c contains the user interface related 
 * code.
 *
 * Author:	D. W. Caress
 * Date:	April 8, 1993
 *
 * $Log: not supported by cvs2svn $
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

/* MBIO include files */
#include "../../include/mb_format.h"
#include "../../include/mb_status.h"
#include "../../include/mb_io.h"

/* xgraphics defines */
#define	CLEAR_ALL	0
#define	BLACK_ALL	1
#define	OVERLAY1_CLEAR	64
#define OVERLAY1_DRAW	65
#define OVERLAY1_DASH	66
#define OVERLAY2_CLEAR	128
#define OVERLAY2_DRAW	129
#define OVERLAY2_DASH	130

/* ping structure definition */
struct mbedit_ping_struct 
	{
	int	id;
	int	record;
	int	time_i[6];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	int	*bath;
	int	*bathdist;
	int	*back;
	int	*backdist;
	int	*bath_x;
	int	*bath_y;
	};

/* id variables */
static char rcs_id[] = "$Id: mbedit.c,v 3.2 1993-08-17 00:28:52 caress Exp $";
static char program_name[] = "MBEDIT";
static char help_message[] =  "MBEDIT is an interactive beam editor for multibeam bathymetry data.\n\tIt can work with any data format supported by the MBIO library.\n\tThis version uses the XVIEW toolkit and has been developed using\n\tthe DEVGUIDE package.  A future version will employ the MOTIF\n\ttoolkit for greater portability.  This file contains the code \n\tthat does not directly depend on the XVIEW interface - the companion \n\tfile mbedit_stubs.c contains the user interface related code.";
static char usage_message[] = "mbedit [-Fformat -Ifile -Ooutfile -V -H]";

/* status variables */
int	error = MB_ERROR_NO_ERROR;
int	verbose = 0;
char	*message;

/* MBIO control parameters */
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
int	beams_bath;
int	beams_back;
char	ifile[128];
char	ofile[128];
int	ofile_defined = MB_NO;
char	*imbio_ptr;
char	*ombio_ptr;

/* mbio read and write values */
char	*store_ptr;
int	kind;
int	id;
int	time_i[6];
double	time_d;
double	navlon;
double	navlat;
double	speed;
double	heading;
double	distance;
int	nbath;
int	*bath;
int	*bathdist;
int	nback;
int	*back;
int	*backdist;
int	idata = 0;
int	icomment = 0;
int	odata = 0;
int	ocomment = 0;
char	comment[256];

/* buffer control variables */
/*#define	MBEDIT_BUFFER_SIZE	MB_BUFFER_MAX*/
#define	MBEDIT_BUFFER_SIZE	1000
int	file_open = MB_NO;
char	*buff_ptr;
int	buffer_size_default = MBEDIT_BUFFER_SIZE;
int	hold_size_default = MBEDIT_BUFFER_SIZE/4;
int	nload = 0;
int	ndump = 0;
int	nbuff = 0;
int	nlist = 0;
int	current = 0;
int	current_id = 0;
int	nload_total = 0;
int	ndump_total = 0;
char	last_ping[128];

/* ping drawing control variables */
#define	MBEDIT_MAX_PINGS	20
#define	MBEDIT_PICK_DISTANCE	50
struct mbedit_ping_struct	ping[MBEDIT_MAX_PINGS];
int	list[MBEDIT_BUFFER_SIZE];
int	plot_size = MBEDIT_MAX_PINGS/2;
int	nplot = 0;
int	mbedit_xgid;
int	borders[4];
int	scale_max = 2000;
int	xscale = 1000;
int	yscale = 1000;
int	x_interval = 1000;
int	y_interval = 250;
int	beam_save = MB_NO;
int	iping_save = 0;
int	jbeam_save = 0;

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
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;
	strcpy(ifile,"\0");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:I:i:O:o:")) != -1)
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
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       help:            %d\n",help);
		fprintf(stderr,"dbg2       format:          %d\n",format);
		fprintf(stderr,"dbg2       input file:      %s\n",ifile);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(MB_ERROR_NO_ERROR);
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
		{
		status = mbedit_action_open(ifile,format,hold_size_default,
				buffer_size_default,
				xscale,yscale,x_interval,y_interval,plot_size,
				&ndump,&nload,&nbuff,&nlist,&current_id,&nplot);
		if (status = MB_SUCCESS)
			*startup_file = MB_YES;
		}
	else
		*startup_file = MB_NO;

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
int mbedit_set_graphics(xgid,brdr)
int	xgid;
int	*brdr;
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
			fprintf(stderr,"dbg2       borders[%d]:   %d\n",
				i,brdr[0]);
		}

	/* set graphics id */
	mbedit_xgid = xgid;

	/* set graphics bounds */
	for (i=0;i<4;i++)
		borders[i] = brdr[i];

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
int mbedit_get_defaults(plt_size_max,plt_size,
	buffer_size_max,buffer_size,hold_size,form,
	sclmx,xscl,yscl,xntrvl,yntrvl)
int	*plt_size_max;
int	*plt_size;
int	*buffer_size_max;
int	*buffer_size;
int	*hold_size;
int	*form;
int	*sclmx;
int	*xscl;
int	*yscl;
int	*xntrvl;
int	*yntrvl;
{
	/* local variables */
	char	*function_name = "mbedit_get_defaults";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* get maximum number of pings to plot */
	*plt_size_max = MBEDIT_MAX_PINGS;
	*plt_size = MBEDIT_MAX_PINGS/2;

	/* get maximum and starting buffer sizes */
	*buffer_size_max = MBEDIT_BUFFER_SIZE;
	*buffer_size = buffer_size_default;

	/* get starting hold size */
	*hold_size = hold_size_default;

	/* get format */
	*form = format;

	/* get scaling */
	*sclmx = scale_max;
	*xscl = xscale;
	*yscl = yscale;

	/* get tick intervals */
	*xntrvl = x_interval;
	*yntrvl = y_interval;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       plot max:    %d\n",*plt_size_max);
		fprintf(stderr,"dbg2       plot_size:   %d\n",*plt_size);
		fprintf(stderr,"dbg2       buffer max:  %d\n",*buffer_size_max);
		fprintf(stderr,"dbg2       buffer_size: %d\n",*buffer_size);
		fprintf(stderr,"dbg2       hold_size:   %d\n",*hold_size);
		fprintf(stderr,"dbg2       format:      %d\n",*form);
		fprintf(stderr,"dbg2       xscale:      %d\n",*xscl);
		fprintf(stderr,"dbg2       yscale:      %d\n",*yscl);
		fprintf(stderr,"dbg2       x_interval:  %d\n",*xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",*yntrvl);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_open(file,form,hold_size,buffer_size,
	xscl,yscl,xntrvl,yntrvl,plt_size,
	ndumped,nloaded,nbuffer,ngood,icurrent,nplt)
char	*file;
int	form;
int	hold_size;
int	buffer_size;
int	xscl;
int	yscl;
int	xntrvl;
int	yntrvl;
int	plt_size;
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
		fprintf(stderr,"dbg2       file:        %s\n",file);
		fprintf(stderr,"dbg2       format:      %d\n",form);
		fprintf(stderr,"dbg2       hold_size:   %d\n",hold_size);
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		fprintf(stderr,"dbg2       xscale:      %d\n",xscl);
		fprintf(stderr,"dbg2       yscale:      %d\n",yscl);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		}

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> Open File Event:\n");

	/* clear the screen */
	status = mbedit_clear_screen();

	/* open the file */
	status = mbedit_open_file(file,form);

	/* load the buffer */
	if (status == MB_SUCCESS)
		status = mbedit_load_data(buffer_size,nloaded,nbuffer,
			ngood,icurrent);

	/* keep going until good data or end of file found */
	while (*nloaded > 0 && *ngood == 0)
		{
		/* dump the buffer */
		status = mbedit_dump_data(hold_size,ndumped,nbuffer);

		/* load the buffer */
		status = mbedit_load_data(buffer_size,nloaded,nbuffer,
			ngood,icurrent);
		}

	/* set up plotting */
	if (ngood > 0)
		{
		status = mbedit_plot_all(xscl,yscl,xntrvl,yntrvl,plt_size,nplt);
		}

	/* reset beam_save */
	beam_save = MB_NO;

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> End Open File Event.\n");

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nloaded:     %d\n",*ndumped);
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
int mbedit_action_next_buffer(hold_size,buffer_size,
	xscl,yscl,xntrvl,yntrvl,plt_size,ndumped,
	nloaded,nbuffer,ngood,icurrent,nplt)
int	hold_size;
int	buffer_size;
int	xscl;
int	yscl;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	*ndumped;
int	*nloaded;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
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
		fprintf(stderr,"dbg2       xscale:      %d\n",xscl);
		fprintf(stderr,"dbg2       yscale:      %d\n",yscl);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		}

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> Next Buffer Event:\n");

	/* clear the screen */
	status = mbedit_clear_screen();

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
			}

		/* else set up plotting */
		else
			{
			status = mbedit_plot_all(xscl,yscl,xntrvl,yntrvl,plt_size,nplt);
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

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> End Next Buffer Event.\n");

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
int mbedit_action_done(buffer_size,ndumped,nloaded,nbuffer,ngood,icurrent)
int	buffer_size;
int	*ndumped;
int	*nloaded;
int	*nbuffer;
int	*ngood;
int	*icurrent;
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

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> Done Event:\n");

	/* clear the screen */
	status = mbedit_clear_screen();

	/* if file has been opened deal with it */
	if (file_open == MB_YES)
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
		fprintf(stderr,"\n>> End Done Event.\n");
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
		fprintf(stderr,"\n>> Quit Event:\n");

	/* clear the screen */
	status = mbedit_clear_screen();

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\nShutting MBEDIT down without further ado...\n");

	/* call routine to deal with saving the current file, if any */
	if (file_open == MB_YES)
		status = mbedit_action_done(buffer_size,ndumped,nloaded,
			nbuffer,ngood,icurrent);

	/* reset beam_save */
	beam_save = MB_NO;

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\nQuitting MBEDIT\nBye Bye...\n");

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
int mbedit_action_step(step,xscl,yscl,xntrvl,yntrvl,plt_size,
	nbuffer,ngood,icurrent,nplt)
int	step;
int	xscl;
int	yscl;
int	xntrvl;
int	yntrvl;
int	plt_size;
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
		fprintf(stderr,"dbg2       xscale:      %d\n",xscl);
		fprintf(stderr,"dbg2       yscale:      %d\n",yscl);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		}

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> Step Event:\n");

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
			status = mbedit_plot_all(xscl,yscl,xntrvl,yntrvl,plt_size,nplt);
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
		*nbuffer = nbuff;
		*ngood = nlist;
		current_id = 0;
		*icurrent = current_id;
		current = 0;
		}

	/* reset beam_save */
	beam_save = MB_NO;

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> End Step Event.\n");

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
int mbedit_action_plot(xscl,yscl,xntrvl,yntrvl,plt_size,
	nbuffer,ngood,icurrent,nplt)
int	xscl;
int	yscl;
int	xntrvl;
int	yntrvl;
int	plt_size;
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
		fprintf(stderr,"dbg2       xscale:      %d\n",xscl);
		fprintf(stderr,"dbg2       yscale:      %d\n",yscl);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		}

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> Plot Event:\n");

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
			status = mbedit_plot_all(xscl,yscl,xntrvl,yntrvl,plt_size,nplt);
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

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> End Plot Event.\n");

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
int mbedit_action_mouse(x_loc,y_loc,
	xscl,yscl,xntrvl,yntrvl,plt_size,
	nbuffer,ngood,icurrent,nplt)
int	x_loc;
int	y_loc;
int	xscl;
int	yscl;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	*nbuffer;
int	*ngood;
int	*icurrent;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_action_mouse";
	int	status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       xscale:      %d\n",xscl);
		fprintf(stderr,"dbg2       yscale:      %d\n",yscl);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		}

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> Mouse Event:\n");

	/* check if a file has been opened */
	if (file_open == MB_YES)
		{

		/* check if a beam has been picked */
		iping = 0;
		jbeam = 0;
		range_min = 100000;
		for (i=0;i<nplot;i++)
			{
			for (j=0;j<beams_bath;j++)
				{
				if (ping[i].bath[j] != 0)
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
			ping[iping].bath[jbeam] = 
				-ping[iping].bath[jbeam];
			status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[iping].id,
				ping[iping].time_i,ping[iping].time_d,
				ping[iping].navlon,ping[iping].navlat,
				ping[iping].speed,ping[iping].heading,
				beams_bath,ping[iping].bath,
				ping[iping].bathdist,
				beams_back,ping[iping].back,
				ping[iping].backdist,
				comment,
				&error);
			if (verbose >= 1)
				{
				fprintf(stderr,"\nping: %d beam:%d depth:%d ",
					iping,jbeam,ping[iping].bath[jbeam]);
				if (ping[iping].bath[jbeam] > 0)
					fprintf(stderr," unflagged\n");
				else
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
		if (found && *ngood > 0)
			{
			status = mbedit_plot_ping(iping);
			status = mbedit_plot_beam(iping,jbeam-1);
			status = mbedit_plot_beam(iping,jbeam);
			status = mbedit_plot_beam(iping,jbeam+1);
			}
		else
			status = MB_FAILURE;
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

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> End Mouse Event.\n");

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
	xscl,yscl,xntrvl,yntrvl,plt_size,
	nbuffer,ngood,icurrent,nplt)
int	xscl;
int	yscl;
int	xntrvl;
int	yntrvl;
int	plt_size;
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
		fprintf(stderr,"dbg2       xscale:      %d\n",xscl);
		fprintf(stderr,"dbg2       yscale:      %d\n",yscl);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		}

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> Bad Ping Event:\n");

	/* check if a file has been opened 
		and a beam has been picked and saved */
	if (file_open == MB_YES && beam_save == MB_YES)
		{

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (j=0;j<beams_bath;j++)
			status = mbedit_unplot_beam(iping_save,j);

		/* flag beams in bad ping */
		for (j=0;j<beams_bath;j++)
			if (ping[iping_save].bath[j] > 0)
				ping[iping_save].bath[j] = 
					-ping[iping_save].bath[j];
		status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[iping_save].id,
				ping[iping_save].time_i,ping[iping_save].time_d,
				ping[iping_save].navlon,ping[iping_save].navlat,
				ping[iping_save].speed,ping[iping_save].heading,
				beams_bath,ping[iping_save].bath,
				ping[iping_save].bathdist,
				beams_back,ping[iping_save].back,
				ping[iping_save].backdist,
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
		}

	/* if no file open or beam saved set failure status */
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

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> End Bad Ping Event.\n");

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
	xscl,yscl,xntrvl,yntrvl,plt_size,
	nbuffer,ngood,icurrent,nplt)
int	xscl;
int	yscl;
int	xntrvl;
int	yntrvl;
int	plt_size;
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
		fprintf(stderr,"dbg2       xscale:      %d\n",xscl);
		fprintf(stderr,"dbg2       yscale:      %d\n",yscl);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		}

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> Good Ping Event:\n");

	/* check if a file has been opened 
		and a beam has been picked and saved */
	if (file_open == MB_YES && beam_save == MB_YES)
		{

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (j=0;j<beams_bath;j++)
			status = mbedit_unplot_beam(iping_save,j);

		/* flag beams in good ping */
		for (j=0;j<beams_bath;j++)
			if (ping[iping_save].bath[j] < 0)
				ping[iping_save].bath[j] = 
					-ping[iping_save].bath[j];
		status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[iping_save].id,
				ping[iping_save].time_i,ping[iping_save].time_d,
				ping[iping_save].navlon,ping[iping_save].navlat,
				ping[iping_save].speed,ping[iping_save].heading,
				beams_bath,ping[iping_save].bath,
				ping[iping_save].bathdist,
				beams_back,ping[iping_save].back,
				ping[iping_save].backdist,
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
		}

	/* if no file open or beam saved set failure status */
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

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> End Good Ping Event.\n");

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
	xscl,yscl,xntrvl,yntrvl,plt_size,
	nbuffer,ngood,icurrent,nplt)
int	xscl;
int	yscl;
int	xntrvl;
int	yntrvl;
int	plt_size;
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
		fprintf(stderr,"dbg2       xscale:      %d\n",xscl);
		fprintf(stderr,"dbg2       yscale:      %d\n",yscl);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		}

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> Left Ping Event:\n");

	/* check if a file has been opened 
		and a beam has been picked and saved */
	if (file_open == MB_YES && beam_save == MB_YES)
		{

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (j=0;j<beams_bath;j++)
			status = mbedit_unplot_beam(iping_save,j);

		/* flag beams to left of picked beam */
		for (j=jbeam_save;j<beams_bath;j++)
			if (ping[iping_save].bath[j] > 0)
				ping[iping_save].bath[j] = 
					-ping[iping_save].bath[j];
		status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[iping_save].id,
				ping[iping_save].time_i,ping[iping_save].time_d,
				ping[iping_save].navlon,ping[iping_save].navlat,
				ping[iping_save].speed,ping[iping_save].heading,
				beams_bath,ping[iping_save].bath,
				ping[iping_save].bathdist,
				beams_back,ping[iping_save].back,
				ping[iping_save].backdist,
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
		status = mbedit_plot_ping(iping_save,OVERLAY2_DRAW);
		for (j=0;j<beams_bath;j++)
			status = mbedit_plot_beam(iping_save,j);
		}

	/* if no file open or beam saved set failure status */
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

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> End Left Ping Event.\n");

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
	xscl,yscl,xntrvl,yntrvl,plt_size,
	nbuffer,ngood,icurrent,nplt)
int	xscl;
int	yscl;
int	xntrvl;
int	yntrvl;
int	plt_size;
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
		fprintf(stderr,"dbg2       xscale:      %d\n",xscl);
		fprintf(stderr,"dbg2       yscale:      %d\n",yscl);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		}

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> Right Ping Event:\n");

	/* check if a file has been opened 
		and a beam has been picked and saved */
	if (file_open == MB_YES && beam_save == MB_YES)
		{

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (j=0;j<beams_bath;j++)
			status = mbedit_unplot_beam(iping_save,j);

		/* flag beams to right of picked beam */
		for (j=0;j<=jbeam_save;j++)
			if (ping[iping_save].bath[j] > 0)
				ping[iping_save].bath[j] = 
					-ping[iping_save].bath[j];
		status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[iping_save].id,
				ping[iping_save].time_i,ping[iping_save].time_d,
				ping[iping_save].navlon,ping[iping_save].navlat,
				ping[iping_save].speed,ping[iping_save].heading,
				beams_bath,ping[iping_save].bath,
				ping[iping_save].bathdist,
				beams_back,ping[iping_save].back,
				ping[iping_save].backdist,
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
		}

	/* if no file open or beam saved set failure status */
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

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\n>> End Right Ping Event.\n");

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
int mbedit_open_file(file,form)
char	*file;
int	form;
{
	/* local variables */
	char	*function_name = "mbedit_open_file";
	int	status = MB_SUCCESS;
	char	*suffix;
	int	len;
	int	i;

	/* time, user, host variables */
	long int	right_now;
	char	date[25], user[128], host[128];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       file:        %s\n",file);
		fprintf(stderr,"dbg2       format:      %d\n",form);
		}

	/* get filenames */
	strcpy(ifile,file);
	if (ofile_defined == MB_NO)
		{
		len = 0;
		if ((suffix = strstr(ifile,".mb")) != NULL)
			len = strlen(suffix);
		if (len >= 4 && len <= 5)
			{
			strncpy(ofile,"\0",126);
			strncpy(ofile,ifile,strlen(ifile)-len);
			strcat(ofile,"e");
			strcat(ofile,suffix);
			}
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
		&beams_bath,&beams_back,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",ifile);
		status = MB_FAILURE;
		return(status);
		}

	/* initialize writing the output multibeam file */
	if ((status = mb_write_init(
		verbose,ofile,format,&ombio_ptr,
		&beams_bath,&beams_back,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",ofile);
		status = MB_FAILURE;
		return(status);
		}

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,beams_bath*sizeof(int),&bath,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(int),&bathdist,&error);
	status = mb_malloc(verbose,beams_back*sizeof(int),&back,&error);
	status = mb_malloc(verbose,beams_back*sizeof(int),&backdist,&error);
	for (i=0;i<MBEDIT_MAX_PINGS;i++)
		{
		status = mb_malloc(verbose,beams_bath*sizeof(int),
			&ping[i].bath,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(int),
			&ping[i].bathdist,&error);
		status = mb_malloc(verbose,beams_back*sizeof(int),
			&ping[i].back,&error);
		status = mb_malloc(verbose,beams_back*sizeof(int),
			&ping[i].backdist,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(int),
			&ping[i].bath_x,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(int),
			&ping[i].bath_y,&error);
		}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize the buffer */
	status = mb_buffer_init(verbose,&buff_ptr,&error);
	nbuff = 0;

	/* write comments to beginning of output file */
	kind = MB_DATA_COMMENT;
	strncpy(comment,"\0",256);
	sprintf(comment,"Bathymetry data edited interactively using program %s version %s",
		program_name,rcs_id);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,bath,bathdist,
			beams_back,back,backdist,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"MB-system Version %s",MB_VERSION);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,bath,bathdist,
			beams_back,back,backdist,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	right_now = time((long *)0);
	strncpy(date,"\0",25);
	right_now = time((long *)0);
	strncpy(date,ctime(&right_now),24);
	strcpy(user,getenv("USER"));
	gethostname(host,128);
	strncpy(comment,"\0",256);
	sprintf(comment,"Run by user <%s> on cpu <%s> at <%s>",
		user,host,date);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,bath,bathdist,
			beams_back,back,backdist,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"Control Parameters:");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,bath,bathdist,
			beams_back,back,backdist,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  MBIO data format:   %d",format);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,bath,bathdist,
			beams_back,back,backdist,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Input file:         %s",ifile);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,bath,bathdist,
			beams_back,back,backdist,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Output file:        %s",ofile);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,bath,bathdist,
			beams_back,back,backdist,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment," ");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,bath,bathdist,
			beams_back,back,backdist,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;

	/* if we got here we must have succeeded */
	if (verbose >= 1)
		{
		fprintf(stderr,"\nMultibeam File <%s> initialized for reading\n",ifile);
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
	status = mb_buffer_close(verbose,buff_ptr,&error);
	status = mb_close(verbose,imbio_ptr,&error);
	status = mb_close(verbose,ombio_ptr,&error);
	ofile_defined = MB_NO;

	/* deallocate memory for data arrays */
	mb_free(verbose,bath,&error);
	mb_free(verbose,bathdist,&error);
	mb_free(verbose,back,&error);
	mb_free(verbose,backdist,&error);
	for (i=0;i<MBEDIT_MAX_PINGS;i++)
		{
		mb_free(verbose,ping[i].bath,&error);
		mb_free(verbose,ping[i].bathdist,&error);
		mb_free(verbose,ping[i].back,&error);
		mb_free(verbose,ping[i].backdist,&error);
		mb_free(verbose,ping[i].bath_x,&error);
		mb_free(verbose,ping[i].bath_y,&error);
		}

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* if we got here we must have succeeded */
	if (verbose >= 1)
		{
		fprintf(stderr,"\nMultibeam Input File <%s> closed\n",ifile);
		fprintf(stderr,"Multibeam Output File <%s> closed\n",ofile);
		fprintf(stderr,"%d data records loaded\n",nload_total);
		fprintf(stderr,"%d data records dumped\n",ndump_total);
		
		}
	file_open = MB_NO;
	nload_total = 0;
	ndump_total = 0;

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
	char	*function_name = "mbedit_get_max_pings";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       hold_size:   %d\n",hold_size);
		}

	/* dump data from the buffer */
	ndump = 0;
	if (nbuff > 0)
		{
		status = mb_buffer_dump(verbose,buff_ptr,ombio_ptr,
			hold_size,&ndump,&nbuff,&error);
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
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d data records dumped to output file <%s>\n",
			*ndumped,ofile);
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
	do
		{
		status = mb_buffer_get_next_data(verbose,
			buff_ptr,imbio_ptr,start,&id,
			time_i,&time_d,
			&navlon,&navlat,
			&speed,&heading,
			&beams_bath,bath,bathdist,
			&beams_back,back,backdist,
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

	/* find index of current ping */
	current_id = 0;
	for (i=0;i<nlist;i++)
		{
		if (list[i] <= current)
			current_id = i;
		}
	*icurrent = current_id;
	current = list[current_id];

	/* print out information */
	if (verbose >= 1)
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
		borders[1]-borders[0],borders[3]-borders[2],CLEAR_ALL);

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
int mbedit_plot_all(xscl,yscl,xntrvl,yntrvl,plt_size,nplt)
int	xscl;
int	yscl;
int	xntrvl;
int	yntrvl;
int	plt_size;
int	*nplt;
{
	/* local variables */
	char	*function_name = "mbedit_plot_all";
	int	status = MB_SUCCESS;
	int	i, j, k, ii;
	int	jbeam_cen;
	int	nbathsum;
	double	bathsum, bathmean;
	double	dxscale, dyscale;
	double	exager;
	int	xmin, xmax, ymin, ymax;
	int	margin;
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
		fprintf(stderr,"dbg2       xscale:      %d\n",xscl);
		fprintf(stderr,"dbg2       yscale:      %d\n",yscl);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		}

	/* set scales and tick intervals */
	xscale = xscl;
	yscale = yscl;
	x_interval = xntrvl;
	y_interval = yntrvl;

	/* figure out which pings to plot */
	plot_size = plt_size;
	if (current_id + plot_size > nlist)
		nplot = nlist - current_id;
	else
		nplot = plot_size;
	*nplt = nplot;

	/* get data into ping arrays and find mean depth value */
	bathsum = 0.0;
	nbathsum = 0;
	ii = current;
	for (i=0;i<nplot;i++)
		{
		status = mb_buffer_get_next_data(verbose,
			buff_ptr,imbio_ptr,ii,&ping[i].id,
			ping[i].time_i,&ping[i].time_d,
			&ping[i].navlon,&ping[i].navlat,
			&ping[i].speed,&ping[i].heading,
			&beams_bath,ping[i].bath,ping[i].bathdist,
			&beams_back,ping[i].back,ping[i].backdist,
			&error);
		if (status == MB_SUCCESS)
			{
			ping[i].record = ping[i].id + ndump_total;
			for (j=0;j<beams_bath;j++)
				{
				if (ping[i].bath[j] > 0)
					{
					bathsum += ping[i].bath[j];
					nbathsum++;
					}
				}
			}
		ii = ping[i].id + 1;
		}
	if (nbathsum > 0)
		bathmean = bathsum/nbathsum;

	/* print out information */
	jbeam_cen = beams_bath/2;
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d data records set for plotting (%d desired)\n",
			nplot,plot_size);
		for (i=0;i<nplot;i++)
			fprintf(stderr,"%4d %4d %4d  %d/%d/%d %2.2d:%2.2d:%2.2d  %4d\n",
				i,ping[i].id,ping[i].record,
				ping[i].time_i[1],ping[i].time_i[2],
				ping[i].time_i[0],ping[i].time_i[3],
				ping[i].time_i[4],ping[i].time_i[5],
				ping[i].bath[jbeam_cen]);
		}

	/* clear screen */
	xg_fillrectangle(mbedit_xgid,borders[0],borders[2],
		borders[1]-borders[0],borders[3]-borders[2],CLEAR_ALL);

	/* set scaling */
	margin = (borders[1] - borders[0])/15;
	xmin = 5*margin;
	xmax = borders[1] - margin;
	ymin = margin;
	ymax = borders[3] - margin;
	xcen = xmin + (xmax - xmin)/2;
	dy = (ymax - ymin)/plot_size;
	dxscale = 100.0/xscale;
	dyscale = 100.0/yscale;

	/* plot top label */
	exager = dyscale/dxscale;
	sprintf(string,"Vertical Exageration: %4.2f",exager);
	xg_justify(mbedit_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(mbedit_xgid,xcen-swidth/2,
		ymin-margin/2-sascent,string,BLACK_ALL);
	sprintf(string,"Crosstrack Distances and Depths in Meters",exager);
	xg_justify(mbedit_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(mbedit_xgid,xcen-swidth/2,
		ymin-margin/2+sascent,string,BLACK_ALL);

	/* plot scale bars */
	dx_width = (xmax - xmin)/dxscale;
	nx_int = 0.5*dx_width/x_interval + 1;
	x_int = x_interval*dxscale;
	xg_drawline(mbedit_xgid,xmin,ymax,xmax,ymax,BLACK_ALL);
	xg_drawline(mbedit_xgid,xmin,ymin,xmax,ymin,BLACK_ALL);
	for (i=0;i<nx_int;i++)
		{
		xx = i*x_int;
		vx = i*x_interval;
		xg_drawline(mbedit_xgid,xcen-xx,ymin,xcen-xx,ymax,
			OVERLAY1_DASH);
		xg_drawline(mbedit_xgid,xcen+xx,ymin,xcen+xx,ymax,
			OVERLAY1_DASH);
		sprintf(string,"%1d",vx);
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,xcen+xx-swidth/2,
			ymax+sascent+5,string,BLACK_ALL);
		xg_drawstring(mbedit_xgid,xcen-xx-swidth/2,
			ymax+sascent+5,string,BLACK_ALL);
		}
	dy_height = (ymax - ymin)/dyscale;
	ny_int = dy_height/y_interval + 1;
	y_int = y_interval*dyscale;
	xg_drawline(mbedit_xgid,xmin,ymin,xmin,ymax,BLACK_ALL);
	xg_drawline(mbedit_xgid,xmax,ymin,xmax,ymax,BLACK_ALL);
	for (i=0;i<ny_int;i++)
		{
		yy = i*y_int;
		vy = i*y_interval;
		xg_drawline(mbedit_xgid,xmin,ymax-yy,xmax,ymax-yy,
			OVERLAY1_DASH);
		sprintf(string,"%1d",vy);
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,xmax+5,
			ymax-yy+sascent/2,string,BLACK_ALL);
		}

	/* plot pings */
	for (i=0;i<nplot;i++)
		{
		/* set and draw info string */
		y = ymax - dy/2 - i*dy;
		sprintf(string,"%d  %d/%d/%d %2.2d:%2.2d:%2.2d  %d",
			ping[i].record,
			ping[i].time_i[1],ping[i].time_i[2],
			ping[i].time_i[0],ping[i].time_i[3],
			ping[i].time_i[4],ping[i].time_i[5],
			ping[i].bath[jbeam_cen]);
		xg_justify(mbedit_xgid,string,&swidth,&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,5*margin-swidth-5,y,
			string,BLACK_ALL);

		/* save string to show last ping seen at end of program */
		strcpy(last_ping,string);

		/* set beam plotting locations */
		for (j=0;j<beams_bath;j++)
			{
			if (ping[i].bath[j] != 0)
				{
				ping[i].bath_x[j] = xcen 
					- dxscale*ping[i].bathdist[j];
				ping[i].bath_y[j] = y + dyscale*
					(fabs((double)ping[i].bath[j]) 
					- bathmean);
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
				OVERLAY1_DRAW);
		else if (ping[iping].bath[jbeam] < 0)
			xg_drawrectangle(mbedit_xgid, 
				ping[iping].bath_x[jbeam]-2, 
				ping[iping].bath_y[jbeam]-2, 4, 4, 
				OVERLAY2_DRAW);
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
	int	first, xold, yold;

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
	for (j=0;j<beams_bath;j++)
		{
		if (ping[iping].bath[j] > 0 && first == MB_YES)
			{
			first = MB_NO;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
			}
		else if (ping[iping].bath[j] > 0)
			{
			xg_drawline(mbedit_xgid,xold,yold,
					ping[iping].bath_x[j],
					ping[iping].bath_y[j],
					OVERLAY1_DRAW);
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

	/* plot the beam */
	if (jbeam >= 0 && jbeam < beams_bath)
		{
		if (ping[iping].bath[jbeam] > 0)
			xg_fillrectangle(mbedit_xgid, 
				ping[iping].bath_x[jbeam]-2, 
				ping[iping].bath_y[jbeam]-2, 4, 4, 
				OVERLAY1_CLEAR);
		else if (ping[iping].bath[jbeam] < 0)
			xg_drawrectangle(mbedit_xgid, 
				ping[iping].bath_x[jbeam]-2, 
				ping[iping].bath_y[jbeam]-2, 4, 4, 
				OVERLAY2_CLEAR);
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

	/* plot the ping profile */
	first = MB_YES;
	for (j=0;j<beams_bath;j++)
		{
		if (ping[iping].bath[j] > 0 && first == MB_YES)
			{
			first = MB_NO;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
			}
		else if (ping[iping].bath[j] > 0)
			{
			xg_drawline(mbedit_xgid,xold,yold,
					ping[iping].bath_x[j],
					ping[iping].bath_y[j],
					OVERLAY1_CLEAR);
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
