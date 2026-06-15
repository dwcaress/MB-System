/*--------------------------------------------------------------------
 *    The MB-system:	mbimport.c
 *    $Id: $
 *
 *    Licenced under the GPL like the rest of the MB-system package.
 *--------------------------------------------------------------------*/
/*
 * MBIMPORT is a GMT supplement module utility which creates an image of swath bathymetry or
 * backscatter data and returns it as an image. Currently this works only in MEX or Julia mode.
 * The image may be shaded relief as well. The modes of operation are:
 *   Mode 1:  Bathymetry
 *   Mode 2:  Bathymetry shaded by illumination
 *   Mode 3:  Bathymetry shaded by amplitude
 *   Mode 4:  amplitude
 *   Mode 5:  sidescan
 *   Mode 6:  Bathymetry shaded by amplitude using cpt gray data
 *
 * Author:	J. Luis but strongly based on MB's mbswath.c program
 * Date:	27 April, 2016
 *
 */

#define THIS_MODULE_NAME	"mbimport"
#define THIS_MODULE_LIB		"mbsystem"
#define THIS_MODULE_PURPOSE	"Plot swath bathymetry, amplitude, or backscatter"
#define THIS_MODULE_KEYS	"<D{,CC(,NC(,MI}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->JRUVXYnt" GMT_OPT("S")

/* GMT5 header file */
#include "gmt_dev.h"

/* MBIO include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_info.h"
#include "mb_io.h"

/* MBSWATH MODE DEFINES */
#define	MBSWATH_BATH		1
#define	MBSWATH_BATH_RELIEF	2
#define	MBSWATH_BATH_AMP	3
#define	MBSWATH_AMP		4
#define	MBSWATH_SS		5
#define	MBSWATH_BATH_AMP_FILTER	6
#define	MBSWATH_AMP_FILTER	7
#define	MBSWATH_SS_FILTER	8
#define MBSWATH_FOOTPRINT_REAL	1
#define MBSWATH_FOOTPRINT_FAKE	2
#define MBSWATH_FOOTPRINT_POINT	3
#define MBSWATH_FILTER_NONE	0
#define MBSWATH_FILTER_AMP	1
#define MBSWATH_FILTER_SIDESCAN	2

/* image type defines */
#define	MBSWATH_IMAGE_8		2
#define	MBSWATH_IMAGE_24	3

/* How B/W TV's convert RGB to Gray */
#define YIQ(rgb) (0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2])

/* global structure definitions */
#define MAXPINGS 50
static struct footprint {
	double	x[4], y[4];
};
static struct ping_local {
	int	pings;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sonardepth;
	int     beams_bath;
	int     beams_amp;
	int     pixels_ss;
	char   *beamflag;
	double *bath;
	double *bathlon;
	double *bathlat;
	double *amp;
	double *ss;
	double *sslon;
	double *sslat;
	char	comment[256];
	double	lonaft;
	double	lataft;
	double	lonfor;
	double	latfor;
	int    *bathflag;
	struct  footprint *bathfoot;
	int    *ssflag;
	struct  footprint *ssfoot;
	double *bathshade;
};
static struct swath_local {
	int	npings;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	struct ping_local data[MAXPINGS];
};

/* Control structure for mbswath */
static struct CTRL {
		/* mbswath variables */
	double  bounds[4];
	int     image_type;
	double  mtodeglon;
	double  mtodeglat;
	double  clipx[4];
	double  clipy[4];
	double  x_inc;
	double  y_inc;
	double  x_side;
	double  y_side;
	double  x0;
	double  y0;
	double  x_inch;
	double  y_inch;
	int     nx;
	int     ny;
	int     nm;
	int     nm2;
	unsigned char *bitimage;
	int     format;
	double	beamwidth_xtrack;
	double	beamwidth_ltrack;
	double  footprint_factor;
	double	btime_d;
	double	etime_d;
	int     read_datalist;
	void	*datalist;
	double	file_weight;
	mb_path file;
	int     filtermode;
	int     beams_bath_max;
	int     beams_amp_max;
	int     pixels_ss_max;
	void   *mbio_ptr;
	struct  swath_local *swath_plot;

	struct mbimport_A {	/* -A<factor>/<mode>/<depth> */
		bool active;
		double factor;
		int mode;
		double depth;
	} A;
	struct mbimport_b {	/* -b<year>/<month>/<day>/<hour>/<minute>/<second> */
		bool active;
		int time_i[7];
	} b;
	struct mbimport_C {	/* -C<cptfile> */
		bool active;
		char *cptfile;
	} C;
	struct mbimport_D {	/* -D<mode>/<ampscale>/<ampmin>/<ampmax> */
		bool active;
		unsigned int mode;
		double ampscale;
		double ampmin;
		double ampmax;
	} D;
	struct mbimport_E {	/* -Ei|<dpi> */
		bool active;
		bool device_dpi;
		unsigned int dpi;
	} E;
	struct mbimport_e {	/* -e<year>/<month>/<day>/<hour>/<minute>/<second> */
		bool active;
		int time_i[7];
	} e;
	struct mbimport_F {	/* -F<format> */
		bool active;
		int format;
	} F;
	struct mbimport_M {	/* -M<out_fname> */
		char *file;
	} M;
	struct mbimport_G {	/* -G<magnitude>/<azimuth | median> */
		bool active;
		double magnitude;
		double azimuth;
	} G;
	struct mbimport_I {	/* -I<inputfile> */
		bool active;
		char *inputfile;
	} I;
	struct mbimport_L {	/* -L<lonflip> */
		bool active;
		int lonflip;
	} L;
	struct mbimport_N {	/* -N<cptfile> */
		bool active;
		char *cptfile;
	} N;
	struct mbimport_p {	/* -p<pings> */
		bool active;
		int pings;
	} p;
	struct mbimport_R {	/* fake -R */
		bool active;
	} Rfake;
	struct mbimport_S {	/* -S<speed> */
		bool active;
		double speed;
	} S;
	struct mbimport_T {	/* -T<timegap> */
		bool active;
		double timegap;
	} T;
	struct mbimport_Z {	/* -Z<mode> */
		bool active;
		int mode;
		int filter;
		int usefiltered;
	} Z;
};

GMT_LOCAL int mbimport_get_footprints(struct CTRL *Ctrl, int *error);
GMT_LOCAL int mbimport_get_shading(struct CTRL *Ctrl, struct GMT_CTRL *GMT, struct GMT_PALETTE *CPT, int *error);
GMT_LOCAL int mbimport_plot_data_footprint(struct CTRL *Ctrl, struct GMT_CTRL *GMT, struct GMT_PALETTE *CPT,
                                           int first, int nplot, int *error);
GMT_LOCAL int mbimport_plot_data_point(struct CTRL *Ctrl, struct GMT_CTRL *GMT, struct GMT_PALETTE *CPT, int first, int nplot, int *error);
GMT_LOCAL int mbimport_plot_box(struct CTRL *Ctrl, double *x, double *y, double *rgb, int *error);
GMT_LOCAL int mbimport_plot_point(struct CTRL *Ctrl, double x, double y, double *rgb, int *error);
GMT_LOCAL int mbimport_ping_copy(int one, int two, struct swath_local *swath, int *error);

static void *New_Ctrl(struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct  CTRL *Ctrl;
	int     status;
	int     verbose = 0;
	double  dummybounds[4];
	int     dummyformat, dummypings;

	Ctrl = gmt_M_memory (GMT, NULL, 1, struct CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	/* get current mb default values */
	status = mb_defaults(verbose, &dummyformat, &dummypings, &Ctrl->L.lonflip, dummybounds,
						 Ctrl->b.time_i, Ctrl->e.time_i, &Ctrl->S.speed, &Ctrl->T.timegap);

	if (status == MB_FAILURE)
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Probable fatal error: failed to get current MB defaults.\n");

	Ctrl->A.factor = 1.0;
	Ctrl->A.mode = MBSWATH_FOOTPRINT_REAL;
	Ctrl->C.cptfile = NULL;
	Ctrl->D.mode = 1;
	Ctrl->D.ampscale = 1.0;
	Ctrl->D.ampmax = 1.0;
	Ctrl->E.dpi = 100;
	Ctrl->G.magnitude = 1.0;
	Ctrl->G.azimuth = 270.0;
	Ctrl->I.inputfile = NULL;
	Ctrl->N.cptfile = NULL;
	Ctrl->p.pings = 1;
	Ctrl->T.timegap = 1.0;
	Ctrl->Z.mode = MBSWATH_BATH_RELIEF;
	Ctrl->Z.usefiltered = MB_NO;
		
	/* mbswath variables */
	Ctrl->image_type = MBSWATH_IMAGE_24;
	Ctrl->bitimage = NULL;
	Ctrl->read_datalist = MB_NO;
	Ctrl->datalist = NULL;
	Ctrl->file[0] = '\0';
	Ctrl->filtermode = MBSWATH_FILTER_NONE;
	Ctrl->mbio_ptr = NULL;
	Ctrl->swath_plot = NULL;

	return (Ctrl);
}

static void Free_Ctrl(struct GMT_CTRL *GMT, struct CTRL *Ctrl) {	/* Deallocate control structure */
	if (!Ctrl) return;
	if (Ctrl->C.cptfile) free (Ctrl->C.cptfile);
	if (Ctrl->I.inputfile) free (Ctrl->I.inputfile);
	if (Ctrl->N.cptfile) free (Ctrl->N.cptfile);
	gmt_M_free (GMT, Ctrl);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: mbimport -I<inputfile> %s\n", GMT_J_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-A<factor>/<mode>/<depth>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-b<year>/<month>/<day>/<hour>/<minute>/<second>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-C<cptfile>] [-D<mode>/<ampscale>/<ampmin>/<ampmax>] [-Ei|<dpi>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-e<year>/<month>/<day>/<hour>/<minute>/<second>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-F<format>] [-G<magnitude>/<azimuth | median>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-I<inputfile>] [-L<lonflip>] [-N<cptfile>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-S<speed>] [-T<timegap>] [-W] [-Z<mode>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-T] [%s] [%s]\n", GMT_Rgeo_OPT, GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s]\n [%s]\n\n", 
									 GMT_X_OPT, GMT_Y_OPT, GMT_n_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<inputfile> is an MB-System datalist referencing the swath data to be plotted.\n");
	GMT_Option (API, "J-");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Color palette file to convert z to rgb.  Optionally, instead give name of a master cpt\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to automatically assign 16 continuous colors over the data range [turbo].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set dpi for the projected output Postscript image\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   if -Jx or -Jm is not selected.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give i to do the interpolation in PostScript at device resolution.\n");
	gmt_rgb_syntax (API->GMT, 'G', "Set transparency color for images that otherwise would result in 1-bit images.\n\t  ");
	GMT_Message (API, GMT_TIME_NONE, "\t-p<pings> Sets the ping averaging of the input data [Default = 1, i.e. no ping average].\n");
	GMT_Option (API, "R");
	GMT_Option (API, "U,V,X,c,.");

	return (EXIT_FAILURE);
}

