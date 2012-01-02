/*--------------------------------------------------------------------
 *    The MB-system:	mbedit.c	4/8/93
 *    $Id$
 *
 *    Copyright (c) 1993-2011 by
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
 * Date:	September 19, 2000 (New version - no buffered i/o)
 *
 * $Log: mbedit_prog.c,v $
 * Revision 5.39  2008/07/19 07:28:06  caress
 * Fixed scaling for swath widths less than 6 m.
 *
 * Revision 5.38  2008/07/10 06:43:40  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.37  2008/01/14 17:49:48  caress
 * Fixed problem with speed data.
 *
 * Revision 5.36  2007/10/08 07:20:21  caress
 * Fixed vertical scaling of large numbers of pings. Augmented zap box function.
 *
 * Revision 5.35  2007/05/14 06:30:29  caress
 * The acrosstrack view of MBedit now shows the real alongtrack
 * positions of the soundings.
 *
 * Revision 5.34  2006/09/11 18:55:52  caress
 * Changes during Western Flyer and Thomas Thompson cruises, August-September
 * 2006.
 *
 * Revision 5.33  2006/08/09 22:35:33  caress
 * A new edit mode called "grab" has been added to MBedit. In this
 * mode, the user drags a rectangle on the ping display. When the mouse
 * is released, all unflagged beams within that rectangle are flagged.
 *
 * Revision 5.32  2006/08/04 03:56:41  caress
 * Working towards 5.1.0 release.
 *
 * Revision 5.31  2006/07/06 05:30:57  caress
 * Working more towards 5.1.0beta
 *
 * Revision 5.30  2006/06/16 19:30:58  caress
 * Check in after the Santa Monica Basin Mapping AUV Expedition.
 *
 * Revision 5.29  2006/01/24 19:12:42  caress
 * Version 5.0.8 beta.
 *
 * Revision 5.28  2006/01/20 19:36:19  caress
 * Working towards 5.0.8
 *
 * Revision 5.27  2006/01/06 18:25:45  caress
 * Working towards 5.0.8
 *
 * Revision 5.26  2005/11/04 22:51:11  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.25  2005/03/25 04:12:24  caress
 * MBedit now allows alongtrack and acrosstrack views as well as the traditional waterfall display of profiles.
 *
 * Revision 5.24  2004/12/02 06:31:02  caress
 * First cut at adding stacked views from along and across track.
 *
 * Revision 5.23  2004/05/21 23:26:04  caress
 * Moved to new version of BX GUI builder
 *
 * Revision 5.22  2003/07/30 16:39:32  caress
 * Fixed ping nulling (shift-! key macro).
 * Augmented patience window.
 *
 * Revision 5.22  2003/07/28 05:19:34  caress
 * Fixed label_message.
 *
 * Revision 5.21  2003/07/26 17:58:52  caress
 * Changed beamflag code.
 *
 * Revision 5.20  2003/04/17 20:50:01  caress
 * Release 5.0.beta30
 *
 * Revision 5.19  2003/04/17 20:45:42  caress
 * Release 5.0.beta30
 *
 * Revision 5.18  2003/03/10 19:57:07  caress
 * Added mr1pr library.
 *
 * Revision 5.17  2003/01/15 20:50:40  caress
 * Release 5.0.beta28
 *
 * Revision 5.16  2002/10/02 23:53:44  caress
 * Release 5.0.beta24
 *
 * Revision 5.15  2002/09/19 00:35:53  caress
 * Release 5.0.beta23
 *
 * Revision 5.14  2002/08/30 19:28:21  caress
 * Added time series style plots.
 *
 * Revision 5.13  2002/07/20 20:45:04  caress
 * Release 5.0.beta20
 *
 * Revision 5.12  2002/05/30 21:33:02  caress
 * Release 5.0.beta18
 *
 * Revision 5.11  2002/05/29 23:36:28  caress
 * Release 5.0.beta18
 *
 * Revision 5.10  2002/05/02 03:53:45  caress
 * Release 5.0.beta17
 *
 * Revision 5.9  2001/11/16 01:25:20  caress
 * Added info mode.
 *
 * Revision 5.8  2001/09/17  17:00:48  caress
 * Added local median filter, angle filter, time display toggle.
 *
 * Revision 5.7  2001/07/31  00:40:17  caress
 * Added flagging by beam number and acrosstrack distance.
 *
 * Revision 5.6  2001/07/20  00:30:32  caress
 * Release 5.0.beta03
 *
 * Revision 5.5  2001/06/30 17:39:31  caress
 * Release 5.0.beta02
 *
 * Revision 5.4  2001/03/22  21:06:55  caress
 * Trying to make release 5.0.beta0
 *
 * Revision 5.3  2001/01/23  01:17:34  caress
 * Removed some unused variable declarations.
 *
 * Revision 5.2  2001/01/22  07:40:13  caress
 * Version 5.0.0beta01
 *
 * Revision 5.1  2000/12/10  20:29:13  caress
 * Version 5.0.alpha02
 *
 * Revision 5.0  2000/12/01  22:54:35  caress
 * First cut at Version 5.0.
 *
 * Revision 4.32  2000/10/11  01:01:50  caress
 * Convert to ANSI C
 *
 * Revision 4.31  2000/10/03  21:49:04  caress
 * Fixed handling of buffer.
 *
 * Revision 4.30  2000/09/30  06:56:36  caress
 * Snapshot for Dale.
 * New version integrated with mbprocess.
 *
 * Revision 4.29  2000/09/08  00:29:20  caress
 * Revision of 7 September 2000.
 *
 * Revision 4.28  2000/03/16  00:35:40  caress
 * Added mode to output edit save file only.
 *
 * Revision 4.27  2000/01/26  03:02:05  caress
 * Fixed bug in making output filename.
 *
 * Revision 4.26  2000/01/25  01:46:20  caress
 * Altered handling of filenames and edit save files.
 *
 * Revision 4.25  2000/01/20  00:05:38  caress
 * Added pick mode and two unflag buttons.
 *
 * Revision 4.24  1999/02/12  21:19:30  caress
 * Fixed buffer size handling.
 *
 * Revision 4.23  1999/02/04  23:52:20  caress
 * MB-System version 4.6beta7
 *
 * Revision 4.22  1998/12/17  22:56:00  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.21  1998/10/28  21:31:56  caress
 * Fixed handling of data with variable numbers of beams.
 *
 * Revision 4.20  1998/10/05  17:45:32  caress
 * MB-System version 4.6beta
 *
 * Revision 4.19  1997/10/03  18:32:07  caress
 * Fixed problem with sort call.
 *
 * Revision 4.18  1997/09/15  19:06:10  caress
 * Real Version 4.5
 *
 * Revision 4.17  1997/07/25  14:42:55  caress
 * Version 4.5beta2
 *
 * Revision 4.16  1997/04/29  15:50:50  caress
 * Fixed autoscaling in case of no good data.
 *
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
#include "mbedit.h"

/* output mode defines */
#define	MBEDIT_OUTPUT_EDIT   1
#define	MBEDIT_OUTPUT_BROWSE 2

/* edit outbounds defines */
#define	MBEDIT_OUTBOUNDS_NONE		0
#define	MBEDIT_OUTBOUNDS_FLAGGED	1
#define	MBEDIT_OUTBOUNDS_UNFLAGGED	2

/* plot modes */
#define MBEDIT_PLOT_WIDE		0
#define MBEDIT_PLOT_TIME		1
#define MBEDIT_PLOT_INTERVAL		2
#define MBEDIT_PLOT_LON			3
#define MBEDIT_PLOT_LAT			4
#define MBEDIT_PLOT_HEADING		5
#define MBEDIT_PLOT_SPEED		6
#define MBEDIT_PLOT_DEPTH		7
#define MBEDIT_PLOT_ALTITUDE		8
#define MBEDIT_PLOT_SONARDEPTH		9
#define MBEDIT_PLOT_ROLL		10
#define MBEDIT_PLOT_PITCH		11
#define MBEDIT_PLOT_HEAVE		12

/* view modes */
#define MBEDIT_VIEW_WATERFALL		0
#define MBEDIT_VIEW_ALONGTRACK		1
#define MBEDIT_VIEW_ACROSSTRACK		2
#define MBEDIT_SHOW_FLAG		0
#define MBEDIT_SHOW_DETECT		1
#define MBEDIT_SHOW_PULSE		2

/* grab modes */
#define MBEDIT_GRAB_START		0
#define MBEDIT_GRAB_MOVE		1
#define MBEDIT_GRAB_END			2

/* Bottom detect type names */
char *detect_name[] = 
	{	"Unknown",
		"Amplitude",
		"Phase"
	};

/* Source pulse type names */
char *pulse_name[] = 
	{	"Unknown",
		"CW",
		"Up-Chirp",
		"Down-Chirp"
	};

