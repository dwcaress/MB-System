/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit_prog.c	6/23/95
 *    $Id: mbnavedit_prog.c,v 5.6 2008-09-13 06:08:09 caress Exp $
 *
 *    Copyright (c) 1995, 2000, 2003 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *    David W. Caress (caress@mbari.org)
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBNAVEDIT is an interactive navigation editor for swath sonar data.
 * It can work with any data format supported by the MBIO library.
 * This file contains the code that does not directly depend on the 
 * MOTIF interface - the companion files mbnavedit.c and 
 * mbnavedit_callbacks.c contain the user interface related code.
 *
 * Author:	D. W. Caress
 * Date:	June 23,  1995
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.5  2007/10/08 16:18:36  caress
 * Renamed plot variables.
 *
 * Revision 5.4  2006/01/27 19:12:12  caress
 * Version 5.0.8beta2
 *
 * Revision 5.3  2003/04/17 21:10:27  caress
 * Release 5.0.beta30
 *
 * Revision 5.2  2001/07/20 00:34:16  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/01/22 07:49:44  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:56:26  caress
 * First cut at Version 5.0.
 *
 * Revision 4.23  2000/10/11  01:05:41  caress
 * Convert to ANSI C
 *
 * Revision 4.22  2000/09/30  07:04:44  caress
 * Snapshot for Dale.
 *
 * Revision 4.21  2000/09/30  07:04:05  caress
 * Snapshot for Dale.
 *
 * Revision 4.20  2000/09/07  20:54:10  caress
 * Made buffer calls compatible with new routines accessing draft values.
 *
 * Revision 4.19  2000/08/28  22:45:11  caress
 * About to kick off new version.
 *
 * Revision 4.18  1999/12/11 04:42:03  caress
 * Moved xgraphics.c to src/xgraphics
 *
 * Revision 4.17  1999/11/12  21:25:21  caress
 * Fixed start date shown below plots
 *
 * Revision 4.16  1999/07/16  19:22:56  caress
 * Smaller window with new dialogs for Linux.
 *
 * Revision 4.15  1999/04/14 04:33:10  caress
 * Final (?) version 4.6 release
 *
 * Revision 4.14  1999/04/09  22:34:08  caress
 * Added time interval plot.
 *
 * Revision 4.13  1999/02/04  23:54:13  caress
 * MB-System version 4.6beta7
 *
 * Revision 4.12  1998/12/18  01:40:25  caress
 * MB-System version 4.6beta5
 *
 * Revision 4.11  1998/10/09  18:43:08  caress
 * MB-System version 4.6beta.
 *
 * Revision 4.10  1997/09/15  19:10:20  caress
 * Real Version 4.5
 *
 * Revision 4.9  1997/04/22  19:25:57  caress
 * Fixed startup mode.
 *
 * Revision 4.8  1997/04/21  17:07:38  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.8  1997/04/16  21:40:29  caress
 * Version for MB-System 4.5.
 *
 * Revision 4.7  1996/08/26  17:33:15  caress
 * Release 4.4 revision.
 *
 * Revision 4.6  1996/04/22  13:22:24  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.5  1996/04/05  20:07:02  caress
 * Added GUI mode so done means quit for real. Also changed done and
 * quit handling in browse mode so that the program doesn't read the
 * entire data file before closing it.
 *
 * Revision 4.4  1995/11/02  19:22:45  caress
 *  Fixed mb_error calls.
 *
 * Revision 4.3  1995/09/28  18:01:01  caress
 * Improved handling of .mbxxx file suffix convention.
 *
 * Revision 4.2  1995/09/18  22:40:44  caress
 * Fixed bug that caused "select all" function to miss last data
 * point on plots of entire data set.
 *
 * Revision 4.1  1995/08/17  14:58:12  caress
 * Revision for release 4.3.
 *
 * Revision 4.0  1995/08/07  18:33:22  caress
 * First cut.
 *
 *
 */

/*--------------------------------------------------------------------*/

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* MBIO include files */
#include "mb_format.h"
#include "mb_status.h"
#include "mb_define.h"
#include "mb_io.h"

/* define global control parameters */
#include "mbnavedit.h"

/* ping structure definition */
struct mbnavedit_ping_struct 
	{
	int	id;
	int	record;
	int	time_i[7];
	double	time_d;
	double	file_time_d;
	double	tint;
	double	lon;
	double	lat;
	double	speed;
	double	heading;
	double	draft;
	double	roll;
	double	pitch;
	double	heave;
	double	time_d_org;
	double	tint_org;
	double	lon_org;
	double	lat_org;
	double	lon_dr;
	double	lat_dr;
	double	speed_org;
	double	heading_org;
	double	speed_made_good;
	double	course_made_good;
	int	tint_x;
	int	tint_y;
	int	lon_x;
	int	lon_y;
	int	lat_x;
	int	lat_y;
	int	speed_x;
	int	speed_y;
	int	heading_x;
	int	heading_y;
	int	tint_select;
	int	lon_select;
	int	lat_select;
	int	speed_select;
	int	heading_select;
	};

/* plot structure definition */
struct mbnavedit_plot_struct 
	{
	int	type;
	int	ixmin;
	int	ixmax;
	int	iymin;
	int	iymax;
	double	xmin;
	double	xmax;
	double	ymin;
	double	ymax;
	double	xscale;
	double	yscale;
	double	xinterval;
	double	yinterval;
	char	xlabel[128];
	char	ylabel1[128];
	char	ylabel2[128];
	};

/* id variables */
static char rcs_id[] = "$Id: mbnavedit_prog.c,v 5.6 2008-09-13 06:08:09 caress Exp $";
static char program_name[] = "MBNAVEDIT";
static char help_message[] =  "MBNAVEDIT is an interactive navigation editor for swath sonar data.\n\tIt can work with any data format supported by the MBIO library.\n";
static char usage_message[] = "mbnavedit [-Byr/mo/da/hr/mn/sc -D  -Eyr/mo/da/hr/mn/sc \n\t-Fformat -Ifile -Ooutfile -V -H]";

/* status variables */
int	error = MB_ERROR_NO_ERROR;
int	verbose = 0;
char	*message = NULL;

/* MBIO control parameters */
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
void	*imbio_ptr = NULL;
void	*ombio_ptr = NULL;

/* mbio read and write values */
void	*store_ptr = NULL;
int	kind;
int	time_i[7];
double	time_d;
double	navlon;
double	navlat;
double	speed;
double	heading;
double	draft;
double	distance;
int	nbath;
int	namp;
int	nss;
char		*beamflag = NULL;
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
#define	MBNAVEDIT_BUFFER_SIZE	MB_BUFFER_MAX
int	file_open = MB_NO;
int	nfile_open = MB_NO;
FILE	*nfp;
void	*buff_ptr = NULL;
int	buffer_size = MBNAVEDIT_BUFFER_SIZE;
int	hold_size = 100;
int	nload = 0;
int	ndump = 0;
int	nbuff = 0;
int	nlist = 0;
int	current = 0;
int	current_id = 0;
int	nload_total = 0;
int	ndump_total = 0;
int	first_read = MB_NO;

/* plotting control variables */
#define	NUMBER_PLOTS_MAX	8
#define	DEFAULT_PLOT_WIDTH	767
#define	DEFAULT_PLOT_HEIGHT	300
#define	MBNAVEDIT_PICK_DISTANCE		50
#define	MBNAVEDIT_ERASE_DISTANCE	10
struct mbnavedit_ping_struct	ping[MBNAVEDIT_BUFFER_SIZE];
int	list[MBNAVEDIT_BUFFER_SIZE];
double	plot_start_time;
double	plot_end_time;
int	nplot;
int	mbnavedit_xgid;
struct mbnavedit_plot_struct mbnavplot[NUMBER_PLOTS_MAX];
int	data_save;
double	file_start_time_d;

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

