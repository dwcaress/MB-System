/*--------------------------------------------------------------------
 *    The MB-system:	mbcontour.c	5/30/93
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
 * MBCONTOUR is a GMT compatible utility which creates a color postscript
 * image of swath bathymetry or backscatter data.  The image
 * may be shaded relief as well.  Complete maps are made by using
 * MBCONTOUR in conjunction with the usual GMT programs.  The modes
 * of operation are:
 *   Mode 1:  Bathymetry
 *   Mode 2:  Bathymetry shaded by illumination
 *   Mode 3:  Bathymetry shaded by amplitude
 *   Mode 4:  amplitude
 *   Mode 5:  sidescan
 *   Mode 6:  Bathymetry shaded by amplitude using cpt gray data
 *
 * Author:	D. W. Caress
 * Date:	May 30, 1993 (original standalone program for GMT3 and GMT4)
 *                           (also originally served to drive Calcomp style pen
 *                              plotters in real time)
 * Date:	January 27, 2015 (recast as GMT5 module, code supporting pen
 *                                  plotters removed)
 */

//#include <stdbool.h>

/* GMT header file */
#include "gmt_dev.h"

/*  Compatibility with old lower-function/macro names use prior to GMT 5.3.0 */
#if GMT_MAJOR_VERSION == 5 && GMT_MINOR_VERSION < 3
#define gmt_M_180_range GMT_180_RANGE
#define gmt_M_free_options GMT_Free_Options
#define gmt_M_ijp GMT_IJP
#define gmt_M_ijpgi GMT_IJPGI
#define gmt_M_check_condition GMT_check_condition
#define gmt_M_get_inc GMT_get_inc
#define gmt_M_get_n GMT_get_n
#define gmt_M_grd_is_global GMT_grd_is_global
#define gmt_M_grd_same_region GMT_grd_same_region
#define gmt_M_is255 GMT_is255
#define gmt_M_is_geographic GMT_is_geographic
#define gmt_M_memcpy GMT_memcpy
#define gmt_M_rgb_copy GMT_rgb_copy
#define gmt_M_u255 GMT_u255
#define gmt_M_err_fail GMT_err_fail
#define gmt_M_free GMT_free
#define gmt_M_is_fnan GMT_is_fnan
#define gmt_M_memory GMT_memory
#define gmt_get_cpt GMT_Get_CPT
#define gmt_access GMT_access
#define gmt_begin_module GMT_begin_module
#define gmt_check_filearg GMT_check_filearg
#define gmt_default_error GMT_default_error
#define gmt_end_module GMT_end_module
#define gmt_geo_to_xy GMT_geo_to_xy
#define gmt_get_api_ptr GMT_get_API_ptr
#define gmt_get_rgb_from_z GMT_get_rgb_from_z
#define gmt_getrgb GMT_getrgb
#define gmt_grd_project GMT_grd_project
#define gmt_grd_setregion GMT_grd_setregion
#define gmt_illuminate GMT_illuminate
#define gmt_map_basemap GMT_map_basemap
#define gmt_map_clip_off GMT_map_clip_off
#define gmt_map_clip_on GMT_map_clip_on
#define gmt_map_setup GMT_map_setup
#define gmt_not_numeric GMT_not_numeric
#define gmt_plane_perspective GMT_plane_perspective
#define gmt_plotcanvas GMT_plotcanvas
#define gmt_plotend GMT_plotend
#define gmt_plotinit GMT_plotinit
#define gmt_project_init GMT_project_init
#define gmt_putrgb GMT_putrgb
#define gmt_rgb_syntax GMT_rgb_syntax
#define gmt_set_grddim GMT_set_grddim
#define gmt_show_name_and_purpose GMT_show_name_and_purpose
#endif

/* Compatibility with old GMT_MSG names prior to version 6 */
#if GMT_MAJOR_VERSION == 6 && GMT_MINOR_VERSION <= 2
#define GMT_MSG_ERROR GMT_MSG_NORMAL
#elif GMT_MAJOR_VERSION < 6
#define GMT_MSG_ERROR GMT_MSG_NORMAL
#endif

#define THIS_MODULE_CLASSIC_NAME "mbcontour"
#define THIS_MODULE_MODERN_NAME "mbcontour"
#define THIS_MODULE_LIB "mbsystem"
#define THIS_MODULE_PURPOSE "Plot swath bathymetry, amplitude, or backscatter"

#define THIS_MODULE_KEYS "<G{+,>}"
#define THIS_MODULE_NEEDS "g"
#define THIS_MODULE_OPTIONS "-:>RV" GMT_OPT("H")

#define GMT_PROG_OPTIONS "->BJKOPRUVXY" GMT_OPT("S")

// Stop warnings about packaging collision between GDAL's cpl_port.h and the 
// Autotools build system mb_config.h
#ifdef PACKAGE_BUGREPORT
#undef PACKAGE_BUGREPORT
#endif
#ifdef PACKAGE_NAME
#undef PACKAGE_NAME
#endif
#ifdef PACKAGE_STRING
#undef PACKAGE_STRING
#endif
#ifdef PACKAGE_TARNAME
#undef PACKAGE_TARNAME
#endif
#ifdef PACKAGE_URL
#undef PACKAGE_URL
#endif
#ifdef PACKAGE_VERSION
#undef PACKAGE_VERSION
#endif

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"

EXTERN_MSC int GMT_mbcontour(void *V_API, int mode, void *args);

/* Control structure for mbcontour */
struct MBCONTOUR_CTRL {

	struct mbcontour_A { /* -A<cont_int>/<col_int>/<tick_int>/<label_int>/<tick_len>/<label_hgt>/<label_spacing> */
		bool active;
		double cont_int;
		double col_int;
		double tick_int;
		double label_int;
		double tick_len;
		double label_hgt;
		double label_spacing;
	} A;
	struct mbcontour_b { /* -b<year>/<month>/<day>/<hour>/<minute>/<second> */
		bool active;
		int time_i[7];
	} b;
	struct mbcontour_C { /* -C<contourfile> */
		bool active;
		char *contourfile;
	} C;
	struct mbcontour_D { /* -D<time_tick_int>/<time_annot_int>/<date_annot_int>/<time_tick_len> */
		bool active;
		double time_tick_int;
		double time_annot_int;
		double date_annot_int;
		double time_tick_len;
	} D;
	struct mbcontour_e { /* -e<year>/<month>/<day>/<hour>/<minute>/<second> */
		bool active;
		int time_i[7];
	} e;
	struct mbcontour_F { /* -F<format> */
		bool active;
		int format;
	} F;
	struct mbcontour_G { /* -G<name_hgt>/<name_perp> */
		bool active;
		double name_hgt;
		int name_perp;
	} G;
	struct mbcontour_I { /* -I<inputfile> */
		bool active;
		char *inputfile;
	} I;
	struct mbcontour_L { /* -L<lonflip> */
		bool active;
		int lonflip;
	} L;
	struct mbcontour_M { /* -M<pingnumber_tick_int>/<pingnumber_annot_int>/<pingnumber_tick_len> */
		bool active;
		double pingnumber_tick_int;
		double pingnumber_annot_int;
		double pingnumber_tick_len;
	} M;
	struct mbcontour_N { /* -N<nplot> */
		bool active;
		int nplot;
	} N;
	struct mbcontour_p { /* -p<pings> */
		bool active;
		int pings;
	} p;
	struct mbcontour_Q { /* -Q */
		bool active;
	} Q;
	struct mbcontour_S { /* -S<speedmin> */
		bool active;
		double speedmin;
	} S;
	struct mbcontour_T { /* -T<timegap> */
		bool active;
		double timegap;
	} T;
	struct mbcontour_W {	/* -W<pen>[+z] */
		bool active;
		bool cpt_effect;
		bool set_color;
		unsigned int sequential;
		struct GMT_PEN pen;
	} W;
	struct mbcontour_Z { /* -Z<algorithm> */
		bool active;
		int contour_algorithm;
	} Z;
};

