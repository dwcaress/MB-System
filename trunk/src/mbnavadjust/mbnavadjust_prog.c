/*--------------------------------------------------------------------
 *    The MB-system:	mbnavadjust_prog.c	3/23/00
 *    $Id$
 *
 *    Copyright (c) 2000-2015 by
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
 * mbnavadjust is an interactive navigation adjustment package
 * for swath sonar data.
 * It can work with any data format supported by the MBIO library.
 * This file contains the code that does not directly depend on the
 * MOTIF interface.
 *
 * Author:	D. W. Caress
 * Date:	March 23, 2000
 *
 *
 */

/*--------------------------------------------------------------------*/

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Intrinsic.h>

/* MBIO include files */
#include "mb_format.h"
#include "mb_status.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_aux.h"
#include "mbsys_ldeoih.h"
#include "mb_xgraphics.h"

/* define global control parameters */
#include "mbnavadjust.h"

/* swath bathymetry raw data structures */
struct	pingraw
	{
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	heading;
	double	draft;
	double	beams_bath;
	char	*beamflag;
	double	*bath;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	};
struct swathraw
	{
	/* raw swath data */
	int	file_id;
	int	npings;
	int	npings_max;
	int	beams_bath;
	struct pingraw *pingraws;
	};

/* id variables */
static char rcs_id[] = "$Id$";
static char program_name[] = "mbnavadjust";
static char help_message[] =  "mbnavadjust is an interactive navigation adjustment package for swath sonar data.\n";
static char usage_message[] = "mbnavadjust [-Iproject -V -H]";

/* status variables */
int	error = MB_ERROR_NO_ERROR;
char	*error_message;
char	message[STRING_MAX];
char	error1[STRING_MAX];
char	error2[STRING_MAX];
char	error3[STRING_MAX];

/* data file parameters */
void	*datalist;

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

/* route color defines (colors different in MBgrdviz than in MBnavadjust) */
#define	ROUTE_COLOR_BLACK			0
#define	ROUTE_COLOR_WHITE			1
#define	ROUTE_COLOR_RED			2
#define	ROUTE_COLOR_YELLOW		3
#define	ROUTE_COLOR_GREEN			4
#define	ROUTE_COLOR_BLUEGREEN		5
#define	ROUTE_COLOR_BLUE			6
#define	ROUTE_COLOR_PURPLE		7

/* color control values */
#define	WHITE	0
#define	BLACK	1
#define RED	2
#define GREEN	3
#define BLUE	4
#define CORAL	5
#define YELLOW	6
#define ORANGE	23
#define PURPLE	255

#define	XG_SOLIDLINE	0
#define	XG_DASHLINE	1
void	*pcont_xgid;
void	*pcorr_xgid;
void	*pzoff_xgid;
void	*pmodp_xgid;
int	ncolors;
int	pixel_values[256];

/* Set these to the dimensions of your canvas drawing */
/* areas, minus 1, located in the uil file             */
static int corr_borders[4];
static int cont_borders[4];
static int zoff_borders[4];
static int modp_borders[4];

/* mb_contour parameters */
struct swathraw *swathraw1 = NULL;
struct swathraw *swathraw2 = NULL;
struct swath *swath1 = NULL;
struct swath *swath2 = NULL;
struct ping *ping = NULL;

/* misfit grid parameters */
int	grid_nx = 0;
int	grid_ny = 0;
int	grid_nxy = 0;
int	grid_nxyzeq = 0;
double	grid_dx = 0.0;
double	grid_dy = 0.0;
double	grid_olon = 0.0;
double	grid_olat = 0.0;
double	misfit_min, misfit_max;
int	gridm_nx = 0;
int	gridm_ny = 0;
int	gridm_nxyz = 0;
double	*grid1 = NULL;
double	*grid2 = NULL;
double	*gridm = NULL;
double	*gridmeq = NULL;
int	*gridn1 = NULL;
int	*gridn2 = NULL;
int	*gridnm = NULL;
#define NINTERVALS_MISFIT 80
int	nmisfit_intervals = NINTERVALS_MISFIT;
double	misfit_intervals[NINTERVALS_MISFIT];
int	nzmisfitcalc;
double	zoff_dz;
double	zmin, zmax;
double	zmisfitmin;
double	zmisfitmax;

/* time, user, host variables */
time_t	right_now;
char	date[32], user[MBP_FILENAMESIZE], *user_ptr, host[MBP_FILENAMESIZE];

/* local prototypes */
int mbnavadjust_crossing_compare(const void *a, const void *b);
void mbnavadjust_plot(double xx,double yy,int ipen);
void mbnavadjust_newpen(int icolor);
void mbnavadjust_setline(int linewidth);
void mbnavadjust_justify_string(double height,char *string, double *s);
void mbnavadjust_plot_string(double x, double y, double hgt, double angle, char *label);

/*--------------------------------------------------------------------*/
int mbnavadjust_init_globals()
{
	/* local variables */
	char	*function_name = "mbnavadjust_init_globals";
	int	iformat;
	int	status = MB_SUCCESS;

	/* set default global control parameters */
	project.open = MB_NO;
	memset(project.name,0,STRING_MAX);
	strcpy(project.name,"None");
 	memset(project.path,0,STRING_MAX);
	memset(project.datadir,0,STRING_MAX);
	project.num_files = 0;
	project.num_files_alloc = 0;
	project.files = NULL;
	project.num_blocks = 0;
	project.num_snavs = 0;
	project.num_pings = 0;
	project.num_beams = 0;
	project.num_crossings = 0;
	project.num_crossings_alloc = 0;
 	project.num_crossings_analyzed = 0;
	project.num_goodcrossings = 0;
	project.num_truecrossings = 0;
 	project.num_truecrossings_analyzed = 0;
 	project.crossings = NULL;
	project.num_ties = 0;
	project.inversion = MBNA_INVERSION_NONE;
	project.modelplot = MB_NO;
	project.modelplot_style = MBNA_MODELPLOT_TIMESERIES;
	project.logfp = NULL;
 	mbna_status = MBNA_STATUS_GUI;
 	mbna_view_list = MBNA_VIEW_LIST_FILES;
 	mbna_view_mode = MBNA_VIEW_MODE_ALL;
	mbna_invert_mode = MBNA_INVERT_ZISOLATED;
	mbna_color_foreground = BLACK;
	mbna_color_background = WHITE;
 	project.section_length = 0.14;
 	project.section_soundings = 100000;
 	project.decimation = 1;
	project.precision = SIGMA_MINIMUM;
	project.smoothing = MBNA_SMOOTHING_DEFAULT;
	project.zoffsetwidth = 5.0;
	mbna_file_id_1 = MBNA_SELECT_NONE;
	mbna_section_1 = MBNA_SELECT_NONE;
	mbna_file_id_2 = MBNA_SELECT_NONE;
	mbna_section_2 = MBNA_SELECT_NONE;
 	mbna_current_crossing = MBNA_SELECT_NONE;
 	mbna_current_tie = MBNA_SELECT_NONE;
	mbna_naverr_load = MB_NO;
 	mbna_file_select = MBNA_SELECT_NONE;
 	mbna_survey_select = MBNA_SELECT_NONE;
 	mbna_section_select = MBNA_SELECT_NONE;
	mbna_crossing_select = MBNA_SELECT_NONE;
	mbna_tie_select = MBNA_SELECT_NONE;
	project.cont_int = 1.;
	project.col_int = 5.;
	project.tick_int = 5.;
	project.label_int = 100000.;
	mbna_contour_algorithm = MB_CONTOUR_OLD;
	/*mbna_contour_algorithm = MB_CONTOUR_TRIANGLES;*/
	mbna_ncolor = 10;
	mbna_contour = NULL;
	mbna_contour1.nvector = 0;
	mbna_contour1.nvector_alloc = 0;
	mbna_contour1.vector = NULL;
	mbna_contour2.nvector = 0;
	mbna_contour2.nvector_alloc = 0;
	mbna_contour2.vector = NULL;
	mbna_smoothweight = 100.0;
	mbna_offsetweight = 0.01;
	mbna_zweightfactor = 1.0;
	mbna_misfit_center = MBNA_MISFIT_AUTOCENTER;
	mbna_minmisfit_nthreshold = MBNA_MISFIT_NTHRESHOLD;
	mbna_minmisfit = 0.0;
	mbna_bias_mode = MBNA_BIAS_SAME;
	mbna_allow_set_tie = MB_NO;
	mbna_modelplot_zoom = MB_NO;
	mbna_modelplot_zoom_x1 = 0;
	mbna_modelplot_zoom_x2 = 0;
	mbna_modelplot_tiezoom = MB_NO;
	mbna_modelplot_tiestart = 0;
	mbna_modelplot_tieend = 0;
	mbna_modelplot_tiestartzoom = 0;
	mbna_modelplot_tieendzoom = 0;
	mbna_modelplot_pickfile = MBNA_SELECT_NONE;
	mbna_modelplot_picksection = MBNA_SELECT_NONE;
	mbna_modelplot_picksnav = MBNA_SELECT_NONE;
	mbna_modelplot_blocksurvey1 = MBNA_SELECT_NONE;
	mbna_modelplot_blocksurvey2 = MBNA_SELECT_NONE;
	mbna_reset_crossings = MB_NO;
	mbna_bin_swathwidth = 160.0;
	mbna_bin_pseudobeamwidth = 1.0;
	mbna_bin_beams_bath = mbna_bin_swathwidth / mbna_bin_pseudobeamwidth + 1;

	/* set mbio default values */
	status = mb_defaults(mbna_verbose,&iformat,&pings,&lonflip,bounds,
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

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
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
int mbnavadjust_init(int argc,char **argv)
{
	/* local variables */
	char	*function_name = "mbnavadjust_init";
	int	status = MB_SUCCESS;
	int	fileflag = 0;
	char	ifile[STRING_MAX];
	int	i;

	/* parsing variables */
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhDdI:i:Rr")) != -1)
	  switch (c)
		{
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			mbna_verbose++;
			break;
		case 'D':
		case 'd':
			mbna_color_foreground = WHITE;
			mbna_color_background = BLACK;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", ifile);
			flag++;
			fileflag++;
			break;
		case 'R':
		case 'r':
			mbna_reset_crossings = MB_YES;
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
	if (mbna_verbose == 1 || help)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       mbna_verbose:         %d\n",mbna_verbose);
		fprintf(stderr,"dbg2       help:            %d\n",help);
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
	if (mbna_verbose >= 2)
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
	        status = mbnavadjust_file_open(ifile);
    		do_update_status();
		}

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
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
int mbnavadjust_set_colors(int ncol, int *pixels)
{
	/* local variables */
	char	*function_name = "mbnavadjust_set_colors";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       ncolors:      %d\n",ncol);
		for (i=0;i<ncol;i++)
			fprintf(stderr,"dbg2       pixel[%d]:     %d\n",
				i, pixels[i]);
		}

	/* set colors */
	ncolors = ncol;
	for (i=0;i<ncolors;i++)
		pixel_values[i] = pixels[i];

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_set_borders(int *cn_brdr, int *cr_brdr, int *zc_brdr)
{
	/* local variables */
	char	*function_name = "mbnavadjust_set_borders";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       cn_brdr:      %d %d %d %d\n",
			cn_brdr[0], cn_brdr[1], cn_brdr[2], cn_brdr[3]);
		fprintf(stderr,"dbg2       cr_brdr:      %d %d %d %d\n",
			cr_brdr[0], cr_brdr[1], cr_brdr[2], cr_brdr[3]);
		fprintf(stderr,"dbg2       zc_brdr:      %d %d %d %d\n",
			zc_brdr[0], zc_brdr[1], zc_brdr[2], zc_brdr[3]);
		}

	/* set borders */
	for (i=0;i<4;i++)
		{
		cont_borders[i] = cn_brdr[i];
		corr_borders[i] = cr_brdr[i];
		zoff_borders[i] = zc_brdr[i];
		}

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_set_graphics(void *cn_xgid, void *cr_xgid, void *zc_xgid)
{
	/* local variables */
	char	*function_name = "mbnavadjust_set_graphics";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       cn_xgid:      %p\n",cn_xgid);
		fprintf(stderr,"dbg2       cr_xgid:      %p\n",cr_xgid);
		fprintf(stderr,"dbg2       zc_xgid:      %p\n",zc_xgid);
		}

	/* set graphics id */
	pcont_xgid = cn_xgid;
	pcorr_xgid = cr_xgid;
	pzoff_xgid = zc_xgid;

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_file_new(char *projectname)
{
	/* local variables */
	char	*function_name = "mbnavadjust_file_new";
	int	status = MB_SUCCESS;
	char	*slashptr, *nameptr;
	struct stat statbuf;

	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       projectname:  %s\n",projectname);
		}

	/* no new project if one already open */
	status = MB_SUCCESS;
	if (project.open == MB_YES)
		{
		strcpy(error1,"Unable to create new project!");
		sprintf(error2,"Project %s",project.name);
		strcpy(error3,"is already open.");
		status = MB_FAILURE;
		}

	/* get filenames and see if they can be generated */
	else
		{
		nameptr = (char *) NULL;
		slashptr = strrchr(projectname,'/');
		if (slashptr != (char *) NULL)
			nameptr = slashptr + 1;
		else
			nameptr = projectname;
		if (strlen(nameptr) > 4
			&& strcmp(&nameptr[strlen(nameptr)-4],".nvh") == 0)
			nameptr[strlen(nameptr)-4] = '\0';
		if (strlen(nameptr) > 0)
			{
			strcpy(project.name,nameptr);
			strncpy(project.path,projectname,
				strlen(projectname)-strlen(nameptr));
			strcpy(project.home,project.path);
			strcat(project.home,project.name);
			strcat(project.home,".nvh");
			strcpy(project.datadir,project.path);
			strcat(project.datadir,project.name);
			strcat(project.datadir,".dir");

			/* no new project if file or directory already exist */
			if (stat(project.home,&statbuf) == 0)
				{
				strcpy(error1,"Unable to create new project!");
				strcpy(error2,"Home file already exists.");
				strcpy(error3," ");
				if (stat(project.datadir,&statbuf) == 0)
				sprintf(error3,"Data directory already exists.");
				status = MB_FAILURE;
				}
			else if (stat(project.datadir,&statbuf) == 0)
				{
				strcpy(error1,"Unable to create new project!");
				strcpy(error2,"Data directory already exists.");
				strcpy(error3," ");
				status = MB_FAILURE;
				}

			/* initialize new project */
			else
				{
				/* set values */
				project.open = MB_YES;
				project.num_files = 0;
				project.num_files_alloc = 0;
				project.files = NULL;
				project.num_snavs = 0;
				project.num_pings = 0;
				project.num_beams = 0;
				project.num_crossings = 0;
				project.num_crossings_alloc = 0;
				project.num_crossings_analyzed = 0;
				project.num_goodcrossings = 0;
				project.num_truecrossings = 0;
				project.num_truecrossings_analyzed = 0;
				project.crossings = NULL;
				project.num_ties = 0;
				project.inversion = MBNA_INVERSION_NONE;
				project.precision = SIGMA_MINIMUM;
				project.smoothing = MBNA_SMOOTHING_DEFAULT;
				project.zoffsetwidth = 5.0;

				/* create data directory */
#ifdef WIN32
				if (_mkdir(project.datadir) != 0)
#else
				if (mkdir(project.datadir,00775) != 0)
#endif
					{
					strcpy(error1,"Unable to create new project!");
					strcpy(error2,"Error creating data directory.");
					strcpy(error3," ");
					status = MB_FAILURE;
					}

				/* write home file and other files */
				else if ((status = mbnavadjust_write_project()) == MB_FAILURE)
					{
					strcpy(error1,"Unable to create new project!");
					strcpy(error2,"Error writing data.");
					strcpy(error3," ");
					status = MB_FAILURE;
					}
				}
			}
		else
 		  	{
			strcpy(error1,"Unable to create new project!");
			strcpy(error2,"No project name was provided.");
			strcpy(error3," ");
			status = MB_FAILURE;
			}
		}

        /* display error message if needed */
	if (status == MB_FAILURE)
		{
		do_error_dialog(error1, error2, error3);
		sprintf(message, "%s\n > %s\n", error1, error2);
		do_info_add(message, MB_YES);
		}
	else
		{
		/* open log file */
		sprintf(message, "%s/log.txt", project.datadir);
		project.logfp = fopen(message, "w");

		/* add info text */
		sprintf(message,"New project initialized: %s\n > Project home: %s\n",
			project.name, project.home);
		do_info_add(message, MB_YES);
		if (project.logfp != NULL)
		    {
		    sprintf(message,"Log file %s/log.txt opened\n",
			project.datadir);
		    do_info_add(message, MB_YES);
		    }
		else
		    {
		    sprintf(message,"Unable to open log file %s/log.txt\n",
			project.datadir);
		    do_info_add(message, MB_YES);
		    }
		}

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_file_open(char *projectname)
{
	/* local variables */
	char	*function_name = "mbnavadjust_file_open";
	int	status = MB_SUCCESS;
	char	*slashptr, *nameptr, *bufptr;
	struct stat statbuf;

	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       projectname:  %s\n",projectname);
		}

	/* no new project if one already open */
	status = MB_SUCCESS;
	if (project.open == MB_YES)
		{
		strcpy(error1,"Unable to open project!");
		sprintf(error2,"Project %s",project.name);
		strcpy(error3,"is already open.");
		status = MB_FAILURE;
		}

	/* get filenames and see if they can be generated */
	else
		{
		nameptr = (char *) NULL;
		slashptr = strrchr(projectname,'/');
		if (slashptr != (char *) NULL)
			nameptr = slashptr + 1;
		else
			nameptr = projectname;
		if (strlen(nameptr) > 4
			&& strcmp(&nameptr[strlen(nameptr)-4],".nvh") == 0)
			nameptr[strlen(nameptr)-4] = '\0';
fprintf(stderr,"projectname:%s nameptr:%s strlen:%ld\n",projectname,nameptr,strlen(nameptr));
		if (strlen(nameptr) > 0)
			{
			strcpy(project.name,nameptr);
			if (slashptr != (char *) NULL)
				{
				strncpy(project.path,projectname,
					strlen(projectname)-strlen(nameptr));
				}
			else
				{
				bufptr = getcwd(project.path, MB_PATH_MAXLINE);
				strcat(project.path,"/");
				}
			strcpy(project.home,project.path);
			strcat(project.home,project.name);
			strcat(project.home,".nvh");
			strcpy(project.datadir,project.path);
			strcat(project.datadir,project.name);
			strcat(project.datadir,".dir");
fprintf(stderr,"In mbnavadjust_file_open: name:%s\npath:%s\nhome:%s\ndatadir:%s\n",
project.name,project.path,project.home,project.datadir);

			/* cant open unless file or directory already exist */
			if (stat(project.home,&statbuf) != 0)
				{
				strcpy(error1,"Unable to open project!");
				strcpy(error2,"Home file does not exist.");
				strcpy(error3," ");
				if (stat(project.datadir,&statbuf) != 0)
				sprintf(error3,"Data directory does not exist.");
				status = MB_FAILURE;
				}
			else if (stat(project.datadir,&statbuf) != 0)
				{
				strcpy(error1,"Unable to open project!");
				strcpy(error2,"Data directory does not exist.");
				strcpy(error3," ");
				status = MB_FAILURE;
				}

			/* open project */
			else
				{
				/* set values */
				project.num_files = 0;
				project.num_files_alloc = 0;
				project.files = NULL;
				project.num_snavs = 0;
				project.num_pings = 0;
				project.num_beams = 0;
				project.num_crossings = 0;
				project.num_crossings_alloc = 0;
				project.crossings = NULL;
				project.num_ties = 0;

				/* read home file and other files */
				if ((status = mbnavadjust_read_project()) == MB_FAILURE)
					{
					strcpy(error1,"Unable to open project!");
					strcpy(error2,"Error reading data.");
					strcpy(error3," ");
					status = MB_FAILURE;
					}
				}
			}
		else
 		  	{
			strcpy(error1,"Unable to open project!");
			strcpy(error2,"No project name was provided.");
			strcpy(error3," ");
			status = MB_FAILURE;
			}
		}

        /* display error message if needed */
	if (status == MB_FAILURE)
		{
		do_error_dialog(error1, error2, error3);
		sprintf(message, "%s\n > %s\n", error1, error2);
		do_info_add(message, MB_YES);
		}
	else
		{
		/* open log file */
		sprintf(message, "%s/log.txt", project.datadir);
		project.logfp = fopen(message, "a");

		/* add info text */
		sprintf(message,"Project opened: %s\n > Project home: %s\n > Number of Files: %d\n > Number of Crossings Found: %d\n > Number of Crossings Analyzed: %d\n > Number of Navigation Ties: %d\n",
			project.name, project.home, project.num_files, project.num_crossings,
			project.num_crossings_analyzed, project.num_ties);
		do_info_add(message, MB_YES);
		if (project.logfp != NULL)
		    {
		    sprintf(message,"Log file %s/log.txt opened\n",
			project.datadir);
		    do_info_add(message, MB_YES);
		    }
		else
		    {
		    sprintf(message,"Unable to open log file %s/log.txt\n",
			project.datadir);
		    do_info_add(message, MB_YES);
		    }
		}

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_close_project()
{
	/* local variables */
	char	*function_name = "mbnavadjust_close_project";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	int	i;

	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

        /* add info text */
	sprintf(message,"Project closed: %s\n", project.name);
	do_info_add(message, MB_YES);
	if (project.logfp != NULL)
	    {
	    sprintf(message,"Log file %s/log.txt closed\n",
		project.datadir);
	    do_info_add(message, MB_YES);
	    }

	/* deallocate memory and reset values */
	for (i=0;i<project.num_files;i++)
		{
		file = &project.files[i];
		if (file->sections != NULL)
			mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&file->sections,&error);
		}
	if (project.files != NULL)
		{
		free(project.files);
		project.files = NULL;
		project.num_files_alloc = 0;
		}
	if (project.crossings != NULL)
		{
		free(project.crossings);
		project.crossings = NULL;
		project.num_crossings_alloc = 0;
		}
	if (project.logfp != NULL)
		{
		fclose(project.logfp);
		project.logfp = NULL;
		}

	/* reset values */
	project.open = MB_NO;
	memset(project.name,0,STRING_MAX);
	strcpy(project.name,"None");
 	memset(project.path,0,STRING_MAX);
	memset(project.datadir,0,STRING_MAX);
	project.num_files = 0;
	project.num_snavs = 0;
	project.num_pings = 0;
	project.num_beams = 0;
	project.num_crossings = 0;
 	project.num_crossings_analyzed = 0;
	project.num_goodcrossings = 0;
	project.num_truecrossings = 0;
 	project.num_truecrossings_analyzed = 0;
	project.num_ties = 0;
 	project.inversion = MBNA_INVERSION_NONE;

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_write_project()
{
	/* local variables */
	char	*function_name = "mbnavadjust_write_project";
	int	status = MB_SUCCESS;
	FILE	*hfp, *xfp, *yfp;
	struct mbna_file *file, *file_1, *file_2;
	struct mbna_section *section, *section_1, *section_2;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	char	datalist[STRING_MAX];
	char	routefile[STRING_MAX];
	char	routename[STRING_MAX];
	char	xoffsetfile[STRING_MAX];
	char	yoffsetfile[STRING_MAX];
	double	navlon1, navlon2, navlat1, navlat2;
	int	nroute;
	int	snav_1, snav_2;
	int	ncrossings_true = 0;
	int	ncrossings_gt50 = 0;
	int	ncrossings_gt25 = 0;
	int	ncrossings_lt25 = 0;
	int	ncrossings_fixed = 0;
	int	nties_unfixed = 0;
	int	nties_fixed = 0;
	char	status_char, truecrossing_char;
	int	routecolor = 1;
	char	*unknown = "Unknown";

	int	i, j, k, l;

	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* open and write home file */
	if ((hfp = fopen(project.home,"w")) != NULL)
		{
fprintf(stderr,"Writing project %s\n", project.name);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			user_ptr = getenv("LOGNAME");
		if (user_ptr != NULL)
			strcpy(user,user_ptr);
		else
			strcpy(user, "unknown");
		gethostname(host,MBP_FILENAMESIZE);
		fprintf(hfp,"##MBNAVADJUST PROJECT\n");
		fprintf(hfp,"MB-SYSTEM_VERSION\t%s\n",MB_VERSION);
		fprintf(hfp,"PROGRAM_VERSION\t%s\n",rcs_id);
		fprintf(hfp,"FILE_VERSION\t3.06\n");
		fprintf(hfp,"ORIGIN\tGenerated by user <%s> on cpu <%s> at <%s>\n", user,host,date);
		fprintf(hfp,"NAME\t%s\n",project.name);
		fprintf(hfp,"PATH\t%s\n",project.path);
		fprintf(hfp,"HOME\t%s\n",project.home);
		fprintf(hfp,"DATADIR\t%s\n",project.datadir);
		fprintf(hfp,"NUMFILES\t%d\n",project.num_files);
		fprintf(hfp,"NUMBLOCKS\t%d\n",project.num_blocks);
		fprintf(hfp,"NUMCROSSINGS\t%d\n",project.num_crossings);
		fprintf(hfp,"SECTIONLENGTH\t%f\n",project.section_length);
		fprintf(hfp,"SECTIONSOUNDINGS\t%d\n",project.section_soundings);
		fprintf(hfp,"DECIMATION\t%d\n",project.decimation);
		fprintf(hfp,"CONTOURINTERVAL\t%f\n",project.cont_int);
		fprintf(hfp,"COLORINTERVAL\t%f\n",project.col_int);
		fprintf(hfp,"TICKINTERVAL\t%f\n",project.tick_int);
		fprintf(hfp,"INVERSION\t%d\n",project.inversion);
		fprintf(hfp,"SMOOTHING\t%f\n",project.smoothing);
		fprintf(hfp,"ZOFFSETWIDTH\t%f\n",project.zoffsetwidth);
		for (i=0;i<project.num_files;i++)
			{
			/* write out basic file info */
			file = &project.files[i];
			fprintf(hfp,"FILE %4d %4d %4d %4d %4d %13.8f %13.8f %13.8f %4.1f %4.1f %4.1f %4.1f %4d %4d %s\n",
					i,
					file->status,
					file->id,
					file->format,
					file->block,
					file->block_offset_x,
					file->block_offset_y,
					file->block_offset_z,
					file->heading_bias_import,
					file->roll_bias_import,
					file->heading_bias,
					file->roll_bias,
					file->num_sections,
					file->output_id,
					file->file);

			/* write out section info */
			for (j=0;j<file->num_sections;j++)
				{
				section = &file->sections[j];
				fprintf(hfp,"SECTION %4d %5d %5d %d %d %10.6f %16.6f %16.6f %13.8f %13.8f %13.8f %13.8f %9.3f %9.3f %d\n",
						j,
						section->num_pings,
						section->num_beams,
						section->num_snav,
						section->continuity,
						section->distance,
						section->btime_d,
						section->etime_d,
						section->lonmin,
						section->lonmax,
						section->latmin,
						section->latmax,
						section->depthmin,
						section->depthmax,
						section->contoursuptodate);
				for (k=MBNA_MASK_DIM-1;k>=0;k--)
				    {
				    for (l=0;l<MBNA_MASK_DIM;l++)
					{
					fprintf(hfp, "%1d", section->coverage[l+k*MBNA_MASK_DIM]);
					}
				    fprintf(hfp, "\n");
				    }
				for (k=0;k<section->num_snav;k++)
				    {
				    fprintf(hfp,"SNAV %4d %5d %10.6f %16.6f %13.8f %13.8f %13.8f %13.8f %13.8f\n",
						k,
						section->snav_id[k],
						section->snav_distance[k],
						section->snav_time_d[k],
						section->snav_lon[k],
						section->snav_lat[k],
						section->snav_lon_offset[k],
						section->snav_lat_offset[k],
						section->snav_z_offset[k]);
				    }
				fprintf(hfp,"GLOBALTIE %2d %4d %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f\n",
						section->global_tie_status,
						section->global_tie_snav,
						section->global_tie_offset_x,
						section->global_tie_offset_y,
						section->global_tie_offset_z_m,
						section->global_tie_xsigma,
						section->global_tie_ysigma,
						section->global_tie_zsigma);
				}
			}

		/* write out crossing info */
		for (i=0;i<project.num_crossings;i++)
			{
			/* write out basic crossing info */
			crossing = &project.crossings[i];
			fprintf(hfp,"CROSSING %5d %d %d %3d %5d %3d %5d %3d %2d\n",
				i,
				crossing->status,
				crossing->truecrossing,
				crossing->overlap,
				crossing->file_id_1,
				crossing->section_1,
				crossing->file_id_2,
				crossing->section_2,
				crossing->num_ties);

			/* write out tie info */
			for (j=0;j<crossing->num_ties;j++)
				{
				/* write out basic tie info */
				tie = &crossing->ties[j];
				fprintf(hfp,"TIE %5d %1d %5d %16.6f %5d %16.6f %13.8f %13.8f %13.8f %1.1d %13.8f %13.8f %13.8f\n",
					j,
					tie->status,
					tie->snav_1,
					tie->snav_1_time_d,
					tie->snav_2,
					tie->snav_2_time_d,
					tie->offset_x,
					tie->offset_y,
					tie->offset_z_m,
					tie->inversion_status,
					tie->inversion_offset_x,
					tie->inversion_offset_y,
					tie->inversion_offset_z_m);
				fprintf(hfp,"COV %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f\n",
					tie->sigmar1,
					tie->sigmax1[0],
					tie->sigmax1[1],
					tie->sigmax1[2],
					tie->sigmar2,
					tie->sigmax2[0],
					tie->sigmax2[1],
					tie->sigmax2[2],
					tie->sigmar3,
					tie->sigmax3[0],
					tie->sigmax3[1],
					tie->sigmax3[2]);
				}
			}

		/* close home file */
		fclose(hfp);
		status = MB_SUCCESS;
		}

	/* else set error */
	else
		{
		status = MB_FAILURE;
		sprintf(message,"Unable to update project %s\n > Home file: %s\n",
			project.name, project.home);
		do_info_add(message, MB_YES);
		}

	/* open and write datalist files */
	sprintf(datalist,"%s%s.mb-1",project.path,project.name);
	if ((hfp = fopen(datalist,"w")) != NULL)
		{
		for (i=0;i<project.num_files;i++)
			{
			/* write file entry */
			file = &project.files[i];
			fprintf(hfp,"%s %d\n", file->file, file->format);
			}
		fclose(hfp);
		}
	sprintf(datalist,"%s/%s.dir/datalist_unfixed.mb-1",project.path,project.name);
	if ((hfp = fopen(datalist,"w")) != NULL)
		{
		for (i=0;i<project.num_files;i++)
			{
			/* write file entry for each unfixed file */
			if (project.files[i].status != MBNA_FILE_FIXEDNAV)
				{
				file = &project.files[i];
				fprintf(hfp,"../%s %d\n", file->file, file->format);
				}
			}
		fclose(hfp);
		}
	sprintf(datalist,"%s/%s.dir/datalist_fixed.mb-1",project.path,project.name);
	if ((hfp = fopen(datalist,"w")) != NULL)
		{
		for (i=0;i<project.num_files;i++)
			{
			/* write file entry for each unfixed file */
			if (project.files[i].status == MBNA_FILE_FIXEDNAV)
				{
				file = &project.files[i];
				fprintf(hfp,"../%s %d\n", file->file, file->format);
				}
			}
		fclose(hfp);
		}

	/* write mbgrdviz route files in which each tie point or crossing is a two point route
		consisting of the connected snav points
		- output several different route files
		- route files of ties (fixed and unfixed separate) represent each tie as a
			two point route consisting of the connected snav points
		- route files of crossings (<25%, >= 25% && < 50%, >= 50%, true crossings)
			represent each crossing as a two point route consisting of the central
			snav points for each of the two sections.
		- first count different types of crossings and ties to output as routes
		- then output each time of route file */
	ncrossings_true = 0;
	ncrossings_gt50 = 0;
	ncrossings_gt25 = 0;
	ncrossings_lt25 = 0;
	ncrossings_fixed = 0;
	nties_unfixed = 0;
	nties_fixed = 0;
	for (i=0;i<project.num_crossings;i++)
		{
		crossing = &project.crossings[i];
		
		/* check all crossings */
		if (project.files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
			    || project.files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV)
			ncrossings_fixed++;
		else
			{
			if (crossing->truecrossing == MB_YES)
				ncrossings_true++;
			else if (crossing->overlap >= 50)
				ncrossings_gt50++;
			else if (crossing->overlap >= 25)
				ncrossings_gt25++;
			else
				ncrossings_lt25++;
			}
    
		/* check ties */
		if (crossing->status == MBNA_CROSSING_STATUS_SET)
			{
			if (project.files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
				|| project.files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV)
				nties_fixed += crossing->num_ties;
			else
				nties_unfixed += crossing->num_ties;
			}
		}		

	/* write mbgrdviz route file for all unfixed true crossings */
	sprintf(routefile,"%s%s_truecrossing.rte",project.path,project.name);
	if ((hfp = fopen(routefile,"w")) == NULL)
		{
		fclose(hfp);
		status = MB_FAILURE;
		error = MB_ERROR_OPEN_FAIL;
		sprintf(message, " > Unable to open output tie route file %s\n", routefile);
		do_info_add(message, MB_NO);
		if (mbna_verbose == 0)
			fprintf(stderr,"%s",message);
		}
	else
		{
		/* write the route file header */
		fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
		fprintf(hfp, "## Output by Program %s\n",program_name);
		fprintf(hfp, "## Program Version %s\n",rcs_id);
		fprintf(hfp, "## MB-System Version %s\n",MB_VERSION);
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			if ((user_ptr = getenv("LOGNAME")) == NULL)
				user_ptr = unknown;
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n",
			user_ptr,host,date);
		fprintf(hfp, "## Number of routes: %d\n",ncrossings_true);
		fprintf(hfp, "## Route point format:\n");
		fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

		/* loop over all crossings */
		nroute = 0;
		for (i=0;i<project.num_crossings;i++)
			{
			crossing = &project.crossings[i];
    
			/* output only unfixed true crossings */
			if (crossing->truecrossing == MB_YES
			    && !(project.files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
				    || project.files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV))
				{
				file_1 = (struct mbna_file *) &project.files[crossing->file_id_1];
				file_2 = (struct mbna_file *) &project.files[crossing->file_id_2];
				section_1 = (struct mbna_section *) &file_1->sections[crossing->section_1];
				section_2 = (struct mbna_section *) &file_2->sections[crossing->section_2];
				snav_1 = section_1->num_snav/2;
				snav_2 = section_2->num_snav/2;
				navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
				navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
				navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
				navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
				if (crossing->status == MBNA_CROSSING_STATUS_NONE)
					{
					status_char = 'U';
					routecolor = ROUTE_COLOR_YELLOW;
					}
				else if (crossing->status == MBNA_CROSSING_STATUS_SET)
					{
					status_char = '*';
					routecolor = ROUTE_COLOR_GREEN;
					}
				else
					{
					status_char = '-';
					routecolor = ROUTE_COLOR_RED;
					}
				if (crossing->truecrossing == MB_NO)
					truecrossing_char = ' ';
				else
					truecrossing_char = 'X';
				sprintf(routename,"%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d",
					status_char, truecrossing_char, i,
					file_1->block,
					crossing->file_id_1,
					crossing->section_1,
					file_2->block,
					crossing->file_id_2,
					crossing->section_2,
					crossing->overlap,
					crossing->num_ties);
				fprintf(hfp,"## ROUTENAME %s\n", routename);
				fprintf(hfp,"## ROUTESIZE %d\n", 1);
				fprintf(hfp,"## ROUTECOLOR %d\n", routecolor);
				fprintf(hfp,"## ROUTEPOINTS %d\n", 2);
				fprintf(hfp,"## ROUTEEDITMODE %d\n", MB_NO);
				fprintf(hfp,"> ## STARTROUTE\n");
				fprintf(hfp,"%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n",
					navlon1,navlat1,navlon2,navlat2);
				nroute++;
				}
			}
		fclose(hfp);
fprintf(stderr,"Output %d (expected %d) true crossing locations to %s\n", nroute, ncrossings_true, routefile);
		}

	/* write mbgrdviz route file for all unfixed >=50% crossings */
	sprintf(routefile,"%s%s_gt50crossing.rte",project.path,project.name);
	if ((hfp = fopen(routefile,"w")) == NULL)
		{
		fclose(hfp);
		status = MB_FAILURE;
		error = MB_ERROR_OPEN_FAIL;
		sprintf(message, " > Unable to open output tie route file %s\n", routefile);
		do_info_add(message, MB_NO);
		if (mbna_verbose == 0)
			fprintf(stderr,"%s",message);
		}
	else
		{
		/* write the route file header */
		fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
		fprintf(hfp, "## Output by Program %s\n",program_name);
		fprintf(hfp, "## Program Version %s\n",rcs_id);
		fprintf(hfp, "## MB-System Version %s\n",MB_VERSION);
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			if ((user_ptr = getenv("LOGNAME")) == NULL)
				user_ptr = unknown;
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n",
			user_ptr,host,date);
		fprintf(hfp, "## Number of routes: %d\n",ncrossings_gt50);
		fprintf(hfp, "## Route point format:\n");
		fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

		/* loop over all crossings */
		nroute = 0;
		for (i=0;i<project.num_crossings;i++)
			{
			crossing = &project.crossings[i];
   
			/* output only unfixed >=50% crossings */
			if (crossing->overlap >= 50
			    && !(project.files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
				    || project.files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV))
				{
				file_1 = (struct mbna_file *) &project.files[crossing->file_id_1];
				file_2 = (struct mbna_file *) &project.files[crossing->file_id_2];
				section_1 = (struct mbna_section *) &file_1->sections[crossing->section_1];
				section_2 = (struct mbna_section *) &file_2->sections[crossing->section_2];
				snav_1 = section_1->num_snav/2;
				snav_2 = section_2->num_snav/2;
				navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
				navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
				navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
				navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
				if (crossing->status == MBNA_CROSSING_STATUS_NONE)
					{
					status_char = 'U';
					routecolor = ROUTE_COLOR_YELLOW;
					}
				else if (crossing->status == MBNA_CROSSING_STATUS_SET)
					{
					status_char = '*';
					routecolor = ROUTE_COLOR_GREEN;
					}
				else
					{
					status_char = '-';
					routecolor = ROUTE_COLOR_RED;
					}
				if (crossing->truecrossing == MB_NO)
					truecrossing_char = ' ';
				else
					truecrossing_char = 'X';
				sprintf(routename,"%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d",
					status_char, truecrossing_char, i,
					file_1->block,
					crossing->file_id_1,
					crossing->section_1,
					file_2->block,
					crossing->file_id_2,
					crossing->section_2,
					crossing->overlap,
					crossing->num_ties);
				fprintf(hfp,"## ROUTENAME %s\n", routename);
				fprintf(hfp,"## ROUTESIZE %d\n", 1);
				fprintf(hfp,"## ROUTECOLOR %d\n", routecolor);
				fprintf(hfp,"## ROUTEPOINTS %d\n", 2);
				fprintf(hfp,"## ROUTEEDITMODE %d\n", MB_NO);
				fprintf(hfp,"> ## STARTROUTE\n");
				fprintf(hfp,"%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n",
					navlon1,navlat1,navlon2,navlat2);
				nroute++;
				}
			}
		fclose(hfp);
fprintf(stderr,"Output %d (expected %d) >=50%% overlap crossing locations to %s\n", nroute, ncrossings_gt50, routefile);
		}

	/* write mbgrdviz route file for all unfixed >=25% but less than 50% crossings */
	sprintf(routefile,"%s%s_gt25crossing.rte",project.path,project.name);
	if ((hfp = fopen(routefile,"w")) == NULL)
		{
		fclose(hfp);
		status = MB_FAILURE;
		error = MB_ERROR_OPEN_FAIL;
		sprintf(message, " > Unable to open output tie route file %s\n", routefile);
		do_info_add(message, MB_NO);
		if (mbna_verbose == 0)
			fprintf(stderr,"%s",message);
		}
	else
		{
		/* write the route file header */
		fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
		fprintf(hfp, "## Output by Program %s\n",program_name);
		fprintf(hfp, "## Program Version %s\n",rcs_id);
		fprintf(hfp, "## MB-System Version %s\n",MB_VERSION);
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			if ((user_ptr = getenv("LOGNAME")) == NULL)
				user_ptr = unknown;
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n",
			user_ptr,host,date);
		fprintf(hfp, "## Number of routes: %d\n",ncrossings_gt25);
		fprintf(hfp, "## Route point format:\n");
		fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

		/* loop over all crossings */
		nroute = 0;
		for (i=0;i<project.num_crossings;i++)
			{
			crossing = &project.crossings[i];
    
			/* output only unfixed >=25% but less than 50% crossings crossings */
			if (crossing->overlap >= 25
			    && !(project.files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
				    || project.files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV))
				{
				file_1 = (struct mbna_file *) &project.files[crossing->file_id_1];
				file_2 = (struct mbna_file *) &project.files[crossing->file_id_2];
				section_1 = (struct mbna_section *) &file_1->sections[crossing->section_1];
				section_2 = (struct mbna_section *) &file_2->sections[crossing->section_2];
				snav_1 = section_1->num_snav/2;
				snav_2 = section_2->num_snav/2;
				navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
				navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
				navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
				navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
				if (crossing->status == MBNA_CROSSING_STATUS_NONE)
					{
					status_char = 'U';
					routecolor = ROUTE_COLOR_YELLOW;
					}
				else if (crossing->status == MBNA_CROSSING_STATUS_SET)
					{
					status_char = '*';
					routecolor = ROUTE_COLOR_GREEN;
					}
				else
					{
					status_char = '-';
					routecolor = ROUTE_COLOR_RED;
					}
				if (crossing->truecrossing == MB_NO)
					truecrossing_char = ' ';
				else
					truecrossing_char = 'X';
				sprintf(routename,"%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d",
					status_char, truecrossing_char, i,
					file_1->block,
					crossing->file_id_1,
					crossing->section_1,
					file_2->block,
					crossing->file_id_2,
					crossing->section_2,
					crossing->overlap,
					crossing->num_ties);
				fprintf(hfp,"## ROUTENAME %s\n", routename);
				fprintf(hfp,"## ROUTESIZE %d\n", 1);
				fprintf(hfp,"## ROUTECOLOR %d\n", routecolor);
				fprintf(hfp,"## ROUTEPOINTS %d\n", 2);
				fprintf(hfp,"## ROUTEEDITMODE %d\n", MB_NO);
				fprintf(hfp,"> ## STARTROUTE\n");
				fprintf(hfp,"%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n",
					navlon1,navlat1,navlon2,navlat2);
				nroute++;
				}
			}
		fclose(hfp);
fprintf(stderr,"Output %d (expected %d) >=25%% && < 50%% overlap crossing locations to %s\n", nroute, ncrossings_gt25, routefile);
		}

	/* write mbgrdviz route file for all unfixed <25% crossings */
	sprintf(routefile,"%s%s_lt25crossing.rte",project.path,project.name);
	if ((hfp = fopen(routefile,"w")) == NULL)
		{
		fclose(hfp);
		status = MB_FAILURE;
		error = MB_ERROR_OPEN_FAIL;
		sprintf(message, " > Unable to open output tie route file %s\n", routefile);
		do_info_add(message, MB_NO);
		if (mbna_verbose == 0)
			fprintf(stderr,"%s",message);
		}
	else
		{
		/* write the route file header */
		fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
		fprintf(hfp, "## Output by Program %s\n",program_name);
		fprintf(hfp, "## Program Version %s\n",rcs_id);
		fprintf(hfp, "## MB-System Version %s\n",MB_VERSION);
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			if ((user_ptr = getenv("LOGNAME")) == NULL)
				user_ptr = unknown;
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n",
			user_ptr,host,date);
		fprintf(hfp, "## Number of routes: %d\n",ncrossings_lt25);
		fprintf(hfp, "## Route point format:\n");
		fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

		/* loop over all crossings */
		nroute = 0;
		for (i=0;i<project.num_crossings;i++)
			{
			crossing = &project.crossings[i];
    
			/* output only unfixed <25% crossings crossings */
			if (crossing->overlap < 25
			    && !(project.files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
				    || project.files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV))
				{
				file_1 = (struct mbna_file *) &project.files[crossing->file_id_1];
				file_2 = (struct mbna_file *) &project.files[crossing->file_id_2];
				section_1 = (struct mbna_section *) &file_1->sections[crossing->section_1];
				section_2 = (struct mbna_section *) &file_2->sections[crossing->section_2];
				snav_1 = section_1->num_snav/2;
				snav_2 = section_2->num_snav/2;
				navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
				navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
				navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
				navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
				if (crossing->status == MBNA_CROSSING_STATUS_NONE)
					{
					status_char = 'U';
					routecolor = ROUTE_COLOR_YELLOW;
					}
				else if (crossing->status == MBNA_CROSSING_STATUS_SET)
					{
					status_char = '*';
					routecolor = ROUTE_COLOR_GREEN;
					}
				else
					{
					status_char = '-';
					routecolor = ROUTE_COLOR_RED;
					}
				if (crossing->truecrossing == MB_NO)
					truecrossing_char = ' ';
				else
					truecrossing_char = 'X';
				sprintf(routename,"%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d",
					status_char, truecrossing_char, i,
					file_1->block,
					crossing->file_id_1,
					crossing->section_1,
					file_2->block,
					crossing->file_id_2,
					crossing->section_2,
					crossing->overlap,
					crossing->num_ties);
				fprintf(hfp,"## ROUTENAME %s\n", routename);
				fprintf(hfp,"## ROUTESIZE %d\n", 1);
				fprintf(hfp,"## ROUTECOLOR %d\n", routecolor);
				fprintf(hfp,"## ROUTEPOINTS %d\n", 2);
				fprintf(hfp,"## ROUTEEDITMODE %d\n", MB_NO);
				fprintf(hfp,"> ## STARTROUTE\n");
				fprintf(hfp,"%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n",
					navlon1,navlat1,navlon2,navlat2);
				nroute++;
				}
			}
		fclose(hfp);
fprintf(stderr,"Output %d (expected %d) <25%% overlap crossing locations to %s\n", nroute, ncrossings_lt25, routefile);
		}

	/* write mbgrdviz route file for all fixed crossings */
	sprintf(routefile,"%s%s_fixedcrossing.rte",project.path,project.name);
	if ((hfp = fopen(routefile,"w")) == NULL)
		{
		fclose(hfp);
		status = MB_FAILURE;
		error = MB_ERROR_OPEN_FAIL;
		sprintf(message, " > Unable to open output fixed crossings route file %s\n", routefile);
		do_info_add(message, MB_NO);
		if (mbna_verbose == 0)
			fprintf(stderr,"%s",message);
		}
	else
		{
		/* write the route file header */
		fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
		fprintf(hfp, "## Output by Program %s\n",program_name);
		fprintf(hfp, "## Program Version %s\n",rcs_id);
		fprintf(hfp, "## MB-System Version %s\n",MB_VERSION);
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			if ((user_ptr = getenv("LOGNAME")) == NULL)
				user_ptr = unknown;
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n",
			user_ptr,host,date);
		fprintf(hfp, "## Number of routes: %d\n",ncrossings_fixed);
		fprintf(hfp, "## Route point format:\n");
		fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

		/* loop over all crossings */
		nroute = 0;
		for (i=0;i<project.num_crossings;i++)
			{
			crossing = &project.crossings[i];
  
			/* output only fixed crossings */
			if (project.files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
				    || project.files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV)
				{
				file_1 = (struct mbna_file *) &project.files[crossing->file_id_1];
				file_2 = (struct mbna_file *) &project.files[crossing->file_id_2];
				section_1 = (struct mbna_section *) &file_1->sections[crossing->section_1];
				section_2 = (struct mbna_section *) &file_2->sections[crossing->section_2];
				snav_1 = section_1->num_snav/2;
				snav_2 = section_2->num_snav/2;
				navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
				navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
				navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
				navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
				if (crossing->status == MBNA_CROSSING_STATUS_NONE)
					{
					status_char = 'U';
					routecolor = ROUTE_COLOR_YELLOW;
					}
				else if (crossing->status == MBNA_CROSSING_STATUS_SET)
					{
					status_char = '*';
					routecolor = ROUTE_COLOR_GREEN;
					}
				else
					{
					status_char = '-';
					routecolor = ROUTE_COLOR_RED;
					}
				if (crossing->truecrossing == MB_NO)
					truecrossing_char = ' ';
				else
					truecrossing_char = 'X';
				sprintf(routename,"%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d",
					status_char, truecrossing_char, i,
					file_1->block,
					crossing->file_id_1,
					crossing->section_1,
					file_2->block,
					crossing->file_id_2,
					crossing->section_2,
					crossing->overlap,
					crossing->num_ties);
				fprintf(hfp,"## ROUTENAME %s\n", routename);
				fprintf(hfp,"## ROUTESIZE %d\n", 1);
				fprintf(hfp,"## ROUTECOLOR %d\n", routecolor);
				fprintf(hfp,"## ROUTEPOINTS %d\n", 2);
				fprintf(hfp,"## ROUTEEDITMODE %d\n", MB_NO);
				fprintf(hfp,"> ## STARTROUTE\n");
				fprintf(hfp,"%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n",
					navlon1,navlat1,navlon2,navlat2);
				nroute++;
				}
			}
		fclose(hfp);
fprintf(stderr,"Output %d (expected %d) fixed crossing locations to %s\n", nroute, ncrossings_fixed, routefile);
		}

	/* write mbgrdviz route file for all unfixed ties */
	sprintf(routefile,"%s%s_unfixedties.rte",project.path,project.name);
	if ((hfp = fopen(routefile,"w")) == NULL)
		{
		fclose(hfp);
		status = MB_FAILURE;
		error = MB_ERROR_OPEN_FAIL;
		sprintf(message, " > Unable to open output unfixed ties route file %s\n", routefile);
		do_info_add(message, MB_NO);
		if (mbna_verbose == 0)
			fprintf(stderr,"%s",message);
		}
	else
		{
		/* write the route file header */
		fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
		fprintf(hfp, "## Output by Program %s\n",program_name);
		fprintf(hfp, "## Program Version %s\n",rcs_id);
		fprintf(hfp, "## MB-System Version %s\n",MB_VERSION);
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			if ((user_ptr = getenv("LOGNAME")) == NULL)
				user_ptr = unknown;
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n",
			user_ptr,host,date);
		fprintf(hfp, "## Number of routes: %d\n",nties_unfixed);
		fprintf(hfp, "## Route point format:\n");
		fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");
		routecolor = ROUTE_COLOR_BLUEGREEN;

		/* loop over all crossings */
		nroute = 0;
		for (i=0;i<project.num_crossings;i++)
			{
			crossing = &project.crossings[i];
   
			/* output only unfixed ties */
			if (crossing->status == MBNA_CROSSING_STATUS_SET
				&& !(project.files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
					|| project.files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV))
				{
				for (j=0;j<crossing->num_ties;j++)
					{
					file_1 = (struct mbna_file *) &project.files[crossing->file_id_1];
					file_2 = (struct mbna_file *) &project.files[crossing->file_id_2];
					section_1 = (struct mbna_section *) &file_1->sections[crossing->section_1];
					section_2 = (struct mbna_section *) &file_2->sections[crossing->section_2];
					tie = (struct mbna_tie *) &crossing->ties[j];
					snav_1 = tie->snav_1;
					snav_2 = tie->snav_2;
					navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
					navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
					navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
					navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
					if (crossing->status == MBNA_CROSSING_STATUS_NONE)
						status_char = 'U';
					else if (crossing->status == MBNA_CROSSING_STATUS_SET)
						status_char = '*';
					else
						status_char = '-';
					if (crossing->truecrossing == MB_NO)
						truecrossing_char = ' ';
					else
						truecrossing_char = 'X';
					sprintf(routename,"Tie: %c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d of %2d",
						status_char, truecrossing_char, i,
						file_1->block,
						crossing->file_id_1,
						crossing->section_1,
						file_2->block,
						crossing->file_id_2,
						crossing->section_2,
						crossing->overlap,
						j, crossing->num_ties);
					fprintf(hfp,"## ROUTENAME %s\n", routename);
					fprintf(hfp,"## ROUTESIZE %d\n", 1);
					fprintf(hfp,"## ROUTECOLOR %d\n", routecolor);
					fprintf(hfp,"## ROUTEPOINTS %d\n", 2);
					fprintf(hfp,"## ROUTEEDITMODE %d\n", MB_NO);
					fprintf(hfp,"> ## STARTROUTE\n");
					fprintf(hfp,"%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n",
						navlon1,navlat1,navlon2,navlat2);
					nroute++;
					}
				}
			}
		fclose(hfp);
fprintf(stderr,"Output %d (expected %d) unfixed tie locations to %s\n", nroute, nties_unfixed, routefile);
		}

	/* write mbgrdviz route file for all fixed ties */
	sprintf(routefile,"%s%s_fixedties.rte",project.path,project.name);
	if ((hfp = fopen(routefile,"w")) == NULL)
		{
		fclose(hfp);
		status = MB_FAILURE;
		error = MB_ERROR_OPEN_FAIL;
		sprintf(message, " > Unable to open output fixed ties route file %s\n", routefile);
		do_info_add(message, MB_NO);
		if (mbna_verbose == 0)
			fprintf(stderr,"%s",message);
		}
	else
		{
		/* write the route file header */
		fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
		fprintf(hfp, "## Output by Program %s\n",program_name);
		fprintf(hfp, "## Program Version %s\n",rcs_id);
		fprintf(hfp, "## MB-System Version %s\n",MB_VERSION);
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strcpy(date,ctime(&right_now));
                date[strlen(date)-1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			if ((user_ptr = getenv("LOGNAME")) == NULL)
				user_ptr = unknown;
		gethostname(host,MB_PATH_MAXLINE);
		fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n",
			user_ptr,host,date);
		fprintf(hfp, "## Number of routes: %d\n",nties_fixed);
		fprintf(hfp, "## Route point format:\n");
		fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");
		routecolor = ROUTE_COLOR_RED;

		/* loop over all crossings */
		nroute = 0;
		for (i=0;i<project.num_crossings;i++)
			{
			crossing = &project.crossings[i];
   
			/* output only fixed ties */
			if (crossing->status == MBNA_CROSSING_STATUS_SET
				&& (project.files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV
					|| project.files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV))
				{
				for (j=0;j<crossing->num_ties;j++)
					{
					file_1 = (struct mbna_file *) &project.files[crossing->file_id_1];
					file_2 = (struct mbna_file *) &project.files[crossing->file_id_2];
					section_1 = (struct mbna_section *) &file_1->sections[crossing->section_1];
					section_2 = (struct mbna_section *) &file_2->sections[crossing->section_2];
					tie = (struct mbna_tie *) &crossing->ties[j];
					snav_1 = tie->snav_1;
					snav_2 = tie->snav_2;
					navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
					navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
					navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
					navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
					if (crossing->status == MBNA_CROSSING_STATUS_NONE)
						status_char = 'U';
					else if (crossing->status == MBNA_CROSSING_STATUS_SET)
						status_char = '*';
					else
						status_char = '-';
					if (crossing->truecrossing == MB_NO)
						truecrossing_char = ' ';
					else
						truecrossing_char = 'X';
					sprintf(routename,"Tie: %c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d of %2d",
						status_char, truecrossing_char, i,
						file_1->block,
						crossing->file_id_1,
						crossing->section_1,
						file_2->block,
						crossing->file_id_2,
						crossing->section_2,
						crossing->overlap,
						j, crossing->num_ties);
					fprintf(hfp,"## ROUTENAME %s\n", routename);
					fprintf(hfp,"## ROUTESIZE %d\n", 1);
					fprintf(hfp,"## ROUTECOLOR %d\n", routecolor);
					fprintf(hfp,"## ROUTEPOINTS %d\n", 2);
					fprintf(hfp,"## ROUTEEDITMODE %d\n", MB_NO);
					fprintf(hfp,"> ## STARTROUTE\n");
					fprintf(hfp,"%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n",
						navlon1,navlat1,navlon2,navlat2);
					nroute++;
					}
				}
			}
		fclose(hfp);
fprintf(stderr,"Output %d (expected %d) fixed tie locations to %s\n", nroute, nties_fixed, routefile);
		}

	/* output offset vectors */
	if (project.inversion == MBNA_INVERSION_CURRENT)
		{
		sprintf(xoffsetfile,"%s%s_dx.txt",project.path,project.name);
		sprintf(yoffsetfile,"%s%s_dy.txt",project.path,project.name);
		if ((xfp = fopen(xoffsetfile,"w")) != NULL
		    && (yfp = fopen(yoffsetfile,"w")) != NULL)
			{
			for (i=0;i<project.num_files;i++)
			    {
			    file = &project.files[i];
			    for (j=0;j<file->num_sections;j++)
				{
				section = &file->sections[j];
				for (k=0;k<section->num_snav;k++)
				    {
				    fprintf(xfp, "%.10f %.10f %.10f\n", section->snav_lon[k], section->snav_lat[k],
								section->snav_lon_offset[k]/mbna_mtodeglon);
				    fprintf(yfp, "%.10f %.10f %.10f\n", section->snav_lon[k], section->snav_lat[k],
								section->snav_lat_offset[k]/mbna_mtodeglat);
				    }
				}
			    }
			fclose(xfp);
			fclose(yfp);
			}

		/* else set error */
		else
			{
			status = MB_FAILURE;
			sprintf(message,"Unable to update project %s\n > Offset vector files: %s %s\n",
				project.name, xoffsetfile, yoffsetfile);
			do_info_add(message, MB_YES);
			}
		}

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_read_project()
{
	/* local variables */
	char	*function_name = "mbnavadjust_read_project";
	int	status = MB_SUCCESS;
	FILE	*hfp;
	struct mbna_file *file;
	struct mbna_section *section, *section1, *section2;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	char	label[STRING_MAX];
	char	buffer[BUFFER_MAX];
	char	obuffer[BUFFER_MAX];
	char	command[MB_PATH_MAXLINE];
	char	*result;
	int	versionmajor, versionminor, version_id;
	double	dummy;
	double	mtodeglon, mtodeglat;
	int	nscan, idummy, jdummy;
	int	s1id, s2id;
	int	shellstatus;
	int	i, j, k, l;

	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* first save copy of the project file */
	sprintf(command,"cp %s %s.save", project.home, project.home);
	shellstatus = system(command);

	/* open and read home file */
	status = MB_SUCCESS;
	if ((hfp = fopen(project.home,"r")) != NULL)
		{
		/* check for proper header */
		if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
			|| strncmp(buffer,"##MBNAVADJUST PROJECT",21) != 0)
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		/* read basic names and stats */
		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
			    	|| strcmp(label,"MB-SYSTEM_VERSION") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
				|| strcmp(label,"PROGRAM_VERSION") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %d.%d",label,&versionmajor,&versionminor)) != 3
				|| strcmp(label,"FILE_VERSION") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}
		version_id = 100 * versionmajor + versionminor;

		if (version_id >= 302)
			{
			if (status == MB_SUCCESS
				&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
					|| (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
					|| strcmp(label,"ORIGIN") != 0))
				status = MB_FAILURE;
			}
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
				|| strcmp(label,"NAME") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
				|| strcmp(label,"PATH") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
				|| strcmp(label,"HOME") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %s",label,obuffer)) != 2
				|| strcmp(label,"DATADIR") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %d",label,&project.num_files)) != 2
				|| strcmp(label,"NUMFILES") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (version_id >= 306)
			{
			if (status == MB_SUCCESS
				&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
					|| (nscan = sscanf(buffer,"%s %d",label,&project.num_blocks)) != 2
					|| strcmp(label,"NUMBLOCKS") != 0))
				status = MB_FAILURE;
			}
		else
			{
			project.num_blocks = 0;
			}
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %d",label,&project.num_crossings)) != 2
				|| strcmp(label,"NUMCROSSINGS") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %lf",label,&project.section_length)) != 2
				|| strcmp(label,"SECTIONLENGTH") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& version_id >= 101
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %d",label,&project.section_soundings)) != 2
				|| strcmp(label,"SECTIONSOUNDINGS") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %d",label,&project.decimation)) != 2
				|| strcmp(label,"DECIMATION") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %lf",label,&project.cont_int)) != 2
				|| strcmp(label,"CONTOURINTERVAL") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %lf",label,&project.col_int)) != 2
				|| strcmp(label,"COLORINTERVAL") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %lf",label,&project.tick_int)) != 2
				|| strcmp(label,"TICKINTERVAL") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS
			&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
				|| (nscan = sscanf(buffer,"%s %d",label,&project.inversion)) != 2
				|| strcmp(label,"INVERSION") != 0))
			status = MB_FAILURE;
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s buffer:%s\n",__LINE__,__FILE__,buffer);exit(0);}

		if (status == MB_SUCCESS)
			{
			if (version_id >= 301)
				{
				if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
					|| (nscan = sscanf(buffer,"%s %lf",label,&project.smoothing)) != 2
					|| strcmp(label,"SMOOTHING") != 0)
					status = MB_FAILURE;
				project.precision = SIGMA_MINIMUM;
				}
			else if (version_id >= 103)
				{
				if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
					|| (nscan = sscanf(buffer,"%s %lf",label,&project.precision)) != 2
					|| strcmp(label,"PRECISION") != 0)
					status = MB_FAILURE;
				project.smoothing = MBNA_SMOOTHING_DEFAULT;
				}
			else
				{
				project.precision = SIGMA_MINIMUM;
				project.smoothing = MBNA_SMOOTHING_DEFAULT;
				}
			}
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}

		if (status == MB_SUCCESS)
			{
			if (version_id >= 105
				&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
					|| (nscan = sscanf(buffer,"%s %lf",label,&project.zoffsetwidth)) != 2
					|| strcmp(label,"ZOFFSETWIDTH") != 0))
				status = MB_FAILURE;
			}

		/* allocate memory for files array */
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}

		if (project.num_files > 0)
			{
			project.files = (struct mbna_file *)
				malloc(sizeof(struct mbna_file) * (project.num_files));
			if (project.files != NULL)
				{
				project.num_files_alloc = project.num_files;
				memset(project.files,0,project.num_files_alloc * sizeof(struct mbna_file));
				}
			else
				{
				project.num_files_alloc = 0;
				status = MB_FAILURE;
				error = MB_ERROR_MEMORY_FAIL;
				}
			}
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}

		if (project.num_crossings > 0)
			{
			project.crossings = (struct mbna_crossing *)
				malloc(sizeof(struct mbna_crossing) * (project.num_crossings));
			if (project.crossings != NULL)
				{
				project.num_crossings_alloc = project.num_crossings;
				memset(project.crossings,0,sizeof(struct mbna_crossing) * project.num_crossings_alloc);
				}
			else
				{
				project.num_crossings_alloc = 0;
				status = MB_FAILURE;
				error = MB_ERROR_MEMORY_FAIL;
				}
			}
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}

		for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			file->num_sections_alloc = 0;
			file->sections = NULL;
			file->num_snavs = 0;
			file->num_pings = 0;
			file->num_beams = 0;
			if (version_id >= 306)
				{
				if (status == MB_SUCCESS
					&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
						|| (nscan = sscanf(buffer,"FILE %d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %d %d %s",
						&idummy,
						&(file->status),
						&(file->id),
						&(file->format),
						&(file->block),
						&(file->block_offset_x),
						&(file->block_offset_y),
						&(file->block_offset_z),
						&(file->heading_bias_import),
						&(file->roll_bias_import),
						&(file->heading_bias),
						&(file->roll_bias),
						&(file->num_sections),
						&(file->output_id),
						file->file)) != 15))
					status = MB_FAILURE;
				}
			else
				{
				if (status == MB_SUCCESS
					&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
						|| (nscan = sscanf(buffer,"FILE %d %d %d %d %lf %lf %lf %lf %d %d %s",
						&idummy,
						&(file->status),
						&(file->id),
						&(file->format),
						&(file->heading_bias_import),
						&(file->roll_bias_import),
						&(file->heading_bias),
						&(file->roll_bias),
						&(file->num_sections),
						&(file->output_id),
						file->file)) != 11))
					status = MB_FAILURE;
				file->block = 0;
				file->block_offset_x = 0.0;
				file->block_offset_y = 0.0;
				file->block_offset_z = 0.0;
				}

			/* set file->path as absolute path
			    - file->file may be a relative path */
			if (status == MB_SUCCESS)
				{
				if (file->file[0] == '/')
				    strcpy(file->path, file->file);
				else
				    {
				    strcpy(file->path, project.path);
				    strcat(file->path, file->file);
				    }
				}

			/* read section info */
			if (file->num_sections > 0)
				{
				file->sections = (struct mbna_section *)
					malloc(sizeof(struct mbna_section) * (file->num_sections));
				if (file->sections != NULL)
					{
					file->num_sections_alloc = file->num_sections;
					memset(file->sections,0,sizeof(struct mbna_section) * file->num_sections_alloc);
				        }
				else
					{
					file->num_sections_alloc = 0;
					status = MB_FAILURE;
					error = MB_ERROR_MEMORY_FAIL;
					}
				}
			for (j=0;j<file->num_sections;j++)
				{
				section = &file->sections[j];
				if (status == MB_SUCCESS)
					result = fgets(buffer,BUFFER_MAX,hfp);
				if (status == MB_SUCCESS && result == buffer)
					nscan = sscanf(buffer,"SECTION %d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %d",
						&idummy,
						&section->num_pings,
						&section->num_beams,
						&section->num_snav,
						&section->continuity,
						&section->distance,
						&section->btime_d,
						&section->etime_d,
						&section->lonmin,
						&section->lonmax,
						&section->latmin,
						&section->latmax,
						&section->depthmin,
						&section->depthmax,
						&section->contoursuptodate);
				if (result != buffer || nscan < 14)
					{
					status = MB_FAILURE;
fprintf(stderr, "read failed on section: %s\n", buffer);
					}
				if (nscan < 15)
					section->contoursuptodate = MB_NO;
				for (k=MBNA_MASK_DIM-1;k>=0;k--)
					{
					if (status == MB_SUCCESS)
					    result = fgets(buffer,BUFFER_MAX,hfp);
					for (l=0;l<MBNA_MASK_DIM;l++)
						{
						sscanf(&buffer[l], "%1d", &section->coverage[l+k*MBNA_MASK_DIM]);
						}
					}
if (status == MB_FAILURE){fprintf(stderr,"Die at line:%d file:%s\n",__LINE__,__FILE__);exit(0);}
/*fprintf(stderr,"%s/nvs_%4.4d_%4.4d.mb71\n",
project.datadir,file->id,j);
for (k=MBNA_MASK_DIM-1;k>=0;k--)
{
for (l=0;l<MBNA_MASK_DIM;l++)
{
fprintf(stderr, "%1d", section->coverage[l + k * MBNA_MASK_DIM]);
}
fprintf(stderr, "\n");
}*/
				for (k=0;k<section->num_snav;k++)
				    {
				    if (status == MB_SUCCESS)
					result = fgets(buffer,BUFFER_MAX,hfp);
				    if (status == MB_SUCCESS && result == buffer)
					nscan = sscanf(buffer,"SNAV %d %d %lf %lf %lf %lf %lf %lf %lf",
						&idummy,
						&section->snav_id[k],
						&section->snav_distance[k],
						&section->snav_time_d[k],
						&section->snav_lon[k],
						&section->snav_lat[k],
						&section->snav_lon_offset[k],
						&section->snav_lat_offset[k],
						&section->snav_z_offset[k]);
				    section->snav_num_ties[k] = 0;
				    section->snav_lon_offset_int[k] = 0.0;
				    section->snav_lat_offset_int[k] = 0.0;
				    section->snav_z_offset_int[k] = 0.0;
				    if (result == buffer && nscan == 6)
				    	{
				    	section->snav_lon_offset[k] = 0.0;
				    	section->snav_lat_offset[k] = 0.0;
				    	section->snav_z_offset[k] = 0.0;
				    	}
				    else if (result == buffer && nscan == 8)
				    	{
				    	section->snav_z_offset[k] = 0.0;
				    	}
				    else if (result != buffer || nscan != 9)
					{
					status = MB_FAILURE;
fprintf(stderr, "read failed on snav: %s\n", buffer);
					}

				    /* reverse offset values if older values */
				    if (version_id < 300)
					{
				    	section->snav_lon_offset[k] *= -1.0;
				    	section->snav_lat_offset[k] *= -1.0;
				    	section->snav_z_offset[k] *= -1.0;
					}
				    }
				    
				/* global fixed frame tie, whether defined or not */
				if (version_id >= 305)
					{
					if (status == MB_SUCCESS)
						result = fgets(buffer,BUFFER_MAX,hfp);
					if (status == MB_SUCCESS && result == buffer)
						nscan = sscanf(buffer,"GLOBALTIE %d %d %lf %lf %lf %lf %lf %lf",
							&section->global_tie_status,
							&section->global_tie_snav,
							&section->global_tie_offset_x,
							&section->global_tie_offset_y,
							&section->global_tie_offset_z_m,
							&section->global_tie_xsigma,
							&section->global_tie_ysigma,
							&section->global_tie_zsigma);
					mb_coor_scale(mbna_verbose,0.5 * (section->latmin + section->latmax),
							&mtodeglon,&mtodeglat);
					section->global_tie_offset_x_m = section->global_tie_offset_x / mtodeglon;
					section->global_tie_offset_y_m = section->global_tie_offset_y / mtodeglat;
/* if (section->global_tie_status != MBNA_TIE_NONE)
fprintf(stderr,"READ GLOBALTIE: %d %d %lf %lf %lf %lf %lf %lf\n",
section->global_tie_status,
section->global_tie_snav,
section->global_tie_offset_x,
section->global_tie_offset_y,
section->global_tie_offset_z_m,
section->global_tie_xsigma,
section->global_tie_ysigma,
section->global_tie_zsigma); */
					}
				else if (version_id == 304)
					{
					if (status == MB_SUCCESS)
						result = fgets(buffer,BUFFER_MAX,hfp);
					if (status == MB_SUCCESS && result == buffer)
						nscan = sscanf(buffer,"GLOBALTIE %d %lf %lf %lf %lf %lf %lf",
							&section->global_tie_snav,
							&section->global_tie_offset_x,
							&section->global_tie_offset_y,
							&section->global_tie_offset_z_m,
							&section->global_tie_xsigma,
							&section->global_tie_ysigma,
							&section->global_tie_zsigma);
					mb_coor_scale(mbna_verbose,0.5 * (section->latmin + section->latmax),
							&mtodeglon,&mtodeglat);
					section->global_tie_offset_x_m = section->global_tie_offset_x / mtodeglon;
					section->global_tie_offset_y_m = section->global_tie_offset_y / mtodeglat;
					if (section->global_tie_snav != MBNA_SELECT_NONE)
						section->global_tie_status = MBNA_TIE_XYZ;
					else
						section->global_tie_status = MBNA_TIE_NONE;
/* if (section->global_tie_snav != MBNA_SELECT_NONE)
fprintf(stderr,"READ GLOBALTIE: %d %lf %lf %lf %lf %lf %lf\n",
section->global_tie_snav,
section->global_tie_offset_x,
section->global_tie_offset_y,
section->global_tie_offset_z_m,
section->global_tie_xsigma,
section->global_tie_ysigma,
section->global_tie_zsigma); */
					}
				else
					{
					section->global_tie_snav = MBNA_TIE_NONE;
					section->global_tie_snav = MBNA_SELECT_NONE;
					section->global_tie_offset_x = 0.0;
					section->global_tie_offset_y = 0.0;
					section->global_tie_offset_z_m = 0.0;
					section->global_tie_xsigma = 0.0;
					section->global_tie_ysigma = 0.0;
					section->global_tie_zsigma = 0.0;
					}

				section->global_start_ping = project.num_pings;
				section->global_start_snav = project.num_snavs - section->continuity;
				file->num_snavs += section->num_pings;
				file->num_pings += section->num_pings;
				file->num_beams += section->num_beams;
				project.num_snavs += (section->num_snav - section->continuity);
				project.num_pings += section->num_pings;
				project.num_beams += section->num_beams;
				}
			}

		/* count the number of blocks */
		if (version_id < 306)
			{
			project.num_blocks = 0;
			for (i=0;i<project.num_files;i++)
				{
				file = &project.files[i];
				if (i==0 || file->sections[0].continuity == MB_NO)
					{
					project.num_blocks++;
					}
				file->block = project.num_blocks - 1;
				file->block_offset_x = 0.0;
				file->block_offset_y = 0.0;
				file->block_offset_z = 0.0;
				}
			}

		/* read crossings */
 		project.num_crossings_analyzed = 0;
		project.num_goodcrossings = 0;
		project.num_truecrossings = 0;
		project.num_truecrossings_analyzed = 0;
 		project.num_ties = 0;
		for (i=0;i<project.num_crossings;i++)
			{
			/* read each crossing */
			crossing = &project.crossings[i];
			if (status == MB_SUCCESS
				&& version_id >= 106)
				{
				if (status == MB_SUCCESS
					&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
						|| (nscan = sscanf(buffer,"CROSSING %d %d %d %d %d %d %d %d %d",
 						&idummy,
						&crossing->status,
						&crossing->truecrossing,
						&crossing->overlap,
						&crossing->file_id_1,
						&crossing->section_1,
						&crossing->file_id_2,
						&crossing->section_2,
						&crossing->num_ties)) != 9))
						{
						status = MB_FAILURE;
fprintf(stderr, "read failed on crossing: %s\n", buffer);
						}
				}
			else if (status == MB_SUCCESS
				&& version_id >= 102)
				{
				crossing->overlap = 0;
				if (status == MB_SUCCESS
					&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
						|| (nscan = sscanf(buffer,"CROSSING %d %d %d %d %d %d %d %d",
 						&idummy,
						&crossing->status,
						&crossing->truecrossing,
						&crossing->file_id_1,
						&crossing->section_1,
						&crossing->file_id_2,
						&crossing->section_2,
						&crossing->num_ties)) != 8))
						{
						status = MB_FAILURE;
fprintf(stderr, "read failed on crossing: %s\n", buffer);
						}
				}
			else if (status == MB_SUCCESS)
				{
				crossing->truecrossing = MB_NO;
				crossing->overlap = 0;
				if (status == MB_SUCCESS
					&& ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
						|| (nscan = sscanf(buffer,"CROSSING %d %d %d %d %d %d %d",
 						&idummy,
						&crossing->status,
						&crossing->file_id_1,
						&crossing->section_1,
						&crossing->file_id_2,
						&crossing->section_2,
						&crossing->num_ties)) != 7))
						{
						status = MB_FAILURE;
fprintf(stderr, "read failed on old format crossing: %s\n", buffer);
						}
				}
			if (status == MB_SUCCESS
			    && crossing->status != MBNA_CROSSING_STATUS_NONE)
				project.num_crossings_analyzed++;
			if (status == MB_SUCCESS
			    && crossing->truecrossing == MB_YES)
				{
				project.num_truecrossings++;
				if (crossing->status != MBNA_CROSSING_STATUS_NONE)
				project.num_truecrossings_analyzed++;
				}

			/* reorder crossing to be early file first older file second if
				file version prior to 3.00 */
			if (version_id < 300)
				{
				idummy = crossing->file_id_1;
				jdummy = crossing->section_1;
				crossing->file_id_1 = crossing->file_id_2;
				crossing->section_1 = crossing->section_2;
				crossing->file_id_2 = idummy;
				crossing->section_2 = jdummy;
				}

			/* read ties */
			if (status == MB_SUCCESS)
			for (j=0;j<crossing->num_ties;j++)
				{
				/* read each tie */
				tie = &crossing->ties[j];
				if (status == MB_SUCCESS && version_id >= 302)
					{
					if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
						|| (nscan = sscanf(buffer,"TIE %d %d %d %lf %d %lf %lf %lf %lf %d %lf %lf %lf",
							&idummy,
							&tie->status,
							&tie->snav_1,
							&tie->snav_1_time_d,
							&tie->snav_2,
							&tie->snav_2_time_d,
							&tie->offset_x,
							&tie->offset_y,
							&tie->offset_z_m,
							&tie->inversion_status,
							&tie->inversion_offset_x,
							&tie->inversion_offset_y,
							&tie->inversion_offset_z_m)) != 13)
						{
						status = MB_FAILURE;
fprintf(stderr, "read failed on tie: %s\n", buffer);
						}
					}
				else if (status == MB_SUCCESS && version_id >= 104)
					{
					if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
						|| (nscan = sscanf(buffer,"TIE %d %d %lf %d %lf %lf %lf %lf %d %lf %lf %lf",
							&idummy,
							&tie->snav_1,
							&tie->snav_1_time_d,
							&tie->snav_2,
							&tie->snav_2_time_d,
							&tie->offset_x,
							&tie->offset_y,
							&tie->offset_z_m,
							&tie->inversion_status,
							&tie->inversion_offset_x,
							&tie->inversion_offset_y,
							&tie->inversion_offset_z_m)) != 12)
						{
						status = MB_FAILURE;
fprintf(stderr, "read failed on tie: %s\n", buffer);
						}
					tie->status = MBNA_TIE_XYZ;
					}
				else if (status == MB_SUCCESS)
					{
					if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
						|| (nscan = sscanf(buffer,"TIE %d %d %lf %d %lf %lf %lf %d %lf %lf",
							&idummy,
							&tie->snav_1,
							&tie->snav_1_time_d,
							&tie->snav_2,
							&tie->snav_2_time_d,
							&tie->offset_x,
							&tie->offset_y,
							&tie->inversion_status,
							&tie->inversion_offset_x,
							&tie->inversion_offset_y)) != 10)
						{
						status = MB_FAILURE;
fprintf(stderr, "read failed on tie: %s\n", buffer);
						}
					tie->status = MBNA_TIE_XYZ;
					tie->offset_z_m = 0.0;
					tie->inversion_offset_z_m = 0.0;
					}

				/* reorder crossing to be early file first older file second if
					file version prior to 3.00 */
				if (version_id < 300)
					{
					idummy = tie->snav_1;
					dummy = tie->snav_1_time_d;
					tie->snav_1 = tie->snav_2;
					tie->snav_1_time_d = tie->snav_2_time_d;
					tie->snav_2 = idummy;
					tie->snav_2_time_d = dummy;
/*					tie->offset_x *= -1.0;
					tie->offset_y *= -1.0;
					tie->offset_z_m *= -1.0;
					tie->inversion_offset_x *= -1.0;
					tie->inversion_offset_y *= -1.0;
					tie->inversion_offset_z_m *= -1.0;*/
					}

				/* for version 2.0 or later read covariance */
				if (status == MB_SUCCESS && version_id >= 200)
					{
					if ((result = fgets(buffer,BUFFER_MAX,hfp)) != buffer
						|| (nscan = sscanf(buffer,"COV %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
							&tie->sigmar1,
							&(tie->sigmax1[0]),
							&(tie->sigmax1[1]),
							&(tie->sigmax1[2]),
							&tie->sigmar2,
							&(tie->sigmax2[0]),
							&(tie->sigmax2[1]),
							&(tie->sigmax2[2]),
							&tie->sigmar3,
							&(tie->sigmax3[0]),
							&(tie->sigmax3[1]),
							&(tie->sigmax3[2]))) != 12)
						{
						status = MB_FAILURE;
fprintf(stderr, "read failed on tie covariance: %s\n", buffer);
						}
					if (tie->sigmar1 <= 0.0)
						{
						tie->sigmax1[0] = 1.0;
						tie->sigmax1[1] = 0.0;
						tie->sigmax1[2] = 0.0;
						mbna_minmisfit_sr1 = 1.0;
						}
					if (tie->sigmar2 <= 0.0)
						{
						tie->sigmax2[0] = 0.0;
						tie->sigmax2[1] = 1.0;
						tie->sigmax2[2] = 0.0;
						mbna_minmisfit_sr2 = 1.0;
						}
					if (tie->sigmar3 <= 0.0)
						{
						tie->sigmax3[0] = 0.0;
						tie->sigmax3[1] = 0.0;
						tie->sigmax3[2] = 1.0;
						mbna_minmisfit_sr3 = 1.0;
						}
					}
				else if (status == MB_SUCCESS)
					{
					tie->sigmar1 = 100.0;
					tie->sigmax1[0] = 1.0;
					tie->sigmax1[1] = 0.0;
					tie->sigmax1[2] = 0.0;
					tie->sigmar2 = 100.0;
					tie->sigmax2[0] = 0.0;
					tie->sigmax2[1] = 1.0;
					tie->sigmax2[2] = 0.0;
					tie->sigmar3 = 100.0;
					tie->sigmax3[0] = 0.0;
					tie->sigmax3[1] = 0.0;
					tie->sigmax3[2] = 1.0;
					}

				/* update number of ties */
				if (status == MB_SUCCESS)
				    {
				    project.num_ties++;
				    }

				/* check for reasonable snav id's */
				if (status == MB_SUCCESS)
				    {
				    file = &project.files[crossing->file_id_1];
				    section = &file->sections[crossing->section_1];
				    if (tie->snav_1 >= section->num_snav)
					{
					tie->snav_1 = ((double)tie->snav_1
								/ (double)section->num_pings)
								* (MBNA_SNAV_NUM - 1);
					tie->snav_1_time_d = section->snav_time_d[tie->snav_1];
fprintf(stderr,"Reset tie snav_1 on read:%d\n",tie->snav_1);
					}
				    file = &project.files[crossing->file_id_2];
				    section = &file->sections[crossing->section_2];
				    if (tie->snav_2 >= section->num_snav)
					{
					tie->snav_2 = ((double)tie->snav_2
								/ (double)section->num_pings)
								* (MBNA_SNAV_NUM - 1);
					tie->snav_2_time_d = section->snav_time_d[tie->snav_2];
fprintf(stderr,"Reset tie snav_2 on read:%d\n",tie->snav_2);
					}
				    }

				/* update number of ties for snavs */
				if (status == MB_SUCCESS)
				    {
				    file = &project.files[crossing->file_id_1];
				    section = &file->sections[crossing->section_1];
				    section->snav_num_ties[tie->snav_1]++;
				    file = &project.files[crossing->file_id_2];
				    section = &file->sections[crossing->section_2];
				    section->snav_num_ties[tie->snav_2]++;
				    }

				/* calculate offsets in local meters */
				if (status == MB_SUCCESS)
				    {
				    section1 = &(project.files[crossing->file_id_1].sections[crossing->section_1]);
				    section2 = &(project.files[crossing->file_id_2].sections[crossing->section_2]);
				    mbna_lon_min = MIN(section1->lonmin,section2->lonmin);
				    mbna_lon_max = MAX(section1->lonmax,section2->lonmax);
				    mbna_lat_min = MIN(section1->latmin,section2->latmin);
				    mbna_lat_max = MAX(section1->latmax,section2->latmax);
				    mb_coor_scale(mbna_verbose,0.5 * (mbna_lat_min + mbna_lat_max),
						    &mbna_mtodeglon,&mbna_mtodeglat);
				    tie->offset_x_m = tie->offset_x / mbna_mtodeglon;
				    tie->offset_y_m = tie->offset_y / mbna_mtodeglat;
				    tie->inversion_offset_x_m = tie->inversion_offset_x / mbna_mtodeglon;
				    tie->inversion_offset_y_m = tie->inversion_offset_y / mbna_mtodeglat;
				    }

				}

			/* finally make sure crossing has later section second, switch if needed */
			s1id = crossing->file_id_1 * 1000 + crossing->section_1;
			s2id = crossing->file_id_2 * 1000 + crossing->section_2;
			if (s2id < s1id)
				{
				idummy = crossing->file_id_1;
				jdummy = crossing->section_1;
				crossing->file_id_1 = crossing->file_id_2;
				crossing->section_1 = crossing->section_2;
				crossing->file_id_2 = idummy;
				crossing->section_2 = jdummy;
				for (j=0;j<crossing->num_ties;j++)
					{
					tie = &crossing->ties[j];
					idummy = tie->snav_1;
					dummy = tie->snav_1_time_d;
					tie->snav_1 = tie->snav_2;
					tie->snav_1_time_d = tie->snav_2_time_d;
					tie->snav_2 = idummy;
					tie->snav_2_time_d = dummy;
					tie->offset_x *= -1.0;
					tie->offset_y *= -1.0;
					tie->offset_x_m *= -1.0;
					tie->offset_y_m *= -1.0;
					tie->offset_z_m *= -1.0;
					tie->inversion_offset_x *= -1.0;
					tie->inversion_offset_y *= -1.0;
					tie->inversion_offset_x_m *= -1.0;
					tie->inversion_offset_y_m *= -1.0;
					tie->inversion_offset_z_m *= -1.0;
					}
				}
			}

		/* close home file */
		fclose(hfp);

		/* set project status flag */
		if (status == MB_SUCCESS)
			project.open = MB_YES;
		else
			{
			for (i=0;i<project.num_files;i++)
				{
				file = &project.files[i];
				if (file->sections != NULL)
					 free( file->sections);
				}
			if (project.files != NULL)
				free(project.files);
			if (project.crossings != NULL)
				free(project.crossings);
			project.open = MB_NO;
			memset(project.name,0,STRING_MAX);
			strcpy(project.name,"None");
 			memset(project.path,0,STRING_MAX);
			memset(project.datadir,0,STRING_MAX);
			project.num_files = 0;
			project.num_files_alloc = 0;
			project.num_snavs = 0;
			project.num_pings = 0;
			project.num_beams = 0;
			project.num_crossings = 0;
			project.num_crossings_alloc = 0;
 			project.num_crossings_analyzed = 0;
			project.num_goodcrossings = 0;
			project.num_truecrossings = 0;
			project.num_truecrossings_analyzed = 0;
			project.num_ties = 0;
 			}

		/* recalculate crossing overlap values if not already set */
		if (project.open == MB_YES)
			{
			for (i=0;i<project.num_crossings;i++)
				{
				crossing = &(project.crossings[i]);
				if (crossing->overlap <= 0)
					{
					mbnavadjust_crossing_overlap(i);
					}
				if (crossing->overlap >= 25)
					project.num_goodcrossings++;
				}
			}

		/* reset crossings to unanalyzed if flag is set */
		if (mbna_reset_crossings == MB_YES)
			{
			for (i=0;i<project.num_crossings;i++)
				{
				/* read each crossing */
				crossing = &project.crossings[i];

				/* reset status */
				crossing->status = MBNA_CROSSING_STATUS_NONE;
				crossing->num_ties = 0;
				project.num_crossings_analyzed = 0;
				project.num_truecrossings_analyzed = 0;
				project.num_ties = 0;
				project.inversion = MBNA_INVERSION_NONE;
				}
			for (i=0;i<project.num_files;i++)
				{
				file = &project.files[i];
				for (j=0;j<file->num_sections;j++)
					{
					section = &file->sections[j];
					for (k=0;k<section->num_snav;k++)
						{
						section->snav_lon_offset[section->num_snav] = 0.0;
						section->snav_lat_offset[section->num_snav] = 0.0;
						section->snav_z_offset[section->num_snav] = 0.0;
						section->snav_lon_offset_int[section->num_snav] = 0.0;
						section->snav_lat_offset_int[section->num_snav] = 0.0;
						section->snav_z_offset_int[section->num_snav] = 0.0;
						}
					}
				}
			}

		/* interpolate inversion solution if it exists */
		if (project.inversion != MBNA_INVERSION_NONE)
			mbnavadjust_interpolatesolution();
		}

	/* else set error */
	else
		{
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_import_data(char *path, int iformat)
{
	/* local variables */
	char	*function_name = "mbnavadjust_import_data";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	int	done;
	char	filename[STRING_MAX];
	double	weight;
	int	form;
	int	firstfile;
	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2               path:     %s\n",path);
		fprintf(stderr,"dbg2               format:   %d\n",iformat);
		}

	/* loop until all files read */
	done = MB_NO;
	firstfile = MB_YES;
	while (done == MB_NO)
		{
		if (iformat > 0)
			{
			status = mbnavadjust_import_file(path,iformat,firstfile);
			done = MB_YES;
			firstfile = MB_NO;
			}
		else if (iformat == -1)
			{
			if ((status = mb_datalist_open(mbna_verbose,&datalist,
							path,MB_DATALIST_LOOK_NO,&error)) == MB_SUCCESS)
				{
				while (done == MB_NO)
					{
					if ((status = mb_datalist_read(mbna_verbose,datalist,
							filename,&form,&weight,&error))
							== MB_SUCCESS)
						{
						status = mbnavadjust_import_file(filename,form,firstfile);
						firstfile = MB_NO;
						}
					else
						{
						mb_datalist_close(mbna_verbose,&datalist,&error);
						done = MB_YES;
						}
					}
				}
			}
		}

	/* look for new crossings */
	status = mbnavadjust_findcrossings();

	/* count the number of blocks */
	project.num_blocks = 0;
	for (i=0;i<project.num_files;i++)
	    {
	    file = &project.files[i];
	    if (i==0 || file->sections[0].continuity == MB_NO)
		{
		project.num_blocks++;
		}
	    file->block = project.num_blocks - 1;
	    file->block_offset_x = 0.0;
	    file->block_offset_y = 0.0;
	    file->block_offset_z = 0.0;
	    }

	/* write updated project */
	mbnavadjust_write_project();

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_import_file(char *path, int iformat, int firstfile)
{
	/* local variables */
	char	*function_name = "mbnavadjust_import_file";
	int	status = MB_SUCCESS;
	struct stat file_status;
	int	fstat;
	char	ipath[STRING_MAX];
	char	mb_suffix[STRING_MAX];
	char	npath[STRING_MAX];
	char	opath[STRING_MAX];
	char	*root;

	/* mbio read and write values */
	void	*imbio_ptr = NULL;
	void	*ombio_ptr = NULL;
	void	*istore_ptr = NULL;
	void	*ostore_ptr = NULL;
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
	double	draft;
	double	roll;
	double	pitch;
	double	heave;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[MB_COMMENT_MAXLINE];

	int	sonartype = MB_TOPOGRAPHY_TYPE_UNKNOWN;
	int	*bin_nbath = NULL;
	double	*bin_bath = NULL;
	double	*bin_bathacrosstrack = NULL;
	double	*bin_bathalongtrack = NULL;
	int	side;
	double	port_time_d, stbd_time_d;
	double	angle, dt, alongtrackdistance, xtrackavg, xtrackmax;
	int	nxtrack;

	int	output_id, found;
	int	obeams_bath,obeams_amp,opixels_ss;
	int	iform;
	int	nread, first;
	int	output_open = MB_NO;
	int	good_beams;
	int	new_segment;
	double	headingx, headingy, mtodeglon, mtodeglat;
	double	lon, lat;
	double	navlon_old, navlat_old;
	FILE	*nfp;
	struct mbna_file *file, *cfile;
	struct mbna_section *section, *csection;
	struct mbsys_ldeoih_struct *ostore;
	struct mb_io_struct *omb_io_ptr;
	int	new_pings, new_crossings;
	double	dx1, dy1;
	int	mbp_heading_mode;
	double	mbp_headingbias;
	int	mbp_rollbias_mode;
	double	mbp_rollbias;
	double	mbp_rollbias_port;
	double	mbp_rollbias_stbd;
	double	depthmax, distmax, depthscale, distscale;
 	int	i, j, k;
	int	ii1, jj1;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2               path:     %s\n",path);
		fprintf(stderr,"dbg2               format:   %d\n",iformat);
		}

	/* get potential processed file name */
	if ((status = mb_get_format(mbna_verbose, path, ipath,
				    &iform, &error))
				    == MB_SUCCESS
	    && iform == iformat)
	    {
	    strcat(ipath,"p");
	    sprintf(mb_suffix, ".mb%d", iformat);
	    strcat(ipath,mb_suffix);
	    }

	/* else just add p.mbXXX to file name */
	else
		{
		strcat(ipath,"p");
		sprintf(mb_suffix, ".mb%d", iformat);
		strcat(ipath,mb_suffix);
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		}

	/* look for processed file and use if available */
	fstat = stat(ipath, &file_status);
	if (fstat != 0
	    || (file_status.st_mode & S_IFMT) == S_IFDIR)
	    {
	    strcpy(ipath, path);
	    }

	/* now look for existing mbnavadjust output files
	 * - increment output id so this mbnavadjust project outputs
	 *   a unique nav file for this input file */
	output_id = 0;
	found = MB_NO;
	while (found == MB_NO)
	    {
	    sprintf(opath, "%s.na%d", path, output_id);
	    fstat = stat(opath, &file_status);
	    if (fstat != 0)
		{
		found = MB_YES;
		}
	    else
		{
		output_id++;
		}
	    }

	/* turn on message */
	root = (char *) strrchr(ipath, '/');
	if (root == NULL)
		root = ipath;
	sprintf(message,"Importing format %d data from %s",iformat,root);
	do_message_on(message);
	fprintf(stderr,"%s\n",message);
	output_open = MB_NO;
	project.inversion = MBNA_INVERSION_NONE;
	new_pings = 0;
	new_crossings = 0;
	good_beams = 0;

	/* allocate mbna_file array if needed */
	if (project.num_files_alloc <= project.num_files)
		{
		project.files = (struct mbna_file *) realloc(project.files,
			sizeof(struct mbna_file) * (project.num_files_alloc + ALLOC_NUM));
		if (project.files != NULL)
			project.num_files_alloc += ALLOC_NUM;
		else
			{
			status = MB_FAILURE;
			error = MB_ERROR_MEMORY_FAIL;
			}
		}

	if (status == MB_SUCCESS)
		{
		/* initialize reading the swath file */
		if ((status = mb_read_init(
			mbna_verbose,ipath,iformat,pings,lonflip,bounds,
			btime_i,etime_i,speedmin,timegap,
			&imbio_ptr,&btime_d,&etime_d,
			&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(mbna_verbose,error,&error_message);
			fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",error_message);
			fprintf(stderr,"\nSwath sonar File <%s> not initialized for reading\n",path);
			}
		}

	/* allocate memory for data arrays */
	if (status == MB_SUCCESS)
		{
		beamflag = NULL;
		bath = NULL;
		amp = NULL;
		bathacrosstrack = NULL;
		bathalongtrack = NULL;
		ss = NULL;
		ssacrosstrack = NULL;
		ssalongtrack = NULL;
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ssalongtrack, &error);

		/* if error initializing memory then don't read the file */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(mbna_verbose,error,&error_message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
				error_message);
			}
 		}

	/* open nav file */
	if (status == MB_SUCCESS)
		{
		sprintf(npath,"%s/nvs_%4.4d.mb166", project.datadir,project.num_files);
		if ((nfp = fopen(npath,"w")) == NULL)
			{
			status = MB_FAILURE;
			error = MB_ERROR_OPEN_FAIL;
			}
		}

	/* read data */
	if (status == MB_SUCCESS)
		{
		nread = 0;
		new_segment = MB_NO;
		first = MB_YES;
		while (error <= MB_ERROR_NO_ERROR)
			{
			/* read a ping of data */
			status = mb_get_all(mbna_verbose,imbio_ptr,&istore_ptr,&kind,
				time_i,&time_d,&navlon,&navlat,&speed,
				&heading,&distance,&altitude,&sonardepth,
				&beams_bath,&beams_amp,&pixels_ss,
				beamflag,bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,&error);

			/* extract all nav values */
			if (error == MB_ERROR_NO_ERROR
			    && (kind == MB_DATA_NAV
				|| kind == MB_DATA_DATA))
				{
				status = mb_extract_nav(mbna_verbose,imbio_ptr,
					istore_ptr,&kind,
					time_i,&time_d,&navlon,&navlat,&speed,
					&heading,&draft,&roll,&pitch,&heave,
					&error);
				}

			/* ignore minor errors */
			if (kind == MB_DATA_DATA
				&& (error == MB_ERROR_TIME_GAP
					|| error == MB_ERROR_OUT_BOUNDS
					|| error == MB_ERROR_OUT_TIME
					|| error == MB_ERROR_SPEED_TOO_SMALL))
				{
				status = MB_SUCCESS;
				error = MB_ERROR_NO_ERROR;
				}

			/* if sonar is interferometric, bin the bathymetry */
			if (kind == MB_DATA_DATA)
				{
				if (sonartype == MB_TOPOGRAPHY_TYPE_UNKNOWN)
					status = mb_sonartype(mbna_verbose, imbio_ptr, istore_ptr, &sonartype, &error);

				if (sonartype == MB_TOPOGRAPHY_TYPE_INTERFEROMETRIC)
					{
					/* allocate bin arrays if needed */
					if (bin_nbath == NULL)
						{
						status = mb_mallocd(mbna_verbose, __FILE__, __LINE__,
								    mbna_bin_beams_bath*sizeof(int),
									(void **)&bin_nbath,&error);
						status = mb_mallocd(mbna_verbose, __FILE__, __LINE__,
								    mbna_bin_beams_bath*sizeof(double),
									(void **)&bin_bath,&error);
						status = mb_mallocd(mbna_verbose, __FILE__, __LINE__,
								    mbna_bin_beams_bath*sizeof(double),
									(void **)&bin_bathacrosstrack,&error);
						status = mb_mallocd(mbna_verbose, __FILE__, __LINE__,
								    mbna_bin_beams_bath*sizeof(double),
									(void **)&bin_bathalongtrack,&error);
						for (i=0;i<mbna_bin_beams_bath;i++)
							{
							bin_nbath[i] = 0;
							bin_bath[i] = 0.0;
							bin_bathacrosstrack[i] = 0.0;
							bin_bathalongtrack[i] = 0.0;
							}
						}

					/* figure out if this is a ping to one side or across the while swath */
					xtrackavg = 0.0;
					xtrackmax = 0.0;
					nxtrack = 0;
					for (i=0;i<beams_bath;i++)
						{
						if (mb_beam_ok(beamflag[i]))
							{
							xtrackavg += bathacrosstrack[i];
							xtrackmax = MAX(xtrackmax, fabs(bathacrosstrack[i]));
							nxtrack++;
							}
						}
					if (nxtrack > 0)
						{
						xtrackavg /= nxtrack;
						}
					if (xtrackavg > 0.25 * xtrackmax)
						{
						side = SIDE_STBD;
						port_time_d = time_d;
						}
					else if (xtrackavg < -0.25 * xtrackmax)
						{
						side = SIDE_PORT;
						stbd_time_d = time_d;
						}
					else
						{
						side = SIDE_FULLSWATH;
						stbd_time_d = time_d;
						}

					/* if side = PORT or FULLSWATH then initialize bin arrays */
					if (side == SIDE_PORT || side == SIDE_FULLSWATH)
						{
						for (i=0;i<mbna_bin_beams_bath;i++)
							{
							bin_nbath[i] = 0;
							bin_bath[i] = 0.0;
							bin_bathacrosstrack[i] = 0.0;
							bin_bathalongtrack[i] = 0.0;
							}
						}

					/* bin the bathymetry */
					for (i=0;i<beams_bath;i++)
						{
						if (mb_beam_ok(beamflag[i]))
							{
							/* get apparent acrosstrack beam angle and bin accordingly */
							angle = RTD * atan(bathacrosstrack[i] / (bath[i] - sonardepth));
							j = (int)floor((angle + 0.5 * mbna_bin_swathwidth
									+ 0.5 * mbna_bin_pseudobeamwidth)
								       / mbna_bin_pseudobeamwidth);
/* fprintf(stderr,"i:%d bath:%f %f %f sonardepth:%f angle:%f j:%d\n",
i,bath[i],bathacrosstrack[i],bathalongtrack[i],sonardepth,angle,j); */
							if (j >= 0 && j < mbna_bin_beams_bath)
								{
								bin_bath[j] += bath[i];
								bin_bathacrosstrack[j] += bathacrosstrack[i];
								bin_bathalongtrack[j] += bathalongtrack[i];
								bin_nbath[j]++;
								}
							}
						}

					/* if side = STBD or FULLSWATH calculate output bathymetry
						- add alongtrack offset to port data from previous ping */
					if (side == SIDE_STBD || side == SIDE_FULLSWATH)
						{
						dt = port_time_d - stbd_time_d;
						if (dt > 0.0 && dt < 0.5)
							alongtrackdistance = -(port_time_d - stbd_time_d) * speed / 3.6;
						else
							alongtrackdistance = 0.0;
						beams_bath = mbna_bin_beams_bath;
						for (j=0;j<mbna_bin_beams_bath;j++)
							{
/* fprintf(stderr,"j:%d angle:%f n:%d bath:%f %f %f\n",
j,j*mbna_bin_pseudobeamwidth - 0.5 * mbna_bin_swathwidth,bin_nbath[j],bin_bath[j],bin_bathacrosstrack[j],bin_bathalongtrack[j]); */
							if (bin_nbath[j] > 0)
								{
								bath[j] = bin_bath[j] / bin_nbath[j];
								bathacrosstrack[j] = bin_bathacrosstrack[j] / bin_nbath[j];
								bathalongtrack[j] = bin_bathalongtrack[j] / bin_nbath[j];
								beamflag[j] = MB_FLAG_NONE;
								if (bin_bathacrosstrack[j] < 0.0)
									bathalongtrack[j] += alongtrackdistance;
								}
							else
								{
								beamflag[j] = MB_FLAG_NULL;
								bath[j] = 0.0;
								bathacrosstrack[j] = 0.0;
								bathalongtrack[j] = 0.0;
								}
							}
						}

					/* if side = PORT set nonfatal error so that bathymetry isn't output until
						the STBD data are read too */
					else if (side == SIDE_PORT)
						{
						error = MB_ERROR_IGNORE;
						}
					}
				}

			/* deal with new file */
			if (kind == MB_DATA_DATA
				&& error == MB_ERROR_NO_ERROR
				&& first == MB_YES)
				{
				file = &project.files[project.num_files];
				file->status = MBNA_FILE_GOODNAV;
				file->id = project.num_files;
				file->output_id = output_id;
				strcpy(file->path,path);
				strcpy(file->file,path);
				mb_get_relative_path(mbna_verbose,
							file->file,
							project.path,
							&error);
				file->format = iformat;
				file->heading_bias = 0.0;
				file->roll_bias = 0.0;
				file->num_snavs = 0;
				file->num_pings = 0;
				file->num_beams = 0;
				file->num_sections = 0;
				file->num_sections_alloc = 0;
				file->sections = NULL;
				project.num_files++;
				new_segment = MB_YES;
				first = MB_NO;

				/* get bias values */
				mb_pr_get_heading(mbna_verbose, file->path,
						    &mbp_heading_mode,
						    &mbp_headingbias,
						    &error);
				mb_pr_get_rollbias(mbna_verbose, file->path,
						    &mbp_rollbias_mode,
						    &mbp_rollbias,
						    &mbp_rollbias_port,
						    &mbp_rollbias_stbd,
						    &error);
				if (mbp_heading_mode == MBP_HEADING_OFFSET
				    || mbp_heading_mode == MBP_HEADING_CALCOFFSET)
				    {
				    file->heading_bias_import = mbp_headingbias;
				    }
				else
				    {
				    file->heading_bias_import = 0.0;
				    }
				if (mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
				    {
				    file->roll_bias_import = mbp_rollbias;
				    }
				else if (mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
				    {
				    file->roll_bias_import = 0.5 * (mbp_rollbias_port
								    + mbp_rollbias_stbd);
				    }
				else
				    {
				    file->roll_bias_import = 0.0;
				    }
				}

			/* check if new segment needed */
			else if (kind == MB_DATA_DATA
				&& error == MB_ERROR_NO_ERROR
				&& (section->distance + distance
					>= project.section_length
					|| section->num_beams >= project.section_soundings))
				{
				new_segment = MB_YES;
/*fprintf(stderr, "NEW SEGMENT: section->distance:%f distance:%f project.section_length:%f\n",
section->distance, distance, project.section_length);*/
				}

			/* if end of segment or end of file resolve position
			    of last snav point in last segment */
			if ((error > MB_ERROR_NO_ERROR || new_segment == MB_YES)
				&& project.num_files > 0
				&& (file->num_sections > 0 && section->num_pings > 0))
				{
				/* resolve position of last snav point in last segment */
				if (section->num_snav == 1
				    || (section->distance >=
					(section->num_snav - 0.5)
					    * project.section_length / (MBNA_SNAV_NUM - 1)))
					{
					section->snav_id[section->num_snav]
						= section->num_pings - 1;
					section->snav_num_ties[section->num_snav]
						= 0;
					section->snav_distance[section->num_snav]
						= section->distance;
					section->snav_time_d[section->num_snav]
						= section->etime_d;
					section->snav_lon[section->num_snav]
						= navlon_old;
					section->snav_lat[section->num_snav]
						= navlat_old;
					section->snav_lon_offset[section->num_snav]
						= 0.0;
					section->snav_lat_offset[section->num_snav]
						= 0.0;
					section->snav_z_offset[section->num_snav]
						= 0.0;
					section->snav_lon_offset_int[section->num_snav]
						= 0.0;
					section->snav_lat_offset_int[section->num_snav]
						= 0.0;
					section->snav_z_offset_int[section->num_snav]
						= 0.0;
					section->num_snav++;
					file->num_snavs++;
					project.num_snavs++;
					}
				else if (section->num_snav > 1)
					{
					section->snav_id[section->num_snav-1]
						= section->num_pings - 1;
					section->snav_num_ties[section->num_snav]
						= 0;
					section->snav_distance[section->num_snav-1]
						= section->distance;
					section->snav_time_d[section->num_snav-1]
						= section->etime_d;
					section->snav_lon[section->num_snav-1]
						= navlon_old;
					section->snav_lat[section->num_snav-1]
						= navlat_old;
					section->snav_lon_offset[section->num_snav-1]
						= 0.0;
					section->snav_lat_offset[section->num_snav-1]
						= 0.0;
					section->snav_z_offset[section->num_snav-1]
						= 0.0;
					section->snav_lon_offset_int[section->num_snav-1]
						= 0.0;
					section->snav_lat_offset_int[section->num_snav-1]
						= 0.0;
					section->snav_z_offset_int[section->num_snav-1]
						= 0.0;
					}
				}

			/* deal with new segment */
			if (kind == MB_DATA_DATA
				&& error == MB_ERROR_NO_ERROR
				&& new_segment == MB_YES)
				{
				/* end old segment */
				if (output_open == MB_YES)
					{
					/* close the swath file */
					status = mb_close(mbna_verbose,&ombio_ptr,&error);
					output_open = MB_NO;
					}

				/* allocate mbna_section array if needed */
				if (file->num_sections_alloc <= file->num_sections)
					{
					file->sections = (struct mbna_section *) realloc(file->sections,
						sizeof(struct mbna_section) * (file->num_sections_alloc + ALLOC_NUM));
					if (file->sections != NULL)
						file->num_sections_alloc += ALLOC_NUM;
					else
						{
						status = MB_FAILURE;
						error = MB_ERROR_MEMORY_FAIL;
						}
					}

				/* initialize new section */
				file->num_sections++;
				section = &file->sections[file->num_sections-1];
				section->num_pings = 0;
				section->num_beams = 0;
				section->continuity = MB_NO;
				section->global_start_ping = project.num_pings;
				section->global_start_snav = project.num_snavs;
				for (i=0;i<MBNA_MASK_DIM*MBNA_MASK_DIM;i++)
				    section->coverage[i] = 0;
				section->num_snav = 0;
				if (file->num_sections > 1)
					{
					csection = &file->sections[file->num_sections-2];
					if (fabs(time_d - csection->etime_d) < MBNA_TIME_GAP_MAX)
						{
						section->continuity = MB_YES;
						section->global_start_snav--;
						file->num_snavs--;
						project.num_snavs--;
						}
					}
				else if (project.num_files > 1 && firstfile == MB_NO)
					{
					cfile = &project.files[project.num_files-2];
					csection = &cfile->sections[cfile->num_sections-1];
					if (fabs(time_d - csection->etime_d) < MBNA_TIME_GAP_MAX)
						{
						section->continuity = MB_YES;
						section->global_start_snav--;
						file->num_snavs--;
						project.num_snavs--;
						}
					}
				section->distance = 0.0;
				section->btime_d = time_d;
				section->etime_d = time_d;
				section->lonmin = navlon;
				section->lonmax = navlon;
				section->latmin = navlat;
				section->latmax = navlat;
				section->depthmin = 0.0;
				section->depthmax = 0.0;
				section->contoursuptodate = MB_NO;
				section->global_tie_status = MBNA_TIE_NONE;
				section->global_tie_snav = MBNA_SELECT_NONE;
				section->global_tie_offset_x = 0.0;
				section->global_tie_offset_y = 0.0;
				section->global_tie_offset_x_m = 0.0;
				section->global_tie_offset_y_m = 0.0;
				section->global_tie_offset_z_m = 0.0;
				section->global_tie_xsigma = 0.0;
				section->global_tie_ysigma = 0.0;
				section->global_tie_zsigma = 0.0;
				new_segment = MB_NO;

				/* open output section file */
				sprintf(opath,"%s/nvs_%4.4d_%4.4d.mb71",
					project.datadir,file->id,file->num_sections-1);
				if ((status = mb_write_init(
					mbna_verbose,opath,71,&ombio_ptr,
					&obeams_bath,&obeams_amp,&opixels_ss,&error)) != MB_SUCCESS)
					{
					mb_error(mbna_verbose,error,&error_message);
					fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",error_message);
					fprintf(stderr,"\nSwath sonar File <%s> not initialized for writing\n",path);
					}
				else
					{
					omb_io_ptr = (struct mb_io_struct *) ombio_ptr;
					ostore_ptr = omb_io_ptr->store_data;
					ostore = (struct mbsys_ldeoih_struct *) ostore_ptr;
					ostore->kind = MB_DATA_DATA;
					ostore->beams_bath = obeams_bath;
					ostore->beams_amp = 0;
					ostore->pixels_ss = 0;
					ostore->kind = MB_DATA_DATA;
					output_open = MB_YES;
					status = mb_mallocd(mbna_verbose, __FILE__, __LINE__,obeams_bath*sizeof(char),
							(void **)&ostore->beamflag,&error);
					status = mb_mallocd(mbna_verbose, __FILE__, __LINE__,obeams_bath*sizeof(double),
							(void **)&ostore->bath,&error);
					status = mb_mallocd(mbna_verbose, __FILE__, __LINE__,obeams_bath*sizeof(double),
							(void **)&ostore->bath_acrosstrack,&error);
					status = mb_mallocd(mbna_verbose, __FILE__, __LINE__,obeams_bath*sizeof(double),
							(void **)&ostore->bath_alongtrack,&error);

					/* if error initializing memory then don't write the file */
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(mbna_verbose,error,&error_message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
							error_message);
						status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&ostore->beamflag,&error);
						status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&ostore->bath,&error);
						status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&ostore->bath_acrosstrack,&error);
						status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&ostore->bath_alongtrack,&error);
						status = mb_close(mbna_verbose,&ombio_ptr,&error);
						output_open = MB_NO;
						}
					}
				}

			/* update section distance for each data ping */
			if (kind == MB_DATA_DATA
				&& error == MB_ERROR_NO_ERROR
				&& section->num_pings > 1)
				section->distance += distance;

			/* handle good bathymetry */
			if (kind == MB_DATA_DATA
				&& error == MB_ERROR_NO_ERROR)
				{
				/* get statistics */
				mb_coor_scale(mbna_verbose,navlat,&mtodeglon,&mtodeglat);
				headingx = sin(DTR*heading);
				headingy = cos(DTR*heading);
				navlon_old = navlon;
				navlat_old = navlat;
				section->etime_d = time_d;
				section->num_pings++;
				file->num_pings++;
				project.num_pings++;
				new_pings++;
				if (section->distance >=
				    section->num_snav * project.section_length / (MBNA_SNAV_NUM - 1))
					{
					section->snav_id[section->num_snav]
						= section->num_pings - 1;
					section->snav_num_ties[section->num_snav]
						= 0;
					section->snav_distance[section->num_snav]
						= section->distance;
					section->snav_time_d[section->num_snav]
						= time_d;
					section->snav_lon[section->num_snav]
						= navlon;
					section->snav_lat[section->num_snav]
						= navlat;
					section->snav_lon_offset[section->num_snav]
						= 0.0;
					section->snav_lat_offset[section->num_snav]
						= 0.0;
					section->snav_z_offset[section->num_snav]
						= 0.0;
					section->snav_lon_offset_int[section->num_snav]
						= 0.0;
					section->snav_lat_offset_int[section->num_snav]
						= 0.0;
					section->snav_z_offset_int[section->num_snav]
						= 0.0;
					section->num_snav++;
					file->num_snavs++;
					project.num_snavs++;
					}
				for (i=0;i<beams_bath;i++)
					{
					if (mb_beam_ok(beamflag[i]) && bath[i] != 0.0)
						{
						good_beams++;
						project.num_beams++;
						file->num_beams++;
						section->num_beams++;
						lon = navlon
							+ headingy*mtodeglon
								* bathacrosstrack[i]
							+ headingx*mtodeglon
								* bathalongtrack[i];
						lat = navlat
							- headingx*mtodeglat
								* bathacrosstrack[i]
							+ headingy*mtodeglat
								* bathalongtrack[i];
						if (lon != 0.0) section->lonmin = MIN(section->lonmin,lon);
						if (lon != 0.0) section->lonmax = MAX(section->lonmax,lon);
						if (lat != 0.0) section->latmin = MIN(section->latmin,lat);
						if (lat != 0.0) section->latmax = MAX(section->latmax,lat);
						if (section->depthmin == 0.0)
							section->depthmin = bath[i];
						else
							section->depthmin = MIN(section->depthmin,bath[i]);
						if (section->depthmin == 0.0)
							section->depthmax = bath[i];
						else
							section->depthmax = MAX(section->depthmax,bath[i]);
						}
					else
						{
						beamflag[i] = MB_FLAG_NULL;
						bath[i] = 0.0;
						bathacrosstrack[i] = 0.0;
						bathalongtrack[i] = 0.0;
						}
					}

				/* write out bath data only to format 71 file */
				if (output_open == MB_YES)
					{
/*if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA)
fprintf(stderr,"%3d %4d/%2d/%2d %2.2d:%2.2d:%2.2d.%6.6d %10f %10f %5.2f %6.2f %7.3f %7.3f %4d %4d %4d\n",
file->num_sections,
time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
navlon,navlat,speed,heading,distance,section->distance,
beams_bath,beams_amp,pixels_ss);*/

					/* get depth and distance scaling */
					depthmax = 0.0;
					distmax = 0.0;
					for (i=0;i<beams_bath;i++)
					    {
					    depthmax = MAX(depthmax,
							fabs(bath[i]));
					    distmax = MAX(distmax,
							fabs(bathacrosstrack[i]));
					    distmax = MAX(distmax,
							fabs(bathalongtrack[i]));
					    }
					depthscale = MAX(0.001, depthmax / 32000);
					distscale = MAX(0.001, distmax / 32000);
					ostore->depth_scale = depthscale;
					ostore->distance_scale = distscale;
					ostore->sonardepth = draft - heave;
					ostore->roll = roll;
					ostore->pitch = pitch;
					ostore->heave = heave;

					/* write out data */
					status = mb_put_all(mbna_verbose,ombio_ptr,ostore_ptr,
							MB_YES,MB_DATA_DATA,
							time_i,time_d,
							navlon,navlat,speed,heading,
							beams_bath,0,0,
							beamflag,bath,amp,bathacrosstrack,bathalongtrack,
							ss,ssacrosstrack,ssalongtrack,
							comment,&error);
					}
				}

			/* write out all nav data to format 166 file */
			if ((kind == MB_DATA_DATA || kind == MB_DATA_NAV)
				&& time_d > 0.0 && time_i[0] > 0
				&& nfp != NULL)
				{
				/*fprintf(stderr, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f\r\n",
					time_i[0], time_i[1], time_i[2], time_i[3],
					time_i[4], time_i[5], time_i[6], time_d,
					navlon, navlat, heading, speed);
				fprintf(nfp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f\r\n",
					time_i[0], time_i[1], time_i[2], time_i[3],
					time_i[4], time_i[5], time_i[6], time_d,
					navlon, navlat, heading, speed);*/
				/*fprintf(stderr, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
						time_i[0], time_i[1], time_i[2], time_i[3],
						time_i[4], time_i[5], time_i[6], time_d,
						navlon, navlat, heading, speed,
						draft, roll, pitch, heave);*/
				fprintf(nfp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
						time_i[0], time_i[1], time_i[2], time_i[3],
						time_i[4], time_i[5], time_i[6], time_d,
						navlon, navlat, heading, speed,
						draft, roll, pitch, heave);
				}

			/* increment counters */
			if (error == MB_ERROR_NO_ERROR)
				nread++;

			/* print debug statements */
			if (mbna_verbose >= 2)
				{
				fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
					program_name);
				fprintf(stderr,"dbg2       kind:           %d\n",kind);
				fprintf(stderr,"dbg2       error:          %d\n",error);
				fprintf(stderr,"dbg2       status:         %d\n",status);
				}
			if (mbna_verbose >= 2 && kind == MB_DATA_COMMENT)
				{
				fprintf(stderr,"dbg2       comment:        %s\n",comment);
				}
			if (mbna_verbose >= 2 && error <= 0 && kind == MB_DATA_DATA)
				{
				fprintf(stderr,"dbg2       time_i:         %4d/%2d/%2d %2.2d:%2.2d:%2.2d.%6.6d\n",
							time_i[0],time_i[1],
							time_i[2],time_i[3],
							time_i[4],time_i[5],time_i[6]);
				fprintf(stderr,"dbg2       time_d:         %f\n",time_d);
				fprintf(stderr,"dbg2       navlon:         %.10f\n",navlon);
				fprintf(stderr,"dbg2       navlat:         %.10f\n",navlat);
				fprintf(stderr,"dbg2       speed:          %f\n",speed);
				fprintf(stderr,"dbg2       heading:        %f\n",heading);
				fprintf(stderr,"dbg2       distance:       %f\n",distance);
				fprintf(stderr,"dbg2       beams_bath:     %d\n",beams_bath);
				fprintf(stderr,"dbg2       beams_amp:      %d\n",beams_amp);
				fprintf(stderr,"dbg2       pixels_ss:      %d\n",pixels_ss);
				}
			}

		/* close the swath file */
		status = mb_close(mbna_verbose,&imbio_ptr,&error);
		if (nfp != NULL)
			fclose(nfp);
		if (output_open == MB_YES)
			{
			status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&ostore->beamflag,&error);
			status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&ostore->bath,&error);
			status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&ostore->bath_acrosstrack,&error);
			status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&ostore->bath_alongtrack,&error);
			status = mb_close(mbna_verbose,&ombio_ptr,&error);
			}

		/* deallocate bin arrays if needed */
		if (bin_nbath != NULL)
			{
			status = mb_freed(mbna_verbose, __FILE__, __LINE__,(void **)&bin_nbath,&error);
			status = mb_freed(mbna_verbose, __FILE__, __LINE__,(void **)&bin_bath,&error);
			status = mb_freed(mbna_verbose, __FILE__, __LINE__,(void **)&bin_bathacrosstrack,&error);
			status = mb_freed(mbna_verbose, __FILE__, __LINE__,(void **)&bin_bathalongtrack,&error);
			}

		/* get coverage masks for each section */
		if (file != NULL && first != MB_YES)
			{
			/* loop over all sections */
			for (k=0;k<file->num_sections;k++)
				{
				/* set section data to be read */
				section = (struct mbna_section *) &file->sections[k];
				sprintf(opath,"%s/nvs_%4.4d_%4.4d.mb71",
					project.datadir,file->id,k);

				/* initialize reading the swath file */
				if ((status = mb_read_init(
				    mbna_verbose,opath,71,1,lonflip,bounds,
				    btime_i,etime_i,speedmin,timegap,
				    &ombio_ptr,&btime_d,&etime_d,
				    &beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
				    {
				    mb_error(mbna_verbose,error,&error_message);
				    fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",error_message);
				    fprintf(stderr,"\nSwath sonar File <%s> not initialized for reading\n",path);
				    }

				/* allocate memory for data arrays */
				if (status == MB_SUCCESS)
				    {
				    beamflag = NULL;
				    bath = NULL;
				    amp = NULL;
				    bathacrosstrack = NULL;
				    bathalongtrack = NULL;
				    ss = NULL;
				    ssacrosstrack = NULL;
				    ssalongtrack = NULL;
				    if (error == MB_ERROR_NO_ERROR)
					    status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY,
									    sizeof(char), (void **)&beamflag, &error);
				    if (error == MB_ERROR_NO_ERROR)
					    status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY,
									    sizeof(double), (void **)&bath, &error);
				    if (error == MB_ERROR_NO_ERROR)
					    status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_AMPLITUDE,
									    sizeof(double), (void **)&amp, &error);
				    if (error == MB_ERROR_NO_ERROR)
					    status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY,
									    sizeof(double), (void **)&bathacrosstrack, &error);
				    if (error == MB_ERROR_NO_ERROR)
					    status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY,
									    sizeof(double), (void **)&bathalongtrack, &error);
				    if (error == MB_ERROR_NO_ERROR)
					    status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_SIDESCAN,
									    sizeof(double), (void **)&ss, &error);
				    if (error == MB_ERROR_NO_ERROR)
					    status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_SIDESCAN,
									    sizeof(double), (void **)&ssacrosstrack, &error);
				    if (error == MB_ERROR_NO_ERROR)
					    status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_SIDESCAN,
									    sizeof(double), (void **)&ssalongtrack, &error);

				    /* if error initializing memory then don't read the file */
				    if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(mbna_verbose,error,&error_message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
						error_message);
					}
				    }

				/* loop over reading data */
				dx1 = (section->lonmax - section->lonmin) / MBNA_MASK_DIM;
				dy1 = (section->latmax - section->latmin) / MBNA_MASK_DIM;
				while (error <= MB_ERROR_NO_ERROR)
				    {
				    /* read a ping of data */
				    status = mb_get_all(mbna_verbose,ombio_ptr,&ostore_ptr,&kind,
					    time_i,&time_d,&navlon,&navlat,&speed,
					    &heading,&distance,&altitude,&sonardepth,
					    &beams_bath,&beams_amp,&pixels_ss,
					    beamflag,bath,amp,bathacrosstrack,bathalongtrack,
					    ss,ssacrosstrack,ssalongtrack,
					    comment,&error);

				    /* ignore minor errors */
				    if (kind == MB_DATA_DATA
					    && (error == MB_ERROR_TIME_GAP
						    || error == MB_ERROR_OUT_BOUNDS
						    || error == MB_ERROR_OUT_TIME
						    || error == MB_ERROR_SPEED_TOO_SMALL))
					{
					status = MB_SUCCESS;
					error = MB_ERROR_NO_ERROR;
					}

				    /* check for good bathymetry */
				    if (kind == MB_DATA_DATA
					    && error == MB_ERROR_NO_ERROR)
					{
					mb_coor_scale(mbna_verbose,navlat,&mtodeglon,&mtodeglat);
					headingx = sin(DTR*heading);
					headingy = cos(DTR*heading);
					for (i=0;i<beams_bath;i++)
					    {
					    if (mb_beam_ok(beamflag[i]) && bath[i] != 0.0)
						{
						lon = navlon
							+ headingy*mtodeglon
								* bathacrosstrack[i]
							+ headingx*mtodeglon
								* bathalongtrack[i];
						lat = navlat
							- headingx*mtodeglat
								* bathacrosstrack[i]
							+ headingy*mtodeglat
								* bathalongtrack[i];
						ii1 = (lon - section->lonmin) / dx1;
						jj1 = (lat - section->latmin) / dy1;
						if (ii1 >= 0 && ii1 < MBNA_MASK_DIM
						    && jj1 >= 0 && jj1 < MBNA_MASK_DIM)
						    {
						    section->coverage[ii1 + jj1 * MBNA_MASK_DIM] = 1;
						    }
						}
					    }
					}
				    }
/*fprintf(stderr,"%s/nvs_%4.4d_%4.4d.mb71\n",
project.datadir,file->id,k);
for (jj1=MBNA_MASK_DIM-1;jj1>=0;jj1--)
{
for (ii1=0;ii1<MBNA_MASK_DIM;ii1++)
{
kk1 = ii1 + jj1 * MBNA_MASK_DIM;
fprintf(stderr, "%1d", section->coverage[kk1]);
}
fprintf(stderr, "\n");
}*/

				/* deallocate memory used for data arrays */
				status = mb_close(mbna_verbose,&ombio_ptr,&error);
				}
			}
		}

	/* add info text */
	if (status == MB_SUCCESS && new_pings > 0)
		{
		sprintf(message, "Imported format %d file: %s\n > Read %d pings\n > Added %d sections %d crossings\n",
			iformat, path, new_pings, file->num_sections, new_crossings);
		do_info_add(message, MB_YES);
		}
	else
		{
		sprintf(message, "Unable to import format %d file: %s\n",
			iformat, path);
		do_info_add(message, MB_YES);
		}

	/* turn off message */
	do_message_off();

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_bin_bathymetry(double altitude, int beams_bath, char *beamflag, double *bath,
				double *bathacrosstrack, double *bathalongtrack,
				int mbna_bin_beams_bath, double mbna_bin_pseudobeamwidth,
				double mbna_bin_swathwidth, char *bin_beamflag,
				double *bin_bath, double *bin_bathacrosstrack, double *bin_bathalongtrack,
				int *error)
{
	/* local variables */
	char	*function_name = "mbnavadjust_bin_bathymetry";
	int	status = MB_SUCCESS;
	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2                       mbna_verbose: %d\n",mbna_verbose);
		fprintf(stderr,"dbg2                       altitude:     %f\n",altitude);
		fprintf(stderr,"dbg2                       beams_bath:   %d\n",beams_bath);
		for (i=0;i<beams_bath;i++)
			fprintf(stderr,"dbg2                       beam[%d]: %f %f %f %d\n",
				i,bath[i],bathacrosstrack[i],bathalongtrack[i],beamflag[i]);
		fprintf(stderr,"dbg2                       mbna_bin_beams_bath:      %d\n",mbna_bin_beams_bath);
		fprintf(stderr,"dbg2                       mbna_bin_pseudobeamwidth: %f\n",mbna_bin_pseudobeamwidth);
		fprintf(stderr,"dbg2                       mbna_bin_swathwidth:      %f\n",mbna_bin_swathwidth);
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		for (i=0;i<mbna_bin_beams_bath;i++)
			fprintf(stderr,"dbg2                       beam[%d]: %f %f %f %d\n",
				i,bin_bath[i],bin_bathacrosstrack[i],bin_bathalongtrack[i],bin_beamflag[i]);
		fprintf(stderr,"dbg2       error:       %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_findcrossings()
{
	/* local variables */
	char	*function_name = "mbnavadjust_findcrossings";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;
	int	ifile, icrossing;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* turn on message */
	sprintf(message,"Checking for crossings...");
	do_message_on(message);

     	/* loop over files looking for new crossings */
    	if (project.open == MB_YES
    		&& project.num_files > 0)
		{
		/* look for new crossings through all files */
		for (ifile=0;ifile<project.num_files;ifile++)
			{
			sprintf(message,"Checking for crossings with file %d of %d...",ifile,project.num_files);
			do_message_update(message);

			status = mbnavadjust_findcrossingsfile(ifile);
			}

		/* resort crossings */
		sprintf(message,"Sorting crossings....");
		do_message_update(message);
		if (project.num_crossings > 1)
		mb_mergesort((void *)project.crossings, (size_t)project.num_crossings, sizeof(struct mbna_crossing), mbnavadjust_crossing_compare);

		/* recalculate overlap fractions, true crossings, good crossing statistics */
		sprintf(message,"Calculating crossing overlaps...");
		do_message_update(message);

		project.num_crossings_analyzed = 0;
		project.num_goodcrossings = 0;
		project.num_truecrossings = 0;
		project.num_truecrossings_analyzed = 0;
		for (icrossing=0;icrossing<project.num_crossings;icrossing++)
			{
			crossing = &(project.crossings[icrossing]);

			/* recalculate crossing overlap */
			mbnavadjust_crossing_overlap(icrossing);
			if (crossing->overlap >= 25)
				project.num_goodcrossings++;

			/* check if this is a true crossing */
			if (mbnavadjust_sections_intersect(icrossing) == MB_YES)
				{
				crossing->truecrossing = MB_YES;
				project.num_truecrossings++;
				if (crossing->status != MBNA_CROSSING_STATUS_NONE)
					project.num_truecrossings_analyzed++;
				}
			else
				crossing->truecrossing = MB_NO;
			if (crossing->status != MBNA_CROSSING_STATUS_NONE)
				project.num_crossings_analyzed++;
			}
		}

	/* turn off message */
	do_message_off();

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_compare(const void *a, const void *b)
{
	struct mbna_crossing	*aa, *bb;
	int a1id, a2id, b1id, b2id;
	int aid, bid;

	aa = (struct mbna_crossing *) a;
	bb = (struct mbna_crossing *) b;

	a1id = aa->file_id_1 * 1000 + aa->section_1;
	a2id = aa->file_id_2 * 1000 + aa->section_2;
	if (a1id > a2id)
		aid = a1id;
	else
		aid = a2id;

	b1id = bb->file_id_1 * 1000 + bb->section_1;
	b2id = bb->file_id_2 * 1000 + bb->section_2;
	if (b1id > b2id)
		bid = b1id;
	else
		bid = b2id;

	if (aid > bid)
		return(1);
	else if (aid < bid)
		return(-1);
	else if (a1id > b1id)
		return(1);
	else if (a1id < b1id)
		return(-1);
	else if (a2id > b2id)
		return(1);
	else if (a2id < b2id)
		return(-1);
	else
		return(0);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_findcrossingsfile(int ifile)
{
	/* local variables */
	char	*function_name = "mbnavadjust_findcrossingsfile";
	int	status = MB_SUCCESS;
	struct mbna_file *file1, *file2;
	struct mbna_section *section1, *section2;
	struct mbna_crossing *crossing;
	int	found, overlap, disqualify;
	int	isection, jfile, jsection, jsectionmax, icrossing;
	double	lonoffset1, latoffset1, lonoffset2, latoffset2;
	double	lonmin1, lonmax1, latmin1, latmax1;
	double	lonmin2, lonmax2, latmin2, latmax2;
	double	cell1lonmin, cell1lonmax, cell1latmin, cell1latmax;
	double	cell2lonmin, cell2lonmax, cell2latmin, cell2latmax;
	double	dx1, dy1, dx2, dy2;
	int	ii1, jj1, kk1, ii2, jj2, kk2;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2                       ifile: %d\n",ifile);
		}

     	/* loop over files and sections, comparing sections from project.files[ifile] with all previous sections */
    	if (project.open == MB_YES
    		&& project.num_files > 0)
		{
		file2 = &(project.files[ifile]);

		/* loop over all sections */
		for (isection=0;isection<file2->num_sections;isection++)
			{
			section2 = &(file2->sections[isection]);
			lonoffset2 = section2->snav_lon_offset[section2->num_snav/2];
			latoffset2 = section2->snav_lat_offset[section2->num_snav/2];

			/* get coverage mask adjusted for most recent inversion solution */
			lonmin2 = section2->lonmin + lonoffset2;
			lonmax2 = section2->lonmax + lonoffset2;
			latmin2 = section2->latmin + latoffset2;
			latmax2 = section2->latmax + latoffset2;
			dx2 = (section2->lonmax - section2->lonmin) / (MBNA_MASK_DIM - 1);
			dy2 = (section2->latmax - section2->latmin) / (MBNA_MASK_DIM - 1);

			/* now loop over all previous sections looking for crossings */
			for (jfile=0;jfile<=ifile;jfile++)
				{
				file1 = &(project.files[jfile]);
				if (jfile < ifile)
					jsectionmax = file1->num_sections;
				else
					jsectionmax = isection;
				for (jsection=0;jsection<jsectionmax;jsection++)
					{
					section1 = &(file1->sections[jsection]);
					lonoffset1 = section1->snav_lon_offset[section1->num_snav/2];
					latoffset1 = section1->snav_lat_offset[section1->num_snav/2];

					/* get coverage mask adjusted for most recent inversion solution */
					lonmin1 = section1->lonmin + lonoffset1;
					lonmax1 = section1->lonmax + lonoffset1;
					latmin1 = section1->latmin + latoffset1;
					latmax1 = section1->latmax + latoffset1;
					dx1 = (section1->lonmax - section1->lonmin) / (MBNA_MASK_DIM - 1);
					dy1 = (section1->latmax - section1->latmin) / (MBNA_MASK_DIM - 1);

					/* check if there is overlap given the current navigation model */
					overlap = 0;
					disqualify = MB_NO;
					if (jfile == ifile && jsection == isection - 1 && section2->continuity == MB_YES)
						disqualify = MB_YES;
					else if (jfile == ifile - 1 && jsection == file1->num_sections - 1 && isection == 0
						&& section2->continuity == MB_YES)
						disqualify = MB_YES;
					else if (!(lonmin2 < lonmax1 && lonmax2 > lonmin1
							&& latmin2 < latmax1 && latmax2 > latmin1))
						{
						disqualify = MB_YES;
						}
					else
						{
						/* loop over the coverage mask cells looking for overlap */
						for (ii2=0;ii2<MBNA_MASK_DIM && overlap == 0;ii2++)
						for (jj2=0;jj2<MBNA_MASK_DIM && overlap == 0;jj2++)
							{
							kk2 = ii2 + jj2 * MBNA_MASK_DIM;
							if (section2->coverage[kk2] == 1)
								{
								cell2lonmin = lonmin2 + ii2 * dx2;
								cell2lonmax = lonmin2 + (ii2 + 1) * dx2;
								cell2latmin = latmin2 + jj2 * dy2;
								cell2latmax = latmin2 + (jj2 + 1) * dy2;

								for (ii1=0;ii1<MBNA_MASK_DIM && overlap == 0;ii1++)
								for (jj1=0;jj1<MBNA_MASK_DIM && overlap == 0;jj1++)
									{
									kk1 = ii1 + jj1 * MBNA_MASK_DIM;
									if (section1->coverage[kk1] == 1)
										{
										cell1lonmin = lonmin1 + ii1 * dx1;
										cell1lonmax = lonmin1 + (ii1 + 1) * dx1;
										cell1latmin = latmin1 + jj1 * dy2;
										cell1latmax = latmin1 + (jj1 + 1) * dy1;

										/* check if these two cells overlap */
										if (cell2lonmin < cell1lonmax && cell2lonmax > cell1lonmin
											&& cell2latmin < cell1latmax && cell2latmax > cell1latmin)
											{
											overlap++;
											}
										}
									}
								}
							}
						}

					/* if not disqualified and overlap found, then this is a crossing */
					/* check to see if the crossing exists, if not add it */
					if (disqualify == MB_NO && overlap > 0)
						{
						found = MB_NO;
						for (icrossing=0;icrossing<project.num_crossings && found == MB_NO;icrossing++)
							{
							crossing = &(project.crossings[icrossing]);
							if (crossing->file_id_2 == ifile && crossing->file_id_1 == jfile
								&& crossing->section_2 == isection && crossing->section_1 == jsection)
								{
								found = MB_YES;
								}
							else if (crossing->file_id_1 == ifile && crossing->file_id_2 == jfile
								&& crossing->section_1 == isection && crossing->section_2 == jsection)
								{
								found = MB_YES;
								}
							}
						if (found == MB_NO)
							{
							/* allocate mbna_crossing array if needed */
							if (project.num_crossings_alloc <= project.num_crossings)
							    {
							    project.crossings = (struct mbna_crossing *) realloc(project.crossings,
									    sizeof(struct mbna_crossing) * (project.num_crossings_alloc + ALLOC_NUM));
							    if (project.crossings != NULL)
								    project.num_crossings_alloc += ALLOC_NUM;
							    else
								{
								status = MB_FAILURE;
								error = MB_ERROR_MEMORY_FAIL;
								}
							    }

							/* add crossing to list */
							crossing = (struct mbna_crossing *) &project.crossings[project.num_crossings];
							crossing->status = MBNA_CROSSING_STATUS_NONE;
							crossing->truecrossing = MB_NO;
							crossing->overlap = 0;
							crossing->file_id_1 = file1->id;
							crossing->section_1 = jsection;
							crossing->file_id_2 = file2->id;
							crossing->section_2 = isection;
							crossing->num_ties = 0;
							project.num_crossings++;

fprintf(stderr,"added crossing: %d  %4d %4d   %4d %4d\n",
project.num_crossings-1,
crossing->file_id_1,crossing->section_1,
crossing->file_id_2,crossing->section_2);
							}
/*else
fprintf(stderr,"no new crossing:    %4d %4d   %4d %4d   duplicate\n",
file2->id,isection,
file1->id,jsection);*/
						}
/*else
fprintf(stderr,"disqualified:       %4d %4d   %4d %4d   disqualify:%d overlaptxt list:%d\n",
file2->id,isection,
file1->id,jsection, disqualify, overlap);*/

					}
				}
			}
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_poornav_file()
{
	/* local variables */
	char	*function_name = "mbnavadjust_poornav_file";
	int	status = MB_SUCCESS;
	int	block;
 	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* set selected file's block to poor nav */
    	if (project.open == MB_YES
    		&& project.num_files > 0
		&& mbna_file_select >= 0
		&& mbna_file_select < project.num_files)
		{
		/* get affected survey block */
		if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS)
			{
			block = mbna_survey_select;
			}
		else if (mbna_view_list == MBNA_VIEW_LIST_FILES)
			{
			block = project.files[mbna_file_select].block;
			}

		/* set all files in block of selected file to poor nav */
		for (i=0;i<project.num_files;i++)
			{
			if (project.files[i].block == block)
				{
				project.files[i].status = MBNA_FILE_POORNAV;
				}
			}

		/* write out updated project */
		mbnavadjust_write_project();

		/* add info text */
		sprintf(message, "Set file %d to have poor nav: %s\n",
			    mbna_file_select,project.files[mbna_file_select].file);
		do_info_add(message, MB_YES);
   		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_goodnav_file()
{
	/* local variables */
	char	*function_name = "mbnavadjust_goodnav_file";
	int	status = MB_SUCCESS;
 	int	block;
	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* set selected file's block to good nav */
    	if (project.open == MB_YES
    		&& project.num_files > 0
		&& mbna_file_select >= 0
		&& mbna_file_select < project.num_files)
		{
		/* get affected survey block */
		if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS)
			{
			block = mbna_survey_select;
			}
		else if (mbna_view_list == MBNA_VIEW_LIST_FILES)
			{
			block = project.files[mbna_file_select].block;
			}

		/* set all files in block of selected file to good nav */
		for (i=0;i<project.num_files;i++)
			{
			if (project.files[i].block == block)
				{
				project.files[i].status = MBNA_FILE_GOODNAV;
				}
			}
		if (project.inversion == MBNA_INVERSION_CURRENT)
			project.inversion = MBNA_INVERSION_OLD;

		/* write out updated project */
		mbnavadjust_write_project();

		/* add info text */
		sprintf(message, "Set file %d to have good nav: %s\n",
			    mbna_file_select,project.files[mbna_file_select].file);
		do_info_add(message, MB_YES);
   		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_fixednav_file()
{
	/* local variables */
	char	*function_name = "mbnavadjust_fixednav_file";
	int	status = MB_SUCCESS;
 	int	block;
	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 0)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* set selected file's block to good nav */
    	if (project.open == MB_YES
    		&& project.num_files > 0
		&& mbna_file_select >= 0
		&& mbna_file_select < project.num_files)
		{
		/* get affected survey block */
		if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS)
			{
			block = mbna_survey_select;
			}
		else if (mbna_view_list == MBNA_VIEW_LIST_FILES)
			{
			block = project.files[mbna_file_select].block;
			}

		/* set all files in block of selected file to fixed nav */
		for (i=0;i<project.num_files;i++)
			{
			if (project.files[i].block == block)
				{
				project.files[i].status = MBNA_FILE_FIXEDNAV;
				fprintf(stderr,"Set file to have fixed nav: %d %s\n",i,project.files[i].file);
				}
			}
		if (project.inversion == MBNA_INVERSION_CURRENT)
			project.inversion = MBNA_INVERSION_OLD;

		/* write out updated project */
		mbnavadjust_write_project();

		/* add info text */
		sprintf(message, "Set file %d to have fixed nav: %s\n",
			    mbna_file_select,project.files[mbna_file_select].file);
		do_info_add(message, MB_YES);
   		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_fixedxynav_file()
{
	/* local variables */
	char	*function_name = "mbnavadjust_fixedxynav_file";
	int	status = MB_SUCCESS;
 	int	block;
	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 0)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* set selected file's block to good nav */
    	if (project.open == MB_YES
    		&& project.num_files > 0
		&& mbna_file_select >= 0
		&& mbna_file_select < project.num_files)
		{
		/* get affected survey block */
		if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS)
			{
			block = mbna_survey_select;
			}
		else if (mbna_view_list == MBNA_VIEW_LIST_FILES)
			{
			block = project.files[mbna_file_select].block;
			}

		/* set all files in block of selected file to fixed nav */
		for (i=0;i<project.num_files;i++)
			{
			if (project.files[i].block == block)
				{
				project.files[i].status = MBNA_FILE_FIXEDXYNAV;
				fprintf(stderr,"Set file to have fixed xy nav: %d %s\n",i,project.files[i].file);
				}
			}
		if (project.inversion == MBNA_INVERSION_CURRENT)
			project.inversion = MBNA_INVERSION_OLD;

		/* write out updated project */
		mbnavadjust_write_project();

		/* add info text */
		sprintf(message, "Set file %d to have fixed xy nav: %s\n",
			    mbna_file_select,project.files[mbna_file_select].file);
		do_info_add(message, MB_YES);
   		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_fixedznav_file()
{
	/* local variables */
	char	*function_name = "mbnavadjust_fixedznav_file";
	int	status = MB_SUCCESS;
 	int	block;
	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 0)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* set selected file's block to good nav */
    	if (project.open == MB_YES
    		&& project.num_files > 0
		&& mbna_file_select >= 0
		&& mbna_file_select < project.num_files)
		{
		/* get affected survey block */
		if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS)
			{
			block = mbna_survey_select;
			}
		else if (mbna_view_list == MBNA_VIEW_LIST_FILES)
			{
			block = project.files[mbna_file_select].block;
			}

		/* set all files in block of selected file to fixed nav */
		for (i=0;i<project.num_files;i++)
			{
			if (project.files[i].block == block)
				{
				project.files[i].status = MBNA_FILE_FIXEDZNAV;
				fprintf(stderr,"Set file to have fixed z nav: %d %s\n",i,project.files[i].file);
				}
			}
		if (project.inversion == MBNA_INVERSION_CURRENT)
			project.inversion = MBNA_INVERSION_OLD;

		/* write out updated project */
		mbnavadjust_write_project();

		/* add info text */
		sprintf(message, "Set file %d to have fixed z nav: %s\n",
			    mbna_file_select,project.files[mbna_file_select].file);
		do_info_add(message, MB_YES);
   		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_set_tie_xyz()
{
	/* local variables */
	char	*function_name = "mbnavadjust_set_tie_xyz";
	int	status = MB_SUCCESS;
    	struct mbna_crossing *crossing;
    	struct mbna_tie *tie;

 	/* print input debug statements */
	if (mbna_verbose >= 0)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* set selected file's block to good nav */
    	if (project.open == MB_YES
    		&& project.num_files > 0
		&& mbna_crossing_select >= 0
		&& mbna_tie_select >= 0)
		{
		/* set tie to fix xyz */
		crossing = &(project.crossings[mbna_crossing_select]);
		tie = (struct mbna_tie *) &crossing->ties[mbna_tie_select];
		tie->status = MBNA_TIE_XYZ;
		fprintf(stderr,"Set crossing %d tie %d to fix XYZ\n",mbna_crossing_select,mbna_tie_select);
		if (project.inversion == MBNA_INVERSION_CURRENT)
			project.inversion = MBNA_INVERSION_OLD;

		/* write out updated project */
		mbnavadjust_write_project();

		/* add info text */
		sprintf(message, "Set crossing %d tie %d to fix XYZ\n",
			    mbna_crossing_select,mbna_tie_select);
		do_info_add(message, MB_YES);
   		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_set_tie_xy()
{
	/* local variables */
	char	*function_name = "mbnavadjust_set_tie_xy";
	int	status = MB_SUCCESS;
    	struct mbna_crossing *crossing;
    	struct mbna_tie *tie;

 	/* print input debug statements */
	if (mbna_verbose >= 0)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* set selected file's block to good nav */
    	if (project.open == MB_YES
    		&& project.num_files > 0
		&& mbna_crossing_select >= 0
		&& mbna_tie_select >= 0)
		{
		/* set tie to fix xy */
		crossing = &(project.crossings[mbna_crossing_select]);
		tie = (struct mbna_tie *) &crossing->ties[mbna_tie_select];
		tie->status = MBNA_TIE_XY;
		fprintf(stderr,"Set crossing %d tie %d to fix XY\n",mbna_crossing_select,mbna_tie_select);
		if (project.inversion == MBNA_INVERSION_CURRENT)
			project.inversion = MBNA_INVERSION_OLD;

		/* write out updated project */
		mbnavadjust_write_project();

		/* add info text */
		sprintf(message, "Set crossing %d tie %d to fix XY\n",
			    mbna_crossing_select,mbna_tie_select);
		do_info_add(message, MB_YES);
   		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_set_tie_z()
{
	/* local variables */
	char	*function_name = "mbnavadjust_set_tie_z";
	int	status = MB_SUCCESS;
    	struct mbna_crossing *crossing;
    	struct mbna_tie *tie;

 	/* print input debug statements */
	if (mbna_verbose >= 0)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* set selected file's block to good nav */
    	if (project.open == MB_YES
    		&& project.num_files > 0
		&& mbna_crossing_select >= 0
		&& mbna_tie_select >= 0)
		{
		/* set tie to fix z */
		crossing = &(project.crossings[mbna_crossing_select]);
		tie = (struct mbna_tie *) &crossing->ties[mbna_tie_select];
		tie->status = MBNA_TIE_Z;
		fprintf(stderr,"Set crossing %d tie %d to fix Z\n",mbna_crossing_select,mbna_tie_select);
		if (project.inversion == MBNA_INVERSION_CURRENT)
			project.inversion = MBNA_INVERSION_OLD;

		/* write out updated project */
		mbnavadjust_write_project();

		/* add info text */
		sprintf(message, "Set crossing %d tie %d to fix Z\n",
			    mbna_crossing_select,mbna_tie_select);
		do_info_add(message, MB_YES);
   		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_save()
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_save";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* save offsets if crossing loaded and ties set */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& mbna_naverr_load == MB_YES
		&& mbna_current_crossing >= 0
		&& mbna_current_tie >= 0)
    		{
		/* save offsets if ties set */
		crossing = &project.crossings[mbna_current_crossing];
		if (crossing->num_ties > 0
		    && mbna_current_tie >= 0)
		    {
		    /* get relevant tie */
		    tie = &crossing->ties[mbna_current_tie];

		    /* reset tie counts for snavs */
		    file = &project.files[crossing->file_id_1];
		    section = &file->sections[crossing->section_1];
		    section->snav_num_ties[tie->snav_1]--;
		    file = &project.files[crossing->file_id_2];
		    section = &file->sections[crossing->section_2];
		    section->snav_num_ties[tie->snav_2]--;

		    /* get new tie values */
/* fprintf(stderr, "tie %d of crossing %d saved...\n", mbna_current_tie, mbna_current_crossing); */
		    /* tie->status = tie->status; */
		    tie->snav_1 = mbna_snav_1;
		    tie->snav_1_time_d = mbna_snav_1_time_d;
		    tie->snav_2 = mbna_snav_2;
		    tie->snav_2_time_d = mbna_snav_2_time_d;
		    if (tie->inversion_status == MBNA_INVERSION_CURRENT
			&& (tie->offset_x != mbna_offset_x
			    || tie->offset_y != mbna_offset_y
			    || tie->offset_z_m != mbna_offset_z))
			{
			tie->inversion_status = MBNA_INVERSION_OLD;
			}
		    tie->offset_x = mbna_offset_x;
		    tie->offset_y = mbna_offset_y;
		    tie->offset_x_m = mbna_offset_x / mbna_mtodeglon;
		    tie->offset_y_m = mbna_offset_y / mbna_mtodeglat;
		    tie->offset_z_m = mbna_offset_z;
		    tie->sigmar1 = mbna_minmisfit_sr1;
		    tie->sigmar2 = mbna_minmisfit_sr2;
		    tie->sigmar3 = mbna_minmisfit_sr3;
		    for (i=0;i<3;i++)
			    {
			    tie->sigmax1[i] = mbna_minmisfit_sx1[i];
			    tie->sigmax2[i] = mbna_minmisfit_sx2[i];
			    tie->sigmax3[i] = mbna_minmisfit_sx3[i];
			    }
		    if (project.inversion == MBNA_INVERSION_CURRENT)
			    project.inversion = MBNA_INVERSION_OLD;

		    /* reset tie counts for snavs */
		    file = &project.files[crossing->file_id_1];
		    section = &file->sections[crossing->section_1];
		    section->snav_num_ties[tie->snav_1]++;
		    file = &project.files[crossing->file_id_2];
		    section = &file->sections[crossing->section_2];
		    section->snav_num_ties[tie->snav_2]++;

		    /* write updated project */
		    mbnavadjust_write_project();

		    /* add info text */
		    sprintf(message,"Save Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f %f m\n",
			    mbna_current_tie, mbna_current_crossing,
			    crossing->file_id_1, crossing->section_1, tie->snav_1,
			    crossing->file_id_2, crossing->section_2, tie->snav_2,
			    tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
		    if (mbna_verbose == 0)
			    fprintf(stderr,"%s",message);
		    do_info_add(message, MB_YES);
		    }
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_specific(int new_crossing, int new_tie)
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_specific";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2               new_crossing: %d\n",new_crossing);
		fprintf(stderr,"dbg2               new_tie:      %d\n",new_tie);
		}

     	/* get current crossing */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
    		/* get next crossing */
		if (new_crossing >= 0
		    && new_crossing < project.num_crossings)
			{
    			mbna_current_crossing = new_crossing;
			if (new_tie >= 0
			    && new_tie < project.crossings[mbna_current_crossing].num_ties)
			    mbna_current_tie = new_tie;
			else
			    mbna_current_tie = -1;
			}
		else
			{
    			mbna_current_crossing = 0;
			mbna_current_tie = -1;
			}

    		/* retrieve crossing parameters */
    		if (mbna_current_crossing >= 0)
    			{
    			crossing = &project.crossings[mbna_current_crossing];
    			mbna_file_id_1 = crossing->file_id_1;
    			mbna_section_1 = crossing->section_1;
     			mbna_file_id_2 = crossing->file_id_2;
    			mbna_section_2 = crossing->section_2;
			if (crossing->num_ties > 0)
			    {
			    if (mbna_current_tie < 0)
				mbna_current_tie = 0;
			    tie = &crossing->ties[mbna_current_tie];
			    mbna_snav_1 = tie->snav_1;
			    mbna_snav_1_time_d = tie->snav_1_time_d;
			    mbna_snav_2 = tie->snav_2;
			    mbna_snav_2_time_d = tie->snav_2_time_d;
			    mbna_offset_x = tie->offset_x;
			    mbna_offset_y = tie->offset_y;
			    mbna_offset_z = tie->offset_z_m;
/* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */
			    }
			else
			    {
			    mbna_current_tie = -1;
			    }

			/* reset survey file and section selections */
			if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY
			    || mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
				{
				if (mbna_survey_select == project.files[crossing->file_id_1].block)
					{
					mbna_file_select = crossing->file_id_1;
					mbna_section_select = crossing->section_1;
					}
				else if (mbna_survey_select == project.files[crossing->file_id_2].block)
					{
					mbna_file_select = crossing->file_id_2;
					mbna_section_select = crossing->section_2;
					}
				else
					{
					mbna_file_select = crossing->file_id_1;
					mbna_section_select = crossing->section_1;
					}
				}
			else if (mbna_view_mode == MBNA_VIEW_MODE_FILE
			    || mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
				{
				if (mbna_file_select == crossing->file_id_1)
					{
					mbna_survey_select = project.files[crossing->file_id_1].block;
					mbna_section_select = crossing->section_1;
					}
				else if (mbna_file_select == crossing->file_id_2)
					{
					mbna_survey_select = project.files[crossing->file_id_2].block;
					mbna_section_select = crossing->section_2;
					}
				else
					{
					mbna_survey_select = project.files[crossing->file_id_1].block;
					mbna_section_select = crossing->section_1;
					}
				}
			else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
				{
				if (mbna_file_select == crossing->file_id_1
				    && mbna_section_select == crossing->section_1)
					{
					mbna_survey_select = project.files[crossing->file_id_1].block;
					mbna_file_select = crossing->file_id_1;
					}
				else if (mbna_file_select == crossing->file_id_2
				    && mbna_section_select == crossing->section_2)
					{
					mbna_survey_select = project.files[crossing->file_id_2].block;
					mbna_file_select = crossing->file_id_2;
					}
				else
					{
					mbna_survey_select = project.files[crossing->file_id_1].block;
					mbna_file_select = crossing->file_id_1;
					}
				}
 			}

  		/* load the crossing */
  		if (mbna_current_crossing >= 0)
  			{
			/* put up message */
			sprintf(message,"Loading crossing %d...",mbna_current_crossing);
			do_message_on(message);

  			mbnavadjust_crossing_load();

			/* turn off message */
			do_message_off();
  			}
   		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_next()
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_next";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	crossing_ok;
	int	i, j, k;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* find next current crossing */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
    		/* get next good crossing */
		j = -1;
		k = -1;
		for (i=0;i<project.num_crossings;i++)
			{
			crossing = &(project.crossings[i]);
			crossing_ok = MB_NO;
			if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_MEDIOCRECROSSINGS
				&& crossing->overlap >= MBNA_MEDIOCREOVERLAP_THRESHOLD)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS
				&& crossing->overlap >= MBNA_GOODOVERLAP_THRESHOLD)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS
				&& crossing->overlap >= MBNA_BETTEROVERLAP_THRESHOLD)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS
				&& crossing->truecrossing == MB_YES)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_TIES
				&& crossing->num_ties > 0)
				crossing_ok = MB_YES;
			if ((mbna_view_mode == MBNA_VIEW_MODE_SURVEY
					&& (mbna_survey_select != project.files[crossing->file_id_1].block
						|| mbna_survey_select != project.files[crossing->file_id_2].block))
				|| (mbna_view_mode == MBNA_VIEW_MODE_FILE
					&& (mbna_file_select != crossing->file_id_1
						|| mbna_file_select != crossing->file_id_2))
				|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY
					&& mbna_survey_select != project.files[crossing->file_id_1].block
					&& mbna_survey_select != project.files[crossing->file_id_2].block)
				|| (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE
					&& mbna_file_select != crossing->file_id_1
					&& mbna_file_select != crossing->file_id_2)
				|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION
					&& (mbna_file_select != crossing->file_id_1
						|| mbna_section_select != crossing->section_1)
					&& (mbna_file_select != crossing->file_id_2
						|| mbna_section_select != crossing->section_2)))
				crossing_ok = MB_NO;

			if (crossing_ok == MB_YES)
				{
				if (j < 0)
					j = i;
				if (k < 0 && i > mbna_current_crossing)
					k = i;
				}
			}
		if (k >= 0)
			mbna_current_crossing = k;
		else if (j >= 0)
			mbna_current_crossing = j;
		else
			mbna_current_crossing = -1;
		mbna_current_tie = -1;
   		}

    	/* retrieve crossing parameters */
    	if (mbna_current_crossing >= 0)
    		{
    		crossing = &project.crossings[mbna_current_crossing];
    		mbna_file_id_1 = crossing->file_id_1;
    		mbna_section_1 = crossing->section_1;
     		mbna_file_id_2 = crossing->file_id_2;
    		mbna_section_2 = crossing->section_2;
		if (crossing->num_ties > 0)
		    {
		    if (mbna_current_tie == -1)
		    	mbna_current_tie = 0;
		    tie = &crossing->ties[0];
		    mbna_snav_1 = tie->snav_1;
		    mbna_snav_1_time_d = tie->snav_1_time_d;
		    mbna_snav_2 = tie->snav_2;
		    mbna_snav_2_time_d = tie->snav_2_time_d;
		    mbna_offset_x = tie->offset_x;
		    mbna_offset_y = tie->offset_y;
		    mbna_offset_z = tie->offset_z_m;
/* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

		    /* reset survey file and section selections */
		    if (mbna_file_select == crossing->file_id_1)
			{
			mbna_section_select = crossing->section_1;
			}
		    else if (mbna_file_select == crossing->file_id_2)
			{
			mbna_section_select = crossing->section_2;
			}
		    else
			{
			mbna_file_select = crossing->file_id_1;
			mbna_survey_select = project.files[crossing->file_id_1].block;
			mbna_section_select = crossing->section_1;
			}
		    }
		else
		    {
		    mbna_current_tie = -1;
		    }
  		}

  	/* load the crossing */
  	if (mbna_current_crossing >= 0)
  		{
		/* put up message */
		sprintf(message,"Loading crossing %d...",mbna_current_crossing);
		do_message_on(message);

  		mbnavadjust_crossing_load();

		/* turn off message */
		do_message_off();
  		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}


/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_previous()
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_previous";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	crossing_ok;
	int	i, j, k;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* find previous current crossing */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
    		/* get next good crossing */
		j = -1;
		k = -1;
		for (i=0;i<project.num_crossings;i++)
			{
			crossing = &(project.crossings[i]);
			crossing_ok = MB_NO;
			if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_MEDIOCRECROSSINGS
				&& crossing->overlap >= MBNA_MEDIOCREOVERLAP_THRESHOLD)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS
				&& crossing->overlap >= MBNA_GOODOVERLAP_THRESHOLD)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS
				&& crossing->overlap >= MBNA_BETTEROVERLAP_THRESHOLD)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS
				&& crossing->truecrossing == MB_YES)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_TIES
				&& crossing->num_ties > 0)
				crossing_ok = MB_YES;
			if ((mbna_view_mode == MBNA_VIEW_MODE_SURVEY
					&& (mbna_survey_select != project.files[crossing->file_id_1].block
						|| mbna_survey_select != project.files[crossing->file_id_2].block))
				|| (mbna_view_mode == MBNA_VIEW_MODE_FILE
					&& (mbna_file_select != crossing->file_id_1
						|| mbna_file_select != crossing->file_id_2))
				|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY
					&& mbna_survey_select != project.files[crossing->file_id_1].block
					&& mbna_survey_select != project.files[crossing->file_id_2].block)
				|| (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE
					&& mbna_file_select != crossing->file_id_1
					&& mbna_file_select != crossing->file_id_2)
				|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION
					&& (mbna_file_select != crossing->file_id_1
						|| mbna_section_select != crossing->section_1)
					&& (mbna_file_select != crossing->file_id_2
						|| mbna_section_select != crossing->section_2)))
				crossing_ok = MB_NO;

			if (crossing_ok == MB_YES)
				{
				if (i < mbna_current_crossing)
					j = i;
				k = i;
				}

			}
		if (j >= 0)
			mbna_current_crossing = j;
		else if (k >= 0)
			mbna_current_crossing = k;
		else
			mbna_current_crossing = -1;
		mbna_current_tie = -1;
   		}


    	/* retrieve crossing parameters */
    	if (mbna_current_crossing >= 0)
    		{
    		crossing = &project.crossings[mbna_current_crossing];
    		mbna_file_id_1 = crossing->file_id_1;
    		mbna_section_1 = crossing->section_1;
     		mbna_file_id_2 = crossing->file_id_2;
    		mbna_section_2 = crossing->section_2;
		if (crossing->num_ties > 0)
		    {
		    if (mbna_current_tie == -1)
		    	mbna_current_tie = 0;
		    tie = &crossing->ties[0];
		    mbna_snav_1 = tie->snav_1;
		    mbna_snav_1_time_d = tie->snav_1_time_d;
		    mbna_snav_2 = tie->snav_2;
		    mbna_snav_2_time_d = tie->snav_2_time_d;
		    mbna_offset_x = tie->offset_x;
		    mbna_offset_y = tie->offset_y;
		    mbna_offset_z = tie->offset_z_m;
/* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

		    /* reset survey file and section selections */
		    if (mbna_file_select == crossing->file_id_1)
			{
			mbna_section_select = crossing->section_1;
			}
		    else if (mbna_file_select == crossing->file_id_2)
			{
			mbna_section_select = crossing->section_2;
			}
		    else
			{
			mbna_file_select = crossing->file_id_1;
			mbna_survey_select = project.files[crossing->file_id_1].block;
			mbna_section_select = crossing->section_1;
			}
		    }
		else
		    {
		    mbna_current_tie = -1;
		    }
  		}

  	/* load the crossing */
  	if (mbna_current_crossing >= 0)
  		{
		/* put up message */
		sprintf(message,"Loading crossing %d...",mbna_current_crossing);
		do_message_on(message);

  		mbnavadjust_crossing_load();

		/* turn off message */
		do_message_off();
  		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_nextunset()
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_nextunset";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	crossing_ok;
  	int	i, j, k;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* find next current unset crossing */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
    		/* get next good crossing */
		j = -1;
		k = -1;
		for (i=0;i<project.num_crossings;i++)
			{
			crossing = &(project.crossings[i]);
			crossing_ok = MB_NO;
			if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_MEDIOCRECROSSINGS
				&& crossing->overlap >= MBNA_MEDIOCREOVERLAP_THRESHOLD)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS
				&& crossing->overlap >= MBNA_GOODOVERLAP_THRESHOLD)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS
				&& crossing->overlap >= MBNA_BETTEROVERLAP_THRESHOLD)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS
				&& crossing->truecrossing == MB_YES)
				crossing_ok = MB_YES;
			else if (mbna_view_list == MBNA_VIEW_LIST_TIES
				&& crossing->num_ties > 0)
				crossing_ok = MB_YES;
			if ((mbna_view_mode == MBNA_VIEW_MODE_SURVEY
					&& (mbna_survey_select != project.files[crossing->file_id_1].block
						|| mbna_survey_select != project.files[crossing->file_id_2].block))
				|| (mbna_view_mode == MBNA_VIEW_MODE_FILE
					&& (mbna_file_select != crossing->file_id_1
						|| mbna_file_select != crossing->file_id_2))
				|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY
					&& mbna_survey_select != project.files[crossing->file_id_1].block
					&& mbna_survey_select != project.files[crossing->file_id_2].block)
				|| (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE
					&& mbna_file_select != crossing->file_id_1
					&& mbna_file_select != crossing->file_id_2)
				|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION
					&& (mbna_file_select != crossing->file_id_1
						|| mbna_section_select != crossing->section_1)
					&& (mbna_file_select != crossing->file_id_2
						|| mbna_section_select != crossing->section_2)))
				crossing_ok = MB_NO;
			if (crossing->status != MBNA_CROSSING_STATUS_NONE)
				crossing_ok = MB_NO;

			if (crossing_ok == MB_YES)
				{
				if (j < 0)
					j = i;
				if (k < 0 && i > mbna_current_crossing)
					k = i;
				}
			}
		if (k >= 0)
			mbna_current_crossing = k;
		else if (j >= 0)
			mbna_current_crossing = j;
		else
			mbna_current_crossing = -1;
		mbna_current_tie = -1;
   		}

    	/* retrieve crossing parameters */
    	if (mbna_current_crossing >= 0)
    		{
    		crossing = &project.crossings[mbna_current_crossing];
    		mbna_file_id_1 = crossing->file_id_1;
    		mbna_section_1 = crossing->section_1;
     		mbna_file_id_2 = crossing->file_id_2;
    		mbna_section_2 = crossing->section_2;
		if (crossing->num_ties > 0)
		    {
		    mbna_current_tie = 0;
		    tie = &crossing->ties[0];
		    mbna_snav_1 = tie->snav_1;
		    mbna_snav_1_time_d = tie->snav_1_time_d;
		    mbna_snav_2 = tie->snav_2;
		    mbna_snav_2_time_d = tie->snav_2_time_d;
		    mbna_offset_x = tie->offset_x;
		    mbna_offset_y = tie->offset_y;
		    mbna_offset_z = tie->offset_z_m;
/* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

		    /* reset survey file and section selections */
		    if (mbna_file_select == crossing->file_id_1)
			{
			mbna_section_select = crossing->section_1;
			}
		    else if (mbna_file_select == crossing->file_id_2)
			{
			mbna_section_select = crossing->section_2;
			}
		    else
			{
			mbna_file_select = crossing->file_id_1;
			mbna_survey_select = project.files[crossing->file_id_1].block;
			mbna_section_select = crossing->section_1;
			}
		    }
		else
		    {
		    mbna_current_tie = -1;
		    }
  		}

  	/* load the crossing */
  	if (mbna_current_crossing >= 0)
  		{
		/* put up message */
		sprintf(message,"Loading crossing %d...",mbna_current_crossing);
		do_message_on(message);

  		mbnavadjust_crossing_load();

		/* turn off message */
		do_message_off();
  		}

	/* else unload previously loaded crossing */
	else if (mbna_naverr_load == MB_YES)
		{
		status = mbnavadjust_crossing_unload();
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_selecttie()
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_selecttie";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	struct mbna_section *section1, *section2;
	struct mbna_file *file1, *file2;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* get current crossing */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
     		/* retrieve crossing parameters */
    		if (mbna_current_crossing >= 0
		    && project.crossings[mbna_current_crossing].num_ties > 0)
    			{
			/* select next tie */
			crossing = &project.crossings[mbna_current_crossing];
			mbna_current_tie++;
			if (mbna_current_tie > crossing->num_ties - 1)
			    mbna_current_tie = 0;
			tie = &crossing->ties[mbna_current_tie];
     			mbna_snav_1 = tie->snav_1;
     			mbna_snav_2 = tie->snav_2;
     			mbna_snav_1_time_d = tie->snav_1_time_d;
     			mbna_snav_2_time_d = tie->snav_2_time_d;
    			mbna_offset_x = tie->offset_x;
    			mbna_offset_y = tie->offset_y;
			mbna_offset_z = tie->offset_z_m;
    			tie->offset_x_m = mbna_offset_x / mbna_mtodeglon;
    			tie->offset_y_m = mbna_offset_y / mbna_mtodeglat;
 			file1 = (struct mbna_file *) &project.files[mbna_file_id_1];
			file2 = (struct mbna_file *) &project.files[mbna_file_id_2];
			section1 = (struct mbna_section *) &file1->sections[mbna_section_1];
			section2 = (struct mbna_section *) &file2->sections[mbna_section_2];
			mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2]
						- section1->snav_lon_offset[mbna_snav_1];
			mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2]
						- section1->snav_lat_offset[mbna_snav_1];
			mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2]
						- section1->snav_z_offset[mbna_snav_1];
  			}
   		}

   	/* set mbna_crossing_select */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0)
		{
    		mbna_crossing_select = mbna_current_crossing;
    		if (mbna_current_tie >= 0)
		    mbna_tie_select = mbna_current_tie;
		else
    		    mbna_tie_select = MBNA_SELECT_NONE;
		}
    	else
		{
    		mbna_crossing_select = MBNA_SELECT_NONE;
    		mbna_tie_select = MBNA_SELECT_NONE;
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_addtie()
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_addtie";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	struct mbna_section *section1, *section2;
	struct mbna_file *file1, *file2;
	int	found;
	int	ix, iy;
   	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* get current crossing */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
     		/* retrieve crossing parameters */
    		if (mbna_current_crossing >= 0
		    && project.crossings[mbna_current_crossing].num_ties < MBNA_SNAV_NUM)
    			{
			/* add tie and set number */
    			crossing = &project.crossings[mbna_current_crossing];
			file1 = (struct mbna_file *) &project.files[mbna_file_id_1];
			file2 = (struct mbna_file *) &project.files[mbna_file_id_2];
			section1 = (struct mbna_section *) &file1->sections[mbna_section_1];
			section2 = (struct mbna_section *) &file2->sections[mbna_section_2];
			mbna_current_tie = crossing->num_ties;
			crossing->num_ties++;
			project.num_ties++;
    			tie = &crossing->ties[mbna_current_tie];

			if (crossing->status == MBNA_CROSSING_STATUS_NONE)
				{
				project.num_crossings_analyzed++;
				if (crossing->truecrossing == MB_YES)
 					project.num_truecrossings_analyzed++;
				}
    			crossing->status = MBNA_CROSSING_STATUS_SET;

			/* look for unused pair of nav points */
     			tie->snav_1 = -1;
			found = MB_NO;
			while (found == MB_NO)
			    {
			    found = MB_YES;
			    tie->snav_1++;
			    for (i=0;i<crossing->num_ties-1;i++)
				{
				if (crossing->ties[i].snav_1 == tie->snav_1)
				    found = MB_NO;
				}
			    }
     			tie->snav_2 = -1;
			found = MB_NO;
			while (found == MB_NO)
			    {
			    found = MB_YES;
			    tie->snav_2++;
			    for (i=0;i<crossing->num_ties-1;i++)
				{
				if (crossing->ties[i].snav_2 == tie->snav_2)
				    found = MB_NO;
				}
			    }

			/* get rest of tie parameters */
			tie->status = MBNA_TIE_XYZ;
			tie->snav_1_time_d = section1->snav_time_d[tie->snav_1];
			tie->snav_2_time_d = section2->snav_time_d[tie->snav_2];
     			mbna_snav_1 = tie->snav_1;
     			mbna_snav_2 = tie->snav_2;
     			mbna_snav_1_time_d = tie->snav_1_time_d;
     			mbna_snav_2_time_d = tie->snav_2_time_d;
    			tie->offset_x = mbna_offset_x;
    			tie->offset_y = mbna_offset_y;
    			tie->offset_x_m = mbna_offset_x / mbna_mtodeglon;
    			tie->offset_y_m = mbna_offset_y / mbna_mtodeglat;
    			tie->offset_z_m = mbna_offset_z;
			tie->sigmar1 = mbna_minmisfit_sr1;
			tie->sigmar2 = mbna_minmisfit_sr2;
			tie->sigmar3 = mbna_minmisfit_sr3;
			for (i=0;i<3;i++)
				{
				tie->sigmax1[i] = mbna_minmisfit_sx1[i];
				tie->sigmax2[i] = mbna_minmisfit_sx2[i];
				tie->sigmax3[i] = mbna_minmisfit_sx3[i];
				}
			if (tie->sigmar1 < MBNA_SMALL)
				{
				tie->sigmar1 = 100.0;
				tie->sigmax1[0] = 1.0;
				tie->sigmax1[1] = 0.0;
				tie->sigmax1[2] = 0.0;
				}
			if (tie->sigmar2 < MBNA_SMALL)
				{
				tie->sigmar2 = 100.0;
				tie->sigmax2[0] = 0.0;
				tie->sigmax2[1] = 1.0;
				tie->sigmax2[2] = 0.0;
				}
			if (tie->sigmar3 < MBNA_SMALL)
				{
				tie->sigmar3 = 100.0;
				tie->sigmax3[0] = 0.0;
				tie->sigmax3[1] = 0.0;
				tie->sigmax3[2] = 1.0;
				}

			file1 = (struct mbna_file *) &project.files[mbna_file_id_1];
			file2 = (struct mbna_file *) &project.files[mbna_file_id_2];
			section1 = (struct mbna_section *) &file1->sections[mbna_section_1];
			section2 = (struct mbna_section *) &file2->sections[mbna_section_2];
			mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2]
						- section1->snav_lon_offset[mbna_snav_1];
			mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2]
						- section1->snav_lat_offset[mbna_snav_1];
			mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2]
						- section1->snav_z_offset[mbna_snav_1];
			tie->inversion_status = MBNA_INVERSION_NONE;
    			tie->inversion_offset_x = mbna_invert_offset_x;
    			tie->inversion_offset_y = mbna_invert_offset_y;
    			tie->inversion_offset_x_m = mbna_invert_offset_x / mbna_mtodeglon;
    			tie->inversion_offset_y_m = mbna_invert_offset_y / mbna_mtodeglat;
    			tie->inversion_offset_z_m = mbna_invert_offset_z;
   			if (project.inversion == MBNA_INVERSION_CURRENT)
    				project.inversion = MBNA_INVERSION_OLD;

			/* now put tie in center of plot */
			ix = (int)(0.5 * (mbna_plot_lon_max - mbna_plot_lon_min)
					* mbna_plotx_scale);
			iy = (int)(cont_borders[3]
					- (0.5 * (mbna_plot_lat_max - mbna_plot_lat_min)
						* mbna_ploty_scale));
			mbnavadjust_naverr_snavpoints(ix, iy);
     			tie->snav_1 = mbna_snav_1;
     			tie->snav_2 = mbna_snav_2;
     			tie->snav_1_time_d = mbna_snav_1_time_d;
     			tie->snav_2_time_d = mbna_snav_2_time_d;

			/* reset tie counts for snavs */
			section1->snav_num_ties[tie->snav_1]++;
			section2->snav_num_ties[tie->snav_2]++;

			/* write updated project */
			mbnavadjust_write_project();

			/* add info text */
			sprintf(message,"Add Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f %f m\n",
				mbna_current_tie, mbna_current_crossing,
				crossing->file_id_1, crossing->section_1, tie->snav_1,
				crossing->file_id_2, crossing->section_2, tie->snav_2,
				tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
			if (mbna_verbose == 0)
				fprintf(stderr,"%s",message);
			do_info_add(message, MB_YES);

 			/* print output debug statements */
			if (mbna_verbose >= 2)
				{
				fprintf(stderr,"\ndbg2  snav point selected in MBnavadjust function <%s>\n",
					function_name);
				fprintf(stderr,"dbg2  snav values:\n");
				fprintf(stderr,"dbg2       mbna_snav_1:        %d\n",mbna_snav_1);
				fprintf(stderr,"dbg2       mbna_snav_1_time_d: %f\n",mbna_snav_1_time_d);
				fprintf(stderr,"dbg2       mbna_snav_1_lon:    %f\n",mbna_snav_1_lon);
				fprintf(stderr,"dbg2       mbna_snav_1_lat:    %f\n",mbna_snav_1_lat);
				fprintf(stderr,"dbg2       section1->num_snav:  %d\n",section1->num_snav);
				for (i=0;i<section1->num_snav;i++)
					{
					fprintf(stderr,"dbg2       section1->snav_time_d[%d]: %f\n",i,section1->snav_time_d[i]);
					fprintf(stderr,"dbg2       section1->snav_lon[%d]:    %.10f\n",i,section1->snav_lon[i]);
					fprintf(stderr,"dbg2       section1->snav_lat[%d]:    %.10f\n",i,section1->snav_lat[i]);
					}
				fprintf(stderr,"dbg2       mbna_snav_2:        %d\n",mbna_snav_2);
				fprintf(stderr,"dbg2       mbna_snav_2_time_d: %f\n",mbna_snav_2_time_d);
				fprintf(stderr,"dbg2       mbna_snav_2_lon:    %.10f\n",mbna_snav_2_lon);
				fprintf(stderr,"dbg2       mbna_snav_2_lat:    %.10f\n",mbna_snav_2_lat);
				fprintf(stderr,"dbg2       section2->num_snav:  %d\n",section2->num_snav);
				for (i=0;i<section2->num_snav;i++)
					{
					fprintf(stderr,"dbg2       section2->snav_time_d[%d]: %f\n",i,section2->snav_time_d[i]);
					fprintf(stderr,"dbg2       section2->snav_lon[%d]:    %.10f\n",i,section2->snav_lon[i]);
					fprintf(stderr,"dbg2       section2->snav_lat[%d]:    %.10f\n",i,section2->snav_lat[i]);
					}
				}
  			}
   		}

   	/* set mbna_crossing_select */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0)
		{
    		mbna_crossing_select = mbna_current_crossing;
    		if (mbna_current_tie >= 0)
		    mbna_tie_select = mbna_current_tie;
		else
    		    mbna_tie_select = MBNA_SELECT_NONE;
		}
    	else
		{
    		mbna_crossing_select = MBNA_SELECT_NONE;
    		mbna_tie_select = MBNA_SELECT_NONE;
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_deletetie()
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_deletetie";
	int	status = MB_SUCCESS;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* get current crossing */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
     		/* retrieve crossing parameters */
    		if (mbna_current_crossing >= 0
		    && mbna_current_tie >= 0)
    			{
			/* delete the tie */
			mbnavadjust_deletetie(mbna_current_crossing, mbna_current_tie, MBNA_CROSSING_STATUS_SKIP);

			/* write updated project */
			mbnavadjust_write_project();
  			}
   		}

   	/* set mbna_crossing_select */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0)
		{
    		mbna_crossing_select = mbna_current_crossing;
    		if (mbna_current_tie >= 0)
		    mbna_tie_select = mbna_current_tie;
		else
    		    mbna_tie_select = MBNA_SELECT_NONE;
		}
    	else
		{
    		mbna_crossing_select = MBNA_SELECT_NONE;
    		mbna_tie_select = MBNA_SELECT_NONE;
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_deletetie(int icrossing, int jtie, int delete_status)
{
	/* local variables */
	char	*function_name = "mbnavadjust_deletetie";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	struct mbna_section *section1, *section2;
	struct mbna_file *file1, *file2;
   	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       icrossing:     %d\n",icrossing);
		fprintf(stderr,"dbg2       jtie:          %d\n",jtie);
		fprintf(stderr,"dbg2       delete_status: %d\n",delete_status);
		}

     	/* get current crossing */
    	if (project.open == MB_YES
		&& icrossing >= 0
		&& jtie >= 0)
    		{
     		/* retrieve crossing parameters */
    		if (project.num_crossings > icrossing
			&& project.crossings[icrossing].num_ties > jtie)
    			{
			/* add info text */
    			crossing = &project.crossings[icrossing];
    			tie = &crossing->ties[jtie];
			if (delete_status == MBNA_CROSSING_STATUS_SKIP)
			    sprintf(message,"Delete Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f %f m\n",
				jtie, icrossing,
				crossing->file_id_1, crossing->section_1, tie->snav_1,
				crossing->file_id_2, crossing->section_2, tie->snav_2,
				tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
			else
			    sprintf(message,"Clear Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f %f m\n",
				jtie, icrossing,
				crossing->file_id_1, crossing->section_1, tie->snav_1,
				crossing->file_id_2, crossing->section_2, tie->snav_2,
				tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
			if (mbna_verbose == 0)
				fprintf(stderr,"%s",message);
			do_info_add(message, MB_YES);

			/* reset tie counts for snavs */
			file1 = &project.files[crossing->file_id_1];
			section1 = &file1->sections[crossing->section_1];
			section1->snav_num_ties[tie->snav_1]--;
			file2 = &project.files[crossing->file_id_2];
			section2 = &file2->sections[crossing->section_2];
			section2->snav_num_ties[tie->snav_2]--;

			/* delete tie and set number */
			for (i=mbna_current_tie;i<crossing->num_ties-1;i++)
			    {
			    crossing->ties[i].snav_1 = crossing->ties[i+1].snav_1;
			    crossing->ties[i].snav_1_time_d = crossing->ties[i+1].snav_1_time_d;
			    crossing->ties[i].snav_2 = crossing->ties[i+1].snav_2;
			    crossing->ties[i].snav_2_time_d = crossing->ties[i+1].snav_2_time_d;
			    crossing->ties[i].offset_x = crossing->ties[i+1].offset_x;
			    crossing->ties[i].offset_y = crossing->ties[i+1].offset_y;
			    crossing->ties[i].offset_x_m = crossing->ties[i+1].offset_x_m;
			    crossing->ties[i].offset_y_m = crossing->ties[i+1].offset_y_m;
			    crossing->ties[i].offset_z_m = crossing->ties[i+1].offset_z_m;
			    }
			crossing->num_ties--;
			project.num_ties--;
			if (mbna_current_tie > crossing->num_ties -1)
			     mbna_current_tie--;

			/* set tie parameters */
			if (crossing->num_ties <= 0)
			    {
			    crossing->num_ties = 0;
    			    crossing->status = delete_status;
			    }
			else if (mbna_current_tie >= 0)
			    {
    			    tie = &crossing->ties[mbna_current_tie];
     			    mbna_snav_1 = tie->snav_1;
     			    mbna_snav_1_time_d = tie->snav_1_time_d;
			    mbna_snav_2 = tie->snav_2;
     			    mbna_snav_2_time_d = tie->snav_2_time_d;
			    mbna_offset_x = tie->offset_x;
			    mbna_offset_y = tie->offset_y;
			    mbna_offset_z = tie->offset_z_m;
/* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */
			    file1 = (struct mbna_file *) &project.files[mbna_file_id_1];
			    file2 = (struct mbna_file *) &project.files[mbna_file_id_2];
			    section1 = (struct mbna_section *) &file1->sections[mbna_section_1];
			    section2 = (struct mbna_section *) &file2->sections[mbna_section_2];
			    mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2]
						    - section1->snav_lon_offset[mbna_snav_1];
			    mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2]
						    - section1->snav_lat_offset[mbna_snav_1];
			    mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2]
						    - section1->snav_z_offset[mbna_snav_1];
			    }
   			if (project.inversion == MBNA_INVERSION_CURRENT)
    				project.inversion = MBNA_INVERSION_OLD;
  			}
   		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_resettie()
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_resettie";
	int	status = MB_SUCCESS;
	struct mbna_file *file1, *file2;
	struct mbna_section *section1, *section2;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* get current crossing */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
     		/* retrieve crossing parameters */
    		if (mbna_current_crossing >= 0
		    && mbna_current_tie >= 0)
    			{
			/* reset tie */
			file1 = (struct mbna_file *) &project.files[mbna_file_id_1];
			file2 = (struct mbna_file *) &project.files[mbna_file_id_2];
			section1 = (struct mbna_section *) &file1->sections[mbna_section_1];
			section2 = (struct mbna_section *) &file2->sections[mbna_section_2];
    			crossing = &project.crossings[mbna_current_crossing];
    			tie = &crossing->ties[mbna_current_tie];
			mbna_snav_1 = tie->snav_1;
			mbna_snav_1_time_d = tie->snav_1_time_d;
			mbna_snav_1_lon = section1->snav_lon[mbna_snav_1];
			mbna_snav_1_lat = section1->snav_lat[mbna_snav_1];
			mbna_snav_2 = tie->snav_2;
			mbna_snav_2_time_d = tie->snav_2_time_d;
			mbna_snav_2_lon = section2->snav_lon[mbna_snav_2];
			mbna_snav_2_lat = section2->snav_lat[mbna_snav_2];
			mbna_offset_x = tie->offset_x;
			mbna_offset_y = tie->offset_y;
			mbna_offset_z = tie->offset_z_m;
/* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */
			mbna_minmisfit_sr1 = tie->sigmar1;
			mbna_minmisfit_sr2 = tie->sigmar2;
			mbna_minmisfit_sr3 = tie->sigmar3;
			for (i=0;i<3;i++)
				{
				mbna_minmisfit_sx1[i] = tie->sigmax1[i];
				mbna_minmisfit_sx2[i] = tie->sigmax2[i];
				mbna_minmisfit_sx3[i] = tie->sigmax3[i];
				}
  			}
   		}

   	/* set mbna_crossing_select */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0)
		{
    		mbna_crossing_select = mbna_current_crossing;
    		if (mbna_current_tie >= 0)
		    mbna_tie_select = mbna_current_tie;
		else
    		    mbna_tie_select = MBNA_SELECT_NONE;
		}
    	else
		{
    		mbna_crossing_select = MBNA_SELECT_NONE;
    		mbna_tie_select = MBNA_SELECT_NONE;
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_checkoksettie()
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_checkoksettie";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* check for changed offsets */
    	mbna_allow_set_tie = MB_NO;
    	if (mbna_current_crossing >= 0
		&& mbna_current_tie >= 0)
	    {
    	    crossing = &project.crossings[mbna_current_crossing];
	    tie = &crossing->ties[mbna_current_tie];
	    if (tie->snav_1 != mbna_snav_1
		|| tie->snav_2 != mbna_snav_2
		|| tie->offset_x != mbna_offset_x
		|| tie->offset_y != mbna_offset_y
		|| tie->offset_z_m != mbna_offset_z)
		{
		mbna_allow_set_tie = MB_YES;
		}

	    /* also check for unset sigma values */
	    if (tie->sigmar1 == 100.0
	    	&& tie->sigmar2 == 100.0
	    	&& tie->sigmar3 == 100.0)
		{
		mbna_allow_set_tie = MB_YES;
		}
	    }

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_skip()
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_skip";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* get current crossing */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
     		/* retrieve crossing parameters */
    		if (mbna_current_crossing >= 0)
    			{
    			crossing = &project.crossings[mbna_current_crossing];
			project.num_ties -= crossing->num_ties;
			crossing->num_ties = 0;
			if (crossing->status == MBNA_CROSSING_STATUS_NONE)
				{
				project.num_crossings_analyzed++;
				if (crossing->truecrossing == MB_YES)
 					project.num_truecrossings_analyzed++;
				}
    			crossing->status = MBNA_CROSSING_STATUS_SKIP;
   			if (project.inversion == MBNA_INVERSION_CURRENT)
    				project.inversion = MBNA_INVERSION_OLD;

			/* write updated project */
			mbnavadjust_write_project();

			/* add info text */
			sprintf(message,"Set crossing %d to be ignored\n",
				mbna_current_crossing);
			if (mbna_verbose == 0)
				fprintf(stderr,"%s",message);
			do_info_add(message, MB_YES);
  			}
   		}

   	/* set mbna_crossing_select */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0)
		{
    		mbna_crossing_select = mbna_current_crossing;
    		if (mbna_current_tie >= 0)
		    mbna_tie_select = mbna_current_tie;
		else
    		    mbna_tie_select = MBNA_SELECT_NONE;
		}
    	else
		{
    		mbna_crossing_select = MBNA_SELECT_NONE;
    		mbna_tie_select = MBNA_SELECT_NONE;
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_unset()
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_skip";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

     	/* get current crossing */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
     		/* retrieve crossing parameters */
    		if (mbna_current_crossing >= 0)
    			{
    			crossing = &project.crossings[mbna_current_crossing];
			project.num_ties -= crossing->num_ties;
			crossing->num_ties = 0;
			if (crossing->status != MBNA_CROSSING_STATUS_NONE)
				{
				project.num_crossings_analyzed--;
				if (crossing->truecrossing == MB_YES)
 					project.num_truecrossings_analyzed--;
				}
    			crossing->status = MBNA_CROSSING_STATUS_NONE;
   			if (project.inversion == MBNA_INVERSION_CURRENT)
    				project.inversion = MBNA_INVERSION_OLD;

			/* write updated project */
			mbnavadjust_write_project();

			/* add info text */
			sprintf(message,"Unset crossing %d\n",
				mbna_current_crossing);
			if (mbna_verbose == 0)
				fprintf(stderr,"%s",message);
			do_info_add(message, MB_YES);
  			}
   		}

   	/* set mbna_crossing_select */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0)
		{
    		mbna_crossing_select = mbna_current_crossing;
    		if (mbna_current_tie >= 0)
		    mbna_tie_select = mbna_current_tie;
		else
    		    mbna_tie_select = MBNA_SELECT_NONE;
		}
    	else
		{
    		mbna_crossing_select = MBNA_SELECT_NONE;
    		mbna_tie_select = MBNA_SELECT_NONE;
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_load()
{
	/* local variables */
	char	*function_name = "mbnavadjust_crossing_load";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	struct mbna_file *file1, *file2;
	struct mbna_section *section1, *section2;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* unload loaded crossing */
	if (mbna_naverr_load == MB_YES)
		{
		status = mbnavadjust_crossing_unload();
		}

     	/* load current crossing */
    	if ((mbna_status == MBNA_STATUS_NAVERR || mbna_status == MBNA_STATUS_AUTOPICK)
		&& project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0)
    		{
		/* put up message */
		sprintf(message,"Loading crossing %d...",mbna_current_crossing);
		do_message_update(message);

    		/* retrieve crossing parameters */
		crossing = &project.crossings[mbna_current_crossing];
		mbna_file_id_1 = crossing->file_id_1;
		mbna_section_1 = crossing->section_1;
		mbna_file_id_2 = crossing->file_id_2;
		mbna_section_2 = crossing->section_2;
		file1 = (struct mbna_file *) &project.files[mbna_file_id_1];
		file2 = (struct mbna_file *) &project.files[mbna_file_id_2];
		section1 = (struct mbna_section *) &file1->sections[mbna_section_1];
		section2 = (struct mbna_section *) &file2->sections[mbna_section_2];
		if (crossing->num_ties > 0 && mbna_current_tie >= 0)
		    {
		    /* get basic crossing parameters */
		    tie = &crossing->ties[mbna_current_tie];
		    mbna_snav_1 = tie->snav_1;
		    mbna_snav_1_time_d = tie->snav_1_time_d;
		    mbna_snav_1_lon = section1->snav_lon[mbna_snav_1];
		    mbna_snav_1_lat = section1->snav_lat[mbna_snav_1];
		    mbna_snav_2 = tie->snav_2;
		    mbna_snav_2_time_d = tie->snav_2_time_d;
		    mbna_snav_2_lon = section2->snav_lon[mbna_snav_2];
		    mbna_snav_2_lat = section2->snav_lat[mbna_snav_2];
		    mbna_offset_x = tie->offset_x;
		    mbna_offset_y = tie->offset_y;
		    mbna_offset_z = tie->offset_z_m;
/* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */
		    file1 = (struct mbna_file *) &project.files[mbna_file_id_1];
		    file2 = (struct mbna_file *) &project.files[mbna_file_id_2];
		    section1 = (struct mbna_section *) &file1->sections[mbna_section_1];
		    section2 = (struct mbna_section *) &file2->sections[mbna_section_2];
		    mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2]
					    - section1->snav_lon_offset[mbna_snav_1];
		    mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2]
					    - section1->snav_lat_offset[mbna_snav_1];
		    mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2]
					    - section1->snav_z_offset[mbna_snav_1];
		    }
		else if (project.inversion != MBNA_INVERSION_NONE)
	    	    {
		    mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2]
					    - section1->snav_lon_offset[mbna_snav_1];
		    mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2]
					    - section1->snav_lat_offset[mbna_snav_1];
		    mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2]
					    - section1->snav_z_offset[mbna_snav_1];
		    mbna_offset_x = mbna_invert_offset_x;
		    mbna_offset_y = mbna_invert_offset_y;
		    mbna_offset_z = mbna_invert_offset_z;
/* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */
		    }
		else
		    {
		    mbna_offset_x = 0.0;
		    mbna_offset_y = 0.0;
		    mbna_offset_z = 0.0;
/* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */
		    }
		mbna_lon_min = MIN(section1->lonmin,section2->lonmin + mbna_offset_x);
		mbna_lon_max = MAX(section1->lonmax,section2->lonmax + mbna_offset_x);
		mbna_lat_min = MIN(section1->latmin,section2->latmin + mbna_offset_y);
		mbna_lat_max = MAX(section1->latmax,section2->latmax + mbna_offset_y);
		mbna_plot_lon_min = mbna_lon_min;
		mbna_plot_lon_max = mbna_lon_max;
		mbna_plot_lat_min = mbna_lat_min;
		mbna_plot_lat_max = mbna_lat_max;
		mb_coor_scale(mbna_verbose,0.5 * (mbna_lat_min + mbna_lat_max),
				&mbna_mtodeglon,&mbna_mtodeglat);

		/* load sections */
		sprintf(message,"Loading section 1 of crossing %d...",mbna_current_crossing);
		do_message_update(message);
		status = mbnavadjust_section_load(mbna_file_id_1, mbna_section_1, (void **) &swathraw1, (void **) &swath1, section1->num_pings);
		sprintf(message,"Loading section 2 of crossing %d...",mbna_current_crossing);
		do_message_update(message);
		status = mbnavadjust_section_load(mbna_file_id_2, mbna_section_2, (void **) &swathraw2, (void **) &swath2, section2->num_pings);

		/* get lon lat positions for soundings */
		sprintf(message,"Transforming section 1 of crossing %d...",mbna_current_crossing);
		do_message_update(message);
		status = mbnavadjust_section_translate(mbna_file_id_1, swathraw1, swath1, 0.0);
		sprintf(message,"Transforming section 2 of crossing %d...",mbna_current_crossing);
		do_message_update(message);
		status = mbnavadjust_section_translate(mbna_file_id_2, swathraw2, swath2, mbna_offset_z);

		/* generate contour data */
		if (mbna_status != MBNA_STATUS_AUTOPICK)
			{
			sprintf(message,"Contouring section 1 of crossing %d...",mbna_current_crossing);
			do_message_update(message);
			status = mbnavadjust_section_contour(mbna_file_id_1,mbna_section_1,swath1,&mbna_contour1);
			sprintf(message,"Contouring section 2 of crossing %d...",mbna_current_crossing);
			do_message_update(message);
			status = mbnavadjust_section_contour(mbna_file_id_2,mbna_section_2,swath2,&mbna_contour2);
			}

		/* set loaded flag */
		mbna_naverr_load = MB_YES;

		/* generate misfit grids */
		sprintf(message,"Getting misfit for crossing %d...",mbna_current_crossing);
		do_message_update(message);
		status = mbnavadjust_get_misfit();

		/* get overlap region */
		mbnavadjust_crossing_overlap(mbna_current_crossing);
  		}

   	/* set mbna_crossing_select */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0)
		{
    		mbna_crossing_select = mbna_current_crossing;
    		if (mbna_current_tie >= 0)
		    mbna_tie_select = mbna_current_tie;
		else
    		    mbna_tie_select = MBNA_SELECT_NONE;
		}
    	else
		{
    		mbna_crossing_select = MBNA_SELECT_NONE;
    		mbna_tie_select = MBNA_SELECT_NONE;
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_unload()
{
	/* local variables */
	char	*function_name = "mbnavadjust_crossing_unload";
	int	status = MB_SUCCESS;
	struct pingraw *pingraw;
   	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* unload loaded crossing */
	if (mbna_naverr_load == MB_YES)
		{
		/* free raw swath data */
		if (swathraw1 != NULL && swathraw1->pingraws != NULL)
		    {
		    for (i=0;i<swathraw1->npings_max;i++)
			{
			pingraw = &swathraw1->pingraws[i];
			status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&pingraw->beamflag, &error);
			status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&pingraw->bath, &error);
			status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&pingraw->bathacrosstrack, &error);
			status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&pingraw->bathalongtrack, &error);
			}
		    status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&swathraw1->pingraws, &error);
		    }
		if (swathraw2 != NULL && swathraw2->pingraws != NULL)
		    {
		    for (i=0;i<swathraw2->npings_max;i++)
			{
			pingraw = &swathraw2->pingraws[i];
			status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&pingraw->beamflag, &error);
			status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&pingraw->bath, &error);
			status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&pingraw->bathacrosstrack, &error);
			status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&pingraw->bathalongtrack, &error);
			}
		    status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&swathraw2->pingraws, &error);
		    }
		if (swathraw1 != NULL)
		    status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&swathraw1, &error);
		if (swathraw2 != NULL)
		    status = mb_freed(mbna_verbose,__FILE__,__LINE__,(void **)&swathraw2, &error);

		/* free contours */
		status = mb_contour_deall(mbna_verbose,swath1,&error);
		status = mb_contour_deall(mbna_verbose,swath2,&error);
		if (mbna_contour1.vector != NULL
			&& mbna_contour1.nvector_alloc > 0)
		    {
		    free(mbna_contour1.vector);
		    }
		if (mbna_contour2.vector != NULL
			&& mbna_contour2.nvector_alloc > 0)
		    {
		    free(mbna_contour2.vector);
		    }
		mbna_contour1.vector = NULL;
		mbna_contour1.nvector = 0;
		mbna_contour1.nvector_alloc = 0;
		mbna_contour2.vector = NULL;
		mbna_contour2.nvector = 0;
		mbna_contour2.nvector_alloc = 0;
		mbna_naverr_load = MB_NO;
		grid_nx = 0;
		grid_ny = 0;
		grid_nxy = 0;
		grid_nxyzeq = 0;
		gridm_nx = 0;
		gridm_ny = 0;
		gridm_nxyz = 0;
		if (grid1 != NULL)
		    {
		    free(grid1);
		    }
		if (grid2 != NULL)
		    {
		    free(grid2);
		    }
		if (gridm != NULL)
		    {
		    free(gridm);
		    }
		if (gridmeq != NULL)
		    {
		    free(gridmeq);
		    }
		if (gridn1 != NULL)
		    {
		    free(gridn1);
		    }
		if (gridn2 != NULL)
		    {
		    free(gridn2);
		    }
		if (gridnm != NULL)
		    {
		    free(gridnm);
		    }
		grid1 = NULL;
		grid2 = NULL;
		gridm = NULL;
		gridmeq = NULL;
		gridn1 = NULL;
		gridn2 = NULL;
		gridnm = NULL;
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_replot()
{
	/* local variables */
	char	*function_name = "mbnavadjust_crossing_replot";
	int	status = MB_SUCCESS;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* replot loaded crossing */
	if ((mbna_status == MBNA_STATUS_NAVERR  || mbna_status == MBNA_STATUS_AUTOPICK)
		&& mbna_naverr_load == MB_YES)
		{
		/* get lon lat positions for soundings */
		status = mbnavadjust_section_translate(mbna_file_id_1, swathraw1, swath1, 0.0);
		status = mbnavadjust_section_translate(mbna_file_id_2, swathraw2, swath2, mbna_offset_z);

		/* generate contour data */
		status = mbnavadjust_section_contour(mbna_file_id_1,mbna_section_1,swath1,&mbna_contour1);
		status = mbnavadjust_section_contour(mbna_file_id_2,mbna_section_2,swath2,&mbna_contour2);
   		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_section_load(int file_id, int section_id, void **swathraw_ptr, void **swath_ptr, int num_pings)
{
	/* local variables */
	char	*function_name = "mbnavadjust_section_load";
	int	status = MB_SUCCESS;
	struct mb_io_struct *imb_io_ptr;
	struct swathraw *swathraw;
	struct pingraw *pingraw;
	struct swath *swath;
	struct mbna_file *file;
	struct mbna_section *section;

	/* mbio read and write values */
	void	*imbio_ptr = NULL;
	void	*istore_ptr = NULL;
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
	double	roll;
	double	pitch;
	double	heave;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[MB_COMMENT_MAXLINE];

	char	path[STRING_MAX];
	int	iformat;
	double	tick_len_map, label_hgt_map;
	int	done;
   	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       file_id:      %d\n",file_id);
		fprintf(stderr,"dbg2       section_id:   %d\n",section_id);
		fprintf(stderr,"dbg2       swath_ptr:    %p  %p\n",swath_ptr, *swath_ptr);
		fprintf(stderr,"dbg2       num_pings:    %d\n",num_pings);
		}

     	/* load specified section */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
		/* set section format and path */
		sprintf(path,"%s/nvs_%4.4d_%4.4d.mb71",
			project.datadir,file_id,section_id);
		iformat = 71;
		file = &(project.files[file_id]);
		section = &(file->sections[section_id]);

		/* initialize section for reading */
		if ((status = mb_read_init(
			mbna_verbose,path,iformat,pings,lonflip,bounds,
			btime_i,etime_i,speedmin,timegap,
			&imbio_ptr,&btime_d,&etime_d,
			&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(mbna_verbose,error,&error_message);
			fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",error_message);
			fprintf(stderr,"\nSwath sonar File <%s> not initialized for reading\n",path);
			exit(0);
			}

		/* allocate memory for data arrays */
		if (status == MB_SUCCESS)
			{
			if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(char), (void **)&beamflag, &error);
			if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bath, &error);
			if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							    sizeof(double), (void **)&amp, &error);
			if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bathacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							    sizeof(double), (void **)&bathalongtrack, &error);
			if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&ss, &error);
			if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&ssacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR)
			    status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							    sizeof(double), (void **)&ssalongtrack, &error);

			/* if error initializing memory then don't read the file */
			if (error != MB_ERROR_NO_ERROR)
				{
				mb_error(mbna_verbose,error,&error_message);
				fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
					error_message);
				}
			}

		/* allocate memory for data arrays */
		if (status == MB_SUCCESS)
			{
			/* get mb_io_ptr */
			imb_io_ptr = (struct mb_io_struct *) imbio_ptr;

			/* initialize data storage */
			status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, sizeof(struct swathraw),
						(void **)swathraw_ptr, &error);
			swathraw = (struct swathraw *) *swathraw_ptr;
			swathraw->beams_bath = beams_bath;
			swathraw->npings_max = num_pings;
			swathraw->npings = 0;
			status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, num_pings * sizeof(struct pingraw),
						(void **)&swathraw->pingraws, &error);
			for (i=0;i<swathraw->npings_max;i++)
				{
				pingraw = &swathraw->pingraws[i];
				pingraw->beams_bath = 0;
				pingraw->beamflag = NULL;
				pingraw->bath = NULL;
				pingraw->bathacrosstrack = NULL;
				pingraw->bathalongtrack = NULL;
				}

			/* initialize contour controls */
			tick_len_map = MAX(section->lonmax - section->lonmin,
						section->latmax - section->latmin) / 500;
			label_hgt_map = MAX(section->lonmax - section->lonmin,
						section->latmax - section->latmin) / 100;
 			status = mb_contour_init(mbna_verbose, (struct swath **)swath_ptr,
					    num_pings,
					    beams_bath,
					    mbna_contour_algorithm,
					    MB_YES,MB_NO,MB_NO,MB_NO,MB_NO,
					    project.cont_int, project.col_int,
					    project.tick_int, project.label_int,
					    tick_len_map, label_hgt_map, 0.0,
					    mbna_ncolor, 0, NULL, NULL, NULL,
					    0.0, 0.0, 0.0, 0.0,
					    0, 0, 0.0, 0.0,
					    &mbnavadjust_plot,
					    &mbnavadjust_newpen,
					    &mbnavadjust_setline,
					    &mbnavadjust_justify_string,
					    &mbnavadjust_plot_string,					    
					    &error);
			swath = (struct swath *) *swath_ptr;
			swath->beams_bath = beams_bath;
			swath->npings = 0;

			/* if error initializing memory then quit */
			if (error != MB_ERROR_NO_ERROR)
				{
				mb_error(mbna_verbose,error,&error_message);
				fprintf(stderr,"\nMBIO Error allocating contour control structure:\n%s\n",error_message);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
			}

		/* now read the data */
		if (status == MB_SUCCESS)
			{
			done = MB_NO;
			while (done == MB_NO)
			    {
			    /* read the next ping */
			    status = mb_get_all(mbna_verbose,imbio_ptr,
				    &istore_ptr, &kind, time_i, &time_d,
				    &navlon, &navlat, &speed,
				    &heading, &distance, &altitude, &sonardepth,
				    &beams_bath, &beams_amp, &pixels_ss,
				    beamflag, bath, amp, bathacrosstrack, bathalongtrack,
				    ss, ssacrosstrack, ssalongtrack,
				    comment, &error);

			    /* handle successful read */
			    if (status == MB_SUCCESS
				&& kind == MB_DATA_DATA)
			    	{
				/* allocate memory for the raw arrays */
				pingraw = &swathraw->pingraws[swathraw->npings];
				status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, beams_bath * sizeof(char),
							(void **)&pingraw->beamflag, &error);
				status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
							(void **)&pingraw->bath, &error);
				status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
							(void **)&pingraw->bathacrosstrack, &error);
				status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
							(void **)&pingraw->bathalongtrack, &error);

				/* make sure enough memory is allocated for contouring arrays */
		        	ping = &swath->pings[swathraw->npings];
				if (ping->beams_bath_alloc < beams_bath)
					{
					status = mb_reallocd(mbna_verbose,__FILE__,__LINE__,beams_bath*sizeof(char),
							(void **)&(ping->beamflag),&error);
					status = mb_reallocd(mbna_verbose,__FILE__,__LINE__,beams_bath*sizeof(double),
							(void **)&(ping->bath),&error);
					status = mb_reallocd(mbna_verbose,__FILE__,__LINE__,beams_bath*sizeof(double),
							(void **)&(ping->bathlon),&error);
					status = mb_reallocd(mbna_verbose,__FILE__,__LINE__,beams_bath*sizeof(double),
							(void **)&(ping->bathlat),&error);
                                        if (mbna_contour_algorithm == MB_CONTOUR_OLD)
                                            {
                                            status = mb_reallocd(mbna_verbose,__FILE__,__LINE__,beams_bath*sizeof(int),
                                                            (void **)&(ping->bflag[0]),&error);
                                            status = mb_reallocd(mbna_verbose,__FILE__,__LINE__,beams_bath*sizeof(int),
                                                            (void **)&(ping->bflag[1]),&error);
                                            }
					ping->beams_bath_alloc = beams_bath;
					}

			    	/* copy arrays and update bookkeeping */
			    	if (error == MB_ERROR_NO_ERROR)
				    {
				    swathraw->npings++;
				    if (swathraw->npings >= swathraw->npings_max)
				    	done = MB_YES;

				    for (i=0;i<7;i++)
				    	pingraw->time_i[i] = time_i[i];
				    pingraw->time_d = time_d;
				    pingraw->navlon = navlon;
				    pingraw->navlat = navlat;
				    pingraw->heading = heading;
				    pingraw->draft = sonardepth;
				    pingraw->beams_bath = beams_bath;
/* fprintf(stderr,"\nPING %d : %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
swathraw->npings,time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6]); */
				    for (i=0;i<beams_bath;i++)
				    	{
					pingraw->beamflag[i] = beamflag[i];
					if (mb_beam_ok(beamflag[i]))
				    		{
						pingraw->beamflag[i] = beamflag[i];
						pingraw->bath[i] = bath[i];
				    		pingraw->bathacrosstrack[i] = bathacrosstrack[i];
				    		pingraw->bathalongtrack[i] = bathalongtrack[i];
						}
					else
				    		{
						pingraw->beamflag[i] = MB_FLAG_NULL;
						pingraw->bath[i] = 0.0;
				    		pingraw->bathacrosstrack[i] = 0.0;
				    		pingraw->bathalongtrack[i] = 0.0;
						}
/* fprintf(stderr,"BEAM: %d:%d  Flag:%d    %f %f %f\n",
swathraw->npings,i,pingraw->beamflag[i],pingraw->bath[i],pingraw->bathacrosstrack[i],pingraw->bathalongtrack[i]); */
					}
				    }

				/* extract all nav values */
				status = mb_extract_nav(mbna_verbose,imbio_ptr,
					istore_ptr,&kind,
					pingraw->time_i, &pingraw->time_d,
					&pingraw->navlon, &pingraw->navlat, &speed,
					&pingraw->heading, &pingraw->draft,
					&roll, &pitch, &heave,
					&error);

/*fprintf(stderr, "%d  %4d/%2d/%2d %2d:%2d:%2d.%6.6d  %15.10f %15.10f %d:%d\n",
status,
ping->time_i[0],ping->time_i[1],ping->time_i[2],
ping->time_i[3],ping->time_i[4],ping->time_i[5],ping->time_i[6],
ping->navlon, ping->navlat, beams_bath, swath->beams_bath);*/


			    	/* print debug statements */
			    	if (mbna_verbose >= 2)
				    {
				    fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
					    program_name);
				    fprintf(stderr,"dbg2       kind:           %d\n",
					    kind);
				    fprintf(stderr,"dbg2       npings:         %d\n",
					    swathraw->npings);
				    fprintf(stderr,"dbg2       time:           %4d %2d %2d %2d %2d %2d %6.6d\n",
					    pingraw->time_i[0],pingraw->time_i[1],pingraw->time_i[2],
					    pingraw->time_i[3],pingraw->time_i[4],pingraw->time_i[5],pingraw->time_i[6]);
				    fprintf(stderr,"dbg2       navigation:     %f  %f\n",
					    pingraw->navlon, pingraw->navlat);
				    fprintf(stderr,"dbg2       beams_bath:     %d\n",
					    beams_bath);
				    fprintf(stderr,"dbg2       beams_amp:      %d\n",
					    beams_amp);
				    fprintf(stderr,"dbg2       pixels_ss:      %d\n",
					    pixels_ss);
				    fprintf(stderr,"dbg2       done:           %d\n",
					    done);
				    fprintf(stderr,"dbg2       error:          %d\n",
					    error);
				    fprintf(stderr,"dbg2       status:         %d\n",
					    status);
				    }
				}
			    else if (error > MB_ERROR_NO_ERROR)
			    	{
			    	status = MB_SUCCESS;
			    	error = MB_ERROR_NO_ERROR;
			    	done = MB_YES;
			    	}
			    }

			/* close the input data file */
			status = mb_close(mbna_verbose,&imbio_ptr,&error);
			}
   		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_section_translate(int file_id, void *swathraw_ptr, void *swath_ptr, double zoffset)
{
	/* local variables */
	char	*function_name = "mbnavadjust_section_translate";
	int	status = MB_SUCCESS;
	struct swathraw *swathraw;
	struct pingraw *pingraw;
	struct swath *swath;
	double	mtodeglon, mtodeglat, headingx, headingy;
	double	depth, depthacrosstrack, depthalongtrack;
	double	alpha, beta, range;
   	int	i, iping;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       file_id:      %d\n",file_id);
		fprintf(stderr,"dbg2       swathraw_ptr: %p\n",swathraw_ptr);
		fprintf(stderr,"dbg2       swath_ptr:    %p\n",swath_ptr);
		fprintf(stderr,"dbg2       zoffset:      %f\n",zoffset);
		}

     	/* translate sounding positions for loaded section */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0)
    		{
		swathraw = (struct swathraw *) swathraw_ptr;
		swath = (struct swath *) swath_ptr;

		/* relocate soundings based on heading bias */
		swath->npings = 0;
		for (iping=0;iping<swathraw->npings;iping++)
		    {
		    swath->npings++;
		    pingraw = &swathraw->pingraws[iping];
		    ping = &swath->pings[swath->npings - 1];
		    for (i=0;i<7;i++)
			ping->time_i[i] = pingraw->time_i[i];
		    ping->time_d = pingraw->time_d;
		    ping->navlon = pingraw->navlon;
		    ping->navlat = pingraw->navlat;
		    ping->heading = pingraw->heading
				    + project.files[file_id].heading_bias;
		    mb_coor_scale(mbna_verbose, pingraw->navlat,
				    &mtodeglon, &mtodeglat);
		    headingx = sin(ping->heading * DTR);
		    headingy = cos(ping->heading * DTR);
		    ping->beams_bath = pingraw->beams_bath;
		    for (i=0;i<ping->beams_bath;i++)
			{
			ping->beamflag[i] = pingraw->beamflag[i];
			if (mb_beam_ok(pingraw->beamflag[i]))
			    {
			    /* strip off transducer depth */
			    depth = pingraw->bath[i] - pingraw->draft;

			    /* get range and angles in
				roll-pitch frame */
			    range = sqrt(depth * depth
					+ pingraw->bathacrosstrack[i]
					    * pingraw->bathacrosstrack[i]
					+ pingraw->bathalongtrack[i]
					    * pingraw->bathalongtrack[i]);
			    alpha = asin(pingraw->bathalongtrack[i]
				    / range);
			    beta = acos(pingraw->bathacrosstrack[i]
				    / range / cos(alpha));

			    /* apply roll correction */
			    beta +=  DTR * project.files[file_id].roll_bias;

			    /* recalculate bathymetry */
			    depth = range * cos(alpha) * sin(beta);
			    depthalongtrack = range * sin(alpha);
			    depthacrosstrack = range * cos(alpha) * cos(beta);

			    /* add heave and draft back in */
			    depth += pingraw->draft;

			    /* add zoffset */
			    depth += zoffset;

			    /* get bathymetry in lon lat */
			    ping->beamflag[i] = pingraw->beamflag[i];
			    ping->bath[i] = depth;
			    ping->bathlon[i] = pingraw->navlon
					    + headingy*mtodeglon
						*depthacrosstrack
					    + headingx*mtodeglon
						*depthalongtrack;
			    ping->bathlat[i] = pingraw->navlat
					    - headingx*mtodeglat
						*depthacrosstrack
					    + headingy*mtodeglat
						*depthalongtrack;
			    }
			}
		    }
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_section_contour(int fileid, int sectionid,
				struct swath *swath,
				struct mbna_contour_vector *contour)
{
	/* local variables */
	char	*function_name = "mbnavadjust_section_contour";
	int	status = MB_SUCCESS;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       fileid:       %d\n",fileid);
		fprintf(stderr,"dbg2       sectionid:    %d\n",sectionid);
		fprintf(stderr,"dbg2       swath:        %p\n",swath);
		fprintf(stderr,"dbg2       contour:      %p\n",contour);
		fprintf(stderr,"dbg2       nvector:      %d\n",contour->nvector);
		fprintf(stderr,"dbg2       nvector_alloc:%d\n",contour->nvector_alloc);
		}

	if (swath != NULL)
		{
		/* set vectors */
		mbna_contour = contour;
		contour->nvector = 0;

    		/* reset contouring parameters */
		swath->contour_int = project.cont_int;
		swath->color_int = project.col_int;
		swath->tick_int = project.tick_int;

		/* generate contours */
		status = mb_contour(mbna_verbose,swath,&error);

		/* set contours up to date flag */
		project.files[fileid].sections[sectionid].contoursuptodate = MB_YES;
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_snavpoints(int ix, int iy)
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_snavpoints";
	int	status = MB_SUCCESS;
	double	x, y, dx, dy, d;
	struct mbna_crossing *crossing;
	struct mbna_section *section;
	double	distance;
	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       ix:           %d\n",ix);
		fprintf(stderr,"dbg2       iy:           %d\n",iy);
		}

     	if (mbna_naverr_load == MB_YES)
    		{
    		/* get position in lon and lat */
	    	x = ix / mbna_plotx_scale +  mbna_plot_lon_min;
	    	y = (cont_borders[3] - iy) / mbna_ploty_scale +  mbna_plot_lat_min;
		crossing = &project.crossings[mbna_current_crossing];

	    	/* get closest snav point in swath 1 */
		section = &project.files[crossing->file_id_1].sections[crossing->section_1];
	    	distance = 999999.999;
		for (i=0;i<section->num_snav;i++)
			{
	    		dx = (section->snav_lon[i] - x) / mbna_mtodeglon;
	    		dy = (section->snav_lat[i] - y) / mbna_mtodeglat;
	    		d = sqrt(dx * dx + dy * dy);
	    		if (d < distance)
	    			{
	    			distance = d;
 			    	mbna_snav_1 = i;
			    	mbna_snav_1_time_d = section->snav_time_d[i];
			    	mbna_snav_1_lon = section->snav_lon[i];
			    	mbna_snav_1_lat = section->snav_lat[i];
	    			}
			}

	    	/* get closest snav point in swath 2 */
		section = &project.files[crossing->file_id_2].sections[crossing->section_2];
	    	distance = 999999.999;
		for (i=0;i<section->num_snav;i++)
			{
	    		dx = (section->snav_lon[i] + mbna_offset_x - x) / mbna_mtodeglon;
	    		dy = (section->snav_lat[i] + mbna_offset_y - y) / mbna_mtodeglat;
	    		d = sqrt(dx * dx + dy * dy);
	    		if (d < distance)
	    			{
	    			distance = d;
 			    	mbna_snav_2 = i;
			    	mbna_snav_2_time_d = section->snav_time_d[i];
			    	mbna_snav_2_lon = section->snav_lon[i];
			    	mbna_snav_2_lat = section->snav_lat[i];
	    			}
			}

		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  snav point selected in MBnavadjust function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  snav values:\n");
		section = &project.files[crossing->file_id_1].sections[crossing->section_1];
		fprintf(stderr,"dbg2       mbna_snav_1:        %d\n",mbna_snav_1);
		fprintf(stderr,"dbg2       mbna_snav_1_time_d: %f\n",mbna_snav_1_time_d);
		fprintf(stderr,"dbg2       mbna_snav_1_lon:    %.10f\n",mbna_snav_1_lon);
		fprintf(stderr,"dbg2       mbna_snav_1_lat:    %.10f\n",mbna_snav_1_lat);
		fprintf(stderr,"dbg2       section->num_snav:  %d\n",section->num_snav);
		for (i=0;i<section->num_snav;i++)
			{
			fprintf(stderr,"dbg2       section1->snav_time_d[%d]: %f\n",i,section->snav_time_d[i]);
			fprintf(stderr,"dbg2       section1->snav_lon[%d]:    %.10f\n",i,section->snav_lon[i]);
			fprintf(stderr,"dbg2       section1->snav_lat[%d]:    %.10f\n",i,section->snav_lat[i]);
			}
		section = &project.files[crossing->file_id_2].sections[crossing->section_2];
		fprintf(stderr,"dbg2       mbna_snav_2:        %d\n",mbna_snav_2);
		fprintf(stderr,"dbg2       mbna_snav_2_time_d: %f\n",mbna_snav_2_time_d);
		fprintf(stderr,"dbg2       mbna_snav_2_lon:    %.10f\n",mbna_snav_2_lon);
		fprintf(stderr,"dbg2       mbna_snav_2_lat:    %.10f\n",mbna_snav_2_lat);
		fprintf(stderr,"dbg2       section->num_snav:  %d\n",section->num_snav);
		for (i=0;i<section->num_snav;i++)
			{
			fprintf(stderr,"dbg2       section2->snav_time_d[%d]: %f\n",i,section->snav_time_d[i]);
			fprintf(stderr,"dbg2       section2->snav_lon[%d]:    %.10f\n",i,section->snav_lon[i]);
			fprintf(stderr,"dbg2       section2->snav_lat[%d]:    %.10f\n",i,section->snav_lat[i]);
			}
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_sections_intersect(int crossing_id)
{
	/* local variables */
	char	*function_name = "mbnavadjust_sections_intersect";
	struct mbna_file *file;
	struct mbna_crossing *crossing;
	struct mbna_section *section;
	int	answer = MB_NO;
	double	xa1, ya1, xa2, ya2;
	double	xb1, yb1, xb2, yb2;
	double	dxa, dya, dxb, dyb;
	double	s, t;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       crossing_id:  %d\n",crossing_id);
		}

	/* get crossing */
	crossing = (struct mbna_crossing *) &project.crossings[crossing_id];

	/* get section endpoints */
	file = &project.files[crossing->file_id_1];
	section = &file->sections[crossing->section_1];
	xa1 = section->snav_lon[0] + section->snav_lon_offset[0];
	ya1 = section->snav_lat[0] + section->snav_lat_offset[0];
	xa2 = section->snav_lon[section->num_snav - 1] + section->snav_lon_offset[section->num_snav - 1];
	ya2 = section->snav_lat[section->num_snav - 1] + section->snav_lat_offset[section->num_snav - 1];
	file = &project.files[crossing->file_id_2];
	section = &file->sections[crossing->section_2];
	xb1 = section->snav_lon[0] + section->snav_lon_offset[0];
	yb1 = section->snav_lat[0] + section->snav_lat_offset[0];
	xb2 = section->snav_lon[section->num_snav - 1] + section->snav_lon_offset[section->num_snav - 1];
	yb2 = section->snav_lat[section->num_snav - 1] + section->snav_lat_offset[section->num_snav - 1];

	/* check for parallel sections */
	dxa = xa2 - xa1;
	dya = ya2 - ya1;
	dxb = xb2 - xb1;
	dyb = yb2 - yb1;
	if ((dxb * dya - dyb * dxa) == 0.0)
		{
		answer = MB_NO;
		}
	else
		{
		/* check for crossing sections */
		s = (dxa * (yb1 - ya1) + dya * (xa1 - xb1)) / (dxb * dya - dyb * dxa);
		t = (dxb * (ya1 - yb1) + dyb * (xb1 - xa1)) / (dyb * dxa - dxb * dya);
		if (s >= 0.0 && s <= 1.0 && t >= 0.0 && t <= 1.0)
			answer = MB_YES;
		else
			answer = MB_NO;
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       answer:      %d\n",answer);
		}

	return(answer);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_overlap(int crossing_id)
{
	/* local variables */
	char	*function_name = "mbnavadjust_crossing_overlap";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_crossing *crossing;
	struct mbna_section *section1;
	struct mbna_section *section2;
	int	overlap1[MBNA_MASK_DIM * MBNA_MASK_DIM];
	int	overlap2[MBNA_MASK_DIM * MBNA_MASK_DIM];
	double	lonoffset, latoffset;
	double	lon1min, lon1max;
	double	lat1min, lat1max;
	double	lon2min, lon2max;
	double	lat2min, lat2max;
	double	dx1, dy1, dx2, dy2;
	double	overlapfraction;
	int	ncoverage1, ncoverage2;
	int	noverlap1, noverlap2;
	int	first;
	int	i, ii1, jj1, kk1, ii2, jj2, kk2;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       crossing_id:          %d\n",crossing_id);
		}

	/* get crossing */
	crossing = (struct mbna_crossing *) &project.crossings[crossing_id];

	/* get section endpoints */
	file = &project.files[crossing->file_id_1];
	section1 = &file->sections[crossing->section_1];
	file = &project.files[crossing->file_id_2];
	section2 = &file->sections[crossing->section_2];
	lonoffset = section2->snav_lon_offset[section2->num_snav/2] - section1->snav_lon_offset[section1->num_snav/2];
	latoffset = section2->snav_lat_offset[section2->num_snav/2] - section1->snav_lat_offset[section1->num_snav/2];

	/* initialize overlap arrays */
	for (i=0;i<MBNA_MASK_DIM*MBNA_MASK_DIM;i++)
		{
		overlap1[i] = 0;
		overlap2[i] = 0;
		}

	/* check coverage masks for overlap */
	first = MB_YES;
	dx1 = (section1->lonmax - section1->lonmin) / MBNA_MASK_DIM;
	dy1 = (section1->latmax - section1->latmin) / MBNA_MASK_DIM;
	dx2 = (section2->lonmax - section2->lonmin) / MBNA_MASK_DIM;
	dy2 = (section2->latmax - section2->latmin) / MBNA_MASK_DIM;
	for (ii1=0;ii1<MBNA_MASK_DIM;ii1++)
	    for (jj1=0;jj1<MBNA_MASK_DIM;jj1++)
		{
		kk1 = ii1 + jj1 * MBNA_MASK_DIM;
		if (section1->coverage[kk1] == 1)
		    {
		    lon1min = section1->lonmin + dx1 * ii1;
		    lon1max = section1->lonmin + dx1 * (ii1 + 1);
		    lat1min = section1->latmin + dy1 * jj1;
		    lat1max = section1->latmin + dy1 * (jj1 + 1);
		    for (ii2=0;ii2<MBNA_MASK_DIM;ii2++)
			for (jj2=0;jj2<MBNA_MASK_DIM;jj2++)
			    {
			    kk2 = ii2 + jj2 * MBNA_MASK_DIM;
			    if (section2->coverage[kk2] == 1)
				{
				lon2min = section2->lonmin + dx2 * ii2 + lonoffset;
				lon2max = section2->lonmin + dx2 * (ii2 + 1) + lonoffset;
				lat2min = section2->latmin + dy2 * jj2 + latoffset;
				lat2max = section2->latmin + dy2 * (jj2 + 1) + latoffset;
				if ((lon1min < lon2max)
				    && (lon1max > lon2min)
				    && (lat1min < lat2max)
				    && (lat1max > lat2min))
				    {
				    overlap1[kk1] = 1;
				    overlap2[kk2] = 1;
				    }
				}
			    }
		    }
		}

	/* count fractions covered */
	ncoverage1 = 0;
	ncoverage2 = 0;
	noverlap1 = 0;
	noverlap2 = 0;
	for (i=0;i<MBNA_MASK_DIM*MBNA_MASK_DIM;i++)
		{
		if (section1->coverage[i] == 1)
			ncoverage1++;
		if (section2->coverage[i] == 1)
			ncoverage2++;
		if (overlap1[i] == 1)
			noverlap1++;
		if (overlap2[i] == 1)
			noverlap2++;
		}
	overlapfraction = (dx1 * dy1) / (dx1 * dy1 + dx2 * dy2)
				* ((double)noverlap1) / ((double)ncoverage1)
			+ (dx2 * dy2) / (dx1 * dy1 + dx2 * dy2)
				* ((double)noverlap2) / ((double)ncoverage2);
	crossing->overlap = (int) (100.0 * overlapfraction);
	if (crossing->overlap < 1)
		crossing->overlap = 1;

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       crossing->overlap: %d\n",crossing->overlap);
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_overlapbounds(int crossing_id,
				double offset_x, double offset_y,
				double *lonmin, double *lonmax,
				double *latmin, double *latmax)
{
	/* local variables */
	char	*function_name = "mbnavadjust_crossing_overlapbounds";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_crossing *crossing;
	struct mbna_section *section1;
	struct mbna_section *section2;
	int	overlap1[MBNA_MASK_DIM * MBNA_MASK_DIM];
	int	overlap2[MBNA_MASK_DIM * MBNA_MASK_DIM];
	double	lon1min, lon1max;
	double	lat1min, lat1max;
	double	lon2min, lon2max;
	double	lat2min, lat2max;
	double	dx1, dy1, dx2, dy2;
	int	first;
	int	i, ii1, jj1, kk1, ii2, jj2, kk2;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       crossing_id:          %d\n",crossing_id);
		fprintf(stderr,"dbg2       offset_x:             %f\n",offset_x);
		fprintf(stderr,"dbg2       offset_y:             %f\n",offset_y);
		}

	/* get crossing */
	crossing = (struct mbna_crossing *) &project.crossings[crossing_id];

	/* get section endpoints */
	file = &project.files[crossing->file_id_1];
	section1 = &file->sections[crossing->section_1];
	file = &project.files[crossing->file_id_2];
	section2 = &file->sections[crossing->section_2];

	/* initialize overlap arrays */
	for (i=0;i<MBNA_MASK_DIM*MBNA_MASK_DIM;i++)
		{
		overlap1[i] = 0;
		overlap2[i] = 0;
		}

	/* check coverage masks for overlap */
	first = MB_YES;
	*lonmin = 0.0;
	*lonmax = 0.0;
	*latmin = 0.0;
	*latmax = 0.0;
	dx1 = (section1->lonmax - section1->lonmin) / MBNA_MASK_DIM;
	dy1 = (section1->latmax - section1->latmin) / MBNA_MASK_DIM;
	dx2 = (section2->lonmax - section2->lonmin) / MBNA_MASK_DIM;
	dy2 = (section2->latmax - section2->latmin) / MBNA_MASK_DIM;
	for (ii1=0;ii1<MBNA_MASK_DIM;ii1++)
	    for (jj1=0;jj1<MBNA_MASK_DIM;jj1++)
		{
		kk1 = ii1 + jj1 * MBNA_MASK_DIM;
		if (section1->coverage[kk1] == 1)
		    {
		    lon1min = section1->lonmin + dx1 * ii1;
		    lon1max = section1->lonmin + dx1 * (ii1 + 1);
		    lat1min = section1->latmin + dy1 * jj1;
		    lat1max = section1->latmin + dy1 * (jj1 + 1);
		    for (ii2=0;ii2<MBNA_MASK_DIM;ii2++)
			for (jj2=0;jj2<MBNA_MASK_DIM;jj2++)
			    {
			    kk2 = ii2 + jj2 * MBNA_MASK_DIM;
			    if (section2->coverage[kk2] == 1)
				{
				lon2min = section2->lonmin + dx2 * ii2 + offset_x;
				lon2max = section2->lonmin + dx2 * (ii2 + 1) + offset_x;
				lat2min = section2->latmin + dy2 * jj2 + offset_y;
				lat2max = section2->latmin + dy2 * (jj2 + 1) + offset_y;
				if ((lon1min < lon2max)
				    && (lon1max > lon2min)
				    && (lat1min < lat2max)
				    && (lat1max > lat2min))
				    {
				    overlap1[kk1] = 1;
				    overlap2[kk2] = 1;
				    if (first == MB_NO)
					{
					*lonmin = MIN(*lonmin, MAX(lon1min, lon2min));
					*lonmax = MAX(*lonmax, MIN(lon1max, lon2max));
					*latmin = MIN(*latmin, MAX(lat1min, lat2min));
					*latmax = MAX(*latmax, MIN(lat1max, lat2max));
					}
				    else
					{
					first = MB_NO;
					*lonmin = MAX(lon1min, lon2min);
					*lonmax = MIN(lon1max, lon2max);
					*latmin = MAX(lat1min, lat2min);
					*latmax = MIN(lat1max, lat2max);
					}
				    }
				}
			    }
		    }
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       lonmin:      %.10f\n",*lonmin);
		fprintf(stderr,"dbg2       lonmax:      %.10f\n",*lonmax);
		fprintf(stderr,"dbg2       latmin:      %.10f\n",*latmin);
		fprintf(stderr,"dbg2       latmax:      %.10f\n",*latmax);
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_get_misfit()
{
	/* local variables */
	char	*function_name = "mbnavadjust_get_misfit";
	int	status = MB_SUCCESS;
	double	dinterval;
	double	zoff;
	double	minmisfitthreshold, dotproduct;
	double	x, y, z, r;
	double	dotproductsave2;
	double	rsave2;
	double	dotproductsave3;
	double	rsave3;
	int	found;
	int	igx, igy;
	int	ic, jc, kc, lc;
	int	ioff, joff, istart, iend, jstart, jend;
	int	i1, i2, j1, j2, k1, k2;
	int	imin, jmin, kmin;
	int	i, j, k, l, ll;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

    	if (project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0
		&& mbna_naverr_load == MB_YES)
    		{
/* fprintf(stderr,"\nDEBUG %s %d: mbnavadjust_get_misfit: mbna_plot minmax: %f %f %f %f\n",
__FILE__,__LINE__,
mbna_plot_lon_min,mbna_plot_lon_max,mbna_plot_lat_min,mbna_plot_lat_max); */

    		/* set message on */
    		if (mbna_verbose > 1)
			fprintf(stderr,"Making misfit grid for crossing %d\n",mbna_current_crossing);
		sprintf(message,"Making misfit grid for crossing %d\n",mbna_current_crossing);
		do_message_update(message);

		/* reset sounding density threshold for misfit calculation
			- will be tuned down if necessary */
		mbna_minmisfit_nthreshold = MBNA_MISFIT_NTHRESHOLD;

		/* figure out lateral extent of grids */
		grid_nx = MBNA_MISFIT_DIMXY;
		grid_ny = MBNA_MISFIT_DIMXY;
		if ((mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon
		    > (mbna_plot_lat_max - mbna_plot_lat_min) / mbna_mtodeglat)
		    {
		    grid_dx = (mbna_plot_lon_max - mbna_plot_lon_min) / (grid_nx - 1);
		    grid_dy = grid_dx * mbna_mtodeglat / mbna_mtodeglon;
/* fprintf(stderr,"DEBUG %s %d: grid scale: grid_dx:%f grid_dy:%f\n",
__FILE__,__LINE__,
grid_dx,grid_dy); */
		    }
		else
		    {
		    grid_dy = (mbna_plot_lat_max - mbna_plot_lat_min) / (grid_ny - 1);
		    grid_dx = grid_dy * mbna_mtodeglon / mbna_mtodeglat;
/* fprintf(stderr,"DEBUG %s %d: grid scale: grid_dx:%f grid_dy:%f\n",
__FILE__,__LINE__,
grid_dx,grid_dy); */
		    }
		grid_nxy = grid_nx * grid_ny;
		grid_olon = 0.5 * (mbna_plot_lon_min + mbna_plot_lon_max)
				    - (grid_nx / 2 + 0.5) * grid_dx;
		grid_olat = 0.5 * (mbna_plot_lat_min + mbna_plot_lat_max)
				    - (grid_ny / 2 + 0.5) * grid_dy;
/* fprintf(stderr,"DEBUG %s %d: grid_olon:%.10f grid_olat:%.10f\n",
__FILE__,__LINE__,
grid_olon,grid_olat); */

		/* get 3d misfit grid */
		nzmisfitcalc = MBNA_MISFIT_DIMZ;
		gridm_nx = grid_nx / 2 + 1;
		gridm_ny = gridm_nx;
		gridm_nxyz = gridm_nx * gridm_ny * nzmisfitcalc;
		if (mbna_misfit_center == MBNA_MISFIT_ZEROCENTER)
		    {
		    mbna_misfit_offset_x = 0.0;
		    mbna_misfit_offset_y = 0.0;
		    mbna_misfit_offset_z = 0.0;
		    }
		else
		    {
		    mbna_misfit_offset_x = mbna_offset_x;
		    mbna_misfit_offset_y = mbna_offset_y;
		    mbna_misfit_offset_z = mbna_offset_z;
		    }
/* fprintf(stderr,"DEBUG %s %d: GRID parameters: dx:%.10f dy:%.10f nx:%d ny:%d  bounds:  grid: %.10f %.10f %.10f %.10f  plot: %.10f %.10f %.10f %.10f\n",
__FILE__,__LINE__,
grid_dx, grid_dy, grid_nx, grid_ny,
grid_olon, grid_olon + grid_nx * grid_dx,
grid_olat, grid_olat + grid_ny * grid_dy,
mbna_lon_min, mbna_lon_max, mbna_lat_min, mbna_lat_max); */

		/* figure out range of z offsets */
		zmin = mbna_misfit_offset_z - 0.5 * project.zoffsetwidth;
		zmax = mbna_misfit_offset_z + 0.5 * project.zoffsetwidth;
		zoff_dz = project.zoffsetwidth / (nzmisfitcalc - 1);
/* fprintf(stderr,"DEBUG %s %d: mbna_misfit_offset_z:%f project.zoffsetwidth:%f nzmisfitcalc:%d zmin:%f zmax:%f zoff_dz:%f\n",
__FILE__,__LINE__,
mbna_misfit_offset_z,project.zoffsetwidth,nzmisfitcalc,zmin,zmax,zoff_dz); */

		/* allocate and initialize grids and arrays */
		grid1 = (double *) realloc(grid1, sizeof(double) * (grid_nxy));
		grid2 = (double *) realloc(grid2, sizeof(double) * (grid_nxy));
		gridm = (double *) realloc(gridm, sizeof(double) * (gridm_nxyz));
		gridmeq = (double *) realloc(gridmeq, sizeof(double) * (gridm_nxyz));
		gridn1 = (int *) realloc(gridn1, sizeof(int) * (grid_nxy));
		gridn2 = (int *) realloc(gridn2, sizeof(int) * (grid_nxy));
		gridnm = (int *) realloc(gridnm, sizeof(int) * (gridm_nxyz));
		memset(grid1, 0, sizeof(double) * (grid_nxy));
		memset(grid2, 0, sizeof(double) * (grid_nxy));
		memset(gridm, 0, sizeof(double) * (gridm_nxyz));
		memset(gridmeq, 0, sizeof(double) * (gridm_nxyz));
		memset(gridn1, 0, sizeof(int) * (grid_nxy));
		memset(gridn2, 0, sizeof(int) * (grid_nxy));
		memset(gridnm, 0, sizeof(int) * (gridm_nxyz));

	    	/* loop over all beams */
	    	for (i=0;i<swath1->npings;i++)
	    		{
	    		for (j=0;j<swath1->pings[i].beams_bath;j++)
	    			{
	    			if (mb_beam_ok(swath1->pings[i].beamflag[j]))
	    				{
	    				x = (swath1->pings[i].bathlon[j] - grid_olon);
	    				y = (swath1->pings[i].bathlat[j] - grid_olat);
					igx = (int) (x / grid_dx);
					igy = (int) (y / grid_dy);
					k = igx + igy * grid_nx;
					if (igx >= 0 && igx < grid_nx
					    && igy >= 0 && igy < grid_ny)
					    {
					    grid1[k] += swath1->pings[i].bath[j];
					    gridn1[k] ++;
					    }
/* else
fprintf(stderr,"DEBUG %s %d: BAD swath1: %d %d  %.10f %.10f  %f %f  %d %d\n",
__FILE__,__LINE__,
i, j, swath1->pings[i].bathlon[j], swath1->pings[i].bathlat[j], x, y, igx, igy); */
	    				}
	    			}
	    		}

	    	/* loop over all beams */
	    	for (i=0;i<swath2->npings;i++)
	    		{
	    		for (j=0;j<swath2->pings[i].beams_bath;j++)
	    			{
	    			if (mb_beam_ok(swath2->pings[i].beamflag[j]))
	    				{
	    				x = (swath2->pings[i].bathlon[j] + mbna_misfit_offset_x - grid_olon);
	    				y = (swath2->pings[i].bathlat[j] + mbna_misfit_offset_y - grid_olat);
					igx = (int) (x / grid_dx);
					igy = (int) (y / grid_dy);
					k = igx + igy * grid_nx;
					if (igx >= 0 && igx < grid_nx
					    && igy >= 0 && igy < grid_ny)
					    {
					    grid2[k] += swath2->pings[i].bath[j];
					    gridn2[k] ++;
					    }
/* else
fprintf(stderr,"DEBUG %s %d: BAD swath2: %d %d  %.10f %.10f  %f %f  %d %d\n",
__FILE__,__LINE__,
i, j, swath2->pings[i].bathlon[j], swath2->pings[i].bathlat[j], x, y, igx, igy); */
	    				}
	    			}
	    		}

		/* calculate gridded bath */
		for (k=0;k<grid_nxy;k++)
		    {
		    if (gridn1[k] > 0)
			{
			grid1[k] = (grid1[k] / gridn1[k]);
			}
		    if (gridn2[k] > 0)
			{
			grid2[k] = (grid2[k] / gridn2[k]);
			}
/* fprintf(stderr,"GRIDDED BATH: k:%d 1:%d %f   2:%d %f\n",
k,gridn1[k],grid1[k],gridn2[k],grid2[k]); */
		    }

		/* calculate gridded misfit over lateral and z offsets */
		for (ic=0;ic<gridm_nx;ic++)
		    for (jc=0;jc<gridm_ny;jc++)
			for (kc=0;kc<nzmisfitcalc;kc++)
			    {
			    lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
			    gridm[lc] = 0.0;
			    gridnm[lc] = 0;

			    ioff = (gridm_nx / 2) - ic;
			    joff = (gridm_ny / 2) - jc;
			    zoff = zmin + zoff_dz * kc;

			    istart = MAX(-ioff, 0);
			    iend = grid_nx - MAX(0, ioff);
			    jstart = MAX(-joff, 0);
			    jend = grid_ny - MAX(0, joff);
			    for (i1=istart;i1<iend;i1++)
				for (j1=jstart;j1<jend;j1++)
				    {
				    i2 = i1 + ioff;
				    j2 = j1 + joff;
				    k1 = i1 + j1 * grid_nx;
				    k2 = i2 + j2 * grid_nx;
				    if (gridn1[k1] > 0 && gridn2[k2] > 0)
					{
					gridm[lc] += (grid2[k2] - grid1[k1] + zoff - mbna_offset_z)
							* (grid2[k2] - grid1[k1] + zoff - mbna_offset_z);
					gridnm[lc]++;
					}
				    }
			    }
		misfit_min = 0.0;
		misfit_max = 0.0;
		mbna_minmisfit = 0.0;
		mbna_minmisfit_n = 0;
		mbna_minmisfit_x = 0.0;
		mbna_minmisfit_y = 0.0;
		mbna_minmisfit_z = 0.0;
		found = MB_NO;
		for (ic=0;ic<gridm_nx;ic++)
		    for (jc=0;jc<gridm_ny;jc++)
			for (kc=0;kc<nzmisfitcalc;kc++)
			    {
			    lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
			    if (gridnm[lc] > 0)
				{
 				gridm[lc] = sqrt(gridm[lc]) / gridnm[lc];
				if (misfit_max == 0.0)
			    	    {
				    misfit_min = gridm[lc];
				    }
				misfit_min = MIN(misfit_min, gridm[lc]);
				misfit_max = MAX(misfit_max, gridm[lc]);
				if (gridnm[lc] > mbna_minmisfit_nthreshold
				    && (mbna_minmisfit_n == 0 || gridm[lc] < mbna_minmisfit))
				    {
				    mbna_minmisfit = gridm[lc];
				    mbna_minmisfit_n = gridnm[lc];
				    mbna_minmisfit_x = (ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x;
				    mbna_minmisfit_y = (jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y;
				    mbna_minmisfit_z = zmin + zoff_dz * kc;
				    imin = ic;
				    jmin = jc,
				    kmin = kc;
				    found = MB_YES;
/* zoff = zmin + zoff_dz * kc;
fprintf(stderr,"DEBUG %s %d: ic:%d jc:%d kc:%d misfit:%f %f %d  pos:%f %f %f zoff:%f mbna_ofset_z:%f\n",
__FILE__,__LINE__,
ic,jc,kc,misfit_min,mbna_minmisfit,mbna_minmisfit_n,mbna_minmisfit_x,mbna_minmisfit_y,mbna_minmisfit_z,
zoff,mbna_offset_z); */
 			    	    }
				}
/* if (ic == jc && kc == 0)
fprintf(stderr,"DEBUG %s %d: ic:%d jc:%d misfit:%d %f\n",
__FILE__,__LINE__,
ic,jc,gridnm[lc],gridm[lc]);*/
			    }
		if (found == MB_NO)
		    {
		    mbna_minmisfit_nthreshold /= 10.0;
		    for (ic=0;ic<gridm_nx;ic++)
			for (jc=0;jc<gridm_ny;jc++)
			    for (kc=0;kc<nzmisfitcalc;kc++)
				{
				lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
				if (gridnm[lc] > mbna_minmisfit_nthreshold/10 && (mbna_minmisfit_n == 0 || gridm[lc] < mbna_minmisfit))
				    {
				    mbna_minmisfit = gridm[lc];
				    mbna_minmisfit_n = gridnm[lc];
				    mbna_minmisfit_x = (ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x;
				    mbna_minmisfit_y = (jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y;
				    mbna_minmisfit_z = zmin + zoff_dz * kc;
				    imin = ic;
				    jmin = jc,
				    kmin = kc;
				    found = MB_YES;
				    }
/* fprintf(stderr,"DEBUG %s %d: ijk:%d %d %d gridm:%d %f  misfit:%f %f %d  pos:%f %f %f\n",
__FILE__,__LINE__,
ic,jc,kc,gridnm[lc],gridm[lc],misfit_min,mbna_minmisfit,mbna_minmisfit_n,mbna_minmisfit_x,mbna_minmisfit_y,mbna_minmisfit_z); */
				}
		    }
		misfit_min = 0.99 * misfit_min;
		misfit_max = 1.01 * misfit_max;
/* if (found == MB_YES)
{
lc = kmin + nzmisfitcalc * (imin + jmin * gridm_nx);
fprintf(stderr,"DEBUG %s %d: min misfit: i:%d j:%d k:%d    n:%d m:%f   offsets: %f %f %f\n",
__FILE__,__LINE__,
imin, jmin, kmin, gridnm[lc], gridm[lc],
mbna_minmisfit_x / mbna_mtodeglon,
mbna_minmisfit_y / mbna_mtodeglat,
mbna_minmisfit_z);
} */

/* fprintf(stderr,"DEBUG %s %d: Misfit bounds: nmin:%d best:%f min:%f max:%f min loc: %f %f %f\n",
__FILE__,__LINE__,
mbna_minmisfit_n,mbna_minmisfit,misfit_min,misfit_max,
mbna_minmisfit_x/mbna_mtodeglon,mbna_minmisfit_y/mbna_mtodeglat,mbna_minmisfit_z); */

    		/* set message on */
    		if (mbna_verbose > 1)
			fprintf(stderr,"Histogram equalizing misfit grid for crossing %d\n",mbna_current_crossing);
		sprintf(message,"Histogram equalizing misfit grid for crossing %d\n",mbna_current_crossing);
		do_message_update(message);

		/* sort the misfit to get histogram equalization */
		grid_nxyzeq = 0;
		for (l=0;l<gridm_nxyz;l++)
		    {
		    if (gridm[l] > 0.0)
			{
			gridmeq[grid_nxyzeq] = gridm[l];
			grid_nxyzeq++;
			}
		    }

		if (grid_nxyzeq > 0)
		    {
			qsort((char *)gridmeq,grid_nxyzeq,sizeof(double),mb_double_compare);
			dinterval = ((double) grid_nxyzeq) / ((double)(nmisfit_intervals-1));
			if (dinterval < 1.0)
			    {
			    for (l=0;l<grid_nxyzeq;l++)
				    misfit_intervals[l] = gridmeq[l];
			    for (l=grid_nxyzeq;l<nmisfit_intervals;l++)
				    misfit_intervals[l] = gridmeq[grid_nxyzeq-1];
			    }
			else
			    {
			    misfit_intervals[0] = misfit_min;
			    misfit_intervals[nmisfit_intervals-1] = misfit_max;
			    for (l=1;l<nmisfit_intervals-1;l++)
				    {
				    ll = (int)(l * dinterval);
				    misfit_intervals[l] = gridmeq[ll];
				    }
		    }

		    /* get minimum misfit in 2D plane at current z offset */
		    mbnavadjust_get_misfitxy();

   		    /* set message on */
    		    if (mbna_verbose > 1)
			fprintf(stderr,"Estimating 3D uncertainty for crossing %d\n",mbna_current_crossing);
		    sprintf(message,"Estimating 3D uncertainty for crossing %d\n",mbna_current_crossing);
		    do_message_update(message);

		    /* estimating 3 component uncertainty vector at minimum misfit point */
		    /* first get the longest vector to a misfit value <= 2 times minimum misfit */
		    minmisfitthreshold = mbna_minmisfit * 3.0;
		    mbna_minmisfit_sr1 = 0.0;
		    for (ic=0;ic<gridm_nx;ic++)
			for (jc=0;jc<gridm_ny;jc++)
			    for (kc=0;kc<nzmisfitcalc;kc++)
				{
				lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
				if (gridnm[lc] > mbna_minmisfit_nthreshold && gridm[lc] <= minmisfitthreshold)
				    {
 				    x = ((ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x - mbna_minmisfit_x) / mbna_mtodeglon;
				    y = ((jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y - mbna_minmisfit_y) / mbna_mtodeglat;
				    z = zmin + zoff_dz * kc - mbna_minmisfit_z;
				    r = sqrt(x * x + y * y + z * z);
/* fprintf(stderr,"DEBUG %s %d: %d %d %d gridm[%d]:%f minmisfitthreshold:%f x: %f %f %f  r:%f\n",
__FILE__,__LINE__,
ic,jc,kc,lc,gridm[lc],minmisfitthreshold,x,y,z,r); */
				    if (r > mbna_minmisfit_sr1)
					{
					mbna_minmisfit_sx1[0] = x;
					mbna_minmisfit_sx1[1] = y;
					mbna_minmisfit_sx1[2] = z;
					mbna_minmisfit_sr1 = r;
					}
				    }
				}
		    mbna_minmisfit_sx1[0] /= mbna_minmisfit_sr1;
		    mbna_minmisfit_sx1[1] /= mbna_minmisfit_sr1;
		    mbna_minmisfit_sx1[2] /= mbna_minmisfit_sr1;
/* fprintf(stderr,"DEBUG %s %d: Longest vector in misfit space. %f %f %f  r:%f\n",
__FILE__,__LINE__,
mbna_minmisfit_sx1[0],mbna_minmisfit_sx1[1],mbna_minmisfit_sx1[2],mbna_minmisfit_sr1); */

		    /* now get a horizontal unit vector perpendicular to the the longest vector
			    and then find the largest r associated with that vector */
		    mbna_minmisfit_sr2 = sqrt(mbna_minmisfit_sx1[0] * mbna_minmisfit_sx1[0] + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx1[1]);
		    if (mbna_minmisfit_sr2 < MBNA_SMALL)
			    {
			    mbna_minmisfit_sx2[0] = 0.0;
			    mbna_minmisfit_sx2[1] = 1.0;
			    mbna_minmisfit_sx2[2] = 0.0;
			    mbna_minmisfit_sr2 = 1.0;
			    }
		    else
			    {
			    mbna_minmisfit_sx2[0] = mbna_minmisfit_sx1[1] / mbna_minmisfit_sr2;
			    mbna_minmisfit_sx2[1] = -mbna_minmisfit_sx1[0] / mbna_minmisfit_sr2;
			    mbna_minmisfit_sx2[2] = 0.0;
			    mbna_minmisfit_sr2 = sqrt(mbna_minmisfit_sx2[0] * mbna_minmisfit_sx2[0] + mbna_minmisfit_sx2[1] * mbna_minmisfit_sx2[1] + mbna_minmisfit_sx2[2] * mbna_minmisfit_sx2[2]);
			    }
/* dotproduct = (mbna_minmisfit_sx1[0] * mbna_minmisfit_sx2[0] + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx2[1] + mbna_minmisfit_sx1[2] * mbna_minmisfit_sx2[2]);
fprintf(stderr,"DEBUG %s %d: Horizontal perpendicular vector in misfit space. %f %f %f  r:%f dotproduct:%f\n",
__FILE__,__LINE__,
mbna_minmisfit_sx2[0],mbna_minmisfit_sx2[1],mbna_minmisfit_sx2[2],mbna_minmisfit_sr2,dotproduct); */

		    /* now get a near-vertical unit vector perpendicular to the the longest vector
			    and then find the largest r associated with that vector */
		    mbna_minmisfit_sr3 = sqrt(mbna_minmisfit_sx1[0] * mbna_minmisfit_sx1[0] + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx1[1]);
		    if (mbna_minmisfit_sr3 < MBNA_SMALL)
			{
			mbna_minmisfit_sx3[0] = 0.0;
			mbna_minmisfit_sx3[1] = 0.0;
			mbna_minmisfit_sx3[2] = 1.0;
			mbna_minmisfit_sr3 = 1.0;
			}
		    else
			{
			if (mbna_minmisfit_sx1[2] >= 0.0)
			    {
			    mbna_minmisfit_sx3[0] = -mbna_minmisfit_sx1[0] * sqrt(1.0 - mbna_minmisfit_sr3 * mbna_minmisfit_sr3) / mbna_minmisfit_sr3;
			    mbna_minmisfit_sx3[1] = -mbna_minmisfit_sx1[1] * sqrt(1.0 - mbna_minmisfit_sr3 * mbna_minmisfit_sr3) / mbna_minmisfit_sr3;
			    }
			else
			    {
			    mbna_minmisfit_sx3[0] = mbna_minmisfit_sx1[0] * sqrt(1.0 - mbna_minmisfit_sr3 * mbna_minmisfit_sr3) / mbna_minmisfit_sr3;
			    mbna_minmisfit_sx3[1] = mbna_minmisfit_sx1[1] * sqrt(1.0 - mbna_minmisfit_sr3 * mbna_minmisfit_sr3) / mbna_minmisfit_sr3;
			    }
			mbna_minmisfit_sx3[2] = mbna_minmisfit_sr3;
			mbna_minmisfit_sr3 = sqrt(mbna_minmisfit_sx3[0] * mbna_minmisfit_sx3[0] + mbna_minmisfit_sx3[1] * mbna_minmisfit_sx3[1] + mbna_minmisfit_sx3[2] * mbna_minmisfit_sx3[2]);
			}
/* dotproduct = (mbna_minmisfit_sx1[0] * mbna_minmisfit_sx3[0] + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx3[1] + mbna_minmisfit_sx1[2] * mbna_minmisfit_sx3[2]);
fprintf(stderr,"DEBUG %s %d: Perpendicular near-vertical vector in misfit space. %f %f %f  r:%f dotproduct:%f\n",
__FILE__,__LINE__,
mbna_minmisfit_sx3[0],mbna_minmisfit_sx3[1],mbna_minmisfit_sx3[2],mbna_minmisfit_sr2,dotproduct); */

		    /* now get the longest r values to a misfit value <= 2 times minimum misfit
			    for both secondary vectors */
		    mbna_minmisfit_sr2 = 0.0;
		    mbna_minmisfit_sr3 = 0.0;
		    dotproductsave2 = 0.0;
		    rsave2 = 0.0;
		    dotproductsave3 = 0.0;
		    rsave3 = 0.0;
		    for (ic=0;ic<gridm_nx;ic++)
			for (jc=0;jc<gridm_ny;jc++)
			    for (kc=0;kc<nzmisfitcalc;kc++)
				{
				lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
				if (gridnm[lc] > mbna_minmisfit_nthreshold && gridm[lc] <= minmisfitthreshold)
				    {
 				    x = ((ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x - mbna_minmisfit_x) / mbna_mtodeglon;
				    y = ((jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y - mbna_minmisfit_y) / mbna_mtodeglat;
				    z = zmin + zoff_dz * kc - mbna_minmisfit_z;
				    r = sqrt(x * x + y * y + z * z);
				    if (r > mbna_minmisfit_sr2)
					{
					dotproduct = (x * mbna_minmisfit_sx2[0] + y * mbna_minmisfit_sx2[1] + z * mbna_minmisfit_sx2[2]) / r ;
					if (fabs(dotproduct) > 0.8)
					    {
/* fprintf(stderr,"DEBUG %s %d: Vector2: %d %d %d gridm[%d]:%f minmisfitthreshold:%f dotproduct:%f x: %f %f %f  r:%f\n",
__FILE__,__LINE__,
ic,jc,kc,lc,gridm[lc],minmisfitthreshold,dotproduct,x,y,z,r); */
					    mbna_minmisfit_sr2 = r;
					    }
					if (fabs(dotproduct) > dotproductsave2)
					    {
					    dotproductsave2 = fabs(dotproduct);
					    rsave2 = r;
					    }
					}
				    if (r > mbna_minmisfit_sr3)
					{
					dotproduct = (x * mbna_minmisfit_sx3[0] + y * mbna_minmisfit_sx3[1] + z * mbna_minmisfit_sx3[2]) / r ;
					if (fabs(dotproduct) > 0.8)
					    {
/* fprintf(stderr,"DEBUG %s %d: Vector3: %d %d %d gridm[%d]:%f minmisfitthreshold:%f dotproduct:%f x: %f %f %f  r:%f\n",
__FILE__,__LINE__,
ic,jc,kc,lc,gridm[lc],minmisfitthreshold,dotproduct,x,y,z,r); */
					    mbna_minmisfit_sr3 = r;
					    }
					if (fabs(dotproduct) > dotproductsave3)
					    {
					    dotproductsave3 = fabs(dotproduct);
					    rsave3 = r;
					    }
					}
				    }
				}
		    if (mbna_minmisfit_sr2 < MBNA_SMALL)
		    	mbna_minmisfit_sr2 = rsave2;
		    if (mbna_minmisfit_sr3 < MBNA_SMALL)
		    	mbna_minmisfit_sr3 = rsave3;
		    }
		else
		    {
		    mbna_minmisfit_sx1[0] = 1.0;
		    mbna_minmisfit_sx1[1] = 0.0;
		    mbna_minmisfit_sx1[2] = 0.0;
		    mbna_minmisfit_sr1 = 100.0;
		    mbna_minmisfit_sx2[0] = 0.0;
		    mbna_minmisfit_sx2[1] = 1.0;
		    mbna_minmisfit_sx2[2] = 0.0;
		    mbna_minmisfit_sr2 = 100.0;
		    mbna_minmisfit_sx3[0] = 0.0;
		    mbna_minmisfit_sx3[1] = 0.0;
		    mbna_minmisfit_sx3[2] = 1.0;
		    mbna_minmisfit_sr3 = 100.0;
		    }
/* fprintf(stderr,"DEBUG %s %d: \nVector1: %f %f %f  mbna_minmisfit_sr1:%f\n",
__FILE__,__LINE__,
mbna_minmisfit_sx1[0],mbna_minmisfit_sx1[1],mbna_minmisfit_sx1[2],mbna_minmisfit_sr1);
fprintf(stderr,"Vector2: %f %f %f  mbna_minmisfit_sr2:%f\n",
mbna_minmisfit_sx2[0],mbna_minmisfit_sx2[1],mbna_minmisfit_sx2[2],mbna_minmisfit_sr2);
fprintf(stderr,"Vector3: %f %f %f  mbna_minmisfit_sr3:%f\n",
mbna_minmisfit_sx3[0],mbna_minmisfit_sx3[1],mbna_minmisfit_sx3[2],mbna_minmisfit_sr3);
dotproduct = (mbna_minmisfit_sx1[0] * mbna_minmisfit_sx2[0]
		+ mbna_minmisfit_sx1[1] * mbna_minmisfit_sx2[1]
		+ mbna_minmisfit_sx1[2] * mbna_minmisfit_sx2[2]);
fprintf(stderr,"Dot products: 1v2:%f ",dotproduct);
dotproduct = (mbna_minmisfit_sx2[0] * mbna_minmisfit_sx3[0]
		+ mbna_minmisfit_sx2[1] * mbna_minmisfit_sx3[1]
		+ mbna_minmisfit_sx2[2] * mbna_minmisfit_sx3[2]);
fprintf(stderr,"2v3:%f ",dotproduct);
dotproduct = (mbna_minmisfit_sx1[0] * mbna_minmisfit_sx3[0]
		+ mbna_minmisfit_sx1[1] * mbna_minmisfit_sx3[1]
		+ mbna_minmisfit_sx1[2] * mbna_minmisfit_sx3[2]);
fprintf(stderr,"3v2:%f\n",dotproduct); */
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_get_misfitxy()
{
	/* local variables */
	char	*function_name = "mbnavadjust_get_misfitxy";
	int	status = MB_SUCCESS;
	int	ic, jc, kc, lc;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

    	if (project.open == MB_YES
    		&& project.num_crossings > 0
    		&& mbna_current_crossing >= 0
		&& mbna_naverr_load == MB_YES)
    		{
		/* get minimum misfit in plane at current z offset */
		if (grid_nxyzeq > 0)
		    {
		    /* get closest to current zoffset in existing 3d grid */
		    misfit_max = 0.0;
		    misfit_min = 0.0;
		    kc = (int)((mbna_offset_z - zmin) / zoff_dz);
		    for (ic=0;ic<gridm_nx;ic++)
			for (jc=0;jc<gridm_ny;jc++)
			    {
			    lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
			    if (gridnm[lc] > mbna_minmisfit_nthreshold)
				{
				if (misfit_max == 0.0)
			    	    {
				    misfit_min = gridm[lc];
				    misfit_max = gridm[lc];
				    }
				else if (gridm[lc] < misfit_min)
				    {
				    misfit_min = gridm[lc];
				    mbna_minmisfit_xh = (ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x;
				    mbna_minmisfit_yh = (jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y;
				    mbna_minmisfit_zh = zmin + zoff_dz * kc;
				    }
				else if (gridm[lc] > misfit_max)
				    {
				    misfit_max = gridm[lc];
				    }
				}
			    }
/* fprintf(stderr,"mbnavadjust_get_misfitxy a mbna_minmisfit_xh:%f mbna_minmisfit_yh:%f mbna_minmisfit_zh:%f\n",
mbna_minmisfit_xh,mbna_minmisfit_yh,mbna_minmisfit_zh); */
		    }
/* fprintf(stderr,"mbnavadjust_get_misfitxy b mbna_minmisfit_xh:%f mbna_minmisfit_yh:%f mbna_minmisfit_zh:%f\n",
mbna_minmisfit_xh,mbna_minmisfit_yh,mbna_minmisfit_zh); */
		}
/* fprintf(stderr,"mbnavadjust_get_misfitxy c mbna_minmisfit_xh:%f mbna_minmisfit_yh:%f mbna_minmisfit_zh:%f\n",
mbna_minmisfit_xh,mbna_minmisfit_yh,mbna_minmisfit_zh); */

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
void mbnavadjust_plot(double xx,double yy,int ipen)
{
	struct mbna_plot_vector *v;
	double	x, y;

	if (mbna_contour->nvector >= mbna_contour->nvector_alloc)
	    {
	    mbna_contour->nvector_alloc += MBNA_VECTOR_ALLOC_INC;
	    mbna_contour->vector = (struct mbna_plot_vector *)
	    		realloc(mbna_contour->vector,
			sizeof(struct mbna_plot_vector)
			    * (mbna_contour->nvector_alloc));
	    if (mbna_contour->vector == NULL)
		mbna_contour->nvector_alloc = 0;
	    }

	if (mbna_contour->nvector_alloc > mbna_contour->nvector)
	    {
	    /* add current origin */
	    x = xx + mbna_ox;
	    y = yy + mbna_oy;

	    /* move pen */
	    if (ipen == MBNA_PEN_UP)
		    {
		    /* save move in vector array */
		    v = &mbna_contour->vector[mbna_contour->nvector];
		    v->command = ipen;
		    v->x = xx;
		    v->y = yy;
		    mbna_contour->nvector++;
		    }

	    /* plot */
	    else if (ipen == MBNA_PEN_DOWN)
		    {
		    /* save move in vector array */
		    v = &mbna_contour->vector[mbna_contour->nvector];
		    v->command = ipen;
		    v->x = xx;
		    v->y = yy;
		    mbna_contour->nvector++;
		    }

	    /* change origin */
	    else if (ipen == MBNA_PEN_ORIGIN)
		    {
		    mbna_ox = x;
		    mbna_oy = y;
		    }

	    }
	/*fprintf(stderr,"plot: %f %f %d\n",x,y,ipen);*/
	return;
}
/*--------------------------------------------------------------------*/
void mbnavadjust_newpen(int icolor)
{
	struct mbna_plot_vector *v;

	if (mbna_contour->nvector >= mbna_contour->nvector_alloc)
	    {
	    mbna_contour->nvector_alloc += MBNA_VECTOR_ALLOC_INC;
	    mbna_contour->vector = (struct mbna_plot_vector *) realloc(mbna_contour->vector,
			sizeof(struct mbna_plot_vector)
			    * (mbna_contour->nvector_alloc + MBNA_VECTOR_ALLOC_INC));
	    if (mbna_contour->vector == NULL)
		mbna_contour->nvector_alloc = 0;
	    }

	if (mbna_contour->nvector_alloc > mbna_contour->nvector)
	    {
	    /* save pen change in vector array */
	    v = &mbna_contour->vector[mbna_contour->nvector];
 	    v->command = MBNA_PEN_COLOR;
	    v->color = pixel_values[icolor * 8 + 7];
	    mbna_contour->nvector++;
	    }

	/*fprintf(stderr,"newpen: %d\n",icolor);*/
	return;
}
/*--------------------------------------------------------------------*/
void mbnavadjust_setline(int linewidth)
{
	return;
}
/*--------------------------------------------------------------------*/
void mbnavadjust_justify_string(double height,char *string, double *s)
{
	int	len;

	len = strlen(string);
	s[0] = 0.0;
	s[1] = 0.185*height*len;
	s[2] = 0.37*len*height;
	s[3] = 0.37*len*height;

	return;
}
/*--------------------------------------------------------------------*/
void mbnavadjust_plot_string(double x, double y, double hgt, double angle, char *label)
{

	return;
}
/*--------------------------------------------------------------------*/

void
mbnavadjust_naverr_scale()
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_scale";
	int	status = MB_SUCCESS;
	double	xscale, yscale;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	if (mbna_naverr_load == MB_YES)
	    {
	    /* set scaling for contour window */
	    xscale = (cont_borders[1] - cont_borders[0])
		    / ((mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon);
	    yscale = (cont_borders[3] - cont_borders[2])
		    / ((mbna_plot_lat_max - mbna_plot_lat_min) / mbna_mtodeglat);
	    if (xscale < yscale)
		{
		mbna_plotx_scale = xscale / mbna_mtodeglon;
		mbna_ploty_scale = xscale / mbna_mtodeglat;
		mbna_plot_lat_min =  0.5 * (mbna_plot_lat_min + mbna_plot_lat_max)
					- 0.5 * (cont_borders[3] - cont_borders[2]) / mbna_ploty_scale;
		mbna_plot_lat_max =  mbna_plot_lat_min
					+ (cont_borders[3] - cont_borders[2]) / mbna_ploty_scale;
		}
	    else
		{
		mbna_plotx_scale = yscale / mbna_mtodeglon;
		mbna_ploty_scale = yscale / mbna_mtodeglat;
		mbna_plot_lon_min =  0.5 * (mbna_plot_lon_min + mbna_plot_lon_max)
					- 0.5 * (cont_borders[1] - cont_borders[0]) / mbna_plotx_scale;
		mbna_plot_lon_max =  mbna_plot_lon_min
					+ (cont_borders[1] - cont_borders[0]) / mbna_plotx_scale;
		}

	    /* set scaling for misfit window */
	    mbna_misfit_xscale = (corr_borders[1] - corr_borders[0])
		    / (grid_dx * (gridm_nx - 1));
	    mbna_misfit_yscale = (corr_borders[3] - corr_borders[2])
		    / (grid_dy * (gridm_ny - 1));
	    }

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}
}
/*--------------------------------------------------------------------*/

void
mbnavadjust_naverr_plot(int plotmode)
{
	/* local variables */
	char	*function_name = "mbnavadjust_naverr_plot";
	int	status = MB_SUCCESS;
	struct mbna_plot_vector *v;
	struct mbna_crossing *crossing;
	struct mbna_file *file1, *file2;
	struct mbna_section *section1, *section2;
	struct mbna_tie *tie;
	int 	ix, iy, ix1, ix2, iy1, iy2, idx, idy;
	int	boxoff, boxwid;
	static int 	ixo, iyo;
	static int 	izx1, izy1, izx2, izy2;
	static int 	pixel, ipixel;
	int	snav_1, snav_2;
	double	dmisfit;
	int	fill, found;
	int	i, j, k, kk, l;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	if (mbna_naverr_load == MB_YES)
	    {
	    /* get structures */
	    crossing = (struct mbna_crossing *) &project.crossings[mbna_current_crossing];
	    file1 = (struct mbna_file *) &project.files[crossing->file_id_1];
	    file2 = (struct mbna_file *) &project.files[crossing->file_id_2];
	    section1 = (struct mbna_section *) &file1->sections[crossing->section_1];
	    section2 = (struct mbna_section *) &file2->sections[crossing->section_2];

	    /* get naverr plot scaling */
	    mbnavadjust_naverr_scale();

	    /* clear screens for first plot */
	    if (plotmode == MBNA_PLOT_MODE_FIRST)
		{
		xg_fillrectangle(pcont_xgid, 0, 0,
			    cont_borders[1], cont_borders[3],
			    pixel_values[mbna_color_background], XG_SOLIDLINE);
		xg_fillrectangle(pcorr_xgid, 0, 0,
			    corr_borders[1], corr_borders[3],
			    pixel_values[mbna_color_background], XG_SOLIDLINE);
		}
	    xg_fillrectangle(pzoff_xgid, 0, 0,
			    zoff_borders[1], zoff_borders[3],
			    pixel_values[mbna_color_background], XG_SOLIDLINE);

	    /* replot section 2 and tie points in background if moving that section */
	    if (plotmode == MBNA_PLOT_MODE_MOVE)
	    {
	    for (i=0;i<mbna_contour2.nvector;i++)
		{
		v = &mbna_contour2.vector[i];

		if (v->command == MBNA_PEN_UP)
		    {
		    ixo = (int)(mbna_plotx_scale * (v->x + mbna_offset_x_old - mbna_plot_lon_min));
		    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (v->y + mbna_offset_y_old - mbna_plot_lat_min));
		    }
		else if (v->command == MBNA_PEN_DOWN)
		    {
		    ix = (int)(mbna_plotx_scale * (v->x + mbna_offset_x_old - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (v->y + mbna_offset_y_old - mbna_plot_lat_min));
		    xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_background], XG_SOLIDLINE);
		    ixo = ix;
		    iyo = iy;
		    }
		}
	    ixo = (int)(mbna_plotx_scale * (swathraw2->pingraws[0].navlon + mbna_offset_x_old - mbna_plot_lon_min));
	    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw2->pingraws[0].navlat + mbna_offset_y_old - mbna_plot_lat_min));
	    for (i=1;i<swathraw2->npings;i++)
		{
		ix = (int)(mbna_plotx_scale * (swathraw2->pingraws[i].navlon + mbna_offset_x_old - mbna_plot_lon_min));
		iy = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw2->pingraws[i].navlat + mbna_offset_y_old - mbna_plot_lat_min));
		xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_background], XG_SOLIDLINE);
		ixo = ix;
		iyo = iy;
		}

	    /* replot tie points */
	    if (crossing->num_ties > 0)
		{
		for (i=0;i<crossing->num_ties;i++)
		    {
		    tie = &crossing->ties[i];
		    if (i == mbna_current_tie)
			{
			boxoff = 6;
			boxwid = 13;
			snav_1 = mbna_snav_1;
			snav_2 = mbna_snav_2;
			}
		    else
			{
			boxoff = 3;
			boxwid = 7;
			snav_1 = tie->snav_1;
			snav_2 = tie->snav_2;
			}
		    ix = (int)(mbna_plotx_scale * (section1->snav_lon[snav_1] - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (section1->snav_lat[snav_1] - mbna_plot_lat_min));
		    xg_fillrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[mbna_color_background], XG_SOLIDLINE);
		    xg_drawrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[mbna_color_background], XG_SOLIDLINE);
		    ixo = ix;
		    iyo = iy;
		    ix = (int)(mbna_plotx_scale * (section2->snav_lon[snav_2] + mbna_offset_x_old - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (section2->snav_lat[snav_2] + mbna_offset_y_old - mbna_plot_lat_min));
		    xg_fillrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[mbna_color_background], XG_SOLIDLINE);
		    xg_drawrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[mbna_color_background], XG_SOLIDLINE);
		    xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_background], XG_SOLIDLINE);
		    }
		}
	    }

	    /* replot zoom box in background if moving that box */
	    if (plotmode == MBNA_PLOT_MODE_ZOOM)
		{
		xg_drawrectangle(pcont_xgid,
				    MIN(izx1, izx2),
				    MIN(izy1, izy2),
				    MAX(izx1, izx2) - MIN(izx1, izx2),
				    MAX(izy1, izy2) - MIN(izy1, izy2),
				    pixel_values[mbna_color_background], XG_SOLIDLINE);
		}

	    /* replot overlap box in background */
	    if (mbna_overlap_lon_max > mbna_overlap_lon_min && mbna_overlap_lat_max > mbna_overlap_lat_min)
		{
		ix1 = (int)(mbna_plotx_scale * (mbna_overlap_lon_min - mbna_plot_lon_min));
		iy1 = (int)(cont_borders[3] - mbna_ploty_scale * (mbna_overlap_lat_min - mbna_plot_lat_min));
		ix2 = (int)(mbna_plotx_scale * (mbna_overlap_lon_max - mbna_plot_lon_min));
		iy2 = (int)(cont_borders[3] - mbna_ploty_scale * (mbna_overlap_lat_max - mbna_plot_lat_min));
		ix = MIN(ix1,ix2);
		iy = MIN(iy1,iy2);
		idx = MAX(ix1,ix2) - ix;
		idy = MAX(iy1,iy2) - iy;
		xg_drawrectangle(pcont_xgid,
	    			    ix, iy, idx, idy,
				    pixel_values[mbna_color_background], XG_DASHLINE);
		}

	    /* plot section 1 */
	    for (i=0;i<mbna_contour1.nvector;i++)
		{
		v = &mbna_contour1.vector[i];

		if (v->command == MBNA_PEN_COLOR)
		    {
		    pixel = v->color;
		    }
		else if (v->command == MBNA_PEN_UP)
		    {
		    ixo = (int)(mbna_plotx_scale * (v->x - mbna_plot_lon_min));
		    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (v->y - mbna_plot_lat_min));
		    }
		else if (v->command == MBNA_PEN_DOWN)
		    {
		    ix = (int)(mbna_plotx_scale * (v->x - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (v->y - mbna_plot_lat_min));
		    xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel, XG_SOLIDLINE);
		    ixo = ix;
		    iyo = iy;
		    }
		}
	    ixo = (int)(mbna_plotx_scale * (swathraw1->pingraws[0].navlon - mbna_plot_lon_min));
	    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw1->pingraws[0].navlat - mbna_plot_lat_min));
	    for (i=1;i<swathraw1->npings;i++)
		{
		ix = (int)(mbna_plotx_scale * (swathraw1->pingraws[i].navlon - mbna_plot_lon_min));
		iy = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw1->pingraws[i].navlat - mbna_plot_lat_min));
		xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		ixo = ix;
		iyo = iy;
		}

	    /* plot section 2 */
	    for (i=0;i<mbna_contour2.nvector;i++)
		{
		v = &mbna_contour2.vector[i];

		if (v->command == MBNA_PEN_COLOR)
		    {
		    pixel = v->color;
		    }
		else if (v->command == MBNA_PEN_UP)
		    {
		    ixo = (int)(mbna_plotx_scale * (v->x + mbna_offset_x - mbna_plot_lon_min));
		    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (v->y + mbna_offset_y - mbna_plot_lat_min));
		    }
		else if (v->command == MBNA_PEN_DOWN)
		    {
		    ix = (int)(mbna_plotx_scale * (v->x + mbna_offset_x - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (v->y + mbna_offset_y - mbna_plot_lat_min));
		    xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel, XG_SOLIDLINE);
		    ixo = ix;
		    iyo = iy;
		    }
		}
	    ixo = (int)(mbna_plotx_scale * (swathraw2->pingraws[0].navlon + mbna_offset_x - mbna_plot_lon_min));
	    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw2->pingraws[0].navlat + mbna_offset_y - mbna_plot_lat_min));
	    for (i=1;i<swathraw2->npings;i++)
		{
		ix = (int)(mbna_plotx_scale * (swathraw2->pingraws[i].navlon + mbna_offset_x - mbna_plot_lon_min));
		iy = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw2->pingraws[i].navlat + mbna_offset_y - mbna_plot_lat_min));
		xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		ixo = ix;
		iyo = iy;
		}

	    /* plot tie points */
	    mbnavadjust_naverr_checkoksettie();
	    crossing = &project.crossings[mbna_current_crossing];
	    if (crossing->num_ties > 0)
		{
		for (i=0;i<crossing->num_ties;i++)
		    {
		    tie = &crossing->ties[i];
		    if (i == mbna_current_tie)
			{
			boxoff = 6;
			boxwid = 13;
			snav_1 = mbna_snav_1;
			snav_2 = mbna_snav_2;
			if (mbna_allow_set_tie == MB_YES)
				fill = pixel_values[RED];
		    	else
				fill = pixel_values[6];
			}
		    else
			{
			boxoff = 3;
			boxwid = 7;
			snav_1 = tie->snav_1;
			snav_2 = tie->snav_2;
		    	fill = pixel_values[6];
			}
		    ix = (int)(mbna_plotx_scale * (section1->snav_lon[snav_1] - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (section1->snav_lat[snav_1] - mbna_plot_lat_min));
		    xg_fillrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, fill, XG_SOLIDLINE);
		    xg_drawrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		    ixo = ix;
		    iyo = iy;
		    ix = (int)(mbna_plotx_scale * (section2->snav_lon[snav_2] + mbna_offset_x - mbna_plot_lon_min));
		    iy = (int)(cont_borders[3] - mbna_ploty_scale * (section2->snav_lat[snav_2] + mbna_offset_y - mbna_plot_lat_min));
		    xg_fillrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, fill, XG_SOLIDLINE);
		    xg_drawrectangle(pcont_xgid, ix-boxoff, iy-boxoff, boxwid, boxwid, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		    xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		    }
		}

	    /* plot overlap box */
	    mbnavadjust_crossing_overlapbounds(mbna_current_crossing, mbna_offset_x, mbna_offset_y,
	    					&mbna_overlap_lon_min, &mbna_overlap_lon_max,
						&mbna_overlap_lat_min, &mbna_overlap_lat_max);
	    ix1 = (int)(mbna_plotx_scale * (mbna_overlap_lon_min - mbna_plot_lon_min));
	    iy1 = (int)(cont_borders[3] - mbna_ploty_scale * (mbna_overlap_lat_min - mbna_plot_lat_min));
	    ix2 = (int)(mbna_plotx_scale * (mbna_overlap_lon_max - mbna_plot_lon_min));
	    iy2 = (int)(cont_borders[3] - mbna_ploty_scale * (mbna_overlap_lat_max - mbna_plot_lat_min));
	    ix = MIN(ix1,ix2);
	    iy = MIN(iy1,iy2);
	    idx = MAX(ix1,ix2) - ix;
	    idy = MAX(iy1,iy2) - iy;
	    xg_drawrectangle(pcont_xgid,
	    			ix, iy, idx, idy,
				pixel_values[mbna_color_foreground], XG_DASHLINE);

	    /* plot zoom box if in zoom mode */
	    if (plotmode == MBNA_PLOT_MODE_ZOOMFIRST || plotmode == MBNA_PLOT_MODE_ZOOM)
		{
		xg_drawrectangle(pcont_xgid,
				    MIN(mbna_zoom_x1, mbna_zoom_x2),
				    MIN(mbna_zoom_y1, mbna_zoom_y2),
				    MAX(mbna_zoom_x1, mbna_zoom_x2) - MIN(mbna_zoom_x1, mbna_zoom_x2),
				    MAX(mbna_zoom_y1, mbna_zoom_y2) - MIN(mbna_zoom_y1, mbna_zoom_y2),
				    pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		izx1 = mbna_zoom_x1;
		izy1 = mbna_zoom_y1;
		izx2 = mbna_zoom_x2;
		izy2 = mbna_zoom_y2;
		}

	    /* plot misfit */
	    ixo = corr_borders[0] + (corr_borders[1] - corr_borders[0]) / 2;
	    iyo = corr_borders[2] + (corr_borders[3] - corr_borders[2]) / 2;
	    dmisfit = log10(misfit_max - misfit_min)/79.99;
	    k = (int)((mbna_offset_z - zmin) / zoff_dz);
	    for (i=0;i<gridm_nx;i++)
		for(j=0;j<gridm_ny;j++)
		    {
		    l = k + nzmisfitcalc * (i + j * gridm_nx);
		    if (gridnm[l] > 0)
			{
			ix = ixo + (int)(mbna_misfit_xscale * grid_dx
					    * (i - gridm_nx / 2 - 0.5));
			iy = iyo - (int)(mbna_misfit_yscale * grid_dy
					    * (j - gridm_ny / 2 + 0.5));
			idx = ixo + (int)(mbna_misfit_xscale * grid_dx
					    * (i - gridm_nx / 2 + 0.5))
				    - ix;
			idy = iyo - (int)(mbna_misfit_yscale * grid_dy
					    * (j - gridm_ny / 2 - 0.5))
				    - iy;

    			/* histogram equalized coloring */
			if (gridm[l] <= misfit_intervals[0])
				ipixel = 7;
			else if (gridm[l] >= misfit_intervals[nmisfit_intervals-1])
				ipixel = 7 + nmisfit_intervals - 1;
			else
				{
				found = MB_NO;
				for (kk=0;kk<nmisfit_intervals && found == MB_NO;kk++)
					{
					if (gridm[l] > misfit_intervals[kk]
						&& gridm[l] <= misfit_intervals[kk+1])
						{
						ipixel = 7 + kk;
						found = MB_YES;
						}
					}
				}
/*fprintf(stderr, "%d %d %f %f %f   %f %d\n",
    i, j, misfit_min, misfit_max, dmisfit, gridm[l], ipixel);*/

			xg_fillrectangle(pcorr_xgid,
				    ix, iy, idx, idy,
				    pixel_values[ipixel], XG_SOLIDLINE);
			}
		    }

	    /* draw dashed crosshair across origin */
	    xg_drawline(pcorr_xgid,
			    ixo - (int)(mbna_misfit_xscale * mbna_misfit_offset_x),
			    corr_borders[2],
			    ixo - (int)(mbna_misfit_xscale * mbna_misfit_offset_x),
			    corr_borders[3],
			    pixel_values[mbna_color_foreground], XG_DASHLINE);
	    xg_drawline(pcorr_xgid,
			    corr_borders[0],
			    iyo + (int)(mbna_misfit_yscale * mbna_misfit_offset_y),
			    corr_borders[1],
			    iyo + (int)(mbna_misfit_yscale * mbna_misfit_offset_y),
			    pixel_values[mbna_color_foreground], XG_DASHLINE);

	    /* draw working offset */
	    ix = ixo + (int)(mbna_misfit_xscale * (mbna_offset_x - mbna_misfit_offset_x));
	    iy = iyo - (int)(mbna_misfit_yscale * (mbna_offset_y - mbna_misfit_offset_y));
	    xg_fillrectangle(pcorr_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
	    xg_drawrectangle(pcorr_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

	    /* draw uncertainty estimate */
	    if (mbna_minmisfit_n > 0)
	    	{
		ix = ixo + (int)(mbna_misfit_xscale * (mbna_minmisfit_x - mbna_misfit_offset_x));
		iy = iyo - (int)(mbna_misfit_yscale * (mbna_minmisfit_y - mbna_misfit_offset_y));
		idx = (int)(mbna_misfit_xscale * (mbna_mtodeglon * mbna_minmisfit_sr1 * mbna_minmisfit_sx1[0]));
		idy = -(int)(mbna_misfit_yscale * (mbna_mtodeglat * mbna_minmisfit_sr1 * mbna_minmisfit_sx1[1]));
		xg_drawline(pcorr_xgid,
				ix - idx, iy - idy,
				ix + idx, iy + idy,
				pixel_values[mbna_color_background], XG_SOLIDLINE);

		ix = ixo + (int)(mbna_misfit_xscale * (mbna_minmisfit_x - mbna_misfit_offset_x));
		iy = iyo - (int)(mbna_misfit_yscale * (mbna_minmisfit_y - mbna_misfit_offset_y));
		idx = (int)(mbna_misfit_xscale * (mbna_mtodeglon * mbna_minmisfit_sr2 * mbna_minmisfit_sx2[0]));
		idy = -(int)(mbna_misfit_yscale * (mbna_mtodeglat * mbna_minmisfit_sr2 * mbna_minmisfit_sx2[1]));
		xg_drawline(pcorr_xgid,
				ix - idx, iy - idy,
				ix + idx, iy + idy,
				pixel_values[mbna_color_background], XG_SOLIDLINE);
		}

	    /* draw x at minimum misfit */
	    if (mbna_minmisfit_n > 0)
	    	{
		ix = ixo + (int)(mbna_misfit_xscale * (mbna_minmisfit_x - mbna_misfit_offset_x));
		iy = iyo - (int)(mbna_misfit_yscale * (mbna_minmisfit_y - mbna_misfit_offset_y));
		xg_drawline(pcorr_xgid,
				ix - 10, iy + 10,
				ix + 10, iy - 10,
				pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		xg_drawline(pcorr_xgid,
				ix + 10, iy + 10,
				ix - 10, iy - 10,
				pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		}

	    /* draw small x at minimum misfit for current z offset */
	    if (mbna_minmisfit_n > 0)
	    	{
		ix = ixo + (int)(mbna_misfit_xscale * (mbna_minmisfit_xh - mbna_misfit_offset_x));
		iy = iyo - (int)(mbna_misfit_yscale * (mbna_minmisfit_yh - mbna_misfit_offset_y));
		xg_drawline(pcorr_xgid,
				ix - 5, iy + 5,
				ix + 5, iy - 5,
				pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		xg_drawline(pcorr_xgid,
				ix + 5, iy + 5,
				ix - 5, iy - 5,
				pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		}

	    /* draw + at inversion solution */
	    if (project.inversion != MBNA_INVERSION_NONE)
	    	{
	    	ix = ixo + (int)(mbna_misfit_xscale * (mbna_invert_offset_x - mbna_misfit_offset_x));
	    	iy = iyo - (int)(mbna_misfit_yscale * (mbna_invert_offset_y - mbna_misfit_offset_y));
	    	xg_drawline(pcorr_xgid,
			    ix - 10, iy,
			    ix + 10, iy,
			    pixel_values[GREEN], XG_SOLIDLINE);
	    	xg_drawline(pcorr_xgid,
			    ix, iy + 10,
			    ix, iy - 10,
			    pixel_values[GREEN], XG_SOLIDLINE);
	    	xg_drawline(pcorr_xgid,
			    ix - 10, iy,
			    ix + 10, iy,
			    pixel_values[mbna_color_foreground], XG_SOLIDLINE);
	    	xg_drawline(pcorr_xgid,
			    ix, iy + 10,
			    ix, iy - 10,
			    pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		}

	    /* plot zoff */
	    ixo = zoff_borders[0];
	    iyo = zoff_borders[3];
	    i = (int)((mbna_offset_x - mbna_misfit_offset_x) / grid_dx) + (int)(gridm_nx / 2);
	    i = MAX(0, MIN(gridm_nx-1, i));
	    j = (int)((mbna_offset_y - mbna_misfit_offset_y) / grid_dy) + (int)(gridm_ny / 2);
	    j = MAX(0, MIN(gridm_ny-1, j));
	    found = MB_NO;
	    zmisfitmin = 10000000.0;
	    zmisfitmax = 0.0;
	    for (k=0;k<nzmisfitcalc;k++)
		{
		l = k + nzmisfitcalc * (i + j * gridm_nx);
		if (gridnm[l] > 0)
		    {
		    if (found == MB_NO)
			{
			zmisfitmin = gridm[l];
			zmisfitmax = gridm[l];
			found = MB_YES;
			}
		    else
			{
			zmisfitmin = MIN(zmisfitmin, gridm[l]);
			zmisfitmax = MAX(zmisfitmax, gridm[l]);
			}
		    }
		}
/*fprintf(stderr,"Current offset: %f %f %f\n",
mbna_offset_x / mbna_mtodeglon, mbna_offset_y / mbna_mtodeglat, mbna_offset_z);
fprintf(stderr,"Current misfit grid offset: %f %f %f\n",
mbna_misfit_offset_x / mbna_mtodeglon, mbna_misfit_offset_y / mbna_mtodeglat, mbna_misfit_offset_z);
fprintf(stderr,"Current min misfit position: %f %f %f\n",
mbna_minmisfit_x / mbna_mtodeglon, mbna_minmisfit_y / mbna_mtodeglat, mbna_minmisfit_z);
fprintf(stderr,"misfitmin:%f misfitmax:%f  zmisfitmin:%f zmisfitmax:%f\n\n",
misfit_min,misfit_max,zmisfitmin,zmisfitmax);*/
	    zmisfitmin = zmisfitmin - 0.05 * (zmisfitmax - zmisfitmin);
	    zmisfitmax = zmisfitmax + 0.04 * (zmisfitmax - zmisfitmin);
	    mbna_zoff_scale_x = (zoff_borders[1] - zoff_borders[0])
		    / (project.zoffsetwidth);
	    mbna_zoff_scale_y = (zoff_borders[3] - zoff_borders[2])
		    / (zmisfitmax - zmisfitmin);
	    for (k=0;k<nzmisfitcalc;k++)
		{
		l = k + nzmisfitcalc * (i + j * gridm_nx);
		if (gridnm[l] > 0)
		    {
   		    /* histogram equalized coloring */
		    if (gridm[l] <= misfit_intervals[0])
			    ipixel = 7;
		    else if (gridm[l] >= misfit_intervals[nmisfit_intervals-1])
			    ipixel = 7 + nmisfit_intervals - 1;
		    else
			{
			found = MB_NO;
			for (kk=0;kk<nmisfit_intervals && found == MB_NO;kk++)
			    {
			    if (gridm[l] > misfit_intervals[kk]
				    && gridm[l] <= misfit_intervals[kk+1])
				{
				ipixel = 7 + kk;
				found = MB_YES;
				}
			    }
			}
		    ix = ixo + (int)(mbna_zoff_scale_x * zoff_dz * (k - 0.5));
		    iy = (int)(mbna_zoff_scale_y * (gridm[l] - zmisfitmin));
		    idx = (int)(mbna_zoff_scale_x * zoff_dz);
		    idx = MAX(idx, 1);
		    idy = iyo - iy;
/* fprintf(stderr,"Fill Zoff: %d %d %d %d  pixel:%d\n",
ix, iy, idx, idy, pixel_values[ipixel]);*/
		    xg_fillrectangle(pzoff_xgid,
				ix, iy, idx, idy,
				pixel_values[ipixel], XG_SOLIDLINE);
		    }
		}

	    /* plot zero zoff */
	    ix = ixo - (int)(mbna_zoff_scale_x * zmin);
	    xg_drawline(pzoff_xgid,
			    ix, zoff_borders[2],
			    ix, zoff_borders[3],
			    pixel_values[mbna_color_foreground], XG_DASHLINE);

	    /* draw working offset */
	    ix = ixo + (int)(mbna_zoff_scale_x * (mbna_offset_z - zmin));
	    xg_drawline(pzoff_xgid,
			    ix, zoff_borders[2],
			    ix, zoff_borders[3],
			    pixel_values[mbna_color_foreground], XG_SOLIDLINE);

	    /* draw x at minimum misfit */
	    if (mbna_minmisfit_n > 0)
	    	{
		ix = ixo + (int)(mbna_zoff_scale_x * (mbna_minmisfit_z - zmin));
		iy = zoff_borders[3] / 2;
		xg_drawline(pzoff_xgid,
				ix - 10, iy + 10,
				ix + 10, iy - 10,
				pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		xg_drawline(pzoff_xgid,
				ix + 10, iy + 10,
				ix - 10, iy - 10,
				pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		}

	    /* draw + at inversion solution */
	    if (project.inversion != MBNA_INVERSION_NONE)
	    	{
	    	ix = ixo + (int)(mbna_zoff_scale_x * (mbna_invert_offset_z - zmin));
	    	iy = zoff_borders[3] / 2;
	    	xg_drawline(pzoff_xgid,
			    ix - 10, iy,
			    ix + 10, iy,
			    pixel_values[mbna_color_foreground], XG_SOLIDLINE);
	    	xg_drawline(pzoff_xgid,
			    ix, iy + 10,
			    ix, iy - 10,
			    pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		}

	    }

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}
}
/*--------------------------------------------------------------------*/

int
mbnavadjust_autopick(int do_vertical)
{
	/* local variables */
	char	*function_name = "mbnavadjust_autopick";
	int	status = MB_SUCCESS;
	struct 	mbna_crossing *crossing;
	struct 	mbna_tie *tie;
	double	firstsonardepth1, firsttime_d1, secondsonardepth1, secondtime_d1, dsonardepth1;
	double	firstsonardepth2, firsttime_d2, secondsonardepth2, secondtime_d2, dsonardepth2;
	double	overlap_scale;
	int	found, process;
	int	nprocess;
	int	i, j, k;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       do_vertical: %d\n",do_vertical);
		}

	/* make sure that all sections referenced in crossings have up-to-date contours made */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
		/* set message dialog on */
		sprintf(message,"Autopicking offsets...");
		do_message_on(message);
		sprintf(message,"Autopicking offsets...\n");
		if (mbna_verbose == 0)
		    fprintf(stderr,"%s\n",message);
		do_info_add(message,MB_YES);

		/* loop over all crossings */
		nprocess = 0;
		for (i=0;i<project.num_crossings;i++)
			{
			/* get structure */
			crossing = &(project.crossings[i]);

			/* check if processing should proceed */
			process = MB_NO;
			if (crossing->status == MBNA_CROSSING_STATUS_NONE
				&& crossing->overlap >=  MBNA_MEDIOCREOVERLAP_THRESHOLD)
				{
				if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS)
					{
					if ((mbna_view_mode == MBNA_VIEW_MODE_ALL)
						|| (mbna_view_mode == MBNA_VIEW_MODE_SURVEY
							&& mbna_survey_select == project.files[crossing->file_id_1].block
							&& mbna_survey_select == project.files[crossing->file_id_2].block)
						|| (mbna_view_mode == MBNA_VIEW_MODE_FILE
							&& mbna_file_select == crossing->file_id_1
							&& mbna_file_select == crossing->file_id_2)
						|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY
							&& (mbna_survey_select == project.files[crossing->file_id_1].block
								|| mbna_survey_select == project.files[crossing->file_id_2].block))
						|| (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE
							&& (mbna_file_select == crossing->file_id_1
								|| mbna_file_select == crossing->file_id_2))
						|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION
							&& mbna_file_select == crossing->file_id_1
							&& mbna_section_select == crossing->section_1)
						|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION
							&& mbna_file_select == crossing->file_id_2
							&& mbna_section_select == crossing->section_2))
						process = MB_YES;
					}
				else if (mbna_view_list == MBNA_VIEW_LIST_MEDIOCRECROSSINGS)
					{
					if (crossing->overlap >= MBNA_MEDIOCREOVERLAP_THRESHOLD)
						{
						if ((mbna_view_mode == MBNA_VIEW_MODE_ALL)
							|| (mbna_view_mode == MBNA_VIEW_MODE_SURVEY
								&& mbna_survey_select == project.files[crossing->file_id_1].block
								&& mbna_survey_select == project.files[crossing->file_id_2].block)
							|| (mbna_view_mode == MBNA_VIEW_MODE_FILE
								&& mbna_file_select == crossing->file_id_1
								&& mbna_file_select == crossing->file_id_2)
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY
								&& (mbna_survey_select == project.files[crossing->file_id_1].block
									|| mbna_survey_select == project.files[crossing->file_id_2].block))
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE
								&& (mbna_file_select == crossing->file_id_1
									|| mbna_file_select == crossing->file_id_2))
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION
								&& mbna_file_select == crossing->file_id_1
								&& mbna_section_select == crossing->section_1)
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION
								&& mbna_file_select == crossing->file_id_2
								&& mbna_section_select == crossing->section_2))
						process = MB_YES;
						}
					}
				else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS)
					{
					if (crossing->overlap >= MBNA_GOODOVERLAP_THRESHOLD)
						{
						if ((mbna_view_mode == MBNA_VIEW_MODE_ALL)
							|| (mbna_view_mode == MBNA_VIEW_MODE_SURVEY
								&& mbna_survey_select == project.files[crossing->file_id_1].block
								&& mbna_survey_select == project.files[crossing->file_id_2].block)
							|| (mbna_view_mode == MBNA_VIEW_MODE_FILE
								&& mbna_file_select == crossing->file_id_1
								&& mbna_file_select == crossing->file_id_2)
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY
								&& (mbna_survey_select == project.files[crossing->file_id_1].block
									|| mbna_survey_select == project.files[crossing->file_id_2].block))
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE
								&& (mbna_file_select == crossing->file_id_1
									|| mbna_file_select == crossing->file_id_2))
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION
								&& mbna_file_select == crossing->file_id_1
								&& mbna_section_select == crossing->section_1)
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION
								&& mbna_file_select == crossing->file_id_2
								&& mbna_section_select == crossing->section_2))
						process = MB_YES;
						}
					}
				else if (mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS)
					{
					if (crossing->overlap >= MBNA_BETTEROVERLAP_THRESHOLD)
						{
						if ((mbna_view_mode == MBNA_VIEW_MODE_ALL)
							|| (mbna_view_mode == MBNA_VIEW_MODE_SURVEY
								&& mbna_survey_select == project.files[crossing->file_id_1].block
								&& mbna_survey_select == project.files[crossing->file_id_2].block)
							|| (mbna_view_mode == MBNA_VIEW_MODE_FILE
								&& mbna_file_select == crossing->file_id_1
								&& mbna_file_select == crossing->file_id_2)
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY
								&& (mbna_survey_select == project.files[crossing->file_id_1].block
									|| mbna_survey_select == project.files[crossing->file_id_2].block))
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE
								&& (mbna_file_select == crossing->file_id_1
									|| mbna_file_select == crossing->file_id_2))
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION
								&& mbna_file_select == crossing->file_id_1
								&& mbna_section_select == crossing->section_1)
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION
								&& mbna_file_select == crossing->file_id_2
								&& mbna_section_select == crossing->section_2))
						process = MB_YES;
						}
					}
				else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS)
					{
					if (crossing->truecrossing == MB_YES)
						{
						if ((mbna_view_mode == MBNA_VIEW_MODE_ALL)
							|| (mbna_view_mode == MBNA_VIEW_MODE_SURVEY
								&& mbna_survey_select == project.files[crossing->file_id_1].block
								&& mbna_survey_select == project.files[crossing->file_id_2].block)
							|| (mbna_view_mode == MBNA_VIEW_MODE_FILE
								&& mbna_file_select == crossing->file_id_1
								&& mbna_file_select == crossing->file_id_2)
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY
								&& (mbna_survey_select == project.files[crossing->file_id_1].block
									|| mbna_survey_select == project.files[crossing->file_id_2].block))
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE
								&& (mbna_file_select == crossing->file_id_1
									|| mbna_file_select == crossing->file_id_2))
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION
								&& mbna_file_select == crossing->file_id_1
								&& mbna_section_select == crossing->section_1)
							|| (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION
								&& mbna_file_select == crossing->file_id_2
								&& mbna_section_select == crossing->section_2))
						process = MB_YES;
						}
					}
				else
					process = MB_YES;
				}
/* fprintf(stderr,"AUTOPICK crossing:%d do_vertical:%d process:%d\n",i,do_vertical,process); */

			/* load the crossing */
			if (process == MB_YES)
				{
				mbna_current_crossing = i;
    				mbna_file_id_1 = crossing->file_id_1;
    				mbna_section_1 = crossing->section_1;
     				mbna_file_id_2 = crossing->file_id_2;
    				mbna_section_2 = crossing->section_2;
				mbna_current_tie = -1;

				/* reset survey file and section selections */
                                if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY
                                    || mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
                                        {
                                        if (mbna_survey_select == project.files[crossing->file_id_1].block)
                                                {
                                                mbna_file_select = crossing->file_id_1;
                                                mbna_section_select = crossing->section_1;
                                                }
                                        else if (mbna_survey_select == project.files[crossing->file_id_2].block)
                                                {
                                                mbna_file_select = crossing->file_id_2;
                                                mbna_section_select = crossing->section_2;
                                                }
                                        else
                                                {
                                                mbna_file_select = crossing->file_id_1;
                                                mbna_section_select = crossing->section_1;
                                                }
                                        }
                                else if (mbna_view_mode == MBNA_VIEW_MODE_FILE
                                    || mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
                                        {
                                        if (mbna_file_select == crossing->file_id_1)
                                                {
                                                mbna_survey_select = project.files[crossing->file_id_1].block;
                                                mbna_section_select = crossing->section_1;
                                                }
                                        else if (mbna_file_select == crossing->file_id_2)
                                                {
                                                mbna_survey_select = project.files[crossing->file_id_2].block;
                                                mbna_section_select = crossing->section_2;
                                                }
                                        else
                                                {
                                                mbna_survey_select = project.files[crossing->file_id_1].block;
                                                mbna_section_select = crossing->section_1;
                                                }
                                        }
                                else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
                                        {
                                        if (mbna_file_select == crossing->file_id_1
                                            && mbna_section_select == crossing->section_1)
                                                {
                                                mbna_survey_select = project.files[crossing->file_id_1].block;
                                                mbna_file_select = crossing->file_id_1;
                                                }
                                        else if (mbna_file_select == crossing->file_id_2
                                            && mbna_section_select == crossing->section_2)
                                                {
                                                mbna_survey_select = project.files[crossing->file_id_2].block;
                                                mbna_file_select = crossing->file_id_2;
                                                }
                                        else
                                                {
                                                mbna_survey_select = project.files[crossing->file_id_1].block;
                                                mbna_file_select = crossing->file_id_1;
                                                }
                                        }
				else if (mbna_file_select == crossing->file_id_1)
					{
					mbna_survey_select = project.files[crossing->file_id_1].block;
					mbna_file_select = crossing->file_id_1;
					mbna_section_select = crossing->section_1;
					}
				else if (mbna_file_select == crossing->file_id_2)
					{
					mbna_survey_select = project.files[crossing->file_id_2].block;
					mbna_file_select = crossing->file_id_2;
					mbna_section_select = crossing->section_2;
					}
				else
					{
					mbna_survey_select = project.files[crossing->file_id_1].block;
					mbna_file_select = crossing->file_id_1;
					mbna_section_select = crossing->section_1;
					}

				/* set message dialog on */
				sprintf(message,"Loading crossing %d...", mbna_current_crossing);
				fprintf(stderr,"\n%s: %s\n",function_name,message);
				do_message_update(message);

    				/* load crossing */
  				mbnavadjust_crossing_load();
/* fprintf(stderr,"mbnavadjust_autopick AA crossing:%d overlap:%d overlap_scale:%f current offsets:%f %f %f  minmisfit3D:%f %f %f  minmisfit2D:%f %f %f\n",
mbna_current_crossing,crossing->overlap,overlap_scale,
mbna_offset_x/mbna_mtodeglon,mbna_offset_y/mbna_mtodeglat,mbna_offset_z,
mbna_minmisfit_x/mbna_mtodeglon,mbna_minmisfit_y/mbna_mtodeglat,mbna_minmisfit_z,
mbna_minmisfit_xh/mbna_mtodeglon,mbna_minmisfit_yh/mbna_mtodeglat,mbna_minmisfit_zh); */
				nprocess++;

				/* update status */
				do_update_status();
/* fprintf(stderr,"mbnavadjust_autopick A crossing:%d overlap:%d overlap_scale:%f current offsets:%f %f %f  minmisfit3D:%f %f %f  minmisfit2D:%f %f %f\n",
mbna_current_crossing,crossing->overlap,overlap_scale,
mbna_offset_x/mbna_mtodeglon,mbna_offset_y/mbna_mtodeglat,mbna_offset_z,
mbna_minmisfit_x/mbna_mtodeglon,mbna_minmisfit_y/mbna_mtodeglat,mbna_minmisfit_z,
mbna_minmisfit_xh/mbna_mtodeglon,mbna_minmisfit_yh/mbna_mtodeglat,mbna_minmisfit_zh); */

				/* if this is a >50% overlap crossing then first set offsets to
					minimum misfit and then recalculate misfit */
				if (crossing->overlap > 50)
					{
					/* set offsets to minimum misfit */
					if (do_vertical == MB_YES)
						{
						mbna_offset_x = mbna_minmisfit_x;
						mbna_offset_y = mbna_minmisfit_y;
						mbna_offset_z = mbna_minmisfit_z;
						}
					else
						{
						mbna_offset_x = mbna_minmisfit_xh;
						mbna_offset_y = mbna_minmisfit_yh;
						mbna_offset_z = mbna_minmisfit_zh;
						}
					mbna_misfit_offset_x = mbna_offset_x;
					mbna_misfit_offset_y = mbna_offset_y;
					mbna_misfit_offset_z = mbna_offset_z;
					mbnavadjust_crossing_replot();

					/* get misfit */
					mbnavadjust_get_misfit();
/* fprintf(stderr,"mbnavadjust_autopick B crossing:%d overlap:%d overlap_scale:%f current offsets:%f %f %f  minmisfit3D:%f %f %f  minmisfit2D:%f %f %f\n",
mbna_current_crossing,crossing->overlap,overlap_scale,
mbna_offset_x/mbna_mtodeglon,mbna_offset_y/mbna_mtodeglat,mbna_offset_z,
mbna_minmisfit_x/mbna_mtodeglon,mbna_minmisfit_y/mbna_mtodeglat,mbna_minmisfit_z,
mbna_minmisfit_xh/mbna_mtodeglon,mbna_minmisfit_yh/mbna_mtodeglat,mbna_minmisfit_zh); */
					}

				/* set plot bounds to overlap region */
				mbnavadjust_crossing_overlapbounds(mbna_current_crossing,
							mbna_offset_x, mbna_offset_y,
	    						&mbna_overlap_lon_min, &mbna_overlap_lon_max,
							&mbna_overlap_lat_min, &mbna_overlap_lat_max);
				mbna_plot_lon_min = mbna_overlap_lon_min;
				mbna_plot_lon_max = mbna_overlap_lon_max;
				mbna_plot_lat_min = mbna_overlap_lat_min;
				mbna_plot_lat_max = mbna_overlap_lat_max;
				overlap_scale = MIN((mbna_overlap_lon_max - mbna_overlap_lon_min) / mbna_mtodeglon,
							(mbna_overlap_lat_max - mbna_overlap_lat_min) / mbna_mtodeglat);

				/* get naverr plot scaling */
				mbnavadjust_naverr_scale();

				/* get misfit */
				mbnavadjust_get_misfit();

				/* check uncertainty estimate for a good pick */
/* fprintf(stderr,"mbnavadjust_autopick C crossing:%d overlap:%d overlap_scale:%f current offsets:%f %f %f  minmisfit3D:%f %f %f  minmisfit2D:%f %f %f\n",
mbna_current_crossing,crossing->overlap,overlap_scale,
mbna_offset_x/mbna_mtodeglon,mbna_offset_y/mbna_mtodeglat,mbna_offset_z,
mbna_minmisfit_x/mbna_mtodeglon,mbna_minmisfit_y/mbna_mtodeglat,mbna_minmisfit_z,
mbna_minmisfit_xh/mbna_mtodeglon,mbna_minmisfit_yh/mbna_mtodeglat,mbna_minmisfit_zh);
fprintf(stderr,"crossing:%d overlap:%d overlap_scale:%f uncertainty axes: %f %f %f",
mbna_current_crossing,crossing->overlap,overlap_scale,mbna_minmisfit_sr1,mbna_minmisfit_sr2,mbna_minmisfit_sr3);
if (MAX(mbna_minmisfit_sr1,mbna_minmisfit_sr2) < 0.5 * overlap_scale
&& MIN(mbna_minmisfit_sr1,mbna_minmisfit_sr2) > 0.0)
fprintf(stderr," USE PICK");
fprintf(stderr,"\n"); */

fprintf(stderr,"Long misfit axis:%.3f Threshold:%.3f",
MAX(mbna_minmisfit_sr1,mbna_minmisfit_sr2),0.5 * overlap_scale);

				if (MAX(mbna_minmisfit_sr1,mbna_minmisfit_sr2) < 0.5 * overlap_scale
					&& MIN(mbna_minmisfit_sr1,mbna_minmisfit_sr2) > 0.0)
					{
					fprintf(stderr," AUTOPICK SUCCEEDED\n");

					/* set offsets to minimum misfit */
					if (do_vertical == MB_YES)
						{
						mbna_offset_x = mbna_minmisfit_x;
						mbna_offset_y = mbna_minmisfit_y;
						mbna_offset_z = mbna_minmisfit_z;
						}
					else
						{
						mbna_offset_x = mbna_minmisfit_xh;
						mbna_offset_y = mbna_minmisfit_yh;
						mbna_offset_z = mbna_minmisfit_zh;
						}

					/* add tie */
    					mbnavadjust_naverr_addtie();

					/* deal with each tie */
					for (j=0;j<crossing->num_ties;j++)
						{
						tie = &(crossing->ties[j]);

						/* calculate sonardepth change rate for swath1 */
						found = MB_NO;
						for (k=0;k<swathraw1->npings;k++)
						    {
						    if (swathraw1->pingraws[k].time_d > tie->snav_1_time_d - 2.0
					    		&& found == MB_NO)
							{
							firstsonardepth1 = swathraw1->pingraws[k].draft;
							firsttime_d1 = swathraw1->pingraws[k].time_d;
							found = MB_YES;
							}
						    if (swathraw1->pingraws[k].time_d < tie->snav_1_time_d + 2.0)
							{
							secondsonardepth1 = swathraw1->pingraws[k].draft;
							secondtime_d1 = swathraw1->pingraws[k].time_d;
							}
						    }
						dsonardepth1 = (secondsonardepth1 - firstsonardepth1)
							/ (secondtime_d1 - firsttime_d1);

						/* calculate sonardepth change rate for swath2 */
						found = MB_NO;
						for (k=0;k<swathraw2->npings;k++)
						    {
						    if (swathraw2->pingraws[k].time_d > tie->snav_2_time_d - 2.0
					    		&& found == MB_NO)
							{
							firstsonardepth2 = swathraw2->pingraws[k].draft;
							firsttime_d2 = swathraw2->pingraws[k].time_d;
							found = MB_YES;
							}
						    if (swathraw2->pingraws[k].time_d < tie->snav_2_time_d + 2.0)
							{
							secondsonardepth2 = swathraw2->pingraws[k].draft;
							secondtime_d2 = swathraw2->pingraws[k].time_d;
							}
						    }
						dsonardepth2 = (secondsonardepth2 - firstsonardepth2)
							/ (secondtime_d2 - firsttime_d2);
/* fprintf(stderr,"mbnavadjust_autopick D crossing:%d tie:%d zoffset:%f   sdrate1:%f %f %f  sdrate2:%f %f %f   inferred time lag:%f\n",
i,j,tie->offset_z_m,
(secondsonardepth1 - firstsonardepth1), (secondtime_d1 - firsttime_d1), dsonardepth1,
(secondsonardepth2 - firstsonardepth2), (secondtime_d2 - firsttime_d2), dsonardepth2,
tie->offset_z_m / (dsonardepth2 - dsonardepth1)); */
						}
					}
				else
					{
					fprintf(stderr," AUTOPICK FAILED\n");
					}

    				/* unload crossing */
  				mbnavadjust_crossing_unload();

fprintf(stderr,"mbna_file_select:%d mbna_survey_select:%d mbna_section_select:%d\n",
mbna_file_select,mbna_survey_select,mbna_section_select);

		    		/* update status periodically */
		    		if (nprocess % 10 == 0)
		    			{
					do_update_status();

					/* update model plot */
					if (project.modelplot == MB_YES)
						{
						/* update model status */
						do_update_modelplot_status();

						/* replot the model */
						mbnavadjust_modelplot_plot();
						}
					}
				}
			}

		/* write updated project */
		mbnavadjust_write_project();

		/* turn off message dialog */
		do_message_off();
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int
mbnavadjust_autosetsvsvertical()
{
	/* local variables */
	char	*function_name = "mbnavadjust_autosetsvsvertical";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct 	mbna_crossing *crossing;
	struct 	mbna_file *file1, *file2;
	struct 	mbna_tie *tie;
	double	overlap_scale;
	int	nprocess;
	int	ntie, nfixed, ncols;
	double	misfit_initial, perturbationsize, perturbationsizeold;
	double	perturbationchange, convergencecriterea;
	double	offset_z_m, block_offset_avg_z;
	double	*x, *xx;
	int	done, iter, nc1, nc2, navg, use, reset_tie;
	int	i, j;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* set up and solve overdetermined least squares problem for a z-offset model with
	 * constant z-offsets within each survey using
	 * the current z-offset ties. Then, loop over all ties to auto-repick the crossings using
	 * the z-offset in the new model (that is, pick the best lateral offset that can be found
	 * using the z-offset in the new model). This should replace the initial ties with ties that
	 * have self-consistent z-offsets between surveys. Using this option only makes sense if the
	 * bathymetry was correctly tide-corrected before import into mbnavadjust.
	 */
   	if (project.open == MB_YES
    		&& project.inversion != MBNA_INVERSION_NONE
		&& project.modelplot == MB_YES)
    		{
		/* calculate the initial misfit */
		ntie = 0;
		misfit_initial = 0.0;
		for (i=0;i<project.num_crossings;i++)
		    {
		    crossing = &project.crossings[i];
		    if (crossing->status == MBNA_CROSSING_STATUS_SET)
		    	{
			ntie += crossing->num_ties;
			for (j=0;j<crossing->num_ties;j++)
				{
				tie = (struct mbna_tie *) &crossing->ties[j];
				misfit_initial += tie->offset_z_m * tie->offset_z_m;
				}
			}
		    }
		misfit_initial = sqrt(misfit_initial) / ntie;
		perturbationsizeold = misfit_initial;

		/* count the number of blocks */
		project.num_blocks = 0;
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    if (i==0 || file->sections[0].continuity == MB_NO)
		    	{
			project.num_blocks++;
			}
		    file->block = project.num_blocks - 1;
		    }

		/* count the number of fixed files */
		nfixed = 0;
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    if (file->status == MBNA_FILE_FIXEDNAV)
		    	nfixed++;
		    }

		/* if only one block just set average offsets to zero */
		if (project.num_blocks <= 1)
		    {
		    block_offset_avg_z = 0.0;
		    for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			file->block_offset_z = 0.0;
			}
		    }

		/* else if more than one block first invert for block offsets  */
		else if (project.num_blocks > 1)
		    {
		    /* allocate space for the inverse problem */
		    ncols = project.num_blocks;
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&x,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&xx,&error);

		    /* initialize x array */
		    for (i=0;i<project.num_blocks;i++)
			{
			x[i] = 0.0;
			}

		    /* construct the inverse problem */
		    sprintf(message,"Solving for survey z offsets...");
		    do_message_update(message);

		    done = MB_NO;
		    iter = 0;
		    while (done == MB_NO)
		    	{
			/* initialize xx array */
			for (i=0;i<project.num_blocks;i++)
			    {
			    xx[i] = 0.0;
			    }

			/* loop over crossings getting set ties */
			ntie = 0;
			for (i=0;i<project.num_crossings;i++)
			    {
			    crossing = &project.crossings[i];

			     /* get block id for first snav point */
			     file1 = &project.files[crossing->file_id_1];
			     nc1 = file1->block;

			     /* get block id for second snav point */
			     file2 = &project.files[crossing->file_id_2];
			     nc2 = file2->block;

			    /* use only set crossings */
			    if (crossing->status == MBNA_CROSSING_STATUS_SET)
			    for (j=0;j<crossing->num_ties;j++)
				{
				/* get tie */
				tie = (struct mbna_tie *) &crossing->ties[j];
				ntie++;

				/* get current offset vector including reduction of block solution */
				if (tie->status != MBNA_TIE_XY)
				    {
				    offset_z_m = tie->offset_z_m - (x[nc2] + xx[nc2] - x[nc1] - xx[nc1]);
				    }
				else
				    {
				    offset_z_m = 0.0;
				    }
/* fprintf(stderr,"icrossing:%d jtie:%d blocks:%d %d offsets: %f %f %f\n",
i,j,nc1,nc2,offsetx,offsety,offset_z_m); */

				/* deal with fixed or unfixed status of sections */
				if ((file1->status == MBNA_FILE_GOODNAV && file2->status == MBNA_FILE_GOODNAV)
					|| (file1->status == MBNA_FILE_POORNAV && file2->status == MBNA_FILE_POORNAV))
				    {
				    xx[nc1] += -mbna_offsetweight * 0.5 * offset_z_m;
				    xx[nc2] +=  mbna_offsetweight * 0.5 * offset_z_m;
				    }
				else if (file1->status == MBNA_FILE_GOODNAV && file2->status == MBNA_FILE_POORNAV)
				    {
				    xx[nc1] += -mbna_offsetweight * 0.005 * offset_z_m;
				    xx[nc2] +=  mbna_offsetweight * 0.995 * offset_z_m;
				    }
				else if (file1->status == MBNA_FILE_POORNAV && file2->status == MBNA_FILE_GOODNAV)
				    {
				    xx[nc1] += -mbna_offsetweight * 0.995 * offset_z_m;
				    xx[nc2] +=  mbna_offsetweight * 0.005 * offset_z_m;
				    }
				else if (file1->status == MBNA_FILE_FIXEDNAV && file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    /*
				    xx[nc1] +=  0.0;
				    xx[nc2] +=  0.0;
				    */
				    }
				else if (file1->status == MBNA_FILE_FIXEDNAV)
				    {
				    if (file2->status == MBNA_FILE_FIXEDXYNAV)
				    	{
				    	/*
					xx[nc1] +=  0.0;
					xx[nc2] +=  0.0;
				    	*/
					xx[3*nc2+2] +=  offset_z_m;
					}
				    else if (file2->status == MBNA_FILE_FIXEDZNAV)
				    	{
				    	/*
					xx[nc1] +=  0.0;
					xx[nc2] +=  0.0;
					*/
					}
				    else
				    	{
				    	/*
					xx[nc1] +=  0.0;
				    	*/
					xx[nc2] +=  mbna_offsetweight * offset_z_m;
					}
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    if (file1->status == MBNA_FILE_FIXEDXYNAV)
				    	{
					xx[nc1] +=  -mbna_offsetweight * offset_z_m;
				    	/*
					xx[nc2] +=  0.0;
				    	*/
					}
				    else if (file1->status == MBNA_FILE_FIXEDZNAV)
				    	{
				    	/*
					xx[nc1] +=  0.0;
					xx[nc2] +=  0.0;
				    	*/
					}
				    else
				    	{
					xx[nc1] +=  -mbna_offsetweight * offset_z_m;
				    	/*
					xx[nc2] +=  0.0;
				    	*/
					}
				    }
				}
			    }

			/* calculate 2-norm of perturbation */
			perturbationsize = 0.0;
			for (i=0;i<ncols;i++)
			    {
			    perturbationsize += xx[i] * xx[i];
			    }
			perturbationsize = sqrt(perturbationsize) / ncols;

			/* apply perturbation */
			for (i=0;i<ncols;i++)
			    {
			    x[i] += xx[i];
			    }

			 /* check for convergence */
			 perturbationchange = perturbationsize - perturbationsizeold;
			 convergencecriterea = fabs(perturbationchange) / misfit_initial;
			 if (convergencecriterea < MBNA_CONVERGENCE || iter > MBNA_INTERATION_MAX)
		    	     done = MB_YES;
fprintf(stderr,"BLOCK INVERT: iter:%d ntie:%d misfit_initial:%f perturbationsize:%g perturbationchange:%g convergencecriterea:%g done:%d\n",
iter,ntie,misfit_initial,perturbationsize,perturbationchange,convergencecriterea,done);

			 if (done == MB_NO)
		             {
			     perturbationsizeold = perturbationsize;
			     iter++;
			     }
			 }

		    /* if there are no fixed blocks contributing to ties,
		    	then get average z-offsets of blocks not flagged as bad
			to provide a static offset to move final model to be more consistent
			with the good blocks than the poorly navigated blocks */
		    block_offset_avg_z = 0.0;
		    navg = 0;
		    if (nfixed == 0)
		    	{
			for (i=0;i<project.num_blocks;i++)
			    {
			    use = MB_YES;
			    for (j=0;j<project.num_files;j++)
				    {
				    file = &project.files[j];
				    if (file->block == i && file->status == MBNA_FILE_POORNAV)
					    use = MB_NO;
				    }
			    if (use == MB_YES)
				    {
				    block_offset_avg_z += x[i];
				    navg++;
				    }
			    }
			if (navg > 0)
		    	    {
			    block_offset_avg_z /= navg;
			    }
			}

		    /* output solution */
		    fprintf(stderr,"\nAverage z-offsets: %f\n",block_offset_avg_z);
		    for (i=0;i<project.num_blocks;i++)
			{
			fprintf(stderr, "Survey block:%d  z-offset: %f  block z-offset:%f\n", i, x[i], x[i] - block_offset_avg_z);
			}

		    /* set block offsets for each file */
		    for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			file->block_offset_z = x[file->block] - block_offset_avg_z;
/* fprintf(stderr,"file:%d block: %d block offsets: %f %f %f\n",
i,file->block,file->block_offset_x,file->block_offset_y,file->block_offset_z);*/
			}

		    /* deallocate arrays */
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&xx,&error);
		    }

		/* loop over over all crossings - reset existing ties using block z-offsets */
		nprocess = 0;
		for (i=0;i<project.num_crossings;i++)
			{
			/* set the crossing */
			crossing = &project.crossings[i];
			file1 = &project.files[crossing->file_id_1];
			file2 = &project.files[crossing->file_id_2];
			offset_z_m = file2->block_offset_z - file1->block_offset_z;

			/* check if any ties exist and are inconsistent with the new survey z-offset model */
			reset_tie = MB_NO;
			for (j=0;j<crossing->num_ties;j++)
				{
				tie = &(crossing->ties[j]);
				if (tie->offset_z_m != offset_z_m)
					reset_tie = MB_YES;
				}

			/* if one or more ties exist and at , load crossing and reset the ties using the new z-offset */
			if (reset_tie == MB_YES)
				{
				mbna_current_crossing = i;
				mbna_file_id_1 = crossing->file_id_1;
				mbna_section_1 = crossing->section_1;
				mbna_file_id_2 = crossing->file_id_2;
				mbna_section_2 = crossing->section_2;
				mbna_current_tie = 0;
				file1 = &project.files[mbna_file_id_1];
				file2 = &project.files[mbna_file_id_2];

				/* set message dialog on */
				sprintf(message,"Loading crossing %d...", mbna_current_crossing);
				fprintf(stderr,"%s: %s\n",function_name,message);
				do_message_update(message);

				/* load crossing */
				mbnavadjust_crossing_load();
				nprocess++;

				/* update status and model plot */
				do_update_status();
				if (project.modelplot == MB_YES)
					{
					do_update_modelplot_status();
					mbnavadjust_modelplot_plot();
					}

				/* delete each tie */
				for (j=0;j<crossing->num_ties;j++)
					{
					mbnavadjust_deletetie(mbna_current_crossing, j, MBNA_CROSSING_STATUS_NONE);
					}

				/* update status and model plot */
				do_update_status();
				if (project.modelplot == MB_YES)
					{
					do_update_modelplot_status();
					mbnavadjust_modelplot_plot();
					}

				/* reset z offset */
				mbna_offset_z = file2->block_offset_z - file1->block_offset_z;

				/* get misfit */
				mbnavadjust_get_misfit();

				/* set offsets to minimum horizontal misfit */
				mbna_offset_x = mbna_minmisfit_xh;
				mbna_offset_y = mbna_minmisfit_yh;
				mbna_offset_z = mbna_minmisfit_zh;
				mbna_misfit_offset_x = mbna_offset_x;
				mbna_misfit_offset_y = mbna_offset_y;
				mbna_misfit_offset_z = mbna_offset_z;
				mbnavadjust_crossing_replot();

				/* get misfit */
				mbnavadjust_get_misfit();

				/* set plot bounds to overlap region */
				mbnavadjust_crossing_overlapbounds(mbna_current_crossing,
							mbna_offset_x, mbna_offset_y,
							&mbna_overlap_lon_min, &mbna_overlap_lon_max,
							&mbna_overlap_lat_min, &mbna_overlap_lat_max);
				mbna_plot_lon_min = mbna_overlap_lon_min;
				mbna_plot_lon_max = mbna_overlap_lon_max;
				mbna_plot_lat_min = mbna_overlap_lat_min;
				mbna_plot_lat_max = mbna_overlap_lat_max;
				overlap_scale = MIN((mbna_overlap_lon_max - mbna_overlap_lon_min) / mbna_mtodeglon,
							(mbna_overlap_lat_max - mbna_overlap_lat_min) / mbna_mtodeglat);

				/* get naverr plot scaling */
				mbnavadjust_naverr_scale();

				/* get misfit */
				mbnavadjust_get_misfit();

				/* check uncertainty estimate for a good pick */
				if (MAX(mbna_minmisfit_sr1,mbna_minmisfit_sr2) < 0.5 * overlap_scale
					&& MIN(mbna_minmisfit_sr1,mbna_minmisfit_sr2) > 0.0)
					{

					/* set offsets to minimum horizontal misfit */
					mbna_offset_x = mbna_minmisfit_xh;
					mbna_offset_y = mbna_minmisfit_yh;
					mbna_offset_z = mbna_minmisfit_zh;

					/* add tie */
					mbnavadjust_naverr_addtie();
					}
				else
					{
					sprintf(message,"Failed to reset Tie Point %d of Crossing %d\n",
							0, mbna_current_crossing);
					if (mbna_verbose == 0)
						fprintf(stderr,"%s",message);
					do_info_add(message, MB_YES);
					}

				/* unload crossing */
				mbnavadjust_crossing_unload();

/*fprintf(stderr,"mbna_file_select:%d mbna_survey_select:%d mbna_section_select:%d\n",
mbna_file_select,mbna_survey_select,mbna_section_select);*/

				/* update status periodically */
				if (nprocess % 10 == 0)
					{
					do_update_status();

					/* update model plot */
					if (project.modelplot == MB_YES)
						{
						/* update model status */
						do_update_modelplot_status();

						/* replot the model */
						mbnavadjust_modelplot_plot();
						}
					}
				}
			}

		/* update status */
		do_update_status();

		/* update model plot */
		if (project.modelplot == MB_YES)
			{
			/* update model status */
			do_update_modelplot_status();

			/* replot the model */
			mbnavadjust_modelplot_plot();
			}
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int
mbnavadjust_zerozoffsets()
{
	/* local variables */
	char	*function_name = "mbnavadjust_zerozoffsets";
	int	status = MB_SUCCESS;
	struct 	mbna_crossing *crossing;
	struct 	mbna_tie *tie;
	int	i, j;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* loop over all crossings */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0)
    		{
		/* set message dialog on */
		sprintf(message,"Zeroing all z offsets...");
		do_message_on(message);
		sprintf(message,"Zeroing all z offsets.\n");
		if (mbna_verbose == 0)
		    fprintf(stderr,"%s",message);
		do_info_add(message,MB_YES);

		/* loop over all crossings */
		for (i=0;i<project.num_crossings;i++)
			{
			/* get structure */
			crossing = &(project.crossings[i]);

			/* deal with each tie */
			for (j=0;j<crossing->num_ties;j++)
				{
				tie = &(crossing->ties[j]);

				/* zero the z offset */
				tie->offset_z_m = 0.0;

				/* set inversion out of date */
				if (project.inversion == MBNA_INVERSION_CURRENT)
					project.inversion = MBNA_INVERSION_OLD;
				}
			}
		/* write updated project */
		mbnavadjust_write_project();

		/* turn off message dialog */
		do_message_off();
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/

int
mbnavadjust_invertnav()
{
	/* local variables */
	char	*function_name = "mbnavadjust_invertnav";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_file *file1;
	struct mbna_file *file2;
	struct mbna_section *section;
	struct mbna_section *section1;
	struct mbna_section *section2;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	nnav;
	int	nsnav;
	int	ndf = 3;
	int	ncols = 0;
	int	ntie = 0;
	int	nfixed = 0;
	int	nglobal = 0;
	int	nmisfit = 0;
	double	*x = NULL;
	double	*xx = NULL;
	double	*xa = NULL;
	int	*nxs = NULL;
	double	*xs = NULL;
	double	*xw = NULL;
	double	misfit, misfit_initial, misfit_ties_initial, misfit_norm_initial, misfit_ties, misfit_norm;
	double	offsetx, offsety, offsetz, offsetr;
	double	projected_offset;
	double	xyweight, zweight;
	double	dtime_d;
	int	done, iter;
	int	nseq, nseqlast;
	int	nchange;
	int	ndx, ndx2;
	int	icrossing, jtie;
	int	ifile, isection, isnav, inav;
	int	nc1, nc2, nc3;
	int	i, j, k;

	double	perturbationsize;
	double	perturbationsizeold;
	double	perturbationchange;
	double	convergencecriterea;
	double	offset_x;
	double	offset_y;
	double	offset_z;
	double	time_d1, time_d2, time_d3;
	double	block_offset_avg_x;
	double	block_offset_avg_y;
	double	block_offset_avg_z;
	int	navg;
	int	use;
	int	ok_to_invert;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* check if it is ok to invert
		- if there is a project
		- enough crossings have been analyzed
		- no problems with offsets and offset uncertainties */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed >= 10
			|| project.num_truecrossings_analyzed == project.num_truecrossings))

    		{
		/* check that all uncertainty magnitudes are nonzero */
		ok_to_invert = MB_YES;
		for (icrossing=0;icrossing<project.num_crossings;icrossing++)
		    {
		    crossing = &project.crossings[icrossing];
		    if (crossing->status == MBNA_CROSSING_STATUS_SET)
		    	{
			for (j=0;j<crossing->num_ties;j++)
				{
				tie = (struct mbna_tie *) &crossing->ties[j];
				if (tie->sigmar1 <= 0.0
					|| tie->sigmar2 <= 0.0
					|| tie->sigmar3 <= 0.0)
					{
					ok_to_invert = MB_NO;
					fprintf(stderr,"PROBLEM WITH TIE: %4d %2d %2.2d:%3.3d:%3.3d:%2.2d %2.2d:%3.3d:%3.3d:%2.2d %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f\n",
						    icrossing, j,
						    project.files[crossing->file_id_1].block,
						    crossing->file_id_1,
						    crossing->section_1,
						    tie->snav_1,
						    project.files[crossing->file_id_2].block,
						    crossing->file_id_2,
						    crossing->section_2,
						    tie->snav_2,
						    tie->offset_x_m,
						    tie->offset_y_m,
						    tie->offset_z_m,
						    tie->sigmar1,
						    tie->sigmar2,
						    tie->sigmar3);
					}
				}
			}
		    }

		/* print out warning */
		if (ok_to_invert == MB_NO)
			{
			fprintf(stderr,"\nThe inversion was not performed because there are one or more zero offset uncertainty values.\n");
			fprintf(stderr,"Please fix the ties with problems noted above before trying again.\n\n");
			}
		}


	/* invert if there is a project and enough crossings have been analyzed */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed >= 10
			|| project.num_truecrossings_analyzed == project.num_truecrossings)
		&& ok_to_invert == MB_YES)

    		{
		fprintf(stderr,"\nInverting for navigation adjustment model...\n");

		/* set message dialog on */
		sprintf(message,"Setting up navigation inversion...");
		do_message_on(message);

		/*----------------------------------------------------------------*/
		/* Step 1 - get block (survey) average offsets                    */
		/*----------------------------------------------------------------*/

		/* figure out the average offsets between connected sets of files
			- invert for x y z offsets for the blocks */

		/* calculate the initial misfit, count number of crossing ties and global ties */
		ntie = 0;
		nglobal = 0;
		misfit_initial = 0.0;
		nmisfit = 0;
		for (icrossing=0;icrossing<project.num_crossings;icrossing++)
		    {
		    crossing = &project.crossings[icrossing];
		    if (crossing->status == MBNA_CROSSING_STATUS_SET)
		    	{
			ntie += crossing->num_ties;
			for (j=0;j<crossing->num_ties;j++)
				{
				tie = (struct mbna_tie *) &crossing->ties[j];
				if (tie->status != MBNA_TIE_Z)
					{
					misfit_initial += tie->offset_x_m * tie->offset_x_m;
					misfit_initial += tie->offset_y_m * tie->offset_y_m;
					nmisfit += 2;
					}
				if (tie->status != MBNA_TIE_XY)
					{
					misfit_initial += tie->offset_z_m * tie->offset_z_m;
					nmisfit++;
					}
				}
			}
		    }
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (isection=0;isection<file->num_sections;isection++)
			{
			section = &file->sections[isection];
			if (section->global_tie_status != MBNA_TIE_NONE)
				{
				nglobal++;
				if (section->global_tie_status != MBNA_TIE_Z)
					{
					misfit_initial += section->global_tie_offset_x_m * section->global_tie_offset_x_m;
					misfit_initial += section->global_tie_offset_y_m * section->global_tie_offset_y_m;
					nmisfit += 2;
					}
				if (section->global_tie_status != MBNA_TIE_XY)
					{
					misfit_initial += section->global_tie_offset_z_m * section->global_tie_offset_z_m;
					nmisfit++;
					}
				}
			}
		    }
		misfit_initial = sqrt(misfit_initial) / nmisfit;
		perturbationsizeold = misfit_initial;

		/* count the number of fixed files */
		nfixed = 0;
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    if (file->status == MBNA_FILE_FIXEDNAV)
		    	nfixed++;
		    }

		/* if only one block just set average offsets to zero */
		if (project.num_blocks <= 1)
		    {
		    block_offset_avg_x = 0.0;
		    block_offset_avg_y = 0.0;
		    block_offset_avg_z = 0.0;
		    for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			file->block_offset_x = 0.0;
			file->block_offset_y = 0.0;
			file->block_offset_z = 0.0;
			}
		    }

		/* else if more than one block first invert for block offsets  */
		else if (project.num_blocks > 1)
		    {
		    /* allocate space for the inverse problem */
		    ncols = ndf * project.num_blocks;
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&x,&error);
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&xx,&error);

		    /* initialize x array */
		    for (i=0;i<ndf*project.num_blocks;i++)
			{
			x[i] = 0.0;
			}

		    /* construct the inverse problem */
		    sprintf(message,"Solving for block offsets...");
		    do_message_update(message);

		    done = MB_NO;
		    iter = 0;
		    while (done == MB_NO)
		    	{
			/* initialize xx array */
			for (i=0;i<ndf*project.num_blocks;i++)
			    {
			    xx[i] = 0.0;
			    }

			/* loop over crossings getting set ties */
			for (icrossing=0;icrossing<project.num_crossings;icrossing++)
			    {
			    crossing = &project.crossings[icrossing];

			     /* get block id for first snav point */
			     file1 = &project.files[crossing->file_id_1];
			     nc1 = file1->block;

			     /* get block id for second snav point */
			     file2 = &project.files[crossing->file_id_2];
			     nc2 = file2->block;

			    /* use only set crossings */
			    if (crossing->status == MBNA_CROSSING_STATUS_SET)
			    for (j=0;j<crossing->num_ties;j++)
				{
				/* get tie */
				tie = (struct mbna_tie *) &crossing->ties[j];

				/* get current offset vector including reduction of block solution */
				if (tie->status != MBNA_TIE_Z)
				    {
				    offsetx = tie->offset_x_m - (x[3*nc2]   + xx[3*nc2]   - x[3*nc1]   - xx[3*nc1]);
				    offsety = tie->offset_y_m - (x[3*nc2+1] + xx[3*nc2+1] - x[3*nc1+1] - xx[3*nc1+1]);
				    }
				else
				    {
				    offsetx = 0.0;
				    offsety = 0.0;
				    }
				if (tie->status != MBNA_TIE_XY)
				    {
				    offsetz = tie->offset_z_m - (x[3*nc2+2] + xx[3*nc2+2] - x[3*nc1+2] - xx[3*nc1+2]);
				    }
				else
				    {
				    offsetz = 0.0;
				    }
/* fprintf(stderr,"icrossing:%d jtie:%d blocks:%d %d offsets: %f %f %f\n",
icrossing,j,nc1,nc2,offsetx,offsety,offsetz); */

				/* deal with fixed or unfixed status of sections */
				if ((file1->status == MBNA_FILE_GOODNAV && file2->status == MBNA_FILE_GOODNAV)
					|| (file1->status == MBNA_FILE_POORNAV && file2->status == MBNA_FILE_POORNAV))
				    {
				    xx[3*nc1]   += -0.5 * offsetx;
				    xx[3*nc1+1] += -0.5 * offsety;
				    xx[3*nc1+2] += -0.5 * offsetz;
				    xx[3*nc2]   +=  0.5 * offsetx;
				    xx[3*nc2+1] +=  0.5 * offsety;
				    xx[3*nc2+2] +=  0.5 * offsetz;
				    }
				else if (file1->status == MBNA_FILE_GOODNAV && file2->status == MBNA_FILE_POORNAV)
				    {
				    xx[3*nc1]   += -0.005 * offsetx;
				    xx[3*nc1+1] += -0.005 * offsety;
				    xx[3*nc1+2] += -0.005 * offsetz;
				    xx[3*nc2]   +=  0.995 * offsetx;
				    xx[3*nc2+1] +=  0.995 * offsety;
				    xx[3*nc2+2] +=  0.995 * offsetz;
				    }
				else if (file1->status == MBNA_FILE_POORNAV && file2->status == MBNA_FILE_GOODNAV)
				    {
				    xx[3*nc1]   += -0.995 * offsetx;
				    xx[3*nc1+1] += -0.995 * offsety;
				    xx[3*nc1+2] += -0.995 * offsetz;
				    xx[3*nc2]   +=  0.005 * offsetx;
				    xx[3*nc2+1] +=  0.005 * offsety;
				    xx[3*nc2+2] +=  0.005 * offsetz;
				    }
				else if (file1->status == MBNA_FILE_FIXEDNAV && file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    /*
				    xx[3*nc1]   +=  0.0;
				    xx[3*nc1+1] +=  0.0;
				    xx[3*nc1+2] +=  0.0;
				    xx[3*nc2]   +=  0.0;
				    xx[3*nc2+1] +=  0.0;
				    xx[3*nc2+2] +=  0.0;
				    */
				    }
				else if (file1->status == MBNA_FILE_FIXEDNAV)
				    {
				    if (file2->status == MBNA_FILE_FIXEDXYNAV)
				    	{
				    	/*
					xx[3*nc1]   +=  0.0;
					xx[3*nc1+1] +=  0.0;
					xx[3*nc1+2] +=  0.0;
					xx[3*nc2]   +=  0.0;
					xx[3*nc2+1] +=  0.0;
				    	*/
					xx[3*nc2+2] +=  offsetz;
					}
				    else if (file2->status == MBNA_FILE_FIXEDZNAV)
				    	{
				    	/*
					xx[3*nc1]   +=  0.0;
					xx[3*nc1+1] +=  0.0;
					xx[3*nc1+2] +=  0.0;
				    	*/
					xx[3*nc2]   +=  offsetx;
					xx[3*nc2+1] +=  offsety;
				    	/*
					xx[3*nc2+2] +=  0.0;
					*/
					}
				    else
				    	{
				    	/*
					xx[3*nc1]   +=  0.0;
					xx[3*nc1+1] +=  0.0;
					xx[3*nc1+2] +=  0.0;
				    	*/
					xx[3*nc2]   +=  offsetx;
					xx[3*nc2+1] +=  offsety;
					xx[3*nc2+2] +=  offsetz;
					}
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    if (file1->status == MBNA_FILE_FIXEDXYNAV)
				    	{
				    	/*
					xx[3*nc1]   +=  0.0;
					xx[3*nc1+1] +=  0.0;
				    	*/
					xx[3*nc1+2] +=  -offsetz;
				    	/*
					xx[3*nc2]   +=  0.0;
					xx[3*nc2+1] +=  0.0;
					xx[3*nc2+2] +=  0.0;
				    	*/
					}
				    else if (file1->status == MBNA_FILE_FIXEDZNAV)
				    	{
					xx[3*nc1]   +=  -offsetx;
					xx[3*nc1+1] +=  -offsety;
				    	/*
					xx[3*nc1+2] +=  0.0;
					xx[3*nc2]   +=  0.0;
					xx[3*nc2+1] +=  0.0;
					xx[3*nc2+2] +=  0.0;
				    	*/
					}
				    else
				    	{
					xx[3*nc1]   +=  -offsetx;
					xx[3*nc1+1] +=  -offsety;
					xx[3*nc1+2] +=  -offsetz;
				    	/*
					xx[3*nc2]   +=  0.0;
					xx[3*nc2+1] +=  0.0;
					xx[3*nc2+2] +=  0.0;
				    	*/
					}
				    }
				}
			    }
			    
			/* loop over all global ties */
			for (ifile=0;ifile<project.num_files;ifile++)
			    {
			    file = &project.files[ifile];
			    for (isection=0;isection<file->num_sections;isection++)
				{
				section = &file->sections[isection];
				if (section->global_tie_status != MBNA_TIE_NONE)
				    {
				    /* get block id for snav point */
				    nc1 = file->block;
    
				    /* get current offset vector including reduction of block solution */
				    if (section->global_tie_status != MBNA_TIE_Z)
					{
					offsetx = section->global_tie_offset_x_m - (x[3*nc1]   + xx[3*nc1]);
					offsety = section->global_tie_offset_y_m - (x[3*nc1+1] + xx[3*nc1+1]);
					
					xx[3*nc1]   += 0.5 * offsetx;
					xx[3*nc1+1] += 0.5 * offsety;
					}
				    if (section->global_tie_status != MBNA_TIE_XY)
					{
					offsetz = section->global_tie_offset_z_m - (x[3*nc1+2] + xx[3*nc1+2]);
					xx[3*nc1+2] += 0.5 * offsetz;
					}

//fprintf(stderr,"STAGE 1 - GLOBAL TIE: %1d %2.2d:%2.2d:%2.2d  x: %f %f %f     xx: %f %f %f     offset: %f %f %f\n",
//section->global_tie_status,ifile,isection,section->global_tie_snav,
//x[3*nc1],x[3*nc1+1],x[3*nc1+2],xx[3*nc1],xx[3*nc1+1],xx[3*nc1+2],
//offsetx,offsety,offsetz);
				    }
				}
			    }

			/* calculate 2-norm of perturbation */
			perturbationsize = 0.0;
			for (i=0;i<ncols;i++)
			    {
			    perturbationsize += xx[i] * xx[i];
			    }
			perturbationsize = sqrt(perturbationsize) / ncols;

			/* apply perturbation */
			for (i=0;i<ncols;i++)
			    {
			    x[i] += xx[i];
			    }

			/* check for convergence */
			perturbationchange = perturbationsize - perturbationsizeold;
			convergencecriterea = fabs(perturbationchange) / misfit_initial;
			if (convergencecriterea < MBNA_CONVERGENCE || iter > MBNA_INTERATION_MAX)
		    	     done = MB_YES;
/* fprintf(stderr,"BLOCK INVERT: iter:%d ntie:%d misfit_initial:%f misfit_ties:%f perturbationsize:%g perturbationchange:%g convergencecriterea:%g done:%d\n",
iter,ntie,misfit_initial,misfit_ties,perturbationsize,perturbationchange,convergencecriterea,done);*/

			if (done == MB_NO)
		             {
			     perturbationsizeold = perturbationsize;
			     iter++;
			     }
			}

		    /* if there are no fixed blocks contributing to ties or global ties,
		    	then get average offsets of blocks not flagged as bad
			to provide a static offset to move final model to be more consistent
			with the good blocks than the poorly navigated blocks */
		    block_offset_avg_x = 0.0;
		    block_offset_avg_y = 0.0;
		    block_offset_avg_z = 0.0;
		    navg = 0;
		    if (nfixed == 0 && nglobal == 0)
		    	{
			for (i=0;i<project.num_blocks;i++)
			    {
			    use = MB_YES;
			    for (j=0;j<project.num_files;j++)
				    {
				    file = &project.files[j];
				    if (file->block == i && file->status == MBNA_FILE_POORNAV)
					    use = MB_NO;
				    }
			    if (use == MB_YES)
				    {
				    block_offset_avg_x += x[3 * i];
				    block_offset_avg_y += x[3 * i + 1];
				    block_offset_avg_z += x[3 * i + 2];
				    navg++;
				    }
			    }
			if (navg > 0)
		    	    {
			    block_offset_avg_x /= navg;
			    block_offset_avg_y /= navg;
			    block_offset_avg_z /= navg;
			    }
			}
/* fprintf(stderr,"Average block offsets: x:%f y:%f z:%f  Used %d of %d blocks\n",
block_offset_avg_x,block_offset_avg_y,block_offset_avg_z,navg,project.num_blocks); */

/* output solution */
/* fprintf(stderr,"\nAverage offsets: %f %f %f\n",block_offset_avg_x,block_offset_avg_y,block_offset_avg_z);
for (i=0;i<ncols/3;i++)
{
fprintf(stderr, "Survey block:%d  offsets: %f %f %f\n",
	    i, x[3*i], x[3*i+1], x[3*i+2]);
} */

		    /* extract results */
		    for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			file->block_offset_x = x[3 * file->block] - block_offset_avg_x;
			file->block_offset_y = x[3 * file->block + 1] - block_offset_avg_y;
			file->block_offset_z = x[3 * file->block + 2] - block_offset_avg_z;
/* fprintf(stderr,"file:%d block: %d block offsets: %f %f %f\n",
i,file->block,file->block_offset_x,file->block_offset_y,file->block_offset_z); */
			}

		    /* deallocate arrays */
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x,&error);
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&xx,&error);
		    }

		/*----------------------------------------------------------------*/
		/* Initialize arrays, solution, perturbation                      */
		/*----------------------------------------------------------------*/

		/* count dimension of solution guess vector */
		nnav = 0;
		nsnav = 0;
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			nsnav += section->num_snav - section->continuity;
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {
			    if (section->snav_num_ties[isnav] > 0
				|| section->global_tie_snav == isnav)
				{
				section->snav_invert_id[isnav] = nnav;
				nnav++;
				}
			    }
			}
		    }

		/* allocate solution vector x, perturbation vector xx, and average solution vector xa */
		ncols = ndf * nnav;
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&x,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&xx,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&xa,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&xs,&error);
		status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols * sizeof(double), (void **)&xw,&error);
		inav = 0;
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			for (isnav=0;isnav<section->num_snav;isnav++)
			    {
			    if (section->snav_num_ties[isnav] > 0
				|| section->global_tie_snav == isnav)
				{
				x[ndf * inav] = 0.0;
				x[ndf * inav + 1] = 0.0;
				x[ndf * inav + 2] = 0.0;
				xx[ndf * inav] = 0.0;
				xx[ndf * inav + 1] = 0.0;
				xx[ndf * inav + 2] = 0.0;
				xa[ndf * inav] = file->block_offset_x - block_offset_avg_x;
				xa[ndf * inav + 1] = file->block_offset_y - block_offset_avg_y;
				xa[ndf * inav + 2] = file->block_offset_z - block_offset_avg_z;
				inav++;
				}
			    }
			}
		    }

		/* calculate initial normalized misfit */
		misfit_ties_initial = 0.0;
		misfit_norm_initial = 0.0;
		nmisfit = 0;
		for (icrossing=0;icrossing<project.num_crossings;icrossing++)
		    {
		    crossing = &project.crossings[icrossing];

		    /* use only set crossings */
		    if (crossing->status == MBNA_CROSSING_STATUS_SET)
		    for (j=0;j<crossing->num_ties;j++)
			{
			/* get tie */
			tie = (struct mbna_tie *) &crossing->ties[j];

			/* get absolute id for first snav point */
			file1 = &project.files[crossing->file_id_1];
			section1 = &file1->sections[crossing->section_1];
			nc1 = section1->snav_invert_id[tie->snav_1];

			/* get absolute id for second snav point */
			file2 = &project.files[crossing->file_id_2];
			section2 = &file2->sections[crossing->section_2];
			nc2 = section2->snav_invert_id[tie->snav_2];
if (nc1 > nsnav - 1 || nc2 > nsnav -1
|| nc1 < 0 || nc2 < 0)
fprintf(stderr, "BAD TIE snav ID: %d %d %d\n", nc1, nc2, nsnav);

			/* get offset after removal of block solution */
			offsetx = tie->offset_x_m - (file2->block_offset_x - file1->block_offset_x);
			offsety = tie->offset_y_m - (file2->block_offset_y - file1->block_offset_y);
			offsetz = tie->offset_z_m - (file2->block_offset_z - file1->block_offset_z);

			if (tie->status != MBNA_TIE_Z)
			    {
			    /* get long axis misfit */
			    misfit = (offsetx * tie->sigmax1[0]
					    + offsety * tie->sigmax1[1]
					    + offsetz * tie->sigmax1[2]);
			    misfit_ties_initial += misfit * misfit;
			    misfit_norm_initial += misfit * misfit / tie->sigmar1 / tie->sigmar1;
			    nmisfit++;

			    /* get horizontal axis misfit */
			    misfit = (offsetx * tie->sigmax2[0]
					    + offsety * tie->sigmax2[1]
					    + offsetz * tie->sigmax2[2]);
			    misfit_ties_initial += misfit * misfit;
			    misfit_norm_initial += misfit * misfit / tie->sigmar2 / tie->sigmar2;
			    nmisfit++;
			    }

			if (tie->status != MBNA_TIE_XY)
			    {
			    /* get semi-vertical axis misfit */
			    misfit = (offsetx * tie->sigmax3[0]
					    + offsety * tie->sigmax3[1]
					    + offsetz * tie->sigmax3[2]);
			    misfit_ties_initial += misfit * misfit;
			    misfit_norm_initial += misfit * misfit / tie->sigmar3 / tie->sigmar3;
			    nmisfit++;
			    }
			}
		    }
		for (ifile=0;ifile<project.num_files;ifile++)
		    {
		    /* get file */
		    file = &project.files[ifile];
		    for (isection=0;isection<file->num_sections;isection++)
			{
			/* get section */
			section = &file->sections[isection];
			if (section->global_tie_status != MBNA_TIE_NONE)
			    {
			    /* get absolute id for snav point */
			    nc1 = section->snav_invert_id[section->global_tie_snav];
if (nc1 > nsnav - 1 || nc1 < 0)
fprintf(stderr, "BAD GLOBAL TIE snav ID: %d %d\n", nc1, nsnav);

			    /* get offset after removal of block solution */
			    offsetx = section->global_tie_offset_x_m - file->block_offset_x;
			    offsety = section->global_tie_offset_y_m - file->block_offset_y;
			    offsetz = section->global_tie_offset_z_m - file->block_offset_z;
	    
			    if (section->global_tie_status != MBNA_TIE_Z)
				{
				/* get x axis misfit */
				misfit = offsetx;
				misfit_ties_initial += misfit * misfit;
				misfit_norm_initial += misfit * misfit / section->global_tie_xsigma / section->global_tie_xsigma;
				nmisfit++;

				/* get y axis misfit */
				misfit = offsety;
				misfit_ties_initial += misfit * misfit;
				misfit_norm_initial += misfit * misfit / section->global_tie_ysigma / section->global_tie_ysigma;
				nmisfit++;
				}

			    if (section->global_tie_status != MBNA_TIE_XY)
				{
				/* get z axis misfit */
				misfit = offsetz;
				misfit_ties_initial += misfit * misfit;
				misfit_norm_initial += misfit * misfit / section->global_tie_zsigma / section->global_tie_zsigma;
				nmisfit++;
				}
			    }
			}
		    }
		misfit_ties_initial = sqrt(misfit_ties_initial) / nmisfit;
		misfit_norm_initial = sqrt(misfit_norm_initial) / nmisfit;

		/*----------------------------------------------------------------*/
		/* Step 2 - construct "average" model satisfying ties with        */
		/*     fixed data (if needed) - this gets pulled out of the       */
		/*     data along with the average block offsets.                 */
		/*----------------------------------------------------------------*/
		if (nfixed > 0 || nglobal > 0)
		    {
		    /* set message dialog on */
		    sprintf(message,"Getting average offsets relative to fixed data...");
		    do_message_update(message);

		    /* allocate some extra arrays */
		    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(int), (void **)&nxs,&error);
		    for (inav=0;inav<nnav;inav++)
		    	{
			nxs[inav] = 0;
			xs[ndf * inav] = 0.0;
			xs[ndf * inav + 1] = 0.0;
			xs[ndf * inav + 2] = 0.0;
			xw[ndf * inav] = 0.0;
			xw[ndf * inav + 1] = 0.0;
			xw[ndf * inav + 2] = 0.0;
			}

		    /* loop over all ties, working only with ties including fixed sections */
		    for (icrossing=0;icrossing<project.num_crossings;icrossing++)
			{
			crossing = &project.crossings[icrossing];

			/* use only set crossings */
			if (crossing->status == MBNA_CROSSING_STATUS_SET)
			for (jtie=0;jtie<crossing->num_ties;jtie++)
			    {
			    /* get tie */
			    tie = (struct mbna_tie *) &crossing->ties[jtie];

			    /* get absolute id for first snav point */
			    file1 = &project.files[crossing->file_id_1];
			    section = &file1->sections[crossing->section_1];
			    nc1 = section->snav_invert_id[tie->snav_1];

			    /* get absolute id for second snav point */
			    file2 = &project.files[crossing->file_id_2];
			    section = &file2->sections[crossing->section_2];
			    nc2 = section->snav_invert_id[tie->snav_2];

			    /* only work with ties in which one section is fixed */
			    if ((file1->status == MBNA_FILE_FIXEDNAV && file2->status != MBNA_FILE_FIXEDNAV)
			    	|| (file1->status != MBNA_FILE_FIXEDNAV && file2->status == MBNA_FILE_FIXEDNAV))
				{
			        /* get current offset vector including reduction of block solution */
				if (tie->status != MBNA_TIE_Z)
				    {
				    offsetx = tie->offset_x_m - (xa[3*nc2] - xa[3*nc1]);
				    offsety = tie->offset_y_m - (xa[3*nc2+1] - xa[3*nc1+1]);
				    }
				else
				    {
				    offsetx = 0.0;
				    offsety = 0.0;
				    }
				if (tie->status != MBNA_TIE_XY)
				    {
				    offsetz = tie->offset_z_m - (xa[3*nc2+2] - xa[3*nc1+2]);
				    }
				else
				    {
				    offsetz = 0.0;
				    }
/* fprintf(stderr,"STAGE 2 START: icrossing:%d jtie:%d nc1:%d %d nc2:%d %d offsets: %f %f %f\n",
icrossing,jtie,nc1,file1->status,nc2,file2->status,offsetx,offsety,offsetz);*/

				/* figure out how far each tied nav point is from the unfixed point in the current tie */
		    		for (inav=0;inav<nnav;inav++)
				    nxs[inav] = 0;

				/* first set the unfixed affect nav point */
				if (file1->status == MBNA_FILE_FIXEDNAV)
				    {
				    nxs[nc2] = 1;
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    nxs[nc1] = 1;
				    offsetx *= -1.0;
				    offsety *= -1.0;
				    offsetz *= -1.0;
				    }

				/* now loop over all nav points repeatedly */
				done = MB_NO;
				while (done == MB_NO)
				    {
				    nchange = 0;

				    /* run forward through the data */
				    inav = 0;
				    nseq = 0;
			    	    nseqlast = 0;
				    for (i=0;i<project.num_files;i++)
					{
					file = &project.files[i];
					for (j=0;j<file->num_sections;j++)
					    {
					    section = &file->sections[j];
					    if (section->continuity == MB_NO)
			    	    		nseq = 0;
					    for (isnav=0;isnav<section->num_snav;isnav++)
						{
						if (section->snav_num_ties[isnav] > 0)
						    {
/* fprintf(stderr,"i:%d j:%d isnav:%d inav:%d nseqqqlast:%d nseq:%d file->status:%d nxs:%d %d",
i,j,isnav,inav,nseqlast,nseq,file->status,nxs[inav-1],nxs[inav]); */
						    if (inav > 0 && nseq > 0 && file->status != MBNA_FILE_FIXEDNAV
						    	&& nxs[inav-1] > 0 && (nxs[inav-1] < nxs[inav] - 1 || nxs[inav] == 0))
						    	{
							nxs[inav] = nxs[inav-1] + 1;
							nchange++;
/* fprintf(stderr," CHANGE nxs:%d %d",nxs[inav-1],nxs[inav]); */
							}
/* fprintf(stderr,"\n"); */

						    /* increment sequence counters */
						    nseqlast = nseq;
						    nseq++;
						    inav++;
						    }
						}
					    }
					}

/* fprintf(stderr,"\n"); */
				    /* run backward through the data */
			    	    nseq = 0;
			    	    nseqlast = 0;
				    for (i=project.num_files-1;i>=0;i--)
					{
					file = &project.files[i];
					for (j=file->num_sections-1;j>=0;j--)
					    {
					    section = &file->sections[j];
					    for (isnav=section->num_snav-1;isnav>=0;isnav--)
						{
						if (section->snav_num_ties[isnav] > 0)
						    {
						    inav--;
/* fprintf(stderr,"i:%d j:%d isnav:%d inav:%d nseqqqlast:%d nseq:%d file->status:%d nxs:%d %d",
i,j,isnav,inav,nseqlast,nseq,file->status,nxs[inav],nxs[inav+1]); */
						    if (inav >= 0 && nseqlast > 0 && file->status != MBNA_FILE_FIXEDNAV
						    	&& nxs[inav+1] > 0 && (nxs[inav+1] < nxs[inav] - 1 || nxs[inav] == 0))
						    	{
							nxs[inav] = nxs[inav+1] + 1;
							nchange++;
/* fprintf(stderr," CHANGE nxs:%d %d",nxs[inav],nxs[inav+1]); */
							}
/* fprintf(stderr,"\n"); */

						    /* increment sequence counters */
						    nseqlast = nseq;
						    nseq++;
						    }
						}
					    if (section->continuity == MB_NO)
			    	    		nseq = 0;
					    }
					}

				    /* run through ties */
				    for (i=0;i<project.num_crossings;i++)
					{
					/* use only set crossings */
					if (project.crossings[i].status == MBNA_CROSSING_STATUS_SET)
					for (j=0;j<project.crossings[i].num_ties;j++)
					    {
					    /* get tie */
					    tie = (struct mbna_tie *) &project.crossings[i].ties[j];

					    /* get absolute id for first snav point */
					    file1 = &project.files[project.crossings[i].file_id_1];
					    section = &file1->sections[project.crossings[i].section_1];
					    nc1 = section->snav_invert_id[tie->snav_1];

					    /* get absolute id for second snav point */
					    file2 = &project.files[project.crossings[i].file_id_2];
					    section = &file2->sections[project.crossings[i].section_2];
					    nc2 = section->snav_invert_id[tie->snav_2];

					    /* check for nav points needing closeness setting */
					    if (nxs[nc1] > 0 && nxs[nc2] == 0)
					    	{
						if (file2->status != MBNA_FILE_FIXEDNAV)
						    {
						    nxs[nc2] = nxs[nc1] + 1;
						    nchange++;
						    }
						}
					    else if (nxs[nc1] > 0 && nxs[nc2] > nxs[nc1] + 1)
					    	{
						if (file2->status != MBNA_FILE_FIXEDNAV)
						    {
						    nxs[nc2] = nxs[nc1] + 1;
						    nchange++;
						    }
						}
					    else if (nxs[nc2] > 0 && nxs[nc1] == 0)
					    	{
						if (file1->status != MBNA_FILE_FIXEDNAV)
						    {
						    nxs[nc1] = nxs[nc2] + 1;
						    nchange++;
						    }
						}
					    else if (nxs[nc2] > 0 && nxs[nc1] > nxs[nc2] + 1)
					    	{
						if (file1->status != MBNA_FILE_FIXEDNAV)
						    {
						    nxs[nc1] = nxs[nc2] + 1;
						    nchange++;
						    }
						}
					    }
					}

				    /* check for done */
			    	    if (nchange == 0)
				    	done = MB_YES;
/* fprintf(stderr,"icrossing:%d nchange:%d done:%d\n",icrossing,nchange,done); */
				    }
/* fprintf(stderr,"\n");*/

				/* now loop over the data adding the offset with weighting inversely set by the
					"distance" from each nav point to the offset point */
				for (inav=0;inav<nnav;inav++)
				    {
/* fprintf(stderr,"inav:%d nxs:%d\n",inav,nxs[inav]); */
				    if (nxs[inav] > 0)
				    	{
					xs[ndf * inav]     += offsetx / (nxs[inav] * nxs[inav]);
					xs[ndf * inav + 1] += offsety / (nxs[inav] * nxs[inav]);
					xs[ndf * inav + 2] += offsetz / (nxs[inav] * nxs[inav]);
					xw[ndf * inav]     += 1.0 / (nxs[inav] * nxs[inav]);
					xw[ndf * inav + 1] += 1.0 / (nxs[inav] * nxs[inav]);
					xw[ndf * inav + 2] += 1.0 / (nxs[inav] * nxs[inav]);
					}
				    }
				}
			    }
			}
			
		    /* loop over all global ties */
		    for (ifile=0;ifile<project.num_files;ifile++)
			{
			file = &project.files[ifile];
			for (isection=0;isection<file->num_sections;isection++)
			    {
			    section = &file->sections[isection];
			    if (section->global_tie_status != MBNA_TIE_NONE)
				{
				/* get absolute id for snav point */
				nc1 = section->snav_invert_id[section->global_tie_snav];

			        /* get current offset vector including reduction of block solution */
				offsetx = section->global_tie_offset_x_m - xa[3*nc1];
				offsety = section->global_tie_offset_y_m - xa[3*nc1+1];
				offsetz = section->global_tie_offset_z_m - xa[3*nc1+2];
/*fprintf(stderr,"STAGE 2 - GLOBAL TIE: %1d %2.2d:%2.2d:%2.2d  tie: %f %f %f  model offset: %f %f %f\n",
section->global_tie_status,ifile,isection,section->global_tie_snav,
section->global_tie_offset_x_m,section->global_tie_offset_y_m,section->global_tie_offset_z_m,
offsetx,offsety,offsetz); */

				/* figure out how far each tied nav point is from the global tie point */
		    		for (inav=0;inav<nnav;inav++)
				    nxs[inav] = 0;

				/* first set the global tied nav point */
				nxs[nc1] = 1;

				/* now loop over all nav points repeatedly */
				done = MB_NO;
				while (done == MB_NO)
				    {
				    nchange = 0;

				    /* run forward through the data */
				    inav = 0;
				    nseq = 0;
			    	    nseqlast = 0;
				    for (i=0;i<project.num_files;i++)
					{
					file1 = &project.files[i];
					for (j=0;j<file1->num_sections;j++)
					    {
					    section1 = &file1->sections[j];
					    if (section1->continuity == MB_NO)
			    	    		nseq = 0;
					    for (isnav=0;isnav<section1->num_snav;isnav++)
						{
						if (section1->snav_num_ties[isnav] > 0)
						    {
						    if (inav > 0 && nseq > 0 && file1->status != MBNA_FILE_FIXEDNAV
						    	&& nxs[inav-1] > 0 && (nxs[inav-1] < nxs[inav] - 1 || nxs[inav] == 0))
						    	{
							nxs[inav] = nxs[inav-1] + 1;
							nchange++;
							}

						    /* increment sequence counters */
						    nseqlast = nseq;
						    nseq++;
						    inav++;
						    }
						}
					    }
					}

				    /* run backward through the data */
			    	    nseq = 0;
			    	    nseqlast = 0;
				    for (i=project.num_files-1;i>=0;i--)
					{
					file1 = &project.files[i];
					for (j=file1->num_sections-1;j>=0;j--)
					    {
					    section1 = &file1->sections[j];
					    for (isnav=section1->num_snav-1;isnav>=0;isnav--)
						{
						if (section1->snav_num_ties[isnav] > 0)
						    {
						    inav--;
						    if (inav >= 0 && nseqlast > 0 && file1->status != MBNA_FILE_FIXEDNAV
						    	&& nxs[inav+1] > 0 && (nxs[inav+1] < nxs[inav] - 1 || nxs[inav] == 0))
						    	{
							nxs[inav] = nxs[inav+1] + 1;
							nchange++;
							}

						    /* increment sequence counters */
						    nseqlast = nseq;
						    nseq++;
						    }
						}
					    if (section1->continuity == MB_NO)
			    	    		nseq = 0;
					    }
					}

				    /* run through ties */
				    for (i=0;i<project.num_crossings;i++)
					{
					/* use only set crossings */
					if (project.crossings[i].status == MBNA_CROSSING_STATUS_SET)
					for (j=0;j<project.crossings[i].num_ties;j++)
					    {
					    /* get tie */
					    tie = (struct mbna_tie *) &project.crossings[i].ties[j];

					    /* get absolute id for first snav point */
					    file1 = &project.files[project.crossings[i].file_id_1];
					    section1 = &file1->sections[project.crossings[i].section_1];
					    nc1 = section1->snav_invert_id[tie->snav_1];

					    /* get absolute id for second snav point */
					    file2 = &project.files[project.crossings[i].file_id_2];
					    section2 = &file2->sections[project.crossings[i].section_2];
					    nc2 = section2->snav_invert_id[tie->snav_2];

					    /* check for nav points needing closeness setting */
					    if (nxs[nc1] > 0 && nxs[nc2] == 0)
					    	{
						if (file2->status != MBNA_FILE_FIXEDNAV)
						    {
						    nxs[nc2] = nxs[nc1] + 1;
						    nchange++;
						    }
						}
					    else if (nxs[nc1] > 0 && nxs[nc2] > nxs[nc1] + 1)
					    	{
						if (file2->status != MBNA_FILE_FIXEDNAV)
						    {
						    nxs[nc2] = nxs[nc1] + 1;
						    nchange++;
						    }
						}
					    else if (nxs[nc2] > 0 && nxs[nc1] == 0)
					    	{
						if (file1->status != MBNA_FILE_FIXEDNAV)
						    {
						    nxs[nc1] = nxs[nc2] + 1;
						    nchange++;
						    }
						}
					    else if (nxs[nc2] > 0 && nxs[nc1] > nxs[nc2] + 1)
					    	{
						if (file1->status != MBNA_FILE_FIXEDNAV)
						    {
						    nxs[nc1] = nxs[nc2] + 1;
						    nchange++;
						    }
						}
					    }
					}

				    /* check for done */
			    	    if (nchange == 0)
				    	done = MB_YES;
				    }

				/* now loop over the data adding the offset with weighting inversely set by the
					"distance" from each nav point to the offset point */
				for (inav=0;inav<nnav;inav++)
				    {
				    if (nxs[inav] > 0)
				    	{
					if (section->global_tie_status != MBNA_TIE_Z)
					    {
					    xs[ndf * inav]     += offsetx / (nxs[inav] * nxs[inav]);
					    xs[ndf * inav + 1] += offsety / (nxs[inav] * nxs[inav]);
					    xw[ndf * inav]     += 1.0 / (nxs[inav] * nxs[inav]);
					    xw[ndf * inav + 1] += 1.0 / (nxs[inav] * nxs[inav]);
					    }
					if (section->global_tie_status != MBNA_TIE_XY)
					    {
					    xs[ndf * inav + 2] += offsetz / (nxs[inav] * nxs[inav]);
					    xw[ndf * inav + 2] += 1.0 / (nxs[inav] * nxs[inav]);
					    }
					}
				    }
				}
			    }
			}

		    /* construct average offset model */
		    for (inav=0;inav<nnav;inav++)
			{
			if (xw[ndf * inav] > 0.0)
			    xa[ndf * inav] += xs[ndf * inav] / xw[ndf * inav];
			if (xw[ndf * inav + 1] > 0.0)
			    xa[ndf * inav + 1] += xs[ndf * inav + 1] / xw[ndf * inav + 1];
			if (xw[ndf * inav + 2] > 0.0)
			    xa[ndf * inav + 2] += xs[ndf * inav + 2] / xw[ndf * inav + 2];
//fprintf(stderr,"XA CALC:  xs: %f %f %f  xw: %f %f %f  xa: %f %f %f\n",
//xs[ndf*inav],xs[ndf*inav+1],xs[ndf*inav+2],xw[ndf*inav],xw[ndf*inav+1],xw[ndf*inav+2],xa[ndf*inav],xa[ndf*inav+1],xa[ndf*inav+2]);
			}

		    /* save solution */
		    k = 0;
		    for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			for (j=0;j<file->num_sections;j++)
			    {
			    section = &file->sections[j];
			    for (isnav=0;isnav<section->num_snav;isnav++)
				{
				if (section->snav_num_ties[isnav] > 0
				    || section->global_tie_snav == isnav)
				    {
				    k = section->snav_invert_id[isnav];
				    section->snav_lon_offset[isnav] = (x[3*k] + xa[3*k]) * mbna_mtodeglon;
/* fprintf(stderr,"i:%d j:%d isnav:%d k:%d x[3*k]:%f xa[3*k]:%f mbna_mtodeglon:%f section->snav_lon_offset[isnav]:%f\n",
i,j,isnav,k,x[3*k],xa[3*k],mbna_mtodeglon,section->snav_lon_offset[isnav]); */
				    section->snav_lat_offset[isnav] = (x[3*k+1] + xa[3*k+1]) * mbna_mtodeglat;
/* fprintf(stderr,"i:%d j:%d isnav:%d k:%d x[3*k+1]:%f xa[3*k+1]:%f mbna_mtodeglat:%f section->snav_lat_offset[isnav]:%f\n",
i,j,isnav,k,x[3*k+1],xa[3*k+1],mbna_mtodeglat,section->snav_lat_offset[isnav]); */
				    section->snav_z_offset[isnav] = (x[3*k+2] + xa[3*k+2]);
/* fprintf(stderr,"i:%d j:%d isnav:%d k:%d x[3*k+2]:%f xa[3*k+2]:%f section->snav_z_offset[isnav]:%f\n\n",
i,j,isnav,k,x[3*k+2],xa[3*k+2],section->snav_z_offset[isnav]); */
				    }
				}
			    }
			}

		    /* interpolate the solution */
		    mbnavadjust_interpolatesolution();

		    /* save interpolated solution */
		    k = 0;
		    for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			for (j=0;j<file->num_sections;j++)
			    {
			    section = &file->sections[j];
			    for (isnav=0;isnav<section->num_snav;isnav++)
				{
				section->snav_lon_offset[isnav] = section->snav_lon_offset_int[isnav];
				section->snav_lat_offset[isnav] = section->snav_lat_offset_int[isnav];
				section->snav_z_offset[isnav] = section->snav_z_offset_int[isnav];
				}
			    }
			}

		    /* update model plot */
		    if (project.modelplot == MB_YES && iter % 25 == 0)
		    	mbnavadjust_modelplot_plot();

		    /* output goodness of fit for ties including fixed sections */
		    for (icrossing=0;icrossing<project.num_crossings;icrossing++)
			{
			crossing = &project.crossings[icrossing];

			/* use only set crossings */
			if (crossing->status == MBNA_CROSSING_STATUS_SET)
			for (jtie=0;jtie<crossing->num_ties;jtie++)
			    {
			    /* get tie */
			    tie = (struct mbna_tie *) &crossing->ties[jtie];

			    /* get absolute id for first snav point */
			    file1 = &project.files[crossing->file_id_1];
			    section = &file1->sections[crossing->section_1];
			    nc1 = section->snav_invert_id[tie->snav_1];

			    /* get absolute id for second snav point */
			    file2 = &project.files[crossing->file_id_2];
			    section = &file2->sections[crossing->section_2];
			    nc2 = section->snav_invert_id[tie->snav_2];

			    /* only work with ties in which one section is fixed */
			    if ((file1->status == MBNA_FILE_FIXEDNAV && file2->status != MBNA_FILE_FIXEDNAV)
			    	|| (file1->status != MBNA_FILE_FIXEDNAV && file2->status == MBNA_FILE_FIXEDNAV))
				{
			        /* get current offset vector including reduction of block solution */
				offsetx = tie->offset_x_m - (xa[3*nc2] - xa[3*nc1]);
				offsety = tie->offset_y_m - (xa[3*nc2+1] - xa[3*nc1+1]);
				offsetz = tie->offset_z_m - (xa[3*nc2+2] - xa[3*nc1+2]);
//fprintf(stderr,"STAGE 2 RESULT - CROSSING TIE: icrossing:%d jtie:%d nc1:%d %d nc2:%d %d offsets: %f %f %f\n",
//icrossing,jtie,nc1,file1->status,nc2,file2->status,offsetx,offsety,offsetz);
				}
			    }
			}

		    /* output goodness of fit for global ties */
		    for (ifile=0;ifile<project.num_files;ifile++)
			{
			file = &project.files[ifile];
			for (isection=0;isection<file->num_sections;isection++)
			    {
			    /* get section */
			    section = (struct mbna_section *) &file->sections[isection];
			    
			    /* only work with set global ties */
			    if (section->global_tie_status != MBNA_TIE_NONE)
				{
				/* get absolute id for first snav point */
				nc1 = section->snav_invert_id[section->global_tie_snav];

//fprintf(stderr,"STAGE 2 RESULT - GLOBAL TIE: %2.2d:%2.2d:%2.2d  tie: \n",
//ifile,isection,section->global_tie_snav);
//if (section->global_tie_status != MBNA_TIE_Z)
//fprintf(stderr,"x: %f y:%f ",section->global_tie_offset_x_m,section->global_tie_offset_y_m);
//if (section->global_tie_status != MBNA_TIE_XY)
//fprintf(stderr,"z:%f ",section->global_tie_offset_z_m);
//fprintf(stderr,"  model offset: ");

			        /* get current offset vector including reduction of block solution */
				if (section->global_tie_status != MBNA_TIE_Z)
				    {
				    offsetx = section->global_tie_offset_x_m - xa[3*nc1];
				    offsety = section->global_tie_offset_y_m - xa[3*nc1+1];
//fprintf(stderr,"x: %f y:%f ",offsetx,offsety);
				    }
				if (section->global_tie_status != MBNA_TIE_XY)
				    {
				    offsetz = section->global_tie_offset_z_m - xa[3*nc1+2];
//fprintf(stderr,"z:%f ",offsetz);	
				    }
				}
			    }
			}

		    /* deallocate arrays */
		    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nxs,&error);
		    }


		/*----------------------------------------------------------------*/
		/* Step 3 - invert for model satisfying all nav ties              */
		/*----------------------------------------------------------------*/

		/* loop until convergence */
		done = MB_NO;
		iter = 0;
		perturbationsizeold = misfit_ties_initial;
		mbna_smoothweight = pow(10.0, project.smoothing) * mbna_offsetweight;
		while (done == MB_NO)
		    {
		    /* zero smoothed adjustment vector solution */
		    for (i=0;i<ncols;i++)
			{
			xx[i] = 0.0;
			xs[i] = 0.0;
			xw[i] = 0.0;
			}

		    /* loop over each crossing, applying offsets evenly to both points */
		    for (icrossing=0;icrossing<project.num_crossings;icrossing++)
			{
			crossing = &project.crossings[icrossing];

			/* use only set crossings */
			if (crossing->status == MBNA_CROSSING_STATUS_SET)
			for (j=0;j<crossing->num_ties;j++)
			    {
			    /* get tie */
			    tie = (struct mbna_tie *) &crossing->ties[j];

			    /* get absolute id for first snav point */
			    file1 = &project.files[crossing->file_id_1];
			    section1 = &file1->sections[crossing->section_1];
			    nc1 = section1->snav_invert_id[tie->snav_1];

			    /* get absolute id for second snav point */
			    file2 = &project.files[crossing->file_id_2];
			    section2 = &file2->sections[crossing->section_2];
			    nc2 = section2->snav_invert_id[tie->snav_2];
if (file1->sections[crossing->section_1].snav_time_d[tie->snav_1] == file2->sections[crossing->section_2].snav_time_d[tie->snav_2])
fprintf(stderr,"ZERO TIME BETWEEN TIED POINTS!!  file:section:snav - %d:%d:%d   %d:%d:%d  DIFF:%f\n",
	crossing->file_id_1,crossing->section_1,tie->snav_1,
	crossing->file_id_2,crossing->section_2,tie->snav_2,
	(file1->sections[crossing->section_1].snav_time_d[tie->snav_1]
	 - file2->sections[crossing->section_2].snav_time_d[tie->snav_2]));

			    /* get current offset vector including reduction of block solution */
			    if (tie->status != MBNA_TIE_Z)
			        {
				offsetx = tie->offset_x_m - (xa[3*nc2] - xa[3*nc1]) - (x[3*nc2] - x[3*nc1]);
				offsety = tie->offset_y_m - (xa[3*nc2+1] - xa[3*nc1+1]) - (x[3*nc2+1] - x[3*nc1+1]);
				}
			    else
			        {
				offsetx = 0.0;
				offsety = 0.0;
				}
			    if (tie->status != MBNA_TIE_XY)
			        {
				offsetz = tie->offset_z_m - (xa[3*nc2+2] - xa[3*nc1+2]) - (x[3*nc2+2] - x[3*nc1+2]);
				}
			    else
			        {
				offsetz = 0.0;
				}
			    offsetr = sqrt(offsetx * offsetx + offsety * offsety + offsetz * offsetz);

			    /* deal with each component of the error ellipse
			    	- project offset vector onto each component by dot-product
				- weight inversely by size of error for that component */

			    /* deal with long axis */
			    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
				projected_offset = offsetx * tie->sigmax1[0]
						+ offsety * tie->sigmax1[1]
						+ offsetz * tie->sigmax1[2];
			    else
				projected_offset = offsetx * tie->sigmax1[0]
						+ offsety * tie->sigmax1[1];
			    if (fabs(tie->sigmar1) > 0.0)
			    	{
				xyweight = sqrt(mbna_offsetweight / tie->sigmar1);
				zweight = sqrt(mbna_offsetweight / mbna_zweightfactor / tie->sigmar1);
				}
			    else
			    	{
				xyweight = 0.0;
			    	zweight = 0.0;
				}
//zweight = 0.0;

			    /* deal with fixed, good, or poor status of sections */
			    if (file1->status == file2->status)
			    	{
				if (file1->status == MBNA_FILE_GOODNAV || file1->status == MBNA_FILE_POORNAV)
				    {
				    xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.5 * zweight * projected_offset * tie->sigmax1[2];
				    xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.5 * zweight * projected_offset * tie->sigmax1[2];				    
				    }
				else if (file1->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.5 * zweight * projected_offset * tie->sigmax1[2];
				    /*
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.5 * zweight * projected_offset * tie->sigmax1[2];
				    }
				else if (file1->status == MBNA_FILE_FIXEDZNAV)
				    {
				    xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax1[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    */
				    xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax1[1];
				    /*
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_GOODNAV)
			    	{
				if (file2->status == MBNA_FILE_POORNAV)
				    {
				    xs[3*nc1]   += -0.005 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc1+1] += -0.005 * xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.005 * zweight * projected_offset * tie->sigmax1[2];
				    xs[3*nc2]   +=  0.995 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc2+1] +=  0.995 * xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.995 * zweight * projected_offset * tie->sigmax1[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    xs[3*nc1]   += -xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc1+1] += -xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax1[2];
				    /*
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				else if (file2->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    xs[3*nc1]   += -xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc1+1] += -xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.5 * zweight * projected_offset * tie->sigmax1[2];
				    /*
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.5 * zweight * projected_offset * tie->sigmax1[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDZNAV)
				    {
				    xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax1[2];
				    xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax1[1];
				    /*
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_POORNAV)
			    	{
				if (file2->status == MBNA_FILE_GOODNAV)
				    {
				    xs[3*nc1]   += -0.995 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc1+1] += -0.995 * xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.995 * zweight * projected_offset * tie->sigmax1[2];
				    xs[3*nc2]   +=  0.005 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc2+1] +=  0.005 * xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.005 * zweight * projected_offset * tie->sigmax1[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    xs[3*nc1]   += -xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc1+1] += -xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax1[2];
				    /*
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				else if (file2->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    xs[3*nc1]   += -xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc1+1] += -xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.995 * zweight * projected_offset * tie->sigmax1[2];
				    /*
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.005 * zweight * projected_offset * tie->sigmax1[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDZNAV)
				    {
				    xs[3*nc1]   += -0.995 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc1+1] += -0.995 * xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax1[2];
				    xs[3*nc2]   +=  0.005 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc2+1] +=  0.005 * xyweight * projected_offset * tie->sigmax1[1];
				    /*
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_FIXEDNAV)
			    	{
				if (file2->status == MBNA_FILE_GOODNAV || file2->status == MBNA_FILE_POORNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    */
				    xs[3*nc2]   +=  xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc2+1] +=  xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax1[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax1[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDZNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    */
				    xs[3*nc2]   +=  xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc2+1] +=  xyweight * projected_offset * tie->sigmax1[1];
				    /*
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_FIXEDXYNAV)
			    	{
				if (file2->status == MBNA_FILE_GOODNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.5 * zweight * projected_offset * tie->sigmax1[2];
				    xs[3*nc2]   +=  xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc2+1] +=  xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.5 * zweight * projected_offset * tie->sigmax1[2];
				    }
				else if (file2->status == MBNA_FILE_POORNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.005 * zweight * projected_offset * tie->sigmax1[2];
				    xs[3*nc2]   +=  xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc2+1] +=  xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.995 * zweight * projected_offset * tie->sigmax1[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax1[2];
				    /*
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				else if (file2->status == MBNA_FILE_FIXEDZNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax1[2];
				    xs[3*nc2]   +=  xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc2+1] +=  xyweight * projected_offset * tie->sigmax1[1];
				    /*
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_FIXEDZNAV)
			    	{
				if (file2->status == MBNA_FILE_GOODNAV)
				    {
				    xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax1[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    */
				    xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax1[2];
				    }
				else if (file2->status == MBNA_FILE_POORNAV)
				    {
				    xs[3*nc1]   += -0.005 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc1+1] += -0.005 * xyweight * projected_offset * tie->sigmax1[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    */
				    xs[3*nc2]   +=  0.995 * xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc2+1] +=  0.995 * xyweight * projected_offset * tie->sigmax1[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax1[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    xs[3*nc1]   += -xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc1+1] += -xyweight * projected_offset * tie->sigmax1[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				else if (file2->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    xs[3*nc1]   += -xyweight * projected_offset * tie->sigmax1[0];
				    xs[3*nc1+1] += -xyweight * projected_offset * tie->sigmax1[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax1[2];
				    }
				}
			    xw[3*nc1]   += xyweight;
			    xw[3*nc1+1] += xyweight;
			    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
				xw[3*nc1+2] += zweight;
			    xw[3*nc2]   += xyweight;
			    xw[3*nc2+1] += xyweight;
			    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
				xw[3*nc2+2] += zweight;
/* fprintf(stderr,"long axis:  nc1:%d xx:%f %f %f  nc2:%d xx:%f %f %f\n",
nc1,xs[3*nc1],xs[3*nc1+1],xs[3*nc1+2],
nc2,xs[3*nc2],xs[3*nc2+1],xs[3*nc2+2]);*/

			    /* deal with horizontal axis */
			    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
				projected_offset = offsetx * tie->sigmax2[0]
						+ offsety * tie->sigmax2[1]
						+ offsetz * tie->sigmax2[2];
			    else
				projected_offset = offsetx * tie->sigmax2[0]
						+ offsety * tie->sigmax2[1];
			    if (fabs(tie->sigmar2) > 0.0)
			    	{
				xyweight = sqrt(mbna_offsetweight / tie->sigmar2);
			    	zweight = sqrt(mbna_offsetweight / mbna_zweightfactor / tie->sigmar2);
				}
			    else
			    	{
				xyweight = 0.0;
			    	zweight = 0.0;
				}
//zweight = 0.0;

			    /* deal with fixed, good, or poor status of sections */
			    if (file1->status == file2->status)
			    	{
				if (file1->status == MBNA_FILE_GOODNAV || file1->status == MBNA_FILE_POORNAV)
				    {
				    xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.5 * zweight * projected_offset * tie->sigmax2[2];
				    xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.5 * zweight * projected_offset * tie->sigmax2[2];
				    }
				else if (file1->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.5 * zweight * projected_offset * tie->sigmax2[2];
				    /*
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.5 * zweight * projected_offset * tie->sigmax2[2];
				    }
				else if (file1->status == MBNA_FILE_FIXEDZNAV)
				    {
				    xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax2[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    */
				    xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax2[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_GOODNAV)
			    	{
				if (file2->status == MBNA_FILE_POORNAV)
				    {
				    xs[3*nc1]   += -0.005 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc1+1] += -0.005 * xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.005 * zweight * projected_offset * tie->sigmax2[2];
				    xs[3*nc2]   +=  0.995 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc2+1] +=  0.995 * xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.995 * zweight * projected_offset * tie->sigmax2[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    xs[3*nc1]   += -xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc1+1] += -xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax2[2];
				    /*
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.0;
				    */
				    }
				else if (file2->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    xs[3*nc1]   += -xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc1+1] += -xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.5 * zweight * projected_offset * tie->sigmax2[2];
				    /*
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.5 * zweight * projected_offset * tie->sigmax2[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDZNAV)
				    {
				    xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax2[2];
				    xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax2[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_POORNAV)
			    	{
				if (file2->status == MBNA_FILE_GOODNAV)
				    {
				    xs[3*nc1]   += -0.995 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc1+1] += -0.995 * xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.995 * zweight * projected_offset * tie->sigmax2[2];
				    xs[3*nc2]   +=  0.005 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc2+1] +=  0.005 * xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.005 * zweight * projected_offset * tie->sigmax2[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    xs[3*nc1]   += -xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc1+1] += -xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax2[2];
				    /*
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.0;
				    */
				    }
				else if (file2->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    xs[3*nc1]   += -xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc1+1] += -xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.995 * zweight * projected_offset * tie->sigmax2[2];
				    /*
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.005 * zweight * projected_offset * tie->sigmax2[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDZNAV)
				    {
				    xs[3*nc1]   += -0.995 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc1+1] += -0.995 * xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax2[2];
				    xs[3*nc2]   +=  0.005 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc2+1] +=  0.005 * xyweight * projected_offset * tie->sigmax2[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_FIXEDNAV)
			    	{
				if (file2->status == MBNA_FILE_GOODNAV || file2->status == MBNA_FILE_POORNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    */
				    xs[3*nc2]   +=  xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc2+1] +=  xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax2[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax2[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDZNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    */
				    xs[3*nc2]   +=  xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc2+1] +=  xyweight * projected_offset * tie->sigmax2[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_FIXEDXYNAV)
			    	{
				if (file2->status == MBNA_FILE_GOODNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.5 * zweight * projected_offset * tie->sigmax2[2];
				    xs[3*nc2]   +=  xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc2+1] +=  xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.5 * zweight * projected_offset * tie->sigmax2[2];
				    }
				else if (file2->status == MBNA_FILE_POORNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -0.005 * zweight * projected_offset * tie->sigmax2[2];
				    xs[3*nc2]   +=  xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc2+1] +=  xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.995 * zweight * projected_offset * tie->sigmax2[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax2[2];
				    /*
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.0;
				    */
				    }
				else if (file2->status == MBNA_FILE_FIXEDZNAV)
				    {
				    /*
				    xs[3*nc1]   +=  0.0;
				    xs[3*nc1+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax2[2];
				    xs[3*nc2]   +=  xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc2+1] +=  xyweight * projected_offset * tie->sigmax2[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_FIXEDZNAV)
			    	{
				if (file2->status == MBNA_FILE_GOODNAV)
				    {
				    xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax2[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    */
				    xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax2[2];
				    }
				else if (file2->status == MBNA_FILE_POORNAV)
				    {
				    xs[3*nc1]   += -0.005 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc1+1] += -0.005 * xyweight * projected_offset * tie->sigmax2[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    */
				    xs[3*nc2]   +=  0.995 * xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc2+1] +=  0.995 * xyweight * projected_offset * tie->sigmax2[1];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax2[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    xs[3*nc1]   += -xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc1+1] += -xyweight * projected_offset * tie->sigmax2[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  0.0;
				    */
				    }
				else if (file2->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    xs[3*nc1]   += -xyweight * projected_offset * tie->sigmax2[0];
				    xs[3*nc1+1] += -xyweight * projected_offset * tie->sigmax2[1];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc1+2] +=  0.0;
				    xs[3*nc2]   +=  0.0;
				    xs[3*nc2+1] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax2[2];
				    }
				}
			    xw[3*nc1]   += xyweight;
			    xw[3*nc1+1] += xyweight;
			    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
				xw[3*nc1+2] += zweight;
			    xw[3*nc2]   += xyweight;
			    xw[3*nc2+1] += xyweight;
			    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
				xw[3*nc2+2] += zweight;
/* fprintf(stderr,"horizontal:  nc1:%d xx:%f %f %f  nc2:%d xx:%f %f %f\n",
nc1,xs[3*nc1],xs[3*nc1+1],xs[3*nc1+2],
nc2,xs[3*nc2],xs[3*nc2+1],xs[3*nc2+2]);*/

			    /* deal with semi-vertical axis */
			    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
				projected_offset = offsetx * tie->sigmax3[0]
						+ offsety * tie->sigmax3[1]
						+ offsetz * tie->sigmax3[2];
			    else
				projected_offset = offsetz * tie->sigmax3[2];
			    if (fabs(tie->sigmar3) > 0.0)
			    	{
				xyweight = sqrt(mbna_offsetweight / tie->sigmar3);
			    	zweight = sqrt(mbna_zweightfactor * mbna_offsetweight / tie->sigmar3);
				}
			    else
			    	{
				xyweight = 0.0;
			    	zweight = 0.0;
				}
//xyweight = 0.0;
//zweight = 1.0;

			    /* deal with fixed, good, or poor status of sections */
			    if (file1->status == file2->status)
			    	{
				if (file1->status == MBNA_FILE_GOODNAV || file1->status == MBNA_FILE_POORNAV)
				    {
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax3[0];
					xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc1+2] += -0.5 * zweight * projected_offset * tie->sigmax3[2];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax3[0];
				        xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc2+2] +=  0.5 * zweight * projected_offset * tie->sigmax3[2];
				    }
				else if (file1->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc1]   +=  0.0;
				        xs[3*nc1+1] +=  0.0;
				        }
				    */
				    xs[3*nc1+2] += -0.5 * zweight * projected_offset * tie->sigmax3[2];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc2]   +=  0.0;
				        xs[3*nc2+1] +=  0.0;
				        }
				    */
				    xs[3*nc2+2] +=  0.5 * zweight * projected_offset * tie->sigmax3[2];
				    }
				else if (file1->status == MBNA_FILE_FIXEDZNAV)
				    {
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax3[0];
					xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    /*
				    xs[3*nc1+2] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax3[0];
				        xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    /*
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_GOODNAV)
			    	{
				if (file2->status == MBNA_FILE_POORNAV)
				    {
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax3[0];
					xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc1+2] += -0.005 * zweight * projected_offset * tie->sigmax3[2];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax3[0];
				        xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc2+2] +=  0.995 * zweight * projected_offset * tie->sigmax3[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax3[0];
					xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax3[2];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc2]   +=  0.0;
				        xs[3*nc2+1] +=  0.0;
				        }
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				else if (file2->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax3[0];
					xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc1+2] += -0.5 * zweight * projected_offset * tie->sigmax3[2];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc2]   +=  0.0;
				        xs[3*nc2+1] +=  0.0;
				        }
				    */
				    xs[3*nc2+2] +=  0.5 * zweight * projected_offset * tie->sigmax3[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDZNAV)
				    {
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax3[0];
					xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax3[2];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax3[0];
				        xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    /*
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_POORNAV)
			    	{
				if (file2->status == MBNA_FILE_GOODNAV)
				    {
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax3[0];
					xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc1+2] += -0.995 * zweight * projected_offset * tie->sigmax3[2];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax3[0];
				        xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc2+2] +=  0.005 * zweight * projected_offset * tie->sigmax3[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax3[0];
					xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax3[2];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc2]   +=  0.0;
				        xs[3*nc2+1] +=  0.0;
				        }
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				else if (file2->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax3[0];
					xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc1+2] += -0.995 * zweight * projected_offset * tie->sigmax3[2];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc2]   +=  0.0;
				        xs[3*nc2+1] +=  0.0;
				        }
				    */
				    xs[3*nc2+2] +=  0.005 * zweight * projected_offset * tie->sigmax3[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDZNAV)
				    {
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax3[0];
					xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax3[2];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax3[0];
				        xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    /*
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_FIXEDNAV)
			    	{
				if (file2->status == MBNA_FILE_GOODNAV || file2->status == MBNA_FILE_POORNAV)
				    {
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc1]   +=  0.0;
				        xs[3*nc1+1] +=  0.0;
				        }
				    xs[3*nc1+2] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax3[0];
				        xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax3[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc1]   +=  0.0;
				        xs[3*nc1+1] +=  0.0;
				        }
				    xs[3*nc1+2] +=  0.0;
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc2]   +=  0.0;
				        xs[3*nc2+1] +=  0.0;
				        }
				    */
				    xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax3[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDZNAV)
				    {
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc1]   +=  0.0;
				        xs[3*nc1+1] +=  0.0;
				        }
				    xs[3*nc1+2] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax3[0];
				        xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    /*
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_FIXEDXYNAV)
			    	{
				if (file2->status == MBNA_FILE_GOODNAV)
				    {
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc1]   +=  0.0;
				        xs[3*nc1+1] +=  0.0;
				        }
				    */
				    xs[3*nc1+2] += -0.5 * zweight * projected_offset * tie->sigmax3[2];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax3[0];
				        xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc2+2] +=  0.5 * zweight * projected_offset * tie->sigmax3[2];
				    }
				else if (file2->status == MBNA_FILE_POORNAV)
				    {
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc1]   +=  0.0;
				        xs[3*nc1+1] +=  0.0;
				        }
				    */
				    xs[3*nc1+2] += -0.005 * zweight * projected_offset * tie->sigmax3[2];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax3[0];
				        xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc2+2] +=  0.995 * zweight * projected_offset * tie->sigmax3[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc1]   +=  0.0;
				        xs[3*nc1+1] +=  0.0;
				        }
				    */
				    xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax3[2];
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc2]   +=  0.0;
				        xs[3*nc2+1] +=  0.0;
				        }
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				else if (file2->status == MBNA_FILE_FIXEDZNAV)
				    {
				    /*
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc1]   +=  0.0;
				        xs[3*nc1+1] +=  0.0;
				        }
				    */
				    xs[3*nc1+2] += -zweight * projected_offset * tie->sigmax3[2];
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax3[0];
				        xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    /*
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				}
			    else if (file1->status == MBNA_FILE_FIXEDZNAV)
			    	{
				if (file2->status == MBNA_FILE_GOODNAV)
				    {
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax3[0];
					xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    /*
				    xs[3*nc1+2] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax3[0];
				        xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax3[2];
				    }
				else if (file2->status == MBNA_FILE_POORNAV)
				    {
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax3[0];
					xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    /*
				    xs[3*nc1+2] +=  0.0;
				    */
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc2]   +=  0.5 * xyweight * projected_offset * tie->sigmax3[0];
				        xs[3*nc2+1] +=  0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax3[2];
				    }
				else if (file2->status == MBNA_FILE_FIXEDNAV)
				    {
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax3[0];
					xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    /*
				    xs[3*nc1+2] +=  0.0;
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc2]   +=  0.0;
				        xs[3*nc2+1] +=  0.0;
				        }
				    xs[3*nc2+2] +=  0.0;
				    */
				    }
				else if (file2->status == MBNA_FILE_FIXEDXYNAV)
				    {
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
					xs[3*nc1]   += -0.5 * xyweight * projected_offset * tie->sigmax3[0];
					xs[3*nc1+1] += -0.5 * xyweight * projected_offset * tie->sigmax3[1];
					}
				    /*
				    xs[3*nc1+2] +=  0.0;
				    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
					{
				        xs[3*nc2]   +=  0.0;
				        xs[3*nc2+1] +=  0.0;
				        }
				    */
				    xs[3*nc2+2] +=  zweight * projected_offset * tie->sigmax3[2];
				    }
				}
			    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
				{
				xw[3*nc1]   += xyweight;
				xw[3*nc1+1] += xyweight;
				}
			    xw[3*nc1+2] += zweight;
			    if (mbna_invert_mode == MBNA_INVERT_ZFULL)
				{
				xw[3*nc2]   += xyweight;
				xw[3*nc2+1] += xyweight;
				}
			    xw[3*nc2+2] += zweight;
/* fprintf(stderr,"semi-vertical:  nc1:%d xx:%f %f %f  nc2:%d xx:%f %f %f\n",
nc1,xs[3*nc1],xs[3*nc1+1],xs[3*nc1+2],
nc2,xs[3*nc2],xs[3*nc2+1],xs[3*nc2+2]);*/

			    }
			}

		    /* loop over all global ties applying offsets to the affected points */
		    for (ifile=0;ifile<project.num_files;ifile++)
			{
			file = &project.files[ifile];

			for (isection=0;isection<file->num_sections;isection++)
			    {
			    /* use only set global ties */
			    section = &file->sections[isection];
			    if (section->global_tie_status != MBNA_TIE_NONE)
				{
				/* get absolute id for first snav point */
				nc1 = section->snav_invert_id[section->global_tie_snav];
				
				/* get current offset vector including reduction of block solution */
				offsetx = 0.0;
				offsety = 0.0;
				offsetz = 0.0;
				if (section->global_tie_status != MBNA_TIE_Z)
				    {
				    offsetx = section->global_tie_offset_x_m - xa[3*nc1] - x[3*nc1];
				    offsety = section->global_tie_offset_y_m - xa[3*nc1+1] - x[3*nc1+1];
				    }
				if (section->global_tie_status != MBNA_TIE_XY)
				    {
				    offsetz = section->global_tie_offset_z_m - xa[3*nc1+2] - x[3*nc1+2];
				    }
				offsetr = sqrt(offsetx * offsetx + offsety * offsety + offsetz * offsetz);
				
				/* global ties have a simple error ellipse oriented on the axes */
				
				if (section->global_tie_status != MBNA_TIE_Z)
				    {
				    /* deal with x-axis */
				    projected_offset = offsetx;
				    if (fabs(section->global_tie_xsigma) > 0.0)
					xyweight = sqrt(mbna_offsetweight / section->global_tie_xsigma);
				    else
					xyweight = 0.0;
//fprintf(stderr,"GLOBAL X: xyweight:%f mbna_offsetweight:%f xsigma:%f\n",xyweight,mbna_offsetweight,section->global_tie_xsigma);
				    xs[3*nc1]   += xyweight * projected_offset;
				    xw[3*nc1]   += xyweight;
					
				    /* deal with y-axis */
				    projected_offset = offsety;
				    if (fabs(section->global_tie_ysigma) > 0.0)
					xyweight = sqrt(mbna_offsetweight / section->global_tie_ysigma);
				    else
					xyweight = 0.0;
//fprintf(stderr,"GLOBAL Y: xyweight:%f mbna_offsetweight:%f zsigma:%f\n",xyweight,mbna_offsetweight,section->global_tie_ysigma);
				    xs[3*nc1+1] += xyweight * projected_offset;
				    xw[3*nc1+1] += xyweight;
				    }
				    
				if (section->global_tie_status != MBNA_TIE_XY)
				    {
				    /* deal with z-axis */
				    projected_offset = offsetz;
				    if (fabs(section->global_tie_zsigma) > 0.0)
					zweight = sqrt(mbna_zweightfactor * mbna_offsetweight / section->global_tie_zsigma);
				    else
					zweight = 0.0;
//fprintf(stderr,"GLOBAL Z: zweight:%f mbna_offsetweight:%f sigma:%f\n",zweight,mbna_offsetweight,section->global_tie_zsigma);
				    xs[3*nc1+2] += zweight * projected_offset;
				    xw[3*nc1+2] += zweight;
				    }
/* fprintf(stderr,"STAGE 3 GLOBAL TIE: %2d:%2d:%2d  status:%d  nc1:%d  tie: ",ifile,isection,section->global_tie_snav,section->global_tie_status,nc1);
if (section->global_tie_status != MBNA_TIE_Z) fprintf(stderr,"x:%f y:%f ",section->global_tie_offset_x_m,section->global_tie_offset_y_m);
if (section->global_tie_status != MBNA_TIE_XY) fprintf(stderr,"z:%f ",section->global_tie_offset_z_m);
fprintf(stderr," xa: ");
if (section->global_tie_status != MBNA_TIE_Z) fprintf(stderr,"x:%f y:%f ",xa[3*nc1],xa[3*nc1+1]);
if (section->global_tie_status != MBNA_TIE_XY) fprintf(stderr,"z:%f ",xa[3*nc1+2]);
fprintf(stderr," x: ");
if (section->global_tie_status != MBNA_TIE_Z) fprintf(stderr,"x:%f y:%f ",x[3*nc1],x[3*nc1+1]);
if (section->global_tie_status != MBNA_TIE_XY) fprintf(stderr,"z:%f ",x[3*nc1+2]);
fprintf(stderr," xx: ");
if (section->global_tie_status != MBNA_TIE_Z) fprintf(stderr,"x:%f y:%f ",xs[3*nc1],xs[3*nc1+1]);
if (section->global_tie_status != MBNA_TIE_XY) fprintf(stderr,"z:%f ",xs[3*nc1+2]);
fprintf(stderr," (off-xa-x-xx): ");
if (section->global_tie_status != MBNA_TIE_Z) fprintf(stderr,"x:%f y:%f ",offsetx,offsety);
if (section->global_tie_status != MBNA_TIE_XY) fprintf(stderr,"z:%f ",offsetz);
fprintf(stderr,"\n");*/
				}
			    }
			}


		    /* now loop over all points applying smoothing to the xx[] vector */
		    nseq = 0;
		    ndx = 0;
		    ndx2 = 0;
		    for (ifile=0;ifile<project.num_files;ifile++)
			{
			file = &project.files[ifile];
			if (file->status != MBNA_FILE_FIXEDNAV)
			for (isection=0;isection<file->num_sections;isection++)
			    {
			    section = &file->sections[isection];
			    if (section->continuity == MB_NO)
			    	nseq = 0;
			    for (isnav=0;isnav<section->num_snav;isnav++)
				{
				/* work only with points that are tied */
				if (section->snav_num_ties[isnav] > 0
				    || (section->global_tie_status != MBNA_TIE_NONE && section->global_tie_snav == isnav))
				    {
				    /* get ids for tied nav points */
				    if (nseq > 1)
				    	{
					nc1 = nc2;
					time_d1 = time_d2;
					}
				    if (nseq > 0)
				    	{
					nc2 = nc3;
					time_d2 = time_d3;
					}
				    nc3 = section->snav_invert_id[isnav];
				    time_d3 = section->snav_time_d[isnav];

				    /* add first derivative constraint if nseq > 1 AND dtime_d > 0.0 */
				    dtime_d = time_d3 - time_d2;
				    if (nseq > 0 && dtime_d > 0.0)
					{
			    		/* get current offset vector */
					offsetx = (x[3*nc3]   - x[3*nc2]);
					offsety = (x[3*nc3+1] - x[3*nc2+1]);
					offsetz = (x[3*nc3+2] - x[3*nc2+2]);

					/* add remaining offsets to both points */
					xyweight = mbna_smoothweight / dtime_d;
					zweight = mbna_smoothweight / dtime_d;
					xs[3*nc2]   +=  0.5 * xyweight * offsetx;
					xs[3*nc2+1] +=  0.5 * xyweight * offsety;
					xs[3*nc2+2] +=  0.5 * zweight * offsetz;
					xs[3*nc3]   += -0.5 * xyweight * offsetx;
					xs[3*nc3+1] += -0.5 * xyweight * offsety;
					xs[3*nc3+2] += -0.5 * zweight * offsetz;
					xw[3*nc2]   += xyweight;
					xw[3*nc2+1] += xyweight;
					xw[3*nc2+2] += zweight;
					xw[3*nc3]   += xyweight;
					xw[3*nc3+1] += xyweight;
					xw[3*nc3+2] += zweight;

					ndx++;
					}

				    /* add second derivative constraint if nseq > 2  AND dtime_d > 0.0 */
				    dtime_d = time_d3 - time_d1;
				    if (nseq > 1 && dtime_d > 0.0)
					{
			    		/* get current offset vector */
					offsetx = x[3*nc1]   - 2.0 * x[3*nc2]   + x[3*nc3];
					offsety = x[3*nc1+1] - 2.0 * x[3*nc2+1] + x[3*nc3+1];
					offsetz = x[3*nc1+2] - 2.0 * x[3*nc2+2] + x[3*nc3+2];

					/* add remaining offsets to both points, or just one if one is fixed (weight 2nd derivative 1/4th of first derivative)*/
					xyweight = mbna_smoothweight / dtime_d / dtime_d;
					zweight = mbna_smoothweight / dtime_d / dtime_d;
					xs[3*nc1]   += -xyweight * offsetx;
					xs[3*nc1+1] += -xyweight * offsety;
					xs[3*nc1+2] += -zweight * offsetz;
					xs[3*nc2]   +=  2.0 * xyweight * offsetx;
					xs[3*nc2+1] +=  2.0 * xyweight * offsety;
					xs[3*nc2+2] +=  2.0 * zweight * offsetz;
					xs[3*nc3]   += -xyweight * offsetx;
					xs[3*nc3+1] += -xyweight * offsety;
					xs[3*nc3+2] += -zweight * offsetz;
					xw[3*nc1]   += xyweight;
					xw[3*nc1+1] += xyweight;
					xw[3*nc1+2] += zweight;
					xw[3*nc2]   += xyweight;
					xw[3*nc2+1] += xyweight;
					xw[3*nc2+2] += zweight;
					xw[3*nc3]   += xyweight;
					xw[3*nc3+1] += xyweight;
					xw[3*nc3+2] += zweight;

					ndx2++;
					}

				    /* increment sequence counter */
				    nseq++;
				    }
				}
			    }
			}

		    /* calculate perturbation and 2-norm of perturbation */
		    perturbationsize = 0.0;
		    for (i=0;i<ncols;i++)
			{
			if (xw[i] > 0.0)
				xx[i] = xs[i] / xw[i];
			else
				xx[i] = 0.0;
			perturbationsize += xx[i] * xx[i];
			}
		    perturbationsize = sqrt(perturbationsize) / ncols;

		    /* apply perturbation */
		    for (i=0;i<ncols;i++)
			{
/* fprintf(stderr,"i:%d x:%f xx:%f ",i,x[i],xx[i]);*/
			x[i] += xx[i];
			xx[i] = 0.0;
/* fprintf(stderr,"x:%f\n",x[i]);*/
			}

		    /* calculate misfit */
		    misfit_ties = 0.0;
		    misfit_norm = 0.0;
		    nmisfit = 0;
		    for (icrossing=0;icrossing<project.num_crossings;icrossing++)
			{
			crossing = &project.crossings[icrossing];

			/* use only set crossings */
			if (crossing->status == MBNA_CROSSING_STATUS_SET)
			for (j=0;j<crossing->num_ties;j++)
			    {
			    /* get tie */
			    tie = (struct mbna_tie *) &crossing->ties[j];

			    /* get absolute id for first snav point */
			    file1 = &project.files[crossing->file_id_1];
			    section1 = &file1->sections[crossing->section_1];
			    nc1 = section1->snav_invert_id[tie->snav_1];

			    /* get absolute id for second snav point */
			    file2 = &project.files[crossing->file_id_2];
			    section2 = &file2->sections[crossing->section_2];
			    nc2 = section2->snav_invert_id[tie->snav_2];
if (nc1 > nsnav - 1 || nc2 > nsnav -1
|| nc1 < 0 || nc2 < 0)
fprintf(stderr, "BAD TIE snav ID: %d %d %d\n", nc1, nc2, nsnav);

			    /* get observed offset vector including removal of block solution */
			    offsetx = (x[3*nc2] + xa[3*nc2] - x[3*nc1] - xa[3*nc1]) - tie->offset_x_m;
			    offsety = (x[3*nc2+1] + xa[3*nc2+1] - x[3*nc1+1] - xa[3*nc1+1]) - tie->offset_y_m;
			    offsetz = (x[3*nc2+2] + xa[3*nc2+2] - x[3*nc1+2] - xa[3*nc1+2]) - tie->offset_z_m;

			    if (tie->status != MBNA_TIE_Z)
				{
				/* get long axis misfit */
				misfit = (offsetx * tie->sigmax1[0]
						+ offsety * tie->sigmax1[1]
						+ offsetz * tie->sigmax1[2]);
				misfit_ties += misfit * misfit;
				misfit_norm += misfit * misfit / tie->sigmar1 / tie->sigmar1;
				nmisfit++;
    
				/* get horizontal axis misfit */
				misfit = (offsetx * tie->sigmax2[0]
						+ offsety * tie->sigmax2[1]
						+ offsetz * tie->sigmax2[2]);
				misfit_ties += misfit * misfit;
				misfit_norm += misfit * misfit / tie->sigmar2 / tie->sigmar2;
				nmisfit++;
				}

			    if (tie->status != MBNA_TIE_XY)
				{
				/* get semi-vertical axis misfit */
				misfit = (offsetx * tie->sigmax3[0]
						+ offsety * tie->sigmax3[1]
						+ offsetz * tie->sigmax3[2]);
				misfit_ties += misfit * misfit;
				misfit_norm += misfit * misfit / tie->sigmar3 / tie->sigmar3;
				nmisfit++;
				}
			    }
			}
		    for (ifile=0;ifile<project.num_files;ifile++)
			{
			/* get file */
			file = &project.files[ifile];
			for (isection=0;isection<file->num_sections;isection++)
			    {
			    /* get section */
			    section = &file->sections[isection];
			    if (section->global_tie_status != MBNA_TIE_NONE)
				{
				/* get absolute id for snav point */
				nc1 = section->snav_invert_id[section->global_tie_snav];
if (nc1 > nsnav - 1 || nc1 < 0)
fprintf(stderr, "BAD GLOBAL TIE snav ID: %d %d\n", nc1, nsnav);

				/* get observed offset vector including removal of block solution */
				offsetx = (x[3*nc1] + xa[3*nc1]) - section->global_tie_offset_x_m;
				offsety = (x[3*nc1+1] + xa[3*nc1+1]) - section->global_tie_offset_y_m;
				offsetz = (x[3*nc1+2] + xa[3*nc1+2]) - section->global_tie_offset_z_m;
		
				if (section->global_tie_status != MBNA_TIE_Z)
				    {
				    /* get x axis misfit */
				    misfit = offsetx;
				    misfit_ties += misfit * misfit;
				    misfit_norm += misfit * misfit / section->global_tie_xsigma / section->global_tie_xsigma;
				    nmisfit++;
    
				    /* get y axis misfit */
				    misfit = offsety;
				    misfit_ties += misfit * misfit;
				    misfit_norm += misfit * misfit / section->global_tie_ysigma / section->global_tie_ysigma;
				    nmisfit++;
				    }
    
				if (section->global_tie_status != MBNA_TIE_XY)
				    {
				    /* get z axis misfit */
				    misfit = offsetz;
				    misfit_ties += misfit * misfit;
				    misfit_norm += misfit * misfit / section->global_tie_zsigma / section->global_tie_zsigma;
				    nmisfit++;
				    }
				}
			    }
			}
		    misfit_ties = sqrt(misfit_ties) / nmisfit;
		    misfit_norm = sqrt(misfit_norm) / nmisfit;

		    /* check for convergence */
		    perturbationchange = perturbationsize - perturbationsizeold;
		    convergencecriterea = fabs(perturbationchange) / misfit_ties_initial;
		    if (convergencecriterea < MBNA_CONVERGENCE || convergencecriterea > 10000.0 || iter > MBNA_INTERATION_MAX)
		    	done = MB_YES;

fprintf(stderr,"MODEL INVERT: iter:%d nmisfit:%d misfit_initial:%f misfit_ties_initial:%f misfit_ties:%f misfit_norm_initial:%f misfit_norm:%f perturbationsize:%g perturbationchange:%g convergencecriterea:%g done:%d\n",
iter,nmisfit,misfit_initial,misfit_ties_initial,misfit_ties,misfit_norm_initial,misfit_norm,perturbationsize,perturbationchange,convergencecriterea,done);

		    if (done == MB_NO)
		        {
			perturbationsizeold = perturbationsize;
		        iter++;
		    	}

		    /* save solution */
		    k = 0;
		    for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			for (j=0;j<file->num_sections;j++)
			    {
			    section = &file->sections[j];
			    for (isnav=0;isnav<section->num_snav;isnav++)
				{
				if (section->snav_num_ties[isnav] > 0
				    || section->global_tie_snav == isnav)
				    {
				    k = section->snav_invert_id[isnav];
				    section->snav_lon_offset[isnav] = (x[3*k] + xa[3*k]) * mbna_mtodeglon;
/* fprintf(stderr,"isnav:%d k:%d x[3*k]:%f xa[3*k]:%f mbna_mtodeglon:%f section->snav_lon_offset[isnav]:%f\n",
isnav,k,x[3*k],xa[3*k],mbna_mtodeglon,section->snav_lon_offset[isnav]);*/
				    section->snav_lat_offset[isnav] = (x[3*k+1] + xa[3*k+1]) * mbna_mtodeglat;
/* fprintf(stderr,"isnav:%d k:%d x[3*k+1]:%f xa[3*k+1]:%f mbna_mtodeglat:%f section->snav_lat_offset[isnav]:%f\n",
isnav,k,x[3*k+1],xa[3*k+1],mbna_mtodeglat,section->snav_lat_offset[isnav]);*/
				    section->snav_z_offset[isnav] = (x[3*k+2] + xa[3*k+2]);
/* fprintf(stderr,"isnav:%d k:%d x[3*k+2]:%f xa[3*k+2]:%f section->snav_lat_offset[isnav]:%f\n\n",
isnav,k,x[3*k+2],xa[3*k+2],section->snav_lat_offset[isnav]); */
				    }
				}
			    }
			}

		    /* interpolate the solution */
		    mbnavadjust_interpolatesolution();

		    /* save interpolated solution */
		    k = 0;
		    for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			for (j=0;j<file->num_sections;j++)
			    {
			    section = &file->sections[j];
			    for (isnav=0;isnav<section->num_snav;isnav++)
				{
				section->snav_lon_offset[isnav] = section->snav_lon_offset_int[isnav];
				section->snav_lat_offset[isnav] = section->snav_lat_offset_int[isnav];
				section->snav_z_offset[isnav] = section->snav_z_offset_int[isnav];
				}
			    }
			}

		    /* set message dialog on */
		    if (iter % 100 == 0)
		    	{
		    	sprintf(message,"Completed inversion iteration %d Convergence:%.2f", iter, convergencecriterea / MBNA_CONVERGENCE);
		    	do_message_update(message);
		    	}


		    /* update model plot */
		    if (project.modelplot == MB_YES && iter % 25 == 0)
		    	mbnavadjust_modelplot_plot();
		    }
		}

	/* output results from navigation solution */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed >= 10
			|| project.num_truecrossings_analyzed == project.num_truecrossings)
		&& ok_to_invert == MB_YES
		&& error == MB_ERROR_NO_ERROR)
    		{
		/* now output inverse solution */
		sprintf(message,"Outputting navigation solution...");
		do_message_update(message);

		sprintf(message, " > Final misfit:%12g\n > Initial misfit:%12g\n",
			misfit_ties, misfit_initial);
		do_info_add(message, MB_NO);

		/* get crossing offset results */
		sprintf(message, " > Nav Tie Offsets (m):  id  observed  solution  error\n");
		do_info_add(message, MB_NO);
		for (icrossing=0;icrossing<project.num_crossings;icrossing++)
		    {
		    crossing = &project.crossings[icrossing];

		    /* check only set ties */
		    if (crossing->status == MBNA_CROSSING_STATUS_SET)
		    for (j=0;j<crossing->num_ties;j++)
			{
			tie = (struct mbna_tie *) &crossing->ties[j];
			offset_x =  project.files[crossing->file_id_2].sections[crossing->section_2].snav_lon_offset[tie->snav_2]
				- project.files[crossing->file_id_1].sections[crossing->section_1].snav_lon_offset[tie->snav_1];
			offset_y =  project.files[crossing->file_id_2].sections[crossing->section_2].snav_lat_offset[tie->snav_2]
				- project.files[crossing->file_id_1].sections[crossing->section_1].snav_lat_offset[tie->snav_1];
			offset_z =  project.files[crossing->file_id_2].sections[crossing->section_2].snav_z_offset[tie->snav_2]
				- project.files[crossing->file_id_1].sections[crossing->section_1].snav_z_offset[tie->snav_1];
			tie->inversion_status = MBNA_INVERSION_CURRENT;
    			tie->inversion_offset_x = offset_x;
    			tie->inversion_offset_y = offset_y;
   			tie->inversion_offset_x_m = offset_x / mbna_mtodeglon;
    			tie->inversion_offset_y_m = offset_y / mbna_mtodeglat;
    			tie->inversion_offset_z_m = offset_z;
/* fprintf(stderr,"mbna_mtodeglon:%f mbna_mtodeglat:%f\n",mbna_mtodeglon,mbna_mtodeglat);
fprintf(stderr,"offsets:%f %f %f   offsets_m:%f %f %f\n",
offset_x,offset_y,offset_z,tie->inversion_offset_x_m,tie->inversion_offset_y_m,tie->inversion_offset_z_m);*/

			sprintf(message, " >     %4d   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f\n",
				icrossing,
				tie->offset_x_m,
				tie->offset_y_m,
				tie->offset_z_m,
				tie->inversion_offset_x_m,
				tie->inversion_offset_y_m,
				tie->inversion_offset_z_m,
				(tie->inversion_offset_x_m - tie->offset_x_m),
				(tie->inversion_offset_y_m - tie->offset_y_m),
				(tie->inversion_offset_z_m - tie->offset_z_m));
			do_info_add(message, MB_NO);
			}
		    }

		/* get global tie results */
		sprintf(message, " > Global Tie Offsets (m):  id  observed  solution  error\n");
		do_info_add(message, MB_NO);
		for (ifile=0;ifile<project.num_files;ifile++)
		    {
		    file = &project.files[ifile];
		    for (isection=0;isection<file->num_sections;isection++)
			{
			section = &file->sections[isection];
			if (section->global_tie_status != MBNA_TIE_NONE)
			    {
			    offset_x =  section->snav_lon_offset[section->global_tie_snav];
			    offset_y =  section->snav_lat_offset[section->global_tie_snav];
			    offset_z =  section->snav_z_offset[section->global_tie_snav];
			    
			    sprintf(message, " >     %2.2d:%2.2d:%2.2d %d   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f\n",
				ifile,isection,section->global_tie_snav,section->global_tie_status,
				section->global_tie_offset_x_m,
				section->global_tie_offset_y_m,
				section->global_tie_offset_z_m,
				offset_x / mbna_mtodeglon,
				offset_y / mbna_mtodeglat,
				offset_z,
				(offset_x / mbna_mtodeglon - section->global_tie_offset_x_m),
				(offset_y / mbna_mtodeglat - section->global_tie_offset_y_m),
				(offset_z - section->global_tie_offset_z_m));
			    do_info_add(message, MB_NO);
			    }
			}
		    }

		/* write updated project */
		project.inversion = MBNA_INVERSION_CURRENT;
		mbnavadjust_write_project();

		/* deallocate arrays */
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&xx,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&xa,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&xs,&error);
		status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&xw,&error);

		/* turn off message dialog */
		do_message_off();
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/

int
mbnavadjust_applynav()
{
	/* local variables */
	char	*function_name = "mbnavadjust_applynav";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_section *section;
	char	npath[STRING_MAX];
	char	apath[STRING_MAX];
	char	opath[STRING_MAX];
	FILE	*nfp, *afp, *ofp;
	char	*result;
	char	buffer[BUFFER_MAX];
	int	nscan;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	heading;
	double	speed;
	double	draft;
	double	roll;
	double	pitch;
	double	heave;
	double	factor;
	double	zoffset;
	char	ostring[STRING_MAX];
	int	mbp_heading_mode;
	double	mbp_headingbias;
	int	mbp_rollbias_mode;
	double	mbp_rollbias;
	double	mbp_rollbias_port;
	double	mbp_rollbias_stbd;
	int	done;
	int	isection, isnav;
	double	seconds;
	int	i;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* output results from navigation solution */
    	if (project.open == MB_YES
    		&& project.num_crossings > 0
		&& (project.num_crossings_analyzed >= 10
			|| project.num_truecrossings_analyzed == project.num_truecrossings)
		&& error == MB_ERROR_NO_ERROR)
    		{

		/* now output inverse solution */
		sprintf(message,"Applying navigation solution...");
		do_message_on(message);

		/* generate new nav files */
		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    sprintf(npath,"%s/nvs_%4.4d.mb166",
			project.datadir,i);
		    sprintf(apath,"%s/nvs_%4.4d.na%d",
			project.datadir,i,file->output_id);
		    sprintf(opath,"%s.na%d", file->path, file->output_id);
		    if (file->status == MBNA_FILE_FIXEDNAV)
			{
			sprintf(message, " > Not outputting updated nav to fixed file %s\n", opath);
			do_info_add(message, MB_NO);
			if (mbna_verbose == 0)
				fprintf(stderr,"%s",message);
			}
		    else if ((nfp = fopen(npath,"r")) == NULL)
			{
			status = MB_FAILURE;
			error = MB_ERROR_OPEN_FAIL;
			sprintf(message, " > Unable to read initial nav file %s\n", npath);
			do_info_add(message, MB_NO);
			if (mbna_verbose == 0)
				fprintf(stderr,"%s",message);
			}
		    else if ((afp = fopen(apath,"w")) == NULL)
			{
		        fclose(nfp);
			status = MB_FAILURE;
			error = MB_ERROR_OPEN_FAIL;
			sprintf(message, " > Unable to open output nav file %s\n", apath);
			do_info_add(message, MB_NO);
			if (mbna_verbose == 0)
				fprintf(stderr,"%s",message);
			}
		    else if ((ofp = fopen(opath,"w")) == NULL)
			{
		        fclose(nfp);
		        fclose(afp);
			status = MB_FAILURE;
			error = MB_ERROR_OPEN_FAIL;
			sprintf(message, " > Unable to open output nav file %s\n", opath);
			do_info_add(message, MB_NO);
			if (mbna_verbose == 0)
				fprintf(stderr,"%s",message);
			}
		    else
			{
			sprintf(message, " > Output updated nav to %s\n", opath);
			do_info_add(message, MB_NO);
			if (mbna_verbose == 0)
				fprintf(stderr,"%s",message);

			/* write file header */
			right_now = time((time_t *)0);
			strcpy(date,ctime(&right_now));
                        date[strlen(date)-1] = '\0';
			if ((user_ptr = getenv("USER")) == NULL)
				user_ptr = getenv("LOGNAME");
			if (user_ptr != NULL)
				strcpy(user,user_ptr);
			else
				strcpy(user, "unknown");
			gethostname(host,MBP_FILENAMESIZE);
			sprintf(ostring, "# Adjusted navigation generated using MBnavadjust\n");
				fprintf(ofp, "%s", ostring);
				fprintf(afp, "%s", ostring);
			sprintf(ostring, "# MB-System version:        %s\n",MB_VERSION);
				fprintf(ofp, "%s", ostring);
				fprintf(afp, "%s", ostring);
			sprintf(ostring, "# MB-System build data:     %s\n",MB_BUILD_DATE);
				fprintf(ofp, "%s", ostring);
				fprintf(afp, "%s", ostring);
			sprintf(ostring, "# MBnavadjust version:      %s\n",rcs_id);
				fprintf(ofp, "%s", ostring);
				fprintf(afp, "%s", ostring);
			sprintf(ostring, "# MBnavadjust project name: %s\n",project.name);
				fprintf(ofp, "%s", ostring);
				fprintf(afp, "%s", ostring);
			sprintf(ostring, "# MBnavadjust project path: %s\n",project.path);
				fprintf(ofp, "%s", ostring);
				fprintf(afp, "%s", ostring);
			sprintf(ostring, "# MBnavadjust project home: %s\n",project.home);
				fprintf(ofp, "%s", ostring);
				fprintf(afp, "%s", ostring);
			sprintf(ostring, "# Generated by user <%s> on cpu <%s> at <%s>\n", user,host,date);
				fprintf(ofp, "%s", ostring);
				fprintf(afp, "%s", ostring);

			/* read the input nav */
			isection = 0;
			section = &file->sections[isection];
			isnav = 0;
			done = MB_NO;
			while (done == MB_NO)
			    {
			    if ((result = fgets(buffer,BUFFER_MAX,nfp)) != buffer)
				{
				done = MB_YES;
				}
			    else if ((nscan = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
						    &time_i[0], &time_i[1], &time_i[2], &time_i[3],
						    &time_i[4], &seconds, &time_d,
						    &navlon, &navlat, &heading, &speed,
						    &draft, &roll, &pitch, &heave)) >= 11)
				{
				/* get integer seconds and microseconds */
				time_i[5] = (int) floor(seconds);
				time_i[6] = (int) ((seconds - (double)time_i[5]) * 1000000);
				
				/* fix nav from early version */
				if (nscan < 15)
				    {
				    draft = 0.0;
				    roll = 0.0;
				    pitch = 0.0;
				    heave = 0.0;
				    }

				/* get next snav interval if needed */
				while (time_d > section->snav_time_d[isnav+1]
				    && !(isection == file->num_sections - 1
					&& isnav == section->num_snav - 2))
				    {
				    if (isnav < section->num_snav - 2)
					{
					isnav++;
					}
				    else if (isection < file->num_sections)
					{
					isection++;
					section = &file->sections[isection];
					isnav = 0;
					}
				    }

				/* update the nav if possible (and it should be...) */
				if (time_d < section->snav_time_d[isnav])
				    {
				    factor = 0.0;
				    }
				else if (time_d > section->snav_time_d[isnav+1])
				    {
				    factor = 1.0;
				    }
				else
				    {
				    if (section->snav_time_d[isnav+1] > section->snav_time_d[isnav])
					{
					factor = (time_d - section->snav_time_d[isnav])
						    / (section->snav_time_d[isnav+1] - section->snav_time_d[isnav]);
/*if (fabs(time_d - section->snav_time_d[isnav]) < 1.0)
fprintf(stderr,"%f %f\n",pitch,section->snav_z_offset[isnav]);
else if (fabs(time_d - section->snav_time_d[isnav+1]) < 1.0)
fprintf(stderr,"%f %f\n",pitch,section->snav_z_offset[isnav+1]);*/
					}
				    else
					factor = 0.0;
				    }

				/* update and output only nonzero navigation */
				if (fabs(navlon) > 0.0000001 && fabs(navlat) > 0.0000001)
				    {
				    navlon += section->snav_lon_offset[isnav]
						    + factor * (section->snav_lon_offset[isnav+1]
							    - section->snav_lon_offset[isnav]);
				    navlat += section->snav_lat_offset[isnav]
						    + factor * (section->snav_lat_offset[isnav+1]
							    - section->snav_lat_offset[isnav]);
				    zoffset = section->snav_z_offset[isnav]
						    + factor * (section->snav_z_offset[isnav+1]
							    - section->snav_z_offset[isnav]);

				    /* write the updated nav out */
				    /* printing this string twice because in some situations the first
					    print has the time_d value come out as "nan" - this is the worst sort
					    of kluge for a real but mysterious bug - apologies to all who find this
					    - DWC 18 Aug 2007 R/V Atlantis Cobb Segment JDF Ridge */
				    sprintf(ostring, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f %.3f %.2f %.2f %.2f %.3f\r\n",
						time_i[0], time_i[1], time_i[2], time_i[3],
						time_i[4], time_i[5], time_i[6], time_d,
						navlon, navlat, heading, speed,
						draft, roll, pitch, heave, zoffset);
				    sprintf(ostring, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f %.3f %.2f %.2f %.2f %.3f\r\n",
						time_i[0], time_i[1], time_i[2], time_i[3],
						time_i[4], time_i[5], time_i[6], time_d,
						navlon, navlat, heading, speed,
						draft, roll, pitch, heave, zoffset);
				    fprintf(ofp, "%s", ostring);
				    fprintf(afp, "%s", ostring);
/* fprintf(stderr, "NAV OUT: %3.3d:%3.3d:%2.2d factor:%f | %s", i,isection,isnav,factor,ostring); */
				    }
				}
			    }
		        fclose(nfp);
		        fclose(afp);
		        fclose(ofp);

			/* get bias values */
			mb_pr_get_heading(mbna_verbose, file->path,
					    &mbp_heading_mode,
					    &mbp_headingbias,
					    &error);
			mb_pr_get_rollbias(mbna_verbose, file->path,
					    &mbp_rollbias_mode,
					    &mbp_rollbias,
					    &mbp_rollbias_port,
					    &mbp_rollbias_stbd,
					    &error);

			/* update output file in mbprocess parameter file */
			status = mb_pr_update_format(mbna_verbose, file->path,
				    MB_YES, file->format,
				    &error);
			status = mb_pr_update_navadj(mbna_verbose, file->path,
				    MBP_NAVADJ_LLZ, opath,
				    MBP_NAV_LINEAR,
				    &error);

			/* update heading bias in mbprocess parameter file */
			mbp_headingbias = file->heading_bias + file->heading_bias_import;
			if (mbp_headingbias == 0.0)
			    {
			    if (mbp_heading_mode == MBP_HEADING_OFF
				|| mbp_heading_mode == MBP_HEADING_OFFSET)
				mbp_heading_mode = MBP_HEADING_OFF;
			    else if (mbp_heading_mode == MBP_HEADING_CALC
				|| mbp_heading_mode == MBP_HEADING_CALCOFFSET)
				mbp_heading_mode = MBP_HEADING_CALC;
			    }
			else
			    {
			    if (mbp_heading_mode == MBP_HEADING_OFF
				|| mbp_heading_mode == MBP_HEADING_OFFSET)
				mbp_heading_mode = MBP_HEADING_OFFSET;
			    else if (mbp_heading_mode == MBP_HEADING_CALC
				|| mbp_heading_mode == MBP_HEADING_CALCOFFSET)
				mbp_heading_mode = MBP_HEADING_CALCOFFSET;
			    }
			status = mb_pr_update_heading(mbna_verbose, file->path,
				    mbp_heading_mode, mbp_headingbias,
				    &error);

			/* update roll bias in mbprocess parameter file */
			mbp_rollbias = file->roll_bias + file->roll_bias_import;
			if (mbp_rollbias == 0.0)
			    {
			    if (mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
				{
				mbp_rollbias_port = mbp_rollbias
					+ mbp_rollbias_port - file->roll_bias_import;
				mbp_rollbias_stbd = mbp_rollbias
					+ mbp_rollbias_stbd - file->roll_bias_import;
				}
			    else
				mbp_rollbias_mode = MBP_ROLLBIAS_OFF;
			    }
			else
			    {
			    if (mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
				{
				mbp_rollbias_port = mbp_rollbias
					+ mbp_rollbias_port - file->roll_bias_import;
				mbp_rollbias_stbd = mbp_rollbias
					+ mbp_rollbias_stbd - file->roll_bias_import;
				}
			    else
				{
				mbp_rollbias_mode = MBP_ROLLBIAS_SINGLE;
				}
			    }
			status = mb_pr_update_rollbias(mbna_verbose, file->path,
				    mbp_rollbias_mode, mbp_rollbias,
				    mbp_rollbias_port, mbp_rollbias_stbd,
				    &error);
			}
		    }

		/* turn off message dialog */
		do_message_off();
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}


/*--------------------------------------------------------------------*/

int
mbnavadjust_interpolatesolution()
{
	/* local variables */
	char	*function_name = "mbnavadjust_interpolatesolution";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_file *pfile;
	struct mbna_section *section;
	struct mbna_section *psection;
	int	previoustie;
	int	ifilestart;
	int	isectionstart;
	int	isnavstart;
	double	plonoffset;
	double	platoffset;
	double	pzoffset;
	double	ptime_d;
	double	factor;
	int	ok;
	int	ii, jj, iisnav;
	int	i, j, isnav;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* linearly interpolate solution between tied snavs */
	previoustie = MB_NO;
	ifilestart = 0;
	isectionstart = 0;
	isnavstart = 0;
	for (i=0;i<project.num_files;i++)
	    {
	    file = &project.files[i];
	    for (j=0;j<file->num_sections;j++)
		{
		section = &file->sections[j];
		for (isnav=0;isnav<section->num_snav;isnav++)
		    {
		    /* deal with constrained snav points */
		    if (section->snav_num_ties[isnav] > 0
			|| section->global_tie_snav == isnav)
			{
			/* if no previous tie set apply current offset to intervening snav points */
			if (previoustie == MB_NO)
			    {
			    for (ii=ifilestart;ii<=i;ii++)
			    	{
				pfile = &project.files[ii];
				for (jj=0;jj<pfile->num_sections;jj++)
				    {
				    psection = &pfile->sections[jj];
				    for (iisnav=0;iisnav<psection->num_snav;iisnav++)
					{
					ok = MB_YES;
					if (ii == ifilestart && jj < isectionstart)
						ok = MB_NO;
					if (ii == ifilestart && jj == isectionstart && iisnav < isnavstart)
						ok = MB_NO;
					if (ii == i && jj > j)
						ok = MB_NO;
					if (ii == i && jj == j && iisnav > isnav)
						ok = MB_NO;
					if (ok == MB_YES)
						{
						psection->snav_lon_offset_int[iisnav] = section->snav_lon_offset[isnav];
						psection->snav_lat_offset_int[iisnav] = section->snav_lat_offset[isnav];
						psection->snav_z_offset_int[iisnav] = section->snav_z_offset[isnav];
/*fprintf(stderr,"SET1: %d %d %d   %f %f %f\n",
ii,jj,iisnav,
psection->snav_lon_offset_int[iisnav],
psection->snav_lat_offset_int[iisnav],
psection->snav_z_offset_int[iisnav]);*/
						}
					}
				    }
				}
			    }

			/* if previous tie set interpolate intervening snav points */
			if (previoustie == MB_YES)
			    {
			    pfile = &project.files[ifilestart];
			    psection = &pfile->sections[isectionstart];
			    plonoffset = psection->snav_lon_offset[isnavstart];
			    platoffset = psection->snav_lat_offset[isnavstart];
			    pzoffset = psection->snav_z_offset[isnavstart];
			    ptime_d = psection->snav_time_d[isnavstart];
			    for (ii=ifilestart;ii<=i;ii++)
			    	{
				pfile = &project.files[ii];
				for (jj=0;jj<pfile->num_sections;jj++)
				    {
				    psection = &pfile->sections[jj];
				    for (iisnav=0;iisnav<psection->num_snav;iisnav++)
					{
					ok = MB_YES;
					if (ii == ifilestart && jj < isectionstart)
						ok = MB_NO;
					if (ii == ifilestart && jj == isectionstart && iisnav <= isnavstart)
						ok = MB_NO;
					if (ii == i && jj > j)
						ok = MB_NO;
					if (ii == i && jj == j && iisnav > isnav)
						ok = MB_NO;
					if (ok == MB_YES)
						{
						if ((section->snav_time_d[isnav] - ptime_d) > 0.0)
							{
							factor = (psection->snav_time_d[iisnav] - ptime_d)
								/ (section->snav_time_d[isnav] - ptime_d);
							}
						else
							{
							factor = 0.0;
							}
						psection->snav_lon_offset_int[iisnav] = plonoffset
							+ factor * (section->snav_lon_offset[isnav] - plonoffset);
						psection->snav_lat_offset_int[iisnav] = platoffset
							+ factor * (section->snav_lat_offset[isnav] - platoffset);
						psection->snav_z_offset_int[iisnav] = pzoffset
							+ factor * (section->snav_z_offset[isnav] - pzoffset);
/*fprintf(stderr,"SET2: %d %d %d   %f %f %f   times: %f %f %f\n",
ii,jj,iisnav,
psection->snav_lon_offset_int[iisnav],
psection->snav_lat_offset_int[iisnav],
psection->snav_z_offset_int[iisnav],
section->snav_time_d[isnav],psection->snav_time_d[iisnav],ptime_d);*/
						}
					}
				    }
				}
			    }

			/* reset tracking */
			previoustie = MB_YES;
			ifilestart = i;
			isectionstart = j;
			isnavstart = isnav;
			}

		    /* deal with a break in continuity */
		    else if (isnav == 0 && section->continuity == MB_NO)
		    	{
			/* if previous tie set apply that offset to intervening snav points */
			if (previoustie == MB_YES)
			    {
			    pfile = &project.files[ifilestart];
			    psection = &pfile->sections[isectionstart];
			    plonoffset = psection->snav_lon_offset[isnavstart];
			    platoffset = psection->snav_lat_offset[isnavstart];
			    pzoffset = psection->snav_z_offset[isnavstart];
			    for (ii=ifilestart;ii<=i;ii++)
			    	{
				pfile = &project.files[ii];
				for (jj=0;jj<pfile->num_sections;jj++)
				    {
				    psection = &pfile->sections[jj];
				    for (iisnav=0;iisnav<psection->num_snav;iisnav++)
					{
					ok = MB_YES;
					if (ii == ifilestart && jj < isectionstart)
						ok = MB_NO;
					if (ii == ifilestart && jj == isectionstart && iisnav <= isnavstart)
						ok = MB_NO;
					if (ii == i && jj > j)
						ok = MB_NO;
					if (ii == i && jj == j && iisnav >= isnav)
						ok = MB_NO;
					if (ok == MB_YES)
						{
						psection->snav_lon_offset_int[iisnav] = plonoffset;
						psection->snav_lat_offset_int[iisnav] = platoffset;
						psection->snav_z_offset_int[iisnav] = pzoffset;
/*fprintf(stderr,"SET3: %d %d %d   %f %f %f\n",
ii,jj,iisnav,
psection->snav_lon_offset_int[iisnav],
psection->snav_lat_offset_int[iisnav],
psection->snav_z_offset_int[iisnav]);*/
						}

					}
				    }
				}
			    }

			/* reset tracking */
			previoustie = MB_NO;
			ifilestart = i;
			isectionstart = j;
			isnavstart = isnav;
			}

		    /* deal with end of data */
		    else if (i == project.num_files - 1
		    	&& j == file->num_sections - 1
			&& isnav == section->num_snav - 1)
		    	{
			/* if previous tie set apply that offset to intervening snav points */
			if (previoustie == MB_YES)
			    {
			    pfile = &project.files[ifilestart];
			    psection = &pfile->sections[isectionstart];
			    plonoffset = psection->snav_lon_offset[isnavstart];
			    platoffset = psection->snav_lat_offset[isnavstart];
			    pzoffset = psection->snav_z_offset[isnavstart];
			    for (ii=ifilestart;ii<=i;ii++)
			    	{
				pfile = &project.files[ii];
				for (jj=0;jj<pfile->num_sections;jj++)
				    {
				    psection = &pfile->sections[jj];
				    for (iisnav=0;iisnav<psection->num_snav;iisnav++)
					{
					ok = MB_YES;
					if (ii == ifilestart && jj < isectionstart)
						ok = MB_NO;
					if (ii == ifilestart && jj == isectionstart && iisnav <= isnavstart)
						ok = MB_NO;
					if (ii == i && jj > j)
						ok = MB_NO;
					if (ii == i && jj == j && iisnav > isnav)
						ok = MB_NO;
					if (ok == MB_YES)
						{
						psection->snav_lon_offset_int[iisnav] = plonoffset;
						psection->snav_lat_offset_int[iisnav] = platoffset;
						psection->snav_z_offset_int[iisnav] = pzoffset;
/*fprintf(stderr,"SET4: %d %d %d   %f %f %f\n",
ii,jj,iisnav,
psection->snav_lon_offset_int[iisnav],
psection->snav_lat_offset_int[iisnav],
psection->snav_z_offset_int[iisnav]);*/
						}

					}
				    }
				}
			    }

			/* reset tracking */
			previoustie = MB_NO;
			ifilestart = i;
			isectionstart = j;
			isnavstart = isnav;
			}

		    /* zero unconstrained snav points - these will be interpolated later if possible */
		    else
		        {
		    	section->snav_lon_offset_int[isnav] = 0.0;
		    	section->snav_lat_offset_int[isnav] = 0.0;
		    	section->snav_z_offset_int[isnav] = 0.0;
/*fprintf(stderr,"SET5: %d %d %d   %f %f %f\n",
i,j,isnav,
section->snav_lon_offset_int[isnav],
section->snav_lat_offset_int[isnav],
section->snav_z_offset_int[isnav]);*/
		        }
		    }
		}
	    }
/*for (i=0;i<project.num_files;i++)
{
file = &project.files[i];
for (j=0;j<file->num_sections;j++)
{
section = &file->sections[j];
for (isnav=0;isnav<section->num_snav;isnav++)
{
fprintf(stderr,"INTERPOLATION: %2d %2d %2d   %f %f %f\n",
i,j,isnav,
section->snav_lon_offset_int[isnav],
section->snav_lat_offset_int[isnav],
section->snav_z_offset_int[isnav]);
}
}
}*/

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_set_modelplot_graphics(void *mp_xgid, int *mp_brdr)
{
	/* local variables */
	char	*function_name = "mbnavadjust_set_modelplot_graphics";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       mp_xgid:      %p\n",mp_xgid);
		fprintf(stderr,"dbg2       mp_brdr:      %d %d %d %d\n",
			mp_brdr[0], mp_brdr[1], mp_brdr[2], mp_brdr[3]);
		}

	/* set graphics id */
	pmodp_xgid = mp_xgid;

	/* set borders */
	for (i=0;i<4;i++)
		{
		modp_borders[i] = mp_brdr[i];
		}

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/

int
mbnavadjust_modelplot_setzoom()
{
	/* local variables */
	char	*function_name = "mbnavadjust_modelplot_setzoom";
	int	status = MB_SUCCESS;
	int	xo;
	int	plot_width;
	double	xscale;
	int	ipingstart, ipingend;
	int	itiestart, itieend;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* plot zoom if active */
	if ((mbna_modelplot_zoom_x1 >= 0 || mbna_modelplot_zoom_x2 >= 0)
	    && mbna_modelplot_zoom_x1 != mbna_modelplot_zoom_x2)
		{
		if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES)
			{
			plot_width = mbna_modelplot_width - 8 * MBNA_MODELPLOT_X_SPACE;
			xo = 5 * MBNA_MODELPLOT_X_SPACE;
			xscale = ((double)plot_width) / (mbna_modelplot_end - mbna_modelplot_start + 1);

			ipingstart = (MIN(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - xo) / xscale
					+ mbna_modelplot_start;
			ipingstart = MIN(MAX(ipingstart, 0), project.num_pings - 1);
			ipingend = (MAX(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - xo) / xscale
					+ mbna_modelplot_start;
			ipingend = MIN(MAX(ipingend, 0), project.num_pings - 1);

			if (ipingend > ipingstart)
				{
				mbna_modelplot_zoom = MB_YES;
				mbna_modelplot_startzoom = ipingstart;
				mbna_modelplot_endzoom = ipingend;
				}
			else
				mbna_modelplot_zoom = MB_NO;
			}

		else if (project.modelplot_style == MBNA_MODELPLOT_PERTURBATION)
			{
			plot_width = mbna_modelplot_width - 8 * MBNA_MODELPLOT_X_SPACE;
			xo = 5 * MBNA_MODELPLOT_X_SPACE;
			xscale = ((double)plot_width) / (mbna_modelplot_end - mbna_modelplot_start + 1);

			ipingstart = (MIN(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - xo) / xscale
					+ mbna_modelplot_start;
			ipingstart = MIN(MAX(ipingstart, 0), project.num_pings - 1);
			ipingend = (MAX(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - xo) / xscale
					+ mbna_modelplot_start;
			ipingend = MIN(MAX(ipingend, 0), project.num_pings - 1);

			if (ipingend > ipingstart)
				{
				mbna_modelplot_zoom = MB_YES;
				mbna_modelplot_startzoom = ipingstart;
				mbna_modelplot_endzoom = ipingend;
				}
			else
				mbna_modelplot_zoom = MB_NO;
			}

		else
			{
			itiestart = (MIN(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo)
					/ mbna_modelplot_xscale;
			itieend = (MAX(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo)
					/ mbna_modelplot_xscale;
			itiestart = MAX(0,itiestart);
			itieend = MIN(mbna_num_ties_plot-1, itieend);
			if (itieend > itiestart)
				{
				mbna_modelplot_tiezoom = MB_YES;
				mbna_modelplot_tiestartzoom = itiestart;
				mbna_modelplot_tieendzoom = itieend;
				}
			else
				mbna_modelplot_tiezoom = MB_NO;
			}

		mbna_modelplot_zoom_x1 = 0;
		mbna_modelplot_zoom_x2 = 0;
		}

	/* reset zoom to off otherwise */
	else
		{
		if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES)
			{
			mbna_modelplot_zoom = MB_NO;
			mbna_modelplot_start = 0;
			mbna_modelplot_end = project.num_pings - 1;
			}
		else if (project.modelplot_style == MBNA_MODELPLOT_PERTURBATION)
			{
			mbna_modelplot_zoom = MB_NO;
			mbna_modelplot_start = 0;
			mbna_modelplot_end = project.num_pings - 1;
			}
		else
			{
			mbna_modelplot_tiezoom = MB_NO;
			mbna_modelplot_tiestart = 0;
			mbna_modelplot_tieend = mbna_num_ties_plot - 1;
			}
		}

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/

int
mbnavadjust_modelplot_pick(int x, int y)
{
	/* local variables */
	char	*function_name = "mbnavadjust_modelplot_pick";
	int	status = MB_SUCCESS;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       x:           %d\n",x);
		fprintf(stderr,"dbg2       y:           %d\n",y);
		}

	/* find nearest snav pt with tie */
    	if (project.open == MB_YES
    		&& project.inversion != MBNA_INVERSION_NONE
		&& project.modelplot == MB_YES)
    		{
		if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES)
			{
			mbnavadjust_modelplot_pick_timeseries(x, y);
			}
		else if (project.modelplot_style == MBNA_MODELPLOT_PERTURBATION)
			{
			mbnavadjust_modelplot_pick_perturbation(x, y);
			}
		else
			{
			mbnavadjust_modelplot_pick_tieoffsets(x, y);
			}
		}

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/

int
mbnavadjust_modelplot_pick_timeseries(int x, int y)
{
	/* local variables */
	char	*function_name = "mbnavadjust_modelplot_pick_timeseries";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	range;
	int	rangemin;
	int	pick_crossing;
	int	pick_tie;
	int	pick_file;
	int	pick_section;
	int	pick_snav;
	int	ntieselect;
	int	i, j, ix, iy, iping;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       x:           %d\n",x);
		fprintf(stderr,"dbg2       y:           %d\n",y);
		}

	/* find nearest snav pt with tie */
    	if (project.open == MB_YES
    		&& project.inversion != MBNA_INVERSION_NONE
		&& project.modelplot == MB_YES)
    		{
		rangemin = 10000000;
fprintf(stderr,"mbnavadjust_modelplot_pick_timeseries: %d %d\n",x,y);
		/* search by looping over crossings */
		for (i=0;i<project.num_crossings;i++)
		    {
		    crossing = &(project.crossings[i]);

		    /* loop over all ties for this crossing */
		    for (j=0;j<crossing->num_ties;j++)
		    	{
			tie = &(crossing->ties[j]);

			/* handle first snav point */
			file = &project.files[crossing->file_id_1];
			section = &file->sections[crossing->section_1];

			if (section->show_in_modelplot == MB_YES)
				{
				iping = section->modelplot_start_count + section->snav_id[tie->snav_1];
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon);
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_1;
					pick_section = crossing->section_1;
					pick_snav = tie->snav_1;
					}

				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat);
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_1;
					pick_section = crossing->section_1;
					pick_snav = tie->snav_1;
					}

				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_1;
					pick_section = crossing->section_1;
					pick_snav = tie->snav_1;
					}
				}

			/* handle second snav point */
			file = &project.files[crossing->file_id_2];
			section = &file->sections[crossing->section_2];

			if (section->show_in_modelplot == MB_YES)
				{
			iping = section->modelplot_start_count + section->snav_id[tie->snav_2];
			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon);
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_2;
					pick_section = crossing->section_2;
					pick_snav = tie->snav_2;
					}

				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat);
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_2;
					pick_section = crossing->section_2;
					pick_snav = tie->snav_2;
					}

				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_2;
					pick_section = crossing->section_2;
					pick_snav = tie->snav_2;
					}
				}
			}
		    }

		/* deal with successful pick */
		if (rangemin < 10000000)
			{
			/* count the number of ties associated with the selected snav point */
			ntieselect = 0;
			for (i=0;i<project.num_crossings;i++)
			    {
			    crossing = &(project.crossings[i]);

			    /* loop over all ties for this crossing */
			    for (j=0;j<crossing->num_ties;j++)
		    		{
				tie = &(crossing->ties[j]);

				/* handle first snav point */
				file = &project.files[crossing->file_id_1];
				section = &file->sections[crossing->section_1];
				if (pick_file == crossing->file_id_1
					&& pick_section == crossing->section_1
					&& pick_snav == tie->snav_1)
					ntieselect++;

				/* handle second snav point */
				file = &project.files[crossing->file_id_2];
				section = &file->sections[crossing->section_2];
				if (pick_file == crossing->file_id_2
					&& pick_section == crossing->section_2
					&& pick_snav == tie->snav_2)
					ntieselect++;
				}
			    }

			/* if only one tie go ahead and select it and open it in naverr */
			if (ntieselect == 1)
				{
			    	mbna_crossing_select = pick_crossing;
			    	mbna_tie_select = pick_tie;
				mbna_modelplot_pickfile = MBNA_SELECT_NONE;
				mbna_modelplot_picksection = MBNA_SELECT_NONE;
				mbna_modelplot_picksnav = MBNA_SELECT_NONE;

				/* bring up naverr window if required */
				if (mbna_naverr_load == MB_NO)
				    {
				    do_naverr_init();
				    }

				/* else if naverr window is up, load selected crossing */
				else
				    {
				    mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
				    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
				    do_update_naverr();
				    do_update_status();
				    }
				}

			/* else if multiple ties */
			else if (ntieselect > 1)
				{
				mbna_modelplot_pickfile = pick_file;
				mbna_modelplot_picksection = pick_section;
				mbna_modelplot_picksnav = pick_snav;

				}
			}
		}

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/

int
mbnavadjust_modelplot_pick_perturbation(int x, int y)
{
	/* local variables */
	char	*function_name = "mbnavadjust_modelplot_pick_perturbation";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	range;
	int	rangemin;
	int	pick_crossing;
	int	pick_tie;
	int	pick_file;
	int	pick_section;
	int	pick_snav;
	int	ntieselect;
	int	i, j, ix, iy, iping;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       x:           %d\n",x);
		fprintf(stderr,"dbg2       y:           %d\n",y);
		}

	/* find nearest snav pt with tie */
    	if (project.open == MB_YES
    		&& project.inversion != MBNA_INVERSION_NONE
		&& project.modelplot == MB_YES)
    		{
		rangemin = 10000000;
fprintf(stderr,"mbnavadjust_modelplot_pick_perturbation: %d %d\n",x,y);
		/* search by looping over crossings */
		for (i=0;i<project.num_crossings;i++)
		    {
		    crossing = &(project.crossings[i]);

		    /* loop over all ties for this crossing */
		    for (j=0;j<crossing->num_ties;j++)
		    	{
			tie = &(crossing->ties[j]);

			/* handle first snav point */
			file = &project.files[crossing->file_id_1];
			section = &file->sections[crossing->section_1];

			if (section->show_in_modelplot == MB_YES)
				{
				iping = section->modelplot_start_count + section->snav_id[tie->snav_1];
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon - file->block_offset_x));
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_1;
					pick_section = crossing->section_1;
					pick_snav = tie->snav_1;
					}

				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat - file->block_offset_y));
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_1;
					pick_section = crossing->section_1;
					pick_snav = tie->snav_1;
					}

				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_1] - file->block_offset_z));
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_1;
					pick_section = crossing->section_1;
					pick_snav = tie->snav_1;
					}
				}

			/* handle second snav point */
			file = &project.files[crossing->file_id_2];
			section = &file->sections[crossing->section_2];

			if (section->show_in_modelplot == MB_YES)
				{
			iping = section->modelplot_start_count + section->snav_id[tie->snav_2];
			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon - file->block_offset_x));
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_2;
					pick_section = crossing->section_2;
					pick_snav = tie->snav_2;
					}

				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat - file->block_offset_y));
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_2;
					pick_section = crossing->section_2;
					pick_snav = tie->snav_2;
					}

				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_2] - file->block_offset_z));
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_2;
					pick_section = crossing->section_2;
					pick_snav = tie->snav_2;
					}
				}
			}
		    }

		/* deal with successful pick */
		if (rangemin < 10000000)
			{
			/* count the number of ties associated with the selected snav point */
			ntieselect = 0;
			for (i=0;i<project.num_crossings;i++)
			    {
			    crossing = &(project.crossings[i]);

			    /* loop over all ties for this crossing */
			    for (j=0;j<crossing->num_ties;j++)
		    		{
				tie = &(crossing->ties[j]);

				/* handle first snav point */
				file = &project.files[crossing->file_id_1];
				section = &file->sections[crossing->section_1];
				if (pick_file == crossing->file_id_1
					&& pick_section == crossing->section_1
					&& pick_snav == tie->snav_1)
					ntieselect++;

				/* handle second snav point */
				file = &project.files[crossing->file_id_2];
				section = &file->sections[crossing->section_2];
				if (pick_file == crossing->file_id_2
					&& pick_section == crossing->section_2
					&& pick_snav == tie->snav_2)
					ntieselect++;
				}
			    }

			/* if only one tie go ahead and select it and open it in naverr */
			if (ntieselect == 1)
				{
			    	mbna_crossing_select = pick_crossing;
			    	mbna_tie_select = pick_tie;
				mbna_modelplot_pickfile = MBNA_SELECT_NONE;
				mbna_modelplot_picksection = MBNA_SELECT_NONE;
				mbna_modelplot_picksnav = MBNA_SELECT_NONE;

				/* bring up naverr window if required */
				if (mbna_naverr_load == MB_NO)
				    {
				    do_naverr_init();
				    }

				/* else if naverr window is up, load selected crossing */
				else
				    {
				    mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
				    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
				    do_update_naverr();
				    do_update_status();
				    }
				}

			/* else if multiple ties */
			else if (ntieselect > 1)
				{
				mbna_modelplot_pickfile = pick_file;
				mbna_modelplot_picksection = pick_section;
				mbna_modelplot_picksnav = pick_snav;

				}
			}
		}

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/

int
mbnavadjust_modelplot_pick_tieoffsets(int x, int y)
{
	/* local variables */
	char	*function_name = "mbnavadjust_modelplot_pick_tieoffsets";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	range;
	int	rangemin;
	int	pick_crossing;
	int	pick_tie;
	int	pick_file;
	int	pick_section;
	int	pick_snav;
	int	i, j, ix, iy;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       x:           %d\n",x);
		fprintf(stderr,"dbg2       y:           %d\n",y);
		}

	/* find nearest snav pt with tie */
    	if (project.open == MB_YES
    		&& project.inversion != MBNA_INVERSION_NONE
		&& project.modelplot == MB_YES)
    		{
		rangemin = 10000000;

		/* search by looping over crossings */
		for (i=0;i<project.num_crossings;i++)
		    {
		    crossing = &(project.crossings[i]);

		    /* loop over all ties for this crossing */
		    for (j=0;j<crossing->num_ties;j++)
		    	{
			tie = &(crossing->ties[j]);

			/* handle first snav point */
			file = &project.files[crossing->file_id_1];
			section = &file->sections[crossing->section_1];

			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (tie->isurveyplotindex - mbna_modelplot_tiestart));

			iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * tie->offset_x_m);
			range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
			if (range < rangemin)
				{
				rangemin = range;
				pick_crossing = i;
				pick_tie = j;
				pick_file = crossing->file_id_1;
				pick_section = crossing->section_1;
				pick_snav = tie->snav_1;
				}

			iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale  * tie->offset_y_m);
			range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
			if (range < rangemin)
				{
				rangemin = range;
				pick_crossing = i;
				pick_tie = j;
				pick_file = crossing->file_id_1;
				pick_section = crossing->section_1;
				pick_snav = tie->snav_1;
				}

			iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale  * tie->offset_z_m);
			range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
			if (range < rangemin)
				{
				rangemin = range;
				pick_crossing = i;
				pick_tie = j;
				pick_file = crossing->file_id_1;
				pick_section = crossing->section_1;
				pick_snav = tie->snav_1;
				}
			}
		    }

		/* deal with successful pick */
		if (rangemin < 10000000)
			{
			mbna_crossing_select = pick_crossing;
			mbna_tie_select = pick_tie;
			/* mbna_modelplot_pickfile = MBNA_SELECT_NONE; */
			mbna_modelplot_picksection = MBNA_SELECT_NONE;
			mbna_modelplot_picksnav = MBNA_SELECT_NONE;

			/* bring up naverr window if required */
			if (mbna_naverr_load == MB_NO)
			    {
			    do_naverr_init();
			    }

			/* else if naverr window is up, load selected crossing */
			else
			    {
			    mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
			    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
			    do_update_naverr();
			    do_update_status();
			    }
			}
		}


	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/

int
mbnavadjust_modelplot_middlepick(int x, int y)
{
	/* local variables */
	char	*function_name = "mbnavadjust_modelplot_middlepick";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	range;
	int	rangemin;
	int	pick_crossing;
	int	pick_tie;
	int	pick_file;
	int	pick_section;
	int	pick_snav;
	int	i, j, ix, iy, iping;

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       x:           %d\n",x);
		fprintf(stderr,"dbg2       y:           %d\n",y);
		}

	/* handle middle button pick */
    	if (project.open == MB_YES
    		&& project.inversion != MBNA_INVERSION_NONE
		&& project.modelplot == MB_YES)
		{
		/* middle pick for timeseries plot is either choosing one of multiple available
			ties from a tied crossing (left button) pick, or if that is not the
			situation, picking the nearest untied crossing */
		if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES)
			{
			/* first snav pick had multiple ties - now pick which one to use */
			if (mbna_modelplot_pickfile != MBNA_SELECT_NONE)
			    {
			    rangemin = 10000000;

			    for (i=0;i<project.num_crossings;i++)
				{
				/* check if this crossing includes the picked snav */
				crossing = &(project.crossings[i]);

				/* check first snav */
				if (crossing->file_id_1 == mbna_modelplot_pickfile
					&& crossing->section_1 == mbna_modelplot_picksection)
				    {
				    /* loop over the ties */
				    for (j=0;j<crossing->num_ties;j++)
					{
					tie = &(crossing->ties[j]);
					if (tie->snav_1 == mbna_modelplot_picksnav)
					    {
					    file = &project.files[crossing->file_id_2];
					    section = &file->sections[crossing->section_2];
					    iping = section->modelplot_start_count + section->snav_id[tie->snav_2];
					    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

					    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon);
					    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
					    if (range < rangemin)
						    {
						    rangemin = range;
						    pick_crossing = i;
						    pick_tie = j;
						    pick_file = crossing->file_id_2;
						    pick_section = crossing->section_2;
						    pick_snav = tie->snav_2;
						    }
	/*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

					    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat);
					    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
					    if (range < rangemin)
						    {
						    rangemin = range;
						    pick_crossing = i;
						    pick_tie = j;
						    pick_file = crossing->file_id_2;
						    pick_section = crossing->section_2;
						    pick_snav = tie->snav_2;
						    }
	    /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

					    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
					    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
					    if (range < rangemin)
						    {
						    rangemin = range;
						    pick_crossing = i;
						    pick_tie = j;
						    pick_file = crossing->file_id_2;
						    pick_section = crossing->section_2;
						    pick_snav = tie->snav_2;
						    }
	    /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/
					    }
					}
				    }

				/* check second snav */
				if (crossing->file_id_2 == mbna_modelplot_pickfile
					&& crossing->section_2 == mbna_modelplot_picksection)
				    {
				    /* loop over the ties */
				    for (j=0;j<crossing->num_ties;j++)
					{
					tie = &(crossing->ties[j]);
					if (tie->snav_2 == mbna_modelplot_picksnav)
					    {
					    file = &project.files[crossing->file_id_1];
					    section = &file->sections[crossing->section_1];
					    iping = section->modelplot_start_count + section->snav_id[tie->snav_1];
					    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

					    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon);
					    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
					    if (range < rangemin)
						    {
						    rangemin = range;
						    pick_crossing = i;
						    pick_tie = j;
						    pick_file = crossing->file_id_1;
						    pick_section = crossing->section_1;
						    pick_snav = tie->snav_1;
						    }
	    /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

					    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat);
					    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
					    if (range < rangemin)
						    {
						    rangemin = range;
						    pick_crossing = i;
						    pick_tie = j;
						    pick_file = crossing->file_id_1;
						    pick_section = crossing->section_1;
						    pick_snav = tie->snav_1;
						    }
	    /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

					    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
					    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
					    if (range < rangemin)
						    {
						    rangemin = range;
						    pick_crossing = i;
						    pick_tie = j;
						    pick_file = crossing->file_id_1;
						    pick_section = crossing->section_1;
						    pick_snav = tie->snav_1;
						    }
	/*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/
					    }
					}
				    }
				}

			    /* deal with successful pick */
			    if (rangemin < 10000000)
				{
				/* select tie and open it in naverr */
				mbna_crossing_select = pick_crossing;
				mbna_tie_select = pick_tie;
				mbna_modelplot_pickfile = MBNA_SELECT_NONE;
				mbna_modelplot_picksection = MBNA_SELECT_NONE;
				mbna_modelplot_picksnav = MBNA_SELECT_NONE;

				/* bring up naverr window if required */
				if (mbna_naverr_load == MB_NO)
				    {
				    do_naverr_init();
				    }

				/* else if naverr window is up, load selected crossing */
				else
				    {
				    mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
				    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
				    do_update_naverr();
				    do_update_status();
				    }
				}
			    }

			/* else pick closest untied crossing */
			else
			    {
			    rangemin = 10000000;

			    /* search by looping over crossings */
			    for (i=0;i<project.num_crossings;i++)
				{
				crossing = &(project.crossings[i]);

				/* check only untied crossings */
				if (crossing->num_ties == 0)
				    {
				    file = &project.files[crossing->file_id_1];
				    section = &file->sections[crossing->section_1];

				    iping = section->modelplot_start_count + section->snav_id[section->num_snav/2];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

				    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[section->num_snav/2] / mbna_mtodeglon);
				    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				    if (range < rangemin)
					    {
					    rangemin = range;
					    pick_crossing = i;
					    pick_tie = MBNA_SELECT_NONE;
					    pick_file = crossing->file_id_1;
					    pick_section = crossing->section_1;
					    pick_snav = section->num_snav/2;
					    }

				    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[section->num_snav/2] / mbna_mtodeglat);
				    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				    if (range < rangemin)
					    {
					    rangemin = range;
					    pick_crossing = i;
					    pick_tie = MBNA_SELECT_NONE;
					    pick_file = crossing->file_id_1;
					    pick_section = crossing->section_1;
					    pick_snav = section->num_snav/2;
					    }

				    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[section->num_snav/2]);
				    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				    if (range < rangemin)
					    {
					    rangemin = range;
					    pick_crossing = i;
					    pick_tie = MBNA_SELECT_NONE;
					    pick_file = crossing->file_id_1;
					    pick_section = crossing->section_1;
					    pick_snav = section->num_snav/2;
					    }

				    /* handle second snav point */
				    file = &project.files[crossing->file_id_2];
				    section = &file->sections[crossing->section_2];

				    iping = section->modelplot_start_count + section->snav_id[section->num_snav/2];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

				    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[section->num_snav/2] / mbna_mtodeglon);
				    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				    if (range < rangemin)
					    {
					    rangemin = range;
					    pick_crossing = i;
					    pick_tie = MBNA_SELECT_NONE;
					    pick_file = crossing->file_id_2;
					    pick_section = crossing->section_2;
					    pick_snav = section->num_snav/2;
					    }

				    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[section->num_snav/2] / mbna_mtodeglat);
				    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				    if (range < rangemin)
					    {
					    rangemin = range;
					    pick_crossing = i;
					    pick_tie = MBNA_SELECT_NONE;
					    pick_file = crossing->file_id_2;
					    pick_section = crossing->section_2;
					    pick_snav = section->num_snav/2;
					    }

				    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[section->num_snav/2]);
				    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				    if (range < rangemin)
					    {
					    rangemin = range;
					    pick_crossing = i;
					    pick_tie = MBNA_SELECT_NONE;
					    pick_file = crossing->file_id_2;
					    pick_section = crossing->section_2;
					    pick_snav = section->num_snav/2;
					    }
				    }
				}

			    /* deal with successful pick */
			    if (rangemin < 10000000)
				{
				mbna_crossing_select = pick_crossing;
				mbna_tie_select = MBNA_SELECT_NONE;
				mbna_modelplot_pickfile = MBNA_SELECT_NONE;
				mbna_modelplot_picksection = MBNA_SELECT_NONE;
				mbna_modelplot_picksnav = MBNA_SELECT_NONE;

				/* bring up naverr window if required */
				if (mbna_naverr_load == MB_NO)
				    {
				    do_naverr_init();
				    }

				/* else if naverr window is up, load selected crossing */
				else
				    {
				    mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
				    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
				    do_update_naverr();
				    do_update_status();
				    }
				}
			    }
			}

		/* middle pick for perturbation plot is either choosing one of multiple available
			ties from a tied crossing (left button) pick, or if that is not the
			situation, picking the nearest untied crossing */
		else if (project.modelplot_style == MBNA_MODELPLOT_PERTURBATION)
			{
			/* first snav pick had multiple ties - now pick which one to use */
			if (mbna_modelplot_pickfile != MBNA_SELECT_NONE)
			    {
			    rangemin = 10000000;

			    for (i=0;i<project.num_crossings;i++)
				{
				/* check if this crossing includes the picked snav */
				crossing = &(project.crossings[i]);

				/* check first snav */
				if (crossing->file_id_1 == mbna_modelplot_pickfile
					&& crossing->section_1 == mbna_modelplot_picksection)
				    {
				    /* loop over the ties */
				    for (j=0;j<crossing->num_ties;j++)
					{
					tie = &(crossing->ties[j]);
					if (tie->snav_1 == mbna_modelplot_picksnav)
					    {
					    file = &project.files[crossing->file_id_2];
					    section = &file->sections[crossing->section_2];
					    iping = section->modelplot_start_count + section->snav_id[tie->snav_2];
					    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

					    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon - file->block_offset_x));
					    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
					    if (range < rangemin)
						    {
						    rangemin = range;
						    pick_crossing = i;
						    pick_tie = j;
						    pick_file = crossing->file_id_2;
						    pick_section = crossing->section_2;
						    pick_snav = tie->snav_2;
						    }
	/*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

					    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat - file->block_offset_y));
					    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
					    if (range < rangemin)
						    {
						    rangemin = range;
						    pick_crossing = i;
						    pick_tie = j;
						    pick_file = crossing->file_id_2;
						    pick_section = crossing->section_2;
						    pick_snav = tie->snav_2;
						    }
	    /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

					    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_2] - file->block_offset_z));
					    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
					    if (range < rangemin)
						    {
						    rangemin = range;
						    pick_crossing = i;
						    pick_tie = j;
						    pick_file = crossing->file_id_2;
						    pick_section = crossing->section_2;
						    pick_snav = tie->snav_2;
						    }
	    /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/
					    }
					}
				    }

				/* check second snav */
				if (crossing->file_id_2 == mbna_modelplot_pickfile
					&& crossing->section_2 == mbna_modelplot_picksection)
				    {
				    /* loop over the ties */
				    for (j=0;j<crossing->num_ties;j++)
					{
					tie = &(crossing->ties[j]);
					if (tie->snav_2 == mbna_modelplot_picksnav)
					    {
					    file = &project.files[crossing->file_id_1];
					    section = &file->sections[crossing->section_1];
					    iping = section->modelplot_start_count + section->snav_id[tie->snav_1];
					    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

					    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon - file->block_offset_x));
					    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
					    if (range < rangemin)
						    {
						    rangemin = range;
						    pick_crossing = i;
						    pick_tie = j;
						    pick_file = crossing->file_id_1;
						    pick_section = crossing->section_1;
						    pick_snav = tie->snav_1;
						    }
	    /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

					    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat - file->block_offset_y));
					    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
					    if (range < rangemin)
						    {
						    rangemin = range;
						    pick_crossing = i;
						    pick_tie = j;
						    pick_file = crossing->file_id_1;
						    pick_section = crossing->section_1;
						    pick_snav = tie->snav_1;
						    }
	    /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

					    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_1] - file->block_offset_z));
					    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
					    if (range < rangemin)
						    {
						    rangemin = range;
						    pick_crossing = i;
						    pick_tie = j;
						    pick_file = crossing->file_id_1;
						    pick_section = crossing->section_1;
						    pick_snav = tie->snav_1;
						    }
	/*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/
					    }
					}
				    }
				}

			    /* deal with successful pick */
			    if (rangemin < 10000000)
				{
				/* select tie and open it in naverr */
				mbna_crossing_select = pick_crossing;
				mbna_tie_select = pick_tie;
				mbna_modelplot_pickfile = MBNA_SELECT_NONE;
				mbna_modelplot_picksection = MBNA_SELECT_NONE;
				mbna_modelplot_picksnav = MBNA_SELECT_NONE;

				/* bring up naverr window if required */
				if (mbna_naverr_load == MB_NO)
				    {
				    do_naverr_init();
				    }

				/* else if naverr window is up, load selected crossing */
				else
				    {
				    mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
				    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
				    do_update_naverr();
				    do_update_status();
				    }
				}
			    }

			/* else pick closest untied crossing */
			else
			    {
			    rangemin = 10000000;

			    /* search by looping over crossings */
			    for (i=0;i<project.num_crossings;i++)
				{
				crossing = &(project.crossings[i]);

				/* check only untied crossings */
				if (crossing->num_ties == 0)
				    {
				    file = &project.files[crossing->file_id_1];
				    section = &file->sections[crossing->section_1];

				    iping = section->modelplot_start_count + section->snav_id[section->num_snav/2];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

				    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[section->num_snav/2] / mbna_mtodeglon - file->block_offset_x));
				    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				    if (range < rangemin)
					    {
					    rangemin = range;
					    pick_crossing = i;
					    pick_tie = MBNA_SELECT_NONE;
					    pick_file = crossing->file_id_1;
					    pick_section = crossing->section_1;
					    pick_snav = section->num_snav/2;
					    }

				    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[section->num_snav/2] / mbna_mtodeglat - file->block_offset_y));
				    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				    if (range < rangemin)
					    {
					    rangemin = range;
					    pick_crossing = i;
					    pick_tie = MBNA_SELECT_NONE;
					    pick_file = crossing->file_id_1;
					    pick_section = crossing->section_1;
					    pick_snav = section->num_snav/2;
					    }

				    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[section->num_snav/2] - file->block_offset_z));
				    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				    if (range < rangemin)
					    {
					    rangemin = range;
					    pick_crossing = i;
					    pick_tie = MBNA_SELECT_NONE;
					    pick_file = crossing->file_id_1;
					    pick_section = crossing->section_1;
					    pick_snav = section->num_snav/2;
					    }

				    /* handle second snav point */
				    file = &project.files[crossing->file_id_2];
				    section = &file->sections[crossing->section_2];

				    iping = section->modelplot_start_count + section->snav_id[section->num_snav/2];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

				    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[section->num_snav/2] / mbna_mtodeglon - file->block_offset_x));
				    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				    if (range < rangemin)
					    {
					    rangemin = range;
					    pick_crossing = i;
					    pick_tie = MBNA_SELECT_NONE;
					    pick_file = crossing->file_id_2;
					    pick_section = crossing->section_2;
					    pick_snav = section->num_snav/2;
					    }

				    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[section->num_snav/2] / mbna_mtodeglat - file->block_offset_y));
				    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				    if (range < rangemin)
					    {
					    rangemin = range;
					    pick_crossing = i;
					    pick_tie = MBNA_SELECT_NONE;
					    pick_file = crossing->file_id_2;
					    pick_section = crossing->section_2;
					    pick_snav = section->num_snav/2;
					    }

				    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[section->num_snav/2] - file->block_offset_z));
				    range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				    if (range < rangemin)
					    {
					    rangemin = range;
					    pick_crossing = i;
					    pick_tie = MBNA_SELECT_NONE;
					    pick_file = crossing->file_id_2;
					    pick_section = crossing->section_2;
					    pick_snav = section->num_snav/2;
					    }
				    }
				}

			    /* deal with successful pick */
			    if (rangemin < 10000000)
				{
				mbna_crossing_select = pick_crossing;
				mbna_tie_select = MBNA_SELECT_NONE;
				mbna_modelplot_pickfile = MBNA_SELECT_NONE;
				mbna_modelplot_picksection = MBNA_SELECT_NONE;
				mbna_modelplot_picksnav = MBNA_SELECT_NONE;

				/* bring up naverr window if required */
				if (mbna_naverr_load == MB_NO)
				    {
				    do_naverr_init();
				    }

				/* else if naverr window is up, load selected crossing */
				else
				    {
				    mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
				    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
				    do_update_naverr();
				    do_update_status();
				    }
				}
			    }
			}

		/* middle pick for tie offsets plot is choosing which survey vs survey group (block)
			to plot by itself */
		else
			{
			rangemin = 10000000;

			/* search by looping over crossings */
			for (i=0;i<project.num_crossings;i++)
			    {
			    crossing = &(project.crossings[i]);

			    /* loop over all ties for this crossing */
			    for (j=0;j<crossing->num_ties;j++)
				{
				tie = &(crossing->ties[j]);

				/* handle first snav point */
				file = &project.files[crossing->file_id_1];
				section = &file->sections[crossing->section_1];

				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (tie->isurveyplotindex - mbna_modelplot_tiestart));

				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * tie->offset_x_m);
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_1;
					pick_section = crossing->section_1;
					pick_snav = tie->snav_1;
					}

				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale  * tie->offset_y_m);
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_1;
					pick_section = crossing->section_1;
					pick_snav = tie->snav_1;
					}

				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale  * tie->offset_z_m);
				range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
				if (range < rangemin)
					{
					rangemin = range;
					pick_crossing = i;
					pick_tie = j;
					pick_file = crossing->file_id_1;
					pick_section = crossing->section_1;
					pick_snav = tie->snav_1;
					}
				}
			    }

			/* deal with successful pick */
			if (rangemin < 10000000)
				{
				crossing = &(project.crossings[pick_crossing]);
				mbna_crossing_select = pick_crossing;
				mbna_tie_select = pick_tie;
				mbna_modelplot_pickfile = MBNA_SELECT_NONE;
				mbna_modelplot_picksection = MBNA_SELECT_NONE;
				mbna_modelplot_picksnav = MBNA_SELECT_NONE;
				mbna_modelplot_blocksurvey1 = project.files[crossing->file_id_1].block;
				mbna_modelplot_blocksurvey2 = project.files[crossing->file_id_2].block;
				mbna_modelplot_tiezoom = MB_NO;

				/* bring up naverr window if required */
				if (mbna_naverr_load == MB_NO)
				    {
				    do_naverr_init();
				    }

				/* else if naverr window is up, load selected crossing */
				else
				    {
				    mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
				    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
				    do_update_naverr();
				    do_update_status();
				    }
				}
			}
		}

	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/

int
mbnavadjust_modelplot_clearblock()
{
	/* local variables */
	char	*function_name = "mbnavadjust_modelplot_clearblock";
	int	status = MB_SUCCESS;
	struct mbna_crossing *crossing;
	int	block1, block2;
	int	i, j;
/*fprintf(stderr,"Called %s\n",function_name);*/

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* only proceed if model plot is active and a crossing is selected */
    	if (project.open == MB_YES
		&& project.modelplot == MB_YES
		&& mbna_current_crossing != MBNA_SELECT_NONE)
    		{
		/* delete all ties associated with the same pair of surveys as the currently selected crossing */
		crossing = &(project.crossings[mbna_current_crossing]);
		block1 = project.files[crossing->file_id_1].block;
		block2 = project.files[crossing->file_id_2].block;
		for (i=0;i<project.num_crossings;i++)
		    {
		    crossing = &(project.crossings[i]);
		    if (crossing->num_ties > 0
		    	&& ((project.files[crossing->file_id_1].block == block1
		    		&& project.files[crossing->file_id_2].block == block2)
			    || (project.files[crossing->file_id_1].block == block2
		    		&& project.files[crossing->file_id_2].block == block1)))
			{
			for (j=crossing->num_ties-1;j>=0;j--)
			    {
			    mbnavadjust_deletetie(i, j, MBNA_CROSSING_STATUS_NONE);
			    }
			}
		    }
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/

int
mbnavadjust_modelplot_plot()
{
	/* local variables */
	char	*function_name = "mbnavadjust_modelplot_plot";
	int	status = MB_SUCCESS;
/*fprintf(stderr,"Called %s\n",function_name);*/

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* plot model if an inversion has been performed */
    	if (project.open == MB_YES
    		&& project.inversion != MBNA_INVERSION_NONE
		&& project.modelplot == MB_YES)
    		{
		if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES)
			{
			mbnavadjust_modelplot_plot_timeseries();
			}
		else if (project.modelplot_style == MBNA_MODELPLOT_PERTURBATION)
			{
			mbnavadjust_modelplot_plot_perturbation();
			}
		else
			{
			mbnavadjust_modelplot_plot_tieoffsets();
			}
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/

int
mbnavadjust_modelplot_plot_timeseries()
{
	/* local variables */
	char	*function_name = "mbnavadjust_modelplot_plot_timeseries";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	double	lon_offset_min;
	double	lon_offset_max;
	double	lat_offset_min;
	double	lat_offset_max;
	double	z_offset_min;
	double	z_offset_max;
	double	xymax, yzmax;
	int	plot_width;
	int	plot_height;
	int	first, iping;
	char	label[STRING_MAX];
	int	stringwidth, stringascent, stringdescent;
	int	pixel;
	int	ixo, iyo, ix, iy;
	int	i, j, isnav;
	int	imodelplot_start, imodelplot_end;
//fprintf(stderr,"Called %s\n",function_name);

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* plot model if an inversion has been performed */
    	if (project.open == MB_YES
    		&& project.inversion != MBNA_INVERSION_NONE
		&& project.modelplot == MB_YES)
    		{
		/* first loop over files setting all plot flags off for both files and sections */
		first = MB_YES;
		mbna_modelplot_count = 0;
 		for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			file->show_in_modelplot = MB_NO;
			for (j=0;j<file->num_sections;j++)
				{
				section = &file->sections[j];
				section->show_in_modelplot = MB_NO;
				}
			}

		/* now loop over files setting file or section plot flags on as necessary */
 		for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];

			/* check if this file will be plotted */
			if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY || mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
				{
				if (file->block == mbna_survey_select)
					{
					file->show_in_modelplot = MB_YES;
					}
				}

			/* check if this file will be plotted */
			else if (mbna_view_mode == MBNA_VIEW_MODE_FILE || mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
				{
				if (i == mbna_file_select)
					{
					file->show_in_modelplot = MB_YES;
					}
				}

			/* check if each section in this file will be plotted */
			else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
				{
				for (j=0;j<file->num_sections;j++)
					{
					section = &file->sections[j];
					if (i == mbna_file_select && j == mbna_section_select)
						{
						section->show_in_modelplot = MB_YES;
						}
					}
				}

			/* else every file will be plotted */
			else if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
				{
				file->show_in_modelplot = MB_YES;
				}
			}

		/* if view mode is with survey loop over all crossings */
		if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
			{
			for (i=0;i<project.num_crossings;i++)
				{
				crossing = &(project.crossings[i]);

				/* if either file is part of the selected survey
					then set plot flags on for both files */
				if (project.files[crossing->file_id_1].block == mbna_survey_select
				    || project.files[crossing->file_id_2].block == mbna_survey_select)
					{
					project.files[crossing->file_id_1].show_in_modelplot = MB_YES;
					project.files[crossing->file_id_2].show_in_modelplot = MB_YES;
					}
				}
			}

		/* else if view mode is with file loop over all crossings */
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
			{
			for (i=0;i<project.num_crossings;i++)
				{
				crossing = &(project.crossings[i]);

				/* if either file is selected
					then set plot flags on for both files */
				if (crossing->file_id_1 == mbna_file_select || crossing->file_id_2 == mbna_file_select)
					{
					project.files[crossing->file_id_1].show_in_modelplot = MB_YES;
					project.files[crossing->file_id_2].show_in_modelplot = MB_YES;
					}
				}
			}

		/* else if view mode is with section loop over all crossings */
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
			{
			for (i=0;i<project.num_crossings;i++)
				{
				crossing = &(project.crossings[i]);

				/* if either section is selected
					then set plot flags on for both files */
				if ((crossing->file_id_1 == mbna_file_select && crossing->section_1 == mbna_section_select)
				    || (crossing->file_id_2 == mbna_file_select && crossing->section_2 == mbna_section_select))
					{
					project.files[crossing->file_id_1].show_in_modelplot = MB_YES;
					project.files[crossing->file_id_2].show_in_modelplot = MB_YES;
					}
				}
			}

		/* finally loop over files again setting all section plot flags on for files with plot flags on */
 		for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			if (file->show_in_modelplot == MB_YES)
				{
				for (j=0;j<file->num_sections;j++)
					{
					section = &file->sections[j];
					section->show_in_modelplot = MB_YES;
					}
				}
			}


		/* get min maxes by looping over files and sections checking for sections to be plotted */
		first = MB_YES;
		mbna_modelplot_count = 0;
 		for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			for (j=0;j<file->num_sections;j++)
				{
				section = &file->sections[j];

				/* if this section will be plotted use the snav values */
				if (section->show_in_modelplot == MB_YES)
					{
					section->modelplot_start_count = mbna_modelplot_count;
					for (isnav=0;isnav<section->num_snav;isnav++)
						{
						if (mbna_modelplot_zoom == MB_NO
						    || (mbna_modelplot_count >= mbna_modelplot_startzoom && mbna_modelplot_count <= mbna_modelplot_endzoom))
						    {
							if (first == MB_YES)
								{
								lon_offset_min = section->snav_lon_offset[isnav] / mbna_mtodeglon;
								lon_offset_max = section->snav_lon_offset[isnav] / mbna_mtodeglon;
								lat_offset_min = section->snav_lat_offset[isnav] / mbna_mtodeglat;
								lat_offset_max = section->snav_lat_offset[isnav] / mbna_mtodeglat;
								z_offset_min = section->snav_z_offset[isnav];
								z_offset_max = section->snav_z_offset[isnav];
								first = MB_NO;
								}
							else
								{
								lon_offset_min = MIN(lon_offset_min, section->snav_lon_offset[isnav] / mbna_mtodeglon);
								lon_offset_max = MAX(lon_offset_max, section->snav_lon_offset[isnav] / mbna_mtodeglon);
								lat_offset_min = MIN(lat_offset_min, section->snav_lat_offset[isnav] / mbna_mtodeglat);
								lat_offset_max = MAX(lat_offset_max, section->snav_lat_offset[isnav] / mbna_mtodeglat);
								z_offset_min = MIN(z_offset_min, section->snav_z_offset[isnav]);
								z_offset_max = MAX(z_offset_max, section->snav_z_offset[isnav]);
								}
							}
						}
					mbna_modelplot_count += section->snav_id[section->num_snav-1];
					}
				}
			}

		/* set plot bounds */
		if (mbna_modelplot_zoom == MB_YES)
			{
			mbna_modelplot_start = mbna_modelplot_startzoom;
			mbna_modelplot_end = mbna_modelplot_endzoom;
			}
		else
			{
			mbna_modelplot_start = 0;
			mbna_modelplot_end = mbna_modelplot_count - 1;
			}

		/* get scaling */
		plot_width = mbna_modelplot_width - 8 * MBNA_MODELPLOT_X_SPACE;
		plot_height = (mbna_modelplot_height - 4 * MBNA_MODELPLOT_Y_SPACE) / 3;
		mbna_modelplot_xo = 5 * MBNA_MODELPLOT_X_SPACE;
		mbna_modelplot_yo_lon = MBNA_MODELPLOT_Y_SPACE + plot_height / 2;
		mbna_modelplot_yo_lat = 2 * MBNA_MODELPLOT_Y_SPACE + 3 * plot_height / 2;
		mbna_modelplot_yo_z = 3 * MBNA_MODELPLOT_Y_SPACE + 5 * plot_height / 2;
		xymax = MAX(fabs(lon_offset_min),fabs(lon_offset_max));
		xymax = MAX(fabs(lat_offset_min),xymax);
		xymax = MAX(fabs(lat_offset_max),xymax);
		mbna_modelplot_xscale = ((double)plot_width) / (mbna_modelplot_end - mbna_modelplot_start + 1);
		mbna_modelplot_yscale = ((double)plot_height) / (2.2 * xymax);
		yzmax = MAX(fabs(z_offset_min),fabs(z_offset_max));
		yzmax = MAX(yzmax,0.5);
		mbna_modelplot_yzscale = ((double)plot_height) / (2.2 * yzmax);

		/* clear screens for first plot */
		xg_fillrectangle(pmodp_xgid, 0, 0,
				modp_borders[1], modp_borders[3],
				pixel_values[mbna_color_background], XG_SOLIDLINE);

		/* plot the bounds */
		xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lon - plot_height / 2, plot_width, plot_height, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lon, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_lon, pixel_values[mbna_color_foreground], XG_DASHLINE);
		xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lat - plot_height / 2, plot_width, plot_height, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lat, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_lat, pixel_values[mbna_color_foreground], XG_DASHLINE);
		xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_z - plot_height / 2, plot_width, plot_height, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_z, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_z, pixel_values[mbna_color_foreground], XG_DASHLINE);

		/* plot title */
		if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
			{
			sprintf(label, "Display Only Selected Survey - Selected Survey:%d", mbna_survey_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
			{
			sprintf(label, "Display Only Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
			{
			sprintf(label, "Display With Selected Survey - Selected Survey:%d", mbna_survey_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
			{
			sprintf(label, "Display With Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
			{
			sprintf(label, "Display With Selected Section: Selected Survey/File/Section:%d/%d/%d",
				mbna_survey_select, mbna_file_select, mbna_section_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
			{
			sprintf(label, "Display All Data");
			}

		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = MBNA_MODELPLOT_Y_SPACE - 2 * stringascent;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		/* plot labels */
		sprintf(label, "East-West Offset (meters) vs. Ping Count");
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = mbna_modelplot_yo_lon - plot_height / 2 - stringascent / 4;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_start);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth / 2;
		iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_end);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
		iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lon - plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 0.0);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lon + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", -1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lon + plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);


		sprintf(label, "North-South Offset (meters) vs. Ping Count");
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = mbna_modelplot_yo_lat - plot_height / 2 - stringascent / 4;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_start);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth / 2;
		iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_end);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
		iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lat - plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 0.0);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lat + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", -1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lat + plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);


		sprintf(label, "Vertical Offset (meters) vs. Ping Count");
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = mbna_modelplot_yo_z - plot_height / 2 - stringascent / 4;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_start);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth / 2;
		iy = mbna_modelplot_yo_z + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_end);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
		iy = mbna_modelplot_yo_z + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 1.1 * yzmax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_z - plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 0.0);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_z + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", -1.1 * yzmax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_z + plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		/* set clipping */
		xg_setclip(pmodp_xgid, mbna_modelplot_xo, 0, plot_width, mbna_modelplot_height);

		/* loop over all crossings and plot and plot those without ties in green */
		for (i=0;i<project.num_crossings;i++)
		    {
		    crossing = &(project.crossings[i]);
		    if (crossing->num_ties== 0)
		    	{
			file = &project.files[crossing->file_id_1];
			section = &file->sections[crossing->section_1];
			iping = section->modelplot_start_count + section->snav_id[section->num_snav/2];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[section->num_snav/2] / mbna_mtodeglon);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[section->num_snav/2] / mbna_mtodeglat);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[section->num_snav/2]);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
				}

			file = &project.files[crossing->file_id_2];
			section = &file->sections[crossing->section_2];
			iping = section->modelplot_start_count + section->snav_id[section->num_snav/2];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[section->num_snav/2] / mbna_mtodeglon);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[section->num_snav/2] / mbna_mtodeglat);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[section->num_snav/2]);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
				}
			}
		    }

		/* Now plot the east-west offsets */
		ixo = 0;
		iyo = 0;
 		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			if (section->show_in_modelplot == MB_YES)
				{
				for (isnav=0;isnav<section->num_snav;isnav++)
				    {
				    iping = section->modelplot_start_count + section->snav_id[isnav];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[isnav] / mbna_mtodeglon);
				    if ((i > 0 || j > 0) && section->continuity == MB_NO && isnav == 0)
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2, pixel_values[GREEN], XG_SOLIDLINE);
				    else if (i > 0 || j > 0)
					{
					/* if (j == 0 && isnav == 0 && section->continuity == MB_YES)
						xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2, pixel_values[CORAL], XG_DASHLINE); */
					xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
					}
				    ixo = ix;
				    iyo = iy;
				    }
				}
			}
		    }

		/* Now plot the north-south offsets */
		ixo = 0;
		iyo = 0;
 		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			if (section->show_in_modelplot == MB_YES)
				{
				for (isnav=0;isnav<section->num_snav;isnav++)
				    {
				    iping = section->modelplot_start_count + section->snav_id[isnav];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[isnav] / mbna_mtodeglat);
				    if ((i > 0 || j > 0) && section->continuity == MB_NO && isnav == 0)
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2, pixel_values[GREEN], XG_SOLIDLINE);
				    else if (i > 0 || j > 0)
					{
					/* if (j == 0 && isnav == 0 && section->continuity == MB_YES)
						xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2, pixel_values[CORAL], XG_DASHLINE); */
					xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
					}
				    ixo = ix;
				    iyo = iy;
				    }
				}
			}
		    }

		/* Now plot the vertical offsets */
		ixo = 0;
		iyo = 0;
 		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			if (section->show_in_modelplot == MB_YES)
				{
				for (isnav=0;isnav<section->num_snav;isnav++)
				    {
				    iping = section->modelplot_start_count + section->snav_id[isnav];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[isnav]);
				    if ((i > 0 || j > 0) && section->continuity == MB_NO && isnav == 0)
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2, pixel_values[GREEN], XG_SOLIDLINE);
				    else if (i > 0 || j > 0)
					{
					/* if (j == 0 && isnav == 0 && section->continuity == MB_YES)
						xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2, pixel_values[CORAL], XG_DASHLINE); */
					xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
					}
				    ixo = ix;
				    iyo = iy;
				    }
				}
			}
		    }

		/* loop over all crossings and plot ties */
		for (i=0;i<project.num_crossings;i++)
		    {
		    crossing = &(project.crossings[i]);
		    for (j=0;j<crossing->num_ties;j++)
		    	{
			tie = &(crossing->ties[j]);

			if (tie->inversion_status == MBNA_INVERSION_CURRENT)
				pixel = pixel_values[mbna_color_foreground];
			else
				pixel = pixel_values[BLUE];

			file = &project.files[crossing->file_id_1];
			section = &file->sections[crossing->section_1];
			iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon);
				xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat);
				xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
				xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
				}

			file = &project.files[crossing->file_id_2];
			section = &file->sections[crossing->section_2];
			iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon);
				xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat);
				xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
				xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
				}
			}
		    }

		/* Loop over all files plotting global ties */
 		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			if (section->show_in_modelplot == MB_YES && section->global_tie_status != MBNA_TIE_NONE)
				{
				for (isnav=0;isnav<section->num_snav;isnav++)
				    {
				    iping = section->modelplot_start_count + section->snav_id[section->global_tie_snav];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				    
				    if (section->global_tie_status != MBNA_TIE_Z)
					{
					/* east-west offsets */
					iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[section->global_tie_snav] / mbna_mtodeglon);
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
					xg_fillrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
					xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
					
					/* north-south offsets */
					iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[section->global_tie_snav] / mbna_mtodeglat);
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
					xg_fillrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
					xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
					}
				    
				    if (section->global_tie_status != MBNA_TIE_XY)
					{
					/* vertical offsets */
					iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[section->global_tie_snav]);
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
					xg_fillrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
					xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
					}
				    }
				}
			}
		    }

		/* plot current tie in red */
		if (mbna_current_crossing != MBNA_SELECT_NONE && mbna_current_tie != MBNA_SELECT_NONE)
			{
			crossing = &(project.crossings[mbna_current_crossing]);
			tie = &(crossing->ties[mbna_current_tie]);

			file = &project.files[crossing->file_id_1];
			section = &file->sections[crossing->section_1];
			iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				}

			file = &project.files[crossing->file_id_2];
			section = &file->sections[crossing->section_2];
			iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				}
			}

		/* or if tie not selected then plot current crossing in red */
		else if (mbna_current_crossing != MBNA_SELECT_NONE)
			{
			crossing = &(project.crossings[mbna_current_crossing]);

			file = &project.files[crossing->file_id_1];
			section = &file->sections[crossing->section_1];
			iping = section->modelplot_start_count + section->snav_id[section->num_snav/2];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[section->num_snav/2] / mbna_mtodeglon);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[section->num_snav/2] / mbna_mtodeglat);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[section->num_snav/2]);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				}

			file = &project.files[crossing->file_id_2];
			section = &file->sections[crossing->section_2];
			iping = section->modelplot_start_count + section->snav_id[section->num_snav/2];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[section->num_snav/2] / mbna_mtodeglon);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[section->num_snav/2] / mbna_mtodeglat);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[section->num_snav/2]);
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				}
			}

		/* if a modelplot pick did not resolve a single tie, plot the options for a second pick */
		if (mbna_modelplot_pickfile != MBNA_SELECT_NONE)
			{
			for (i=0;i<project.num_crossings;i++)
				{
				/* check if this crossing includes the picked snav */
				crossing = &(project.crossings[i]);

				/* check first snav */
				if (crossing->file_id_1 == mbna_modelplot_pickfile
					&& crossing->section_1 == mbna_modelplot_picksection)
					{
					/* loop over the ties */
					for (j=0;j<crossing->num_ties;j++)
						{
						tie = &(crossing->ties[j]);
						if (crossing->file_id_1 == mbna_modelplot_pickfile
						    && crossing->section_1 == mbna_modelplot_picksection
						    && tie->snav_1 == mbna_modelplot_picksnav)
							{
							file = &project.files[crossing->file_id_1];
							section = &file->sections[crossing->section_1];
							iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

							if (section->show_in_modelplot == MB_YES
							    && (mbna_modelplot_zoom == MB_NO
								|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
								{
								ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
								iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon);
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat);
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								}

							file = &project.files[crossing->file_id_2];
							section = &file->sections[crossing->section_2];
							iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

							if (section->show_in_modelplot == MB_YES
							    && (mbna_modelplot_zoom == MB_NO
								|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
								{
								ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
								iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon);
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat);
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								}
							}
						}
					}

				/* check second snav */
				if (crossing->file_id_2 == mbna_modelplot_pickfile
					&& crossing->section_2 == mbna_modelplot_picksection)
					{
					/* loop over the ties */
					for (j=0;j<crossing->num_ties;j++)
						{
						tie = &(crossing->ties[j]);
						if (crossing->file_id_2 == mbna_modelplot_pickfile
						    && crossing->section_2 == mbna_modelplot_picksection
						    && tie->snav_2 == mbna_modelplot_picksnav)
							{
							file = &project.files[crossing->file_id_2];
							section = &file->sections[crossing->section_2];
							iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

							if (section->show_in_modelplot == MB_YES
							    && (mbna_modelplot_zoom == MB_NO
								|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
								{
								ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
								iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon);
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat);
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								}

							file = &project.files[crossing->file_id_1];
							section = &file->sections[crossing->section_1];
							iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

							if (section->show_in_modelplot == MB_YES
							    && (mbna_modelplot_zoom == MB_NO
								|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
								{
								ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
								iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon);
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat);
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								}
							}
						}
					}
				}
			}

		/* plot zoom if active */
		if (mbna_modelplot_zoom_x1 != 0 || mbna_modelplot_zoom_x2 != 0)
			{
			imodelplot_start = (MIN(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale + mbna_modelplot_start;
			imodelplot_start = MIN(MAX(imodelplot_start, 0), project.num_pings - 1);
			imodelplot_end = (MAX(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale + mbna_modelplot_start;
			imodelplot_end = MIN(MAX(imodelplot_end, 0), project.num_pings - 1);

			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (imodelplot_start - mbna_modelplot_start));
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);

			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (imodelplot_end - mbna_modelplot_start));
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
			}

		/* reset clipping */
		xg_setclip(pmodp_xgid, 0, 0, mbna_modelplot_width, mbna_modelplot_height);

		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/

int
mbnavadjust_modelplot_plot_perturbation()
{
	/* local variables */
	char	*function_name = "mbnavadjust_modelplot_plot_perturbation";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	double	lon_offset_min;
	double	lon_offset_max;
	double	lat_offset_min;
	double	lat_offset_max;
	double	z_offset_min;
	double	z_offset_max;
	double	xymax, yzmax;
	int	plot_width;
	int	plot_height;
	int	first, iping;
	char	label[STRING_MAX];
	int	stringwidth, stringascent, stringdescent;
	int	pixel;
	int	ixo, iyo, ix, iy;
	int	i, j, isnav;
	int	imodelplot_start, imodelplot_end;
//fprintf(stderr,"Called %s\n",function_name);

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* plot model if an inversion has been performed */
    	if (project.open == MB_YES
    		&& project.inversion != MBNA_INVERSION_NONE
		&& project.modelplot == MB_YES)
    		{
		/* first loop over files setting all plot flags off for both files and sections */
		first = MB_YES;
		mbna_modelplot_count = 0;
 		for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			file->show_in_modelplot = MB_NO;
			for (j=0;j<file->num_sections;j++)
				{
				section = &file->sections[j];
				section->show_in_modelplot = MB_NO;
				}
			}

		/* now loop over files setting file or section plot flags on as necessary */
 		for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];

			/* check if this file will be plotted */
			if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY || mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
				{
				if (file->block == mbna_survey_select)
					{
					file->show_in_modelplot = MB_YES;
					}
				}

			/* check if this file will be plotted */
			else if (mbna_view_mode == MBNA_VIEW_MODE_FILE || mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
				{
				if (i == mbna_file_select)
					{
					file->show_in_modelplot = MB_YES;
					}
				}

			/* check if each section in this file will be plotted */
			else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
				{
				for (j=0;j<file->num_sections;j++)
					{
					section = &file->sections[j];
					if (i == mbna_file_select && j == mbna_section_select)
						{
						section->show_in_modelplot = MB_YES;
						}
					}
				}

			/* else every file will be plotted */
			else if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
				{
				file->show_in_modelplot = MB_YES;
				}
			}

		/* if view mode is with survey loop over all crossings */
		if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
			{
			for (i=0;i<project.num_crossings;i++)
				{
				crossing = &(project.crossings[i]);

				/* if either file is part of the selected survey
					then set plot flags on for both files */
				if (project.files[crossing->file_id_1].block == mbna_survey_select
				    || project.files[crossing->file_id_2].block == mbna_survey_select)
					{
					project.files[crossing->file_id_1].show_in_modelplot = MB_YES;
					project.files[crossing->file_id_2].show_in_modelplot = MB_YES;
					}
				}
			}

		/* else if view mode is with file loop over all crossings */
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
			{
			for (i=0;i<project.num_crossings;i++)
				{
				crossing = &(project.crossings[i]);

				/* if either file is selected
					then set plot flags on for both files */
				if (crossing->file_id_1 == mbna_file_select || crossing->file_id_2 == mbna_file_select)
					{
					project.files[crossing->file_id_1].show_in_modelplot = MB_YES;
					project.files[crossing->file_id_2].show_in_modelplot = MB_YES;
					}
				}
			}

		/* else if view mode is with section loop over all crossings */
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
			{
			for (i=0;i<project.num_crossings;i++)
				{
				crossing = &(project.crossings[i]);

				/* if either section is selected
					then set plot flags on for both files */
				if ((crossing->file_id_1 == mbna_file_select && crossing->section_1 == mbna_section_select)
				    || (crossing->file_id_2 == mbna_file_select && crossing->section_2 == mbna_section_select))
					{
					project.files[crossing->file_id_1].show_in_modelplot = MB_YES;
					project.files[crossing->file_id_2].show_in_modelplot = MB_YES;
					}
				}
			}

		/* finally loop over files again setting all section plot flags on for files with plot flags on */
 		for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			if (file->show_in_modelplot == MB_YES)
				{
				for (j=0;j<file->num_sections;j++)
					{
					section = &file->sections[j];
					section->show_in_modelplot = MB_YES;
					}
				}
			}


		/* get min maxes by looping over files and sections checking for sections to be plotted */
		first = MB_YES;
		mbna_modelplot_count = 0;
 		for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			for (j=0;j<file->num_sections;j++)
				{
				section = &file->sections[j];

				/* if this section will be plotted use the snav values */
				if (section->show_in_modelplot == MB_YES)
					{
					section->modelplot_start_count = mbna_modelplot_count;
					for (isnav=0;isnav<section->num_snav;isnav++)
						{
						if (mbna_modelplot_zoom == MB_NO
						    || (mbna_modelplot_count >= mbna_modelplot_startzoom && mbna_modelplot_count <= mbna_modelplot_endzoom))
						    {
							if (first == MB_YES)
								{
								lon_offset_min = section->snav_lon_offset[isnav] / mbna_mtodeglon - file->block_offset_x;
								lon_offset_max = section->snav_lon_offset[isnav] / mbna_mtodeglon - file->block_offset_x;
								lat_offset_min = section->snav_lat_offset[isnav] / mbna_mtodeglat - file->block_offset_y;
								lat_offset_max = section->snav_lat_offset[isnav] / mbna_mtodeglat - file->block_offset_y;
								z_offset_min = section->snav_z_offset[isnav] - file->block_offset_z;
								z_offset_max = section->snav_z_offset[isnav] - file->block_offset_z;
								first = MB_NO;
								}
							else
								{
								lon_offset_min = MIN(lon_offset_min, section->snav_lon_offset[isnav] / mbna_mtodeglon - file->block_offset_x);
								lon_offset_max = MAX(lon_offset_max, section->snav_lon_offset[isnav] / mbna_mtodeglon - file->block_offset_x);
								lat_offset_min = MIN(lat_offset_min, section->snav_lat_offset[isnav] / mbna_mtodeglat - file->block_offset_y);
								lat_offset_max = MAX(lat_offset_max, section->snav_lat_offset[isnav] / mbna_mtodeglat - file->block_offset_y);
								z_offset_min = MIN(z_offset_min, section->snav_z_offset[isnav] - file->block_offset_z);
								z_offset_max = MAX(z_offset_max, section->snav_z_offset[isnav] - file->block_offset_z);
								}
							}
						}
					mbna_modelplot_count += section->snav_id[section->num_snav-1];
					}
				}
			}

		/* set plot bounds */
		if (mbna_modelplot_zoom == MB_YES)
			{
			mbna_modelplot_start = mbna_modelplot_startzoom;
			mbna_modelplot_end = mbna_modelplot_endzoom;
			}
		else
			{
			mbna_modelplot_start = 0;
			mbna_modelplot_end = mbna_modelplot_count - 1;
			}

		/* get scaling */
		plot_width = mbna_modelplot_width - 8 * MBNA_MODELPLOT_X_SPACE;
		plot_height = (mbna_modelplot_height - 4 * MBNA_MODELPLOT_Y_SPACE) / 3;
		mbna_modelplot_xo = 5 * MBNA_MODELPLOT_X_SPACE;
		mbna_modelplot_yo_lon = MBNA_MODELPLOT_Y_SPACE + plot_height / 2;
		mbna_modelplot_yo_lat = 2 * MBNA_MODELPLOT_Y_SPACE + 3 * plot_height / 2;
		mbna_modelplot_yo_z = 3 * MBNA_MODELPLOT_Y_SPACE + 5 * plot_height / 2;
		xymax = MAX(fabs(lon_offset_min),fabs(lon_offset_max));
		xymax = MAX(fabs(lat_offset_min),xymax);
		xymax = MAX(fabs(lat_offset_max),xymax);
		mbna_modelplot_xscale = ((double)plot_width) / (mbna_modelplot_end - mbna_modelplot_start + 1);
		mbna_modelplot_yscale = ((double)plot_height) / (2.2 * xymax);
		yzmax = MAX(fabs(z_offset_min),fabs(z_offset_max));
		yzmax = MAX(yzmax,0.5);
		mbna_modelplot_yzscale = ((double)plot_height) / (2.2 * yzmax);

		/* clear screens for first plot */
		xg_fillrectangle(pmodp_xgid, 0, 0,
				modp_borders[1], modp_borders[3],
				pixel_values[mbna_color_background], XG_SOLIDLINE);

		/* plot the bounds */
		xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lon - plot_height / 2, plot_width, plot_height, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lon, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_lon, pixel_values[mbna_color_foreground], XG_DASHLINE);
		xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lat - plot_height / 2, plot_width, plot_height, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lat, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_lat, pixel_values[mbna_color_foreground], XG_DASHLINE);
		xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_z - plot_height / 2, plot_width, plot_height, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_z, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_z, pixel_values[mbna_color_foreground], XG_DASHLINE);

		/* plot title */
		if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
			{
			sprintf(label, "Display Only Selected Survey - Selected Survey:%d", mbna_survey_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
			{
			sprintf(label, "Display Only Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
			{
			sprintf(label, "Display With Selected Survey - Selected Survey:%d", mbna_survey_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
			{
			sprintf(label, "Display With Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
			{
			sprintf(label, "Display With Selected Section: Selected Survey/File/Section:%d/%d/%d",
				mbna_survey_select, mbna_file_select, mbna_section_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
			{
			sprintf(label, "Display All Data");
			}

		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = MBNA_MODELPLOT_Y_SPACE - 2 * stringascent;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		/* plot labels */
		sprintf(label, "East-West Offset (meters) vs. Ping Count");
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = mbna_modelplot_yo_lon - plot_height / 2 - stringascent / 4;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_start);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth / 2;
		iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_end);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
		iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lon - plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 0.0);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lon + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", -1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lon + plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);


		sprintf(label, "North-South Offset (meters) vs. Ping Count");
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = mbna_modelplot_yo_lat - plot_height / 2 - stringascent / 4;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_start);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth / 2;
		iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_end);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
		iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lat - plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 0.0);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lat + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", -1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lat + plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);


		sprintf(label, "Vertical Offset (meters) vs. Ping Count");
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = mbna_modelplot_yo_z - plot_height / 2 - stringascent / 4;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_start);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth / 2;
		iy = mbna_modelplot_yo_z + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_end);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
		iy = mbna_modelplot_yo_z + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 1.1 * yzmax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_z - plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 0.0);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_z + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", -1.1 * yzmax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_z + plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		/* set clipping */
		xg_setclip(pmodp_xgid, mbna_modelplot_xo, 0, plot_width, mbna_modelplot_height);

		/* loop over all crossings and plot and plot those without ties in green */
		for (i=0;i<project.num_crossings;i++)
		    {
		    crossing = &(project.crossings[i]);
		    if (crossing->num_ties== 0)
		    	{
			file = &project.files[crossing->file_id_1];
			section = &file->sections[crossing->section_1];
			iping = section->modelplot_start_count + section->snav_id[section->num_snav/2];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[section->num_snav/2] / mbna_mtodeglon - file->block_offset_x));
				xg_drawrectangle(pmodp_xgid, ix-3, iy-1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[section->num_snav/2] / mbna_mtodeglat - file->block_offset_y));
				xg_drawrectangle(pmodp_xgid, ix-3, iy-1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[section->num_snav/2] - file->block_offset_z));
				xg_drawrectangle(pmodp_xgid, ix-3, iy-1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
				}

			file = &project.files[crossing->file_id_2];
			section = &file->sections[crossing->section_2];
			iping = section->modelplot_start_count + section->snav_id[section->num_snav/2];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[section->num_snav/2] / mbna_mtodeglon - file->block_offset_x));
				xg_drawrectangle(pmodp_xgid, ix-3, iy-1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[section->num_snav/2] / mbna_mtodeglat - file->block_offset_y));
				xg_drawrectangle(pmodp_xgid, ix-3, iy-1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[section->num_snav/2] - file->block_offset_z));
				xg_drawrectangle(pmodp_xgid, ix-3, iy-1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
				}
			}
		    }

		/* Now plot the east-west offsets */
		ixo = 0;
		iyo = 0;
 		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			if (section->show_in_modelplot == MB_YES)
				{
				for (isnav=0;isnav<section->num_snav;isnav++)
				    {
				    iping = section->modelplot_start_count + section->snav_id[isnav];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[isnav] / mbna_mtodeglon - file->block_offset_x));
				    if ((i > 0 || j > 0) && section->continuity == MB_NO && isnav == 0)
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2, pixel_values[GREEN], XG_SOLIDLINE);
				    else if (i > 0 || j > 0)
					{
					/* if (j == 0 && isnav == 0 && section->continuity == MB_YES)
						xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2, pixel_values[CORAL], XG_DASHLINE); */
					xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
					}
				    ixo = ix;
				    iyo = iy;
				    }
				}
			}
		    }

		/* Now plot the north-south offsets */
		ixo = 0;
		iyo = 0;
 		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			if (section->show_in_modelplot == MB_YES)
				{
				for (isnav=0;isnav<section->num_snav;isnav++)
				    {
				    iping = section->modelplot_start_count + section->snav_id[isnav];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[isnav] / mbna_mtodeglat - file->block_offset_y));
				    if ((i > 0 || j > 0) && section->continuity == MB_NO && isnav == 0)
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2, pixel_values[GREEN], XG_SOLIDLINE);
				    else if (i > 0 || j > 0)
					{
					/* if (j == 0 && isnav == 0 && section->continuity == MB_YES)
						xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2, pixel_values[CORAL], XG_DASHLINE); */
					xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
					}
				    ixo = ix;
				    iyo = iy;
				    }
				}
			}
		    }

		/* Now plot the vertical offsets */
		ixo = 0;
		iyo = 0;
 		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			if (section->show_in_modelplot == MB_YES)
				{
				for (isnav=0;isnav<section->num_snav;isnav++)
				    {
				    iping = section->modelplot_start_count + section->snav_id[isnav];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[isnav] - file->block_offset_z));
				    if ((i > 0 || j > 0) && section->continuity == MB_NO && isnav == 0)
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2, pixel_values[GREEN], XG_SOLIDLINE);
				    else if (i > 0 || j > 0)
					{
					/* if (j == 0 && isnav == 0 && section->continuity == MB_YES)
						xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2, pixel_values[CORAL], XG_DASHLINE); */
					xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
					}
				    ixo = ix;
				    iyo = iy;
				    }
				}
			}
		    }

		/* loop over all crossings and plot ties */
		for (i=0;i<project.num_crossings;i++)
		    {
		    crossing = &(project.crossings[i]);
		    for (j=0;j<crossing->num_ties;j++)
		    	{
			tie = &(crossing->ties[j]);

			if (tie->inversion_status == MBNA_INVERSION_CURRENT)
				pixel = pixel_values[mbna_color_foreground];
			else
				pixel = pixel_values[BLUE];

			file = &project.files[crossing->file_id_1];
			section = &file->sections[crossing->section_1];
			iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon - file->block_offset_x));
				xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat - file->block_offset_y));
				xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_1] - file->block_offset_z));
				xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
				}

			file = &project.files[crossing->file_id_2];
			section = &file->sections[crossing->section_2];
			iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon - file->block_offset_x));
				xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat - file->block_offset_y));
				xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_2] - file->block_offset_z));
				xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
				}
			}
		    }

		/* Loop over all files plotting global ties */
 		for (i=0;i<project.num_files;i++)
		    {
		    file = &project.files[i];
		    for (j=0;j<file->num_sections;j++)
			{
			section = &file->sections[j];
			if (section->show_in_modelplot == MB_YES && section->global_tie_status != MBNA_TIE_NONE)
				{
				for (isnav=0;isnav<section->num_snav;isnav++)
				    {
				    iping = section->modelplot_start_count + section->snav_id[section->global_tie_snav];
				    ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				    
				    if (section->global_tie_status != MBNA_TIE_Z)
					{
					/* east-west offsets */
					iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[section->global_tie_snav] / mbna_mtodeglon - file->block_offset_x));
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
					xg_fillrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
					xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
					
					/* north-south offsets */
					iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[section->global_tie_snav] / mbna_mtodeglat - file->block_offset_y));
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
					xg_fillrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
					xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
					}
				    
				    if (section->global_tie_status != MBNA_TIE_XY)
					{
					/* vertical offsets */
					iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[section->global_tie_snav] - file->block_offset_z));
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
					xg_fillrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
					xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
					}
				    }
				}
			}
		    }

		/* plot current tie in red */
		if (mbna_current_crossing != MBNA_SELECT_NONE && mbna_current_tie != MBNA_SELECT_NONE)
			{
			crossing = &(project.crossings[mbna_current_crossing]);
			tie = &(crossing->ties[mbna_current_tie]);

			file = &project.files[crossing->file_id_1];
			section = &file->sections[crossing->section_1];
			iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon - file->block_offset_x));
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat - file->block_offset_y));
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_1] - file->block_offset_z));
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				}

			file = &project.files[crossing->file_id_2];
			section = &file->sections[crossing->section_2];
			iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon - file->block_offset_x));
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat - file->block_offset_y));
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_2] - file->block_offset_z));
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				}
			}

		/* or if tie not selected then plot current crossing in red */
		else if (mbna_current_crossing != MBNA_SELECT_NONE)
			{
			crossing = &(project.crossings[mbna_current_crossing]);

			file = &project.files[crossing->file_id_1];
			section = &file->sections[crossing->section_1];
			iping = section->modelplot_start_count + section->snav_id[section->num_snav/2];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[section->num_snav/2] / mbna_mtodeglon - file->block_offset_x));
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[section->num_snav/2] / mbna_mtodeglat - file->block_offset_y));
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[section->num_snav/2] - file->block_offset_z));
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				}

			file = &project.files[crossing->file_id_2];
			section = &file->sections[crossing->section_2];
			iping = section->modelplot_start_count + section->snav_id[section->num_snav/2];

			if (section->show_in_modelplot == MB_YES
			    && (mbna_modelplot_zoom == MB_NO
				|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
				{
				ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
				iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[section->num_snav/2] / mbna_mtodeglon - file->block_offset_x));
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[section->num_snav/2] / mbna_mtodeglat - file->block_offset_y));
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[section->num_snav/2] - file->block_offset_z));
				xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
				xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
				}
			}

		/* if a modelplot pick did not resolve a single tie, plot the options for a second pick */
		if (mbna_modelplot_pickfile != MBNA_SELECT_NONE)
			{
			for (i=0;i<project.num_crossings;i++)
				{
				/* check if this crossing includes the picked snav */
				crossing = &(project.crossings[i]);

				/* check first snav */
				if (crossing->file_id_1 == mbna_modelplot_pickfile
					&& crossing->section_1 == mbna_modelplot_picksection)
					{
					/* loop over the ties */
					for (j=0;j<crossing->num_ties;j++)
						{
						tie = &(crossing->ties[j]);
						if (crossing->file_id_1 == mbna_modelplot_pickfile
						    && crossing->section_1 == mbna_modelplot_picksection
						    && tie->snav_1 == mbna_modelplot_picksnav)
							{
							file = &project.files[crossing->file_id_1];
							section = &file->sections[crossing->section_1];
							iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

							if (section->show_in_modelplot == MB_YES
							    && (mbna_modelplot_zoom == MB_NO
								|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
								{
								ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
								iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon - file->block_offset_x));
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat - file->block_offset_y));
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_1] - file->block_offset_z));
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								}

							file = &project.files[crossing->file_id_2];
							section = &file->sections[crossing->section_2];
							iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

							if (section->show_in_modelplot == MB_YES
							    && (mbna_modelplot_zoom == MB_NO
								|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
								{
								ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
								iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon - file->block_offset_x));
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat - file->block_offset_y));
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_2] - file->block_offset_z));
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								}
							}
						}
					}

				/* check second snav */
				if (crossing->file_id_2 == mbna_modelplot_pickfile
					&& crossing->section_2 == mbna_modelplot_picksection)
					{
					/* loop over the ties */
					for (j=0;j<crossing->num_ties;j++)
						{
						tie = &(crossing->ties[j]);
						if (crossing->file_id_2 == mbna_modelplot_pickfile
						    && crossing->section_2 == mbna_modelplot_picksection
						    && tie->snav_2 == mbna_modelplot_picksnav)
							{
							file = &project.files[crossing->file_id_2];
							section = &file->sections[crossing->section_2];
							iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

							if (section->show_in_modelplot == MB_YES
							    && (mbna_modelplot_zoom == MB_NO
								|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
								{
								ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
								iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[tie->snav_2] / mbna_mtodeglon - file->block_offset_x));
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[tie->snav_2] / mbna_mtodeglat - file->block_offset_y));
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_2] - file->block_offset_z));
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								}

							file = &project.files[crossing->file_id_1];
							section = &file->sections[crossing->section_1];
							iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

							if (section->show_in_modelplot == MB_YES
							    && (mbna_modelplot_zoom == MB_NO
								|| (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom)))
								{
								ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
								iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->snav_lon_offset[tie->snav_1] / mbna_mtodeglon - file->block_offset_x));
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->snav_lat_offset[tie->snav_1] / mbna_mtodeglat - file->block_offset_y));
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_1] - file->block_offset_z));
								xg_fillrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[6], XG_SOLIDLINE);
								xg_drawrectangle(pmodp_xgid, ix-5, iy-5, 11, 11, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
								}
							}
						}
					}
				}
			}

		/* plot zoom if active */
		if (mbna_modelplot_zoom_x1 != 0 || mbna_modelplot_zoom_x2 != 0)
			{
			imodelplot_start = (MIN(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale + mbna_modelplot_start;
			imodelplot_start = MIN(MAX(imodelplot_start, 0), project.num_pings - 1);
			imodelplot_end = (MAX(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale + mbna_modelplot_start;
			imodelplot_end = MIN(MAX(imodelplot_end, 0), project.num_pings - 1);

			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (imodelplot_start - mbna_modelplot_start));
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);

			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (imodelplot_end - mbna_modelplot_start));
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
			}

		/* reset clipping */
		xg_setclip(pmodp_xgid, 0, 0, mbna_modelplot_width, mbna_modelplot_height);

		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/

int
mbnavadjust_modelplot_plot_tieoffsets()
{
	/* local variables */
	char	*function_name = "mbnavadjust_modelplot_plot_tieoffsets";
	int	status = MB_SUCCESS;
	struct mbna_file *file;
	struct mbna_section *section;
	struct mbna_crossing *crossing;
	struct mbna_tie *tie;
	int	num_ties;
	int	num_surveys;
	double	lon_offset_min;
	double	lon_offset_max;
	double	lat_offset_min;
	double	lat_offset_max;
	double	z_offset_min;
	double	z_offset_max;
	double	xymax, yzmax;
	int	plot_width;
	int	plot_height;
	int	first;
	char	label[STRING_MAX];
	int	stringwidth, stringascent, stringdescent;
	int	pixel;
	int	itiestart, itieend;
	int	ix, iy;
	int	num_ties_block;
	int	num_blocks;
	int	plot_index;
	int	isurvey1, isurvey2;
	int	i, j;
//fprintf(stderr,"Called %s\n",function_name);

 	/* print input debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		}

	/* plot model if an inversion has been performed */
    	if (project.open == MB_YES
    		&& project.inversion != MBNA_INVERSION_NONE
		&& project.modelplot == MB_YES)
    		{
		/* get number of ties and surveys */
		num_ties = project.num_ties;
		mbna_num_ties_plot = 0;
		num_surveys = 0;

		/* count surveys and plot blocks by looping over files */
		num_surveys = 1;
 		for (i=0;i<project.num_files;i++)
			{
			file = &project.files[i];
			file->show_in_modelplot = -1;
			for (j=0;j<file->num_sections;j++)
				{
				section = &file->sections[j];
				if ((i > 0 || j > 0) && section->continuity == MB_NO)
					num_surveys++;
				}
			}
		num_blocks = 0;
		for (i=0;i<num_surveys;i++)
			num_blocks += i + 1;

		/* Figure out which ties might be plotted */
 		for (i=0;i<project.num_crossings;i++)
			{
			crossing = &project.crossings[i];
			for (j=0;j<crossing->num_ties;j++)
				{
				tie = &crossing->ties[j];
				tie->block_1 = project.files[crossing->file_id_1].block;
				tie->block_2 = project.files[crossing->file_id_2].block;
				tie->isurveyplotindex = -1;

				/* check to see if this tie should be plotted */
				if (mbna_modelplot_blocksurvey1 != MBNA_SELECT_NONE && mbna_modelplot_blocksurvey2 != MBNA_SELECT_NONE)
					{
					if (tie->block_1 == mbna_modelplot_blocksurvey1
						&& tie->block_2 == mbna_modelplot_blocksurvey2)
						{
						tie->isurveyplotindex = 1;
						mbna_num_ties_plot++;
						}
					}
				else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
					{
					if (tie->block_1 == mbna_survey_select
					    && tie->block_2 == mbna_survey_select)
						{
						tie->isurveyplotindex = 1;
						mbna_num_ties_plot++;
						}
					}
				else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
					{
					if (tie->block_1 == mbna_survey_select
					    || tie->block_2 == mbna_survey_select)
						{
						tie->isurveyplotindex = 1;
						mbna_num_ties_plot++;
						}
					}
				else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
					{
					if (crossing->file_id_1 == mbna_file_select
					    && crossing->file_id_2 == mbna_file_select)
						{
						tie->isurveyplotindex = 1;
						mbna_num_ties_plot++;
						}
					}
				else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
					{
					if (crossing->file_id_1 == mbna_file_select
					    || crossing->file_id_2 == mbna_file_select)
						{
						tie->isurveyplotindex = 1;
						mbna_num_ties_plot++;
						}
					}
				else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
					{
					if ((crossing->file_id_1 == mbna_file_select && crossing->section_1 == mbna_section_select)
					    || (crossing->file_id_2 == mbna_file_select && crossing->section_2 == mbna_section_select))
						{
						tie->isurveyplotindex = 1;
						mbna_num_ties_plot++;
						}
					}
				else if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
					{
					tie->isurveyplotindex = 1;
					mbna_num_ties_plot++;
					}
				}
			}

		/* Get min maxes by looping over surveys to locate each tie in plot */
		plot_index = 0;
		first = MB_YES;
		if (mbna_modelplot_tiezoom == MB_YES)
			{
			mbna_modelplot_tiestart = mbna_modelplot_tiestartzoom;
			mbna_modelplot_tieend = mbna_modelplot_tieendzoom;
			}
		else
			{
			mbna_modelplot_tiestart = 0;
			mbna_modelplot_tieend = mbna_num_ties_plot - 1;
			}
		for (isurvey2=0;isurvey2<num_surveys;isurvey2++)
			{
			for (isurvey1=0;isurvey1<=isurvey2;isurvey1++)
				{
				/* loop over all ties looking for ones in current plot block
					- current plot block is for ties between
					surveys isurvey1 and isurvey2 */
				for (i=0;i<project.num_crossings;i++)
					{
					crossing = &project.crossings[i];
					for (j=0;j<crossing->num_ties;j++)
						{
						tie = &crossing->ties[j];

						/* check if this tie is between the desired surveys */
						if (tie->isurveyplotindex >= 0
						    && ((tie->block_1 == isurvey1 && tie->block_2 == isurvey2)
							|| (tie->block_2 == isurvey1 && tie->block_1 == isurvey2)))
							{
							/* set plot index for tie */
							tie->isurveyplotindex = plot_index;

							/* increment plot_index */
							plot_index++;

							/* if tie will be plotted (check for zoom) use it for min max */
							if (tie->isurveyplotindex >= 0
							    && (tie->isurveyplotindex >= mbna_modelplot_tiestart
								    && tie->isurveyplotindex <= mbna_modelplot_tieend))
								{
								if (first == MB_YES)
									{
									lon_offset_min = tie->offset_x_m;
									lon_offset_max = tie->offset_x_m;
									lat_offset_min = tie->offset_y_m;
									lat_offset_max = tie->offset_y_m;
									z_offset_min = tie->offset_z_m;
									z_offset_max = tie->offset_z_m;
									first = MB_NO;
									}
								else
									{
									lon_offset_min = MIN(lon_offset_min, tie->offset_x_m);
									lon_offset_max = MAX(lon_offset_max, tie->offset_x_m);
									lat_offset_min = MIN(lat_offset_min, tie->offset_y_m);
									lat_offset_max = MAX(lat_offset_max, tie->offset_y_m);
									z_offset_min = MIN(z_offset_min, tie->offset_z_m);
									z_offset_max = MAX(z_offset_max, tie->offset_z_m);
									}
								}
							}
						}
					}
				}
			}

		/* get scaling */
		plot_width = mbna_modelplot_width - 8 * MBNA_MODELPLOT_X_SPACE;
		plot_height = (mbna_modelplot_height - 4 * MBNA_MODELPLOT_Y_SPACE) / 3;
		mbna_modelplot_xo = 5 * MBNA_MODELPLOT_X_SPACE;
		mbna_modelplot_yo_lon = MBNA_MODELPLOT_Y_SPACE + plot_height / 2;
		mbna_modelplot_yo_lat = 2 * MBNA_MODELPLOT_Y_SPACE + 3 * plot_height / 2;
		mbna_modelplot_yo_z = 3 * MBNA_MODELPLOT_Y_SPACE + 5 * plot_height / 2;
		xymax = MAX(fabs(lon_offset_min),fabs(lon_offset_max));
		xymax = MAX(fabs(lat_offset_min),xymax);
		xymax = MAX(fabs(lat_offset_max),xymax);
		mbna_modelplot_xscale = ((double)plot_width) / (mbna_modelplot_tieend - mbna_modelplot_tiestart + 1);
		mbna_modelplot_yscale = ((double)plot_height) / (2.2 * xymax);
		yzmax = MAX(fabs(z_offset_min),fabs(z_offset_max));
		yzmax = MAX(yzmax,0.5);
		mbna_modelplot_yzscale = ((double)plot_height) / (2.2 * yzmax);

		/* clear screens for first plot */
		xg_fillrectangle(pmodp_xgid, 0, 0,
				modp_borders[1], modp_borders[3],
				pixel_values[mbna_color_background], XG_SOLIDLINE);

		/* plot the bounds */
		xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lon - plot_height / 2, plot_width, plot_height, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lon, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_lon, pixel_values[mbna_color_foreground], XG_DASHLINE);
		xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lat - plot_height / 2, plot_width, plot_height, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lat, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_lat, pixel_values[mbna_color_foreground], XG_DASHLINE);
		xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_z - plot_height / 2, plot_width, plot_height, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
		xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_z, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_z, pixel_values[mbna_color_foreground], XG_DASHLINE);

		/* plot title */
		if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY)
			{
			sprintf(label, "Display Only Selected Survey - Selected Survey:%d", mbna_survey_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_FILE)
			{
			sprintf(label, "Display Only Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY)
			{
			sprintf(label, "Display With Selected Survey - Selected Survey:%d", mbna_survey_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE)
			{
			sprintf(label, "Display With Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION)
			{
			sprintf(label, "Display With Selected Section: Selected Survey/File/Section:%d/%d/%d",
				mbna_survey_select, mbna_file_select, mbna_section_select);
			}
		else if (mbna_view_mode == MBNA_VIEW_MODE_ALL)
			{
			sprintf(label, "Display All Data");
			}

		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = MBNA_MODELPLOT_Y_SPACE - 2 * stringascent;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		/* plot labels */
		sprintf(label, "Tie East-West Offset (meters) Grouped by Surveys");
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = mbna_modelplot_yo_lon - plot_height / 2 - stringascent / 4;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_tiestart);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth / 2;
		iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_tieend);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
		iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lon - plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 0.0);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lon + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", -1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lon + plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);


		sprintf(label, "Tie North-South Offset (meters) Grouped by Surveys");
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = mbna_modelplot_yo_lat - plot_height / 2 - stringascent / 4;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_tiestart);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth / 2;
		iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_tieend);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
		iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lat - plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 0.0);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lat + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", -1.1 * xymax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_lat + plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);


		sprintf(label, "Tie Vertical Offset (meters) Grouped by Surveys");
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
		iy = mbna_modelplot_yo_z - plot_height / 2 - stringascent / 4;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_tiestart);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth / 2;
		iy = mbna_modelplot_yo_z + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%d", mbna_modelplot_tieend);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
		iy = mbna_modelplot_yo_z + plot_height / 2 + 3 *stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 1.1 * yzmax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_z - plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", 0.0);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_z + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		sprintf(label,"%.2f", -1.1 * yzmax);
		xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
		ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
		iy = mbna_modelplot_yo_z + plot_height / 2 + stringascent / 2;
		xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

		/* set clipping */
		xg_setclip(pmodp_xgid, mbna_modelplot_xo, 0, plot_width, mbna_modelplot_height);

		/* loop over surveys to locate each tie in plot */
		plot_index = 0;
		for (isurvey2=0;isurvey2<num_surveys;isurvey2++)
			{
			for (isurvey1=0;isurvey1<=isurvey2;isurvey1++)
				{
				num_ties_block = 0;

				/* loop over all ties looking for ones in current plot block
					- current plot block is for ties between
					surveys isurvey1 and isurvey2 */
				for (i=0;i<project.num_crossings;i++)
					{
					crossing = &project.crossings[i];
					for (j=0;j<crossing->num_ties;j++)
						{
						tie = &crossing->ties[j];

						/* check if this tie is between the desired surveys */
						if (tie->isurveyplotindex >= 0
						    && ((tie->block_1 == isurvey1 && tie->block_2 == isurvey2)
							|| (tie->block_2 == isurvey1 && tie->block_1 == isurvey2)))
							{
							if (tie->isurveyplotindex >= mbna_modelplot_tiestart
								    && tie->isurveyplotindex <= mbna_modelplot_tieend)
								{
								/* plot tie */
								if (tie->inversion_status == MBNA_INVERSION_CURRENT)
									pixel = pixel_values[mbna_color_foreground];
								else
									pixel = pixel_values[BLUE];

								ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (tie->isurveyplotindex - mbna_modelplot_tiestart));

								iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * tie->offset_x_m);
								if (i == mbna_current_crossing && j == mbna_current_tie)
									{
									xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
									xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
									}
								else
									xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);

								iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * tie->offset_y_m);
								if (i == mbna_current_crossing && j == mbna_current_tie)
									{
									xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
									xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
									}
								else
									xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);

								iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * tie->offset_z_m);
								if (i == mbna_current_crossing && j == mbna_current_tie)
									{
									xg_fillrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
									xg_drawrectangle(pmodp_xgid, ix-3, iy-3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
									}
								else
									xg_drawrectangle(pmodp_xgid, ix-2, iy-2, 5, 5, pixel, XG_SOLIDLINE);
								}

							/* increment plot_index */
							plot_index++;

							/* increment num_ties_block */
							num_ties_block++;
							}
						}
					}

				/* plot line for boundary between plot blocks */
				if (num_ties_block > 0)
					{
					/* plot line for boundary between plot blocks */
					ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (plot_index - mbna_modelplot_tiestart - 0.5));
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2,
								ix, mbna_modelplot_yo_lon + plot_height / 2,
								pixel_values[GREEN], XG_DASHLINE);
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2,
								ix, mbna_modelplot_yo_lat + plot_height / 2,
								pixel_values[GREEN], XG_DASHLINE);
					xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2,
								ix, mbna_modelplot_yo_z + plot_height / 2,
								pixel_values[GREEN], XG_DASHLINE);
					}
				}
			}

		/* plot zoom if active */
		if (mbna_modelplot_zoom_x1 != 0 || mbna_modelplot_zoom_x2 != 0)
			{
			itiestart = (MIN(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale + mbna_modelplot_tiestart;
			itiestart = MIN(MAX(itiestart, 0), mbna_num_ties_plot - 1);
			itieend = (MAX(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale + mbna_modelplot_tiestart;
			itieend = MIN(MAX(itieend, 0), mbna_num_ties_plot - 1);

			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (itiestart - mbna_modelplot_tiestart));
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);

			ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (itieend - mbna_modelplot_tiestart));
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
		   	xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2, pixel_values[mbna_color_foreground], XG_DASHLINE);
			}

		/* reset clipping */
		xg_setclip(pmodp_xgid, 0, 0, mbna_modelplot_width, mbna_modelplot_height);
		}

 	/* print output debug statements */
	if (mbna_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBnavadjust function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:       %d\n",error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------*/
