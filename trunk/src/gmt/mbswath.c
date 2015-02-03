/*--------------------------------------------------------------------
 *    The MB-system:	mbswath.c	5/30/93
 *    $Id$
 *
 *    Copyright (c) 1993-2014 by
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
 * MBSWATH is a GMT compatible utility which creates a color postscript
 * image of swath bathymetry or backscatter data.  The image
 * may be shaded relief as well.  Complete maps are made by using
 * MBSWATH in conjunction with the usual GMT programs.  The modes
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
 * Date:	January 27, 2015 (recast as GMT5 module)
 *
 */

#define THIS_MODULE_NAME	"mbswath"
#define THIS_MODULE_LIB		"mbgmt"
#define THIS_MODULE_PURPOSE	"Plot swath bathymetry, amplitude, or backscatter"

/* GMT5 header file */
#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->BJKOPRUVXY" GMT_OPT("S")

/* MBIO include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"

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
#define	MBSWATH_IMAGE_VECTOR	1
#define	MBSWATH_IMAGE_8		2
#define	MBSWATH_IMAGE_24	3

/* How B/W TV's convert RGB to Gray */
#define YIQ(rgb) (0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2])

/* global structure definitions */
#define MAXPINGS 50
struct	footprint
	{
	double	x[4];
	double	y[4];
	};
struct	ping
	{
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
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	*beamflag;
	double	*bath;
	double	*bathlon;
	double	*bathlat;
	double	*amp;
	double	*ss;
	double	*sslon;
	double	*sslat;
	char	comment[256];
	double	lonaft;
	double	lataft;
	double	lonfor;
	double	latfor;
	int	*bathflag;
	struct footprint *bathfoot;
	int	*ssflag;
	struct footprint *ssfoot;
	double	*bathshade;
	};
struct swath
	{
	int	npings;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	struct ping data[MAXPINGS];
	};

/* Control structure for mbswath */
struct MBSWATH_CTRL {

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
	int	read_data;
	void	*datalist;
	double	file_weight;
        mb_path file;
	int	filtermode;
	int	beams_bath_max;
	int	beams_amp_max;
	int	pixels_ss_max;
	void	*mbio_ptr;
	struct swath *swath_plot;

	struct mbswath_A {	/* -A<factor>/<mode>/<depth> */
		bool active;
		double factor;
                int mode;
                double depth;
	} A;
	struct mbswath_b {	/* -b<year>/<month>/<day>/<hour>/<minute>/<second> */
		bool active;
		int time_i[7];
	} b;
	struct mbswath_C {	/* -C<cptfile> */
		bool active;
		char *cptfile;
	} C;
	struct mbswath_D {	/* -D<mode>/<ampscale>/<ampmin>/<ampmax> */
		bool active;
		unsigned int mode;
		double ampscale;
		double ampmin;
		double ampmax;
	} D;
	struct mbswath_E {	/* -Ei|<dpi> */
		bool active;
		bool device_dpi;
		unsigned int dpi;
	} E;
	struct mbswath_e {	/* -e<year>/<month>/<day>/<hour>/<minute>/<second> */
		bool active;
		int time_i[7];
	} e;
	struct mbswath_F {	/* -F<format> */
		bool active;
		int format;
	} F;
	struct mbswath_G {	/* -G<magnitude>/<azimuth | median> */
		bool active;
		double magnitude;
                double azimuth;
	} G;
	struct mbswath_I {	/* -I<inputfile> */
		bool active;
		char *inputfile;
	} I;
	struct mbswath_L {	/* -L<lonflip> */
		bool active;
                int lonflip;
	} L;
	struct mbswath_N {	/* -N<cptfile> */
		bool active;
                char *cptfile;
	} N;
	struct mbswath_S {	/* -S<speed> */
		bool active;
		double speed;
	} S;
	struct mbswath_T {	/* -T<timegap> */
		bool active;
		double timegap;
	} T;
	struct mbswath_W {	/* -W */
		bool active;
	} W;
	struct mbswath_Z {	/* -Z<mode> */
		bool active;
                int mode;
                int filter;
                int usefiltered;
	} Z;
};

int mbswath_get_footprints(int verbose, struct MBSWATH_CTRL *Ctrl, int *error);
int mbswath_get_shading(int verbose, struct MBSWATH_CTRL *Ctrl,
                        struct GMT_CTRL *GMT, struct GMT_PALETTE *CPT, int *error);
int mbswath_plot_data_footprint(int verbose, struct MBSWATH_CTRL *Ctrl,
                        struct GMT_CTRL *GMT, struct GMT_PALETTE *CPT, struct PSL_CTRL *PSL,
                        int first, int nplot, int *error);
int mbswath_plot_data_point(int verbose, struct MBSWATH_CTRL *Ctrl,
                        struct GMT_CTRL *GMT, struct GMT_PALETTE *CPT, struct PSL_CTRL *PSL,
                        int first, int nplot, int *error);
int mbswath_plot_box(int verbose, struct MBSWATH_CTRL *Ctrl,
                        struct GMT_CTRL *GMT, struct PSL_CTRL *PSL,
                        double *x, double *y, double *rgb, int *error);
int mbswath_plot_point(int verbose, struct MBSWATH_CTRL *Ctrl, struct GMT_CTRL *GMT, struct PSL_CTRL *PSL,
                        double x, double y, double *rgb, int *error);
int mbswath_ping_copy(int verbose, int one, int two, struct swath *swath, int *error);

void *New_mbswath_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MBSWATH_CTRL *Ctrl;
        int     status;
        int     verbose = 0;
        double  dummybounds[4];
        int     dummyformat;
        int     dummypings;
        int     i;

	Ctrl = GMT_memory (GMT, NULL, 1, struct MBSWATH_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	/* get current mb default values */
	status = mb_defaults(verbose, &dummyformat, &dummypings, &Ctrl->L.lonflip, dummybounds,
		Ctrl->b.time_i, Ctrl->e.time_i, &Ctrl->S.speed, &Ctrl->T.timegap);

        Ctrl->A.factor = 1.0;
	Ctrl->A.mode = MBSWATH_FOOTPRINT_REAL;
        Ctrl->A.depth = 3000.0;
        Ctrl->C.cptfile = NULL;
        Ctrl->D.mode = 1;
        Ctrl->D.ampscale = 1.0;
        Ctrl->D.ampmin = 0.0;
        Ctrl->D.ampmax = 1.0;
        Ctrl->E.device_dpi = 0;
        Ctrl->E.dpi = 100;
        Ctrl->F.format = 0;
        Ctrl->G.magnitude = 1.0;
        Ctrl->G.azimuth = 270.0;
        Ctrl->I.inputfile = NULL;
        Ctrl->N.cptfile = NULL;
        Ctrl->S.speed = 0.0;
        Ctrl->T.timegap = 1.0;
	Ctrl->Z.mode = MBSWATH_BATH;
        Ctrl->Z.filter = 0;
        Ctrl->Z.usefiltered = MB_NO;
        
        /* mbswath variables */
        for (i=0;i<4;i++)
                {
                Ctrl->bounds[i] = 0.0;
                Ctrl->clipx[i] = 0.0;
                Ctrl->clipy[i] = 0.0;
                }
        Ctrl->image_type = MBSWATH_IMAGE_24;
        Ctrl->mtodeglon = 0.0;
        Ctrl->mtodeglat = 0.0;
        Ctrl->x_inch = 0.0;
        Ctrl->y_inch = 0.0;
	Ctrl->x_inc = 0.0;
	Ctrl->y_inc = 0.0;
	Ctrl->x_side = 0.0;
	Ctrl->y_side = 0.0;
        Ctrl->x0 = 0.0;
        Ctrl->y0 = 0.0;
        Ctrl->nx = 0;
        Ctrl->ny = 0;
        Ctrl->nm = 0;
	Ctrl->bitimage = NULL;
        Ctrl->format = 0;
	Ctrl->beamwidth_xtrack = 0.0;
	Ctrl->beamwidth_ltrack = 0.0;
	Ctrl->btime_d = 0.0;
	Ctrl->etime_d = 0.0;
        Ctrl->read_datalist = MB_NO;
	Ctrl->read_data = 0;
	Ctrl->datalist = NULL;
	Ctrl->file_weight = 0.0;
        Ctrl->file[0] = '\0';
	Ctrl->filtermode = MBSWATH_FILTER_NONE;
	Ctrl->beams_bath_max = 0;
	Ctrl->beams_amp_max = 0;
	Ctrl->pixels_ss_max = 0;
	Ctrl->mbio_ptr = NULL;
	Ctrl->swath_plot = NULL;

	return (Ctrl);
}

void Free_mbswath_Ctrl (struct GMT_CTRL *GMT, struct MBSWATH_CTRL *Ctrl) {	/* Deallocate control structure */
	if (!Ctrl) return;
	if (Ctrl->C.cptfile) free (Ctrl->C.cptfile);
	if (Ctrl->I.inputfile) free (Ctrl->I.inputfile);
	if (Ctrl->N.cptfile) free (Ctrl->N.cptfile);
	GMT_free (GMT, Ctrl);
}

int GMT_mbswath_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: mbswath -I<inputfile> %s [%s]\n", GMT_J_OPT, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-A<factor>/<mode>/<depth>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-b<year>/<month>/<day>/<hour>/<minute>/<second>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-C<cptfile>] [-D<mode>/<ampscale>/<ampmin>/<ampmax>] [-Ei|<dpi>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-e<year>/<month>/<day>/<hour>/<minute>/<second>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-F<format>] [-G<magnitude>/<azimuth | median>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-I<inputfile>] [-L<lonflip>] [-N<cptfile>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-S<speed>] [-T<timegap>] [-W] [-Z<mode>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-T] [%s] [%s]\n", GMT_Rgeo_OPT, GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s]\n\t[%s] [%s]\n\n", 
			GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_f_OPT, GMT_n_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<inputfile> is an MB-System datalist referencing the swath data to be plotted.\n");
	GMT_Option (API, "J-");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Color palette file to convert z to rgb.  Optionally, instead give name of a master cpt\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to automatically assign 16 continuous colors over the data range [rainbow].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set dpi for the projected output Postscript image\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   if -Jx or -Jm is not selected.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give i to do the interpolation in PostScript at device resolution.\n");
	GMT_rgb_syntax (API->GMT, 'G', "Set transparency color for images that otherwise would result in 1-bit images.\n\t  ");
	GMT_Option (API, "K");
	GMT_Option (API, "O,P");
	GMT_Option (API, "R");
	GMT_Option (API, "U,V,X,c,f,n,p,t,.");

	return (EXIT_FAILURE);
}

