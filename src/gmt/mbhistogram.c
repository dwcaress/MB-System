/*--------------------------------------------------------------------
 *    The MB-system:	mbhistogram.c	12/28/94
 *
 *    Copyright (c) 1993-2025 by
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
 * MBHISTOGRAM reads a swath sonar data file and generates a histogram
 * of the bathymetry,  amplitude,  or sidescan values. Alternatively,
 * mbhistogram can output a list of values which break up the
 * distribution into equal sized regions.
 * The results are dumped to stdout.
 *
 * Author:	D. W. Caress
 * Date:	December 28, 1994
 *
 * GMT-module rewrite of mbhistogram.cc: wrapped as GMT_mbhistogram entry
 * so it can be invoked from the GMT API (Julia FFI / Matlab MEX).
 */

#define THIS_MODULE_NAME		"mbhistogram"
#define THIS_MODULE_LIB			"mbsystem"
#define THIS_MODULE_PURPOSE		"Generate histogram of bathymetry, amplitude, or sidescan values from swath sonar data"
#define THIS_MODULE_KEYS		">D}"
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"-:>RVh"

#include "gmt_dev.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

typedef enum {
	MBHISTOGRAM_BATH = 0,
	MBHISTOGRAM_AMP  = 1,
	MBHISTOGRAM_SS   = 2,
} histogram_mode_t;

static const char help_message[] =
    "MBHISTOGRAM reads a swath sonar data file and generates a histogram\n"
    "\tof the bathymetry,  amplitude, or sidescan values. Alternatively,\n"
    "\tmbhistogram can output a list of values which break up the\n"
    "\tdistribution into equal sized regions.\n"
    "\tThe results are dumped to stdout.";
static const char usage_message[] =
    "mbhistogram [-Akind -Byr/mo/da/hr/mn/sc -Dmin/max -Eyr/mo/da/hr/mn/sc -Fformat -G -Ifile -Llonflip "
    "-Mnintervals -Nnbins -Ppings -Rw/e/s/n -Sspeed -V -H]";

/*--------------------------------------------------------------------*/

/* double qsnorm(p)
 * double	p;
 *
 * Function to invert the cumulative normal probability
 * function.  If z is a standardized normal random deviate,
 * and Q(z) = p is the cumulative Gaussian probability
 * function, then z = qsnorm(p).
 *
 * Note that 0.0 < p < 1.0.  Data values outside this range
 * will return +/- a large number (1.0e6).
 * To compute p from a sample of data to test for Normalcy,
 * sort the N samples into non-decreasing order, label them
 * i=[1, N], and then compute p = i/(N+1).
 *
 * Author:	Walter H. F. Smith
 * Date:	19 February, 1991-1995.
 *
 * Based on a Fortran subroutine by R. L. Parker.  I had been
 * using IMSL library routine DNORIN(DX) to do what qsnorm(p)
 * does, when I was at the Lamont-Doherty Geological Observatory
 * which had a site license for IMSL.  I now need to invert the
 * gaussian CDF without calling IMSL; hence, this routine.
 *
 */

static double qsnorm(double p) {
	if (p <= 0.0) {
		return (-1.0e6);
	}
	if (p >= 1.0) {
		return (1.0e6);
	}
	if (p == 0.5) {
		return (0.0);
	}
	if (p > 0.5) {
		const double t = sqrt(-2.0 * log(1.0 - p));
		const double z = t - (2.515517 + t * (0.802853 + t * 0.010328)) / (1.0 + t * (1.432788 + t * (0.189269 + t * 0.001308)));
		return (z);
	}

	const double t = sqrt(-2.0 * log(p));
	const double z = t - (2.515517 + t * (0.802853 + t * 0.010328)) / (1.0 + t * (1.432788 + t * (0.189269 + t * 0.001308)));
	return (-z);
}

/*--------------------------------------------------------------------*/

/* --- Control structure ---------------------------------------------- */

