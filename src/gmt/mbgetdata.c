/*--------------------------------------------------------------------
 *    The MB-system:	mbgetdata.c
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License or GNU Lesser
 * General Public License as published by the Free Software Foundation,
 * either version 2 of the Licenses, or (at your option) any later version.
 *--------------------------------------------------------------------*/
/*
 * MBGETDATA is a GMT supplement module utility which 
 *
 * Author:	J. Luis but based on other MB codes
 * Date:	27 April, 2017
 *
 */

#define THIS_MODULE_NAME	"mbgetdata"
#define THIS_MODULE_LIB			"mbsystem"
#define THIS_MODULE_PURPOSE	"Get swath bathymetry, amplitude, or backscatter data"
#define THIS_MODULE_KEYS	"<D{,ND),MD}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->JRV" GMT_OPT("S")

/* GMT5 header file */
#include "gmt_dev.h"

/* MBIO include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_info.h"
#include "mb_io.h"
#include "mb_process.h"

/* global structure definitions */
static struct ping_local {
	int	pings, kind;
	int	time_i[7];
	double	time_d, navlon, navlat, speed, heading, distance, altitude, sonardepth;
	int     beams_bath, beams_amp, pixels_ss;
	char   *beamflag;
	double *bath;
	double *bathlon;
	double *bathlat;
	double *amp;
	double *ss;
	double *sslon;
	double *sslat;
	char	comment[256];
};

/* Control structure for mbgetdata */
static struct MBGETDATA_CTRL {
	double  bounds[4];
	int     format;
	double	btime_d;
	double	etime_d;
	int     read_datalist;
	void	*datalist;
	double	file_weight;
	int     beams_bath_max;
	int     beams_amp_max;
	int     pixels_ss_max;
	void   *mbio_ptr;
	struct  ping_local data;

	struct mbgetdata_b {	/* -b<year>/<month>/<day>/<hour>/<minute>/<second> */
		bool active;
		int time_i[7];
	} b;
	struct mbgetdata_e {	/* -e<year>/<month>/<day>/<hour>/<minute>/<second> */
		bool active;
		int time_i[7];
	} e;
	struct mswath_p {		/* -p<pings> */
		bool active;
		int pings;
	} p;
	struct mbgetdata_A {	/* -A apply flags */
		bool active;
		bool no_flags;
		double value;
	} A;
	struct mbgetdata_C {	/* -C */
		bool active;
		int type;		/* 0 -> SS; 1 -> Amp */
	} C;
	struct mbgetdata_D {	/* -D<mode>/<ampscale>/<ampmin>/<ampmax> */
		bool active;
		unsigned int mode;
		double ampscale;
		double ampmin;
		double ampmax;
	} D;
	struct mbgetdata_F {	/* -F<format> */
		bool active;
		int format;
	} F;
	struct mbgetdata_I {	/* -I<inputfile> */
		bool active;
		char *file;
	} I;
	struct mbgetdata_L {	/* -L<lonflip> */
		bool active;
		int lonflip;
	} L;
	struct mbgetdata_M {	/* -M<out_fname> */
		bool active;
		char *file;
	} M;
	struct mbgetdata_N {	/* -N<index_file> */
		bool active;
		char *file;
	} N;
	struct mbgetdata_R {	/* fake -R */
		bool active;
	} Rfake;
	struct mbgetdata_S {	/* -S<speed> */
		bool active;
		double speed;
	} S;
	struct mbgetdata_T {	/* -T<timegap> */
		bool active;
		double timegap;
	} T;
	struct mbgetdata_Z {	/* -Z<mode> */
		bool active;
		char *file;
	} Z;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct  MBGETDATA_CTRL *Ctrl;
	int     status;
	int     verbose = 0;
	double  dummybounds[4];
	int     dummyformat, dummypings;

