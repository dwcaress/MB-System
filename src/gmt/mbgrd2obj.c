/*--------------------------------------------------------------------
 *    The MB-system:	mbgrd2obj.c	5/12/2020
 *
 *    Copyright (c) 2020-2023 by
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
 * mbgrd2obj is a GMT module which reads a topography grid file and
 * generates an OBJ format 3D module file representing the topography.
 * The OBJ format is an open format used by visualization software and
 * 3D printers.
 *   https://en.wikipedia.org/wiki/Wavefront_.obj_file
 *
 * Most of the code here derives from the GMT version 6.0.0
 * source files grdinfo.c, grd2xyz.c, and grd2kml.c.
 *
 * Author:	D. W. Caress
 * Date:	May 12, 2020
 *
 */
/*--------------------------------------------------------------------*/

#include "mb_define.h"

// include gmt_dev.h but first undefine PACKAGE variables to prevent
// warnings about name collision between GDAL's cpl_port.h and the 
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
#define gmt_M_is_nonlinear_graticule GMT_IS_NONLINEAR_GRATICULE
#define gmt_M_memcpy GMT_memcpy
#define gmt_M_rgb_copy GMT_rgb_copy
#define gmt_M_u255 GMT_u255
#define gmt_M_err_fail GMT_err_fail
#define gmt_M_free GMT_free
#define gmt_M_is_fnan GMT_is_fnan
#define gmt_M_memory GMT_memory
#define gmt_M_yiq GMT_YIQ
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
#elif GMT_MAJOR_VERSION == 6
#define gmt_M_grd_is_global gmt_grd_is_global
#endif

#define THIS_MODULE_CLASSIC_NAME "mbgrd2obj"
#define THIS_MODULE_MODERN_NAME "mbgrd2obj"
#define THIS_MODULE_LIB "mbsystem"
#define THIS_MODULE_PURPOSE "Convert grid to OBJ format 3D model file"
#define THIS_MODULE_KEYS "<G{+,>}"
#define THIS_MODULE_NEEDS "g"
#define THIS_MODULE_OPTIONS "-:>RV" GMT_OPT("H")

struct MBGRD2OBJ_CTRL {
	struct MBGRD2OBJ_In { /* <gridfile> */
		bool active;
		char *file;
	} In;
	struct MBGRD2OBJ_G { /* -G<objfile> */
		bool active;
		char *file;
	} G;
};

