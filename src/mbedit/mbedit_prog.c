/*--------------------------------------------------------------------
 *    The MB-system:	mbedit.c	4/8/93
 *
 *    Copyright (c) 1993-2023 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
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
 */

/*--------------------------------------------------------------------*/

#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#	ifndef WIN32
#		define WIN32
#	endif
#	include <WinSock2.h>
#include <windows.h>
#endif

#include <X11/Intrinsic.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mb_process.h"
#include "mb_swap.h"
#include "mb_xgraphics.h"
#include "mbedit.h"

/* output mode defines */
#define MBEDIT_OUTPUT_EDIT 1
#define MBEDIT_OUTPUT_BROWSE 2

/* edit outbounds defines */
#define MBEDIT_OUTBOUNDS_NONE 0
#define MBEDIT_OUTBOUNDS_FLAGGED 1
#define MBEDIT_OUTBOUNDS_UNFLAGGED 2

/* plot modes */
#define MBEDIT_PLOT_WIDE 0
#define MBEDIT_PLOT_TIME 1
#define MBEDIT_PLOT_INTERVAL 2
#define MBEDIT_PLOT_LON 3
#define MBEDIT_PLOT_LAT 4
#define MBEDIT_PLOT_HEADING 5
#define MBEDIT_PLOT_SPEED 6
#define MBEDIT_PLOT_DEPTH 7
#define MBEDIT_PLOT_ALTITUDE 8
#define MBEDIT_PLOT_SENSORDEPTH 9
#define MBEDIT_PLOT_ROLL 10
#define MBEDIT_PLOT_PITCH 11
#define MBEDIT_PLOT_HEAVE 12

/* view modes */
#define MBEDIT_VIEW_WATERFALL 0
#define MBEDIT_VIEW_ALONGTRACK 1
#define MBEDIT_VIEW_ACROSSTRACK 2
#define MBEDIT_SHOW_FLAG 0
#define MBEDIT_SHOW_DETECT 1
#define MBEDIT_SHOW_PULSE 2

/* grab modes */
#define MBEDIT_GRAB_START 0
#define MBEDIT_GRAB_MOVE 1
#define MBEDIT_GRAB_END 2

/* Bottom detect type names */
char *detect_name[] = {"Unknown", "Amplitude", "Phase"};

/* Source pulse type names */
char *pulse_name[] = {"Unknown", "CW", "Up-Chirp", "Down-Chirp"};

/* ping structure definition */
struct mbedit_ping_struct {
	int allocated;
	int id;
	int record;
	int outbounds;
	int time_i[7];
	double time_d;
	int multiplicity;
	double time_interval;
	double navlon;
	double navlat;
	double speed;
	double heading;
	double altitude;
	double sensordepth;
	double roll;
	double pitch;
	double heave;
	double distance;
	int beams_bath;
	char *beamflag;
	char *beamflagorg;
	double *bath;
	double *amp;
	double *bathacrosstrack;
	double *bathalongtrack;
	int *detect;
	int *priority;
	int *pulses;
	int *bath_x;
	int *bath_y;
	int label_x;
	int label_y;
	int zap_x1;
	int zap_x2;
	int zap_y1;
	int zap_y2;
};

static const char program_name[] = "MBedit";
static const char help_message[] =
    "MBedit is an interactive editor used to identify and flag\n"
    "artifacts in swath sonar bathymetry data. Once a file has\n"
    "been read in, MBedit displays the bathymetry profiles from\n"
    "several pings, allowing the user to identify and flag\n"
    "anomalous beams. Flagging is handled internally by setting\n"
    "depth values negative, so that no information is lost.";
static const char usage_message[] =
    "mbedit [-Byr/mo/da/hr/mn/sc -D  -Eyr/mo/da/hr/mn/sc \n\t-Fformat "
    "-Ifile -Ooutfile -S -X -V -H]";

/* status variables */
static int error = MB_ERROR_NO_ERROR;
static int verbose = 0;
static char *message = NULL;

/* MBIO control parameters */
static int format;
static int pings;
static int lonflip;
static double bounds[4];
static int btime_i[7];
static int etime_i[7];
static double btime_d;
static double etime_d;
static double speedmin;
static double timegap;
static int beams_bath;
static int beams_amp;
static int pixels_ss;
static char ifile[MB_PATH_MAXLINE];
static void *imbio_ptr = NULL;
static int output_mode = MBEDIT_OUTPUT_EDIT;
static bool run_mbprocess = false;
static bool gui_mode = false;
static bool uselockfiles = true;

/* mbio read and write values */
static void *store_ptr = NULL;
static int kind;
static double distance;
static double draft;
static char *beamflag = NULL;
static double *bath = NULL;
static double *bathacrosstrack = NULL;
static double *bathalongtrack = NULL;
static double *amp = NULL;
static double *ss = NULL;
static double *ssacrosstrack = NULL;
static double *ssalongtrack = NULL;
static int *detect = NULL;
static int *priority = NULL;
static int *pulses = NULL;
static int *editcount = NULL;
static char comment[MB_COMMENT_MAXLINE];

/* buffer control variables */
#define MBEDIT_BUFFER_SIZE 30000
static bool file_open = false;
static int buff_size = MBEDIT_BUFFER_SIZE;
static int buff_size_max = MBEDIT_BUFFER_SIZE;
static int holdd_size = MBEDIT_BUFFER_SIZE / 1000;
static int nload = 0;
static int ndump = 0;
static int nbuff = 0;
static int current_id = 0;
static int nload_total = 0;
static int ndump_total = 0;
static char last_ping[MB_PATH_MAXLINE];
static int file_id;
static int num_files;

/* info parameters */
static bool info_set = false;
static int info_ping;
static int info_beam;
static int info_time_i[7];
static double info_time_d;
static double info_navlon;
static double info_navlat;
static double info_speed;
static double info_heading;
static double info_altitude;
static int info_beams_bath;
static char info_beamflag;
static double info_bath;
static double info_amp;
static double info_bathacrosstrack;
static double info_bathalongtrack;
static int info_detect;
static int info_priority;
static int info_pulse;

/* grab parameters */
static int grab_set = false;
static int grab_start_x;
static int grab_start_y;
static int grab_end_x;
static int grab_end_y;

/* save file control variables */
static bool esffile_open = false;
struct mb_esf_struct esf;
static char esffile[MB_PATH_MAXLINE];
static char notice[MB_PATH_MAXLINE];

/* filter variables */
static bool filter_medianspike = false;
static int filter_medianspike_threshold = 10;
static int filter_medianspike_xtrack = 5;
static int filter_medianspike_ltrack = 1;
static bool filter_wrongside = false;
static int filter_wrongside_threshold = 15;
static bool filter_cutbeam = false;
static int filter_cutbeam_begin = 0;
static int filter_cutbeam_end = 0;
static bool filter_cutdistance = false;
static double filter_cutdistance_begin = 0.0;
static double filter_cutdistance_end = 0.0;
static bool filter_cutangle = false;
static double filter_cutangle_begin = 0.0;
static double filter_cutangle_end = 0.0;

/* ping drawing control variables */
#define MBEDIT_MAX_PINGS 250
#define MBEDIT_PICK_DISTANCE 50
#define MBEDIT_ERASE_DISTANCE 15
struct mbedit_ping_struct ping[MBEDIT_BUFFER_SIZE];
static int view_mode = MBEDIT_VIEW_WATERFALL;
static int plot_size = 10;
static int nplot = 0;
static void *mbedit_xgid;
static int borders[4];
static int margin;
static int xmin, xmax;
static int ymin, ymax;
static int exager = 100;
static int plot_width = 5000;
static double xscale;
static double yscale;
static int x_interval = 1000;
static int y_interval = 250;
static int show_mode = MBEDIT_SHOW_FLAG;
static bool show_flaggedsoundings = true;
static bool show_flaggedprofiles = false;
static int show_time = MBEDIT_PLOT_TIME;
static bool beam_save = false;
static int iping_save = 0;
static int jbeam_save = 0;
static double *bathlist;

/* color control values */
typedef enum {
    WHITE = 0,
    BLACK = 1,
    RED = 2,
    GREEN = 3,
    BLUE = 4,
    CORAL = 5,
    LIGHTGREY = 6,
} mbedit_color_t;

static int ncolors;
static unsigned int pixel_values[256];