/*--------------------------------------------------------------------*/
int mbnavedit_init_globals()
{
	/* local variables */
	char	*function_name = "mbnavedit_init_globals";
	int	status = MB_SUCCESS;

	/* set default global control parameters */
	output_mode = OUTPUT_MODE_OUTPUT;
	gui_mode = MB_NO;
	data_show_max = 2000;
	data_show_size = 1000;
	data_step_max = 2000;
	data_step_size = 750;
	mode_pick = PICK_MODE_PICK;
	mode_set_interval = MB_NO;
	plot_tint = MB_YES;
	plot_tint_org = MB_YES;
	plot_lon = MB_YES;
	plot_lon_org = MB_YES;
	plot_lon_dr = MB_NO;
	plot_lat = MB_YES;
	plot_lat_org = MB_YES;
	plot_lat_dr = MB_NO;
	plot_speed = MB_YES;
	plot_speed_org = MB_YES;
	plot_smg = MB_YES;
	plot_heading = MB_YES;
	plot_heading_org = MB_YES;
	plot_cmg = MB_YES;
	plot_roll = MB_NO;
	plot_pitch = MB_NO;
	plot_heave = MB_NO;
	drift_lon = 0;
	drift_lat = 0;
	strcpy(ifile,"\0");
	strcpy(ofile,"\0");
	ofile_defined = MB_NO;	
	plot_width = DEFAULT_PLOT_WIDTH;
	plot_height = DEFAULT_PLOT_HEIGHT;
	number_plots = 0;
	if (plot_tint == MB_YES)    number_plots++;
	if (plot_lon == MB_YES)	    number_plots++;
	if (plot_lat == MB_YES)	    number_plots++;
	if (plot_speed == MB_YES)   number_plots++;
	if (plot_heading == MB_YES) number_plots++;
	time_fix = MB_NO;
	use_ping_data = MB_NO;

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
int mbnavedit_init(int argc, char **argv, int *startup_file)
{
	/* local variables */
	char	*function_name = "mbnavedit_init";
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
	strcpy(ofile,"\0");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhB:b:DdE:e:F:f:GgI:i:O:o:PpTt")) != -1)
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
			output_mode = OUTPUT_MODE_BROWSE;
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
		case 'P':
		case 'p':
			use_ping_data = MB_YES;
			flag++;
			break;
		case 'T':
		case 't':
			time_fix = MB_YES;
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
		{
		status = mbnavedit_action_open();
		if (status == MB_SUCCESS)
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
int mbnavedit_set_graphics(int xgid, int ncol, int *pixels)
{
	/* local variables */
	char	*function_name = "mbnavedit_set_graphics";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       xgid:         %d\n",xgid);
		fprintf(stderr,"dbg2       ncolors:      %d\n",ncol);
		for (i=0;i<ncol;i++)
			fprintf(stderr,"dbg2       pixel[%d]:     %d\n",
				i, pixels[i]);
		}

	/* set graphics id */
	mbnavedit_xgid = xgid;

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
int mbnavedit_action_open()
{
	/* local variables */
	char	*function_name = "mbnavedit_action_open";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* clear the screen */
	status = mbnavedit_clear_screen();

	/* open the file */
	status = mbnavedit_open_file();

	/* load the buffer */
	if (status == MB_SUCCESS)
		status = mbnavedit_load_data();

	/* keep going until good data or end of file found */
	while (nload > 0 && nlist == 0)
		{
		/* dump the buffer */
		status = mbnavedit_dump_data(hold_size);

		/* load the buffer */
		status = mbnavedit_load_data();
		}

	/* set up plotting */
	if (nlist > 0)
		{
		/* set time span to zero so plotting resets it */
		data_show_size = 0;
		
		/* turn file button off */
		do_filebutton_off();

		/* now plot it */
		status = mbnavedit_plot_all();
		}
		
	/* if no data read show error dialog */
	else
		do_error_dialog("No data were read from the input", 
				"file. You may have specified an", 
				"incorrect MB-System format id!");

	/* reset data_save */
	data_save = MB_NO;

	/* print debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  File open attempted in MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Buffer values:\n");
		fprintf(stderr,"dbg2       nload:       %d\n",ndump);
		fprintf(stderr,"dbg2       nload:       %d\n",nload);
		fprintf(stderr,"dbg2       nbuff:       %d\n",nbuff);
		fprintf(stderr,"dbg2       nlist:       %d\n",nlist);
		fprintf(stderr,"dbg2       current:     %d\n",current);
		fprintf(stderr,"dbg2       current_id:  %d\n",current_id);
		fprintf(stderr,"dbg2       error:       %d\n",error);
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
int mbnavedit_open_file()
{
	/* local variables */
	char	*function_name = "mbnavedit_open_file";
	int	status = MB_SUCCESS;
	char	*mb_suffix;
	char	*sb_suffix;
	int	mb_len;
	int	sb_len;
	int	i;

	/* time, user, host variables */
	time_t	right_now;
	char	date[25], user[128], *user_ptr, host[128];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       file:        %s\n",ifile);
		fprintf(stderr,"dbg2       format:      %d\n",format);
		}

	/* get filenames */
 	if (ofile_defined == MB_NO
		&& output_mode == OUTPUT_MODE_OUTPUT)
		{
		mbedit_get_output_file(ifile,ofile,&format);
		}

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
	if (output_mode == OUTPUT_MODE_OUTPUT)
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
	status = mb_mallocd(verbose,__FILE__,__LINE__,beams_bath*sizeof(char),(void **)&beamflag,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,beams_bath*sizeof(double),(void **)&bath,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,beams_amp*sizeof(double),(void **)&amp,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,beams_bath*sizeof(double),
			(void **)&bathacrosstrack,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,beams_bath*sizeof(double),
			(void **)&bathalongtrack,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,pixels_ss*sizeof(double),(void **)&ss,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,pixels_ss*sizeof(double),
			(void **)&ssacrosstrack,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,pixels_ss*sizeof(double),
			(void **)&ssalongtrack,&error);

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
	first_read = MB_NO;

	/* reset plotting time span */
	plot_start_time = 0.0;
	plot_end_time = data_show_size;

	/* write comments to beginning of output file */
	if (output_mode == OUTPUT_MODE_OUTPUT)
		{
		kind = MB_DATA_COMMENT;
		strncpy(comment,"\0",256);
		sprintf(comment,"Navigation data edited interactively using program %s version %s",
			program_name,rcs_id);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"MB-system Version %s",MB_VERSION);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
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
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"Control Parameters:");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  MBIO data format:   %d",format);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  Input file:         %s",ifile);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  Output file:        %s",ofile);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment," ");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
		
	/* now deal with new nav save file */
	nfile_open = MB_NO;
	if (status == MB_SUCCESS
		&& output_mode != OUTPUT_MODE_BROWSE)
		{
		/* get nav edit save file */
		sprintf(nfile, "%s.nve", ifile);
		
		/* open the nav edit save file */
		if ((nfp = fopen(nfile,"w")) != NULL)
		    {
		    nfile_open = MB_YES;
		    fprintf(stderr, "\nOpened new nav edit save file %s\n",
			nfile);
		    }
		else
		    {
		    nfile_open = MB_NO;
		    fprintf(stderr, "\nUnable to open new nav save file %s\n",
			nfile);
		    do_error_dialog("Unable to open new nav edit save file.",
				    "You may not have write",
				    "permission in this directory!");
		    }
		}

	/* if we got here we must have succeeded */
	if (verbose >= 1)
		{
		fprintf(stderr,"\nMultibeam File <%s> initialized for reading\n",ifile);
		if (output_mode == OUTPUT_MODE_OUTPUT)
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
int mbedit_get_output_file(char *file1, char *file2, int *form)
{
	/* local variables */
	char	*function_name = "mbedit_get_output_file";
	int	status = MB_SUCCESS;
	char	mb_suffix[32];
	int	tform;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       file1:       %s\n",file1);
		fprintf(stderr,"dbg2       format:      %d\n",*form);
		}

	/* get filenames */
	/* look for MB suffix convention */
	if ((status = mb_get_format(verbose, file1, file2,
				    &tform, &error))
				    == MB_SUCCESS)
	    {
	    if (strstr(file2, "_") != NULL)
		    strcat(file2, "n");
	    else
		    strcat(file2,"_n");
	    *form = tform;
	    sprintf(mb_suffix, ".mb%d", *form);
	    strcat(file2,mb_suffix);
	    }

	/* else just at ".ned" to file name */
	else
		{
		strcpy(file2,file1);
		strcat(file2,".ned");
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		}
		

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       file2:       %s\n",file2);
		fprintf(stderr,"dbg2       format:      %d\n",*form);
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavedit_close_file()
{
	/* local variables */
	char	*function_name = "mbnavedit_close_file";
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
	if (nfile_open == MB_YES)
	    {
	    fclose(nfp);
	    nfile_open = MB_NO;
	    }

	/* deallocate memory for data arrays */
	mb_freed(verbose,__FILE__,__LINE__,(void **)&beamflag,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&bath,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&amp,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&bathacrosstrack,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&bathalongtrack,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&ss,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&ssacrosstrack,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&ssalongtrack,&error);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* if we got here we must have succeeded */
	if (verbose >= 1)
		{
		fprintf(stderr,"\nMultibeam Input File <%s> closed\n",ifile);
		if (output_mode == OUTPUT_MODE_OUTPUT)
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
int mbnavedit_dump_data(int hold)
{
	/* local variables */
	char	*function_name = "mbnavedit_dump_data";
	int	status = MB_SUCCESS;
	int	iping;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       hold:       %d\n",hold);
		}

	/* insert changed data into buffer */
	for (iping=0;iping<nlist;iping++)
		{
		if (ping[iping].time_d != ping[iping].time_d_org
			|| ping[iping].lon != ping[iping].lon_org
			|| ping[iping].lat != ping[iping].lat_org
			|| ping[iping].speed != ping[iping].speed_org
			|| ping[iping].heading != ping[iping].heading_org
			|| time_fix == MB_YES)
			{
			status = mb_buffer_insert_nav(verbose,
				buff_ptr,imbio_ptr,ping[iping].id,
				ping[iping].time_i,ping[iping].time_d,
				ping[iping].lon,ping[iping].lat,
				ping[iping].speed,ping[iping].heading,
				ping[iping].draft,
				ping[iping].roll,ping[iping].pitch, 
				ping[iping].heave, 
				&error);
			}
		if (nfile_open == MB_YES
			&& iping < nlist - hold)
			{
			/* write the nav out */
		    	fprintf(nfp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f\r\n",
				ping[iping].time_i[0],
				ping[iping].time_i[1],
				ping[iping].time_i[2],
				ping[iping].time_i[3],
				ping[iping].time_i[4],
				ping[iping].time_i[5],
				ping[iping].time_i[6],
				ping[iping].time_d,
				ping[iping].lon,
				ping[iping].lat,
				ping[iping].heading,
				ping[iping].speed);
			}
		}

	/* dump or clear data from the buffer */
	ndump = 0;
	if (nbuff > 0)
		{
		if (output_mode == OUTPUT_MODE_OUTPUT)
			{
			/* turn message on */
			do_message_on("MBnaveditoldold is dumping data...");

			status = mb_buffer_dump(verbose,
					buff_ptr,ombio_ptr,
					hold,&ndump,&nbuff,
					&error);
			}
		else
			{
			/* turn message on */
			do_message_on("MBnaveditold is clearing data...");

			status = mb_buffer_clear(verbose,
					buff_ptr,imbio_ptr,
					hold,&ndump,&nbuff,
					&error);
			}

		/* turn message off */
		do_message_off();
		}
	ndump_total += ndump;

	/* reset current data pointer */
	if (ndump > 0)
		current = current - ndump;
	if (current < 0)
		current = 0;
	if (current > nbuff - 1)
		current = nbuff - 1;

	/* flag lack of indexing */
	nlist = 0;

	/* print out information */
	if (verbose >= 1)
		{
		if (output_mode == OUTPUT_MODE_OUTPUT)
			fprintf(stderr,"\n%d data records dumped to output file <%s>\n",
				ndump,ofile);
		else
			fprintf(stderr,"\n%d data records dumped from buffer\n",
				ndump);
		fprintf(stderr,"%d data records remain in buffer\n",nbuff);
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
int mbnavedit_load_data()
{
	/* local variables */
	char	*function_name = "mbnavedit_load_data";
	int	status = MB_SUCCESS;
	int	i;
	int	start;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}
		
	/* turn message on */
	do_message_on("MBnaveditold is loading data...");

	/* load data into buffer */
	status = mb_buffer_load(verbose,buff_ptr,imbio_ptr,buffer_size,
			&nload,&nbuff,&error);
	nload_total += nload;

	/* read data in and set up index of pings */
	nlist = 0;
	start = 0;
	list[0] = 0;
	if (status == MB_SUCCESS) 
	do
		{
		if (use_ping_data == MB_NO)
		    status = mb_buffer_get_next_nav(verbose,
			buff_ptr,imbio_ptr,start,&ping[nlist].id,
			ping[nlist].time_i,&ping[nlist].time_d,
			&ping[nlist].lon,&ping[nlist].lat,
			&ping[nlist].speed,&ping[nlist].heading,
			&ping[nlist].draft,
			&ping[nlist].roll,&ping[nlist].pitch,
			&ping[nlist].heave,
			&error);
		else
		    {
		    status = mb_buffer_get_next_data(verbose,
			buff_ptr,imbio_ptr,start,&ping[nlist].id,
			ping[nlist].time_i,&ping[nlist].time_d,
			&ping[nlist].lon,&ping[nlist].lat,
			&ping[nlist].speed,&ping[nlist].heading,
			&nbath,&namp,&nss,
			beamflag,bath,amp,  
			bathacrosstrack,bathalongtrack, 
			ss,ssacrosstrack,ssalongtrack,
			&error);
		    status = mb_buffer_extract_nav(verbose,
			buff_ptr,imbio_ptr,ping[nlist].id,&kind,
			ping[nlist].time_i,&ping[nlist].time_d,
			&ping[nlist].lon,&ping[nlist].lat,
			&ping[nlist].speed,&ping[nlist].heading,
			&ping[nlist].draft,
			&ping[nlist].roll,&ping[nlist].pitch,
			&ping[nlist].heave,
			&error);
		    }
		if (status == MB_SUCCESS)
			{
			/* get first time value if first record */
			if (first_read == MB_NO)
				{
				file_start_time_d = ping[nlist].time_d;
				first_read = MB_YES;
				}

			/* get original values */
			ping[nlist].record = ping[nlist].id + ndump_total;
			ping[nlist].lon_org = ping[nlist].lon;
			ping[nlist].lat_org = ping[nlist].lat;
			ping[nlist].speed_org = ping[nlist].speed;
			ping[nlist].heading_org = ping[nlist].heading;
			ping[nlist].file_time_d = 
				ping[nlist].time_d - file_start_time_d;

			/* set everything deselected */
			ping[nlist].tint_select = MB_NO;
			ping[nlist].lon_select = MB_NO;
			ping[nlist].lat_select = MB_NO;
			ping[nlist].speed_select = MB_NO;
			ping[nlist].heading_select = MB_NO;

			/* print output debug statements */
			if (verbose >= 5)
				{
				fprintf(stderr,"\ndbg5  Next good data found in function <%s>:\n",
					function_name);
				fprintf(stderr,"dbg5       %4d %4d %4d  %d/%d/%d %2.2d:%2.2d:%2.2d.%6.6d  %11.6f %11.6f %5.2f %5.1f %5.2f %5.2f %5.2f\n",
					nlist,ping[nlist].id,ping[nlist].record,
					ping[nlist].time_i[1],ping[nlist].time_i[2],
					ping[nlist].time_i[0],ping[nlist].time_i[3],
					ping[nlist].time_i[4],ping[nlist].time_i[5],
					ping[nlist].time_i[6],
					ping[nlist].lon, ping[nlist].lat, 
					ping[nlist].speed, ping[nlist].heading, 
					ping[nlist].roll, ping[nlist].pitch, 
					ping[nlist].heave);
				}

			/* increment counting variables */
			start = ping[nlist].id + 1;
			list[nlist] = ping[nlist].id;
			nlist++;
			}
		}
	while (status == MB_SUCCESS);

	/* define success */
	if (nlist > 0)
		{
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		}
		
	/* if requested fix time stamp repeats */
	if (time_fix == MB_YES)
		mbnavedit_action_fixtime();
		
	/* calculate expected time */
	if (nlist > 1)
	    {
	    for (i=1;i<nlist;i++)
		{
		ping[i].tint = ping[i].time_d - ping[i-1].time_d;
		ping[i].tint_org = ping[i].tint;
		ping[i].time_d_org = ping[i].time_d;
		}
	    ping[0].tint = ping[1].tint;
	    ping[0].tint_org = ping[1].tint_org;
	    ping[0].time_d_org = ping[0].time_d;
	    }
	else if (nlist == 0)
	    {
	    ping[0].tint = 0.0;
	    ping[0].tint_org = 0.0;
	    ping[0].time_d_org = ping[0].time_d;
	    }

	/* calculate speed-made-good and course-made-good */
	for (i=0;i<nlist;i++)
		mbnavedit_get_smgcmg(i);
		
	/* calculate dead reckoning */
	mbnavedit_get_dr();

	/* find index of current ping */
	current_id = 0;
	for (i=0;i<nlist;i++)
		{
		if (list[i] <= current)
			current_id = i;
		}
	current = list[current_id];

	/* reset plotting time span */
/*	if (data_show_size > 0 && nlist > 0)
		{
		plot_start_time = ping[current_id].file_time_d;
		plot_end_time = plot_start_time + data_show_size;
		}
	else if (nlist > 0)
		{
		plot_start_time = ping[0].file_time_d;
		plot_end_time = ping[nlist-1].file_time_d;
		}*/
	if (nlist > 0)
		{
		data_show_size = 0;
		plot_start_time = ping[0].file_time_d;
		plot_end_time = ping[nlist-1].file_time_d;
		}
		
	/* turn message off */
	do_message_off();

	/* print out information */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d data records loaded from input file <%s>\n",
			nload,ifile);
		fprintf(stderr,"%d data records now in buffer\n",nbuff);
		fprintf(stderr,"%d editable navigation records now in buffer\n",nlist);
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
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbnavedit_clear_screen()
{
	/* local variables */
	char	*function_name = "mbnavedit_clear_screen";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* clear screen */
	xg_fillrectangle(mbnavedit_xgid,0,0,
		plot_width,NUMBER_PLOTS_MAX*plot_height,
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
int mbnavedit_action_next_buffer(int *quit)
{
	/* local variables */
	char	*function_name = "mbnavedit_action_next_buffer";
	int	status = MB_SUCCESS;
	int	save_dumped = 0;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* clear the screen */
	status = mbnavedit_clear_screen();
	
	/* set quit off */
	*quit = MB_NO;

	/* check if a file has been opened */
	if (file_open == MB_YES)
		{

		/* keep going until good data or end of file found */
		do
			{
			/* dump the buffer */
			status = mbnavedit_dump_data(hold_size);

			/* load the buffer */
			status = mbnavedit_load_data();
			}
		while (nload > 0 && nlist == 0);

		/* if end of file reached then 
			dump last buffer and close file */
		if (nload <= 0)
			{
			save_dumped = ndump;
			status = mbnavedit_dump_data(0);
			status = mbnavedit_close_file();
			ndump = ndump + save_dumped;
				
			/* if in normal mode last next_buffer 
				does not mean quit,
				if in gui mode it does mean quit */
			if (gui_mode == MB_YES)
				*quit = MB_YES;
			else
				*quit = MB_NO;
		
			/* if quitting let the world know... */
			if (*quit == MB_YES && verbose >= 1)
				fprintf(stderr,"\nQuitting MBnaveditold\nBye Bye...\n");
			}

		/* else plot it */
		else
			{
			status = mbnavedit_plot_all();
			}
		}

	/* if no file open set failure status */
	else
		{
		status = MB_FAILURE;
		ndump = 0;
		nload = 0;
		current_id = 0;
		current = 0;
		}

	/* reset data_save */
	data_save = MB_NO;


	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       quit:        %d\n",*quit);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavedit_action_close()
{
	/* local variables */
	char	*function_name = "mbnavedit_action_close";
	int	status = MB_SUCCESS;
	int	save_nloaded = 0;
	int	save_ndumped = 0;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* clear the screen */
	status = mbnavedit_clear_screen();

	/* if file has been opened and browse mode 
		just dump the current buffer and close the file */
	if (file_open == MB_YES 
		&& output_mode == OUTPUT_MODE_BROWSE)
		{
		/* dump the buffer */
		status = mbnavedit_dump_data(0);
		save_ndumped = save_ndumped + ndump;
		ndump = save_ndumped;
		nload = save_nloaded;

		/* now close the file */
		status = mbnavedit_close_file();
		}
	/* if file has been opened deal with it */
	else if (file_open == MB_YES)
		{

		/* dump and load until the end of the file is reached */
		do
			{
			/* dump the buffer */
			status = mbnavedit_dump_data(0);
			save_ndumped = save_ndumped + ndump;

			/* load the buffer */
			status = mbnavedit_load_data();
			save_nloaded = save_nloaded + nload;
			}
		while (nload > 0);
		ndump = save_ndumped;
		nload = save_nloaded;

		/* now close the file */
		status = mbnavedit_close_file();
		}

	else
		{
		ndump = 0;
		nload = 0;
		nbuff = 0;
		nlist = 0;
		current = 0;
		status = MB_FAILURE;
		}

	/* reset data_save */
	data_save = MB_NO;

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
int mbnavedit_action_done(int *quit)
{
	/* local variables */
	char	*function_name = "mbnavedit_action_done";
	int	status = MB_SUCCESS;
	int	save_nloaded = 0;
	int	save_ndumped = 0;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}
		
	/* if in normal mode done does not mean quit,
		if in gui mode done does mean quit */
	if (gui_mode == MB_YES)
		*quit = MB_YES;
	else
		*quit = MB_NO;

	/* if quitting let the world know... */
	if (*quit == MB_YES && verbose >= 1)
		fprintf(stderr,"\nShutting MBnaveditold down without further ado...\n");

	/* call routine to deal with saving the current file, if any */
	if (file_open == MB_YES)
		status = mbnavedit_action_close();

	/* if quitting let the world know... */
	if (*quit == MB_YES && verbose >= 1)
		fprintf(stderr,"\nQuitting MBnaveditold\nBye Bye...\n");

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       quit:        %d\n",*quit);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavedit_action_quit()
{
	/* local variables */
	char	*function_name = "mbnavedit_action_quit";
	int	status = MB_SUCCESS;
	int	save_nloaded = 0;
	int	save_ndumped = 0;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\nShutting MBnaveditold down without further ado...\n");

	/* call routine to deal with saving the current file, if any */
	if (file_open == MB_YES)
		status = mbnavedit_action_close();

	/* let the world know... */
	if (verbose >= 1)
		fprintf(stderr,"\nQuitting MBnaveditold\nBye Bye...\n");

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
int mbnavedit_action_step(int step)
{
	/* local variables */
	char	*function_name = "mbnavedit_action_step";
	int	status = MB_SUCCESS;
	int	old_id, new_id;
	int	set;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       step:       %d\n",step);
		}

	/* check if a file has been opened */
	if (file_open == MB_YES && nlist > 0)
		{

		/* if current time span includes last data don't step */
		if (step >= 0 
			&& plot_end_time < ping[nlist-1].file_time_d)
			{
			plot_start_time = plot_start_time + step;
			plot_end_time = plot_start_time + data_show_size;
			}
		else if (step < 0 
			&& plot_start_time > ping[0].file_time_d)
			{
			plot_start_time = plot_start_time + step;
			plot_end_time = plot_start_time + data_show_size;
			}

		/* get current start of plotting data */
		set = MB_NO;
		old_id = current_id;
		for (i=0;i<nlist;i++)
			{
			if (set == MB_NO 
				&& ping[i].file_time_d 
				>= plot_start_time)
				{
				new_id = i;
				set = MB_YES;
				}
			}
		if (new_id < 0)
			new_id = 0;
		if (new_id >= nlist)
			new_id = nlist - 1;
		current_id = new_id;
		current = list[current_id];

		/* replot */
		if (nlist > 0)
			{
			status = mbnavedit_plot_all();
			}

		/* set failure flag if no step was made */
		if (new_id == old_id)
			status = MB_FAILURE;
		}

	/* if no file open set failure status */
	else
		{
		status = MB_FAILURE;
		current_id = 0;
		current = 0;
		}

	/* print out information */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Current buffer values:\n");
		fprintf(stderr,"dbg2       nload:       %d\n",nload);
		fprintf(stderr,"dbg2       nbuff:       %d\n",nbuff);
		fprintf(stderr,"dbg2       nbuff:       %d\n",nbuff);
		fprintf(stderr,"dbg2       nlist:       %d\n",nlist);
		fprintf(stderr,"dbg2       current_id:  %d\n",current_id);
		fprintf(stderr,"dbg2       current:     %d\n",current);
		}

	/* reset data_save */
	data_save = MB_NO;

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
int mbnavedit_action_mouse_pick(int xx, int yy)
{
	/* local variables */
	char	*function_name = "mbnavedit_action_mouse_pick";
	int	status = MB_SUCCESS;
	int	deselect;
	int	iplot;
	int	active_plot;
	int	range, range_min;
	int	iping;
	int	ix, iy;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       xx:         %d\n",xx);
		fprintf(stderr,"dbg2       yy:         %d\n",yy);
		}

	/* don't try to do anything if no data */
	active_plot = -1;
	if (nplot > 0)
		{

		/* figure out which plot the cursor is in */
		for (iplot=0;iplot<number_plots;iplot++)
			{
			if (xx >= mbnavplot[iplot].ixmin
				&& xx <= mbnavplot[iplot].ixmax
				&& yy <= mbnavplot[iplot].iymin
				&& yy >= mbnavplot[iplot].iymax)
				active_plot = iplot;
			}
		}

	/* don't try to do anything if no data or not in plot */
	if (nplot > 0 && active_plot > -1)
	{

	/* deselect everything in non-active plots */
	deselect = MB_NO;
	for (iplot=0;iplot<number_plots;iplot++)
		{
		if (iplot != active_plot)
			{
			status = mbnavedit_action_deselect_all(
					mbnavplot[iplot].type);
			if (status == MB_SUCCESS)
				deselect = MB_YES;
			}
		}

	/* if anything was actually deselected, replot */
	if (deselect == MB_SUCCESS)
		{
		/* clear the screen */
		status = mbnavedit_clear_screen();

		/* replot the screen */
		status = mbnavedit_plot_all();
		}
	status = MB_SUCCESS;

	/* figure out which data point is closest to cursor */
	range_min = 100000;
	for (i=current_id+1;i<current_id+nplot;i++)
		{
		if (mbnavplot[active_plot].type == PLOT_TINT)
			{
			ix = xx - ping[i].tint_x;
			iy = yy - ping[i].tint_y;
			}
		else if (mbnavplot[active_plot].type == PLOT_LONGITUDE)
			{
			ix = xx - ping[i].lon_x;
			iy = yy - ping[i].lon_y;
			}
		else if (mbnavplot[active_plot].type == PLOT_LATITUDE)
			{
			ix = xx - ping[i].lat_x;
			iy = yy - ping[i].lat_y;
			}
		else if (mbnavplot[active_plot].type == PLOT_SPEED)
			{
			ix = xx - ping[i].speed_x;
			iy = yy - ping[i].speed_y;
			}
		else if (mbnavplot[active_plot].type == PLOT_HEADING)
			{
			ix = xx - ping[i].heading_x;
			iy = yy - ping[i].heading_y;
			}
		range = (int) sqrt((double) (ix*ix + iy*iy));
		if (range < range_min)
			{
			range_min = range;
			iping = i;
			}
		}

	/* if it is close enough select or unselect the value
		and replot it */
	if (range_min <= MBNAVEDIT_PICK_DISTANCE)
		{
		if (mbnavplot[active_plot].type == PLOT_TINT)
			{
			if (ping[iping].tint_select == MB_YES)
				ping[iping].tint_select = MB_NO;
			else
				ping[iping].tint_select = MB_YES;
			mbnavedit_plot_tint_value(active_plot,iping); 
			}
		else if (mbnavplot[active_plot].type == PLOT_LONGITUDE)
			{
			if (ping[iping].lon_select == MB_YES)
				ping[iping].lon_select = MB_NO;
			else
				ping[iping].lon_select = MB_YES;
			mbnavedit_plot_lon_value(active_plot,iping); 
			}
		else if (mbnavplot[active_plot].type == PLOT_LATITUDE)
			{
			if (ping[iping].lat_select == MB_YES)
				ping[iping].lat_select = MB_NO;
			else
				ping[iping].lat_select = MB_YES;
			mbnavedit_plot_lat_value(active_plot,iping); 
			}
		else if (mbnavplot[active_plot].type == PLOT_SPEED)
			{
			if (ping[iping].speed_select == MB_YES)
				ping[iping].speed_select = MB_NO;
			else
				ping[iping].speed_select = MB_YES;
			mbnavedit_plot_speed_value(active_plot,iping); 
			}
		else if (mbnavplot[active_plot].type == PLOT_HEADING)
			{
			if (ping[iping].heading_select == MB_YES)
				ping[iping].heading_select = MB_NO;
			else
				ping[iping].heading_select = MB_YES;
			mbnavedit_plot_heading_value(active_plot,iping); 
			}
		}
	}
	/* if no data then set failure flag */
	else
	status = MB_FAILURE;

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
int mbnavedit_action_mouse_select(int xx, int yy)
{
	/* local variables */
	char	*function_name = "mbnavedit_action_mouse_select";
	int	status = MB_SUCCESS;
	int	deselect;
	int	iplot;
	int	active_plot;
	int	range;
	int	ix, iy;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       xx:         %d\n",xx);
		fprintf(stderr,"dbg2       yy:         %d\n",yy);
		}

	/* don't try to do anything if no data */
	active_plot = -1;
	if (nplot > 0)
		{

		/* figure out which plot the cursor is in */
		for (iplot=0;iplot<number_plots;iplot++)
			{
			if (xx >= mbnavplot[iplot].ixmin
				&& xx <= mbnavplot[iplot].ixmax
				&& yy <= mbnavplot[iplot].iymin
				&& yy >= mbnavplot[iplot].iymax)
				active_plot = iplot;
			}
		}

	/* don't try to do anything if no data or not in plot */
	if (nplot > 0 && active_plot > -1)
	{

	/* deselect everything in non-active plots */
	deselect = MB_NO;
	for (iplot=0;iplot<number_plots;iplot++)
		{
		if (iplot != active_plot)
			{
			status = mbnavedit_action_deselect_all(
					mbnavplot[iplot].type);
			if (status == MB_SUCCESS)
				deselect = MB_YES;
			}
		}

	/* if anything was actually deselected, replot */
	if (deselect == MB_SUCCESS)
		{
		/* clear the screen */
		status = mbnavedit_clear_screen();

		/* replot the screen */
		status = mbnavedit_plot_all();
		}
	status = MB_SUCCESS;

	/* find all data points that are close enough */
	for (i=current_id;i<current_id+nplot;i++)
		{
		if (mbnavplot[active_plot].type == PLOT_TINT)
			{
			ix = xx - ping[i].tint_x;
			iy = yy - ping[i].tint_y;
			}
		else if (mbnavplot[active_plot].type == PLOT_LONGITUDE)
			{
			ix = xx - ping[i].lon_x;
			iy = yy - ping[i].lon_y;
			}
		else if (mbnavplot[active_plot].type == PLOT_LATITUDE)
			{
			ix = xx - ping[i].lat_x;
			iy = yy - ping[i].lat_y;
			}
		else if (mbnavplot[active_plot].type == PLOT_SPEED)
			{
			ix = xx - ping[i].speed_x;
			iy = yy - ping[i].speed_y;
			}
		else if (mbnavplot[active_plot].type == PLOT_HEADING)
			{
			ix = xx - ping[i].heading_x;
			iy = yy - ping[i].heading_y;
			}
		range = (int) sqrt((double) (ix*ix + iy*iy));

		/* if it is close enough select the value
			and replot it */
		if (range <= MBNAVEDIT_ERASE_DISTANCE)
			{
			if (mbnavplot[active_plot].type == PLOT_TINT)
				{
				ping[i].tint_select = MB_YES;
				mbnavedit_plot_tint_value(active_plot,i); 
				}
			else if (mbnavplot[active_plot].type == PLOT_LONGITUDE)
				{
				ping[i].lon_select = MB_YES;
				mbnavedit_plot_lon_value(active_plot,i); 
				}
			else if (mbnavplot[active_plot].type == PLOT_LATITUDE)
				{
				ping[i].lat_select = MB_YES;
				mbnavedit_plot_lat_value(active_plot,i); 
				}
			else if (mbnavplot[active_plot].type == PLOT_SPEED)
				{
				ping[i].speed_select = MB_YES;
				mbnavedit_plot_speed_value(active_plot,i); 
				}
			else if (mbnavplot[active_plot].type == PLOT_HEADING)
				{
				ping[i].heading_select = MB_YES;
				mbnavedit_plot_heading_value(active_plot,i); 
				}
			}
		}
	}
	/* if no data then set failure flag */
	else
	status = MB_FAILURE;

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
int mbnavedit_action_mouse_deselect(int xx, int yy)
{
	/* local variables */
	char	*function_name = "mbnavedit_action_mouse_deselect";
	int	status = MB_SUCCESS;
	int	deselect;
	int	iplot;
	int	active_plot;
	int	range;
	int	ix, iy;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       xx:         %d\n",xx);
		fprintf(stderr,"dbg2       yy:         %d\n",yy);
		}

	/* don't try to do anything if no data */
	active_plot = -1;
	if (nplot > 0)
		{

		/* figure out which plot the cursor is in */
		for (iplot=0;iplot<number_plots;iplot++)
			{
			if (xx >= mbnavplot[iplot].ixmin
				&& xx <= mbnavplot[iplot].ixmax
				&& yy <= mbnavplot[iplot].iymin
				&& yy >= mbnavplot[iplot].iymax)
				active_plot = iplot;
			}
		}

	/* don't try to do anything if no data or not in plot */
	if (nplot > 0 && active_plot > -1)
	{

	/* deselect everything in non-active plots */
	deselect = MB_NO;
	for (iplot=0;iplot<number_plots;iplot++)
		{
		if (iplot != active_plot)
			{
			status = mbnavedit_action_deselect_all(
					mbnavplot[iplot].type);
			if (status == MB_SUCCESS)
				deselect = MB_YES;
			}
		}

	/* if anything was actually deselected, replot */
	if (deselect == MB_SUCCESS)
		{
		/* clear the screen */
		status = mbnavedit_clear_screen();

		/* replot the screen */
		status = mbnavedit_plot_all();
		}
	status = MB_SUCCESS;

	/* find all data points that are close enough */
	for (i=current_id;i<current_id+nplot;i++)
		{
		if (mbnavplot[active_plot].type == PLOT_TINT)
			{
			ix = xx - ping[i].tint_x;
			iy = yy - ping[i].tint_y;
			}
		else if (mbnavplot[active_plot].type == PLOT_LONGITUDE)
			{
			ix = xx - ping[i].lon_x;
			iy = yy - ping[i].lon_y;
			}
		else if (mbnavplot[active_plot].type == PLOT_LATITUDE)
			{
			ix = xx - ping[i].lat_x;
			iy = yy - ping[i].lat_y;
			}
		else if (mbnavplot[active_plot].type == PLOT_SPEED)
			{
			ix = xx - ping[i].speed_x;
			iy = yy - ping[i].speed_y;
			}
		else if (mbnavplot[active_plot].type == PLOT_HEADING)
			{
			ix = xx - ping[i].heading_x;
			iy = yy - ping[i].heading_y;
			}
		range = (int) sqrt((double) (ix*ix + iy*iy));

		/* if it is close enough deselect the value
			and replot it */
		if (range <= MBNAVEDIT_ERASE_DISTANCE)
			{
			if (mbnavplot[active_plot].type == PLOT_TINT)
				{
				ping[i].tint_select = MB_NO;
				mbnavedit_plot_tint_value(active_plot,i); 
				}
			else if (mbnavplot[active_plot].type == PLOT_LONGITUDE)
				{
				ping[i].lon_select = MB_NO;
				mbnavedit_plot_lon_value(active_plot,i); 
				}
			else if (mbnavplot[active_plot].type == PLOT_LATITUDE)
				{
				ping[i].lat_select = MB_NO;
				mbnavedit_plot_lat_value(active_plot,i); 
				}
			else if (mbnavplot[active_plot].type == PLOT_SPEED)
				{
				ping[i].speed_select = MB_NO;
				mbnavedit_plot_speed_value(active_plot,i); 
				}
			else if (mbnavplot[active_plot].type == PLOT_HEADING)
				{
				ping[i].heading_select = MB_NO;
				mbnavedit_plot_heading_value(active_plot,i); 
				}
			}
		}
	}
	/* if no data then set failure flag */
	else
	status = MB_FAILURE;

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
int mbnavedit_action_mouse_selectall(int xx, int yy)
{
	/* local variables */
	char	*function_name = "mbnavedit_action_mouse_selectall";
	int	status = MB_SUCCESS;
	int	iplot;
	int	active_plot;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       xx:         %d\n",xx);
		fprintf(stderr,"dbg2       yy:         %d\n",yy);
		}

	/* don't try to do anything if no data */
	active_plot = -1;
	if (nplot > 0)
		{

		/* figure out which plot the cursor is in */
		for (iplot=0;iplot<number_plots;iplot++)
			{
			if (xx >= mbnavplot[iplot].ixmin
				&& xx <= mbnavplot[iplot].ixmax
				&& yy <= mbnavplot[iplot].iymin
				&& yy >= mbnavplot[iplot].iymax)
				active_plot = iplot;
			}
		}

	/* don't try to do anything if no data or not in plot */
	if (nplot > 0 && active_plot > -1)
	{

	/* deselect everything in non-active plots */
	for (iplot=0;iplot<number_plots;iplot++)
		{
		if (iplot != active_plot)
			{
			mbnavedit_action_deselect_all(
					mbnavplot[iplot].type);
			}
		}

	/* select all data points in active plot */
	for (i=current_id;i<current_id+nplot;i++)
		{
		if (mbnavplot[active_plot].type == PLOT_TINT)
			ping[i].tint_select = MB_YES;
		else if (mbnavplot[active_plot].type == PLOT_LONGITUDE)
			ping[i].lon_select = MB_YES;
		else if (mbnavplot[active_plot].type == PLOT_LATITUDE)
			ping[i].lat_select = MB_YES;
		else if (mbnavplot[active_plot].type == PLOT_SPEED)
			ping[i]. speed_select = MB_YES;
		else if (mbnavplot[active_plot].type == PLOT_HEADING)
			ping[i].heading_select = MB_YES;
		}

	/* clear the screen */
	status = mbnavedit_clear_screen();

	/* replot the screen */
	status = mbnavedit_plot_all();

	}
	/* if no data then set failure flag */
	else
	status = MB_FAILURE;

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
int mbnavedit_action_mouse_deselectall(int xx, int yy)
{
	/* local variables */
	char	*function_name = "mbnavedit_action_mouse_deselectall";
	int	status = MB_SUCCESS;
	int	iplot;
	int	active_plot;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       xx:         %d\n",xx);
		fprintf(stderr,"dbg2       yy:         %d\n",yy);
		}

	/* don't try to do anything if no data */
	if (nplot > 0)
	{

	/* deselect all data points in all plots 
		- this logic follows from deselecting all
		active plots plus all non-active plots */
	for (i=current_id;i<current_id+nplot;i++)
		{
		ping[i].tint_select = MB_NO;
		ping[i].lon_select = MB_NO;
		ping[i].lat_select = MB_NO;
		ping[i]. speed_select = MB_NO;
		ping[i].heading_select = MB_NO;
		}

	/* clear the screen */
	status = mbnavedit_clear_screen();

	/* replot the screen */
	status = mbnavedit_plot_all();

	}
	/* if no data then set failure flag */
	else
	status = MB_FAILURE;

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
int mbnavedit_action_deselect_all(int type)
{
	/* local variables */
	char	*function_name = "mbnavedit_action_mouse_deselect_all";
	int	status = MB_SUCCESS;
	int	ndeselect;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       type:       %d\n",type);
		}

	/* don't try to do anything if no data */
	if (nplot > 0)
	{

	/* deselect all data points in specified data type */
	ndeselect = 0;
	for (i=0;i<nlist;i++)
		{
		if (type == PLOT_TINT 
			&& ping[i].tint_select == MB_YES)
			{
			ping[i].tint_select = MB_NO;
			ndeselect++;
			}
		else if (type == PLOT_LONGITUDE 
			&& ping[i].lon_select == MB_YES)
			{
			ping[i].lon_select = MB_NO;
			ndeselect++;
			}
		else if (type == PLOT_LATITUDE 
			&& ping[i].lat_select == MB_YES)
			{
			ping[i].lat_select = MB_NO;
			ndeselect++;
			}
		else if (type == PLOT_SPEED 
			&& ping[i].speed_select == MB_YES)
			{
			ping[i].speed_select = MB_NO;
			ndeselect++;
			}
		else if (type == PLOT_HEADING 
			&& ping[i].heading_select == MB_YES)
			{
			ping[i].heading_select = MB_NO;
			ndeselect++;
			}
		}
	if (ndeselect > 0)
		status = MB_SUCCESS;
	else
		status = MB_FAILURE;
	}
	/* if no data then set failure flag */
	else
	status = MB_FAILURE;

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
int mbnavedit_action_set_interval(int xx, int yy, int which)
{
	/* local variables */
	char	*function_name = "mbnavedit_action_set_interval";
	int	status = MB_SUCCESS;
	static int interval_bound1;
	static int interval_bound2;
	static double interval_time1;
	static double interval_time2;
	static int interval_set1 = MB_NO;
	static int interval_set2 = MB_NO;
	int	itmp;
	double	dtmp;
	int	set;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       xx:         %d\n",xx);
		fprintf(stderr,"dbg2       yy:         %d\n",yy);
		fprintf(stderr,"dbg2       which:      %d\n",which);
		}

	/* don't try to do anything if no data */
	if (nplot > 0 && number_plots > 0)
	    {

	    /* if which = 0 set first bound and draw dashed lines */
	    if (which == 0)
		{
		/* unplot old line on all plots */
		if (interval_set1 == MB_YES)
		for (i=0;i<number_plots;i++)
			{
			xg_drawline(mbnavedit_xgid,
				interval_bound1, 
				mbnavplot[i].iymin, 
				interval_bound1, 
				mbnavplot[i].iymax, 
				pixel_values[WHITE],XG_DASHLINE);
			}

		if (xx < mbnavplot[0].ixmin)
			xx = mbnavplot[0].ixmin;
		if (xx > mbnavplot[0].ixmax)
			xx = mbnavplot[0].ixmax;

		/* get lower bound time and location */
		interval_bound1 = xx;
		interval_time1 = mbnavplot[0].xmin + 
			(xx - mbnavplot[0].ixmin)/mbnavplot[0].xscale;
		interval_set1 = MB_YES;

		/* plot line on all plots */
		for (i=0;i<number_plots;i++)
			{
			xg_drawline(mbnavedit_xgid,
				interval_bound1, 
				mbnavplot[i].iymin, 
				interval_bound1, 
				mbnavplot[i].iymax, 
				pixel_values[RED],XG_DASHLINE);
			}
				
		}

	    /* if which = 1 set second bound and draw dashed lines */
	    else if (which == 1)
		{
		/* unplot old line on all plots */
		if (interval_set1 == MB_YES)
		for (i=0;i<number_plots;i++)
			{
			xg_drawline(mbnavedit_xgid,
				interval_bound2, 
				mbnavplot[i].iymin, 
				interval_bound2, 
				mbnavplot[i].iymax, 
				pixel_values[WHITE],XG_DASHLINE);
			}

		if (xx < mbnavplot[0].ixmin)
			xx = mbnavplot[0].ixmin;
		if (xx > mbnavplot[0].ixmax)
			xx = mbnavplot[0].ixmax;

		/* get lower bound time and location */
		interval_bound2 = xx;
		interval_time2 = mbnavplot[0].xmin + 
			(xx - mbnavplot[0].ixmin)/mbnavplot[0].xscale;
		interval_set2 = MB_YES;

		/* plot line on all plots */
		for (i=0;i<number_plots;i++)
			{
			xg_drawline(mbnavedit_xgid,
				interval_bound2, 
				mbnavplot[i].iymin, 
				interval_bound2, 
				mbnavplot[i].iymax, 
				pixel_values[RED],XG_DASHLINE);
			}
				
		}

	    /* if which = 2 use bounds and replot */
	    else if (which == 2 
		&& interval_set1 == MB_YES 
		&& interval_set2 == MB_YES
		&& interval_bound1 != interval_bound2)
		{
		/* switch bounds if necessary */
		if (interval_bound1 > interval_bound2)
			{
			itmp = interval_bound2;
			dtmp = interval_time2;
			interval_bound2 = interval_bound1;
			interval_time2 = interval_time1;
			interval_bound1 = itmp;
			interval_time1 = dtmp;
			}

		/* reset plotting parameters */
		plot_start_time = interval_time1;
		plot_end_time = interval_time2;
		data_show_size = plot_end_time - plot_start_time;
		
		/* get current start of plotting data */
		set = MB_NO;
		for (i=0;i<nlist;i++)
			{
			if (set == MB_NO && 
				ping[i].file_time_d >= plot_start_time)
				{
				current_id = i;
				set = MB_YES;
				}
			}
		if (current_id < 0)
			current_id = 0;
		if (current_id >= nlist)
			current_id = nlist - 1;
		current = list[current_id];

		/* replot */
		mbnavedit_plot_all();
		}

	    /* else if which = 3 unset bounds */
	    else if (which == 3)
		{
		interval_set1 = MB_NO;
		interval_set2 = MB_NO;
		}

	    /* else failure */
	    else
		status = MB_FAILURE;
	    }
	/* if no data then set failure flag */
	else
	    status = MB_FAILURE;

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
int mbnavedit_action_use_dr()
{
	/* local variables */
	char	*function_name = "mbnavedit_action_use_dr";
	int	status = MB_SUCCESS;
	int	iplot;
	int	active_plot;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* don't try to do anything if no data */
	if (nplot > 0)
	{

	/* make sure either a lon or lat plot is active */
	active_plot = -1;
	for (iplot=0;iplot<number_plots;iplot++)
		{
		if (mbnavplot[iplot].type == PLOT_LONGITUDE)
			active_plot = iplot;
		else if (mbnavplot[iplot].type == PLOT_LATITUDE)
			active_plot = iplot;
		}

	/* set lonlat to dr lonlat for selected visible data */
	if (active_plot > -1)
		{
		for (i=current_id;i<current_id+nplot;i++)
			{
			if (ping[i].lon_select == MB_YES
			    || ping[i].lat_select == MB_YES)
				{
				ping[i].lon = ping[i].lon_dr;
				ping[i].lat = ping[i].lat_dr;
				}
			}

		/* calculate speed-made-good and course-made-good */
		for (i=0;i<nlist;i++)
			mbnavedit_get_smgcmg(i);

		/* clear the screen */
		status = mbnavedit_clear_screen();

		/* replot the screen */
		status = mbnavedit_plot_all();
		}
		
	else
		status = MB_FAILURE;

	}
	/* if no data then set failure flag */
	else
	status = MB_FAILURE;

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
int mbnavedit_action_use_smg()
{
	/* local variables */
	char	*function_name = "mbnavedit_action_use_smg";
	int	status = MB_SUCCESS;
	int	iplot;
	int	active_plot;
	int	speedheading_change;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* don't try to do anything if no data */
	if (nplot > 0)
	{


	/* figure out which plot is speed */
	active_plot = -1;
	for (iplot=0;iplot<number_plots;iplot++)
		{
		if (mbnavplot[iplot].type == PLOT_SPEED)
			active_plot = iplot;
		}

	/* set speed to speed made good for selected visible data */
	if (active_plot > -1)
		{
		speedheading_change = MB_NO;
		for (i=current_id;i<current_id+nplot;i++)
			{
			if (ping[i].speed_select == MB_YES)
				{
				ping[i].speed = ping[i].speed_made_good;
				speedheading_change = MB_YES;
				}
			}

		/* recalculate dead reckoning */
		if (speedheading_change == MB_YES)
			mbnavedit_get_dr(i);

		/* clear the screen */
		status = mbnavedit_clear_screen();

		/* replot the screen */
		status = mbnavedit_plot_all();
		}
		
	else
		status = MB_FAILURE;

	}
	/* if no data then set failure flag */
	else
	status = MB_FAILURE;

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
int mbnavedit_action_use_cmg()
{
	/* local variables */
	char	*function_name = "mbnavedit_action_use_cmg";
	int	status = MB_SUCCESS;
	int	iplot;
	int	active_plot;
	int	speedheading_change;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* don't try to do anything if no data */
	if (nplot > 0)
	{


	/* figure out which plot is heading */
	active_plot = -1;
	for (iplot=0;iplot<number_plots;iplot++)
		{
		if (mbnavplot[iplot].type == PLOT_HEADING)
			active_plot = iplot;
		}

	/* set heading to course made good for selected visible data */
	if (active_plot > -1)
		{
		speedheading_change = MB_NO;
		for (i=current_id;i<current_id+nplot;i++)
			{
			if (ping[i].heading_select == MB_YES)
				{
				ping[i].heading = ping[i].course_made_good;
				speedheading_change = MB_YES;
				}
			}

		/* recalculate dead reckoning */
		if (speedheading_change == MB_YES)
			mbnavedit_get_dr(i);
			
		/* clear the screen */
		status = mbnavedit_clear_screen();

		/* replot the screen */
		status = mbnavedit_plot_all();
		}
		
	else
		status = MB_FAILURE;

	}
	/* if no data then set failure flag */
	else
	status = MB_FAILURE;

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
int mbnavedit_action_interpolate()
{
	/* local variables */
	char	*function_name = "mbnavedit_action_interpolate";
	int	status = MB_SUCCESS;
	int	iplot;
	int	iping;
	int	ibefore, iafter;
	int	timelonlat_change;
	int	speedheading_change;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* don't try to do anything if no data */
	if (nplot > 0)
	{
	/* look for position or time changes */
	timelonlat_change = MB_NO;
	speedheading_change = MB_NO;

	/* do expected time */
	for (iping=0;iping<nlist;iping++)
	    {
	    if (ping[iping].tint_select == MB_YES)
		{
		ibefore = iping;
		for (i=iping-1;i>=0;i--)
		    if (ping[i].tint_select == MB_NO
			&& ibefore == iping)
			ibefore = i;
		iafter = iping;
		for (i=iping+1;i<nlist;i++)
		    if (ping[i].tint_select == MB_NO
			&& iafter == iping)
			iafter = i;
		if (ibefore < iping && iafter > iping)
		    {
		    ping[iping].time_d = ping[ibefore].time_d 
			+ (ping[iafter].time_d - ping[ibefore].time_d)
			*((double)(iping - ibefore))
			    /	((double)(iafter - ibefore));
		    ping[iping].tint = ping[iping].time_d 
					- ping[iping-1].time_d;
		    timelonlat_change = MB_YES;
		    }
		else if (ibefore < iping && ibefore > 0)
		    {
		    ping[iping].time_d = ping[ibefore].time_d 
			+ (ping[ibefore].time_d - ping[ibefore-1].time_d)
			* (iping - ibefore);
		    ping[iping].tint = ping[iping].time_d 
					- ping[iping-1].time_d;
		    timelonlat_change = MB_YES;
		    }
		else if (ibefore < iping)
		    {
		    ping[iping].time_d = ping[ibefore].time_d;
		    ping[iping].tint = ping[iping].time_d 
					- ping[iping-1].time_d;
		    timelonlat_change = MB_YES;
		    }
		else if (iafter > iping && iafter < nlist - 1)
		    {
		    ping[iping].time_d = ping[iafter].time_d 
			+ (ping[iafter+1].time_d - ping[iafter].time_d)
			*(iping - iafter);
		    ping[iping].tint = 0.0;
		    timelonlat_change = MB_YES;
		    }
		else if (iafter > iping)
		    {
		    ping[iping].time_d = ping[iafter].time_d;
		    ping[iping].tint = ping[iping].time_d 
					- ping[iping-1].time_d;
		    timelonlat_change = MB_YES;
		    }
		ping[iping].file_time_d = 
			ping[iping].time_d - file_start_time_d;
		status = mb_get_date(verbose, ping[iping].time_d, ping[iping].time_i);
		if (iping < nlist - 1)
		    if (ping[iping+1].tint_select == MB_NO)
			ping[iping+1].tint = ping[iping+1].time_d 
						- ping[iping].time_d;
		}
	    }

	/* do longitude */
	for (iping=0;iping<nlist;iping++)
	    {
	    if (ping[iping].lon_select == MB_YES)
		{
		ibefore = iping;
		for (i=iping-1;i>=0;i--)
		    if (ping[i].lon_select == MB_NO
			&& ibefore == iping)
			ibefore = i;
		iafter = iping;
		for (i=iping+1;i<nlist;i++)
		    if (ping[i].lon_select == MB_NO
			&& iafter == iping)
			iafter = i;
		if (ibefore < iping && iafter > iping)
		    {
		    ping[iping].lon = ping[ibefore].lon 
			+ (ping[iafter].lon - ping[ibefore].lon)
			*(ping[iping].time_d - ping[ibefore].time_d)
			/(ping[iafter].time_d - ping[ibefore].time_d);
		    timelonlat_change = MB_YES;
		    }
		else if (ibefore < iping && ibefore > 0)
		    {
		    ping[iping].lon = ping[ibefore].lon 
			+ (ping[ibefore].lon - ping[ibefore-1].lon)
			*(ping[iping].time_d - ping[ibefore].time_d)
			/(ping[ibefore].time_d - ping[ibefore-1].time_d);
		    timelonlat_change = MB_YES;
		    }
		else if (ibefore < iping)
		    {
		    ping[iping].lon = ping[ibefore].lon;
		    timelonlat_change = MB_YES;
		    }
		else if (iafter > iping && iafter < nlist - 1)
		    {
		    ping[iping].lon = ping[iafter].lon 
			+ (ping[iafter+1].lon - ping[iafter].lon)
			*(ping[iping].time_d - ping[iafter].time_d)
			/(ping[iafter+1].time_d - ping[iafter].time_d);
		    timelonlat_change = MB_YES;
		    }
		else if (iafter > iping)
		    {
		    ping[iping].lon = ping[iafter].lon;
		    timelonlat_change = MB_YES;
		    }
		}
	    }

	/* do latitude */
	for (iping=0;iping<nlist;iping++)
	    {
	    if (ping[iping].lat_select == MB_YES)
		{
		ibefore = iping;
		for (i=iping-1;i>=0;i--)
		    if (ping[i].lat_select == MB_NO
			&& ibefore == iping)
			ibefore = i;
		iafter = iping;
		for (i=iping+1;i<nlist;i++)
		    if (ping[i].lat_select == MB_NO
			&& iafter == iping)
			iafter = i;
		if (ibefore < iping && iafter > iping)
		    {
		    ping[iping].lat = ping[ibefore].lat 
			+ (ping[iafter].lat - ping[ibefore].lat)
			*(ping[iping].time_d - ping[ibefore].time_d)
			/(ping[iafter].time_d - ping[ibefore].time_d);
		    timelonlat_change = MB_YES;
		    }
		else if (ibefore < iping && ibefore > 0)
		    {
		    ping[iping].lat = ping[ibefore].lat 
			+ (ping[ibefore].lat - ping[ibefore-1].lat)
			*(ping[iping].time_d - ping[ibefore].time_d)
			/(ping[ibefore].time_d - ping[ibefore-1].time_d);
		    timelonlat_change = MB_YES;
		    }
		else if (ibefore < iping)
		    {
		    ping[iping].lat = ping[ibefore].lat;
		    timelonlat_change = MB_YES;
		    }
		else if (iafter > iping && iafter < nlist - 1)
		    {
		    ping[iping].lat = ping[iafter].lat 
			+ (ping[iafter+1].lat - ping[iafter].lat)
			*(ping[iping].time_d - ping[iafter].time_d)
			/(ping[iafter+1].time_d - ping[iafter].time_d);
		    timelonlat_change = MB_YES;
		    }
		else if (iafter > iping)
		    {
		    ping[iping].lat = ping[iafter].lat;
		    timelonlat_change = MB_YES;
		    }
		}
	    }

	/* do speed */
	for (iping=0;iping<nlist;iping++)
	    {
	    if (ping[iping].speed_select == MB_YES)
		{
		ibefore = iping;
		for (i=iping-1;i>=0;i--)
		    if (ping[i].speed_select == MB_NO
			&& ibefore == iping)
			ibefore = i;
		iafter = iping;
		for (i=iping+1;i<nlist;i++)
		    if (ping[i].speed_select == MB_NO
			&& iafter == iping)
			iafter = i;
		if (ibefore < iping && iafter > iping)
		    {
		    ping[iping].speed = ping[ibefore].speed 
			+ (ping[iafter].speed - ping[ibefore].speed)
			*(ping[iping].time_d - ping[ibefore].time_d)
			/(ping[iafter].time_d - ping[ibefore].time_d);
		    speedheading_change = MB_YES;
		    }
		else if (ibefore < iping)
		    {
		    ping[iping].speed = ping[ibefore].speed;
		    speedheading_change = MB_YES;
		    }
		else if (iafter > iping)
		    {
		    ping[iping].speed = ping[iafter].speed;
		    speedheading_change = MB_YES;
		    }
		}
	    }

	/* do heading */
	for (iping=0;iping<nlist;iping++)
	    {
	    if (ping[iping].heading_select == MB_YES)
		{
		ibefore = iping;
		for (i=iping-1;i>=0;i--)
		    if (ping[i].heading_select == MB_NO
			&& ibefore == iping)
			ibefore = i;
		iafter = iping;
		for (i=iping+1;i<nlist;i++)
		    if (ping[i].heading_select == MB_NO
			&& iafter == iping)
			iafter = i;
		if (ibefore < iping && iafter > iping)
		    {
		    ping[iping].heading = ping[ibefore].heading 
			+ (ping[iafter].heading - ping[ibefore].heading)
			*(ping[iping].time_d - ping[ibefore].time_d)
			/(ping[iafter].time_d - ping[ibefore].time_d);
		    speedheading_change = MB_YES;
		    }
		else if (ibefore < iping)
		    {
		    ping[iping].heading = ping[ibefore].heading;
		    speedheading_change = MB_YES;
		    }
		else if (iafter > iping)
		    {
		    ping[iping].heading = ping[iafter].heading;
		    speedheading_change = MB_YES;
		    }
		}
	    }

	/* recalculate speed-made-good and course-made-good */
	if (timelonlat_change == MB_YES)
		for (i=0;i<nlist;i++)
			mbnavedit_get_smgcmg(i);

	/* recalculate dead reckoning */
	if (speedheading_change == MB_YES)
		mbnavedit_get_dr();
	}
	/* if no data then set failure flag */
	else
	status = MB_FAILURE;

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
int mbnavedit_action_revert()
{
	/* local variables */
	char	*function_name = "mbnavedit_action_revert";
	int	status = MB_SUCCESS;
	int	iplot;
	int	active_plot;
	int	timelonlat_change;
	int	speedheading_change;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* don't try to do anything if no data */
	if (nplot > 0)
	{

	/* look for position changes */
	timelonlat_change = MB_NO;
	speedheading_change = MB_NO;

	/* loop over each of the plots */
	for (iplot=0;iplot<number_plots;iplot++)
		{
		for (i=current_id;i<current_id+nplot;i++)
			{
			if (mbnavplot[iplot].type == PLOT_TINT)
				{
				if (ping[i].tint_select == MB_YES)
					{
					ping[i].time_d = ping[i].time_d_org;
					ping[i].file_time_d = 
						ping[i].time_d - file_start_time_d;
					ping[i].tint = ping[i].time_d 
							- ping[i-1].time_d;
					timelonlat_change = MB_YES;
					if (i < nlist - 1)
					    ping[i+1].tint = ping[i+1].time_d 
							    - ping[i].time_d;
					status = mb_get_date(verbose, ping[i].time_d, ping[i].time_i);
					}
				}
			else if (mbnavplot[iplot].type == PLOT_LONGITUDE)
				{
				if (ping[i].lon_select == MB_YES)
					{
					ping[i].lon = ping[i].lon_org;
					timelonlat_change = MB_YES;
					}
				}
			else if (mbnavplot[iplot].type == PLOT_LATITUDE)
				{
				if (ping[i].lat_select == MB_YES)
					{
					ping[i].lat = ping[i].lat_org;
					timelonlat_change = MB_YES;
					}
				}
			else if (mbnavplot[iplot].type == PLOT_SPEED)
				{
				if (ping[i].speed_select == MB_YES)
					{
					ping[i].speed = ping[i].speed_org;
					speedheading_change = MB_YES;
					}
				}
			else if (mbnavplot[iplot].type == PLOT_HEADING)
				{
				if (ping[i].heading_select == MB_YES)
					{
					ping[i].heading = ping[i].heading_org;
					speedheading_change = MB_YES;
					}
				}
			}
		}

	/* recalculate speed-made-good and course-made-good */
	if (timelonlat_change == MB_YES)
		for (i=0;i<nlist;i++)
			mbnavedit_get_smgcmg(i);

	/* recalculate dead reckoning */
	if (speedheading_change == MB_YES)
		mbnavedit_get_dr();
		
	/* clear the screen */
	status = mbnavedit_clear_screen();

	/* replot the screen */
	status = mbnavedit_plot_all();

	}
	/* if no data then set failure flag */
	else
	status = MB_FAILURE;

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
int mbnavedit_action_fixtime()
{
	/* local variables */
	char	*function_name = "mbnavedit_action_fixtime";
	int	status = MB_SUCCESS;
	int	iplot;
	int	active_plot;
	int	istart, iend;
	double	start_time_d, end_time_d;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* loop over the data */
	for (i=0;i<nlist;i++)
		{
		if (i == 0)
			{
			istart = i;
			start_time_d = ping[i].time_d;
			}
		else if (ping[i].time_d > start_time_d)
			{
			iend = i;
			end_time_d = ping[i].time_d;
			for (j=istart+1;j<iend;j++)
				{
				ping[j].time_d = start_time_d
				    + (j - istart)
				    * (end_time_d - start_time_d)
				    / (iend - istart);
				mb_get_date(verbose, 
					ping[j].time_d, 
					ping[j].time_i);
				}
			istart = i;
			start_time_d = ping[i].time_d;
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
int mbnavedit_action_showall()
{
	/* local variables */
	char	*function_name = "mbnavedit_action_showall";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* reset plotting time span */
	if (nlist > 0)
		{
		plot_start_time = ping[0].file_time_d;
		plot_end_time = ping[nlist-1].file_time_d;
		data_show_size = 0;
		current_id = 0;
		current = list[current_id];
		}

	/* replot */
	status = mbnavedit_plot_all();

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
int mbnavedit_get_smgcmg(int i)
{
	/* local variables */
	char	*function_name = "mbnavedit_get_smgcmg";
	int	status = MB_SUCCESS;
	double	time_d1, lon1, lat1;
	double	time_d2, lon2, lat2;
	double	mtodeglon, mtodeglat;
	double	del_time, dx, dy, dist;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       i:          %d\n",i);
		}

	/* calculate speed made good and course made for ping i */
	if (i < nlist)
		{
		if (i == 0)
			{
			time_d1 = ping[i].time_d;
			lon1 = ping[i].lon;
			lat1 = ping[i].lat;
			time_d2 = ping[i+1].time_d;
			lon2 = ping[i+1].lon;
			lat2 = ping[i+1].lat;
			}
		else if (i == nlist - 1)
			{
			time_d1 = ping[i-1].time_d;
			lon1 = ping[i-1].lon;
			lat1 = ping[i-1].lat;
			time_d2 = ping[i].time_d;
			lon2 = ping[i].lon;
			lat2 = ping[i].lat;
			}
		else
			{
			time_d1 = ping[i-1].time_d;
			lon1 = ping[i-1].lon;
			lat1 = ping[i-1].lat;
			time_d2 = ping[i].time_d;
			lon2 = ping[i].lon;
			lat2 = ping[i].lat;
			}
		mb_coor_scale(verbose,lat1,&mtodeglon,&mtodeglat);
		del_time = time_d2 - time_d1;
		dx = (lon2 - lon1)/mtodeglon;
		dy = (lat2 - lat1)/mtodeglat;
		dist = sqrt(dx*dx + dy*dy);
		if (del_time > 0.0)
			ping[i].speed_made_good = 3.6*dist/del_time;
		else
			ping[i].speed_made_good = 0.0;
		if (dist > 0.0)
			ping[i].course_made_good = RTD*atan2(dx/dist,dy/dist);
		else
			ping[i].course_made_good = ping[i].heading;
		if (ping[i].course_made_good < 0.0)
			ping[i].course_made_good = 
				ping[i].course_made_good
				+ 360.0;
		
		status = MB_SUCCESS;
		}
	else
		status = MB_FAILURE;

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
int mbnavedit_get_dr()
{
	/* local variables */
	char	*function_name = "mbnavedit_get_dr";
	int	status = MB_SUCCESS;
	double	mtodeglon, mtodeglat;
	double	del_time, dx, dy;
	double	driftlon, driftlat;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* calculate dead reckoning */
	driftlon = 0.00001 * drift_lon;
	driftlat = 0.00001 * drift_lat;
	for (i=0;i<nlist;i++)
		{
		if (i == 0)
			{
			ping[i].lon_dr = ping[i].lon;
			ping[i].lat_dr = ping[i].lat;
			}
		else
			{
			del_time = ping[i].time_d - ping[i-1].time_d;
			if (del_time < 300.0)
			    {
			    mb_coor_scale(verbose,ping[i].lat,&mtodeglon,&mtodeglat);
			    dx = sin(DTR * ping[i].heading) * ping[i].speed * del_time / 3.6;
			    dy = cos(DTR * ping[i].heading) * ping[i].speed * del_time / 3.6;
			    ping[i].lon_dr = ping[i-1].lon_dr + dx * mtodeglon + del_time * driftlon / 3600.0;
			    ping[i].lat_dr = ping[i-1].lat_dr + dy * mtodeglat + del_time * driftlat / 3600.0;
			    }
			else
			    {
			    ping[i].lon_dr = ping[i].lon;
			    ping[i].lat_dr = ping[i].lat;
			    }
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
int mbnavedit_plot_all()
{
	/* local variables */
	char	*function_name = "mbnavedit_plot_all";
	int	status = MB_SUCCESS;
	double	time_min;
	double	time_max;
	double	tint_min;
	double	tint_max;
	double	lon_min;
	double	lon_max;
	double	lat_min;
	double	lat_max;
	double	speed_min;
	double	speed_max;
	double	heading_min;
	double	heading_max;
	double	roll_min;
	double	roll_max;
	double	pitch_min;
	double	pitch_max;
	double	heave_min;
	double	heave_max;
	double	center, range;
	int	margin_x, margin_y;
	int	iyzero;
	int	iplot;
	int	center_x, center_y;
	double	dx, x;
	int	xtime_i[7];
	int	ix;
	int	swidth, sascent, sdescent;
	char	yformat[10];
	int	i, j, k, ii;
	char	string[128];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* figure out which pings to plot */
	nplot = 0;
	if (data_show_size > 0 && nlist > 0)
		{
		plot_start_time = ping[current_id].file_time_d;
		plot_end_time = plot_start_time + data_show_size;
		for (i=current_id;i<nlist;i++)
			if (ping[i].file_time_d <= plot_end_time)
				nplot++;
		}
	else if (nlist > 0)
		{
		plot_start_time = ping[0].file_time_d;
		plot_end_time = ping[nlist-1].file_time_d;
		data_show_size = plot_end_time - plot_start_time + 1;
		if (data_show_max < data_show_size)
			data_show_max = data_show_size;
		nplot = nlist;
		}

	/* deselect data outside plots */
	for (i=0;i<current_id;i++)
		{
		ping[i].tint_select = MB_NO;
		ping[i].lon_select = MB_NO;
		ping[i].lat_select = MB_NO;
		ping[i].speed_select = MB_NO;
		ping[i].heading_select = MB_NO;
		}
	for (i=current_id+nplot;i<nlist;i++)
		{
		ping[i].tint_select = MB_NO;
		ping[i].lon_select = MB_NO;
		ping[i].lat_select = MB_NO;
		ping[i].speed_select = MB_NO;
		ping[i].heading_select = MB_NO;
		}

	/* don't try to plot if no data */
	if (nplot > 0)
	{

	/* find min max values */
	time_min = plot_start_time;
	time_max = plot_end_time;
	tint_min = ping[current_id].tint;
	tint_max = ping[current_id].tint;
	lon_min = ping[current_id].lon;
	lon_max = ping[current_id].lon;
	lat_min = ping[current_id].lat;
	lat_max = ping[current_id].lat;
	speed_min = 0.0;
	speed_max = ping[current_id].speed;
	heading_min = ping[current_id].heading;
	heading_max = ping[current_id].heading;
	roll_min = ping[current_id].roll;
	roll_max = ping[current_id].roll;
	pitch_min = ping[current_id].pitch;
	pitch_max = ping[current_id].pitch;
	heave_min = ping[current_id].heave;
	heave_max = ping[current_id].heave;
	for (i=current_id+1;i<current_id+nplot;i++)
		{
		tint_min = MIN(ping[i].tint, tint_min);
		tint_max = MAX(ping[i].tint, tint_max);
		if (plot_tint_org == MB_YES)
			{
			tint_min = MIN(ping[i].tint_org, tint_min);
			tint_max = MAX(ping[i].tint_org, tint_max);
			}
		lon_min = MIN(ping[i].lon, lon_min);
		lon_max = MAX(ping[i].lon, lon_max);
		if (plot_lon_org == MB_YES)
			{
			lon_min = MIN(ping[i].lon_org, lon_min);
			lon_max = MAX(ping[i].lon_org, lon_max);
			}
		if (plot_lon_dr == MB_YES)
			{
			lon_min = MIN(ping[i].lon_dr, lon_min);
			lon_max = MAX(ping[i].lon_dr, lon_max);
			}
		lat_min = MIN(ping[i].lat, lat_min);
		lat_max = MAX(ping[i].lat, lat_max);
		if (plot_lat_org == MB_YES)
			{
			lat_min = MIN(ping[i].lat_org, lat_min);
			lat_max = MAX(ping[i].lat_org, lat_max);
			}
		if (plot_lat_dr == MB_YES)
			{
			lat_min = MIN(ping[i].lat_dr, lat_min);
			lat_max = MAX(ping[i].lat_dr, lat_max);
			}
		speed_min = MIN(ping[i].speed, speed_min);
		speed_max = MAX(ping[i].speed, speed_max);
		if (plot_speed_org == MB_YES)
			{
			speed_min = MIN(ping[i].speed_org, speed_min);
			speed_max = MAX(ping[i].speed_org, speed_max);
			}
		if (plot_smg == MB_YES)
			{
			speed_min = MIN(ping[i].speed_made_good, speed_min);
			speed_max = MAX(ping[i].speed_made_good, speed_max);
			}
		heading_min = MIN(ping[i].heading, heading_min);
		heading_max = MAX(ping[i].heading, heading_max);
		if (plot_heading_org == MB_YES)
			{
			heading_min = MIN(ping[i].heading_org, 
				heading_min);
			heading_max = MAX(ping[i].heading_org, 
				heading_max);
			}
		if (plot_cmg == MB_YES)
			{
			heading_min = MIN(ping[i].course_made_good, 
				heading_min);
			heading_max = MAX(ping[i].course_made_good, 
				heading_max);
			}
		roll_min = MIN(ping[i].roll, roll_min);
		roll_max = MAX(ping[i].roll, roll_max);
		pitch_min = MIN(ping[i].pitch, pitch_min);
		pitch_max = MAX(ping[i].pitch, pitch_max);
		heave_min = MIN(ping[i].heave, heave_min);
		heave_max = MAX(ping[i].heave, heave_max);
		}

	/* scale the min max a bit larger so all points fit on plots */
	center = 0.5*(time_min + time_max);
	range = 0.51*(time_max - time_min);
	time_min = center - range;
	time_max = center + range;
	center = 0.5*(tint_min + tint_max);
	range = 0.55*(tint_max - tint_min);
	tint_min = center - range;
	tint_max = center + range;
	center = 0.5*(lon_min + lon_max);
	range = 0.55*(lon_max - lon_min);
	lon_min = center - range;
	lon_max = center + range;
	center = 0.5*(lat_min + lat_max);
	range = 0.55*(lat_max - lat_min);
	lat_min = center - range;
	lat_max = center + range;
	if (speed_min < 0.0)
		{
		center = 0.5*(speed_min + speed_max);
		range = 0.55*(speed_max - speed_min);
		speed_min = center - range;
		speed_max = center + range;
		}
	else
		speed_max = 1.05*speed_max;
	center = 0.5*(heading_min + heading_max);
	range = 0.55*(heading_max - heading_min);
	heading_min = center - range;
	heading_max = center + range;
	roll_max = 1.1*MAX(fabs(roll_min), fabs(roll_max));
	roll_min = -roll_max;
	pitch_max = 1.1*MAX(fabs(pitch_min), fabs(pitch_max));
	pitch_min = -pitch_max;
	heave_max = 1.1*MAX(fabs(heave_min), fabs(heave_max));
	heave_min = -heave_max;

	/* make sure lon and lat scaled the same if both plotted */
	if (plot_lon == MB_YES && plot_lat == MB_YES)
		{
		if ((lon_max - lon_min) > (lat_max - lat_min))
			{
			center = 0.5*(lat_min + lat_max);
			lat_min = center - 0.5*(lon_max - lon_min);
			lat_max = center + 0.5*(lon_max - lon_min);
			}
		else
			{
			center = 0.5*(lon_min + lon_max);
			lon_min = center - 0.5*(lat_max - lat_min);
			lon_max = center + 0.5*(lat_max - lat_min);
			}
		}

	/* make sure min max values aren't too small */
	if ((tint_max - tint_min) < 0.01)
		{
		center = 0.5*(tint_min + tint_max);
		tint_min = center - 0.005;
		tint_max = center + 0.005;
		}
	if ((lon_max - lon_min) < 0.01)
		{
		center = 0.5*(lon_min + lon_max);
		lon_min = center - 0.005;
		lon_max = center + 0.005;
		}
	if ((lat_max - lat_min) < 0.01)
		{
		center = 0.5*(lat_min + lat_max);
		lat_min = center - 0.005;
		lat_max = center + 0.005;
		}
	if (speed_max < 10.0)
		speed_max = 10.0;
	if ((heading_max - heading_min) < 10.0)
		{
		center = 0.5*(heading_min + heading_max);
		heading_min = center - 5;
		heading_max = center + 5;
		}
	if ((roll_max - roll_min) < 2.0)
		{
		center = 0.5*(roll_min + roll_max);
		roll_min = center - 1;
		roll_max = center + 1;
		}
	if ((pitch_max - pitch_min) < 2.0)
		{
		center = 0.5*(pitch_min + pitch_max);
		pitch_min = center - 1;
		pitch_max = center + 1;
		}
	if ((heave_max - heave_min) < 0.02)
		{
		center = 0.5*(heave_min + heave_max);
		heave_min = center - 0.01;
		heave_max = center + 0.01;
		}

	/* print out information */
	if (verbose >= 2)
		{
		fprintf(stderr,"\n%d data records set for plotting (%d desired)\n",
			nplot,data_show_size);
		for (i=current_id;i<current_id+nplot;i++)
			fprintf(stderr,"dbg5       %4d %4d %4d  %d/%d/%d %2.2d:%2.2d:%2.2d.%6.6d  %11.6f  %11.6f  %11.6f  %11.6f %11.6f %5.2f %5.1f %5.1f %5.1f %5.1f\n",
				i,ping[i].id,ping[i].record,
				ping[i].time_i[1],ping[i].time_i[2],
				ping[i].time_i[0],ping[i].time_i[3],
				ping[i].time_i[4],ping[i].time_i[5],
				ping[i].time_i[6],ping[i].time_d,
				ping[i].file_time_d,ping[i].tint,
				ping[i].lon, ping[i].lat, 
				ping[i].speed, ping[i].heading, 
				ping[i].roll, ping[i].pitch, 
				ping[i].heave);
		}

	/* get plot margins */
	margin_x = plot_width/10;
	margin_y = plot_height/6;
	
	/* get date at start of file */
	mb_get_date(verbose, file_start_time_d + plot_start_time, xtime_i);

	/* figure out how many plots to make */
	number_plots = 0;
	if (plot_tint == MB_YES)
		{
		mbnavplot[number_plots].type = PLOT_TINT;
		mbnavplot[number_plots].ixmin = 1.75*margin_x;
		mbnavplot[number_plots].ixmax = plot_width - margin_x/2;
		mbnavplot[number_plots].iymin = plot_height - margin_y
			+ number_plots*plot_height;
		mbnavplot[number_plots].iymax = number_plots*plot_height 
			+ margin_y;
		mbnavplot[number_plots].xmin = time_min;
		mbnavplot[number_plots].xmax = time_max;
		mbnavplot[number_plots].ymin = tint_min;
		mbnavplot[number_plots].ymax = tint_max;
		mbnavplot[number_plots].xscale = 
			(mbnavplot[number_plots].ixmax 
			- mbnavplot[number_plots].ixmin)
			/(mbnavplot[number_plots].xmax 
			- mbnavplot[number_plots].xmin);
		mbnavplot[number_plots].yscale = 
			(mbnavplot[number_plots].iymax 
			- mbnavplot[number_plots].iymin)
			/(mbnavplot[number_plots].ymax 
			- mbnavplot[number_plots].ymin);
		mbnavplot[number_plots].xinterval = 100.0;
		mbnavplot[number_plots].yinterval = 5.0;
		sprintf(mbnavplot[number_plots].xlabel, 
			"Time (HH:MM:SS.SSS) beginning on %2.2d/%2.2d/%4.4d", 
			xtime_i[1], xtime_i[2], xtime_i[0]);
		sprintf(mbnavplot[number_plots].ylabel1, 
			"dT");
		sprintf(mbnavplot[number_plots].ylabel2, 
			"(seconds)");
		number_plots++;
		}
	if (plot_lon == MB_YES)
		{
		mbnavplot[number_plots].type = PLOT_LONGITUDE;
		mbnavplot[number_plots].ixmin = 1.75*margin_x;
		mbnavplot[number_plots].ixmax = plot_width - margin_x/2;
		mbnavplot[number_plots].iymin = plot_height - margin_y
			+ number_plots*plot_height;
		mbnavplot[number_plots].iymax = number_plots*plot_height 
			+ margin_y;
		mbnavplot[number_plots].xmin = time_min;
		mbnavplot[number_plots].xmax = time_max;
		mbnavplot[number_plots].ymin = lon_min;
		mbnavplot[number_plots].ymax = lon_max;
		mbnavplot[number_plots].xscale = 
			(mbnavplot[number_plots].ixmax 
			- mbnavplot[number_plots].ixmin)
			/(mbnavplot[number_plots].xmax 
			- mbnavplot[number_plots].xmin);
		mbnavplot[number_plots].yscale = 
			(mbnavplot[number_plots].iymax 
			- mbnavplot[number_plots].iymin)
			/(mbnavplot[number_plots].ymax 
			- mbnavplot[number_plots].ymin);
		mbnavplot[number_plots].xinterval = 100.0;
		mbnavplot[number_plots].yinterval = 45.0;
		sprintf(mbnavplot[number_plots].xlabel, 
			"Time (HH:MM:SS.SSS) beginning on %2.2d/%2.2d/%4.4d", 
			xtime_i[1], xtime_i[2], xtime_i[0]);
		sprintf(mbnavplot[number_plots].ylabel1, 
			"Longitude");
		sprintf(mbnavplot[number_plots].ylabel2, 
			"(degrees)");
		number_plots++;
		}
	if (plot_lat == MB_YES)
		{
		mbnavplot[number_plots].type = PLOT_LATITUDE;
		mbnavplot[number_plots].ixmin = 1.75*margin_x;
		mbnavplot[number_plots].ixmax = plot_width - margin_x/2;
		mbnavplot[number_plots].iymin = plot_height - margin_y
			+ number_plots*plot_height;
		mbnavplot[number_plots].iymax = number_plots*plot_height 
			+ margin_y;
		mbnavplot[number_plots].xmin = time_min;
		mbnavplot[number_plots].xmax = time_max;
		mbnavplot[number_plots].ymin = lat_min;
		mbnavplot[number_plots].ymax = lat_max;
		mbnavplot[number_plots].xscale = 
			(mbnavplot[number_plots].ixmax 
			- mbnavplot[number_plots].ixmin)
			/(mbnavplot[number_plots].xmax 
			- mbnavplot[number_plots].xmin);
		mbnavplot[number_plots].yscale = 
			(mbnavplot[number_plots].iymax 
			- mbnavplot[number_plots].iymin)
			/(mbnavplot[number_plots].ymax 
			- mbnavplot[number_plots].ymin);
		mbnavplot[number_plots].xinterval = 100.0;
		mbnavplot[number_plots].yinterval = 45.0;
		sprintf(mbnavplot[number_plots].xlabel, 
			"Time (HH:MM:SS.SSS) beginning on %2.2d/%2.2d/%4.4d", 
			xtime_i[1], xtime_i[2], xtime_i[0]);
		sprintf(mbnavplot[number_plots].ylabel1, 
			"Latitude");
		sprintf(mbnavplot[number_plots].ylabel2, 
			"(degrees)");
		number_plots++;
		}
	if (plot_speed == MB_YES)
		{
		mbnavplot[number_plots].type = PLOT_SPEED;
		mbnavplot[number_plots].ixmin = 1.75*margin_x;
		mbnavplot[number_plots].ixmax = plot_width - margin_x/2;
		mbnavplot[number_plots].iymin = plot_height - margin_y
			+ number_plots*plot_height;
		mbnavplot[number_plots].iymax = number_plots*plot_height 
			+ margin_y;
		mbnavplot[number_plots].xmin = time_min;
		mbnavplot[number_plots].xmax = time_max;
		mbnavplot[number_plots].ymin = speed_min;
		mbnavplot[number_plots].ymax = speed_max;
		mbnavplot[number_plots].xscale = 
			(mbnavplot[number_plots].ixmax 
			- mbnavplot[number_plots].ixmin)
			/(mbnavplot[number_plots].xmax 
			- mbnavplot[number_plots].xmin);
		mbnavplot[number_plots].yscale = 
			(mbnavplot[number_plots].iymax 
			- mbnavplot[number_plots].iymin)
			/(mbnavplot[number_plots].ymax 
			- mbnavplot[number_plots].ymin);
		mbnavplot[number_plots].xinterval = 100.0;
		mbnavplot[number_plots].yinterval = 10;
		sprintf(mbnavplot[number_plots].xlabel, 
			"Time (HH:MM:SS.SSS) beginning on %2.2d/%2.2d/%4.4d", 
			xtime_i[1], xtime_i[2], xtime_i[0]);
		sprintf(mbnavplot[number_plots].ylabel1, 
			"Speed");
		sprintf(mbnavplot[number_plots].ylabel2, 
			"(km/hr)");
		number_plots++;
		}
	if (plot_heading == MB_YES)
		{
		mbnavplot[number_plots].type = PLOT_HEADING;
		mbnavplot[number_plots].ixmin = 1.75*margin_x;
		mbnavplot[number_plots].ixmax = plot_width - margin_x/2;
		mbnavplot[number_plots].iymin = plot_height - margin_y
			+ number_plots*plot_height;
		mbnavplot[number_plots].iymax = number_plots*plot_height 
			+ margin_y;
		mbnavplot[number_plots].xmin = time_min;
		mbnavplot[number_plots].xmax = time_max;
		mbnavplot[number_plots].ymin = heading_min;
		mbnavplot[number_plots].ymax = heading_max;
		mbnavplot[number_plots].xscale = 
			(mbnavplot[number_plots].ixmax 
			- mbnavplot[number_plots].ixmin)
			/(mbnavplot[number_plots].xmax 
			- mbnavplot[number_plots].xmin);
		mbnavplot[number_plots].yscale = 
			(mbnavplot[number_plots].iymax 
			- mbnavplot[number_plots].iymin)
			/(mbnavplot[number_plots].ymax 
			- mbnavplot[number_plots].ymin);
		mbnavplot[number_plots].xinterval = 100.0;
		mbnavplot[number_plots].yinterval = 45.0;
		sprintf(mbnavplot[number_plots].xlabel, 
			"Time (HH:MM:SS.SSS) beginning on %2.2d/%2.2d/%4.4d", 
			xtime_i[1], xtime_i[2], xtime_i[0]);
		sprintf(mbnavplot[number_plots].ylabel1, 
			"Heading");
		sprintf(mbnavplot[number_plots].ylabel2, 
			"(degrees)");
		number_plots++;
		}
	if (plot_roll == MB_YES)
		{
		mbnavplot[number_plots].type = PLOT_ROLL;
		mbnavplot[number_plots].ixmin = 1.75*margin_x;
		mbnavplot[number_plots].ixmax = plot_width - margin_x/2;
		mbnavplot[number_plots].iymin = plot_height - margin_y
			+ number_plots*plot_height;
		mbnavplot[number_plots].iymax = number_plots*plot_height 
			+ margin_y;
		mbnavplot[number_plots].xmin = time_min;
		mbnavplot[number_plots].xmax = time_max;
		mbnavplot[number_plots].ymin = roll_min;
		mbnavplot[number_plots].ymax = roll_max;
		mbnavplot[number_plots].xscale = 
			(mbnavplot[number_plots].ixmax 
			- mbnavplot[number_plots].ixmin)
			/(mbnavplot[number_plots].xmax 
			- mbnavplot[number_plots].xmin);
		mbnavplot[number_plots].yscale = 
			(mbnavplot[number_plots].iymax 
			- mbnavplot[number_plots].iymin)
			/(mbnavplot[number_plots].ymax 
			- mbnavplot[number_plots].ymin);
		mbnavplot[number_plots].xinterval = 100.0;
		mbnavplot[number_plots].yinterval = 45.0;
		sprintf(mbnavplot[number_plots].xlabel, 
			"Time (HH:MM:SS.SSS) beginning on %2.2d/%2.2d/%4.4d", 
			xtime_i[1], xtime_i[2], xtime_i[0]);
		sprintf(mbnavplot[number_plots].ylabel1, 
			"Roll");
		sprintf(mbnavplot[number_plots].ylabel2, 
			"(degrees)");
		number_plots++;
		}
	if (plot_pitch == MB_YES)
		{
		mbnavplot[number_plots].type = PLOT_PITCH;
		mbnavplot[number_plots].ixmin = 1.75*margin_x;
		mbnavplot[number_plots].ixmax = plot_width - margin_x/2;
		mbnavplot[number_plots].iymin = plot_height - margin_y
			+ number_plots*plot_height;
		mbnavplot[number_plots].iymax = number_plots*plot_height 
			+ margin_y;
		mbnavplot[number_plots].xmin = time_min;
		mbnavplot[number_plots].xmax = time_max;
		mbnavplot[number_plots].ymin = pitch_min;
		mbnavplot[number_plots].ymax = pitch_max;
		mbnavplot[number_plots].xscale = 
			(mbnavplot[number_plots].ixmax 
			- mbnavplot[number_plots].ixmin)
			/(mbnavplot[number_plots].xmax 
			- mbnavplot[number_plots].xmin);
		mbnavplot[number_plots].yscale = 
			(mbnavplot[number_plots].iymax 
			- mbnavplot[number_plots].iymin)
			/(mbnavplot[number_plots].ymax 
			- mbnavplot[number_plots].ymin);
		mbnavplot[number_plots].xinterval = 100.0;
		mbnavplot[number_plots].yinterval = 45.0;
		sprintf(mbnavplot[number_plots].xlabel, 
			"Time (HH:MM:SS.SSS) beginning on %2.2d/%2.2d/%4.4d", 
			xtime_i[1], xtime_i[2], xtime_i[0]);
		sprintf(mbnavplot[number_plots].ylabel1, 
			"Pitch");
		sprintf(mbnavplot[number_plots].ylabel2, 
			"(degrees)");
		number_plots++;
		}
	if (plot_heave == MB_YES)
		{
		mbnavplot[number_plots].type = PLOT_HEAVE;
		mbnavplot[number_plots].ixmin = 1.75*margin_x;
		mbnavplot[number_plots].ixmax = plot_width - margin_x/2;
		mbnavplot[number_plots].iymin = plot_height - margin_y
			+ number_plots*plot_height;
		mbnavplot[number_plots].iymax = number_plots*plot_height 
			+ margin_y;
		mbnavplot[number_plots].xmin = time_min;
		mbnavplot[number_plots].xmax = time_max;
		mbnavplot[number_plots].ymin = heave_min;
		mbnavplot[number_plots].ymax = heave_max;
		mbnavplot[number_plots].xscale = 
			(mbnavplot[number_plots].ixmax 
			- mbnavplot[number_plots].ixmin)
			/(mbnavplot[number_plots].xmax 
			- mbnavplot[number_plots].xmin);
		mbnavplot[number_plots].yscale = 
			(mbnavplot[number_plots].iymax 
			- mbnavplot[number_plots].iymin)
			/(mbnavplot[number_plots].ymax 
			- mbnavplot[number_plots].ymin);
		mbnavplot[number_plots].xinterval = 100.0;
		mbnavplot[number_plots].yinterval = 45.0;
		sprintf(mbnavplot[number_plots].xlabel, 
			"Time (HH:MM:SS.SSS) beginning on %2.2d/%2.2d/%4.4d", 
			xtime_i[1], xtime_i[2], xtime_i[0]);
		sprintf(mbnavplot[number_plots].ylabel1, 
			"Heave");
		sprintf(mbnavplot[number_plots].ylabel2, 
			"(meters)");
		number_plots++;
		}

	/* clear screen */
	status = mbnavedit_clear_screen();

	/* plot filename */
	sprintf(string,"Current Data File:");
	xg_justify(mbnavedit_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(mbnavedit_xgid,50,
		margin_y/2-sascent,string,pixel_values[BLACK],XG_SOLIDLINE);
	xg_drawstring(mbnavedit_xgid,50,
		margin_y/2+sascent,ifile,pixel_values[BLACK],XG_SOLIDLINE);

	/* do plots */
	for (iplot=0;iplot<number_plots;iplot++)
		{
		/* get center locations */
		center_x = (mbnavplot[iplot].ixmin 
			+ mbnavplot[iplot].ixmax)/2;
		center_y = (mbnavplot[iplot].iymin 
			+ mbnavplot[iplot].iymax)/2;

		/* plot x label */
		xg_justify(mbnavedit_xgid,
			mbnavplot[iplot].xlabel,
			&swidth,&sascent,&sdescent);
		xg_drawstring(mbnavedit_xgid,
			(int)(center_x - swidth/2), 
			(int)(mbnavplot[iplot].iymin+0.95*margin_y),
			mbnavplot[iplot].xlabel,
			pixel_values[BLACK],XG_SOLIDLINE);

		/* plot y labels */
		xg_justify(mbnavedit_xgid,
			mbnavplot[iplot].ylabel1,
			&swidth,&sascent,&sdescent);
		xg_drawstring(mbnavedit_xgid,
			(int)(mbnavplot[iplot].ixmin - swidth/2 - 1.25*margin_x), 
			(int)(center_y-sascent),
			mbnavplot[iplot].ylabel1,
			pixel_values[BLACK],XG_SOLIDLINE);
		xg_justify(mbnavedit_xgid,
			mbnavplot[iplot].ylabel2,
			&swidth,&sascent,&sdescent);
		xg_drawstring(mbnavedit_xgid,
			(int)(mbnavplot[iplot].ixmin - swidth/2 - 1.25*margin_x), 
			(int)(center_y+2*sascent),
			mbnavplot[iplot].ylabel2,
			pixel_values[BLACK],XG_SOLIDLINE);

		/* plot x axis time annotation */
		dx = (plot_end_time - plot_start_time) / 5;
		for (i=0;i<6;i++)
			{
			/* get x position */
			x = plot_start_time + i * dx;
			ix = mbnavplot[iplot].ixmin 
				+ mbnavplot[iplot].xscale 
				    * (x - mbnavplot[iplot].xmin);
			x += file_start_time_d;
				    
			/* draw tickmarks */
			xg_drawline(mbnavedit_xgid,
				ix, 
				mbnavplot[iplot].iymin, 
				ix, 
				mbnavplot[iplot].iymin + 5, 
				pixel_values[BLACK],XG_SOLIDLINE);
				
			/* draw annotations */
			mb_get_date(verbose, x, xtime_i);
			sprintf(string, "%2.2d:%2.2d:%2.2d.%3.3d", 
				xtime_i[3], 
				xtime_i[4], 
				xtime_i[5], 
				(int)(0.001 * xtime_i[6]));
			xg_justify(mbnavedit_xgid,
				string,
				&swidth,&sascent,&sdescent);
			xg_drawstring(mbnavedit_xgid,
				(int)(ix - swidth/2), 
				(int)(mbnavplot[iplot].iymin 
					+ 5 + 1.75*sascent),
				string,
				pixel_values[BLACK],XG_SOLIDLINE);
			}
			
		/* plot y min max values */
		if (mbnavplot[iplot].type == PLOT_LONGITUDE ||
			mbnavplot[iplot].type == PLOT_LATITUDE)
			strcpy(yformat, "%11.6f");
		else
			strcpy(yformat, "%6.2f");
		sprintf(string, yformat, mbnavplot[iplot].ymin);
		xg_justify(mbnavedit_xgid,
			string,
			&swidth,&sascent,&sdescent);
		xg_drawstring(mbnavedit_xgid,
			(int)(mbnavplot[iplot].ixmin - swidth - 0.03*margin_x), 
			(int)(mbnavplot[iplot].iymin + 0.5*sascent),
			string,
			pixel_values[BLACK],XG_SOLIDLINE);
		sprintf(string, yformat, mbnavplot[iplot].ymax);
		xg_justify(mbnavedit_xgid,
			string,
			&swidth,&sascent,&sdescent);
		xg_drawstring(mbnavedit_xgid,
			(int)(mbnavplot[iplot].ixmin - swidth - 0.03*margin_x), 
			(int)(mbnavplot[iplot].iymax + 0.5*sascent),
			string,
			pixel_values[BLACK],XG_SOLIDLINE);

		/* plot zero values */
		if (mbnavplot[iplot].ymax > 0.0 && mbnavplot[iplot].ymin < 0.0)
			{
			if (mbnavplot[iplot].type == PLOT_LONGITUDE ||
				mbnavplot[iplot].type == PLOT_LATITUDE)
				strcpy(yformat, "%11.6f");
			else
				strcpy(yformat, "%6.2f");
			sprintf(string, yformat, 0.0);
			xg_justify(mbnavedit_xgid,
				string,
				&swidth,&sascent,&sdescent);
			iyzero = mbnavplot[iplot].iymin - mbnavplot[iplot].yscale*mbnavplot[iplot].ymin;
			xg_drawstring(mbnavedit_xgid,
				(int)(mbnavplot[iplot].ixmin - swidth - 0.03*margin_x), 
				(int)(iyzero + 0.5*sascent),
				string,
				pixel_values[BLACK],XG_SOLIDLINE);
			xg_drawline(mbnavedit_xgid,
				mbnavplot[iplot].ixmin, iyzero, 
				mbnavplot[iplot].ixmax, iyzero, 
				pixel_values[BLACK],XG_DASHLINE);
			}

		/* plot bounding box */
		xg_drawline(mbnavedit_xgid,
			mbnavplot[iplot].ixmin, 
			mbnavplot[iplot].iymin, 
			mbnavplot[iplot].ixmax, 
			mbnavplot[iplot].iymin, 
			pixel_values[BLACK],XG_SOLIDLINE);
		xg_drawline(mbnavedit_xgid,
			mbnavplot[iplot].ixmax, 
			mbnavplot[iplot].iymin, 
			mbnavplot[iplot].ixmax, 
			mbnavplot[iplot].iymax, 
			pixel_values[BLACK],XG_SOLIDLINE);
		xg_drawline(mbnavedit_xgid,
			mbnavplot[iplot].ixmax, 
			mbnavplot[iplot].iymax, 
			mbnavplot[iplot].ixmin, 
			mbnavplot[iplot].iymax, 
			pixel_values[BLACK],XG_SOLIDLINE);
		xg_drawline(mbnavedit_xgid,
			mbnavplot[iplot].ixmin, 
			mbnavplot[iplot].iymax, 
			mbnavplot[iplot].ixmin, 
			mbnavplot[iplot].iymin, 
			pixel_values[BLACK],XG_SOLIDLINE);

		/* now plot the data */
		if (mbnavplot[iplot].type == PLOT_TINT)
			mbnavedit_plot_tint(iplot);
		else if (mbnavplot[iplot].type == PLOT_LONGITUDE)
			mbnavedit_plot_lon(iplot);
		else if (mbnavplot[iplot].type == PLOT_LATITUDE)
			mbnavedit_plot_lat(iplot);
		else if (mbnavplot[iplot].type == PLOT_SPEED)
			mbnavedit_plot_speed(iplot);
		else if (mbnavplot[iplot].type == PLOT_HEADING)
			mbnavedit_plot_heading(iplot);
		else if (mbnavplot[iplot].type == PLOT_ROLL)
			mbnavedit_plot_roll(iplot);
		else if (mbnavplot[iplot].type == PLOT_PITCH)
			mbnavedit_plot_pitch(iplot);
		else if (mbnavplot[iplot].type == PLOT_HEAVE)
			mbnavedit_plot_heave(iplot);
		}

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
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbnavedit_plot_tint(int iplot)
{
	/* local variables */
	char	*function_name = "mbnavedit_plot_tint";
	int	status = MB_SUCCESS;
	int	ixmin, ixmax, iymin, iymax;
	double	xmin, xmax, ymin, ymax;
	double	xscale, yscale;
	int	tint_x1, tint_y1, tint_x2, tint_y2;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iplot:       %d\n",iplot);
		}

	/* get scaling values */
	ixmin = mbnavplot[iplot].ixmin;
	ixmax = mbnavplot[iplot].ixmax;
	iymin = mbnavplot[iplot].iymin;
	iymax = mbnavplot[iplot].iymax;
	xmin = mbnavplot[iplot].xmin;
	xmax = mbnavplot[iplot].xmax;
	ymin = mbnavplot[iplot].ymin;
	ymax = mbnavplot[iplot].ymax;
	xscale = mbnavplot[iplot].xscale;
	yscale = mbnavplot[iplot].yscale;

	/* plot original expected time data */
	if (plot_tint_org == MB_YES)
	{
	tint_x1 = ixmin + xscale*(ping[current_id].file_time_d - xmin);
	tint_y1 = iymin + yscale*(ping[current_id].tint_org - ymin);
	for (i=current_id+1;i<current_id+nplot;i++)
		{
		tint_x2 = ixmin + xscale*(ping[i].file_time_d - xmin);
		tint_y2 = iymin + yscale*(ping[i].tint_org - ymin);
		xg_drawline(mbnavedit_xgid,
			tint_x1, tint_y1, tint_x2, tint_y2, 
			pixel_values[GREEN],XG_SOLIDLINE);
		tint_x1 = tint_x2;
		tint_y1 = tint_y2;
		}
	}

	/* plot basic expected time data */
	for (i=current_id;i<current_id+nplot;i++)
		{
		ping[i].tint_x = ixmin + xscale*(ping[i].file_time_d - xmin);
		ping[i].tint_y = iymin + yscale*(ping[i].tint - ymin);
		if (ping[i].tint_select == MB_YES)
			xg_drawrectangle(mbnavedit_xgid, 
				ping[i].tint_x-2, 
				ping[i].tint_y-2, 4, 4, 
				pixel_values[RED],XG_SOLIDLINE);
		else
			xg_fillrectangle(mbnavedit_xgid, 
				ping[i].tint_x-2, 
				ping[i].tint_y-2, 4, 4, 
				pixel_values[BLACK],XG_SOLIDLINE);
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
int mbnavedit_plot_lon(int iplot)
{
	/* local variables */
	char	*function_name = "mbnavedit_plot_lon";
	int	status = MB_SUCCESS;
	int	ixmin, ixmax, iymin, iymax;
	double	xmin, xmax, ymin, ymax;
	double	xscale, yscale;
	int	lon_x1, lon_y1, lon_x2, lon_y2;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iplot:       %d\n",iplot);
		}

	/* get scaling values */
	ixmin = mbnavplot[iplot].ixmin;
	ixmax = mbnavplot[iplot].ixmax;
	iymin = mbnavplot[iplot].iymin;
	iymax = mbnavplot[iplot].iymax;
	xmin = mbnavplot[iplot].xmin;
	xmax = mbnavplot[iplot].xmax;
	ymin = mbnavplot[iplot].ymin;
	ymax = mbnavplot[iplot].ymax;
	xscale = mbnavplot[iplot].xscale;
	yscale = mbnavplot[iplot].yscale;

	/* plot original longitude data */
	if (plot_lon_org == MB_YES)
	{
	lon_x1 = ixmin + xscale*(ping[current_id].file_time_d - xmin);
	lon_y1 = iymin + yscale*(ping[current_id].lon_org - ymin);
	for (i=current_id+1;i<current_id+nplot;i++)
		{
		lon_x2 = ixmin + xscale*(ping[i].file_time_d - xmin);
		lon_y2 = iymin + yscale*(ping[i].lon_org - ymin);
		xg_drawline(mbnavedit_xgid,
			lon_x1, lon_y1, lon_x2, lon_y2, 
			pixel_values[GREEN],XG_SOLIDLINE);
		lon_x1 = lon_x2;
		lon_y1 = lon_y2;
		}
	}

	/* plot dr longitude data */
	if (plot_lon_dr == MB_YES)
	{
	lon_x1 = ixmin + xscale*(ping[current_id].file_time_d - xmin);
	lon_y1 = iymin + yscale*(ping[current_id].lon_dr - ymin);
	for (i=current_id+1;i<current_id+nplot;i++)
		{
		lon_x2 = ixmin + xscale*(ping[i].file_time_d - xmin);
		lon_y2 = iymin + yscale*(ping[i].lon_dr - ymin);
		xg_drawline(mbnavedit_xgid,
			lon_x1, lon_y1, lon_x2, lon_y2,
			pixel_values[BLUE],XG_SOLIDLINE);
		lon_x1 = lon_x2;
		lon_y1 = lon_y2;
		}
	}

	/* plot basic longitude data */
	for (i=current_id;i<current_id+nplot;i++)
		{
		ping[i].lon_x = ixmin + xscale*(ping[i].file_time_d - xmin);
		ping[i].lon_y = iymin + yscale*(ping[i].lon - ymin);
		if (ping[i].lon_select == MB_YES)
			xg_drawrectangle(mbnavedit_xgid, 
				ping[i].lon_x-2, 
				ping[i].lon_y-2, 4, 4, 
				pixel_values[RED],XG_SOLIDLINE);
		else
			xg_fillrectangle(mbnavedit_xgid, 
				ping[i].lon_x-2, 
				ping[i].lon_y-2, 4, 4, 
				pixel_values[BLACK],XG_SOLIDLINE);
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
int mbnavedit_plot_lat(int iplot)
{
	/* local variables */
	char	*function_name = "mbnavedit_plot_lat";
	int	status = MB_SUCCESS;
	int	ixmin, ixmax, iymin, iymax;
	double	xmin, xmax, ymin, ymax;
	double	xscale, yscale;
	int	lat_x1, lat_y1, lat_x2, lat_y2;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iplot:       %d\n",iplot);
		}

	/* get scaling values */
	ixmin = mbnavplot[iplot].ixmin;
	ixmax = mbnavplot[iplot].ixmax;
	iymin = mbnavplot[iplot].iymin;
	iymax = mbnavplot[iplot].iymax;
	xmin = mbnavplot[iplot].xmin;
	xmax = mbnavplot[iplot].xmax;
	ymin = mbnavplot[iplot].ymin;
	ymax = mbnavplot[iplot].ymax;
	xscale = mbnavplot[iplot].xscale;
	yscale = mbnavplot[iplot].yscale;

	/* plot original latitude data */
	if (plot_lat_org == MB_YES)
	{
	lat_x1 = ixmin + xscale*(ping[current_id].file_time_d - xmin);
	lat_y1 = iymin + yscale*(ping[current_id].lat_org - ymin);
	for (i=current_id+1;i<current_id+nplot;i++)
		{
		lat_x2 = ixmin + xscale*(ping[i].file_time_d - xmin);
		lat_y2 = iymin + yscale*(ping[i].lat_org - ymin);
		xg_drawline(mbnavedit_xgid,
			lat_x1, lat_y1, lat_x2, lat_y2, 
			pixel_values[GREEN],XG_SOLIDLINE);
		lat_x1 = lat_x2;
		lat_y1 = lat_y2;
		}
	}

	/* plot dr latitude data */
	if (plot_lat_dr == MB_YES)
	{
	lat_x1 = ixmin + xscale*(ping[current_id].file_time_d - xmin);
	lat_y1 = iymin + yscale*(ping[current_id].lat_dr - ymin);
	for (i=current_id+1;i<current_id+nplot;i++)
		{
		lat_x2 = ixmin + xscale*(ping[i].file_time_d - xmin);
		lat_y2 = iymin + yscale*(ping[i].lat_dr - ymin);
		xg_drawline(mbnavedit_xgid,
			lat_x1, lat_y1, lat_x2, lat_y2,
			pixel_values[BLUE],XG_SOLIDLINE);
		lat_x1 = lat_x2;
		lat_y1 = lat_y2;
		}
	}

	/* plot basic latitude data */
	for (i=current_id;i<current_id+nplot;i++)
		{
		ping[i].lat_x = ixmin + xscale*(ping[i].file_time_d - xmin);
		ping[i].lat_y = iymin + yscale*(ping[i].lat - ymin);
		if (ping[i].lat_select == MB_YES)
			xg_drawrectangle(mbnavedit_xgid, 
				ping[i].lat_x-2, 
				ping[i].lat_y-2, 4, 4, 
				pixel_values[RED],XG_SOLIDLINE);
		else
			xg_fillrectangle(mbnavedit_xgid, 
				ping[i].lat_x-2, 
				ping[i].lat_y-2, 4, 4, 
				pixel_values[BLACK],XG_SOLIDLINE);
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
int mbnavedit_plot_speed(int iplot)
{
	/* local variables */
	char	*function_name = "mbnavedit_plot_speed";
	int	status = MB_SUCCESS;
	int	ixmin, ixmax, iymin, iymax;
	double	xmin, xmax, ymin, ymax;
	double	xscale, yscale;
	int	speed_x1, speed_y1, speed_x2, speed_y2;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iplot:       %d\n",iplot);
		}

	/* get scaling values */
	ixmin = mbnavplot[iplot].ixmin;
	ixmax = mbnavplot[iplot].ixmax;
	iymin = mbnavplot[iplot].iymin;
	iymax = mbnavplot[iplot].iymax;
	xmin = mbnavplot[iplot].xmin;
	xmax = mbnavplot[iplot].xmax;
	ymin = mbnavplot[iplot].ymin;
	ymax = mbnavplot[iplot].ymax;
	xscale = mbnavplot[iplot].xscale;
	yscale = mbnavplot[iplot].yscale;

	/* plot original speed data */
	if (plot_speed_org == MB_YES)
	{
	speed_x1 = ixmin + xscale*(ping[current_id].file_time_d - xmin);
	speed_y1 = iymin + yscale*(ping[current_id].speed - ymin);
	for (i=current_id+1;i<current_id+nplot;i++)
		{
		speed_x2 = ixmin + xscale*(ping[i].file_time_d - xmin);
		speed_y2 = iymin + yscale*(ping[i].speed_org - ymin);
		xg_drawline(mbnavedit_xgid,
			speed_x1, speed_y1, speed_x2, speed_y2, 
			pixel_values[GREEN],XG_SOLIDLINE);
		speed_x1 = speed_x2;
		speed_y1 = speed_y2;
		}
	}

	/* plot speed made good data */
	if (plot_smg == MB_YES)
	{
	speed_x1 = ixmin + xscale*(ping[current_id].file_time_d - xmin);
	speed_y1 = iymin + yscale*(ping[current_id].speed_made_good - ymin);
	for (i=current_id+1;i<current_id+nplot;i++)
		{
		speed_x2 = ixmin + xscale*(ping[i].file_time_d - xmin);
		speed_y2 = iymin + yscale*(ping[i].speed_made_good - ymin);
		xg_drawline(mbnavedit_xgid,
			speed_x1, speed_y1, speed_x2, speed_y2, 
			pixel_values[BLUE],XG_SOLIDLINE);
		speed_x1 = speed_x2;
		speed_y1 = speed_y2;
		}
	}

	/* plot basic speed data */
	for (i=current_id;i<current_id+nplot;i++)
		{
		ping[i].speed_x = ixmin + xscale*(ping[i].file_time_d - xmin);
		ping[i].speed_y = iymin + yscale*(ping[i].speed - ymin);
		if (ping[i].speed_select == MB_YES)
			xg_drawrectangle(mbnavedit_xgid, 
				ping[i].speed_x-2, 
				ping[i].speed_y-2, 4, 4, 
				pixel_values[RED],XG_SOLIDLINE);
		else
			xg_fillrectangle(mbnavedit_xgid, 
				ping[i].speed_x-2, 
				ping[i].speed_y-2, 4, 4, 
				pixel_values[BLACK],XG_SOLIDLINE);
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
int mbnavedit_plot_heading(int iplot)
{
	/* local variables */
	char	*function_name = "mbnavedit_plot_heading";
	int	status = MB_SUCCESS;
	int	ixmin, ixmax, iymin, iymax;
	double	xmin, xmax, ymin, ymax;
	double	xscale, yscale;
	int	heading_x1, heading_y1, heading_x2, heading_y2;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iplot:       %d\n",iplot);
		}

	/* get scaling values */
	ixmin = mbnavplot[iplot].ixmin;
	ixmax = mbnavplot[iplot].ixmax;
	iymin = mbnavplot[iplot].iymin;
	iymax = mbnavplot[iplot].iymax;
	xmin = mbnavplot[iplot].xmin;
	xmax = mbnavplot[iplot].xmax;
	ymin = mbnavplot[iplot].ymin;
	ymax = mbnavplot[iplot].ymax;
	xscale = mbnavplot[iplot].xscale;
	yscale = mbnavplot[iplot].yscale;

	/* plot original heading data */
	if (plot_heading_org == MB_YES)
	{
	heading_x1 = ixmin + xscale*(ping[current_id].file_time_d - xmin);
	heading_y1 = iymin + yscale*(ping[current_id].heading - ymin);
	for (i=current_id+1;i<current_id+nplot;i++)
		{
		heading_x2 = ixmin + xscale*(ping[i].file_time_d - xmin);
		heading_y2 = iymin + yscale*(ping[i].heading_org - ymin);
		xg_drawline(mbnavedit_xgid,
			heading_x1, heading_y1, heading_x2, heading_y2, 
			pixel_values[GREEN],XG_SOLIDLINE);
		heading_x1 = heading_x2;
		heading_y1 = heading_y2;
		}
	}

	/* plot course made good data */
	if (plot_cmg == MB_YES)
	{
	heading_x1 = ixmin + xscale*(ping[current_id].file_time_d - xmin);
	heading_y1 = iymin + yscale*(ping[current_id].course_made_good - ymin);
	for (i=current_id+1;i<current_id+nplot;i++)
		{
		heading_x2 = ixmin + xscale*(ping[i].file_time_d - xmin);
		heading_y2 = iymin + yscale*(ping[i].course_made_good - ymin);
		xg_drawline(mbnavedit_xgid,
			heading_x1, heading_y1, heading_x2, heading_y2, 
			pixel_values[BLUE],XG_SOLIDLINE);
		heading_x1 = heading_x2;
		heading_y1 = heading_y2;
		}
	}

	/* plot basic heading data */
	for (i=current_id;i<current_id+nplot;i++)
		{
		ping[i].heading_x = ixmin + xscale*(ping[i].file_time_d - xmin);
		ping[i].heading_y = iymin + yscale*(ping[i].heading - ymin);
		if (ping[i].heading_select == MB_YES)
			xg_drawrectangle(mbnavedit_xgid, 
				ping[i].heading_x-2, 
				ping[i].heading_y-2, 4, 4, 
				pixel_values[RED],XG_SOLIDLINE);
		else
			xg_fillrectangle(mbnavedit_xgid, 
				ping[i].heading_x-2, 
				ping[i].heading_y-2, 4, 4, 
				pixel_values[BLACK],XG_SOLIDLINE);
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
int mbnavedit_plot_roll(int iplot)
{
	/* local variables */
	char	*function_name = "mbnavedit_plot_roll";
	int	status = MB_SUCCESS;
	int	ixmin, ixmax, iymin, iymax;
	double	xmin, xmax, ymin, ymax;
	double	xscale, yscale;
	int	roll_x1, roll_y1, roll_x2, roll_y2;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iplot:       %d\n",iplot);
		}

	/* get scaling values */
	ixmin = mbnavplot[iplot].ixmin;
	ixmax = mbnavplot[iplot].ixmax;
	iymin = mbnavplot[iplot].iymin;
	iymax = mbnavplot[iplot].iymax;
	xmin = mbnavplot[iplot].xmin;
	xmax = mbnavplot[iplot].xmax;
	ymin = mbnavplot[iplot].ymin;
	ymax = mbnavplot[iplot].ymax;
	xscale = mbnavplot[iplot].xscale;
	yscale = mbnavplot[iplot].yscale;

	/* plot roll data */
	if (plot_roll == MB_YES)
	{
	roll_x1 = ixmin + xscale*(ping[current_id].file_time_d - xmin);
	roll_y1 = iymin + yscale*(ping[current_id].roll - ymin);
	for (i=current_id+1;i<current_id+nplot;i++)
		{
		roll_x2 = ixmin + xscale*(ping[i].file_time_d - xmin);
		roll_y2 = iymin + yscale*(ping[i].roll - ymin);
		xg_drawline(mbnavedit_xgid,
			roll_x1, roll_y1, roll_x2, roll_y2, 
			pixel_values[GREEN],XG_SOLIDLINE);
		roll_x1 = roll_x2;
		roll_y1 = roll_y2;
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
int mbnavedit_plot_pitch(int iplot)
{
	/* local variables */
	char	*function_name = "mbnavedit_plot_pitch";
	int	status = MB_SUCCESS;
	int	ixmin, ixmax, iymin, iymax;
	double	xmin, xmax, ymin, ymax;
	double	xscale, yscale;
	int	pitch_x1, pitch_y1, pitch_x2, pitch_y2;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iplot:       %d\n",iplot);
		}

	/* get scaling values */
	ixmin = mbnavplot[iplot].ixmin;
	ixmax = mbnavplot[iplot].ixmax;
	iymin = mbnavplot[iplot].iymin;
	iymax = mbnavplot[iplot].iymax;
	xmin = mbnavplot[iplot].xmin;
	xmax = mbnavplot[iplot].xmax;
	ymin = mbnavplot[iplot].ymin;
	ymax = mbnavplot[iplot].ymax;
	xscale = mbnavplot[iplot].xscale;
	yscale = mbnavplot[iplot].yscale;

	/* plot pitch data */
	if (plot_pitch == MB_YES)
	{
	pitch_x1 = ixmin + xscale*(ping[current_id].file_time_d - xmin);
	pitch_y1 = iymin + yscale*(ping[current_id].pitch - ymin);
	for (i=current_id+1;i<current_id+nplot;i++)
		{
		pitch_x2 = ixmin + xscale*(ping[i].file_time_d - xmin);
		pitch_y2 = iymin + yscale*(ping[i].pitch - ymin);
		xg_drawline(mbnavedit_xgid,
			pitch_x1, pitch_y1, pitch_x2, pitch_y2, 
			pixel_values[GREEN],XG_SOLIDLINE);
		pitch_x1 = pitch_x2;
		pitch_y1 = pitch_y2;
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
int mbnavedit_plot_heave(int iplot)
{
	/* local variables */
	char	*function_name = "mbnavedit_plot_heave";
	int	status = MB_SUCCESS;
	int	ixmin, ixmax, iymin, iymax;
	double	xmin, xmax, ymin, ymax;
	double	xscale, yscale;
	int	heave_x1, heave_y1, heave_x2, heave_y2;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iplot:       %d\n",iplot);
		}

	/* get scaling values */
	ixmin = mbnavplot[iplot].ixmin;
	ixmax = mbnavplot[iplot].ixmax;
	iymin = mbnavplot[iplot].iymin;
	iymax = mbnavplot[iplot].iymax;
	xmin = mbnavplot[iplot].xmin;
	xmax = mbnavplot[iplot].xmax;
	ymin = mbnavplot[iplot].ymin;
	ymax = mbnavplot[iplot].ymax;
	xscale = mbnavplot[iplot].xscale;
	yscale = mbnavplot[iplot].yscale;

	/* plot heave data */
	if (plot_heave == MB_YES)
	{
	heave_x1 = ixmin + xscale*(ping[current_id].file_time_d - xmin);
	heave_y1 = iymin + yscale*(ping[current_id].heave - ymin);
	for (i=current_id+1;i<current_id+nplot;i++)
		{
		heave_x2 = ixmin + xscale*(ping[i].file_time_d - xmin);
		heave_y2 = iymin + yscale*(ping[i].heave - ymin);
		xg_drawline(mbnavedit_xgid,
			heave_x1, heave_y1, heave_x2, heave_y2, 
			pixel_values[GREEN],XG_SOLIDLINE);
		heave_x1 = heave_x2;
		heave_y1 = heave_y2;
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
int mbnavedit_plot_tint_value(int iplot, int iping)
{
	/* local variables */
	char	*function_name = "mbnavedit_plot_tint_value";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iplot:       %d\n",iplot);
		fprintf(stderr,"dbg2       iping:       %d\n",iping);
		}

	/* unplot basic expected time data value */
	xg_drawrectangle(mbnavedit_xgid, 
			ping[iping].tint_x-2, 
			ping[iping].tint_y-2, 4, 4, 
			pixel_values[WHITE],XG_SOLIDLINE);
	xg_fillrectangle(mbnavedit_xgid, 
			ping[iping].tint_x-2, 
			ping[iping].tint_y-2, 4, 4, 
			pixel_values[WHITE],XG_SOLIDLINE);

	/* replot basic expected time data value */
	if (ping[iping].tint_select == MB_YES)
		xg_drawrectangle(mbnavedit_xgid, 
			ping[iping].tint_x-2, 
			ping[iping].tint_y-2, 4, 4, 
			pixel_values[RED],XG_SOLIDLINE);
	else
		xg_fillrectangle(mbnavedit_xgid, 
			ping[iping].tint_x-2, 
			ping[iping].tint_y-2, 4, 4, 
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
int mbnavedit_plot_lon_value(int iplot, int iping)
{
	/* local variables */
	char	*function_name = "mbnavedit_plot_lon_value";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iplot:       %d\n",iplot);
		fprintf(stderr,"dbg2       iping:       %d\n",iping);
		}

	/* unplot basic lon data value */
	xg_drawrectangle(mbnavedit_xgid, 
			ping[iping].lon_x-2, 
			ping[iping].lon_y-2, 4, 4, 
			pixel_values[WHITE],XG_SOLIDLINE);
	xg_fillrectangle(mbnavedit_xgid, 
			ping[iping].lon_x-2, 
			ping[iping].lon_y-2, 4, 4, 
			pixel_values[WHITE],XG_SOLIDLINE);

	/* replot basic lon data value */
	if (ping[iping].lon_select == MB_YES)
		xg_drawrectangle(mbnavedit_xgid, 
			ping[iping].lon_x-2, 
			ping[iping].lon_y-2, 4, 4, 
			pixel_values[RED],XG_SOLIDLINE);
	else
		xg_fillrectangle(mbnavedit_xgid, 
			ping[iping].lon_x-2, 
			ping[iping].lon_y-2, 4, 4, 
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
int mbnavedit_plot_lat_value(int iplot, int iping)
{
	/* local variables */
	char	*function_name = "mbnavedit_plot_lat_value";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iplot:       %d\n",iplot);
		fprintf(stderr,"dbg2       iping:       %d\n",iping);
		}

	/* unplot basic lat data value */
	xg_drawrectangle(mbnavedit_xgid, 
			ping[iping].lat_x-2, 
			ping[iping].lat_y-2, 4, 4, 
			pixel_values[WHITE],XG_SOLIDLINE);
	xg_fillrectangle(mbnavedit_xgid, 
			ping[iping].lat_x-2, 
			ping[iping].lat_y-2, 4, 4, 
			pixel_values[WHITE],XG_SOLIDLINE);

	/* replot basic lat data value */
	if (ping[iping].lat_select == MB_YES)
		xg_drawrectangle(mbnavedit_xgid, 
			ping[iping].lat_x-2, 
			ping[iping].lat_y-2, 4, 4, 
			pixel_values[RED],XG_SOLIDLINE);
	else
		xg_fillrectangle(mbnavedit_xgid, 
			ping[iping].lat_x-2, 
			ping[iping].lat_y-2, 4, 4, 
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
int mbnavedit_plot_speed_value(int iplot, int iping)
{
	/* local variables */
	char	*function_name = "mbnavedit_plot_speed_value";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iplot:       %d\n",iplot);
		fprintf(stderr,"dbg2       iping:       %d\n",iping);
		}

	/* unplot basic speed data value */
	xg_drawrectangle(mbnavedit_xgid, 
			ping[iping].speed_x-2, 
			ping[iping].speed_y-2, 4, 4, 
			pixel_values[WHITE],XG_SOLIDLINE);
	xg_fillrectangle(mbnavedit_xgid, 
			ping[iping].speed_x-2, 
			ping[iping].speed_y-2, 4, 4, 
			pixel_values[WHITE],XG_SOLIDLINE);

	/* replot basic speed data value */
	if (ping[iping].speed_select == MB_YES)
		xg_drawrectangle(mbnavedit_xgid, 
			ping[iping].speed_x-2, 
			ping[iping].speed_y-2, 4, 4, 
			pixel_values[RED],XG_SOLIDLINE);
	else
		xg_fillrectangle(mbnavedit_xgid, 
			ping[iping].speed_x-2, 
			ping[iping].speed_y-2, 4, 4, 
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
int mbnavedit_plot_heading_value(int iplot, int iping)
{
	/* local variables */
	char	*function_name = "mbnavedit_plot_heading_value";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iplot:       %d\n",iplot);
		fprintf(stderr,"dbg2       iping:       %d\n",iping);
		}

	/* unplot basic heading data value */
	xg_drawrectangle(mbnavedit_xgid, 
			ping[iping].heading_x-2, 
			ping[iping].heading_y-2, 4, 4, 
			pixel_values[WHITE],XG_SOLIDLINE);
	xg_fillrectangle(mbnavedit_xgid, 
			ping[iping].heading_x-2, 
			ping[iping].heading_y-2, 4, 4, 
			pixel_values[WHITE],XG_SOLIDLINE);

	/* replot basic heading data value */
	if (ping[iping].heading_select == MB_YES)
		xg_drawrectangle(mbnavedit_xgid, 
			ping[iping].heading_x-2, 
			ping[iping].heading_y-2, 4, 4, 
			pixel_values[RED],XG_SOLIDLINE);
	else
		xg_fillrectangle(mbnavedit_xgid, 
			ping[iping].heading_x-2, 
			ping[iping].heading_y-2, 4, 4, 
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
