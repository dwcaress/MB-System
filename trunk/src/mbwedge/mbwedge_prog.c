/*--------------------------------------------------------------------
 *    The MB-system:	mbwedge_prog.c	6/9/2010
 *    $Id$
 *
 *    Copyright (c) 2010 by
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
 * MBwedge is an interactive sonar display tool for use with the
 * watercolumn data from multibeam sonars.
 *
 * Author:	D. W. Caress
 * Date:	June 9, 2010
 *
 */
/*--------------------------------------------------------------------*/

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <X11/Intrinsic.h>

/* MBIO include files */
#include "../../include/mb_format.h"
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_io.h"
#include "../../include/mb_swap.h"
#include "../../include/mb_process.h"
#include "../../include/mb_xgraphics.h"
#include "mbwedge.h"

/* sonar data structure definition */
struct mbwedge_beam_struct
	{
	int	sourcetype;
	int	picktype;
	int	beamflag;
	double	range;
	float	*trace;
	};
struct mbwedge_ping_struct 
	{
	int	recordid;
	int	time_i[7];
	double	time_d;
	double	time_interval;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	altitude;
	double	sonardepth;
	double	roll;
	double	pitch;
	double	heave;
	int	nsamples;
	double	sample_interval;	/* usec */
	double	start_time;		/* seconds */
	double	end_time;		/* seconds */
	int	samples;
	int	nbeams;
	struct mbwedge_beam_struct *beam;
	};

/* id variables */
static char rcs_id[] = "$Id$";
static char program_name[] = "MBwedge";
static char help_message[] =  
"MBwedge is an interactive sonar display tool for use with the\n\
watercolumn data from multibeam sonars..";
static char usage_message[] = "mbwedge [-Fformat -Ifile -V -H]";

/* status variables */
static char	*message = NULL;

/* MBIO control parameters */
static int	pings;
static int	lonflip;
static double	bounds[4];
static int	btime_i[7];
static int	etime_i[7];
static double	btime_d;
static double	etime_d;
static double	speedmin;
static double	timegap;
static int	beams_bath;
static int	beams_amp;
static int	pixels_ss;
static void	*imbio_ptr = NULL;

/* mbio read and write values */
static void	*store_ptr = NULL;
static int	kind;
static double	distance;
static double	draft;
static char	*beamflag = NULL;
static double	*bath = NULL;
static double	*bathacrosstrack = NULL;
static double	*bathalongtrack = NULL;
static double	*amp = NULL;
static double	*ss = NULL;
static double	*ssacrosstrack = NULL;
static double	*ssalongtrack = NULL;
static int	*detect = NULL;
static int	*pulses = NULL;
static int	*editcount = NULL;
static char	comment[MB_COMMENT_MAXLINE];

/* buffer control variables */
#define	MBWEDGE_BUFFER_SIZE	25000
static int	file_open = MB_NO;
static int	buff_size = MBWEDGE_BUFFER_SIZE;
static int	buff_size_max = MBWEDGE_BUFFER_SIZE;
static int	holdd_size = MBWEDGE_BUFFER_SIZE / 1000;
static int	nload = 0;
static int	ndump = 0;
static int	nbuff = 0;
static int	current_id = 0;
static int	nload_total = 0;
static int	ndump_total = 0;
static char	last_ping[MB_PATH_MAXLINE];

/* ping drawing control variables */
#define	MBWEDGE_MAX_PINGS	250
struct mbwedge_ping_struct	ping[MBWEDGE_BUFFER_SIZE];

