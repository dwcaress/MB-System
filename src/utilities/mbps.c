/*--------------------------------------------------------------------
 *    The MB-system:	mbps.c	11/4/93
 *
 *    Copyright (c) 1993-2019 by
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
 * MBPS is a program that plots an almost correct perspective view
 * of a piece of swath data. Input is some swath data
 * file; output is PostScript code.
 *
 * Authors:	Russ Alexander, UCSB
 *		Alberto Malinverno, L-DEO
 * Date:	September 15, 1993 (version 3)
 * Date:	August 31, 1991 (original version)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

/* GMT include files */
#include "gmt.h"
#include "pslib.h"

#include "mb_status.h"
#include "mb_define.h"

/*--------------------------------------------------------------------*/

/* Global defines */
#define MBPS_MAXPINGS 1000
#define DY_DEF (0.1) /* km */
#define DZ_DEF (-50) /* m */
#define PLOT_XMAX_DEF 5.0
#define PLOT_YMAX_DEF 8.0
#define VIEWDIR_DEF 'S'
#define ALPHA_DEF 70.0
#define ETA_DEF 45.0
#define BAD -9999999.99
#define VE_DEF 5.0

struct ping {
	int beams_bath;
	char *beamflag;
	double *bath;
	double *bathacrosstrack;
	double *bathalongtrack;
	double *xp;
	double *yp;
};

static const int rgb_black[] = {0, 0, 0};
static const int rgb_white[] = {255, 255, 255};

static const char program_name[] = "MBPS";
static const char help_message[] =
    "MBPS reads a swath bathymetry data file and creates a postscript 3-d mesh plot";