/*--------------------------------------------------------------------*/
/* Global variables */

struct PSL_CTRL *PSL = NULL; /* General PSL interal parameters */
struct GMT_CTRL *GMT = NULL; /* General GMT interal parameters */

/* pen variables */
int ncolor;
// int nlevel;
double *level = NULL;
int *red = NULL;
int *green = NULL;
int *blue = NULL;
int *label = NULL;
int *tick = NULL;

/* inch to map scale */
double inchtolon;

/* line plotting variables */
#define MBCONTOUR_PLOT_ALLOC_INC 1024
#define MBCONTOUR_PLOT_MOVE 3
#define MBCONTOUR_PLOT_DRAW 2
#define MBCONTOUR_PLOT_STROKE -2
#define MBCONTOUR_PLOT_OR -3
int ncontour_plot = 0;
int ncontour_plot_alloc = 0;
double *contour_x = NULL;
double *contour_y = NULL;

/*--------------------------------------------------------------------*/
void *New_mbcontour_Ctrl(struct GMT_CTRL *GMT) { /* Allocate and initialize a new control structure */
	int verbose = 0;
	double dummybounds[4];
	int dummyformat;
	int dummypings;

	struct MBCONTOUR_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBCONTOUR_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	/* get current mb default values */
	// const int status =
	mb_defaults(verbose, &dummyformat, &dummypings, &Ctrl->L.lonflip, dummybounds, Ctrl->b.time_i, Ctrl->e.time_i,
	            &Ctrl->S.speedmin, &Ctrl->T.timegap);

	Ctrl->A.active = false;
	Ctrl->A.cont_int = 25.;
	Ctrl->A.col_int = 100.;
	Ctrl->A.tick_int = 100.;
	Ctrl->A.label_int = 100.;
	Ctrl->A.tick_len = 0.05;
	Ctrl->A.label_hgt = 0.1;
	Ctrl->A.label_spacing = 0.0;
	Ctrl->b.active = false;
	Ctrl->C.active = false;
	Ctrl->C.contourfile = NULL;
	Ctrl->D.active = false;
	Ctrl->D.time_tick_int = 0.25;
	Ctrl->D.time_annot_int = 1.0;
	Ctrl->D.date_annot_int = 4.0;
	Ctrl->D.time_tick_len = 0.1;
	Ctrl->e.active = false;
	Ctrl->F.active = false;
	Ctrl->F.format = 0;
	Ctrl->G.active = false;
	Ctrl->G.name_hgt = 0.1;
	Ctrl->G.name_perp = false;
	Ctrl->I.active = false;
	Ctrl->I.inputfile = NULL;
	Ctrl->L.active = false;
	Ctrl->M.active = false;
	Ctrl->M.pingnumber_tick_int = 50;
	Ctrl->M.pingnumber_annot_int = 100;
	Ctrl->M.pingnumber_tick_len = 0.1;
	Ctrl->N.active = false;
	Ctrl->N.nplot = 0;
	Ctrl->Q.active = false;
	Ctrl->S.active = false;
	Ctrl->T.active = false;
	Ctrl->W.active = false;
	Ctrl->W.pen = GMT->current.setting.map_default_pen;
	Ctrl->W.active = false;
	Ctrl->W.cpt_effect = false;
	Ctrl->W.set_color = false;
	Ctrl->W.sequential = 0;
	Ctrl->Z.active = false;
	Ctrl->Z.contour_algorithm = MB_CONTOUR_OLD;

	return (Ctrl);
}

void Free_mbcontour_Ctrl(struct GMT_CTRL *GMT, struct MBCONTOUR_CTRL *Ctrl) { /* Deallocate control structure */
	if (!Ctrl)
		return;
	gmt_M_str_free (Ctrl->C.contourfile);
	gmt_M_str_free (Ctrl->I.inputfile);
	gmt_freepen (GMT, &Ctrl->W.pen);
	gmt_M_free(GMT, Ctrl);
}

int GMT_mbcontour_usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE)
		return (GMT_NOERROR);
	GMT_Message(API, GMT_TIME_NONE, "usage: mbcontour -I<inputfile> %s [%s]\n", GMT_J_OPT, GMT_B_OPT);
	GMT_Message(API, GMT_TIME_NONE, "\t[-A<factor>/<mode>/<depth>]\n");
	GMT_Message(API, GMT_TIME_NONE, "\t[-b<year>/<month>/<day>/<hour>/<minute>/<second>]\n");
	GMT_Message(API, GMT_TIME_NONE, "\t[-C<cptfile>] [-D<mode>/<ampscale>/<ampmin>/<ampmax>] [-Ei|<dpi>]\n");
	GMT_Message(API, GMT_TIME_NONE, "\t[-e<year>/<month>/<day>/<hour>/<minute>/<second>]\n");
	GMT_Message(API, GMT_TIME_NONE, "\t[-F<format>] [-G<magnitude>/<azimuth | median>]\n");
	GMT_Message(API, GMT_TIME_NONE, "\t[-I<inputfile>] [-L<lonflip>] [-N<cptfile>]\n");
	GMT_Message(API, GMT_TIME_NONE, "\t[-S<speed>] [-T<timegap>] [-W<pen>] [-Z<mode>]\n");
	GMT_Message(API, GMT_TIME_NONE, "\t[%s] [-T] [%s] [%s]\n", GMT_Rgeo_OPT, GMT_U_OPT, GMT_V_OPT);
#if GMT_MAJOR_VERSION >= 6
	GMT_Message(API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s]\n\t[%s] [%s]\n\n", GMT_X_OPT, GMT_Y_OPT, GMT_f_OPT, GMT_n_OPT,
	            GMT_p_OPT, GMT_t_OPT);
#else
	GMT_Message(API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s]\n\t[%s] [%s]\n\n", GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_f_OPT,
	            GMT_n_OPT, GMT_p_OPT, GMT_t_OPT);