static int parse(struct GMT_CTRL *GMT, struct CTRL *Ctrl, struct GMT_OPTION *options) {
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
					Ctrl->I.inputfile = strdup (opt->arg);
					n_files = 1;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: only one input file is allowed.\n");
					n_errors++;
				}
				break;

			/* Processes program-specific parameters */

			case 'A':	/* footprint controls */
				n = sscanf(opt->arg, "%lf/%d/%lf", &(Ctrl->A.factor), &(Ctrl->A.mode), &(Ctrl->A.depth));
				if (n > 0)
					Ctrl->A.active = true;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -A option: \n");
					n_errors++;
				}
				break;
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
			case 'C':	/* CPT file */
				Ctrl->C.active = true;
				if (Ctrl->C.cptfile) free (Ctrl->C.cptfile);
				Ctrl->C.cptfile = strdup (opt->arg);
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
			case 'E':	/* dpi */
				if (strcmp(opt->arg, "i") == 0) {
					Ctrl->E.device_dpi = true;
					Ctrl->E.active = true;
				}
				else {
					n = sscanf(opt->arg, "%d", &(Ctrl->E.dpi));
					if (n == 1)
						Ctrl->E.active = true;
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -E option: \n");
						n_errors++;
					}
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
			case 'F':	/* format */
				n = sscanf(opt->arg, "%d", &(Ctrl->F.format));
				if (n == 1)
					Ctrl->F.active = true;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: \n");
					n_errors++;
				}
				break;
			case 'G':	/* -G<magnitude>/<azimuth | median> */
				n = sscanf(opt->arg, "%lf/%lf", &(Ctrl->G.magnitude), &(Ctrl->G.azimuth));
				if (n >= 1)
					Ctrl->G.active = true;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -G option: \n");
					n_errors++;
				}
				break;
			case 'I':	/* -I<inputfile> */
				Ctrl->I.active = true;
				if (!gmt_access (GMT, opt->arg, R_OK)) {	/* Got a file */
					Ctrl->I.inputfile = strdup (opt->arg);
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
			case 'N':	/* -N<cptfile> */
				Ctrl->N.active = true;
				if (Ctrl->N.cptfile) free (Ctrl->N.cptfile);
				Ctrl->N.cptfile = strdup (opt->arg);
				break;
			case 'p':	/* Sets the ping averaging */
				Ctrl->p.active = true;
				Ctrl->p.pings = atoi(opt->arg);
				if (Ctrl->p.pings < 0) {
					GMT_Report (API, GMT_MSG_NORMAL, "Error -p option: Don't invent, number of pings must be >= 0\n");
					Ctrl->p.pings = 1;
				}
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
			case 'Z':	/* -Z<mode> */
				Ctrl->Z.active = true;
				if ((n = sscanf(opt->arg, "%d", &(Ctrl->Z.mode))) == 1) {
					if (opt->arg[1] == 'f' || opt->arg[1] == 'F')
						Ctrl->Z.usefiltered = MB_YES;
					else
						Ctrl->Z.usefiltered = MB_NO;
				}
				else {
					GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -Z option: \n");
					n_errors++;
				}
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error(GMT, opt->option);
				break;
		}
	}

	if (!GMT->common.R.active[RSET]) {	/* When no Region specified, set a fake one here */
		gmt_parse_common_options (GMT, "R", 'R', "0/1/0/1");
		GMT->common.R.active[RSET] = true;
		Ctrl->Rfake.active = true;		/* This one will be used later to set the true -R from data, ... or die */
	}

	if (!GMT->common.J.active) {	/* When no projection specified, use fake linear projection */
		gmt_parse_common_options (GMT, "J", 'J', "X20c");
		GMT->common.J.active = true;
	}

	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Syntax error: Must specify one input file(s)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && !Ctrl->I.inputfile,
	                                   "Syntax error -I option: Must specify input file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && !Ctrl->E.device_dpi && Ctrl->E.dpi <= 0,
									   "Syntax error -E option: dpi must be positive\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_mbimport (void *V_API, int mode, void *args) {

	unsigned int nopad[4] = {0, 0, 0, 0};
	uint64_t  dim[3];
	char   Title[GMT_LEN32] = {""};
	double xy_inc[2];
	struct GMT_PALETTE *CPTcolor = NULL;
	struct GMT_PALETTE *CPTshade = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	struct CTRL *Ctrl = NULL;
	struct GMT_IMAGE *I = NULL;

	/* MBIO status variables */
	bool    plot = false, done = false, flush = false, read_data = false;
	int     status = MB_SUCCESS;
	int     verbose = 0;
	int     error = MB_ERROR_NO_ERROR;
	char   *message = NULL;

	bool    file_in_bounds = false;
	char    file[MB_PATH_MAXLINE] = {""}, dfile[MB_PATH_MAXLINE] = {""};
	int     format, nping_read = 0;
	int     start, first, nplot, save_new, i, pings;
	int    *npings = NULL;
	double  amplog;
	struct ping_local *pingcur = NULL;
	struct mb_info_struct mb_info;


	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