static const char usage_message[] =
    "mbps [-Iinfile -Fformat -Nnpings -Ppings\n\t-Byr/mo/da/hr/mn/sc -Eyr/mo/da/hr/mn/sc  \n\t-Aalpha "
    "-Keta -Dviewdir -Xvertexag \n\t-T\"title\" -Wmetersperinch \n\t-Sspeedmin -Ggap -Ydisplay_stats "
    "\n\t-Zdisplay_scales -V -H]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int errflg = 0;
	int c;
	int help = 0;
	int flag = 0;

	/*ALBERTO definitions */
	int gap = 1;
	double *xp, *yp;
	double xl[4], yl[4];
	double alpha = ALPHA_DEF;
	double eta = ETA_DEF;
	double ve = VE_DEF;
	char viewdir = VIEWDIR_DEF;
	int display_stats = MB_YES;
	int display_scales = MB_YES;
	double sin_eta, cos_eta;
	double sin_alpha, cos_alpha;
	double track_length, xscale, zscale, zscale_inch;
	double mean_xp = 0.0, mean_yp = 0.0, min_xp, max_xp, min_yp, max_yp;
	double scaling, x_off, y_off;
	double min_z, max_z, range_z, meters_per_inch = (-1.0);
	double mean_lat = 0.0;
	double mean_lon = 0.0;
	double mean_latmin;
	double mean_lonmin;
	double mean_hdg = 0.0;
	int done, mean_knt = 0;
	int orient;
	char label[100];
	int a, b, rotate;
	double x, y, z;

	/* MBIO status variables */
	int verbose = 0;
	int error = MB_ERROR_NO_ERROR;
	char *message;

	/* MBIO read control parameters */
	int format;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double btime_d;
	double etime_d;
	double speedmin;
	double timegap;
	char file[MB_PATH_MAXLINE];
	int pings = 1;
	int beams_bath;
	int beams_amp;
	int pixels_ss;
	int num_pings_max = MBPS_MAXPINGS;

	/* MBIO read values */
	void *mbio_ptr = NULL;
	int kind;
	struct ping data[MBPS_MAXPINGS + 3];
	int time_i[7];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double heading;
	double distance;
	double altitude;
	double sonardepth;
	char *beamflag;
	double *bath = NULL;
	double *bathacrosstrack = NULL;
	double *bathalongtrack = NULL;
	double *amp = NULL;
	double *ss = NULL;
	double *ssacrosstrack = NULL;
	double *ssalongtrack = NULL;
	char comment[MB_COMMENT_MAXLINE];
	int timbeg_i[7];
	int timend_i[7];
	double distot = 0.0;
	int nread;

	char title[MB_COMMENT_MAXLINE];
	int forward;
	double xx, yy, zz;
	double heading_start, dheading, dheadingx, dheadingy;
	int jj;

	// TODO(schwehr): Remove embedded prototypes.
	void Polygon_Fill();
	void Good_Polygon();

	/* initialize some time variables */
	for (int i = 0; i < 7; i++) {
		timbeg_i[i] = 0;
		timend_i[i] = 0;
	}

	/* get current default values */
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	/* set default input to stdin */
	strcpy(file, "stdin");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:B:b:E:e:S:s:T:t:I:i:A:a:X:x:K:k:D:d:N:n:P:p:W:w:G:g:YyZz")) != -1)
		switch (c) {
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'A':
		case 'a':
			sscanf(optarg, "%lf", &alpha);
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf(optarg, "%d/%d/%d/%d/%d/%d", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &btime_i[5]);
			btime_i[6] = 0;
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf(optarg, "%c", &viewdir);
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf(optarg, "%d/%d/%d/%d/%d/%d", &etime_i[0], &etime_i[1], &etime_i[2], &etime_i[3], &etime_i[4], &etime_i[5]);
			etime_i[6] = 0;
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf(optarg, "%d", &format);
			flag++;
			break;
		case 'G':
		case 'g':
			sscanf(optarg, "%d", &gap);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf(optarg, "%s", file);
			flag++;
			break;
		case 'K':
		case 'k':
			sscanf(optarg, "%lf", &eta);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf(optarg, "%d", &num_pings_max);
			if (num_pings_max < 2 || num_pings_max > MBPS_MAXPINGS)
				num_pings_max = MBPS_MAXPINGS;
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf(optarg, "%d", &pings);
			flag++;
			break;
		case 'S':
		case 's':
			sscanf(optarg, "%lf", &speedmin);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf(optarg, "%s", title);
			flag++;
			break;
		case 'X':
		case 'x':
			sscanf(optarg, "%lf", &ve);
			flag++;
			break;
		case 'W':
		case 'w':
			sscanf(optarg, "%lf", &meters_per_inch);
			flag++;
			break;
		case 'Y':
		case 'y':
			display_stats = MB_NO;
			flag++;
			break;
		case 'Z':
		case 'z':
			display_scales = MB_NO;
			flag++;
			break;
		case '?':
			errflg++;
			break;
		} /* switch */

	/* Process the title of the plot */
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-' && ((argv[i][1] == 'T') || (argv[i][1] == 't'))) {
			strcpy(title, argv[i]);
			title[0] = ' ';
			title[1] = ' ';
		}
	}

	/* check that otions are allowed */
	if ((viewdir != 'P') && (viewdir != 'S') && (viewdir != 'B') && (viewdir != 'p') && (viewdir != 's') && (viewdir != 'b')) {
		fprintf(stderr, "viewdir must be either P/p (port) S/s (stbd) or B/b (back)\n");
		errflg++;
	}

	/* if error flagged then print it and exit */
	if (errflg) {
		fprintf(stderr, "usage: %s\n", usage_message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
	}

	if (verbose == 1 || help) {
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Control Parameters:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       help:       %d\n", help);
		fprintf(stderr, "dbg2       format:     %d\n", format);
		fprintf(stderr, "dbg2       btime_i[0]: %d\n", btime_i[0]);
		fprintf(stderr, "dbg2       btime_i[1]: %d\n", btime_i[1]);
		fprintf(stderr, "dbg2       btime_i[2]: %d\n", btime_i[2]);
		fprintf(stderr, "dbg2       btime_i[3]: %d\n", btime_i[3]);
		fprintf(stderr, "dbg2       btime_i[4]: %d\n", btime_i[4]);
		fprintf(stderr, "dbg2       btime_i[5]: %d\n", btime_i[5]);
		fprintf(stderr, "dbg2       btime_i[6]: %d\n", btime_i[6]);
		fprintf(stderr, "dbg2       etime_i[0]: %d\n", etime_i[0]);
		fprintf(stderr, "dbg2       etime_i[1]: %d\n", etime_i[1]);
		fprintf(stderr, "dbg2       etime_i[2]: %d\n", etime_i[2]);
		fprintf(stderr, "dbg2       etime_i[3]: %d\n", etime_i[3]);
		fprintf(stderr, "dbg2       etime_i[4]: %d\n", etime_i[4]);
		fprintf(stderr, "dbg2       etime_i[5]: %d\n", etime_i[5]);
		fprintf(stderr, "dbg2       etime_i[6]: %d\n", etime_i[6]);
		fprintf(stderr, "dbg2       speedmin:   %f\n", speedmin);
		fprintf(stderr, "dbg2       file:       %s\n", file);
	}

	/* if help desired then print it and exit */
	if (help) {
		fprintf(stderr, "\n%s\n", help_message);
		fprintf(stderr, "\nusage: %s\n", usage_message);
		exit(error);
	}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, file, NULL, &format, &error);

	/* initialize reading the swath file */
	if ((status = mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
	                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) != MB_SUCCESS) {
		mb_error(verbose, error, &message);
		fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
		fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* initialize values */
	sin_alpha = sin(alpha * DTR);
	cos_alpha = cos(alpha * DTR);
	sin_eta = sin(eta * DTR);
	cos_eta = cos(eta * DTR);
	min_z = 0.0;
	max_z = -9999.0;

	/* allocate memory for data arrays */
	beamflag = NULL;
	bath = NULL;
	bathacrosstrack = NULL;
	bathalongtrack = NULL;
	amp = NULL;
	ss = NULL;
	ssacrosstrack = NULL;
	ssalongtrack = NULL;
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);
	for (int i = 0; i < num_pings_max + 3; i++) {
		data[i].beams_bath = 0;
		data[i].beamflag = NULL;
		data[i].bath = NULL;
		data[i].bathacrosstrack = NULL;
		data[i].bathalongtrack = NULL;
		data[i].xp = NULL;
		data[i].yp = NULL;
	}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		mb_error(verbose, error, &message);
		fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* read and process data */
	nread = 0;
	done = MB_NO;
	error = MB_ERROR_NO_ERROR;
	while (done == MB_NO && error <= MB_ERROR_NO_ERROR) {
		/* read a ping of data */
		status = mb_get(verbose, mbio_ptr, &kind, &pings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance,
		                &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathacrosstrack,
		                bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

		/* only work with survey data */
		if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {

			/* allocate arrays */
			data[nread].beams_bath = beams_bath;
			status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(char), (void **)&(data[nread].beamflag), &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&(data[nread].bath), &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&(data[nread].bathacrosstrack),
			                    &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&(data[nread].bathalongtrack),
			                    &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&(data[nread].xp), &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&(data[nread].yp), &error);

			/* copy data to storage arrays */
			for (int i = 0; i < beams_bath; i++) {
				data[nread].beamflag[i] = beamflag[i];
				data[nread].bath[i] = bath[i];
				data[nread].bathacrosstrack[i] = bathacrosstrack[i];
				data[nread].bathalongtrack[i] = bathalongtrack[i];
				data[nread].xp[i] = BAD;
				data[nread].yp[i] = BAD;
			}

			/* ignore time gaps */
			if (error == MB_ERROR_TIME_GAP) {
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}

			/* output error messages */
			if (error == MB_ERROR_COMMENT) {
				/* do nothing */
			}
			else if (verbose >= 1 && error < MB_ERROR_NO_ERROR && error >= MB_ERROR_OTHER) {
				mb_error(verbose, error, &message);
				fprintf(stderr, "\nNonfatal MBIO Error:\n%s\n", message);
				fprintf(stderr, "Time: %d %d %d %d %d %d %d\n", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5],
				        time_i[6]);
			}
			else if (verbose >= 1 && error < MB_ERROR_NO_ERROR) {
				mb_error(verbose, error, &message);
				fprintf(stderr, "\nNonfatal MBIO Error:\n%s\n", message);
				fprintf(stderr, "Number of good records so far: %d\n", nread);
			}
			else if (verbose >= 1 && error > MB_ERROR_NO_ERROR && error != MB_ERROR_EOF) {
				mb_error(verbose, error, &message);
				fprintf(stderr, "\nFatal MBIO Error:\n%s\n", message);
				fprintf(stderr, "Last Good Time: %d %d %d %d %d %d %d\n", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4],
				        time_i[5], time_i[6]);
			}

			/* calculate raw x,y locations for each beam */
			if (status == MB_SUCCESS) {
				/* set initial heading */
				if (nread == 0)
					heading_start = heading;

				/* get heading x and y components */
				dheading = heading - heading_start;
				if (dheading > 360.0)
					dheading -= 360.0;
				else if (dheading < 0.0)
					dheading += 360.0;
				dheadingx = sin(DTR * dheading);
				dheadingy = cos(DTR * dheading);

				/* get alongtrack distance in nav */
				distot += distance * 1000.0; /* distance in meters */

				/* loop over the beams */
				for (int j = 0; j < beams_bath; j++) {
					if (j >= data[nread].beams_bath) {
						data[nread].beamflag[j] = MB_FLAG_NULL;
						data[nread].xp[j] = BAD;
						data[nread].yp[j] = BAD;
					}
					else if (mb_beam_ok(beamflag[j])) {
						xx = dheadingy * bathacrosstrack[j] + dheadingx * bathalongtrack[j];
						yy = distot - dheadingx * bathacrosstrack[j] + dheadingy * bathalongtrack[j];
						zz = -bath[j];
						if (viewdir == 'S' || viewdir == 's') {
							data[nread].xp[j] = yy + xx * sin_eta * cos_alpha;
							data[nread].yp[j] = zz * cos_eta * ve - xx * sin_eta * sin_alpha;
						}
						else if (viewdir == 'P' || viewdir == 'p') {
							data[nread].xp[j] = -yy - xx * sin_eta * cos_alpha;
							data[nread].yp[j] = zz * cos_eta * ve + xx * sin_eta * sin_alpha;
						}
						else if (viewdir == 'B' || viewdir == 'b') {
							data[nread].xp[j] = xx + yy * sin_eta * cos_alpha;
							data[nread].yp[j] = zz * cos_eta * ve + yy * sin_eta * sin_alpha;
						}
						mean_lat += navlat;
						mean_lon += navlon;
						mean_hdg += heading;
						mean_xp += data[nread].xp[j];
						mean_yp += data[nread].yp[j];
						mean_knt++;

						if (-data[nread].bath[j] < min_z)
							min_z = -data[nread].bath[j];
						if (-data[nread].bath[j] > max_z)
							max_z = -data[nread].bath[j];
					}
					else {
						data[nread].xp[j] = BAD;
						data[nread].yp[j] = BAD;
					}
				} /* for j=0 ... */

				if (nread == 0) {
					for (k = 0; k < 7; k++)
						timbeg_i[k] = time_i[k];
				}
				else {
					for (k = 0; k < 7; k++)
						timend_i[k] = time_i[k];
				}
			} /* if status==MB_SUCCESS */

			/* increment counters */
			if (error == MB_ERROR_NO_ERROR) {
				nread++;
			}
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Reading loop finished in program <%s>\n", program_name);
			fprintf(stderr, "dbg2       status:     %d\n", status);
			fprintf(stderr, "dbg2       error:      %d\n", error);
			fprintf(stderr, "dbg2       nread:      %d\n", nread);
			fprintf(stderr, "dbg2       pings:      %d\n", pings);
		}

		/* test if done */
		if (nread >= num_pings_max && verbose >= 1) {
			fprintf(stderr, "%s: Maximum number of pings [%d] read before end of file reached...\n", program_name, num_pings_max);
			done = MB_YES;
		}
		if (nread >= num_pings_max || error > MB_ERROR_NO_ERROR) {
			done = MB_YES;
		}

	} /* end of processing data, 1'st while under read/process data */

	/* close the swath file */
	status = mb_close(verbose, &mbio_ptr, &error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Reading loop finished in program <%s>\n", program_name);
		fprintf(stderr, "dbg2       status:     %d\n", status);
		fprintf(stderr, "dbg2       error:      %d\n", error);
		fprintf(stderr, "dbg2       nread:      %d\n", nread);
		fprintf(stderr, "dbg2       pings: %d\n", pings);
	}

	/* total track length in m */
	track_length = distot;
	mean_lat /= mean_knt;
	mean_latmin = fabs(mean_lat - (int)mean_lat) * 60.0;
	mean_lon /= mean_knt;
	mean_lonmin = fabs(mean_lon - (int)mean_lon) * 60.0;
	mean_hdg /= mean_knt;
	mean_xp /= mean_knt;
	mean_yp /= mean_knt;

	/* rescale xp[],yp[] to zero mean; get min and max */
	max_yp = min_yp = max_xp = min_xp = 0.0;
	for (int i = 0; i < nread; i++) {
		beamflag = data[i].beamflag;
		xp = data[i].xp;
		yp = data[i].yp;
		for (int j = 0; j < data[i].beams_bath; j++) {
			if (mb_beam_ok(beamflag[j])) {
				yp[j] -= mean_yp;
				xp[j] -= mean_xp;
				min_xp = MIN(min_xp, xp[j]);
				max_xp = MAX(max_xp, xp[j]);
				min_yp = MIN(min_yp, yp[j]);
				max_yp = MAX(max_yp, yp[j]);
			} /* if yp[][] */
		}     /* for j */
	}         /* for i */

	/* get page orientation, scaling(in/m) factor and startup plot */
	if ((viewdir == 'P') || (viewdir == 'S') || (viewdir == 'p') || (viewdir == 's')) {
		/* Landscape */
		orient = 0;
		if (meters_per_inch > 0.0) {
			scaling = 1.0 / meters_per_inch;
			x_off = 11. / 2;
			y_off = 8.5 / 2.;
		}
		else {
			if ((5.2 / (max_yp - min_yp)) < (8.5 / (max_xp - min_xp)))
				scaling = (5.2 / (max_yp - min_yp));
			else
				scaling = (8.5 / (max_xp - min_xp));
			x_off = (-(max_xp + min_xp) * scaling / 2.0) + (11. / 2);
			y_off = (-(max_yp + min_yp) * scaling / 2.0) + (8.5 / 2) - .2;
		}
	}
	else {
		/* Portrait */
		orient = 1;
		if (meters_per_inch > 0.0) {
			scaling = 1.0 / meters_per_inch;
			x_off = 8.5 / 2.0;
			y_off = 11. / 2.0;
		}
		else {
			if ((8. / (max_yp - min_yp)) < (6.5 / (max_xp - min_xp)))
				scaling = (8. / (max_yp - min_yp));
			else
				scaling = (6.5 / (max_xp - min_xp));
			x_off = (-(max_xp + min_xp) * scaling / 2.0) + (8.5 / 2);
			y_off = (-(max_yp + min_yp) * scaling / 2.0) + (11. / 2) - .2;
		}
	}