/*--------------------------------------------------------------------*/
int mbwedge_init(int argc, char ** argv, int *startup_file)
{
	/* local variables */
	char	*function_name = "mbwedge_init";
	int	status = MB_SUCCESS;
	int	fileflag = 0;
	int	i;

	/* parsing variables */
	extern char *optarg;
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
	strcpy(input_file,"\0");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:I:i:")) != -1)
	  {
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
			sscanf (optarg,"%s", input_file);
			flag++;
			fileflag++;
			break;
		case '?':
			errflg++;
		}
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
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       help:            %d\n",help);
		fprintf(stderr,"dbg2       format:          %d\n",format);
		fprintf(stderr,"dbg2       input_file:      %s\n",input_file);
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
int mbwedge_set_graphics(void *xgid, int ncol, unsigned int *pixels)
{
	/* local variables */
	char	*function_name = "mbwedge_set_graphics";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       xgid:         %lu\n",(size_t)xgid);
		fprintf(stderr,"dbg2       ncolors:      %d\n",ncol);
		for (i=0;i<ncol;i++)
			fprintf(stderr,"dbg2       pixel[%d]:     %d\n",
				i, pixels[i]);
		}

	/* set graphics id */
	mbwedge_xgid = xgid;

	/* set colors */
	ncolors = ncol;
	for (i=0;i<ncolors;i++)
		pixel_values[i] = pixels[i];

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
int mbwedge_action_open()
{
	/* local variables */
	char	*function_name = "mbwedge_action_open";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		fprintf(stderr,"dbg2       nbuffer:     %d\n",nbuffer);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",nbuffer);
		fprintf(stderr,"dbg2       ndumped:     %d\n",ndumped);
		fprintf(stderr,"dbg2       nloaded:     %d\n",nloaded);
		fprintf(stderr,"dbg2       icurrent:    %d\n",icurrent);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbwedge_action_load()
{
	/* local variables */
	char	*function_name = "mbwedge_action_load";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		fprintf(stderr,"dbg2       nbuffer:     %d\n",nbuffer);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",nbuffer);
		fprintf(stderr,"dbg2       ndumped:     %d\n",ndumped);
		fprintf(stderr,"dbg2       nloaded:     %d\n",nloaded);
		fprintf(stderr,"dbg2       icurrent:    %d\n",icurrent);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbwedge_action_dump()
{
	/* local variables */
	char	*function_name = "mbwedge_action_dump";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		fprintf(stderr,"dbg2       nbuffer:     %d\n",nbuffer);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",nbuffer);
		fprintf(stderr,"dbg2       ndumped:     %d\n",ndumped);
		fprintf(stderr,"dbg2       nloaded:     %d\n",nloaded);
		fprintf(stderr,"dbg2       icurrent:    %d\n",icurrent);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbwedge_action_close()
{
	/* local variables */
	char	*function_name = "mbwedge_action_close";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		fprintf(stderr,"dbg2       nbuffer:     %d\n",nbuffer);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",nbuffer);
		fprintf(stderr,"dbg2       ndumped:     %d\n",ndumped);
		fprintf(stderr,"dbg2       nloaded:     %d\n",nloaded);
		fprintf(stderr,"dbg2       icurrent:    %d\n",icurrent);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbwedge_action_plot()
{
	/* local variables */
	char	*function_name = "mbwedge_action_plot";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		fprintf(stderr,"dbg2       nbuffer:     %d\n",nbuffer);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nbuffer:     %d\n",nbuffer);
		fprintf(stderr,"dbg2       ndumped:     %d\n",ndumped);
		fprintf(stderr,"dbg2       nloaded:     %d\n",nloaded);
		fprintf(stderr,"dbg2       icurrent:    %d\n",icurrent);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbwedge_action_quit()
{
	/* local variables */
	char	*function_name = "mbwedge_action_quit";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		fprintf(stderr,"dbg2       nbuffer:     %d\n",nbuffer);
		}

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\nShutting MBwedge down without further ado...\n");

	/* call routine to deal with saving the current file, if any */
	if (file_open == MB_YES)
		status = mbwedge_action_close();

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\nQuitting MBwedge\nBye Bye...\n");

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndumped:     %d\n",ndumped);
		fprintf(stderr,"dbg2       nbuffer:     %d\n",nbuffer);
		fprintf(stderr,"dbg2       nloaded:     %d\n",nloaded);
		fprintf(stderr,"dbg2       icurrent:    %d\n",icurrent);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