/*--------------------------------------------------------------------*/
int mbedit_init(int argc, char **argv, int *startup_file) {
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);
	status = mb_uselockfiles(verbose, &uselockfiles);
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
	strcpy(ifile, "");

	int fileflag = 0;

	int errflg = 0;
	int c;
	int help = 0;

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhB:b:DdE:e:F:f:GgI:i:SsXx")) != -1) {
		switch (c) {
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
			sscanf(optarg, "%d/%d/%d/%d/%d/%d", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &btime_i[5]);
			btime_i[6] = 0;
			break;
		case 'D':
		case 'd':
			output_mode = MBEDIT_OUTPUT_BROWSE;
			break;
		case 'E':
		case 'e':
			sscanf(optarg, "%d/%d/%d/%d/%d/%d", &etime_i[0], &etime_i[1], &etime_i[2], &etime_i[3], &etime_i[4], &etime_i[5]);
			etime_i[6] = 0;
			break;
		case 'F':
		case 'f':
			sscanf(optarg, "%d", &format);
			break;
		case 'G':
		case 'g':
			gui_mode = true;
			break;
		case 'I':
		case 'i':
			sscanf(optarg, "%s", ifile);
			do_parse_datalist(ifile, format);
			fileflag++;
			break;
		case 'X':
		case 'x':
			run_mbprocess = true;
			break;
		case '?':
			errflg++;
		}
	}

	/* if error flagged then print it and exit */
	if (errflg) {
		fprintf(stderr, "usage: %s\n", usage_message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
	}

	/* print starting message */
	if (verbose == 1 || help) {
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
	}

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Control Parameters:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       help:            %d\n", help);
		fprintf(stderr, "dbg2       format:          %d\n", format);
		fprintf(stderr, "dbg2       input file:      %s\n", ifile);
		fprintf(stderr, "dbg2       output mode:     %d\n", output_mode);
	}

	if (help) {
		fprintf(stderr, "\n%s\n", help_message);
		fprintf(stderr, "\nusage: %s\n", usage_message);
		exit(error);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       argc:      %d\n", argc);
		for (int i = 0; i < argc; i++)
			fprintf(stderr, "dbg2       argv[%d]:    %s\n", i, argv[i]);
	}

	/* if file specified then use it */
	*startup_file = fileflag > 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       startup_file: %d\n", *startup_file);
		fprintf(stderr, "dbg2       error:        %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbedit_set_graphics(void *xgid, int ncol, unsigned int *pixels) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       xgid:         %p\n", xgid);
		fprintf(stderr, "dbg2       ncolors:      %d\n", ncol);
		for (int i = 0; i < ncol; i++)
			fprintf(stderr, "dbg2       pixel[%d]:     %u\n", i, pixels[i]);
	}

	/* set graphics id */
	mbedit_xgid = xgid;

	/* set colors */
	ncolors = ncol;
	for (int i = 0; i < ncolors; i++)
		pixel_values[i] = pixels[i];

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbedit_set_scaling(int *brdr, int sh_time) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		for (int i = 0; i < 4; i++)
			fprintf(stderr, "dbg2       brdr[%d]:     %d\n", i, brdr[i]);
		fprintf(stderr, "dbg2       show_time:      %d\n", sh_time);
	}

	/* set graphics bounds */
	for (int i = 0; i < 4; i++)
		borders[i] = brdr[i];

	/* set scaling */
	show_time = sh_time;
	if (show_time > MBEDIT_PLOT_WIDE) {
		margin = (borders[1] - borders[0]) / 16;
		xmin = 5 * margin;
		xmax = borders[1] - margin;
		ymin = margin;
		ymax = borders[3] - margin / 2;
		xscale = 100.0 * plot_width / (xmax - xmin);
		yscale = (xscale * exager) / 100.0;
	}
	else {
		margin = (borders[1] - borders[0]) / 16;
		xmin = 2 * margin + 20;
		xmax = borders[1] - margin;
		ymin = margin;
		ymax = borders[3] - margin / 2;
		xscale = 100.0 * plot_width / (xmax - xmin);
		yscale = (xscale * exager) / 100.0;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_set_filters(int f_m, int f_m_t, int f_m_x, int f_m_l, int f_w, int f_w_t, int f_b, int f_b_b, int f_b_e, int f_d,
                       double f_d_b, double f_d_e, int f_a, double f_a_b, double f_a_e) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2       f_m:     %d\n", f_m);
		fprintf(stderr, "dbg2       f_m_t:   %d\n", f_m_t);
		fprintf(stderr, "dbg2       f_m_x:   %d\n", f_m_x);
		fprintf(stderr, "dbg2       f_m_l:   %d\n", f_m_l);
		fprintf(stderr, "dbg2       f_w:     %d\n", f_w);
		fprintf(stderr, "dbg2       f_w_t:   %d\n", f_w_t);
		fprintf(stderr, "dbg2       f_b:     %d\n", f_b);
		fprintf(stderr, "dbg2       f_b_b:   %d\n", f_b_b);
		fprintf(stderr, "dbg2       f_b_e:   %d\n", f_b_e);
		fprintf(stderr, "dbg2       f_d:     %d\n", f_d);
		fprintf(stderr, "dbg2       f_d_b:   %f\n", f_d_b);
		fprintf(stderr, "dbg2       f_d_e:   %f\n", f_d_e);
		fprintf(stderr, "dbg2       f_a:     %d\n", f_a);
		fprintf(stderr, "dbg2       f_a_b:   %f\n", f_a_b);
		fprintf(stderr, "dbg2       f_a_e:   %f\n", f_a_e);
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

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_get_filters(int *b_m, double *d_m, int *f_m, int *f_m_t, int *f_m_x, int *f_m_l, int *f_w, int *f_w_t, int *f_b,
                       int *f_b_b, int *f_b_e, int *f_d, double *f_d_b, double *f_d_e, int *f_a, double *f_a_b, double *f_a_e) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2       b_m:     %p\n", b_m);
		fprintf(stderr, "dbg2       d_m:     %p\n", d_m);
		fprintf(stderr, "dbg2       f_m:     %p\n", f_m);
		fprintf(stderr, "dbg2       f_m_t:   %p\n", f_m_t);
		fprintf(stderr, "dbg2       f_m_x:   %p\n", f_m_x);
		fprintf(stderr, "dbg2       f_m_l:   %p\n", f_m_l);
		fprintf(stderr, "dbg2       f_w:     %p\n", f_w);
		fprintf(stderr, "dbg2       f_w_t:   %p\n", f_w_t);
		fprintf(stderr, "dbg2       f_b:     %p\n", f_b);
		fprintf(stderr, "dbg2       f_b_b:   %p\n", f_b_b);
		fprintf(stderr, "dbg2       f_b_e:   %p\n", f_b_e);
		fprintf(stderr, "dbg2       f_d:     %p\n", f_d);
		fprintf(stderr, "dbg2       f_d_b:   %p\n", f_d_b);
		fprintf(stderr, "dbg2       f_d_e:   %p\n", f_d_e);
		fprintf(stderr, "dbg2       f_a:     %p\n", f_a);
		fprintf(stderr, "dbg2       f_a_b:   %p\n", f_a_b);
		fprintf(stderr, "dbg2       f_a_e:   %p\n", f_a_e);
	}

	/* set max beam number and acrosstrack distance */
	*b_m = 0;
	*d_m = 0.0;
	if (file_open) {
		/* loop over all pings */
		for (int i = 0; i < nbuff; i++) {
			for (int j = 0; j < ping[i].beams_bath; j++) {
				if (mb_beam_ok(ping[i].beamflag[j])) {
					*b_m = MAX(*b_m, ping[i].beams_bath);
					*d_m = MAX(*d_m, fabs(ping[i].bathacrosstrack[j]));
					;
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

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       b_m:     %d\n", *b_m);
		fprintf(stderr, "dbg2       d_m:     %f\n", *d_m);
		fprintf(stderr, "dbg2       f_m:     %d\n", *f_m);
		fprintf(stderr, "dbg2       f_m_t:   %d\n", *f_m_t);
		fprintf(stderr, "dbg2       f_m_x:   %d\n", *f_m_x);
		fprintf(stderr, "dbg2       f_m_l:   %d\n", *f_m_l);
		fprintf(stderr, "dbg2       f_w:     %d\n", *f_w);
		fprintf(stderr, "dbg2       f_w_t:   %d\n", *f_w_t);
		fprintf(stderr, "dbg2       f_b:     %d\n", *f_b);
		fprintf(stderr, "dbg2       f_b_b:   %d\n", *f_b_b);
		fprintf(stderr, "dbg2       f_b_e:   %d\n", *f_b_e);
		fprintf(stderr, "dbg2       f_d:     %d\n", *f_d);
		fprintf(stderr, "dbg2       f_d_b:   %f\n", *f_d_b);
		fprintf(stderr, "dbg2       f_d_e:   %f\n", *f_d_e);
		fprintf(stderr, "dbg2       f_a:     %d\n", *f_a);
		fprintf(stderr, "dbg2       f_a_b:   %f\n", *f_a_b);
		fprintf(stderr, "dbg2       f_a_e:   %f\n", *f_a_e);
		fprintf(stderr, "dbg2       error:   %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbedit_get_defaults(int *plt_size_max, int *plt_size, int *sh_mode, int *sh_flggdsdg, int *sh_flggdprf, int *sh_time, int *buffer_size_max,
                        int *buffer_size, int *hold_size, int *form, int *plwd, int *exgr, int *xntrvl, int *yntrvl, int *ttime_i,
                        int *outmode) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
	}

	/* get maximum number of pings to plot */
	*plt_size_max = MBEDIT_MAX_PINGS;
	*plt_size = plot_size;

	/* get show mode flag */
	*sh_mode = show_mode;

	/* get show flagged soundings flag */
	*sh_flggdsdg = show_flaggedsoundings;

	/* get show flagged profiles flag */
	*sh_flggdprf = show_flaggedprofiles;

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
	if (file_open && nbuff > 0) {
		for (int i = 0; i < 7; i++)
			ttime_i[i] = ping[0].time_i[i];
	} else {
		for (int i = 0; i < 7; i++)
			ttime_i[i] = btime_i[i];
	}

	/* get output mode */
	*outmode = output_mode;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       plot max:               %d\n", *plt_size_max);
		fprintf(stderr, "dbg2       plot_size:              %d\n", *plt_size);
		fprintf(stderr, "dbg2       show_mode:              %d\n", *sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:  %d\n", *sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:   %d\n", *sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:              %d\n", *sh_time);
		fprintf(stderr, "dbg2       buffer max:             %d\n", *buffer_size_max);
		fprintf(stderr, "dbg2       buffer_size:            %d\n", *buffer_size);
		fprintf(stderr, "dbg2       hold_size:              %d\n", *hold_size);
		fprintf(stderr, "dbg2       format:                 %d\n", *form);
		fprintf(stderr, "dbg2       plot_width:             %d\n", *plwd);
		fprintf(stderr, "dbg2       exager:                 %d\n", *exgr);
		fprintf(stderr, "dbg2       x_interval:             %d\n", *xntrvl);
		fprintf(stderr, "dbg2       y_interval:             %d\n", *yntrvl);
		for (int i = 0; i < 7; i++)
			fprintf(stderr, "dbg2       ttime[%d]:               %d\n", i, ttime_i[i]);
		fprintf(stderr, "dbg2       outmode:                %d\n", *outmode);
		fprintf(stderr, "dbg2       error:                  %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                 %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_get_viewmode(int *vw_mode) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
	}

	/* get view mode */
	*vw_mode = view_mode;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       view_mode:   %d\n", *vw_mode);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_set_viewmode(int vw_mode) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       view_mode:   %d\n", vw_mode);
	}

	/* get view mode */
	view_mode = vw_mode;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_open(char *file, int form, int fileid, int numfiles, int savemode, int outmode, int plwd, int exgr, int xntrvl,
                       int yntrvl, int plt_size, int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time, int *buffer_size, int *buffer_size_max,
                       int *hold_size, int *ndumped, int *nloaded, int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       file:            %s\n", file);
		fprintf(stderr, "dbg2       format:          %d\n", form);
		fprintf(stderr, "dbg2       fileid:          %d\n", fileid);
		fprintf(stderr, "dbg2       numfiles:        %d\n", numfiles);
		fprintf(stderr, "dbg2       savemode:        %d\n", savemode);
		fprintf(stderr, "dbg2       outmode:         %d\n", outmode);
		fprintf(stderr, "dbg2       plot_width:      %d\n", plwd);
		fprintf(stderr, "dbg2       exager:          %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:      %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:      %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:       %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:       %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:       %d\n", sh_time);
		fprintf(stderr, "dbg2       buffer_size:     %d\n", *buffer_size);
		fprintf(stderr, "dbg2       buffer_size_max: %d\n", *buffer_size_max);
		fprintf(stderr, "dbg2       hold_size:       %d\n", *hold_size);
	}

	/* reset info */
	info_set = false;

	/* set the output mode */
	output_mode = outmode;

	/* clear the screen */
	int status = mbedit_clear_screen();

	/* open the file */
	status = mbedit_open_file(file, form, savemode);

	/* check buffer size */
	if (status == MB_SUCCESS) {
		if (*hold_size > *buffer_size)
			*hold_size = *buffer_size / 2;
		buff_size = *buffer_size;
		buff_size_max = *buffer_size_max;
		holdd_size = *hold_size;
	}

	/* load the buffer */
	if (status == MB_SUCCESS) {
		status = mbedit_load_data(*buffer_size, nloaded, nbuffer, ngood, icurrent);

		/* if no data read show error dialog */
		if (*nloaded == 0)
			do_error_dialog("No data were loaded from the input", "file. You may have specified an",
			                "incorrect MB-System format id!");
	}

	/* set up plotting */
	if (status == MB_SUCCESS && *ngood > 0) {
		/* turn file button off */
		do_filebutton_off();

		/* now plot it */
		status = mbedit_plot_all(plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf, sh_time, nplt, true);

		/* set fileid and numfiles */
		file_id = fileid;
		num_files = numfiles;
	}

	/* reset beam_save */
	beam_save = false;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       buffer_size:     %d\n", *buffer_size);
		fprintf(stderr, "dbg2       buffer_size_max: %d\n", *buffer_size_max);
		fprintf(stderr, "dbg2       hold_size:       %d\n", *hold_size);
		fprintf(stderr, "dbg2       ndumped:         %d\n", *ndumped);
		fprintf(stderr, "dbg2       nloaded:         %d\n", *nloaded);
		fprintf(stderr, "dbg2       nbuffer:         %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:           %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:        %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplot:           %d\n", *nplt);
		fprintf(stderr, "dbg2       error:           %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_next_buffer(int hold_size, int buffer_size, int plwd, int exgr, int xntrvl, int yntrvl, int plt_size,
                              int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time, int *ndumped, int *nloaded, int *nbuffer, int *ngood,
                              int *icurrent, int *nplt, int *quit) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       hold_size:   %d\n", hold_size);
		fprintf(stderr, "dbg2       buffer_size: %d\n", buffer_size);
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	/* reset info */
	info_set = false;

	int status = mbedit_clear_screen();

	*quit = false;

	/* check if a file has been opened */
	if (file_open) {
		/* set buffer size */
		buff_size = buffer_size;
		holdd_size = hold_size;

		/* keep going until good data or end of file found */
		do {
			/* dump the buffer */
			status = mbedit_dump_data(hold_size, ndumped, nbuffer);

			/* load the buffer */
			status = mbedit_load_data(buffer_size, nloaded, nbuffer, ngood, icurrent);
		} while (*nloaded > 0 && *ngood == 0);

		/* if end of file reached then
		    dump last buffer and close file */
		if (*nloaded <= 0) {
			const int save_dumped = *ndumped;
			status = mbedit_dump_data(0, ndumped, nbuffer);
			status = mbedit_close_file();
			*ndumped = *ndumped + save_dumped;
			*nplt = 0;

			/* if in normal mode last next_buffer
			    does not mean quit,
			    if in gui mode it does mean quit */
			if (gui_mode)
				*quit = true;
			else
				*quit = false;

			/* if quitting let the world know... */
			if (*quit && verbose >= 1)
				fprintf(stderr, "\nQuitting MBedit\nBye Bye...\n");
		} else {
			// else set up plotting
			status = mbedit_plot_all(plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf, sh_time, nplt, true);
		}
	} else {
		// if no file open set failure status
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
	beam_save = false;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ndumped:     %d\n", *ndumped);
		fprintf(stderr, "dbg2       nloaded:     %d\n", *nloaded);
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplot:       %d\n", *nplt);
		fprintf(stderr, "dbg2       quit:        %d\n", *quit);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_close(int buffer_size, int *ndumped, int *nloaded, int *nbuffer, int *ngood, int *icurrent) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       buffer_size: %d\n", buffer_size);
	}

	/* reset info */
	info_set = false;

	int status = mbedit_clear_screen();

	/* if file has been opened and in browse mode
	    just dump the current buffer and close the file */
	int save_nloaded = 0;
	int save_ndumped = 0;
	if (file_open && (output_mode == MBEDIT_OUTPUT_BROWSE || (output_mode == MBEDIT_OUTPUT_EDIT && esf.nedit == 0))) {

		/* dump the buffer */
		status = mbedit_dump_data(0, ndumped, nbuffer);
		save_ndumped = save_ndumped + *ndumped;
		*ndumped = save_ndumped;
		*nloaded = save_nloaded;

		/* now close the file */
		status = mbedit_close_file();
	}

	/* if file has been opened deal with all of the data */
	else if (file_open) {

		/* dump and load until the end of the file is reached */
		do {
			/* dump the buffer */
			status = mbedit_dump_data(0, ndumped, nbuffer);
			save_ndumped = save_ndumped + *ndumped;

			/* load the buffer */
			status = mbedit_load_data(buffer_size, nloaded, nbuffer, ngood, icurrent);
			save_nloaded = save_nloaded + *nloaded;
		} while (*nloaded > 0);
		*ndumped = save_ndumped;
		*nloaded = save_nloaded;

		/* now close the file */
		status = mbedit_close_file();
	}

	else {
		*ndumped = 0;
		*nloaded = 0;
		*nbuffer = 0;
		*ngood = 0;
		*icurrent = 0;
		status = MB_FAILURE;
	}

	/* reset beam_save */
	beam_save = false;

	/* let the world know... */
	if (verbose >= 1) {
		fprintf(stderr, "\nLast ping viewed: %s\n", last_ping);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ndumped:     %d\n", *ndumped);
		fprintf(stderr, "dbg2       nloaded:     %d\n", *nloaded);
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_done(int buffer_size, int *ndumped, int *nloaded, int *nbuffer, int *ngood, int *icurrent, int *quit) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       buffer_size: %d\n", buffer_size);
	}

	/* reset info */
	info_set = false;

	/* if in normal mode done does not mean quit,
	    if in gui mode done does mean quit */
	if (gui_mode)
		*quit = true;
	else
		*quit = false;

	/* if quitting let the world know... */
	if (*quit && verbose >= 1)
		fprintf(stderr, "\nShutting MBedit down without further ado...\n");

	int status = MB_SUCCESS;

	/* call routine to deal with saving the current file, if any */
	if (file_open)
		status = mbedit_action_close(buffer_size, ndumped, nloaded, nbuffer, ngood, icurrent);

	/* if quitting let the world know... */
	if (*quit && verbose >= 1)
		fprintf(stderr, "\nQuitting MBedit\nBye Bye...\n");

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ndumped:     %d\n", *ndumped);
		fprintf(stderr, "dbg2       nloaded:     %d\n", *nloaded);
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       quit:        %d\n", *quit);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_quit(int buffer_size, int *ndumped, int *nloaded, int *nbuffer, int *ngood, int *icurrent) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       buffer_size: %d\n", buffer_size);
	}

	if (verbose >= 1)
		fprintf(stderr, "\nShutting MBedit down without further ado...\n");

	/* reset info */
	info_set = false;

	int status = MB_SUCCESS;

	/* call routine to deal with saving the current file, if any */
	if (file_open)
		status = mbedit_action_close(buffer_size, ndumped, nloaded, nbuffer, ngood, icurrent);

	if (verbose >= 1)
		fprintf(stderr, "\nQuitting MBedit\nBye Bye...\n");

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ndumped:     %d\n", *ndumped);
		fprintf(stderr, "dbg2       nloaded:     %d\n", *nloaded);
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_step(int step, int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time,
                       int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       step:        %d\n", step);
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	/* reset info */
	info_set = false;

	int status = MB_SUCCESS;

	/* check if a file has been opened and there are data */
	if (file_open && nbuff > 0) {
		/* figure out if stepping is possible */
		const int old_id = current_id;
		int new_id = current_id + step;
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
		if (*ngood > 0) {
			status = mbedit_plot_all(plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf, sh_time, nplt, false);
		}

		/* set failure flag if no step was made */
		if (new_id == old_id)
			status = MB_FAILURE;
	}

	/* if no file open set failure status */
	else {
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
	}

	/* reset beam_save */
	beam_save = false;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_plot(int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time,
                       int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	mbedit_clear_screen();

	int status = MB_SUCCESS;

	/* check if a file has been opened */
	if (file_open) {

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* set the plotting list */
		if (*ngood > 0) {
			status = mbedit_plot_all(plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf, sh_time, nplt, false);
		}
	} else {
		// if no file open set failure statu
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_mouse_toggle(int x_loc, int y_loc, int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode,
                               int sh_flggdsdg, int sh_flggdprf, int sh_time, int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       x_loc:       %d\n", x_loc);
		fprintf(stderr, "dbg2       y_loc:       %d\n", y_loc);
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	/* do nothing unless file has been opened */
	bool zap_box = false;
	if (file_open) {
		/* check if a zap box has been picked */
		// zap_box = false;
		int zap_ping = 0;
		for (int i = current_id; i < current_id + nplot; i++) {
			if (ping[i].outbounds == MBEDIT_OUTBOUNDS_UNFLAGGED) {
				if (x_loc >= ping[i].zap_x1 && x_loc <= ping[i].zap_x2 && y_loc >= ping[i].zap_y1 && y_loc <= ping[i].zap_y2) {
					zap_box = true;
					zap_ping = i;
				}
			}
		}

		/* if a zap box has been picked call zap routine */
		if (zap_box)
			status = mbedit_action_zap_outbounds(zap_ping, plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf, sh_time,
			                                     nbuffer, ngood, icurrent, nplt);
	}

	/* do not look for beam pick unless file has been opened
	    and no zap box was picked */
	if (file_open && !zap_box) {
		/* check if a beam has been picked */
		int iping = 0;
		int jbeam = 0;
		int range_min = 100000;
		for (int i = current_id; i < current_id + nplot; i++) {
			for (int j = 0; j < ping[i].beams_bath; j++) {
				if (!mb_beam_check_flag_unusable2(ping[i].beamflag[j])) {
					const int ix = x_loc - ping[i].bath_x[j];
					const int iy = y_loc - ping[i].bath_y[j];
					const int range = (int)sqrt((double)(ix * ix + iy * iy));
					if (range < range_min) {
						range_min = range;
						iping = i;
						jbeam = j;
					}
				}
			}
		}

		/* check to see if closest beam is
		    close enough to be toggled */
		bool found;
		if (range_min <= MBEDIT_PICK_DISTANCE)
			found = true;
		else
			found = false;

		/* unplot the affected beam and ping */
		if (found && *ngood > 0) {
			status = mbedit_unplot_ping(iping);
			status = mbedit_unplot_beam(iping, jbeam);
		}

		/* reset picked beam */
		if (found) {
			/* write edit to save file */
			if (esffile_open) {
				if (mb_beam_ok(ping[iping].beamflag[jbeam]))
					mb_ess_save(verbose, &esf, ping[iping].time_d, jbeam + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
					            MBP_EDIT_FLAG, &error);
				else if (!mb_beam_check_flag_unusable2(ping[iping].beamflag[jbeam]))
					mb_ess_save(verbose, &esf, ping[iping].time_d, jbeam + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
					            MBP_EDIT_UNFLAG, &error);
			}

			/* apply edit */
			if (mb_beam_ok(ping[iping].beamflag[jbeam])) {
				/* reset the beam value - if beam was originally flagged
				    then reset to the original flag value */
				if (mb_beam_ok(ping[iping].beamflagorg[jbeam]))
					ping[iping].beamflag[jbeam] = mb_beam_set_flag_manual(ping[iping].beamflag[jbeam]);
				else
					ping[iping].beamflag[jbeam] = ping[iping].beamflagorg[jbeam];
				if (verbose >= 1) {
					fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping, jbeam, ping[iping].bath[jbeam]);
					fprintf(stderr, " flagged\n");
				}
			}
			else if (!mb_beam_check_flag_unusable2(ping[iping].beamflag[jbeam])) {
				ping[iping].beamflag[jbeam] = mb_beam_set_flag_none(ping[iping].beamflag[jbeam]);
				if (verbose >= 1) {
					fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping, jbeam, ping[iping].bath[jbeam]);
					fprintf(stderr, " unflagged\n");
				}
			}
			beam_save = true;
			iping_save = iping;
			jbeam_save = jbeam;
		}

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping);
		status = mbedit_plot_beam(iping, jbeam - 1);
		status = mbedit_plot_beam(iping, jbeam);
		status = mbedit_plot_beam(iping, jbeam + 1);

		/* if beam out of bounds replot label */
		if (ping[iping].bath_x[jbeam] < xmin || ping[iping].bath_x[jbeam] > xmax || ping[iping].bath_y[jbeam] < ymin ||
		    ping[iping].bath_y[jbeam] > ymax)
			status = mbedit_plot_ping_label(iping, false);
	} else if (!file_open) {
		// if no file open set failure status
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_mouse_pick(int x_loc, int y_loc, int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode,
                             int sh_flggdsdg, int sh_flggdprf, int sh_time, int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       x_loc:       %d\n", x_loc);
		fprintf(stderr, "dbg2       y_loc:       %d\n", y_loc);
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	/* do nothing unless file has been opened */
	bool zap_box = false;
	if (file_open) {
		/* check if a zap box has been picked */
		// zap_box = false;
		int zap_ping = 0;
		for (int i = current_id; i < current_id + nplot; i++) {
			if (ping[i].outbounds == MBEDIT_OUTBOUNDS_UNFLAGGED) {
				if (x_loc >= ping[i].zap_x1 && x_loc <= ping[i].zap_x2 && y_loc >= ping[i].zap_y1 && y_loc <= ping[i].zap_y2) {
					zap_box = true;
					zap_ping = i;
				}
			}
		}

		/* if a zap box has been picked call zap routine */
		if (zap_box)
			status = mbedit_action_zap_outbounds(zap_ping, plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf, sh_time,
			                                     nbuffer, ngood, icurrent, nplt);
	}

	/* do not look for beam pick unless file has been opened
	    and no zap box was picked */
	if (file_open && !zap_box) {
		/* check if a beam has been picked */
		int iping = 0;
		int jbeam = 0;
		int range_min = 100000;
		for (int i = current_id; i < current_id + nplot; i++) {
			for (int j = 0; j < ping[i].beams_bath; j++) {
				if (mb_beam_ok(ping[i].beamflag[j])) {
					const int ix = x_loc - ping[i].bath_x[j];
					const int iy = y_loc - ping[i].bath_y[j];
					const int range = (int)sqrt((double)(ix * ix + iy * iy));
					if (range < range_min) {
						range_min = range;
						iping = i;
						jbeam = j;
					}
				}
			}
		}

		/* check to see if closest beam is
		    close enough to be picked */
		bool found = range_min <= MBEDIT_PICK_DISTANCE;

		/* unplot the affected beam and ping */
		if (found && *ngood > 0) {
			status = mbedit_unplot_ping(iping);
			status = mbedit_unplot_beam(iping, jbeam);
		}

		/* reset picked beam */
		if (found) {
			/* write edit to save file */
			if (esffile_open) {
				mb_ess_save(verbose, &esf, ping[iping].time_d, jbeam + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
				            MBP_EDIT_FLAG, &error);
			}

			/* reset the beam value - if beam was originally flagged
			    then reset to the original flag value */
			if (mb_beam_ok(ping[iping].beamflagorg[jbeam]))
				ping[iping].beamflag[jbeam] = mb_beam_set_flag_manual(ping[iping].beamflag[jbeam]);
			else
				ping[iping].beamflag[jbeam] = ping[iping].beamflagorg[jbeam];
			if (verbose >= 1) {
				fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping, jbeam, ping[iping].bath[jbeam]);
				fprintf(stderr, " flagged\n");
			}
			beam_save = true;
			iping_save = iping;
			jbeam_save = jbeam;
		}

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping);
		status = mbedit_plot_beam(iping, jbeam - 1);
		status = mbedit_plot_beam(iping, jbeam);
		status = mbedit_plot_beam(iping, jbeam + 1);

		/* if beam out of bounds replot label */
		if (ping[iping].bath_x[jbeam] < xmin || ping[iping].bath_x[jbeam] > xmax || ping[iping].bath_y[jbeam] < ymin ||
		    ping[iping].bath_y[jbeam] > ymax)
			status = mbedit_plot_ping_label(iping, false);
	} else if (!file_open) {
		// if no file open set failure status
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_mouse_erase(int x_loc, int y_loc, int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode,
                              int sh_flggdsdg, int sh_flggdprf, int sh_time, int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       x_loc:       %d\n", x_loc);
		fprintf(stderr, "dbg2       y_loc:       %d\n", y_loc);
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	/* do nothing unless file has been opened */
	bool zap_box = false;
	if (file_open) {
		/* check if a zap box has been picked */
		// zap_box = false;
		for (int i = current_id; i < current_id + nplot; i++) {
			if (ping[i].outbounds == MBEDIT_OUTBOUNDS_UNFLAGGED) {
				if (x_loc >= ping[i].zap_x1 && x_loc <= ping[i].zap_x2 && y_loc >= ping[i].zap_y1 && y_loc <= ping[i].zap_y2) {
					zap_box = true;
					const int zap_ping = i;

					/* if a zap box has been picked call zap routine */
					status = mbedit_action_zap_outbounds(zap_ping, plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf,
					                                     sh_time, nbuffer, ngood, icurrent, nplt);
				}
			}
		}
	}

	/* do not look for beam erase unless file has been opened
	    and no zap box was picked */
	if (file_open && !zap_box) {
		/* look for beams to be erased */
		for (int i = current_id; i < current_id + nplot; i++) {
			bool found = false;
			bool replot_label = false;
			for (int j = 0; j < ping[i].beams_bath; j++) {
				if (mb_beam_ok(ping[i].beamflag[j])) {
					const int ix = x_loc - ping[i].bath_x[j];
					const int iy = y_loc - ping[i].bath_y[j];
					const int range = (int)sqrt((double)(ix * ix + iy * iy));
					if (range < MBEDIT_ERASE_DISTANCE && * ngood > 0) {
						/* write edit to save file */
						if (esffile_open) {
							mb_ess_save(verbose, &esf, ping[i].time_d, j + ping[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
							            MBP_EDIT_FLAG, &error);
						}

						/* unplot the affected beam and ping */
						status = mbedit_unplot_ping(i);
						status = mbedit_unplot_beam(i, j);

						/* reset the beam value - if beam was originally flagged
						    then reset to the original flag value */
						if (mb_beam_ok(ping[i].beamflagorg[j]))
							ping[i].beamflag[j] = mb_beam_set_flag_manual(ping[i].beamflag[j]);
						else
							ping[i].beamflag[j] = ping[i].beamflagorg[j];
						if (verbose >= 1) {
							fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", i, j, ping[i].bath[j]);
							fprintf(stderr, " flagged\n");
						}

						/* replot the affected beams */
						found = true;
						beam_save = true;
						iping_save = i;
						jbeam_save = j;
						status = mbedit_plot_beam(i, j - 1);
						status = mbedit_plot_beam(i, j);
						status = mbedit_plot_beam(i, j + 1);

						/* if beam out of bounds replot label */
						if (ping[i].bath_x[j] < xmin || ping[i].bath_x[j] > xmax || ping[i].bath_y[j] < ymin ||
						    ping[i].bath_y[j] > ymax)
							replot_label = true;
					}
				}
			}

			/* replot affected ping */
			if (found && *ngood > 0)
				status = mbedit_plot_ping(i);
			if (replot_label)
				status = mbedit_plot_ping_label(i, false);
		}

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;
	} else if (!file_open) {
		// if no file open set failure status
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_mouse_restore(int x_loc, int y_loc, int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode,
                                int sh_flggdsdg, int sh_flggdprf, int sh_time, int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       x_loc:       %d\n", x_loc);
		fprintf(stderr, "dbg2       y_loc:       %d\n", y_loc);
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	bool zap_box = false;

	/* do nothing unless file has been opened */
	if (file_open) {
		/* check if a zap box has been picked */
		// zap_box = false;
		for (int i = current_id; i < current_id + nplot; i++) {
			if (ping[i].outbounds == MBEDIT_OUTBOUNDS_UNFLAGGED) {
				if (x_loc >= ping[i].zap_x1 && x_loc <= ping[i].zap_x2 && y_loc >= ping[i].zap_y1 && y_loc <= ping[i].zap_y2) {
					zap_box = true;
					const int zap_ping = i;

					/* if a zap box has been picked call zap routine */
					status = mbedit_action_zap_outbounds(zap_ping, plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf,
					                                     sh_time, nbuffer, ngood, icurrent, nplt);
				}
			}
		}
	}

	/* do not look for beam restore unless file has been opened
	    and no zap box was picked */
	if (file_open && !zap_box) {

		/* look for beams to be erased */
		for (int i = current_id; i < current_id + nplot; i++) {
			bool found = false;
			bool replot_label = false;
			for (int j = 0; j < ping[i].beams_bath; j++) {
				if (!mb_beam_ok(ping[i].beamflag[j]) && !mb_beam_check_flag_unusable2(ping[i].beamflag[j])) {
					const int ix = x_loc - ping[i].bath_x[j];
					const int iy = y_loc - ping[i].bath_y[j];
					const int range = (int)sqrt((double)(ix * ix + iy * iy));
					if (range < MBEDIT_ERASE_DISTANCE && * ngood > 0) {
						/* write edit to save file */
						if (esffile_open) {
							mb_ess_save(verbose, &esf, ping[i].time_d, j + ping[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
							            MBP_EDIT_UNFLAG, &error);
						}

						/* unplot the affected beam and ping */
						if (!found)
							status = mbedit_unplot_ping(i);
						status = mbedit_unplot_beam(i, j);

						/* reset the beam value */
						if (!mb_beam_ok(ping[i].beamflag[j]) && !mb_beam_check_flag_unusable2(ping[i].beamflag[j]))
							ping[i].beamflag[j] = mb_beam_set_flag_none(ping[i].beamflag[j]);
						if (verbose >= 1) {
							fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", i, j, ping[i].bath[j]);
							fprintf(stderr, " flagged\n");
						}

						/* replot the affected beams */
						found = true;
						beam_save = true;
						iping_save = i;
						jbeam_save = j;
						status = mbedit_plot_beam(i, j - 1);
						status = mbedit_plot_beam(i, j);
						status = mbedit_plot_beam(i, j + 1);

						/* if beam out of bounds replot label */
						if (ping[i].bath_x[j] < xmin || ping[i].bath_x[j] > xmax || ping[i].bath_y[j] < ymin ||
						    ping[i].bath_y[j] > ymax)
							replot_label = true;
					}
				}
			}

			/* replot affected ping */
			if (found && *ngood > 0)
				status = mbedit_plot_ping(i);
			if (replot_label)
				status = mbedit_plot_ping_label(i, false);
		}

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;
	} else if (!file_open) {
		// if no file open set failure status
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_mouse_grab(int grabmode, int x_loc, int y_loc, int plwd, int exgr, int xntrvl, int yntrvl, int plt_size,
                             int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time, int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       grabmode:    %d\n", grabmode);
		fprintf(stderr, "dbg2       x_loc:       %d\n", x_loc);
		fprintf(stderr, "dbg2       y_loc:       %d\n", y_loc);
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;
	// int xgmin, xgmax, ygmin, ygmax;
	// bool found;
	// bool replot_label;

	/* do nothing unless file has been opened */
	if (file_open) {
		/* replot old info beam if needed */
		if (info_set) {
			status = mbedit_unplot_beam(info_ping, info_beam);
			status = mbedit_unplot_info();
			info_set = false;
			status = mbedit_plot_beam(info_ping, info_beam - 1);
			status = mbedit_plot_beam(info_ping, info_beam);
			status = mbedit_plot_beam(info_ping, info_beam + 1);
			status = mbedit_plot_ping(info_ping);
		}

		/* get start of grab rectangle */
		if (grabmode == MBEDIT_GRAB_START) {
			grab_set = true;
			grab_start_x = x_loc;
			grab_start_y = y_loc;
			grab_end_x = x_loc;
			grab_end_y = y_loc;

			int xgmin, xgmax;
			/* get grab rectangle to use */
			if (grab_start_x <= grab_end_x) {
				xgmin = grab_start_x;
				xgmax = grab_end_x;
			} else {
				xgmin = grab_end_x;
				xgmax = grab_start_x;
			}
			int ygmin, ygmax;
			if (grab_start_y <= grab_end_y) {
				ygmin = grab_start_y;
				ygmax = grab_end_y;
			} else {
				ygmin = grab_end_y;
				ygmax = grab_start_y;
			}

			/* draw grab ractangle */
			xg_drawrectangle(mbedit_xgid, xgmin, ygmin, xgmax - xgmin, ygmax - ygmin, pixel_values[RED], XG_SOLIDLINE);
		}

		/* change grab rectangle */
		else if (grabmode == MBEDIT_GRAB_MOVE) {
			/* get grab rectangle to use */
			int xgmin, xgmax;
			if (grab_start_x <= grab_end_x) {
				xgmin = grab_start_x;
				xgmax = grab_end_x;
			} else {
				xgmin = grab_end_x;
				xgmax = grab_start_x;
			}
			int ygmin, ygmax;
			if (grab_start_y <= grab_end_y) {
				ygmin = grab_start_y;
				ygmax = grab_end_y;
			} else {
				ygmin = grab_end_y;
				ygmax = grab_start_y;
			}

			/* undraw old grab rectangle */
			xg_drawrectangle(mbedit_xgid, xgmin, ygmin, xgmax - xgmin, ygmax - ygmin, pixel_values[WHITE], XG_SOLIDLINE);

			/* update grab rectangle */
			grab_set = true;
			grab_end_x = x_loc;
			grab_end_y = y_loc;

			/* get grab rectangle to use */
			if (grab_start_x <= grab_end_x) {
				xgmin = grab_start_x;
				xgmax = grab_end_x;
			}
			else {
				xgmin = grab_end_x;
				xgmax = grab_start_x;
			}
			if (grab_start_y <= grab_end_y) {
				ygmin = grab_start_y;
				ygmax = grab_end_y;
			}
			else {
				ygmin = grab_end_y;
				ygmax = grab_start_y;
			}

			/* draw grab rectangle */
			xg_drawrectangle(mbedit_xgid, xgmin, ygmin, xgmax - xgmin, ygmax - ygmin, pixel_values[RED], XG_SOLIDLINE);

			/* replot beams on bounds of the grab box */
			for (int i = current_id; i < current_id + nplot; i++) {
				bool found = false;
				bool replot_label = false;
				for (int j = 0; j < ping[i].beams_bath; j++) {
					if (!mb_beam_check_flag_unusable2(ping[i].beamflag[j])) {
						if (abs(ping[i].bath_x[j] - xgmin) <= 10 || abs(ping[i].bath_x[j] - xgmax) <= 10 ||
						    abs(ping[i].bath_y[j] - ygmin) <= 10 || abs(ping[i].bath_y[j] - ygmax) <= 10) {
							/* replot the affected beams */
							found = true;
							status = mbedit_plot_beam(i, j);

							/* if beam out of bounds replot label */
							if (ping[i].bath_x[j] < xmin || ping[i].bath_x[j] > xmax || ping[i].bath_y[j] < ymin ||
							    ping[i].bath_y[j] > ymax)
								replot_label = true;
						}
					}
				}

				/* replot affected ping */
				if (found && *ngood > 0)
					status = mbedit_plot_ping(i);
				if (replot_label)
					status = mbedit_plot_ping_label(i, false);
			}
		}

		/* apply grab rectangle */
		else if (grabmode == MBEDIT_GRAB_END) {
			/* get final grab rectangle */
			grab_set = false;
			grab_end_x = x_loc;
			grab_end_y = y_loc;

			int xgmin, xgmax;
			/* get grab rectangle to use */
			if (grab_start_x <= grab_end_x) {
				xgmin = grab_start_x;
				xgmax = grab_end_x;
			} else {
				xgmin = grab_end_x;
				xgmax = grab_start_x;
			}
			int ygmin, ygmax;
			if (grab_start_y <= grab_end_y) {
				ygmin = grab_start_y;
				ygmax = grab_end_y;
			} else {
				ygmin = grab_end_y;
				ygmax = grab_start_y;
			}

			/* check if any zap boxes has been picked */
			// bool zap_box = false;
			for (int i = current_id; i < current_id + nplot; i++) {
				if (ping[i].outbounds == MBEDIT_OUTBOUNDS_UNFLAGGED) {
					if (xgmin <= ping[i].zap_x1 && xgmax >= ping[i].zap_x2 && ygmin <= ping[i].zap_y1 &&
					    ygmax >= ping[i].zap_y2) {
						// zap_box = true;
						const int zap_ping = i;

						/* if a zap box has been picked call zap routine */
						status = mbedit_action_zap_outbounds(zap_ping, plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf,
						                                     sh_time, nbuffer, ngood, icurrent, nplt);
					}
				}
			}

			/* look for beams to be erased */
			for (int i = current_id; i < current_id + nplot; i++) {
				// found = false;
				// replot_label = false;
				for (int j = 0; j < ping[i].beams_bath; j++) {
					if (mb_beam_ok(ping[i].beamflag[j])) {
						if (ping[i].bath_x[j] >= xgmin && ping[i].bath_x[j] <= xgmax && ping[i].bath_y[j] >= ygmin &&
						    ping[i].bath_y[j] <= ygmax && *ngood > 0) {
							/* write edit to save file */
							if (esffile_open) {
								mb_ess_save(verbose, &esf, ping[i].time_d, j + ping[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
								            MBP_EDIT_FLAG, &error);
							}

							/* reset the beam value - if beam was originally flagged
							                        then reset to the original flag value */
							if (mb_beam_ok(ping[i].beamflagorg[j]))
								ping[i].beamflag[j] = mb_beam_set_flag_manual(ping[i].beamflag[j]);
							else
								ping[i].beamflag[j] = ping[i].beamflagorg[j];
							if (verbose >= 1) {
								fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", i, j, ping[i].bath[j]);
								fprintf(stderr, " flagged\n");
							}

							/* replot the affected beams */
							// found = true;
							beam_save = true;
							iping_save = i;
							jbeam_save = j;

							/* if beam out of bounds replot label */
							// if (ping[i].bath_x[j] < xmin || ping[i].bath_x[j] > xmax || ping[i].bath_y[j] < ymin ||
							//     ping[i].bath_y[j] > ymax) {
								// replot_label = true;
							// }
						}
					}
				}
			}

			/* replot everything */
			status = mbedit_plot_all(plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf, sh_time, nplt, false);
		}

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;
	} else /* if (!file_open) */ {
		// if no file open set failure status
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_mouse_info(int x_loc, int y_loc, int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode,
                             int sh_flggdsdg, int sh_flggdprf, int sh_time, int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       x_loc:       %d\n", x_loc);
		fprintf(stderr, "dbg2       y_loc:       %d\n", y_loc);
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* do nothing unless file has been opened */
	if (file_open) {
		/* replot old info beam if needed */
		if (info_set) {
			status = mbedit_unplot_beam(info_ping, info_beam);
			status = mbedit_unplot_info();
			info_set = false;
			status = mbedit_plot_beam(info_ping, info_beam - 1);
			status = mbedit_plot_beam(info_ping, info_beam);
			status = mbedit_plot_beam(info_ping, info_beam + 1);
			status = mbedit_plot_ping(info_ping);
		}

		/* check if a beam has been picked */
		int iping = 0;
		int jbeam = 0;
		int range_min = 100000;
		for (int i = current_id; i < current_id + nplot; i++) {
			for (int j = 0; j < ping[i].beams_bath; j++) {
				if (!mb_beam_check_flag_unusable2(ping[i].beamflag[j])) {
					const int ix = x_loc - ping[i].bath_x[j];
					const int iy = y_loc - ping[i].bath_y[j];
					const int range = (int)sqrt((double)(ix * ix + iy * iy));
					if (range < range_min) {
						range_min = range;
						iping = i;
						jbeam = j;
					}
				}
			}
		}

		/* check to see if closest beam is
		    close enough to be id'd */
		if (range_min <= MBEDIT_PICK_DISTANCE) {
			info_set = true;
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
			info_amp = ping[iping].amp[jbeam];
			info_bathacrosstrack = ping[iping].bathacrosstrack[jbeam];
			info_bathalongtrack = ping[iping].bathalongtrack[jbeam];
			info_detect = ping[iping].detect[jbeam];
			info_priority = ping[iping].priority[jbeam];
			info_pulse = ping[iping].pulses[jbeam];
			/*			fprintf(stderr,"\nping: %d beam:%d depth:%10.3f \n",
			                iping,jbeam,ping[iping].bath[jbeam]);*/

			/* replot old info beam if needed */
			status = mbedit_plot_beam(info_ping, info_beam);
			status = mbedit_plot_info();
		}
		else
			info_set = false;

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;
	} else /* if (!file_open) */ {
		// if no file open set failure status
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_zap_outbounds(int iping, int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode, int sh_flggdsdg, int sh_flggdprf,
                                int sh_time, int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       iping:       %d\n", iping);
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	/* do nothing unless file has been opened */
	if (file_open) {

		/* look for beams to be erased */
		bool found = false;
		for (int j = 0; j < ping[iping].beams_bath; j++) {
			if (mb_beam_ok(ping[iping].beamflag[j]) && (ping[iping].bath_x[j] < xmin || ping[iping].bath_x[j] > xmax ||
			                                            ping[iping].bath_y[j] < ymin || ping[iping].bath_y[j] > ymax)) {
				/* write edit to save file */
				if (esffile_open) {
					mb_ess_save(verbose, &esf, ping[iping].time_d, j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
					            MBP_EDIT_FLAG, &error);
				}

				/* unplot the affected beam and ping */
				status = mbedit_unplot_ping(iping);
				status = mbedit_unplot_beam(iping, j);

				/* reset the beam value - if beam was originally flagged
				    then reset to the original flag value */
				if (mb_beam_ok(ping[iping].beamflagorg[j]))
					ping[iping].beamflag[j] = mb_beam_set_flag_manual(ping[iping].beamflag[j]);
				else
					ping[iping].beamflag[j] = ping[iping].beamflagorg[j];
				if (verbose >= 1) {
					fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping, j, ping[iping].bath[j]);
					fprintf(stderr, " flagged\n");
				}

				/* replot the affected beams */
				found = true;
				beam_save = true;
				iping_save = iping;
				jbeam_save = j;
				status = mbedit_plot_beam(iping, j - 1);
				status = mbedit_plot_beam(iping, j);
				status = mbedit_plot_beam(iping, j + 1);
			}
		}

		/* replot affected ping */
		if (found && *ngood > 0) {
			status = mbedit_plot_ping(iping);
			status = mbedit_plot_ping_label(iping, false);
		}

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;
	} else /* if (!file_open) */ {
		// if no file open set failure status
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_bad_ping(int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time,
                           int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	/* check if a file has been opened
	    and a beam has been picked and saved */
	if (file_open && beam_save) {
		/* write edits to save file */
		if (esffile_open) {
			for (int j = 0; j < ping[iping_save].beams_bath; j++)
				if (mb_beam_ok(ping[iping_save].beamflag[j]))
					mb_ess_save(verbose, &esf, ping[iping_save].time_d,
					            j + ping[iping_save].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FLAG, &error);
		}

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (int j = 0; j < ping[iping_save].beams_bath; j++)
			status = mbedit_unplot_beam(iping_save, j);

		/* flag beams in bad ping */
		if (verbose >= 1)
			fprintf(stderr, "\nbeams in ping: %d flagged\n", iping_save);
		for (int j = 0; j < ping[iping_save].beams_bath; j++) {
			if (mb_beam_ok(ping[iping_save].beamflag[j])) {
				/* reset the beam value - if beam was originally flagged
				    then reset to the original flag value */
				if (mb_beam_ok(ping[iping_save].beamflagorg[j]))
					ping[iping_save].beamflag[j] = mb_beam_set_flag_manual(ping[iping_save].beamflag[j]);
				else
					ping[iping_save].beamflag[j] = ping[iping_save].beamflagorg[j];
				if (verbose >= 1) {
					fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping_save, j, ping[iping_save].bath[j]);
					fprintf(stderr, " flagged\n");
				}
			}
		}

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (int j = 0; j < ping[iping_save].beams_bath; j++)
			status = mbedit_plot_beam(iping_save, j);

		/* if ping has outbounds flag replot label */
		if (ping[iping_save].outbounds != MBEDIT_OUTBOUNDS_NONE)
			status = mbedit_plot_ping_label(iping_save, false);
	}

	/* if no file open or beam saved set failure status */
	else {
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_good_ping(int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time,
                            int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	/* check if a file has been opened
	    and a beam has been picked and saved */
	if (file_open && beam_save) {
		/* write edits to save file */
		if (esffile_open) {
			for (int j = 0; j < ping[iping_save].beams_bath; j++)
				if (!mb_beam_ok(ping[iping_save].beamflag[j]) && !mb_beam_check_flag_unusable2(ping[iping_save].beamflag[j]))
					mb_ess_save(verbose, &esf, ping[iping_save].time_d,
					            j + ping[iping_save].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_UNFLAG, &error);
		}

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (int j = 0; j < ping[iping_save].beams_bath; j++)
			status = mbedit_unplot_beam(iping_save, j);

		/* flag beams in good ping */
		for (int j = 0; j < ping[iping_save].beams_bath; j++)
			if (!mb_beam_ok(ping[iping_save].beamflag[j]) && !mb_beam_check_flag_unusable2(ping[iping_save].beamflag[j]))
				ping[iping_save].beamflag[j] = mb_beam_set_flag_none(ping[iping_save].beamflag[j]);
		if (verbose >= 1)
			fprintf(stderr, "\nbeams in ping: %d unflagged\n", iping_save);

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (int j = 0; j < ping[iping_save].beams_bath; j++)
			status = mbedit_plot_beam(iping_save, j);

		/* if ping has outbounds flag replot label */
		if (ping[iping_save].outbounds != MBEDIT_OUTBOUNDS_NONE)
			status = mbedit_plot_ping_label(iping_save, false);
	}

	/* if no file open or beam saved set failure status */
	else {
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_left_ping(int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time,
                            int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	/* check if a file has been opened
	    and a beam has been picked and saved */
	if (file_open && beam_save) {
		/* write edits to save file */
		if (esffile_open) {
			for (int j = 0; j <= jbeam_save; j++)
				if (mb_beam_ok(ping[iping_save].beamflag[j]))
					mb_ess_save(verbose, &esf, ping[iping_save].time_d,
					            j + ping[iping_save].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FLAG, &error);
		}

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (int j = 0; j < ping[iping_save].beams_bath; j++)
			status = mbedit_unplot_beam(iping_save, j);

		/* flag beams to left of picked beam */
		if (verbose >= 1)
			fprintf(stderr, "\nbeams in ping: %d left of beam: %d flagged\n", iping_save, jbeam_save);
		for (int j = 0; j <= jbeam_save; j++) {
			if (mb_beam_ok(ping[iping_save].beamflag[j])) {
				/* reset the beam value - if beam was originally flagged
				     then reset to the original flag value */
				if (mb_beam_ok(ping[iping_save].beamflagorg[j]))
					ping[iping_save].beamflag[j] = mb_beam_set_flag_manual(ping[iping_save].beamflag[j]);
				else
					ping[iping_save].beamflag[j] = ping[iping_save].beamflagorg[j];
				if (verbose >= 1) {
					fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping_save, j, ping[iping_save].bath[j]);
					fprintf(stderr, " flagged\n");
				}
			}
		}

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (int j = 0; j < ping[iping_save].beams_bath; j++)
			status = mbedit_plot_beam(iping_save, j);

		/* if ping has outbounds flag replot label */
		if (ping[iping_save].outbounds != MBEDIT_OUTBOUNDS_NONE)
			status = mbedit_plot_ping_label(iping_save, false);
	}

	/* if no file open or beam saved set failure status */
	else {
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_right_ping(int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time,
                             int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	/* check if a file has been opened
	    and a beam has been picked and saved */
	if (file_open && beam_save) {
		/* write edits to save file */
		if (esffile_open) {
			for (int j = jbeam_save; j < ping[iping_save].beams_bath; j++)
				if (mb_beam_ok(ping[iping_save].beamflag[j]))
					mb_ess_save(verbose, &esf, ping[iping_save].time_d,
					            j + ping[iping_save].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FLAG, &error);
		}

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (int j = 0; j < ping[iping_save].beams_bath; j++)
			status = mbedit_unplot_beam(iping_save, j);

		/* flag beams to right of picked beam */
		if (verbose >= 1)
			fprintf(stderr, "\nbeams in ping: %d right of beam: %d flagged\n", iping_save, jbeam_save);
		for (int j = jbeam_save; j < ping[iping_save].beams_bath; j++) {
			if (mb_beam_ok(ping[iping_save].beamflag[j])) {
				/* reset the beam value - if beam was originally flagged
				     then reset to the original flag value */
				if (mb_beam_ok(ping[iping_save].beamflagorg[j]))
					ping[iping_save].beamflag[j] = mb_beam_set_flag_manual(ping[iping_save].beamflag[j]);
				else
					ping[iping_save].beamflag[j] = ping[iping_save].beamflagorg[j];
				if (verbose >= 1) {
					fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping_save, j, ping[iping_save].bath[j]);
					fprintf(stderr, " flagged\n");
				}
			}
		}

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (int j = 0; j < ping[iping_save].beams_bath; j++)
			status = mbedit_plot_beam(iping_save, j);

		/* if ping has outbounds flag replot label */
		if (ping[iping_save].outbounds != MBEDIT_OUTBOUNDS_NONE)
			status = mbedit_plot_ping_label(iping_save, false);
	}

	/* if no file open or beam saved set failure status */
	else {
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_zero_ping(int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time,
                            int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	/* check if a file has been opened
	    and a beam has been picked and saved */
	if (file_open && beam_save) {
		/* write edits to save file */
		if (esffile_open) {
			for (int j = 0; j < ping[iping_save].beams_bath; j++) {
				if (!mb_beam_check_flag_unusable2(ping[iping_save].beamflag[j]))
					mb_ess_save(verbose, &esf, ping[iping_save].time_d,
					            j + ping[iping_save].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_ZERO, &error);
			}
		}

		/* unplot the affected beam and ping */
		status = mbedit_unplot_ping(iping_save);
		for (int j = 0; j < ping[iping_save].beams_bath; j++)
			status = mbedit_unplot_beam(iping_save, j);

		/* null beams in bad ping */
		for (int j = 0; j < ping[iping_save].beams_bath; j++) {
			ping[iping_save].beamflag[j] = mb_beam_set_flag_null(ping[iping_save].beamflag[j]);
		}
		if (verbose >= 1)
			fprintf(stderr, "\nbeams in ping: %d nulled\n", iping_save);

		/* set some return values */
		*nbuffer = nbuff;
		*ngood = nbuff;
		*icurrent = current_id;

		/* replot the affected beam and ping */
		status = mbedit_plot_ping(iping_save);
		for (int j = 0; j < ping[iping_save].beams_bath; j++)
			status = mbedit_plot_beam(iping_save, j);

		/* if ping has outbounds flag replot label */
		if (ping[iping_save].outbounds != MBEDIT_OUTBOUNDS_NONE)
			status = mbedit_plot_ping_label(iping_save, false);
	}

	/* if no file open or beam saved set failure status */
	else {
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_flag_view(int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time,
                            int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	/* do nothing unless file has been opened */
	if (file_open) {
		/* flag all unflagged beams */
		for (int i = current_id; i < current_id + nplot; i++) {
			for (int j = 0; j < ping[i].beams_bath; j++) {
				if (mb_beam_ok(ping[i].beamflag[j])) {
					/* write edit to save file */
					if (esffile_open)
						mb_ess_save(verbose, &esf, ping[i].time_d, j + ping[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
						            MBP_EDIT_FLAG, &error);

					/* reset the beam value - if beam was originally flagged
					    then reset to the original flag value */
					if (mb_beam_ok(ping[i].beamflagorg[j]))
						ping[i].beamflag[j] = mb_beam_set_flag_manual(ping[i].beamflag[j]);
					else
						ping[i].beamflag[j] = ping[i].beamflagorg[j];
					if (verbose >= 1) {
						fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", i, j, ping[i].bath[j]);
						fprintf(stderr, " flagged\n");
					}
					beam_save = true;
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
		if (*ngood > 0) {
			status = mbedit_plot_all(plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf, sh_time, nplt, false);
		}
	} else /* if (!file_open) */ {
		// if no file open set failure status
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_unflag_view(int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time,
                              int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	/* do nothing unless file has been opened */
	if (file_open) {
		/* unflag all flagged beams */
		for (int i = current_id; i < current_id + nplot; i++) {
			for (int j = 0; j < ping[i].beams_bath; j++) {
				if (!mb_beam_ok(ping[i].beamflag[j]) && !mb_beam_check_flag_unusable2(ping[i].beamflag[j])) {
					/* write edit to save file */
					if (esffile_open)
						mb_ess_save(verbose, &esf, ping[i].time_d, j + ping[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
						            MBP_EDIT_UNFLAG, &error);

					/* apply edit */
					ping[i].beamflag[j] = mb_beam_set_flag_none(ping[i].beamflag[j]);
					if (verbose >= 1) {
						fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", i, j, ping[i].bath[j]);
						fprintf(stderr, " unflagged\n");
					}
					beam_save = true;
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
		if (*ngood > 0) {
			status = mbedit_plot_all(plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf, sh_time, nplt, false);
		}
	} else /* if (!file_open) */ {
		// if no file open set failure status
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_unflag_all(int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time,
                             int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	/* do nothing unless file has been opened */
	if (file_open) {
		/* unflag all flagged beams from current point in buffer */
		for (int i = current_id; i < nbuff; i++) {
			for (int j = 0; j < ping[i].beams_bath; j++) {
				if (!mb_beam_ok(ping[i].beamflag[j]) && !mb_beam_check_flag_unusable2(ping[i].beamflag[j])) {
					/* write edit to save file */
					if (esffile_open)
						mb_ess_save(verbose, &esf, ping[i].time_d, j + ping[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
						            MBP_EDIT_UNFLAG, &error);

					/* apply edit */
					ping[i].beamflag[j] = mb_beam_set_flag_none(ping[i].beamflag[j]);
					if (verbose >= 1) {
						fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", i, j, ping[i].bath[j]);
						fprintf(stderr, " unflagged\n");
					}
					beam_save = false;
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
		if (*ngood > 0) {
			status = mbedit_plot_all(plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf, sh_time, nplt, false);
		}
	} else /* if (!file_open) */ {
		// if no file open set failure status
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_filter_all(int plwd, int exgr, int xntrvl, int yntrvl, int plt_size, int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time,
                             int *nbuffer, int *ngood, int *icurrent, int *nplt) {
	fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	/* do nothing unless file has been opened */
	if (file_open) {
		do_message_on("MBedit is applying bathymetry filters...");

		/* filter all pings in buffer */
		for (int i = current_id; i < nbuff; i++) {
			mbedit_filter_ping(i);

			/* update message every 250 records */
			if (i % 250 == 0) {
				char string[MB_PATH_MAXLINE];
				sprintf(string, "MBedit: filters applied to %d of %d records so far...", i, nbuff - current_id - 1);
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
		if (*ngood > 0) {
			status = mbedit_plot_all(plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf, sh_time, nplt, false);
		}
	} else /* if (!file_open) */ {
		// if no file open set failure status
		status = MB_FAILURE;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplt:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_filter_ping(int iping) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       iping:       %d\n", iping);
	}

	int status = MB_SUCCESS;

	/* reset info */
	if (info_set) {
		status = mbedit_unplot_beam(info_ping, info_beam);
		status = mbedit_unplot_info();
		info_set = false;
		status = mbedit_plot_beam(info_ping, info_beam - 1);
		status = mbedit_plot_beam(info_ping, info_beam);
		status = mbedit_plot_beam(info_ping, info_beam + 1);
		status = mbedit_plot_ping(info_ping);
	}

	/* do nothing unless file has been opened and filters set on */
	if (file_open && iping >= 0 && iping < nbuff) {
		/* work on good data */
		if (status == MB_SUCCESS) {
			/* clear previous filter flags */
			for (int j = 0; j < ping[iping].beams_bath; j++) {
				if (mb_beam_check_flag_filter2(ping[iping].beamflag[j])) {
					/* write edit to save file */
					if (esffile_open)
						mb_ess_save(verbose, &esf, ping[iping].time_d, j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
						            MBP_EDIT_UNFLAG, &error);

					/* apply edit */
					ping[iping].beamflag[j] = mb_beam_set_flag_none(ping[iping].beamflag[j]);
					if (verbose >= 1) {
						fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping, j, ping[iping].bath[j]);
						fprintf(stderr, " unflagged\n");
					}
				}
			}

			/* apply median filter if desired */
			if (filter_medianspike) {
				/* loop over all beams in the ping */
				for (int jbeam = 0; jbeam < ping[iping].beams_bath; jbeam++) {
					/* calculate median if beam not flagged */
					if (mb_beam_ok(ping[iping].beamflag[jbeam])) {
						int nbathlist = 0;
						double bathmedian = 0.0;
						const int istart = MAX(iping - filter_medianspike_ltrack / 2, 0);
						const int iend = MIN(iping + filter_medianspike_ltrack / 2, nbuff - 1);
						for (int i = istart; i <= iend; i++) {
							const int jstart = MAX(jbeam - filter_medianspike_xtrack / 2, 0);
							const int jend = MIN(jbeam + filter_medianspike_xtrack / 2, ping[iping].beams_bath - 1);
							for (int j = jstart; j <= jend; j++) {
								if (mb_beam_ok(ping[i].beamflag[j])) {
									bathlist[nbathlist] = ping[i].bath[j];
									nbathlist++;
								}
							}
						}
						if (nbathlist > 0) {
							qsort(bathlist, nbathlist, sizeof(double), mb_double_compare);
							bathmedian = bathlist[nbathlist / 2];
						}
						if (100 * fabs(ping[iping].bath[jbeam] - bathmedian) / ping[iping].altitude >
						    filter_medianspike_threshold) {
							/* write edit to save file */
							if (esffile_open)
								mb_ess_save(verbose, &esf, ping[iping].time_d,
								            jbeam + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER,
								            &error);

							/* reset the beam value - if beam was originally flagged
							    then reset to the original flag value */
							if (mb_beam_ok(ping[iping].beamflagorg[jbeam]))
								ping[iping].beamflag[jbeam] = mb_beam_set_flag_filter2(ping[iping].beamflag[jbeam]);
							else
								ping[iping].beamflag[jbeam] = ping[iping].beamflagorg[jbeam];
							if (verbose >= 1) {
								fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping, jbeam, ping[iping].bath[jbeam]);
								fprintf(stderr, " flagged\n");
							}
						}
					}
				}
			}

			/* apply wrongside filter if desired */
			if (filter_wrongside) {
				int start = 0;
				int end = (ping[iping].beams_bath / 2) - filter_wrongside_threshold;
				for (int j = start; j < end; j++) {
					if (mb_beam_ok(ping[iping].beamflag[j]) && ping[iping].bathacrosstrack[j] > 0.0) {
						/* write edit to save file */
						if (esffile_open)
							mb_ess_save(verbose, &esf, ping[iping].time_d,
							            j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);

						/* reset the beam value - if beam was originally flagged
						    then reset to the original flag value */
						if (mb_beam_ok(ping[iping].beamflagorg[j]))
							ping[iping].beamflag[j] = mb_beam_set_flag_filter2(ping[iping].beamflag[j]);
						else
							ping[iping].beamflag[j] = ping[iping].beamflagorg[j];
						if (verbose >= 1) {
							fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping, j, ping[iping].bath[j]);
							fprintf(stderr, " flagged\n");
						}
					}
				}
				start = (ping[iping].beams_bath / 2) + filter_wrongside_threshold;
				end = ping[iping].beams_bath;
				for (int j = start; j < end; j++) {
					if (mb_beam_ok(ping[iping].beamflag[j]) && ping[iping].bathacrosstrack[j] < 0.0) {
						/* write edit to save file */
						if (esffile_open)
							mb_ess_save(verbose, &esf, ping[iping].time_d,
							            j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);

						/* reset the beam value - if beam was originally flagged
						    then reset to the original flag value */
						if (mb_beam_ok(ping[iping].beamflagorg[j]))
							ping[iping].beamflag[j] = mb_beam_set_flag_filter2(ping[iping].beamflag[j]);
						else
							ping[iping].beamflag[j] = ping[iping].beamflagorg[j];
						if (verbose >= 1) {
							fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping, j, ping[iping].bath[j]);
							fprintf(stderr, " flagged\n");
						}
					}
				}
			}

			/* apply cut by beam number filter if desired */
			if (filter_cutbeam) {
				/* handle cut inside swath */
				if (filter_cutbeam_begin <= filter_cutbeam_end) {
					const int start = MAX(filter_cutbeam_begin, 0);
					const int end = MIN(filter_cutbeam_end, ping[iping].beams_bath - 1);
					for (int j = start; j < end; j++) {
						if (mb_beam_ok(ping[iping].beamflag[j])) {
							/* write edit to save file */
							if (esffile_open)
								mb_ess_save(verbose, &esf, ping[iping].time_d,
								            j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);

							/* reset the beam value - if beam was originally flagged
							    then reset to the original flag value */
							if (mb_beam_ok(ping[iping].beamflagorg[j]))
								ping[iping].beamflag[j] =
								    mb_beam_set_flag_filter2(ping[iping].beamflag[j]);
							else
								ping[iping].beamflag[j] = ping[iping].beamflagorg[j];
							if (verbose >= 1) {
								fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping, j, ping[iping].bath[j]);
								fprintf(stderr, " flagged\n");
							}
						}
					}
				} else /* if (filter_cutbeam_begin > filter_cutbeam_end) */ {
					/* handle cut at edges of swath */
					for (int j = 0; j < ping[iping].beams_bath; j++) {
						if ((j <= filter_cutbeam_end || j >= filter_cutbeam_begin) && mb_beam_ok(ping[iping].beamflag[j])) {
							/* write edit to save file */
							if (esffile_open)
								mb_ess_save(verbose, &esf, ping[iping].time_d,
								            j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);

							/* reset the beam value - if beam was originally flagged
							    then reset to the original flag value */
							if (mb_beam_ok(ping[iping].beamflagorg[j]))
								ping[iping].beamflag[j] =
								    mb_beam_set_flag_filter2(ping[iping].beamflag[j]);
							else
								ping[iping].beamflag[j] = ping[iping].beamflagorg[j];
							if (verbose >= 1) {
								fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping, j, ping[iping].bath[j]);
								fprintf(stderr, " flagged\n");
							}
						}
					}
				}
			}

			/* apply cut by distance filter if desired */
			if (filter_cutdistance) {
				/* handle cut inside swath */
				if (filter_cutdistance_begin <= filter_cutdistance_end) {
					for (int j = 0; j < ping[iping].beams_bath; j++) {
						if (mb_beam_ok(ping[iping].beamflag[j])) {
							if (ping[iping].bathacrosstrack[j] >= filter_cutdistance_begin &&
							    ping[iping].bathacrosstrack[j] <= filter_cutdistance_end) {
								/* write edit to save file */
								if (esffile_open)
									mb_ess_save(verbose, &esf, ping[iping].time_d,
									            j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER,
									            &error);

								/* reset the beam value - if beam was originally flagged
								    then reset to the original flag value */
								if (mb_beam_ok(ping[iping].beamflagorg[j]))
									ping[iping].beamflag[j] = mb_beam_set_flag_filter2(ping[iping].beamflag[j]);
								else
									ping[iping].beamflag[j] = ping[iping].beamflagorg[j];
								if (verbose >= 1) {
									fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping, j, ping[iping].bath[j]);
									fprintf(stderr, " flagged\n");
								}
							}
						}
					}
				}

				/* handle cut at edges of swath */
				else /* if (filter_cutdistance_begin > filter_cutdistance_end) */ {
					for (int j = 0; j < ping[iping].beams_bath; j++) {
						if (mb_beam_ok(ping[iping].beamflag[j])) {
							if (ping[iping].bathacrosstrack[j] >= filter_cutdistance_begin ||
							    ping[iping].bathacrosstrack[j] <= filter_cutdistance_end) {
								/* write edit to save file */
								if (esffile_open)
									mb_ess_save(verbose, &esf, ping[iping].time_d,
									            j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER,
									            &error);

								/* reset the beam value - if beam was originally flagged
								    then reset to the original flag value */
								if (mb_beam_ok(ping[iping].beamflagorg[j]))
									ping[iping].beamflag[j] = mb_beam_set_flag_filter2(ping[iping].beamflag[j]);
								else
									ping[iping].beamflag[j] = ping[iping].beamflagorg[j];
								if (verbose >= 1) {
									fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping, j, ping[iping].bath[j]);
									fprintf(stderr, " flagged\n");
								}
							}
						}
					}
				}
			}

			/* apply cut by angle filter if desired */
			if (filter_cutangle) {
				/* handle cut inside swath */
				if (filter_cutangle_begin <= filter_cutangle_end) {
					for (int j = 0; j < ping[iping].beams_bath; j++) {
						if (mb_beam_ok(ping[iping].beamflag[j]) && ping[iping].altitude > 0.0) {
							const double angle = RTD * atan(ping[iping].bathacrosstrack[j] / ping[iping].altitude);
							if (angle >= filter_cutangle_begin && angle <= filter_cutangle_end) {
								/* write edit to save file */
								if (esffile_open)
									mb_ess_save(verbose, &esf, ping[iping].time_d,
									            j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER,
									            &error);

								/* reset the beam value - if beam was originally flagged
								    then reset to the original flag value */
								if (mb_beam_ok(ping[iping].beamflagorg[j]))
									ping[iping].beamflag[j] = mb_beam_set_flag_filter2(ping[iping].beamflag[j]);
								else
									ping[iping].beamflag[j] = ping[iping].beamflagorg[j];
								if (verbose >= 1) {
									fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping, j, ping[iping].bath[j]);
									fprintf(stderr, " flagged\n");
								}
							}
						}
					}
				} else /* if (filter_cutangle_begin > filter_cutangle_end) */ {
					/* handle cut at edges of swath */
					for (int j = 0; j < ping[iping].beams_bath; j++) {
						if (mb_beam_ok(ping[iping].beamflag[j]) && ping[iping].altitude > 0.0) {
							const double angle = RTD * atan(ping[iping].bathacrosstrack[j] / ping[iping].altitude);
							if (angle >= filter_cutangle_begin || angle <= filter_cutangle_end) {
								/* write edit to save file */
								if (esffile_open)
									mb_ess_save(verbose, &esf, ping[iping].time_d,
									            j + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER,
									            &error);

								/* reset the beam value - if beam was originally flagged
								    then reset to the original flag value */
								if (mb_beam_ok(ping[iping].beamflagorg[j]))
									ping[iping].beamflag[j] = mb_beam_set_flag_filter2(ping[iping].beamflag[j]);
								else
									ping[iping].beamflag[j] = ping[iping].beamflagorg[j];
								if (verbose >= 1) {
									fprintf(stderr, "\nping: %d beam:%d depth:%10.3f ", iping, j, ping[iping].bath[j]);
									fprintf(stderr, " flagged\n");
								}
							}
						}
					}
				}
			}
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_get_format(char *file, int *form) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       file:        %s\n", file);
		fprintf(stderr, "dbg2       format:      %d\n", *form);
	}

	int status = MB_SUCCESS;
	char tmp[MB_PATH_MAXLINE];
	int tform;

	/* get filenames */
	/* look for MB suffix convention */
	if ((status = mb_get_format(verbose, file, tmp, &tform, &error)) == MB_SUCCESS) {
		*form = tform;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       format:      %d\n", *form);
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_open_file(char *file, int form, bool savemode) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       file:        %s\n", file);
		fprintf(stderr, "dbg2       format:      %d\n", form);
		fprintf(stderr, "dbg2       savemode:    %d\n", savemode);
	}

	int status = MB_SUCCESS;
	char error1[3072] = "";
	char error2[3072] = "";
	char error3[3072] = "";

	/* swath file locking variables */
	bool locked = false;
	int lock_purpose = MBP_LOCK_NONE;
	mb_path lock_program = "";
	mb_path lock_cpu = "";
	mb_path lock_user = "";
	char lock_date[25] = "";

	/* reset message */
	do_message_on("MBedit is opening a data file...");

	/* get filenames */
	strcpy(ifile, file);
	format = form;

	/* try to lock file */
	if (uselockfiles) {
		status = mb_pr_lockswathfile(verbose, ifile, MBP_LOCK_EDITBATHY, program_name, &error);
	}
	else {
		// int lock_status =
		    mb_pr_lockinfo(verbose, ifile, &locked, &lock_purpose, lock_program, lock_user, lock_cpu, lock_date, &error);

		/* if locked get lock info */
		if (error == MB_ERROR_FILE_LOCKED) {
			fprintf(stderr, "\nFile %s locked but lock ignored\n", ifile);
			fprintf(stderr, "File locked by <%s> running <%s>\n", lock_user, lock_program);
			fprintf(stderr, "on cpu <%s> at <%s>\n", lock_cpu, lock_date);
			error = MB_ERROR_NO_ERROR;
		}
	}

	/* if locked let the user know file can't be opened */
	if (status == MB_FAILURE) {
		/* turn off message */
		do_message_off();

		/* if locked get lock info */
		if (error == MB_ERROR_FILE_LOCKED) {
			// int lock_status =
			    mb_pr_lockinfo(verbose, ifile, &locked, &lock_purpose, lock_program, lock_user, lock_cpu, lock_date, &error);

			sprintf(error1, "Unable to open input file:");
			sprintf(error2, "File locked by <%s> running <%s>", lock_user, lock_program);
			sprintf(error3, "on cpu <%s> at <%s>", lock_cpu, lock_date);
			fprintf(stderr, "\nUnable to open input file:\n");
			fprintf(stderr, "  %s\n", ifile);
			fprintf(stderr, "File locked by <%s> running <%s>\n", lock_user, lock_program);
			fprintf(stderr, "on cpu <%s> at <%s>\n", lock_cpu, lock_date);
		}

		/* else if unable to create lock file there is a permissions problem */
		else if (error == MB_ERROR_OPEN_FAIL) {
			sprintf(error1, "Unable to create lock file");
			sprintf(error2, "for intended input file:");
			sprintf(error3, "-Likely permissions issue");
			fprintf(stderr, "Unable to create lock file\n");
			fprintf(stderr, "for intended input file:\n");
			fprintf(stderr, "  %s\n", ifile);
			fprintf(stderr, "-Likely permissions issue\n");
		}

		/* put up error dialog */
		do_error_dialog(error1, error2, error3);
	}

	/* if successfully locked (or lock ignored) proceed */
	if (status == MB_SUCCESS) {
		/* initialize reading the input multibeam file */
		if ((status = mb_read_init(verbose, ifile, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
		                           &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) != MB_SUCCESS) {
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", ifile);
			status = MB_FAILURE;
			do_error_dialog("Unable to open input file.", "You may not have read", "permission in this directory!");
			return (status);
		}

		/* allocate memory for data arrays */
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&detect, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&priority, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&pulses, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&editcount, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, MBEDIT_MAX_PINGS * sizeof(double), (void **)&bathlist, &error);
		for (int i = 0; i < MBEDIT_BUFFER_SIZE; i++) {
			ping[i].allocated = 0;
			ping[i].beamflag = NULL;
			ping[i].bath = NULL;
			ping[i].amp = NULL;
			ping[i].bathacrosstrack = NULL;
			ping[i].bathalongtrack = NULL;
			ping[i].detect = NULL;
			ping[i].priority = NULL;
			ping[i].pulses = NULL;
			ping[i].bath_x = NULL;
			ping[i].bath_y = NULL;
		}

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR) {
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* initialize the buffer */
		nbuff = 0;
	}

	/* if success so far deal with edit save files */
	if (status == MB_SUCCESS) {
		/* reset message */
		if (savemode) {
			sprintf(notice, "MBedit is sorting %d old edits...", esf.nedit);
			do_message_on(notice);
		}

		/* handle esf edits */
    const int outputmode = ((output_mode == MBEDIT_OUTPUT_BROWSE) ? MBP_ESF_NOWRITE : MBP_ESF_WRITE);
		if (savemode || (outputmode == MBP_ESF_WRITE)) {
			status = mb_esf_load(verbose, program_name, ifile, savemode, outputmode, esffile, &esf, &error);
			if (output_mode != MBEDIT_OUTPUT_BROWSE && status == MB_SUCCESS && esf.esffp != NULL)
				esffile_open = true;
			if (status == MB_FAILURE && error == MB_ERROR_OPEN_FAIL) {
				esffile_open = false;
				fprintf(stderr, "\nUnable to open new edit save file %s\n", esf.esffile);
				do_error_dialog("Unable to open new edit save file.", "You may not have write", "permission in this directory!");
			}
			else if (status == MB_FAILURE && error == MB_ERROR_MEMORY_FAIL) {
				esffile_open = false;
				fprintf(stderr, "\nUnable to allocate memory for edits in esf file %s\n", esf.esffile);
				do_error_dialog("Unable to allocate memory for.", "edits in existing edit", "save file!");
			}
		}
	}

	/* deal with success */
	if (status == MB_SUCCESS) {
		file_open = true;
		if (verbose >= 0) {
			fprintf(stderr, "\nMultibeam File <%s> initialized for reading\n", ifile);
			fprintf(stderr, "Multibeam Data Format ID: %d\n", format);
		}
	}
	else {
		file_open = false;
		if (verbose >= 0) {
			fprintf(stderr, "\nERROR: Multibeam File <%s> NOT initialized for reading\n", ifile);
			fprintf(stderr, "Multibeam Data Format ID: %d\n", format);
		}
	}

	/* turn off message */
	do_message_off();

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	verbose = 0;
	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_close_file() {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
	}

	/* reset message */
	do_message_on("MBedit is closing a data file...");

	/* deallocate memory for data arrays */
	for (int i = 0; i < MBEDIT_BUFFER_SIZE; i++) {
		if (ping[i].allocated > 0) {
			ping[i].allocated = 0;
			free(ping[i].beamflag);
			free(ping[i].bath);
			free(ping[i].amp);
			free(ping[i].bathacrosstrack);
			free(ping[i].bathalongtrack);
			free(ping[i].detect);
			free(ping[i].priority);
			free(ping[i].pulses);
			free(ping[i].bath_x);
			free(ping[i].bath_y);

			/* reset message */
			if (i % 250 == 0) {
				sprintf(notice, "MBedit: %d pings deallocated...", i);
				do_message_on(notice);
			}
		}
	}

	int status = MB_SUCCESS;

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	/* close the files */
	status = mb_close(verbose, &imbio_ptr, &error);
	if (esf.edit != NULL || esf.esffp != NULL) {
		status = mb_esf_close(verbose, &esf, &error);
	}

	/* unlock the raw swath file */
	if (uselockfiles)
		status = mb_pr_unlockswathfile(verbose, ifile, MBP_LOCK_EDITBATHY, program_name, &error);

	/* set mbprocess parameters */
	if (output_mode == MBEDIT_OUTPUT_EDIT) {
		/* update mbprocess parameter file */
		status = mb_pr_update_format(verbose, ifile, true, format, &error);
		status = mb_pr_update_edit(verbose, ifile, MBP_EDIT_ON, esf.esffile, &error);

		/* run mbprocess if desired */
		if (run_mbprocess) {
			/* turn message on */
			do_message_on("Bathymetry edits being applied using mbprocess...");

			/* run mbprocess */
			char command[2*MB_PATH_MAXLINE] = "";
			sprintf(command, "mbprocess -I %s\n", ifile);
			/* int shellstatus = */ system(command);
		}
	}

	/* if we got here we must have succeeded */
	if (verbose >= 0) {
		fprintf(stderr, "\nMultibeam Input File <%s> closed\n", ifile);
		fprintf(stderr, "%d data records loaded\n", nload_total);
		fprintf(stderr, "%d data records dumped\n", ndump_total);
	}
	file_open = false;
	nload_total = 0;
	ndump_total = 0;

	/* turn file button on */
	do_filebutton_on();
	do_nextbutton_off();

	/* turn off message */
	do_message_off();

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_dump_data(int hold_size, int *ndumped, int *nbuffer) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       hold_size:   %d\n", hold_size);
	}

	int status = MB_SUCCESS;

	/* dump or clear data from the buffer */
	ndump = 0;
	if (nbuff > 0) {
		/* turn message on */
		do_message_on("MBedit is clearing data...");

		/* output changed edits in pings to be dumped */
		for (int iping = 0; iping < nbuff - hold_size; iping++) {
			for (int jbeam = 0; jbeam < ping[iping].beams_bath; jbeam++) {
				if (ping[iping].beamflag[jbeam] != ping[iping].beamflagorg[jbeam]) {
					int action;
					if (mb_beam_ok(ping[iping].beamflag[jbeam]))
						action = MBP_EDIT_UNFLAG;
					else if (mb_beam_check_flag_filter2(ping[iping].beamflag[jbeam]))
						action = MBP_EDIT_FILTER;
					else if (mb_beam_check_flag_filter(ping[iping].beamflag[jbeam]))
						action = MBP_EDIT_FILTER;
					else if (!mb_beam_check_flag_unusable2(ping[iping].beamflag[jbeam]))
						action = MBP_EDIT_FLAG;
					else
						action = MBP_EDIT_ZERO;
					mb_esf_save(verbose, &esf, ping[iping].time_d, jbeam + ping[iping].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
					            action, &error);
				}
			}
		}

		/* deallocate pings to be dumped */
		for (int iping = 0; iping < nbuff - hold_size; iping++) {
			if (ping[iping].allocated > 0) {
				ping[iping].allocated = 0;
				free(ping[iping].beamflag);
				free(ping[iping].beamflagorg);
				free(ping[iping].bath);
				free(ping[iping].amp);
				free(ping[iping].bathacrosstrack);
				free(ping[iping].bathalongtrack);
				free(ping[iping].detect);
				free(ping[iping].priority);
				free(ping[iping].pulses);
				free(ping[iping].bath_x);
				free(ping[iping].bath_y);
			}
		}

		/* copy data to be held */
		for (int iping = 0; iping < hold_size; iping++) {
			ping[iping] = ping[iping + nbuff - hold_size];
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
	if (verbose >= 2) {
		fprintf(stderr, "\n%d data records dumped from buffer\n", *ndumped);
		fprintf(stderr, "%d data records remain in buffer\n", *nbuffer);
	// }

	// if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ndumped:    %d\n", *ndumped);
		fprintf(stderr, "dbg2       nbuffer:    %d\n", *nbuffer);
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_load_data(int buffer_size, int *nloaded, int *nbuffer, int *ngood, int *icurrent) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       buffer_size: %d\n", buffer_size);
	}

	int status = MB_SUCCESS;
	int namp, nss;
	char string[MB_PATH_MAXLINE];
	int detect_status, detect_error, nbeams;
	double speed_nav;
	int sensorhead = 0;
	int sensorhead_error = MB_ERROR_NO_ERROR;

	/* turn message on */
	nload = 0;
	sprintf(string, "MBedit: %d records loaded so far...", nload);
	do_message_on(string);

	/* load data */
	do {
		error = MB_ERROR_NO_ERROR;
		status = mb_get_all(verbose, imbio_ptr, &store_ptr, &kind, ping[nbuff].time_i, &ping[nbuff].time_d, &ping[nbuff].navlon,
		                    &ping[nbuff].navlat, &ping[nbuff].speed, &ping[nbuff].heading, &distance, &ping[nbuff].altitude,
		                    &ping[nbuff].sensordepth, &ping[nbuff].beams_bath, &namp, &nss, beamflag, bath, amp, bathacrosstrack,
		                    bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);
		if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
			if (nbuff > 0)
				ping[nbuff].time_interval = ping[nbuff].time_d - ping[nbuff - 1].time_d;
			status = mb_extract_nav(verbose, imbio_ptr, store_ptr, &kind, ping[nbuff].time_i, &ping[nbuff].time_d,
			                        &ping[nbuff].navlon, &ping[nbuff].navlat, &speed_nav, &ping[nbuff].heading, &draft,
			                        &ping[nbuff].roll, &ping[nbuff].pitch, &ping[nbuff].heave, &error);
			const int sensorhead_status = mb_sensorhead(verbose, imbio_ptr, store_ptr, &sensorhead, &sensorhead_error);
			if (sensorhead_status == MB_SUCCESS) {
				ping[nbuff].multiplicity = sensorhead;
			}
			else if (nbuff > 0 && fabs(ping[nbuff].time_d - ping[nbuff - 1].time_d) < MB_ESF_MAXTIMEDIFF) {
				ping[nbuff].multiplicity = ping[nbuff - 1].multiplicity + 1;
			}
			else {
				ping[nbuff].multiplicity = 0;
			}
			if (nbuff == 0)
				ping[nbuff].distance = 0.0;
			else
				ping[nbuff].distance = ping[nbuff - 1].distance + ping[nbuff].speed * ping[nbuff].time_interval / 3.6;
			nbeams = ping[nbuff].beams_bath;
			detect_status = mb_detects(verbose, imbio_ptr, store_ptr, &kind, &nbeams, detect, &detect_error);
			if (detect_status != MB_SUCCESS) {
				status = MB_SUCCESS;
				for (int i = 0; i < ping[nbuff].beams_bath; i++) {
					detect[i] = MB_DETECT_UNKNOWN;
					priority[i] = 0;
				}
			}
			else {
				for (int i = 0; i < ping[nbuff].beams_bath; i++) {
          priority[i] = (detect[i] & 0x0000FF00) << 8;
					detect[i] = detect[i] & 0x000000FF;
				}
			}
			detect_status = mb_pulses(verbose, imbio_ptr, store_ptr, &kind, &nbeams, pulses, &detect_error);
			if (detect_status != MB_SUCCESS) {
				status = MB_SUCCESS;
				for (int i = 0; i < ping[nbuff].beams_bath; i++) {
					pulses[i] = MB_PULSE_UNKNOWN;
				}
			}
		}
		if (error <= MB_ERROR_NO_ERROR && (kind == MB_DATA_DATA) &&
		    (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP || error == MB_ERROR_OUT_BOUNDS ||
		     error == MB_ERROR_OUT_TIME || error == MB_ERROR_SPEED_TOO_SMALL)) {
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
		}
		else if (error <= MB_ERROR_NO_ERROR) {
			status = MB_FAILURE;
			error = MB_ERROR_OTHER;
		}

		if (status == MB_SUCCESS && ping[nbuff].allocated > 0 && ping[nbuff].allocated < ping[nbuff].beams_bath) {
			ping[nbuff].allocated = 0;
			free(ping[nbuff].beamflag);
			free(ping[nbuff].beamflagorg);
			free(ping[nbuff].bath);
			free(ping[nbuff].amp);
			free(ping[nbuff].bathacrosstrack);
			free(ping[nbuff].bathalongtrack);
			free(ping[nbuff].detect);
			free(ping[nbuff].priority);
			free(ping[nbuff].pulses);
			free(ping[nbuff].bath_x);
			free(ping[nbuff].bath_y);
		}
		if (status == MB_SUCCESS && ping[nbuff].allocated < ping[nbuff].beams_bath) {
			ping[nbuff].beamflag = NULL;
			ping[nbuff].beamflagorg = NULL;
			ping[nbuff].bath = NULL;
			ping[nbuff].amp = NULL;
			ping[nbuff].bathacrosstrack = NULL;
			ping[nbuff].bathalongtrack = NULL;
			ping[nbuff].bath_x = NULL;
			ping[nbuff].bath_y = NULL;
			ping[nbuff].beamflag = (char *)malloc(ping[nbuff].beams_bath * sizeof(char));
			ping[nbuff].beamflagorg = (char *)malloc(ping[nbuff].beams_bath * sizeof(char));
			ping[nbuff].bath = (double *)malloc(ping[nbuff].beams_bath * sizeof(double));
			ping[nbuff].amp = (double *)malloc(ping[nbuff].beams_bath * sizeof(double));
			ping[nbuff].bathacrosstrack = (double *)malloc(ping[nbuff].beams_bath * sizeof(double));
			ping[nbuff].bathalongtrack = (double *)malloc(ping[nbuff].beams_bath * sizeof(double));
			ping[nbuff].detect = (int *)malloc(ping[nbuff].beams_bath * sizeof(int));
			ping[nbuff].priority = (int *)malloc(ping[nbuff].beams_bath * sizeof(int));
			ping[nbuff].pulses = (int *)malloc(ping[nbuff].beams_bath * sizeof(int));
			ping[nbuff].bath_x = (int *)malloc(ping[nbuff].beams_bath * sizeof(int));
			ping[nbuff].bath_y = (int *)malloc(ping[nbuff].beams_bath * sizeof(int));
			ping[nbuff].allocated = ping[nbuff].beams_bath;
		}
		if (status == MB_SUCCESS && ping[nbuff].allocated > 0) {
			for (int i = 0; i < ping[nbuff].beams_bath; i++) {
				ping[nbuff].beamflag[i] = beamflag[i];
				ping[nbuff].beamflagorg[i] = beamflag[i];
				ping[nbuff].bath[i] = bath[i];
				ping[nbuff].amp[i] = amp[i];
				ping[nbuff].bathacrosstrack[i] = bathacrosstrack[i];
				ping[nbuff].bathalongtrack[i] = bathalongtrack[i];
				ping[nbuff].detect[i] = detect[i];
				ping[nbuff].priority[i] = priority[i];
				ping[nbuff].pulses[i] = pulses[i];
				ping[nbuff].bath_x[i] = 0;
				ping[nbuff].bath_y[i] = 0;
			}
		}
		if (status == MB_SUCCESS) {
			nbuff++;
			nload++;

			/* update message every 250 records */
			if (nload % 250 == 0) {
				sprintf(string, "MBedit: %d records loaded so far...", nload);
				do_message_on(string);
			}

					if (verbose >= 5) {
				fprintf(stderr, "\ndbg5  Next good data found in function <%s>:\n", __func__);
				fprintf(stderr, "dbg5       buffer id: %d   global id: %d\n", nbuff - 1, nbuff - 1 + ndump_total);
			}
		}
	} while (error <= MB_ERROR_NO_ERROR && nbuff < buffer_size);
	*ngood = nbuff;
	*nbuffer = nbuff;
	*nloaded = nload;
	nload_total += nload;

	/* define success */
	if (nload > 0) {
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
	}
	else {
		status = MB_FAILURE;
		error = MB_ERROR_EOF;
	}

	/* find index of current ping */
	current_id = 0;
	*icurrent = current_id;

	/* if desired apply saved edits */
	if (esf.nedit > 0) {
		/* reset message */
		do_message_on("MBedit is applying saved edits...");

		/* loop over each data record, checking each edit */
		for (int i = 0; i < nbuff; i++) {
			/* apply edits for this ping */
			status =
			    mb_esf_apply(verbose, &esf, ping[i].time_d, ping[i].multiplicity, ping[i].beams_bath, ping[i].beamflag, &error);

			/* check beamflags versus original beamflags - beams that were originally flagged
			        flagged by sonar but have since been flagged for other reasons should have
			        beamflag reset to flag by sonar */
			for (int j = 0; j < ping[i].beams_bath; j++) {
				if (!mb_beam_ok(ping[i].beamflag[j]) && mb_beam_check_flag_sonar(ping[i].beamflagorg[j]))
					ping[i].beamflag[j] = mb_beam_set_flag_sonar(ping[i].beamflag[j]);
			}

			/* update message every 250 records */
			if (i % 250 == 0) {
				sprintf(string, "MBedit: saved edits applied to %d of %d records so far...", i, nbuff - 1);
				do_message_on(string);
			}
		}
	}

	/* if desired filter pings */
	if (filter_medianspike || filter_wrongside || filter_cutbeam || filter_cutdistance ||
	    filter_cutangle) {
		/* reset message */
		do_message_on("MBedit is applying bathymetry filters...");

		/* loop over each data record, checking each edit */
		for (int i = 0; i < nbuff; i++) {
			mbedit_filter_ping(i);

			/* update message every 250 records */
			if (i % 250 == 0) {
				sprintf(string, "MBedit: filters applied to %d of %d records so far...", i, nbuff - 1);
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
	if (verbose >= 0) {
		fprintf(stderr, "\n%d data records loaded from input file <%s>\n", *nloaded, ifile);
		fprintf(stderr, "%d data records now in buffer\n", *nbuffer);
		fprintf(stderr, "%d editable survey data records now in buffer\n", *ngood);
		fprintf(stderr, "Current data record:        %d\n", current_id);
		fprintf(stderr, "Current global data record: %d\n", current_id + ndump_total);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nloaded:    %d\n", *nloaded);
		fprintf(stderr, "dbg2       nbuffer:    %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:      %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:   %d\n", *icurrent);
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_clear_screen() {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
	}

	/* clear screen */
	xg_fillrectangle(mbedit_xgid, borders[0], borders[2], borders[1] - borders[0], borders[3] - borders[2], pixel_values[WHITE],
	                 XG_SOLIDLINE);

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_plot_all(int plwd, int exgr, int xntrvl, int yntrvl, int plt_size,
                    int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time,
                    int *nplt, bool autoscale) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
		fprintf(stderr, "dbg2       nplt:        %p\n", nplt);
		fprintf(stderr, "dbg2       autoscale:   %d\n", autoscale);
	}

	/* set scales and tick intervals */
	plot_width = plwd;
	exager = exgr;
	show_mode = sh_mode;
	show_flaggedsoundings = sh_flggdsdg;
	show_flaggedprofiles = sh_flggdprf;
	show_time = sh_time,

	/* figure out which pings to plot */
	    plot_size = plt_size;
	if (current_id + plot_size > nbuff)
		nplot = nbuff - current_id;
	else
		nplot = plot_size;
	*nplt = nplot;

	/* get data into ping arrays and find median depth value */
	// double bathsum = 0.0;
	int nbathsum = 0;
	int nbathlist = 0;
	double xtrack_max = 0.0;
	for (int i = current_id; i < current_id + nplot; i++) {
		ping[i].record = i + ndump_total;
		ping[i].outbounds = MBEDIT_OUTBOUNDS_NONE;
		for (int j = 0; j < ping[i].beams_bath; j++) {
			if (mb_beam_ok(ping[i].beamflag[j])) {
				// bathsum += ping[i].bath[j];
				nbathsum++;
				bathlist[nbathlist] = ping[i].bath[j];
				nbathlist++;
				xtrack_max = MAX(xtrack_max, fabs(ping[i].bathacrosstrack[j]));
			}
		}
	}

	/* if not enough information in unflagged bathymetry look
	    into the flagged bathymetry */
	if (nbathlist <= 0 || xtrack_max <= 0.0) {
		for (int i = current_id; i < current_id + nplot; i++) {
			for (int j = 0; j < ping[i].beams_bath; j++) {
				if (!mb_beam_ok(ping[i].beamflag[j]) && !mb_beam_check_flag_unusable2(ping[i].beamflag[j])) {
					// bathsum += ping[i].bath[j];
					nbathsum++;
					bathlist[nbathlist] = ping[i].bath[j];
					nbathlist++;
					xtrack_max = MAX(xtrack_max, fabs(ping[i].bathacrosstrack[j]));
				}
			}
		}
	}
	double bathmedian = 0.0;  // -Wmaybe-uninitialized
	if (nbathlist > 0) {
		qsort(bathlist, nbathlist, sizeof(double), mb_double_compare);
		bathmedian = bathlist[nbathlist / 2];
	}

	/* reset xtrack_max if required */
	if (autoscale && xtrack_max < 0.5) {
		xtrack_max = 1000.0;
	}
	else if (autoscale && xtrack_max > 100000.0) {
		xtrack_max = 100000.0;
	}

	/* if autoscale on reset plot width */
	if (autoscale && xtrack_max > 0.0) {
		plot_width = (int)(2.4 * xtrack_max);
		const int ndec = MAX(1, (int)log10((double)plot_width));
		int maxx = 1;
		for (int i = 0; i < ndec; i++)
			maxx = maxx * 10;
		maxx = (plot_width / maxx + 1) * maxx;

		xntrvl = plot_width / 10;
		if (xntrvl > 1000) {
			xntrvl = 1000 * (xntrvl / 1000);
		}
		else if (xntrvl > 500) {
			xntrvl = 500 * (xntrvl / 500);
		}
		else if (xntrvl > 250) {
			xntrvl = 250 * (xntrvl / 250);
		}
		else if (xntrvl > 100) {
			xntrvl = 100 * (xntrvl / 100);
		}
		else if (xntrvl > 50) {
			xntrvl = 50 * (xntrvl / 50);
		}
		else if (xntrvl > 25) {
			xntrvl = 25 * (xntrvl / 25);
		}
		else if (xntrvl > 10) {
			xntrvl = 10 * (xntrvl / 10);
		}
		else if (xntrvl > 5) {
			xntrvl = 5 * (xntrvl / 5);
		}
		else if (xntrvl > 2) {
			xntrvl = 2 * (xntrvl / 2);
		}
		else {
			xntrvl = 1;
		}
		yntrvl = xntrvl;
		do_reset_scale_x(plot_width, maxx, xntrvl, yntrvl);
	}

	/* print out information */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2       %d data records set for plotting (%d desired)\n", nplot, plot_size);
		fprintf(stderr, "dbg2       xtrack_max:  %f\n", xtrack_max);
		fprintf(stderr, "dbg2       bathmedian:  %f\n", bathmedian);
		fprintf(stderr, "dbg2       nbathlist:   %d\n", nbathlist);
		fprintf(stderr, "dbg2       nbathsum:    %d\n", nbathsum);
		for (int i = current_id; i < current_id + nplot; i++) {
			fprintf(stderr, "dbg2       %4d %4d %4d  %d/%d/%d %2.2d:%2.2d:%2.2d.%6.6d  %10.3f\n", i, ping[i].id, ping[i].record,
			        ping[i].time_i[1], ping[i].time_i[2], ping[i].time_i[0], ping[i].time_i[3], ping[i].time_i[4],
			        ping[i].time_i[5], ping[i].time_i[6], ping[i].bath[ping[i].beams_bath / 2]);
		}
	}

	/* clear screen */
	xg_fillrectangle(mbedit_xgid, borders[0], borders[2], borders[1] - borders[0], borders[3] - borders[2], pixel_values[WHITE],
	                 XG_SOLIDLINE);

	/* set scaling */
	x_interval = xntrvl;
	y_interval = yntrvl;
	const int xcen = xmin + (xmax - xmin) / 2;
	const int ycen = ymin + (ymax - ymin) / 2;
	// const double dx = ((double)(xmax - xmin)) / plot_size;
	const double dy = ((double)(ymax - ymin)) / plot_size;
	xscale = 100.0 * plot_width / (xmax - xmin);
	yscale = (xscale * 100.0) / exager;
	const double dxscale = 100.0 / xscale;
	const double dyscale = 100.0 / yscale;

	if (info_set) {
		mbedit_plot_info();
	}

	char string[MB_PATH_MAXLINE];
	int swidth;
	int sascent;
	int sdescent;
	int sxstart;

	if (sh_mode == MBEDIT_SHOW_FLAG) {
		sprintf(string, "Sounding Colors by Flagging:  Unflagged  Manual  Filter  Sonar");
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		sxstart = xcen - swidth / 2;

		sprintf(string, "Sounding Colors by Flagging:  Unflagged  ");
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, sxstart, ymin - margin / 2 + sascent + 5, string, pixel_values[BLACK], XG_SOLIDLINE);

		sxstart += swidth;
		sprintf(string, "Manual  ");
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, sxstart, ymin - margin / 2 + sascent + 5, string, pixel_values[RED], XG_SOLIDLINE);

		sxstart += swidth;
		sprintf(string, "Filter  ");
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, sxstart, ymin - margin / 2 + sascent + 5, string, pixel_values[BLUE], XG_SOLIDLINE);

		sxstart += swidth;
		sprintf(string, "Sonar");
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, sxstart, ymin - margin / 2 + sascent + 5, string, pixel_values[GREEN], XG_SOLIDLINE);
	}
	else if (sh_mode == MBEDIT_SHOW_DETECT) {
		sprintf(string, "Sounding Colors by Bottom Detection:  Amplitude  Phase  Unknown");
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		sxstart = xcen - swidth / 2;

		sprintf(string, "Sounding Colors by Bottom Detection:  Amplitude  ");
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, sxstart, ymin - margin / 2 + sascent + 5, string, pixel_values[BLACK], XG_SOLIDLINE);

		sxstart += swidth;
		sprintf(string, "Phase  ");
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, sxstart, ymin - margin / 2 + sascent + 5, string, pixel_values[RED], XG_SOLIDLINE);

		sxstart += swidth;
		sprintf(string, "Unknown");
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, sxstart, ymin - margin / 2 + sascent + 5, string, pixel_values[GREEN], XG_SOLIDLINE);
	}
	else if (sh_mode == MBEDIT_SHOW_PULSE) {
		sprintf(string, "Sounding Colors by Source Type:  CW  Up-Chirp  Down-Chirp  Unknown");
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		sxstart = xcen - swidth / 2;

		sprintf(string, "Sounding Colors by Source Type:  CW  ");
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, sxstart, ymin - margin / 2 + sascent + 5, string, pixel_values[BLACK], XG_SOLIDLINE);

		sxstart += swidth;
		sprintf(string, "Up-Chirp  ");
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, sxstart, ymin - margin / 2 + sascent + 5, string, pixel_values[RED], XG_SOLIDLINE);

		sxstart += swidth;
		sprintf(string, "Down-Chirp  ");
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, sxstart, ymin - margin / 2 + sascent + 5, string, pixel_values[BLUE], XG_SOLIDLINE);

		sxstart += swidth;
		sprintf(string, "Unknown");
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, sxstart, ymin - margin / 2 + sascent + 5, string, pixel_values[GREEN], XG_SOLIDLINE);
	}

	sprintf(string, "Vertical Exageration: %4.2f   All Distances and Depths in Meters", (exager / 100.));
	xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
	xg_drawstring(mbedit_xgid, xcen - swidth / 2, ymin - margin / 2 + 2 * (sascent + sdescent) + 5, string, pixel_values[BLACK],
	              XG_SOLIDLINE);

	/* plot filename */
	sprintf(string, "File %d of %d:", file_id + 1, num_files);
	xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
	xg_drawstring(mbedit_xgid, margin / 2, ymin - 3 * margin / 4, string, pixel_values[BLACK], XG_SOLIDLINE);
	char *string_ptr = strrchr(ifile, '/');
	if (string_ptr == NULL)
		string_ptr = ifile;
	else if (strlen(string_ptr) > 0)
		string_ptr++;
	xg_drawstring(mbedit_xgid, margin / 2 + 2 + swidth, ymin - margin / 2 - 1 * (sascent + sdescent) - 5, string_ptr,
	              pixel_values[BLACK], XG_SOLIDLINE);

	/* plot file position bar */
	int fpx = margin / 2 + ((4 * margin) * current_id) / nbuff;
	const int fpdx = MAX((((4 * margin) * nplot) / nbuff), 5);
	const int fpy = ymin - 5 * margin / 8;
	const int fpdy = margin / 4;
	if (fpx + fpdx > 9 * margin / 2)
		fpx = 9 * margin / 2 - fpdx;
	xg_drawrectangle(mbedit_xgid, margin / 2, ymin - 5 * margin / 8, 4 * margin, margin / 4, pixel_values[BLACK], XG_SOLIDLINE);
	xg_drawrectangle(mbedit_xgid, margin / 2 - 1, ymin - 5 * margin / 8 - 1, 4 * margin + 2, margin / 4 + 2, pixel_values[BLACK],
	                 XG_SOLIDLINE);
	xg_fillrectangle(mbedit_xgid, fpx, fpy, fpdx, fpdy, pixel_values[LIGHTGREY], XG_SOLIDLINE);
	xg_drawrectangle(mbedit_xgid, fpx, fpy, fpdx, fpdy, pixel_values[BLACK], XG_SOLIDLINE);
	sprintf(string, "0 ");
	xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
	xg_drawstring(mbedit_xgid, margin / 2 - swidth, ymin - margin / 2 + sascent / 2, string, pixel_values[BLACK], XG_SOLIDLINE);
	sprintf(string, " %d", nbuff);
	xg_drawstring(mbedit_xgid, 9 * margin / 2, ymin - margin / 2 + sascent / 2, string, pixel_values[BLACK], XG_SOLIDLINE);

	/* plot scale bars */
	const double dx_width = (xmax - xmin) / dxscale;
	const int nx_int = (int)(0.5 * dx_width / x_interval + 1);
	const int x_int = (int)(x_interval * dxscale);
	xg_drawline(mbedit_xgid, xmin, ymax, xmax, ymax, pixel_values[BLACK], XG_SOLIDLINE);
	xg_drawline(mbedit_xgid, xmin, ymin, xmax, ymin, pixel_values[BLACK], XG_SOLIDLINE);
	for (int i = 0; i < nx_int; i++) {
		const int xx = i * x_int;
		const int vx = i * x_interval;
		xg_drawline(mbedit_xgid, xcen - xx, ymin, xcen - xx, ymax, pixel_values[BLACK], XG_DASHLINE);
		xg_drawline(mbedit_xgid, xcen + xx, ymin, xcen + xx, ymax, pixel_values[BLACK], XG_DASHLINE);
		sprintf(string, "%1d", vx);
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, xcen + xx - swidth / 2, ymax + sascent + 5, string, pixel_values[BLACK], XG_SOLIDLINE);
		xg_drawstring(mbedit_xgid, xcen - xx - swidth / 2, ymax + sascent + 5, string, pixel_values[BLACK], XG_SOLIDLINE);
	}
	const double dy_height = (ymax - ymin) / dyscale;
	const int ny_int = (int)(dy_height / y_interval + 1);
	const int y_int = (int)(y_interval * dyscale);
	xg_drawline(mbedit_xgid, xmin, ymin, xmin, ymax, pixel_values[BLACK], XG_SOLIDLINE);
	xg_drawline(mbedit_xgid, xmax, ymin, xmax, ymax, pixel_values[BLACK], XG_SOLIDLINE);
	for (int i = 0; i < ny_int; i++) {
		const int yy = i * y_int;
		const int vy = i * y_interval;
		xg_drawline(mbedit_xgid, xmin, ymax - yy, xmax, ymax - yy, pixel_values[BLACK], XG_DASHLINE);
		sprintf(string, "%1d", vy);
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, xmax + 5, ymax - yy + sascent / 2, string, pixel_values[BLACK], XG_SOLIDLINE);
	}

	// int x0;
	// int y0;
	double tsmin;
	double tsmax;
	double tsvalue;
	double tsslope;

	/* plot time series if desired */
	if (show_time > MBEDIT_PLOT_TIME) {
		/* get scaling */
		mbedit_tsminmax(current_id, nplot, show_time, &tsmin, &tsmax);
		const double tsscale = 2.0 * margin / (tsmax - tsmin);

		/* draw time series plot box */
		xg_drawline(mbedit_xgid, margin / 2, ymin, margin / 2, ymax, pixel_values[BLACK], XG_SOLIDLINE);
		xg_drawline(mbedit_xgid, margin, ymin, margin, ymax, pixel_values[BLACK], XG_DASHLINE);
		xg_drawline(mbedit_xgid, 3 * margin / 2, ymin, 3 * margin / 2, ymax, pixel_values[BLACK], XG_DASHLINE);
		xg_drawline(mbedit_xgid, 2 * margin, ymin, 2 * margin, ymax, pixel_values[BLACK], XG_DASHLINE);
		xg_drawline(mbedit_xgid, 5 * margin / 2, ymin, 5 * margin / 2, ymax, pixel_values[BLACK], XG_SOLIDLINE);
		xg_drawline(mbedit_xgid, margin / 2, ymax, 5 * margin / 2, ymax, pixel_values[BLACK], XG_SOLIDLINE);
		xg_drawline(mbedit_xgid, margin / 2, ymin, 5 * margin / 2, ymin, pixel_values[BLACK], XG_SOLIDLINE);

		/* draw time series labels */
		/*sprintf(string,"Heading (deg)");*/
		mbedit_tslabel(show_time, string);
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, 3 * margin / 2 - swidth / 2, ymin - sdescent, string, pixel_values[BLACK], XG_SOLIDLINE);
		sprintf(string, "%g", tsmin);
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, margin / 2 - swidth / 2, ymax + sascent + 5, string, pixel_values[BLACK], XG_SOLIDLINE);
		sprintf(string, "%g", tsmax);
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, 5 * margin / 2 - swidth / 2, ymax + sascent + 5, string, pixel_values[BLACK], XG_SOLIDLINE);

		/*x0 = margin/2 + ping[current_id].heading / 360.0 * 2 * margin;*/
		mbedit_tsvalue(current_id, show_time, &tsvalue);
		int x0 = margin / 2 + (int)((tsvalue - tsmin) * tsscale);
		int y0 = ymax - (int)(dy / 2);
		for (int i = current_id; i < current_id + nplot; i++) {
			/*x = margin/2 + ping[i].heading / 360.0 * 2 * margin;*/
			mbedit_tsvalue(i, show_time, &tsvalue);
			const int x = margin / 2 + (int)((tsvalue - tsmin) * tsscale);
			const int y = ymax - (int)(dy / 2) - (int)((i - current_id) * dy);
			xg_drawline(mbedit_xgid, x0, y0, x, y, pixel_values[BLACK], XG_SOLIDLINE);
			xg_fillrectangle(mbedit_xgid, x - 2, y - 2, 4, 4, pixel_values[BLACK], XG_SOLIDLINE);
			x0 = x;
			y0 = y;
		}

		/* if plotting roll, also plot acrosstrack slope */
		if (show_time == MBEDIT_PLOT_ROLL) {
			mbedit_xtrackslope(current_id, &tsslope);
			x0 = margin / 2 + (int)((tsslope - tsmin) * tsscale);
			y0 = ymax - (int)(dy / 2);
			for (int i = current_id; i < current_id + nplot; i++) {
				/*x = margin/2 + ping[i].heading / 360.0 * 2 * margin;*/
				mbedit_xtrackslope(i, &tsslope);
				const int x = margin / 2 + (int)((tsslope - tsmin) * tsscale);
				const int y = ymax - (int)(dy / 2) - (int)((i - current_id) * dy);
				xg_drawline(mbedit_xgid, x0, y0, x, y, pixel_values[RED], XG_SOLIDLINE);
				x0 = x;
				y0 = y;
			}
		}

		/* if plotting roll, also plot acrosstrack slope - roll */
		if (show_time == MBEDIT_PLOT_ROLL) {
			mbedit_xtrackslope(current_id, &tsslope);
			int i_tmp = 0;
			mbedit_tsvalue(i_tmp, show_time, &tsvalue);
			x0 = margin / 2 + (int)((tsvalue - tsslope - tsmin) * tsscale);
			y0 = ymax - (int)(dy / 2);
			for (int i = current_id; i < current_id + nplot; i++) {
				/*x = margin/2 + ping[i].heading / 360.0 * 2 * margin;*/
				mbedit_xtrackslope(i, &tsslope);
				mbedit_tsvalue(i, show_time, &tsvalue);
				const int x = margin / 2 + (int)((tsvalue - tsslope - tsmin) * tsscale);
				const int y = ymax - (int)(dy / 2) - (int)((i - current_id) * dy);
				xg_drawline(mbedit_xgid, x0, y0, x, y, pixel_values[BLUE], XG_SOLIDLINE);
				x0 = x;
				y0 = y;
			}
		}
	}

	int status = MB_SUCCESS;

	/* plot pings */
	for (int i = current_id; i < current_id + nplot; i++) {
		/* set beam plotting locations */
		// const int x = xmax - (int)(dx / 2) - (int)((i - current_id) * dx);
		const int y = ymax - (int)(dy / 2) - (int)((i - current_id) * dy);
		ping[i].label_x = xmin - 5;
		ping[i].label_y = y;
		for (int j = 0; j < ping[i].beams_bath; j++) {
			if (!mb_beam_check_flag_unusable2(ping[i].beamflag[j])) {
				if (view_mode == MBEDIT_VIEW_WATERFALL) {
					ping[i].bath_x[j] = (int)(xcen + dxscale * ping[i].bathacrosstrack[j]);
					ping[i].bath_y[j] = (int)(y + dyscale * ((double)ping[i].bath[j] - bathmedian));
				}
				else if (view_mode == MBEDIT_VIEW_ALONGTRACK) {
					ping[i].bath_x[j] = (int)(xcen + dxscale * ping[i].bathacrosstrack[j]);
					ping[i].bath_y[j] = (int)(ycen + dyscale * ((double)ping[i].bath[j] - bathmedian));
				}
				else {
					/* ping[i].bath_x[j] = x;*/
					ping[i].bath_x[j] = (int)(xcen + dxscale * (ping[i].bathalongtrack[j] +
					                          ping[i].distance - ping[current_id + nplot / 2].distance));
					ping[i].bath_y[j] = (int)(ycen + dyscale * ((double)ping[i].bath[j] - bathmedian));
				}
			}
			else {
				ping[i].bath_x[j] = 0;
				ping[i].bath_y[j] = 0;
			}
		}

		/* plot the beams */
		for (int j = 0; j < ping[i].beams_bath; j++)
			status = mbedit_plot_beam(i, j);

		/* plot the ping profile */
		status = mbedit_plot_ping(i);

		/* set and draw info string */
		mbedit_plot_ping_label(i, true);
	}

	/* set status */
	if (nplot > 0)
		status = MB_SUCCESS;
	else
		status = MB_FAILURE;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nplot:       %d\n", *nplt);
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_plot_beam(int iping, int jbeam) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       iping:       %d\n", iping);
		fprintf(stderr, "dbg2       jbeam:       %d\n", jbeam);
	}

	int status = MB_SUCCESS;
	int beam_color;

	/* plot the beam */
	if (info_set && iping == info_ping && jbeam == info_beam) {
		if (!mb_beam_check_flag_unusable2(ping[iping].beamflag[jbeam]))
			xg_fillrectangle(mbedit_xgid, ping[iping].bath_x[jbeam] - 4, ping[iping].bath_y[jbeam] - 4, 8, 8, pixel_values[BLUE],
			                 XG_SOLIDLINE);
	}
	else if (jbeam >= 0 && jbeam < ping[iping].beams_bath && !mb_beam_check_flag_unusable2(ping[iping].beamflag[jbeam])) {
		beam_color = BLACK;
		if (show_mode == MBEDIT_SHOW_FLAG) {
			if (mb_beam_ok(ping[iping].beamflag[jbeam]))
				beam_color = BLACK;
			else if (mb_beam_check_flag_filter2(ping[iping].beamflag[jbeam]))
				beam_color = BLUE;
			else if (mb_beam_check_flag_filter(ping[iping].beamflag[jbeam]))
				beam_color = BLUE;
			else if (mb_beam_check_flag_sonar(ping[iping].beamflag[jbeam]))
				beam_color = GREEN;
			else {
				beam_color = RED;
//fprintf(stderr, "Beam:%d flag:%u priority:%d detect:%d\n",
//jbeam,ping[iping].beamflag[jbeam],ping[iping].priority[jbeam],ping[iping].detect[jbeam]);
      }
		}
		else if (show_mode == MBEDIT_SHOW_DETECT) {
			if (ping[iping].detect[jbeam] == MB_DETECT_AMPLITUDE)
				beam_color = BLACK;
			else if (ping[iping].detect[jbeam] == MB_DETECT_PHASE)
				beam_color = RED;
			else
				beam_color = GREEN;
		}
		else if (show_mode == MBEDIT_SHOW_PULSE) {
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
			xg_fillrectangle(mbedit_xgid, ping[iping].bath_x[jbeam] - 2, ping[iping].bath_y[jbeam] - 2, 4, 4,
			                 pixel_values[beam_color], XG_SOLIDLINE);
		else if (show_flaggedsoundings)
			xg_drawrectangle(mbedit_xgid, ping[iping].bath_x[jbeam] - 2, ping[iping].bath_y[jbeam] - 2, 4, 4,
			                 pixel_values[beam_color], XG_SOLIDLINE);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_plot_ping(int iping) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       iping:       %d\n", iping);
	}

	int status = MB_SUCCESS;
	int xold, yold;

	/* plot the ping profile */
	bool first = true;
	bool last_flagged = false;
	for (int j = 0; j < ping[iping].beams_bath; j++) {
		if (show_flaggedprofiles && !mb_beam_ok(ping[iping].beamflag[j]) &&
		    !mb_beam_check_flag_unusable2(ping[iping].beamflag[j]) && first) {
			first = false;
			last_flagged = true;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
		}
		else if (mb_beam_ok(ping[iping].beamflag[j]) && first) {
			first = false;
			last_flagged = false;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
		}
		else if (!last_flagged && mb_beam_ok(ping[iping].beamflag[j])) {
			xg_drawline(mbedit_xgid, xold, yold, ping[iping].bath_x[j], ping[iping].bath_y[j], pixel_values[BLACK], XG_SOLIDLINE);
			last_flagged = false;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
		}
		else if (mb_beam_ok(ping[iping].beamflag[j])) {
			xg_drawline(mbedit_xgid, xold, yold, ping[iping].bath_x[j], ping[iping].bath_y[j], pixel_values[RED], XG_SOLIDLINE);
			last_flagged = false;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
		}
		else if (show_flaggedprofiles && !mb_beam_ok(ping[iping].beamflag[j]) &&
		         !mb_beam_check_flag_unusable2(ping[iping].beamflag[j])) {
			if (j > 0)
				xg_drawline(mbedit_xgid, xold, yold, ping[iping].bath_x[j], ping[iping].bath_y[j], pixel_values[RED],
				            XG_SOLIDLINE);
			last_flagged = true;
			xold = ping[iping].bath_x[j];
			yold = ping[iping].bath_y[j];
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_plot_ping_label(int iping, bool save) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       iping:       %d\n", iping);
		fprintf(stderr, "dbg2       save:        %d\n", save);
	}

	int status = MB_SUCCESS;
	int sascent, sdescent, swidth;
	char string[MB_PATH_MAXLINE];

	/* get the ping outbounds value */
	ping[iping].outbounds = MBEDIT_OUTBOUNDS_NONE;
	for (int j = 0; j < ping[iping].beams_bath; j++) {
		if (!mb_beam_check_flag_unusable2(ping[iping].beamflag[j]) &&
		    (ping[iping].bath_x[j] < xmin || ping[iping].bath_x[j] > xmax || ping[iping].bath_y[j] < ymin ||
		     ping[iping].bath_y[j] > ymax)) {
			if (mb_beam_ok(ping[iping].beamflag[j]))
				ping[iping].outbounds = MBEDIT_OUTBOUNDS_UNFLAGGED;
			else if (!mb_beam_check_flag_unusable2(ping[iping].beamflag[j]) && ping[iping].outbounds != MBEDIT_OUTBOUNDS_UNFLAGGED)
				ping[iping].outbounds = MBEDIT_OUTBOUNDS_FLAGGED;
		}
	}

	/* set info string with time tag */
	if (show_time == MBEDIT_PLOT_TIME || save) {
		if (ping[iping].beams_bath > 0 && mb_beam_ok(ping[iping].beamflag[ping[iping].beams_bath / 2]))
			sprintf(string, "%5d %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d %10.3f", ping[iping].record + 1, ping[iping].time_i[1],
			        ping[iping].time_i[2], ping[iping].time_i[0], ping[iping].time_i[3], ping[iping].time_i[4],
			        ping[iping].time_i[5], (int)(0.001 * ping[iping].time_i[6]), ping[iping].bath[ping[iping].beams_bath / 2]);
		else if (ping[iping].beams_bath > 0)
			sprintf(string, "%5d %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d %10.3f", ping[iping].record + 1, ping[iping].time_i[1],
			        ping[iping].time_i[2], ping[iping].time_i[0], ping[iping].time_i[3], ping[iping].time_i[4],
			        ping[iping].time_i[5], (int)(0.001 * ping[iping].time_i[6]), ping[iping].altitude + ping[iping].sensordepth);
		else
			sprintf(string, "%5d %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d %10.3f", ping[iping].record + 1, ping[iping].time_i[1],
			        ping[iping].time_i[2], ping[iping].time_i[0], ping[iping].time_i[3], ping[iping].time_i[4],
			        ping[iping].time_i[5], (int)(0.001 * ping[iping].time_i[6]), 0.0);

		/* save string to show last ping seen at end of program */
		if (save)
			strcpy(last_ping, string);
	}

	/* set info string without time tag */
	if (show_time != MBEDIT_PLOT_TIME) {
		if (ping[iping].beams_bath > 0)
			sprintf(string, "%5d %10.3f", ping[iping].record, ping[iping].bath[ping[iping].beams_bath / 2]);
		else
			sprintf(string, "%5d %10.3f", ping[iping].record, 0.0);

		/* save string to show last ping seen at end of program */
		if (save)
			strcpy(last_ping, string);
	}

	/* justify the string */
	xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);

	/* unplot the ping label */
	xg_fillrectangle(mbedit_xgid, ping[iping].label_x - swidth - 21, ping[iping].label_y - sascent - 1, swidth + 22,
	                 sascent + sdescent + 2, pixel_values[WHITE], XG_SOLIDLINE);

	/* plot the ping label */
	if (ping[iping].outbounds == MBEDIT_OUTBOUNDS_UNFLAGGED) {
		xg_fillrectangle(mbedit_xgid, ping[iping].label_x - swidth, ping[iping].label_y - sascent, swidth, sascent + sdescent,
		                 pixel_values[RED], XG_SOLIDLINE);
		ping[iping].zap_x1 = ping[iping].label_x - swidth - 20;
		ping[iping].zap_x2 = ping[iping].zap_x1 + 10;
		ping[iping].zap_y1 = ping[iping].label_y - sascent;
		ping[iping].zap_y2 = ping[iping].zap_y1 + sascent + sdescent;
		xg_drawrectangle(mbedit_xgid, ping[iping].zap_x1, ping[iping].zap_y1, 10, sascent + sdescent, pixel_values[BLACK],
		                 XG_SOLIDLINE);
	}
	else if (ping[iping].outbounds == MBEDIT_OUTBOUNDS_FLAGGED)
		xg_fillrectangle(mbedit_xgid, ping[iping].label_x - swidth, ping[iping].label_y - sascent, swidth, sascent + sdescent,
		                 pixel_values[GREEN], XG_SOLIDLINE);
	xg_drawstring(mbedit_xgid, ping[iping].label_x - swidth, ping[iping].label_y, string, pixel_values[BLACK], XG_SOLIDLINE);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_plot_info() {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
	}

	int status = MB_SUCCESS;
	int sascent, sdescent, swidth;

	/* plot the info */
	if (info_set) {
		int xcen = xmin + (xmax - xmin) / 2;

		char string[MB_PATH_MAXLINE];
		sprintf(string, "Ping:%d  Beam:%d  Time: %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d", info_ping, info_beam, info_time_i[1],
		        info_time_i[2], info_time_i[0], info_time_i[3], info_time_i[4], info_time_i[5], (int)(0.001 * info_time_i[6]));
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, xcen - swidth / 2, ymin - margin / 2 - 2 * (sascent + sdescent) + 3, string, pixel_values[BLACK],
		              XG_SOLIDLINE);
    fprintf(stdout, "\nSelected soundng:\n%s\n", string);

		sprintf(string, "Lon:%.5f deg  Lat:%.5f deg  Hdg:%.1f deg  Spd:%.1f km/hr",
            info_navlon, info_navlat, info_heading, info_speed);
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, xcen - swidth / 2, ymin - margin / 2 - 1 * (sascent + sdescent) + 3, string, pixel_values[BLACK],
		              XG_SOLIDLINE);
    fprintf(stdout, "%s\n", string);

		sprintf(string, "Depth:%.2f  X:%.2f  L:%.2f  Alt:%.2f  Amp:%.2f  Detect:%s  Pulse:%s", info_bath,
		        info_bathacrosstrack, info_bathalongtrack, info_altitude, info_amp, detect_name[info_detect], pulse_name[info_pulse]);
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, xcen - swidth / 2, ymin - margin / 2 + 3, string, pixel_values[BLACK], XG_SOLIDLINE);
    fprintf(stdout, "%s\n", string);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_unplot_beam(int iping, int jbeam) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       iping:       %d\n", iping);
		fprintf(stderr, "dbg2       jbeam:       %d\n", jbeam);
	}

	/* unplot the beam */
	if (info_set && iping == info_ping && jbeam == info_beam) {
		if (!mb_beam_check_flag_unusable2(ping[iping].beamflag[jbeam]))
			xg_fillrectangle(mbedit_xgid, ping[iping].bath_x[jbeam] - 4, ping[iping].bath_y[jbeam] - 4, 8, 8, pixel_values[WHITE],
			                 XG_SOLIDLINE);
	}
	else if (jbeam >= 0 && jbeam < ping[iping].beams_bath) {
		if (mb_beam_ok(ping[iping].beamflag[jbeam]))
			xg_fillrectangle(mbedit_xgid, ping[iping].bath_x[jbeam] - 2, ping[iping].bath_y[jbeam] - 2, 4, 4, pixel_values[WHITE],
			                 XG_SOLIDLINE);
		else if (!mb_beam_check_flag_unusable2(ping[iping].beamflag[jbeam]))
			xg_drawrectangle(mbedit_xgid, ping[iping].bath_x[jbeam] - 2, ping[iping].bath_y[jbeam] - 2, 4, 4, pixel_values[WHITE],
			                 XG_SOLIDLINE);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_unplot_ping(int iping) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       iping:       %d\n", iping);
	}

	int xold, yold;

	/* unplot the ping profile */
	bool first = true;
	for (int j = 0; j < ping[iping].beams_bath; j++) {
		if (mb_beam_ok(ping[iping].beamflag[j])) {
      if (first) {
			  first = false;
			  xold = ping[iping].bath_x[j];
			  yold = ping[iping].bath_y[j];
		  } else {
			  xg_drawline(mbedit_xgid, xold, yold, ping[iping].bath_x[j], ping[iping].bath_y[j], pixel_values[WHITE], XG_SOLIDLINE);
			  xold = ping[iping].bath_x[j];
			  yold = ping[iping].bath_y[j];
		  }
    }
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_unplot_info() {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
	}

	int sascent, sdescent, swidth;

	/* plot the info */
	if (info_set) {
		const int xcen = xmin + (xmax - xmin) / 2;

		char string[MB_PATH_MAXLINE];
		sprintf(string, "Ping:%d  Beam:%d  Time: %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d", info_ping, info_beam, info_time_i[1],
		        info_time_i[2], info_time_i[0], info_time_i[3], info_time_i[4], info_time_i[5], (int)(0.001 * info_time_i[6]));
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, xcen - swidth / 2, ymin - margin / 2 - 2 * (sascent + sdescent) + 3, string, pixel_values[WHITE],
		              XG_SOLIDLINE);

		sprintf(string, "Lon:%.5f deg  Lat:%.5f deg  Hdg:%.1f deg  Spd:%.1f km/hr",
                info_navlon, info_navlat, info_heading, info_speed);
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, xcen - swidth / 2, ymin - margin / 2 - 1 * (sascent + sdescent) + 3, string, pixel_values[WHITE],
		              XG_SOLIDLINE);

		sprintf(string, "Depth:%.2f  X:%.2f  L:%.2f  Alt:%.2f  Amp:%.2f  Detect:%s  Pulse:%s", info_bath,
		        info_bathacrosstrack, info_bathalongtrack, info_altitude, info_amp, detect_name[info_detect], pulse_name[info_pulse]);
		xg_justify(mbedit_xgid, string, &swidth, &sascent, &sdescent);
		xg_drawstring(mbedit_xgid, xcen - swidth / 2, ymin - margin / 2 + 3, string, pixel_values[WHITE], XG_SOLIDLINE);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_action_goto(int ttime_i[7], int hold_size, int buffer_size, int plwd, int exgr, int xntrvl, int yntrvl, int plt_size,
                       int sh_mode, int sh_flggdsdg, int sh_flggdprf, int sh_time, int *ndumped, int *nloaded, int *nbuffer, int *ngood,
                       int *icurrent, int *nplt) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       time_i[0]:   %d\n", ttime_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:   %d\n", ttime_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:   %d\n", ttime_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:   %d\n", ttime_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:   %d\n", ttime_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:   %d\n", ttime_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:   %d\n", ttime_i[6]);
		fprintf(stderr, "dbg2       hold_size:   %d\n", hold_size);
		fprintf(stderr, "dbg2       buffer_size: %d\n", buffer_size);
		fprintf(stderr, "dbg2       plot_width:  %d\n", plwd);
		fprintf(stderr, "dbg2       exager:      %d\n", exgr);
		fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
		fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
		fprintf(stderr, "dbg2       plot_size:   %d\n", plt_size);
		fprintf(stderr, "dbg2       show_mode:   %d\n", sh_mode);
		fprintf(stderr, "dbg2       show_flaggedsoundings:    %d\n", sh_flggdsdg);
		fprintf(stderr, "dbg2       show_flaggedprofiles:     %d\n", sh_flggdprf);
		fprintf(stderr, "dbg2       show_time:   %d\n", sh_time);
	}

	/* let the world know... */
	if (verbose >= 1) {
		fprintf(stderr, "\n>> Looking for time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n\n", ttime_i[0], ttime_i[1],
		        ttime_i[2], ttime_i[3], ttime_i[4], ttime_i[5], ttime_i[6]);
	}

	double ttime_d;

	bool found = false;

	/* get time_d value */
	mb_get_time(verbose, ttime_i, &ttime_d);

	int status = MB_SUCCESS;

	/* check if a file has been opened */
	if (!file_open) {
		status = MB_FAILURE;
		*ndumped = 0;
		*nloaded = 0;
		*nbuffer = nbuff;
		*ngood = nbuff;
		current_id = 0;
		*icurrent = current_id;
		*nplt = 0;
		if (verbose >= 1)
			fprintf(stderr, "\n>> No data file has been opened...\n");
		do_error_dialog("No data file has", "been opened...", "  ");
	}

	/* check if the target time is in the present buffer */
	else if (nbuff > 0) {
		/* check if the present buffer starts
		    later than the target time */
		if (ping[0].time_d > ttime_d) {
			status = MB_FAILURE;
			*ndumped = 0;
			*nloaded = 0;
			*nbuffer = nbuff;
			*ngood = nbuff;
			*icurrent = current_id;
			*nplt = 0;
			if (verbose >= 1)
				fprintf(stderr, "\n>> Beginning of present buffer is later than target time...\n");
			do_error_dialog("Beginning of loaded data", "is later than the", "specified target time...");
		}

		/* check if the file ends
		    before the target time */
		else if (ping[nbuff - 1].time_d < ttime_d && nbuff < buffer_size) {
			status = MB_FAILURE;
			*ndumped = 0;
			*nloaded = 0;
			*nbuffer = nbuff;
			*ngood = nbuff;
			*icurrent = current_id;
			*nplt = 0;
			if (verbose >= 1)
				fprintf(stderr, "\n>> Target time is beyond end of file...\n");
			do_error_dialog("Target time is", "beyond the end", "of the data file...");
		}
	}

	/* loop through buffers until the target time is found
	    or the file ends */
	while (!found && status == MB_SUCCESS) {
		/* check out current buffer */
		for (int i = 0; i < nbuff; i++) {
			if (ping[i].time_d > ttime_d && !found) {
				found = true;
				current_id = i;
			}
		}

		/* load new buffer if needed */
		if (!found && nbuff >= buffer_size) {
			/* dump the buffer */
			/* status = */ mbedit_dump_data(hold_size, ndumped, nbuffer);

			/* load the buffer */
			status = mbedit_load_data(buffer_size, nloaded, nbuffer, ngood, icurrent);

			/* if end of file close it */
			if (status == MB_FAILURE) {
				/* status = */ mbedit_dump_data(0, ndumped, nbuffer);
				mbedit_close_file();
				status = MB_FAILURE;
				*nbuffer = nbuff;
				*ngood = nbuff;
				*icurrent = current_id;
				*nplt = 0;
				if (verbose >= 1)
					fprintf(stderr, "\n>> Target time is beyond end of file, file closed...\n");
				do_error_dialog("Target time is beyond the", "end of the data file!", "The file has been closed...");
			}
		}

		/* turns out the file ends
		    before the target time */
		else if (!found && nbuff < buffer_size) {
			status = MB_FAILURE;
			*nbuffer = nbuff;
			*ngood = nbuff;
			*icurrent = current_id;
			*nplt = 0;
			if (verbose >= 1)
				fprintf(stderr, "\n>> Target time is beyond end of file...\n");
			do_error_dialog("Target time is", "beyond the end", "of the data file...");
		}
	}

	/* clear the screen */
	status = mbedit_clear_screen();

	/* set up plotting */
	if (*ngood > 0) {
		status = mbedit_plot_all(plwd, exgr, xntrvl, yntrvl, plt_size, sh_mode, sh_flggdsdg, sh_flggdprf, sh_time, nplt, false);
	}

	/* let the world know... */
	if (verbose >= 2) {
    if (found) {
		  fprintf(stderr, "\n>> Target time %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d found\n", ttime_i[0], ttime_i[1], ttime_i[2],
		        ttime_i[3], ttime_i[4], ttime_i[5], ttime_i[6]);
		  fprintf(stderr, ">> Found time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", ping[0].time_i[0], ping[0].time_i[1],
		        ping[0].time_i[2], ping[0].time_i[3], ping[0].time_i[4], ping[0].time_i[5], ping[0].time_i[6]);
		  fprintf(stderr, "Current data record index:  %d\n", current_id);
		  fprintf(stderr, "Current global data record: %d\n", current_id + ndump_total);
    } else {
		  fprintf(stderr, "\n>> Target time %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d found\n", ttime_i[0], ttime_i[1], ttime_i[2],
		        ttime_i[3], ttime_i[4], ttime_i[5], ttime_i[6]);
		  fprintf(stderr, "\n>> Unable to go to target time...\n");
    }
	}

	/* reset beam_save */
	beam_save = false;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ndumped:     %d\n", *ndumped);
		fprintf(stderr, "dbg2       nloaded:     %d\n", *nloaded);
		fprintf(stderr, "dbg2       nbuffer:     %d\n", *nbuffer);
		fprintf(stderr, "dbg2       ngood:       %d\n", *ngood);
		fprintf(stderr, "dbg2       icurrent:    %d\n", *icurrent);
		fprintf(stderr, "dbg2       nplot:        %d\n", *nplt);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_tslabel(int data_id, char *label) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       data_id:         %d\n", data_id);
	}

	/* get the time series label */
	switch (data_id) {
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
	case MBEDIT_PLOT_SENSORDEPTH:
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

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       label:       %s\n", label);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_tsvalue(int iping, int data_id, double *value) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       iping:           %d\n", iping);
		fprintf(stderr, "dbg2       data_id:         %d\n", data_id);
	}

	/* get the time series value */
	if (iping >= 0 && nbuff > iping) {
		switch (data_id) {
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
			*value = ping[iping].bath[ping[iping].beams_bath / 2];
			break;
		case MBEDIT_PLOT_ALTITUDE:
			*value = ping[iping].altitude;
			break;
		case MBEDIT_PLOT_SENSORDEPTH:
			*value = ping[iping].sensordepth;
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

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       value:       %f\n", *value);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_tsminmax(int iping, int nping, int data_id, double *tsmin, double *tsmax) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       iping:           %d\n", iping);
		fprintf(stderr, "dbg2       nping:           %d\n", nping);
		fprintf(stderr, "dbg2       data_id:         %d\n", data_id);
	}

	/* get the time series minimum and maximum value */
	*tsmin = 0.0;
	*tsmax = 0.0;
	if (iping >= 0 && nbuff > iping && nping > 0 && iping + nping - 1 < nbuff) {
		mbedit_tsvalue(iping, data_id, tsmin);
		*tsmax = *tsmin;
		for (int i = iping; i < iping + nping; i++) {
		  double value;
			mbedit_tsvalue(i, data_id, &value);
			*tsmin = MIN(*tsmin, value);
			*tsmax = MAX(*tsmax, value);

			/* handle slope plotting in roll plot */
			if (data_id == MBEDIT_PLOT_ROLL) {
		    double value2;
				mbedit_xtrackslope(i, &value2);
				*tsmin = MIN(*tsmin, value2);
				*tsmax = MAX(*tsmax, value2);
				*tsmin = MIN(*tsmin, value - value2);
				*tsmax = MAX(*tsmax, value - value2);
			}
		}
	}

	/* adjust the min max according to data type */
	switch (data_id) {
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
	{
		const double halfwidth = MAX(0.001, 0.55 * (*tsmax - *tsmin));
		const double center = 0.5 * (*tsmin + *tsmax);
		*tsmin = center - halfwidth;
		*tsmax = center + halfwidth;
		break;
	}
	case MBEDIT_PLOT_LAT:
	{
		const double halfwidth = MAX(0.001, 0.55 * (*tsmax - *tsmin));
		const double center = 0.5 * (*tsmin + *tsmax);
		*tsmin = center - halfwidth;
		*tsmax = center + halfwidth;
		break;
	}
	case MBEDIT_PLOT_HEADING:
		*tsmin = 0.0;
		*tsmax = 360.0;
		break;
	case MBEDIT_PLOT_SPEED:
		*tsmin = 0.0;
		*tsmax = MAX(*tsmax, 5.0);
		break;
	case MBEDIT_PLOT_DEPTH:
	{
		const double halfwidth = MAX(1.0, 0.55 * (*tsmax - *tsmin));
		const double center = 0.5 * (*tsmin + *tsmax);
		*tsmin = center - halfwidth;
		*tsmax = center + halfwidth;
		break;
	}
	case MBEDIT_PLOT_ALTITUDE:
	{
		const double halfwidth = MAX(1.0, 0.55 * (*tsmax - *tsmin));
		const double center = 0.5 * (*tsmin + *tsmax);
		*tsmin = center - halfwidth;
		*tsmax = center + halfwidth;
		break;
	}
	case MBEDIT_PLOT_SENSORDEPTH:
	{
		const double halfwidth = MAX(1.0, 0.55 * (*tsmax - *tsmin));
		const double center = 0.5 * (*tsmin + *tsmax);
		*tsmin = center - halfwidth;
		*tsmax = center + halfwidth;
		break;
	}
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

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       tsmin:       %f\n", *tsmin);
		fprintf(stderr, "dbg2       tsmax:       %f\n", *tsmax);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbedit_xtrackslope(int iping, double *slope) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       iping:           %d\n", iping);
	}

	*slope = 0.0;

	/* get the slope value */
	if (iping >= 0 && nbuff > iping) {
		/* initialize linear fit variables */
		double sx = 0.0;
		double sy = 0.0;
		double sxx = 0.0;
		double sxy = 0.0;
		int ns = 0;
		for (int jbeam = 0; jbeam < ping[iping].beams_bath; jbeam++) {
			/* use valid beams to calculate slope */
			if (mb_beam_ok(ping[iping].beamflag[jbeam])) {
				sx += ping[iping].bathacrosstrack[jbeam];
				sy += ping[iping].bath[jbeam];
				sxx += ping[iping].bathacrosstrack[jbeam] * ping[iping].bathacrosstrack[jbeam];
				sxy += ping[iping].bathacrosstrack[jbeam] * ping[iping].bath[jbeam];
				ns++;
			}
		}

		/* get linear fit to ping */
		if (ns > 0) {
			const double delta = ns * sxx - sx * sx;
			/* a = (sxx*sy - sx*sxy)/delta; */
			const double b = (ns * sxy - sx * sy) / delta;
			*slope = -RTD * atan(b);
			;
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       slope:       %f\n", *slope);
		fprintf(stderr, "dbg2       error:       %d\n", error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:      %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