/* initialize the Postscript plotting */
#ifdef GMT_MAJOR_VERSION >= 5 && GMT_MINOR_VERSION
	ps_plotinit_hires(NULL, 0, orient, x_off, y_off, 1.0, 1.0, 1, 300, 1, gmtdefs.paper_width, gmtdefs.page_rgb,
	                  gmtdefs.encoding.name, (struct EPS *)GMT_epsinfo(argv[0]));
#else
	ps_plotinit(NULL, 0, orient, x_off, y_off, 1.0, 1.0, 1, 300, 1, gmtdefs.paper_width, gmtdefs.page_rgb, gmtdefs.encoding.name,
	            (struct EPS *)GMT_epsinfo(argv[0]));
#endif
	GMT_echo_command(argc, argv);

	/* now loop over the data in the appropriate order
	    laying down white filled boxes with black outlines
	    wherever the data is good */

	if ((viewdir == 'S') || (viewdir == 's'))
		forward = MB_YES;
	else if ((viewdir == 'P') || (viewdir == 'p'))
		forward = MB_NO;
	else if ((viewdir == 'B') || (viewdir == 'b')) {
		if (alpha < 90.0)
			forward = MB_YES;
		else
			forward = MB_NO;
	}
	for (int j = 0; j < beams_bath - 1; j++) {
		for (int i = 0; i < nread - 1; i++) {
			if (forward == MB_YES)
				jj = j;
			else
				jj = beams_bath - 2 - j;

			/* make box */
			if (mb_beam_ok(data[i].beamflag[jj]) && mb_beam_ok(data[i + 1].beamflag[jj]) &&
			    mb_beam_ok(data[i].beamflag[jj + 1]) && mb_beam_ok(data[i + 1].beamflag[jj + 1])) {
				xl[0] = scaling * data[i].xp[jj];
				yl[0] = scaling * data[i].yp[jj];
				xl[1] = scaling * data[i + 1].xp[jj];
				yl[1] = scaling * data[i + 1].yp[jj];
				xl[2] = scaling * data[i + 1].xp[jj + 1];
				yl[2] = scaling * data[i + 1].yp[jj + 1];
				xl[3] = scaling * data[i].xp[jj + 1];
				yl[3] = scaling * data[i].yp[jj + 1];
				ps_polygon(xl, yl, 4, rgb_white, 1);
			}
		}
	}

	/* titles and such */
	ps_setline(2); /* set line width */

	if (display_stats == MB_NO) {
		/* plot a title */
		xl[0] = 0;
		yl[0] = max_yp * scaling + .6;
		sprintf(label, "%s", title);
		ps_text(xl[0], yl[0], 20., label, 0., 6, 0);
	}
	else {
		/* plot a title */
		xl[0] = 0;
		yl[0] = max_yp * scaling + 1.3;
		sprintf(label, "%s", title);
		ps_text(xl[0], yl[0], 20., label, 0., 6, 0);

		/*xl[0]-=3.25;*/
		yl[0] -= 0.3;
		sprintf(label, "Mean Lat.: %3d@+o@+ %4.1f'   Mean Lon.: %4d@+o @+%4.1f'   Heading: %.1lf@+o @+", (int)mean_lat,
		        mean_latmin, (int)mean_lon, mean_lonmin, mean_hdg);
		ps_text(xl[0], yl[0], 15., label, 0., 6, 0);

		yl[0] -= 0.3;
		sprintf(label, "View Angle: %.1lf@+o @+  V.E.: %.1lfX   Scale: %.0lf m/inch   Track Length: %.1lf km", eta, ve,
		        1.0 / scaling, track_length / 1000.0);
		ps_text(xl[0], yl[0], 15., label, 0., 6, 0);

		yl[0] -= 0.3;
		sprintf(label, "From %.4d/%.2d/%.2d %.2d:%.2d:%.2d   to  %.4d/%.2d/%.2d %.2d:%.2d:%.2d", timbeg_i[0], timbeg_i[1],
		        timbeg_i[2], timbeg_i[3], timbeg_i[4], timbeg_i[5], timend_i[0], timend_i[1], timend_i[2], timend_i[3],
		        timend_i[4], timend_i[5]);
		ps_text(xl[0], yl[0], 15., label, 0., 6, 0);
	} /* else after if display_stats */

	if (display_scales == MB_YES) {
		/* plot the x-scale */
		xscale = 10000; /* x scale in m */
		if (track_length < 50000)
			xscale = 5000;
		if (track_length < 20000)
			xscale = 2000;
		if (track_length < 10000)
			xscale = 1000;
		xl[0] = xl[1] = (-xscale * scaling / 2.0);
		xl[2] = xl[3] = (-xl[0]);
		xl[0] += 2.;
		xl[1] += 2.;
		xl[2] += 2.;
		xl[3] += 2.;
		yl[1] = yl[2] = min_yp * scaling - 1.;
		yl[0] = yl[3] = yl[1] + 0.1;

#ifdef GMT_MAJOR_VERSION >= 5 && GMT_MINOR_VERSION
		ps_line(xl, yl, 4, 3, 0);
#else
		ps_line(xl, yl, 4, 3, 0, 0);
#endif
		sprintf(label, "%.0f km", xscale / 1000.0);
		ps_text(xl[0] + .5, yl[0] + .05, 15., label, 0., 6, 0);

		/* plot the z-scale */
		range_z = (max_z - min_z);
		zscale = 2000; /* z scale in m */
		if (range_z < 3000)
			zscale = 1000;
		if (range_z < 1000)
			zscale = 500;
		if (range_z < 500)
			zscale = 200;
		if (range_z < 250)
			zscale = 100;
		zscale_inch = zscale * scaling * cos_eta * ve;
		xl[1] = xl[2] + 0.5;
		xl[2] = xl[1];
		xl[0] = xl[3] = xl[1] + .1;
		yl[0] = yl[1] = min_yp * scaling - 1.;
		yl[2] = yl[3] = yl[0] + zscale_inch;

#ifdef GMT_MAJOR_VERSION >= 5 && GMT_MINOR_VERSION
		ps_line(xl, yl, 4, 3, 0);
#else
		ps_line(xl, yl, 4, 3, 0, 0);
#endif
		sprintf(label, "%.0f m", zscale);
		ps_text(xl[0] + 0.3, yl[0] + zscale_inch / 2.0, 15., label, 0., 6, 0);

		/* plot an arrow in the ship's direction */
		a = 0;
		b = beams_bath / 2;
		while (!mb_beam_ok(data[a++].beamflag[b])) {
		}
		xl[0] = data[--a].xp[b];
		yl[0] = data[a].yp[b];
		a = nread - 1;
		while (!mb_beam_ok(data[a--].beamflag[b])) {
		}
		xl[1] = data[++a].xp[b];
		yl[1] = data[a].yp[b];
		xl[1] = ((xl[1] - xl[0]) / distot / 2) + .6;
		yl[1] = ((yl[1] - yl[0]) / distot / 2) + min_yp * scaling - 1.;
		xl[0] = 0. + .6;
		yl[0] = 0. + min_yp * scaling - 0.85;
		ps_vector(xl[0], yl[0], xl[1], yl[1], 0.01, 0.25, 0.1, 1.0, rgb_black, 0);
		ps_text(xl[0] - 1.7, yl[0] + .2, 15., "ship heading", 0., 1, 0);
		ps_text(xl[0] - 1.7, yl[0], 15., "direction", 0., 1, 0);

		/* plot the three axes */
		for (int i = 0; i < 3; i++) {
			xl[0] = 0.; /* point in center of page */
			yl[0] = 0.;
			rotate = 0; /* set to 1 if arrow is rotated below */
			if (i == 0) {
				/* x-axis */
				x = 1.;
				y = 0;
				z = 0;
			}
			else if (i == 1) {
				/* y-axis */
				x = 0;
				y = 1.;
				z = 0;
			}
			else if (i == 2) {
				/* z-axis */
				x = 0;
				y = 0;
				z = -1.;
			}

			if (viewdir == 'P' || viewdir == 'p') {
				xl[1] = -y - x * sin_eta * cos_alpha + xl[0];
				yl[1] = -z * cos_eta + x * sin_eta * sin_alpha + yl[0];
			}
			else if (viewdir == 'B' || viewdir == 'b') {
				xl[1] = (x + y * sin_eta * cos_alpha) + xl[0];
				yl[1] = -z * cos_eta + y * sin_eta * sin_alpha + yl[0];
			}
			else if (viewdir == 'S' || viewdir == 's') {
				xl[1] = y + x * sin_eta * cos_alpha + xl[0];
				yl[1] = z * cos_eta - x * sin_eta * sin_alpha + yl[0];
			}

			if (yl[1] < yl[0]) {
				/* rotate arrows 180 if facing downward */
				xl[1] = -xl[1];
				yl[1] = -yl[1];
				rotate = 1;
			}

			xl[0] = (-3.); /* move arrows from center to lower left corner */
			yl[0] = (min_yp * scaling - 1.);
			xl[1] = xl[0] + xl[1];
			yl[1] = yl[0] + yl[1];

			ps_vector(xl[0], yl[0], xl[1], yl[1], 0.01, 0.25, 0.1, 1.0, rgb_black, 0);

			if (i == 0 && rotate == 0)
				ps_text(xl[1], yl[1] + .15, 15., "x", 0., 6, 0);
			else if (i == 1 && rotate == 0)
				ps_text(xl[1], yl[1] + .15, 15., "y", 0., 6, 0);
			else if (i == 2 && rotate == 0)
				ps_text(xl[1], yl[1] + .15, 15., "z", 0., 6, 0);
			else if (i == 0 && rotate == 1)
				ps_text(xl[1], yl[1] + .15, 15., "-x", 0., 6, 0);
			else if (i == 1 && rotate == 1)
				ps_text(xl[1], yl[1] + .15, 15., "-y", 0., 6, 0);
			else if (i == 2 && rotate == 1)
				ps_text(xl[1], yl[1] + .15, 15., "z", 0., 6, 0);

		} /* (i=0;i<3;i++) */
	}     /* if display_scales */

	/* end the postscript file */
	ps_plotend(1);

	/* deallocate arrays */
	for (int i = 0; i < nread; i++) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(data[i].beams_bath), &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(data[i].bath), &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(data[i].bathacrosstrack), &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(data[i].bathalongtrack), &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(data[i].xp), &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(data[i].yp), &error);
	}

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	exit(error);

} /* main */

/*--------------------------------------------------------------------*/