/* ping structure definition */
struct mbedit_ping_struct 
	{
	int	allocated;
	int	id;
	int	record;
	int	outbounds;
	int	time_i[7];
	double	time_d;
	int	multiplicity;
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
	double	distance;
	int	beams_bath;
	char	*beamflag;
	char	*beamflagorg;
	double	*bath;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	int	*detect;
	int	*pulses;
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
static char rcs_id[] = "$Id$";
static char program_name[] = "MBedit";
static char help_message[] =  
"MBedit is an interactive editor used to identify and flag\n\
artifacts in swath sonar bathymetry data. Once a file has\n\
been read in, MBedit displays the bathymetry profiles from\n\
several pings, allowing the user to identify and flag\n\
anomalous beams. Flagging is handled internally by setting\n\
depth values negative, so that no information is lost.";
static char usage_message[] = "mbedit [-Byr/mo/da/hr/mn/sc -D  -Eyr/mo/da/hr/mn/sc \n\t-Fformat -Ifile -Ooutfile -S -X -V -H]";

/* status variables */
static int	error = MB_ERROR_NO_ERROR;
static int	verbose = 0;
static char	*message = NULL;

/* MBIO control parameters */
static int	format;
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
static char	ifile[MB_PATH_MAXLINE];
static void	*imbio_ptr = NULL;
static int	output_mode = MBEDIT_OUTPUT_EDIT;
static int	run_mbprocess = MB_NO;
static int	gui_mode = MB_NO;
static int	uselockfiles = MB_YES;

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
#define	MBEDIT_BUFFER_SIZE	30000
static int	file_open = MB_NO;
static int	buff_size = MBEDIT_BUFFER_SIZE;
static int	buff_size_max = MBEDIT_BUFFER_SIZE;
static int	holdd_size = MBEDIT_BUFFER_SIZE / 1000;
static int	nload = 0;
static int	ndump = 0;
static int	nbuff = 0;
static int	current_id = 0;
static int	nload_total = 0;
static int	ndump_total = 0;
static char	last_ping[MB_PATH_MAXLINE];
static int	file_id;
static int	num_files;

/* info parameters */
static int	info_set = MB_NO;
static int	info_ping;
static int	info_beam;
static int	info_time_i[7];
static double	info_time_d;
static double	info_navlon;
static double	info_navlat;
static double	info_speed;
static double	info_heading;
static double	info_altitude;
static int	info_beams_bath;
static char	info_beamflag;
static double	info_bath;
static double	info_bathacrosstrack;
static double	info_bathalongtrack;
static int	info_detect;
static int	info_pulse;

/* grab parameters */
static int	grab_set = MB_NO;
static int	grab_start_x;
static int	grab_start_y;
static int	grab_end_x;
static int	grab_end_y;

/* save file control variables */
static int	esffile_open = MB_NO;
struct mb_esf_struct esf;
static char	esffile[MB_PATH_MAXLINE];
static char	notice[MB_PATH_MAXLINE];

/* filter variables */
static int	filter_medianspike = MB_NO;
static int	filter_medianspike_threshold = 10;
static int	filter_medianspike_xtrack = 5;
static int	filter_medianspike_ltrack = 1;
static int	filter_wrongside = MB_NO;
static int	filter_wrongside_threshold = 15;
static int	filter_cutbeam = MB_NO;
static int	filter_cutbeam_begin = 0;
static int	filter_cutbeam_end = 0;
static int	filter_cutdistance = MB_NO;
static double	filter_cutdistance_begin = 0.0;
static double	filter_cutdistance_end = 0.0;
static int	filter_cutangle = MB_NO;
static double	filter_cutangle_begin = 0.0;
static double	filter_cutangle_end = 0.0;

/* ping drawing control variables */
#define	MBEDIT_MAX_PINGS	250
#define	MBEDIT_PICK_DISTANCE	50
#define	MBEDIT_ERASE_DISTANCE	15
struct mbedit_ping_struct	ping[MBEDIT_BUFFER_SIZE];
static int	view_mode = MBEDIT_VIEW_WATERFALL;
static int	plot_size = 10;
static int	nplot = 0;
static void	*mbedit_xgid;
static int	borders[4];
static int	margin;
static int	xmin, xmax;
static int	ymin, ymax;
static int	exager = 100;
static int	plot_width = 5000;
static double	xscale;
static double	yscale;
static int	x_interval = 1000;
static int	y_interval = 250;
static int	show_mode = MBEDIT_SHOW_FLAG;
static int	show_flagged = MB_NO;
static int	show_time = MBEDIT_PLOT_TIME;
static int	beam_save = MB_NO;
static int	iping_save = 0;
static int	jbeam_save = 0;
static double	*bathlist;

/* color control values */
#define	WHITE	0	
#define	BLACK	1	
#define RED	2
#define GREEN	3
#define BLUE	4
#define CORAL	5
#define LIGHTGREY	6
#define	XG_SOLIDLINE	0
#define	XG_DASHLINE	1
static int	ncolors;
static unsigned int	pixel_values[256];

/*--------------------------------------------------------------------*/
int mbedit_init(int argc, char ** argv, int *startup_file)
{
	/* local variables */
	char	*function_name = "mbedit_init";
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
	status = mb_uselockfiles(verbose,&uselockfiles);
	format = 0;
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
	while ((c = getopt(argc, argv, "VvHhB:b:DdE:e:F:f:GgI:i:SsXx")) != -1)
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
			do_parse_datalist(ifile, format);
			flag++;
			fileflag++;
			break;
		case 'X':
		case 'x':
			run_mbprocess = MB_YES;
			flag++;
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
		fprintf(stderr,"dbg2       input file:      %s\n",ifile);
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
int mbedit_set_graphics(void *xgid, int ncol, unsigned int *pixels)
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
		fprintf(stderr,"dbg2       xgid:         %lu\n",(size_t)xgid);
		fprintf(stderr,"dbg2       ncolors:      %d\n",ncol);
		for (i=0;i<ncol;i++)
			fprintf(stderr,"dbg2       pixel[%d]:     %d\n",
				i, pixels[i]);
		}

	/* set graphics id */
	mbedit_xgid = xgid;

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
int mbedit_set_scaling(int *brdr, int sh_time)
{
	/* local variables */
	char	*function_name = "mbedit_set_scaling";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		for (i=0;i<4;i++)
			fprintf(stderr,"dbg2       brdr[%d]:     %d\n",
				i,brdr[i]);
		fprintf(stderr,"dbg2       show_time:      %d\n",sh_time);
		}

	/* set graphics bounds */
	for (i=0;i<4;i++)
		borders[i] = brdr[i];

	/* set scaling */
	show_time = sh_time;
	if (show_time > MBEDIT_PLOT_WIDE)
		{
		margin = (borders[1] - borders[0])/16;
		xmin = 5 * margin;
		xmax = borders[1] - margin;
		ymin = margin;
		ymax = borders[3] - margin/2;
		xscale = 100.0 * plot_width / (xmax - xmin);
		yscale = (xscale * exager) / 100.0;
		}
	else
		{
		margin = (borders[1] - borders[0])/16;
		xmin = 2 * margin + 20;
		xmax = borders[1] - margin;
		ymin = margin;
		ymax = borders[3] - margin/2;
		xscale = 100.0 * plot_width / (xmax - xmin);
		yscale = (xscale * exager) / 100.0;
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

	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_set_filters(int f_m, int f_m_t, 
			int f_m_x, int f_m_l, 
			int f_w, int f_w_t, 
			int f_b, int f_b_b, int f_b_e, 
			int f_d, double f_d_b, double f_d_e, 
			int f_a, double f_a_b, double f_a_e)
{
	/* local variables */
	char	*function_name = "mbedit_set_filters";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       f_m:     %d\n",f_m);
		fprintf(stderr,"dbg2       f_m_t:   %d\n",f_m_t);
		fprintf(stderr,"dbg2       f_m_x:   %d\n",f_m_x);
		fprintf(stderr,"dbg2       f_m_l:   %d\n",f_m_l);
 		fprintf(stderr,"dbg2       f_w:     %d\n",f_w);
		fprintf(stderr,"dbg2       f_w_t:   %d\n",f_w_t);
 		fprintf(stderr,"dbg2       f_b:     %d\n",f_b);
		fprintf(stderr,"dbg2       f_b_b:   %d\n",f_b_b);
		fprintf(stderr,"dbg2       f_b_e:   %d\n",f_b_e);
 		fprintf(stderr,"dbg2       f_d:     %d\n",f_d);
		fprintf(stderr,"dbg2       f_d_b:   %f\n",f_d_b);
		fprintf(stderr,"dbg2       f_d_e:   %f\n",f_d_e);
 		fprintf(stderr,"dbg2       f_a:     %d\n",f_a);
		fprintf(stderr,"dbg2       f_a_b:   %f\n",f_a_b);
		fprintf(stderr,"dbg2       f_a_e:   %f\n",f_a_e);
 		}
 		
 	/* set the filter values */
 	filter_medianspike = f_m;
 	filter_medianspike_threshold = f_m_t;
	filter_medianspike_xtrack = f_m_x;
	filter_medianspike_ltrack = f_m_l;
 	filter_wrongside = f_w;
 	filter_wrongside_threshold = f_w_t;
 	filter_cutbeam = f_b;
 	filter_cutbeam_begin = f_b_b;
 	filter_cutbeam_end = f_b_e;
 	filter_cutdistance = f_d;
 	filter_cutdistance_begin = f_d_b;
 	filter_cutdistance_end = f_d_e;
 	filter_cutangle = f_a;
 	filter_cutangle_begin = f_a_b;
 	filter_cutangle_end = f_a_e;

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
int mbedit_get_filters( int *b_m, double *d_m,
			int *f_m, int *f_m_t, 
			int *f_m_x, int *f_m_l, 
			int *f_w, int *f_w_t, 
			int *f_b, int *f_b_b, int *f_b_e, 
			int *f_d, double *f_d_b, double *f_d_e, 
			int *f_a, double *f_a_b, double *f_a_e)
{
	/* local variables */
	char	*function_name = "mbedit_get_filters";
	int	status = MB_SUCCESS;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       b_m:     %lu\n",(size_t)b_m);
		fprintf(stderr,"dbg2       d_m:     %lu\n",(size_t)d_m);
 		fprintf(stderr,"dbg2       f_m:     %lu\n",(size_t)f_m);
		fprintf(stderr,"dbg2       f_m_t:   %lu\n",(size_t)f_m_t);
		fprintf(stderr,"dbg2       f_m_x:   %lu\n",(size_t)f_m_x);
		fprintf(stderr,"dbg2       f_m_l:   %lu\n",(size_t)f_m_l);
 		fprintf(stderr,"dbg2       f_w:     %lu\n",(size_t)f_w);
		fprintf(stderr,"dbg2       f_w_t:   %lu\n",(size_t)f_w_t);
 		fprintf(stderr,"dbg2       f_b:     %lu\n",(size_t)f_b);
		fprintf(stderr,"dbg2       f_b_b:   %lu\n",(size_t)f_b_b);
		fprintf(stderr,"dbg2       f_b_e:   %lu\n",(size_t)f_b_e);
 		fprintf(stderr,"dbg2       f_d:     %lu\n",(size_t)f_d);
		fprintf(stderr,"dbg2       f_d_b:   %lu\n",(size_t)f_d_b);
		fprintf(stderr,"dbg2       f_d_e:   %lu\n",(size_t)f_d_e);
 		fprintf(stderr,"dbg2       f_a:     %lu\n",(size_t)f_a);
		fprintf(stderr,"dbg2       f_a_b:   %lu\n",(size_t)f_a_b);
		fprintf(stderr,"dbg2       f_a_e:   %lu\n",(size_t)f_a_e);
		}

	/* set max beam number and acrosstrack distance */
	*b_m = 0;
	*d_m = 0.0;
	if (file_open == MB_YES)
		{
		/* loop over all pings */
		for (i=0;i<nbuff;i++)
		    {
		    for (j=0;j<ping[i].beams_bath;j++)
			{
			if (mb_beam_ok(ping[i].beamflag[j]))
			    {
			    *b_m = MAX(*b_m, ping[i].beams_bath);
			    *d_m = MAX(*d_m, fabs(ping[i].bathacrosstrack[j]));;
			    }
			}
		    }
		}
	if (*b_m == 0)
 		*b_m = 200;
	if (*d_m == 0.0)
 		*d_m = 10000.0;
		
 	/* set the filter values */
 	*f_m = filter_medianspike;
 	*f_m_t = filter_medianspike_threshold;
 	*f_m_x = filter_medianspike_xtrack;
 	*f_m_l = filter_medianspike_ltrack;
 	*f_w = filter_wrongside;
 	*f_w_t = filter_wrongside_threshold;
 	*f_b = filter_cutbeam;
 	*f_b_b = filter_cutbeam_begin;
 	*f_b_e = filter_cutbeam_end;
 	*f_d = filter_cutdistance;
 	*f_d_b = filter_cutdistance_begin;
 	*f_d_e = filter_cutdistance_end;
 	*f_a = filter_cutangle;
 	*f_a_b = filter_cutangle_begin;
 	*f_a_e = filter_cutangle_end;
 	
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       b_m:     %d\n",*b_m);
		fprintf(stderr,"dbg2       d_m:     %f\n",*d_m);
		fprintf(stderr,"dbg2       f_m:     %d\n",*f_m);
		fprintf(stderr,"dbg2       f_m_t:   %d\n",*f_m_t);
		fprintf(stderr,"dbg2       f_m_x:   %d\n",*f_m_x);
		fprintf(stderr,"dbg2       f_m_l:   %d\n",*f_m_l);
 		fprintf(stderr,"dbg2       f_w:     %d\n",*f_w);
		fprintf(stderr,"dbg2       f_w_t:   %d\n",*f_w_t);
 		fprintf(stderr,"dbg2       f_b:     %d\n",*f_b);
		fprintf(stderr,"dbg2       f_b_b:   %d\n",*f_b_b);
		fprintf(stderr,"dbg2       f_b_e:   %d\n",*f_b_e);
 		fprintf(stderr,"dbg2       f_d:     %d\n",*f_d);
		fprintf(stderr,"dbg2       f_d_b:   %f\n",*f_d_b);
		fprintf(stderr,"dbg2       f_d_e:   %f\n",*f_d_e);
 		fprintf(stderr,"dbg2       f_a:     %d\n",*f_a);
		fprintf(stderr,"dbg2       f_a_b:   %f\n",*f_a_b);
		fprintf(stderr,"dbg2       f_a_e:   %f\n",*f_a_e);
		fprintf(stderr,"dbg2       error:   %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbedit_get_defaults(
		int	*plt_size_max, 
		int	*plt_size, 
		int	*sh_mode, 
		int	*sh_flggd, 
		int	*sh_time,
		int	*buffer_size_max, 
		int	*buffer_size, 
		int	*hold_size, 
		int	*form, 
		int	*plwd, 
		int	*exgr, 
		int	*xntrvl, 
		int	*yntrvl, 
		int	*ttime_i, 
		int	*outmode)
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
	
	/* get show mode flag */
	*sh_mode = show_mode;
	
	/* get show flagged flag */
	*sh_flggd = show_flagged;

	/* get show time flag */
	*sh_time = show_time;

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
	if (file_open == MB_YES && nbuff > 0)
		{
		for (i=0;i<7;i++)
			ttime_i[i] = ping[0].time_i[i];
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
		fprintf(stderr,"dbg2       show_mode:   %d\n",*sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",*sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",*sh_time);
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
				i, ttime_i[i]);
		fprintf(stderr,"dbg2       outmode:     %d\n",*outmode);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_get_viewmode(int *vw_mode)
{
	/* local variables */
	char	*function_name = "mbedit_get_viewmode";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* get view mode */
	*vw_mode = view_mode;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       view_mode:   %d\n",*vw_mode);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_set_viewmode(int vw_mode)
{
	/* local variables */
	char	*function_name = "mbedit_set_viewmode";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       view_mode:   %d\n",vw_mode);
		}

	/* get view mode */
	view_mode = vw_mode;

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
int mbedit_action_open(
		char	*file, 
		int	form, 
		int	fileid, 
		int	numfiles, 
		int	savemode, 
		int	outmode, 
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*buffer_size, 
		int	*buffer_size_max, 
		int	*hold_size, 
		int	*ndumped, 
		int	*nloaded, 
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
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
		fprintf(stderr,"dbg2       fileid:          %d\n",fileid);
		fprintf(stderr,"dbg2       numfiles:        %d\n",numfiles);
		fprintf(stderr,"dbg2       savemode:        %d\n",savemode);
		fprintf(stderr,"dbg2       outmode:         %d\n",outmode);
		fprintf(stderr,"dbg2       plot_width:      %d\n",plwd);
		fprintf(stderr,"dbg2       exager:          %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:      %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:      %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:       %d\n",plt_size);
		fprintf(stderr,"dbg2       show_mode:       %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:    %d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:       %d\n",sh_time);
		fprintf(stderr,"dbg2       buffer_size:     %d\n",*buffer_size);
		fprintf(stderr,"dbg2       buffer_size_max: %d\n",*buffer_size_max);
		fprintf(stderr,"dbg2       hold_size:       %d\n",*hold_size);
		}

	/* reset info */
	info_set = MB_NO;

	/* set the output mode */
	output_mode = outmode;

	/* clear the screen */
	status = mbedit_clear_screen();

	/* open the file */
	status = mbedit_open_file(file, form, savemode);
	
	/* check buffer size */
	if (status == MB_SUCCESS)
		{
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
		
		/* if no data read show error dialog */
		if (*nloaded == 0)
			do_error_dialog("No data were loaded from the input", 
					"file. You may have specified an", 
					"incorrect MB-System format id!");
		}

	/* set up plotting */
	if (status == MB_SUCCESS && *ngood > 0)
		{		
		/* turn file button off */
		do_filebutton_off();

		/* now plot it */
		status = mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,
			    plt_size,sh_mode,sh_flggd,sh_time,nplt,MB_YES);
		
		/* set fileid and numfiles */
		file_id = fileid;
		num_files = numfiles;
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
int mbedit_action_next_buffer(
		int	hold_size, 
		int	buffer_size, 
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*ndumped, 
		int	*nloaded, 
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt, 
		int	*quit)
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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	info_set = MB_NO;

	/* clear the screen */
	status = mbedit_clear_screen();
	
	/* set quit off */
	*quit = MB_NO;

	/* check if a file has been opened */
	if (file_open == MB_YES)
		{
		/* set buffer size */
		buff_size = buffer_size;
		holdd_size = hold_size;

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
				plt_size,sh_mode,sh_flggd,sh_time,nplt,MB_YES);
			}
		}

	/* if no file open set failure status */
	else
		{
		status = MB_FAILURE;
		*ndumped = 0;
		*nloaded = 0;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
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
int mbedit_action_close(
		int	buffer_size, 
		int	*ndumped, 
		int	*nloaded, 
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent)
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

	/* reset info */
	info_set = MB_NO;

	/* clear the screen */
	status = mbedit_clear_screen();

	/* if file has been opened and in browse mode 
		just dump the current buffer and close the file */
	if (file_open == MB_YES 
		&& (output_mode == MBEDIT_OUTPUT_BROWSE
		    || (output_mode == MBEDIT_OUTPUT_EDIT
			&& esf.nedit == 0)))
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
int mbedit_action_done(
		int	buffer_size, 
		int	*ndumped, 
		int	*nloaded, 
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*quit)
{
	/* local variables */
	char	*function_name = "mbedit_action_done";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		}

	/* reset info */
	info_set = MB_NO;

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
int mbedit_action_quit(
		int	buffer_size, 
		int	*ndumped, 
		int	*nloaded, 
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent)
{
	/* local variables */
	char	*function_name = "mbedit_action_quit";
	int	status = MB_SUCCESS;

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

	/* reset info */
	info_set = MB_NO;

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
int mbedit_action_step(
		int	step, 
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	info_set = MB_NO;

	/* check if a file has been opened and there are data */
	if (file_open == MB_YES && nbuff > 0)
		{

		/* figure out if stepping is possible */
		old_id = current_id;
		new_id = current_id + step;
		if (new_id < 0)
			new_id = 0;
		if (new_id >= nbuff)
			new_id = nbuff - 1;

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = new_id;
		*icurrent = current_id;

		/* set the plotting list */
		if (*ngood > 0)
			{
			status = mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,
				    plt_size,sh_mode,sh_flggd,sh_time,nplt,MB_NO);
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
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
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
int mbedit_action_plot(
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_plot";
	int	status = MB_SUCCESS;

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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}
		
	/* clear the screen */
	mbedit_clear_screen();

	/* check if a file has been opened */
	if (file_open == MB_YES)
		{

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* set the plotting list */
		if (*ngood > 0)
			{
			status = mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,
					plt_size,sh_mode,sh_flggd,sh_time,nplt,MB_NO);
			}
		}

	/* if no file open set failure status */
	else
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
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
int mbedit_action_mouse_toggle(
		int	x_loc, 
		int	y_loc, 
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_mouse_toggle";
	int	status = MB_SUCCESS;
	int	zap_box, zap_ping;
	int	ix, iy, range, range_min;
	int	found;
	int	iping, jbeam;
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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
		{
		/* check if a zap box has been picked */
		zap_box = MB_NO;
		for (i=current_id;i<current_id+nplot;i++)
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
				plwd,exgr,xntrvl,yntrvl,
				plt_size,sh_mode,sh_flggd,sh_time,
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
		for (i=current_id;i<current_id+nplot;i++)
			{
			for (j=0;j<ping[i].beams_bath;j++)
				{
				if (ping[i].beamflag[j] != MB_FLAG_NULL)
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
			close enough to be toggled */
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
			if (esffile_open == MB_YES)
			    {
			    if (mb_beam_ok(ping[iping].beamflag[jbeam]))
				mb_ess_save(verbose, &esf,
				    ping[iping].time_d, 
				    jbeam + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
				    MBP_EDIT_FLAG, &error);
			    else if (ping[iping].beamflag[jbeam] != MB_FLAG_NULL)
				mb_ess_save(verbose, &esf,
				    ping[iping].time_d, 
				    jbeam + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
				    MBP_EDIT_UNFLAG, &error);
			    }
			
			/* apply edit */
			if (mb_beam_ok(ping[iping].beamflag[jbeam]))
			    ping[iping].beamflag[jbeam] = 
				MB_FLAG_FLAG + MB_FLAG_MANUAL;
			else if (ping[iping].beamflag[jbeam] 
				    != MB_FLAG_NULL)
			    ping[iping].beamflag[jbeam] = MB_FLAG_NONE;
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
		*ngood = nbuff;
		*icurrent = current_id;

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
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
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
int mbedit_action_mouse_pick(
	int	x_loc, 
	int	y_loc, 
	int	plwd, 
	int	exgr, 
	int	xntrvl, 
	int	yntrvl, 
	int	plt_size, 
	int	sh_mode, 
	int	sh_flggd, 
	int	sh_time,
	int	*nbuffer, 
	int	*ngood, 
	int	*icurrent, 
	int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_mouse_pick";
	int	status = MB_SUCCESS;
	int	zap_box, zap_ping;
	int	ix, iy, range, range_min;
	int	found;
	int	iping, jbeam;
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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
		{
		/* check if a zap box has been picked */
		zap_box = MB_NO;
		for (i=current_id;i<current_id+nplot;i++)
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
				plwd,exgr,xntrvl,yntrvl,
				plt_size,sh_mode,sh_flggd,sh_time,
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
		for (i=current_id;i<current_id+nplot;i++)
			{
			for (j=0;j<ping[i].beams_bath;j++)
				{
				if (mb_beam_ok(ping[i].beamflag[j]))
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
			if (esffile_open == MB_YES)
			    {
			    mb_ess_save(verbose, &esf,
				    ping[iping].time_d, 
				    jbeam + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
				    MBP_EDIT_FLAG, &error);
			    }
			
			/* apply edit */
			ping[iping].beamflag[jbeam] = 
				MB_FLAG_FLAG + MB_FLAG_MANUAL;
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
		*ngood = nbuff;
		*icurrent = current_id;

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
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
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
int mbedit_action_mouse_erase(
		int	x_loc, 
		int	y_loc, 
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_mouse_erase";
	int	status = MB_SUCCESS;
	int	zap_box, zap_ping;
	int	ix, iy, range;
	int	found;
	int	replot_label;
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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
	    {
	    /* check if a zap box has been picked */
	    zap_box = MB_NO;
	    for (i=current_id;i<current_id+nplot;i++)
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
		    
			/* if a zap box has been picked call zap routine */
			status = mbedit_action_zap_outbounds(zap_ping,
				plwd,exgr,xntrvl,yntrvl,plt_size,sh_mode,sh_flggd,sh_time,
				nbuffer,ngood,icurrent,nplt);
			}
		    }
		}
	    }

	/* do not look for beam erase unless file has been opened 
		and no zap box was picked */
	if (file_open == MB_YES && zap_box == MB_NO)
	  {

	  /* look for beams to be erased */
	  for (i=current_id;i<current_id+nplot;i++)
	    {
	    found = MB_NO;
	    replot_label = MB_NO;
	    for (j=0;j<ping[i].beams_bath;j++)
	      {
	      if (mb_beam_ok(ping[i].beamflag[j]))
		{
		ix = x_loc - ping[i].bath_x[j];
		iy = y_loc - ping[i].bath_y[j];
		range = (int) sqrt((double) (ix*ix + iy*iy));
		if (range < MBEDIT_ERASE_DISTANCE && *ngood > 0)
			{
			/* write edit to save file */
			if (esffile_open == MB_YES)
			    {
			    mb_ess_save(verbose, &esf,
			    	    ping[i].time_d, 
				    j + ping[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
				    MBP_EDIT_FLAG, &error);
			    }
			
	          	/* unplot the affected beam and ping */
			status = mbedit_unplot_ping(i);
			status = mbedit_unplot_beam(i,j);

			/* reset the beam value */
			if (mb_beam_ok(ping[i].beamflag[j]))
			ping[i].beamflag[j] = 
				MB_FLAG_FLAG + MB_FLAG_MANUAL;
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
	  *ngood = nbuff;
	  *icurrent = current_id;
	  }
		
	/* if no file open set failure status */
	else if (file_open == MB_NO)
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
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
int mbedit_action_mouse_restore(
		int	x_loc, 
		int	y_loc, 
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_mouse_restore";
	int	status = MB_SUCCESS;
	int	zap_box, zap_ping;
	int	ix, iy, range;
	int	found;
	int	replot_label;
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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
	    {
	    /* check if a zap box has been picked */
	    zap_box = MB_NO;
	    for (i=current_id;i<current_id+nplot;i++)
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

			/* if a zap box has been picked call zap routine */
			status = mbedit_action_zap_outbounds(zap_ping,
				plwd,exgr,xntrvl,yntrvl,plt_size,sh_mode,sh_flggd,sh_time,
				nbuffer,ngood,icurrent,nplt);
			}
		    }
		}
	    }

	/* do not look for beam restore unless file has been opened 
		and no zap box was picked */
	if (file_open == MB_YES && zap_box == MB_NO)
	  {

	  /* look for beams to be erased */
	  for (i=current_id;i<current_id+nplot;i++)
	    {
	    found = MB_NO;
	    replot_label = MB_NO;
	    for (j=0;j<ping[i].beams_bath;j++)
	      {
	      if (!mb_beam_ok(ping[i].beamflag[j])
		&& ping[i].beamflag[j] != MB_FLAG_NULL)
		{
		ix = x_loc - ping[i].bath_x[j];
		iy = y_loc - ping[i].bath_y[j];
		range = (int) sqrt((double) (ix*ix + iy*iy));
		if (range < MBEDIT_ERASE_DISTANCE && *ngood > 0)
			{
			/* write edit to save file */
			if (esffile_open == MB_YES)
			    {
			    mb_ess_save(verbose, &esf,
				    ping[i].time_d, 
				    j + ping[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
				    MBP_EDIT_UNFLAG, &error);
			    }
			
	          	/* unplot the affected beam and ping */
			if (found == MB_NO)
				status = mbedit_unplot_ping(i);
			status = mbedit_unplot_beam(i,j);

			/* reset the beam value */
			if (!mb_beam_ok(ping[i].beamflag[j])
			    && ping[i].beamflag[j] != MB_FLAG_NULL)
			    ping[i].beamflag[j] = MB_FLAG_NONE;
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
	  *ngood = nbuff;
	  *icurrent = current_id;
	  }
		
	/* if no file open set failure status */
	else if (file_open == MB_NO)
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
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
int mbedit_action_mouse_grab(
		int	grabmode,
		int	x_loc, 
		int	y_loc, 
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_mouse_grab";
	int	status = MB_SUCCESS;
	int	zap_box, zap_ping;
	int	xgmin, xgmax, ygmin, ygmax;
	int	found;
	int	replot_label;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       grabmode:    %d\n",grabmode);
		fprintf(stderr,"dbg2       x_loc:       %d\n",x_loc);
		fprintf(stderr,"dbg2       y_loc:       %d\n",y_loc);
		fprintf(stderr,"dbg2       plot_width:  %d\n",plwd);
		fprintf(stderr,"dbg2       exager:      %d\n",exgr);
		fprintf(stderr,"dbg2       x_interval:  %d\n",xntrvl);
		fprintf(stderr,"dbg2       y_interval:  %d\n",yntrvl);
		fprintf(stderr,"dbg2       plot_size:   %d\n",plt_size);
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
		{
		/* replot old info beam if needed */
		if (info_set == MB_YES)
			{
			status = mbedit_unplot_beam(info_ping,info_beam);
			status = mbedit_unplot_info();
			info_set = MB_NO;
			status = mbedit_plot_beam(info_ping,info_beam-1);
			status = mbedit_plot_beam(info_ping,info_beam);
			status = mbedit_plot_beam(info_ping,info_beam+1);
			status = mbedit_plot_ping(info_ping);
			}

		/* get start of grab rectangle */
		if (grabmode == MBEDIT_GRAB_START)
			{
			grab_set = MB_YES;
			grab_start_x = x_loc;
			grab_start_y = y_loc;
			grab_end_x = x_loc;
			grab_end_y = y_loc;

			/* get grab rectangle to use */
			if (grab_start_x <= grab_end_x)
				{
				xgmin = grab_start_x;
				xgmax = grab_end_x;
				}
			else
				{
				xgmin = grab_end_x;
				xgmax = grab_start_x;
				}
			if (grab_start_y <= grab_end_y)
				{
				ygmin = grab_start_y;
				ygmax = grab_end_y;
				}
			else
				{
				ygmin = grab_end_y;
				ygmax = grab_start_y;
				}

			/* draw grab ractangle */
			xg_drawrectangle(mbedit_xgid,
				xgmin,
				ygmin, 
				xgmax - xgmin, 
				ygmax - ygmin,
				pixel_values[RED],XG_SOLIDLINE);
			}

		/* change grab rectangle */
		else if (grabmode == MBEDIT_GRAB_MOVE)
			{
			/* get grab rectangle to use */
			if (grab_start_x <= grab_end_x)
				{
				xgmin = grab_start_x;
				xgmax = grab_end_x;
				}
			else
				{
				xgmin = grab_end_x;
				xgmax = grab_start_x;
				}
			if (grab_start_y <= grab_end_y)
				{
				ygmin = grab_start_y;
				ygmax = grab_end_y;
				}
			else
				{
				ygmin = grab_end_y;
				ygmax = grab_start_y;
				}

			/* undraw old grab rectangle */
			xg_drawrectangle(mbedit_xgid,
				xgmin,
				ygmin, 
				xgmax - xgmin, 
				ygmax - ygmin,
				pixel_values[WHITE],XG_SOLIDLINE);

			/* update grab rectangle */
			grab_set = MB_YES;
			grab_end_x = x_loc;
			grab_end_y = y_loc;

			/* get grab rectangle to use */
			if (grab_start_x <= grab_end_x)
				{
				xgmin = grab_start_x;
				xgmax = grab_end_x;
				}
			else
				{
				xgmin = grab_end_x;
				xgmax = grab_start_x;
				}
			if (grab_start_y <= grab_end_y)
				{
				ygmin = grab_start_y;
				ygmax = grab_end_y;
				}
			else
				{
				ygmin = grab_end_y;
				ygmax = grab_start_y;
				}

			/* draw grab rectangle */
			xg_drawrectangle(mbedit_xgid,
				xgmin,
				ygmin, 
				xgmax - xgmin, 
				ygmax - ygmin,
				pixel_values[RED],XG_SOLIDLINE);

			/* replot beams on bounds of the grab box */
			for (i=current_id;i<current_id+nplot;i++)
			    {
			    found = MB_NO;
			    replot_label = MB_NO;
			    for (j=0;j<ping[i].beams_bath;j++)
			      {
			      if (ping[i].beamflag[j] != MB_FLAG_NULL)
				{
				if (abs(ping[i].bath_x[j] - xgmin) <= 10
					|| abs(ping[i].bath_x[j] - xgmax) <= 10
					|| abs(ping[i].bath_y[j] - ygmin) <= 10
					|| abs(ping[i].bath_y[j] - ygmax) <= 10)
					{
					/* replot the affected beams */
		 			found = MB_YES;
					status = mbedit_plot_beam(i,j);

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
			}

		/* apply grab rectangle */
		else if (grabmode == MBEDIT_GRAB_END)
			{
			/* get final grab rectangle */
			grab_set = MB_NO;
			grab_end_x = x_loc;
			grab_end_y = y_loc;

			/* get grab rectangle to use */
			if (grab_start_x <= grab_end_x)
				{
				xgmin = grab_start_x;
				xgmax = grab_end_x;
				}
			else
				{
				xgmin = grab_end_x;
				xgmax = grab_start_x;
				}
			if (grab_start_y <= grab_end_y)
				{
				ygmin = grab_start_y;
				ygmax = grab_end_y;
				}
			else
				{
				ygmin = grab_end_y;
				ygmax = grab_start_y;
				}

			/* check if any zap boxes has been picked */
			zap_box = MB_NO;
			for (i=current_id;i<current_id+nplot;i++)
			    {
			    if (ping[i].outbounds == MBEDIT_OUTBOUNDS_UNFLAGGED)
				{
				if (xgmin <= ping[i].zap_x1
				    && xgmax >= ping[i].zap_x2
				    && ygmin <= ping[i].zap_y1
				    && ygmax >= ping[i].zap_y2)
				    {
				    zap_box = MB_YES;
				    zap_ping = i;

				    /* if a zap box has been picked call zap routine */
				    status = mbedit_action_zap_outbounds(zap_ping,
					    plwd,exgr,xntrvl,yntrvl,plt_size,sh_mode,sh_flggd,sh_time,
					    nbuffer,ngood,icurrent,nplt);
				    }
				}
			    }

			/* look for beams to be erased */
			for (i=current_id;i<current_id+nplot;i++)
			    {
			    found = MB_NO;
			    replot_label = MB_NO;
			    for (j=0;j<ping[i].beams_bath;j++)
			      {
			      if (mb_beam_ok(ping[i].beamflag[j]))
				{
				if (ping[i].bath_x[j] >= xgmin
					&& ping[i].bath_x[j] <= xgmax
					&& ping[i].bath_y[j] >= ygmin
					&& ping[i].bath_y[j] <= ygmax
					&& *ngood > 0)
					{
					/* write edit to save file */
					if (esffile_open == MB_YES)
					    {
					    mb_ess_save(verbose, &esf,
			    			    ping[i].time_d, 
						    j + ping[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
						    MBP_EDIT_FLAG, &error);
					    }

					/* reset the beam value */
					if (mb_beam_ok(ping[i].beamflag[j]))
					ping[i].beamflag[j] = 
						MB_FLAG_FLAG + MB_FLAG_MANUAL;
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

					/* if beam out of bounds replot label */
					if (ping[i].bath_x[j] < xmin
					    || ping[i].bath_x[j] > xmax
					    || ping[i].bath_y[j] < ymin
					    || ping[i].bath_y[j] > ymax)
					    replot_label = MB_YES;
					}
				}
			      }
			    }

			/* replot everything */
			status = mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,
				    plt_size,sh_mode,sh_flggd,sh_time,nplt,MB_NO);
			}

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;
		}
		
	/* if no file open set failure status */
	else if (file_open == MB_NO)
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
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
int mbedit_action_mouse_info(
		int	x_loc, 
		int	y_loc, 
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_mouse_info";
	int	status = MB_SUCCESS;
	int	ix, iy, range, range_min;
	int	iping, jbeam;
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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
		{
		/* replot old info beam if needed */
		if (info_set == MB_YES)
			{
			status = mbedit_unplot_beam(info_ping,info_beam);
			status = mbedit_unplot_info();
			info_set = MB_NO;
			status = mbedit_plot_beam(info_ping,info_beam-1);
			status = mbedit_plot_beam(info_ping,info_beam);
			status = mbedit_plot_beam(info_ping,info_beam+1);
			status = mbedit_plot_ping(info_ping);
			}

		/* check if a beam has been picked */
		iping = 0;
		jbeam = 0;
		range_min = 100000;
		for (i=current_id;i<current_id+nplot;i++)
			{
			for (j=0;j<ping[i].beams_bath;j++)
				{
				if (ping[i].beamflag[j] != MB_FLAG_NULL)
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
			close enough to be id'd */
		if (range_min <= MBEDIT_PICK_DISTANCE)
			{
			info_set = MB_YES;
			info_ping = iping;
			info_beam = jbeam;
			info_time_i[0] = ping[iping].time_i[0];
			info_time_i[1] = ping[iping].time_i[1];
			info_time_i[2] = ping[iping].time_i[2];
			info_time_i[3] = ping[iping].time_i[3];
			info_time_i[4] = ping[iping].time_i[4];
			info_time_i[5] = ping[iping].time_i[5];
			info_time_i[6] = ping[iping].time_i[6];
			info_time_d = ping[iping].time_d;
			info_navlon = ping[iping].navlon;
			info_navlat = ping[iping].navlat;
			info_speed = ping[iping].speed;
			info_heading = ping[iping].heading;
			info_altitude = ping[iping].altitude;
			info_beams_bath = ping[iping].beams_bath;
			info_beamflag = ping[iping].beamflag[jbeam];
			info_bath = ping[iping].bath[jbeam];
			info_bathacrosstrack = ping[iping].bathacrosstrack[jbeam];
			info_bathalongtrack = ping[iping].bathalongtrack[jbeam];
			info_detect = ping[iping].detect[jbeam];
			info_pulse = ping[iping].pulses[jbeam];
/*			fprintf(stderr,"\nping: %d beam:%d depth:%10.3f \n",
				iping,jbeam,ping[iping].bath[jbeam]);*/

			/* replot old info beam if needed */
			status = mbedit_plot_beam(info_ping,info_beam);
			status = mbedit_plot_info();

			}
		else
			info_set = MB_NO;

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;
		}
		
	/* if no file open set failure status */
	else if (file_open == MB_NO)
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
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
int mbedit_action_zap_outbounds(
		int	iping, 
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_zap_outbounds";
	int	status = MB_SUCCESS;
	int	found;
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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
	    {

	    /* look for beams to be erased */
	    found = MB_NO;
	    for (j=0;j<ping[iping].beams_bath;j++)
	      {
	      if (mb_beam_ok(ping[iping].beamflag[j])
		    && (ping[iping].bath_x[j] < xmin
			|| ping[iping].bath_x[j] > xmax
			|| ping[iping].bath_y[j] < ymin
			|| ping[iping].bath_y[j] > ymax))
		    {
		    /* write edit to save file */
		    if (esffile_open == MB_YES)
			{
			mb_ess_save(verbose, &esf,
				ping[iping].time_d, 
				j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
				MBP_EDIT_FLAG, &error);
			}
		    
		    /* unplot the affected beam and ping */
		    status = mbedit_unplot_ping(iping);
		    status = mbedit_unplot_beam(iping,j);

		    /* reset the beam value */
		    if (mb_beam_ok(ping[iping].beamflag[j]))
		    ping[iping].beamflag[j] = 
			    MB_FLAG_FLAG + MB_FLAG_MANUAL;
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
	   *ngood = nbuff;
	   *icurrent = current_id;
	   }
		
	/* if no file open set failure status */
	else if (file_open == MB_NO)
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
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
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_bad_ping";
	int	status = MB_SUCCESS;
	int	j;

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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* check if a file has been opened 
		and a beam has been picked and saved */
	if (file_open == MB_YES && beam_save == MB_YES)
		{
		/* write edits to save file */
		if (esffile_open == MB_YES)
		    {
		    for (j=0;j<ping[iping_save].beams_bath;j++)
			if (mb_beam_ok(ping[iping_save].beamflag[j]))
			    mb_ess_save(verbose, &esf,
				ping[iping_save].time_d, 
				j + ping[iping_save].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
				MBP_EDIT_FLAG, &error);
		    }

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (j=0;j<ping[iping_save].beams_bath;j++)
			status = mbedit_unplot_beam(iping_save,j);

		/* flag beams in bad ping */
		for (j=0;j<ping[iping_save].beams_bath;j++)
			if (mb_beam_ok(ping[iping_save].beamflag[j]))
				ping[iping_save].beamflag[j] = 
					MB_FLAG_FLAG + MB_FLAG_MANUAL;
		if (verbose >= 1)
			fprintf(stderr,"\nbeams in ping: %d flagged\n",
				iping_save);

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (j=0;j<ping[iping_save].beams_bath;j++)
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
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time, 
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_good_ping";
	int	status = MB_SUCCESS;
	int	j;

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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* check if a file has been opened 
		and a beam has been picked and saved */
	if (file_open == MB_YES && beam_save == MB_YES)
		{
		/* write edits to save file */
		if (esffile_open == MB_YES)
		    {
		    for (j=0;j<ping[iping_save].beams_bath;j++)
			if (!mb_beam_ok(ping[iping_save].beamflag[j])
			    && ping[iping_save].beamflag[j] != MB_FLAG_NULL)
			    mb_ess_save(verbose, &esf,
				ping[iping_save].time_d, 
				j + ping[iping_save].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
				MBP_EDIT_UNFLAG, &error);
		    }

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (j=0;j<ping[iping_save].beams_bath;j++)
			status = mbedit_unplot_beam(iping_save,j);

		/* flag beams in good ping */
		for (j=0;j<ping[iping_save].beams_bath;j++)
			if (!mb_beam_ok(ping[iping_save].beamflag[j])
			    && ping[iping_save].beamflag[j] != MB_FLAG_NULL)
				ping[iping_save].beamflag[j] = 
					MB_FLAG_NONE;
		if (verbose >= 1)
			fprintf(stderr,"\nbeams in ping: %d unflagged\n",
				iping_save);

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (j=0;j<ping[iping_save].beams_bath;j++)
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
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_left_ping";
	int	status = MB_SUCCESS;
	int	j;

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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* check if a file has been opened 
		and a beam has been picked and saved */
	if (file_open == MB_YES && beam_save == MB_YES)
		{
		/* write edits to save file */
		if (esffile_open == MB_YES)
		    {
		    for (j=0;j<=jbeam_save;j++)
			if (mb_beam_ok(ping[iping_save].beamflag[j]))
			    mb_ess_save(verbose, &esf,
				ping[iping_save].time_d, 
				j + ping[iping_save].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
				MBP_EDIT_FLAG, &error);
		    }

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (j=0;j<ping[iping_save].beams_bath;j++)
			status = mbedit_unplot_beam(iping_save,j);

		/* flag beams to left of picked beam */
		for (j=0;j<=jbeam_save;j++)
			if (mb_beam_ok(ping[iping_save].beamflag[j]))
				ping[iping_save].beamflag[j] = 
					MB_FLAG_FLAG + MB_FLAG_MANUAL;
		if (verbose >= 1)
			fprintf(stderr,"\nbeams in ping: %d left of beam: %d flagged\n",
				iping_save,jbeam_save);

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (j=0;j<ping[iping_save].beams_bath;j++)
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
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_right_ping";
	int	status = MB_SUCCESS;
	int	j;

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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* check if a file has been opened 
		and a beam has been picked and saved */
	if (file_open == MB_YES && beam_save == MB_YES)
		{
		/* write edits to save file */
		if (esffile_open == MB_YES)
		    {
		    for (j=jbeam_save;j<ping[iping_save].beams_bath;j++)
			if (mb_beam_ok(ping[iping_save].beamflag[j]))
			    mb_ess_save(verbose, &esf,
				ping[iping_save].time_d, 
				j + ping[iping_save].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
				MBP_EDIT_FLAG, &error);
		    }

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (j=0;j<ping[iping_save].beams_bath;j++)
			status = mbedit_unplot_beam(iping_save,j);

		/* flag beams to right of picked beam */
		for (j=jbeam_save;j<ping[iping_save].beams_bath;j++)
			if (mb_beam_ok(ping[iping_save].beamflag[j]))
				ping[iping_save].beamflag[j] = 
					MB_FLAG_FLAG + MB_FLAG_MANUAL;
		if (verbose >= 1)
			fprintf(stderr,"\nbeams in ping: %d right of beam: %d flagged\n",
				iping_save,jbeam_save);

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (j=0;j<ping[iping_save].beams_bath;j++)
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
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_zero_ping";
	int	status = MB_SUCCESS;
	int	j;

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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* check if a file has been opened 
		and a beam has been picked and saved */
	if (file_open == MB_YES && beam_save == MB_YES)
		{
		/* write edits to save file */
		if (esffile_open == MB_YES)
		    {
		    for (j=0;j<ping[iping_save].beams_bath;j++)
		    	{
			if (ping[iping_save].beamflag[j] != MB_FLAG_NULL)
			    mb_ess_save(verbose, &esf,
				ping[iping_save].time_d, 
				j + ping[iping_save].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
				MBP_EDIT_ZERO, &error);
			}
		    }

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (j=0;j<ping[iping_save].beams_bath;j++)
			status = mbedit_unplot_beam(iping_save,j);

		/* null beams in bad ping */
		for (j=0;j<ping[iping_save].beams_bath;j++)
			{
			ping[iping_save].beamflag[j] = MB_FLAG_NULL;
			}
		if (verbose >= 1)
			fprintf(stderr,"\nbeams in ping: %d nulled\n",
				iping_save);

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (j=0;j<ping[iping_save].beams_bath;j++)
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
int mbedit_action_flag_view(
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_flag_view";
	int	status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
		{
		/* flag all unflagged beams */
		for (i=current_id;i<current_id+nplot;i++)
			{
			for (j=0;j<ping[i].beams_bath;j++)
			    {
			    if (mb_beam_ok(ping[i].beamflag[j]))
				    {
				    /* write edit to save file */
				    if (esffile_open == MB_YES)
					mb_ess_save(verbose, &esf,
						ping[i].time_d, 
						j + ping[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
						MBP_EDIT_FLAG, &error);
		    
				    /* apply edit */
				    ping[i].beamflag[j] =  MB_FLAG_FLAG + MB_FLAG_MANUAL;
				    if (verbose >= 1)
					{
					fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
						i,j,ping[i].bath[j]);
					fprintf(stderr," flagged\n");
					}
				    beam_save = MB_YES;
				    iping_save = i;
				    jbeam_save = j;
				    }
			    }
			}


		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* clear the screen */
		status = mbedit_clear_screen();
	
		/* set up plotting */
		if (*ngood > 0)
			{
			status = mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,
					plt_size,sh_mode,sh_flggd,sh_time,nplt, MB_NO);
			}
		}
		
	/* if no file open set failure status */
	else if (file_open == MB_NO)
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
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
int mbedit_action_unflag_view(
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_unflag_view";
	int	status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
		{
		/* unflag all flagged beams */
		for (i=current_id;i<current_id+nplot;i++)
			{
			for (j=0;j<ping[i].beams_bath;j++)
			    {
			    if (!mb_beam_ok(ping[i].beamflag[j])
				&& ping[i].beamflag[j] != MB_FLAG_NULL)
				    {
				    /* write edit to save file */
				    if (esffile_open == MB_YES)
					mb_ess_save(verbose, &esf,
						ping[i].time_d, 
						j + ping[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
						MBP_EDIT_UNFLAG, &error);
		    
				    /* apply edit */
				    ping[i].beamflag[j] =  MB_FLAG_NONE;
				    if (verbose >= 1)
					{
					fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
						i,j,ping[i].bath[j]);
					fprintf(stderr," unflagged\n");
					}
				    beam_save = MB_YES;
				    iping_save = i;
				    jbeam_save = j;
				    }
			    }
			}


		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* clear the screen */
		status = mbedit_clear_screen();
	
		/* set up plotting */
		if (*ngood > 0)
			{
			status = mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,
					plt_size,sh_mode,sh_flggd,sh_time,nplt, MB_NO);
			}
		}
		
	/* if no file open set failure status */
	else if (file_open == MB_NO)
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
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
int mbedit_action_unflag_all(
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_unflag_all";
	int	status = MB_SUCCESS;
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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
		{
		/* unflag all flagged beams from current point in buffer */
		for (i=current_id;i<nbuff;i++)
		    {
		    for (j=0;j<ping[i].beams_bath;j++)
			{
			if (!mb_beam_ok(ping[i].beamflag[j])
			    && ping[i].beamflag[j] != MB_FLAG_NULL)
			    {
			    /* write edit to save file */
			    if (esffile_open == MB_YES)
				mb_ess_save(verbose, &esf,
					ping[i].time_d, 
					j + ping[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, 
					MBP_EDIT_UNFLAG, &error);
	    
			    /* apply edit */
			    ping[i].beamflag[j] =  MB_FLAG_NONE;
			    if (verbose >= 1)
				{
				fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
					i,j,ping[i].bath[j]);
				fprintf(stderr," unflagged\n");
				}
			    beam_save = MB_NO;
			    }
			}
		    }


		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* clear the screen */
		status = mbedit_clear_screen();
	
		/* set up plotting */
		if (*ngood > 0)
			{
			status = mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,
					plt_size,sh_mode,sh_flggd,sh_time,nplt, MB_NO);
			}
		}
		
	/* if no file open set failure status */
	else if (file_open == MB_NO)
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
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
int mbedit_action_filter_all(
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_filter_all";
	int	status = MB_SUCCESS;
	char	string[MB_PATH_MAXLINE];
	int	i;

	/* print input debug statements */
fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
function_name);
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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* do nothing unless file has been opened */
	if (file_open == MB_YES)
		{
		do_message_on("MBedit is applying bathymetry filters...");

		/* filter all pings in buffer */
		for (i=current_id;i<nbuff;i++)
		    {
		    mbedit_filter_ping(i);
			
		    /* update message every 250 records */
		    if (i % 250 == 0)
			{
			sprintf(string, "MBedit: filters applied to %d of %d records so far...", 
				i, nbuff - current_id - 1);
			do_message_on(string);
			}
		    }

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* clear the screen */
		status = mbedit_clear_screen();
	
		/* set up plotting */
		do_message_off();
		if (*ngood > 0)
			{
			status = mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,
					plt_size,sh_mode,sh_flggd,sh_time,nplt, MB_NO);
			}
		}
		
	/* if no file open set failure status */
	else if (file_open == MB_NO)
		{
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
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
int mbedit_filter_ping(int iping)
{
	/* local variables */
	char	*function_name = "mbedit_filter_ping";
	int	status = MB_SUCCESS;
	int	nbathsum, nbathlist;
	double	bathsum, bathmedian;
	int	start, end;
	double	angle;
	int	istart, iend, jstart, jend, jbeam;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iping:       %d\n",iping);
		}

	/* reset info */
	if (info_set == MB_YES)
		{
		status = mbedit_unplot_beam(info_ping,info_beam);
		status = mbedit_unplot_info();
		info_set = MB_NO;
		status = mbedit_plot_beam(info_ping,info_beam-1);
		status = mbedit_plot_beam(info_ping,info_beam);
		status = mbedit_plot_beam(info_ping,info_beam+1);
		status = mbedit_plot_ping(info_ping);
		}

	/* do nothing unless file has been opened and filters set on */
	if (file_open == MB_YES
		&& iping >= 0 && iping < nbuff)
		{
		/* work on good data */
		if (status == MB_SUCCESS)
		    {
		    /* clear previous filter flags */
		    for (j=0;j<ping[iping].beams_bath;j++)
		    	{
		    	if (mb_beam_check_flag_filter2(ping[iping].beamflag[j]))
		    		{
			    	/* write edit to save file */
			    	if (esffile_open == MB_YES)
				    mb_ess_save(verbose, &esf,
						ping[iping].time_d, 
						j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
						MBP_EDIT_UNFLAG, &error);
	
			    	/* apply edit */
				ping[iping].beamflag[j] = MB_FLAG_NONE;
			    	if (verbose >= 1)
				    {
				    fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
						iping,j,ping[iping].bath[j]);
				    fprintf(stderr," unflagged\n");
				    }
		    		}
		    	}
		
		    /* apply median filter if desired */
		    if (filter_medianspike == MB_YES)
		    	{
			/* loop over all beams in the ping */
		    	for (jbeam=0;jbeam<ping[iping].beams_bath;jbeam++)
			    {
			    /* calculate median if beam not flagged */
			    if (mb_beam_ok(ping[iping].beamflag[jbeam]))
				{
		    		nbathlist = 0;
		    		nbathsum = 0;
		    		bathsum = 0.0;
		    		bathmedian = 0.0;
				istart = MAX(iping - filter_medianspike_ltrack / 2, 0);
				iend = MIN(iping + filter_medianspike_ltrack / 2, nbuff - 1);
				for (i=istart;i<=iend;i++)
				    {
				    jstart = MAX(jbeam - filter_medianspike_xtrack / 2, 0);
				    jend = MIN(jbeam + filter_medianspike_xtrack / 2, ping[iping].beams_bath - 1);
				    for (j=jstart;j<=jend;j++)
					{
			    		if (mb_beam_ok(ping[i].beamflag[j]))
					    {
					    bathsum += ping[i].bath[j];
				    	    nbathsum++;
				    	    bathlist[nbathlist] = ping[i].bath[j];
					    nbathlist++;
					    }
					}
				    }
				if (nbathlist > 0)
				    {
				    qsort((char *)bathlist,nbathlist,sizeof(double),(void *)mb_double_compare);
				    bathmedian = bathlist[nbathlist/2];
		 		    }
			    	if (100 * fabs(ping[iping].bath[jbeam] - bathmedian) / ping[iping].altitude
			    	        > filter_medianspike_threshold)
				    {
			    	    /* write edit to save file */
			    	    if (esffile_open == MB_YES)
					mb_ess_save(verbose, &esf,
						ping[iping].time_d, 
						jbeam + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
						MBP_EDIT_FILTER, &error);
	
			    	    /* apply edit */
				    ping[iping].beamflag[jbeam] = MB_FLAG_FILTER2 + MB_FLAG_FLAG;
			    	    if (verbose >= 1)
				 	{
					fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
						iping,jbeam,ping[iping].bath[jbeam]);
					fprintf(stderr," flagged\n");
					}
				    }
				}
			    }
			}

		
		    /* apply wrongside filter if desired */
		    if (filter_wrongside == MB_YES)
		    	{
		    	start = 0;
		    	end = (ping[iping].beams_bath / 2) - filter_wrongside_threshold;
		 	for (j=start;j<end;j++)
		 	    {
		 	    if (mb_beam_ok(ping[iping].beamflag[j])
		 	    	&& ping[iping].bathacrosstrack[j] > 0.0)
		 	    	{
			   	/* write edit to save file */
			        if (esffile_open == MB_YES)
					mb_ess_save(verbose, &esf,
						ping[iping].time_d, 
						j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
						MBP_EDIT_FILTER, &error);
	
			        /* apply edit */
			    	ping[iping].beamflag[j] = MB_FLAG_FILTER2 + MB_FLAG_FLAG;
			       	if (verbose >= 1)
				 	{
					fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
						iping,j,ping[iping].bath[j]);
					fprintf(stderr," flagged\n");
					}
		 	    	}
		 	    }
		    	start = (ping[iping].beams_bath / 2) + filter_wrongside_threshold;
		    	end = ping[iping].beams_bath;
		 	for (j=start;j<end;j++)
		 	    {
		 	    if (mb_beam_ok(ping[iping].beamflag[j])
		 	    	&& ping[iping].bathacrosstrack[j] < 0.0)
		 	    	{
			   	/* write edit to save file */
			        if (esffile_open == MB_YES)
					mb_ess_save(verbose, &esf,
						ping[iping].time_d, 
						j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
						MBP_EDIT_FILTER, &error);
	
			        /* apply edit */
			    	ping[iping].beamflag[j] = MB_FLAG_FILTER2 + MB_FLAG_FLAG;
			       	if (verbose >= 1)
				 	{
					fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
						iping,j,ping[iping].bath[j]);
					fprintf(stderr," flagged\n");
					}
		 	    	}
		 	    }
			}
		
		    /* apply cut by beam number filter if desired */
		    if (filter_cutbeam == MB_YES)
		    	{
			/* handle cut inside swath */
		 	if (filter_cutbeam_begin <= filter_cutbeam_end)
		 	    {
		    	    start = MAX(filter_cutbeam_begin, 0);
		    	    end = MIN(filter_cutbeam_end, ping[iping].beams_bath - 1);
		    	    for (j=start;j<end;j++)
			    	{
			    	if (mb_beam_ok(ping[iping].beamflag[j]))
				    {
			    	    /* write edit to save file */
			    	    if (esffile_open == MB_YES)
					mb_ess_save(verbose, &esf,
						ping[iping].time_d, 
						j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
						MBP_EDIT_FILTER, &error);
	
			    	    /* apply edit */
				    ping[iping].beamflag[j] = MB_FLAG_FILTER2 + MB_FLAG_FLAG;
			    	    if (verbose >= 1)
				 	{
					fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
						iping,j,ping[iping].bath[j]);
					fprintf(stderr," flagged\n");
					}
				    }
			    	}
			    }

			/* handle cut at edges of swath */
		 	else if (filter_cutbeam_begin > filter_cutbeam_end)
		 	    {
		    	    for (j=0;j<ping[iping].beams_bath;j++)
			    	{
			    	if ((j <= filter_cutbeam_end || j >= filter_cutbeam_begin)
				    && mb_beam_ok(ping[iping].beamflag[j]))
				    {
			    	    /* write edit to save file */
			    	    if (esffile_open == MB_YES)
					mb_ess_save(verbose, &esf,
						ping[iping].time_d, 
						j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
						MBP_EDIT_FILTER, &error);
	
			    	    /* apply edit */
				    ping[iping].beamflag[j] = MB_FLAG_FILTER2 + MB_FLAG_FLAG;
			    	    if (verbose >= 1)
				 	{
					fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
						iping,j,ping[iping].bath[j]);
					fprintf(stderr," flagged\n");
					}
				    }
			    	}
			    }
			}
		
		    /* apply cut by distance filter if desired */
		    if (filter_cutdistance == MB_YES)
		    	{
			/* handle cut inside swath */
		 	if (filter_cutdistance_begin <= filter_cutdistance_end)
		 	    {
		    	    for (j=0;j<ping[iping].beams_bath;j++)
			    	{
			    	if (mb_beam_ok(ping[iping].beamflag[j]))
				    {
				    if (ping[iping].bathacrosstrack[j] >= filter_cutdistance_begin
				    	&& ping[iping].bathacrosstrack[j] <= filter_cutdistance_end)
				    	{
			    	    	/* write edit to save file */
			    	    	if (esffile_open == MB_YES)
						mb_ess_save(verbose, &esf,
							ping[iping].time_d, 
							j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
							MBP_EDIT_FILTER, &error);
	
			    	    	/* apply edit */
				    	ping[iping].beamflag[j] = MB_FLAG_FILTER2 + MB_FLAG_FLAG;
			    	    	if (verbose >= 1)
				 		{
						fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
							iping,j,ping[iping].bath[j]);
						fprintf(stderr," flagged\n");
						}
				  	}
				    }
			    	}
			    }

			/* handle cut at edges of swath */
		 	else if (filter_cutdistance_begin > filter_cutdistance_end)
		 	    {
		    	    for (j=0;j<ping[iping].beams_bath;j++)
			    	{
			    	if (mb_beam_ok(ping[iping].beamflag[j]))
				    {
				    if (ping[iping].bathacrosstrack[j] >= filter_cutdistance_begin
					|| ping[iping].bathacrosstrack[j] <= filter_cutdistance_end)
				    	{
			    	    	/* write edit to save file */
			    	    	if (esffile_open == MB_YES)
						mb_ess_save(verbose, &esf,
							ping[iping].time_d, 
							j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
							MBP_EDIT_FILTER, &error);
	
			    	    	/* apply edit */
				    	ping[iping].beamflag[j] = MB_FLAG_FILTER2 + MB_FLAG_FLAG;
			    	    	if (verbose >= 1)
				 		{
						fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
							iping,j,ping[iping].bath[j]);
						fprintf(stderr," flagged\n");
						}
					}
				    }
			    	}
			    }
			}
		
		    /* apply cut by angle filter if desired */
		    if (filter_cutangle == MB_YES)
		    	{
			/* handle cut inside swath */
		 	if (filter_cutangle_begin <= filter_cutangle_end)
		 	    {
		    	    for (j=0;j<ping[iping].beams_bath;j++)
			    	{
			    	if (mb_beam_ok(ping[iping].beamflag[j])
				    && ping[iping].altitude > 0.0)
				    {
				    angle = RTD * atan(ping[iping].bathacrosstrack[j] 
							/ ping[iping].altitude);
				    if (angle >= filter_cutangle_begin
				    	&& angle <= filter_cutangle_end)
				    	{
			    	    	/* write edit to save file */
			    	    	if (esffile_open == MB_YES)
						mb_ess_save(verbose, &esf,
							ping[iping].time_d, 
							j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
							MBP_EDIT_FILTER, &error);
	
			    	    	/* apply edit */
				    	ping[iping].beamflag[j] = MB_FLAG_FILTER2 + MB_FLAG_FLAG;
			    	    	if (verbose >= 1)
				 		{
						fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
							iping,j,ping[iping].bath[j]);
						fprintf(stderr," flagged\n");
						}
				  	}
				    }
			    	}
			    }

			/* handle cut at edges of swath */
		 	else if (filter_cutangle_begin > filter_cutangle_end)
		 	    {
		    	    for (j=0;j<ping[iping].beams_bath;j++)
			    	{
			    	if (mb_beam_ok(ping[iping].beamflag[j])
				    && ping[iping].altitude > 0.0)
				    {
				    angle = RTD * atan(ping[iping].bathacrosstrack[j] 
							/ ping[iping].altitude);
				    if (angle >= filter_cutangle_begin
					|| angle <= filter_cutangle_end)
				    	{
			    	    	/* write edit to save file */
			    	    	if (esffile_open == MB_YES)
						mb_ess_save(verbose, &esf,
							ping[iping].time_d, 
							j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
							MBP_EDIT_FILTER, &error);
	
			    	    	/* apply edit */
				    	ping[iping].beamflag[j] = MB_FLAG_FILTER2 + MB_FLAG_FLAG;
			    	    	if (verbose >= 1)
				 		{
						fprintf(stderr,"\nping: %d beam:%d depth:%10.3f ",
							iping,j,ping[iping].bath[j]);
						fprintf(stderr," flagged\n");
						}
					}
				    }
			    	}
			    }
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
int mbedit_get_format(char *file, int *form)
{
	/* local variables */
	char	*function_name = "mbedit_get_format";
	int	status = MB_SUCCESS;
	char	tmp[MB_PATH_MAXLINE];
	int	tform;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       file:        %s\n",file);
		fprintf(stderr,"dbg2       format:      %d\n",*form);
		}

	/* get filenames */
	/* look for MB suffix convention */
	if ((status = mb_get_format(verbose, file, tmp, 
				    &tform, &error))
				    == MB_SUCCESS)
	    {
	    *form = tform;
	    }		

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       format:      %d\n",*form);
		fprintf(stderr,"dbg2       error:      %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_open_file(char *file, int form, int savemode)
{
	/* local variables */
	char	*function_name = "mbedit_open_file";
	int	status = MB_SUCCESS;
	int	outputmode;
	int	i;
	mb_path	error1;
	mb_path	error2;
	mb_path	error3;
	
	/* swath file locking variables */
	int	lock_status;
	int	locked;
	int	lock_purpose;
	mb_path	lock_program;
	mb_path lock_cpu;
	mb_path lock_user;
	char	lock_date[25];

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

	/* reset message */
	do_message_on("MBedit is opening a data file...");	

	/* get filenames */
	strcpy(ifile,file);
	format = form;

	/* try to lock file */
	if (uselockfiles == MB_YES)
		{
		status = mb_pr_lockswathfile(verbose, ifile, 
				MBP_LOCK_EDITBATHY, program_name, &error);
		}
	else
		{
		lock_status = mb_pr_lockinfo(verbose, ifile, &locked,
				&lock_purpose, lock_program, lock_user, lock_cpu, 
				lock_date, &error);

		/* if locked get lock info */
		if (error == MB_ERROR_FILE_LOCKED)
			{
			fprintf(stderr, "\nFile %s locked but lock ignored\n", ifile);
			fprintf(stderr, "File locked by <%s> running <%s>\n", lock_user, lock_program);
			fprintf(stderr, "on cpu <%s> at <%s>\n", lock_cpu, lock_date);
			error = MB_ERROR_NO_ERROR;
			}
		}
		
	/* if locked let the user know file can't be opened */
	if (status == MB_FAILURE)
		{	
		/* turn off message */
		do_message_off();
	
		/* if locked get lock info */
		if (error == MB_ERROR_FILE_LOCKED)
			{
			lock_status = mb_pr_lockinfo(verbose, ifile, &locked,
					&lock_purpose, lock_program, lock_user, lock_cpu, 
					lock_date, &error);

			sprintf(error1, "Unable to open input file:");
			sprintf(error2, "File locked by <%s> running <%s>", lock_user, lock_program);
			sprintf(error3, "on cpu <%s> at <%s>", lock_cpu, lock_date);
			fprintf(stderr, "\nUnable to open input file:\n");
			fprintf(stderr, "  %s\n", ifile);
			fprintf(stderr, "File locked by <%s> running <%s>\n", lock_user, lock_program);
			fprintf(stderr, "on cpu <%s> at <%s>\n", lock_cpu, lock_date);
			}

		/* else if unable to create lock file there is a permissions problem */
		else if (error == MB_ERROR_OPEN_FAIL)
			{
			sprintf(error1, "Unable to create lock file");
			sprintf(error2, "for intended input file:");
			sprintf(error3, "-Likely permissions issue");
			fprintf(stderr, "Unable to create lock file\n");
			fprintf(stderr, "for intended input file:\n");
			fprintf(stderr, "  %s\n", ifile);
			fprintf(stderr, "-Likely permissions issue\n");
			}

		/* put up error dialog */
		do_error_dialog(error1,error2, error3);
		}
		
	/* if successfully locked (or lock ignored) proceed */
	if (status == MB_SUCCESS)
		{
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

		/* allocate memory for data arrays */
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ssalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(int), (void **)&detect, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(int), (void **)&pulses, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(int), (void **)&editcount, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							MBEDIT_MAX_PINGS*sizeof(double), (void **)&bathlist, &error);
		for (i=0;i<MBEDIT_BUFFER_SIZE;i++)
			{
			ping[i].allocated = 0;
			ping[i].beamflag = NULL;
			ping[i].bath = NULL;
			ping[i].bathacrosstrack = NULL;
			ping[i].bathalongtrack = NULL;
			ping[i].detect = NULL;
			ping[i].pulses = NULL;
			ping[i].bath_x = NULL;
			ping[i].bath_y = NULL;
			}

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
		nbuff = 0;
		}
	
	/* if success so far deal with edit save files */
	if (status == MB_SUCCESS)
		{
		/* reset message */
		if (savemode == MB_YES)
			{
			sprintf(notice, "MBedit is sorting %d old edits...", esf.nedit);
			do_message_on(notice);
			}

		/* handle esf edits */
		if (output_mode != MBEDIT_OUTPUT_BROWSE)
			outputmode = MB_YES;
		else
			outputmode = MB_NO;
		if (savemode == MB_YES || outputmode == MB_YES)
			{
			status = mb_esf_load(verbose, ifile, 
					savemode, outputmode, esffile, &esf, &error);
			if (output_mode != MBEDIT_OUTPUT_BROWSE
				&& status == MB_SUCCESS
				&& esf.esffp != NULL)
				esffile_open = MB_YES;
			if (status == MB_FAILURE 
				&& error == MB_ERROR_OPEN_FAIL)
				{
				esffile_open = MB_NO;
				fprintf(stderr, "\nUnable to open new edit save file %s\n", 
				    esf.esffile);
				do_error_dialog("Unable to open new edit save file.", 
						"You may not have write", 
						"permission in this directory!");
				}
			else if (status == MB_FAILURE 
				&& error == MB_ERROR_MEMORY_FAIL)
				{
				esffile_open = MB_NO;
				fprintf(stderr, "\nUnable to allocate memory for edits in esf file %s\n", esf.esffile);
				do_error_dialog("Unable to allocate memory for.", 
						"edits in existing edit", 
						"save file!");
				}
			}
		}

	/* deal with success */
	if (status == MB_SUCCESS)
		{
		file_open = MB_YES;
		if (verbose >= 0)
			{
			fprintf(stderr,"\nMultibeam File <%s> initialized for reading\n",ifile);
			fprintf(stderr,"Multibeam Data Format ID: %d\n",format);
			}
		}
	else
		{
		file_open = MB_NO;
		if (verbose >= 0)
			{
			fprintf(stderr,"\nERROR: Multibeam File <%s> NOT initialized for reading\n",ifile);
			fprintf(stderr,"Multibeam Data Format ID: %d\n",format);
			}
		}
	
	/* turn off message */
	do_message_off();

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

verbose = 0;
	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_close_file()
{
	/* local variables */
	char	*function_name = "mbedit_close_file";
	int	status = MB_SUCCESS;
	char	command[MB_PATH_MAXLINE];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* reset message */
	do_message_on("MBedit is closing a data file...");	

	/* deallocate memory for data arrays */
	for (i=0;i<MBEDIT_BUFFER_SIZE;i++)
		{
		if (ping[i].allocated > 0)
		    {
		    ping[i].allocated = 0;
		    free(ping[i].beamflag);
		    free(ping[i].bath);
		    free(ping[i].bathacrosstrack);
		    free(ping[i].bathalongtrack);
		    free(ping[i].detect);
		    free(ping[i].pulses);
		    free(ping[i].bath_x);
		    free(ping[i].bath_y);

		    /* reset message */
		    if (i%250 == 0)
			{
			sprintf(notice, "MBedit: %d pings deallocated...", i);
			do_message_on(notice);	
			}
		    }
		}

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* close the files */
	status = mb_close(verbose,&imbio_ptr,&error);
	if (esf.nedit > 0 || esf.esffp != NULL)
	    {
	    status = mb_esf_close(verbose, &esf, &error);
	    }
	
	/* unlock the raw swath file */
	if (uselockfiles == MB_YES)
		status = mb_pr_unlockswathfile(verbose, ifile, 
						MBP_LOCK_EDITBATHY, program_name, &error);
						
	/* set mbprocess parameters */
	if (output_mode == MBEDIT_OUTPUT_EDIT)
	    {
	    /* update mbprocess parameter file */
	    status = mb_pr_update_format(verbose, ifile, 
			MB_YES, format, 
			&error);
	    status = mb_pr_update_edit(verbose, ifile, 
			MBP_EDIT_ON, esf.esffile, 
			&error);
			
	    /* run mbprocess if desired */
	    if (run_mbprocess == MB_YES)
		    {
		    /* turn message on */
		    do_message_on("Bathymetry edits being applied using mbprocess...");
		    
		    /* run mbprocess */
		    sprintf(command, "mbprocess -I %s\n",ifile);
		    system(command);
		    }
	    }

	/* if we got here we must have succeeded */
	if (verbose >= 0)
		{
		fprintf(stderr,"\nMultibeam Input File <%s> closed\n",ifile);
		fprintf(stderr,"%d data records loaded\n",nload_total);
		fprintf(stderr,"%d data records dumped\n",ndump_total);
		
		}
	file_open = MB_NO;
	nload_total = 0;
	ndump_total = 0;
	
	/* turn file button on */
	do_filebutton_on();
	do_nextbutton_off();
	
	/* turn off message */
	do_message_off();

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
int mbedit_dump_data(int hold_size, int *ndumped, int *nbuffer)
{
	/* local variables */
	char	*function_name = "mbedit_dump_data";
	int	status = MB_SUCCESS;
	int	action;
	int	iping, jbeam;

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
		/* turn message on */
		do_message_on("MBedit is clearing data...");

		/* output changed edits in pings to be dumped */
		for (iping=0;iping<nbuff-hold_size;iping++)
			{
			for (jbeam=0;jbeam<ping[iping].beams_bath;jbeam++)
			    {
			    if (ping[iping].beamflag[jbeam] != ping[iping].beamflagorg[jbeam])
			    	{
				if (mb_beam_ok(ping[iping].beamflag[jbeam]))
					action = MBP_EDIT_UNFLAG;
				else if (mb_beam_check_flag_filter2(ping[iping].beamflag[jbeam]))
					action = MBP_EDIT_FILTER;
				else if (mb_beam_check_flag_filter(ping[iping].beamflag[jbeam]))
					action = MBP_EDIT_FILTER;
				else if (ping[iping].beamflag[jbeam] != MB_FLAG_NULL)
					action = MBP_EDIT_FLAG;
				else
					action = MBP_EDIT_ZERO;
				mb_esf_save(verbose, &esf,
						ping[iping].time_d, 
						jbeam + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
						action, &error);
				}
			    }
			}

		/* deallocate pings to be dumped */
		for (iping=0;iping<nbuff-hold_size;iping++)
			{
			if (ping[iping].allocated > 0)
			    {
			    ping[iping].allocated = 0;
			    free(ping[iping].beamflag);
			    free(ping[iping].beamflagorg);
			    free(ping[iping].bath);
			    free(ping[iping].bathacrosstrack);
			    free(ping[iping].bathalongtrack);
			    free(ping[iping].detect);
			    free(ping[iping].pulses);
			    free(ping[iping].bath_x);
			    free(ping[iping].bath_y);
			    }
			}

		/* copy data to be held */
		for (iping=0;iping<hold_size;iping++)
			{
			ping[iping] = ping[iping+nbuff-hold_size];
			}
		ndump = nbuff - hold_size;
		nbuff = hold_size;

		/* turn message off */
		do_message_off();
		}
	*ndumped = ndump;
	ndump_total += ndump;

	/* reset current data pointer */
	if (ndump > 0)
		current_id = current_id - ndump;
	if (current_id < 0)
		current_id = 0;
	if (current_id > nbuff - 1)
		current_id = nbuff - 1;
	*nbuffer = nbuff;

	/* print out information */
	if (verbose >= 2)
		{
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
int mbedit_load_data(int buffer_size, 
		int *nloaded, int *nbuffer, int *ngood, int *icurrent)
{
	/* local variables */
	char	*function_name = "mbedit_load_data";
	int	status = MB_SUCCESS;
	int	namp, nss;
	char	string[MB_PATH_MAXLINE];
	int	detect_status, detect_error, nbeams;
	double	speed_nav;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       buffer_size: %d\n",buffer_size);
		}
		
	/* turn message on */
	nload = 0;
	sprintf(string, "MBedit: %d records loaded so far...", nload);
	do_message_on(string);

	/* load data */
	do
		{
		error = MB_ERROR_NO_ERROR;
		status = mb_get_all(verbose,imbio_ptr,&store_ptr,&kind,
				ping[nbuff].time_i,
				&ping[nbuff].time_d,
				&ping[nbuff].navlon,
				&ping[nbuff].navlat,
				&ping[nbuff].speed,
				&ping[nbuff].heading,
				&distance,&ping[nbuff].altitude,&ping[nbuff].sonardepth,
				&ping[nbuff].beams_bath,&namp,&nss,
				beamflag,bath,amp, 
				bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,&error);
		if (error <= MB_ERROR_NO_ERROR
		    && kind == MB_DATA_DATA)
		    	{
			if (nbuff > 0)
				ping[nbuff].time_interval =
					ping[nbuff].time_d
						- ping[nbuff-1].time_d;
			status = mb_extract_nav(verbose,imbio_ptr,
						store_ptr,&kind,
						ping[nbuff].time_i,
						&ping[nbuff].time_d,
						&ping[nbuff].navlon,
						&ping[nbuff].navlat,
						&speed_nav,
						&ping[nbuff].heading,
						&draft, 
						&ping[nbuff].roll,
						&ping[nbuff].pitch,
						&ping[nbuff].heave, 
						&error);
			if (nbuff > 0 && ping[nbuff].time_d == ping[nbuff-1].time_d)
				{
				ping[nbuff].multiplicity = ping[nbuff-1].multiplicity + 1;
				}
			else
				{
				ping[nbuff].multiplicity = 0;
				}
			if (nbuff == 0)
				ping[nbuff].distance = 0.0;
			else
				ping[nbuff].distance = ping[nbuff-1].distance 
							+ ping[nbuff].speed * ping[nbuff].time_interval / 3.6;
			nbeams = ping[nbuff].beams_bath;
			detect_status = mb_detects(verbose,imbio_ptr,store_ptr,
						&kind,&nbeams,detect,&detect_error);
			if (detect_status != MB_SUCCESS)
				{
				status = MB_SUCCESS;
				for (i=0;i<ping[nbuff].beams_bath;i++)
					{
					detect[i] = MB_DETECT_UNKNOWN;
					}
				}
			detect_status = mb_pulses(verbose,imbio_ptr,store_ptr,
						&kind,&nbeams,pulses,&detect_error);
			if (detect_status != MB_SUCCESS)
				{
				status = MB_SUCCESS;
				for (i=0;i<ping[nbuff].beams_bath;i++)
					{
					pulses[i] = MB_PULSE_UNKNOWN;
					}
				}
		    	}
		if (error <= MB_ERROR_NO_ERROR
		    && (kind == MB_DATA_DATA)
		    && (error == MB_ERROR_NO_ERROR
			    || error == MB_ERROR_TIME_GAP
			    || error == MB_ERROR_OUT_BOUNDS
			    || error == MB_ERROR_OUT_TIME
			    || error == MB_ERROR_SPEED_TOO_SMALL))
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}
		else if (error <= MB_ERROR_NO_ERROR)
			{
			status = MB_FAILURE;
			error = MB_ERROR_OTHER;
			}

		if (status == MB_SUCCESS
			&& ping[nbuff].allocated > 0
			&& ping[nbuff].allocated < ping[nbuff].beams_bath)
			{
			ping[nbuff].allocated = 0;
			free(ping[nbuff].beamflag);
			free(ping[nbuff].beamflagorg);
			free(ping[nbuff].bath);
			free(ping[nbuff].bathacrosstrack);
			free(ping[nbuff].bathalongtrack);
			free(ping[nbuff].detect);
			free(ping[nbuff].pulses);
			free(ping[nbuff].bath_x);
			free(ping[nbuff].bath_y);
			}
		if (status == MB_SUCCESS
			&& ping[nbuff].allocated < ping[nbuff].beams_bath)
			{
			ping[nbuff].beamflag = NULL;
			ping[nbuff].beamflagorg = NULL;
			ping[nbuff].bath = NULL;
			ping[nbuff].bathacrosstrack = NULL;
			ping[nbuff].bathalongtrack = NULL;
			ping[nbuff].bath_x = NULL;
			ping[nbuff].bath_y = NULL;
			ping[nbuff].beamflag = (char *) malloc(ping[nbuff].beams_bath*sizeof(char));
			ping[nbuff].beamflagorg = (char *) malloc(ping[nbuff].beams_bath*sizeof(char));
			ping[nbuff].bath = (double *) malloc(ping[nbuff].beams_bath*sizeof(double));
			ping[nbuff].bathacrosstrack = (double *) malloc(ping[nbuff].beams_bath*sizeof(double));
			ping[nbuff].bathalongtrack = (double *) malloc(ping[nbuff].beams_bath*sizeof(double));
			ping[nbuff].detect = (int *) malloc(ping[nbuff].beams_bath*sizeof(int));
			ping[nbuff].pulses = (int *) malloc(ping[nbuff].beams_bath*sizeof(int));
			ping[nbuff].bath_x = (int *) malloc(ping[nbuff].beams_bath*sizeof(int));
			ping[nbuff].bath_y = (int *) malloc(ping[nbuff].beams_bath*sizeof(int));
			ping[nbuff].allocated = ping[nbuff].beams_bath;
			}
		if (status == MB_SUCCESS
			&& ping[nbuff].allocated > 0)
			{
			for (i=0;i<ping[nbuff].beams_bath;i++)
			    {
			    ping[nbuff].beamflag[i] = beamflag[i];
			    ping[nbuff].beamflagorg[i] = beamflag[i];
			    ping[nbuff].bath[i] = bath[i];
			    ping[nbuff].bathacrosstrack[i] = bathacrosstrack[i];
			    ping[nbuff].bathalongtrack[i] = bathalongtrack[i];
			    ping[nbuff].detect[i] = detect[i];
			    ping[nbuff].pulses[i] = pulses[i];
			    ping[nbuff].bath_x[i] = 0;
			    ping[nbuff].bath_y[i] = 0;
			    }
			}
		if (status == MB_SUCCESS)
			{
			nbuff++;
			nload++;
			
			/* update message every 250 records */
			if (nload % 250 == 0)
			    {
			    sprintf(string, "MBedit: %d records loaded so far...", nload);
			    do_message_on(string);
			    }

			/* print output debug statements */
			if (verbose >= 5)
			    {
			    fprintf(stderr,"\ndbg5  Next good data found in function <%s>:\n",
				    function_name);
			    fprintf(stderr,"dbg5       buffer id: %d   global id: %d\n",
				    nbuff - 1, nbuff - 1 + ndump_total);
			    }
			}
		}
	while (error <= MB_ERROR_NO_ERROR
		&& nbuff < buffer_size);
	*ngood = nbuff;
	*nbuffer = nbuff;
	*nloaded = nload;
	nload_total += nload;

	/* define success */
	if (nload > 0)
		{
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		error = MB_ERROR_EOF;
		}

	/* find index of current ping */
	current_id = 0;
	*icurrent = current_id;
	
	/* if desired apply saved edits */
	if (esf.nedit > 0)
		{
		/* reset message */
		do_message_on("MBedit is applying saved edits...");
		
		/* loop over each data record, checking each edit */
		for (i = 0; i < nbuff; i++)
		    {
		    /* apply edits for this ping */
		    status = mb_esf_apply(verbose, &esf, 
		    		ping[i].time_d, ping[i].multiplicity, ping[i].beams_bath, 
				ping[i].beamflag, &error);
			
		    /* update message every 250 records */
		    if (i % 250 == 0)
			{
			sprintf(string, "MBedit: saved edits applied to %d of %d records so far...", 
				i, nbuff - 1);
			do_message_on(string);
			}
		    }
		}
	
	/* if desired filter pings */
	if (filter_medianspike == MB_YES
		|| filter_wrongside == MB_YES
		|| filter_cutbeam == MB_YES
		|| filter_cutdistance == MB_YES
		|| filter_cutangle == MB_YES)
		{
		/* reset message */
		do_message_on("MBedit is applying bathymetry filters...");
		
		/* loop over each data record, checking each edit */
		for (i = 0; i < nbuff; i++)
		    {		
		    mbedit_filter_ping(i);
			
		    /* update message every 250 records */
		    if (i % 250 == 0)
			{
			sprintf(string, "MBedit: filters applied to %d of %d records so far...", 
				i, nbuff - 1);
			do_message_on(string);
			}
 		    }
		}
		
	/* set next button */
	if (*nbuffer < buffer_size)
		do_nextbutton_off();
	else
		do_nextbutton_on();
		
	/* turn message off */
	do_message_off();

	/* print out information */
	if (verbose >= 0)
		{
		fprintf(stderr,"\n%d data records loaded from input file <%s>\n",
			*nloaded,ifile);
		fprintf(stderr,"%d data records now in buffer\n",*nbuffer);
		fprintf(stderr,"%d editable survey data records now in buffer\n",*ngood);
		fprintf(stderr,"Current data record:        %d\n",
			current_id);
		fprintf(stderr,"Current global data record: %d\n",
			current_id + ndump_total);
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
int mbedit_plot_all(
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*nplt, 
		int	autoscale)
{
	/* local variables */
	char	*function_name = "mbedit_plot_all";
	int	status = MB_SUCCESS;
	int	i, j;
	int	nbathsum,  nbathlist;
	double	bathsum, bathmedian;
	double	xtrack_max;
	int	ndec, maxx;
	double	dxscale, dyscale;
	double	dx_width, dy_height;
	int	nx_int, ny_int;
	int	x_int, y_int;
	int	xx, vx, yy, vy;
	int	swidth, sascent, sdescent;
	int	sxstart;
	int	xcen, ycen;
	int	x0, y0, x, y;
	double	dx, dy;
	char	string[MB_PATH_MAXLINE];
	char	*string_ptr;
	int	fpx, fpdx, fpy, fpdy;
	double	tsmin, tsmax, tsscale, tsvalue, tsslope;

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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
		fprintf(stderr,"dbg2       nplt:        %lu\n",(size_t)nplt);
		fprintf(stderr,"dbg2       autoscale:   %d\n",autoscale);
		}

	/* set scales and tick intervals */
	plot_width = plwd;
	exager = exgr;
	show_mode = sh_mode;
	show_flagged = sh_flggd;
	show_time = sh_time,

	/* figure out which pings to plot */
	plot_size = plt_size;
	if (current_id + plot_size > nbuff)
		nplot = nbuff - current_id;
	else
		nplot = plot_size;
	*nplt = nplot;

	/* get data into ping arrays and find median depth value */
	bathsum = 0.0;
	nbathsum = 0;
	nbathlist = 0;
	xtrack_max = 0.0;
	for (i=current_id;i<current_id+nplot;i++)
		{
		ping[i].record = i + ndump_total;
		ping[i].outbounds = MBEDIT_OUTBOUNDS_NONE;
		for (j=0;j<ping[i].beams_bath;j++)
			{
			if (mb_beam_ok(ping[i].beamflag[j]))
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
		
	/* if not enough information in unflagged bathymetry look
	    into the flagged bathymetry */
	if (nbathlist <= 0 || xtrack_max <= 0.0)
		{
		for (i=current_id;i<current_id+nplot;i++)
			{
			for (j=0;j<ping[i].beams_bath;j++)
				{
				if (!mb_beam_ok(ping[i].beamflag[j])
				    && ping[i].beamflag[j] != MB_FLAG_NULL)
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
		}
	if (nbathlist > 0)
		{
		qsort((char *)bathlist,nbathlist,sizeof(double),(void *)mb_double_compare);
		bathmedian = bathlist[nbathlist/2];
		}
		
	/* reset xtrack_max if required */
	if (autoscale && xtrack_max < 0.5)
		{
		xtrack_max = 1000.0;
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

		xntrvl = plot_width / 10;
		if (xntrvl > 1000)
			{
			xntrvl = 1000 * (xntrvl / 1000);
			}
		else if (xntrvl > 500)
			{
			xntrvl = 500 * (xntrvl / 500);
			}
		else if (xntrvl > 250)
			{
			xntrvl = 250 * (xntrvl / 250);
			}
		else if (xntrvl > 100)
			{
			xntrvl = 100 * (xntrvl / 100);
			}
		else if (xntrvl > 50)
			{
			xntrvl = 50 * (xntrvl / 50);
			}
		else if (xntrvl > 25)
			{
			xntrvl = 25 * (xntrvl / 25);
			}
		else if (xntrvl > 10)
			{
			xntrvl = 10 * (xntrvl / 10);
			}
		else if (xntrvl > 5)
			{
			xntrvl = 5 * (xntrvl / 5);
			}
		else if (xntrvl > 2)
			{
			xntrvl = 2 * (xntrvl / 2);
			}
		else
			{
			xntrvl = 1;
			}
		yntrvl = xntrvl;
		do_reset_scale_x(plot_width, maxx, xntrvl, yntrvl);
		}

	/* print out information */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2       %d data records set for plotting (%d desired)\n",
			nplot,plot_size);
		fprintf(stderr,"dbg2       xtrack_max:  %f\n",xtrack_max);
		fprintf(stderr,"dbg2       bathmedian:  %f\n",bathmedian);
		fprintf(stderr,"dbg2       nbathlist:   %d\n",nbathlist);
		fprintf(stderr,"dbg2       nbathsum:    %d\n",nbathsum);
		for (i=current_id;i<current_id+nplot;i++)
			{
			fprintf(stderr,"dbg2       %4d %4d %4d  %d/%d/%d %2.2d:%2.2d:%2.2d.%6.6d  %10.3f\n",
				i,ping[i].id,ping[i].record,
				ping[i].time_i[1],ping[i].time_i[2],
				ping[i].time_i[0],ping[i].time_i[3],
				ping[i].time_i[4],ping[i].time_i[5],
				ping[i].time_i[6],
				ping[i].bath[ping[i].beams_bath/2]);
			}
		}

	/* clear screen */
	xg_fillrectangle(mbedit_xgid,borders[0],borders[2],
		borders[1]-borders[0],borders[3]-borders[2],
		pixel_values[WHITE],XG_SOLIDLINE);

	/* set scaling */
	x_interval = xntrvl;
	y_interval = yntrvl;
	xcen = xmin + (xmax - xmin)/2;
	ycen = ymin + (ymax - ymin)/2;
	dx = ((double)(xmax - xmin))/plot_size;
	dy = ((double)(ymax - ymin))/plot_size;
	xscale = 100.0 * plot_width / (xmax - xmin);
	yscale = (xscale * 100.0) / exager;
	dxscale = 100.0 / xscale;
	dyscale = 100.0 / yscale;

	if (info_set == MB_YES)
		{
		mbedit_plot_info();
		}
	if (sh_mode == MBEDIT_SHOW_FLAG)
		{
		sprintf(string,"Sounding Colors by Flagging:  Unflagged  Manual  Filter  Sonar");
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		sxstart = xcen - swidth / 2;
		
		sprintf(string,"Sounding Colors by Flagging:  Unflagged  ");
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,sxstart,
			ymin-margin/2+sascent+5,string,
			pixel_values[BLACK],XG_SOLIDLINE);
		
		sxstart += swidth;
		sprintf(string,"Manual  ");
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,sxstart,
			ymin-margin/2+sascent+5,string,
			pixel_values[RED],XG_SOLIDLINE);
		
		sxstart += swidth;
		sprintf(string,"Filter  ");
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,sxstart,
			ymin-margin/2+sascent+5,string,
			pixel_values[BLUE],XG_SOLIDLINE);
		
		sxstart += swidth;
		sprintf(string,"Sonar");
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,sxstart,
			ymin-margin/2+sascent+5,string,
			pixel_values[GREEN],XG_SOLIDLINE);
		}
	else if (sh_mode == MBEDIT_SHOW_DETECT)
		{
		sprintf(string,"Sounding Colors by Bottom Detection:  Amplitude  Phase  Unknown");
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		sxstart = xcen - swidth / 2;
		
		sprintf(string,"Sounding Colors by Bottom Detection:  Amplitude  ");
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,sxstart,
			ymin-margin/2+sascent+5,string,
			pixel_values[BLACK],XG_SOLIDLINE);
		
		sxstart += swidth;
		sprintf(string,"Phase  ");
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,sxstart,
			ymin-margin/2+sascent+5,string,
			pixel_values[RED],XG_SOLIDLINE);
		
		sxstart += swidth;
		sprintf(string,"Unknown");
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,sxstart,
			ymin-margin/2+sascent+5,string,
			pixel_values[GREEN],XG_SOLIDLINE);
		}
	else if (sh_mode == MBEDIT_SHOW_PULSE)
		{
		sprintf(string,"Sounding Colors by Source Type:  CW  Up-Chirp  Down-Chirp  Unknown");
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		sxstart = xcen - swidth / 2;
		
		sprintf(string,"Sounding Colors by Source Type:  CW  ");
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,sxstart,
			ymin-margin/2+sascent+5,string,
			pixel_values[BLACK],XG_SOLIDLINE);
		
		sxstart += swidth;
		sprintf(string,"Up-Chirp  ");
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,sxstart,
			ymin-margin/2+sascent+5,string,
			pixel_values[RED],XG_SOLIDLINE);
		
		sxstart += swidth;
		sprintf(string,"Down-Chirp  ");
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,sxstart,
			ymin-margin/2+sascent+5,string,
			pixel_values[BLUE],XG_SOLIDLINE);
		
		sxstart += swidth;
		sprintf(string,"Unknown");
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,sxstart,
			ymin-margin/2+sascent+5,string,
			pixel_values[GREEN],XG_SOLIDLINE);
		}

	sprintf(string,"Vertical Exageration: %4.2f   All Distances and Depths in Meters",(exager/100.));
	xg_justify(mbedit_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(mbedit_xgid,xcen-swidth/2,
		ymin-margin/2+2*(sascent+sdescent)+5,string,
		pixel_values[BLACK],XG_SOLIDLINE);

	/* plot filename */
	sprintf(string,"File %d of %d:", file_id + 1, num_files);
	xg_justify(mbedit_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(mbedit_xgid,margin/2,
		ymin-3*margin/4,string,
		pixel_values[BLACK],XG_SOLIDLINE);
	string_ptr = strrchr(ifile, '/');
	if (string_ptr == NULL)
		string_ptr = ifile;
	else if (strlen(string_ptr) > 0)
		string_ptr++;
	xg_drawstring(mbedit_xgid,margin/2+2+swidth,
		ymin-margin/2-1*(sascent+sdescent)-5,string_ptr,
		pixel_values[BLACK],XG_SOLIDLINE);
		
	/* plot file position bar */
	fpx = margin/2 + ((4 * margin) * current_id) / nbuff;
	fpdx = MAX((((4 * margin) * nplot) / nbuff), 5);
	fpy = ymin - 5*margin/8;
	fpdy = margin/4;
	if (fpx + fpdx > 9 * margin / 2)
	    fpx = 9 * margin /2 - fpdx;
	xg_drawrectangle(mbedit_xgid,
		margin/2,
		ymin-5*margin/8, 
		4*margin, 
		margin/4,
		pixel_values[BLACK],XG_SOLIDLINE);
	xg_drawrectangle(mbedit_xgid,
		margin/2-1,
		ymin-5*margin/8-1, 
		4*margin+2, 
		margin/4+2,
		pixel_values[BLACK],XG_SOLIDLINE);
	xg_fillrectangle(mbedit_xgid,
		fpx, fpy, fpdx, fpdy, 
		pixel_values[LIGHTGREY],XG_SOLIDLINE);
	xg_drawrectangle(mbedit_xgid,
		fpx, fpy, fpdx, fpdy, 
		pixel_values[BLACK],XG_SOLIDLINE);
	sprintf(string,"0 ");
	xg_justify(mbedit_xgid,string,&swidth,
		&sascent,&sdescent);
	xg_drawstring(mbedit_xgid,margin/2-swidth,
		ymin-margin/2+sascent/2,string,
		pixel_values[BLACK],XG_SOLIDLINE);
	sprintf(string," %d", nbuff);
	xg_drawstring(mbedit_xgid,9*margin/2,
		ymin-margin/2+sascent/2,string,
		pixel_values[BLACK],XG_SOLIDLINE);

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
		
	/* plot time series if desired */
	if (show_time > MBEDIT_PLOT_TIME)
		{
		/* get scaling */
		mbedit_tsminmax(current_id, nplot, show_time, &tsmin, &tsmax);
		tsscale = 2.0 * margin / (tsmax - tsmin);
		
		/* draw time series plot box */
		xg_drawline(mbedit_xgid,margin/2,ymin,margin/2,ymax,
			pixel_values[BLACK],XG_SOLIDLINE);
		xg_drawline(mbedit_xgid,margin,ymin,margin,ymax,
			pixel_values[BLACK],XG_DASHLINE);
		xg_drawline(mbedit_xgid,3*margin/2,ymin,3*margin/2,ymax,
			pixel_values[BLACK],XG_DASHLINE);
		xg_drawline(mbedit_xgid,2*margin,ymin,2*margin,ymax,
			pixel_values[BLACK],XG_DASHLINE);
		xg_drawline(mbedit_xgid,5*margin/2,ymin,5*margin/2,ymax,
			pixel_values[BLACK],XG_SOLIDLINE);
		xg_drawline(mbedit_xgid,margin/2,ymax,5*margin/2,ymax,
			pixel_values[BLACK],XG_SOLIDLINE);
		xg_drawline(mbedit_xgid,margin/2,ymin,5*margin/2,ymin,
			pixel_values[BLACK],XG_SOLIDLINE);
		
		/* draw time series labels */
		/*sprintf(string,"Heading (deg)");*/
		mbedit_tslabel(show_time, string);
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,3*margin/2-swidth/2,
			ymin-sdescent,string,
			pixel_values[BLACK],XG_SOLIDLINE);
		sprintf(string,"%g",tsmin);
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,margin/2-swidth/2,
			ymax+sascent+5,string,
			pixel_values[BLACK],XG_SOLIDLINE);
		sprintf(string,"%g",tsmax);
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,5*margin/2-swidth/2,
			ymax+sascent+5,string,
			pixel_values[BLACK],XG_SOLIDLINE);
			
		/*x0 = margin/2 + ping[current_id].heading / 360.0 * 2 * margin;*/
		mbedit_tsvalue(current_id, show_time, &tsvalue);
		x0 = margin/2 + (int) ((tsvalue - tsmin) * tsscale);
		y0 = ymax - (int)(dy / 2);
		for (i=current_id;i<current_id+nplot;i++)
			{
			/*x = margin/2 + ping[i].heading / 360.0 * 2 * margin;*/
			mbedit_tsvalue(i, show_time, &tsvalue);
			x = margin/2 + (int) ((tsvalue - tsmin) * tsscale);
			y = ymax - (int)(dy / 2) - (int)((i - current_id) * dy);
			xg_drawline(mbedit_xgid,x0,y0,x,y,
				pixel_values[BLACK],XG_SOLIDLINE);
			xg_fillrectangle(mbedit_xgid, 
				x-2, y-2, 4, 4, 
				pixel_values[BLACK],XG_SOLIDLINE);
			x0 = x;
			y0 = y;
			}
			
		/* if plotting roll, also plot acrosstrack slope */
		if (show_time == MBEDIT_PLOT_ROLL)
			{
			mbedit_xtrackslope(current_id, &tsslope);
			x0 = margin/2 + (int) ((tsslope - tsmin) * tsscale);
			y0 = ymax - (int)(dy / 2);
			for (i=current_id;i<current_id+nplot;i++)
				{
				/*x = margin/2 + ping[i].heading / 360.0 * 2 * margin;*/
				mbedit_xtrackslope(i, &tsslope);
				x = margin/2 + (int) ((tsslope - tsmin) * tsscale);
				y = ymax - (int)(dy / 2) - (int)((i - current_id) * dy);
				xg_drawline(mbedit_xgid,x0,y0,x,y,
					pixel_values[RED],XG_SOLIDLINE);
				x0 = x;
				y0 = y;
				}
			}
			
		/* if plotting roll, also plot acrosstrack slope - roll */
		if (show_time == MBEDIT_PLOT_ROLL)
			{
			mbedit_xtrackslope(current_id, &tsslope);
			mbedit_tsvalue(i, show_time, &tsvalue);
			x0 = margin/2 + (int) ((tsvalue - tsslope - tsmin) * tsscale);
			y0 = ymax - (int)(dy / 2);
			for (i=current_id;i<current_id+nplot;i++)
				{
				/*x = margin/2 + ping[i].heading / 360.0 * 2 * margin;*/
				mbedit_xtrackslope(i, &tsslope);
				mbedit_tsvalue(i, show_time, &tsvalue);
				x = margin/2 + (int) ((tsvalue - tsslope - tsmin) * tsscale);
				y = ymax - (int)(dy / 2) - (int)((i - current_id) * dy);
				xg_drawline(mbedit_xgid,x0,y0,x,y,
					pixel_values[BLUE],XG_SOLIDLINE);
				x0 = x;
				y0 = y;
				}
			}
		}

	/* plot pings */
	for (i=current_id;i<current_id+nplot;i++)
		{
		/* set beam plotting locations */
		x = xmax - (int)(dx / 2) - (int)((i - current_id) * dx);
		y = ymax - (int)(dy / 2) - (int)((i - current_id) * dy);
		ping[i].label_x = xmin - 5;
		ping[i].label_y = y;
		for (j=0;j<ping[i].beams_bath;j++)
			{
			if (ping[i].beamflag[j] != MB_FLAG_NULL)
				{
				if (view_mode == MBEDIT_VIEW_WATERFALL)
					{
					ping[i].bath_x[j] = xcen 
						+ dxscale*ping[i].bathacrosstrack[j];
					ping[i].bath_y[j] = y + dyscale*
						(fabs((double)ping[i].bath[j]) 
						- bathmedian);
					}
				else if (view_mode == MBEDIT_VIEW_ALONGTRACK)
					{
					ping[i].bath_x[j] = xcen 
						+ dxscale*ping[i].bathacrosstrack[j];
					ping[i].bath_y[j] = ycen + dyscale*
						(fabs((double)ping[i].bath[j]) 
						- bathmedian);
					}
				else
					{
					/* ping[i].bath_x[j] = x;*/
					ping[i].bath_x[j] = xcen
						+ dxscale*(ping[i].bathalongtrack[j] + ping[i].distance - ping[current_id+nplot/2].distance);
					ping[i].bath_y[j] = ycen + dyscale*
						(fabs((double)ping[i].bath[j]) 
						- bathmedian);
					}
				}
			else
				{
				ping[i].bath_x[j] = 0;
				ping[i].bath_y[j] = 0;
				}
			}

		/* plot the beams */
		for (j=0;j<ping[i].beams_bath;j++)
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
int mbedit_plot_beam(int iping, int jbeam)
{
	/* local variables */
	char	*function_name = "mbedit_plot_beam";
	int	status = MB_SUCCESS;
	int	beam_color;

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
	if (info_set == MB_YES && iping == info_ping && jbeam == info_beam)
		{
		if (ping[iping].beamflag[jbeam] != MB_FLAG_NULL)
			xg_fillrectangle(mbedit_xgid, 
				ping[iping].bath_x[jbeam]-4, 
				ping[iping].bath_y[jbeam]-4, 8, 8, 
				pixel_values[BLUE],XG_SOLIDLINE);
		}
	else if (jbeam >= 0 && jbeam < ping[iping].beams_bath
		&& ping[iping].beamflag[jbeam] != MB_FLAG_NULL)
		{
		if (show_mode == MBEDIT_SHOW_FLAG)
			{
			if (mb_beam_ok(ping[iping].beamflag[jbeam]))
				beam_color = BLACK;
			else if (mb_beam_check_flag_filter2(ping[iping].beamflag[jbeam]))
				beam_color = BLUE;
			else if (mb_beam_check_flag_filter(ping[iping].beamflag[jbeam]))
				beam_color = BLUE;
			else if (mb_beam_check_flag_sonar(ping[iping].beamflag[jbeam]))
				beam_color = GREEN;
			else if (ping[iping].beamflag[jbeam] != MB_FLAG_NULL)
				beam_color = RED;
			else
				beam_color = GREEN;
			}
		else if (show_mode == MBEDIT_SHOW_DETECT)
			{
			if (ping[iping].detect[jbeam] == MB_DETECT_AMPLITUDE)
				beam_color = BLACK;
			else if (ping[iping].detect[jbeam] == MB_DETECT_PHASE)
				beam_color = RED;
			else
				beam_color = GREEN;
			}
		else if (show_mode == MBEDIT_SHOW_PULSE)
			{
			if (ping[iping].pulses[jbeam] == MB_PULSE_CW)
				beam_color = BLACK;
			else if (ping[iping].pulses[jbeam] == MB_PULSE_UPCHIRP)
				beam_color = RED;
			else if (ping[iping].pulses[jbeam] == MB_PULSE_DOWNCHIRP)
				beam_color = BLUE;
			else
				beam_color = GREEN;
			}
		if (mb_beam_ok(ping[iping].beamflag[jbeam]))
			xg_fillrectangle(mbedit_xgid, 
				ping[iping].bath_x[jbeam]-2, 
				ping[iping].bath_y[jbeam]-2, 4, 4, 
				pixel_values[beam_color],XG_SOLIDLINE);
		else
			xg_drawrectangle(mbedit_xgid, 
				ping[iping].bath_x[jbeam]-2, 
				ping[iping].bath_y[jbeam]-2, 4, 4, 
				pixel_values[beam_color],XG_SOLIDLINE);
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
int mbedit_plot_ping(int iping)
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
	for (j=0;j<ping[iping].beams_bath;j++)
		{
		if (show_flagged == MB_YES 
			&& !mb_beam_ok(ping[iping].beamflag[j]) 
			&& ping[iping].beamflag[j] != MB_FLAG_NULL
			&& first == MB_YES)
			{
			first = MB_NO;
			last_flagged = MB_YES;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
			}
		else if (mb_beam_ok(ping[iping].beamflag[j]) 
			&& first == MB_YES)
			{
			first = MB_NO;
			last_flagged = MB_NO;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
			}
		else if (last_flagged == MB_NO 
			&& mb_beam_ok(ping[iping].beamflag[j]))
			{
			xg_drawline(mbedit_xgid,xold,yold,
					ping[iping].bath_x[j],
					ping[iping].bath_y[j],
					pixel_values[BLACK],XG_SOLIDLINE);
			last_flagged = MB_NO;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
			}
		else if (mb_beam_ok(ping[iping].beamflag[j]))
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
			&& !mb_beam_ok(ping[iping].beamflag[j])
			&& ping[iping].beamflag[j] != MB_FLAG_NULL)
			{
			if (j > 0)
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
int mbedit_plot_ping_label(int iping, int save)
{
	/* local variables */
	char	*function_name = "mbedit_plot_ping_label";
	int	status = MB_SUCCESS;
	int	sascent, sdescent, swidth;
	char	string[MB_PATH_MAXLINE];
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
	for (j=0;j<ping[iping].beams_bath;j++)
		{
		if (ping[iping].beamflag[j] != MB_FLAG_NULL
		    && (ping[iping].bath_x[j] < xmin
		    || ping[iping].bath_x[j] > xmax
		    || ping[iping].bath_y[j] < ymin
		    || ping[iping].bath_y[j] > ymax))
		    {
		    if (mb_beam_ok(ping[iping].beamflag[j]))
			ping[iping].outbounds 
			    = MBEDIT_OUTBOUNDS_UNFLAGGED;
		    else if (ping[iping].beamflag[j] != MB_FLAG_NULL
			&& ping[iping].outbounds != MBEDIT_OUTBOUNDS_UNFLAGGED)
			ping[iping].outbounds 
			    = MBEDIT_OUTBOUNDS_FLAGGED;
		    }
		}

	/* set info string with time tag */
	if (show_time == MBEDIT_PLOT_TIME || save == MB_YES)
		{
		if (ping[iping].beams_bath > 0 
			&& mb_beam_ok(ping[iping].beamflag[ping[iping].beams_bath/2]))
		sprintf(string,"%5d %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d %10.3f",
			ping[iping].record + 1,
			ping[iping].time_i[1],ping[iping].time_i[2],
			ping[iping].time_i[0],ping[iping].time_i[3],
			ping[iping].time_i[4],ping[iping].time_i[5],
			(int)(0.001 * ping[iping].time_i[6]),
			ping[iping].bath[ping[iping].beams_bath/2]);
		else if (ping[iping].beams_bath > 0)
		sprintf(string,"%5d %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d %10.3f",
			ping[iping].record + 1,
			ping[iping].time_i[1],ping[iping].time_i[2],
			ping[iping].time_i[0],ping[iping].time_i[3],
			ping[iping].time_i[4],ping[iping].time_i[5],
			(int)(0.001 * ping[iping].time_i[6]),
			ping[iping].altitude + ping[iping].sonardepth);
		else
		sprintf(string,"%5d %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d %10.3f",
			ping[iping].record + 1,
			ping[iping].time_i[1],ping[iping].time_i[2],
			ping[iping].time_i[0],ping[iping].time_i[3],
			ping[iping].time_i[4],ping[iping].time_i[5],
			(int)(0.001 * ping[iping].time_i[6]),
			0.0);

		/* save string to show last ping seen at end of program */
		if (save == MB_YES)
			strcpy(last_ping,string);
		}

	/* set info string without time tag */
	if (show_time != MBEDIT_PLOT_TIME)
		{
		if (ping[iping].beams_bath > 0)
		sprintf(string,"%5d %10.3f",
			ping[iping].record,
			ping[iping].bath[ping[iping].beams_bath/2]);
		else
		sprintf(string,"%5d %10.3f",
			ping[iping].record,0.0);

		/* save string to show last ping seen at end of program */
		if (save == MB_YES)
			strcpy(last_ping,string);
		}

	/* justify the string */
	xg_justify(mbedit_xgid,string,&swidth,&sascent,&sdescent);

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
int mbedit_plot_info()
{
	/* local variables */
	char	*function_name = "mbedit_plot_info";
	int	status = MB_SUCCESS;
	char	string[MB_PATH_MAXLINE];
	int	sascent, sdescent, swidth;
	int	xcen;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* plot the info */
	if (info_set == MB_YES)
		{
		xcen = xmin + (xmax - xmin)/2;

		sprintf(string,"Selected Sounding: Ping:%d Beam:%d",
			info_ping, info_beam);
		sprintf(string,"Ping:%d  Beam:%d  Time: %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d",
			info_ping, info_beam,
			info_time_i[1],info_time_i[2],
			info_time_i[0],info_time_i[3],
			info_time_i[4],info_time_i[5],
			(int)(0.001 * info_time_i[6]));
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,xcen-swidth/2,
			ymin-margin/2-2*(sascent+sdescent),string,
			pixel_values[BLACK],XG_SOLIDLINE);

		sprintf(string,"Longitude:%.5f  Latitude:%.5f  Heading:%.1f  Speed:%.1f",
			info_navlon, info_navlat, info_heading, info_speed);
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,xcen-swidth/2,
			ymin-margin/2-1*(sascent+sdescent),string,
			pixel_values[BLACK],XG_SOLIDLINE);

		sprintf(string,"Depth:%.2f  XTrack:%.2f  LTrack:%.2f  Altitude:%.2f  Detect:%s  Pulse:%s",
			info_bath, info_bathacrosstrack, 
			info_bathalongtrack, info_altitude, detect_name[info_detect], pulse_name[info_pulse]);
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,xcen-swidth/2,
			ymin-margin/2,string,
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
int mbedit_unplot_beam(int iping, int jbeam)
{
	/* local variables */
	char	*function_name = "mbedit_unplot_beam";
	int	status = MB_SUCCESS;

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
	if (info_set == MB_YES && iping == info_ping && jbeam == info_beam)
		{
		if (ping[iping].beamflag[jbeam] != MB_FLAG_NULL)
			xg_fillrectangle(mbedit_xgid, 
				ping[iping].bath_x[jbeam]-4, 
				ping[iping].bath_y[jbeam]-4, 8, 8, 
				pixel_values[WHITE],XG_SOLIDLINE);
		}
	else if (jbeam >= 0 && jbeam < ping[iping].beams_bath)
		{
		if (mb_beam_ok(ping[iping].beamflag[jbeam]))
			xg_fillrectangle(mbedit_xgid, 
				ping[iping].bath_x[jbeam]-2, 
				ping[iping].bath_y[jbeam]-2, 4, 4, 
				pixel_values[WHITE],XG_SOLIDLINE);
		else if (ping[iping].beamflag[jbeam] != MB_FLAG_NULL)
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
int mbedit_unplot_ping(int iping)
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
	for (j=0;j<ping[iping].beams_bath;j++)
		{
		if (mb_beam_ok(ping[iping].beamflag[j]) && first == MB_YES)
			{
			first = MB_NO;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
			}
		else if (mb_beam_ok(ping[iping].beamflag[j]))
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
int mbedit_unplot_info()
{
	/* local variables */
	char	*function_name = "mbedit_unplot_info";
	int	status = MB_SUCCESS;
	char	string[MB_PATH_MAXLINE];
	int	sascent, sdescent, swidth;
	int	xcen;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* plot the info */
	if (info_set == MB_YES)
		{
		xcen = xmin + (xmax - xmin)/2;

		sprintf(string,"Selected Sounding: Ping:%d Beam:%d",
			info_ping, info_beam);
		sprintf(string,"Ping:%d  Beam:%d  Time: %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d",
			info_ping, info_beam,
			info_time_i[1],info_time_i[2],
			info_time_i[0],info_time_i[3],
			info_time_i[4],info_time_i[5],
			(int)(0.001 * info_time_i[6]));
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,xcen-swidth/2,
			ymin-margin/2-2*(sascent+sdescent),string,
			pixel_values[WHITE],XG_SOLIDLINE);

		sprintf(string,"Longitude:%.5f  Latitude:%.5f  Heading:%.1f  Speed:%.1f",
			info_navlon, info_navlat, info_heading, info_speed);
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,xcen-swidth/2,
			ymin-margin/2-1*(sascent+sdescent),string,
			pixel_values[WHITE],XG_SOLIDLINE);

		sprintf(string,"Depth:%.2f  XTrack:%.2f  LTrack:%.2f  Altitude:%.2f  Detect:%d  Pulse:%d",
			info_bath, info_bathacrosstrack, 
			info_bathalongtrack, info_altitude, info_detect, info_pulse);
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,xcen-swidth/2,
			ymin-margin/2,string,
			pixel_values[WHITE],XG_SOLIDLINE);

		sprintf(string,"Depth:%.2f  XTrack:%.2f  LTrack:%.2f  Altitude:%.2f  Detect:%s  Pulse:%s",
			info_bath, info_bathacrosstrack, 
			info_bathalongtrack, info_altitude, detect_name[info_detect], pulse_name[info_pulse]);
		xg_justify(mbedit_xgid,string,&swidth,
			&sascent,&sdescent);
		xg_drawstring(mbedit_xgid,xcen-swidth/2,
			ymin-margin/2,string,
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
int mbedit_action_goto(
		int	ttime_i[7], 
		int	hold_size, 
		int	buffer_size, 
		int	plwd, 
		int	exgr, 
		int	xntrvl, 
		int	yntrvl, 
		int	plt_size, 
		int	sh_mode, 
		int	sh_flggd, 
		int	sh_time,
		int	*ndumped, 
		int	*nloaded, 
		int	*nbuffer, 
		int	*ngood, 
		int	*icurrent, 
		int	*nplt)
{
	/* local variables */
	char	*function_name = "mbedit_action_goto";
	int	status = MB_SUCCESS;
	double	ttime_d;
	int	found;
	int	i;

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
		fprintf(stderr,"dbg2       show_mode:   %d\n",sh_mode);
		fprintf(stderr,"dbg2       show_flagged:%d\n",sh_flggd);
		fprintf(stderr,"dbg2       show_time:   %d\n",sh_time);
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
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
		*nplt = 0;
		if (verbose >= 1)
			fprintf(stderr,"\n>> No data file has been opened...\n");
		do_error_dialog("No data file has", 
				"been opened...", 
				"  ");
		}

	/* check if the target time is in the present buffer */
	else if (nbuff > 0)
		{
		/* check if the present buffer starts 
		    later than the target time */
		if (ping[0].time_d > ttime_d)
			{
			status = MB_FAILURE;
			*ndumped = 0;
			*nloaded = 0;
			*nbuffer = nbuff;
			*ngood = nbuff;
			*icurrent = current_id;
			*nplt = 0;
			if (verbose >= 1)
				fprintf(stderr,"\n>> Beginning of present buffer is later than target time...\n");
			do_error_dialog("Beginning of loaded data",
					"is later than the", 
					"specified target time...");
			}

		/* check if the file ends 
		    before the target time */
		else if (ping[nbuff-1].time_d < ttime_d 
			&& nbuff < buffer_size)
			{
			status = MB_FAILURE;
			*ndumped = 0;
			*nloaded = 0;
			*nbuffer = nbuff;
			*ngood = nbuff;
			*icurrent = current_id;
			*nplt = 0;
			if (verbose >= 1)
				fprintf(stderr,"\n>> Target time is beyond end of file...\n");
			do_error_dialog("Target time is",
					"beyond the end", 
					"of the data file...");
			}
		}
		
	/* loop through buffers until the target time is found
		or the file ends */
	while (found == MB_NO && status == MB_SUCCESS)
		{
		/* check out current buffer */
		for (i=0;i<nbuff;i++)
			{
			if (ping[i].time_d > ttime_d && found == MB_NO)
				{
				found = MB_YES;
				current_id = i;
				}
			}

		/* load new buffer if needed */
		if (found == MB_NO && nbuff >= buffer_size)
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
				*nbuffer = nbuff;
				*ngood = nbuff;
				*icurrent = current_id;
				*nplt = 0;
				if (verbose >= 1)
					fprintf(stderr,"\n>> Target time is beyond end of file, file closed...\n");
				do_error_dialog("Target time is beyond the",
						"end of the data file!", 
						"The file has been closed...");
				}
			}
	
		/* turns out the file ends 
		    before the target time */
		else if (found == MB_NO && nbuff < buffer_size)
			{
			status = MB_FAILURE;
			*nbuffer = nbuff;
			*ngood = nbuff;
			*icurrent = current_id;
			*nplt = 0;
			if (verbose >= 1)
				fprintf(stderr,"\n>> Target time is beyond end of file...\n");
			do_error_dialog("Target time is",
					"beyond the end", 
					"of the data file...");
			}
		}

	/* clear the screen */
	status = mbedit_clear_screen();

	/* set up plotting */
	if (*ngood > 0)
		{
		status = mbedit_plot_all(plwd,exgr,xntrvl,yntrvl,
				plt_size,sh_mode,sh_flggd,sh_time,nplt, MB_NO);
		}

	/* let the world know... */
	if (verbose >= 2 && found == MB_YES)
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
		fprintf(stderr,"Current global data record: %d\n",
			current_id + ndump_total);
		}
	else if (verbose >= 2)
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
int mbedit_tslabel(int data_id, char *label)
{
	/* local variables */
	char	*function_name = "mbedit_tslabel";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       data_id:         %d\n",data_id);
		}
		
	/* get the time series label */
       switch (data_id)
	       {
	       case MBEDIT_PLOT_WIDE:
		       strcpy(label, "WIDE PLOT");
		       break;
	       case MBEDIT_PLOT_TIME:
		       strcpy(label, "TIME STAMP");
		       break;
	       case MBEDIT_PLOT_INTERVAL:
		       strcpy(label, "Ping Interval (sec)");
		       break;
	       case MBEDIT_PLOT_LON:
		       strcpy(label, "Longitude (deg)");
		       break;
	       case MBEDIT_PLOT_LAT:
		       strcpy(label, "Latitude (deg)");
		       break;
	       case MBEDIT_PLOT_HEADING:
		       strcpy(label, "Heading (deg)");
		       break;
	       case MBEDIT_PLOT_SPEED:
		       strcpy(label, "Speed (km/hr)");
		       break;
	       case MBEDIT_PLOT_DEPTH:
		       strcpy(label, "Center Beam Depth (m)");
		       break;
	       case MBEDIT_PLOT_ALTITUDE:
		       strcpy(label, "Sonar Altitude (m)");
		       break;
	       case MBEDIT_PLOT_SONARDEPTH:
		       strcpy(label, "Sonar Depth (m)");
		       break;
	       case MBEDIT_PLOT_ROLL:
		       strcpy(label, "Roll (deg)");
		       break;
	       case MBEDIT_PLOT_PITCH:
		       strcpy(label, "Pitch (deg)");
		       break;
	       case MBEDIT_PLOT_HEAVE:
		       strcpy(label, "Heave (m)");
		       break;
	       }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       label:       %s\n",label);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_tsvalue(int iping, int data_id, double *value)
{
	/* local variables */
	char	*function_name = "mbedit_tsvalue";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iping:           %d\n",iping);
		fprintf(stderr,"dbg2       data_id:         %d\n",data_id);
		}
		
	/* get the time series value */
	if (iping >= 0 && nbuff > iping)
		{
		switch (data_id)
			{
			case MBEDIT_PLOT_WIDE:
				*value = 0.0;
				break;
			case MBEDIT_PLOT_TIME:
				*value = 0.0;
				break;
			case MBEDIT_PLOT_INTERVAL:
				*value = ping[iping].time_interval;
				break;
			case MBEDIT_PLOT_LON:
				*value = ping[iping].navlon;
				break;
			case MBEDIT_PLOT_LAT:
				*value = ping[iping].navlat;
				break;
			case MBEDIT_PLOT_HEADING:
				*value = ping[iping].heading;
				break;
			case MBEDIT_PLOT_SPEED:
				*value = ping[iping].speed;
				break;
			case MBEDIT_PLOT_DEPTH:
				*value = ping[iping].bath[ping[iping].beams_bath/2];
				break;
			case MBEDIT_PLOT_ALTITUDE:
				*value = ping[iping].altitude;
				break;
			case MBEDIT_PLOT_SONARDEPTH:
				*value = ping[iping].sonardepth;
				break;
			case MBEDIT_PLOT_ROLL:
				*value = ping[iping].roll;
				break;
			case MBEDIT_PLOT_PITCH:
				*value = ping[iping].pitch;
				break;
			case MBEDIT_PLOT_HEAVE:
				*value = ping[iping].heave;
				break;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       value:       %f\n",*value);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_tsminmax(int iping, int nping, int data_id, double *tsmin, double *tsmax)
{
	/* local variables */
	char	*function_name = "mbedit_tsminmax";
	int	status = MB_SUCCESS;
	double	value, value2;
	double	halfwidth;
	double	center;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iping:           %d\n",iping);
		fprintf(stderr,"dbg2       nping:           %d\n",nping);
		fprintf(stderr,"dbg2       data_id:         %d\n",data_id);
		}
		
	/* get the time series minimum and maximum value */
	*tsmin = 0.0;
	*tsmax = 0.0;
	if (iping >= 0 
		&& nbuff > iping 
		&& nping > 0 
		&& iping + nping - 1 < nbuff)
		{
		mbedit_tsvalue(iping, data_id, tsmin);
		*tsmax = *tsmin;
		for (i=iping;i<iping+nping;i++)
			{
			mbedit_tsvalue(i, data_id, &value);
			*tsmin = MIN(*tsmin, value);
			*tsmax = MAX(*tsmax, value);
			
			/* handle slope plotting in roll plot */
			if (data_id == MBEDIT_PLOT_ROLL)
				{
				mbedit_xtrackslope(i, &value2);
				*tsmin = MIN(*tsmin, value2);
				*tsmax = MAX(*tsmax, value2);
				*tsmin = MIN(*tsmin, value - value2);
				*tsmax = MAX(*tsmax, value - value2);
				}
			}
		}
		
	/* adjust the min max according to data type */
	switch (data_id)
		{
		case MBEDIT_PLOT_WIDE:
			*tsmin = 0.0;
			*tsmax = 1.0;
			break;
		case MBEDIT_PLOT_TIME:
			*tsmin = 0.0;
			*tsmax = 1.0;
			break;
		case MBEDIT_PLOT_INTERVAL:
			*tsmin = 0.0;
			*tsmax = MAX(1.1 * (*tsmax), 0.01);
			break;
		case MBEDIT_PLOT_LON:
			halfwidth = MAX(0.001, 0.55 * (*tsmax - *tsmin));
			center = 0.5 * (*tsmin + *tsmax);
			*tsmin = center - halfwidth;
			*tsmax = center + halfwidth;
			break;
		case MBEDIT_PLOT_LAT:
			halfwidth = MAX(0.001, 0.55 * (*tsmax - *tsmin));
			center = 0.5 * (*tsmin + *tsmax);
			*tsmin = center - halfwidth;
			*tsmax = center + halfwidth;
			break;
		case MBEDIT_PLOT_HEADING:
			*tsmin = 0.0;
			*tsmax = 360.0;
			break;
		case MBEDIT_PLOT_SPEED:
			*tsmin = 0.0;
			*tsmax = MAX(*tsmax, 5.0);
			break;
		case MBEDIT_PLOT_DEPTH:
			halfwidth = MAX(1.0, 0.55 * (*tsmax - *tsmin));
			center = 0.5 * (*tsmin + *tsmax);
			*tsmin = center - halfwidth;
			*tsmax = center + halfwidth;
			break;
		case MBEDIT_PLOT_ALTITUDE:
			halfwidth = MAX(1.0, 0.55 * (*tsmax - *tsmin));
			center = 0.5 * (*tsmin + *tsmax);
			*tsmin = center - halfwidth;
			*tsmax = center + halfwidth;
			break;
		case MBEDIT_PLOT_SONARDEPTH:
			halfwidth = MAX(1.0, 0.55 * (*tsmax - *tsmin));
			center = 0.5 * (*tsmin + *tsmax);
			*tsmin = center - halfwidth;
			*tsmax = center + halfwidth;
			break;
		case MBEDIT_PLOT_ROLL:
			*tsmax = 1.1 * MAX(fabs(*tsmin), fabs(*tsmax));
			*tsmax = MAX(*tsmax, 1.0);
			*tsmin = -(*tsmax);
			break;
		case MBEDIT_PLOT_PITCH:
			*tsmax = 1.1 * MAX(fabs(*tsmin), fabs(*tsmax));
			*tsmax = MAX(*tsmax, 1.0);
			*tsmin = -(*tsmax);
			break;
		case MBEDIT_PLOT_HEAVE:
			*tsmax = 1.1 * MAX(fabs(*tsmin), fabs(*tsmax));
			*tsmax = MAX(*tsmax, 0.25);
			*tsmin = -(*tsmax);
			break;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       tsmin:       %f\n",*tsmin);
		fprintf(stderr,"dbg2       tsmax:       %f\n",*tsmax);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbedit_xtrackslope(int iping, double *slope)
{
	/* local variables */
	char	*function_name = "mbedit_xtrackslope";
	int	status = MB_SUCCESS;
	int	jbeam;
	int	ns;
	double	sx, sy, sxx, sxy, delta, b;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       iping:           %d\n",iping);
		}

	/* initialize linear fit variables */
	sx = 0.0;
	sy = 0.0;
	sxx = 0.0;
	sxy = 0.0;
	ns = 0;
	*slope = 0.0;
		
	/* get the slope value */
	if (iping >= 0 && nbuff > iping)
		{
		ns = 0;
		for (jbeam=0;jbeam<ping[iping].beams_bath;jbeam++)
		    	{
			/* use valid beams to calculate slope */
			if (mb_beam_ok(ping[iping].beamflag[jbeam]))
				{
				sx += ping[iping].bathacrosstrack[jbeam];
				sy += ping[iping].bath[jbeam];
				sxx += ping[iping].bathacrosstrack[jbeam] * ping[iping].bathacrosstrack[jbeam];
				sxy += ping[iping].bathacrosstrack[jbeam] * ping[iping].bath[jbeam];
				ns++;
				}
			}

		/* get linear fit to ping */
		if (ns > 0)
			{
			delta = ns*sxx - sx*sx;
			/* a = (sxx*sy - sx*sxy)/delta; */
			b = (ns*sxy - sx*sy)/delta;
			*slope = -RTD * atan(b);;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       slope:       %f\n",*slope);
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------*/