#if GMT_MAJOR_VERSION >= 6
	if ((GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
#else
	GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
#endif
	if (GMT_Parse_Common(API, THIS_MODULE_OPTIONS, options)) Return (API->error);

	Ctrl = (struct CTRL *) New_Ctrl(GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse(GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the mbswath main code ----------------------------*/

	pings = Ctrl->p.pings;		/* If pings were set by user, prefer it */

	/* set verbosity */
	verbose = GMT->common.V.active;
				
	/* get format if required */
	if (Ctrl->F.format == 0)
		mb_get_format(verbose, Ctrl->I.inputfile, NULL, &Ctrl->F.format, &error);

	format = Ctrl->F.format;

	/* determine whether to read one file or a list of files */
	if (format < 0)
		Ctrl->read_datalist = MB_YES;

	/* If -R<region> was not passed in */
	if (Ctrl->Rfake.active) {		/* True when no -R provided */
		if (format < 0)
			status = mb_get_info_datalist(verbose, Ctrl->I.inputfile, &format, &mb_info, Ctrl->L.lonflip, &error);
		else
			status = mb_get_info(verbose, Ctrl->I.inputfile, &mb_info, Ctrl->L.lonflip, &error);
		if (status) {
			GMT->common.R.wesn[0] = mb_info.lon_min;	GMT->common.R.wesn[1] = mb_info.lon_max;
			GMT->common.R.wesn[2] = mb_info.lat_min;	GMT->common.R.wesn[3] = mb_info.lat_max;
		}
		else {
			GMT_Report (API, GMT_MSG_NORMAL, "ERROR: no -R<region> provided and no .inf files to get it from.\n"
			                                 "You must run first 'mbinfo -O' on your datalist file.\n");
			Return(EXIT_FAILURE);
		}
	}

	/* set bounds for data reading larger than map borders */
	Ctrl->bounds[0] = GMT->common.R.wesn[0] - 0.15*(GMT->common.R.wesn[1] - GMT->common.R.wesn[0]);
	Ctrl->bounds[1] = GMT->common.R.wesn[1] + 0.15*(GMT->common.R.wesn[1] - GMT->common.R.wesn[0]);
	Ctrl->bounds[2] = GMT->common.R.wesn[2] - 0.15*(GMT->common.R.wesn[3] - GMT->common.R.wesn[2]);
	Ctrl->bounds[3] = GMT->common.R.wesn[3] + 0.15*(GMT->common.R.wesn[3] - GMT->common.R.wesn[2]);

	/* get scaling from degrees to km */
	mb_coor_scale(verbose, 0.5*(Ctrl->bounds[2] + Ctrl->bounds[3]), &Ctrl->mtodeglon, &Ctrl->mtodeglat);

	/* set lonflip if needed */
	if (!Ctrl->L.active) {
		if (Ctrl->bounds[0] < -180.0)
			Ctrl->L.lonflip = -1;
		else if (Ctrl->bounds[1] > 180.0)
			Ctrl->L.lonflip = 1;
		else if (Ctrl->L.lonflip == -1 && Ctrl->bounds[1] > 0.0)
			Ctrl->L.lonflip = 0;
		else if (Ctrl->L.lonflip == 1 && Ctrl->bounds[0] < 0.0)
			Ctrl->L.lonflip = 0;
	}

	/* Read the color palette file */
	if (Ctrl->C.active) {
		if ((CPTcolor = gmt_get_palette(GMT, Ctrl->C.cptfile, GMT_CPT_REQUIRED, 0.0, 0.0, 0.0)) == NULL)
			Return (API->error);
		if (CPTcolor->is_gray && Ctrl->image_type == MBSWATH_IMAGE_24)
			Ctrl->image_type = MBSWATH_IMAGE_8;
	}
	else if (Ctrl->Rfake.active) {
		char   file[8] = {""};	/* If used as is, it will mean 'turbo' to gmt_get_cpt() -- NOT USED!!!! -- */
		double zmin, zmax;
		if (Ctrl->Z.mode <= MBSWATH_BATH_AMP) {		/* Cases 1, 2 and 3. They plot the bathymetry. */
			zmin = mb_info.depth_min;	zmax = mb_info.depth_max;	strcat(file, "turbo");
		}
		else if (Ctrl->Z.mode == MBSWATH_AMP) { 	/* Case 4 */
			zmin = mb_info.amp_min;		zmax = mb_info.amp_max;		strcat(file, "gray");
			Ctrl->image_type = MBSWATH_IMAGE_8;
		}
		else {		/* Case 5 (MBSWATH_SS) */
			zmin = mb_info.ss_min;		zmax = mb_info.ss_max;		strcat(file, "gray");
			Ctrl->image_type = MBSWATH_IMAGE_8;
		}

		if ((CPTcolor = gmt_get_palette(GMT, file, GMT_CPT_OPTIONAL, zmin, zmax, 0.0)) == NULL)	/* Dedaults to turbo (others are harder) */
			Return (API->error);
		gmt_scale_cpt (GMT, CPTcolor, -1);		/* Flip the color scale because Z is pos down (Blheak) */
		CPTcolor->data->z_low = zmin;
		CPTcolor->data->z_high = zmax;

		if (CPTcolor->is_gray && Ctrl->image_type == MBSWATH_IMAGE_24)
			Ctrl->image_type = MBSWATH_IMAGE_8;
	}

	/* Read the color palette file for amplitude shading if requested */
	if (Ctrl->N.active) {
		if ((CPTshade = gmt_get_palette (GMT, Ctrl->N.cptfile, GMT_CPT_REQUIRED, 0.0, 0.0, 0.0)) == NULL)
			Return (API->error);
	}

	gmt_M_err_fail (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "");

	/* Set particulars of output image for the postscript plot */
	gmt_geo_to_xy(GMT, GMT->common.R.wesn[0], GMT->common.R.wesn[2], &Ctrl->clipx[0], &Ctrl->clipy[0]);
	gmt_geo_to_xy(GMT, GMT->common.R.wesn[1], GMT->common.R.wesn[2], &Ctrl->clipx[1], &Ctrl->clipy[1]);
	gmt_geo_to_xy(GMT, GMT->common.R.wesn[1], GMT->common.R.wesn[3], &Ctrl->clipx[2], &Ctrl->clipy[2]);
	gmt_geo_to_xy(GMT, GMT->common.R.wesn[0], GMT->common.R.wesn[3], &Ctrl->clipx[3], &Ctrl->clipy[3]);
	Ctrl->x_inch = Ctrl->clipx[1] - Ctrl->clipx[0];
	Ctrl->y_inch = Ctrl->clipy[2] - Ctrl->clipy[1];
	Ctrl->x0 = Ctrl->clipx[0];
	Ctrl->y0 = Ctrl->clipy[0];
	Ctrl->nx = (int)(Ctrl->x_inch * Ctrl->E.dpi);
	Ctrl->ny = (int)(Ctrl->y_inch * Ctrl->E.dpi);
	Ctrl->x_inc = (GMT->common.R.wesn[1] - GMT->common.R.wesn[0]) / (Ctrl->nx - 1);
	Ctrl->y_inc = (GMT->common.R.wesn[3] - GMT->common.R.wesn[2]) / (Ctrl->ny - 1);
	Ctrl->x_side = Ctrl->x_inc * Ctrl->nx;
	Ctrl->y_side = Ctrl->y_inc * Ctrl->ny;
	Ctrl->nm = Ctrl->nx * Ctrl->ny;
	Ctrl->nm2 = 2 * Ctrl->nm;

	/* allocate and initialize the output image */
	if (Ctrl->image_type == MBSWATH_IMAGE_8) {
		Ctrl->bitimage = gmt_M_memory (GMT, NULL, Ctrl->nm, unsigned char);
		memset(Ctrl->bitimage, 255, Ctrl->nm);
	}
	else if (Ctrl->image_type == MBSWATH_IMAGE_24) {
		Ctrl->bitimage = gmt_M_memory (GMT, NULL, 3 * Ctrl->nm, unsigned char);
		memset(Ctrl->bitimage, 255, 3 * Ctrl->nm);
	}

	/* turn on looking for filtered amp or sidescan if needed */
	if (Ctrl->Z.usefiltered == MB_YES) {
		if (Ctrl->Z.mode == MBSWATH_BATH_AMP)
			Ctrl->filtermode = MBSWATH_FILTER_AMP;
		else if (Ctrl->Z.mode == MBSWATH_AMP)
			Ctrl->filtermode = MBSWATH_FILTER_AMP;
		else if (Ctrl->Z.mode == MBSWATH_SS)
			Ctrl->filtermode = MBSWATH_FILTER_SIDESCAN;
	}

	/* open file list */
	if (Ctrl->read_datalist == MB_YES) {
		if ((status = mb_datalist_open(verbose, &Ctrl->datalist, Ctrl->I.inputfile, MB_DATALIST_LOOK_UNSET, &error)) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			GMT_Report(API, GMT_MSG_NORMAL,"\nUnable to open data list file: %s\n", Ctrl->I.inputfile);
			GMT_Report(API, GMT_MSG_NORMAL,"\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
			Return(error);
		}
		if ((status = mb_datalist_read(verbose, Ctrl->datalist, file, dfile, &format, &Ctrl->file_weight, &error)) == MB_SUCCESS)
			read_data = true;
		else
			read_data = false;
	}
	else {
		strcpy(file, Ctrl->I.inputfile);
		read_data = true;
	}

	/* loop over files in file list */
	while (read_data) {
		/* check for mbinfo file - get file bounds if possible */
		status = mb_check_info(verbose, file, Ctrl->L.lonflip, Ctrl->bounds, &file_in_bounds, &error);
		if (status == MB_FAILURE) {
			file_in_bounds = MB_YES;
			error = MB_ERROR_NO_ERROR;
		}

		/* read if data may be in bounds */
		if (file_in_bounds == MB_YES) {
			/* check for "fast bathymetry" or "fbt" file */
			if (Ctrl->Z.mode == MBSWATH_BATH || Ctrl->Z.mode == MBSWATH_BATH_RELIEF)
				mb_get_fbt(verbose, file, &format, &error);

			/* check for filtered amplitude or sidescan file */
			if (Ctrl->filtermode == MBSWATH_FILTER_AMP) {
				if ((status = mb_get_ffa(verbose, file, &format, &error)) != MB_SUCCESS) {
					mb_error(verbose,error,&message);
					GMT_Report(API, GMT_MSG_NORMAL,"\nMBIO Error returned from function <mb_get_ffa>:\n%s\n",message);
					GMT_Report(API, GMT_MSG_NORMAL,"Requested filtered amplitude file missing\n");
					GMT_Report(API, GMT_MSG_NORMAL,"\nMultibeam File <%s> not initialized for reading\n",file);
					GMT_Report(API, GMT_MSG_NORMAL,"\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
					Return(error);
				}
			}
			else if (Ctrl->filtermode == MBSWATH_FILTER_SIDESCAN) {
				if ((status = mb_get_ffs(verbose, file, &format, &error)) != MB_SUCCESS) {
					mb_error(verbose,error,&message);
					GMT_Report(API, GMT_MSG_NORMAL,"\nMBIO Error returned from function <mb_get_ffs>:\n%s\n",message);
					GMT_Report(API, GMT_MSG_NORMAL,"Requested filtered sidescan file missing\n");
					GMT_Report(API, GMT_MSG_NORMAL,"\nMultibeam File <%s> not initialized for reading\n",file);
					GMT_Report(API, GMT_MSG_NORMAL,"\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
					Return(error);
				}
			}

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

			/* get fore-aft beam_width */
			mb_format_beamwidth(verbose, &format, &Ctrl->beamwidth_xtrack, &Ctrl->beamwidth_ltrack, &error);
			if (Ctrl->beamwidth_ltrack <= 0.0)
				Ctrl->beamwidth_ltrack = 2.0;
			if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL)
				Ctrl->footprint_factor = Ctrl->A.factor * Ctrl->beamwidth_ltrack;
			else
				Ctrl->footprint_factor = Ctrl->A.factor;

			/* allocate memory for data arrays */
			mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct swath_local), (void **)&Ctrl->swath_plot, &error);
			npings = &Ctrl->swath_plot->npings;
			Ctrl->swath_plot->beams_bath = Ctrl->beams_bath_max;
			Ctrl->swath_plot->beams_amp = Ctrl->beams_amp_max;
			Ctrl->swath_plot->pixels_ss = Ctrl->pixels_ss_max;
			for (i = 0; i < MAXPINGS; i++) {
				pingcur = &(Ctrl->swath_plot->data[i]);
				pingcur->beamflag = NULL;
				pingcur->bath = NULL;
				pingcur->amp = NULL;
				pingcur->bathlon = NULL;
				pingcur->bathlat = NULL;
				pingcur->ss = NULL;
				pingcur->sslon = NULL;
				pingcur->sslat = NULL;
				pingcur->bathflag = NULL;
				pingcur->bathfoot = NULL;
				pingcur->ssflag = NULL;
				pingcur->ssfoot = NULL;
				pingcur->bathshade = NULL;
				if (error == MB_ERROR_NO_ERROR)
					mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&(pingcur->beamflag), &error);
				if (error == MB_ERROR_NO_ERROR)
					mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&(pingcur->bath), &error);
				if (error == MB_ERROR_NO_ERROR)
					mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&(pingcur->amp), &error);
				if (error == MB_ERROR_NO_ERROR)
					mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&(pingcur->bathlon), &error);
				if (error == MB_ERROR_NO_ERROR)
					mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&(pingcur->bathlat), &error);
				if (error == MB_ERROR_NO_ERROR)
					mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&(pingcur->ss), &error);
				if (error == MB_ERROR_NO_ERROR)
					mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&(pingcur->sslon), &error);
				if (error == MB_ERROR_NO_ERROR)
					mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&(pingcur->sslat), &error);
				if (error == MB_ERROR_NO_ERROR)
					mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&(pingcur->bathflag), &error);
				if (error == MB_ERROR_NO_ERROR)
					mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(struct footprint), (void **)&(pingcur->bathfoot), &error);
				if (error == MB_ERROR_NO_ERROR)
					mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(int), (void **)&(pingcur->ssflag), &error);
				if (error == MB_ERROR_NO_ERROR)
					mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(struct footprint), (void **)&(pingcur->ssfoot), &error);
				if (error == MB_ERROR_NO_ERROR)
					mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&(pingcur->bathshade), &error);
			}

			/* if error initializing memory then quit */
			if (error != MB_ERROR_NO_ERROR) {
				mb_error(verbose,error,&message);
				GMT_Report(API, GMT_MSG_NORMAL,"\nMBIO Error allocating data arrays:\n%s\n",message);
				GMT_Report(API, GMT_MSG_NORMAL,"\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
				Return(error);
			}

			/* print message */
			if (verbose) GMT_Report(API, GMT_MSG_NORMAL,"processing data in %s...\n",file);

			/* loop over reading */
			*npings = 0;
			start = MB_YES;
			done = false;
			while (!done) {
				pingcur = &Ctrl->swath_plot->data[*npings];
				mb_read(verbose,Ctrl->mbio_ptr,&(pingcur->kind), &(pingcur->pings),pingcur->time_i,
				        &(pingcur->time_d), &(pingcur->navlon),&(pingcur->navlat), &(pingcur->speed),
				        &(pingcur->heading), &(pingcur->distance),&(pingcur->altitude),
				        &(pingcur->sonardepth), &(pingcur->beams_bath), &(pingcur->beams_amp),
				        &(pingcur->pixels_ss), pingcur->beamflag,pingcur->bath,pingcur->amp,
				        pingcur->bathlon,pingcur->bathlat, pingcur->ss,pingcur->sslon,pingcur->sslat,
				        pingcur->comment,&error);

				/* ignore time gaps */
				if (error == MB_ERROR_TIME_GAP)
					error = MB_ERROR_NO_ERROR;

				/* update bookkeeping */
				if (error == MB_ERROR_NO_ERROR) {
					nping_read += pingcur->pings;
					(*npings)++;
				}

				/* scale amplitudes if necessary */
				if (error == MB_ERROR_NO_ERROR && (mode == MBSWATH_BATH_AMP || mode == MBSWATH_AMP) && Ctrl->D.mode > 0) {
					for (i=0;i<pingcur->beams_amp;i++) {
						if (mb_beam_ok(pingcur->beamflag[i]) && Ctrl->D.mode == 1) {
							pingcur->amp[i] = Ctrl->D.ampscale * (pingcur->amp[i] - Ctrl->D.ampmin) / (Ctrl->D.ampmax - Ctrl->D.ampmin);
						}
						else if (mb_beam_ok(pingcur->beamflag[i]) && Ctrl->D.mode == 2) {
							pingcur->amp[i] = MIN(pingcur->amp[i],Ctrl->D.ampmax);
							pingcur->amp[i] = MAX(pingcur->amp[i],Ctrl->D.ampmin);
							pingcur->amp[i] = Ctrl->D.ampscale * (pingcur->amp[i] - Ctrl->D.ampmin) / (Ctrl->D.ampmax - Ctrl->D.ampmin);
						}
						else if (mb_beam_ok(pingcur->beamflag[i]) && Ctrl->D.mode == 3) {
							amplog = 20.0 * log10(pingcur->amp[i]);
							pingcur->amp[i] = Ctrl->D.ampscale * (amplog - Ctrl->D.ampmin) / (Ctrl->D.ampmax - Ctrl->D.ampmin);
						}
						else if (mb_beam_ok(pingcur->beamflag[i]) && Ctrl->D.mode == 4) {
							amplog = 20.0 * log10(pingcur->amp[i]);
								amplog = MIN(amplog,Ctrl->D.ampmax);
							amplog = MAX(amplog,Ctrl->D.ampmin);
							pingcur->amp[i] = Ctrl->D.ampscale * (amplog - Ctrl->D.ampmin) / (Ctrl->D.ampmax - Ctrl->D.ampmin);
						}
					}
				}

				/* scale sidescan if necessary */
				if (error == MB_ERROR_NO_ERROR && mode == MBSWATH_SS && Ctrl->D.mode > 0) {
					for (i = 0; i < pingcur->pixels_ss; i++) {
						if (pingcur->ss[i] > MB_SIDESCAN_NULL && Ctrl->D.mode == 1) {
							pingcur->ss[i] = Ctrl->D.ampscale * (pingcur->ss[i] - Ctrl->D.ampmin) / (Ctrl->D.ampmax - Ctrl->D.ampmin);
						}
						else if (pingcur->ss[i] > MB_SIDESCAN_NULL && Ctrl->D.mode == 2) {
							pingcur->ss[i] = MIN(pingcur->ss[i],Ctrl->D.ampmax);
							pingcur->ss[i] = MAX(pingcur->ss[i],Ctrl->D.ampmin);
							pingcur->ss[i] = Ctrl->D.ampscale * (pingcur->ss[i] - Ctrl->D.ampmin) / (Ctrl->D.ampmax - Ctrl->D.ampmin);
						}
						else if (pingcur->ss[i] > MB_SIDESCAN_NULL && Ctrl->D.mode == 3) {
							amplog = 20.0 * log10(pingcur->ss[i]);
							pingcur->ss[i] = Ctrl->D.ampscale * (amplog - Ctrl->D.ampmin) / (Ctrl->D.ampmax - Ctrl->D.ampmin);
						}
						else if (pingcur->ss[i] > MB_SIDESCAN_NULL && Ctrl->D.mode == 4) {
							amplog = 20.0 * log10(pingcur->ss[i]);
							amplog = MIN(amplog,Ctrl->D.ampmax);
							amplog = MAX(amplog,Ctrl->D.ampmin);
							pingcur->ss[i] = Ctrl->D.ampscale * (amplog - Ctrl->D.ampmin) / (Ctrl->D.ampmax - Ctrl->D.ampmin);
						}
					}
				}

				/* decide whether to plot, whether to save the new ping, and if done */
				plot  = false;
				flush = false;
				if (*npings >= MAXPINGS)
					plot = true;
				if (*npings > 0 && (error >  MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP || error == MB_ERROR_OUT_BOUNDS ||
									error == MB_ERROR_OUT_TIME || error == MB_ERROR_SPEED_TOO_SMALL)) {
					plot = true;
					flush = true;
				}
				save_new = MB_NO;
				if (error == MB_ERROR_TIME_GAP)
					save_new = MB_YES;
				if (error > MB_ERROR_NO_ERROR)
					done = true;

				/* if enough pings read in, plot them */
				if (plot) {
					/* get footprint locations */
					if (Ctrl->A.mode != MBSWATH_FOOTPRINT_POINT)
						mbimport_get_footprints(Ctrl, &error);

					/* get shading */
					if (Ctrl->Z.mode == MBSWATH_BATH_RELIEF || Ctrl->Z.mode == MBSWATH_BATH_AMP)
						mbimport_get_shading(Ctrl, GMT, CPTshade, &error);

					/* plot data */
					if (start == MB_YES) {
						first = 0;
						start = MB_NO;
					}
					else
						first = 1;
					if (done)
						nplot = *npings - first;
					else
						nplot = *npings - first - 1;

					if (Ctrl->A.mode == MBSWATH_FOOTPRINT_POINT)
						mbimport_plot_data_point(Ctrl, GMT, CPTcolor, first, nplot, &error);
					else
						mbimport_plot_data_footprint(Ctrl, GMT, CPTcolor, first, nplot, &error);

					/* reorganize data */
					if (flush && save_new == MB_YES) {
						mbimport_ping_copy(0, *npings, Ctrl->swath_plot, &error);
						*npings = 1;
						start = MB_YES;
					}
					else if (flush) {
						*npings = 0;
						start = MB_YES;
					}
					else if (*npings > 1) {
						for (i = 0; i < 2; i++)
							mbimport_ping_copy(i, *npings-2+i, Ctrl->swath_plot, &error);
						*npings = 2;
					}
				}
			}
			mb_close(verbose,&Ctrl->mbio_ptr,&error);

			mb_freed(verbose,__FILE__, __LINE__, (void **)&Ctrl->swath_plot, &error);	/* deallocate memory for data arrays */
		} /* end if file in bounds */

		/* figure out whether and what to read next */
		if (Ctrl->read_datalist == MB_YES) {
			if ((status = mb_datalist_read(verbose, Ctrl->datalist, file, dfile, &format, &Ctrl->file_weight, &error)) == MB_SUCCESS)
				read_data = true;
			else
				read_data = false;
		}
		else
			read_data = false;

	} /* end loop over files in list */

	if (Ctrl->read_datalist == MB_YES)
		mb_datalist_close(verbose, &Ctrl->datalist, &error);

	dim[GMT_X] = Ctrl->nx;	dim[GMT_Y] = Ctrl->ny;
	dim[GMT_Z] = (Ctrl->image_type == MBSWATH_IMAGE_8) ? 1 : 3; 
	xy_inc[0] = Ctrl->x_inc;	xy_inc[1] = Ctrl->y_inc;
	if ((I = GMT_Create_Data (API, GMT_IS_IMAGE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, dim, GMT->common.R.wesn, xy_inc, 0, 0, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Could not create Image structure\n");
		gmt_M_free (GMT, Ctrl->bitimage);
		return EXIT_FAILURE;
	}

	if (Ctrl->Z.mode == MBSWATH_BATH)               strncpy(Title, "Bathymetry color fill", GMT_LEN32-1);
	else if (Ctrl->Z.mode == MBSWATH_BATH_RELIEF)   strncpy(Title, "Shaded relief bathymetry", GMT_LEN32-1);
	else if (Ctrl->Z.mode == MBSWATH_BATH_AMP)      strncpy(Title, "Bathymetry shaded by amplitude", GMT_LEN32-1);
	else if (Ctrl->Z.mode == MBSWATH_AMP)           strncpy(Title, "Amplitude", GMT_LEN32-1);
	else if (Ctrl->Z.mode == MBSWATH_SS)            strncpy(Title, "Side Scan", GMT_LEN32-1);
	strcpy(I->header->title, Title);

	I->data = Ctrl->bitimage;

	I->type = GMT_CHAR;
	I->header->n_columns = (uint32_t)dim[GMT_X];	I->header->n_rows = (uint32_t)dim[GMT_Y];	I->header->n_bands = (uint32_t)dim[GMT_Z];
	I->header->registration = GMT_GRID_PIXEL_REG;
	gmt_M_memcpy (I->header->mem_layout, "TCBa", 4, char);  /* Signal that data is Band interleaved */
	gmt_M_grd_setpad (GMT, I->header, nopad);               /* Copy the no pad to the header */
	if (GMT_Write_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->M.file, I) != GMT_OK)
		return EXIT_FAILURE;

	if (!Ctrl->C.active && GMT_Destroy_Data (API, &CPTcolor) != GMT_OK)
		Return (API->error);
	if (!Ctrl->N.active && GMT_Destroy_Data (API, &CPTshade) != GMT_OK)
		Return (API->error);

	Return (EXIT_SUCCESS);
}