GMT_LOCAL void *New_Ctrl(struct GMT_CTRL *GMT) { /* Allocate and initialize a new control structure */
	struct MBGRD2OBJ_CTRL *C;

	C = gmt_M_memory(GMT, NULL, 1, struct MBGRD2OBJ_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl(struct GMT_CTRL *GMT, struct MBGRD2OBJ_CTRL *C) { /* Deallocate control structure */
	if (!C)
		return;
	gmt_M_str_free(C->In.file);
	gmt_M_str_free(C->G.file);
	gmt_M_free(GMT, C);
}

GMT_LOCAL int GMT_mbgrd2obj_usage(struct GMTAPI_CTRL *API, int level) {
#if GMT_MAJOR_VERSION < 6
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE)
		return (GMT_NOERROR);
	GMT_Message(API, GMT_TIME_NONE, "usage: mbgrd2obj <gridfile>  -G<objfile> [%s] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT);
#else
	const char *name = gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE)
		return (GMT_NOERROR);
	GMT_Message(API, GMT_TIME_NONE, "usage: %s <gridfile>  -G<objfile> [%s] [%s]\n", name, GMT_Rgeo_OPT, GMT_V_OPT);
	GMT_Message(API, GMT_TIME_NONE, "\t[%s]\n\n", GMT_PAR_OPT);
#endif

	if (level == GMT_SYNOPSIS)
		return (GMT_MODULE_SYNOPSIS);

	GMT_Message(API, GMT_TIME_NONE, "\n\t<gridfile> is the input topography grid file.\n");
	GMT_Message(API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-G<objfile> Output topography OBJ format file. \n");
	GMT_Message(API, GMT_TIME_NONE, "\t   Default is to add \".obj\" suffix to the input grid\n");
	GMT_Message(API, GMT_TIME_NONE, "\t   file name (replacing \".grd\" suffix if possible).\n");
	GMT_Option(API, "R,V");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse(struct GMT_CTRL *GMT, struct MBGRD2OBJ_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	// struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) { /* Process all the options given */

		switch (opt->option) {
		case '<': /* Input file - there can only be one */
			if (gmt_check_filearg(GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID) && n_files == 0) {
				Ctrl->In.file = strdup(opt->arg);
				n_files++;
			}
			else
				n_errors++;
			break;

			/* Processes program-specific parameters */

		case 'G':
			if (opt->arg[0]) {
				Ctrl->G.active = true;
				Ctrl->G.file = strdup(opt->arg);
			}
			else
				n_errors++;
			break;

		default: /* Report bad options */
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	n_errors += gmt_M_check_condition(GMT, n_files != 1, "Syntax error: Must specify a single grid file\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->In.file == NULL, "Syntax error: Must specify a single grid file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code)                                                                                                            \
	{                                                                                                                            \
		gmt_M_free_options(mode);                                                                                                \
		return (code);                                                                                                           \
	}
#define Return(code)                                                                                                             \
	{                                                                                                                            \
		Free_Ctrl(GMT, Ctrl);                                                                                                    \
		gmt_end_module(GMT, GMT_cpy);                                                                                            \
		bailout(code);                                                                                                           \
	}

int GMT_mbgrd2obj(void *V_API, int mode, void *args) {
	// bool first = true;
	unsigned int row, col;
	int error = 0;

	uint64_t ij;

	// char header[GMT_BUFSIZ];

	double wesn[4];
  // double d_value;

	struct GMT_GRID *G = NULL;
	struct MBGRD2OBJ_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr(V_API); /* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL)
		return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE)
		return (GMT_mbgrd2obj_usage(API, GMT_MODULE_PURPOSE)); /* Return the purpose of program */
	options = GMT_Create_Options(API, mode, args);
	if (API->error)
		return (API->error); /* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE)
		bailout(GMT_mbgrd2obj_usage(API, GMT_USAGE)); /* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS)
		bailout(GMT_mbgrd2obj_usage(API, GMT_SYNOPSIS)); /* Return the synopsis */

		/* Parse the command-line arguments */

#if GMT_MAJOR_VERSION >= 6 && GMT_MINOR_VERSION >= 1
	if ((GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL,
	                           &options, &GMT_cpy)) == NULL)
		bailout(API->error); /* Save current state */
#elif GMT_MAJOR_VERSION >= 6
	if ((GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options,
	                           &GMT_cpy)) == NULL)
		bailout(API->error); /* Save current state */
#else
	GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, &GMT_cpy); /* Save current state */
#endif

	if (GMT_Parse_Common(API, THIS_MODULE_OPTIONS, options))
		Return(API->error);
	Ctrl = New_Ctrl(GMT); /* Allocate and initialize a new control structure */
	if ((error = parse(GMT, Ctrl, options)) != 0)
		Return(error);

	/*---------------------------- This is the mbgrd2obj main code ----------------------------*/

	GMT_Report(API, GMT_MSG_LONG_VERBOSE, "Processing input grid\n");

	gmt_M_memcpy(wesn, GMT->common.R.wesn, 4, double); /* Current -R setting, if any */

	if ((G = GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) ==
	    NULL) {
		Return(API->error);
	}

	if (gmt_M_is_subset(GMT, G->header, wesn)) /* Subset requested; make sure wesn matches header spacing */
		gmt_M_err_fail(GMT, gmt_adjust_loose_wesn(GMT, wesn, G->header), "");

	if (GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->In.file, G) == NULL) {
		Return(API->error); /* Get subset */
	}

	/* If needed create output filename by adding ".obj" to input filename
	   (replacing ".grd" if possible) */

	char *root = NULL;
	if (!Ctrl->G.active) {
		if (strlen(Ctrl->In.file) > 4 && (strncmp(&Ctrl->In.file[strlen(Ctrl->In.file) - 4], ".grd", 4) == 0 ||
		                                  strncmp(&Ctrl->In.file[strlen(Ctrl->In.file) - 4], ".GRD", 4) == 0)) {
			Ctrl->G.file = strdup(Ctrl->In.file);
			char *suffix = &Ctrl->G.file[strlen(Ctrl->G.file) - 4];
			strcpy(suffix, ".obj");
			root = strndup(Ctrl->In.file, strlen(Ctrl->In.file) - 4);
		}
		else {
			Ctrl->G.file = strndup(Ctrl->In.file, (strlen(Ctrl->In.file) + 5));
			strcat(Ctrl->G.file, ".obj");
			root = strdup(Ctrl->In.file);
		}
		Ctrl->G.active = true;
	}

	/* Open output file */
	FILE *fp = NULL;
	if ((fp = fopen(Ctrl->G.file, "w")) == NULL) {
		GMT_Report(API, GMT_MSG_NORMAL, "Unable to create file : %s\n", Ctrl->G.file);
		Return(GMT_RUNTIME_ERROR);
	}

	/* Output user information */
  char user[256], host[256], date[32];
  const int verbose = 0;
  mb_user_host_date(verbose, user, host, date, &error);
	fprintf(fp, "# OBJ format 3D model file\n");
	fprintf(fp, "# This file created by mbgrd2obs\n");
	fprintf(fp, "# MB-System Version %s\n", MB_VERSION);
	fprintf(fp, "# Run by %s on <%s> at <%s>\n#\n", user, host, date);
	fprintf(fp, "# Input grid:   %s\n", Ctrl->In.file);
	fprintf(fp, "# Output model: %s\n#\n", Ctrl->G.file);

	/* Compute grid node positions once only */

	double *x = gmt_grd_coord(GMT, G->header, GMT_X);
	double *y = gmt_grd_coord(GMT, G->header, GMT_Y);

	/* use only NaNs to represent no data in memory */

	gmt_M_grd_loop(GMT, G, row, col, ij) {
		if (gmt_input_is_nan_proxy(GMT, G->data[ij])) {
			G->data[ij] = GMT->session.d_NaN;
		}
	}

	/* Get arrays for valid vertices and triangles */

	unsigned long long nvertex = 0;
	unsigned long long nvertex_max = gmt_M_ijp(G->header, G->header->n_rows, G->header->n_columns);
	unsigned long long *vertex_id = gmt_M_memory(GMT, NULL, nvertex_max, unsigned long long);

	gmt_M_grd_loop(GMT, G, row, col, ij) {
		if (!gmt_M_is_dnan(G->data[ij])) {
			nvertex++;
			vertex_id[ij] = nvertex;
			fprintf(fp, "v %.6f %.6f %.6f\n", x[col], y[row], G->data[ij]);
		}
		else {
			vertex_id[ij] = 0;
		}
	}
	fprintf(fp, "# %llu vertices\n#\n", nvertex);

	fprintf(fp, "o o_%s\n", root);
	fprintf(fp, "g g_%s\n", root);

	int ntriangle = 0;

	gmt_M_grd_loop(GMT, G, row, col, ij) {

		if ((col < G->header->n_columns - 1) && (row < G->header->n_rows - 1)) {
			double a[3], b[3], s[3], rr;

			/* check first possible triangle */
			int ij1 = gmt_M_ijp(G->header, row + 1, col);
			int ij2 = gmt_M_ijp(G->header, row + 1, col + 1);
			int ij3 = gmt_M_ijp(G->header, row, col + 1);
			if (!(gmt_M_is_dnan(G->data[ij1]) || gmt_M_is_dnan(G->data[ij2]) || gmt_M_is_dnan(G->data[ij3]))) {
				// get surface normal
				a[0] = x[col + 1] - x[col];
				a[1] = y[row + 1] - y[row + 1];
				a[2] = G->data[ij2] - G->data[ij1];
				b[0] = x[col + 1] - x[col];
				b[1] = y[row] - y[row + 1];
				b[2] = G->data[ij3] - G->data[ij1];
				s[0] = a[1] * b[2] - a[2] * b[1];
				s[1] = a[2] * b[0] - a[0] * b[2];
				s[2] = a[0] * b[1] - a[1] * b[0];
				rr = sqrt(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
				s[0] /= rr;
				s[1] /= rr;
				s[2] /= rr;
				fprintf(fp, "f %llu %llu %llu\n", vertex_id[ij1], vertex_id[ij2], vertex_id[ij3]);
				// fprintf(fp,"1: %.6f %.6f %.6f  2: %.6f %.6f %.6f 3: %.6f %.6f %.6f\n",
				// x[col],y[col],G->data[ij1],x[col+1],y[col],G->data[ij2],x[col+1],y[col+1],G->data[ij3]);
				// fprintf(fp,"a: %.6f %.6f %.6f  b: %.6f %.6f %.6f s: %.6f %.6f %.6f\n",
				// a[0],a[1],a[2],b[0],b[1],b[2],s[0],s[1],s[2]);
				ntriangle++;
			}

			/* check second possible triangle */
			ij1 = gmt_M_ijp(G->header, row + 1, col);
			ij2 = gmt_M_ijp(G->header, row, col + 1);
			ij3 = gmt_M_ijp(G->header, row, col);
			if (!(gmt_M_is_dnan(G->data[ij1]) || gmt_M_is_dnan(G->data[ij2]) || gmt_M_is_dnan(G->data[ij3]))) {
				// get surface normal
				a[0] = x[col + 1] - x[col];
				a[1] = y[row] - y[row + 1];
				a[2] = G->data[ij2] - G->data[ij1];
				b[0] = x[col] - x[col];
				b[1] = y[row] - y[row + 1];
				b[2] = G->data[ij3] - G->data[ij1];
				s[0] = a[1] * b[2] - a[2] * b[1];
				s[1] = a[2] * b[0] - a[0] * b[2];
				s[2] = a[0] * b[1] - a[1] * b[0];
				rr = sqrt(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
				s[0] /= rr;
				s[1] /= rr;
				s[2] /= rr;

				fprintf(fp, "f %llu %llu %llu\n", vertex_id[ij1], vertex_id[ij2], vertex_id[ij3]);
				// fprintf(fp,"1: %.6f %.6f %.6f  2: %.6f %.6f %.6f 3: %.6f %.6f %.6f\n",
				// x[col],y[col],G->data[ij1],x[col+1],y[col+1],G->data[ij2],x[col],y[col+1],G->data[ij3]);
				// fprintf(fp,"a: %.6f %.6f %.6f  b: %.6f %.6f %.6f s: %.6f %.6f %.6f\n",
				// a[0],a[1],a[2],b[0],b[1],b[2],s[0],s[1],s[2]);
				ntriangle++;
			}
		}
	}
	fprintf(fp, "# %d triangles\n\n", ntriangle);

	fclose(fp);

	/* clean up */

	gmt_M_free(GMT, vertex_id);
	gmt_M_free(GMT, x);
	gmt_M_free(GMT, y);

	GMT_Report(API, GMT_MSG_LONG_VERBOSE, "%" PRIu64 " vertices output\n", nvertex);
	GMT_Report(API, GMT_MSG_LONG_VERBOSE, "%" PRIu64 " triangles output\n", ntriangle);

	Return(GMT_NOERROR);
}