	Ctrl = gmt_M_memory (GMT, NULL, 1, struct MBGETDATA_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	/* get current mb default values */
	status = mb_defaults(verbose, &dummyformat, &dummypings, &Ctrl->L.lonflip, dummybounds,
						 Ctrl->b.time_i, Ctrl->e.time_i, &Ctrl->S.speed, &Ctrl->T.timegap);

	if (status == MB_FAILURE)
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Probable fatal error: failed to get current MB defaults.\n");

	Ctrl->D.mode = 1;
	Ctrl->D.ampscale = 1.0;
	Ctrl->D.ampmax = 1.0;
	Ctrl->I.file = NULL;
	Ctrl->p.pings = 1;
	Ctrl->T.timegap = 1.0;
		
	/* mbswath variables */
	Ctrl->read_datalist = MB_NO;
	Ctrl->datalist = NULL;
	Ctrl->mbio_ptr = NULL;

	return (Ctrl);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct MBGETDATA_CTRL *Ctrl) {	/* Deallocate control structure */
	if (!Ctrl) return;
	if (Ctrl->I.file) free (Ctrl->I.file);
	if (Ctrl->M.file) free (Ctrl->M.file);
	if (Ctrl->N.file) free (Ctrl->N.file);
	if (Ctrl->Z.file) free (Ctrl->Z.file);
	gmt_M_free (GMT, Ctrl);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: mbgetdata -I<inputfile> [-A[value]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-b<year>/<month>/<day>/<hour>/<minute>/<second>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-e<year>/<month>/<day>/<hour>/<minute>/<second>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-C[a]] [-D<mode>/<ampscale>/<ampmin>/<ampmax>] [-F<format>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-I<inputfile>] [-L<lonflip>] [-N<index_file>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-S<speed>] [-T<timegap>] [-W] [-Z<mode>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-T] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<inputfile> is an MB-System datalist referencing the swath data to be plotted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Replace flagged beans with NaN. Use -A<val> to assign a constant value to the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   flagged beans. Use negative <val> to also search for flagged beans in .esf files.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Output SideScan, or amplitude if -Ca, instead of bathymetry. This case ignores -A\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-p<pings> Sets the ping averaging of the input data [Default = 1, i.e. no ping average].\n");
	GMT_Option (API, "R");
	GMT_Option (API, "U,V,.");

	return (EXIT_FAILURE);
}

static int parse(struct GMT_CTRL *GMT, struct MBGETDATA_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to mbswath and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;
	int     n;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one or three is accepted) */
				Ctrl->I.active = true;
				if (gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) {
					Ctrl->I.file = strdup (opt->arg);
					n_files = 1;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: only one input file is allowed.\n");
					n_errors++;
				}
				break;

			/* Processes program-specific parameters */