struct MBHISTOGRAM_CTRL {
	struct mhi_A { bool active; histogram_mode_t mode; } A;
	struct mhi_B { bool active; int t[7]; } B;
	struct mhi_D { bool active; double value_min, value_max; } D;
	struct mhi_E { bool active; int t[7]; } E;
	struct mhi_F { bool active; int format; } F;
	struct mhi_G { bool active; } G;                                /* gaussian */
	struct mhi_I { bool active; char *inputfile; } I;
	struct mhi_L { bool active; int lonflip; } L;
	struct mhi_M { bool active; int nintervals; } M;
	struct mhi_N { bool active; int nbins; } N;
	struct mhi_P { bool active; int pings; } P;
	struct mhi_R { bool active; double bounds[4]; } R;
	struct mhi_S { bool active; double speedmin; } S;
	struct mhi_T { bool active; double timegap; } T;
};

static void *New_mbhistogram_Ctrl(struct GMT_CTRL *GMT) {
	struct MBHISTOGRAM_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBHISTOGRAM_CTRL);
	Ctrl->A.mode = MBHISTOGRAM_SS;
	Ctrl->D.value_min = 0.0;
	Ctrl->D.value_max = 128.0;
	Ctrl->M.nintervals = 0;
	Ctrl->N.nbins = 0;
	return Ctrl;
}

static void Free_mbhistogram_Ctrl(struct GMT_CTRL *GMT, struct MBHISTOGRAM_CTRL *Ctrl) {
	if (!Ctrl) return;
	if (Ctrl->I.inputfile) free(Ctrl->I.inputfile);
	gmt_M_free(GMT, Ctrl);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;
	GMT_Message(API, GMT_TIME_NONE, "usage: %s\n\n", usage_message);
	if (level == GMT_SYNOPSIS) return GMT_PARSE_ERROR;
	GMT_Message(API, GMT_TIME_NONE, "\n%s\n\n", help_message);
	return GMT_PARSE_ERROR;
}

static int parse(struct GMT_CTRL *GMT, struct MBHISTOGRAM_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0, n_files = 0;
	int n;
	struct GMT_OPTION *opt;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
		case '<':
			Ctrl->I.active = true;
			if (gmt_check_filearg(GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) {
				Ctrl->I.inputfile = strdup(opt->arg); n_files = 1;
			} else n_errors++;
			break;
		case 'A': {
			int tmp;
			if (sscanf(opt->arg, "%d", &tmp) > 0) {
				Ctrl->A.mode = (histogram_mode_t)tmp;
				Ctrl->A.active = true;
			} else n_errors++;
			break;
		}
		case 'B':
			Ctrl->B.t[6] = 0;
			n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d", &Ctrl->B.t[0], &Ctrl->B.t[1], &Ctrl->B.t[2],
			           &Ctrl->B.t[3], &Ctrl->B.t[4], &Ctrl->B.t[5]);
			if (n == 6) Ctrl->B.active = true; else n_errors++;
			break;
		case 'D':
			n = sscanf(opt->arg, "%lf/%lf", &Ctrl->D.value_min, &Ctrl->D.value_max);
			if (n == 2) Ctrl->D.active = true; else n_errors++;
			break;
		case 'E':
			Ctrl->E.t[6] = 0;
			n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d", &Ctrl->E.t[0], &Ctrl->E.t[1], &Ctrl->E.t[2],
			           &Ctrl->E.t[3], &Ctrl->E.t[4], &Ctrl->E.t[5]);
			if (n == 6) Ctrl->E.active = true; else n_errors++;
			break;
		case 'F':
			if (sscanf(opt->arg, "%d", &Ctrl->F.format) > 0) Ctrl->F.active = true; else n_errors++;
			break;
		case 'G':
			Ctrl->G.active = true;
			break;
		case 'I':
			if (!gmt_access(GMT, opt->arg, R_OK)) {
				Ctrl->I.inputfile = strdup(opt->arg); Ctrl->I.active = true; n_files = 1;
			} else n_errors++;
			break;
		case 'L':
			if (sscanf(opt->arg, "%d", &Ctrl->L.lonflip) > 0) Ctrl->L.active = true; else n_errors++;
			break;
		case 'M':
			if (sscanf(opt->arg, "%d", &Ctrl->M.nintervals) > 0) Ctrl->M.active = true; else n_errors++;
			break;
		case 'N':
			if (sscanf(opt->arg, "%d", &Ctrl->N.nbins) > 0) Ctrl->N.active = true; else n_errors++;
			break;
		case 'P':
			if (sscanf(opt->arg, "%d", &Ctrl->P.pings) > 0) Ctrl->P.active = true; else n_errors++;
			break;
		case 'R':
			mb_get_bounds(opt->arg, Ctrl->R.bounds);
			Ctrl->R.active = true;
			break;
		case 'S':
			if (sscanf(opt->arg, "%lf", &Ctrl->S.speedmin) > 0) Ctrl->S.active = true; else n_errors++;
			break;
		case 'T':
			if (sscanf(opt->arg, "%lf", &Ctrl->T.timegap) > 0) Ctrl->T.active = true; else n_errors++;
			break;
		default:
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	(void)n_files;
	return n_errors ? GMT_PARSE_ERROR : GMT_OK;
}