#endif

	if (level == GMT_SYNOPSIS)
		return (EXIT_FAILURE);

	GMT_Message(API, GMT_TIME_NONE, "\t<inputfile> is an MB-System datalist referencing the swath data to be plotted.\n");
	GMT_Option(API, "J-");
	GMT_Message(API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option(API, "B-");
	GMT_Message(API, GMT_TIME_NONE, "\t-C Contour file. Defines contour levels, style, and colors.\n");
	GMT_Message(API, GMT_TIME_NONE, "\t   to automatically assign 16 continuous colors over the data range [rainbow].\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-E Set dpi for the projected output Postscript image\n");
	GMT_Message(API, GMT_TIME_NONE, "\t   if -Jx or -Jm is not selected.\n");
	GMT_Message(API, GMT_TIME_NONE, "\t   Give i to do the interpolation in PostScript at device resolution.\n");
	gmt_rgb_syntax(API->GMT, 'G', "Set transparency color for images that otherwise would result in 1-bit images.\n\t  ");
	GMT_Option(API, "K");
	GMT_Option(API, "O,P");
	GMT_Message(API, GMT_TIME_NONE,
	            "\t-p<pings> Sets the ping averaging of the input data [Default = 1, i.e. no ping average].\n");
	GMT_Option(API, "R");
	GMT_Option(API, "U,V,X,.");

	return (EXIT_FAILURE);
}