int GMT_mbswath_parse (struct GMT_CTRL *GMT, struct MBSWATH_CTRL *Ctrl, struct GMT_OPTION *options)
{
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
				if (GMT_check_filearg (GMT, '<', opt->arg, GMT_IN))
					{
                                        Ctrl->I.inputfile = strdup (opt->arg);
                                        n_files = 1;
                                        }
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* footprint controls */
				n = sscanf(opt->arg, "%lf/%d/%lf", &(Ctrl->A.factor), &(Ctrl->A.mode), &(Ctrl->A.depth));
                                if (n > 0)
                                        Ctrl->A.active = true;
                                else
                                        n_errors++;
 				break;
			case 'b':	/* btime_i */
				n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d",
                                           &(Ctrl->b.time_i[0]), &(Ctrl->b.time_i[1]), &(Ctrl->b.time_i[2]),
                                           &(Ctrl->b.time_i[3]), &(Ctrl->b.time_i[4]), &(Ctrl->b.time_i[5]));
                                Ctrl->b.time_i[6] = 0;
                                if (n == 6)
                                        Ctrl->b.active = true;
                                else
                                        n_errors++;
				break;
			case 'C':	/* CPT file */
				Ctrl->C.active = true;
				if (Ctrl->C.cptfile) free (Ctrl->C.cptfile);
				Ctrl->C.cptfile = strdup (opt->arg);
				break;
			case 'D':	/* amplitude scaling */
				n = sscanf(opt->arg, "%d/%lf/%lf/%lf",
                                           &(Ctrl->D.mode), &(Ctrl->D.ampscale),
                                           &(Ctrl->D.ampmin), &(Ctrl->D.ampmax));
                                if (n > 0)
                                        Ctrl->A.active = true;
                                else
                                        n_errors++;
 				break;
			case 'E':	/* dpi */
				if (strcmp(opt->arg, "i") == 0)
                                        {
                                        Ctrl->E.device_dpi = true;
                                        Ctrl->E.active = true;
                                        }
                                else
                                        {
                                        n = sscanf(opt->arg, "%d", &(Ctrl->E.dpi));
                                        if (n == 1)
                                                Ctrl->E.active = true;
                                        else
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
                                else
                                        n_errors++;
				break;
			case 'f':	/* format */
			case 'F':	/* format */
				n = sscanf(opt->arg, "%d", &(Ctrl->F.format));
                                if (n == 1)
                                        Ctrl->F.active = true;
                                else
                                        n_errors++;
				break;
			case 'G':	/* -G<magnitude>/<azimuth | median> */
				n = sscanf(opt->arg, "%lf/%lf", &(Ctrl->G.magnitude), &(Ctrl->G.azimuth));
                                if (n >= 1)
                                        Ctrl->G.active = true;
                                else
                                        n_errors++;
				break;

			case 'I':	/* -I<inputfile> */
				Ctrl->I.active = true;
				if (!GMT_access (GMT, opt->arg, R_OK))	/* Got a file */
					{
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
                                else
                                        n_errors++;
				break;
			case 'N':	/* -N<cptfile> */
				Ctrl->N.active = true;
				if (Ctrl->N.cptfile) free (Ctrl->N.cptfile);
				Ctrl->N.cptfile = strdup (opt->arg);
				break;
			case 'S':	/* -S<speed> */
				n = sscanf(opt->arg, "%lf", &(Ctrl->S.speed));
                                if (n == 1)
                                        Ctrl->S.active = true;
                                else
                                        n_errors++;
				break;
			case 'T':	/* -T<timegap> */
				n = sscanf(opt->arg, "%lf", &(Ctrl->T.timegap));
                                if (n == 1)
                                        Ctrl->T.active = true;
                                else
                                        n_errors++;
				break;
			case 'W':	/* -W */
				Ctrl->W.active = true;
				break;
			case 'Z':	/* -Z<mode> */
				n = sscanf(opt->arg, "%d", &(Ctrl->Z.mode));
                                if (n == 1)
                                        {
                                        Ctrl->Z.active = true;
                                        if (opt->arg[1] == 'f' || opt->arg[1] == 'F')
                                                Ctrl->Z.usefiltered = MB_YES;
                                        else
                                                Ctrl->Z.usefiltered = MB_NO;
                                        }
                                else
                                        n_errors++;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !GMT->common.J.active, 
					"Syntax error: Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, n_files != 1, 
					"Syntax error: Must specify one input file(s)\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && !Ctrl->I.inputfile, 
					"Syntax error -I option: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && !Ctrl->E.device_dpi && Ctrl->E.dpi <= 0, 
					"Syntax error -E option: dpi must be positive\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_mbswath_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_mbswath (void *V_API, int mode, void *args)
{
//	bool done, need_to_project, normal_x, normal_y, resampled = false, gray_only = false;
//	bool nothing_inside = false, use_intensity_grid;
//	unsigned int k, nx = 0, ny = 0, grid_registration = GMT_GRID_NODE_REG, n_grids;
//	unsigned int colormask_offset = 0, try, row, actual_row, col;
//	uint64_t node_RGBA = 0;		/* uint64_t for the RGB(A) image array. */
//	uint64_t node, kk, nm, byte;
//	int index = 0, ks, error = 0;
	
//	unsigned char *bitimage_8 = NULL, *bitimage_24 = NULL, *rgb_used = NULL, i_rgb[3];

//	double dx, dy, x_side, y_side, x0 = 0.0, y0 = 0.0, rgb[4] = {0.0, 0.0, 0.0, 0.0};
//	double *NaN_rgb = NULL, red[4] = {1.0, 0.0, 0.0, 0.0}, wesn[4];

//	struct GMT_GRID *Grid_orig[3] = {NULL, NULL, NULL}, *Grid_proj[3] = {NULL, NULL, NULL};
//	struct GMT_GRID *Intens_orig = NULL, *Intens_proj = NULL;
//	struct GMT_PALETTE *P = NULL;
//	struct MBSWATH_CTRL *Ctrl = NULL;
//	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT interal parameters */
//	struct GMT_OPTION *options = NULL;
//	struct PSL_CTRL *PSL = NULL;	/* General PSL interal parameters */
//	struct GMT_GRID_HEADER *header_work = NULL;	/* Pointer to a GMT header for the image or grid */
//	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	char program_name[] = "mbswath";
//	char help_message[] =  "mbswath is a GMT compatible utility which creates a color postscript \nimage of swath bathymetry or backscatter data.  The image \nmay be shaded relief as well.  Complete maps are made by using \nMBSWATH in conjunction with the usual GMT programs.";
//	char usage_message[] = "mbswath -Ccptfile -Jparameters -Rwest/east/south/north \n\t[-Afactor -Btickinfo -byr/mon/day/hour/min/sec \n\t-ccopies -Dmode/ampscale/ampmin/ampmax \n\t-Eyr/mon/day/hour/min/sec -fformat \n\t-Fred/green/blue -Gmagnitude/azimuth -Idatalist \n\t-K -Ncptfile -O -P -ppings -Qdpi -Ttimegap -U -W -Xx-shift -Yy-shift \n\t-Zmode[F] -V -H]";

	struct GMT_PALETTE *CPTcolor = NULL;
	struct GMT_PALETTE *CPTshade = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;	/* General PSL interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
        struct MBSWATH_CTRL *Ctrl = NULL;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message = NULL;

        mb_path file;
        int     format;
        int     file_in_bounds;
        int     read_data;
	struct ping *pingcur;
        double  amplog;
        int     *npings;
        int     nping_read = 0;
        int     start, done, first, nplot;
        int     plot;
        int     flush;
        int     save_new;
        int     i;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_mbswath_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_mbswath_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_mbswath_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options))
                Return (API->error);
	Ctrl = (struct MBSWATH_CTRL *) New_mbswath_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_mbswath_parse (GMT, Ctrl, options)))
                Return (error);

	/*---------------------------- This is the mbswath main code ----------------------------*/
        
        /* set verbosity */
        verbose = GMT->common.V.active;

        /* set bounds for data reading larger than map borders */
	Ctrl->bounds[0] = GMT->common.R.wesn[0]
		- 0.25*(GMT->common.R.wesn[1] - GMT->common.R.wesn[0]);
	Ctrl->bounds[1] = GMT->common.R.wesn[1]
		+ 0.25*(GMT->common.R.wesn[1] - GMT->common.R.wesn[0]);
	Ctrl->bounds[2] = GMT->common.R.wesn[2]
		- 0.25*(GMT->common.R.wesn[3] - GMT->common.R.wesn[2]);
	Ctrl->bounds[3] = GMT->common.R.wesn[3]
		+ 0.25*(GMT->common.R.wesn[3] - GMT->common.R.wesn[2]);

	/* get scaling from degrees to km */
	mb_coor_scale(verbose, 0.5*(Ctrl->bounds[2] + Ctrl->bounds[3]), &Ctrl->mtodeglon, &Ctrl->mtodeglat);

	/* set lonflip if needed */
	if (!Ctrl->L.active)
		{
		if (Ctrl->bounds[0] < -180.0)
			Ctrl->L.lonflip = -1;
		else if (Ctrl->bounds[1] > 180.0)
			Ctrl->L.lonflip = 1;
		else if (Ctrl->L.lonflip == -1 && Ctrl->bounds[1] > 0.0)
			Ctrl->L.lonflip = 0;
		else if (Ctrl->L.lonflip == 1 && Ctrl->bounds[0] < 0.0)
			Ctrl->L.lonflip = 0;
		}

	/* Start the postscript plot */
	GMT_err_fail (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "");
	PSL = GMT_plotinit (GMT, options);
	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	GMT_plotcanvas (GMT);	/* Fill canvas if requested */
	GMT_map_clip_on (GMT, GMT->session.no_rgb, 3);

	/* Read the color palette file */
	if (Ctrl->C.active)
                {   /* Read palette file */
		if ((CPTcolor = GMT_Get_CPT (GMT, Ctrl->C.cptfile, GMT_CPT_REQUIRED, 0.0, 0.0)) == NULL)
                        {
			Return (API->error);
			}
                if (CPTcolor && CPTcolor->is_gray && Ctrl->image_type == MBSWATH_IMAGE_24)
                        Ctrl->image_type = MBSWATH_IMAGE_8;
        	}

	/* Read the color palette file for amplitude shading if requested */
	if (Ctrl->N.active)
                {   /* Read palette file */
		if ((CPTshade = GMT_Get_CPT (GMT, Ctrl->N.cptfile, GMT_CPT_REQUIRED, 0.0, 0.0)) == NULL)
                        {
			Return (API->error);
			}
        	}
                
        /* Set particulars of output image for the postscript plot */
	GMT_geo_to_xy(GMT, GMT->common.R.wesn[0], GMT->common.R.wesn[2], &Ctrl->clipx[0], &Ctrl->clipy[0]);
	GMT_geo_to_xy(GMT, GMT->common.R.wesn[1], GMT->common.R.wesn[2], &Ctrl->clipx[1], &Ctrl->clipy[1]);
	GMT_geo_to_xy(GMT, GMT->common.R.wesn[1], GMT->common.R.wesn[3], &Ctrl->clipx[2], &Ctrl->clipy[2]);
	GMT_geo_to_xy(GMT, GMT->common.R.wesn[0], GMT->common.R.wesn[3], &Ctrl->clipx[3], &Ctrl->clipy[3]);
	Ctrl->x_inch = Ctrl->clipx[1] - Ctrl->clipx[0];
	Ctrl->y_inch = Ctrl->clipy[2] - Ctrl->clipy[1];
	Ctrl->x0 = Ctrl->clipx[0];
	Ctrl->y0 = Ctrl->clipy[0];
	Ctrl->nx = Ctrl->x_inch * Ctrl->E.dpi;
	Ctrl->ny = Ctrl->y_inch * Ctrl->E.dpi;
        Ctrl->x_inc = (GMT->common.R.wesn[1] - GMT->common.R.wesn[0]) / (Ctrl->nx - 1);
        Ctrl->y_inc = (GMT->common.R.wesn[3] - GMT->common.R.wesn[2]) / (Ctrl->ny - 1);
        Ctrl->x_side = Ctrl->x_inc * Ctrl->nx;
        Ctrl->y_side = Ctrl->y_inc * Ctrl->ny;
	Ctrl->nm = Ctrl->nx * Ctrl->ny;
	Ctrl->nm2 = 2 * Ctrl->nm;

        /* allocate and initialize the output image */
        if (Ctrl->image_type == MBSWATH_IMAGE_8)
		{
                Ctrl->bitimage = GMT_memory (GMT, NULL, Ctrl->nm, unsigned char);
                memset(Ctrl->bitimage, 255, Ctrl->nm);
                }
	else if (Ctrl->image_type == MBSWATH_IMAGE_24)
		{
                Ctrl->bitimage = GMT_memory (GMT, NULL, 3 * Ctrl->nm, unsigned char);
                memset(Ctrl->bitimage, 255, 3 * Ctrl->nm);
                }
                
	/* get format if required */
	if (Ctrl->F.format == 0)
		mb_get_format(verbose, Ctrl->I.inputfile, NULL, &Ctrl->F.format, &error);

	/* turn on looking for filtered amp or sidescan if needed */
	if (Ctrl->Z.usefiltered == MB_YES)
		{
		if (Ctrl->Z.mode == MBSWATH_BATH_AMP)
			Ctrl->filtermode = MBSWATH_FILTER_AMP;
		else if (Ctrl->Z.mode == MBSWATH_AMP)
			Ctrl->filtermode = MBSWATH_FILTER_AMP;
		else if (Ctrl->Z.mode == MBSWATH_SS)
			Ctrl->filtermode = MBSWATH_FILTER_SIDESCAN;
		}

	/* determine whether to read one file or a list of files */
	if (Ctrl->F.format < 0)
		Ctrl->read_datalist = MB_YES;

	/* open file list */
	if (Ctrl->read_datalist == MB_YES)
	    {
	    if ((status = mb_datalist_open(verbose, &Ctrl->datalist,
					    Ctrl->I.inputfile, MB_DATALIST_LOOK_UNSET, &error)) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open data list file: %s\n", Ctrl->I.inputfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
		exit(error);
		}
	    if ((status = mb_datalist_read(verbose, Ctrl->datalist,
			     file, &format, &Ctrl->file_weight, &error))
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	else
	    {
	    strcpy(file, Ctrl->I.inputfile);
	    read_data = MB_YES;
	    }

	/* loop over files in file list */
	if (verbose == 1)
		fprintf(stderr,"\n");
	while (read_data == MB_YES)
	    {
	    /* check for mbinfo file - get file bounds if possible */
	    status = mb_check_info(verbose, file, Ctrl->L.lonflip, Ctrl->bounds,
			    &file_in_bounds, &error);
	    if (status == MB_FAILURE)
		    {
		    file_in_bounds = MB_YES;
		    status = MB_SUCCESS;
		    error = MB_ERROR_NO_ERROR;
		    }

	    /* read if data may be in bounds */
	    if (file_in_bounds == MB_YES)
		{
		/* check for "fast bathymetry" or "fbt" file */
		if (Ctrl->Z.mode == MBSWATH_BATH
		    || Ctrl->Z.mode == MBSWATH_BATH_RELIEF)
		    {
		    mb_get_fbt(verbose, file, &format, &error);
		    }

		/* check for filtered amplitude or sidescan file */
		if (Ctrl->filtermode == MBSWATH_FILTER_AMP)
		    {
		    if ((status = mb_get_ffa(verbose, file, &format, &error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error returned from function <mb_get_ffa>:\n%s\n",message);
			fprintf(stderr,"Requested filtered amplitude file missing\n");
			fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		    }
		else if (Ctrl->filtermode == MBSWATH_FILTER_SIDESCAN)
		    {
		    if ((status = mb_get_ffs(verbose, file, &format, &error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error returned from function <mb_get_ffs>:\n%s\n",message);
			fprintf(stderr,"Requested filtered sidescan file missing\n");
			fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		    }

		/* call mb_read_init() */
		if ((status = mb_read_init(verbose, file, format, 1, Ctrl->L.lonflip, Ctrl->bounds,
                                            Ctrl->b.time_i, Ctrl->e.time_i, Ctrl->S.speed, Ctrl->T.timegap,
                                            &Ctrl->mbio_ptr, &Ctrl->btime_d, &Ctrl->etime_d,
                                            &Ctrl->beams_bath_max, &Ctrl->beams_amp_max, &Ctrl->pixels_ss_max, &error)) != MB_SUCCESS)
		    {
		    mb_error(verbose,error,&message);
		    fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		    fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }

		/* get fore-aft beam_width */
		status = mb_format_beamwidth(verbose, &format,
				&Ctrl->beamwidth_xtrack, &Ctrl->beamwidth_ltrack,
				&error);
		if (Ctrl->beamwidth_ltrack <= 0.0)
			Ctrl->beamwidth_ltrack = 2.0;
		if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL)
                        Ctrl->footprint_factor = Ctrl->A.factor * Ctrl->beamwidth_ltrack;
		else
                        Ctrl->footprint_factor = Ctrl->A.factor;

		/* allocate memory for data arrays */
		status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct swath),
				(void **)&Ctrl->swath_plot, &error);
		npings = &Ctrl->swath_plot->npings;
		Ctrl->swath_plot->beams_bath = Ctrl->beams_bath_max;
		Ctrl->swath_plot->beams_amp = Ctrl->beams_amp_max;
		Ctrl->swath_plot->pixels_ss = Ctrl->pixels_ss_max;
		for (i=0;i<MAXPINGS;i++)
		    {
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
			    status = mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(char), (void **)&(pingcur->beamflag), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&(pingcur->bath), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(double), (void **)&(pingcur->amp), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&(pingcur->bathlon), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&(pingcur->bathlat), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&(pingcur->ss), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&(pingcur->sslon), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&(pingcur->sslat), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(int), (void **)&(pingcur->bathflag), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(struct footprint), (void **)&(pingcur->bathfoot), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(int), (void **)&(pingcur->ssflag), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(struct footprint), (void **)&(pingcur->ssfoot), &error);
		    if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(verbose, Ctrl->mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&(pingcur->bathshade), &error);
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

		/* print message */
		if (verbose >= 2)
		    fprintf(stderr,"\n");
		if (verbose >= 1)
		    fprintf(stderr,"processing data in %s...\n",file);

		/* loop over reading */
		*npings = 0;
		start = MB_YES;
		done = MB_NO;
		while (done == MB_NO)
		    {
		    pingcur = &Ctrl->swath_plot->data[*npings];
		    status = mb_read(verbose,Ctrl->mbio_ptr,&(pingcur->kind),
			    &(pingcur->pings),pingcur->time_i,&(pingcur->time_d),
			    &(pingcur->navlon),&(pingcur->navlat),
			    &(pingcur->speed),&(pingcur->heading),
			    &(pingcur->distance),&(pingcur->altitude),
			    &(pingcur->sonardepth),
			    &(pingcur->beams_bath),
			    &(pingcur->beams_amp),
			    &(pingcur->pixels_ss),
			    pingcur->beamflag,pingcur->bath,pingcur->amp,
			    pingcur->bathlon,pingcur->bathlat,
			    pingcur->ss,pingcur->sslon,pingcur->sslat,
			    pingcur->comment,&error);

		    /* print debug statements */
		    if (verbose >= 2)
			    {
			    fprintf(stderr,"\ndbg2  Ping read in program <%s>\n", program_name);
			    fprintf(stderr,"dbg2       kind:           %d\n", pingcur->kind);
			    fprintf(stderr,"dbg2       beams_bath:     %d\n", pingcur->beams_bath);
			    fprintf(stderr,"dbg2       beams_amp:      %d\n", pingcur->beams_amp);
			    fprintf(stderr,"dbg2       pixels_ss:      %d\n", pingcur->pixels_ss);
			    fprintf(stderr,"dbg2       error:          %d\n", error);
			    fprintf(stderr,"dbg2       status:         %d\n", status);
			    for (i=0;i<pingcur->beams_bath;i++)
				    {
				    fprintf(stderr, "bath[%4d]:  %3d  %f  %f  %f\n",
					    i, pingcur->beamflag[i], pingcur->bath[i], pingcur->bathlon[i], pingcur->bathlat[i]);
				    }
			    for (i=0;i<pingcur->beams_amp;i++)
				    {
				    fprintf(stderr, "amp[%4d]:  %f  %f  %f\n",
					    i, pingcur->amp[i], pingcur->bathlon[i], pingcur->bathlat[i]);
				    }
			    for (i=0;i<pingcur->pixels_ss;i++)
				    {
				    fprintf(stderr, "ss[%4d]:  %f  %f  %f\n",
					    i, pingcur->ss[i], pingcur->sslon[i], pingcur->sslat[i]);
				    }
			    }

		    /* ignore time gaps */
		    if (error == MB_ERROR_TIME_GAP)
		    	    {
			    error = MB_ERROR_NO_ERROR;
			    status = MB_SUCCESS;
			    }

		    /* update bookkeeping */
		    if (error == MB_ERROR_NO_ERROR)
			    {
			    nping_read += pingcur->pings;
			    (*npings)++;
			    }

		    /* scale amplitudes if necessary */
		    if (error == MB_ERROR_NO_ERROR
			    && (mode == MBSWATH_BATH_AMP
			    || mode == MBSWATH_AMP)
			    && Ctrl->D.mode > 0)
			    {
			    for (i=0;i<pingcur->beams_amp;i++)
				    {
				    if (mb_beam_ok(pingcur->beamflag[i]) && Ctrl->D.mode == 1)
					{
					pingcur->amp[i] = Ctrl->D.ampscale * (pingcur->amp[i] - Ctrl->D.ampmin)
					     / (Ctrl->D.ampmax - Ctrl->D.ampmin);
					}
				    else if (mb_beam_ok(pingcur->beamflag[i]) && Ctrl->D.mode == 2)
					{
					pingcur->amp[i] = MIN(pingcur->amp[i],Ctrl->D.ampmax);
					pingcur->amp[i] = MAX(pingcur->amp[i],Ctrl->D.ampmin);
					pingcur->amp[i] = Ctrl->D.ampscale * (pingcur->amp[i] - Ctrl->D.ampmin)
                                                            / (Ctrl->D.ampmax - Ctrl->D.ampmin);
					}
				    else if (mb_beam_ok(pingcur->beamflag[i]) && Ctrl->D.mode == 3)
					{
					amplog = 20.0 * log10(pingcur->amp[i]);
					pingcur->amp[i] = Ctrl->D.ampscale * (amplog - Ctrl->D.ampmin)
					     / (Ctrl->D.ampmax - Ctrl->D.ampmin);
					}
				    else if (mb_beam_ok(pingcur->beamflag[i]) && Ctrl->D.mode == 4)
					{
					amplog = 20.0 * log10(pingcur->amp[i]);
					amplog = MIN(amplog,Ctrl->D.ampmax);
					amplog = MAX(amplog,Ctrl->D.ampmin);
					pingcur->amp[i] = Ctrl->D.ampscale * (amplog - Ctrl->D.ampmin)
					        / (Ctrl->D.ampmax - Ctrl->D.ampmin);
					}
				    }
			    }

		    /* scale bathymetry from meters to feet if necessary */
		    if (error == MB_ERROR_NO_ERROR
			    && Ctrl->W.active == true)
			    {
			    for (i=0;i<pingcur->beams_bath;i++)
				    {
				    pingcur->bath[i] = 3.2808399 * pingcur->bath[i];
				    }
			    }

		    /* scale sidescan if necessary */
		    if (error == MB_ERROR_NO_ERROR
			    && mode == MBSWATH_SS
			    && Ctrl->D.mode > 0)
			    {
			    for (i=0;i<pingcur->pixels_ss;i++)
				    {
				    if (pingcur->ss[i] > MB_SIDESCAN_NULL && Ctrl->D.mode == 1)
					{
					pingcur->ss[i] = Ctrl->D.ampscale * (pingcur->ss[i] - Ctrl->D.ampmin)
					     / (Ctrl->D.ampmax - Ctrl->D.ampmin);
					}
				    else if (pingcur->ss[i] > MB_SIDESCAN_NULL && Ctrl->D.mode == 2)
					{
					pingcur->ss[i] = MIN(pingcur->ss[i],Ctrl->D.ampmax);
					pingcur->ss[i] = MAX(pingcur->ss[i],Ctrl->D.ampmin);
					pingcur->ss[i] = Ctrl->D.ampscale * (pingcur->ss[i] - Ctrl->D.ampmin)
					     / (Ctrl->D.ampmax - Ctrl->D.ampmin);
					}
				    else if (pingcur->ss[i] > MB_SIDESCAN_NULL && Ctrl->D.mode == 3)
					{
					amplog = 20.0 * log10(pingcur->ss[i]);
					pingcur->ss[i] = Ctrl->D.ampscale * (amplog - Ctrl->D.ampmin)
					     / (Ctrl->D.ampmax - Ctrl->D.ampmin);
					}
				    else if (pingcur->ss[i] > MB_SIDESCAN_NULL && Ctrl->D.mode == 4)
					{
					amplog = 20.0 * log10(pingcur->ss[i]);
					amplog = MIN(amplog,Ctrl->D.ampmax);
					amplog = MAX(amplog,Ctrl->D.ampmin);
					pingcur->ss[i] = Ctrl->D.ampscale * (amplog - Ctrl->D.ampmin)
					        / (Ctrl->D.ampmax - Ctrl->D.ampmin);
					}
				    }
			    }

		    /* decide whether to plot, whether to
			    save the new ping, and if done */
		    plot = MB_NO;
		    flush = MB_NO;
		    if (*npings >= MAXPINGS)
			    plot = MB_YES;
		    if (*npings > 0
			    && (error > MB_ERROR_NO_ERROR
			    || error == MB_ERROR_TIME_GAP
			    || error == MB_ERROR_OUT_BOUNDS
			    || error == MB_ERROR_OUT_TIME
			    || error == MB_ERROR_SPEED_TOO_SMALL))
			    {
			    plot = MB_YES;
			    flush = MB_YES;
			    }
		    save_new = MB_NO;
		    if (error == MB_ERROR_TIME_GAP)
			    save_new = MB_YES;
		    if (error > MB_ERROR_NO_ERROR)
			    done = MB_YES;

		    /* if enough pings read in, plot them */
		    if (plot == MB_YES)
			    {
			    /* get footprint locations */
			    if (Ctrl->A.mode != MBSWATH_FOOTPRINT_POINT)
			    status = mbswath_get_footprints(verbose, Ctrl, &error);

			    /* get shading */
			    if (Ctrl->Z.mode == MBSWATH_BATH_RELIEF
				    || Ctrl->Z.mode == MBSWATH_BATH_AMP)
				    status = mbswath_get_shading(verbose, Ctrl, GMT, CPTshade, &error);

			    /* plot data */
			    if (start == MB_YES)
				    {
				    first = 0;
				    start = MB_NO;
				    }
			    else
				    first = 1;
			    if (done == MB_YES)
				    nplot = *npings - first;
			    else
				    nplot = *npings - first - 1;

			    if (Ctrl->A.mode == MBSWATH_FOOTPRINT_POINT)
				    status = mbswath_plot_data_point(verbose, Ctrl, GMT, CPTcolor, PSL,
                                                                first, nplot, &error);
			    else
				    status = mbswath_plot_data_footprint(verbose, Ctrl, GMT, CPTcolor, PSL,
                                                                first, nplot, &error);


			    /* reorganize data */
			    if (flush == MB_YES && save_new == MB_YES)
				    {
				    status = mbswath_ping_copy(verbose,0, *npings, Ctrl->swath_plot, &error);
				    *npings = 1;
				    start = MB_YES;
				    }
			    else if (flush == MB_YES)
				    {
				    *npings = 0;
				    start = MB_YES;
				    }
			    else if (*npings > 1)
				    {
				    for (i=0;i<2;i++)
					    status = mbswath_ping_copy(verbose,i, *npings-2+i, Ctrl->swath_plot, &error);
				    *npings = 2;
				    }
			    }
		    }
		status = mb_close(verbose,&Ctrl->mbio_ptr,&error);

		/* deallocate memory for data arrays */
		mb_freed(verbose,__FILE__, __LINE__, (void **)&Ctrl->swath_plot, &error);
		} /* end if file in bounds */

	    /* figure out whether and what to read next */
	    if (Ctrl->read_datalist == MB_YES)
                {
		if ((status = mb_datalist_read(verbose, Ctrl->datalist, file, &format, &Ctrl->file_weight, &error))
			    == MB_SUCCESS)
                        read_data = MB_YES;
                else
                        read_data = MB_NO;
                }
	    else
                {
                read_data = MB_NO;
                }

	    /* end loop over files in list */
	    }
	if (Ctrl->read_datalist == MB_YES)
		mb_datalist_close(verbose, &Ctrl->datalist, &error);

	/* Generate grayscale 8-bit image */
        if (Ctrl->image_type == MBSWATH_IMAGE_8)
                {
		GMT_Report (API, GMT_MSG_VERBOSE, "Creating 8-bit grayshade image\n");
		PSL_plotcolorimage (PSL, Ctrl->x0, Ctrl->y0, Ctrl->x_inch, Ctrl->y_inch, PSL_BL, Ctrl->bitimage, Ctrl->nx, Ctrl->ny, (Ctrl->E.device_dpi ? -8 : 8));
                }
                
        /* Generate full color 24-bit image */
	else if (Ctrl->image_type == MBSWATH_IMAGE_24)
                {
		GMT_Report (API, GMT_MSG_VERBOSE, "Creating 24-bit color image\n");
		PSL_plotcolorimage (PSL, Ctrl->x0, Ctrl->y0, Ctrl->x_inch, Ctrl->y_inch, PSL_BL, Ctrl->bitimage, Ctrl->nx, Ctrl->ny, (Ctrl->E.device_dpi ? -24 : 24));
                }

	GMT_map_clip_off (GMT);

	GMT_map_basemap (GMT);
	GMT_plane_perspective (GMT, -1, 0.0);
	GMT_plotend (GMT);

	/* Free bitimage arrays. GMT_free will not complain if they have not been used (NULL) */
	if (Ctrl->bitimage) GMT_free (GMT, Ctrl->bitimage);

	if (!Ctrl->C.active && GMT_Destroy_Data (API, &CPTcolor) != GMT_OK)
                {
		Return (API->error);
                }
	if (!Ctrl->N.active && GMT_Destroy_Data (API, &CPTshade) != GMT_OK)
                {
		Return (API->error);
                }
	Return (EXIT_SUCCESS);
}
/*--------------------------------------------------------------------*/
int mbswath_get_footprints(int verbose, struct MBSWATH_CTRL *Ctrl, int *error)
{
	char	*function_name = "mbswath_get_footprints";
	int	status = MB_SUCCESS;
        struct swath *swath;
        struct ping *pingcur;
	struct footprint *print;
	int	dobath, doss;
	double	headingx, headingy;
	double	dx, dy, r, dlon1, dlon2, dlat1, dlat2, tt, x, y;
	double	ddlonx, ddlaty, rfactor;
	static double	dddepth = 0.0;
	int	setprint;
	int	i, j, k;
                
        /* get swath */
        swath = Ctrl->swath_plot;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                  %d\n",verbose);
		fprintf(stderr,"dbg2       Ctrl->A.mode:             %d\n", Ctrl->A.mode);
		fprintf(stderr,"dbg2       Ctrl->A.factor:           %f\n", Ctrl->A.factor);
		fprintf(stderr,"dbg2       Ctrl->A.depth:            %f\n",Ctrl->A.depth);
		fprintf(stderr,"dbg2       Ctrl->footprint_factor:   %f\n",Ctrl->footprint_factor);
		fprintf(stderr,"dbg2       Ctrl->swath_plot:         %p\n",Ctrl->swath_plot);
		fprintf(stderr,"dbg2       Ctrl->mtodeglon:          %f\n",Ctrl->mtodeglon);
		fprintf(stderr,"dbg2       Ctrl->mtodeglat:          %f\n",Ctrl->mtodeglat);
		fprintf(stderr,"dbg2       pings:                    %d\n",swath->npings);
		}

	/* set mode of operation */
	if (Ctrl->Z.mode != MBSWATH_SS
                && Ctrl->Z.mode != MBSWATH_SS_FILTER)
		{
		dobath = MB_YES;
		doss = MB_NO;
		}
	else
		{
		dobath = MB_NO;
		doss = MB_YES;
		}

	/* set all footprint flags to zero */
	for (i=0;i<swath->npings;i++)
		{
		pingcur = &swath->data[i];
		for (j=0;j<pingcur->beams_bath;j++)
			pingcur->bathflag[j] = MB_NO;
		for (j=0;j<pingcur->pixels_ss;j++)
			pingcur->ssflag[j] = MB_NO;
		}

	/* get fore-aft components of beam footprints */
	if (swath->npings > 1 && Ctrl->A.mode == MBSWATH_FOOTPRINT_FAKE)
	  {
	  for (i=0;i<swath->npings;i++)
		{
		/* initialize */
		pingcur = &swath->data[i];
		pingcur->lonaft = 0.0;
		pingcur->lataft = 0.0;
		pingcur->lonfor = 0.0;
		pingcur->latfor = 0.0;

		/* get aft looking */
		if (i > 0)
			{
			headingx = sin(pingcur->heading*DTR);
			headingy = cos(pingcur->heading*DTR);
			dx = (swath->data[i-1].navlon - pingcur->navlon)
				/Ctrl->mtodeglon;
			dy = (swath->data[i-1].navlat - pingcur->navlat)
				/Ctrl->mtodeglat;
			r = sqrt(dx*dx + dy*dy);
			pingcur->lonaft = Ctrl->footprint_factor * r * headingx * Ctrl->mtodeglon;
			pingcur->lataft = Ctrl->footprint_factor * r * headingy * Ctrl->mtodeglat;
			}

		/* get forward looking */
		if (i < swath->npings - 1)
			{
			headingx = sin(pingcur->heading*DTR);
			headingy = cos(pingcur->heading*DTR);
			dx = (swath->data[i+1].navlon - pingcur->navlon)
				/Ctrl->mtodeglon;
			dy = (swath->data[i+1].navlat - pingcur->navlat)
				/Ctrl->mtodeglat;
			r = sqrt(dx*dx + dy*dy);
			pingcur->lonfor = Ctrl->footprint_factor * r * headingx * Ctrl->mtodeglon;
			pingcur->latfor = Ctrl->footprint_factor * r * headingy * Ctrl->mtodeglat;
			}

		/* take care of first ping */
		if (i == 0)
			{
			pingcur->lonaft = -pingcur->lonfor;
			pingcur->lataft = -pingcur->latfor;
			}

		/* take care of last ping */
		if (i == swath->npings - 1)
			{
			pingcur->lonfor = -pingcur->lonaft;
			pingcur->latfor = -pingcur->lataft;
			}
		}
	  }

	/* take care of just one ping with nonzero center beam */
	else if (swath->npings == 1 && Ctrl->A.mode == MBSWATH_FOOTPRINT_FAKE
		&& mb_beam_ok(swath->data[0].beamflag[pingcur->beams_bath/2])
		&& Ctrl->A.depth <= 0.0)
	  {
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

	/* else get rfactor if using fore-aft beam width */
	else if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL)
	  {
	  rfactor = 0.5*sin(DTR*Ctrl->footprint_factor);
	  }

	/* loop over the inner beams and get
		the obvious footprint boundaries */
	for (i=0;i<swath->npings;i++)
		{
		pingcur = &swath->data[i];

		/* get heading if using fore-aft beam width */
		if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL)
			{
			headingx = sin(pingcur->heading*DTR);
			headingy = cos(pingcur->heading*DTR);
			}

		/* do bathymetry */
		if (dobath == MB_YES)
		  for (j=1;j<pingcur->beams_bath-1;j++)
		    if (mb_beam_ok(pingcur->beamflag[j]))
			{
			x = pingcur->bathlon[j];
			y = pingcur->bathlat[j];
			setprint = MB_NO;
			if (mb_beam_ok(pingcur->beamflag[j-1])
				&& mb_beam_ok(pingcur->beamflag[j+1]))
				{
				setprint = MB_YES;
				dlon1 = pingcur->bathlon[j-1]
					- pingcur->bathlon[j];
				dlat1 = pingcur->bathlat[j-1]
					- pingcur->bathlat[j];
				dlon2 = pingcur->bathlon[j+1]
					- pingcur->bathlon[j];
				dlat2 = pingcur->bathlat[j+1]
					- pingcur->bathlat[j];
				}
			else if (mb_beam_ok(pingcur->beamflag[j-1]))
				{
				setprint = MB_YES;
				dlon1 = pingcur->bathlon[j-1]
					- pingcur->bathlon[j];
				dlat1 = pingcur->bathlat[j-1]
					- pingcur->bathlat[j];
				dlon2 = -dlon1;
				dlat2 = -dlat1;
				}
			else if (mb_beam_ok(pingcur->beamflag[j+1]))
				{
				setprint = MB_YES;
				dlon2 = pingcur->bathlon[j+1]
					- pingcur->bathlon[j];
				dlat2 = pingcur->bathlat[j+1]
					- pingcur->bathlat[j];
				dlon1 = -dlon2;
				dlat1 = -dlat2;
				}

			/* do it using fore-aft beam width */
			if (setprint == MB_YES
				&& Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL
				&& Ctrl->A.depth <= 0.0)
				{
				print = &pingcur->bathfoot[j];
				pingcur->bathflag[j] = MB_YES;
				ddlonx = (pingcur->bathlon[j]
					- pingcur->navlon)/Ctrl->mtodeglon;
				ddlaty = (pingcur->bathlat[j]
					- pingcur->navlat)/Ctrl->mtodeglat;
				if (Ctrl->A.depth > 0.0)
					dddepth = Ctrl->A.depth;
				else if (pingcur->altitude > 0.0)
					dddepth = pingcur->altitude;
				else
					dddepth = pingcur->bath[j];
				r = rfactor*sqrt(ddlonx*ddlonx
					+ ddlaty*ddlaty
					+ dddepth*dddepth);
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

			/* do it using ping nav separation */
			else if (setprint == MB_YES)
				{
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

		/* do sidescan */
		if (doss == MB_YES)
		  for (j=1;j<pingcur->pixels_ss-1;j++)
		    if (pingcur->ss[j] > MB_SIDESCAN_NULL)
			{
			x = pingcur->sslon[j];
			y = pingcur->sslat[j];
			setprint = MB_NO;
			if (pingcur->ss[j-1] > MB_SIDESCAN_NULL
				&& pingcur->ss[j+1] > MB_SIDESCAN_NULL)
				{
				setprint = MB_YES;
				dlon1 = pingcur->sslon[j-1]
					- pingcur->sslon[j];
				dlat1 = pingcur->sslat[j-1]
					- pingcur->sslat[j];
				dlon2 = pingcur->sslon[j+1]
					- pingcur->sslon[j];
				dlat2 = pingcur->sslat[j+1]
					- pingcur->sslat[j];
				}
			else if (pingcur->ss[j-1] > MB_SIDESCAN_NULL)
				{
				setprint = MB_YES;
				dlon1 = pingcur->sslon[j-1]
					- pingcur->sslon[j];
				dlat1 = pingcur->sslat[j-1]
					- pingcur->sslat[j];
				dlon2 = -dlon1;
				dlat2 = -dlat1;
				}
			else if (pingcur->ss[j+1] > MB_SIDESCAN_NULL)
				{
				setprint = MB_YES;
				dlon2 = pingcur->sslon[j+1]
					- pingcur->sslon[j];
				dlat2 = pingcur->sslat[j+1]
					- pingcur->sslat[j];
				dlon1 = -dlon2;
				dlat1 = -dlat2;
				}

			/* do it using fore-aft beam width */
			if (setprint == MB_YES
				&& Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL)
				{
				print = &pingcur->ssfoot[j];
				pingcur->ssflag[j] = MB_YES;
				ddlonx = (pingcur->sslon[j]
					- pingcur->navlon)/Ctrl->mtodeglon;
				ddlaty = (pingcur->sslat[j]
					- pingcur->navlat)/Ctrl->mtodeglat;
				if (Ctrl->A.depth > 0.0)
					dddepth = Ctrl->A.depth;
				else if (pingcur->altitude > 0.0)
					dddepth = pingcur->altitude;
				else if (pingcur->beams_bath > 0
					&& mb_beam_ok(pingcur->beamflag[pingcur->beams_bath/2]))
					dddepth = pingcur->bath[pingcur->beams_bath/2];
				r = rfactor*sqrt(ddlonx*ddlonx
					+ ddlaty*ddlaty + dddepth*dddepth);
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

			/* do it using ping nav separation */
			else if (setprint == MB_YES)
				{
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

	/* loop over the outer beams and get
		the obvious footprint boundaries */
	for (i=0;i<swath->npings;i++)
		{
		pingcur = &swath->data[i];

		/* get heading if using fore-aft beam width */
		if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL)
			{
			headingx = sin(pingcur->heading*DTR);
			headingy = cos(pingcur->heading*DTR);
			}

		/* do bathymetry with more than 2 soundings */
		if (dobath == MB_YES && pingcur->beams_bath > 2)
		  {
		  j = 0;
		  if (mb_beam_ok(pingcur->beamflag[j])
			&& mb_beam_ok(pingcur->beamflag[j+1]))
			{
			x = pingcur->bathlon[j];
			y = pingcur->bathlat[j];
			dlon2 = pingcur->bathlon[j+1]
				- pingcur->bathlon[j];
			dlat2 = pingcur->bathlat[j+1]
				- pingcur->bathlat[j];
			dlon1 = -dlon2;
			dlat1 = -dlat2;
			print = &pingcur->bathfoot[j];
			pingcur->bathflag[j] = MB_YES;

			/* using fore-aft beam width */
			if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL)
				{
				ddlonx = (pingcur->bathlon[j]
					- pingcur->navlon)/Ctrl->mtodeglon;
				ddlaty = (pingcur->bathlat[j]
					- pingcur->navlat)/Ctrl->mtodeglat;
				if (Ctrl->A.depth > 0.0)
					dddepth = Ctrl->A.depth;
				else if (pingcur->altitude > 0.0)
					dddepth = pingcur->altitude;
				else
					dddepth = pingcur->bath[j];
				r = rfactor*sqrt(ddlonx*ddlonx
					+ ddlaty*ddlaty
					+ dddepth*dddepth);
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
		  if (mb_beam_ok(pingcur->beamflag[j])
			&& mb_beam_ok(pingcur->beamflag[j-1]))
			{
			x = pingcur->bathlon[j];
			y = pingcur->bathlat[j];
			dlon1 = pingcur->bathlon[j-1]
				- pingcur->bathlon[j];
			dlat1 = pingcur->bathlat[j-1]
				- pingcur->bathlat[j];
			dlon2 = -dlon1;
			dlat2 = -dlat1;
			print = &pingcur->bathfoot[j];
			pingcur->bathflag[j] = MB_YES;

			/* using fore-aft beam width */
			if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL)
				{
				ddlonx = (pingcur->bathlon[j]
					- pingcur->navlon)/Ctrl->mtodeglon;
				ddlaty = (pingcur->bathlat[j]
					- pingcur->navlat)/Ctrl->mtodeglat;
				if (Ctrl->A.depth > 0.0)
					dddepth = Ctrl->A.depth;
				else if (pingcur->altitude > 0.0)
					dddepth = pingcur->altitude;
				else
					dddepth = pingcur->bath[j];
				r = rfactor*sqrt(ddlonx*ddlonx
					+ ddlaty*ddlaty
					+ dddepth*dddepth);
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
		if (dobath == MB_YES && Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL && pingcur->beams_bath == 1)
		  {
		  if (mb_beam_ok(pingcur->beamflag[0]))
			{
			print = &pingcur->bathfoot[0];
			pingcur->bathflag[0] = MB_YES;
			ddlonx = (pingcur->bathlon[0]
				- pingcur->navlon)/Ctrl->mtodeglon;
			ddlaty = (pingcur->bathlat[0]
				- pingcur->navlat)/Ctrl->mtodeglat;
			if (Ctrl->A.depth > 0.0)
				dddepth = Ctrl->A.depth;
			else if (pingcur->altitude > 0.0)
				dddepth = pingcur->altitude;
			else
				dddepth = pingcur->bath[0];
			r = rfactor*sqrt(ddlonx*ddlonx
				+ ddlaty*ddlaty
				+ dddepth*dddepth);

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
		if (doss == MB_YES && pingcur->pixels_ss > 2)
		  {
		  j = 0;
		  if (pingcur->ss[j] > MB_SIDESCAN_NULL && pingcur->ss[j+1] > MB_SIDESCAN_NULL)
			{
			x = pingcur->sslon[j];
			y = pingcur->sslat[j];
			dlon2 = pingcur->sslon[j+1]
				- pingcur->sslon[j];
			dlat2 = pingcur->sslat[j+1]
				- pingcur->sslat[j];
			dlon1 = -dlon2;
			dlat1 = -dlat2;
			print = &pingcur->ssfoot[j];
			pingcur->ssflag[j] = MB_YES;

			/* using fore-aft beam width */
			if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL)
				{
				ddlonx = (pingcur->sslon[j]
					- pingcur->navlon)/Ctrl->mtodeglon;
				ddlaty = (pingcur->sslat[j]
					- pingcur->navlat)/Ctrl->mtodeglat;
				if (Ctrl->A.depth > 0.0)
					dddepth = Ctrl->A.depth;
				else if (pingcur->altitude > 0.0)
					dddepth = pingcur->altitude;
				else if (pingcur->beams_bath > 0
					&& mb_beam_ok(pingcur->beamflag[pingcur->beams_bath/2]))
					dddepth = pingcur->bath[pingcur->beams_bath/2];
				r = rfactor*sqrt(ddlonx*ddlonx
					+ ddlaty*ddlaty + dddepth*dddepth);
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
		  if (pingcur->ss[j] > MB_SIDESCAN_NULL && pingcur->ss[j-1] > MB_SIDESCAN_NULL)
			{
			x = pingcur->sslon[j];
			y = pingcur->sslat[j];
			dlon1 = pingcur->sslon[j-1]
				- pingcur->sslon[j];
			dlat1 = pingcur->sslat[j-1]
				- pingcur->sslat[j];
			dlon2 = -dlon1;
			dlat2 = -dlat1;
			print = &pingcur->ssfoot[j];
			pingcur->ssflag[j] = MB_YES;

			/* using fore-aft beam width */
			if (Ctrl->A.mode == MBSWATH_FOOTPRINT_REAL)
				{
				ddlonx = (pingcur->sslon[j]
					- pingcur->navlon)/Ctrl->mtodeglon;
				ddlaty = (pingcur->sslat[j]
					- pingcur->navlat)/Ctrl->mtodeglat;
				if (Ctrl->A.depth > 0.0)
					dddepth = Ctrl->A.depth;
				else if (pingcur->altitude > 0.0)
					dddepth = pingcur->altitude;
				else if (pingcur->beams_bath > 0
					&& mb_beam_ok(pingcur->beamflag[pingcur->beams_bath/2]))
					dddepth = pingcur->bath[pingcur->beams_bath/2];
				r = rfactor*sqrt(ddlonx*ddlonx
					+ ddlaty*ddlaty + dddepth*dddepth);
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

	/* print debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Beam footprints found in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2       npings:         %d\n", swath->npings);
		fprintf(stderr,"dbg2       error:          %d\n", *error);
		fprintf(stderr,"dbg2       status:         %d\n", status);
		for (i=0;i<swath->npings;i++)
			{
			fprintf(stderr,"dbg2\ndbg2       ping:           %d\n",i);
			pingcur = &swath->data[i];
			if (dobath == MB_YES)
			  for (j=0;j<pingcur->beams_bath;j++)
				{
				print = &pingcur->bathfoot[j];
				fprintf(stderr,"dbg2       %d  %d %g %g   ",
					j,pingcur->bathflag[j],
					pingcur->bathlon[j],
					pingcur->bathlat[j]);
				for (k=0;k<4;k++)
					fprintf(stderr,"  %g %g",
						print->x[k],
						print->y[k]);
				fprintf(stderr,"\n");
				}
			if (doss == MB_YES)
			  for (j=0;j<pingcur->pixels_ss;j++)
				{
				print = &pingcur->ssfoot[j];
				fprintf(stderr,"dbg2       %d  %d %g %g   ",
					j,pingcur->ssflag[j],
					pingcur->sslon[j],
					pingcur->sslat[j]);
				for (k=0;k<4;k++)
					fprintf(stderr,"  %g %g",
						print->x[k],
						print->y[k]);
				fprintf(stderr,"\n");
				}
			}
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbswath_get_shading(int verbose, struct MBSWATH_CTRL *Ctrl, struct GMT_CTRL *GMT,
                struct GMT_PALETTE *CPT, int *error)
{
	char	*function_name = "mbswath_get_shading";
	int	status = MB_SUCCESS;
        struct swath *swath;
	struct ping *ping0;
	struct ping *ping1;
	struct ping *ping2;
	int	drvcount;
	double	dx, dy, dd;
	double	dst2;
	double	drvx, drvy;
	double	sinx,cosy;
	double	median;
	double	graylevel;
        double  rgb[4];
        int     cpt_index;
	int	i, j;
                
        /* get swath */
        swath = Ctrl->swath_plot;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:            %d\n",verbose);
		fprintf(stderr,"dbg2       Ctrl:               %p\n",Ctrl);
		fprintf(stderr,"dbg2       Ctrl->Z.mode:       %d\n",Ctrl->Z.mode);
              if (Ctrl->Z.mode == MBSWATH_BATH_RELIEF)
                        {
                        fprintf(stderr,"dbg2       Ctrl->G.magnitude:  %f shaded relief magnitude\n",Ctrl->G.magnitude);
                        fprintf(stderr,"dbg2       Ctrl->G.azimuth:    %f shaded relief azimuth\n",Ctrl->G.azimuth);
                        }
                else if (Ctrl->Z.mode == MBSWATH_BATH_AMP)
                        {
                        fprintf(stderr,"dbg2       Ctrl->G.magnitude:  %f amplitude shading magnitude\n",Ctrl->G.magnitude);
                        fprintf(stderr,"dbg2       Ctrl->G.azimuth:    %f amplitude shading center\n",Ctrl->G.azimuth);
                        fprintf(stderr,"dbg2       Ctrl->N.active:     %d\n",Ctrl->N.active);
                        if (Ctrl->N.active)
                                fprintf(stderr,"dbg2       Ctrl->N.cptfile:    %s\n",Ctrl->N.cptfile);
                        }
		fprintf(stderr,"dbg2       GMT:                %p\n",GMT);
		fprintf(stderr,"dbg2       CPT:                %p\n",CPT);
		fprintf(stderr,"dbg2       swath:              %p\n",swath);
		fprintf(stderr,"dbg2       pings:              %d\n",swath->npings);
		fprintf(stderr,"dbg2       Ctrl->mtodeglon:          %f\n",Ctrl->mtodeglon);
		fprintf(stderr,"dbg2       Ctrl->mtodeglat:          %f\n",Ctrl->mtodeglat);
		}

	/* get shading from directional bathymetric gradient */
	if (Ctrl->Z.mode == MBSWATH_BATH_RELIEF)
	  {
	  /* get directional factors */
	  sinx = sin(DTR * Ctrl->G.azimuth);
	  cosy = cos(DTR * Ctrl->G.azimuth);

	  /* loop over the pings and beams */
	  for (i=0;i<swath->npings;i++)
	    {
	    if (i > 0) ping0 = &swath->data[i-1];
	    ping1 = &swath->data[i];
	    if (i < swath->npings - 1) ping2 = &swath->data[i+1];
	    for (j=0;j<ping1->beams_bath;j++)
	      if (mb_beam_ok(ping1->beamflag[j]))
		{
		/* do across track components */
		drvcount = 0;
		dx = 0.0;
		dy = 0.0;
		dd = 0.0;
		drvx = 0.0;
		drvy = 0.0;
		if (j > 0 && j < ping1->beams_bath - 1
			&& mb_beam_ok(ping1->beamflag[j-1])
			&& mb_beam_ok(ping1->beamflag[j+1]))
			{
			dx = (ping1->bathlon[j+1] - ping1->bathlon[j-1])
				 / Ctrl->mtodeglon;
			dy = (ping1->bathlat[j+1] - ping1->bathlat[j-1])
				 / Ctrl->mtodeglat;
			dd = ping1->bath[j+1] - ping1->bath[j-1];
			}
		else if (j < ping1->beams_bath - 1
			&& mb_beam_ok(ping1->beamflag[j])
			&& mb_beam_ok(ping1->beamflag[j+1]))
			{
			dx = (ping1->bathlon[j+1] - ping1->bathlon[j])
				 / Ctrl->mtodeglon;
			dy = (ping1->bathlat[j+1] - ping1->bathlat[j])
				 / Ctrl->mtodeglat;
			dd = ping1->bath[j+1] - ping1->bath[j];
			}
		else if (j > 0
			&& mb_beam_ok(ping1->beamflag[j-1])
			&& mb_beam_ok(ping1->beamflag[j]))
			{
			dx = (ping1->bathlon[j] - ping1->bathlon[j-1])
				 / Ctrl->mtodeglon;
			dy = (ping1->bathlat[j] - ping1->bathlat[j-1])
				 / Ctrl->mtodeglat;
			dd = ping1->bath[j] - ping1->bath[j-1];
			}
		dst2 = dx * dx + dy * dy;
		if (dst2 > 0.0)
			{
			drvx = dd * dx / dst2;
			drvy = dd * dy / dst2;
			drvcount++;
			}

		/* do along track components */
		dx = 0.0;
		dy = 0.0;
		dd = 0.0;
		if (i > 0 && i < swath->npings - 1
			&& mb_beam_ok(ping0->beamflag[j])
			&& mb_beam_ok(ping2->beamflag[j]))
			{
			dx = (ping2->bathlon[j] - ping0->bathlon[j])
				 / Ctrl->mtodeglon;
			dy = (ping2->bathlat[j] - ping0->bathlat[j])
				 / Ctrl->mtodeglat;
			dd = ping2->bath[j] - ping0->bath[j];
			}
		else if (i < swath->npings - 1
			&& mb_beam_ok(ping1->beamflag[j])
			&& mb_beam_ok(ping2->beamflag[j]))
			{
			dx = (ping2->bathlon[j] - ping1->bathlon[j])
				 / Ctrl->mtodeglon;
			dy = (ping2->bathlat[j] - ping1->bathlat[j])
				 / Ctrl->mtodeglat;
			dd = ping2->bath[j] - ping1->bath[j];
			}
		else if (i > 0
			&& mb_beam_ok(ping0->beamflag[j])
			&& mb_beam_ok(ping1->beamflag[j]))
			{
			dx = (ping1->bathlon[j] - ping0->bathlon[j])
				 / Ctrl->mtodeglon;
			dy = (ping1->bathlat[j] - ping0->bathlat[j])
				 / Ctrl->mtodeglat;
			dd = ping1->bath[j] - ping0->bath[j];
			}
		dst2 = dx * dx + dy * dy;
		if (dst2 > 0.0)
			{
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

	/* get shading from amplitude data using cpt file */
	else if (Ctrl->Z.mode == MBSWATH_BATH_AMP && Ctrl->N.active)
	  {
	  /* loop over the pings and beams */
	  for (i=0;i<swath->npings;i++)
	    {
	    if (i > 0) ping0 = &swath->data[i-1];
	    ping1 = &swath->data[i];
	    if (i < swath->npings - 1) ping2 = &swath->data[i+1];
	    for (j=0;j<ping1->beams_bath;j++)
	      if (mb_beam_ok(ping1->beamflag[j]))
		{
		/* calculate shading */
		if (mb_beam_ok(ping1->beamflag[j]))
			{
                        /* get shading value from cpt */
                        cpt_index = GMT_get_rgb_from_z(GMT, CPT, ping1->amp[j], rgb);
                        graylevel = (rgb[0] + rgb[1] + rgb[2]) / 3.0;
			ping1->bathshade[j] = Ctrl->G.magnitude * (graylevel - Ctrl->G.azimuth) / 128.;
			}
		else
			ping1->bathshade[j] = 0.0;
		}
	    }
	  }

	/* get shading from amplitude data */
	else if (Ctrl->Z.mode == MBSWATH_BATH_AMP)
	  {
	  /* get median value from value entered as azimuth */
	  median = Ctrl->G.azimuth;

	  /* loop over the pings and beams */
	  for (i=0;i<swath->npings;i++)
	    {
	    if (i > 0) ping0 = &swath->data[i-1];
	    ping1 = &swath->data[i];
	    if (i < swath->npings - 1) ping2 = &swath->data[i+1];
	    for (j=0;j<ping1->beams_bath;j++)
	      if (mb_beam_ok(ping1->beamflag[j]))
		{
		/* calculate shading */
		if (mb_beam_ok(ping1->beamflag[j]))
			ping1->bathshade[j] = Ctrl->G.magnitude * (ping1->amp[j] - Ctrl->G.azimuth);
		else
			ping1->bathshade[j] = 0.0;
		}
	    }
	  }

	/* print debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Shading values in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2       npings:         %d\n",
			swath->npings);
		fprintf(stderr,"dbg2       error:          %d\n",
			*error);
		fprintf(stderr,"dbg2       status:         %d\n",
			status);
		for (i=0;i<swath->npings;i++)
			{
			fprintf(stderr,"dbg2\ndbg2       ping:           %d\n",i);
			ping1 = &swath->data[i];
			for (j=0;j<ping1->beams_bath;j++)
				{
				fprintf(stderr,"dbg2       %d  %d  %g  %g\n",
					j,ping1->bathflag[j],
					ping1->bath[j],
					ping1->bathshade[j]);
				}
			}
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbswath_plot_data_footprint(int verbose, struct MBSWATH_CTRL *Ctrl, struct GMT_CTRL *GMT,
                        struct GMT_PALETTE *CPT, struct PSL_CTRL *PSL, 
                        int first, int nplot, int *error)
{
	char	*function_name = "mbswath_plot_data_footprint";
	int	status = MB_SUCCESS;
        struct swath *swath;
	struct ping *pingcur;
	struct footprint *print;
	double	*x, *y;
	double	xx[4], yy[4];
	double 	rgb[4];
        int     cpt_index;
	int	i, j, k;
                
        /* get swath */
        swath = Ctrl->swath_plot;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       Ctrl:               %p\n",Ctrl);
		fprintf(stderr,"dbg2       Ctrl->Z.mode:       %d\n",Ctrl->Z.mode);
		fprintf(stderr,"dbg2       Ctrl->C.cptfile:    %s\n",Ctrl->C.cptfile);
              if (Ctrl->Z.mode == MBSWATH_BATH_RELIEF)
                        {
                        fprintf(stderr,"dbg2       Ctrl->G.magnitude:  %f shaded relief magnitude\n",Ctrl->G.magnitude);
                        fprintf(stderr,"dbg2       Ctrl->G.azimuth:    %f shaded relief azimuth\n",Ctrl->G.azimuth);
                        }
                else if (Ctrl->Z.mode == MBSWATH_BATH_AMP)
                        {
                        fprintf(stderr,"dbg2       Ctrl->G.magnitude:  %f amplitude shading magnitude\n",Ctrl->G.magnitude);
                        fprintf(stderr,"dbg2       Ctrl->G.azimuth:    %f amplitude shading center\n",Ctrl->G.azimuth);
                        fprintf(stderr,"dbg2       Ctrl->N.active:     %d\n",Ctrl->N.active);
                        if (Ctrl->N.active)
                                fprintf(stderr,"dbg2       Ctrl->N.cptfile:    %s\n",Ctrl->N.cptfile);
                        }
		fprintf(stderr,"dbg2       GMT:                %p\n",GMT);
		fprintf(stderr,"dbg2       CPT:                %p\n",CPT);
		fprintf(stderr,"dbg2       PSL:                %p\n",PSL);
		fprintf(stderr,"dbg2       swath:      %p\n",swath);
		fprintf(stderr,"dbg2       pings:      %d\n",swath->npings);
		fprintf(stderr,"dbg2       first:      %d\n",first);
		fprintf(stderr,"dbg2       nplot:      %d\n",nplot);
		}

	if (Ctrl->Z.mode == MBSWATH_BATH
		|| Ctrl->Z.mode == MBSWATH_BATH_RELIEF
		|| Ctrl->Z.mode == MBSWATH_BATH_AMP)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->beams_bath;j++)
			  if (pingcur->bathflag[j] == MB_YES)
				{
				print = &pingcur->bathfoot[j];
				x = &(print->x[0]);
				y = &(print->y[0]);
				for (k=0;k<4;k++)
					GMT_geo_to_xy(GMT, x[k], y[k], &xx[k], &yy[k]);
                                cpt_index = GMT_get_rgb_from_z(GMT, CPT, pingcur->bath[j], rgb);
				if (Ctrl->Z.mode == MBSWATH_BATH_RELIEF
					|| Ctrl->Z.mode == MBSWATH_BATH_AMP)
                                        {
//fprintf(stderr,"Illuminate: shade:%f rgb: %f %f %f ",pingcur->bathshade[j], rgb[0],rgb[1],rgb[2]);
					GMT_illuminate(GMT, pingcur->bathshade[j], rgb);
//fprintf(stderr,"    %f %f %f\n",rgb[0],rgb[1],rgb[2]);
                                        }
/*fprintf(stderr,"Calling mbswath_plot_box ping:%d of %d   beam:%d of %d\n",
i,nplot,j,pingcur->beams_bath);*/
				status = mbswath_plot_box(verbose, Ctrl, GMT, PSL, xx, yy, rgb, error);
				}
			}
		}
	else if (Ctrl->Z.mode == MBSWATH_AMP)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->beams_amp;j++)
			  if (pingcur->bathflag[j] == MB_YES)
				{
				print = &pingcur->bathfoot[j];
				x = &(print->x[0]);
				y = &(print->y[0]);
				for (k=0;k<4;k++)
					GMT_geo_to_xy(GMT, x[k], y[k], &xx[k], &yy[k]);
                                cpt_index = GMT_get_rgb_from_z(GMT, CPT, pingcur->amp[j], rgb);
				status = mbswath_plot_box(verbose, Ctrl, GMT, PSL, xx, yy, rgb, error);
				}
			}
		}
	else if (Ctrl->Z.mode == MBSWATH_SS)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->pixels_ss;j++)
			  if (pingcur->ssflag[j] == MB_YES)
				{
				print = &pingcur->ssfoot[j];
				x = &(print->x[0]);
				y = &(print->y[0]);
				for (k=0;k<4;k++)
					GMT_geo_to_xy(GMT, x[k], y[k], &xx[k], &yy[k]);
                                cpt_index = GMT_get_rgb_from_z(GMT, CPT, pingcur->ss[j], rgb);
				status = mbswath_plot_box(verbose, Ctrl, GMT, PSL, xx, yy, rgb, error);
				}
			}
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbswath_plot_data_point(int verbose, struct MBSWATH_CTRL *Ctrl, struct GMT_CTRL *GMT,
		struct GMT_PALETTE *CPT, struct PSL_CTRL *PSL,
                int first, int nplot, int *error)
{
	char	*function_name = "mbswath_plot_data_point";
	int	status = MB_SUCCESS;
        struct swath *swath;
	struct ping	*pingcur;
	double	xx, yy;
	double	rgb[4];
        int     cpt_index;
	int	i, j;
                
        /* get swath */
        swath = Ctrl->swath_plot;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       Ctrl:               %p\n",Ctrl);
		fprintf(stderr,"dbg2       Ctrl->Z.mode:       %d\n",Ctrl->Z.mode);
              if (Ctrl->Z.mode == MBSWATH_BATH_RELIEF)
                        {
                        fprintf(stderr,"dbg2       Ctrl->G.magnitude:  %f shaded relief magnitude\n",Ctrl->G.magnitude);
                        fprintf(stderr,"dbg2       Ctrl->G.azimuth:    %f shaded relief azimuth\n",Ctrl->G.azimuth);
                        }
                else if (Ctrl->Z.mode == MBSWATH_BATH_AMP)
                        {
                        fprintf(stderr,"dbg2       Ctrl->G.magnitude:  %f amplitude shading magnitude\n",Ctrl->G.magnitude);
                        fprintf(stderr,"dbg2       Ctrl->G.azimuth:    %f amplitude shading center\n",Ctrl->G.azimuth);
                        fprintf(stderr,"dbg2       Ctrl->N.active:     %d\n",Ctrl->N.active);
                        if (Ctrl->N.active)
                                fprintf(stderr,"dbg2       Ctrl->N.cptfile:    %s\n",Ctrl->N.cptfile);
                        }
		fprintf(stderr,"dbg2       GMT:                %p\n",GMT);
		fprintf(stderr,"dbg2       CPT:                %p\n",CPT);
		fprintf(stderr,"dbg2       PSL:                %p\n",PSL);
		fprintf(stderr,"dbg2       swath:      %p\n",swath);
		fprintf(stderr,"dbg2       pings:      %d\n",swath->npings);
		fprintf(stderr,"dbg2       first:      %d\n",first);
		fprintf(stderr,"dbg2       nplot:      %d\n",nplot);
		}

	if (Ctrl->Z.mode == MBSWATH_BATH
		|| Ctrl->Z.mode == MBSWATH_BATH_RELIEF
		|| Ctrl->Z.mode == MBSWATH_BATH_AMP)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->beams_bath;j++)
			  if (mb_beam_ok(pingcur->beamflag[j]))
				{
				GMT_geo_to_xy(GMT, pingcur->bathlon[j], pingcur->bathlat[j],
					&xx, &yy);
                                cpt_index = GMT_get_rgb_from_z(GMT, CPT, pingcur->bath[j], rgb);
				if (Ctrl->Z.mode == MBSWATH_BATH_RELIEF
					|| Ctrl->Z.mode == MBSWATH_BATH_AMP)
					GMT_illuminate(GMT, pingcur->bathshade[j], rgb);
				status = mbswath_plot_point(verbose, Ctrl, GMT, PSL, xx, yy, rgb, error);
				}
			}
		}
	else if (Ctrl->Z.mode == MBSWATH_AMP)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->beams_amp;j++)
			  if (mb_beam_ok(pingcur->beamflag[j]))
				{
				GMT_geo_to_xy(GMT, pingcur->bathlon[j], pingcur->bathlat[j],
					&xx, &yy);
				cpt_index = GMT_get_rgb_from_z(GMT, CPT, pingcur->amp[j], rgb);
				status = mbswath_plot_point(verbose, Ctrl, GMT, PSL, xx, yy, rgb, error);
				}
			}
		}
	else if (Ctrl->Z.mode == MBSWATH_SS)
		{
		/* loop over all pings and beams and plot the good ones */
		for (i=first;i<first+nplot;i++)
			{
			pingcur = &swath->data[i];
			for (j=0;j<pingcur->pixels_ss;j++)
			  if (pingcur->ss[j] > MB_SIDESCAN_NULL)
				{
				GMT_geo_to_xy(GMT, pingcur->sslon[j], pingcur->sslat[j],
					&xx, &yy);
				cpt_index = GMT_get_rgb_from_z(GMT, CPT, pingcur->ss[j], rgb);
				status = mbswath_plot_point(verbose, Ctrl, GMT, PSL, xx, yy, rgb, error);
				}
			}
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbswath_plot_box(int verbose, struct MBSWATH_CTRL *Ctrl,
                        struct GMT_CTRL *GMT, struct PSL_CTRL *PSL,
                        double *x, double *y, double *rgb, int *error)
{
	char	*function_name = "mbswath_plot_box";
	int	status = MB_SUCCESS;
	int	ix[5], iy[5];
	int	ixmin, ixmax, iymin, iymax;
	int	ixx, iyy;
	int	ixx1, ixx2;
	double	dx, dy;
	int	ncross, xcross[10];
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:            %d\n",verbose);
		fprintf(stderr,"dbg2       GMT:                %p\n",GMT);
		fprintf(stderr,"dbg2       x[0]:               %f\n",x[0]);
		fprintf(stderr,"dbg2       y[0]:               %f\n",y[0]);
		fprintf(stderr,"dbg2       x[1]:               %f\n",x[1]);
		fprintf(stderr,"dbg2       y[1]:               %f\n",y[1]);
		fprintf(stderr,"dbg2       x[2]:               %f\n",x[2]);
		fprintf(stderr,"dbg2       y[2]:               %f\n",y[2]);
		fprintf(stderr,"dbg2       x[3]:               %f\n",x[3]);
		fprintf(stderr,"dbg2       y[3]:               %f\n",y[3]);
		fprintf(stderr,"dbg2       rgb[0]:             %f\n",rgb[0]);
		fprintf(stderr,"dbg2       rgb[1]:             %f\n",rgb[1]);
		fprintf(stderr,"dbg2       rgb[2]:             %f\n",rgb[2]);
		fprintf(stderr,"dbg2       rgb[3]:             %f\n",rgb[3]);
		}

	/* if simple case just plot polygon */
	if (Ctrl->image_type == MBSWATH_IMAGE_VECTOR)
		{
                PSL_setcolor(PSL, rgb, PSL_IS_FILL);
		PSL_plotpolygon(PSL, x, y, 4);
		}

	/* if image plot then rasterize the box */
	else if (Ctrl->image_type == MBSWATH_IMAGE_8 || Ctrl->image_type == MBSWATH_IMAGE_24)
		{
		/* get bounds of box in pixels */
		for (i=0;i<4;i++)
			{
			ix[i] = Ctrl->nx * x[i] / Ctrl->x_inch;
			iy[i] = Ctrl->ny * y[i] / Ctrl->y_inch;
			}
		ix[4] = ix[0];
		iy[4] = iy[0];

		/* get min max values of bounding corners in pixels */
		ixmin = ix[0];
		ixmax = ix[0];
		iymin = iy[0];
		iymax = iy[0];
		for (i=1;i<4;i++)
			{
			if (ix[i] < ixmin)
				ixmin = ix[i];
			if (ix[i] > ixmax)
				ixmax = ix[i];
			if (iy[i] < iymin)
				iymin = iy[i];
			if (iy[i] > iymax)
				iymax = iy[i];
			}
		if (ixmin < 0) ixmin = 0;
		if (ixmax > Ctrl->nx-1) ixmax = Ctrl->nx - 1;
		if (iymin < 1) iymin = 1;
		if (iymax > Ctrl->ny-1) iymax = Ctrl->ny - 1;

		/* loop over all y values */
		for (iyy=iymin;iyy<=iymax;iyy++)
		  {
		  /* find crossings */
		  ncross = 0;
		  for (i=0;i<4;i++)
		    {
		    if ((iy[i] <= iyy && iy[i+1] >= iyy)
		      || (iy[i] >= iyy && iy[i+1] <= iyy))
		      {
		      if (iy[i] == iy[i+1])
		        {
		        xcross[ncross] = ix[i];
		        ncross++;
		        xcross[ncross] = ix[i+1];
		        ncross++;
		        }
		      else
		        {
		        dy = iy[i+1] - iy[i];
		        dx = ix[i+1] - ix[i];
		        xcross[ncross] = (int) ((iyy - iy[i]) * dx / dy + ix[i]);
		        ncross++;
		        }
		      }
		    }

		  /* plot lines between crossings */
		  for (j=0;j<ncross-1;j++)
		    {
		    if (xcross[j] < xcross[j+1])
		      {
		      ixx1 = xcross[j];
		      ixx2 = xcross[j+1];
		      }
		    else
		      {
		      ixx1 = xcross[j+1];
		      ixx2 = xcross[j];
		      }
		    if ((ixx1 < ixmin && ixx2 < ixmin)
		      || (ixx1 > ixmax && ixx2 > ixmax))
		      ixx2 = ixx1 - 1; /* disable plotting */
		    else
		      {
		      if (ixx1 < ixmin) ixx1 = ixmin;
		      if (ixx2 > ixmax) ixx2 = ixmax;
		      }
		    for (ixx=ixx1;ixx<=ixx2;ixx++)
		      {
/*			fprintf(stderr,"plot %d %d\n",ixx,iyy);*/
		      if (Ctrl->image_type == MBSWATH_IMAGE_8)
			{
			k = Ctrl->nx * (Ctrl->ny - iyy) + ixx;
			Ctrl->bitimage[k] = (unsigned char) (255 * YIQ(rgb));
//fprintf(stderr,"k:%d rgb: %f %f %f   bitimage: %u\n",
//k,rgb[0],rgb[1],rgb[2],Ctrl->bitimage[k]);                        
			}
		      else
			{
			k = 3 * (Ctrl->nx * (Ctrl->ny - iyy) + ixx);
			Ctrl->bitimage[k] = (unsigned char) (255 * rgb[0]);
			Ctrl->bitimage[k+1] = (unsigned char) (255 * rgb[1]);
			Ctrl->bitimage[k+2] = (unsigned char) (255 * rgb[2]);
//fprintf(stderr,"k:%d rgb: %f %f %f   bitimage: %u %u %u\n",
//k,rgb[0],rgb[1],rgb[2],Ctrl->bitimage[k],Ctrl->bitimage[k+1],Ctrl->bitimage[k+2]);
			}
		      }
		    }
		  }
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbswath_plot_point(int verbose, struct MBSWATH_CTRL *Ctrl, struct GMT_CTRL *GMT, struct PSL_CTRL *PSL,
                        double x, double y, double *rgb, int *error)
{
	char	*function_name = "mbswath_plot_point";
	int	status = MB_SUCCESS;
        double  size = 0.005;
	int	ix, iy;
	int	k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:            %d\n",verbose);
		fprintf(stderr,"dbg2       GMT:                %p\n",GMT);
		fprintf(stderr,"dbg2       x:                  %f\n",x);
		fprintf(stderr,"dbg2       y:                  %f\n",y);
		fprintf(stderr,"dbg2       rgb[0]:             %f\n",rgb[0]);
		fprintf(stderr,"dbg2       rgb[1]:             %f\n",rgb[1]);
		fprintf(stderr,"dbg2       rgb[2]:             %f\n",rgb[2]);
		fprintf(stderr,"dbg2       rgb[3]:             %f\n",rgb[3]);
		}

	/* if simple case just plot point (well, a very small cross actually) */
	if (Ctrl->image_type == MBSWATH_IMAGE_VECTOR)
		{
                PSL_setcolor(PSL, rgb, PSL_IS_STROKE);
                PSL_plotsymbol(PSL, x, y, &size, PSL_CROSS);
		}

	/* if image plot then plot pixel */
	else
		{
		/* get pixel */
		ix = Ctrl->nx * x / Ctrl->x_inch;
		iy = Ctrl->ny * y / Ctrl->y_inch;

		/* plot pixel */
		if (Ctrl->image_type == MBSWATH_IMAGE_8)
			{
			k = Ctrl->nx * (Ctrl->ny - iy) + ix;
			Ctrl->bitimage[k] = (unsigned char ) YIQ(rgb);
			}
		else
			{
			k = 3 * (Ctrl->nx * (Ctrl->ny - iy) + ix);
			Ctrl->bitimage[k] = (unsigned char) (255 * rgb[0]);
			Ctrl->bitimage[k+1] = (unsigned char) (255 * rgb[1]);
			Ctrl->bitimage[k+2] = (unsigned char) (255 * rgb[2]);
			}
		  }

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbswath_ping_copy(int verbose, int one, int two, struct swath *swath, int *error)
{
	char	*function_name = "mbswath_ping_copy";
	int	status = MB_SUCCESS;

	struct ping	*ping1;
	struct ping	*ping2;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       one:        %d\n",one);
		fprintf(stderr,"dbg2       two:        %d\n",two);
		fprintf(stderr,"dbg2       swath:      %p\n",swath);
		fprintf(stderr,"dbg2       pings:      %d\n",swath->npings);
		}

	/* copy things */
	ping1 = &swath->data[one];
	ping2 = &swath->data[two];
	ping1->pings = ping2->pings;
	ping1->kind = ping2->kind;
	for (i=0;i<7;i++)
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
	for (i=0;i<ping1->beams_bath;i++)
		{
		ping1->beamflag[i] = ping2->beamflag[i];
		ping1->bath[i] = ping2->bath[i];
		ping1->bathlon[i] = ping2->bathlon[i];
		ping1->bathlat[i] = ping2->bathlat[i];
		ping1->bathflag[i] = ping2->bathflag[i];
		for (j=0;j<4;j++)
			{
			ping1->bathfoot[i].x[j] = ping2->bathfoot[i].x[j];
			ping1->bathfoot[i].y[j] = ping2->bathfoot[i].y[j];
			}
		}
	for (i=0;i<ping1->beams_amp;i++)
		{
		ping1->amp[i] = ping2->amp[i];
		}
	for (i=0;i<ping1->pixels_ss;i++)
		{
		ping1->ss[i] = ping2->ss[i];
		ping1->sslon[i] = ping2->sslon[i];
		ping1->sslat[i] = ping2->sslat[i];
		ping1->ssflag[i] = ping2->ssflag[i];
		for (j=0;j<4;j++)
			{
			ping1->ssfoot[i].x[j] = ping2->ssfoot[i].x[j];
			ping1->ssfoot[i].y[j] = ping2->ssfoot[i].y[j];
			}
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