#define bailout(code)  { gmt_M_free_options(mode); return code; }
#define Return(code)   { Free_mbhistogram_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

/*--------------------------------------------------------------------*/
int GMT_mbhistogram(void *V_API, int mode, void *args) {
	int error = MB_ERROR_NO_ERROR;

	struct MBHISTOGRAM_CTRL *Ctrl = NULL;
	struct GMT_CTRL         *GMT  = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION       *options = NULL;
	struct GMTAPI_CTRL      *API = gmt_get_api_ptr(V_API);

	if (API == NULL) return GMT_NOT_A_SESSION;
	if (mode == GMT_MODULE_PURPOSE) return usage(API, GMT_MODULE_PURPOSE);
	options = GMT_Create_Options(API, mode, args);
	if (API->error) return API->error;
	if (!options || options->option == GMT_OPT_USAGE)    bailout(usage(API, GMT_USAGE));
	if (options->option == GMT_OPT_SYNOPSIS)             bailout(usage(API, GMT_SYNOPSIS));

#if GMT_MAJOR_VERSION >= 6
	if ((GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout(API->error);
#else
	GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy);
#endif
	if (GMT_Parse_Common(API, THIS_MODULE_OPTIONS, options)) Return(API->error);

	Ctrl = New_mbhistogram_Ctrl(GMT);
	if ((error = parse(GMT, Ctrl, options)) != 0) Return (error);

	int verbose = GMT->common.V.active;
	int format, pings, lonflip;
	double bounds[4];
	int btime_i[7], etime_i[7];
	double speedmin, timegap;
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	if (Ctrl->F.active) format   = Ctrl->F.format;
	if (Ctrl->P.active) pings    = Ctrl->P.pings;
	if (Ctrl->L.active) lonflip  = Ctrl->L.lonflip;
	if (Ctrl->S.active) speedmin = Ctrl->S.speedmin;
	if (Ctrl->T.active) timegap  = Ctrl->T.timegap;
	if (Ctrl->R.active) for (int i = 0; i < 4; i++) bounds[i] = Ctrl->R.bounds[i];
	if (Ctrl->B.active) for (int i = 0; i < 7; i++) btime_i[i] = Ctrl->B.t[i];
	if (Ctrl->E.active) for (int i = 0; i < 7; i++) etime_i[i] = Ctrl->E.t[i];

	char read_file[MB_PATH_MAXLINE];
	strcpy(read_file, Ctrl->I.active ? Ctrl->I.inputfile : "stdin");

	histogram_mode_t hmode = Ctrl->A.active ? Ctrl->A.mode : MBHISTOGRAM_SS;
	double value_min  = Ctrl->D.value_min;
	double value_max  = Ctrl->D.value_max;
	bool   gaussian   = Ctrl->G.active;
	int    nintervals = Ctrl->M.nintervals;
	int    nbins      = Ctrl->N.nbins;

	if (verbose == 1) {
		GMT_Report(API, GMT_MSG_NORMAL, "\nProgram %s\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "MB-system Version %s\n", MB_VERSION);
	}

	if (format == 0)
		mb_get_format(verbose, read_file, NULL, &format, &error);

	/* figure out histogram dimensions */
	if (nintervals > 0 && nbins <= 0)
		nbins = 50 * nintervals;
	if (nbins <= 0)
		nbins = 16;

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s>\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  MB-system Version %s\n", MB_VERSION);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Control Parameters:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       verbose:    %d\n", verbose);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       format:     %d\n", format);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       pings:      %d\n", pings);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       lonflip:    %d\n", lonflip);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       bounds[0]:  %f\n", bounds[0]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       bounds[1]:  %f\n", bounds[1]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       bounds[2]:  %f\n", bounds[2]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       bounds[3]:  %f\n", bounds[3]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[0]: %d\n", btime_i[0]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[1]: %d\n", btime_i[1]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[2]: %d\n", btime_i[2]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[3]: %d\n", btime_i[3]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[4]: %d\n", btime_i[4]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[5]: %d\n", btime_i[5]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[6]: %d\n", btime_i[6]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[0]: %d\n", etime_i[0]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[1]: %d\n", etime_i[1]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[2]: %d\n", etime_i[2]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[3]: %d\n", etime_i[3]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[4]: %d\n", etime_i[4]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[5]: %d\n", etime_i[5]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[6]: %d\n", etime_i[6]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       speedmin:   %f\n", speedmin);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       timegap:    %f\n", timegap);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       file:       %s\n", read_file);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       mode:       %d\n", hmode);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       gaussian:   %d\n", gaussian);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       nbins:      %d\n", nbins);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       nintervals: %d\n", nintervals);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       value_min:  %f\n", value_min);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       value_max:  %f\n", value_max);
	}


	/* MBIO read control parameters */
	void *datalist;
	double file_weight;
	double btime_d;
	double etime_d;
	char file[MB_PATH_MAXLINE];
	char dfile[MB_PATH_MAXLINE];
	int beams_bath;
	int beams_amp;
	int pixels_ss;

	/* MBIO read values */
	void *mbio_ptr = NULL;
	int kind;
	int time_i[7];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double heading;
	double distance;
	double altitude;
	double sensordepth;
	char *beamflag = NULL;
	double *bath = NULL;
	double *bathacrosstrack = NULL;
	double *bathalongtrack = NULL;
	double *amp = NULL;
	double *ss = NULL;
	double *ssacrosstrack = NULL;
	double *ssalongtrack = NULL;
	char comment[MB_COMMENT_MAXLINE];

	/* histogram variables */
	double dvalue_bin;
	double *histogram = NULL;
	double *intervals = NULL;
	double total;
	double target;
	double dinterval;
	double bin_fraction;
	int ibin;

	int nrectot = 0;
	int nvaluetot = 0;

	/* allocate memory for histogram arrays */
	if (error == MB_ERROR_NO_ERROR)
		status = mb_mallocd(verbose, __FILE__, __LINE__, nbins * sizeof(double), (void **)&histogram, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_mallocd(verbose, __FILE__, __LINE__, nintervals * sizeof(double), (void **)&intervals, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		char *message;
		mb_error(verbose, error, &message);
		GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating histogram arrays:\n%s\n", message);
		GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
		Return(error);
	}

	/* output some information */
	if (verbose > 0) {
		GMT_Report(API, GMT_MSG_NORMAL, "\nNumber of data bins: %d\n", nbins);
		GMT_Report(API, GMT_MSG_NORMAL, "Minimum value:         %f\n", value_min);
		GMT_Report(API, GMT_MSG_NORMAL, "Maximum value:         %f\n", value_max);
		if (hmode == MBHISTOGRAM_BATH)
			GMT_Report(API, GMT_MSG_NORMAL, "Working on bathymetry data...\n");
		else if (hmode == MBHISTOGRAM_AMP)
			GMT_Report(API, GMT_MSG_NORMAL, "Working on beam amplitude data...\n");
		else
			GMT_Report(API, GMT_MSG_NORMAL, "Working on sidescan data...\n");
	}

	/* get size of bins */
	dvalue_bin = (value_max - value_min) / (nbins - 1);
	const double value_bin_min = value_min - 0.5 * dvalue_bin;
	/* const double value_bin_max = value_max + 0.5 * dvalue_bin; */

	/* initialize histogram */
	for (int i = 0; i < nbins; i++)
		histogram[i] = 0;

	/* determine whether to read one file or a list of files */
	const bool read_datalist = format < 0;
	bool read_data;

	/* open file list */
	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", read_file);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
			mb_freed(verbose, __FILE__, __LINE__, (void **)&histogram, &error);
			mb_freed(verbose, __FILE__, __LINE__, (void **)&intervals, &error);
			Return(MB_ERROR_OPEN_FAIL);
		}
		read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
	} else {
		/* else copy single filename to be read */
		strcpy(file, read_file);
		read_data = true;
	}

	double data_min = INFINITY;
	double data_max = -INFINITY;
	bool data_first = true;

	/* loop over all files to be read */
	while (read_data) {

		/* obtain format array location - format id will
		    be aliased to current id if old format id given */
		status = mb_format(verbose, &format, &error);

		/* initialize reading the swath sonar data file */
		if (mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
		                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMultibeam File <%s> not initialized for reading\n", file);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
			mb_freed(verbose, __FILE__, __LINE__, (void **)&histogram, &error);
			mb_freed(verbose, __FILE__, __LINE__, (void **)&intervals, &error);
			if (read_datalist) mb_datalist_close(verbose, &datalist, &error);
			Return(error);
		}

		/* allocate memory for data arrays */
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
			    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
			    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
			mb_close(verbose, &mbio_ptr, &error);
			mb_freed(verbose, __FILE__, __LINE__, (void **)&histogram, &error);
			mb_freed(verbose, __FILE__, __LINE__, (void **)&intervals, &error);
			if (read_datalist) mb_datalist_close(verbose, &datalist, &error);
			Return(error);
		}

		/* output information */
		if (error == MB_ERROR_NO_ERROR && verbose > 0) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nprocessing file: %s %d\n", file, format);
		}

		/* initialize counting variables */
		int nrec = 0;
		int nvalue = 0;

		/* read and process data */
		while (error <= MB_ERROR_NO_ERROR) {

			/* read a ping of data */
			status = mb_get(verbose, mbio_ptr, &kind, &pings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance,
			                &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathacrosstrack,
			                bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

			/* process the pings */
			if (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP) {
				/* increment record counter */
				nrec++;

				/* do the bathymetry */
				if (hmode == MBHISTOGRAM_BATH)
					for (int i = 0; i < beams_bath; i++) {
						if (mb_beam_ok(beamflag[i])) {
							nvalue++;
							const int j = (bath[i] - value_bin_min) / dvalue_bin;
							if (j >= 0 && j < nbins)
								histogram[j]++;
							if (data_first) {
								data_min = bath[i];
								data_max = bath[i];
								data_first = false;
							}
							else {
								if (bath[i] < data_min) data_min = bath[i];
								if (bath[i] > data_max) data_max = bath[i];
							}
						}
					}

				/* do the amplitude */
				if (hmode == MBHISTOGRAM_AMP)
					for (int i = 0; i < beams_amp; i++) {
						if (mb_beam_ok(beamflag[i])) {
							nvalue++;
							const int j = (amp[i] - value_bin_min) / dvalue_bin;
							if (j >= 0 && j < nbins)
								histogram[j]++;
							if (data_first) {
								data_min = amp[i];
								data_max = amp[i];
								data_first = false;
							}
							else {
								if (amp[i] < data_min) data_min = amp[i];
								if (amp[i] > data_max) data_max = amp[i];
							}
						}
					}

				/* do the sidescan */
				if (hmode == MBHISTOGRAM_SS)
					for (int i = 0; i < pixels_ss; i++) {
						if (ss[i] > MB_SIDESCAN_NULL) {
							nvalue++;
							const int j = (ss[i] - value_bin_min) / dvalue_bin;
							if (j >= 0 && j < nbins)
								histogram[j]++;
							if (data_first) {
								data_min = ss[i];
								data_max = ss[i];
								data_first = false;
							}
							else {
								if (ss[i] < data_min) data_min = ss[i];
								if (ss[i] > data_max) data_max = ss[i];
							}
						}
					}
			}
		}

		/* close the swath sonar data file */
		status &= mb_close(verbose, &mbio_ptr, &error);
		nrectot += nrec;
		nvaluetot += nvalue;

		/* output information */
		if (error == MB_ERROR_NO_ERROR && verbose > 0) {
			GMT_Report(API, GMT_MSG_NORMAL, "%d records processed\n%d data processed\n", nrec, nvalue);
		}

		/* figure out whether and what to read next */
		if (read_datalist) {
			read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
		} else {
			read_data = false;
		}

		/* end loop over files in list */
	}
	if (read_datalist)
		mb_datalist_close(verbose, &datalist, &error);

	/* output information */
	if (error == MB_ERROR_NO_ERROR && verbose > 0) {
		GMT_Report(API, GMT_MSG_NORMAL, "\n%d total records processed\n", nrectot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total data processed\n\n", nvaluetot);
	}

	/* recast histogram as gaussian */
	if (gaussian) {
		/* get total number of good values */
		total = 0.0;
		for (int i = 0; i < nbins; i++)
			total = total + histogram[i];

		/* recast histogram */
		double sum = 0.0;
		for (int i = 0; i < nbins; i++) {
			const double p = (histogram[i] / 2 + sum) / (total + 1);
			sum = sum + histogram[i];
			histogram[i] = qsnorm(p);
		}
	}

	/* calculate gaussian intervals if required */
	if (nintervals > 0 && gaussian) {
		/* get interval spacing */
		double target_min = -2.0;
		double target_max = 2.0;
		dinterval = (target_max - target_min) / (nintervals - 1);

		/* get intervals */
		intervals[0] = (data_min > value_min) ? data_min : value_min;
		intervals[nintervals - 1] = (data_max < value_max) ? data_max : value_max;
		ibin = 0;
		for (int j = 1; j < nintervals - 1; j++) {
			target = target_min + j * dinterval;
			while (ibin < nbins - 1 && histogram[ibin] < target)
				ibin++;
			if (ibin > 0)
				bin_fraction = 1.0 - (histogram[ibin] - target) / (histogram[ibin] - histogram[ibin - 1]);
			else
				bin_fraction = 0.0;
			intervals[j] = value_bin_min + dvalue_bin * ibin + bin_fraction * dvalue_bin;
		}
	}

	/* calculate linear intervals if required */
	else if (nintervals > 0) {
		/* get total number of good values */
		total = 0.0;
		for (int i = 0; i < nbins; i++)
			total = total + histogram[i];

		/* get interval spacing */
		dinterval = total / (nintervals - 1);

		/* get intervals */
		intervals[0] = value_bin_min;
		total = 0.0;
		ibin = -1;
		for (int j = 1; j < nintervals; j++) {
			target = j * dinterval;
			while (total < target && ibin < nbins - 1) {
				ibin++;
				total = total + histogram[ibin];
				if (total <= 0.0)
					intervals[0] = value_bin_min + dvalue_bin * ibin;
			}
			bin_fraction = 1.0 - (total - target) / histogram[ibin];
			intervals[j] = value_bin_min + dvalue_bin * ibin + bin_fraction * dvalue_bin;
		}
	}

	/* print out the results */
	if (nintervals <= 0 && gaussian) {
		for (int i = 0; i < nbins; i++) {
			fprintf(stdout, "%f %f\n", value_min + i * dvalue_bin, histogram[i]);
		}
	}
	else if (nintervals <= 0) {
		for (int i = 0; i < nbins; i++) {
			fprintf(stdout, "%f %d\n", value_min + i * dvalue_bin, (int)histogram[i]);
		}
	}
	else {
		for (int i = 0; i < nintervals; i++)
			fprintf(stdout, "%f\n", intervals[i]);
	}

	/* deallocate memory used for data arrays */
	mb_freed(verbose, __FILE__, __LINE__, (void **)&histogram, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&intervals, &error);

	/* check memory */
	if (verbose >= 4)
		status &= mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s> completed\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Ending status:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:  %d\n", status);
	}

	fprintf(stdout, "\n");
	Return(error);
}
/*--------------------------------------------------------------------*/