int GMT_mbcontour_old_W_parser (struct GMTAPI_CTRL *API, struct MBCONTOUR_CTRL *Ctrl, char *text) {
	unsigned int j = 0, n_errors = 0;
	if (text[j] == '-') {Ctrl->W.pen.cptmode = 1; j++;}
	if (text[j] == '+') {Ctrl->W.pen.cptmode = 3; j++;}
	if (text[j] && gmt_getpen (API->GMT, &text[j], &Ctrl->W.pen)) {
#if GMT_MAJOR_VERSION == 6 && GMT_MINOR_VERSION <= 2
		gmt_pen_syntax (API->GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", 15);
#elif GMT_MAJOR_VERSION >= 6
		gmt_pen_syntax (API->GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", NULL, 15);
#else
		gmt_pen_syntax (API->GMT, 'W', "sets pen attributes [Default pen is %s]:", 15);
#endif
		n_errors++;
	}
	return n_errors;
}

int GMT_mbcontour_parse(struct GMT_CTRL *GMT, struct MBCONTOUR_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to mbcontour and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	unsigned int n_files = 0;
	int n;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) { /* Process all the options given */

		switch (opt->option) {
		case '<': /* Input file (only one or three is accepted) */
			Ctrl->I.active = true;
#if GMT_MAJOR_VERSION < 5 || (GMT_MAJOR_VERSION == 5 && GMT_MINOR_VERSION == 1 && GMT_RELEASE_VERSION < 2)
			if (gmt_check_filearg(GMT, '<', opt->arg, GMT_IN)) {
#else
			if (gmt_check_filearg(GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) {
#endif
				Ctrl->I.inputfile = strdup(opt->arg);
				n_files = 1;
			}
			else {
				GMT_Report(API, GMT_MSG_ERROR, "Syntax error: only one input file is allowed.\n");
				n_errors++;
			}
			break;

			/* Processes program-specific parameters */

		case 'A': /* contour controls */
			n = sscanf(opt->arg, "%lf/%lf/%lf/%lf/%lf/%lf/%lf", &(Ctrl->A.cont_int), &(Ctrl->A.col_int), &(Ctrl->A.tick_int),
			           &(Ctrl->A.label_int), &(Ctrl->A.tick_len), &(Ctrl->A.label_hgt), &(Ctrl->A.label_spacing));
			if (n > 0)
				Ctrl->A.active = true;
			else {
				GMT_Report(API, GMT_MSG_ERROR, "Syntax error -A option: \n");
				n_errors++;
			}
			break;
		case 'b': /* btime_i */
			n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d", &(Ctrl->b.time_i[0]), &(Ctrl->b.time_i[1]), &(Ctrl->b.time_i[2]),
			           &(Ctrl->b.time_i[3]), &(Ctrl->b.time_i[4]), &(Ctrl->b.time_i[5]));
			Ctrl->b.time_i[6] = 0;
			if (n == 6)
				Ctrl->b.active = true;
			else {
				GMT_Report(API, GMT_MSG_ERROR, "Syntax error -b option: \n");
				n_errors++;
			}
			break;
		case 'C': /* contour file */
			Ctrl->C.active = true;
			if (Ctrl->C.contourfile)
				free(Ctrl->C.contourfile);
			Ctrl->C.contourfile = strdup(opt->arg);
			break;
		case 'D': /* track annotation */
			n = sscanf(opt->arg, "%lf/%lf/%lf/%lf", &(Ctrl->D.time_tick_int), &(Ctrl->D.time_annot_int),
			           &(Ctrl->D.date_annot_int), &(Ctrl->D.time_tick_len));
			if (n > 0)
				Ctrl->D.active = true;
			else {
				GMT_Report(API, GMT_MSG_ERROR, "Syntax error -D option: \n");
				n_errors++;
			}
			break;
		case 'e': /* etime_i */
			n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d", &(Ctrl->e.time_i[0]), &(Ctrl->e.time_i[1]), &(Ctrl->e.time_i[2]),
			           &(Ctrl->e.time_i[3]), &(Ctrl->e.time_i[4]), &(Ctrl->e.time_i[5]));
			Ctrl->e.time_i[6] = 0;
			if (n == 6)
				Ctrl->e.active = true;
			else {
				GMT_Report(API, GMT_MSG_ERROR, "Syntax error -e option: \n");
				n_errors++;
			}
			break;
		case 'f': /* format */
		case 'F': /* format */
			n = sscanf(opt->arg, "%d", &(Ctrl->F.format));
			if (n == 1)
				Ctrl->F.active = true;
			else {
				GMT_Report(API, GMT_MSG_ERROR, "Syntax error -F option: \n");
				n_errors++;
			}
			break;
		case 'G': /* file annotation */
			n = sscanf(opt->arg, "%lf/%d", &(Ctrl->G.name_hgt), &(Ctrl->G.name_perp));
			if (n == 2)
				Ctrl->G.active = true;
			else if (n == 1) {
				Ctrl->G.active = true;
				Ctrl->G.name_perp = false;
			}
			else {
				GMT_Report(API, GMT_MSG_ERROR, "Syntax error -G option: \n");
				n_errors++;
			}
			break;

		case 'I': /* -I<inputfile> */
			Ctrl->I.active = true;
			if (!gmt_access(GMT, opt->arg, R_OK)) { /* Got a file */
				Ctrl->I.inputfile = strdup(opt->arg);
				n_files = 1;
			}
			else {
				GMT_Report(API, GMT_MSG_ERROR, "Syntax error -I: Requires a valid file\n");
				n_errors++;
			}
			break;
		case 'L': /* -L<lonflip> */
			n = sscanf(opt->arg, "%d", &(Ctrl->L.lonflip));
			if (n == 1)
				Ctrl->L.active = true;
			else {
				GMT_Report(API, GMT_MSG_ERROR, "Syntax error -L option: \n");
				n_errors++;
			}
			break;
		case 'M': /* ping number annotation */
			n = sscanf(opt->arg, "%lf/%lf/%lf", &(Ctrl->M.pingnumber_tick_int), &(Ctrl->M.pingnumber_annot_int),
			           &(Ctrl->M.pingnumber_tick_len));
			if (n > 0)
				Ctrl->M.active = true;
			else {
				GMT_Report(API, GMT_MSG_ERROR, "Syntax error -M option: \n");
				n_errors++;
			}
			break;
		case 'N': /* nplot */
			n = sscanf(opt->arg, "%d", &(Ctrl->N.nplot));
			if (n > 0)
				Ctrl->N.active = true;
			else {
				GMT_Report(API, GMT_MSG_ERROR, "Syntax error -N option: \n");
				n_errors++;
			}
			break;
		case 'p': /* Sets the ping averaging */
			Ctrl->p.active = true;
			Ctrl->p.pings = (int)strtol(opt->arg, NULL, 10);
			if (Ctrl->p.pings < 0) {
				GMT_Report(API, GMT_MSG_ERROR, "Error -p option: Don't invent, number of pings must be >= 0\n");
				Ctrl->p.pings = 1;
			}
			break;
		case 'Q': /* plot triangles */
			Ctrl->Q.active = true;
			break;
		case 'S': /* -S<speed> */
			n = sscanf(opt->arg, "%lf", &(Ctrl->S.speedmin));
			if (n == 1)
				Ctrl->S.active = true;
			else {
				GMT_Report(API, GMT_MSG_ERROR, "Syntax error -S option: \n");
				n_errors++;
			}
			break;
		case 'T': /* -T<timegap> */
			n = sscanf(opt->arg, "%lf", &(Ctrl->T.timegap));
			if (n == 1)
				Ctrl->T.active = true;
			else {
				GMT_Report(API, GMT_MSG_ERROR, "Syntax error -T option: \n");
				n_errors++;
			}
			break;
		case 'W':		/* Set line attributes */
                {
#if GMT_MAJOR_VERSION >= 6 && GMT_MINOR_VERSION > 2
                    n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
#endif
                    char *c = NULL;
                    if ((c = strstr (opt->arg, "+z"))) {
                        Ctrl->W.set_color = true;
                        c[0] = '\0';	/* Chop off this modifier */
                    }
                    if (opt->arg[0] == '-' || (opt->arg[0] == '+' && opt->arg[1] != 'c')) {	/* Definitively old-style args */
                        if (gmt_M_compat_check (API->GMT, 5)) {	/* Sorry */
                            GMT_Report (API, GMT_MSG_ERROR, "Your -W syntax is obsolete; see program usage.\n");
                            n_errors++;
                        }
                        else {
                            GMT_Report (API, GMT_MSG_ERROR, "Your -W syntax is obsolete; see program usage.\n");
                            n_errors += GMT_mbcontour_old_W_parser (API, Ctrl, opt->arg);
                        }
                    }
                    else if (opt->arg[0]) {
                        if (gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
#if GMT_MAJOR_VERSION == 6 && GMT_MINOR_VERSION <= 2
                            gmt_pen_syntax (GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", 11);
#elif GMT_MAJOR_VERSION >= 6
                            gmt_pen_syntax (GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", NULL, 11);
#else
                            gmt_pen_syntax (API->GMT, 'W', "sets pen attributes [Default pen is %s]:", 11);
#endif
                            n_errors++;
                        }
                    }
                    if (Ctrl->W.pen.cptmode) Ctrl->W.cpt_effect = true;
                    if (c) c[0] = '+';	/* Restore */
                    if (Ctrl->W.pen.rgb[0] < -4.0) Ctrl->W.sequential = irint (Ctrl->W.pen.rgb[0] + 7.0);
                }
			break;

		case 'Z': /* contour algorithm */
			n = sscanf(opt->arg, "%d", &(Ctrl->Z.contour_algorithm));
			if (n == 1)
				Ctrl->Z.active = true;
			else {
				GMT_Report(API, GMT_MSG_ERROR, "Syntax error -Z option: \n");
				n_errors++;
			}
			break;
		default: /* Report bad options */
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

#if GMT_MAJOR_VERSION == 5 && GMT_MINOR_VERSION <= 3
	n_errors += gmt_M_check_condition(GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
#else
	n_errors += gmt_M_check_condition(GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
#endif
	n_errors +=
	    gmt_M_check_condition(GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += gmt_M_check_condition(GMT, n_files != 1, "Syntax error: Must specify one input file(s)\n");
	n_errors +=
	    gmt_M_check_condition(GMT, Ctrl->I.active && !Ctrl->I.inputfile, "Syntax error -I option: Must specify input file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code)                                                                                                            \
	{                                                                                                                            \
		gmt_M_free_options(mode);                                                                                                \
		return (code);                                                                                                           \
	}
#define Return(code)                                                                                                             \
	{                                                                                                                            \
		Free_mbcontour_Ctrl(GMT, Ctrl);                                                                                          \
		gmt_end_module(GMT, GMT_cpy);                                                                                            \
		bailout(code);                                                                                                           \
	}

/*--------------------------------------------------------------------------*/
int mbcontour_ping_copy(int verbose, int one, int two, struct swath *swath, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBCONTOUR function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       one:        %d\n", one);
		fprintf(stderr, "dbg2       two:        %d\n", two);
		fprintf(stderr, "dbg2       swath:      %p\n", swath);
		fprintf(stderr, "dbg2       pings:      %d\n", swath->npings);
		fprintf(stderr, "dbg2       time_i[two]:%4d  %4d %2d %2d %2d %2d %2d %6.6d\n", two, swath->pings[two].time_i[0],
		        swath->pings[two].time_i[1], swath->pings[two].time_i[2], swath->pings[two].time_i[3],
		        swath->pings[two].time_i[4], swath->pings[two].time_i[5], swath->pings[two].time_i[6]);
	}

	struct ping *ping1 = &swath->pings[one];
	struct ping *ping2 = &swath->pings[two];

	int status = MB_SUCCESS;

	/* make sure enough memory is allocated */
	if (ping1->beams_bath_alloc < ping2->beams_bath) {
		status = mb_reallocd(verbose, __FILE__, __LINE__, ping2->beams_bath * sizeof(char), (void **)&(ping1->beamflag), error);
		status = mb_reallocd(verbose, __FILE__, __LINE__, ping2->beams_bath * sizeof(double), (void **)&(ping1->bath), error);
		status = mb_reallocd(verbose, __FILE__, __LINE__, ping2->beams_bath * sizeof(double), (void **)&(ping1->bathlon), error);
		status = mb_reallocd(verbose, __FILE__, __LINE__, ping2->beams_bath * sizeof(double), (void **)&(ping1->bathlat), error);
		status = mb_reallocd(verbose, __FILE__, __LINE__, ping2->beams_bath * sizeof(int), (void **)&(ping1->bflag[0]), error);
		status = mb_reallocd(verbose, __FILE__, __LINE__, ping2->beams_bath * sizeof(int), (void **)&(ping1->bflag[1]), error);
		ping1->beams_bath_alloc = ping2->beams_bath;
	}

	/* copy things */
	for (int i = 0; i < 7; i++)
		ping1->time_i[i] = ping2->time_i[i];
	ping1->time_d = ping2->time_d;
	ping1->navlon = ping2->navlon;
	ping1->navlat = ping2->navlat;
	ping1->heading = ping2->heading;
	ping1->pingnumber = ping2->pingnumber;
	ping1->beams_bath = ping2->beams_bath;
	for (int i = 0; i < ping2->beams_bath; i++) {
		ping1->beamflag[i] = ping2->beamflag[i];
		ping1->bath[i] = ping2->bath[i];
		ping1->bathlon[i] = ping2->bathlon[i];
		ping1->bathlat[i] = ping2->bathlat[i];
		ping1->bflag[0][i] = ping2->bflag[1][i];
	}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBCONTOUR function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------------*/
void mbcontour_plot(double x, double y, int ipen) {
	/* make sure contour arrays are large enough */
	if (ncontour_plot >= ncontour_plot_alloc) {
		ncontour_plot_alloc += MBCONTOUR_PLOT_ALLOC_INC;
		double *p = (double *)realloc(contour_x, sizeof(double) * (ncontour_plot_alloc));
		if (p != NULL) {
			contour_x = p;
		}
		else {
			free(contour_x);
			contour_x = NULL;
			ncontour_plot_alloc = 0;
		}
		if ((p = (double *)realloc(contour_y, sizeof(double) * (ncontour_plot_alloc))) != NULL) {
			contour_y = p;
		}
		else {
			free(contour_y);
			contour_y = NULL;
			ncontour_plot_alloc = 0;
		}
	}

	/* convert to map units */
	double xx;
	double yy;
	gmt_geo_to_xy(GMT, x, y, &xx, &yy);

	/* if command is move then start new contour */
	if (ipen == MBCONTOUR_PLOT_MOVE) {
		ncontour_plot = 0;
		contour_x[ncontour_plot] = xx;
		contour_y[ncontour_plot] = yy;
		ncontour_plot++;
	}

	/* else if command is to draw then add the point to the list */
	else if (ipen == MBCONTOUR_PLOT_DRAW) {
		contour_x[ncontour_plot] = xx;
		contour_y[ncontour_plot] = yy;
		ncontour_plot++;
	}

	/* else if command is to areokw then add the point to the list
	    and call PSL_plotline() */
	else if (ipen == MBCONTOUR_PLOT_STROKE) {
		contour_x[ncontour_plot] = xx;
		contour_y[ncontour_plot] = yy;
		ncontour_plot++;

		PSL_plotline(PSL, contour_x, contour_y, ncontour_plot, PSL_MOVE + PSL_STROKE);

		ncontour_plot = 0;
	}
}

/*--------------------------------------------------------------------------*/
void mbcontour_setline(int linewidth) {
	PSL_setlinewidth(PSL, (double)linewidth);
}

/*--------------------------------------------------------------------------*/
void mbcontour_newpen(int ipen) {
	if (ipen > -1 && ipen < ncolor) {
		double rgb[4] = {
		    red[ipen] / 255.0, green[ipen] / 255.0, blue[ipen] / 255.0,
		    0, // To not fall into the transparency case of pslib.c/psl_putcolor()
		};
		PSL_setcolor(PSL, rgb, PSL_IS_STROKE);
	}
}

/*--------------------------------------------------------------------------*/
void mbcontour_justify_string(double height, char *string, double *s) {
	const int len = strlen(string);
	s[0] = 0.0;
	s[1] = 0.185 * height * len;
	s[2] = 0.37 * len * height;
	s[3] = 0.37 * len * height;
}

/*--------------------------------------------------------------------------*/
void mbcontour_plot_string(double x, double y, double hgt, double angle, char *label) {
	double xx;
	double yy;
	const double fontsize = 72.0 * hgt / inchtolon;
	gmt_geo_to_xy(GMT, x, y, &xx, &yy);
	const int justify = 5;
	const int mode = 0;
	PSL_plottext(PSL, xx, yy, fontsize, label, angle, justify, mode);
}

/*--------------------------------------------------------------------------*/
void mb_set_colors(int ncolor, int *red, int *green, int *blue) {
	(void)ncolor; // Unused parameter
	(void)red;    // Unused parameter
	(void)green;  // Unused parameter
	(void)blue;   // Unused parameter
}

/*--------------------------------------------------------------------------*/
int GMT_mbcontour(void *V_API, int mode, void *args) {
	/*----------------------- Standard module initialization and parsing ----------------------*/
	struct GMT_CTRL *GMT_cpy = NULL; /* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr(V_API); /* Cast from void to GMTAPI_CTRL pointer */
	struct MBCONTOUR_CTRL *Ctrl = NULL;
	int error = MB_ERROR_NO_ERROR;

	if (API == NULL)
		return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE)
		return (GMT_mbcontour_usage(API, GMT_MODULE_PURPOSE)); /* Return the purpose of program */
	options = GMT_Create_Options(API, mode, args);
	if (API->error)
		return (API->error); /* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE)
		bailout(GMT_mbcontour_usage(API, GMT_USAGE)); /* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS)
		bailout(GMT_mbcontour_usage(API, GMT_SYNOPSIS)); /* Return the synopsis */

		/* Parse the command-line arguments */
#if GMT_MAJOR_VERSION >= 6 && GMT_MINOR_VERSION >= 1
	GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy); /* Save current state */
#elif GMT_MAJOR_VERSION >= 6
	GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy); /* Save current state */
#else
	GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, &GMT_cpy); /* Save current state */
#endif
	if (GMT_Parse_Common(API, GMT_PROG_OPTIONS, options)) {
		fprintf(stderr, "Error from GMT_Parse_common():%d\n", API->error);
		Return(API->error);
	}

	Ctrl = (struct MBCONTOUR_CTRL *)New_mbcontour_Ctrl(GMT); /* Allocate and initialize a new control structure */
	if ((error = GMT_mbcontour_parse(GMT, Ctrl, options))) {
		fprintf(stderr, "Error from GMT_mbcontour_parse():%d\n", error);
		Return(error);
	}

	/*-------------------------------- Variable initialization --------------------------------*/

	/* get current mb default values */
	int verbose = 0;
	int format;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	if (Ctrl->p.active)
		pings = Ctrl->p.pings; /* If pings were set by user, prefer it */

	bool plot_contours = false;

	double cont_int = 25.;
	double col_int = 100.;
	double tick_int = 100.;
	double label_int = 100.;
	double tick_len = 0.05;
	double label_hgt = 0.1;
	double label_spacing = 0.0;

	/* set modes */
	if (Ctrl->A.active) {
		plot_contours = true;
		cont_int = Ctrl->A.cont_int;
		col_int = Ctrl->A.col_int;
		tick_int = Ctrl->A.tick_int;
		label_int = Ctrl->A.label_int;
		tick_len = Ctrl->A.tick_len;
		label_hgt = Ctrl->A.label_hgt;
		label_spacing = Ctrl->A.label_spacing;
	}
	if (Ctrl->b.active) {
		for (int i = 0; i < 7; i++)
			btime_i[i] = Ctrl->b.time_i[i];
	}

	bool set_contours = false;
	char contourfile[MB_PATH_MAXLINE];
	if (Ctrl->C.active) {
		plot_contours = true;
		set_contours = true;
		strcpy(contourfile, Ctrl->C.contourfile);
	}
	bool plot_track = false;
	double time_tick_int = 0.25;
	double time_annot_int = 1.0;
	double date_annot_int = 4.0;
	double time_tick_len = 0.1;
	if (Ctrl->D.active) {
		plot_track = true;
		time_tick_int = Ctrl->D.time_tick_int;
		time_annot_int = Ctrl->D.time_annot_int;
		date_annot_int = Ctrl->D.date_annot_int;
		time_tick_len = Ctrl->D.time_tick_len;
	}
	if (Ctrl->e.active) {
		for (int i = 0; i < 7; i++)
			etime_i[i] = Ctrl->e.time_i[i];
	}
	if (Ctrl->F.active)
		format = Ctrl->F.format;
	bool plot_name = false;
	double name_hgt = 0.1;
	bool name_perp = false;
	if (Ctrl->G.active) {
		plot_name = true;
		name_hgt = Ctrl->G.name_hgt;
		name_perp = Ctrl->G.name_perp;
	}
	mb_path read_file = "datalist.mb-1";
	if (Ctrl->I.active) {
		strcpy(read_file, Ctrl->I.inputfile);
	}
	bool lonflip_set = false;
	if (Ctrl->L.active) {
		lonflip_set = true;
		lonflip = Ctrl->L.lonflip;
	}
	bool plot_pingnumber = false;
	int pingnumber_tick_int = 50;
	int pingnumber_annot_int = 100;
	double pingnumber_tick_len = 0.1;
	if (Ctrl->M.active) {
		plot_pingnumber = true;
		pingnumber_tick_int = (int)Ctrl->M.pingnumber_tick_int;
		pingnumber_annot_int = (int)Ctrl->M.pingnumber_annot_int;
		pingnumber_tick_len = Ctrl->M.pingnumber_tick_len;
	}
	int nplot = 0;
	if (Ctrl->N.active)
		nplot = Ctrl->N.nplot;
	bool plot_triangles = false;
	if (Ctrl->Q.active)
		plot_triangles = true;
	if (Ctrl->S.active)
		speedmin = Ctrl->S.speedmin;
	if (Ctrl->T.active)
		timegap = Ctrl->T.timegap;
	bool bathy_in_feet = false;
	int contour_algorithm = MB_CONTOUR_OLD;
	if (Ctrl->Z.active)
		contour_algorithm = Ctrl->Z.contour_algorithm;

	/* set verbosity */
	verbose = GMT->common.V.active;

	/* set number of pings to be plotted if not set */
	if (nplot == 0 && contour_algorithm == MB_CONTOUR_TRIANGLES)
		nplot = 5;
	else if (nplot == 0)
		nplot = 50;

	/* if nothing set to be plotted, plot contours and track */
	if (!plot_contours && !plot_triangles && !plot_track && !plot_pingnumber) {
		plot_contours = true;
		plot_track = true;
	}
	if (plot_name && !plot_track && !plot_pingnumber) {
		plot_track = true;
	}
	if (!plot_track && plot_pingnumber) {
		plot_track = true;
		time_tick_int = 10000000.0;
		time_annot_int = 10000000.0;
		date_annot_int = 10000000.0;
	}

	static const char program_name[] = "mbcontour";

	if (verbose == 1) {
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
	}

	/*---------------------------- This is the mbcontour main code ----------------------------*/

	int nlevel = 0;

	/* read contours from file */
	if (set_contours) {
		/* open contour file */
		FILE *fp = fopen(contourfile, "r");
		if (fp == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open contour file: %s\n", contourfile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* count lines in file */
		nlevel = 0;
		mb_path line;
		while (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
			nlevel++;
		fclose(fp);

		/* set number of colors equal to levels */
		ncolor = nlevel;

		/* allocate memory */
		status = mb_mallocd(verbose, __FILE__, __LINE__, nlevel * sizeof(double), (void **)&level, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nlevel * sizeof(int), (void **)&label, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nlevel * sizeof(int), (void **)&tick, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, ncolor * sizeof(int), (void **)&red, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, ncolor * sizeof(int), (void **)&green, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, ncolor * sizeof(int), (void **)&blue, &error);

		/* reopen contour file */
		if ((fp = fopen(contourfile, "r")) == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open contour file: %s\n", contourfile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* read contour levels from file */
		nlevel = 0;
		while (fgets(line, MB_PATH_MAXLINE, fp) != NULL) {
			mb_path labelstr;
			mb_path tickstr;
			const int count = sscanf(line, "%lf %s %s %d %d %d", &level[nlevel], labelstr, tickstr, &red[nlevel], &green[nlevel],
			                         &blue[nlevel]);
			bool setcolors = true;
			if (count >= 2 && labelstr[0] == 'a')
				label[nlevel] = 1;
			else if (count >= 2 && labelstr[0] == 'n')
				label[nlevel] = 0;
			else {
				label[nlevel] = 0;
				setcolors = false;
			}
			if (count >= 3 && tickstr[0] == 't')
				tick[nlevel] = 1;
			else if (count >= 3 && tickstr[0] == 'n')
				tick[nlevel] = 0;
			else {
				tick[nlevel] = 0;
				setcolors = false;
			}
			if (count < 6 || !setcolors) {
				red[nlevel] = 0;
				green[nlevel] = 0;
				blue[nlevel] = 0;
			}
			if (count > 0)
				nlevel++;
		}
		fclose(fp);
	}

	/* else set default colors and use contour intervals */
	else {
		/* set defaults */
		nlevel = 0;
		ncolor = 4;

		/* allocate memory */
		status = mb_mallocd(verbose, __FILE__, __LINE__, ncolor * sizeof(int), (void **)&red, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, ncolor * sizeof(int), (void **)&green, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, ncolor * sizeof(int), (void **)&blue, &error);

		/* set colors */
		red[0] = 0;
		green[0] = 0;
		blue[0] = 0; /* black */
		red[1] = 255;
		green[1] = 0;
		blue[1] = 0; /* red */
		red[2] = 0;
		green[2] = 200;
		blue[2] = 0; /* green */
		red[3] = 0;
		green[3] = 0;
		blue[3] = 255; /* blue */
	}

	/* set colors */
	mb_set_colors(ncolor, red, green, blue);

	/* set bounds for data reading larger than map borders */
	bounds[0] = GMT->common.R.wesn[0] - 0.25 * (GMT->common.R.wesn[1] - GMT->common.R.wesn[0]);
	bounds[1] = GMT->common.R.wesn[1] + 0.25 * (GMT->common.R.wesn[1] - GMT->common.R.wesn[0]);
	bounds[2] = GMT->common.R.wesn[2] - 0.25 * (GMT->common.R.wesn[3] - GMT->common.R.wesn[2]);
	bounds[3] = GMT->common.R.wesn[3] + 0.25 * (GMT->common.R.wesn[3] - GMT->common.R.wesn[2]);

	//	/* get scaling from degrees to km */
	//	mb_coor_scale(verbose, 0.5*(bounds[2] + bounds[3]), &mtodeglon, &mtodeglat);

	/* set lonflip if possible */
	if (!lonflip_set) {
		if (bounds[0] < -180.0)
			lonflip = -1;
		else if (bounds[1] > 180.0)
			lonflip = 1;
		else if (lonflip == -1 && bounds[1] > 0.0)
			lonflip = 0;
		else if (lonflip == 1 && bounds[0] < 0.0)
			lonflip = 0;
	}

	/* Start the postscript plot */
	gmt_M_err_fail(GMT, gmt_map_setup(GMT, GMT->common.R.wesn), "");
	PSL = gmt_plotinit(GMT, options);
	gmt_plane_perspective(GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_plotcanvas(GMT); /* Fill canvas if requested */
	gmt_map_clip_on(GMT, GMT->session.no_rgb, 3);


	/* Set particulars of output image for the postscript plot */
	double clipx[4];
	double clipy[4];
	gmt_geo_to_xy(GMT, GMT->common.R.wesn[0], GMT->common.R.wesn[2], &clipx[0], &clipy[0]);
	gmt_geo_to_xy(GMT, GMT->common.R.wesn[1], GMT->common.R.wesn[2], &clipx[1], &clipy[1]);
	gmt_geo_to_xy(GMT, GMT->common.R.wesn[1], GMT->common.R.wesn[3], &clipx[2], &clipy[2]);
	gmt_geo_to_xy(GMT, GMT->common.R.wesn[0], GMT->common.R.wesn[3], &clipx[3], &clipy[3]);
	//	x_inch = clipx[1] - clipx[0];
	//	y_inch = clipy[2] - clipy[1];
	//	x0 = GMT->common.R.wesn[0];
	//	y0 = GMT->common.R.wesn[2];
	inchtolon = (GMT->common.R.wesn[1] - GMT->common.R.wesn[0]) / (clipx[1] - clipx[0]);

	/* scale label and tick sizes */
	const double label_hgt_map = inchtolon * label_hgt;
	const double label_spacing_map = inchtolon * label_spacing;
	const double tick_len_map = inchtolon * tick_len;
	const double time_tick_len_map = inchtolon * time_tick_len;
	const double name_hgt_map = inchtolon * name_hgt;
	const double pingnumber_tick_len_map = inchtolon * pingnumber_tick_len;

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, read_file, NULL, &format, &error);

	/* determine whether to read one file or a list of files */
	const bool read_datalist = format < 0;

	/* open file list */
	int nping_read = 0;
	bool read_data = false;
	void *datalist;
	double file_weight;
	mb_path file;
	mb_path dfile;

	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if ((status = mb_datalist_open(verbose, &datalist, read_file, look_processed, &error)) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}
		if ((status = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error)) == MB_SUCCESS)
			read_data = true;
		else
			read_data = false;
	}
	else {
		strcpy(file, read_file);
		read_data = true;
	}

	// TODO(schwehr): Localize variables

	struct swath *swath_plot = NULL;
	struct ping *pingcur = NULL;
	char *beamflag = NULL;
	double *bath = NULL;
	double *bathlon = NULL;
	double *bathlat = NULL;
	double *amp = NULL;
	double *ss = NULL;
	double *sslon = NULL;
	double *sslat = NULL;
	unsigned int pingnumber;

	/* loop over files in file list */
	if (verbose == 1)
		fprintf(stderr, "\n");

	while (read_data) {
		/* check for mbinfo file - get file bounds if possible */
		bool file_in_bounds = false;
		status = mb_check_info(verbose, file, lonflip, bounds, &file_in_bounds, &error);
		if (status == MB_FAILURE) {
			file_in_bounds = true;
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
		}

		/* read if data may be in bounds */
		if (file_in_bounds) {
			/* check for "fast bathymetry" or "fbt" file */
			if (plot_contours) {
				mb_get_fbt(verbose, file, &format, &error);
			}

			/* else check for "fast nav" or "fnv" file */
			else if (plot_track || plot_pingnumber) {
				mb_get_fnv(verbose, file, &format, &error);
			}

			double btime_d;
			double etime_d;
			int beams_bath;
			int beams_amp;
			int pixels_ss;
			void *mbio_ptr = NULL;
			if ((status = mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
			                           &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) !=
			    MB_SUCCESS) {
				char *message = NULL;
				mb_error(verbose, error, &message);
				fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
				fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
				fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
				exit(error);
			}

			/* allocate memory for data arrays */
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

			/* if error initializing memory then quit */
			if (error != MB_ERROR_NO_ERROR) {
				char *message = NULL;
				mb_error(verbose, error, &message);
				fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
				fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
				exit(error);
			}

			/* initialize contour controls */
			status = mb_contour_init(verbose, &swath_plot, nplot, beams_bath, contour_algorithm, plot_contours, plot_triangles,
			                         plot_track, plot_name, plot_pingnumber, cont_int, col_int, tick_int, label_int, tick_len_map,
			                         label_hgt_map, label_spacing_map, ncolor, nlevel, level, label, tick, time_tick_int,
			                         time_annot_int, date_annot_int, time_tick_len_map, name_hgt_map, pingnumber_tick_int,
			                         pingnumber_annot_int, pingnumber_tick_len_map, &mbcontour_plot, &mbcontour_newpen,
			                         &mbcontour_setline, &mbcontour_justify_string, &mbcontour_plot_string, &error);
			swath_plot->beams_bath = beams_bath;

			/* if error initializing memory then quit */
			if (error != MB_ERROR_NO_ERROR) {
				char *message = NULL;
				mb_error(verbose, error, &message);
				fprintf(stderr, "\nMBIO Error allocating contour control structure:\n%s\n", message);
				fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
				exit(error);
			}

			/* print message */
			if (verbose >= 2)
				fprintf(stderr, "\n");
			if (verbose >= 1)
				fprintf(stderr, "processing data in %s...\n", file);

			/* loop over reading */
			int *npings = &swath_plot->npings;
			*npings = 0;
			bool done = false;
			bool plotted_name = false;
			while (!done) {
				mb_path comment;
				int kind;
				int pings_read;
				int time_i[7];
				double time_d;
				double navlon;
				double navlat;
				double speed;
				double heading;
				double distance;
				double altitude;
				double sensordepth;

				/* read the next ping */
				status = mb_read(verbose, mbio_ptr, &kind, &pings_read, time_i, &time_d, &navlon, &navlat, &speed, &heading,
				                 &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
				                 bathlon, bathlat, ss, sslon, sslat, comment, &error);

				/* get pingnumber */
				if (status == MB_SUCCESS) {
					status = mb_pingnumber(verbose, mbio_ptr, &pingnumber, &error);
				}

				/* copy data to swath_plot */
				if (status == MB_SUCCESS || error == MB_ERROR_TIME_GAP) {
					pingcur = &swath_plot->pings[*npings];

					/* make sure enough memory is allocated */
					if (pingcur->beams_bath_alloc < beams_bath) {
						status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(char),
						                     (void **)&(pingcur->beamflag), &error);
						status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&(pingcur->bath),
						                     &error);
						status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
						                     (void **)&(pingcur->bathlon), &error);
						status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
						                     (void **)&(pingcur->bathlat), &error);
						status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(int), (void **)&(pingcur->bflag[0]),
						                     &error);
						status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(int), (void **)&(pingcur->bflag[1]),
						                     &error);
						pingcur->beams_bath_alloc = beams_bath;
					}

					/* insert the data */
					for (int i = 0; i < 7; i++)
						pingcur->time_i[i] = time_i[i];
					pingcur->time_d = time_d;
					pingcur->navlon = navlon;
					pingcur->navlat = navlat;
					pingcur->heading = heading;
					pingcur->sensordepth = sensordepth;
					pingcur->beams_bath = beams_bath;
					pingcur->pingnumber = pingnumber;
					for (int i = 0; i < beams_bath; i++) {
						pingcur->beamflag[i] = beamflag[i];
						pingcur->bath[i] = bath[i];
						pingcur->bathlon[i] = bathlon[i];
						pingcur->bathlat[i] = bathlat[i];
						pingcur->bflag[0][i] = 0;
						pingcur->bflag[1][i] = 0;
					}
				}

				/* null out any unused beams for formats with
				variable numbers of beams */
				for (int i = beams_bath; i < swath_plot->beams_bath; i++)
					beamflag[i] = MB_FLAG_NULL;

				/* print debug statements */
				if (verbose >= 2) {
					fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
					fprintf(stderr, "dbg2       kind:           %d\n", kind);
					fprintf(stderr, "dbg2       npings:         %d\n", *npings);
					fprintf(stderr, "dbg2       time:           %4d %2d %2d %2d %2d %2d %6.6d\n", time_i[0], time_i[1], time_i[2],
					        time_i[3], time_i[4], time_i[5], time_i[6]);
					fprintf(stderr, "dbg2       navigation:     %f  %f\n", navlon, navlat);
					fprintf(stderr, "dbg2       beams_bath:     %d\n", beams_bath);
					fprintf(stderr, "dbg2       beams_amp:      %d\n", beams_amp);
					fprintf(stderr, "dbg2       pixels_ss:      %d\n", pixels_ss);
					fprintf(stderr, "dbg2       error:          %d\n", error);
					fprintf(stderr, "dbg2       status:         %d\n", status);
				}

				/* scale bathymetry if necessary */
				if (error == MB_ERROR_NO_ERROR && bathy_in_feet) {
					for (int i = 0; i < beams_bath; i++) {
						bath[i] = 3.2808399 * bath[i];
					}
				}

				/* update bookkeeping */
				if (error == MB_ERROR_NO_ERROR) {
					nping_read += pings_read;
					(*npings)++;
				}

				/* decide whether to plot, whether to
				    save the new ping, and if done */
				bool plot = false;
				bool flush = false;
				if (*npings >= nplot)
					plot = true;
				if (*npings > 0 && (error > MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP || error == MB_ERROR_OUT_BOUNDS ||
				                    error == MB_ERROR_OUT_TIME || error == MB_ERROR_SPEED_TOO_SMALL)) {
					plot = true;
					flush = true;
				}
				bool save_new = false;
				if (error == MB_ERROR_TIME_GAP)
					save_new = true;
				if (error > MB_ERROR_NO_ERROR)
					done = true;

				/* if enough pings read in, plot them */
				if (plot) {

					/* print debug statements */
					if (verbose >= 2) {
						fprintf(stderr, "\ndbg2  Plotting %d pings in program <%s>\n", *npings, program_name);
						for (int i = 0; i < *npings; i++) {
							pingcur = &swath_plot->pings[i];
							fprintf(stderr, "dbg2       %4d  %4d %2d %2d %2d %2d %2d %6.6d\n", i, pingcur->time_i[0],
							        pingcur->time_i[1], pingcur->time_i[2], pingcur->time_i[3], pingcur->time_i[4],
							        pingcur->time_i[5], pingcur->time_i[6]);
						}
					}

					/* plot data */
					if (plot_contours || plot_triangles)
						mb_contour(verbose, swath_plot, &error);

					/* plot nav track */
					if (plot_track)
						mb_track(verbose, swath_plot, &error);

					/* annotate pingnumber */
					if (plot_pingnumber) {
						mb_trackpingnumber(verbose, swath_plot, &error);
					}

					if (plot_name && !plotted_name) {
						mb_trackname(verbose, name_perp, swath_plot, file, &error);
						plotted_name = true;
					}

					/* reorganize data */
					if (flush && save_new) {
						status = mbcontour_ping_copy(verbose, 0, *npings, swath_plot, &error);
						*npings = 1;
					}
					else if (flush) {
						*npings = 0;
					}
					else if (*npings > 1) {
						status = mbcontour_ping_copy(verbose, 0, *npings - 1, swath_plot, &error);
						*npings = 1;
					}
				}
			}
			status = mb_close(verbose, &mbio_ptr, &error);

			/* deallocate memory for data arrays */
			status = mb_contour_deall(verbose, swath_plot, &error);
		} /* end if file in bounds */

		/* figure out whether and what to read next */
		if (read_datalist) {
			if ((status = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error)) == MB_SUCCESS)
				read_data = true;
			else
				read_data = false;
		}
		else {
			read_data = false;
		}

		/* end loop over files in list */
	}
	if (read_datalist)
		mb_datalist_close(verbose, &datalist, &error);

	gmt_map_clip_off(GMT);

	gmt_map_basemap(GMT);
	gmt_plane_perspective(GMT, -1, 0.0);
	gmt_plotend(GMT);

	/* deallocate memory for data arrays */
	mb_freed(verbose, __FILE__, __LINE__, (void **)&level, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&label, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&tick, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&red, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&green, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&blue, &error);

	/* print ending info */
	if (verbose >= 1)
		fprintf(stderr, "\n%d pings read and plotted\n", nping_read);

	/* check memory */
	if (verbose >= 2)
		status = mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	struct GMT_PALETTE *CPTcolor = NULL;
	if (!Ctrl->C.active && GMT_Destroy_Data(API, &CPTcolor) != GMT_OK) {
		Return(API->error);
	}
	Return(EXIT_SUCCESS);
}
/*--------------------------------------------------------------------------*/