/*--------------------------------------------------------------------*/
GMT_LOCAL int mbimport_get_footprints(struct CTRL *Ctrl, int *error) {
	int	   dobath, doss, setprint, i, j;
	struct swath_local *swath = NULL;
	struct ping_local *pingcur = NULL;
	struct footprint *print = NULL;
	double headingx = 1, headingy = 1, dx, dy, r, tt, x, y;
	double dlon1 = 0, dlon2 = 0, dlat1 = 0, dlat2 = 0;
	double ddlonx, ddlaty, rfactor = 1;
	static double dddepth = 0.0;
	struct mb_io_struct *mb_io_ptr = NULL;
				
	/* get swath */
	swath = Ctrl->swath_plot;
	mb_io_ptr = (struct mb_io_struct *) Ctrl->mbio_ptr;

	/* set mode of operation */
	if (Ctrl->Z.mode != MBSWATH_SS && Ctrl->Z.mode != MBSWATH_SS_FILTER) {
		dobath = MB_YES;
		doss = MB_NO;
	}
	else {
		dobath = MB_NO;
		doss = MB_YES;
	}

	/* set all footprint flags to zero */
	for (i = 0; i < swath->npings; i++) {
		pingcur = &swath->data[i];
		for (j = 0; j < pingcur->beams_bath; j++)
			pingcur->bathflag[j] = 0;
		for (j = 0; j < pingcur->pixels_ss; j++)
			pingcur->ssflag[j] = 0;
	}

	/* get fore-aft components of beam footprints */
	if (swath->npings > 1 && Ctrl->A.mode == MBSWATH_FOOTPRINT_FAKE) {
		for (i = 0; i < swath->npings; i++) {
			/* initialize */
			pingcur = &swath->data[i];
			pingcur->lonaft = 0.0;
			pingcur->lataft = 0.0;
			pingcur->lonfor = 0.0;
			pingcur->latfor = 0.0;

			/* get aft looking */
			if (i > 0) {
				headingx = sin(pingcur->heading*DTR);
				headingy = cos(pingcur->heading*DTR);
				dx = (swath->data[i-1].navlon - pingcur->navlon) /Ctrl->mtodeglon;
				dy = (swath->data[i-1].navlat - pingcur->navlat) /Ctrl->mtodeglat;
				r = sqrt(dx*dx + dy*dy);
				pingcur->lonaft = Ctrl->footprint_factor * r * headingx * Ctrl->mtodeglon;
				pingcur->lataft = Ctrl->footprint_factor * r * headingy * Ctrl->mtodeglat;
			}
			else {			/* take care of first ping */
				pingcur->lonaft = -pingcur->lonfor;
				pingcur->lataft = -pingcur->latfor;
			}

			/* get forward looking */
			if (i < swath->npings - 1) {
				headingx = sin(pingcur->heading*DTR);
				headingy = cos(pingcur->heading*DTR);
				dx = (swath->data[i+1].navlon - pingcur->navlon) /Ctrl->mtodeglon;
				dy = (swath->data[i+1].navlat - pingcur->navlat) /Ctrl->mtodeglat;
				r = sqrt(dx*dx + dy*dy);
				pingcur->lonfor = Ctrl->footprint_factor * r * headingx * Ctrl->mtodeglon;
				pingcur->latfor = Ctrl->footprint_factor * r * headingy * Ctrl->mtodeglat;
			}
			else {			/* take care of last ping */
				pingcur->lonfor = -pingcur->lonaft;
				pingcur->latfor = -pingcur->lataft;
			}
		}
	}
	/* take care of just one ping with nonzero center beam */
	else if (swath->npings == 1 && Ctrl->A.mode == MBSWATH_FOOTPRINT_FAKE &&
	         mb_beam_ok(swath->data[0].beamflag[pingcur->beams_bath/2]) && Ctrl->A.depth <= 0.0) {
	  pingcur = &swath->data[0];
	  headingx = sin(pingcur->heading*DTR);
	  headingy = cos(pingcur->heading*DTR);
	  tt = pingcur->bath[pingcur->beams_bath/2]/750.0; /* in s */
	  r = tt * pingcur->speed * 0.55555556; /* in m */
	  pingcur->lonaft = -Ctrl->footprint_factor * r * headingx * Ctrl->mtodeglon;
	  pingcur->lataft = -Ctrl->footprint_factor * r * headingy * Ctrl->mtodeglat;
	  pingcur->lonfor = Ctrl->footprint_factor * r * headingx * Ctrl->mtodeglon;
	  pingcur->latfor = Ctrl->footprint_factor * r * headingy * Ctrl->mtodeglat;
	}
	else if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL) /* else get rfactor if using fore-aft beam width */
		rfactor = 0.5*sin(DTR*Ctrl->footprint_factor);

	/* loop over the inner beams and get the obvious footprint boundaries */
	for (i = 0; i < swath->npings; i++) {
		pingcur = &swath->data[i];

		/* get heading if using fore-aft beam width */
		if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL) {
			headingx = sin(pingcur->heading*DTR);
			headingy = cos(pingcur->heading*DTR);
		}

		/* do bathymetry */
		if (dobath == MB_YES) {
			for (j = 1; j < pingcur->beams_bath - 1; j++) {
				if (mb_beam_ok(pingcur->beamflag[j])) {
					x = pingcur->bathlon[j];
					y = pingcur->bathlat[j];
					setprint = MB_NO;
					if (mb_beam_ok(pingcur->beamflag[j-1]) && mb_beam_ok(pingcur->beamflag[j+1])) {
						setprint = MB_YES;
						dlon1 = pingcur->bathlon[j-1] - pingcur->bathlon[j];
						dlat1 = pingcur->bathlat[j-1] - pingcur->bathlat[j];
						dlon2 = pingcur->bathlon[j+1] - pingcur->bathlon[j];
						dlat2 = pingcur->bathlat[j+1] - pingcur->bathlat[j];
					}
					else if (mb_beam_ok(pingcur->beamflag[j-1])) {
						setprint = MB_YES;
						dlon1 = pingcur->bathlon[j-1] - pingcur->bathlon[j];
						dlat1 = pingcur->bathlat[j-1] - pingcur->bathlat[j];
						dlon2 = -dlon1;
						dlat2 = -dlat1;
					}
					else if (mb_beam_ok(pingcur->beamflag[j+1])) {
						setprint = MB_YES;
						dlon2 = pingcur->bathlon[j+1] - pingcur->bathlon[j];
						dlat2 = pingcur->bathlat[j+1] - pingcur->bathlat[j];
						dlon1 = -dlon2;
						dlat1 = -dlat2;
					}

					if (setprint == MB_YES && Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL) { /* do it using fore-aft beam width */
						print = &pingcur->bathfoot[j];
						pingcur->bathflag[j] = MB_YES;
						ddlonx = (pingcur->bathlon[j] - pingcur->navlon)/Ctrl->mtodeglon;
						ddlaty = (pingcur->bathlat[j] - pingcur->navlat)/Ctrl->mtodeglat;
						if (Ctrl->A.depth > 0.0)
							dddepth = Ctrl->A.depth;
						else if (pingcur->altitude > 0.0)
							dddepth = pingcur->altitude;
						else
							dddepth = pingcur->bath[j];
						r = rfactor*sqrt(ddlonx*ddlonx + ddlaty*ddlaty + dddepth*dddepth);
						pingcur->lonaft = -r * headingx * Ctrl->mtodeglon;
						pingcur->lataft = -r * headingy * Ctrl->mtodeglat;
						pingcur->lonfor = r * headingx * Ctrl->mtodeglon;
						pingcur->latfor = r * headingy * Ctrl->mtodeglat;
						print->x[0] = x + dlon1 + pingcur->lonaft;
						print->y[0] = y + dlat1 + pingcur->lataft;
						print->x[1] = x + dlon2 + pingcur->lonaft;
						print->y[1] = y + dlat2 + pingcur->lataft;
						print->x[2] = x + dlon2 + pingcur->lonfor;
						print->y[2] = y + dlat2 + pingcur->latfor;
						print->x[3] = x + dlon1 + pingcur->lonfor;
						print->y[3] = y + dlat1 + pingcur->latfor;
					}
					else if (setprint == MB_YES) { /* do it using ping nav separation */
						print = &pingcur->bathfoot[j];
						pingcur->bathflag[j] = MB_YES;
						print->x[0] = x + dlon1 + pingcur->lonaft;
						print->y[0] = y + dlat1 + pingcur->lataft;
						print->x[1] = x + dlon2 + pingcur->lonaft;
						print->y[1] = y + dlat2 + pingcur->lataft;
						print->x[2] = x + dlon2 + pingcur->lonfor;
						print->y[2] = y + dlat2 + pingcur->latfor;
						print->x[3] = x + dlon1 + pingcur->lonfor;
						print->y[3] = y + dlat1 + pingcur->latfor;
					}
				}
			}
		}

		/* do sidescan */
		if (doss == MB_YES) {
			for (j = 1; j < pingcur->pixels_ss - 1; j++) {
				if (pingcur->ss[j] > MB_SIDESCAN_NULL) {
					x = pingcur->sslon[j];
					y = pingcur->sslat[j];
					setprint = MB_NO;
					if (pingcur->ss[j-1] > MB_SIDESCAN_NULL && pingcur->ss[j+1] > MB_SIDESCAN_NULL) {
						setprint = MB_YES;
						dlon1 = pingcur->sslon[j-1] - pingcur->sslon[j];
						dlat1 = pingcur->sslat[j-1] - pingcur->sslat[j];
						dlon2 = pingcur->sslon[j+1] - pingcur->sslon[j];
						dlat2 = pingcur->sslat[j+1] - pingcur->sslat[j];
					}
					else if (pingcur->ss[j-1] > MB_SIDESCAN_NULL) {
						setprint = MB_YES;
						dlon1 = pingcur->sslon[j-1] - pingcur->sslon[j];
						dlat1 = pingcur->sslat[j-1] - pingcur->sslat[j];
						dlon2 = -dlon1;
						dlat2 = -dlat1;
					}
					else if (pingcur->ss[j+1] > MB_SIDESCAN_NULL) {
						setprint = MB_YES;
						dlon2 = pingcur->sslon[j+1] - pingcur->sslon[j];
						dlat2 = pingcur->sslat[j+1] - pingcur->sslat[j];
						dlon1 = -dlon2;
						dlat1 = -dlat2;
					}

					/* do it using fore-aft beam width */
					if (setprint == MB_YES && Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL) {
						print = &pingcur->ssfoot[j];
						pingcur->ssflag[j] = MB_YES;
						ddlonx = (pingcur->sslon[j] - pingcur->navlon)/Ctrl->mtodeglon;
						ddlaty = (pingcur->sslat[j] - pingcur->navlat)/Ctrl->mtodeglat;
						if (Ctrl->A.depth > 0.0)
							dddepth = Ctrl->A.depth;
						else if (pingcur->altitude > 0.0)
							dddepth = pingcur->altitude;
						else if (pingcur->beams_bath > 0 && mb_beam_ok(pingcur->beamflag[pingcur->beams_bath/2]))
							dddepth = pingcur->bath[pingcur->beams_bath/2];
						r = rfactor*sqrt(ddlonx*ddlonx + ddlaty*ddlaty + dddepth*dddepth);
						pingcur->lonaft = -r * headingx * Ctrl->mtodeglon;
						pingcur->lataft = -r * headingy * Ctrl->mtodeglat;
						pingcur->lonfor = r * headingx * Ctrl->mtodeglon;
						pingcur->latfor = r * headingy * Ctrl->mtodeglat;
						print->x[0] = x + dlon1 + pingcur->lonaft;
						print->y[0] = y + dlat1 + pingcur->lataft;
						print->x[1] = x + dlon2 + pingcur->lonaft;
						print->y[1] = y + dlat2 + pingcur->lataft;
						print->x[2] = x + dlon2 + pingcur->lonfor;
						print->y[2] = y + dlat2 + pingcur->latfor;
						print->x[3] = x + dlon1 + pingcur->lonfor;
						print->y[3] = y + dlat1 + pingcur->latfor;
					}
					else if (setprint == MB_YES) {	/* do it using ping nav separation */
						print = &pingcur->ssfoot[j];
						pingcur->ssflag[j] = MB_YES;
						print->x[0] = x + dlon1 + pingcur->lonaft;
						print->y[0] = y + dlat1 + pingcur->lataft;
						print->x[1] = x + dlon2 + pingcur->lonaft;
						print->y[1] = y + dlat2 + pingcur->lataft;
						print->x[2] = x + dlon2 + pingcur->lonfor;
						print->y[2] = y + dlat2 + pingcur->latfor;
						print->x[3] = x + dlon1 + pingcur->lonfor;
						print->y[3] = y + dlat1 + pingcur->latfor;
					}
				}
			}
		}
	}

	/* loop over the outer beams and get the obvious footprint boundaries */
	for (i = 0; i < swath->npings; i++) {
		pingcur = &swath->data[i];

		/* get heading if using fore-aft beam width */
		if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL) {
			headingx = sin(pingcur->heading*DTR);
			headingy = cos(pingcur->heading*DTR);
		}

		/* do bathymetry with more than 2 soundings */
		if (dobath == MB_YES && pingcur->beams_bath > 2) {
			j = 0;
			if (mb_beam_ok(pingcur->beamflag[j]) && mb_beam_ok(pingcur->beamflag[j+1])) {
				x = pingcur->bathlon[j];
				y = pingcur->bathlat[j];
				dlon2 = pingcur->bathlon[j+1] - pingcur->bathlon[j];
				dlat2 = pingcur->bathlat[j+1] - pingcur->bathlat[j];
				dlon1 = -dlon2;
				dlat1 = -dlat2;
				print = &pingcur->bathfoot[j];
				pingcur->bathflag[j] = MB_YES;

				/* using fore-aft beam width */
				if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL) {
					ddlonx = (pingcur->bathlon[j] - pingcur->navlon)/Ctrl->mtodeglon;
					ddlaty = (pingcur->bathlat[j] - pingcur->navlat)/Ctrl->mtodeglat;
					if (Ctrl->A.depth > 0.0)
						dddepth = Ctrl->A.depth;
					else if (pingcur->altitude > 0.0)
						dddepth = pingcur->altitude;
					else
						dddepth = pingcur->bath[j];
					r = rfactor*sqrt(ddlonx*ddlonx + ddlaty*ddlaty + dddepth*dddepth);
					pingcur->lonaft = -r * headingx * Ctrl->mtodeglon;
					pingcur->lataft = -r * headingy * Ctrl->mtodeglat;
					pingcur->lonfor = r * headingx * Ctrl->mtodeglon;
					pingcur->latfor = r * headingy * Ctrl->mtodeglat;
				}

				print->x[0] = x + dlon1 + pingcur->lonaft;
				print->y[0] = y + dlat1 + pingcur->lataft;
				print->x[1] = x + dlon2 + pingcur->lonaft;
				print->y[1] = y + dlat2 + pingcur->lataft;
				print->x[2] = x + dlon2 + pingcur->lonfor;
				print->y[2] = y + dlat2 + pingcur->latfor;
				print->x[3] = x + dlon1 + pingcur->lonfor;
				print->y[3] = y + dlat1 + pingcur->latfor;
			}
			j = pingcur->beams_bath-1;
			if (mb_beam_ok(pingcur->beamflag[j]) && mb_beam_ok(pingcur->beamflag[j-1])) {
				x = pingcur->bathlon[j];
				y = pingcur->bathlat[j];
				dlon1 = pingcur->bathlon[j-1] - pingcur->bathlon[j];
				dlat1 = pingcur->bathlat[j-1] - pingcur->bathlat[j];
				dlon2 = -dlon1;
				dlat2 = -dlat1;
				print = &pingcur->bathfoot[j];
				pingcur->bathflag[j] = MB_YES;

				/* using fore-aft beam width */
				if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL) {
					ddlonx = (pingcur->bathlon[j] - pingcur->navlon)/Ctrl->mtodeglon;
					ddlaty = (pingcur->bathlat[j] - pingcur->navlat)/Ctrl->mtodeglat;
					if (Ctrl->A.depth > 0.0)
						dddepth = Ctrl->A.depth;
					else if (pingcur->altitude > 0.0)
						dddepth = pingcur->altitude;
					else
						dddepth = pingcur->bath[j];
					r = rfactor*sqrt(ddlonx*ddlonx + ddlaty*ddlaty + dddepth*dddepth);
					pingcur->lonaft = -r * headingx * Ctrl->mtodeglon;
					pingcur->lataft = -r * headingy * Ctrl->mtodeglat;
					pingcur->lonfor = r * headingx * Ctrl->mtodeglon;
					pingcur->latfor = r * headingy * Ctrl->mtodeglat;
				}

				print->x[0] = x + dlon1 + pingcur->lonaft;
				print->y[0] = y + dlat1 + pingcur->lataft;
				print->x[1] = x + dlon2 + pingcur->lonaft;
				print->y[1] = y + dlat2 + pingcur->lataft;
				print->x[2] = x + dlon2 + pingcur->lonfor;
				print->y[2] = y + dlat2 + pingcur->latfor;
				print->x[3] = x + dlon1 + pingcur->lonfor;
				print->y[3] = y + dlat1 + pingcur->latfor;
			}
		}

		/* do bathymetry with 1 sounding */
		if (dobath == MB_YES && Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL && pingcur->beams_bath == 1) {
			if (mb_beam_ok(pingcur->beamflag[0])) {
				print = &pingcur->bathfoot[0];
				pingcur->bathflag[0] = MB_YES;
				ddlonx = (pingcur->bathlon[0] - pingcur->navlon)/Ctrl->mtodeglon;
				ddlaty = (pingcur->bathlat[0] - pingcur->navlat)/Ctrl->mtodeglat;
				if (Ctrl->A.depth > 0.0)
					dddepth = Ctrl->A.depth;
				else if (pingcur->altitude > 0.0)
					dddepth = pingcur->altitude;
				else
					dddepth = pingcur->bath[0];
				r = rfactor*sqrt(ddlonx*ddlonx + ddlaty*ddlaty + dddepth*dddepth);

				dlon2 = -r * headingy * Ctrl->mtodeglon;
				dlat2 = -r * headingx * Ctrl->mtodeglat;
				dlon1 = r * headingy * Ctrl->mtodeglon;
				dlat1 = r * headingx * Ctrl->mtodeglat;
				pingcur->lonaft = -r * headingx * Ctrl->mtodeglon;
				pingcur->lataft = -r * headingy * Ctrl->mtodeglat;
				pingcur->lonfor = r * headingx * Ctrl->mtodeglon;
				pingcur->latfor = r * headingy * Ctrl->mtodeglat;
				print->x[0] = pingcur->bathlon[0] + dlon1 + pingcur->lonaft;
				print->y[0] = pingcur->bathlat[0] + dlat1 + pingcur->lataft;
				print->x[1] = pingcur->bathlon[0] + dlon2 + pingcur->lonaft;
				print->y[1] = pingcur->bathlat[0] + dlat2 + pingcur->lataft;
				print->x[2] = pingcur->bathlon[0] + dlon2 + pingcur->lonfor;
				print->y[2] = pingcur->bathlat[0] + dlat2 + pingcur->latfor;
				print->x[3] = pingcur->bathlon[0] + dlon1 + pingcur->lonfor;
				print->y[3] = pingcur->bathlat[0] + dlat1 + pingcur->latfor;
			}
		}

		/* do sidescan */
		if (doss == MB_YES && pingcur->pixels_ss > 2) {
			j = 0;
			if (pingcur->ss[j] > MB_SIDESCAN_NULL && pingcur->ss[j+1] > MB_SIDESCAN_NULL) {
				x = pingcur->sslon[j];
				y = pingcur->sslat[j];
				dlon2 = pingcur->sslon[j+1] - pingcur->sslon[j];
				dlat2 = pingcur->sslat[j+1] - pingcur->sslat[j];
				dlon1 = -dlon2;
				dlat1 = -dlat2;
				print = &pingcur->ssfoot[j];
				pingcur->ssflag[j] = MB_YES;

				/* using fore-aft beam width */
				if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL) {
					ddlonx = (pingcur->sslon[j] - pingcur->navlon)/Ctrl->mtodeglon;
					ddlaty = (pingcur->sslat[j] - pingcur->navlat)/Ctrl->mtodeglat;
					if (Ctrl->A.depth > 0.0)
						dddepth = Ctrl->A.depth;
					else if (pingcur->altitude > 0.0)
						dddepth = pingcur->altitude;
					else if (pingcur->beams_bath > 0 && mb_beam_ok(pingcur->beamflag[pingcur->beams_bath/2]))
						dddepth = pingcur->bath[pingcur->beams_bath/2];
					r = rfactor*sqrt(ddlonx*ddlonx + ddlaty*ddlaty + dddepth*dddepth);
					pingcur->lonaft = -r * headingx * Ctrl->mtodeglon;
					pingcur->lataft = -r * headingy * Ctrl->mtodeglat;
					pingcur->lonfor = r * headingx * Ctrl->mtodeglon;
					pingcur->latfor = r * headingy * Ctrl->mtodeglat;
				}

				print->x[0] = x + dlon1 + pingcur->lonaft;
				print->y[0] = y + dlat1 + pingcur->lataft;
				print->x[1] = x + dlon2 + pingcur->lonaft;
				print->y[1] = y + dlat2 + pingcur->lataft;
				print->x[2] = x + dlon2 + pingcur->lonfor;
				print->y[2] = y + dlat2 + pingcur->latfor;
				print->x[3] = x + dlon1 + pingcur->lonfor;
				print->y[3] = y + dlat1 + pingcur->latfor;
			}

			j = pingcur->pixels_ss-1;
			if (pingcur->ss[j] > MB_SIDESCAN_NULL && pingcur->ss[j-1] > MB_SIDESCAN_NULL) {
				x = pingcur->sslon[j];
				y = pingcur->sslat[j];
				dlon1 = pingcur->sslon[j-1] - pingcur->sslon[j];
				dlat1 = pingcur->sslat[j-1] - pingcur->sslat[j];
				dlon2 = -dlon1;
				dlat2 = -dlat1;
				print = &pingcur->ssfoot[j];
				pingcur->ssflag[j] = MB_YES;

				/* using fore-aft beam width */
				if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL) {
					ddlonx = (pingcur->sslon[j] - pingcur->navlon)/Ctrl->mtodeglon;
					ddlaty = (pingcur->sslat[j] - pingcur->navlat)/Ctrl->mtodeglat;
					if (Ctrl->A.depth > 0.0)
						dddepth = Ctrl->A.depth;
					else if (pingcur->altitude > 0.0)
						dddepth = pingcur->altitude;
					else if (pingcur->beams_bath > 0 && mb_beam_ok(pingcur->beamflag[pingcur->beams_bath/2]))
						dddepth = pingcur->bath[pingcur->beams_bath/2];
					r = rfactor*sqrt(ddlonx*ddlonx + ddlaty*ddlaty + dddepth*dddepth);
					pingcur->lonaft = -r * headingx * Ctrl->mtodeglon;
					pingcur->lataft = -r * headingy * Ctrl->mtodeglat;
					pingcur->lonfor = r * headingx * Ctrl->mtodeglon;
					pingcur->latfor = r * headingy * Ctrl->mtodeglat;
				}

				print->x[0] = x + dlon1 + pingcur->lonaft;
				print->y[0] = y + dlat1 + pingcur->lataft;
				print->x[1] = x + dlon2 + pingcur->lonaft;
				print->y[1] = y + dlat2 + pingcur->lataft;
				print->x[2] = x + dlon2 + pingcur->lonfor;
				print->y[2] = y + dlat2 + pingcur->latfor;
				print->x[3] = x + dlon1 + pingcur->lonfor;
				print->y[3] = y + dlat1 + pingcur->latfor;
			}
		}
	}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	return(MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
GMT_LOCAL int mbimport_get_shading(struct CTRL *Ctrl, struct GMT_CTRL *GMT, struct GMT_PALETTE *CPT, int *error) {
	int    cpt_index, i, j, drvcount;
	struct swath_local *swath = NULL;
	struct ping_local *ping0 = NULL;
	struct ping_local *ping1 = NULL;
	struct ping_local *ping2 = NULL;
	double dx, dy, dd, dst2, drvx, drvy, sinx,cosy, median, graylevel, rgb[4];
				
	/* get swath */
	swath = Ctrl->swath_plot;

	/* get shading from directional bathymetric gradient */
	if (Ctrl->Z.mode == MBSWATH_BATH_RELIEF) {
		/* get directional factors */
		sinx = sin(DTR * Ctrl->G.azimuth);
		cosy = cos(DTR * Ctrl->G.azimuth);

		/* loop over the pings and beams */
		for (i = 0; i < swath->npings; i++) {
			if (i > 0) ping0 = &swath->data[i-1];
			ping1 = &swath->data[i];
			if (i < swath->npings - 1) ping2 = &swath->data[i+1];
			for (j = 0; j < ping1->beams_bath; j++) {
				if (mb_beam_ok(ping1->beamflag[j])) {
					/* do across track components */
					drvcount = 0;
					dx = dy = dd = drvx = drvy = 0.0;
					if (j > 0 && j < ping1->beams_bath - 1 && mb_beam_ok(ping1->beamflag[j-1]) && mb_beam_ok(ping1->beamflag[j+1])) {
						dx = (ping1->bathlon[j+1] - ping1->bathlon[j-1]) / Ctrl->mtodeglon;
						dy = (ping1->bathlat[j+1] - ping1->bathlat[j-1]) / Ctrl->mtodeglat;
						dd = ping1->bath[j+1] - ping1->bath[j-1];
					}
					else if (j < ping1->beams_bath - 1 && mb_beam_ok(ping1->beamflag[j]) && mb_beam_ok(ping1->beamflag[j+1])) {
						dx = (ping1->bathlon[j+1] - ping1->bathlon[j]) / Ctrl->mtodeglon;
						dy = (ping1->bathlat[j+1] - ping1->bathlat[j]) / Ctrl->mtodeglat;
						dd = ping1->bath[j+1] - ping1->bath[j];
					}
					else if (j > 0 && mb_beam_ok(ping1->beamflag[j-1]) && mb_beam_ok(ping1->beamflag[j])) {
						dx = (ping1->bathlon[j] - ping1->bathlon[j-1]) / Ctrl->mtodeglon;
						dy = (ping1->bathlat[j] - ping1->bathlat[j-1]) / Ctrl->mtodeglat;
						dd = ping1->bath[j] - ping1->bath[j-1];
					}
					dst2 = dx * dx + dy * dy;
					if (dst2 > 0.0) {
						drvx = dd * dx / dst2;
						drvy = dd * dy / dst2;
						drvcount++;
					}

					/* do along track components */
					dx = dy = dd = 0.0;
					if (i > 0 && i < swath->npings - 1 && mb_beam_ok(ping0->beamflag[j]) && mb_beam_ok(ping2->beamflag[j])) {
						dx = (ping2->bathlon[j] - ping0->bathlon[j]) / Ctrl->mtodeglon;
						dy = (ping2->bathlat[j] - ping0->bathlat[j]) / Ctrl->mtodeglat;
						dd = ping2->bath[j] - ping0->bath[j];
					}
					else if (i < swath->npings - 1 && mb_beam_ok(ping1->beamflag[j]) && mb_beam_ok(ping2->beamflag[j])) {
						dx = (ping2->bathlon[j] - ping1->bathlon[j]) / Ctrl->mtodeglon;
						dy = (ping2->bathlat[j] - ping1->bathlat[j]) / Ctrl->mtodeglat;
						dd = ping2->bath[j] - ping1->bath[j];
					}
					else if (i > 0 && mb_beam_ok(ping0->beamflag[j]) && mb_beam_ok(ping1->beamflag[j])) {
						dx = (ping1->bathlon[j] - ping0->bathlon[j]) / Ctrl->mtodeglon;
						dy = (ping1->bathlat[j] - ping0->bathlat[j]) / Ctrl->mtodeglat;
						dd = ping1->bath[j] - ping0->bath[j];
					}
					dst2 = dx * dx + dy * dy;
					if (dst2 > 0.0) {
						drvx = drvx + dd * dx / dst2;
						drvy = drvy + dd * dy / dst2;
						drvcount++;
					}

					/* calculate directional derivative */
					if (drvcount == 2)
						ping1->bathshade[j] = Ctrl->G.magnitude * (drvx*sinx + drvy*cosy);
					else
						ping1->bathshade[j] = 0.0;
				}
			}
		}
	}
	else if (Ctrl->N.active && Ctrl->Z.mode == MBSWATH_BATH_AMP) {	/* get shading from amplitude data using cpt file */
		/* loop over the pings and beams */
		for (i = 0; i < swath->npings; i++) {
			if (i > 0) ping0 = &swath->data[i-1];
			ping1 = &swath->data[i];
			if (i < swath->npings - 1) ping2 = &swath->data[i+1];
			for (j = 0; j < ping1->beams_bath; j++) {
				if (mb_beam_ok(ping1->beamflag[j])) {
					/* calculate shading */
					if (mb_beam_ok(ping1->beamflag[j])) {
						/* get shading value from cpt */
						cpt_index = gmt_get_rgb_from_z(GMT, CPT, ping1->amp[j], rgb);
						graylevel = (rgb[0] + rgb[1] + rgb[2]) / 3.0;
						ping1->bathshade[j] = Ctrl->G.magnitude * (graylevel - Ctrl->G.azimuth) / 128.;
					}
					else
						ping1->bathshade[j] = 0.0;
				}
			}
		}
	}
	else if (Ctrl->Z.mode == MBSWATH_BATH_AMP) {	/* get shading from amplitude data */
		/* get median value from value entered as azimuth */
		median = Ctrl->G.azimuth;

		/* loop over the pings and beams */
		for (i = 0; i < swath->npings; i++) {
			if (i > 0) ping0 = &swath->data[i-1];
			ping1 = &swath->data[i];
			if (i < swath->npings - 1) ping2 = &swath->data[i+1];
			for (j=0;j<ping1->beams_bath;j++) {
				if (mb_beam_ok(ping1->beamflag[j])) {
					/* calculate shading */
					if (mb_beam_ok(ping1->beamflag[j]))
						ping1->bathshade[j] = Ctrl->G.magnitude * (ping1->amp[j] - Ctrl->G.azimuth);
					else
						ping1->bathshade[j] = 0.0;
				}
			}
		}
	}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	return(MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
GMT_LOCAL int mbimport_plot_data_footprint(struct CTRL *Ctrl, struct GMT_CTRL *GMT, struct GMT_PALETTE *CPT,
										  int first, int nplot, int *error) {
	int	status = MB_SUCCESS;
	struct swath_local *swath;
	struct ping_local *pingcur;
	struct footprint *print;
	double	*x, *y;
	double	xx[4], yy[4], rgb[4];
	int     cpt_index, i, j, k;
				
	/* get swath */
	swath = Ctrl->swath_plot;

	if (Ctrl->Z.mode == MBSWATH_BATH || Ctrl->Z.mode == MBSWATH_BATH_RELIEF || Ctrl->Z.mode == MBSWATH_BATH_AMP) {
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++) {
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->beams_bath;j++) {
				if (pingcur->bathflag[j] == MB_YES) {
					print = &pingcur->bathfoot[j];
					x = &(print->x[0]);
					y = &(print->y[0]);
					for (k = 0; k < 4; k++)
						gmt_geo_to_xy(GMT, x[k], y[k], &xx[k], &yy[k]);
					cpt_index = gmt_get_rgb_from_z(GMT, CPT, pingcur->bath[j], rgb);
					if (Ctrl->Z.mode == MBSWATH_BATH_RELIEF || Ctrl->Z.mode == MBSWATH_BATH_AMP)
						gmt_illuminate(GMT, pingcur->bathshade[j], rgb);
					status = mbimport_plot_box(Ctrl, xx, yy, rgb, error);
				}
			}
		}
	}
	else if (Ctrl->Z.mode == MBSWATH_AMP) {
		for (i = first; i < first + nplot; i++) {			/* loop over all pings and beams and plot the good ones */
			pingcur = &swath->data[i];
			for (j = 0; j < pingcur->beams_amp; j++) {
				if (pingcur->bathflag[j] == MB_YES) {
					print = &pingcur->bathfoot[j];
					x = &(print->x[0]);
					y = &(print->y[0]);
					for (k = 0; k < 4; k++)
						gmt_geo_to_xy(GMT, x[k], y[k], &xx[k], &yy[k]);
					cpt_index = gmt_get_rgb_from_z(GMT, CPT, pingcur->amp[j], rgb);
					status = mbimport_plot_box(Ctrl, xx, yy, rgb, error);
				}
			}
		}
	}
	else if (Ctrl->Z.mode == MBSWATH_SS) {
		for (i = first; i < first+nplot; i++) {		/* loop over all pings and beams and plot the good ones */
			pingcur = &swath->data[i];
			for (j = 0; j < pingcur->pixels_ss; j++) {
				if (pingcur->ssflag[j] == MB_YES) {
					print = &pingcur->ssfoot[j];
					x = &(print->x[0]);
					y = &(print->y[0]);
					for (k = 0; k < 4; k++)
						gmt_geo_to_xy(GMT, x[k], y[k], &xx[k], &yy[k]);
					cpt_index = gmt_get_rgb_from_z(GMT, CPT, pingcur->ss[j], rgb);
					status = mbimport_plot_box(Ctrl, xx, yy, rgb, error);
				}
			}
		}
	}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	return(MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
GMT_LOCAL int mbimport_plot_data_point(struct CTRL *Ctrl, struct GMT_CTRL *GMT, struct GMT_PALETTE *CPT,
									  int first, int nplot, int *error) {
	int    status = MB_SUCCESS;
	int    cpt_index, i, j;
	struct swath_local *swath;
	struct ping_local	*pingcur;
	double xx, yy, rgb[4];
				
	/* get swath */
	swath = Ctrl->swath_plot;

	if (Ctrl->Z.mode == MBSWATH_BATH || Ctrl->Z.mode == MBSWATH_BATH_RELIEF || Ctrl->Z.mode == MBSWATH_BATH_AMP) {
		for (i = first; i < first + nplot; i++) {		/* loop over all pings and beams and plot the good ones */
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->beams_bath;j++) {
				if (mb_beam_ok(pingcur->beamflag[j])) {
					gmt_geo_to_xy(GMT, pingcur->bathlon[j], pingcur->bathlat[j], &xx, &yy);
					cpt_index = gmt_get_rgb_from_z(GMT, CPT, pingcur->bath[j], rgb);
					if (Ctrl->Z.mode == MBSWATH_BATH_RELIEF || Ctrl->Z.mode == MBSWATH_BATH_AMP)
						gmt_illuminate(GMT, pingcur->bathshade[j], rgb);
					status = mbimport_plot_point(Ctrl, xx, yy, rgb, error);
				}
			}
		}
	}
	else if (Ctrl->Z.mode == MBSWATH_AMP) {
		for (i=first;i<first+nplot;i++) {		/* loop over all pings and beams and plot the good ones */
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->beams_amp;j++) {
				if (mb_beam_ok(pingcur->beamflag[j])) {
					gmt_geo_to_xy(GMT, pingcur->bathlon[j], pingcur->bathlat[j], &xx, &yy);
					cpt_index = gmt_get_rgb_from_z(GMT, CPT, pingcur->amp[j], rgb);
					status = mbimport_plot_point(Ctrl, xx, yy, rgb, error);
				}
			}
		}
	}
	else if (Ctrl->Z.mode == MBSWATH_SS) {
		/* loop over all pings and beams and plot the good ones */
		for (i = first; i < first+nplot; i++) {
			pingcur = &swath->data[i];
			for (j = 0; j < pingcur->pixels_ss; j++) {
				if (pingcur->ss[j] > MB_SIDESCAN_NULL) {
					gmt_geo_to_xy(GMT, pingcur->sslon[j], pingcur->sslat[j], &xx, &yy);
					cpt_index = gmt_get_rgb_from_z(GMT, CPT, pingcur->ss[j], rgb);
					status = mbimport_plot_point(Ctrl, xx, yy, rgb, error);
				}
			}
		}
	}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	return(MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
GMT_LOCAL int mbimport_plot_box(struct CTRL *Ctrl, double *x, double *y, double *rgb, int *error) {
	int	ix[5], iy[5], ncross, xcross[10];
	int	ixmin, ixmax, iymin, iymax, ixx, iyy, ixx1, ixx2, i, j, k;
	double	dx, dy;

	/* get bounds of box in pixels */
	for (i = 0; i < 4; i++) {
		ix[i] = (int)(Ctrl->nx * x[i] / Ctrl->x_inch);
		iy[i] = (int)(Ctrl->ny * y[i] / Ctrl->y_inch);
	}
	ix[4] = ix[0];
	iy[4] = iy[0];

	/* get min max values of bounding corners in pixels */
	ixmin = ixmax = ix[0];
	iymin = iymax = iy[0];
	for (i = 1; i < 4; i++) {
		if (ix[i] < ixmin) ixmin = ix[i];
		else if (ix[i] > ixmax) ixmax = ix[i];
		if (iy[i] < iymin) iymin = iy[i];
		else if (iy[i] > iymax) iymax = iy[i];
	}
	if (ixmin < 0) ixmin = 0;
	if (ixmax > Ctrl->nx-1) ixmax = Ctrl->nx - 1;
	if (iymin < 1) iymin = 1;
	if (iymax > Ctrl->ny-1) iymax = Ctrl->ny - 1;

	/* loop over all y values */
	for (iyy = iymin; iyy <= iymax; iyy++) {
		/* find crossings */
		ncross = 0;
		for (i = 0; i < 4; i++) {
			if ((iy[i] <= iyy && iy[i+1] >= iyy) || (iy[i] >= iyy && iy[i+1] <= iyy)) {
				if (iy[i] == iy[i+1]) {
					xcross[ncross] = ix[i];
					ncross++;
					xcross[ncross] = ix[i+1];
					ncross++;
				}
				else {
					dy = iy[i+1] - iy[i];
					dx = ix[i+1] - ix[i];
					xcross[ncross] = (int)((iyy - iy[i]) * dx / dy + ix[i]);
					ncross++;
				}
			}
		}

		/* plot lines between crossings */
		for (j = 0; j < ncross-1; j++) {
			if (xcross[j] < xcross[j+1]) {
				ixx1 = xcross[j];
				ixx2 = xcross[j+1];
			}
			else {
				ixx1 = xcross[j+1];
				ixx2 = xcross[j];
			}
			if ((ixx1 < ixmin && ixx2 < ixmin) || (ixx1 > ixmax && ixx2 > ixmax))
				ixx2 = ixx1 - 1; /* disable plotting */
			else {
				if (ixx1 < ixmin) ixx1 = ixmin;
				else if (ixx2 > ixmax) ixx2 = ixmax;
			}
			for (ixx = ixx1; ixx <= ixx2; ixx++) {
				int row, col;
				row = iyy;
				col = ixx;
				k   = col * Ctrl->ny + row;
				if (Ctrl->image_type == MBSWATH_IMAGE_8) {
					Ctrl->bitimage[k] = (unsigned char)(255 * YIQ(rgb));
				}
				else {
					Ctrl->bitimage[k] = (unsigned char)(255 * rgb[0]);
					Ctrl->bitimage[k + Ctrl->nx * Ctrl->ny]   = (unsigned char)(255 * rgb[1]);
					Ctrl->bitimage[k + 2*Ctrl->nx * Ctrl->ny] = (unsigned char)(255 * rgb[2]);
				}
			}
		}
	}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	return(MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
GMT_LOCAL int mbimport_plot_point(struct CTRL *Ctrl, double x, double y, double *rgb, int *error) {
	int	ix, iy, k;

	/* get pixel */
	ix = (int)(Ctrl->nx * x / Ctrl->x_inch);
	iy = (int)(Ctrl->ny * y / Ctrl->y_inch);

	/* plot pixel */
	if (Ctrl->image_type == MBSWATH_IMAGE_8) {
		k = Ctrl->nx * (Ctrl->ny - iy) + ix;
		Ctrl->bitimage[k] = (unsigned char ) YIQ(rgb);
	}
	else {
		k = 3 * (Ctrl->nx * (Ctrl->ny - iy) + ix);
		Ctrl->bitimage[k] = (unsigned char) (255 * rgb[0]);
		Ctrl->bitimage[k+1] = (unsigned char) (255 * rgb[1]);
		Ctrl->bitimage[k+2] = (unsigned char) (255 * rgb[2]);
	}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
int mbimport_ping_copy(int one, int two, struct swath_local *swath, int *error) {
	struct ping_local *ping1;
	struct ping_local *ping2;
	int	i, j;

	/* copy things */
	ping1 = &swath->data[one];
	ping2 = &swath->data[two];
	ping1->pings = ping2->pings;
	ping1->kind = ping2->kind;
	for (i = 0; i < 7; i++)
		ping1->time_i[i] = ping2->time_i[i];
	ping1->time_d = ping2->time_d;
	ping1->navlon = ping2->navlon;
	ping1->navlat = ping2->navlat;
	ping1->speed = ping2->speed;
	ping1->heading = ping2->heading;
	ping1->distance = ping2->distance;
	ping1->altitude = ping2->altitude;
	ping1->sonardepth = ping2->sonardepth;
	strcpy(ping1->comment,ping2->comment);
	ping1->beams_bath = ping2->beams_bath;
	ping1->beams_amp = ping2->beams_amp;
	ping1->pixels_ss = ping2->pixels_ss;
	for (i=0;i<ping1->beams_bath;i++) {
		ping1->beamflag[i] = ping2->beamflag[i];
		ping1->bath[i] = ping2->bath[i];
		ping1->bathlon[i] = ping2->bathlon[i];
		ping1->bathlat[i] = ping2->bathlat[i];
		ping1->bathflag[i] = ping2->bathflag[i];
		for (j=0;j<4;j++) {
			ping1->bathfoot[i].x[j] = ping2->bathfoot[i].x[j];
			ping1->bathfoot[i].y[j] = ping2->bathfoot[i].y[j];
		}
	}
	for (i = 0; i < ping1->beams_amp; i++)
		ping1->amp[i] = ping2->amp[i];
	for (i=0;i<ping1->pixels_ss;i++) {
		ping1->ss[i] = ping2->ss[i];
		ping1->sslon[i] = ping2->sslon[i];
		ping1->sslat[i] = ping2->sslat[i];
		ping1->ssflag[i] = ping2->ssflag[i];
		for (j=0;j<4;j++) {
			ping1->ssfoot[i].x[j] = ping2->ssfoot[i].x[j];
			ping1->ssfoot[i].y[j] = ping2->ssfoot[i].y[j];
		}
	}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	return MB_SUCCESS;
}