			case 'b':	/* btime_i */
				n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d",
						   &(Ctrl->b.time_i[0]), &(Ctrl->b.time_i[1]), &(Ctrl->b.time_i[2]),
						   &(Ctrl->b.time_i[3]), &(Ctrl->b.time_i[4]), &(Ctrl->b.time_i[5]));
				Ctrl->b.time_i[6] = 0;
				if (n == 6)
					Ctrl->b.active = true;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -b option: \n");
					n_errors++;
				}
				break;
			case 'e':	/* etime_i */
				n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d",
						   &(Ctrl->e.time_i[0]), &(Ctrl->e.time_i[1]), &(Ctrl->e.time_i[2]),
						   &(Ctrl->e.time_i[3]), &(Ctrl->e.time_i[4]), &(Ctrl->e.time_i[5]));
				Ctrl->e.time_i[6] = 0;
				if (n == 6)
					Ctrl->e.active = true;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -e option: \n");
					n_errors++;
				}
				break;
			case 'p':	/* Sets the ping averaging */
				Ctrl->p.active = true;
				Ctrl->p.pings = atoi(opt->arg);
				if (Ctrl->p.pings < 0) {
					GMT_Report (API, GMT_MSG_NORMAL, "Error -p option: Don't invent, number of pings must be >= 0\n");
					Ctrl->p.pings = 1;
				}
				break;

			case 'A':	/* Apply flags, Make z nan or add a flag value */
				Ctrl->A.active = true;
				if (opt->arg) Ctrl->A.value = atof(opt->arg);
				if (Ctrl->A.value < 0) {
					Ctrl->A.no_flags = true;
					Ctrl->A.value *= -1;
				}
				break;
			case 'C':	/* SideScan or Amplitude */
				Ctrl->C.active = true;
				if (opt->arg && opt->arg[0] == 'a') Ctrl->C.type = 1;
				break;
			case 'D':	/* amplitude scaling */
				n = sscanf(opt->arg, "%d/%lf/%lf/%lf", &(Ctrl->D.mode), &(Ctrl->D.ampscale),
				           &(Ctrl->D.ampmin), &(Ctrl->D.ampmax));
				if (n > 0)
					Ctrl->D.active = true;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -D option: \n");
					n_errors++;
				}
				break;
			case 'F':	/* format */
				n = sscanf(opt->arg, "%d", &(Ctrl->F.format));
				if (n == 1)
					Ctrl->F.active = true;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: \n");
					n_errors++;
				}
				break;
			case 'I':	/* -I<inputfile> */
				Ctrl->I.active = true;
				if (!gmt_access (GMT, opt->arg, R_OK)) {	/* Got a file */
					Ctrl->I.file = strdup (opt->arg);
					n_files = 1;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -I: Requires a valid file\n");
					n_errors++;
				}
				break;
			case 'L':	/* -L<lonflip> */
				n = sscanf(opt->arg, "%d", &(Ctrl->L.lonflip));
				if (n == 1)
					Ctrl->L.active = true;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -L option: \n");
					n_errors++;
				}
				break;
			case 'M':	/* -M fake option */
				Ctrl->M.file = strdup(opt->arg);
				break;
			case 'N':	/* -N<index_file> */
				Ctrl->N.active = true;
				Ctrl->N.file = strdup(opt->arg);
				break;
			case 'S':	/* -S<speed> */
				n = sscanf(opt->arg, "%lf", &(Ctrl->S.speed));
				if (n == 1)
					Ctrl->S.active = true;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -S option: \n");
					n_errors++;
				}
				break;
			case 'T':	/* -T<timegap> */
				n = sscanf(opt->arg, "%lf", &(Ctrl->T.timegap));
				if (n == 1)
					Ctrl->T.active = true;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: \n");
					n_errors++;
				}
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->C.active) Ctrl->A.active = false;		/* To play safe */

	if (!GMT->common.R.active[RSET]) {	/* When no Region specified, set a fake one here */
		gmt_parse_common_options (GMT, "R", 'R', "0/1/0/1");
		GMT->common.R.active[RSET] = true;
		Ctrl->Rfake.active = true;		/* This one will be used later to set the true -R from data, ... or die */
	}

	if (!GMT->common.J.active) {	/* When no projection specified, use fake linear projection */
		gmt_parse_common_options (GMT, "J", 'J', "X20c");
		GMT->common.J.active = true;
	}

	n_errors += gmt_M_check_condition(GMT, n_files != 1, "Syntax error: Must specify one input file(s)\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->I.active && !Ctrl->I.file,
	                                  "Syntax error -I option: Must specify input file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_mbgetdata(void *V_API, int mode, void *args) {

	uint64_t  dim[4];
	int    k, k_last_ping, n_alloc = 200;
	double NaN;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	struct MBGETDATA_CTRL *Ctrl = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASET *D2 = NULL;

	/* MBIO status variables */
	bool   done = false, read_data = false, found;
	int    status = MB_SUCCESS;
	int    verbose = 0, n_files = 0;
	int    error = MB_ERROR_NO_ERROR;
	char  *message = NULL;

	char   file[MB_PATH_MAXLINE] = {""}, dfile[MB_PATH_MAXLINE] = {""}, esffile[MB_PATH_MAXLINE] = {""};
	int    format, pings, n_pings, n_beams, n_beams_max = 0, col;
	bool   file_in_bounds = false;
	int   *index;
	struct mb_info_struct mb_info;
	struct mb_esf_struct esf;
	gmt_M_make_dnan(NaN);

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

#if GMT_MAJOR_VERSION >= 6
	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
#else
	GMT = gmt_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
#endif
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);

	Ctrl = (struct MBGETDATA_CTRL *) New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the mbgetdata main code ----------------------------*/

	memset(&esf, 0, sizeof(struct mb_esf_struct));

	pings = Ctrl->p.pings;		/* If pings were set by user, prefer it */

	/* set verbosity */
	verbose = GMT->common.V.active;
				
	/* get format if required */
	if (Ctrl->F.format == 0)
		mb_get_format(verbose, Ctrl->I.file, NULL, &Ctrl->F.format, &error);

	format = Ctrl->F.format;

	/* determine whether to read one file or a list of files */
	if (format < 0)
		Ctrl->read_datalist = MB_YES;

	/* If -R<region> was not passed in */
	if (Ctrl->Rfake.active) {		/* True when no -R provided */
		if (format < 0)
			status = mb_get_info_datalist(verbose, Ctrl->I.file, &format, &mb_info, Ctrl->L.lonflip, &error);
		else
			status = mb_get_info(verbose, Ctrl->I.file, &mb_info, Ctrl->L.lonflip, &error);
		if (status) {
			GMT->common.R.wesn[0] = mb_info.lon_min;	GMT->common.R.wesn[1] = mb_info.lon_max;
			GMT->common.R.wesn[2] = mb_info.lat_min;	GMT->common.R.wesn[3] = mb_info.lat_max;
		}
		else {
			int	btime_i[7], etime_i[7];
			int	pings_get = 1;
			double	speedmin, timegap;
			GMT_Report (API, GMT_MSG_NORMAL, "ERROR: no -R<region> provided and no .inf files to get it from.\n"
			                                 "You must run first 'mbinfo -O' on your datalist file.\n");
			/* get current default values */
			mb_defaults(verbose,&format,&pings_get,&Ctrl->L.lonflip,Ctrl->bounds, btime_i,etime_i,&speedmin,&timegap);
			//Return(EXIT_FAILURE);
		}
	}

	/* set bounds for data reading larger than map borders */
	Ctrl->bounds[0] = GMT->common.R.wesn[0] - 0.15*(GMT->common.R.wesn[1] - GMT->common.R.wesn[0]);
	Ctrl->bounds[1] = GMT->common.R.wesn[1] + 0.15*(GMT->common.R.wesn[1] - GMT->common.R.wesn[0]);
	Ctrl->bounds[2] = GMT->common.R.wesn[2] - 0.15*(GMT->common.R.wesn[3] - GMT->common.R.wesn[2]);
	Ctrl->bounds[3] = GMT->common.R.wesn[3] + 0.15*(GMT->common.R.wesn[3] - GMT->common.R.wesn[2]);

	/* set lonflip if needed */
	if (!Ctrl->L.active) {
		/*
		if (Ctrl->bounds[0] < -180.0)
			Ctrl->L.lonflip = -1;
		else if (Ctrl->bounds[1] > 180.0)
			Ctrl->L.lonflip = 1;
		else if (Ctrl->L.lonflip == -1 && Ctrl->bounds[1] > 0.0)
			Ctrl->L.lonflip = 0;
		else if (Ctrl->L.lonflip == 1 && Ctrl->bounds[0] < 0.0)
			Ctrl->L.lonflip = 0;
		*/
	}

	/* open file list */
	if (Ctrl->read_datalist == MB_YES) {
		if ((status = mb_datalist_open(verbose, &Ctrl->datalist, Ctrl->I.file, MB_DATALIST_LOOK_UNSET, &error)) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			GMT_Report(API, GMT_MSG_NORMAL,"\nUnable to open data list file: %s\n", Ctrl->I.file);
			GMT_Report(API, GMT_MSG_NORMAL,"\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
			Return(error);
		}
		if ((status = mb_datalist_read(verbose, Ctrl->datalist, file, dfile, &format, &Ctrl->file_weight, &error)) == MB_SUCCESS)
			read_data = true;
		else
			read_data = false;
	}
	else {
		strcpy(file, Ctrl->I.file);
		read_data = true;
	}

	index = gmt_M_memory (GMT, NULL, 1024, int);	/* 1024 is already an unthinkable number of files */

	/* loop over files in file list */
	n_pings = 0;
	while (read_data) {
		/* check for mbinfo file - get file bounds if possible */
		status = mb_check_info(verbose, file, Ctrl->L.lonflip, Ctrl->bounds, &file_in_bounds, &error);
		if (status == MB_FAILURE) {
			file_in_bounds = MB_YES;
			error = MB_ERROR_NO_ERROR;
		}

		/* read if data may be in bounds */
		if (file_in_bounds == MB_YES) {
			if (!Ctrl->C.active)		/* check for "fast bathymetry" or "fbt" file */
				mb_get_fbt(verbose, file, &format, &error);

			if ((status = mb_read_init(verbose, file, format, pings, Ctrl->L.lonflip, Ctrl->bounds, Ctrl->b.time_i,
									   Ctrl->e.time_i, Ctrl->S.speed, Ctrl->T.timegap, &Ctrl->mbio_ptr, &Ctrl->btime_d,
									   &Ctrl->etime_d, &Ctrl->beams_bath_max, &Ctrl->beams_amp_max, &Ctrl->pixels_ss_max,
									   &error)) != MB_SUCCESS) {
				mb_error(verbose,error,&message);
				GMT_Report(API, GMT_MSG_NORMAL,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
				GMT_Report(API, GMT_MSG_NORMAL,"\nMultibeam File <%s> not initialized for reading\n",file);
				GMT_Report(API, GMT_MSG_NORMAL,"\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
				Return(error);
			}
#if 0
			if (Ctrl->C.active && Ctrl->C.type == 0)	/* SideScan */
				n_beams_max = Ctrl->pixels_ss_max;
			else
				n_beams_max = Ctrl->beams_bath_max;
#endif

			/* allocate memory for data arrays */
			if (error == MB_ERROR_NO_ERROR)
				mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), &(Ctrl->data.beamflag), &error);
			if (error == MB_ERROR_NO_ERROR)
				mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), &(Ctrl->data.bath), &error);
			if (error == MB_ERROR_NO_ERROR)
				mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), &(Ctrl->data.amp), &error);
			if (error == MB_ERROR_NO_ERROR)
				mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), &(Ctrl->data.bathlon), &error);
			if (error == MB_ERROR_NO_ERROR)
				mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), &(Ctrl->data.bathlat), &error);
			if (error == MB_ERROR_NO_ERROR)
				mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), &(Ctrl->data.ss), &error);
			if (error == MB_ERROR_NO_ERROR)
				mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), &(Ctrl->data.sslon), &error);
			if (error == MB_ERROR_NO_ERROR)
				mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), &(Ctrl->data.sslat), &error);
			//if (error == MB_ERROR_NO_ERROR && Ctrl->A.active)
				//mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), &(Ctrl->data.time_d), &error);

			/* if error initializing memory then quit */
			if (error != MB_ERROR_NO_ERROR) {
				mb_error(verbose,error,&message);
				GMT_Report(API, GMT_MSG_NORMAL,"\nMBIO Error allocating data arrays:\n%s\n",message);
				GMT_Report(API, GMT_MSG_NORMAL,"Program <%s> Terminated\n", THIS_MODULE_NAME);
				Return(error);
			}

			/* print message */
			if (verbose) GMT_Report(API, GMT_MSG_NORMAL,"processing data in %s...\n",file);

			if (Ctrl->A.active)
				status = mb_esf_load(verbose, THIS_MODULE_NAME, file, MB_YES, MB_NO, esffile, &esf, &error);

			/* loop over reading */
			k_last_ping = 0;
			done = false;
			while (!done) {
				mb_read(verbose,Ctrl->mbio_ptr,&(Ctrl->data.kind), &(Ctrl->data.pings),Ctrl->data.time_i,
				        &(Ctrl->data.time_d), &(Ctrl->data.navlon),&(Ctrl->data.navlat), &(Ctrl->data.speed),
				        &(Ctrl->data.heading), &(Ctrl->data.distance),&(Ctrl->data.altitude),
				        &(Ctrl->data.sonardepth), &(Ctrl->data.beams_bath), &(Ctrl->data.beams_amp),
				        &(Ctrl->data.pixels_ss), Ctrl->data.beamflag,Ctrl->data.bath,Ctrl->data.amp,
				        Ctrl->data.bathlon,Ctrl->data.bathlat, Ctrl->data.ss,Ctrl->data.sslon,Ctrl->data.sslat,
				        Ctrl->data.comment,&error);

				if (error == MB_ERROR_COMMENT) continue;

				/* update bookkeeping */
				if (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP) {		/* ignore time gaps */
					error = MB_ERROR_NO_ERROR;
					if (n_beams_max == 0) {
						if (Ctrl->C.active) {
							if (Ctrl->C.type == 0)
								n_beams = Ctrl->data.pixels_ss;
							else
								n_beams = Ctrl->data.beams_amp;
							if (n_beams == 0) continue;
						}
						else
							n_beams = Ctrl->data.beams_bath;
					}

					n_beams_max = MAX(n_beams_max, n_beams);
					if (n_pings == 0) {
						dim[0] = 1;		dim[1] = 3;		dim[2] = n_alloc;		dim[3] = n_beams_max;
						if ((D = GMT_Create_Data(API, GMT_IS_DATASET, GMT_IS_POINT, GMT_CONTAINER_AND_DATA,
						                         dim, NULL, NULL, 0, 0, NULL)) == NULL) {
							GMT_Report(API, GMT_MSG_NORMAL, "Could not create Matrix structure\n");
							return EXIT_FAILURE;
						}
					}
					if (n_pings >= n_alloc) {
						n_alloc = (int)(1.7 * n_alloc);
						GMT_Alloc_Segment(API, GMT_NO_STRINGS, (uint64_t)n_alloc, (uint64_t)n_beams_max, NULL, D->table[0]->segment[0]);
						GMT_Alloc_Segment(API, GMT_NO_STRINGS, (uint64_t)n_alloc, (uint64_t)n_beams_max, NULL, D->table[0]->segment[1]);
						GMT_Alloc_Segment(API, GMT_NO_STRINGS, (uint64_t)n_alloc, (uint64_t)n_beams_max, NULL, D->table[0]->segment[2]);
					}
					if (!Ctrl->C.active) {			/* The bathymetry case */
						for (col = 0; col < n_beams; col++) {
							D->table[0]->segment[0]->data[col][n_pings] = Ctrl->data.bathlon[col];
							D->table[0]->segment[1]->data[col][n_pings] = Ctrl->data.bathlat[col];
							D->table[0]->segment[2]->data[col][n_pings] = -Ctrl->data.bath[col];
						}
						for (col = n_beams; col < n_beams_max; col++) {		/* NaNify the remaining beams of this ping */
							D->table[0]->segment[0]->data[col][n_pings] = NaN;
							D->table[0]->segment[1]->data[col][n_pings] = NaN;
							D->table[0]->segment[2]->data[col][n_pings] = NaN;
						}
						if (Ctrl->A.active) {		/* Convert all flagged beams to NaN or add a cte value */
							if (Ctrl->A.no_flags && esf.nedit > 0) {
								found = false;
								while (k_last_ping <= esf.nedit && esf.edit[k_last_ping].time_d < Ctrl->data.time_d) {
									found = true;
									k_last_ping++;
								}
								if (found) {
									k = k_last_ping;
									while (esf.edit[k].time_d == esf.edit[k+1].time_d && k < esf.nedit) {
										if (esf.edit[k].action != MBP_EDIT_UNFLAG)
											Ctrl->data.beamflag[esf.edit[k].beam] = 1;
										k++;
									}
								}
							}
							if (Ctrl->A.value) {
								for (col = 0; col < n_beams; col++)
									if (Ctrl->data.beamflag[col]) D->table[0]->segment[2]->data[col][n_pings] += Ctrl->A.value;
							}
							else {
								for (col = 0; col < n_beams; col++) {
									if (Ctrl->data.beamflag[col]) D->table[0]->segment[2]->data[col][n_pings] = NaN;
								}
							}
						}
					}
					else if (Ctrl->C.type == 0) {	/* The SideScan case */
						for (col = 0; col < n_beams_max; col++)
							if (Ctrl->data.ss[col] == -1000000000) Ctrl->data.ss[col] = NaN;

						for (col = 0; col < n_beams_max; col++) {
							D->table[0]->segment[0]->data[col][n_pings] = Ctrl->data.sslon[col];
							D->table[0]->segment[1]->data[col][n_pings] = Ctrl->data.sslat[col];
							D->table[0]->segment[2]->data[col][n_pings] = Ctrl->data.ss[col];
						}
					}
					else {							/* The Amplitude case */
						for (col = 0; col < n_beams_max; col++) {
							D->table[0]->segment[0]->data[col][n_pings] = Ctrl->data.bathlon[col];
							D->table[0]->segment[1]->data[col][n_pings] = Ctrl->data.bathlat[col];
							D->table[0]->segment[2]->data[col][n_pings] = Ctrl->data.amp[col];
						}
					}
					n_pings++;
				}
				else if (error > MB_ERROR_NO_ERROR)
					done = true;
			}
			if (n_beams_max) {
				GMT_Alloc_Segment(API, GMT_NO_STRINGS, (uint64_t)n_pings, (uint64_t)n_beams_max, NULL, D->table[0]->segment[0]);
				GMT_Alloc_Segment(API, GMT_NO_STRINGS, (uint64_t)n_pings, (uint64_t)n_beams_max, NULL, D->table[0]->segment[1]);
				GMT_Alloc_Segment(API, GMT_NO_STRINGS, (uint64_t)n_pings, (uint64_t)n_beams_max, NULL, D->table[0]->segment[2]);
			}
			mb_close(verbose,&Ctrl->mbio_ptr,&error);
			index[n_files] = n_pings;		/* Store the index at which a new file comes in contributing to out data */
			if (Ctrl->A.active && esf.nedit > 0 || esf.esffp != NULL) {
				status = mb_esf_close(verbose, &esf, &error);
			}
			n_files++;
		} /* end if file in bounds */

		/* figure out whether and what to read next */
		if (Ctrl->read_datalist == MB_YES) {
			if ((status = mb_datalist_read(verbose, Ctrl->datalist, file, dfile, &format,
			                               &Ctrl->file_weight, &error)) == MB_SUCCESS)
				read_data = true;
			else
				read_data = false;
		}
		else
			read_data = false;

		n_alloc = n_pings;		/* Prepare for an eventual next file if we are reading from a datalist */
	} /* end loop over files in list */

	if (Ctrl->read_datalist == MB_YES)
		mb_datalist_close(verbose, &Ctrl->datalist, &error);

	if (n_beams_max && (GMT_Write_Data(API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_WRITE_SET, NULL, Ctrl->M.file, D) != GMT_OK))
		return EXIT_FAILURE;

	if (Ctrl->N.active) {			/* Have no idea anymore what this option is for */
		int n;
		dim[0] = 1;		dim[1] = 1;		dim[2] = n_files;	dim[3] = 1;
		if ((D2 = GMT_Create_Data(API, GMT_IS_DATASET, GMT_IS_POINT, GMT_CONTAINER_AND_DATA,
		                          dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report(API, GMT_MSG_NORMAL, "Could not create Matrix structure\n");
			gmt_M_free(GMT, index);
			return EXIT_FAILURE;
		}
		//GMT_Put_Matrix(API, M, GMT_INT, index);
		for (n = 0; n < n_files; n++) {
			D2->table[0]->segment[0]->data[0][n] = index[n];
		}
		if (GMT_Write_Data(API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_WRITE_SET, NULL, Ctrl->N.file, D2) != GMT_OK) {
			gmt_M_free(GMT, index);
			return EXIT_FAILURE;
		}
	}
	gmt_M_free(GMT, index);

	Return (EXIT_SUCCESS);
}
